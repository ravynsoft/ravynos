/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <xf86drm.h>

#include "drm-uapi/pvr_drm.h"
#include "pvr_device_info.h"
#include "pvr_drm.h"
#include "pvr_drm_bo.h"
#include "pvr_drm_job_compute.h"
#include "pvr_drm_job_null.h"
#include "pvr_drm_job_render.h"
#include "pvr_drm_job_transfer.h"
#include "pvr_drm_public.h"
#include "pvr_private.h"
#include "pvr_winsys.h"
#include "pvr_winsys_helper.h"
#include "vk_alloc.h"
#include "vk_drm_syncobj.h"
#include "vk_log.h"

static void pvr_drm_finish_heaps(struct pvr_drm_winsys *const drm_ws)
{
   if (!pvr_winsys_helper_winsys_heap_finish(
          &drm_ws->transfer_frag_heap.base)) {
      vk_errorf(NULL,
                VK_ERROR_UNKNOWN,
                "Transfer fragment heap in use, can't deinit");
   }

   if (!pvr_winsys_helper_winsys_heap_finish(&drm_ws->vis_test_heap.base)) {
      vk_errorf(NULL,
                VK_ERROR_UNKNOWN,
                "Visibility test heap in use, can't deinit");
   }

   if (drm_ws->rgn_hdr_heap_present) {
      if (!pvr_winsys_helper_winsys_heap_finish(&drm_ws->rgn_hdr_heap.base)) {
         vk_errorf(NULL,
                   VK_ERROR_UNKNOWN,
                   "Region header heap in use, can't deinit");
      }
   }

   if (!pvr_winsys_helper_winsys_heap_finish(&drm_ws->usc_heap.base))
      vk_errorf(NULL, VK_ERROR_UNKNOWN, "USC heap in use, can't deinit");

   if (!pvr_winsys_helper_winsys_heap_finish(&drm_ws->pds_heap.base))
      vk_errorf(NULL, VK_ERROR_UNKNOWN, "PDS heap in use, can't deinit");

   if (!pvr_winsys_helper_winsys_heap_finish(&drm_ws->general_heap.base))
      vk_errorf(NULL, VK_ERROR_UNKNOWN, "General heap in use, can't deinit");
}

static void pvr_drm_winsys_destroy(struct pvr_winsys *ws)
{
   struct pvr_drm_winsys *const drm_ws = to_pvr_drm_winsys(ws);
   struct drm_pvr_ioctl_destroy_vm_context_args destroy_vm_context_args = {
      .handle = drm_ws->vm_context,
   };

   pvr_winsys_helper_free_static_memory(drm_ws->general_vma,
                                        drm_ws->pds_vma,
                                        drm_ws->usc_vma);

   pvr_drm_finish_heaps(drm_ws);

   pvr_ioctl(ws->render_fd,
             DRM_IOCTL_PVR_DESTROY_VM_CONTEXT,
             &destroy_vm_context_args,
             VK_ERROR_UNKNOWN);

   vk_free(ws->alloc, drm_ws);
}

/**
 * Linear search a uint32_t array for a value.
 *
 * \param array Pointer to array start.
 * \param len Number of uint32_t terms to compare.
 * \param val The value to search for.
 * \return
 *  * true if val is found, or
 *  * false.
 */
static bool
pvr_u32_in_array(const uint32_t *array, const size_t len, const uint32_t val)
{
   for (int i = 0; i < len; i++) {
      if (array[i] == val)
         return true;
   }

   return false;
}

static VkResult pvr_drm_override_quirks(struct pvr_drm_winsys *drm_ws,
                                        struct pvr_device_info *dev_info)
{
   struct drm_pvr_dev_query_quirks query = { 0 };
   struct drm_pvr_ioctl_dev_query_args args = {
      .type = DRM_PVR_DEV_QUERY_QUIRKS_GET,
      .size = sizeof(query),
      .pointer = (__u64)&query,
   };

/* clang-format off */
#define PVR_QUIRKS(x) \
   x(48545) \
   x(49927) \
   x(51764) \
   x(62269)
   /* clang-format on */

#define PVR_QUIRK_EXPAND_COMMA(number) number,

