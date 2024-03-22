/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/ralloc.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_framebuffer.h"

#include "util/format/u_formats.h"
#include "pipe/p_screen.h"

#include "tr_dump.h"
#include "tr_dump_defines.h"
#include "tr_dump_state.h"
#include "tr_public.h"
#include "tr_screen.h"
#include "tr_texture.h"
#include "tr_context.h"
#include "tr_util.h"
#include "tr_video.h"


struct trace_query
{
   struct threaded_query base;
   unsigned type;
   unsigned index;

   struct pipe_query *query;
};


static inline struct trace_query *
trace_query(struct pipe_query *query)
{
   return (struct trace_query *)query;
}


static inline struct pipe_query *
trace_query_unwrap(struct pipe_query *query)
{
   if (query) {
      return trace_query(query)->query;
   } else {
      return NULL;
   }
}


static inline struct pipe_surface *
trace_surface_unwrap(struct trace_context *tr_ctx,
                     struct pipe_surface *surface)
{
   struct trace_surface *tr_surf;

   if (!surface)
      return NULL;

   assert(surface->texture);
   if (!surface->texture)
      return surface;

   tr_surf = trace_surface(surface);

   assert(tr_surf->surface);
   return tr_surf->surface;
}

static void
dump_fb_state(struct trace_context *tr_ctx,
              const char *method,
              bool deep)
{
   struct pipe_context *pipe = tr_ctx->pipe;
   struct pipe_framebuffer_state *state = &tr_ctx->unwrapped_state;

   trace_dump_call_begin("pipe_context", method);

   trace_dump_arg(ptr, pipe);
   if (deep)
      trace_dump_arg(framebuffer_state_deep, state);
   else
      trace_dump_arg(framebuffer_state, state);
   trace_dump_call_end();

   tr_ctx->seen_fb_state = true;
}

static void
trace_context_draw_vbo(struct pipe_context *_pipe,
                       const struct pipe_draw_info *info,
                       unsigned drawid_offset,
                       const struct pipe_draw_indirect_info *indirect,
                       const struct pipe_draw_start_count_bias *draws,
                       unsigned num_draws)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   if (!tr_ctx->seen_fb_state && trace_dump_is_triggered())
      dump_fb_state(tr_ctx, "current_framebuffer_state", true);

   trace_dump_call_begin("pipe_context", "draw_vbo");

   trace_dump_arg(ptr,  pipe);
   trace_dump_arg(draw_info, info);
   trace_dump_arg(int, drawid_offset);
   trace_dump_arg(draw_indirect_info, indirect);
   trace_dump_arg_begin("draws");
   trace_dump_struct_array(draw_start_count, draws, num_draws);
   trace_dump_arg_end();
   trace_dump_arg(uint, num_draws);

   trace_dump_trace_flush();

   pipe->draw_vbo(pipe, info, drawid_offset, indirect, draws, num_draws);

   trace_dump_call_end();
}

static void
trace_context_draw_mesh_tasks(struct pipe_context *_pipe,
                              const struct pipe_grid_info *info)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "draw_mesh_tasks");

   trace_dump_arg(ptr,  pipe);
   trace_dump_arg(grid_info, info);

   trace_dump_trace_flush();

   pipe->draw_mesh_tasks(pipe, info);

   trace_dump_call_end();
}


static void
trace_context_draw_vertex_state(struct pipe_context *_pipe,
                                struct pipe_vertex_state *state,
                                uint32_t partial_velem_mask,
                                struct pipe_draw_vertex_state_info info,
                                const struct pipe_draw_start_count_bias *draws,
                                unsigned num_draws)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   if (!tr_ctx->seen_fb_state && trace_dump_is_triggered())
      dump_fb_state(tr_ctx, "current_framebuffer_state", true);

   trace_dump_call_begin("pipe_context", "draw_vertex_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);
   trace_dump_arg(uint, partial_velem_mask);
   trace_dump_arg(draw_vertex_state_info, info);
   trace_dump_arg_begin("draws");
   trace_dump_struct_array(draw_start_count, draws, num_draws);
   trace_dump_arg_end();
   trace_dump_arg(uint, num_draws);

   trace_dump_trace_flush();

   pipe->draw_vertex_state(pipe, state, partial_velem_mask, info, draws,
                           num_draws);
   trace_dump_call_end();
}


static struct pipe_query *
trace_context_create_query(struct pipe_context *_pipe,
                           unsigned query_type,
                           unsigned index)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct pipe_query *query;

   trace_dump_call_begin("pipe_context", "create_query");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(query_type, query_type);
   trace_dump_arg(int, index);

   query = pipe->create_query(pipe, query_type, index);

   trace_dump_ret(ptr, query);

   trace_dump_call_end();

   /* Wrap query object. */
   if (query) {
      struct trace_query *tr_query = CALLOC_STRUCT(trace_query);
      if (tr_query) {
         tr_query->type = query_type;
         tr_query->query = query;
         tr_query->index = index;
         query = (struct pipe_query *)tr_query;
      } else {
         pipe->destroy_query(pipe, query);
         query = NULL;
      }
   }

   return query;
}


static void
trace_context_destroy_query(struct pipe_context *_pipe,
                            struct pipe_query *_query)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct trace_query *tr_query = trace_query(_query);
   struct pipe_query *query = tr_query->query;

   FREE(tr_query);

   trace_dump_call_begin("pipe_context", "destroy_query");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, query);

   pipe->destroy_query(pipe, query);

   trace_dump_call_end();
}


static bool
trace_context_begin_query(struct pipe_context *_pipe,
                          struct pipe_query *query)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   bool ret;

   query = trace_query_unwrap(query);

   trace_dump_call_begin("pipe_context", "begin_query");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, query);

   ret = pipe->begin_query(pipe, query);

   trace_dump_call_end();
   return ret;
}


static bool
trace_context_end_query(struct pipe_context *_pipe,
                        struct pipe_query *_query)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   bool ret;

   struct pipe_query *query = trace_query_unwrap(_query);

   trace_dump_call_begin("pipe_context", "end_query");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, query);

   if (tr_ctx->threaded)
      threaded_query(query)->flushed = trace_query(_query)->base.flushed;
   ret = pipe->end_query(pipe, query);

   trace_dump_call_end();
   return ret;
}


static bool
trace_context_get_query_result(struct pipe_context *_pipe,
                               struct pipe_query *_query,
                               bool wait,
                               union pipe_query_result *result)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct trace_query *tr_query = trace_query(_query);
   struct pipe_query *query = tr_query->query;
   bool ret;

   trace_dump_call_begin("pipe_context", "get_query_result");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, query);
   trace_dump_arg(bool, wait);

   if (tr_ctx->threaded)
      threaded_query(query)->flushed = trace_query(_query)->base.flushed;

   ret = pipe->get_query_result(pipe, query, wait, result);

   trace_dump_arg_begin("result");
   if (ret) {
      trace_dump_query_result(tr_query->type, tr_query->index, result);
   } else {
      trace_dump_null();
   }
   trace_dump_arg_end();

   trace_dump_ret(bool, ret);

   trace_dump_call_end();

   return ret;
}

static void
trace_context_get_query_result_resource(struct pipe_context *_pipe,
                                        struct pipe_query *_query,
                                        enum pipe_query_flags flags,
                                        enum pipe_query_value_type result_type,
                                        int index,
                                        struct pipe_resource *resource,
                                        unsigned offset)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct trace_query *tr_query = trace_query(_query);
   struct pipe_query *query = tr_query->query;

   trace_dump_call_begin("pipe_context", "get_query_result_resource");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, query);
   trace_dump_arg(query_flags, flags);
   trace_dump_arg(uint, result_type);
   trace_dump_arg(uint, index);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(uint, offset);

   if (tr_ctx->threaded)
      threaded_query(query)->flushed = tr_query->base.flushed;

   trace_dump_call_end();

   pipe->get_query_result_resource(pipe, query, flags, result_type, index, resource, offset);
}


