/*
 * Copyright 2021 Alyssa Rosenzweig
 * Copyright 2019-2021 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#ifndef AGX_STATE_H
#define AGX_STATE_H

#include "asahi/compiler/agx_compile.h"
#include "asahi/layout/layout.h"
#include "asahi/lib/agx_bo.h"
#include "asahi/lib/agx_device.h"
#include "asahi/lib/agx_nir_lower_vbo.h"
#include "asahi/lib/agx_pack.h"
#include "asahi/lib/agx_tilebuffer.h"
#include "asahi/lib/pool.h"
#include "asahi/lib/shaders/geometry.h"
#include "compiler/nir/nir_lower_blend.h"
#include "compiler/shader_enums.h"
#include "gallium/auxiliary/util/u_blitter.h"
#include "gallium/include/pipe/p_context.h"
#include "gallium/include/pipe/p_screen.h"
#include "gallium/include/pipe/p_state.h"
#include "util/bitset.h"
#include "util/disk_cache.h"
#include "util/hash_table.h"
#include "util/u_range.h"
#include "agx_helpers.h"
#include "agx_meta.h"

#ifdef __GLIBC__
#include <errno.h>
#define agx_msg(fmt, ...)                                                      \
   fprintf(stderr, "[%s] " fmt, program_invocation_short_name, ##__VA_ARGS__)
#else
#define agx_msg(...) fprintf(stderr, __VA_ARGS__)
#endif

#define AGX_NUM_TEXTURE_STATE_REGS 16

struct agx_streamout_target {
   struct pipe_stream_output_target base;
   struct pipe_resource *offset;

   /* Current stride (bytes per vertex) */
   uint32_t stride;
};

static inline struct agx_streamout_target *
agx_so_target(struct pipe_stream_output_target *target)
{
   return (struct agx_streamout_target *)target;
}

struct agx_streamout {
   struct pipe_stream_output_target *targets[PIPE_MAX_SO_BUFFERS];
   unsigned num_targets;
};

/* Shaders can access fixed-function state through system values.
 * It is convenient to stash all of this information into a single "root"
 * descriptor, then push individual parts as needed.
 *
 * In the future, we could optimize this to reduce CPU overhead, e.g. splitting
 * into multiple descriptors for finer dirty tracking. This is not ABI with the
 * compiler. The layout is up to us and handled by our code lowering system
 * values to uniforms.
 */
enum agx_sysval_table {
   AGX_SYSVAL_TABLE_ROOT,
   AGX_SYSVAL_TABLE_PARAMS,
   AGX_SYSVAL_TABLE_GRID,
   AGX_SYSVAL_TABLE_VS,
   AGX_SYSVAL_TABLE_TCS,
   AGX_SYSVAL_TABLE_TES,
   AGX_SYSVAL_TABLE_GS,
   AGX_SYSVAL_TABLE_FS,
   AGX_SYSVAL_TABLE_CS,
   AGX_NUM_SYSVAL_TABLES
};

#define AGX_SYSVAL_STAGE(stage) (AGX_SYSVAL_TABLE_VS + (stage))

static_assert(AGX_SYSVAL_STAGE(PIPE_SHADER_VERTEX) == AGX_SYSVAL_TABLE_VS,
              "fixed enum orderings");
static_assert(AGX_SYSVAL_STAGE(PIPE_SHADER_TESS_CTRL) == AGX_SYSVAL_TABLE_TCS,
              "fixed enum orderings");
static_assert(AGX_SYSVAL_STAGE(PIPE_SHADER_TESS_EVAL) == AGX_SYSVAL_TABLE_TES,
              "fixed enum orderings");
static_assert(AGX_SYSVAL_STAGE(PIPE_SHADER_GEOMETRY) == AGX_SYSVAL_TABLE_GS,
              "fixed enum orderings");
static_assert(AGX_SYSVAL_STAGE(PIPE_SHADER_FRAGMENT) == AGX_SYSVAL_TABLE_FS,
              "fixed enum orderings");
static_assert(AGX_SYSVAL_STAGE(PIPE_SHADER_COMPUTE) == AGX_SYSVAL_TABLE_CS,
              "fixed enum orderings");

/* Root system value table */
struct PACKED agx_draw_uniforms {
   /* Pointers to the system value tables themselves (for indirection) */
   uint64_t tables[AGX_NUM_SYSVAL_TABLES];

   /* Vertex buffer object bases, if present */
   uint64_t vbo_base[PIPE_MAX_ATTRIBS];

