/**************************************************************************
 *
 * Copyright 2007-2021 VMware, Inc.
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
 * Rasterization for binned rectangles within a tile
 */

#include <limits.h>
#include "util/u_math.h"
#include "lp_debug.h"
#include "lp_perf.h"
#include "lp_rast_priv.h"


/* Our 16-pixel stamps are layed out as:
 *
 *    0  1  2  3
 *    4  5  6  7
 *    8  9  10 11
 *    12 13 14 15
 *
 * Define bitmasks for each row and column in this layout:
 */
#define COLUMN0 ((1<<0)|(1<<4)|(1<<8) |(1<<12))
#define COLUMN1 ((1<<1)|(1<<5)|(1<<9) |(1<<13))
#define COLUMN2 ((1<<2)|(1<<6)|(1<<10)|(1<<14))
#define COLUMN3 ((1<<3)|(1<<7)|(1<<11)|(1<<15))

#define ROW0 ((1<<0) |(1<<1) |(1<<2) |(1<<3))
#define ROW1 ((1<<4) |(1<<5) |(1<<6) |(1<<7))
#define ROW2 ((1<<8) |(1<<9) |(1<<10)|(1<<11))
#define ROW3 ((1<<12)|(1<<13)|(1<<14)|(1<<15))

#define STAMP_SIZE 4

static const unsigned left_mask_tab[STAMP_SIZE] = {
   COLUMN0 | COLUMN1 | COLUMN2 | COLUMN3,
   COLUMN1 | COLUMN2 | COLUMN3,
   COLUMN2 | COLUMN3,
   COLUMN3,
};

static const unsigned right_mask_tab[STAMP_SIZE] = {
   COLUMN0,
   COLUMN0 | COLUMN1,
   COLUMN0 | COLUMN1 | COLUMN2,
   COLUMN0 | COLUMN1 | COLUMN2 | COLUMN3,
};

static const unsigned top_mask_tab[STAMP_SIZE] = {
   ROW0 | ROW1 | ROW2 | ROW3,
   ROW1 | ROW2 | ROW3,
   ROW2 | ROW3,
   ROW3,
};

static const unsigned bottom_mask_tab[STAMP_SIZE] = {
   ROW0,
   ROW0 | ROW1,
   ROW0 | ROW1 | ROW2,
   ROW0 | ROW1 | ROW2 | ROW3,
};



static void
shade_quads(struct lp_rasterizer_task *task,
            const struct lp_rast_shader_inputs *inputs,
            unsigned x, unsigned y,
            unsigned mask)
{
   const struct lp_rast_state *state = task->state;
   const struct lp_fragment_shader_variant *variant = state->variant;
   const struct lp_scene *scene = task->scene;
   const unsigned stride = scene->cbufs[0].stride;
   uint8_t *cbufs[1] = { scene->cbufs[0].map + y * stride + x * 4 };
   unsigned strides[1] = { stride };

   assert(!variant->key.depth.enabled);

   /* Propagate non-interpolated raster state */
   task->thread_data.raster_state.viewport_index = inputs->viewport_index;

   /* run shader on 4x4 block */
   BEGIN_JIT_CALL(state, task);
   const unsigned fn_index = mask == 0xffff ? RAST_WHOLE : RAST_EDGE_TEST;
   variant->jit_function[fn_index](&state->jit_context,
                                   &state->jit_resources,
                                   x, y,
                                   inputs->frontfacing,
                                   GET_A0(inputs),
                                   GET_DADX(inputs),
                                   GET_DADY(inputs),
                                   cbufs,
                                   NULL,
                                   mask,
                                   &task->thread_data,
                                   strides, 0, 0, 0);
   END_JIT_CALL();
}


/* Shade a 4x4 stamp which may be partially outside the rectangle,
 * according to the mask parameter.
 */
static inline void
partial(struct lp_rasterizer_task *task,
        const struct lp_rast_shader_inputs *inputs,
        unsigned ix, unsigned iy,
        unsigned mask)
{
   /* Unfortunately we can end up generating full blocks on this path,
    * need to catch them.
    */
   assert(mask != 0x0);
   shade_quads(task, inputs, ix * STAMP_SIZE, iy * STAMP_SIZE, mask);
}


