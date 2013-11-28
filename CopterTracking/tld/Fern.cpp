#include "../include/Fern.h"

Fern::Fern() {}
Fern::Fern(int x1, int y1, int x2, int y2)
{
    d1 = Point2f(x1,y1);
    d2 = Point2f(x2,y2);
    //ctor
}

Fern::~Fern()
{
    //dtor
}

bool Fern::value(const Mat& patch)
{
    //cout << d1 << " " << d2 << endl;
    return patch.at<unsigned char>(d1) > patch.at<unsigned char>(d2);
}

