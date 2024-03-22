/*
 * Copyright © 2023 Intel Corporation
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

#include <sys/mman.h>

#include "common/xe/intel_engine.h"

#include "anv_private.h"

#include "xe/anv_batch_chain.h"

#include "drm-uapi/gpu_scheduler.h"
#include "drm-uapi/xe_drm.h"

static uint32_t
xe_gem_create(struct anv_device *device,
              const struct intel_memory_class_instance **regions,
              uint16_t regions_count, uint64_t size,
              enum anv_bo_alloc_flags alloc_flags,
              uint64_t *actual_size)
{
   /* TODO: protected content */
   assert((alloc_flags & ANV_BO_ALLOC_PROTECTED) == 0);
   /* WB+0 way coherent not supported by Xe KMD */
   assert(alloc_flags & ANV_BO_ALLOC_HOST_COHERENT);

   uint32_t flags = 0;
   if (alloc_flags & ANV_BO_ALLOC_SCANOUT)
      flags |= DRM_XE_GEM_CREATE_FLAG_SCANOUT;
   if ((alloc_flags & (ANV_BO_ALLOC_MAPPED | ANV_BO_ALLOC_LOCAL_MEM_CPU_VISIBLE)) &&
       !(alloc_flags & ANV_BO_ALLOC_NO_LOCAL_MEM) &&
       device->physical->vram_non_mappable.size > 0)
      flags |= DRM_XE_GEM_CREATE_FLAG_NEEDS_VISIBLE_VRAM;

   struct drm_xe_gem_create gem_create = {
     /* From xe_drm.h: If a VM is specified, this BO must:
      * 1. Only ever be bound to that VM.
      * 2. Cannot be exported as a PRIME fd.
      */
     .vm_id = alloc_flags & ANV_BO_ALLOC_EXTERNAL ? 0 : device->vm_id,
     .size = align64(size, device->info->mem_alignment),
     .flags = flags,
   };
   for (uint16_t i = 0; i < regions_count; i++)
      gem_create.placement |= BITFIELD_BIT(regions[i]->instance);

   const struct intel_device_info_pat_entry *pat_entry =
         anv_device_get_pat_entry(device, alloc_flags);
   switch (pat_entry->mmap) {
   case INTEL_DEVICE_INFO_MMAP_MODE_WC:
      gem_create.cpu_caching = DRM_XE_GEM_CPU_CACHING_WC;
      break;
   case INTEL_DEVICE_INFO_MMAP_MODE_WB:
      gem_create.cpu_caching = DRM_XE_GEM_CPU_CACHING_WB;
      break;
   default:
      unreachable("missing");
      gem_create.cpu_caching = DRM_XE_GEM_CPU_CACHING_WC;
   }

   if (intel_ioctl(device->fd, DRM_IOCTL_XE_GEM_CREATE, &gem_create))
      return 0;

   *actual_size = gem_create.size;
   return gem_create.handle;
}

static void
xe_gem_close(struct anv_device *device, struct anv_bo *bo)
{
   if (bo->from_host_ptr)
      return;

   struct drm_gem_close close = {
      .handle = bo->gem_handle,
   };
   intel_ioctl(device->fd, DRM_IOCTL_GEM_CLOSE, &close);
}

static void *
xe_gem_mmap(struct anv_device *device, struct anv_bo *bo, uint64_t offset,
            uint64_t size)
{
   struct drm_xe_gem_mmap_offset args = {
      .handle = bo->gem_handle,
   };
   if (intel_ioctl(device->fd, DRM_IOCTL_XE_GEM_MMAP_OFFSET, &args))
      return MAP_FAILED;

   return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
               device->fd, args.offset);
}

static inline int
xe_vm_bind_op(struct anv_device *device,
              struct anv_sparse_submission *submit)
{
   struct drm_xe_sync xe_sync = {
      .type = DRM_XE_SYNC_TYPE_SYNCOBJ,
      .flags = DRM_XE_SYNC_FLAG_SIGNAL,
   };
   struct drm_xe_vm_bind args = {
      .vm_id = device->vm_id,
      .num_binds = submit->binds_len,
      .bind = {},
      .num_syncs = 1,
      .syncs = (uintptr_t)&xe_sync,
   };
   struct drm_syncobj_create syncobj_create = {};
   struct drm_syncobj_destroy syncobj_destroy = {};
   struct drm_syncobj_wait syncobj_wait = {
      .timeout_nsec = INT64_MAX,
      .count_handles = 1,
   };
   int ret;

   STACK_ARRAY(struct drm_xe_vm_bind_op, xe_binds_stackarray,
               submit->binds_len);
   struct drm_xe_vm_bind_op *xe_binds;
   if (submit->binds_len > 1) {
      if (!xe_binds_stackarray)
         return -ENOMEM;

      xe_binds = xe_binds_stackarray;
      args.vector_of_binds = (uintptr_t)xe_binds;
   } else {
      xe_binds = &args.bind;
   }

