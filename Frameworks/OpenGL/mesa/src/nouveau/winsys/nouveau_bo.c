#include "nouveau_bo.h"

#include "drm-uapi/nouveau_drm.h"
#include "util/hash_table.h"
#include "util/u_math.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <xf86drm.h>

static void
bo_bind(struct nouveau_ws_device *dev,
        uint32_t handle, uint64_t addr,
        uint64_t range, uint64_t bo_offset,
        uint32_t flags)
{
   int ret;

   struct drm_nouveau_vm_bind_op newbindop = {
      .op = DRM_NOUVEAU_VM_BIND_OP_MAP,
      .handle = handle,
      .addr = addr,
      .range = range,
      .bo_offset = bo_offset,
      .flags = flags,
   };
   struct drm_nouveau_vm_bind vmbind = {
      .op_count = 1,
      .op_ptr = (uint64_t)(uintptr_t)(void *)&newbindop,
   };
   ret = drmCommandWriteRead(dev->fd, DRM_NOUVEAU_VM_BIND, &vmbind, sizeof(vmbind));
   if (ret)
      fprintf(stderr, "vm bind failed %d\n", errno);
   assert(ret == 0);
}

static void
bo_unbind(struct nouveau_ws_device *dev,
          uint64_t offset, uint64_t range,
          uint32_t flags)
{
   struct drm_nouveau_vm_bind_op newbindop = {
      .op = DRM_NOUVEAU_VM_BIND_OP_UNMAP,
      .addr = offset,
      .range = range,
      .flags = flags,
   };
   struct drm_nouveau_vm_bind vmbind = {
      .op_count = 1,
      .op_ptr = (uint64_t)(uintptr_t)(void *)&newbindop,
   };
   ASSERTED int ret = drmCommandWriteRead(dev->fd, DRM_NOUVEAU_VM_BIND, &vmbind, sizeof(vmbind));
   assert(ret == 0);
}

uint64_t
nouveau_ws_alloc_vma(struct nouveau_ws_device *dev,
                     uint64_t req_addr, uint64_t size, uint64_t align,
                     bool bda_capture_replay,
                     bool sparse_resident)
{
   assert(dev->has_vm_bind);

   uint64_t offset;
   simple_mtx_lock(&dev->vma_mutex);
   if (bda_capture_replay) {
      if (req_addr != 0) {
         bool found = util_vma_heap_alloc_addr(&dev->bda_heap, req_addr, size);
         offset = found ? req_addr : 0;
      } else {
         offset = util_vma_heap_alloc(&dev->bda_heap, size, align);
      }
   } else {
      offset = util_vma_heap_alloc(&dev->vma_heap, size, align);
   }
   simple_mtx_unlock(&dev->vma_mutex);

   if (offset == 0) {
      if (dev->debug_flags & NVK_DEBUG_VM) {
         fprintf(stderr, "alloc vma FAILED: %" PRIx64 " sparse: %d\n",
                 size, sparse_resident);
      }
      return 0;
   }

   if (dev->debug_flags & NVK_DEBUG_VM)
      fprintf(stderr, "alloc vma %" PRIx64 " %" PRIx64 " sparse: %d\n",
              offset, size, sparse_resident);

   if (sparse_resident)
      bo_bind(dev, 0, offset, size, 0, DRM_NOUVEAU_VM_BIND_SPARSE);

   return offset;
}

void
nouveau_ws_free_vma(struct nouveau_ws_device *dev,
                    uint64_t offset, uint64_t size,
                    bool bda_capture_replay,
                    bool sparse_resident)
{
   assert(dev->has_vm_bind);

   if (dev->debug_flags & NVK_DEBUG_VM)
      fprintf(stderr, "free vma %" PRIx64 " %" PRIx64 "\n",
              offset, size);

   if (sparse_resident)
      bo_unbind(dev, offset, size, DRM_NOUVEAU_VM_BIND_SPARSE);

   simple_mtx_lock(&dev->vma_mutex);
   if (bda_capture_replay) {
      util_vma_heap_free(&dev->bda_heap, offset, size);
   } else {
      util_vma_heap_free(&dev->vma_heap, offset, size);
   }
   simple_mtx_unlock(&dev->vma_mutex);
}

void
nouveau_ws_bo_unbind_vma(struct nouveau_ws_device *dev,
                         uint64_t offset, uint64_t range)
{
   assert(dev->has_vm_bind);

   if (dev->debug_flags & NVK_DEBUG_VM)
      fprintf(stderr, "unbind vma %" PRIx64 " %" PRIx64 "\n",
              offset, range);
   bo_unbind(dev, offset, range, 0);
}