   const uint32_t supported_quirks[] = { PVR_QUIRKS(PVR_QUIRK_EXPAND_COMMA) };

#undef PVR_QUIRK_EXPAND_COMMA

   VkResult result;

   /* Get the length and allocate enough for it */
   result = pvr_ioctl(drm_ws->base.render_fd,
                      DRM_IOCTL_PVR_DEV_QUERY,
                      &args,
                      VK_ERROR_INITIALIZATION_FAILED);
   if (result != VK_SUCCESS)
      goto out;

   /* It's possible there are no quirks, so we can skip the rest. */
   if (!query.count) {
      result = VK_SUCCESS;
      goto out;
   }

   query.quirks = (__u64)vk_zalloc(drm_ws->base.alloc,
                                   sizeof(uint32_t) * query.count,
                                   8,
                                   VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!query.quirks) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto out;
   }

   /* Get the data */
   result = pvr_ioctl(drm_ws->base.render_fd,
                      DRM_IOCTL_PVR_DEV_QUERY,
                      &args,
                      VK_ERROR_INITIALIZATION_FAILED);
   if (result != VK_SUCCESS)
      goto out_free_quirks;

#define PVR_QUIRK_EXPAND_SET(number)  \
   dev_info->quirks.has_brn##number = \
      pvr_u32_in_array((uint32_t *)query.quirks, query.count, number);

   /*
    * For each quirk, check that if it is a "must have" that it is set in
    * dev_info, then set the dev_info value to the one received from the kernel.
    */
   PVR_QUIRKS(PVR_QUIRK_EXPAND_SET);

#undef PVR_QUIRK_EXPAND_SET
#undef PVR_QUIRKS

   /* Check all musthave quirks are supported */
   for (int i = 0; i < query.musthave_count; i++) {
      if (!pvr_u32_in_array(supported_quirks,
                            ARRAY_SIZE(supported_quirks),
                            ((uint32_t *)query.quirks)[i])) {
         result = VK_ERROR_INCOMPATIBLE_DRIVER;
         goto out_free_quirks;
      }
   }

   result = VK_SUCCESS;

out_free_quirks:
   vk_free(drm_ws->base.alloc, (__u64 *)query.quirks);

out:
   return result;
}

static VkResult pvr_drm_override_enhancements(struct pvr_drm_winsys *drm_ws,
                                              struct pvr_device_info *dev_info)
{
   struct drm_pvr_dev_query_enhancements query = { 0 };
   struct drm_pvr_ioctl_dev_query_args args = {
      .type = DRM_PVR_DEV_QUERY_ENHANCEMENTS_GET,
      .size = sizeof(query),
      .pointer = (__u64)&query
   };

   VkResult result;

   /* Get the length and allocate enough for it */
   result = pvr_ioctl(drm_ws->base.render_fd,
                      DRM_IOCTL_PVR_DEV_QUERY,
                      &args,
                      VK_ERROR_INITIALIZATION_FAILED);
   if (result != VK_SUCCESS)
      goto out;

   /* It's possible there are no enhancements, so we can skip the rest. */
   if (!query.count) {
      result = VK_SUCCESS;
      goto out;
   }

   query.enhancements = (__u64)vk_zalloc(drm_ws->base.alloc,
                                         sizeof(uint32_t) * query.count,
                                         8,
                                         VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!query.enhancements) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto out;
   }

   /* Get the data */
   result = pvr_ioctl(drm_ws->base.render_fd,
                      DRM_IOCTL_PVR_DEV_QUERY,
                      &args,
                      VK_ERROR_INITIALIZATION_FAILED);
   if (result != VK_SUCCESS)
      goto out_free_enhancements;

