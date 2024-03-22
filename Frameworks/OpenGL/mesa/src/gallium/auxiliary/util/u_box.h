#ifndef UTIL_BOX_INLINES_H
#define UTIL_BOX_INLINES_H

#include "pipe/p_state.h"
#include "util/u_math.h"
#include "util/format/u_format.h"

static inline void
u_box_1d(unsigned x, unsigned w, struct pipe_box *box)
{
   box->x = x;
   box->y = 0;
   box->z = 0;
   box->width = w;
   box->height = 1;
   box->depth = 1;
}

static inline void
u_box_2d(unsigned x,unsigned y, unsigned w, unsigned h, struct pipe_box *box)
{
   box->x = x;
   box->y = y;
   box->z = 0;
   box->width = w;
   box->height = h;
   box->depth = 1;
}

static inline void
u_box_origin_2d(unsigned w, unsigned h, struct pipe_box *box)
{
   box->x = 0;
   box->y = 0;
   box->z = 0;
   box->width = w;
   box->height = h;
   box->depth = 1;
}

static inline void
u_box_2d_zslice(unsigned x, unsigned y, unsigned z,
                unsigned w, unsigned h, struct pipe_box *box)
{
   box->x = x;
   box->y = y;
   box->z = z;
   box->width = w;
   box->height = h;
   box->depth = 1;
}

static inline void
u_box_3d(unsigned x, unsigned y, unsigned z,
         unsigned w, unsigned h, unsigned d,
         struct pipe_box *box)
{
   box->x = x;
   box->y = y;
   box->z = z;
   box->width = w;
   box->height = h;
   box->depth = d;
}

/* Clips @dst to width @w and height @h.
 * Returns -1 if the resulting box would be empty (then @dst is left unchanged).
 *          0 if nothing has been reduced.
 *          1 if width has been reduced.
 *          2 if height has been reduced.
 *          3 if both width and height have been reduced.
 * Aliasing permitted.
 */
static inline int
u_box_clip_2d(struct pipe_box *dst,
              const struct pipe_box *box, int w, int h)
{
   unsigned i;
   int a[2], b[2], dim[2];
   int *start, *end;
   int res = 0;

   if (!box->width || !box->height)
      return -1;
   dim[0] = w;
   dim[1] = h;
   a[0] = box->x;
   a[1] = box->y;
   b[0] = box->x + box->width;
   b[1] = box->y + box->height;

   for (i = 0; i < 2; ++i) {
      start = (a[i] <= b[i]) ? &a[i] : &b[i];
      end = (a[i] <= b[i]) ? &b[i] : &a[i];

      if (*end < 0 || *start >= dim[i])
         return -1;
      if (*start < 0) {
         *start = 0;
         res |= (1 << i);
      }
      if (*end > dim[i]) {
         *end = dim[i];
         res |= (1 << i);
      }
   }

   if (res) {
      dst->x = a[0];
      dst->y = a[1];
      dst->width = b[0] - a[0];
      dst->height = b[1] - a[1];
   }
   return res;
}

static inline int64_t
u_box_volume_3d(const struct pipe_box *box)
{
   return (int64_t)box->width * box->height * box->depth;
}

/* Aliasing of @dst permitted. Supports empty width */
static inline void
u_box_union_1d(struct pipe_box *dst,
               const struct pipe_box *a, const struct pipe_box *b)
{
   int x, width;

   if (a->width == 0) {
       x = b->x;
       width = b->width;
   } else if (b->width == 0) {
       x = a->x;
       width = a->width;
   } else {
       x = MIN2(a->x, b->x);
       width = MAX2(a->x + a->width, b->x + b->width) - x;
   }

   dst->x = x;
   dst->width = width;
}

/* Aliasing of @dst permitted. */
static inline void
u_box_intersect_1d(struct pipe_box *dst,
               const struct pipe_box *a, const struct pipe_box *b)
{
   int x;

   x = MAX2(a->x, b->x);

   dst->width = MIN2(a->x + a->width, b->x + b->width) - x;
   dst->x = x;
   if (dst->width <= 0) {
      dst->x = 0;
      dst->width = 0;
   }
}

/* Aliasing of @dst permitted. */
static inline void
u_box_union_2d(struct pipe_box *dst,
               const struct pipe_box *a, const struct pipe_box *b)
{
   int x, y;

   x = MIN2(a->x, b->x);
   y = MIN2(a->y, b->y);

   dst->width = MAX2(a->x + a->width, b->x + b->width) - x;
   dst->height = MAX2(a->y + a->height, b->y + b->height) - y;
   dst->x = x;
   dst->y = y;
}

