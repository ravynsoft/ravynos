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

#ifndef DD_H_
#define DD_H_

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "pipe/p_screen.h"
#include "dd_util.h"
#include "util/u_thread.h"
#include "util/list.h"
#include "util/u_log.h"
#include "util/u_queue.h"

struct dd_context;

enum dd_dump_mode {
   DD_DUMP_ONLY_HANGS,
   DD_DUMP_ALL_CALLS,
   DD_DUMP_APITRACE_CALL,
};

struct dd_screen
{
   struct pipe_screen base;
   struct pipe_screen *screen;
   unsigned timeout_ms;
   enum dd_dump_mode dump_mode;
   bool flush_always;
   bool transfers;
   bool verbose;
   unsigned skip_count;
   unsigned apitrace_dump_call;
};

enum call_type
{
   CALL_FLUSH,
   CALL_DRAW_VBO,
   CALL_LAUNCH_GRID,
   CALL_RESOURCE_COPY_REGION,
   CALL_BLIT,
   CALL_FLUSH_RESOURCE,
   CALL_CLEAR,
   CALL_CLEAR_BUFFER,
   CALL_CLEAR_TEXTURE,
   CALL_CLEAR_RENDER_TARGET,
   CALL_CLEAR_DEPTH_STENCIL,
   CALL_GENERATE_MIPMAP,
   CALL_GET_QUERY_RESULT_RESOURCE,
   CALL_TRANSFER_MAP,
   CALL_TRANSFER_FLUSH_REGION,
   CALL_TRANSFER_UNMAP,
   CALL_BUFFER_SUBDATA,
   CALL_TEXTURE_SUBDATA,
};

struct call_resource_copy_region
{
   struct pipe_resource *dst;
   unsigned dst_level;
   unsigned dstx, dsty, dstz;
   struct pipe_resource *src;
   unsigned src_level;
   struct pipe_box src_box;
};

struct call_clear
{
   unsigned buffers;
   struct pipe_scissor_state scissor_state;
   union pipe_color_union color;
   double depth;
   unsigned stencil;
};

struct call_clear_buffer
{
   struct pipe_resource *res;
   unsigned offset;
   unsigned size;
   const void *clear_value;
   int clear_value_size;
};

struct call_generate_mipmap {
   struct pipe_resource *res;
   enum pipe_format format;
   unsigned base_level;
   unsigned last_level;
   unsigned first_layer;
   unsigned last_layer;
};

struct call_flush {
   unsigned flags;
};

struct call_draw_info {
   struct pipe_draw_info info;
   unsigned drawid_offset;
   struct pipe_draw_indirect_info indirect;
   struct pipe_draw_start_count_bias draw;
};

struct call_get_query_result_resource {
   struct pipe_query *query;
   enum pipe_query_type query_type;
   enum pipe_query_flags flags;
   enum pipe_query_value_type result_type;
   int index;
   struct pipe_resource *resource;
   unsigned offset;
};

struct call_transfer_map {
   struct pipe_transfer *transfer_ptr;
   struct pipe_transfer transfer;
   void *ptr;
};

struct call_transfer_flush_region {
   struct pipe_transfer *transfer_ptr;
   struct pipe_transfer transfer;
   struct pipe_box box;
};

struct call_transfer_unmap {
   struct pipe_transfer *transfer_ptr;
   struct pipe_transfer transfer;
};

struct call_buffer_subdata {
   struct pipe_resource *resource;
   unsigned usage;
   unsigned offset;
   unsigned size;
   const void *data;
};

struct call_texture_subdata {
   struct pipe_resource *resource;
   unsigned level;
   unsigned usage;
   struct pipe_box box;
   const void *data;
   unsigned stride;
   uintptr_t layer_stride;
};

struct dd_call
{
   enum call_type type;

   union {
      struct call_flush flush;
      struct call_draw_info draw_vbo;
      struct pipe_grid_info launch_grid;
      struct call_resource_copy_region resource_copy_region;
      struct pipe_blit_info blit;
      struct pipe_resource *flush_resource;
      struct call_clear clear;
      struct call_clear_buffer clear_buffer;
      struct call_generate_mipmap generate_mipmap;
      struct call_get_query_result_resource get_query_result_resource;
      struct call_transfer_map transfer_map;
      struct call_transfer_flush_region transfer_flush_region;
      struct call_transfer_unmap transfer_unmap;
      struct call_buffer_subdata buffer_subdata;
      struct call_texture_subdata texture_subdata;
   } info;
};

struct dd_query
{
   unsigned type;
   struct pipe_query *query;
};

struct dd_state
{
   void *cso;

