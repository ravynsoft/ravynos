/*
 * Copyright 2010 Red Hat Inc.
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
#include <stdio.h>
#include <errno.h>
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_transfer.h"

static void noop_draw_vbo(struct pipe_context *ctx, const struct pipe_draw_info *info,
                          unsigned drawid_offset,
                          const struct pipe_draw_indirect_info *indirect,
                          const struct pipe_draw_start_count_bias *draws,
                          unsigned num_draws)
{
}

static void noop_draw_vertex_state(struct pipe_context *ctx,
                                   struct pipe_vertex_state *state,
                                   uint32_t partial_velem_mask,
                                   struct pipe_draw_vertex_state_info info,
                                   const struct pipe_draw_start_count_bias *draws,
                                   unsigned num_draws)
{
}

static void noop_launch_grid(struct pipe_context *ctx,
                             const struct pipe_grid_info *info)
{
}

static void noop_set_blend_color(struct pipe_context *ctx,
                                 const struct pipe_blend_color *state)
{
}

static void *noop_create_blend_state(struct pipe_context *ctx,
                                     const struct pipe_blend_state *state)
{
   return MALLOC(1);
}

static void *noop_create_dsa_state(struct pipe_context *ctx,
                                   const struct pipe_depth_stencil_alpha_state *state)
{
   return MALLOC(1);
}

static void *noop_create_rs_state(struct pipe_context *ctx,
                                  const struct pipe_rasterizer_state *state)
{
   return MALLOC(1);
}

static void *noop_create_sampler_state(struct pipe_context *ctx,
                                       const struct pipe_sampler_state *state)
{
   return MALLOC(1);
}

static struct pipe_sampler_view *noop_create_sampler_view(struct pipe_context *ctx,
                                                          struct pipe_resource *texture,
                                                          const struct pipe_sampler_view *state)
{
   struct pipe_sampler_view *sampler_view = CALLOC_STRUCT(pipe_sampler_view);

   if (!sampler_view)
      return NULL;

   /* initialize base object */
   *sampler_view = *state;
   sampler_view->texture = NULL;
   pipe_resource_reference(&sampler_view->texture, texture);
   pipe_reference_init(&sampler_view->reference, 1);
   sampler_view->context = ctx;
   return sampler_view;
}

static struct pipe_surface *noop_create_surface(struct pipe_context *ctx,
                                                struct pipe_resource *texture,
                                                const struct pipe_surface *surf_tmpl)
{
   struct pipe_surface *surface = CALLOC_STRUCT(pipe_surface);

   if (!surface)
      return NULL;
   pipe_reference_init(&surface->reference, 1);
   pipe_resource_reference(&surface->texture, texture);
   surface->context = ctx;
   surface->format = surf_tmpl->format;
   surface->width = texture->width0;
   surface->height = texture->height0;
   surface->texture = texture;
   surface->u.tex.first_layer = surf_tmpl->u.tex.first_layer;
   surface->u.tex.last_layer = surf_tmpl->u.tex.last_layer;
   surface->u.tex.level = surf_tmpl->u.tex.level;

   return surface;
}

static void noop_set_sampler_views(struct pipe_context *ctx,
                                   enum pipe_shader_type shader,
                                   unsigned start, unsigned count,
                                   unsigned unbind_num_trailing_slots,
                                   bool take_ownership,
                                   struct pipe_sampler_view **views)
{
}

static void noop_bind_sampler_states(struct pipe_context *ctx,
                                     enum pipe_shader_type shader,
                                     unsigned start, unsigned count,
                                     void **states)
{
}

static void noop_set_clip_state(struct pipe_context *ctx,
                                const struct pipe_clip_state *state)
{
}

static void noop_set_polygon_stipple(struct pipe_context *ctx,
                                     const struct pipe_poly_stipple *state)
{
}

static void noop_set_sample_mask(struct pipe_context *pipe, unsigned sample_mask)
{
}

