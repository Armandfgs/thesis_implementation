#ifndef TLD_H
#define TLD_H

#include <stdio.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/legacy/legacy.hpp>

#include "Tracker.h"
#include "Detector.h"
#include "ParticleFilter.h"
#include "KalmanTracker.h"

using namespace std;
using namespace cv;

class TLD
{
    public:
        TLD();
        virtual ~TLD();
        //Initialisierung der Lernalgorithmen
        void initLearning(Rect&);
        void multi_initLearning(int,Rect&);
        bool process(Mat&, Rect&, Rect&, Mat&);
        bool useTracker;
        bool useDetector;
        //Ausgabe des Tracker-Fensters
        bool showTracker;
        //Ausgabe der Detector-Fensters
        bool showDetector;
        //Ausgabe des stabilisierten Bildes
        bool showBoundingBox;
        //Koordinaten der gefundenen Box
        bool showCoordinates;
        // Anzeige der positiven Boxen mit overlap > .6
        bool showPositives;
        //Ausgabe der Postiven Beispiele für den EnsembleClassifier
        bool showFernPos, showFernNeg;
        // Synthetics und NNC-Beispiele
        bool showNNCPos, showNNCNeg;
        // Anzeigen der Hülle
        bool showHull;
        //gibt an, ob tld bereits aktiv ist (also initialisiert wurde)
        bool isActive;
        //Gibt an, ob die gefundene Box valide ist. Falls true, wird gelernt
        bool isValide;
        bool wasValide; // den Letzten Status der Daten > wichtig für die Aktivierung des Lernens
        float OFFSET;
        bool doLearn;
        //Funktion zum Lernen, wenn das gefundene valide war
        void learn(Rect&, Mat&);
        void init(Mat&, Rect&, int,int);
        Detector detector;

        /// FÜR DIE AUSGABE
        Mat showFrame;

        /// NEU! ABFRAGE DER WERTE
        Rect& getTrackerBox() { return tbb; }
        vector<Rect>& getDetectorBox() { return dBoxes; }
        Rect& getResultBox() { return resultBox; }
        void generateWindows(Mat&,int,int,Scalar);
        float getTrackerSim() { return detector.nearestnCF.sim_t; }
        float getDetectorSim() { return detector.nearestnCF.sim_d; }
        float getSim() { return detector.nearestnCF.sim; }
    protected:
    private:
        ParticleFilter particleFilter;
        KalmanTracker kalman;
        Tracker tracker;
        Mat lastFrame;
        Rect tbb;
        Rect kbb;
        vector<Rect> dBoxes;
        Rect resultBox;
        bool hasTracked;
        bool hasDetected;
        bool lastState;
        bool isInilized;
        /// Wird für das Erzeugen der pos. Beispiele genutzt!!!!
        PatchGenerator patchGenerator;
        // void calculateExamples(const Rect&, vector<Object>&,vector<pair<int,float> >&, vector<pair<int,float> >&,int,int);
        // void generateAndLearnExamples(vector<Object>&,vector<pair<int,float> >&, vector<pair<int,float> >&,int);
        void generateAndLearnPosExamples(const Rect&, vector<Object>&,size_t,size_t); /// ALT
        void generateAndLearnNegExamples(size_t,size_t); /// ALT
        bool compareBoundingBoxes(Mat&, Rect&, vector<Rect>&, Rect&);
};

#endif // TLD_H