   /* Address of input assembly buffer if geom/tess is used, else 0 */
   uint64_t input_assembly;

   /* Address of geometry param buffer if geometry shaders are used, else 0 */
   uint64_t geometry_params;

   /* Blend constant if any */
   float blend_constant[4];

   /* glPointSize value */
   float fixed_point_size;

   /* Value of the multisample control register, containing sample positions in
    * each byte (x in low nibble, y in high nibble).
    */
   uint32_t ppp_multisamplectl;

   /* gl_DrawID for a direct multidraw */
   uint32_t draw_id;

   /* Sprite coord replacement mask */
   uint16_t sprite_mask;

   /* glSampleMask */
   uint16_t sample_mask;

   /* Nonzero if the last vertex stage writes the layer ID, zero otherwise */
   uint16_t layer_id_written;

   /* Nonzero for indexed draws, zero otherwise */
   uint16_t is_indexed_draw;
};

struct PACKED agx_stage_uniforms {
   /* Pointer to binding table for texture descriptor, or 0 if none. This must
    * be first so that u0_u1 is always available for lowering binding
    * tables to bindless access.
    */
   uint64_t texture_base;

   /* Uniform buffer objects */
   uint64_t ubo_base[PIPE_MAX_CONSTANT_BUFFERS];
   uint32_t ubo_size[PIPE_MAX_CONSTANT_BUFFERS];

   /* Shader storage buffer objects */
   uint64_t ssbo_base[PIPE_MAX_SHADER_BUFFERS];
   uint32_t ssbo_size[PIPE_MAX_SHADER_BUFFERS];

   /* If lowered to bindless, sampler index in the heap */
   uint16_t sampler_handle[PIPE_MAX_SAMPLERS];

   /* LOD bias as float16 */
   uint16_t lod_bias[PIPE_MAX_SAMPLERS];
};

/* In the architecture, there are 512 uniform registers, each 16-bits. In a
 * theoretical worst case, we could push to all of them. We use a worst-case
 * maximum because the expression for a tight upper bound is too messy and easy
 * to go out of sync with the code.
 */
#define AGX_MAX_PUSH_RANGES (512)

struct agx_push_range {
   /* Base 16-bit uniform to push to */
   uint16_t uniform;

   /* Offset into the table to push in bytes */
   uint16_t offset;

   /* Which table to push from */
   uint8_t table;

   /* Number of consecutive 16-bit uniforms to push */
   uint8_t length;
};

struct agx_compiled_shader {
   /* Mapped executable memory */
   struct agx_bo *bo;

   /* Metadata returned from the compiler */
   struct agx_shader_info info;

   /* Uniforms the driver must push */
   unsigned push_range_count;
   struct agx_push_range push[AGX_MAX_PUSH_RANGES];

   /* Auxiliary programs, or NULL if not used */
   struct agx_compiled_shader *gs_count, *pre_gs;
   struct agx_uncompiled_shader *gs_copy;

   /* Output primitive mode for geometry shaders */
   enum mesa_prim gs_output_mode;

   /* Number of words per primitive in the count buffer */
   unsigned gs_count_words;
};

struct agx_uncompiled_shader {
   struct pipe_shader_state base;
   enum pipe_shader_type type;
   struct blob early_serialized_nir;
   struct blob serialized_nir;
   uint8_t nir_sha1[20];
   struct agx_uncompiled_shader_info info;
   struct hash_table *variants;
   struct agx_uncompiled_shader *passthrough_progs[MESA_PRIM_COUNT][3][2];

   uint32_t xfb_strides[4];
   bool has_xfb_info;

   /* Whether the shader accesses indexed samplers via the bindless heap */
   bool uses_bindless_samplers;

   /* Set on VS, passed to FS for linkage */
   unsigned base_varying;
};

enum agx_stage_dirty {
   AGX_STAGE_DIRTY_CONST = BITFIELD_BIT(0),
   AGX_STAGE_DIRTY_SSBO = BITFIELD_BIT(1),
   AGX_STAGE_DIRTY_IMAGE = BITFIELD_BIT(2),
   AGX_STAGE_DIRTY_SAMPLER = BITFIELD_BIT(3),
};

struct agx_stage {
   struct agx_uncompiled_shader *shader;
   uint32_t dirty;

   struct pipe_constant_buffer cb[PIPE_MAX_CONSTANT_BUFFERS];
   uint32_t cb_mask;

   struct pipe_shader_buffer ssbo[PIPE_MAX_SHADER_BUFFERS];
   uint32_t ssbo_mask;

