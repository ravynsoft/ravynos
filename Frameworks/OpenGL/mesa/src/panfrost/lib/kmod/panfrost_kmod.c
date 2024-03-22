/*
 * Copyright © 2023 Collabora, Ltd.
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <xf86drm.h>

#include "util/hash_table.h"
#include "util/macros.h"
#include "util/simple_mtx.h"

#include "drm-uapi/panfrost_drm.h"

#include "pan_kmod_backend.h"

const struct pan_kmod_ops panfrost_kmod_ops;

struct panfrost_kmod_vm {
   struct pan_kmod_vm base;
};

struct panfrost_kmod_dev {
   struct pan_kmod_dev base;
   struct panfrost_kmod_vm *vm;
};

struct panfrost_kmod_bo {
   struct pan_kmod_bo base;

   /* This is actually the VA assigned to the BO at creation/import time.
    * We don't control it, it's automatically assigned by the kernel driver.
    */
   uint64_t offset;
};

static struct pan_kmod_dev *
panfrost_kmod_dev_create(int fd, uint32_t flags, drmVersionPtr version,
                         const struct pan_kmod_allocator *allocator)
{
   struct panfrost_kmod_dev *panfrost_dev =
      pan_kmod_alloc(allocator, sizeof(*panfrost_dev));
   if (!panfrost_dev) {
      mesa_loge("failed to allocate a panfrost_kmod_dev object");
      return NULL;
   }

   pan_kmod_dev_init(&panfrost_dev->base, fd, flags, version,
                     &panfrost_kmod_ops, allocator);
   return &panfrost_dev->base;
}

static void
panfrost_kmod_dev_destroy(struct pan_kmod_dev *dev)
{
   struct panfrost_kmod_dev *panfrost_dev =
      container_of(dev, struct panfrost_kmod_dev, base);

   pan_kmod_dev_cleanup(dev);
   pan_kmod_free(dev->allocator, panfrost_dev);
}

/* Abstraction over the raw drm_panfrost_get_param ioctl for fetching
 * information about devices.
 */
static __u64
panfrost_query_raw(int fd, enum drm_panfrost_param param, bool required,
                   unsigned default_value)
{
   struct drm_panfrost_get_param get_param = {};
   ASSERTED int ret;

   get_param.param = param;
   ret = drmIoctl(fd, DRM_IOCTL_PANFROST_GET_PARAM, &get_param);

   if (ret) {
      assert(!required);
      return default_value;
   }

   return get_param.value;
}

static void
panfrost_dev_query_props(const struct pan_kmod_dev *dev,
                         struct pan_kmod_dev_props *props)
{
   int fd = dev->fd;

   memset(props, 0, sizeof(*props));
   props->gpu_prod_id =
      panfrost_query_raw(fd, DRM_PANFROST_PARAM_GPU_PROD_ID, true, 0);
   props->gpu_revision =
      panfrost_query_raw(fd, DRM_PANFROST_PARAM_GPU_REVISION, true, 0);
   props->shader_present =
      panfrost_query_raw(fd, DRM_PANFROST_PARAM_SHADER_PRESENT, false, 0xffff);
   props->tiler_features =
      panfrost_query_raw(fd, DRM_PANFROST_PARAM_TILER_FEATURES, false, 0x809);
   props->mem_features =
      panfrost_query_raw(fd, DRM_PANFROST_PARAM_MEM_FEATURES, true, 0);
   props->mmu_features =
      panfrost_query_raw(fd, DRM_PANFROST_PARAM_MMU_FEATURES, false, 0);

