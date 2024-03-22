/*
 * Copyright © 2019 Raspberry Pi Ltd
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

#include "v3dv_private.h"

#include <errno.h>
#include <sys/mman.h>

#include "drm-uapi/v3d_drm.h"
#include "util/u_memory.h"

/* Default max size of the bo cache, in MB.
 *
 * This value comes from testing different Vulkan application. Greater values
 * didn't get any further performance benefit. This looks somewhat small, but
 * from testing those applications, the main consumer of the bo cache are
 * the bos used for the CLs, that are usually small.
 */
#define DEFAULT_MAX_BO_CACHE_SIZE 64

/* Discarded to use a V3D_DEBUG for this, as it would mean adding a run-time
 * check for most of the calls
 */
static const bool dump_stats = false;

static void
bo_dump_stats(struct v3dv_device *device)
{
   struct v3dv_bo_cache *cache = &device->bo_cache;

   fprintf(stderr, "  BOs allocated:   %d\n", device->bo_count);
   fprintf(stderr, "  BOs size:        %dkb\n", device->bo_size / 1024);
   fprintf(stderr, "  BOs cached:      %d\n", cache->cache_count);
   fprintf(stderr, "  BOs cached size: %dkb\n", cache->cache_size / 1024);

   if (!list_is_empty(&cache->time_list)) {
      struct v3dv_bo *first = list_first_entry(&cache->time_list,
                                              struct v3dv_bo,
                                              time_list);
      struct v3dv_bo *last = list_last_entry(&cache->time_list,
                                            struct v3dv_bo,
                                            time_list);

      fprintf(stderr, "  oldest cache time: %ld\n",
              (long)first->free_time);
      fprintf(stderr, "  newest cache time: %ld\n",
              (long)last->free_time);

      struct timespec time;
      clock_gettime(CLOCK_MONOTONIC, &time);
      fprintf(stderr, "  now:               %lld\n",
              (long long)time.tv_sec);
   }

   if (cache->size_list_size) {
      uint32_t empty_size_list = 0;
      for (uint32_t i = 0; i < cache->size_list_size; i++) {
         if (list_is_empty(&cache->size_list[i]))
            empty_size_list++;
      }
      fprintf(stderr, "  Empty size_list lists: %d\n", empty_size_list);
   }
}

static void
bo_remove_from_cache(struct v3dv_bo_cache *cache, struct v3dv_bo *bo)
{
   list_del(&bo->time_list);
   list_del(&bo->size_list);

   cache->cache_count--;
   cache->cache_size -= bo->size;
}

static struct v3dv_bo *
bo_from_cache(struct v3dv_device *device, uint32_t size, const char *name)
{
   struct v3dv_bo_cache *cache = &device->bo_cache;
   uint32_t page_index = size / 4096 - 1;

   if (cache->size_list_size <= page_index)
      return NULL;

   struct v3dv_bo *bo = NULL;

   mtx_lock(&cache->lock);
   if (!list_is_empty(&cache->size_list[page_index])) {
      bo = list_first_entry(&cache->size_list[page_index],
                            struct v3dv_bo, size_list);

      /* Check that the BO has gone idle.  If not, then we want to
       * allocate something new instead, since we assume that the
       * user will proceed to CPU map it and fill it with stuff.
       */
      if (!v3dv_bo_wait(device, bo, 0)) {
         mtx_unlock(&cache->lock);
         return NULL;
      }

      bo_remove_from_cache(cache, bo);
      bo->name = name;
      p_atomic_set(&bo->refcnt, 1);
   }
   mtx_unlock(&cache->lock);
   return bo;
}

static bool
bo_free(struct v3dv_device *device,
        struct v3dv_bo *bo)
{
   if (!bo)
      return true;

   assert(p_atomic_read(&bo->refcnt) == 0);
   assert(bo->map == NULL);

