/**************************************************************************
 *
 * Copyright 2007-2009 VMware, Inc.
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
 * Rasterization for binned triangles within a tile
 */

#include <limits.h>
#include "util/u_math.h"
#include "lp_debug.h"
#include "lp_perf.h"
#include "lp_rast_priv.h"

/**
 * Shade all pixels in a 4x4 block.
 */
static void
block_full_4(struct lp_rasterizer_task *task,
             const struct lp_rast_triangle *tri,
             int x, int y)
{
   lp_rast_shade_quads_all(task, &tri->inputs, x, y);
}


/**
 * Shade all pixels in a 16x16 block.
 */
static void
block_full_16(struct lp_rasterizer_task *task,
              const struct lp_rast_triangle *tri,
              int x, int y)
{
   assert(x % 16 == 0);
   assert(y % 16 == 0);
   for (unsigned iy = 0; iy < 16; iy += 4)
      for (unsigned ix = 0; ix < 16; ix += 4)
         block_full_4(task, tri, x + ix, y + iy);
}

static inline unsigned
build_mask_linear(int32_t c, int32_t dcdx, int32_t dcdy)
{
   unsigned mask = 0;

   int32_t c0 = c;
   int32_t c1 = c0 + dcdy;
   int32_t c2 = c1 + dcdy;
   int32_t c3 = c2 + dcdy;

   mask |= ((c0 + 0 * dcdx) >> 31) & (1 << 0);
   mask |= ((c0 + 1 * dcdx) >> 31) & (1 << 1);
   mask |= ((c0 + 2 * dcdx) >> 31) & (1 << 2);
   mask |= ((c0 + 3 * dcdx) >> 31) & (1 << 3);
   mask |= ((c1 + 0 * dcdx) >> 31) & (1 << 4);
   mask |= ((c1 + 1 * dcdx) >> 31) & (1 << 5);
   mask |= ((c1 + 2 * dcdx) >> 31) & (1 << 6);
   mask |= ((c1 + 3 * dcdx) >> 31) & (1 << 7);
   mask |= ((c2 + 0 * dcdx) >> 31) & (1 << 8);
   mask |= ((c2 + 1 * dcdx) >> 31) & (1 << 9);
   mask |= ((c2 + 2 * dcdx) >> 31) & (1 << 10);
   mask |= ((c2 + 3 * dcdx) >> 31) & (1 << 11);
   mask |= ((c3 + 0 * dcdx) >> 31) & (1 << 12);
   mask |= ((c3 + 1 * dcdx) >> 31) & (1 << 13);
   mask |= ((c3 + 2 * dcdx) >> 31) & (1 << 14);
   mask |= ((c3 + 3 * dcdx) >> 31) & (1 << 15);

   return mask;
}


UNUSED static inline void
build_masks(int32_t c,
            int32_t cdiff,
            int32_t dcdx,
            int32_t dcdy,
            unsigned *outmask,
            unsigned *partmask)
{
   *outmask |= build_mask_linear(c, dcdx, dcdy);
   *partmask |= build_mask_linear(c + cdiff, dcdx, dcdy);
}

void
lp_rast_triangle_3_16(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   union lp_rast_cmd_arg arg2;
   arg2.triangle.tri = arg.triangle.tri;
   arg2.triangle.plane_mask = (1<<3)-1;
   lp_rast_triangle_3(task, arg2);
}

void
lp_rast_triangle_3_4(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   lp_rast_triangle_3_16(task, arg);
}

void
lp_rast_triangle_4_16(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   union lp_rast_cmd_arg arg2;
   arg2.triangle.tri = arg.triangle.tri;
   arg2.triangle.plane_mask = (1<<4)-1;
   lp_rast_triangle_4(task, arg2);
}

void
lp_rast_triangle_ms_3_16(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   union lp_rast_cmd_arg arg2;
   arg2.triangle.tri = arg.triangle.tri;
   arg2.triangle.plane_mask = (1<<3)-1;
   lp_rast_triangle_ms_3(task, arg2);
}

void
lp_rast_triangle_ms_3_4(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   lp_rast_triangle_ms_3_16(task, arg);
}

