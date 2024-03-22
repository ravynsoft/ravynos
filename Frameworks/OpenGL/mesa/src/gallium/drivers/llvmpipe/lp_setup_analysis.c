/**************************************************************************
 *
 * Copyright 2010-2021 VMWare, Inc.
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

/**
 * Look for common topology patterns which can be converted into rectangles.
 */


#include "lp_setup_context.h"
#include "draw/draw_vbuf.h"
#include "draw/draw_vertex.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "lp_state_fs.h"
#include "lp_state_setup.h"
#include "lp_perf.h"


/**
 * Duplicated from lp_setup_vbuf.c.
 */
typedef const float (*const_float4_ptr)[4];


static inline
const_float4_ptr get_vert(const void *vertex_buffer, int index, int stride)
{
   return (const_float4_ptr)((char *)vertex_buffer + index * stride);
}


/* Aero sends these weird zero area triangles.  Test for them here.
 */
static bool
is_zero_area(const float (*v0)[4],
             const float (*v1)[4],
             const float (*v2)[4])
{
   /* Specialized test for v0.y == v1.y == v2.y.
    */
   return (v0[0][1] == v1[0][1] &&
           v0[0][1] == v2[0][1]);
}


/**
 * Assuming axis-aligned stretch blit (s a function of x alone, t a
 * function of y alone), create a new vertex as in:
 *
 *   vx------+
 *    |      |
 *    |      |
 *   out-----vy
 */
static void
make_vert(const float (*vx)[4],
          const float (*vy)[4],
          float (*out)[4])
{
   out[0][0] = vx[0][0];
   out[0][1] = vy[0][1];
   out[0][2] = vx[0][2];
   out[0][3] = vx[0][3];
   out[1][0] = vx[1][0];
   out[1][1] = vy[1][1];
}


/* Calculate axis-aligned interpolant for s as a function of x.
 */
static void
calc_interps(float x0, float x1,
             float s0, float s1,
             float *a, float *b)
{
   assert(x0 != x1);
   *a = (s0 - s1) / (x0 - x1);
   *b = s0 - *a * x0;
}


/* Validate axis-aligned interpolant for s and t as functions of x and
 * y respectively.
 */
static bool
test_interps(const_float4_ptr v,
             float as, float bs,
             float at, float bt)
{
   float s = as * v[0][0] + bs;
   float t = at * v[0][1] + bt;
   return (util_is_approx(s, v[1][0], 1/4096.0) &&
           util_is_approx(t, v[1][1], 1/4096.0));
}


static void
rect(struct lp_setup_context *setup,
     const float (*v0)[4],
     const float (*v1)[4],
     const float (*v2)[4])
{
   ASSERTED int culled = LP_COUNT_GET(nr_culled_rects);

   if (0) {
      float as, bs, at, bt;
      calc_interps(v0[0][0], v2[0][0], v0[1][0], v2[1][0], &as, &bs);
      calc_interps(v0[0][1], v2[0][1], v0[1][1], v2[1][1], &at, &bt);
      assert(test_interps(v1, as, bs, at, bt));
   }

   assert(v0[0][0] == v1[0][0]);
   assert(v1[0][1] == v2[0][1]);

   lp_rect_cw(setup, v0, v1, v2, true);

   assert(culled == LP_COUNT_GET(nr_culled_rects));
}


/**
 * Check this is an axis-aligned rectangle as in:
 *
 *   v3------v0
 *    |      |
 *    |      |
 *   v2------v1
 */
static bool
test_rect(const_float4_ptr v0,
          const_float4_ptr v1,
          const_float4_ptr v2,
          const_float4_ptr v3)
{
   if (v0[0][0] != v1[0][0] ||
       v1[0][1] != v2[0][1] ||
       v2[0][0] != v3[0][0] ||
       v3[0][1] != v0[0][1])
      return false;

   if (v0[0][3] != 1.0 ||
       v1[0][3] != 1.0 ||
       v2[0][3] != 1.0 ||
       v3[0][3] != 1.0)
      return false;

   return true;
}


