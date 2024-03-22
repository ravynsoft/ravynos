/*
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

#ifndef H_LIMA_CONTEXT
#define H_LIMA_CONTEXT

#include "util/list.h"
#include "util/slab.h"
#include "util/u_debug.h"

#include "pipe/p_context.h"
#include "pipe/p_state.h"

struct lima_context_framebuffer {
   struct pipe_framebuffer_state base;
   int tiled_w, tiled_h;
   int shift_w, shift_h;
   int block_w, block_h;
   int shift_min;
};

struct lima_depth_stencil_alpha_state {
   struct pipe_depth_stencil_alpha_state base;
};

struct lima_fs_compiled_shader {
   struct lima_bo *bo;
   void *shader;
   struct {
      int shader_size;
      int stack_size;
      int frag_color0_reg;
      int frag_color1_reg;
      int frag_depth_reg;
      bool uses_discard;
   } state;
};

struct lima_fs_uncompiled_shader {
   struct pipe_shader_state base;
   unsigned char nir_sha1[20];
};

struct lima_fs_key {
   unsigned char nir_sha1[20];
   struct {
      uint8_t swizzle[4];
   } tex[PIPE_MAX_SAMPLERS];
};

#define LIMA_MAX_VARYING_NUM 13

struct lima_varying_info {
   int components;
   int component_size;
   int offset;
};

struct lima_vs_compiled_shader {
   struct lima_bo *bo;
   void *shader;
   void *constant;
   struct {
      int shader_size;
      int prefetch;
      int uniform_size;
      int constant_size;
      struct lima_varying_info varying[LIMA_MAX_VARYING_NUM];
      int varying_stride;
      int num_outputs;
      int num_varyings;
      int gl_pos_idx;
      int point_size_idx;
   } state;
};

struct lima_vs_uncompiled_shader {
   struct pipe_shader_state base;
   unsigned char nir_sha1[20];
};

struct lima_vs_key {
   unsigned char nir_sha1[20];
};

struct lima_rasterizer_state {
   struct pipe_rasterizer_state base;
};

struct lima_blend_state {
   struct pipe_blend_state base;
};

struct lima_vertex_element_state {
   struct pipe_vertex_element pipe[PIPE_MAX_ATTRIBS];
   unsigned num_elements;
};

struct lima_context_vertex_buffer {
   struct pipe_vertex_buffer vb[PIPE_MAX_ATTRIBS];
   unsigned count;
   uint32_t enabled_mask;
};

struct lima_context_viewport_state {
   struct pipe_viewport_state transform;
   float left, right, bottom, top;
   float near, far;
};

struct lima_context_constant_buffer {
   const void *buffer;
   uint32_t size;
   bool dirty;
};

enum lima_ctx_buff {
   lima_ctx_buff_gp_varying_info,
   lima_ctx_buff_gp_attribute_info,
   lima_ctx_buff_gp_uniform,
   lima_ctx_buff_pp_plb_rsw,
   lima_ctx_buff_pp_uniform_array,
   lima_ctx_buff_pp_uniform,
   lima_ctx_buff_pp_tex_desc,
   lima_ctx_buff_num,
   lima_ctx_buff_num_gp = lima_ctx_buff_pp_plb_rsw,
};

struct lima_ctx_buff_state {
   struct pipe_resource *res;
   unsigned offset;
   unsigned size;
};

struct lima_texture_stateobj {
   struct pipe_sampler_view *textures[PIPE_MAX_SAMPLERS];
   unsigned num_textures;
   struct pipe_sampler_state *samplers[PIPE_MAX_SAMPLERS];
   unsigned num_samplers;
};

struct lima_ctx_plb_pp_stream_key {
   uint16_t plb_index;
   /* Coordinates are in tiles */
   uint16_t minx, miny, maxx, maxy;
   /* FB params */
   uint16_t shift_w, shift_h;
   uint16_t block_w, block_h;
};

struct lima_ctx_plb_pp_stream {
   struct list_head lru_list;
   struct lima_ctx_plb_pp_stream_key key;
   struct lima_bo *bo;
   uint32_t offset[8];
};

struct lima_pp_stream_state {
   void *map;
   uint32_t va;
   uint32_t offset[8];
};

struct lima_context {
   struct pipe_context base;

   enum {
      LIMA_CONTEXT_DIRTY_FRAMEBUFFER  = (1 << 0),
      LIMA_CONTEXT_DIRTY_CLEAR        = (1 << 1),
      LIMA_CONTEXT_DIRTY_COMPILED_VS  = (1 << 2),
      LIMA_CONTEXT_DIRTY_COMPILED_FS  = (1 << 3),
      LIMA_CONTEXT_DIRTY_VERTEX_ELEM  = (1 << 4),
      LIMA_CONTEXT_DIRTY_VERTEX_BUFF  = (1 << 5),
      LIMA_CONTEXT_DIRTY_VIEWPORT     = (1 << 6),
      LIMA_CONTEXT_DIRTY_SCISSOR      = (1 << 7),
      LIMA_CONTEXT_DIRTY_RASTERIZER   = (1 << 8),
      LIMA_CONTEXT_DIRTY_ZSA          = (1 << 9),
      LIMA_CONTEXT_DIRTY_BLEND_COLOR  = (1 << 10),
      LIMA_CONTEXT_DIRTY_BLEND        = (1 << 11),
      LIMA_CONTEXT_DIRTY_STENCIL_REF  = (1 << 12),
      LIMA_CONTEXT_DIRTY_CONST_BUFF   = (1 << 13),
      LIMA_CONTEXT_DIRTY_TEXTURES     = (1 << 14),
      LIMA_CONTEXT_DIRTY_CLIP         = (1 << 15),
      LIMA_CONTEXT_DIRTY_UNCOMPILED_VS = (1 << 16),
      LIMA_CONTEXT_DIRTY_UNCOMPILED_FS = (1 << 17),
      LIMA_CONTEXT_DIRTY_SAMPLE_MASK   = (1 << 18),
   } dirty;

