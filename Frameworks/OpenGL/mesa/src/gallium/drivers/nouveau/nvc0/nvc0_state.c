/*
 * Copyright 2010 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "pipe/p_defines.h"
#include "util/u_framebuffer.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_transfer.h"

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_serialize.h"
#include "nir/tgsi_to_nir.h"

#include "nvc0/nvc0_stateobj.h"
#include "nvc0/nvc0_context.h"
#include "nvc0/nvc0_query_hw.h"

#include "nvc0/nvc0_3d.xml.h"

#include "nouveau_gldefs.h"

static inline uint32_t
nvc0_colormask(unsigned mask)
{
    uint32_t ret = 0;

    if (mask & PIPE_MASK_R)
        ret |= 0x0001;
    if (mask & PIPE_MASK_G)
        ret |= 0x0010;
    if (mask & PIPE_MASK_B)
        ret |= 0x0100;
    if (mask & PIPE_MASK_A)
        ret |= 0x1000;

    return ret;
}

#define NVC0_BLEND_FACTOR_CASE(a, b) \
   case PIPE_BLENDFACTOR_##a: return NV50_BLEND_FACTOR_##b

static inline uint32_t
nvc0_blend_fac(unsigned factor)
{
   switch (factor) {
   NVC0_BLEND_FACTOR_CASE(ONE, ONE);
   NVC0_BLEND_FACTOR_CASE(SRC_COLOR, SRC_COLOR);
   NVC0_BLEND_FACTOR_CASE(SRC_ALPHA, SRC_ALPHA);
   NVC0_BLEND_FACTOR_CASE(DST_ALPHA, DST_ALPHA);
   NVC0_BLEND_FACTOR_CASE(DST_COLOR, DST_COLOR);
   NVC0_BLEND_FACTOR_CASE(SRC_ALPHA_SATURATE, SRC_ALPHA_SATURATE);
   NVC0_BLEND_FACTOR_CASE(CONST_COLOR, CONSTANT_COLOR);
   NVC0_BLEND_FACTOR_CASE(CONST_ALPHA, CONSTANT_ALPHA);
   NVC0_BLEND_FACTOR_CASE(SRC1_COLOR, SRC1_COLOR);
   NVC0_BLEND_FACTOR_CASE(SRC1_ALPHA, SRC1_ALPHA);
   NVC0_BLEND_FACTOR_CASE(ZERO, ZERO);
   NVC0_BLEND_FACTOR_CASE(INV_SRC_COLOR, ONE_MINUS_SRC_COLOR);
   NVC0_BLEND_FACTOR_CASE(INV_SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
   NVC0_BLEND_FACTOR_CASE(INV_DST_ALPHA, ONE_MINUS_DST_ALPHA);
   NVC0_BLEND_FACTOR_CASE(INV_DST_COLOR, ONE_MINUS_DST_COLOR);
   NVC0_BLEND_FACTOR_CASE(INV_CONST_COLOR, ONE_MINUS_CONSTANT_COLOR);
   NVC0_BLEND_FACTOR_CASE(INV_CONST_ALPHA, ONE_MINUS_CONSTANT_ALPHA);
   NVC0_BLEND_FACTOR_CASE(INV_SRC1_COLOR, ONE_MINUS_SRC1_COLOR);
   NVC0_BLEND_FACTOR_CASE(INV_SRC1_ALPHA, ONE_MINUS_SRC1_ALPHA);
   default:
      return NV50_BLEND_FACTOR_ZERO;
   }
}

static void *
nvc0_blend_state_create(struct pipe_context *pipe,
                        const struct pipe_blend_state *cso)
{
   struct nvc0_blend_stateobj *so = CALLOC_STRUCT(nvc0_blend_stateobj);
   int i;
   int r; /* reference */
   uint32_t ms;
   uint8_t blend_en = 0;
   bool indep_masks = false;
   bool indep_funcs = false;

   so->pipe = *cso;

   /* check which states actually have differing values */
   if (cso->independent_blend_enable) {
      for (r = 0; r < 8 && !cso->rt[r].blend_enable; ++r);
      blend_en |= 1 << r;
      for (i = r + 1; i < 8; ++i) {
         if (!cso->rt[i].blend_enable)
            continue;
         blend_en |= 1 << i;
         if (cso->rt[i].rgb_func != cso->rt[r].rgb_func ||
             cso->rt[i].rgb_src_factor != cso->rt[r].rgb_src_factor ||
             cso->rt[i].rgb_dst_factor != cso->rt[r].rgb_dst_factor ||
             cso->rt[i].alpha_func != cso->rt[r].alpha_func ||
             cso->rt[i].alpha_src_factor != cso->rt[r].alpha_src_factor ||
             cso->rt[i].alpha_dst_factor != cso->rt[r].alpha_dst_factor) {
            indep_funcs = true;
            break;
         }
      }
      for (; i < 8; ++i)
         blend_en |= (cso->rt[i].blend_enable ? 1 : 0) << i;

      for (i = 1; i < 8; ++i) {
         if (cso->rt[i].colormask != cso->rt[0].colormask) {
            indep_masks = true;
            break;
         }
      }
   } else {
      r = 0;
      if (cso->rt[0].blend_enable)
         blend_en = 0xff;
   }

   if (cso->logicop_enable) {
      SB_BEGIN_3D(so, LOGIC_OP_ENABLE, 2);
      SB_DATA    (so, 1);
      SB_DATA    (so, nvgl_logicop_func(cso->logicop_func));

      SB_IMMED_3D(so, MACRO_BLEND_ENABLES, 0);
   } else {
      SB_IMMED_3D(so, LOGIC_OP_ENABLE, 0);

      SB_IMMED_3D(so, BLEND_INDEPENDENT, indep_funcs);
      SB_IMMED_3D(so, MACRO_BLEND_ENABLES, blend_en);
      if (indep_funcs) {
         for (i = 0; i < 8; ++i) {
            if (cso->rt[i].blend_enable) {
               SB_BEGIN_3D(so, IBLEND_EQUATION_RGB(i), 6);
               SB_DATA    (so, nvgl_blend_eqn(cso->rt[i].rgb_func));
               SB_DATA    (so, nvc0_blend_fac(cso->rt[i].rgb_src_factor));
               SB_DATA    (so, nvc0_blend_fac(cso->rt[i].rgb_dst_factor));
               SB_DATA    (so, nvgl_blend_eqn(cso->rt[i].alpha_func));
               SB_DATA    (so, nvc0_blend_fac(cso->rt[i].alpha_src_factor));
               SB_DATA    (so, nvc0_blend_fac(cso->rt[i].alpha_dst_factor));
            }
         }
      } else
      if (blend_en) {
         SB_BEGIN_3D(so, BLEND_EQUATION_RGB, 5);
         SB_DATA    (so, nvgl_blend_eqn(cso->rt[r].rgb_func));
         SB_DATA    (so, nvc0_blend_fac(cso->rt[r].rgb_src_factor));
         SB_DATA    (so, nvc0_blend_fac(cso->rt[r].rgb_dst_factor));
         SB_DATA    (so, nvgl_blend_eqn(cso->rt[r].alpha_func));
         SB_DATA    (so, nvc0_blend_fac(cso->rt[r].alpha_src_factor));
         SB_BEGIN_3D(so, BLEND_FUNC_DST_ALPHA, 1);
         SB_DATA    (so, nvc0_blend_fac(cso->rt[r].alpha_dst_factor));
      }

      SB_IMMED_3D(so, COLOR_MASK_COMMON, !indep_masks);
      if (indep_masks) {
         SB_BEGIN_3D(so, COLOR_MASK(0), 8);
         for (i = 0; i < 8; ++i)
            SB_DATA(so, nvc0_colormask(cso->rt[i].colormask));
      } else {
         SB_BEGIN_3D(so, COLOR_MASK(0), 1);
         SB_DATA    (so, nvc0_colormask(cso->rt[0].colormask));
      }
   }

   ms = 0;
   if (cso->alpha_to_coverage)
      ms |= NVC0_3D_MULTISAMPLE_CTRL_ALPHA_TO_COVERAGE;
   if (cso->alpha_to_one)
      ms |= NVC0_3D_MULTISAMPLE_CTRL_ALPHA_TO_ONE;

   SB_BEGIN_3D(so, MULTISAMPLE_CTRL, 1);
   SB_DATA    (so, ms);

   assert(so->size <= ARRAY_SIZE(so->state));
   return so;
}

