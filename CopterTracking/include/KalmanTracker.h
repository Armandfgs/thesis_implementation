#ifndef KALMANTRACKER_H
#define KALMANTRACKER_H


#include <iostream>
#include <opencv2/opencv.hpp>
#include <cv.h>
#include <highgui.h>
#include <math.h>


using namespace std;
using namespace cv;

class KalmanTracker
{
    public:
        KalmanTracker();
        virtual ~KalmanTracker();
        void init(Rect&);
        bool track(Rect&,Rect&);
    protected:
    private:
        KalmanFilter* kalman;
        Mat_<float> state; // State
        Mat noise; // Weiﬂes Rauschen
        Mat_<float> measurement; // Messungen
        int numberOfPoints;
};

#endif // KALMANTRACKER_H
