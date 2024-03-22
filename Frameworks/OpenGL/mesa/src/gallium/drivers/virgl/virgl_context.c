/*
 * Copyright 2014, 2015 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <libsync.h>
#include "pipe/p_shader_tokens.h"

#include "compiler/nir/nir.h"
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#include "nir/nir_to_tgsi.h"
#include "util/format/u_format.h"
#include "indices/u_primconvert.h"
#include "util/u_draw.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_surface.h"
#include "util/u_transfer.h"
#include "util/u_helpers.h"
#include "util/slab.h"
#include "util/u_upload_mgr.h"
#include "util/u_blitter.h"

#include "virgl_encode.h"
#include "virgl_context.h"
#include "virtio-gpu/virgl_protocol.h"
#include "virgl_resource.h"
#include "virgl_screen.h"
#include "virgl_staging_mgr.h"
#include "virgl_video.h"

static uint32_t next_handle;
uint32_t virgl_object_assign_handle(void)
{
   return p_atomic_inc_return(&next_handle);
}

bool
virgl_can_rebind_resource(struct virgl_context *vctx,
                          struct pipe_resource *res)
{
   /* We cannot rebind resources that are referenced by host objects, which
    * are
    *
    *  - VIRGL_OBJECT_SURFACE
    *  - VIRGL_OBJECT_SAMPLER_VIEW
    *  - VIRGL_OBJECT_STREAMOUT_TARGET
    *
    * Because surfaces cannot be created from buffers, we require the resource
    * to be a buffer instead (and avoid tracking VIRGL_OBJECT_SURFACE binds).
    */
   const unsigned unsupported_bind = (PIPE_BIND_SAMPLER_VIEW |
                                      PIPE_BIND_STREAM_OUTPUT);
   const unsigned bind_history = virgl_resource(res)->bind_history;
   return res->target == PIPE_BUFFER && !(bind_history & unsupported_bind);
}

void
virgl_rebind_resource(struct virgl_context *vctx,
                      struct pipe_resource *res)
{
   /* Queries use internally created buffers and do not go through transfers.
    * Index buffers are not bindable.  They are not tracked.
    */
   ASSERTED const unsigned tracked_bind = (PIPE_BIND_VERTEX_BUFFER |
                                               PIPE_BIND_CONSTANT_BUFFER |
                                               PIPE_BIND_SHADER_BUFFER |
                                               PIPE_BIND_SHADER_IMAGE);
   const unsigned bind_history = virgl_resource(res)->bind_history;
   unsigned i;

   assert(virgl_can_rebind_resource(vctx, res) &&
          (bind_history & tracked_bind) == bind_history);

   if (bind_history & PIPE_BIND_VERTEX_BUFFER) {
      for (i = 0; i < vctx->num_vertex_buffers; i++) {
         if (vctx->vertex_buffer[i].buffer.resource == res) {
            vctx->vertex_array_dirty = true;
            break;
         }
      }
   }

   if (bind_history & PIPE_BIND_SHADER_BUFFER) {
      uint32_t remaining_mask = vctx->atomic_buffer_enabled_mask;
      while (remaining_mask) {
         int i = u_bit_scan(&remaining_mask);
         if (vctx->atomic_buffers[i].buffer == res) {
            const struct pipe_shader_buffer *abo = &vctx->atomic_buffers[i];
            virgl_encode_set_hw_atomic_buffers(vctx, i, 1, abo);
         }
      }
   }

   /* check per-stage shader bindings */
   if (bind_history & (PIPE_BIND_CONSTANT_BUFFER |
                       PIPE_BIND_SHADER_BUFFER |
                       PIPE_BIND_SHADER_IMAGE)) {
      enum pipe_shader_type shader_type;
      for (shader_type = 0; shader_type < PIPE_SHADER_TYPES; shader_type++) {
         const struct virgl_shader_binding_state *binding =
            &vctx->shader_bindings[shader_type];

         if (bind_history & PIPE_BIND_CONSTANT_BUFFER) {
            uint32_t remaining_mask = binding->ubo_enabled_mask;
            while (remaining_mask) {
               int i = u_bit_scan(&remaining_mask);
               if (binding->ubos[i].buffer == res) {
                  const struct pipe_constant_buffer *ubo = &binding->ubos[i];
                  virgl_encoder_set_uniform_buffer(vctx, shader_type, i,
                                                   ubo->buffer_offset,
                                                   ubo->buffer_size,
                                                   virgl_resource(res));
               }
            }
         }

         if (bind_history & PIPE_BIND_SHADER_BUFFER) {
            uint32_t remaining_mask = binding->ssbo_enabled_mask;
            while (remaining_mask) {
               int i = u_bit_scan(&remaining_mask);
               if (binding->ssbos[i].buffer == res) {
                  const struct pipe_shader_buffer *ssbo = &binding->ssbos[i];
                  virgl_encode_set_shader_buffers(vctx, shader_type, i, 1,
                                                  ssbo);
               }
            }
         }

         if (bind_history & PIPE_BIND_SHADER_IMAGE) {
            uint32_t remaining_mask = binding->image_enabled_mask;
            while (remaining_mask) {
               int i = u_bit_scan(&remaining_mask);
               if (binding->images[i].resource == res) {
                  const struct pipe_image_view *image = &binding->images[i];
                  virgl_encode_set_shader_images(vctx, shader_type, i, 1,
                                                 image);
               }
            }
         }
      }
   }
}

static void virgl_attach_res_framebuffer(struct virgl_context *vctx)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   struct pipe_surface *surf;
   struct virgl_resource *res;
   unsigned i;

   surf = vctx->framebuffer.zsbuf;
   if (surf) {
      res = virgl_resource(surf->texture);
      if (res) {
         vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
         virgl_resource_dirty(res, surf->u.tex.level);
      }
   }
   for (i = 0; i < vctx->framebuffer.nr_cbufs; i++) {
      surf = vctx->framebuffer.cbufs[i];
      if (surf) {
         res = virgl_resource(surf->texture);
         if (res) {
            vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
            virgl_resource_dirty(res, surf->u.tex.level);
         }
      }
   }
}

static void virgl_attach_res_sampler_views(struct virgl_context *vctx,
                                           enum pipe_shader_type shader_type)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   const struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader_type];

   for (int i = 0; i < PIPE_MAX_SHADER_SAMPLER_VIEWS; ++i) {
      if (binding->views[i] && binding->views[i]->texture) {
         struct virgl_resource *res = virgl_resource(binding->views[i]->texture);
         vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
      }
   }
}

static void virgl_attach_res_vertex_buffers(struct virgl_context *vctx)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   struct virgl_resource *res;
   unsigned i;

   for (i = 0; i < vctx->num_vertex_buffers; i++) {
      res = virgl_resource(vctx->vertex_buffer[i].buffer.resource);
      if (res)
         vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
   }
}

static void virgl_attach_res_index_buffer(struct virgl_context *vctx,
					  struct virgl_indexbuf *ib)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   struct virgl_resource *res;

   res = virgl_resource(ib->buffer);
   if (res)
      vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
}

static void virgl_attach_res_so_targets(struct virgl_context *vctx)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   struct virgl_resource *res;
   unsigned i;

   for (i = 0; i < vctx->num_so_targets; i++) {
      res = virgl_resource(vctx->so_targets[i].base.buffer);
      if (res)
         vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
   }
}

static void virgl_attach_res_uniform_buffers(struct virgl_context *vctx,
                                             enum pipe_shader_type shader_type)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   const struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader_type];
   uint32_t remaining_mask = binding->ubo_enabled_mask;
   struct virgl_resource *res;

   while (remaining_mask) {
      int i = u_bit_scan(&remaining_mask);
      res = virgl_resource(binding->ubos[i].buffer);
      assert(res);
      vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
   }
}

static void virgl_attach_res_shader_buffers(struct virgl_context *vctx,
                                            enum pipe_shader_type shader_type)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   const struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader_type];
   uint32_t remaining_mask = binding->ssbo_enabled_mask;
   struct virgl_resource *res;

   while (remaining_mask) {
      int i = u_bit_scan(&remaining_mask);
      res = virgl_resource(binding->ssbos[i].buffer);
      assert(res);
      vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
   }
}

