/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VN_RENDERER_H
#define VN_RENDERER_H

#include "vn_common.h"

struct vn_renderer_shmem {
   struct vn_refcount refcount;

   uint32_t res_id;
   size_t mmap_size; /* for internal use only (i.e., munmap) */
   void *mmap_ptr;

   struct list_head cache_head;
   int64_t cache_timestamp;
};

struct vn_renderer_bo {
   struct vn_refcount refcount;

   uint32_t res_id;
   /* for internal use only */
   size_t mmap_size;
   void *mmap_ptr;
};

/*
 * A sync consists of a uint64_t counter.  The counter can be updated by CPU
 * or by GPU.  It can also be waited on by CPU or by GPU until it reaches
 * certain values.
 *
 * This models after timeline VkSemaphore rather than timeline drm_syncobj.
 * The main difference is that drm_syncobj can have unsignaled value 0.
 */
struct vn_renderer_sync {
   uint32_t sync_id;
};

struct vn_renderer_info {
   struct {
      bool has_primary;
      int primary_major;
      int primary_minor;
      bool has_render;
      int render_major;
      int render_minor;
   } drm;

   struct {
      uint16_t vendor_id;
      uint16_t device_id;

      bool has_bus_info;
      uint16_t domain;
      uint8_t bus;
      uint8_t device;
      uint8_t function;
   } pci;

   bool has_dma_buf_import;
   bool has_external_sync;
   bool has_implicit_fencing;
   bool has_guest_vram;

   uint32_t max_timeline_count;

   /* hw capset */
   uint32_t wire_format_version;
   uint32_t vk_xml_version;
   uint32_t vk_ext_command_serialization_spec_version;
   uint32_t vk_mesa_venus_protocol_spec_version;
   uint32_t supports_blob_id_0;
   /* combined mask for vk_extension_mask1, 2,..., N */
   uint32_t vk_extension_mask[32];
   uint32_t allow_vk_wait_syncs;
   uint32_t supports_multiple_timelines;
};

struct vn_renderer_submit_batch {
   const void *cs_data;
   size_t cs_size;

   /*
    * Submit cs to the timeline identified by ring_idx. A timeline is
    * typically associated with a physical VkQueue and bound to the ring_idx
    * during VkQueue creation. After execution completes on the VkQueue, the
    * timeline sync point is signaled.
    *
    * ring_idx 0 is reserved for the context-specific CPU timeline. sync
    * points on the CPU timeline are signaled immediately after command
    * processing by the renderer.
    */
   uint32_t ring_idx;

   /* syncs to update when the timeline is signaled */
   struct vn_renderer_sync *const *syncs;
   /* TODO allow NULL when syncs are all binary? */
   const uint64_t *sync_values;
   uint32_t sync_count;
};

struct vn_renderer_submit {
   /* BOs to pin and to fence implicitly
    *
    * TODO track all bos and automatically pin them.  We don't do it yet
    * because each vn_command_buffer owns a bo.  We can probably make do by
    * returning the bos to a bo cache and exclude bo cache from pinning.
    */
   struct vn_renderer_bo *const *bos;
   uint32_t bo_count;

   const struct vn_renderer_submit_batch *batches;
   uint32_t batch_count;
};

struct vn_renderer_wait {
   bool wait_any;
   uint64_t timeout;

   struct vn_renderer_sync *const *syncs;
   /* TODO allow NULL when syncs are all binary? */
   const uint64_t *sync_values;
   uint32_t sync_count;
};

struct vn_renderer_ops {
   void (*destroy)(struct vn_renderer *renderer,
                   const VkAllocationCallbacks *alloc);

   VkResult (*submit)(struct vn_renderer *renderer,
                      const struct vn_renderer_submit *submit);

   /*
    * On success, returns VK_SUCCESS or VK_TIMEOUT.  On failure, returns
    * VK_ERROR_DEVICE_LOST or out of device/host memory.
    */
   VkResult (*wait)(struct vn_renderer *renderer,
                    const struct vn_renderer_wait *wait);
};

struct vn_renderer_shmem_ops {
   struct vn_renderer_shmem *(*create)(struct vn_renderer *renderer,
                                       size_t size);
   void (*destroy)(struct vn_renderer *renderer,
                   struct vn_renderer_shmem *shmem);
};

struct vn_renderer_bo_ops {
   VkResult (*create_from_device_memory)(
      struct vn_renderer *renderer,
      VkDeviceSize size,
      vn_object_id mem_id,
      VkMemoryPropertyFlags flags,
      VkExternalMemoryHandleTypeFlags external_handles,
      struct vn_renderer_bo **out_bo);

   VkResult (*create_from_dma_buf)(struct vn_renderer *renderer,
                                   VkDeviceSize size,
                                   int fd,
                                   VkMemoryPropertyFlags flags,
                                   struct vn_renderer_bo **out_bo);

