/*
 * Copyright © 2014-2018 NVIDIA Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <inttypes.h>
#include <stdlib.h>

#include "util/u_debug.h"
#include "util/u_draw.h"
#include "util/u_inlines.h"
#include "util/u_upload_mgr.h"

#include "tegra_context.h"
#include "tegra_resource.h"
#include "tegra_screen.h"

static void
tegra_destroy(struct pipe_context *pcontext)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   if (context->base.stream_uploader)
      u_upload_destroy(context->base.stream_uploader);

   context->gpu->destroy(context->gpu);
   free(context);
}

static void
tegra_draw_vbo(struct pipe_context *pcontext,
               const struct pipe_draw_info *pinfo,
               unsigned drawid_offset,
               const struct pipe_draw_indirect_info *pindirect,
               const struct pipe_draw_start_count_bias *draws,
               unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pcontext, pinfo, drawid_offset, pindirect, draws, num_draws);
      return;
   }

   if (!pindirect && (!draws[0].count || !pinfo->instance_count))
      return;

   struct tegra_context *context = to_tegra_context(pcontext);
   struct pipe_draw_indirect_info indirect;
   struct pipe_draw_info info;

   if (pinfo && ((pindirect && pindirect->buffer) || pinfo->index_size)) {
      memcpy(&info, pinfo, sizeof(info));

      if (pindirect && pindirect->buffer) {
         memcpy(&indirect, pindirect, sizeof(indirect));
         indirect.buffer = tegra_resource_unwrap(pindirect->buffer);
         indirect.indirect_draw_count = tegra_resource_unwrap(pindirect->indirect_draw_count);
         pindirect = &indirect;
      }

      if (pinfo->index_size && !pinfo->has_user_indices)
         info.index.resource = tegra_resource_unwrap(info.index.resource);

      pinfo = &info;
   }

   context->gpu->draw_vbo(context->gpu, pinfo, drawid_offset, pindirect, draws, num_draws);
}

static void
tegra_render_condition(struct pipe_context *pcontext,
                       struct pipe_query *query,
                       bool condition,
                       unsigned int mode)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->render_condition(context->gpu, query, condition, mode);
}

static struct pipe_query *
tegra_create_query(struct pipe_context *pcontext, unsigned int query_type,
                   unsigned int index)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_query(context->gpu, query_type, index);
}

static struct pipe_query *
tegra_create_batch_query(struct pipe_context *pcontext,
                         unsigned int num_queries,
                         unsigned int *queries)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_batch_query(context->gpu, num_queries,
                                           queries);
}

static void
tegra_destroy_query(struct pipe_context *pcontext, struct pipe_query *query)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->destroy_query(context->gpu, query);
}

static bool
tegra_begin_query(struct pipe_context *pcontext, struct pipe_query *query)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->begin_query(context->gpu, query);
}

static bool
tegra_end_query(struct pipe_context *pcontext, struct pipe_query *query)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->end_query(context->gpu, query);
}

static bool
tegra_get_query_result(struct pipe_context *pcontext,
                       struct pipe_query *query,
                       bool wait,
                       union pipe_query_result *result)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->get_query_result(context->gpu, query, wait,
                     result);
}

static void
tegra_get_query_result_resource(struct pipe_context *pcontext,
                                struct pipe_query *query,
                                enum pipe_query_flags flags,
                                enum pipe_query_value_type result_type,
                                int index,
                                struct pipe_resource *resource,
                                unsigned int offset)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->get_query_result_resource(context->gpu, query, flags,
                                           result_type, index, resource,
                                           offset);
}

static void
tegra_set_active_query_state(struct pipe_context *pcontext, bool enable)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_active_query_state(context->gpu, enable);
}

static void *
tegra_create_blend_state(struct pipe_context *pcontext,
                         const struct pipe_blend_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_blend_state(context->gpu, cso);
}

static void
tegra_bind_blend_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_blend_state(context->gpu, so);
}

static void
tegra_delete_blend_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_blend_state(context->gpu, so);
}

static void *
tegra_create_sampler_state(struct pipe_context *pcontext,
                           const struct pipe_sampler_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_sampler_state(context->gpu, cso);
}

static void
tegra_bind_sampler_states(struct pipe_context *pcontext, enum pipe_shader_type shader,
                          unsigned start_slot, unsigned num_samplers,
                          void **samplers)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_sampler_states(context->gpu, shader, start_slot,
                                     num_samplers, samplers);
}

static void
tegra_delete_sampler_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_sampler_state(context->gpu, so);
}

static void *
tegra_create_rasterizer_state(struct pipe_context *pcontext,
                              const struct pipe_rasterizer_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_rasterizer_state(context->gpu, cso);
}

static void
tegra_bind_rasterizer_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_rasterizer_state(context->gpu, so);
}

static void
tegra_delete_rasterizer_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_rasterizer_state(context->gpu, so);
}

static void *
tegra_create_depth_stencil_alpha_state(struct pipe_context *pcontext,
                                       const struct pipe_depth_stencil_alpha_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_depth_stencil_alpha_state(context->gpu, cso);
}

static void
tegra_bind_depth_stencil_alpha_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_depth_stencil_alpha_state(context->gpu, so);
}

static void
tegra_delete_depth_stencil_alpha_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_depth_stencil_alpha_state(context->gpu, so);
}

static void *
tegra_create_fs_state(struct pipe_context *pcontext,
                      const struct pipe_shader_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_fs_state(context->gpu, cso);
}

static void
tegra_bind_fs_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_fs_state(context->gpu, so);
}

static void
tegra_delete_fs_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_fs_state(context->gpu, so);
}

static void *
tegra_create_vs_state(struct pipe_context *pcontext,
                      const struct pipe_shader_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_vs_state(context->gpu, cso);
}

static void
tegra_bind_vs_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_vs_state(context->gpu, so);
}

static void
tegra_delete_vs_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_vs_state(context->gpu, so);
}

static void *
tegra_create_gs_state(struct pipe_context *pcontext,
                      const struct pipe_shader_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_gs_state(context->gpu, cso);
}

static void
tegra_bind_gs_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_gs_state(context->gpu, so);
}

static void
tegra_delete_gs_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_gs_state(context->gpu, so);
}

static void *
tegra_create_tcs_state(struct pipe_context *pcontext,
                       const struct pipe_shader_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_tcs_state(context->gpu, cso);
}

static void
tegra_bind_tcs_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_tcs_state(context->gpu, so);
}

static void
tegra_delete_tcs_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_tcs_state(context->gpu, so);
}

static void *
tegra_create_tes_state(struct pipe_context *pcontext,
                       const struct pipe_shader_state *cso)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_tes_state(context->gpu, cso);
}

static void
tegra_bind_tes_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_tes_state(context->gpu, so);
}

static void
tegra_delete_tes_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_tes_state(context->gpu, so);
}

static void *
tegra_create_vertex_elements_state(struct pipe_context *pcontext,
                                   unsigned num_elements,
                                   const struct pipe_vertex_element *elements)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_vertex_elements_state(context->gpu,
                                                     num_elements,
                                                     elements);
}

static void
tegra_bind_vertex_elements_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_vertex_elements_state(context->gpu, so);
}

static void
tegra_delete_vertex_elements_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_vertex_elements_state(context->gpu, so);
}

static void
tegra_set_blend_color(struct pipe_context *pcontext,
                      const struct pipe_blend_color *color)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_blend_color(context->gpu, color);
}

static void
tegra_set_stencil_ref(struct pipe_context *pcontext,
                      const struct pipe_stencil_ref ref)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_stencil_ref(context->gpu, ref);
}

static void
tegra_set_sample_mask(struct pipe_context *pcontext, unsigned int mask)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_sample_mask(context->gpu, mask);
}

static void
tegra_set_min_samples(struct pipe_context *pcontext, unsigned int samples)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_min_samples(context->gpu, samples);
}

static void
tegra_set_clip_state(struct pipe_context *pcontext,
                     const struct pipe_clip_state *state)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_clip_state(context->gpu, state);
}

static void
tegra_set_constant_buffer(struct pipe_context *pcontext, enum pipe_shader_type shader,
                          unsigned int index, bool take_ownership,
                          const struct pipe_constant_buffer *buf)
{
   struct tegra_context *context = to_tegra_context(pcontext);
   struct pipe_constant_buffer buffer;

   if (buf && buf->buffer) {
      memcpy(&buffer, buf, sizeof(buffer));
      buffer.buffer = tegra_resource_unwrap(buffer.buffer);
      buf = &buffer;
   }

   context->gpu->set_constant_buffer(context->gpu, shader, index, take_ownership, buf);
}

static void
tegra_set_framebuffer_state(struct pipe_context *pcontext,
                            const struct pipe_framebuffer_state *fb)
{
   struct tegra_context *context = to_tegra_context(pcontext);
   struct pipe_framebuffer_state state;
   unsigned i;

   if (fb) {
      memcpy(&state, fb, sizeof(state));

      for (i = 0; i < fb->nr_cbufs; i++)
         state.cbufs[i] = tegra_surface_unwrap(fb->cbufs[i]);

      while (i < PIPE_MAX_COLOR_BUFS)
         state.cbufs[i++] = NULL;

      state.zsbuf = tegra_surface_unwrap(fb->zsbuf);

      fb = &state;
   }

   context->gpu->set_framebuffer_state(context->gpu, fb);
}

static void
tegra_set_polygon_stipple(struct pipe_context *pcontext,
                          const struct pipe_poly_stipple *stipple)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_polygon_stipple(context->gpu, stipple);
}

static void
tegra_set_scissor_states(struct pipe_context *pcontext, unsigned start_slot,
                         unsigned num_scissors,
                         const struct pipe_scissor_state *scissors)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_scissor_states(context->gpu, start_slot, num_scissors,
                                    scissors);
}

static void
tegra_set_window_rectangles(struct pipe_context *pcontext, bool include,
                            unsigned int num_rectangles,
                            const struct pipe_scissor_state *rectangles)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_window_rectangles(context->gpu, include, num_rectangles,
                                       rectangles);
}

static void
tegra_set_viewport_states(struct pipe_context *pcontext, unsigned start_slot,
                          unsigned num_viewports,
                          const struct pipe_viewport_state *viewports)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_viewport_states(context->gpu, start_slot, num_viewports,
                                     viewports);
}

static void
tegra_set_sampler_views(struct pipe_context *pcontext, enum pipe_shader_type shader,
                        unsigned start_slot, unsigned num_views,
                        unsigned unbind_num_trailing_slots,
                        bool take_ownership,
                        struct pipe_sampler_view **pviews)
{
   struct pipe_sampler_view *views[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_sampler_view *view;
   unsigned i;

   for (i = 0; i < num_views; i++) {
      /* adjust private reference count */
      view = to_tegra_sampler_view(pviews[i]);
      if (view) {
         view->refcount--;
         if (!view->refcount) {
            view->refcount = 100000000;
            p_atomic_add(&view->gpu->reference.count, view->refcount);
         }
      }

      views[i] = tegra_sampler_view_unwrap(pviews[i]);
   }

   context->gpu->set_sampler_views(context->gpu, shader, start_slot,
                                   num_views, unbind_num_trailing_slots,
                                   take_ownership, views);
}

