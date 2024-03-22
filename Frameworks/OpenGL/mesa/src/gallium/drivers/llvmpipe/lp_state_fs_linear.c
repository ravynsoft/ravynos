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
#include "util/u_pack_color.h"
#include "util/u_surface.h"
#include "util/u_sse.h"

#include "lp_jit.h"
#include "lp_rast.h"
#include "lp_debug.h"
#include "lp_state_fs.h"
#include "lp_linear_priv.h"


#if DETECT_ARCH_SSE

#include <emmintrin.h>


struct nearest_sampler {
   alignas(16) uint32_t out[64];

   const struct lp_jit_texture *texture;
   float fsrc_x;                /* src_x0 */
   float fsrc_y;                /* src_y0 */
   float fdsdx;              /* sx */
   float fdsdy;              /* sx */
   float fdtdx;              /* sy */
   float fdtdy;              /* sy */
   int width;
   int y;

   const uint32_t *(*fetch)(struct nearest_sampler *samp);
};


struct linear_interp {
   alignas(16) uint32_t out[64];
   __m128i a0;
   __m128i dadx;
   __m128i dady;
   int width;                   /* rounded up to multiple of 4 */
   bool is_constant;
};

/* Organize all the information needed for blending in one place.
 * Could have blend function pointer here, but we currently always
 * know which one we want to call.
 */
struct color_blend {
   const uint32_t *src;
   uint8_t *color;
   int stride;
   int width;                   /* the exact width */
};


/* Organize all the information needed for running each of the shaders
 * in one place.
 */
struct shader {
   alignas(16) uint32_t out0[64];
   const uint32_t *src0;
   const uint32_t *src1;
   __m128i const0;
   int width;                   /* rounded up to multiple of 4 */
};


/* For a row of pixels, perform add/one/inv_src_alpha (ie
 * premultiplied alpha) blending between the incoming pixels and the
 * destination buffer.
 *
 * Used to implement the BLIT_RGBA + blend shader, there are no
 * operations from the pixel shader left to implement at this level -
 * effectively the pixel shader was just a texture fetch which has
 * already been performed.  This routine then purely implements
 * blending.
 */
static void
blend_premul(struct color_blend *blend)
{
   const uint32_t *src = blend->src;  /* aligned */
   uint32_t *dst = (uint32_t *)blend->color;      /* unaligned */
   const int width = blend->width;
   int i;
   union { __m128i m128; uint ui[4]; } dstreg;

   blend->color += blend->stride;

   for (i = 0; i + 3 < width; i += 4) {
      __m128i tmp;
      tmp = _mm_loadu_si128((const __m128i *)&dst[i]);  /* UNALIGNED READ */
      dstreg.m128 = util_sse2_blend_premul_4(*(const __m128i *)&src[i],
                                             tmp);
      _mm_storeu_si128((__m128i *)&dst[i], dstreg.m128); /* UNALIGNED WRITE */
   }

   if (i < width) {
      int j;
      for (j = 0; j < width - i ; j++) {
         dstreg.ui[j] = dst[i+j];
      }
      dstreg.m128 = util_sse2_blend_premul_4(*(const __m128i *)&src[i],
                                             dstreg.m128);
      for (; i < width; i++)
         dst[i] = dstreg.ui[i&3];
   }
}


static void
blend_noop(struct color_blend *blend)
{
   memcpy(blend->color, blend->src, blend->width * sizeof(unsigned));
   blend->color += blend->stride;
}


static void
init_blend(struct color_blend *blend,
           int x, int y, int width, int height,
           uint8_t *color,
           int stride)
{
   blend->color = color + x * 4 + y * stride;
   blend->stride = stride;
   blend->width = width;
}


/*
 * Perform nearest filtered lookup of a row of texels.  Texture lookup
 * is assumed to be axis aligned but with arbitrary scaling.
 *
 * Texture coordinate interpolation is performed in 24.8 fixed point.
 * Note that the longest span we will encounter is 64 pixels long,
 * meaning that 8 fractional bits is more than sufficient to represent
 * the shallowest gradient possible within this span.
 *
 * After 64 pixels (ie. in the next tile), the starting point will be
 * recalculated with floating point arithmetic.
 *
 * XXX: migrate this to use Jose's quad blitter texture fetch routines.
 */
