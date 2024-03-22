/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef RADV_PRIVATE_H
#define RADV_PRIVATE_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_VALGRIND
#include <memcheck.h>
#include <valgrind.h>
#define VG(x) x
#else
#define VG(x) ((void)0)
#endif

#include "c11/threads.h"
#ifndef _WIN32
#include <amdgpu.h>
#include <xf86drm.h>
#endif
#include "compiler/shader_enums.h"
#include "util/bitscan.h"
#include "util/list.h"
#include "util/macros.h"
#include "util/rwlock.h"
#include "util/xmlconfig.h"
#include "vk_alloc.h"
#include "vk_buffer.h"
#include "vk_buffer_view.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_debug_report.h"
#include "vk_device.h"
#include "vk_format.h"
#include "vk_image.h"
#include "vk_instance.h"
#include "vk_log.h"
#include "vk_physical_device.h"
#include "vk_query_pool.h"
#include "vk_queue.h"
#include "vk_sampler.h"
#include "vk_shader_module.h"
#include "vk_texcompress_astc.h"
#include "vk_texcompress_etc2.h"
#include "vk_util.h"
#include "vk_video.h"
#include "vk_ycbcr_conversion.h"

#include "rmv/vk_rmv_common.h"
#include "rmv/vk_rmv_tokens.h"

#include "ac_binary.h"
#include "ac_gpu_info.h"
#include "ac_shader_util.h"
#include "ac_spm.h"
#include "ac_sqtt.h"
#include "ac_surface.h"
#include "ac_vcn.h"
#include "radv_constants.h"
#include "radv_descriptor_set.h"
#include "radv_radeon_winsys.h"
#include "radv_shader.h"
#include "radv_shader_args.h"
#include "sid.h"

#include "radix_sort/radix_sort_vk_devaddr.h"

/* Pre-declarations needed for WSI entrypoints */
struct wl_surface;
struct wl_display;
typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_visualid_t;
typedef uint32_t xcb_window_t;

#include <vulkan/vk_android_native_buffer.h>
#include <vulkan/vk_icd.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#include "radv_entrypoints.h"

#include "wsi_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Helper to determine if we should compile
 * any of the Android AHB support.
 *
 * To actually enable the ext we also need
 * the necessary kernel support.
 */
#if defined(ANDROID) && ANDROID_API_LEVEL >= 26
#define RADV_SUPPORT_ANDROID_HARDWARE_BUFFER 1
#include <vndk/hardware_buffer.h>
#else
#define RADV_SUPPORT_ANDROID_HARDWARE_BUFFER 0
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR) || defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR) ||   \
   defined(VK_USE_PLATFORM_DISPLAY_KHR)
#define RADV_USE_WSI_PLATFORM
#endif

#ifdef ANDROID_STRICT
#define RADV_API_VERSION VK_MAKE_VERSION(1, 1, VK_HEADER_VERSION)
#else
#define RADV_API_VERSION VK_MAKE_VERSION(1, 3, VK_HEADER_VERSION)
#endif

#ifdef _WIN32
#define RADV_SUPPORT_CALIBRATED_TIMESTAMPS 0
#else
#define RADV_SUPPORT_CALIBRATED_TIMESTAMPS 1
#endif

#ifdef _WIN32
#define radv_printflike(a, b)
#else
#define radv_printflike(a, b) __attribute__((__format__(__printf__, a, b)))
#endif

/* The "RAW" clocks on Linux are called "FAST" on FreeBSD */
#if !defined(CLOCK_MONOTONIC_RAW) && defined(CLOCK_MONOTONIC_FAST)
#define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC_FAST
#endif

static inline uint32_t
align_u32(uint32_t v, uint32_t a)
{
   assert(a != 0 && a == (a & -a));
   return (v + a - 1) & ~(a - 1);
}

static inline uint32_t
align_u32_npot(uint32_t v, uint32_t a)
{
   return (v + a - 1) / a * a;
}

static inline uint64_t
align_u64(uint64_t v, uint64_t a)
{
   assert(a != 0 && a == (a & -a));
   return (v + a - 1) & ~(a - 1);
}

/** Alignment must be a power of 2. */
static inline bool
radv_is_aligned(uintmax_t n, uintmax_t a)
{
   assert(a == (a & -a));
   return (n & (a - 1)) == 0;
}

static inline uint32_t
radv_minify(uint32_t n, uint32_t levels)
{
   if (unlikely(n == 0))
      return 0;
   else
      return MAX2(n >> levels, 1);
}

static inline int
radv_float_to_sfixed(float value, unsigned frac_bits)
{
   return value * (1 << frac_bits);
}

static inline unsigned int
radv_float_to_ufixed(float value, unsigned frac_bits)
{
   return value * (1 << frac_bits);
}

/* Whenever we generate an error, pass it through this function. Useful for
 * debugging, where we can break on it. Only call at error site, not when
 * propagating errors. Might be useful to plug in a stack trace here.
 */

struct radv_image_view;
struct radv_instance;
struct rvcn_decode_buffer_s;

/* queue types */
enum radv_queue_family {
   RADV_QUEUE_GENERAL,
   RADV_QUEUE_COMPUTE,
   RADV_QUEUE_TRANSFER,
   RADV_QUEUE_SPARSE,
   RADV_QUEUE_VIDEO_DEC,
   RADV_QUEUE_VIDEO_ENC,
   RADV_MAX_QUEUE_FAMILIES,
   RADV_QUEUE_FOREIGN = RADV_MAX_QUEUE_FAMILIES,
   RADV_QUEUE_IGNORED,
};

struct radv_perfcounter_desc;

struct radv_binning_settings {
   unsigned context_states_per_bin;    /* allowed range: [1, 6] */
   unsigned persistent_states_per_bin; /* allowed range: [1, 32] */
   unsigned fpovs_per_batch;           /* allowed range: [0, 255], 0 = unlimited */
};

struct radv_physical_device {
   struct vk_physical_device vk;

   struct radv_instance *instance;

   struct radeon_winsys *ws;
   struct radeon_info rad_info;
   char name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
   char marketing_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
   uint8_t driver_uuid[VK_UUID_SIZE];
   uint8_t device_uuid[VK_UUID_SIZE];
   uint8_t cache_uuid[VK_UUID_SIZE];

   int local_fd;
   int master_fd;
   struct wsi_device wsi_device;

   /* Whether DCC should be enabled for MSAA textures. */
   bool dcc_msaa_allowed;

   /* Whether to enable FMASK compression for MSAA textures (GFX6-GFX10.3) */
   bool use_fmask;

   /* Whether to enable NGG. */
   bool use_ngg;

   /* Whether to enable NGG culling. */
   bool use_ngg_culling;

   /* Whether to enable NGG streamout. */
   bool use_ngg_streamout;

   /* Whether to emulate the number of primitives generated by GS. */
   bool emulate_ngg_gs_query_pipeline_stat;

   /* Whether to emulate mesh/task shader queries. */
   bool emulate_mesh_shader_queries;

   /* Number of threads per wave. */
   uint8_t ps_wave_size;
   uint8_t cs_wave_size;
   uint8_t ge_wave_size;
   uint8_t rt_wave_size;

   /* Maximum compute shared memory size. */
   uint32_t max_shared_size;

   /* Whether to use the LLVM compiler backend */
   bool use_llvm;

   /* Whether to emulate ETC2 image support on HW without support. */
   bool emulate_etc2;

   /* Whether to emulate ASTC image support on HW without support. */
   bool emulate_astc;

   VkPhysicalDeviceMemoryProperties memory_properties;
   enum radeon_bo_domain memory_domains[VK_MAX_MEMORY_TYPES];
   enum radeon_bo_flag memory_flags[VK_MAX_MEMORY_TYPES];
   unsigned heaps;

   /* Bitmask of memory types that use the 32-bit address space. */
   uint32_t memory_types_32bit;

#ifndef _WIN32
   int available_nodes;
   drmPciBusInfo bus_info;

   dev_t primary_devid;
   dev_t render_devid;
#endif

   nir_shader_compiler_options nir_options[MESA_VULKAN_SHADER_STAGES];

   enum radv_queue_family vk_queue_to_radv[RADV_MAX_QUEUE_FAMILIES];
   uint32_t num_queues;

   uint32_t gs_table_depth;

   struct ac_hs_info hs;
   struct ac_task_info task_info;

   struct radv_binning_settings binning_settings;

   /* Performance counters. */
   struct ac_perfcounters ac_perfcounters;

   uint32_t num_perfcounters;
   struct radv_perfcounter_desc *perfcounters;

   struct {
      unsigned data0;
      unsigned data1;
      unsigned cmd;
      unsigned cntl;
   } vid_dec_reg;
   enum amd_ip_type vid_decode_ip;
   uint32_t vid_addr_gfx_mode;
   uint32_t stream_handle_base;
   uint32_t stream_handle_counter;
};

uint32_t radv_find_memory_index(const struct radv_physical_device *pdevice, VkMemoryPropertyFlags flags);

VkResult create_null_physical_device(struct vk_instance *vk_instance);

VkResult create_drm_physical_device(struct vk_instance *vk_instance, struct _drmDevice *device,
                                    struct vk_physical_device **out);

void radv_physical_device_destroy(struct vk_physical_device *vk_device);

enum radv_trace_mode {
   /** Radeon GPU Profiler */
   RADV_TRACE_MODE_RGP = 1 << VK_TRACE_MODE_COUNT,

   /** Radeon Raytracing Analyzer */
   RADV_TRACE_MODE_RRA = 1 << (VK_TRACE_MODE_COUNT + 1),
};

struct radv_instance {
   struct vk_instance vk;

   VkAllocationCallbacks alloc;

   uint64_t debug_flags;
   uint64_t perftest_flags;

   struct {
      struct driOptionCache options;
      struct driOptionCache available_options;

      bool enable_mrt_output_nan_fixup;
      bool disable_tc_compat_htile_in_general;
      bool disable_shrink_image_store;
      bool disable_aniso_single_level;
      bool disable_trunc_coord;
      bool zero_vram;
      bool disable_sinking_load_input_fs;
      bool flush_before_query_copy;
      bool enable_unified_heap_on_apu;
      bool tex_non_uniform;
      bool ssbo_non_uniform;
      bool flush_before_timestamp_write;
      bool force_rt_wave64;
      bool dual_color_blend_by_location;
      bool legacy_sparse_binding;
      bool clear_lds;
      bool enable_dgc;
      bool enable_khr_present_wait;
      bool report_llvm9_version_string;
      bool vk_require_etc2;
      bool vk_require_astc;
      bool force_active_accel_struct_leaves;
      char *app_layer;
      uint8_t override_graphics_shader_version;
      uint8_t override_compute_shader_version;
      uint8_t override_ray_tracing_shader_version;
      int override_vram_size;
      int override_uniform_offset_alignment;
   } drirc;
};

VkResult radv_init_wsi(struct radv_physical_device *physical_device);
void radv_finish_wsi(struct radv_physical_device *physical_device);

struct radv_shader_binary_part;

bool radv_pipeline_cache_search(struct radv_device *device, struct vk_pipeline_cache *cache,
                                struct radv_pipeline *pipeline, const unsigned char *sha1,
                                bool *found_in_application_cache);

void radv_pipeline_cache_insert(struct radv_device *device, struct vk_pipeline_cache *cache,
                                struct radv_pipeline *pipeline, const unsigned char *sha1);

struct radv_ray_tracing_pipeline;
bool radv_ray_tracing_pipeline_cache_search(struct radv_device *device, struct vk_pipeline_cache *cache,
                                            struct radv_ray_tracing_pipeline *pipeline,
                                            const VkRayTracingPipelineCreateInfoKHR *create_info);

void radv_ray_tracing_pipeline_cache_insert(struct radv_device *device, struct vk_pipeline_cache *cache,
                                            struct radv_ray_tracing_pipeline *pipeline, unsigned num_stages,
                                            const unsigned char *sha1);

struct vk_pipeline_cache_object *
radv_pipeline_cache_search_nir(struct radv_device *device, struct vk_pipeline_cache *cache, const unsigned char *sha1);

struct vk_pipeline_cache_object *radv_pipeline_cache_nir_to_handle(struct radv_device *device,
                                                                   struct vk_pipeline_cache *cache,
                                                                   struct nir_shader *nir, const unsigned char *sha1,
                                                                   bool cached);

struct nir_shader *radv_pipeline_cache_handle_to_nir(struct radv_device *device,
                                                     struct vk_pipeline_cache_object *object);

struct radv_meta_state {
   VkAllocationCallbacks alloc;

   VkPipelineCache cache;
   uint32_t initial_cache_entries;

   /*
    * For on-demand pipeline creation, makes sure that
    * only one thread tries to build a pipeline at the same time.
    */
   mtx_t mtx;

   /**
    * Use array element `i` for images with `2^i` samples.
    */
   struct {
      VkPipeline color_pipelines[NUM_META_FS_KEYS];
   } color_clear[MAX_SAMPLES_LOG2][MAX_RTS];

   struct {
      VkPipeline depth_only_pipeline[NUM_DEPTH_CLEAR_PIPELINES];
      VkPipeline stencil_only_pipeline[NUM_DEPTH_CLEAR_PIPELINES];
      VkPipeline depthstencil_pipeline[NUM_DEPTH_CLEAR_PIPELINES];

      VkPipeline depth_only_unrestricted_pipeline[NUM_DEPTH_CLEAR_PIPELINES];
      VkPipeline stencil_only_unrestricted_pipeline[NUM_DEPTH_CLEAR_PIPELINES];
      VkPipeline depthstencil_unrestricted_pipeline[NUM_DEPTH_CLEAR_PIPELINES];
   } ds_clear[MAX_SAMPLES_LOG2];

   VkPipelineLayout clear_color_p_layout;
   VkPipelineLayout clear_depth_p_layout;
   VkPipelineLayout clear_depth_unrestricted_p_layout;

   /* Optimized compute fast HTILE clear for stencil or depth only. */
   VkPipeline clear_htile_mask_pipeline;
   VkPipelineLayout clear_htile_mask_p_layout;
   VkDescriptorSetLayout clear_htile_mask_ds_layout;

   /* Copy VRS into HTILE. */
   VkPipeline copy_vrs_htile_pipeline;
   VkPipelineLayout copy_vrs_htile_p_layout;
   VkDescriptorSetLayout copy_vrs_htile_ds_layout;

   /* Clear DCC with comp-to-single. */
   VkPipeline clear_dcc_comp_to_single_pipeline[2]; /* 0: 1x, 1: 2x/4x/8x */
   VkPipelineLayout clear_dcc_comp_to_single_p_layout;
   VkDescriptorSetLayout clear_dcc_comp_to_single_ds_layout;

   struct {
      /** Pipeline that blits from a 1D image. */
      VkPipeline pipeline_1d_src[NUM_META_FS_KEYS];

      /** Pipeline that blits from a 2D image. */
      VkPipeline pipeline_2d_src[NUM_META_FS_KEYS];

      /** Pipeline that blits from a 3D image. */
      VkPipeline pipeline_3d_src[NUM_META_FS_KEYS];

      VkPipeline depth_only_1d_pipeline;
      VkPipeline depth_only_2d_pipeline;
      VkPipeline depth_only_3d_pipeline;

      VkPipeline stencil_only_1d_pipeline;
      VkPipeline stencil_only_2d_pipeline;
      VkPipeline stencil_only_3d_pipeline;
      VkPipelineLayout pipeline_layout;
      VkDescriptorSetLayout ds_layout;
   } blit;

   struct {
      VkPipelineLayout p_layouts[5];
      VkDescriptorSetLayout ds_layouts[5];
      VkPipeline pipelines[5][NUM_META_FS_KEYS];

      VkPipeline depth_only_pipeline[5];

      VkPipeline stencil_only_pipeline[5];
   } blit2d[MAX_SAMPLES_LOG2];

   struct {
      VkPipelineLayout img_p_layout;
      VkDescriptorSetLayout img_ds_layout;
      VkPipeline pipeline;
      VkPipeline pipeline_3d;
   } itob;
   struct {
      VkPipelineLayout img_p_layout;
      VkDescriptorSetLayout img_ds_layout;
      VkPipeline pipeline;
      VkPipeline pipeline_3d;
   } btoi;
   struct {
      VkPipelineLayout img_p_layout;
      VkDescriptorSetLayout img_ds_layout;
      VkPipeline pipeline;
   } btoi_r32g32b32;
   struct {
      VkPipelineLayout img_p_layout;
      VkDescriptorSetLayout img_ds_layout;
      VkPipeline pipeline[MAX_SAMPLES_LOG2];
      VkPipeline pipeline_3d;
   } itoi;
   struct {
      VkPipelineLayout img_p_layout;
      VkDescriptorSetLayout img_ds_layout;
      VkPipeline pipeline;
   } itoi_r32g32b32;
   struct {
      VkPipelineLayout img_p_layout;
      VkDescriptorSetLayout img_ds_layout;
      VkPipeline pipeline[MAX_SAMPLES_LOG2];
      VkPipeline pipeline_3d;
   } cleari;
   struct {
      VkPipelineLayout img_p_layout;
      VkDescriptorSetLayout img_ds_layout;
      VkPipeline pipeline;
   } cleari_r32g32b32;
   struct {
      VkPipelineLayout p_layout;
      VkDescriptorSetLayout ds_layout;
      VkPipeline pipeline[MAX_SAMPLES_LOG2];
   } fmask_copy;

   struct {
      VkPipelineLayout p_layout;
      VkPipeline pipeline[NUM_META_FS_KEYS];
   } resolve;

