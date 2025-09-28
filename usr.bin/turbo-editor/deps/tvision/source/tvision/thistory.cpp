/*------------------------------------------------------------*/
/* filename -       thistory.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  THistory member functions                 */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_THistory
#define Uses_TKeys
#define Uses_TRect
#define Uses_TEvent
#define Uses_TInputLine
#define Uses_THistoryWindow
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __CTYPE_H )
#include <ctype.h>
#endif  // __CTYPE_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

#define cpHistory "\x16\x17"

THistory::THistory( const TRect& bounds,
                    TInputLine *aLink,
                    ushort aHistoryId ) noexcept :
    TView(bounds),
    link( aLink ),
    historyId( aHistoryId )
{
    options |= ofPostProcess;
    eventMask |= evBroadcast;
}

void THistory::shutDown()
{
    link = 0;
    TView::shutDown();
}

void THistory::draw()
{
    TDrawBuffer b;

    b.moveCStr( 0, icon, getColor(0x0102) );
    writeLine( 0, 0, size.x, size.y, b );
}

TPalette& THistory::getPalette() const
{
    static TPalette palette( cpHistory, sizeof( cpHistory )-1 );
    return palette;
}

void THistory::handleEvent( TEvent& event )
{
    THistoryWindow *historyWindow;
    TRect  r, p;
    ushort c;

    TView::handleEvent( event );
    if( event.what == evMouseDown ||
          ( event.what == evKeyDown &&
            ctrlToArrow( event.keyDown.keyCode ) ==  kbDown &&
            (link->state & sfFocused) != 0
          )
      )
        {
        if (!link->focus())
            {
            clearEvent(event);
            return;
            }
        recordHistory(link->data);
        r = link->getBounds();
        r.a.x--;
        r.b.x++;
        r.b.y += 7;
        r.a.y--;
        p = owner->getExtent();
        r.intersect( p );
        r.b.y--;
        historyWindow = initHistoryWindow( r );
        if( historyWindow != 0 )
            {
            c = owner->execView( historyWindow );
            if( c == cmOK )
                {
                char rslt[256];
                historyWindow->getSelection( rslt );
                strnzcpy( link->data, rslt, link->maxLen+1 );
                link->selectAll( True );
                link->drawView();
                }
            destroy( historyWindow );
            }
        clearEvent( event );
        }
    else
        if( event.what == evBroadcast )
            {
            if( (event.message.command == cmReleasedFocus &&
                 event.message.infoPtr ==  link) ||
                event.message.command ==  cmRecordHistory
              )
                recordHistory(link->data );
            }
}

THistoryWindow *THistory::initHistoryWindow( const TRect& bounds )
{
    THistoryWindow *p = new THistoryWindow( bounds, historyId );
    p->helpCtx = link->helpCtx;
    return p;
}

void THistory::recordHistory(const char* s)
{
    historyAdd(historyId, s);
}

#if !defined(NO_STREAMABLE)

void THistory::write( opstream& os )
{
    TView::write( os );
    os << link << historyId;
}

void *THistory::read( ipstream& is )
{
    TView::read( is );
    is >> link >> historyId;
    return this;
}

TStreamable *THistory::build()
{
    return new THistory( streamableInit );
}

THistory::THistory( StreamableInit ) noexcept : TView( streamableInit )
{
}

#endif
