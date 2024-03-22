/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "radeon_drm_cs.h"

#include "util/u_hash_table.h"
#include "util/u_memory.h"
#include "util/u_thread.h"
#include "util/os_mman.h"
#include "util/os_time.h"

#include "frontend/drm_driver.h"

#include <sys/ioctl.h>
#include <xf86drm.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>

static struct pb_buffer_lean *
radeon_winsys_bo_create(struct radeon_winsys *rws,
                        uint64_t size,
                        unsigned alignment,
                        enum radeon_bo_domain domain,
                        enum radeon_bo_flag flags);

static inline struct radeon_bo *radeon_bo(struct pb_buffer_lean *bo)
{
   return (struct radeon_bo *)bo;
}

struct radeon_bo_va_hole {
   struct list_head list;
   uint64_t         offset;
   uint64_t         size;
};

static bool radeon_real_bo_is_busy(struct radeon_bo *bo)
{
   struct drm_radeon_gem_busy args = {0};

   args.handle = bo->handle;
   return drmCommandWriteRead(bo->rws->fd, DRM_RADEON_GEM_BUSY,
                              &args, sizeof(args)) != 0;
}

static bool radeon_bo_is_busy(struct radeon_winsys *rws, struct radeon_bo *bo)
{
   unsigned num_idle;
   bool busy = false;

   if (bo->handle)
      return radeon_real_bo_is_busy(bo);

   mtx_lock(&bo->rws->bo_fence_lock);
   for (num_idle = 0; num_idle < bo->u.slab.num_fences; ++num_idle) {
      if (radeon_real_bo_is_busy(bo->u.slab.fences[num_idle])) {
         busy = true;
         break;
      }
      radeon_ws_bo_reference(rws, &bo->u.slab.fences[num_idle], NULL);
   }
   memmove(&bo->u.slab.fences[0], &bo->u.slab.fences[num_idle],
         (bo->u.slab.num_fences - num_idle) * sizeof(bo->u.slab.fences[0]));
   bo->u.slab.num_fences -= num_idle;
   mtx_unlock(&bo->rws->bo_fence_lock);

   return busy;
}

static void radeon_real_bo_wait_idle(struct radeon_bo *bo)
{
   struct drm_radeon_gem_wait_idle args = {0};

   args.handle = bo->handle;
   while (drmCommandWrite(bo->rws->fd, DRM_RADEON_GEM_WAIT_IDLE,
                          &args, sizeof(args)) == -EBUSY);
}

static void radeon_bo_wait_idle(struct radeon_winsys *rws, struct radeon_bo *bo)
{
   if (bo->handle) {
      radeon_real_bo_wait_idle(bo);
   } else {
      mtx_lock(&bo->rws->bo_fence_lock);
      while (bo->u.slab.num_fences) {
         struct radeon_bo *fence = NULL;
         radeon_ws_bo_reference(rws, &fence, bo->u.slab.fences[0]);
         mtx_unlock(&bo->rws->bo_fence_lock);

         /* Wait without holding the fence lock. */
         radeon_real_bo_wait_idle(fence);

         mtx_lock(&bo->rws->bo_fence_lock);
         if (bo->u.slab.num_fences && fence == bo->u.slab.fences[0]) {
            radeon_ws_bo_reference(rws, &bo->u.slab.fences[0], NULL);
            memmove(&bo->u.slab.fences[0], &bo->u.slab.fences[1],
                  (bo->u.slab.num_fences - 1) * sizeof(bo->u.slab.fences[0]));
            bo->u.slab.num_fences--;
         }
         radeon_ws_bo_reference(rws, &fence, NULL);
      }
      mtx_unlock(&bo->rws->bo_fence_lock);
   }
}

static bool radeon_bo_wait(struct radeon_winsys *rws,
                           struct pb_buffer_lean *_buf, uint64_t timeout,
                           unsigned usage)
{
   struct radeon_bo *bo = radeon_bo(_buf);
   int64_t abs_timeout;

   /* No timeout. Just query. */
   if (timeout == 0)
      return !bo->num_active_ioctls && !radeon_bo_is_busy(rws, bo);

   abs_timeout = os_time_get_absolute_timeout(timeout);

   /* Wait if any ioctl is being submitted with this buffer. */
   if (!os_wait_until_zero_abs_timeout(&bo->num_active_ioctls, abs_timeout))
      return false;

   /* Infinite timeout. */
   if (abs_timeout == OS_TIMEOUT_INFINITE) {
      radeon_bo_wait_idle(rws, bo);
      return true;
   }

   /* Other timeouts need to be emulated with a loop. */
   while (radeon_bo_is_busy(rws, bo)) {
      if (os_time_get_nano() >= abs_timeout)
         return false;
      os_time_sleep(10);
   }

   return true;
}

static enum radeon_bo_domain get_valid_domain(enum radeon_bo_domain domain)
{
   /* Zero domains the driver doesn't understand. */
   domain &= RADEON_DOMAIN_VRAM_GTT;

   /* If no domain is set, we must set something... */
   if (!domain)
      domain = RADEON_DOMAIN_VRAM_GTT;

   return domain;
}

static enum radeon_bo_domain radeon_bo_get_initial_domain(
      struct pb_buffer_lean *buf)
{
   struct radeon_bo *bo = (struct radeon_bo*)buf;
   struct drm_radeon_gem_op args;

   memset(&args, 0, sizeof(args));
   args.handle = bo->handle;
   args.op = RADEON_GEM_OP_GET_INITIAL_DOMAIN;

   if (drmCommandWriteRead(bo->rws->fd, DRM_RADEON_GEM_OP,
                           &args, sizeof(args))) {
      fprintf(stderr, "radeon: failed to get initial domain: %p 0x%08X\n",
              bo, bo->handle);
      /* Default domain as returned by get_valid_domain. */
      return RADEON_DOMAIN_VRAM_GTT;
   }

   /* GEM domains and winsys domains are defined the same. */
   return get_valid_domain(args.value);
}