   struct {
      VkDescriptorSetLayout ds_layout;
      VkPipelineLayout p_layout;
      struct {
         VkPipeline pipeline;
         VkPipeline i_pipeline;
         VkPipeline srgb_pipeline;
      } rc[MAX_SAMPLES_LOG2];

      VkPipeline depth_zero_pipeline;
      struct {
         VkPipeline average_pipeline;
         VkPipeline max_pipeline;
         VkPipeline min_pipeline;
      } depth[MAX_SAMPLES_LOG2];

      VkPipeline stencil_zero_pipeline;
      struct {
         VkPipeline max_pipeline;
         VkPipeline min_pipeline;
      } stencil[MAX_SAMPLES_LOG2];
   } resolve_compute;

   struct {
      VkDescriptorSetLayout ds_layout;
      VkPipelineLayout p_layout;

      struct {
         VkPipeline pipeline[NUM_META_FS_KEYS];
      } rc[MAX_SAMPLES_LOG2];

      VkPipeline depth_zero_pipeline;
      struct {
         VkPipeline average_pipeline;
         VkPipeline max_pipeline;
         VkPipeline min_pipeline;
      } depth[MAX_SAMPLES_LOG2];

      VkPipeline stencil_zero_pipeline;
      struct {
         VkPipeline max_pipeline;
         VkPipeline min_pipeline;
      } stencil[MAX_SAMPLES_LOG2];
   } resolve_fragment;

   struct {
      VkPipelineLayout p_layout;
      VkPipeline decompress_pipeline;
      VkPipeline resummarize_pipeline;
   } depth_decomp[MAX_SAMPLES_LOG2];

   VkDescriptorSetLayout expand_depth_stencil_compute_ds_layout;
   VkPipelineLayout expand_depth_stencil_compute_p_layout;
   VkPipeline expand_depth_stencil_compute_pipeline;

   struct {
      VkPipelineLayout p_layout;
      VkPipeline cmask_eliminate_pipeline;
      VkPipeline fmask_decompress_pipeline;
      VkPipeline dcc_decompress_pipeline;

      VkDescriptorSetLayout dcc_decompress_compute_ds_layout;
      VkPipelineLayout dcc_decompress_compute_p_layout;
      VkPipeline dcc_decompress_compute_pipeline;
   } fast_clear_flush;

   struct {
      VkPipelineLayout fill_p_layout;
      VkPipelineLayout copy_p_layout;
      VkPipeline fill_pipeline;
      VkPipeline copy_pipeline;
   } buffer;

   struct {
      VkDescriptorSetLayout ds_layout;
      VkPipelineLayout p_layout;
      VkPipeline occlusion_query_pipeline;
      VkPipeline pipeline_statistics_query_pipeline;
      VkPipeline tfb_query_pipeline;
      VkPipeline timestamp_query_pipeline;
      VkPipeline pg_query_pipeline;
      VkPipeline ms_prim_gen_query_pipeline;
   } query;

   struct {
      VkDescriptorSetLayout ds_layout;
      VkPipelineLayout p_layout;
      VkPipeline pipeline[MAX_SAMPLES_LOG2];
   } fmask_expand;

   struct {
      VkDescriptorSetLayout ds_layout;
      VkPipelineLayout p_layout;
      VkPipeline pipeline[32];
   } dcc_retile;

   struct {
      VkPipelineLayout leaf_p_layout;
      VkPipeline leaf_pipeline;
      VkPipelineLayout morton_p_layout;
      VkPipeline morton_pipeline;
      VkPipelineLayout lbvh_main_p_layout;
      VkPipeline lbvh_main_pipeline;
      VkPipelineLayout lbvh_generate_ir_p_layout;
      VkPipeline lbvh_generate_ir_pipeline;
      VkPipelineLayout ploc_p_layout;
      VkPipeline ploc_pipeline;
      VkPipelineLayout encode_p_layout;
      VkPipeline encode_pipeline;
      VkPipeline encode_compact_pipeline;
      VkPipelineLayout header_p_layout;
      VkPipeline header_pipeline;
      VkPipelineLayout update_p_layout;
      VkPipeline update_pipeline;
      VkPipelineLayout copy_p_layout;
      VkPipeline copy_pipeline;

      struct radix_sort_vk *radix_sort;

      struct {
         VkBuffer buffer;
         VkDeviceMemory memory;
         VkAccelerationStructureKHR accel_struct;
      } null;
   } accel_struct_build;

   struct vk_texcompress_etc2_state etc_decode;

   struct vk_texcompress_astc_state *astc_decode;

   struct {
      VkDescriptorSetLayout ds_layout;
      VkPipelineLayout p_layout;
      VkPipeline pipeline;
   } dgc_prepare;
};

#define RADV_NUM_HW_CTX (RADEON_CTX_PRIORITY_REALTIME + 1)

static inline enum radv_queue_family
vk_queue_to_radv(const struct radv_physical_device *phys_dev, int queue_family_index)
{
   if (queue_family_index == VK_QUEUE_FAMILY_EXTERNAL || queue_family_index == VK_QUEUE_FAMILY_FOREIGN_EXT)
      return RADV_QUEUE_FOREIGN;
   if (queue_family_index == VK_QUEUE_FAMILY_IGNORED)
      return RADV_QUEUE_IGNORED;

   assert(queue_family_index < RADV_MAX_QUEUE_FAMILIES);
   return phys_dev->vk_queue_to_radv[queue_family_index];
}

enum amd_ip_type radv_queue_family_to_ring(const struct radv_physical_device *physical_device,
                                           enum radv_queue_family f);

static inline bool
radv_has_uvd(struct radv_physical_device *phys_dev)
{
   enum radeon_family family = phys_dev->rad_info.family;
   /* Only support UVD on TONGA+ */
   if (family < CHIP_TONGA)
      return false;
   return phys_dev->rad_info.ip[AMD_IP_UVD].num_queues > 0;
}

struct radv_queue_ring_info {
   uint32_t scratch_size_per_wave;
   uint32_t scratch_waves;
   uint32_t compute_scratch_size_per_wave;
   uint32_t compute_scratch_waves;
   uint32_t esgs_ring_size;
   uint32_t gsvs_ring_size;
   uint32_t attr_ring_size;
   bool tess_rings;
   bool task_rings;
   bool mesh_scratch_ring;
   bool gds;
   bool gds_oa;
   bool sample_positions;
};

struct radv_queue_state {
   enum radv_queue_family qf;
   struct radv_queue_ring_info ring_info;

   struct radeon_winsys_bo *scratch_bo;
   struct radeon_winsys_bo *descriptor_bo;
   struct radeon_winsys_bo *compute_scratch_bo;
   struct radeon_winsys_bo *esgs_ring_bo;
   struct radeon_winsys_bo *gsvs_ring_bo;
   struct radeon_winsys_bo *tess_rings_bo;
   struct radeon_winsys_bo *task_rings_bo;
   struct radeon_winsys_bo *mesh_scratch_ring_bo;
   struct radeon_winsys_bo *attr_ring_bo;
   struct radeon_winsys_bo *gds_bo;
   struct radeon_winsys_bo *gds_oa_bo;

   struct radeon_cmdbuf *initial_preamble_cs;
   struct radeon_cmdbuf *initial_full_flush_preamble_cs;
   struct radeon_cmdbuf *continue_preamble_cs;
   struct radeon_cmdbuf *gang_wait_preamble_cs;
   struct radeon_cmdbuf *gang_wait_postamble_cs;

   /* the uses_shadow_regs here will be set only for general queue */
   bool uses_shadow_regs;
   /* register state is saved in shadowed_regs buffer */
   struct radeon_winsys_bo *shadowed_regs;
   /* shadow regs preamble ib. This will be the first preamble ib.
    * This ib has the packets to start register shadowing.
    */
   struct radeon_winsys_bo *shadow_regs_ib;
   uint32_t shadow_regs_ib_size_dw;
};

struct radv_queue {
   struct vk_queue vk;
   struct radv_device *device;
   struct radeon_winsys_ctx *hw_ctx;
   enum radeon_ctx_priority priority;
   struct radv_queue_state state;
   struct radv_queue_state *follower_state;
   struct radeon_winsys_bo *gang_sem_bo;

   uint64_t last_shader_upload_seq;
   bool sqtt_present;
};

int radv_queue_init(struct radv_device *device, struct radv_queue *queue, int idx,
                    const VkDeviceQueueCreateInfo *create_info,
                    const VkDeviceQueueGlobalPriorityCreateInfoKHR *global_priority);

void radv_queue_finish(struct radv_queue *queue);

enum radeon_ctx_priority radv_get_queue_global_priority(const VkDeviceQueueGlobalPriorityCreateInfoKHR *pObj);

#define RADV_BORDER_COLOR_COUNT       4096
#define RADV_BORDER_COLOR_BUFFER_SIZE (sizeof(VkClearColorValue) * RADV_BORDER_COLOR_COUNT)

struct radv_device_border_color_data {
   bool used[RADV_BORDER_COLOR_COUNT];

   struct radeon_winsys_bo *bo;
   VkClearColorValue *colors_gpu_ptr;

   /* Mutex is required to guarantee vkCreateSampler thread safety
    * given that we are writing to a buffer and checking color occupation */
   mtx_t mutex;
};

enum radv_force_vrs {
   RADV_FORCE_VRS_1x1 = 0,
   RADV_FORCE_VRS_2x2,
   RADV_FORCE_VRS_2x1,
   RADV_FORCE_VRS_1x2,
};

struct radv_notifier {
   int fd;
   int watch;
   bool quit;
   thrd_t thread;
};

struct radv_memory_trace_data {
   /* ID of the PTE update event in ftrace data */
   uint16_t ftrace_update_ptes_id;

   uint32_t num_cpus;
   int *pipe_fds;
};

struct radv_rra_accel_struct_data {
   VkEvent build_event;
   uint64_t va;
   uint64_t size;
   VkBuffer buffer;
   VkDeviceMemory memory;
   VkAccelerationStructureTypeKHR type;
   bool is_dead;
};

void radv_destroy_rra_accel_struct_data(VkDevice device, struct radv_rra_accel_struct_data *data);

struct radv_rra_trace_data {
   struct hash_table *accel_structs;
   struct hash_table_u64 *accel_struct_vas;
   simple_mtx_t data_mtx;
   bool validate_as;
   bool copy_after_build;
   uint32_t copy_memory_index;
};

enum radv_dispatch_table {
   RADV_DEVICE_DISPATCH_TABLE,
   RADV_APP_DISPATCH_TABLE,
   RADV_RGP_DISPATCH_TABLE,
   RADV_RRA_DISPATCH_TABLE,
   RADV_RMV_DISPATCH_TABLE,
   RADV_DISPATCH_TABLE_COUNT,
};

struct radv_layer_dispatch_tables {
   struct vk_device_dispatch_table app;
   struct vk_device_dispatch_table rgp;
   struct vk_device_dispatch_table rra;
   struct vk_device_dispatch_table rmv;
};

enum radv_buffer_robustness {
   RADV_BUFFER_ROBUSTNESS_DISABLED,
   RADV_BUFFER_ROBUSTNESS_1, /* robustBufferAccess */
   RADV_BUFFER_ROBUSTNESS_2, /* robustBufferAccess2 */
};

struct radv_sqtt_timestamp {
   uint8_t *map;
   unsigned offset;
   uint64_t size;
   struct radeon_winsys_bo *bo;
   struct list_head list;
};

struct radv_device_cache_key {
   uint32_t clear_lds : 1;
   uint32_t cs_wave32 : 1;
   uint32_t disable_aniso_single_level : 1;
   uint32_t disable_shrink_image_store : 1;
   uint32_t disable_sinking_load_input_fs : 1;
   uint32_t disable_trunc_coord : 1;
   uint32_t dual_color_blend_by_location : 1;
   uint32_t emulate_rt : 1;
   uint32_t ge_wave32 : 1;
   uint32_t image_2d_view_of_3d : 1;
   uint32_t invariant_geom : 1;
   uint32_t lower_discard_to_demote : 1;
   uint32_t mesh_shader_queries : 1;
   uint32_t no_fmask : 1;
   uint32_t no_rt : 1;
   uint32_t primitives_generated_query : 1;
   uint32_t ps_wave32 : 1;
   uint32_t rt_wave64 : 1;
   uint32_t split_fma : 1;
   uint32_t ssbo_non_uniform : 1;
   uint32_t tex_non_uniform : 1;
   uint32_t use_llvm : 1;
   uint32_t use_ngg : 1;
   uint32_t use_ngg_culling : 1;
};

struct radv_device {
   struct vk_device vk;

   struct radv_instance *instance;
   struct radeon_winsys *ws;

   struct radv_layer_dispatch_tables layer_dispatch;

   struct radeon_winsys_ctx *hw_ctx[RADV_NUM_HW_CTX];
   struct radv_meta_state meta_state;

   struct radv_queue *queues[RADV_MAX_QUEUE_FAMILIES];
   int queue_count[RADV_MAX_QUEUE_FAMILIES];

   bool pbb_allowed;
   uint32_t scratch_waves;
   uint32_t dispatch_initiator;
   uint32_t dispatch_initiator_task;

   /* MSAA sample locations.
    * The first index is the sample index.
    * The second index is the coordinate: X, Y. */
   float sample_locations_1x[1][2];
   float sample_locations_2x[2][2];
   float sample_locations_4x[4][2];
   float sample_locations_8x[8][2];

   /* GFX7 and later */
   uint32_t gfx_init_size_dw;
   struct radeon_winsys_bo *gfx_init;

   struct radeon_winsys_bo *trace_bo;
   uint32_t *trace_id_ptr;

   /* Whether to keep shader debug info, for debugging. */
   bool keep_shader_info;

   struct radv_physical_device *physical_device;

   /* Backup in-memory cache to be used if the app doesn't provide one */
   struct vk_pipeline_cache *mem_cache;

   /*
    * use different counters so MSAA MRTs get consecutive surface indices,
    * even if MASK is allocated in between.
    */
   uint32_t image_mrt_offset_counter;
   uint32_t fmask_mrt_offset_counter;

   struct list_head shader_arenas;
   struct hash_table_u64 *capture_replay_arena_vas;
   unsigned shader_arena_shift;
   uint8_t shader_free_list_mask;
   struct radv_shader_free_list shader_free_list;
   struct radv_shader_free_list capture_replay_free_list;
   struct list_head shader_block_obj_pool;
   mtx_t shader_arena_mutex;

   mtx_t shader_upload_hw_ctx_mutex;
   struct radeon_winsys_ctx *shader_upload_hw_ctx;
   VkSemaphore shader_upload_sem;
   uint64_t shader_upload_seq;
   struct list_head shader_dma_submissions;
   mtx_t shader_dma_submission_list_mutex;
   cnd_t shader_dma_submission_list_cond;

   /* Whether to DMA shaders to invisible VRAM or to upload directly through BAR. */
   bool shader_use_invisible_vram;

   /* Whether the app has enabled the robustBufferAccess/robustBufferAccess2 features. */
   enum radv_buffer_robustness buffer_robustness;

   /* Whether to inline the compute dispatch size in user sgprs. */
   bool load_grid_size_from_user_sgpr;

   /* Whether the driver uses a global BO list. */
   bool use_global_bo_list;

   /* Whether attachment VRS is enabled. */
   bool attachment_vrs_enabled;

   /* Whether shader image 32-bit float atomics are enabled. */
   bool image_float32_atomics;

   /* Whether 2D views of 3D image is enabled. */
   bool image_2d_view_of_3d;

   /* Whether primitives generated query features are enabled. */
   bool primitives_generated_query;

   /* Whether to use GS_FAST_LAUNCH(2) for mesh shaders. */
   bool mesh_fast_launch_2;

   /* Whether anisotropy is forced with RADV_TEX_ANISO (-1 is disabled). */
   int force_aniso;

   /* Always disable TRUNC_COORD. */
   bool disable_trunc_coord;

   struct radv_device_border_color_data border_color_data;

   /* Thread trace. */
   struct ac_sqtt sqtt;
   bool sqtt_enabled;
   bool sqtt_triggered;

   /* SQTT timestamps for queue events. */
   simple_mtx_t sqtt_timestamp_mtx;
   struct radv_sqtt_timestamp sqtt_timestamp;

   /* SQTT timed cmd buffers. */
   simple_mtx_t sqtt_command_pool_mtx;
   struct vk_command_pool *sqtt_command_pool[2];

   /* Memory trace. */
   struct radv_memory_trace_data memory_trace;

   /* SPM. */
   struct ac_spm spm;

   /* Radeon Raytracing Analyzer trace. */
   struct radv_rra_trace_data rra_trace;

   /* Trap handler. */
   struct radv_shader *trap_handler_shader;
   struct radeon_winsys_bo *tma_bo; /* Trap Memory Address */
   uint32_t *tma_ptr;

   /* Overallocation. */
   bool overallocation_disallowed;
   uint64_t allocated_memory_size[VK_MAX_MEMORY_HEAPS];
   mtx_t overallocation_mutex;

   /* RADV_FORCE_VRS. */
   struct radv_notifier notifier;
   enum radv_force_vrs force_vrs;

   /* Depth image for VRS when not bound by the app. */
   struct {
      struct radv_image *image;
      struct radv_buffer *buffer; /* HTILE */
      struct radv_device_memory *mem;
   } vrs;

   /* Prime blit sdma queue */
   struct radv_queue *private_sdma_queue;

   struct radv_shader_part_cache vs_prologs;
   struct radv_shader_part *simple_vs_prologs[MAX_VERTEX_ATTRIBS];
   struct radv_shader_part *instance_rate_vs_prologs[816];

   struct radv_shader_part_cache ps_epilogs;

   struct radv_shader_part_cache tcs_epilogs;

