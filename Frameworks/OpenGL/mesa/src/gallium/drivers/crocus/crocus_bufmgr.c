/*
 * Copyright © 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file crocus_bufmgr.c
 *
 * The crocus buffer manager.
 *
 * XXX: write better comments
 * - BOs
 * - Explain BO cache
 * - main interface to GEM in the kernel
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xf86drm.h>
#include <util/u_atomic.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>

#include "errno.h"
#include "common/intel_mem.h"
#include "dev/intel_debug.h"
#include "common/intel_gem.h"
#include "dev/intel_device_info.h"
#include "util/u_debug.h"
#include "util/macros.h"
#include "util/hash_table.h"
#include "util/list.h"
#include "util/os_file.h"
#include "util/u_dynarray.h"
#include "util/vma.h"
#include "crocus_bufmgr.h"
#include "crocus_context.h"
#include "string.h"

#include "drm-uapi/i915_drm.h"

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x)
#endif

/**
 * For debugging purposes, this returns a time in seconds.
 */
static double
get_time(void)
{
   struct timespec tp;

   clock_gettime(CLOCK_MONOTONIC, &tp);

   return tp.tv_sec + tp.tv_nsec / 1000000000.0;
}

/* VALGRIND_FREELIKE_BLOCK unfortunately does not actually undo the earlier
 * VALGRIND_MALLOCLIKE_BLOCK but instead leaves vg convinced the memory is
 * leaked. All because it does not call VG(cli_free) from its
 * VG_USERREQ__FREELIKE_BLOCK handler. Instead of treating the memory like
 * and allocation, we mark it available for use upon mmapping and remove
 * it upon unmapping.
 */
#define VG_DEFINED(ptr, size) VG(VALGRIND_MAKE_MEM_DEFINED(ptr, size))
#define VG_NOACCESS(ptr, size) VG(VALGRIND_MAKE_MEM_NOACCESS(ptr, size))

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define WARN_ONCE(cond, fmt...) do {                            \
   if (unlikely(cond)) {                                        \
      static bool _warned = false;                              \
      if (!_warned) {                                           \
         fprintf(stderr, "WARNING: ");                          \
         fprintf(stderr, fmt);                                  \
         _warned = true;                                        \
      }                                                         \
   }                                                            \
} while (0)

#define FILE_DEBUG_FLAG DEBUG_BUFMGR

struct bo_cache_bucket {
   /** List of cached BOs. */
   struct list_head head;

   /** Size of this bucket, in bytes. */
   uint64_t size;
};

struct bo_export {
   /** File descriptor associated with a handle export. */
   int drm_fd;

   /** GEM handle in drm_fd */
   uint32_t gem_handle;

   struct list_head link;
};

struct crocus_bufmgr {
   /**
    * List into the list of bufmgr.
    */
   struct list_head link;

   uint32_t refcount;

   int fd;

   simple_mtx_t lock;

   /** Array of lists of cached gem objects of power-of-two sizes */
   struct bo_cache_bucket cache_bucket[14 * 4];
   int num_buckets;
   time_t time;

   struct hash_table *name_table;
   struct hash_table *handle_table;

   /**
    * List of BOs which we've effectively freed, but are hanging on to
    * until they're idle before closing and returning the VMA.
    */
   struct list_head zombie_list;

   bool has_llc:1;
   bool has_mmap_offset:1;
   bool has_tiling_uapi:1;
   bool bo_reuse:1;
};

static simple_mtx_t global_bufmgr_list_mutex = SIMPLE_MTX_INITIALIZER;
static struct list_head global_bufmgr_list = {
   .next = &global_bufmgr_list,
   .prev = &global_bufmgr_list,
};

static int bo_set_tiling_internal(struct crocus_bo *bo, uint32_t tiling_mode,
                                  uint32_t stride);

static void bo_free(struct crocus_bo *bo);

static uint32_t
key_hash_uint(const void *key)
{
   return _mesa_hash_data(key, 4);
}

static bool
key_uint_equal(const void *a, const void *b)
{
   return *((unsigned *) a) == *((unsigned *) b);
}

static struct crocus_bo *
find_and_ref_external_bo(struct hash_table *ht, unsigned int key)
{
   struct hash_entry *entry = _mesa_hash_table_search(ht, &key);
   struct crocus_bo *bo = entry ? entry->data : NULL;

   if (bo) {
      assert(bo->external);
      assert(!bo->reusable);

      /* Being non-reusable, the BO cannot be in the cache lists, but it
       * may be in the zombie list if it had reached zero references, but
       * we hadn't yet closed it...and then reimported the same BO.  If it
       * is, then remove it since it's now been resurrected.
       */
      if (bo->head.prev || bo->head.next)
         list_del(&bo->head);

      crocus_bo_reference(bo);
   }

   return bo;
}

/**
 * This function finds the correct bucket fit for the input size.
 * The function works with O(1) complexity when the requested size
 * was queried instead of iterating the size through all the buckets.
 */
static struct bo_cache_bucket *
bucket_for_size(struct crocus_bufmgr *bufmgr, uint64_t size)
{
   /* Calculating the pages and rounding up to the page size. */
   const unsigned pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

   /* Row  Bucket sizes    clz((x-1) | 3)   Row    Column
    *        in pages                      stride   size
    *   0:   1  2  3  4 -> 30 30 30 30        4       1
    *   1:   5  6  7  8 -> 29 29 29 29        4       1
    *   2:  10 12 14 16 -> 28 28 28 28        8       2
    *   3:  20 24 28 32 -> 27 27 27 27       16       4
    */
   const unsigned row = 30 - __builtin_clz((pages - 1) | 3);
   const unsigned row_max_pages = 4 << row;

   /* The '& ~2' is the special case for row 1. In row 1, max pages /
    * 2 is 2, but the previous row maximum is zero (because there is
    * no previous row). All row maximum sizes are power of 2, so that
    * is the only case where that bit will be set.
    */
   const unsigned prev_row_max_pages = (row_max_pages / 2) & ~2;
   int col_size_log2 = row - 1;
   col_size_log2 += (col_size_log2 < 0);

   const unsigned col = (pages - prev_row_max_pages +
                         ((1 << col_size_log2) - 1)) >> col_size_log2;

   /* Calculating the index based on the row and column. */
   const unsigned index = (row * 4) + (col - 1);

   return (index < bufmgr->num_buckets) ?
          &bufmgr->cache_bucket[index] : NULL;
}


int
crocus_bo_busy(struct crocus_bo *bo)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;
   struct drm_i915_gem_busy busy = { .handle = bo->gem_handle };

   int ret = intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_BUSY, &busy);
   if (ret == 0) {
      bo->idle = !busy.busy;
      return busy.busy;
   }
   return false;
}

