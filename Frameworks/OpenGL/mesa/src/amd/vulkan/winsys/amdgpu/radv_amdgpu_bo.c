/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based on amdgpu winsys.
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Advanced Micro Devices, Inc.
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

#include <stdio.h>

#include "radv_amdgpu_bo.h"
#include "radv_debug.h"

#include <amdgpu.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include "drm-uapi/amdgpu_drm.h"

#include "util/os_time.h"
#include "util/u_atomic.h"
#include "util/u_math.h"
#include "util/u_memory.h"

static void radv_amdgpu_winsys_bo_destroy(struct radeon_winsys *_ws, struct radeon_winsys_bo *_bo);

static int
radv_amdgpu_bo_va_op(struct radv_amdgpu_winsys *ws, amdgpu_bo_handle bo, uint64_t offset, uint64_t size, uint64_t addr,
                     uint32_t bo_flags, uint64_t internal_flags, uint32_t ops)
{
   uint64_t flags = internal_flags;
   if (bo) {
      flags = AMDGPU_VM_PAGE_READABLE | AMDGPU_VM_PAGE_EXECUTABLE;

      if ((bo_flags & RADEON_FLAG_VA_UNCACHED) && ws->info.gfx_level >= GFX9)
         flags |= AMDGPU_VM_MTYPE_UC;

      if (!(bo_flags & RADEON_FLAG_READ_ONLY))
         flags |= AMDGPU_VM_PAGE_WRITEABLE;
   }

   size = align64(size, getpagesize());

   return amdgpu_bo_va_op_raw(ws->dev, bo, offset, size, addr, flags, ops);
}

static int
bo_comparator(const void *ap, const void *bp)
{
   struct radv_amdgpu_bo *a = *(struct radv_amdgpu_bo *const *)ap;
   struct radv_amdgpu_bo *b = *(struct radv_amdgpu_bo *const *)bp;
   return (a > b) ? 1 : (a < b) ? -1 : 0;
}

static VkResult
radv_amdgpu_winsys_rebuild_bo_list(struct radv_amdgpu_winsys_bo *bo)
{
   u_rwlock_wrlock(&bo->lock);

   if (bo->bo_capacity < bo->range_count) {
      uint32_t new_count = MAX2(bo->bo_capacity * 2, bo->range_count);
      struct radv_amdgpu_winsys_bo **bos = realloc(bo->bos, new_count * sizeof(struct radv_amdgpu_winsys_bo *));
      if (!bos) {
         u_rwlock_wrunlock(&bo->lock);
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }
      bo->bos = bos;
      bo->bo_capacity = new_count;
   }

   uint32_t temp_bo_count = 0;
   for (uint32_t i = 0; i < bo->range_count; ++i)
      if (bo->ranges[i].bo)
         bo->bos[temp_bo_count++] = bo->ranges[i].bo;

   qsort(bo->bos, temp_bo_count, sizeof(struct radv_amdgpu_winsys_bo *), &bo_comparator);

   if (!temp_bo_count) {
      bo->bo_count = 0;
   } else {
      uint32_t final_bo_count = 1;
      for (uint32_t i = 1; i < temp_bo_count; ++i)
         if (bo->bos[i] != bo->bos[i - 1])
            bo->bos[final_bo_count++] = bo->bos[i];

      bo->bo_count = final_bo_count;
   }

   u_rwlock_wrunlock(&bo->lock);
   return VK_SUCCESS;
}

