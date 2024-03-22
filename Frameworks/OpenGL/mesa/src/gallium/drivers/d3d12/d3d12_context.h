/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_CONTEXT_H
#define D3D12_CONTEXT_H

#include "d3d12_batch.h"
#include "d3d12_descriptor_pool.h"
#include "d3d12_pipeline_state.h"

#include "dxil_nir_lower_int_samplers.h"

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/list.h"
#include "util/slab.h"
#include "util/u_suballoc.h"
#include "util/u_threaded_context.h"

#define D3D12_GFX_SHADER_STAGES (PIPE_SHADER_TYPES - 1)

enum d3d12_dirty_flags
{
   D3D12_DIRTY_NONE             = 0,
   D3D12_DIRTY_BLEND            = (1 << 0),
   D3D12_DIRTY_RASTERIZER       = (1 << 1),
   D3D12_DIRTY_ZSA              = (1 << 2),
   D3D12_DIRTY_VERTEX_ELEMENTS  = (1 << 3),
   D3D12_DIRTY_BLEND_COLOR      = (1 << 4),
   D3D12_DIRTY_STENCIL_REF      = (1 << 5),
   D3D12_DIRTY_SAMPLE_MASK      = (1 << 6),
   D3D12_DIRTY_VIEWPORT         = (1 << 7),
   D3D12_DIRTY_FRAMEBUFFER      = (1 << 8),
   D3D12_DIRTY_SCISSOR          = (1 << 9),
   D3D12_DIRTY_VERTEX_BUFFERS   = (1 << 10),
   D3D12_DIRTY_INDEX_BUFFER     = (1 << 11),
   D3D12_DIRTY_PRIM_MODE        = (1 << 12),
   D3D12_DIRTY_SHADER           = (1 << 13),
   D3D12_DIRTY_ROOT_SIGNATURE   = (1 << 14),
   D3D12_DIRTY_STREAM_OUTPUT    = (1 << 15),
   D3D12_DIRTY_STRIP_CUT_VALUE  = (1 << 16),
   D3D12_DIRTY_COMPUTE_SHADER   = (1 << 17),
   D3D12_DIRTY_COMPUTE_ROOT_SIGNATURE = (1 << 18),
};

enum d3d12_shader_dirty_flags
{
   D3D12_SHADER_DIRTY_CONSTBUF      = (1 << 0),
   D3D12_SHADER_DIRTY_SAMPLER_VIEWS = (1 << 1),
   D3D12_SHADER_DIRTY_SAMPLERS      = (1 << 2),
   D3D12_SHADER_DIRTY_SSBO          = (1 << 3),
   D3D12_SHADER_DIRTY_IMAGE         = (1 << 4),
};

#define D3D12_DIRTY_GFX_PSO (D3D12_DIRTY_BLEND | D3D12_DIRTY_RASTERIZER | D3D12_DIRTY_ZSA | \
                             D3D12_DIRTY_FRAMEBUFFER | D3D12_DIRTY_SAMPLE_MASK | \
                             D3D12_DIRTY_VERTEX_ELEMENTS | D3D12_DIRTY_PRIM_MODE | \
                             D3D12_DIRTY_SHADER | D3D12_DIRTY_ROOT_SIGNATURE | \
                             D3D12_DIRTY_STRIP_CUT_VALUE | D3D12_DIRTY_STREAM_OUTPUT)
#define D3D12_DIRTY_COMPUTE_PSO (D3D12_DIRTY_COMPUTE_SHADER | D3D12_DIRTY_COMPUTE_ROOT_SIGNATURE)

#define D3D12_DIRTY_COMPUTE_MASK (D3D12_DIRTY_COMPUTE_SHADER | D3D12_DIRTY_COMPUTE_ROOT_SIGNATURE)
#define D3D12_DIRTY_GFX_MASK ~D3D12_DIRTY_COMPUTE_MASK


#define D3D12_SHADER_DIRTY_ALL (D3D12_SHADER_DIRTY_CONSTBUF | D3D12_SHADER_DIRTY_SAMPLER_VIEWS | \
                                D3D12_SHADER_DIRTY_SAMPLERS | D3D12_SHADER_DIRTY_SSBO | \
                                D3D12_SHADER_DIRTY_IMAGE)

