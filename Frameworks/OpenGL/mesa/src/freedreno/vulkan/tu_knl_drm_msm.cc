/*
 * Copyright © 2018 Google, Inc.
 * Copyright © 2015 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include "tu_knl.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <xf86drm.h>

#include "vk_util.h"

#include "drm-uapi/msm_drm.h"
#include "util/u_debug.h"
#include "util/hash_table.h"

#include "tu_cmd_buffer.h"
#include "tu_cs.h"
#include "tu_device.h"
#include "tu_dynamic_rendering.h"
#include "tu_knl_drm.h"
#include "redump.h"

struct tu_queue_submit
{
   struct vk_queue_submit *vk_submit;
   struct tu_u_trace_submission_data *u_trace_submission_data;

   struct tu_cmd_buffer **cmd_buffers;
   struct drm_msm_gem_submit_cmd *cmds;
   struct drm_msm_gem_submit_syncobj *in_syncobjs;
   struct drm_msm_gem_submit_syncobj *out_syncobjs;

   uint32_t nr_cmd_buffers;
   uint32_t nr_in_syncobjs;
   uint32_t nr_out_syncobjs;
   uint32_t entry_count;
   uint32_t perf_pass_index;

   bool     autotune_fence;
};

struct tu_u_trace_syncobj
{
   uint32_t msm_queue_id;
   uint32_t fence;
};

static int
tu_drm_get_param(int fd, uint32_t param, uint64_t *value)
{
   /* Technically this requires a pipe, but the kernel only supports one pipe
    * anyway at the time of writing and most of these are clearly pipe
    * independent. */
   struct drm_msm_param req = {
      .pipe = MSM_PIPE_3D0,
      .param = param,
   };

   int ret = drmCommandWriteRead(fd, DRM_MSM_GET_PARAM, &req, sizeof(req));
   if (ret)
      return ret;

   *value = req.value;

   return 0;
}

static int
tu_drm_get_gpu_id(const struct tu_physical_device *dev, uint32_t *id)
{
   uint64_t value;
   int ret = tu_drm_get_param(dev->local_fd, MSM_PARAM_GPU_ID, &value);
   if (ret)
      return ret;

   *id = value;
   return 0;
}

static int
tu_drm_get_gmem_size(const struct tu_physical_device *dev, uint32_t *size)
{
   uint64_t value;
   int ret = tu_drm_get_param(dev->local_fd, MSM_PARAM_GMEM_SIZE, &value);
   if (ret)
      return ret;

   *size = value;
   return 0;
}

static int
tu_drm_get_gmem_base(const struct tu_physical_device *dev, uint64_t *base)
{
   return tu_drm_get_param(dev->local_fd, MSM_PARAM_GMEM_BASE, base);
}

static int
tu_drm_get_va_prop(const struct tu_physical_device *dev,
                   uint64_t *va_start, uint64_t *va_size)
{
   uint64_t value;
   int ret = tu_drm_get_param(dev->local_fd, MSM_PARAM_VA_START, &value);
   if (ret)
      return ret;

   *va_start = value;

   ret = tu_drm_get_param(dev->local_fd, MSM_PARAM_VA_SIZE, &value);
   if (ret)
      return ret;

   *va_size = value;

   return 0;
}

static uint32_t
tu_drm_get_priorities(const struct tu_physical_device *dev)
{
   uint64_t val = 1;
   tu_drm_get_param(dev->local_fd, MSM_PARAM_PRIORITIES, &val);
   assert(val >= 1);

   return val;
}

static bool
tu_drm_is_memory_type_supported(int fd, uint32_t flags)
{
   struct drm_msm_gem_new req_alloc = { .size = 0x1000, .flags = flags };

   int ret =
      drmCommandWriteRead(fd, DRM_MSM_GEM_NEW, &req_alloc, sizeof(req_alloc));
   if (ret) {
      return false;
   }

   struct drm_gem_close req_close = {
      .handle = req_alloc.handle,
   };
   drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &req_close);

   return true;
}

static VkResult
msm_device_init(struct tu_device *dev)
{
   int fd = open(dev->physical_device->fd_path, O_RDWR | O_CLOEXEC);
   if (fd < 0) {
      return vk_startup_errorf(
            dev->physical_device->instance, VK_ERROR_INITIALIZATION_FAILED,
            "failed to open device %s", dev->physical_device->fd_path);
   }

   int ret = tu_drm_get_param(fd, MSM_PARAM_FAULTS, &dev->fault_count);
   if (ret != 0) {
      close(fd);
      return vk_startup_errorf(dev->physical_device->instance,
                               VK_ERROR_INITIALIZATION_FAILED,
                               "Failed to get initial fault count: %d", ret);
   }

   dev->fd = fd;

   return VK_SUCCESS;
}

static void
msm_device_finish(struct tu_device *dev)
{
   close(dev->fd);
}

static int
msm_device_get_gpu_timestamp(struct tu_device *dev, uint64_t *ts)
{
   return tu_drm_get_param(dev->fd, MSM_PARAM_TIMESTAMP, ts);
}

static int
msm_device_get_suspend_count(struct tu_device *dev, uint64_t *suspend_count)
{
   int ret = tu_drm_get_param(dev->fd, MSM_PARAM_SUSPENDS, suspend_count);
   return ret;
}