static void
trace_context_set_active_query_state(struct pipe_context *_pipe,
                                     bool enable)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_active_query_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(bool, enable);

   pipe->set_active_query_state(pipe, enable);

   trace_dump_call_end();
}


static void *
trace_context_create_blend_state(struct pipe_context *_pipe,
                                 const struct pipe_blend_state *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   void * result;

   trace_dump_call_begin("pipe_context", "create_blend_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(blend_state, state);

   result = pipe->create_blend_state(pipe, state);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   struct pipe_blend_state *blend = ralloc(tr_ctx, struct pipe_blend_state);
   if (blend) {
      memcpy(blend, state, sizeof(struct pipe_blend_state));
      _mesa_hash_table_insert(&tr_ctx->blend_states, result, blend);
   }

   return result;
}


static void
trace_context_bind_blend_state(struct pipe_context *_pipe,
                               void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "bind_blend_state");

   trace_dump_arg(ptr, pipe);
   if (state && trace_dump_is_triggered()) {
      struct hash_entry *he = _mesa_hash_table_search(&tr_ctx->blend_states, state);
      if (he)
         trace_dump_arg(blend_state, he->data);
      else
         trace_dump_arg(blend_state, NULL);
   } else
      trace_dump_arg(ptr, state);

   pipe->bind_blend_state(pipe, state);

   trace_dump_call_end();
}


static void
trace_context_delete_blend_state(struct pipe_context *_pipe,
                                 void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "delete_blend_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);

   pipe->delete_blend_state(pipe, state);

   if (state) {
      struct hash_entry *he = _mesa_hash_table_search(&tr_ctx->blend_states, state);
      if (he) {
         ralloc_free(he->data);
         _mesa_hash_table_remove(&tr_ctx->blend_states, he);
      }
   }

   trace_dump_call_end();
}


static void *
trace_context_create_sampler_state(struct pipe_context *_pipe,
                                   const struct pipe_sampler_state *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   void * result;

   trace_dump_call_begin("pipe_context", "create_sampler_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(sampler_state, state);

   result = pipe->create_sampler_state(pipe, state);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   return result;
}


static void
trace_context_bind_sampler_states(struct pipe_context *_pipe,
                                  enum pipe_shader_type shader,
                                  unsigned start,
                                  unsigned num_states,
                                  void **states)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   /* remove this when we have pipe->bind_sampler_states(..., start, ...) */
   assert(start == 0);

   trace_dump_call_begin("pipe_context", "bind_sampler_states");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg_enum(pipe_shader_type, shader);
   trace_dump_arg(uint, start);
   trace_dump_arg(uint, num_states);
   trace_dump_arg_array(ptr, states, num_states);

   pipe->bind_sampler_states(pipe, shader, start, num_states, states);

   trace_dump_call_end();
}


static void
trace_context_delete_sampler_state(struct pipe_context *_pipe,
                                   void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "delete_sampler_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);

   pipe->delete_sampler_state(pipe, state);

   trace_dump_call_end();
}


static void *
trace_context_create_rasterizer_state(struct pipe_context *_pipe,
                                      const struct pipe_rasterizer_state *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   void * result;

   trace_dump_call_begin("pipe_context", "create_rasterizer_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(rasterizer_state, state);

   result = pipe->create_rasterizer_state(pipe, state);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   struct pipe_rasterizer_state *rasterizer = ralloc(tr_ctx, struct pipe_rasterizer_state);
   if (rasterizer) {
      memcpy(rasterizer, state, sizeof(struct pipe_rasterizer_state));
      _mesa_hash_table_insert(&tr_ctx->rasterizer_states, result, rasterizer);
   }

   return result;
}


static void
trace_context_bind_rasterizer_state(struct pipe_context *_pipe,
                                    void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "bind_rasterizer_state");

   trace_dump_arg(ptr, pipe);
   if (state && trace_dump_is_triggered()) {
      struct hash_entry *he = _mesa_hash_table_search(&tr_ctx->rasterizer_states, state);
      if (he)
         trace_dump_arg(rasterizer_state, he->data);
      else
         trace_dump_arg(rasterizer_state, NULL);
   } else
      trace_dump_arg(ptr, state);

   pipe->bind_rasterizer_state(pipe, state);

   trace_dump_call_end();
}


static void
trace_context_delete_rasterizer_state(struct pipe_context *_pipe,
                                      void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "delete_rasterizer_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);

   pipe->delete_rasterizer_state(pipe, state);

   trace_dump_call_end();

   if (state) {
      struct hash_entry *he = _mesa_hash_table_search(&tr_ctx->rasterizer_states, state);
      if (he) {
         ralloc_free(he->data);
         _mesa_hash_table_remove(&tr_ctx->rasterizer_states, he);
      }
   }
}


static void *
trace_context_create_depth_stencil_alpha_state(struct pipe_context *_pipe,
                                               const struct pipe_depth_stencil_alpha_state *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   void * result;

   trace_dump_call_begin("pipe_context", "create_depth_stencil_alpha_state");

   result = pipe->create_depth_stencil_alpha_state(pipe, state);

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(depth_stencil_alpha_state, state);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   struct pipe_depth_stencil_alpha_state *depth_stencil_alpha = ralloc(tr_ctx, struct pipe_depth_stencil_alpha_state);
   if (depth_stencil_alpha) {
      memcpy(depth_stencil_alpha, state, sizeof(struct pipe_depth_stencil_alpha_state));
      _mesa_hash_table_insert(&tr_ctx->depth_stencil_alpha_states, result, depth_stencil_alpha);
   }

   return result;
}


static void
trace_context_bind_depth_stencil_alpha_state(struct pipe_context *_pipe,
                                             void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "bind_depth_stencil_alpha_state");

   trace_dump_arg(ptr, pipe);
   if (state && trace_dump_is_triggered()) {
      struct hash_entry *he = _mesa_hash_table_search(&tr_ctx->depth_stencil_alpha_states, state);
      if (he)
         trace_dump_arg(depth_stencil_alpha_state, he->data);
      else
         trace_dump_arg(depth_stencil_alpha_state, NULL);
   } else
      trace_dump_arg(ptr, state);

   pipe->bind_depth_stencil_alpha_state(pipe, state);

   trace_dump_call_end();
}


static void
trace_context_delete_depth_stencil_alpha_state(struct pipe_context *_pipe,
                                               void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "delete_depth_stencil_alpha_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);

   pipe->delete_depth_stencil_alpha_state(pipe, state);

   trace_dump_call_end();

   if (state) {
      struct hash_entry *he = _mesa_hash_table_search(&tr_ctx->depth_stencil_alpha_states, state);
      if (he) {
         ralloc_free(he->data);
         _mesa_hash_table_remove(&tr_ctx->depth_stencil_alpha_states, he);
      }
   }
}


