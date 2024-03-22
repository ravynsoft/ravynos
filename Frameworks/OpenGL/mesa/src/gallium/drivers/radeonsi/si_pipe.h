/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef SI_PIPE_H
#define SI_PIPE_H

#include "si_shader.h"
#include "si_state.h"
#include "winsys/radeon_winsys.h"
#include "util/u_blitter.h"
#include "util/u_idalloc.h"
#include "util/u_suballoc.h"
#include "util/u_threaded_context.h"
#include "util/u_vertex_state_cache.h"
#include "util/perf/u_trace.h"
#include "ac_sqtt.h"
#include "ac_spm.h"
#include "si_perfetto.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ac_llvm_compiler;

#define ATI_VENDOR_ID         0x1002
#define SI_NOT_QUERY          0xffffffff

/* special primitive types */
#define SI_PRIM_RECTANGLE_LIST MESA_PRIM_COUNT

/* The primitive restart can be any number, but we must pick one which will
 * mean "unknown" for the purpose of state tracking and the number shouldn't
 * be a commonly-used one.
 */
#define SI_RESTART_INDEX_UNKNOWN  ((unsigned)INT_MIN)
#define SI_INSTANCE_COUNT_UNKNOWN ((unsigned)INT_MIN)
#define SI_NUM_SMOOTH_AA_SAMPLES  4
#define SI_MAX_POINT_SIZE         2048
#define SI_GS_PER_ES              128
/* Alignment for optimal CP DMA performance. */
#define SI_CPDMA_ALIGNMENT 32

/* Tunables for compute-based clear_buffer and copy_buffer: */
#define SI_COMPUTE_CLEAR_DW_PER_THREAD 4
#define SI_COMPUTE_COPY_DW_PER_THREAD  4
/* L2 LRU is recommended because the compute shader can finish sooner due to fewer L2 evictions. */
#define SI_COMPUTE_DST_CACHE_POLICY    L2_LRU

/* Pipeline & streamout query controls. */
#define SI_CONTEXT_START_PIPELINE_STATS  (1 << 0)
#define SI_CONTEXT_STOP_PIPELINE_STATS   (1 << 1)
/* gap */
/* Instruction cache. */
#define SI_CONTEXT_INV_ICACHE (1 << 3)
/* Scalar cache. (GFX6-9: scalar L1; GFX10: scalar L0)
 * GFX10: This also invalidates the L1 shader array cache. */
#define SI_CONTEXT_INV_SCACHE (1 << 4)
/* Vector cache. (GFX6-9: vector L1; GFX10: vector L0)
 * GFX10: This also invalidates the L1 shader array cache. */
#define SI_CONTEXT_INV_VCACHE (1 << 5)
/* L2 cache + L2 metadata cache writeback & invalidate.
 * GFX6-8: Used by shaders only. GFX9-10: Used by everything. */
#define SI_CONTEXT_INV_L2 (1 << 6)
/* L2 writeback (write dirty L2 lines to memory for non-L2 clients).
 * Only used for coherency with non-L2 clients like CB, DB, CP on GFX6-8.
 * GFX6-7 will do complete invalidation, because the writeback is unsupported. */
#define SI_CONTEXT_WB_L2 (1 << 7)
/* Writeback & invalidate the L2 metadata cache only. It can only be coupled with
 * a CB or DB flush. */
#define SI_CONTEXT_INV_L2_METADATA (1 << 8)
/* Framebuffer caches. */
#define SI_CONTEXT_FLUSH_AND_INV_DB      (1 << 9)
#define SI_CONTEXT_FLUSH_AND_INV_DB_META (1 << 10)
#define SI_CONTEXT_FLUSH_AND_INV_CB      (1 << 11)
/* Engine synchronization. */
#define SI_CONTEXT_VS_PARTIAL_FLUSH   (1 << 12)
#define SI_CONTEXT_PS_PARTIAL_FLUSH   (1 << 13)
#define SI_CONTEXT_CS_PARTIAL_FLUSH   (1 << 14)
#define SI_CONTEXT_VGT_FLUSH          (1 << 15)
#define SI_CONTEXT_VGT_STREAMOUT_SYNC (1 << 16)
/* PFP waits for ME to finish. Used to sync for index and indirect buffers and render
 * condition. It's typically set when doing a VS/PS/CS partial flush for buffers. */
#define SI_CONTEXT_PFP_SYNC_ME        (1 << 17)

#define SI_PREFETCH_LS              (1 << 1)
#define SI_PREFETCH_HS              (1 << 2)
#define SI_PREFETCH_ES              (1 << 3)
#define SI_PREFETCH_GS              (1 << 4)
#define SI_PREFETCH_VS              (1 << 5)
#define SI_PREFETCH_PS              (1 << 6)

#define SI_MAX_BORDER_COLORS              4096
#define SI_MAX_VIEWPORTS                  16
#define SI_MAP_BUFFER_ALIGNMENT           64
/* We only support the minimum allowed value (512), so that we can pack a 3D block size
 * in 1 SGPR. */
#define SI_MAX_VARIABLE_THREADS_PER_BLOCK 512

#define SI_CONTEXT_FLAG_AUX               (1u << 31)

#define SI_RESOURCE_FLAG_FORCE_LINEAR      (PIPE_RESOURCE_FLAG_DRV_PRIV << 0)
#define SI_RESOURCE_FLAG_FLUSHED_DEPTH     (PIPE_RESOURCE_FLAG_DRV_PRIV << 1)
#define SI_RESOURCE_FLAG_FORCE_MSAA_TILING (PIPE_RESOURCE_FLAG_DRV_PRIV << 2)
#define SI_RESOURCE_FLAG_DISABLE_DCC       (PIPE_RESOURCE_FLAG_DRV_PRIV << 3)
#define SI_RESOURCE_FLAG_DRIVER_INTERNAL   (PIPE_RESOURCE_FLAG_DRV_PRIV << 4)
#define SI_RESOURCE_FLAG_READ_ONLY         (PIPE_RESOURCE_FLAG_DRV_PRIV << 5)
#define SI_RESOURCE_FLAG_32BIT             (PIPE_RESOURCE_FLAG_DRV_PRIV << 6)
#define SI_RESOURCE_FLAG_CLEAR             (PIPE_RESOURCE_FLAG_DRV_PRIV << 7)
#define SI_RESOURCE_AUX_PLANE              (PIPE_RESOURCE_FLAG_DRV_PRIV << 8)
/* Set a micro tile mode: */
#define SI_RESOURCE_FLAG_FORCE_MICRO_TILE_MODE (PIPE_RESOURCE_FLAG_DRV_PRIV << 9)
#define SI_RESOURCE_FLAG_MICRO_TILE_MODE_SHIFT (util_logbase2(PIPE_RESOURCE_FLAG_DRV_PRIV) + 10)
#define SI_RESOURCE_FLAG_MICRO_TILE_MODE_SET(x)                                                    \
   (((x)&0x3) << SI_RESOURCE_FLAG_MICRO_TILE_MODE_SHIFT)
#define SI_RESOURCE_FLAG_MICRO_TILE_MODE_GET(x)                                                    \
   (((x) >> SI_RESOURCE_FLAG_MICRO_TILE_MODE_SHIFT) & 0x3)
#define SI_RESOURCE_FLAG_GL2_BYPASS        (PIPE_RESOURCE_FLAG_DRV_PRIV << 12)
/* Discard instead of evict. */
#define SI_RESOURCE_FLAG_DISCARDABLE       (PIPE_RESOURCE_FLAG_DRV_PRIV << 13)

enum si_has_gs {
   GS_OFF,
   GS_ON,
};

enum si_has_tess {
   TESS_OFF,
   TESS_ON,
};

enum si_has_ngg {
   NGG_OFF,
   NGG_ON,
};

#define DCC_CODE(x) (((x) << 24) | ((x) << 16) | ((x) << 8) | (x))

enum si_clear_code
{
   /* Common clear codes. */
   DCC_CLEAR_0000    = DCC_CODE(0x00), /* all bits are 0 */
   DCC_UNCOMPRESSED  = DCC_CODE(0xFF),

   GFX8_DCC_CLEAR_0000     = DCC_CLEAR_0000,
   GFX8_DCC_CLEAR_0001     = DCC_CODE(0x40),
   GFX8_DCC_CLEAR_1110     = DCC_CODE(0x80),
   GFX8_DCC_CLEAR_1111     = DCC_CODE(0xC0),
   GFX8_DCC_CLEAR_REG      = DCC_CODE(0x20),
   GFX9_DCC_CLEAR_SINGLE   = DCC_CODE(0x10),

   GFX11_DCC_CLEAR_SINGLE     = DCC_CODE(0x01),
   GFX11_DCC_CLEAR_0000       = DCC_CLEAR_0000, /* all bits are 0 */
   GFX11_DCC_CLEAR_1111_UNORM = DCC_CODE(0x02), /* all bits are 1 */
   GFX11_DCC_CLEAR_1111_FP16  = DCC_CODE(0x04), /* all 16-bit words are 0x3c00, max 64bpp */
   GFX11_DCC_CLEAR_1111_FP32  = DCC_CODE(0x06), /* all 32-bit words are 0x3f800000 */
   /* Color bits are 0, alpha bits are 1; only 88, 8888, 16161616 */
   GFX11_DCC_CLEAR_0001_UNORM = DCC_CODE(0x08),
   /* Color bits are 1, alpha bits are 0, only 88, 8888, 16161616 */
   GFX11_DCC_CLEAR_1110_UNORM = DCC_CODE(0x0A),
};

#define SI_IMAGE_ACCESS_DCC_OFF              (1 << 8)
#define SI_IMAGE_ACCESS_ALLOW_DCC_STORE      (1 << 9)
#define SI_IMAGE_ACCESS_BLOCK_FORMAT_AS_UINT (1 << 10) /* for compressed/subsampled images */

enum si_occlusion_query_mode {
   SI_OCCLUSION_QUERY_MODE_DISABLE,
   SI_OCCLUSION_QUERY_MODE_PRECISE_INTEGER,
   SI_OCCLUSION_QUERY_MODE_PRECISE_BOOLEAN,
   SI_OCCLUSION_QUERY_MODE_CONSERVATIVE_BOOLEAN,
};

/* Debug flags. */
enum
{
   /* Shader logging options: */
   DBG_VS = MESA_SHADER_VERTEX,
   DBG_TCS = MESA_SHADER_TESS_CTRL,
   DBG_TES = MESA_SHADER_TESS_EVAL,
   DBG_GS = MESA_SHADER_GEOMETRY,
   DBG_PS = MESA_SHADER_FRAGMENT,
   DBG_CS = MESA_SHADER_COMPUTE,
   DBG_INIT_NIR,
   DBG_NIR,
   DBG_INIT_LLVM,
   DBG_LLVM,
   DBG_INIT_ACO,
   DBG_ACO,
   DBG_ASM,
   DBG_STATS,

   /* Shader compiler options the shader cache should be aware of: */
   DBG_W32_GE,
   DBG_W32_PS,
   DBG_W32_CS,
   DBG_W64_GE,
   DBG_W64_PS,
   DBG_W64_CS,

   /* Shader compiler options (with no effect on the shader cache): */
   DBG_CHECK_IR,
   DBG_MONOLITHIC_SHADERS,
   DBG_NO_OPT_VARIANT,

   /* Information logging options: */
   DBG_INFO,
   DBG_TEX,
   DBG_COMPUTE,
   DBG_VM,
   DBG_CACHE_STATS,
   DBG_IB,
   DBG_VERTEX_ELEMENTS,

   /* Driver options: */
   DBG_NO_WC,
   DBG_NO_WC_STREAM,
   DBG_CHECK_VM,
   DBG_RESERVE_VMID,
   DBG_SHADOW_REGS,
   DBG_NO_FAST_DISPLAY_LIST,
   DBG_NO_DMA_SHADERS,

   /* Multimedia options: */
   DBG_NO_EFC,

