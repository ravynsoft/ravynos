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

#include <c11/threads.h>

#include "util/macros.h"
#include "util/hash_table.h"
#include "util/vma.h"

#include <xf86drm.h>

#ifdef __linux__
#define DRM_MAJOR 226
#endif

typedef int (*ioctl_fn_t)(int fd, unsigned long request, void *arg);

struct shim_bo;

struct shim_device {
   /* Mapping from int fd to struct shim_fd *. */
   struct hash_table *fd_map;

   /* Mapping from mmap offset to shim_bo */
   struct hash_table_u64 *offset_map;

   mtx_t mem_lock;
   /* Heap from which shim_bo are allocated */
   struct util_vma_heap mem_heap;

   int mem_fd;

   int (**driver_ioctls)(int fd, unsigned long request, void *arg);
   int driver_ioctl_count;

   void (*driver_bo_free)(struct shim_bo *bo);

   /* Returned by drmGetVersion(). */
   const char *driver_name;

   /* Returned by drmGetBusid(). */
   const char *unique;

   int version_major, version_minor, version_patchlevel;
   int bus_type;
};

extern struct shim_device shim_device;

struct shim_fd {
   int fd;
   int refcount;
   mtx_t handle_lock;
   /* mapping from int gem handle to struct shim_bo *. */
   struct hash_table *handles;
};

struct shim_bo {
   uint64_t mem_addr;
   void *map;
   int refcount;
   uint32_t size;
};

/* Core support. */
extern int render_node_minor;
void drm_shim_device_init(void);
void drm_shim_override_file(const char *contents,
                            const char *path_format, ...) PRINTFLIKE(2, 3);
void drm_shim_fd_register(int fd, struct shim_fd *shim_fd);
void drm_shim_fd_unregister(int fd);
struct shim_fd *drm_shim_fd_lookup(int fd);
int drm_shim_ioctl(int fd, unsigned long request, void *arg);
void *drm_shim_mmap(struct shim_fd *shim_fd, size_t length, int prot, int flags,
                    int fd, off64_t offset);

int drm_shim_bo_init(struct shim_bo *bo, size_t size);
void drm_shim_bo_get(struct shim_bo *bo);
void drm_shim_bo_put(struct shim_bo *bo);
struct shim_bo *drm_shim_bo_lookup(struct shim_fd *shim_fd, int handle);
int drm_shim_bo_get_handle(struct shim_fd *shim_fd, struct shim_bo *bo);
uint64_t drm_shim_bo_get_mmap_offset(struct shim_fd *shim_fd,
                                     struct shim_bo *bo);

/* driver-specific hooks. */
void drm_shim_driver_init(void);
extern bool drm_shim_driver_prefers_new_render_node;
