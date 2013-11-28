#include "../include/Detector.h"
#include <time.h>

Detector::Detector()
{
    cout << "Const. Detector" << endl;
    isInit = false;
    gridSize = 0;
    //ctor
}

Detector::~Detector()
{
    //dtor
}

/**
 * Initialisierung aller Komponenten.
 * offset: Scalierung des Suchraums (offset = 0.5)
 */
void Detector::init(Rect& bb,int maxW, int maxH)
{
    // Falls noch nicht initialisiert, dann initialisiere. Ansonsten springe raus!
    //if (!isInit) isInit = true;
    //else return;
    showDetector = false;
    useFerns = true;
    useNNC = true;
//    objectModel = ObjectModel();
    //Die Classifier
    varianceCF = VarianceClassifier();
    ensembleCF = EnsembleClassifier();
    nearestnCF = NearestNeighbourClassifier();
    //Ereugen des Grids nach Vorgabe der ersten BoundingBox bb
    //objectModel.generateGrid(bb,offset);
    /// NEU
    generateGrid(bb,maxW,maxH);
    // resultSet initialisieren
    resultSet = new Result();
    resultSet->variances = new float[gridSize];
    resultSet->posteriors = new float[gridSize];
    resultSet->featureVectors = new int[gridSize * ensembleCF.getNumberOfFerns()];

    resultSet->validResult = false;

    //Initialisieren des EnsembleClassifiers (Hier werden die Ferns erzeugt)
    ensembleCF.init(grid,gridSize);
    cout << "Gridsize: " << gridSize << endl;

}

/**
 * Neue Version mittels int*
 */
void Detector::generateGrid(Rect& bb, int maxW = 640, int maxH = 480)
{
    int w = bb.width;
    int h = bb.height;
    //Nur halbe Höhe und Breite (AUF JEDEN FALL LIEGT HIER EIN FEHLER!!!)
    float shift = 0.1; //Mutlitplikator für die Schrittweite im Bild
    float s_factor = 1.2; //Skalierungsfaktor
    int a = -10; //Der Esponent > Quasi die äußere Schleife von -10 bis +10
    int minWidth = 20;
    maxW == 320 ? minWidth = 20 : minWidth = 40;
    int minHeight = 20; //Min-Max-Werte
    maxH == 320 ? minHeight = 20 : minHeight = 40;
    float s; //Die berechnete Skalierung s_factor^a
    float temp = 0; //eine temporäre Variables zum Zwischenspeichern der Skalierung, bevor auf- bzw. abgerundet wird.
    int width = 0; //die berechnete breite einer erzeugten Box
    int height = 0; //die berechnete Höhe einer erzeugten Box
    int min_side = 0; //Um über das Bild zu laufen wird die minimale Seitelänge ermittelt (entweder width oder height)
    int id = 0; id = id;
    Rect r;
    int* rect = new int[4];
    vector<int*> windowGrid;
    cout << "Detector::generateGrid" << endl;
    for (a = -10; a <= 10; ++a)
    {
        s = pow(s_factor,a);
        temp = s * w;
        //Auf- bzw. Abrunden
        width = (int)(temp * 10) % 10 < 5 ? floor(temp) : ceil(temp);
        temp = s * h;
        //Auf- bzw. Abrunden
        height = (int)(temp * 10) % 10 < 5 ? floor(temp) : ceil(temp);
        //Falls sie zu kleine sind, dann mach nich weiter, sondern berechne neue Box
        if (width < minWidth || height < minHeight) continue;
        //Falls zu groß, berechne auch neu
        if (width > maxW || height > maxH) continue;
        //Nun wird für jede mögliche Position eine Box erzeugt > also x und y werden gesetzt
        //dazu muss min_side ermittelt werden > Quasi die Schrittweite
        min_side = min(width,height);
        temp = min_side * shift;
        //Wieder auf bzw. abrunden
        temp = (int)(temp * 10) % 10 < 5 ? floor(temp) : ceil(temp);
        for (int y = 0; y < maxH - height; y += temp)
        {
            for (int x = 0; x < maxW - width; x += temp)
            {
                rect = new int[4];
                rect[0] = x;
                rect[1] = y;
                rect[2] = width;
                rect[3] = height;
                windowGrid.push_back(rect);
                this->gridSize++;
            }
        }
    }
    /// NUN WIRD DAS EIGENTLICHE GRID INITIALISIERT
    int realSizeOfGrid = this->gridSize * 4;
    grid = new int[realSizeOfGrid];
    // Und die Werte werden gespeichert
    for (int i = 0; i < gridSize; ++i) {
        rect = windowGrid[i];
        grid[i * 4] = rect[0];
        grid[i * 4 + 1] = rect[1];
        grid[i * 4 + 2] = rect[2];
        grid[i * 4 + 3] = rect[3];
    }

    cout << " Anzahl Fenster: " << this->gridSize << endl;

}