#define TRACE_SHADER_STATE(shader_type) \
   static void * \
   trace_context_create_##shader_type##_state(struct pipe_context *_pipe, \
                                 const struct pipe_shader_state *state) \
   { \
      struct trace_context *tr_ctx = trace_context(_pipe); \
      struct pipe_context *pipe = tr_ctx->pipe; \
      void * result; \
      trace_dump_call_begin("pipe_context", "create_" #shader_type "_state"); \
      trace_dump_arg(ptr, pipe); \
      trace_dump_arg(shader_state, state); \
      result = pipe->create_##shader_type##_state(pipe, state); \
      trace_dump_ret(ptr, result); \
      trace_dump_call_end(); \
      return result; \
   } \
    \
   static void \
   trace_context_bind_##shader_type##_state(struct pipe_context *_pipe, \
                               void *state) \
   { \
      struct trace_context *tr_ctx = trace_context(_pipe); \
      struct pipe_context *pipe = tr_ctx->pipe; \
      trace_dump_call_begin("pipe_context", "bind_" #shader_type "_state"); \
      trace_dump_arg(ptr, pipe); \
      trace_dump_arg(ptr, state); \
      pipe->bind_##shader_type##_state(pipe, state); \
      trace_dump_call_end(); \
   } \
    \
   static void \
   trace_context_delete_##shader_type##_state(struct pipe_context *_pipe, \
                                 void *state) \
   { \
      struct trace_context *tr_ctx = trace_context(_pipe); \
      struct pipe_context *pipe = tr_ctx->pipe; \
      trace_dump_call_begin("pipe_context", "delete_" #shader_type "_state"); \
      trace_dump_arg(ptr, pipe); \
      trace_dump_arg(ptr, state); \
      pipe->delete_##shader_type##_state(pipe, state); \
      trace_dump_call_end(); \
   }

TRACE_SHADER_STATE(fs)
TRACE_SHADER_STATE(vs)
TRACE_SHADER_STATE(gs)
TRACE_SHADER_STATE(tcs)
TRACE_SHADER_STATE(tes)
TRACE_SHADER_STATE(ms)
TRACE_SHADER_STATE(ts)

#undef TRACE_SHADER_STATE

static void
trace_context_link_shader(struct pipe_context *_pipe, void **shaders)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "link_shader");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg_array(ptr, shaders, PIPE_SHADER_TYPES);
   pipe->link_shader(pipe, shaders);
   trace_dump_call_end();
}

static inline void *
trace_context_create_compute_state(struct pipe_context *_pipe,
                                   const struct pipe_compute_state *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   void * result;

   trace_dump_call_begin("pipe_context", "create_compute_state");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg(compute_state, state);
   result = pipe->create_compute_state(pipe, state);
   trace_dump_ret(ptr, result);
   trace_dump_call_end();
   return result;
}

static inline void
trace_context_bind_compute_state(struct pipe_context *_pipe,
                                 void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "bind_compute_state");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);
   pipe->bind_compute_state(pipe, state);
   trace_dump_call_end();
}

static inline void
trace_context_delete_compute_state(struct pipe_context *_pipe,
                                   void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "delete_compute_state");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);
   pipe->delete_compute_state(pipe, state);
   trace_dump_call_end();
}

static void *
trace_context_create_vertex_elements_state(struct pipe_context *_pipe,
                                           unsigned num_elements,
                                           const struct  pipe_vertex_element *elements)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   void * result;

   trace_dump_call_begin("pipe_context", "create_vertex_elements_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, num_elements);

   trace_dump_arg_begin("elements");
   trace_dump_struct_array(vertex_element, elements, num_elements);
   trace_dump_arg_end();

   result = pipe->create_vertex_elements_state(pipe, num_elements, elements);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   return result;
}


static void
trace_context_bind_vertex_elements_state(struct pipe_context *_pipe,
                                         void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "bind_vertex_elements_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);

   pipe->bind_vertex_elements_state(pipe, state);

   trace_dump_call_end();
}


static void
trace_context_delete_vertex_elements_state(struct pipe_context *_pipe,
                                           void *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "delete_vertex_elements_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, state);

   pipe->delete_vertex_elements_state(pipe, state);

   trace_dump_call_end();
}


static void
trace_context_set_blend_color(struct pipe_context *_pipe,
                              const struct pipe_blend_color *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_blend_color");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(blend_color, state);

   pipe->set_blend_color(pipe, state);

   trace_dump_call_end();
}


static void
trace_context_set_stencil_ref(struct pipe_context *_pipe,
                              const struct pipe_stencil_ref state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_stencil_ref");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(stencil_ref, &state);

   pipe->set_stencil_ref(pipe, state);

   trace_dump_call_end();
}


static void
trace_context_set_clip_state(struct pipe_context *_pipe,
                             const struct pipe_clip_state *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_clip_state");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(clip_state, state);

   pipe->set_clip_state(pipe, state);

   trace_dump_call_end();
}

static void
trace_context_set_sample_mask(struct pipe_context *_pipe,
                              unsigned sample_mask)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_sample_mask");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, sample_mask);

   pipe->set_sample_mask(pipe, sample_mask);

   trace_dump_call_end();
}

static void
trace_context_set_constant_buffer(struct pipe_context *_pipe,
                                  enum pipe_shader_type shader, uint index,
                                  bool take_ownership,
                                  const struct pipe_constant_buffer *constant_buffer)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_constant_buffer");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg_enum(pipe_shader_type, shader);
   trace_dump_arg(uint, index);
   trace_dump_arg(bool, take_ownership);
   trace_dump_arg(constant_buffer, constant_buffer);

   pipe->set_constant_buffer(pipe, shader, index, take_ownership, constant_buffer);

   trace_dump_call_end();
}


static void
trace_context_set_framebuffer_state(struct pipe_context *_pipe,
                                    const struct pipe_framebuffer_state *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   unsigned i;

   /* Unwrap the input state */
   memcpy(&tr_ctx->unwrapped_state, state, sizeof(tr_ctx->unwrapped_state));
   for (i = 0; i < state->nr_cbufs; ++i)
      tr_ctx->unwrapped_state.cbufs[i] = trace_surface_unwrap(tr_ctx, state->cbufs[i]);
   for (i = state->nr_cbufs; i < PIPE_MAX_COLOR_BUFS; ++i)
      tr_ctx->unwrapped_state.cbufs[i] = NULL;
   tr_ctx->unwrapped_state.zsbuf = trace_surface_unwrap(tr_ctx, state->zsbuf);
   state = &tr_ctx->unwrapped_state;

   dump_fb_state(tr_ctx, "set_framebuffer_state", trace_dump_is_triggered());

   pipe->set_framebuffer_state(pipe, state);
}

static void
trace_context_set_inlinable_constants(struct pipe_context *_pipe, enum pipe_shader_type shader,
                                      uint num_values, uint32_t *values)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_inlinable_constants");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg_enum(pipe_shader_type, shader);
   trace_dump_arg(uint, num_values);
   trace_dump_arg_array(uint, values, num_values);

   pipe->set_inlinable_constants(pipe, shader, num_values, values);

   trace_dump_call_end();
}


static void
trace_context_set_polygon_stipple(struct pipe_context *_pipe,
                                  const struct pipe_poly_stipple *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_polygon_stipple");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(poly_stipple, state);

   pipe->set_polygon_stipple(pipe, state);

   trace_dump_call_end();
}

static void
trace_context_set_min_samples(struct pipe_context *_pipe,
                              unsigned min_samples)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_min_samples");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, min_samples);

   pipe->set_min_samples(pipe, min_samples);

   trace_dump_call_end();
}


static void
trace_context_set_scissor_states(struct pipe_context *_pipe,
                                 unsigned start_slot,
                                 unsigned num_scissors,
                                 const struct pipe_scissor_state *states)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_scissor_states");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, start_slot);
   trace_dump_arg(uint, num_scissors);
   trace_dump_arg(scissor_state, states);

   pipe->set_scissor_states(pipe, start_slot, num_scissors, states);

   trace_dump_call_end();
}


static void
trace_context_set_viewport_states(struct pipe_context *_pipe,
                                  unsigned start_slot,
                                  unsigned num_viewports,
                                  const struct pipe_viewport_state *states)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_viewport_states");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, start_slot);
   trace_dump_arg(uint, num_viewports);
   trace_dump_arg(viewport_state, states);

   pipe->set_viewport_states(pipe, start_slot, num_viewports, states);

   trace_dump_call_end();
}


static struct pipe_sampler_view *
trace_context_create_sampler_view(struct pipe_context *_pipe,
                                  struct pipe_resource *resource,
                                  const struct pipe_sampler_view *templ)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct pipe_sampler_view *result;

   trace_dump_call_begin("pipe_context", "create_sampler_view");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, resource);

   trace_dump_arg_begin("templ");
   trace_dump_sampler_view_template(templ);
   trace_dump_arg_end();

   result = pipe->create_sampler_view(pipe, resource, templ);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   result = trace_sampler_view_create(tr_ctx, resource, result);

   return result;
}


