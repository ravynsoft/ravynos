/*------------------------------------------------------------*/
/* filename - histlist.cpp                                    */
/*                                                            */
/* function(s)                                                */
/*          startId                                           */
/*          historyCount                                      */
/*          historyAdd                                        */
/*          historyStr                                        */
/*          clearHistory                                      */
/*          initHistory                                       */
/*          doneHistory                                       */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __MEM_H )
#include <mem.h>
#endif  // __MEM_H

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

#if !defined( __STDLIB_H )
#include <stdlib.h>
#endif  // __STDLIB_H

class HistRec
{

public:

    HistRec( uchar nId, TStringView nStr ) noexcept;

    void *operator new( size_t ) noexcept;
    void *operator new( size_t, HistRec * ) noexcept;

    uchar id;
    uchar len;
    char str[1];

};

void *HistRec::operator new( size_t, HistRec *hr ) noexcept
{
    return hr;
}

void *HistRec::operator new( size_t ) noexcept
{
    abort();
    return 0;
}

inline HistRec::HistRec( uchar nId, TStringView nStr ) noexcept :
    id( nId ),
    len( nStr.size() + 3 )
{
    memcpy( str, nStr.data(), nStr.size() );
    str[nStr.size()] = EOS;
}


inline HistRec *advance( HistRec *ptr, size_t s ) noexcept
{
    return (HistRec *)((char *)ptr + s);
}

inline HistRec *backup( HistRec *ptr, size_t s ) noexcept
{
    return (HistRec *)((char *)ptr - s);
}

inline HistRec *next( HistRec *ptr ) noexcept
{
    return advance( ptr, ptr->len );
}

inline HistRec *prev( HistRec *ptr ) noexcept
{
    return backup( ptr, ptr->len );
}

ushort historySize = 1024;  // initial size of history block

static uchar curId;
static HistRec *curRec;
static HistRec *historyBlock;
static HistRec *lastRec;

void advanceStringPointer() noexcept
{
    curRec = next( curRec );
    while( curRec < lastRec && curRec->id != curId )
        curRec = next( curRec );
    if( curRec >= lastRec )
        curRec = 0;
}

void deleteString() noexcept
{
    size_t len = curRec->len;
    HistRec *n = next( curRec );
#if !defined(__FLAT__)
    movmem( n, curRec, size_t( (char *)lastRec - (char *)n ) );
#else
    memcpy( curRec, n, size_t( (char *)lastRec - (char *)n ) );
#endif
    lastRec = backup( lastRec, len );
}

void insertString( uchar id, TStringView str ) noexcept
{
    ushort len = str.size() + 3;
    while( len > historySize - ( (char *)lastRec - (char *)historyBlock ) )
        {
        ushort firstLen = historyBlock->len;
        HistRec *dst = historyBlock;
        HistRec *src = next( historyBlock );
#if !defined(__FLAT__)
        movmem( src, dst,  size_t( (char *)lastRec - (char *)src ) );
#else
        memcpy( dst, src,  size_t( (char *)lastRec - (char *)src ) );
#endif
        lastRec = backup( lastRec, firstLen );
        }
    new( lastRec ) HistRec( id, str );
    lastRec = next( lastRec );
}

void startId( uchar id ) noexcept
{
    curId = id;
    curRec = historyBlock;
}

ushort historyCount( uchar id ) noexcept
{
    startId( id );
    ushort count =  0;
    advanceStringPointer();
    while( curRec != 0 )
        {
        count++;
        advanceStringPointer();
        }
    return count;
}

void historyAdd( uchar id, TStringView str ) noexcept
{
    if( str.empty() )
        return;
    startId( id );
    advanceStringPointer();
    while( curRec != 0 )
        {
        if( str == curRec->str )
            deleteString();
        advanceStringPointer();
        }
    insertString( id, str );
}

const char *historyStr( uchar id, int index ) noexcept
{
    startId( id );
    for( short i = 0; i <= index; i++ )
        advanceStringPointer();
    if( curRec != 0 )
        return curRec->str;
    else
        return 0;
}

void clearHistory() noexcept
{
    new (historyBlock) HistRec( 0, "" );
    lastRec = next( historyBlock );
}

void initHistory() noexcept
{
    historyBlock = (HistRec *) ::calloc(historySize, 1);
    clearHistory();
}

void doneHistory() noexcept
{
    ::free(historyBlock);
}