static void noop_set_scissor_states(struct pipe_context *ctx,
                                    unsigned start_slot,
                                    unsigned num_scissors,
                                    const struct pipe_scissor_state *state)
{
}

static void noop_set_stencil_ref(struct pipe_context *ctx,
                                 const struct pipe_stencil_ref state)
{
}

static void noop_set_viewport_states(struct pipe_context *ctx,
                                     unsigned start_slot,
                                     unsigned num_viewports,
                                     const struct pipe_viewport_state *state)
{
}

static void noop_set_framebuffer_state(struct pipe_context *ctx,
                                       const struct pipe_framebuffer_state *state)
{
}

static void noop_set_constant_buffer(struct pipe_context *ctx,
                                     enum pipe_shader_type shader, uint index,
                                     bool take_ownership,
                                     const struct pipe_constant_buffer *cb)
{
}

static void noop_set_inlinable_constants(struct pipe_context *ctx,
                                         enum pipe_shader_type shader,
                                         uint num_values, uint32_t *values)
{
}


static void noop_sampler_view_destroy(struct pipe_context *ctx,
                                      struct pipe_sampler_view *state)
{
   pipe_resource_reference(&state->texture, NULL);
   FREE(state);
}


static void noop_surface_destroy(struct pipe_context *ctx,
                                 struct pipe_surface *surface)
{
   pipe_resource_reference(&surface->texture, NULL);
   FREE(surface);
}

static void noop_bind_state(struct pipe_context *ctx, void *state)
{
}

static void noop_delete_state(struct pipe_context *ctx, void *state)
{
   FREE(state);
}

static void noop_set_vertex_buffers(struct pipe_context *ctx,
                                    unsigned count,
                                    unsigned unbind_num_trailing_slots,
                                    bool take_ownership,
                                    const struct pipe_vertex_buffer *buffers)
{
}

static void *noop_create_vertex_elements(struct pipe_context *ctx,
                                         unsigned count,
                                         const struct pipe_vertex_element *state)
{
   return MALLOC(1);
}

static void *noop_create_shader_state(struct pipe_context *ctx,
                                      const struct pipe_shader_state *state)
{
   return MALLOC(1);
}

static void *noop_create_compute_state(struct pipe_context *ctx,
                                       const struct pipe_compute_state *state)
{
   return MALLOC(1);
}

static struct pipe_stream_output_target *noop_create_stream_output_target(
      struct pipe_context *ctx,
      struct pipe_resource *res,
      unsigned buffer_offset,
      unsigned buffer_size)
{
   struct pipe_stream_output_target *t = CALLOC_STRUCT(pipe_stream_output_target);
   if (!t)
      return NULL;

   pipe_reference_init(&t->reference, 1);
   pipe_resource_reference(&t->buffer, res);
   t->buffer_offset = buffer_offset;
   t->buffer_size = buffer_size;
   return t;
}

static void noop_stream_output_target_destroy(struct pipe_context *ctx,
                                              struct pipe_stream_output_target *t)
{
   pipe_resource_reference(&t->buffer, NULL);
   FREE(t);
}

static void noop_set_stream_output_targets(struct pipe_context *ctx,
                                           unsigned num_targets,
                                           struct pipe_stream_output_target **targets,
                                           const unsigned *offsets)
{
}

static void noop_set_window_rectangles(struct pipe_context *ctx,
                                       bool include,
                                       unsigned num_rectangles,
                                       const struct pipe_scissor_state *rects)
{
}

static void noop_set_shader_buffers(struct pipe_context *ctx,
                                    enum pipe_shader_type shader,
                                    unsigned start_slot, unsigned count,
                                    const struct pipe_shader_buffer *buffers,
                                    unsigned writable_bitmask)
{
}

static void noop_set_shader_images(struct pipe_context *ctx,
                                   enum pipe_shader_type shader,
                                   unsigned start_slot, unsigned count,
                                   unsigned unbind_num_trailing_slots,
                                   const struct pipe_image_view *images)
{
}

