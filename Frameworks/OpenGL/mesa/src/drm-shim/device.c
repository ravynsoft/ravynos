/*
 * Copyright © 2018 Broadcom
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file
 *
 * Implements core GEM support (particularly ioctls) underneath the libc ioctl
 * wrappers, and calls into the driver-specific code as necessary.
 */

#include <c11/threads.h>
#include <errno.h>
#include <linux/memfd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "drm-uapi/drm.h"
#include "drm_shim.h"
#include "util/hash_table.h"
#include "util/u_atomic.h"

#define SHIM_MEM_SIZE (4ull * 1024 * 1024 * 1024)

#ifndef HAVE_MEMFD_CREATE
#include <sys/syscall.h>

static inline int
memfd_create(const char *name, unsigned int flags)
{
   return syscall(SYS_memfd_create, name, flags);
}
#endif

/* Global state for the shim shared between libc, core, and driver. */
struct shim_device shim_device;

long shim_page_size;

static uint32_t
uint_key_hash(const void *key)
{
   return (uintptr_t)key;
}

static bool
uint_key_compare(const void *a, const void *b)
{
   return a == b;
}

/**
 * Called when the first libc shim is called, to initialize GEM simulation
 * state (other than the shims themselves).
 */
void
drm_shim_device_init(void)
{
   shim_device.fd_map = _mesa_hash_table_create(NULL,
                                                uint_key_hash,
                                                uint_key_compare);

   shim_device.offset_map = _mesa_hash_table_u64_create(NULL);

   mtx_init(&shim_device.mem_lock, mtx_plain);

   shim_device.mem_fd = memfd_create("shim mem", MFD_CLOEXEC);
   assert(shim_device.mem_fd != -1);

   ASSERTED int ret = ftruncate(shim_device.mem_fd, SHIM_MEM_SIZE);
   assert(ret == 0);

   /* The man page for mmap() says
    *
    *    offset must be a multiple of the page size as returned by
    *    sysconf(_SC_PAGE_SIZE).
    *
    * Depending on the configuration of the kernel, this may not be 4096. Get
    * this page size once and use it as the page size throughout, ensuring that
    * are offsets are page-size aligned as required. Otherwise, mmap will fail
    * with EINVAL.
    */

   shim_page_size = sysconf(_SC_PAGE_SIZE);

   util_vma_heap_init(&shim_device.mem_heap, shim_page_size,
                      SHIM_MEM_SIZE - shim_page_size);

   drm_shim_driver_init();
}

static struct shim_fd *
drm_shim_file_create(int fd)
{
   struct shim_fd *shim_fd = calloc(1, sizeof(*shim_fd));

   shim_fd->fd = fd;
   p_atomic_set(&shim_fd->refcount, 1);
   mtx_init(&shim_fd->handle_lock, mtx_plain);
   shim_fd->handles = _mesa_hash_table_create(NULL,
                                              uint_key_hash,
                                              uint_key_compare);

   return shim_fd;
}

/**
 * Called when the libc shims have interposed an open or dup of our simulated
 * DRM device.
 */
void drm_shim_fd_register(int fd, struct shim_fd *shim_fd)
{
   if (!shim_fd)
      shim_fd = drm_shim_file_create(fd);
   else
      p_atomic_inc(&shim_fd->refcount);

   _mesa_hash_table_insert(shim_device.fd_map, (void *)(uintptr_t)(fd + 1), shim_fd);
}

static void handle_delete_fxn(struct hash_entry *entry)
{
   drm_shim_bo_put(entry->data);
}

void drm_shim_fd_unregister(int fd)
{
   if (fd == -1)
      return;

   struct hash_entry *entry =
         _mesa_hash_table_search(shim_device.fd_map, (void *)(uintptr_t)(fd + 1));
   if (!entry)
      return;
   struct shim_fd *shim_fd = entry->data;
   _mesa_hash_table_remove(shim_device.fd_map, entry);

   if (!p_atomic_dec_zero(&shim_fd->refcount))
      return;

   _mesa_hash_table_destroy(shim_fd->handles, handle_delete_fxn);
   free(shim_fd);
}

struct shim_fd *
drm_shim_fd_lookup(int fd)
{
   if (fd == -1)
      return NULL;

   struct hash_entry *entry =
      _mesa_hash_table_search(shim_device.fd_map, (void *)(uintptr_t)(fd + 1));

   if (!entry)
      return NULL;
   return entry->data;
}

