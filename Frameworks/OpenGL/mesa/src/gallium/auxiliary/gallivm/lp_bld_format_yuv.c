/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
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


/**
 * @file
 * YUV pixel format manipulation.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#include "util/format/u_format.h"
#include "util/u_cpu_detect.h"

#include "lp_bld_arit.h"
#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_conv.h"
#include "lp_bld_gather.h"
#include "lp_bld_format.h"
#include "lp_bld_init.h"
#include "lp_bld_logic.h"

/**
 * Extract Y, U, V channels from packed UYVY.
 * @param packed  is a <n x i32> vector with the packed UYVY blocks
 * @param i  is a <n x i32> vector with the x pixel coordinate (0 or 1)
 */
static void
uyvy_to_yuv_soa(struct gallivm_state *gallivm,
                unsigned n,
                LLVMValueRef packed,
                LLVMValueRef i,
                LLVMValueRef *y,
                LLVMValueRef *u,
                LLVMValueRef *v)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type type;
   LLVMValueRef mask;

   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   assert(lp_check_value(type, packed));
   assert(lp_check_value(type, i));

   /*
    * Little endian:
    * y = (uyvy >> (16*i + 8)) & 0xff
    * u = (uyvy        ) & 0xff
    * v = (uyvy >> 16  ) & 0xff
    *
    * Big endian:
    * y = (uyvy >> (-16*i + 16)) & 0xff
    * u = (uyvy >> 24) & 0xff
    * v = (uyvy >>  8) & 0xff
    */

#if DETECT_ARCH_X86 || DETECT_ARCH_X86_64
   /*
    * Avoid shift with per-element count.
    * No support on x86, gets translated to roughly 5 instructions
    * per element. Didn't measure performance but cuts shader size
    * by quite a bit (less difference if cpu has no sse4.1 support).
    */
   if (util_get_cpu_caps()->has_sse2 && n > 1) {
      LLVMValueRef sel, tmp, tmp2;
      struct lp_build_context bld32;

      lp_build_context_init(&bld32, gallivm, type);

      tmp = LLVMBuildLShr(builder, packed, lp_build_const_int_vec(gallivm, type, 8), "");
      tmp2 = LLVMBuildLShr(builder, tmp, lp_build_const_int_vec(gallivm, type, 16), "");
      sel = lp_build_compare(gallivm, type, PIPE_FUNC_EQUAL, i, lp_build_const_int_vec(gallivm, type, 0));
      *y = lp_build_select(&bld32, sel, tmp, tmp2);
   } else
#endif
   {
      LLVMValueRef shift;
#if UTIL_ARCH_LITTLE_ENDIAN
      shift = LLVMBuildMul(builder, i, lp_build_const_int_vec(gallivm, type, 16), "");
      shift = LLVMBuildAdd(builder, shift, lp_build_const_int_vec(gallivm, type, 8), "");
#else
      shift = LLVMBuildMul(builder, i, lp_build_const_int_vec(gallivm, type, -16), "");
      shift = LLVMBuildAdd(builder, shift, lp_build_const_int_vec(gallivm, type, 16), "");
#endif
      *y = LLVMBuildLShr(builder, packed, shift, "");
   }

#if UTIL_ARCH_LITTLE_ENDIAN
   *u = packed;
   *v = LLVMBuildLShr(builder, packed, lp_build_const_int_vec(gallivm, type, 16), "");
#else
   *u = LLVMBuildLShr(builder, packed, lp_build_const_int_vec(gallivm, type, 24), "");
   *v = LLVMBuildLShr(builder, packed, lp_build_const_int_vec(gallivm, type, 8), "");
#endif

   mask = lp_build_const_int_vec(gallivm, type, 0xff);

   *y = LLVMBuildAnd(builder, *y, mask, "y");
   *u = LLVMBuildAnd(builder, *u, mask, "u");
   *v = LLVMBuildAnd(builder, *v, mask, "v");
}


/**
 * Extract Y, U, V channels from packed YUYV.
 * @param packed  is a <n x i32> vector with the packed YUYV blocks
 * @param i  is a <n x i32> vector with the x pixel coordinate (0 or 1)
 */
