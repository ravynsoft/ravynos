/*
 * Copyright 2020 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <xf86drm.h>

#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif

#include "drm-uapi/virtgpu_drm.h"
#include "util/sparse_array.h"
#define VIRGL_RENDERER_UNSTABLE_APIS
#include "virtio-gpu/virglrenderer_hw.h"

#include "vn_renderer_internal.h"

#ifndef VIRTGPU_PARAM_GUEST_VRAM
/* All guest allocations happen via virtgpu dedicated heap. */
#define VIRTGPU_PARAM_GUEST_VRAM 9
#endif

#ifndef VIRTGPU_BLOB_MEM_GUEST_VRAM
#define VIRTGPU_BLOB_MEM_GUEST_VRAM 0x0004
#endif

/* XXX comment these out to really use kernel uapi */
#define SIMULATE_BO_SIZE_FIX 1
#define SIMULATE_SYNCOBJ     1
#define SIMULATE_SUBMIT      1

#define VIRTGPU_PCI_VENDOR_ID 0x1af4
#define VIRTGPU_PCI_DEVICE_ID 0x1050

struct virtgpu;

struct virtgpu_shmem {
   struct vn_renderer_shmem base;
   uint32_t gem_handle;
};

struct virtgpu_bo {
   struct vn_renderer_bo base;
   uint32_t gem_handle;
   uint32_t blob_flags;
};

struct virtgpu_sync {
   struct vn_renderer_sync base;

   /*
    * drm_syncobj is in one of these states
    *
    *  - value N:      drm_syncobj has a signaled fence chain with seqno N
    *  - pending N->M: drm_syncobj has an unsignaled fence chain with seqno M
    *                  (which may point to another unsignaled fence chain with
    *                   seqno between N and M, and so on)
    *
    * TODO Do we want to use binary drm_syncobjs?  They would be
    *
    *  - value 0: drm_syncobj has no fence
    *  - value 1: drm_syncobj has a signaled fence with seqno 0
    *
    * They are cheaper but require special care.
    */
   uint32_t syncobj_handle;
};

struct virtgpu {
   struct vn_renderer base;

   struct vn_instance *instance;

   int fd;

   bool has_primary;
   int primary_major;
   int primary_minor;
   int render_major;
   int render_minor;

   int bustype;
   drmPciBusInfo pci_bus_info;

   uint32_t max_timeline_count;

   struct {
      enum virgl_renderer_capset id;
      uint32_t version;
      struct virgl_renderer_capset_venus data;
   } capset;

   uint32_t shmem_blob_mem;
   uint32_t bo_blob_mem;

   /* note that we use gem_handle instead of res_id to index because
    * res_id is monotonically increasing by default (see
    * virtio_gpu_resource_id_get)
    */
   struct util_sparse_array shmem_array;
   struct util_sparse_array bo_array;

   mtx_t dma_buf_import_mutex;

   struct vn_renderer_shmem_cache shmem_cache;
};

#ifdef SIMULATE_SYNCOBJ

#include "util/hash_table.h"
#include "util/u_idalloc.h"

static struct {
   mtx_t mutex;
   struct hash_table *syncobjs;
   struct util_idalloc ida;

   int signaled_fd;
} sim;

struct sim_syncobj {
   mtx_t mutex;
   uint64_t point;

   int pending_fd;
   uint64_t pending_point;
   bool pending_cpu;
};

static uint32_t
sim_syncobj_create(struct virtgpu *gpu, bool signaled)
{
   struct sim_syncobj *syncobj = calloc(1, sizeof(*syncobj));
   if (!syncobj)
      return 0;

   mtx_init(&syncobj->mutex, mtx_plain);
   syncobj->pending_fd = -1;

   mtx_lock(&sim.mutex);

   /* initialize lazily */
   if (!sim.syncobjs) {
      sim.syncobjs = _mesa_pointer_hash_table_create(NULL);
      if (!sim.syncobjs) {
         mtx_unlock(&sim.mutex);
         return 0;
      }

      util_idalloc_init(&sim.ida, 32);

      struct drm_virtgpu_execbuffer args = {
         .flags = VIRTGPU_EXECBUF_FENCE_FD_OUT |
                  (gpu->base.info.supports_multiple_timelines
                      ? VIRTGPU_EXECBUF_RING_IDX
                      : 0),
         .ring_idx = 0, /* CPU ring */
      };
      int ret = drmIoctl(gpu->fd, DRM_IOCTL_VIRTGPU_EXECBUFFER, &args);
      if (ret || args.fence_fd < 0) {
         _mesa_hash_table_destroy(sim.syncobjs, NULL);
         sim.syncobjs = NULL;
         mtx_unlock(&sim.mutex);
         return 0;
      }

      sim.signaled_fd = args.fence_fd;
   }

   const unsigned syncobj_handle = util_idalloc_alloc(&sim.ida) + 1;
   _mesa_hash_table_insert(sim.syncobjs,
                           (const void *)(uintptr_t)syncobj_handle, syncobj);

   mtx_unlock(&sim.mutex);

   return syncobj_handle;
}

static void
sim_syncobj_destroy(struct virtgpu *gpu, uint32_t syncobj_handle)
{
   struct sim_syncobj *syncobj = NULL;

   mtx_lock(&sim.mutex);

   struct hash_entry *entry = _mesa_hash_table_search(
      sim.syncobjs, (const void *)(uintptr_t)syncobj_handle);
   if (entry) {
      syncobj = entry->data;
      _mesa_hash_table_remove(sim.syncobjs, entry);
      util_idalloc_free(&sim.ida, syncobj_handle - 1);
   }

   mtx_unlock(&sim.mutex);

   if (syncobj) {
      if (syncobj->pending_fd >= 0)
         close(syncobj->pending_fd);
      mtx_destroy(&syncobj->mutex);
      free(syncobj);
   }
}

static VkResult
sim_syncobj_poll(int fd, int poll_timeout)
{
   struct pollfd pollfd = {
      .fd = fd,
      .events = POLLIN,
   };
   int ret;
   do {
      ret = poll(&pollfd, 1, poll_timeout);
   } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

   if (ret < 0 || (ret > 0 && !(pollfd.revents & POLLIN))) {
      return (ret < 0 && errno == ENOMEM) ? VK_ERROR_OUT_OF_HOST_MEMORY
                                          : VK_ERROR_DEVICE_LOST;
   }

   return ret ? VK_SUCCESS : VK_TIMEOUT;
}

static void
sim_syncobj_set_point_locked(struct sim_syncobj *syncobj, uint64_t point)
{
   syncobj->point = point;

   if (syncobj->pending_fd >= 0) {
      close(syncobj->pending_fd);
      syncobj->pending_fd = -1;
      syncobj->pending_point = point;
   }
}

