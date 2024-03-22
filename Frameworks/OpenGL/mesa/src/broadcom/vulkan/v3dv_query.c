/*
 * Copyright © 2020 Raspberry Pi Ltd
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

#include "v3dv_private.h"

#include "util/timespec.h"
#include "compiler/nir/nir_builder.h"

static void
kperfmon_create(struct v3dv_device *device,
                struct v3dv_query_pool *pool,
                uint32_t query)
{
   for (uint32_t i = 0; i < pool->perfmon.nperfmons; i++) {
      assert(i * DRM_V3D_MAX_PERF_COUNTERS < pool->perfmon.ncounters);

      struct drm_v3d_perfmon_create req = {
         .ncounters = MIN2(pool->perfmon.ncounters -
                           i * DRM_V3D_MAX_PERF_COUNTERS,
                           DRM_V3D_MAX_PERF_COUNTERS),
      };
      memcpy(req.counters,
             &pool->perfmon.counters[i * DRM_V3D_MAX_PERF_COUNTERS],
             req.ncounters);

      int ret = v3dv_ioctl(device->pdevice->render_fd,
                           DRM_IOCTL_V3D_PERFMON_CREATE,
                           &req);
      if (ret)
         fprintf(stderr, "Failed to create perfmon for query %d: %s\n", query, strerror(ret));

      pool->queries[query].perf.kperfmon_ids[i] = req.id;
   }
}

static void
kperfmon_destroy(struct v3dv_device *device,
                 struct v3dv_query_pool *pool,
                 uint32_t query)
{
   /* Skip destroying if never created */
   if (!pool->queries[query].perf.kperfmon_ids[0])
      return;

   for (uint32_t i = 0; i < pool->perfmon.nperfmons; i++) {
      struct drm_v3d_perfmon_destroy req = {
         .id = pool->queries[query].perf.kperfmon_ids[i]
      };

      int ret = v3dv_ioctl(device->pdevice->render_fd,
                           DRM_IOCTL_V3D_PERFMON_DESTROY,
                           &req);

      if (ret) {
         fprintf(stderr, "Failed to destroy perfmon %u: %s\n",
                 req.id, strerror(ret));
      }
   }
}

/**
 * Creates a VkBuffer (and VkDeviceMemory) to access a BO.
 */
static VkResult
create_vk_storage_buffer(struct v3dv_device *device,
                         struct v3dv_bo *bo,
                         VkBuffer *vk_buf,
                         VkDeviceMemory *vk_mem)
{
   VkDevice vk_device = v3dv_device_to_handle(device);

   VkBufferCreateInfo buf_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = bo->size,
      .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
   };
   VkResult result = v3dv_CreateBuffer(vk_device, &buf_info, NULL, vk_buf);
   if (result != VK_SUCCESS)
      return result;

   struct v3dv_device_memory *mem =
      vk_object_zalloc(&device->vk, NULL, sizeof(*mem),
                       VK_OBJECT_TYPE_DEVICE_MEMORY);
   if (!mem)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   mem->bo = bo;
   mem->type = &device->pdevice->memory.memoryTypes[0];

   *vk_mem = v3dv_device_memory_to_handle(mem);
   VkBindBufferMemoryInfo bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
      .buffer = *vk_buf,
      .memory = *vk_mem,
      .memoryOffset = 0,
   };
   v3dv_BindBufferMemory2(vk_device, 1, &bind_info);

   return VK_SUCCESS;
}

static void
destroy_vk_storage_buffer(struct v3dv_device *device,
                          VkBuffer *vk_buf,
                          VkDeviceMemory *vk_mem)
{
   if (*vk_mem) {
      vk_object_free(&device->vk, NULL, v3dv_device_memory_from_handle(*vk_mem));
      *vk_mem = VK_NULL_HANDLE;
   }

   v3dv_DestroyBuffer(v3dv_device_to_handle(device), *vk_buf, NULL);
   *vk_buf = VK_NULL_HANDLE;
}

/**
 * Allocates descriptor sets to access query pool BO (availability and
 * occlusion query results) from Vulkan pipelines.
 */
static VkResult
create_pool_descriptors(struct v3dv_device *device,
                        struct v3dv_query_pool *pool)
{
   assert(pool->query_type == VK_QUERY_TYPE_OCCLUSION);
   VkDevice vk_device = v3dv_device_to_handle(device);

   VkDescriptorPoolSize pool_size = {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = 1,
   };
   VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
   };
   VkResult result =
      v3dv_CreateDescriptorPool(vk_device, &pool_info, NULL,
                                &pool->meta.descriptor_pool);

   if (result != VK_SUCCESS)
      return result;

   VkDescriptorSetAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = pool->meta.descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &device->queries.buf_descriptor_set_layout,
   };
   result = v3dv_AllocateDescriptorSets(vk_device, &alloc_info,
                                        &pool->meta.descriptor_set);
   if (result != VK_SUCCESS)
      return result;

   VkDescriptorBufferInfo desc_buf_info = {
      .buffer = pool->meta.buf,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
   };

   VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = pool->meta.descriptor_set,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &desc_buf_info,
   };
   v3dv_UpdateDescriptorSets(vk_device, 1, &write, 0, NULL);

   return VK_SUCCESS;
}

static void
destroy_pool_descriptors(struct v3dv_device *device,
                         struct v3dv_query_pool *pool)
{
   assert(pool->query_type == VK_QUERY_TYPE_OCCLUSION);

   v3dv_FreeDescriptorSets(v3dv_device_to_handle(device),
                           pool->meta.descriptor_pool,
                           1, &pool->meta.descriptor_set);
   pool->meta.descriptor_set = VK_NULL_HANDLE;

   v3dv_DestroyDescriptorPool(v3dv_device_to_handle(device),
                              pool->meta.descriptor_pool, NULL);
   pool->meta.descriptor_pool = VK_NULL_HANDLE;
}

static VkResult
pool_create_meta_resources(struct v3dv_device *device,
                           struct v3dv_query_pool *pool)
{
   VkResult result;

   if (pool->query_type != VK_QUERY_TYPE_OCCLUSION)
      return VK_SUCCESS;

   result = create_vk_storage_buffer(device, pool->occlusion.bo,
                                     &pool->meta.buf, &pool->meta.mem);
   if (result != VK_SUCCESS)
      return result;

   result = create_pool_descriptors(device, pool);
   if (result != VK_SUCCESS)
       return result;

   return VK_SUCCESS;
}

static void
pool_destroy_meta_resources(struct v3dv_device *device,
                            struct v3dv_query_pool *pool)
{
   if (pool->query_type != VK_QUERY_TYPE_OCCLUSION)
      return;