   /* 3D engine options: */
   DBG_NO_NGG,
   DBG_ALWAYS_NGG_CULLING_ALL,
   DBG_NO_NGG_CULLING,
   DBG_SWITCH_ON_EOP,
   DBG_NO_OUT_OF_ORDER,
   DBG_NO_DPBB,
   DBG_DPBB,
   DBG_NO_HYPERZ,
   DBG_NO_2D_TILING,
   DBG_NO_TILING,
   DBG_NO_DISPLAY_TILING,
   DBG_NO_DISPLAY_DCC,
   DBG_NO_EXPORTED_DCC,
   DBG_NO_DCC,
   DBG_NO_DCC_CLEAR,
   DBG_NO_DCC_STORE,
   DBG_DCC_STORE,
   DBG_NO_DCC_MSAA,
   DBG_NO_FMASK,
   DBG_NO_DMA,

   DBG_EXTRA_METADATA,

   DBG_TMZ,
   DBG_SQTT,
   DBG_USE_ACO,

   DBG_COUNT
};

enum
{
   /* Tests: */
   DBG_TEST_IMAGE_COPY,
   DBG_TEST_CB_RESOLVE,
   DBG_TEST_COMPUTE_BLIT,
   DBG_TEST_VMFAULT_CP,
   DBG_TEST_VMFAULT_SHADER,
   DBG_TEST_DMA_PERF,
   DBG_TEST_GDS,
   DBG_TEST_GDS_MM,
   DBG_TEST_GDS_OA_MM,
};

#define DBG_ALL_SHADERS (((1 << (DBG_CS + 1)) - 1))
#define DBG(name)       (1ull << DBG_##name)

enum si_cache_policy
{
   L2_BYPASS,
   L2_STREAM, /* same as SLC=1 */
   L2_LRU,    /* same as SLC=0 */
};

enum si_coherency
{
   SI_COHERENCY_NONE, /* no cache flushes needed */
   SI_COHERENCY_SHADER,
   SI_COHERENCY_CB_META,
   SI_COHERENCY_DB_META,
   SI_COHERENCY_CP,
};

#define SI_BIND_CONSTANT_BUFFER_SHIFT     0
#define SI_BIND_SHADER_BUFFER_SHIFT       6
#define SI_BIND_IMAGE_BUFFER_SHIFT        12
#define SI_BIND_SAMPLER_BUFFER_SHIFT      18
#define SI_BIND_OTHER_BUFFER_SHIFT        24

/* Bind masks for all 6 shader stages. */
#define SI_BIND_CONSTANT_BUFFER_ALL       (0x3f << SI_BIND_CONSTANT_BUFFER_SHIFT)
#define SI_BIND_SHADER_BUFFER_ALL         (0x3f << SI_BIND_SHADER_BUFFER_SHIFT)
#define SI_BIND_IMAGE_BUFFER_ALL          (0x3f << SI_BIND_IMAGE_BUFFER_SHIFT)
#define SI_BIND_SAMPLER_BUFFER_ALL        (0x3f << SI_BIND_SAMPLER_BUFFER_SHIFT)

#define SI_BIND_CONSTANT_BUFFER(shader)   ((1 << (shader)) << SI_BIND_CONSTANT_BUFFER_SHIFT)
#define SI_BIND_SHADER_BUFFER(shader)     ((1 << (shader)) << SI_BIND_SHADER_BUFFER_SHIFT)
#define SI_BIND_IMAGE_BUFFER(shader)      ((1 << (shader)) << SI_BIND_IMAGE_BUFFER_SHIFT)
#define SI_BIND_SAMPLER_BUFFER(shader)    ((1 << (shader)) << SI_BIND_SAMPLER_BUFFER_SHIFT)
#define SI_BIND_VERTEX_BUFFER             (1 << (SI_BIND_OTHER_BUFFER_SHIFT + 0))
#define SI_BIND_STREAMOUT_BUFFER          (1 << (SI_BIND_OTHER_BUFFER_SHIFT + 1))

/* Only 32-bit buffer allocations are supported, gallium doesn't support more
 * at the moment.
 */
struct si_resource {
   struct threaded_resource b;

   /* If we remove this seemingly useless padding, performance in Viewperf2020/catiav5test1
    * decreases by 8%.
    */
   uint32_t _pad;

   /* Winsys objects. */
   struct pb_buffer_lean *buf;
   uint64_t gpu_address;

   /* Resource properties. */
   uint64_t bo_size;
   uint8_t bo_alignment_log2;
   enum radeon_bo_domain domains:8;
   enum radeon_bo_flag flags:16;
   unsigned bind_history; /* bitmask of SI_BIND_xxx_BUFFER */

   /* The buffer range which is initialized (with a write transfer,
    * streamout, DMA, or as a random access target). The rest of
    * the buffer is considered invalid and can be mapped unsynchronized.
    *
    * This allows unsynchronized mapping of a buffer range which hasn't
    * been used yet. It's for applications which forget to use
    * the unsynchronized map flag and expect the driver to figure it out.
    */
   struct util_range valid_buffer_range;

   /* For buffers only. This indicates that a write operation has been
    * performed by TC L2, but the cache hasn't been flushed.
    * Any hw block which doesn't use or bypasses TC L2 should check this
    * flag and flush the cache before using the buffer.
    *
    * For example, TC L2 must be flushed if a buffer which has been
    * modified by a shader store instruction is about to be used as
    * an index buffer. The reason is that VGT DMA index fetching doesn't
    * use TC L2.
    */
   bool TC_L2_dirty;

   /* Whether this resource is referenced by bindless handles. */
   bool texture_handle_allocated;
   bool image_handle_allocated;

   /* Whether the resource has been exported via resource_get_handle. */
   uint8_t external_usage; /* PIPE_HANDLE_USAGE_* */
};

struct si_transfer {
   struct threaded_transfer b;
   struct si_resource *staging;
};

struct si_texture {
   struct si_resource buffer;

   struct radeon_surf surface;
   struct si_texture *flushed_depth_texture;

   /* One texture allocation can contain these buffers:
    * - image (pixel data)
    * - FMASK buffer (MSAA compression)
    * - CMASK buffer (MSAA compression and/or legacy fast color clear)
    * - HTILE buffer (Z/S compression and fast Z/S clear)
    * - DCC buffer (color compression and new fast color clear)
    * - displayable DCC buffer (if the DCC buffer is not displayable)
    */
   uint64_t cmask_base_address_reg;
   struct si_resource *cmask_buffer;
   unsigned cb_color_info; /* fast clear enable bit */
   unsigned color_clear_value[2]; /* not on gfx11 */
   unsigned last_msaa_resolve_target_micro_mode;
   bool swap_rgb_to_bgr_on_next_clear;
   bool swap_rgb_to_bgr;
   unsigned num_level0_transfers;
   unsigned plane_index; /* other planes are different pipe_resources */
   unsigned num_planes;
   enum pipe_format multi_plane_format;

   /* Depth buffer compression and fast clear. */
   float depth_clear_value[RADEON_SURF_MAX_LEVELS];
   uint8_t stencil_clear_value[RADEON_SURF_MAX_LEVELS];
   uint16_t depth_cleared_level_mask_once; /* if it was cleared at least once */
   uint16_t depth_cleared_level_mask;     /* track if it's cleared (can be false negative) */
   uint16_t stencil_cleared_level_mask_once; /* if it was cleared at least once */
   uint16_t dirty_level_mask;         /* each bit says if that mipmap is compressed */
   uint16_t stencil_dirty_level_mask; /* each bit says if that mipmap is compressed */
   enum pipe_format db_render_format : 16;
   bool fmask_is_identity : 1;
   bool tc_compatible_htile : 1;
   bool enable_tc_compatible_htile_next_clear : 1;
   bool htile_stencil_disabled : 1;
   bool upgraded_depth : 1;  /* upgraded from unorm to Z32_FLOAT */
   bool is_depth : 1;
   bool db_compatible : 1;
   bool can_sample_z : 1;
   bool can_sample_s : 1;
   bool need_flush_after_depth_decompression: 1;

   /* We need to track DCC dirtiness, because st/dri usually calls
    * flush_resource twice per frame (not a bug) and we don't wanna
    * decompress DCC twice.
    */
   bool displayable_dcc_dirty : 1;

   /* Counter that should be non-zero if the texture is bound to a
    * framebuffer.
    */
   unsigned framebuffers_bound;
};

/* State trackers create separate textures in a next-chain for extra planes
 * even if those are planes created purely for modifiers. Because the linking
 * of the chain happens outside of the driver, and NULL is interpreted as
 * failure, let's create some dummy texture structs. We could use these
 * later to use the offsets for linking if we really wanted to.
 *
 * For now just create a dummy struct and completely ignore it.
 *
 * Potentially in the future we could store stride/offset and use it during
 * creation, though we might want to change how linking is done first.
 */
struct si_auxiliary_texture {
   struct threaded_resource b;
   struct pb_buffer_lean *buffer;
   uint32_t offset;
   uint32_t stride;
};

struct si_surface {
   struct pipe_surface base;

   /* These can vary with block-compressed textures. */
   uint16_t width0;
   uint16_t height0;

   bool color_initialized : 1;
   bool depth_initialized : 1;

   /* Misc. color flags. */
   bool color_is_int8 : 1;
   bool color_is_int10 : 1;
   bool dcc_incompatible : 1;
   uint8_t db_format_index : 3;

   /* Color registers. */
   unsigned cb_color_info;
   unsigned cb_color_view;
   unsigned cb_color_attrib;
   unsigned cb_color_attrib2;                      /* GFX9 and later */
   unsigned cb_color_attrib3;                      /* GFX10 and later */
   unsigned cb_dcc_control;                        /* GFX8 and later */
   unsigned spi_shader_col_format : 8;             /* no blending, no alpha-to-coverage. */
   unsigned spi_shader_col_format_alpha : 8;       /* alpha-to-coverage */
   unsigned spi_shader_col_format_blend : 8;       /* blending without alpha. */
   unsigned spi_shader_col_format_blend_alpha : 8; /* blending with alpha. */

   /* DB registers. */
   uint64_t db_depth_base; /* DB_Z_READ/WRITE_BASE */
   uint64_t db_stencil_base;
   uint64_t db_htile_data_base;
   unsigned db_depth_info;
   unsigned db_z_info;
   unsigned db_z_info2; /* GFX9 only */
   unsigned db_depth_view;
   unsigned db_depth_size;
   unsigned db_depth_slice;
   unsigned db_stencil_info;
   unsigned db_stencil_info2; /* GFX9 only */
   unsigned db_htile_surface;
};

struct si_mmio_counter {
   unsigned busy;
   unsigned idle;
};

union si_mmio_counters {
   struct si_mmio_counters_named {
      /* For global GPU load including SDMA. */
      struct si_mmio_counter gpu;

      /* GRBM_STATUS */
      struct si_mmio_counter spi;
      struct si_mmio_counter gui;
      struct si_mmio_counter ta;
      struct si_mmio_counter gds;
      struct si_mmio_counter vgt;
      struct si_mmio_counter ia;
      struct si_mmio_counter sx;
      struct si_mmio_counter wd;
      struct si_mmio_counter bci;
      struct si_mmio_counter sc;
      struct si_mmio_counter pa;
      struct si_mmio_counter db;
      struct si_mmio_counter cp;
      struct si_mmio_counter cb;

      /* SRBM_STATUS2 */
      struct si_mmio_counter sdma;

      /* CP_STAT */
      struct si_mmio_counter pfp;
      struct si_mmio_counter meq;
      struct si_mmio_counter me;
      struct si_mmio_counter surf_sync;
      struct si_mmio_counter cp_dma;
      struct si_mmio_counter scratch_ram;
   } named;

   unsigned array[sizeof(struct si_mmio_counters_named) / sizeof(unsigned)];
};

struct si_memory_object {
   struct pipe_memory_object b;
   struct pb_buffer_lean *buf;
   uint32_t stride;
};