   union {
      struct pipe_blend_state blend;
      struct pipe_depth_stencil_alpha_state dsa;
      struct pipe_rasterizer_state rs;
      struct pipe_sampler_state sampler;
      struct {
         struct pipe_vertex_element velems[PIPE_MAX_ATTRIBS];
         unsigned count;
      } velems;
      struct pipe_shader_state shader;
   } state;
};

struct dd_draw_state
{
   struct {
      struct dd_query *query;
      bool condition;
      unsigned mode;
   } render_cond;

   struct pipe_vertex_buffer vertex_buffers[PIPE_MAX_ATTRIBS];

   unsigned num_so_targets;
   struct pipe_stream_output_target *so_targets[PIPE_MAX_SO_BUFFERS];
   unsigned so_offsets[PIPE_MAX_SO_BUFFERS];

   struct dd_state *shaders[PIPE_SHADER_TYPES];
   struct pipe_constant_buffer constant_buffers[PIPE_SHADER_TYPES][PIPE_MAX_CONSTANT_BUFFERS];
   struct pipe_sampler_view *sampler_views[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   struct dd_state *sampler_states[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   struct pipe_image_view shader_images[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_IMAGES];
   struct pipe_shader_buffer shader_buffers[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_BUFFERS];

   struct dd_state *velems;
   struct dd_state *rs;
   struct dd_state *dsa;
   struct dd_state *blend;

   struct pipe_blend_color blend_color;
   struct pipe_stencil_ref stencil_ref;
   unsigned sample_mask;
   unsigned min_samples;
   struct pipe_clip_state clip_state;
   struct pipe_framebuffer_state framebuffer_state;
   struct pipe_poly_stipple polygon_stipple;
   struct pipe_scissor_state scissors[PIPE_MAX_VIEWPORTS];
   struct pipe_viewport_state viewports[PIPE_MAX_VIEWPORTS];
   float tess_default_levels[6];

   unsigned apitrace_call_number;
};

struct dd_draw_state_copy
{
   struct dd_draw_state base;

   /* dd_draw_state_copy does not reference real CSOs. Instead, it points to
    * these variables, which serve as storage.
    */
   struct dd_query render_cond;
   struct dd_state shaders[PIPE_SHADER_TYPES];
   struct dd_state sampler_states[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   struct dd_state velems;
   struct dd_state rs;
   struct dd_state dsa;
   struct dd_state blend;
};

struct dd_draw_record {
   struct list_head list;
   struct dd_context *dctx;

   int64_t time_before;
   int64_t time_after;
   unsigned draw_call;

   /* The fence pointers are guaranteed to be valid once driver_finished is signalled */
   struct pipe_fence_handle *prev_bottom_of_pipe;
   struct pipe_fence_handle *top_of_pipe;
   struct pipe_fence_handle *bottom_of_pipe;

   struct dd_call call;
   struct dd_draw_state_copy draw_state;

   struct util_queue_fence driver_finished;
   struct u_log_page *log_page;
};

struct dd_context
{
   struct pipe_context base;
   struct pipe_context *pipe;

   struct dd_draw_state draw_state;
   unsigned num_draw_calls;

   struct u_log_context log;

   /* Pipelined hang detection.
    *
    * Before each draw call, a new dd_draw_record is created that contains
    * a copy of all states. After each draw call, the driver's log is added
    * to this record. Additionally, deferred fences are associated to each
    * record both before and after the draw.
    *
    * The records are handed off to a separate thread which waits on the
    * records' fences. Records with signalled fences are freed. When a timeout
    * is detected, the thread dumps the records of in-flight draws.
    */
   thrd_t thread;
   mtx_t mutex;
   cnd_t cond;
   struct list_head records; /* oldest record first */
   unsigned num_records;
   bool kill_thread;
   bool api_stalled;
};


struct pipe_context *
dd_context_create(struct dd_screen *dscreen, struct pipe_context *pipe);

void
dd_init_draw_functions(struct dd_context *dctx);

void
dd_thread_join(struct dd_context *dctx);
int
dd_thread_main(void *input);

FILE *
dd_get_file_stream(struct dd_screen *dscreen, unsigned apitrace_call_number);

static inline struct dd_context *
dd_context(struct pipe_context *pipe)
{
   return (struct dd_context *)pipe;
}

static inline struct dd_screen *
dd_screen(struct pipe_screen *screen)
{
   return (struct dd_screen*)screen;
}

static inline struct dd_query *
dd_query(struct pipe_query *query)
{
   return (struct dd_query *)query;
}

static inline struct pipe_query *
dd_query_unwrap(struct pipe_query *query)
{
   if (query) {
      return dd_query(query)->query;
   } else {
      return NULL;
   }
}


#define CTX_INIT(_member) \
   dctx->base._member = dctx->pipe->_member ? dd_context_##_member : NULL

#endif /* DD_H_ */