int
crocus_bo_madvise(struct crocus_bo *bo, int state)
{
   struct drm_i915_gem_madvise madv = {
      .handle = bo->gem_handle,
      .madv = state,
      .retained = 1,
   };

   intel_ioctl(bo->bufmgr->fd, DRM_IOCTL_I915_GEM_MADVISE, &madv);

   return madv.retained;
}

static struct crocus_bo *
bo_calloc(void)
{
   struct crocus_bo *bo = calloc(1, sizeof(*bo));
   if (!bo)
      return NULL;

   list_inithead(&bo->exports);
   bo->hash = _mesa_hash_pointer(bo);
   return bo;
}

static struct crocus_bo *
alloc_bo_from_cache(struct crocus_bufmgr *bufmgr,
                    struct bo_cache_bucket *bucket,
                    uint32_t alignment,
                    unsigned flags)
{
   if (!bucket)
      return NULL;

   struct crocus_bo *bo = NULL;

   list_for_each_entry_safe(struct crocus_bo, cur, &bucket->head, head) {
      /* If the last BO in the cache is busy, there are no idle BOs.  Bail,
       * either falling back to a non-matching memzone, or if that fails,
       * allocating a fresh buffer.
       */
      if (crocus_bo_busy(cur))
         return NULL;

      list_del(&cur->head);

      /* Tell the kernel we need this BO.  If it still exists, we're done! */
      if (crocus_bo_madvise(cur, I915_MADV_WILLNEED)) {
         bo = cur;
         break;
      }

      /* This BO was purged, throw it out and keep looking. */
      bo_free(cur);
   }

   if (!bo)
      return NULL;

   /* Zero the contents if necessary.  If this fails, fall back to
    * allocating a fresh BO, which will always be zeroed by the kernel.
    */
   if (flags & BO_ALLOC_ZEROED) {
      void *map = crocus_bo_map(NULL, bo, MAP_WRITE | MAP_RAW);
      if (map) {
         memset(map, 0, bo->size);
      } else {
         bo_free(bo);
         return NULL;
      }
   }

   return bo;
}

static struct crocus_bo *
alloc_fresh_bo(struct crocus_bufmgr *bufmgr, uint64_t bo_size)
{
   struct crocus_bo *bo = bo_calloc();
   if (!bo)
      return NULL;

   struct drm_i915_gem_create create = { .size = bo_size };

   /* All new BOs we get from the kernel are zeroed, so we don't need to
    * worry about that here.
    */
   if (intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_CREATE, &create) != 0) {
      free(bo);
      return NULL;
   }

   bo->gem_handle = create.handle;
   bo->bufmgr = bufmgr;
   bo->size = bo_size;
   bo->idle = true;
   bo->tiling_mode = I915_TILING_NONE;
   bo->swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
   bo->stride = 0;

   /* Calling set_domain() will allocate pages for the BO outside of the
    * struct mutex lock in the kernel, which is more efficient than waiting
    * to create them during the first execbuf that uses the BO.
    */
   struct drm_i915_gem_set_domain sd = {
      .handle = bo->gem_handle,
      .read_domains = I915_GEM_DOMAIN_CPU,
      .write_domain = 0,
   };

   if (intel_ioctl(bo->bufmgr->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &sd) != 0) {
      bo_free(bo);
      return NULL;
   }

   return bo;
}

static struct crocus_bo *
bo_alloc_internal(struct crocus_bufmgr *bufmgr,
                  const char *name,
                  uint64_t size,
                  uint32_t alignment,
                  unsigned flags,
                  uint32_t tiling_mode,
                  uint32_t stride)
{
   struct crocus_bo *bo;
   unsigned int page_size = getpagesize();
   struct bo_cache_bucket *bucket = bucket_for_size(bufmgr, size);

   /* Round the size up to the bucket size, or if we don't have caching
    * at this size, a multiple of the page size.
    */
   uint64_t bo_size =
      bucket ? bucket->size : MAX2(align64(size, page_size), page_size);

   simple_mtx_lock(&bufmgr->lock);

   /* Get a buffer out of the cache if available.  First, we try to find
    * one with a matching memory zone so we can avoid reallocating VMA.
    */
   bo = alloc_bo_from_cache(bufmgr, bucket, alignment, flags);

   simple_mtx_unlock(&bufmgr->lock);

   if (!bo) {
      bo = alloc_fresh_bo(bufmgr, bo_size);
      if (!bo)
         return NULL;
   }

   if (bo_set_tiling_internal(bo, tiling_mode, stride))
      goto err_free;

   bo->name = name;
   p_atomic_set(&bo->refcount, 1);
   bo->reusable = bucket && bufmgr->bo_reuse;
   bo->cache_coherent = bufmgr->has_llc;
   bo->index = -1;
   bo->kflags = 0;

   if (flags & BO_ALLOC_SCANOUT)
      bo->scanout = 1;

   if ((flags & BO_ALLOC_COHERENT) && !bo->cache_coherent) {
      struct drm_i915_gem_caching arg = {
         .handle = bo->gem_handle,
         .caching = 1,
      };
      if (intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_SET_CACHING, &arg) == 0) {
         bo->cache_coherent = true;
         bo->reusable = false;
      }
   }

   DBG("bo_create: buf %d (%s) %llub\n", bo->gem_handle,
       bo->name, (unsigned long long) size);

   return bo;

err_free:
   bo_free(bo);
   return NULL;
}

struct crocus_bo *
crocus_bo_alloc(struct crocus_bufmgr *bufmgr,
                const char *name,
                uint64_t size)
{
   return bo_alloc_internal(bufmgr, name, size, 1,
                            0, I915_TILING_NONE, 0);
}

struct crocus_bo *
crocus_bo_alloc_tiled(struct crocus_bufmgr *bufmgr, const char *name,
                      uint64_t size, uint32_t alignment,
                      uint32_t tiling_mode, uint32_t pitch, unsigned flags)
{
   return bo_alloc_internal(bufmgr, name, size, alignment,
                            flags, tiling_mode, pitch);
}

struct crocus_bo *
crocus_bo_create_userptr(struct crocus_bufmgr *bufmgr, const char *name,
                         void *ptr, size_t size)
{
   struct crocus_bo *bo;

   bo = bo_calloc();
   if (!bo)
      return NULL;

   struct drm_i915_gem_userptr arg = {
      .user_ptr = (uintptr_t)ptr,
      .user_size = size,
   };
   if (intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_USERPTR, &arg))
      goto err_free;
   bo->gem_handle = arg.handle;

   /* Check the buffer for validity before we try and use it in a batch */
   struct drm_i915_gem_set_domain sd = {
      .handle = bo->gem_handle,
      .read_domains = I915_GEM_DOMAIN_CPU,
   };
   if (intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &sd))
      goto err_close;

   bo->name = name;
   bo->size = size;
   bo->map_cpu = ptr;

   bo->bufmgr = bufmgr;
   bo->kflags = 0;

   p_atomic_set(&bo->refcount, 1);
   bo->userptr = true;
   bo->cache_coherent = true;
   bo->index = -1;
   bo->idle = true;

   return bo;

