#ifndef ENSEMBLECLASSIFIER_H
#define ENSEMBLECLASSIFIER_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

#include "FernFeature.h"
#include "Object.h"
#include "Result.h"

using namespace std;
using namespace cv;

class EnsembleClassifier
{
    public:
        EnsembleClassifier();
        virtual ~EnsembleClassifier();
        void init(int*, int&); /// NEU
        /// NEU
        void calculateFernFeatureVector(int,int*,Mat*,int*);
        bool classify(int,Result*,int*,Mat*);
        float calculateConfidence(int* ferns) { return calculateConfidence(ferns, false); }
        float calculateConfidence(int*,bool);
        void learn(int,Result*,bool);
        void updatePosteriors(int*,bool);

        int getNumberOfFerns() { return numberOfFerns; }
        int getNumberOfFeatures() { return numberOfFeatures; }
        float getThreshold() { return threshold; }

    protected:
    private:
        //Anzahl der Features (also Fern-Gruppen)
        int numberOfFeatures;
        //Anzahl der Ferns pro Gruppe
        int numberOfFerns;
        //Anzahl der Gesamtfeatures und damit auch Anzahl der Indizes der Posteriors
        int numberOfIndices;
        //Vector mit den FernFeatures für die Klassfizierung
        //vector< vector<FernFeature> > features;
        /* A-Priorie-Wahrsceinlichkeiten */
        //vector<vector<float> > posteriors;  /// ALT
        /* Pos. und neg. Zähler */
        //vector<vector<int> > negCounter;    /// ALT
        //vector<vector<int> > posCounter;    /// ALT

        float* posteriorsf;
        int* negCounteri;
        int* posCounteri;
        int* featuresi;
        //Tresholds
        float threshold;
        float thresholdPositive;
        float thresholdNegative;

};

#endif // ENSEMBLECLASSIFIER_H
