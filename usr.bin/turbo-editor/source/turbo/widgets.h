#ifndef TURBO_WIDGETS_H
#define TURBO_WIDGETS_H

#define Uses_TView
#include <tvision/tv.h>

class TClockView : public TView
{
// Copied from TVDemo

public:

    TClockView( TRect& r );
    virtual void draw();
    virtual void update();

private:

    char lastTime[9];
    char curTime[9];

};

#endif