   struct pipe_image_view images[PIPE_MAX_SHADER_IMAGES];
   uint32_t image_mask;

   /* Need full CSOs for u_blitter */
   struct agx_sampler_state *samplers[PIPE_MAX_SAMPLERS];
   struct agx_sampler_view *textures[PIPE_MAX_SHADER_SAMPLER_VIEWS];

   /* Does any bound sampler require custom border colours? */
   bool custom_borders;

   unsigned sampler_count, texture_count;
   uint32_t valid_samplers;
};

union agx_batch_result {
};

/* This is a firmware limit. It should be possible to raise to 2048 in the
 * future... still not good enough for VK though :-(
 */
#define AGX_SAMPLER_HEAP_SIZE (1024)

struct agx_sampler_heap {
   struct agx_bo *bo;
   uint16_t count;
};

uint16_t agx_sampler_heap_add(struct agx_device *dev,
                              struct agx_sampler_heap *heap,
                              struct agx_sampler_packed *sampler);

struct agx_encoder {
   struct agx_bo *bo;
   uint8_t *current;
   uint8_t *end;
};

struct agx_batch {
   struct agx_context *ctx;
   struct pipe_framebuffer_state key;
   uint64_t seqnum;
   uint32_t syncobj;
   uint32_t draws;

   struct agx_tilebuffer_layout tilebuffer_layout;

   /* PIPE_CLEAR_* bitmask */
   uint32_t clear, draw, load, resolve;
   bool any_draws;
   bool initialized;

   uint64_t uploaded_clear_color[PIPE_MAX_COLOR_BUFS];
   double clear_depth;
   unsigned clear_stencil;

   /* Whether we're drawing points, lines, or triangles */
   enum mesa_prim reduced_prim;

   /* Whether the bound FS needs a primitive ID that is not supplied by the
    * bound hardware VS (software GS)
    */
   bool generate_primitive_id;

   /* Current varyings linkage structures */
   uint32_t varyings;

   struct agx_draw_uniforms uniforms;

   /* Indirect buffer allocated for geometry shader */
   uint64_t geom_indirect;
   struct agx_bo *geom_indirect_bo;

   /* Geometry state buffer if geometry/etc shaders are used */
   uint64_t geometry_state;

   /* Uploaded descriptors */
   uint64_t textures[PIPE_SHADER_TYPES];
   uint32_t texture_count[PIPE_SHADER_TYPES];

   uint64_t samplers[PIPE_SHADER_TYPES];
   uint32_t sampler_count[PIPE_SHADER_TYPES];

   struct agx_sampler_heap sampler_heap;

   /* Resource list requirements, represented as a bit set indexed by BO
    * handles (GEM handles on Linux, or IOGPU's equivalent on macOS)
    */
   struct {
      BITSET_WORD *set;
      unsigned word_count;
   } bo_list;

   struct agx_pool pool, pipeline_pool;

   /* We may enqueue both CDM and VDM work, possibly to the same batch for
    * geometry/tessellation.
    */
   struct agx_encoder vdm;
   struct agx_encoder cdm;

   /* Scissor and depth-bias descriptors, uploaded at GPU time */
   struct util_dynarray scissor, depth_bias;

   /* Indexed occlusion queries within the occlusion buffer, and the occlusion
    * buffer itself which is allocated at submit time.
    */
   struct util_dynarray occlusion_queries;
   struct agx_ptr occlusion_buffer;

   /* Non-occlusion queries */
   struct util_dynarray nonocclusion_queries;
   struct util_dynarray timestamp_queries;

   /* Result buffer where the kernel places command execution information */
   union agx_batch_result *result;
   size_t result_off;

   /* Actual pointer in a uniform */
   struct agx_bo *geom_params_bo;
};

struct agx_zsa {
   struct pipe_depth_stencil_alpha_state base;
   struct agx_fragment_face_packed depth;
   struct agx_fragment_stencil_packed front_stencil, back_stencil;

   /* PIPE_CLEAR_* bitmask corresponding to this depth/stencil state */
   uint32_t load, store;
};

struct agx_blend_key {
   nir_lower_blend_rt rt[8];
   unsigned logicop_func;
   bool alpha_to_coverage, alpha_to_one;
};

struct agx_blend {
   struct agx_blend_key key;

   /* PIPE_CLEAR_* bitmask corresponding to this blend state */
   uint32_t store;
};