static void
yuyv_to_yuv_soa(struct gallivm_state *gallivm,
                unsigned n,
                LLVMValueRef packed,
                LLVMValueRef i,
                LLVMValueRef *y,
                LLVMValueRef *u,
                LLVMValueRef *v)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type type;
   LLVMValueRef mask;

   memset(&type, 0, sizeof type);
   type.width = 32;
   type.length = n;

   assert(lp_check_value(type, packed));
   assert(lp_check_value(type, i));

   /*
   * Little endian:
    * y = (yuyv >> 16*i) & 0xff
    * u = (yuyv >> 8   ) & 0xff
    * v = (yuyv >> 24  ) & 0xff
    *
    * Big endian:
    * y = (yuyv >> (-16*i + 24) & 0xff
    * u = (yuyv >> 16)          & 0xff
    * v = (yuyv)                & 0xff
    */

#if DETECT_ARCH_X86 || DETECT_ARCH_X86_64
   /*
    * Avoid shift with per-element count.
    * No support on x86, gets translated to roughly 5 instructions
    * per element. Didn't measure performance but cuts shader size
    * by quite a bit (less difference if cpu has no sse4.1 support).
    */
   if (util_get_cpu_caps()->has_sse2 && n > 1) {
      LLVMValueRef sel, tmp;
      struct lp_build_context bld32;

      lp_build_context_init(&bld32, gallivm, type);

      tmp = LLVMBuildLShr(builder, packed, lp_build_const_int_vec(gallivm, type, 16), "");
      sel = lp_build_compare(gallivm, type, PIPE_FUNC_EQUAL, i, lp_build_const_int_vec(gallivm, type, 0));
       *y = lp_build_select(&bld32, sel, packed, tmp);
   } else
#endif
   {
      LLVMValueRef shift;
#if UTIL_ARCH_LITTLE_ENDIAN
      shift = LLVMBuildMul(builder, i, lp_build_const_int_vec(gallivm, type, 16), "");
#else
      shift = LLVMBuildMul(builder, i, lp_build_const_int_vec(gallivm, type, -16), "");
      shift = LLVMBuildAdd(builder, shift, lp_build_const_int_vec(gallivm, type, 24), "");
#endif
      *y = LLVMBuildLShr(builder, packed, shift, "");
   }

#if UTIL_ARCH_LITTLE_ENDIAN
   *u = LLVMBuildLShr(builder, packed, lp_build_const_int_vec(gallivm, type, 8), "");
   *v = LLVMBuildLShr(builder, packed, lp_build_const_int_vec(gallivm, type, 24), "");
#else
   *u = LLVMBuildLShr(builder, packed, lp_build_const_int_vec(gallivm, type, 16), "");
   *v = packed;
#endif

   mask = lp_build_const_int_vec(gallivm, type, 0xff);

   *y = LLVMBuildAnd(builder, *y, mask, "y");
   *u = LLVMBuildAnd(builder, *u, mask, "u");
   *v = LLVMBuildAnd(builder, *v, mask, "v");
}


static inline void
yuv_to_rgb_soa(struct gallivm_state *gallivm,
               unsigned n,
               LLVMValueRef y, LLVMValueRef u, LLVMValueRef v,
               LLVMValueRef *r, LLVMValueRef *g, LLVMValueRef *b)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type type;
   struct lp_build_context bld;

   LLVMValueRef c0;
   LLVMValueRef c8;
   LLVMValueRef c16;
   LLVMValueRef c128;
   LLVMValueRef c255;

   LLVMValueRef cy;
   LLVMValueRef cug;
   LLVMValueRef cub;
   LLVMValueRef cvr;
   LLVMValueRef cvg;

   memset(&type, 0, sizeof type);
   type.sign = true;
   type.width = 32;
   type.length = n;

   lp_build_context_init(&bld, gallivm, type);

   assert(lp_check_value(type, y));
   assert(lp_check_value(type, u));
   assert(lp_check_value(type, v));

   /*
    * Constants
    */

   c0   = lp_build_const_int_vec(gallivm, type,   0);
   c8   = lp_build_const_int_vec(gallivm, type,   8);
   c16  = lp_build_const_int_vec(gallivm, type,  16);
   c128 = lp_build_const_int_vec(gallivm, type, 128);
   c255 = lp_build_const_int_vec(gallivm, type, 255);

   cy  = lp_build_const_int_vec(gallivm, type,  298);
   cug = lp_build_const_int_vec(gallivm, type, -100);
   cub = lp_build_const_int_vec(gallivm, type,  516);
   cvr = lp_build_const_int_vec(gallivm, type,  409);
   cvg = lp_build_const_int_vec(gallivm, type, -208);

   /*
    *  y -= 16;
    *  u -= 128;
    *  v -= 128;
    */

   y = LLVMBuildSub(builder, y, c16, "");
   u = LLVMBuildSub(builder, u, c128, "");
   v = LLVMBuildSub(builder, v, c128, "");

   /*
    * r = 298 * _y            + 409 * _v + 128;
    * g = 298 * _y - 100 * _u - 208 * _v + 128;
    * b = 298 * _y + 516 * _u            + 128;
    */

   y = LLVMBuildMul(builder, y, cy, "");
   y = LLVMBuildAdd(builder, y, c128, "");

   *r = LLVMBuildMul(builder, v, cvr, "");
   *g = LLVMBuildAdd(builder,
                     LLVMBuildMul(builder, u, cug, ""),
                     LLVMBuildMul(builder, v, cvg, ""),
                     "");
   *b = LLVMBuildMul(builder, u, cub, "");

   *r = LLVMBuildAdd(builder, *r, y, "");
   *g = LLVMBuildAdd(builder, *g, y, "");
   *b = LLVMBuildAdd(builder, *b, y, "");

   /*
    * r >>= 8;
    * g >>= 8;
    * b >>= 8;
    */

   *r = LLVMBuildAShr(builder, *r, c8, "r");
   *g = LLVMBuildAShr(builder, *g, c8, "g");
   *b = LLVMBuildAShr(builder, *b, c8, "b");

   /*
    * Clamp
    */

   *r = lp_build_clamp(&bld, *r, c0, c255);
   *g = lp_build_clamp(&bld, *g, c0, c255);
   *b = lp_build_clamp(&bld, *b, c0, c255);
}


