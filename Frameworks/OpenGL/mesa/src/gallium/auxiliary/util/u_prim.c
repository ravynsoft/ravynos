/* Copyright (c) 2008 VMware, Inc.
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "u_prim.h"
#include "pipe/p_state.h"


/** Return string name of given primitive type */
const char *
u_prim_name(enum mesa_prim prim)
{
#if defined(__GNUC__)
   /* Check that the enum is packed: */
   STATIC_ASSERT(sizeof(enum mesa_prim) == 1);
#endif

   /* Draw merging in u_threaded_context requires that sizeof(mode) == 1. */
   struct pipe_draw_info info;
   STATIC_ASSERT(sizeof(info.mode) == 1);

   struct pipe_draw_vertex_state_info dvs_info;
   STATIC_ASSERT(sizeof(dvs_info.mode) == 1);

   static const struct debug_named_value names[] = {
      DEBUG_NAMED_VALUE(MESA_PRIM_POINTS),
      DEBUG_NAMED_VALUE(MESA_PRIM_LINES),
      DEBUG_NAMED_VALUE(MESA_PRIM_LINE_LOOP),
      DEBUG_NAMED_VALUE(MESA_PRIM_LINE_STRIP),
      DEBUG_NAMED_VALUE(MESA_PRIM_TRIANGLES),
      DEBUG_NAMED_VALUE(MESA_PRIM_TRIANGLE_STRIP),
      DEBUG_NAMED_VALUE(MESA_PRIM_TRIANGLE_FAN),
      DEBUG_NAMED_VALUE(MESA_PRIM_QUADS),
      DEBUG_NAMED_VALUE(MESA_PRIM_QUAD_STRIP),
      DEBUG_NAMED_VALUE(MESA_PRIM_POLYGON),
      DEBUG_NAMED_VALUE(MESA_PRIM_LINES_ADJACENCY),
      DEBUG_NAMED_VALUE(MESA_PRIM_LINE_STRIP_ADJACENCY),
      DEBUG_NAMED_VALUE(MESA_PRIM_TRIANGLES_ADJACENCY),
      DEBUG_NAMED_VALUE(MESA_PRIM_TRIANGLE_STRIP_ADJACENCY),
      DEBUG_NAMED_VALUE_END
   };
   return debug_dump_enum(names, prim);
}