   bool (*destroy)(struct vn_renderer *renderer, struct vn_renderer_bo *bo);

   int (*export_dma_buf)(struct vn_renderer *renderer,
                         struct vn_renderer_bo *bo);

   /* map is not thread-safe */
   void *(*map)(struct vn_renderer *renderer, struct vn_renderer_bo *bo);

   void (*flush)(struct vn_renderer *renderer,
                 struct vn_renderer_bo *bo,
                 VkDeviceSize offset,
                 VkDeviceSize size);
   void (*invalidate)(struct vn_renderer *renderer,
                      struct vn_renderer_bo *bo,
                      VkDeviceSize offset,
                      VkDeviceSize size);
};

enum vn_renderer_sync_flags {
   VN_RENDERER_SYNC_SHAREABLE = 1u << 0,
   VN_RENDERER_SYNC_BINARY = 1u << 1,
};

struct vn_renderer_sync_ops {
   VkResult (*create)(struct vn_renderer *renderer,
                      uint64_t initial_val,
                      uint32_t flags,
                      struct vn_renderer_sync **out_sync);

   VkResult (*create_from_syncobj)(struct vn_renderer *renderer,
                                   int fd,
                                   bool sync_file,
                                   struct vn_renderer_sync **out_sync);
   void (*destroy)(struct vn_renderer *renderer,
                   struct vn_renderer_sync *sync);

   int (*export_syncobj)(struct vn_renderer *renderer,
                         struct vn_renderer_sync *sync,
                         bool sync_file);

   /* reset the counter */
   VkResult (*reset)(struct vn_renderer *renderer,
                     struct vn_renderer_sync *sync,
                     uint64_t initial_val);

   /* read the current value from the counter */
   VkResult (*read)(struct vn_renderer *renderer,
                    struct vn_renderer_sync *sync,
                    uint64_t *val);

   /* write a new value (larger than the current one) to the counter */
   VkResult (*write)(struct vn_renderer *renderer,
                     struct vn_renderer_sync *sync,
                     uint64_t val);
};

struct vn_renderer {
   struct vn_renderer_info info;
   struct vn_renderer_ops ops;
   struct vn_renderer_shmem_ops shmem_ops;
   struct vn_renderer_bo_ops bo_ops;
   struct vn_renderer_sync_ops sync_ops;
};

VkResult
vn_renderer_create_virtgpu(struct vn_instance *instance,
                           const VkAllocationCallbacks *alloc,
                           struct vn_renderer **renderer);

VkResult
vn_renderer_create_vtest(struct vn_instance *instance,
                         const VkAllocationCallbacks *alloc,
                         struct vn_renderer **renderer);

static inline VkResult
vn_renderer_create(struct vn_instance *instance,
                   const VkAllocationCallbacks *alloc,
                   struct vn_renderer **renderer)
{
   if (VN_DEBUG(VTEST)) {
      VkResult result = vn_renderer_create_vtest(instance, alloc, renderer);
      if (result == VK_SUCCESS)
         return VK_SUCCESS;
   }

   return vn_renderer_create_virtgpu(instance, alloc, renderer);
}

static inline void
vn_renderer_destroy(struct vn_renderer *renderer,
                    const VkAllocationCallbacks *alloc)
{
   renderer->ops.destroy(renderer, alloc);
}

static inline VkResult
vn_renderer_submit(struct vn_renderer *renderer,
                   const struct vn_renderer_submit *submit)
{
   return renderer->ops.submit(renderer, submit);
}

static inline VkResult
vn_renderer_wait(struct vn_renderer *renderer,
                 const struct vn_renderer_wait *wait)
{
   return renderer->ops.wait(renderer, wait);
}

static inline struct vn_renderer_shmem *
vn_renderer_shmem_create(struct vn_renderer *renderer, size_t size)
{
   VN_TRACE_FUNC();
   struct vn_renderer_shmem *shmem =
      renderer->shmem_ops.create(renderer, size);
   if (shmem) {
      assert(vn_refcount_is_valid(&shmem->refcount));
      assert(shmem->res_id);
      assert(shmem->mmap_size >= size);
      assert(shmem->mmap_ptr);
   }

   return shmem;
}

static inline struct vn_renderer_shmem *
vn_renderer_shmem_ref(struct vn_renderer *renderer,
                      struct vn_renderer_shmem *shmem)
{
   vn_refcount_inc(&shmem->refcount);
   return shmem;
}

static inline void
vn_renderer_shmem_unref(struct vn_renderer *renderer,
                        struct vn_renderer_shmem *shmem)
{
   if (vn_refcount_dec(&shmem->refcount))
      renderer->shmem_ops.destroy(renderer, shmem);
}