static LLVMValueRef
rgb_to_rgba_aos(struct gallivm_state *gallivm,
                unsigned n,
                LLVMValueRef r, LLVMValueRef g, LLVMValueRef b)
{
   LLVMBuilderRef builder = gallivm->builder;
   struct lp_type type;
   LLVMValueRef a;
   LLVMValueRef rgba;

   memset(&type, 0, sizeof type);
   type.sign = true;
   type.width = 32;
   type.length = n;

   assert(lp_check_value(type, r));
   assert(lp_check_value(type, g));
   assert(lp_check_value(type, b));

   /*
    * Make a 4 x unorm8 vector
    */

#if UTIL_ARCH_LITTLE_ENDIAN
   g = LLVMBuildShl(builder, g, lp_build_const_int_vec(gallivm, type, 8), "");
   b = LLVMBuildShl(builder, b, lp_build_const_int_vec(gallivm, type, 16), "");
   a = lp_build_const_int_vec(gallivm, type, 0xff000000);
#else
   r = LLVMBuildShl(builder, r, lp_build_const_int_vec(gallivm, type, 24), "");
   g = LLVMBuildShl(builder, g, lp_build_const_int_vec(gallivm, type, 16), "");
   b = LLVMBuildShl(builder, b, lp_build_const_int_vec(gallivm, type, 8), "");
   a = lp_build_const_int_vec(gallivm, type, 0x000000ff);
#endif

   rgba = r;
   rgba = LLVMBuildOr(builder, rgba, g, "");
   rgba = LLVMBuildOr(builder, rgba, b, "");
   rgba = LLVMBuildOr(builder, rgba, a, "");

   rgba = LLVMBuildBitCast(builder, rgba,
                           LLVMVectorType(LLVMInt8TypeInContext(gallivm->context), 4*n), "");

   return rgba;
}


/**
 * Convert from <n x i32> packed UYVY to <4n x i8> RGBA AoS
 */
static LLVMValueRef
uyvy_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef y, u, v;
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   uyvy_to_yuv_soa(gallivm, n, packed, i, &y, &u, &v);
   yuv_to_rgb_soa(gallivm, n, y, u, v, &r, &g, &b);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}

/**
 * Convert from <n x i32> packed VYUY to <4n x i8> RGBA AoS
 */
static LLVMValueRef
vyuy_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef y, u, v;
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   /* VYUY is UYVY with U/V swapped */
   uyvy_to_yuv_soa(gallivm, n, packed, i, &y, &v, &u);
   yuv_to_rgb_soa(gallivm, n, y, u, v, &r, &g, &b);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}

/**
 * Convert from <n x i32> packed YUYV to <4n x i8> RGBA AoS
 */
static LLVMValueRef
yuyv_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef y, u, v;
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   yuyv_to_yuv_soa(gallivm, n, packed, i, &y, &u, &v);
   yuv_to_rgb_soa(gallivm, n, y, u, v, &r, &g, &b);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}

