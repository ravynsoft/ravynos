/*
 * Copyright © 2017 Intel Corporation
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
#ifndef CROCUS_CONTEXT_H
#define CROCUS_CONTEXT_H

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/u_debug.h"
#include "util/u_threaded_context.h"
#include "intel/blorp/blorp.h"
#include "intel/dev/intel_debug.h"
#include "intel/compiler/brw_compiler.h"
#include "crocus_batch.h"
#include "crocus_fence.h"
#include "crocus_resource.h"
#include "crocus_screen.h"
#include "util/u_blitter.h"

struct crocus_bo;
struct crocus_context;
struct blorp_batch;
struct blorp_params;

#define CROCUS_MAX_TEXTURE_BUFFER_SIZE (1 << 27)
#define CROCUS_MAX_TEXTURE_SAMPLERS 32
/* CROCUS_MAX_ABOS and CROCUS_MAX_SSBOS must be the same. */
#define CROCUS_MAX_ABOS 16
#define CROCUS_MAX_SSBOS 16
#define CROCUS_MAX_VIEWPORTS 16
#define CROCUS_MAX_CLIP_PLANES 8

enum crocus_param_domain {
   BRW_PARAM_DOMAIN_BUILTIN = 0,
   BRW_PARAM_DOMAIN_IMAGE,
};

enum {
   DRI_CONF_BO_REUSE_DISABLED,
   DRI_CONF_BO_REUSE_ALL
};

#define BRW_PARAM(domain, val)   (BRW_PARAM_DOMAIN_##domain << 24 | (val))
#define BRW_PARAM_DOMAIN(param)  ((uint32_t)(param) >> 24)
#define BRW_PARAM_VALUE(param)   ((uint32_t)(param) & 0x00ffffff)
#define BRW_PARAM_IMAGE(idx, offset) BRW_PARAM(IMAGE, ((idx) << 8) | (offset))
#define BRW_PARAM_IMAGE_IDX(value)   (BRW_PARAM_VALUE(value) >> 8)
#define BRW_PARAM_IMAGE_OFFSET(value)(BRW_PARAM_VALUE(value) & 0xf)

/**
 * Dirty flags.  When state changes, we flag some combination of these
 * to indicate that particular GPU commands need to be re-emitted.
 *
 * Each bit typically corresponds to a single 3DSTATE_* command packet, but
 * in rare cases they map to a group of related packets that need to be
 * emitted together.
 *
 * See crocus_upload_render_state().
 */
#define CROCUS_DIRTY_COLOR_CALC_STATE         (1ull <<  0)
#define CROCUS_DIRTY_POLYGON_STIPPLE          (1ull <<  1)
#define CROCUS_DIRTY_CC_VIEWPORT              (1ull <<  2)
#define CROCUS_DIRTY_SF_CL_VIEWPORT           (1ull <<  3)
#define CROCUS_DIRTY_RASTER                   (1ull <<  4)
#define CROCUS_DIRTY_CLIP                     (1ull <<  5)
#define CROCUS_DIRTY_LINE_STIPPLE             (1ull <<  6)
#define CROCUS_DIRTY_VERTEX_ELEMENTS          (1ull <<  7)
#define CROCUS_DIRTY_VERTEX_BUFFERS           (1ull <<  8)
#define CROCUS_DIRTY_DRAWING_RECTANGLE        (1ull <<  9)
#define CROCUS_DIRTY_GEN6_URB                 (1ull << 10)
#define CROCUS_DIRTY_DEPTH_BUFFER             (1ull << 11)
#define CROCUS_DIRTY_WM                       (1ull << 12)
#define CROCUS_DIRTY_SO_DECL_LIST             (1ull << 13)
#define CROCUS_DIRTY_STREAMOUT                (1ull << 14)
#define CROCUS_DIRTY_GEN4_CONSTANT_COLOR      (1ull << 15)
#define CROCUS_DIRTY_GEN4_CURBE               (1ull << 16)
#define CROCUS_DIRTY_GEN4_URB_FENCE           (1ull << 17)
#define CROCUS_DIRTY_GEN5_PIPELINED_POINTERS  (1ull << 18)
#define CROCUS_DIRTY_GEN5_BINDING_TABLE_POINTERS  (1ull << 19)
#define CROCUS_DIRTY_GEN6_BLEND_STATE         (1ull << 20)
#define CROCUS_DIRTY_GEN6_SCISSOR_RECT        (1ull << 21)
#define CROCUS_DIRTY_GEN6_WM_DEPTH_STENCIL    (1ull << 22)
#define CROCUS_DIRTY_GEN6_MULTISAMPLE         (1ull << 23)
#define CROCUS_DIRTY_GEN6_SAMPLE_MASK         (1ull << 24)
#define CROCUS_DIRTY_GEN7_SBE                 (1ull << 25)
#define CROCUS_DIRTY_GEN7_L3_CONFIG           (1ull << 26)
#define CROCUS_DIRTY_GEN7_SO_BUFFERS          (1ull << 27)
#define CROCUS_DIRTY_GEN75_VF                 (1ull << 28)
#define CROCUS_DIRTY_RENDER_RESOLVES_AND_FLUSHES  (1ull << 29)
#define CROCUS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES (1ull << 30)
#define CROCUS_DIRTY_VF_STATISTICS            (1ull << 31)
#define CROCUS_DIRTY_GEN4_CLIP_PROG           (1ull << 32)
#define CROCUS_DIRTY_GEN4_SF_PROG             (1ull << 33)
#define CROCUS_DIRTY_GEN4_FF_GS_PROG          (1ull << 34)
#define CROCUS_DIRTY_GEN6_SAMPLER_STATE_POINTERS (1ull << 35)
#define CROCUS_DIRTY_GEN6_SVBI                (1ull << 36)
#define CROCUS_DIRTY_GEN8_VF_TOPOLOGY         (1ull << 37)
#define CROCUS_DIRTY_GEN8_PMA_FIX             (1ull << 38)
#define CROCUS_DIRTY_GEN8_VF_SGVS             (1ull << 39)
#define CROCUS_DIRTY_GEN8_PS_BLEND            (1ull << 40)