static VkResult
radv_amdgpu_winsys_bo_virtual_bind(struct radeon_winsys *_ws, struct radeon_winsys_bo *_parent, uint64_t offset,
                                   uint64_t size, struct radeon_winsys_bo *_bo, uint64_t bo_offset)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct radv_amdgpu_winsys_bo *parent = (struct radv_amdgpu_winsys_bo *)_parent;
   struct radv_amdgpu_winsys_bo *bo = (struct radv_amdgpu_winsys_bo *)_bo;
   int range_count_delta, new_idx;
   int first = 0, last;
   struct radv_amdgpu_map_range new_first, new_last;
   VkResult result;
   int r;

   assert(parent->is_virtual);
   assert(!bo || !bo->is_virtual);

   /* When the BO is NULL, AMDGPU will reset the PTE VA range to the initial state. Otherwise, it
    * will first unmap all existing VA that overlap the requested range and then map.
    */
   if (bo) {
      r = radv_amdgpu_bo_va_op(ws, bo->bo, bo_offset, size, parent->base.va + offset, 0, 0, AMDGPU_VA_OP_REPLACE);
   } else {
      r =
         radv_amdgpu_bo_va_op(ws, NULL, 0, size, parent->base.va + offset, 0, AMDGPU_VM_PAGE_PRT, AMDGPU_VA_OP_REPLACE);
   }

   if (r) {
      fprintf(stderr, "radv/amdgpu: Failed to replace a PRT VA region (%d).\n", r);
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;
   }

   /* Do not add the BO to the virtual BO list if it's already in the global list to avoid dangling
    * BO references because it might have been destroyed without being previously unbound. Resetting
    * it to NULL clears the old BO ranges if present.
    *
    * This is going to be clarified in the Vulkan spec:
    * https://gitlab.khronos.org/vulkan/vulkan/-/issues/3125
    *
    * The issue still exists for non-global BO but it will be addressed later, once we are 100% it's
    * RADV fault (mostly because the solution looks more complicated).
    */
   if (bo && radv_buffer_is_resident(&bo->base)) {
      bo = NULL;
      bo_offset = 0;
   }

   /* We have at most 2 new ranges (1 by the bind, and another one by splitting a range that
    * contains the newly bound range). */
   if (parent->range_capacity - parent->range_count < 2) {
      uint32_t range_capacity = parent->range_capacity + 2;
      struct radv_amdgpu_map_range *ranges =
         realloc(parent->ranges, range_capacity * sizeof(struct radv_amdgpu_map_range));
      if (!ranges)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      parent->ranges = ranges;
      parent->range_capacity = range_capacity;
   }

   /*
    * [first, last] is exactly the range of ranges that either overlap the
    * new parent, or are adjacent to it. This corresponds to the bind ranges
    * that may change.
    */
   while (first + 1 < parent->range_count && parent->ranges[first].offset + parent->ranges[first].size < offset)
      ++first;

   last = first;
   while (last + 1 < parent->range_count && parent->ranges[last + 1].offset <= offset + size)
      ++last;

   /* Whether the first or last range are going to be totally removed or just
    * resized/left alone. Note that in the case of first == last, we will split
    * this into a part before and after the new range. The remove flag is then
    * whether to not create the corresponding split part. */
   bool remove_first = parent->ranges[first].offset == offset;
   bool remove_last = parent->ranges[last].offset + parent->ranges[last].size == offset + size;

   assert(parent->ranges[first].offset <= offset);
   assert(parent->ranges[last].offset + parent->ranges[last].size >= offset + size);

   /* Try to merge the new range with the first range. */
   if (parent->ranges[first].bo == bo &&
       (!bo || offset - bo_offset == parent->ranges[first].offset - parent->ranges[first].bo_offset)) {
      size += offset - parent->ranges[first].offset;
      offset = parent->ranges[first].offset;
      bo_offset = parent->ranges[first].bo_offset;
      remove_first = true;
   }

   /* Try to merge the new range with the last range. */
   if (parent->ranges[last].bo == bo &&
       (!bo || offset - bo_offset == parent->ranges[last].offset - parent->ranges[last].bo_offset)) {
      size = parent->ranges[last].offset + parent->ranges[last].size - offset;
      remove_last = true;
   }

   range_count_delta = 1 - (last - first + 1) + !remove_first + !remove_last;
   new_idx = first + !remove_first;

   /* If the first/last range are not left alone we unmap then and optionally map
    * them again after modifications. Not that this implicitly can do the splitting
    * if first == last. */
   new_first = parent->ranges[first];
   new_last = parent->ranges[last];

   if (parent->ranges[first].offset + parent->ranges[first].size > offset || remove_first) {
      if (!remove_first) {
         new_first.size = offset - new_first.offset;
      }
   }

   if (parent->ranges[last].offset < offset + size || remove_last) {
      if (!remove_last) {
         new_last.size -= offset + size - new_last.offset;
         new_last.bo_offset += (offset + size - new_last.offset);
         new_last.offset = offset + size;
      }
   }

   /* Moves the range list after last to account for the changed number of ranges. */
   memmove(parent->ranges + last + 1 + range_count_delta, parent->ranges + last + 1,
           sizeof(struct radv_amdgpu_map_range) * (parent->range_count - last - 1));

   if (!remove_first)
      parent->ranges[first] = new_first;

   if (!remove_last)
      parent->ranges[new_idx + 1] = new_last;

   /* Actually set up the new range. */
   parent->ranges[new_idx].offset = offset;
   parent->ranges[new_idx].size = size;
   parent->ranges[new_idx].bo = bo;
   parent->ranges[new_idx].bo_offset = bo_offset;

   parent->range_count += range_count_delta;

   result = radv_amdgpu_winsys_rebuild_bo_list(parent);
   if (result != VK_SUCCESS)
      return result;

   return VK_SUCCESS;
}

struct radv_amdgpu_winsys_bo_log {
   struct list_head list;
   uint64_t va;
   uint64_t size;
   uint64_t timestamp; /* CPU timestamp */
   uint8_t is_virtual : 1;
   uint8_t destroyed : 1;
};

static void
radv_amdgpu_log_bo(struct radv_amdgpu_winsys *ws, struct radv_amdgpu_winsys_bo *bo, bool destroyed)
{
   struct radv_amdgpu_winsys_bo_log *bo_log = NULL;

   if (!ws->debug_log_bos)
      return;

   bo_log = malloc(sizeof(*bo_log));
   if (!bo_log)
      return;

   bo_log->va = bo->base.va;
   bo_log->size = bo->size;
   bo_log->timestamp = os_time_get_nano();
   bo_log->is_virtual = bo->is_virtual;
   bo_log->destroyed = destroyed;

   u_rwlock_wrlock(&ws->log_bo_list_lock);
   list_addtail(&bo_log->list, &ws->log_bo_list);
   u_rwlock_wrunlock(&ws->log_bo_list_lock);
}

static int
radv_amdgpu_global_bo_list_add(struct radv_amdgpu_winsys *ws, struct radv_amdgpu_winsys_bo *bo)
{
   u_rwlock_wrlock(&ws->global_bo_list.lock);
   if (ws->global_bo_list.count == ws->global_bo_list.capacity) {
      unsigned capacity = MAX2(4, ws->global_bo_list.capacity * 2);
      void *data = realloc(ws->global_bo_list.bos, capacity * sizeof(struct radv_amdgpu_winsys_bo *));
      if (!data) {
         u_rwlock_wrunlock(&ws->global_bo_list.lock);
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }

      ws->global_bo_list.bos = (struct radv_amdgpu_winsys_bo **)data;
      ws->global_bo_list.capacity = capacity;
   }

   ws->global_bo_list.bos[ws->global_bo_list.count++] = bo;
   bo->base.use_global_list = true;
   u_rwlock_wrunlock(&ws->global_bo_list.lock);
   return VK_SUCCESS;
}