   destroy_pool_descriptors(device, pool);
   destroy_vk_storage_buffer(device, &pool->meta.buf, &pool->meta.mem);
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_CreateQueryPool(VkDevice _device,
                     const VkQueryPoolCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator,
                     VkQueryPool *pQueryPool)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);

   assert(pCreateInfo->queryType == VK_QUERY_TYPE_OCCLUSION ||
          pCreateInfo->queryType == VK_QUERY_TYPE_TIMESTAMP ||
          pCreateInfo->queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR);
   assert(pCreateInfo->queryCount > 0);

   struct v3dv_query_pool *pool =
      vk_object_zalloc(&device->vk, pAllocator, sizeof(*pool),
                       VK_OBJECT_TYPE_QUERY_POOL);
   if (pool == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pool->query_type = pCreateInfo->queryType;
   pool->query_count = pCreateInfo->queryCount;

   uint32_t query_idx = 0;
   VkResult result;

   const uint32_t pool_bytes = sizeof(struct v3dv_query) * pool->query_count;
   pool->queries = vk_alloc2(&device->vk.alloc, pAllocator, pool_bytes, 8,
                             VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (pool->queries == NULL) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail;
   }

   switch (pool->query_type) {
   case VK_QUERY_TYPE_OCCLUSION: {
      /* The hardware allows us to setup groups of 16 queries in consecutive
       * 4-byte addresses, requiring only that each group of 16 queries is
       * aligned to a 1024 byte boundary.
       */
      const uint32_t query_groups = DIV_ROUND_UP(pool->query_count, 16);
      uint32_t bo_size = query_groups * 1024;
      /* After the counters we store avalability data, 1 byte/query */
      pool->occlusion.avail_offset = bo_size;
      bo_size += pool->query_count;
      pool->occlusion.bo = v3dv_bo_alloc(device, bo_size, "query:o", true);
      if (!pool->occlusion.bo) {
         result = vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         goto fail;
      }
      if (!v3dv_bo_map(device, pool->occlusion.bo, bo_size)) {
         result = vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         goto fail;
      }
      break;
   }
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      const VkQueryPoolPerformanceCreateInfoKHR *pq_info =
         vk_find_struct_const(pCreateInfo->pNext,
                              QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR);

      assert(pq_info);

      pool->perfmon.ncounters = pq_info->counterIndexCount;
      for (uint32_t i = 0; i < pq_info->counterIndexCount; i++)
         pool->perfmon.counters[i] = pq_info->pCounterIndices[i];

      pool->perfmon.nperfmons = DIV_ROUND_UP(pool->perfmon.ncounters,
                                             DRM_V3D_MAX_PERF_COUNTERS);

      assert(pool->perfmon.nperfmons <= V3DV_MAX_PERFMONS);
      break;
   }
   case VK_QUERY_TYPE_TIMESTAMP: {
      /* 8 bytes per query used for the timestamp value. We have all
       * timestamps tightly packed first in the buffer.
       */
      const uint32_t bo_size = pool->query_count * 8;
      pool->timestamp.bo = v3dv_bo_alloc(device, bo_size, "query:t", true);
      if (!pool->timestamp.bo) {
         result = vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         goto fail;
      }
      if (!v3dv_bo_map(device, pool->timestamp.bo, bo_size)) {
         result = vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         goto fail;
      }
      break;
   }
   default:
      unreachable("Unsupported query type");
   }

   /* Initialize queries in the pool */
   for (; query_idx < pool->query_count; query_idx++) {
      pool->queries[query_idx].maybe_available = false;
      switch (pool->query_type) {
      case VK_QUERY_TYPE_OCCLUSION: {
         const uint32_t query_group = query_idx / 16;
         const uint32_t query_offset = query_group * 1024 + (query_idx % 16) * 4;
         pool->queries[query_idx].occlusion.offset = query_offset;
         break;
         }
      case VK_QUERY_TYPE_TIMESTAMP:
         pool->queries[query_idx].timestamp.offset = query_idx * 8;
         result = vk_sync_create(&device->vk,
                                 &device->pdevice->drm_syncobj_type, 0, 0,
                                 &pool->queries[query_idx].timestamp.sync);
         if (result != VK_SUCCESS)
            goto fail;
         break;
      case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
         result = vk_sync_create(&device->vk,
                                 &device->pdevice->drm_syncobj_type, 0, 0,
                                 &pool->queries[query_idx].perf.last_job_sync);
         if (result != VK_SUCCESS)
            goto fail;

         kperfmon_create(device, pool, query_idx);
         break;
         }
      default:
         unreachable("Unsupported query type");
      }
   }

   /* Create meta resources */
   result = pool_create_meta_resources(device, pool);
   if (result != VK_SUCCESS)
      goto fail;

   *pQueryPool = v3dv_query_pool_to_handle(pool);

   return VK_SUCCESS;

fail:
   if (pool->query_type == VK_QUERY_TYPE_TIMESTAMP) {
      for (uint32_t j = 0; j < query_idx; j++)
         vk_sync_destroy(&device->vk, pool->queries[j].timestamp.sync);
   }

   if (pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
      for (uint32_t j = 0; j < query_idx; j++)
         vk_sync_destroy(&device->vk, pool->queries[j].perf.last_job_sync);
   }

   if (pool->occlusion.bo)
      v3dv_bo_free(device, pool->occlusion.bo);
   if (pool->timestamp.bo)
      v3dv_bo_free(device, pool->timestamp.bo);
   if (pool->queries)
      vk_free2(&device->vk.alloc, pAllocator, pool->queries);
   pool_destroy_meta_resources(device, pool);
   vk_object_free(&device->vk, pAllocator, pool);

   return result;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_DestroyQueryPool(VkDevice _device,
                      VkQueryPool queryPool,
                      const VkAllocationCallbacks *pAllocator)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   if (!pool)
      return;

   if (pool->occlusion.bo)
      v3dv_bo_free(device, pool->occlusion.bo);

   if (pool->timestamp.bo)
      v3dv_bo_free(device, pool->timestamp.bo);

   if (pool->query_type == VK_QUERY_TYPE_TIMESTAMP) {
      for (uint32_t i = 0; i < pool->query_count; i++)
         vk_sync_destroy(&device->vk, pool->queries[i].timestamp.sync);
   }

   if (pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
      for (uint32_t i = 0; i < pool->query_count; i++) {
         kperfmon_destroy(device, pool, i);
         vk_sync_destroy(&device->vk, pool->queries[i].perf.last_job_sync);
      }
   }

   if (pool->queries)
      vk_free2(&device->vk.alloc, pAllocator, pool->queries);

   pool_destroy_meta_resources(device, pool);

   vk_object_free(&device->vk, pAllocator, pool);
}

static void
write_to_buffer(void *dst, uint32_t idx, bool do_64bit, uint64_t value)
{
   if (do_64bit) {
      uint64_t *dst64 = (uint64_t *) dst;
      dst64[idx] = value;
   } else {
      uint32_t *dst32 = (uint32_t *) dst;
      dst32[idx] = (uint32_t) value;
   }
}