static const uint32_t *
fetch_row(struct nearest_sampler *samp)
{
   const int y = samp->y++;
   uint32_t *row = samp->out;
   const struct lp_jit_texture *texture = samp->texture;
   const int yy = util_iround(samp->fsrc_y + samp->fdtdy * y);
   const uint32_t *src_row =
      (const uint32_t *)((const uint8_t *)texture->base +
                         yy * texture->row_stride[0]);
   const int iscale_x = samp->fdsdx * 256;
   const int width = samp->width;
   int acc = samp->fsrc_x * 256 + 128;

   for (int i = 0; i < width; i++) {
      row[i] = src_row[acc>>8];
      acc += iscale_x;
   }

   return row;
}


/* Version of fetch_row which can cope with texture edges.  In
 * practise, aero never triggers this.
 */
static const uint32_t *
fetch_row_clamped(struct nearest_sampler *samp)
{
   const int y = samp->y++;
   uint32_t *row = samp->out;
   const struct lp_jit_texture *texture = samp->texture;
   const int yy = util_iround(samp->fsrc_y + samp->fdtdy * y);
   const uint32_t *src_row =
      (const uint32_t *)((const uint8_t *)texture->base +
                         CLAMP(yy, 0, texture->height-1) *
                         texture->row_stride[0]);
   const float src_x0 = samp->fsrc_x;
   const float scale_x = samp->fdsdx;
   const int width = samp->width;

   for (int i = 0; i < width; i++) {
      row[i] = src_row[CLAMP(util_iround(src_x0 + i * scale_x),
                             0, texture->width - 1)];
   }

   return row;
}

/* It vary rarely happens that some non-axis-aligned texturing creeps
 * into the linear path.  Handle it here.  The alternative would be
 * more pre-checking or an option to fallback by returning false from
 * jit_linear.
 */
static const uint32_t *
fetch_row_xy_clamped(struct nearest_sampler *samp)
{
   const int y = samp->y++;
   uint32_t *row = samp->out;
   const struct lp_jit_texture *texture = samp->texture;
   const float yrow = samp->fsrc_y + samp->fdtdy * y;
   const float xrow = samp->fsrc_x + samp->fdsdy * y;
   const int width  = samp->width;

   for (int i = 0; i < width; i++) {
      int yy = util_iround(yrow + samp->fdtdx * i);
      int xx = util_iround(xrow + samp->fdsdx * i);

      const uint32_t *src_row =
         (const uint32_t *)((const uint8_t *) texture->base +
                            CLAMP(yy, 0, texture->height-1) *
                            texture->row_stride[0]);

      row[i] = src_row[CLAMP(xx, 0, texture->width - 1)];
   }

   return row;
}


static bool
init_nearest_sampler(struct nearest_sampler *samp,
                     const struct lp_jit_texture *texture,
                     int x0, int y0,
                     int width, int height,
                     float s0, float dsdx, float dsdy,
                     float t0, float dtdx, float dtdy,
                     float w0, float dwdx, float dwdy)
{
   const float oow = 1.0f / w0;

   if (dwdx != 0.0 || dwdy != 0.0)
      return false;

   samp->texture = texture;
   samp->width = width;
   samp->fdsdx = dsdx * texture->width * oow;
   samp->fdsdy = dsdy * texture->width * oow;
   samp->fdtdx = dtdx * texture->height * oow;
   samp->fdtdy = dtdy * texture->height * oow;
   samp->fsrc_x = (samp->fdsdx * x0 +
                   samp->fdsdy * y0 +
                   s0 * texture->width * oow - 0.5f);

   samp->fsrc_y = (samp->fdtdx * x0 +
                   samp->fdtdy * y0 +
                   t0 * texture->height * oow - 0.5f);
   samp->y = 0;

   /* Because we want to permit consumers of this data to round up to
    * the next multiple of 4, and because we don't want valgrind to
    * complain about uninitialized reads, set the last bit of the
    * buffer to zero:
    */
   for (int i = width; i & 3; i++)
      samp->out[i] = 0;

   if (dsdy != 0 || dtdx != 0) {
      /* Arbitrary texture lookup:
       */
      samp->fetch = fetch_row_xy_clamped;
   } else {
      /* Axis aligned stretch blit, abitrary scaling factors including
       * flipped, minifying and magnifying:
       */
      int isrc_x = util_iround(samp->fsrc_x);
      int isrc_y = util_iround(samp->fsrc_y);
      int isrc_x1 = util_iround(samp->fsrc_x + width * samp->fdsdx);
      int isrc_y1 = util_iround(samp->fsrc_y + height * samp->fdtdy);

      /* Look at the maximum and minimum texture coordinates we will be
       * fetching and figure out if we need to use clamping.  There is
       * similar code in u_blit_sw.c which takes a better approach to
       * this which could be substituted later.
       */
      if (isrc_x  <= texture->width  && isrc_x  >= 0 &&
          isrc_y  <= texture->height && isrc_y  >= 0 &&
          isrc_x1 <= texture->width  && isrc_x1 >= 0 &&
          isrc_y1 <= texture->height && isrc_y1 >= 0) {
         samp->fetch = fetch_row;
      } else {
         samp->fetch = fetch_row_clamped;
      }
   }

   return true;
}