#define CROCUS_ALL_DIRTY_FOR_COMPUTE (CROCUS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES)

#define CROCUS_ALL_DIRTY_FOR_RENDER (~CROCUS_ALL_DIRTY_FOR_COMPUTE)

/**
 * Per-stage dirty flags.  When state changes, we flag some combination of
 * these to indicate that particular GPU commands need to be re-emitted.
 * Unlike the IRIS_DIRTY_* flags these are shader stage-specific and can be
 * indexed by shifting the mask by the shader stage index.
 *
 * See crocus_upload_render_state().
 */
#define CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS        (1ull << 0)
#define CROCUS_STAGE_DIRTY_SAMPLER_STATES_TCS       (1ull << 1)
#define CROCUS_STAGE_DIRTY_SAMPLER_STATES_TES       (1ull << 2)
#define CROCUS_STAGE_DIRTY_SAMPLER_STATES_GS        (1ull << 3)
#define CROCUS_STAGE_DIRTY_SAMPLER_STATES_PS        (1ull << 4)
#define CROCUS_STAGE_DIRTY_SAMPLER_STATES_CS        (1ull << 5)
#define CROCUS_STAGE_DIRTY_UNCOMPILED_VS            (1ull << 6)
#define CROCUS_STAGE_DIRTY_UNCOMPILED_TCS           (1ull << 7)
#define CROCUS_STAGE_DIRTY_UNCOMPILED_TES           (1ull << 8)
#define CROCUS_STAGE_DIRTY_UNCOMPILED_GS            (1ull << 9)
#define CROCUS_STAGE_DIRTY_UNCOMPILED_FS            (1ull << 10)
#define CROCUS_STAGE_DIRTY_UNCOMPILED_CS            (1ull << 11)
#define CROCUS_STAGE_DIRTY_VS                       (1ull << 12)
#define CROCUS_STAGE_DIRTY_TCS                      (1ull << 13)
#define CROCUS_STAGE_DIRTY_TES                      (1ull << 14)
#define CROCUS_STAGE_DIRTY_GS                       (1ull << 15)
#define CROCUS_STAGE_DIRTY_FS                       (1ull << 16)
#define CROCUS_STAGE_DIRTY_CS                       (1ull << 17)
#define CROCUS_SHIFT_FOR_STAGE_DIRTY_CONSTANTS      18
#define CROCUS_STAGE_DIRTY_CONSTANTS_VS             (1ull << 18)
#define CROCUS_STAGE_DIRTY_CONSTANTS_TCS            (1ull << 19)
#define CROCUS_STAGE_DIRTY_CONSTANTS_TES            (1ull << 20)
#define CROCUS_STAGE_DIRTY_CONSTANTS_GS             (1ull << 21)
#define CROCUS_STAGE_DIRTY_CONSTANTS_FS             (1ull << 22)
#define CROCUS_STAGE_DIRTY_CONSTANTS_CS             (1ull << 23)
#define CROCUS_STAGE_DIRTY_BINDINGS_VS              (1ull << 24)
#define CROCUS_STAGE_DIRTY_BINDINGS_TCS             (1ull << 25)
#define CROCUS_STAGE_DIRTY_BINDINGS_TES             (1ull << 26)
#define CROCUS_STAGE_DIRTY_BINDINGS_GS              (1ull << 27)
#define CROCUS_STAGE_DIRTY_BINDINGS_FS              (1ull << 28)
#define CROCUS_STAGE_DIRTY_BINDINGS_CS              (1ull << 29)

#define CROCUS_ALL_STAGE_DIRTY_FOR_COMPUTE (CROCUS_STAGE_DIRTY_CS | \
                                          CROCUS_STAGE_DIRTY_SAMPLER_STATES_CS | \
                                          CROCUS_STAGE_DIRTY_UNCOMPILED_CS |    \
                                          CROCUS_STAGE_DIRTY_CONSTANTS_CS |     \
                                          CROCUS_STAGE_DIRTY_BINDINGS_CS)

#define CROCUS_ALL_STAGE_DIRTY_FOR_RENDER (~CROCUS_ALL_STAGE_DIRTY_FOR_COMPUTE)

#define CROCUS_ALL_STAGE_DIRTY_BINDINGS (CROCUS_STAGE_DIRTY_BINDINGS_VS  | \
                                       CROCUS_STAGE_DIRTY_BINDINGS_TCS | \
                                       CROCUS_STAGE_DIRTY_BINDINGS_TES | \
                                       CROCUS_STAGE_DIRTY_BINDINGS_GS  | \
                                       CROCUS_STAGE_DIRTY_BINDINGS_FS  | \
                                       CROCUS_STAGE_DIRTY_BINDINGS_CS)

