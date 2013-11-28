#include "../include/EnsembleClassifier.h"

EnsembleClassifier::EnsembleClassifier()
{
    //Anzahl der Features pro Fern
    numberOfFeatures = 13;
    //Anzahl der Ferns insgesamt
    numberOfFerns = 10;
    // Threshold
    threshold = .5;
    thresholdPositive = .5;
    thresholdNegative = .5;
    // Anzahl der gesamtFeatures und damit auch Indizes der Posteriors: 2^numberOfFeatures (2^13 ~ 8.200)
    numberOfIndices = pow(2.0f, numberOfFeatures);
    //ctor
}

EnsembleClassifier::~EnsembleClassifier()
{
    //dtor
}

/**
 * Initialisierung des EnsembleClassifier.
 * Neben der Generierung der einzelnen Ferns durch zufällige Punkte
 * werden auch die vectoren für das Zählen (und spätere Lernen) der
 * positiven und negativen Beispiele initialisiert.
 */
/*void EnsembleClassifier::init(vector<Object>& objects)
{
    cout << "Initialisiere EnsembleClassifier" << endl;

    int nfeatures = numberOfFeatures * numberOfFerns;
    features = vector<vector<FernFeature> >(objects.size(),vector<FernFeature> (nfeatures));
    //Zufallsgenerator
    RNG& rng = theRNG();
    float x1f,x2f,y1f,y2f;
    int x1, x2, y1, y2, i, s, counter = 0;

    for (i = 0; i < nfeatures; ++i){
        x1f = (float)rng;
        y1f = (float)rng;
        x2f = (float)rng;
        y2f = (float)rng;
        //Die erzeugten Punkte werden nun für jedes Objekt im Grid als Features definiert, damit sie auch zu den jeweiligen Skapielrungen passen
        for (s = 0; s < (int)objects.size(); ++s){
            x1 = x1f * objects[s].getRect().width;// - objects[s].getRect().x;
            y1 = y1f * objects[s].getRect().height;// - objects[s].getRect().y;
            x2 = x2f * objects[s].getRect().width;// - objects[s].getRect().x;
            y2 = y2f * objects[s].getRect().height;// - objects[s].getRect().y;
///            features[s][i] = FernFeature(x1, y1, x2, y2);
            /// NUR ZUM TEST
            //objects[s].ferns[i] = features[s][i];
            counter++;
        }

    }
    //Threshold
    thresholdNegative = 0.5 * numberOfFerns;

    // Erzeuge den WSK- und die Zähler-Vectoren
    for (i = 0; i<numberOfFerns; ++i) {
        posteriors.push_back(vector<float>(numberOfIndices, 0));
        posCounter.push_back(vector<int>(numberOfIndices, 0));
        negCounter.push_back(vector<int>(numberOfIndices, 0));
    }


}
*/
/**
 * Initialisierung des EnsembleClassifier.
 * Neben der Generierung der einzelnen Ferns durch zufällige Punkte
 * werden auch die vectoren für das Zählen (und spätere Lernen) der
 * positiven und negativen Beispiele initialisiert.
 */
void EnsembleClassifier::init(int* grid, int& gridSize)
{
    //gridSize ist die Anzahl der Fenster. Die Größe des grids ist allerdings 4 mal so groß
    cout << "Initialisiere EnsembleClassifier" << endl;
    /**
     Erläuterung: Die Idee ist, dass für JEDES Objekt im Grid, alle Fern-Gruppierungen erzeugt werden.
     D.h, dass das Objekt an der Stelle s, alle i Features "zugewiesen" bekommt. Somit gibt es für jedes Objekt
     quasi einen kompletten Satz Features, anstelle von einem Satz Features für Alle Objekte.
     */
    int nfeatures = numberOfFeatures * numberOfFerns;
    /// NEU
    int sizeOfFeatures = gridSize * nfeatures * 4; // für jedes Window werden für jedes Features eines Ferns 4 Werte gespeichert
    featuresi = new int[sizeOfFeatures];

///    features = vector<vector<FernFeature> >(gridSize,vector<FernFeature> (nfeatures)); /// ALT
    //Zufallsgenerator
    RNG& rng = theRNG();
    float x1f,x2f,y1f,y2f;
    int i, s, counter = 0, index = 0; /// DER INDEX FÜR DIE ADDRESSIERUNG DER FEATURES

    for (i = 0; i < nfeatures; ++i){
        x1f = (float)rng;
        y1f = (float)rng;
        x2f = (float)rng;
        y2f = (float)rng;
        //Die erzeugten Punkte werden nun für jedes Objekt im Grid als Features definiert, damit sie auch zu den jeweiligen Skapielrungen passen
        for (s = 0; s < gridSize; ++s){
            /*x1 = x1f * grid[s * 4 + 2];
            y1 = y1f * grid[s * 4 + 3];
            x2 = x2f * grid[s * 4 + 2];
            y2 = y2f * grid[s * 4 + 3];*/
            //features[s][i] = FernFeature(x1, y1, x2, y2);
            index = (4 * numberOfFerns * numberOfFeatures * s) + (4 * i);

            featuresi[index]     = x1f * grid[s * 4 + 2];
            featuresi[index + 1] = y1f * grid[s * 4 + 3];
            featuresi[index + 2] = x2f * grid[s * 4 + 2];
            featuresi[index + 3] = y2f * grid[s * 4 + 3];
            /// NUR ZUM TEST
            //objects[s].ferns[i] = features[s][i];
            counter++;
        }

    }
    //Threshold
    thresholdNegative = 0.5 * numberOfFerns;
    cout << "FEATURES:" << counter << endl;
    posteriorsf = new float[numberOfFerns * numberOfIndices];
    negCounteri = new int[numberOfFerns * numberOfIndices];
    posCounteri = new int[numberOfFerns * numberOfIndices];

    for(int i = 0; i < numberOfFerns; i++)
    {
        for(int j = 0; j < numberOfIndices; j++)
        {
            posteriorsf[i * numberOfIndices + j] = 0;
            negCounteri[i * numberOfIndices + j] = 0;
            posCounteri[i * numberOfIndices + j] = 0;
        }
    }
}