static uint64_t radeon_bomgr_find_va(const struct radeon_info *info,
                                     struct radeon_vm_heap *heap,
                                     uint64_t size, uint64_t alignment)
{
   struct radeon_bo_va_hole *hole, *n;
   uint64_t offset = 0, waste = 0;

   /* All VM address space holes will implicitly start aligned to the
    * size alignment, so we don't need to sanitize the alignment here
    */
   size = align(size, info->gart_page_size);

   mtx_lock(&heap->mutex);
   /* first look for a hole */
   LIST_FOR_EACH_ENTRY_SAFE(hole, n, &heap->holes, list) {
      offset = hole->offset;
      waste = offset % alignment;
      waste = waste ? alignment - waste : 0;
      offset += waste;
      if (offset >= (hole->offset + hole->size)) {
         continue;
      }
      if (!waste && hole->size == size) {
         offset = hole->offset;
         list_del(&hole->list);
         FREE(hole);
         mtx_unlock(&heap->mutex);
         return offset;
      }
      if ((hole->size - waste) > size) {
         if (waste) {
            n = CALLOC_STRUCT(radeon_bo_va_hole);
            n->size = waste;
            n->offset = hole->offset;
            list_add(&n->list, &hole->list);
         }
         hole->size -= (size + waste);
         hole->offset += size + waste;
         mtx_unlock(&heap->mutex);
         return offset;
      }
      if ((hole->size - waste) == size) {
         hole->size = waste;
         mtx_unlock(&heap->mutex);
         return offset;
      }
   }

   offset = heap->start;
   waste = offset % alignment;
   waste = waste ? alignment - waste : 0;

   if (offset + waste + size > heap->end) {
      mtx_unlock(&heap->mutex);
      return 0;
   }

   if (waste) {
      n = CALLOC_STRUCT(radeon_bo_va_hole);
      n->size = waste;
      n->offset = offset;
      list_add(&n->list, &heap->holes);
   }
   offset += waste;
   heap->start += size + waste;
   mtx_unlock(&heap->mutex);
   return offset;
}

static uint64_t radeon_bomgr_find_va64(struct radeon_drm_winsys *ws,
                                       uint64_t size, uint64_t alignment)
{
   uint64_t va = 0;

   /* Try to allocate from the 64-bit address space first.
    * If it doesn't exist (start = 0) or if it doesn't have enough space,
    * fall back to the 32-bit address space.
    */
   if (ws->vm64.start)
      va = radeon_bomgr_find_va(&ws->info, &ws->vm64, size, alignment);
   if (!va)
      va = radeon_bomgr_find_va(&ws->info, &ws->vm32, size, alignment);
   return va;
}

static void radeon_bomgr_free_va(const struct radeon_info *info,
                                 struct radeon_vm_heap *heap,
                                 uint64_t va, uint64_t size)
{
   struct radeon_bo_va_hole *hole = NULL;

   size = align(size, info->gart_page_size);

   mtx_lock(&heap->mutex);
   if ((va + size) == heap->start) {
      heap->start = va;
      /* Delete uppermost hole if it reaches the new top */
      if (!list_is_empty(&heap->holes)) {
         hole = container_of(heap->holes.next, struct radeon_bo_va_hole, list);
         if ((hole->offset + hole->size) == va) {
            heap->start = hole->offset;
            list_del(&hole->list);
            FREE(hole);
         }
      }
   } else {
      struct radeon_bo_va_hole *next;

      hole = container_of(&heap->holes, struct radeon_bo_va_hole, list);
      LIST_FOR_EACH_ENTRY(next, &heap->holes, list) {
         if (next->offset < va)
            break;
         hole = next;
      }

      if (&hole->list != &heap->holes) {
         /* Grow upper hole if it's adjacent */
         if (hole->offset == (va + size)) {
            hole->offset = va;
            hole->size += size;
            /* Merge lower hole if it's adjacent */
            if (next != hole && &next->list != &heap->holes &&
                (next->offset + next->size) == va) {
               next->size += hole->size;
               list_del(&hole->list);
               FREE(hole);
            }
            goto out;
         }
      }

      /* Grow lower hole if it's adjacent */
      if (next != hole && &next->list != &heap->holes &&
          (next->offset + next->size) == va) {
         next->size += size;
         goto out;
      }

      /* FIXME on allocation failure we just lose virtual address space
       * maybe print a warning
       */
      next = CALLOC_STRUCT(radeon_bo_va_hole);
      if (next) {
         next->size = size;
         next->offset = va;
         list_add(&next->list, &hole->list);
      }
   }
out:
   mtx_unlock(&heap->mutex);
}

void radeon_bo_destroy(void *winsys, struct pb_buffer_lean *_buf)
{
   struct radeon_bo *bo = radeon_bo((struct pb_buffer_lean*)_buf);
   struct radeon_drm_winsys *rws = bo->rws;
   struct drm_gem_close args;

   assert(bo->handle && "must not be called for slab entries");

   memset(&args, 0, sizeof(args));

   mtx_lock(&rws->bo_handles_mutex);
   /* radeon_winsys_bo_from_handle might have revived the bo */
   if (pipe_is_referenced(&bo->base.reference)) {
      mtx_unlock(&rws->bo_handles_mutex);
      return;
   }
   _mesa_hash_table_remove_key(rws->bo_handles, (void*)(uintptr_t)bo->handle);
   if (bo->flink_name) {
      _mesa_hash_table_remove_key(rws->bo_names,
                                  (void*)(uintptr_t)bo->flink_name);
   }
   mtx_unlock(&rws->bo_handles_mutex);

   if (bo->u.real.ptr)
      os_munmap(bo->u.real.ptr, bo->base.size);

   if (rws->info.r600_has_virtual_memory) {
      if (rws->va_unmap_working) {
         struct drm_radeon_gem_va va;

         va.handle = bo->handle;
         va.vm_id = 0;
         va.operation = RADEON_VA_UNMAP;
         va.flags = RADEON_VM_PAGE_READABLE |
                    RADEON_VM_PAGE_WRITEABLE |
                    RADEON_VM_PAGE_SNOOPED;
         va.offset = bo->va;

         if (drmCommandWriteRead(rws->fd, DRM_RADEON_GEM_VA, &va,
                                 sizeof(va)) != 0 &&
             va.operation == RADEON_VA_RESULT_ERROR) {
            fprintf(stderr, "radeon: Failed to deallocate virtual address for buffer:\n");
            fprintf(stderr, "radeon:    size      : %"PRIu64" bytes\n", bo->base.size);
            fprintf(stderr, "radeon:    va        : 0x%"PRIx64"\n", bo->va);
         }
      }

      radeon_bomgr_free_va(&rws->info,
                           bo->va < rws->vm32.end ? &rws->vm32 : &rws->vm64,
                           bo->va, bo->base.size);
   }

   /* Close object. */
   args.handle = bo->handle;
   drmIoctl(rws->fd, DRM_IOCTL_GEM_CLOSE, &args);

   mtx_destroy(&bo->u.real.map_mutex);

   if (bo->initial_domain & RADEON_DOMAIN_VRAM)
      rws->allocated_vram -= align(bo->base.size, rws->info.gart_page_size);
   else if (bo->initial_domain & RADEON_DOMAIN_GTT)
      rws->allocated_gtt -= align(bo->base.size, rws->info.gart_page_size);

   if (bo->u.real.map_count >= 1) {
      if (bo->initial_domain & RADEON_DOMAIN_VRAM)
         bo->rws->mapped_vram -= bo->base.size;
      else
         bo->rws->mapped_gtt -= bo->base.size;
      bo->rws->num_mapped_buffers--;
   }

   FREE(bo);
}

