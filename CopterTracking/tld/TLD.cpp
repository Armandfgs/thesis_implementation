#include "../include/TLD.h"

TLD::TLD()
{
    //TLD ist standardmäßig inaktiv. Wird erst durch definition der ersten Box aktiviert!
    isActive = false;
    wasValide = false;
    isInilized = false;
    useDetector = true;
    useTracker = true;
}

TLD::~TLD()
{
    //dtor
}

void TLD::init(Mat& f, Rect& bb, int maxW, int maxH)
{
    if (isInilized) return;
    lastFrame = f.clone();
    tbb = Rect();
    dBoxes = vector<Rect>();
    //Tracker vorbereiten
    tracker = Tracker();
    hasTracked = false;
    showTracker = false;

    /// ParticleFilter vorbereiten
    particleFilter = ParticleFilter();
    particleFilter.init();
    /// KalmanTracker vorbereiten
    kalman = KalmanTracker();
    kalman.init(bb);

    //Detector vorbereiten
    detector = Detector();
    detector.init(bb,maxW,maxH);

    hasDetected = false;
    showDetector = false;
    //Ausgabe der Box im extra Fenster
    showBoundingBox = false;
    //Ausgabe der Koordinaten der Box
    showCoordinates = false;
    //Ausgabe der Positiven Beispiele zum Lernen
    showPositives = false; showHull = false;
    showFernPos = true; showFernNeg = true;
    showNNCPos = false; showNNCNeg = false;
    //Aktiviere Lernen
    doLearn = true;
    // Initialisierung des PatchGenerators zum Erzeugten der skalierten Patches

    int noise_init = 5;
    float scale_init = .02;
    int angle_init = 20;
    patchGenerator = PatchGenerator (0,0,noise_init,true,1-scale_init,1+scale_init,-angle_init*CV_PI/180,angle_init*CV_PI/180,-angle_init*CV_PI/180,angle_init*CV_PI/180);
    //Jetzt den lastState (der besagt, ob was gefunden wurde im letzten Durchlauf - hier natürlich nach der Definition der ersten Box) auf true setzen
    lastState = true;
    //Nach Initialisierung wird TLD aktiv
    isActive = true;
}

/**
 * Die Methode bereitet die Klassifizierer vor. Genauer gesagt,
 * werden die posteriors für den ersten Patch p = f(bb) berechnet
 * die FernFeatures inititalisiert.
 *
 * f: das aktuelle Bild
 * bb: die Box, mit deren Hilfe die Pos. und Neg. Beispiele berechnet werden
 *
 */