static void
trace_context_sampler_view_destroy(struct pipe_context *_pipe,
                                   struct pipe_sampler_view *_view)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct trace_sampler_view *tr_view = trace_sampler_view(_view);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct pipe_sampler_view *view = tr_view->sampler_view;

   trace_dump_call_begin("pipe_context", "sampler_view_destroy");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, view);

   trace_sampler_view_destroy(tr_view);

   trace_dump_call_end();
}

/********************************************************************
 * surface
 */


static struct pipe_surface *
trace_context_create_surface(struct pipe_context *_pipe,
                             struct pipe_resource *resource,
                             const struct pipe_surface *surf_tmpl)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct pipe_surface *result = NULL;

   trace_dump_call_begin("pipe_context", "create_surface");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, resource);

   trace_dump_arg_begin("surf_tmpl");
   trace_dump_surface_template(surf_tmpl, resource->target);
   trace_dump_arg_end();


   result = pipe->create_surface(pipe, resource, surf_tmpl);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   result = trace_surf_create(tr_ctx, resource, result);

   return result;
}


static void
trace_context_surface_destroy(struct pipe_context *_pipe,
                              struct pipe_surface *_surface)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct trace_surface *tr_surf = trace_surface(_surface);
   struct pipe_surface *surface = tr_surf->surface;

   trace_dump_call_begin("pipe_context", "surface_destroy");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, surface);

   trace_dump_call_end();

   trace_surf_destroy(tr_surf);
}


static void
trace_context_set_sampler_views(struct pipe_context *_pipe,
                                enum pipe_shader_type shader,
                                unsigned start,
                                unsigned num,
                                unsigned unbind_num_trailing_slots,
                                bool take_ownership,
                                struct pipe_sampler_view **views)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct trace_sampler_view *tr_view;
   struct pipe_context *pipe = tr_ctx->pipe;
   struct pipe_sampler_view *unwrapped_views[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   unsigned i;

   /* remove this when we have pipe->set_sampler_views(..., start, ...) */
   assert(start == 0);

   for (i = 0; i < num; ++i) {
      tr_view = trace_sampler_view(views[i]);
      unwrapped_views[i] = trace_sampler_view_unwrap(tr_view);
   }
   views = unwrapped_views;

   trace_dump_call_begin("pipe_context", "set_sampler_views");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg_enum(pipe_shader_type, shader);
   trace_dump_arg(uint, start);
   trace_dump_arg(uint, num);
   trace_dump_arg(uint, unbind_num_trailing_slots);
   trace_dump_arg(bool, take_ownership);
   trace_dump_arg_array(ptr, views, num);

   pipe->set_sampler_views(pipe, shader, start, num,
                           unbind_num_trailing_slots, take_ownership, views);

   trace_dump_call_end();
}


static void
trace_context_set_vertex_buffers(struct pipe_context *_pipe,
                                 unsigned num_buffers,
                                 unsigned unbind_num_trailing_slots,
                                 bool take_ownership,
                                 const struct pipe_vertex_buffer *buffers)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_vertex_buffers");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, num_buffers);
   trace_dump_arg(uint, unbind_num_trailing_slots);
   trace_dump_arg(bool, take_ownership);

   trace_dump_arg_begin("buffers");
   trace_dump_struct_array(vertex_buffer, buffers, num_buffers);
   trace_dump_arg_end();

   pipe->set_vertex_buffers(pipe, num_buffers,
                            unbind_num_trailing_slots, take_ownership,
                            buffers);

   trace_dump_call_end();
}


static struct pipe_stream_output_target *
trace_context_create_stream_output_target(struct pipe_context *_pipe,
                                          struct pipe_resource *res,
                                          unsigned buffer_offset,
                                          unsigned buffer_size)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct pipe_stream_output_target *result;

   trace_dump_call_begin("pipe_context", "create_stream_output_target");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, res);
   trace_dump_arg(uint, buffer_offset);
   trace_dump_arg(uint, buffer_size);

   result = pipe->create_stream_output_target(pipe,
                                              res, buffer_offset, buffer_size);

   trace_dump_ret(ptr, result);

   trace_dump_call_end();

   return result;
}


static void
trace_context_stream_output_target_destroy(
   struct pipe_context *_pipe,
   struct pipe_stream_output_target *target)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "stream_output_target_destroy");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, target);

   pipe->stream_output_target_destroy(pipe, target);

   trace_dump_call_end();
}


static void
trace_context_set_stream_output_targets(struct pipe_context *_pipe,
                                        unsigned num_targets,
                                        struct pipe_stream_output_target **tgs,
                                        const unsigned *offsets)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_stream_output_targets");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, num_targets);
   trace_dump_arg_array(ptr, tgs, num_targets);
   trace_dump_arg_array(uint, offsets, num_targets);

   pipe->set_stream_output_targets(pipe, num_targets, tgs, offsets);

   trace_dump_call_end();
}


static void
trace_context_resource_copy_region(struct pipe_context *_pipe,
                                   struct pipe_resource *dst,
                                   unsigned dst_level,
                                   unsigned dstx, unsigned dsty, unsigned dstz,
                                   struct pipe_resource *src,
                                   unsigned src_level,
                                   const struct pipe_box *src_box)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "resource_copy_region");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, dst);
   trace_dump_arg(uint, dst_level);
   trace_dump_arg(uint, dstx);
   trace_dump_arg(uint, dsty);
   trace_dump_arg(uint, dstz);
   trace_dump_arg(ptr, src);
   trace_dump_arg(uint, src_level);
   trace_dump_arg(box, src_box);

   pipe->resource_copy_region(pipe,
                              dst, dst_level, dstx, dsty, dstz,
                              src, src_level, src_box);

   trace_dump_call_end();
}


static void
trace_context_blit(struct pipe_context *_pipe,
                   const struct pipe_blit_info *_info)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   struct pipe_blit_info info = *_info;

   trace_dump_call_begin("pipe_context", "blit");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(blit_info, _info);

   pipe->blit(pipe, &info);

   trace_dump_call_end();
}


static void
trace_context_flush_resource(struct pipe_context *_pipe,
                             struct pipe_resource *resource)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "flush_resource");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, resource);

   pipe->flush_resource(pipe, resource);

   trace_dump_call_end();
}


static void
trace_context_clear(struct pipe_context *_pipe,
                    unsigned buffers,
                    const struct pipe_scissor_state *scissor_state,
                    const union pipe_color_union *color,
                    double depth,
                    unsigned stencil)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "clear");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, buffers);
   trace_dump_arg_begin("scissor_state");
   trace_dump_scissor_state(scissor_state);
   trace_dump_arg_end();
   if (color)
      trace_dump_arg_array(uint, color->ui, 4);
   else
      trace_dump_null();
   trace_dump_arg(float, depth);
   trace_dump_arg(uint, stencil);

   pipe->clear(pipe, buffers, scissor_state, color, depth, stencil);

   trace_dump_call_end();
}


static void
trace_context_clear_render_target(struct pipe_context *_pipe,
                                  struct pipe_surface *dst,
                                  const union pipe_color_union *color,
                                  unsigned dstx, unsigned dsty,
                                  unsigned width, unsigned height,
                                  bool render_condition_enabled)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   dst = trace_surface_unwrap(tr_ctx, dst);

   trace_dump_call_begin("pipe_context", "clear_render_target");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, dst);
   trace_dump_arg_array(uint, color->ui, 4);
   trace_dump_arg(uint, dstx);
   trace_dump_arg(uint, dsty);
   trace_dump_arg(uint, width);
   trace_dump_arg(uint, height);
   trace_dump_arg(bool, render_condition_enabled);

   pipe->clear_render_target(pipe, dst, color, dstx, dsty, width, height,
                             render_condition_enabled);

   trace_dump_call_end();
}

