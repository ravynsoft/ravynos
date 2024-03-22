/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/


#ifndef ST_DEBUG_H
#define ST_DEBUG_H

#include "util/compiler.h"
#include "util/u_debug.h"

struct st_context;

#define DEBUG_MESA            BITFIELD_BIT(0)
#define DEBUG_PRINT_IR        BITFIELD_BIT(1)
#define DEBUG_FALLBACK        BITFIELD_BIT(2)
#define DEBUG_BUFFER          BITFIELD_BIT(3)
#define DEBUG_WIREFRAME       BITFIELD_BIT(4)
#define DEBUG_GREMEDY         BITFIELD_BIT(5)
#define DEBUG_NOREADPIXCACHE  BITFIELD_BIT(6)

extern int ST_DEBUG;

void st_debug_init( void );

static inline void
ST_DBG( unsigned flag, const char *fmt, ... )
{
    if (ST_DEBUG & flag)
    {
        va_list args;

        va_start( args, fmt );
        debug_vprintf( fmt, args );
        va_end( args );
    }
}


#endif /* ST_DEBUG_H */