   if (!bo->is_import) {
      device->bo_count--;
      device->bo_size -= bo->size;

      if (dump_stats) {
         fprintf(stderr, "Freed %s%s%dkb:\n",
                 bo->name ? bo->name : "",
                 bo->name ? " " : "",
                 bo->size / 1024);
         bo_dump_stats(device);
      }
   }

   uint32_t handle = bo->handle;
   /* Our BO structs are stored in a sparse array in the physical device,
    * so we don't want to free the BO pointer, instead we want to reset it
    * to 0, to signal that array entry as being free.
    *
    * We must do the reset before we actually free the BO in the kernel, since
    * otherwise there is a chance the application creates another BO in a
    * different thread and gets the same array entry, causing a race.
    */
   memset(bo, 0, sizeof(*bo));

   struct drm_gem_close c;
   memset(&c, 0, sizeof(c));
   c.handle = handle;
   int ret = v3dv_ioctl(device->pdevice->render_fd, DRM_IOCTL_GEM_CLOSE, &c);
   if (ret != 0)
      fprintf(stderr, "close object %d: %s\n", handle, strerror(errno));

   return ret == 0;
}

static void
bo_cache_free_all(struct v3dv_device *device,
                       bool with_lock)
{
   struct v3dv_bo_cache *cache = &device->bo_cache;

   if (with_lock)
      mtx_lock(&cache->lock);
   list_for_each_entry_safe(struct v3dv_bo, bo, &cache->time_list,
                            time_list) {
      bo_remove_from_cache(cache, bo);
      bo_free(device, bo);
   }
   if (with_lock)
      mtx_unlock(&cache->lock);

}

void
v3dv_bo_init(struct v3dv_bo *bo,
             uint32_t handle,
             uint32_t size,
             uint32_t offset,
             const char *name,
             bool private)
{
   p_atomic_set(&bo->refcnt, 1);
   bo->handle = handle;
   bo->handle_bit = 1ull << (handle % 64);
   bo->size = size;
   bo->offset = offset;
   bo->map = NULL;
   bo->map_size = 0;
   bo->name = name;
   bo->private = private;
   bo->dumb_handle = -1;
   bo->is_import = false;
   list_inithead(&bo->list_link);
}

void
v3dv_bo_init_import(struct v3dv_bo *bo,
                    uint32_t handle,
                    uint32_t size,
                    uint32_t offset,
                    bool private)
{
   v3dv_bo_init(bo, handle, size, offset, "import", private);
   bo->is_import = true;
}

struct v3dv_bo *
v3dv_bo_alloc(struct v3dv_device *device,
              uint32_t size,
              const char *name,
              bool private)
{
   struct v3dv_bo *bo;

   const uint32_t page_align = 4096; /* Always allocate full pages */
   size = align(size, page_align);

   if (private) {
      bo = bo_from_cache(device, size, name);
      if (bo) {
         if (dump_stats) {
            fprintf(stderr, "Allocated %s %dkb from cache:\n",
                    name, size / 1024);
            bo_dump_stats(device);
         }
         return bo;
      }
   }

 retry:
   ;

   bool cleared_and_retried = false;
   struct drm_v3d_create_bo create = {
      .size = size
   };

   int ret = v3dv_ioctl(device->pdevice->render_fd,
                        DRM_IOCTL_V3D_CREATE_BO, &create);
   if (ret != 0) {
      if (!list_is_empty(&device->bo_cache.time_list) &&
          !cleared_and_retried) {
         cleared_and_retried = true;
         bo_cache_free_all(device, true);
         goto retry;
      }

      fprintf(stderr, "Failed to allocate device memory for BO\n");
      return NULL;
   }

   assert(create.offset % page_align == 0);
   assert((create.offset & 0xffffffff) == create.offset);

   bo = v3dv_device_lookup_bo(device->pdevice, create.handle);
   assert(bo && bo->handle == 0);

   v3dv_bo_init(bo, create.handle, size, create.offset, name, private);

   device->bo_count++;
   device->bo_size += bo->size;
   if (dump_stats) {
      fprintf(stderr, "Allocated %s %dkb:\n", name, size / 1024);
      bo_dump_stats(device);
   }

   return bo;
}

