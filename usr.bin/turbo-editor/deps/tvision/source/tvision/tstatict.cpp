/*------------------------------------------------------------*/
/* filename -       tstatict.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TStaticText member functions              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TStaticText
#define Uses_TDrawBuffer
#define Uses_opstream
#define Uses_ipstream
#define Uses_TText
#include <tvision/tv.h>

#if !defined( __CTYPE_H )
#include <ctype.h>
#endif  // __CTYPE_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#define cpStaticText "\x06"

TStaticText::TStaticText( const TRect& bounds, TStringView aText ) noexcept :
    TView( bounds ),
    text( newStr( aText ) )
{
    growMode |= gfFixed;
}

TStaticText::~TStaticText()
{
    delete[] (char *)text;
}

void TStaticText::draw()
{
    TColorAttr color;
    Boolean center;
    int i, j, l, p, y;
    TDrawBuffer b;
    char buf[256];
    TStringView s;

    color = getColor(1);
    getText(buf);
    s = buf;
    // If possible, read directly from 'text' to prevent truncation.
    if (s == TStringView(text).substr(0, s.size()))
        s = text;
    l = (int) s.size();
    p = 0;
    y = 0;
    center = False;
    while (y < size.y)
        {
        b.moveChar(0, ' ', color, size.x);
        if (p < l)
            {
            if (s[p] == 3)
                {
                center = True;
                ++p;
                }
            i = p;
            int last = i + TText::scroll(s.substr(i), size.x, False);
            do {
                j = p;
                while ((p < l) && (s[p] == ' '))
                    ++p;
                while ((p < l) && (s[p] != ' ') && (s[p] != '\n'))
                    p += TText::next(s.substr(p));
                } while ((p < l) && (p < last) && (s[p] != '\n'));
            if (p > last)
                {
                if (j > i)
                    p = j;
                else
                    p = last;
                }
            int width = strwidth(s.substr(i, p-i));
            if (center == True)
                j = (size.x - width) / 2 ;
            else
                j = 0;
            b.moveStr(j, s.substr(i), color, (ushort) width);
            while ((p < l) && (s[p] == ' '))
                p++;
            if ((p < l) && (s[p] == '\n'))
                {
                center = False;
                p++;
                }
            }
        writeLine(0, y++, size.x, 1, b);
        }
}

TPalette& TStaticText::getPalette() const
{
    static TPalette palette( cpStaticText, sizeof( cpStaticText )-1 );
    return palette;
}

void TStaticText::getText( char *s )
{
    if( text == 0 )
        *s = EOS;
    else
    {
        strncpy( s, text, 255 );
        s[255] = EOS;
    }
}

#if !defined(NO_STREAMABLE)

void TStaticText::write( opstream& os )
{
    TView::write( os );
    os.writeString( text );
}

void *TStaticText::read( ipstream& is )
{
    TView::read( is );
    text = is.readString();
    return this;
}

TStreamable *TStaticText::build()
{
    return new TStaticText( streamableInit );
}

TStaticText::TStaticText( StreamableInit ) noexcept : TView( streamableInit )
{
}

#endif
