/*------------------------------------------------------------*/
/* filename -       tparamte.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TParamText member functions               */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TParamText
#define Uses_ipstream
#define Uses_opstream
#include <tvision/tv.h>

#if !defined( __STDIO_H )
#include <stdio.h>
#endif  // __STDIO_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

TParamText::TParamText( const TRect& bounds ) noexcept :
    TStaticText(bounds, 0 ),
    str( new char [256] )
{
    str[0] = EOS;
}

TParamText::~TParamText()
{
    delete[] (char*) str;
}

void TParamText::getText( char *s )
{
    if( str != 0 )
        strcpy( s, str );
    else
        *s = EOS;
}

int TParamText::getTextLen()
{
    return (str != 0) ? strlen( str ) : 0;
}

void TParamText::setText( const char *fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );
    vsnprintf( str, 256, fmt, ap );
    va_end( ap );

    drawView();
}

#if !defined(NO_STREAMABLE)

void TParamText::write( opstream& os )
{
    TStaticText::write( os );
    os.writeString(str);
}

void *TParamText::read( ipstream& is )
{
    TStaticText::read( is );
    str = new char [256];
    is.readString(str, 256);
    return this;
}

TStreamable *TParamText::build()
{
    return new TParamText( streamableInit );
}

TParamText::TParamText( StreamableInit ) noexcept : TStaticText( streamableInit )
{
}

#endif