   simple_mtx_t trace_mtx;

   /* Whether per-vertex VRS is forced. */
   bool force_vrs_enabled;

   simple_mtx_t pstate_mtx;
   unsigned pstate_cnt;

   /* BO to contain some performance counter helpers:
    * - A lock for profiling cmdbuffers.
    * - a temporary fence for the end query synchronization.
    * - the pass to use for profiling. (as an array of bools)
    */
   struct radeon_winsys_bo *perf_counter_bo;

   /* Interleaved lock/unlock commandbuffers for perfcounter passes. */
   struct radeon_cmdbuf **perf_counter_lock_cs;

   bool uses_device_generated_commands;

   /* Whether smooth lines is enabled. */
   bool smooth_lines;

   /* Whether mesh shader queries are enabled. */
   bool mesh_shader_queries;

   bool uses_shadow_regs;

   struct hash_table *rt_handles;
   simple_mtx_t rt_handles_mtx;

   struct radv_device_cache_key cache_key;
   blake3_hash cache_hash;
};

bool radv_device_set_pstate(struct radv_device *device, bool enable);
bool radv_device_acquire_performance_counters(struct radv_device *device);
void radv_device_release_performance_counters(struct radv_device *device);

struct radv_device_memory {
   struct vk_object_base base;
   struct radeon_winsys_bo *bo;
   /* for dedicated allocations */
   struct radv_image *image;
   struct radv_buffer *buffer;
   uint32_t heap_index;
   uint64_t alloc_size;
   void *map;
   void *user_ptr;

#if RADV_SUPPORT_ANDROID_HARDWARE_BUFFER
   struct AHardwareBuffer *android_hardware_buffer;
#endif
};

void radv_device_memory_init(struct radv_device_memory *mem, struct radv_device *device, struct radeon_winsys_bo *bo);
void radv_device_memory_finish(struct radv_device_memory *mem);

struct radv_buffer {
   struct vk_buffer vk;

   /* Set when bound */
   struct radeon_winsys_bo *bo;
   VkDeviceSize offset;
};

void radv_buffer_init(struct radv_buffer *buffer, struct radv_device *device, struct radeon_winsys_bo *bo,
                      uint64_t size, uint64_t offset);
void radv_buffer_finish(struct radv_buffer *buffer);

enum radv_dynamic_state_bits {
   RADV_DYNAMIC_VIEWPORT = 1ull << 0,
   RADV_DYNAMIC_SCISSOR = 1ull << 1,
   RADV_DYNAMIC_LINE_WIDTH = 1ull << 2,
   RADV_DYNAMIC_DEPTH_BIAS = 1ull << 3,
   RADV_DYNAMIC_BLEND_CONSTANTS = 1ull << 4,
   RADV_DYNAMIC_DEPTH_BOUNDS = 1ull << 5,
   RADV_DYNAMIC_STENCIL_COMPARE_MASK = 1ull << 6,
   RADV_DYNAMIC_STENCIL_WRITE_MASK = 1ull << 7,
   RADV_DYNAMIC_STENCIL_REFERENCE = 1ull << 8,
   RADV_DYNAMIC_DISCARD_RECTANGLE = 1ull << 9,
   RADV_DYNAMIC_SAMPLE_LOCATIONS = 1ull << 10,
   RADV_DYNAMIC_LINE_STIPPLE = 1ull << 11,
   RADV_DYNAMIC_CULL_MODE = 1ull << 12,
   RADV_DYNAMIC_FRONT_FACE = 1ull << 13,
   RADV_DYNAMIC_PRIMITIVE_TOPOLOGY = 1ull << 14,
   RADV_DYNAMIC_DEPTH_TEST_ENABLE = 1ull << 15,
   RADV_DYNAMIC_DEPTH_WRITE_ENABLE = 1ull << 16,
   RADV_DYNAMIC_DEPTH_COMPARE_OP = 1ull << 17,
   RADV_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE = 1ull << 18,
   RADV_DYNAMIC_STENCIL_TEST_ENABLE = 1ull << 19,
   RADV_DYNAMIC_STENCIL_OP = 1ull << 20,
   RADV_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE = 1ull << 21,
   RADV_DYNAMIC_FRAGMENT_SHADING_RATE = 1ull << 22,
   RADV_DYNAMIC_PATCH_CONTROL_POINTS = 1ull << 23,
   RADV_DYNAMIC_RASTERIZER_DISCARD_ENABLE = 1ull << 24,
   RADV_DYNAMIC_DEPTH_BIAS_ENABLE = 1ull << 25,
   RADV_DYNAMIC_LOGIC_OP = 1ull << 26,
   RADV_DYNAMIC_PRIMITIVE_RESTART_ENABLE = 1ull << 27,
   RADV_DYNAMIC_COLOR_WRITE_ENABLE = 1ull << 28,
   RADV_DYNAMIC_VERTEX_INPUT = 1ull << 29,
   RADV_DYNAMIC_POLYGON_MODE = 1ull << 30,
   RADV_DYNAMIC_TESS_DOMAIN_ORIGIN = 1ull << 31,
   RADV_DYNAMIC_LOGIC_OP_ENABLE = 1ull << 32,
   RADV_DYNAMIC_LINE_STIPPLE_ENABLE = 1ull << 33,
   RADV_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE = 1ull << 34,
   RADV_DYNAMIC_SAMPLE_MASK = 1ull << 35,
   RADV_DYNAMIC_DEPTH_CLIP_ENABLE = 1ull << 36,
   RADV_DYNAMIC_CONSERVATIVE_RAST_MODE = 1ull << 37,
   RADV_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE = 1ull << 38,
   RADV_DYNAMIC_PROVOKING_VERTEX_MODE = 1ull << 39,
   RADV_DYNAMIC_DEPTH_CLAMP_ENABLE = 1ull << 40,
   RADV_DYNAMIC_COLOR_WRITE_MASK = 1ull << 41,
   RADV_DYNAMIC_COLOR_BLEND_ENABLE = 1ull << 42,
   RADV_DYNAMIC_RASTERIZATION_SAMPLES = 1ull << 43,
   RADV_DYNAMIC_LINE_RASTERIZATION_MODE = 1ull << 44,
   RADV_DYNAMIC_COLOR_BLEND_EQUATION = 1ull << 45,
   RADV_DYNAMIC_DISCARD_RECTANGLE_ENABLE = 1ull << 46,
   RADV_DYNAMIC_DISCARD_RECTANGLE_MODE = 1ull << 47,
   RADV_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE = 1ull << 48,
   RADV_DYNAMIC_SAMPLE_LOCATIONS_ENABLE = 1ull << 49,
   RADV_DYNAMIC_ALL = (1ull << 50) - 1,
};

enum radv_cmd_dirty_bits {
   /* Keep the dynamic state dirty bits in sync with
    * enum radv_dynamic_state_bits */
   RADV_CMD_DIRTY_DYNAMIC_VIEWPORT = 1ull << 0,
   RADV_CMD_DIRTY_DYNAMIC_SCISSOR = 1ull << 1,
   RADV_CMD_DIRTY_DYNAMIC_LINE_WIDTH = 1ull << 2,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_BIAS = 1ull << 3,
   RADV_CMD_DIRTY_DYNAMIC_BLEND_CONSTANTS = 1ull << 4,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_BOUNDS = 1ull << 5,
   RADV_CMD_DIRTY_DYNAMIC_STENCIL_COMPARE_MASK = 1ull << 6,
   RADV_CMD_DIRTY_DYNAMIC_STENCIL_WRITE_MASK = 1ull << 7,
   RADV_CMD_DIRTY_DYNAMIC_STENCIL_REFERENCE = 1ull << 8,
   RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE = 1ull << 9,
   RADV_CMD_DIRTY_DYNAMIC_SAMPLE_LOCATIONS = 1ull << 10,
   RADV_CMD_DIRTY_DYNAMIC_LINE_STIPPLE = 1ull << 11,
   RADV_CMD_DIRTY_DYNAMIC_CULL_MODE = 1ull << 12,
   RADV_CMD_DIRTY_DYNAMIC_FRONT_FACE = 1ull << 13,
   RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_TOPOLOGY = 1ull << 14,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_TEST_ENABLE = 1ull << 15,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_WRITE_ENABLE = 1ull << 16,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_COMPARE_OP = 1ull << 17,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE = 1ull << 18,
   RADV_CMD_DIRTY_DYNAMIC_STENCIL_TEST_ENABLE = 1ull << 19,
   RADV_CMD_DIRTY_DYNAMIC_STENCIL_OP = 1ull << 20,
   RADV_CMD_DIRTY_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE = 1ull << 21,
   RADV_CMD_DIRTY_DYNAMIC_FRAGMENT_SHADING_RATE = 1ull << 22,
   RADV_CMD_DIRTY_DYNAMIC_PATCH_CONTROL_POINTS = 1ull << 23,
   RADV_CMD_DIRTY_DYNAMIC_RASTERIZER_DISCARD_ENABLE = 1ull << 24,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_BIAS_ENABLE = 1ull << 25,
   RADV_CMD_DIRTY_DYNAMIC_LOGIC_OP = 1ull << 26,
   RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_RESTART_ENABLE = 1ull << 27,
   RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_ENABLE = 1ull << 28,
   RADV_CMD_DIRTY_DYNAMIC_VERTEX_INPUT = 1ull << 29,
   RADV_CMD_DIRTY_DYNAMIC_POLYGON_MODE = 1ull << 30,
   RADV_CMD_DIRTY_DYNAMIC_TESS_DOMAIN_ORIGIN = 1ull << 31,
   RADV_CMD_DIRTY_DYNAMIC_LOGIC_OP_ENABLE = 1ull << 32,
   RADV_CMD_DIRTY_DYNAMIC_LINE_STIPPLE_ENABLE = 1ull << 33,
   RADV_CMD_DIRTY_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE = 1ull << 34,
   RADV_CMD_DIRTY_DYNAMIC_SAMPLE_MASK = 1ull << 35,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_ENABLE = 1ull << 36,
   RADV_CMD_DIRTY_DYNAMIC_CONSERVATIVE_RAST_MODE = 1ull << 37,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE = 1ull << 38,
   RADV_CMD_DIRTY_DYNAMIC_PROVOKING_VERTEX_MODE = 1ull << 39,
   RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLAMP_ENABLE = 1ull << 40,
   RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_MASK = 1ull << 41,
   RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_ENABLE = 1ull << 42,
   RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES = 1ull << 43,
   RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE = 1ull << 44,
   RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_EQUATION = 1ull << 45,
   RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE_ENABLE = 1ull << 46,
   RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE_MODE = 1ull << 47,
   RADV_CMD_DIRTY_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE = 1ull << 48,
   RADV_CMD_DIRTY_DYNAMIC_SAMPLE_LOCATIONS_ENABLE = 1ull << 49,
   RADV_CMD_DIRTY_DYNAMIC_ALL = (1ull << 50) - 1,
   RADV_CMD_DIRTY_PIPELINE = 1ull << 50,
   RADV_CMD_DIRTY_INDEX_BUFFER = 1ull << 51,
   RADV_CMD_DIRTY_FRAMEBUFFER = 1ull << 52,
   RADV_CMD_DIRTY_VERTEX_BUFFER = 1ull << 53,
   RADV_CMD_DIRTY_STREAMOUT_BUFFER = 1ull << 54,
   RADV_CMD_DIRTY_GUARDBAND = 1ull << 55,
   RADV_CMD_DIRTY_RBPLUS = 1ull << 56,
   RADV_CMD_DIRTY_SHADER_QUERY = 1ull << 57,
   RADV_CMD_DIRTY_OCCLUSION_QUERY = 1ull << 58,
   RADV_CMD_DIRTY_DB_SHADER_CONTROL = 1ull << 59,
};

enum radv_cmd_flush_bits {
   /* Instruction cache. */
   RADV_CMD_FLAG_INV_ICACHE = 1 << 0,
   /* Scalar L1 cache. */
   RADV_CMD_FLAG_INV_SCACHE = 1 << 1,
   /* Vector L1 cache. */
   RADV_CMD_FLAG_INV_VCACHE = 1 << 2,
   /* L2 cache + L2 metadata cache writeback & invalidate.
    * GFX6-8: Used by shaders only. GFX9-10: Used by everything. */
   RADV_CMD_FLAG_INV_L2 = 1 << 3,
   /* L2 writeback (write dirty L2 lines to memory for non-L2 clients).
    * Only used for coherency with non-L2 clients like CB, DB, CP on GFX6-8.
    * GFX6-7 will do complete invalidation, because the writeback is unsupported. */
   RADV_CMD_FLAG_WB_L2 = 1 << 4,
   /* Invalidate the metadata cache. To be used when the DCC/HTILE metadata
    * changed and we want to read an image from shaders. */
   RADV_CMD_FLAG_INV_L2_METADATA = 1 << 5,
   /* Framebuffer caches */
   RADV_CMD_FLAG_FLUSH_AND_INV_CB_META = 1 << 6,
   RADV_CMD_FLAG_FLUSH_AND_INV_DB_META = 1 << 7,
   RADV_CMD_FLAG_FLUSH_AND_INV_DB = 1 << 8,
   RADV_CMD_FLAG_FLUSH_AND_INV_CB = 1 << 9,
   /* Engine synchronization. */
   RADV_CMD_FLAG_VS_PARTIAL_FLUSH = 1 << 10,
   RADV_CMD_FLAG_PS_PARTIAL_FLUSH = 1 << 11,
   RADV_CMD_FLAG_CS_PARTIAL_FLUSH = 1 << 12,
   RADV_CMD_FLAG_VGT_FLUSH = 1 << 13,
   /* Pipeline query controls. */
   RADV_CMD_FLAG_START_PIPELINE_STATS = 1 << 14,
   RADV_CMD_FLAG_STOP_PIPELINE_STATS = 1 << 15,
   RADV_CMD_FLAG_VGT_STREAMOUT_SYNC = 1 << 16,

   RADV_CMD_FLUSH_AND_INV_FRAMEBUFFER = (RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_CB_META |
                                         RADV_CMD_FLAG_FLUSH_AND_INV_DB | RADV_CMD_FLAG_FLUSH_AND_INV_DB_META),

   RADV_CMD_FLUSH_ALL_COMPUTE = (RADV_CMD_FLAG_INV_ICACHE | RADV_CMD_FLAG_INV_SCACHE | RADV_CMD_FLAG_INV_VCACHE |
                                 RADV_CMD_FLAG_INV_L2 | RADV_CMD_FLAG_WB_L2 | RADV_CMD_FLAG_CS_PARTIAL_FLUSH),
};

struct radv_vertex_binding {
   VkDeviceSize offset;
   VkDeviceSize size;
   VkDeviceSize stride;
};

struct radv_streamout_binding {
   struct radv_buffer *buffer;
   VkDeviceSize offset;
   VkDeviceSize size;
};

struct radv_streamout_state {
   /* Mask of bound streamout buffers. */
   uint8_t enabled_mask;

   /* State of VGT_STRMOUT_BUFFER_(CONFIG|END) */
   uint32_t hw_enabled_mask;

   /* State of VGT_STRMOUT_(CONFIG|EN) */
   bool streamout_enabled;
};

struct radv_sample_locations_state {
   VkSampleCountFlagBits per_pixel;
   VkExtent2D grid_size;
   uint32_t count;
   VkSampleLocationEXT locations[MAX_SAMPLE_LOCATIONS];
};

struct radv_dynamic_state {
   struct vk_dynamic_graphics_state vk;

   /**
    * Bitmask of (1ull << VK_DYNAMIC_STATE_*).
    * Defines the set of saved dynamic state.
    */
   uint64_t mask;

   struct {
      struct {
         float scale[3];
         float translate[3];
      } xform[MAX_VIEWPORTS];
   } hw_vp;

   struct radv_sample_locations_state sample_location;

   VkImageAspectFlags feedback_loop_aspects;
};

const char *radv_get_debug_option_name(int id);

const char *radv_get_perftest_option_name(int id);

struct radv_color_buffer_info {
   uint64_t cb_color_base;
   uint64_t cb_color_cmask;
   uint64_t cb_color_fmask;
   uint64_t cb_dcc_base;
   uint32_t cb_color_slice;
   uint32_t cb_color_view;
   uint32_t cb_color_info;
   uint32_t cb_color_attrib;
   uint32_t cb_color_attrib2; /* GFX9 and later */
   uint32_t cb_color_attrib3; /* GFX10 and later */
   uint32_t cb_dcc_control;
   uint32_t cb_color_cmask_slice;
   uint32_t cb_color_fmask_slice;
   union {
      uint32_t cb_color_pitch; // GFX6-GFX8
      uint32_t cb_mrt_epitch;  // GFX9+
   };
};

struct radv_ds_buffer_info {
   uint64_t db_z_read_base;
   uint64_t db_stencil_read_base;
   uint64_t db_z_write_base;
   uint64_t db_stencil_write_base;
   uint64_t db_htile_data_base;
   uint32_t db_depth_info;
   uint32_t db_z_info;
   uint32_t db_stencil_info;
   uint32_t db_depth_view;
   uint32_t db_depth_size;
   uint32_t db_depth_slice;
   uint32_t db_htile_surface;
   uint32_t db_z_info2;       /* GFX9 only */
   uint32_t db_stencil_info2; /* GFX9 only */
   uint32_t db_render_override2;
   uint32_t db_render_control;
};

void radv_initialise_color_surface(struct radv_device *device, struct radv_color_buffer_info *cb,
                                   struct radv_image_view *iview);
void radv_initialise_ds_surface(const struct radv_device *device, struct radv_ds_buffer_info *ds,
                                struct radv_image_view *iview, VkImageAspectFlags ds_aspects);
