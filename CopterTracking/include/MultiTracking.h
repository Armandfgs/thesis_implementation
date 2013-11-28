#ifndef MULTITRACKING_H
#define MULTITRACKING_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

#include "TLD.h"


class MultiTracking
{
    public:
        MultiTracking(VideoCapture*);
        virtual ~MultiTracking();
        void go();
    protected:
    private:
        VideoCapture* input;
        TLD* tld_1;
        TLD* tld_2;
        TLD* tld_3;
        bool keyResult(char);
        void checkBox(Rect&,int,int);
        void showTLDStates(Mat&);
        Mat lastFrame, actualFrame, showFrame;

};

#endif // MULTITRACKING_H