static void
radv_amdgpu_global_bo_list_del(struct radv_amdgpu_winsys *ws, struct radv_amdgpu_winsys_bo *bo)
{
   u_rwlock_wrlock(&ws->global_bo_list.lock);
   for (unsigned i = ws->global_bo_list.count; i-- > 0;) {
      if (ws->global_bo_list.bos[i] == bo) {
         ws->global_bo_list.bos[i] = ws->global_bo_list.bos[ws->global_bo_list.count - 1];
         --ws->global_bo_list.count;
         bo->base.use_global_list = false;
         break;
      }
   }
   u_rwlock_wrunlock(&ws->global_bo_list.lock);
}

static void
radv_amdgpu_winsys_bo_destroy(struct radeon_winsys *_ws, struct radeon_winsys_bo *_bo)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct radv_amdgpu_winsys_bo *bo = radv_amdgpu_winsys_bo(_bo);

   radv_amdgpu_log_bo(ws, bo, true);

   if (bo->is_virtual) {
      int r;

      /* Clear mappings of this PRT VA region. */
      r = radv_amdgpu_bo_va_op(ws, NULL, 0, bo->size, bo->base.va, 0, 0, AMDGPU_VA_OP_CLEAR);
      if (r) {
         fprintf(stderr, "radv/amdgpu: Failed to clear a PRT VA region (%d).\n", r);
      }

      free(bo->bos);
      free(bo->ranges);
      u_rwlock_destroy(&bo->lock);
   } else {
      if (ws->debug_all_bos)
         radv_amdgpu_global_bo_list_del(ws, bo);
      radv_amdgpu_bo_va_op(ws, bo->bo, 0, bo->size, bo->base.va, 0, 0, AMDGPU_VA_OP_UNMAP);
      amdgpu_bo_free(bo->bo);
   }

   if (bo->base.initial_domain & RADEON_DOMAIN_VRAM) {
      if (bo->base.vram_no_cpu_access) {
         p_atomic_add(&ws->allocated_vram, -align64(bo->size, ws->info.gart_page_size));
      } else {
         p_atomic_add(&ws->allocated_vram_vis, -align64(bo->size, ws->info.gart_page_size));
      }
   }

   if (bo->base.initial_domain & RADEON_DOMAIN_GTT)
      p_atomic_add(&ws->allocated_gtt, -align64(bo->size, ws->info.gart_page_size));

   amdgpu_va_range_free(bo->va_handle);
   FREE(bo);
}