enum d3d12_binding_type {
   D3D12_BINDING_CONSTANT_BUFFER,
   D3D12_BINDING_SHADER_RESOURCE_VIEW,
   D3D12_BINDING_SAMPLER,
   D3D12_BINDING_STATE_VARS,
   D3D12_BINDING_SSBO,
   D3D12_BINDING_IMAGE,
   D3D12_NUM_BINDING_TYPES
};

struct d3d12_sampler_state {
   struct d3d12_descriptor_handle handle, handle_without_shadow;
   bool is_integer_texture;
   bool is_shadow_sampler;
   enum pipe_tex_wrap wrap_r;
   enum pipe_tex_wrap wrap_s;
   enum pipe_tex_wrap wrap_t;
   enum pipe_tex_filter filter;
   float lod_bias;
   float min_lod, max_lod;
   float border_color[4];
   enum pipe_compare_func compare_func;
};

enum d3d12_blend_factor_flags {
   D3D12_BLEND_FACTOR_NONE  = 0,
   D3D12_BLEND_FACTOR_COLOR = 1 << 0,
   D3D12_BLEND_FACTOR_ALPHA = 1 << 1,
   D3D12_BLEND_FACTOR_ANY   = 1 << 2,
};

struct d3d12_sampler_view {
   struct pipe_sampler_view base;
   struct d3d12_descriptor_handle handle;
   unsigned mip_levels;
   unsigned array_size;
   unsigned texture_generation_id;
   unsigned swizzle_override_r:3;         /**< PIPE_SWIZZLE_x for red component */
   unsigned swizzle_override_g:3;         /**< PIPE_SWIZZLE_x for green component */
   unsigned swizzle_override_b:3;         /**< PIPE_SWIZZLE_x for blue component */
   unsigned swizzle_override_a:3;         /**< PIPE_SWIZZLE_x for alpha component */
};

static inline struct d3d12_sampler_view *
d3d12_sampler_view(struct pipe_sampler_view *pview)
{
   return (struct d3d12_sampler_view *)pview;
}

struct d3d12_stream_output_target {
   struct pipe_stream_output_target base;
   struct pipe_resource *fill_buffer;
   unsigned fill_buffer_offset;
};

struct d3d12_shader_state {
   struct d3d12_shader *current;
   unsigned state_dirty;
};

struct blitter_context;
struct primconvert_context;

#ifdef _WIN32
struct dxil_validator;
#endif

#ifdef __cplusplus
class ResourceStateManager;
#endif

#define D3D12_CONTEXT_NO_ID 0xffffffff

struct d3d12_context {
   struct pipe_context base;

   unsigned id;
   struct slab_child_pool transfer_pool;
   struct slab_child_pool transfer_pool_unsync;
   struct list_head context_list_entry;
   struct threaded_context *threaded_context;
   struct primconvert_context *primconvert;
   struct blitter_context *blitter;
   struct u_suballocator query_allocator;
   struct u_suballocator so_allocator;
   struct hash_table *pso_cache;
   struct hash_table *compute_pso_cache;
   struct hash_table *root_signature_cache;
   struct hash_table *cmd_signature_cache;
   struct hash_table *gs_variant_cache;
   struct hash_table *tcs_variant_cache;
   struct hash_table *compute_transform_cache;
   struct hash_table_u64 *bo_state_table;

   struct d3d12_batch batches[8];
   unsigned current_batch_idx;

   struct util_dynarray recently_destroyed_bos;
   struct util_dynarray barrier_scratch;
   struct set *pending_barriers_bos;
   struct util_dynarray local_pending_barriers_bos;

