/*
 * Copyright (c) 2011-2013 Luc Verhaegen <libv@skynet.be>
 * Copyright (c) 2017-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_helpers.h"
#include "util/u_debug.h"
#include "util/u_framebuffer.h"
#include "util/u_viewport.h"

#include "pipe/p_state.h"

#include "lima_screen.h"
#include "lima_context.h"
#include "lima_format.h"
#include "lima_resource.h"

static void
lima_set_framebuffer_state(struct pipe_context *pctx,
                           const struct pipe_framebuffer_state *framebuffer)
{
   struct lima_context *ctx = lima_context(pctx);

   /* make sure there are always single job in this context */
   if (lima_debug & LIMA_DEBUG_SINGLE_JOB)
      lima_flush(ctx);

   struct lima_context_framebuffer *fb = &ctx->framebuffer;

   util_copy_framebuffer_state(&fb->base, framebuffer);

   ctx->job = NULL;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_FRAMEBUFFER;
}

static void
lima_set_polygon_stipple(struct pipe_context *pctx,
                         const struct pipe_poly_stipple *stipple)
{

}

static void *
lima_create_depth_stencil_alpha_state(struct pipe_context *pctx,
                                      const struct pipe_depth_stencil_alpha_state *cso)
{
   struct lima_depth_stencil_alpha_state *so;

   so = CALLOC_STRUCT(lima_depth_stencil_alpha_state);
   if (!so)
      return NULL;

   so->base = *cso;

   return so;
}

static void
lima_bind_depth_stencil_alpha_state(struct pipe_context *pctx, void *hwcso)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->zsa = hwcso;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_ZSA;
}

static void
lima_delete_depth_stencil_alpha_state(struct pipe_context *pctx, void *hwcso)
{
   FREE(hwcso);
}

static void *
lima_create_rasterizer_state(struct pipe_context *pctx,
                             const struct pipe_rasterizer_state *cso)
{
   struct lima_rasterizer_state *so;

   so = CALLOC_STRUCT(lima_rasterizer_state);
   if (!so)
      return NULL;

   so->base = *cso;

   return so;
}

static void
lima_bind_rasterizer_state(struct pipe_context *pctx, void *hwcso)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->rasterizer = hwcso;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_RASTERIZER;
}

static void
lima_delete_rasterizer_state(struct pipe_context *pctx, void *hwcso)
{
   FREE(hwcso);
}

static void *
lima_create_blend_state(struct pipe_context *pctx,
                        const struct pipe_blend_state *cso)
{
   struct lima_blend_state *so;

   so = CALLOC_STRUCT(lima_blend_state);
   if (!so)
      return NULL;

   so->base = *cso;

   return so;
}

static void
lima_bind_blend_state(struct pipe_context *pctx, void *hwcso)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->blend = hwcso;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_BLEND;
}

static void
lima_delete_blend_state(struct pipe_context *pctx, void *hwcso)
{
   FREE(hwcso);
}

static void *
lima_create_vertex_elements_state(struct pipe_context *pctx, unsigned num_elements,
                                  const struct pipe_vertex_element *elements)
{
   struct lima_vertex_element_state *so;

   so = CALLOC_STRUCT(lima_vertex_element_state);
   if (!so)
      return NULL;

   memcpy(so->pipe, elements, sizeof(*elements) * num_elements);
   so->num_elements = num_elements;

   return so;
}

static void
lima_bind_vertex_elements_state(struct pipe_context *pctx, void *hwcso)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->vertex_elements = hwcso;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_VERTEX_ELEM;
}

static void
lima_delete_vertex_elements_state(struct pipe_context *pctx, void *hwcso)
{
   FREE(hwcso);
}

static void
lima_set_vertex_buffers(struct pipe_context *pctx,
                        unsigned count,
                        unsigned unbind_num_trailing_slots,
                        bool take_ownership,
                        const struct pipe_vertex_buffer *vb)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_context_vertex_buffer *so = &ctx->vertex_buffers;

   util_set_vertex_buffers_mask(so->vb, &so->enabled_mask,
                                vb, count,
                                unbind_num_trailing_slots,
                                take_ownership);
   so->count = util_last_bit(so->enabled_mask);

   ctx->dirty |= LIMA_CONTEXT_DIRTY_VERTEX_BUFF;
}

