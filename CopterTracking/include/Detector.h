#ifndef DETECTOR_H
#define DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Utility.h"
#include "VarianceClassifier.h"
#include "EnsembleClassifier.h"
#include "NearestNeighbourClassifier.h"
#include "Result.h"

using namespace std;
using namespace cv;

class Detector
{
    public:
        Detector();
        virtual ~Detector();
        bool showDetector;
        bool useFerns;
        bool useNNC;
        void init(Rect&,int,int);
        bool detect(Mat&,vector<Rect>&);
        VarianceClassifier varianceCF;
        EnsembleClassifier ensembleCF;
        NearestNeighbourClassifier nearestnCF;
        //ObjectModel objectModel;
        bool detectorState;
        int gridSize;

        /// NEU:
        // grid enthält die Infos für die einzelnen Fenster
        // für das i-te Fenster gilt:
        // grid[i + 1] = x, grid[i + 2] = y, grid[i + 3] = w, grid[i + 4] = h
        //vector<int*> windowGrid;
        int* grid; /// DAMIT WIRD GEARBEITET: grid[i] = x, grid[i + 1] = y, grid[i + 2] = w, grid[i + 3] = h
        Result* resultSet;
    protected:
    private:
        void clusterBoxes(vector<Rect>&, vector<Rect>&);
        void generateGrid(Rect&, int,int);
        int isInit;

};

#endif // DETECTOR_H
