/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef SVGA_DEBUG_H
#define SVGA_DEBUG_H

#include "util/compiler.h"
#include "util/u_debug.h"

#define DEBUG_DMA          0x1
#define DEBUG_TGSI         0x4
#define DEBUG_PIPE         0x8
#define DEBUG_STATE        0x10
#define DEBUG_SCREEN       0x20
#define DEBUG_TEX          0x40
#define DEBUG_SWTNL        0x80
#define DEBUG_CONSTS       0x100
#define DEBUG_VIEWPORT     0x200
#define DEBUG_VIEWS        0x400
#define DEBUG_PERF         0x800    /* print something when we hit any slow path operation */
#define DEBUG_FLUSH        0x1000   /* flush after every draw */
#define DEBUG_SYNC         0x2000   /* sync after every flush */
#define DEBUG_QUERY        0x4000
#define DEBUG_CACHE        0x8000
#define DEBUG_STREAMOUT    0x10000
#define DEBUG_SAMPLERS     0x20000
#define DEBUG_IMAGE        0x40000
#define DEBUG_UAV          0x80000
#define DEBUG_RETRY        0x100000

#ifdef DEBUG
extern int SVGA_DEBUG;
#define DBSTR(x) x
#else
#define SVGA_DEBUG 0
#define DBSTR(x) ""
#endif

static inline void
SVGA_DBG( unsigned flag, const char *fmt, ... )
{
#ifdef DEBUG 
    if (SVGA_DEBUG & flag)
    {
        va_list args;

        va_start( args, fmt );
        debug_vprintf( fmt, args );
        va_end( args );
    }
#else
    (void)flag;
    (void)fmt;
#endif
}


#endif
