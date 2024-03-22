/*
 * Copyright © 2023 Intel Corporation
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

#include <string.h>

#include "intel/dev/i915/intel_device_info.h"
#include "intel/dev/intel_device_info.h"

#include "intel/dev/intel_hwconfig.h"
#include "intel/common/intel_gem.h"
#include "intel/common/i915/intel_gem.h"

#include "util/bitscan.h"
#include "util/log.h"
#include "util/os_misc.h"

#include "drm-uapi/i915_drm.h"

/* At some point in time, some people decided to redefine what topology means,
 * from useful HW related information (slice, subslice, etc...), to much less
 * useful generic stuff that no one cares about (a single slice with lots of
 * subslices). Of course all of this was done without asking the people who
 * defined the topology query in the first place, to solve a lack of
 * information Gfx10+. This function is here to workaround the fact it's not
 * possible to change people's mind even before this stuff goes upstream. Sad
 * times...
 */
static void
update_from_single_slice_topology(struct intel_device_info *devinfo,
                                  const struct drm_i915_query_topology_info *topology,
                                  const struct drm_i915_query_topology_info *geom_topology)
{
   /* An array of bit masks of the subslices available for 3D
    * workloads, analogous to intel_device_info::subslice_masks.  This
    * may differ from the set of enabled subslices on XeHP+ platforms
    * with compute-only subslices.
    */
   uint8_t geom_subslice_masks[ARRAY_SIZE(devinfo->subslice_masks)] = { 0 };

   assert(devinfo->verx10 >= 125);

   intel_device_info_topology_reset_masks(devinfo);

   assert(topology->max_slices == 1);
   assert(topology->max_subslices > 0);
   assert(topology->max_eus_per_subslice > 0);

   /* i915 gives us only one slice so we have to rebuild that out of groups of
    * 4 dualsubslices.
    */
   devinfo->max_subslices_per_slice = 4;
   devinfo->max_eus_per_subslice = 16;
   devinfo->subslice_slice_stride = 1;
   devinfo->eu_slice_stride = DIV_ROUND_UP(16 * 4, 8);
   devinfo->eu_subslice_stride = DIV_ROUND_UP(16, 8);

   for (uint32_t ss_idx = 0; ss_idx < topology->max_subslices; ss_idx++) {
      const uint32_t s = ss_idx / 4;
      const uint32_t ss = ss_idx % 4;

      /* Determine whether ss_idx is enabled (ss_idx_available) and
       * available for 3D workloads (geom_ss_idx_available), which may
       * differ on XeHP+ if ss_idx is a compute-only DSS.
       */
      const bool ss_idx_available =
         (topology->data[topology->subslice_offset + ss_idx / 8] >>
          (ss_idx % 8)) & 1;
      const bool geom_ss_idx_available =
         (geom_topology->data[geom_topology->subslice_offset + ss_idx / 8] >>
          (ss_idx % 8)) & 1;

      if (geom_ss_idx_available) {
         assert(ss_idx_available);
         geom_subslice_masks[s * devinfo->subslice_slice_stride +
                             ss / 8] |= 1u << (ss % 8);
      }

      if (!ss_idx_available)
         continue;

      devinfo->max_slices = MAX2(devinfo->max_slices, s + 1);
      devinfo->slice_masks |= 1u << s;

      devinfo->subslice_masks[s * devinfo->subslice_slice_stride +
                              ss / 8] |= 1u << (ss % 8);

      for (uint32_t eu = 0; eu < devinfo->max_eus_per_subslice; eu++) {
         const bool eu_available =
            (topology->data[topology->eu_offset +
                            ss_idx * topology->eu_stride +
                            eu / 8] >> (eu % 8)) & 1;

         if (!eu_available)
            continue;

         devinfo->eu_masks[s * devinfo->eu_slice_stride +
                           ss * devinfo->eu_subslice_stride +
                           eu / 8] |= 1u << (eu % 8);
      }
   }

   intel_device_info_topology_update_counts(devinfo);
   intel_device_info_update_pixel_pipes(devinfo, geom_subslice_masks);
   intel_device_info_update_l3_banks(devinfo);
}