/* ioctl used by drmGetVersion() */
static int
drm_shim_ioctl_version(int fd, unsigned long request, void *arg)
{
   struct drm_version *args = arg;
   const char *date = "20190320";
   const char *desc = "shim";

   args->version_major = shim_device.version_major;
   args->version_minor = shim_device.version_minor;
   args->version_patchlevel = shim_device.version_patchlevel;

   if (args->name)
      strncpy(args->name, shim_device.driver_name, args->name_len);
   if (args->date)
      strncpy(args->date, date, args->date_len);
   if (args->desc)
      strncpy(args->desc, desc, args->desc_len);
   args->name_len = strlen(shim_device.driver_name);
   args->date_len = strlen(date);
   args->desc_len = strlen(desc);

   return 0;
}

static int
drm_shim_ioctl_get_unique(int fd, unsigned long request, void *arg)
{
   struct drm_unique *gu = arg;

   if (gu->unique && shim_device.unique)
      strncpy(gu->unique, shim_device.unique, gu->unique_len);
   gu->unique_len = shim_device.unique ? strlen(shim_device.unique) : 0;

   return 0;
}

static int
drm_shim_ioctl_get_cap(int fd, unsigned long request, void *arg)
{
   struct drm_get_cap *gc = arg;

   switch (gc->capability) {
   case DRM_CAP_PRIME:
   case DRM_CAP_SYNCOBJ:
   case DRM_CAP_SYNCOBJ_TIMELINE:
      gc->value = 1;
      return 0;

   default:
      fprintf(stderr, "DRM_IOCTL_GET_CAP: unhandled 0x%x\n",
              (int)gc->capability);
      return -1;
   }
}

static int
drm_shim_ioctl_gem_close(int fd, unsigned long request, void *arg)
{
   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct drm_gem_close *c = arg;

   if (!c->handle)
      return 0;

   mtx_lock(&shim_fd->handle_lock);
   struct hash_entry *entry =
      _mesa_hash_table_search(shim_fd->handles, (void *)(uintptr_t)c->handle);
   if (!entry) {
      mtx_unlock(&shim_fd->handle_lock);
      return -EINVAL;
   }

   struct shim_bo *bo = entry->data;
   _mesa_hash_table_remove(shim_fd->handles, entry);
   drm_shim_bo_put(bo);
   mtx_unlock(&shim_fd->handle_lock);
   return 0;
}

static int
drm_shim_ioctl_syncobj_create(int fd, unsigned long request, void *arg)
{
   struct drm_syncobj_create *create = arg;

   create->handle = 1; /* 0 is invalid */

   return 0;
}

static int
drm_shim_ioctl_stub(int fd, unsigned long request, void *arg)
{
   return 0;
}

ioctl_fn_t core_ioctls[] = {
   [_IOC_NR(DRM_IOCTL_VERSION)] = drm_shim_ioctl_version,
   [_IOC_NR(DRM_IOCTL_GET_UNIQUE)] = drm_shim_ioctl_get_unique,
   [_IOC_NR(DRM_IOCTL_GET_CAP)] = drm_shim_ioctl_get_cap,
   [_IOC_NR(DRM_IOCTL_GEM_CLOSE)] = drm_shim_ioctl_gem_close,
   [_IOC_NR(DRM_IOCTL_SYNCOBJ_CREATE)] = drm_shim_ioctl_syncobj_create,
   [_IOC_NR(DRM_IOCTL_SYNCOBJ_DESTROY)] = drm_shim_ioctl_stub,
   [_IOC_NR(DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD)] = drm_shim_ioctl_stub,
   [_IOC_NR(DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE)] = drm_shim_ioctl_stub,
   [_IOC_NR(DRM_IOCTL_SYNCOBJ_WAIT)] = drm_shim_ioctl_stub,
   [_IOC_NR(DRM_IOCTL_SYNCOBJ_TRANSFER)] = drm_shim_ioctl_stub,
};

/**
 * Implements the GEM core ioctls, and calls into driver-specific ioctls.
 */
