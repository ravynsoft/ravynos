/**************************************************************************
 *
 * Copyright 2015 Advanced Micro Devices, Inc.
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
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
 *
 **************************************************************************/

#include "dd_pipe.h"
#include "tgsi/tgsi_parse.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"


static void
safe_memcpy(void *dst, const void *src, size_t size)
{
   if (src)
      memcpy(dst, src, size);
   else
      memset(dst, 0, size);
}


/********************************************************************
 * queries
 */

static struct pipe_query *
dd_context_create_query(struct pipe_context *_pipe, unsigned query_type,
                        unsigned index)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   struct pipe_query *query;

   query = pipe->create_query(pipe, query_type, index);

   /* Wrap query object. */
   if (query) {
      struct dd_query *dd_query = CALLOC_STRUCT(dd_query);
      if (dd_query) {
         dd_query->type = query_type;
         dd_query->query = query;
         query = (struct pipe_query *)dd_query;
      } else {
         pipe->destroy_query(pipe, query);
         query = NULL;
      }
   }

   return query;
}

static struct pipe_query *
dd_context_create_batch_query(struct pipe_context *_pipe, unsigned num_queries,
                              unsigned *query_types)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   struct pipe_query *query;

   query = pipe->create_batch_query(pipe, num_queries, query_types);

   /* Wrap query object. */
   if (query) {
      struct dd_query *dd_query = CALLOC_STRUCT(dd_query);
      if (dd_query) {
         /* no special handling for batch queries yet */
         dd_query->type = query_types[0];
         dd_query->query = query;
         query = (struct pipe_query *)dd_query;
      } else {
         pipe->destroy_query(pipe, query);
         query = NULL;
      }
   }

   return query;
}

static void
dd_context_destroy_query(struct pipe_context *_pipe,
                         struct pipe_query *query)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->destroy_query(pipe, dd_query_unwrap(query));
   FREE(query);
}

static bool
dd_context_begin_query(struct pipe_context *_pipe, struct pipe_query *query)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   return pipe->begin_query(pipe, dd_query_unwrap(query));
}

static bool
dd_context_end_query(struct pipe_context *_pipe, struct pipe_query *query)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   return pipe->end_query(pipe, dd_query_unwrap(query));
}

static bool
dd_context_get_query_result(struct pipe_context *_pipe,
                            struct pipe_query *query, bool wait,
                            union pipe_query_result *result)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   return pipe->get_query_result(pipe, dd_query_unwrap(query), wait, result);
}

static void
dd_context_set_active_query_state(struct pipe_context *_pipe, bool enable)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->set_active_query_state(pipe, enable);
}

static void
dd_context_render_condition(struct pipe_context *_pipe,
                            struct pipe_query *query, bool condition,
                            enum pipe_render_cond_flag mode)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_state *dstate = &dctx->draw_state;

   pipe->render_condition(pipe, dd_query_unwrap(query), condition, mode);
   dstate->render_cond.query = dd_query(query);
   dstate->render_cond.condition = condition;
   dstate->render_cond.mode = mode;
}


/********************************************************************
 * constant (immutable) non-shader states
 */

#define DD_CSO_CREATE(name, shortname) \
   static void * \
   dd_context_create_##name##_state(struct pipe_context *_pipe, \
                                    const struct pipe_##name##_state *state) \
   { \
      struct pipe_context *pipe = dd_context(_pipe)->pipe; \
      struct dd_state *hstate = CALLOC_STRUCT(dd_state); \
 \
      if (!hstate) \
         return NULL; \
      hstate->cso = pipe->create_##name##_state(pipe, state); \
      hstate->state.shortname = *state; \
      return hstate; \
   }

#define DD_CSO_BIND(name, shortname) \
   static void \
   dd_context_bind_##name##_state(struct pipe_context *_pipe, void *state) \
   { \
      struct dd_context *dctx = dd_context(_pipe); \
      struct pipe_context *pipe = dctx->pipe; \
      struct dd_state *hstate = state; \
 \
      dctx->draw_state.shortname = hstate; \
      pipe->bind_##name##_state(pipe, hstate ? hstate->cso : NULL); \
   }

