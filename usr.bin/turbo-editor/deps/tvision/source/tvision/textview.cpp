/*-------------------------------------------------------------*/
/* filename -       textview.cpp                               */
/*                                                             */
/* function(s)                                                 */
/*                  TTerminal and TTextDevice member functions */
/*-------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TTextDevice
#define Uses_TTerminal
#define Uses_otstream
#define Uses_TText
#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#include <malloc.h>

#pragma argsused
TTextDevice::TTextDevice( const TRect& bounds,
                          TScrollBar *aHScrollBar,
                          TScrollBar *aVScrollBar ) noexcept :
    TScroller(bounds, aHScrollBar, aVScrollBar)
{
}

int TTextDevice::overflow( int c )
{
    if( c != EOF )
        {
        char b = c;
        do_sputn( &b, 1 );
        }
    return 1;
}

#if !defined( __BORLANDC__ )
// The 'xsputn' method in modern STL is the equivalent of 'do_sputn' in
// Borland's RTL. We must invoke 'do_sputn' here in order to replicate
// the original behaviour. Otherwise, the default 'xsputn' will fall back on
// 'overflow' and the inserted characters will be processed one by one,
// which in the case of TTerminal may lead to performance issues.
std::streamsize TTextDevice::xsputn(const char *s, std::streamsize count)
{
    return (std::streamsize) do_sputn(s, (int) count);
}
#endif

TTerminal::TTerminal( const TRect& bounds,
                      TScrollBar *aHScrollBar,
                      TScrollBar *aVScrollBar,
                      ushort aBufSize ) noexcept :
    TTextDevice(bounds, aHScrollBar, aVScrollBar),
    queFront( 0 ),
    queBack( 0 )
{
    growMode = gfGrowHiX + gfGrowHiY;
    bufSize = min( 32000U, aBufSize );
    buffer = new char[ bufSize ];
    setLimit( 0, 1 );
    setCursor( 0, 0 );
    showCursor();
}


TTerminal::~TTerminal()
{
    delete[] buffer;
}

void TTerminal::bufDec( ushort& val )
{
    if (val == 0)
        val = bufSize - 1;
    else
        val--;
}

void TTerminal::bufInc( ushort& val )
{
    if( ++val >= bufSize )
        val = 0;
}

Boolean TTerminal::canInsert( ushort amount )
{
    long T = (queFront < queBack) ?
        ( queFront +  amount ) :
        ( long(queFront) - bufSize + amount);   // cast needed so we get
                                                // signed comparison
    return Boolean( queBack > T );
}

static void discardPossiblyTruncatedCharsAtEnd( const char (&s)[256],
                                                size_t &sLen )
{
#if !defined( __BORLANDC__ )
    if( sLen == sizeof(s) )
        {
        sLen = 0;
        while( sLen < sizeof(s) - (maxCharLength - 1) )
            sLen += TText::next( TStringView( &s[sLen], sizeof(s) - sLen ) );
        }
#else
    (void) s, (void) sLen;
#endif
}

void TTerminal::draw()
{
    TDrawBuffer b;
    char s[256];
    size_t sLen;
    int x, y;
    ushort begLine, endLine, linePos;
    ushort bottomLine;
    TColorAttr color = mapColor(1);

    setCursor( -1, -1 );

    bottomLine = size.y + delta.y;
    if( limit.y > bottomLine )
        {
        endLine = prevLines( queFront, limit.y - bottomLine );
        bufDec( endLine );
        }
    else
        endLine = queFront;

    if( limit.y > size.y )
        y = size.y - 1;
    else
        {
        for( y = limit.y; y < size.y; y++ )
            writeChar(0, y, ' ', 1, size.x);
        y = limit.y - 1;
        }

    for( ; y >= 0; y-- )
        {
        x = 0;
        begLine = prevLines(endLine, 1);
        linePos = begLine;
        while( linePos != endLine )
            {
            if( endLine >= linePos )
                {
                size_t cpyLen = min( endLine - linePos, sizeof(s) );
                memcpy( s, &buffer[linePos], cpyLen );
                sLen = cpyLen;
                }
            else
                {
                size_t fstCpyLen = min( bufSize - linePos, sizeof(s) );
                size_t sndCpyLen = min( endLine, sizeof(s) - fstCpyLen );
                memcpy( s, &buffer[linePos], fstCpyLen );
                memcpy( &s[fstCpyLen], buffer, sndCpyLen );
                sLen = fstCpyLen + sndCpyLen;
                }

            discardPossiblyTruncatedCharsAtEnd(s, sLen);
            if( linePos >= bufSize - sLen )
                linePos = sLen - (bufSize - linePos);
            else
                linePos += sLen;

            x += b.moveStr( x, TStringView( s, sLen ), color );
            }

        b.moveChar( x, ' ', color, max( size.x - x, 0 ) );
        writeBuf( 0, y, size.x, 1, b );
        // Draw the cursor when this is the last line.
        if( endLine == queFront )
            setCursor( x, y );
        endLine = begLine;
        bufDec( endLine );
        }
}

ushort TTerminal::nextLine( ushort pos )
{
    while( pos != queFront && buffer[pos] != '\n' )
        bufInc( pos );
    if( pos != queFront )
        bufInc( pos );
    return pos;
}

int TTerminal::do_sputn( const char *s, int count )
{
    ushort screenLines = limit.y;
    ushort i;

    if( count > bufSize - 1 )
        {
        s += count - (bufSize - 1);
        count = bufSize - 1;
        }

    for( i = 0; i < count; i++ )
        if( s[i] == '\n' )
            screenLines++;

    while( !canInsert( count ) )
        {
        queBack = nextLine( queBack );
        if( screenLines > 1 )
            screenLines--;
        }

    if( queFront + count >= bufSize )
        {
        i = bufSize - queFront;
        memcpy( &buffer[queFront], s, i );
        memcpy( buffer, &s[i], count - i );
        queFront = count - i;
        }
    else
        {
        memcpy( &buffer[queFront], s, count );
        queFront += count;
        }

    // drawLock: avoid redundant calls to drawView()
    drawLock++;
    setLimit( limit.x, screenLines );
    scrollTo( 0, screenLines + 1 );
    drawLock--;

    drawView();
    return count;
}

Boolean TTerminal::queEmpty()
{
    return Boolean( queBack == queFront );
}

otstream::otstream( TTerminal *tt ) :
    ostream(tt)
{
}
