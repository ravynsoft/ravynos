/*
 * Copyright 2015 Intel Corporation
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#ifndef ISL_PRIV_H
#define ISL_PRIV_H

#include <assert.h>
#include <stddef.h>
#include <strings.h>

#include "dev/intel_device_info.h"
#include "util/macros.h"

#include "isl.h"

typedef void (*isl_surf_fill_state_s_func)(
   const struct isl_device *dev, void *state,
   const struct isl_surf_fill_state_info *restrict info);

typedef void (*isl_buffer_fill_state_s_func)(
   const struct isl_device *dev, void *state,
   const struct isl_buffer_fill_state_info *restrict info);

typedef void (*isl_emit_depth_stencil_hiz_s_func)(
   const struct isl_device *dev, void *state,
   const struct isl_depth_stencil_hiz_emit_info *restrict info);

typedef void (*isl_null_fill_state_s_func)(const struct isl_device *dev, void *state,
                                           const struct isl_null_fill_state_info *restrict info);

typedef void (*isl_emit_cpb_control_s_func)(const struct isl_device *dev, void *batch,
                                            const struct isl_cpb_emit_info *restrict info);

#define isl_genX_declare_get_func(func)                                 \
   static inline isl_##func##_func                                      \
   isl_##func##_get_func(const struct isl_device *dev) {                \
      switch (ISL_GFX_VERX10(dev)) {                                    \
      case 40:                                                          \
         return isl_gfx4_##func;                                        \
      case 45:                                                          \
         /* G45 surface state is the same as gfx5 */                    \
      case 50:                                                          \
         return isl_gfx5_##func;                                        \
      case 60:                                                          \
         return isl_gfx6_##func;                                        \
      case 70:                                                          \
         return isl_gfx7_##func;                                        \
      case 75:                                                          \
         return isl_gfx75_##func;                                       \
      case 80:                                                          \
         return isl_gfx8_##func;                                        \
      case 90:                                                          \
         return isl_gfx9_##func;                                        \
      case 110:                                                         \
         return isl_gfx11_##func;                                       \
      case 120:                                                         \
         return isl_gfx12_##func;                                       \
      case 125:                                                         \
         return isl_gfx125_##func;                                      \
      case 200:                                                         \
         return isl_gfx20_##func;                                       \
      default:                                                          \
         assert(!"Unknown hardware generation");                        \
         return NULL;                                                   \
      }                                                                 \
   }

#define isl_finishme(format, ...) \
   do { \
      static bool reported = false; \
      if (!reported) { \
         __isl_finishme(__FILE__, __LINE__, format, ##__VA_ARGS__); \
         reported = true; \
      } \
   } while (0)

void PRINTFLIKE(3, 4) UNUSED
__isl_finishme(const char *file, int line, const char *fmt, ...);

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef void *(*isl_mem_copy_fn)(void *dest, const void *src, size_t n);

static inline bool
isl_is_pow2(uintmax_t n)
{
   return !(n & (n - 1));
}

/**
 * Alignment must be a power of 2.
 */
static inline bool
isl_is_aligned(uintmax_t n, uintmax_t a)
{
   assert(isl_is_pow2(a));
   return (n & (a - 1)) == 0;
}

/**
 * Alignment must be a power of 2.
 */
static inline uintmax_t
isl_align(uintmax_t n, uintmax_t a)
{
   assert(a != 0 && isl_is_pow2(a));
   return (n + a - 1) & ~(a - 1);
}

static inline uintmax_t
isl_align_npot(uintmax_t n, uintmax_t a)
{
   assert(a > 0);
   return ((n + a - 1) / a) * a;
}

static inline uintmax_t
isl_assert_div(uintmax_t n, uintmax_t a)
{
   assert(n % a == 0);
   return n / a;
}

/**
 * Alignment must be a power of 2.
 */
static inline uintmax_t
isl_align_div(uintmax_t n, uintmax_t a)
{
   return isl_align(n, a) / a;
}

static inline uintmax_t
isl_align_div_npot(uintmax_t n, uintmax_t a)
{
   return isl_align_npot(n, a) / a;
}

/**
 * Log base 2, rounding towards zero.
 */
static inline uint32_t
isl_log2u(uint32_t n)
{
   assert(n != 0);
   return 31 - __builtin_clz(n);
}

static inline uint32_t
isl_round_up_to_power_of_two(uint32_t value)
{
   if (value <= 1)
      return value;

   return 1 << (32 - __builtin_clz(value - 1));
}

static inline uint32_t
isl_minify(uint32_t n, uint32_t levels)
{
   if (unlikely(n == 0))
      return 0;
   else
      return MAX(n >> levels, 1);
}

static inline struct isl_extent3d
isl_extent3d_sa_to_el(enum isl_format fmt, struct isl_extent3d extent_sa)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   assert(extent_sa.w % fmtl->bw == 0);
   assert(extent_sa.h % fmtl->bh == 0);
   assert(extent_sa.d % fmtl->bd == 0);

   return (struct isl_extent3d) {
      .w = extent_sa.w / fmtl->bw,
      .h = extent_sa.h / fmtl->bh,
      .d = extent_sa.d / fmtl->bd,
   };
}

