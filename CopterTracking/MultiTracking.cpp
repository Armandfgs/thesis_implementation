#include "include/MultiTracking.h"
#include <time.h>
/// FÜRS MULTITRACKING
/**
 IDEE

 Es werden für jedes Objekt TLD-Objekte vorbereitet. Für jede Box wird ein Counter inkrementiert.

*/


char key;
float RESIZE = 1;
int BOX_MIN_HEIGHT = 40 * RESIZE;
int BOX_MIN_WIDTH = 40 * RESIZE;
int objectCounter = 0;
Rect newBox;
bool isNewBox;
bool newBoxReady;
bool showStabeliziedBox = false;
bool pause = false;
Scalar red = Scalar(0,0,255), green = Scalar(0,255,0),blue = Scalar(255,0,0), yellow = Scalar(0,255,255);
Rect box_tld_1, box_tld_2, box_tld_3;
Mat framePause;


static void mouseCallback( int event, int x, int y, int flags, void* param )
{
     if( event == CV_EVENT_LBUTTONDOWN && objectCounter < 3 )
     {
         //cout << "DOWN at " << x << "," << y << endl;
         newBox = Rect(x,y,0,0);
         isNewBox = true;
         //startBox = true;
         //boxDefined = false;
     }

     if ( isNewBox && event == CV_EVENT_MOUSEMOVE && objectCounter < 3 )
     {
         //cout << "MOVE at " << x << "," << y << endl;
         newBox.width = x - newBox.x;
         newBox.height = y - newBox.y;
     }

     if( event == CV_EVENT_LBUTTONUP && objectCounter < 3 )
     {
         isNewBox = false;
         newBoxReady = true;
     }
 }


MultiTracking::MultiTracking(VideoCapture* i)
{
    input = i;
    tld_1 = 0;
    tld_2 = 0;
    tld_3 = 0;
}

MultiTracking::~MultiTracking()
{
    //dtor
}