static void
tegra_set_tess_state(struct pipe_context *pcontext,
                     const float default_outer_level[4],
                     const float default_inner_level[2])
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_tess_state(context->gpu, default_outer_level,
                                default_inner_level);
}

static void
tegra_set_debug_callback(struct pipe_context *pcontext,
                         const struct util_debug_callback *callback)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_debug_callback(context->gpu, callback);
}

static void
tegra_set_shader_buffers(struct pipe_context *pcontext, enum pipe_shader_type shader,
                         unsigned start, unsigned count,
                         const struct pipe_shader_buffer *buffers,
                         unsigned writable_bitmask)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_shader_buffers(context->gpu, shader, start, count,
                                    buffers, writable_bitmask);
}

static void
tegra_set_shader_images(struct pipe_context *pcontext, enum pipe_shader_type shader,
                        unsigned start, unsigned count,
                        unsigned unbind_num_trailing_slots,
                        const struct pipe_image_view *images)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_shader_images(context->gpu, shader, start, count,
                                   unbind_num_trailing_slots, images);
}

static void
tegra_set_vertex_buffers(struct pipe_context *pcontext,
                         unsigned num_buffers, unsigned unbind_num_trailing_slots,
                         bool take_ownership,
                         const struct pipe_vertex_buffer *buffers)
{
   struct tegra_context *context = to_tegra_context(pcontext);
   struct pipe_vertex_buffer buf[PIPE_MAX_SHADER_INPUTS];
   unsigned i;

   if (num_buffers && buffers) {
      memcpy(buf, buffers, num_buffers * sizeof(struct pipe_vertex_buffer));

      for (i = 0; i < num_buffers; i++) {
         if (!buf[i].is_user_buffer)
            buf[i].buffer.resource = tegra_resource_unwrap(buf[i].buffer.resource);
      }

      buffers = buf;
   }

   context->gpu->set_vertex_buffers(context->gpu, num_buffers,
                                    unbind_num_trailing_slots,
                                    take_ownership, buffers);
}