static const uint32_t *
shade_rgb1(struct shader *shader)
{
   const __m128i rgb1 = _mm_set1_epi32(0xff000000);
   const uint32_t *src0 = shader->src0;
   uint32_t *dst = shader->out0;
   int width = shader->width;
   int i;

   for (i = 0; i + 3 < width; i += 4) {
      __m128i s = *(const __m128i *)&src0[i];
      *(__m128i *)&dst[i] = _mm_or_si128(s, rgb1);
   }

   return shader->out0;
}


static void
init_shader(struct shader *shader,
           int x, int y, int width, int height)
{
   shader->width = align(width, 4);
}


/* Linear shader which implements the BLIT_RGBA shader with the
 * additional constraints imposed by lp_setup_is_blit().
 */
static bool
blit_rgba_blit(const struct lp_rast_state *state,
               unsigned x, unsigned y,
               unsigned width, unsigned height,
               const float (*a0)[4],
               const float (*dadx)[4],
               const float (*dady)[4],
               uint8_t *color,
               unsigned stride)
{
   const struct lp_jit_resources *resources = &state->jit_resources;
   const struct lp_jit_texture *texture = &resources->textures[0];
   const uint8_t *src;
   unsigned src_stride;
   int src_x, src_y;

   LP_DBG(DEBUG_RAST, "%s\n", __func__);

   /* Require w==1.0:
    */
   if (a0[0][3] != 1.0 ||
       dadx[0][3] != 0.0 ||
       dady[0][3] != 0.0)
      return false;

   src_x = x + util_iround(a0[1][0]*texture->width - 0.5f);
   src_y = y + util_iround(a0[1][1]*texture->height - 0.5f);

   src = texture->base;
   src_stride = texture->row_stride[0];

   /* Fall back to blit_rgba() if clamping required:
    */
   if (src_x < 0 ||
       src_y < 0 ||
       src_x + width > texture->width ||
       src_y + height > texture->height)
      return false;

   util_copy_rect(color, PIPE_FORMAT_B8G8R8A8_UNORM, stride,
                  x, y,
                  width, height,
                  src, src_stride,
                  src_x, src_y);

   return true;
}


/* Linear shader which implements the BLIT_RGB1 shader, with the
 * additional constraints imposed by lp_setup_is_blit().
 */
