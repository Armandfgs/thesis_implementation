#ifndef OVERLAPCOMPERATOR_H
#define OVERLAPCOMPERATOR_H

#include "Object.h"

using namespace std;

/** Vergleichsklasse für overlap-Werte der Objekte.
 * Dient vor allem der Sortierung im Detector.
 */

class OverlapComperator
{
    public:
        OverlapComperator();
        OverlapComperator(vector<Object>& ): grid(grid) {};
        virtual ~OverlapComperator();

        bool operator ()(const int &id1, const int &id2)
        {
            return grid[id1].getOverlap() > grid[id2].getOverlap();
        }
    protected:
    private:
        vector<Object> grid;
};

#endif // OVERLAPCOMPERATOR_H