static VkResult
radv_amdgpu_winsys_bo_create(struct radeon_winsys *_ws, uint64_t size, unsigned alignment,
                             enum radeon_bo_domain initial_domain, enum radeon_bo_flag flags, unsigned priority,
                             uint64_t replay_address, struct radeon_winsys_bo **out_bo)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct radv_amdgpu_winsys_bo *bo;
   struct amdgpu_bo_alloc_request request = {0};
   struct radv_amdgpu_map_range *ranges = NULL;
   amdgpu_bo_handle buf_handle;
   uint64_t va = 0;
   amdgpu_va_handle va_handle;
   int r;
   VkResult result = VK_SUCCESS;

   /* Just be robust for callers that might use NULL-ness for determining if things should be freed.
    */
   *out_bo = NULL;

   bo = CALLOC_STRUCT(radv_amdgpu_winsys_bo);
   if (!bo) {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   unsigned virt_alignment = alignment;
   if (size >= ws->info.pte_fragment_size)
      virt_alignment = MAX2(virt_alignment, ws->info.pte_fragment_size);

   assert(!replay_address || (flags & RADEON_FLAG_REPLAYABLE));

   const uint64_t va_flags = AMDGPU_VA_RANGE_HIGH | (flags & RADEON_FLAG_32BIT ? AMDGPU_VA_RANGE_32_BIT : 0) |
                             (flags & RADEON_FLAG_REPLAYABLE ? AMDGPU_VA_RANGE_REPLAYABLE : 0);
   r = amdgpu_va_range_alloc(ws->dev, amdgpu_gpu_va_range_general, size, virt_alignment, replay_address, &va,
                             &va_handle, va_flags);
   if (r) {
      result = replay_address ? VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS : VK_ERROR_OUT_OF_DEVICE_MEMORY;
      goto error_va_alloc;
   }

   bo->base.va = va;
   bo->va_handle = va_handle;
   bo->size = size;
   bo->is_virtual = !!(flags & RADEON_FLAG_VIRTUAL);

   if (flags & RADEON_FLAG_VIRTUAL) {
      ranges = realloc(NULL, sizeof(struct radv_amdgpu_map_range));
      if (!ranges) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto error_ranges_alloc;
      }

      u_rwlock_init(&bo->lock);

      bo->ranges = ranges;
      bo->range_count = 1;
      bo->range_capacity = 1;

      bo->ranges[0].offset = 0;
      bo->ranges[0].size = size;
      bo->ranges[0].bo = NULL;
      bo->ranges[0].bo_offset = 0;

      /* Reserve a PRT VA region. */
      r = radv_amdgpu_bo_va_op(ws, NULL, 0, size, bo->base.va, 0, AMDGPU_VM_PAGE_PRT, AMDGPU_VA_OP_MAP);
      if (r) {
         fprintf(stderr, "radv/amdgpu: Failed to reserve a PRT VA region (%d).\n", r);
         result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
         goto error_ranges_alloc;
      }

      radv_amdgpu_log_bo(ws, bo, false);

      *out_bo = (struct radeon_winsys_bo *)bo;
      return VK_SUCCESS;
   }

   request.alloc_size = size;
   request.phys_alignment = alignment;

   if (initial_domain & RADEON_DOMAIN_VRAM) {
      request.preferred_heap |= AMDGPU_GEM_DOMAIN_VRAM;

      /* Since VRAM and GTT have almost the same performance on
       * APUs, we could just set GTT. However, in order to decrease
       * GTT(RAM) usage, which is shared with the OS, allow VRAM
       * placements too. The idea is not to use VRAM usefully, but
       * to use it so that it's not unused and wasted.
       *
       * Furthermore, even on discrete GPUs this is beneficial. If
       * both GTT and VRAM are set then AMDGPU still prefers VRAM
       * for the initial placement, but it makes the buffers
       * spillable. Otherwise AMDGPU tries to place the buffers in
       * VRAM really hard to the extent that we are getting a lot
       * of unnecessary movement. This helps significantly when
       * e.g. Horizon Zero Dawn allocates more memory than we have
       * VRAM.
       */
      request.preferred_heap |= AMDGPU_GEM_DOMAIN_GTT;
   }

   if (initial_domain & RADEON_DOMAIN_GTT)
      request.preferred_heap |= AMDGPU_GEM_DOMAIN_GTT;
   if (initial_domain & RADEON_DOMAIN_GDS)
      request.preferred_heap |= AMDGPU_GEM_DOMAIN_GDS;
   if (initial_domain & RADEON_DOMAIN_OA)
      request.preferred_heap |= AMDGPU_GEM_DOMAIN_OA;

   if (flags & RADEON_FLAG_CPU_ACCESS)
      request.flags |= AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED;
   if (flags & RADEON_FLAG_NO_CPU_ACCESS) {
      bo->base.vram_no_cpu_access = initial_domain & RADEON_DOMAIN_VRAM;
      request.flags |= AMDGPU_GEM_CREATE_NO_CPU_ACCESS;
   }
   if (flags & RADEON_FLAG_GTT_WC)
      request.flags |= AMDGPU_GEM_CREATE_CPU_GTT_USWC;
   if (!(flags & RADEON_FLAG_IMPLICIT_SYNC))
      request.flags |= AMDGPU_GEM_CREATE_EXPLICIT_SYNC;
   if ((initial_domain & RADEON_DOMAIN_VRAM_GTT) && (flags & RADEON_FLAG_NO_INTERPROCESS_SHARING) &&
       ((ws->perftest & RADV_PERFTEST_LOCAL_BOS) || (flags & RADEON_FLAG_PREFER_LOCAL_BO))) {
      bo->base.is_local = true;
      request.flags |= AMDGPU_GEM_CREATE_VM_ALWAYS_VALID;
   }

   if (initial_domain & RADEON_DOMAIN_VRAM) {
      if (ws->zero_all_vram_allocs || (flags & RADEON_FLAG_ZERO_VRAM))
         request.flags |= AMDGPU_GEM_CREATE_VRAM_CLEARED;
   }

   if (flags & RADEON_FLAG_DISCARDABLE && ws->info.drm_minor >= 47)
      request.flags |= AMDGPU_GEM_CREATE_DISCARDABLE;

   r = amdgpu_bo_alloc(ws->dev, &request, &buf_handle);
   if (r) {
      fprintf(stderr, "radv/amdgpu: Failed to allocate a buffer:\n");
      fprintf(stderr, "radv/amdgpu:    size      : %" PRIu64 " bytes\n", size);
      fprintf(stderr, "radv/amdgpu:    alignment : %u bytes\n", alignment);
      fprintf(stderr, "radv/amdgpu:    domains   : %u\n", initial_domain);
      result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
      goto error_bo_alloc;
   }

   r = radv_amdgpu_bo_va_op(ws, buf_handle, 0, size, va, flags, 0, AMDGPU_VA_OP_MAP);
   if (r) {
      result = VK_ERROR_UNKNOWN;
      goto error_va_map;
   }

   bo->bo = buf_handle;
   bo->base.initial_domain = initial_domain;
   bo->base.use_global_list = false;
   bo->priority = priority;

   r = amdgpu_bo_export(buf_handle, amdgpu_bo_handle_type_kms, &bo->bo_handle);
   assert(!r);

   if (initial_domain & RADEON_DOMAIN_VRAM) {
      /* Buffers allocated in VRAM with the NO_CPU_ACCESS flag
       * aren't mappable and they are counted as part of the VRAM
       * counter.
       *
       * Otherwise, buffers with the CPU_ACCESS flag or without any
       * of both (imported buffers) are counted as part of the VRAM
       * visible counter because they can be mapped.
       */
      if (bo->base.vram_no_cpu_access) {
         p_atomic_add(&ws->allocated_vram, align64(bo->size, ws->info.gart_page_size));
      } else {
         p_atomic_add(&ws->allocated_vram_vis, align64(bo->size, ws->info.gart_page_size));
      }
   }

   if (initial_domain & RADEON_DOMAIN_GTT)
      p_atomic_add(&ws->allocated_gtt, align64(bo->size, ws->info.gart_page_size));

   if (ws->debug_all_bos)
      radv_amdgpu_global_bo_list_add(ws, bo);
   radv_amdgpu_log_bo(ws, bo, false);

   *out_bo = (struct radeon_winsys_bo *)bo;
   return VK_SUCCESS;
error_va_map:
   amdgpu_bo_free(buf_handle);

error_bo_alloc:
   free(ranges);

error_ranges_alloc:
   amdgpu_va_range_free(va_handle);

error_va_alloc:
   FREE(bo);
   return result;
}

static void *
radv_amdgpu_winsys_bo_map(struct radeon_winsys_bo *_bo)
{
   struct radv_amdgpu_winsys_bo *bo = radv_amdgpu_winsys_bo(_bo);
   int ret;
   void *data;
   ret = amdgpu_bo_cpu_map(bo->bo, &data);
   if (ret)
      return NULL;
   return data;
}