#define DD_CSO_DELETE(name) \
   static void \
   dd_context_delete_##name##_state(struct pipe_context *_pipe, void *state) \
   { \
      struct dd_context *dctx = dd_context(_pipe); \
      struct pipe_context *pipe = dctx->pipe; \
      struct dd_state *hstate = state; \
 \
      pipe->delete_##name##_state(pipe, hstate->cso); \
      FREE(hstate); \
   }

#define DD_CSO_WHOLE(name, shortname) \
   DD_CSO_CREATE(name, shortname) \
   DD_CSO_BIND(name, shortname) \
   DD_CSO_DELETE(name)

DD_CSO_WHOLE(blend, blend)
DD_CSO_WHOLE(rasterizer, rs)
DD_CSO_WHOLE(depth_stencil_alpha, dsa)

DD_CSO_CREATE(sampler, sampler)
DD_CSO_DELETE(sampler)

static void
dd_context_bind_sampler_states(struct pipe_context *_pipe,
                               enum pipe_shader_type shader,
                               unsigned start, unsigned count, void **states)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   memcpy(&dctx->draw_state.sampler_states[shader][start], states,
          sizeof(void*) * count);

   if (states) {
      void *samp[PIPE_MAX_SAMPLERS];
      int i;

      for (i = 0; i < count; i++) {
         struct dd_state *s = states[i];
         samp[i] = s ? s->cso : NULL;
      }

      pipe->bind_sampler_states(pipe, shader, start, count, samp);
   }
   else
      pipe->bind_sampler_states(pipe, shader, start, count, NULL);
}

static void *
dd_context_create_vertex_elements_state(struct pipe_context *_pipe,
                                        unsigned num_elems,
                                        const struct pipe_vertex_element *elems)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   struct dd_state *hstate = CALLOC_STRUCT(dd_state);

   if (!hstate)
      return NULL;
   hstate->cso = pipe->create_vertex_elements_state(pipe, num_elems, elems);
   memcpy(hstate->state.velems.velems, elems, sizeof(elems[0]) * num_elems);
   hstate->state.velems.count = num_elems;
   return hstate;
}

DD_CSO_BIND(vertex_elements, velems)
DD_CSO_DELETE(vertex_elements)


/********************************************************************
 * shaders
 */

#define DD_SHADER_NOCREATE(NAME, name) \
   static void \
   dd_context_bind_##name##_state(struct pipe_context *_pipe, void *state) \
   { \
      struct dd_context *dctx = dd_context(_pipe); \
      struct pipe_context *pipe = dctx->pipe; \
      struct dd_state *hstate = state; \
   \
      dctx->draw_state.shaders[PIPE_SHADER_##NAME] = hstate; \
      pipe->bind_##name##_state(pipe, hstate ? hstate->cso : NULL); \
   } \
    \
   static void \
   dd_context_delete_##name##_state(struct pipe_context *_pipe, void *state) \
   { \
      struct dd_context *dctx = dd_context(_pipe); \
      struct pipe_context *pipe = dctx->pipe; \
      struct dd_state *hstate = state; \
   \
      pipe->delete_##name##_state(pipe, hstate->cso); \
      if (hstate->state.shader.type == PIPE_SHADER_IR_TGSI) \
         tgsi_free_tokens(hstate->state.shader.tokens); \
      FREE(hstate); \
   }

#define DD_SHADER(NAME, name) \
   static void * \
   dd_context_create_##name##_state(struct pipe_context *_pipe, \
                                    const struct pipe_shader_state *state) \
   { \
      struct pipe_context *pipe = dd_context(_pipe)->pipe; \
      struct dd_state *hstate = CALLOC_STRUCT(dd_state); \
 \
      if (!hstate) \
         return NULL; \
      hstate->cso = pipe->create_##name##_state(pipe, state); \
      hstate->state.shader = *state; \
      if (hstate->state.shader.type == PIPE_SHADER_IR_TGSI) \
         hstate->state.shader.tokens = tgsi_dup_tokens(state->tokens); \
      return hstate; \
   } \
    \
   DD_SHADER_NOCREATE(NAME, name)

