#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <stdio.h>
#include <iostream>

#include <opencv2/core/core.hpp>

#include "Object.h"

using namespace std;
using namespace cv;

class ObjectModel
{
    public:
        ObjectModel();
        virtual ~ObjectModel();
        bool generateGrid(Rect&,float&);
        vector<Object>& getGrid();
        /// Neu. Während der Detection werden diese vectoren gefüllt
        /// Sind public, weil dann leichter zu handhaben
        vector<Object> negFernExamples;
        vector<Object> negNNCExamples;
        vector<Object> positiveExamples;
    protected:
    private:
        vector<Object> grid;

};

#endif // OBJECTMODEL_H