static void
radv_amdgpu_winsys_bo_unmap(struct radeon_winsys_bo *_bo)
{
   struct radv_amdgpu_winsys_bo *bo = radv_amdgpu_winsys_bo(_bo);
   amdgpu_bo_cpu_unmap(bo->bo);
}

static uint64_t
radv_amdgpu_get_optimal_vm_alignment(struct radv_amdgpu_winsys *ws, uint64_t size, unsigned alignment)
{
   uint64_t vm_alignment = alignment;

   /* Increase the VM alignment for faster address translation. */
   if (size >= ws->info.pte_fragment_size)
      vm_alignment = MAX2(vm_alignment, ws->info.pte_fragment_size);

   /* Gfx9: Increase the VM alignment to the most significant bit set
    * in the size for faster address translation.
    */
   if (ws->info.gfx_level >= GFX9) {
      unsigned msb = util_last_bit64(size); /* 0 = no bit is set */
      uint64_t msb_alignment = msb ? 1ull << (msb - 1) : 0;

      vm_alignment = MAX2(vm_alignment, msb_alignment);
   }
   return vm_alignment;
}

static VkResult
radv_amdgpu_winsys_bo_from_ptr(struct radeon_winsys *_ws, void *pointer, uint64_t size, unsigned priority,
                               struct radeon_winsys_bo **out_bo)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   amdgpu_bo_handle buf_handle;
   struct radv_amdgpu_winsys_bo *bo;
   uint64_t va;
   amdgpu_va_handle va_handle;
   uint64_t vm_alignment;
   VkResult result = VK_SUCCESS;
   int ret;

   /* Just be robust for callers that might use NULL-ness for determining if things should be freed.
    */
   *out_bo = NULL;

   bo = CALLOC_STRUCT(radv_amdgpu_winsys_bo);
   if (!bo)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   ret = amdgpu_create_bo_from_user_mem(ws->dev, pointer, size, &buf_handle);
   if (ret) {
      if (ret == -EINVAL) {
         result = VK_ERROR_INVALID_EXTERNAL_HANDLE;
      } else {
         result = VK_ERROR_UNKNOWN;
      }
      goto error;
   }

   /* Using the optimal VM alignment also fixes GPU hangs for buffers that
    * are imported.
    */
   vm_alignment = radv_amdgpu_get_optimal_vm_alignment(ws, size, ws->info.gart_page_size);

   if (amdgpu_va_range_alloc(ws->dev, amdgpu_gpu_va_range_general, size, vm_alignment, 0, &va, &va_handle,
                             AMDGPU_VA_RANGE_HIGH)) {
      result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
      goto error_va_alloc;
   }

   if (amdgpu_bo_va_op(buf_handle, 0, size, va, 0, AMDGPU_VA_OP_MAP)) {
      result = VK_ERROR_UNKNOWN;
      goto error_va_map;
   }

   /* Initialize it */
   bo->base.va = va;
   bo->va_handle = va_handle;
   bo->size = size;
   bo->bo = buf_handle;
   bo->base.initial_domain = RADEON_DOMAIN_GTT;
   bo->base.use_global_list = false;
   bo->priority = priority;

   ASSERTED int r = amdgpu_bo_export(buf_handle, amdgpu_bo_handle_type_kms, &bo->bo_handle);
   assert(!r);

   p_atomic_add(&ws->allocated_gtt, align64(bo->size, ws->info.gart_page_size));

   if (ws->debug_all_bos)
      radv_amdgpu_global_bo_list_add(ws, bo);
   radv_amdgpu_log_bo(ws, bo, false);

   *out_bo = (struct radeon_winsys_bo *)bo;
   return VK_SUCCESS;

error_va_map:
   amdgpu_va_range_free(va_handle);

error_va_alloc:
   amdgpu_bo_free(buf_handle);

error:
   FREE(bo);
   return result;
}

static VkResult
radv_amdgpu_winsys_bo_from_fd(struct radeon_winsys *_ws, int fd, unsigned priority, struct radeon_winsys_bo **out_bo,
                              uint64_t *alloc_size)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct radv_amdgpu_winsys_bo *bo;
   uint64_t va;
   amdgpu_va_handle va_handle;
   enum amdgpu_bo_handle_type type = amdgpu_bo_handle_type_dma_buf_fd;
   struct amdgpu_bo_import_result result;
   struct amdgpu_bo_info info;
   enum radeon_bo_domain initial = 0;
   int r;
   VkResult vk_result = VK_SUCCESS;

   /* Just be robust for callers that might use NULL-ness for determining if things should be freed.
    */
   *out_bo = NULL;

   bo = CALLOC_STRUCT(radv_amdgpu_winsys_bo);
   if (!bo)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   r = amdgpu_bo_import(ws->dev, type, fd, &result);
   if (r) {
      vk_result = VK_ERROR_INVALID_EXTERNAL_HANDLE;
      goto error;
   }

   r = amdgpu_bo_query_info(result.buf_handle, &info);
   if (r) {
      vk_result = VK_ERROR_UNKNOWN;
      goto error_query;
   }

   if (alloc_size) {
      *alloc_size = info.alloc_size;
   }

   r = amdgpu_va_range_alloc(ws->dev, amdgpu_gpu_va_range_general, result.alloc_size, 1 << 20, 0, &va, &va_handle,
                             AMDGPU_VA_RANGE_HIGH);
   if (r) {
      vk_result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
      goto error_query;
   }

   r = radv_amdgpu_bo_va_op(ws, result.buf_handle, 0, result.alloc_size, va, 0, 0, AMDGPU_VA_OP_MAP);
   if (r) {
      vk_result = VK_ERROR_UNKNOWN;
      goto error_va_map;
   }

   if (info.preferred_heap & AMDGPU_GEM_DOMAIN_VRAM)
      initial |= RADEON_DOMAIN_VRAM;
   if (info.preferred_heap & AMDGPU_GEM_DOMAIN_GTT)
      initial |= RADEON_DOMAIN_GTT;

   bo->bo = result.buf_handle;
   bo->base.va = va;
   bo->va_handle = va_handle;
   bo->base.initial_domain = initial;
   bo->base.use_global_list = false;
   bo->size = result.alloc_size;
   bo->priority = priority;

   r = amdgpu_bo_export(result.buf_handle, amdgpu_bo_handle_type_kms, &bo->bo_handle);
   assert(!r);

   if (bo->base.initial_domain & RADEON_DOMAIN_VRAM)
      p_atomic_add(&ws->allocated_vram, align64(bo->size, ws->info.gart_page_size));
   if (bo->base.initial_domain & RADEON_DOMAIN_GTT)
      p_atomic_add(&ws->allocated_gtt, align64(bo->size, ws->info.gart_page_size));

   if (ws->debug_all_bos)
      radv_amdgpu_global_bo_list_add(ws, bo);
   radv_amdgpu_log_bo(ws, bo, false);

   *out_bo = (struct radeon_winsys_bo *)bo;
   return VK_SUCCESS;
