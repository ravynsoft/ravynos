/*------------------------------------------------------------*/
/* filename -       thstview.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  THistoryViewer member functions           */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_THistoryViewer
#define Uses_TScrollBar
#define Uses_TEvent
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

#define cpHistoryViewer "\x06\x06\x07\x06\x06"

THistoryViewer::THistoryViewer( const TRect& bounds,
                                TScrollBar *aHScrollBar,
                                TScrollBar *aVScrollBar,
                                ushort aHistoryId ) noexcept :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    historyId( aHistoryId )
{
    setRange( historyCount( aHistoryId ) );
    if( range > 1 )
        focusItem( 1 );
    hScrollBar->setRange( 0, historyWidth() - size.x + 3 );
}

TPalette& THistoryViewer::getPalette() const
{
    static TPalette palette( cpHistoryViewer, sizeof( cpHistoryViewer )-1 );
    return palette;
}

void THistoryViewer::getText( char *dest, short item, short maxChars )
{
    const char *str = historyStr( historyId, item );
    if( str != 0 )
        {
        strncpy( dest, str, maxChars );
        dest[maxChars] = '\0';
        }
    else
        *dest = EOS;
}

void THistoryViewer::handleEvent( TEvent& event )
{
    if( (event.what == evMouseDown && (event.mouse.eventFlags & meDoubleClick) ) ||
        (event.what == evKeyDown && event.keyDown.keyCode == kbEnter)
      )
        {
        endModal( cmOK );
        clearEvent( event );
        }
    else
        if( (event.what ==  evKeyDown && event.keyDown.keyCode == kbEsc) ||
            (event.what ==  evCommand && event.message.command ==  cmCancel)
          )
            {
            endModal( cmCancel );
            clearEvent( event );
            }
        else
            TListViewer::handleEvent( event );
}

int THistoryViewer::historyWidth() noexcept
{
    int width = 0;
    int count = historyCount( historyId );
    for( int i = 0; i < count; i++ )
        {
        int T = strwidth( historyStr( historyId, i ) );
        width = max( width, T );
        }
    return width;
}