bool EnsembleClassifier::classify(int index, Result* resultSet, int* grid, Mat* f)
{
    int* ferns = resultSet->featureVectors + numberOfFerns * index;
    calculateFernFeatureVector(index,grid,f,ferns);

    resultSet->posteriors[index] = calculateConfidence(ferns);
    //if (resultSet->posteriors[index] > 0) cout << "classifiy Index: " << index << " " << resultSet->posteriors[index] << endl;
    return resultSet->posteriors[index] > .5;
}

/**
 * Erzeugt den FeatureVector mit den zusammengerechneten Werten der einzelnen FernFeatures
 */
void EnsembleClassifier::calculateFernFeatureVector(int index, int* data, Mat* frame, int* ferns)
{
    int fernNumber = 0;
    int featureNumber = 0;
    int fernValue = 0;
    int* winIndex = data + index * 4; //Adresse des Windows im Grid
    //int* featureIndex = featuresi + (4 * numberOfFerns * numberOfFeatures * index);
    /// cout << index << ": " << winIndex[0] << "," << winIndex[1] << endl;
    //Mat roi = frame(Rect(winIndex[0],winIndex[1],winIndex[2], winIndex[3]));
    int* feature = 0; feature = feature;

    for (fernNumber = 0; fernNumber < numberOfFerns; ++fernNumber) {
        fernValue = 0;
        //cout << "Fern[" << fernNumber << "] = ";
        for (featureNumber = 0; featureNumber < numberOfFeatures; ++featureNumber) {
            fernValue <<= 1;
            feature = featuresi + (4 * numberOfFerns * numberOfFeatures * index)  + ( fernNumber * numberOfFeatures * 4 ) + featureNumber * 4; /// NEU > Zeiger weiter schieben
            /// feature = featuresi + (4 * numberOfFerns * numberOfFeatures * index)  + featureNumber * 4; /// NEU > Zeiger weiter schieben
            //cout << index << ":" << ((4 * numberOfFerns * numberOfFeatures * index) + ( fernNumber * numberOfFeatures * 4 ) + featureNumber * 4);
            //cout << " > " << feature[0] << " " << feature[1] << " " << feature[2] << " " << feature[3] << " " << endl;
            // Vergleich der beiden Features
            /// Es muss sichergestellt werden, dass wir auch immernoch im Bild sind
            if (winIndex[0] + feature[0] + 1 > frame->cols) continue;
            if (winIndex[1] + feature[1] + 1 > frame->rows) continue;
            if (winIndex[0] + feature[3] + 1 > frame->cols) continue;
            if (winIndex[1] + feature[4] + 1 > frame->rows) continue;
            /// if (roi.at<unsigned char>(feature[0],feature[1]) > roi.at<unsigned char>(feature[2],feature[3])) fernValue |= 1;
            if (frame->at<unsigned char>(winIndex[0] + feature[0],winIndex[1] + feature[1]) >
                 frame->at<unsigned char>(winIndex[0] + feature[2],winIndex[1] + feature[3])) fernValue |= 1;
            //cout << (roi.at<unsigned char>(feature[0],feature[1]) > roi.at<unsigned char>(feature[2],feature[3]));
        }
        //cout << endl;
        //Speichern der berechneten Zahl als Index für den Posterior
        ferns[fernNumber] = fernValue;
    }
}

/**
 * Diese Funktion updated die Posteriors.
 * Außerdem müssen die positiven und negativen Bewertungen gesetzt werden.
 *
 * featureVector: Vector mit den Bewertungen für alle Ferns, also 0000000000 - 1111111111
 * isPositive: true, falls diese Bewertung positiv ist, false sonst
 */