/* Saved CS data for debugging features. */
struct radeon_saved_cs {
   uint32_t *ib;
   unsigned num_dw;

   struct radeon_bo_list_item *bo_list;
   unsigned bo_count;
};

struct si_aux_context {
   struct pipe_context *ctx;
   mtx_t lock;
};

struct si_screen {
   struct pipe_screen b;
   struct radeon_winsys *ws;
   struct disk_cache *disk_shader_cache;

   struct radeon_info info;
   struct nir_shader_compiler_options *nir_options;
   uint64_t debug_flags;
   char renderer_string[183];

   void (*make_texture_descriptor)(struct si_screen *screen, struct si_texture *tex, bool sampler,
                                   enum pipe_texture_target target, enum pipe_format pipe_format,
                                   const unsigned char state_swizzle[4], unsigned first_level,
                                   unsigned last_level, unsigned first_layer, unsigned last_layer,
                                   unsigned width, unsigned height, unsigned depth,
                                   bool get_bo_metadata, uint32_t *state, uint32_t *fmask_state);

   unsigned pa_sc_raster_config;
   unsigned pa_sc_raster_config_1;
   unsigned se_tile_repeat;
   unsigned gs_table_depth;
   struct ac_hs_info hs;
   unsigned eqaa_force_coverage_samples;
   unsigned eqaa_force_z_samples;
   unsigned eqaa_force_color_samples;
   unsigned pbb_context_states_per_bin;
   unsigned pbb_persistent_states_per_bin;
   bool has_draw_indirect_multi;
   bool dpbb_allowed;
   bool use_ngg;
   bool use_ngg_culling;
   bool allow_dcc_msaa_clear_to_reg_for_bpp[5]; /* indexed by log2(Bpp) */
   bool always_allow_dcc_stores;
   bool use_aco;

   struct {
#define OPT_BOOL(name, dflt, description) bool name : 1;
#define OPT_INT(name, dflt, description) int name;
#include "si_debug_options.h"
   } options;

   /* Whether shaders are monolithic (1-part) or separate (3-part). */
   bool use_monolithic_shaders;
   bool record_llvm_ir;
   const char *context_roll_log_filename;

   struct slab_parent_pool pool_transfers;

   /* Texture filter settings. */
   int force_aniso; /* -1 = disabled */

   unsigned max_texel_buffer_elements;

   /* Auxiliary context. Used to initialize resources and upload shaders. */
   union {
      struct {
         struct si_aux_context general;

         /* Second auxiliary context for uploading shaders. When the first auxiliary context is
          * locked and wants to compile and upload shaders, we need to use a second auxiliary
          * context because the first one is locked.
          */
         struct si_aux_context shader_upload;
      } aux_context;
      struct si_aux_context aux_contexts[2];
   };

   /* Async compute context for DRI_PRIME copies. */
   struct pipe_context *async_compute_context;
   simple_mtx_t async_compute_context_lock;

   /* This must be in the screen, because UE4 uses one context for
    * compilation and another one for rendering.
    */
   unsigned num_compilations;
   /* Along with ST_DEBUG=precompile, this should show if applications
    * are loading shaders on demand. This is a monotonic counter.
    */
   unsigned num_shaders_created;
   unsigned num_memory_shader_cache_hits;
   unsigned num_memory_shader_cache_misses;
   unsigned num_disk_shader_cache_hits;
   unsigned num_disk_shader_cache_misses;

   /* GPU load thread. */
   simple_mtx_t gpu_load_mutex;
   thrd_t gpu_load_thread;
   bool gpu_load_thread_created;
   union si_mmio_counters mmio_counters;
   volatile unsigned gpu_load_stop_thread; /* bool */

   /* Performance counters. */
   struct si_perfcounters *perfcounters;

   /* If pipe_screen wants to recompute and re-emit the framebuffer,
    * sampler, and image states of all contexts, it should atomically
    * increment this.
    *
    * Each context will compare this with its own last known value of
    * the counter before drawing and re-emit the states accordingly.
    */
   unsigned dirty_tex_counter;
   unsigned dirty_buf_counter;

   /* Atomically increment this counter when an existing texture's
    * metadata is enabled or disabled in a way that requires changing
    * contexts' compressed texture binding masks.
    */
   unsigned compressed_colortex_counter;

   struct {
      /* Context flags to set so that all writes from earlier jobs
       * in the CP are seen by L2 clients.
       */
      unsigned cp_to_L2;

      /* Context flags to set so that all writes from earlier jobs
       * that end in L2 are seen by CP.
       */
      unsigned L2_to_cp;
   } barrier_flags;

   simple_mtx_t shader_parts_mutex;
   struct si_shader_part *vs_prologs;
   struct si_shader_part *tcs_epilogs;
   struct si_shader_part *ps_prologs;
   struct si_shader_part *ps_epilogs;

   /* Shader cache in memory.
    *
    * Design & limitations:
    * - The shader cache is per screen (= per process), never saved to
    *   disk, and skips redundant shader compilations from NIR to bytecode.
    * - It can only be used with one-variant-per-shader support, in which
    *   case only the main (typically middle) part of shaders is cached.
    * - Only VS, TCS, TES, PS are cached, out of which only the hw VS
    *   variants of VS and TES are cached, so LS and ES aren't.
    * - GS and CS aren't cached, but it's certainly possible to cache
    *   those as well.
    */
   simple_mtx_t shader_cache_mutex;
   struct hash_table *shader_cache;
   /* Maximum and current size */
   uint32_t shader_cache_size;
   uint32_t shader_cache_max_size;

   /* Shader cache of live shaders. */
   struct util_live_shader_cache live_shader_cache;

   /* Shader compiler queue for multithreaded compilation. */
   struct util_queue shader_compiler_queue;
   /* Compiler instances for asynchronous shader compilation of new shader CSOs,
    * one for each thread of the shader compiler queue.
    */
   struct ac_llvm_compiler *compiler[24]; /* used by the queue only */

   struct util_queue shader_compiler_queue_opt_variants;
   /* Compiler instances for asynchronous shader compilation of optimized shader variants,
    * one for each thread of the low-priority shader compiler queue. */
   struct ac_llvm_compiler *compiler_lowp[10];

   struct util_idalloc_mt buffer_ids;
   struct util_vertex_state_cache vertex_state_cache;

   struct si_resource *attribute_ring;

   /* NGG streamout. */
   simple_mtx_t gds_mutex;
   struct pb_buffer_lean *gds_oa;
};

struct si_compute {
   struct si_shader_selector sel;
   struct si_shader shader;

   unsigned ir_type;
   unsigned input_size;

   int max_global_buffers;
   struct pipe_resource **global_buffers;
};

struct si_sampler_view {
   struct pipe_sampler_view base;
   /* [0..7] = image descriptor
    * [4..7] = buffer descriptor */
   uint32_t state[8];
   uint32_t fmask_state[8];
   const struct legacy_surf_level *base_level_info;
   uint8_t block_width;
   bool is_stencil_sampler;
   bool dcc_incompatible;
};

#define SI_SAMPLER_STATE_MAGIC 0x34f1c35a

struct si_sampler_state {
#ifndef NDEBUG
   unsigned magic;
#endif
   uint32_t val[4];
   uint32_t upgraded_depth_val[4];
};

struct si_cs_shader_state {
   struct si_compute *program;
   struct si_compute *emitted_program;
   unsigned offset;
   uint32_t variable_shared_size;
};

struct si_samplers {
   struct pipe_sampler_view *views[SI_NUM_SAMPLERS];
   struct si_sampler_state *sampler_states[SI_NUM_SAMPLERS];

   /* The i-th bit is set if that element is enabled (non-NULL resource). */
   unsigned enabled_mask;
   uint32_t has_depth_tex_mask;
   uint32_t needs_depth_decompress_mask;
   uint32_t needs_color_decompress_mask;
};

struct si_images {
   struct pipe_image_view views[SI_NUM_IMAGES];
   uint32_t needs_color_decompress_mask;
   unsigned enabled_mask;
   unsigned display_dcc_store_mask;
};

struct si_framebuffer {
   struct pipe_framebuffer_state state;
   unsigned colorbuf_enabled_4bit;
   unsigned spi_shader_col_format;
   unsigned spi_shader_col_format_alpha;
   unsigned spi_shader_col_format_blend;
   unsigned spi_shader_col_format_blend_alpha;
   uint8_t nr_samples : 5;   /* at most 16xAA */
   uint8_t log_samples : 3;  /* at most 4 = 16xAA */
   uint8_t nr_color_samples; /* at most 8xAA */
   uint8_t compressed_cb_mask;
   uint8_t uncompressed_cb_mask;
   uint8_t color_is_int8;
   uint8_t color_is_int10;
   uint8_t dirty_cbufs;
   uint8_t min_bytes_per_pixel;
   bool dirty_zsbuf;
   bool any_dst_linear;
   bool CB_has_shader_readable_metadata;
   bool DB_has_shader_readable_metadata;
   bool all_DCC_pipe_aligned;
   bool has_dcc_msaa;
};

enum si_quant_mode
{
   /* This is the list we want to support. */
   SI_QUANT_MODE_16_8_FIXED_POINT_1_256TH,
   SI_QUANT_MODE_14_10_FIXED_POINT_1_1024TH,
   SI_QUANT_MODE_12_12_FIXED_POINT_1_4096TH,
};

struct si_signed_scissor {
   int minx;
   int miny;
   int maxx;
   int maxy;
   enum si_quant_mode quant_mode;
};

struct si_viewports {
   struct pipe_viewport_state states[SI_MAX_VIEWPORTS];
   struct si_signed_scissor as_scissor[SI_MAX_VIEWPORTS];
};

struct si_streamout_target {
   struct pipe_stream_output_target b;

   /* The buffer where BUFFER_FILLED_SIZE is stored. */
   struct si_resource *buf_filled_size;
   unsigned buf_filled_size_offset;
   bool buf_filled_size_valid;

   unsigned stride_in_dw;
};

struct si_streamout {
   bool begin_emitted;

   unsigned enabled_mask;
   unsigned num_targets;
   struct si_streamout_target *targets[PIPE_MAX_SO_BUFFERS];

   unsigned append_bitmask;
   bool suspended;

   /* External state which comes from the vertex shader,
    * it must be set explicitly when binding a shader. */
   uint8_t *stride_in_dw;
   unsigned enabled_stream_buffers_mask; /* stream0 buffers0-3 in 4 LSB */

   /* The state of VGT_STRMOUT_BUFFER_(CONFIG|EN). */
   unsigned hw_enabled_mask;

   /* The state of VGT_STRMOUT_(CONFIG|EN). */
   bool streamout_enabled;
   bool prims_gen_query_enabled;
   int num_prims_gen_queries;
};

/* A shader state consists of the shader selector, which is a constant state
 * object shared by multiple contexts and shouldn't be modified, and
 * the current shader variant selected for this context.
 */
struct si_shader_ctx_state {
   struct si_shader_selector *cso;
   struct si_shader *current;
   /* The shader variant key representing the current state. */
   union si_shader_key key;
};

#define SI_NUM_VGT_PARAM_KEY_BITS 12
#define SI_NUM_VGT_PARAM_STATES   (1 << SI_NUM_VGT_PARAM_KEY_BITS)

/* The IA_MULTI_VGT_PARAM key used to index the table of precomputed values.
 * Some fields are set by state-change calls, most are set by draw_vbo.
 */