void
nouveau_ws_bo_bind_vma(struct nouveau_ws_device *dev,
                       struct nouveau_ws_bo *bo,
                       uint64_t addr,
                       uint64_t range,
                       uint64_t bo_offset,
                       uint32_t pte_kind)
{
   assert(dev->has_vm_bind);

   if (dev->debug_flags & NVK_DEBUG_VM)
      fprintf(stderr, "bind vma %x %" PRIx64 " %" PRIx64 " %" PRIx64 " %d\n",
              bo->handle, addr, range, bo_offset, pte_kind);
   bo_bind(dev, bo->handle, addr, range, bo_offset, pte_kind);
}

struct nouveau_ws_bo *
nouveau_ws_bo_new_mapped(struct nouveau_ws_device *dev,
                         uint64_t size, uint64_t align,
                         enum nouveau_ws_bo_flags flags,
                         enum nouveau_ws_bo_map_flags map_flags,
                         void **map_out)
{
   struct nouveau_ws_bo *bo = nouveau_ws_bo_new(dev, size, align,
                                                flags | NOUVEAU_WS_BO_MAP);
   if (!bo)
      return NULL;

   void *map = nouveau_ws_bo_map(bo, map_flags);
   if (map == NULL) {
      nouveau_ws_bo_destroy(bo);
      return NULL;
   }

   *map_out = map;
   return bo;
}

static struct nouveau_ws_bo *
nouveau_ws_bo_new_locked(struct nouveau_ws_device *dev,
                         uint64_t size, uint64_t align,
                         enum nouveau_ws_bo_flags flags)
{
   struct drm_nouveau_gem_new req = {};

   /* if the caller doesn't care, use the GPU page size */
   if (align == 0)
      align = 0x1000;

   /* Align the size */
   size = align64(size, align);

   req.info.domain = 0;

   /* TODO:
    *
    * VRAM maps on Kepler appear to be broken and we don't really know why.
    * My NVIDIA contact doesn't remember them not working so they probably
    * should but they don't today.  Force everything that may be mapped to
    * use GART for now.
    */
   if (flags & NOUVEAU_WS_BO_GART)
      req.info.domain |= NOUVEAU_GEM_DOMAIN_GART;
   else if (dev->info.chipset < 0x110 && (flags & NOUVEAU_WS_BO_MAP))
      req.info.domain |= NOUVEAU_GEM_DOMAIN_GART;
   else
      req.info.domain |= dev->local_mem_domain;

   if (flags & NOUVEAU_WS_BO_MAP)
      req.info.domain |= NOUVEAU_GEM_DOMAIN_MAPPABLE;

   if (flags & NOUVEAU_WS_BO_NO_SHARE)
      req.info.domain |= NOUVEAU_GEM_DOMAIN_NO_SHARE;

   req.info.size = size;
   req.align = align;

   int ret = drmCommandWriteRead(dev->fd, DRM_NOUVEAU_GEM_NEW, &req, sizeof(req));
   if (ret != 0)
      return NULL;

   struct nouveau_ws_bo *bo = CALLOC_STRUCT(nouveau_ws_bo);
   bo->size = size;
   bo->align = align;
   bo->offset = -1ULL;
   bo->handle = req.info.handle;
   bo->map_handle = req.info.map_handle;
   bo->dev = dev;
   bo->flags = flags;
   bo->refcnt = 1;

   if (dev->has_vm_bind) {
      bo->offset = nouveau_ws_alloc_vma(dev, 0, bo->size, align, false, false);
      if (bo->offset == 0)
         goto fail_gem_new;

      nouveau_ws_bo_bind_vma(dev, bo, bo->offset, bo->size, 0, 0);
   }

   _mesa_hash_table_insert(dev->bos, (void *)(uintptr_t)bo->handle, bo);

   return bo;

fail_gem_new:
   drmCloseBufferHandle(dev->fd, req.info.handle);
   FREE(bo);

   return NULL;
}

struct nouveau_ws_bo *
nouveau_ws_bo_new(struct nouveau_ws_device *dev,
                  uint64_t size, uint64_t align,
                  enum nouveau_ws_bo_flags flags)
{
   struct nouveau_ws_bo *bo;

   simple_mtx_lock(&dev->bos_lock);
   bo = nouveau_ws_bo_new_locked(dev, size, align, flags);
   simple_mtx_unlock(&dev->bos_lock);

   return bo;
}

