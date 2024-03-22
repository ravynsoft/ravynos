/*
 * Copyright (C) 2012-2013 Rob Clark <robclark@freedesktop.org>
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
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "fd2_texture.h"
#include "fd2_util.h"

static enum sq_tex_clamp
tex_clamp(unsigned wrap)
{
   switch (wrap) {
   case PIPE_TEX_WRAP_REPEAT:
      return SQ_TEX_WRAP;
   case PIPE_TEX_WRAP_CLAMP:
      return SQ_TEX_CLAMP_HALF_BORDER;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      return SQ_TEX_CLAMP_LAST_TEXEL;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      return SQ_TEX_CLAMP_BORDER;
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
      return SQ_TEX_MIRROR;
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
      return SQ_TEX_MIRROR_ONCE_HALF_BORDER;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      return SQ_TEX_MIRROR_ONCE_LAST_TEXEL;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      return SQ_TEX_MIRROR_ONCE_BORDER;
   default:
      DBG("invalid wrap: %u", wrap);
      return 0;
   }
}

static enum sq_tex_filter
tex_filter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_FILTER_NEAREST:
      return SQ_TEX_FILTER_POINT;
   case PIPE_TEX_FILTER_LINEAR:
      return SQ_TEX_FILTER_BILINEAR;
   default:
      DBG("invalid filter: %u", filter);
      return 0;
   }
}

static enum sq_tex_filter
mip_filter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_MIPFILTER_NONE:
      return SQ_TEX_FILTER_BASEMAP;
   case PIPE_TEX_MIPFILTER_NEAREST:
      return SQ_TEX_FILTER_POINT;
   case PIPE_TEX_MIPFILTER_LINEAR:
      return SQ_TEX_FILTER_BILINEAR;
   default:
      DBG("invalid filter: %u", filter);
      return 0;
   }
}

static void *
fd2_sampler_state_create(struct pipe_context *pctx,
                         const struct pipe_sampler_state *cso)
{
   struct fd2_sampler_stateobj *so = CALLOC_STRUCT(fd2_sampler_stateobj);

   if (!so)
      return NULL;

   so->base = *cso;

   /* TODO
    * cso->max_anisotropy
    * cso->unnormalized_coords (dealt with by shader for rect textures?)
    */

   /* SQ_TEX0_PITCH() must be OR'd in later when we know the bound texture: */
   so->tex0 = A2XX_SQ_TEX_0_CLAMP_X(tex_clamp(cso->wrap_s)) |
              A2XX_SQ_TEX_0_CLAMP_Y(tex_clamp(cso->wrap_t)) |
              A2XX_SQ_TEX_0_CLAMP_Z(tex_clamp(cso->wrap_r));

   so->tex3 = A2XX_SQ_TEX_3_XY_MAG_FILTER(tex_filter(cso->mag_img_filter)) |
              A2XX_SQ_TEX_3_XY_MIN_FILTER(tex_filter(cso->min_img_filter)) |
              A2XX_SQ_TEX_3_MIP_FILTER(mip_filter(cso->min_mip_filter));

   so->tex4 = 0;
   if (cso->min_mip_filter != PIPE_TEX_MIPFILTER_NONE)
      so->tex4 = A2XX_SQ_TEX_4_LOD_BIAS(cso->lod_bias);

   return so;
}

static void
fd2_sampler_states_bind(struct pipe_context *pctx, enum pipe_shader_type shader,
                        unsigned start, unsigned nr, void **hwcso) in_dt
{
   if (!hwcso)
      nr = 0;

   if (shader == PIPE_SHADER_FRAGMENT) {
      struct fd_context *ctx = fd_context(pctx);

      /* on a2xx, since there is a flat address space for textures/samplers,
       * a change in # of fragment textures/samplers will trigger patching and
       * re-emitting the vertex shader:
       */
      if (nr != ctx->tex[PIPE_SHADER_FRAGMENT].num_samplers)
         ctx->dirty |= FD_DIRTY_TEXSTATE;
   }

   fd_sampler_states_bind(pctx, shader, start, nr, hwcso);
}

static enum sq_tex_dimension
tex_dimension(unsigned target)
{
   switch (target) {
   default:
      unreachable("Unsupported target");
   case PIPE_TEXTURE_1D:
      assert(0); /* TODO */
      return SQ_TEX_DIMENSION_1D;
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D:
      return SQ_TEX_DIMENSION_2D;
   case PIPE_TEXTURE_3D:
      assert(0); /* TODO */
      return SQ_TEX_DIMENSION_3D;
   case PIPE_TEXTURE_CUBE:
      return SQ_TEX_DIMENSION_CUBE;
   }
}