int
drm_shim_ioctl(int fd, unsigned long request, void *arg)
{
   ASSERTED int type = _IOC_TYPE(request);
   int nr = _IOC_NR(request);

   assert(type == DRM_IOCTL_BASE);

   if (nr >= DRM_COMMAND_BASE && nr < DRM_COMMAND_END) {
      int driver_nr = nr - DRM_COMMAND_BASE;

      if (driver_nr < shim_device.driver_ioctl_count &&
          shim_device.driver_ioctls[driver_nr]) {
         return shim_device.driver_ioctls[driver_nr](fd, request, arg);
      }
   } else {
      if (nr < ARRAY_SIZE(core_ioctls) && core_ioctls[nr]) {
         return core_ioctls[nr](fd, request, arg);
      }
   }

   if (nr >= DRM_COMMAND_BASE && nr < DRM_COMMAND_END) {
      fprintf(stderr,
              "DRM_SHIM: unhandled driver DRM ioctl %d (0x%08lx)\n",
              nr - DRM_COMMAND_BASE, request);
   } else {
      fprintf(stderr,
              "DRM_SHIM: unhandled core DRM ioctl 0x%X (0x%08lx)\n",
              nr, request);
   }

   return -EINVAL;
}

int
drm_shim_bo_init(struct shim_bo *bo, size_t size)
{

   mtx_lock(&shim_device.mem_lock);
   bo->mem_addr = util_vma_heap_alloc(&shim_device.mem_heap, size, shim_page_size);
   mtx_unlock(&shim_device.mem_lock);

   if (!bo->mem_addr)
      return -ENOMEM;

   bo->size = size;

   return 0;
}

struct shim_bo *
drm_shim_bo_lookup(struct shim_fd *shim_fd, int handle)
{
   if (!handle)
      return NULL;

   mtx_lock(&shim_fd->handle_lock);
   struct hash_entry *entry =
      _mesa_hash_table_search(shim_fd->handles, (void *)(uintptr_t)handle);
   struct shim_bo *bo = entry ? entry->data : NULL;
   mtx_unlock(&shim_fd->handle_lock);

   if (bo)
      p_atomic_inc(&bo->refcount);

   return bo;
}

void
drm_shim_bo_get(struct shim_bo *bo)
{
   p_atomic_inc(&bo->refcount);
}

void
drm_shim_bo_put(struct shim_bo *bo)
{
   if (p_atomic_dec_return(&bo->refcount) == 0)
      return;

   if (shim_device.driver_bo_free)
      shim_device.driver_bo_free(bo);

   mtx_lock(&shim_device.mem_lock);
   util_vma_heap_free(&shim_device.mem_heap, bo->mem_addr, bo->size);
   mtx_unlock(&shim_device.mem_lock);
   free(bo);
}

int
drm_shim_bo_get_handle(struct shim_fd *shim_fd, struct shim_bo *bo)
{
   /* We should probably have some real datastructure for finding the free
    * number.
    */
   mtx_lock(&shim_fd->handle_lock);
   for (int new_handle = 1; ; new_handle++) {
      void *key = (void *)(uintptr_t)new_handle;
      if (!_mesa_hash_table_search(shim_fd->handles, key)) {
         drm_shim_bo_get(bo);
         _mesa_hash_table_insert(shim_fd->handles, key, bo);
         mtx_unlock(&shim_fd->handle_lock);
         return new_handle;
      }
   }
   mtx_unlock(&shim_fd->handle_lock);

   return 0;
}

/* Creates an mmap offset for the BO in the DRM fd.
 */
uint64_t
drm_shim_bo_get_mmap_offset(struct shim_fd *shim_fd, struct shim_bo *bo)
{
   mtx_lock(&shim_device.mem_lock);
   _mesa_hash_table_u64_insert(shim_device.offset_map, bo->mem_addr, bo);
   mtx_unlock(&shim_device.mem_lock);

   /* reuse the buffer address as the mmap offset: */
   return bo->mem_addr;
}

/* For mmap() on the DRM fd, look up the BO from the "offset" and map the BO's
 * fd.
 */
void *
drm_shim_mmap(struct shim_fd *shim_fd, size_t length, int prot, int flags,
              int fd, off64_t offset)
{
   mtx_lock(&shim_device.mem_lock);
   struct shim_bo *bo = _mesa_hash_table_u64_search(shim_device.offset_map, offset);
   mtx_unlock(&shim_device.mem_lock);

   if (!bo)
      return MAP_FAILED;

   if (length > bo->size)
      return MAP_FAILED;

   /* The offset we pass to mmap must be aligned to the page size */
   assert((bo->mem_addr & (shim_page_size - 1)) == 0);

   return mmap(NULL, length, prot, flags, shim_device.mem_fd, bo->mem_addr);
}
