/*
 * Copyright (C) 2017-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "xf86drm.h"
#include "drm-uapi/lima_drm.h"

#include "util/u_hash_table.h"
#include "util/u_math.h"
#include "util/os_time.h"
#include "util/os_mman.h"

#include "frontend/drm_driver.h"

#include "lima_screen.h"
#include "lima_bo.h"
#include "lima_util.h"

bool lima_bo_table_init(struct lima_screen *screen)
{
   screen->bo_handles = util_hash_table_create_ptr_keys();
   if (!screen->bo_handles)
      return false;

   screen->bo_flink_names = util_hash_table_create_ptr_keys();
   if (!screen->bo_flink_names)
      goto err_out0;

   mtx_init(&screen->bo_table_lock, mtx_plain);
   return true;

err_out0:
   _mesa_hash_table_destroy(screen->bo_handles, NULL);
   return false;
}

bool lima_bo_cache_init(struct lima_screen *screen)
{
   mtx_init(&screen->bo_cache_lock, mtx_plain);
   list_inithead(&screen->bo_cache_time);
   for (int i = 0; i < NR_BO_CACHE_BUCKETS; i++)
      list_inithead(&screen->bo_cache_buckets[i]);

   return true;
}

void lima_bo_table_fini(struct lima_screen *screen)
{
   mtx_destroy(&screen->bo_table_lock);
   _mesa_hash_table_destroy(screen->bo_handles, NULL);
   _mesa_hash_table_destroy(screen->bo_flink_names, NULL);
}

static void
lima_bo_cache_remove(struct lima_bo *bo)
{
   list_del(&bo->size_list);
   list_del(&bo->time_list);
}

static void lima_close_kms_handle(struct lima_screen *screen, uint32_t handle)
{
   struct drm_gem_close args = {
      .handle = handle,
   };

   drmIoctl(screen->fd, DRM_IOCTL_GEM_CLOSE, &args);
}

static void
lima_bo_free(struct lima_bo *bo)
{
   struct lima_screen *screen = bo->screen;

   if (lima_debug & LIMA_DEBUG_BO_CACHE)
      fprintf(stderr, "%s: %p (size=%d)\n", __func__,
              bo, bo->size);

   mtx_lock(&screen->bo_table_lock);
   _mesa_hash_table_remove_key(screen->bo_handles,
                          (void *)(uintptr_t)bo->handle);
   if (bo->flink_name)
      _mesa_hash_table_remove_key(screen->bo_flink_names,
                             (void *)(uintptr_t)bo->flink_name);
   mtx_unlock(&screen->bo_table_lock);

   if (bo->map)
      lima_bo_unmap(bo);

   lima_close_kms_handle(screen, bo->handle);
   free(bo);
}

void lima_bo_cache_fini(struct lima_screen *screen)
{
   mtx_destroy(&screen->bo_cache_lock);

   list_for_each_entry_safe(struct lima_bo, entry,
                            &screen->bo_cache_time, time_list) {
      lima_bo_cache_remove(entry);
      lima_bo_free(entry);
   }
}

static bool lima_bo_get_info(struct lima_bo *bo)
{
   struct drm_lima_gem_info req = {
      .handle = bo->handle,
   };

   if(drmIoctl(bo->screen->fd, DRM_IOCTL_LIMA_GEM_INFO, &req))
      return false;

   bo->offset = req.offset;
   bo->va = req.va;
   return true;
}

static unsigned
lima_bucket_index(unsigned size)
{
   /* Round down to POT to compute a bucket index */

   unsigned bucket_index = util_logbase2(size);

   /* Clamp the bucket index; all huge allocations will be
    * sorted into the largest bucket */
   bucket_index = CLAMP(bucket_index, MIN_BO_CACHE_BUCKET,
                        MAX_BO_CACHE_BUCKET);

   /* Reindex from 0 */
   return (bucket_index - MIN_BO_CACHE_BUCKET);
}

static struct list_head *
lima_bo_cache_get_bucket(struct lima_screen *screen, unsigned size)
{
   return &screen->bo_cache_buckets[lima_bucket_index(size)];
}

