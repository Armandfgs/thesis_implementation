#ifndef FERNFEATURE_H
#define FERNFEATURE_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

class FernFeature
{
    public:
        FernFeature();
        FernFeature(int,int,int,int);
        virtual ~FernFeature();
        bool value(const Mat&);
    protected:
    private:
        int x1;
        int y1;
        int x2;
        int y2;
};

#endif // FERNFEATURE_H