static void radeon_bo_destroy_or_cache(void *winsys, struct pb_buffer_lean *_buf)
{
   struct radeon_drm_winsys *rws = (struct radeon_drm_winsys *)winsys;
   struct radeon_bo *bo = radeon_bo(_buf);

   assert(bo->handle && "must not be called for slab entries");

   if (bo->u.real.use_reusable_pool)
      pb_cache_add_buffer(&rws->bo_cache, &bo->u.real.cache_entry);
   else
      radeon_bo_destroy(NULL, _buf);
}

void *radeon_bo_do_map(struct radeon_bo *bo)
{
   struct drm_radeon_gem_mmap args = {0};
   void *ptr;
   unsigned offset;

   /* If the buffer is created from user memory, return the user pointer. */
   if (bo->user_ptr)
      return bo->user_ptr;

   if (bo->handle) {
      offset = 0;
   } else {
      offset = bo->va - bo->u.slab.real->va;
      bo = bo->u.slab.real;
   }

   /* Map the buffer. */
   mtx_lock(&bo->u.real.map_mutex);
   /* Return the pointer if it's already mapped. */
   if (bo->u.real.ptr) {
      bo->u.real.map_count++;
      mtx_unlock(&bo->u.real.map_mutex);
      return (uint8_t*)bo->u.real.ptr + offset;
   }
   args.handle = bo->handle;
   args.offset = 0;
   args.size = (uint64_t)bo->base.size;
   if (drmCommandWriteRead(bo->rws->fd,
                           DRM_RADEON_GEM_MMAP,
                           &args,
                           sizeof(args))) {
      mtx_unlock(&bo->u.real.map_mutex);
      fprintf(stderr, "radeon: gem_mmap failed: %p 0x%08X\n",
              bo, bo->handle);
      return NULL;
   }

   ptr = os_mmap(0, args.size, PROT_READ|PROT_WRITE, MAP_SHARED,
                 bo->rws->fd, args.addr_ptr);
   if (ptr == MAP_FAILED) {
      /* Clear the cache and try again. */
      pb_cache_release_all_buffers(&bo->rws->bo_cache);

      ptr = os_mmap(0, args.size, PROT_READ|PROT_WRITE, MAP_SHARED,
                    bo->rws->fd, args.addr_ptr);
      if (ptr == MAP_FAILED) {
         mtx_unlock(&bo->u.real.map_mutex);
         fprintf(stderr, "radeon: mmap failed, errno: %i\n", errno);
         return NULL;
      }
   }
   bo->u.real.ptr = ptr;
   bo->u.real.map_count = 1;

   if (bo->initial_domain & RADEON_DOMAIN_VRAM)
      bo->rws->mapped_vram += bo->base.size;
   else
      bo->rws->mapped_gtt += bo->base.size;
   bo->rws->num_mapped_buffers++;

   mtx_unlock(&bo->u.real.map_mutex);
   return (uint8_t*)bo->u.real.ptr + offset;
}

static void *radeon_bo_map(struct radeon_winsys *rws,
                           struct pb_buffer_lean *buf,
                           struct radeon_cmdbuf *rcs,
                           enum pipe_map_flags usage)
{
   struct radeon_bo *bo = (struct radeon_bo*)buf;
   struct radeon_drm_cs *cs = rcs ? radeon_drm_cs(rcs) : NULL;

   /* If it's not unsynchronized bo_map, flush CS if needed and then wait. */
   if (!(usage & PIPE_MAP_UNSYNCHRONIZED)) {
      /* DONTBLOCK doesn't make sense with UNSYNCHRONIZED. */
      if (usage & PIPE_MAP_DONTBLOCK) {
         if (!(usage & PIPE_MAP_WRITE)) {
            /* Mapping for read.
             *
             * Since we are mapping for read, we don't need to wait
             * if the GPU is using the buffer for read too
             * (neither one is changing it).
             *
             * Only check whether the buffer is being used for write. */
            if (cs && radeon_bo_is_referenced_by_cs_for_write(cs, bo)) {
               cs->flush_cs(cs->flush_data,
                            RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW, NULL);
               return NULL;
            }

            if (!radeon_bo_wait(rws, (struct pb_buffer_lean*)bo, 0,
                                RADEON_USAGE_WRITE)) {
               return NULL;
            }
         } else {
            if (cs && radeon_bo_is_referenced_by_cs(cs, bo)) {
               cs->flush_cs(cs->flush_data,
                            RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW, NULL);
               return NULL;
            }

            if (!radeon_bo_wait(rws, (struct pb_buffer_lean*)bo, 0,
                                RADEON_USAGE_READWRITE)) {
               return NULL;
            }
         }
      } else {
         uint64_t time = os_time_get_nano();

         if (!(usage & PIPE_MAP_WRITE)) {
            /* Mapping for read.
             *
             * Since we are mapping for read, we don't need to wait
             * if the GPU is using the buffer for read too
             * (neither one is changing it).
             *
             * Only check whether the buffer is being used for write. */
            if (cs && radeon_bo_is_referenced_by_cs_for_write(cs, bo)) {
               cs->flush_cs(cs->flush_data,
                            RADEON_FLUSH_START_NEXT_GFX_IB_NOW, NULL);
            }
            radeon_bo_wait(rws, (struct pb_buffer_lean*)bo, OS_TIMEOUT_INFINITE,
                           RADEON_USAGE_WRITE);
         } else {
            /* Mapping for write. */
            if (cs) {
               if (radeon_bo_is_referenced_by_cs(cs, bo)) {
                  cs->flush_cs(cs->flush_data,
                               RADEON_FLUSH_START_NEXT_GFX_IB_NOW, NULL);
               } else {
                  /* Try to avoid busy-waiting in radeon_bo_wait. */
                  if (p_atomic_read(&bo->num_active_ioctls))
                     radeon_drm_cs_sync_flush(rcs);
               }
            }

            radeon_bo_wait(rws, (struct pb_buffer_lean*)bo, OS_TIMEOUT_INFINITE,
                           RADEON_USAGE_READWRITE);
         }

         bo->rws->buffer_wait_time += os_time_get_nano() - time;
      }
   }

   return radeon_bo_do_map(bo);
}

static void radeon_bo_unmap(struct radeon_winsys *rws, struct pb_buffer_lean *_buf)
{
   struct radeon_bo *bo = (struct radeon_bo*)_buf;

   if (bo->user_ptr)
      return;

   if (!bo->handle)
      bo = bo->u.slab.real;

   mtx_lock(&bo->u.real.map_mutex);
   if (!bo->u.real.ptr) {
      mtx_unlock(&bo->u.real.map_mutex);
      return; /* it's not been mapped */
   }

   assert(bo->u.real.map_count);
   if (--bo->u.real.map_count) {
      mtx_unlock(&bo->u.real.map_mutex);
      return; /* it's been mapped multiple times */
   }

   os_munmap(bo->u.real.ptr, bo->base.size);
   bo->u.real.ptr = NULL;

   if (bo->initial_domain & RADEON_DOMAIN_VRAM)
      bo->rws->mapped_vram -= bo->base.size;
   else
      bo->rws->mapped_gtt -= bo->base.size;
   bo->rws->num_mapped_buffers--;

   mtx_unlock(&bo->u.real.map_mutex);
}

