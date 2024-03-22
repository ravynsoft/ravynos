/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "fd5_format.h"
#include "fd5_texture.h"

static enum a5xx_tex_clamp
tex_clamp(unsigned wrap, bool *needs_border)
{
   switch (wrap) {
   case PIPE_TEX_WRAP_REPEAT:
      return A5XX_TEX_REPEAT;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      return A5XX_TEX_CLAMP_TO_EDGE;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      *needs_border = true;
      return A5XX_TEX_CLAMP_TO_BORDER;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      /* only works for PoT.. need to emulate otherwise! */
      return A5XX_TEX_MIRROR_CLAMP;
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
      return A5XX_TEX_MIRROR_REPEAT;
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      /* these two we could perhaps emulate, but we currently
       * just don't advertise PIPE_CAP_TEXTURE_MIRROR_CLAMP
       */
   default:
      DBG("invalid wrap: %u", wrap);
      return 0;
   }
}

static enum a5xx_tex_filter
tex_filter(unsigned filter, bool aniso)
{
   switch (filter) {
   case PIPE_TEX_FILTER_NEAREST:
      return A5XX_TEX_NEAREST;
   case PIPE_TEX_FILTER_LINEAR:
      return aniso ? A5XX_TEX_ANISO : A5XX_TEX_LINEAR;
   default:
      DBG("invalid filter: %u", filter);
      return 0;
   }
}

static void *
fd5_sampler_state_create(struct pipe_context *pctx,
                         const struct pipe_sampler_state *cso)
{
   struct fd5_sampler_stateobj *so = CALLOC_STRUCT(fd5_sampler_stateobj);
   unsigned aniso = util_last_bit(MIN2(cso->max_anisotropy >> 1, 8));
   bool miplinear = false;

   if (!so)
      return NULL;

   so->base = *cso;

   if (cso->min_mip_filter == PIPE_TEX_MIPFILTER_LINEAR)
      miplinear = true;

   so->needs_border = false;
   so->texsamp0 =
      COND(miplinear, A5XX_TEX_SAMP_0_MIPFILTER_LINEAR_NEAR) |
      A5XX_TEX_SAMP_0_XY_MAG(tex_filter(cso->mag_img_filter, aniso)) |
      A5XX_TEX_SAMP_0_XY_MIN(tex_filter(cso->min_img_filter, aniso)) |
      A5XX_TEX_SAMP_0_ANISO(aniso) |
      A5XX_TEX_SAMP_0_WRAP_S(tex_clamp(cso->wrap_s, &so->needs_border)) |
      A5XX_TEX_SAMP_0_WRAP_T(tex_clamp(cso->wrap_t, &so->needs_border)) |
      A5XX_TEX_SAMP_0_WRAP_R(tex_clamp(cso->wrap_r, &so->needs_border));

   so->texsamp1 =
      COND(!cso->seamless_cube_map, A5XX_TEX_SAMP_1_CUBEMAPSEAMLESSFILTOFF) |
      COND(cso->unnormalized_coords, A5XX_TEX_SAMP_1_UNNORM_COORDS);

   so->texsamp0 |= A5XX_TEX_SAMP_0_LOD_BIAS(cso->lod_bias);

   if (cso->min_mip_filter != PIPE_TEX_MIPFILTER_NONE) {
      so->texsamp1 |= A5XX_TEX_SAMP_1_MIN_LOD(cso->min_lod) |
                      A5XX_TEX_SAMP_1_MAX_LOD(cso->max_lod);
   } else {
      /* If we're not doing mipmap filtering, we still need a slightly > 0
       * LOD clamp so the HW can decide between min and mag filtering of
       * level 0.
       */
      so->texsamp1 |= A5XX_TEX_SAMP_1_MIN_LOD(MIN2(cso->min_lod, 0.125f)) |
                      A5XX_TEX_SAMP_1_MAX_LOD(MIN2(cso->max_lod, 0.125f));
   }

   if (cso->compare_mode)
      so->texsamp1 |=
         A5XX_TEX_SAMP_1_COMPARE_FUNC(cso->compare_func); /* maps 1:1 */

   return so;
}

