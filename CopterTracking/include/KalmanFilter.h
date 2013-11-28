#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include "cv.h"
#include "highgui.h"

using namespace std;
using namespace cv;

class KalmanFilter
{
    public:
        KalmanFilter();
        virtual ~KalmanFilter();
    protected:
    private:
        CvKalman* kalman; // Das Kalman-Objekt (naja, ne C-Struktur, aber is egal)
        RNG& rng; //Zufallszahlengenerator
        CvMat* state_k; // Status x_k
        CvMat* noise_k; // Noise w_k
        CvMat* messu_k; // Messurment z_k

};

#endif // KALMANFILTER_H