err_close:
   intel_ioctl(bufmgr->fd, DRM_IOCTL_GEM_CLOSE, &bo->gem_handle);
err_free:
   free(bo);
   return NULL;
}

/**
 * Returns a crocus_bo wrapping the given buffer object handle.
 *
 * This can be used when one application needs to pass a buffer object
 * to another.
 */
struct crocus_bo *
crocus_bo_gem_create_from_name(struct crocus_bufmgr *bufmgr,
                               const char *name, unsigned int handle)
{
   struct crocus_bo *bo;

   /* At the moment most applications only have a few named bo.
    * For instance, in a DRI client only the render buffers passed
    * between X and the client are named. And since X returns the
    * alternating names for the front/back buffer a linear search
    * provides a sufficiently fast match.
    */
   simple_mtx_lock(&bufmgr->lock);
   bo = find_and_ref_external_bo(bufmgr->name_table, handle);
   if (bo)
      goto out;

   struct drm_gem_open open_arg = { .name = handle };
   int ret = intel_ioctl(bufmgr->fd, DRM_IOCTL_GEM_OPEN, &open_arg);
   if (ret != 0) {
      DBG("Couldn't reference %s handle 0x%08x: %s\n",
          name, handle, strerror(errno));
      bo = NULL;
      goto out;
   }
   /* Now see if someone has used a prime handle to get this
    * object from the kernel before by looking through the list
    * again for a matching gem_handle
    */
   bo = find_and_ref_external_bo(bufmgr->handle_table, open_arg.handle);
   if (bo)
      goto out;

   bo = bo_calloc();
   if (!bo)
      goto out;

   p_atomic_set(&bo->refcount, 1);

   bo->size = open_arg.size;
   bo->gtt_offset = 0;
   bo->bufmgr = bufmgr;
   bo->gem_handle = open_arg.handle;
   bo->name = name;
   bo->global_name = handle;
   bo->reusable = false;
   bo->external = true;
   bo->kflags = 0;

   _mesa_hash_table_insert(bufmgr->handle_table, &bo->gem_handle, bo);
   _mesa_hash_table_insert(bufmgr->name_table, &bo->global_name, bo);

   struct drm_i915_gem_get_tiling get_tiling = { .handle = bo->gem_handle };
   ret = intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_GET_TILING, &get_tiling);
   if (ret != 0)
      goto err_unref;

   bo->tiling_mode = get_tiling.tiling_mode;
   bo->swizzle_mode = get_tiling.swizzle_mode;
   /* XXX stride is unknown */
   DBG("bo_create_from_handle: %d (%s)\n", handle, bo->name);

out:
   simple_mtx_unlock(&bufmgr->lock);
   return bo;

err_unref:
   bo_free(bo);
   simple_mtx_unlock(&bufmgr->lock);
   return NULL;
}

static void
bo_close(struct crocus_bo *bo)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   if (bo->external) {
      struct hash_entry *entry;

      if (bo->global_name) {
         entry = _mesa_hash_table_search(bufmgr->name_table, &bo->global_name);
         _mesa_hash_table_remove(bufmgr->name_table, entry);
      }

      entry = _mesa_hash_table_search(bufmgr->handle_table, &bo->gem_handle);
      _mesa_hash_table_remove(bufmgr->handle_table, entry);

      list_for_each_entry_safe(struct bo_export, export, &bo->exports, link) {
         struct drm_gem_close close = { .handle = export->gem_handle };
         intel_ioctl(export->drm_fd, DRM_IOCTL_GEM_CLOSE, &close);

         list_del(&export->link);
         free(export);
      }
   } else {
      assert(list_is_empty(&bo->exports));
   }

   /* Close this object */
   struct drm_gem_close close = { .handle = bo->gem_handle };
   int ret = intel_ioctl(bufmgr->fd, DRM_IOCTL_GEM_CLOSE, &close);
   if (ret != 0) {
      DBG("DRM_IOCTL_GEM_CLOSE %d failed (%s): %s\n",
          bo->gem_handle, bo->name, strerror(errno));
   }

   free(bo);
}

static void
bo_free(struct crocus_bo *bo)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   if (bo->map_cpu && !bo->userptr) {
      VG_NOACCESS(bo->map_cpu, bo->size);
      munmap(bo->map_cpu, bo->size);
   }
   if (bo->map_wc) {
      VG_NOACCESS(bo->map_wc, bo->size);
      munmap(bo->map_wc, bo->size);
   }
   if (bo->map_gtt) {
      VG_NOACCESS(bo->map_gtt, bo->size);
      munmap(bo->map_gtt, bo->size);
   }

   if (bo->idle) {
      bo_close(bo);
   } else {
      /* Defer closing the GEM BO and returning the VMA for reuse until the
       * BO is idle.  Just move it to the dead list for now.
       */
      list_addtail(&bo->head, &bufmgr->zombie_list);
   }
}

/** Frees all cached buffers significantly older than @time. */
static void
cleanup_bo_cache(struct crocus_bufmgr *bufmgr, time_t time)
{
   int i;

   if (bufmgr->time == time)
      return;

   for (i = 0; i < bufmgr->num_buckets; i++) {
      struct bo_cache_bucket *bucket = &bufmgr->cache_bucket[i];

      list_for_each_entry_safe(struct crocus_bo, bo, &bucket->head, head) {
         if (time - bo->free_time <= 1)
            break;

         list_del(&bo->head);

         bo_free(bo);
      }
   }

   list_for_each_entry_safe(struct crocus_bo, bo, &bufmgr->zombie_list, head) {
      /* Stop once we reach a busy BO - all others past this point were
       * freed more recently so are likely also busy.
       */
      if (!bo->idle && crocus_bo_busy(bo))
         break;

      list_del(&bo->head);
      bo_close(bo);
   }

   bufmgr->time = time;
}

static void
bo_unreference_final(struct crocus_bo *bo, time_t time)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;
   struct bo_cache_bucket *bucket;

   DBG("bo_unreference final: %d (%s)\n", bo->gem_handle, bo->name);

   bucket = NULL;
   if (bo->reusable)
      bucket = bucket_for_size(bufmgr, bo->size);
   /* Put the buffer into our internal cache for reuse if we can. */
   if (bucket && crocus_bo_madvise(bo, I915_MADV_DONTNEED)) {
      bo->free_time = time;
      bo->name = NULL;

      list_addtail(&bo->head, &bucket->head);
   } else {
      bo_free(bo);
   }
}

