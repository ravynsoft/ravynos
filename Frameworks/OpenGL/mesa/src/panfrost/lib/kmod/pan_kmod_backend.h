/*
 * Copyright © 2023 Collabora, Ltd.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "util/log.h"

#include "pan_kmod.h"

static inline void
pan_kmod_dev_init(struct pan_kmod_dev *dev, int fd, uint32_t flags,
                  drmVersionPtr version, const struct pan_kmod_ops *ops,
                  const struct pan_kmod_allocator *allocator)
{
   simple_mtx_init(&dev->handle_to_bo.lock, mtx_plain);
   util_sparse_array_init(&dev->handle_to_bo.array,
                          sizeof(struct pan_kmod_bo *), 512);
   dev->driver.version.major = version->version_major;
   dev->driver.version.minor = version->version_minor;
   dev->fd = fd;
   dev->flags = flags;
   dev->ops = ops;
   dev->allocator = allocator;
}

static inline void
pan_kmod_dev_cleanup(struct pan_kmod_dev *dev)
{
   if (dev->flags & PAN_KMOD_DEV_FLAG_OWNS_FD)
      close(dev->fd);

   util_sparse_array_finish(&dev->handle_to_bo.array);
   simple_mtx_destroy(&dev->handle_to_bo.lock);
}

static inline void *
pan_kmod_alloc(const struct pan_kmod_allocator *allocator, size_t size)
{
   return allocator->zalloc(allocator, size, false);
}

static inline void *
pan_kmod_alloc_transient(const struct pan_kmod_allocator *allocator,
                         size_t size)
{
   return allocator->zalloc(allocator, size, true);
}

static inline void
pan_kmod_free(const struct pan_kmod_allocator *allocator, void *data)
{
   return allocator->free(allocator, data);
}

static inline void *
pan_kmod_dev_alloc(struct pan_kmod_dev *dev, size_t size)
{
   return pan_kmod_alloc(dev->allocator, size);
}

static inline void *
pan_kmod_dev_alloc_transient(struct pan_kmod_dev *dev, size_t size)
{
   return pan_kmod_alloc_transient(dev->allocator, size);
}

static inline void
pan_kmod_dev_free(const struct pan_kmod_dev *dev, void *data)
{
   return pan_kmod_free(dev->allocator, data);
}

static inline void
pan_kmod_bo_init(struct pan_kmod_bo *bo, struct pan_kmod_dev *dev,
                 struct pan_kmod_vm *exclusive_vm, size_t size, uint32_t flags,
                 uint32_t handle)
{
   bo->dev = dev;
   bo->exclusive_vm = exclusive_vm;
   bo->size = size;
   bo->flags = flags;
   bo->handle = handle;
   p_atomic_set(&bo->refcnt, 1);
}

static inline void
pan_kmod_vm_init(struct pan_kmod_vm *vm, struct pan_kmod_dev *dev,
                 uint32_t handle, uint32_t flags)
{
   vm->dev = dev;
   vm->handle = handle;
   vm->flags = flags;
}

static inline int
pank_kmod_vm_op_check(struct pan_kmod_vm *vm, enum pan_kmod_vm_op_mode mode,
                      struct pan_kmod_vm_op *op)
{
   /* We should only have sync operations on an async VM bind request. */
   if (mode != PAN_KMOD_VM_OP_MODE_ASYNC && op->syncs.count) {
      mesa_loge("only PAN_KMOD_VM_OP_MODE_ASYNC can be passed sync operations");
      return -1;
   }

   /* Make sure the PAN_KMOD_VM_FLAG_AUTO_VA and VA passed to the op match. */
   if (op->type == PAN_KMOD_VM_OP_TYPE_MAP &&
       !!(vm->flags & PAN_KMOD_VM_FLAG_AUTO_VA) !=
          (op->va.start == PAN_KMOD_VM_MAP_AUTO_VA)) {
      mesa_loge("op->va.start and vm->flags don't match");
      return -1;
   }

   return 0;
}
