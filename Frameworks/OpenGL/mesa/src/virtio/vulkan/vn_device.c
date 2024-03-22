/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_device.h"

#include "util/disk_cache.h"
#include "util/hex.h"
#include "venus-protocol/vn_protocol_driver_device.h"

#include "vn_android.h"
#include "vn_instance.h"
#include "vn_physical_device.h"
#include "vn_queue.h"

/* device commands */

static void
vn_queue_fini(struct vn_queue *queue)
{
   VkDevice dev_handle = vk_device_to_handle(queue->base.base.base.device);

   if (queue->wait_fence != VK_NULL_HANDLE) {
      vn_DestroyFence(dev_handle, queue->wait_fence, NULL);
   }
   if (queue->sparse_semaphore != VK_NULL_HANDLE) {
      vn_DestroySemaphore(dev_handle, queue->sparse_semaphore, NULL);
   }
   vn_queue_base_fini(&queue->base);
}

static VkResult
vn_queue_init(struct vn_device *dev,
              struct vn_queue *queue,
              const VkDeviceQueueCreateInfo *queue_info,
              uint32_t queue_index)
{
   VkResult result =
      vn_queue_base_init(&queue->base, &dev->base, queue_info, queue_index);
   if (result != VK_SUCCESS)
      return result;

   VkDeviceQueueTimelineInfoMESA timeline_info;
   const struct vn_renderer_info *renderer_info =
      &dev->instance->renderer->info;
   if (renderer_info->supports_multiple_timelines) {
      int ring_idx = vn_instance_acquire_ring_idx(dev->instance);
      if (ring_idx < 0) {
         vn_log(dev->instance, "failed binding VkQueue to renderer timeline");
         return VK_ERROR_INITIALIZATION_FAILED;
      }
      queue->ring_idx = (uint32_t)ring_idx;

      timeline_info = (VkDeviceQueueTimelineInfoMESA){
         .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_TIMELINE_INFO_MESA,
         .ringIdx = queue->ring_idx,
      };
   }

   const VkDeviceQueueInfo2 device_queue_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
      .pNext =
         renderer_info->supports_multiple_timelines ? &timeline_info : NULL,
      .flags = queue_info->flags,
      .queueFamilyIndex = queue_info->queueFamilyIndex,
      .queueIndex = queue_index,
   };

   VkQueue queue_handle = vn_queue_to_handle(queue);
   vn_async_vkGetDeviceQueue2(dev->primary_ring, vn_device_to_handle(dev),
                              &device_queue_info, &queue_handle);

   return VK_SUCCESS;
}

static VkResult
vn_device_init_queues(struct vn_device *dev,
                      const VkDeviceCreateInfo *create_info)
{
   const VkAllocationCallbacks *alloc = &dev->base.base.alloc;

   uint32_t count = 0;
   for (uint32_t i = 0; i < create_info->queueCreateInfoCount; i++)
      count += create_info->pQueueCreateInfos[i].queueCount;

   struct vn_queue *queues =
      vk_zalloc(alloc, sizeof(*queues) * count, VN_DEFAULT_ALIGN,
                VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!queues)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   count = 0;
   for (uint32_t i = 0; i < create_info->queueCreateInfoCount; i++) {
      VkResult result;

      const VkDeviceQueueCreateInfo *queue_info =
         &create_info->pQueueCreateInfos[i];
      for (uint32_t j = 0; j < queue_info->queueCount; j++) {
         result = vn_queue_init(dev, &queues[count], queue_info, j);
         if (result != VK_SUCCESS) {
            for (uint32_t k = 0; k < count; k++)
               vn_queue_fini(&queues[k]);
            vk_free(alloc, queues);

            return result;
         }

         count++;
      }
   }

   dev->queues = queues;
   dev->queue_count = count;

   return VK_SUCCESS;
}

static bool
vn_device_queue_family_init(struct vn_device *dev,
                            const VkDeviceCreateInfo *create_info)
{
   const VkAllocationCallbacks *alloc = &dev->base.base.alloc;
   uint32_t *queue_families = NULL;
   uint32_t count = 0;

   queue_families = vk_zalloc(
      alloc, sizeof(*queue_families) * create_info->queueCreateInfoCount,
      VN_DEFAULT_ALIGN, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!queue_families)
      return false;

   for (uint32_t i = 0; i < create_info->queueCreateInfoCount; i++) {
      const uint32_t index =
         create_info->pQueueCreateInfos[i].queueFamilyIndex;
      bool new_index = true;

      for (uint32_t j = 0; j < count; j++) {
         if (queue_families[j] == index) {
            new_index = false;
            break;
         }
      }
      if (new_index)
         queue_families[count++] = index;
   }

   dev->queue_families = queue_families;
   dev->queue_family_count = count;

   return true;
}