#define CROCUS_RENDER_STAGE_DIRTY_CONSTANTS (CROCUS_STAGE_DIRTY_CONSTANTS_VS  | \
                                             CROCUS_STAGE_DIRTY_CONSTANTS_TCS | \
                                             CROCUS_STAGE_DIRTY_CONSTANTS_TES | \
                                             CROCUS_STAGE_DIRTY_CONSTANTS_GS  | \
                                             CROCUS_STAGE_DIRTY_CONSTANTS_FS)

/**
 * Non-orthogonal state (NOS) dependency flags.
 *
 * Shader programs may depend on non-orthogonal state.  These flags are
 * used to indicate that a shader's key depends on the state provided by
 * a certain Gallium CSO.  Changing any CSOs marked as a dependency will
 * cause the driver to re-compute the shader key, possibly triggering a
 * shader recompile.
 */
enum crocus_nos_dep {
   CROCUS_NOS_FRAMEBUFFER,
   CROCUS_NOS_DEPTH_STENCIL_ALPHA,
   CROCUS_NOS_RASTERIZER,
   CROCUS_NOS_BLEND,
   CROCUS_NOS_LAST_VUE_MAP,
   CROCUS_NOS_TEXTURES,
   CROCUS_NOS_VERTEX_ELEMENTS,
   CROCUS_NOS_COUNT,
};

struct crocus_depth_stencil_alpha_state;

/**
 * Cache IDs for the in-memory program cache (ice->shaders.cache).
 */
enum crocus_program_cache_id {
   CROCUS_CACHE_VS  = MESA_SHADER_VERTEX,
   CROCUS_CACHE_TCS = MESA_SHADER_TESS_CTRL,
   CROCUS_CACHE_TES = MESA_SHADER_TESS_EVAL,
   CROCUS_CACHE_GS  = MESA_SHADER_GEOMETRY,
   CROCUS_CACHE_FS  = MESA_SHADER_FRAGMENT,
   CROCUS_CACHE_CS  = MESA_SHADER_COMPUTE,
   CROCUS_CACHE_BLORP,
   CROCUS_CACHE_SF,
   CROCUS_CACHE_CLIP,
   CROCUS_CACHE_FF_GS,
};

/** @{
 *
 * Defines for PIPE_CONTROL operations, which trigger cache flushes,
 * synchronization, pipelined memory writes, and so on.
 *
 * The bits here are not the actual hardware values.  The actual fields
 * move between various generations, so we just have flags for each
 * potential operation, and use genxml to encode the actual packet.
 */
enum pipe_control_flags
{
   PIPE_CONTROL_FLUSH_LLC                       = (1 << 1),
   PIPE_CONTROL_LRI_POST_SYNC_OP                = (1 << 2),
   PIPE_CONTROL_STORE_DATA_INDEX                = (1 << 3),
   PIPE_CONTROL_CS_STALL                        = (1 << 4),
   PIPE_CONTROL_GLOBAL_SNAPSHOT_COUNT_RESET     = (1 << 5),
   PIPE_CONTROL_SYNC_GFDT                       = (1 << 6),
   PIPE_CONTROL_TLB_INVALIDATE                  = (1 << 7),
   PIPE_CONTROL_MEDIA_STATE_CLEAR               = (1 << 8),
   PIPE_CONTROL_WRITE_IMMEDIATE                 = (1 << 9),
   PIPE_CONTROL_WRITE_DEPTH_COUNT               = (1 << 10),
   PIPE_CONTROL_WRITE_TIMESTAMP                 = (1 << 11),
   PIPE_CONTROL_DEPTH_STALL                     = (1 << 12),
   PIPE_CONTROL_RENDER_TARGET_FLUSH             = (1 << 13),
   PIPE_CONTROL_INSTRUCTION_INVALIDATE          = (1 << 14),
   PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE        = (1 << 15),
   PIPE_CONTROL_INDIRECT_STATE_POINTERS_DISABLE = (1 << 16),
   PIPE_CONTROL_NOTIFY_ENABLE                   = (1 << 17),
   PIPE_CONTROL_FLUSH_ENABLE                    = (1 << 18),
   PIPE_CONTROL_DATA_CACHE_FLUSH                = (1 << 19),
   PIPE_CONTROL_VF_CACHE_INVALIDATE             = (1 << 20),
   PIPE_CONTROL_CONST_CACHE_INVALIDATE          = (1 << 21),
   PIPE_CONTROL_STATE_CACHE_INVALIDATE          = (1 << 22),
   PIPE_CONTROL_STALL_AT_SCOREBOARD             = (1 << 23),
   PIPE_CONTROL_DEPTH_CACHE_FLUSH               = (1 << 24),
   PIPE_CONTROL_TILE_CACHE_FLUSH                = (1 << 25),
};

#define PIPE_CONTROL_CACHE_FLUSH_BITS           \
   (PIPE_CONTROL_DEPTH_CACHE_FLUSH |            \
    PIPE_CONTROL_DATA_CACHE_FLUSH |             \
    PIPE_CONTROL_RENDER_TARGET_FLUSH)

