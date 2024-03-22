/*
 * © Copyright 2018 Alyssa Rosenzweig
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef __BUILDER_H__
#define __BUILDER_H__

#define _LARGEFILE64_SOURCE 1
#include <assert.h>
#include <sys/mman.h>
#include "pan_afbc_cso.h"
#include "pan_blend_cso.h"
#include "pan_earlyzs.h"
#include "pan_encoder.h"
#include "pan_job.h"
#include "pan_resource.h"
#include "pan_texture.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#include "util/compiler.h"
#include "util/detect.h"
#include "util/format/u_formats.h"
#include "util/hash_table.h"
#include "util/simple_mtx.h"
#include "util/u_blitter.h"

#include "compiler/shader_enums.h"
#include "midgard/midgard_compile.h"

#define SET_BIT(lval, bit, cond)                                               \
   if (cond)                                                                   \
      lval |= (bit);                                                           \
   else                                                                        \
      lval &= ~(bit);

/* Dirty tracking flags. 3D is for general 3D state. Shader flags are
 * per-stage. Renderer refers to Renderer State Descriptors. Vertex refers to
 * vertex attributes/elements. */

enum pan_dirty_3d {
   PAN_DIRTY_VIEWPORT = BITFIELD_BIT(0),
   PAN_DIRTY_SCISSOR = BITFIELD_BIT(1),
   PAN_DIRTY_VERTEX = BITFIELD_BIT(2),
   PAN_DIRTY_PARAMS = BITFIELD_BIT(3),
   PAN_DIRTY_DRAWID = BITFIELD_BIT(4),
   PAN_DIRTY_TLS_SIZE = BITFIELD_BIT(5),
   PAN_DIRTY_ZS = BITFIELD_BIT(6),
   PAN_DIRTY_BLEND = BITFIELD_BIT(7),
   PAN_DIRTY_MSAA = BITFIELD_BIT(8),
   PAN_DIRTY_OQ = BITFIELD_BIT(9),
   PAN_DIRTY_RASTERIZER = BITFIELD_BIT(10),
   PAN_DIRTY_POINTS = BITFIELD_BIT(11),
   PAN_DIRTY_SO = BITFIELD_BIT(12),
};

enum pan_dirty_shader {
   PAN_DIRTY_STAGE_SHADER = BITFIELD_BIT(0),
   PAN_DIRTY_STAGE_TEXTURE = BITFIELD_BIT(1),
   PAN_DIRTY_STAGE_SAMPLER = BITFIELD_BIT(2),
   PAN_DIRTY_STAGE_IMAGE = BITFIELD_BIT(3),
   PAN_DIRTY_STAGE_CONST = BITFIELD_BIT(4),
   PAN_DIRTY_STAGE_SSBO = BITFIELD_BIT(5),
};

struct panfrost_constant_buffer {
   struct pipe_constant_buffer cb[PIPE_MAX_CONSTANT_BUFFERS];
   uint32_t enabled_mask;
};

struct panfrost_query {
   /* Passthrough from Gallium */
   unsigned type;
   unsigned index;

   /* For computed queries. 64-bit to prevent overflow */
   struct {
      uint64_t start;
      uint64_t end;
   };

   /* Memory for the GPU to writeback the value of the query */
   struct pipe_resource *rsrc;

   /* Whether an occlusion query is for a MSAA framebuffer */
   bool msaa;
};

struct panfrost_streamout_target {
   struct pipe_stream_output_target base;
   uint32_t offset;
};

struct panfrost_streamout {
   struct pipe_stream_output_target *targets[PIPE_MAX_SO_BUFFERS];
   unsigned num_targets;
};

struct panfrost_context {
   /* Gallium context */
   struct pipe_context base;

   /* Dirty global state */
   enum pan_dirty_3d dirty;

   /* Per shader stage dirty state */
   enum pan_dirty_shader dirty_shader[PIPE_SHADER_TYPES];

   /* Unowned pools, so manage yourself. */
   struct panfrost_pool descs, shaders;

   /* Sync obj used to keep track of in-flight jobs. */
   uint32_t syncobj;

   /* Set of 32 batches. When the set is full, the LRU entry (the batch
    * with the smallest seqnum) is flushed to free a slot.
    */
   struct {
      uint64_t seqnum;
      struct panfrost_batch slots[PAN_MAX_BATCHES];

      /** Set of active batches for faster traversal */
      BITSET_DECLARE(active, PAN_MAX_BATCHES);
   } batches;

   /* Map from resources to panfrost_batches */
   struct hash_table *writers;

   /* Bound job batch */
   struct panfrost_batch *batch;

   /* Within a launch_grid call.. */
   const struct pipe_grid_info *compute_grid;

   struct pipe_framebuffer_state pipe_framebuffer;
   struct panfrost_streamout streamout;

   bool active_queries;
   uint64_t prims_generated;
   uint64_t tf_prims_generated;
   uint64_t draw_calls;
   struct panfrost_query *occlusion_query;

   unsigned drawid;
   unsigned vertex_count;
   unsigned instance_count;
   unsigned offset_start;
   unsigned base_vertex;
   unsigned base_instance;
   enum mesa_prim active_prim;

   /* If instancing is enabled, vertex count padded for instance; if
    * it is disabled, just equal to plain vertex count */
   unsigned padded_count;

   struct panfrost_constant_buffer constant_buffer[PIPE_SHADER_TYPES];
   struct panfrost_rasterizer *rasterizer;
   struct panfrost_vertex_state *vertex;

   struct panfrost_uncompiled_shader *uncompiled[PIPE_SHADER_TYPES];
   struct panfrost_compiled_shader *prog[PIPE_SHADER_TYPES];

   struct pipe_vertex_buffer vertex_buffers[PIPE_MAX_ATTRIBS];
   uint32_t vb_mask;

   struct pipe_shader_buffer ssbo[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_BUFFERS];
   uint32_t ssbo_mask[PIPE_SHADER_TYPES];

   struct pipe_image_view images[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_IMAGES];
   uint32_t image_mask[PIPE_SHADER_TYPES];

   struct panfrost_sampler_state *samplers[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   unsigned sampler_count[PIPE_SHADER_TYPES];
   uint32_t valid_samplers[PIPE_SHADER_TYPES];

   struct panfrost_sampler_view
      *sampler_views[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];
   unsigned sampler_view_count[PIPE_SHADER_TYPES];

   struct blitter_context *blitter;

   struct pan_afbc_shaders afbc_shaders;

   struct panfrost_blend_state *blend;

   /* On Valhall, does the current blend state use a blend shader for any
    * output? We need this information in a hot path to decide if
    * per-sample shading should be enabled.
    */
   bool valhall_has_blend_shader;

   struct pipe_viewport_state pipe_viewport;
   struct pipe_scissor_state scissor;
   struct pipe_blend_color blend_color;
   struct panfrost_zsa_state *depth_stencil;
   struct pipe_stencil_ref stencil_ref;
   uint16_t sample_mask;
   unsigned min_samples;

   struct panfrost_query *cond_query;
   bool cond_cond;
   enum pipe_render_cond_flag cond_mode;

   bool is_noop;

   /* Mask of active render targets */
   uint8_t fb_rt_mask;

   int in_sync_fd;
   uint32_t in_sync_obj;
};

/* Corresponds to the CSO */

struct panfrost_rasterizer;

/* Linked varyings */
struct pan_linkage {
   /* If the upload is owned by the CSO instead
    * of the pool, the referenced BO. Else,
    * NULL. */
   struct panfrost_bo *bo;

   /* Uploaded attribute descriptors */
   mali_ptr producer, consumer;

   /* Varyings buffers required */
   uint32_t present;

