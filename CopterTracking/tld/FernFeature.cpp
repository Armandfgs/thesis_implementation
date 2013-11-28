#include "../include/FernFeature.h"

FernFeature::FernFeature()
{
    //ctor
}

FernFeature::FernFeature(int x_1, int y_1, int x_2, int y_2)
{
    x1 = x_1;
    y1 = y_1;
    x2 = x_2;
    y2 = y_2;
    //ctor
}


FernFeature::~FernFeature()
{
    //dtor
}

bool FernFeature::value(const Mat& patch)
{
    //cout << "[" << x1 << "," << y1 << "]" << endl;
    return patch.at<unsigned char>(x1,y1) > patch.at<unsigned char>(x2,y2);
}
