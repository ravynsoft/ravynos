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


#ifndef LP_DEBUG_H
#define LP_DEBUG_H

#include "util/compiler.h"
#include "util/u_debug.h"

#define DEBUG_PIPE      0x1
#define DEBUG_TGSI      0x2
#define DEBUG_TEX       0x4
#define DEBUG_SETUP     0x10
#define DEBUG_RAST      0x20
#define DEBUG_QUERY     0x40
#define DEBUG_SCREEN    0x80
#define DEBUG_COUNTERS      0x800
#define DEBUG_SCENE         0x1000
#define DEBUG_FENCE         0x2000
#define DEBUG_MEM           0x4000
#define DEBUG_FS            0x8000
#define DEBUG_CS            0x10000
// unused                   0x40000
#define DEBUG_NO_FASTPATH   0x80000
#define DEBUG_LINEAR        0x100000
#define DEBUG_LINEAR2       0x200000
#define DEBUG_SHOW_DEPTH    0x400000
#define DEBUG_ACCURATE_A0   0x800000 /* verbose */
#define DEBUG_MESH         0x1000000

/* Performance flags.  These are active even on release builds.
 */
#define PERF_TEX_MEM        0x1  	/* minimize texture cache footprint */
#define PERF_NO_MIP_LINEAR  0x2  	/* MIP_FILTER_LINEAR ==> _NEAREST */
#define PERF_NO_MIPMAPS     0x4  	/* MIP_FILTER_NONE always */
#define PERF_NO_LINEAR      0x8  	/* FILTER_NEAREST always */
#define PERF_NO_TEX         0x10  	/* sample white always */
#define PERF_NO_BLEND       0x20  	/* disable blending */
#define PERF_NO_DEPTH       0x40  	/* disable depth buffering entirely */
#define PERF_NO_ALPHATEST   0x80  	/* disable alpha testing */
#define PERF_NO_RAST_LINEAR 0x100  	/* disable linear rast */
#define PERF_NO_SHADE       0x200  	/* disable fragment shaders */


extern int LP_PERF;

extern int LP_DEBUG;

void
st_debug_init(void);


static inline void
LP_DBG(unsigned flag, const char *fmt, ...)
{
   if (LP_DEBUG & flag) {
      va_list args;
      va_start(args, fmt);
      debug_vprintf(fmt, args);
      va_end(args);
   }
}


#endif /* LP_DEBUG_H */
