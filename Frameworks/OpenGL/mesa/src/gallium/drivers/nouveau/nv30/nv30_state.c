/*
 * Copyright 2012 Red Hat Inc.
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
 *
 * Authors: Ben Skeggs
 *
 */

#include "util/format/u_format.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"

#include "nouveau_gldefs.h"
#include "nv_object.xml.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_winsys.h"

#define NV40_3D_MRT_BLEND_ENABLE 0x0000036c

static void *
nv30_blend_state_create(struct pipe_context *pipe,
                        const struct pipe_blend_state *cso)
{
   struct nouveau_object *eng3d = nv30_context(pipe)->screen->eng3d;
   struct nv30_blend_stateobj *so;
   uint32_t blend[2], cmask[2];
   int i;

   so = CALLOC_STRUCT(nv30_blend_stateobj);
   if (!so)
      return NULL;
   so->pipe = *cso;

   if (cso->logicop_enable) {
      SB_MTHD30(so, COLOR_LOGIC_OP_ENABLE, 2);
      SB_DATA  (so, 1);
      SB_DATA  (so, nvgl_logicop_func(cso->logicop_func));
   } else {
      SB_MTHD30(so, COLOR_LOGIC_OP_ENABLE, 1);
      SB_DATA  (so, 0);
   }

   SB_MTHD30(so, DITHER_ENABLE, 1);
   SB_DATA  (so, cso->dither);

   blend[0] = cso->rt[0].blend_enable;
   cmask[0] = !!(cso->rt[0].colormask & PIPE_MASK_A) << 24 |
              !!(cso->rt[0].colormask & PIPE_MASK_R) << 16 |
              !!(cso->rt[0].colormask & PIPE_MASK_G) <<  8 |
              !!(cso->rt[0].colormask & PIPE_MASK_B);
   if (cso->independent_blend_enable) {
      blend[1] = 0;
      cmask[1] = 0;
      for (i = 1; i < 4; i++) {
         blend[1] |= cso->rt[i].blend_enable << i;
         cmask[1] |= !!(cso->rt[i].colormask & PIPE_MASK_A) << (0 + (i * 4)) |
                     !!(cso->rt[i].colormask & PIPE_MASK_R) << (1 + (i * 4)) |
                     !!(cso->rt[i].colormask & PIPE_MASK_G) << (2 + (i * 4)) |
                     !!(cso->rt[i].colormask & PIPE_MASK_B) << (3 + (i * 4));
      }
   } else {
      blend[1]  = 0x0000000e *   (blend[0] & 0x00000001);
      cmask[1]  = 0x00001110 * !!(cmask[0] & 0x01000000);
      cmask[1] |= 0x00002220 * !!(cmask[0] & 0x00010000);
      cmask[1] |= 0x00004440 * !!(cmask[0] & 0x00000100);
      cmask[1] |= 0x00008880 * !!(cmask[0] & 0x00000001);
   }

   if (eng3d->oclass >= NV40_3D_CLASS) {
      SB_MTHD40(so, MRT_BLEND_ENABLE, 2);
      SB_DATA  (so, blend[1]);
      SB_DATA  (so, cmask[1]);
   }

   if (blend[0] || blend[1]) {
      SB_MTHD30(so, BLEND_FUNC_ENABLE, 3);
      SB_DATA  (so, blend[0]);
      SB_DATA  (so, (nvgl_blend_func(cso->rt[0].alpha_src_factor) << 16) |
                     nvgl_blend_func(cso->rt[0].rgb_src_factor));
      SB_DATA  (so, (nvgl_blend_func(cso->rt[0].alpha_dst_factor) << 16) |
                     nvgl_blend_func(cso->rt[0].rgb_dst_factor));
      if (eng3d->oclass < NV40_3D_CLASS) {
         SB_MTHD30(so, BLEND_EQUATION, 1);
         SB_DATA  (so, nvgl_blend_eqn(cso->rt[0].rgb_func));
      } else {
         SB_MTHD40(so, BLEND_EQUATION, 1);
         SB_DATA  (so, (nvgl_blend_eqn(cso->rt[0].alpha_func) << 16) |
                        nvgl_blend_eqn(cso->rt[0].rgb_func));
      }
   } else {
      SB_MTHD30(so, BLEND_FUNC_ENABLE, 1);
      SB_DATA  (so, blend[0]);
   }

   SB_MTHD30(so, COLOR_MASK, 1);
   SB_DATA  (so, cmask[0]);
   return so;
}

static void
nv30_blend_state_bind(struct pipe_context *pipe, void *hwcso)
{
   struct nv30_context *nv30 = nv30_context(pipe);

   nv30->blend = hwcso;
   nv30->dirty |= NV30_NEW_BLEND;
}