/**
 * Aero sends the following shape as
 *
 *  18                                                        12
 *    +-------------------------------------------------/----+
 *    |\                                      /---------    /|
 *    | \                           /---------             / |
 *    |  \                /---------                      /  |
 *    |   \     /---------                               /   |
 * vA +    +--------------------------------------------+    + vC
 *    |    | 9                                        6 |    |
 *    |   /|                                            |\   |
 *    |   ||                                            ||   |
 *    |  / |                                            | \  |
 *    |  | |                                            | |  |
 *    |  | | 3                                        0 |  \ |
 * vB + /  +--------------------------------------------+  | + vD
 *    | | /                               ---------/     \ | |
 *    |/ /                      ---------/                \ \|
 *    ||/             ---------/                           \||
 *    |/    ---------/                                      \|
 *    +----/-------------------------------------------------+
 *   1                                                        2
 *
 * and in the following decomposition:
 *   (0, 1, 2)
 *   (3, 0, 1),
 *   (6, 0, 2),
 *   (9, 3, 1),
 *   (12, 2, 6),
 *   (12, 6, 9),
 *   (18, 1, 9),
 *   (18, 9, 12),
 *
 * There's no straight-forward way to interpret the existing vertices
 * as rectangles.  Instead we convert this into four axis-aligned
 * rectangles by introducing new vertices at vA, vB, vC and vD, and
 * then drawing rectangles.
 */
static bool
check_elts24(struct lp_setup_context *setup, const void *vb, int stride)
{
   const int count = 24;
   const int uniq[8] = { 0, 1, 2, 3, 6, 9, 12, 18 };
   const int elts[24] = {
      0, 1, 2,
      3, 0, 1,
      6, 0, 2,
      9, 3, 1,
      12, 2, 6,
      12, 6, 9,
      18, 1, 9,
      18, 9, 12
   };
   const_float4_ptr v0  = get_vert(vb, stride, 0);
   const_float4_ptr v1  = get_vert(vb, stride, 1);
   const_float4_ptr v2  = get_vert(vb, stride, 2);
   const_float4_ptr v3  = get_vert(vb, stride, 3);
   const_float4_ptr v6  = get_vert(vb, stride, 6);
   const_float4_ptr v9  = get_vert(vb, stride, 9);
   const_float4_ptr v12 = get_vert(vb, stride, 12);
   const_float4_ptr v18 = get_vert(vb, stride, 18);

   /* Could just calculate a set of interpolants and bin rectangle
    * commands for this triangle list directly.  Instead, introduce
    * some new vertices and feed to the rectangle setup code:
    */
   alignas(16) float vA[2][4];
   alignas(16) float vB[2][4];
   alignas(16) float vC[2][4];
   alignas(16) float vD[2][4];

   float as, bs;
   float at, bt;
   int i;

   if (stride != 32)
      return false;

   /* Check the shape is two rectangles:
    */
   if (!test_rect(v12, v2, v1, v18))
      return false;

   if (!test_rect(v6, v0, v3, v9))
      return false;

   /* XXX: check one rect is inside the other?
    */

   /* Check our tesselation matches:
    */
   for (i = 0; i < count; i++) {
      if (memcmp(get_vert(vb, i, stride),
                 get_vert(vb, elts[i], stride),
                 6 * sizeof(float)) != 0)
         return false;
   }

   /* Test that this is a stretch blit, meaning we should be able to
    * introduce vertices at will.
    */
   calc_interps(v0[0][0], v2[0][0], v0[1][0], v2[1][0], &as, &bs);
   calc_interps(v0[0][1], v2[0][1], v0[1][1], v2[1][1], &at, &bt);

   for (i = 0; i < ARRAY_SIZE(uniq); i++) {
      const_float4_ptr v = get_vert(vb, stride, i);
      if (!test_interps(v, as, bs, at, bt))
         return false;
   }

   make_vert(v18, v9, vA);
   make_vert(v18, v3, vB);
   make_vert(v12, v9, vC);
   make_vert(v12, v3, vD);

   assert(test_interps((const_float4_ptr)vA, as, bs, at, bt));
   assert(test_interps((const_float4_ptr)vB, as, bs, at, bt));
   assert(test_interps((const_float4_ptr)vC, as, bs, at, bt));
   assert(test_interps((const_float4_ptr)vD, as, bs, at, bt));

   rect(setup,
        (const_float4_ptr)v12,
        (const_float4_ptr)vC,
        (const_float4_ptr)vA);

   rect(setup,
        (const_float4_ptr)v9,
        (const_float4_ptr)v3,
        (const_float4_ptr)vB);

   rect(setup,
        (const_float4_ptr)vD,
        (const_float4_ptr)v2,
        (const_float4_ptr)v1);

   rect(setup,
        (const_float4_ptr)vC,
        (const_float4_ptr)vD,
        (const_float4_ptr)v0);

   return true;
}