bool
v3dv_bo_map_unsynchronized(struct v3dv_device *device,
                           struct v3dv_bo *bo,
                           uint32_t size)
{
   assert(bo != NULL && size <= bo->size);

   if (bo->map)
      return bo->map;

   struct drm_v3d_mmap_bo map;
   memset(&map, 0, sizeof(map));
   map.handle = bo->handle;
   int ret = v3dv_ioctl(device->pdevice->render_fd,
                        DRM_IOCTL_V3D_MMAP_BO, &map);
   if (ret != 0) {
      fprintf(stderr, "map ioctl failure\n");
      return false;
   }

   bo->map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                  device->pdevice->render_fd, map.offset);
   if (bo->map == MAP_FAILED) {
      fprintf(stderr, "mmap of bo %d (offset 0x%016llx, size %d) failed\n",
              bo->handle, (long long)map.offset, (uint32_t)bo->size);
      return false;
   }
   VG(VALGRIND_MALLOCLIKE_BLOCK(bo->map, bo->size, 0, false));

   bo->map_size = size;

   return true;
}

bool
v3dv_bo_wait(struct v3dv_device *device,
             struct v3dv_bo *bo,
             uint64_t timeout_ns)
{
   struct drm_v3d_wait_bo wait = {
      .handle = bo->handle,
      .timeout_ns = timeout_ns,
   };
   return v3dv_ioctl(device->pdevice->render_fd,
                     DRM_IOCTL_V3D_WAIT_BO, &wait) == 0;
}

bool
v3dv_bo_map(struct v3dv_device *device, struct v3dv_bo *bo, uint32_t size)
{
   assert(bo && size <= bo->size);

   bool ok = v3dv_bo_map_unsynchronized(device, bo, size);
   if (!ok)
      return false;

   ok = v3dv_bo_wait(device, bo, OS_TIMEOUT_INFINITE);
   if (!ok) {
      fprintf(stderr, "memory wait for map failed\n");
      return false;
   }

   return true;
}

void
v3dv_bo_unmap(struct v3dv_device *device, struct v3dv_bo *bo)
{
   assert(bo && bo->map && bo->map_size > 0);

   munmap(bo->map, bo->map_size);
   VG(VALGRIND_FREELIKE_BLOCK(bo->map, 0));
   bo->map = NULL;
   bo->map_size = 0;
}