   for (unsigned i = 0; i < ARRAY_SIZE(props->texture_features); i++) {
      /* If unspecified, assume ASTC/ETC only. Factory default for Juno, and
       * should exist on any Mali configuration. All hardware should report
       * these texture formats but the kernel might not be new enough. */
      static const uint32_t default_tex_features[4] = {
         (1 << 1) |     // ETC2 RGB8
            (1 << 2) |  // ETC2 R11 UNORM
            (1 << 3) |  // ETC2 RGBA8
            (1 << 4) |  // ETC2 RG11 UNORM
            (1 << 17) | // ETC2 R11 SNORM
            (1 << 18) | // ETC2 RG11 SNORM
            (1 << 19) | // ETC2 RGB8A1
            (1 << 20) | // ASTC 3D LDR
            (1 << 21) | // ASTC 3D HDR
            (1 << 22) | // ASTC 2D LDR
            (1 << 23),  // ASTC 2D HDR
         0,
         0,
         0,
      };

      props->texture_features[i] =
         panfrost_query_raw(fd, DRM_PANFROST_PARAM_TEXTURE_FEATURES0 + i, false,
                            default_tex_features[i]);
   }

   props->thread_tls_alloc =
      panfrost_query_raw(fd, DRM_PANFROST_PARAM_THREAD_TLS_ALLOC, false, 0);
   props->afbc_features =
      panfrost_query_raw(fd, DRM_PANFROST_PARAM_AFBC_FEATURES, false, 0);
}

static uint32_t
to_panfrost_bo_flags(struct pan_kmod_dev *dev, uint32_t flags)
{
   uint32_t panfrost_flags = 0;

   if (dev->driver.version.major > 1 || dev->driver.version.minor >= 1) {
      /* The alloc-on-fault feature is only used for the tiler HEAP object,
       * hence the name of the flag on panfrost.
       */
      if (flags & PAN_KMOD_BO_FLAG_ALLOC_ON_FAULT)
         panfrost_flags |= PANFROST_BO_HEAP;

      if (!(flags & PAN_KMOD_BO_FLAG_EXECUTABLE))
         panfrost_flags |= PANFROST_BO_NOEXEC;
   }

   return panfrost_flags;
}

static struct pan_kmod_bo *
panfrost_kmod_bo_alloc(struct pan_kmod_dev *dev,
                       struct pan_kmod_vm *exclusive_vm, size_t size,
                       uint32_t flags)
{
   /* We can't map GPU uncached. */
   if (flags & PAN_KMOD_BO_FLAG_GPU_UNCACHED)
      return NULL;

   struct panfrost_kmod_bo *bo = pan_kmod_dev_alloc(dev, sizeof(*bo));
   if (!bo)
      return NULL;

   struct drm_panfrost_create_bo req = {
      .size = size,
      .flags = to_panfrost_bo_flags(dev, flags),
   };

   int ret = drmIoctl(dev->fd, DRM_IOCTL_PANFROST_CREATE_BO, &req);
   if (ret) {
      mesa_loge("DRM_IOCTL_PANFROST_CREATE_BO failed (err=%d)", errno);
      goto err_free_bo;
   }

   pan_kmod_bo_init(&bo->base, dev, exclusive_vm, req.size, flags, req.handle);
   bo->offset = req.offset;
   return &bo->base;

err_free_bo:
   pan_kmod_dev_free(dev, bo);
   return NULL;
}

static void
panfrost_kmod_bo_free(struct pan_kmod_bo *bo)
{
   drmCloseBufferHandle(bo->dev->fd, bo->handle);
   pan_kmod_dev_free(bo->dev, bo);
}

static struct pan_kmod_bo *
panfrost_kmod_bo_import(struct pan_kmod_dev *dev, uint32_t handle, size_t size,
                        uint32_t flags)
{
   struct panfrost_kmod_bo *panfrost_bo =
      pan_kmod_dev_alloc(dev, sizeof(*panfrost_bo));
   if (!panfrost_bo) {
      mesa_loge("failed to allocate a panfrost_kmod_bo object");
      return NULL;
   }

   struct drm_panfrost_get_bo_offset get_bo_offset = {.handle = handle, 0};
   int ret =
      drmIoctl(dev->fd, DRM_IOCTL_PANFROST_GET_BO_OFFSET, &get_bo_offset);
   if (ret) {
      mesa_loge("DRM_IOCTL_PANFROST_GET_BO_OFFSET failed (err=%d)", errno);
      goto err_free_bo;
   }

   panfrost_bo->offset = get_bo_offset.offset;

   pan_kmod_bo_init(&panfrost_bo->base, dev, NULL, size,
                    flags | PAN_KMOD_BO_FLAG_IMPORTED, handle);
   return &panfrost_bo->base;

err_free_bo:
   pan_kmod_dev_free(dev, panfrost_bo);
   return NULL;
}