static void
sim_syncobj_update_point_locked(struct sim_syncobj *syncobj, int poll_timeout)
{
   if (syncobj->pending_fd >= 0) {
      VkResult result;
      if (syncobj->pending_cpu) {
         if (poll_timeout == -1) {
            const int max_cpu_timeout = 2000;
            poll_timeout = max_cpu_timeout;
            result = sim_syncobj_poll(syncobj->pending_fd, poll_timeout);
            if (result == VK_TIMEOUT) {
               vn_log(NULL, "cpu sync timed out after %dms; ignoring",
                      poll_timeout);
               result = VK_SUCCESS;
            }
         } else {
            result = sim_syncobj_poll(syncobj->pending_fd, poll_timeout);
         }
      } else {
         result = sim_syncobj_poll(syncobj->pending_fd, poll_timeout);
      }
      if (result == VK_SUCCESS) {
         close(syncobj->pending_fd);
         syncobj->pending_fd = -1;
         syncobj->point = syncobj->pending_point;
      }
   }
}

static struct sim_syncobj *
sim_syncobj_lookup(struct virtgpu *gpu, uint32_t syncobj_handle)
{
   struct sim_syncobj *syncobj = NULL;

   mtx_lock(&sim.mutex);
   struct hash_entry *entry = _mesa_hash_table_search(
      sim.syncobjs, (const void *)(uintptr_t)syncobj_handle);
   if (entry)
      syncobj = entry->data;
   mtx_unlock(&sim.mutex);

   return syncobj;
}

static int
sim_syncobj_reset(struct virtgpu *gpu, uint32_t syncobj_handle)
{
   struct sim_syncobj *syncobj = sim_syncobj_lookup(gpu, syncobj_handle);
   if (!syncobj)
      return -1;

   mtx_lock(&syncobj->mutex);
   sim_syncobj_set_point_locked(syncobj, 0);
   mtx_unlock(&syncobj->mutex);

   return 0;
}

static int
sim_syncobj_query(struct virtgpu *gpu,
                  uint32_t syncobj_handle,
                  uint64_t *point)
{
   struct sim_syncobj *syncobj = sim_syncobj_lookup(gpu, syncobj_handle);
   if (!syncobj)
      return -1;

   mtx_lock(&syncobj->mutex);
   sim_syncobj_update_point_locked(syncobj, 0);
   *point = syncobj->point;
   mtx_unlock(&syncobj->mutex);

   return 0;
}

static int
sim_syncobj_signal(struct virtgpu *gpu,
                   uint32_t syncobj_handle,
                   uint64_t point)
{
   struct sim_syncobj *syncobj = sim_syncobj_lookup(gpu, syncobj_handle);
   if (!syncobj)
      return -1;

   mtx_lock(&syncobj->mutex);
   sim_syncobj_set_point_locked(syncobj, point);
   mtx_unlock(&syncobj->mutex);

   return 0;
}

static int
sim_syncobj_submit(struct virtgpu *gpu,
                   uint32_t syncobj_handle,
                   int sync_fd,
                   uint64_t point,
                   bool cpu)
{
   struct sim_syncobj *syncobj = sim_syncobj_lookup(gpu, syncobj_handle);
   if (!syncobj)
      return -1;

   int pending_fd = dup(sync_fd);
   if (pending_fd < 0) {
      vn_log(gpu->instance, "failed to dup sync fd");
      return -1;
   }

   mtx_lock(&syncobj->mutex);

   if (syncobj->pending_fd >= 0) {
      mtx_unlock(&syncobj->mutex);

      /* TODO */
      vn_log(gpu->instance, "sorry, no simulated timeline semaphore");
      close(pending_fd);
      return -1;
   }
   if (syncobj->point >= point)
      vn_log(gpu->instance, "non-monotonic signaling");

   syncobj->pending_fd = pending_fd;
   syncobj->pending_point = point;
   syncobj->pending_cpu = cpu;

   mtx_unlock(&syncobj->mutex);

   return 0;
}

static int
timeout_to_poll_timeout(uint64_t timeout)
{
   const uint64_t ns_per_ms = 1000000;
   const uint64_t ms = (timeout + ns_per_ms - 1) / ns_per_ms;
   if (!ms && timeout)
      return -1;
   return ms <= INT_MAX ? ms : -1;
}

static int
sim_syncobj_wait(struct virtgpu *gpu,
                 const struct vn_renderer_wait *wait,
                 bool wait_avail)
{
   if (wait_avail)
      return -1;

   const int poll_timeout = timeout_to_poll_timeout(wait->timeout);

   /* TODO poll all fds at the same time */
   for (uint32_t i = 0; i < wait->sync_count; i++) {
      struct virtgpu_sync *sync = (struct virtgpu_sync *)wait->syncs[i];
      const uint64_t point = wait->sync_values[i];

      struct sim_syncobj *syncobj =
         sim_syncobj_lookup(gpu, sync->syncobj_handle);
      if (!syncobj)
         return -1;

      mtx_lock(&syncobj->mutex);

      if (syncobj->point < point)
         sim_syncobj_update_point_locked(syncobj, poll_timeout);

      if (syncobj->point < point) {
         if (wait->wait_any && i < wait->sync_count - 1 &&
             syncobj->pending_fd < 0) {
            mtx_unlock(&syncobj->mutex);
            continue;
         }
         errno = ETIME;
         mtx_unlock(&syncobj->mutex);
         return -1;
      }

      mtx_unlock(&syncobj->mutex);

      if (wait->wait_any)
         break;

      /* TODO adjust poll_timeout */
   }

   return 0;
}

static int
sim_syncobj_export(struct virtgpu *gpu, uint32_t syncobj_handle)
{
   struct sim_syncobj *syncobj = sim_syncobj_lookup(gpu, syncobj_handle);
   if (!syncobj)
      return -1;

   int fd = -1;
   mtx_lock(&syncobj->mutex);
   if (syncobj->pending_fd >= 0)
      fd = dup(syncobj->pending_fd);
   else
      fd = dup(sim.signaled_fd);
   mtx_unlock(&syncobj->mutex);

   return fd;
}

static uint32_t
sim_syncobj_import(struct virtgpu *gpu, uint32_t syncobj_handle, int fd)
{
   struct sim_syncobj *syncobj = sim_syncobj_lookup(gpu, syncobj_handle);
   if (!syncobj)
      return 0;

   if (sim_syncobj_submit(gpu, syncobj_handle, fd, 1, false))
      return 0;

   return syncobj_handle;
}

#endif /* SIMULATE_SYNCOBJ */

#ifdef SIMULATE_SUBMIT

static int
sim_submit_signal_syncs(struct virtgpu *gpu,
                        int sync_fd,
                        struct vn_renderer_sync *const *syncs,
                        const uint64_t *sync_values,
                        uint32_t sync_count,
                        bool cpu)
{
   for (uint32_t i = 0; i < sync_count; i++) {
      struct virtgpu_sync *sync = (struct virtgpu_sync *)syncs[i];
      const uint64_t pending_point = sync_values[i];

#ifdef SIMULATE_SYNCOBJ
      int ret = sim_syncobj_submit(gpu, sync->syncobj_handle, sync_fd,
                                   pending_point, cpu);
      if (ret)
         return ret;
#else
      /* we can in theory do a DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE followed by a
       * DRM_IOCTL_SYNCOBJ_TRANSFER
       */
      return -1;
#endif
   }

   return 0;
}

