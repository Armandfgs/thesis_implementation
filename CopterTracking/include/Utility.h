#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <iostream>

#include "opencv2/core/core.hpp"

#include "Object.h"

using namespace std;
using namespace cv;

class Utility
{
    public:
        Utility();
        virtual ~Utility();
        static float calcMedian(vector<float>&);
        static double calcVariance(Rect&,const Mat&, const Mat&);
        static float calcVariance(Mat&);
        static float calcOverlap(const Rect&, const Rect&);
        static void calcOverlap(int*,int&, Rect&, float*);
        static bool compareBoxes(const Rect&, const Rect&);
        static bool compareConfidence(const Object&, const Object&);
        static bool compareOverlap(const pair<int,float>,const pair<int,float>);
        static bool generateMultipleImages(vector<Mat>&,Mat&,int,int,int,int);
        static bool calcHull(int*, vector<pair<int,float> >&, Rect&);
        static void generateNormalizedPatch(const Mat&, Mat&);
    protected:
    private:
};

#endif // UTILITY_H