static void
lima_bo_cache_free_stale_bos(struct lima_screen *screen, time_t time)
{
   unsigned cnt = 0;
   list_for_each_entry_safe(struct lima_bo, entry,
                            &screen->bo_cache_time, time_list) {
      /* Free BOs that are sitting idle for longer than 5 seconds */
      if (time - entry->free_time > 6) {
         lima_bo_cache_remove(entry);
         lima_bo_free(entry);
         cnt++;
      } else
         break;
   }
   if ((lima_debug & LIMA_DEBUG_BO_CACHE) && cnt)
      fprintf(stderr, "%s: freed %d stale BOs\n", __func__, cnt);
}

static void
lima_bo_cache_print_stats(struct lima_screen *screen)
{
   fprintf(stderr, "===============\n");
   fprintf(stderr, "BO cache stats:\n");
   unsigned total_size = 0;
   for (int i = 0; i < NR_BO_CACHE_BUCKETS; i++) {
      struct list_head *bucket = &screen->bo_cache_buckets[i];
      unsigned bucket_size = 0;
      list_for_each_entry(struct lima_bo, entry, bucket, size_list) {
         bucket_size += entry->size;
         total_size += entry->size;
      }
      fprintf(stderr, "Bucket #%d, BOs: %d, size: %u\n", i,
              list_length(bucket),
              bucket_size);
   }
   fprintf(stderr, "Total size: %u\n", total_size);
}

static bool
lima_bo_cache_put(struct lima_bo *bo)
{
   if (!bo->cacheable)
      return false;

   struct lima_screen *screen = bo->screen;

   mtx_lock(&screen->bo_cache_lock);
   struct list_head *bucket = lima_bo_cache_get_bucket(screen, bo->size);

   if (!bucket) {
      mtx_unlock(&screen->bo_cache_lock);
      return false;
   }

   struct timespec time;
   clock_gettime(CLOCK_MONOTONIC, &time);
   bo->free_time = time.tv_sec;
   list_addtail(&bo->size_list, bucket);
   list_addtail(&bo->time_list, &screen->bo_cache_time);
   lima_bo_cache_free_stale_bos(screen, time.tv_sec);
   if (lima_debug & LIMA_DEBUG_BO_CACHE) {
      fprintf(stderr, "%s: put BO: %p (size=%d)\n", __func__, bo, bo->size);
      lima_bo_cache_print_stats(screen);
   }
   mtx_unlock(&screen->bo_cache_lock);

   return true;
}

static struct lima_bo *
lima_bo_cache_get(struct lima_screen *screen, uint32_t size, uint32_t flags)
{
   /* we won't cache heap buffer */
   if (flags & LIMA_BO_FLAG_HEAP)
      return NULL;

   struct lima_bo *bo = NULL;
   mtx_lock(&screen->bo_cache_lock);
   struct list_head *bucket = lima_bo_cache_get_bucket(screen, size);

   if (!bucket) {
      mtx_unlock(&screen->bo_cache_lock);
      return false;
   }

   list_for_each_entry_safe(struct lima_bo, entry, bucket, size_list) {
      if (entry->size >= size) {
         /* Check if BO is idle. If it's not it's better to allocate new one */
         if (!lima_bo_wait(entry, LIMA_GEM_WAIT_WRITE, 0)) {
            if (lima_debug & LIMA_DEBUG_BO_CACHE) {
               fprintf(stderr, "%s: found BO %p but it's busy\n", __func__,
                       entry);
            }
            break;
         }

         lima_bo_cache_remove(entry);
         p_atomic_set(&entry->refcnt, 1);
         entry->flags = flags;
         bo = entry;
         if (lima_debug & LIMA_DEBUG_BO_CACHE) {
            fprintf(stderr, "%s: got BO: %p (size=%d), requested size %d\n",
                    __func__, bo, bo->size, size);
            lima_bo_cache_print_stats(screen);
         }
         break;
      }
   }

   mtx_unlock(&screen->bo_cache_lock);

   return bo;
}

