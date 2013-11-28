#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>
#include <iostream>

#include <opencv2/core/core.hpp>

#include "Fern.h"

using namespace std;
using namespace cv;

class Object
{
    public:
        Object();
        Object(int,Rect);
        virtual ~Object();
        float confidence;
        float variance;
        float s_relative;
        //vector<Fern> ferns; /// NUR ZUM TEST > aber noch nicht wirklich durchgeführt
        int getIndex() { return index; }
        Rect& getRect() {return rect; };
        void setOverlap(float over) { overlap = over; }
        float getOverlap() { return overlap; }
        void setPatch(Mat& p) { patch = p.clone();}
        Mat getPatch() { return patch; }
        vector<int>& getObjectIds() { return objectIds; }
    protected:
    private:
        int index;
        Rect rect;
        float overlap;
        Mat patch;
        vector<int> objectIds;

};

#endif // OBJECT_H