DD_SHADER(FRAGMENT, fs)
DD_SHADER(VERTEX, vs)
DD_SHADER(GEOMETRY, gs)
DD_SHADER(TESS_CTRL, tcs)
DD_SHADER(TESS_EVAL, tes)

static void * \
dd_context_create_compute_state(struct pipe_context *_pipe,
                                 const struct pipe_compute_state *state)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   struct dd_state *hstate = CALLOC_STRUCT(dd_state);

   if (!hstate)
      return NULL;
   hstate->cso = pipe->create_compute_state(pipe, state);

   hstate->state.shader.type = state->ir_type;

   if (state->ir_type == PIPE_SHADER_IR_TGSI)
      hstate->state.shader.tokens = tgsi_dup_tokens(state->prog);

   return hstate;
}

DD_SHADER_NOCREATE(COMPUTE, compute)

/********************************************************************
 * immediate states
 */

#define DD_IMM_STATE(name, type, deref, ref) \
   static void \
   dd_context_set_##name(struct pipe_context *_pipe, type deref) \
   { \
      struct dd_context *dctx = dd_context(_pipe); \
      struct pipe_context *pipe = dctx->pipe; \
 \
      dctx->draw_state.name = deref; \
      pipe->set_##name(pipe, ref); \
   }

DD_IMM_STATE(blend_color, const struct pipe_blend_color, *state, state)
DD_IMM_STATE(stencil_ref, const struct pipe_stencil_ref, state, state)
DD_IMM_STATE(clip_state, const struct pipe_clip_state, *state, state)
DD_IMM_STATE(sample_mask, unsigned, sample_mask, sample_mask)
DD_IMM_STATE(min_samples, unsigned, min_samples, min_samples)
DD_IMM_STATE(framebuffer_state, const struct pipe_framebuffer_state, *state, state)
DD_IMM_STATE(polygon_stipple, const struct pipe_poly_stipple, *state, state)

static void
dd_context_set_constant_buffer(struct pipe_context *_pipe,
                               enum pipe_shader_type shader, uint index,
                               bool take_ownership,
                               const struct pipe_constant_buffer *constant_buffer)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   safe_memcpy(&dctx->draw_state.constant_buffers[shader][index],
               constant_buffer, sizeof(*constant_buffer));
   pipe->set_constant_buffer(pipe, shader, index, take_ownership, constant_buffer);
}

static void
dd_context_set_scissor_states(struct pipe_context *_pipe,
                              unsigned start_slot, unsigned num_scissors,
                              const struct pipe_scissor_state *states)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   safe_memcpy(&dctx->draw_state.scissors[start_slot], states,
               sizeof(*states) * num_scissors);
   pipe->set_scissor_states(pipe, start_slot, num_scissors, states);
}

static void
dd_context_set_viewport_states(struct pipe_context *_pipe,
                               unsigned start_slot, unsigned num_viewports,
                               const struct pipe_viewport_state *states)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   safe_memcpy(&dctx->draw_state.viewports[start_slot], states,
               sizeof(*states) * num_viewports);
   pipe->set_viewport_states(pipe, start_slot, num_viewports, states);
}

static void dd_context_set_tess_state(struct pipe_context *_pipe,
                                      const float default_outer_level[4],
                                      const float default_inner_level[2])
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   memcpy(dctx->draw_state.tess_default_levels, default_outer_level,
          sizeof(float) * 4);
   memcpy(dctx->draw_state.tess_default_levels+4, default_inner_level,
          sizeof(float) * 2);
   pipe->set_tess_state(pipe, default_outer_level, default_inner_level);
}