static uint32_t *
sim_submit_alloc_gem_handles(struct vn_renderer_bo *const *bos,
                             uint32_t bo_count)
{
   uint32_t *gem_handles = malloc(sizeof(*gem_handles) * bo_count);
   if (!gem_handles)
      return NULL;

   for (uint32_t i = 0; i < bo_count; i++) {
      struct virtgpu_bo *bo = (struct virtgpu_bo *)bos[i];
      gem_handles[i] = bo->gem_handle;
   }

   return gem_handles;
}

static int
sim_submit(struct virtgpu *gpu, const struct vn_renderer_submit *submit)
{
   const bool use_ring_idx = gpu->base.info.supports_multiple_timelines;

   /* TODO replace submit->bos by submit->gem_handles to avoid malloc/loop */
   uint32_t *gem_handles = NULL;
   if (submit->bo_count) {
      gem_handles =
         sim_submit_alloc_gem_handles(submit->bos, submit->bo_count);
      if (!gem_handles)
         return -1;
   }

   assert(submit->batch_count);

   int ret = 0;
   for (uint32_t i = 0; i < submit->batch_count; i++) {
      const struct vn_renderer_submit_batch *batch = &submit->batches[i];

      struct drm_virtgpu_execbuffer args = {
         .flags = (batch->sync_count ? VIRTGPU_EXECBUF_FENCE_FD_OUT : 0) |
                  (use_ring_idx ? VIRTGPU_EXECBUF_RING_IDX : 0),
         .size = batch->cs_size,
         .command = (uintptr_t)batch->cs_data,
         .bo_handles = (uintptr_t)gem_handles,
         .num_bo_handles = submit->bo_count,
         .ring_idx = (use_ring_idx ? batch->ring_idx : 0),
      };

      ret = drmIoctl(gpu->fd, DRM_IOCTL_VIRTGPU_EXECBUFFER, &args);
      if (ret) {
         vn_log(gpu->instance, "failed to execbuffer: %s", strerror(errno));
         break;
      }

      if (batch->sync_count) {
         ret = sim_submit_signal_syncs(gpu, args.fence_fd, batch->syncs,
                                       batch->sync_values, batch->sync_count,
                                       batch->ring_idx == 0);
         close(args.fence_fd);
         if (ret)
            break;
      }
   }

   free(gem_handles);
   return ret;
}

#endif /* SIMULATE_SUBMIT */

static int
virtgpu_ioctl(struct virtgpu *gpu, unsigned long request, void *args)
{
   return drmIoctl(gpu->fd, request, args);
}

static uint64_t
virtgpu_ioctl_getparam(struct virtgpu *gpu, uint64_t param)
{
   /* val must be zeroed because kernel only writes the lower 32 bits */
   uint64_t val = 0;
   struct drm_virtgpu_getparam args = {
      .param = param,
      .value = (uintptr_t)&val,
   };

   const int ret = virtgpu_ioctl(gpu, DRM_IOCTL_VIRTGPU_GETPARAM, &args);
   return ret ? 0 : val;
}

static int
virtgpu_ioctl_get_caps(struct virtgpu *gpu,
                       enum virgl_renderer_capset id,
                       uint32_t version,
                       void *capset,
                       size_t capset_size)
{
   struct drm_virtgpu_get_caps args = {
      .cap_set_id = id,
      .cap_set_ver = version,
      .addr = (uintptr_t)capset,
      .size = capset_size,
   };

   return virtgpu_ioctl(gpu, DRM_IOCTL_VIRTGPU_GET_CAPS, &args);
}

static int
virtgpu_ioctl_context_init(struct virtgpu *gpu,
                           enum virgl_renderer_capset capset_id)
{
   struct drm_virtgpu_context_set_param ctx_set_params[3] = {
      {
         .param = VIRTGPU_CONTEXT_PARAM_CAPSET_ID,
         .value = capset_id,
      },
      {
         .param = VIRTGPU_CONTEXT_PARAM_NUM_RINGS,
         .value = 64,
      },
      {
         .param = VIRTGPU_CONTEXT_PARAM_POLL_RINGS_MASK,
         .value = 0, /* don't generate drm_events on fence signaling */
      },
   };

   struct drm_virtgpu_context_init args = {
      .num_params = ARRAY_SIZE(ctx_set_params),
      .ctx_set_params = (uintptr_t)&ctx_set_params,
   };

   return virtgpu_ioctl(gpu, DRM_IOCTL_VIRTGPU_CONTEXT_INIT, &args);
}

static uint32_t
virtgpu_ioctl_resource_create_blob(struct virtgpu *gpu,
                                   uint32_t blob_mem,
                                   uint32_t blob_flags,
                                   size_t blob_size,
                                   uint64_t blob_id,
                                   uint32_t *res_id)
{
#ifdef SIMULATE_BO_SIZE_FIX
   blob_size = align64(blob_size, 4096);
#endif

   struct drm_virtgpu_resource_create_blob args = {
      .blob_mem = blob_mem,
      .blob_flags = blob_flags,
      .size = blob_size,
      .blob_id = blob_id,
   };

   if (virtgpu_ioctl(gpu, DRM_IOCTL_VIRTGPU_RESOURCE_CREATE_BLOB, &args))
      return 0;

   *res_id = args.res_handle;
   return args.bo_handle;
}

static int
virtgpu_ioctl_resource_info(struct virtgpu *gpu,
                            uint32_t gem_handle,
                            struct drm_virtgpu_resource_info *info)
{
   *info = (struct drm_virtgpu_resource_info){
      .bo_handle = gem_handle,
   };

   return virtgpu_ioctl(gpu, DRM_IOCTL_VIRTGPU_RESOURCE_INFO, info);
}

static void
virtgpu_ioctl_gem_close(struct virtgpu *gpu, uint32_t gem_handle)
{
   struct drm_gem_close args = {
      .handle = gem_handle,
   };

   ASSERTED const int ret = virtgpu_ioctl(gpu, DRM_IOCTL_GEM_CLOSE, &args);
   assert(!ret);
}

static int
virtgpu_ioctl_prime_handle_to_fd(struct virtgpu *gpu,
                                 uint32_t gem_handle,
                                 bool mappable)
{
   struct drm_prime_handle args = {
      .handle = gem_handle,
      .flags = DRM_CLOEXEC | (mappable ? DRM_RDWR : 0),
   };

   const int ret = virtgpu_ioctl(gpu, DRM_IOCTL_PRIME_HANDLE_TO_FD, &args);
   return ret ? -1 : args.fd;
}

