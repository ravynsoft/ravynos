/*------------------------------------------------------------*/
/* filename - teditwnd.cpp                                    */
/*                                                            */
/* function(s)                                                */
/*            TEditWindow member functions                    */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TFrame
#define Uses_TWindow
#define Uses_TRect
#define Uses_TIndicator
#define Uses_TEditor
#define Uses_TMemo
#define Uses_TFileEditor
#define Uses_TEditWindow
#define Uses_TEvent
#define Uses_TScrollBar
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

const TPoint minEditWinSize = {24, 6};

TEditWindow::TEditWindow( const TRect& bounds,
                          TStringView fileName,
                          int aNumber
                        ) noexcept :
    TWindowInit( &TEditWindow::initFrame ),
    TWindow( bounds, 0, aNumber )
{
    options |= ofTileable;

    TScrollBar *hScrollBar =
        new TScrollBar( TRect( 18, size.y - 1, size.x - 2, size.y ) );
    hScrollBar->hide();
    insert(hScrollBar);

    TScrollBar *vScrollBar =
        new TScrollBar( TRect( size.x - 1, 1, size.x, size.y - 1 ) );
    vScrollBar->hide();
    insert(vScrollBar);

    TIndicator *indicator =
        new TIndicator( TRect( 2, size.y - 1, 16, size.y ) );
    indicator->hide();
    insert(indicator);


    TRect r( getExtent() );
    r.grow(-1, -1);
    editor = new TFileEditor( r, hScrollBar, vScrollBar, indicator, fileName );
    insert(editor);
}

void TEditWindow::close()
{
    if( editor->isClipboard() == True )
        hide();
    else
        TWindow::close();
}

const char *TEditWindow::getTitle( short )
{
    if( editor->isClipboard() == True )
        return clipboardTitle;
    else if( *(editor->fileName) == EOS )
        return untitled;
    else
        return editor->fileName;
}

void TEditWindow::handleEvent( TEvent& event )
{
    TWindow::handleEvent(event);
    if( event.what == evBroadcast && event.message.command == cmUpdateTitle )
        {
        if( frame != 0 )
            frame->drawView();
        clearEvent(event);
        }
}

void TEditWindow::sizeLimits( TPoint& min, TPoint& max )
{
    TWindow::sizeLimits(min, max);
    min = minEditWinSize;
}

#if !defined(NO_STREAMABLE)

void TEditWindow::write( opstream& os )
{
    TWindow::write( os );
    os << editor;
}

void *TEditWindow::read( ipstream& is )
{
    TWindow::read( is );
    is >> editor;
    return this;
}

TStreamable *TEditWindow::build()
{
    return new TEditWindow( streamableInit );
}

TEditWindow::TEditWindow( StreamableInit ) noexcept :
    TWindowInit( 0 ),
    TWindow( streamableInit )
{
}

#endif