static struct radeon_bo *radeon_create_bo(struct radeon_drm_winsys *rws,
                                          unsigned size, unsigned alignment,
                                          unsigned initial_domains,
                                          unsigned flags,
                                          int heap)
{
   struct radeon_bo *bo;
   struct drm_radeon_gem_create args;
   int r;

   memset(&args, 0, sizeof(args));

   assert(initial_domains);
   assert((initial_domains &
           ~(RADEON_GEM_DOMAIN_GTT | RADEON_GEM_DOMAIN_VRAM)) == 0);

   args.size = size;
   args.alignment = alignment;
   args.initial_domain = initial_domains;
   args.flags = 0;

   /* If VRAM is just stolen system memory, allow both VRAM and
    * GTT, whichever has free space. If a buffer is evicted from
    * VRAM to GTT, it will stay there.
    */
   if (!rws->info.has_dedicated_vram)
      args.initial_domain |= RADEON_DOMAIN_GTT;

   if (flags & RADEON_FLAG_GTT_WC)
      args.flags |= RADEON_GEM_GTT_WC;
   if (flags & RADEON_FLAG_NO_CPU_ACCESS)
      args.flags |= RADEON_GEM_NO_CPU_ACCESS;

   if (drmCommandWriteRead(rws->fd, DRM_RADEON_GEM_CREATE,
                           &args, sizeof(args))) {
      fprintf(stderr, "radeon: Failed to allocate a buffer:\n");
      fprintf(stderr, "radeon:    size      : %u bytes\n", size);
      fprintf(stderr, "radeon:    alignment : %u bytes\n", alignment);
      fprintf(stderr, "radeon:    domains   : %u\n", args.initial_domain);
      fprintf(stderr, "radeon:    flags     : %u\n", args.flags);
      return NULL;
   }

   assert(args.handle != 0);

   bo = CALLOC_STRUCT(radeon_bo);
   if (!bo)
      return NULL;

   pipe_reference_init(&bo->base.reference, 1);
   bo->base.alignment_log2 = util_logbase2(alignment);
   bo->base.usage = 0;
   bo->base.size = size;
   bo->rws = rws;
   bo->handle = args.handle;
   bo->va = 0;
   bo->initial_domain = initial_domains;
   bo->hash = __sync_fetch_and_add(&rws->next_bo_hash, 1);
   (void) mtx_init(&bo->u.real.map_mutex, mtx_plain);

   if (heap >= 0) {
      pb_cache_init_entry(&rws->bo_cache, &bo->u.real.cache_entry, &bo->base,
                          heap);
   }

   if (rws->info.r600_has_virtual_memory) {
      struct drm_radeon_gem_va va;
      unsigned va_gap_size;

      va_gap_size = rws->check_vm ? MAX2(4 * alignment, 64 * 1024) : 0;

      if (flags & RADEON_FLAG_32BIT) {
         bo->va = radeon_bomgr_find_va(&rws->info, &rws->vm32,
                                       size + va_gap_size, alignment);
         assert(bo->va + size < rws->vm32.end);
      } else {
         bo->va = radeon_bomgr_find_va64(rws, size + va_gap_size, alignment);
      }

      va.handle = bo->handle;
      va.vm_id = 0;
      va.operation = RADEON_VA_MAP;
      va.flags = RADEON_VM_PAGE_READABLE |
                 RADEON_VM_PAGE_WRITEABLE |
                 RADEON_VM_PAGE_SNOOPED;
      va.offset = bo->va;
      r = drmCommandWriteRead(rws->fd, DRM_RADEON_GEM_VA, &va, sizeof(va));
      if (r && va.operation == RADEON_VA_RESULT_ERROR) {
         fprintf(stderr, "radeon: Failed to allocate virtual address for buffer:\n");
         fprintf(stderr, "radeon:    size      : %d bytes\n", size);
         fprintf(stderr, "radeon:    alignment : %d bytes\n", alignment);
         fprintf(stderr, "radeon:    domains   : %d\n", args.initial_domain);
         fprintf(stderr, "radeon:    va        : 0x%016llx\n", (unsigned long long)bo->va);
         radeon_bo_destroy(NULL, &bo->base);
         return NULL;
      }
      mtx_lock(&rws->bo_handles_mutex);
      if (va.operation == RADEON_VA_RESULT_VA_EXIST) {
         struct pb_buffer_lean *b = &bo->base;
         struct radeon_bo *old_bo =
               _mesa_hash_table_u64_search(rws->bo_vas, va.offset);

         mtx_unlock(&rws->bo_handles_mutex);
         radeon_bo_reference(&rws->base, &b, &old_bo->base);
         return radeon_bo(b);
      }

      _mesa_hash_table_u64_insert(rws->bo_vas, bo->va, bo);
      mtx_unlock(&rws->bo_handles_mutex);
   }

   if (initial_domains & RADEON_DOMAIN_VRAM)
      rws->allocated_vram += align(size, rws->info.gart_page_size);
   else if (initial_domains & RADEON_DOMAIN_GTT)
      rws->allocated_gtt += align(size, rws->info.gart_page_size);

   return bo;
}

bool radeon_bo_can_reclaim(void *winsys, struct pb_buffer_lean *_buf)
{
   struct radeon_bo *bo = radeon_bo((struct pb_buffer_lean*)_buf);

   if (radeon_bo_is_referenced_by_any_cs(bo))
      return false;

   return radeon_bo_wait(winsys, (struct pb_buffer_lean*)_buf, 0, RADEON_USAGE_READWRITE);
}

bool radeon_bo_can_reclaim_slab(void *priv, struct pb_slab_entry *entry)
{
   struct radeon_bo *bo = container_of(entry, struct radeon_bo, u.slab.entry);

   return radeon_bo_can_reclaim(NULL, &bo->base);
}

static void radeon_bo_slab_destroy(void *winsys, struct pb_buffer_lean *_buf)
{
   struct radeon_bo *bo = radeon_bo(_buf);

   assert(!bo->handle);

   pb_slab_free(&bo->rws->bo_slabs, &bo->u.slab.entry);
}