static uint32_t
virtgpu_ioctl_prime_fd_to_handle(struct virtgpu *gpu, int fd)
{
   struct drm_prime_handle args = {
      .fd = fd,
   };

   const int ret = virtgpu_ioctl(gpu, DRM_IOCTL_PRIME_FD_TO_HANDLE, &args);
   return ret ? 0 : args.handle;
}

static void *
virtgpu_ioctl_map(struct virtgpu *gpu, uint32_t gem_handle, size_t size)
{
   struct drm_virtgpu_map args = {
      .handle = gem_handle,
   };

   if (virtgpu_ioctl(gpu, DRM_IOCTL_VIRTGPU_MAP, &args))
      return NULL;

   void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, gpu->fd,
                    args.offset);
   if (ptr == MAP_FAILED)
      return NULL;

   return ptr;
}

static uint32_t
virtgpu_ioctl_syncobj_create(struct virtgpu *gpu, bool signaled)
{
#ifdef SIMULATE_SYNCOBJ
   return sim_syncobj_create(gpu, signaled);
#endif

   struct drm_syncobj_create args = {
      .flags = signaled ? DRM_SYNCOBJ_CREATE_SIGNALED : 0,
   };

   const int ret = virtgpu_ioctl(gpu, DRM_IOCTL_SYNCOBJ_CREATE, &args);
   return ret ? 0 : args.handle;
}

static void
virtgpu_ioctl_syncobj_destroy(struct virtgpu *gpu, uint32_t syncobj_handle)
{
#ifdef SIMULATE_SYNCOBJ
   sim_syncobj_destroy(gpu, syncobj_handle);
   return;
#endif

   struct drm_syncobj_destroy args = {
      .handle = syncobj_handle,
   };

   ASSERTED const int ret =
      virtgpu_ioctl(gpu, DRM_IOCTL_SYNCOBJ_DESTROY, &args);
   assert(!ret);
}

static int
virtgpu_ioctl_syncobj_handle_to_fd(struct virtgpu *gpu,
                                   uint32_t syncobj_handle,
                                   bool sync_file)
{
#ifdef SIMULATE_SYNCOBJ
   return sync_file ? sim_syncobj_export(gpu, syncobj_handle) : -1;
#endif

   struct drm_syncobj_handle args = {
      .handle = syncobj_handle,
      .flags =
         sync_file ? DRM_SYNCOBJ_HANDLE_TO_FD_FLAGS_EXPORT_SYNC_FILE : 0,
   };

   int ret = virtgpu_ioctl(gpu, DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD, &args);
   if (ret)
      return -1;

   return args.fd;
}

static uint32_t
virtgpu_ioctl_syncobj_fd_to_handle(struct virtgpu *gpu,
                                   int fd,
                                   uint32_t syncobj_handle)
{
#ifdef SIMULATE_SYNCOBJ
   return syncobj_handle ? sim_syncobj_import(gpu, syncobj_handle, fd) : 0;
#endif

   struct drm_syncobj_handle args = {
      .handle = syncobj_handle,
      .flags =
         syncobj_handle ? DRM_SYNCOBJ_FD_TO_HANDLE_FLAGS_IMPORT_SYNC_FILE : 0,
      .fd = fd,
   };

   int ret = virtgpu_ioctl(gpu, DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE, &args);
   if (ret)
      return 0;

   return args.handle;
}

static int
virtgpu_ioctl_syncobj_reset(struct virtgpu *gpu, uint32_t syncobj_handle)
{
#ifdef SIMULATE_SYNCOBJ
   return sim_syncobj_reset(gpu, syncobj_handle);
#endif

   struct drm_syncobj_array args = {
      .handles = (uintptr_t)&syncobj_handle,
      .count_handles = 1,
   };

   return virtgpu_ioctl(gpu, DRM_IOCTL_SYNCOBJ_RESET, &args);
}

static int
virtgpu_ioctl_syncobj_query(struct virtgpu *gpu,
                            uint32_t syncobj_handle,
                            uint64_t *point)
{
#ifdef SIMULATE_SYNCOBJ
   return sim_syncobj_query(gpu, syncobj_handle, point);
#endif

   struct drm_syncobj_timeline_array args = {
      .handles = (uintptr_t)&syncobj_handle,
      .points = (uintptr_t)point,
      .count_handles = 1,
   };

   return virtgpu_ioctl(gpu, DRM_IOCTL_SYNCOBJ_QUERY, &args);
}

static int
virtgpu_ioctl_syncobj_timeline_signal(struct virtgpu *gpu,
                                      uint32_t syncobj_handle,
                                      uint64_t point)
{
#ifdef SIMULATE_SYNCOBJ
   return sim_syncobj_signal(gpu, syncobj_handle, point);
#endif

   struct drm_syncobj_timeline_array args = {
      .handles = (uintptr_t)&syncobj_handle,
      .points = (uintptr_t)&point,
      .count_handles = 1,
   };

   return virtgpu_ioctl(gpu, DRM_IOCTL_SYNCOBJ_TIMELINE_SIGNAL, &args);
}

static int
virtgpu_ioctl_syncobj_timeline_wait(struct virtgpu *gpu,
                                    const struct vn_renderer_wait *wait,
                                    bool wait_avail)
{
#ifdef SIMULATE_SYNCOBJ
   return sim_syncobj_wait(gpu, wait, wait_avail);
#endif

   /* always enable wait-before-submit */
   uint32_t flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT;
   if (!wait->wait_any)
      flags |= DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
   /* wait for fences to appear instead of signaling */
   if (wait_avail)
      flags |= DRM_SYNCOBJ_WAIT_FLAGS_WAIT_AVAILABLE;

   /* TODO replace wait->syncs by wait->sync_handles to avoid malloc/loop */
   uint32_t *syncobj_handles =
      malloc(sizeof(*syncobj_handles) * wait->sync_count);
   if (!syncobj_handles)
      return -1;
   for (uint32_t i = 0; i < wait->sync_count; i++) {
      struct virtgpu_sync *sync = (struct virtgpu_sync *)wait->syncs[i];
      syncobj_handles[i] = sync->syncobj_handle;
   }

   struct drm_syncobj_timeline_wait args = {
      .handles = (uintptr_t)syncobj_handles,
      .points = (uintptr_t)wait->sync_values,
      .timeout_nsec = os_time_get_absolute_timeout(wait->timeout),
      .count_handles = wait->sync_count,
      .flags = flags,
   };

   const int ret = virtgpu_ioctl(gpu, DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT, &args);

   free(syncobj_handles);

   return ret;
}

static int
virtgpu_ioctl_submit(struct virtgpu *gpu,
                     const struct vn_renderer_submit *submit)
{
#ifdef SIMULATE_SUBMIT
   return sim_submit(gpu, submit);
#endif
   return -1;
}

static VkResult
virtgpu_sync_write(struct vn_renderer *renderer,
                   struct vn_renderer_sync *_sync,
                   uint64_t val)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_sync *sync = (struct virtgpu_sync *)_sync;

   const int ret =
      virtgpu_ioctl_syncobj_timeline_signal(gpu, sync->syncobj_handle, val);

   return ret ? VK_ERROR_OUT_OF_DEVICE_MEMORY : VK_SUCCESS;
}