static VkResult
msm_device_check_status(struct tu_device *device)
{
   uint64_t last_fault_count = device->fault_count;
   int ret = tu_drm_get_param(device->fd, MSM_PARAM_FAULTS, &device->fault_count);
   if (ret != 0)
      return vk_device_set_lost(&device->vk, "error getting GPU fault count: %d", ret);

   if (last_fault_count != device->fault_count)
      return vk_device_set_lost(&device->vk, "GPU faulted or hung");

   return VK_SUCCESS;
}

static int
msm_submitqueue_new(struct tu_device *dev,
                    int priority,
                    uint32_t *queue_id)
{
   assert(priority >= 0 &&
          priority < dev->physical_device->submitqueue_priority_count);
   struct drm_msm_submitqueue req = {
      .flags = 0,
      .prio = priority,
   };

   int ret = drmCommandWriteRead(dev->fd,
                                 DRM_MSM_SUBMITQUEUE_NEW, &req, sizeof(req));
   if (ret)
      return ret;

   *queue_id = req.id;
   return 0;
}

static void
msm_submitqueue_close(struct tu_device *dev, uint32_t queue_id)
{
   drmCommandWrite(dev->fd, DRM_MSM_SUBMITQUEUE_CLOSE,
                   &queue_id, sizeof(uint32_t));
}

static void
tu_gem_close(const struct tu_device *dev, uint32_t gem_handle)
{
   struct drm_gem_close req = {
      .handle = gem_handle,
   };

   drmIoctl(dev->fd, DRM_IOCTL_GEM_CLOSE, &req);
}

/** Helper for DRM_MSM_GEM_INFO, returns 0 on error. */
static uint64_t
tu_gem_info(const struct tu_device *dev, uint32_t gem_handle, uint32_t info)
{
   struct drm_msm_gem_info req = {
      .handle = gem_handle,
      .info = info,
   };

   int ret = drmCommandWriteRead(dev->fd,
                                 DRM_MSM_GEM_INFO, &req, sizeof(req));
   if (ret < 0)
      return 0;

   return req.value;
}

static VkResult
tu_wait_fence(struct tu_device *dev,
              uint32_t queue_id,
              int fence,
              uint64_t timeout_ns)
{
   /* fence was created when no work was yet submitted */
   if (fence < 0)
      return VK_SUCCESS;

   struct drm_msm_wait_fence req = {
      .fence = fence,
      .queueid = queue_id,
   };
   int ret;

   get_abs_timeout(&req.timeout, timeout_ns);

   ret = drmCommandWrite(dev->fd, DRM_MSM_WAIT_FENCE, &req, sizeof(req));
   if (ret) {
      if (ret == -ETIMEDOUT) {
         return VK_TIMEOUT;
      } else {
         mesa_loge("tu_wait_fence failed! %d (%s)", ret, strerror(errno));
         return VK_ERROR_UNKNOWN;
      }
   }

   return VK_SUCCESS;
}

static VkResult
tu_free_zombie_vma_locked(struct tu_device *dev, bool wait)
{
   if (!u_vector_length(&dev->zombie_vmas))
      return VK_SUCCESS;

   if (wait) {
      struct tu_zombie_vma *vma = (struct tu_zombie_vma *)
            u_vector_head(&dev->zombie_vmas);
      /* Wait for 3s (arbitrary timeout) */
      VkResult ret = tu_wait_fence(dev, dev->queues[0]->msm_queue_id,
                                   vma->fence, 3000000000);

      if (ret != VK_SUCCESS)
         return ret;
   }

   int last_signaled_fence = -1;
   while (u_vector_length(&dev->zombie_vmas) > 0) {
      struct tu_zombie_vma *vma = (struct tu_zombie_vma *)
            u_vector_tail(&dev->zombie_vmas);
      if (vma->fence > last_signaled_fence) {
         VkResult ret =
            tu_wait_fence(dev, dev->queues[0]->msm_queue_id, vma->fence, 0);
         if (ret != VK_SUCCESS)
            return ret;

         last_signaled_fence = vma->fence;
      }

      /* Ensure that internal kernel's vma is freed. */
      struct drm_msm_gem_info req = {
         .handle = vma->gem_handle,
         .info = MSM_INFO_SET_IOVA,
         .value = 0,
      };

      int ret =
         drmCommandWriteRead(dev->fd, DRM_MSM_GEM_INFO, &req, sizeof(req));
      if (ret < 0) {
         mesa_loge("MSM_INFO_SET_IOVA(0) failed! %d (%s)", ret,
                   strerror(errno));
         return VK_ERROR_UNKNOWN;
      }

      tu_gem_close(dev, vma->gem_handle);

      util_vma_heap_free(&dev->vma, vma->iova, vma->size);
      u_vector_remove(&dev->zombie_vmas);
   }

   return VK_SUCCESS;
}