static void dd_context_set_patch_vertices(struct pipe_context *_pipe,
                                          uint8_t patch_vertices)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   pipe->set_patch_vertices(pipe, patch_vertices);
}

static void dd_context_set_window_rectangles(struct pipe_context *_pipe,
                                             bool include,
                                             unsigned num_rectangles,
                                             const struct pipe_scissor_state *rects)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   pipe->set_window_rectangles(pipe, include, num_rectangles, rects);
}


/********************************************************************
 * views
 */

static struct pipe_surface *
dd_context_create_surface(struct pipe_context *_pipe,
                          struct pipe_resource *resource,
                          const struct pipe_surface *surf_tmpl)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   struct pipe_surface *view =
      pipe->create_surface(pipe, resource, surf_tmpl);

   if (!view)
      return NULL;
   view->context = _pipe;
   return view;
}

static void
dd_context_surface_destroy(struct pipe_context *_pipe,
                           struct pipe_surface *surf)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->surface_destroy(pipe, surf);
}

static struct pipe_sampler_view *
dd_context_create_sampler_view(struct pipe_context *_pipe,
                               struct pipe_resource *resource,
                               const struct pipe_sampler_view *templ)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   struct pipe_sampler_view *view =
      pipe->create_sampler_view(pipe, resource, templ);

   if (!view)
      return NULL;
   view->context = _pipe;
   return view;
}

static void
dd_context_sampler_view_destroy(struct pipe_context *_pipe,
                                struct pipe_sampler_view *view)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->sampler_view_destroy(pipe, view);
}

static struct pipe_stream_output_target *
dd_context_create_stream_output_target(struct pipe_context *_pipe,
                                       struct pipe_resource *res,
                                       unsigned buffer_offset,
                                       unsigned buffer_size)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   struct pipe_stream_output_target *view =
      pipe->create_stream_output_target(pipe, res, buffer_offset,
                                        buffer_size);

   if (!view)
      return NULL;
   view->context = _pipe;
   return view;
}

static void
dd_context_stream_output_target_destroy(struct pipe_context *_pipe,
                                        struct pipe_stream_output_target *target)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->stream_output_target_destroy(pipe, target);
}


/********************************************************************
 * set states
 */

static void
dd_context_set_sampler_views(struct pipe_context *_pipe,
                             enum pipe_shader_type shader,
                             unsigned start, unsigned num,
                             unsigned unbind_num_trailing_slots,
                             bool take_ownership,
                             struct pipe_sampler_view **views)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   safe_memcpy(&dctx->draw_state.sampler_views[shader][start], views,
               sizeof(views[0]) * num);
   safe_memcpy(&dctx->draw_state.sampler_views[shader][start + num], views,
               sizeof(views[0]) * unbind_num_trailing_slots);
   pipe->set_sampler_views(pipe, shader, start, num, take_ownership,
                           unbind_num_trailing_slots, views);
}

static void
dd_context_set_shader_images(struct pipe_context *_pipe,
                             enum pipe_shader_type shader,
                             unsigned start, unsigned num,
                             unsigned unbind_num_trailing_slots,
                             const struct pipe_image_view *views)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   safe_memcpy(&dctx->draw_state.shader_images[shader][start], views,
               sizeof(views[0]) * num);
   safe_memcpy(&dctx->draw_state.shader_images[shader][start + num], NULL,
               sizeof(views[0]) * unbind_num_trailing_slots);
   pipe->set_shader_images(pipe, shader, start, num,
                           unbind_num_trailing_slots, views);
}

static void
dd_context_set_shader_buffers(struct pipe_context *_pipe,
                              enum pipe_shader_type shader,
                              unsigned start, unsigned num_buffers,
                              const struct pipe_shader_buffer *buffers,
                              unsigned writable_bitmask)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   safe_memcpy(&dctx->draw_state.shader_buffers[shader][start], buffers,
               sizeof(buffers[0]) * num_buffers);
   pipe->set_shader_buffers(pipe, shader, start, num_buffers, buffers,
                            writable_bitmask);
}