static off_t
panfrost_kmod_bo_get_mmap_offset(struct pan_kmod_bo *bo)
{
   struct drm_panfrost_mmap_bo mmap_bo = {.handle = bo->handle};
   int ret = drmIoctl(bo->dev->fd, DRM_IOCTL_PANFROST_MMAP_BO, &mmap_bo);
   if (ret) {
      fprintf(stderr, "DRM_IOCTL_PANFROST_MMAP_BO failed: %m\n");
      assert(0);
   }

   return mmap_bo.offset;
}

static bool
panfrost_kmod_bo_wait(struct pan_kmod_bo *bo, int64_t timeout_ns,
                      bool for_read_only_access)
{
   struct drm_panfrost_wait_bo req = {
      .handle = bo->handle,
      .timeout_ns = timeout_ns,
   };

   /* The ioctl returns >= 0 value when the BO we are waiting for is ready
    * -1 otherwise.
    */
   if (drmIoctl(bo->dev->fd, DRM_IOCTL_PANFROST_WAIT_BO, &req) != -1)
      return true;

   assert(errno == ETIMEDOUT || errno == EBUSY);
   return false;
}

static void
panfrost_kmod_bo_make_evictable(struct pan_kmod_bo *bo)
{
   struct drm_panfrost_madvise req = {
      .handle = bo->handle,
      .madv = PANFROST_MADV_DONTNEED,
   };

   drmIoctl(bo->dev->fd, DRM_IOCTL_PANFROST_MADVISE, &req);
}

static bool
panfrost_kmod_bo_make_unevictable(struct pan_kmod_bo *bo)
{
   struct drm_panfrost_madvise req = {
      .handle = bo->handle,
      .madv = PANFROST_MADV_WILLNEED,
   };

   if (drmIoctl(bo->dev->fd, DRM_IOCTL_PANFROST_MADVISE, &req) == 0 &&
       req.retained == 0)
      return false;

   return true;
}

/* The VA range is restricted by the kernel driver. Lower 32MB are reserved, and
 * the address space is limited to 32-bit.
 */
#define PANFROST_KMOD_VA_START 0x2000000ull
#define PANFROST_KMOD_VA_END   (1ull << 32)

static struct pan_kmod_va_range
panfrost_kmod_dev_query_user_va_range(const struct pan_kmod_dev *dev)
{
   return (struct pan_kmod_va_range){
      .start = PANFROST_KMOD_VA_START,
      .size = PANFROST_KMOD_VA_END - PANFROST_KMOD_VA_START,
   };
}

static struct pan_kmod_vm *
panfrost_kmod_vm_create(struct pan_kmod_dev *dev, uint32_t flags,
                        uint64_t va_start, uint64_t va_range)
{
   struct panfrost_kmod_dev *panfrost_dev =
      container_of(dev, struct panfrost_kmod_dev, base);

   /* Only one VM per device. */
   if (panfrost_dev->vm) {
      mesa_loge("panfrost_kmod only supports one VM per device");
      return NULL;
   }

   /* Panfrost kernel driver doesn't support userspace VA management. */
   if (!(flags & PAN_KMOD_VM_FLAG_AUTO_VA)) {
      mesa_loge("panfrost_kmod only supports PAN_KMOD_VM_FLAG_AUTO_VA");
      assert(0);
      return NULL;
   }

   struct panfrost_kmod_vm *vm = pan_kmod_dev_alloc(dev, sizeof(*vm));
   if (!vm) {
      mesa_loge("failed to allocate a panfrost_kmod_vm object");
      return NULL;
   }

   pan_kmod_vm_init(&vm->base, dev, 0, flags);
   panfrost_dev->vm = vm;
   return &vm->base;
}

