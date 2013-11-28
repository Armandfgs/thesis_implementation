#ifndef FERN_H
#define FERN_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

class Fern
{
    public:
        Fern();
        Fern(int,int,int,int);
        virtual ~Fern();
        bool value(const Mat&);
        Point2f& getD1() { return d1; }
        Point2f& getD2() { return d2; }
    protected:
    private:
        Point2f d1;
        Point2f d2;
};

#endif // FERN_H