void radv_initialise_vrs_surface(struct radv_image *image, struct radv_buffer *htile_buffer,
                                 struct radv_ds_buffer_info *ds);

void radv_gfx11_set_db_render_control(const struct radv_device *device, unsigned num_samples,
                                      unsigned *db_render_control);
/**
 * Attachment state when recording a renderpass instance.
 *
 * The clear value is valid only if there exists a pending clear.
 */
struct radv_attachment {
   VkFormat format;
   struct radv_image_view *iview;
   VkImageLayout layout;
   VkImageLayout stencil_layout;

   union {
      struct radv_color_buffer_info cb;
      struct radv_ds_buffer_info ds;
   };

   struct radv_image_view *resolve_iview;
   VkResolveModeFlagBits resolve_mode;
   VkResolveModeFlagBits stencil_resolve_mode;
   VkImageLayout resolve_layout;
   VkImageLayout stencil_resolve_layout;
};

struct radv_rendering_state {
   bool active;
   bool has_image_views;
   VkRect2D area;
   uint32_t layer_count;
   uint32_t view_mask;
   uint32_t color_samples;
   uint32_t ds_samples;
   uint32_t max_samples;
   struct radv_sample_locations_state sample_locations;
   uint32_t color_att_count;
   struct radv_attachment color_att[MAX_RTS];
   struct radv_attachment ds_att;
   VkImageAspectFlags ds_att_aspects;
   struct radv_attachment vrs_att;
   VkExtent2D vrs_texel_size;
};

struct radv_descriptor_state {
   struct radv_descriptor_set *sets[MAX_SETS];
   uint32_t dirty;
   uint32_t valid;
   struct radv_push_descriptor_set push_set;
   uint32_t dynamic_buffers[4 * MAX_DYNAMIC_BUFFERS];
   uint64_t descriptor_buffers[MAX_SETS];
   bool need_indirect_descriptor_sets;
};

struct radv_push_constant_state {
   uint32_t size;
   uint32_t dynamic_offset_count;
};

enum rgp_flush_bits {
   RGP_FLUSH_WAIT_ON_EOP_TS = 0x1,
   RGP_FLUSH_VS_PARTIAL_FLUSH = 0x2,
   RGP_FLUSH_PS_PARTIAL_FLUSH = 0x4,
   RGP_FLUSH_CS_PARTIAL_FLUSH = 0x8,
   RGP_FLUSH_PFP_SYNC_ME = 0x10,
   RGP_FLUSH_SYNC_CP_DMA = 0x20,
   RGP_FLUSH_INVAL_VMEM_L0 = 0x40,
   RGP_FLUSH_INVAL_ICACHE = 0x80,
   RGP_FLUSH_INVAL_SMEM_L0 = 0x100,
   RGP_FLUSH_FLUSH_L2 = 0x200,
   RGP_FLUSH_INVAL_L2 = 0x400,
   RGP_FLUSH_FLUSH_CB = 0x800,
   RGP_FLUSH_INVAL_CB = 0x1000,
   RGP_FLUSH_FLUSH_DB = 0x2000,
   RGP_FLUSH_INVAL_DB = 0x4000,
   RGP_FLUSH_INVAL_L1 = 0x8000,
};

struct radv_multisample_state {
   bool sample_shading_enable;
   float min_sample_shading;
};

struct radv_ia_multi_vgt_param_helpers {
   uint32_t base;
   bool partial_es_wave;
   bool ia_switch_on_eoi;
   bool partial_vs_wave;
};

struct radv_cmd_state {
   /* Vertex descriptors */
   uint64_t vb_va;
   unsigned vb_size;

   bool predicating;
   uint64_t dirty;

   VkShaderStageFlags active_stages;
   struct radv_shader *shaders[MESA_VULKAN_SHADER_STAGES];
   struct radv_shader *gs_copy_shader;
   struct radv_shader *last_vgt_shader;
   struct radv_shader *rt_prolog;

   uint32_t prefetch_L2_mask;

   struct radv_graphics_pipeline *graphics_pipeline;
   struct radv_graphics_pipeline *emitted_graphics_pipeline;
   struct radv_compute_pipeline *compute_pipeline;
   struct radv_compute_pipeline *emitted_compute_pipeline;
   struct radv_ray_tracing_pipeline *rt_pipeline; /* emitted = emitted_compute_pipeline */
   struct radv_dynamic_state dynamic;
   struct radv_vs_input_state dynamic_vs_input;
   struct radv_streamout_state streamout;

   struct radv_rendering_state render;

   /* Index buffer */
   uint32_t index_type;
   uint32_t max_index_count;
   uint64_t index_va;
   int32_t last_index_type;

   uint32_t last_primitive_reset_index; /* only relevant on GFX6-7 */
   enum radv_cmd_flush_bits flush_bits;
   unsigned active_occlusion_queries;
   bool perfect_occlusion_queries_enabled;
   unsigned active_pipeline_queries;
   unsigned active_pipeline_gds_queries;
   unsigned active_pipeline_ace_queries; /* Task shader invocations query */
   unsigned active_prims_gen_queries;
   unsigned active_prims_xfb_queries;
   unsigned active_prims_gen_gds_queries;
   unsigned active_prims_xfb_gds_queries;
   uint32_t trace_id;
   uint32_t last_ia_multi_vgt_param;
   uint32_t last_ge_cntl;

   uint32_t last_num_instances;
   uint32_t last_first_instance;
   bool last_vertex_offset_valid;
   uint32_t last_vertex_offset;
   uint32_t last_drawid;
   uint32_t last_subpass_color_count;

   uint32_t last_sx_ps_downconvert;
   uint32_t last_sx_blend_opt_epsilon;
   uint32_t last_sx_blend_opt_control;

   uint32_t last_db_count_control;

   uint32_t last_db_shader_control;

   /* Whether CP DMA is busy/idle. */
   bool dma_is_busy;

   /* Whether any images that are not L2 coherent are dirty from the CB. */
   bool rb_noncoherent_dirty;

   /* Conditional rendering info. */
   uint8_t predication_op; /* 32-bit or 64-bit predicate value */
   int predication_type;   /* -1: disabled, 0: normal, 1: inverted */
   uint64_t predication_va;

   /* Inheritance info. */
   VkQueryPipelineStatisticFlags inherited_pipeline_statistics;
   bool inherited_occlusion_queries;
   VkQueryControlFlags inherited_query_control_flags;

   bool context_roll_without_scissor_emitted;

   /* SQTT related state. */
   uint32_t current_event_type;
   uint32_t num_events;
   uint32_t num_layout_transitions;
   bool in_barrier;
   bool pending_sqtt_barrier_end;
   enum rgp_flush_bits sqtt_flush_bits;

   /* NGG culling state. */
   bool has_nggc;

   /* Mesh shading state. */
   bool mesh_shading;

   uint8_t cb_mip[MAX_RTS];
   uint8_t ds_mip;

   /* Whether DRAW_{INDEX}_INDIRECT_{MULTI} is emitted. */
   bool uses_draw_indirect;

   uint32_t rt_stack_size;

   struct radv_shader_part *emitted_vs_prolog;
   uint32_t vbo_misaligned_mask;
   uint32_t vbo_misaligned_mask_invalid;
   uint32_t vbo_bound_mask;

   struct radv_shader_part *emitted_tcs_epilog;
   struct radv_shader_part *emitted_ps_epilog;

   /* Per-vertex VRS state. */
   uint32_t last_vrs_rates;
   int8_t last_vrs_rates_sgpr_idx;

   /* Whether to suspend streamout for internal driver operations. */
   bool suspend_streamout;

   /* Whether this commandbuffer uses performance counters. */
   bool uses_perf_counters;

   struct radv_ia_multi_vgt_param_helpers ia_multi_vgt_param;

   /* Tessellation info when patch control points is dynamic. */
   unsigned tess_num_patches;
   unsigned tess_lds_size;

   unsigned col_format_non_compacted;

   /* Binning state */
   unsigned last_pa_sc_binner_cntl_0;

   struct radv_multisample_state ms;

   /* Custom blend mode for internal operations. */
   unsigned custom_blend_mode;
   unsigned db_render_control;

   unsigned rast_prim;

   uint32_t vtx_base_sgpr;
   uint8_t vtx_emit_num;
   bool uses_drawid;
   bool uses_baseinstance;

   bool uses_out_of_order_rast;
   bool uses_vrs_attachment;
   bool uses_dynamic_patch_control_points;
   bool uses_dynamic_vertex_binding_stride;
};

struct radv_cmd_buffer_upload {
   uint8_t *map;
   unsigned offset;
   uint64_t size;
   struct radeon_winsys_bo *upload_bo;
   struct list_head list;
};

struct radv_cmd_buffer {
   struct vk_command_buffer vk;

   struct radv_device *device;

   VkCommandBufferUsageFlags usage_flags;
   struct radeon_cmdbuf *cs;
   struct radv_cmd_state state;
   struct radv_buffer *vertex_binding_buffers[MAX_VBS];
   struct radv_vertex_binding vertex_bindings[MAX_VBS];
   uint32_t used_vertex_bindings;
   struct radv_streamout_binding streamout_bindings[MAX_SO_BUFFERS];
   enum radv_queue_family qf;

   uint8_t push_constants[MAX_PUSH_CONSTANTS_SIZE];
   VkShaderStageFlags push_constant_stages;
   struct radv_descriptor_set_header meta_push_descriptors;

   struct radv_descriptor_state descriptors[MAX_BIND_POINTS];

   struct radv_push_constant_state push_constant_state[MAX_BIND_POINTS];

   uint64_t descriptor_buffers[MAX_SETS];

   struct radv_cmd_buffer_upload upload;

   uint32_t scratch_size_per_wave_needed;
   uint32_t scratch_waves_wanted;
   uint32_t compute_scratch_size_per_wave_needed;
   uint32_t compute_scratch_waves_wanted;
   uint32_t esgs_ring_size_needed;
   uint32_t gsvs_ring_size_needed;
   bool tess_rings_needed;
   bool task_rings_needed;
   bool mesh_scratch_ring_needed;
   bool gds_needed;    /* for GFX10 streamout and NGG GS queries */
   bool gds_oa_needed; /* for GFX10 streamout */
   bool sample_positions_needed;

   uint64_t gfx9_fence_va;
   uint32_t gfx9_fence_idx;
   uint64_t gfx9_eop_bug_va;

   uint64_t mec_inv_pred_va;  /* For inverted predication when using MEC. */
   bool mec_inv_pred_emitted; /* To ensure we don't have to repeat inverting the VA. */

   struct set vs_prologs;
   struct set ps_epilogs;
   struct set tcs_epilogs;

   /**
    * Gang state.
    * Used when the command buffer needs work done on a different queue
    * (eg. when a graphics command buffer needs compute work).
    * Currently only one follower is possible per command buffer.
    */
   struct {
      /** Follower command stream. */
      struct radeon_cmdbuf *cs;

      /** Flush bits for the follower cmdbuf. */
      enum radv_cmd_flush_bits flush_bits;

      /**
       * For synchronization between the follower and leader.
       * The value of these semaphores are incremented whenever we
       * encounter a barrier that affects the follower.
       *
       * DWORD 0: Leader to follower semaphore.
       *          The leader writes the value and the follower waits.
       * DWORD 1: Follower to leader semaphore.
       *          The follower writes the value, and the leader waits.
       */
      struct {
         uint64_t va;                     /* Virtual address of the semaphore. */
         uint32_t leader_value;           /* Current value of the leader. */
         uint32_t emitted_leader_value;   /* Last value emitted by the leader. */
         uint32_t follower_value;         /* Current value of the follower. */
         uint32_t emitted_follower_value; /* Last value emitted by the follower. */
      } sem;
   } gang;

   /**
    * Whether a query pool has been reset and we have to flush caches.
    */
   bool pending_reset_query;

   /**
    * Bitmask of pending active query flushes.
    */
   enum radv_cmd_flush_bits active_query_flush_bits;

   struct {
      struct radv_video_session *vid;
      struct radv_video_session_params *params;
      struct rvcn_sq_var sq;
      struct rvcn_decode_buffer_s *decode_buffer;
   } video;

   struct {
      /* Temporary space for some transfer queue copy command workarounds. */
      struct radeon_winsys_bo *copy_temp;
   } transfer;

   uint64_t shader_upload_seq;

   uint32_t sqtt_cb_id;
};

static inline bool
radv_cmdbuf_has_stage(const struct radv_cmd_buffer *cmd_buffer, gl_shader_stage stage)
{
   return !!(cmd_buffer->state.active_stages & mesa_to_vk_shader_stage(stage));
}

static inline uint32_t
radv_get_num_pipeline_stat_queries(struct radv_cmd_buffer *cmd_buffer)
{
   /* SAMPLE_STREAMOUTSTATS also requires PIPELINESTAT_START to be enabled. */
   return cmd_buffer->state.active_pipeline_queries + cmd_buffer->state.active_prims_gen_queries +
          cmd_buffer->state.active_prims_xfb_queries;
}

extern const struct vk_command_buffer_ops radv_cmd_buffer_ops;

struct radv_dispatch_info {
   /**
    * Determine the layout of the grid (in block units) to be used.
    */
   uint32_t blocks[3];

   /**
    * A starting offset for the grid. If unaligned is set, the offset
    * must still be aligned.
    */
   uint32_t offsets[3];

   /**
    * Whether it's an unaligned compute dispatch.
    */
   bool unaligned;

   /**
    * Whether waves must be launched in order.
    */
   bool ordered;

   /**
    * Indirect compute parameters resource.
    */
   struct radeon_winsys_bo *indirect;
   uint64_t va;
};

void radv_compute_dispatch(struct radv_cmd_buffer *cmd_buffer, const struct radv_dispatch_info *info);

struct radv_image;
struct radv_image_view;

bool radv_cmd_buffer_uses_mec(struct radv_cmd_buffer *cmd_buffer);

void radv_emit_streamout_enable(struct radv_cmd_buffer *cmd_buffer);

void radv_emit_graphics(struct radv_device *device, struct radeon_cmdbuf *cs);
void radv_emit_compute(struct radv_device *device, struct radeon_cmdbuf *cs);

void radv_create_gfx_config(struct radv_device *device);

void radv_write_scissors(struct radeon_cmdbuf *cs, int count, const VkRect2D *scissors, const VkViewport *viewports);

void radv_write_guardband(struct radeon_cmdbuf *cs, int count, const VkViewport *viewports, unsigned rast_prim,
                          unsigned polygon_mode, float line_width);

VkResult radv_create_shadow_regs_preamble(const struct radv_device *device, struct radv_queue_state *queue_state);
void radv_destroy_shadow_regs_preamble(struct radv_queue_state *queue_state, struct radeon_winsys *ws);
void radv_emit_shadow_regs_preamble(struct radeon_cmdbuf *cs, const struct radv_device *device,
                                    struct radv_queue_state *queue_state);
VkResult radv_init_shadowed_regs_buffer_state(const struct radv_device *device, struct radv_queue *queue);

uint32_t radv_get_ia_multi_vgt_param(struct radv_cmd_buffer *cmd_buffer, bool instanced_draw, bool indirect_draw,
                                     bool count_from_stream_output, uint32_t draw_vertex_count, unsigned topology,
                                     bool prim_restart_enable, unsigned patch_control_points,
                                     unsigned num_tess_patches);
void radv_cs_emit_write_event_eop(struct radeon_cmdbuf *cs, enum amd_gfx_level gfx_level, enum radv_queue_family qf,
                                  unsigned event, unsigned event_flags, unsigned dst_sel, unsigned data_sel,
                                  uint64_t va, uint32_t new_fence, uint64_t gfx9_eop_bug_va);

struct radv_vgt_shader_key {
   uint8_t tess : 1;
   uint8_t gs : 1;
   uint8_t mesh_scratch_ring : 1;
   uint8_t mesh : 1;
   uint8_t ngg_passthrough : 1;
   uint8_t ngg : 1;       /* gfx10+ */
   uint8_t streamout : 1; /* only used with NGG */
   uint8_t hs_wave32 : 1;
   uint8_t gs_wave32 : 1;
   uint8_t vs_wave32 : 1;
};

void radv_cs_emit_cache_flush(struct radeon_winsys *ws, struct radeon_cmdbuf *cs, enum amd_gfx_level gfx_level,
                              uint32_t *flush_cnt, uint64_t flush_va, enum radv_queue_family qf,
                              enum radv_cmd_flush_bits flush_bits, enum rgp_flush_bits *sqtt_flush_bits,
                              uint64_t gfx9_eop_bug_va);
void radv_emit_cache_flush(struct radv_cmd_buffer *cmd_buffer);
void radv_emit_set_predication_state(struct radv_cmd_buffer *cmd_buffer, bool draw_visible, unsigned pred_op,
                                     uint64_t va);
void radv_cp_dma_buffer_copy(struct radv_cmd_buffer *cmd_buffer, uint64_t src_va, uint64_t dest_va, uint64_t size);
void radv_cs_cp_dma_prefetch(const struct radv_device *device, struct radeon_cmdbuf *cs, uint64_t va, unsigned size,
                             bool predicating);
void radv_cp_dma_prefetch(struct radv_cmd_buffer *cmd_buffer, uint64_t va, unsigned size);
void radv_cp_dma_clear_buffer(struct radv_cmd_buffer *cmd_buffer, uint64_t va, uint64_t size, unsigned value);
void radv_cp_dma_wait_for_idle(struct radv_cmd_buffer *cmd_buffer);

uint32_t radv_get_vgt_index_size(uint32_t type);

unsigned radv_instance_rate_prolog_index(unsigned num_attributes, uint32_t instance_rate_inputs);