static void
update_from_topology(struct intel_device_info *devinfo,
                     const struct drm_i915_query_topology_info *topology)
{
   intel_device_info_topology_reset_masks(devinfo);

   assert(topology->max_slices > 0);
   assert(topology->max_subslices > 0);
   assert(topology->max_eus_per_subslice > 0);

   devinfo->subslice_slice_stride = topology->subslice_stride;

   devinfo->eu_subslice_stride = DIV_ROUND_UP(topology->max_eus_per_subslice, 8);
   devinfo->eu_slice_stride = topology->max_subslices * devinfo->eu_subslice_stride;

   assert(sizeof(devinfo->slice_masks) >= DIV_ROUND_UP(topology->max_slices, 8));
   memcpy(&devinfo->slice_masks, topology->data, DIV_ROUND_UP(topology->max_slices, 8));
   devinfo->max_slices = topology->max_slices;
   devinfo->max_subslices_per_slice = topology->max_subslices;
   devinfo->max_eus_per_subslice = topology->max_eus_per_subslice;

   uint32_t subslice_mask_len =
      topology->max_slices * topology->subslice_stride;
   assert(sizeof(devinfo->subslice_masks) >= subslice_mask_len);
   memcpy(devinfo->subslice_masks, &topology->data[topology->subslice_offset],
          subslice_mask_len);

   uint32_t eu_mask_len =
      topology->eu_stride * topology->max_subslices * topology->max_slices;
   assert(sizeof(devinfo->eu_masks) >= eu_mask_len);
   memcpy(devinfo->eu_masks, &topology->data[topology->eu_offset], eu_mask_len);

   /* Now that all the masks are in place, update the counts. */
   intel_device_info_topology_update_counts(devinfo);
   intel_device_info_update_pixel_pipes(devinfo, devinfo->subslice_masks);
   intel_device_info_update_l3_banks(devinfo);
}

/* Generate detailed mask from the I915_PARAM_SLICE_MASK,
 * I915_PARAM_SUBSLICE_MASK & I915_PARAM_EU_TOTAL getparam.
 */
bool
intel_device_info_i915_update_from_masks(struct intel_device_info *devinfo, uint32_t slice_mask,
                       uint32_t subslice_mask, uint32_t n_eus)
{
   struct drm_i915_query_topology_info *topology;

   assert((slice_mask & 0xff) == slice_mask);

   size_t data_length = 100;

   topology = calloc(1, sizeof(*topology) + data_length);
   if (!topology)
      return false;

   topology->max_slices = util_last_bit(slice_mask);
   topology->max_subslices = util_last_bit(subslice_mask);

   topology->subslice_offset = DIV_ROUND_UP(topology->max_slices, 8);
   topology->subslice_stride = DIV_ROUND_UP(topology->max_subslices, 8);

   uint32_t n_subslices = __builtin_popcount(slice_mask) *
      __builtin_popcount(subslice_mask);
   uint32_t max_eus_per_subslice = DIV_ROUND_UP(n_eus, n_subslices);
   uint32_t eu_mask = (1U << max_eus_per_subslice) - 1;

   topology->max_eus_per_subslice = max_eus_per_subslice;
   topology->eu_offset = topology->subslice_offset +
      topology->max_slices * DIV_ROUND_UP(topology->max_subslices, 8);
   topology->eu_stride = DIV_ROUND_UP(max_eus_per_subslice, 8);

   /* Set slice mask in topology */
   for (int b = 0; b < topology->subslice_offset; b++)
      topology->data[b] = (slice_mask >> (b * 8)) & 0xff;

   for (int s = 0; s < topology->max_slices; s++) {

      /* Set subslice mask in topology */
      for (int b = 0; b < topology->subslice_stride; b++) {
         int subslice_offset = topology->subslice_offset +
            s * topology->subslice_stride + b;

         topology->data[subslice_offset] = (subslice_mask >> (b * 8)) & 0xff;
      }

      /* Set eu mask in topology */
      for (int ss = 0; ss < topology->max_subslices; ss++) {
         for (int b = 0; b < topology->eu_stride; b++) {
            int eu_offset = topology->eu_offset +
               (s * topology->max_subslices + ss) * topology->eu_stride + b;

            topology->data[eu_offset] = (eu_mask >> (b * 8)) & 0xff;
         }
      }
   }

   update_from_topology(devinfo, topology);
   free(topology);

   return true;
}

static bool
getparam(int fd, uint32_t param, int *value)
{
   int tmp;

   struct drm_i915_getparam gp = {
      .param = param,
      .value = &tmp,
   };

   int ret = intel_ioctl(fd, DRM_IOCTL_I915_GETPARAM, &gp);
   if (ret != 0)
      return false;

   *value = tmp;
   return true;
}