void EnsembleClassifier::updatePosteriors(int* ferns, bool isPositive)
{
    //cout << "EnsembleClassifier::updatePosteriors(vector,bool)" << endl;
    // Das Bewerten geht folgendermaßen:
    //- an der Stelle posteriors[fernNumber][featureValue] (z.B. posteriors[3][322]) wird gespeichert, wie ein einzelnen Fern den Patch bewertet hat
    //- dazu wird einfach das Verhältnis von positiven Bewertungen zu allen Bewertungen (pos. und neg.) für ein Fern gespeichert
    //- beim classifizieren eine Patches, also beim Bewerten, wird dann an der Stelle fernNumber,featureValue (also z.B. 3, 322)
    //    der Verhältniswert von pos zu negativen Zählern gefunden.

    int fernNumber = 0;
    int arrayIndex = 0;
    //cout << "updatePosterior: Ferns " << ferns << endl;
    for (fernNumber = 0; fernNumber < numberOfFerns; ++fernNumber)
    {
        // Der Wert des Ferns mit der Nummer fernNumber
        // Nun wird der posCounter für den Fern und dem wert für diesen Fern (z.B. fern[3][3423]) um eins erzöht,
        // wenn dieser Wert "positiv" angesehen werden soll, ansonsten "negativ" und damit negCounter.
        /// ALT isPositive ? posCounter[fernNumber][featureValue]++ : negCounter[fernNumber][featureValue]++;
        // cout << "+:" << posCounter[fernNumber][featureValue] << " -:" << negCounter[fernNumber][featureValue];
        //if (posCounter[fernNumber][featureValue] == 0) posteriors[fernNumber][featureValue] = 0;
        //else
        /// ALT posteriors[fernNumber][featureValue] = ((float) posCounter[fernNumber][featureValue]) / (posCounter[fernNumber][featureValue] + negCounter[fernNumber][featureValue]);
        // cout << " p:" << posteriors[fernNumber][featureValue] << endl;
        //cout << "Fern[" << fernNumber << "]: " << ferns[fernNumber] << endl;
        arrayIndex = fernNumber * numberOfIndices + ferns[fernNumber];
        (isPositive) ? posCounteri[arrayIndex]++ : negCounteri[arrayIndex]++;
        // Falls es keine positiven Beispiele gibt, wird immer mit 0 bewertet
        if (posCounteri[arrayIndex] == 0) posteriorsf[arrayIndex] = 0;
        else posteriorsf[arrayIndex] = ((float) posCounteri[arrayIndex]) / (posCounteri[arrayIndex] + negCounteri[arrayIndex]);
        //cout << "Posterior[" << arrayIndex << "]: " << posteriorsf[arrayIndex] << endl;
    }


}

/**
 * Berechnung des confidence-Wertes
 *
 * Das wird über alle Posteriors iteriert und der fern bewertet. Der
 * Vector wird für jeden Patch durch calculateFernFeatureVector erzeugt.
 */
float EnsembleClassifier::calculateConfidence(int* fern, bool show)
{
    //if (show) cout << "Adresse:" << fern << endl;
    float confValue = 0.0;
    int fernNumber = 0; //Index für den Fern-Wert im FeatureVector
    for (fernNumber = 0; fernNumber < numberOfFerns; ++fernNumber) {
        //confValue += posteriors[fernNumber][fern[fernNumber]];
        confValue += posteriorsf[fernNumber * numberOfIndices + fern[fernNumber]];
    }
    /// Hier muss der Durschnitt des conf-Values aller Ferns gebildet werden
    return confValue / numberOfFerns;
}

/**
 * Diese Funktion trainiert den EnsembleClassifier.
 *
 * ferns: vector mit den Fern-Values für einen Patch.
 * isPositiv: letzte Bewertung, true, wenn positiv
 *
 */
//void EnsembleClassifier::learn(int index, bool isPositiv, const Mat& f)
void EnsembleClassifier::learn(int index, Result* resultSet, bool isPositive)
{
    //cout << "EC::learn" << endl;
    int* ferns = resultSet->featureVectors + numberOfFerns * index;
    //cout << "Adresse:" << resultSet->featureVectors << " + " << (numberOfFerns * index) << " = " << ferns << endl;
    float conf = calculateConfidence(ferns, true);
    //cout << "learn conf:" << conf << " resultSet.posterior: " << resultSet->posteriors[index] << endl;
    //Es darf nur gelernt werden, wenn die Bewertung isPositive sich von dem confidence-Wert
    //des Classifiers unterschiedet. Also entweder:

            // true && conf < threshold (dann positiv)
            // false && conf > threshold (dann negativ)

    //threshold = .5
    if ((isPositive && conf <= threshold) || (!isPositive && conf >= threshold)) {
        //cout << "UPDATE " << endl;
        updatePosteriors(ferns, isPositive);
    }
}