static VkResult
query_wait_available(struct v3dv_device *device,
                     struct v3dv_query_pool *pool,
                     struct v3dv_query *q,
                     uint32_t query_idx)
{
   /* For occlusion queries we prefer to poll the availability BO in a loop
    * to waiting on the query results BO, because the latter would
    * make us wait for any job running queries from the pool, even if those
    * queries do not involve the one we want to wait on.
    */
   if (pool->query_type == VK_QUERY_TYPE_OCCLUSION) {
      uint8_t *q_addr = ((uint8_t *) pool->occlusion.bo->map) +
                        pool->occlusion.avail_offset + query_idx;
      while (*q_addr == 0)
         usleep(250);
      return VK_SUCCESS;
   }

   if (pool->query_type == VK_QUERY_TYPE_TIMESTAMP) {
      if (vk_sync_wait(&device->vk, q->timestamp.sync,
                       0, VK_SYNC_WAIT_COMPLETE, UINT64_MAX) != VK_SUCCESS) {
         return vk_device_set_lost(&device->vk, "Query job wait failed");
      }
      return VK_SUCCESS;
   }

   assert(pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR);

   /* For performance queries we need to wait for the queue to signal that
    * the query has been submitted for execution before anything else.
    */
   VkResult result = VK_SUCCESS;
   if (!q->maybe_available) {
      struct timespec timeout;
      timespec_get(&timeout, TIME_UTC);
      timespec_add_msec(&timeout, &timeout, 2000);

      mtx_lock(&device->query_mutex);
      while (!q->maybe_available) {
         if (vk_device_is_lost(&device->vk)) {
            result = VK_ERROR_DEVICE_LOST;
            break;
         }

         int ret = cnd_timedwait(&device->query_ended,
                                 &device->query_mutex,
                                 &timeout);
         if (ret != thrd_success) {
            mtx_unlock(&device->query_mutex);
            result = vk_device_set_lost(&device->vk, "Query wait failed");
            break;
         }
      }
      mtx_unlock(&device->query_mutex);

      if (result != VK_SUCCESS)
         return result;

      /* For performance queries, we also need to wait for the relevant syncobj
       * to be signaled to ensure completion of the GPU work.
       */
      if (pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR &&
          vk_sync_wait(&device->vk, q->perf.last_job_sync,
                       0, VK_SYNC_WAIT_COMPLETE, UINT64_MAX) != VK_SUCCESS) {
        return vk_device_set_lost(&device->vk, "Query job wait failed");
      }
   }

   return result;
}

static VkResult
query_check_available(struct v3dv_device *device,
                      struct v3dv_query_pool *pool,
                      struct v3dv_query *q,
                      uint32_t query_idx)
{
   /* For occlusion we check the availability BO */
   if (pool->query_type == VK_QUERY_TYPE_OCCLUSION) {
      const uint8_t *q_addr = ((uint8_t *) pool->occlusion.bo->map) +
                              pool->occlusion.avail_offset + query_idx;
      return (*q_addr != 0) ? VK_SUCCESS : VK_NOT_READY;
   }

   /* For timestamp queries, we need to check if the relevant job
    * has completed.
    */
   if (pool->query_type == VK_QUERY_TYPE_TIMESTAMP) {
      if (vk_sync_wait(&device->vk, q->timestamp.sync,
                       0, VK_SYNC_WAIT_COMPLETE, 0) != VK_SUCCESS) {
         return VK_NOT_READY;
      }
      return VK_SUCCESS;
   }

   /* For other queries we need to check if the queue has submitted the query
    * for execution at all.
    */
   assert(pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR);
   if (!q->maybe_available)
      return VK_NOT_READY;

   /* For performance queries, we also need to check if the relevant GPU job
    * has completed.
    */
   if (pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR &&
       vk_sync_wait(&device->vk, q->perf.last_job_sync,
                    0, VK_SYNC_WAIT_COMPLETE, 0) != VK_SUCCESS) {
         return VK_NOT_READY;
   }

   return VK_SUCCESS;
}

static VkResult
query_is_available(struct v3dv_device *device,
                   struct v3dv_query_pool *pool,
                   uint32_t query,
                   bool do_wait,
                   bool *available)
{
   struct v3dv_query *q = &pool->queries[query];

   if (do_wait) {
      VkResult result = query_wait_available(device, pool, q, query);
      if (result != VK_SUCCESS) {
         *available = false;
         return result;
      }

      *available = true;
   } else {
      VkResult result = query_check_available(device, pool, q, query);
      assert(result == VK_SUCCESS || result == VK_NOT_READY);
      *available = (result == VK_SUCCESS);
   }

   return VK_SUCCESS;
}

static VkResult
write_occlusion_query_result(struct v3dv_device *device,
                             struct v3dv_query_pool *pool,
                             uint32_t query,
                             bool do_64bit,
                             void *data,
                             uint32_t slot)
{
   assert(pool && pool->query_type == VK_QUERY_TYPE_OCCLUSION);

   if (vk_device_is_lost(&device->vk))
      return VK_ERROR_DEVICE_LOST;

   struct v3dv_query *q = &pool->queries[query];
   assert(pool->occlusion.bo && pool->occlusion.bo->map);

   const uint8_t *query_addr =
      ((uint8_t *) pool->occlusion.bo->map) + q->occlusion.offset;
   write_to_buffer(data, slot, do_64bit, (uint64_t) *((uint32_t *)query_addr));
   return VK_SUCCESS;
}

static VkResult
write_timestamp_query_result(struct v3dv_device *device,
                             struct v3dv_query_pool *pool,
                             uint32_t query,
                             bool do_64bit,
                             void *data,
                             uint32_t slot)
{
   assert(pool && pool->query_type == VK_QUERY_TYPE_TIMESTAMP);

   struct v3dv_query *q = &pool->queries[query];

   const uint8_t *query_addr =
      ((uint8_t *) pool->timestamp.bo->map) + q->timestamp.offset;

   write_to_buffer(data, slot, do_64bit, *((uint64_t *)query_addr));
   return VK_SUCCESS;
}

static VkResult
write_performance_query_result(struct v3dv_device *device,
                               struct v3dv_query_pool *pool,
                               uint32_t query,
                               bool do_64bit,
                               void *data,
                               uint32_t slot)
{
   assert(pool && pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR);

   struct v3dv_query *q = &pool->queries[query];
   uint64_t counter_values[V3D_MAX_PERFCNT];

   for (uint32_t i = 0; i < pool->perfmon.nperfmons; i++) {
      struct drm_v3d_perfmon_get_values req = {
         .id = q->perf.kperfmon_ids[i],
         .values_ptr = (uintptr_t)(&counter_values[i *
                                   DRM_V3D_MAX_PERF_COUNTERS])
      };

      int ret = v3dv_ioctl(device->pdevice->render_fd,
                           DRM_IOCTL_V3D_PERFMON_GET_VALUES,
                           &req);

      if (ret) {
         fprintf(stderr, "failed to get perfmon values: %s\n", strerror(ret));
         return vk_error(device, VK_ERROR_DEVICE_LOST);
      }
   }

   for (uint32_t i = 0; i < pool->perfmon.ncounters; i++)
      write_to_buffer(data, slot + i, do_64bit, counter_values[i]);

   return VK_SUCCESS;
}

static VkResult
write_query_result(struct v3dv_device *device,
                   struct v3dv_query_pool *pool,
                   uint32_t query,
                   bool do_64bit,
                   void *data,
                   uint32_t slot)
{
   switch (pool->query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
      return write_occlusion_query_result(device, pool, query, do_64bit,
                                          data, slot);
   case VK_QUERY_TYPE_TIMESTAMP:
      return write_timestamp_query_result(device, pool, query, do_64bit,
                                          data, slot);
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
      return write_performance_query_result(device, pool, query, do_64bit,
                                            data, slot);
   default:
      unreachable("Unsupported query type");
   }
}

static uint32_t
get_query_result_count(struct v3dv_query_pool *pool)
{
   switch (pool->query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
   case VK_QUERY_TYPE_TIMESTAMP:
      return 1;
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
      return pool->perfmon.ncounters;
   default:
      unreachable("Unsupported query type");
   }
}