struct pb_slab *radeon_bo_slab_alloc(void *priv, unsigned heap,
                                     unsigned entry_size,
                                     unsigned group_index)
{
   struct radeon_drm_winsys *ws = priv;
   struct radeon_slab *slab = CALLOC_STRUCT(radeon_slab);
   enum radeon_bo_domain domains = radeon_domain_from_heap(heap);
   enum radeon_bo_flag flags = radeon_flags_from_heap(heap);
   unsigned base_hash;

   if (!slab)
      return NULL;

   slab->buffer = radeon_bo(radeon_winsys_bo_create(&ws->base,
                                                    64 * 1024, 64 * 1024,
                                                    domains, flags));
   if (!slab->buffer)
      goto fail;

   assert(slab->buffer->handle);

   slab->base.num_entries = slab->buffer->base.size / entry_size;
   slab->base.num_free = slab->base.num_entries;
   slab->base.group_index = group_index;
   slab->base.entry_size = entry_size;
   slab->entries = CALLOC(slab->base.num_entries, sizeof(*slab->entries));
   if (!slab->entries)
      goto fail_buffer;

   list_inithead(&slab->base.free);

   base_hash = __sync_fetch_and_add(&ws->next_bo_hash, slab->base.num_entries);

   for (unsigned i = 0; i < slab->base.num_entries; ++i) {
      struct radeon_bo *bo = &slab->entries[i];

      bo->base.alignment_log2 = util_logbase2(entry_size);
      bo->base.usage = slab->buffer->base.usage;
      bo->base.size = entry_size;
      bo->rws = ws;
      bo->va = slab->buffer->va + i * entry_size;
      bo->initial_domain = domains;
      bo->hash = base_hash + i;
      bo->u.slab.entry.slab = &slab->base;
      bo->u.slab.real = slab->buffer;

      list_addtail(&bo->u.slab.entry.head, &slab->base.free);
   }

   return &slab->base;

fail_buffer:
   radeon_ws_bo_reference(&ws->base, &slab->buffer, NULL);
fail:
   FREE(slab);
   return NULL;
}

void radeon_bo_slab_free(void *priv, struct pb_slab *pslab)
{
   struct radeon_winsys *rws = (struct radeon_winsys *)priv;
   struct radeon_slab *slab = (struct radeon_slab *)pslab;

   for (unsigned i = 0; i < slab->base.num_entries; ++i) {
      struct radeon_bo *bo = &slab->entries[i];
      for (unsigned j = 0; j < bo->u.slab.num_fences; ++j)
         radeon_ws_bo_reference(rws, &bo->u.slab.fences[j], NULL);
      FREE(bo->u.slab.fences);
   }

   FREE(slab->entries);
   radeon_ws_bo_reference(rws, &slab->buffer, NULL);
   FREE(slab);
}

static unsigned eg_tile_split(unsigned tile_split)
{
   switch (tile_split) {
   case 0:     tile_split = 64;    break;
   case 1:     tile_split = 128;   break;
   case 2:     tile_split = 256;   break;
   case 3:     tile_split = 512;   break;
   default:
   case 4:     tile_split = 1024;  break;
   case 5:     tile_split = 2048;  break;
   case 6:     tile_split = 4096;  break;
   }
   return tile_split;
}

static unsigned eg_tile_split_rev(unsigned eg_tile_split)
{
   switch (eg_tile_split) {
   case 64:    return 0;
   case 128:   return 1;
   case 256:   return 2;
   case 512:   return 3;
   default:
   case 1024:  return 4;
   case 2048:  return 5;
   case 4096:  return 6;
   }
}

static void radeon_bo_get_metadata(struct radeon_winsys *rws,
                                   struct pb_buffer_lean *_buf,
                                   struct radeon_bo_metadata *md,
                                   struct radeon_surf *surf)
{
   struct radeon_bo *bo = radeon_bo(_buf);
   struct drm_radeon_gem_set_tiling args;

   assert(bo->handle && "must not be called for slab entries");

   memset(&args, 0, sizeof(args));

   args.handle = bo->handle;

   drmCommandWriteRead(bo->rws->fd,
                       DRM_RADEON_GEM_GET_TILING,
                       &args,
                       sizeof(args));

   if (surf) {
      if (args.tiling_flags & RADEON_TILING_MACRO)
         md->mode = RADEON_SURF_MODE_2D;
      else if (args.tiling_flags & RADEON_TILING_MICRO)
         md->mode = RADEON_SURF_MODE_1D;
      else
         md->mode = RADEON_SURF_MODE_LINEAR_ALIGNED;

      surf->u.legacy.bankw = (args.tiling_flags >> RADEON_TILING_EG_BANKW_SHIFT) & RADEON_TILING_EG_BANKW_MASK;
      surf->u.legacy.bankh = (args.tiling_flags >> RADEON_TILING_EG_BANKH_SHIFT) & RADEON_TILING_EG_BANKH_MASK;
      surf->u.legacy.tile_split = (args.tiling_flags >> RADEON_TILING_EG_TILE_SPLIT_SHIFT) & RADEON_TILING_EG_TILE_SPLIT_MASK;
      surf->u.legacy.tile_split = eg_tile_split(surf->u.legacy.tile_split);
      surf->u.legacy.mtilea = (args.tiling_flags >> RADEON_TILING_EG_MACRO_TILE_ASPECT_SHIFT) & RADEON_TILING_EG_MACRO_TILE_ASPECT_MASK;

      if (bo->rws->gen >= DRV_SI && !(args.tiling_flags & RADEON_TILING_R600_NO_SCANOUT))
         surf->flags |= RADEON_SURF_SCANOUT;
      else
         surf->flags &= ~RADEON_SURF_SCANOUT;
      return;
   }

   md->u.legacy.microtile = RADEON_LAYOUT_LINEAR;
   md->u.legacy.macrotile = RADEON_LAYOUT_LINEAR;
   if (args.tiling_flags & RADEON_TILING_MICRO)
      md->u.legacy.microtile = RADEON_LAYOUT_TILED;
   else if (args.tiling_flags & RADEON_TILING_MICRO_SQUARE)
      md->u.legacy.microtile = RADEON_LAYOUT_SQUARETILED;

   if (args.tiling_flags & RADEON_TILING_MACRO)
      md->u.legacy.macrotile = RADEON_LAYOUT_TILED;

   md->u.legacy.bankw = (args.tiling_flags >> RADEON_TILING_EG_BANKW_SHIFT) & RADEON_TILING_EG_BANKW_MASK;
   md->u.legacy.bankh = (args.tiling_flags >> RADEON_TILING_EG_BANKH_SHIFT) & RADEON_TILING_EG_BANKH_MASK;
   md->u.legacy.tile_split = (args.tiling_flags >> RADEON_TILING_EG_TILE_SPLIT_SHIFT) & RADEON_TILING_EG_TILE_SPLIT_MASK;
   md->u.legacy.mtilea = (args.tiling_flags >> RADEON_TILING_EG_MACRO_TILE_ASPECT_SHIFT) & RADEON_TILING_EG_MACRO_TILE_ASPECT_MASK;
   md->u.legacy.tile_split = eg_tile_split(md->u.legacy.tile_split);
   md->u.legacy.scanout = bo->rws->gen >= DRV_SI && !(args.tiling_flags & RADEON_TILING_R600_NO_SCANOUT);
}