static void virgl_attach_res_shader_images(struct virgl_context *vctx,
                                           enum pipe_shader_type shader_type)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   const struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader_type];
   uint32_t remaining_mask = binding->image_enabled_mask;
   struct virgl_resource *res;

   while (remaining_mask) {
      int i = u_bit_scan(&remaining_mask);
      res = virgl_resource(binding->images[i].resource);
      assert(res);
      vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
   }
}

static void virgl_attach_res_atomic_buffers(struct virgl_context *vctx)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   uint32_t remaining_mask = vctx->atomic_buffer_enabled_mask;
   struct virgl_resource *res;

   while (remaining_mask) {
      int i = u_bit_scan(&remaining_mask);
      res = virgl_resource(vctx->atomic_buffers[i].buffer);
      assert(res);
      vws->emit_res(vws, vctx->cbuf, res->hw_res, false);
   }
}

/*
 * after flushing, the hw context still has a bunch of
 * resources bound, so we need to rebind those here.
 */
static void virgl_reemit_draw_resources(struct virgl_context *vctx)
{
   enum pipe_shader_type shader_type;

   /* reattach any flushed resources */
   /* framebuffer, sampler views, vertex/index/uniform/stream buffers */
   virgl_attach_res_framebuffer(vctx);

   for (shader_type = 0; shader_type < PIPE_SHADER_COMPUTE; shader_type++) {
      virgl_attach_res_sampler_views(vctx, shader_type);
      virgl_attach_res_uniform_buffers(vctx, shader_type);
      virgl_attach_res_shader_buffers(vctx, shader_type);
      virgl_attach_res_shader_images(vctx, shader_type);
   }
   virgl_attach_res_atomic_buffers(vctx);
   virgl_attach_res_vertex_buffers(vctx);
   virgl_attach_res_so_targets(vctx);
}

static void virgl_reemit_compute_resources(struct virgl_context *vctx)
{
   virgl_attach_res_sampler_views(vctx, PIPE_SHADER_COMPUTE);
   virgl_attach_res_uniform_buffers(vctx, PIPE_SHADER_COMPUTE);
   virgl_attach_res_shader_buffers(vctx, PIPE_SHADER_COMPUTE);
   virgl_attach_res_shader_images(vctx, PIPE_SHADER_COMPUTE);

   virgl_attach_res_atomic_buffers(vctx);
}

static struct pipe_surface *virgl_create_surface(struct pipe_context *ctx,
                                                struct pipe_resource *resource,
                                                const struct pipe_surface *templ)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_surface *surf;
   struct virgl_resource *res = virgl_resource(resource);
   uint32_t handle;

   /* no support for buffer surfaces */
   if (resource->target == PIPE_BUFFER)
      return NULL;

   surf = CALLOC_STRUCT(virgl_surface);
   if (!surf)
      return NULL;

   assert(ctx->screen->get_param(ctx->screen,
                                 PIPE_CAP_DEST_SURFACE_SRGB_CONTROL) ||
          (util_format_is_srgb(templ->format) ==
           util_format_is_srgb(resource->format)));

   virgl_resource_dirty(res, 0);
   handle = virgl_object_assign_handle();
   pipe_reference_init(&surf->base.reference, 1);
   pipe_resource_reference(&surf->base.texture, resource);
   surf->base.context = ctx;
   surf->base.format = templ->format;

   surf->base.width = u_minify(resource->width0, templ->u.tex.level);
   surf->base.height = u_minify(resource->height0, templ->u.tex.level);
   surf->base.u.tex.level = templ->u.tex.level;
   surf->base.u.tex.first_layer = templ->u.tex.first_layer;
   surf->base.u.tex.last_layer = templ->u.tex.last_layer;
   surf->base.nr_samples = templ->nr_samples;

   virgl_encoder_create_surface(vctx, handle, res, &surf->base);
   surf->handle = handle;
   return &surf->base;
}

static void virgl_surface_destroy(struct pipe_context *ctx,
                                 struct pipe_surface *psurf)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_surface *surf = virgl_surface(psurf);

   pipe_resource_reference(&surf->base.texture, NULL);
   virgl_encode_delete_object(vctx, surf->handle, VIRGL_OBJECT_SURFACE);
   FREE(surf);
}

static void *virgl_create_blend_state(struct pipe_context *ctx,
                                              const struct pipe_blend_state *blend_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle;
   handle = virgl_object_assign_handle();

   virgl_encode_blend_state(vctx, handle, blend_state);
   return (void *)(unsigned long)handle;

}

static void virgl_bind_blend_state(struct pipe_context *ctx,
                                           void *blend_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle = (unsigned long)blend_state;
   virgl_encode_bind_object(vctx, handle, VIRGL_OBJECT_BLEND);
}

static void virgl_delete_blend_state(struct pipe_context *ctx,
                                     void *blend_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle = (unsigned long)blend_state;
   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_BLEND);
}

static void *virgl_create_depth_stencil_alpha_state(struct pipe_context *ctx,
                                                   const struct pipe_depth_stencil_alpha_state *blend_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle;
   handle = virgl_object_assign_handle();

   virgl_encode_dsa_state(vctx, handle, blend_state);
   return (void *)(unsigned long)handle;
}

static void virgl_bind_depth_stencil_alpha_state(struct pipe_context *ctx,
                                                void *blend_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle = (unsigned long)blend_state;
   virgl_encode_bind_object(vctx, handle, VIRGL_OBJECT_DSA);
}

static void virgl_delete_depth_stencil_alpha_state(struct pipe_context *ctx,
                                                  void *dsa_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle = (unsigned long)dsa_state;
   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_DSA);
}

static void *virgl_create_rasterizer_state(struct pipe_context *ctx,
                                                   const struct pipe_rasterizer_state *rs_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_rasterizer_state *vrs = CALLOC_STRUCT(virgl_rasterizer_state);

   if (!vrs)
      return NULL;
   vrs->rs = *rs_state;
   vrs->handle = virgl_object_assign_handle();

   assert(rs_state->depth_clip_near ||
          virgl_screen(ctx->screen)->caps.caps.v1.bset.depth_clip_disable);

   virgl_encode_rasterizer_state(vctx, vrs->handle, rs_state);
   return (void *)vrs;
}

static void virgl_bind_rasterizer_state(struct pipe_context *ctx,
                                                void *rs_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle = 0;
   if (rs_state) {
      struct virgl_rasterizer_state *vrs = rs_state;
      vctx->rs_state = *vrs;
      handle = vrs->handle;
   }
   virgl_encode_bind_object(vctx, handle, VIRGL_OBJECT_RASTERIZER);
}

static void virgl_delete_rasterizer_state(struct pipe_context *ctx,
                                         void *rs_state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_rasterizer_state *vrs = rs_state;
   virgl_encode_delete_object(vctx, vrs->handle, VIRGL_OBJECT_RASTERIZER);
   FREE(vrs);
}

static void virgl_set_framebuffer_state(struct pipe_context *ctx,
                                                const struct pipe_framebuffer_state *state)
{
   struct virgl_context *vctx = virgl_context(ctx);

   vctx->framebuffer = *state;
   virgl_encoder_set_framebuffer_state(vctx, state);
   virgl_attach_res_framebuffer(vctx);
}

static void virgl_set_viewport_states(struct pipe_context *ctx,
                                     unsigned start_slot,
                                     unsigned num_viewports,
                                     const struct pipe_viewport_state *state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   virgl_encoder_set_viewport_states(vctx, start_slot, num_viewports, state);
}