static struct nouveau_ws_bo *
nouveau_ws_bo_from_dma_buf_locked(struct nouveau_ws_device *dev, int fd)
{
   uint32_t handle;
   int ret = drmPrimeFDToHandle(dev->fd, fd, &handle);
   if (ret != 0)
      return NULL;

   struct hash_entry *entry =
      _mesa_hash_table_search(dev->bos, (void *)(uintptr_t)handle);
   if (entry != NULL)
      return entry->data;

   /*
    * If we got here, no BO exists for the retrieved handle. If we error
    * after this point, we need to close the handle.
    */

   struct drm_nouveau_gem_info info = {
      .handle = handle
   };
   ret = drmCommandWriteRead(dev->fd, DRM_NOUVEAU_GEM_INFO,
                             &info, sizeof(info));
   if (ret != 0)
      goto fail_fd_to_handle;

   enum nouveau_ws_bo_flags flags = 0;
   if (info.domain & NOUVEAU_GEM_DOMAIN_GART)
      flags |= NOUVEAU_WS_BO_GART;
   if (info.map_handle)
      flags |= NOUVEAU_WS_BO_MAP;

   struct nouveau_ws_bo *bo = CALLOC_STRUCT(nouveau_ws_bo);
   bo->size = info.size;
   bo->offset = info.offset;
   bo->handle = info.handle;
   bo->map_handle = info.map_handle;
   bo->dev = dev;
   bo->flags = flags;
   bo->refcnt = 1;

   uint64_t align = (1ULL << 12);
   if (info.domain & NOUVEAU_GEM_DOMAIN_VRAM)
      align = (1ULL << 16);

   assert(bo->size == align64(bo->size, align));

   bo->offset = nouveau_ws_alloc_vma(dev, 0, bo->size, align, false, false);
   if (bo->offset == 0)
      goto fail_calloc;

   nouveau_ws_bo_bind_vma(dev, bo, bo->offset, bo->size, 0, 0);
   _mesa_hash_table_insert(dev->bos, (void *)(uintptr_t)handle, bo);

   return bo;

fail_calloc:
   FREE(bo);
fail_fd_to_handle:
   drmCloseBufferHandle(dev->fd, handle);

   return NULL;
}

struct nouveau_ws_bo *
nouveau_ws_bo_from_dma_buf(struct nouveau_ws_device *dev, int fd)
{
   struct nouveau_ws_bo *bo;

   simple_mtx_lock(&dev->bos_lock);
   bo = nouveau_ws_bo_from_dma_buf_locked(dev, fd);
   simple_mtx_unlock(&dev->bos_lock);

   return bo;
}

void
nouveau_ws_bo_destroy(struct nouveau_ws_bo *bo)
{
   if (--bo->refcnt)
      return;

   struct nouveau_ws_device *dev = bo->dev;

   simple_mtx_lock(&dev->bos_lock);

   _mesa_hash_table_remove_key(dev->bos, (void *)(uintptr_t)bo->handle);

   if (dev->has_vm_bind) {
      nouveau_ws_bo_unbind_vma(bo->dev, bo->offset, bo->size);
      nouveau_ws_free_vma(bo->dev, bo->offset, bo->size, false, false);
   }

   drmCloseBufferHandle(bo->dev->fd, bo->handle);
   FREE(bo);

   simple_mtx_unlock(&dev->bos_lock);
}

void *
nouveau_ws_bo_map(struct nouveau_ws_bo *bo, enum nouveau_ws_bo_map_flags flags)
{
   size_t prot = 0;

   if (flags & NOUVEAU_WS_BO_RD)
      prot |= PROT_READ;
   if (flags & NOUVEAU_WS_BO_WR)
      prot |= PROT_WRITE;

   void *res = mmap(NULL, bo->size, prot, MAP_SHARED, bo->dev->fd, bo->map_handle);
   if (res == MAP_FAILED)
      return NULL;

   return res;
}

bool
nouveau_ws_bo_wait(struct nouveau_ws_bo *bo, enum nouveau_ws_bo_map_flags flags)
{
   struct drm_nouveau_gem_cpu_prep req = {};

   req.handle = bo->handle;
   if (flags & NOUVEAU_WS_BO_WR)
      req.flags |= NOUVEAU_GEM_CPU_PREP_WRITE;

   return !drmCommandWrite(bo->dev->fd, DRM_NOUVEAU_GEM_CPU_PREP, &req, sizeof(req));
}

int
nouveau_ws_bo_dma_buf(struct nouveau_ws_bo *bo, int *fd)
{
   return drmPrimeHandleToFD(bo->dev->fd, bo->handle, DRM_CLOEXEC, fd);
}