/* Aliasing of @dst permitted. */
static inline void
u_box_union_3d(struct pipe_box *dst,
               const struct pipe_box *a, const struct pipe_box *b)
{
   int x, y, z;

   x = MIN2(a->x, b->x);
   y = MIN2(a->y, b->y);
   z = MIN2(a->z, b->z);

   dst->width = MAX2(a->x + a->width, b->x + b->width) - x;
   dst->height = MAX2(a->y + a->height, b->y + b->height) - y;
   dst->depth = MAX2(a->z + a->depth, b->z + b->depth) - z;
   dst->x = x;
   dst->y = y;
   dst->z = z;
}

static inline bool
u_box_test_intersection_1d(const struct pipe_box *a,
                           const struct pipe_box *b)
{
   int ax[2], bx[2];

   ax[0] = MIN2(a->x, a->x + a->width);
   ax[1] = MAX2(a->x, a->x + a->width - 1);

   bx[0] = MIN2(b->x, b->x + b->width);
   bx[1] = MAX2(b->x, b->x + b->width - 1);

   return ax[1] >= bx[0] && bx[1] >= ax[0];
}

static inline bool
u_box_test_intersection_2d(const struct pipe_box *a,
                           const struct pipe_box *b)
{
   unsigned i;
   int a_l[2], a_r[2], b_l[2], b_r[2];

   a_l[0] = MIN2(a->x, a->x + a->width);
   a_r[0] = MAX2(a->x, a->x + a->width);
   a_l[1] = MIN2(a->y, a->y + a->height);
   a_r[1] = MAX2(a->y, a->y + a->height);

   b_l[0] = MIN2(b->x, b->x + b->width);
   b_r[0] = MAX2(b->x, b->x + b->width);
   b_l[1] = MIN2(b->y, b->y + b->height);
   b_r[1] = MAX2(b->y, b->y + b->height);

   for (i = 0; i < 2; ++i) {
      if (a_l[i] > b_r[i] || a_r[i] < b_l[i])
         return false;
   }
   return true;
}

static inline bool
u_box_test_intersection_3d(const struct pipe_box *a,
                           const struct pipe_box *b)
{
   int ax[2], ay[2], ad[2], bx[2], by[2], bd[2];

   ax[0] = MIN2(a->x, a->x + a->width);
   ax[1] = MAX2(a->x, a->x + a->width - 1);
   ay[0] = MIN2(a->y, a->y + a->height);
   ay[1] = MAX2(a->y, a->y + a->height - 1);
   ad[0] = MIN2(a->z, a->z + a->depth);
   ad[1] = MAX2(a->z, a->z + a->depth - 1);

   bx[0] = MIN2(b->x, b->x + b->width);
   bx[1] = MAX2(b->x, b->x + b->width - 1);
   by[0] = MIN2(b->y, b->y + b->height);
   by[1] = MAX2(b->y, b->y + b->height - 1);
   bd[0] = MIN2(b->z, b->z + b->depth);
   bd[1] = MAX2(b->z, b->z + b->depth - 1);

   return ax[1] >= bx[0] && bx[1] >= ax[0] &&
          ay[1] >= by[0] && by[1] >= ay[0] &&
          ad[1] >= bd[0] && bd[1] >= ad[0];
}

static inline void
u_box_minify_2d(struct pipe_box *dst,
                const struct pipe_box *src, unsigned l)
{
   dst->x = src->x >> l;
   dst->y = src->y >> l;
   dst->width = MAX2(src->width >> l, 1);
   dst->height = MAX2(src->height >> l, 1);
}

static inline void
u_box_minify_3d(struct pipe_box *dst,
                const struct pipe_box *src, unsigned l)
{
   dst->x = src->x >> l;
   dst->y = src->y >> l;
   dst->z = src->z >> l;
   dst->width = MAX2(src->width >> l, 1);
   dst->height = MAX2(src->height >> l, 1);
   dst->depth = MAX2(src->depth >> l, 1);
}

/* Converts a box specified in pixels to an equivalent box specified
 * in blocks, where the boxes represent a region-of-interest of an image with
 * the given format. This is trivial (a copy) for uncompressed formats.
 */
static inline void
u_box_pixels_to_blocks(struct pipe_box *blocks,
                       const struct pipe_box *pixels, enum pipe_format format)
{
   u_box_3d(
         pixels->x / util_format_get_blockwidth(format),
         pixels->y / util_format_get_blockheight(format),
         pixels->z,
         DIV_ROUND_UP(pixels->width, util_format_get_blockwidth(format)),
         DIV_ROUND_UP(pixels->height, util_format_get_blockheight(format)),
         pixels->depth,
         blocks);
}

#endif