struct asahi_vs_shader_key {
   struct agx_attribute attribs[AGX_MAX_VBUFS];
   bool clip_halfz;
   bool fixed_point_size;
   uint64_t outputs_flat_shaded;
   uint64_t outputs_linear_shaded;
};

struct asahi_fs_shader_key {
   struct agx_blend_key blend;

   /* Set if glSampleMask() is used with a mask other than all-1s. If not, we
    * don't want to emit lowering code for it, since it would disable early-Z.
    */
   bool api_sample_mask;

   uint8_t cull_distance_size;
   uint8_t clip_plane_enable;
   uint8_t nr_samples;
   enum pipe_format rt_formats[PIPE_MAX_COLOR_BUFS];
};

struct asahi_gs_shader_key {
   /* Input assembly key */
   struct agx_ia_key ia;

   /* Vertex shader key */
   struct agx_attribute attribs[AGX_MAX_VBUFS];

   /* If true, this GS is run only for its side effects (including XFB) */
   bool rasterizer_discard;

   /* Geometry shaders must be linked with a vertex shader. In a monolithic
    * pipeline, this is the vertex shader (or tessellation evaluation shader).
    * With separate shaders, this needs to be an internal passthrough program.
    */
   uint8_t input_nir_sha1[20];
};

union asahi_shader_key {
   struct asahi_vs_shader_key vs;
   struct asahi_gs_shader_key gs;
   struct asahi_fs_shader_key fs;
};

enum agx_dirty {
   AGX_DIRTY_VERTEX = BITFIELD_BIT(0),
   AGX_DIRTY_VIEWPORT = BITFIELD_BIT(1),
   AGX_DIRTY_SCISSOR_ZBIAS = BITFIELD_BIT(2),
   AGX_DIRTY_ZS = BITFIELD_BIT(3),
   AGX_DIRTY_STENCIL_REF = BITFIELD_BIT(4),
   AGX_DIRTY_RS = BITFIELD_BIT(5),
   AGX_DIRTY_SPRITE_COORD_MODE = BITFIELD_BIT(6),
   AGX_DIRTY_PRIM = BITFIELD_BIT(7),

   /* Vertex/fragment pipelines, including uniforms and textures */
   AGX_DIRTY_VS = BITFIELD_BIT(8),
   AGX_DIRTY_FS = BITFIELD_BIT(9),

   /* Just the progs themselves */
   AGX_DIRTY_VS_PROG = BITFIELD_BIT(10),
   AGX_DIRTY_FS_PROG = BITFIELD_BIT(11),

   AGX_DIRTY_BLEND = BITFIELD_BIT(12),
   AGX_DIRTY_QUERY = BITFIELD_BIT(13),
   AGX_DIRTY_XFB = BITFIELD_BIT(14),
   AGX_DIRTY_SAMPLE_MASK = BITFIELD_BIT(15),
   AGX_DIRTY_BLEND_COLOR = BITFIELD_BIT(16),
};

/* Maximum number of in-progress + under-construction GPU batches.
 * Must be large enough for silly workloads that do things like
 * glGenerateMipmap on every frame, otherwise we end up losing performance.
 */
#define AGX_MAX_BATCHES (128)

static_assert(PIPE_TEX_FILTER_NEAREST < 2, "known order");
static_assert(PIPE_TEX_FILTER_LINEAR < 2, "known order");

enum asahi_blit_clamp {
   ASAHI_BLIT_CLAMP_NONE,
   ASAHI_BLIT_CLAMP_UINT_TO_SINT,
   ASAHI_BLIT_CLAMP_SINT_TO_UINT,

   /* keep last */
   ASAHI_BLIT_CLAMP_COUNT,
};

struct asahi_blitter {
   bool active;

   /* [clamp_type][is_array] */
   void *blit_cs[ASAHI_BLIT_CLAMP_COUNT][2];

   /* [filter] */
   void *sampler[2];

   struct pipe_constant_buffer saved_cb;

   bool has_saved_image;
   struct pipe_image_view saved_image;

   unsigned saved_num_sampler_states;
   void *saved_sampler_states[PIPE_MAX_SAMPLERS];

   struct pipe_sampler_view *saved_sampler_view;

   void *saved_cs;
};

struct agx_context {
   struct pipe_context base;
   struct agx_compiled_shader *vs, *fs, *gs;
   uint32_t dirty;

   /* Heap for dynamic memory allocation for geometry/tessellation shaders */
   struct pipe_resource *heap;

   /* Acts as a context-level shader key */
   bool support_lod_bias;