#define PIPE_CONTROL_CACHE_INVALIDATE_BITS      \
   (PIPE_CONTROL_STATE_CACHE_INVALIDATE |       \
    PIPE_CONTROL_CONST_CACHE_INVALIDATE |       \
    PIPE_CONTROL_VF_CACHE_INVALIDATE |          \
    PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |     \
    PIPE_CONTROL_INSTRUCTION_INVALIDATE)

enum crocus_predicate_state {
   /* The first two states are used if we can determine whether to draw
    * without having to look at the values in the query object buffer. This
    * will happen if there is no conditional render in progress, if the query
    * object is already completed or if something else has already added
    * samples to the preliminary result.
    */
   CROCUS_PREDICATE_STATE_RENDER,
   CROCUS_PREDICATE_STATE_DONT_RENDER,

   /* In this case whether to draw or not depends on the result of an
    * MI_PREDICATE command so the predicate enable bit needs to be checked.
    */
   CROCUS_PREDICATE_STATE_USE_BIT,
   /* In this case, either MI_PREDICATE doesn't exist or we lack the
    * necessary kernel features to use it.  Stall for the query result.
    */
   CROCUS_PREDICATE_STATE_STALL_FOR_QUERY,
};

/** @} */

/**
 * An uncompiled, API-facing shader.  This is the Gallium CSO for shaders.
 * It primarily contains the NIR for the shader.
 *
 * Each API-facing shader can be compiled into multiple shader variants,
 * based on non-orthogonal state dependencies, recorded in the shader key.
 *
 * See crocus_compiled_shader, which represents a compiled shader variant.
 */
struct crocus_uncompiled_shader {
   struct nir_shader *nir;

   struct pipe_stream_output_info stream_output;

   /* A SHA1 of the serialized NIR for the disk cache. */
   unsigned char nir_sha1[20];

   unsigned program_id;

   /** Bitfield of (1 << CROCUS_NOS_*) flags. */
   unsigned nos;

   /** Have any shader variants been compiled yet? */
   bool compiled_once;

   bool needs_edge_flag;

   /** Constant data scraped from the shader by nir_opt_large_constants */
   struct pipe_resource *const_data;

   /** Surface state for const_data */
   struct crocus_state_ref const_data_state;
};

enum crocus_surface_group {
   CROCUS_SURFACE_GROUP_RENDER_TARGET,
   CROCUS_SURFACE_GROUP_RENDER_TARGET_READ,
   CROCUS_SURFACE_GROUP_SOL,
   CROCUS_SURFACE_GROUP_CS_WORK_GROUPS,
   CROCUS_SURFACE_GROUP_TEXTURE,
   CROCUS_SURFACE_GROUP_TEXTURE_GATHER,
   CROCUS_SURFACE_GROUP_IMAGE,
   CROCUS_SURFACE_GROUP_UBO,
   CROCUS_SURFACE_GROUP_SSBO,

   CROCUS_SURFACE_GROUP_COUNT,
};

enum {
   /* Invalid value for a binding table index. */
   CROCUS_SURFACE_NOT_USED = 0xa0a0a0a0,
};

struct crocus_binding_table {
   uint32_t size_bytes;

   /** Number of surfaces in each group, before compacting. */
   uint32_t sizes[CROCUS_SURFACE_GROUP_COUNT];

   /** Initial offset of each group. */
   uint32_t offsets[CROCUS_SURFACE_GROUP_COUNT];

   /** Mask of surfaces used in each group. */
   uint64_t used_mask[CROCUS_SURFACE_GROUP_COUNT];
};

/**
 * A compiled shader variant, containing a pointer to the GPU assembly,
 * as well as program data and other packets needed by state upload.
 *
 * There can be several crocus_compiled_shader variants per API-level shader
 * (crocus_uncompiled_shader), due to state-based recompiles (brw_*_prog_key).
 */
struct crocus_compiled_shader {
   /** Reference to the uploaded assembly. */
   uint32_t offset;

   /* asm size in map */
   uint32_t map_size;

   /** The program data (owned by the program cache hash table) */
   struct brw_stage_prog_data *prog_data;
   uint32_t prog_data_size;

   /** A list of system values to be uploaded as uniforms. */
   enum brw_param_builtin *system_values;
   unsigned num_system_values;

   /** Number of constbufs expected by the shader. */
   unsigned num_cbufs;

   /**
    * Derived 3DSTATE_STREAMOUT and 3DSTATE_SO_DECL_LIST packets
    * (the VUE-based information for transform feedback outputs).
    */
   uint32_t *streamout;

   struct crocus_binding_table bt;

   uint32_t bind_bo_offset;
   uint32_t surf_offset[128];//TODO
};

/**
 * API context state that is replicated per shader stage.
 */
struct crocus_shader_state {
   /** Uniform Buffers */
   struct pipe_constant_buffer constbufs[PIPE_MAX_CONSTANT_BUFFERS];

   bool sysvals_need_upload;

   /** Shader Storage Buffers */
   struct pipe_shader_buffer ssbo[PIPE_MAX_SHADER_BUFFERS];

   /** Shader Storage Images (image load store) */
   struct crocus_image_view image[PIPE_MAX_SHADER_IMAGES];