VkResult
v3dv_get_query_pool_results_cpu(struct v3dv_device *device,
                                struct v3dv_query_pool *pool,
                                uint32_t first,
                                uint32_t count,
                                void *data,
                                VkDeviceSize stride,
                                VkQueryResultFlags flags)
{
   assert(first < pool->query_count);
   assert(first + count <= pool->query_count);
   assert(data);

   const bool do_64bit = flags & VK_QUERY_RESULT_64_BIT ||
      pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
   const bool do_wait = flags & VK_QUERY_RESULT_WAIT_BIT;
   const bool do_partial = flags & VK_QUERY_RESULT_PARTIAL_BIT;

   uint32_t result_count = get_query_result_count(pool);

   VkResult result = VK_SUCCESS;
   for (uint32_t i = first; i < first + count; i++) {
      bool available = false;
      VkResult query_result =
         query_is_available(device, pool, i, do_wait, &available);
      if (query_result == VK_ERROR_DEVICE_LOST)
         result = VK_ERROR_DEVICE_LOST;

      /**
       * From the Vulkan 1.0 spec:
       *
       *    "If VK_QUERY_RESULT_WAIT_BIT and VK_QUERY_RESULT_PARTIAL_BIT are
       *     both not set then no result values are written to pData for queries
       *     that are in the unavailable state at the time of the call, and
       *     vkGetQueryPoolResults returns VK_NOT_READY. However, availability
       *     state is still written to pData for those queries if
       *     VK_QUERY_RESULT_WITH_AVAILABILITY_BIT is set."
       */
      uint32_t slot = 0;

      const bool write_result = available || do_partial;
      if (write_result)
         write_query_result(device, pool, i, do_64bit, data, slot);
      slot += result_count;

      if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)
         write_to_buffer(data, slot++, do_64bit, available ? 1u : 0u);

      if (!write_result && result != VK_ERROR_DEVICE_LOST)
         result = VK_NOT_READY;

      data += stride;
   }

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetQueryPoolResults(VkDevice _device,
                         VkQueryPool queryPool,
                         uint32_t firstQuery,
                         uint32_t queryCount,
                         size_t dataSize,
                         void *pData,
                         VkDeviceSize stride,
                         VkQueryResultFlags flags)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   return v3dv_get_query_pool_results_cpu(device, pool, firstQuery, queryCount,
                                          pData, stride, flags);
}

/* Emits a series of vkCmdDispatchBase calls to execute all the workgroups
 * required to handle a number of queries considering per-dispatch limits.
 */
static void
cmd_buffer_emit_dispatch_queries(struct v3dv_cmd_buffer *cmd_buffer,
                                 uint32_t query_count)
{
   VkCommandBuffer vk_cmd_buffer = v3dv_cmd_buffer_to_handle(cmd_buffer);

   uint32_t dispatched = 0;
   const uint32_t max_batch_size = 65535;
   while (dispatched < query_count) {
      uint32_t batch_size = MIN2(query_count - dispatched, max_batch_size);
      v3dv_CmdDispatchBase(vk_cmd_buffer, dispatched, 0, 0, batch_size, 1, 1);
      dispatched += batch_size;
   }
}

void
v3dv_cmd_buffer_emit_set_query_availability(struct v3dv_cmd_buffer *cmd_buffer,
                                            struct v3dv_query_pool *pool,
                                            uint32_t query, uint32_t count,
                                            uint8_t availability)
{
   assert(pool->query_type == VK_QUERY_TYPE_OCCLUSION ||
          pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR);

   struct v3dv_device *device = cmd_buffer->device;
   VkCommandBuffer vk_cmd_buffer = v3dv_cmd_buffer_to_handle(cmd_buffer);

   /* We are about to emit a compute job to set query availability and we need
    * to ensure this executes after the graphics work using the queries has
    * completed.
    */
   VkMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
   };
   VkDependencyInfo barrier_info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .memoryBarrierCount = 1,
      .pMemoryBarriers = &barrier,
   };
   v3dv_cmd_buffer_emit_pipeline_barrier(cmd_buffer, &barrier_info);

   /* Dispatch queries */
   v3dv_cmd_buffer_meta_state_push(cmd_buffer, true);

   v3dv_CmdBindPipeline(vk_cmd_buffer,
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        device->queries.avail_pipeline);

   v3dv_CmdBindDescriptorSets(vk_cmd_buffer,
                              VK_PIPELINE_BIND_POINT_COMPUTE,
                              device->queries.avail_pipeline_layout,
                              0, 1, &pool->meta.descriptor_set,
                              0, NULL);

   struct {
      uint32_t offset;
      uint32_t query;
      uint8_t availability;
   } push_data = { pool->occlusion.avail_offset, query, availability };
   v3dv_CmdPushConstants(vk_cmd_buffer,
                         device->queries.avail_pipeline_layout,
                         VK_SHADER_STAGE_COMPUTE_BIT,
                         0, sizeof(push_data), &push_data);
   cmd_buffer_emit_dispatch_queries(cmd_buffer, count);

   v3dv_cmd_buffer_meta_state_pop(cmd_buffer, false);
}

static void
cmd_buffer_emit_reset_occlusion_query_pool(struct v3dv_cmd_buffer *cmd_buffer,
                                           struct v3dv_query_pool *pool,
                                           uint32_t query, uint32_t count)
{
   struct v3dv_device *device = cmd_buffer->device;
   VkCommandBuffer vk_cmd_buffer = v3dv_cmd_buffer_to_handle(cmd_buffer);

   /* Ensure the GPU is done with the queries in the graphics queue before
    * we reset in the compute queue.
    */
   VkMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
   };
   VkDependencyInfo barrier_info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .memoryBarrierCount = 1,
      .pMemoryBarriers = &barrier,
   };
   v3dv_cmd_buffer_emit_pipeline_barrier(cmd_buffer, &barrier_info);

   /* Emit compute reset */
   v3dv_cmd_buffer_meta_state_push(cmd_buffer, true);

   v3dv_CmdBindPipeline(vk_cmd_buffer,
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        device->queries.reset_occlusion_pipeline);

   v3dv_CmdBindDescriptorSets(vk_cmd_buffer,
                              VK_PIPELINE_BIND_POINT_COMPUTE,
                              device->queries.reset_occlusion_pipeline_layout,
                              0, 1, &pool->meta.descriptor_set,
                              0, NULL);
   struct {
      uint32_t offset;
      uint32_t query;
   } push_data = { pool->occlusion.avail_offset, query };
   v3dv_CmdPushConstants(vk_cmd_buffer,
                         device->queries.reset_occlusion_pipeline_layout,
                         VK_SHADER_STAGE_COMPUTE_BIT,
                         0, sizeof(push_data), &push_data);

   cmd_buffer_emit_dispatch_queries(cmd_buffer, count);

   v3dv_cmd_buffer_meta_state_pop(cmd_buffer, false);

   /* Ensure future work in the graphics queue using the queries doesn't start
    * before the reset completed.
    */
   barrier = (VkMemoryBarrier2) {
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
   };
   barrier_info = (VkDependencyInfo) {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .memoryBarrierCount = 1,
      .pMemoryBarriers = &barrier,
   };
   v3dv_cmd_buffer_emit_pipeline_barrier(cmd_buffer, &barrier_info);
}

static void
cmd_buffer_emit_reset_query_pool(struct v3dv_cmd_buffer *cmd_buffer,
                                 struct v3dv_query_pool *pool,
                                 uint32_t first, uint32_t count)
{
   assert(pool->query_type == VK_QUERY_TYPE_OCCLUSION);
   cmd_buffer_emit_reset_occlusion_query_pool(cmd_buffer, pool, first, count);
}