struct radv_ps_epilog_state {
   uint8_t color_attachment_count;
   VkFormat color_attachment_formats[MAX_RTS];

   uint32_t color_write_mask;
   uint32_t color_blend_enable;

   uint32_t colors_written;
   bool mrt0_is_dual_src;
   bool export_depth;
   bool export_stencil;
   bool export_sample_mask;
   bool alpha_to_coverage_via_mrtz;
   uint8_t need_src_alpha;
};

struct radv_ps_epilog_key radv_generate_ps_epilog_key(const struct radv_device *device,
                                                      const struct radv_ps_epilog_state *state);

bool radv_needs_null_export_workaround(const struct radv_device *device, const struct radv_shader *ps,
                                       unsigned custom_blend_mode);

void radv_cmd_buffer_reset_rendering(struct radv_cmd_buffer *cmd_buffer);
bool radv_cmd_buffer_upload_alloc_aligned(struct radv_cmd_buffer *cmd_buffer, unsigned size, unsigned alignment,
                                          unsigned *out_offset, void **ptr);
bool radv_cmd_buffer_upload_alloc(struct radv_cmd_buffer *cmd_buffer, unsigned size, unsigned *out_offset, void **ptr);
bool radv_cmd_buffer_upload_data(struct radv_cmd_buffer *cmd_buffer, unsigned size, const void *data,
                                 unsigned *out_offset);
void radv_write_vertex_descriptors(const struct radv_cmd_buffer *cmd_buffer,
                                   const struct radv_graphics_pipeline *pipeline, bool full_null_descriptors,
                                   void *vb_ptr);

void radv_emit_default_sample_locations(struct radeon_cmdbuf *cs, int nr_samples);
unsigned radv_get_default_max_sample_dist(int log_samples);
void radv_device_init_msaa(struct radv_device *device);
VkResult radv_device_init_vrs_state(struct radv_device *device);

void radv_cs_write_data_imm(struct radeon_cmdbuf *cs, unsigned engine_sel, uint64_t va, uint32_t imm);

void radv_update_ds_clear_metadata(struct radv_cmd_buffer *cmd_buffer, const struct radv_image_view *iview,
                                   VkClearDepthStencilValue ds_clear_value, VkImageAspectFlags aspects);

void radv_update_color_clear_metadata(struct radv_cmd_buffer *cmd_buffer, const struct radv_image_view *iview,
                                      int cb_idx, uint32_t color_values[2]);

void radv_set_mutable_tex_desc_fields(struct radv_device *device, struct radv_image *image,
                                      const struct legacy_surf_level *base_level_info, unsigned plane_id,
                                      unsigned base_level, unsigned first_level, unsigned block_width, bool is_stencil,
                                      bool is_storage_image, bool disable_compression, bool enable_write_compression,
                                      uint32_t *state, const struct ac_surf_nbc_view *nbc_view);

void radv_make_texture_descriptor(struct radv_device *device, struct radv_image *image, bool is_storage_image,
                                  VkImageViewType view_type, VkFormat vk_format, const VkComponentMapping *mapping,
                                  unsigned first_level, unsigned last_level, unsigned first_layer, unsigned last_layer,
                                  unsigned width, unsigned height, unsigned depth, float min_lod, uint32_t *state,
                                  uint32_t *fmask_state, VkImageCreateFlags img_create_flags,
                                  const struct ac_surf_nbc_view *nbc_view,
                                  const VkImageViewSlicedCreateInfoEXT *sliced_3d);

bool radv_image_use_dcc_image_stores(const struct radv_device *device, const struct radv_image *image);
bool radv_image_use_dcc_predication(const struct radv_device *device, const struct radv_image *image);

bool radv_image_can_fast_clear(const struct radv_device *device, const struct radv_image *image);

unsigned radv_plane_from_aspect(VkImageAspectFlags mask);

void radv_update_fce_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                              const VkImageSubresourceRange *range, bool value);

void radv_update_dcc_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                              const VkImageSubresourceRange *range, bool value);
enum radv_cmd_flush_bits radv_src_access_flush(struct radv_cmd_buffer *cmd_buffer, VkAccessFlags2 src_flags,
                                               const struct radv_image *image);
enum radv_cmd_flush_bits radv_dst_access_flush(struct radv_cmd_buffer *cmd_buffer, VkAccessFlags2 dst_flags,
                                               const struct radv_image *image);

void radv_write_timestamp(struct radv_cmd_buffer *cmd_buffer, uint64_t va, VkPipelineStageFlags2 stage);

void radv_cmd_buffer_trace_emit(struct radv_cmd_buffer *cmd_buffer);
bool radv_get_memory_fd(struct radv_device *device, struct radv_device_memory *memory, int *pFD);
void radv_free_memory(struct radv_device *device, const VkAllocationCallbacks *pAllocator,
                      struct radv_device_memory *mem);

static inline void
radv_emit_shader_pointer_head(struct radeon_cmdbuf *cs, unsigned sh_offset, unsigned pointer_count,
                              bool use_32bit_pointers)
{
   radeon_emit(cs, PKT3(PKT3_SET_SH_REG, pointer_count * (use_32bit_pointers ? 1 : 2), 0));
   radeon_emit(cs, (sh_offset - SI_SH_REG_OFFSET) >> 2);
}

static inline void
radv_emit_shader_pointer_body(struct radv_device *device, struct radeon_cmdbuf *cs, uint64_t va,
                              bool use_32bit_pointers)
{
   radeon_emit(cs, va);

   if (use_32bit_pointers) {
      assert(va == 0 || (va >> 32) == device->physical_device->rad_info.address32_hi);
   } else {
      radeon_emit(cs, va >> 32);
   }
}

static inline void
radv_emit_shader_pointer(struct radv_device *device, struct radeon_cmdbuf *cs, uint32_t sh_offset, uint64_t va,
                         bool global)
{
   bool use_32bit_pointers = !global;

   radv_emit_shader_pointer_head(cs, sh_offset, 1, use_32bit_pointers);
   radv_emit_shader_pointer_body(device, cs, va, use_32bit_pointers);
}

static inline unsigned
vk_to_bind_point(VkPipelineBindPoint bind_point)
{
   return bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR ? 2 : bind_point;
}

static inline struct radv_descriptor_state *
radv_get_descriptors_state(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point)
{
   return &cmd_buffer->descriptors[vk_to_bind_point(bind_point)];
}

static inline const struct radv_push_constant_state *
radv_get_push_constants_state(const struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point)
{
   return &cmd_buffer->push_constant_state[vk_to_bind_point(bind_point)];
}

void radv_get_viewport_xform(const VkViewport *viewport, float scale[3], float translate[3]);

/*
 * Takes x,y,z as exact numbers of invocations, instead of blocks.
 *
 * Limitations: Can't call normal dispatch functions without binding or rebinding
 *              the compute pipeline.
 */
void radv_unaligned_dispatch(struct radv_cmd_buffer *cmd_buffer, uint32_t x, uint32_t y, uint32_t z);

void radv_indirect_dispatch(struct radv_cmd_buffer *cmd_buffer, struct radeon_winsys_bo *bo, uint64_t va);

struct radv_event {
   struct vk_object_base base;
   struct radeon_winsys_bo *bo;
   uint64_t *map;
};

struct radv_pipeline_key;
struct radv_ray_tracing_group;

void radv_pipeline_stage_init(const VkPipelineShaderStageCreateInfo *sinfo, const struct radv_pipeline_layout *layout,
                              struct radv_shader_stage *out_stage);

void radv_hash_shaders(const struct radv_device *device, unsigned char *hash, const struct radv_shader_stage *stages,
                       uint32_t stage_count, const struct radv_pipeline_layout *layout,
                       const struct radv_pipeline_key *key);

void radv_hash_rt_stages(struct mesa_sha1 *ctx, const VkPipelineShaderStageCreateInfo *stages, unsigned stage_count);

void radv_hash_rt_shaders(const struct radv_device *device, unsigned char *hash,
                          const VkRayTracingPipelineCreateInfoKHR *pCreateInfo, const struct radv_pipeline_key *key,
                          const struct radv_ray_tracing_group *groups);

bool radv_enable_rt(const struct radv_physical_device *pdevice, bool rt_pipelines);

bool radv_emulate_rt(const struct radv_physical_device *pdevice);

struct radv_prim_vertex_count {
   uint8_t min;
   uint8_t incr;
};

enum radv_pipeline_type {
   RADV_PIPELINE_GRAPHICS,
   RADV_PIPELINE_GRAPHICS_LIB,
   /* Compute pipeline */
   RADV_PIPELINE_COMPUTE,
   /* Raytracing pipeline */
   RADV_PIPELINE_RAY_TRACING,
};

struct radv_pipeline_group_handle {
   uint64_t recursive_shader_ptr;

   union {
      uint32_t general_index;
      uint32_t closest_hit_index;
   };
   union {
      uint32_t intersection_index;
      uint32_t any_hit_index;
   };
};

struct radv_rt_capture_replay_handle {
   struct radv_serialized_shader_arena_block recursive_shader_alloc;
   uint32_t non_recursive_idx;
};

struct radv_pipeline {
   struct vk_object_base base;
   enum radv_pipeline_type type;

   VkPipelineCreateFlags2KHR create_flags;

   struct vk_pipeline_cache_object *cache_object;

   bool is_internal;
   bool need_indirect_descriptor_sets;
   struct radv_shader *shaders[MESA_VULKAN_SHADER_STAGES];
   struct radv_shader *gs_copy_shader;

   uint64_t shader_upload_seq;

   struct radeon_cmdbuf cs;
   uint32_t ctx_cs_hash;
   struct radeon_cmdbuf ctx_cs;

   uint32_t user_data_0[MESA_VULKAN_SHADER_STAGES];

   /* Unique pipeline hash identifier. */
   uint64_t pipeline_hash;

   /* Pipeline layout info. */
   uint32_t push_constant_size;
   uint32_t dynamic_offset_count;
};

struct radv_sqtt_shaders_reloc {
   struct radeon_winsys_bo *bo;
   union radv_shader_arena_block *alloc;
   uint64_t va[MESA_VULKAN_SHADER_STAGES];
};

struct radv_graphics_pipeline {
   struct radv_pipeline base;

   bool uses_drawid;
   bool uses_baseinstance;

   /* Whether the pipeline forces per-vertex VRS (GFX10.3+). */
   bool force_vrs_per_vertex;

   /* Whether the pipeline uses NGG (GFX10+). */
   bool is_ngg;
   bool has_ngg_culling;

   uint8_t vtx_emit_num;

   uint32_t vtx_base_sgpr;
   uint64_t dynamic_states;
   uint64_t needed_dynamic_state;

   VkShaderStageFlags active_stages;

   /* Used for rbplus */
   uint32_t col_format_non_compacted;

   struct radv_dynamic_state dynamic_state;

   struct radv_vs_input_state vs_input_state;

   struct radv_multisample_state ms;
   struct radv_ia_multi_vgt_param_helpers ia_multi_vgt_param;
   uint32_t binding_stride[MAX_VBS];
   uint8_t attrib_bindings[MAX_VERTEX_ATTRIBS];
   uint32_t attrib_ends[MAX_VERTEX_ATTRIBS];
   uint32_t attrib_index_offset[MAX_VERTEX_ATTRIBS];
   uint32_t db_render_control;

   /* Last pre-PS API stage */
   gl_shader_stage last_vgt_api_stage;

   /* Not NULL if graphics pipeline uses streamout. */
   struct radv_shader *streamout_shader;

   unsigned rast_prim;

   /* For vk_graphics_pipeline_state */
   void *state_data;

   /* Custom blend mode for internal operations. */
   unsigned custom_blend_mode;

   /* Whether the pipeline uses out-of-order rasterization. */
   bool uses_out_of_order_rast;

   /* Whether the pipeline uses a VRS attachment. */
   bool uses_vrs_attachment;

   /* For graphics pipeline library */
   bool retain_shaders;

   /* For relocation of shaders with RGP. */
   struct radv_sqtt_shaders_reloc *sqtt_shaders_reloc;
};

struct radv_compute_pipeline {
   struct radv_pipeline base;
};

struct radv_ray_tracing_group {
   VkRayTracingShaderGroupTypeKHR type;
   uint32_t recursive_shader; /* generalShader or closestHitShader */
   uint32_t any_hit_shader;
   uint32_t intersection_shader;
   struct radv_pipeline_group_handle handle;
};

struct radv_ray_tracing_stage {
   struct vk_pipeline_cache_object *nir;
   struct radv_shader *shader;
   gl_shader_stage stage;
   uint32_t stack_size;

   bool can_inline;

   uint8_t sha1[SHA1_DIGEST_LENGTH];
};

struct radv_ray_tracing_pipeline {
   struct radv_compute_pipeline base;

   struct radv_shader *prolog;

   struct radv_ray_tracing_stage *stages;
   struct radv_ray_tracing_group *groups;
   unsigned stage_count;
   unsigned non_imported_stage_count;
   unsigned group_count;

   uint8_t sha1[SHA1_DIGEST_LENGTH];
   uint32_t stack_size;

   /* set if any shaders from this pipeline require robustness2 in the merged traversal shader */
   bool traversal_storage_robustness2 : 1;
   bool traversal_uniform_robustness2 : 1;
};

struct radv_retained_shaders {
   struct {
      void *serialized_nir;
      size_t serialized_nir_size;
      unsigned char shader_sha1[SHA1_DIGEST_LENGTH];
   } stages[MESA_VULKAN_SHADER_STAGES];
};

struct radv_graphics_lib_pipeline {
   struct radv_graphics_pipeline base;

   struct radv_pipeline_layout layout;

   struct vk_graphics_pipeline_state graphics_state;

   VkGraphicsPipelineLibraryFlagsEXT lib_flags;

   struct radv_retained_shaders retained_shaders;

   void *mem_ctx;

   unsigned stage_count;
   VkPipelineShaderStageCreateInfo *stages;
};

