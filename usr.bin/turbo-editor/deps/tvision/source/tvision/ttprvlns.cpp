/*------------------------------------------------------------*/
/* filename -       ttprvlns.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TTerminal prevLines member function       */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TTerminal
#include <tvision/tv.h>

static Boolean findLfBackwards( const char *buffer, ushort &pos, ushort count )
// Pre: count >= 1.
// Post: 'pos' points to the last checked character.
{
    ++pos;
    do
    {
        if (buffer[--pos] == '\n')
            return True;
    } while (--count > 0);
    return False;
}

ushort TTerminal::prevLines( ushort pos, ushort lines )
{
    if (lines > 0 && pos != queBack)
    {
        do
        {
            if (pos == queBack)
                return queBack;
            bufDec(pos);
            ushort count = (pos >= queBack ? pos - queBack : pos) + 1;
            if (findLfBackwards(buffer, pos, count))
                --lines;
        } while (lines > 0);
        bufInc(pos);
    }
    return pos;
}
