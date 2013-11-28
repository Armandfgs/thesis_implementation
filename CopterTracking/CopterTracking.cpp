#include "include/CopterTracking.h"
#include<time.h>

Rect box;
bool startBox;
bool boxDefined;
char keyPressed;
int MIN_HEIGHT = 20;
int MIN_WIDTH = 20;

static void mouseCallback( int event, int x, int y, int flags, void* param )
 {
     if( event == CV_EVENT_LBUTTONDOWN )
     {
         //cout << "DOWN at " << x << "," << y << endl;
         box = Rect(x,y,0,0);
         startBox = true;
         boxDefined = false;
     }

     if ( startBox && event == CV_EVENT_MOUSEMOVE)
     {
         //cout << "MOVE at " << x << "," << y << endl;
         box.width = x - box.x;
         box.height = y - box.y;
     }

     if( event == CV_EVENT_LBUTTONUP)
     {
         startBox = false;
         boxDefined = true;

     }
 }


CopterTracking::CopterTracking(VideoCapture* i)
{
    input = i;
    tld = new TLD();

    //ctor
}

CopterTracking::~CopterTracking()
{
    //dtor
}

void CopterTracking::go() {
    Mat lastFrame;
    Mat nextFrame;
    Mat framePause;
    Rect nextBox;
    bool success;
    bool pause;
    // für FPS
    time_t start_t, end_t; int counter = 0; double fps; double sec;
    //bool boxDefined = true;
    //Das Ausgabefenster
    namedWindow("Ausgabe",1);
    //callback für mouse events
    setMouseCallback("Ausgabe",mouseCallback,0);

    //zu Beginn das erste und das zweite Bild speichern
    *input >> lastFrame;
    tld->showFrame = lastFrame.clone();
    *input >> nextFrame;

    //Eingrauen
    cvtColor( lastFrame,lastFrame,CV_BGR2GRAY);
    cvtColor( nextFrame,nextFrame,CV_BGR2GRAY);
    // FPS berechnen
    time(&start_t);
    //Schleife
    while (true)
    {

        //TLD starten, falls eine Box angegeben wurde
        if (boxDefined)
        {
            //Es wurde eine Box definiert, also ist startBox = false;
            startBox = false;
            //Die Box verkleinern
            box.x = ceil(box.x);
            box.y = ceil(box.y);
            box.width = ceil(box.width);
            box.height = ceil(box.height);
            ///Falls tld das erste Mal gestartet wird, muss es initialisiert werden
            if (!tld->isActive) {
                tld->init(lastFrame,box,lastFrame.cols,lastFrame.rows);
            }
        }
        if (tld->isActive && !startBox) {
            success = tld->process(nextFrame,box,nextBox,tld->showFrame);
        } else if (startBox  && box.width > 0 && box.height > 0)
        {
            tld->isActive = false;
            //Die Box muss eine Mindestgröße haben!!!
            if (box.width < MIN_WIDTH || box.height < MIN_HEIGHT)
            {
                 rectangle(tld->showFrame,cvPoint(box.x,box.y),cvPoint(box.x + box.width, box.y + box.height),Scalar(0,0,255),2); //Wenn zu klein, dann ROT
                 boxDefined = false;
            }
            else
            {
                rectangle(tld->showFrame,cvPoint(box.x,box.y),cvPoint(box.x + box.width, box.y + box.height),Scalar(0,255,0),2); //Wenn ok, dann GRÜN
            }
        //Falls nicht gefunden und auch nicht definiert, lösche box und nextBox
        } else {
            //box.x = 0; box.y = 0; box.width = 0; box.height = 0;
            boxDefined = false;
            //nextBox = Rect();
            nextBox = box;
        }
        //Falls tld erfolgreich war, dann zeige box im nextFrame an
        if (success )
        {
            success = false;
            startBox = false;
            boxDefined = true;
            //Die Box vergrößern
            nextBox.x = ceil(nextBox.x);
            nextBox.y = ceil(nextBox.y);
            nextBox.width = ceil(nextBox.width);
            nextBox.height = ceil(nextBox.height);
            //Die Grenzen genau definieren!!! (REICHT DAS SO???)
            //x
            if (nextBox.x < 0) nextBox.x = 0;
            if (nextBox.x >= tld->showFrame.size().width) nextBox.x = tld->showFrame.size().width - 1;
            //y
            if (nextBox.y < 0) nextBox.x = 0;
            if (nextBox.y >= tld->showFrame.size().height) nextBox.y = tld->showFrame.size().height - 1;

            //Die Breite
            if (nextBox.x + nextBox.width > tld->showFrame.size().width) nextBox.width = nextBox.width - (nextBox.x + nextBox.width - tld->showFrame.size().width - 1);
            //Die Höhe
            if (nextBox.y + nextBox.height > tld->showFrame.size().height) nextBox.height = nextBox.height - (nextBox.y + nextBox.height - tld->showFrame.size().height - 1);

            //aktuelle Box setzen
            box = nextBox;
            //Ausgabe
            if (tld->showBoundingBox)
            {
                imshow("Box",tld->showFrame(box));
            }
            if (tld->showCoordinates)
            {
                //Falls eine Box gefunden wurde, dann gebe die Koordinaten aus, falls gewünscht
                char buffer[20];
                sprintf(buffer,"%d, %d",box.x + (box.width / 2), box.y + (box.height / 2));
                putText(tld->showFrame,
                    buffer,
                    Point(box.x + (box.width / 2) + 5, box.y + (box.height / 2)),
                    CV_FONT_HERSHEY_DUPLEX,
                    0.3,
                    Scalar(0, 255, 255)
                );
                 circle(tld->showFrame, Point2f(box.x + (box.width / 2), box.y + (box.height / 2)), 1, Scalar( 0, 0, 255  ), 5);
            } else {
                rectangle(tld->showFrame,cvPoint(nextBox.x,nextBox.y),cvPoint(nextBox.x + nextBox.width, nextBox.y + nextBox.height),Scalar(0,255,0),2);

            }

        }
        // FPS berechnen
        time(&end_t);
        ++counter;
        sec = difftime(end_t,start_t);
        fps = counter/sec;
        char buffer[20];
        sprintf(buffer,"FPS: %.2f",fps);
        putText(tld->showFrame,
            buffer,
            cvPoint(40,40),
            CV_FONT_HERSHEY_DUPLEX,
            0.3,
            Scalar(0, 255, 255)
        );

        //Ausgabe des nextFrame
        imshow("Ausgabe",  tld->showFrame);
        keyPressed = cvWaitKey( 10 );
        //Das Programm verlassen
        if (keyPressed == 27)
        {
            return;
        }

        //An- und Abschalten der Trackerausgabe
        if (keyPressed == '1')
        {
            tld->showTracker = !tld->showTracker;
            if (tld->showTracker && success) namedWindow("Tracker",1);
            else  destroyWindow("Tracker");

        }
        //An- und Abschalten der Detectorausgabe
        if (keyPressed == '2')
        {
            tld->showDetector = !tld->showDetector;
            if (tld->showDetector && success) namedWindow("Detector",1);
            else  destroyWindow("Detector");

        }

        //Anzeigen der erzeugten Beispiele
        if (keyPressed == '4')
        {
            tld->showFernPos = !tld->showFernPos;
            if (tld->showFernPos && success)
            {
                namedWindow("PositiveFerns",1);

            }
            else  destroyWindow("PositiveFerns");

        }

        //Anzeigen der erzeugten Beispiele
        if (keyPressed == '5')
        {
            tld->showNNCPos = !tld->showNNCPos;
            if (tld->showNNCPos && success)
            {
                namedWindow("PositiveNNC",1);

            }
            else  destroyWindow("PositiveNNC");

        }

        //Anzeigen der erzeugten Beispiele
        if (keyPressed == '6')
        {
            tld->showFernNeg = !tld->showFernNeg;
            if (tld->showFernNeg && success)
            {
                namedWindow("NegativeFerns",1);

            }
            else  destroyWindow("NegativeFerns");

        }

        //Anzeigen der erzeugten Beispiele
        if (keyPressed == '7')
        {
            tld->showNNCNeg = !tld->showNNCNeg;
            if (tld->showNNCNeg && success)
            {
                namedWindow("NegativeNNC",1);

            }
            else  destroyWindow("NegativeNNC");

        }
        if ( keyPressed == 'p')
        {
            pause = !pause;
            *input >> framePause;
        }

        //An- und Abschalten der Ausgabe der Positiven Esembler-Boxen
        if (keyPressed == 'h')
        {
            tld->showHull = !tld->showHull;
        }

        //An- und Abschalten des Lernens
        if (keyPressed == 'l')
        {
            tld->doLearn = !tld->doLearn;
        }
        //Anzeigen der stabilisierten BoundingBox
        if (keyPressed == 'b')
        {
            tld->showBoundingBox = !tld->showBoundingBox;
            if (tld->showBoundingBox && success)
            {
                namedWindow("Box",1);

            }
            else  destroyWindow("Box");

        }

        //Ausgabe der Kooridinaten
        if (keyPressed == 'k') {
                tld->showCoordinates = !tld->showCoordinates;
        }

        //Werte aktualisieren
        lastFrame = nextFrame;
        if (!pause) {
            *input >> nextFrame;
        }  else {
            nextFrame = framePause.clone();
        }


        //Ausgabe
        tld->showFrame = nextFrame.clone();
        cvtColor( nextFrame,nextFrame,CV_BGR2GRAY);

        //boxDefined = success;

    }
}