union si_vgt_param_key {
   struct {
#if UTIL_ARCH_LITTLE_ENDIAN
      uint16_t prim : 4;
      uint16_t uses_instancing : 1;
      uint16_t multi_instances_smaller_than_primgroup : 1;
      uint16_t primitive_restart : 1;
      uint16_t count_from_stream_output : 1;
      uint16_t line_stipple_enabled : 1;
      uint16_t uses_tess : 1;
      uint16_t tess_uses_prim_id : 1;
      uint16_t uses_gs : 1;
      uint16_t _pad : 16 - SI_NUM_VGT_PARAM_KEY_BITS;
#else /* UTIL_ARCH_BIG_ENDIAN */
      uint16_t _pad : 16 - SI_NUM_VGT_PARAM_KEY_BITS;
      uint16_t uses_gs : 1;
      uint16_t tess_uses_prim_id : 1;
      uint16_t uses_tess : 1;
      uint16_t line_stipple_enabled : 1;
      uint16_t count_from_stream_output : 1;
      uint16_t primitive_restart : 1;
      uint16_t multi_instances_smaller_than_primgroup : 1;
      uint16_t uses_instancing : 1;
      uint16_t prim : 4;
#endif
   } u;
   uint16_t index;
};

struct si_texture_handle {
   unsigned desc_slot;
   bool desc_dirty;
   struct pipe_sampler_view *view;
   struct si_sampler_state sstate;
};

struct si_image_handle {
   unsigned desc_slot;
   bool desc_dirty;
   struct pipe_image_view view;
};

struct si_saved_cs {
   struct pipe_reference reference;
   struct si_context *ctx;
   struct radeon_saved_cs gfx;
   struct radeon_saved_cs compute;
   struct si_resource *trace_buf;
   unsigned trace_id;

   unsigned gfx_last_dw;
   bool flushed;
   int64_t time_flush;
};

struct si_sqtt_fake_pipeline {
   struct si_pm4_state pm4; /* base class */
   uint64_t code_hash;
   struct si_resource *bo;
   uint32_t offset[SI_NUM_GRAPHICS_SHADERS];
};

struct si_small_prim_cull_info {
   float scale[2], translate[2];
   float scale_no_aa[2], translate_no_aa[2];
   float clip_half_line_width[2];      /* line_width * 0.5 in clip space in X and Y directions */
};

struct si_vertex_state {
   struct pipe_vertex_state b;
   struct si_vertex_elements velems;
   uint32_t descriptors[4 * SI_MAX_ATTRIBS];
};

/* The structure layout is identical to a pair of registers in SET_*_REG_PAIRS_PACKED. */
struct gfx11_reg_pair {
   union {
      /* A pair of register offsets. */
      struct {
         uint16_t reg_offset[2];
      };
      /* The same pair of register offsets as a dword. */
      uint32_t reg_offsets;
   };
   /* A pair of register values for the register offsets above. */
   uint32_t reg_value[2];
};

typedef void (*pipe_draw_vertex_state_func)(struct pipe_context *ctx,
                                            struct pipe_vertex_state *vstate,
                                            uint32_t partial_velem_mask,
                                            struct pipe_draw_vertex_state_info info,
                                            const struct pipe_draw_start_count_bias *draws,
                                            unsigned num_draws);

struct si_context {
   struct pipe_context b; /* base class */

   enum radeon_family family;
   enum amd_gfx_level gfx_level;

   struct radeon_winsys *ws;
   struct radeon_winsys_ctx *ctx;
   struct radeon_cmdbuf gfx_cs; /* compute IB if graphics is disabled */
   struct radeon_cmdbuf *sdma_cs;
   struct pipe_fence_handle *last_gfx_fence;
   struct si_resource *eop_bug_scratch;
   struct si_resource *eop_bug_scratch_tmz;
   struct u_upload_mgr *cached_gtt_allocator;
   struct threaded_context *tc;
   struct u_suballocator allocator_zeroed_memory;
   struct slab_child_pool pool_transfers;
   struct slab_child_pool pool_transfers_unsync; /* for threaded_context */
   struct pipe_device_reset_callback device_reset_callback;
   struct u_log_context *log;
   void *query_result_shader;
   void *sh_query_result_shader;
   struct {
      /* Memory where the shadowed registers will be saved and loaded from. */
      struct si_resource *registers;
      /* Context Save Area: scratch area to save other required data. Only
       * used if info->has_fw_based_mcbp is true.
       */
      struct si_resource *csa;
   } shadowing;

   void (*emit_cache_flush)(struct si_context *ctx, struct radeon_cmdbuf *cs);

   struct blitter_context *blitter;
   void *noop_blend;
   void *noop_dsa;
   void *no_velems_state;
   void *discard_rasterizer_state;
   void *custom_dsa_flush;
   void *custom_blend_resolve;
   void *custom_blend_fmask_decompress;
   void *custom_blend_eliminate_fastclear;
   void *custom_blend_dcc_decompress;
   void *vs_blit_pos;
   void *vs_blit_pos_layered;
   void *vs_blit_color;
   void *vs_blit_color_layered;
   void *vs_blit_texcoord;
   void *cs_clear_buffer;
   void *cs_clear_buffer_rmw;
   void *cs_copy_buffer;
   void *cs_ubyte_to_ushort;
   void *cs_copy_image[3][2][2]; /* [wg_dim-1][src_is_1d][dst_is_1d] */
   void *cs_clear_render_target;
   void *cs_clear_render_target_1d_array;
   void *cs_clear_12bytes_buffer;
   void *cs_dcc_retile[32];
   void *cs_fmask_expand[3][2]; /* [log2(samples)-1][is_array] */
   struct hash_table *cs_blit_shaders;
   struct si_screen *screen;
   struct util_debug_callback debug;
   struct ac_llvm_compiler *compiler; /* only non-threaded compilation */
   struct hash_table *fixed_func_tcs_shader_cache;
   struct si_resource *wait_mem_scratch;
   struct si_resource *wait_mem_scratch_tmz;
   unsigned wait_mem_number;
   uint16_t prefetch_L2_mask;

   bool blitter_running:1;
   bool suppress_update_ps_colorbuf0_slot:1;
   bool is_noop:1;
   bool has_graphics:1;
   bool gfx_flush_in_progress : 1;
   bool gfx_last_ib_is_busy : 1;
   bool compute_is_busy : 1;
   bool gfx11_force_msaa_num_samples_zero:1;
   int8_t pipeline_stats_enabled; /* -1 = unknown, 0 = disabled, 1 = enabled */

   unsigned num_gfx_cs_flushes;
   unsigned initial_gfx_cs_size;
   unsigned last_dirty_tex_counter;
   unsigned last_dirty_buf_counter;
   unsigned last_compressed_colortex_counter;
   unsigned last_num_draw_calls;
   unsigned flags; /* flush flags */

   /* Atoms (state emit functions). */
   union si_state_atoms atoms;
   uint64_t dirty_atoms; /* mask */
   union si_state queued;
   union si_state emitted;

   /* Gfx11+: Buffered SH registers for SET_SH_REG_PAIRS_*. */
   unsigned num_buffered_gfx_sh_regs;
   unsigned num_buffered_compute_sh_regs;
   struct {
      struct gfx11_reg_pair buffered_gfx_sh_regs[32];
      struct gfx11_reg_pair buffered_compute_sh_regs[32];
   } gfx11;

   /* Atom declarations. */
   struct si_framebuffer framebuffer;
   unsigned sample_locs_num_samples;
   uint16_t sample_mask;
   unsigned last_cb_target_mask;
   struct pipe_blend_color blend_color;
   struct pipe_clip_state clip_state;
   struct si_shader_data shader_pointers;
   struct si_stencil_ref stencil_ref;
   bool blend_color_any_nonzeros:1;
   bool clip_state_any_nonzeros:1;
   bool viewport0_y_inverted;
   struct pipe_scissor_state scissors[SI_MAX_VIEWPORTS];
   struct si_streamout streamout;
   struct si_viewports viewports;
   unsigned num_window_rectangles;
   bool window_rectangles_include;
   struct pipe_scissor_state window_rectangles[4];

   /* Precomputed states. */
   struct si_pm4_state *cs_preamble_state;
   struct si_pm4_state *cs_preamble_state_tmz;
   uint16_t gs_ring_state_dw_offset;
   uint16_t gs_ring_state_dw_offset_tmz;
   bool cs_preamble_has_vgt_flush;
   bool cs_preamble_has_vgt_flush_tmz;
   uint32_t vgt_shader_stages_en;
   uint32_t ge_cntl;

   /* shaders */
   union {
      struct {
         struct si_shader_ctx_state vs;
         struct si_shader_ctx_state tcs;
         struct si_shader_ctx_state tes;
         struct si_shader_ctx_state gs;
         struct si_shader_ctx_state ps;
      } shader;
      /* indexed access using pipe_shader_type (not by MESA_SHADER_*) */
      struct si_shader_ctx_state shaders[SI_NUM_GRAPHICS_SHADERS];
   };
   struct si_cs_shader_state cs_shader_state;
   /* if current tcs set by user */
   bool is_user_tcs;

   /* video context */
   bool vcn_has_ctx;
   enum vcn_version vcn_ip_ver;

   /* shader information */
   uint64_t ps_inputs_read_or_disabled;
   struct si_vertex_elements *vertex_elements;
   unsigned num_vertex_elements;
   unsigned cs_max_waves_per_sh;
   bool uses_nontrivial_vs_prolog;
   bool force_trivial_vs_prolog;
   bool do_update_shaders;
   bool compute_shaderbuf_sgprs_dirty;
   bool compute_image_sgprs_dirty;
   bool vs_uses_base_instance;
   bool vs_uses_draw_id;
   uint8_t patch_vertices;

   /* shader descriptors */
   struct si_descriptors descriptors[SI_NUM_DESCS];
   unsigned descriptors_dirty;
   unsigned shader_pointers_dirty;
   unsigned shader_needs_decompress_mask;
   unsigned shader_has_depth_tex;
   struct si_buffer_resources internal_bindings;
   struct si_buffer_resources const_and_shader_buffers[SI_NUM_SHADERS];
   struct si_samplers samplers[SI_NUM_SHADERS];
   struct si_images images[SI_NUM_SHADERS];
   bool bo_list_add_all_resident_resources;
   bool bo_list_add_all_compute_resources;

   /* other shader resources */
   struct pipe_constant_buffer null_const_buf; /* used for set_constant_buffer(NULL) on GFX7 */
   struct pipe_resource *esgs_ring;
   struct pipe_resource *gsvs_ring;
   struct pipe_resource *tess_rings;
   struct pipe_resource *tess_rings_tmz;
   union pipe_color_union *border_color_table; /* in CPU memory, any endian */
   struct si_resource *border_color_buffer;
   union pipe_color_union *border_color_map; /* in VRAM (slow access), little endian */
   unsigned border_color_count;
   unsigned num_vs_blit_sgprs;
   uint32_t vs_blit_sh_data[MAX_SI_VS_BLIT_SGPRS];
   uint32_t cs_user_data[4];

   /* Vertex buffers. */
   bool vertex_buffers_dirty;
   uint16_t vertex_buffer_unaligned; /* bitmask of not dword-aligned buffers */
   struct pipe_vertex_buffer vertex_buffer[SI_NUM_VERTEX_BUFFERS];

   /* Even though we don't need this variable, u_upload_alloc has an optimization that skips
    * reference counting when the new upload buffer is the same as the last one. So keep
    * the last upload buffer here and always pass &last_const_upload_buffer to u_upload_alloc.
    */
   struct si_resource *last_const_upload_buffer;

   /* MSAA config state. */
   int ps_iter_samples;
   bool ps_uses_fbfetch;
   bool smoothing_enabled;

   /* point smoothing state.*/
   bool point_smoothing_enabled;

   /* DB render state. */
   unsigned ps_db_shader_control;
   unsigned dbcb_copy_sample;
   bool dbcb_depth_copy_enabled : 1;
   bool dbcb_stencil_copy_enabled : 1;
   bool db_flush_depth_inplace : 1;
   bool db_flush_stencil_inplace : 1;
   bool db_depth_clear : 1;
   bool db_depth_disable_expclear : 1;
   bool db_stencil_clear : 1;
   bool db_stencil_disable_expclear : 1;
   bool occlusion_queries_disabled : 1;
   bool generate_mipmap_for_depth : 1;
   bool allow_flat_shading : 1;