static void
nvc0_blend_state_bind(struct pipe_context *pipe, void *hwcso)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->blend = hwcso;
    nvc0->dirty_3d |= NVC0_NEW_3D_BLEND;
}

static void
nvc0_blend_state_delete(struct pipe_context *pipe, void *hwcso)
{
    FREE(hwcso);
}

/* NOTE: ignoring line_last_pixel */
static void *
nvc0_rasterizer_state_create(struct pipe_context *pipe,
                             const struct pipe_rasterizer_state *cso)
{
    struct nvc0_rasterizer_stateobj *so;
    uint16_t class_3d = nouveau_screen(pipe->screen)->class_3d;
    uint32_t reg;

    so = CALLOC_STRUCT(nvc0_rasterizer_stateobj);
    if (!so)
        return NULL;
    so->pipe = *cso;

    /* Scissor enables are handled in scissor state, we will not want to
     * always emit 16 commands, one for each scissor rectangle, here.
     */

    SB_IMMED_3D(so, PROVOKING_VERTEX_LAST, !cso->flatshade_first);
    SB_IMMED_3D(so, VERTEX_TWO_SIDE_ENABLE, cso->light_twoside);

    SB_IMMED_3D(so, VERT_COLOR_CLAMP_EN, cso->clamp_vertex_color);
    SB_BEGIN_3D(so, FRAG_COLOR_CLAMP_EN, 1);
    SB_DATA    (so, cso->clamp_fragment_color ? 0x11111111 : 0x00000000);

    SB_IMMED_3D(so, MULTISAMPLE_ENABLE, cso->multisample);

    SB_IMMED_3D(so, LINE_SMOOTH_ENABLE, cso->line_smooth);
    if (cso->line_smooth || cso->multisample)
       SB_BEGIN_3D(so, LINE_WIDTH_SMOOTH, 1);
    else
       SB_BEGIN_3D(so, LINE_WIDTH_ALIASED, 1);
    SB_DATA    (so, fui(cso->line_width));

    SB_IMMED_3D(so, LINE_STIPPLE_ENABLE, cso->line_stipple_enable);
    if (cso->line_stipple_enable) {
        SB_BEGIN_3D(so, LINE_STIPPLE_PATTERN, 1);
        SB_DATA    (so, (cso->line_stipple_pattern << 8) |
                         cso->line_stipple_factor);

    }

    SB_IMMED_3D(so, VP_POINT_SIZE, cso->point_size_per_vertex);
    if (!cso->point_size_per_vertex) {
       SB_BEGIN_3D(so, POINT_SIZE, 1);
       SB_DATA    (so, fui(cso->point_size));
    }

    reg = (cso->sprite_coord_mode == PIPE_SPRITE_COORD_UPPER_LEFT) ?
       NVC0_3D_POINT_COORD_REPLACE_COORD_ORIGIN_UPPER_LEFT :
       NVC0_3D_POINT_COORD_REPLACE_COORD_ORIGIN_LOWER_LEFT;

    SB_BEGIN_3D(so, POINT_COORD_REPLACE, 1);
    SB_DATA    (so, ((cso->sprite_coord_enable & 0xff) << 3) | reg);
    SB_IMMED_3D(so, POINT_SPRITE_ENABLE, cso->point_quad_rasterization);
    SB_IMMED_3D(so, POINT_SMOOTH_ENABLE, cso->point_smooth);

    if (class_3d >= GM200_3D_CLASS) {
       SB_IMMED_3D(so, FILL_RECTANGLE,
                   cso->fill_front == PIPE_POLYGON_MODE_FILL_RECTANGLE ?
                   NVC0_3D_FILL_RECTANGLE_ENABLE : 0);
    }

    SB_BEGIN_3D(so, MACRO_POLYGON_MODE_FRONT, 1);
    SB_DATA    (so, nvgl_polygon_mode(cso->fill_front));
    SB_BEGIN_3D(so, MACRO_POLYGON_MODE_BACK, 1);
    SB_DATA    (so, nvgl_polygon_mode(cso->fill_back));
    SB_IMMED_3D(so, POLYGON_SMOOTH_ENABLE, cso->poly_smooth);

    SB_BEGIN_3D(so, CULL_FACE_ENABLE, 3);
    SB_DATA    (so, cso->cull_face != PIPE_FACE_NONE);
    SB_DATA    (so, cso->front_ccw ? NVC0_3D_FRONT_FACE_CCW :
                                     NVC0_3D_FRONT_FACE_CW);
    switch (cso->cull_face) {
    case PIPE_FACE_FRONT_AND_BACK:
       SB_DATA(so, NVC0_3D_CULL_FACE_FRONT_AND_BACK);
       break;
    case PIPE_FACE_FRONT:
       SB_DATA(so, NVC0_3D_CULL_FACE_FRONT);
       break;
    case PIPE_FACE_BACK:
    default:
       SB_DATA(so, NVC0_3D_CULL_FACE_BACK);
       break;
    }

    SB_IMMED_3D(so, POLYGON_STIPPLE_ENABLE, cso->poly_stipple_enable);
    SB_BEGIN_3D(so, POLYGON_OFFSET_POINT_ENABLE, 3);
    SB_DATA    (so, cso->offset_point);
    SB_DATA    (so, cso->offset_line);
    SB_DATA    (so, cso->offset_tri);

    if (cso->offset_point || cso->offset_line || cso->offset_tri) {
        SB_BEGIN_3D(so, POLYGON_OFFSET_FACTOR, 1);
        SB_DATA    (so, fui(cso->offset_scale));
        if (!cso->offset_units_unscaled) {
           SB_BEGIN_3D(so, POLYGON_OFFSET_UNITS, 1);
           SB_DATA    (so, fui(cso->offset_units * 2.0f));
        }
        SB_BEGIN_3D(so, POLYGON_OFFSET_CLAMP, 1);
        SB_DATA    (so, fui(cso->offset_clamp));
    }

    if (cso->depth_clip_near)
       reg = NVC0_3D_VIEW_VOLUME_CLIP_CTRL_UNK1_UNK1;
    else
       reg =
          NVC0_3D_VIEW_VOLUME_CLIP_CTRL_UNK1_UNK1 |
          NVC0_3D_VIEW_VOLUME_CLIP_CTRL_DEPTH_CLAMP_NEAR |
          NVC0_3D_VIEW_VOLUME_CLIP_CTRL_DEPTH_CLAMP_FAR |
          NVC0_3D_VIEW_VOLUME_CLIP_CTRL_UNK12_UNK2;

    SB_BEGIN_3D(so, VIEW_VOLUME_CLIP_CTRL, 1);
    SB_DATA    (so, reg);

    SB_IMMED_3D(so, DEPTH_CLIP_NEGATIVE_Z, cso->clip_halfz);

    SB_IMMED_3D(so, PIXEL_CENTER_INTEGER, !cso->half_pixel_center);

    if (class_3d >= GM200_3D_CLASS) {
        if (cso->conservative_raster_mode != PIPE_CONSERVATIVE_RASTER_OFF) {
            bool post_snap = cso->conservative_raster_mode ==
                PIPE_CONSERVATIVE_RASTER_POST_SNAP;
            uint32_t state = cso->subpixel_precision_x;
            state |= cso->subpixel_precision_y << 4;
            state |= (uint32_t)(cso->conservative_raster_dilate * 4) << 8;
            state |= (post_snap || class_3d < GP100_3D_CLASS) ? 1 << 10 : 0;
            SB_IMMED_3D(so, MACRO_CONSERVATIVE_RASTER_STATE, state);
        } else {
            SB_IMMED_3D(so, CONSERVATIVE_RASTER, 0);
        }
    }

    assert(so->size <= ARRAY_SIZE(so->state));
    return (void *)so;
}