/**
 * Sucht das Objekt und generiert eine entsprechend BoundingBox.
 * Gibt false zurück, wenn kein Objekt gefunden wurde. True sonst.
 *
 * NEUE IDEE: Damit das Lernen schneller geht, werden hier bereits die
 *  negativen Beispiele für den EnsembledClassifier und den NearestNeigbourClassifier
 *  generiert.
 */
bool Detector::detect(Mat& f2, vector<Rect>& possibleBoxes)
{
    resultSet->validResult = false;
    /// NEU > ZEITMESSUNG
    time_t start_t, var_t, esc_t, nnc_t;
    time(&start_t);
    //cout << "Detector: Start detect....";
    //Nur für die Ausgabe, falls gewollt
    Mat dMat;
    vector<Rect> positiveBB;
    if (showDetector) {
        dMat = f2.clone();
    }
    //Iteriere über das gesamte Grid und schicke jedes mögliche Objekt durch die Kaskade
    int i;
    /// ResultSet hat alle möglichen Boxen und deren Werte (varianz, confidence, templateMatch (TODO) )
    resultSet->reset();
    //vector<Object> positives = *objectModel.getPositives();
//    Object* object;
    Mat patch;
    Mat gaussF;
    GaussianBlur(f2,gaussF,Size(9,9),1.5);
    // Erzeugen des Integralen Bildes zur Bestimmung der Varianz
    varianceCF.calcIntegral(f2);
    Rect r;

    int rest = gridSize;
    //detectorState = false;
    for (i = 0; i < gridSize; ++i)
    {
        /// Varianzfilter OK
        if (!varianceCF.classify(i,grid, resultSet)) {
            resultSet->posteriors[i] = 0;
            rest--;
            continue; // Die Box ist raus, ansonsten weiter...
        }
        time(&var_t);
        //cout << "\tVor EC" << endl;

        /// EnsembleClassifier OK (Wahrscheinlich)
        //if (!useFerns || !ensembleCF.classify(i,resultSet,grid,f2)) {
        if (!ensembleCF.classify(i,resultSet,grid,&f2)) {
            rest--;
            continue; // Die Box ist raus, ansonsten weiter...
        }
        time(&esc_t);
        /// TESTTEST
        //nearestnCF.classifyViaTrack(i,resultSet,grid,f2);
        //cout << "\tNach EC" << endl;
        /// NearestNeigbour OK
        //if (!(useFerns && useNNC) || !nearestnCF.classify(i,resultSet,grid,f2)) {
        if (!nearestnCF.classify(i,resultSet,grid,f2)) {
            rest--;
            continue;
        }

        time(&nnc_t);

        /// Wenn bis hierhier durchgekommen, dann merke als mögliche Box
        positiveBB.push_back(Rect(grid[4*i],grid[4*i + 1],grid[4*i + 2],grid[4*i + 3]));
    }
    /// ZEITERFASSUNG:
//    cout << "Time_VAR:" << difftime(start_t,var_t) << " Time_ESC:" << difftime(start_t,esc_t) << " Time_NNC:" << difftime(start_t,esc_t) << endl;

    //cout << "Restboxen nach EC:" << rest << endl;
    resultSet->validResult = true;

    // Clustern der Boxen (Nur wenn auch durch Fern gefiltert)
    if (useFerns && positiveBB.size() > 0) clusterBoxes(positiveBB,possibleBoxes);
    // Ausgabe, wenn aktiviert (schwarz = posDetection, weiß = cluster und damit mögliche Box)
    if (showDetector)
    {
        for (i = 0; i < (int)positiveBB.size(); ++i)
        {
            Rect rect = positiveBB.at(i);
            rectangle(dMat ,cvPoint(rect.x,rect.y),cvPoint(rect.x + rect.width, rect.y + rect.height),Scalar(0,0,0),1);
        }
        //rectangle(dMat ,cvPoint(dbb.x,dbb.y),cvPoint(dbb.x + dbb.width, dbb.y + dbb.height),Scalar(255,255,255),1);
        ///HIER DIE CLUSTER AUSGEBEN
        for (i = 0; i < (int) possibleBoxes.size(); ++i) rectangle(dMat ,cvPoint(possibleBoxes[i].x, possibleBoxes[i].y),cvPoint(possibleBoxes[i].x + possibleBoxes[i].width, possibleBoxes[i].y + possibleBoxes[i].height), Scalar(255,255,255),1);
        // Bild zeichnen
        imshow("Detector",  dMat);

    }

    return true;
    /*
    //cout << " ...DONE!" << endl;
    detectorState = possibleBoxes.size() > 0;
    return possibleBoxes.size() > 0;
    */

}