   /* Emitted draw state. */
   bool ngg : 1;
   bool disable_instance_packing : 1;
   uint16_t ngg_culling;
   unsigned last_index_size;
   unsigned last_instance_count;
   int last_primitive_restart_en;
   unsigned last_restart_index;
   unsigned last_prim;
   unsigned current_vs_state; /* all VS bits including LS bits */
   unsigned current_gs_state; /* only GS and NGG bits */
   unsigned last_vs_state;
   unsigned last_gs_state;
   enum mesa_prim current_rast_prim; /* primitive type after TES, GS */
   unsigned gs_out_prim;

   struct si_small_prim_cull_info last_small_prim_cull_info;
   struct si_resource *small_prim_cull_info_buf;
   uint64_t small_prim_cull_info_address;

   /* Scratch buffer */
   struct si_resource *scratch_buffer;
   unsigned spi_tmpring_size;
   unsigned max_seen_scratch_bytes_per_wave;
   unsigned max_seen_compute_scratch_bytes_per_wave;

   struct si_resource *compute_scratch_buffer;

   /* Emitted derived tessellation state. */
   /* Local shader (VS), or HS if LS-HS are merged. */
   struct si_shader *last_ls;
   struct si_shader_selector *last_tcs;
   unsigned last_num_tcs_input_cp;
   unsigned last_tes_sh_base;
   bool last_tess_uses_primid;
   unsigned num_patches_per_workgroup;
   unsigned tcs_offchip_layout;
   unsigned tes_offchip_ring_va_sgpr;
   unsigned ls_hs_rsrc2;
   unsigned ls_hs_config;

   /* Debug state. */
   bool is_debug;
   struct si_saved_cs *current_saved_cs;
   uint64_t dmesg_timestamp;
   unsigned apitrace_call_number;

   /* Other state */
   bool need_check_render_feedback;
   bool decompression_enabled;
   bool dpbb_force_off;
   bool dpbb_force_off_profile_vs;
   bool dpbb_force_off_profile_ps;
   bool vs_writes_viewport_index;
   bool vs_disables_clipping_viewport;
   bool has_reset_been_notified;

   /* The number of pixels outside the viewport that are not culled by the clipper.
    * Normally, the clipper clips everything outside the viewport, however, points and lines
    * can have vertices outside the viewport, but their edges can be inside the viewport. Those
    * shouldn't be culled. The problem is that the register setting (PA_CL_GB_*_DISC_ADJ) that
    * controls the discard distance, which depends on the point size and line width, applies to
    * all primitive types, and we would have to set 0 distance for triangles and non-zero for
    * points and lines whenever the primitive type changes, which would add overhead and cause
    * context rolls.
    *
    * To reduce that, whenever the discard distance changes for points and lines, we keep it
    * at that higher value up to a certain small number for all primitive types including all
    * points and lines within a specific size. This is slightly inefficient, but it eliminates
    * a lot of guardband state updates and context register changes.
    */
   float min_clip_discard_distance_watermark;
   float current_clip_discard_distance;

   /* Precomputed IA_MULTI_VGT_PARAM */
   union si_vgt_param_key ia_multi_vgt_param_key;
   unsigned ia_multi_vgt_param[SI_NUM_VGT_PARAM_STATES];

   /* Bindless descriptors. */
   struct si_descriptors bindless_descriptors;
   struct util_idalloc bindless_used_slots;
   unsigned num_bindless_descriptors;
   bool bindless_descriptors_dirty;
   bool graphics_bindless_pointer_dirty;
   bool compute_bindless_pointer_dirty;
   bool gs_attribute_ring_pointer_dirty;

   /* Allocated bindless handles */
   struct hash_table *tex_handles;
   struct hash_table *img_handles;

   /* Resident bindless handles */
   struct util_dynarray resident_tex_handles;
   struct util_dynarray resident_img_handles;

   /* Resident bindless handles which need decompression */
   struct util_dynarray resident_tex_needs_color_decompress;
   struct util_dynarray resident_img_needs_color_decompress;
   struct util_dynarray resident_tex_needs_depth_decompress;

   /* Bindless state */
   bool uses_bindless_samplers;
   bool uses_bindless_images;

   /* MSAA sample locations.
    * The first index is the sample index.
    * The second index is the coordinate: X, Y. */
   struct {
      float x1[1][2];
      float x2[2][2];
      float x4[4][2];
      float x8[8][2];
      float x16[16][2];
   } sample_positions;
   struct pipe_resource *sample_pos_buffer;

   /* Misc stats. */
   unsigned num_draw_calls;
   unsigned num_decompress_calls;
   unsigned num_compute_calls;
   unsigned num_cp_dma_calls;
   unsigned num_vs_flushes;
   unsigned num_ps_flushes;
   unsigned num_cs_flushes;
   unsigned num_cb_cache_flushes;
   unsigned num_db_cache_flushes;
   unsigned num_L2_invalidates;
   unsigned num_L2_writebacks;
   unsigned num_resident_handles;
   uint64_t num_alloc_tex_transfer_bytes;
   unsigned last_tex_ps_draw_ratio; /* for query */
   unsigned context_roll;

   /* Queries. */
   /* Maintain the list of active queries for pausing between IBs. */
   enum si_occlusion_query_mode occlusion_query_mode;
   int num_integer_occlusion_queries;
   int num_boolean_occlusion_queries;
   int num_conservative_occlusion_queries;
   int num_pipeline_stat_queries;
   int num_pipeline_stat_emulated_queries;
   int num_hw_pipestat_streamout_queries;
   struct list_head active_queries;
   unsigned num_cs_dw_queries_suspend;
   /* Shared buffer for pipeline stats queries implemented with an atomic op */
   struct si_resource *pipeline_stats_query_buf;

   /* Render condition. */
   struct pipe_query *render_cond;
   unsigned render_cond_mode;
   bool render_cond_invert;
   bool render_cond_enabled; /* for u_blitter */

   /* Shader-based queries. */
   struct list_head shader_query_buffers;
   unsigned num_active_shader_queries;

   bool force_cb_shader_coherent;

   struct si_tracked_regs tracked_regs;

   /* Resources that need to be flushed, but will not get an explicit
    * flush_resource from the frontend and that will need to get flushed during
    * a context flush.
    */
   struct hash_table *dirty_implicit_resources;

   pipe_draw_func draw_vbo[2][2][2];
   pipe_draw_vertex_state_func draw_vertex_state[2][2][2];
   /* When b.draw_vbo is a wrapper, real_draw_vbo is the real draw_vbo function */
   pipe_draw_func real_draw_vbo;
   pipe_draw_vertex_state_func real_draw_vertex_state;
   void (*emit_spi_map[33])(struct si_context *sctx, unsigned index);

   /* SQTT */
   struct ac_sqtt *sqtt;
   struct ac_spm spm;
   struct pipe_fence_handle *last_sqtt_fence;
   enum rgp_sqtt_marker_event_type sqtt_next_event;
   bool sqtt_enabled;

   unsigned context_flags;

   /* Shaders. */
   /* TODO: move other shaders here too */
   /* Only used for DCC MSAA clears with 4-8 fragments and 4-16 samples. */
   void *cs_clear_dcc_msaa[32][5][2][3][2]; /* [swizzle_mode][log2(bpe)][fragments == 8][log2(samples)-2][is_array] */
   
   /* u_trace logging*/
   struct si_ds_device ds;
   /** Where tracepoints are recorded */
   struct u_trace trace;
   struct si_ds_queue ds_queue;
   uint32_t *last_timestamp_cmd;
   unsigned int last_timestamp_cmd_cdw;
};

/* si_blit.c */
enum si_blitter_op /* bitmask */
{
   SI_SAVE_TEXTURES = 1,
   SI_SAVE_FRAMEBUFFER = 2,
   SI_SAVE_FRAGMENT_STATE = 4,
   SI_SAVE_FRAGMENT_CONSTANT = 8,
   SI_DISABLE_RENDER_COND = 16,
};

void si_blitter_begin(struct si_context *sctx, enum si_blitter_op op);
void si_blitter_end(struct si_context *sctx);
void si_init_blit_functions(struct si_context *sctx);
void gfx6_decompress_textures(struct si_context *sctx, unsigned shader_mask);
void gfx11_decompress_textures(struct si_context *sctx, unsigned shader_mask);
void si_decompress_subresource(struct pipe_context *ctx, struct pipe_resource *tex, unsigned planes,
                               unsigned level, unsigned first_layer, unsigned last_layer,
                               bool need_fmask_expand);
void si_resource_copy_region(struct pipe_context *ctx, struct pipe_resource *dst,
                             unsigned dst_level, unsigned dstx, unsigned dsty, unsigned dstz,
                             struct pipe_resource *src, unsigned src_level,
                             const struct pipe_box *src_box);
void si_decompress_dcc(struct si_context *sctx, struct si_texture *tex);
void si_flush_implicit_resources(struct si_context *sctx);
bool si_msaa_resolve_blit_via_CB(struct pipe_context *ctx, const struct pipe_blit_info *info);
void si_gfx_blit(struct pipe_context *ctx, const struct pipe_blit_info *info);

/* si_nir_optim.c */
bool si_nir_is_output_const_if_tex_is_const(struct nir_shader *shader, float *in, float *out, int *texunit);

/* si_buffer.c */
bool si_cs_is_buffer_referenced(struct si_context *sctx, struct pb_buffer_lean *buf,
                                unsigned usage);
void *si_buffer_map(struct si_context *sctx, struct si_resource *resource,
                    unsigned usage);
void si_init_resource_fields(struct si_screen *sscreen, struct si_resource *res, uint64_t size,
                             unsigned alignment);
bool si_alloc_resource(struct si_screen *sscreen, struct si_resource *res);
struct pipe_resource *pipe_aligned_buffer_create(struct pipe_screen *screen, unsigned flags,
                                                 unsigned usage, unsigned size, unsigned alignment);
struct si_resource *si_aligned_buffer_create(struct pipe_screen *screen, unsigned flags,
                                             unsigned usage, unsigned size, unsigned alignment);
struct pipe_resource *si_buffer_from_winsys_buffer(struct pipe_screen *screen,
                                                   const struct pipe_resource *templ,
                                                   struct pb_buffer_lean *imported_buf,
                                                   uint64_t offset);
void si_replace_buffer_storage(struct pipe_context *ctx, struct pipe_resource *dst,
                               struct pipe_resource *src, unsigned num_rebinds,
                               uint32_t rebind_mask, uint32_t delete_buffer_id);
void si_init_screen_buffer_functions(struct si_screen *sscreen);
void si_init_buffer_functions(struct si_context *sctx);

/* si_clear.c */
#define SI_CLEAR_TYPE_CMASK  (1 << 0)
#define SI_CLEAR_TYPE_DCC    (1 << 1)
#define SI_CLEAR_TYPE_HTILE  (1 << 2)

struct si_clear_info {
   struct pipe_resource *resource;
   uint64_t offset;
   uint32_t size;
   uint32_t clear_value;
   uint32_t writemask;
   bool is_dcc_msaa; /* Clear it as a DCC MSAA image. */
};

enum pipe_format si_simplify_cb_format(enum pipe_format format);
bool vi_alpha_is_on_msb(struct si_screen *sscreen, enum pipe_format format);
bool vi_dcc_get_clear_info(struct si_context *sctx, struct si_texture *tex, unsigned level,
                           unsigned clear_value, struct si_clear_info *out);
void si_init_buffer_clear(struct si_clear_info *info,
                          struct pipe_resource *resource, uint64_t offset,
                          uint32_t size, uint32_t clear_value);
void si_execute_clears(struct si_context *sctx, struct si_clear_info *info,
                       unsigned num_clears, unsigned types);
void si_init_clear_functions(struct si_context *sctx);

/* si_compute.c */
void si_destroy_compute(struct si_compute *program);

