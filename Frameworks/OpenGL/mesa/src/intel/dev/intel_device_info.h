 /*
  * Copyright © 2013 Intel Corporation
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
  *
  */

#ifndef INTEL_DEVICE_INFO_H
#define INTEL_DEVICE_INFO_H

#include <stdbool.h>
#include <stdint.h>

#include "util/bitset.h"
#include "util/macros.h"
#include "compiler/shader_enums.h"
#include "intel_kmd.h"

#include "intel/common/intel_engine.h"
#include "intel/dev/intel_wa.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INTEL_DEVICE_MAX_NAME_SIZE        64
#define INTEL_DEVICE_MAX_SLICES           8
#define INTEL_DEVICE_MAX_SUBSLICES        (8)  /* Maximum on gfx11 */
#define INTEL_DEVICE_MAX_EUS_PER_SUBSLICE (16) /* Maximum on gfx12 */
#define INTEL_DEVICE_MAX_PIXEL_PIPES      (16) /* Maximum on DG2 */

#define INTEL_PLATFORM_GROUP_START(group, new_enum) \
   new_enum, INTEL_PLATFORM_ ## group ## _START = new_enum
#define INTEL_PLATFORM_GROUP_END(group, new_enum) \
   new_enum, INTEL_PLATFORM_ ## group ## _END = new_enum

enum intel_platform {
   INTEL_PLATFORM_GFX3 = 1,
   INTEL_PLATFORM_I965,
   INTEL_PLATFORM_ILK,
   INTEL_PLATFORM_G4X,
   INTEL_PLATFORM_SNB,
   INTEL_PLATFORM_IVB,
   INTEL_PLATFORM_BYT,
   INTEL_PLATFORM_HSW,
   INTEL_PLATFORM_BDW,
   INTEL_PLATFORM_CHV,
   INTEL_PLATFORM_SKL,
   INTEL_PLATFORM_BXT,
   INTEL_PLATFORM_KBL,
   INTEL_PLATFORM_GLK,
   INTEL_PLATFORM_CFL,
   INTEL_PLATFORM_ICL,
   INTEL_PLATFORM_EHL,
   INTEL_PLATFORM_TGL,
   INTEL_PLATFORM_RKL,
   INTEL_PLATFORM_DG1,
   INTEL_PLATFORM_ADL,
   INTEL_PLATFORM_RPL,
   INTEL_PLATFORM_GROUP_START(DG2, INTEL_PLATFORM_DG2_G10),
   INTEL_PLATFORM_DG2_G11,
   INTEL_PLATFORM_GROUP_END(DG2, INTEL_PLATFORM_DG2_G12),
   INTEL_PLATFORM_GROUP_START(ATSM, INTEL_PLATFORM_ATSM_G10),
   INTEL_PLATFORM_GROUP_END(ATSM, INTEL_PLATFORM_ATSM_G11),
   INTEL_PLATFORM_GROUP_START(MTL, INTEL_PLATFORM_MTL_U),
   INTEL_PLATFORM_GROUP_END(MTL, INTEL_PLATFORM_MTL_H),
   INTEL_PLATFORM_LNL,
};

#undef INTEL_PLATFORM_GROUP_START
#undef INTEL_PLATFORM_GROUP_END

