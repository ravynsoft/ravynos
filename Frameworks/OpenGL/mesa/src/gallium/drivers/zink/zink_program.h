/*
 * Copyright 2018 Collabora Ltd.
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

#ifndef ZINK_PROGRAM_H
#define ZINK_PROGRAM_H

#include "zink_types.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "util/u_prim.h"

struct compute_pipeline_cache_entry {
   struct zink_compute_pipeline_state state;
   VkPipeline pipeline;
};

#define ZINK_MAX_INLINED_VARIANTS 5

static inline enum zink_descriptor_type
zink_desc_type_from_vktype(VkDescriptorType type)
{
   switch (type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      return ZINK_DESCRIPTOR_TYPE_UBO;
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      return ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW;
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return ZINK_DESCRIPTOR_TYPE_SSBO;
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return ZINK_DESCRIPTOR_TYPE_IMAGE;
   default:
      unreachable("unhandled descriptor type");
   }
}

static inline VkPrimitiveTopology
zink_primitive_topology(enum mesa_prim mode)
{
   switch (mode) {
   case MESA_PRIM_POINTS:
      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

   case MESA_PRIM_LINES:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

   case MESA_PRIM_LINE_STRIP:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

   case MESA_PRIM_TRIANGLES:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

   case MESA_PRIM_TRIANGLE_STRIP:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

   case MESA_PRIM_TRIANGLE_FAN:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;

   case MESA_PRIM_LINES_ADJACENCY:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;

   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;

   case MESA_PRIM_TRIANGLES_ADJACENCY:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;

   case MESA_PRIM_PATCHES:
      return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

   case MESA_PRIM_QUADS:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;

   default:
      unreachable("unexpected enum mesa_prim");
   }
}

void
zink_delete_shader_state(struct pipe_context *pctx, void *cso);
void *
zink_create_gfx_shader_state(struct pipe_context *pctx, const struct pipe_shader_state *shader);

unsigned
zink_program_num_bindings_typed(const struct zink_program *pg, enum zink_descriptor_type type);

unsigned
zink_program_num_bindings(const struct zink_program *pg);

bool
zink_program_descriptor_is_buffer(struct zink_context *ctx, gl_shader_stage stage, enum zink_descriptor_type type, unsigned i);

void
zink_gfx_program_update(struct zink_context *ctx);
void
zink_gfx_program_update_optimal(struct zink_context *ctx);


struct zink_gfx_library_key *
zink_create_pipeline_lib(struct zink_screen *screen, struct zink_gfx_program *prog, struct zink_gfx_pipeline_state *state);
uint32_t hash_gfx_output(const void *key);
uint32_t hash_gfx_output_ds3(const void *key);
uint32_t hash_gfx_input(const void *key);
uint32_t hash_gfx_input_dynamic(const void *key);

void
zink_gfx_program_compile_queue(struct zink_context *ctx, struct zink_gfx_pipeline_cache_entry *pc_entry);

static inline unsigned
get_primtype_idx(enum mesa_prim mode)
{
   if (mode == MESA_PRIM_PATCHES)
      return 3;
   switch (u_reduced_prim(mode)) {
   case MESA_PRIM_POINTS:
      return 0;
   case MESA_PRIM_LINES:
      return 1;
   default:
      return 2;
   }
}

struct zink_gfx_program *
zink_create_gfx_program(struct zink_context *ctx,
                        struct zink_shader **stages,
                        unsigned vertices_per_patch,
                        uint32_t gfx_hash);

void
zink_destroy_gfx_program(struct zink_screen *screen,
                         struct zink_gfx_program *prog);
void
zink_gfx_lib_cache_unref(struct zink_screen *screen, struct zink_gfx_lib_cache *libs);
void
zink_program_init(struct zink_context *ctx);

uint32_t
zink_program_get_descriptor_usage(struct zink_context *ctx, gl_shader_stage stage, enum zink_descriptor_type type);

void
debug_describe_zink_gfx_program(char* buf, const struct zink_gfx_program *ptr);

static inline bool
zink_gfx_program_reference(struct zink_screen *screen,
                           struct zink_gfx_program **dst,
                           struct zink_gfx_program *src)
{
   struct zink_gfx_program *old_dst = dst ? *dst : NULL;
   bool ret = false;

   if (pipe_reference_described(old_dst ? &old_dst->base.reference : NULL, &src->base.reference,
                                (debug_reference_descriptor)debug_describe_zink_gfx_program)) {
      zink_destroy_gfx_program(screen, old_dst);
      ret = true;
   }
   if (dst) *dst = src;
   return ret;
}

void
zink_destroy_compute_program(struct zink_screen *screen,
                             struct zink_compute_program *comp);

void
debug_describe_zink_compute_program(char* buf, const struct zink_compute_program *ptr);

static inline bool
zink_compute_program_reference(struct zink_screen *screen,
                           struct zink_compute_program **dst,
                           struct zink_compute_program *src)
{
   struct zink_compute_program *old_dst = dst ? *dst : NULL;
   bool ret = false;

   if (pipe_reference_described(old_dst ? &old_dst->base.reference : NULL, &src->base.reference,
                                (debug_reference_descriptor)debug_describe_zink_compute_program)) {
      zink_destroy_compute_program(screen, old_dst);
      ret = true;
   }
   if (dst) *dst = src;
   return ret;
}

static inline bool
zink_program_reference(struct zink_screen *screen,
                       struct zink_program **dst,
                       struct zink_program *src)
{
   struct zink_program *pg = src ? src : dst ? *dst : NULL;
   if (!pg)
      return false;
   if (pg->is_compute) {
      struct zink_compute_program *comp = (struct zink_compute_program*)pg;
      return zink_compute_program_reference(screen, &comp, NULL);
   } else {
      struct zink_gfx_program *prog = (struct zink_gfx_program*)pg;
      return zink_gfx_program_reference(screen, &prog, NULL);
   }
}

VkPipelineLayout
zink_pipeline_layout_create(struct zink_screen *screen, VkDescriptorSetLayout *dsl, unsigned num_dsl, bool is_compute, VkPipelineLayoutCreateFlags flags);

void
zink_program_update_compute_pipeline_state(struct zink_context *ctx, struct zink_compute_program *comp, const struct pipe_grid_info *info);
void
zink_update_compute_program(struct zink_context *ctx);
VkPipeline
zink_get_compute_pipeline(struct zink_screen *screen,
                      struct zink_compute_program *comp,
                      struct zink_compute_pipeline_state *state);

static inline bool
zink_program_has_descriptors(const struct zink_program *pg)
{
   return pg->num_dsl > 0;
}

static inline struct zink_fs_key_base *
zink_set_fs_base_key(struct zink_context *ctx)
{
   ctx->dirty_gfx_stages |= BITFIELD_BIT(MESA_SHADER_FRAGMENT);
   return zink_screen(ctx->base.screen)->optimal_keys ?
          &ctx->gfx_pipeline_state.shader_keys_optimal.key.fs :
          &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_FRAGMENT].key.fs.base;
}

static inline const struct zink_fs_key_base *
zink_get_fs_base_key(const struct zink_context *ctx)
{
   return zink_screen(ctx->base.screen)->optimal_keys ?
          &ctx->gfx_pipeline_state.shader_keys_optimal.key.fs :
          &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_FRAGMENT].key.fs.base;
}

static inline struct zink_fs_key *
zink_set_fs_key(struct zink_context *ctx)
{
   assert(!zink_screen(ctx->base.screen)->optimal_keys);
   ctx->dirty_gfx_stages |= BITFIELD_BIT(MESA_SHADER_FRAGMENT);
   return &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_FRAGMENT].key.fs;
}

static inline const struct zink_fs_key *
zink_get_fs_key(const struct zink_context *ctx)
{
   assert(!zink_screen(ctx->base.screen)->optimal_keys);
   return &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_FRAGMENT].key.fs;
}

static inline struct zink_gs_key *
zink_set_gs_key(struct zink_context *ctx)
{
   ctx->dirty_gfx_stages |= BITFIELD_BIT(MESA_SHADER_GEOMETRY);
   assert(!zink_screen(ctx->base.screen)->optimal_keys);
   return &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_GEOMETRY].key.gs;
}

static inline const struct zink_gs_key *
zink_get_gs_key(const struct zink_context *ctx)
{
   return &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_GEOMETRY].key.gs;
}

static inline bool
zink_set_tcs_key_patches(struct zink_context *ctx, uint8_t patch_vertices)
{
   struct zink_tcs_key *tcs = zink_screen(ctx->base.screen)->optimal_keys ?
                              &ctx->gfx_pipeline_state.shader_keys_optimal.key.tcs :
                              &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_TESS_CTRL].key.tcs;
   if (tcs->patch_vertices == patch_vertices)
      return false;
   ctx->dirty_gfx_stages |= BITFIELD_BIT(MESA_SHADER_TESS_CTRL);
   tcs->patch_vertices = patch_vertices;
   return true;
}

static inline const struct zink_tcs_key *
zink_get_tcs_key(const struct zink_context *ctx)
{
   return zink_screen(ctx->base.screen)->optimal_keys ?
          &ctx->gfx_pipeline_state.shader_keys_optimal.key.tcs :
          &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_TESS_CTRL].key.tcs;
}

void
zink_update_fs_key_samples(struct zink_context *ctx);

void
zink_update_gs_key_rectangular_line(struct zink_context *ctx);

static inline struct zink_vs_key *
zink_set_vs_key(struct zink_context *ctx)
{
   ctx->dirty_gfx_stages |= BITFIELD_BIT(MESA_SHADER_VERTEX);
   assert(!zink_screen(ctx->base.screen)->optimal_keys);
   return &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_VERTEX].key.vs;
}

static inline const struct zink_vs_key *
zink_get_vs_key(const struct zink_context *ctx)
{
   assert(!zink_screen(ctx->base.screen)->optimal_keys);
   return &ctx->gfx_pipeline_state.shader_keys.key[MESA_SHADER_VERTEX].key.vs;
}

static inline struct zink_vs_key_base *
zink_set_last_vertex_key(struct zink_context *ctx)
{
   ctx->last_vertex_stage_dirty = true;
   return zink_screen(ctx->base.screen)->optimal_keys ?
          &ctx->gfx_pipeline_state.shader_keys_optimal.key.vs_base :
          &ctx->gfx_pipeline_state.shader_keys.last_vertex.key.vs_base;
}

static inline const struct zink_vs_key_base *
zink_get_last_vertex_key(const struct zink_context *ctx)
{
   return zink_screen(ctx->base.screen)->optimal_keys ?
          &ctx->gfx_pipeline_state.shader_keys_optimal.key.vs_base :
          &ctx->gfx_pipeline_state.shader_keys.last_vertex.key.vs_base;
}

static inline void
zink_set_fs_point_coord_key(struct zink_context *ctx)
{
   const struct zink_fs_key_base *fs = zink_get_fs_base_key(ctx);
   bool disable = ctx->gfx_pipeline_state.rast_prim != MESA_PRIM_POINTS;
   uint8_t coord_replace_bits = disable ? 0 : ctx->rast_state->base.sprite_coord_enable;
   bool point_coord_yinvert = disable ? false : !!ctx->rast_state->base.sprite_coord_mode;
   if (fs->coord_replace_bits != coord_replace_bits || fs->point_coord_yinvert != point_coord_yinvert) {
      zink_set_fs_base_key(ctx)->coord_replace_bits = coord_replace_bits;
      zink_set_fs_base_key(ctx)->point_coord_yinvert = point_coord_yinvert;
   }
}

void
zink_set_primitive_emulation_keys(struct zink_context *ctx);

void
zink_create_primitive_emulation_gs(struct zink_context *ctx);

static inline const struct zink_shader_key_base *
zink_get_shader_key_base(const struct zink_context *ctx, gl_shader_stage pstage)
{
   assert(!zink_screen(ctx->base.screen)->optimal_keys);
   return &ctx->gfx_pipeline_state.shader_keys.key[pstage].base;
}

static inline struct zink_shader_key_base *
zink_set_shader_key_base(struct zink_context *ctx, gl_shader_stage pstage)
{
   ctx->dirty_gfx_stages |= BITFIELD_BIT(pstage);
   assert(!zink_screen(ctx->base.screen)->optimal_keys);
   return &ctx->gfx_pipeline_state.shader_keys.key[pstage].base;
}

static inline void
zink_set_zs_needs_shader_swizzle_key(struct zink_context *ctx, gl_shader_stage pstage, bool swizzle_update)
{
   if (!zink_screen(ctx->base.screen)->driver_workarounds.needs_zs_shader_swizzle) {
      if (pstage != MESA_SHADER_FRAGMENT)
         return;
      const struct zink_fs_key_base *fs = zink_get_fs_base_key(ctx);
      bool enable = ctx->gfx_stages[MESA_SHADER_FRAGMENT] && (ctx->gfx_stages[MESA_SHADER_FRAGMENT]->fs.legacy_shadow_mask & ctx->di.zs_swizzle[pstage].mask) > 0;
      if (enable != fs->shadow_needs_shader_swizzle || (enable && swizzle_update))
         zink_set_fs_base_key(ctx)->shadow_needs_shader_swizzle = enable;
      return;
   }
   bool enable = !!ctx->di.zs_swizzle[pstage].mask;
   const struct zink_shader_key_base *key = zink_get_shader_key_base(ctx, pstage);
   if (enable != key->needs_zs_shader_swizzle || (enable && swizzle_update))
      zink_set_shader_key_base(ctx, pstage)->needs_zs_shader_swizzle = enable;
}

ALWAYS_INLINE static bool
zink_can_use_pipeline_libs(const struct zink_context *ctx)
{
   return
          /* TODO: if there's ever a dynamic render extension with input attachments */
          !ctx->gfx_pipeline_state.render_pass &&
          /* this is just terrible */
          !zink_get_fs_base_key(ctx)->shadow_needs_shader_swizzle &&
          /* TODO: is sample shading even possible to handle with GPL? */
          !ctx->gfx_stages[MESA_SHADER_FRAGMENT]->info.fs.uses_sample_shading &&
          !zink_get_fs_base_key(ctx)->fbfetch_ms &&
          !ctx->gfx_pipeline_state.force_persample_interp &&
          !ctx->gfx_pipeline_state.min_samples &&
          !ctx->is_generated_gs_bound;
}

