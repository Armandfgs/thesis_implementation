#ifndef TRACKER_H
#define TRACKER_H


#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Utility.h"

using namespace std;
using namespace cv;

class Tracker
{
    public:
        Tracker();
        virtual ~Tracker();
        bool track(Mat&, Mat&, Rect&, Rect&, Rect&);
        bool showTracker;
    protected:
    private:
        void calcEuklidianDistance(vector<Point2f>&,vector<Point2f>&, vector<float>&);
        void calcValidePoints(vector<Point2f>&,vector<Point2f>&, vector<uchar>&, vector<float>&, float&, vector<float>&);
        void calcBoundingBox(Rect&,Rect&,vector<Point2f>&,vector<Point2f>&);
        void calcNormCrossCorrelation(Mat&,Mat&,vector<Point2f>&,vector<Point2f>&, vector<uchar>&,vector<float>&);
};

#endif // TRACKER_H