void TLD::initLearning(Rect& bb)
{
    isInilized = true;
    cout << "Start first Learning... " << endl;
    /// VarianceCLassifier
    Scalar stdev, mean;
    meanStdDev(lastFrame(bb),mean,stdev);
    detector.varianceCF.variance = pow(stdev[0],2) * .5;
    /// Beispiele generieren und lernen
    // TODO
    Result *result = detector.resultSet;
    vector<Rect> dummy;
    cout << "\tFirst Detection ";
    detector.detect(lastFrame,dummy);
    cout << "\tDONE!" << endl;
    //This is the positive patch
    Mat patch;
    Utility::generateNormalizedPatch(lastFrame(bb),patch);

    // Überlappungen berechnen!
    float *overlap = new float[detector.gridSize];
    Utility::calcOverlap(detector.grid,detector.gridSize,bb,overlap);
    //Add all bounding boxes with high overlap

    vector< pair<int, float> > positiveIndices;
    vector<int> negativeIndices;

    //First: Find overlapping positive and negative patches

    for(int i = 0; i < detector.gridSize; ++i)
    {

        if(overlap[i] > 0.6)
        {
            positiveIndices.push_back(pair<int, float>(i, overlap[i]));
        }

        if(overlap[i] < 0.2)
        {
            float variance = result->variances[i];

            if(variance > detector.varianceCF.variance)   //TODO: This check is unnecessary if minVar would be set before calling detect.
            {
                negativeIndices.push_back(i);
            }
        }
    }

    sort(positiveIndices.begin(), positiveIndices.end(), Utility::compareOverlap);

    vector<Mat> patches;

    patches.push_back(patch); //Add first patch to patch list
    detector.nearestnCF.learn(patches,true);/// HIER NNC-Lernen


    int numIterations = std::min<size_t>(positiveIndices.size(), 10); //Take at most 10 bounding boxes (sorted by overlap)
    int index = 0;
    // Hier wird der EnsembleClassifier trainiert.
    /// TODO > Nutzen des PatchGenerators!
    /* Für den PatchGenerator muss erst die Hülle über alle positiven Boxen generiert werden.
        Danach kann das Bild verändert und als Quelle für neue Versionen genutzt werden.
    */
    /// Berechne Hülle (für das Erzeugen der synthetischen Beispiele)
    Rect hull;
    Utility::calcHull(detector.grid,positiveIndices,hull);
    Point2f center = Point2f(hull.width/2, hull.height/2); center = center;
    Mat gaussF; // Gaussian Version des patches
    GaussianBlur(lastFrame,gaussF,Size(9,9),1.5);
    Mat clone = lastFrame.clone(); // Bild muss geklont werden, weil es verändert wird
    Mat roi = clone(hull); // Das ist der Teil, der geändert wird
    RNG& random = theRNG(); random = random;// Der Patchgenerator braucht einen Zufallszahlengenerator
    /// HIER WEITER
    // EC - lernen positiver Beispiele
    for (int j = 0; j < 20; ++j) {
        patchGenerator(gaussF,center,roi,hull.size(),random);
        for(int i = 0; i < numIterations; i++)
        {
            index = positiveIndices.at(i).first;
            /// An dieser Stelle werden für jeden positiven Patch 20 warped-Versionen erzeugt
            // Fern-Vector für die aktuelle BB und den generierten Patch berechnen (das Rechteck wird innerhalb der Funktion ausgeschnitten)
            detector.ensembleCF.calculateFernFeatureVector(index,detector.grid,&gaussF,&result->featureVectors[detector.ensembleCF.getNumberOfFerns() * index]);
            detector.ensembleCF.learn(index,result,true);
        }

    }
    srand(1); //TODO: This is not guaranteed to affect random_shuffle

    random_shuffle(negativeIndices.begin(), negativeIndices.end());
    patches.clear();
    Rect rect;
    //NNC - Lernen negativer Beispiele
    for(size_t i = 0; i < std::min<size_t>(100, negativeIndices.size()); i++)
    {
        index = negativeIndices.at(i);
        /// Patch ermitteln
        rect = Rect(detector.grid[index * 4], detector.grid[index * 4 + 1],detector.grid[index * 4 + 2],detector.grid[index * 4 + 3]);
        patch = lastFrame(rect);
        patches.push_back(patch);
    }

    /// Negative NNC lernen (TODO)
    //detectorCascade->nnClassifier->learn(patches);
    detector.nearestnCF.learn(patches,false);
    delete[] overlap;





    //generateAndLearnPosExamples(bb,detector.objectModel.getGrid(),10,20);
    //generateAndLearnNegExamples(100,100); //So auch im Beispiel
    wasValide = true;
    cout << "First learning DONE!" << endl;
}


bool TLD::process(Mat& f2, Rect& box, Rect& nextBox, Mat& show)
{
    if (!isInilized) {
        initLearning(box);
        wasValide = true;
    }
    // Alterniere Detector
    //useDetector = !useDetector;
    dBoxes.clear();
    //1. Tracker
    tracker.showTracker = showTracker;
    hasTracked = false;
    //Falls eine Box gefunden wurde im letzten durchgang (lastState = true), dann tracke!!
    tbb = Rect();
    kbb = Rect();
    kalman.track(box,kbb);
    if (useTracker && lastState) {
        hasTracked = tracker.track(lastFrame,f2,box,kbb,tbb);
        //particleFilter.track(box,lastFrame);

    } else hasTracked = false;
    //2. Detector
    detector.showDetector = showDetector;
    /// HIER KÖNNTE MAN NOCH PRÜFEN, OB WIRKLICH GETRACKED WERDEN MUSS, INDEM MAN DIE TRACKERBOX BEWERTET >> Naja, nicht wirklich besser!!!
    /*
    float t_sim_r = 0;
    if (hasTracked) {
        Mat patch = lastFrame(tbb);
        Mat normalizedPatch;
        Utility::generateNormalizedPatch(patch,normalizedPatch);
        t_sim_r = detector.nearestnCF.classify(normalizedPatch);
    }
    cout << t_sim_r << endl;*/
    //if (!hasTracked)
    if (useDetector) hasDetected = detector.detect(f2,dBoxes);
    else hasDetected = false;
    // Hier werden nun die Boxen analysiert und der Erfolg von Tracking/Detection zurückgegeben
    lastState = compareBoundingBoxes(f2,tbb,dBoxes,nextBox);
    //LastFrame aktualisieren
    lastFrame = f2.clone();
    //Lernen, falls valide
    if (doLearn && isValide) learn(nextBox, show);
    //if (wasValide) wasValide = false;
    //else
    wasValide = isValide; // Validität merken
    // NUR FÜR DIE TESTAUSGABEN
    resultBox = nextBox; /// Kann eigentlich raus, oder?
    //generateWindows(showFrame);
    // Status zurückgeben
    return lastState;
}


