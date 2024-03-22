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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */

#include "draw/draw_context.h"
#include "draw/draw_private.h"
#include "draw/draw_pt.h"
#include "util/u_debug.h"


void
draw_pt_split_prim(enum mesa_prim prim, unsigned *first, unsigned *incr)
{
   switch (prim) {
   case MESA_PRIM_POINTS:
      *first = 1;
      *incr = 1;
      break;
   case MESA_PRIM_LINES:
      *first = 2;
      *incr = 2;
      break;
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_LINE_LOOP:
      *first = 2;
      *incr = 1;
      break;
   case MESA_PRIM_LINES_ADJACENCY:
      *first = 4;
      *incr = 4;
      break;
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      *first = 4;
      *incr = 1;
      break;
   case MESA_PRIM_TRIANGLES:
      *first = 3;
      *incr = 3;
      break;
   case MESA_PRIM_TRIANGLES_ADJACENCY:
      *first = 6;
      *incr = 6;
      break;
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_TRIANGLE_FAN:
   case MESA_PRIM_POLYGON:
      *first = 3;
      *incr = 1;
      break;
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      *first = 6;
      *incr = 2;
      break;
   case MESA_PRIM_QUADS:
      *first = 4;
      *incr = 4;
      break;
   case MESA_PRIM_QUAD_STRIP:
      *first = 4;
      *incr = 2;
      break;
   default:
      assert(0);
      *first = 0;
      *incr = 1;		/* set to one so that count % incr works */
      break;
   }
}


unsigned
draw_pt_trim_count(unsigned count, unsigned first, unsigned incr)
{
   if (count < first)
      return 0;
   return count - (count - first) % incr;
}