void
lp_rast_triangle_ms_4_16(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   union lp_rast_cmd_arg arg2;
   arg2.triangle.tri = arg.triangle.tri;
   arg2.triangle.plane_mask = (1<<4)-1;
   lp_rast_triangle_ms_4(task, arg2);
}

#if DETECT_ARCH_SSE

#include <emmintrin.h>
#include "util/u_sse.h"


static inline void
build_masks_sse(int c,
                int cdiff,
                int dcdx,
                int dcdy,
                unsigned *outmask,
                unsigned *partmask)
{
   __m128i cstep0 = _mm_setr_epi32(c, c+dcdx, c+dcdx*2, c+dcdx*3);
   __m128i xdcdy = _mm_set1_epi32(dcdy);

   /* Get values across the quad
    */
   __m128i cstep1 = _mm_add_epi32(cstep0, xdcdy);
   __m128i cstep2 = _mm_add_epi32(cstep1, xdcdy);
   __m128i cstep3 = _mm_add_epi32(cstep2, xdcdy);

   {
      __m128i cstep01, cstep23, result;

      cstep01 = _mm_packs_epi32(cstep0, cstep1);
      cstep23 = _mm_packs_epi32(cstep2, cstep3);
      result = _mm_packs_epi16(cstep01, cstep23);

      *outmask |= _mm_movemask_epi8(result);
   }


   {
      __m128i cio4 = _mm_set1_epi32(cdiff);
      __m128i cstep01, cstep23, result;

      cstep0 = _mm_add_epi32(cstep0, cio4);
      cstep1 = _mm_add_epi32(cstep1, cio4);
      cstep2 = _mm_add_epi32(cstep2, cio4);
      cstep3 = _mm_add_epi32(cstep3, cio4);

      cstep01 = _mm_packs_epi32(cstep0, cstep1);
      cstep23 = _mm_packs_epi32(cstep2, cstep3);
      result = _mm_packs_epi16(cstep01, cstep23);

      *partmask |= _mm_movemask_epi8(result);
   }
}


static inline unsigned
build_mask_linear_sse(int c, int dcdx, int dcdy)
{
   __m128i cstep0 = _mm_setr_epi32(c, c+dcdx, c+dcdx*2, c+dcdx*3);
   __m128i xdcdy = _mm_set1_epi32(dcdy);

   /* Get values across the quad
    */
   __m128i cstep1 = _mm_add_epi32(cstep0, xdcdy);
   __m128i cstep2 = _mm_add_epi32(cstep1, xdcdy);
   __m128i cstep3 = _mm_add_epi32(cstep2, xdcdy);

   /* pack pairs of results into epi16
    */
   __m128i cstep01 = _mm_packs_epi32(cstep0, cstep1);
   __m128i cstep23 = _mm_packs_epi32(cstep2, cstep3);

   /* pack into epi8, preserving sign bits
    */
   __m128i result = _mm_packs_epi16(cstep01, cstep23);

   /* extract sign bits to create mask
    */
   return _mm_movemask_epi8(result);
}

static inline unsigned
sign_bits4(const __m128i *cstep, int cdiff)
{

   /* Adjust the step values
    */
   __m128i cio4 = _mm_set1_epi32(cdiff);
   __m128i cstep0 = _mm_add_epi32(cstep[0], cio4);
   __m128i cstep1 = _mm_add_epi32(cstep[1], cio4);
   __m128i cstep2 = _mm_add_epi32(cstep[2], cio4);
   __m128i cstep3 = _mm_add_epi32(cstep[3], cio4);

   /* Pack down to epi8
    */
   __m128i cstep01 = _mm_packs_epi32(cstep0, cstep1);
   __m128i cstep23 = _mm_packs_epi32(cstep2, cstep3);
   __m128i result = _mm_packs_epi16(cstep01, cstep23);

   /* Extract the sign bits
    */
   return _mm_movemask_epi8(result);
}

#define COLUMN0 ((1<<0)|(1<<4)|(1<<8) |(1<<12))
#define COLUMN1 ((1<<1)|(1<<5)|(1<<9) |(1<<13))
#define COLUMN2 ((1<<2)|(1<<6)|(1<<10)|(1<<14))
#define COLUMN3 ((1<<3)|(1<<7)|(1<<11)|(1<<15))