static inline void
vn_device_queue_family_fini(struct vn_device *dev)
{
   vk_free(&dev->base.base.alloc, dev->queue_families);
}

static VkResult
vn_device_memory_report_init(struct vn_device *dev,
                             const VkDeviceCreateInfo *create_info)
{
   const struct vk_features *app_feats = &dev->base.base.enabled_features;
   if (!app_feats->deviceMemoryReport)
      return VK_SUCCESS;

   uint32_t count = 0;
   vk_foreach_struct_const(pnext, create_info->pNext) {
      if (pnext->sType ==
          VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT)
         count++;
   }

   struct vn_device_memory_report *mem_reports = NULL;
   if (count) {
      mem_reports =
         vk_alloc(&dev->base.base.alloc, sizeof(*mem_reports) * count,
                  VN_DEFAULT_ALIGN, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
      if (!mem_reports)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   count = 0;
   vk_foreach_struct_const(pnext, create_info->pNext) {
      if (pnext->sType ==
          VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT) {
         const struct VkDeviceDeviceMemoryReportCreateInfoEXT *report =
            (void *)pnext;
         mem_reports[count].callback = report->pfnUserCallback;
         mem_reports[count].data = report->pUserData;
         count++;
      }
   }

   dev->memory_report_count = count;
   dev->memory_reports = mem_reports;

   return VK_SUCCESS;
}

static inline void
vn_device_memory_report_fini(struct vn_device *dev)
{
   vk_free(&dev->base.base.alloc, dev->memory_reports);
}

static bool
find_extension_names(const char *const *exts,
                     uint32_t ext_count,
                     const char *name)
{
   for (uint32_t i = 0; i < ext_count; i++) {
      if (!strcmp(exts[i], name))
         return true;
   }
   return false;
}

static bool
merge_extension_names(const char *const *exts,
                      uint32_t ext_count,
                      const char *const *extra_exts,
                      uint32_t extra_count,
                      const char *const *block_exts,
                      uint32_t block_count,
                      const VkAllocationCallbacks *alloc,
                      const char *const **out_exts,
                      uint32_t *out_count)
{
   const char **merged =
      vk_alloc(alloc, sizeof(*merged) * (ext_count + extra_count),
               VN_DEFAULT_ALIGN, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!merged)
      return false;

   uint32_t count = 0;
   for (uint32_t i = 0; i < ext_count; i++) {
      if (!find_extension_names(block_exts, block_count, exts[i]))
         merged[count++] = exts[i];
   }
   for (uint32_t i = 0; i < extra_count; i++) {
      if (!find_extension_names(exts, ext_count, extra_exts[i]))
         merged[count++] = extra_exts[i];
   }

   *out_exts = merged;
   *out_count = count;
   return true;
}

static const VkDeviceCreateInfo *
vn_device_fix_create_info(const struct vn_device *dev,
                          const VkDeviceCreateInfo *dev_info,
                          const VkAllocationCallbacks *alloc,
                          VkDeviceCreateInfo *local_info)
{
   const struct vn_physical_device *physical_dev = dev->physical_device;
   const struct vk_device_extension_table *app_exts =
      &dev->base.base.enabled_extensions;
   /* extra_exts and block_exts must not overlap */
   const char *extra_exts[16];
   const char *block_exts[16];
   uint32_t extra_count = 0;
   uint32_t block_count = 0;

   /* fix for WSI (treat AHB as WSI extension for simplicity) */
   const bool has_wsi =
      app_exts->KHR_swapchain || app_exts->ANDROID_native_buffer ||
      app_exts->ANDROID_external_memory_android_hardware_buffer;
   if (has_wsi) {
      if (!app_exts->EXT_image_drm_format_modifier) {
         extra_exts[extra_count++] =
            VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME;

         if (physical_dev->renderer_version < VK_API_VERSION_1_2 &&
             !app_exts->KHR_image_format_list) {
            extra_exts[extra_count++] =
               VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME;
         }
      }

      if (!app_exts->EXT_queue_family_foreign) {
         extra_exts[extra_count++] =
            VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME;
      }

      if (app_exts->KHR_swapchain) {
         /* see vn_physical_device_get_native_extensions */
         block_exts[block_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
         block_exts[block_count++] =
            VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME;
         block_exts[block_count++] =
            VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME;
      }

      if (app_exts->ANDROID_native_buffer) {
         /* see vn_QueueSignalReleaseImageANDROID */
         if (!app_exts->KHR_external_fence_fd) {
            assert(physical_dev->renderer_sync_fd.fence_exportable);
            extra_exts[extra_count++] =
               VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
         }

         block_exts[block_count++] = VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME;
      }

      if (app_exts->ANDROID_external_memory_android_hardware_buffer) {
         block_exts[block_count++] =
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME;
      }
   }

   if (app_exts->KHR_external_memory_fd ||
       app_exts->EXT_external_memory_dma_buf || has_wsi) {
      if (physical_dev->external_memory.renderer_handle_type ==
          VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT) {
         if (!app_exts->EXT_external_memory_dma_buf) {
            extra_exts[extra_count++] =
               VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME;
         }
         if (!app_exts->KHR_external_memory_fd) {
            extra_exts[extra_count++] =
               VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
         }
      }
   }

   /* see vn_queue_submission_count_batch_semaphores */
   if (!app_exts->KHR_external_semaphore_fd && has_wsi) {
      assert(physical_dev->renderer_sync_fd.semaphore_importable);
      extra_exts[extra_count++] = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
   }

   if (app_exts->EXT_device_memory_report) {
      /* see vn_physical_device_get_native_extensions */
      block_exts[block_count++] = VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME;
   }

   if (app_exts->EXT_physical_device_drm) {
      /* see vn_physical_device_get_native_extensions */
      block_exts[block_count++] = VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME;
   }

   if (app_exts->EXT_tooling_info) {
      /* see vn_physical_device_get_native_extensions */
      block_exts[block_count++] = VK_EXT_TOOLING_INFO_EXTENSION_NAME;
   }

   if (app_exts->EXT_pci_bus_info) {
      /* always filter for simplicity */
      block_exts[block_count++] = VK_EXT_PCI_BUS_INFO_EXTENSION_NAME;
   }

   assert(extra_count <= ARRAY_SIZE(extra_exts));
   assert(block_count <= ARRAY_SIZE(block_exts));

   if (!extra_count && (!block_count || !dev_info->enabledExtensionCount))
      return dev_info;

   *local_info = *dev_info;
   if (!merge_extension_names(dev_info->ppEnabledExtensionNames,
                              dev_info->enabledExtensionCount, extra_exts,
                              extra_count, block_exts, block_count, alloc,
                              &local_info->ppEnabledExtensionNames,
                              &local_info->enabledExtensionCount))
      return NULL;

   return local_info;
}

static inline VkResult
vn_device_feedback_pool_init(struct vn_device *dev)
{
   /* The feedback pool defaults to suballocate slots of 8 bytes each. Initial
    * pool size of 4096 corresponds to a total of 512 fences, semaphores and
    * events, which well covers the common scenarios. Pool can grow anyway.
    */
   static const uint32_t pool_size = 4096;
   const VkAllocationCallbacks *alloc = &dev->base.base.alloc;

   if (VN_PERF(NO_EVENT_FEEDBACK) && VN_PERF(NO_FENCE_FEEDBACK) &&
       VN_PERF(NO_TIMELINE_SEM_FEEDBACK))
      return VK_SUCCESS;

   return vn_feedback_pool_init(dev, &dev->feedback_pool, pool_size, alloc);
}

static inline void
vn_device_feedback_pool_fini(struct vn_device *dev)
{
   if (VN_PERF(NO_EVENT_FEEDBACK) && VN_PERF(NO_FENCE_FEEDBACK) &&
       VN_PERF(NO_TIMELINE_SEM_FEEDBACK))
      return;

   vn_feedback_pool_fini(&dev->feedback_pool);
}

static void
vn_device_update_shader_cache_id(struct vn_device *dev)
{
   /* venus utilizes the host side shader cache.
    * This is a WA to generate shader cache files containing headers
    * with a unique cache id that will change based on host driver
    * identifiers. This allows fossilize replay to detect if the host
    * side shader cach is no longer up to date.
    * The shader cache is destroyed after creating the necessary files
    * and not utilized by venus.
    */
#if !defined(ANDROID) && defined(ENABLE_SHADER_CACHE)
   const VkPhysicalDeviceProperties *vulkan_1_0_props =
      &dev->physical_device->properties.vulkan_1_0;

   char uuid[VK_UUID_SIZE * 2 + 1];
   mesa_bytes_to_hex(uuid, vulkan_1_0_props->pipelineCacheUUID, VK_UUID_SIZE);

   struct disk_cache *cache = disk_cache_create("venus", uuid, 0);
   if (!cache)
      return;

   /* The entry header is what contains the cache id / timestamp so we
    * need to create a fake entry.
    */
   uint8_t key[20];
   char data[] = "Fake Shader";

   disk_cache_compute_key(cache, data, sizeof(data), key);
   disk_cache_put(cache, key, data, sizeof(data), NULL);

   disk_cache_destroy(cache);
#endif
}

static VkResult
vn_device_init(struct vn_device *dev,
               struct vn_physical_device *physical_dev,
               const VkDeviceCreateInfo *create_info,
               const VkAllocationCallbacks *alloc)
{
   struct vn_instance *instance = physical_dev->instance;
   VkPhysicalDevice physical_dev_handle =
      vn_physical_device_to_handle(physical_dev);
   VkDevice dev_handle = vn_device_to_handle(dev);
   VkDeviceCreateInfo local_create_info;
   VkResult result;

   dev->instance = instance;
   dev->physical_device = physical_dev;
   dev->renderer = instance->renderer;
   dev->primary_ring = instance->ring.ring;

   create_info =
      vn_device_fix_create_info(dev, create_info, alloc, &local_create_info);
   if (!create_info)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   result = vn_call_vkCreateDevice(dev->primary_ring, physical_dev_handle,
                                   create_info, NULL, &dev_handle);

   /* free the fixed extensions here since no longer needed below */
   if (create_info == &local_create_info)
      vk_free(alloc, (void *)create_info->ppEnabledExtensionNames);

   if (result != VK_SUCCESS)
      return result;

   result = vn_device_memory_report_init(dev, create_info);
   if (result != VK_SUCCESS)
      goto out_destroy_device;

   if (!vn_device_queue_family_init(dev, create_info)) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto out_memory_report_fini;
   }

   for (uint32_t i = 0; i < ARRAY_SIZE(dev->memory_pools); i++) {
      struct vn_device_memory_pool *pool = &dev->memory_pools[i];
      mtx_init(&pool->mutex, mtx_plain);
   }

   result = vn_device_feedback_pool_init(dev);
   if (result != VK_SUCCESS)
      goto out_memory_pool_fini;

   result = vn_feedback_cmd_pools_init(dev);
   if (result != VK_SUCCESS)
      goto out_feedback_pool_fini;

   result = vn_device_init_queues(dev, create_info);
   if (result != VK_SUCCESS)
      goto out_cmd_pools_fini;

   vn_buffer_reqs_cache_init(dev);
   vn_image_reqs_cache_init(dev);

   /* This is a WA to allow fossilize replay to detect if the host side shader
    * cache is no longer up to date.
    */
   vn_device_update_shader_cache_id(dev);

   return VK_SUCCESS;

out_cmd_pools_fini:
   vn_feedback_cmd_pools_fini(dev);

out_feedback_pool_fini:
   vn_device_feedback_pool_fini(dev);

out_memory_pool_fini:
   for (uint32_t i = 0; i < ARRAY_SIZE(dev->memory_pools); i++)
      vn_device_memory_pool_fini(dev, i);

   vn_device_queue_family_fini(dev);

out_memory_report_fini:
   vn_device_memory_report_fini(dev);

out_destroy_device:
   vn_call_vkDestroyDevice(dev->primary_ring, dev_handle, NULL);

   return result;
}

VkResult
vn_CreateDevice(VkPhysicalDevice physicalDevice,
                const VkDeviceCreateInfo *pCreateInfo,
                const VkAllocationCallbacks *pAllocator,
                VkDevice *pDevice)
{
   VN_TRACE_FUNC();
   struct vn_physical_device *physical_dev =
      vn_physical_device_from_handle(physicalDevice);
   struct vn_instance *instance = physical_dev->instance;
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &instance->base.base.alloc;
   struct vn_device *dev;
   VkResult result;

   dev = vk_zalloc(alloc, sizeof(*dev), VN_DEFAULT_ALIGN,
                   VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!dev)
      return vn_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_device_dispatch_table dispatch_table;
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
                                             &vn_device_entrypoints, true);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
                                             &wsi_device_entrypoints, false);
   result = vn_device_base_init(&dev->base, &physical_dev->base,
                                &dispatch_table, pCreateInfo, alloc);
   if (result != VK_SUCCESS) {
      vk_free(alloc, dev);
      return vn_error(instance, result);
   }

   result = vn_device_init(dev, physical_dev, pCreateInfo, alloc);
   if (result != VK_SUCCESS) {
      vn_device_base_fini(&dev->base);
      vk_free(alloc, dev);
      return vn_error(instance, result);
   }

   if (VN_DEBUG(LOG_CTX_INFO)) {
      vn_log(instance, "%s", physical_dev->properties.vulkan_1_0.deviceName);
      vn_log(instance, "%s", physical_dev->properties.vulkan_1_2.driverInfo);
   }

   vn_tls_set_async_pipeline_create();

   *pDevice = vn_device_to_handle(dev);

   return VK_SUCCESS;
}

