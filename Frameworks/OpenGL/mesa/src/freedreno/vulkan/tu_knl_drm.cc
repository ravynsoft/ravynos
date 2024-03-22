/*
 * Copyright © 2018 Google, Inc.
 * Copyright © 2015 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include <fcntl.h>
#include <sys/mman.h>
#include <xf86drm.h>

#include "tu_knl_drm.h"
#include "tu_device.h"

static inline void
tu_sync_cacheline_to_gpu(void const *p __attribute__((unused)))
{
#if DETECT_ARCH_AARCH64
   /* Clean data cache. */
   __asm volatile("dc cvac, %0" : : "r" (p) : "memory");
#elif (DETECT_ARCH_X86 || DETECT_ARCH_X86_64)
   __builtin_ia32_clflush(p);
#elif DETECT_ARCH_ARM
   /* DCCMVAC - same as DC CVAC on aarch64.
    * Seems to be illegal to call from userspace.
    */
   //__asm volatile("mcr p15, 0, %0, c7, c10, 1" : : "r" (p) : "memory");
   unreachable("Cache line clean is unsupported on ARMv7");
#endif
}

static inline void
tu_sync_cacheline_from_gpu(void const *p __attribute__((unused)))
{
#if DETECT_ARCH_AARCH64
   /* Clean and Invalidate data cache, there is no separate Invalidate. */
   __asm volatile("dc civac, %0" : : "r" (p) : "memory");
#elif (DETECT_ARCH_X86 || DETECT_ARCH_X86_64)
   __builtin_ia32_clflush(p);
#elif DETECT_ARCH_ARM
   /* DCCIMVAC - same as DC CIVAC on aarch64.
    * Seems to be illegal to call from userspace.
    */
   //__asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r" (p) : "memory");
   unreachable("Cache line invalidate is unsupported on ARMv7");
#endif
}

void
tu_sync_cache_bo(struct tu_device *dev,
                 struct tu_bo *bo,
                 VkDeviceSize offset,
                 VkDeviceSize size,
                 enum tu_mem_sync_op op)
{
   uintptr_t level1_dcache_size = dev->physical_device->level1_dcache_size;
   char *start = (char *) bo->map + offset;
   char *end = start + (size == VK_WHOLE_SIZE ? (bo->size - offset) : size);

   start = (char *) ((uintptr_t) start & ~(level1_dcache_size - 1));

   for (; start < end; start += level1_dcache_size) {
      if (op == TU_MEM_SYNC_CACHE_TO_GPU) {
         tu_sync_cacheline_to_gpu(start);
      } else {
         tu_sync_cacheline_from_gpu(start);
      }
   }
}

static VkResult
sync_cache(VkDevice _device,
           enum tu_mem_sync_op op,
           uint32_t count,
           const VkMappedMemoryRange *ranges)
{
   TU_FROM_HANDLE(tu_device, device, _device);

   if (!device->physical_device->has_cached_non_coherent_memory) {
      tu_finishme(
         "data cache clean and invalidation are unsupported on this arch!");
      return VK_SUCCESS;
   }

   for (uint32_t i = 0; i < count; i++) {
      TU_FROM_HANDLE(tu_device_memory, mem, ranges[i].memory);
      tu_sync_cache_bo(device, mem->bo, ranges[i].offset, ranges[i].size, op);
   }

   return VK_SUCCESS;
}

VkResult
tu_FlushMappedMemoryRanges(VkDevice _device,
                           uint32_t memoryRangeCount,
                           const VkMappedMemoryRange *pMemoryRanges)
{
   return sync_cache(_device, TU_MEM_SYNC_CACHE_TO_GPU, memoryRangeCount,
                     pMemoryRanges);
}

VkResult
tu_InvalidateMappedMemoryRanges(VkDevice _device,
                                uint32_t memoryRangeCount,
                                const VkMappedMemoryRange *pMemoryRanges)
{
   return sync_cache(_device, TU_MEM_SYNC_CACHE_FROM_GPU, memoryRangeCount,
                     pMemoryRanges);
}