static struct pipe_stream_output_target *
tegra_create_stream_output_target(struct pipe_context *pcontext,
                                  struct pipe_resource *presource,
                                  unsigned buffer_offset,
                                  unsigned buffer_size)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_stream_output_target(context->gpu,
                                                    resource->gpu,
                                                    buffer_offset,
                                                    buffer_size);
}

static void
tegra_stream_output_target_destroy(struct pipe_context *pcontext,
                                   struct pipe_stream_output_target *target)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->stream_output_target_destroy(context->gpu, target);
}

static void
tegra_set_stream_output_targets(struct pipe_context *pcontext,
                                unsigned num_targets,
                                struct pipe_stream_output_target **targets,
                                const unsigned *offsets)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_stream_output_targets(context->gpu, num_targets,
                                           targets, offsets);
}

static void
tegra_resource_copy_region(struct pipe_context *pcontext,
                           struct pipe_resource *pdst,
                           unsigned int dst_level,
                           unsigned int dstx,
                           unsigned int dsty,
                           unsigned int dstz,
                           struct pipe_resource *psrc,
                           unsigned int src_level,
                           const struct pipe_box *src_box)
{
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_resource *dst = to_tegra_resource(pdst);
   struct tegra_resource *src = to_tegra_resource(psrc);

   context->gpu->resource_copy_region(context->gpu, dst->gpu, dst_level, dstx,
                                      dsty, dstz, src->gpu, src_level,
                                      src_box);
}