static void *virgl_create_vertex_elements_state(struct pipe_context *ctx,
                                                        unsigned num_elements,
                                                        const struct pipe_vertex_element *elements)
{
   struct pipe_vertex_element new_elements[PIPE_MAX_ATTRIBS];
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_vertex_elements_state *state =
      CALLOC_STRUCT(virgl_vertex_elements_state);

   for (int i = 0; i < num_elements; ++i) {
      if (elements[i].instance_divisor) {
         /* Virglrenderer doesn't deal with instance_divisor correctly if
         * there isn't a 1:1 relationship between elements and bindings.
         * So let's make sure there is, by duplicating bindings.
         */
         for (int j = 0; j < num_elements; ++j) {
            new_elements[j] = elements[j];
            new_elements[j].vertex_buffer_index = j;
            state->binding_map[j] = elements[j].vertex_buffer_index;
         }
         elements = new_elements;
         state->num_bindings = num_elements;
         break;
      }
   }
   for (int i = 0; i < num_elements; ++i)
      state->strides[elements[i].vertex_buffer_index] = elements[i].src_stride;

   state->handle = virgl_object_assign_handle();
   virgl_encoder_create_vertex_elements(vctx, state->handle,
                                       num_elements, elements);
   return state;
}

static void virgl_delete_vertex_elements_state(struct pipe_context *ctx,
                                              void *ve)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_vertex_elements_state *state =
      (struct virgl_vertex_elements_state *)ve;
   virgl_encode_delete_object(vctx, state->handle, VIRGL_OBJECT_VERTEX_ELEMENTS);
   FREE(state);
}

static void virgl_bind_vertex_elements_state(struct pipe_context *ctx,
                                                     void *ve)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_vertex_elements_state *state =
      (struct virgl_vertex_elements_state *)ve;
   vctx->vertex_elements = state;
   virgl_encode_bind_object(vctx, state ? state->handle : 0,
                            VIRGL_OBJECT_VERTEX_ELEMENTS);
   vctx->vertex_array_dirty = true;
}

static void virgl_set_vertex_buffers(struct pipe_context *ctx,
                                    unsigned num_buffers,
                                     unsigned unbind_num_trailing_slots,
                                     bool take_ownership,
                                    const struct pipe_vertex_buffer *buffers)
{
   struct virgl_context *vctx = virgl_context(ctx);

   util_set_vertex_buffers_count(vctx->vertex_buffer,
                                 &vctx->num_vertex_buffers,
                                 buffers, num_buffers,
                                 unbind_num_trailing_slots,
                                 take_ownership);

   if (buffers) {
      for (unsigned i = 0; i < num_buffers; i++) {
         struct virgl_resource *res =
            virgl_resource(buffers[i].buffer.resource);
         if (res && !buffers[i].is_user_buffer)
            res->bind_history |= PIPE_BIND_VERTEX_BUFFER;
      }
   }

   vctx->vertex_array_dirty = true;
}

static void virgl_hw_set_vertex_buffers(struct virgl_context *vctx)
{
   if (vctx->vertex_array_dirty) {
      const struct virgl_vertex_elements_state *ve = vctx->vertex_elements;

      if (ve->num_bindings) {
         struct pipe_vertex_buffer vertex_buffers[PIPE_MAX_ATTRIBS];
         for (int i = 0; i < ve->num_bindings; ++i)
            vertex_buffers[i] = vctx->vertex_buffer[ve->binding_map[i]];

         virgl_encoder_set_vertex_buffers(vctx, ve->num_bindings, vertex_buffers);
      } else
         virgl_encoder_set_vertex_buffers(vctx, vctx->num_vertex_buffers, vctx->vertex_buffer);

      virgl_attach_res_vertex_buffers(vctx);

      vctx->vertex_array_dirty = false;
   }
}

static void virgl_set_stencil_ref(struct pipe_context *ctx,
                                 const struct pipe_stencil_ref ref)
{
   struct virgl_context *vctx = virgl_context(ctx);
   virgl_encoder_set_stencil_ref(vctx, &ref);
}

static void virgl_set_blend_color(struct pipe_context *ctx,
                                 const struct pipe_blend_color *color)
{
   struct virgl_context *vctx = virgl_context(ctx);
   virgl_encoder_set_blend_color(vctx, color);
}

static void virgl_hw_set_index_buffer(struct virgl_context *vctx,
                                     struct virgl_indexbuf *ib)
{
   virgl_encoder_set_index_buffer(vctx, ib);
   virgl_attach_res_index_buffer(vctx, ib);
}

static void virgl_set_constant_buffer(struct pipe_context *ctx,
                                     enum pipe_shader_type shader, uint index,
                                      bool take_ownership,
                                     const struct pipe_constant_buffer *buf)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader];

   if (buf && buf->buffer) {
      struct virgl_resource *res = virgl_resource(buf->buffer);
      res->bind_history |= PIPE_BIND_CONSTANT_BUFFER;

      virgl_encoder_set_uniform_buffer(vctx, shader, index,
                                       buf->buffer_offset,
                                       buf->buffer_size, res);

      if (take_ownership) {
         pipe_resource_reference(&binding->ubos[index].buffer, NULL);
         binding->ubos[index].buffer = buf->buffer;
      } else {
         pipe_resource_reference(&binding->ubos[index].buffer, buf->buffer);
      }
      binding->ubos[index] = *buf;
      binding->ubo_enabled_mask |= 1 << index;
   } else {
      static const struct pipe_constant_buffer dummy_ubo;
      if (!buf)
         buf = &dummy_ubo;
      virgl_encoder_write_constant_buffer(vctx, shader, index,
                                          buf->buffer_size / 4,
                                          buf->user_buffer);

      pipe_resource_reference(&binding->ubos[index].buffer, NULL);
      binding->ubo_enabled_mask &= ~(1 << index);
   }
}

static bool
lower_gles_arrayshadow_offset_filter(const nir_instr *instr,
                                     UNUSED const void *data)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   if (!tex->is_shadow || !tex->is_array)
      return false;

   // textureGradOffset can be used directly
   int grad_index = nir_tex_instr_src_index(tex, nir_tex_src_ddx);
   int proj_index = nir_tex_instr_src_index(tex, nir_tex_src_projector);
   if (grad_index >= 0 && proj_index < 0)
      return false;

   int offset_index = nir_tex_instr_src_index(tex, nir_tex_src_offset);
   if (offset_index >= 0)
      return true;

   return false;
}

static void *virgl_shader_encoder(struct pipe_context *ctx,
                                  const struct pipe_shader_state *shader,
                                  unsigned type)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);
   uint32_t handle;
   const struct tgsi_token *tokens;
   const struct tgsi_token *ntt_tokens = NULL;
   struct tgsi_token *new_tokens;
   int ret;
   bool is_separable = false;

   if (shader->type == PIPE_SHADER_IR_NIR) {
      struct nir_to_tgsi_options options = {
         .unoptimized_ra = true,
         .lower_fabs = true,
         .lower_ssbo_bindings =
               rs->caps.caps.v2.host_feature_check_version >= 16,
         .non_compute_membar_needs_all_modes = true
      };

      if (!(rs->caps.caps.v2.capability_bits_v2 & VIRGL_CAP_V2_TEXTURE_SHADOW_LOD) &&
          rs->caps.caps.v2.capability_bits & VIRGL_CAP_HOST_IS_GLES) {
         nir_lower_tex_options lower_tex_options = {
            .lower_offset_filter = lower_gles_arrayshadow_offset_filter,
         };

         NIR_PASS_V(shader->ir.nir, nir_lower_tex, &lower_tex_options);
      }

      nir_shader *s = nir_shader_clone(NULL, shader->ir.nir);

      /* The host can't handle certain IO slots as separable, because we can't assign
       * more than 32 IO locations explicitly, and with varyings and patches we already
       * exhaust the possible ways of handling this for the varyings with generic names,
       * so drop the flag in these cases */
      const uint64_t drop_slots_for_separable_io = 0xffull << VARYING_SLOT_TEX0 |
                                                        1 <<  VARYING_SLOT_FOGC |
                                                        1 <<  VARYING_SLOT_BFC0 |
                                                        1 <<  VARYING_SLOT_BFC1 |
                                                        1 <<  VARYING_SLOT_COL0 |
                                                        1 <<  VARYING_SLOT_COL1;
      bool keep_separable_flags = true;
      if (s->info.stage != MESA_SHADER_VERTEX)
         keep_separable_flags &= !(s->info.inputs_read & drop_slots_for_separable_io);
      if (s->info.stage != MESA_SHADER_FRAGMENT)
         keep_separable_flags &= !(s->info.outputs_written & drop_slots_for_separable_io);

      /* Propagare the separable shader property to the host, unless
       * it is an internal shader - these are marked separable even though they are not. */
      is_separable = s->info.separate_shader && !s->info.internal && keep_separable_flags;
      ntt_tokens = tokens = nir_to_tgsi_options(s, vctx->base.screen, &options); /* takes ownership */
   } else {
      tokens = shader->tokens;
   }

   new_tokens = virgl_tgsi_transform(rs, tokens, is_separable);
   if (!new_tokens)
      return NULL;

   handle = virgl_object_assign_handle();
   /* encode VS state */
   ret = virgl_encode_shader_state(vctx, handle, type,
                                   &shader->stream_output, 0,
                                   new_tokens);
   if (ret) {
      FREE((void *)ntt_tokens);
      return NULL;
   }

   FREE((void *)ntt_tokens);
   FREE(new_tokens);
   return (void *)(unsigned long)handle;

}
static void *virgl_create_vs_state(struct pipe_context *ctx,
                                   const struct pipe_shader_state *shader)
{
   return virgl_shader_encoder(ctx, shader, PIPE_SHADER_VERTEX);
}