   struct u_upload_mgr *uploader;
   struct blitter_context *blitter;

   struct slab_child_pool transfer_pool;

   struct lima_context_framebuffer framebuffer;
   struct lima_context_viewport_state viewport;
   /* input for PLBU_CMD_VIEWPORT_* */
   struct lima_context_viewport_state ext_viewport;
   struct pipe_scissor_state scissor;
   struct pipe_scissor_state clipped_scissor;
   struct lima_vs_compiled_shader *vs;
   struct lima_fs_compiled_shader *fs;
   struct lima_vs_uncompiled_shader *uncomp_vs;
   struct lima_fs_uncompiled_shader *uncomp_fs;
   struct lima_vertex_element_state *vertex_elements;
   struct lima_context_vertex_buffer vertex_buffers;
   struct lima_rasterizer_state *rasterizer;
   struct lima_depth_stencil_alpha_state *zsa;
   struct pipe_blend_color blend_color;
   struct lima_blend_state *blend;
   struct pipe_stencil_ref stencil_ref;
   struct pipe_clip_state clip;
   struct lima_context_constant_buffer const_buffer[PIPE_SHADER_TYPES];
   struct lima_texture_stateobj tex_stateobj;
   struct lima_pp_stream_state pp_stream;

   #define LIMA_MAX_SAMPLES 4
   unsigned sample_mask;

   unsigned min_index;
   unsigned max_index;

   #define LIMA_CTX_PLB_MIN_NUM  1
   #define LIMA_CTX_PLB_MAX_NUM  4
   #define LIMA_CTX_PLB_DEF_NUM  2
   #define LIMA_CTX_PLB_BLK_SIZE 512
   unsigned plb_size;
   unsigned plb_gp_size;

   struct lima_bo *plb[LIMA_CTX_PLB_MAX_NUM];
   struct lima_bo *gp_tile_heap[LIMA_CTX_PLB_MAX_NUM];
   uint32_t gp_tile_heap_size;
   struct lima_bo *plb_gp_stream;
   struct lima_bo *gp_output;
   uint32_t gp_output_varyings_offt;
   uint32_t gp_output_point_size_offt;

   struct hash_table *plb_pp_stream;
   struct list_head plb_pp_stream_lru_list;
   uint32_t plb_index;
   size_t plb_stream_cache_size;

   struct hash_table *fs_cache;
   struct hash_table *vs_cache;

   struct lima_ctx_buff_state buffer_state[lima_ctx_buff_num];

   /* current job */
   struct lima_job *job;

   /* map from lima_job_key to lima_job */
   struct hash_table *jobs;

   /* map from pipe_resource to lima_job which write to it */
   struct hash_table *write_jobs;

   int in_sync_fd;
   uint32_t in_sync[2];
   uint32_t out_sync[2];

   int id;

   unsigned index_offset;
   struct lima_resource *index_res;
};

static inline struct lima_context *
lima_context(struct pipe_context *pctx)
{
   return (struct lima_context *)pctx;
}

struct lima_sampler_state {
   struct pipe_sampler_state base;
};

static inline struct lima_sampler_state *
lima_sampler_state(struct pipe_sampler_state *psstate)
{
   return (struct lima_sampler_state *)psstate;
}

struct lima_sampler_view {
   struct pipe_sampler_view base;
   uint8_t swizzle[4];
};

static inline struct lima_sampler_view *
lima_sampler_view(struct pipe_sampler_view *psview)
{
   return (struct lima_sampler_view *)psview;
}

uint32_t lima_ctx_buff_va(struct lima_context *ctx, enum lima_ctx_buff buff);
void *lima_ctx_buff_map(struct lima_context *ctx, enum lima_ctx_buff buff);
void *lima_ctx_buff_alloc(struct lima_context *ctx, enum lima_ctx_buff buff,
                          unsigned size);

void lima_state_init(struct lima_context *ctx);
void lima_state_fini(struct lima_context *ctx);
void lima_draw_init(struct lima_context *ctx);
void lima_program_init(struct lima_context *ctx);
void lima_program_fini(struct lima_context *ctx);
void lima_query_init(struct lima_context *ctx);

struct pipe_context *
lima_context_create(struct pipe_screen *pscreen, void *priv, unsigned flags);

void lima_flush(struct lima_context *ctx);
void lima_flush_job_accessing_bo(
   struct lima_context *ctx, struct lima_bo *bo, bool write);
void lima_flush_previous_job_writing_resource(
   struct lima_context *ctx, struct pipe_resource *prsc);

#endif
