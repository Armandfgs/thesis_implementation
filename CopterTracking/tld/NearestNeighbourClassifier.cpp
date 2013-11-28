#include "../include/NearestNeighbourClassifier.h"

NearestNeighbourClassifier::NearestNeighbourClassifier()
{
    thresholdPos = .65;
    thresholdNeg = .5; // Der muss so hoch sein...
    similarity = .95;
    positiveExamples = vector<Mat>();
    negativeExamples = vector<Mat>();
    tracker = new Tracker();
    //ctor
}

NearestNeighbourClassifier::~NearestNeighbourClassifier()
{
    //dtor
}

void NearestNeighbourClassifier::learn(vector<Mat>& patches, bool isPositive)
{
    for(size_t i = 0; i < patches.size(); i++)
    {
        learn(patches[i],isPositive);
    }
}

void NearestNeighbourClassifier::learn(Mat& patch, bool isPositive)
{
    //float s_con = .0; // Konservative Ähnlichkeit
    //bool isSignifikantPos = false;
    //bool isSignifikantNeg = false;
    float s_rel;
    Mat normalizedPatch;
    Utility::generateNormalizedPatch(patch,normalizedPatch);
    s_rel = classify(normalizedPatch);
    // Lerne nur, wenn positiv bewertet und .5 > s_rel <= .65 ist
    if(isPositive && s_rel <= thresholdPos) positiveExamples.push_back(normalizedPatch.clone());
    if (!isPositive && s_rel >= thresholdNeg) negativeExamples.push_back(normalizedPatch.clone());
    // ACHTUNG: WENN ZU VIELE BEISPIELE EXISTIEREN, DANN LÖSCHEN RANDOM
    /*if (negativeExamples.size() > 800) {
        // Mische!
        random_shuffle(negativeExamples.begin(),negativeExamples.end());
        // Verwerfe die Hälfte
        negativeExamples.resize(500);
    }*/

}

bool NearestNeighbourClassifier::classify(int index, Result* result, int* grid, Mat& f)
{
    float s_rel;//, s_con;
    //bool isPositive = false; // Nur Platzhalter. Sagt etwas über die Signifikanz aus, hier jedoch nicht wichtig
    //bool isNegative = false; // Nur Platzhalter. Sagt etwas über die Signifikanz aus, hier jedoch nicht wichtig
    Mat patch = f(Rect(grid[index * 4],grid[index * 4 + 1],grid[index * 4 + 2],grid[index * 4 + 3] ));
    Mat normalizedPatch;
    Utility::generateNormalizedPatch(patch,normalizedPatch);
    //calculateSimilarity(normalizedPatch, s_con, s_rel,isPositive, isNegative);
    s_rel = classify(normalizedPatch);
    //cout << "CLASSIFY: " << s_rel << endl;
    return s_rel >= thresholdPos;
}


float NearestNeighbourClassifier::classify(Mat& patch)
{
    if(positiveExamples.empty())
    {
        return 0;
    }

    if(negativeExamples.empty())
    {
        return 1;
    }

    float ccorr_max_p = 0;

    //Compare patch to positive patches
    for(size_t i = 0; i < positiveExamples.size(); i++)
    {
        float ccorr = ncc(positiveExamples.at(i).data, patch.data);

        if(ccorr > ccorr_max_p)
        {
            ccorr_max_p = ccorr;
        }
    }

    float ccorr_max_n = 0;

    //Compare patch to negative patches
    for(size_t i = 0; i < negativeExamples.size(); i++)
    {
        float ccorr = ncc(negativeExamples.at(i).data, patch.data);

        if(ccorr > ccorr_max_n)
        {
            ccorr_max_n = ccorr;
        }
    }

    float dN = 1 - ccorr_max_n;
    float dP = 1 - ccorr_max_p;

    float distance = dN / (dN + dP);
    return distance;
}


/// TEST
float NearestNeighbourClassifier::ncc(uchar *f1, uchar *f2)
{
    double corr = 0;
    double norm1 = 0;
    double norm2 = 0;

    int size = 15 * 15;

    for(int i = 0; i < size; i++)
    {
        corr += f1[i] * f2[i];
        norm1 += f1[i] * f1[i];
        norm2 += f2[i] * f2[i];
    }

    // normalization to <0,1>

    return (corr / sqrt(norm1 * norm2) + 1) / 2.0;
}

/** NEU: 06.07.2013 > Tracken der Features */
/**
    Das aktuelle Bild (index) wird in allen positiven und negativen Beispielen gesucht (getrackt).


*/
bool NearestNeighbourClassifier::classifyViaTrack(int index, Result* result, int* grid, Mat& f)
{
    // Patch vorbereiten
    Mat patch = f(Rect(grid[index * 4],grid[index * 4 + 1],grid[index * 4 + 2],grid[index * 4 + 3] ));
    Mat normalizedPatch;
    Utility::generateNormalizedPatch(patch,normalizedPatch);
    /// TEST!!! Mal goodFeaturesToTrack ins Bild!
    vector<Point2f> features;
    goodFeaturesToTrack(normalizedPatch,features,500,0.1,10);
    // Ausgabe > ausgeschaltet
    //for (size_t i = 0; i < features.size(); ++i)
    //    cout << features.at(i) << endl;

    Rect b1 = Rect(0,0,15,15), b2;
    size_t i = 0;
    // Über alle pos. Beispiele iterieren
    for(i = 0; i < positiveExamples.size(); i++)
    {
        // Patch "tracken"
        if (tracker->track(patch,positiveExamples.at(i),b1, b1,b2)) cout << "Positive Getracked" << endl;
        else cout << "Positive NICHT getracked" << endl;

    }




    return true;
}