#define RADV_DECL_PIPELINE_DOWNCAST(pipe_type, pipe_enum)                                                              \
   static inline struct radv_##pipe_type##_pipeline *radv_pipeline_to_##pipe_type(struct radv_pipeline *pipeline)      \
   {                                                                                                                   \
      assert(pipeline->type == pipe_enum);                                                                             \
      return (struct radv_##pipe_type##_pipeline *)pipeline;                                                           \
   }

RADV_DECL_PIPELINE_DOWNCAST(graphics, RADV_PIPELINE_GRAPHICS)
RADV_DECL_PIPELINE_DOWNCAST(graphics_lib, RADV_PIPELINE_GRAPHICS_LIB)
RADV_DECL_PIPELINE_DOWNCAST(compute, RADV_PIPELINE_COMPUTE)
RADV_DECL_PIPELINE_DOWNCAST(ray_tracing, RADV_PIPELINE_RAY_TRACING)

struct radv_shader_layout {
   uint32_t num_sets;

   struct {
      struct radv_descriptor_set_layout *layout;
      uint32_t dynamic_offset_start;
   } set[MAX_SETS];

   uint32_t push_constant_size;
   bool use_dynamic_descriptors;
};

struct radv_shader_stage {
   gl_shader_stage stage;

   struct {
      const struct vk_object_base *object;
      const char *data;
      uint32_t size;
   } spirv;

   const char *entrypoint;
   const VkSpecializationInfo *spec_info;

   unsigned char shader_sha1[20];

   nir_shader *nir;
   nir_shader *internal_nir; /* meta shaders */

   struct radv_shader_info info;
   struct radv_shader_args args;

   VkPipelineCreationFeedback feedback;

   struct radv_shader_layout layout;
};

void radv_shader_layout_init(const struct radv_pipeline_layout *pipeline_layout, gl_shader_stage stage,
                             struct radv_shader_layout *layout);

static inline bool
radv_is_last_vgt_stage(const struct radv_shader_stage *stage)
{
   return (stage->info.stage == MESA_SHADER_VERTEX || stage->info.stage == MESA_SHADER_TESS_EVAL ||
           stage->info.stage == MESA_SHADER_GEOMETRY || stage->info.stage == MESA_SHADER_MESH) &&
          (stage->info.next_stage == MESA_SHADER_FRAGMENT || stage->info.next_stage == MESA_SHADER_NONE);
}

static inline bool
radv_pipeline_has_stage(const struct radv_graphics_pipeline *pipeline, gl_shader_stage stage)
{
   return pipeline->base.shaders[stage];
}

bool radv_pipeline_has_ngg_passthrough(const struct radv_graphics_pipeline *pipeline);

bool radv_pipeline_has_gs_copy_shader(const struct radv_pipeline *pipeline);

const struct radv_userdata_info *radv_get_user_sgpr(const struct radv_shader *shader, int idx);

struct radv_shader *radv_get_shader(struct radv_shader *const *shaders, gl_shader_stage stage);

void radv_emit_compute_shader(const struct radv_physical_device *pdevice, struct radeon_cmdbuf *cs,
                              const struct radv_shader *shader);

bool radv_mem_vectorize_callback(unsigned align_mul, unsigned align_offset, unsigned bit_size, unsigned num_components,
                                 nir_intrinsic_instr *low, nir_intrinsic_instr *high, void *data);

void radv_compute_pipeline_init(const struct radv_device *device, struct radv_compute_pipeline *pipeline,
                                const struct radv_pipeline_layout *layout, struct radv_shader *shader);

struct radv_graphics_pipeline_create_info {
   bool use_rectlist;
   bool db_depth_clear;
   bool db_stencil_clear;
   bool depth_compress_disable;
   bool stencil_compress_disable;
   bool resummarize_enable;
   uint32_t custom_blend_mode;
};

struct radv_pipeline_key radv_generate_pipeline_key(const struct radv_device *device,
                                                    const VkPipelineShaderStageCreateInfo *stages,
                                                    const unsigned num_stages, VkPipelineCreateFlags2KHR flags,
                                                    const void *pNext);

void radv_pipeline_init(struct radv_device *device, struct radv_pipeline *pipeline, enum radv_pipeline_type type);

VkResult radv_graphics_pipeline_create(VkDevice device, VkPipelineCache cache,
                                       const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                       const struct radv_graphics_pipeline_create_info *extra,
                                       const VkAllocationCallbacks *alloc, VkPipeline *pPipeline);

VkResult radv_compute_pipeline_create(VkDevice _device, VkPipelineCache _cache,
                                      const VkComputePipelineCreateInfo *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipeline);

bool radv_pipeline_capture_shaders(const struct radv_device *device, VkPipelineCreateFlags2KHR flags);
bool radv_pipeline_capture_shader_stats(const struct radv_device *device, VkPipelineCreateFlags2KHR flags);

VkPipelineShaderStageCreateInfo *radv_copy_shader_stage_create_info(struct radv_device *device, uint32_t stageCount,
                                                                    const VkPipelineShaderStageCreateInfo *pStages,
                                                                    void *mem_ctx);

bool radv_shader_need_indirect_descriptor_sets(const struct radv_shader *shader);

bool radv_pipeline_has_ngg(const struct radv_graphics_pipeline *pipeline);

void radv_pipeline_destroy(struct radv_device *device, struct radv_pipeline *pipeline,
                           const VkAllocationCallbacks *allocator);

struct vk_format_description;
uint32_t radv_translate_buffer_dataformat(const struct util_format_description *desc, int first_non_void);
uint32_t radv_translate_buffer_numformat(const struct util_format_description *desc, int first_non_void);
bool radv_is_buffer_format_supported(VkFormat format, bool *scaled);
uint32_t radv_colorformat_endian_swap(uint32_t colorformat);
unsigned radv_translate_colorswap(VkFormat format, bool do_endian_swap);
uint32_t radv_translate_dbformat(VkFormat format);
uint32_t radv_translate_tex_dataformat(VkFormat format, const struct util_format_description *desc, int first_non_void);
uint32_t radv_translate_tex_numformat(VkFormat format, const struct util_format_description *desc, int first_non_void);
bool radv_format_pack_clear_color(VkFormat format, uint32_t clear_vals[2], VkClearColorValue *value);
bool radv_is_storage_image_format_supported(const struct radv_physical_device *physical_device, VkFormat format);
bool radv_is_colorbuffer_format_supported(const struct radv_physical_device *pdevice, VkFormat format, bool *blendable);
bool radv_dcc_formats_compatible(enum amd_gfx_level gfx_level, VkFormat format1, VkFormat format2,
                                 bool *sign_reinterpret);
bool radv_is_atomic_format_supported(VkFormat format);
bool radv_device_supports_etc(const struct radv_physical_device *physical_device);
bool radv_is_format_emulated(const struct radv_physical_device *physical_device, VkFormat format);

static const VkImageUsageFlags RADV_IMAGE_USAGE_WRITE_BITS =
   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
   VK_IMAGE_USAGE_STORAGE_BIT;

struct radv_image_plane {
   VkFormat format;
   struct radeon_surf surface;
};

struct radv_image_binding {
   /* Set when bound */
   struct radeon_winsys_bo *bo;
   VkDeviceSize offset;
};

struct radv_image {
   struct vk_image vk;

   VkDeviceSize size;
   uint32_t alignment;

   unsigned queue_family_mask;
   bool exclusive;
   bool shareable;
   bool l2_coherent;
   bool dcc_sign_reinterpret;
   bool support_comp_to_single;

   struct radv_image_binding bindings[3];
   bool tc_compatible_cmask;

   uint64_t clear_value_offset;
   uint64_t fce_pred_offset;
   uint64_t dcc_pred_offset;

   /*
    * Metadata for the TC-compat zrange workaround. If the 32-bit value
    * stored at this offset is UINT_MAX, the driver will emit
    * DB_Z_INFO.ZRANGE_PRECISION=0, otherwise it will skip the
    * SET_CONTEXT_REG packet.
    */
   uint64_t tc_compat_zrange_offset;

   /* For VK_ANDROID_native_buffer, the WSI image owns the memory, */
   VkDeviceMemory owned_memory;

   unsigned plane_count;
   bool disjoint;
   struct radv_image_plane planes[0];
};

struct ac_surf_info radv_get_ac_surf_info(struct radv_device *device, const struct radv_image *image);

/* Whether the image has a htile  that is known consistent with the contents of
 * the image and is allowed to be in compressed form.
 *
 * If this is false reads that don't use the htile should be able to return
 * correct results.
 */
bool radv_layout_is_htile_compressed(const struct radv_device *device, const struct radv_image *image,
                                     VkImageLayout layout, unsigned queue_mask);

bool radv_layout_can_fast_clear(const struct radv_device *device, const struct radv_image *image, unsigned level,
                                VkImageLayout layout, unsigned queue_mask);

bool radv_layout_dcc_compressed(const struct radv_device *device, const struct radv_image *image, unsigned level,
                                VkImageLayout layout, unsigned queue_mask);

enum radv_fmask_compression {
   RADV_FMASK_COMPRESSION_NONE,
   RADV_FMASK_COMPRESSION_PARTIAL,
   RADV_FMASK_COMPRESSION_FULL,
};

enum radv_fmask_compression radv_layout_fmask_compression(const struct radv_device *device,
                                                          const struct radv_image *image, VkImageLayout layout,
                                                          unsigned queue_mask);

/**
 * Return whether the image has CMASK metadata for color surfaces.
 */
static inline bool
radv_image_has_cmask(const struct radv_image *image)
{
   return image->planes[0].surface.cmask_offset;
}

/**
 * Return whether the image has FMASK metadata for color surfaces.
 */
static inline bool
radv_image_has_fmask(const struct radv_image *image)
{
   return image->planes[0].surface.fmask_offset;
}

/**
 * Return whether the image has DCC metadata for color surfaces.
 */
static inline bool
radv_image_has_dcc(const struct radv_image *image)
{
   return !(image->planes[0].surface.flags & RADEON_SURF_Z_OR_SBUFFER) && image->planes[0].surface.meta_offset;
}

/**
 * Return whether the image is TC-compatible CMASK.
 */
static inline bool
radv_image_is_tc_compat_cmask(const struct radv_image *image)
{
   return radv_image_has_fmask(image) && image->tc_compatible_cmask;
}

/**
 * Return whether DCC metadata is enabled for a level.
 */
static inline bool
radv_dcc_enabled(const struct radv_image *image, unsigned level)
{
   return radv_image_has_dcc(image) && level < image->planes[0].surface.num_meta_levels;
}

/**
 * Return whether the image has CB metadata.
 */
static inline bool
radv_image_has_CB_metadata(const struct radv_image *image)
{
   return radv_image_has_cmask(image) || radv_image_has_fmask(image) || radv_image_has_dcc(image);
}

/**
 * Return whether the image has HTILE metadata for depth surfaces.
 */
static inline bool
radv_image_has_htile(const struct radv_image *image)
{
   return image->planes[0].surface.flags & RADEON_SURF_Z_OR_SBUFFER && image->planes[0].surface.meta_size;
}

/**
 * Return whether the image has VRS HTILE metadata for depth surfaces
 */
static inline bool
radv_image_has_vrs_htile(const struct radv_device *device, const struct radv_image *image)
{
   const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;

   /* Any depth buffer can potentially use VRS on GFX10.3. */
   return gfx_level == GFX10_3 && device->attachment_vrs_enabled && radv_image_has_htile(image) &&
          (image->vk.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

/**
 * Return whether HTILE metadata is enabled for a level.
 */
static inline bool
radv_htile_enabled(const struct radv_image *image, unsigned level)
{
   return radv_image_has_htile(image) && level < image->planes[0].surface.num_meta_levels;
}

/**
 * Return whether the image is TC-compatible HTILE.
 */
static inline bool
radv_image_is_tc_compat_htile(const struct radv_image *image)
{
   return radv_image_has_htile(image) && (image->planes[0].surface.flags & RADEON_SURF_TC_COMPATIBLE_HTILE);
}

/**
 * Return whether the entire HTILE buffer can be used for depth in order to
 * improve HiZ Z-Range precision.
 */
static inline bool
radv_image_tile_stencil_disabled(const struct radv_device *device, const struct radv_image *image)
{
   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      return !vk_format_has_stencil(image->vk.format) && !radv_image_has_vrs_htile(device, image);
   } else {
      /* Due to a hw bug, TILE_STENCIL_DISABLE must be set to 0 for
       * the TC-compat ZRANGE issue even if no stencil is used.
       */
      return !vk_format_has_stencil(image->vk.format) && !radv_image_is_tc_compat_htile(image);
   }
}

static inline bool
radv_image_has_clear_value(const struct radv_image *image)
{
   return image->clear_value_offset != 0;
}

static inline uint64_t
radv_image_get_fast_clear_va(const struct radv_image *image, uint32_t base_level)
{
   assert(radv_image_has_clear_value(image));

   uint64_t va = radv_buffer_get_va(image->bindings[0].bo);
   va += image->bindings[0].offset + image->clear_value_offset + base_level * 8;
   return va;
}

static inline uint64_t
radv_image_get_fce_pred_va(const struct radv_image *image, uint32_t base_level)
{
   assert(image->fce_pred_offset != 0);

   uint64_t va = radv_buffer_get_va(image->bindings[0].bo);
   va += image->bindings[0].offset + image->fce_pred_offset + base_level * 8;
   return va;
}

static inline uint64_t
radv_image_get_dcc_pred_va(const struct radv_image *image, uint32_t base_level)
{
   assert(image->dcc_pred_offset != 0);

   uint64_t va = radv_buffer_get_va(image->bindings[0].bo);
   va += image->bindings[0].offset + image->dcc_pred_offset + base_level * 8;
   return va;
}

static inline uint64_t
radv_get_tc_compat_zrange_va(const struct radv_image *image, uint32_t base_level)
{
   assert(image->tc_compat_zrange_offset != 0);

   uint64_t va = radv_buffer_get_va(image->bindings[0].bo);
   va += image->bindings[0].offset + image->tc_compat_zrange_offset + base_level * 4;
   return va;
}

static inline uint64_t
radv_get_ds_clear_value_va(const struct radv_image *image, uint32_t base_level)
{
   assert(radv_image_has_clear_value(image));

   uint64_t va = radv_buffer_get_va(image->bindings[0].bo);
   va += image->bindings[0].offset + image->clear_value_offset + base_level * 8;
   return va;
}

static inline uint32_t
radv_get_htile_initial_value(const struct radv_device *device, const struct radv_image *image)
{
   uint32_t initial_value;

   if (radv_image_tile_stencil_disabled(device, image)) {
      /* Z only (no stencil):
       *
       * |31     18|17      4|3     0|
       * +---------+---------+-------+
       * |  Max Z  |  Min Z  | ZMask |
       */
      initial_value = 0xfffc000f;
   } else {
      /* Z and stencil:
       *
       * |31       12|11 10|9    8|7   6|5   4|3     0|
       * +-----------+-----+------+-----+-----+-------+
       * |  Z Range  |     | SMem | SR1 | SR0 | ZMask |
       *
       * SR0/SR1 contains the stencil test results. Initializing
       * SR0/SR1 to 0x3 means the stencil test result is unknown.
       *
       * Z, stencil and 4 bit VRS encoding:
       * |31       12|11        10|9    8|7          6|5   4|3     0|
       * +-----------+------------+------+------------+-----+-------+
       * |  Z Range  | VRS y-rate | SMem | VRS x-rate | SR0 | ZMask |
       */
      if (radv_image_has_vrs_htile(device, image)) {
         /* Initialize the VRS x-rate value at 0, so the hw interprets it as 1 sample. */
         initial_value = 0xfffff33f;
      } else {
         initial_value = 0xfffff3ff;
      }
   }

   return initial_value;
}

static inline bool
radv_image_get_iterate256(const struct radv_device *device, struct radv_image *image)
{
   /* ITERATE_256 is required for depth or stencil MSAA images that are TC-compatible HTILE. */
   return device->physical_device->rad_info.gfx_level >= GFX10 &&
          (image->vk.usage & (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)) &&
          radv_image_is_tc_compat_htile(image) && image->vk.samples > 1;
}

unsigned radv_image_queue_family_mask(const struct radv_image *image, enum radv_queue_family family,
                                      enum radv_queue_family queue_family);

bool radv_image_is_renderable(const struct radv_device *device, const struct radv_image *image);

unsigned radv_tile_mode_index(const struct radv_image_plane *plane, unsigned level, bool stencil);

struct radeon_bo_metadata;
void radv_init_metadata(struct radv_device *device, struct radv_image *image, struct radeon_bo_metadata *metadata);

void radv_image_override_offset_stride(struct radv_device *device, struct radv_image *image, uint64_t offset,
                                       uint32_t stride);

union radv_descriptor {
   struct {
      uint32_t plane0_descriptor[8];
      uint32_t fmask_descriptor[8];
   };
   struct {
      uint32_t plane_descriptors[3][8];
   };
};

struct radv_image_view {
   struct vk_image_view vk;
   struct radv_image *image; /**< VkImageViewCreateInfo::image */

   unsigned plane_id;
   VkExtent3D extent; /**< Extent of VkImageViewCreateInfo::baseMipLevel. */

   /* Whether the image iview supports fast clear. */
   bool support_fast_clear;

   bool disable_dcc_mrt;

   union radv_descriptor descriptor;

   /* Descriptor for use as a storage image as opposed to a sampled image.
    * This has a few differences for cube maps (e.g. type).
    */
   union radv_descriptor storage_descriptor;

   /* Block-compressed image views on GFX10+. */
   struct ac_surf_nbc_view nbc_view;
};

struct radv_image_create_info {
   const VkImageCreateInfo *vk_info;
   bool scanout;
   bool no_metadata_planes;
   bool prime_blit_src;
   const struct radeon_bo_metadata *bo_metadata;
};

VkResult radv_image_create_layout(struct radv_device *device, struct radv_image_create_info create_info,
                                  const struct VkImageDrmFormatModifierExplicitCreateInfoEXT *mod_info,
                                  const struct VkVideoProfileListInfoKHR *profile_list, struct radv_image *image);

VkResult radv_image_create(VkDevice _device, const struct radv_image_create_info *info,
                           const VkAllocationCallbacks *alloc, VkImage *pImage, bool is_internal);

bool radv_are_formats_dcc_compatible(const struct radv_physical_device *pdev, const void *pNext, VkFormat format,
                                     VkImageCreateFlags flags, bool *sign_reinterpret);

bool vi_alpha_is_on_msb(const struct radv_device *device, const VkFormat format);

unsigned radv_get_dcc_max_uncompressed_block_size(const struct radv_device *device, const struct radv_image *image);

VkResult radv_image_from_gralloc(VkDevice device_h, const VkImageCreateInfo *base_info,
                                 const VkNativeBufferANDROID *gralloc_info, const VkAllocationCallbacks *alloc,
                                 VkImage *out_image_h);
VkResult radv_import_ahb_memory(struct radv_device *device, struct radv_device_memory *mem, unsigned priority,
                                const VkImportAndroidHardwareBufferInfoANDROID *info);
VkResult radv_create_ahb_memory(struct radv_device *device, struct radv_device_memory *mem, unsigned priority,
                                const VkMemoryAllocateInfo *pAllocateInfo);

unsigned radv_ahb_format_for_vk_format(VkFormat vk_format);

VkFormat radv_select_android_external_format(const void *next, VkFormat default_format);

bool radv_android_gralloc_supports_format(VkFormat format, VkImageUsageFlagBits usage);

struct radv_image_view_extra_create_info {
   bool disable_compression;
   bool enable_compression;
   bool disable_dcc_mrt;
   bool from_client; /**< Set only if this came from vkCreateImage */
};

void radv_image_view_init(struct radv_image_view *view, struct radv_device *device,
                          const VkImageViewCreateInfo *pCreateInfo, VkImageCreateFlags img_create_flags,
                          const struct radv_image_view_extra_create_info *extra_create_info);
void radv_image_view_finish(struct radv_image_view *iview);

VkFormat radv_get_aspect_format(struct radv_image *image, VkImageAspectFlags mask);

void radv_compose_swizzle(const struct util_format_description *desc, const VkComponentMapping *mapping,
                          enum pipe_swizzle swizzle[4]);

unsigned radv_map_swizzle(unsigned swizzle);

struct radv_buffer_view {
   struct vk_buffer_view vk;
   struct radeon_winsys_bo *bo;
   uint32_t state[4];
};
void radv_buffer_view_init(struct radv_buffer_view *view, struct radv_device *device,
                           const VkBufferViewCreateInfo *pCreateInfo);
void radv_buffer_view_finish(struct radv_buffer_view *view);

static inline bool
radv_image_extent_compare(const struct radv_image *image, const VkExtent3D *extent)
{
   if (extent->width != image->vk.extent.width || extent->height != image->vk.extent.height ||
       extent->depth != image->vk.extent.depth)
      return false;
   return true;
}

struct radv_sampler {
   struct vk_sampler vk;
   uint32_t state[4];
   uint32_t border_color_slot;
};

struct radv_resolve_barrier {
   VkPipelineStageFlags2 src_stage_mask;
   VkPipelineStageFlags2 dst_stage_mask;
   VkAccessFlags2 src_access_mask;
   VkAccessFlags2 dst_access_mask;
};

void radv_emit_resolve_barrier(struct radv_cmd_buffer *cmd_buffer, const struct radv_resolve_barrier *barrier);

struct radv_query_pool {
   struct vk_query_pool vk;
   struct radeon_winsys_bo *bo;
   uint32_t stride;
   uint32_t availability_offset;
   uint64_t size;
   char *ptr;
   bool uses_gds; /* For NGG GS on GFX10+ */
   bool uses_ace; /* For task shader invocations on GFX10.3+ */
};

struct radv_perfcounter_impl;

struct radv_pc_query_pool {
   struct radv_query_pool b;

   uint32_t *pc_regs;
   unsigned num_pc_regs;

   unsigned num_passes;

   unsigned num_counters;
   struct radv_perfcounter_impl *counters;
};

void radv_pc_deinit_query_pool(struct radv_pc_query_pool *pool);
VkResult radv_pc_init_query_pool(struct radv_physical_device *pdevice, const VkQueryPoolCreateInfo *pCreateInfo,
                                 struct radv_pc_query_pool *pool);
void radv_pc_begin_query(struct radv_cmd_buffer *cmd_buffer, struct radv_pc_query_pool *pool, uint64_t va);
void radv_pc_end_query(struct radv_cmd_buffer *cmd_buffer, struct radv_pc_query_pool *pool, uint64_t va);
void radv_pc_get_results(const struct radv_pc_query_pool *pc_pool, const uint64_t *data, void *out);

#define VL_MACROBLOCK_WIDTH  16
#define VL_MACROBLOCK_HEIGHT 16

struct radv_vid_mem {
   struct radv_device_memory *mem;
   VkDeviceSize offset;
   VkDeviceSize size;
};

struct radv_video_session {
   struct vk_video_session vk;

   uint32_t stream_handle;
   unsigned stream_type;
   bool interlaced;
   enum { DPB_MAX_RES = 0, DPB_DYNAMIC_TIER_1, DPB_DYNAMIC_TIER_2 } dpb_type;
   unsigned db_alignment;

   struct radv_vid_mem sessionctx;
   struct radv_vid_mem ctx;

   unsigned dbg_frame_cnt;
};

struct radv_video_session_params {
   struct vk_video_session_parameters vk;
};

bool radv_queue_internal_submit(struct radv_queue *queue, struct radeon_cmdbuf *cs);

int radv_queue_init(struct radv_device *device, struct radv_queue *queue, int idx,
                    const VkDeviceQueueCreateInfo *create_info,
                    const VkDeviceQueueGlobalPriorityCreateInfoKHR *global_priority);

void radv_set_descriptor_set(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point,
                             struct radv_descriptor_set *set, unsigned idx);

void radv_cmd_update_descriptor_sets(struct radv_device *device, struct radv_cmd_buffer *cmd_buffer,
                                     VkDescriptorSet overrideSet, uint32_t descriptorWriteCount,
                                     const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                     const VkCopyDescriptorSet *pDescriptorCopies);

void radv_cmd_update_descriptor_set_with_template(struct radv_device *device, struct radv_cmd_buffer *cmd_buffer,
                                                  struct radv_descriptor_set *set,
                                                  VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                  const void *pData);

void radv_meta_push_descriptor_set(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint pipelineBindPoint,
                                   VkPipelineLayout _layout, uint32_t set, uint32_t descriptorWriteCount,
                                   const VkWriteDescriptorSet *pDescriptorWrites);

void radv_make_texel_buffer_descriptor(struct radv_device *device, uint64_t va, VkFormat vk_format, unsigned offset,
                                       unsigned range, uint32_t *state);

uint32_t radv_init_dcc(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                       const VkImageSubresourceRange *range, uint32_t value);

uint32_t radv_init_fmask(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                         const VkImageSubresourceRange *range);

/* radv_nir_to_llvm.c */
struct radv_shader_args;
struct radv_nir_compiler_options;
struct radv_shader_info;

void llvm_compile_shader(const struct radv_nir_compiler_options *options, const struct radv_shader_info *info,
                         unsigned shader_count, struct nir_shader *const *shaders, struct radv_shader_binary **binary,
                         const struct radv_shader_args *args);

bool radv_sqtt_init(struct radv_device *device);
void radv_sqtt_finish(struct radv_device *device);
bool radv_begin_sqtt(struct radv_queue *queue);
bool radv_end_sqtt(struct radv_queue *queue);
bool radv_get_sqtt_trace(struct radv_queue *queue, struct ac_sqtt_trace *sqtt_trace);
void radv_reset_sqtt_trace(struct radv_device *device);
void radv_emit_sqtt_userdata(const struct radv_cmd_buffer *cmd_buffer, const void *data, uint32_t num_dwords);
bool radv_is_instruction_timing_enabled(void);
bool radv_sqtt_queue_events_enabled(void);
bool radv_sqtt_sample_clocks(struct radv_device *device);

void radv_emit_inhibit_clockgating(const struct radv_device *device, struct radeon_cmdbuf *cs, bool inhibit);
void radv_emit_spi_config_cntl(const struct radv_device *device, struct radeon_cmdbuf *cs, bool enable);

VkResult radv_sqtt_get_timed_cmdbuf(struct radv_queue *queue, struct radeon_winsys_bo *timestamp_bo,
                                    uint32_t timestamp_offset, VkPipelineStageFlags2 timestamp_stage,
                                    VkCommandBuffer *pcmdbuf);

VkResult radv_sqtt_acquire_gpu_timestamp(struct radv_device *device, struct radeon_winsys_bo **gpu_timestamp_bo,
                                         uint32_t *gpu_timestamp_offset, void **gpu_timestamp_ptr);

void radv_rra_trace_init(struct radv_device *device);

VkResult radv_rra_dump_trace(VkQueue vk_queue, char *filename);
void radv_rra_trace_finish(VkDevice vk_device, struct radv_rra_trace_data *data);

void radv_memory_trace_init(struct radv_device *device);
void radv_rmv_log_bo_allocate(struct radv_device *device, struct radeon_winsys_bo *bo, uint32_t size, bool is_internal);
void radv_rmv_log_bo_destroy(struct radv_device *device, struct radeon_winsys_bo *bo);
void radv_rmv_log_heap_create(struct radv_device *device, VkDeviceMemory heap, bool is_internal,
                              VkMemoryAllocateFlags alloc_flags);
void radv_rmv_log_buffer_bind(struct radv_device *device, VkBuffer _buffer);
void radv_rmv_log_image_create(struct radv_device *device, const VkImageCreateInfo *create_info, bool is_internal,
                               VkImage _image);
void radv_rmv_log_image_bind(struct radv_device *device, VkImage _image);
void radv_rmv_log_query_pool_create(struct radv_device *device, VkQueryPool pool, bool is_internal);
void radv_rmv_log_command_buffer_bo_create(struct radv_device *device, struct radeon_winsys_bo *bo,
                                           uint32_t executable_size, uint32_t data_size, uint32_t scratch_size);
void radv_rmv_log_command_buffer_bo_destroy(struct radv_device *device, struct radeon_winsys_bo *bo);
void radv_rmv_log_border_color_palette_create(struct radv_device *device, struct radeon_winsys_bo *bo);
void radv_rmv_log_border_color_palette_destroy(struct radv_device *device, struct radeon_winsys_bo *bo);
void radv_rmv_log_sparse_add_residency(struct radv_device *device, struct radeon_winsys_bo *src_bo, uint64_t offset);
void radv_rmv_log_sparse_remove_residency(struct radv_device *device, struct radeon_winsys_bo *src_bo, uint64_t offset);
void radv_rmv_log_descriptor_pool_create(struct radv_device *device, const VkDescriptorPoolCreateInfo *create_info,
                                         VkDescriptorPool pool, bool is_internal);
void radv_rmv_log_graphics_pipeline_create(struct radv_device *device, struct radv_pipeline *pipeline,
                                           bool is_internal);
void radv_rmv_log_compute_pipeline_create(struct radv_device *device, struct radv_pipeline *pipeline, bool is_internal);
void radv_rmv_log_rt_pipeline_create(struct radv_device *device, struct radv_ray_tracing_pipeline *pipeline);
void radv_rmv_log_event_create(struct radv_device *device, VkEvent event, VkEventCreateFlags flags, bool is_internal);
void radv_rmv_log_resource_destroy(struct radv_device *device, uint64_t handle);
void radv_rmv_log_submit(struct radv_device *device, enum amd_ip_type type);
void radv_rmv_fill_device_info(const struct radv_physical_device *device, struct vk_rmv_device_info *info);
void radv_rmv_collect_trace_events(struct radv_device *device);
void radv_memory_trace_finish(struct radv_device *device);

VkResult radv_create_buffer(struct radv_device *device, const VkBufferCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer, bool is_internal);
VkResult radv_alloc_memory(struct radv_device *device, const VkMemoryAllocateInfo *pAllocateInfo,
                           const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMem, bool is_internal);
VkResult radv_create_query_pool(struct radv_device *device, const VkQueryPoolCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool, bool is_internal);
VkResult radv_create_event(struct radv_device *device, const VkEventCreateInfo *pCreateInfo,
                           const VkAllocationCallbacks *pAllocator, VkEvent *pEvent, bool is_internal);

/* radv_sqtt_layer_.c */
struct radv_barrier_data {
   union {
      struct {
         uint16_t depth_stencil_expand : 1;
         uint16_t htile_hiz_range_expand : 1;
         uint16_t depth_stencil_resummarize : 1;
         uint16_t dcc_decompress : 1;
         uint16_t fmask_decompress : 1;
         uint16_t fast_clear_eliminate : 1;
         uint16_t fmask_color_expand : 1;
         uint16_t init_mask_ram : 1;
         uint16_t reserved : 8;
      };
      uint16_t all;
   } layout_transitions;
};

/**
 * Value for the reason field of an RGP barrier start marker originating from
 * the Vulkan client (does not include PAL-defined values). (Table 15)
 */
enum rgp_barrier_reason {
   RGP_BARRIER_UNKNOWN_REASON = 0xFFFFFFFF,

   /* External app-generated barrier reasons, i.e. API synchronization
    * commands Range of valid values: [0x00000001 ... 0x7FFFFFFF].
    */
   RGP_BARRIER_EXTERNAL_CMD_PIPELINE_BARRIER = 0x00000001,
   RGP_BARRIER_EXTERNAL_RENDER_PASS_SYNC = 0x00000002,
   RGP_BARRIER_EXTERNAL_CMD_WAIT_EVENTS = 0x00000003,

   /* Internal barrier reasons, i.e. implicit synchronization inserted by
    * the Vulkan driver Range of valid values: [0xC0000000 ... 0xFFFFFFFE].
    */
   RGP_BARRIER_INTERNAL_BASE = 0xC0000000,
   RGP_BARRIER_INTERNAL_PRE_RESET_QUERY_POOL_SYNC = RGP_BARRIER_INTERNAL_BASE + 0,
   RGP_BARRIER_INTERNAL_POST_RESET_QUERY_POOL_SYNC = RGP_BARRIER_INTERNAL_BASE + 1,
   RGP_BARRIER_INTERNAL_GPU_EVENT_RECYCLE_STALL = RGP_BARRIER_INTERNAL_BASE + 2,
   RGP_BARRIER_INTERNAL_PRE_COPY_QUERY_POOL_RESULTS_SYNC = RGP_BARRIER_INTERNAL_BASE + 3
};

void radv_describe_begin_cmd_buffer(struct radv_cmd_buffer *cmd_buffer);
void radv_describe_end_cmd_buffer(struct radv_cmd_buffer *cmd_buffer);
void radv_describe_draw(struct radv_cmd_buffer *cmd_buffer);
void radv_describe_dispatch(struct radv_cmd_buffer *cmd_buffer, const struct radv_dispatch_info *info);
void radv_describe_begin_render_pass_clear(struct radv_cmd_buffer *cmd_buffer, VkImageAspectFlagBits aspects);
void radv_describe_end_render_pass_clear(struct radv_cmd_buffer *cmd_buffer);
void radv_describe_begin_render_pass_resolve(struct radv_cmd_buffer *cmd_buffer);
void radv_describe_end_render_pass_resolve(struct radv_cmd_buffer *cmd_buffer);
void radv_describe_barrier_start(struct radv_cmd_buffer *cmd_buffer, enum rgp_barrier_reason reason);
void radv_describe_barrier_end(struct radv_cmd_buffer *cmd_buffer);
void radv_describe_barrier_end_delayed(struct radv_cmd_buffer *cmd_buffer);
void radv_describe_layout_transition(struct radv_cmd_buffer *cmd_buffer, const struct radv_barrier_data *barrier);

void radv_sqtt_emit_relocated_shaders(struct radv_cmd_buffer *cmd_buffer, struct radv_graphics_pipeline *pipeline);

struct radv_indirect_command_layout {
   struct vk_object_base base;

   VkIndirectCommandsLayoutUsageFlagsNV flags;
   VkPipelineBindPoint pipeline_bind_point;

   uint32_t input_stride;
   uint32_t token_count;

   bool indexed;
   bool binds_index_buffer;
   bool draw_mesh_tasks;
   uint16_t draw_params_offset;
   uint16_t index_buffer_offset;

   uint16_t dispatch_params_offset;

   uint16_t state_offset;

   uint32_t bind_vbo_mask;
   uint32_t vbo_offsets[MAX_VBS];

   uint64_t push_constant_mask;
   uint32_t push_constant_offsets[MAX_PUSH_CONSTANTS_SIZE / 4];

   uint32_t ibo_type_32;
   uint32_t ibo_type_8;

   VkIndirectCommandsLayoutTokenNV tokens[0];
};

uint32_t radv_get_indirect_cmdbuf_size(const VkGeneratedCommandsInfoNV *cmd_info);

bool radv_use_dgc_predication(struct radv_cmd_buffer *cmd_buffer,
                              const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo);
void radv_prepare_dgc(struct radv_cmd_buffer *cmd_buffer, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo);

bool radv_dgc_can_preprocess(const struct radv_indirect_command_layout *layout, struct radv_pipeline *pipeline);

static inline uint32_t
radv_conv_prim_to_gs_out(uint32_t topology, bool is_ngg)
{
   switch (topology) {
   case V_008958_DI_PT_POINTLIST:
   case V_008958_DI_PT_PATCH:
      return V_028A6C_POINTLIST;
   case V_008958_DI_PT_LINELIST:
   case V_008958_DI_PT_LINESTRIP:
   case V_008958_DI_PT_LINELIST_ADJ:
   case V_008958_DI_PT_LINESTRIP_ADJ:
      return V_028A6C_LINESTRIP;
   case V_008958_DI_PT_TRILIST:
   case V_008958_DI_PT_TRISTRIP:
   case V_008958_DI_PT_TRIFAN:
   case V_008958_DI_PT_TRILIST_ADJ:
   case V_008958_DI_PT_TRISTRIP_ADJ:
      return V_028A6C_TRISTRIP;
   case V_008958_DI_PT_RECTLIST:
      return is_ngg ? V_028A6C_RECTLIST : V_028A6C_TRISTRIP;
   default:
      assert(0);
      return 0;
   }
}

static inline uint32_t
radv_translate_prim(unsigned topology)
{
   switch (topology) {
   case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
      return V_008958_DI_PT_POINTLIST;
   case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
      return V_008958_DI_PT_LINELIST;
   case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
      return V_008958_DI_PT_LINESTRIP;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
      return V_008958_DI_PT_TRILIST;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
      return V_008958_DI_PT_TRISTRIP;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
      return V_008958_DI_PT_TRIFAN;
   case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
      return V_008958_DI_PT_LINELIST_ADJ;
   case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
      return V_008958_DI_PT_LINESTRIP_ADJ;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
      return V_008958_DI_PT_TRILIST_ADJ;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
      return V_008958_DI_PT_TRISTRIP_ADJ;
   case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
      return V_008958_DI_PT_PATCH;
   default:
      unreachable("unhandled primitive type");
   }
}

static inline bool
radv_prim_is_points_or_lines(unsigned topology)
{
   switch (topology) {
   case V_008958_DI_PT_POINTLIST:
   case V_008958_DI_PT_LINELIST:
   case V_008958_DI_PT_LINESTRIP:
   case V_008958_DI_PT_LINELIST_ADJ:
   case V_008958_DI_PT_LINESTRIP_ADJ:
      return true;
   default:
      return false;
   }
}

static inline bool
radv_rast_prim_is_point(unsigned rast_prim)
{
   return rast_prim == V_028A6C_POINTLIST;
}

static inline bool
radv_rast_prim_is_line(unsigned rast_prim)
{
   return rast_prim == V_028A6C_LINESTRIP;
}

static inline bool
radv_rast_prim_is_points_or_lines(unsigned rast_prim)
{
   return radv_rast_prim_is_point(rast_prim) || radv_rast_prim_is_line(rast_prim);
}

static inline bool
radv_polygon_mode_is_point(unsigned polygon_mode)
{
   return polygon_mode == V_028814_X_DRAW_POINTS;
}

static inline bool
radv_polygon_mode_is_line(unsigned polygon_mode)
{
   return polygon_mode == V_028814_X_DRAW_LINES;
}

static inline bool
radv_polygon_mode_is_points_or_lines(unsigned polygon_mode)
{
   return radv_polygon_mode_is_point(polygon_mode) || radv_polygon_mode_is_line(polygon_mode);
}

static inline bool
radv_primitive_topology_is_line_list(unsigned primitive_topology)
{
   return primitive_topology == V_008958_DI_PT_LINELIST || primitive_topology == V_008958_DI_PT_LINELIST_ADJ;
}

static inline unsigned
radv_get_num_vertices_per_prim(const struct radv_pipeline_key *pipeline_key)
{
   if (pipeline_key->vs.topology == V_008958_DI_PT_NONE) {
      /* When the topology is unknown (with graphics pipeline library), return the maximum number of
       * vertices per primitives for VS. This is used to lower NGG (the HW will ignore the extra
       * bits for points/lines) and also to enable NGG culling unconditionally (it will be disabled
       * dynamically for points/lines).
       */
      return 3;
   } else {
      /* Need to add 1, because: V_028A6C_POINTLIST=0, V_028A6C_LINESTRIP=1, V_028A6C_TRISTRIP=2, etc. */
      return radv_conv_prim_to_gs_out(pipeline_key->vs.topology, false) + 1;
   }
}

static inline uint32_t
radv_translate_fill(VkPolygonMode func)
{
   switch (func) {
   case VK_POLYGON_MODE_FILL:
      return V_028814_X_DRAW_TRIANGLES;
   case VK_POLYGON_MODE_LINE:
      return V_028814_X_DRAW_LINES;
   case VK_POLYGON_MODE_POINT:
      return V_028814_X_DRAW_POINTS;
   default:
      assert(0);
      return V_028814_X_DRAW_POINTS;
   }
}

static inline uint32_t
radv_translate_stencil_op(enum VkStencilOp op)
{
   switch (op) {
   case VK_STENCIL_OP_KEEP:
      return V_02842C_STENCIL_KEEP;
   case VK_STENCIL_OP_ZERO:
      return V_02842C_STENCIL_ZERO;
   case VK_STENCIL_OP_REPLACE:
      return V_02842C_STENCIL_REPLACE_TEST;
   case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
      return V_02842C_STENCIL_ADD_CLAMP;
   case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
      return V_02842C_STENCIL_SUB_CLAMP;
   case VK_STENCIL_OP_INVERT:
      return V_02842C_STENCIL_INVERT;
   case VK_STENCIL_OP_INCREMENT_AND_WRAP:
      return V_02842C_STENCIL_ADD_WRAP;
   case VK_STENCIL_OP_DECREMENT_AND_WRAP:
      return V_02842C_STENCIL_SUB_WRAP;
   default:
      return 0;
   }
}

static inline uint32_t
radv_translate_blend_logic_op(VkLogicOp op)
{
   switch (op) {
   case VK_LOGIC_OP_CLEAR:
      return V_028808_ROP3_CLEAR;
   case VK_LOGIC_OP_AND:
      return V_028808_ROP3_AND;
   case VK_LOGIC_OP_AND_REVERSE:
      return V_028808_ROP3_AND_REVERSE;
   case VK_LOGIC_OP_COPY:
      return V_028808_ROP3_COPY;
   case VK_LOGIC_OP_AND_INVERTED:
      return V_028808_ROP3_AND_INVERTED;
   case VK_LOGIC_OP_NO_OP:
      return V_028808_ROP3_NO_OP;
   case VK_LOGIC_OP_XOR:
      return V_028808_ROP3_XOR;
   case VK_LOGIC_OP_OR:
      return V_028808_ROP3_OR;
   case VK_LOGIC_OP_NOR:
      return V_028808_ROP3_NOR;
   case VK_LOGIC_OP_EQUIVALENT:
      return V_028808_ROP3_EQUIVALENT;
   case VK_LOGIC_OP_INVERT:
      return V_028808_ROP3_INVERT;
   case VK_LOGIC_OP_OR_REVERSE:
      return V_028808_ROP3_OR_REVERSE;
   case VK_LOGIC_OP_COPY_INVERTED:
      return V_028808_ROP3_COPY_INVERTED;
   case VK_LOGIC_OP_OR_INVERTED:
      return V_028808_ROP3_OR_INVERTED;
   case VK_LOGIC_OP_NAND:
      return V_028808_ROP3_NAND;
   case VK_LOGIC_OP_SET:
      return V_028808_ROP3_SET;
   default:
      unreachable("Unhandled logic op");
   }
}

static inline uint32_t
radv_translate_blend_function(VkBlendOp op)
{
   switch (op) {
   case VK_BLEND_OP_ADD:
      return V_028780_COMB_DST_PLUS_SRC;
   case VK_BLEND_OP_SUBTRACT:
      return V_028780_COMB_SRC_MINUS_DST;
   case VK_BLEND_OP_REVERSE_SUBTRACT:
      return V_028780_COMB_DST_MINUS_SRC;
   case VK_BLEND_OP_MIN:
      return V_028780_COMB_MIN_DST_SRC;
   case VK_BLEND_OP_MAX:
      return V_028780_COMB_MAX_DST_SRC;
   default:
      return 0;
   }
}

static inline uint32_t
radv_translate_blend_factor(enum amd_gfx_level gfx_level, VkBlendFactor factor)
{
   switch (factor) {
   case VK_BLEND_FACTOR_ZERO:
      return V_028780_BLEND_ZERO;
   case VK_BLEND_FACTOR_ONE:
      return V_028780_BLEND_ONE;
   case VK_BLEND_FACTOR_SRC_COLOR:
      return V_028780_BLEND_SRC_COLOR;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
      return V_028780_BLEND_ONE_MINUS_SRC_COLOR;
   case VK_BLEND_FACTOR_DST_COLOR:
      return V_028780_BLEND_DST_COLOR;
   case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
      return V_028780_BLEND_ONE_MINUS_DST_COLOR;
   case VK_BLEND_FACTOR_SRC_ALPHA:
      return V_028780_BLEND_SRC_ALPHA;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
      return V_028780_BLEND_ONE_MINUS_SRC_ALPHA;
   case VK_BLEND_FACTOR_DST_ALPHA:
      return V_028780_BLEND_DST_ALPHA;
   case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
      return V_028780_BLEND_ONE_MINUS_DST_ALPHA;
   case VK_BLEND_FACTOR_CONSTANT_COLOR:
      return gfx_level >= GFX11 ? V_028780_BLEND_CONSTANT_COLOR_GFX11 : V_028780_BLEND_CONSTANT_COLOR_GFX6;
   case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
      return gfx_level >= GFX11 ? V_028780_BLEND_ONE_MINUS_CONSTANT_COLOR_GFX11
                                : V_028780_BLEND_ONE_MINUS_CONSTANT_COLOR_GFX6;
   case VK_BLEND_FACTOR_CONSTANT_ALPHA:
      return gfx_level >= GFX11 ? V_028780_BLEND_CONSTANT_ALPHA_GFX11 : V_028780_BLEND_CONSTANT_ALPHA_GFX6;
   case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
      return gfx_level >= GFX11 ? V_028780_BLEND_ONE_MINUS_CONSTANT_ALPHA_GFX11
                                : V_028780_BLEND_ONE_MINUS_CONSTANT_ALPHA_GFX6;
   case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
      return V_028780_BLEND_SRC_ALPHA_SATURATE;
   case VK_BLEND_FACTOR_SRC1_COLOR:
      return gfx_level >= GFX11 ? V_028780_BLEND_SRC1_COLOR_GFX11 : V_028780_BLEND_SRC1_COLOR_GFX6;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
      return gfx_level >= GFX11 ? V_028780_BLEND_INV_SRC1_COLOR_GFX11 : V_028780_BLEND_INV_SRC1_COLOR_GFX6;
   case VK_BLEND_FACTOR_SRC1_ALPHA:
      return gfx_level >= GFX11 ? V_028780_BLEND_SRC1_ALPHA_GFX11 : V_028780_BLEND_SRC1_ALPHA_GFX6;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
      return gfx_level >= GFX11 ? V_028780_BLEND_INV_SRC1_ALPHA_GFX11 : V_028780_BLEND_INV_SRC1_ALPHA_GFX6;
   default:
      return 0;
   }
}

static inline uint32_t
radv_translate_blend_opt_factor(VkBlendFactor factor, bool is_alpha)
{
   switch (factor) {
   case VK_BLEND_FACTOR_ZERO:
      return V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_ALL;
   case VK_BLEND_FACTOR_ONE:
      return V_028760_BLEND_OPT_PRESERVE_ALL_IGNORE_NONE;
   case VK_BLEND_FACTOR_SRC_COLOR:
      return is_alpha ? V_028760_BLEND_OPT_PRESERVE_A1_IGNORE_A0 : V_028760_BLEND_OPT_PRESERVE_C1_IGNORE_C0;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
      return is_alpha ? V_028760_BLEND_OPT_PRESERVE_A0_IGNORE_A1 : V_028760_BLEND_OPT_PRESERVE_C0_IGNORE_C1;
   case VK_BLEND_FACTOR_SRC_ALPHA:
      return V_028760_BLEND_OPT_PRESERVE_A1_IGNORE_A0;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
      return V_028760_BLEND_OPT_PRESERVE_A0_IGNORE_A1;
   case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
      return is_alpha ? V_028760_BLEND_OPT_PRESERVE_ALL_IGNORE_NONE : V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_A0;
   default:
      return V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_NONE;
   }
}

static inline uint32_t
radv_translate_blend_opt_function(VkBlendOp op)
{
   switch (op) {
   case VK_BLEND_OP_ADD:
      return V_028760_OPT_COMB_ADD;
   case VK_BLEND_OP_SUBTRACT:
      return V_028760_OPT_COMB_SUBTRACT;
   case VK_BLEND_OP_REVERSE_SUBTRACT:
      return V_028760_OPT_COMB_REVSUBTRACT;
   case VK_BLEND_OP_MIN:
      return V_028760_OPT_COMB_MIN;
   case VK_BLEND_OP_MAX:
      return V_028760_OPT_COMB_MAX;
   default:
      return V_028760_OPT_COMB_BLEND_DISABLED;
   }
}

static inline bool
radv_blend_factor_uses_dst(VkBlendFactor factor)
{
   return factor == VK_BLEND_FACTOR_DST_COLOR || factor == VK_BLEND_FACTOR_DST_ALPHA ||
          factor == VK_BLEND_FACTOR_SRC_ALPHA_SATURATE || factor == VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA ||
          factor == VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
}

static inline bool
radv_is_dual_src(VkBlendFactor factor)
{
   switch (factor) {
   case VK_BLEND_FACTOR_SRC1_COLOR:
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
   case VK_BLEND_FACTOR_SRC1_ALPHA:
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
      return true;
   default:
      return false;
   }
}

static ALWAYS_INLINE bool
radv_can_enable_dual_src(const struct vk_color_blend_attachment_state *att)
{
   VkBlendOp eqRGB = att->color_blend_op;
   VkBlendFactor srcRGB = att->src_color_blend_factor;
   VkBlendFactor dstRGB = att->dst_color_blend_factor;
   VkBlendOp eqA = att->alpha_blend_op;
   VkBlendFactor srcA = att->src_alpha_blend_factor;
   VkBlendFactor dstA = att->dst_alpha_blend_factor;
   bool eqRGB_minmax = eqRGB == VK_BLEND_OP_MIN || eqRGB == VK_BLEND_OP_MAX;
   bool eqA_minmax = eqA == VK_BLEND_OP_MIN || eqA == VK_BLEND_OP_MAX;

   if (!eqRGB_minmax && (radv_is_dual_src(srcRGB) || radv_is_dual_src(dstRGB)))
      return true;
   if (!eqA_minmax && (radv_is_dual_src(srcA) || radv_is_dual_src(dstA)))
      return true;
   return false;
}

static inline void
radv_normalize_blend_factor(VkBlendOp op, VkBlendFactor *src_factor, VkBlendFactor *dst_factor)
{
   if (op == VK_BLEND_OP_MIN || op == VK_BLEND_OP_MAX) {
      *src_factor = VK_BLEND_FACTOR_ONE;
      *dst_factor = VK_BLEND_FACTOR_ONE;
   }
}

void radv_blend_remove_dst(VkBlendOp *func, VkBlendFactor *src_factor, VkBlendFactor *dst_factor,
                           VkBlendFactor expected_dst, VkBlendFactor replacement_src);

ALWAYS_INLINE static bool
radv_is_streamout_enabled(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_streamout_state *so = &cmd_buffer->state.streamout;

   /* Streamout must be enabled for the PRIMITIVES_GENERATED query to work. */
   return (so->streamout_enabled || cmd_buffer->state.active_prims_gen_queries) && !cmd_buffer->state.suspend_streamout;
}

/*
 * Queue helper to get ring.
 * placed here as it needs queue + device structs.
 */
static inline enum amd_ip_type
radv_queue_ring(const struct radv_queue *queue)
{
   return radv_queue_family_to_ring(queue->device->physical_device, queue->state.qf);
}

/* radv_video */
void radv_init_physical_device_decoder(struct radv_physical_device *pdevice);
void radv_video_get_profile_alignments(struct radv_physical_device *pdevice,
                                       const VkVideoProfileListInfoKHR *profile_list, uint32_t *width_align_out,
                                       uint32_t *height_align_out);
/**
 * Helper used for debugging compiler issues by enabling/disabling LLVM for a
 * specific shader stage (developers only).
 */
static inline bool
radv_use_llvm_for_stage(const struct radv_device *device, UNUSED gl_shader_stage stage)
{
   return device->physical_device->use_llvm;
}

static inline bool
radv_has_shader_buffer_float_minmax(const struct radv_physical_device *pdevice, unsigned bitsize)
{
   return (pdevice->rad_info.gfx_level <= GFX7 && !pdevice->use_llvm) || pdevice->rad_info.gfx_level == GFX10 ||
          pdevice->rad_info.gfx_level == GFX10_3 || (pdevice->rad_info.gfx_level == GFX11 && bitsize == 32);
}

static inline bool
radv_has_pops(const struct radv_physical_device *pdevice)
{
   return pdevice->rad_info.gfx_level >= GFX9 && !pdevice->use_llvm;
}

unsigned radv_compact_spi_shader_col_format(const struct radv_shader *ps, uint32_t spi_shader_col_format);

/* radv_perfcounter.c */
void radv_perfcounter_emit_shaders(struct radv_device *device, struct radeon_cmdbuf *cs, unsigned shaders);
void radv_perfcounter_emit_spm_reset(struct radeon_cmdbuf *cs);
void radv_perfcounter_emit_spm_start(struct radv_device *device, struct radeon_cmdbuf *cs, int family);
void radv_perfcounter_emit_spm_stop(struct radv_device *device, struct radeon_cmdbuf *cs, int family);

/* radv_spm.c */
bool radv_spm_init(struct radv_device *device);
void radv_spm_finish(struct radv_device *device);
void radv_emit_spm_setup(struct radv_device *device, struct radeon_cmdbuf *cs, enum radv_queue_family qf);

void radv_destroy_graphics_pipeline(struct radv_device *device, struct radv_graphics_pipeline *pipeline);
void radv_destroy_graphics_lib_pipeline(struct radv_device *device, struct radv_graphics_lib_pipeline *pipeline);
void radv_destroy_compute_pipeline(struct radv_device *device, struct radv_compute_pipeline *pipeline);
void radv_destroy_ray_tracing_pipeline(struct radv_device *device, struct radv_ray_tracing_pipeline *pipeline);

void radv_begin_conditional_rendering(struct radv_cmd_buffer *cmd_buffer, uint64_t va, bool draw_visible);
void radv_end_conditional_rendering(struct radv_cmd_buffer *cmd_buffer);

bool radv_gang_init(struct radv_cmd_buffer *cmd_buffer);
void radv_gang_cache_flush(struct radv_cmd_buffer *cmd_buffer);

#define RADV_FROM_HANDLE(__radv_type, __name, __handle) VK_FROM_HANDLE(__radv_type, __name, __handle)

VK_DEFINE_HANDLE_CASTS(radv_cmd_buffer, vk.base, VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER)
VK_DEFINE_HANDLE_CASTS(radv_device, vk.base, VkDevice, VK_OBJECT_TYPE_DEVICE)
VK_DEFINE_HANDLE_CASTS(radv_instance, vk.base, VkInstance, VK_OBJECT_TYPE_INSTANCE)
VK_DEFINE_HANDLE_CASTS(radv_physical_device, vk.base, VkPhysicalDevice, VK_OBJECT_TYPE_PHYSICAL_DEVICE)
VK_DEFINE_HANDLE_CASTS(radv_queue, vk.base, VkQueue, VK_OBJECT_TYPE_QUEUE)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_buffer, vk.base, VkBuffer, VK_OBJECT_TYPE_BUFFER)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_buffer_view, vk.base, VkBufferView, VK_OBJECT_TYPE_BUFFER_VIEW)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_descriptor_pool, base, VkDescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_descriptor_set, header.base, VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_descriptor_set_layout, vk.base, VkDescriptorSetLayout,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_descriptor_update_template, base, VkDescriptorUpdateTemplate,
                               VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_device_memory, base, VkDeviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_event, base, VkEvent, VK_OBJECT_TYPE_EVENT)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_image, vk.base, VkImage, VK_OBJECT_TYPE_IMAGE)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_image_view, vk.base, VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW);
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_indirect_command_layout, base, VkIndirectCommandsLayoutNV,
                               VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_pipeline, base, VkPipeline, VK_OBJECT_TYPE_PIPELINE)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_pipeline_layout, base, VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_query_pool, vk.base, VkQueryPool, VK_OBJECT_TYPE_QUERY_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_sampler, vk.base, VkSampler, VK_OBJECT_TYPE_SAMPLER)

VK_DEFINE_NONDISP_HANDLE_CASTS(radv_video_session, vk.base, VkVideoSessionKHR, VK_OBJECT_TYPE_VIDEO_SESSION_KHR)
VK_DEFINE_NONDISP_HANDLE_CASTS(radv_video_session_params, vk.base, VkVideoSessionParametersKHR,
                               VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR)

#ifdef __cplusplus
}
#endif

#endif /* RADV_PRIVATE_H */