static void
trace_context_clear_depth_stencil(struct pipe_context *_pipe,
                                  struct pipe_surface *dst,
                                  unsigned clear_flags,
                                  double depth,
                                  unsigned stencil,
                                  unsigned dstx, unsigned dsty,
                                  unsigned width, unsigned height,
                                  bool render_condition_enabled)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   dst = trace_surface_unwrap(tr_ctx, dst);

   trace_dump_call_begin("pipe_context", "clear_depth_stencil");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, dst);
   trace_dump_arg(uint, clear_flags);
   trace_dump_arg(float, depth);
   trace_dump_arg(uint, stencil);
   trace_dump_arg(uint, dstx);
   trace_dump_arg(uint, dsty);
   trace_dump_arg(uint, width);
   trace_dump_arg(uint, height);
   trace_dump_arg(bool, render_condition_enabled);

   pipe->clear_depth_stencil(pipe, dst, clear_flags, depth, stencil,
                             dstx, dsty, width, height,
                             render_condition_enabled);

   trace_dump_call_end();
}

static inline void
trace_context_clear_buffer(struct pipe_context *_pipe,
                           struct pipe_resource *res,
                           unsigned offset,
                           unsigned size,
                           const void *clear_value,
                           int clear_value_size)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;


   trace_dump_call_begin("pipe_context", "clear_buffer");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, res);
   trace_dump_arg(uint, offset);
   trace_dump_arg(uint, size);
   trace_dump_arg(ptr, clear_value);
   trace_dump_arg(int, clear_value_size);

   pipe->clear_buffer(pipe, res, offset, size, clear_value, clear_value_size);

   trace_dump_call_end();
}

static inline void
trace_context_clear_texture(struct pipe_context *_pipe,
                            struct pipe_resource *res,
                            unsigned level,
                            const struct pipe_box *box,
                            const void *data)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   const struct util_format_description *desc = util_format_description(res->format);
   struct pipe_context *pipe = tr_ctx->pipe;
   union pipe_color_union color;
   float depth = 0.0f;
   uint8_t stencil = 0;

   trace_dump_call_begin("pipe_context", "clear_texture");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, res);
   trace_dump_arg(uint, level);
   trace_dump_arg_begin("box");
   trace_dump_box(box);
   trace_dump_arg_end();
   if (util_format_has_depth(desc)) {
      util_format_unpack_z_float(res->format, &depth, data, 1);
      trace_dump_arg(float, depth);
   }
   if (util_format_has_stencil(desc)) {
      util_format_unpack_s_8uint(res->format, &stencil, data, 1);
      trace_dump_arg(uint, stencil);
   }
   if (!util_format_is_depth_or_stencil(res->format)) {
      util_format_unpack_rgba(res->format, color.ui, data, 1);
      trace_dump_arg_array(uint, color.ui, 4);
   }

   pipe->clear_texture(pipe, res, level, box, data);

   trace_dump_call_end();
}

static void
trace_context_flush(struct pipe_context *_pipe,
                    struct pipe_fence_handle **fence,
                    unsigned flags)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "flush");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, flags);

   pipe->flush(pipe, fence, flags);

   if (fence)
      trace_dump_ret(ptr, *fence);

   trace_dump_call_end();

   if (flags & PIPE_FLUSH_END_OF_FRAME) {
      trace_dump_check_trigger();
      tr_ctx->seen_fb_state = false;
   }
}


static void
trace_context_create_fence_fd(struct pipe_context *_pipe,
                              struct pipe_fence_handle **fence,
                              int fd,
                              enum pipe_fd_type type)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "create_fence_fd");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg_enum(pipe_fd_type, fd);
   trace_dump_arg(uint, type);

   pipe->create_fence_fd(pipe, fence, fd, type);

   if (fence)
      trace_dump_ret(ptr, *fence);

   trace_dump_call_end();
}


static void
trace_context_fence_server_sync(struct pipe_context *_pipe,
                                struct pipe_fence_handle *fence)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "fence_server_sync");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, fence);

   pipe->fence_server_sync(pipe, fence);

   trace_dump_call_end();
}


static void
trace_context_fence_server_signal(struct pipe_context *_pipe,
                                struct pipe_fence_handle *fence)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "fence_server_signal");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, fence);

   pipe->fence_server_signal(pipe, fence);

   trace_dump_call_end();
}


static inline bool
trace_context_generate_mipmap(struct pipe_context *_pipe,
                              struct pipe_resource *res,
                              enum pipe_format format,
                              unsigned base_level,
                              unsigned last_level,
                              unsigned first_layer,
                              unsigned last_layer)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   bool ret;

   trace_dump_call_begin("pipe_context", "generate_mipmap");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, res);

   trace_dump_arg(format, format);
   trace_dump_arg(uint, base_level);
   trace_dump_arg(uint, last_level);
   trace_dump_arg(uint, first_layer);
   trace_dump_arg(uint, last_layer);

   ret = pipe->generate_mipmap(pipe, res, format, base_level, last_level,
                               first_layer, last_layer);

   trace_dump_ret(bool, ret);
   trace_dump_call_end();

   return ret;
}


static void
trace_context_destroy(struct pipe_context *_pipe)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "destroy");
   trace_dump_arg(ptr, pipe);
   trace_dump_call_end();

   pipe->destroy(pipe);

   ralloc_free(tr_ctx);
}


/********************************************************************
 * transfer
 */


static void *
trace_context_transfer_map(struct pipe_context *_context,
                           struct pipe_resource *resource,
                           unsigned level,
                           unsigned usage,
                           const struct pipe_box *box,
                           struct pipe_transfer **transfer)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *pipe = tr_context->pipe;
   struct pipe_transfer *xfer = NULL;
   void *map;

   if (resource->target == PIPE_BUFFER)
      map = pipe->buffer_map(pipe, resource, level, usage, box, &xfer);
   else
      map = pipe->texture_map(pipe, resource, level, usage, box, &xfer);
   if (!map)
      return NULL;
   *transfer = trace_transfer_create(tr_context, resource, xfer);
   trace_dump_call_begin("pipe_context", resource->target == PIPE_BUFFER ? "buffer_map" : "texture_map");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(uint, level);
   trace_dump_arg_enum(pipe_map_flags, usage);
   trace_dump_arg(box, box);

   trace_dump_arg(ptr, xfer);
   trace_dump_ret(ptr, map);

   trace_dump_call_end();

   if (map) {
      if (usage & PIPE_MAP_WRITE) {
         trace_transfer(*transfer)->map = map;
      }
   }

   return *transfer ? map : NULL;
}

static void
trace_context_transfer_flush_region( struct pipe_context *_context,
				     struct pipe_transfer *_transfer,
				     const struct pipe_box *box)
{
   struct trace_context *tr_context = trace_context(_context);
   struct trace_transfer *tr_transfer = trace_transfer(_transfer);
   struct pipe_context *pipe = tr_context->pipe;
   struct pipe_transfer *transfer = tr_transfer->transfer;

   trace_dump_call_begin("pipe_context", "transfer_flush_region");

   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, transfer);
   trace_dump_arg(box, box);

   trace_dump_call_end();

   pipe->transfer_flush_region(pipe, transfer, box);
}

static void
trace_context_transfer_unmap(struct pipe_context *_context,
                             struct pipe_transfer *_transfer)
{
   struct trace_context *tr_ctx = trace_context(_context);
   struct trace_transfer *tr_trans = trace_transfer(_transfer);
   struct pipe_context *context = tr_ctx->pipe;
   struct pipe_transfer *transfer = tr_trans->transfer;


   trace_dump_call_begin("pipe_context", "transfer_unmap");

   trace_dump_arg(ptr, context);
   trace_dump_arg(ptr, transfer);

   trace_dump_call_end();