static VkResult
virtgpu_sync_read(struct vn_renderer *renderer,
                  struct vn_renderer_sync *_sync,
                  uint64_t *val)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_sync *sync = (struct virtgpu_sync *)_sync;

   const int ret =
      virtgpu_ioctl_syncobj_query(gpu, sync->syncobj_handle, val);

   return ret ? VK_ERROR_OUT_OF_DEVICE_MEMORY : VK_SUCCESS;
}

static VkResult
virtgpu_sync_reset(struct vn_renderer *renderer,
                   struct vn_renderer_sync *_sync,
                   uint64_t initial_val)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_sync *sync = (struct virtgpu_sync *)_sync;

   int ret = virtgpu_ioctl_syncobj_reset(gpu, sync->syncobj_handle);
   if (!ret) {
      ret = virtgpu_ioctl_syncobj_timeline_signal(gpu, sync->syncobj_handle,
                                                  initial_val);
   }

   return ret ? VK_ERROR_OUT_OF_DEVICE_MEMORY : VK_SUCCESS;
}

static int
virtgpu_sync_export_syncobj(struct vn_renderer *renderer,
                            struct vn_renderer_sync *_sync,
                            bool sync_file)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_sync *sync = (struct virtgpu_sync *)_sync;

   return virtgpu_ioctl_syncobj_handle_to_fd(gpu, sync->syncobj_handle,
                                             sync_file);
}

static void
virtgpu_sync_destroy(struct vn_renderer *renderer,
                     struct vn_renderer_sync *_sync)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_sync *sync = (struct virtgpu_sync *)_sync;

   virtgpu_ioctl_syncobj_destroy(gpu, sync->syncobj_handle);

   free(sync);
}

static VkResult
virtgpu_sync_create_from_syncobj(struct vn_renderer *renderer,
                                 int fd,
                                 bool sync_file,
                                 struct vn_renderer_sync **out_sync)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;

   uint32_t syncobj_handle;
   if (sync_file) {
      syncobj_handle = virtgpu_ioctl_syncobj_create(gpu, false);
      if (!syncobj_handle)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      if (!virtgpu_ioctl_syncobj_fd_to_handle(gpu, fd, syncobj_handle)) {
         virtgpu_ioctl_syncobj_destroy(gpu, syncobj_handle);
         return VK_ERROR_INVALID_EXTERNAL_HANDLE;
      }
   } else {
      syncobj_handle = virtgpu_ioctl_syncobj_fd_to_handle(gpu, fd, 0);
      if (!syncobj_handle)
         return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }

   struct virtgpu_sync *sync = calloc(1, sizeof(*sync));
   if (!sync) {
      virtgpu_ioctl_syncobj_destroy(gpu, syncobj_handle);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   sync->syncobj_handle = syncobj_handle;
   sync->base.sync_id = 0; /* TODO */

   *out_sync = &sync->base;

   return VK_SUCCESS;
}

static VkResult
virtgpu_sync_create(struct vn_renderer *renderer,
                    uint64_t initial_val,
                    uint32_t flags,
                    struct vn_renderer_sync **out_sync)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;

   /* TODO */
   if (flags & VN_RENDERER_SYNC_SHAREABLE)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   /* always false because we don't use binary drm_syncobjs */
   const bool signaled = false;
   const uint32_t syncobj_handle =
      virtgpu_ioctl_syncobj_create(gpu, signaled);
   if (!syncobj_handle)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   /* add a signaled fence chain with seqno initial_val */
   const int ret =
      virtgpu_ioctl_syncobj_timeline_signal(gpu, syncobj_handle, initial_val);
   if (ret) {
      virtgpu_ioctl_syncobj_destroy(gpu, syncobj_handle);
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;
   }

   struct virtgpu_sync *sync = calloc(1, sizeof(*sync));
   if (!sync) {
      virtgpu_ioctl_syncobj_destroy(gpu, syncobj_handle);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   sync->syncobj_handle = syncobj_handle;
   /* we will have a sync_id when shareable is true and virtio-gpu associates
    * a host sync object with guest drm_syncobj
    */
   sync->base.sync_id = 0;

   *out_sync = &sync->base;

   return VK_SUCCESS;
}

static void
virtgpu_bo_invalidate(struct vn_renderer *renderer,
                      struct vn_renderer_bo *bo,
                      VkDeviceSize offset,
                      VkDeviceSize size)
{
   /* nop because kernel makes every mapping coherent */
}

static void
virtgpu_bo_flush(struct vn_renderer *renderer,
                 struct vn_renderer_bo *bo,
                 VkDeviceSize offset,
                 VkDeviceSize size)
{
   /* nop because kernel makes every mapping coherent */
}

static void *
virtgpu_bo_map(struct vn_renderer *renderer, struct vn_renderer_bo *_bo)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_bo *bo = (struct virtgpu_bo *)_bo;
   const bool mappable = bo->blob_flags & VIRTGPU_BLOB_FLAG_USE_MAPPABLE;

   /* not thread-safe but is fine */
   if (!bo->base.mmap_ptr && mappable) {
      bo->base.mmap_ptr =
         virtgpu_ioctl_map(gpu, bo->gem_handle, bo->base.mmap_size);
   }

   return bo->base.mmap_ptr;
}

static int
virtgpu_bo_export_dma_buf(struct vn_renderer *renderer,
                          struct vn_renderer_bo *_bo)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_bo *bo = (struct virtgpu_bo *)_bo;
   const bool mappable = bo->blob_flags & VIRTGPU_BLOB_FLAG_USE_MAPPABLE;
   const bool shareable = bo->blob_flags & VIRTGPU_BLOB_FLAG_USE_SHAREABLE;

   return shareable
             ? virtgpu_ioctl_prime_handle_to_fd(gpu, bo->gem_handle, mappable)
             : -1;
}

static bool
virtgpu_bo_destroy(struct vn_renderer *renderer, struct vn_renderer_bo *_bo)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_bo *bo = (struct virtgpu_bo *)_bo;

   mtx_lock(&gpu->dma_buf_import_mutex);

   /* Check the refcount again after the import lock is grabbed.  Yes, we use
    * the double-checked locking anti-pattern.
    */
   if (vn_refcount_is_valid(&bo->base.refcount)) {
      mtx_unlock(&gpu->dma_buf_import_mutex);
      return false;
   }

   if (bo->base.mmap_ptr)
      munmap(bo->base.mmap_ptr, bo->base.mmap_size);
   virtgpu_ioctl_gem_close(gpu, bo->gem_handle);

   /* set gem_handle to 0 to indicate that the bo is invalid */
   bo->gem_handle = 0;

   mtx_unlock(&gpu->dma_buf_import_mutex);

   return true;
}

