/*------------------------------------------------------------*/
/* filename -       palette.cpp                               */
/*                                                            */
/* function(s)                                                */
/*                  TPalette member functions                 */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TPalette
#include <tvision/tv.h>

#if !defined( __MEM_H )
#include <mem.h>
#endif  // __MEM_H


TPalette::TPalette( const char* d, ushort len ) noexcept :
    data( new TColorAttr[ len+1 ] )
{
    data[0] = len;
    for (ushort i = 0; i < len; ++i)
        data[i + 1] = d[i];
}

#ifndef __BORLANDC__
TPalette::TPalette( const TColorAttr* d, ushort len ) noexcept :
    data( new TColorAttr[ len+1 ] )
{
    data[0] = len;
    memcpy( data+1, d, len*sizeof(TColorAttr) );
}
#endif

TPalette::TPalette( const TPalette& tp ) noexcept :
    data( new TColorAttr[ tp.data[0] + 1 ] )
{
    memcpy( data, tp.data, (tp.data[0] + 1)*sizeof(TColorAttr) );
}

TPalette::~TPalette()
{
    delete[] data;
}

TPalette& TPalette::operator = ( const TPalette& tp ) noexcept
{
    if( data != tp.data )
        {
        if( data[0] != tp.data[0] )
            {
            delete[] data;
            data = new TColorAttr[ tp.data[0] + 1 ];
            data[0] = tp.data[0];
            }
        memcpy( data+1, tp.data+1, data[0]*sizeof(TColorAttr) );
        }
    return *this;
}