bool
zink_set_rasterizer_discard(struct zink_context *ctx, bool disable);
void
zink_driver_thread_add_job(struct pipe_screen *pscreen, void *data,
                           struct util_queue_fence *fence,
                           pipe_driver_thread_func execute,
                           pipe_driver_thread_func cleanup,
                           const size_t job_size);
equals_gfx_pipeline_state_func
zink_get_gfx_pipeline_eq_func(struct zink_screen *screen, struct zink_gfx_program *prog);

static inline uint32_t
zink_sanitize_optimal_key(struct zink_shader **shaders, uint32_t val)
{
   union zink_shader_key_optimal k;
   if (shaders[MESA_SHADER_TESS_EVAL] && !shaders[MESA_SHADER_TESS_CTRL])
      k.val = val;
   else
      k.val = zink_shader_key_optimal_no_tcs(val);
   if (!(shaders[MESA_SHADER_FRAGMENT]->info.outputs_written & BITFIELD64_BIT(FRAG_RESULT_SAMPLE_MASK)))
      k.fs.samples = false;
   if (!(shaders[MESA_SHADER_FRAGMENT]->info.outputs_written & BITFIELD64_BIT(FRAG_RESULT_DATA1)))
      k.fs.force_dual_color_blend = false;
   return k.val;
}
#ifdef __cplusplus
}
#endif

#endif