/* clang-format off */
#define PVR_ENHANCEMENT_SET(number) \
   dev_info->enhancements.has_ern##number = \
      pvr_u32_in_array((uint32_t *)query.enhancements, query.count, number)
   /* clang-format on */

   PVR_ENHANCEMENT_SET(35421);

#undef PVR_ENHANCEMENT_SET

   result = VK_SUCCESS;

out_free_enhancements:
   vk_free(drm_ws->base.alloc, (__u64 *)query.enhancements);

out:
   return result;
}

static VkResult
pvr_drm_get_runtime_info(struct pvr_drm_winsys *drm_ws,
                         struct drm_pvr_dev_query_runtime_info *const value)
{
   struct drm_pvr_ioctl_dev_query_args args = {
      .type = DRM_PVR_DEV_QUERY_RUNTIME_INFO_GET,
      .size = sizeof(*value),
      .pointer = (__u64)value
   };

   return pvr_ioctl(drm_ws->base.render_fd,
                    DRM_IOCTL_PVR_DEV_QUERY,
                    &args,
                    VK_ERROR_INITIALIZATION_FAILED);
}

static VkResult
pvr_drm_get_gpu_info(struct pvr_drm_winsys *drm_ws,
                     struct drm_pvr_dev_query_gpu_info *const value)
{
   struct drm_pvr_ioctl_dev_query_args args = {
      .type = DRM_PVR_DEV_QUERY_GPU_INFO_GET,
      .size = sizeof(*value),
      .pointer = (__u64)value
   };

   return pvr_ioctl(drm_ws->base.render_fd,
                    DRM_IOCTL_PVR_DEV_QUERY,
                    &args,
                    VK_ERROR_INITIALIZATION_FAILED);
}

static VkResult
pvr_drm_winsys_device_info_init(struct pvr_winsys *ws,
                                struct pvr_device_info *dev_info,
                                struct pvr_device_runtime_info *runtime_info)
{
   struct drm_pvr_dev_query_runtime_info kmd_runtime_info = { 0 };
   struct drm_pvr_dev_query_gpu_info gpu_info = { 0 };
   struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ws);
   VkResult result;
   int ret;

   ret = pvr_device_info_init(dev_info, drm_ws->bvnc);
   if (ret) {
      result = vk_errorf(NULL,
                         VK_ERROR_INCOMPATIBLE_DRIVER,
                         "Unsupported BVNC: %u.%u.%u.%u\n",
                         PVR_BVNC_UNPACK_B(drm_ws->bvnc),
                         PVR_BVNC_UNPACK_V(drm_ws->bvnc),
                         PVR_BVNC_UNPACK_N(drm_ws->bvnc),
                         PVR_BVNC_UNPACK_C(drm_ws->bvnc));
      goto err_out;
   }

   result = pvr_drm_override_quirks(drm_ws, dev_info);
   if (result != VK_SUCCESS) {
      mesa_logw("Failed to get quirks for this GPU\n");
      goto err_out;
   }

   result = pvr_drm_override_enhancements(drm_ws, dev_info);
   if (result != VK_SUCCESS) {
      mesa_logw("Failed to get enhancements for this GPU\n");
      goto err_out;
   }

   /* TODO: When kernel support is added, fetch the actual core count. */
   if (PVR_HAS_FEATURE(dev_info, gpu_multicore_support))
      mesa_logw("Core count fetching is unimplemented. Setting 1 for now.");
   runtime_info->core_count = 1;

   result = pvr_drm_get_gpu_info(drm_ws, &gpu_info);
   if (result != VK_SUCCESS)
      goto err_out;

   runtime_info->num_phantoms = gpu_info.num_phantoms;

   result = pvr_drm_get_runtime_info(drm_ws, &kmd_runtime_info);
   if (result != VK_SUCCESS)
      goto err_out;

   runtime_info->min_free_list_size = kmd_runtime_info.free_list_min_pages
                                      << ROGUE_BIF_PM_PHYSICAL_PAGE_SHIFT;
   runtime_info->max_free_list_size = kmd_runtime_info.free_list_max_pages
                                      << ROGUE_BIF_PM_PHYSICAL_PAGE_SHIFT;
   runtime_info->reserved_shared_size =
      kmd_runtime_info.common_store_alloc_region_size;
   runtime_info->total_reserved_partition_size =
      kmd_runtime_info.common_store_partition_space_size;
   runtime_info->max_coeffs = kmd_runtime_info.max_coeffs;
   runtime_info->cdm_max_local_mem_size_regs =
      kmd_runtime_info.cdm_max_local_mem_size_regs;

   return VK_SUCCESS;