static void
nv30_blend_state_delete(struct pipe_context *pipe, void *hwcso)
{
   FREE(hwcso);
}

static void *
nv30_rasterizer_state_create(struct pipe_context *pipe,
                             const struct pipe_rasterizer_state *cso)
{
   struct nv30_rasterizer_stateobj *so;

   so = CALLOC_STRUCT(nv30_rasterizer_stateobj);
   if (!so)
      return NULL;
   so->pipe = *cso;

   SB_MTHD30(so, SHADE_MODEL, 1);
   SB_DATA  (so, cso->flatshade ? NV30_3D_SHADE_MODEL_FLAT :
                                  NV30_3D_SHADE_MODEL_SMOOTH);

   SB_MTHD30(so, POLYGON_MODE_FRONT, 6);
   SB_DATA  (so, nvgl_polygon_mode(cso->fill_front));
   SB_DATA  (so, nvgl_polygon_mode(cso->fill_back));
   if (cso->cull_face == PIPE_FACE_FRONT_AND_BACK)
      SB_DATA  (so, NV30_3D_CULL_FACE_FRONT_AND_BACK);
   else
   if (cso->cull_face == PIPE_FACE_FRONT)
      SB_DATA  (so, NV30_3D_CULL_FACE_FRONT);
   else
      SB_DATA  (so, NV30_3D_CULL_FACE_BACK);
   SB_DATA  (so, cso->front_ccw ? NV30_3D_FRONT_FACE_CCW :
                                  NV30_3D_FRONT_FACE_CW);
   SB_DATA  (so, cso->poly_smooth);
   SB_DATA  (so, cso->cull_face != PIPE_FACE_NONE);

   SB_MTHD30(so, POLYGON_OFFSET_POINT_ENABLE, 3);
   SB_DATA  (so, cso->offset_point);
   SB_DATA  (so, cso->offset_line);
   SB_DATA  (so, cso->offset_tri);
   if (cso->offset_point || cso->offset_line || cso->offset_tri) {
      SB_MTHD30(so, POLYGON_OFFSET_FACTOR, 2);
      SB_DATA  (so, fui(cso->offset_scale));
      SB_DATA  (so, fui(cso->offset_units * 2.0));
   }

   SB_MTHD30(so, LINE_WIDTH, 2);
   SB_DATA  (so, (unsigned char)(cso->line_width * 8.0) & 0xff);
   SB_DATA  (so, cso->line_smooth);
   SB_MTHD30(so, LINE_STIPPLE_ENABLE, 2);
   SB_DATA  (so, cso->line_stipple_enable);
   SB_DATA  (so, (cso->line_stipple_pattern << 16) |
                  cso->line_stipple_factor);

   SB_MTHD30(so, VERTEX_TWO_SIDE_ENABLE, 1);
   SB_DATA  (so, cso->light_twoside);
   SB_MTHD30(so, POLYGON_STIPPLE_ENABLE, 1);
   SB_DATA  (so, cso->poly_stipple_enable);
   SB_MTHD30(so, POINT_SIZE, 1);
   SB_DATA  (so, fui(cso->point_size));
   SB_MTHD30(so, FLATSHADE_FIRST, 1);
   SB_DATA  (so, cso->flatshade_first);

   SB_MTHD30(so, DEPTH_CONTROL, 1);
   SB_DATA  (so, cso->depth_clip_near ? 0x00000001 : 0x00000010);
   return so;
}

static void
nv30_rasterizer_state_bind(struct pipe_context *pipe, void *hwcso)
{
   struct nv30_context *nv30 = nv30_context(pipe);

   nv30->rast = hwcso;
   nv30->dirty |= NV30_NEW_RASTERIZER;
}

static void
nv30_rasterizer_state_delete(struct pipe_context *pipe, void *hwcso)
{
   FREE(hwcso);
}