/**
 * Convert from <n x i32> packed YUYV to <4n x i8> RGBA AoS
 */
static LLVMValueRef
yvyu_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef y, u, v;
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   /* YVYU is YUYV with U/V swapped */
   yuyv_to_yuv_soa(gallivm, n, packed, i, &y, &v, &u);
   yuv_to_rgb_soa(gallivm, n, y, u, v, &r, &g, &b);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}

/**
 * Convert from <n x i32> packed RG_BG to <4n x i8> RGBA AoS
 */
static LLVMValueRef
rgbg_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   uyvy_to_yuv_soa(gallivm, n, packed, i, &g, &r, &b);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}


/**
 * Convert from <n x i32> packed GR_GB to <4n x i8> RGBA AoS
 */
static LLVMValueRef
grgb_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   yuyv_to_yuv_soa(gallivm, n, packed, i, &g, &r, &b);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}

/**
 * Convert from <n x i32> packed GR_BR to <4n x i8> RGBA AoS
 */
static LLVMValueRef
grbr_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   uyvy_to_yuv_soa(gallivm, n, packed, i, &r, &g, &b);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}


/**
 * Convert from <n x i32> packed RG_RB to <4n x i8> RGBA AoS
 */
static LLVMValueRef
rgrb_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   yuyv_to_yuv_soa(gallivm, n, packed, i, &r, &g, &b);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}

/**
 * Convert from <n x i32> packed GB_GR to <4n x i8> RGBA AoS
 */
static LLVMValueRef
gbgr_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   yuyv_to_yuv_soa(gallivm, n, packed, i, &g, &b, &r);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}

/**
 * Convert from <n x i32> packed BG_RG to <4n x i8> RGBA AoS
 */
static LLVMValueRef
bgrg_to_rgba_aos(struct gallivm_state *gallivm,
                 unsigned n,
                 LLVMValueRef packed,
                 LLVMValueRef i)
{
   LLVMValueRef r, g, b;
   LLVMValueRef rgba;

   uyvy_to_yuv_soa(gallivm, n, packed, i, &g, &b, &r);
   rgba = rgb_to_rgba_aos(gallivm, n, r, g, b);

   return rgba;
}

/**
 * @param n  is the number of pixels processed
 * @param packed  is a <n x i32> vector with the packed YUYV blocks
 * @param i  is a <n x i32> vector with the x pixel coordinate (0 or 1)
 * @return  a <4*n x i8> vector with the pixel RGBA values in AoS
 */
LLVMValueRef
lp_build_fetch_subsampled_rgba_aos(struct gallivm_state *gallivm,
                                   const struct util_format_description *format_desc,
                                   unsigned n,
                                   LLVMValueRef base_ptr,
                                   LLVMValueRef offset,
                                   LLVMValueRef i,
                                   LLVMValueRef j)
{
   LLVMValueRef packed;
   LLVMValueRef rgba;
   struct lp_type fetch_type;

   assert(format_desc->layout == UTIL_FORMAT_LAYOUT_SUBSAMPLED);
   assert(format_desc->block.bits == 32);
   assert(format_desc->block.width == 2);
   assert(format_desc->block.height == 1);

   fetch_type = lp_type_uint(32);
   packed = lp_build_gather(gallivm, n, 32, fetch_type, true, base_ptr, offset, false);

   (void)j;

   switch (format_desc->format) {
   case PIPE_FORMAT_UYVY:
      rgba = uyvy_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_VYUY:
      rgba = vyuy_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_YUYV:
      rgba = yuyv_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_YVYU:
      rgba = yvyu_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_R8G8_B8G8_UNORM:
      rgba = rgbg_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_G8R8_G8B8_UNORM:
      rgba = grgb_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_G8R8_B8R8_UNORM:
      rgba = grbr_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_R8G8_R8B8_UNORM:
      rgba = rgrb_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_G8B8_G8R8_UNORM:
      rgba = gbgr_to_rgba_aos(gallivm, n, packed, i);
      break;
   case PIPE_FORMAT_B8G8_R8G8_UNORM:
      rgba = bgrg_to_rgba_aos(gallivm, n, packed, i);
      break;
   default:
      assert(0);
      rgba =  LLVMGetUndef(LLVMVectorType(LLVMInt8TypeInContext(gallivm->context), 4*n));
      break;
   }

   return rgba;
}