   /* Set of batches. When full, the LRU entry (the batch with the smallest
    * seqnum) is flushed to free a slot.
    */
   struct {
      uint64_t seqnum;
      struct agx_batch slots[AGX_MAX_BATCHES];

      /** Set of active batches for faster traversal */
      BITSET_DECLARE(active, AGX_MAX_BATCHES);

      /** Set of submitted batches for faster traversal */
      BITSET_DECLARE(submitted, AGX_MAX_BATCHES);
   } batches;

   struct agx_batch *batch;
   struct agx_bo *result_buf;

   struct pipe_vertex_buffer vertex_buffers[PIPE_MAX_ATTRIBS];
   uint32_t vb_mask;

   struct agx_stage stage[PIPE_SHADER_TYPES];
   struct agx_attribute *attributes;
   struct agx_rasterizer *rast;
   struct agx_zsa *zs;
   struct agx_blend *blend;
   struct pipe_blend_color blend_color;
   struct pipe_viewport_state viewport[AGX_MAX_VIEWPORTS];
   struct pipe_scissor_state scissor[AGX_MAX_VIEWPORTS];
   struct pipe_stencil_ref stencil_ref;
   struct agx_streamout streamout;
   uint16_t sample_mask;
   struct pipe_framebuffer_state framebuffer;

   struct pipe_query *cond_query;
   bool cond_cond;
   enum pipe_render_cond_flag cond_mode;

   struct agx_query *occlusion_query;
   struct agx_query *prims_generated[4];
   struct agx_query *tf_prims_generated[4];
   struct agx_query *tf_overflow[4];
   struct agx_query *tf_any_overflow;
   struct agx_query *time_elapsed;
   bool active_queries;

   struct util_debug_callback debug;
   bool is_noop;

   struct blitter_context *blitter;
   struct asahi_blitter compute_blitter;

   /* Map of GEM handle to (batch index + 1) that (conservatively) writes that
    * BO, or 0 if no writer.
    */
   struct util_dynarray writer;

   /* Bound CL global buffers */
   struct util_dynarray global_buffers;

   struct agx_compiled_shader *gs_prefix_sums[16];
   struct agx_compiled_shader *gs_setup_indirect[MESA_PRIM_MAX][2];
   struct agx_compiled_shader *gs_unroll_restart[MESA_PRIM_MAX][3];
   struct agx_meta_cache meta;

   uint32_t syncobj;
   uint32_t dummy_syncobj;
   int in_sync_fd;
   uint32_t in_sync_obj;
};

static void
agx_writer_add(struct agx_context *ctx, uint8_t batch_index, unsigned handle)
{
   assert(batch_index < AGX_MAX_BATCHES && "invariant");
   static_assert(AGX_MAX_BATCHES < 0xFF, "no overflow on addition");

   /* If we need to grow, double the capacity so insertion is amortized O(1). */
   if (unlikely(handle >= ctx->writer.size)) {
      unsigned new_size =
         MAX2(ctx->writer.capacity * 2, util_next_power_of_two(handle + 1));
      unsigned grow = new_size - ctx->writer.size;

      memset(util_dynarray_grow(&ctx->writer, uint8_t, grow), 0,
             grow * sizeof(uint8_t));
   }

   /* There is now room */
   uint8_t *value = util_dynarray_element(&ctx->writer, uint8_t, handle);
   assert((*value) == 0 && "there should be no existing writer");
   *value = batch_index + 1;
}

static struct agx_batch *
agx_writer_get(struct agx_context *ctx, unsigned handle)
{
   if (handle >= ctx->writer.size)
      return NULL;

   uint8_t value = *util_dynarray_element(&ctx->writer, uint8_t, handle);

   if (value > 0)
      return &ctx->batches.slots[value - 1];
   else
      return NULL;
}

static void
agx_writer_remove(struct agx_context *ctx, unsigned handle)
{
   if (handle >= ctx->writer.size)
      return;

   uint8_t *value = util_dynarray_element(&ctx->writer, uint8_t, handle);
   *value = 0;
}

static inline struct agx_context *
agx_context(struct pipe_context *pctx)
{
   return (struct agx_context *)pctx;
}

void agx_launch(struct agx_batch *batch, const struct pipe_grid_info *info,
                struct agx_compiled_shader *cs, enum pipe_shader_type stage);

void agx_init_query_functions(struct pipe_context *ctx);

void
agx_primitives_update_direct(struct agx_context *ctx,
                             const struct pipe_draw_info *info,
                             const struct pipe_draw_start_count_bias *draw);

