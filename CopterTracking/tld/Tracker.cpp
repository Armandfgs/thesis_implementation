#include "../include/Tracker.h"

Tracker::Tracker()
{
    cout << "Const. Tracker" << endl;
    showTracker = false;
    //ctor
}

Tracker::~Tracker()
{
    //dtor
}

bool Tracker::track(Mat& f1, Mat& f2, Rect& bb, Rect& kbb, Rect& tbb)
{
    //cout << "Start tracking...";
    //Berechnen der validen Punkte
    vector<Point2f> v0Points = vector<Point2f>(); //Ausgangs
    vector<Point2f> v1Points = vector<Point2f>(); //Ergebnis 1
    vector<Point2f> v2Points = vector<Point2f>(); //Ergebnis 2

    int stepx = min((int)ceil(bb.width / 10), 10);
    int stepy = min((int)ceil(bb.height / 10), 10);

    /// NEU: Definiere ROI anhand des Kalmar-Outputs
    Rect roi = Rect(kbb.x - kbb.width/2, kbb.y - kbb.height/2, kbb.width*2, kbb.height*2);

    for(int i = 0; i < bb.width; i += stepx)
        for (int j = 0; j < bb.height; j += stepy)
            v0Points.push_back(Point2f(bb.x + i, bb.y + j));
    // NEU! Wir Suchen gute Features für die bb.
    //GaussianBlur(f1,f1,Size(9,9),1.5);
    //GaussianBlur(f2,f2,Size(9,9),1.5);
    /** TEST goodFeaturesToTrack(f1(bb),v0Points,200,0.05,1);
    // Im Anschluss müssen die x-y-Werte der Features noch an die richtige Stelle geschoben werden ;)
    for (size_t i = 0; i < v0Points.size(); ++i) {
        v0Points.at(i).x += bb.x;
        v0Points.at(i).y += bb.y;
    }*/
    //LKT
    Size winSize = Size(10,10);
    int lvl = 3;
    TermCriteria crit = TermCriteria( TermCriteria::COUNT + TermCriteria::EPS, 100, 0.001);
    //Der Status durch forward_backward
    vector<uchar> status_forward;
    vector<uchar> status_backward;
    //Der Error durch forward_backward
    vector<float> error_forward;
    vector<float> error_backward;
    vector<float> norm_cross_correlation = vector<float>();
    //forward-backward-LKT
/// TODO (Ronald#1#): An diser Stelle bricht das Programm manchmal ab. Anscheinend ist die Anzahl der Punkte nicht richtig. Ist jetzt gefixed.
    if (v0Points.size() == 0) return false;

    calcOpticalFlowPyrLK(f1,f2,v0Points,v1Points,status_forward, error_forward, winSize,lvl,crit,OPTFLOW_LK_GET_MIN_EIGENVALS);
    //calcOpticalFlowPyrLK(f1(roi),f2(roi),v0Points,v1Points,status_forward, error_forward, winSize,lvl,crit,0.5,0);
    if (v1Points.size() == 0) return false;
    calcOpticalFlowPyrLK(f2,f1,v1Points,v2Points,status_backward, error_backward, winSize,lvl,crit,OPTFLOW_LK_GET_MIN_EIGENVALS);
    //calcOpticalFlowPyrLK(f2(roi),f1(roi),v1Points,v2Points,status_backward, error_backward, winSize,lvl,crit,0.5,0);
    if (v2Points.size() == 0) return false;
    //Euklidischer Abstand berechnen
    vector<float> distance = vector<float>();
    calcEuklidianDistance(v0Points,v2Points,distance);
    float medianDistance = Utility::calcMedian(distance);

    //NCC berechnen
    calcNormCrossCorrelation(f1,f2, v0Points, v1Points, status_forward, norm_cross_correlation);
    //Berechnung der validen Punkte (auch v0Points muss übergeben werden!!)
    calcValidePoints(v0Points,v1Points,status_forward,distance,medianDistance,norm_cross_correlation);
    //Berechnung der neuen Box
    calcBoundingBox(bb,tbb,v0Points,v1Points);
    //Falls medianDistance größer als treshhold ist, dann verwerfe das Ergebnis und gib KEINE Box zurück
    Point2f centerT = Point2f((tbb.x + tbb.width / 2),(tbb.y + tbb.height / 2));
    Point2f centerK = Point2f((kbb.x + kbb.width / 2),(kbb.y + kbb.height / 2));

    float dist = sqrt((centerK.x - centerT.x) * (centerK.x - centerT.x) + (centerK.y - centerT.y) * (centerK.y - centerT.y));

    //Falls erwünscht die Trackerbox anzeigen
    if (showTracker)
    {
        Mat tMat = f2.clone();
        //size_t i = 0;
        for (size_t i = 0; i < v0Points.size(); ++i)
        {
            circle(tMat, Point2f(v1Points[i].x, v1Points[i].y), 1, Scalar( 0, 0, 255  ), 1);

        }
        rectangle(tMat ,cvPoint(tbb.x,tbb.y),cvPoint(tbb.x + tbb.width, tbb.y + tbb.height),Scalar(0,255,0),2);
        rectangle(tMat ,cvPoint(kbb.x,kbb.y),cvPoint(kbb.x + kbb.width, kbb.y + kbb.height),Scalar(255,255,0),2);
        rectangle(tMat ,cvPoint(roi.x,roi.y),cvPoint(roi.x + roi.width, roi.y + roi.height),Scalar(255,255,255),2);
        imshow("Tracker",  tMat);

    }
    if (medianDistance > dist) {
        cout << "MedianTreshold!!! Tracker findet nichts mehr. MedianDistance > dist zu Kalman: " << medianDistance << " < " << dist << endl;
        // Korrektur durch KalmanFilter
        cout << "Overlap mit Kalman: " << Utility::calcOverlap(kbb,tbb) << endl;
        return false;
    }
    //cout << "DONE!" << endl;
    return true;
}

