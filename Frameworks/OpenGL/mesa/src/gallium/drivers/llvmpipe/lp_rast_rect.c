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

static unsigned left_mask_tab[STAMP_SIZE] = {
   COLUMN0 | COLUMN1 | COLUMN2 | COLUMN3,
   COLUMN1 | COLUMN2 | COLUMN3,
   COLUMN2 | COLUMN3,
   COLUMN3,
};

static unsigned right_mask_tab[STAMP_SIZE] = {
   COLUMN0,
   COLUMN0 | COLUMN1,
   COLUMN0 | COLUMN1 | COLUMN2,
   COLUMN0 | COLUMN1 | COLUMN2 | COLUMN3,
};

static unsigned top_mask_tab[STAMP_SIZE] = {
   ROW0 | ROW1 | ROW2 | ROW3,
   ROW1 | ROW2 | ROW3,
   ROW2 | ROW3,
   ROW3,
};

static unsigned bottom_mask_tab[STAMP_SIZE] = {
   ROW0,
   ROW0 | ROW1,
   ROW0 | ROW1 | ROW2,
   ROW0 | ROW1 | ROW2 | ROW3,
};


static inline void
full(struct lp_rasterizer_task *task,
     const struct lp_rast_rectangle *rect,
     unsigned ix, unsigned iy)
{
   LP_COUNT(nr_rect_fully_covered_4);
   lp_rast_shade_quads_all(task,
                           &rect->inputs,
                           task->x + ix * STAMP_SIZE,
                           task->y + iy * STAMP_SIZE);
}


static inline void
partial(struct lp_rasterizer_task *task,
        const struct lp_rast_rectangle *rect,
        unsigned ix, unsigned iy,
        unsigned mask)
{
   /* Unfortunately we can end up generating full blocks on this path,
    * need to catch them.
    */
   if (mask == 0xffff) {
      full(task, rect, ix, iy);
   } else {
      assert(mask);
      LP_COUNT(nr_rect_partially_covered_4);
      lp_rast_shade_quads_mask(task,
                               &rect->inputs,
                               task->x + ix * STAMP_SIZE,
                               task->y + iy * STAMP_SIZE,
                               mask);
   }
}


static inline void
intersect_rect_and_tile(struct lp_rasterizer_task *task,
                        const struct lp_rast_rectangle *rect,
                        struct u_rect *box)
{
   box->x0 = task->x;
   box->y0 = task->y;
   box->x1 = task->x + TILE_SIZE - 1;
   box->y1 = task->y + TILE_SIZE - 1;

   assert(u_rect_test_intersection(&rect->box, box));

   u_rect_find_intersection(&rect->box, box);

   box->x0 -= task->x;
   box->x1 -= task->x;
   box->y0 -= task->y;
   box->y1 -= task->y;
}


/**
 * Scan the tile in chunks and figure out which pixels to rasterize
 * for this rectangle.
 */
void
lp_rast_rectangle(struct lp_rasterizer_task *task,
                  const union lp_rast_cmd_arg arg)
{
   const struct lp_rast_rectangle *rect = arg.rectangle;

   /* Check for "disabled" rectangles generated in out-of-memory
    * conditions.
    */
   if (rect->inputs.disable) {
      /* This command was partially binned and has been disabled */
      return;
   }

   /* Intersect the rectangle with this tile.
    */
   struct u_rect box;
   intersect_rect_and_tile(task, rect, &box);

   /* The interior of the rectangle (if there is one) will be
    * rasterized as full 4x4 stamps.
    *
    * At each edge of the rectangle, however, there will be a fringe
    * of partial blocks where the edge lands somewhere in the middle
    * of a 4-pixel stamp.
    *
    * For each edge, precalculate a mask of the pixels inside that
    * edge for the first 4-pixel stamp.
    *
    * Note that at the corners, and for narrow rectangles, an
    * individual stamp may have two or more edges active.  We'll deal
    * with that below by combining these masks as appropriate.
    */
   unsigned left_mask   = left_mask_tab   [box.x0 & (STAMP_SIZE - 1)];
   unsigned right_mask  = right_mask_tab  [box.x1 & (STAMP_SIZE - 1)];
   unsigned top_mask    = top_mask_tab    [box.y0 & (STAMP_SIZE - 1)];
   unsigned bottom_mask = bottom_mask_tab [box.y1 & (STAMP_SIZE - 1)];

   unsigned ix0 = box.x0 / STAMP_SIZE;
   unsigned ix1 = box.x1 / STAMP_SIZE;
   unsigned iy0 = box.y0 / STAMP_SIZE;
   unsigned iy1 = box.y1 / STAMP_SIZE;

   /* Various special cases.
    */
   if (ix0 == ix1 && iy0 == iy1) {
      /* Rectangle is contained within a single 4x4 stamp:
       */
      partial(task, rect, ix0, iy0,
              (left_mask & right_mask & top_mask & bottom_mask));
   } else if (ix0 == ix1) {
      /* Left and right edges fall on the same 4-pixel-wide column:
       */
      unsigned mask = left_mask & right_mask;
      partial(task, rect, ix0, iy0, mask & top_mask);
      for (unsigned i = iy0 + 1; i < iy1; i++)
         partial(task, rect, ix0, i, mask);
      partial(task, rect, ix0, iy1, mask & bottom_mask);
   } else if (iy0 == iy1) {
      /* Top and bottom edges fall on the same 4-pixel-wide row:
       */
      unsigned mask = top_mask & bottom_mask;
      partial(task, rect, ix0, iy0, mask & left_mask);
      for (unsigned i = ix0 + 1; i < ix1; i++)
         partial(task, rect, i, iy0, mask);
      partial(task, rect, ix1, iy0, mask & right_mask);
   } else {
      /* Each pair of edges falls in a separate 4-pixel-wide
       * row/column.
       */
      partial(task, rect, ix0, iy0, left_mask  & top_mask);
      partial(task, rect, ix0, iy1, left_mask  & bottom_mask);
      partial(task, rect, ix1, iy0, right_mask & top_mask);
      partial(task, rect, ix1, iy1, right_mask & bottom_mask);

      for (unsigned i = ix0 + 1; i < ix1; i++)
         partial(task, rect, i, iy0, top_mask);

      for (unsigned i = ix0 + 1; i < ix1; i++)
         partial(task, rect, i, iy1, bottom_mask);

      for (unsigned i = iy0 + 1; i < iy1; i++)
         partial(task, rect, ix0, i, left_mask);

      for (unsigned i = iy0 + 1; i < iy1; i++)
         partial(task, rect, ix1, i, right_mask);

      /* Full interior blocks
       */
      for (unsigned j = iy0 + 1; j < iy1; j++) {
         for (unsigned i = ix0 + 1; i < ix1; i++) {
            full(task, rect, i, j);
         }
      }
   }
}


