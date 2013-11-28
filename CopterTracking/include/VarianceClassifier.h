#ifndef VARIANCECLASSIFIER_H
#define VARIANCECLASSIFIER_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Utility.h"
#include "Result.h"

using namespace std;
using namespace cv;

class VarianceClassifier
{
    public:
        VarianceClassifier();
        virtual ~VarianceClassifier();
        double variance;
        vector<double> variances;
        void calcIntegral(const Mat&);
        //void calcMinVariance(Rect&);
        bool classify(Object&,const Mat&);
        bool classify(int&,int*,Result*);
        //void multi_calcMinVariance(Object&);
        bool multi_classify(Object&, Mat&);
    protected:
    private:
        Mat integralImg;
        Mat integralImgSqrt;
};

#endif // VARIANCECLASSIFIER_H