#define intel_platform_in_range(platform, platform_range) \
   (((platform) >= INTEL_PLATFORM_ ## platform_range ## _START) && \
    ((platform) <= INTEL_PLATFORM_ ## platform_range ## _END))

#define intel_device_info_is_atsm(devinfo) \
   intel_platform_in_range((devinfo)->platform, ATSM)

#define intel_device_info_is_dg2(devinfo) \
   (intel_platform_in_range((devinfo)->platform, DG2) || \
    intel_platform_in_range((devinfo)->platform, ATSM))

#define intel_device_info_is_mtl(devinfo) \
   intel_platform_in_range((devinfo)->platform, MTL)

#define intel_device_info_is_adln(devinfo) \
   (devinfo->is_adl_n == true)

struct intel_memory_class_instance {
   /* Kernel backend specific class value, no translation needed yet */
   uint16_t klass;
   uint16_t instance;
};

enum intel_device_info_mmap_mode {
      INTEL_DEVICE_INFO_MMAP_MODE_UC = 0,
      INTEL_DEVICE_INFO_MMAP_MODE_WC,
      INTEL_DEVICE_INFO_MMAP_MODE_WB,
};

enum intel_device_info_coherency_mode {
   INTEL_DEVICE_INFO_COHERENCY_MODE_NONE = 0,
   INTEL_DEVICE_INFO_COHERENCY_MODE_1WAY, /* CPU caches are snooped by GPU */
   INTEL_DEVICE_INFO_COHERENCY_MODE_2WAY /* Fully coherent between GPU and CPU */
};

struct intel_device_info_pat_entry {
   uint8_t index;
   enum intel_device_info_mmap_mode mmap;
   enum intel_device_info_coherency_mode coherency;
};

#define PAT_ENTRY(index_, mmap_, coh_)                      \
{                                                           \
   .index = index_,                                         \
   .mmap = INTEL_DEVICE_INFO_MMAP_MODE_##mmap_,             \
   .coherency = INTEL_DEVICE_INFO_COHERENCY_MODE_##coh_     \
}

enum intel_cooperative_matrix_component_type
{
   INTEL_CMAT_FLOAT16,
   INTEL_CMAT_FLOAT32,
   INTEL_CMAT_SINT32,
   INTEL_CMAT_SINT8,
   INTEL_CMAT_UINT32,
   INTEL_CMAT_UINT8,
};

struct intel_cooperative_matrix_configuration
{
   mesa_scope scope;

   /* Matrix A is MxK.
    * Matrix B is KxN.
    * Matrix C and Matrix Result are MxN.
    *
    * Result = A * B + C;
    */
   uint8_t m, n, k;
   enum intel_cooperative_matrix_component_type a, b, c, result;
};

/**
 * Intel hardware information and quirks
 */
struct intel_device_info
{
   enum intel_kmd_type kmd_type;

   /* Driver internal numbers used to differentiate platforms. */
   int ver;
   int verx10;
   int display_ver;

   /**
    * This revision is from ioctl (I915_PARAM_REVISION) unlike
    * pci_revision_id from drm device. Its value is not always
    * same as the pci_revision_id.
    */
   int revision;
   int gt;

   /* PCI info */
   uint16_t pci_domain;
   uint8_t pci_bus;
   uint8_t pci_dev;
   uint8_t pci_func;
   uint16_t pci_device_id;
   uint8_t pci_revision_id;

   enum intel_platform platform;

   bool has_hiz_and_separate_stencil;
   bool must_use_separate_stencil;
   bool has_sample_with_hiz;
   bool has_bit6_swizzle;
   bool has_llc;

   bool has_pln;
   bool has_64bit_float;
   bool has_64bit_float_via_math_pipe;
   bool has_64bit_int;
   bool has_integer_dword_mul;
   bool has_compr4;
   bool has_surface_tile_offset;
   bool supports_simd16_3src;
   bool disable_ccs_repack;

   /**
    * True if CCS needs to be initialized before use.
    */
   bool has_illegal_ccs_values;

   /**
    * True if CCS uses a flat virtual address translation to a memory
    * carve-out, rather than aux map translations, or additional surfaces.
    */
   bool has_flat_ccs;
   bool has_aux_map;
   bool has_caching_uapi;
   bool has_tiling_uapi;
   bool has_ray_tracing;
   bool has_ray_query;
   bool has_local_mem;
   bool has_lsc;
   bool has_mesh_shading;
   bool has_mmap_offset;
   bool has_userptr_probe;
   bool has_context_isolation;
   bool has_set_pat_uapi;
   bool has_indirect_unroll;

   /**
    * \name Intel hardware quirks
    *  @{
    */
   bool has_negative_rhw_bug;

   /**
    * Whether this platform supports fragment shading rate controlled by a
    * primitive in geometry shaders and by a control buffer.
    */
   bool has_coarse_pixel_primitive_and_cb;

   /**
    * Whether this platform has compute engine
    */
   bool has_compute_engine;

   /**
    * Some versions of Gen hardware don't do centroid interpolation correctly
    * on unlit pixels, causing incorrect values for derivatives near triangle
    * edges.  Enabling this flag causes the fragment shader to use
    * non-centroid interpolation for unlit pixels, at the expense of two extra
    * fragment shader instructions.
    */
   bool needs_unlit_centroid_workaround;

   /**
    * We need this for ADL-N specific Wa_14014966230.
    */
   bool is_adl_n;
   /** @} */

   /**
    * \name GPU hardware limits
    *
    * In general, you can find shader thread maximums by looking at the "Maximum
    * Number of Threads" field in the Intel PRM description of the 3DSTATE_VS,
    * 3DSTATE_GS, 3DSTATE_HS, 3DSTATE_DS, and 3DSTATE_PS commands. URB entry
    * limits come from the "Number of URB Entries" field in the
    * 3DSTATE_URB_VS command and friends.
    *
    * These fields are used to calculate the scratch space to allocate.  The
    * amount of scratch space can be larger without being harmful on modern
    * GPUs, however, prior to Haswell, programming the maximum number of threads
    * to greater than the hardware maximum would cause GPU performance to tank.
    *
    *  @{
    */
   /**
    * Total number of slices present on the device whether or not they've been
    * fused off.
    *
    * XXX: CS thread counts are limited by the inability to do cross subslice
    * communication. It is the effectively the number of logical threads which
    * can be executed in a subslice. Fuse configurations may cause this number
    * to change, so we program @max_cs_threads as the lower maximum.
    */
   unsigned num_slices;

   /**
    * Maximum number of slices present on this device (can be more than
    * num_slices if some slices are fused).
    */
   unsigned max_slices;

   /**
    * Number of subslices for each slice (used to be uniform until CNL).
    */
   unsigned num_subslices[INTEL_DEVICE_MAX_SLICES];

   /**
    * Maximum number of subslices per slice present on this device (can be
    * more than the maximum value in the num_subslices[] array if some
    * subslices are fused).
    */
   unsigned max_subslices_per_slice;

   /**
    * Number of subslices on each pixel pipe (ICL).
    */
   unsigned ppipe_subslices[INTEL_DEVICE_MAX_PIXEL_PIPES];

   /**
    * Maximum number of EUs per subslice (some EUs can be fused off).
    */
   unsigned max_eus_per_subslice;

   /**
    * Number of threads per eu, varies between 4 and 8 between generations.
    */
   unsigned num_thread_per_eu;

   /**
    * A bit mask of the slices available.
    */
   uint8_t slice_masks;

   /**
    * An array of bit mask of the subslices available, use subslice_slice_stride
    * to access this array.
    */
   uint8_t subslice_masks[INTEL_DEVICE_MAX_SLICES *
                          DIV_ROUND_UP(INTEL_DEVICE_MAX_SUBSLICES, 8)];

   /**
    * The number of enabled subslices (considering fusing). For exactly which
    * subslices are enabled, see subslice_masks[].
    */
   unsigned subslice_total;

   /**
    * An array of bit mask of EUs available, use eu_slice_stride &
    * eu_subslice_stride to access this array.
    */
   uint8_t eu_masks[INTEL_DEVICE_MAX_SLICES *
                    INTEL_DEVICE_MAX_SUBSLICES *
                    DIV_ROUND_UP(INTEL_DEVICE_MAX_EUS_PER_SUBSLICE, 8)];

   /**
    * Stride to access subslice_masks[].
    */
   uint16_t subslice_slice_stride;

   /**
    * Strides to access eu_masks[].
    */
   uint16_t eu_slice_stride;
   uint16_t eu_subslice_stride;

   unsigned l3_banks;
   unsigned max_vs_threads;   /**< Maximum Vertex Shader threads */
   unsigned max_tcs_threads;  /**< Maximum Hull Shader threads */
   unsigned max_tes_threads;  /**< Maximum Domain Shader threads */
   unsigned max_gs_threads;   /**< Maximum Geometry Shader threads. */
   /**
    * Theoretical maximum number of Pixel Shader threads.
    *
    * PSD means Pixel Shader Dispatcher. On modern Intel GPUs, hardware will
    * automatically scale pixel shader thread count, based on a single value
    * programmed into 3DSTATE_PS.
    *
    * To calculate the maximum number of threads for Gfx8 beyond (which have
    * multiple Pixel Shader Dispatchers):
    *
    * - Look up 3DSTATE_PS and find "Maximum Number of Threads Per PSD"
    * - Usually there's only one PSD per subslice, so use the number of
    *   subslices for number of PSDs.
    * - For max_wm_threads, the total should be PSD threads * #PSDs.
    */
   unsigned max_wm_threads;

   unsigned max_threads_per_psd;

   /**
    * Maximum Compute Shader threads.
    *
    * Thread count * number of EUs per subslice
    */
   unsigned max_cs_threads;

   /**
    * Maximum number of threads per workgroup supported by the GPGPU_WALKER or
    * COMPUTE_WALKER command.
    *
    * This may be smaller than max_cs_threads as it takes into account added
    * restrictions on the GPGPU/COMPUTE_WALKER commands.  While max_cs_threads
    * expresses the total parallelism of the GPU, this expresses the maximum
    * number of threads we can dispatch in a single workgroup.
    */
   unsigned max_cs_workgroup_threads;

   /**
    * The maximum number of potential scratch ids. Due to hardware
    * implementation details, the range of scratch ids may be larger than the
    * number of subslices.
    */
   unsigned max_scratch_ids[MESA_SHADER_STAGES];

   struct {
      /**
       * Fixed size of the URB.
       *
       * On Gfx6 and DG1, this is measured in KB.  Gfx4-5 instead measure
       * this in 512b blocks, as that's more convenient there.
       *
       * On most Gfx7+ platforms, the URB is a section of the L3 cache,
       * and can be resized based on the L3 programming.  For those platforms,
       * simply leave this field blank (zero) - it isn't used.
       */
      unsigned size;

      /**
       * The minimum number of URB entries.  See the 3DSTATE_URB_<XS> docs.
       */
      unsigned min_entries[4];

      /**
       * The maximum number of URB entries.  See the 3DSTATE_URB_<XS> docs.
       */
      unsigned max_entries[4];
   } urb;

   /* Maximum size in Kb that can be allocated to constants in the URB, this
    * is usually divided among the stages for implementing push constants.
    * See 3DSTATE_PUSH_CONSTANT_ALLOC_*.
    */
   unsigned max_constant_urb_size_kb;

   /* Maximum size that can be allocated to constants in mesh pipeline.
    * This essentially applies to fragment shaders only, since mesh stages
    * don't need to allocate space for push constants.
    */
   unsigned mesh_max_constant_urb_size_kb;

   /**
    * Size of the command streamer prefetch. This is important to know for
    * self modifying batches.
    */
   unsigned engine_class_prefetch[INTEL_ENGINE_CLASS_COMPUTE + 1];

   /**
    * Memory alignment requirement for this device.
    */
   unsigned mem_alignment;

   /**
    * For the longest time the timestamp frequency for Gen's timestamp counter
    * could be assumed to be 12.5MHz, where the least significant bit neatly
    * corresponded to 80 nanoseconds.
    *
    * Since Gfx9 the numbers aren't so round, with a a frequency of 12MHz for
    * SKL (or scale factor of 83.33333333) and a frequency of 19200000Hz for
    * BXT.
    *
    * For simplicity to fit with the current code scaling by a single constant
    * to map from raw timestamps to nanoseconds we now do the conversion in
    * floating point instead of integer arithmetic.
    *
    * In general it's probably worth noting that the documented constants we
    * have for the per-platform timestamp frequencies aren't perfect and
    * shouldn't be trusted for scaling and comparing timestamps with a large
    * delta.
    *
    * E.g. with crude testing on my system using the 'correct' scale factor I'm
    * seeing a drift of ~2 milliseconds per second.
    */
   uint64_t timestamp_frequency;

   uint64_t aperture_bytes;
   uint64_t gtt_size;

   /**
    * ID to put into the .aub files.
    */
   int simulator_id;

   /**
    * holds the name of the device
    */
   char name[INTEL_DEVICE_MAX_NAME_SIZE];

   /**
    * no_hw is true when the pci_device_id has been overridden
    */
   bool no_hw;

   /**
    * apply_hwconfig is true when the platform should apply hwconfig values
    */
   bool apply_hwconfig;

   struct {
      bool use_class_instance;
      struct {
         struct intel_memory_class_instance mem;
         struct {
            uint64_t size;
            uint64_t free;
         } mappable, unmappable;
      } sram, vram;
   } mem;

   struct {
      /* To be used when CPU access is frequent, WB + 1 or 2 way coherent */
      struct intel_device_info_pat_entry cached_coherent;
      /* scanout and external BOs */
      struct intel_device_info_pat_entry scanout;
      /* BOs without special needs, can be WB not coherent or WC it depends on the platforms and KMD */
      struct intel_device_info_pat_entry writeback_incoherent;
      struct intel_device_info_pat_entry writecombining;
   } pat;

   BITSET_DECLARE(workarounds, INTEL_WA_NUM);

   struct intel_cooperative_matrix_configuration cooperative_matrix_configurations[4];
   /** @} */
};

#ifdef GFX_VER

#define intel_device_info_is_9lp(devinfo) \
   (GFX_VER == 9 && ((devinfo)->platform == INTEL_PLATFORM_BXT || \
                     (devinfo)->platform == INTEL_PLATFORM_GLK))

#else

#define intel_device_info_is_9lp(devinfo) \
   ((devinfo)->platform == INTEL_PLATFORM_BXT || \
    (devinfo)->platform == INTEL_PLATFORM_GLK)

#endif

static inline bool
intel_device_info_slice_available(const struct intel_device_info *devinfo,
                                  int slice)
{
   assert(slice < INTEL_DEVICE_MAX_SLICES);
   return (devinfo->slice_masks & (1U << slice)) != 0;
}

static inline bool
intel_device_info_subslice_available(const struct intel_device_info *devinfo,
                                     int slice, int subslice)
{
   return (devinfo->subslice_masks[slice * devinfo->subslice_slice_stride +
                                   subslice / 8] & (1U << (subslice % 8))) != 0;
}

static inline bool
intel_device_info_eu_available(const struct intel_device_info *devinfo,
                               int slice, int subslice, int eu)
{
   unsigned subslice_offset = slice * devinfo->eu_slice_stride +
      subslice * devinfo->eu_subslice_stride;

   return (devinfo->eu_masks[subslice_offset + eu / 8] & (1U << eu % 8)) != 0;
}

static inline uint32_t
intel_device_info_subslice_total(const struct intel_device_info *devinfo)
{
   uint32_t total = 0;

   for (size_t i = 0; i < ARRAY_SIZE(devinfo->subslice_masks); i++) {
      total += __builtin_popcount(devinfo->subslice_masks[i]);
   }

   return total;
}

static inline uint32_t
intel_device_info_eu_total(const struct intel_device_info *devinfo)
{
   uint32_t total = 0;

   for (size_t i = 0; i < ARRAY_SIZE(devinfo->eu_masks); i++)
      total += __builtin_popcount(devinfo->eu_masks[i]);

   return total;
}

/**
 * Computes the bound of dualsubslice ID that can be used on this device.
 *
 * You should use this number if you're going to make calculation based on the
 * slice/dualsubslice ID provided by the SR0.0 EU register. The maximum
 * dualsubslice ID can be superior to the total number of dualsubslices on the
 * device, depending on fusing.
 *
 * On a 16 dualsubslice GPU, the maximum dualsubslice ID is 15. This function
 * would return the exclusive bound : 16.
 */
static inline unsigned
intel_device_info_dual_subslice_id_bound(const struct intel_device_info *devinfo)
{
   /* Start from the last slice/subslice so we find the answer faster. */
   for (int s = devinfo->max_slices - 1; s >= 0; s--) {
      for (int ss = devinfo->max_subslices_per_slice - 1; ss >= 0; ss--) {
         if (intel_device_info_subslice_available(devinfo, s, ss))
            return s * devinfo->max_subslices_per_slice + ss + 1;
      }
   }
   unreachable("Invalid topology");
   return 0;
}

int intel_device_name_to_pci_device_id(const char *name);

static inline uint64_t
intel_device_info_timebase_scale(const struct intel_device_info *devinfo,
                                 uint64_t gpu_timestamp)
{
   /* Try to avoid going over the 64bits when doing the scaling */
   uint64_t upper_ts = gpu_timestamp >> 32;
   uint64_t lower_ts = gpu_timestamp & 0xffffffff;
   uint64_t upper_scaled_ts = upper_ts * 1000000000ull / devinfo->timestamp_frequency;
   uint64_t lower_scaled_ts = lower_ts * 1000000000ull / devinfo->timestamp_frequency;
   return (upper_scaled_ts << 32) + lower_scaled_ts;
}

static inline bool
intel_vram_all_mappable(const struct intel_device_info *devinfo)
{
   return devinfo->mem.vram.unmappable.size == 0;
}

bool intel_get_device_info_from_fd(int fh, struct intel_device_info *devinfo);
bool intel_get_device_info_from_pci_id(int pci_id,
                                       struct intel_device_info *devinfo);

/* Only updates intel_device_info::regions::...::free fields. The
 * class/instance/size should remain the same over time.
 */
bool intel_device_info_update_memory_info(struct intel_device_info *devinfo,
                                          int fd);

void intel_device_info_topology_reset_masks(struct intel_device_info *devinfo);
void intel_device_info_topology_update_counts(struct intel_device_info *devinfo);
void intel_device_info_update_pixel_pipes(struct intel_device_info *devinfo, uint8_t *subslice_masks);
void intel_device_info_update_l3_banks(struct intel_device_info *devinfo);
void intel_device_info_update_cs_workgroup_threads(struct intel_device_info *devinfo);
bool intel_device_info_compute_system_memory(struct intel_device_info *devinfo, bool update);
void intel_device_info_update_after_hwconfig(struct intel_device_info *devinfo);

#ifdef GFX_VERx10
#define intel_needs_workaround(devinfo, id)         \
   (INTEL_WA_ ## id ## _GFX_VER &&                              \
    BITSET_TEST(devinfo->workarounds, INTEL_WA_##id))
#else
#define intel_needs_workaround(devinfo, id) \
   BITSET_TEST(devinfo->workarounds, INTEL_WA_##id)
#endif

enum intel_wa_steppings intel_device_info_wa_stepping(struct intel_device_info *devinfo);

#ifdef __cplusplus
}
#endif

#endif /* INTEL_DEVICE_INFO_H */
