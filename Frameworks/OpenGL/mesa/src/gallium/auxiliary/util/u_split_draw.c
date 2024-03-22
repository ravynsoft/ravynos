/*
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "pipe/p_defines.h"
#include "util/u_debug.h"
#include "util/u_split_draw.h"

bool
u_split_draw(const struct pipe_draw_info *info, uint32_t max_verts,
             uint32_t *count, uint32_t *step)
{
   if (*count <= max_verts) {
      *step = *count;
      return false;
   }

   switch (info->mode) {
      case MESA_PRIM_POINTS:
         *count = *step = max_verts;
         break;
      case MESA_PRIM_LINES:
         *count = *step = max_verts - (max_verts % 2);
         break;
      case MESA_PRIM_LINE_STRIP:
         *count = max_verts;
         *step = max_verts - 1;
         break;
      case MESA_PRIM_LINE_LOOP:
         *count = max_verts;
         *step = max_verts - 1;
         debug_warn_once("unhandled line loop "
                         "looping behavior with "
                         ">max vert count\n");
         break;
      case MESA_PRIM_TRIANGLES:
         *count = *step = max_verts - (max_verts % 3);
         break;
      case MESA_PRIM_TRIANGLE_STRIP:
         *count = max_verts;
         *step = max_verts - 2;
         break;
      default:
         debug_warn_once("unhandled primitive "
                         "max vert count, truncating\n");
         *count = *step = max_verts;
   }

   return true;
}
