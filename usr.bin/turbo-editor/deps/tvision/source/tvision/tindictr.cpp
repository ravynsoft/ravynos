/*------------------------------------------------------------*/
/* filename - tindictr.cpp                                    */
/*                                                            */
/* function(s)                                                */
/*            TIndicator member functions                     */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TIndicator
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TView
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __STRSTREA_H )
#include <strstrea.h>
#endif	// __STRSTREA_H

#define cpIndicator "\x02\x03"

TIndicator::TIndicator( const TRect& bounds ) noexcept :
    TView( bounds ),
    location( TPoint() ),
    modified( False )
{
    growMode = gfGrowLoY | gfGrowHiY;
}

void TIndicator::draw()
{
    TColorAttr color;
    char frame;
    TDrawBuffer b;
    char s[15];

    if( (state & sfDragging) == 0 )
        {
        color = getColor(1);
        frame = dragFrame;
        }
    else
        {
        color = getColor(2);
        frame = normalFrame;
        }

    b.moveChar( 0, frame, color, size.x );
    if( modified )
        b.putChar( 0, 15 );
    ostrstream os( s, 15 );

    os << ' ' << (location.y+1)
       << ':' << (location.x+1) << ' ' << ends;

    b.moveStr( 8-int(strchr(s, ':')-s), s, color);
    writeBuf(0, 0, size.x, 1, b);
}

TPalette& TIndicator::getPalette() const
{
    static TPalette palette( cpIndicator, sizeof( cpIndicator )-1 );
    return palette;
}

void TIndicator::setState( ushort aState, Boolean enable )
{
    TView::setState(aState, enable);
    if( aState == sfDragging )
        drawView();
}

void TIndicator::setValue( const TPoint& aLocation, Boolean aModified )
{
    if( (location !=  aLocation) || (modified != aModified) )
        {
        location = aLocation;
        modified = aModified;
        drawView();
        }
}

#if !defined(NO_STREAMABLE)

TStreamable *TIndicator::build()
{
    return new TIndicator( streamableInit );
}

TIndicator::TIndicator( StreamableInit ) noexcept : TView( streamableInit )
{
}

#endif