static void *virgl_create_tcs_state(struct pipe_context *ctx,
                                   const struct pipe_shader_state *shader)
{
   return virgl_shader_encoder(ctx, shader, PIPE_SHADER_TESS_CTRL);
}

static void *virgl_create_tes_state(struct pipe_context *ctx,
                                   const struct pipe_shader_state *shader)
{
   return virgl_shader_encoder(ctx, shader, PIPE_SHADER_TESS_EVAL);
}

static void *virgl_create_gs_state(struct pipe_context *ctx,
                                   const struct pipe_shader_state *shader)
{
   return virgl_shader_encoder(ctx, shader, PIPE_SHADER_GEOMETRY);
}

static void *virgl_create_fs_state(struct pipe_context *ctx,
                                   const struct pipe_shader_state *shader)
{
   return virgl_shader_encoder(ctx, shader, PIPE_SHADER_FRAGMENT);
}

static void
virgl_delete_fs_state(struct pipe_context *ctx,
                     void *fs)
{
   uint32_t handle = (unsigned long)fs;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_SHADER);
}

static void
virgl_delete_gs_state(struct pipe_context *ctx,
                     void *gs)
{
   uint32_t handle = (unsigned long)gs;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_SHADER);
}

static void
virgl_delete_vs_state(struct pipe_context *ctx,
                     void *vs)
{
   uint32_t handle = (unsigned long)vs;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_SHADER);
}

static void
virgl_delete_tcs_state(struct pipe_context *ctx,
                       void *tcs)
{
   uint32_t handle = (unsigned long)tcs;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_SHADER);
}

static void
virgl_delete_tes_state(struct pipe_context *ctx,
                      void *tes)
{
   uint32_t handle = (unsigned long)tes;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_SHADER);
}

static void virgl_bind_vs_state(struct pipe_context *ctx,
                                        void *vss)
{
   uint32_t handle = (unsigned long)vss;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_bind_shader(vctx, handle, PIPE_SHADER_VERTEX);
}

static void virgl_bind_tcs_state(struct pipe_context *ctx,
                               void *vss)
{
   uint32_t handle = (unsigned long)vss;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_bind_shader(vctx, handle, PIPE_SHADER_TESS_CTRL);
}

static void virgl_bind_tes_state(struct pipe_context *ctx,
                               void *vss)
{
   uint32_t handle = (unsigned long)vss;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_bind_shader(vctx, handle, PIPE_SHADER_TESS_EVAL);
}

static void virgl_bind_gs_state(struct pipe_context *ctx,
                               void *vss)
{
   uint32_t handle = (unsigned long)vss;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_bind_shader(vctx, handle, PIPE_SHADER_GEOMETRY);
}


static void virgl_bind_fs_state(struct pipe_context *ctx,
                                        void *vss)
{
   uint32_t handle = (unsigned long)vss;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_bind_shader(vctx, handle, PIPE_SHADER_FRAGMENT);
}

static void virgl_clear(struct pipe_context *ctx,
                                unsigned buffers,
                                const struct pipe_scissor_state *scissor_state,
                                const union pipe_color_union *color,
                                double depth, unsigned stencil)
{
   struct virgl_context *vctx = virgl_context(ctx);

   if (!vctx->num_draws)
      virgl_reemit_draw_resources(vctx);
   vctx->num_draws++;

   virgl_encode_clear(vctx, buffers, color, depth, stencil);
}

static void virgl_clear_render_target(struct pipe_context *ctx,
                                      struct pipe_surface *dst,
                                      const union pipe_color_union *color,
                                      unsigned dstx, unsigned dsty,
                                      unsigned width, unsigned height,
                                      bool render_condition_enabled)
{
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_clear_surface(vctx, dst, PIPE_CLEAR_COLOR0, color,
                             dstx, dsty, width, height, render_condition_enabled);

   /* Mark as dirty, since we are updating the host side resource
    * without going through the corresponding guest side resource, and
    * hence the two will diverge.
    */
   virgl_resource_dirty(virgl_resource(dst->texture), dst->u.tex.level);
}

static void virgl_clear_depth_stencil(struct pipe_context *ctx,
                                      struct pipe_surface *dst,
                                      unsigned clear_flags,
                                      double depth,
                                      unsigned stencil,
                                      unsigned dstx, unsigned dsty,
                                      unsigned width, unsigned height,
                                      bool render_condition_enabled)
{
   struct virgl_context *vctx = virgl_context(ctx);

   union pipe_color_union color;
   memcpy(color.ui, &depth, sizeof(double));
   color.ui[3] = stencil;

   virgl_encode_clear_surface(vctx, dst, clear_flags, &color,
                             dstx, dsty, width, height, render_condition_enabled);

   /* Mark as dirty, since we are updating the host side resource
    * without going through the corresponding guest side resource, and
    * hence the two will diverge.
    */
   virgl_resource_dirty(virgl_resource(dst->texture), dst->u.tex.level);
}

static void virgl_clear_render_target_stub(struct pipe_context *ctx,
                                           struct pipe_surface *dst,
                                           const union pipe_color_union *color,
                                           unsigned dstx, unsigned dsty,
                                           unsigned width, unsigned height,
                                           bool render_condition_enabled)
{
   if (virgl_debug & VIRGL_DEBUG_VERBOSE)
         debug_printf("VIRGL: clear depth stencil unsupported.\n");
   return;
}

static void virgl_clear_texture(struct pipe_context *ctx,
                                struct pipe_resource *res,
                                unsigned int level,
                                const struct pipe_box *box,
                                const void *data)
{
   struct virgl_screen *rs = virgl_screen(ctx->screen);
   struct virgl_resource *vres = virgl_resource(res);

   if (rs->caps.caps.v2.capability_bits & VIRGL_CAP_CLEAR_TEXTURE) {
      struct virgl_context *vctx = virgl_context(ctx);
      virgl_encode_clear_texture(vctx, vres, level, box, data);
   } else {
      u_default_clear_texture(ctx, res, level, box, data);
   }
   /* Mark as dirty, since we are updating the host side resource
    * without going through the corresponding guest side resource, and
    * hence the two will diverge.
    */
   virgl_resource_dirty(vres, level);
}

