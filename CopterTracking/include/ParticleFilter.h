#ifndef PARTICLEFILTER_H
#define PARTICLEFILTER_H

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/legacy/legacy.hpp>

using namespace cv;
using namespace std;

class ParticleFilter
{
    public:
        ParticleFilter();
        virtual ~ParticleFilter();
        void init();
        bool track(Rect&,Mat&);

    protected:
    private:
        size_t vector_size; // Größe des Vectors für die Initialisierungen
        int dynamical_vector; // DP
        int messurement_vector; // MP
        int number_of_samples;   // Anzahl der Partikel
        CvConDensation* condensation;
        void updateConfidence(float*);
};

#endif // PARTICLEFILTER_H
