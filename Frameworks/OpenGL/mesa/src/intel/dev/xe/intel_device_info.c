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

#include "xe/intel_device_info.h"

#include "common/intel_gem.h"
#include "dev/intel_device_info.h"
#include "dev/intel_hwconfig.h"

#include "util/log.h"

#include "drm-uapi/xe_drm.h"

static void *
xe_query_alloc_fetch(int fd, uint32_t query_id, int32_t *len)
{
   struct drm_xe_device_query query = {
      .query = query_id,
   };
   if (intel_ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query))
      return NULL;

   void *data = calloc(1, query.size);
   if (!data)
      return NULL;

   query.data = (uintptr_t)data;
   if (intel_ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query))
      goto data_query_failed;

   if (len)
      *len = query.size;
   return data;

data_query_failed:
   free(data);
   return NULL;
}

static bool
xe_query_config(int fd, struct intel_device_info *devinfo)
{
   struct drm_xe_query_config *config;
   config = xe_query_alloc_fetch(fd, DRM_XE_DEVICE_QUERY_CONFIG, NULL);
   if (!config)
      return false;

   if (config->info[DRM_XE_QUERY_CONFIG_FLAGS] & DRM_XE_QUERY_CONFIG_FLAG_HAS_VRAM)
      devinfo->has_local_mem = true;

   devinfo->revision = (config->info[DRM_XE_QUERY_CONFIG_REV_AND_DEVICE_ID] >> 16) & 0xFFFF;
   devinfo->gtt_size = 1ull << config->info[DRM_XE_QUERY_CONFIG_VA_BITS];
   devinfo->mem_alignment = config->info[DRM_XE_QUERY_CONFIG_MIN_ALIGNMENT];

   free(config);
   return true;
}

bool
intel_device_info_xe_query_regions(int fd, struct intel_device_info *devinfo,
                                   bool update)
{
   struct drm_xe_query_mem_regions *regions;
   regions = xe_query_alloc_fetch(fd, DRM_XE_DEVICE_QUERY_MEM_REGIONS, NULL);
   if (!regions)
      return false;

   for (int i = 0; i < regions->num_mem_regions; i++) {
      struct drm_xe_mem_region *region = &regions->mem_regions[i];

      switch (region->mem_class) {
      case DRM_XE_MEM_REGION_CLASS_SYSMEM: {
         if (!update) {
            devinfo->mem.sram.mem.klass = region->mem_class;
            devinfo->mem.sram.mem.instance = region->instance;
            devinfo->mem.sram.mappable.size = region->total_size;
         } else {
            assert(devinfo->mem.sram.mem.klass == region->mem_class);
            assert(devinfo->mem.sram.mem.instance == region->instance);
            assert(devinfo->mem.sram.mappable.size == region->total_size);
         }
         devinfo->mem.sram.mappable.free = region->total_size - region->used;
         break;
      }
      case DRM_XE_MEM_REGION_CLASS_VRAM: {
         if (!update) {
            devinfo->mem.vram.mem.klass = region->mem_class;
            devinfo->mem.vram.mem.instance = region->instance;
            devinfo->mem.vram.mappable.size = region->cpu_visible_size;
            devinfo->mem.vram.unmappable.size = region->total_size - region->cpu_visible_size;
         } else {
            assert(devinfo->mem.vram.mem.klass == region->mem_class);
            assert(devinfo->mem.vram.mem.instance == region->instance);
            assert(devinfo->mem.vram.mappable.size == region->cpu_visible_size);
            assert(devinfo->mem.vram.unmappable.size == (region->total_size - region->cpu_visible_size));
         }
         devinfo->mem.vram.mappable.free = devinfo->mem.vram.mappable.size - region->cpu_visible_used;
         devinfo->mem.vram.unmappable.free = devinfo->mem.vram.unmappable.size - (region->used - region->cpu_visible_used);
         break;
      }
      default:
         mesa_loge("Unhandled Xe memory class");
         break;
      }
   }

   devinfo->mem.use_class_instance = true;
   free(regions);
   return true;
}

static bool
xe_query_gts(int fd, struct intel_device_info *devinfo)
{
   struct drm_xe_query_gt_list *gt_list;
   gt_list = xe_query_alloc_fetch(fd, DRM_XE_DEVICE_QUERY_GT_LIST, NULL);
   if (!gt_list)
      return false;

   for (uint32_t i = 0; i < gt_list->num_gt; i++) {
      if (gt_list->gt_list[i].type == DRM_XE_QUERY_GT_TYPE_MAIN)
         devinfo->timestamp_frequency = gt_list->gt_list[i].reference_clock;
   }

   free(gt_list);
   return true;
}

void *
intel_device_info_xe_query_hwconfig(int fd, int32_t *len)
{
   return xe_query_alloc_fetch(fd, DRM_XE_DEVICE_QUERY_HWCONFIG, len);
}

static bool
xe_query_process_hwconfig(int fd, struct intel_device_info *devinfo)
{
   int32_t len;
   void *data = intel_device_info_xe_query_hwconfig(fd, &len);

   if (!data)
      return false;

   bool ret = intel_hwconfig_process_table(devinfo, data, len);
   free(data);
   return ret;
}

