#include "../include/Object.h"

Object::Object()
{
    //ctor
}

Object::Object(int i, Rect r)
{
    index = i;
    rect = r;
}

Object::~Object()
{
    //dtor
}
