/*------------------------------------------------------------*/
/* filename -       tbkgrnd.cpp                               */
/*                                                            */
/* function(s)                                                */
/*          TBackground member functions                      */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TBackground
#define Uses_TDrawBuffer
#define Uses_opstream
#define Uses_ipstream

#include <tvision/tv.h>

#define cpBackground "\x01"      // background palette

TBackground::TBackground( const TRect& bounds, char aPattern ) noexcept :
    TView(bounds),
    pattern( aPattern )
{
    growMode = gfGrowHiX | gfGrowHiY;
}

void TBackground::draw()
{
    TDrawBuffer b;

    b.moveChar( 0, pattern, getColor(0x01), size.x );
    writeLine( 0, 0, size.x, size.y, b );
}

TPalette& TBackground::getPalette() const
{
    static TPalette palette( cpBackground, sizeof( cpBackground )-1 );
    return palette;
}

#if !defined(NO_STREAMABLE)

TBackground::TBackground( StreamableInit ) noexcept : TView( streamableInit )
{
}

void TBackground::write( opstream& os )
{
    TView::write( os );
    os << pattern;
}

void *TBackground::read( ipstream& is )
{
    TView::read( is );
    is >> pattern;
    return this;
}

TStreamable *TBackground::build()
{
    return new TBackground( streamableInit );
}

#endif