static void virgl_draw_vbo(struct pipe_context *ctx,
                           const struct pipe_draw_info *dinfo,
                           unsigned drawid_offset,
                           const struct pipe_draw_indirect_info *indirect,
                           const struct pipe_draw_start_count_bias *draws,
                           unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(ctx, dinfo, drawid_offset, indirect, draws, num_draws);
      return;
   }

   if (!indirect && (!draws[0].count || !dinfo->instance_count))
      return;

   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);
   struct virgl_indexbuf ib = {};
   struct pipe_draw_info info = *dinfo;

   if (!indirect &&
       !dinfo->primitive_restart &&
       !u_trim_pipe_prim(dinfo->mode, (unsigned*)&draws[0].count))
      return;

   if (!(rs->caps.caps.v1.prim_mask & (1 << dinfo->mode))) {
      util_primconvert_save_rasterizer_state(vctx->primconvert, &vctx->rs_state.rs);
      util_primconvert_draw_vbo(vctx->primconvert, dinfo, drawid_offset, indirect, draws, num_draws);
      return;
   }
   if (info.index_size) {
           pipe_resource_reference(&ib.buffer, info.has_user_indices ? NULL : info.index.resource);
           ib.user_buffer = info.has_user_indices ? info.index.user : NULL;
           ib.index_size = dinfo->index_size;
           ib.offset = draws[0].start * ib.index_size;

           if (ib.user_buffer) {
                   unsigned start_offset = draws[0].start * ib.index_size;
                   u_upload_data(vctx->uploader, 0,
                                 draws[0].count * ib.index_size, 4,
                                 (char*)ib.user_buffer + start_offset,
                                 &ib.offset, &ib.buffer);
                   ib.user_buffer = NULL;
           }
           virgl_hw_set_index_buffer(vctx, &ib);
   }

   if (!vctx->num_draws)
      virgl_reemit_draw_resources(vctx);
   vctx->num_draws++;

   virgl_hw_set_vertex_buffers(vctx);

   virgl_encoder_draw_vbo(vctx, &info, drawid_offset, indirect, &draws[0]);

   pipe_resource_reference(&ib.buffer, NULL);

}

static void virgl_submit_cmd(struct virgl_winsys *vws,
                             struct virgl_cmd_buf *cbuf,
			     struct pipe_fence_handle **fence)
{
   if (unlikely(virgl_debug & VIRGL_DEBUG_SYNC)) {
      struct pipe_fence_handle *sync_fence = NULL;

      vws->submit_cmd(vws, cbuf, &sync_fence);

      vws->fence_wait(vws, sync_fence, OS_TIMEOUT_INFINITE);
      vws->fence_reference(vws, &sync_fence, NULL);
   } else {
      vws->submit_cmd(vws, cbuf, fence);
   }
}

void virgl_flush_eq(struct virgl_context *ctx, void *closure,
                    struct pipe_fence_handle **fence)
{
   struct virgl_screen *rs = virgl_screen(ctx->base.screen);

   /* skip empty cbuf */
   if (ctx->cbuf->cdw == ctx->cbuf_initial_cdw &&
       ctx->queue.num_dwords == 0 &&
       !fence)
      return;

   if (ctx->num_draws)
      u_upload_unmap(ctx->uploader);

   /* send the buffer to the remote side for decoding */
   ctx->num_draws = ctx->num_compute = 0;

   virgl_transfer_queue_clear(&ctx->queue, ctx->cbuf);

   virgl_submit_cmd(rs->vws, ctx->cbuf, fence);

   /* Reserve some space for transfers. */
   if (ctx->encoded_transfers)
      ctx->cbuf->cdw = VIRGL_MAX_TBUF_DWORDS;

   virgl_encoder_set_sub_ctx(ctx, ctx->hw_sub_ctx_id);

   ctx->cbuf_initial_cdw = ctx->cbuf->cdw;

   /* We have flushed the command queue, including any pending copy transfers
    * involving staging resources.
    */
   ctx->queued_staging_res_size = 0;
}

static void virgl_flush_from_st(struct pipe_context *ctx,
                               struct pipe_fence_handle **fence,
                               enum pipe_flush_flags flags)
{
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_flush_eq(vctx, vctx, fence);
}

static struct pipe_sampler_view *virgl_create_sampler_view(struct pipe_context *ctx,
                                      struct pipe_resource *texture,
                                      const struct pipe_sampler_view *state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_sampler_view *grview;
   uint32_t handle;
   struct virgl_resource *res;

   if (!state)
      return NULL;

   grview = CALLOC_STRUCT(virgl_sampler_view);
   if (!grview)
      return NULL;

   res = virgl_resource(texture);
   handle = virgl_object_assign_handle();
   virgl_encode_sampler_view(vctx, handle, res, state);

   grview->base = *state;
   grview->base.reference.count = 1;

   grview->base.texture = NULL;
   grview->base.context = ctx;
   pipe_resource_reference(&grview->base.texture, texture);
   grview->handle = handle;
   return &grview->base;
}

static void virgl_set_sampler_views(struct pipe_context *ctx,
                                   enum pipe_shader_type shader_type,
                                   unsigned start_slot,
                                   unsigned num_views,
                                   unsigned unbind_num_trailing_slots,
                                   bool take_ownership,
                                   struct pipe_sampler_view **views)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader_type];

   for (unsigned i = 0; i < num_views; i++) {
      unsigned idx = start_slot + i;
      if (views && views[i]) {
         struct virgl_resource *res = virgl_resource(views[i]->texture);
         res->bind_history |= PIPE_BIND_SAMPLER_VIEW;

         if (take_ownership) {
            pipe_sampler_view_reference(&binding->views[idx], NULL);
            binding->views[idx] = views[i];
         } else {
            pipe_sampler_view_reference(&binding->views[idx], views[i]);
         }
      } else {
         pipe_sampler_view_reference(&binding->views[idx], NULL);
      }
   }

   virgl_encode_set_sampler_views(vctx, shader_type,
         start_slot, num_views, (struct virgl_sampler_view **)binding->views);
   virgl_attach_res_sampler_views(vctx, shader_type);

   if (unbind_num_trailing_slots) {
      virgl_set_sampler_views(ctx, shader_type, start_slot + num_views,
                              unbind_num_trailing_slots, 0, false, NULL);
   }
}

static void
virgl_texture_barrier(struct pipe_context *ctx, unsigned flags)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);

   if (!(rs->caps.caps.v2.capability_bits & VIRGL_CAP_TEXTURE_BARRIER) &&
       !(rs->caps.caps.v2.capability_bits_v2 & VIRGL_CAP_V2_BLEND_EQUATION))
      return;
   virgl_encode_texture_barrier(vctx, flags);
}

static void virgl_destroy_sampler_view(struct pipe_context *ctx,
                                 struct pipe_sampler_view *view)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_sampler_view *grview = virgl_sampler_view(view);

   virgl_encode_delete_object(vctx, grview->handle, VIRGL_OBJECT_SAMPLER_VIEW);
   pipe_resource_reference(&view->texture, NULL);
   FREE(view);
}

static void *virgl_create_sampler_state(struct pipe_context *ctx,
                                        const struct pipe_sampler_state *state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle;

   handle = virgl_object_assign_handle();

   virgl_encode_sampler_state(vctx, handle, state);
   return (void *)(unsigned long)handle;
}

static void virgl_delete_sampler_state(struct pipe_context *ctx,
                                      void *ss)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle = (unsigned long)ss;

   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_SAMPLER_STATE);
}

static void virgl_bind_sampler_states(struct pipe_context *ctx,
                                     enum pipe_shader_type shader,
                                     unsigned start_slot,
                                     unsigned num_samplers,
                                     void **samplers)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handles[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   int i;
   for (i = 0; i < num_samplers; i++) {
      handles[i] = (unsigned long)(samplers[i]);
   }
   virgl_encode_bind_sampler_states(vctx, shader, start_slot, num_samplers, handles);
}

static void virgl_set_polygon_stipple(struct pipe_context *ctx,
                                     const struct pipe_poly_stipple *ps)
{
   struct virgl_context *vctx = virgl_context(ctx);
   virgl_encoder_set_polygon_stipple(vctx, ps);
}

static void virgl_set_scissor_states(struct pipe_context *ctx,
                                    unsigned start_slot,
                                    unsigned num_scissor,
                                   const struct pipe_scissor_state *ss)
{
   struct virgl_context *vctx = virgl_context(ctx);
   virgl_encoder_set_scissor_state(vctx, start_slot, num_scissor, ss);
}

static void virgl_set_sample_mask(struct pipe_context *ctx,
                                 unsigned sample_mask)
{
   struct virgl_context *vctx = virgl_context(ctx);
   virgl_encoder_set_sample_mask(vctx, sample_mask);
}

static void virgl_set_min_samples(struct pipe_context *ctx,
                                 unsigned min_samples)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);

   if (!(rs->caps.caps.v2.capability_bits & VIRGL_CAP_SET_MIN_SAMPLES))
      return;
   virgl_encoder_set_min_samples(vctx, min_samples);
}