#define ROW0 ((1<<0) |(1<<1) |(1<<2) |(1<<3))
#define ROW1 ((1<<4) |(1<<5) |(1<<6) |(1<<7))
#define ROW2 ((1<<8) |(1<<9) |(1<<10)|(1<<11))
#define ROW3 ((1<<12)|(1<<13)|(1<<14)|(1<<15))

#define STAMP_SIZE 4
static unsigned bottom_mask_tab[STAMP_SIZE] = {
   ROW3,
   ROW3 | ROW2,
   ROW3 | ROW2 | ROW1,
   ROW3 | ROW2 | ROW1 | ROW0,
};

static unsigned right_mask_tab[STAMP_SIZE] = {
   COLUMN3,
   COLUMN3 | COLUMN2,
   COLUMN3 | COLUMN2 | COLUMN1,
   COLUMN3 | COLUMN2 | COLUMN1 | COLUMN0,
};


#define NR_PLANES 3

void
lp_rast_triangle_32_3_16(struct lp_rasterizer_task *task,
                         const union lp_rast_cmd_arg arg)
{
   const struct lp_rast_triangle *tri = arg.triangle.tri;
   const struct lp_rast_plane *plane = GET_PLANES(tri);
   const int x = (arg.triangle.plane_mask & 0xff) + task->x;
   const int y = (arg.triangle.plane_mask >> 8) + task->y;

   struct { unsigned mask:16; unsigned i:8; unsigned j:8; } out[16];
   unsigned nr = 0;

   /* p0 and p2 are aligned, p1 is not (plane size 24 bytes). */
   __m128i p0 = _mm_load_si128((__m128i *)&plane[0]); /* clo, chi, dcdx, dcdy */
   __m128i p1 = _mm_loadu_si128((__m128i *)&plane[1]);
   __m128i p2 = _mm_load_si128((__m128i *)&plane[2]);
   __m128i zero = _mm_setzero_si128();

   __m128i c, dcdx, dcdy, rej4;
   __m128i dcdx_neg_mask, dcdy_neg_mask;
   __m128i dcdx2, dcdx3;

   __m128i span_0;                /* 0,dcdx,2dcdx,3dcdx for plane 0 */
   __m128i span_1;                /* 0,dcdx,2dcdx,3dcdx for plane 1 */
   __m128i span_2;                /* 0,dcdx,2dcdx,3dcdx for plane 2 */
   __m128i unused;

   transpose4_epi32(&p0, &p1, &p2, &zero,
                    &c, &unused, &dcdx, &dcdy);

   /* recalc eo - easier than trying to load as scalars / shuffle... */
   dcdx_neg_mask = _mm_srai_epi32(dcdx, 31);
   dcdy_neg_mask = _mm_srai_epi32(dcdy, 31);
   rej4 = _mm_sub_epi32(_mm_andnot_si128(dcdy_neg_mask, dcdy),
                        _mm_and_si128(dcdx_neg_mask, dcdx));

   /* Adjust dcdx;
    */
   dcdx = _mm_sub_epi32(zero, dcdx);

   c = _mm_add_epi32(c, mm_mullo_epi32(dcdx, _mm_set1_epi32(x)));
   c = _mm_add_epi32(c, mm_mullo_epi32(dcdy, _mm_set1_epi32(y)));
   rej4 = _mm_slli_epi32(rej4, 2);

   /* Adjust so we can just check the sign bit (< 0 comparison), instead of having to do a less efficient <= 0 comparison */
   c = _mm_sub_epi32(c, _mm_set1_epi32(1));
   rej4 = _mm_add_epi32(rej4, _mm_set1_epi32(1));

   dcdx2 = _mm_add_epi32(dcdx, dcdx);
   dcdx3 = _mm_add_epi32(dcdx2, dcdx);

   transpose4_epi32(&zero, &dcdx, &dcdx2, &dcdx3,
                    &span_0, &span_1, &span_2, &unused);

   for (unsigned i = 0; i < 4; i++) {
      __m128i cx = c;

      for (unsigned j = 0; j < 4; j++) {
         __m128i c4rej = _mm_add_epi32(cx, rej4);
         __m128i rej_masks = _mm_srai_epi32(c4rej, 31);

         /* if (is_zero(rej_masks)) */
         if (_mm_movemask_epi8(rej_masks) == 0) {
            __m128i c0_0 = _mm_add_epi32(SCALAR_EPI32(cx, 0), span_0);
            __m128i c1_0 = _mm_add_epi32(SCALAR_EPI32(cx, 1), span_1);
            __m128i c2_0 = _mm_add_epi32(SCALAR_EPI32(cx, 2), span_2);

            __m128i c_0 = _mm_or_si128(_mm_or_si128(c0_0, c1_0), c2_0);

            __m128i c0_1 = _mm_add_epi32(c0_0, SCALAR_EPI32(dcdy, 0));
            __m128i c1_1 = _mm_add_epi32(c1_0, SCALAR_EPI32(dcdy, 1));
            __m128i c2_1 = _mm_add_epi32(c2_0, SCALAR_EPI32(dcdy, 2));

            __m128i c_1 = _mm_or_si128(_mm_or_si128(c0_1, c1_1), c2_1);
            __m128i c_01 = _mm_packs_epi32(c_0, c_1);

            __m128i c0_2 = _mm_add_epi32(c0_1, SCALAR_EPI32(dcdy, 0));
            __m128i c1_2 = _mm_add_epi32(c1_1, SCALAR_EPI32(dcdy, 1));
            __m128i c2_2 = _mm_add_epi32(c2_1, SCALAR_EPI32(dcdy, 2));

            __m128i c_2 = _mm_or_si128(_mm_or_si128(c0_2, c1_2), c2_2);

            __m128i c0_3 = _mm_add_epi32(c0_2, SCALAR_EPI32(dcdy, 0));
            __m128i c1_3 = _mm_add_epi32(c1_2, SCALAR_EPI32(dcdy, 1));
            __m128i c2_3 = _mm_add_epi32(c2_2, SCALAR_EPI32(dcdy, 2));

            __m128i c_3 = _mm_or_si128(_mm_or_si128(c0_3, c1_3), c2_3);
            __m128i c_23 = _mm_packs_epi32(c_2, c_3);
            __m128i c_0123 = _mm_packs_epi16(c_01, c_23);

            unsigned mask = _mm_movemask_epi8(c_0123);

            out[nr].i = i;
            out[nr].j = j;
            out[nr].mask = mask;
            if (mask != 0xffff)
               nr++;
         }
         cx = _mm_add_epi32(cx, _mm_slli_epi32(dcdx, 2));
      }

      c = _mm_add_epi32(c, _mm_slli_epi32(dcdy, 2));
   }

   for (unsigned i = 0; i < nr; i++)
      lp_rast_shade_quads_mask(task,
                               &tri->inputs,
                               x + 4 * out[i].j,
                               y + 4 * out[i].i,
                               0xffff & ~out[i].mask);
}