static inline struct isl_extent3d
isl_extent3d_el_to_sa(enum isl_format fmt, struct isl_extent3d extent_el)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return (struct isl_extent3d) {
      .w = extent_el.w * fmtl->bw,
      .h = extent_el.h * fmtl->bh,
      .d = extent_el.d * fmtl->bd,
   };
}

void
_isl_memcpy_linear_to_tiled(uint32_t xt1, uint32_t xt2,
                            uint32_t yt1, uint32_t yt2,
                            char *dst, const char *src,
                            uint32_t dst_pitch, int32_t src_pitch,
                            bool has_swizzling,
                            enum isl_tiling tiling,
                            isl_memcpy_type copy_type);

void
_isl_memcpy_tiled_to_linear(uint32_t xt1, uint32_t xt2,
                            uint32_t yt1, uint32_t yt2,
                            char *dst, const char *src,
                            int32_t dst_pitch, uint32_t src_pitch,
                            bool has_swizzling,
                            enum isl_tiling tiling,
                            isl_memcpy_type copy_type);

void
_isl_memcpy_linear_to_tiled_sse41(uint32_t xt1, uint32_t xt2,
                                  uint32_t yt1, uint32_t yt2,
                                  char *dst, const char *src,
                                  uint32_t dst_pitch, int32_t src_pitch,
                                  bool has_swizzling,
                                  enum isl_tiling tiling,
                                  isl_memcpy_type copy_type);

void
_isl_memcpy_tiled_to_linear_sse41(uint32_t xt1, uint32_t xt2,
                                  uint32_t yt1, uint32_t yt2,
                                  char *dst, const char *src,
                                  int32_t dst_pitch, uint32_t src_pitch,
                                  bool has_swizzling,
                                  enum isl_tiling tiling,
                                  isl_memcpy_type copy_type);

void PRINTFLIKE(4, 5)
_isl_notify_failure(const struct isl_surf_init_info *surf_info,
                    const char *file, int line, const char *fmt, ...);

#define notify_failure(surf_info, ...) \
   (_isl_notify_failure(surf_info, __FILE__, __LINE__, __VA_ARGS__), false)


/* This is useful for adding the isl_prefix to genX functions */
#define isl_genX(x) CONCAT2(isl_, genX(x))

#ifdef genX
#  include "isl_genX_priv.h"
#else
#  define genX(x) gfx4_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx5_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx6_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx7_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx75_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx8_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx9_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx11_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx12_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx125_##x
#  include "isl_genX_priv.h"
#  undef genX
#  define genX(x) gfx20_##x
#  include "isl_genX_priv.h"
#  undef genX
#endif

#endif /* ISL_PRIV_H */
