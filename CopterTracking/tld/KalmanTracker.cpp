#include "../include/KalmanTracker.h"

KalmanTracker::KalmanTracker()
{
    numberOfPoints = 1; // Für jeden Punkt werden 2 Werte genommen + die jeweiligen Deltas
    state = Mat_<float>(numberOfPoints * 2 * 2,1);
    noise = Mat(numberOfPoints * 2 * 2,1,CV_32F);
    measurement = Mat_<float>(numberOfPoints * 2,1); // Gemessen werden natürlich nur die 2 Werte (ohne Deltas)
    measurement.setTo(Scalar(0));
    //ctor
}

KalmanTracker::~KalmanTracker()
{
    //dtor
}

void KalmanTracker::init(Rect& bb)
{
    // Kalman vorbereiten > KalmanFilter(int dynamicParams, int messureParams, 0)
    kalman = new KalmanFilter(numberOfPoints * 2 * 2, 2, 0);

    // Nun mit den Punkten Initialisieren (HIER ERSTEINMAL NUR EINER)
    float x = bb.x + bb.width / 2;
    float y = bb.y + bb.height / 2;
    // Den Status mittels
    kalman->statePre.at<float>(0) = x;
    kalman->statePre.at<float>(1) = y;
    kalman->statePre.at<float>(2) = 0;
    kalman->statePre.at<float>(3) = 0;

    measurement.at<float>(0) = x;
    measurement.at<float>(1) = y;
    int speed = 10;
    kalman->transitionMatrix = *(Mat_<float>(4, 4) <<   1,0,speed,0,
                                                        0,1,0,speed,
                                                        0,0,1,0,
                                                        0,0,0,1);

    /*kalman->transitionMatrix = Mat_<float>(numberOfPoints * 2 * 2,numberOfPoints * 2 * 2);
    for (int i = 0; i < numberOfPoints * 2 * 2; ++i) {
        for (int j = 0; j < numberOfPoints * 2 * 2; ++j) {*/
            // Matrix initialisieren, und zwar in der Form
            /*
                |j
            ----+--------
            i   |1,0,1,0
                |0,1,0,1
                |0,0,1,0
                |0,0,0,1
            ----+--------
                 x y vx vy
            */
     /*       if (i % 2 == 1) kalman->transitionMatrix.at<float>(j * i) = 1;
            else  kalman->transitionMatrix.at<float>(j * i) = 0;

        }
    }*/
    setIdentity(kalman->measurementMatrix);
    setIdentity(kalman->processNoiseCov, Scalar::all(1e-4));
    setIdentity(kalman->measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(kalman->errorCovPost, Scalar::all(.1));
    //NUR TEST
    //cvNamedWindow("Kalman",1);

}

bool KalmanTracker::track(Rect& box, Rect& kbb)
{
    kbb.width = box.width;
    kbb.height = box.height;
    Mat prediction = kalman->predict();
    // Predict
    Point predictPt(prediction.at<float>(0),prediction.at<float>(1));
    // Measure
    measurement(0) = box.x + box.width / 2;
	measurement(1) = box.y + box.height / 2;
    // Update
	Mat estimated = kalman->correct(measurement);
	Point statePt(estimated.at<float>(0),estimated.at<float>(1));
    kbb.x = statePt.x - box.width / 2;
    kbb.y = statePt.y - box.height / 2;
    /*circle(frame, predictPt, 1, Scalar( 255, 255, 255 ), 5);
            randn( noise, Scalar(0), Scalar::all(sqrt(kalman->processNoiseCov.at<float>(0, 0))));
           state = kalman->transitionMatrix*state + noise;

    imshow("Kalman",frame);*/
    return true;
}