static inline VkResult
vn_renderer_bo_create_from_device_memory(
   struct vn_renderer *renderer,
   VkDeviceSize size,
   vn_object_id mem_id,
   VkMemoryPropertyFlags flags,
   VkExternalMemoryHandleTypeFlags external_handles,
   struct vn_renderer_bo **out_bo)
{
   struct vn_renderer_bo *bo;
   VkResult result = renderer->bo_ops.create_from_device_memory(
      renderer, size, mem_id, flags, external_handles, &bo);
   if (result != VK_SUCCESS)
      return result;

   assert(vn_refcount_is_valid(&bo->refcount));
   assert(bo->res_id);
   assert(!bo->mmap_size || bo->mmap_size >= size);

   *out_bo = bo;
   return VK_SUCCESS;
}

static inline VkResult
vn_renderer_bo_create_from_dma_buf(struct vn_renderer *renderer,
                                   VkDeviceSize size,
                                   int fd,
                                   VkMemoryPropertyFlags flags,
                                   struct vn_renderer_bo **out_bo)
{
   struct vn_renderer_bo *bo;
   VkResult result =
      renderer->bo_ops.create_from_dma_buf(renderer, size, fd, flags, &bo);
   if (result != VK_SUCCESS)
      return result;

   assert(vn_refcount_is_valid(&bo->refcount));
   assert(bo->res_id);
   assert(!bo->mmap_size || bo->mmap_size >= size);

   *out_bo = bo;
   return VK_SUCCESS;
}

static inline struct vn_renderer_bo *
vn_renderer_bo_ref(struct vn_renderer *renderer, struct vn_renderer_bo *bo)
{
   vn_refcount_inc(&bo->refcount);
   return bo;
}

static inline bool
vn_renderer_bo_unref(struct vn_renderer *renderer, struct vn_renderer_bo *bo)
{
   if (vn_refcount_dec(&bo->refcount))
      return renderer->bo_ops.destroy(renderer, bo);
   return false;
}

static inline int
vn_renderer_bo_export_dma_buf(struct vn_renderer *renderer,
                              struct vn_renderer_bo *bo)
{
   return renderer->bo_ops.export_dma_buf(renderer, bo);
}

static inline void *
vn_renderer_bo_map(struct vn_renderer *renderer, struct vn_renderer_bo *bo)
{
   return renderer->bo_ops.map(renderer, bo);
}

static inline void
vn_renderer_bo_flush(struct vn_renderer *renderer,
                     struct vn_renderer_bo *bo,
                     VkDeviceSize offset,
                     VkDeviceSize end)
{
   renderer->bo_ops.flush(renderer, bo, offset, end);
}

static inline void
vn_renderer_bo_invalidate(struct vn_renderer *renderer,
                          struct vn_renderer_bo *bo,
                          VkDeviceSize offset,
                          VkDeviceSize size)
{
   renderer->bo_ops.invalidate(renderer, bo, offset, size);
}

static inline VkResult
vn_renderer_sync_create(struct vn_renderer *renderer,
                        uint64_t initial_val,
                        uint32_t flags,
                        struct vn_renderer_sync **out_sync)
{
   return renderer->sync_ops.create(renderer, initial_val, flags, out_sync);
}

static inline VkResult
vn_renderer_sync_create_from_syncobj(struct vn_renderer *renderer,
                                     int fd,
                                     bool sync_file,
                                     struct vn_renderer_sync **out_sync)
{
   return renderer->sync_ops.create_from_syncobj(renderer, fd, sync_file,
                                                 out_sync);
}

static inline void
vn_renderer_sync_destroy(struct vn_renderer *renderer,
                         struct vn_renderer_sync *sync)
{
   renderer->sync_ops.destroy(renderer, sync);
}

static inline int
vn_renderer_sync_export_syncobj(struct vn_renderer *renderer,
                                struct vn_renderer_sync *sync,
                                bool sync_file)
{
   return renderer->sync_ops.export_syncobj(renderer, sync, sync_file);
}

static inline VkResult
vn_renderer_sync_reset(struct vn_renderer *renderer,
                       struct vn_renderer_sync *sync,
                       uint64_t initial_val)
{
   return renderer->sync_ops.reset(renderer, sync, initial_val);
}

static inline VkResult
vn_renderer_sync_read(struct vn_renderer *renderer,
                      struct vn_renderer_sync *sync,
                      uint64_t *val)
{
   return renderer->sync_ops.read(renderer, sync, val);
}

static inline VkResult
vn_renderer_sync_write(struct vn_renderer *renderer,
                       struct vn_renderer_sync *sync,
                       uint64_t val)
{
   return renderer->sync_ops.write(renderer, sync, val);
}

#endif /* VN_RENDERER_H */