static void
nvc0_rasterizer_state_bind(struct pipe_context *pipe, void *hwcso)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);

   nvc0->rast = hwcso;
   nvc0->dirty_3d |= NVC0_NEW_3D_RASTERIZER;
}

static void
nvc0_rasterizer_state_delete(struct pipe_context *pipe, void *hwcso)
{
   FREE(hwcso);
}

static void *
nvc0_zsa_state_create(struct pipe_context *pipe,
                      const struct pipe_depth_stencil_alpha_state *cso)
{
   struct nvc0_zsa_stateobj *so = CALLOC_STRUCT(nvc0_zsa_stateobj);

   so->pipe = *cso;

   SB_IMMED_3D(so, DEPTH_TEST_ENABLE, cso->depth_enabled);
   if (cso->depth_enabled) {
      SB_IMMED_3D(so, DEPTH_WRITE_ENABLE, cso->depth_writemask);
      SB_BEGIN_3D(so, DEPTH_TEST_FUNC, 1);
      SB_DATA    (so, nvgl_comparison_op(cso->depth_func));
   }

   SB_IMMED_3D(so, DEPTH_BOUNDS_EN, cso->depth_bounds_test);
   if (cso->depth_bounds_test) {
      SB_BEGIN_3D(so, DEPTH_BOUNDS(0), 2);
      SB_DATA    (so, fui(cso->depth_bounds_min));
      SB_DATA    (so, fui(cso->depth_bounds_max));
   }

   if (cso->stencil[0].enabled) {
      SB_BEGIN_3D(so, STENCIL_ENABLE, 5);
      SB_DATA    (so, 1);
      SB_DATA    (so, nvgl_stencil_op(cso->stencil[0].fail_op));
      SB_DATA    (so, nvgl_stencil_op(cso->stencil[0].zfail_op));
      SB_DATA    (so, nvgl_stencil_op(cso->stencil[0].zpass_op));
      SB_DATA    (so, nvgl_comparison_op(cso->stencil[0].func));
      SB_BEGIN_3D(so, STENCIL_FRONT_FUNC_MASK, 2);
      SB_DATA    (so, cso->stencil[0].valuemask);
      SB_DATA    (so, cso->stencil[0].writemask);
   } else {
      SB_IMMED_3D(so, STENCIL_ENABLE, 0);
   }

   if (cso->stencil[1].enabled) {
      assert(cso->stencil[0].enabled);
      SB_BEGIN_3D(so, STENCIL_TWO_SIDE_ENABLE, 5);
      SB_DATA    (so, 1);
      SB_DATA    (so, nvgl_stencil_op(cso->stencil[1].fail_op));
      SB_DATA    (so, nvgl_stencil_op(cso->stencil[1].zfail_op));
      SB_DATA    (so, nvgl_stencil_op(cso->stencil[1].zpass_op));
      SB_DATA    (so, nvgl_comparison_op(cso->stencil[1].func));
      SB_BEGIN_3D(so, STENCIL_BACK_MASK, 2);
      SB_DATA    (so, cso->stencil[1].writemask);
      SB_DATA    (so, cso->stencil[1].valuemask);
   } else
   if (cso->stencil[0].enabled) {
      SB_IMMED_3D(so, STENCIL_TWO_SIDE_ENABLE, 0);
   }

   SB_IMMED_3D(so, ALPHA_TEST_ENABLE, cso->alpha_enabled);
   if (cso->alpha_enabled) {
      SB_BEGIN_3D(so, ALPHA_TEST_REF, 2);
      SB_DATA    (so, fui(cso->alpha_ref_value));
      SB_DATA    (so, nvgl_comparison_op(cso->alpha_func));
   }

   assert(so->size <= ARRAY_SIZE(so->state));
   return (void *)so;
}

static void
nvc0_zsa_state_bind(struct pipe_context *pipe, void *hwcso)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);

   nvc0->zsa = hwcso;
   nvc0->dirty_3d |= NVC0_NEW_3D_ZSA;
}

static void
nvc0_zsa_state_delete(struct pipe_context *pipe, void *hwcso)
{
   FREE(hwcso);
}

/* ====================== SAMPLERS AND TEXTURES ================================
 */

#define NV50_TSC_WRAP_CASE(n) \
    case PIPE_TEX_WRAP_##n: return NV50_TSC_WRAP_##n

static void
nvc0_sampler_state_delete(struct pipe_context *pipe, void *hwcso)
{
   unsigned s, i;

   for (s = 0; s < 6; ++s)
      for (i = 0; i < nvc0_context(pipe)->num_samplers[s]; ++i)
         if (nvc0_context(pipe)->samplers[s][i] == hwcso)
            nvc0_context(pipe)->samplers[s][i] = NULL;

   nvc0_screen_tsc_free(nvc0_context(pipe)->screen, nv50_tsc_entry(hwcso));

   FREE(hwcso);
}

static inline void
nvc0_stage_sampler_states_bind(struct nvc0_context *nvc0,
                               unsigned s,
                               unsigned nr, void **hwcsos)
{
   unsigned highest_found = 0;
   unsigned i;

   for (i = 0; i < nr; ++i) {
      struct nv50_tsc_entry *hwcso = hwcsos ? nv50_tsc_entry(hwcsos[i]) : NULL;
      struct nv50_tsc_entry *old = nvc0->samplers[s][i];

      if (hwcso)
         highest_found = i;

      if (hwcso == old)
         continue;
      nvc0->samplers_dirty[s] |= 1 << i;

      nvc0->samplers[s][i] = hwcso;
      if (old)
         nvc0_screen_tsc_unlock(nvc0->screen, old);
   }
   if (nr >= nvc0->num_samplers[s])
      nvc0->num_samplers[s] = highest_found + 1;
}

static void
nvc0_bind_sampler_states(struct pipe_context *pipe,
                         enum pipe_shader_type shader,
                         unsigned start, unsigned nr, void **samplers)
{
   const unsigned s = nvc0_shader_stage(shader);

   assert(start == 0);
   nvc0_stage_sampler_states_bind(nvc0_context(pipe), s, nr, samplers);

   if (s == 5)
      nvc0_context(pipe)->dirty_cp |= NVC0_NEW_CP_SAMPLERS;
   else
      nvc0_context(pipe)->dirty_3d |= NVC0_NEW_3D_SAMPLERS;
}