static void
dd_context_set_vertex_buffers(struct pipe_context *_pipe,
                              unsigned num_buffers,
                              unsigned unbind_num_trailing_slots,
                              bool take_ownership,
                              const struct pipe_vertex_buffer *buffers)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   safe_memcpy(&dctx->draw_state.vertex_buffers[0], buffers,
               sizeof(buffers[0]) * num_buffers);
   safe_memcpy(&dctx->draw_state.vertex_buffers[num_buffers], NULL,
               sizeof(buffers[0]) * unbind_num_trailing_slots);
   pipe->set_vertex_buffers(pipe, num_buffers,
                            unbind_num_trailing_slots, take_ownership,
                            buffers);
}

static void
dd_context_set_stream_output_targets(struct pipe_context *_pipe,
                                     unsigned num_targets,
                                     struct pipe_stream_output_target **tgs,
                                     const unsigned *offsets)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;
   struct dd_draw_state *dstate = &dctx->draw_state;

   dstate->num_so_targets = num_targets;
   safe_memcpy(dstate->so_targets, tgs, sizeof(*tgs) * num_targets);
   safe_memcpy(dstate->so_offsets, offsets, sizeof(*offsets) * num_targets);
   pipe->set_stream_output_targets(pipe, num_targets, tgs, offsets);
}


static void
dd_context_fence_server_sync(struct pipe_context *_pipe,
                             struct pipe_fence_handle *fence)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   pipe->fence_server_sync(pipe, fence);
}


static void
dd_context_create_fence_fd(struct pipe_context *_pipe,
                           struct pipe_fence_handle **fence,
                           int fd,
                           enum pipe_fd_type type)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   pipe->create_fence_fd(pipe, fence, fd, type);
}


void
dd_thread_join(struct dd_context *dctx)
{
   mtx_lock(&dctx->mutex);
   dctx->kill_thread = true;
   cnd_signal(&dctx->cond);
   mtx_unlock(&dctx->mutex);
   thrd_join(dctx->thread, NULL);
}

static void
dd_context_destroy(struct pipe_context *_pipe)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   dd_thread_join(dctx);
   mtx_destroy(&dctx->mutex);
   cnd_destroy(&dctx->cond);

   assert(list_is_empty(&dctx->records));

   if (pipe->set_log_context) {
      pipe->set_log_context(pipe, NULL);

      if (dd_screen(dctx->base.screen)->dump_mode == DD_DUMP_ALL_CALLS) {
         FILE *f = dd_get_file_stream(dd_screen(dctx->base.screen), 0);
         if (f) {
            fprintf(f, "Remainder of driver log:\n\n");
         }

         u_log_new_page_print(&dctx->log, f);
         fclose(f);
      }
   }
   u_log_context_destroy(&dctx->log);

   pipe->destroy(pipe);
   FREE(dctx);
}


/********************************************************************
 * miscellaneous
 */

static void
dd_context_texture_barrier(struct pipe_context *_pipe, unsigned flags)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->texture_barrier(pipe, flags);
}

static void
dd_context_memory_barrier(struct pipe_context *_pipe, unsigned flags)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->memory_barrier(pipe, flags);
}

static bool
dd_context_resource_commit(struct pipe_context *_pipe,
                           struct pipe_resource *resource,
                           unsigned level, struct pipe_box *box, bool commit)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   return pipe->resource_commit(pipe, resource, level, box, commit);
}

static void
dd_context_set_compute_resources(struct pipe_context *_pipe,
				 unsigned start, unsigned count,
				 struct pipe_surface **resources)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   pipe->set_compute_resources(pipe, start, count, resources);
}

static void
dd_context_set_global_binding(struct pipe_context *_pipe,
			      unsigned first, unsigned count,
			      struct pipe_resource **resources,
			      uint32_t **handles)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;
   pipe->set_global_binding(pipe, first, count, resources, handles);
}

