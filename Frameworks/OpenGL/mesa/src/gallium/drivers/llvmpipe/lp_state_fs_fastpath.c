/**************************************************************************
 *
 * Copyright 2010-2021 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/


#include "util/detect.h"

#include "util/u_math.h"
#include "util/u_cpu_detect.h"
#include "util/u_sse.h"

#include "lp_jit.h"
#include "lp_state_fs.h"
#include "lp_debug.h"


#if DETECT_ARCH_SSE

#include <emmintrin.h>


static void
no_op(const struct lp_jit_context *context,
      const struct lp_jit_resources *resources,
      uint32_t x,
      uint32_t y,
      uint32_t facing,
      const void *a0,
      const void *dadx,
      const void *dady,
      uint8_t **cbufs,
      uint8_t *depth,
      uint64_t mask,
      struct lp_jit_thread_data *thread_data,
      unsigned *strides,
      unsigned depth_stride,
      unsigned *color_sample_stride,
      unsigned depth_sample_stride)
{
}


/*
 * m ? a : b
 */
static inline __m128i
mm_select_si128(__m128i m, __m128i a, __m128i b)
{
   __m128i res;

   /*
    * TODO: use PBLENVB when available.
    */

   res = _mm_or_si128(_mm_and_si128(m, a),
                      _mm_andnot_si128(m, b));

   return res;
}


/*
 * *p = m ? a : *p;
 */
static inline void
mm_store_mask_si128(__m128i *p, __m128i m, __m128i a)
{
   _mm_store_si128(p, mm_select_si128(m, a, _mm_load_si128(p)));
}


/**
 * Expand the mask from a 16 bit integer to a 4 x 4 x 32 bit vector mask, ie.
 * 1 bit -> 32bits.
 */
static inline void
expand_mask(uint32_t int_mask,
            __m128i *vec_mask)
{
   __m128i inv_mask = _mm_set1_epi32(~int_mask & 0xffff);
   __m128i zero = _mm_setzero_si128();

   vec_mask[0] = _mm_and_si128(inv_mask, _mm_setr_epi32(0x0001, 0x0002, 0x0004, 0x0008));
   vec_mask[1] = _mm_and_si128(inv_mask, _mm_setr_epi32(0x0010, 0x0020, 0x0040, 0x0080));
   inv_mask = _mm_srli_epi32(inv_mask, 8);
   vec_mask[2] = _mm_and_si128(inv_mask, _mm_setr_epi32(0x0001, 0x0002, 0x0004, 0x0008));
   vec_mask[3] = _mm_and_si128(inv_mask, _mm_setr_epi32(0x0010, 0x0020, 0x0040, 0x0080));

   vec_mask[0] = _mm_cmpeq_epi32(vec_mask[0], zero);
   vec_mask[1] = _mm_cmpeq_epi32(vec_mask[1], zero);
   vec_mask[2] = _mm_cmpeq_epi32(vec_mask[2], zero);
   vec_mask[3] = _mm_cmpeq_epi32(vec_mask[3], zero);
}


/**
 * Draw opaque color (for debugging).
 */
static void
opaque_color(uint8_t **cbufs, unsigned *strides,
             uint32_t int_mask,
             uint32_t color)
{
   __m128i *cbuf = (__m128i *)cbufs[0];
   unsigned stride = strides[0] / sizeof *cbuf;
   __m128i vec_mask[4];
   __m128i vec_color = _mm_set1_epi32(color);

   expand_mask(int_mask, vec_mask);

   mm_store_mask_si128(cbuf, vec_mask[0], vec_color); cbuf += stride;
   mm_store_mask_si128(cbuf, vec_mask[1], vec_color); cbuf += stride;
   mm_store_mask_si128(cbuf, vec_mask[2], vec_color); cbuf += stride;
   mm_store_mask_si128(cbuf, vec_mask[3], vec_color);
}


/**
 * Draw opaque red (for debugging).
 */
static void
red(const struct lp_jit_context *context,
    const struct lp_jit_resources *resources,
    uint32_t x,
    uint32_t y,
    uint32_t facing,
    const void *a0,
    const void *dadx,
    const void *dady,
    uint8_t **cbufs,
    uint8_t *depth,
    uint64_t int_mask,
    struct lp_jit_thread_data *thread_data,
    unsigned *strides,
    unsigned depth_stride,
    unsigned *sample_stride,
    unsigned depth_sample_stride)
{
   opaque_color(cbufs, strides, int_mask, 0xffff0000);
   (void)facing;
   (void)depth;
   (void)thread_data;
}


/**
 * Draw opaque green (for debugging).
 */
static void
green(const struct lp_jit_context *context,
      const struct lp_jit_resources *resources,
      uint32_t x,
      uint32_t y,
      uint32_t facing,
      const void *a0,
      const void *dadx,
      const void *dady,
      uint8_t **cbufs,
      uint8_t *depth,
      uint64_t int_mask,
      struct lp_jit_thread_data *thread_data,
      unsigned *strides,
      unsigned depth_stride,
      unsigned *sample_stride,
      unsigned depth_sample_stride)
{
   opaque_color(cbufs, strides, int_mask, 0xff00ff00);
   (void)facing;
   (void)depth;
   (void)thread_data;
}


void
llvmpipe_fs_variant_fastpath(struct lp_fragment_shader_variant *variant)
{
   variant->jit_function[RAST_WHOLE]     = NULL;
   variant->jit_function[RAST_EDGE_TEST] = NULL;

   if (LP_DEBUG & DEBUG_NO_FASTPATH)
      return;

   if (variant->key.cbuf_format[0] != PIPE_FORMAT_B8G8R8A8_UNORM &&
       variant->key.cbuf_format[0] != PIPE_FORMAT_B8G8R8X8_UNORM) {
      return;
   }

   if (0) {
      variant->jit_function[RAST_WHOLE]     = red;
      variant->jit_function[RAST_EDGE_TEST] = red;
   }

   if (0) {
      variant->jit_function[RAST_WHOLE]     = green;
      variant->jit_function[RAST_EDGE_TEST] = green;
   }

   if (0) {
      variant->jit_function[RAST_WHOLE]     = no_op;
      variant->jit_function[RAST_EDGE_TEST] = no_op;
   }

   /* Make it easier to see triangles:
    */
   if ((LP_DEBUG & DEBUG_LINEAR) || (LP_PERF & PERF_NO_SHADE)) {
      variant->jit_function[RAST_EDGE_TEST] = red;
      variant->jit_function[RAST_WHOLE] = green;
   }
}

#else

void
llvmpipe_fs_variant_fastpath(struct lp_fragment_shader_variant *variant)
{
}

#endif