/* NOTE: only called when not referenced anywhere, won't be bound */
static void
nvc0_sampler_view_destroy(struct pipe_context *pipe,
                          struct pipe_sampler_view *view)
{
   pipe_resource_reference(&view->texture, NULL);

   nvc0_screen_tic_free(nvc0_context(pipe)->screen, nv50_tic_entry(view));

   FREE(nv50_tic_entry(view));
}

static inline void
nvc0_stage_set_sampler_views(struct nvc0_context *nvc0, int s,
                             unsigned nr, bool take_ownership,
                             struct pipe_sampler_view **views)
{
   unsigned i;

   for (i = 0; i < nr; ++i) {
      struct pipe_sampler_view *view = views ? views[i] : NULL;
      struct nv50_tic_entry *old = nv50_tic_entry(nvc0->textures[s][i]);

      if (view == nvc0->textures[s][i]) {
         if (take_ownership)
            pipe_sampler_view_reference(&view, NULL);
         continue;
      }
      nvc0->textures_dirty[s] |= 1 << i;

      if (view && view->texture) {
         struct pipe_resource *res = view->texture;
         if (res->target == PIPE_BUFFER &&
             (res->flags & PIPE_RESOURCE_FLAG_MAP_COHERENT))
            nvc0->textures_coherent[s] |= 1 << i;
         else
            nvc0->textures_coherent[s] &= ~(1 << i);
      } else {
         nvc0->textures_coherent[s] &= ~(1 << i);
      }

      if (old) {
         if (s == 5)
            nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_TEX(i));
         else
            nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_TEX(s, i));
         nvc0_screen_tic_unlock(nvc0->screen, old);
      }

      if (take_ownership) {
         pipe_sampler_view_reference(&nvc0->textures[s][i], NULL);
         nvc0->textures[s][i] = view;
      } else {
         pipe_sampler_view_reference(&nvc0->textures[s][i], view);
      }
   }

   for (i = nr; i < nvc0->num_textures[s]; ++i) {
      struct nv50_tic_entry *old = nv50_tic_entry(nvc0->textures[s][i]);
      if (old) {
         if (s == 5)
            nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_TEX(i));
         else
            nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_TEX(s, i));
         nvc0_screen_tic_unlock(nvc0->screen, old);
         pipe_sampler_view_reference(&nvc0->textures[s][i], NULL);
      }
   }

   nvc0->num_textures[s] = nr;
}

static void
nvc0_set_sampler_views(struct pipe_context *pipe, enum pipe_shader_type shader,
                       unsigned start, unsigned nr,
                       unsigned unbind_num_trailing_slots,
                       bool take_ownership,
                       struct pipe_sampler_view **views)
{
   const unsigned s = nvc0_shader_stage(shader);

   assert(start == 0);
   nvc0_stage_set_sampler_views(nvc0_context(pipe), s, nr, take_ownership, views);

   if (s == 5)
      nvc0_context(pipe)->dirty_cp |= NVC0_NEW_CP_TEXTURES;
   else
      nvc0_context(pipe)->dirty_3d |= NVC0_NEW_3D_TEXTURES;
}

/* ============================= SHADERS =======================================
 */

static void *
nvc0_sp_state_create(struct pipe_context *pipe,
                     const struct pipe_shader_state *cso, unsigned type)
{
   struct nvc0_program *prog;

   prog = CALLOC_STRUCT(nvc0_program);
   if (!prog)
      return NULL;

   prog->type = type;

   switch(cso->type) {
   case PIPE_SHADER_IR_TGSI:
      prog->nir = tgsi_to_nir(cso->tokens, pipe->screen, false);
      break;
   case PIPE_SHADER_IR_NIR:
      prog->nir = cso->ir.nir;
      break;
   default:
      assert(!"unsupported IR!");
      free(prog);
      return NULL;
   }

   if (cso->stream_output.num_outputs)
      prog->stream_output = cso->stream_output;

   prog->translated = nvc0_program_translate(
      prog, nvc0_context(pipe)->screen->base.device->chipset,
      nvc0_context(pipe)->screen->base.disk_shader_cache,
      &nouveau_context(pipe)->debug);

   return (void *)prog;
}

static void
nvc0_sp_state_delete(struct pipe_context *pipe, void *hwcso)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   struct nvc0_program *prog = (struct nvc0_program *)hwcso;

   simple_mtx_lock(&nvc0->screen->state_lock);
   nvc0_program_destroy(nvc0_context(pipe), prog);
   simple_mtx_unlock(&nvc0->screen->state_lock);

   ralloc_free(prog->nir);
   FREE(prog);
}

static void *
nvc0_vp_state_create(struct pipe_context *pipe,
                     const struct pipe_shader_state *cso)
{
   return nvc0_sp_state_create(pipe, cso, PIPE_SHADER_VERTEX);
}

static void
nvc0_vp_state_bind(struct pipe_context *pipe, void *hwcso)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->vertprog = hwcso;
    nvc0->dirty_3d |= NVC0_NEW_3D_VERTPROG;
}

static void *
nvc0_fp_state_create(struct pipe_context *pipe,
                     const struct pipe_shader_state *cso)
{
   return nvc0_sp_state_create(pipe, cso, PIPE_SHADER_FRAGMENT);
}

static void
nvc0_fp_state_bind(struct pipe_context *pipe, void *hwcso)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->fragprog = hwcso;
    nvc0->dirty_3d |= NVC0_NEW_3D_FRAGPROG;
}

static void *
nvc0_gp_state_create(struct pipe_context *pipe,
                     const struct pipe_shader_state *cso)
{
   return nvc0_sp_state_create(pipe, cso, PIPE_SHADER_GEOMETRY);
}

static void
nvc0_gp_state_bind(struct pipe_context *pipe, void *hwcso)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->gmtyprog = hwcso;
    nvc0->dirty_3d |= NVC0_NEW_3D_GMTYPROG;
}

static void *
nvc0_tcp_state_create(struct pipe_context *pipe,
                     const struct pipe_shader_state *cso)
{
   return nvc0_sp_state_create(pipe, cso, PIPE_SHADER_TESS_CTRL);
}

static void
nvc0_tcp_state_bind(struct pipe_context *pipe, void *hwcso)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->tctlprog = hwcso;
    nvc0->dirty_3d |= NVC0_NEW_3D_TCTLPROG;
}

static void *
nvc0_tep_state_create(struct pipe_context *pipe,
                     const struct pipe_shader_state *cso)
{
   return nvc0_sp_state_create(pipe, cso, PIPE_SHADER_TESS_EVAL);
}

static void
nvc0_tep_state_bind(struct pipe_context *pipe, void *hwcso)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->tevlprog = hwcso;
    nvc0->dirty_3d |= NVC0_NEW_3D_TEVLPROG;
}