static bool
blit_rgb1_blit(const struct lp_rast_state *state,
               unsigned x, unsigned y,
               unsigned width, unsigned height,
               const float (*a0)[4],
               const float (*dadx)[4],
               const float (*dady)[4],
               uint8_t *color,
               unsigned stride)
{
   const struct lp_jit_resources *resources = &state->jit_resources;
   const struct lp_jit_texture *texture = &resources->textures[0];
   const uint8_t *src;
   unsigned src_stride;
   int src_x, src_y;

   LP_DBG(DEBUG_RAST, "%s\n", __func__);

   /* Require w==1.0:
    */
   if (a0[0][3] != 1.0 ||
       dadx[0][3] != 0.0 ||
       dady[0][3] != 0.0)
      return false;

   color += x * 4 + y * stride;

   src_x = x + util_iround(a0[1][0]*texture->width - 0.5f);
   src_y = y + util_iround(a0[1][1]*texture->height - 0.5f);

   src = texture->base;
   src_stride = texture->row_stride[0];
   src += src_x * 4;
   src += src_y * src_stride;

   if (src_x < 0 ||
       src_y < 0 ||
       src_x + width > texture->width ||
       src_y + height > texture->height)
      return false;

   for (y = 0; y < height; y++) {
      const uint32_t *src_row = (const uint32_t *)src;
      uint32_t *dst_row = (uint32_t *)color;

      for (x = 0; x < width; x++) {
         *dst_row++ = *src_row++ | 0xff000000;
      }

      color += stride;
      src += src_stride;
   }

   return true;
}


/* Linear shader variant implementing the BLIT_RGBA shader without
 * blending.
 */
static bool
blit_rgba(const struct lp_rast_state *state,
          unsigned x, unsigned y,
          unsigned width, unsigned height,
          const float (*a0)[4],
          const float (*dadx)[4],
          const float (*dady)[4],
          uint8_t *color,
          unsigned stride)
{
   const struct lp_jit_resources *resources = &state->jit_resources;
   struct nearest_sampler samp;
   struct color_blend blend;

   LP_DBG(DEBUG_RAST, "%s\n", __func__);

   if (!init_nearest_sampler(&samp,
                             &resources->textures[0],
                             x, y, width, height,
                             a0[1][0], dadx[1][0], dady[1][0],
                             a0[1][1], dadx[1][1], dady[1][1],
                             a0[0][3], dadx[0][3], dady[0][3]))
      return false;

   init_blend(&blend,
              x, y, width, height,
              color, stride);

   /* Rasterize the rectangle and run the shader:
    */
   for (y = 0; y < height; y++) {
      blend.src = samp.fetch(&samp);
      blend_noop(&blend);
   }

   return true;
}


static bool
blit_rgb1(const struct lp_rast_state *state,
          unsigned x, unsigned y,
          unsigned width, unsigned height,
          const float (*a0)[4],
          const float (*dadx)[4],
          const float (*dady)[4],
          uint8_t *color,
          unsigned stride)
{
   const struct lp_jit_resources *resources = &state->jit_resources;
   struct nearest_sampler samp;
   struct color_blend blend;
   struct shader shader;

   LP_DBG(DEBUG_RAST, "%s\n", __func__);

   if (!init_nearest_sampler(&samp,
                             &resources->textures[0],
                             x, y, width, height,
                             a0[1][0], dadx[1][0], dady[1][0],
                             a0[1][1], dadx[1][1], dady[1][1],
                             a0[0][3], dadx[0][3], dady[0][3]))
      return false;

   init_blend(&blend, x, y, width, height, color, stride);

   init_shader(&shader, x, y, width, height);

   /* Rasterize the rectangle and run the shader:
    */
   for (y = 0; y < height; y++) {
      shader.src0 = samp.fetch(&samp);
      blend.src = shade_rgb1(&shader);
      blend_noop(&blend);
   }

   return true;
}


/* Linear shader variant implementing the BLIT_RGBA shader with
 * one/inv_src_alpha blending.
 */
static bool
blit_rgba_blend_premul(const struct lp_rast_state *state,
                       unsigned x, unsigned y,
                       unsigned width, unsigned height,
                       const float (*a0)[4],
                       const float (*dadx)[4],
                       const float (*dady)[4],
                       uint8_t *color,
                       unsigned stride)
{
   const struct lp_jit_resources *resources = &state->jit_resources;
   struct nearest_sampler samp;
   struct color_blend blend;

   LP_DBG(DEBUG_RAST, "%s\n", __func__);

   if (!init_nearest_sampler(&samp,
                             &resources->textures[0],
                             x, y, width, height,
                             a0[1][0], dadx[1][0], dady[1][0],
                             a0[1][1], dadx[1][1], dady[1][1],
                             a0[0][3], dadx[0][3], dady[0][3]))
      return false;

   init_blend(&blend, x, y, width, height, color, stride);

   /* Rasterize the rectangle and run the shader:
    */
   for (y = 0; y < height; y++) {
      blend.src = samp.fetch(&samp);
      blend_premul(&blend);
   }

   return true;
}


