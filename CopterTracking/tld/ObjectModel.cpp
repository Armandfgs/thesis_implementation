#include "../include/ObjectModel.h"

/**
 Das ObjectModel sammelt alle möglichen Objekte (das Grid) und speichert auch bei jedem
 Durchlauf die positiven und negativen Indexe.
 */
ObjectModel::ObjectModel()
{
    cout << "Const. ObjectModel" << endl;
    grid = vector<Object>();
    //ctor
}

ObjectModel::~ObjectModel()
{
    //dtor
}

vector<Object>& ObjectModel::getGrid() { return grid; }

/**
 * Erzeugt anhand der Initialien Box bb das Grid, einmalig zum Start.
 *
 * Hier sind ein paar Boxen zu wenig, weil nur auf der halben Größe gearbeitet wird.... trotzdem ok???
 */
bool ObjectModel::generateGrid(Rect& bb, float& offset)
{
    cout << "ObjectModel::generateGrid()" << endl;
    /// ANMERKUNG: DIE BOXEN WERDEN IMMER MIT EINEM INDEX VERSEHEN
    int w = bb.width;
    int h = bb.height;
    /*int maxW = 640;
    int maxH = 480;*/
    //Nur halbe Höhe und Breite (AUF JEDEN FALL LIEGT HIER EIN FEHLER!!!)
    int maxW = 640 * offset;
    int maxH = 480 * offset;
    float shift = 0.1; //Mutlitplikator für die Schrittweite im Bild
    float s_factor = 1.2; //Skalierungsfaktor
    int a = -10; //Der Esponent > Quasi die äußere Schleife von -10 bis +10
    int minWidth = 40 * offset , minHeight = 40 * offset; //Min-Max-Werte
    float s; //Die berechnete Skalierung s_factor^a
    float temp = 0; //eine temporäre Variables zum Zwischenspeichern der Skalierung, bevor auf- bzw. abgerundet wird.
    int width = 0; //die berechnete breite einer erzeugten Box
    int height = 0; //die berechnete Höhe einer erzeugten Box
    int min_side = 0; //Um über das Bild zu laufen wird die minimale Seitelänge ermittelt (entweder width oder height)
    int id = 0;
    Rect r;
    for (a = -10; a <= 10; ++a)
    {
        s = pow(s_factor,a);
        temp = s * w;
        //Auf- bzw. Abrunden
        width = (int)(temp * 10) % 10 < 5 ? floor(temp) : ceil(temp);
        temp = s * h;
        //Auf- bzw. Abrunden
        height = (int)(temp * 10) % 10 < 5 ? floor(temp) : ceil(temp);
        //Falls sie zu kleine sind, dann mach nich weiter, sondern berechne neue Box
        if (width < minWidth || height < minHeight) continue;
        //Falls zu groß, berechne auch neu
        if (width > maxW || height > maxH) continue;
        //Nun wird für jede mögliche Position eine Box erzeugt > also x und y werden gesetzt
        //dazu muss min_side ermittelt werden > Quasi die Schrittweite
        min_side = min(width,height);
        temp = min_side * shift;
        //Wieder auf bzw. abrunden
        temp = (int)(temp * 10) % 10 < 5 ? floor(temp) : ceil(temp);
        for (int y = 0; y < maxH - height; y += temp)
        {
            for (int x = 0; x < maxW - width; x += temp)
            {
                //window_grid.push_back(Rect(x,y,width,height));
                r = Rect(x,y,width,height);
                //Object o(id++);
                //o.setRect(r);
                //HIER WIRD AUCH GLEICH OVERLAP BESTIMMT
                //o.setOverlap(Utility::calcOverlap(getCurrentBox(),r));
                grid.push_back(Object(id++,r));
            }

        }
    }
    return true;
}