static void radeon_bo_set_metadata(struct radeon_winsys *rws,
                                   struct pb_buffer_lean *_buf,
                                   struct radeon_bo_metadata *md,
                                   struct radeon_surf *surf)
{
   struct radeon_bo *bo = radeon_bo(_buf);
   struct drm_radeon_gem_set_tiling args;

   assert(bo->handle && "must not be called for slab entries");

   memset(&args, 0, sizeof(args));

   os_wait_until_zero(&bo->num_active_ioctls, OS_TIMEOUT_INFINITE);

   if (surf) {
      if (surf->u.legacy.level[0].mode >= RADEON_SURF_MODE_1D)
         args.tiling_flags |= RADEON_TILING_MICRO;
      if (surf->u.legacy.level[0].mode >= RADEON_SURF_MODE_2D)
         args.tiling_flags |= RADEON_TILING_MACRO;

      args.tiling_flags |= (surf->u.legacy.bankw & RADEON_TILING_EG_BANKW_MASK) <<
                           RADEON_TILING_EG_BANKW_SHIFT;
      args.tiling_flags |= (surf->u.legacy.bankh & RADEON_TILING_EG_BANKH_MASK) <<
                           RADEON_TILING_EG_BANKH_SHIFT;
      if (surf->u.legacy.tile_split) {
         args.tiling_flags |= (eg_tile_split_rev(surf->u.legacy.tile_split) &
                               RADEON_TILING_EG_TILE_SPLIT_MASK) <<
                              RADEON_TILING_EG_TILE_SPLIT_SHIFT;
      }
      args.tiling_flags |= (surf->u.legacy.mtilea & RADEON_TILING_EG_MACRO_TILE_ASPECT_MASK) <<
                           RADEON_TILING_EG_MACRO_TILE_ASPECT_SHIFT;

      if (bo->rws->gen >= DRV_SI && !(surf->flags & RADEON_SURF_SCANOUT))
         args.tiling_flags |= RADEON_TILING_R600_NO_SCANOUT;

      args.pitch = surf->u.legacy.level[0].nblk_x * surf->bpe;
   } else {
      if (md->u.legacy.microtile == RADEON_LAYOUT_TILED)
         args.tiling_flags |= RADEON_TILING_MICRO;
      else if (md->u.legacy.microtile == RADEON_LAYOUT_SQUARETILED)
         args.tiling_flags |= RADEON_TILING_MICRO_SQUARE;

      if (md->u.legacy.macrotile == RADEON_LAYOUT_TILED)
         args.tiling_flags |= RADEON_TILING_MACRO;

      args.tiling_flags |= (md->u.legacy.bankw & RADEON_TILING_EG_BANKW_MASK) <<
                           RADEON_TILING_EG_BANKW_SHIFT;
      args.tiling_flags |= (md->u.legacy.bankh & RADEON_TILING_EG_BANKH_MASK) <<
                           RADEON_TILING_EG_BANKH_SHIFT;
      if (md->u.legacy.tile_split) {
         args.tiling_flags |= (eg_tile_split_rev(md->u.legacy.tile_split) &
                               RADEON_TILING_EG_TILE_SPLIT_MASK) <<
                              RADEON_TILING_EG_TILE_SPLIT_SHIFT;
      }
      args.tiling_flags |= (md->u.legacy.mtilea & RADEON_TILING_EG_MACRO_TILE_ASPECT_MASK) <<
                           RADEON_TILING_EG_MACRO_TILE_ASPECT_SHIFT;

      if (bo->rws->gen >= DRV_SI && !md->u.legacy.scanout)
         args.tiling_flags |= RADEON_TILING_R600_NO_SCANOUT;

      args.pitch = md->u.legacy.stride;
   }

   args.handle = bo->handle;

   drmCommandWriteRead(bo->rws->fd,
                       DRM_RADEON_GEM_SET_TILING,
                       &args,
                       sizeof(args));
}

static struct pb_buffer_lean *
radeon_winsys_bo_create(struct radeon_winsys *rws,
                        uint64_t size,
                        unsigned alignment,
                        enum radeon_bo_domain domain,
                        enum radeon_bo_flag flags)
{
   struct radeon_drm_winsys *ws = radeon_drm_winsys(rws);
   struct radeon_bo *bo;

   radeon_canonicalize_bo_flags(&domain, &flags);

   assert(!(flags & RADEON_FLAG_SPARSE)); /* not supported */

   /* Only 32-bit sizes are supported. */
   if (size > UINT_MAX)
      return NULL;

   int heap = radeon_get_heap_index(domain, flags);

   /* Sub-allocate small buffers from slabs. */
   if (heap >= 0 &&
       size <= (1 << RADEON_SLAB_MAX_SIZE_LOG2) &&
       ws->info.r600_has_virtual_memory &&
       alignment <= MAX2(1 << RADEON_SLAB_MIN_SIZE_LOG2, util_next_power_of_two(size))) {
      struct pb_slab_entry *entry;

      entry = pb_slab_alloc(&ws->bo_slabs, size, heap);
      if (!entry) {
         /* Clear the cache and try again. */
         pb_cache_release_all_buffers(&ws->bo_cache);

         entry = pb_slab_alloc(&ws->bo_slabs, size, heap);
      }
      if (!entry)
         return NULL;

      bo = container_of(entry, struct radeon_bo, u.slab.entry);

      pipe_reference_init(&bo->base.reference, 1);

      return &bo->base;
   }

   /* Align size to page size. This is the minimum alignment for normal
    * BOs. Aligning this here helps the cached bufmgr. Especially small BOs,
    * like constant/uniform buffers, can benefit from better and more reuse.
    */
   size = align(size, ws->info.gart_page_size);
   alignment = align(alignment, ws->info.gart_page_size);

   bool use_reusable_pool = flags & RADEON_FLAG_NO_INTERPROCESS_SHARING &&
                            !(flags & RADEON_FLAG_DISCARDABLE);

   /* Shared resources don't use cached heaps. */
   if (use_reusable_pool) {
      /* RADEON_FLAG_NO_SUBALLOC is irrelevant for the cache. */
      heap = radeon_get_heap_index(domain, flags & ~RADEON_FLAG_NO_SUBALLOC);
      assert(heap >= 0 && heap < RADEON_NUM_HEAPS);

      bo = radeon_bo((struct pb_buffer_lean*)pb_cache_reclaim_buffer(&ws->bo_cache, size,
                                                                alignment, 0, heap));
      if (bo)
         return &bo->base;
   }