static bool
get_context_param(int fd, uint32_t context, uint32_t param, uint64_t *value)
{
   struct drm_i915_gem_context_param gp = {
      .ctx_id = context,
      .param = param,
   };

   int ret = intel_ioctl(fd, DRM_IOCTL_I915_GEM_CONTEXT_GETPARAM, &gp);
   if (ret != 0)
      return false;

   *value = gp.value;
   return true;
}

/**
 * for gfx8/gfx9, SLICE_MASK/SUBSLICE_MASK can be used to compute the topology
 * (kernel 4.13+)
 */
static bool
getparam_topology(struct intel_device_info *devinfo, int fd)
{
   int slice_mask = 0;
   if (!getparam(fd, I915_PARAM_SLICE_MASK, &slice_mask))
      goto maybe_warn;

   int n_eus;
   if (!getparam(fd, I915_PARAM_EU_TOTAL, &n_eus))
      goto maybe_warn;

   int subslice_mask = 0;
   if (!getparam(fd, I915_PARAM_SUBSLICE_MASK, &subslice_mask))
      goto maybe_warn;

   return intel_device_info_i915_update_from_masks(devinfo, slice_mask, subslice_mask, n_eus);

 maybe_warn:
   /* Only with Gfx8+ are we starting to see devices with fusing that can only
    * be detected at runtime.
    */
   if (devinfo->ver >= 8)
      mesa_logw("Kernel 4.1 required to properly query GPU properties.");

   return false;
}

/**
 * preferred API for updating the topology in devinfo (kernel 4.17+)
 */
static bool
query_topology(struct intel_device_info *devinfo, int fd)
{
   struct drm_i915_query_topology_info *topo_info =
      intel_i915_query_alloc(fd, DRM_I915_QUERY_TOPOLOGY_INFO, NULL);
   if (topo_info == NULL)
      return false;

   if (devinfo->verx10 >= 125) {
      struct drm_i915_query_topology_info *geom_topo_info =
         intel_i915_query_alloc(fd, DRM_I915_QUERY_GEOMETRY_SUBSLICES, NULL);
      if (geom_topo_info == NULL) {
         free(topo_info);
         return false;
      }

      update_from_single_slice_topology(devinfo, topo_info, geom_topo_info);
      free(geom_topo_info);
   } else {
      update_from_topology(devinfo, topo_info);
   }

   free(topo_info);

   return true;

}

/**
 * Reports memory region info, and allows buffers to target system-memory,
 * and/or device local memory.
 */
bool
intel_device_info_i915_query_regions(struct intel_device_info *devinfo, int fd, bool update)
{
   struct drm_i915_query_memory_regions *meminfo =
      intel_i915_query_alloc(fd, DRM_I915_QUERY_MEMORY_REGIONS, NULL);
   if (meminfo == NULL)
      return false;

   for (int i = 0; i < meminfo->num_regions; i++) {
      const struct drm_i915_memory_region_info *mem = &meminfo->regions[i];
      switch (mem->region.memory_class) {
      case I915_MEMORY_CLASS_SYSTEM: {
         if (!update) {
            devinfo->mem.sram.mem.klass = mem->region.memory_class;
            devinfo->mem.sram.mem.instance = mem->region.memory_instance;
            devinfo->mem.sram.mappable.size = mem->probed_size;
         } else {
            assert(devinfo->mem.sram.mem.klass == mem->region.memory_class);
            assert(devinfo->mem.sram.mem.instance == mem->region.memory_instance);
            assert(devinfo->mem.sram.mappable.size == mem->probed_size);
         }
         /* The kernel uAPI only reports an accurate unallocated_size value
          * for I915_MEMORY_CLASS_DEVICE.
          */
         uint64_t available;
         if (os_get_available_system_memory(&available))
            devinfo->mem.sram.mappable.free = MIN2(available, mem->probed_size);
         break;
      }
      case I915_MEMORY_CLASS_DEVICE:
         if (!update) {
            devinfo->mem.vram.mem.klass = mem->region.memory_class;
            devinfo->mem.vram.mem.instance = mem->region.memory_instance;
            if (mem->probed_cpu_visible_size > 0) {
               devinfo->mem.vram.mappable.size = mem->probed_cpu_visible_size;
               devinfo->mem.vram.unmappable.size =
                  mem->probed_size - mem->probed_cpu_visible_size;
            } else {
               /* We are running on an older kernel without support for the
                * small-bar uapi. These kernels only support systems where the
                * entire vram is mappable.
                */
               devinfo->mem.vram.mappable.size = mem->probed_size;
               devinfo->mem.vram.unmappable.size = 0;
            }
         } else {
            assert(devinfo->mem.vram.mem.klass == mem->region.memory_class);
            assert(devinfo->mem.vram.mem.instance == mem->region.memory_instance);
            assert((devinfo->mem.vram.mappable.size +
                    devinfo->mem.vram.unmappable.size) == mem->probed_size);
         }
         if (mem->unallocated_cpu_visible_size > 0) {
            if (mem->unallocated_size != -1) {
               devinfo->mem.vram.mappable.free = mem->unallocated_cpu_visible_size;
               devinfo->mem.vram.unmappable.free =
                  mem->unallocated_size - mem->unallocated_cpu_visible_size;
            }
         } else {
            /* We are running on an older kernel without support for the
             * small-bar uapi. These kernels only support systems where the
             * entire vram is mappable.
             */
            if (mem->unallocated_size != -1) {
               devinfo->mem.vram.mappable.free = mem->unallocated_size;
               devinfo->mem.vram.unmappable.free = 0;
            }
         }
         break;
      default:
         break;
      }
   }

   free(meminfo);
   devinfo->mem.use_class_instance = true;
   return true;
}

