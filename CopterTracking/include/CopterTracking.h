#ifndef COPTERTRACKING_H
#define COPTERTRACKING_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

#include "TLD.h"

using namespace cv;

class CopterTracking
{
    public:
        CopterTracking(VideoCapture*);
        virtual ~CopterTracking();
        void go();
    protected:
    private:
        VideoCapture* input;
        TLD* tld;
        TLD* tld_1;
        int objectCounter;

};

#endif // COPTERTRACKING_H
