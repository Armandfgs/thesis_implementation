#ifndef NEARESTNEIGHBOURCLASSIFIER_H
#define NEARESTNEIGHBOURCLASSIFIER_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

#include "Result.h"
#include "Utility.h"
#include "Tracker.h"


using namespace std;
using namespace cv;

class NearestNeighbourClassifier
{
    public:
        NearestNeighbourClassifier();
        virtual ~NearestNeighbourClassifier();
        bool classify(int, Result*, int*, Mat&);
        float classify(Mat&);
        bool classifyViaTrack(int,Result*,int*,Mat&);
        void learn(vector<Mat>&,bool);
        void learn(Mat&,bool);
        void calculateSimilarity(Mat&, float&, float&, bool&, bool&);
        vector<Mat>& getPositives() { return positiveExamples; }
        vector<Mat>& getNegatives() { return negativeExamples; }
        float thresholdPos; // Threshold .65
        float thresholdNeg; // Threshold .5
        float similarity; // .95;
        float sim; // Aktuelle similarity
        float sim_t; // similarity des Trackers
        float sim_d; // similatirty des Detectors
    protected:
    private:
        vector<Mat> positiveExamples;
        vector<Mat> negativeExamples;
        float ncc(uchar*,uchar*);
        /// Test-Tracker
        Tracker* tracker;
        /* float s_pos; // maximale Ähnlichkeit zu den positiven Beispielen
        float s_neg; // maximale Ähnlichkeit zu den negativen Beispielen
        float s_pos_50; // Ähnlichkeit zu den ersten 50% pos. Beispielen
        float s_r; // relative Ähnlichkeit
        float s_c; // konservative Ähnlichkeit*/
};

#endif // NEARESTNEIGHBOURCLASSIFIER_H