static int
intel_get_aperture_size(int fd, uint64_t *size)
{
   struct drm_i915_gem_get_aperture aperture = { 0 };

   int ret = intel_ioctl(fd, DRM_IOCTL_I915_GEM_GET_APERTURE, &aperture);
   if (ret == 0 && size)
      *size = aperture.aper_size;

   return ret;
}

static bool
has_bit6_swizzle(int fd)
{
   struct drm_gem_close close;

   struct drm_i915_gem_create gem_create = {
      .size = 4096,
   };

   if (intel_ioctl(fd, DRM_IOCTL_I915_GEM_CREATE, &gem_create)) {
      unreachable("Failed to create GEM BO");
      return false;
   }

   bool swizzled = false;

   /* set_tiling overwrites the input on the error path, so we have to open
    * code intel_ioctl.
    */
   struct drm_i915_gem_set_tiling set_tiling = {
      .handle = gem_create.handle,
      .tiling_mode = I915_TILING_X,
      .stride = 512,
   };

   if (intel_ioctl(fd, DRM_IOCTL_I915_GEM_SET_TILING, &set_tiling)) {
      unreachable("Failed to set BO tiling");
      goto close_and_return;
   }

   struct drm_i915_gem_get_tiling get_tiling = {
      .handle = gem_create.handle,
   };

   if (intel_ioctl(fd, DRM_IOCTL_I915_GEM_GET_TILING, &get_tiling)) {
      unreachable("Failed to get BO tiling");
      goto close_and_return;
   }

   assert(get_tiling.tiling_mode == I915_TILING_X);
   swizzled = get_tiling.swizzle_mode != I915_BIT_6_SWIZZLE_NONE;

close_and_return:
   memset(&close, 0, sizeof(close));
   close.handle = gem_create.handle;
   intel_ioctl(fd, DRM_IOCTL_GEM_CLOSE, &close);

   return swizzled;
}

static bool
has_get_tiling(int fd)
{
   int ret;

   struct drm_i915_gem_create gem_create = {
      .size = 4096,
   };

   if (intel_ioctl(fd, DRM_IOCTL_I915_GEM_CREATE, &gem_create)) {
      unreachable("Failed to create GEM BO");
      return false;
   }

   struct drm_i915_gem_get_tiling get_tiling = {
      .handle = gem_create.handle,
   };
   ret = intel_ioctl(fd, DRM_IOCTL_I915_GEM_SET_TILING, &get_tiling);

   struct drm_gem_close close = {
      .handle = gem_create.handle,
   };
   intel_ioctl(fd, DRM_IOCTL_GEM_CLOSE, &close);

   return ret == 0;
}

static void
fixup_chv_device_info(struct intel_device_info *devinfo)
{
   assert(devinfo->platform == INTEL_PLATFORM_CHV);

   /* Cherryview is annoying.  The number of EUs is depending on fusing and
    * isn't determinable from the PCI ID alone.  We default to the minimum
    * available for that PCI ID and then compute the real value from the
    * subslice information we get from the kernel.
    */
   const uint32_t subslice_total = intel_device_info_subslice_total(devinfo);
   const uint32_t eu_total = intel_device_info_eu_total(devinfo);

   /* Logical CS threads = EUs per subslice * num threads per EU */
   uint32_t max_cs_threads =
      eu_total / subslice_total * devinfo->num_thread_per_eu;

   /* Fuse configurations may give more threads than expected, never less. */
   if (max_cs_threads > devinfo->max_cs_threads)
      devinfo->max_cs_threads = max_cs_threads;

   intel_device_info_update_cs_workgroup_threads(devinfo);

   /* Braswell is even more annoying.  Its marketing name isn't determinable
    * from the PCI ID and is also dependent on fusing.
    */
   if (devinfo->pci_device_id != 0x22B1)
      return;

   char *bsw_model;
   switch (eu_total) {
   case 16: bsw_model = "405"; break;
   case 12: bsw_model = "400"; break;
   default: bsw_model = "   "; break;
   }

   char *needle = strstr(devinfo->name, "XXX");
   assert(needle);
   if (needle)
      memcpy(needle, bsw_model, 3);
}