static void noop_render_condition( struct pipe_context *pipe,
                                   struct pipe_query *query,
                                   bool condition,
                                   enum pipe_render_cond_flag mode )
{
}

static void noop_get_query_result_resource(struct pipe_context *pipe,
                                           struct pipe_query *q,
                                           enum pipe_query_flags flags,
                                           enum pipe_query_value_type result_type,
                                           int index,
                                           struct pipe_resource *resource,
                                           unsigned offset)
{
}

static void noop_set_min_samples( struct pipe_context *ctx,
                                  unsigned min_samples )
{
}

static void noop_set_sample_locations( struct pipe_context *ctx,
                                       size_t size, const uint8_t *locations )
{
}

static void noop_set_tess_state(struct pipe_context *ctx,
                                const float default_outer_level[4],
                                const float default_inner_level[2])
{
}

static void noop_clear_texture(struct pipe_context *pipe,
                               struct pipe_resource *res,
                               unsigned level,
                               const struct pipe_box *box,
                               const void *data)
{
}

static void noop_clear_buffer(struct pipe_context *pipe,
                              struct pipe_resource *res,
                              unsigned offset,
                              unsigned size,
                              const void *clear_value,
                              int clear_value_size)
{
}

static void noop_fence_server_sync(struct pipe_context *pipe,
                                   struct pipe_fence_handle *fence)
{
}

static void noop_texture_barrier(struct pipe_context *ctx, unsigned flags)
{
}

static void noop_memory_barrier(struct pipe_context *ctx, unsigned flags)
{
}

static bool noop_resource_commit(struct pipe_context *ctx, struct pipe_resource *res,
                                 unsigned level, struct pipe_box *box, bool commit)
{
   return true;
}

static void noop_get_sample_position(struct pipe_context *context,
                                     unsigned sample_count,
                                     unsigned sample_index,
                                     float *out_value)
{
}

static enum pipe_reset_status noop_get_device_reset_status(struct pipe_context *ctx)
{
   return PIPE_NO_RESET;
}

static uint64_t noop_create_texture_handle(struct pipe_context *ctx,
                                           struct pipe_sampler_view *view,
                                           const struct pipe_sampler_state *state)
{
   return 1;
}

static void noop_delete_texture_handle(struct pipe_context *ctx, uint64_t handle)
{
}

static void noop_make_texture_handle_resident(struct pipe_context *ctx,
                                              uint64_t handle, bool resident)
{
}

static uint64_t noop_create_image_handle(struct pipe_context *ctx,
                                         const struct pipe_image_view *image)
{
   return 2;
}

static void noop_delete_image_handle(struct pipe_context *ctx, uint64_t handle)
{
}

static void noop_make_image_handle_resident(struct pipe_context *ctx, uint64_t handle,
                                            unsigned access, bool resident)
{
}

static void noop_set_patch_vertices(struct pipe_context *ctx,
                                    uint8_t patch_vertices)
{
}

void noop_init_state_functions(struct pipe_context *ctx);

