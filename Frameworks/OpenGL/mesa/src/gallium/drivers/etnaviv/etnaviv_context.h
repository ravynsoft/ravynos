/*
 * Copyright (c) 2012-2015 Etnaviv Project
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
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef H_ETNAVIV_CONTEXT
#define H_ETNAVIV_CONTEXT

#include <stdint.h>

#include "etnaviv_resource.h"
#include "etnaviv_tiling.h"
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "util/format/u_formats.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_state.h"
#include "util/slab.h"
#include <util/u_suballoc.h>

struct pipe_screen;
struct etna_shader_variant;
struct etna_sampler_ts;

struct etna_index_buffer {
   struct etna_reloc FE_INDEX_STREAM_BASE_ADDR;
   uint32_t FE_INDEX_STREAM_CONTROL;
   uint32_t FE_PRIMITIVE_RESTART_INDEX;
};

struct etna_shader_input {
   int vs_reg; /* VS input register */
};

enum etna_varying_special {
   ETNA_VARYING_VSOUT = 0, /* from VS */
   ETNA_VARYING_POINTCOORD, /* point texture coord */
};

struct etna_shader_varying {
   int num_components;
   enum etna_varying_special special;
   int pa_attributes;
   int vs_reg; /* VS output register */
};

struct etna_transfer {
   struct pipe_transfer base;
   struct pipe_resource *rsc;
   void *staging;
   void *mapped;
};

struct etna_constbuf_state {
   struct pipe_constant_buffer cb[ETNA_MAX_CONST_BUF];
   uint32_t enabled_mask;
};

struct etna_vertexbuf_state {
   struct pipe_vertex_buffer vb[PIPE_MAX_ATTRIBS];
   struct compiled_set_vertex_buffer cvb[PIPE_MAX_ATTRIBS];
   unsigned count;
   uint32_t enabled_mask;
};

struct etna_shader_state {
   void *bind_vs, *bind_fs;
   struct etna_shader_variant *vs, *fs;
};

enum etna_uniform_contents {
   ETNA_UNIFORM_UNUSED = 0,
   ETNA_UNIFORM_CONSTANT,
   ETNA_UNIFORM_UNIFORM,
   ETNA_UNIFORM_TEXRECT_SCALE_X,
   ETNA_UNIFORM_TEXRECT_SCALE_Y,
   ETNA_UNIFORM_TEXTURE_WIDTH,
   ETNA_UNIFORM_TEXTURE_HEIGHT,
   ETNA_UNIFORM_TEXTURE_DEPTH,
   ETNA_UNIFORM_UBO0_ADDR,
   ETNA_UNIFORM_UBOMAX_ADDR = ETNA_UNIFORM_UBO0_ADDR + ETNA_MAX_CONST_BUF - 1,
};

struct etna_shader_uniform_info {
   enum etna_uniform_contents *contents;
   uint32_t *data;
   uint32_t count;
};

struct etna_context {
   struct pipe_context base;

   /* GPU-specific implementation to emit texture state */
   void (*emit_texture_state)(struct etna_context *pctx);
   /* Get sampler TS pointer for sampler view */
   struct etna_sampler_ts *(*ts_for_sampler_view)(struct pipe_sampler_view *pview);
   /* GPU-specific blit implementation */
   bool (*blit)(struct pipe_context *pipe, const struct pipe_blit_info *info);

   struct etna_screen *screen;
   struct etna_cmd_stream *stream;