void
__crocus_bo_unreference(struct crocus_bo *bo)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;
   struct timespec time;

   clock_gettime(CLOCK_MONOTONIC, &time);

   simple_mtx_lock(&bufmgr->lock);

   if (p_atomic_dec_zero(&bo->refcount)) {
      bo_unreference_final(bo, time.tv_sec);
      cleanup_bo_cache(bufmgr, time.tv_sec);
   }

   simple_mtx_unlock(&bufmgr->lock);
}

static void
bo_wait_with_stall_warning(struct util_debug_callback *dbg,
                           struct crocus_bo *bo,
                           const char *action)
{
   bool busy = dbg && !bo->idle;
   double elapsed = unlikely(busy) ? -get_time() : 0.0;

   crocus_bo_wait_rendering(bo);

   if (unlikely(busy)) {
      elapsed += get_time();
      if (elapsed > 1e-5) /* 0.01ms */ {
         perf_debug(dbg, "%s a busy \"%s\" BO stalled and took %.03f ms.\n",
                    action, bo->name, elapsed * 1000);
      }
   }
}

static void
print_flags(unsigned flags)
{
   if (flags & MAP_READ)
      DBG("READ ");
   if (flags & MAP_WRITE)
      DBG("WRITE ");
   if (flags & MAP_ASYNC)
      DBG("ASYNC ");
   if (flags & MAP_PERSISTENT)
      DBG("PERSISTENT ");
   if (flags & MAP_COHERENT)
      DBG("COHERENT ");
   if (flags & MAP_RAW)
      DBG("RAW ");
   DBG("\n");
}

static void *
crocus_bo_gem_mmap_legacy(struct util_debug_callback *dbg,
                          struct crocus_bo *bo, bool wc)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   struct drm_i915_gem_mmap mmap_arg = {
      .handle = bo->gem_handle,
      .size = bo->size,
      .flags = wc ? I915_MMAP_WC : 0,
   };

   int ret = intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_MMAP, &mmap_arg);
   if (ret != 0) {
      DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
          __FILE__, __LINE__, bo->gem_handle, bo->name, strerror(errno));
      return NULL;
   }
   void *map = (void *) (uintptr_t) mmap_arg.addr_ptr;

   return map;
}

static void *
crocus_bo_gem_mmap_offset(struct util_debug_callback *dbg, struct crocus_bo *bo,
                          bool wc)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   struct drm_i915_gem_mmap_offset mmap_arg = {
      .handle = bo->gem_handle,
      .flags = wc ? I915_MMAP_OFFSET_WC : I915_MMAP_OFFSET_WB,
   };

   /* Get the fake offset back */
   int ret = intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_MMAP_OFFSET, &mmap_arg);
   if (ret != 0) {
      DBG("%s:%d: Error preparing buffer %d (%s): %s .\n",
          __FILE__, __LINE__, bo->gem_handle, bo->name, strerror(errno));
      return NULL;
   }

   /* And map it */
   void *map = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    bufmgr->fd, mmap_arg.offset);
   if (map == MAP_FAILED) {
      DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
          __FILE__, __LINE__, bo->gem_handle, bo->name, strerror(errno));
      return NULL;
   }

   return map;
}

static void *
crocus_bo_gem_mmap(struct util_debug_callback *dbg, struct crocus_bo *bo, bool wc)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   if (bufmgr->has_mmap_offset)
      return crocus_bo_gem_mmap_offset(dbg, bo, wc);
   else
      return crocus_bo_gem_mmap_legacy(dbg, bo, wc);
}

static void *
crocus_bo_map_cpu(struct util_debug_callback *dbg,
                  struct crocus_bo *bo, unsigned flags)
{
   /* We disallow CPU maps for writing to non-coherent buffers, as the
    * CPU map can become invalidated when a batch is flushed out, which
    * can happen at unpredictable times.  You should use WC maps instead.
    */
   assert(bo->cache_coherent || !(flags & MAP_WRITE));

   if (!bo->map_cpu) {
      DBG("crocus_bo_map_cpu: %d (%s)\n", bo->gem_handle, bo->name);

      void *map = crocus_bo_gem_mmap(dbg, bo, false);
      if (!map) {
         return NULL;
      }

      VG_DEFINED(map, bo->size);

      if (p_atomic_cmpxchg(&bo->map_cpu, NULL, map)) {
         VG_NOACCESS(map, bo->size);
         munmap(map, bo->size);
      }
   }
   assert(bo->map_cpu);

   DBG("crocus_bo_map_cpu: %d (%s) -> %p, ", bo->gem_handle, bo->name,
       bo->map_cpu);
   print_flags(flags);

   if (!(flags & MAP_ASYNC)) {
      bo_wait_with_stall_warning(dbg, bo, "CPU mapping");
   }

   if (!bo->cache_coherent && !bo->bufmgr->has_llc) {
      /* If we're reusing an existing CPU mapping, the CPU caches may
       * contain stale data from the last time we read from that mapping.
       * (With the BO cache, it might even be data from a previous buffer!)
       * Even if it's a brand new mapping, the kernel may have zeroed the
       * buffer via CPU writes.
       *
       * We need to invalidate those cachelines so that we see the latest
       * contents, and so long as we only read from the CPU mmap we do not
       * need to write those cachelines back afterwards.
       *
       * On LLC, the emprical evidence suggests that writes from the GPU
       * that bypass the LLC (i.e. for scanout) do *invalidate* the CPU
       * cachelines. (Other reads, such as the display engine, bypass the
       * LLC entirely requiring us to keep dirty pixels for the scanout
       * out of any cache.)
       */
#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
      intel_invalidate_range(bo->map_cpu, bo->size);
#endif
   }

   return bo->map_cpu;
}

static void *
crocus_bo_map_wc(struct util_debug_callback *dbg,
                 struct crocus_bo *bo, unsigned flags)
{
   if (!bo->map_wc) {
      DBG("crocus_bo_map_wc: %d (%s)\n", bo->gem_handle, bo->name);

      void *map = crocus_bo_gem_mmap(dbg, bo, true);
      if (!map) {
         return NULL;
      }

      VG_DEFINED(map, bo->size);

      if (p_atomic_cmpxchg(&bo->map_wc, NULL, map)) {
         VG_NOACCESS(map, bo->size);
         munmap(map, bo->size);
      }
   }
   assert(bo->map_wc);