void noop_init_state_functions(struct pipe_context *ctx)
{
   ctx->create_blend_state = noop_create_blend_state;
   ctx->create_depth_stencil_alpha_state = noop_create_dsa_state;
   ctx->create_fs_state = noop_create_shader_state;
   ctx->create_rasterizer_state = noop_create_rs_state;
   ctx->create_sampler_state = noop_create_sampler_state;
   ctx->create_sampler_view = noop_create_sampler_view;
   ctx->create_surface = noop_create_surface;
   ctx->create_vertex_elements_state = noop_create_vertex_elements;
   ctx->create_compute_state = noop_create_compute_state;
   ctx->create_tcs_state = noop_create_shader_state;
   ctx->create_tes_state = noop_create_shader_state;
   ctx->create_gs_state = noop_create_shader_state;
   ctx->create_vs_state = noop_create_shader_state;
   ctx->bind_blend_state = noop_bind_state;
   ctx->bind_depth_stencil_alpha_state = noop_bind_state;
   ctx->bind_sampler_states = noop_bind_sampler_states;
   ctx->bind_fs_state = noop_bind_state;
   ctx->bind_rasterizer_state = noop_bind_state;
   ctx->bind_vertex_elements_state = noop_bind_state;
   ctx->bind_compute_state = noop_bind_state;
   ctx->bind_tcs_state = noop_bind_state;
   ctx->bind_tes_state = noop_bind_state;
   ctx->bind_gs_state = noop_bind_state;
   ctx->bind_vs_state = noop_bind_state;
   ctx->delete_blend_state = noop_delete_state;
   ctx->delete_depth_stencil_alpha_state = noop_delete_state;
   ctx->delete_fs_state = noop_delete_state;
   ctx->delete_rasterizer_state = noop_delete_state;
   ctx->delete_sampler_state = noop_delete_state;
   ctx->delete_vertex_elements_state = noop_delete_state;
   ctx->delete_compute_state = noop_delete_state;
   ctx->delete_tcs_state = noop_delete_state;
   ctx->delete_tes_state = noop_delete_state;
   ctx->delete_gs_state = noop_delete_state;
   ctx->delete_vs_state = noop_delete_state;
   ctx->set_blend_color = noop_set_blend_color;
   ctx->set_clip_state = noop_set_clip_state;
   ctx->set_constant_buffer = noop_set_constant_buffer;
   ctx->set_inlinable_constants = noop_set_inlinable_constants;
   ctx->set_sampler_views = noop_set_sampler_views;
   ctx->set_shader_buffers = noop_set_shader_buffers;
   ctx->set_shader_images = noop_set_shader_images;
   ctx->set_framebuffer_state = noop_set_framebuffer_state;
   ctx->set_polygon_stipple = noop_set_polygon_stipple;
   ctx->set_sample_mask = noop_set_sample_mask;
   ctx->set_scissor_states = noop_set_scissor_states;
   ctx->set_stencil_ref = noop_set_stencil_ref;
   ctx->set_vertex_buffers = noop_set_vertex_buffers;
   ctx->set_viewport_states = noop_set_viewport_states;
   ctx->set_window_rectangles = noop_set_window_rectangles;
   ctx->sampler_view_destroy = noop_sampler_view_destroy;
   ctx->surface_destroy = noop_surface_destroy;
   ctx->draw_vbo = noop_draw_vbo;
   ctx->draw_vertex_state = noop_draw_vertex_state;
   ctx->launch_grid = noop_launch_grid;
   ctx->create_stream_output_target = noop_create_stream_output_target;
   ctx->stream_output_target_destroy = noop_stream_output_target_destroy;
   ctx->set_stream_output_targets = noop_set_stream_output_targets;
   ctx->render_condition = noop_render_condition;
   ctx->get_query_result_resource = noop_get_query_result_resource;
   ctx->set_min_samples = noop_set_min_samples;
   ctx->set_sample_locations = noop_set_sample_locations;
   ctx->set_tess_state = noop_set_tess_state;
   ctx->clear_texture = noop_clear_texture;
   ctx->clear_buffer = noop_clear_buffer;
   ctx->fence_server_sync = noop_fence_server_sync;
   ctx->texture_barrier = noop_texture_barrier;
   ctx->memory_barrier = noop_memory_barrier;
   ctx->resource_commit = noop_resource_commit;
   ctx->get_sample_position = noop_get_sample_position;
   ctx->get_device_reset_status = noop_get_device_reset_status;
   ctx->create_texture_handle = noop_create_texture_handle;
   ctx->delete_texture_handle = noop_delete_texture_handle;
   ctx->make_texture_handle_resident = noop_make_texture_handle_resident;
   ctx->create_image_handle = noop_create_image_handle;
   ctx->delete_image_handle = noop_delete_image_handle;
   ctx->make_image_handle_resident = noop_make_image_handle_resident;
   ctx->set_patch_vertices = noop_set_patch_vertices;
}