static void
tegra_blit(struct pipe_context *pcontext, const struct pipe_blit_info *pinfo)
{
   struct tegra_context *context = to_tegra_context(pcontext);
   struct pipe_blit_info info;

   if (pinfo) {
      memcpy(&info, pinfo, sizeof(info));
      info.dst.resource = tegra_resource_unwrap(info.dst.resource);
      info.src.resource = tegra_resource_unwrap(info.src.resource);
      pinfo = &info;
   }

   context->gpu->blit(context->gpu, pinfo);
}

static void
tegra_clear(struct pipe_context *pcontext, unsigned buffers, const struct pipe_scissor_state *scissor_state,
            const union pipe_color_union *color, double depth,
            unsigned stencil)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->clear(context->gpu, buffers, NULL, color, depth, stencil);
}

static void
tegra_clear_render_target(struct pipe_context *pcontext,
                          struct pipe_surface *pdst,
                          const union pipe_color_union *color,
                          unsigned int dstx,
                          unsigned int dsty,
                          unsigned int width,
                          unsigned int height,
                          bool render_condition)
{
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_surface *dst = to_tegra_surface(pdst);

   context->gpu->clear_render_target(context->gpu, dst->gpu, color, dstx,
                                     dsty, width, height, render_condition);
}

static void
tegra_clear_depth_stencil(struct pipe_context *pcontext,
                          struct pipe_surface *pdst,
                          unsigned int flags,
                          double depth,
                          unsigned int stencil,
                          unsigned int dstx,
                          unsigned int dsty,
                          unsigned int width,
                          unsigned int height,
                          bool render_condition)
{
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_surface *dst = to_tegra_surface(pdst);

   context->gpu->clear_depth_stencil(context->gpu, dst->gpu, flags, depth,
                                     stencil, dstx, dsty, width, height,
                                     render_condition);
}

static void
tegra_clear_texture(struct pipe_context *pcontext,
                    struct pipe_resource *presource,
                    unsigned int level,
                    const struct pipe_box *box,
                    const void *data)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->clear_texture(context->gpu, resource->gpu, level, box, data);
}

static void
tegra_clear_buffer(struct pipe_context *pcontext,
                   struct pipe_resource *presource,
                   unsigned int offset,
                   unsigned int size,
                   const void *value,
                   int value_size)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->clear_buffer(context->gpu, resource->gpu, offset, size,
                              value, value_size);
}

static void
tegra_flush(struct pipe_context *pcontext, struct pipe_fence_handle **fence,
            unsigned flags)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->flush(context->gpu, fence, flags);
}

static void
tegra_create_fence_fd(struct pipe_context *pcontext,
                      struct pipe_fence_handle **fence,
                      int fd, enum pipe_fd_type type)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   assert(type == PIPE_FD_TYPE_NATIVE_SYNC);
   context->gpu->create_fence_fd(context->gpu, fence, fd, type);
}

static void
tegra_fence_server_sync(struct pipe_context *pcontext,
                        struct pipe_fence_handle *fence)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->fence_server_sync(context->gpu, fence);
}

