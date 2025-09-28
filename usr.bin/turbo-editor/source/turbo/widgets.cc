#include "widgets.h"
#include <ctime>

TClockView::TClockView( TRect& r ) :
    TView(r)
{
    strcpy(lastTime, "        ");
    strcpy(curTime, "        ");
}


void TClockView::draw()
{
    TDrawBuffer buf;
    uchar c = getColor(2);

    buf.moveChar(0, ' ', c, size.x);
    buf.moveStr(0, curTime, c);
    writeLine(0, 0, size.x, 1, buf);
}


void TClockView::update()
{
    time_t t = time(0);
    char *date = ctime(&t);

    date[19] = '\0';
    strcpy(curTime, &date[11]);

    if (strcmp(lastTime, curTime)) {
        drawView();
        strcpy(lastTime, curTime);
    }
}