   struct crocus_sampler_state *samplers[CROCUS_MAX_TEXTURE_SAMPLERS];
   struct crocus_sampler_view *textures[CROCUS_MAX_TEXTURE_SAMPLERS];

   /** Bitfield of which constant buffers are bound (non-null). */
   uint32_t bound_cbufs;

   /** Bitfield of which image views are bound (non-null). */
   uint32_t bound_image_views;

   /** Bitfield of which sampler views are bound (non-null). */
   uint32_t bound_sampler_views;

   /** Bitfield of which shader storage buffers are bound (non-null). */
   uint32_t bound_ssbos;

   /** Bitfield of which shader storage buffers are writable. */
   uint32_t writable_ssbos;

   uint32_t sampler_offset;
};

/**
 * The API context (derived from pipe_context).
 *
 * Most driver state is tracked here.
 */
struct crocus_context {
   struct pipe_context ctx;
   struct threaded_context *thrctx;

   /** A debug callback for KHR_debug output. */
   struct util_debug_callback dbg;

   /** A device reset status callback for notifying that the GPU is hosed. */
   struct pipe_device_reset_callback reset;

   /** Slab allocator for crocus_transfer_map objects. */
   struct slab_child_pool transfer_pool;

   /** Slab allocator for threaded_context's crocus_transfer_map objects */
   struct slab_child_pool transfer_pool_unsync;

   struct blorp_context blorp;

   int batch_count;
   struct crocus_batch batches[CROCUS_BATCH_COUNT];

   struct u_upload_mgr *query_buffer_uploader;

   struct blitter_context *blitter;

   struct {
      struct {
         /**
          * Either the value of BaseVertex for indexed draw calls or the value
          * of the argument <first> for non-indexed draw calls.
          */
         int firstvertex;
         int baseinstance;
      } params;

      /**
       * Are the above values the ones stored in the draw_params buffer?
       * If so, we can compare them against new values to see if anything
       * changed.  If not, we need to assume they changed.
       */
      bool params_valid;

      /**
       * Resource and offset that stores draw_parameters from the indirect
       * buffer or to the buffer that stures the previous values for non
       * indirect draws.
       */
      struct crocus_state_ref draw_params;

      struct {
         /**
          * The value of DrawID. This always comes in from it's own vertex
          * buffer since it's not part of the indirect draw parameters.
          */
         int drawid;

         /**
          * Stores if an indexed or non-indexed draw (~0/0). Useful to
          * calculate BaseVertex as an AND of firstvertex and is_indexed_draw.
          */
         int is_indexed_draw;
      } derived_params;

      /**
       * Resource and offset used for GL_ARB_shader_draw_parameters which
       * contains parameters that are not present in the indirect buffer as
       * drawid and is_indexed_draw. They will go in their own vertex element.
       */
      struct crocus_state_ref derived_draw_params;
   } draw;

   struct {
      struct crocus_uncompiled_shader *uncompiled[MESA_SHADER_STAGES];
      struct crocus_compiled_shader *prog[MESA_SHADER_STAGES];
      struct brw_vue_map *last_vue_map;

      struct crocus_bo *cache_bo;
      uint32_t cache_next_offset;
      void *cache_bo_map;
      struct hash_table *cache;

      unsigned urb_size;

      /* gen 4/5 clip/sf progs */
      struct crocus_compiled_shader *clip_prog;
      struct crocus_compiled_shader *sf_prog;
      /* gen4/5 prims, gen6 streamout */
      struct crocus_compiled_shader *ff_gs_prog;
      uint32_t clip_offset;
      uint32_t sf_offset;
      uint32_t wm_offset;
      uint32_t vs_offset;
      uint32_t gs_offset;
      uint32_t cc_offset;

      /** Is a GS or TES outputting points or lines? */
      bool output_topology_is_points_or_lines;

      /* Track last VS URB entry size */
      unsigned last_vs_entry_size;

      /**
       * Scratch buffers for various sizes and stages.
       *
       * Indexed by the "Per-Thread Scratch Space" field's 4-bit encoding,
       * and shader stage.
       */
      struct crocus_bo *scratch_bos[1 << 4][MESA_SHADER_STAGES];
   } shaders;

   struct {
      struct crocus_query *query;
      bool condition;
      enum pipe_render_cond_flag mode;
   } condition;

   struct intel_perf_context *perf_ctx;

   struct {
      uint64_t dirty;
      uint64_t stage_dirty;
      uint64_t stage_dirty_for_nos[CROCUS_NOS_COUNT];

      unsigned num_viewports;
      unsigned sample_mask;
      struct crocus_blend_state *cso_blend;
      struct crocus_rasterizer_state *cso_rast;
      struct crocus_depth_stencil_alpha_state *cso_zsa;
      struct crocus_vertex_element_state *cso_vertex_elements;
      struct pipe_blend_color blend_color;
      struct pipe_poly_stipple poly_stipple;
      struct pipe_viewport_state viewports[CROCUS_MAX_VIEWPORTS];
      struct pipe_scissor_state scissors[CROCUS_MAX_VIEWPORTS];
      struct pipe_stencil_ref stencil_ref;
      struct pipe_framebuffer_state framebuffer;
      struct pipe_clip_state clip_planes;

      float default_outer_level[4];
      float default_inner_level[2];