static struct pipe_sampler_view *
tegra_create_sampler_view(struct pipe_context *pcontext,
                          struct pipe_resource *presource,
                          const struct pipe_sampler_view *template)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_sampler_view *view;

   view = calloc(1, sizeof(*view));
   if (!view)
      return NULL;

   view->base = *template;
   view->base.context = pcontext;
   /* overwrite to prevent reference from being released */
   view->base.texture = NULL;
   pipe_reference_init(&view->base.reference, 1);
   pipe_resource_reference(&view->base.texture, presource);

   view->gpu = context->gpu->create_sampler_view(context->gpu, resource->gpu,
                                                 template);

   /* use private reference count */
   view->gpu->reference.count += 100000000;
   view->refcount = 100000000;

   return &view->base;
}

static void
tegra_sampler_view_destroy(struct pipe_context *pcontext,
                           struct pipe_sampler_view *pview)
{
   struct tegra_sampler_view *view = to_tegra_sampler_view(pview);

   pipe_resource_reference(&view->base.texture, NULL);
   /* adjust private reference count */
   p_atomic_add(&view->gpu->reference.count, -view->refcount);
   pipe_sampler_view_reference(&view->gpu, NULL);
   free(view);
}

static struct pipe_surface *
tegra_create_surface(struct pipe_context *pcontext,
                     struct pipe_resource *presource,
                     const struct pipe_surface *template)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_surface *surface;

   surface = calloc(1, sizeof(*surface));
   if (!surface)
      return NULL;

   surface->gpu = context->gpu->create_surface(context->gpu, resource->gpu,
                                               template);
   if (!surface->gpu) {
      free(surface);
      return NULL;
   }

   memcpy(&surface->base, surface->gpu, sizeof(*surface->gpu));
   /* overwrite to prevent reference from being released */
   surface->base.texture = NULL;

   pipe_reference_init(&surface->base.reference, 1);
   pipe_resource_reference(&surface->base.texture, presource);
   surface->base.context = &context->base;

   return &surface->base;
}

static void
tegra_surface_destroy(struct pipe_context *pcontext,
                      struct pipe_surface *psurface)
{
   struct tegra_surface *surface = to_tegra_surface(psurface);

   pipe_resource_reference(&surface->base.texture, NULL);
   pipe_surface_reference(&surface->gpu, NULL);
   free(surface);
}

static void *
tegra_transfer_map(struct pipe_context *pcontext,
                   struct pipe_resource *presource,
                   unsigned level, unsigned usage,
                   const struct pipe_box *box,
                   struct pipe_transfer **ptransfer)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);
   struct tegra_transfer *transfer;

   transfer = calloc(1, sizeof(*transfer));
   if (!transfer)
      return NULL;

   if (presource->target == PIPE_BUFFER) {
      transfer->map = context->gpu->buffer_map(context->gpu, resource->gpu,
                                                 level, usage, box,
                                                 &transfer->gpu);
   } else {
      transfer->map = context->gpu->texture_map(context->gpu, resource->gpu,
                                                 level, usage, box,
                                                 &transfer->gpu);
   }
   memcpy(&transfer->base, transfer->gpu, sizeof(*transfer->gpu));
   transfer->base.resource = NULL;
   pipe_resource_reference(&transfer->base.resource, presource);

   *ptransfer = &transfer->base;

   return transfer->map;
}

static void
tegra_transfer_flush_region(struct pipe_context *pcontext,
                            struct pipe_transfer *ptransfer,
                            const struct pipe_box *box)
{
   struct tegra_transfer *transfer = to_tegra_transfer(ptransfer);
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->transfer_flush_region(context->gpu, transfer->gpu, box);
}

static void
tegra_transfer_unmap(struct pipe_context *pcontext,
                     struct pipe_transfer *ptransfer)
{
   struct tegra_transfer *transfer = to_tegra_transfer(ptransfer);
   struct tegra_context *context = to_tegra_context(pcontext);

   if (ptransfer->resource->target == PIPE_BUFFER)
      context->gpu->buffer_unmap(context->gpu, transfer->gpu);
   else
      context->gpu->texture_unmap(context->gpu, transfer->gpu);
   pipe_resource_reference(&transfer->base.resource, NULL);
   free(transfer);
}

static void
tegra_buffer_subdata(struct pipe_context *pcontext,
                     struct pipe_resource *presource,
                     unsigned usage, unsigned offset,
                     unsigned size, const void *data)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->buffer_subdata(context->gpu, resource->gpu, usage, offset,
                                size, data);
}

static void
tegra_texture_subdata(struct pipe_context *pcontext,
                      struct pipe_resource *presource,
                      unsigned level,
                      unsigned usage,
                      const struct pipe_box *box,
                      const void *data,
                      unsigned stride,
                      uintptr_t layer_stride)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->texture_subdata(context->gpu, resource->gpu, level, usage,
                                 box, data, stride, layer_stride);
}