static void
panfrost_kmod_vm_destroy(struct pan_kmod_vm *vm)
{
   struct panfrost_kmod_dev *panfrost_dev =
      container_of(vm->dev, struct panfrost_kmod_dev, base);

   panfrost_dev->vm = NULL;
   pan_kmod_dev_free(vm->dev, vm);
}

static int
panfrost_kmod_vm_bind(struct pan_kmod_vm *vm, enum pan_kmod_vm_op_mode mode,
                      struct pan_kmod_vm_op *ops, uint32_t op_count)
{
   UNUSED struct panfrost_kmod_vm *panfrost_vm =
      container_of(vm, struct panfrost_kmod_vm, base);

   /* We only support IMMEDIATE and WAIT_IDLE mode. Actually we always do
    * WAIT_IDLE in practice, but it shouldn't matter.
    */
   if (mode != PAN_KMOD_VM_OP_MODE_IMMEDIATE &&
       mode != PAN_KMOD_VM_OP_MODE_DEFER_TO_NEXT_IDLE_POINT) {
      mesa_loge("panfrost_kmod doesn't support mode=%d", mode);
      assert(0);
      return -1;
   }

   for (uint32_t i = 0; i < op_count; i++) {

      if (ops[i].type == PAN_KMOD_VM_OP_TYPE_MAP) {
         struct panfrost_kmod_bo *panfrost_bo =
            container_of(ops[i].map.bo, struct panfrost_kmod_bo, base);

         /* Panfrost kernel driver doesn't support userspace VA management. */
         if (ops[i].va.start != PAN_KMOD_VM_MAP_AUTO_VA) {
            mesa_loge("panfrost_kmod can only do auto-VA allocation");
            assert(0);
            return -1;
         }

         /* Panfrost kernel driver only support full BO mapping. */
         if (ops[i].map.bo_offset != 0 ||
             ops[i].va.size != ops[i].map.bo->size) {
            mesa_loge("panfrost_kmod doesn't support partial BO mapping");
            assert(0);
            return -1;
         }

         ops[i].va.start = panfrost_bo->offset;
      } else if (ops[i].type == PAN_KMOD_VM_OP_TYPE_UNMAP) {
         /* Do nothing, unmapping is done at BO destruction time. */
      } else {
         /* We reject PAN_KMOD_VM_OP_TYPE_SYNC_ONLY as this implies
          * supporting PAN_KMOD_VM_OP_MODE_ASYNC, which we don't support.
          */
         mesa_loge("panfrost_kmod doesn't support op=%d", ops[i].type);
         assert(0);
         return -1;
      }
   }

   return 0;
}

const struct pan_kmod_ops panfrost_kmod_ops = {
   .dev_create = panfrost_kmod_dev_create,
   .dev_destroy = panfrost_kmod_dev_destroy,
   .dev_query_props = panfrost_dev_query_props,
   .dev_query_user_va_range = panfrost_kmod_dev_query_user_va_range,
   .bo_alloc = panfrost_kmod_bo_alloc,
   .bo_free = panfrost_kmod_bo_free,
   .bo_import = panfrost_kmod_bo_import,
   .bo_get_mmap_offset = panfrost_kmod_bo_get_mmap_offset,
   .bo_wait = panfrost_kmod_bo_wait,
   .bo_make_evictable = panfrost_kmod_bo_make_evictable,
   .bo_make_unevictable = panfrost_kmod_bo_make_unevictable,
   .vm_create = panfrost_kmod_vm_create,
   .vm_destroy = panfrost_kmod_vm_destroy,
   .vm_bind = panfrost_kmod_vm_bind,
};
