/*
 * Copyright © 2023 Collabora, Ltd.
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <xf86drm.h>

#include "util/macros.h"
#include "pan_kmod.h"

extern const struct pan_kmod_ops panfrost_kmod_ops;

static const struct {
   const char *name;
   const struct pan_kmod_ops *ops;
} drivers[] = {
   {
      "panfrost",
      &panfrost_kmod_ops,
   },
};

static void *
default_zalloc(const struct pan_kmod_allocator *allocator, size_t size,
               UNUSED bool transient)
{
   return rzalloc_size(allocator, size);
}

static void
default_free(const struct pan_kmod_allocator *allocator, void *data)
{
   return ralloc_free(data);
}

static const struct pan_kmod_allocator *
create_default_allocator(void)
{
   struct pan_kmod_allocator *allocator =
      rzalloc(NULL, struct pan_kmod_allocator);

   if (allocator) {
      allocator->zalloc = default_zalloc;
      allocator->free = default_free;
   }

   return allocator;
}

struct pan_kmod_dev *
pan_kmod_dev_create(int fd, uint32_t flags,
                    const struct pan_kmod_allocator *allocator)
{
   drmVersionPtr version = drmGetVersion(fd);
   struct pan_kmod_dev *dev = NULL;

   if (!version)
      return NULL;

   if (!allocator) {
      allocator = create_default_allocator();
      if (!allocator)
         goto out_free_version;
   }

   for (unsigned i = 0; i < ARRAY_SIZE(drivers); i++) {
      if (!strcmp(drivers[i].name, version->name)) {
         const struct pan_kmod_ops *ops = drivers[i].ops;

         dev = ops->dev_create(fd, flags, version, allocator);
         if (dev)
            goto out_free_version;

         break;
      }
   }

   if (allocator->zalloc == default_zalloc)
      ralloc_free((void *)allocator);

out_free_version:
   drmFreeVersion(version);
   return dev;
}

void
pan_kmod_dev_destroy(struct pan_kmod_dev *dev)
{
   const struct pan_kmod_allocator *allocator = dev->allocator;

   dev->ops->dev_destroy(dev);

   if (allocator->zalloc == default_zalloc)
      ralloc_free((void *)allocator);
}

struct pan_kmod_bo *
pan_kmod_bo_alloc(struct pan_kmod_dev *dev, struct pan_kmod_vm *exclusive_vm,
                  size_t size, uint32_t flags)
{
   struct pan_kmod_bo *bo;

   bo = dev->ops->bo_alloc(dev, exclusive_vm, size, flags);
   if (!bo)
      return NULL;

   /* We intentionally don't take the lock when filling the sparse array,
    * because we just created the BO, and haven't exported it yet, so
    * there's no risk of imports racing with our BO insertion.
    */
   struct pan_kmod_bo **slot =
      util_sparse_array_get(&dev->handle_to_bo.array, bo->handle);

   if (!slot) {
      mesa_loge("failed to allocate slot in the handle_to_bo array");
      bo->dev->ops->bo_free(bo);
      return NULL;
   }

   assert(*slot == NULL);
   *slot = bo;
   return bo;
}

void
pan_kmod_bo_put(struct pan_kmod_bo *bo)
{
   if (!bo)
      return;

   int32_t refcnt = p_atomic_dec_return(&bo->refcnt);

   assert(refcnt >= 0);

   if (refcnt)
      return;

   struct pan_kmod_dev *dev = bo->dev;

   simple_mtx_lock(&dev->handle_to_bo.lock);

   /* If some import took a ref on this BO while we were trying to acquire the
    * lock, skip the destruction.
    */
   if (!p_atomic_read(&bo->refcnt)) {
      struct pan_kmod_bo **slot = (struct pan_kmod_bo **)util_sparse_array_get(
         &dev->handle_to_bo.array, bo->handle);

      assert(slot);
      *slot = NULL;
      bo->dev->ops->bo_free(bo);
   }

   simple_mtx_unlock(&dev->handle_to_bo.lock);
}

static bool
pan_kmod_bo_check_import_flags(struct pan_kmod_bo *bo, uint32_t flags)
{
   uint32_t mask = PAN_KMOD_BO_FLAG_EXECUTABLE |
                   PAN_KMOD_BO_FLAG_ALLOC_ON_FAULT | PAN_KMOD_BO_FLAG_NO_MMAP |
                   PAN_KMOD_BO_FLAG_GPU_UNCACHED;

   /* If the BO exists, make sure the import flags match the original flags. */
   return (bo->flags & mask) == (flags & mask);
}

struct pan_kmod_bo *
pan_kmod_bo_import(struct pan_kmod_dev *dev, int fd, uint32_t flags)
{
   struct pan_kmod_bo *bo = NULL;
   struct pan_kmod_bo **slot;

   simple_mtx_lock(&dev->handle_to_bo.lock);

   uint32_t handle;
   int ret = drmPrimeFDToHandle(dev->fd, fd, &handle);
   if (ret)
      goto err_unlock;

   slot = util_sparse_array_get(&dev->handle_to_bo.array, handle);
   if (!slot)
      goto err_close_handle;

   if (*slot) {
      if (!pan_kmod_bo_check_import_flags(*slot, flags)) {
         mesa_loge("invalid import flags");
         goto err_unlock;
      }

      bo = *slot;

      p_atomic_inc(&bo->refcnt);
   } else {
      size_t size = lseek(fd, 0, SEEK_END);
      if (size == 0 || size == (size_t)-1) {
         mesa_loge("invalid dmabuf size");
         goto err_close_handle;
      }

      bo = dev->ops->bo_import(dev, handle, size, flags);
      if (!bo)
         goto err_close_handle;

      *slot = bo;
   }

   assert(p_atomic_read(&bo->refcnt) > 0);

   simple_mtx_unlock(&dev->handle_to_bo.lock);

   return bo;

err_close_handle:
   drmCloseBufferHandle(dev->fd, handle);

err_unlock:
   simple_mtx_unlock(&dev->handle_to_bo.lock);

   return NULL;
}

