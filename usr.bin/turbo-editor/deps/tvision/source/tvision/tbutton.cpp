/*------------------------------------------------------------*/
/* filename -       tbutton.cpp                               */
/*                                                            */
/* function(s)                                                */
/*          TButton member functions                          */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TButton
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TRect
#define Uses_TGroup
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

const int

    cmGrabDefault    = 61,
    cmReleaseDefault = 62;

#define cpButton "\x0A\x0B\x0C\x0D\x0E\x0E\x0E\x0F"

TButton::TButton( const TRect& bounds,
                  TStringView aTitle,
                  ushort aCommand,
                  ushort aFlags ) noexcept :
    TView( bounds ),
    title( newStr( aTitle ) ),
    command( aCommand ),
    flags( aFlags ),
    amDefault( Boolean( (aFlags & bfDefault) != 0 ) ),
    animationTimer( 0 )
{
    options |= ofSelectable | ofFirstClick | ofPreProcess | ofPostProcess;
    eventMask |= evBroadcast;
    if( !commandEnabled(aCommand) )
        state |= sfDisabled;
}

TButton::~TButton()
{
    delete[] (char *)title;
}

void TButton::draw()
{
    drawState(False);
}

void TButton::drawTitle( TDrawBuffer &b,
                         int s,
                         int i,
                         TAttrPair cButton,
                         Boolean down
                       )
{
    int l, scOff;
    if( (flags & bfLeftJust) != 0 )
        l = 1;
    else
        {
        l = (s - cstrlen(title) - 1)/2;
        if( l < 1 )
            l = 1;
        }
    b.moveCStr( i+l, title, cButton );

    if( showMarkers == True && !down )
        {
        if( (state & sfSelected) != 0 )
            scOff = 0;
        else if( amDefault )
            scOff = 2;
        else
            scOff = 4;
        b.putChar( 0, specialChars[scOff] );
        b.putChar( s, specialChars[scOff+1] );
        }
}

void TButton::drawState(Boolean down)
{
    TAttrPair cButton, cShadow;
    TDrawBuffer b;

    if( (state & sfDisabled) != 0 )
        cButton = getColor(0x0404);
    else
        {
        cButton = getColor(0x0501);
        if( (state & sfActive) != 0 )
            {
            if( (state & sfSelected) != 0 )
                cButton = getColor(0x0703);
            else if( amDefault )
                cButton = getColor(0x0602);
            }
        }
    cShadow = getColor(8);
    int s = size.x-1;
    int T = size.y / 2 - 1;
    char ch = ' ';
    for( int y = 0; y <= size.y-2; y++ )
        {
        int i;
        b.moveChar( 0, ' ', cButton, size.x );
        b.putAttribute( 0, cShadow );
        if( down )
            {
            b.putAttribute( 1, cShadow );
            ch =  ' ';
            i = 2;
            }
        else
            {
            b.putAttribute( s, cShadow );
            if( showMarkers == True )
                ch = ' ';
            else
                {
                if( y == 0 )
                    b.putChar( s, shadows[0] );
                else
                    b.putChar( s, shadows[1] );
                ch = shadows[2];
                }
            i =  1;
            }

        if( y == T && title != 0 )
            drawTitle( b, s, i, cButton, down );

        if( showMarkers && !down )
            {
            b.putChar( 1, markers[0] );
            b.putChar( s-1, markers[1] );
            }
        writeLine( 0, y, size.x, 1, b );
        }
    b.moveChar( 0, ' ', cShadow, 2 );
    b.moveChar( 2, ch, cShadow, s-1 );
    writeLine( 0, size.y-1, size.x, 1, b );
}

TPalette& TButton::getPalette() const
{
    static TPalette palette( cpButton, sizeof( cpButton )-1 );
    return palette;
}

void TButton::handleEvent( TEvent& event )
{
    TPoint mouse;
    TRect clickRect;

    clickRect = getExtent();
    clickRect.a.x++;
    clickRect.b.x--;
    clickRect.b.y--;

    if( event.what == evMouseDown )
        {
        mouse = makeLocal( event.mouse.where );
        if( !clickRect.contains(mouse) )
            clearEvent( event );
        }
    if (flags & bfGrabFocus)
        TView::handleEvent(event);

    char c = hotKey( title );
    switch( event.what )
        {
        case evMouseDown:
            if ((state & sfDisabled) == 0)
                {
                clickRect.b.x++;
                Boolean down = False;
                do  {
                    mouse = makeLocal( event.mouse.where );
                    if( down != clickRect.contains( mouse ) )
                        {
                        down = Boolean( !down );
                        drawState( down );
                        }
                    } while( mouseEvent( event, evMouseMove ) );
                if( down )
                    {
                    press();
                    drawState( False );
                    }
                }
            clearEvent( event );
            break;

        case evKeyDown:
            if( event.keyDown.keyCode != 0 &&
                ( event.keyDown.keyCode == getAltCode(c) ||
                  ( owner->phase == phPostProcess &&
                    c != 0 &&
                    toupper(event.keyDown.charScan.charCode) == c
                  ) ||
                  ( (state & sfFocused) != 0 &&
                    event.keyDown.charScan.charCode == ' '
                  )
                )
              )
                {
                drawState( True );
                if( animationTimer != 0 )
                    press();
                animationTimer = setTimer( animationDuration );
                clearEvent( event );
                }
            break;

        case evBroadcast:
            switch( event.message.command )
                {
                case cmDefault:
                    if( amDefault && !(state & sfDisabled) )
                        {
                        press();
                        clearEvent(event);
                        }
                    break;

                case cmGrabDefault:
                case cmReleaseDefault:
                    if( (flags & bfDefault) != 0 )
                        {
                        amDefault = Boolean(event.message.command == cmReleaseDefault);
                        drawView();
                        }
                    break;

                case cmCommandSetChanged:
                    setState(sfDisabled,Boolean(!commandEnabled(command)));
                    drawView();
                    break;

                case cmTimeout:
                    if( animationTimer != 0 && event.message.infoPtr == animationTimer )
                        {
                        animationTimer = 0;
                        drawState( False );
                        press();
                        clearEvent( event );
                        }
                    break;
                }
        break;
        }
}

void TButton::makeDefault( Boolean enable )
{
    if( (flags & bfDefault) == 0 )
        {
        message( owner,
                 evBroadcast,
                 (enable == True) ? cmGrabDefault : cmReleaseDefault,
                 this
               );
        amDefault = enable;
        drawView();
        }
}

void TButton::setState( ushort aState, Boolean enable )
{
    TView::setState(aState, enable);
    if( aState & (sfSelected | sfActive) )
        drawView();
    if( (aState & sfFocused) != 0 )
        makeDefault( enable );
}

void TButton::press()
{
    message( owner, evBroadcast, cmRecordHistory, 0 );
    if( (flags & bfBroadcast) != 0 )
        message( owner, evBroadcast, command, this );
    else
        {
        TEvent e;
        e.what = evCommand;
        e.message.command = command;
        e.message.infoPtr = this;
        putEvent( e );
        }
}

#if !defined(NO_STREAMABLE)

void TButton::write( opstream& os )
{
    TView::write( os );
    os.writeString( title );
    os << command << flags << (int)amDefault;
}

void *TButton::read( ipstream& is )
{
    TView::read( is );
    title = is.readString();
    int temp;
    is >> command >> flags >> temp;
    amDefault = Boolean(temp);
    if( TButton::commandEnabled( command ) )
        state &= ~sfDisabled;
    else
        state |= sfDisabled;
    return this;
}

TStreamable *TButton::build()
{
    return new TButton( streamableInit );
}

#endif