static void virgl_set_clip_state(struct pipe_context *ctx,
                                const struct pipe_clip_state *clip)
{
   struct virgl_context *vctx = virgl_context(ctx);
   virgl_encoder_set_clip_state(vctx, clip);
}

static void virgl_set_tess_state(struct pipe_context *ctx,
                                 const float default_outer_level[4],
                                 const float default_inner_level[2])
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);

   if (!rs->caps.caps.v1.bset.has_tessellation_shaders)
      return;
   virgl_encode_set_tess_state(vctx, default_outer_level, default_inner_level);
}

static void virgl_set_patch_vertices(struct pipe_context *ctx, uint8_t patch_vertices)
{
   struct virgl_context *vctx = virgl_context(ctx);

   vctx->patch_vertices = patch_vertices;
}

static void virgl_resource_copy_region(struct pipe_context *ctx,
                                      struct pipe_resource *dst,
                                      unsigned dst_level,
                                      unsigned dstx, unsigned dsty, unsigned dstz,
                                      struct pipe_resource *src,
                                      unsigned src_level,
                                      const struct pipe_box *src_box)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_resource *dres = virgl_resource(dst);
   struct virgl_resource *sres = virgl_resource(src);

   if (dres->b.target == PIPE_BUFFER)
      util_range_add(&dres->b, &dres->valid_buffer_range, dstx, dstx + src_box->width);
   virgl_resource_dirty(dres, dst_level);

   virgl_encode_resource_copy_region(vctx, dres,
                                    dst_level, dstx, dsty, dstz,
                                    sres, src_level,
                                    src_box);
}

static void
virgl_flush_resource(struct pipe_context *pipe,
                    struct pipe_resource *resource)
{
}

static void virgl_blit(struct pipe_context *ctx,
                      const struct pipe_blit_info *blit)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_resource *dres = virgl_resource(blit->dst.resource);
   struct virgl_resource *sres = virgl_resource(blit->src.resource);

   assert(ctx->screen->get_param(ctx->screen,
                                 PIPE_CAP_DEST_SURFACE_SRGB_CONTROL) ||
          (util_format_is_srgb(blit->dst.resource->format) ==
            util_format_is_srgb(blit->dst.format)));

   virgl_resource_dirty(dres, blit->dst.level);
   virgl_encode_blit(vctx, dres, sres,
                    blit);
}

static void virgl_set_hw_atomic_buffers(struct pipe_context *ctx,
                                        unsigned start_slot,
                                        unsigned count,
                                        const struct pipe_shader_buffer *buffers)
{
   struct virgl_context *vctx = virgl_context(ctx);

   vctx->atomic_buffer_enabled_mask &= ~u_bit_consecutive(start_slot, count);
   for (unsigned i = 0; i < count; i++) {
      unsigned idx = start_slot + i;
      if (buffers && buffers[i].buffer) {
         struct virgl_resource *res = virgl_resource(buffers[i].buffer);
         res->bind_history |= PIPE_BIND_SHADER_BUFFER;

         pipe_resource_reference(&vctx->atomic_buffers[idx].buffer,
                                 buffers[i].buffer);
         vctx->atomic_buffers[idx] = buffers[i];
         vctx->atomic_buffer_enabled_mask |= 1 << idx;
      } else {
         pipe_resource_reference(&vctx->atomic_buffers[idx].buffer, NULL);
      }
   }

   virgl_encode_set_hw_atomic_buffers(vctx, start_slot, count, buffers);
}

static void virgl_set_shader_buffers(struct pipe_context *ctx,
                                     enum pipe_shader_type shader,
                                     unsigned start_slot, unsigned count,
                                     const struct pipe_shader_buffer *buffers,
                                     unsigned writable_bitmask)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);
   struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader];

   binding->ssbo_enabled_mask &= ~u_bit_consecutive(start_slot, count);
   for (unsigned i = 0; i < count; i++) {
      unsigned idx = start_slot + i;
      if (buffers && buffers[i].buffer) {
         struct virgl_resource *res = virgl_resource(buffers[i].buffer);
         res->bind_history |= PIPE_BIND_SHADER_BUFFER;

         pipe_resource_reference(&binding->ssbos[idx].buffer, buffers[i].buffer);
         binding->ssbos[idx] = buffers[i];
         binding->ssbo_enabled_mask |= 1 << idx;
      } else {
         pipe_resource_reference(&binding->ssbos[idx].buffer, NULL);
      }
   }

   uint32_t max_shader_buffer = (shader == PIPE_SHADER_FRAGMENT || shader == PIPE_SHADER_COMPUTE) ?
      rs->caps.caps.v2.max_shader_buffer_frag_compute :
      rs->caps.caps.v2.max_shader_buffer_other_stages;
   if (!max_shader_buffer)
      return;
   virgl_encode_set_shader_buffers(vctx, shader, start_slot, count, buffers);
}

static void virgl_create_fence_fd(struct pipe_context *ctx,
                                  struct pipe_fence_handle **fence,
                                  int fd,
                                  enum pipe_fd_type type)
{
   assert(type == PIPE_FD_TYPE_NATIVE_SYNC);
   struct virgl_screen *rs = virgl_screen(ctx->screen);

   if (rs->vws->cs_create_fence)
      *fence = rs->vws->cs_create_fence(rs->vws, fd);
}

static void virgl_fence_server_sync(struct pipe_context *ctx,
			            struct pipe_fence_handle *fence)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);

   if (rs->vws->fence_server_sync)
      rs->vws->fence_server_sync(rs->vws, vctx->cbuf, fence);
}

static void virgl_set_shader_images(struct pipe_context *ctx,
                                    enum pipe_shader_type shader,
                                    unsigned start_slot, unsigned count,
                                    unsigned unbind_num_trailing_slots,
                                    const struct pipe_image_view *images)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);
   struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader];

   binding->image_enabled_mask &= ~u_bit_consecutive(start_slot, count);
   for (unsigned i = 0; i < count; i++) {
      unsigned idx = start_slot + i;
      if (images && images[i].resource) {
         struct virgl_resource *res = virgl_resource(images[i].resource);
         res->bind_history |= PIPE_BIND_SHADER_IMAGE;

         pipe_resource_reference(&binding->images[idx].resource,
                                 images[i].resource);
         binding->images[idx] = images[i];
         binding->image_enabled_mask |= 1 << idx;
      } else {
         pipe_resource_reference(&binding->images[idx].resource, NULL);
      }
   }

   uint32_t max_shader_images = (shader == PIPE_SHADER_FRAGMENT || shader == PIPE_SHADER_COMPUTE) ?
     rs->caps.caps.v2.max_shader_image_frag_compute :
     rs->caps.caps.v2.max_shader_image_other_stages;
   if (!max_shader_images)
      return;
   virgl_encode_set_shader_images(vctx, shader, start_slot, count, images);

   if (unbind_num_trailing_slots) {
      virgl_set_shader_images(ctx, shader, start_slot + count,
                              unbind_num_trailing_slots, 0, NULL);
   }
}

static void virgl_memory_barrier(struct pipe_context *ctx,
                                 unsigned flags)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);

   if (!(rs->caps.caps.v2.capability_bits & VIRGL_CAP_MEMORY_BARRIER))
      return;
   virgl_encode_memory_barrier(vctx, flags);
}

static void *virgl_create_compute_state(struct pipe_context *ctx,
                                        const struct pipe_compute_state *state)
{
   struct virgl_context *vctx = virgl_context(ctx);
   uint32_t handle;
   const struct tgsi_token *ntt_tokens = NULL;
   const struct tgsi_token *tokens;
   struct pipe_stream_output_info so_info = {};
   int ret;

   if (state->ir_type == PIPE_SHADER_IR_NIR) {
      struct nir_to_tgsi_options options = {
         .unoptimized_ra = true,
         .lower_fabs = true
      };
      nir_shader *s = nir_shader_clone(NULL, state->prog);
      ntt_tokens = tokens = nir_to_tgsi_options(s, vctx->base.screen, &options); /* takes ownership */
   } else {
      tokens = state->prog;
   }

   void *new_tokens = virgl_tgsi_transform((struct virgl_screen *)vctx->base.screen, tokens, false);
   if (!new_tokens)
      return NULL;

   handle = virgl_object_assign_handle();
   ret = virgl_encode_shader_state(vctx, handle, PIPE_SHADER_COMPUTE,
                                   &so_info,
                                   state->static_shared_mem,
                                   new_tokens);
   if (ret) {
      FREE((void *)ntt_tokens);
      return NULL;
   }

   FREE((void *)ntt_tokens);
   FREE(new_tokens);

   return (void *)(unsigned long)handle;
}