static VkResult
msm_allocate_userspace_iova(struct tu_device *dev,
                            uint32_t gem_handle,
                            uint64_t size,
                            uint64_t client_iova,
                            enum tu_bo_alloc_flags flags,
                            uint64_t *iova)
{
   VkResult result;

   mtx_lock(&dev->vma_mutex);

   *iova = 0;

   tu_free_zombie_vma_locked(dev, false);

   result = tu_allocate_userspace_iova(dev, size, client_iova, flags, iova);
   if (result == VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS) {
      /* Address may be already freed by us, but not considered as
       * freed by the kernel. We have to wait until all work that
       * may hold the address is done. Since addresses are meant to
       * be replayed only by debug tooling, it should be ok to wait.
       */
      tu_free_zombie_vma_locked(dev, true);
      result = tu_allocate_userspace_iova(dev, size, client_iova, flags, iova);
   }

   mtx_unlock(&dev->vma_mutex);

   if (result != VK_SUCCESS)
      return result;

   struct drm_msm_gem_info req = {
      .handle = gem_handle,
      .info = MSM_INFO_SET_IOVA,
      .value = *iova,
   };

   int ret =
      drmCommandWriteRead(dev->fd, DRM_MSM_GEM_INFO, &req, sizeof(req));
   if (ret < 0) {
      mesa_loge("MSM_INFO_SET_IOVA failed! %d (%s)", ret, strerror(errno));
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   return VK_SUCCESS;
}

static VkResult
tu_allocate_kernel_iova(struct tu_device *dev,
                        uint32_t gem_handle,
                        uint64_t *iova)
{
   *iova = tu_gem_info(dev, gem_handle, MSM_INFO_GET_IOVA);
   if (!*iova)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   return VK_SUCCESS;
}

static VkResult
tu_bo_init(struct tu_device *dev,
           struct tu_bo *bo,
           uint32_t gem_handle,
           uint64_t size,
           uint64_t client_iova,
           enum tu_bo_alloc_flags flags,
           const char *name)
{
   VkResult result = VK_SUCCESS;
   uint64_t iova = 0;

   assert(!client_iova || dev->physical_device->has_set_iova);

   if (dev->physical_device->has_set_iova) {
      result = msm_allocate_userspace_iova(dev, gem_handle, size, client_iova,
                                           flags, &iova);
   } else {
      result = tu_allocate_kernel_iova(dev, gem_handle, &iova);
   }

   if (result != VK_SUCCESS) {
      tu_gem_close(dev, gem_handle);
      return result;
   }

   name = tu_debug_bos_add(dev, size, name);

   mtx_lock(&dev->bo_mutex);
   uint32_t idx = dev->bo_count++;

   /* grow the bo list if needed */
   if (idx >= dev->bo_list_size) {
      uint32_t new_len = idx + 64;
      struct drm_msm_gem_submit_bo *new_ptr = (struct drm_msm_gem_submit_bo *)
         vk_realloc(&dev->vk.alloc, dev->bo_list, new_len * sizeof(*dev->bo_list),
                    8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
      if (!new_ptr) {
         dev->bo_count--;
         mtx_unlock(&dev->bo_mutex);
         tu_gem_close(dev, gem_handle);
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }

      dev->bo_list = new_ptr;
      dev->bo_list_size = new_len;
   }

   bool dump = flags & TU_BO_ALLOC_ALLOW_DUMP;
   dev->bo_list[idx] = (struct drm_msm_gem_submit_bo) {
      .flags = MSM_SUBMIT_BO_READ | MSM_SUBMIT_BO_WRITE |
               COND(dump, MSM_SUBMIT_BO_DUMP),
      .handle = gem_handle,
      .presumed = iova,
   };

   *bo = (struct tu_bo) {
      .gem_handle = gem_handle,
      .size = size,
      .iova = iova,
      .name = name,
      .refcnt = 1,
      .bo_list_idx = idx,
   };

   mtx_unlock(&dev->bo_mutex);

   return VK_SUCCESS;
}

/**
 * Sets the name in the kernel so that the contents of /debug/dri/0/gem are more
 * useful.
 *
 * We skip this on release builds (when we're also not doing BO debugging) to
 * reduce overhead.
 */
static void
tu_bo_set_kernel_name(struct tu_device *dev, struct tu_bo *bo, const char *name)
{
   bool kernel_bo_names = dev->bo_sizes != NULL;
#ifdef DEBUG
   kernel_bo_names = true;
#endif
   if (!kernel_bo_names)
      return;

   struct drm_msm_gem_info req = {
      .handle = bo->gem_handle,
      .info = MSM_INFO_SET_NAME,
      .value = (uintptr_t)(void *)name,
      .len = strlen(name),
   };

   int ret = drmCommandWrite(dev->fd, DRM_MSM_GEM_INFO, &req, sizeof(req));
   if (ret) {
      mesa_logw_once("Failed to set BO name with DRM_MSM_GEM_INFO: %d",
                     ret);
   }
}

static VkResult
msm_bo_init(struct tu_device *dev,
            struct tu_bo **out_bo,
            uint64_t size,
            uint64_t client_iova,
            VkMemoryPropertyFlags mem_property,
            enum tu_bo_alloc_flags flags,
            const char *name)
{
   struct drm_msm_gem_new req = {
      .size = size,
      .flags = 0
   };

   if (mem_property & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
      if (mem_property & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
         req.flags |= MSM_BO_CACHED_COHERENT;
      } else {
         req.flags |= MSM_BO_CACHED;
      }
   } else {
      req.flags |= MSM_BO_WC;
   }

   if (flags & TU_BO_ALLOC_GPU_READ_ONLY)
      req.flags |= MSM_BO_GPU_READONLY;

   int ret = drmCommandWriteRead(dev->fd,
                                 DRM_MSM_GEM_NEW, &req, sizeof(req));
   if (ret)
      return vk_error(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   struct tu_bo* bo = tu_device_lookup_bo(dev, req.handle);
   assert(bo && bo->gem_handle == 0);

   VkResult result =
      tu_bo_init(dev, bo, req.handle, size, client_iova, flags, name);

   if (result != VK_SUCCESS)
      memset(bo, 0, sizeof(*bo));
   else
      *out_bo = bo;

   /* We don't use bo->name here because for the !TU_DEBUG=bo case bo->name is NULL. */
   tu_bo_set_kernel_name(dev, bo, name);

   if (result == VK_SUCCESS &&
       (mem_property & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) &&
       !(mem_property & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      tu_bo_map(dev, bo);

      /* Cached non-coherent memory may already have dirty cache lines,
       * we should clean the cache lines before GPU got the chance to
       * write into this memory.
       *
       * MSM already does this automatically for uncached (MSM_BO_WC) memory.
       */
      tu_sync_cache_bo(dev, bo, 0, VK_WHOLE_SIZE, TU_MEM_SYNC_CACHE_TO_GPU);
   }

   return result;
}

static VkResult
msm_bo_init_dmabuf(struct tu_device *dev,
                   struct tu_bo **out_bo,
                   uint64_t size,
                   int prime_fd)
{
   /* lseek() to get the real size */
   off_t real_size = lseek(prime_fd, 0, SEEK_END);
   lseek(prime_fd, 0, SEEK_SET);
   if (real_size < 0 || (uint64_t) real_size < size)
      return vk_error(dev, VK_ERROR_INVALID_EXTERNAL_HANDLE);

   /* iova allocation needs to consider the object's *real* size: */
   size = real_size;

   /* Importing the same dmabuf several times would yield the same
    * gem_handle. Thus there could be a race when destroying
    * BO and importing the same dmabuf from different threads.
    * We must not permit the creation of dmabuf BO and its release
    * to happen in parallel.
    */
   u_rwlock_wrlock(&dev->dma_bo_lock);

   uint32_t gem_handle;
   int ret = drmPrimeFDToHandle(dev->fd, prime_fd,
                                &gem_handle);
   if (ret) {
      u_rwlock_wrunlock(&dev->dma_bo_lock);
      return vk_error(dev, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }

   struct tu_bo* bo = tu_device_lookup_bo(dev, gem_handle);

   if (bo->refcnt != 0) {
      p_atomic_inc(&bo->refcnt);
      u_rwlock_wrunlock(&dev->dma_bo_lock);

      *out_bo = bo;
      return VK_SUCCESS;
   }

   VkResult result =
      tu_bo_init(dev, bo, gem_handle, size, 0, TU_BO_ALLOC_NO_FLAGS, "dmabuf");

   if (result != VK_SUCCESS)
      memset(bo, 0, sizeof(*bo));
   else
      *out_bo = bo;

   u_rwlock_wrunlock(&dev->dma_bo_lock);

   return result;
}

static VkResult
msm_bo_map(struct tu_device *dev, struct tu_bo *bo)
{
   if (bo->map)
      return VK_SUCCESS;

   uint64_t offset = tu_gem_info(dev, bo->gem_handle, MSM_INFO_GET_OFFSET);
   if (!offset)
      return vk_error(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   /* TODO: Should we use the wrapper os_mmap() like Freedreno does? */
   void *map = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    dev->fd, offset);
   if (map == MAP_FAILED)
      return vk_error(dev, VK_ERROR_MEMORY_MAP_FAILED);

   bo->map = map;
   return VK_SUCCESS;
}

static void
msm_bo_allow_dump(struct tu_device *dev, struct tu_bo *bo)
{
   mtx_lock(&dev->bo_mutex);
   dev->bo_list[bo->bo_list_idx].flags |= MSM_SUBMIT_BO_DUMP;
   mtx_unlock(&dev->bo_mutex);
}


static void
msm_bo_set_metadata(struct tu_device *dev, struct tu_bo *bo,
                    void *metadata, uint32_t metadata_size)
{
   struct drm_msm_gem_info req = {
      .handle = bo->gem_handle,
      .info = MSM_INFO_SET_METADATA,
      .value = (uintptr_t)(void *)metadata,
      .len = metadata_size,
   };

   int ret = drmCommandWrite(dev->fd, DRM_MSM_GEM_INFO, &req, sizeof(req));
   if (ret) {
      mesa_logw_once("Failed to set BO metadata with DRM_MSM_GEM_INFO: %d",
                     ret);
   }
}

static int
msm_bo_get_metadata(struct tu_device *dev, struct tu_bo *bo,
                    void *metadata, uint32_t metadata_size)
{
   struct drm_msm_gem_info req = {
      .handle = bo->gem_handle,
      .info = MSM_INFO_GET_METADATA,
      .value = (uintptr_t)(void *)metadata,
      .len = metadata_size,
   };

   int ret = drmCommandWrite(dev->fd, DRM_MSM_GEM_INFO, &req, sizeof(req));
   if (ret) {
      mesa_logw_once("Failed to get BO metadata with DRM_MSM_GEM_INFO: %d",
                     ret);
   }

   return ret;
}

static VkResult
tu_queue_submit_create_locked(struct tu_queue *queue,
                              struct vk_queue_submit *vk_submit,
                              const uint32_t nr_in_syncobjs,
                              const uint32_t nr_out_syncobjs,
                              uint32_t perf_pass_index,
                              struct tu_queue_submit *new_submit)
{
   VkResult result;

   bool u_trace_enabled = u_trace_should_process(&queue->device->trace_context);
   bool has_trace_points = false;

   struct vk_command_buffer **vk_cmd_buffers = vk_submit->command_buffers;

   memset(new_submit, 0, sizeof(struct tu_queue_submit));

   new_submit->cmd_buffers = (struct tu_cmd_buffer **) vk_cmd_buffers;
   new_submit->nr_cmd_buffers = vk_submit->command_buffer_count;
   tu_insert_dynamic_cmdbufs(queue->device, &new_submit->cmd_buffers,
                             &new_submit->nr_cmd_buffers);

   uint32_t entry_count = 0;
   for (uint32_t j = 0; j < new_submit->nr_cmd_buffers; ++j) {
      struct tu_cmd_buffer *cmdbuf = new_submit->cmd_buffers[j];

      if (perf_pass_index != ~0)
         entry_count++;

      entry_count += cmdbuf->cs.entry_count;

      if (u_trace_enabled && u_trace_has_points(&cmdbuf->trace)) {
         if (!(cmdbuf->usage_flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT))
            entry_count++;

         has_trace_points = true;
      }
   }

   new_submit->autotune_fence =
      tu_autotune_submit_requires_fence(new_submit->cmd_buffers, new_submit->nr_cmd_buffers);
   if (new_submit->autotune_fence)
      entry_count++;

   new_submit->cmds = (struct drm_msm_gem_submit_cmd *) vk_zalloc(
      &queue->device->vk.alloc, entry_count * sizeof(*new_submit->cmds), 8,
      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

   if (new_submit->cmds == NULL) {
      result = vk_error(queue, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_cmds;
   }

   if (has_trace_points) {
      result =
         tu_u_trace_submission_data_create(
            queue->device, new_submit->cmd_buffers,
            new_submit->nr_cmd_buffers,
            &new_submit->u_trace_submission_data);

      if (result != VK_SUCCESS) {
         goto fail_u_trace_submission_data;
      }
   }

   /* Allocate without wait timeline semaphores */
   new_submit->in_syncobjs = (struct drm_msm_gem_submit_syncobj *) vk_zalloc(
      &queue->device->vk.alloc,
      nr_in_syncobjs * sizeof(*new_submit->in_syncobjs), 8,
      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

   if (new_submit->in_syncobjs == NULL) {
      result = vk_error(queue, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_in_syncobjs;
   }

   /* Allocate with signal timeline semaphores considered */
   new_submit->out_syncobjs = (struct drm_msm_gem_submit_syncobj *) vk_zalloc(
      &queue->device->vk.alloc,
      nr_out_syncobjs * sizeof(*new_submit->out_syncobjs), 8,
      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

   if (new_submit->out_syncobjs == NULL) {
      result = vk_error(queue, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_out_syncobjs;
   }

   new_submit->entry_count = entry_count;
   new_submit->nr_in_syncobjs = nr_in_syncobjs;
   new_submit->nr_out_syncobjs = nr_out_syncobjs;
   new_submit->perf_pass_index = perf_pass_index;
   new_submit->vk_submit = vk_submit;

   return VK_SUCCESS;

fail_out_syncobjs:
   vk_free(&queue->device->vk.alloc, new_submit->in_syncobjs);
fail_in_syncobjs:
   if (new_submit->u_trace_submission_data)
      tu_u_trace_submission_data_finish(queue->device,
                                        new_submit->u_trace_submission_data);
fail_u_trace_submission_data:
   vk_free(&queue->device->vk.alloc, new_submit->cmds);
fail_cmds:
   return result;
}

static void
tu_queue_submit_finish(struct tu_queue *queue, struct tu_queue_submit *submit)
{
   vk_free(&queue->device->vk.alloc, submit->cmds);
   vk_free(&queue->device->vk.alloc, submit->in_syncobjs);
   vk_free(&queue->device->vk.alloc, submit->out_syncobjs);
   if (submit->cmd_buffers != (void *) submit->vk_submit->command_buffers)
      vk_free(&queue->device->vk.alloc, submit->cmd_buffers);
}

static void
tu_fill_msm_gem_submit(struct tu_device *dev,
                       struct drm_msm_gem_submit_cmd *cmd,
                       struct tu_cs_entry *cs_entry)
{
   cmd->type = MSM_SUBMIT_CMD_BUF;
   cmd->submit_idx = cs_entry->bo->bo_list_idx;
   cmd->submit_offset = cs_entry->offset;
   cmd->size = cs_entry->size;
   cmd->pad = 0;
   cmd->nr_relocs = 0;
   cmd->relocs = 0;
}

static void
tu_queue_build_msm_gem_submit_cmds(struct tu_queue *queue,
                                   struct tu_queue_submit *submit,
                                   struct tu_cs *autotune_cs)
{
   struct tu_device *dev = queue->device;
   struct drm_msm_gem_submit_cmd *cmds = submit->cmds;

   uint32_t entry_idx = 0;
   for (uint32_t j = 0; j < submit->nr_cmd_buffers; ++j) {
      struct tu_device *dev = queue->device;
      struct tu_cmd_buffer *cmdbuf = submit->cmd_buffers[j];
      struct tu_cs *cs = &cmdbuf->cs;

      if (submit->perf_pass_index != ~0) {
         struct tu_cs_entry *perf_cs_entry =
            &dev->perfcntrs_pass_cs_entries[submit->perf_pass_index];

         tu_fill_msm_gem_submit(dev, &cmds[entry_idx], perf_cs_entry);
         entry_idx++;
      }

      for (unsigned i = 0; i < cs->entry_count; ++i, ++entry_idx) {
         tu_fill_msm_gem_submit(dev, &cmds[entry_idx], &cs->entries[i]);
      }

      if (submit->u_trace_submission_data) {
         struct tu_cs *ts_cs =
            submit->u_trace_submission_data->cmd_trace_data[j].timestamp_copy_cs;
         if (ts_cs) {
            tu_fill_msm_gem_submit(dev, &cmds[entry_idx], &ts_cs->entries[0]);
            entry_idx++;
         }
      }
   }

   if (autotune_cs) {
      assert(autotune_cs->entry_count == 1);
      tu_fill_msm_gem_submit(dev, &cmds[entry_idx], &autotune_cs->entries[0]);
      entry_idx++;
   }
}

static VkResult
tu_queue_submit_locked(struct tu_queue *queue, struct tu_queue_submit *submit)
{
   queue->device->submit_count++;

   struct tu_cs *autotune_cs = NULL;
   if (submit->autotune_fence) {
      autotune_cs = tu_autotune_on_submit(queue->device,
                                          &queue->device->autotune,
                                          submit->cmd_buffers,
                                          submit->nr_cmd_buffers);
   }

   uint32_t flags = MSM_PIPE_3D0;

   if (submit->vk_submit->wait_count)
      flags |= MSM_SUBMIT_SYNCOBJ_IN;

   if (submit->vk_submit->signal_count)
      flags |= MSM_SUBMIT_SYNCOBJ_OUT;

   mtx_lock(&queue->device->bo_mutex);

   if (queue->device->implicit_sync_bo_count == 0)
      flags |= MSM_SUBMIT_NO_IMPLICIT;

   /* drm_msm_gem_submit_cmd requires index of bo which could change at any
    * time when bo_mutex is not locked. So we build submit cmds here the real
    * place to submit.
    */
   tu_queue_build_msm_gem_submit_cmds(queue, submit, autotune_cs);

   struct drm_msm_gem_submit req = {
      .flags = flags,
      .nr_bos = submit->entry_count ? queue->device->bo_count : 0,
      .nr_cmds = submit->entry_count,
      .bos = (uint64_t)(uintptr_t) queue->device->bo_list,
      .cmds = (uint64_t)(uintptr_t)submit->cmds,
      .queueid = queue->msm_queue_id,
      .in_syncobjs = (uint64_t)(uintptr_t)submit->in_syncobjs,
      .out_syncobjs = (uint64_t)(uintptr_t)submit->out_syncobjs,
      .nr_in_syncobjs = submit->nr_in_syncobjs,
      .nr_out_syncobjs = submit->nr_out_syncobjs,
      .syncobj_stride = sizeof(struct drm_msm_gem_submit_syncobj),
   };

   if (TU_DEBUG(RD)) {
      struct tu_device *device = queue->device;
      static uint32_t submit_idx;
      char path[32];
      sprintf(path, "%.5d.rd", p_atomic_inc_return(&submit_idx));
      int rd = open(path, O_CLOEXEC | O_WRONLY | O_CREAT | O_TRUNC, 0777);
      if (rd >= 0) {
         rd_write_section(rd, RD_CHIP_ID, &device->physical_device->dev_id.chip_id, 4);

         rd_write_section(rd, RD_CMD, "tu-dump", 8);

         for (unsigned i = 0; i < device->bo_count; i++) {
            struct drm_msm_gem_submit_bo bo = device->bo_list[i];
            struct tu_bo *tu_bo = tu_device_lookup_bo(device, bo.handle);
            uint64_t iova = bo.presumed;

            uint32_t buf[3] = { iova, tu_bo->size, iova >> 32 };
            rd_write_section(rd, RD_GPUADDR, buf, 12);
            if (bo.flags & MSM_SUBMIT_BO_DUMP) {
               msm_bo_map(device, tu_bo); /* note: this would need locking to be safe */
               rd_write_section(rd, RD_BUFFER_CONTENTS, tu_bo->map, tu_bo->size);
            }
         }

         for (unsigned i = 0; i < req.nr_cmds; i++) {
            struct drm_msm_gem_submit_cmd *cmd = &submit->cmds[i];
            uint64_t iova = device->bo_list[cmd->submit_idx].presumed + cmd->submit_offset;
            uint32_t size = cmd->size >> 2;
            uint32_t buf[3] = { iova, size, iova >> 32 };
            rd_write_section(rd, RD_CMDSTREAM_ADDR, buf, 12);
         }
         close(rd);
      }
   }

   int ret = drmCommandWriteRead(queue->device->fd,
                                 DRM_MSM_GEM_SUBMIT,
                                 &req, sizeof(req));

   mtx_unlock(&queue->device->bo_mutex);

   tu_debug_bos_print_stats(queue->device);

   if (ret)
      return vk_device_set_lost(&queue->device->vk, "submit failed: %m");

   p_atomic_set(&queue->fence, req.fence);

   uint64_t gpu_offset = 0;
#if HAVE_PERFETTO
   struct tu_perfetto_clocks clocks =
      tu_perfetto_submit(queue->device, queue->device->submit_count, NULL);
   gpu_offset = clocks.gpu_ts_offset;
#endif

   if (submit->u_trace_submission_data) {
      struct tu_u_trace_submission_data *submission_data =
         submit->u_trace_submission_data;
      submission_data->submission_id = queue->device->submit_count;
      submission_data->gpu_ts_offset = gpu_offset;
      /* We have to allocate it here since it is different between drm/kgsl */
      submission_data->syncobj = (struct tu_u_trace_syncobj *)
         vk_alloc(&queue->device->vk.alloc, sizeof(struct tu_u_trace_syncobj),
               8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
      submission_data->syncobj->fence = req.fence;
      submission_data->syncobj->msm_queue_id = queue->msm_queue_id;

      submit->u_trace_submission_data = NULL;

      for (uint32_t i = 0; i < submission_data->cmd_buffer_count; i++) {
         bool free_data = i == submission_data->last_buffer_with_tracepoints;
         if (submission_data->cmd_trace_data[i].trace)
            u_trace_flush(submission_data->cmd_trace_data[i].trace,
                          submission_data, free_data);

         if (!submission_data->cmd_trace_data[i].timestamp_copy_cs) {
            /* u_trace is owned by cmd_buffer */
            submission_data->cmd_trace_data[i].trace = NULL;
         }
      }
   }

   for (uint32_t i = 0; i < submit->vk_submit->wait_count; i++) {
      if (!vk_sync_is_tu_timeline_sync(submit->vk_submit->waits[i].sync))
         continue;

      struct tu_timeline_sync *sync =
         container_of(submit->vk_submit->waits[i].sync, struct tu_timeline_sync, base);

      assert(sync->state != TU_TIMELINE_SYNC_STATE_RESET);

      /* Set SIGNALED to the state of the wait timeline sync since this means the syncobj
       * is done and ready again so this can be garbage-collectioned later.
       */
      sync->state = TU_TIMELINE_SYNC_STATE_SIGNALED;
   }

   for (uint32_t i = 0; i < submit->vk_submit->signal_count; i++) {
      if (!vk_sync_is_tu_timeline_sync(submit->vk_submit->signals[i].sync))
         continue;

      struct tu_timeline_sync *sync =
         container_of(submit->vk_submit->signals[i].sync, struct tu_timeline_sync, base);

      assert(sync->state == TU_TIMELINE_SYNC_STATE_RESET);
      /* Set SUBMITTED to the state of the signal timeline sync so we could wait for
       * this timeline sync until completed if necessary.
       */
      sync->state = TU_TIMELINE_SYNC_STATE_SUBMITTED;
   }

   pthread_cond_broadcast(&queue->device->timeline_cond);

   return VK_SUCCESS;
}

static VkResult
msm_device_wait_u_trace(struct tu_device *dev, struct tu_u_trace_syncobj *syncobj)
{
   return tu_wait_fence(dev, syncobj->msm_queue_id, syncobj->fence, 1000000000);
}

static VkResult
msm_queue_submit(struct tu_queue *queue, struct vk_queue_submit *submit)
{
   MESA_TRACE_FUNC();
   uint32_t perf_pass_index = queue->device->perfcntrs_pass_cs ?
                              submit->perf_pass_index : ~0;
   struct tu_queue_submit submit_req;

   if (TU_DEBUG(LOG_SKIP_GMEM_OPS)) {
      tu_dbg_log_gmem_load_store_skips(queue->device);
   }

   pthread_mutex_lock(&queue->device->submit_mutex);

   VkResult ret = tu_queue_submit_create_locked(queue, submit,
         submit->wait_count, submit->signal_count,
         perf_pass_index, &submit_req);

   if (ret != VK_SUCCESS) {
      pthread_mutex_unlock(&queue->device->submit_mutex);
      return ret;
   }

   /* note: assuming there won't be any very large semaphore counts */
   struct drm_msm_gem_submit_syncobj *in_syncobjs = submit_req.in_syncobjs;
   struct drm_msm_gem_submit_syncobj *out_syncobjs = submit_req.out_syncobjs;

   uint32_t nr_in_syncobjs = 0, nr_out_syncobjs = 0;

   for (uint32_t i = 0; i < submit->wait_count; i++) {
      struct vk_sync *sync = submit->waits[i].sync;

      in_syncobjs[nr_in_syncobjs++] = (struct drm_msm_gem_submit_syncobj) {
         .handle = tu_syncobj_from_vk_sync(sync),
         .flags = 0,
         .point = submit->waits[i].wait_value,
      };
   }

   for (uint32_t i = 0; i < submit->signal_count; i++) {
      struct vk_sync *sync = submit->signals[i].sync;

      out_syncobjs[nr_out_syncobjs++] = (struct drm_msm_gem_submit_syncobj) {
         .handle = tu_syncobj_from_vk_sync(sync),
         .flags = 0,
         .point = submit->signals[i].signal_value,
      };
   }

   ret = tu_queue_submit_locked(queue, &submit_req);

   pthread_mutex_unlock(&queue->device->submit_mutex);
   tu_queue_submit_finish(queue, &submit_req);

   if (ret != VK_SUCCESS)
       return ret;

   u_trace_context_process(&queue->device->trace_context, true);

   return VK_SUCCESS;
}

static const struct tu_knl msm_knl_funcs = {
      .name = "msm",

      .device_init = msm_device_init,
      .device_finish = msm_device_finish,
      .device_get_gpu_timestamp = msm_device_get_gpu_timestamp,
      .device_get_suspend_count = msm_device_get_suspend_count,
      .device_check_status = msm_device_check_status,
      .submitqueue_new = msm_submitqueue_new,
      .submitqueue_close = msm_submitqueue_close,
      .bo_init = msm_bo_init,
      .bo_init_dmabuf = msm_bo_init_dmabuf,
      .bo_export_dmabuf = tu_drm_export_dmabuf,
      .bo_map = msm_bo_map,
      .bo_allow_dump = msm_bo_allow_dump,
      .bo_finish = tu_drm_bo_finish,
      .bo_set_metadata = msm_bo_set_metadata,
      .bo_get_metadata = msm_bo_get_metadata,
      .device_wait_u_trace = msm_device_wait_u_trace,
      .queue_submit = msm_queue_submit,
};

VkResult
tu_knl_drm_msm_load(struct tu_instance *instance,
                    int fd, struct _drmVersion *version,
                    struct tu_physical_device **out)
{
   VkResult result = VK_SUCCESS;

   /* Version 1.6 added SYNCOBJ support. */
   const int min_version_major = 1;
   const int min_version_minor = 6;

   if (version->version_major != min_version_major ||
       version->version_minor < min_version_minor) {
      result = vk_startup_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                                 "kernel driver for device %s has version %d.%d, "
                                 "but Vulkan requires version >= %d.%d",
                                 version->name,
                                 version->version_major, version->version_minor,
                                 min_version_major, min_version_minor);
      return result;
   }

   struct tu_physical_device *device = (struct tu_physical_device *)
      vk_zalloc(&instance->vk.alloc, sizeof(*device), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!device) {
      result = vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail;
   }

   device->msm_major_version = version->version_major;
   device->msm_minor_version = version->version_minor;

   device->instance = instance;
   device->local_fd = fd;

   if (tu_drm_get_gpu_id(device, &device->dev_id.gpu_id)) {
      result = vk_startup_errorf(instance, VK_ERROR_INITIALIZATION_FAILED,
                                 "could not get GPU ID");
      goto fail;
   }

   if (tu_drm_get_param(fd, MSM_PARAM_CHIP_ID, &device->dev_id.chip_id)) {
      result = vk_startup_errorf(instance, VK_ERROR_INITIALIZATION_FAILED,
                                 "could not get CHIP ID");
      goto fail;
   }

   if (tu_drm_get_gmem_size(device, &device->gmem_size)) {
      result = vk_startup_errorf(instance, VK_ERROR_INITIALIZATION_FAILED,
                                "could not get GMEM size");
      goto fail;
   }
   device->gmem_size = debug_get_num_option("TU_GMEM", device->gmem_size);

   if (tu_drm_get_gmem_base(device, &device->gmem_base)) {
      result = vk_startup_errorf(instance, VK_ERROR_INITIALIZATION_FAILED,
                                 "could not get GMEM size");
      goto fail;
   }

   device->has_set_iova = !tu_drm_get_va_prop(device, &device->va_start,
                                              &device->va_size);

   /* Even if kernel is new enough, the GPU itself may not support it. */
   device->has_cached_coherent_memory =
      (device->msm_minor_version >= 8) &&
      tu_drm_is_memory_type_supported(fd, MSM_BO_CACHED_COHERENT);

   device->submitqueue_priority_count = tu_drm_get_priorities(device);

   device->syncobj_type = vk_drm_syncobj_get_type(fd);
   /* we don't support DRM_CAP_SYNCOBJ_TIMELINE, but drm-shim does */
   if (!(device->syncobj_type.features & VK_SYNC_FEATURE_TIMELINE))
      device->timeline_type = vk_sync_timeline_get_type(&tu_timeline_sync_type);

   device->sync_types[0] = &device->syncobj_type;
   device->sync_types[1] = &device->timeline_type.sync;
   device->sync_types[2] = NULL;

   device->heap.size = tu_get_system_heap_size(device);
   device->heap.used = 0u;
   device->heap.flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;

   instance->knl = &msm_knl_funcs;

   *out = device;

   return VK_SUCCESS;

fail:
   vk_free(&instance->vk.alloc, device);
   return result;
}