      /** Bitfield of which vertex buffers are bound (non-null). */
      uint32_t bound_vertex_buffers;
      struct pipe_vertex_buffer vertex_buffers[16];
      uint32_t vb_end[16];

      bool primitive_restart;
      unsigned cut_index;
      enum mesa_prim reduced_prim_mode:8;
      enum mesa_prim prim_mode:8;
      bool prim_is_points_or_lines;
      uint8_t vertices_per_patch;
      uint8_t patch_vertices;

      bool window_space_position;

      /** The last compute group size */
      uint32_t last_block[3];

      /** The last compute grid size */
      uint32_t last_grid[3];
      /** Reference to the BO containing the compute grid size */
      struct crocus_state_ref grid_size;

      /**
       * Array of aux usages for drawing, altered to account for any
       * self-dependencies from resources bound for sampling and rendering.
       */
      enum isl_aux_usage draw_aux_usage[BRW_MAX_DRAW_BUFFERS];

      /** Aux usage of the fb's depth buffer (which may or may not exist). */
      enum isl_aux_usage hiz_usage;

      /** Bitfield of whether color blending is enabled for RT[i] */
      uint8_t blend_enables;

      /** Are depth writes enabled?  (Depth buffer may or may not exist.) */
      bool depth_writes_enabled;

      /** Are stencil writes enabled?  (Stencil buffer may or may not exist.) */
      bool stencil_writes_enabled;

      /** GenX-specific current state */
      struct crocus_genx_state *genx;

      struct crocus_shader_state shaders[MESA_SHADER_STAGES];

      /* track if geom shader is active for IVB GT2 workaround */
      bool gs_enabled;
      /** Do vertex shader uses shader draw parameters ? */
      bool vs_uses_draw_params;
      bool vs_uses_derived_draw_params;
      bool vs_needs_sgvs_element;
      bool vs_uses_vertexid;
      bool vs_uses_instanceid;

      /** Do vertex shader uses edge flag ? */
      bool vs_needs_edge_flag;

      struct pipe_stream_output_target *so_target[PIPE_MAX_SO_BUFFERS];
      bool streamout_active;
      int so_targets;

      bool statistics_counters_enabled;

      /** Current conditional rendering mode */
      enum crocus_predicate_state predicate;
      bool predicate_supported;

      /**
       * Query BO with a MI_PREDICATE_RESULT snapshot calculated on the
       * render context that needs to be uploaded to the compute context.
       */
      struct crocus_bo *compute_predicate;

      /** Is a PIPE_QUERY_PRIMITIVES_GENERATED query active? */
      bool prims_generated_query_active;

      /** 3DSTATE_STREAMOUT and 3DSTATE_SO_DECL_LIST packets */
      uint32_t *streamout;

      /**
       * Resources containing streamed state which our render context
       * currently points to.  Used to re-add these to the validation
       * list when we start a new batch and haven't resubmitted commands.
       */
      struct {
         struct pipe_resource *res;
         uint32_t offset;
         uint32_t size;
         uint32_t index_size;
         bool prim_restart;
      } index_buffer;

      uint32_t sf_vp_address;
      uint32_t clip_vp_address;
      uint32_t cc_vp_address;

      uint32_t stats_wm;
      float global_depth_offset_clamp;

      uint32_t last_xfb_verts_per_prim;
      uint64_t svbi;
   } state;

   /* BRW_NEW_URB_ALLOCATIONS:
    */
   struct {
      uint32_t vsize;                /* vertex size plus header in urb registers */
      uint32_t gsize;                /* GS output size in urb registers */
      uint32_t hsize;             /* Tessellation control output size in urb registers */
      uint32_t dsize;             /* Tessellation evaluation output size in urb registers */
      uint32_t csize;                /* constant buffer size in urb registers */
      uint32_t sfsize;                /* setup data size in urb registers */

      bool constrained;

      uint32_t nr_vs_entries;
      uint32_t nr_hs_entries;
      uint32_t nr_ds_entries;
      uint32_t nr_gs_entries;
      uint32_t nr_clip_entries;
      uint32_t nr_sf_entries;
      uint32_t nr_cs_entries;

      uint32_t vs_start;
      uint32_t hs_start;
      uint32_t ds_start;
      uint32_t gs_start;
      uint32_t clip_start;
      uint32_t sf_start;
      uint32_t cs_start;
      /**
       * URB size in the current configuration.  The units this is expressed
       * in are somewhat inconsistent, see intel_device_info::urb::size.
       *
       * FINISHME: Represent the URB size consistently in KB on all platforms.
       */
      uint32_t size;

      /* True if the most recently sent _3DSTATE_URB message allocated
       * URB space for the GS.
       */
      bool gs_present;

      /* True if the most recently sent _3DSTATE_URB message allocated
       * URB space for the HS and DS.
       */
      bool tess_present;
   } urb;

   /* GEN4/5 curbe */
   struct {
      unsigned wm_start;
      unsigned wm_size;
      unsigned clip_start;
      unsigned clip_size;
      unsigned vs_start;
      unsigned vs_size;
      unsigned total_size;

      struct crocus_resource *curbe_res;
      unsigned curbe_offset;
   } curbe;

