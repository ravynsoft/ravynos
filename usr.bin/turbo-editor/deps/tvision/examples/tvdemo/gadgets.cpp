/*-------------------------------------------------------------------*/
/*                                                                   */
/*   Turbo Vision Demo                                               */
/*                                                                   */
/*   Gadgets.cpp:  Gadgets for the Turbo Vision Demo.  Includes a    */
/*        heap view and a clock view which display the clock at the  */
/*        right end of the menu bar and the current heap space at    */
/*        the right end of the status line.                          */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TRect
#define Uses_TView
#define Uses_TDrawBuffer
#include <tvision/tv.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <strstrea.h>
#include <iomanip.h>
#include <malloc.h>
#include <time.h>

#include "gadgets.h"

#if !defined( __BORLANDC__ ) && defined( _WIN32 )
#include <psapi.h>
#endif

//
// ------------- Heap Viewer functions
//

THeapView::THeapView(TRect& r) : TView( r )
{
    oldMem = 0;
    newMem = heapSize();
}


void THeapView::draw()
{
    TDrawBuffer buf;
    TColorAttr c = getColor(2);

    buf.moveChar(0, ' ', c, (short)size.x);
    buf.moveStr(0, heapStr, c);
    writeLine(0, 0, (short)size.x, 1, buf);
}


void THeapView::update()
{
    if( (newMem = heapSize()) != oldMem )
        {
        oldMem = newMem;
        drawView();
        }
}


uint32_t THeapView::heapSize()
{
    ostrstream totalStr( heapStr, sizeof heapStr);

#if defined( __BORLANDC__ )
    // When using Borland C++, display the unused physical memory.
#if !defined( __DPMI16__ ) && !defined( __DPMI32__ )
    struct farheapinfo heap;
#endif
    uint32_t total = farcoreleft();

    switch( heapcheck() )
        {
        case _HEAPEMPTY:
            strcpy(heapStr, "     No heap");
            total = -1;
            break;

        case _HEAPCORRUPT:
            strcpy(heapStr, "Heap corrupt");
            total = -2;
            break;

        case _HEAPOK:
#if !defined( __DPMI16__ ) && !defined( __DPMI32__ )
            heap.ptr = NULL;
            while(farheapwalk(&heap) != _HEAPEND)
                if(!heap.in_use)
                    total += heap.size;
#endif
            totalStr << setw(12) << total << ends;
            break;
        }

    return(total);
#elif defined( __GLIBC__ ) && !defined( __UCLIBC__ ) && !defined( __MUSL__ )
    // When using Glibc, display the memory consumed by malloc allocations in use.
    size_t allocatedSize =
#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 33)
        mallinfo2().uordblks;
#else
        (uint) mallinfo().uordblks;
#endif
    totalStr << setw(12) << allocatedSize << ends;
    return (uint32_t) allocatedSize;
#elif defined( _WIN32 )
    // When on Windows, display the virtual memory used by the process.
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc {};
    GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *) &pmc, sizeof(pmc));
    totalStr << setw(12) << pmc.PrivateUsage << ends;
    return (uint32_t) pmc.PrivateUsage;
#else
    totalStr << ends;
    return 0;
#endif
}


//
// -------------- Clock Viewer functions
//

TClockView::TClockView( TRect& r ) : TView( r )
{
    strcpy(lastTime, "        ");
    strcpy(curTime, "        ");
}


void TClockView::draw()
{
    TDrawBuffer buf;
    TColorAttr c = getColor(2);

    buf.moveChar(0, ' ', c, (short)size.x);
    buf.moveStr(0, curTime, c);
    writeLine(0, 0, (short)size.x, 1, buf);
}


void TClockView::update()
{
    time_t t = time(0);
    char *date = ctime(&t);

    date[19] = '\0';
    strcpy(curTime, &date[11]);        /* Extract time. */

    if( strcmp(lastTime, curTime) )
        {
        drawView();
        strcpy(lastTime, curTime);
        }
}