   if (tr_trans->map && !tr_ctx->threaded) {
      /*
       * Fake a texture/buffer_subdata
       */

      struct pipe_resource *resource = transfer->resource;
      unsigned usage = transfer->usage;
      const struct pipe_box *box = &transfer->box;
      unsigned stride = transfer->stride;
      uintptr_t layer_stride = transfer->layer_stride;

      if (resource->target == PIPE_BUFFER) {
         unsigned offset = box->x;
         unsigned size = box->width;

         trace_dump_call_begin("pipe_context", "buffer_subdata");

         trace_dump_arg(ptr, context);
         trace_dump_arg(ptr, resource);
         trace_dump_arg_enum(pipe_map_flags, usage);
         trace_dump_arg(uint, offset);
         trace_dump_arg(uint, size);

         trace_dump_arg_begin("data");
         trace_dump_box_bytes(tr_trans->map,
                              resource,
                              box,
                              stride,
                              layer_stride);
         trace_dump_arg_end();

         trace_dump_arg(uint, stride);
         trace_dump_arg(uint, layer_stride);

         trace_dump_call_end();
      } else {
         unsigned level = transfer->level;

         trace_dump_call_begin("pipe_context", "texture_subdata");

         trace_dump_arg(ptr, context);
         trace_dump_arg(ptr, resource);
         trace_dump_arg(uint, level);
         trace_dump_arg_enum(pipe_map_flags, usage);
         trace_dump_arg(box, box);

         trace_dump_arg_begin("data");
         trace_dump_box_bytes(tr_trans->map,
                              resource,
                              box,
                              stride,
                              layer_stride);
         trace_dump_arg_end();

         trace_dump_arg(uint, stride);
         trace_dump_arg(uint, layer_stride);

         trace_dump_call_end();
      }

      tr_trans->map = NULL;
   }

   if (transfer->resource->target == PIPE_BUFFER)
      context->buffer_unmap(context, transfer);
   else
      context->texture_unmap(context, transfer);
   trace_transfer_destroy(tr_ctx, tr_trans);
}


static void
trace_context_buffer_subdata(struct pipe_context *_context,
                             struct pipe_resource *resource,
                             unsigned usage, unsigned offset,
                             unsigned size, const void *data)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;
   struct pipe_box box;

   trace_dump_call_begin("pipe_context", "buffer_subdata");

   trace_dump_arg(ptr, context);
   trace_dump_arg(ptr, resource);
   trace_dump_arg_enum(pipe_map_flags, usage);
   trace_dump_arg(uint, offset);
   trace_dump_arg(uint, size);

   trace_dump_arg_begin("data");
   u_box_1d(offset, size, &box);
   trace_dump_box_bytes(data, resource, &box, 0, 0);
   trace_dump_arg_end();

   trace_dump_call_end();

   context->buffer_subdata(context, resource, usage, offset, size, data);
}


static void
trace_context_texture_subdata(struct pipe_context *_context,
                              struct pipe_resource *resource,
                              unsigned level,
                              unsigned usage,
                              const struct pipe_box *box,
                              const void *data,
                              unsigned stride,
                              uintptr_t layer_stride)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "texture_subdata");

   trace_dump_arg(ptr, context);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(uint, level);
   trace_dump_arg_enum(pipe_map_flags, usage);
   trace_dump_arg(box, box);

   trace_dump_arg_begin("data");
   trace_dump_box_bytes(data,
                        resource,
                        box,
                        stride,
                        layer_stride);
   trace_dump_arg_end();

   trace_dump_arg(uint, stride);
   trace_dump_arg(uint, layer_stride);

   trace_dump_call_end();

   context->texture_subdata(context, resource, level, usage, box,
                            data, stride, layer_stride);
}

static void
trace_context_invalidate_resource(struct pipe_context *_context,
                                  struct pipe_resource *resource)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "invalidate_resource");

   trace_dump_arg(ptr, context);
   trace_dump_arg(ptr, resource);

   trace_dump_call_end();

   context->invalidate_resource(context, resource);
}

static void
trace_context_set_context_param(struct pipe_context *_context,
                                enum pipe_context_param param,
                                unsigned value)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "set_context_param");

   trace_dump_arg(ptr, context);
   trace_dump_arg(uint, param);
   trace_dump_arg(uint, value);

   trace_dump_call_end();

   context->set_context_param(context, param, value);
}

static void
trace_context_set_debug_callback(struct pipe_context *_context, const struct util_debug_callback *cb)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "set_debug_callback");

   trace_dump_arg(ptr, context);

   trace_dump_call_end();

   context->set_debug_callback(context, cb);
}

static void
trace_context_render_condition(struct pipe_context *_context,
                               struct pipe_query *query,
                               bool condition,
                               enum pipe_render_cond_flag mode)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   query = trace_query_unwrap(query);

   trace_dump_call_begin("pipe_context", "render_condition");

   trace_dump_arg(ptr, context);
   trace_dump_arg(ptr, query);
   trace_dump_arg(bool, condition);
   trace_dump_arg(uint, mode);

   trace_dump_call_end();

   context->render_condition(context, query, condition, mode);
}

static void
trace_context_render_condition_mem(struct pipe_context *_context,
                                    struct pipe_resource *buffer,
                                    uint32_t offset,
                                    bool condition)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "render_condition_mem");

   trace_dump_arg(ptr, context);
   trace_dump_arg(ptr, buffer);
   trace_dump_arg(uint, offset);
   trace_dump_arg(bool, condition);

   trace_dump_call_end();

   context->render_condition_mem(context, buffer, offset, condition);
}


static void
trace_context_texture_barrier(struct pipe_context *_context, unsigned flags)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "texture_barrier");

   trace_dump_arg(ptr, context);
   trace_dump_arg(uint, flags);

   trace_dump_call_end();

   context->texture_barrier(context, flags);
}


static void
trace_context_memory_barrier(struct pipe_context *_context,
                             unsigned flags)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "memory_barrier");
   trace_dump_arg(ptr, context);
   trace_dump_arg(uint, flags);
   trace_dump_call_end();

   context->memory_barrier(context, flags);
}


static bool
trace_context_resource_commit(struct pipe_context *_context,
                              struct pipe_resource *resource,
                              unsigned level, struct pipe_box *box, bool commit)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "resource_commit");
   trace_dump_arg(ptr, context);
   trace_dump_arg(ptr, resource);
   trace_dump_arg(uint, level);
   trace_dump_arg(box, box);
   trace_dump_arg(bool, commit);
   trace_dump_call_end();

   return context->resource_commit(context, resource, level, box, commit);
}

static struct pipe_video_codec *
trace_context_create_video_codec(struct pipe_context *_context,
                                 const struct pipe_video_codec *templat)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;
   struct pipe_video_codec *result;

   trace_dump_call_begin("pipe_context", "create_video_codec");

   trace_dump_arg(ptr, context);
   trace_dump_arg(video_codec_template, templat);

   result = context->create_video_codec(context, templat);

   trace_dump_ret(ptr, result);
   trace_dump_call_end();

   result = trace_video_codec_create(tr_context, result);

   return result;
}

static struct pipe_video_buffer *
trace_context_create_video_buffer_with_modifiers(struct pipe_context *_context,
                                                 const struct pipe_video_buffer *templat,
                                                 const uint64_t *modifiers,
                                                 unsigned int modifiers_count)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;
   struct pipe_video_buffer *result;

   trace_dump_call_begin("pipe_screen", "create_video_buffer_with_modifiers");

   trace_dump_arg(ptr, context);
   trace_dump_arg(video_buffer_template, templat);
   trace_dump_arg_array(uint, modifiers, modifiers_count);
   trace_dump_arg(uint, modifiers_count);

   result = context->create_video_buffer_with_modifiers(context, templat, modifiers, modifiers_count);

   trace_dump_ret(ptr, result);
   trace_dump_call_end();

   result = trace_video_buffer_create(tr_context, result);

   return result;
}