/**
 * Berechnen der Normalized Cross Correlation
 */
void Tracker::calcNormCrossCorrelation(Mat& f1, Mat& f2, vector<Point2f>& p1, vector<Point2f> &p2, vector<uchar>& status, vector<float>& ncc)
{
    Mat patch0(10,10,CV_8U);
    Mat patch1(10,10,CV_8U);
    Mat result(1,1,CV_32F);
    int size = p1.size();
    for (int i = 0; i < size; i++) {
        if (status[i]) {
            getRectSubPix( f1, Size(10,10), p1[i],patch0 );
            getRectSubPix( f2, Size(10,10), p2[i],patch1);
            matchTemplate( patch0,patch1, result, CV_TM_CCOEFF_NORMED);
            ncc.push_back(((float *)(result.data))[0]);
        } else {
/// Ist es wirklich ok, wenn ich die einfach auslasse? Problem könnte sein, dass die Größe von NCC-Vector und v1Points-Vector abweichen.
/// Ansonsten Workaround für die Berechnung des Median von NCC.
            //ncc.push_back(0.0);
        }
    }
    patch0.release();
    patch1.release();
    result.release();
}

/**
 * Aus der alten Box bb und der mittleren Änderung der Punkte wird eine neue Box berechnet.
 */
void Tracker::calcBoundingBox(Rect& bb, Rect& tbb, vector<Point2f>& v0Points, vector<Point2f>& v1Points)
{
/// TODO: BEHANDLUNG VON FEHLERN, WENN v0Points oder v1Points z.B. leer sind
    //Als erstes wird berechnet, wie sehr sich x und y im Mittel über alle Punkte von v0Points zu v1Points geändert haben
    vector<float> changeX = vector<float>();
    vector<float> changeY = vector<float>();
    float medianChangeX = 0.0;
    float medianChangeY = 0.0;
    float medianChangeP = 0.0;
    //float medianDistance = 1.0;
    //Die Zählvariable
    size_t i,j;
    for (i = 0; i < v0Points.size(); ++i)
    {
        changeX.push_back(v0Points[i].x - v1Points[i].x);
        changeY.push_back(v0Points[i].y - v1Points[i].y);

    }
    medianChangeX = Utility::calcMedian(changeX);
    medianChangeY = Utility::calcMedian(changeY);

    /** Berechne die Änderung der getrackten Punkte im Mittel */
    if (v0Points.size() > 1){
        vector<float> d;
        d.reserve(v0Points.size() * (v0Points.size() - 1) / 2);
        for (i=0; i < v0Points.size(); ++i){
            for (j = i + 1; j < v0Points.size(); ++j){
                d.push_back(norm(v1Points[i]-v1Points[j])/norm(v0Points[i]-v0Points[j]));
            }
        }
        medianChangeP = Utility::calcMedian(d);
    }
    else {
        medianChangeP = 1.0;
    }

    float s1 = 0.5 * (medianChangeP - 1) * bb.width;
    float s2 = 0.5 * (medianChangeP - 1) * bb.height;

    float temp = bb.x - medianChangeX + s1;
    tbb.x = ((int(temp * 10)) % 10 < 5) ? floor(temp) : ceil(temp);

    temp = bb.y - medianChangeY + s2;
    tbb.y = ((int(temp * 10)) % 10 < 5) ? floor(temp) : ceil(temp);

    temp = bb.width * medianChangeP;
    tbb.width = ((int(temp * 10)) % 10 < 5) ? floor(temp) : ceil(temp);

    temp = bb.height * medianChangeP;
    tbb.height = ((int(temp * 10)) % 10 < 5) ? floor(temp) : ceil(temp);

    //cout << " bb[x,y,w,h]: " << bb.x << "," << bb.y << "," << bb.width << "," << bb.height << endl;
    //cout << "tbb[x,y,w,h]: " << tbb.x << "," << tbb.y << "," << tbb.width << "," << tbb.height << endl;
}