void agx_draw_vbo_from_xfb(struct pipe_context *pctx,
                           const struct pipe_draw_info *info,
                           unsigned drawid_offset,
                           const struct pipe_draw_indirect_info *indirect);

uint64_t agx_batch_get_so_address(struct agx_batch *batch, unsigned buffer,
                                  uint32_t *size);

void agx_init_streamout_functions(struct pipe_context *ctx);

static inline void
agx_dirty_all(struct agx_context *ctx)
{
   ctx->dirty = ~0;

   for (unsigned i = 0; i < ARRAY_SIZE(ctx->stage); ++i)
      ctx->stage[i].dirty = ~0;
}

static inline void
agx_dirty_reset_graphics(struct agx_context *ctx)
{
   ctx->dirty = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(ctx->stage); ++i) {
      if (i != PIPE_SHADER_COMPUTE)
         ctx->stage[i].dirty = 0;
   }
}

struct agx_rasterizer {
   struct pipe_rasterizer_state base;
   uint8_t cull[AGX_CULL_LENGTH];
   uint8_t line_width;
   uint8_t polygon_mode;
};

struct agx_query {
   unsigned type;
   unsigned index;

   /* Invariant for occlusion queries:
    *
    *    writer != NULL => writer->occlusion_queries[writer_index] == this, and
    *    writer == NULL => no batch such that this in batch->occlusion_queries
    */
   struct agx_batch *writer;
   unsigned writer_index;

   /* For GPU queries other than occlusion queries, the value of the query as
    * written by the `writer` if a writer is non-NULL, and irrelevant otherwise.
    * When flushing the query, this value is read and added to agx_query::value.
    */
   struct agx_ptr ptr;

   /* Accumulator flushed to the CPU */
   union {
      uint64_t value;
      uint64_t timestamp_end;
   };

   /* For time elapsed queries, end is in the above union for consistent
    * handling witn timestamp queries.
    */
   uint64_t timestamp_begin;
};

struct agx_sampler_state {
   struct pipe_sampler_state base;

   /* Prepared descriptor */
   struct agx_sampler_packed desc, desc_without_custom_border;

   /* Whether a custom border colour is required */
   bool uses_custom_border;

   /* Packed custom border colour, or zero if none is required */
   struct agx_border_packed border;

   /* LOD bias packed as fp16, the form we'll pass to the shader */
   uint16_t lod_bias_as_fp16;
};

struct agx_sampler_view {
   struct pipe_sampler_view base;

   /* Resource/format, may differ from base in case of separate stencil */
   struct agx_resource *rsrc;
   enum pipe_format format;

   /* Prepared descriptor */
   struct agx_texture_packed desc;
};

struct agx_screen {
   struct pipe_screen pscreen;
   struct agx_device dev;
   struct disk_cache *disk_cache;
   /* Queue handle */
   uint32_t queue_id;
};

static inline struct agx_screen *
agx_screen(struct pipe_screen *p)
{
   return (struct agx_screen *)p;
}

static inline struct agx_device *
agx_device(struct pipe_screen *p)
{
   return &(agx_screen(p)->dev);
}

#define perf_debug(dev, ...)                                                   \
   do {                                                                        \
      if (unlikely((dev)->debug & AGX_DBG_PERF))                               \
         mesa_logw(__VA_ARGS__);                                               \
   } while (0)

#define perf_debug_ctx(ctx, ...)                                               \
   perf_debug(agx_device((ctx)->base.screen), __VA_ARGS__)

struct agx_resource {
   struct pipe_resource base;
   uint64_t modifier;

   /* Should probably be part of the modifier. Affects the tiling algorithm, or
    * something like that.
    */
   bool mipmapped;

   /* Hardware backing */
   struct agx_bo *bo;

   struct renderonly_scanout *scanout;

   BITSET_DECLARE(data_valid, PIPE_MAX_TEXTURE_LEVELS);

   struct ail_layout layout;

   /* Metal does not support packed depth/stencil formats; presumably AGX does
    * not either. Instead, we create separate depth and stencil resources,
    * managed by u_transfer_helper.  We provide the illusion of packed
    * resources.
    */
   struct agx_resource *separate_stencil;

   /* Valid buffer range tracking, to optimize buffer appends */
   struct util_range valid_buffer_range;

   /* Cumulative shadowed byte count for this resource, that is, the number of
    * times multiplied by the resource size.
    */
   size_t shadowed_bytes;
};