err_out:
   return result;
}

static void pvr_drm_winsys_get_heaps_info(struct pvr_winsys *ws,
                                          struct pvr_winsys_heaps *heaps)
{
   struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ws);

   heaps->general_heap = &drm_ws->general_heap.base;
   heaps->pds_heap = &drm_ws->pds_heap.base;
   heaps->transfer_frag_heap = &drm_ws->transfer_frag_heap.base;
   heaps->usc_heap = &drm_ws->usc_heap.base;
   heaps->vis_test_heap = &drm_ws->vis_test_heap.base;

   if (drm_ws->rgn_hdr_heap_present)
      heaps->rgn_hdr_heap = &drm_ws->rgn_hdr_heap.base;
   else
      heaps->rgn_hdr_heap = &drm_ws->general_heap.base;
}

static const struct pvr_winsys_ops drm_winsys_ops = {
   .destroy = pvr_drm_winsys_destroy,
   .device_info_init = pvr_drm_winsys_device_info_init,
   .get_heaps_info = pvr_drm_winsys_get_heaps_info,
   .buffer_create = pvr_drm_winsys_buffer_create,
   .buffer_create_from_fd = pvr_drm_winsys_buffer_create_from_fd,
   .buffer_destroy = pvr_drm_winsys_buffer_destroy,
   .buffer_get_fd = pvr_drm_winsys_buffer_get_fd,
   .buffer_map = pvr_drm_winsys_buffer_map,
   .buffer_unmap = pvr_drm_winsys_buffer_unmap,
   .heap_alloc = pvr_drm_winsys_heap_alloc,
   .heap_free = pvr_drm_winsys_heap_free,
   .vma_map = pvr_drm_winsys_vma_map,
   .vma_unmap = pvr_drm_winsys_vma_unmap,
   .free_list_create = pvr_drm_winsys_free_list_create,
   .free_list_destroy = pvr_drm_winsys_free_list_destroy,
   .render_target_dataset_create = pvr_drm_render_target_dataset_create,
   .render_target_dataset_destroy = pvr_drm_render_target_dataset_destroy,
   .render_ctx_create = pvr_drm_winsys_render_ctx_create,
   .render_ctx_destroy = pvr_drm_winsys_render_ctx_destroy,
   .render_submit = pvr_drm_winsys_render_submit,
   .compute_ctx_create = pvr_drm_winsys_compute_ctx_create,
   .compute_ctx_destroy = pvr_drm_winsys_compute_ctx_destroy,
   .compute_submit = pvr_drm_winsys_compute_submit,
   .transfer_ctx_create = pvr_drm_winsys_transfer_ctx_create,
   .transfer_ctx_destroy = pvr_drm_winsys_transfer_ctx_destroy,
   .transfer_submit = pvr_drm_winsys_transfer_submit,
   .null_job_submit = pvr_drm_winsys_null_job_submit,
};

struct pvr_static_data_area_description {
   struct pvr_winsys_static_data_offsets offsets;
   size_t total_size;
};

