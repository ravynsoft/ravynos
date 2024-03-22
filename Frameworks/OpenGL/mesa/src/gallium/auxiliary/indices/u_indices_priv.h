/*
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VMWARE AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef U_INDICES_PRIV_H
#define U_INDICES_PRIV_H

#include "util/compiler.h"
#include "u_indices.h"

#define IN_UINT8      0
#define IN_UINT16     1
#define IN_UINT32     2
#define IN_COUNT      3

#define OUT_UINT16    0
#define OUT_UINT32    1
#define OUT_COUNT     2


#define PRIM_COUNT   (MESA_PRIM_TRIANGLE_STRIP_ADJACENCY + 1)

static void translate_memcpy_uint( const void *in,
                                   unsigned start,
                                   unsigned in_nr,
                                   unsigned out_nr,
                                   unsigned restart_index,
                                   void *out )
{
   memcpy(out, &((int *)in)[start], out_nr*sizeof(int));
}

static void translate_memcpy_ushort( const void *in,
                                     unsigned start,
                                     unsigned in_nr,
                                     unsigned out_nr,
                                     unsigned restart_index,
                                     void *out )
{
   memcpy(out, &((short *)in)[start], out_nr*sizeof(short));
}

static unsigned out_size_idx( unsigned index_size )
{
   switch (index_size) {
   case 4: return OUT_UINT32;
   case 2: return OUT_UINT16;
   default: assert(0); return OUT_UINT16;
   }
}

static unsigned in_size_idx( unsigned index_size )
{
   switch (index_size) {
   case 4: return IN_UINT32;
   case 2: return IN_UINT16;
   case 1: return IN_UINT8;
   default: assert(0); return IN_UINT8;
   }
}

#endif