static inline struct agx_resource *
agx_resource(struct pipe_resource *pctx)
{
   return (struct agx_resource *)pctx;
}

static inline bool
agx_resource_valid(struct agx_resource *rsrc, int level)
{
   /* Shared BOs can always be potentially valid */
   if (rsrc->bo && rsrc->bo->flags & AGX_BO_SHARED) {
      assert(level == 0);
      return true;
   }

   return BITSET_TEST(rsrc->data_valid, level);
}

static inline void *
agx_map_texture_cpu(struct agx_resource *rsrc, unsigned level, unsigned z)
{
   return ((uint8_t *)rsrc->bo->ptr.cpu) +
          ail_get_layer_level_B(&rsrc->layout, z, level);
}

static inline uint64_t
agx_map_texture_gpu(struct agx_resource *rsrc, unsigned z)
{
   return rsrc->bo->ptr.gpu +
          (uint64_t)ail_get_layer_offset_B(&rsrc->layout, z);
}

void agx_decompress(struct agx_context *ctx, struct agx_resource *rsrc,
                    const char *reason);

void agx_legalize_compression(struct agx_context *ctx,
                              struct agx_resource *rsrc,
                              enum pipe_format format);

struct agx_transfer {
   struct pipe_transfer base;
   void *map;
   struct {
      struct pipe_resource *rsrc;
      struct pipe_box box;
   } staging;
};

static inline struct agx_transfer *
agx_transfer(struct pipe_transfer *p)
{
   return (struct agx_transfer *)p;
}

void agx_upload_vbos(struct agx_batch *batch);
void agx_upload_uniforms(struct agx_batch *batch);

uint64_t agx_upload_stage_uniforms(struct agx_batch *batch, uint64_t textures,
                                   enum pipe_shader_type stage);

void agx_nir_lower_point_size(nir_shader *nir, bool fixed_point_size);

bool agx_nir_lower_sysvals(nir_shader *shader, bool lower_draw_params);

bool agx_nir_layout_uniforms(nir_shader *shader,
                             struct agx_compiled_shader *compiled,
                             unsigned *push_size);

bool agx_nir_lower_bindings(nir_shader *shader, bool *uses_bindless_samplers);

bool agx_batch_is_active(struct agx_batch *batch);
bool agx_batch_is_submitted(struct agx_batch *batch);

/* Add a BO to a batch. This needs to be amortized O(1) since it's called in
 * hot paths. To achieve this we model BO lists by bit sets */

static unsigned
agx_batch_bo_list_bits(struct agx_batch *batch)
{
   return batch->bo_list.word_count * sizeof(BITSET_WORD) * 8;
}

static bool
agx_batch_uses_bo(struct agx_batch *batch, struct agx_bo *bo)
{
   if (bo->handle < agx_batch_bo_list_bits(batch))
      return BITSET_TEST(batch->bo_list.set, bo->handle);
   else
      return false;
}

static inline void
agx_batch_add_bo(struct agx_batch *batch, struct agx_bo *bo)
{
   /* Double the size of the BO list if we run out, this is amortized O(1) */
   if (unlikely(bo->handle > agx_batch_bo_list_bits(batch))) {
      unsigned word_count =
         MAX2(batch->bo_list.word_count * 2,
              util_next_power_of_two(BITSET_WORDS(bo->handle + 1)));

      batch->bo_list.set =
         rerzalloc(batch->ctx, batch->bo_list.set, BITSET_WORD,
                   batch->bo_list.word_count, word_count);
      batch->bo_list.word_count = word_count;
   }

   /* The batch holds a single reference to each BO in the batch, released when
    * the batch finishes execution.
    */
   if (!BITSET_TEST(batch->bo_list.set, bo->handle))
      agx_bo_reference(bo);

   BITSET_SET(batch->bo_list.set, bo->handle);
}

static unsigned
agx_batch_num_bo(struct agx_batch *batch)
{
   return __bitset_count(batch->bo_list.set, batch->bo_list.word_count);
}

#define AGX_BATCH_FOREACH_BO_HANDLE(batch, handle)                             \
   BITSET_FOREACH_SET(handle, (batch)->bo_list.set,                            \
                      agx_batch_bo_list_bits(batch))

void agx_batch_submit(struct agx_context *ctx, struct agx_batch *batch,
                      uint32_t barriers, enum drm_asahi_cmd_type cmd_type,
                      void *cmdbuf);