static void
dd_context_get_sample_position(struct pipe_context *_pipe,
                               unsigned sample_count, unsigned sample_index,
                               float *out_value)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->get_sample_position(pipe, sample_count, sample_index,
                             out_value);
}

static void
dd_context_invalidate_resource(struct pipe_context *_pipe,
                               struct pipe_resource *resource)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->invalidate_resource(pipe, resource);
}

static enum pipe_reset_status
dd_context_get_device_reset_status(struct pipe_context *_pipe)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   return pipe->get_device_reset_status(pipe);
}

static void
dd_context_set_device_reset_callback(struct pipe_context *_pipe,
                                     const struct pipe_device_reset_callback *cb)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->set_device_reset_callback(pipe, cb);
}

static void
dd_context_emit_string_marker(struct pipe_context *_pipe,
                              const char *string, int len)
{
   struct dd_context *dctx = dd_context(_pipe);
   struct pipe_context *pipe = dctx->pipe;

   pipe->emit_string_marker(pipe, string, len);
   dd_parse_apitrace_marker(string, len, &dctx->draw_state.apitrace_call_number);
}

static void
dd_context_dump_debug_state(struct pipe_context *_pipe, FILE *stream,
                            unsigned flags)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->dump_debug_state(pipe, stream, flags);
}

static uint64_t
dd_context_create_texture_handle(struct pipe_context *_pipe,
                                 struct pipe_sampler_view *view,
                                 const struct pipe_sampler_state *state)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   return pipe->create_texture_handle(pipe, view, state);
}

static void
dd_context_delete_texture_handle(struct pipe_context *_pipe, uint64_t handle)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->delete_texture_handle(pipe, handle);
}

static void
dd_context_make_texture_handle_resident(struct pipe_context *_pipe,
                                        uint64_t handle, bool resident)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->make_texture_handle_resident(pipe, handle, resident);
}

static uint64_t
dd_context_create_image_handle(struct pipe_context *_pipe,
                               const struct pipe_image_view *image)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   return pipe->create_image_handle(pipe, image);
}

static void
dd_context_delete_image_handle(struct pipe_context *_pipe, uint64_t handle)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->delete_image_handle(pipe, handle);
}

static void
dd_context_make_image_handle_resident(struct pipe_context *_pipe,
                                      uint64_t handle, unsigned access,
                                      bool resident)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->make_image_handle_resident(pipe, handle, access, resident);
}

static void
dd_context_set_context_param(struct pipe_context *_pipe,
                             enum pipe_context_param param,
                             unsigned value)
{
   struct pipe_context *pipe = dd_context(_pipe)->pipe;

   pipe->set_context_param(pipe, param, value);
}