   /**
    * A buffer containing a marker + description of the driver. This buffer is
    * added to all execbufs syscalls so that we can identify the driver that
    * generated a hang by looking at the content of the buffer in the error
    * state. It is also used for hardware workarounds that require scratch
    * writes or reads from some unimportant memory. To avoid overriding the
    * debug data, use the workaround_address field for workarounds.
    */
   struct crocus_bo *workaround_bo;
   unsigned workaround_offset;
};

#define perf_debug(dbg, ...) do {                      \
   if (INTEL_DEBUG(DEBUG_PERF))                        \
      dbg_printf(__VA_ARGS__);                         \
   if (unlikely(dbg))                                  \
      util_debug_message(dbg, PERF_INFO, __VA_ARGS__); \
} while(0)


struct pipe_context *
crocus_create_context(struct pipe_screen *screen, void *priv, unsigned flags);

void crocus_lost_context_state(struct crocus_batch *batch);

void crocus_init_blit_functions(struct pipe_context *ctx);
void crocus_init_clear_functions(struct pipe_context *ctx);
void crocus_init_program_functions(struct pipe_context *ctx);
void crocus_init_resource_functions(struct pipe_context *ctx);
void crocus_init_perfquery_functions(struct pipe_context *ctx);
bool crocus_update_compiled_shaders(struct crocus_context *ice);
void crocus_update_compiled_compute_shader(struct crocus_context *ice);
void crocus_fill_cs_push_const_buffer(struct brw_cs_prog_data *cs_prog_data,
                                      unsigned threads, uint32_t *dst);


/* crocus_blit.c */
enum crocus_blitter_op
{
   CROCUS_SAVE_TEXTURES      = 1,
   CROCUS_SAVE_FRAMEBUFFER   = 2,
   CROCUS_SAVE_FRAGMENT_STATE = 4,
   CROCUS_DISABLE_RENDER_COND = 8,
};
void crocus_blitter_begin(struct crocus_context *ice, enum crocus_blitter_op op, bool render_cond);

void crocus_blorp_surf_for_resource(struct crocus_vtable *vtbl,
                                    struct isl_device *isl_dev,
                                    struct blorp_surf *surf,
                                    struct pipe_resource *p_res,
                                    enum isl_aux_usage aux_usage,
                                    unsigned level,
                                    bool is_render_target);
void crocus_copy_region(struct blorp_context *blorp,
                        struct crocus_batch *batch,
                        struct pipe_resource *dst,
                        unsigned dst_level,
                        unsigned dstx, unsigned dsty, unsigned dstz,
                        struct pipe_resource *src,
                        unsigned src_level,
                        const struct pipe_box *src_box);

/* crocus_draw.c */
void crocus_draw_vbo(struct pipe_context *ctx,
                     const struct pipe_draw_info *info,
                     unsigned drawid_offset,
                     const struct pipe_draw_indirect_info *indirect,
                     const struct pipe_draw_start_count_bias *draws,
                     unsigned num_draws);
void crocus_launch_grid(struct pipe_context *, const struct pipe_grid_info *);

/* crocus_pipe_control.c */

void crocus_emit_pipe_control_flush(struct crocus_batch *batch,
                                    const char *reason, uint32_t flags);
void crocus_emit_pipe_control_write(struct crocus_batch *batch,
                                    const char *reason, uint32_t flags,
                                    struct crocus_bo *bo, uint32_t offset,
                                    uint64_t imm);
void crocus_emit_mi_flush(struct crocus_batch *batch);
void crocus_emit_depth_stall_flushes(struct crocus_batch *batch);
void crocus_emit_post_sync_nonzero_flush(struct crocus_batch *batch);
void crocus_emit_end_of_pipe_sync(struct crocus_batch *batch,
                                  const char *reason, uint32_t flags);
void crocus_flush_all_caches(struct crocus_batch *batch);

#define crocus_handle_always_flush_cache(batch)                 \
   if (unlikely(batch->screen->driconf.always_flush_cache))     \
      crocus_flush_all_caches(batch);

void crocus_init_flush_functions(struct pipe_context *ctx);

/* crocus_program.c */
const struct shader_info *crocus_get_shader_info(const struct crocus_context *ice,
                                                 gl_shader_stage stage);
struct crocus_bo *crocus_get_scratch_space(struct crocus_context *ice,
                                           unsigned per_thread_scratch,
                                           gl_shader_stage stage);
/**
 * Map a <group, index> pair to a binding table index.
 *
 * For example: <UBO, 5> => binding table index 12
 */
static inline uint32_t crocus_group_index_to_bti(const struct crocus_binding_table *bt,
                                                 enum crocus_surface_group group,
                                                 uint32_t index)
{
   assert(index < bt->sizes[group]);
   uint64_t mask = bt->used_mask[group];
   uint64_t bit = 1ull << index;
   if (bit & mask) {
      return bt->offsets[group] + util_bitcount64((bit - 1) & mask);
   } else {
      return CROCUS_SURFACE_NOT_USED;
   }
}

/**
 * Map a binding table index back to a <group, index> pair.
 *
 * For example: binding table index 12 => <UBO, 5>
 */
static inline uint32_t
crocus_bti_to_group_index(const struct crocus_binding_table *bt,
                          enum crocus_surface_group group, uint32_t bti)
{
   uint64_t used_mask = bt->used_mask[group];
   assert(bti >= bt->offsets[group]);