static void *
nvc0_cp_state_create(struct pipe_context *pipe,
                     const struct pipe_compute_state *cso)
{
   struct nvc0_program *prog;

   prog = CALLOC_STRUCT(nvc0_program);
   if (!prog)
      return NULL;
   prog->type = PIPE_SHADER_COMPUTE;

   prog->cp.smem_size = cso->static_shared_mem;
   prog->parm_size = cso->req_input_mem;

   switch(cso->ir_type) {
   case PIPE_SHADER_IR_TGSI: {
      const struct tgsi_token *tokens = cso->prog;
      prog->nir = tgsi_to_nir(tokens, pipe->screen, false);
      break;
   }
   case PIPE_SHADER_IR_NIR:
      prog->nir = (nir_shader *)cso->prog;
      break;
   case PIPE_SHADER_IR_NIR_SERIALIZED: {
      struct blob_reader reader;
      const struct pipe_binary_program_header *hdr = cso->prog;

      blob_reader_init(&reader, hdr->blob, hdr->num_bytes);
      prog->nir = nir_deserialize(NULL, pipe->screen->get_compiler_options(pipe->screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE), &reader);
      break;
   }
   default:
      assert(!"unsupported IR!");
      free(prog);
      return NULL;
   }

   prog->translated = nvc0_program_translate(
      prog, nvc0_context(pipe)->screen->base.device->chipset,
      nvc0_context(pipe)->screen->base.disk_shader_cache,
      &nouveau_context(pipe)->debug);

   return (void *)prog;
}

static void
nvc0_cp_state_bind(struct pipe_context *pipe, void *hwcso)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->compprog = hwcso;
    nvc0->dirty_cp |= NVC0_NEW_CP_PROGRAM;
}

static void
nvc0_get_compute_state_info(struct pipe_context *pipe, void *hwcso,
                            struct pipe_compute_state_object_info *info)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   struct nvc0_program *prog = (struct nvc0_program *)hwcso;
   uint16_t obj_class = nvc0->screen->compute->oclass;
   uint32_t chipset = nvc0->screen->base.device->chipset;
   uint32_t smregs;

   // fermi and a handful of tegra devices have less gprs per SM
   if (obj_class < NVE4_COMPUTE_CLASS || chipset == 0xea || chipset == 0x12b || chipset == 0x13b)
      smregs = 32768;
   else
      smregs = 65536;

   // TODO: not 100% sure about 8 for volta, but earlier reverse engineering indicates it
   uint32_t gpr_alloc_size = obj_class >= GV100_COMPUTE_CLASS ? 8 : 4;
   uint32_t threads = smregs / align(prog->num_gprs, gpr_alloc_size);

   info->max_threads = MIN2(ROUND_DOWN_TO(threads, 32), 1024);
   info->private_memory = prog->hdr[1] & 0xfffff0;
   info->preferred_simd_size = 32;
   info->simd_sizes = 32;
}

static void
nvc0_set_constant_buffer(struct pipe_context *pipe,
                         enum pipe_shader_type shader, uint index,
                         bool take_ownership,
                         const struct pipe_constant_buffer *cb)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   struct pipe_resource *res = cb ? cb->buffer : NULL;
   const unsigned s = nvc0_shader_stage(shader);
   const unsigned i = index;

   if (unlikely(shader == PIPE_SHADER_COMPUTE)) {
      if (nvc0->constbuf[s][i].user)
         nvc0->constbuf[s][i].u.buf = NULL;
      else
      if (nvc0->constbuf[s][i].u.buf)
         nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_CB(i));

      nvc0->dirty_cp |= NVC0_NEW_CP_CONSTBUF;
   } else {
      if (nvc0->constbuf[s][i].user)
         nvc0->constbuf[s][i].u.buf = NULL;
      else
      if (nvc0->constbuf[s][i].u.buf)
         nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_CB(s, i));

      nvc0->dirty_3d |= NVC0_NEW_3D_CONSTBUF;
   }
   nvc0->constbuf_dirty[s] |= 1 << i;

   if (nvc0->constbuf[s][i].u.buf)
      nv04_resource(nvc0->constbuf[s][i].u.buf)->cb_bindings[s] &= ~(1 << i);

   if (take_ownership) {
      pipe_resource_reference(&nvc0->constbuf[s][i].u.buf, NULL);
      nvc0->constbuf[s][i].u.buf = res;
   } else {
      pipe_resource_reference(&nvc0->constbuf[s][i].u.buf, res);
   }

   nvc0->constbuf[s][i].user = (cb && cb->user_buffer) ? true : false;
   if (nvc0->constbuf[s][i].user) {
      nvc0->constbuf[s][i].u.data = cb->user_buffer;
      nvc0->constbuf[s][i].size = MIN2(cb->buffer_size, 0x10000);
      nvc0->constbuf_valid[s] |= 1 << i;
      nvc0->constbuf_coherent[s] &= ~(1 << i);
   } else
   if (cb) {
      nvc0->constbuf[s][i].offset = cb->buffer_offset;
      nvc0->constbuf[s][i].size = MIN2(align(cb->buffer_size, 0x100), 0x10000);
      nvc0->constbuf_valid[s] |= 1 << i;
      if (res && res->flags & PIPE_RESOURCE_FLAG_MAP_COHERENT)
         nvc0->constbuf_coherent[s] |= 1 << i;
      else
         nvc0->constbuf_coherent[s] &= ~(1 << i);
   }
   else {
      nvc0->constbuf_valid[s] &= ~(1 << i);
      nvc0->constbuf_coherent[s] &= ~(1 << i);
   }
}

/* =============================================================================
 */

static void
nvc0_set_blend_color(struct pipe_context *pipe,
                     const struct pipe_blend_color *bcol)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->blend_colour = *bcol;
    nvc0->dirty_3d |= NVC0_NEW_3D_BLEND_COLOUR;
}

static void
nvc0_set_stencil_ref(struct pipe_context *pipe,
                     const struct pipe_stencil_ref sr)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->stencil_ref = sr;
    nvc0->dirty_3d |= NVC0_NEW_3D_STENCIL_REF;
}

static void
nvc0_set_clip_state(struct pipe_context *pipe,
                    const struct pipe_clip_state *clip)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    memcpy(nvc0->clip.ucp, clip->ucp, sizeof(clip->ucp));

    nvc0->dirty_3d |= NVC0_NEW_3D_CLIP;
}

static void
nvc0_set_sample_mask(struct pipe_context *pipe, unsigned sample_mask)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->sample_mask = sample_mask;
    nvc0->dirty_3d |= NVC0_NEW_3D_SAMPLE_MASK;
}

static void
nvc0_set_min_samples(struct pipe_context *pipe, unsigned min_samples)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);

   if (nvc0->min_samples != min_samples) {
      nvc0->min_samples = min_samples;
      nvc0->dirty_3d |= NVC0_NEW_3D_MIN_SAMPLES;
   }
}

static void
nvc0_set_framebuffer_state(struct pipe_context *pipe,
                           const struct pipe_framebuffer_state *fb)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_FB);

    util_copy_framebuffer_state(&nvc0->framebuffer, fb);

    nvc0->dirty_3d |= NVC0_NEW_3D_FRAMEBUFFER | NVC0_NEW_3D_SAMPLE_LOCATIONS |
       NVC0_NEW_3D_TEXTURES;
    nvc0->dirty_cp |= NVC0_NEW_CP_TEXTURES;
}

static void
nvc0_set_sample_locations(struct pipe_context *pipe,
                          size_t size, const uint8_t *locations)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->sample_locations_enabled = size && locations;
    if (size > sizeof(nvc0->sample_locations))
       size = sizeof(nvc0->sample_locations);
    memcpy(nvc0->sample_locations, locations, size);

    nvc0->dirty_3d |= NVC0_NEW_3D_SAMPLE_LOCATIONS;
}