   DBG("crocus_bo_map_wc: %d (%s) -> %p\n", bo->gem_handle, bo->name, bo->map_wc);
   print_flags(flags);

   if (!(flags & MAP_ASYNC)) {
      bo_wait_with_stall_warning(dbg, bo, "WC mapping");
   }

   return bo->map_wc;
}

/**
 * Perform an uncached mapping via the GTT.
 *
 * Write access through the GTT is not quite fully coherent. On low power
 * systems especially, like modern Atoms, we can observe reads from RAM before
 * the write via GTT has landed. A write memory barrier that flushes the Write
 * Combining Buffer (i.e. sfence/mfence) is not sufficient to order the later
 * read after the write as the GTT write suffers a small delay through the GTT
 * indirection. The kernel uses an uncached mmio read to ensure the GTT write
 * is ordered with reads (either by the GPU, WB or WC) and unconditionally
 * flushes prior to execbuf submission. However, if we are not informing the
 * kernel about our GTT writes, it will not flush before earlier access, such
 * as when using the cmdparser. Similarly, we need to be careful if we should
 * ever issue a CPU read immediately following a GTT write.
 *
 * Telling the kernel about write access also has one more important
 * side-effect. Upon receiving notification about the write, it cancels any
 * scanout buffering for FBC/PSR and friends. Later FBC/PSR is then flushed by
 * either SW_FINISH or DIRTYFB. The presumption is that we never write to the
 * actual scanout via a mmaping, only to a backbuffer and so all the FBC/PSR
 * tracking is handled on the buffer exchange instead.
 */
static void *
crocus_bo_map_gtt(struct util_debug_callback *dbg,
                  struct crocus_bo *bo, unsigned flags)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   /* If we don't support get/set_tiling, there's no support for GTT mapping
    * either (it won't do any de-tiling for us).
    */
   assert(bufmgr->has_tiling_uapi);

   /* Get a mapping of the buffer if we haven't before. */
   if (bo->map_gtt == NULL) {
      DBG("bo_map_gtt: mmap %d (%s)\n", bo->gem_handle, bo->name);

      struct drm_i915_gem_mmap_gtt mmap_arg = { .handle = bo->gem_handle };

      /* Get the fake offset back... */
      int ret = intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_MMAP_GTT, &mmap_arg);
      if (ret != 0) {
         DBG("%s:%d: Error preparing buffer map %d (%s): %s .\n",
             __FILE__, __LINE__, bo->gem_handle, bo->name, strerror(errno));
         return NULL;
      }

      /* and mmap it. */
      void *map = mmap(0, bo->size, PROT_READ | PROT_WRITE,
                       MAP_SHARED, bufmgr->fd, mmap_arg.offset);
      if (map == MAP_FAILED) {
         DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
             __FILE__, __LINE__, bo->gem_handle, bo->name, strerror(errno));
         return NULL;
      }

      /* We don't need to use VALGRIND_MALLOCLIKE_BLOCK because Valgrind will
       * already intercept this mmap call. However, for consistency between
       * all the mmap paths, we mark the pointer as defined now and mark it
       * as inaccessible afterwards.
       */
      VG_DEFINED(map, bo->size);

      if (p_atomic_cmpxchg(&bo->map_gtt, NULL, map)) {
         VG_NOACCESS(map, bo->size);
         munmap(map, bo->size);
      }
   }
   assert(bo->map_gtt);

   DBG("bo_map_gtt: %d (%s) -> %p, ", bo->gem_handle, bo->name, bo->map_gtt);
   print_flags(flags);

   if (!(flags & MAP_ASYNC)) {
      bo_wait_with_stall_warning(dbg, bo, "GTT mapping");
   }

   return bo->map_gtt;
}

static bool
can_map_cpu(struct crocus_bo *bo, unsigned flags)
{
   if (bo->scanout)
      return false;

   if (bo->cache_coherent)
      return true;

   /* Even if the buffer itself is not cache-coherent (such as a scanout), on
    * an LLC platform reads always are coherent (as they are performed via the
    * central system agent). It is just the writes that we need to take special
    * care to ensure that land in main memory and not stick in the CPU cache.
    */
   if (!(flags & MAP_WRITE) && bo->bufmgr->has_llc)
      return true;

   /* If PERSISTENT or COHERENT are set, the mmapping needs to remain valid
    * across batch flushes where the kernel will change cache domains of the
    * bo, invalidating continued access to the CPU mmap on non-LLC device.
    *
    * Similarly, ASYNC typically means that the buffer will be accessed via
    * both the CPU and the GPU simultaneously.  Batches may be executed that
    * use the BO even while it is mapped.  While OpenGL technically disallows
    * most drawing while non-persistent mappings are active, we may still use
    * the GPU for blits or other operations, causing batches to happen at
    * inconvenient times.
    *
    * If RAW is set, we expect the caller to be able to handle a WC buffer
    * more efficiently than the involuntary clflushes.
    */
   if (flags & (MAP_PERSISTENT | MAP_COHERENT | MAP_ASYNC | MAP_RAW))
      return false;

   return !(flags & MAP_WRITE);
}

void *
crocus_bo_map(struct util_debug_callback *dbg,
              struct crocus_bo *bo, unsigned flags)
{
   if (bo->tiling_mode != I915_TILING_NONE && !(flags & MAP_RAW))
      return crocus_bo_map_gtt(dbg, bo, flags);

   void *map;

   if (can_map_cpu(bo, flags))
      map = crocus_bo_map_cpu(dbg, bo, flags);
   else
      map = crocus_bo_map_wc(dbg, bo, flags);

   /* Allow the attempt to fail by falling back to the GTT where necessary.
    *
    * Not every buffer can be mmaped directly using the CPU (or WC), for
    * example buffers that wrap stolen memory or are imported from other
    * devices. For those, we have little choice but to use a GTT mmapping.
    * However, if we use a slow GTT mmapping for reads where we expected fast
    * access, that order of magnitude difference in throughput will be clearly
    * expressed by angry users.
    *
    * We skip MAP_RAW because we want to avoid map_gtt's fence detiling.
    */
   if (!map && !(flags & MAP_RAW)) {
      perf_debug(dbg, "Fallback GTT mapping for %s with access flags %x\n",
                 bo->name, flags);
      map = crocus_bo_map_gtt(dbg, bo, flags);
   }

   return map;
}

/** Waits for all GPU rendering with the object to have completed. */
void
crocus_bo_wait_rendering(struct crocus_bo *bo)
{
   /* We require a kernel recent enough for WAIT_IOCTL support.
    * See intel_init_bufmgr()
    */
   crocus_bo_wait(bo, -1);
}