/* si_compute_blit.c */
#define SI_OP_SYNC_CS_BEFORE              (1 << 0)
#define SI_OP_SYNC_PS_BEFORE              (1 << 1)
#define SI_OP_SYNC_CPDMA_BEFORE           (1 << 2) /* only affects CP DMA calls */
#define SI_OP_SYNC_BEFORE                 (SI_OP_SYNC_CS_BEFORE | SI_OP_SYNC_PS_BEFORE | SI_OP_SYNC_CPDMA_BEFORE)
#define SI_OP_SYNC_AFTER                  (1 << 3)
#define SI_OP_SYNC_BEFORE_AFTER           (SI_OP_SYNC_BEFORE | SI_OP_SYNC_AFTER)
#define SI_OP_SKIP_CACHE_INV_BEFORE       (1 << 4) /* don't invalidate caches */
#define SI_OP_CS_IMAGE                    (1 << 5)
#define SI_OP_CS_RENDER_COND_ENABLE       (1 << 6)
#define SI_OP_CPDMA_SKIP_CHECK_CS_SPACE   (1 << 7) /* don't call need_cs_space */
#define SI_OP_SYNC_GE_BEFORE              (1 << 8) /* only sync VS, TCS, TES, GS */

unsigned si_get_flush_flags(struct si_context *sctx, enum si_coherency coher,
                            enum si_cache_policy cache_policy);
void si_launch_grid_internal_ssbos(struct si_context *sctx, struct pipe_grid_info *info,
                                   void *shader, unsigned flags, enum si_coherency coher,
                                   unsigned num_buffers, const struct pipe_shader_buffer *buffers,
                                   unsigned writeable_bitmask);
enum si_clear_method {
  SI_CP_DMA_CLEAR_METHOD,
  SI_COMPUTE_CLEAR_METHOD,
  SI_AUTO_SELECT_CLEAR_METHOD
};
void si_clear_buffer(struct si_context *sctx, struct pipe_resource *dst,
                     uint64_t offset, uint64_t size, uint32_t *clear_value,
                     uint32_t clear_value_size, unsigned flags,
                     enum si_coherency coher, enum si_clear_method method);
void si_compute_clear_buffer_rmw(struct si_context *sctx, struct pipe_resource *dst,
                                 unsigned dst_offset, unsigned size,
                                 uint32_t clear_value, uint32_t writebitmask,
                                 unsigned flags, enum si_coherency coher);
void si_copy_buffer(struct si_context *sctx, struct pipe_resource *dst, struct pipe_resource *src,
                    uint64_t dst_offset, uint64_t src_offset, unsigned size, unsigned flags);
void si_compute_shorten_ubyte_buffer(struct si_context *sctx, struct pipe_resource *dst, struct pipe_resource *src,
                                     uint64_t dst_offset, uint64_t src_offset, unsigned size, unsigned flags);
bool si_compute_copy_image(struct si_context *sctx, struct pipe_resource *dst, unsigned dst_level,
                           struct pipe_resource *src, unsigned src_level, unsigned dstx,
                           unsigned dsty, unsigned dstz, const struct pipe_box *src_box,
                           unsigned flags);
void si_compute_clear_render_target(struct pipe_context *ctx, struct pipe_surface *dstsurf,
                                    const union pipe_color_union *color, unsigned dstx,
                                    unsigned dsty, unsigned width, unsigned height,
                                    bool render_condition_enabled);
void si_retile_dcc(struct si_context *sctx, struct si_texture *tex);
void gfx9_clear_dcc_msaa(struct si_context *sctx, struct pipe_resource *res, uint32_t clear_value,
                         unsigned flags, enum si_coherency coher);
void si_compute_expand_fmask(struct pipe_context *ctx, struct pipe_resource *tex);
bool si_compute_blit(struct si_context *sctx, const struct pipe_blit_info *info, bool testing);
void si_init_compute_blit_functions(struct si_context *sctx);

/* si_cp_dma.c */
void si_cp_dma_wait_for_idle(struct si_context *sctx, struct radeon_cmdbuf *cs);
void si_cp_dma_clear_buffer(struct si_context *sctx, struct radeon_cmdbuf *cs,
                            struct pipe_resource *dst, uint64_t offset, uint64_t size,
                            unsigned value, unsigned user_flags, enum si_coherency coher,
                            enum si_cache_policy cache_policy);
void si_cp_dma_copy_buffer(struct si_context *sctx, struct pipe_resource *dst,
                           struct pipe_resource *src, uint64_t dst_offset, uint64_t src_offset,
                           unsigned size, unsigned user_flags, enum si_coherency coher,
                           enum si_cache_policy cache_policy);
void si_test_gds(struct si_context *sctx);
void si_cp_write_data(struct si_context *sctx, struct si_resource *buf, unsigned offset,
                      unsigned size, unsigned dst_sel, unsigned engine, const void *data);
void si_cp_copy_data(struct si_context *sctx, struct radeon_cmdbuf *cs, unsigned dst_sel,
                     struct si_resource *dst, unsigned dst_offset, unsigned src_sel,
                     struct si_resource *src, unsigned src_offset);

/* si_cp_reg_shadowing.c */
void si_init_cp_reg_shadowing(struct si_context *sctx);

/* si_debug.c */
void si_gather_context_rolls(struct si_context *sctx);
void si_save_cs(struct radeon_winsys *ws, struct radeon_cmdbuf *cs, struct radeon_saved_cs *saved,
                bool get_buffer_list);
void si_clear_saved_cs(struct radeon_saved_cs *saved);
void si_destroy_saved_cs(struct si_saved_cs *scs);
void si_auto_log_cs(void *data, struct u_log_context *log);
void si_log_hw_flush(struct si_context *sctx);
void si_log_draw_state(struct si_context *sctx, struct u_log_context *log);
void si_log_compute_state(struct si_context *sctx, struct u_log_context *log);
void si_init_debug_functions(struct si_context *sctx);
void si_check_vm_faults(struct si_context *sctx, struct radeon_saved_cs *saved,
                        enum amd_ip_type ring);
bool si_replace_shader(unsigned num, struct si_shader_binary *binary);
void si_print_current_ib(struct si_context *sctx, FILE *f);

/* si_fence.c */
void si_cp_release_mem(struct si_context *ctx, struct radeon_cmdbuf *cs, unsigned event,
                       unsigned event_flags, unsigned dst_sel, unsigned int_sel, unsigned data_sel,
                       struct si_resource *buf, uint64_t va, uint32_t new_fence,
                       unsigned query_type);
unsigned si_cp_write_fence_dwords(struct si_screen *screen);
void si_cp_wait_mem(struct si_context *ctx, struct radeon_cmdbuf *cs, uint64_t va, uint32_t ref,
                    uint32_t mask, unsigned flags);
void si_init_fence_functions(struct si_context *ctx);
void si_init_screen_fence_functions(struct si_screen *screen);
struct pipe_fence_handle *si_create_fence(struct pipe_context *ctx,
                                          struct tc_unflushed_batch_token *tc_token);

/* si_get.c */
void si_init_screen_get_functions(struct si_screen *sscreen);

bool si_sdma_copy_image(struct si_context *ctx, struct si_texture *dst, struct si_texture *src);

/* si_gfx_cs.c */
void si_flush_gfx_cs(struct si_context *ctx, unsigned flags, struct pipe_fence_handle **fence);
void si_allocate_gds(struct si_context *ctx);
void si_set_tracked_regs_to_clear_state(struct si_context *ctx);
void si_begin_new_gfx_cs(struct si_context *ctx, bool first_cs);
void si_trace_emit(struct si_context *sctx);
void si_emit_ts(struct si_context *sctx, struct si_resource* buffer, unsigned int offset);
void si_emit_surface_sync(struct si_context *sctx, struct radeon_cmdbuf *cs,
                          unsigned cp_coher_cntl);
void gfx10_emit_cache_flush(struct si_context *sctx, struct radeon_cmdbuf *cs);
void gfx6_emit_cache_flush(struct si_context *sctx, struct radeon_cmdbuf *cs);
/* Replace the sctx->b.draw_vbo function with a wrapper. This can be use to implement
 * optimizations without affecting the normal draw_vbo functions perf.
 */
void si_install_draw_wrapper(struct si_context *sctx, pipe_draw_func wrapper,
                             pipe_draw_vertex_state_func vstate_wrapper);

/* si_gpu_load.c */
void si_gpu_load_kill_thread(struct si_screen *sscreen);
uint64_t si_begin_counter(struct si_screen *sscreen, unsigned type);
unsigned si_end_counter(struct si_screen *sscreen, unsigned type, uint64_t begin);

/* si_compute.c */
void si_emit_initial_compute_regs(struct si_context *sctx, struct radeon_cmdbuf *cs);
void si_init_compute_functions(struct si_context *sctx);

/* si_pipe.c */
struct ac_llvm_compiler *si_create_llvm_compiler(struct si_screen *sscreen);
void si_init_aux_async_compute_ctx(struct si_screen *sscreen);
struct si_context *si_get_aux_context(struct si_aux_context *ctx);
void si_put_aux_context_flush(struct si_aux_context *ctx);
void si_put_aux_shader_upload_context_flush(struct si_screen *sscreen);

/* si_perfcounters.c */
void si_init_perfcounters(struct si_screen *screen);
void si_destroy_perfcounters(struct si_screen *screen);
void si_inhibit_clockgating(struct si_context *sctx, struct radeon_cmdbuf *cs, bool inhibit);
void si_pc_emit_shaders(struct radeon_cmdbuf *cs, unsigned shaders);
void si_pc_emit_spm_start(struct radeon_cmdbuf *cs);
void si_pc_emit_spm_stop(struct radeon_cmdbuf *cs, bool never_stop_sq_perf_counters,
                         bool never_send_perfcounter_stop);
void si_pc_emit_spm_reset(struct radeon_cmdbuf *cs);
void si_emit_spm_setup(struct si_context *sctx, struct radeon_cmdbuf *cs);
bool si_spm_init(struct si_context *sctx);
void si_spm_finish(struct si_context *sctx);

/* si_query.c */
void si_init_screen_query_functions(struct si_screen *sscreen);
void si_init_query_functions(struct si_context *sctx);
void si_suspend_queries(struct si_context *sctx);
void si_resume_queries(struct si_context *sctx);

/* si_shaderlib_nir.c */
void *si_create_copy_image_cs(struct si_context *sctx, unsigned wg_dim,
                              bool src_is_1d_array, bool dst_is_1d_array);
void *si_create_dcc_retile_cs(struct si_context *sctx, struct radeon_surf *surf);
void *gfx9_create_clear_dcc_msaa_cs(struct si_context *sctx, struct si_texture *tex);
void *si_create_passthrough_tcs(struct si_context *sctx);

union si_compute_blit_shader_key {
   struct {
      /* The key saved in _mesa_hash_table_create_u32_keys() can't be 0. */
      bool always_true:1;
      /* Declaration modifiers. */
      uint8_t wg_dim:2; /* 1, 2, or 3 */
      bool src_is_1d:1;
      bool dst_is_1d:1;
      bool src_is_msaa:1;
      bool dst_is_msaa:1;
      uint8_t log2_samples:4;
      bool sample0_only:1; /* src is MSAA, dst is not MSAA, log2_samples is ignored */
      /* Source coordinate modifiers. */
      bool xy_clamp_to_edge:1;
      bool flip_x:1;
      bool flip_y:1;
      /* Output modifiers. */
      bool sint_to_uint:1;
      bool uint_to_sint:1;
      bool dst_is_srgb:1;
      bool use_integer_one:1;
      uint8_t last_src_channel:2;
      uint8_t last_dst_channel:2;
      bool fp16_rtz:1; /* only for equality with pixel shaders, not necessary otherwise */
   };
   uint32_t key;
};

void *si_create_blit_cs(struct si_context *sctx, const union si_compute_blit_shader_key *options);

/* si_shaderlib_nir.c */
void *si_get_blitter_vs(struct si_context *sctx, enum blitter_attrib_type type,
                        unsigned num_layers);