/**
 * Aus tbb (TrackerBox) und dbb (DetectorBox) wird eine finale Box (fbb) erzeugt.
 * Diese definiert dann das Endergebnis.
 */
bool TLD::compareBoundingBoxes(Mat& f,Rect& tbb, vector<Rect>& dBoxes, Rect& fbb)
{
    //float t_sim_c = .0, t_sim_r = .0, d_sim_c = .0, d_sim_r = .0; // Bewertung der Similarites von Tracker und Detector
    //bool isSigPos, isSigNeg;
    float d_sim_r, t_sim_r;
    detector.nearestnCF.sim_d = 0;
    detector.nearestnCF.sim_t = 0;
    Mat patch;
    bool success = false;
    isValide = false;
    Mat normalizedPatch;
    // Falls es nur eine Box vom Detector gab, dann berechne similarity
    if (dBoxes.size() == 1) {
        patch = f(dBoxes[0]);
        Utility::generateNormalizedPatch(patch,normalizedPatch);
        //detector.nearestnCF.calculateSimilarity(normalizedPatch,d_sim_c,d_sim_r,isSigPos,isSigNeg);
        d_sim_r = detector.nearestnCF.classify(normalizedPatch);
        // Merken der aktuellen similarity
        detector.nearestnCF.sim = d_sim_r;
        detector.nearestnCF.sim_d = d_sim_r;
        // nur für den Falls, dass nicht getrackt wurde, wird fbb gesetzt
    }
    //if (wasValide) wasValide = false;
    // Falls was GETRACKED wurde
    if (hasTracked) {
        // Berechne similarity vom Tracker
        // Prüfe, ob die Box nicht außerhalb des Bildes liegt
        if (tbb.x + tbb.width > f.cols) tbb.width -= (tbb.width + tbb.x - f.cols) + 1;
        if (tbb.y + tbb.height > f.rows) tbb.height -= (tbb.height + tbb.y - f.rows) + 1;
        if (tbb.x < 0) tbb.x = 0;
        if (tbb.y < 0) tbb.y = 0;
        if (tbb.width < 1 || tbb.height < 1) return false;
        patch = f(tbb);
        Utility::generateNormalizedPatch(patch,normalizedPatch);
        //detector.nearestnCF.calculateSimilarity(normalizedPatch,t_sim_c,t_sim_r,isSigPos,isSigNeg);
        t_sim_r = detector.nearestnCF.classify(normalizedPatch);
        // Falls nun nur EINE BOX vom Detector gefunden wurde, deren sim-Wert > als der vom Tracker ist
        // und die Tracker und Detecotorbox sich um weniger als .5 überlappen, dann ist die neue Box die Detectorbox
        //cout << "D_SIM: " << d_sim_r << " T_SIM: " << t_sim_r << " overlap: ";
        if (dBoxes.size() == 1 && d_sim_r > t_sim_r && Utility::calcOverlap(dBoxes[0],tbb) < .6) {
        //if (dBoxes.size() == 1 && d_sim_c > t_sim_c && Utility::calcOverlap(dBoxes[0],tbb) < .5) {
            fbb = dBoxes[0];
            success = true;
           // cout << "TLD::compareBoundingBox > Trackerbox ist schlechter als Detectorbox! Reinitalisierung!" << endl;
        // Ansonsten ist natürlich die Trackerbox die nächste, wenn sie was gefunden hat, was ähnlich genug ist
        } else if (t_sim_r >= detector.nearestnCF.thresholdNeg) {
            // Falls beide was gefunden habe, versuche mal das Mittel aus beiden Boxen
            if (dBoxes.size() == 1) {
                fbb.x = (tbb.x + dBoxes[0].x) / 2;
                fbb.y = (tbb.y + dBoxes[0].y) / 2;
                fbb.width = (tbb.width + dBoxes[0].width) / 2;
                fbb.height = (tbb.height + dBoxes[0].height) / 2;

            } else fbb = tbb;
            //cout << "sig_pos: " << isSigPos << " t_sim_r:" << t_sim_r << " t_sim_c:" << t_sim_c << " wasValide=" << wasValide << " MARGIN:" << (t_sim_r - detector.nearestnCF.thresholdPos) << endl;
            // und es muss über das Lernen entschieden werden.
            // Gelernt wird nur, wenn
            // die gefundene Box des Trackers ähnlicher ist als der positive Threshold (.65)
            // oder das Ergebnis davor valide war und die Box des Trackers ähnlicher als negativer Threshold war (.5)
            if (t_sim_r >= detector.nearestnCF.thresholdPos /* && t_sim_r < .75*/) isValide = true; // Entweder der Tracker ist im Core gelandet, also wird er valide
            else if (wasValide && t_sim_r >= detector.nearestnCF.thresholdNeg /* && t_sim_r < .75*/) isValide = true; // oder der Tracker war valide und ist es weiterhin
            // Merken der aktuellen similarity > Eigentlich nur für die Ausgabe
            detector.nearestnCF.sim = t_sim_r;
            detector.nearestnCF.sim_t = t_sim_r;
            success = true;
        }

    } else if (dBoxes.size() == 1) {
        // Hier muss eigentlich noch überprüt werden, ob das Gefundene auch ok ist ;)
        fbb = dBoxes[0];
        detector.nearestnCF.sim = d_sim_r;
        success = true;
    } else {
        success = false;
    }
    return success;
}