   struct pipe_constant_buffer cbufs[PIPE_SHADER_TYPES][PIPE_MAX_CONSTANT_BUFFERS];
   struct pipe_framebuffer_state fb;
   struct pipe_vertex_buffer vbs[PIPE_MAX_ATTRIBS];
   D3D12_VERTEX_BUFFER_VIEW vbvs[PIPE_MAX_ATTRIBS];
   unsigned num_vbs;
   float flip_y;
   bool need_zero_one_depth_range;
   enum mesa_prim initial_api_prim;
   struct pipe_viewport_state viewport_states[PIPE_MAX_VIEWPORTS];
   D3D12_VIEWPORT viewports[PIPE_MAX_VIEWPORTS];
   unsigned num_viewports;
   struct pipe_scissor_state scissor_states[PIPE_MAX_VIEWPORTS];
   D3D12_RECT scissors[PIPE_MAX_VIEWPORTS];
   float blend_factor[4];
   struct pipe_stencil_ref stencil_ref;
   struct pipe_sampler_view *sampler_views[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];
   unsigned num_sampler_views[PIPE_SHADER_TYPES];
   unsigned has_int_samplers;
   struct pipe_shader_buffer ssbo_views[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_BUFFERS];
   unsigned num_ssbo_views[PIPE_SHADER_TYPES];
   struct pipe_image_view image_views[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_IMAGES];
   enum pipe_format image_view_emulation_formats[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_IMAGES];
   unsigned num_image_views[PIPE_SHADER_TYPES];
   struct d3d12_sampler_state *samplers[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   unsigned num_samplers[PIPE_SHADER_TYPES];
   D3D12_INDEX_BUFFER_VIEW ibv;

   dxil_wrap_sampler_state tex_wrap_states[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];
   dxil_wrap_sampler_state tex_wrap_states_shader_key[PIPE_MAX_SHADER_SAMPLER_VIEWS];

   dxil_texture_swizzle_state tex_swizzle_state[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];
   enum compare_func tex_compare_func[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];

   struct {
      bool enabled;
      uint32_t pattern[32];
      struct pipe_resource *texture;
      struct pipe_sampler_view *sampler_view;
      struct d3d12_sampler_state *sampler_cso;
   } pstipple;

   struct pipe_stream_output_target *so_targets[PIPE_MAX_SO_BUFFERS];
   D3D12_STREAM_OUTPUT_BUFFER_VIEW so_buffer_views[PIPE_MAX_SO_BUFFERS];
   struct pipe_stream_output_target *fake_so_targets[PIPE_MAX_SO_BUFFERS];
   D3D12_STREAM_OUTPUT_BUFFER_VIEW fake_so_buffer_views[PIPE_MAX_SO_BUFFERS];
   unsigned fake_so_buffer_factor;
   uint8_t patch_vertices;
   float default_outer_tess_factor[4];
   float default_inner_tess_factor[2];

   struct d3d12_shader_selector *gfx_stages[D3D12_GFX_SHADER_STAGES];
   struct d3d12_shader_selector *compute_state;

   bool has_flat_varyings;
   bool missing_dual_src_outputs;
   bool manual_depth_range;

   struct d3d12_gfx_pipeline_state gfx_pipeline_state;
   struct d3d12_compute_pipeline_state compute_pipeline_state;
   unsigned shader_dirty[PIPE_SHADER_TYPES];
   unsigned state_dirty;
   unsigned cmdlist_dirty;
   ID3D12PipelineState *current_gfx_pso;
   ID3D12PipelineState *current_compute_pso;
   uint16_t reverse_depth_range;

   uint64_t submit_id;
   ID3D12GraphicsCommandList *cmdlist;
   ID3D12GraphicsCommandList2 *cmdlist2;
   ID3D12GraphicsCommandList8 *cmdlist8;
   ID3D12GraphicsCommandList *state_fixup_cmdlist;

   struct list_head active_queries;
   bool queries_disabled;

   struct d3d12_descriptor_pool *sampler_pool;
   struct d3d12_descriptor_handle null_sampler;

   PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature;
#ifndef _GAMING_XBOX
   ID3D12DeviceConfiguration *dev_config;
#endif
#ifdef _WIN32
   struct dxil_validator *dxil_validator;
#endif

   struct d3d12_resource *current_predication;
   bool predication_condition;
   bool queries_suspended;

   uint32_t transform_state_vars[8];

#ifdef __cplusplus
   ResourceStateManager *resource_state_manager;
#else
   void *resource_state_manager; /* opaque pointer; we don't know about classes in C */
#endif
   struct pipe_query *timestamp_query;

   /* used by d3d12_blit.cpp */
   void *stencil_resolve_vs, *stencil_resolve_fs, *stencil_resolve_fs_no_flip, *sampler_state;
};

static inline struct d3d12_context *
d3d12_context(struct pipe_context *context)
{
   return (struct d3d12_context *)context;
}

static inline struct d3d12_batch *
d3d12_current_batch(struct d3d12_context *ctx)
{
   assert(ctx->current_batch_idx < ARRAY_SIZE(ctx->batches));
   return ctx->batches + ctx->current_batch_idx;
}

#define d3d12_foreach_submitted_batch(ctx, batch) \
   unsigned oldest = (ctx->current_batch_idx + 1) % ARRAY_SIZE(ctx->batches); \
   while (ctx->batches[oldest].fence == NULL && oldest != ctx->current_batch_idx) \
      oldest = (oldest + 1) % ARRAY_SIZE(ctx->batches); \
   struct d3d12_batch *batch = &ctx->batches[oldest]; \
   for (; oldest != ctx->current_batch_idx; \
        oldest = (oldest + 1) % ARRAY_SIZE(ctx->batches), \
        batch = &ctx->batches[oldest])

struct pipe_context *
d3d12_context_create(struct pipe_screen *pscreen, void *priv, unsigned flags);

bool
d3d12_enable_fake_so_buffers(struct d3d12_context *ctx, unsigned factor);

bool
d3d12_disable_fake_so_buffers(struct d3d12_context *ctx);

void
d3d12_flush_cmdlist(struct d3d12_context *ctx);

void
d3d12_flush_cmdlist_and_wait(struct d3d12_context *ctx);


enum d3d12_transition_flags {
   D3D12_TRANSITION_FLAG_NONE = 0,
   D3D12_TRANSITION_FLAG_INVALIDATE_BINDINGS = 1,
   D3D12_TRANSITION_FLAG_ACCUMULATE_STATE = 2,
   D3D12_TRANSITION_FLAG_PENDING_MEMORY_BARRIER = 4,
};

void
d3d12_transition_resource_state(struct d3d12_context* ctx,
                                struct d3d12_resource* res,
                                D3D12_RESOURCE_STATES state,
                                d3d12_transition_flags flags);

void
d3d12_transition_subresources_state(struct d3d12_context *ctx,
                                    struct d3d12_resource *res,
                                    unsigned start_level, unsigned num_levels,
                                    unsigned start_layer, unsigned num_layers,
                                    unsigned start_plane, unsigned num_planes,
                                    D3D12_RESOURCE_STATES state,
                                    d3d12_transition_flags flags);

void
d3d12_apply_resource_states(struct d3d12_context* ctx, bool is_implicit_dispatch);

void
d3d12_draw_vbo(struct pipe_context *pctx,
               const struct pipe_draw_info *dinfo,
               unsigned drawid_offset,
               const struct pipe_draw_indirect_info *indirect,
               const struct pipe_draw_start_count_bias *draws,
               unsigned num_draws);

void
d3d12_launch_grid(struct pipe_context *pctx,
                  const struct pipe_grid_info *info);

void
d3d12_blit(struct pipe_context *pctx,
           const struct pipe_blit_info *info);

void
d3d12_context_query_init(struct pipe_context *pctx);

bool
d3d12_need_zero_one_depth_range(struct d3d12_context *ctx);

void
d3d12_init_sampler_view_descriptor(struct d3d12_sampler_view *sampler_view);

void
d3d12_invalidate_context_bindings(struct d3d12_context *ctx, struct d3d12_resource *res);

#ifdef HAVE_GALLIUM_D3D12_VIDEO
struct pipe_video_codec* d3d12_video_create_codec( struct pipe_context *context,
                                                const struct pipe_video_codec *t);
#endif

#endif