static void
nvc0_set_polygon_stipple(struct pipe_context *pipe,
                         const struct pipe_poly_stipple *stipple)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->stipple = *stipple;
    nvc0->dirty_3d |= NVC0_NEW_3D_STIPPLE;
}

static void
nvc0_set_scissor_states(struct pipe_context *pipe,
                        unsigned start_slot,
                        unsigned num_scissors,
                        const struct pipe_scissor_state *scissor)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   int i;

   assert(start_slot + num_scissors <= NVC0_MAX_VIEWPORTS);
   for (i = 0; i < num_scissors; i++) {
      if (!memcmp(&nvc0->scissors[start_slot + i], &scissor[i], sizeof(*scissor)))
         continue;
      nvc0->scissors[start_slot + i] = scissor[i];
      nvc0->scissors_dirty |= 1 << (start_slot + i);
      nvc0->dirty_3d |= NVC0_NEW_3D_SCISSOR;
   }
}

static void
nvc0_set_viewport_states(struct pipe_context *pipe,
                         unsigned start_slot,
                         unsigned num_viewports,
                         const struct pipe_viewport_state *vpt)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   int i;

   assert(start_slot + num_viewports <= NVC0_MAX_VIEWPORTS);
   for (i = 0; i < num_viewports; i++) {
      if (!memcmp(&nvc0->viewports[start_slot + i], &vpt[i], sizeof(*vpt)))
         continue;
      nvc0->viewports[start_slot + i] = vpt[i];
      nvc0->viewports_dirty |= 1 << (start_slot + i);
      nvc0->dirty_3d |= NVC0_NEW_3D_VIEWPORT;
   }

}

static void
nvc0_set_window_rectangles(struct pipe_context *pipe,
                           bool include,
                           unsigned num_rectangles,
                           const struct pipe_scissor_state *rectangles)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);

   nvc0->window_rect.inclusive = include;
   nvc0->window_rect.rects = MIN2(num_rectangles, NVC0_MAX_WINDOW_RECTANGLES);
   memcpy(nvc0->window_rect.rect, rectangles,
          sizeof(struct pipe_scissor_state) * nvc0->window_rect.rects);

   nvc0->dirty_3d |= NVC0_NEW_3D_WINDOW_RECTS;
}

static void
nvc0_set_tess_state(struct pipe_context *pipe,
                    const float default_tess_outer[4],
                    const float default_tess_inner[2])
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);

   memcpy(nvc0->default_tess_outer, default_tess_outer, 4 * sizeof(float));
   memcpy(nvc0->default_tess_inner, default_tess_inner, 2 * sizeof(float));
   nvc0->dirty_3d |= NVC0_NEW_3D_TESSFACTOR;
}

static void
nvc0_set_patch_vertices(struct pipe_context *pipe, uint8_t patch_vertices)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);

   nvc0->patch_vertices = patch_vertices;
}

static void
nvc0_set_vertex_buffers(struct pipe_context *pipe,
                        unsigned count,
                        unsigned unbind_num_trailing_slots,
                        bool take_ownership,
                        const struct pipe_vertex_buffer *vb)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);
    unsigned i;

    nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_VTX);
    nvc0->dirty_3d |= NVC0_NEW_3D_ARRAYS;

    util_set_vertex_buffers_count(nvc0->vtxbuf, &nvc0->num_vtxbufs, vb,
                                  count, unbind_num_trailing_slots,
                                  take_ownership);

    unsigned clear_mask = ~u_bit_consecutive(count, unbind_num_trailing_slots);
    nvc0->vbo_user &= clear_mask;
    nvc0->constant_vbos &= clear_mask;
    nvc0->vtxbufs_coherent &= clear_mask;

    if (!vb) {
       clear_mask = ~u_bit_consecutive(0, count);
       nvc0->vbo_user &= clear_mask;
       nvc0->constant_vbos &= clear_mask;
       nvc0->vtxbufs_coherent &= clear_mask;
       return;
    }

    for (i = 0; i < count; ++i) {
       unsigned dst_index = i;

       if (vb[i].is_user_buffer) {
          nvc0->vbo_user |= 1 << dst_index;
          nvc0->vtxbufs_coherent &= ~(1 << dst_index);
       } else {
          nvc0->vbo_user &= ~(1 << dst_index);

          if (vb[i].buffer.resource &&
              vb[i].buffer.resource->flags & PIPE_RESOURCE_FLAG_MAP_COHERENT)
             nvc0->vtxbufs_coherent |= (1 << dst_index);
          else
             nvc0->vtxbufs_coherent &= ~(1 << dst_index);
       }
    }
}

static void
nvc0_vertex_state_bind(struct pipe_context *pipe, void *hwcso)
{
    struct nvc0_context *nvc0 = nvc0_context(pipe);

    nvc0->vertex = hwcso;
    nvc0->dirty_3d |= NVC0_NEW_3D_VERTEX;
}

static struct pipe_stream_output_target *
nvc0_so_target_create(struct pipe_context *pipe,
                      struct pipe_resource *res,
                      unsigned offset, unsigned size)
{
   struct nv04_resource *buf = (struct nv04_resource *)res;
   struct nvc0_so_target *targ = MALLOC_STRUCT(nvc0_so_target);
   if (!targ)
      return NULL;

   targ->pq = pipe->create_query(pipe, NVC0_HW_QUERY_TFB_BUFFER_OFFSET, 0);
   if (!targ->pq) {
      FREE(targ);
      return NULL;
   }
   targ->clean = true;

   targ->pipe.buffer_size = size;
   targ->pipe.buffer_offset = offset;
   targ->pipe.context = pipe;
   targ->pipe.buffer = NULL;
   pipe_resource_reference(&targ->pipe.buffer, res);
   pipe_reference_init(&targ->pipe.reference, 1);

   assert(buf->base.target == PIPE_BUFFER);
   util_range_add(&buf->base, &buf->valid_buffer_range, offset, offset + size);

   return &targ->pipe;
}

static void
nvc0_so_target_save_offset(struct pipe_context *pipe,
                           struct pipe_stream_output_target *ptarg,
                           unsigned index, bool *serialize)
{
   struct nvc0_so_target *targ = nvc0_so_target(ptarg);

   if (*serialize) {
      *serialize = false;
      PUSH_SPACE(nvc0_context(pipe)->base.pushbuf, 1);
      IMMED_NVC0(nvc0_context(pipe)->base.pushbuf, NVC0_3D(SERIALIZE), 0);

      NOUVEAU_DRV_STAT(nouveau_screen(pipe->screen), gpu_serialize_count, 1);
   }

   nvc0_query(targ->pq)->index = index;
   pipe->end_query(pipe, targ->pq);
}

static void
nvc0_so_target_destroy(struct pipe_context *pipe,
                       struct pipe_stream_output_target *ptarg)
{
   struct nvc0_so_target *targ = nvc0_so_target(ptarg);
   pipe->destroy_query(pipe, targ->pq);
   pipe_resource_reference(&targ->pipe.buffer, NULL);
   FREE(targ);
}