static struct pipe_video_buffer *
trace_context_create_video_buffer(struct pipe_context *_context,
                                  const struct pipe_video_buffer *templat)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;
   struct pipe_video_buffer *result;

   trace_dump_call_begin("pipe_screen", "create_video_buffer");

   trace_dump_arg(ptr, context);
   trace_dump_arg(video_buffer_template, templat);

   result = context->create_video_buffer(context, templat);

   trace_dump_ret(ptr, result);
   trace_dump_call_end();

   result = trace_video_buffer_create(tr_context, result);

   return result;
}

static void
trace_context_set_tess_state(struct pipe_context *_context,
                             const float default_outer_level[4],
                             const float default_inner_level[2])
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "set_tess_state");
   trace_dump_arg(ptr, context);
   trace_dump_arg_array(float, default_outer_level, 4);
   trace_dump_arg_array(float, default_inner_level, 2);
   trace_dump_call_end();

   context->set_tess_state(context, default_outer_level, default_inner_level);
}

static void
trace_context_set_patch_vertices(struct pipe_context *_context,
                                 uint8_t patch_vertices)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "set_patch_vertices");
   trace_dump_arg(ptr, context);
   trace_dump_arg(uint, patch_vertices);
   trace_dump_call_end();

   context->set_patch_vertices(context, patch_vertices);
}

static void trace_context_set_shader_buffers(struct pipe_context *_context,
                                             enum pipe_shader_type shader,
                                             unsigned start, unsigned nr,
                                             const struct pipe_shader_buffer *buffers,
                                             unsigned writable_bitmask)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "set_shader_buffers");
   trace_dump_arg(ptr, context);
   trace_dump_arg(uint, shader);
   trace_dump_arg(uint, start);
   trace_dump_arg_begin("buffers");
   trace_dump_struct_array(shader_buffer, buffers, nr);
   trace_dump_arg_end();
   trace_dump_arg(uint, writable_bitmask);
   trace_dump_call_end();

   context->set_shader_buffers(context, shader, start, nr, buffers,
                               writable_bitmask);
}

static void trace_context_set_shader_images(struct pipe_context *_context,
                                            enum pipe_shader_type shader,
                                            unsigned start, unsigned nr,
                                            unsigned unbind_num_trailing_slots,
                                            const struct pipe_image_view *images)
{
   struct trace_context *tr_context = trace_context(_context);
   struct pipe_context *context = tr_context->pipe;

   trace_dump_call_begin("pipe_context", "set_shader_images");
   trace_dump_arg(ptr, context);
   trace_dump_arg(uint, shader);
   trace_dump_arg(uint, start);
   trace_dump_arg_begin("images");
   trace_dump_struct_array(image_view, images, nr);
   trace_dump_arg_end();
   trace_dump_arg(uint, unbind_num_trailing_slots);
   trace_dump_call_end();

   context->set_shader_images(context, shader, start, nr,
                              unbind_num_trailing_slots, images);
}

static void trace_context_launch_grid(struct pipe_context *_pipe,
                                      const struct pipe_grid_info *info)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "launch_grid");

   trace_dump_arg(ptr,  pipe);
   trace_dump_arg(grid_info, info);

   trace_dump_trace_flush();

   pipe->launch_grid(pipe, info);

   trace_dump_call_end();
}

static uint64_t trace_context_create_texture_handle(struct pipe_context *_pipe,
                                                    struct pipe_sampler_view *view,
                                                    const struct pipe_sampler_state *state)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   uint64_t handle;

   trace_dump_call_begin("pipe_context", "create_texture_handle");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg(ptr, view);
   trace_dump_arg(sampler_state, state);

   handle = pipe->create_texture_handle(pipe, view, state);

   uintptr_t *texture_handle = (uintptr_t*)(uintptr_t)handle;
   trace_dump_ret(ptr, texture_handle);

   trace_dump_call_end();

   return handle;
}

static void trace_context_delete_texture_handle(struct pipe_context *_pipe,
                                                uint64_t handle)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "delete_texture_handle");
   trace_dump_arg(ptr, pipe);
   uintptr_t *texture_handle = (uintptr_t*)(uintptr_t)handle;
   trace_dump_ret(ptr, texture_handle);
   trace_dump_call_end();

   pipe->delete_texture_handle(pipe, handle);
}

static void trace_context_make_texture_handle_resident(struct pipe_context *_pipe,
                                                       uint64_t handle,
                                                       bool resident)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "make_texture_handle_resident");
   trace_dump_arg(ptr, pipe);
   uintptr_t *texture_handle = (uintptr_t*)(uintptr_t)handle;
   trace_dump_ret(ptr, texture_handle);
   trace_dump_arg(bool, resident);
   trace_dump_call_end();

   pipe->make_texture_handle_resident(pipe, handle, resident);
}

static uint64_t trace_context_create_image_handle(struct pipe_context *_pipe,
                                                  const struct pipe_image_view *image)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;
   uint64_t handle;

   trace_dump_call_begin("pipe_context", "create_image_handle");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg_begin("image");
   trace_dump_image_view(image);
   trace_dump_arg_end();

   handle = pipe->create_image_handle(pipe, image);

   uintptr_t *image_handle = (uintptr_t*)(uintptr_t)handle;
   trace_dump_ret(ptr, image_handle);
   trace_dump_call_end();

   return handle;
}

static void trace_context_delete_image_handle(struct pipe_context *_pipe,
                                              uint64_t handle)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "delete_image_handle");
   trace_dump_arg(ptr, pipe);
   uintptr_t *image_handle = (uintptr_t*)(uintptr_t)handle;
   trace_dump_ret(ptr, image_handle);
   trace_dump_call_end();

   pipe->delete_image_handle(pipe, handle);
}

static void trace_context_make_image_handle_resident(struct pipe_context *_pipe,
                                                    uint64_t handle,
                                                    unsigned access,
                                                    bool resident)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "make_image_handle_resident");
   trace_dump_arg(ptr, pipe);
   uintptr_t *image_handle = (uintptr_t*)(uintptr_t)handle;
   trace_dump_ret(ptr, image_handle);
   trace_dump_arg(uint, access);
   trace_dump_arg(bool, resident);
   trace_dump_call_end();

   pipe->make_image_handle_resident(pipe, handle, access, resident);
}

static void trace_context_set_global_binding(struct pipe_context *_pipe,
                                             unsigned first, unsigned count,
                                             struct pipe_resource **resources,
                                             uint32_t **handles)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_global_binding");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, first);
   trace_dump_arg(uint, count);
   trace_dump_arg_array(ptr, resources, count);
   trace_dump_arg_array_val(uint, handles, count);

   pipe->set_global_binding(pipe, first, count, resources, handles);

   /* TODO: the handles are 64 bit if ADDRESS_BITS are 64, this is better than
    * nothing though
    */
   trace_dump_ret_array_val(uint, handles, count);
   trace_dump_call_end();
}

static void
trace_context_set_hw_atomic_buffers(struct pipe_context *_pipe,
                                    unsigned start_slot, unsigned count,
                                    const struct pipe_shader_buffer *buffers)
{
   struct trace_context *tr_ctx = trace_context(_pipe);
   struct pipe_context *pipe = tr_ctx->pipe;

   trace_dump_call_begin("pipe_context", "set_hw_atomic_buffers");
   trace_dump_arg(ptr, pipe);
   trace_dump_arg(uint, start_slot);
   trace_dump_arg(uint, count);

   trace_dump_arg_begin("buffers");
   trace_dump_struct_array(shader_buffer, buffers, count);
   trace_dump_arg_end();

   pipe->set_hw_atomic_buffers(pipe, start_slot, count, buffers);

   trace_dump_call_end();
}

struct pipe_context *
trace_context_create(struct trace_screen *tr_scr,
                     struct pipe_context *pipe)
{
   struct trace_context *tr_ctx;

   if (!pipe)
      goto error1;

   if (!trace_enabled())
      goto error1;

   tr_ctx = rzalloc(NULL, struct trace_context);
   if (!tr_ctx)
      goto error1;