void MultiTracking::go()
{
    // Die Matrizen vorbereiten
    Scalar col;
    bool success_1 = false, success_2 = false, success_3 = false;
    // für FPS
    time_t start_t, end_t; int counter = 0; double fps; double sec;
    time(&start_t);

    //Das Ausgabefenster
    namedWindow("Ausgabe",1);
    //callback für mouse events
    setMouseCallback("Ausgabe",mouseCallback,0);
    // Die Hauptschleife
    while(true) {
        // Das aktuelle Bild holen
        if (!pause)
            *input >> actualFrame;
        else
            actualFrame = framePause;

        //resize(actualFrame,actualFrame,cvSize(actualFrame.cols * RESIZE,actualFrame.rows * RESIZE));
        // und für die Ausgabe kopieren
        showFrame = actualFrame.clone();
        // nun auf Grau stellen
        cvtColor( actualFrame,actualFrame,CV_BGR2GRAY);
        /// Die Reaktion auf die Auswahl
        // Falls gerade eine Box definiert wird, dann gibt sie aus
        if (isNewBox) {
            if (newBox.width < BOX_MIN_WIDTH || newBox.height < BOX_MIN_HEIGHT) col = red; // Wenn zu klein, dann ROT
            else {
                //Je nachdem, wie viele Boxen definiert werden
                if (objectCounter == 0) col = green;
                else if (objectCounter == 1) col = blue;
                else if (objectCounter == 2) col = yellow;
            }
            rectangle(showFrame,cvPoint(newBox.x,newBox.y),cvPoint(newBox.x + newBox.width, newBox.y + newBox.height),col,2);
        }
        // Falls eine Neue Box defniert wurde, initialisiere TLD (je nach Counter)
        if (newBoxReady) {
            switch(objectCounter) {
                case 0: tld_1 = new TLD();
                    tld_1->init(actualFrame,newBox,actualFrame.cols,actualFrame.rows);
                    box_tld_1 = newBox;
                    break;
                case 1: tld_2 = new TLD();
                    tld_2->init(actualFrame,newBox,actualFrame.cols,actualFrame.rows);
                    box_tld_2 = newBox;
                    break;
                case 2: tld_3 = new TLD();
                    tld_3->init(actualFrame,newBox,actualFrame.cols,actualFrame.rows);
                    box_tld_3 = newBox;
                    break;
            }
            objectCounter++;
            newBoxReady = false;
        }
        // Nun das Processing
        if (tld_1 != 0 && tld_1->isActive) {
            success_1 = tld_1->process(actualFrame,box_tld_1,box_tld_1,showFrame);
        }
        if (tld_2 != 0 && tld_2->isActive) {
            success_2 = tld_2->process(actualFrame,box_tld_2,box_tld_2,showFrame);
        }
        if (tld_3 != 0 && tld_3->isActive) {
            success_3 = tld_3->process(actualFrame,box_tld_3,box_tld_3,showFrame);
        }

        //Auswertung des Prozessing
        if (success_1 || success_2 || success_3) {
            // eine von den TLDs erfolgreich, dann gebe aus
            if (success_1) {
                checkBox(box_tld_1,actualFrame.cols,actualFrame.rows);
                // Anzeige des Objekts in einem Extrafenster
                if (success_1 && showStabeliziedBox) {
                    Rect b = box_tld_1; b.x -= 20; b.y -= 20; b.width += 40; b.height += 40;
                    checkBox(b,showFrame.cols, showFrame.rows);
                    imshow("1", actualFrame(b));
                }
                rectangle(showFrame,cvPoint(box_tld_1.x,box_tld_1.y),cvPoint(box_tld_1.x + box_tld_1.width, box_tld_1.y + box_tld_1.height),green,2);

            }
            if (success_2) {
                checkBox(box_tld_2,actualFrame.cols,actualFrame.rows);
                // Anzeige des Objekts in einem Extrafenster
                if (success_2 && showStabeliziedBox) {
                    Rect b = box_tld_2; b.x -= 20; b.y -= 20; b.width += 40; b.height += 40;
                    checkBox(b,showFrame.cols, showFrame.rows);
                    imshow("2", actualFrame(b));
                }
                rectangle(showFrame,cvPoint(box_tld_2.x,box_tld_2.y),cvPoint(box_tld_2.x + box_tld_2.width, box_tld_2.y + box_tld_2.height),blue,2);
            }
            if (success_3) {
                checkBox(box_tld_3,actualFrame.cols,actualFrame.rows);
                // Anzeige des Objekts in einem Extrafenster
                if (success_3 && showStabeliziedBox) {
                    Rect b = box_tld_3; b.x -= 20; b.y -= 20; b.width += 40; b.height += 40;
                    checkBox(b,showFrame.cols, showFrame.rows);
                    imshow("3", actualFrame(b));
                }
                rectangle(showFrame,cvPoint(box_tld_3.x,box_tld_3.y),cvPoint(box_tld_3.x + box_tld_3.width, box_tld_3.y + box_tld_3.height),yellow,2);
            }
        }
        // Ausgabe der FPS
        time(&end_t);
        ++counter;
        sec = difftime(end_t,start_t);
        fps = counter/sec;
        char buffer[20];
        sprintf(buffer,"FPS: %.2f",fps);
        putText(showFrame, buffer, cvPoint(40,40), CV_FONT_HERSHEY_DUPLEX, 0.3, yellow );
        // Ausgabe der Status alle aktiven TLD-Instanzen
        showTLDStates(showFrame);
        key = cvWaitKey( 50 );
        // Auswertung der Eingabe >> Falls false, dann springe raus!
        if(!keyResult(key)) return;
        // am Ende das aktuelle Bild als lastFrame speichern
        lastFrame = actualFrame.clone();
        // Ergebnis ausgeben
        imshow("Ausgabe",showFrame);

    }
}