static uint32_t
virtgpu_bo_blob_flags(VkMemoryPropertyFlags flags,
                      VkExternalMemoryHandleTypeFlags external_handles)
{
   uint32_t blob_flags = 0;
   if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      blob_flags |= VIRTGPU_BLOB_FLAG_USE_MAPPABLE;
   if (external_handles)
      blob_flags |= VIRTGPU_BLOB_FLAG_USE_SHAREABLE;
   if (external_handles & VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT)
      blob_flags |= VIRTGPU_BLOB_FLAG_USE_CROSS_DEVICE;

   return blob_flags;
}

static VkResult
virtgpu_bo_create_from_dma_buf(struct vn_renderer *renderer,
                               VkDeviceSize size,
                               int fd,
                               VkMemoryPropertyFlags flags,
                               struct vn_renderer_bo **out_bo)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct drm_virtgpu_resource_info info;
   uint32_t gem_handle = 0;
   struct virtgpu_bo *bo = NULL;

   mtx_lock(&gpu->dma_buf_import_mutex);

   gem_handle = virtgpu_ioctl_prime_fd_to_handle(gpu, fd);
   if (!gem_handle)
      goto fail;
   bo = util_sparse_array_get(&gpu->bo_array, gem_handle);

   if (virtgpu_ioctl_resource_info(gpu, gem_handle, &info))
      goto fail;

   /* Upon import, blob_flags is not passed to the kernel and is only for
    * internal use. Set it to what works best for us.
    * - blob mem: SHAREABLE + conditional MAPPABLE per VkMemoryPropertyFlags
    * - classic 3d: SHAREABLE only for export and to fail the map
    */
   uint32_t blob_flags = VIRTGPU_BLOB_FLAG_USE_SHAREABLE;
   size_t mmap_size = 0;
   if (info.blob_mem) {
      /* must be VIRTGPU_BLOB_MEM_HOST3D or VIRTGPU_BLOB_MEM_GUEST_VRAM */
      if (info.blob_mem != gpu->bo_blob_mem)
         goto fail;

      blob_flags |= virtgpu_bo_blob_flags(flags, 0);

      /* mmap_size is only used when mappable */
      mmap_size = 0;
      if (blob_flags & VIRTGPU_BLOB_FLAG_USE_MAPPABLE) {
         if (info.size < size)
            goto fail;

         mmap_size = size;
      }
   }

   /* we check bo->gem_handle instead of bo->refcount because bo->refcount
    * might only be memset to 0 and is not considered initialized in theory
    */
   if (bo->gem_handle == gem_handle) {
      if (bo->base.mmap_size < mmap_size)
         goto fail;
      if (blob_flags & ~bo->blob_flags)
         goto fail;

      /* we can't use vn_renderer_bo_ref as the refcount may drop to 0
       * temporarily before virtgpu_bo_destroy grabs the lock
       */
      vn_refcount_fetch_add_relaxed(&bo->base.refcount, 1);
   } else {
      *bo = (struct virtgpu_bo){
         .base = {
            .refcount = VN_REFCOUNT_INIT(1),
            .res_id = info.res_handle,
            .mmap_size = mmap_size,
         },
         .gem_handle = gem_handle,
         .blob_flags = blob_flags,
      };
   }

   mtx_unlock(&gpu->dma_buf_import_mutex);

   *out_bo = &bo->base;

   return VK_SUCCESS;

fail:
   if (gem_handle && bo->gem_handle != gem_handle)
      virtgpu_ioctl_gem_close(gpu, gem_handle);
   mtx_unlock(&gpu->dma_buf_import_mutex);
   return VK_ERROR_INVALID_EXTERNAL_HANDLE;
}

static VkResult
virtgpu_bo_create_from_device_memory(
   struct vn_renderer *renderer,
   VkDeviceSize size,
   vn_object_id mem_id,
   VkMemoryPropertyFlags flags,
   VkExternalMemoryHandleTypeFlags external_handles,
   struct vn_renderer_bo **out_bo)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   const uint32_t blob_flags = virtgpu_bo_blob_flags(flags, external_handles);

   uint32_t res_id;
   uint32_t gem_handle = virtgpu_ioctl_resource_create_blob(
      gpu, gpu->bo_blob_mem, blob_flags, size, mem_id, &res_id);
   if (!gem_handle)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   struct virtgpu_bo *bo = util_sparse_array_get(&gpu->bo_array, gem_handle);
   *bo = (struct virtgpu_bo){
      .base = {
         .refcount = VN_REFCOUNT_INIT(1),
         .res_id = res_id,
         .mmap_size = size,
      },
      .gem_handle = gem_handle,
      .blob_flags = blob_flags,
   };

   *out_bo = &bo->base;

   return VK_SUCCESS;
}

static void
virtgpu_shmem_destroy_now(struct vn_renderer *renderer,
                          struct vn_renderer_shmem *_shmem)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;
   struct virtgpu_shmem *shmem = (struct virtgpu_shmem *)_shmem;

   munmap(shmem->base.mmap_ptr, shmem->base.mmap_size);
   virtgpu_ioctl_gem_close(gpu, shmem->gem_handle);
}

static void
virtgpu_shmem_destroy(struct vn_renderer *renderer,
                      struct vn_renderer_shmem *shmem)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;

   if (vn_renderer_shmem_cache_add(&gpu->shmem_cache, shmem))
      return;

   virtgpu_shmem_destroy_now(&gpu->base, shmem);
}

static struct vn_renderer_shmem *
virtgpu_shmem_create(struct vn_renderer *renderer, size_t size)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;

   struct vn_renderer_shmem *cached_shmem =
      vn_renderer_shmem_cache_get(&gpu->shmem_cache, size);
   if (cached_shmem) {
      cached_shmem->refcount = VN_REFCOUNT_INIT(1);
      return cached_shmem;
   }

   uint32_t res_id;
   uint32_t gem_handle = virtgpu_ioctl_resource_create_blob(
      gpu, gpu->shmem_blob_mem, VIRTGPU_BLOB_FLAG_USE_MAPPABLE, size, 0,
      &res_id);
   if (!gem_handle)
      return NULL;

   void *ptr = virtgpu_ioctl_map(gpu, gem_handle, size);
   if (!ptr) {
      virtgpu_ioctl_gem_close(gpu, gem_handle);
      return NULL;
   }

   struct virtgpu_shmem *shmem =
      util_sparse_array_get(&gpu->shmem_array, gem_handle);
   *shmem = (struct virtgpu_shmem){
      .base = {
         .refcount = VN_REFCOUNT_INIT(1),
         .res_id = res_id,
         .mmap_size = size,
         .mmap_ptr = ptr,
      },
      .gem_handle = gem_handle,
   };

   return &shmem->base;
}

static VkResult
virtgpu_wait(struct vn_renderer *renderer,
             const struct vn_renderer_wait *wait)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;

   const int ret = virtgpu_ioctl_syncobj_timeline_wait(gpu, wait, false);
   if (ret && errno != ETIME)
      return VK_ERROR_DEVICE_LOST;

   return ret ? VK_TIMEOUT : VK_SUCCESS;
}