VkResult
tu_allocate_userspace_iova(struct tu_device *dev,
                           uint64_t size,
                           uint64_t client_iova,
                           enum tu_bo_alloc_flags flags,
                           uint64_t *iova)
{
   *iova = 0;

   if (flags & TU_BO_ALLOC_REPLAYABLE) {
      if (client_iova) {
         if (util_vma_heap_alloc_addr(&dev->vma, client_iova, size)) {
            *iova = client_iova;
         } else {
            return VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS;
         }
      } else {
         /* We have to separate replayable IOVAs from ordinary one in order to
          * for them not to clash. The easiest way to do this is to allocate
          * them from the other end of the address space.
          */
         dev->vma.alloc_high = true;
         *iova = util_vma_heap_alloc(&dev->vma, size, 0x1000);
      }
   } else {
      dev->vma.alloc_high = false;
      *iova = util_vma_heap_alloc(&dev->vma, size, 0x1000);
   }

   if (!*iova)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   return VK_SUCCESS;
}

int
tu_drm_export_dmabuf(struct tu_device *dev, struct tu_bo *bo)
{
   int prime_fd;
   int ret = drmPrimeHandleToFD(dev->fd, bo->gem_handle,
                                DRM_CLOEXEC | DRM_RDWR, &prime_fd);

   return ret == 0 ? prime_fd : -1;
}

void
tu_drm_bo_finish(struct tu_device *dev, struct tu_bo *bo)
{
   assert(bo->gem_handle);

   u_rwlock_rdlock(&dev->dma_bo_lock);

   if (!p_atomic_dec_zero(&bo->refcnt)) {
      u_rwlock_rdunlock(&dev->dma_bo_lock);
      return;
   }

   if (bo->map)
      munmap(bo->map, bo->size);

   tu_debug_bos_del(dev, bo);

   mtx_lock(&dev->bo_mutex);
   dev->bo_count--;
   dev->bo_list[bo->bo_list_idx] = dev->bo_list[dev->bo_count];

   struct tu_bo* exchanging_bo = tu_device_lookup_bo(dev, dev->bo_list[bo->bo_list_idx].handle);
   exchanging_bo->bo_list_idx = bo->bo_list_idx;

   if (bo->implicit_sync)
      dev->implicit_sync_bo_count--;

   mtx_unlock(&dev->bo_mutex);

   if (dev->physical_device->has_set_iova) {
      mtx_lock(&dev->vma_mutex);
      struct tu_zombie_vma *vma = (struct tu_zombie_vma *)
            u_vector_add(&dev->zombie_vmas);
      vma->gem_handle = bo->gem_handle;
#ifdef TU_HAS_VIRTIO
      vma->res_id = bo->res_id;
#endif
      vma->iova = bo->iova;
      vma->size = bo->size;
      vma->fence = p_atomic_read(&dev->queues[0]->fence);

      /* Must be cleared under the VMA mutex, or another thread could race to
       * reap the VMA, closing the BO and letting a new GEM allocation produce
       * this handle again.
       */
      memset(bo, 0, sizeof(*bo));
      mtx_unlock(&dev->vma_mutex);
   } else {
      /* Our BO structs are stored in a sparse array in the physical device,
       * so we don't want to free the BO pointer, instead we want to reset it
       * to 0, to signal that array entry as being free.
       */
      uint32_t gem_handle = bo->gem_handle;
      memset(bo, 0, sizeof(*bo));

      /* Note that virtgpu GEM_CLOSE path is a bit different, but it does
       * not use the !has_set_iova path so we can ignore that
       */
      struct drm_gem_close req = {
         .handle = gem_handle,
      };

      drmIoctl(dev->fd, DRM_IOCTL_GEM_CLOSE, &req);
   }

   u_rwlock_rdunlock(&dev->dma_bo_lock);
}