bool MultiTracking::keyResult(char key)
{
    // Falls ESC, dann breche ab
    if (key == 27) return false;
    // Ausgabe des Tracker (NUR OBJEKT 1)
    if (key == '1') {
        if (tld_1 != 0) tld_1->showTracker = !tld_1->showTracker;
    }
    // Ausgabe des Detectors (NUR OBJEKT 1)
    if (key == '2') {
        if (tld_1 != 0) tld_1->showDetector = !tld_1->showDetector;
    }
    // Ausgabe der Pos. NNC-Beispiele
    if (key == '3')
    {
        if (tld_1 != 0){
            tld_1->showNNCPos = !tld_1->showNNCPos;
            if (tld_1->showNNCPos) namedWindow("PositiveNNC",1);
            else  destroyWindow("PositiveNNC");
        }
    }
    // Ausgabe der Pos. NNC-Beispiele
    if (key == '4')
    {
        if (tld_1 != 0){
            tld_1->showNNCNeg = !tld_1->showNNCNeg;
            if (tld_1->showNNCNeg) namedWindow("NegativeNNC",1);
            else  destroyWindow("NegativeNNC");
        }
    }
    // Tracker an-/abschalten
    if (key == 't') {
        if (tld_1 != 0) tld_1->useTracker = !tld_1->useTracker;
    }
    // Detector an-/abschalten
    if (key == 'd') {
        if (tld_1 != 0) tld_1->useDetector = !tld_1->useDetector;
        cout << "Detector: " << tld_1->useDetector;
    }
    // Ausgabe der stabilisierten Boxen
    if (key == 'b')
    {
        showStabeliziedBox = !showStabeliziedBox;
        if (tld_1 != 0) {
            if (showStabeliziedBox) namedWindow("1",1);
            else  destroyWindow("1");
        }
        if (tld_2 != 0) {
            if (showStabeliziedBox) namedWindow("2",1);
            else  destroyWindow("2");
        }
        if (tld_3 != 0) {
            if (showStabeliziedBox) namedWindow("3",1);
            else  destroyWindow("3");
        }
    }
    //An- und Abschalten des Lernens
    if (key == 'l')
    {
        if (tld_1 != 0) tld_1->doLearn = !tld_1->doLearn;
        if (tld_2 != 0) tld_2->doLearn = !tld_2->doLearn;
        if (tld_3 != 0) tld_3->doLearn = !tld_3->doLearn;
        cout << "Learning: ";
        if (tld_1->doLearn) cout << "AN" << endl;
        else cout << "AUS" << endl;
    }
    //Fern nutzen
    if (key == 'f') {
        if (tld_1 != 0) tld_1->detector.useFerns = !tld_1->detector.useFerns;
    }
    // NNC nutzen
    if (key == 'n') {
        if (tld_1 != 0) tld_1->detector.useNNC = !tld_1->detector.useNNC;
    }

    if (key == 'p') {
        pause = !pause;
        if (pause) *input >> framePause;
    }
    return true;
}

/**
 * Ausgabe der Informationen zu jedem TLD-Objekt
 */
void MultiTracking::showTLDStates(Mat& f)
{
    if (tld_1 != 0) {
        tld_1->generateWindows(f,35,60,green);
    }
    if (tld_2 != 0) {
        tld_2->generateWindows(f,35,100,blue);
    }
    if (tld_3 != 0) {
        tld_3->generateWindows(f,35,140,yellow);
    }
}

/**
 * Korrigiert die Box
 */
void MultiTracking::checkBox(Rect& box, int width, int height)
{
    // x
    if (box.x < 0) box.x = 0;
    if (box.x >= width) box.x = width - 1;
    // y
    if (box.y < 0) box.x = 0;
    if (box.y >= height) box.y = height - 1;
    // Die Breite
    if (box.x + box.width > width) box.width = box.width - (box.x + box.width - width - 1);
    // Die Höhe
    if (box.y + box.height > height) box.height = box.height - (box.y + box.height - height - 1);
}