void
vn_DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   if (!dev)
      return;

   vn_image_reqs_cache_fini(dev);
   vn_buffer_reqs_cache_fini(dev);

   for (uint32_t i = 0; i < dev->queue_count; i++)
      vn_queue_fini(&dev->queues[i]);

   vn_feedback_cmd_pools_fini(dev);

   vn_device_feedback_pool_fini(dev);

   for (uint32_t i = 0; i < ARRAY_SIZE(dev->memory_pools); i++)
      vn_device_memory_pool_fini(dev, i);

   vn_device_queue_family_fini(dev);

   vn_device_memory_report_fini(dev);

   /* We must emit vkDestroyDevice before freeing dev->queues.  Otherwise,
    * another thread might reuse their object ids while they still refer to
    * the queues in the renderer.
    */
   vn_async_vkDestroyDevice(dev->primary_ring, device, NULL);

   /* We must emit vn_call_vkDestroyDevice before releasing bound ring_idx.
    * Otherwise, another thread might reuse their ring_idx while they
    * are still bound to the queues in the renderer.
    */
   if (dev->renderer->info.supports_multiple_timelines) {
      for (uint32_t i = 0; i < dev->queue_count; i++) {
         vn_instance_release_ring_idx(dev->instance, dev->queues[i].ring_idx);
      }
   }

   vk_free(alloc, dev->queues);

   vn_device_base_fini(&dev->base);
   vk_free(alloc, dev);
}

