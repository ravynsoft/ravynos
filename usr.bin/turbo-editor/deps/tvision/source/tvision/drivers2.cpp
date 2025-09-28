/*------------------------------------------------------------------------*/
/*                                                                        */
/*  filename - drivers2.cpp                                               */
/*                                                                        */
/*  function(s)                                                           */
/*      ctrlToArrow -- map control keys to arrow keys                     */
/*      cstrlen     -- calculate length of a control string               */
/*                                                                        */
/*------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TText
#include <tvision/tv.h>

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  ctrlToArrow                                                           */
/*                                                                        */
/*  argument:                                                             */
/*                                                                        */
/*      keyCode - scan code to be mapped to keypad arrow code             */
/*                                                                        */
/*  returns:                                                              */
/*                                                                        */
/*      scan code for arrow key corresponding to Wordstar key,            */
/*      or original key code if no correspondence exists                  */
/*                                                                        */
/*------------------------------------------------------------------------*/
ushort ctrlToArrow(ushort keyCode) noexcept
{

const uchar ctrlCodes[] =
    {
    kbCtrlS, kbCtrlD, kbCtrlE, kbCtrlX, kbCtrlA,
    kbCtrlF, kbCtrlG, kbCtrlV, kbCtrlR, kbCtrlC, kbCtrlH
    };

const ushort arrowCodes[] =
    {
    kbLeft, kbRight, kbUp, kbDown, kbHome,
    kbEnd,  kbDel,   kbIns,kbPgUp, kbPgDn, kbBack
    };

    for( size_t i = 0; i < sizeof(ctrlCodes); i++ )
        if( (keyCode & 0x00ff) == ctrlCodes[i] )
            return arrowCodes[i];
    return keyCode;
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  cstrlen                                                               */
/*                                                                        */
/*  argument:                                                             */
/*                                                                        */
/*      text    - input string                                            */
/*                                                                        */
/*  returns                                                               */
/*                                                                        */
/*      length of string, ignoring '~' characters.                        */
/*                                                                        */
/*  Comments:                                                             */
/*                                                                        */
/*      Used in determining the displayed length of command strings,      */
/*      which use '~' to toggle between display attributes                */
/*                                                                        */
/*------------------------------------------------------------------------*/

int cstrlen( TStringView text ) noexcept
{
#ifdef __BORLANDC__
    const char _FAR *limit = &text[text.size()];
    const char _FAR *s = &text[0];
    int len = 0;
    while( s < limit )
        {
        if( *s++ != '~' )
            len++;
        }
    return len;
#else
    size_t i = 0, width = 0;
    while (i < text.size())
    {
        if (text[i] != '~')
            TText::next(text, i, width);
        else
            ++i;
    }
    return width;
#endif
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  strwidth                                                              */
/*                                                                        */
/*  argument:                                                             */
/*                                                                        */
/*      text    - input string                                            */
/*                                                                        */
/*  returns                                                               */
/*                                                                        */
/*      displayed length of string.                                       */
/*                                                                        */
/*------------------------------------------------------------------------*/

int strwidth( TStringView text ) noexcept
{
    return TText::width(text);
}