static void
lima_set_viewport_states(struct pipe_context *pctx,
                         unsigned start_slot,
                         unsigned num_viewports,
                         const struct pipe_viewport_state *viewport)
{
   struct lima_context *ctx = lima_context(pctx);

   /* reverse calculate the parameter of glViewport */
   ctx->viewport.left = ctx->ext_viewport.left =
      viewport->translate[0] - fabsf(viewport->scale[0]);
   ctx->viewport.right = ctx->ext_viewport.right =
      viewport->translate[0] + fabsf(viewport->scale[0]);
   ctx->viewport.bottom = ctx->ext_viewport.bottom =
      viewport->translate[1] - fabsf(viewport->scale[1]);
   ctx->viewport.top = ctx->ext_viewport.top =
      viewport->translate[1] + fabsf(viewport->scale[1]);

   /* reverse calculate the parameter of glDepthRange */
   float near, far;
   bool halfz = ctx->rasterizer && ctx->rasterizer->base.clip_halfz;
   util_viewport_zmin_zmax(viewport, halfz, &near, &far);

   ctx->viewport.near = ctx->rasterizer && ctx->rasterizer->base.depth_clip_near ? near : 0.0f;
   ctx->viewport.far = ctx->rasterizer && ctx->rasterizer->base.depth_clip_far ? far : 1.0f;

   ctx->viewport.transform = *viewport;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_VIEWPORT;
}

static void
lima_set_scissor_states(struct pipe_context *pctx,
                        unsigned start_slot,
                        unsigned num_scissors,
                        const struct pipe_scissor_state *scissor)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->scissor = *scissor;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_SCISSOR;
}

static void
lima_set_blend_color(struct pipe_context *pctx,
                     const struct pipe_blend_color *blend_color)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->blend_color = *blend_color;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_BLEND_COLOR;
}

static void
lima_set_stencil_ref(struct pipe_context *pctx,
                     const struct pipe_stencil_ref stencil_ref)
{
   struct lima_context *ctx = lima_context(pctx);

   ctx->stencil_ref = stencil_ref;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_STENCIL_REF;
}

static void
lima_set_clip_state(struct pipe_context *pctx,
                    const struct pipe_clip_state *clip)
{
   struct lima_context *ctx = lima_context(pctx);
   ctx->clip = *clip;

   ctx->dirty |= LIMA_CONTEXT_DIRTY_CLIP;
}

static void
lima_set_constant_buffer(struct pipe_context *pctx,
                         enum pipe_shader_type shader, uint index,
                         bool pass_reference,
                         const struct pipe_constant_buffer *cb)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_context_constant_buffer *so = ctx->const_buffer + shader;

   assert(index == 0);

   if (unlikely(!cb)) {
      so->buffer = NULL;
      so->size = 0;
   } else {
      assert(!cb->buffer);

      so->buffer = cb->user_buffer + cb->buffer_offset;
      so->size = cb->buffer_size;
   }

   so->dirty = true;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_CONST_BUFF;

}

static void *
lima_create_sampler_state(struct pipe_context *pctx,
                         const struct pipe_sampler_state *cso)
{
   struct lima_sampler_state *so = CALLOC_STRUCT(lima_sampler_state);
   if (!so)
      return NULL;

   memcpy(so, cso, sizeof(*cso));

   return so;
}

static void
lima_sampler_state_delete(struct pipe_context *pctx, void *sstate)
{
   free(sstate);
}

static void
lima_sampler_states_bind(struct pipe_context *pctx,
                        enum pipe_shader_type shader, unsigned start,
                        unsigned nr, void **hwcso)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_texture_stateobj *lima_tex = &ctx->tex_stateobj;
   unsigned i;
   unsigned new_nr = 0;

   assert(start == 0);

   for (i = 0; i < nr; i++) {
      if (hwcso[i])
         new_nr = i + 1;
      lima_tex->samplers[i] = hwcso[i];
   }

   for (; i < lima_tex->num_samplers; i++) {
      lima_tex->samplers[i] = NULL;
   }

   lima_tex->num_samplers = new_nr;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_TEXTURES;
}

static struct pipe_sampler_view *
lima_create_sampler_view(struct pipe_context *pctx, struct pipe_resource *prsc,
                        const struct pipe_sampler_view *cso)
{
   struct lima_sampler_view *so = CALLOC_STRUCT(lima_sampler_view);

   if (!so)
      return NULL;

   so->base = *cso;

   pipe_reference(NULL, &prsc->reference);
   so->base.texture = prsc;
   so->base.reference.count = 1;
   so->base.context = pctx;

   uint8_t sampler_swizzle[4] = { cso->swizzle_r, cso->swizzle_g,
                                  cso->swizzle_b, cso->swizzle_a };
   const uint8_t *format_swizzle = lima_format_get_texel_swizzle(cso->format);
   util_format_compose_swizzles(format_swizzle, sampler_swizzle, so->swizzle);

   return &so->base;
}