/* Linear shader which always emits red.  Used for debugging.
 */
static bool
linear_red(const struct lp_rast_state *state,
           unsigned x, unsigned y,
           unsigned width, unsigned height,
           const float (*a0)[4],
           const float (*dadx)[4],
           const float (*dady)[4],
           uint8_t *color,
           unsigned stride)
{
   union util_color uc;

   util_pack_color_ub(0xff, 0, 0, 0xff,
                      PIPE_FORMAT_B8G8R8A8_UNORM, &uc);

   util_fill_rect(color,
                  PIPE_FORMAT_B8G8R8A8_UNORM,
                  stride,
                  x,
                  y,
                  width,
                  height,
                  &uc);

   return true;
}


/* Noop linear shader variant, for debugging.
 */
static bool
linear_no_op(const struct lp_rast_state *state,
             unsigned x, unsigned y,
             unsigned width, unsigned height,
             const float (*a0)[4],
             const float (*dadx)[4],
             const float (*dady)[4],
             uint8_t *color,
             unsigned stride)
{
   return true;
}


/* Check for ADD/ONE/INV_SRC_ALPHA, ie premultiplied-alpha blending.
 */
static bool
is_one_inv_src_alpha_blend(const struct lp_fragment_shader_variant *variant)
{
   return
      !variant->key.blend.logicop_enable &&
      variant->key.blend.rt[0].blend_enable &&
      variant->key.blend.rt[0].rgb_func == PIPE_BLEND_ADD &&
      variant->key.blend.rt[0].rgb_src_factor == PIPE_BLENDFACTOR_ONE &&
      variant->key.blend.rt[0].rgb_dst_factor == PIPE_BLENDFACTOR_INV_SRC_ALPHA &&
      variant->key.blend.rt[0].alpha_func == PIPE_BLEND_ADD &&
      variant->key.blend.rt[0].alpha_src_factor == PIPE_BLENDFACTOR_ONE &&
      variant->key.blend.rt[0].alpha_dst_factor == PIPE_BLENDFACTOR_INV_SRC_ALPHA &&
      variant->key.blend.rt[0].colormask == 0xf;
}


/* Examine the fragment shader variant and determine whether we can
 * substitute a fastpath linear shader implementation.
 */
void
llvmpipe_fs_variant_linear_fastpath(struct lp_fragment_shader_variant *variant)
{
   if (LP_PERF & PERF_NO_SHADE) {
      variant->jit_linear = linear_red;
      return;
   }

   struct lp_sampler_static_state *samp0 =
      lp_fs_variant_key_sampler_idx(&variant->key, 0);
   if (!samp0)
      return;

   enum pipe_format tex_format = samp0->texture_state.format;
   if (variant->shader->kind == LP_FS_KIND_BLIT_RGBA &&
       tex_format == PIPE_FORMAT_B8G8R8A8_UNORM &&
       is_nearest_clamp_sampler(samp0)) {
      if (variant->opaque) {
         variant->jit_linear_blit = blit_rgba_blit;
         variant->jit_linear = blit_rgba;
      } else if (is_one_inv_src_alpha_blend(variant) &&
                 util_get_cpu_caps()->has_sse2) {
         variant->jit_linear = blit_rgba_blend_premul;
      }
      return;
   }

   if (variant->shader->kind == LP_FS_KIND_BLIT_RGB1 &&
       variant->opaque &&
       (tex_format == PIPE_FORMAT_B8G8R8A8_UNORM ||
        tex_format == PIPE_FORMAT_B8G8R8X8_UNORM) &&
       is_nearest_clamp_sampler(samp0)) {
      variant->jit_linear_blit = blit_rgb1_blit;
      variant->jit_linear = blit_rgb1;
      return;
   }

   if (0) {
      variant->jit_linear = linear_no_op;
      return;
   }
}
#else
void
llvmpipe_fs_variant_linear_fastpath(struct lp_fragment_shader_variant *variant)
{
   /* don't bother if there is no SSE */
}
#endif