static void
cmd_buffer_emit_reset_query_pool_cpu(struct v3dv_cmd_buffer *cmd_buffer,
                                     struct v3dv_query_pool *pool,
                                     uint32_t first, uint32_t count)
{
   assert(pool->query_type != VK_QUERY_TYPE_OCCLUSION);

   struct v3dv_job *job =
      v3dv_cmd_buffer_create_cpu_job(cmd_buffer->device,
                                     V3DV_JOB_TYPE_CPU_RESET_QUERIES,
                                     cmd_buffer, -1);
   v3dv_return_if_oom(cmd_buffer, NULL);
   job->cpu.query_reset.pool = pool;
   job->cpu.query_reset.first = first;
   job->cpu.query_reset.count = count;
   list_addtail(&job->list_link, &cmd_buffer->jobs);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdResetQueryPool(VkCommandBuffer commandBuffer,
                       VkQueryPool queryPool,
                       uint32_t firstQuery,
                       uint32_t queryCount)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   /* Resets can only happen outside a render pass instance so we should not
    * be in the middle of job recording.
    */
   assert(cmd_buffer->state.pass == NULL);
   assert(cmd_buffer->state.job == NULL);

   assert(firstQuery < pool->query_count);
   assert(firstQuery + queryCount <= pool->query_count);

   /* We can reset occlusion queries in the GPU, but for other query types
    * we emit a CPU job that will call v3dv_reset_query_pool_cpu when executed
    * in the queue.
    */
   if (pool->query_type == VK_QUERY_TYPE_OCCLUSION) {
      cmd_buffer_emit_reset_query_pool(cmd_buffer, pool, firstQuery, queryCount);
   } else {
      cmd_buffer_emit_reset_query_pool_cpu(cmd_buffer, pool,
                                           firstQuery, queryCount);
   }
}

/**
 * Creates a descriptor pool so we can create a descriptors for the destination
 * buffers of vkCmdCopyQueryResults for queries where this is implemented in
 * the GPU.
 */
static VkResult
create_storage_buffer_descriptor_pool(struct v3dv_cmd_buffer *cmd_buffer)
{
   /* If this is not the first pool we create one for this command buffer
    * size it based on the size of the currently exhausted pool.
    */
   uint32_t descriptor_count = 32;
   if (cmd_buffer->meta.query.dspool != VK_NULL_HANDLE) {
      struct v3dv_descriptor_pool *exhausted_pool =
         v3dv_descriptor_pool_from_handle(cmd_buffer->meta.query.dspool);
      descriptor_count = MIN2(exhausted_pool->max_entry_count * 2, 1024);
   }

   /* Create the descriptor pool */
   cmd_buffer->meta.query.dspool = VK_NULL_HANDLE;
   VkDescriptorPoolSize pool_size = {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
      .descriptorCount = descriptor_count,
   };
   VkDescriptorPoolCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = descriptor_count,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
      .flags = 0,
   };
   VkResult result =
      v3dv_CreateDescriptorPool(v3dv_device_to_handle(cmd_buffer->device),
                                &info,
                                &cmd_buffer->device->vk.alloc,
                                &cmd_buffer->meta.query.dspool);

   if (result == VK_SUCCESS) {
      assert(cmd_buffer->meta.query.dspool != VK_NULL_HANDLE);
      const VkDescriptorPool vk_pool = cmd_buffer->meta.query.dspool;

      v3dv_cmd_buffer_add_private_obj(
         cmd_buffer, (uintptr_t) vk_pool,
         (v3dv_cmd_buffer_private_obj_destroy_cb)v3dv_DestroyDescriptorPool);

      struct v3dv_descriptor_pool *pool =
         v3dv_descriptor_pool_from_handle(vk_pool);
      pool->is_driver_internal = true;
   }

   return result;
}

static VkResult
allocate_storage_buffer_descriptor_set(struct v3dv_cmd_buffer *cmd_buffer,
                                       VkDescriptorSet *set)
{
   /* Make sure we have a descriptor pool */
   VkResult result;
   if (cmd_buffer->meta.query.dspool == VK_NULL_HANDLE) {
      result = create_storage_buffer_descriptor_pool(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;
   }
   assert(cmd_buffer->meta.query.dspool != VK_NULL_HANDLE);

   /* Allocate descriptor set */
   struct v3dv_device *device = cmd_buffer->device;
   VkDevice vk_device = v3dv_device_to_handle(device);
   VkDescriptorSetAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = cmd_buffer->meta.query.dspool,
      .descriptorSetCount = 1,
      .pSetLayouts = &device->queries.buf_descriptor_set_layout,
   };
   result = v3dv_AllocateDescriptorSets(vk_device, &info, set);

   /* If we ran out of pool space, grow the pool and try again */
   if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
      result = create_storage_buffer_descriptor_pool(cmd_buffer);
      if (result == VK_SUCCESS) {
         info.descriptorPool = cmd_buffer->meta.query.dspool;
         result = v3dv_AllocateDescriptorSets(vk_device, &info, set);
      }
   }

   return result;
}

static uint32_t
copy_pipeline_index_from_flags(VkQueryResultFlags flags)
{
   uint32_t index = 0;
   if (flags & VK_QUERY_RESULT_64_BIT)
      index |= 1;
   if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)
      index |= 2;
   if (flags & VK_QUERY_RESULT_PARTIAL_BIT)
      index |= 4;
   assert(index < 8);
   return index;
}

static nir_shader *
get_copy_query_results_cs(VkQueryResultFlags flags);

static void
cmd_buffer_emit_copy_query_pool_results(struct v3dv_cmd_buffer *cmd_buffer,
                                        struct v3dv_query_pool *pool,
                                        uint32_t first, uint32_t count,
                                        struct v3dv_buffer *buf,
                                        uint32_t offset, uint32_t stride,
                                        VkQueryResultFlags flags)
{
   struct v3dv_device *device = cmd_buffer->device;
   VkDevice vk_device = v3dv_device_to_handle(device);
   VkCommandBuffer vk_cmd_buffer = v3dv_cmd_buffer_to_handle(cmd_buffer);

   /* Create the required copy pipeline if not yet created */
   uint32_t pipeline_idx = copy_pipeline_index_from_flags(flags);
   if (!device->queries.copy_pipeline[pipeline_idx]) {
      nir_shader *copy_query_results_cs_nir = get_copy_query_results_cs(flags);
      VkResult result =
         v3dv_create_compute_pipeline_from_nir(
               device, copy_query_results_cs_nir,
               device->queries.copy_pipeline_layout,
               &device->queries.copy_pipeline[pipeline_idx]);
      ralloc_free(copy_query_results_cs_nir);
      if (result != VK_SUCCESS) {
         fprintf(stderr, "Failed to create copy query results pipeline\n");
         return;
      }
   }

   /* FIXME: do we need this barrier? Since vkCmdEndQuery should've been called
    * and that already waits maybe we don't (since this is serialized
    * in the compute queue with EndQuery anyway).
    */
   if (flags & VK_QUERY_RESULT_WAIT_BIT) {
      VkMemoryBarrier2 barrier = {
         .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
         .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
         .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      };
      VkDependencyInfo barrier_info = {
         .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
         .memoryBarrierCount = 1,
         .pMemoryBarriers = &barrier,
      };
      v3dv_cmd_buffer_emit_pipeline_barrier(cmd_buffer, &barrier_info);
   }

   /* Allocate and setup descriptor set for output buffer */
   VkDescriptorSet out_buf_descriptor_set;
   VkResult result =
      allocate_storage_buffer_descriptor_set(cmd_buffer,
                                             &out_buf_descriptor_set);
   if (result != VK_SUCCESS) {
      fprintf(stderr, "vkCmdCopyQueryPoolResults failed: "
              "could not allocate descriptor.\n");
      return;
   }

   VkDescriptorBufferInfo desc_buf_info = {
      .buffer = v3dv_buffer_to_handle(buf),
      .offset = 0,
      .range = VK_WHOLE_SIZE,
   };
   VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = out_buf_descriptor_set,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pBufferInfo = &desc_buf_info,
   };
   v3dv_UpdateDescriptorSets(vk_device, 1, &write, 0, NULL);

   /* Dispatch copy */
   v3dv_cmd_buffer_meta_state_push(cmd_buffer, true);

   assert(device->queries.copy_pipeline[pipeline_idx]);
   v3dv_CmdBindPipeline(vk_cmd_buffer,
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        device->queries.copy_pipeline[pipeline_idx]);

   VkDescriptorSet sets[2] = {
      pool->meta.descriptor_set,
      out_buf_descriptor_set,
   };
   v3dv_CmdBindDescriptorSets(vk_cmd_buffer,
                              VK_PIPELINE_BIND_POINT_COMPUTE,
                              device->queries.copy_pipeline_layout,
                              0, 2, sets, 0, NULL);

   struct {
      uint32_t avail_offset, first, offset, stride, flags;
   } push_data = { pool->occlusion.avail_offset, first, offset, stride, flags };
   v3dv_CmdPushConstants(vk_cmd_buffer,
                         device->queries.copy_pipeline_layout,
                         VK_SHADER_STAGE_COMPUTE_BIT,
                         0, sizeof(push_data), &push_data);

   cmd_buffer_emit_dispatch_queries(cmd_buffer, count);

   v3dv_cmd_buffer_meta_state_pop(cmd_buffer, false);
}