bool
lp_setup_analyse_triangles(struct lp_setup_context *setup,
                           const void *vb,
                           int stride,
                           int nr)
{
   int i;
   const bool variant_blit = setup->fs.current.variant->blit;

   if (0) {
      debug_printf("%s %d\n", __func__, nr);

      if (variant_blit) {
         debug_printf("  - blit variant\n");
      }

      for (i = 0; i < nr; i += 3) {
         const_float4_ptr v0 = get_vert(vb, i, stride);
         const_float4_ptr v1 = get_vert(vb, i+1, stride);
         const_float4_ptr v2 = get_vert(vb, i+2, stride);
         lp_setup_print_triangle(setup, v0, v1, v2);
      }
   }

   /* When drawing some window navigator bars, aero sends a mixed up
    * rectangle:
    *
    *    - first triangle ccw
    *    - second triangle cw
    *    - third triangle zero area.
    */
   if (nr == 9 &&
       is_zero_area(get_vert(vb, nr-1, stride),
                    get_vert(vb, nr-2, stride),
                    get_vert(vb, nr-3, stride)))
   {
      const float (*v0)[4] = get_vert(vb, 0, stride);
      const float (*v1)[4] = get_vert(vb, 1, stride);
      const float (*v2)[4] = get_vert(vb, 2, stride);
      const float (*v3)[4] = get_vert(vb, 3, stride);
      const float (*v4)[4] = get_vert(vb, 4, stride);
      const float (*v5)[4] = get_vert(vb, 5, stride);

      /*
       * [   v0,       v1,       v2   ]  [   v3,       v4,       v5   ]
       * [(X0, Y0), (X0, Y1), (X1, Y1)]  [(X1, Y0), (X1, Y1), (X0, Y0)]
       */
      if (v0[0][0] == v1[0][0] && v0[0][0] == v5[0][0] &&
          v2[0][0] == v3[0][0] && v2[0][0] == v4[0][0] &&
          v0[0][1] == v3[0][1] && v0[0][1] == v5[0][1] &&
          v1[0][1] == v2[0][1] && v1[0][1] == v4[0][1]) {

         lp_rect_cw(setup, v0, v1, v2, true);
      }
      return true;
   }

   /* When highlighting (?) windows, aero sends a window border
    * comprised of non-rectangular triangles, but which as a whole can
    * be decomposed into rectangles.
    *
    * Again, with a zero-area trailing triangle.
    *
    * This requires introducing a couple of new vertices, which are
    * luckily easy to compute.
    */
   if (nr == 27 &&
       variant_blit &&
       setup->setup.variant->key.inputs[0].src_index == 1 &&
       setup->setup.variant->key.inputs[0].usage_mask == 0x3 &&
       is_zero_area(get_vert(vb, nr-1, stride),
                    get_vert(vb, nr-2, stride),
                    get_vert(vb, nr-3, stride)) &&
       check_elts24(setup, vb, stride))
   {
      return true;
   }

   return false;
}
