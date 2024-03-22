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

#ifndef CROCUS_BUFMGR_H
#define CROCUS_BUFMGR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "util/macros.h"
#include "util/u_atomic.h"
#include "util/list.h"
#include "pipe/p_defines.h"

struct crocus_batch;
struct intel_device_info;
struct util_debug_callback;

#define CROCUS_BINDER_SIZE (64 * 1024)
#define CROCUS_MAX_BINDERS 100

struct crocus_bo {
   /**
    * Size in bytes of the buffer object.
    *
    * The size may be larger than the size originally requested for the
    * allocation, such as being aligned to page size.
    */
   uint64_t size;

   /** Buffer manager context associated with this buffer object */
   struct crocus_bufmgr *bufmgr;

   /** The GEM handle for this buffer object. */
   uint32_t gem_handle;

   /**
    * Virtual address of the buffer inside the PPGTT (Per-Process Graphics
    * Translation Table).
    *
    * Although each hardware context has its own VMA, we assign BO's to the
    * same address in all contexts, for simplicity.
    */
   uint64_t gtt_offset;

   /**
    * The validation list index for this buffer, or -1 when not in a batch.
    * Note that a single buffer may be in multiple batches (contexts), and
    * this is a global field, which refers to the last batch using the BO.
    * It should not be considered authoritative, but can be used to avoid a
    * linear walk of the validation list in the common case by guessing that
    * exec_bos[bo->index] == bo and confirming whether that's the case.
    *
    * XXX: this is not ideal now that we have more than one batch per context,
    * XXX: as the index will flop back and forth between the render index and
    * XXX: compute index...
    */
   unsigned index;

   /**
    * Boolean of whether the GPU is definitely not accessing the buffer.
    *
    * This is only valid when reusable, since non-reusable
    * buffers are those that have been shared with other
    * processes, so we don't know their state.
    */
   bool idle;

   int refcount;
   const char *name;

   uint64_t kflags;

   /**
    * Kenel-assigned global name for this object
    *
    * List contains both flink named and prime fd'd objects
    */
   unsigned global_name;

   /**
    * Current tiling mode
    */
   uint32_t tiling_mode;
   uint32_t swizzle_mode;
   uint32_t stride;

   time_t free_time;

   /** Mapped address for the buffer, saved across map/unmap cycles */
   void *map_cpu;
   /** GTT virtual address for the buffer, saved across map/unmap cycles */
   void *map_gtt;
   /** WC CPU address for the buffer, saved across map/unmap cycles */
   void *map_wc;

   /** BO cache list */
   struct list_head head;

   /** List of GEM handle exports of this buffer (bo_export) */
   struct list_head exports;

   /**
    * Boolean of whether this buffer can be re-used
    */
   bool reusable;

   /**
    * Boolean of whether this buffer has been shared with an external client.
    */
   bool external;

   /**
    * Boolean of whether this buffer is cache coherent
    */
   bool cache_coherent;

   /**
    * Boolean of whether this buffer points into user memory
    */
   bool userptr;

   /**
    * Boolean of if this is used for scanout.
    */
   bool scanout;

   /** Pre-computed hash using _mesa_hash_pointer for cache tracking sets */
   uint32_t hash;
};

#define BO_ALLOC_ZEROED   (1 << 0)
#define BO_ALLOC_COHERENT (1 << 1)
#define BO_ALLOC_SCANOUT  (1 << 2)

/**
 * Allocate a buffer object.
 *
 * Buffer objects are not necessarily initially mapped into CPU virtual
 * address space or graphics device aperture.  They must be mapped
 * using crocus_bo_map() to be used by the CPU.
 */
struct crocus_bo *crocus_bo_alloc(struct crocus_bufmgr *bufmgr,
                                  const char *name, uint64_t size);

/**
 * Allocate a tiled buffer object.
 *
 * Alignment for tiled objects is set automatically; the 'flags'
 * argument provides a hint about how the object will be used initially.
 *
 * Valid tiling formats are:
 *  I915_TILING_NONE
 *  I915_TILING_X
 *  I915_TILING_Y
 */
struct crocus_bo *crocus_bo_alloc_tiled(struct crocus_bufmgr *bufmgr,
                                        const char *name, uint64_t size,
                                        uint32_t alignment,
                                        uint32_t tiling_mode, uint32_t pitch,
                                        unsigned flags);

struct crocus_bo *crocus_bo_create_userptr(struct crocus_bufmgr *bufmgr,
                                           const char *name, void *ptr,
                                           size_t size);

/** Takes a reference on a buffer object */
static inline void
crocus_bo_reference(struct crocus_bo *bo)
{
   p_atomic_inc(&bo->refcount);
}

static inline int
atomic_add_unless(int *v, int add, int unless)
{
   int c, old;
   c = p_atomic_read(v);
   while (c != unless && (old = p_atomic_cmpxchg(v, c, c + add)) != c)
      c = old;
   return c == unless;
}

void __crocus_bo_unreference(struct crocus_bo *bo);

/**
 * Releases a reference on a buffer object, freeing the data if
 * no references remain.
 */
static inline void crocus_bo_unreference(struct crocus_bo *bo)
{
   if (bo == NULL)
      return;

   assert(p_atomic_read(&bo->refcount) > 0);

   if (atomic_add_unless(&bo->refcount, -1, 1)) {
      __crocus_bo_unreference(bo);
   }
}

