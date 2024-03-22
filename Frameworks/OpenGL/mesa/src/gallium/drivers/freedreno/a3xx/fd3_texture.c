/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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

#include "fd3_format.h"
#include "fd3_texture.h"

static enum a3xx_tex_clamp
tex_clamp(unsigned wrap, bool *needs_border)
{
   switch (wrap) {
   case PIPE_TEX_WRAP_REPEAT:
      return A3XX_TEX_REPEAT;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      return A3XX_TEX_CLAMP_TO_EDGE;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      *needs_border = true;
      return A3XX_TEX_CLAMP_TO_BORDER;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      /* only works for PoT.. need to emulate otherwise! */
      return A3XX_TEX_MIRROR_CLAMP;
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
      return A3XX_TEX_MIRROR_REPEAT;
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

static enum a3xx_tex_filter
tex_filter(unsigned filter, bool aniso)
{
   switch (filter) {
   case PIPE_TEX_FILTER_NEAREST:
      return A3XX_TEX_NEAREST;
   case PIPE_TEX_FILTER_LINEAR:
      return aniso ? A3XX_TEX_ANISO : A3XX_TEX_LINEAR;
   default:
      DBG("invalid filter: %u", filter);
      return 0;
   }
}

static void *
fd3_sampler_state_create(struct pipe_context *pctx,
                         const struct pipe_sampler_state *cso)
{
   struct fd3_sampler_stateobj *so = CALLOC_STRUCT(fd3_sampler_stateobj);
   unsigned aniso = util_last_bit(MIN2(cso->max_anisotropy >> 1, 8));
   bool miplinear = false;

   if (!so)
      return NULL;

   if (cso->min_mip_filter == PIPE_TEX_MIPFILTER_LINEAR)
      miplinear = true;

   so->base = *cso;

   so->needs_border = false;
   so->texsamp0 =
      COND(cso->unnormalized_coords, A3XX_TEX_SAMP_0_UNNORM_COORDS) |
      COND(!cso->seamless_cube_map, A3XX_TEX_SAMP_0_CUBEMAPSEAMLESSFILTOFF) |
      COND(miplinear, A3XX_TEX_SAMP_0_MIPFILTER_LINEAR) |
      A3XX_TEX_SAMP_0_XY_MAG(tex_filter(cso->mag_img_filter, aniso)) |
      A3XX_TEX_SAMP_0_XY_MIN(tex_filter(cso->min_img_filter, aniso)) |
      A3XX_TEX_SAMP_0_ANISO(aniso) |
      A3XX_TEX_SAMP_0_WRAP_S(tex_clamp(cso->wrap_s, &so->needs_border)) |
      A3XX_TEX_SAMP_0_WRAP_T(tex_clamp(cso->wrap_t, &so->needs_border)) |
      A3XX_TEX_SAMP_0_WRAP_R(tex_clamp(cso->wrap_r, &so->needs_border));

   if (cso->compare_mode)
      so->texsamp0 |=
         A3XX_TEX_SAMP_0_COMPARE_FUNC(cso->compare_func); /* maps 1:1 */

   so->texsamp1 = A3XX_TEX_SAMP_1_LOD_BIAS(cso->lod_bias);

   if (cso->min_mip_filter != PIPE_TEX_MIPFILTER_NONE) {
      so->texsamp1 |= A3XX_TEX_SAMP_1_MIN_LOD(cso->min_lod) |
                      A3XX_TEX_SAMP_1_MAX_LOD(cso->max_lod);
   } else {
      /* If we're not doing mipmap filtering, we still need a slightly > 0
       * LOD clamp so the HW can decide between min and mag filtering of
       * level 0.
       */
      so->texsamp1 |= A3XX_TEX_SAMP_1_MIN_LOD(MIN2(cso->min_lod, 0.125f)) |
                      A3XX_TEX_SAMP_1_MAX_LOD(MIN2(cso->max_lod, 0.125f));
   }

   return so;
}

static enum a3xx_tex_type
tex_type(unsigned target)
{
   switch (target) {
   default:
      unreachable("Unsupported target");
   case PIPE_BUFFER:
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_1D_ARRAY:
      return A3XX_TEX_1D;
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_2D_ARRAY:
      return A3XX_TEX_2D;
   case PIPE_TEXTURE_3D:
      return A3XX_TEX_3D;
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return A3XX_TEX_CUBE;
   }
}

static struct pipe_sampler_view *
fd3_sampler_view_create(struct pipe_context *pctx, struct pipe_resource *prsc,
                        const struct pipe_sampler_view *cso)
{
   struct fd3_pipe_sampler_view *so = CALLOC_STRUCT(fd3_pipe_sampler_view);
   struct fd_resource *rsc = fd_resource(prsc);
   unsigned lvl;

   if (!so)
      return NULL;

   so->base = *cso;
   pipe_reference(NULL, &prsc->reference);
   so->base.texture = prsc;
   so->base.reference.count = 1;
   so->base.context = pctx;

   so->texconst0 = A3XX_TEX_CONST_0_TILE_MODE(rsc->layout.tile_mode) |
                   A3XX_TEX_CONST_0_TYPE(tex_type(prsc->target)) |
                   A3XX_TEX_CONST_0_FMT(fd3_pipe2tex(cso->format)) |
                   fd3_tex_swiz(cso->format, cso->swizzle_r, cso->swizzle_g,
                                cso->swizzle_b, cso->swizzle_a);

   if (prsc->target == PIPE_BUFFER || util_format_is_pure_integer(cso->format))
      so->texconst0 |= A3XX_TEX_CONST_0_NOCONVERT;
   if (util_format_is_srgb(cso->format))
      so->texconst0 |= A3XX_TEX_CONST_0_SRGB;

   if (prsc->target == PIPE_BUFFER) {
      lvl = 0;
      so->texconst1 =
         A3XX_TEX_CONST_1_WIDTH(cso->u.buf.size /
                                util_format_get_blocksize(cso->format)) |
         A3XX_TEX_CONST_1_HEIGHT(1);
   } else {
      unsigned miplevels;

      lvl = fd_sampler_first_level(cso);
      miplevels = fd_sampler_last_level(cso) - lvl;

      so->texconst0 |= A3XX_TEX_CONST_0_MIPLVLS(miplevels);
      so->texconst1 = A3XX_TEX_CONST_1_PITCHALIGN(rsc->layout.pitchalign - 4) |
                      A3XX_TEX_CONST_1_WIDTH(u_minify(prsc->width0, lvl)) |
                      A3XX_TEX_CONST_1_HEIGHT(u_minify(prsc->height0, lvl));
   }
   /* when emitted, A3XX_TEX_CONST_2_INDX() must be OR'd in: */
   struct fdl_slice *slice = fd_resource_slice(rsc, lvl);
   so->texconst2 = A3XX_TEX_CONST_2_PITCH(fd_resource_pitch(rsc, lvl));
   switch (prsc->target) {
   case PIPE_TEXTURE_1D_ARRAY:
   case PIPE_TEXTURE_2D_ARRAY:
      so->texconst3 = A3XX_TEX_CONST_3_DEPTH(prsc->array_size - 1) |
                      A3XX_TEX_CONST_3_LAYERSZ1(slice->size0);
      break;
   case PIPE_TEXTURE_3D:
      so->texconst3 = A3XX_TEX_CONST_3_DEPTH(u_minify(prsc->depth0, lvl)) |
                      A3XX_TEX_CONST_3_LAYERSZ1(slice->size0);
      so->texconst3 |= A3XX_TEX_CONST_3_LAYERSZ2(
         fd_resource_slice(rsc, prsc->last_level)->size0);
      break;
   default:
      so->texconst3 = 0x00000000;
      break;
   }

   return &so->base;
}

void
fd3_texture_init(struct pipe_context *pctx)
{
   pctx->create_sampler_state = fd3_sampler_state_create;
   pctx->bind_sampler_states = fd_sampler_states_bind;
   pctx->create_sampler_view = fd3_sampler_view_create;
   pctx->set_sampler_views = fd_set_sampler_views;
}
