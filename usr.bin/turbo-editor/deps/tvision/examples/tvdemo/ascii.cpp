/*----------------------------------------------------------*/
/*                                                          */
/*   Ascii.cpp: Member functions of following classes:      */
/*                TTable                                    */
/*                TReport                                   */
/*                TAsciiChart                               */
/*                                                          */
/*----------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TRect
#define Uses_TEvent
#define Uses_TKeys
#define Uses_TDrawBuffer
#define Uses_TStreamableClass
#define Uses_TStreamable
#define Uses_TView
#define Uses_TWindow
#include <tvision/tv.h>
__link( RView )
__link( RWindow )

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <strstrea.h>
#include <iostream.h>
#include <iomanip.h>

#include "ascii.h"


//
// TTable functions
//

const char * const TTable::name = "TTable";


void TTable::write( opstream& os )
{
    TView::write( os );
}


void *TTable::read( ipstream& is )
{
    TView::read( is );
    return this;
}


TStreamable *TTable::build()
{
    return new TTable( streamableInit );
}


TStreamableClass RTable( TTable::name,
             TTable::build,
             __DELTA(TTable)
               );


TTable::TTable(TRect& r) :
 TView( r )
{
  eventMask |= evKeyboard;
}


void TTable::draw()
{
    TDrawBuffer buf;
    TColorAttr  color = getColor(6);

    for(ushort y = 0; y <= size.y-1; y++)
    {
        buf.moveChar(0, ' ', color, (short)size.x );
        for(ushort x = 0; x <= size.x-1; x++)
            buf.moveChar(x, (ushort)(32*y+x), color, (ushort)1 );
        writeLine(0, y, (short)size.x, (ushort)1, buf);
    }
    showCursor();
}

//
// cmCharFocused is a offset value (basically the ascii code of the
// current selected character) thus should be added, not or'ed, to
// cmAsciiTableCmdBase.
//

void TTable::charFocused()
{
    message(owner, evBroadcast, cmAsciiTableCmdBase + cmCharFocused,
      (void *)(size_t)(cursor.x + 32 * cursor.y));
}


void TTable::handleEvent(TEvent& event)
{
    TView::handleEvent(event);

    if (event.what == evMouseDown)
    {
    do
        {
        if(mouseInView(event.mouse.where))
        {
        TPoint spot = makeLocal(event.mouse.where);
        setCursor(spot.x, spot.y);
        charFocused();
        }
        } while (mouseEvent(event, evMouseMove));
    clearEvent(event);
    }
    else
    {
    if (event.what == evKeyboard)
        {
        switch (event.keyDown.keyCode)
        {
        case kbHome:
            setCursor(0,0);
            break;
        case kbEnd:
            setCursor(size.x-1, size.y-1);
            break;
        case kbUp:
            if (cursor.y > 0)
            setCursor(cursor.x, cursor.y-1);
            break;
        case kbDown:
            if (cursor.y < size.y-1)
            setCursor(cursor.x, cursor.y+1);
            break;
        case kbLeft:
            if (cursor.x > 0)
            setCursor(cursor.x-1, cursor.y);
            break;
        case kbRight:
            if (cursor.x < size.x-1)
            setCursor(cursor.x+1, cursor.y);
                    break;
        default:
                    setCursor(event.keyDown.charScan.charCode % 32,
                      event.keyDown.charScan.charCode / 32);
                    break;
                }
            charFocused();
            clearEvent(event);
        }
        }
}


//
// TReport functions
//

const char * const TReport::name = "TReport";


void TReport::write( opstream& os )
{
    TView::write( os );
    os << asciiChar;
}


void *TReport::read( ipstream& is )
{
    TView::read( is );
    is >> asciiChar;
    return this;
}


TStreamable *TReport::build()
{
    return new TReport( streamableInit );
}


TStreamableClass RReport( TReport::name,
              TReport::build,
              __DELTA(TReport)
            );


TReport::TReport(TRect& r) :
 TView(r)
{
    asciiChar = 0;
}


void TReport::draw()
{
    TDrawBuffer buf;
    TColorAttr  color = getColor(6);
    char        str[80];
    ostrstream  statusStr( str, sizeof str );

    statusStr
      << "  Char: " << (char ) ((asciiChar == 0) ? 0x20 : asciiChar)
      << " Decimal: " << setw(3) << (int) asciiChar
      << " Hex " << hex << setiosflags(ios::uppercase)
      << setw(2) << (int) asciiChar << "     " << ends;

    buf.moveStr(0, str, color);
    writeLine(0, 0, 32, 1, buf);
}


void TReport::handleEvent(TEvent& event)
{
    TView::handleEvent(event);
    if (event.what == evBroadcast)
        {
        if (event.message.command == cmAsciiTableCmdBase + cmCharFocused)
            {
            asciiChar = event.message.infoByte;
            drawView();
            }
        }
}


//
// TAsciiChart functions
//

const char * const TAsciiChart::name = "TAsciiChart";


void TAsciiChart::write( opstream& os )
{
    TWindow::write( os );
}


void *TAsciiChart::read( ipstream& is )
{
    TWindow::read( is );
    return this;
}


TStreamable *TAsciiChart::build()
{
    return new TAsciiChart( streamableInit );
}


TStreamableClass RAsciiChart( TAsciiChart::name,
                  TAsciiChart::build,
                  __DELTA(TAsciiChart)
                );


TAsciiChart::TAsciiChart() :
    TWindowInit( &TAsciiChart::initFrame ),
    TWindow(TRect(0, 0, 34, 12), "ASCII Chart", wnNoNumber)
{
    TView *control;

    flags &= ~(wfGrow | wfZoom);
    growMode = 0;
    palette = wpGrayWindow;

    TRect r = getExtent();
    r.grow(-1, -1);
    r.a.y = r.b.y - 1;
    control = new TReport( r );
    control->options |= ofFramed;
    control->eventMask |= evBroadcast;
    insert(control);

    r = getExtent();
    r.grow(-1, -1);
    r.b.y = r.b.y - 2;
    control = new TTable( r );
    control->options |= ofFramed;
    control->options |= ofSelectable;
    control->blockCursor();
    insert(control);
    control->select();
}

void TAsciiChart::handleEvent( TEvent &event ) {
  TWindow::handleEvent( event );
}