void
lp_rast_triangle_32_3_4(struct lp_rasterizer_task *task,
                        const union lp_rast_cmd_arg arg)
{
   const struct lp_rast_triangle *tri = arg.triangle.tri;
   const struct lp_rast_plane *plane = GET_PLANES(tri);
   const unsigned x = (arg.triangle.plane_mask & 0xff) + task->x;
   const unsigned y = (arg.triangle.plane_mask >> 8) + task->y;

   /* p0 and p2 are aligned, p1 is not (plane size 24 bytes). */
   __m128i p0 = _mm_load_si128((__m128i *)&plane[0]); /* clo, chi, dcdx, dcdy */
   __m128i p1 = _mm_loadu_si128((__m128i *)&plane[1]);
   __m128i p2 = _mm_load_si128((__m128i *)&plane[2]);
   __m128i zero = _mm_setzero_si128();

   __m128i c, dcdx, dcdy;
   __m128i dcdx2, dcdx3;

   __m128i span_0;                /* 0,dcdx,2dcdx,3dcdx for plane 0 */
   __m128i span_1;                /* 0,dcdx,2dcdx,3dcdx for plane 1 */
   __m128i span_2;                /* 0,dcdx,2dcdx,3dcdx for plane 2 */
   __m128i unused;

   transpose4_epi32(&p0, &p1, &p2, &zero,
                    &c, &unused, &dcdx, &dcdy);

   /* Adjust dcdx;
    */
   dcdx = _mm_sub_epi32(zero, dcdx);

   c = _mm_add_epi32(c, mm_mullo_epi32(dcdx, _mm_set1_epi32(x)));
   c = _mm_add_epi32(c, mm_mullo_epi32(dcdy, _mm_set1_epi32(y)));

   /* Adjust so we can just check the sign bit (< 0 comparison), instead of having to do a less efficient <= 0 comparison */
   c = _mm_sub_epi32(c, _mm_set1_epi32(1));

   dcdx2 = _mm_add_epi32(dcdx, dcdx);
   dcdx3 = _mm_add_epi32(dcdx2, dcdx);

   transpose4_epi32(&zero, &dcdx, &dcdx2, &dcdx3,
                    &span_0, &span_1, &span_2, &unused);


   {
      __m128i c0_0 = _mm_add_epi32(SCALAR_EPI32(c, 0), span_0);
      __m128i c1_0 = _mm_add_epi32(SCALAR_EPI32(c, 1), span_1);
      __m128i c2_0 = _mm_add_epi32(SCALAR_EPI32(c, 2), span_2);

      __m128i c_0 = _mm_or_si128(_mm_or_si128(c0_0, c1_0), c2_0);

      __m128i c0_1 = _mm_add_epi32(c0_0, SCALAR_EPI32(dcdy, 0));
      __m128i c1_1 = _mm_add_epi32(c1_0, SCALAR_EPI32(dcdy, 1));
      __m128i c2_1 = _mm_add_epi32(c2_0, SCALAR_EPI32(dcdy, 2));

      __m128i c_1 = _mm_or_si128(_mm_or_si128(c0_1, c1_1), c2_1);
      __m128i c_01 = _mm_packs_epi32(c_0, c_1);

      __m128i c0_2 = _mm_add_epi32(c0_1, SCALAR_EPI32(dcdy, 0));
      __m128i c1_2 = _mm_add_epi32(c1_1, SCALAR_EPI32(dcdy, 1));
      __m128i c2_2 = _mm_add_epi32(c2_1, SCALAR_EPI32(dcdy, 2));

      __m128i c_2 = _mm_or_si128(_mm_or_si128(c0_2, c1_2), c2_2);

      __m128i c0_3 = _mm_add_epi32(c0_2, SCALAR_EPI32(dcdy, 0));
      __m128i c1_3 = _mm_add_epi32(c1_2, SCALAR_EPI32(dcdy, 1));
      __m128i c2_3 = _mm_add_epi32(c2_2, SCALAR_EPI32(dcdy, 2));

      __m128i c_3 = _mm_or_si128(_mm_or_si128(c0_3, c1_3), c2_3);
      __m128i c_23 = _mm_packs_epi32(c_2, c_3);
      __m128i c_0123 = _mm_packs_epi16(c_01, c_23);

      unsigned mask = _mm_movemask_epi8(c_0123);

      if (mask != 0xffff)
         lp_rast_shade_quads_mask(task,
                                  &tri->inputs,
                                  x,
                                  y,
                                  0xffff & ~mask);
   }
}