struct pipe_context *
dd_context_create(struct dd_screen *dscreen, struct pipe_context *pipe)
{
   struct dd_context *dctx;

   if (!pipe)
      return NULL;

   dctx = CALLOC_STRUCT(dd_context);
   if (!dctx)
      goto fail;

   dctx->pipe = pipe;
   dctx->base.priv = pipe->priv; /* expose wrapped priv data */
   dctx->base.screen = &dscreen->base;
   dctx->base.stream_uploader = pipe->stream_uploader;
   dctx->base.const_uploader = pipe->const_uploader;

   dctx->base.destroy = dd_context_destroy;

   CTX_INIT(render_condition);
   CTX_INIT(create_query);
   CTX_INIT(create_batch_query);
   CTX_INIT(destroy_query);
   CTX_INIT(begin_query);
   CTX_INIT(end_query);
   CTX_INIT(get_query_result);
   CTX_INIT(set_active_query_state);
   CTX_INIT(create_blend_state);
   CTX_INIT(bind_blend_state);
   CTX_INIT(delete_blend_state);
   CTX_INIT(create_sampler_state);
   CTX_INIT(bind_sampler_states);
   CTX_INIT(delete_sampler_state);
   CTX_INIT(create_rasterizer_state);
   CTX_INIT(bind_rasterizer_state);
   CTX_INIT(delete_rasterizer_state);
   CTX_INIT(create_depth_stencil_alpha_state);
   CTX_INIT(bind_depth_stencil_alpha_state);
   CTX_INIT(delete_depth_stencil_alpha_state);
   CTX_INIT(create_fs_state);
   CTX_INIT(bind_fs_state);
   CTX_INIT(delete_fs_state);
   CTX_INIT(create_vs_state);
   CTX_INIT(bind_vs_state);
   CTX_INIT(delete_vs_state);
   CTX_INIT(create_gs_state);
   CTX_INIT(bind_gs_state);
   CTX_INIT(delete_gs_state);
   CTX_INIT(create_tcs_state);
   CTX_INIT(bind_tcs_state);
   CTX_INIT(delete_tcs_state);
   CTX_INIT(create_tes_state);
   CTX_INIT(bind_tes_state);
   CTX_INIT(delete_tes_state);
   CTX_INIT(create_compute_state);
   CTX_INIT(bind_compute_state);
   CTX_INIT(delete_compute_state);
   CTX_INIT(create_vertex_elements_state);
   CTX_INIT(bind_vertex_elements_state);
   CTX_INIT(delete_vertex_elements_state);
   CTX_INIT(set_blend_color);
   CTX_INIT(set_stencil_ref);
   CTX_INIT(set_sample_mask);
   CTX_INIT(set_min_samples);
   CTX_INIT(set_clip_state);
   CTX_INIT(set_constant_buffer);
   CTX_INIT(set_framebuffer_state);
   CTX_INIT(set_polygon_stipple);
   CTX_INIT(set_scissor_states);
   CTX_INIT(set_viewport_states);
   CTX_INIT(set_sampler_views);
   CTX_INIT(set_tess_state);
   CTX_INIT(set_patch_vertices);
   CTX_INIT(set_shader_buffers);
   CTX_INIT(set_shader_images);
   CTX_INIT(set_vertex_buffers);
   CTX_INIT(set_window_rectangles);
   CTX_INIT(create_stream_output_target);
   CTX_INIT(stream_output_target_destroy);
   CTX_INIT(set_stream_output_targets);
   CTX_INIT(create_fence_fd);
   CTX_INIT(fence_server_sync);
   CTX_INIT(create_sampler_view);
   CTX_INIT(sampler_view_destroy);
   CTX_INIT(create_surface);
   CTX_INIT(surface_destroy);
   CTX_INIT(texture_barrier);
   CTX_INIT(memory_barrier);
   CTX_INIT(resource_commit);
   CTX_INIT(set_compute_resources);
   CTX_INIT(set_global_binding);
   /* create_video_codec */
   /* create_video_buffer */
   CTX_INIT(get_sample_position);
   CTX_INIT(invalidate_resource);
   CTX_INIT(get_device_reset_status);
   CTX_INIT(set_device_reset_callback);
   CTX_INIT(dump_debug_state);
   CTX_INIT(emit_string_marker);
   CTX_INIT(create_texture_handle);
   CTX_INIT(delete_texture_handle);
   CTX_INIT(make_texture_handle_resident);
   CTX_INIT(create_image_handle);
   CTX_INIT(delete_image_handle);
   CTX_INIT(make_image_handle_resident);
   CTX_INIT(set_context_param);

   dd_init_draw_functions(dctx);

   u_log_context_init(&dctx->log);
   if (pipe->set_log_context)
      pipe->set_log_context(pipe, &dctx->log);

   dctx->draw_state.sample_mask = ~0;

   list_inithead(&dctx->records);
   (void) mtx_init(&dctx->mutex, mtx_plain);
   (void) cnd_init(&dctx->cond);
   if (thrd_success != u_thread_create(&dctx->thread,dd_thread_main, dctx)) {
      mtx_destroy(&dctx->mutex);
      goto fail;
   }

   return &dctx->base;

fail:
   FREE(dctx);
   pipe->destroy(pipe);
   return NULL;
}
