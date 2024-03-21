/*------------------------------------------------------------*/
/* filename -       twindow.cpp                               */
/*                                                            */
/* function(s)                                                */
/*                  TWindow member functions                  */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TWindow
#define Uses_TEvent
#define Uses_TRect
#define Uses_TFrame
#define Uses_TCommandSet
#define Uses_TScrollBar
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

const TPoint minWinSize = {16, 6};

TWindowInit::TWindowInit( TFrame *(*cFrame)( TRect ) ) noexcept :
    createFrame( cFrame )
{
}

TWindow::TWindow( const TRect& bounds,
                  TStringView aTitle,
                  short aNumber
                ) noexcept :
    TWindowInit( &TWindow::initFrame ),
    TGroup( bounds ),
    flags( wfMove | wfGrow | wfClose | wfZoom ),
    zoomRect( getBounds() ),
    number( aNumber ),
    palette( wpBlueWindow ),
    title( newStr( aTitle ) )
{
    state |= sfShadow;
    options |= ofSelectable | ofTopSelect;
    growMode = gfGrowAll | gfGrowRel;

    if( createFrame != 0 &&
        (frame = createFrame( getExtent() )) != 0
      )
        insert( frame );
}

TWindow::~TWindow()
{
    delete[] (char *)title;
}

void TWindow::close()
{
    if( valid( cmClose ) )
    {
        frame = 0;  // so we don't try to use the frame after it's been deleted
        destroy( this );
    }
}

void TWindow::shutDown()
{
    frame = 0;
    TGroup::shutDown();
}

TPalette& TWindow::getPalette() const
{
    static TPalette blue( cpBlueWindow, sizeof( cpBlueWindow )-1 );
    static TPalette cyan( cpCyanWindow, sizeof( cpCyanWindow )-1 );
    static TPalette gray( cpGrayWindow, sizeof( cpGrayWindow )-1 );
    static TPalette *palettes[] =
        {
        &blue,
        &cyan,
        &gray
        };
    return *(palettes[palette]);
}

const char *TWindow::getTitle( short )
{
    return title;
}

void TWindow::handleEvent( TEvent& event )
{
 TRect  limits;
 TPoint min, max;

    TGroup::handleEvent(event);
    if( event.what== evCommand )
        switch (event.message.command)
            {
            case  cmResize:
                if( (flags & (wfMove | wfGrow)) != 0 )
                    {
                    limits = owner->getExtent();
                    sizeLimits(min, max);
                    dragView( event, dragMode | (flags & (wfMove | wfGrow)),
                              limits, min, max);
                    clearEvent(event);
                    }
                break;
            case  cmClose:
                if( (flags & wfClose) != 0 &&
                    ( event.message.infoPtr == 0 || event.message.infoPtr == this )
                  )
                    {
                    clearEvent(event);
                    if( (state & sfModal) == 0 )
                        close();
                    else
                        {
                        event.what = evCommand;
                        event.message.command = cmCancel;
                        putEvent( event );
                        clearEvent( event );
                        }
                    }
                break;
            case  cmZoom:
                if( (flags & wfZoom) != 0 &&
                    (event.message.infoPtr == 0 || event.message.infoPtr == this)
                  )
                    {
                    zoom();
                    clearEvent(event);
                    }
                break;
            }
    else if( event.what == evKeyDown )
            switch (event.keyDown.keyCode)
                {
                case  kbTab:
                    focusNext(False);
                    clearEvent(event);
                    break;
                case  kbShiftTab:
                    focusNext(True);
                    clearEvent(event);
                    break;
                }
    else if( event.what == evBroadcast &&
             event.message.command == cmSelectWindowNum &&
             event.message.infoInt == number &&
             (options & ofSelectable) != 0
           )
            {
            select();
            clearEvent(event);
            }
}

TFrame *TWindow::initFrame( TRect r )
{
    return new TFrame(r);
}

void TWindow::setState( ushort aState, Boolean enable )
{
    TCommandSet windowCommands;

    TGroup::setState(aState, enable);
    if( (aState & sfSelected) != 0 )
        {
        setState(sfActive, enable);
        if( frame != 0 )
            frame->setState(sfActive,enable);
        windowCommands += cmNext;
        windowCommands += cmPrev;
        if( (flags & (wfGrow | wfMove)) != 0 )
            windowCommands += cmResize;
        if( (flags & wfClose) != 0 )
            windowCommands += cmClose;
        if( (flags & wfZoom) != 0 )
            windowCommands += cmZoom;
        if( enable != False )
            enableCommands(windowCommands);
        else
            disableCommands(windowCommands);
        }
}

TScrollBar *TWindow::standardScrollBar( ushort aOptions ) noexcept
{
    TRect r = getExtent();
    if( (aOptions & sbVertical) != 0 )
        r = TRect( r.b.x-1, r.a.y+1, r.b.x, r.b.y-1 );
    else
        r = TRect( r.a.x+2, r.b.y-1, r.b.x-2, r.b.y );

    TScrollBar *s;
    insert( s = new TScrollBar(r) );
    if( (aOptions & sbHandleKeyboard) != 0 )
        s->options |= ofPostProcess;
    return s;
}

void TWindow::sizeLimits( TPoint& min, TPoint& max )
{
    TView::sizeLimits(min, max);
    min = minWinSize;
}

void TWindow::zoom()
{
    TPoint minSize, maxSize;
    sizeLimits( minSize, maxSize );
    if( size != maxSize )
        {
        zoomRect = getBounds();
        TRect r( 0, 0, maxSize.x, maxSize.y );
        locate(r);
        }
    else
        locate( zoomRect );
}

#if !defined(NO_STREAMABLE)

void TWindow::write( opstream& os )
{
    TGroup::write( os );
    os << flags << zoomRect << number << palette;
    os << frame;
    os.writeString( title );
}

void *TWindow::read( ipstream& is )
{
    TGroup::read( is );
    is >> flags >> zoomRect >> number >> palette;
    is >> frame;
    title = is.readString();
    return this;
}

TStreamable *TWindow::build()
{
    return new TWindow( streamableInit );
}

TWindow::TWindow( StreamableInit ) noexcept :
    TWindowInit( 0 ),
    TGroup( streamableInit )
{
}

#endif