static void virgl_bind_compute_state(struct pipe_context *ctx, void *state)
{
   uint32_t handle = (unsigned long)state;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_bind_shader(vctx, handle, PIPE_SHADER_COMPUTE);
}

static void virgl_delete_compute_state(struct pipe_context *ctx, void *state)
{
   uint32_t handle = (unsigned long)state;
   struct virgl_context *vctx = virgl_context(ctx);

   virgl_encode_delete_object(vctx, handle, VIRGL_OBJECT_SHADER);
}

static void virgl_launch_grid(struct pipe_context *ctx,
                              const struct pipe_grid_info *info)
{
   struct virgl_context *vctx = virgl_context(ctx);

   if (!vctx->num_compute)
      virgl_reemit_compute_resources(vctx);
   vctx->num_compute++;

   virgl_encode_launch_grid(vctx, info);
}

static void
virgl_release_shader_binding(struct virgl_context *vctx,
                             enum pipe_shader_type shader_type)
{
   struct virgl_shader_binding_state *binding =
      &vctx->shader_bindings[shader_type];

   for (int i = 0; i < PIPE_MAX_SHADER_SAMPLER_VIEWS; ++i) {
      if (binding->views[i]) {
         pipe_sampler_view_reference(
                  (struct pipe_sampler_view **)&binding->views[i], NULL);
      }
   }

   while (binding->ubo_enabled_mask) {
      int i = u_bit_scan(&binding->ubo_enabled_mask);
      pipe_resource_reference(&binding->ubos[i].buffer, NULL);
   }

   while (binding->ssbo_enabled_mask) {
      int i = u_bit_scan(&binding->ssbo_enabled_mask);
      pipe_resource_reference(&binding->ssbos[i].buffer, NULL);
   }

   while (binding->image_enabled_mask) {
      int i = u_bit_scan(&binding->image_enabled_mask);
      pipe_resource_reference(&binding->images[i].resource, NULL);
   }
}

static void
virgl_emit_string_marker(struct pipe_context *ctx, const char *message,  int len)
{
    struct virgl_context *vctx = virgl_context(ctx);
    virgl_encode_emit_string_marker(vctx, message, len);
}

static void
virgl_context_destroy( struct pipe_context *ctx )
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(ctx->screen);
   enum pipe_shader_type shader_type;

   vctx->framebuffer.zsbuf = NULL;
   vctx->framebuffer.nr_cbufs = 0;
   virgl_encoder_destroy_sub_ctx(vctx, vctx->hw_sub_ctx_id);
   virgl_flush_eq(vctx, vctx, NULL);

   for (shader_type = 0; shader_type < PIPE_SHADER_TYPES; shader_type++)
      virgl_release_shader_binding(vctx, shader_type);

   while (vctx->atomic_buffer_enabled_mask) {
      int i = u_bit_scan(&vctx->atomic_buffer_enabled_mask);
      pipe_resource_reference(&vctx->atomic_buffers[i].buffer, NULL);
   }

   rs->vws->cmd_buf_destroy(vctx->cbuf);
   if (vctx->uploader)
      u_upload_destroy(vctx->uploader);
   if (vctx->supports_staging)
      virgl_staging_destroy(&vctx->staging);
   util_primconvert_destroy(vctx->primconvert);
   virgl_transfer_queue_fini(&vctx->queue);

   slab_destroy_child(&vctx->transfer_pool);
   FREE(vctx);
}

static void virgl_get_sample_position(struct pipe_context *ctx,
				      unsigned sample_count,
				      unsigned index,
				      float *out_value)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *vs = virgl_screen(vctx->base.screen);

   if (sample_count > vs->caps.caps.v1.max_samples) {
      debug_printf("VIRGL: requested %d MSAA samples, but only %d supported\n",
		   sample_count, vs->caps.caps.v1.max_samples);
      return;
   }

   /* The following is basically copied from dri/i965gen6_get_sample_position
    * The only addition is that we hold the msaa positions for all sample
    * counts in a flat array. */
   uint32_t bits = 0;
   if (sample_count == 1) {
      out_value[0] = out_value[1] = 0.5f;
      return;
   } else if (sample_count == 2) {
      bits = vs->caps.caps.v2.sample_locations[0] >> (8 * index);
   } else if (sample_count <= 4) {
      bits = vs->caps.caps.v2.sample_locations[1] >> (8 * index);
   } else if (sample_count <= 8) {
      bits = vs->caps.caps.v2.sample_locations[2 + (index >> 2)] >> (8 * (index & 3));
   } else if (sample_count <= 16) {
      bits = vs->caps.caps.v2.sample_locations[4 + (index >> 2)] >> (8 * (index & 3));
   }
   out_value[0] = ((bits >> 4) & 0xf) / 16.0f;
   out_value[1] = (bits & 0xf) / 16.0f;

   if (virgl_debug & VIRGL_DEBUG_VERBOSE)
      debug_printf("VIRGL: sample postion [%2d/%2d] = (%f, %f)\n",
                   index, sample_count, out_value[0], out_value[1]);
}

static void virgl_send_tweaks(struct virgl_context *vctx, struct virgl_screen *rs)
{
   if (rs->tweak_gles_emulate_bgra)
      virgl_encode_tweak(vctx, virgl_tweak_gles_brga_emulate, 1);

   if (rs->tweak_gles_apply_bgra_dest_swizzle)
      virgl_encode_tweak(vctx, virgl_tweak_gles_brga_apply_dest_swizzle, 1);

   if (rs->tweak_gles_tf3_value > 0)
      virgl_encode_tweak(vctx, virgl_tweak_gles_tf3_samples_passes_multiplier,
                         rs->tweak_gles_tf3_value);
}

static void virgl_link_shader(struct pipe_context *ctx, void **handles)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *rs = virgl_screen(vctx->base.screen);

   uint32_t shader_handles[PIPE_SHADER_TYPES];
   for (uint32_t i = 0; i < PIPE_SHADER_TYPES; ++i)
      shader_handles[i] = (uintptr_t)handles[i];
   virgl_encode_link_shader(vctx, shader_handles);

   /* block until shader linking is finished on host */
   if (rs->shader_sync && !unlikely(virgl_debug & VIRGL_DEBUG_SYNC)) {
      struct virgl_winsys *vws = rs->vws;
      struct pipe_fence_handle *sync_fence;
      virgl_flush_eq(vctx, vctx, &sync_fence);
      vws->fence_wait(vws, sync_fence, OS_TIMEOUT_INFINITE);
      vws->fence_reference(vws, &sync_fence, NULL);
   }
}