   /* which state objects need to be re-emit'd: */
   enum {
      ETNA_DIRTY_BLEND           = (1 << 0),
      ETNA_DIRTY_SAMPLERS        = (1 << 1),
      ETNA_DIRTY_RASTERIZER      = (1 << 2),
      ETNA_DIRTY_ZSA             = (1 << 3),
      ETNA_DIRTY_VERTEX_ELEMENTS = (1 << 4),
      ETNA_DIRTY_BLEND_COLOR     = (1 << 6),
      ETNA_DIRTY_STENCIL_REF     = (1 << 7),
      ETNA_DIRTY_SAMPLE_MASK     = (1 << 8),
      ETNA_DIRTY_VIEWPORT        = (1 << 9),
      ETNA_DIRTY_FRAMEBUFFER     = (1 << 10),
      ETNA_DIRTY_SCISSOR         = (1 << 11),
      ETNA_DIRTY_SAMPLER_VIEWS   = (1 << 12),
      ETNA_DIRTY_CONSTBUF        = (1 << 13),
      ETNA_DIRTY_VERTEX_BUFFERS  = (1 << 14),
      ETNA_DIRTY_INDEX_BUFFER    = (1 << 15),
      ETNA_DIRTY_SHADER          = (1 << 16),
      ETNA_DIRTY_TS              = (1 << 17),
      ETNA_DIRTY_TEXTURE_CACHES  = (1 << 18),
      ETNA_DIRTY_DERIVE_TS       = (1 << 19),
      ETNA_DIRTY_SCISSOR_CLIP    = (1 << 20),
   } dirty;

   struct slab_child_pool transfer_pool;
   struct u_suballocator tex_desc_allocator;
   struct blitter_context *blitter;

   /* compiled bindable state */
   unsigned sample_mask;
   struct pipe_blend_state *blend;
   unsigned num_fragment_samplers;
   uint32_t active_samplers;
   uint32_t prev_active_samplers;
   struct pipe_sampler_state *sampler[PIPE_MAX_SAMPLERS];
   struct pipe_rasterizer_state *rasterizer;
   struct pipe_depth_stencil_alpha_state *zsa;
   struct compiled_vertex_elements_state *vertex_elements;
   struct compiled_shader_state shader_state;
   struct pipe_scissor_state clipping;

   /* to simplify the emit process we store pre compiled state objects,
    * which got 'compiled' during state change. */
   struct compiled_blend_color blend_color;
   struct compiled_stencil_ref stencil_ref;
   struct compiled_framebuffer_state framebuffer;
   struct compiled_viewport_state viewport;
   unsigned num_fragment_sampler_views;
   uint32_t active_sampler_views;
   uint32_t dirty_sampler_views;
   struct pipe_sampler_view *sampler_view[PIPE_MAX_SAMPLERS];
   struct etna_constbuf_state constant_buffer[PIPE_SHADER_TYPES];
   struct etna_vertexbuf_state vertex_buffer;
   struct etna_index_buffer index_buffer;
   struct etna_shader_state shader;

   /* saved parameter-like state. these are mainly kept around for the blitter */
   struct pipe_framebuffer_state framebuffer_s;
   struct pipe_stencil_ref stencil_ref_s;
   struct pipe_viewport_state viewport_s;
   struct pipe_scissor_state scissor;

   /* stats/counters */
   struct {
      uint64_t prims_generated;
      uint64_t draw_calls;
      uint64_t rs_operations;
   } stats;

   int in_fence_fd;

   /* list of accumulated HW queries */
   struct list_head active_acc_queries;

   /* set of resources used by currently-unsubmitted renders */
   struct hash_table *pending_resources;

   /* resources that must be flushed implicitly at the context flush time */
   struct set *flush_resources;

   bool is_noop;

   /* conditional rendering */
   struct pipe_query *cond_query;
   bool cond_cond; /* inverted rendering condition */
   uint cond_mode;
};

static inline struct etna_context *
etna_context(struct pipe_context *pctx)
{
   return (struct etna_context *)pctx;
}

static inline struct etna_transfer *
etna_transfer(struct pipe_transfer *p)
{
   return (struct etna_transfer *)p;
}

struct pipe_context *
etna_context_create(struct pipe_screen *pscreen, void *priv, unsigned flags);

void
etna_context_add_flush_resource(struct etna_context *ctx,
                                struct pipe_resource *rsc);

void
etna_flush(struct pipe_context *pctx, struct pipe_fence_handle **fence,
           enum pipe_flush_flags flags, bool internal);

bool
etna_render_condition_check(struct pipe_context *pctx);

#endif
