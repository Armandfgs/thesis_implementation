#ifndef RESULT_H
#define RESULT_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

using namespace std;

class Result
{
    public:
        Result();
        virtual ~Result();
        bool validResult; // true, falls es mindestens eine Box durch den Detector geschafft hat
        float *variances; // die Varianz für die Box i

        float *posteriors;  /* Contains the posteriors for each slding window. Is of size numWindows. Allocated by tldInitClassifier. */
        //Indizes der postiven Boxen
        vector<int>* confidentIndices;
        int *featureVectors;
        int numberOfCluster;
        cv::Rect *detectorBB;
        void reset();
    protected:
    private:
};

#endif // RESULT_H