static void
xe_compute_topology(struct intel_device_info * devinfo,
                    const uint8_t *geo_dss_mask,
                    const uint32_t geo_dss_num_bytes,
                    const uint32_t *eu_per_dss_mask)
{
   intel_device_info_topology_reset_masks(devinfo);
   /* TGL/DG1/ADL-P: 1 slice x 6 dual sub slices
    * RKL/ADL-S: 1 slice x 2 dual sub slices
    * DG2: 8 slices x 4 dual sub slices
    */
   if (devinfo->verx10 >= 125) {
      devinfo->max_slices = 8;
      devinfo->max_subslices_per_slice = 4;
   } else {
      devinfo->max_slices = 1;
      devinfo->max_subslices_per_slice = 6;
   }
   devinfo->max_eus_per_subslice = 16;
   devinfo->subslice_slice_stride = 1;
   devinfo->eu_slice_stride = DIV_ROUND_UP(16 * 4, 8);
   devinfo->eu_subslice_stride = DIV_ROUND_UP(16, 8);

   assert((sizeof(uint32_t) * 8) >= devinfo->max_subslices_per_slice);
   assert((sizeof(uint32_t) * 8) >= devinfo->max_eus_per_subslice);

   const uint32_t dss_mask_in_slice = (1u << devinfo->max_subslices_per_slice) - 1;
   struct slice {
      uint32_t dss_mask;
      struct {
         bool enabled;
         uint32_t eu_mask;
      } dual_subslice[INTEL_DEVICE_MAX_SUBSLICES];
   } slices[INTEL_DEVICE_MAX_SLICES] = {};

   /* Compute and fill slices */
   for (unsigned s = 0; s < devinfo->max_slices; s++) {
      const unsigned first_bit = s * devinfo->max_subslices_per_slice;
      const unsigned dss_index = first_bit / 8;
      const unsigned shift = first_bit % 8;

      assert(geo_dss_num_bytes > dss_index);

      const uint32_t *dss_mask_ptr = (const uint32_t *)&geo_dss_mask[dss_index];
      uint32_t dss_mask = *dss_mask_ptr;
      dss_mask >>= shift;
      dss_mask &= dss_mask_in_slice;

      if (dss_mask) {
         slices[s].dss_mask = dss_mask;
         for (uint32_t dss = 0; dss < devinfo->max_subslices_per_slice; dss++) {
            if ((1u << dss) & slices[s].dss_mask) {
               slices[s].dual_subslice[dss].enabled = true;
               slices[s].dual_subslice[dss].eu_mask = *eu_per_dss_mask;
            }
         }
      }
   }

   /* Set devinfo masks */
   for (unsigned s = 0; s < devinfo->max_slices; s++) {
      if (!slices[s].dss_mask)
         continue;

      devinfo->slice_masks |= (1u << s);

      for (unsigned ss = 0; ss < devinfo->max_subslices_per_slice; ss++) {
         if (!slices[s].dual_subslice[ss].eu_mask)
            continue;

         devinfo->subslice_masks[s * devinfo->subslice_slice_stride +
                                 ss / 8] |= (1u << (ss % 8));

         for (unsigned eu = 0; eu < devinfo->max_eus_per_subslice; eu++) {
            if (!(slices[s].dual_subslice[ss].eu_mask & (1u << eu)))
               continue;

            devinfo->eu_masks[s * devinfo->eu_slice_stride +
                              ss * devinfo->eu_subslice_stride +
                              eu / 8] |= (1u << (eu % 8));
         }
      }

   }

   intel_device_info_topology_update_counts(devinfo);
   intel_device_info_update_pixel_pipes(devinfo, devinfo->subslice_masks);
   intel_device_info_update_l3_banks(devinfo);
}

static bool
xe_query_topology(int fd, struct intel_device_info *devinfo)
{
   struct drm_xe_query_topology_mask *topology;
   int32_t len;
   topology = xe_query_alloc_fetch(fd, DRM_XE_DEVICE_QUERY_GT_TOPOLOGY, &len);
   if (!topology)
      return false;

   uint32_t geo_dss_num_bytes = 0, *eu_per_dss_mask = NULL;
   uint8_t *geo_dss_mask = NULL, *tmp;
   const struct drm_xe_query_topology_mask *head = topology;

   tmp = (uint8_t *)topology + len;
   const struct drm_xe_query_topology_mask *end = (struct drm_xe_query_topology_mask *)tmp;

   while (topology < end) {
      if (topology->gt_id == 0) {
         switch (topology->type) {
         case DRM_XE_TOPO_DSS_GEOMETRY:
            geo_dss_mask = topology->mask;
            geo_dss_num_bytes = topology->num_bytes;
            break;
         case DRM_XE_TOPO_EU_PER_DSS:
            eu_per_dss_mask = (uint32_t *)topology->mask;
            break;
         }
      }

      topology = (struct drm_xe_query_topology_mask *)&topology->mask[topology->num_bytes];
   }

   bool ret = true;
   if (!geo_dss_num_bytes || !geo_dss_mask || !eu_per_dss_mask) {
      ret = false;
      goto parse_failed;
   }

   xe_compute_topology(devinfo, geo_dss_mask, geo_dss_num_bytes, eu_per_dss_mask);

parse_failed:
   free((void *)head);
   return ret;
}

bool
intel_device_info_xe_get_info_from_fd(int fd, struct intel_device_info *devinfo)
{
   if (!intel_device_info_xe_query_regions(fd, devinfo, false))
      return false;

   if (!xe_query_config(fd, devinfo))
      return false;

   if (!xe_query_gts(fd, devinfo))
      return false;

   if (xe_query_process_hwconfig(fd, devinfo))
      intel_device_info_update_after_hwconfig(devinfo);

   if (!xe_query_topology(fd, devinfo))
         return false;

   devinfo->has_context_isolation = true;
   devinfo->has_mmap_offset = true;
   devinfo->has_caching_uapi = false;
   devinfo->has_set_pat_uapi = true;

   return true;
}
