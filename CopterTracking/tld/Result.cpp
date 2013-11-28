#include "../include/Result.h"

/**
 * Diese Klasse sammelt alle Ids der Boxen,
 * mit den dazugeh�rigen Werten (variance, posterior, similarity)
 */

Result::Result()
{
    //ctor
}

Result::~Result()
{
    //dtor
}

/**
 * setzt alle Werte zur�ck.
 */
void Result::reset()
{
    validResult = false;

    //if(fgList != NULL) fgList->clear();

//    if(confidentIndices != NULL) confidentIndices->clear();

    numberOfCluster = 0;
}