static VkResult
virtgpu_submit(struct vn_renderer *renderer,
               const struct vn_renderer_submit *submit)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;

   const int ret = virtgpu_ioctl_submit(gpu, submit);
   return ret ? VK_ERROR_DEVICE_LOST : VK_SUCCESS;
}

static void
virtgpu_init_renderer_info(struct virtgpu *gpu)
{
   struct vn_renderer_info *info = &gpu->base.info;

   info->drm.has_primary = gpu->has_primary;
   info->drm.primary_major = gpu->primary_major;
   info->drm.primary_minor = gpu->primary_minor;
   info->drm.has_render = true;
   info->drm.render_major = gpu->render_major;
   info->drm.render_minor = gpu->render_minor;

   info->pci.vendor_id = VIRTGPU_PCI_VENDOR_ID;
   info->pci.device_id = VIRTGPU_PCI_DEVICE_ID;

   if (gpu->bustype == DRM_BUS_PCI) {
      info->pci.has_bus_info = true;
      info->pci.domain = gpu->pci_bus_info.domain;
      info->pci.bus = gpu->pci_bus_info.bus;
      info->pci.device = gpu->pci_bus_info.dev;
      info->pci.function = gpu->pci_bus_info.func;
   } else {
      info->pci.has_bus_info = false;
   }

   info->has_dma_buf_import = true;
   /* TODO switch from emulation to drm_syncobj */
   info->has_external_sync = true;

   info->has_implicit_fencing = false;

   const struct virgl_renderer_capset_venus *capset = &gpu->capset.data;
   info->wire_format_version = capset->wire_format_version;
   info->vk_xml_version = capset->vk_xml_version;
   info->vk_ext_command_serialization_spec_version =
      capset->vk_ext_command_serialization_spec_version;
   info->vk_mesa_venus_protocol_spec_version =
      capset->vk_mesa_venus_protocol_spec_version;
   info->supports_blob_id_0 = capset->supports_blob_id_0;

   /* ensure vk_extension_mask is large enough to hold all capset masks */
   STATIC_ASSERT(sizeof(info->vk_extension_mask) >=
                 sizeof(capset->vk_extension_mask1));
   memcpy(info->vk_extension_mask, capset->vk_extension_mask1,
          sizeof(capset->vk_extension_mask1));

   info->allow_vk_wait_syncs = capset->allow_vk_wait_syncs;

   info->supports_multiple_timelines = capset->supports_multiple_timelines;
   info->max_timeline_count = gpu->max_timeline_count;

   if (gpu->bo_blob_mem == VIRTGPU_BLOB_MEM_GUEST_VRAM)
      info->has_guest_vram = true;

   /* Use guest blob allocations from dedicated heap (Host visible memory) */
   if (gpu->bo_blob_mem == VIRTGPU_BLOB_MEM_HOST3D && capset->use_guest_vram)
      info->has_guest_vram = true;
}

static void
virtgpu_destroy(struct vn_renderer *renderer,
                const VkAllocationCallbacks *alloc)
{
   struct virtgpu *gpu = (struct virtgpu *)renderer;

   vn_renderer_shmem_cache_fini(&gpu->shmem_cache);

   if (gpu->fd >= 0)
      close(gpu->fd);

   mtx_destroy(&gpu->dma_buf_import_mutex);

   util_sparse_array_finish(&gpu->shmem_array);
   util_sparse_array_finish(&gpu->bo_array);

   vk_free(alloc, gpu);
}

static inline void
virtgpu_init_shmem_blob_mem(ASSERTED struct virtgpu *gpu)
{
   /* VIRTGPU_BLOB_MEM_GUEST allocates from the guest system memory.  They are
    * logically contiguous in the guest but are sglists (iovecs) in the host.
    * That makes them slower to process in the host.  With host process
    * isolation, it also becomes impossible for the host to access sglists
    * directly.
    *
    * While there are ideas (and shipped code in some cases) such as creating
    * udmabufs from sglists, or having a dedicated guest heap, it seems the
    * easiest way is to reuse VIRTGPU_BLOB_MEM_HOST3D.  That is, when the
    * renderer sees a request to export a blob where
    *
    *  - blob_mem is VIRTGPU_BLOB_MEM_HOST3D
    *  - blob_flags is VIRTGPU_BLOB_FLAG_USE_MAPPABLE
    *  - blob_id is 0
    *
    * it allocates a host shmem.
    *
    * supports_blob_id_0 has been enforced by mandated render server config.
    */
   assert(gpu->capset.data.supports_blob_id_0);
   gpu->shmem_blob_mem = VIRTGPU_BLOB_MEM_HOST3D;
}