static void
cmd_buffer_emit_copy_query_pool_results_cpu(struct v3dv_cmd_buffer *cmd_buffer,
                                            struct v3dv_query_pool *pool,
                                            uint32_t first,
                                            uint32_t count,
                                            struct v3dv_buffer *dst,
                                            uint32_t offset,
                                            uint32_t stride,
                                            VkQueryResultFlags flags)
{
   struct v3dv_job *job =
      v3dv_cmd_buffer_create_cpu_job(cmd_buffer->device,
                                     V3DV_JOB_TYPE_CPU_COPY_QUERY_RESULTS,
                                     cmd_buffer, -1);
   v3dv_return_if_oom(cmd_buffer, NULL);

   job->cpu.query_copy_results.pool = pool;
   job->cpu.query_copy_results.first = first;
   job->cpu.query_copy_results.count = count;
   job->cpu.query_copy_results.dst = dst;
   job->cpu.query_copy_results.offset = offset;
   job->cpu.query_copy_results.stride = stride;
   job->cpu.query_copy_results.flags = flags;

   list_addtail(&job->list_link, &cmd_buffer->jobs);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer,
                             VkQueryPool queryPool,
                             uint32_t firstQuery,
                             uint32_t queryCount,
                             VkBuffer dstBuffer,
                             VkDeviceSize dstOffset,
                             VkDeviceSize stride,
                             VkQueryResultFlags flags)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);
   V3DV_FROM_HANDLE(v3dv_buffer, dst, dstBuffer);

   /* Copies can only happen outside a render pass instance so we should not
    * be in the middle of job recording.
    */
   assert(cmd_buffer->state.pass == NULL);
   assert(cmd_buffer->state.job == NULL);

   assert(firstQuery < pool->query_count);
   assert(firstQuery + queryCount <= pool->query_count);

   /* For occlusion queries we implement the copy in the GPU but for other
    * queries we emit a CPU job that will call v3dv_get_query_pool_results_cpu
    * when executed in the queue.
    */
   if (pool->query_type == VK_QUERY_TYPE_OCCLUSION) {
      cmd_buffer_emit_copy_query_pool_results(cmd_buffer, pool,
                                              firstQuery, queryCount,
                                              dst, (uint32_t) dstOffset,
                                              (uint32_t) stride, flags);
   } else {
      cmd_buffer_emit_copy_query_pool_results_cpu(cmd_buffer, pool,
                                                  firstQuery, queryCount,
                                                  dst, (uint32_t)dstOffset,
                                                  (uint32_t) stride, flags);
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdBeginQuery(VkCommandBuffer commandBuffer,
                   VkQueryPool queryPool,
                   uint32_t query,
                   VkQueryControlFlags flags)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   v3dv_cmd_buffer_begin_query(cmd_buffer, pool, query, flags);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdEndQuery(VkCommandBuffer commandBuffer,
                 VkQueryPool queryPool,
                 uint32_t query)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   v3dv_cmd_buffer_end_query(cmd_buffer, pool, query);
}

void
v3dv_reset_query_pool_cpu(struct v3dv_device *device,
                          struct v3dv_query_pool *pool,
                          uint32_t first,
                          uint32_t count)
{
   mtx_lock(&device->query_mutex);

   if (pool->query_type == VK_QUERY_TYPE_TIMESTAMP) {
      assert(first + count <= pool->query_count);

      /* Reset timestamp */
      uint8_t *base_addr;
      base_addr  = ((uint8_t *) pool->timestamp.bo->map) +
                    pool->queries[first].timestamp.offset;
      memset(base_addr, 0, 8 * count);

      for (uint32_t i = first; i < first + count; i++) {
         if (vk_sync_reset(&device->vk, pool->queries[i].timestamp.sync) != VK_SUCCESS)
            fprintf(stderr, "Failed to reset sync");
      }

      mtx_unlock(&device->query_mutex);
      return;
   }

   for (uint32_t i = first; i < first + count; i++) {
      assert(i < pool->query_count);
      struct v3dv_query *q = &pool->queries[i];
      q->maybe_available = false;
      switch (pool->query_type) {
      case VK_QUERY_TYPE_OCCLUSION: {
         /* Reset availability */
         uint8_t *base_addr = ((uint8_t *) pool->occlusion.bo->map) +
                              pool->occlusion.avail_offset + first;
         memset(base_addr, 0, count);

         /* Reset occlusion counter */
         const uint8_t *q_addr =
            ((uint8_t *) pool->occlusion.bo->map) + q->occlusion.offset;
         uint32_t *counter = (uint32_t *) q_addr;
         *counter = 0;
         break;
      }
      case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
         kperfmon_destroy(device, pool, i);
         kperfmon_create(device, pool, i);
         if (vk_sync_reset(&device->vk, q->perf.last_job_sync) != VK_SUCCESS)
            fprintf(stderr, "Failed to reset sync");
         break;
      default:
         unreachable("Unsupported query type");
      }
   }

   mtx_unlock(&device->query_mutex);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_ResetQueryPool(VkDevice _device,
                    VkQueryPool queryPool,
                    uint32_t firstQuery,
                    uint32_t queryCount)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   v3dv_reset_query_pool_cpu(device, pool, firstQuery, queryCount);
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
   VkPhysicalDevice physicalDevice,
   uint32_t queueFamilyIndex,
   uint32_t *pCounterCount,
   VkPerformanceCounterKHR *pCounters,
   VkPerformanceCounterDescriptionKHR *pCounterDescriptions)
{
   V3DV_FROM_HANDLE(v3dv_physical_device, pDevice, physicalDevice);

   return v3dv_X(pDevice, enumerate_performance_query_counters)(pCounterCount,
                                                                pCounters,
                                                                pCounterDescriptions);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
   VkPhysicalDevice physicalDevice,
   const VkQueryPoolPerformanceCreateInfoKHR *pPerformanceQueryCreateInfo,
   uint32_t *pNumPasses)
{
   *pNumPasses = DIV_ROUND_UP(pPerformanceQueryCreateInfo->counterIndexCount,
                              DRM_V3D_MAX_PERF_COUNTERS);
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_AcquireProfilingLockKHR(
   VkDevice _device,
   const VkAcquireProfilingLockInfoKHR *pInfo)
{
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_ReleaseProfilingLockKHR(VkDevice device)
{
}

static inline void
nir_set_query_availability(nir_builder *b,
                           nir_def *buf,
                           nir_def *offset,
                           nir_def *query_idx,
                           nir_def *avail)
{
   offset = nir_iadd(b, offset, query_idx); /* we use 1B per query */
   nir_store_ssbo(b, avail, buf, offset, .write_mask = 0x1, .align_mul = 1);
}

static inline nir_def *
nir_get_query_availability(nir_builder *b,
                           nir_def *buf,
                           nir_def *offset,
                           nir_def *query_idx)
{
   offset = nir_iadd(b, offset, query_idx); /* we use 1B per query */
   nir_def *avail = nir_load_ssbo(b, 1, 8, buf, offset, .align_mul = 1);
   return nir_i2i32(b, avail);
}

static nir_shader *
get_set_query_availability_cs()
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options,
                                                  "set query availability cs");

   nir_def *buf =
      nir_vulkan_resource_index(&b, 2, 32, nir_imm_int(&b, 0),
                                .desc_set = 0,
                                .binding = 0,
                                .desc_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

   /* This assumes a local size of 1 and a horizontal-only dispatch. If we
    * ever change any of these parameters we need to update how we compute the
    * query index here.
    */
   nir_def *wg_id = nir_channel(&b, nir_load_workgroup_id(&b), 0);

   nir_def *offset =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 0, .range = 4);

   nir_def *query_idx =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 4, .range = 4);

   nir_def *avail =
      nir_load_push_constant(&b, 1, 8, nir_imm_int(&b, 0), .base = 8, .range = 1);

   query_idx = nir_iadd(&b, query_idx, wg_id);
   nir_set_query_availability(&b, buf, offset, query_idx, avail);

   return b.shader;
}

