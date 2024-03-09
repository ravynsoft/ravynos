/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   BUFFERS.H                                                             */
/*                                                                         */
/*   defines the functions getBufMem() and freeBufMem() for use            */
/*   in allocating and freeing viedo buffers                               */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif

#if defined( Uses_TVMemMgr ) && !defined( __TVMemMgr )
#define __TVMemMgr

const int DEFAULT_SAFETY_POOL_SIZE = 4096;

class TBufListEntry
{

private:

    TBufListEntry( void*&, size_t sz ) noexcept;
#if __cplusplus >= 201103L
    TBufListEntry(const TBufListEntry &) = default;
#endif
    void destroy() noexcept;

    void *operator new( size_t, size_t ) noexcept;
    void *operator new( size_t ) noexcept;
    void operator delete( void * ) noexcept;

    TBufListEntry *next;
    TBufListEntry *prev;
    void*& owner;
    size_t sz;

    static TBufListEntry *_NEAR bufList;
    static Boolean freeHead() noexcept;

    friend class TVMemMgr;
    friend void *operator new( size_t );
    friend void * allocBlock( size_t );

};

class TVMemMgr
{

public:

    TVMemMgr() noexcept;
    ~TVMemMgr();

    static void resizeSafetyPool( size_t = DEFAULT_SAFETY_POOL_SIZE ) noexcept;
    static int safetyPoolExhausted() noexcept;

    static void allocateDiscardable( void *&, size_t ) noexcept;
    static void reallocateDiscardable( void *&, size_t ) noexcept;
    static void freeDiscardable( void * ) noexcept;

private:

    static void * _NEAR safetyPool;
    static size_t _NEAR safetyPoolSize;
    static int _NEAR inited;
    static int initMemMgr() noexcept;

};

#endif  // Uses_TVMemMgr

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif
