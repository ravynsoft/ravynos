/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FREEDRENO_CONTEXT_H_
#define FREEDRENO_CONTEXT_H_

#include "pipe/p_context.h"
#include "util/libsync.h"
#include "util/list.h"
#include "util/slab.h"
#include "util/u_blitter.h"
#include "util/u_string.h"
#include "util/u_threaded_context.h"
#include "util/perf/u_trace.h"

#include "freedreno_autotune.h"
#include "freedreno_gmem.h"
#include "freedreno_perfetto.h"
#include "freedreno_screen.h"
#include "freedreno_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BORDER_COLOR_UPLOAD_SIZE (2 * PIPE_MAX_SAMPLERS * BORDERCOLOR_SIZE)

struct fd_vertex_stateobj;
struct fd_batch;

struct fd_texture_stateobj {
   struct pipe_sampler_view *textures[PIPE_MAX_SAMPLERS];
   unsigned num_textures;
   unsigned valid_textures;
   struct pipe_sampler_state *samplers[PIPE_MAX_SAMPLERS];
   unsigned num_samplers;
   unsigned valid_samplers;
};

struct fd_program_stateobj {
   void *vs, *hs, *ds, *gs, *fs;
};

struct fd_constbuf_stateobj {
   struct pipe_constant_buffer cb[PIPE_MAX_CONSTANT_BUFFERS];
   uint32_t enabled_mask;
};

struct fd_shaderbuf_stateobj {
   struct pipe_shader_buffer sb[PIPE_MAX_SHADER_BUFFERS];
   uint32_t enabled_mask;
   uint32_t writable_mask;
};

struct fd_shaderimg_stateobj {
   struct pipe_image_view si[PIPE_MAX_SHADER_IMAGES];
   uint32_t enabled_mask;
};

struct fd_vertexbuf_stateobj {
   struct pipe_vertex_buffer vb[PIPE_MAX_ATTRIBS];
   unsigned count;
   uint32_t enabled_mask;
};

struct fd_vertex_stateobj {
   struct pipe_vertex_element pipe[PIPE_MAX_ATTRIBS];
   unsigned strides[PIPE_MAX_ATTRIBS];
   unsigned num_elements;
};

struct fd_stream_output_target {
   struct pipe_stream_output_target base;
   struct pipe_resource *offset_buf;
   /* stride of the last stream out recorded to this target, for
    * glDrawTransformFeedback(). */
   uint32_t stride;
};

struct fd_streamout_stateobj {
   struct pipe_stream_output_target *targets[PIPE_MAX_SO_BUFFERS];
   /* Bitmask of stream that should be reset. */
   unsigned reset;

   unsigned num_targets;
   /* Track offset from vtxcnt for streamout data.  This counter
    * is just incremented by # of vertices on each draw until
    * reset or new streamout buffer bound.
    *
    * When we eventually have GS, the CPU won't actually know the
    * number of vertices per draw, so I think we'll have to do
    * something more clever.
    */
   unsigned offsets[PIPE_MAX_SO_BUFFERS];

   /* Pre-a6xx, the maximum number of vertices that could be recorded to this
    * set of targets with the current vertex shader.  a6xx and newer, hardware
    * queries are used.
    */
   unsigned max_tf_vtx;

   /* Pre-a6xx, the number of verts written to the buffers since the last
    * Begin.  Used for overflow checking for SW queries.
    */
   unsigned verts_written;
};

#define MAX_GLOBAL_BUFFERS 16
struct fd_global_bindings_stateobj {
   struct pipe_resource *buf[MAX_GLOBAL_BUFFERS];
   uint32_t enabled_mask;
};

/* group together the vertex and vertexbuf state.. for ease of passing
 * around, and because various internal operations (gmem<->mem, etc)
 * need their own vertex state:
 */
struct fd_vertex_state {
   struct fd_vertex_stateobj *vtx;
   struct fd_vertexbuf_stateobj vertexbuf;
};

/* global 3d pipeline dirty state: */
enum fd_dirty_3d_state {
   FD_DIRTY_BLEND = BIT(0),
   FD_DIRTY_RASTERIZER = BIT(1),
   FD_DIRTY_ZSA = BIT(2),
   FD_DIRTY_BLEND_COLOR = BIT(3),
   FD_DIRTY_STENCIL_REF = BIT(4),
   FD_DIRTY_SAMPLE_MASK = BIT(5),
   FD_DIRTY_FRAMEBUFFER = BIT(6),
   FD_DIRTY_STIPPLE = BIT(7),
   FD_DIRTY_VIEWPORT = BIT(8),
   FD_DIRTY_VTXSTATE = BIT(9),
   FD_DIRTY_VTXBUF = BIT(10),
   FD_DIRTY_MIN_SAMPLES = BIT(11),
   FD_DIRTY_SCISSOR = BIT(12),
   FD_DIRTY_STREAMOUT = BIT(13),
   FD_DIRTY_UCP = BIT(14),
   FD_DIRTY_PROG = BIT(15),
   FD_DIRTY_CONST = BIT(16),
   FD_DIRTY_TEX = BIT(17),
   FD_DIRTY_IMAGE = BIT(18),
   FD_DIRTY_SSBO = BIT(19),
   FD_DIRTY_QUERY = BIT(20),
   FD_DIRTY_SAMPLE_LOCATIONS = BIT(21),

   /* only used by a2xx.. possibly can be removed.. */
   FD_DIRTY_TEXSTATE = BIT(22),

   /* fine grained state changes, for cases where state is not orthogonal
    * from hw perspective:
    */
   FD_DIRTY_RASTERIZER_DISCARD = BIT(24),
   FD_DIRTY_RASTERIZER_CLIP_PLANE_ENABLE = BIT(25),
   FD_DIRTY_BLEND_DUAL = BIT(26),
   FD_DIRTY_BLEND_COHERENT = BIT(27),
#define NUM_DIRTY_BITS 28
};

static inline void
fd_print_dirty_state(BITMASK_ENUM(fd_dirty_3d_state) dirty)
{
#ifdef DEBUG
   if (!FD_DBG(MSGS))
      return;

   struct {
      enum fd_dirty_3d_state state;
      const char *name;
   } tbl[] = {
#define STATE(n) { FD_DIRTY_ ## n, #n }
         STATE(BLEND),
         STATE(RASTERIZER),
         STATE(ZSA),
         STATE(BLEND_COLOR),
         STATE(STENCIL_REF),
         STATE(SAMPLE_MASK),
         STATE(FRAMEBUFFER),
         STATE(STIPPLE),
         STATE(VIEWPORT),
         STATE(VTXSTATE),
         STATE(VTXBUF),
         STATE(MIN_SAMPLES),
         STATE(SCISSOR),
         STATE(STREAMOUT),
         STATE(UCP),
         STATE(PROG),
         STATE(CONST),
         STATE(TEX),
         STATE(IMAGE),
         STATE(SSBO),
         STATE(QUERY),
         STATE(TEXSTATE),
         STATE(RASTERIZER_DISCARD),
         STATE(RASTERIZER_CLIP_PLANE_ENABLE),
         STATE(BLEND_DUAL),
         STATE(BLEND_COHERENT),
#undef STATE
   };

   struct log_stream *s = mesa_log_streami();

   mesa_log_stream_printf(s, "dirty:");

   if ((uint32_t)dirty == ~0) {
      mesa_log_stream_printf(s, " ALL");
      dirty = 0;
   }

   for (unsigned i = 0; i < ARRAY_SIZE(tbl); i++) {
      if (dirty & tbl[i].state) {
         mesa_log_stream_printf(s, " %s", tbl[i].name);
         dirty &= ~tbl[i].state;
      }
   }

   assert(!dirty);

   mesa_log_stream_destroy(s);
#endif
}

/* per shader-stage dirty state: */
enum fd_dirty_shader_state {
   FD_DIRTY_SHADER_PROG = BIT(0),
   FD_DIRTY_SHADER_CONST = BIT(1),
   FD_DIRTY_SHADER_TEX = BIT(2),
   FD_DIRTY_SHADER_SSBO = BIT(3),
   FD_DIRTY_SHADER_IMAGE = BIT(4),
#define NUM_DIRTY_SHADER_BITS 5
};

enum fd_buffer_mask {
   /* align bitmask values w/ PIPE_CLEAR_*.. since that is convenient.. */
   FD_BUFFER_COLOR = PIPE_CLEAR_COLOR,
   FD_BUFFER_DEPTH = PIPE_CLEAR_DEPTH,
   FD_BUFFER_STENCIL = PIPE_CLEAR_STENCIL,
   FD_BUFFER_ALL = FD_BUFFER_COLOR | FD_BUFFER_DEPTH | FD_BUFFER_STENCIL,

   /* A special internal buffer bit to signify that the LRZ buffer needs
    * clearing
    */
   FD_BUFFER_LRZ = BIT(15),
};

#define MAX_HW_SAMPLE_PROVIDERS 10
struct fd_hw_sample_provider;
struct fd_hw_sample;

struct ir3_shader_key;

struct fd_context {
   struct pipe_context base;

   unsigned flags;      /* PIPE_CONTEXT_x */

   struct threaded_context *tc;

   struct list_head node; /* node in screen->context_list */

   /* We currently need to serialize emitting GMEM batches, because of
    * VSC state access in the context.
    *
    * In practice this lock should not be contended, since pipe_context
    * use should be single threaded.  But it is needed to protect the
    * case, with batch reordering where a ctxB batch triggers flushing
    * a ctxA batch
    */
   simple_mtx_t gmem_lock;

   struct fd_device *dev;
   struct fd_screen *screen;
   struct fd_pipe *pipe;

   struct blitter_context *blitter dt;
   void *clear_rs_state[2] dt;

   /* slab for pipe_transfer allocations: */
   struct slab_child_pool transfer_pool dt;
   struct slab_child_pool transfer_pool_unsync; /* for threaded_context */

   struct fd_autotune autotune dt;

   /**
    * query related state:
    */
   /*@{*/
   /* slabs for fd_hw_sample and fd_hw_sample_period allocations: */
   struct slab_mempool sample_pool dt;
   struct slab_mempool sample_period_pool dt;

   /* sample-providers for hw queries: */
   const struct fd_hw_sample_provider
      *hw_sample_providers[MAX_HW_SAMPLE_PROVIDERS];

   /* list of active queries: */
   struct list_head hw_active_queries dt;

   /* sample-providers for accumulating hw queries: */
   const struct fd_acc_sample_provider
      *acc_sample_providers[MAX_HW_SAMPLE_PROVIDERS];

   /* list of active accumulating queries: */
   struct list_head acc_active_queries dt;
   /*@}*/

   float default_outer_level[4] dt;
   float default_inner_level[2] dt;
   uint8_t patch_vertices dt;

   /* Current state of pctx->set_active_query_state() (i.e. "should drawing
    * be counted against non-perfcounter queries")
    */
   bool active_queries dt;

   /* shaders used by clear, and gmem->mem blits: */
   struct fd_program_stateobj solid_prog; // TODO move to screen?
   struct fd_program_stateobj solid_layered_prog;

   /* shaders used by mem->gmem blits: */
   struct fd_program_stateobj
      blit_prog[MAX_RENDER_TARGETS]; // TODO move to screen?
   struct fd_program_stateobj blit_z, blit_zs;

   /* Stats/counters:
    */
   struct {
      uint64_t prims_emitted;
      uint64_t prims_generated;
      uint64_t draw_calls;
      uint64_t batch_total, batch_sysmem, batch_gmem, batch_nondraw,
         batch_restore;
      uint64_t staging_uploads, shadow_uploads;
      uint64_t vs_regs, hs_regs, ds_regs, gs_regs, fs_regs;
   } stats dt;

   /* Counter for number of users who need sw counters (so we can
    * skip collecting them when not needed)
    */
   unsigned stats_users;

   /* Current batch.. the rule here is that you can deref ctx->batch
    * in codepaths from pipe_context entrypoints.  But not in code-
    * paths from fd_batch_flush() (basically, the stuff that gets
    * called from GMEM code), since in those code-paths the batch
    * you care about is not necessarily the same as ctx->batch.
    */
   struct fd_batch *batch dt;

   /* Current nondraw batch.  Rules are the same as for draw batch.
    */
   struct fd_batch *batch_nondraw dt;

   /* NULL if there has been rendering since last flush.  Otherwise
    * keeps a reference to the last fence so we can re-use it rather
    * than having to flush no-op batch.
    */
   struct pipe_fence_handle *last_fence dt;

   /* Fence fd we are told to wait on via ->fence_server_sync() (or -1
    * if none).  The in-fence is transferred over to the batch on the
    * next draw/blit/grid.
    *
    * The reason for this extra complexity is that apps will typically
    * do eglWaitSyncKHR()/etc at the beginning of the frame, before the
    * first draw.  But mesa/st doesn't flush down framebuffer state
    * change until we hit a draw, so at ->fence_server_sync() time, we
    * don't yet have the correct batch.  If we created a batch at that
    * point, it would be the wrong one, and we'd have to flush it pre-
    * maturely, causing us to stall early in the frame where we could
    * be building up cmdstream.
    */
   int in_fence_fd dt;

   /**
    * If we *ever* see an in-fence-fd, assume that userspace is
    * not relying on implicit fences.
    */
   bool no_implicit_sync;

   /* track last known reset status globally and per-context to
    * determine if more resets occurred since then.  If global reset
    * count increases, it means some other context crashed.  If
    * per-context reset count increases, it means we crashed the
    * gpu.
    *
    * Only accessed by front-end thread, never accessed by TC driver
    * thread.
    */
   uint32_t context_reset_count;
   uint32_t global_reset_count;

   /* Context sequence #, used for batch-cache key: */
   uint16_t seqno;

   /* Cost per draw, used in conjunction with samples-passed history to
    * estimate whether GMEM or bypass is the better option.
    */
   uint8_t draw_cost;

   /* Are we in process of shadowing a resource? Used to detect recursion
    * in transfer_map, and skip unneeded synchronization.
    */
   bool in_shadow : 1 dt;

   /* For catching recursion problems with blit fallback: */
   bool in_blit : 1 dt;

   /* points to either scissor or disabled_scissor depending on rast state: */
   struct pipe_scissor_state *current_scissor dt;

   /* Note that all the scissor state that is traced is inclusive, ie the
    * maxiumum maxx is one less than the width.
    */
   struct pipe_scissor_state scissor[PIPE_MAX_VIEWPORTS] dt;

   /* we don't have a disable/enable bit for scissor, so instead we keep
    * a disabled-scissor state which matches the entire bound framebuffer
    * and use that when scissor is not enabled.
    */
   struct pipe_scissor_state disabled_scissor[PIPE_MAX_VIEWPORTS] dt;

   /* Per vsc pipe bo's (a2xx-a5xx): */
   struct fd_bo *vsc_pipe_bo[32] dt;

   /* Table of bo's attached to all batches up-front (because they
    * are commonly used, and that is easier than attaching on-use).
    * In particular, these are driver internal buffers which do not
    * participate in batch resource tracking.
    */
   struct fd_bo *private_bos[3];
   unsigned num_private_bos;

   /* Maps generic gallium oriented fd_dirty_3d_state bits to generation
    * specific bitmask of state "groups".
    */
   uint32_t gen_dirty_map[NUM_DIRTY_BITS];
   uint32_t gen_dirty_shader_map[PIPE_SHADER_TYPES][NUM_DIRTY_SHADER_BITS];

   /* Bitmask of all possible gen_dirty bits: */
   uint32_t gen_all_dirty;

   /* Generation specific bitmask of dirty state groups: */
   uint32_t gen_dirty;

   /* which state objects need to be re-emit'd: */
   BITMASK_ENUM(fd_dirty_3d_state) dirty dt;

   /* As above, but also needs draw time resource tracking: */
   BITMASK_ENUM(fd_dirty_3d_state) dirty_resource dt;

   /* per shader-stage dirty status: */
   BITMASK_ENUM(fd_dirty_shader_state) dirty_shader[PIPE_SHADER_TYPES] dt;

   /* As above, but also needs draw time resource tracking: */
   BITMASK_ENUM(fd_dirty_shader_state) dirty_shader_resource[PIPE_SHADER_TYPES] dt;

   void *compute dt;
   struct pipe_blend_state *blend dt;
   struct pipe_rasterizer_state *rasterizer dt;
   struct pipe_depth_stencil_alpha_state *zsa dt;

   struct fd_texture_stateobj tex[PIPE_SHADER_TYPES] dt;

   struct fd_program_stateobj prog dt;
   uint32_t bound_shader_stages dt;

   struct fd_vertex_state vtx dt;

   struct pipe_blend_color blend_color dt;
   struct pipe_stencil_ref stencil_ref dt;
   unsigned sample_mask dt;
   unsigned min_samples dt;

   /* 1x1 grid, max 4x MSAA: */
   uint8_t sample_locations[4] dt;
   bool sample_locations_enabled dt;

   /* local context fb state, for when ctx->batch is null: */
   struct pipe_framebuffer_state framebuffer dt;
   uint32_t all_mrt_channel_mask dt;

   struct pipe_poly_stipple stipple dt;
   struct pipe_viewport_state viewport[PIPE_MAX_VIEWPORTS] dt;
   struct pipe_scissor_state viewport_scissor[PIPE_MAX_VIEWPORTS] dt;
   struct {
      unsigned x, y;
   } guardband dt;
   struct fd_constbuf_stateobj constbuf[PIPE_SHADER_TYPES] dt;
   struct fd_shaderbuf_stateobj shaderbuf[PIPE_SHADER_TYPES] dt;
   struct fd_shaderimg_stateobj shaderimg[PIPE_SHADER_TYPES] dt;
   struct fd_streamout_stateobj streamout dt;
   struct fd_global_bindings_stateobj global_bindings dt;
   struct pipe_clip_state ucp dt;

   struct pipe_query *cond_query dt;
   bool cond_cond dt; /* inverted rendering condition */
   uint cond_mode dt;

   /* Private memory is a memory space where each fiber gets its own piece of
    * memory, in addition to registers. It is backed by a buffer which needs
    * to be large enough to hold the contents of every possible wavefront in
    * every core of the GPU. Because it allocates space via the internal
    * wavefront ID which is shared between all currently executing shaders,
    * the same buffer can be reused by all shaders, as long as all shaders
    * sharing the same buffer use the exact same configuration. There are two
    * inputs to the configuration, the amount of per-fiber space and whether
    * to use the newer per-wave or older per-fiber layout. We only ever
    * increase the size, and shaders with a smaller size requirement simply
    * use the larger existing buffer, so that we only need to keep track of
    * one buffer and its size, but we still need to keep track of per-fiber
    * and per-wave buffers separately so that we never use the same buffer
    * for different layouts. pvtmem[0] is for per-fiber, and pvtmem[1] is for
    * per-wave.
    */
   struct {
      struct fd_bo *bo;
      uint32_t per_fiber_size;
      uint32_t per_sp_size;
   } pvtmem[2] dt;

   /* maps per-shader-stage state plus variant key to hw
    * program stateobj:
    */
   struct ir3_cache *shader_cache;

   struct util_debug_callback debug;

   struct u_trace_context trace_context dt;

#ifdef HAVE_PERFETTO
   struct fd_perfetto_state perfetto;
#endif

   /*
    * Counter to generate submit-ids
    */
   uint32_t submit_count;

   /* Called on rebind_resource() for any per-gen cleanup required: */
   void (*rebind_resource)(struct fd_context *ctx, struct fd_resource *rsc) dt;

   /* GMEM/tile handling fxns: */
   void (*emit_tile_init)(struct fd_batch *batch) dt;
   void (*emit_tile_prep)(struct fd_batch *batch,
                          const struct fd_tile *tile) dt;
   void (*emit_tile_mem2gmem)(struct fd_batch *batch,
                              const struct fd_tile *tile) dt;
   void (*emit_tile_renderprep)(struct fd_batch *batch,
                                const struct fd_tile *tile) dt;
   void (*emit_tile)(struct fd_batch *batch, const struct fd_tile *tile) dt;
   void (*emit_tile_gmem2mem)(struct fd_batch *batch,
                              const struct fd_tile *tile) dt;
   void (*emit_tile_fini)(struct fd_batch *batch) dt; /* optional */

   /* optional, for GMEM bypass: */
   void (*emit_sysmem_prep)(struct fd_batch *batch) dt;
   void (*emit_sysmem)(struct fd_batch *batch) dt;
   void (*emit_sysmem_fini)(struct fd_batch *batch) dt;

   /* draw: */
   void (*draw_vbos)(struct fd_context *ctx, const struct pipe_draw_info *info,
                     unsigned drawid_offset,
                     const struct pipe_draw_indirect_info *indirect,
                     const struct pipe_draw_start_count_bias *draws,
                     unsigned num_draws,
                     unsigned index_offset) dt;
   bool (*clear)(struct fd_context *ctx, enum fd_buffer_mask buffers,
                 const union pipe_color_union *color, double depth,
                 unsigned stencil) dt;

   /* called to update draw_vbo func after bound shader stages change, etc: */
   void (*update_draw)(struct fd_context *ctx);

   /* compute: */
   void (*launch_grid)(struct fd_context *ctx,
                       const struct pipe_grid_info *info) dt;

   /* query: */
   struct fd_query *(*create_query)(struct fd_context *ctx, unsigned query_type,
                                    unsigned index);
   void (*query_prepare)(struct fd_batch *batch, uint32_t num_tiles) dt;
   void (*query_prepare_tile)(struct fd_batch *batch, uint32_t n,
                              struct fd_ringbuffer *ring) dt;
   void (*query_update_batch)(struct fd_batch *batch, bool disable_all) dt;

   /* blitter: */
   bool (*blit)(struct fd_context *ctx, const struct pipe_blit_info *info) dt;
   void (*clear_ubwc)(struct fd_batch *batch, struct fd_resource *rsc) dt;

   /* uncompress resource, if necessary, to use as the specified format: */
   void (*validate_format)(struct fd_context *ctx, struct fd_resource *rsc,
                           enum pipe_format format) dt;

   /* logger: */
   void (*record_timestamp)(struct fd_ringbuffer *ring, struct fd_bo *bo,
                            unsigned offset);
   uint64_t (*ts_to_ns)(uint64_t ts);

   /*
    * Common pre-cooked VBO state (used for a3xx and later):
    */

   /* for clear/gmem->mem vertices, and mem->gmem */
   struct pipe_resource *solid_vbuf;

   /* for mem->gmem tex coords: */
   struct pipe_resource *blit_texcoord_vbuf;

   /* vertex state for solid_vbuf:
    *    - solid_vbuf / 12 / R32G32B32_FLOAT
    */
   struct fd_vertex_state solid_vbuf_state;

   /* vertex state for blit_prog:
    *    - blit_texcoord_vbuf / 8 / R32G32_FLOAT
    *    - solid_vbuf / 12 / R32G32B32_FLOAT
    */
   struct fd_vertex_state blit_vbuf_state;

   /*
    * Info about state of previous draw, for state that comes from
    * pipe_draw_info (ie. not part of a CSO).  This allows us to
    * skip some register emit when the state doesn't change from
    * draw-to-draw
    */
   struct {
      bool dirty; /* last draw state unknown */
      bool primitive_restart;
      uint32_t index_start;
      uint32_t instance_start;
      uint32_t restart_index;
      uint32_t streamout_mask;

      /* some state changes require a different shader variant.  Keep
       * track of this so we know when we need to re-emit shader state
       * due to variant change.  See ir3_fixup_shader_state()
       *
       * (used for a3xx+, NULL otherwise)
       */
      struct ir3_shader_key *key;

   } last dt;
};

static inline struct fd_context *
fd_context(struct pipe_context *pctx)
{
   return (struct fd_context *)pctx;
}

static inline struct fd_stream_output_target *
fd_stream_output_target(struct pipe_stream_output_target *target)
{
   return (struct fd_stream_output_target *)target;
}

void fd_context_add_private_bo(struct fd_context *ctx, struct fd_bo *bo);

/* Mark specified non-shader-stage related state as dirty: */
static inline void
fd_context_dirty(struct fd_context *ctx, BITMASK_ENUM(fd_dirty_3d_state) dirty)
   assert_dt
{
   assert(util_is_power_of_two_nonzero(dirty));
   assert(ffs(dirty) <= ARRAY_SIZE(ctx->gen_dirty_map));

   ctx->gen_dirty |= ctx->gen_dirty_map[ffs(dirty) - 1];
   ctx->dirty |= dirty;

   /* These are still not handled at bind time: */
   if (dirty & (FD_DIRTY_FRAMEBUFFER | FD_DIRTY_QUERY | FD_DIRTY_ZSA))
      ctx->dirty_resource |= dirty;
}

static inline enum fd_dirty_3d_state
dirty_shader_to_dirty_state(BITMASK_ENUM(fd_dirty_shader_state) dirty)
{
   const enum fd_dirty_3d_state map[] = {
      FD_DIRTY_PROG, FD_DIRTY_CONST, FD_DIRTY_TEX,
      FD_DIRTY_SSBO, FD_DIRTY_IMAGE,
   };

   /* Need to update the table above if these shift: */
   STATIC_ASSERT(FD_DIRTY_SHADER_PROG == BIT(0));
   STATIC_ASSERT(FD_DIRTY_SHADER_CONST == BIT(1));
   STATIC_ASSERT(FD_DIRTY_SHADER_TEX == BIT(2));
   STATIC_ASSERT(FD_DIRTY_SHADER_SSBO == BIT(3));
   STATIC_ASSERT(FD_DIRTY_SHADER_IMAGE == BIT(4));

   assert(ffs(dirty) <= ARRAY_SIZE(map));

   return map[ffs(dirty) - 1];
}

static inline void
fd_context_dirty_shader(struct fd_context *ctx, enum pipe_shader_type shader,
                        BITMASK_ENUM(fd_dirty_shader_state) dirty)
   assert_dt
{
   assert(util_is_power_of_two_nonzero(dirty));
   ctx->gen_dirty |= ctx->gen_dirty_shader_map[shader][ffs(dirty) - 1];
   ctx->dirty_shader[shader] |= dirty;
   fd_context_dirty(ctx, dirty_shader_to_dirty_state(dirty));
}

/* mark all state dirty: */
static inline void
fd_context_all_dirty(struct fd_context *ctx) assert_dt
{
   ctx->last.dirty = true;
   ctx->dirty = (enum fd_dirty_3d_state) ~0;
   ctx->dirty_resource = (enum fd_dirty_3d_state) ~0;

   /* NOTE: don't use ~0 for gen_dirty, because the gen specific
    * emit code will loop over all the bits:
    */
   ctx->gen_dirty = ctx->gen_all_dirty;

   for (unsigned i = 0; i < PIPE_SHADER_TYPES; i++) {
      ctx->dirty_shader[i] = (enum fd_dirty_shader_state) ~0;
      ctx->dirty_shader_resource[i] = (enum fd_dirty_shader_state) ~0;
   }
}

static inline void
fd_context_all_clean(struct fd_context *ctx) assert_dt
{
   ctx->last.dirty = false;
   ctx->dirty = (enum fd_dirty_3d_state)0;
   ctx->dirty_resource = (enum fd_dirty_3d_state)0;
   ctx->gen_dirty = 0;
   for (unsigned i = 0; i < PIPE_SHADER_TYPES; i++) {
      ctx->dirty_shader[i] = (enum fd_dirty_shader_state)0;
      ctx->dirty_shader_resource[i] = (enum fd_dirty_shader_state)0;
   }
}

/**
 * Add mapping between global dirty bit and generation specific dirty
 * bit.
 */
static inline void
fd_context_add_map(struct fd_context *ctx, uint32_t dirty, uint32_t gen_dirty)
{
   u_foreach_bit (b, dirty) {
      ctx->gen_dirty_map[b] |= gen_dirty;
   }
   ctx->gen_all_dirty |= gen_dirty;
}

/**
 * Add mapping between shader stage specific dirty bit and generation
 * specific dirty bit
 */
static inline void
fd_context_add_shader_map(struct fd_context *ctx, enum pipe_shader_type shader,
                          BITMASK_ENUM(fd_dirty_shader_state) dirty, uint32_t gen_dirty)
{
   u_foreach_bit (b, dirty) {
      ctx->gen_dirty_shader_map[shader][b] |= gen_dirty;
   }
   ctx->gen_all_dirty |= gen_dirty;
}

static inline struct pipe_scissor_state *
fd_context_get_scissor(struct fd_context *ctx) assert_dt
{
   return ctx->current_scissor;
}

void fd_context_switch_from(struct fd_context *ctx) assert_dt;
void fd_context_switch_to(struct fd_context *ctx,
                          struct fd_batch *batch) assert_dt;
struct fd_batch *fd_context_batch(struct fd_context *ctx) assert_dt;
struct fd_batch *fd_context_batch_locked(struct fd_context *ctx) assert_dt;
struct fd_batch *fd_context_batch_nondraw(struct fd_context *ctx) assert_dt;

void fd_context_setup_common_vbos(struct fd_context *ctx);
void fd_context_cleanup_common_vbos(struct fd_context *ctx);
void fd_emit_string(struct fd_ringbuffer *ring, const char *string, int len);
void fd_emit_string5(struct fd_ringbuffer *ring, const char *string, int len);
__attribute__((format(printf, 3, 4))) void
fd_cs_trace_msg(struct u_trace_context *utctx, void *cs, const char *fmt, ...);
__attribute__((format(printf, 3, 4))) void
fd_cs_trace_start(struct u_trace_context *utctx, void *cs, const char *fmt,
                  ...);
__attribute__((format(printf, 3, 4))) void
fd_cs_trace_end(struct u_trace_context *utctx, void *cs, const char *fmt, ...);

struct pipe_context *fd_context_init(struct fd_context *ctx,
                                     struct pipe_screen *pscreen,
                                     void *priv, unsigned flags);
struct pipe_context *fd_context_init_tc(struct pipe_context *pctx,
                                        unsigned flags);

void fd_context_destroy(struct pipe_context *pctx) assert_dt;

#ifdef __cplusplus
}
#endif

#endif /* FREEDRENO_CONTEXT_H_ */