struct lima_bo *lima_bo_create(struct lima_screen *screen,
                               uint32_t size, uint32_t flags)
{
   struct lima_bo *bo;

   size = align(size, LIMA_PAGE_SIZE);

   /* Try to get bo from cache first */
   bo = lima_bo_cache_get(screen, size, flags);
   if (bo)
      return bo;

   struct drm_lima_gem_create req = {
      .size = size,
      .flags = flags,
   };

   if (!(bo = calloc(1, sizeof(*bo))))
      return NULL;

   list_inithead(&bo->time_list);
   list_inithead(&bo->size_list);

   if (drmIoctl(screen->fd, DRM_IOCTL_LIMA_GEM_CREATE, &req))
      goto err_out0;

   bo->screen = screen;
   bo->size = req.size;
   bo->flags = req.flags;
   bo->handle = req.handle;
   bo->cacheable = !(lima_debug & LIMA_DEBUG_NO_BO_CACHE ||
                     flags & LIMA_BO_FLAG_HEAP);
   p_atomic_set(&bo->refcnt, 1);

   if (!lima_bo_get_info(bo))
      goto err_out1;

   if (lima_debug & LIMA_DEBUG_BO_CACHE)
      fprintf(stderr, "%s: %p (size=%d)\n", __func__,
              bo, bo->size);

   return bo;

err_out1:
   lima_close_kms_handle(screen, bo->handle);
err_out0:
   free(bo);
   return NULL;
}

void lima_bo_unreference(struct lima_bo *bo)
{
   if (!p_atomic_dec_zero(&bo->refcnt))
      return;

   /* Try to put it into cache */
   if (lima_bo_cache_put(bo))
      return;

   lima_bo_free(bo);
}

void *lima_bo_map(struct lima_bo *bo)
{
   if (!bo->map) {
      bo->map = os_mmap(0, bo->size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, bo->screen->fd, bo->offset);
      if (bo->map == MAP_FAILED)
          bo->map = NULL;
   }

   return bo->map;
}

void lima_bo_unmap(struct lima_bo *bo)
{
   if (bo->map) {
      os_munmap(bo->map, bo->size);
      bo->map = NULL;
   }
}

bool lima_bo_export(struct lima_bo *bo, struct winsys_handle *handle)
{
   struct lima_screen *screen = bo->screen;

   /* Don't cache exported BOs */
   bo->cacheable = false;

   switch (handle->type) {
   case WINSYS_HANDLE_TYPE_SHARED:
      if (!bo->flink_name) {
         struct drm_gem_flink flink = {
            .handle = bo->handle,
            .name = 0,
         };
         if (drmIoctl(screen->fd, DRM_IOCTL_GEM_FLINK, &flink))
            return false;

         bo->flink_name = flink.name;

         mtx_lock(&screen->bo_table_lock);
         _mesa_hash_table_insert(screen->bo_flink_names,
                             (void *)(uintptr_t)bo->flink_name, bo);
         mtx_unlock(&screen->bo_table_lock);
      }
      handle->handle = bo->flink_name;
      return true;

   case WINSYS_HANDLE_TYPE_KMS:
      mtx_lock(&screen->bo_table_lock);
      _mesa_hash_table_insert(screen->bo_handles,
                          (void *)(uintptr_t)bo->handle, bo);
      mtx_unlock(&screen->bo_table_lock);

      handle->handle = bo->handle;
      return true;

   case WINSYS_HANDLE_TYPE_FD:
      if (drmPrimeHandleToFD(screen->fd, bo->handle, DRM_CLOEXEC,
                             (int*)&handle->handle))
         return false;

      mtx_lock(&screen->bo_table_lock);
      _mesa_hash_table_insert(screen->bo_handles,
                          (void *)(uintptr_t)bo->handle, bo);
      mtx_unlock(&screen->bo_table_lock);
      return true;

   default:
      return false;
   }
}

struct lima_bo *lima_bo_import(struct lima_screen *screen,
                               struct winsys_handle *handle)
{
   struct lima_bo *bo = NULL;
   struct drm_gem_open req = {0};
   uint32_t dma_buf_size = 0;
   unsigned h = handle->handle;