static bool
reallocate_size_list(struct v3dv_bo_cache *cache,
                     struct v3dv_device *device,
                     uint32_t size)
{
   struct list_head *new_list =
      vk_alloc(&device->vk.alloc, sizeof(struct list_head) * size, 8,
               VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

   if (!new_list) {
      fprintf(stderr, "Failed to allocate host memory for cache bo list\n");
      return false;
   }
   struct list_head *old_list = cache->size_list;

   /* Move old list contents over (since the array has moved, and
    * therefore the pointers to the list heads have to change).
    */
   for (int i = 0; i < cache->size_list_size; i++) {
      struct list_head *old_head = &cache->size_list[i];
      if (list_is_empty(old_head)) {
         list_inithead(&new_list[i]);
      } else {
         new_list[i].next = old_head->next;
         new_list[i].prev = old_head->prev;
         new_list[i].next->prev = &new_list[i];
         new_list[i].prev->next = &new_list[i];
      }
   }
   for (int i = cache->size_list_size; i < size; i++)
      list_inithead(&new_list[i]);

   cache->size_list = new_list;
   cache->size_list_size = size;
   vk_free(&device->vk.alloc, old_list);

   return true;
}

void
v3dv_bo_cache_init(struct v3dv_device *device)
{
   device->bo_size = 0;
   device->bo_count = 0;
   list_inithead(&device->bo_cache.time_list);
   /* FIXME: perhaps set a initial size for the size-list, to avoid run-time
    * reallocations
    */
   device->bo_cache.size_list_size = 0;

   const char *max_cache_size_str = getenv("V3DV_MAX_BO_CACHE_SIZE");
   if (max_cache_size_str == NULL)
      device->bo_cache.max_cache_size = DEFAULT_MAX_BO_CACHE_SIZE;
   else
      device->bo_cache.max_cache_size = atoll(max_cache_size_str);

   if (dump_stats) {
      fprintf(stderr, "MAX BO CACHE SIZE: %iMB\n", device->bo_cache.max_cache_size);
   }

   device->bo_cache.max_cache_size *= 1024 * 1024;
   device->bo_cache.cache_count = 0;
   device->bo_cache.cache_size = 0;
}

void
v3dv_bo_cache_destroy(struct v3dv_device *device)
{
   bo_cache_free_all(device, true);
   vk_free(&device->vk.alloc, device->bo_cache.size_list);

   if (dump_stats) {
      fprintf(stderr, "BO stats after screen destroy:\n");
      bo_dump_stats(device);
   }
}


static void
free_stale_bos(struct v3dv_device *device,
               time_t time)
{
   struct v3dv_bo_cache *cache = &device->bo_cache;
   bool freed_any = false;

   list_for_each_entry_safe(struct v3dv_bo, bo, &cache->time_list,
                            time_list) {
      /* If it's more than a second old, free it. */
      if (time - bo->free_time > 2) {
         if (dump_stats && !freed_any) {
            fprintf(stderr, "Freeing stale BOs:\n");
            bo_dump_stats(device);
            freed_any = true;
         }

         bo_remove_from_cache(cache, bo);
         bo_free(device, bo);
      } else {
         break;
      }
   }

   if (dump_stats && freed_any) {
      fprintf(stderr, "Freed stale BOs:\n");
      bo_dump_stats(device);
   }
}

bool
v3dv_bo_free(struct v3dv_device *device,
             struct v3dv_bo *bo)
{
   if (!bo)
      return true;

   if (!p_atomic_dec_zero(&bo->refcnt))
      return true;

   if (bo->map)
      v3dv_bo_unmap(device, bo);

   struct timespec time;
   struct v3dv_bo_cache *cache = &device->bo_cache;
   uint32_t page_index = bo->size / 4096 - 1;

   if (bo->private &&
       bo->size > cache->max_cache_size - cache->cache_size) {
      clock_gettime(CLOCK_MONOTONIC, &time);
      mtx_lock(&cache->lock);
      free_stale_bos(device, time.tv_sec);
      mtx_unlock(&cache->lock);
   }

   if (!bo->private ||
       bo->size > cache->max_cache_size - cache->cache_size) {
      return bo_free(device, bo);
   }

   clock_gettime(CLOCK_MONOTONIC, &time);
   mtx_lock(&cache->lock);

   if (cache->size_list_size <= page_index) {
      if (!reallocate_size_list(cache, device, page_index + 1)) {
         bool outcome = bo_free(device, bo);
         /* If the reallocation failed, it usually means that we are out of
          * memory, so we also free all the bo cache. We need to call it to
          * not use the cache lock, as we are already under it.
          */
         bo_cache_free_all(device, false);
         mtx_unlock(&cache->lock);
         return outcome;
      }
   }

   bo->free_time = time.tv_sec;
   list_addtail(&bo->size_list, &cache->size_list[page_index]);
   list_addtail(&bo->time_list, &cache->time_list);

   cache->cache_count++;
   cache->cache_size += bo->size;

   if (dump_stats) {
      fprintf(stderr, "Freed %s %dkb to cache:\n",
              bo->name, bo->size / 1024);
      bo_dump_stats(device);
   }
   bo->name = NULL;

   free_stale_bos(device, time.tv_sec);

   mtx_unlock(&cache->lock);

   return true;
}