static inline nir_def *
nir_get_occlusion_counter_offset(nir_builder *b, nir_def *query_idx)
{
   nir_def *query_group = nir_udiv_imm(b, query_idx, 16);
   nir_def *query_group_offset = nir_umod_imm(b, query_idx, 16);
   nir_def *offset =
      nir_iadd(b, nir_imul_imm(b, query_group, 1024),
                  nir_imul_imm(b, query_group_offset, 4));
   return offset;
}

static inline void
nir_reset_occlusion_counter(nir_builder *b,
                            nir_def *buf,
                            nir_def *query_idx)
{
   nir_def *offset = nir_get_occlusion_counter_offset(b, query_idx);
   nir_def *zero = nir_imm_int(b, 0);
   nir_store_ssbo(b, zero, buf, offset, .write_mask = 0x1, .align_mul = 4);
}

static inline nir_def *
nir_read_occlusion_counter(nir_builder *b,
                           nir_def *buf,
                           nir_def *query_idx)
{
   nir_def *offset = nir_get_occlusion_counter_offset(b, query_idx);
   return nir_load_ssbo(b, 1, 32, buf, offset, .access = 0, .align_mul = 4);
}

static nir_shader *
get_reset_occlusion_query_cs()
{
   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options,
                                                  "reset occlusion query cs");

   nir_def *buf =
      nir_vulkan_resource_index(&b, 2, 32, nir_imm_int(&b, 0),
                                .desc_set = 0,
                                .binding = 0,
                                .desc_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

   /* This assumes a local size of 1 and a horizontal-only dispatch. If we
    * ever change any of these parameters we need to update how we compute the
    * query index here.
    */
   nir_def *wg_id = nir_channel(&b, nir_load_workgroup_id(&b), 0);

   nir_def *avail_offset =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 0, .range = 4);

   nir_def *base_query_idx =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 4, .range = 4);

   nir_def *query_idx = nir_iadd(&b, base_query_idx, wg_id);

   nir_set_query_availability(&b, buf, avail_offset, query_idx,
                              nir_imm_intN_t(&b, 0, 8));
   nir_reset_occlusion_counter(&b, buf, query_idx);

   return b.shader;
}

static void
write_query_buffer(nir_builder *b,
                   nir_def *buf,
                   nir_def **offset,
                   nir_def *value,
                   bool flag_64bit)
{
   if (flag_64bit) {
      /* Create a 64-bit value using a vec2 with the .Y component set to 0
       * so we can write a 64-bit value in a single store.
       */
      nir_def *value64 = nir_vec2(b, value, nir_imm_int(b, 0));
      nir_store_ssbo(b, value64, buf, *offset, .write_mask = 0x3, .align_mul = 8);
      *offset = nir_iadd_imm(b, *offset, 8);
   } else {
      nir_store_ssbo(b, value, buf, *offset, .write_mask = 0x1, .align_mul = 4);
      *offset = nir_iadd_imm(b, *offset, 4);
   }
}

static nir_shader *
get_copy_query_results_cs(VkQueryResultFlags flags)
{
   bool flag_64bit = flags & VK_QUERY_RESULT_64_BIT;
   bool flag_avail = flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
   bool flag_partial = flags & VK_QUERY_RESULT_PARTIAL_BIT;

   const nir_shader_compiler_options *options = v3dv_pipeline_get_nir_options();
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options,
                                                  "copy query results cs");

   nir_def *buf =
      nir_vulkan_resource_index(&b, 2, 32, nir_imm_int(&b, 0),
                                .desc_set = 0,
                                .binding = 0,
                                .desc_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

   nir_def *buf_out =
      nir_vulkan_resource_index(&b, 2, 32, nir_imm_int(&b, 0),
                                .desc_set = 1,
                                .binding = 0,
                                .desc_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

   /* Read push constants */
   nir_def *avail_offset =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 0, .range = 4);

   nir_def *base_query_idx =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 4, .range = 4);

   nir_def *base_offset_out =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 8, .range = 4);

   nir_def *stride =
      nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .base = 12, .range = 4);

   /* This assumes a local size of 1 and a horizontal-only dispatch. If we
    * ever change any of these parameters we need to update how we compute the
    * query index here.
    */
   nir_def *wg_id = nir_channel(&b, nir_load_workgroup_id(&b), 0);
   nir_def *query_idx = nir_iadd(&b, base_query_idx, wg_id);

   /* Read query availability if needed */
   nir_def *avail = NULL;
   if (flag_avail || !flag_partial)
      avail = nir_get_query_availability(&b, buf, avail_offset, query_idx);

   /* Write occusion query result... */
   nir_def *offset =
      nir_iadd(&b, base_offset_out, nir_imul(&b, wg_id, stride));

   /* ...if partial is requested, we always write */
   if(flag_partial) {
      nir_def *query_res = nir_read_occlusion_counter(&b, buf, query_idx);
      write_query_buffer(&b, buf_out, &offset, query_res, flag_64bit);
   } else {
      /*...otherwise, we only write if the query is available */
      nir_if *if_stmt = nir_push_if(&b, nir_ine_imm(&b, avail, 0));
         nir_def *query_res = nir_read_occlusion_counter(&b, buf, query_idx);
         write_query_buffer(&b, buf_out, &offset, query_res, flag_64bit);
      nir_pop_if(&b, if_stmt);
   }

   /* Write query availability */
   if (flag_avail)
      write_query_buffer(&b, buf_out, &offset, avail, flag_64bit);

   return b.shader;
}