/**
 * Waits on a BO for the given amount of time.
 *
 * @bo: buffer object to wait for
 * @timeout_ns: amount of time to wait in nanoseconds.
 *   If value is less than 0, an infinite wait will occur.
 *
 * Returns 0 if the wait was successful ie. the last batch referencing the
 * object has completed within the allotted time. Otherwise some negative return
 * value describes the error. Of particular interest is -ETIME when the wait has
 * failed to yield the desired result.
 *
 * Similar to crocus_bo_wait_rendering except a timeout parameter allows
 * the operation to give up after a certain amount of time. Another subtle
 * difference is the internal locking semantics are different (this variant does
 * not hold the lock for the duration of the wait). This makes the wait subject
 * to a larger userspace race window.
 *
 * The implementation shall wait until the object is no longer actively
 * referenced within a batch buffer at the time of the call. The wait will
 * not guarantee that the buffer is re-issued via another thread, or an flinked
 * handle. Userspace must make sure this race does not occur if such precision
 * is important.
 *
 * Note that some kernels have broken the inifite wait for negative values
 * promise, upgrade to latest stable kernels if this is the case.
 */
int
crocus_bo_wait(struct crocus_bo *bo, int64_t timeout_ns)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   /* If we know it's idle, don't bother with the kernel round trip */
   if (bo->idle && !bo->external)
      return 0;

   struct drm_i915_gem_wait wait = {
      .bo_handle = bo->gem_handle,
      .timeout_ns = timeout_ns,
   };
   int ret = intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_WAIT, &wait);
   if (ret != 0)
      return -errno;

   bo->idle = true;

   return ret;
}

static void
crocus_bufmgr_destroy(struct crocus_bufmgr *bufmgr)
{
   simple_mtx_destroy(&bufmgr->lock);

   /* Free any cached buffer objects we were going to reuse */
   for (int i = 0; i < bufmgr->num_buckets; i++) {
      struct bo_cache_bucket *bucket = &bufmgr->cache_bucket[i];

      list_for_each_entry_safe(struct crocus_bo, bo, &bucket->head, head) {
         list_del(&bo->head);

         bo_free(bo);
      }
   }

   /* Close any buffer objects on the dead list. */
   list_for_each_entry_safe(struct crocus_bo, bo, &bufmgr->zombie_list, head) {
      list_del(&bo->head);
      bo_close(bo);
   }

   _mesa_hash_table_destroy(bufmgr->name_table, NULL);
   _mesa_hash_table_destroy(bufmgr->handle_table, NULL);

   close(bufmgr->fd);

   free(bufmgr);
}

static int
bo_set_tiling_internal(struct crocus_bo *bo, uint32_t tiling_mode,
                       uint32_t stride)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;
   struct drm_i915_gem_set_tiling set_tiling;
   int ret;

   if (bo->global_name == 0 &&
       tiling_mode == bo->tiling_mode && stride == bo->stride)
      return 0;

   memset(&set_tiling, 0, sizeof(set_tiling));
   do {
      /* set_tiling is slightly broken and overwrites the
       * input on the error path, so we have to open code
       * drm_ioctl.
       */
      set_tiling.handle = bo->gem_handle;
      set_tiling.tiling_mode = tiling_mode;
      set_tiling.stride = stride;

      ret = ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_SET_TILING, &set_tiling);
   } while (ret == -1 && (errno == EINTR || errno == EAGAIN));
   if (ret == -1)
      return -errno;

   bo->tiling_mode = set_tiling.tiling_mode;
   bo->swizzle_mode = set_tiling.swizzle_mode;
   bo->stride = set_tiling.stride;
   return 0;
}

int
crocus_bo_get_tiling(struct crocus_bo *bo, uint32_t *tiling_mode,
                     uint32_t *swizzle_mode)
{
   *tiling_mode = bo->tiling_mode;
   *swizzle_mode = bo->swizzle_mode;
   return 0;
}

struct crocus_bo *
crocus_bo_import_dmabuf(struct crocus_bufmgr *bufmgr, int prime_fd,
                        uint64_t modifier)
{
   uint32_t handle;
   struct crocus_bo *bo;

   simple_mtx_lock(&bufmgr->lock);
   int ret = drmPrimeFDToHandle(bufmgr->fd, prime_fd, &handle);
   if (ret) {
      DBG("import_dmabuf: failed to obtain handle from fd: %s\n",
          strerror(errno));
      simple_mtx_unlock(&bufmgr->lock);
      return NULL;
   }

   /*
    * See if the kernel has already returned this buffer to us. Just as
    * for named buffers, we must not create two bo's pointing at the same
    * kernel object
    */
   bo = find_and_ref_external_bo(bufmgr->handle_table, handle);
   if (bo)
      goto out;

   bo = bo_calloc();
   if (!bo)
      goto out;

   p_atomic_set(&bo->refcount, 1);

   /* Determine size of bo.  The fd-to-handle ioctl really should
    * return the size, but it doesn't.  If we have kernel 3.12 or
    * later, we can lseek on the prime fd to get the size.  Older
    * kernels will just fail, in which case we fall back to the
    * provided (estimated or guess size). */
   ret = lseek(prime_fd, 0, SEEK_END);
   if (ret != -1)
      bo->size = ret;

   bo->bufmgr = bufmgr;
   bo->name = "prime";
   bo->reusable = false;
   bo->external = true;
   bo->kflags = 0;
   bo->gem_handle = handle;
   _mesa_hash_table_insert(bufmgr->handle_table, &bo->gem_handle, bo);

   const struct isl_drm_modifier_info *mod_info =
      isl_drm_modifier_get_info(modifier);
   if (mod_info) {
      bo->tiling_mode = isl_tiling_to_i915_tiling(mod_info->tiling);
   } else if (bufmgr->has_tiling_uapi) {
      struct drm_i915_gem_get_tiling get_tiling = { .handle = bo->gem_handle };
      if (intel_ioctl(bufmgr->fd, DRM_IOCTL_I915_GEM_GET_TILING, &get_tiling))
         goto err;

      bo->tiling_mode = get_tiling.tiling_mode;
   } else {
      bo->tiling_mode = I915_TILING_NONE;
   }

out:
   simple_mtx_unlock(&bufmgr->lock);
   return bo;

err:
   bo_free(bo);
   simple_mtx_unlock(&bufmgr->lock);
   return NULL;
}