/**
 * Run the full SoA shader.
 */
void
lp_rast_linear_rect_fallback(struct lp_rasterizer_task *task,
                             const struct lp_rast_shader_inputs *inputs,
                             const struct u_rect *box)
{
   /* The interior of the rectangle (if there is one) will be
    * rasterized as full 4x4 stamps.
    *
    * At each edge of the rectangle, however, there will be a fringe
    * of partial blocks where the edge lands somewhere in the middle
    * of a 4x4-pixel stamp.
    *
    * For each edge, precalculate a mask of the pixels inside that
    * edge for the first 4x4-pixel stamp.
    *
    * Note that at the corners, and for narrow rectangles, an
    * individual stamp may have two or more edges active.  We'll deal
    * with that below by combining these masks as appropriate.
    */
   const unsigned left_mask   = left_mask_tab   [box->x0 & (STAMP_SIZE - 1)];
   const unsigned right_mask  = right_mask_tab  [box->x1 & (STAMP_SIZE - 1)];
   const unsigned top_mask    = top_mask_tab    [box->y0 & (STAMP_SIZE - 1)];
   const unsigned bottom_mask = bottom_mask_tab [box->y1 & (STAMP_SIZE - 1)];

   const unsigned ix0 = box->x0 / STAMP_SIZE;
   const unsigned ix1 = box->x1 / STAMP_SIZE;
   const unsigned iy0 = box->y0 / STAMP_SIZE;
   const unsigned iy1 = box->y1 / STAMP_SIZE;

   /* Various special cases */
   if (ix0 == ix1 && iy0 == iy1) {
      /* Rectangle is contained within a single 4x4-pixel stamp */
      partial(task, inputs, ix0, iy0,
              (left_mask & right_mask &
               top_mask & bottom_mask));
   }
   else if (ix0 == ix1) {
      /* Left and right edges fall on the same 4-pixel-wide column */
      unsigned mask = left_mask & right_mask;
      partial(task, inputs, ix0, iy0, mask & top_mask);
      for (unsigned i = iy0 + 1; i < iy1; i++)
         partial(task, inputs, ix0, i, mask);
      partial(task, inputs, ix0, iy1, mask & bottom_mask);
   }
   else if (iy0 == iy1) {
      /* Top and bottom edges fall on the same 4-pixel-wide row */
      unsigned mask = top_mask & bottom_mask;
      partial(task, inputs, ix0, iy0, mask & left_mask);
      for (unsigned i = ix0 + 1; i < ix1; i++)
         partial(task, inputs, i, iy0, mask);
      partial(task, inputs, ix1, iy0, mask & right_mask);
   } else {
      /* Each pair of edges falls in a separate 4-pixel-wide
       * row/column.
       */
      partial(task, inputs, ix0, iy0, left_mask  & top_mask);
      partial(task, inputs, ix0, iy1, left_mask  & bottom_mask);
      partial(task, inputs, ix1, iy0, right_mask & top_mask);
      partial(task, inputs, ix1, iy1, right_mask & bottom_mask);

      for (unsigned i = ix0 + 1; i < ix1; i++)
         partial(task, inputs, i, iy0, top_mask);

      for (unsigned i = ix0 + 1; i < ix1; i++)
         partial(task, inputs, i, iy1, bottom_mask);

      for (unsigned i = iy0 + 1; i < iy1; i++)
         partial(task, inputs, ix0, i, left_mask);

      for (unsigned i = iy0 + 1; i < iy1; i++)
         partial(task, inputs, ix1, i, right_mask);

      /* Full interior blocks */
      for (unsigned j = iy0 + 1; j < iy1; j++) {
         for (unsigned i = ix0 + 1; i < ix1; i++) {
            shade_quads(task, inputs, i * STAMP_SIZE, j * STAMP_SIZE, 0xffff);
         }
      }
   }
}