void *si_create_dma_compute_shader(struct si_context *sctx, unsigned num_dwords_per_thread,
                                   bool dst_stream_cache_policy, bool is_copy);
void *si_create_ubyte_to_ushort_compute_shader(struct si_context *sctx);
void *si_create_clear_buffer_rmw_cs(struct si_context *sctx);
void *si_clear_render_target_shader(struct si_context *sctx, enum pipe_texture_target type);
void *si_clear_12bytes_buffer_shader(struct si_context *sctx);
void *si_create_fmask_expand_cs(struct si_context *sctx, unsigned num_samples, bool is_array);
void *si_create_query_result_cs(struct si_context *sctx);
void *gfx11_create_sh_query_result_cs(struct si_context *sctx);

/* gfx11_query.c */
void si_gfx11_init_query(struct si_context *sctx);
void si_gfx11_destroy_query(struct si_context *sctx);

/* si_test_image_copy_region.c */
void si_test_image_copy_region(struct si_screen *sscreen);
void si_test_blit(struct si_screen *sscreen, unsigned test_flags);

/* si_test_clearbuffer.c */
void si_test_dma_perf(struct si_screen *sscreen);

/* si_uvd.c */
struct pipe_video_codec *si_uvd_create_decoder(struct pipe_context *context,
                                               const struct pipe_video_codec *templ);

struct pipe_video_buffer *si_video_buffer_create(struct pipe_context *pipe,
                                                 const struct pipe_video_buffer *tmpl);
struct pipe_video_buffer *si_video_buffer_create_with_modifiers(struct pipe_context *pipe,
                                                                const struct pipe_video_buffer *tmpl,
                                                                const uint64_t *modifiers,
                                                                unsigned int modifiers_count);

/* si_state_viewport.c */
void si_update_vs_viewport_state(struct si_context *ctx);
void si_init_viewport_functions(struct si_context *ctx);

/* si_texture.c */
void si_eliminate_fast_color_clear(struct si_context *sctx, struct si_texture *tex,
                                   bool *ctx_flushed);
void si_texture_discard_cmask(struct si_screen *sscreen, struct si_texture *tex);
bool si_init_flushed_depth_texture(struct pipe_context *ctx, struct pipe_resource *texture);
void si_print_texture_info(struct si_screen *sscreen, struct si_texture *tex,
                           struct u_log_context *log);
struct pipe_resource *si_texture_create(struct pipe_screen *screen,
                                        const struct pipe_resource *templ);
bool si_texture_commit(struct si_context *ctx, struct si_resource *res, unsigned level,
                       struct pipe_box *box, bool commit);
bool vi_dcc_formats_compatible(struct si_screen *sscreen, enum pipe_format format1,
                               enum pipe_format format2);
bool vi_dcc_formats_are_incompatible(struct pipe_resource *tex, unsigned level,
                                     enum pipe_format view_format);
void vi_disable_dcc_if_incompatible_format(struct si_context *sctx, struct pipe_resource *tex,
                                           unsigned level, enum pipe_format view_format);
unsigned si_translate_colorswap(enum amd_gfx_level gfx_level, enum pipe_format format,
                                bool do_endian_swap);
bool si_texture_disable_dcc(struct si_context *sctx, struct si_texture *tex);
void si_init_screen_texture_functions(struct si_screen *sscreen);
void si_init_context_texture_functions(struct si_context *sctx);

/* si_sqtt.c */
void si_sqtt_write_event_marker(struct si_context* sctx, struct radeon_cmdbuf *rcs,
                                enum rgp_sqtt_marker_event_type api_type,
                                uint32_t vertex_offset_user_data,
                                uint32_t instance_offset_user_data,
                                uint32_t draw_index_user_data);
bool si_sqtt_register_pipeline(struct si_context* sctx, struct si_sqtt_fake_pipeline *pipeline, bool is_compute);
bool si_sqtt_pipeline_is_registered(struct ac_sqtt *sqtt,
                                    uint64_t pipeline_hash);
void si_sqtt_describe_pipeline_bind(struct si_context* sctx, uint64_t pipeline_hash, int bind_point);
void
si_write_event_with_dims_marker(struct si_context* sctx, struct radeon_cmdbuf *rcs,
                                enum rgp_sqtt_marker_event_type api_type,
                                uint32_t x, uint32_t y, uint32_t z);
void
si_write_user_event(struct si_context* sctx, struct radeon_cmdbuf *rcs,
                    enum rgp_sqtt_marker_user_event_type type,
                    const char *str, int len);
void
si_sqtt_describe_barrier_start(struct si_context* sctx, struct radeon_cmdbuf *rcs);
void
si_sqtt_describe_barrier_end(struct si_context* sctx, struct radeon_cmdbuf *rcs, unsigned flags);
bool si_init_sqtt(struct si_context *sctx);
void si_destroy_sqtt(struct si_context *sctx);
void si_handle_sqtt(struct si_context *sctx, struct radeon_cmdbuf *rcs);

/*
 * common helpers
 */

static inline void si_compute_reference(struct si_compute **dst, struct si_compute *src)
{
   if (pipe_reference(&(*dst)->sel.base.reference, &src->sel.base.reference))
      si_destroy_compute(*dst);

   *dst = src;
}

static inline struct si_resource *si_resource(struct pipe_resource *r)
{
   return (struct si_resource *)r;
}

static inline void si_resource_reference(struct si_resource **ptr, struct si_resource *res)
{
   pipe_resource_reference((struct pipe_resource **)ptr, (struct pipe_resource *)res);
}

static inline void si_texture_reference(struct si_texture **ptr, struct si_texture *res)
{
   pipe_resource_reference((struct pipe_resource **)ptr, &res->buffer.b.b);
}

static inline void
si_shader_selector_reference(struct si_context *sctx, /* sctx can optionally be NULL */
                             struct si_shader_selector **dst, struct si_shader_selector *src)
{
   if (*dst == src)
      return;

   struct si_screen *sscreen = src ? src->screen : (*dst)->screen;
   util_shader_reference(&sctx->b, &sscreen->live_shader_cache, (void **)dst, src);
}

static inline bool vi_dcc_enabled(struct si_texture *tex, unsigned level)
{
   return !tex->is_depth && tex->surface.meta_offset && level < tex->surface.num_meta_levels;
}

static inline unsigned si_tile_mode_index(struct si_texture *tex, unsigned level, bool stencil)
{
   if (stencil)
      return tex->surface.u.legacy.zs.stencil_tiling_index[level];
   else
      return tex->surface.u.legacy.tiling_index[level];
}

static inline unsigned si_get_minimum_num_gfx_cs_dwords(struct si_context *sctx,
                                                        unsigned num_draws)
{
   /* Don't count the needed CS space exactly and just use an upper bound.
    *
    * Also reserve space for stopping queries at the end of IB, because
    * the number of active queries is unlimited in theory.
    */
   return 2048 + sctx->num_cs_dw_queries_suspend + num_draws * 10;
}

static inline uint64_t si_get_atom_bit(struct si_context *sctx, struct si_atom *atom)
{
   return 1ull << (atom - sctx->atoms.array);
}

static inline void si_set_atom_dirty(struct si_context *sctx, struct si_atom *atom, bool dirty)
{
   uint64_t bit = si_get_atom_bit(sctx, atom);

   if (dirty)
      sctx->dirty_atoms |= bit;
   else
      sctx->dirty_atoms &= ~bit;
}

static inline bool si_is_atom_dirty(struct si_context *sctx, struct si_atom *atom)
{
   return (sctx->dirty_atoms & si_get_atom_bit(sctx, atom)) != 0;
}

static inline void si_mark_atom_dirty(struct si_context *sctx, struct si_atom *atom)
{
   si_set_atom_dirty(sctx, atom, true);
}

/* This should be evaluated at compile time if all parameters except sctx are constants. */
static ALWAYS_INLINE struct si_shader_ctx_state *
si_get_vs_inline(struct si_context *sctx, enum si_has_tess has_tess, enum si_has_gs has_gs)
{
   if (has_gs)
      return &sctx->shader.gs;
   if (has_tess)
      return &sctx->shader.tes;

   return &sctx->shader.vs;
}

static inline struct si_shader_ctx_state *si_get_vs(struct si_context *sctx)
{
   return si_get_vs_inline(sctx, sctx->shader.tes.cso ? TESS_ON : TESS_OFF,
                           sctx->shader.gs.cso ? GS_ON : GS_OFF);
}

static inline bool si_get_strmout_en(struct si_context *sctx)
{
   return sctx->streamout.streamout_enabled || sctx->streamout.prims_gen_query_enabled;
}

static inline unsigned si_optimal_tcc_alignment(struct si_context *sctx, unsigned upload_size)
{
   unsigned alignment, tcc_cache_line_size;

   /* If the upload size is less than the cache line size (e.g. 16, 32),
    * the whole thing will fit into a cache line if we align it to its size.
    * The idea is that multiple small uploads can share a cache line.
    * If the upload size is greater, align it to the cache line size.
    */
   alignment = util_next_power_of_two(upload_size);
   tcc_cache_line_size = sctx->screen->info.tcc_cache_line_size;
   return MIN2(alignment, tcc_cache_line_size);
}

static inline void si_saved_cs_reference(struct si_saved_cs **dst, struct si_saved_cs *src)
{
   if (pipe_reference(&(*dst)->reference, &src->reference))
      si_destroy_saved_cs(*dst);

   *dst = src;
}

static inline void si_make_CB_shader_coherent(struct si_context *sctx, unsigned num_samples,
                                              bool shaders_read_metadata, bool dcc_pipe_aligned)
{
   sctx->flags |= SI_CONTEXT_FLUSH_AND_INV_CB | SI_CONTEXT_INV_VCACHE;
   sctx->force_cb_shader_coherent = false;

   if (sctx->gfx_level >= GFX10) {
      if (sctx->screen->info.tcc_rb_non_coherent)
         sctx->flags |= SI_CONTEXT_INV_L2;
      else if (shaders_read_metadata)
         sctx->flags |= SI_CONTEXT_INV_L2_METADATA;
   } else if (sctx->gfx_level == GFX9) {
      /* Single-sample color is coherent with shaders on GFX9, but
       * L2 metadata must be flushed if shaders read metadata.
       * (DCC, CMASK).
       */
      if (num_samples >= 2 || (shaders_read_metadata && !dcc_pipe_aligned))
         sctx->flags |= SI_CONTEXT_INV_L2;
      else if (shaders_read_metadata)
         sctx->flags |= SI_CONTEXT_INV_L2_METADATA;
   } else {
      /* GFX6-GFX8 */
      sctx->flags |= SI_CONTEXT_INV_L2;
   }

   si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
}

static inline void si_make_DB_shader_coherent(struct si_context *sctx, unsigned num_samples,
                                              bool include_stencil, bool shaders_read_metadata)
{
   sctx->flags |= SI_CONTEXT_FLUSH_AND_INV_DB | SI_CONTEXT_INV_VCACHE;

   if (sctx->gfx_level >= GFX10) {
      if (sctx->screen->info.tcc_rb_non_coherent)
         sctx->flags |= SI_CONTEXT_INV_L2;
      else if (shaders_read_metadata)
         sctx->flags |= SI_CONTEXT_INV_L2_METADATA;
   } else if (sctx->gfx_level == GFX9) {
      /* Single-sample depth (not stencil) is coherent with shaders
       * on GFX9, but L2 metadata must be flushed if shaders read
       * metadata.
       */
      if (num_samples >= 2 || include_stencil)
         sctx->flags |= SI_CONTEXT_INV_L2;
      else if (shaders_read_metadata)
         sctx->flags |= SI_CONTEXT_INV_L2_METADATA;
   } else {
      /* GFX6-GFX8 */
      sctx->flags |= SI_CONTEXT_INV_L2;
   }

   si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
}

static inline bool si_can_sample_zs(struct si_texture *tex, bool stencil_sampler)
{
   return (stencil_sampler && tex->can_sample_s) || (!stencil_sampler && tex->can_sample_z);
}