static void
tegra_texture_barrier(struct pipe_context *pcontext, unsigned int flags)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->texture_barrier(context->gpu, flags);
}

static void
tegra_memory_barrier(struct pipe_context *pcontext, unsigned int flags)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   if (!(flags & ~PIPE_BARRIER_UPDATE))
      return;

   context->gpu->memory_barrier(context->gpu, flags);
}

static struct pipe_video_codec *
tegra_create_video_codec(struct pipe_context *pcontext,
                         const struct pipe_video_codec *template)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_video_codec(context->gpu, template);
}

static struct pipe_video_buffer *
tegra_create_video_buffer(struct pipe_context *pcontext,
                          const struct pipe_video_buffer *template)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_video_buffer(context->gpu, template);
}

static void *
tegra_create_compute_state(struct pipe_context *pcontext,
                           const struct pipe_compute_state *template)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_compute_state(context->gpu, template);
}

static void
tegra_bind_compute_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->bind_compute_state(context->gpu, so);
}

static void
tegra_delete_compute_state(struct pipe_context *pcontext, void *so)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_compute_state(context->gpu, so);
}

static void
tegra_set_compute_resources(struct pipe_context *pcontext,
                            unsigned int start, unsigned int count,
                            struct pipe_surface **resources)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   /* XXX unwrap resources */

   context->gpu->set_compute_resources(context->gpu, start, count, resources);
}

static void
tegra_set_global_binding(struct pipe_context *pcontext, unsigned int first,
                         unsigned int count, struct pipe_resource **resources,
                         uint32_t **handles)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   /* XXX unwrap resources */

   context->gpu->set_global_binding(context->gpu, first, count, resources,
                                    handles);
}

static void
tegra_launch_grid(struct pipe_context *pcontext,
                  const struct pipe_grid_info *info)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   /* XXX unwrap info->indirect? */

   context->gpu->launch_grid(context->gpu, info);
}

static void
tegra_get_sample_position(struct pipe_context *pcontext, unsigned int count,
                          unsigned int index, float *value)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->get_sample_position(context->gpu, count, index, value);
}

static uint64_t
tegra_get_timestamp(struct pipe_context *pcontext)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->get_timestamp(context->gpu);
}

static void
tegra_flush_resource(struct pipe_context *pcontext,
                     struct pipe_resource *presource)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->flush_resource(context->gpu, resource->gpu);
}

static void
tegra_invalidate_resource(struct pipe_context *pcontext,
                          struct pipe_resource *presource)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->invalidate_resource(context->gpu, resource->gpu);
}

static enum pipe_reset_status
tegra_get_device_reset_status(struct pipe_context *pcontext)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->get_device_reset_status(context->gpu);
}

static void
tegra_set_device_reset_callback(struct pipe_context *pcontext,
                                const struct pipe_device_reset_callback *cb)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->set_device_reset_callback(context->gpu, cb);
}

static void
tegra_dump_debug_state(struct pipe_context *pcontext, FILE *stream,
                       unsigned int flags)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->dump_debug_state(context->gpu, stream, flags);
}

static void
tegra_emit_string_marker(struct pipe_context *pcontext, const char *string,
                         int length)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->emit_string_marker(context->gpu, string, length);
}

static bool
tegra_generate_mipmap(struct pipe_context *pcontext,
                      struct pipe_resource *presource,
                      enum pipe_format format,
                      unsigned int base_level,
                      unsigned int last_level,
                      unsigned int first_layer,
                      unsigned int last_layer)
{
   struct tegra_resource *resource = to_tegra_resource(presource);
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->generate_mipmap(context->gpu, resource->gpu, format,
                                        base_level, last_level, first_layer,
                                        last_layer);
}

static uint64_t
tegra_create_texture_handle(struct pipe_context *pcontext,
                            struct pipe_sampler_view *view,
                            const struct pipe_sampler_state *state)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_texture_handle(context->gpu, view, state);
}

static void tegra_delete_texture_handle(struct pipe_context *pcontext,
                                        uint64_t handle)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_texture_handle(context->gpu, handle);
}

static void tegra_make_texture_handle_resident(struct pipe_context *pcontext,
                                               uint64_t handle, bool resident)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->make_texture_handle_resident(context->gpu, handle, resident);
}

static uint64_t tegra_create_image_handle(struct pipe_context *pcontext,
                                          const struct pipe_image_view *image)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   return context->gpu->create_image_handle(context->gpu, image);
}