static void
nvc0_set_transform_feedback_targets(struct pipe_context *pipe,
                                    unsigned num_targets,
                                    struct pipe_stream_output_target **targets,
                                    const unsigned *offsets)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   unsigned i;
   bool serialize = true;

   assert(num_targets <= 4);

   for (i = 0; i < num_targets; ++i) {
      const bool changed = nvc0->tfbbuf[i] != targets[i];
      const bool append = (offsets[i] == ((unsigned)-1));
      if (!changed && append)
         continue;
      nvc0->tfbbuf_dirty |= 1 << i;

      if (nvc0->tfbbuf[i] && changed)
         nvc0_so_target_save_offset(pipe, nvc0->tfbbuf[i], i, &serialize);

      if (targets[i] && !append)
         nvc0_so_target(targets[i])->clean = true;

      pipe_so_target_reference(&nvc0->tfbbuf[i], targets[i]);
   }
   for (; i < nvc0->num_tfbbufs; ++i) {
      if (nvc0->tfbbuf[i]) {
         nvc0->tfbbuf_dirty |= 1 << i;
         nvc0_so_target_save_offset(pipe, nvc0->tfbbuf[i], i, &serialize);
         pipe_so_target_reference(&nvc0->tfbbuf[i], NULL);
      }
   }
   nvc0->num_tfbbufs = num_targets;

   if (nvc0->tfbbuf_dirty) {
      nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_TFB);
      nvc0->dirty_3d |= NVC0_NEW_3D_TFB_TARGETS;
   }
}

static void
nvc0_bind_surfaces_range(struct nvc0_context *nvc0, const unsigned t,
                         unsigned start, unsigned nr,
                         struct pipe_surface **psurfaces)
{
   const unsigned end = start + nr;
   const unsigned mask = ((1 << nr) - 1) << start;
   unsigned i;

   if (psurfaces) {
      for (i = start; i < end; ++i) {
         const unsigned p = i - start;
         if (psurfaces[p])
            nvc0->surfaces_valid[t] |= (1 << i);
         else
            nvc0->surfaces_valid[t] &= ~(1 << i);
         pipe_surface_reference(&nvc0->surfaces[t][i], psurfaces[p]);
      }
   } else {
      for (i = start; i < end; ++i)
         pipe_surface_reference(&nvc0->surfaces[t][i], NULL);
      nvc0->surfaces_valid[t] &= ~mask;
   }
   nvc0->surfaces_dirty[t] |= mask;

   if (t == 0)
      nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_SUF);
   else
      nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_SUF);
}

static void
nvc0_set_compute_resources(struct pipe_context *pipe,
                           unsigned start, unsigned nr,
                           struct pipe_surface **resources)
{
   nvc0_bind_surfaces_range(nvc0_context(pipe), 1, start, nr, resources);

   nvc0_context(pipe)->dirty_cp |= NVC0_NEW_CP_SURFACES;
}

static bool
nvc0_bind_images_range(struct nvc0_context *nvc0, const unsigned s,
                       unsigned start, unsigned nr,
                       const struct pipe_image_view *pimages)
{
   const unsigned end = start + nr;
   unsigned mask = 0;
   unsigned i;

   assert(s < 6);

   if (pimages) {
      for (i = start; i < end; ++i) {
         struct pipe_image_view *img = &nvc0->images[s][i];
         const unsigned p = i - start;

         if (img->resource == pimages[p].resource &&
             img->format == pimages[p].format &&
             img->access == pimages[p].access) {
            if (img->resource == NULL)
               continue;
            if (img->resource->target == PIPE_BUFFER &&
                img->u.buf.offset == pimages[p].u.buf.offset &&
                img->u.buf.size == pimages[p].u.buf.size)
               continue;
            if (img->resource->target != PIPE_BUFFER &&
                img->u.tex.first_layer == pimages[p].u.tex.first_layer &&
                img->u.tex.last_layer == pimages[p].u.tex.last_layer &&
                img->u.tex.level == pimages[p].u.tex.level)
               continue;
         }

         mask |= (1 << i);
         if (pimages[p].resource)
            nvc0->images_valid[s] |= (1 << i);
         else
            nvc0->images_valid[s] &= ~(1 << i);

         img->format = pimages[p].format;
         img->access = pimages[p].access;
         if (pimages[p].resource && pimages[p].resource->target == PIPE_BUFFER)
            img->u.buf = pimages[p].u.buf;
         else
            img->u.tex = pimages[p].u.tex;

         pipe_resource_reference(
               &img->resource, pimages[p].resource);

         if (nvc0->screen->base.class_3d >= GM107_3D_CLASS) {
            if (nvc0->images_tic[s][i]) {
               struct nv50_tic_entry *old =
                  nv50_tic_entry(nvc0->images_tic[s][i]);
               nvc0_screen_tic_unlock(nvc0->screen, old);
               pipe_sampler_view_reference(&nvc0->images_tic[s][i], NULL);
            }

            nvc0->images_tic[s][i] =
               gm107_create_texture_view_from_image(&nvc0->base.pipe,
                                                    &pimages[p]);
         }
      }
      if (!mask)
         return false;
   } else {
      mask = ((1 << nr) - 1) << start;
      if (!(nvc0->images_valid[s] & mask))
         return false;
      for (i = start; i < end; ++i) {
         pipe_resource_reference(&nvc0->images[s][i].resource, NULL);
         if (nvc0->screen->base.class_3d >= GM107_3D_CLASS) {
            struct nv50_tic_entry *old = nv50_tic_entry(nvc0->images_tic[s][i]);
            if (old) {
               nvc0_screen_tic_unlock(nvc0->screen, old);
               pipe_sampler_view_reference(&nvc0->images_tic[s][i], NULL);
            }
         }
      }
      nvc0->images_valid[s] &= ~mask;
   }
   nvc0->images_dirty[s] |= mask;

   if (s == 5)
      nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_SUF);
   else
      nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_SUF);

   return true;
}

static void
nvc0_set_shader_images(struct pipe_context *pipe,
                       enum pipe_shader_type shader,
                       unsigned start, unsigned nr,
                       unsigned unbind_num_trailing_slots,
                       const struct pipe_image_view *images)
{
   const unsigned s = nvc0_shader_stage(shader);

   nvc0_bind_images_range(nvc0_context(pipe), s, start + nr,
                          unbind_num_trailing_slots, NULL);

   if (!nvc0_bind_images_range(nvc0_context(pipe), s, start, nr, images))
      return;

   if (s == 5)
      nvc0_context(pipe)->dirty_cp |= NVC0_NEW_CP_SURFACES;
   else
      nvc0_context(pipe)->dirty_3d |= NVC0_NEW_3D_SURFACES;
}

static bool
nvc0_bind_buffers_range(struct nvc0_context *nvc0, const unsigned t,
                        unsigned start, unsigned nr,
                        const struct pipe_shader_buffer *pbuffers)
{
   const unsigned end = start + nr;
   unsigned mask = 0;
   unsigned i;

   assert(t < 6);

   if (pbuffers) {
      for (i = start; i < end; ++i) {
         struct pipe_shader_buffer *buf = &nvc0->buffers[t][i];
         const unsigned p = i - start;
         if (buf->buffer == pbuffers[p].buffer &&
             buf->buffer_offset == pbuffers[p].buffer_offset &&
             buf->buffer_size == pbuffers[p].buffer_size)
            continue;

         mask |= (1 << i);
         if (pbuffers[p].buffer)
            nvc0->buffers_valid[t] |= (1 << i);
         else
            nvc0->buffers_valid[t] &= ~(1 << i);
         buf->buffer_offset = pbuffers[p].buffer_offset;
         buf->buffer_size = pbuffers[p].buffer_size;
         pipe_resource_reference(&buf->buffer, pbuffers[p].buffer);
      }
      if (!mask)
         return false;
   } else {
      mask = ((1 << nr) - 1) << start;
      if (!(nvc0->buffers_valid[t] & mask))
         return false;
      for (i = start; i < end; ++i)
         pipe_resource_reference(&nvc0->buffers[t][i].buffer, NULL);
      nvc0->buffers_valid[t] &= ~mask;
   }
   nvc0->buffers_dirty[t] |= mask;

   if (t == 5)
      nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_BUF);
   else
      nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_BUF);

   return true;
}