struct pipe_context *virgl_context_create(struct pipe_screen *pscreen,
                                          void *priv,
                                          unsigned flags)
{
   struct virgl_context *vctx;
   struct virgl_screen *rs = virgl_screen(pscreen);
   vctx = CALLOC_STRUCT(virgl_context);
   const char *host_debug_flagstring;

   vctx->cbuf = rs->vws->cmd_buf_create(rs->vws, VIRGL_MAX_CMDBUF_DWORDS);
   if (!vctx->cbuf) {
      FREE(vctx);
      return NULL;
   }

   vctx->base.destroy = virgl_context_destroy;
   vctx->base.create_surface = virgl_create_surface;
   vctx->base.surface_destroy = virgl_surface_destroy;
   vctx->base.set_framebuffer_state = virgl_set_framebuffer_state;
   vctx->base.create_blend_state = virgl_create_blend_state;
   vctx->base.bind_blend_state = virgl_bind_blend_state;
   vctx->base.delete_blend_state = virgl_delete_blend_state;
   vctx->base.create_depth_stencil_alpha_state = virgl_create_depth_stencil_alpha_state;
   vctx->base.bind_depth_stencil_alpha_state = virgl_bind_depth_stencil_alpha_state;
   vctx->base.delete_depth_stencil_alpha_state = virgl_delete_depth_stencil_alpha_state;
   vctx->base.create_rasterizer_state = virgl_create_rasterizer_state;
   vctx->base.bind_rasterizer_state = virgl_bind_rasterizer_state;
   vctx->base.delete_rasterizer_state = virgl_delete_rasterizer_state;

   vctx->base.set_viewport_states = virgl_set_viewport_states;
   vctx->base.create_vertex_elements_state = virgl_create_vertex_elements_state;
   vctx->base.bind_vertex_elements_state = virgl_bind_vertex_elements_state;
   vctx->base.delete_vertex_elements_state = virgl_delete_vertex_elements_state;
   vctx->base.set_vertex_buffers = virgl_set_vertex_buffers;
   vctx->base.set_constant_buffer = virgl_set_constant_buffer;

   vctx->base.set_tess_state = virgl_set_tess_state;
   vctx->base.set_patch_vertices = virgl_set_patch_vertices;
   vctx->base.create_vs_state = virgl_create_vs_state;
   vctx->base.create_tcs_state = virgl_create_tcs_state;
   vctx->base.create_tes_state = virgl_create_tes_state;
   vctx->base.create_gs_state = virgl_create_gs_state;
   vctx->base.create_fs_state = virgl_create_fs_state;

   vctx->base.bind_vs_state = virgl_bind_vs_state;
   vctx->base.bind_tcs_state = virgl_bind_tcs_state;
   vctx->base.bind_tes_state = virgl_bind_tes_state;
   vctx->base.bind_gs_state = virgl_bind_gs_state;
   vctx->base.bind_fs_state = virgl_bind_fs_state;

   vctx->base.delete_vs_state = virgl_delete_vs_state;
   vctx->base.delete_tcs_state = virgl_delete_tcs_state;
   vctx->base.delete_tes_state = virgl_delete_tes_state;
   vctx->base.delete_gs_state = virgl_delete_gs_state;
   vctx->base.delete_fs_state = virgl_delete_fs_state;

   vctx->base.create_compute_state = virgl_create_compute_state;
   vctx->base.bind_compute_state = virgl_bind_compute_state;
   vctx->base.delete_compute_state = virgl_delete_compute_state;
   vctx->base.launch_grid = virgl_launch_grid;

   vctx->base.clear = virgl_clear;
   if (rs->caps.caps.v2.host_feature_check_version >= 21) {
      vctx->base.clear_render_target = virgl_clear_render_target;
      vctx->base.clear_depth_stencil = virgl_clear_depth_stencil;
   } else {
      // Stub is required by VL backend
      vctx->base.clear_render_target = virgl_clear_render_target_stub;
   }
   vctx->base.clear_texture = virgl_clear_texture;
   vctx->base.draw_vbo = virgl_draw_vbo;
   vctx->base.flush = virgl_flush_from_st;
   vctx->base.screen = pscreen;
   vctx->base.create_sampler_view = virgl_create_sampler_view;
   vctx->base.sampler_view_destroy = virgl_destroy_sampler_view;
   vctx->base.set_sampler_views = virgl_set_sampler_views;
   vctx->base.texture_barrier = virgl_texture_barrier;

   vctx->base.create_sampler_state = virgl_create_sampler_state;
   vctx->base.delete_sampler_state = virgl_delete_sampler_state;
   vctx->base.bind_sampler_states = virgl_bind_sampler_states;

   vctx->base.set_polygon_stipple = virgl_set_polygon_stipple;
   vctx->base.set_scissor_states = virgl_set_scissor_states;
   vctx->base.set_sample_mask = virgl_set_sample_mask;
   vctx->base.set_min_samples = virgl_set_min_samples;
   vctx->base.set_stencil_ref = virgl_set_stencil_ref;
   vctx->base.set_clip_state = virgl_set_clip_state;

   vctx->base.set_blend_color = virgl_set_blend_color;

   vctx->base.get_sample_position = virgl_get_sample_position;

   vctx->base.resource_copy_region = virgl_resource_copy_region;
   vctx->base.flush_resource = virgl_flush_resource;
   vctx->base.blit =  virgl_blit;
   vctx->base.create_fence_fd = virgl_create_fence_fd;
   vctx->base.fence_server_sync = virgl_fence_server_sync;

   vctx->base.set_shader_buffers = virgl_set_shader_buffers;
   vctx->base.set_hw_atomic_buffers = virgl_set_hw_atomic_buffers;
   vctx->base.set_shader_images = virgl_set_shader_images;
   vctx->base.memory_barrier = virgl_memory_barrier;
   vctx->base.emit_string_marker = virgl_emit_string_marker;

   vctx->base.create_video_codec = virgl_video_create_codec;
   vctx->base.create_video_buffer = virgl_video_create_buffer;

   if (rs->caps.caps.v2.host_feature_check_version >= 7)
      vctx->base.link_shader = virgl_link_shader;

   virgl_init_context_resource_functions(&vctx->base);
   virgl_init_query_functions(vctx);
   virgl_init_so_functions(vctx);

   slab_create_child(&vctx->transfer_pool, &rs->transfer_pool);
   virgl_transfer_queue_init(&vctx->queue, vctx);
   vctx->encoded_transfers = (rs->vws->supports_encoded_transfers &&
                       (rs->caps.caps.v2.capability_bits & VIRGL_CAP_TRANSFER));

   /* Reserve some space for transfers. */
   if (vctx->encoded_transfers)
      vctx->cbuf->cdw = VIRGL_MAX_TBUF_DWORDS;

   vctx->primconvert = util_primconvert_create(&vctx->base, rs->caps.caps.v1.prim_mask);
   vctx->uploader = u_upload_create(&vctx->base, 1024 * 1024,
                                     PIPE_BIND_INDEX_BUFFER, PIPE_USAGE_STREAM, 0);
   if (!vctx->uploader)
           goto fail;
   vctx->base.stream_uploader = vctx->uploader;
   vctx->base.const_uploader = vctx->uploader;

   /* We use a special staging buffer as the source of copy transfers. */
   if ((rs->caps.caps.v2.capability_bits & VIRGL_CAP_COPY_TRANSFER) &&
       vctx->encoded_transfers) {
      virgl_staging_init(&vctx->staging, &vctx->base, 1024 * 1024);
      vctx->supports_staging = true;
   }

   vctx->hw_sub_ctx_id = p_atomic_inc_return(&rs->sub_ctx_id);
   virgl_encoder_create_sub_ctx(vctx, vctx->hw_sub_ctx_id);

   virgl_encoder_set_sub_ctx(vctx, vctx->hw_sub_ctx_id);

   if (rs->caps.caps.v2.capability_bits & VIRGL_CAP_GUEST_MAY_INIT_LOG) {
      host_debug_flagstring = getenv("VIRGL_HOST_DEBUG");
      if (host_debug_flagstring)
         virgl_encode_host_debug_flagstring(vctx, host_debug_flagstring);
   }

   if (rs->caps.caps.v2.capability_bits & VIRGL_CAP_APP_TWEAK_SUPPORT)
      virgl_send_tweaks(vctx, rs);

   /* On Android, a virgl_screen is generally created first by the HWUI
    * service, followed by the application's no-op attempt to do the same with
    * eglInitialize(). To retain the ability for apps to set their own driver
    * config procedurally right before context creation, we must check the
    * envvar again.
    */
#if DETECT_OS_ANDROID
   if (!rs->shader_sync) {
      uint64_t debug_options = debug_get_flags_option("VIRGL_DEBUG",
                                                      virgl_debug_options, 0);
      rs->shader_sync |= !!(debug_options & VIRGL_DEBUG_SHADER_SYNC);
   }
#endif

   return &vctx->base;
fail:
   virgl_context_destroy(&vctx->base);
   return NULL;
}