error_va_map:
   amdgpu_va_range_free(va_handle);

error_query:
   amdgpu_bo_free(result.buf_handle);

error:
   FREE(bo);
   return vk_result;
}

static bool
radv_amdgpu_winsys_get_fd(struct radeon_winsys *_ws, struct radeon_winsys_bo *_bo, int *fd)
{
   struct radv_amdgpu_winsys_bo *bo = radv_amdgpu_winsys_bo(_bo);
   enum amdgpu_bo_handle_type type = amdgpu_bo_handle_type_dma_buf_fd;
   int r;
   unsigned handle;
   r = amdgpu_bo_export(bo->bo, type, &handle);
   if (r)
      return false;

   *fd = (int)handle;
   return true;
}

static bool
radv_amdgpu_bo_get_flags_from_fd(struct radeon_winsys *_ws, int fd, enum radeon_bo_domain *domains,
                                 enum radeon_bo_flag *flags)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct amdgpu_bo_import_result result = {0};
   struct amdgpu_bo_info info = {0};
   int r;

   *domains = 0;
   *flags = 0;

   r = amdgpu_bo_import(ws->dev, amdgpu_bo_handle_type_dma_buf_fd, fd, &result);
   if (r)
      return false;

   r = amdgpu_bo_query_info(result.buf_handle, &info);
   amdgpu_bo_free(result.buf_handle);
   if (r)
      return false;

   if (info.preferred_heap & AMDGPU_GEM_DOMAIN_VRAM)
      *domains |= RADEON_DOMAIN_VRAM;
   if (info.preferred_heap & AMDGPU_GEM_DOMAIN_GTT)
      *domains |= RADEON_DOMAIN_GTT;
   if (info.preferred_heap & AMDGPU_GEM_DOMAIN_GDS)
      *domains |= RADEON_DOMAIN_GDS;
   if (info.preferred_heap & AMDGPU_GEM_DOMAIN_OA)
      *domains |= RADEON_DOMAIN_OA;

   if (info.alloc_flags & AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED)
      *flags |= RADEON_FLAG_CPU_ACCESS;
   if (info.alloc_flags & AMDGPU_GEM_CREATE_NO_CPU_ACCESS)
      *flags |= RADEON_FLAG_NO_CPU_ACCESS;
   if (!(info.alloc_flags & AMDGPU_GEM_CREATE_EXPLICIT_SYNC))
      *flags |= RADEON_FLAG_IMPLICIT_SYNC;
   if (info.alloc_flags & AMDGPU_GEM_CREATE_CPU_GTT_USWC)
      *flags |= RADEON_FLAG_GTT_WC;
   if (info.alloc_flags & AMDGPU_GEM_CREATE_VM_ALWAYS_VALID)
      *flags |= RADEON_FLAG_NO_INTERPROCESS_SHARING | RADEON_FLAG_PREFER_LOCAL_BO;
   if (info.alloc_flags & AMDGPU_GEM_CREATE_VRAM_CLEARED)
      *flags |= RADEON_FLAG_ZERO_VRAM;
   return true;
}

static unsigned
eg_tile_split(unsigned tile_split)
{
   switch (tile_split) {
   case 0:
      tile_split = 64;
      break;
   case 1:
      tile_split = 128;
      break;
   case 2:
      tile_split = 256;
      break;
   case 3:
      tile_split = 512;
      break;
   default:
   case 4:
      tile_split = 1024;
      break;
   case 5:
      tile_split = 2048;
      break;
   case 6:
      tile_split = 4096;
      break;
   }
   return tile_split;
}

static unsigned
radv_eg_tile_split_rev(unsigned eg_tile_split)
{
   switch (eg_tile_split) {
   case 64:
      return 0;
   case 128:
      return 1;
   case 256:
      return 2;
   case 512:
      return 3;
   default:
   case 1024:
      return 4;
   case 2048:
      return 5;
   case 4096:
      return 6;
   }
}

#define AMDGPU_TILING_DCC_MAX_COMPRESSED_BLOCK_SIZE_SHIFT 45
#define AMDGPU_TILING_DCC_MAX_COMPRESSED_BLOCK_SIZE_MASK  0x3