static void
lima_sampler_view_destroy(struct pipe_context *pctx,
                         struct pipe_sampler_view *pview)
{
   struct lima_sampler_view *view = lima_sampler_view(pview);

   pipe_resource_reference(&pview->texture, NULL);

   free(view);
}

static void
lima_set_sampler_views(struct pipe_context *pctx,
                      enum pipe_shader_type shader,
                      unsigned start, unsigned nr,
                       unsigned unbind_num_trailing_slots,
                       bool take_ownership,
                      struct pipe_sampler_view **views)
{
   struct lima_context *ctx = lima_context(pctx);
   struct lima_texture_stateobj *lima_tex = &ctx->tex_stateobj;
   int i;
   unsigned new_nr = 0;

   assert(start == 0);

   for (i = 0; i < nr; i++) {
      if (views[i])
         new_nr = i + 1;

      if (take_ownership) {
         pipe_sampler_view_reference(&lima_tex->textures[i], NULL);
         lima_tex->textures[i] = views[i];
      } else {
         pipe_sampler_view_reference(&lima_tex->textures[i], views[i]);
      }
   }

   for (; i < lima_tex->num_textures; i++) {
      pipe_sampler_view_reference(&lima_tex->textures[i], NULL);
   }

   lima_tex->num_textures = new_nr;
   ctx->dirty |= LIMA_CONTEXT_DIRTY_TEXTURES;
}

static void
lima_set_sample_mask(struct pipe_context *pctx,
                     unsigned sample_mask)
{
   struct lima_context *ctx = lima_context(pctx);
   ctx->sample_mask = sample_mask & ((1 << LIMA_MAX_SAMPLES) - 1);
   ctx->dirty |= LIMA_CONTEXT_DIRTY_SAMPLE_MASK;
}

void
lima_state_init(struct lima_context *ctx)
{
   ctx->base.set_framebuffer_state = lima_set_framebuffer_state;
   ctx->base.set_polygon_stipple = lima_set_polygon_stipple;
   ctx->base.set_viewport_states = lima_set_viewport_states;
   ctx->base.set_scissor_states = lima_set_scissor_states;
   ctx->base.set_blend_color = lima_set_blend_color;
   ctx->base.set_stencil_ref = lima_set_stencil_ref;
   ctx->base.set_clip_state = lima_set_clip_state;

   ctx->base.set_vertex_buffers = lima_set_vertex_buffers;
   ctx->base.set_constant_buffer = lima_set_constant_buffer;

   ctx->base.create_depth_stencil_alpha_state = lima_create_depth_stencil_alpha_state;
   ctx->base.bind_depth_stencil_alpha_state = lima_bind_depth_stencil_alpha_state;
   ctx->base.delete_depth_stencil_alpha_state = lima_delete_depth_stencil_alpha_state;

   ctx->base.create_rasterizer_state = lima_create_rasterizer_state;
   ctx->base.bind_rasterizer_state = lima_bind_rasterizer_state;
   ctx->base.delete_rasterizer_state = lima_delete_rasterizer_state;

   ctx->base.create_blend_state = lima_create_blend_state;
   ctx->base.bind_blend_state = lima_bind_blend_state;
   ctx->base.delete_blend_state = lima_delete_blend_state;

   ctx->base.create_vertex_elements_state = lima_create_vertex_elements_state;
   ctx->base.bind_vertex_elements_state = lima_bind_vertex_elements_state;
   ctx->base.delete_vertex_elements_state = lima_delete_vertex_elements_state;

   ctx->base.create_sampler_state = lima_create_sampler_state;
   ctx->base.delete_sampler_state = lima_sampler_state_delete;
   ctx->base.bind_sampler_states = lima_sampler_states_bind;

   ctx->base.create_sampler_view = lima_create_sampler_view;
   ctx->base.sampler_view_destroy = lima_sampler_view_destroy;
   ctx->base.set_sampler_views = lima_set_sampler_views;

   ctx->base.set_sample_mask = lima_set_sample_mask;
}

void
lima_state_fini(struct lima_context *ctx)
{
   struct lima_context_vertex_buffer *so = &ctx->vertex_buffers;

   util_set_vertex_buffers_mask(so->vb, &so->enabled_mask, NULL,
                                0, ARRAY_SIZE(so->vb), false);
}
