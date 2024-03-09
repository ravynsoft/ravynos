/*---------------------------------------------------------*/
/*                                                         */
/*   Mousedlg.cpp : Member functions of following classes: */
/*                     TClickTester                        */
/*                     TMouseDialog                        */
/*                                                         */
/*---------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TRect
#define Uses_TStaticText
#define Uses_TEvent
#define Uses_TDrawBuffer
#define Uses_TDialog
#define Uses_TLabel
#define Uses_TScrollBar
#define Uses_TCheckBoxes
#define Uses_TButton
#define Uses_TSItem
#define Uses_TEventQueue
#include <tvision/tv.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <strstrea.h>
#include <iomanip.h>

#include "mousedlg.h"


#define cpMousePalette "\x07\x08"


//
// TClickTester functions
//

TClickTester::TClickTester(TRect& r, const char *aText) :
    TStaticText(r, aText)
{
    clicked = 0;
}


TPalette& TClickTester::getPalette() const
{
    static TPalette palette( cpMousePalette, sizeof(cpMousePalette)-1 );
    return palette;
}


void TClickTester::handleEvent(TEvent& event)
{
    TStaticText::handleEvent(event);

    if (event.what == evMouseDown)
        {
        if (event.mouse.eventFlags & meDoubleClick)
            {
            clicked = (short)((clicked) ? 0 : 1);
            drawView();
            }
        clearEvent(event);
        }
}


void TClickTester::draw()
{
    TDrawBuffer buf;
    TColorAttr c;

    if (clicked)
        c = getColor(2);
    else
        c = getColor(1);

    buf.moveChar(0, ' ', c, (short)size.x);
    buf.moveStr(0, text, c);
    writeLine(0, 0, (short)size.x, 1, buf);
}


//
// TMouseDialog functions
//

TMouseDialog::TMouseDialog() :
    TWindowInit( &TMouseDialog::initFrame ),
    TDialog( TRect(0, 0, 34, 12), "Mouse options" )
{
    TRect r(3, 4, 30, 5);

    options |= ofCentered;

    mouseScrollBar = new TScrollBar(r);
    mouseScrollBar->setParams(1, 1, 20, 20, 1);
    mouseScrollBar->options |= ofSelectable;
    mouseScrollBar->setValue(TEventQueue::doubleDelay);
    insert(mouseScrollBar);

    r = TRect(2, 2, 21, 3);
    insert(new TLabel(r, "~M~ouse double click", mouseScrollBar));

    r = TRect(3, 3, 30, 4);
    insert(new TClickTester(r, "Fast       Medium      Slow"));

    r = TRect(3, 6, 30, 7);
    insert(new TCheckBoxes(r, new TSItem("~R~everse mouse buttons", NULL)));
    oldDelay = TEventQueue::doubleDelay;

    r = TRect(9, 9, 19, 11);
    insert(new TButton(r, "O~K~", cmOK, bfDefault));

    r = TRect(21, 9, 31, 11);
    insert(new TButton(r, "Cancel", cmCancel, bfNormal));

    selectNext( (Boolean) 0);
}


void TMouseDialog::handleEvent(TEvent& event)
{
    TDialog::handleEvent(event);
    switch(event.what)
        {
        case evCommand:
            if(event.message.command == cmCancel)
                TEventQueue::doubleDelay = oldDelay;
            break;

        case evBroadcast:
            if(event.message.command == cmScrollBarChanged)
                {
                TEventQueue::doubleDelay = (short)mouseScrollBar->value;
                clearEvent(event);
                }
            break;
        }
}