static void
radv_amdgpu_winsys_bo_set_metadata(struct radeon_winsys *_ws, struct radeon_winsys_bo *_bo,
                                   struct radeon_bo_metadata *md)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct radv_amdgpu_winsys_bo *bo = radv_amdgpu_winsys_bo(_bo);
   struct amdgpu_bo_metadata metadata = {0};
   uint64_t tiling_flags = 0;

   if (ws->info.gfx_level >= GFX9) {
      tiling_flags |= AMDGPU_TILING_SET(SWIZZLE_MODE, md->u.gfx9.swizzle_mode);
      tiling_flags |= AMDGPU_TILING_SET(DCC_OFFSET_256B, md->u.gfx9.dcc_offset_256b);
      tiling_flags |= AMDGPU_TILING_SET(DCC_PITCH_MAX, md->u.gfx9.dcc_pitch_max);
      tiling_flags |= AMDGPU_TILING_SET(DCC_INDEPENDENT_64B, md->u.gfx9.dcc_independent_64b_blocks);
      tiling_flags |= AMDGPU_TILING_SET(DCC_INDEPENDENT_128B, md->u.gfx9.dcc_independent_128b_blocks);
      tiling_flags |= AMDGPU_TILING_SET(DCC_MAX_COMPRESSED_BLOCK_SIZE, md->u.gfx9.dcc_max_compressed_block_size);
      tiling_flags |= AMDGPU_TILING_SET(SCANOUT, md->u.gfx9.scanout);
   } else {
      if (md->u.legacy.macrotile == RADEON_LAYOUT_TILED)
         tiling_flags |= AMDGPU_TILING_SET(ARRAY_MODE, 4); /* 2D_TILED_THIN1 */
      else if (md->u.legacy.microtile == RADEON_LAYOUT_TILED)
         tiling_flags |= AMDGPU_TILING_SET(ARRAY_MODE, 2); /* 1D_TILED_THIN1 */
      else
         tiling_flags |= AMDGPU_TILING_SET(ARRAY_MODE, 1); /* LINEAR_ALIGNED */

      tiling_flags |= AMDGPU_TILING_SET(PIPE_CONFIG, md->u.legacy.pipe_config);
      tiling_flags |= AMDGPU_TILING_SET(BANK_WIDTH, util_logbase2(md->u.legacy.bankw));
      tiling_flags |= AMDGPU_TILING_SET(BANK_HEIGHT, util_logbase2(md->u.legacy.bankh));
      if (md->u.legacy.tile_split)
         tiling_flags |= AMDGPU_TILING_SET(TILE_SPLIT, radv_eg_tile_split_rev(md->u.legacy.tile_split));
      tiling_flags |= AMDGPU_TILING_SET(MACRO_TILE_ASPECT, util_logbase2(md->u.legacy.mtilea));
      tiling_flags |= AMDGPU_TILING_SET(NUM_BANKS, util_logbase2(md->u.legacy.num_banks) - 1);

      if (md->u.legacy.scanout)
         tiling_flags |= AMDGPU_TILING_SET(MICRO_TILE_MODE, 0); /* DISPLAY_MICRO_TILING */
      else
         tiling_flags |= AMDGPU_TILING_SET(MICRO_TILE_MODE, 1); /* THIN_MICRO_TILING */
   }

   metadata.tiling_info = tiling_flags;
   metadata.size_metadata = md->size_metadata;
   memcpy(metadata.umd_metadata, md->metadata, sizeof(md->metadata));

   amdgpu_bo_set_metadata(bo->bo, &metadata);
}

static void
radv_amdgpu_winsys_bo_get_metadata(struct radeon_winsys *_ws, struct radeon_winsys_bo *_bo,
                                   struct radeon_bo_metadata *md)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct radv_amdgpu_winsys_bo *bo = radv_amdgpu_winsys_bo(_bo);
   struct amdgpu_bo_info info = {0};

   int r = amdgpu_bo_query_info(bo->bo, &info);
   if (r)
      return;

   uint64_t tiling_flags = info.metadata.tiling_info;

   if (ws->info.gfx_level >= GFX9) {
      md->u.gfx9.swizzle_mode = AMDGPU_TILING_GET(tiling_flags, SWIZZLE_MODE);
      md->u.gfx9.scanout = AMDGPU_TILING_GET(tiling_flags, SCANOUT);
   } else {
      md->u.legacy.microtile = RADEON_LAYOUT_LINEAR;
      md->u.legacy.macrotile = RADEON_LAYOUT_LINEAR;

      if (AMDGPU_TILING_GET(tiling_flags, ARRAY_MODE) == 4) /* 2D_TILED_THIN1 */
         md->u.legacy.macrotile = RADEON_LAYOUT_TILED;
      else if (AMDGPU_TILING_GET(tiling_flags, ARRAY_MODE) == 2) /* 1D_TILED_THIN1 */
         md->u.legacy.microtile = RADEON_LAYOUT_TILED;

      md->u.legacy.pipe_config = AMDGPU_TILING_GET(tiling_flags, PIPE_CONFIG);
      md->u.legacy.bankw = 1 << AMDGPU_TILING_GET(tiling_flags, BANK_WIDTH);
      md->u.legacy.bankh = 1 << AMDGPU_TILING_GET(tiling_flags, BANK_HEIGHT);
      md->u.legacy.tile_split = eg_tile_split(AMDGPU_TILING_GET(tiling_flags, TILE_SPLIT));
      md->u.legacy.mtilea = 1 << AMDGPU_TILING_GET(tiling_flags, MACRO_TILE_ASPECT);
      md->u.legacy.num_banks = 2 << AMDGPU_TILING_GET(tiling_flags, NUM_BANKS);
      md->u.legacy.scanout = AMDGPU_TILING_GET(tiling_flags, MICRO_TILE_MODE) == 0; /* DISPLAY */
   }

   md->size_metadata = info.metadata.size_metadata;
   memcpy(md->metadata, info.metadata.umd_metadata, sizeof(md->metadata));
}