#undef NR_PLANES

#else

#if defined(_ARCH_PWR8) && UTIL_ARCH_LITTLE_ENDIAN

#include <altivec.h>
#include "util/u_pwr8.h"

static inline void
build_masks_ppc(int c,
                int cdiff,
                int dcdx,
                int dcdy,
                unsigned *outmask,
                unsigned *partmask)
{
   __m128i cstep0 = vec_setr_epi32(c, c+dcdx, c+dcdx*2, c+dcdx*3);
   __m128i xdcdy = (__m128i) vec_splats(dcdy);

   /* Get values across the quad
    */
   __m128i cstep1 = vec_add_epi32(cstep0, xdcdy);
   __m128i cstep2 = vec_add_epi32(cstep1, xdcdy);
   __m128i cstep3 = vec_add_epi32(cstep2, xdcdy);

   {
      __m128i cstep01, cstep23, result;

      cstep01 = vec_packs_epi32(cstep0, cstep1);
      cstep23 = vec_packs_epi32(cstep2, cstep3);
      result = vec_packs_epi16(cstep01, cstep23);

      *outmask |= vec_movemask_epi8(result);
   }


   {
      __m128i cio4 = (__m128i) vec_splats(cdiff);
      __m128i cstep01, cstep23, result;

      cstep0 = vec_add_epi32(cstep0, cio4);
      cstep1 = vec_add_epi32(cstep1, cio4);
      cstep2 = vec_add_epi32(cstep2, cio4);
      cstep3 = vec_add_epi32(cstep3, cio4);

      cstep01 = vec_packs_epi32(cstep0, cstep1);
      cstep23 = vec_packs_epi32(cstep2, cstep3);
      result = vec_packs_epi16(cstep01, cstep23);

      *partmask |= vec_movemask_epi8(result);
   }
}