static void
nvc0_set_shader_buffers(struct pipe_context *pipe,
                        enum pipe_shader_type shader,
                        unsigned start, unsigned nr,
                        const struct pipe_shader_buffer *buffers,
                        unsigned writable_bitmask)
{
   const unsigned s = nvc0_shader_stage(shader);
   if (!nvc0_bind_buffers_range(nvc0_context(pipe), s, start, nr, buffers))
      return;

   if (s == 5)
      nvc0_context(pipe)->dirty_cp |= NVC0_NEW_CP_BUFFERS;
   else
      nvc0_context(pipe)->dirty_3d |= NVC0_NEW_3D_BUFFERS;
}

static inline void
nvc0_set_global_handle(uint32_t *phandle, struct pipe_resource *res)
{
   struct nv04_resource *buf = nv04_resource(res);
   if (buf) {
      uint64_t address = buf->address + *phandle;
      /* even though it's a pointer to uint32_t that's fine */
      memcpy(phandle, &address, 8);
   } else {
      *phandle = 0;
   }
}

static void
nvc0_set_global_bindings(struct pipe_context *pipe,
                         unsigned start, unsigned nr,
                         struct pipe_resource **resources,
                         uint32_t **handles)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   struct pipe_resource **ptr;
   unsigned i;
   const unsigned end = start + nr;

   if (!nr)
      return;

   if (nvc0->global_residents.size < (end * sizeof(struct pipe_resource *))) {
      const unsigned old_size = nvc0->global_residents.size;
      if (util_dynarray_resize(&nvc0->global_residents, struct pipe_resource *, end)) {
         memset((uint8_t *)nvc0->global_residents.data + old_size, 0,
                nvc0->global_residents.size - old_size);
      } else {
         NOUVEAU_ERR("Could not resize global residents array\n");
         return;
      }
   }

   if (resources) {
      ptr = util_dynarray_element(
         &nvc0->global_residents, struct pipe_resource *, start);
      for (i = 0; i < nr; ++i) {
         pipe_resource_reference(&ptr[i], resources[i]);
         nvc0_set_global_handle(handles[i], resources[i]);
      }
   } else {
      ptr = util_dynarray_element(
         &nvc0->global_residents, struct pipe_resource *, start);
      for (i = 0; i < nr; ++i)
         pipe_resource_reference(&ptr[i], NULL);
   }

   nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_GLOBAL);

   nvc0->dirty_cp |= NVC0_NEW_CP_GLOBALS;
}

void
nvc0_init_state_functions(struct nvc0_context *nvc0)
{
   struct pipe_context *pipe = &nvc0->base.pipe;

   pipe->create_blend_state = nvc0_blend_state_create;
   pipe->bind_blend_state = nvc0_blend_state_bind;
   pipe->delete_blend_state = nvc0_blend_state_delete;

   pipe->create_rasterizer_state = nvc0_rasterizer_state_create;
   pipe->bind_rasterizer_state = nvc0_rasterizer_state_bind;
   pipe->delete_rasterizer_state = nvc0_rasterizer_state_delete;

   pipe->create_depth_stencil_alpha_state = nvc0_zsa_state_create;
   pipe->bind_depth_stencil_alpha_state = nvc0_zsa_state_bind;
   pipe->delete_depth_stencil_alpha_state = nvc0_zsa_state_delete;

   pipe->create_sampler_state = nv50_sampler_state_create;
   pipe->delete_sampler_state = nvc0_sampler_state_delete;
   pipe->bind_sampler_states = nvc0_bind_sampler_states;

   pipe->create_sampler_view = nvc0_create_sampler_view;
   pipe->sampler_view_destroy = nvc0_sampler_view_destroy;
   pipe->set_sampler_views = nvc0_set_sampler_views;

   pipe->create_vs_state = nvc0_vp_state_create;
   pipe->create_fs_state = nvc0_fp_state_create;
   pipe->create_gs_state = nvc0_gp_state_create;
   pipe->create_tcs_state = nvc0_tcp_state_create;
   pipe->create_tes_state = nvc0_tep_state_create;
   pipe->bind_vs_state = nvc0_vp_state_bind;
   pipe->bind_fs_state = nvc0_fp_state_bind;
   pipe->bind_gs_state = nvc0_gp_state_bind;
   pipe->bind_tcs_state = nvc0_tcp_state_bind;
   pipe->bind_tes_state = nvc0_tep_state_bind;
   pipe->delete_vs_state = nvc0_sp_state_delete;
   pipe->delete_fs_state = nvc0_sp_state_delete;
   pipe->delete_gs_state = nvc0_sp_state_delete;
   pipe->delete_tcs_state = nvc0_sp_state_delete;
   pipe->delete_tes_state = nvc0_sp_state_delete;

   pipe->create_compute_state = nvc0_cp_state_create;
   pipe->bind_compute_state = nvc0_cp_state_bind;
   pipe->get_compute_state_info = nvc0_get_compute_state_info;
   pipe->delete_compute_state = nvc0_sp_state_delete;

   pipe->set_blend_color = nvc0_set_blend_color;
   pipe->set_stencil_ref = nvc0_set_stencil_ref;
   pipe->set_clip_state = nvc0_set_clip_state;
   pipe->set_sample_mask = nvc0_set_sample_mask;
   pipe->set_min_samples = nvc0_set_min_samples;
   pipe->set_constant_buffer = nvc0_set_constant_buffer;
   pipe->set_framebuffer_state = nvc0_set_framebuffer_state;
   pipe->set_sample_locations = nvc0_set_sample_locations;
   pipe->set_polygon_stipple = nvc0_set_polygon_stipple;
   pipe->set_scissor_states = nvc0_set_scissor_states;
   pipe->set_viewport_states = nvc0_set_viewport_states;
   pipe->set_window_rectangles = nvc0_set_window_rectangles;
   pipe->set_tess_state = nvc0_set_tess_state;
   pipe->set_patch_vertices = nvc0_set_patch_vertices;

   pipe->create_vertex_elements_state = nvc0_vertex_state_create;
   pipe->delete_vertex_elements_state = nvc0_vertex_state_delete;
   pipe->bind_vertex_elements_state = nvc0_vertex_state_bind;

   pipe->set_vertex_buffers = nvc0_set_vertex_buffers;

   pipe->create_stream_output_target = nvc0_so_target_create;
   pipe->stream_output_target_destroy = nvc0_so_target_destroy;
   pipe->set_stream_output_targets = nvc0_set_transform_feedback_targets;

   pipe->set_global_binding = nvc0_set_global_bindings;
   pipe->set_compute_resources = nvc0_set_compute_resources;
   pipe->set_shader_images = nvc0_set_shader_images;
   pipe->set_shader_buffers = nvc0_set_shader_buffers;

   nvc0->sample_mask = ~0;
   nvc0->min_samples = 1;
   nvc0->default_tess_outer[0] =
   nvc0->default_tess_outer[1] =
   nvc0->default_tess_outer[2] =
   nvc0->default_tess_outer[3] = 1.0;
   nvc0->default_tess_inner[0] =
   nvc0->default_tess_inner[1] = 1.0;
}