static VkResult
radv_amdgpu_winsys_bo_make_resident(struct radeon_winsys *_ws, struct radeon_winsys_bo *_bo, bool resident)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct radv_amdgpu_winsys_bo *bo = radv_amdgpu_winsys_bo(_bo);
   VkResult result = VK_SUCCESS;

   /* Do not add the BO to the global list if it's a local BO because the
    * kernel maintains a list for us.
    */
   if (bo->base.is_local)
      return VK_SUCCESS;

   /* Do not add the BO twice to the global list if the allbos debug
    * option is enabled.
    */
   if (ws->debug_all_bos)
      return VK_SUCCESS;

   if (resident) {
      result = radv_amdgpu_global_bo_list_add(ws, bo);
   } else {
      radv_amdgpu_global_bo_list_del(ws, bo);
   }

   return result;
}

static int
radv_amdgpu_bo_va_compare(const void *a, const void *b)
{
   const struct radv_amdgpu_winsys_bo *bo_a = *(const struct radv_amdgpu_winsys_bo *const *)a;
   const struct radv_amdgpu_winsys_bo *bo_b = *(const struct radv_amdgpu_winsys_bo *const *)b;
   return bo_a->base.va < bo_b->base.va ? -1 : bo_a->base.va > bo_b->base.va ? 1 : 0;
}

static uint64_t
radv_amdgpu_canonicalize_va(uint64_t va)
{
   /* Would be less hardcoded to use addr32_hi (0xffff8000) to generate a mask,
    * but there are confusing differences between page fault reports from kernel where
    * it seems to report the top 48 bits, where addr32_hi has 47-bits. */
   return va & ((1ull << 48) - 1);
}

static void
radv_amdgpu_dump_bo_log(struct radeon_winsys *_ws, FILE *file)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   struct radv_amdgpu_winsys_bo_log *bo_log;

   if (!ws->debug_log_bos)
      return;

   u_rwlock_rdlock(&ws->log_bo_list_lock);
   LIST_FOR_EACH_ENTRY (bo_log, &ws->log_bo_list, list) {
      fprintf(file, "timestamp=%llu, VA=%.16llx-%.16llx, destroyed=%d, is_virtual=%d\n", (long long)bo_log->timestamp,
              (long long)radv_amdgpu_canonicalize_va(bo_log->va),
              (long long)radv_amdgpu_canonicalize_va(bo_log->va + bo_log->size), bo_log->destroyed, bo_log->is_virtual);
   }
   u_rwlock_rdunlock(&ws->log_bo_list_lock);
}

static void
radv_amdgpu_dump_bo_ranges(struct radeon_winsys *_ws, FILE *file)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   if (ws->debug_all_bos) {
      struct radv_amdgpu_winsys_bo **bos = NULL;
      int i = 0;

      u_rwlock_rdlock(&ws->global_bo_list.lock);
      bos = malloc(sizeof(*bos) * ws->global_bo_list.count);
      if (!bos) {
         u_rwlock_rdunlock(&ws->global_bo_list.lock);
         fprintf(file, "  Failed to allocate memory to sort VA ranges for dumping\n");
         return;
      }

      for (i = 0; i < ws->global_bo_list.count; i++) {
         bos[i] = ws->global_bo_list.bos[i];
      }
      qsort(bos, ws->global_bo_list.count, sizeof(bos[0]), radv_amdgpu_bo_va_compare);

      for (i = 0; i < ws->global_bo_list.count; ++i) {
         fprintf(file, "  VA=%.16llx-%.16llx, handle=%d\n", (long long)radv_amdgpu_canonicalize_va(bos[i]->base.va),
                 (long long)radv_amdgpu_canonicalize_va(bos[i]->base.va + bos[i]->size), bos[i]->bo_handle);
      }
      free(bos);
      u_rwlock_rdunlock(&ws->global_bo_list.lock);
   } else
      fprintf(file, "  To get BO VA ranges, please specify RADV_DEBUG=allbos\n");
}
void
radv_amdgpu_bo_init_functions(struct radv_amdgpu_winsys *ws)
{
   ws->base.buffer_create = radv_amdgpu_winsys_bo_create;
   ws->base.buffer_destroy = radv_amdgpu_winsys_bo_destroy;
   ws->base.buffer_map = radv_amdgpu_winsys_bo_map;
   ws->base.buffer_unmap = radv_amdgpu_winsys_bo_unmap;
   ws->base.buffer_from_ptr = radv_amdgpu_winsys_bo_from_ptr;
   ws->base.buffer_from_fd = radv_amdgpu_winsys_bo_from_fd;
   ws->base.buffer_get_fd = radv_amdgpu_winsys_get_fd;
   ws->base.buffer_set_metadata = radv_amdgpu_winsys_bo_set_metadata;
   ws->base.buffer_get_metadata = radv_amdgpu_winsys_bo_get_metadata;
   ws->base.buffer_virtual_bind = radv_amdgpu_winsys_bo_virtual_bind;
   ws->base.buffer_get_flags_from_fd = radv_amdgpu_bo_get_flags_from_fd;
   ws->base.buffer_make_resident = radv_amdgpu_winsys_bo_make_resident;
   ws->base.dump_bo_ranges = radv_amdgpu_dump_bo_ranges;
   ws->base.dump_bo_log = radv_amdgpu_dump_bo_log;
}
