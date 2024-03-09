/*------------------------------------------------------------------------*/
/* filename - drivers.cpp                                                 */
/*                                                                        */
/* function(s)                                                            */
/*        moveBuf  --   moves a buffer of char/attribute pairs            */
/*        moveChar --   sets a buffer with a char/attribute pair          */
/*        moveCStr --   moves a char array into a buffer & adds an        */
/*                      attribute to each char                            */
/*------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TDrawBuffer
#define Uses_TScreen
#define Uses_TText
#include <tvision/tv.h>

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

#include <string.h>

#if !defined( __BORLANDC__ )
#define register
#endif

#pragma warn -asc

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveBuf                                                  */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent - character position within the buffer where the data      */
/*               is to go                                                 */
/*                                                                        */
/*      source - far pointer to an array of characters                    */
/*                                                                        */
/*      attr   - attribute to be used for all characters (0 to retain     */
/*               the attribute from 'source')                             */
/*                                                                        */
/*      count   - number of characters to move                            */
/*                                                                        */
/*------------------------------------------------------------------------*/

void TDrawBuffer::moveBuf( ushort indent, const void _FAR *source,
                           TColorAttr attr, ushort count ) noexcept

{
#if !defined( __FLAT__ )

I   MOV     CX, count
I   JCXZ    __5
I   PUSH    DS

    _ES = FP_SEG( &data[indent] );
    _DI = FP_OFF( &data[indent] );

//    _DS = FP_SEG( source );
//    _SI = FP_OFF( source );
I   LDS     SI, source

I   MOV     AH, [BYTE PTR attr]
I   CLD
I   OR      AH, AH
I   JE      __3

__1:

I   LODSB
I   STOSW
I   LOOP    __1
I   JMP     __4

__2:

I   INC     DI

__3:

I   MOVSB
I   LOOP    __2

__4:

I   POP     DS

__5:
        ;
#else
    moveStr(indent, TStringView((const char*) source, count), attr);
#endif
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveChar                                                 */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      c       - character to be put into the buffer (0 to retain the    */
/*                already present characters)                             */
/*                                                                        */
/*      attr    - attribute to be put into the buffer (0 to retain the    */
/*                already present attributes)                             */
/*                                                                        */
/*      count   - number of character/attribute pairs to put into the     */
/*                buffer                                                  */
/*                                                                        */
/*  Comments:                                                             */
/*                                                                        */
/*      If both 'c' and 'attr' are 0, the attributes are retained         */
/*      but the characters are not.                                       */
/*                                                                        */
/*------------------------------------------------------------------------*/

void TDrawBuffer::moveChar( ushort indent, char c, TColorAttr attr, ushort count ) noexcept
{
#if !defined( __FLAT__ )
I   MOV     CX,count
I   JCXZ    __4

    _ES = FP_SEG( &data[indent] );
    _DI = FP_OFF( &data[indent] );

I   MOV     AL,c
I   MOV     AH,[BYTE PTR attr]
I   CLD
I   OR      AL,AL
I   JE      __1
I   OR      AH,AH
I   JE      __3
I   REP     STOSW
I   JMP     __4

__1:

I   MOV     AL,AH

__2:

I   INC     DI

__3:

I   STOSB
I   LOOP    __2

__4:
    ;

#else
    register TScreenCell *dest = &data[indent];
    count = min(count, max(length() - indent, 0));

    if (attr != 0)
        if (c != 0)
        {
            TScreenCell cell;
            ::setCell(cell, (uchar) c, attr);
            while (count--)
                *dest++ = cell;
        }
        else
            while(count--)
                ::setAttr(*dest++, attr);
    else
        while (count--)
            ::setChar(*dest++, (uchar) c);
#endif
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveCStr                                                 */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      str     - string of characters to be moved into the buffer        */
/*                                                                        */
/*      attrs   - pair of text attributes to be put into the buffer       */
/*                with each character in the string.  Initially the       */
/*                low byte is used, and a '~' in the string toggles       */
/*                between the low byte and the high byte.                 */
/*                                                                        */
/*  returns:                                                              */
/*                                                                        */
/*      actual number of display columns that were filled with text.      */
/*                                                                        */
/*------------------------------------------------------------------------*/

ushort TDrawBuffer::moveCStr( ushort indent, TStringView str, TAttrPair attrs ) noexcept
{
#ifdef __BORLANDC__
    register ushort *dest = &data[indent];
    ushort *limit = &data[length()];
    register uchar _FAR *s = (uchar _FAR *) str.data();
    ushort count = (ushort) str.size();
    int toggle = 1;
    uchar curAttr = ((uchar *)&attrs)[0];

    for (; dest < limit && count; --count, ++s)
        {
        uchar c = *s;
        if (c == '~')
            {
            curAttr = ((uchar *)&attrs)[toggle];
            toggle = 1-toggle;
            }
        else
            {
            ((uchar*)dest)[0] = c;
            ((uchar*)dest)[1] = curAttr;
            dest++;
            }
        }
    return dest - &data[indent];
#else
    size_t i = indent, j = 0;
    int toggle = 1;
    auto curAttr = attrs[0];

    while (j < str.size())
        if (str[j] == '~')
            {
            curAttr = attrs[toggle];
            toggle = 1 - toggle;
            ++j;
            }
        else if (!TText::drawOne(data, i, str, j, curAttr))
            break;
    return i - indent;
#endif
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveCStr (2)                                             */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      str     - string of characters to be moved into the buffer        */
/*                                                                        */
/*      attrs   - pair of text attributes to be put into the buffer       */
/*                with each character in the string.  Initially the       */
/*                low byte is used, and a '~' in the string toggles       */
/*                between the low byte and the high byte.                 */
/*                                                                        */
/*      width   - number of display columns to be copied from str.        */
/*                                                                        */
/*      begin   - initial display column in str where to start counting.  */
/*                                                                        */
/*  returns:                                                              */
/*                                                                        */
/*      actual number of display columns that were filled with text.      */
/*                                                                        */
/*------------------------------------------------------------------------*/

ushort TDrawBuffer::moveCStr( ushort indent, TStringView str, TAttrPair attrs, ushort width, ushort begin ) noexcept
{
    size_t i = indent, j = 0, w = 0;
    int toggle = 1;
    TColorAttr curAttr = ((TColorAttr *) &attrs)[0];
    TSpan<TScreenCell> span(&data[0], min(indent + width, length()));
    while (j < str.size())
        if (str[j] == '~')
            {
            curAttr = ((TColorAttr *) &attrs)[toggle];
            toggle = 1 - toggle;
            ++j;
            }
        else
            {
            if (begin <= w)
                {
                if (!TText::drawOne(span, i, str, j, curAttr))
                    break;
                }
            else
                {
                if (!TText::next(str, j, w))
                    break;
                if (begin < w && i < span.size())
                    // 'begin' is in the middle of a double-width character.
                    ::setCell(span[i++], ' ', curAttr);
                }
            }
    return i - indent;
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveStr                                                  */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      str     - string of characters to be moved into the buffer        */
/*                                                                        */
/*      attr    - text attribute to be put into the buffer with each      */
/*                character in the string.                                */
/*                                                                        */
/*  returns:                                                              */
/*                                                                        */
/*      actual number of display columns that were filled with text.      */
/*                                                                        */
/*------------------------------------------------------------------------*/

ushort TDrawBuffer::moveStr( ushort indent, TStringView str, TColorAttr attr ) noexcept
{
#ifdef __BORLANDC__
    if (indent < length())
        {
        register ushort *dest = &data[indent];
        register uchar _FAR *s = (uchar _FAR *) str.data();
        ushort count = min(str.size(), length() - indent);
        ushort remain = count;

        if (attr != 0)
            for (; remain; --remain, ++s, ++dest)
                {
                ((uchar*)dest)[0] = *s;
                ((uchar*)dest)[1] = (uchar)attr;
                }
        else
            for (; remain; --remain, ++s, ++dest)
                *(uchar *)dest = *s;
        return count;
        }
    return 0;
#else
    if (attr != 0)
        return TText::drawStr(data, indent, str, 0, attr);
    else
        return TText::drawStr(data, indent, str, 0);
#endif
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveStr (2)                                              */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      str     - string of characters to be moved into the buffer        */
/*                                                                        */
/*      attr    - text attribute to be put into the buffer with each      */
/*                character in the string.                                */
/*                                                                        */
/*      width   - number of display columns to be copied from str.        */
/*                                                                        */
/*      begin   - initial display column in str where to start counting.  */
/*                                                                        */
/*  returns:                                                              */
/*                                                                        */
/*      actual number of display columns that were filled with text.      */
/*                                                                        */
/*------------------------------------------------------------------------*/

ushort TDrawBuffer::moveStr( ushort indent, TStringView str, TColorAttr attr,
                             ushort width, ushort begin ) noexcept
{
#ifdef __BORLANDC__
    if (begin < str.size())
        return moveStr(indent, str.substr(begin, width), attr);
    return 0;
#else
    if (attr != 0)
        return TText::drawStr(data.subspan(0, indent + width), indent, str, begin, attr);
    else
        return TText::drawStr(data.subspan(0, indent + width), indent, str, begin);
#endif
}

#ifdef __FLAT__
// This makes it possible to create TDrawBuffers for screens wider than 'maxViewWidth'.
// This does not work nor is necessary in non-flat builds.
// We must take the greatest of the screen's dimensions, because we cannot assume
// that 'screenWidth > screenHeight' and TDrawBuffer can also be used to draw
// vertical views (e.g. TScrollBar).
static TSpan<TScreenCell> allocData() noexcept
{
    size_t len = max(max(TScreen::screenWidth, TScreen::screenHeight), 80);
    return TSpan<TScreenCell>(new TScreenCell[len], len);
}

TDrawBuffer::TDrawBuffer() noexcept :
    data(allocData())
{
#ifndef __BORLANDC__
    // Uninitialized data could cause severe screen garbling.
    memset(data.data(), 0, data.size_bytes());
#endif
}

TDrawBuffer::~TDrawBuffer()
{
    delete[] data.data();
}
#endif

#pragma warn .asc