void agx_flush_batch(struct agx_context *ctx, struct agx_batch *batch);
void agx_flush_batch_for_reason(struct agx_context *ctx,
                                struct agx_batch *batch, const char *reason);
void agx_flush_all(struct agx_context *ctx, const char *reason);
void agx_flush_readers(struct agx_context *ctx, struct agx_resource *rsrc,
                       const char *reason);
void agx_flush_writer(struct agx_context *ctx, struct agx_resource *rsrc,
                      const char *reason);

void agx_sync_writer(struct agx_context *ctx, struct agx_resource *rsrc,
                     const char *reason);
void agx_sync_readers(struct agx_context *ctx, struct agx_resource *rsrc,
                      const char *reason);
void agx_sync_batch(struct agx_context *ctx, struct agx_batch *batch);
void agx_sync_all(struct agx_context *ctx, const char *reason);
void agx_sync_batch_for_reason(struct agx_context *ctx, struct agx_batch *batch,
                               const char *reason);

/* Use these instead of batch_add_bo for proper resource tracking */
void agx_batch_reads(struct agx_batch *batch, struct agx_resource *rsrc);
void agx_batch_writes(struct agx_batch *batch, struct agx_resource *rsrc,
                      unsigned level);
void agx_batch_track_image(struct agx_batch *batch,
                           struct pipe_image_view *image);

bool agx_any_batch_uses_resource(struct agx_context *ctx,
                                 struct agx_resource *rsrc);

/* 16384 is the maximum framebuffer dimension, so we use a larger width (the
 * maximum uint16_t) as a sentinel to identify the compute batch. This ensures
 * compute batches don't mix with graphics. This is a bit of a hack but it
 * works.
 */
#define AGX_COMPUTE_BATCH_WIDTH 0xFFFF

static inline bool
agx_batch_is_compute(struct agx_batch *batch)
{
   return batch->key.width == AGX_COMPUTE_BATCH_WIDTH;
}

struct agx_batch *agx_get_batch(struct agx_context *ctx);
struct agx_batch *agx_get_compute_batch(struct agx_context *ctx);
void agx_batch_reset(struct agx_context *ctx, struct agx_batch *batch);
int agx_cleanup_batches(struct agx_context *ctx);

void agx_batch_add_timestamp_query(struct agx_batch *batch,
                                   struct agx_query *q);
void agx_add_timestamp_end_query(struct agx_context *ctx, struct agx_query *q);

/* Blit shaders */
void agx_blitter_save(struct agx_context *ctx, struct blitter_context *blitter,
                      bool render_cond);

void agx_blit(struct pipe_context *pipe, const struct pipe_blit_info *info);

void agx_resource_copy_region(struct pipe_context *pctx,
                              struct pipe_resource *dst, unsigned dst_level,
                              unsigned dstx, unsigned dsty, unsigned dstz,
                              struct pipe_resource *src, unsigned src_level,
                              const struct pipe_box *src_box);

/* Batch logic */

struct agx_encoder agx_encoder_allocate(struct agx_batch *batch,
                                        struct agx_device *dev);

void agx_batch_init_state(struct agx_batch *batch);

uint64_t agx_build_meta(struct agx_batch *batch, bool store,
                        bool partial_render);

/* Query management */
uint16_t agx_get_oq_index(struct agx_batch *batch, struct agx_query *query);
uint64_t agx_get_query_address(struct agx_batch *batch,
                               struct agx_query *query);

void agx_finish_batch_queries(struct agx_batch *batch, uint64_t begin_ts,
                              uint64_t end_ts);

bool agx_render_condition_check_inner(struct agx_context *ctx);

static inline bool
agx_render_condition_check(struct agx_context *ctx)
{
   if (likely(!ctx->cond_query))
      return true;
   else
      return agx_render_condition_check_inner(ctx);
}

/* Texel buffers lowered to (at most) 1024x16384 2D textures */
#define AGX_TEXTURE_BUFFER_WIDTH      1024
#define AGX_TEXTURE_BUFFER_MAX_HEIGHT 16384
#define AGX_TEXTURE_BUFFER_MAX_SIZE                                            \
   (AGX_TEXTURE_BUFFER_WIDTH * AGX_TEXTURE_BUFFER_MAX_HEIGHT)

static inline uint32_t
agx_texture_buffer_size_el(enum pipe_format format, uint32_t size)
{
   unsigned blocksize = util_format_get_blocksize(format);

   return MIN2(AGX_TEXTURE_BUFFER_MAX_SIZE, size / blocksize);
}

#endif