struct crocus_bo *
crocus_bo_import_dmabuf_no_mods(struct crocus_bufmgr *bufmgr,
                                int prime_fd)
{
   uint32_t handle;
   struct crocus_bo *bo;

   simple_mtx_lock(&bufmgr->lock);
   int ret = drmPrimeFDToHandle(bufmgr->fd, prime_fd, &handle);
   if (ret) {
      DBG("import_dmabuf: failed to obtain handle from fd: %s\n",
          strerror(errno));
      simple_mtx_unlock(&bufmgr->lock);
      return NULL;
   }

   /*
    * See if the kernel has already returned this buffer to us. Just as
    * for named buffers, we must not create two bo's pointing at the same
    * kernel object
    */
   bo = find_and_ref_external_bo(bufmgr->handle_table, handle);
   if (bo)
      goto out;

   bo = bo_calloc();
   if (!bo)
      goto out;

   p_atomic_set(&bo->refcount, 1);

   /* Determine size of bo.  The fd-to-handle ioctl really should
    * return the size, but it doesn't.  If we have kernel 3.12 or
    * later, we can lseek on the prime fd to get the size.  Older
    * kernels will just fail, in which case we fall back to the
    * provided (estimated or guess size). */
   ret = lseek(prime_fd, 0, SEEK_END);
   if (ret != -1)
      bo->size = ret;

   bo->bufmgr = bufmgr;
   bo->name = "prime";
   bo->reusable = false;
   bo->external = true;
   bo->kflags = 0;
   bo->gem_handle = handle;
   _mesa_hash_table_insert(bufmgr->handle_table, &bo->gem_handle, bo);

out:
   simple_mtx_unlock(&bufmgr->lock);
   return bo;
}

static void
crocus_bo_make_external_locked(struct crocus_bo *bo)
{
   if (!bo->external) {
      _mesa_hash_table_insert(bo->bufmgr->handle_table, &bo->gem_handle, bo);
      bo->external = true;
      bo->reusable = false;
   }
}

static void
crocus_bo_make_external(struct crocus_bo *bo)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   if (bo->external) {
      assert(!bo->reusable);
      return;
   }

   simple_mtx_lock(&bufmgr->lock);
   crocus_bo_make_external_locked(bo);
   simple_mtx_unlock(&bufmgr->lock);
}

int
crocus_bo_export_dmabuf(struct crocus_bo *bo, int *prime_fd)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   crocus_bo_make_external(bo);

   if (drmPrimeHandleToFD(bufmgr->fd, bo->gem_handle,
                          DRM_CLOEXEC | DRM_RDWR, prime_fd) != 0)
      return -errno;

   return 0;
}

uint32_t
crocus_bo_export_gem_handle(struct crocus_bo *bo)
{
   crocus_bo_make_external(bo);

   return bo->gem_handle;
}

int
crocus_bo_flink(struct crocus_bo *bo, uint32_t *name)
{
   struct crocus_bufmgr *bufmgr = bo->bufmgr;

   if (!bo->global_name) {
      struct drm_gem_flink flink = { .handle = bo->gem_handle };

      if (intel_ioctl(bufmgr->fd, DRM_IOCTL_GEM_FLINK, &flink))
         return -errno;

      simple_mtx_lock(&bufmgr->lock);
      if (!bo->global_name) {
         crocus_bo_make_external_locked(bo);
         bo->global_name = flink.name;
         _mesa_hash_table_insert(bufmgr->name_table, &bo->global_name, bo);
      }
      simple_mtx_unlock(&bufmgr->lock);
   }

   *name = bo->global_name;
   return 0;
}

int
crocus_bo_export_gem_handle_for_device(struct crocus_bo *bo, int drm_fd,
                                       uint32_t *out_handle)
{
   /* Only add the new GEM handle to the list of export if it belongs to a
    * different GEM device. Otherwise we might close the same buffer multiple
    * times.
    */
   struct crocus_bufmgr *bufmgr = bo->bufmgr;
   int ret = os_same_file_description(drm_fd, bufmgr->fd);
   WARN_ONCE(ret < 0,
             "Kernel has no file descriptor comparison support: %s\n",
             strerror(errno));
   if (ret == 0) {
      *out_handle = crocus_bo_export_gem_handle(bo);
      return 0;
   }

   struct bo_export *export = calloc(1, sizeof(*export));
   if (!export)
      return -ENOMEM;

   export->drm_fd = drm_fd;

   int dmabuf_fd = -1;
   int err = crocus_bo_export_dmabuf(bo, &dmabuf_fd);
   if (err) {
      free(export);
      return err;
   }

   simple_mtx_lock(&bufmgr->lock);
   err = drmPrimeFDToHandle(drm_fd, dmabuf_fd, &export->gem_handle);
   close(dmabuf_fd);
   if (err) {
      simple_mtx_unlock(&bufmgr->lock);
      free(export);
      return err;
   }

   bool found = false;
   list_for_each_entry(struct bo_export, iter, &bo->exports, link) {
      if (iter->drm_fd != drm_fd)
         continue;
      /* Here we assume that for a given DRM fd, we'll always get back the
       * same GEM handle for a given buffer.
       */
      assert(iter->gem_handle == export->gem_handle);
      free(export);
      export = iter;
      found = true;
      break;
   }
   if (!found)
      list_addtail(&export->link, &bo->exports);

   simple_mtx_unlock(&bufmgr->lock);

   *out_handle = export->gem_handle;

   return 0;
}

static void
add_bucket(struct crocus_bufmgr *bufmgr, int size)
{
   unsigned int i = bufmgr->num_buckets;

   assert(i < ARRAY_SIZE(bufmgr->cache_bucket));

   list_inithead(&bufmgr->cache_bucket[i].head);
   bufmgr->cache_bucket[i].size = size;
   bufmgr->num_buckets++;

   assert(bucket_for_size(bufmgr, size) == &bufmgr->cache_bucket[i]);
   assert(bucket_for_size(bufmgr, size - 2048) == &bufmgr->cache_bucket[i]);
   assert(bucket_for_size(bufmgr, size + 1) != &bufmgr->cache_bucket[i]);
}

static void
init_cache_buckets(struct crocus_bufmgr *bufmgr)
{
   uint64_t size, cache_max_size = 64 * 1024 * 1024;

   /* OK, so power of two buckets was too wasteful of memory.
    * Give 3 other sizes between each power of two, to hopefully
    * cover things accurately enough.  (The alternative is
    * probably to just go for exact matching of sizes, and assume
    * that for things like composited window resize the tiled
    * width/height alignment and rounding of sizes to pages will
    * get us useful cache hit rates anyway)
    */
   add_bucket(bufmgr, PAGE_SIZE);
   add_bucket(bufmgr, PAGE_SIZE * 2);
   add_bucket(bufmgr, PAGE_SIZE * 3);

   /* Initialize the linked lists for BO reuse cache. */
   for (size = 4 * PAGE_SIZE; size <= cache_max_size; size *= 2) {
      add_bucket(bufmgr, size);

      add_bucket(bufmgr, size + size * 1 / 4);
      add_bucket(bufmgr, size + size * 2 / 4);
      add_bucket(bufmgr, size + size * 3 / 4);
   }
}