static bool
create_query_pipelines(struct v3dv_device *device)
{
   VkResult result;
   VkPipeline pipeline;

   /* Set layout: single storage buffer */
   if (!device->queries.buf_descriptor_set_layout) {
      VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {
         .binding = 0,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      };
      VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = {
         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .bindingCount = 1,
         .pBindings = &descriptor_set_layout_binding,
      };
      result =
         v3dv_CreateDescriptorSetLayout(v3dv_device_to_handle(device),
                                        &descriptor_set_layout_info,
                                        &device->vk.alloc,
                                        &device->queries.buf_descriptor_set_layout);
      if (result != VK_SUCCESS)
         return false;
   }

   /* Set availability pipeline.
    *
    * Pipeline layout:
    *  - 1 storage buffer for the BO with the query availability.
    *  - 2 push constants:
    *    0B: offset of the availability info in the buffer (4 bytes)
    *    4B: base query index (4 bytes).
    *    8B: availability (1 byte).
    */
   if (!device->queries.avail_pipeline_layout) {
      VkPipelineLayoutCreateInfo pipeline_layout_info = {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount = 1,
         .pSetLayouts = &device->queries.buf_descriptor_set_layout,
         .pushConstantRangeCount = 1,
         .pPushConstantRanges =
             &(VkPushConstantRange) { VK_SHADER_STAGE_COMPUTE_BIT, 0, 9 },
      };

      result =
         v3dv_CreatePipelineLayout(v3dv_device_to_handle(device),
                                   &pipeline_layout_info,
                                   &device->vk.alloc,
                                   &device->queries.avail_pipeline_layout);

      if (result != VK_SUCCESS)
         return false;
   }

   if (!device->queries.avail_pipeline) {
      nir_shader *set_query_availability_cs_nir = get_set_query_availability_cs();
      result = v3dv_create_compute_pipeline_from_nir(device,
                                                     set_query_availability_cs_nir,
                                                     device->queries.avail_pipeline_layout,
                                                     &pipeline);
      ralloc_free(set_query_availability_cs_nir);
      if (result != VK_SUCCESS)
         return false;

      device->queries.avail_pipeline = pipeline;
   }

   /* Reset occlusion query pipeline.
    *
    * Pipeline layout:
    *  - 1 storage buffer for the BO with the occlusion and availability data.
    *  - Push constants:
    *    0B: offset of the availability info in the buffer (4B)
    *    4B: base query index (4B)
    */
   if (!device->queries.reset_occlusion_pipeline_layout) {
      VkPipelineLayoutCreateInfo pipeline_layout_info = {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount = 1,
         .pSetLayouts = &device->queries.buf_descriptor_set_layout,
         .pushConstantRangeCount = 1,
         .pPushConstantRanges =
             &(VkPushConstantRange) { VK_SHADER_STAGE_COMPUTE_BIT, 0, 8 },
      };

      result =
         v3dv_CreatePipelineLayout(v3dv_device_to_handle(device),
                                   &pipeline_layout_info,
                                   &device->vk.alloc,
                                   &device->queries.reset_occlusion_pipeline_layout);

      if (result != VK_SUCCESS)
         return false;
   }

   if (!device->queries.reset_occlusion_pipeline) {
      nir_shader *reset_occlusion_query_cs_nir = get_reset_occlusion_query_cs();
      result = v3dv_create_compute_pipeline_from_nir(
                  device,
                  reset_occlusion_query_cs_nir,
                  device->queries.reset_occlusion_pipeline_layout,
                  &pipeline);
      ralloc_free(reset_occlusion_query_cs_nir);
      if (result != VK_SUCCESS)
         return false;

      device->queries.reset_occlusion_pipeline = pipeline;
   }

   /* Copy query results pipelines.
    *
    * Pipeline layout:
    *  - 1 storage buffer for the BO with the query availability and occlusion.
    *  - 1 storage buffer for the output.
    *  - Push constants:
    *    0B: offset of the availability info in the buffer (4B)
    *    4B: base query index (4B)
    *    8B: offset into output buffer (4B)
    *    12B: stride (4B)
    *
    * We create multiple specialized pipelines depending on the copy flags
    * to remove conditionals from the copy shader and get more optimized
    * pipelines.
    */
   if (!device->queries.copy_pipeline_layout) {
      VkDescriptorSetLayout set_layouts[2] = {
         device->queries.buf_descriptor_set_layout,
         device->queries.buf_descriptor_set_layout
      };
      VkPipelineLayoutCreateInfo pipeline_layout_info = {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount = 2,
         .pSetLayouts = set_layouts,
         .pushConstantRangeCount = 1,
         .pPushConstantRanges =
             &(VkPushConstantRange) { VK_SHADER_STAGE_COMPUTE_BIT, 0, 16 },
      };

      result =
         v3dv_CreatePipelineLayout(v3dv_device_to_handle(device),
                                   &pipeline_layout_info,
                                   &device->vk.alloc,
                                   &device->queries.copy_pipeline_layout);

      if (result != VK_SUCCESS)
         return false;
   }

   /* Actual copy pipelines are created lazily on demand since there can be up
    * to 8 depending on the flags used, however it is likely that applications
    * will use the same flags every time and only one pipeline is required.
    */

   return true;
}

static void
destroy_query_pipelines(struct v3dv_device *device)
{
   VkDevice _device = v3dv_device_to_handle(device);

   /* Availability pipeline */
   v3dv_DestroyPipeline(_device, device->queries.avail_pipeline,
                         &device->vk.alloc);
   device->queries.avail_pipeline = VK_NULL_HANDLE;
   v3dv_DestroyPipelineLayout(_device, device->queries.avail_pipeline_layout,
                              &device->vk.alloc);
   device->queries.avail_pipeline_layout = VK_NULL_HANDLE;

   /* Reset occlusion pipeline */
   v3dv_DestroyPipeline(_device, device->queries.reset_occlusion_pipeline,
                         &device->vk.alloc);
   device->queries.reset_occlusion_pipeline = VK_NULL_HANDLE;
   v3dv_DestroyPipelineLayout(_device,
                              device->queries.reset_occlusion_pipeline_layout,
                              &device->vk.alloc);
   device->queries.reset_occlusion_pipeline_layout = VK_NULL_HANDLE;

   /* Copy pipelines */
   for (int i = 0; i < 8; i++) {
      v3dv_DestroyPipeline(_device, device->queries.copy_pipeline[i],
                            &device->vk.alloc);
      device->queries.copy_pipeline[i] = VK_NULL_HANDLE;
   }
   v3dv_DestroyPipelineLayout(_device, device->queries.copy_pipeline_layout,
                              &device->vk.alloc);
   device->queries.copy_pipeline_layout = VK_NULL_HANDLE;

   v3dv_DestroyDescriptorSetLayout(_device,
                                   device->queries.buf_descriptor_set_layout,
                                   &device->vk.alloc);
   device->queries.buf_descriptor_set_layout = VK_NULL_HANDLE;
}

/**
 * Allocates device resources for implementing certain types of queries.
 */
VkResult
v3dv_query_allocate_resources(struct v3dv_device *device)
{
   if (!create_query_pipelines(device))
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   return VK_SUCCESS;
}

void
v3dv_query_free_resources(struct v3dv_device *device)
{
   destroy_query_pipelines(device);
}