static struct pipe_sampler_view *
fd5_sampler_view_create(struct pipe_context *pctx, struct pipe_resource *prsc,
                        const struct pipe_sampler_view *cso)
{
   struct fd5_pipe_sampler_view *so = CALLOC_STRUCT(fd5_pipe_sampler_view);
   struct fd_resource *rsc = fd_resource(prsc);
   enum pipe_format format = cso->format;
   unsigned lvl, layers = 0;

   if (!so)
      return NULL;

   if (format == PIPE_FORMAT_X32_S8X24_UINT) {
      rsc = rsc->stencil;
      format = rsc->b.b.format;
   }

   so->base = *cso;
   pipe_reference(NULL, &prsc->reference);
   so->base.texture = prsc;
   so->base.reference.count = 1;
   so->base.context = pctx;

   so->texconst0 = A5XX_TEX_CONST_0_FMT(fd5_pipe2tex(format)) |
                   A5XX_TEX_CONST_0_SAMPLES(fd_msaa_samples(prsc->nr_samples)) |
                   fd5_tex_swiz(format, cso->swizzle_r, cso->swizzle_g,
                                cso->swizzle_b, cso->swizzle_a);

   /* NOTE: since we sample z24s8 using 8888_UINT format, the swizzle
    * we get isn't quite right.  Use SWAP(XYZW) as a cheap and cheerful
    * way to re-arrange things so stencil component is where the swiz
    * expects.
    *
    * Note that gallium expects stencil sampler to return (s,s,s,s)
    * which isn't quite true.  To make that happen we'd have to massage
    * the swizzle.  But in practice only the .x component is used.
    */
   if (format == PIPE_FORMAT_X24S8_UINT) {
      so->texconst0 |= A5XX_TEX_CONST_0_SWAP(XYZW);
   }

   if (util_format_is_srgb(format)) {
      so->texconst0 |= A5XX_TEX_CONST_0_SRGB;
   }

   if (cso->target == PIPE_BUFFER) {
      unsigned elements = cso->u.buf.size / util_format_get_blocksize(format);

      lvl = 0;
      so->texconst1 = A5XX_TEX_CONST_1_WIDTH(elements & MASK(15)) |
                      A5XX_TEX_CONST_1_HEIGHT(elements >> 15);
      so->texconst2 = A5XX_TEX_CONST_2_BUFFER;
      so->offset = cso->u.buf.offset;
   } else {
      unsigned miplevels;

      lvl = fd_sampler_first_level(cso);
      miplevels = fd_sampler_last_level(cso) - lvl;
      layers = cso->u.tex.last_layer - cso->u.tex.first_layer + 1;

      so->texconst0 |= A5XX_TEX_CONST_0_MIPLVLS(miplevels);
      so->texconst1 = A5XX_TEX_CONST_1_WIDTH(u_minify(prsc->width0, lvl)) |
                      A5XX_TEX_CONST_1_HEIGHT(u_minify(prsc->height0, lvl));
      so->texconst2 = A5XX_TEX_CONST_2_PITCHALIGN(rsc->layout.pitchalign - 6) |
                      A5XX_TEX_CONST_2_PITCH(fd_resource_pitch(rsc, lvl));
      so->offset = fd_resource_offset(rsc, lvl, cso->u.tex.first_layer);
   }

   so->texconst2 |= A5XX_TEX_CONST_2_TYPE(fd5_tex_type(cso->target));

   switch (cso->target) {
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_2D:
      so->texconst3 = A5XX_TEX_CONST_3_ARRAY_PITCH(rsc->layout.layer_size);
      so->texconst5 = A5XX_TEX_CONST_5_DEPTH(1);
      break;
   case PIPE_TEXTURE_1D_ARRAY:
   case PIPE_TEXTURE_2D_ARRAY:
      so->texconst3 = A5XX_TEX_CONST_3_ARRAY_PITCH(rsc->layout.layer_size);
      so->texconst5 = A5XX_TEX_CONST_5_DEPTH(layers);
      break;
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      so->texconst3 = A5XX_TEX_CONST_3_ARRAY_PITCH(rsc->layout.layer_size);
      so->texconst5 = A5XX_TEX_CONST_5_DEPTH(layers / 6);
      break;
   case PIPE_TEXTURE_3D:
      so->texconst3 =
         A5XX_TEX_CONST_3_MIN_LAYERSZ(
            fd_resource_slice(rsc, prsc->last_level)->size0) |
         A5XX_TEX_CONST_3_ARRAY_PITCH(fd_resource_slice(rsc, lvl)->size0);
      so->texconst5 = A5XX_TEX_CONST_5_DEPTH(u_minify(prsc->depth0, lvl));
      break;
   default:
      so->texconst3 = 0x00000000;
      break;
   }

   return &so->base;
}

void
fd5_texture_init(struct pipe_context *pctx)
{
   pctx->create_sampler_state = fd5_sampler_state_create;
   pctx->bind_sampler_states = fd_sampler_states_bind;
   pctx->create_sampler_view = fd5_sampler_view_create;
   pctx->set_sampler_views = fd_set_sampler_views;
}
