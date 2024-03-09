/*------------------------------------------------------------*/
/* filename -       tstrlist.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TStrListMaker member functions            */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TStringList
#define Uses_TStrIndexRec
#define Uses_TStrListMaker
#define Uses_opstream
#define Uses_ipstream
#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __MEM_H )
#include <mem.h>
#endif  // __MEM_H

#if !defined( __IOSTREAM_H )
#include <iostream.h>
#endif  // __IOSTREAM_H

const int MAXKEYS = 16;

TStrIndexRec::TStrIndexRec() noexcept :
    count(0)
{
}

TStrListMaker::TStrListMaker( ushort aStrSize, ushort aIndexSize ) noexcept :
    strPos( 0 ),
    strSize( aStrSize ),
    strings( new char[aStrSize] ),
    indexPos( 0 ),
    indexSize( aIndexSize ),
    index( new TStrIndexRec[aIndexSize] )
{
}

#pragma warn -dsz

TStrListMaker::~TStrListMaker()
{
    delete [] strings;
    delete [] index;
}

#pragma warn .dsz

void TStrListMaker::closeCurrent()
{
    if( cur.count != 0 )
        {
        index[indexPos++] = cur;
        cur.count = 0;
        }
}

void TStrListMaker::put( ushort key, char *str )
{
    if( cur.count == MAXKEYS || key != cur.key + cur.count )
        closeCurrent();
    if( cur.count == 0 )
        {
        cur.key = key;
        cur.offset = strPos;
        }
    int len = strlen( str );
    strings[strPos] = len;
    memcpy( strings+strPos+1, str, len);
    strPos += len+1;
    cur.count++;
}

#if !defined(NO_STREAMABLE)

TStringList::TStringList( StreamableInit ) noexcept :
    basePos(0),
    indexSize(0),
    index(0)
{
}
#endif

#pragma warn -dsz

TStringList::~TStringList()
{
    delete [] index;
}

#pragma warn .dsz

void TStringList::get( char *dest, ushort key )
{
    if( indexSize == 0 )
        {
        *dest = EOS;
        return;
        }

    TStrIndexRec *cur = index;
    while( cur->key + cur->count -1 < key && cur - index < indexSize )
        cur++;
    if( cur->key + cur->count - 1 < key || cur->key > key )
        {
        *dest = EOS;
        return;
        }
    ip->seekg( basePos + cur->offset );
    int count = key - cur->key;
    do  {
        uchar sz = ip->readByte();
        ip->readBytes( dest, sz );
        dest[sz] = EOS;
        } while( count-- > 0 );
}

#if !defined(NO_STREAMABLE)

void TStrListMaker::write( opstream& os )
{
    closeCurrent();
    os << strPos;
    os.writeBytes( strings, strPos );
    os << indexPos;
    os.writeBytes( index, indexPos * sizeof( TStrIndexRec ) );
}

void *TStringList::read( ipstream& is )
{
    ip = &is;

    ushort strSize;
    is >> strSize;

    basePos = is.tellg();
    is.seekg( basePos + strSize );
    is >> indexSize;
    index = new TStrIndexRec[indexSize];
    is.readBytes( index, indexSize * sizeof( TStrIndexRec ) );
    return this;
}

TStreamable *TStringList::build()
{
    return new TStringList( streamableInit );
}

#endif