uint32_t
tu_syncobj_from_vk_sync(struct vk_sync *sync)
{
   uint32_t syncobj = -1;
   if (vk_sync_is_tu_timeline_sync(sync)) {
      syncobj = to_tu_timeline_sync(sync)->syncobj;
   } else if (vk_sync_type_is_drm_syncobj(sync->type)) {
      syncobj = vk_sync_as_drm_syncobj(sync)->syncobj;
   }

   assert(syncobj != -1);

   return syncobj;
}

static VkResult
tu_timeline_sync_init(struct vk_device *vk_device,
                      struct vk_sync *vk_sync,
                      uint64_t initial_value)
{
   struct tu_device *device = container_of(vk_device, struct tu_device, vk);
   struct tu_timeline_sync *sync = to_tu_timeline_sync(vk_sync);
   uint32_t flags = 0;

   assert(device->fd >= 0);

   int err = drmSyncobjCreate(device->fd, flags, &sync->syncobj);

   if (err < 0) {
        return vk_error(device, VK_ERROR_DEVICE_LOST);
   }

   sync->state = initial_value ? TU_TIMELINE_SYNC_STATE_SIGNALED :
                                    TU_TIMELINE_SYNC_STATE_RESET;

   return VK_SUCCESS;
}

static void
tu_timeline_sync_finish(struct vk_device *vk_device,
                   struct vk_sync *vk_sync)
{
   struct tu_device *dev = container_of(vk_device, struct tu_device, vk);
   struct tu_timeline_sync *sync = to_tu_timeline_sync(vk_sync);

   assert(dev->fd >= 0);
   ASSERTED int err = drmSyncobjDestroy(dev->fd, sync->syncobj);
   assert(err == 0);
}

static VkResult
tu_timeline_sync_reset(struct vk_device *vk_device,
                  struct vk_sync *vk_sync)
{
   struct tu_device *dev = container_of(vk_device, struct tu_device, vk);
   struct tu_timeline_sync *sync = to_tu_timeline_sync(vk_sync);

   int err = drmSyncobjReset(dev->fd, &sync->syncobj, 1);
   if (err) {
      return vk_errorf(dev, VK_ERROR_UNKNOWN,
                       "DRM_IOCTL_SYNCOBJ_RESET failed: %m");
   } else {
       sync->state = TU_TIMELINE_SYNC_STATE_RESET;
   }

   return VK_SUCCESS;
}

static VkResult
drm_syncobj_wait(struct tu_device *device,
                 uint32_t *handles, uint32_t count_handles,
                 uint64_t timeout_nsec, bool wait_all)
{
   uint32_t syncobj_wait_flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT;
   if (wait_all) syncobj_wait_flags |= DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;

   /* syncobj absolute timeouts are signed.  clamp OS_TIMEOUT_INFINITE down. */
   timeout_nsec = MIN2(timeout_nsec, (uint64_t)INT64_MAX);

   int err = drmSyncobjWait(device->fd, handles,
                            count_handles, timeout_nsec,
                            syncobj_wait_flags,
                            NULL /* first_signaled */);
   if (err && errno == ETIME) {
      return VK_TIMEOUT;
   } else if (err) {
      return vk_errorf(device, VK_ERROR_UNKNOWN,
                       "DRM_IOCTL_SYNCOBJ_WAIT failed: %m");
   }

   return VK_SUCCESS;
}