static inline unsigned
build_mask_linear_ppc(int c, int dcdx, int dcdy)
{
   __m128i cstep0 = vec_setr_epi32(c, c+dcdx, c+dcdx*2, c+dcdx*3);
   __m128i xdcdy = (__m128i) vec_splats(dcdy);

   /* Get values across the quad
    */
   __m128i cstep1 = vec_add_epi32(cstep0, xdcdy);
   __m128i cstep2 = vec_add_epi32(cstep1, xdcdy);
   __m128i cstep3 = vec_add_epi32(cstep2, xdcdy);

   /* pack pairs of results into epi16
    */
   __m128i cstep01 = vec_packs_epi32(cstep0, cstep1);
   __m128i cstep23 = vec_packs_epi32(cstep2, cstep3);

   /* pack into epi8, preserving sign bits
    */
   __m128i result = vec_packs_epi16(cstep01, cstep23);

   /* extract sign bits to create mask
    */
   return vec_movemask_epi8(result);
}

static inline __m128i
lp_plane_to_m128i(const struct lp_rast_plane *plane)
{
   return vec_setr_epi32((int32_t)plane->c, (int32_t)plane->dcdx,
                         (int32_t)plane->dcdy, (int32_t)plane->eo);
}

#define NR_PLANES 3

void
lp_rast_triangle_32_3_16(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   const struct lp_rast_triangle *tri = arg.triangle.tri;
   const struct lp_rast_plane *plane = GET_PLANES(tri);
   const int x = (arg.triangle.plane_mask & 0xff) + task->x;
   const int y = (arg.triangle.plane_mask >> 8) + task->y;

   struct { unsigned mask:16; unsigned i:8; unsigned j:8; } out[16];
   unsigned nr = 0;

   __m128i p0 = lp_plane_to_m128i(&plane[0]); /* c, dcdx, dcdy, eo */
   __m128i p1 = lp_plane_to_m128i(&plane[1]); /* c, dcdx, dcdy, eo */
   __m128i p2 = lp_plane_to_m128i(&plane[2]); /* c, dcdx, dcdy, eo */
   __m128i zero = vec_splats((unsigned char) 0);

   __m128i c;
   __m128i dcdx;
   __m128i dcdy;
   __m128i rej4;

   __m128i dcdx2;
   __m128i dcdx3;

   __m128i span_0;                /* 0,dcdx,2dcdx,3dcdx for plane 0 */
   __m128i span_1;                /* 0,dcdx,2dcdx,3dcdx for plane 1 */
   __m128i span_2;                /* 0,dcdx,2dcdx,3dcdx for plane 2 */
   __m128i unused;

   __m128i vshuf_mask0;
   __m128i vshuf_mask1;
   __m128i vshuf_mask2;

#if UTIL_ARCH_LITTLE_ENDIAN
   vshuf_mask0 = (__m128i) vec_splats((unsigned int) 0x03020100);
   vshuf_mask1 = (__m128i) vec_splats((unsigned int) 0x07060504);
   vshuf_mask2 = (__m128i) vec_splats((unsigned int) 0x0B0A0908);
#else
   vshuf_mask0 = (__m128i) vec_splats((unsigned int) 0x0C0D0E0F);
   vshuf_mask1 = (__m128i) vec_splats((unsigned int) 0x08090A0B);
   vshuf_mask2 = (__m128i) vec_splats((unsigned int) 0x04050607);
#endif

   transpose4_epi32(&p0, &p1, &p2, &zero,
                    &c, &dcdx, &dcdy, &rej4);

   /* Adjust dcdx;
    */
   dcdx = vec_sub_epi32(zero, dcdx);

   c = vec_add_epi32(c, vec_mullo_epi32(dcdx, (__m128i) vec_splats(x)));
   c = vec_add_epi32(c, vec_mullo_epi32(dcdy, (__m128i) vec_splats(y)));
   rej4 = vec_slli_epi32(rej4, 2);

   /*
    * Adjust so we can just check the sign bit (< 0 comparison),
    * instead of having to do a less efficient <= 0 comparison
    */
   c = vec_sub_epi32(c, (__m128i) vec_splats((unsigned int) 1));
   rej4 = vec_add_epi32(rej4, (__m128i) vec_splats((unsigned int) 1));

   dcdx2 = vec_add_epi32(dcdx, dcdx);
   dcdx3 = vec_add_epi32(dcdx2, dcdx);

   transpose4_epi32(&zero, &dcdx, &dcdx2, &dcdx3,
                    &span_0, &span_1, &span_2, &unused);

   for (unsigned i = 0; i < 4; i++) {
      __m128i cx = c;

      for (unsigned j = 0; j < 4; j++) {
         __m128i c4rej = vec_add_epi32(cx, rej4);
         __m128i rej_masks = vec_srai_epi32(c4rej, 31);

         /* if (is_zero(rej_masks)) */
         if (vec_movemask_epi8(rej_masks) == 0) {
            __m128i c0_0 = vec_add_epi32(vec_perm(cx, cx, vshuf_mask0), span_0);
            __m128i c1_0 = vec_add_epi32(vec_perm(cx, cx, vshuf_mask1), span_1);
            __m128i c2_0 = vec_add_epi32(vec_perm(cx, cx, vshuf_mask2), span_2);

            __m128i c_0 = vec_or(vec_or(c0_0, c1_0), c2_0);

            __m128i c0_1 = vec_add_epi32(c0_0, vec_perm(dcdy, dcdy, vshuf_mask0));
            __m128i c1_1 = vec_add_epi32(c1_0, vec_perm(dcdy, dcdy, vshuf_mask1));
            __m128i c2_1 = vec_add_epi32(c2_0, vec_perm(dcdy, dcdy, vshuf_mask2));

            __m128i c_1 = vec_or(vec_or(c0_1, c1_1), c2_1);
            __m128i c_01 = vec_packs_epi32(c_0, c_1);

            __m128i c0_2 = vec_add_epi32(c0_1, vec_perm(dcdy, dcdy, vshuf_mask0));
            __m128i c1_2 = vec_add_epi32(c1_1, vec_perm(dcdy, dcdy, vshuf_mask1));
            __m128i c2_2 = vec_add_epi32(c2_1, vec_perm(dcdy, dcdy, vshuf_mask2));

            __m128i c_2 = vec_or(vec_or(c0_2, c1_2), c2_2);

            __m128i c0_3 = vec_add_epi32(c0_2, vec_perm(dcdy, dcdy, vshuf_mask0));
            __m128i c1_3 = vec_add_epi32(c1_2, vec_perm(dcdy, dcdy, vshuf_mask1));
            __m128i c2_3 = vec_add_epi32(c2_2, vec_perm(dcdy, dcdy, vshuf_mask2));

            __m128i c_3 = vec_or(vec_or(c0_3, c1_3), c2_3);
            __m128i c_23 = vec_packs_epi32(c_2, c_3);
            __m128i c_0123 = vec_packs_epi16(c_01, c_23);

            unsigned mask = vec_movemask_epi8(c_0123);

            out[nr].i = i;
            out[nr].j = j;
            out[nr].mask = mask;
            if (mask != 0xffff)
               nr++;
         }
         cx = vec_add_epi32(cx, vec_slli_epi32(dcdx, 2));
      }

      c = vec_add_epi32(c, vec_slli_epi32(dcdy, 2));
   }

   for (unsigned i = 0; i < nr; i++)
      lp_rast_shade_quads_mask(task,
                               &tri->inputs,
                               x + 4 * out[i].j,
                               y + 4 * out[i].i,
                               0xffff & ~out[i].mask);
}