static void *
nv30_zsa_state_create(struct pipe_context *pipe,
                      const struct pipe_depth_stencil_alpha_state *cso)
{
   struct nouveau_object *eng3d = nv30_context(pipe)->screen->eng3d;
   struct nv30_zsa_stateobj *so;

   so = CALLOC_STRUCT(nv30_zsa_stateobj);
   if (!so)
      return NULL;
   so->pipe = *cso;

   SB_MTHD30(so, DEPTH_FUNC, 3);
   SB_DATA  (so, nvgl_comparison_op(cso->depth_func));
   SB_DATA  (so, cso->depth_writemask);
   SB_DATA  (so, cso->depth_enabled);

   if (eng3d->oclass == NV35_3D_CLASS || eng3d->oclass >= NV40_3D_CLASS) {
      SB_MTHD35(so, DEPTH_BOUNDS_TEST_ENABLE, 3);
      SB_DATA  (so, cso->depth_bounds_test);
      SB_DATA  (so, fui(cso->depth_bounds_min));
      SB_DATA  (so, fui(cso->depth_bounds_max));
   }

   if (cso->stencil[0].enabled) {
      SB_MTHD30(so, STENCIL_ENABLE(0), 3);
      SB_DATA  (so, 1);
      SB_DATA  (so, cso->stencil[0].writemask);
      SB_DATA  (so, nvgl_comparison_op(cso->stencil[0].func));
      SB_MTHD30(so, STENCIL_FUNC_MASK(0), 4);
      SB_DATA  (so, cso->stencil[0].valuemask);
      SB_DATA  (so, nvgl_stencil_op(cso->stencil[0].fail_op));
      SB_DATA  (so, nvgl_stencil_op(cso->stencil[0].zfail_op));
      SB_DATA  (so, nvgl_stencil_op(cso->stencil[0].zpass_op));
   } else {
      SB_MTHD30(so, STENCIL_ENABLE(0), 2);
      SB_DATA  (so, 0);
      SB_DATA  (so, 0x000000ff);
   }

   if (cso->stencil[1].enabled) {
      SB_MTHD30(so, STENCIL_ENABLE(1), 3);
      SB_DATA  (so, 1);
      SB_DATA  (so, cso->stencil[1].writemask);
      SB_DATA  (so, nvgl_comparison_op(cso->stencil[1].func));
      SB_MTHD30(so, STENCIL_FUNC_MASK(1), 4);
      SB_DATA  (so, cso->stencil[1].valuemask);
      SB_DATA  (so, nvgl_stencil_op(cso->stencil[1].fail_op));
      SB_DATA  (so, nvgl_stencil_op(cso->stencil[1].zfail_op));
      SB_DATA  (so, nvgl_stencil_op(cso->stencil[1].zpass_op));
   } else {
      SB_MTHD30(so, STENCIL_ENABLE(1), 1);
      SB_DATA  (so, 0);
   }

   SB_MTHD30(so, ALPHA_FUNC_ENABLE, 3);
   SB_DATA  (so, cso->alpha_enabled ? 1 : 0);
   SB_DATA  (so, nvgl_comparison_op(cso->alpha_func));
   SB_DATA  (so, float_to_ubyte(cso->alpha_ref_value));

   return so;
}

static void
nv30_zsa_state_bind(struct pipe_context *pipe, void *hwcso)
{
   struct nv30_context *nv30 = nv30_context(pipe);

   nv30->zsa = hwcso;
   nv30->dirty |= NV30_NEW_ZSA;
}

static void
nv30_zsa_state_delete(struct pipe_context *pipe, void *hwcso)
{
   FREE(hwcso);
}

static void
nv30_set_blend_color(struct pipe_context *pipe,
                     const struct pipe_blend_color *bcol)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    nv30->blend_colour = *bcol;
    nv30->dirty |= NV30_NEW_BLEND_COLOUR;
}

static void
nv30_set_stencil_ref(struct pipe_context *pipe,
                     const struct pipe_stencil_ref sr)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    nv30->stencil_ref = sr;
    nv30->dirty |= NV30_NEW_STENCIL_REF;
}

static void
nv30_set_clip_state(struct pipe_context *pipe,
                    const struct pipe_clip_state *clip)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    memcpy(nv30->clip.ucp, clip->ucp, sizeof(clip->ucp));

    nv30->dirty |= NV30_NEW_CLIP;
}

static void
nv30_set_sample_mask(struct pipe_context *pipe, unsigned sample_mask)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    nv30->sample_mask = sample_mask;
    nv30->dirty |= NV30_NEW_SAMPLE_MASK;
}

static void
nv30_set_constant_buffer(struct pipe_context *pipe,
                         enum pipe_shader_type shader, uint index,
                         bool pass_reference,
                         const struct pipe_constant_buffer *cb)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   struct pipe_resource *buf = cb ? cb->buffer : NULL;
   unsigned size;

   if (cb && cb->user_buffer) {
      buf = nouveau_user_buffer_create(pipe->screen, (void*)cb->user_buffer,
                                       cb->buffer_size,
                                       PIPE_BIND_CONSTANT_BUFFER);
   }

   size = 0;
   if (buf)
      size = buf->width0 / (4 * sizeof(float));

   if (shader == PIPE_SHADER_VERTEX) {
      if (pass_reference) {
         pipe_resource_reference(&nv30->vertprog.constbuf, NULL);
         nv30->vertprog.constbuf = buf;
      } else {
         pipe_resource_reference(&nv30->vertprog.constbuf, buf);
      }
      nv30->vertprog.constbuf_nr = size;
      nv30->dirty |= NV30_NEW_VERTCONST;
   } else
   if (shader == PIPE_SHADER_FRAGMENT) {
      if (pass_reference) {
         pipe_resource_reference(&nv30->fragprog.constbuf, NULL);
         nv30->fragprog.constbuf = buf;
      } else {
         pipe_resource_reference(&nv30->fragprog.constbuf, buf);
      }
      nv30->fragprog.constbuf_nr = size;
      nv30->dirty |= NV30_NEW_FRAGCONST;
   }

   if (cb && cb->user_buffer) {
      pipe_resource_reference(&buf, NULL);
   }
}