   /* Per-vertex stride for general varying buffer */
   uint32_t stride;
};

/* System value infrastructure */
#define MAX_SYSVAL_COUNT 32

/* Allow 2D of sysval IDs, while allowing nonparametric sysvals to equal
 * their class for equal comparison */

#define PAN_SYSVAL(type, no)    (((no) << 16) | PAN_SYSVAL_##type)
#define PAN_SYSVAL_TYPE(sysval) ((sysval)&0xffff)
#define PAN_SYSVAL_ID(sysval)   ((sysval) >> 16)

/* Define some common types. We start at one for easy indexing of hash
 * tables internal to the compiler */

enum {
   PAN_SYSVAL_VIEWPORT_SCALE = 1,
   PAN_SYSVAL_VIEWPORT_OFFSET = 2,
   PAN_SYSVAL_TEXTURE_SIZE = 3,
   PAN_SYSVAL_SSBO = 4,
   PAN_SYSVAL_NUM_WORK_GROUPS = 5,
   PAN_SYSVAL_SAMPLER = 7,
   PAN_SYSVAL_LOCAL_GROUP_SIZE = 8,
   PAN_SYSVAL_WORK_DIM = 9,
   PAN_SYSVAL_IMAGE_SIZE = 10,
   PAN_SYSVAL_SAMPLE_POSITIONS = 11,
   PAN_SYSVAL_MULTISAMPLED = 12,
   PAN_SYSVAL_RT_CONVERSION = 13,
   PAN_SYSVAL_VERTEX_INSTANCE_OFFSETS = 14,
   PAN_SYSVAL_DRAWID = 15,
   PAN_SYSVAL_BLEND_CONSTANTS = 16,
   PAN_SYSVAL_XFB = 17,
   PAN_SYSVAL_NUM_VERTICES = 18,
};

#define PAN_TXS_SYSVAL_ID(texidx, dim, is_array)                               \
   ((texidx) | ((dim) << 7) | ((is_array) ? (1 << 9) : 0))

#define PAN_SYSVAL_ID_TO_TXS_TEX_IDX(id)  ((id)&0x7f)
#define PAN_SYSVAL_ID_TO_TXS_DIM(id)      (((id) >> 7) & 0x3)
#define PAN_SYSVAL_ID_TO_TXS_IS_ARRAY(id) !!((id) & (1 << 9))

struct panfrost_sysvals {
   /* The mapping of sysvals to uniforms, the count, and the off-by-one inverse */
   unsigned sysvals[MAX_SYSVAL_COUNT];
   unsigned sysval_count;
};

#define RSD_WORDS 16

/* Variants bundle together to form the backing CSO, bundling multiple
 * shaders with varying emulated features baked in
 */
struct panfrost_fs_key {
   /* Number of colour buffers if gl_FragColor is written */
   unsigned nr_cbufs_for_fragcolor;

   /* On Valhall, fixed_varying_mask of the linked vertex shader */
   uint32_t fixed_varying_mask;

   /* Midgard shaders that read the tilebuffer must be keyed for
    * non-blendable formats
    */
   enum pipe_format rt_formats[8];

   /* From rasterize state, to lower point sprites */
   uint16_t sprite_coord_enable;

   /* User clip plane lowering */
   uint8_t clip_plane_enable;
};

struct panfrost_shader_key {
   union {
      /* Vertex shaders do not use shader keys. However, we have a
       * special "transform feedback" vertex program derived from a
       * vertex shader. If vs_is_xfb is set on a vertex shader, this
       * is a transform feedback shader, else it is a regular
       * (unkeyed) vertex shader.
       */
      bool vs_is_xfb;

      /* Fragment shaders use regular shader keys */
      struct panfrost_fs_key fs;
   };
};

struct panfrost_compiled_shader {
   /* Respectively, shader binary and Renderer State Descriptor */
   struct panfrost_pool_ref bin, state;

   /* For fragment shaders, a prepared (but not uploaded RSD) */
   uint32_t partial_rsd[RSD_WORDS];

   struct pan_shader_info info;
   struct panfrost_sysvals sysvals;

   struct pan_earlyzs_lut earlyzs;

   /* Linked varyings, for non-separable programs */
   struct pan_linkage linkage;

   struct pipe_stream_output_info stream_output;

   struct panfrost_shader_key key;

   /* Mask of state that dirties the sysvals */
   unsigned dirty_3d, dirty_shader;
};

/* Shader CSO */
struct panfrost_uncompiled_shader {
   /* NIR for the shader. For graphics, this will be non-NULL even for
    * TGSI. For compute, this will be NULL after the shader is compiled,
    * as we don't need any compute variants.
    */
   const nir_shader *nir;

   /* A SHA1 of the serialized NIR for the disk cache. */
   unsigned char nir_sha1[20];

   /* Stream output information */
   struct pipe_stream_output_info stream_output;

   /** Lock for the variants array */
   simple_mtx_t lock;

   /* Array of panfrost_compiled_shader */
   struct util_dynarray variants;

   /* Compiled transform feedback program, if one is required */
   struct panfrost_compiled_shader *xfb;

   /* On vertex shaders, bit mask of special desktop-only varyings to link
    * with the fragment shader. Used on Valhall to implement separable
    * shaders for desktop GL.
    */
   uint32_t fixed_varying_mask;