PFN_vkVoidFunction
vn_GetDeviceProcAddr(VkDevice device, const char *pName)
{
   struct vn_device *dev = vn_device_from_handle(device);
   return vk_device_get_proc_addr(&dev->base.base, pName);
}

void
vn_GetDeviceGroupPeerMemoryFeatures(
   VkDevice device,
   uint32_t heapIndex,
   uint32_t localDeviceIndex,
   uint32_t remoteDeviceIndex,
   VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
   struct vn_device *dev = vn_device_from_handle(device);

   /* TODO get and cache the values in vkCreateDevice */
   vn_call_vkGetDeviceGroupPeerMemoryFeatures(
      dev->primary_ring, device, heapIndex, localDeviceIndex,
      remoteDeviceIndex, pPeerMemoryFeatures);
}

VkResult
vn_GetCalibratedTimestampsEXT(
   VkDevice device,
   uint32_t timestampCount,
   const VkCalibratedTimestampInfoEXT *pTimestampInfos,
   uint64_t *pTimestamps,
   uint64_t *pMaxDeviation)
{
   struct vn_device *dev = vn_device_from_handle(device);
   uint64_t begin, end, max_clock_period = 0;
   VkResult ret;
   int domain;

#ifdef CLOCK_MONOTONIC_RAW
   begin = vk_clock_gettime(CLOCK_MONOTONIC_RAW);
#else
   begin = vk_clock_gettime(CLOCK_MONOTONIC);
#endif

   for (domain = 0; domain < timestampCount; domain++) {
      switch (pTimestampInfos[domain].timeDomain) {
      case VK_TIME_DOMAIN_DEVICE_EXT: {
         uint64_t device_max_deviation = 0;

         ret = vn_call_vkGetCalibratedTimestampsEXT(
            dev->primary_ring, device, 1, &pTimestampInfos[domain],
            &pTimestamps[domain], &device_max_deviation);

         if (ret != VK_SUCCESS)
            return vn_error(dev->instance, ret);

         max_clock_period = MAX2(max_clock_period, device_max_deviation);
         break;
      }
      case VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT:
         pTimestamps[domain] = vk_clock_gettime(CLOCK_MONOTONIC);
         max_clock_period = MAX2(max_clock_period, 1);
         break;
#ifdef CLOCK_MONOTONIC_RAW
      case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT:
         pTimestamps[domain] = begin;
         break;
#endif
      default:
         pTimestamps[domain] = 0;
         break;
      }
   }

#ifdef CLOCK_MONOTONIC_RAW
   end = vk_clock_gettime(CLOCK_MONOTONIC_RAW);
#else
   end = vk_clock_gettime(CLOCK_MONOTONIC);
#endif

   *pMaxDeviation = vk_time_max_deviation(begin, end, max_clock_period);

   return VK_SUCCESS;
}