/**
 * Berechnet die Ähnlichkeit (similarity) des Patches:
 * s_max_pos_50, die Ähnlichkeiten des Patches zu den ersten 50% der positiven Beispiele,
 * s_relative = s_pos / (s_pos + s_neg), die relative Ähnlichkeit, und
 * s_conservative = s_pos_50 / (s_pos_50 + s_neg), die konservative Ähnlichkeit berechnet
 *
 * patch: der normalisierte 15 x 15 Patch
 * s_conservative: der berechnete konservative Wert
 * s_relative: der berechnete relative Wert
 * isPositive: true, falls patch signifikant positiv
 * isNegative: true, falls patch signifikant negativ
 *
 */
void NearestNeighbourClassifier::calculateSimilarity(Mat& patch, float& s_conservative, float& s_relative, bool& isPositive, bool& isNegative)
{

    // Falls keine positiven Beispiele existieren, dann ist alles NEGATIV => s_c = 0
    if(positiveExamples.empty())
    {
        //cout << "kein positives Beispiel, also ist alles negativ" << endl;
        s_conservative = 0;
        s_relative = 0;
        isNegative = true;
        isPositive = false;
        return;
    }
    // Falls keine negativen Beispiele existieren, dann ist alles POSITIV = s_c = 1
    if(negativeExamples.empty())
    {
        //cout << "kein negatives Beispiel, also ist alles positiv" << endl;
        s_conservative = 1;
        s_relative = 1;
        isNegative = false;
        isPositive = true;
        return;
    }

    // Ähnlichkeit ist definiert als s = .5(ncc(p,p') + 1), mit ncc = normalized correlation coefficient
    // die Funktion templateMatching() berechnet genau das, je nachdem, welche Funktion gewollte ist.
    // Hier wird immer gleich die konservative Berechnung durchgeführt.
    float sim;
    float sim_max_pos = 0;
    float sim_max_pos_50 = 0;
    float sim_max_neg = 0;
    size_t i = 0;
    int half = positiveExamples.size() / 2;
    //Mat result = Mat(1,1,patch.type());
    //cout << "NNC: PosCount:" << positiveExamples.size() << " NegCount:" << negativeExamples.size() << endl;
    //int matchMethod = CV_TM_CCORR_NORMED; ///0: SQDIFF 1: SQDIFF NORMED 2: TM CCORR 3: TM CCORR NORMED 4: TM COEFF 5: TM COEFF NORMED
    //Iteriere über alle positiven Beispiele
    for(i = 0; i < positiveExamples.size(); i++)
    {
        //matchTemplate(positiveExamples[i],patch,result,matchMethod);
        // Das ist die Formel s = .5(ncc(p,p') + 1)
        //sim = .5 * (((float*)result.data)[0] + 1);
        /// TEST
        sim = .5 * (ncc(positiveExamples[i].data,patch.data ) + 1);
        //cout << " SIM_2:" << sim << endl;
        // wenn eine neue maximale Änlichkeit gefunden wurde, dann merke sie dir
        if (sim > sim_max_pos) {
            sim_max_pos = sim;
            if ((int) i <= half) sim_max_pos_50 = sim;
        }
        // Falls sim größer als similarity, dann ist es ein signifikant positiver Patch!
        if (sim > similarity) {
            isPositive = true; // EGAL
        }
        //imshow("Box",positiveExamples[i]);
    }

    // Nun mit negativen Beispielen vergleichen!
    for(i = 0; i < negativeExamples.size(); i++)
    {
        //matchTemplate(negativeExamples[i],patch,result,matchMethod);
        // Das ist die Formel s = .5(ncc(p,p') + 1)
        //sim = .5 * (((float*)result.data)[0] + 1);
        sim = .5 * (ncc(negativeExamples[i].data,patch.data ) + 1);
        // wenn eine neue maximale Änlichkeit gefunden wurde, dann merke sie dir
        if (sim > sim_max_neg) {
            sim_max_neg = sim;
        }
        // Falls sim größer als similarity, dann ist es ein Signifikant negativer Patch!
        if (sim > similarity) {
            isNegative = true; // EGAL
        }
    }

    //Nun werden die Distanzen berechnet
    float distancePos = 1 - sim_max_pos;
    float distanceNeg = 1 - sim_max_neg;
    //cout << "dist+:" << distancePos << " dist-:" << distanceNeg << endl;

    s_relative =(float)distanceNeg / (distanceNeg + distancePos);

    distancePos = 1 - sim_max_pos_50;
    s_conservative = (float) distanceNeg / (distanceNeg + distancePos);
}