static VkResult
virtgpu_init_context(struct virtgpu *gpu)
{
   assert(!gpu->capset.version);
   const int ret = virtgpu_ioctl_context_init(gpu, gpu->capset.id);
   if (ret) {
      if (VN_DEBUG(INIT)) {
         vn_log(gpu->instance, "failed to initialize context: %s",
                strerror(errno));
      }
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   return VK_SUCCESS;
}

static VkResult
virtgpu_init_capset(struct virtgpu *gpu)
{
   gpu->capset.id = VIRGL_RENDERER_CAPSET_VENUS;
   gpu->capset.version = 0;

   const int ret =
      virtgpu_ioctl_get_caps(gpu, gpu->capset.id, gpu->capset.version,
                             &gpu->capset.data, sizeof(gpu->capset.data));
   if (ret) {
      if (VN_DEBUG(INIT)) {
         vn_log(gpu->instance, "failed to get venus v%d capset: %s",
                gpu->capset.version, strerror(errno));
      }
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   return VK_SUCCESS;
}

static VkResult
virtgpu_init_params(struct virtgpu *gpu)
{
   const uint64_t required_params[] = {
      VIRTGPU_PARAM_3D_FEATURES,   VIRTGPU_PARAM_CAPSET_QUERY_FIX,
      VIRTGPU_PARAM_RESOURCE_BLOB, VIRTGPU_PARAM_CROSS_DEVICE,
      VIRTGPU_PARAM_CONTEXT_INIT,
   };
   uint64_t val;
   for (uint32_t i = 0; i < ARRAY_SIZE(required_params); i++) {
      val = virtgpu_ioctl_getparam(gpu, required_params[i]);
      if (!val) {
         if (VN_DEBUG(INIT)) {
            vn_log(gpu->instance, "required kernel param %d is missing",
                   (int)required_params[i]);
         }
         return VK_ERROR_INITIALIZATION_FAILED;
      }
   }

   val = virtgpu_ioctl_getparam(gpu, VIRTGPU_PARAM_HOST_VISIBLE);
   if (val) {
      gpu->bo_blob_mem = VIRTGPU_BLOB_MEM_HOST3D;
   } else {
      val = virtgpu_ioctl_getparam(gpu, VIRTGPU_PARAM_GUEST_VRAM);
      if (val) {
         gpu->bo_blob_mem = VIRTGPU_BLOB_MEM_GUEST_VRAM;
      }
   }

   if (!val) {
      vn_log(gpu->instance,
             "one of required kernel params (%d or %d) is missing",
             (int)VIRTGPU_PARAM_HOST_VISIBLE, (int)VIRTGPU_PARAM_GUEST_VRAM);
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   /* implied by CONTEXT_INIT uapi */
   gpu->max_timeline_count = 64;

   return VK_SUCCESS;
}

static VkResult
virtgpu_open_device(struct virtgpu *gpu, const drmDevicePtr dev)
{
   bool supported_bus = false;

   switch (dev->bustype) {
   case DRM_BUS_PCI:
      if (dev->deviceinfo.pci->vendor_id == VIRTGPU_PCI_VENDOR_ID &&
          dev->deviceinfo.pci->device_id == VIRTGPU_PCI_DEVICE_ID)
         supported_bus = true;
      break;
   case DRM_BUS_PLATFORM:
      supported_bus = true;
      break;
   default:
      break;
   }

   if (!supported_bus || !(dev->available_nodes & (1 << DRM_NODE_RENDER))) {
      if (VN_DEBUG(INIT)) {
         const char *name = "unknown";
         for (uint32_t i = 0; i < DRM_NODE_MAX; i++) {
            if (dev->available_nodes & (1 << i)) {
               name = dev->nodes[i];
               break;
            }
         }
         vn_log(gpu->instance, "skipping DRM device %s", name);
      }
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   const char *primary_path = dev->nodes[DRM_NODE_PRIMARY];
   const char *node_path = dev->nodes[DRM_NODE_RENDER];

   int fd = open(node_path, O_RDWR | O_CLOEXEC);
   if (fd < 0) {
      if (VN_DEBUG(INIT))
         vn_log(gpu->instance, "failed to open %s", node_path);
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   drmVersionPtr version = drmGetVersion(fd);
   if (!version || strcmp(version->name, "virtio_gpu") ||
       version->version_major != 0) {
      if (VN_DEBUG(INIT)) {
         if (version) {
            vn_log(gpu->instance, "unknown DRM driver %s version %d",
                   version->name, version->version_major);
         } else {
            vn_log(gpu->instance, "failed to get DRM driver version");
         }
      }
      if (version)
         drmFreeVersion(version);
      close(fd);
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   gpu->fd = fd;

   struct stat st;
   if (stat(primary_path, &st) == 0) {
      gpu->has_primary = true;
      gpu->primary_major = major(st.st_rdev);
      gpu->primary_minor = minor(st.st_rdev);
   } else {
      gpu->has_primary = false;
      gpu->primary_major = 0;
      gpu->primary_minor = 0;
   }
   stat(node_path, &st);
   gpu->render_major = major(st.st_rdev);
   gpu->render_minor = minor(st.st_rdev);

   gpu->bustype = dev->bustype;
   if (dev->bustype == DRM_BUS_PCI)
      gpu->pci_bus_info = *dev->businfo.pci;

   drmFreeVersion(version);

   if (VN_DEBUG(INIT))
      vn_log(gpu->instance, "using DRM device %s", node_path);

   return VK_SUCCESS;
}

static VkResult
virtgpu_open(struct virtgpu *gpu)
{
   drmDevicePtr devs[8];
   int count = drmGetDevices2(0, devs, ARRAY_SIZE(devs));
   if (count < 0) {
      if (VN_DEBUG(INIT))
         vn_log(gpu->instance, "failed to enumerate DRM devices");
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   VkResult result = VK_ERROR_INITIALIZATION_FAILED;
   for (int i = 0; i < count; i++) {
      result = virtgpu_open_device(gpu, devs[i]);
      if (result == VK_SUCCESS)
         break;
   }

   drmFreeDevices(devs, count);

   return result;
}

static VkResult
virtgpu_init(struct virtgpu *gpu)
{
   util_sparse_array_init(&gpu->shmem_array, sizeof(struct virtgpu_shmem),
                          1024);
   util_sparse_array_init(&gpu->bo_array, sizeof(struct virtgpu_bo), 1024);

   mtx_init(&gpu->dma_buf_import_mutex, mtx_plain);

   VkResult result = virtgpu_open(gpu);
   if (result == VK_SUCCESS)
      result = virtgpu_init_params(gpu);
   if (result == VK_SUCCESS)
      result = virtgpu_init_capset(gpu);
   if (result == VK_SUCCESS)
      result = virtgpu_init_context(gpu);
   if (result != VK_SUCCESS)
      return result;

   virtgpu_init_shmem_blob_mem(gpu);

   vn_renderer_shmem_cache_init(&gpu->shmem_cache, &gpu->base,
                                virtgpu_shmem_destroy_now);

   virtgpu_init_renderer_info(gpu);

   gpu->base.ops.destroy = virtgpu_destroy;
   gpu->base.ops.submit = virtgpu_submit;
   gpu->base.ops.wait = virtgpu_wait;

   gpu->base.shmem_ops.create = virtgpu_shmem_create;
   gpu->base.shmem_ops.destroy = virtgpu_shmem_destroy;

   gpu->base.bo_ops.create_from_device_memory =
      virtgpu_bo_create_from_device_memory;
   gpu->base.bo_ops.create_from_dma_buf = virtgpu_bo_create_from_dma_buf;
   gpu->base.bo_ops.destroy = virtgpu_bo_destroy;
   gpu->base.bo_ops.export_dma_buf = virtgpu_bo_export_dma_buf;
   gpu->base.bo_ops.map = virtgpu_bo_map;
   gpu->base.bo_ops.flush = virtgpu_bo_flush;
   gpu->base.bo_ops.invalidate = virtgpu_bo_invalidate;

   gpu->base.sync_ops.create = virtgpu_sync_create;
   gpu->base.sync_ops.create_from_syncobj = virtgpu_sync_create_from_syncobj;
   gpu->base.sync_ops.destroy = virtgpu_sync_destroy;
   gpu->base.sync_ops.export_syncobj = virtgpu_sync_export_syncobj;
   gpu->base.sync_ops.reset = virtgpu_sync_reset;
   gpu->base.sync_ops.read = virtgpu_sync_read;
   gpu->base.sync_ops.write = virtgpu_sync_write;

   return VK_SUCCESS;
}

VkResult
vn_renderer_create_virtgpu(struct vn_instance *instance,
                           const VkAllocationCallbacks *alloc,
                           struct vn_renderer **renderer)
{
   struct virtgpu *gpu = vk_zalloc(alloc, sizeof(*gpu), VN_DEFAULT_ALIGN,
                                   VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!gpu)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   gpu->instance = instance;
   gpu->fd = -1;

   VkResult result = virtgpu_init(gpu);
   if (result != VK_SUCCESS) {
      virtgpu_destroy(&gpu->base, alloc);
      return result;
   }

   *renderer = &gpu->base;

   return VK_SUCCESS;
}