   for (int i = 0; i < submit->binds_len; i++) {
      struct anv_vm_bind *bind = &submit->binds[i];
      struct anv_bo *bo = bind->bo;

      struct drm_xe_vm_bind_op *xe_bind = &xe_binds[i];
      *xe_bind = (struct drm_xe_vm_bind_op) {
         .obj = 0,
         .obj_offset = bind->bo_offset,
         .range = bind->size,
         .addr = intel_48b_address(bind->address),
         .op = DRM_XE_VM_BIND_OP_UNMAP,
         .flags = 0,
         .prefetch_mem_region_instance = 0,
      };

      if (bind->op == ANV_VM_BIND) {
         const enum anv_bo_alloc_flags alloc_flags = bo ? bo->alloc_flags : 0;

         xe_bind->pat_index = anv_device_get_pat_entry(device, alloc_flags)->index;
         if (!bo) {
            xe_bind->op = DRM_XE_VM_BIND_OP_MAP;
            xe_bind->flags |= DRM_XE_VM_BIND_FLAG_NULL;
            assert(xe_bind->obj_offset == 0);
         } else if (bo->from_host_ptr) {
            xe_bind->op = DRM_XE_VM_BIND_OP_MAP_USERPTR;
         } else {
            xe_bind->op = DRM_XE_VM_BIND_OP_MAP;
            xe_bind->obj = bo->gem_handle;
         }
      }

      /* userptr and bo_offset are an union! */
      if (bo && bo->from_host_ptr)
         xe_bind->userptr = (uintptr_t)bo->map;
   }

   ret = intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_CREATE, &syncobj_create);
   if (ret)
      goto out_stackarray;

   xe_sync.handle = syncobj_create.handle;
   ret = intel_ioctl(device->fd, DRM_IOCTL_XE_VM_BIND, &args);
   if (ret)
      goto out_destroy_syncobj;

   syncobj_wait.handles = (uintptr_t)&xe_sync.handle;
   ret = intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_WAIT, &syncobj_wait);

out_destroy_syncobj:
   syncobj_destroy.handle = xe_sync.handle;
   intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_DESTROY, &syncobj_destroy);
out_stackarray:
   STACK_ARRAY_FINISH(xe_binds_stackarray);

   return ret;
}

static int
xe_vm_bind(struct anv_device *device, struct anv_sparse_submission *submit)
{
   return xe_vm_bind_op(device, submit);
}

static int xe_vm_bind_bo(struct anv_device *device, struct anv_bo *bo)
{
   struct anv_vm_bind bind = {
      .bo = bo,
      .address = bo->offset,
      .bo_offset = 0,
      .size = bo->actual_size,
      .op = ANV_VM_BIND,
   };
   struct anv_sparse_submission submit = {
      .queue = NULL,
      .binds = &bind,
      .binds_len = 1,
      .binds_capacity = 1,
      .wait_count = 0,
      .signal_count = 0,
   };
   return xe_vm_bind_op(device, &submit);
}

static int xe_vm_unbind_bo(struct anv_device *device, struct anv_bo *bo)
{
   struct anv_vm_bind bind = {
      .bo = bo,
      .address = bo->offset,
      .bo_offset = 0,
      .size = bo->actual_size,
      .op = ANV_VM_UNBIND,
   };
   struct anv_sparse_submission submit = {
      .queue = NULL,
      .binds = &bind,
      .binds_len = 1,
      .binds_capacity = 1,
      .wait_count = 0,
      .signal_count = 0,
   };
   return xe_vm_bind_op(device, &submit);
}

static uint32_t
xe_gem_create_userptr(struct anv_device *device, void *mem, uint64_t size)
{
   /* We return the workaround BO gem_handle here, because Xe doesn't
    * create handles for userptrs. But we still need to make it look
    * to the rest of Anv that the operation succeeded.
    */
   return device->workaround_bo->gem_handle;
}

static uint32_t
xe_bo_alloc_flags_to_bo_flags(struct anv_device *device,
                              enum anv_bo_alloc_flags alloc_flags)
{
   return 0;
}

const struct anv_kmd_backend *
anv_xe_kmd_backend_get(void)
{
   static const struct anv_kmd_backend xe_backend = {
      .gem_create = xe_gem_create,
      .gem_create_userptr = xe_gem_create_userptr,
      .gem_close = xe_gem_close,
      .gem_mmap = xe_gem_mmap,
      .vm_bind = xe_vm_bind,
      .vm_bind_bo = xe_vm_bind_bo,
      .vm_unbind_bo = xe_vm_unbind_bo,
      .execute_simple_batch = xe_execute_simple_batch,
      .execute_trtt_batch = xe_execute_trtt_batch,
      .queue_exec_locked = xe_queue_exec_locked,
      .queue_exec_trace = xe_queue_exec_utrace_locked,
      .bo_alloc_flags_to_bo_flags = xe_bo_alloc_flags_to_bo_flags,
   };
   return &xe_backend;
}