/* Based on anv_bo_sync_wait */
static VkResult
tu_timeline_sync_wait(struct vk_device *vk_device,
                 uint32_t wait_count,
                 const struct vk_sync_wait *waits,
                 enum vk_sync_wait_flags wait_flags,
                 uint64_t abs_timeout_ns)
{
   struct tu_device *dev = container_of(vk_device, struct tu_device, vk);
   bool wait_all = !(wait_flags & VK_SYNC_WAIT_ANY);

   uint32_t handles[wait_count];
   uint32_t submit_count;
   VkResult ret = VK_SUCCESS;
   uint32_t pending = wait_count;
   struct tu_timeline_sync *submitted_syncs[wait_count];

   while (pending) {
      pending = 0;
      submit_count = 0;

      for (unsigned i = 0; i < wait_count; ++i) {
         struct tu_timeline_sync *sync = to_tu_timeline_sync(waits[i].sync);

         if (sync->state == TU_TIMELINE_SYNC_STATE_RESET) {
            assert(!(wait_flags & VK_SYNC_WAIT_PENDING));
            pending++;
         } else if (sync->state == TU_TIMELINE_SYNC_STATE_SIGNALED) {
            if (wait_flags & VK_SYNC_WAIT_ANY)
               return VK_SUCCESS;
         } else if (sync->state == TU_TIMELINE_SYNC_STATE_SUBMITTED) {
            if (!(wait_flags & VK_SYNC_WAIT_PENDING)) {
               handles[submit_count] = sync->syncobj;
               submitted_syncs[submit_count++] = sync;
            }
         }
      }

      if (submit_count > 0) {
         do {
            ret = drm_syncobj_wait(dev, handles, submit_count, abs_timeout_ns, wait_all);
         } while (ret == VK_TIMEOUT && os_time_get_nano() < abs_timeout_ns);

         if (ret == VK_SUCCESS) {
            for (unsigned i = 0; i < submit_count; ++i) {
               struct tu_timeline_sync *sync = submitted_syncs[i];
               sync->state = TU_TIMELINE_SYNC_STATE_SIGNALED;
            }
         } else {
            /* return error covering timeout */
            return ret;
         }
      } else if (pending > 0) {
         /* If we've hit this then someone decided to vkWaitForFences before
          * they've actually submitted any of them to a queue.  This is a
          * fairly pessimal case, so it's ok to lock here and use a standard
          * pthreads condition variable.
          */
         pthread_mutex_lock(&dev->submit_mutex);

         /* It's possible that some of the fences have changed state since the
          * last time we checked.  Now that we have the lock, check for
          * pending fences again and don't wait if it's changed.
          */
         uint32_t now_pending = 0;
         for (uint32_t i = 0; i < wait_count; i++) {
            struct tu_timeline_sync *sync = to_tu_timeline_sync(waits[i].sync);
            if (sync->state == TU_TIMELINE_SYNC_STATE_RESET)
               now_pending++;
         }
         assert(now_pending <= pending);

         if (now_pending == pending) {
            struct timespec abstime = {
               .tv_sec = abs_timeout_ns / NSEC_PER_SEC,
               .tv_nsec = abs_timeout_ns % NSEC_PER_SEC,
            };

            ASSERTED int ret;
            ret = pthread_cond_timedwait(&dev->timeline_cond,
                                         &dev->submit_mutex, &abstime);
            assert(ret != EINVAL);
            if (os_time_get_nano() >= abs_timeout_ns) {
               pthread_mutex_unlock(&dev->submit_mutex);
               return VK_TIMEOUT;
            }
         }

         pthread_mutex_unlock(&dev->submit_mutex);
      }
   }

   return ret;
}

const struct vk_sync_type tu_timeline_sync_type = {
   .size = sizeof(struct tu_timeline_sync),
   .features = (enum vk_sync_features)(
      VK_SYNC_FEATURE_BINARY | VK_SYNC_FEATURE_GPU_WAIT |
      VK_SYNC_FEATURE_GPU_MULTI_WAIT | VK_SYNC_FEATURE_CPU_WAIT |
      VK_SYNC_FEATURE_CPU_RESET | VK_SYNC_FEATURE_WAIT_ANY |
      VK_SYNC_FEATURE_WAIT_PENDING),
   .init = tu_timeline_sync_init,
   .finish = tu_timeline_sync_finish,
   .reset = tu_timeline_sync_reset,
   .wait_many = tu_timeline_sync_wait,
};