static void
nv30_set_framebuffer_state(struct pipe_context *pipe,
                           const struct pipe_framebuffer_state *fb)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    nouveau_bufctx_reset(nv30->bufctx, BUFCTX_FB);

    nv30->framebuffer = *fb;
    nv30->dirty |= NV30_NEW_FRAMEBUFFER;

   /* Hardware can't handle different swizzled-ness or different blocksizes
    * for zs and cbufs. If both are supplied and something doesn't match,
    * blank out the zs for now so that at least *some* rendering can occur.
    */
    if (fb->nr_cbufs > 0 && fb->zsbuf) {
       struct nv30_miptree *color_mt = nv30_miptree(fb->cbufs[0]->texture);
       struct nv30_miptree *zeta_mt = nv30_miptree(fb->zsbuf->texture);

       if (color_mt->swizzled != zeta_mt->swizzled ||
           (color_mt->swizzled &&
            (util_format_get_blocksize(fb->zsbuf->format) > 2) !=
            (util_format_get_blocksize(fb->cbufs[0]->format) > 2))) {
          nv30->framebuffer.zsbuf = NULL;
          debug_printf("Mismatched color and zeta formats, ignoring zeta.\n");
       }
    }
}

static void
nv30_set_polygon_stipple(struct pipe_context *pipe,
                         const struct pipe_poly_stipple *stipple)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    nv30->stipple = *stipple;
    nv30->dirty |= NV30_NEW_STIPPLE;
}

static void
nv30_set_scissor_states(struct pipe_context *pipe,
                        unsigned start_slot,
                        unsigned num_viewports,
                        const struct pipe_scissor_state *scissor)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    nv30->scissor = *scissor;
    nv30->dirty |= NV30_NEW_SCISSOR;
}

static void
nv30_set_viewport_states(struct pipe_context *pipe,
                         unsigned start_slot,
                         unsigned num_viewports,
                         const struct pipe_viewport_state *vpt)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    nv30->viewport = *vpt;
    nv30->dirty |= NV30_NEW_VIEWPORT;
}

static void
nv30_set_vertex_buffers(struct pipe_context *pipe,
                        unsigned count,
                        unsigned unbind_num_trailing_slots,
                        bool take_ownership,
                        const struct pipe_vertex_buffer *vb)
{
    struct nv30_context *nv30 = nv30_context(pipe);

    nouveau_bufctx_reset(nv30->bufctx, BUFCTX_VTXBUF);

    util_set_vertex_buffers_count(nv30->vtxbuf, &nv30->num_vtxbufs,
                                  vb, count,
                                  unbind_num_trailing_slots,
                                  take_ownership);

    nv30->dirty |= NV30_NEW_ARRAYS;
}

void
nv30_state_init(struct pipe_context *pipe)
{
   pipe->create_blend_state = nv30_blend_state_create;
   pipe->bind_blend_state = nv30_blend_state_bind;
   pipe->delete_blend_state = nv30_blend_state_delete;

   pipe->create_rasterizer_state = nv30_rasterizer_state_create;
   pipe->bind_rasterizer_state = nv30_rasterizer_state_bind;
   pipe->delete_rasterizer_state = nv30_rasterizer_state_delete;

   pipe->create_depth_stencil_alpha_state = nv30_zsa_state_create;
   pipe->bind_depth_stencil_alpha_state = nv30_zsa_state_bind;
   pipe->delete_depth_stencil_alpha_state = nv30_zsa_state_delete;

   pipe->set_blend_color = nv30_set_blend_color;
   pipe->set_stencil_ref = nv30_set_stencil_ref;
   pipe->set_clip_state = nv30_set_clip_state;
   pipe->set_sample_mask = nv30_set_sample_mask;
   pipe->set_constant_buffer = nv30_set_constant_buffer;
   pipe->set_framebuffer_state = nv30_set_framebuffer_state;
   pipe->set_polygon_stipple = nv30_set_polygon_stipple;
   pipe->set_scissor_states = nv30_set_scissor_states;
   pipe->set_viewport_states = nv30_set_viewport_states;

   pipe->set_vertex_buffers = nv30_set_vertex_buffers;
}