   _mesa_hash_table_init(&tr_ctx->blend_states, tr_ctx, _mesa_hash_pointer, _mesa_key_pointer_equal);
   _mesa_hash_table_init(&tr_ctx->rasterizer_states, tr_ctx, _mesa_hash_pointer, _mesa_key_pointer_equal);
   _mesa_hash_table_init(&tr_ctx->depth_stencil_alpha_states, tr_ctx, _mesa_hash_pointer, _mesa_key_pointer_equal);

   tr_ctx->base.priv = pipe->priv; /* expose wrapped priv data */
   tr_ctx->base.screen = &tr_scr->base;
   tr_ctx->base.stream_uploader = pipe->stream_uploader;
   tr_ctx->base.const_uploader = pipe->const_uploader;

   tr_ctx->base.destroy = trace_context_destroy;

#define TR_CTX_INIT(_member) \
   tr_ctx->base . _member = pipe -> _member ? trace_context_ ## _member : NULL

   TR_CTX_INIT(draw_vbo);
   TR_CTX_INIT(draw_mesh_tasks);
   TR_CTX_INIT(draw_vertex_state);
   TR_CTX_INIT(render_condition);
   TR_CTX_INIT(render_condition_mem);
   TR_CTX_INIT(create_query);
   TR_CTX_INIT(destroy_query);
   TR_CTX_INIT(begin_query);
   TR_CTX_INIT(end_query);
   TR_CTX_INIT(get_query_result);
   TR_CTX_INIT(get_query_result_resource);
   TR_CTX_INIT(set_active_query_state);
   TR_CTX_INIT(create_blend_state);
   TR_CTX_INIT(bind_blend_state);
   TR_CTX_INIT(delete_blend_state);
   TR_CTX_INIT(create_sampler_state);
   TR_CTX_INIT(bind_sampler_states);
   TR_CTX_INIT(delete_sampler_state);
   TR_CTX_INIT(create_rasterizer_state);
   TR_CTX_INIT(bind_rasterizer_state);
   TR_CTX_INIT(delete_rasterizer_state);
   TR_CTX_INIT(create_depth_stencil_alpha_state);
   TR_CTX_INIT(bind_depth_stencil_alpha_state);
   TR_CTX_INIT(delete_depth_stencil_alpha_state);
   TR_CTX_INIT(create_fs_state);
   TR_CTX_INIT(bind_fs_state);
   TR_CTX_INIT(delete_fs_state);
   TR_CTX_INIT(create_vs_state);
   TR_CTX_INIT(bind_vs_state);
   TR_CTX_INIT(delete_vs_state);
   TR_CTX_INIT(create_gs_state);
   TR_CTX_INIT(bind_gs_state);
   TR_CTX_INIT(delete_gs_state);
   TR_CTX_INIT(create_tcs_state);
   TR_CTX_INIT(bind_tcs_state);
   TR_CTX_INIT(delete_tcs_state);
   TR_CTX_INIT(create_tes_state);
   TR_CTX_INIT(bind_tes_state);
   TR_CTX_INIT(delete_tes_state);
   TR_CTX_INIT(create_ts_state);
   TR_CTX_INIT(bind_ts_state);
   TR_CTX_INIT(delete_ts_state);
   TR_CTX_INIT(create_ms_state);
   TR_CTX_INIT(bind_ms_state);
   TR_CTX_INIT(delete_ms_state);
   TR_CTX_INIT(create_compute_state);
   TR_CTX_INIT(bind_compute_state);
   TR_CTX_INIT(delete_compute_state);
   TR_CTX_INIT(link_shader);
   TR_CTX_INIT(create_vertex_elements_state);
   TR_CTX_INIT(bind_vertex_elements_state);
   TR_CTX_INIT(delete_vertex_elements_state);
   TR_CTX_INIT(set_blend_color);
   TR_CTX_INIT(set_stencil_ref);
   TR_CTX_INIT(set_clip_state);
   TR_CTX_INIT(set_sample_mask);
   TR_CTX_INIT(set_constant_buffer);
   TR_CTX_INIT(set_framebuffer_state);
   TR_CTX_INIT(set_inlinable_constants);
   TR_CTX_INIT(set_polygon_stipple);
   TR_CTX_INIT(set_min_samples);
   TR_CTX_INIT(set_scissor_states);
   TR_CTX_INIT(set_viewport_states);
   TR_CTX_INIT(set_sampler_views);
   TR_CTX_INIT(create_sampler_view);
   TR_CTX_INIT(sampler_view_destroy);
   TR_CTX_INIT(create_surface);
   TR_CTX_INIT(surface_destroy);
   TR_CTX_INIT(set_vertex_buffers);
   TR_CTX_INIT(create_stream_output_target);
   TR_CTX_INIT(stream_output_target_destroy);
   TR_CTX_INIT(set_stream_output_targets);
   /* this is lavapipe-only and can't be traced */
   tr_ctx->base.stream_output_target_offset = pipe->stream_output_target_offset;
   TR_CTX_INIT(resource_copy_region);
   TR_CTX_INIT(blit);
   TR_CTX_INIT(flush_resource);
   TR_CTX_INIT(clear);
   TR_CTX_INIT(clear_render_target);
   TR_CTX_INIT(clear_depth_stencil);
   TR_CTX_INIT(clear_texture);
   TR_CTX_INIT(clear_buffer);
   TR_CTX_INIT(flush);
   TR_CTX_INIT(create_fence_fd);
   TR_CTX_INIT(fence_server_sync);
   TR_CTX_INIT(fence_server_signal);
   TR_CTX_INIT(generate_mipmap);
   TR_CTX_INIT(texture_barrier);
   TR_CTX_INIT(memory_barrier);
   TR_CTX_INIT(resource_commit);
   TR_CTX_INIT(create_video_codec);
   TR_CTX_INIT(create_video_buffer_with_modifiers);
   TR_CTX_INIT(create_video_buffer);
   TR_CTX_INIT(set_tess_state);
   TR_CTX_INIT(set_patch_vertices);
   TR_CTX_INIT(set_shader_buffers);
   TR_CTX_INIT(launch_grid);
   TR_CTX_INIT(set_shader_images);
   TR_CTX_INIT(create_texture_handle);
   TR_CTX_INIT(delete_texture_handle);
   TR_CTX_INIT(make_texture_handle_resident);
   TR_CTX_INIT(create_image_handle);
   TR_CTX_INIT(delete_image_handle);
   TR_CTX_INIT(make_image_handle_resident);

   tr_ctx->base.buffer_map = tr_ctx->base.texture_map = trace_context_transfer_map;
   tr_ctx->base.buffer_unmap = tr_ctx->base.texture_unmap = trace_context_transfer_unmap;
   TR_CTX_INIT(transfer_flush_region);
   TR_CTX_INIT(buffer_subdata);
   TR_CTX_INIT(texture_subdata);
   TR_CTX_INIT(invalidate_resource);
   TR_CTX_INIT(set_context_param);
   TR_CTX_INIT(set_debug_callback);
   TR_CTX_INIT(set_global_binding);
   TR_CTX_INIT(set_hw_atomic_buffers);


#undef TR_CTX_INIT

   tr_ctx->pipe = pipe;

   return &tr_ctx->base;

error1:
   return pipe;
}


/**
 * Sanity checker: check that the given context really is a
 * trace context (and not the wrapped driver's context).
 */
void
trace_context_check(const struct pipe_context *pipe)
{
   ASSERTED struct trace_context *tr_ctx = (struct trace_context *) pipe;
   assert(tr_ctx->base.destroy == trace_context_destroy);
}

/**
 * Threaded context is not wrapped, and so it may call fence functions directly
 */
struct pipe_context *
trace_get_possibly_threaded_context(struct pipe_context *pipe)
{
   return pipe->destroy == trace_context_destroy ? ((struct trace_context*)pipe)->pipe : pipe;
}