uint32_t
crocus_create_hw_context(struct crocus_bufmgr *bufmgr)
{
   uint32_t ctx_id;
   if (!intel_gem_create_context(bufmgr->fd, &ctx_id)) {
      DBG("intel_gem_create_context failed: %s\n", strerror(errno));
      return 0;
   }

   /* Upon declaring a GPU hang, the kernel will zap the guilty context
    * back to the default logical HW state and attempt to continue on to
    * our next submitted batchbuffer.  However, our render batches assume
    * the previous GPU state is preserved, and only emit commands needed
    * to incrementally change that state.  In particular, we inherit the
    * STATE_BASE_ADDRESS and PIPELINE_SELECT settings, which are critical.
    * With default base addresses, our next batches will almost certainly
    * cause more GPU hangs, leading to repeated hangs until we're banned
    * or the machine is dead.
    *
    * Here we tell the kernel not to attempt to recover our context but
    * immediately (on the next batchbuffer submission) report that the
    * context is lost, and we will do the recovery ourselves.  Ideally,
    * we'll have two lost batches instead of a continual stream of hangs.
    */
   intel_gem_set_context_param(bufmgr->fd, ctx_id,
                               I915_CONTEXT_PARAM_RECOVERABLE, false);

   return ctx_id;
}

static int
crocus_hw_context_get_priority(struct crocus_bufmgr *bufmgr, uint32_t ctx_id)
{
   uint64_t priority = 0;
   intel_gem_get_context_param(bufmgr->fd, ctx_id,
                               I915_CONTEXT_PARAM_PRIORITY, &priority);
   return priority; /* on error, return 0 i.e. default priority */
}

int
crocus_hw_context_set_priority(struct crocus_bufmgr *bufmgr,
                               uint32_t ctx_id,
                               int priority)
{
   int err = 0;
   if (!intel_gem_set_context_param(bufmgr->fd, ctx_id,
                                    I915_CONTEXT_PARAM_PRIORITY, priority))
      err = -errno;

   return err;
}

uint32_t
crocus_clone_hw_context(struct crocus_bufmgr *bufmgr, uint32_t ctx_id)
{
   uint32_t new_ctx = crocus_create_hw_context(bufmgr);

   if (new_ctx) {
      int priority = crocus_hw_context_get_priority(bufmgr, ctx_id);
      crocus_hw_context_set_priority(bufmgr, new_ctx, priority);
   }

   return new_ctx;
}

void
crocus_destroy_hw_context(struct crocus_bufmgr *bufmgr, uint32_t ctx_id)
{
   if (ctx_id != 0 &&
       !intel_gem_destroy_context(bufmgr->fd, ctx_id)) {
      fprintf(stderr, "DRM_IOCTL_I915_GEM_CONTEXT_DESTROY failed: %s\n",
              strerror(errno));
   }
}

/**
 * Initializes the GEM buffer manager, which uses the kernel to allocate, map,
 * and manage map buffer objections.
 *
 * \param fd File descriptor of the opened DRM device.
 */
static struct crocus_bufmgr *
crocus_bufmgr_create(struct intel_device_info *devinfo, int fd, bool bo_reuse)
{
   struct crocus_bufmgr *bufmgr = calloc(1, sizeof(*bufmgr));
   if (bufmgr == NULL)
      return NULL;

   /* Handles to buffer objects belong to the device fd and are not
    * reference counted by the kernel.  If the same fd is used by
    * multiple parties (threads sharing the same screen bufmgr, or
    * even worse the same device fd passed to multiple libraries)
    * ownership of those handles is shared by those independent parties.
    *
    * Don't do this! Ensure that each library/bufmgr has its own device
    * fd so that its namespace does not clash with another.
    */
   bufmgr->fd = os_dupfd_cloexec(fd);

   p_atomic_set(&bufmgr->refcount, 1);

   simple_mtx_init(&bufmgr->lock, mtx_plain);

   list_inithead(&bufmgr->zombie_list);

   bufmgr->has_llc = devinfo->has_llc;
   bufmgr->has_tiling_uapi = devinfo->has_tiling_uapi;
   bufmgr->bo_reuse = bo_reuse;
   bufmgr->has_mmap_offset = devinfo->has_mmap_offset;

   init_cache_buckets(bufmgr);

   bufmgr->name_table =
      _mesa_hash_table_create(NULL, key_hash_uint, key_uint_equal);
   bufmgr->handle_table =
      _mesa_hash_table_create(NULL, key_hash_uint, key_uint_equal);

   return bufmgr;
}

static struct crocus_bufmgr *
crocus_bufmgr_ref(struct crocus_bufmgr *bufmgr)
{
   p_atomic_inc(&bufmgr->refcount);
   return bufmgr;
}

void
crocus_bufmgr_unref(struct crocus_bufmgr *bufmgr)
{
   simple_mtx_lock(&global_bufmgr_list_mutex);
   if (p_atomic_dec_zero(&bufmgr->refcount)) {
      list_del(&bufmgr->link);
      crocus_bufmgr_destroy(bufmgr);
   }
   simple_mtx_unlock(&global_bufmgr_list_mutex);
}

/**
 * Gets an already existing GEM buffer manager or create a new one.
 *
 * \param fd File descriptor of the opened DRM device.
 */
struct crocus_bufmgr *
crocus_bufmgr_get_for_fd(struct intel_device_info *devinfo, int fd, bool bo_reuse)
{
   struct stat st;

   if (fstat(fd, &st))
      return NULL;

   struct crocus_bufmgr *bufmgr = NULL;

   simple_mtx_lock(&global_bufmgr_list_mutex);
   list_for_each_entry(struct crocus_bufmgr, iter_bufmgr, &global_bufmgr_list, link) {
      struct stat iter_st;
      if (fstat(iter_bufmgr->fd, &iter_st))
         continue;

      if (st.st_rdev == iter_st.st_rdev) {
         assert(iter_bufmgr->bo_reuse == bo_reuse);
         bufmgr = crocus_bufmgr_ref(iter_bufmgr);
         goto unlock;
      }
   }

   bufmgr = crocus_bufmgr_create(devinfo, fd, bo_reuse);
   if (bufmgr)
      list_addtail(&bufmgr->link, &global_bufmgr_list);

 unlock:
   simple_mtx_unlock(&global_bufmgr_list_mutex);

   return bufmgr;
}

int
crocus_bufmgr_get_fd(struct crocus_bufmgr *bufmgr)
{
   return bufmgr->fd;
}