#define MAP_READ          PIPE_MAP_READ
#define MAP_WRITE         PIPE_MAP_WRITE
#define MAP_ASYNC         PIPE_MAP_UNSYNCHRONIZED
#define MAP_PERSISTENT    PIPE_MAP_PERSISTENT
#define MAP_COHERENT      PIPE_MAP_COHERENT
/* internal */
#define MAP_INTERNAL_MASK (0xff << 24)
#define MAP_RAW           (0x01 << 24)

#define MAP_FLAGS         (MAP_READ | MAP_WRITE | MAP_ASYNC | \
                           MAP_PERSISTENT | MAP_COHERENT | MAP_INTERNAL_MASK)

/**
 * Maps the buffer into userspace.
 *
 * This function will block waiting for any existing execution on the
 * buffer to complete, first.  The resulting mapping is returned.
 */
MUST_CHECK void *crocus_bo_map(struct util_debug_callback *dbg,
                             struct crocus_bo *bo, unsigned flags);

/**
 * Reduces the refcount on the userspace mapping of the buffer
 * object.
 */
static inline int crocus_bo_unmap(struct crocus_bo *bo) { return 0; }

/**
 * Waits for rendering to an object by the GPU to have completed.
 *
 * This is not required for any access to the BO by bo_map,
 * bo_subdata, etc.  It is merely a way for the driver to implement
 * glFinish.
 */
void crocus_bo_wait_rendering(struct crocus_bo *bo);

/**
 * Unref a buffer manager instance.
 */
void crocus_bufmgr_unref(struct crocus_bufmgr *bufmgr);

/**
 * Get the current tiling (and resulting swizzling) mode for the bo.
 *
 * \param buf Buffer to get tiling mode for
 * \param tiling_mode returned tiling mode
 * \param swizzle_mode returned swizzling mode
 */
int crocus_bo_get_tiling(struct crocus_bo *bo, uint32_t *tiling_mode,
                         uint32_t *swizzle_mode);

/**
 * Create a visible name for a buffer which can be used by other apps
 *
 * \param buf Buffer to create a name for
 * \param name Returned name
 */
int crocus_bo_flink(struct crocus_bo *bo, uint32_t *name);

/**
 * Is this buffer shared with external clients (exported)?
 */
static inline bool
crocus_bo_is_external(const struct crocus_bo *bo)
{
   return bo->external;
}

/**
 * Returns 1 if mapping the buffer for write could cause the process
 * to block, due to the object being active in the GPU.
 */
int crocus_bo_busy(struct crocus_bo *bo);

/**
 * Specify the volatility of the buffer.
 * \param bo Buffer to create a name for
 * \param madv The purgeable status
 *
 * Use I915_MADV_DONTNEED to mark the buffer as purgeable, and it will be
 * reclaimed under memory pressure. If you subsequently require the buffer,
 * then you must pass I915_MADV_WILLNEED to mark the buffer as required.
 *
 * Returns 1 if the buffer was retained, or 0 if it was discarded whilst
 * marked as I915_MADV_DONTNEED.
 */
int crocus_bo_madvise(struct crocus_bo *bo, int madv);

struct crocus_bufmgr *
crocus_bufmgr_get_for_fd(struct intel_device_info *devinfo, int fd,
                         bool bo_reuse);
int crocus_bufmgr_get_fd(struct crocus_bufmgr *bufmgr);

struct crocus_bo *crocus_bo_gem_create_from_name(struct crocus_bufmgr *bufmgr,
                                                 const char *name,
                                                 unsigned handle);

int crocus_bo_wait(struct crocus_bo *bo, int64_t timeout_ns);

uint32_t crocus_create_hw_context(struct crocus_bufmgr *bufmgr);
uint32_t crocus_clone_hw_context(struct crocus_bufmgr *bufmgr, uint32_t ctx_id);

#define CROCUS_CONTEXT_LOW_PRIORITY    ((I915_CONTEXT_MIN_USER_PRIORITY - 1) / 2)
#define CROCUS_CONTEXT_MEDIUM_PRIORITY (I915_CONTEXT_DEFAULT_PRIORITY)
#define CROCUS_CONTEXT_HIGH_PRIORITY   ((I915_CONTEXT_MAX_USER_PRIORITY + 1) / 2)

int crocus_hw_context_set_priority(struct crocus_bufmgr *bufmgr,
                                   uint32_t ctx_id, int priority);

void crocus_destroy_hw_context(struct crocus_bufmgr *bufmgr, uint32_t ctx_id);

int crocus_bo_export_dmabuf(struct crocus_bo *bo, int *prime_fd);
struct crocus_bo *crocus_bo_import_dmabuf(struct crocus_bufmgr *bufmgr,
                                          int prime_fd, uint64_t modifier);
struct crocus_bo *crocus_bo_import_dmabuf_no_mods(struct crocus_bufmgr *bufmgr,
                                                  int prime_fd);

/**
 * Exports a bo as a GEM handle into a given DRM file descriptor
 * \param bo Buffer to export
 * \param drm_fd File descriptor where the new handle is created
 * \param out_handle Pointer to store the new handle
 *
 * Returns 0 if the buffer was successfully exported, a non zero error code
 * otherwise.
 */
int crocus_bo_export_gem_handle_for_device(struct crocus_bo *bo, int drm_fd,
                                           uint32_t *out_handle);

uint32_t crocus_bo_export_gem_handle(struct crocus_bo *bo);

int drm_ioctl(int fd, unsigned long request, void *arg);

#endif /* CROCUS_BUFMGR_H */
