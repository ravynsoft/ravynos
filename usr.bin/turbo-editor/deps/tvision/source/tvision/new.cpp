/*------------------------------------------------------------*/
/* filename -       new.cpp                                   */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TVMemMgr
#include <tvision/tv.h>

#ifndef NDEBUG
#define NDEBUG
#endif
#include <assert.h>

#if !defined( __MEM_H )
#include <mem.h>
#endif  // __MEM_H

#if !defined( __ALLOC_H )

#include <alloc.h>
#endif  // __ALLOC_H

#if !defined( __STDLIB_H )
#include <stdlib.h>
#endif  // __STDLIB_H

TBufListEntry * _NEAR TBufListEntry::bufList = 0;

TBufListEntry::TBufListEntry( void*& o, size_t sz ) noexcept :
    owner( o ),
    sz( sz )
{
    next = bufList;
    prev = 0;
    bufList = this;
    if( next != 0 )
        next->prev = this;
}

void TBufListEntry::destroy() noexcept
{
    owner = 0;
    if( prev == 0 )
        bufList = next;
    else
        prev->next = next;
    if( next != 0 )
        next->prev = prev;
    delete this;
}

void *TBufListEntry::operator new( size_t sz, size_t extra ) noexcept
{
    return malloc( sz + extra );
}

void *TBufListEntry::operator new( size_t ) noexcept
{
    return 0;
}

void TBufListEntry::operator delete( void *b ) noexcept
{
    free( b );
}

Boolean TBufListEntry::freeHead() noexcept
{
    if( bufList == 0 )
        return False;
    else
        {
        bufList->destroy();
        return True;
        }
}

void * _NEAR TVMemMgr::safetyPool = 0;
size_t _NEAR TVMemMgr::safetyPoolSize = 0;
int _NEAR TVMemMgr::inited = 0;

TVMemMgr memMgr;

TVMemMgr::TVMemMgr() noexcept
{
    if( !inited )
        resizeSafetyPool();
}

TVMemMgr::~TVMemMgr()
{
    resizeSafetyPool(0);
    inited = 0;
}

void TVMemMgr::resizeSafetyPool( size_t sz ) noexcept
{
    inited = 1;
    free( safetyPool );
    if( sz == 0 )
        safetyPool = 0;
    else
        safetyPool = malloc( sz );
    safetyPoolSize = sz;
}

int TVMemMgr::safetyPoolExhausted() noexcept
{
    return inited && (safetyPool == 0);
}

void TVMemMgr::allocateDiscardable( void *&adr, size_t sz ) noexcept
{
    if( safetyPoolExhausted() )
        adr = 0;
    else
        {
        TBufListEntry *newEntry = new( sz ) TBufListEntry( adr, sz );
        if( newEntry == 0 )
            adr = 0;
        else
            adr = (char *)newEntry + sizeof(TBufListEntry);
        }
}

void TVMemMgr::reallocateDiscardable( void *&adr, size_t sz ) noexcept
{
    if( !sz )
        {
        freeDiscardable( adr );
        adr = 0;
        }
    else if( !adr )
        allocateDiscardable( adr, sz );
    else
        {
        TBufListEntry *entry = (TBufListEntry *)((char *)adr - sizeof(TBufListEntry));
        if( sz < entry->sz )
            {
            void *p = ::realloc(entry, sizeof(TBufListEntry) + sz);
            if( p )
                {
                TBufListEntry *newEntry = (TBufListEntry *)p;
                if( newEntry->prev )
                    newEntry->prev->next = newEntry;
                else
                    TBufListEntry::bufList = newEntry;
                if( newEntry->next )
                    newEntry->next->prev = newEntry;
                newEntry->sz = sz;
                adr = (char *)p + sizeof(TBufListEntry);
                }
            else
                {
                freeDiscardable( adr );
                adr = 0;
                }
            }
        else if( sz > entry->sz )
            {
            freeDiscardable( adr );
            allocateDiscardable( adr, sz );
            }
        }
}

void TVMemMgr::freeDiscardable( void *block ) noexcept
{
    if (block)
        ((TBufListEntry *)((char *)block - sizeof(TBufListEntry)))->destroy();
}

#if defined( __BORLANDC__ )

#if !defined( NDEBUG )
const int BLK_SIZE = 16;
const int BLK_DATA = 0xA6;
#else
const int BLK_SIZE = 0;
#endif

void * allocBlock( size_t sz )
{
    assert( heapcheck() >= 0 );

    sz += BLK_SIZE;
    if( sz == 0 )
        sz = 1;

    void *temp = malloc( sz );
    while( temp == 0 && TBufListEntry::freeHead() == True )
        temp = malloc( sz );
    if( temp == 0 )
        {
        if( TVMemMgr::safetyPoolExhausted() )
            abort();
        else
            {
            TVMemMgr::resizeSafetyPool( 0 );
            temp = malloc( sz );
            if( temp == 0 )
                abort();
            }
        }
#if !defined( NDEBUG )
    memset( temp, BLK_DATA, BLK_SIZE );
#endif
    return (char *)temp + BLK_SIZE;
}

void * operator new[] ( size_t sz )
{
   return allocBlock(sz);
}

void * operator new ( size_t sz )
{
   return allocBlock(sz);
}

#if !defined( NDEBUG )
static void check( void *blk )
{
    for( int i = 0; i < BLK_SIZE; i++ )
        assert( *((unsigned char *)blk + i) == BLK_DATA );
}
#endif

static void deleteBlock( void *blk )
{
    assert( heapcheck() >= 0 );
    if( blk == 0 )
        return;
    void *tmp = (char *)blk - BLK_SIZE;
#if !defined( NDEBUG )
    check( tmp );
#endif
    free( tmp );
    if( TVMemMgr::safetyPoolExhausted() )
        TVMemMgr::resizeSafetyPool();
}

void operator delete ( void *blk )
{
   deleteBlock(blk);
}

void operator delete[] ( void *blk )
{
   deleteBlock(blk);
}

#endif