static struct pipe_sampler_view *
fd2_sampler_view_create(struct pipe_context *pctx, struct pipe_resource *prsc,
                        const struct pipe_sampler_view *cso)
{
   struct fd2_pipe_sampler_view *so = CALLOC_STRUCT(fd2_pipe_sampler_view);
   struct fd_resource *rsc = fd_resource(prsc);
   struct surface_format fmt = fd2_pipe2surface(cso->format);

   if (!so)
      return NULL;

   so->base = *cso;
   pipe_reference(NULL, &prsc->reference);
   so->base.texture = prsc;
   so->base.reference.count = 1;
   so->base.context = pctx;

   so->tex0 = A2XX_SQ_TEX_0_SIGN_X(fmt.sign) | A2XX_SQ_TEX_0_SIGN_Y(fmt.sign) |
              A2XX_SQ_TEX_0_SIGN_Z(fmt.sign) | A2XX_SQ_TEX_0_SIGN_W(fmt.sign) |
              A2XX_SQ_TEX_0_PITCH(fdl2_pitch_pixels(&rsc->layout, 0) *
                                  util_format_get_blockwidth(prsc->format)) |
              COND(rsc->layout.tile_mode, A2XX_SQ_TEX_0_TILED);
   so->tex1 = A2XX_SQ_TEX_1_FORMAT(fmt.format) |
              A2XX_SQ_TEX_1_CLAMP_POLICY(SQ_TEX_CLAMP_POLICY_OGL);
   so->tex2 = A2XX_SQ_TEX_2_HEIGHT(prsc->height0 - 1) |
              A2XX_SQ_TEX_2_WIDTH(prsc->width0 - 1);
   so->tex3 = A2XX_SQ_TEX_3_NUM_FORMAT(fmt.num_format) |
              fd2_tex_swiz(cso->format, cso->swizzle_r, cso->swizzle_g,
                           cso->swizzle_b, cso->swizzle_a) |
              A2XX_SQ_TEX_3_EXP_ADJUST(fmt.exp_adjust);

   so->tex4 = A2XX_SQ_TEX_4_MIP_MIN_LEVEL(fd_sampler_first_level(cso)) |
              A2XX_SQ_TEX_4_MIP_MAX_LEVEL(fd_sampler_last_level(cso));

   so->tex5 = A2XX_SQ_TEX_5_DIMENSION(tex_dimension(prsc->target));

   return &so->base;
}

static void
fd2_set_sampler_views(struct pipe_context *pctx, enum pipe_shader_type shader,
                      unsigned start, unsigned nr,
                      unsigned unbind_num_trailing_slots,
                      bool take_ownership,
                      struct pipe_sampler_view **views) in_dt
{
   if (shader == PIPE_SHADER_FRAGMENT) {
      struct fd_context *ctx = fd_context(pctx);

      /* on a2xx, since there is a flat address space for textures/samplers,
       * a change in # of fragment textures/samplers will trigger patching and
       * re-emitting the vertex shader:
       */
      if (nr != ctx->tex[PIPE_SHADER_FRAGMENT].num_textures)
         ctx->dirty |= FD_DIRTY_TEXSTATE;
   }

   fd_set_sampler_views(pctx, shader, start, nr, unbind_num_trailing_slots,
                        take_ownership, views);
}

/* map gallium sampler-id to hw const-idx.. adreno uses a flat address
 * space of samplers (const-idx), so we need to map the gallium sampler-id
 * which is per-shader to a global const-idx space.
 *
 * Fragment shader sampler maps directly to const-idx, and vertex shader
 * is offset by the # of fragment shader samplers.  If the # of fragment
 * shader samplers changes, this shifts the vertex shader indexes.
 *
 * TODO maybe we can do frag shader 0..N  and vert shader N..0 to avoid
 * this??
 */
unsigned
fd2_get_const_idx(struct fd_context *ctx, struct fd_texture_stateobj *tex,
                  unsigned samp_id) assert_dt
{
   if (tex == &ctx->tex[PIPE_SHADER_FRAGMENT])
      return samp_id;
   return samp_id + ctx->tex[PIPE_SHADER_FRAGMENT].num_samplers;
}

void
fd2_texture_init(struct pipe_context *pctx)
{
   pctx->create_sampler_state = fd2_sampler_state_create;
   pctx->bind_sampler_states = fd2_sampler_states_bind;
   pctx->create_sampler_view = fd2_sampler_view_create;
   pctx->set_sampler_views = fd2_set_sampler_views;
}