#undef NR_PLANES

#else

void
lp_rast_triangle_32_3_16(struct lp_rasterizer_task *task,
                         const union lp_rast_cmd_arg arg)
{
   union lp_rast_cmd_arg arg2;
   arg2.triangle.tri = arg.triangle.tri;
   arg2.triangle.plane_mask = (1<<3)-1;
   lp_rast_triangle_32_3(task, arg2);
}

#endif /* _ARCH_PWR8 && UTIL_ARCH_LITTLE_ENDIAN */

void
lp_rast_triangle_32_4_16(struct lp_rasterizer_task *task,
                         const union lp_rast_cmd_arg arg)
{
   union lp_rast_cmd_arg arg2;
   arg2.triangle.tri = arg.triangle.tri;
   arg2.triangle.plane_mask = (1<<4)-1;
   lp_rast_triangle_32_4(task, arg2);
}

void
lp_rast_triangle_32_3_4(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   lp_rast_triangle_32_3_16(task, arg);
}

#endif

#if DETECT_ARCH_SSE
#define BUILD_MASKS(c, cdiff, dcdx, dcdy, omask, pmask) build_masks_sse((int)c, (int)cdiff, dcdx, dcdy, omask, pmask)
#define BUILD_MASK_LINEAR(c, dcdx, dcdy) build_mask_linear_sse((int)c, dcdx, dcdy)
#elif (defined(_ARCH_PWR8) && UTIL_ARCH_LITTLE_ENDIAN)
#define BUILD_MASKS(c, cdiff, dcdx, dcdy, omask, pmask) build_masks_ppc((int)c, (int)cdiff, dcdx, dcdy, omask, pmask)
#define BUILD_MASK_LINEAR(c, dcdx, dcdy) build_mask_linear_ppc((int)c, dcdx, dcdy)
#else
#define BUILD_MASKS(c, cdiff, dcdx, dcdy, omask, pmask) build_masks(c, cdiff, dcdx, dcdy, omask, pmask)
#define BUILD_MASK_LINEAR(c, dcdx, dcdy) build_mask_linear(c, dcdx, dcdy)
#endif