/**
 * Trainieren der Classifier
 * r: aktuelle BoundingBox
 * s: aktuelles Ausgabebild > Nur für Analysezwecke (z.B. Anzeige der gewählten positiven Beispiele)
 */
void TLD::learn(Rect& bb, Mat& s)
{
    //cout << "START LEARNING!" << endl;
    Mat patch;
    Utility::generateNormalizedPatch(lastFrame(bb),patch);
    Result* result = detector.resultSet;
    // Überlappungen berechnen!
    float *overlap = new float[detector.gridSize];
    Utility::calcOverlap(detector.grid,detector.gridSize,bb,overlap);
    //Add all bounding boxes with high overlap

    vector< pair<int, float> > positiveIndices;
    vector<int> negativeIndices;
    vector<int> negativeIndicesNNC;
    //First: Find overlapping positive and negative patches

    for(int i = 0; i < detector.gridSize; ++i)
    {

        if(overlap[i] > 0.6)
        {
            positiveIndices.push_back(pair<int, float>(i, overlap[i]));
        }

        if(overlap[i] < 0.2)
        {
            //float variance = result->variances[i];
            // Nur die nehmen, die nicht zu schlecht sind!
            //if(variance > detector.varianceCF.variance)
            if (result->posteriors[i] > .1)
            {
                negativeIndices.push_back(i);
            }

            if (result->posteriors[i] >= .5)
            {
                negativeIndicesNNC.push_back(i);
            }
        }
    }

    vector<Mat> patches;

    patches.push_back(patch); //Add first patch to patch list
    detector.nearestnCF.learn(patches,true);/// HIER NNC-Lernen

    // EC - NEGATIVE BEISPIELE LERNEN!
    int index = 0;
    for(size_t i = 0; i < negativeIndices.size(); i++)
    {
        index = negativeIndices.at(i);
        detector.ensembleCF.learn(index,result,false);
    }

    sort(positiveIndices.begin(), positiveIndices.end(), Utility::compareOverlap);
    int numberOfPositives = std::min<size_t>(positiveIndices.size(), 10); //Take at most 10 bounding boxes (sorted by overlap)
    // Hier wird der EnsembleClassifier trainiert.
    /* Für den PatchGenerator muss erst die Hülle über alle positiven Boxen generiert werden.
        Danach kann das Bild verändert und als Quelle für neue Versionen genutzt werden.
    */
    // Berechne Hülle (für das Erzeugen der synthetischen Beispiele)
    Rect hull;
    Utility::calcHull(detector.grid,positiveIndices,hull);
    Point2f center = Point2f(hull.width/2, hull.height/2); center = center;
    Mat gaussF; // Gaussian Version des patches
    GaussianBlur(lastFrame,gaussF,Size(9,9),1.5);
    Mat clone = lastFrame.clone(); // Bild muss geklont werden, weil es verändert wird
    Mat roi = clone(hull); // Das ist der Teil, der geändert wird
    RNG& random = theRNG(); random = random;// Der Patchgenerator braucht einen Zufallszahlengenerator
    Rect rect;
    // EC - POSITIVE BEISPIELE LERNEN
    for (int j = 0; j < 1; ++j) { /// WARPING AUS! > HAT EIGENTLICH KEINEN EINFLUSS AUF DIE LAUFZEIT
        /// An dieser Stelle werden für jeden positiven Patch 20 warped-Versionen erzeugt
        patchGenerator(gaussF,center,roi,hull.size(),random);
        for(int i = 0; i < numberOfPositives; i++)
        {
            index = positiveIndices.at(i).first;
            // Fern-Vector für die aktuelle BB und den generierten Patch berechnen (das Rechteck wird innerhalb der Funktion ausgeschnitten)
            /// detector.ensembleCF.calculateFernFeatureVector(index,detector.grid,gaussF,&result->featureVectors[detector.ensembleCF.getNumberOfFerns() * index]);
            detector.ensembleCF.calculateFernFeatureVector(index,detector.grid,&gaussF,&result->featureVectors[detector.ensembleCF.getNumberOfFeatures() * index]);
            detector.ensembleCF.learn(index,result,true);
                /// TEST!! SYNTHETISCHE NNC
                //rect = Rect(detector.grid[index * 4], detector.grid[index * 4 + 1],detector.grid[index * 4 + 2],detector.grid[index * 4 + 3]);
                //patch = gaussF(hull);
                //patches.push_back(patch);
        }

    }
    /// TEST!!!!
    //detector.nearestnCF.learn(patches,true);/// HIER NNC-Lernen
    //srand(1); //TODO: This is not guaranteed to affect random_shuffle

    //random_shuffle(negativeIndices.begin(), negativeIndices.end());
    patches.clear(); //Den positiven Patch vergessen!
    //NNC - Lernen negativer Beispiele
    for(size_t i = 0; i < negativeIndicesNNC.size(); ++i)
    {
        index = negativeIndicesNNC.at(i);
        // PATCH BERECHNEN
        rect = Rect(detector.grid[index * 4], detector.grid[index * 4 + 1],detector.grid[index * 4 + 2],detector.grid[index * 4 + 3]);
        patch = lastFrame(rect);
        patches.push_back(patch);
    }

    /// Negative NNC lernen (TODO)
    //detectorCascade->nnClassifier->learn(patches);
    detector.nearestnCF.learn(patches,false);
    delete[] overlap;
    result->reset();
    //cout << "LEARNING DONE!" << endl;
}