static inline bool si_htile_enabled(struct si_texture *tex, unsigned level, unsigned zs_mask)
{
   if (zs_mask == PIPE_MASK_S && (tex->htile_stencil_disabled || !tex->surface.has_stencil))
      return false;

   if (!tex->is_depth || !tex->surface.meta_offset)
      return false;

   struct si_screen *sscreen = (struct si_screen *)tex->buffer.b.b.screen;
   if (sscreen->info.gfx_level >= GFX8) {
      return level < tex->surface.num_meta_levels;
   } else {
      /* GFX6-7 don't have TC-compatible HTILE, which means they have to run
       * a decompression pass for every mipmap level before texturing, so compress
       * only one level to reduce the number of decompression passes to a minimum.
       */
      return level == 0;
   }
}

static inline bool vi_tc_compat_htile_enabled(struct si_texture *tex, unsigned level,
                                              unsigned zs_mask)
{
   assert(!tex->tc_compatible_htile || tex->surface.meta_offset);
   return tex->tc_compatible_htile && si_htile_enabled(tex, level, zs_mask);
}

static inline unsigned si_get_ps_iter_samples(struct si_context *sctx)
{
   if (sctx->gfx11_force_msaa_num_samples_zero)
      return 1;

   if (sctx->ps_uses_fbfetch)
      return sctx->framebuffer.nr_color_samples;

   return MIN2(sctx->ps_iter_samples, sctx->framebuffer.nr_color_samples);
}

static inline bool si_any_colorbuffer_written(struct si_context *sctx)
{
   if (sctx->queued.named.rasterizer->rasterizer_discard)
      return false;

   struct si_shader_selector *ps = sctx->shader.ps.cso;
   if (!ps || !ps->info.colors_written_4bit)
      return false;

   return (sctx->framebuffer.colorbuf_enabled_4bit &
           sctx->queued.named.blend->cb_target_enabled_4bit &
           (ps->info.color0_writes_all_cbufs ? ~0 : ps->info.colors_written_4bit)) != 0;
}

#define UTIL_ALL_PRIM_LINE_MODES                                                                   \
   ((1 << MESA_PRIM_LINES) | (1 << MESA_PRIM_LINE_LOOP) | (1 << MESA_PRIM_LINE_STRIP) |            \
    (1 << MESA_PRIM_LINES_ADJACENCY) | (1 << MESA_PRIM_LINE_STRIP_ADJACENCY))

#define UTIL_ALL_PRIM_TRIANGLE_MODES \
   ((1 << MESA_PRIM_TRIANGLES) | (1 << MESA_PRIM_TRIANGLE_STRIP) | \
    (1 << MESA_PRIM_TRIANGLE_FAN) | (1 << MESA_PRIM_QUADS) | (1 << MESA_PRIM_QUAD_STRIP) | \
    (1 << MESA_PRIM_POLYGON) | (1 << MESA_PRIM_TRIANGLES_ADJACENCY) | \
    (1 << MESA_PRIM_TRIANGLE_STRIP_ADJACENCY))

static inline bool util_prim_is_lines(unsigned prim)
{
   return ((1 << prim) & UTIL_ALL_PRIM_LINE_MODES) != 0;
}

static inline bool util_prim_is_points_or_lines(unsigned prim)
{
   return ((1 << prim) & (UTIL_ALL_PRIM_LINE_MODES | (1 << MESA_PRIM_POINTS))) != 0;
}

static inline bool util_rast_prim_is_triangles(unsigned prim)
{
   return ((1 << prim) & UTIL_ALL_PRIM_TRIANGLE_MODES) != 0;
}

static inline bool util_rast_prim_is_lines_or_triangles(unsigned prim)
{
   return ((1 << prim) & (UTIL_ALL_PRIM_LINE_MODES | UTIL_ALL_PRIM_TRIANGLE_MODES)) != 0;
}

static inline void si_need_gfx_cs_space(struct si_context *ctx, unsigned num_draws)
{
   struct radeon_cmdbuf *cs = &ctx->gfx_cs;

   if (!ctx->ws->cs_check_space(cs, si_get_minimum_num_gfx_cs_dwords(ctx, num_draws)))
      si_flush_gfx_cs(ctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW, NULL);
}

/**
 * Add a buffer to the buffer list for the given command stream (CS).
 *
 * All buffers used by a CS must be added to the list. This tells the kernel
 * driver which buffers are used by GPU commands. Other buffers can
 * be swapped out (not accessible) during execution.
 *
 * The buffer list becomes empty after every context flush and must be
 * rebuilt.
 */
static inline void radeon_add_to_buffer_list(struct si_context *sctx, struct radeon_cmdbuf *cs,
                                             struct si_resource *bo, unsigned usage)
{
   assert(usage);
   sctx->ws->cs_add_buffer(cs, bo->buf, usage | RADEON_USAGE_SYNCHRONIZED,
                           bo->domains);
}

static inline void si_select_draw_vbo(struct si_context *sctx)
{
   pipe_draw_func draw_vbo = sctx->draw_vbo[!!sctx->shader.tes.cso]
                                           [!!sctx->shader.gs.cso]
                                           [sctx->ngg];
   pipe_draw_vertex_state_func draw_vertex_state =
      sctx->draw_vertex_state[!!sctx->shader.tes.cso]
                             [!!sctx->shader.gs.cso]
                             [sctx->ngg];
   assert(draw_vbo);
   assert(draw_vertex_state);

   if (unlikely(sctx->real_draw_vbo)) {
      assert(sctx->real_draw_vertex_state);
      sctx->real_draw_vbo = draw_vbo;
      sctx->real_draw_vertex_state = draw_vertex_state;
   } else {
      assert(!sctx->real_draw_vertex_state);
      sctx->b.draw_vbo = draw_vbo;
      sctx->b.draw_vertex_state = draw_vertex_state;
   }
}

/* Return the number of samples that the rasterizer uses. */
static inline unsigned si_get_num_coverage_samples(struct si_context *sctx)
{
   if (sctx->framebuffer.nr_samples > 1 &&
       sctx->queued.named.rasterizer->multisample_enable)
      return sctx->framebuffer.nr_samples;

   /* Note that smoothing_enabled is set by si_update_shaders. */
   if (sctx->smoothing_enabled)
      return SI_NUM_SMOOTH_AA_SAMPLES;

   return 1;
}

static unsigned ALWAYS_INLINE
si_num_vbos_in_user_sgprs_inline(enum amd_gfx_level gfx_level)
{
   /* This decreases CPU overhead if all descriptors are in user SGPRs because we don't
    * have to allocate and count references for the upload buffer.
    */
   return gfx_level >= GFX9 ? 5 : 1;
}

static inline unsigned si_num_vbos_in_user_sgprs(struct si_screen *sscreen)
{
   return si_num_vbos_in_user_sgprs_inline(sscreen->info.gfx_level);
}

static inline
void si_check_dirty_buffers_textures(struct si_context *sctx)
{
   /* Recompute and re-emit the texture resource states if needed. */
   unsigned dirty_tex_counter = p_atomic_read(&sctx->screen->dirty_tex_counter);
   if (unlikely(dirty_tex_counter != sctx->last_dirty_tex_counter)) {
      sctx->last_dirty_tex_counter = dirty_tex_counter;
      sctx->framebuffer.dirty_cbufs |= ((1 << sctx->framebuffer.state.nr_cbufs) - 1);
      sctx->framebuffer.dirty_zsbuf = true;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);
      si_update_all_texture_descriptors(sctx);
   }

   unsigned dirty_buf_counter = p_atomic_read(&sctx->screen->dirty_buf_counter);
   if (unlikely(dirty_buf_counter != sctx->last_dirty_buf_counter)) {
      sctx->last_dirty_buf_counter = dirty_buf_counter;
      /* Rebind all buffers unconditionally. */
      si_rebind_buffer(sctx, NULL);
   }
}

static inline void si_set_clip_discard_distance(struct si_context *sctx, float distance)
{
   /* Determine whether the guardband registers change.
    *
    * When we see a value greater than min_clip_discard_distance_watermark, we increase it
    * up to a certain number to eliminate those state changes next time they happen.
    * See the comment at min_clip_discard_distance_watermark.
    */
   if (distance > sctx->min_clip_discard_distance_watermark) {
      /* The maximum number was determined from Viewperf. The number is in units of half-pixels. */
      sctx->min_clip_discard_distance_watermark = MIN2(distance, 6);

      float old_distance = sctx->current_clip_discard_distance;
      float new_distance = MAX2(distance, sctx->min_clip_discard_distance_watermark);

      if (old_distance != new_distance) {
         sctx->current_clip_discard_distance = new_distance;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.guardband);
      }
   }
}

/* Update these two GS_STATE fields. They depend on whatever the last shader before PS is
 * and the rasterizer state.
 *
 * It's expected that hw_vs and ngg are inline constants in draw_vbo after optimizations.
 */
static inline void
si_update_ngg_prim_state_sgpr(struct si_context *sctx, struct si_shader *hw_vs, bool ngg)
{
   if (!ngg || !hw_vs)
      return;

   if (hw_vs->uses_vs_state_provoking_vertex) {
      unsigned vtx_index = sctx->queued.named.rasterizer->flatshade_first ? 0 : sctx->gs_out_prim;

      SET_FIELD(sctx->current_gs_state, GS_STATE_PROVOKING_VTX_INDEX, vtx_index);
   }

   if (hw_vs->uses_gs_state_outprim) {
      SET_FIELD(sctx->current_gs_state, GS_STATE_OUTPRIM, sctx->gs_out_prim);
   }
}

/* Set the primitive type seen by the rasterizer. GS and tessellation affect this.
 * It's expected that hw_vs and ngg are inline constants in draw_vbo after optimizations.
 */
static ALWAYS_INLINE void
si_set_rasterized_prim(struct si_context *sctx, enum mesa_prim rast_prim,
                       struct si_shader *hw_vs, bool ngg)
{
   if (rast_prim != sctx->current_rast_prim) {
      bool is_rect = rast_prim == SI_PRIM_RECTANGLE_LIST;
      bool is_points = rast_prim == MESA_PRIM_POINTS;
      bool is_lines = util_prim_is_lines(rast_prim);

      if (is_points) {
         si_set_clip_discard_distance(sctx, sctx->queued.named.rasterizer->max_point_size);
         sctx->gs_out_prim = V_028A6C_POINTLIST;
      } else if (is_lines) {
         si_set_clip_discard_distance(sctx, sctx->queued.named.rasterizer->line_width);
         sctx->gs_out_prim = V_028A6C_LINESTRIP;
      } else if (is_rect) {
         /* Don't change the clip discard distance for rectangles. */
         sctx->gs_out_prim = V_028A6C_RECTLIST;
      } else {
         si_set_clip_discard_distance(sctx, 0);
         sctx->gs_out_prim = V_028A6C_TRISTRIP;
      }

      sctx->current_rast_prim = rast_prim;
      si_vs_ps_key_update_rast_prim_smooth_stipple(sctx);
      si_update_ngg_prim_state_sgpr(sctx, hw_vs, ngg);
   }
}

/* There are 3 ways to flush caches and all of them are correct.
 *
 * 1) sctx->flags |= ...;
 *    si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush); // deferred
 *
 * 2) sctx->flags |= ...;
 *    si_emit_cache_flush_direct(sctx); // immediate
 *
 * 3) sctx->flags |= ...;
 *    sctx->emit_cache_flush(sctx, cs); // immediate (2 is better though)
 */
static inline void si_emit_cache_flush_direct(struct si_context *sctx)
{
   sctx->emit_cache_flush(sctx, &sctx->gfx_cs);
   sctx->dirty_atoms &= ~SI_ATOM_BIT(cache_flush);
}

#define PRINT_ERR(fmt, args...)                                                                    \
   fprintf(stderr, "EE %s:%d %s - " fmt, __FILE__, __LINE__, __func__, ##args)

#ifdef __cplusplus
}
#endif

#endif