#define RASTER_64 1

#define TAG(x) x##_1
#define NR_PLANES 1
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_2
#define NR_PLANES 2
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_3
#define NR_PLANES 3
/*#define TRI_4 lp_rast_triangle_3_4*/
/*#define TRI_16 lp_rast_triangle_3_16*/
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_4
#define NR_PLANES 4
/*#define TRI_16 lp_rast_triangle_4_16*/
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_5
#define NR_PLANES 5
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_6
#define NR_PLANES 6
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_7
#define NR_PLANES 7
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_8
#define NR_PLANES 8
#include "lp_rast_tri_tmp.h"

#undef RASTER_64

#define TAG(x) x##_32_1
#define NR_PLANES 1
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_32_2
#define NR_PLANES 2
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_32_3
#define NR_PLANES 3
/*#define TRI_4 lp_rast_triangle_3_4*/
/*#define TRI_16 lp_rast_triangle_3_16*/
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_32_4
#define NR_PLANES 4
#if DETECT_ARCH_SSE
#define TRI_16 lp_rast_triangle_32_4_16
#endif
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_32_5
#define NR_PLANES 5
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_32_6
#define NR_PLANES 6
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_32_7
#define NR_PLANES 7
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_32_8
#define NR_PLANES 8
#include "lp_rast_tri_tmp.h"

#define MULTISAMPLE 1
#define RASTER_64 1

#define TAG(x) x##_ms_1
#define NR_PLANES 1
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_ms_2
#define NR_PLANES 2
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_ms_3
#define NR_PLANES 3
/*#define TRI_4 lp_rast_triangle_3_4*/
/*#define TRI_16 lp_rast_triangle_3_16*/
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_ms_4
#define NR_PLANES 4
/*#define TRI_16 lp_rast_triangle_4_16*/
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_ms_5
#define NR_PLANES 5
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_ms_6
#define NR_PLANES 6
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_ms_7
#define NR_PLANES 7
#include "lp_rast_tri_tmp.h"

#define TAG(x) x##_ms_8
#define NR_PLANES 8
#include "lp_rast_tri_tmp.h"

#undef RASTER_64