/**
 * Erzeugt cluster aus den positiven Boxen.
 *
 * Als Kriterium dient der Grad der Überlappung (und confidence... vielleicht).
 * Ein Cluster wird so lange erweitert, solange eine Box einen overlap-Wert von mindeste 0.6 hat
 */
void Detector::clusterBoxes(vector<Rect>& positives, vector<Rect>& cluster)
{
    vector<Rect> tempCluster;
    vector<int> temp ;
    int numberOfPositives = positives.size();
    size_t i = 0;
    int classes = 0;
    Rect r, r1,r2;
    // alle Rects aus dem Grid ins Cluster übertragen,
    // sonst ist der Vergleich nicht möglich
    for (i = 0; i < positives.size(); ++i) tempCluster.push_back(positives[i]);
    //Iteriere über alle positiven Boxen und Berechne die Überlappungen
    //Ermittle Anzahl der gefundenen Boxen
    if (numberOfPositives == 1) cluster.push_back(positives[0]);
    else if (numberOfPositives == 2) {
        temp =vector<int>(2,0);
        r1 = positives[0];
        r2 = positives[1];
        //Wie ähnlich sind sich die Objekte?
        if (1 - Utility::calcOverlap(r1,r2) > 0.6){
            temp[1] = 1;
            classes = 2;
        }
    } else {
        temp = vector<int>(numberOfPositives,0);
        classes = partition(tempCluster,temp,(*Utility::compareBoxes));
    }
    // temp[i] = c >> der Vector temp enthält an der Stelle i die Klasse c für das i-te Objekt
    //cout << "Anzahl Classen: " << classes << endl;
    // hier wird für jede Classe die "mittlere" Box bestimmt!
    for (int i = 0; i < classes; ++i){
        //float cnf = 0;
        int counter = 0,mx = 0,my = 0,mw = 0,mh = 0;
        for (int j = 0; j < (int)temp.size(); ++j)
        {
            //cout << "temp[" << j << "]:" << temp[i] << endl;

            if (temp[j] == i){
                r = positives[j];
                //cnf = cnf + detectorConf[j];
                mx += r.x;
                my += r.y;
                mw += r.width;
                mh += r.height;
                ++counter;
            }
        }
        if (counter > 0){
            r = Rect();
            //o = Object();
            //clusterConf[i] = cnf / N;
            r.x = cvRound(mx/counter);
            r.y = cvRound(my/counter);
            r.width = cvRound(mw/counter);
            r.height = cvRound(mh/counter);
            //cout << "ClusterBox: " << r.x << "," << r.y << "," << r.width << "," << r.height << endl;
            //o.setRect(r);
            //clusterObj[i] = o;
            cluster.push_back(r);
        }
    }
}
