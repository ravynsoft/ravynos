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
#ifndef VIRGL_CONTEXT_H
#define VIRGL_CONTEXT_H

#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "util/slab.h"
#include "util/list.h"

#include "virgl_staging_mgr.h"
#include "virgl_transfer_queue.h"

struct pipe_screen;
struct tgsi_token;
struct u_upload_mgr;
struct virgl_cmd_buf;
struct virgl_vertex_elements_state;

struct virgl_sampler_view {
   struct pipe_sampler_view base;
   uint32_t handle;
};

struct virgl_so_target {
   struct pipe_stream_output_target base;
   uint32_t handle;
};

struct virgl_rasterizer_state {
   struct pipe_rasterizer_state rs;
   uint32_t handle;
};

struct virgl_shader_binding_state {
   struct pipe_sampler_view *views[PIPE_MAX_SHADER_SAMPLER_VIEWS];

   struct pipe_constant_buffer ubos[PIPE_MAX_CONSTANT_BUFFERS];
   uint32_t ubo_enabled_mask;

   struct pipe_shader_buffer ssbos[PIPE_MAX_SHADER_BUFFERS];
   uint32_t ssbo_enabled_mask;

   struct pipe_image_view images[PIPE_MAX_SHADER_IMAGES];
   uint32_t image_enabled_mask;
};

struct virgl_context {
   struct pipe_context base;
   struct virgl_cmd_buf *cbuf;
   unsigned cbuf_initial_cdw;

   struct virgl_shader_binding_state shader_bindings[PIPE_SHADER_TYPES];
   struct pipe_shader_buffer atomic_buffers[PIPE_MAX_HW_ATOMIC_BUFFERS];
   uint32_t atomic_buffer_enabled_mask;

   struct virgl_vertex_elements_state *vertex_elements;

   struct pipe_framebuffer_state framebuffer;

   struct slab_child_pool transfer_pool;
   struct virgl_transfer_queue queue;
   struct u_upload_mgr *uploader;
   struct virgl_staging_mgr staging;
   bool encoded_transfers;
   bool supports_staging;
   uint8_t patch_vertices;

   struct pipe_vertex_buffer vertex_buffer[PIPE_MAX_ATTRIBS];
   unsigned num_vertex_buffers;
   bool vertex_array_dirty;

   struct virgl_rasterizer_state rs_state;
   struct virgl_so_target so_targets[PIPE_MAX_SO_BUFFERS];
   unsigned num_so_targets;

   uint32_t num_draws, num_compute;

   struct primconvert_context *primconvert;
   uint32_t hw_sub_ctx_id;

   /* The total size of staging resources used in queued copy transfers. */
   uint64_t queued_staging_res_size;
};

struct virgl_vertex_elements_state {
   uint32_t handle;
   uint8_t binding_map[PIPE_MAX_ATTRIBS];
   uint8_t num_bindings;
   uint16_t strides[PIPE_MAX_ATTRIBS];
};

static inline struct virgl_sampler_view *
virgl_sampler_view(struct pipe_sampler_view *view)
{
   return (struct virgl_sampler_view *)view;
};

static inline struct virgl_so_target *
virgl_so_target(struct pipe_stream_output_target *target)
{
   return (struct virgl_so_target *)target;
}

static inline struct virgl_context *virgl_context(struct pipe_context *ctx)
{
   return (struct virgl_context *)ctx;
}

struct pipe_context *virgl_context_create(struct pipe_screen *pscreen,
                                          void *priv, unsigned flags);

void virgl_init_blit_functions(struct virgl_context *vctx);
void virgl_init_query_functions(struct virgl_context *vctx);
void virgl_init_so_functions(struct virgl_context *vctx);

struct tgsi_token *virgl_tgsi_transform(struct virgl_screen *vscreen, const struct tgsi_token *tokens_in,
                                        bool is_separable);

bool
virgl_can_rebind_resource(struct virgl_context *vctx,
                          struct pipe_resource *res);

void
virgl_rebind_resource(struct virgl_context *vctx,
                      struct pipe_resource *res);

void virgl_flush_eq(struct virgl_context *ctx, void *closure, struct pipe_fence_handle **fence);

#endif