void *
intel_device_info_i915_query_hwconfig(int fd, int32_t *len)
{
   return intel_i915_query_alloc(fd, DRM_I915_QUERY_HWCONFIG_BLOB, len);
}

bool intel_device_info_i915_get_info_from_fd(int fd, struct intel_device_info *devinfo)
{
   void *hwconfig_blob;
   int32_t len;

   hwconfig_blob = intel_device_info_i915_query_hwconfig(fd, &len);
   if (hwconfig_blob) {
      if (intel_hwconfig_process_table(devinfo, hwconfig_blob, len))
         intel_device_info_update_after_hwconfig(devinfo);

      free(hwconfig_blob);
   }

   int val;
   if (getparam(fd, I915_PARAM_CS_TIMESTAMP_FREQUENCY, &val))
      devinfo->timestamp_frequency = val;
   else if (devinfo->ver >= 10) {
      mesa_loge("Kernel 4.15 required to read the CS timestamp frequency.");
      return false;
   }

   if (!getparam(fd, I915_PARAM_REVISION, &devinfo->revision))
      devinfo->revision = 0;

   if (!query_topology(devinfo, fd)) {
      if (devinfo->ver >= 10) {
         /* topology uAPI required for CNL+ (kernel 4.17+) */
         return false;
      }

      /* else use the kernel 4.13+ api for gfx8+.  For older kernels, topology
       * will be wrong, affecting GPU metrics. In this case, fail silently.
       */
      getparam_topology(devinfo, fd);
   }

   /* If the memory region uAPI query is not available, try to generate some
    * numbers out of os_* utils for sram only.
    */
   if (!intel_device_info_i915_query_regions(devinfo, fd, false))
      intel_device_info_compute_system_memory(devinfo, false);

   if (devinfo->platform == INTEL_PLATFORM_CHV)
      fixup_chv_device_info(devinfo);

   /* Broadwell PRM says:
    *
    *   "Before Gfx8, there was a historical configuration control field to
    *    swizzle address bit[6] for in X/Y tiling modes. This was set in three
    *    different places: TILECTL[1:0], ARB_MODE[5:4], and
    *    DISP_ARB_CTL[14:13].
    *
    *    For Gfx8 and subsequent generations, the swizzle fields are all
    *    reserved, and the CPU's memory controller performs all address
    *    swizzling modifications."
    */
   devinfo->has_bit6_swizzle = devinfo->ver < 8 && has_bit6_swizzle(fd);

   intel_get_aperture_size(fd, &devinfo->aperture_bytes);
   get_context_param(fd, 0, I915_CONTEXT_PARAM_GTT_SIZE, &devinfo->gtt_size);
   devinfo->has_tiling_uapi = has_get_tiling(fd);
   devinfo->has_caching_uapi =
      devinfo->platform < INTEL_PLATFORM_DG2_START && !devinfo->has_local_mem;
   if (devinfo->ver > 12 || intel_device_info_is_mtl(devinfo))
      devinfo->has_set_pat_uapi = true;

   if (getparam(fd, I915_PARAM_MMAP_GTT_VERSION, &val))
      devinfo->has_mmap_offset = val >= 4;
   if (getparam(fd, I915_PARAM_HAS_USERPTR_PROBE, &val))
      devinfo->has_userptr_probe = val;
   if (getparam(fd, I915_PARAM_HAS_CONTEXT_ISOLATION, &val))
      devinfo->has_context_isolation = val;

   /* TODO: We might be able to reduce alignment to 4Kb on DG1. */
   if (devinfo->verx10 >= 125)
      devinfo->mem_alignment = 64 * 1024;
   else if (devinfo->has_local_mem)
      devinfo->mem_alignment = 64 * 1024;
   else
      devinfo->mem_alignment = 4096;

   return true;
}