   uint32_t c = bti - bt->offsets[group];
   while (used_mask) {
      int i = u_bit_scan64(&used_mask);
      if (c == 0)
         return i;
      c--;
   }

   return CROCUS_SURFACE_NOT_USED;
}


/* crocus_disk_cache.c */

void crocus_disk_cache_store(struct disk_cache *cache,
                             const struct crocus_uncompiled_shader *ish,
                             const struct crocus_compiled_shader *shader,
                             void *map,
                             const void *prog_key,
                             uint32_t prog_key_size);
struct crocus_compiled_shader *
crocus_disk_cache_retrieve(struct crocus_context *ice,
                           const struct crocus_uncompiled_shader *ish,
                           const void *prog_key,
                           uint32_t prog_key_size);

/* crocus_program_cache.c */

void crocus_init_program_cache(struct crocus_context *ice);
void crocus_destroy_program_cache(struct crocus_context *ice);
void crocus_print_program_cache(struct crocus_context *ice);
struct crocus_compiled_shader *crocus_find_cached_shader(struct crocus_context *ice,
                                                         enum crocus_program_cache_id,
                                                         uint32_t key_size,
                                                         const void *key);
struct crocus_compiled_shader *crocus_upload_shader(struct crocus_context *ice,
                                                    enum crocus_program_cache_id,
                                                    uint32_t key_size,
                                                    const void *key,
                                                    const void *assembly,
                                                    uint32_t asm_size,
                                                    struct brw_stage_prog_data *,
                                                    uint32_t prog_data_size,
                                                    uint32_t *streamout,
                                                    enum brw_param_builtin *sysv,
                                                    unsigned num_system_values,
                                                    unsigned num_cbufs,
                                                    const struct crocus_binding_table *bt);
const void *crocus_find_previous_compile(const struct crocus_context *ice,
                                         enum crocus_program_cache_id cache_id,
                                         unsigned program_string_id);
bool crocus_blorp_lookup_shader(struct blorp_batch *blorp_batch,
                                const void *key,
                                uint32_t key_size,
                                uint32_t *kernel_out,
                                void *prog_data_out);
bool crocus_blorp_upload_shader(struct blorp_batch *blorp_batch,
                                uint32_t stage,
                                const void *key, uint32_t key_size,
                                const void *kernel, uint32_t kernel_size,
                                const struct brw_stage_prog_data *prog_data,
                                uint32_t prog_data_size,
                                uint32_t *kernel_out,
                                void *prog_data_out);

/* crocus_resolve.c */

void crocus_predraw_resolve_inputs(struct crocus_context *ice,
                                   struct crocus_batch *batch,
                                   bool *draw_aux_buffer_disabled,
                                   gl_shader_stage stage,
                                   bool consider_framebuffer);
void crocus_predraw_resolve_framebuffer(struct crocus_context *ice,
                                        struct crocus_batch *batch,
                                        bool *draw_aux_buffer_disabled);
void crocus_postdraw_update_resolve_tracking(struct crocus_context *ice,
                                             struct crocus_batch *batch);
void crocus_cache_sets_clear(struct crocus_batch *batch);
void crocus_flush_depth_and_render_caches(struct crocus_batch *batch);
void crocus_cache_flush_for_read(struct crocus_batch *batch, struct crocus_bo *bo);
void crocus_cache_flush_for_render(struct crocus_batch *batch,
                                   struct crocus_bo *bo,
                                   enum isl_format format,
                                   enum isl_aux_usage aux_usage);
void crocus_render_cache_add_bo(struct crocus_batch *batch,
                                struct crocus_bo *bo,
                                enum isl_format format,
                                enum isl_aux_usage aux_usage);
void crocus_cache_flush_for_depth(struct crocus_batch *batch, struct crocus_bo *bo);
void crocus_depth_cache_add_bo(struct crocus_batch *batch, struct crocus_bo *bo);
int crocus_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                                 struct pipe_driver_query_info *info);
int crocus_get_driver_query_group_info(struct pipe_screen *pscreen,
                                       unsigned index,
                                       struct pipe_driver_query_group_info *info);

struct pipe_rasterizer_state *crocus_get_rast_state(struct crocus_context *ctx);

bool crocus_sw_check_cond_render(struct crocus_context *ice);
static inline bool crocus_check_conditional_render(struct crocus_context *ice)
{
   if (ice->state.predicate == CROCUS_PREDICATE_STATE_STALL_FOR_QUERY)
      return crocus_sw_check_cond_render(ice);
   return ice->state.predicate != CROCUS_PREDICATE_STATE_DONT_RENDER;
}

#ifdef genX
#  include "crocus_genx_protos.h"
#else
#  define genX(x) gfx4_##x
#  include "crocus_genx_protos.h"
#  undef genX
#  define genX(x) gfx45_##x
#  include "crocus_genx_protos.h"
#  undef genX
#  define genX(x) gfx5_##x
#  include "crocus_genx_protos.h"
#  undef genX
#  define genX(x) gfx6_##x
#  include "crocus_genx_protos.h"
#  undef genX
#  define genX(x) gfx7_##x
#  include "crocus_genx_protos.h"
#  undef genX
#  define genX(x) gfx75_##x
#  include "crocus_genx_protos.h"
#  undef genX
#  define genX(x) gfx8_##x
#  include "crocus_genx_protos.h"
#  undef genX
#endif

#endif