   /* If gl_FragColor was lowered, we need to optimize the stores later */
   bool fragcolor_lowered;
};

/* The binary artefacts of compiling a shader. This differs from
 * panfrost_compiled_shader, which adds extra metadata beyond compiling but
 * throws away information not needed after the initial compile.
 *
 * This structure is serialized for the shader disk cache.
 */
struct panfrost_shader_binary {
   /* Collected information about the compiled shader */
   struct pan_shader_info info;
   struct panfrost_sysvals sysvals;

   /* The binary itself */
   struct util_dynarray binary;
};

void
panfrost_disk_cache_store(struct disk_cache *cache,
                          const struct panfrost_uncompiled_shader *uncompiled,
                          const struct panfrost_shader_key *key,
                          const struct panfrost_shader_binary *binary);

bool panfrost_disk_cache_retrieve(
   struct disk_cache *cache,
   const struct panfrost_uncompiled_shader *uncompiled,
   const struct panfrost_shader_key *key,
   struct panfrost_shader_binary *binary);

void panfrost_disk_cache_init(struct panfrost_screen *screen);

bool panfrost_nir_remove_fragcolor_stores(nir_shader *s, unsigned nr_cbufs);

bool panfrost_nir_lower_sysvals(nir_shader *s,
                                struct panfrost_sysvals *sysvals);

/** (Vertex buffer index, divisor) tuple that will become an Attribute Buffer
 * Descriptor at draw-time on Midgard
 */
struct pan_vertex_buffer {
   unsigned vbi;
   unsigned divisor;
};

unsigned pan_assign_vertex_buffer(struct pan_vertex_buffer *buffers,
                                  unsigned *nr_bufs, unsigned vbi,
                                  unsigned divisor);

struct panfrost_zsa_state;
struct panfrost_sampler_state;
struct panfrost_sampler_view;
struct panfrost_vertex_state;

static inline struct panfrost_context *
pan_context(struct pipe_context *pcontext)
{
   return (struct panfrost_context *)pcontext;
}

static inline struct panfrost_streamout_target *
pan_so_target(struct pipe_stream_output_target *target)
{
   return (struct panfrost_streamout_target *)target;
}

struct pipe_context *panfrost_create_context(struct pipe_screen *screen,
                                             void *priv, unsigned flags);

bool panfrost_writes_point_size(struct panfrost_context *ctx);

struct panfrost_ptr panfrost_vertex_tiler_job(struct panfrost_context *ctx,
                                              bool is_tiler);

void panfrost_flush(struct pipe_context *pipe, struct pipe_fence_handle **fence,
                    unsigned flags);

bool panfrost_render_condition_check(struct panfrost_context *ctx);

void panfrost_update_shader_variant(struct panfrost_context *ctx,
                                    enum pipe_shader_type type);

void panfrost_analyze_sysvals(struct panfrost_compiled_shader *ss);

mali_ptr
panfrost_get_index_buffer(struct panfrost_batch *batch,
                          const struct pipe_draw_info *info,
                          const struct pipe_draw_start_count_bias *draw);

mali_ptr
panfrost_get_index_buffer_bounded(struct panfrost_batch *batch,
                                  const struct pipe_draw_info *info,
                                  const struct pipe_draw_start_count_bias *draw,
                                  unsigned *min_index, unsigned *max_index);

/* Instancing */

mali_ptr panfrost_vertex_buffer_address(struct panfrost_context *ctx,
                                        unsigned i);

void panfrost_shader_context_init(struct pipe_context *pctx);

static inline void
panfrost_dirty_state_all(struct panfrost_context *ctx)
{
   ctx->dirty = ~0;

   for (unsigned i = 0; i < PIPE_SHADER_TYPES; ++i)
      ctx->dirty_shader[i] = ~0;
}

static inline void
panfrost_clean_state_3d(struct panfrost_context *ctx)
{
   ctx->dirty = 0;

   for (unsigned i = 0; i < PIPE_SHADER_TYPES; ++i) {
      if (i != PIPE_SHADER_COMPUTE)
         ctx->dirty_shader[i] = 0;
   }
}

void panfrost_set_batch_masks_blend(struct panfrost_batch *batch);

void panfrost_set_batch_masks_zs(struct panfrost_batch *batch);

void panfrost_track_image_access(struct panfrost_batch *batch,
                                 enum pipe_shader_type stage,
                                 struct pipe_image_view *image);

#endif