/**
 * Je nach Wunsch (durch Tasteneingabe) werden verschiedene Ausgabefenster generiert...
 */
void TLD::generateWindows(Mat& show, int x, int y, Scalar col)
{
    Scalar red = Scalar(0,0,255), yellow = Scalar(0,255,255), blue = Scalar(255,255,0) ;

    /// Anzeige der positiven Beispiele des NCC
    if (showNNCPos) {
        Mat result_pos = Mat::zeros(500,500,show.type());
        Utility::generateMultipleImages(detector.nearestnCF.getPositives(),result_pos,500,500,30,30);
        imshow("PositiveNNC",result_pos);
    }
    if (showNNCNeg) {
        Mat result_neg = Mat::zeros(500,500,show.type());
        Utility::generateMultipleImages(detector.nearestnCF.getNegatives(),result_neg,500,500,30,30);
        imshow("NegativeNNC",result_neg);
    }

    // Ausgabe des Lernstatus
    if (!doLearn) col = red;
    else if (wasValide && doLearn) col = yellow;

    //rectangle(show,cvPoint(x,y),cvPoint(x + 5,y + 5),col,3);

    // Ausgabe der Distanz des aktuellen Bildes
    if (true) {
        /// aktuelle Similarity
        char buffer[20];
        sprintf(buffer,"SIM  : %.2f",detector.nearestnCF.sim);
        putText(show, buffer, cvPoint(x + 5,y), CV_FONT_HERSHEY_DUPLEX, 0.3, col);
        /// vom Tracker
        sprintf(buffer,"SIM_T: %.2f",detector.nearestnCF.sim_t);
        putText(show, buffer, cvPoint(x + 5,y + 10), CV_FONT_HERSHEY_DUPLEX, 0.3, blue);
        /// vom Detector
        sprintf(buffer,"SIM_D: %.2f",detector.nearestnCF.sim_d);
        putText(show, buffer, cvPoint(x + 5,y + 20), CV_FONT_HERSHEY_DUPLEX, 0.3, blue);
    }

}