   bo = radeon_create_bo(ws, size, alignment, domain, flags, heap);
   if (!bo) {
      /* Clear the cache and try again. */
      if (ws->info.r600_has_virtual_memory)
         pb_slabs_reclaim(&ws->bo_slabs);
      pb_cache_release_all_buffers(&ws->bo_cache);
      bo = radeon_create_bo(ws, size, alignment, domain, flags, heap);
      if (!bo)
         return NULL;
   }

   bo->u.real.use_reusable_pool = use_reusable_pool;

   mtx_lock(&ws->bo_handles_mutex);
   _mesa_hash_table_insert(ws->bo_handles, (void*)(uintptr_t)bo->handle, bo);
   mtx_unlock(&ws->bo_handles_mutex);

   return &bo->base;
}

static void radeon_winsys_bo_destroy(struct radeon_winsys *ws, struct pb_buffer_lean *buf)
{
   struct radeon_bo *bo = radeon_bo(buf);

   if (bo->handle)
      radeon_bo_destroy_or_cache(ws, buf);
   else
      radeon_bo_slab_destroy(ws, buf);
}

static struct pb_buffer_lean *radeon_winsys_bo_from_ptr(struct radeon_winsys *rws,
                                                   void *pointer, uint64_t size,
                                                   enum radeon_bo_flag flags)
{
   struct radeon_drm_winsys *ws = radeon_drm_winsys(rws);
   struct drm_radeon_gem_userptr args;
   struct radeon_bo *bo;
   int r;

   bo = CALLOC_STRUCT(radeon_bo);
   if (!bo)
      return NULL;

   memset(&args, 0, sizeof(args));
   args.addr = (uintptr_t)pointer;
   args.size = align(size, ws->info.gart_page_size);

   if (flags & RADEON_FLAG_READ_ONLY)
      args.flags = RADEON_GEM_USERPTR_READONLY |
                   RADEON_GEM_USERPTR_VALIDATE;
   else
      args.flags = RADEON_GEM_USERPTR_ANONONLY |
                   RADEON_GEM_USERPTR_REGISTER |
                   RADEON_GEM_USERPTR_VALIDATE;

   if (drmCommandWriteRead(ws->fd, DRM_RADEON_GEM_USERPTR,
                           &args, sizeof(args))) {
      FREE(bo);
      return NULL;
   }

   assert(args.handle != 0);

   mtx_lock(&ws->bo_handles_mutex);

   /* Initialize it. */
   pipe_reference_init(&bo->base.reference, 1);
   bo->handle = args.handle;
   bo->base.alignment_log2 = 0;
   bo->base.size = size;
   bo->rws = ws;
   bo->user_ptr = pointer;
   bo->va = 0;
   bo->initial_domain = RADEON_DOMAIN_GTT;
   bo->hash = __sync_fetch_and_add(&ws->next_bo_hash, 1);
   (void) mtx_init(&bo->u.real.map_mutex, mtx_plain);

   _mesa_hash_table_insert(ws->bo_handles, (void*)(uintptr_t)bo->handle, bo);

   mtx_unlock(&ws->bo_handles_mutex);

   if (ws->info.r600_has_virtual_memory) {
      struct drm_radeon_gem_va va;

      bo->va = radeon_bomgr_find_va64(ws, bo->base.size, 1 << 20);

      va.handle = bo->handle;
      va.operation = RADEON_VA_MAP;
      va.vm_id = 0;
      va.offset = bo->va;
      va.flags = RADEON_VM_PAGE_READABLE |
                 RADEON_VM_PAGE_WRITEABLE |
                 RADEON_VM_PAGE_SNOOPED;
      va.offset = bo->va;
      r = drmCommandWriteRead(ws->fd, DRM_RADEON_GEM_VA, &va, sizeof(va));
      if (r && va.operation == RADEON_VA_RESULT_ERROR) {
         fprintf(stderr, "radeon: Failed to assign virtual address space\n");
         radeon_bo_destroy(NULL, &bo->base);
         return NULL;
      }
      mtx_lock(&ws->bo_handles_mutex);
      if (va.operation == RADEON_VA_RESULT_VA_EXIST) {
         struct pb_buffer_lean *b = &bo->base;
         struct radeon_bo *old_bo =
               _mesa_hash_table_u64_search(ws->bo_vas, va.offset);

         mtx_unlock(&ws->bo_handles_mutex);
         radeon_bo_reference(rws, &b, &old_bo->base);
         return b;
      }

      _mesa_hash_table_u64_insert(ws->bo_vas, bo->va, bo);
      mtx_unlock(&ws->bo_handles_mutex);
   }

   ws->allocated_gtt += align(bo->base.size, ws->info.gart_page_size);

   return (struct pb_buffer_lean*)bo;
}