static void tegra_delete_image_handle(struct pipe_context *pcontext,
                                      uint64_t handle)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->delete_image_handle(context->gpu, handle);
}

static void tegra_make_image_handle_resident(struct pipe_context *pcontext,
                                             uint64_t handle, unsigned access,
                                             bool resident)
{
   struct tegra_context *context = to_tegra_context(pcontext);

   context->gpu->make_image_handle_resident(context->gpu, handle, access,
                                            resident);
}

struct pipe_context *
tegra_screen_context_create(struct pipe_screen *pscreen, void *priv,
                            unsigned int flags)
{
   struct tegra_screen *screen = to_tegra_screen(pscreen);
   struct tegra_context *context;

   context = calloc(1, sizeof(*context));
   if (!context)
      return NULL;

   context->gpu = screen->gpu->context_create(screen->gpu, priv, flags);
   if (!context->gpu) {
      debug_error("failed to create GPU context\n");
      goto free;
   }

   context->base.screen = &screen->base;
   context->base.priv = priv;

   /*
    * Create custom stream and const uploaders. Note that technically nouveau
    * already creates uploaders that could be reused, but that would make the
    * resource unwrapping rather complicate. The reason for that is that both
    * uploaders create resources based on the context that they were created
    * from, which means that nouveau's uploader will use the nouveau context
    * which means that those resources must not be unwrapped. So before each
    * resource is unwrapped, the code would need to check that it does not
    * correspond to the uploaders' buffers.
    *
    * However, duplicating the uploaders here sounds worse than it is. The
    * default implementation that nouveau uses allocates buffers lazily, and
    * since it is never used, no buffers will every be allocated and the only
    * memory wasted is that occupied by the nouveau uploader itself.
    */
   context->base.stream_uploader = u_upload_create_default(&context->base);
   if (!context->base.stream_uploader)
      goto destroy;

   context->base.const_uploader = context->base.stream_uploader;

   context->base.destroy = tegra_destroy;

   context->base.draw_vbo = tegra_draw_vbo;

   context->base.render_condition = tegra_render_condition;

   context->base.create_query = tegra_create_query;
   context->base.create_batch_query = tegra_create_batch_query;
   context->base.destroy_query = tegra_destroy_query;
   context->base.begin_query = tegra_begin_query;
   context->base.end_query = tegra_end_query;
   context->base.get_query_result = tegra_get_query_result;
   context->base.get_query_result_resource = tegra_get_query_result_resource;
   context->base.set_active_query_state = tegra_set_active_query_state;

   context->base.create_blend_state = tegra_create_blend_state;
   context->base.bind_blend_state = tegra_bind_blend_state;
   context->base.delete_blend_state = tegra_delete_blend_state;

   context->base.create_sampler_state = tegra_create_sampler_state;
   context->base.bind_sampler_states = tegra_bind_sampler_states;
   context->base.delete_sampler_state = tegra_delete_sampler_state;

   context->base.create_rasterizer_state = tegra_create_rasterizer_state;
   context->base.bind_rasterizer_state = tegra_bind_rasterizer_state;
   context->base.delete_rasterizer_state = tegra_delete_rasterizer_state;

   context->base.create_depth_stencil_alpha_state = tegra_create_depth_stencil_alpha_state;
   context->base.bind_depth_stencil_alpha_state = tegra_bind_depth_stencil_alpha_state;
   context->base.delete_depth_stencil_alpha_state = tegra_delete_depth_stencil_alpha_state;

   context->base.create_fs_state = tegra_create_fs_state;
   context->base.bind_fs_state = tegra_bind_fs_state;
   context->base.delete_fs_state = tegra_delete_fs_state;

   context->base.create_vs_state = tegra_create_vs_state;
   context->base.bind_vs_state = tegra_bind_vs_state;
   context->base.delete_vs_state = tegra_delete_vs_state;

   context->base.create_gs_state = tegra_create_gs_state;
   context->base.bind_gs_state = tegra_bind_gs_state;
   context->base.delete_gs_state = tegra_delete_gs_state;

   context->base.create_tcs_state = tegra_create_tcs_state;
   context->base.bind_tcs_state = tegra_bind_tcs_state;
   context->base.delete_tcs_state = tegra_delete_tcs_state;

   context->base.create_tes_state = tegra_create_tes_state;
   context->base.bind_tes_state = tegra_bind_tes_state;
   context->base.delete_tes_state = tegra_delete_tes_state;

   context->base.create_vertex_elements_state = tegra_create_vertex_elements_state;
   context->base.bind_vertex_elements_state = tegra_bind_vertex_elements_state;
   context->base.delete_vertex_elements_state = tegra_delete_vertex_elements_state;

   context->base.set_blend_color = tegra_set_blend_color;
   context->base.set_stencil_ref = tegra_set_stencil_ref;
   context->base.set_sample_mask = tegra_set_sample_mask;
   context->base.set_min_samples = tegra_set_min_samples;
   context->base.set_clip_state = tegra_set_clip_state;

   context->base.set_constant_buffer = tegra_set_constant_buffer;
   context->base.set_framebuffer_state = tegra_set_framebuffer_state;
   context->base.set_polygon_stipple = tegra_set_polygon_stipple;
   context->base.set_scissor_states = tegra_set_scissor_states;
   context->base.set_window_rectangles = tegra_set_window_rectangles;
   context->base.set_viewport_states = tegra_set_viewport_states;
   context->base.set_sampler_views = tegra_set_sampler_views;
   context->base.set_tess_state = tegra_set_tess_state;

   context->base.set_debug_callback = tegra_set_debug_callback;

   context->base.set_shader_buffers = tegra_set_shader_buffers;
   context->base.set_shader_images = tegra_set_shader_images;
   context->base.set_vertex_buffers = tegra_set_vertex_buffers;

   context->base.create_stream_output_target = tegra_create_stream_output_target;
   context->base.stream_output_target_destroy = tegra_stream_output_target_destroy;
   context->base.set_stream_output_targets = tegra_set_stream_output_targets;

   context->base.resource_copy_region = tegra_resource_copy_region;
   context->base.blit = tegra_blit;
   context->base.clear = tegra_clear;
   context->base.clear_render_target = tegra_clear_render_target;
   context->base.clear_depth_stencil = tegra_clear_depth_stencil;
   context->base.clear_texture = tegra_clear_texture;
   context->base.clear_buffer = tegra_clear_buffer;
   context->base.flush = tegra_flush;

   context->base.create_fence_fd = tegra_create_fence_fd;
   context->base.fence_server_sync = tegra_fence_server_sync;

   context->base.create_sampler_view = tegra_create_sampler_view;
   context->base.sampler_view_destroy = tegra_sampler_view_destroy;

   context->base.create_surface = tegra_create_surface;
   context->base.surface_destroy = tegra_surface_destroy;

   context->base.buffer_map = tegra_transfer_map;
   context->base.texture_map = tegra_transfer_map;
   context->base.transfer_flush_region = tegra_transfer_flush_region;
   context->base.buffer_unmap = tegra_transfer_unmap;
   context->base.texture_unmap = tegra_transfer_unmap;
   context->base.buffer_subdata = tegra_buffer_subdata;
   context->base.texture_subdata = tegra_texture_subdata;

   context->base.texture_barrier = tegra_texture_barrier;
   context->base.memory_barrier = tegra_memory_barrier;

   context->base.create_video_codec = tegra_create_video_codec;
   context->base.create_video_buffer = tegra_create_video_buffer;

   context->base.create_compute_state = tegra_create_compute_state;
   context->base.bind_compute_state = tegra_bind_compute_state;
   context->base.delete_compute_state = tegra_delete_compute_state;
   context->base.set_compute_resources = tegra_set_compute_resources;
   context->base.set_global_binding = tegra_set_global_binding;
   context->base.launch_grid = tegra_launch_grid;
   context->base.get_sample_position = tegra_get_sample_position;
   context->base.get_timestamp = tegra_get_timestamp;

   context->base.flush_resource = tegra_flush_resource;
   context->base.invalidate_resource = tegra_invalidate_resource;

   context->base.get_device_reset_status = tegra_get_device_reset_status;
   context->base.set_device_reset_callback = tegra_set_device_reset_callback;
   context->base.dump_debug_state = tegra_dump_debug_state;
   context->base.emit_string_marker = tegra_emit_string_marker;

   context->base.generate_mipmap = tegra_generate_mipmap;

   context->base.create_texture_handle = tegra_create_texture_handle;
   context->base.delete_texture_handle = tegra_delete_texture_handle;
   context->base.make_texture_handle_resident = tegra_make_texture_handle_resident;
   context->base.create_image_handle = tegra_create_image_handle;
   context->base.delete_image_handle = tegra_delete_image_handle;
   context->base.make_image_handle_resident = tegra_make_image_handle_resident;

   return &context->base;

destroy:
   context->gpu->destroy(context->gpu);
free:
   free(context);
   return NULL;
}