/**
 * Berechnen des euklidischen Abstands zwischen Punktevectoren
 */
void Tracker::calcEuklidianDistance(vector<Point2f>& v0Points, vector<Point2f>& v2Points, vector<float>& distance)
{
    size_t i = 0;
    for (i = 0; i < v0Points.size(); ++i)
        distance.push_back(sqrt((v0Points[i].x - v2Points[i].x) * (v0Points[i].x - v2Points[i].x) + (v0Points[i].y - v2Points[i].y) * (v0Points[i].y - v2Points[i].y)));
}

/**
 * Berechnet die validen Punkte, die für die Berechnung der nächste Box in Frage kommen.
 */
void Tracker::calcValidePoints(vector<Point2f>& v0Points, vector<Point2f>& v1Points, vector<uchar>& status_forward, vector<float>& distance, float& medianDistance, vector<float>& ncc)
{
    // Iteriere über alle v1Points und prüfe, ob
    // 1. überhaupt ein Punkt gefunden wurde (Status)
    // 2. die medianDistance-Bedingung erfüllt wurde

    /*ACHTUNG:
        Damit die Berechnung der neuen Box gelingt (in calcBoundingBox) muss auch v0Points entsprechend geändert werden. D.h. auch hier
        werden alle Punkte gelöscht, die kein Pendant in v1Points mehr haben!!!
    */
    size_t i;
    size_t validPoints = 0;
///TODO: Eigentlich wäre es Besser, wenn ncc mit dem medianNCC verglichen wird... Also Bestimmung des Median einbauen, damit 50% gefiltert werden??
///      Ein fester Wert gibt allerdings bessere Ergebnisse.
    float medianNCC = .98;
    //float medianNCC = Utility::calcMedian(ncc);
    //cout << "MEDIAN_NCC: " << medianNCC << " #Werte: " << ncc.size() << endl;
    //float medNCC = Utility::calcMedian(ncc);
    for (i = 0; i < v1Points.size(); ++i)
    {
        if (status_forward[i] && distance[i] < medianDistance && ncc[i] > medianNCC)
        {
            v0Points[validPoints] = v0Points[i];
            v1Points[validPoints] = v1Points[i];
            ++validPoints;
        }
    }
    //Größe der Sammlung aller validen Punkte anpassen
    v1Points.resize(validPoints);
}