   mtx_lock(&screen->bo_table_lock);

   /* Convert a DMA buf handle to a KMS handle now. */
   if (handle->type == WINSYS_HANDLE_TYPE_FD) {
      uint32_t prime_handle;
      off_t size;

      /* Get a KMS handle. */
      if (drmPrimeFDToHandle(screen->fd, h, &prime_handle)) {
         mtx_unlock(&screen->bo_table_lock);
         return NULL;
      }

      /* Query the buffer size. */
      size = lseek(h, 0, SEEK_END);
      if (size == (off_t)-1) {
         mtx_unlock(&screen->bo_table_lock);
         lima_close_kms_handle(screen, prime_handle);
         return NULL;
      }
      lseek(h, 0, SEEK_SET);

      dma_buf_size = size;
      h = prime_handle;
   }

   switch (handle->type) {
   case WINSYS_HANDLE_TYPE_SHARED:
      bo = util_hash_table_get(screen->bo_flink_names,
                               (void *)(uintptr_t)h);
      break;
   case WINSYS_HANDLE_TYPE_KMS:
   case WINSYS_HANDLE_TYPE_FD:
      bo = util_hash_table_get(screen->bo_handles,
                               (void *)(uintptr_t)h);
      break;
   default:
      mtx_unlock(&screen->bo_table_lock);
      return NULL;
   }

   if (bo) {
      p_atomic_inc(&bo->refcnt);
      /* Don't cache imported BOs */
      bo->cacheable = false;
      mtx_unlock(&screen->bo_table_lock);
      return bo;
   }

   if (!(bo = calloc(1, sizeof(*bo)))) {
      mtx_unlock(&screen->bo_table_lock);
      if (handle->type == WINSYS_HANDLE_TYPE_FD)
         lima_close_kms_handle(screen, h);
      return NULL;
   }

   /* Don't cache imported BOs */
   bo->cacheable = false;
   list_inithead(&bo->time_list);
   list_inithead(&bo->size_list);
   bo->screen = screen;
   p_atomic_set(&bo->refcnt, 1);

   switch (handle->type) {
   case WINSYS_HANDLE_TYPE_SHARED:
      req.name = h;
      if (drmIoctl(screen->fd, DRM_IOCTL_GEM_OPEN, &req)) {
         mtx_unlock(&screen->bo_table_lock);
         free(bo);
         return NULL;
      }
      bo->handle = req.handle;
      bo->flink_name = h;
      bo->size = req.size;
      break;
   case WINSYS_HANDLE_TYPE_FD:
      bo->handle = h;
      bo->size = dma_buf_size;
      break;
   default:
      /* not possible */
      assert(0);
   }

   if (lima_bo_get_info(bo)) {
      if (handle->type == WINSYS_HANDLE_TYPE_SHARED)
         _mesa_hash_table_insert(screen->bo_flink_names,
                          (void *)(uintptr_t)bo->flink_name, bo);
      _mesa_hash_table_insert(screen->bo_handles,
                          (void*)(uintptr_t)bo->handle, bo);
   }
   else {
      lima_close_kms_handle(screen, bo->handle);
      free(bo);
      bo = NULL;
   }

   mtx_unlock(&screen->bo_table_lock);

   return bo;
}

bool lima_bo_wait(struct lima_bo *bo, uint32_t op, uint64_t timeout_ns)
{
   int64_t abs_timeout;

   if (timeout_ns == 0)
      abs_timeout = 0;
   else
      abs_timeout = os_time_get_absolute_timeout(timeout_ns);

   if (abs_timeout == OS_TIMEOUT_INFINITE)
      abs_timeout = INT64_MAX;

   struct drm_lima_gem_wait req = {
      .handle = bo->handle,
      .op = op,
      .timeout_ns = abs_timeout,
   };

   return drmIoctl(bo->screen->fd, DRM_IOCTL_LIMA_GEM_WAIT, &req) == 0;
}