static VkResult pvr_drm_get_heap_static_data_descriptions(
   struct pvr_drm_winsys *const drm_ws,
   struct pvr_static_data_area_description desc_out[DRM_PVR_HEAP_COUNT])
{
   struct drm_pvr_dev_query_static_data_areas query = { 0 };
   struct drm_pvr_ioctl_dev_query_args args = {
      .type = DRM_PVR_DEV_QUERY_STATIC_DATA_AREAS_GET,
      .size = sizeof(query),
      .pointer = (__u64)&query
   };
   struct drm_pvr_static_data_area *array;
   VkResult result;

   /* Get the array length */
   result = pvr_ioctlf(drm_ws->base.render_fd,
                       DRM_IOCTL_PVR_DEV_QUERY,
                       &args,
                       VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to fetch static area array size");
   if (result != VK_SUCCESS)
      goto out;

   array = vk_alloc(drm_ws->base.alloc,
                    sizeof(*array) * query.static_data_areas.count,
                    8,
                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!array) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto out;
   }

   VG(VALGRIND_MAKE_MEM_DEFINED(array,
                                sizeof(*array) *
                                   query.static_data_areas.count));

   query.static_data_areas.array = (__u64)array;

   /* Get the array */
   result = pvr_ioctlf(drm_ws->base.render_fd,
                       DRM_IOCTL_PVR_DEV_QUERY,
                       &args,
                       VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to fetch static area offset array");
   if (result != VK_SUCCESS)
      goto out_free_array;

   for (size_t i = 0; i < query.static_data_areas.count; i++) {
      /* Unknown heaps might cause a write outside the array bounds. */
      if (array[i].location_heap_id >= DRM_PVR_HEAP_COUNT)
         continue;

      switch (array[i].area_usage) {
      case DRM_PVR_STATIC_DATA_AREA_EOT:
         desc_out[array[i].location_heap_id].offsets.eot = array[i].offset;
         break;

      case DRM_PVR_STATIC_DATA_AREA_FENCE:
         desc_out[array[i].location_heap_id].offsets.fence = array[i].offset;
         break;

      case DRM_PVR_STATIC_DATA_AREA_VDM_SYNC:
         desc_out[array[i].location_heap_id].offsets.vdm_sync = array[i].offset;
         break;

      case DRM_PVR_STATIC_DATA_AREA_YUV_CSC:
         desc_out[array[i].location_heap_id].offsets.yuv_csc = array[i].offset;
         break;

      default:
         mesa_logd("Unknown drm static area id. ID: %d.", array[i].area_usage);
         continue;
      }

      desc_out[array[i].location_heap_id].total_size += array[i].size;
   }

   result = VK_SUCCESS;

out_free_array:
   vk_free(drm_ws->base.alloc, array);

out:
   return result;
}

static VkResult pvr_drm_setup_heaps(struct pvr_drm_winsys *const drm_ws)
{
   struct pvr_winsys_heap *const winsys_heaps[DRM_PVR_HEAP_COUNT] = {
      [DRM_PVR_HEAP_GENERAL] = &drm_ws->general_heap.base,
      [DRM_PVR_HEAP_PDS_CODE_DATA] = &drm_ws->pds_heap.base,
      [DRM_PVR_HEAP_USC_CODE] = &drm_ws->usc_heap.base,
      [DRM_PVR_HEAP_RGNHDR] = &drm_ws->rgn_hdr_heap.base,
      [DRM_PVR_HEAP_VIS_TEST] = &drm_ws->vis_test_heap.base,
      [DRM_PVR_HEAP_TRANSFER_FRAG] = &drm_ws->transfer_frag_heap.base,
   };
   struct pvr_static_data_area_description
      static_data_descriptions[DRM_PVR_HEAP_COUNT] = { 0 };
   struct drm_pvr_dev_query_heap_info query = { 0 };
   struct drm_pvr_ioctl_dev_query_args args = {
      .type = DRM_PVR_DEV_QUERY_HEAP_INFO_GET,
      .size = sizeof(query),
      .pointer = (__u64)&query
   };
   struct drm_pvr_heap *array;
   VkResult result;
   int i = 0;

   /* Get the array length */
   result = pvr_ioctlf(drm_ws->base.render_fd,
                       DRM_IOCTL_PVR_DEV_QUERY,
                       &args,
                       VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to fetch heap info array size");
   if (result != VK_SUCCESS)
      goto out;

   array = vk_alloc(drm_ws->base.alloc,
                    sizeof(*array) * query.heaps.count,
                    8,
                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!array) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto out;
   }

   VG(VALGRIND_MAKE_MEM_DEFINED(array, sizeof(*array) * query.heaps.count));

   query.heaps.array = (__u64)array;

   /* Get the array */
   result = pvr_ioctlf(drm_ws->base.render_fd,
                       DRM_IOCTL_PVR_DEV_QUERY,
                       &args,
                       VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to fetch heap info array");
   if (result != VK_SUCCESS)
      goto out_free_array;

   result = pvr_drm_get_heap_static_data_descriptions(drm_ws,
                                                      static_data_descriptions);
   if (result != VK_SUCCESS)
      goto out_free_array;

   for (; i < query.heaps.count; i++) {
      const bool present = array[i].size;
      const pvr_dev_addr_t base_addr = PVR_DEV_ADDR(array[i].base);
      const pvr_dev_addr_t vma_heap_begin_addr =
         PVR_DEV_ADDR_OFFSET(base_addr, static_data_descriptions[i].total_size);
      const uint64_t vma_heap_size =
         array[i].size - static_data_descriptions[i].total_size;

      /* Optional heaps */
      switch (i) {
      case DRM_PVR_HEAP_RGNHDR:
         drm_ws->rgn_hdr_heap_present = present;
         if (!present)
            continue;
         break;
      default:
         break;
      }

      /* Required heaps */
      if (!present) {
         result = vk_errorf(NULL,
                            VK_ERROR_INITIALIZATION_FAILED,
                            "Required heap not present: %d.",
                            i);
         goto err_pvr_drm_heap_finish_all_heaps;
      }

      assert(base_addr.addr);
      assert(static_data_descriptions[i].total_size <= array[i].size);

      winsys_heaps[i]->ws = &drm_ws->base;
      winsys_heaps[i]->base_addr = base_addr;
      winsys_heaps[i]->static_data_carveout_addr = base_addr;
      winsys_heaps[i]->size = array[i].size;
      winsys_heaps[i]->static_data_carveout_size =
         static_data_descriptions[i].total_size;
      winsys_heaps[i]->page_size = 1 << array[i].page_size_log2;
      winsys_heaps[i]->log2_page_size = array[i].page_size_log2;

      /* For now we don't support the heap page size being different from the
       * host page size.
       */
      assert(winsys_heaps[i]->page_size == drm_ws->base.page_size);
      assert(winsys_heaps[i]->log2_page_size == drm_ws->base.log2_page_size);

      winsys_heaps[i]->static_data_offsets =
         static_data_descriptions[i].offsets;

      util_vma_heap_init(&winsys_heaps[i]->vma_heap,
                         vma_heap_begin_addr.addr,
                         vma_heap_size);

      winsys_heaps[i]->vma_heap.alloc_high = false;

      /* It's expected that the heap destroy function to be the last thing that
       * is called, so we start the ref_count at 0.
       */
      p_atomic_set(&winsys_heaps[i]->ref_count, 0);

      if (pthread_mutex_init(&winsys_heaps[i]->lock, NULL)) {
         result = vk_error(NULL, VK_ERROR_INITIALIZATION_FAILED);
         goto err_pvr_drm_heap_finish_all_heaps;
      }
   }

   result = VK_SUCCESS;
   goto out_free_array;

err_pvr_drm_heap_finish_all_heaps:
   /* Undo from where we left off */
   while (--i >= 0) {
      /* Optional heaps */
      switch (i) {
      case DRM_PVR_HEAP_RGNHDR:
         if (drm_ws->rgn_hdr_heap_present)
            break;
         continue;
      default:
         break;
      }

      pvr_winsys_helper_winsys_heap_finish(winsys_heaps[i]);
   }

out_free_array:
   vk_free(drm_ws->base.alloc, array);

out:
   return result;
}

VkResult pvr_drm_winsys_create(const int render_fd,
                               const int display_fd,
                               const VkAllocationCallbacks *alloc,
                               struct pvr_winsys **const ws_out)
{
   struct drm_pvr_ioctl_create_vm_context_args create_vm_context_args = { 0 };
   struct drm_pvr_ioctl_destroy_vm_context_args destroy_vm_context_args = { 0 };
   struct drm_pvr_dev_query_gpu_info gpu_info = { 0 };

   struct pvr_drm_winsys *drm_ws;
   VkResult result;

   drm_ws =
      vk_zalloc(alloc, sizeof(*drm_ws), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!drm_ws) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_out;
   }

   drm_ws->base.ops = &drm_winsys_ops;
   drm_ws->base.render_fd = render_fd;
   drm_ws->base.display_fd = display_fd;
   drm_ws->base.alloc = alloc;

   os_get_page_size(&drm_ws->base.page_size);
   drm_ws->base.log2_page_size = util_logbase2(drm_ws->base.page_size);

   drm_ws->base.syncobj_type = vk_drm_syncobj_get_type(render_fd);
   drm_ws->base.sync_types[0] = &drm_ws->base.syncobj_type;
   drm_ws->base.sync_types[1] = NULL;

   result = pvr_drm_get_gpu_info(drm_ws, &gpu_info);
   if (result != VK_SUCCESS)
      goto err_vk_free_drm_ws;

   drm_ws->bvnc = gpu_info.gpu_id;

   result = pvr_ioctl(render_fd,
                      DRM_IOCTL_PVR_CREATE_VM_CONTEXT,
                      &create_vm_context_args,
                      VK_ERROR_INITIALIZATION_FAILED);
   if (result != VK_SUCCESS)
      goto err_pvr_destroy_vm_context;

   drm_ws->vm_context = create_vm_context_args.handle;

   result = pvr_drm_setup_heaps(drm_ws);
   if (result != VK_SUCCESS)
      goto err_pvr_destroy_vm_context;

   result =
      pvr_winsys_helper_allocate_static_memory(&drm_ws->base,
                                               pvr_drm_heap_alloc_carveout,
                                               &drm_ws->general_heap.base,
                                               &drm_ws->pds_heap.base,
                                               &drm_ws->usc_heap.base,
                                               &drm_ws->general_vma,
                                               &drm_ws->pds_vma,
                                               &drm_ws->usc_vma);
   if (result != VK_SUCCESS)
      goto err_pvr_heap_finish;

   result = pvr_winsys_helper_fill_static_memory(&drm_ws->base,
                                                 drm_ws->general_vma,
                                                 drm_ws->pds_vma,
                                                 drm_ws->usc_vma);
   if (result != VK_SUCCESS)
      goto err_pvr_free_static_memory;

   *ws_out = &drm_ws->base;

   return VK_SUCCESS;

err_pvr_free_static_memory:
   pvr_winsys_helper_free_static_memory(drm_ws->general_vma,
                                        drm_ws->pds_vma,
                                        drm_ws->usc_vma);

err_pvr_heap_finish:
   pvr_drm_finish_heaps(drm_ws);

err_pvr_destroy_vm_context:
   destroy_vm_context_args.handle = drm_ws->vm_context;
   pvr_ioctl(render_fd,
             DRM_IOCTL_PVR_DESTROY_VM_CONTEXT,
             &destroy_vm_context_args,
             VK_ERROR_UNKNOWN);

err_vk_free_drm_ws:
   vk_free(alloc, drm_ws);

err_out:
   return result;
}