static struct pb_buffer_lean *radeon_winsys_bo_from_handle(struct radeon_winsys *rws,
                                                      struct winsys_handle *whandle,
                                                      unsigned vm_alignment,
                                                      bool is_dri_prime_linear_buffer)
{
   struct radeon_drm_winsys *ws = radeon_drm_winsys(rws);
   struct radeon_bo *bo;
   int r;
   unsigned handle;
   uint64_t size = 0;

   /* We must maintain a list of pairs <handle, bo>, so that we always return
    * the same BO for one particular handle. If we didn't do that and created
    * more than one BO for the same handle and then relocated them in a CS,
    * we would hit a deadlock in the kernel.
    *
    * The list of pairs is guarded by a mutex, of course. */
   mtx_lock(&ws->bo_handles_mutex);

   if (whandle->type == WINSYS_HANDLE_TYPE_SHARED) {
      /* First check if there already is an existing bo for the handle. */
      bo = util_hash_table_get(ws->bo_names, (void*)(uintptr_t)whandle->handle);
   } else if (whandle->type == WINSYS_HANDLE_TYPE_FD) {
      /* We must first get the GEM handle, as fds are unreliable keys */
      r = drmPrimeFDToHandle(ws->fd, whandle->handle, &handle);
      if (r)
         goto fail;
      bo = util_hash_table_get(ws->bo_handles, (void*)(uintptr_t)handle);
   } else {
      /* Unknown handle type */
      goto fail;
   }

   if (bo) {
      /* Increase the refcount. */
      p_atomic_inc(&bo->base.reference.count);
      goto done;
   }

   /* There isn't, create a new one. */
   bo = CALLOC_STRUCT(radeon_bo);
   if (!bo) {
      goto fail;
   }

   if (whandle->type == WINSYS_HANDLE_TYPE_SHARED) {
      struct drm_gem_open open_arg = {};
      memset(&open_arg, 0, sizeof(open_arg));
      /* Open the BO. */
      open_arg.name = whandle->handle;
      if (drmIoctl(ws->fd, DRM_IOCTL_GEM_OPEN, &open_arg)) {
         FREE(bo);
         goto fail;
      }
      handle = open_arg.handle;
      size = open_arg.size;
      bo->flink_name = whandle->handle;
   } else if (whandle->type == WINSYS_HANDLE_TYPE_FD) {
      size = lseek(whandle->handle, 0, SEEK_END);
      /*
       * Could check errno to determine whether the kernel is new enough, but
       * it doesn't really matter why this failed, just that it failed.
       */
      if (size == (off_t)-1) {
         FREE(bo);
         goto fail;
      }
      lseek(whandle->handle, 0, SEEK_SET);
   }

   assert(handle != 0);

   bo->handle = handle;

   /* Initialize it. */
   pipe_reference_init(&bo->base.reference, 1);
   bo->base.alignment_log2 = 0;
   bo->base.size = (unsigned) size;
   bo->rws = ws;
   bo->va = 0;
   bo->hash = __sync_fetch_and_add(&ws->next_bo_hash, 1);
   (void) mtx_init(&bo->u.real.map_mutex, mtx_plain);

   if (bo->flink_name)
      _mesa_hash_table_insert(ws->bo_names, (void*)(uintptr_t)bo->flink_name, bo);

   _mesa_hash_table_insert(ws->bo_handles, (void*)(uintptr_t)bo->handle, bo);

done:
   mtx_unlock(&ws->bo_handles_mutex);

   if (ws->info.r600_has_virtual_memory && !bo->va) {
      struct drm_radeon_gem_va va;

      bo->va = radeon_bomgr_find_va64(ws, bo->base.size, vm_alignment);

      va.handle = bo->handle;
      va.operation = RADEON_VA_MAP;
      va.vm_id = 0;
      va.offset = bo->va;
      va.flags = RADEON_VM_PAGE_READABLE |
                 RADEON_VM_PAGE_WRITEABLE |
                 RADEON_VM_PAGE_SNOOPED;
      va.offset = bo->va;
      r = drmCommandWriteRead(ws->fd, DRM_RADEON_GEM_VA, &va, sizeof(va));
      if (r && va.operation == RADEON_VA_RESULT_ERROR) {
         fprintf(stderr, "radeon: Failed to assign virtual address space\n");
         radeon_bo_destroy(NULL, &bo->base);
         return NULL;
      }
      mtx_lock(&ws->bo_handles_mutex);
      if (va.operation == RADEON_VA_RESULT_VA_EXIST) {
         struct pb_buffer_lean *b = &bo->base;
         struct radeon_bo *old_bo =
               _mesa_hash_table_u64_search(ws->bo_vas, va.offset);

         mtx_unlock(&ws->bo_handles_mutex);
         radeon_bo_reference(rws, &b, &old_bo->base);
         return b;
      }

      _mesa_hash_table_u64_insert(ws->bo_vas, bo->va, bo);
      mtx_unlock(&ws->bo_handles_mutex);
   }

   bo->initial_domain = radeon_bo_get_initial_domain((void*)bo);

   if (bo->initial_domain & RADEON_DOMAIN_VRAM)
      ws->allocated_vram += align(bo->base.size, ws->info.gart_page_size);
   else if (bo->initial_domain & RADEON_DOMAIN_GTT)
      ws->allocated_gtt += align(bo->base.size, ws->info.gart_page_size);

   return (struct pb_buffer_lean*)bo;

fail:
   mtx_unlock(&ws->bo_handles_mutex);
   return NULL;
}

static bool radeon_winsys_bo_get_handle(struct radeon_winsys *rws,
                                        struct pb_buffer_lean *buffer,
                                        struct winsys_handle *whandle)
{
   struct drm_gem_flink flink;
   struct radeon_bo *bo = radeon_bo(buffer);
   struct radeon_drm_winsys *ws = bo->rws;

   /* Don't allow exports of slab entries. */
   if (!bo->handle)
      return false;

   memset(&flink, 0, sizeof(flink));

   bo->u.real.use_reusable_pool = false;

   if (whandle->type == WINSYS_HANDLE_TYPE_SHARED) {
      if (!bo->flink_name) {
         flink.handle = bo->handle;

         if (ioctl(ws->fd, DRM_IOCTL_GEM_FLINK, &flink)) {
            return false;
         }

         bo->flink_name = flink.name;

         mtx_lock(&ws->bo_handles_mutex);
         _mesa_hash_table_insert(ws->bo_names, (void*)(uintptr_t)bo->flink_name, bo);
         mtx_unlock(&ws->bo_handles_mutex);
      }
      whandle->handle = bo->flink_name;
   } else if (whandle->type == WINSYS_HANDLE_TYPE_KMS) {
      whandle->handle = bo->handle;
   } else if (whandle->type == WINSYS_HANDLE_TYPE_FD) {
      if (drmPrimeHandleToFD(ws->fd, bo->handle, DRM_CLOEXEC, (int*)&whandle->handle))
         return false;
   }

   return true;
}

static bool radeon_winsys_bo_is_user_ptr(struct pb_buffer_lean *buf)
{
   return ((struct radeon_bo*)buf)->user_ptr != NULL;
}

static bool radeon_winsys_bo_is_suballocated(struct pb_buffer_lean *buf)
{
   return !((struct radeon_bo*)buf)->handle;
}

static uint64_t radeon_winsys_bo_va(struct pb_buffer_lean *buf)
{
   return ((struct radeon_bo*)buf)->va;
}

static unsigned radeon_winsys_bo_get_reloc_offset(struct pb_buffer_lean *buf)
{
   struct radeon_bo *bo = radeon_bo(buf);

   if (bo->handle)
      return 0;

   return bo->va - bo->u.slab.real->va;
}

void radeon_drm_bo_init_functions(struct radeon_drm_winsys *ws)
{
   ws->base.buffer_set_metadata = radeon_bo_set_metadata;
   ws->base.buffer_get_metadata = radeon_bo_get_metadata;
   ws->base.buffer_map = radeon_bo_map;
   ws->base.buffer_unmap = radeon_bo_unmap;
   ws->base.buffer_wait = radeon_bo_wait;
   ws->base.buffer_create = radeon_winsys_bo_create;
   ws->base.buffer_destroy = radeon_winsys_bo_destroy;
   ws->base.buffer_from_handle = radeon_winsys_bo_from_handle;
   ws->base.buffer_from_ptr = radeon_winsys_bo_from_ptr;
   ws->base.buffer_is_user_ptr = radeon_winsys_bo_is_user_ptr;
   ws->base.buffer_is_suballocated = radeon_winsys_bo_is_suballocated;
   ws->base.buffer_get_handle = radeon_winsys_bo_get_handle;
   ws->base.buffer_get_virtual_address = radeon_winsys_bo_va;
   ws->base.buffer_get_reloc_offset = radeon_winsys_bo_get_reloc_offset;
   ws->base.buffer_get_initial_domain = radeon_bo_get_initial_domain;
}
