/*
 * Copyright (C) 2012-2018 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FREEDRENO_DRMIF_H_
#define FREEDRENO_DRMIF_H_

#include <stdint.h>

#include "util/bitset.h"
#include "util/list.h"
#include "util/u_debug.h"
#include "util/u_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fd_bo;
struct fd_pipe;
struct fd_device;

enum fd_pipe_id {
   FD_PIPE_3D = 1,
   FD_PIPE_2D = 2,
   /* some devices have two 2d blocks.. not really sure how to
    * use that yet, so just ignoring the 2nd 2d pipe for now
    */
   FD_PIPE_MAX
};

enum fd_param_id {
   FD_DEVICE_ID,
   FD_GMEM_SIZE,
   FD_GMEM_BASE,     /* 64b */
   FD_GPU_ID,
   FD_CHIP_ID,       /* 64b */
   FD_MAX_FREQ,
   FD_TIMESTAMP,
   FD_NR_PRIORITIES,      /* # of rings == # of distinct priority levels */
   FD_CTX_FAULTS,    /* # of per context faults */
   FD_GLOBAL_FAULTS, /* # of global (all context) faults */
   FD_SUSPEND_COUNT, /* # of times the GPU has suspended, and potentially lost state */
   FD_SYSPROF,       /* Settable (for CAP_SYS_ADMIN) param for system profiling */
   FD_VA_SIZE,       /* GPU virtual address size */
};

/**
 * Helper for fence/seqno comparisions which deals properly with rollover.
 * Returns true if fence 'a' is before fence 'b'
 */
static inline bool
fd_fence_before(uint32_t a, uint32_t b)
{
   return (int32_t)(a - b) < 0;
}

static inline bool
fd_fence_after(uint32_t a, uint32_t b)
{
   return (int32_t)(a - b) > 0;
}

/**
 * Encapsulates submit out-fence(s), which consist of a 'timestamp' (per-
 * pipe (submitqueue) sequence number) and optionally, if requested, an
 * out-fence-fd
 *
 * Per submit, there are actually two fences:
 *  1) The userspace maintained fence, which is used to optimistically
 *     avoid kernel ioctls to query if specific rendering is completed
 *  2) The kernel maintained fence, which we cannot directly do anything
 *     with, other than pass it back to the kernel
 *
 * The userspace fence is mostly internal to the drm layer, but we want
 * the gallium layer to be able to pass it back to us for things like
 * fd_pipe_wait().  So this struct encapsulates the two.
 */
struct fd_fence {
   /**
    * Note refcnt is *not* atomic, but protected by fence_lock, since the
    * fence_lock is held in fd_bo_add_fence(), which is the hotpath.
    */
   int32_t refcnt;

   struct fd_pipe *pipe;

   /**
    * The ready fence is signaled once the submit is actually flushed down
    * to the kernel, and fence/fence_fd are populated.  You must wait for
    * this fence to be signaled before reading fence/fence_fd.
    */
   struct util_queue_fence ready;

   uint32_t kfence;     /* kernel fence */
   uint32_t ufence;     /* userspace fence */

   /**
    * Optional dma_fence fd, returned by submit if use_fence_fd is true
    */
   int fence_fd;
   bool use_fence_fd;
};

struct fd_fence *fd_fence_new(struct fd_pipe *pipe, bool use_fence_fd);
struct fd_fence *fd_fence_ref(struct fd_fence *f);
struct fd_fence *fd_fence_ref_locked(struct fd_fence *f);
void fd_fence_del(struct fd_fence *f);
void fd_fence_del_locked(struct fd_fence *f);
void fd_fence_flush(struct fd_fence *f);
int fd_fence_wait(struct fd_fence *f);

/*
 * bo flags:
 */

#define FD_BO_CACHED_COHERENT     BITSET_BIT(0) /* Default caching is WRITECOMBINE */
#define FD_BO_GPUREADONLY         BITSET_BIT(1)
#define FD_BO_NOMAP               BITSET_BIT(2) /* Hint that the bo will not be mmap'd */

/* Hint that the bo will be exported/shared: */
#define FD_BO_SHARED              BITSET_BIT(4)
#define FD_BO_SCANOUT             BITSET_BIT(5)

/* internal bo flags: */
#define _FD_BO_NOSYNC             BITSET_BIT(7) /* Avoid userspace fencing on control buffers */

/*
 * bo access flags: (keep aligned to MSM_PREP_x)
 */
#define FD_BO_PREP_READ   BITSET_BIT(0)
#define FD_BO_PREP_WRITE  BITSET_BIT(1)
#define FD_BO_PREP_NOSYNC BITSET_BIT(2)
#define FD_BO_PREP_FLUSH  BITSET_BIT(3)


/* device functions:
 */

struct fd_device *fd_device_new(int fd);
struct fd_device *fd_device_new_dup(int fd);
struct fd_device *fd_device_open(void);
struct fd_device *fd_device_ref(struct fd_device *dev);
void fd_device_purge(struct fd_device *dev);
void fd_device_del(struct fd_device *dev);
int fd_device_fd(struct fd_device *dev);

enum fd_version {
   FD_VERSION_MADVISE = 1,             /* kernel supports madvise */
   FD_VERSION_UNLIMITED_CMDS = 1,      /* submits w/ >4 cmd buffers (growable ringbuffer) */
   FD_VERSION_FENCE_FD = 2,            /* submit command supports in/out fences */
   FD_VERSION_GMEM_BASE = 3,           /* supports querying GMEM base address */
   FD_VERSION_SUBMIT_QUEUES = 3,       /* submit queues and multiple priority levels */
   FD_VERSION_BO_IOVA = 3,             /* supports fd_bo_get/put_iova() */
   FD_VERSION_SOFTPIN = 4,             /* adds softpin, bo name, and dump flag */
   FD_VERSION_ROBUSTNESS = 5,          /* adds FD_NR_FAULTS and FD_PP_PGTABLE */
   FD_VERSION_MEMORY_FD = 2,           /* supports shared memory objects */
   FD_VERSION_SUSPENDS = 7,            /* Adds MSM_PARAM_SUSPENDS to detect device suspend */
   FD_VERSION_CACHED_COHERENT = 8,     /* Adds cached-coherent support (a6xx+) */
   FD_VERSION_VA_SIZE = 9,
};
enum fd_version fd_device_version(struct fd_device *dev);

bool fd_has_syncobj(struct fd_device *dev);

/* pipe functions:
 */

struct fd_pipe *fd_pipe_new(struct fd_device *dev, enum fd_pipe_id id);
struct fd_pipe *fd_pipe_new2(struct fd_device *dev, enum fd_pipe_id id,
                             uint32_t prio);
struct fd_pipe *fd_pipe_ref(struct fd_pipe *pipe);
struct fd_pipe *fd_pipe_ref_locked(struct fd_pipe *pipe);
void fd_pipe_del(struct fd_pipe *pipe);
void fd_pipe_purge(struct fd_pipe *pipe);
const struct fd_dev_id * fd_pipe_dev_id(struct fd_pipe *pipe);
int fd_pipe_get_param(struct fd_pipe *pipe, enum fd_param_id param,
                      uint64_t *value);
int fd_pipe_set_param(struct fd_pipe *pipe, enum fd_param_id param,
                      uint64_t value);
int fd_pipe_wait(struct fd_pipe *pipe, const struct fd_fence *fence);
/* timeout in nanosec */
int fd_pipe_wait_timeout(struct fd_pipe *pipe, const struct fd_fence *fence,
                         uint64_t timeout);

/* buffer-object functions:
 */

struct fd_bo {
   struct fd_device *dev;
   uint32_t size;
   uint32_t handle;
   uint32_t name;
   int32_t refcnt;
   uint32_t reloc_flags; /* flags like FD_RELOC_DUMP to use for relocs to this BO */
   uint32_t alloc_flags; /* flags that control allocation/mapping, ie. FD_BO_x */
   uint64_t iova;
   void *map;
   const struct fd_bo_funcs *funcs;

   enum {
      NO_CACHE = 0,
      BO_CACHE = 1,
      RING_CACHE = 2,
   } bo_reuse : 2;

   /* Most recent index in submit's bo table, used to optimize the common
    * case where a bo is used many times in the same submit.
    */
   uint32_t idx;

   struct list_head node; /* bucket-list entry */
   time_t free_time;      /* time when added to bucket-list */

   unsigned short nr_fences, max_fences;
   struct fd_fence **fences;

   /* In the common case, there is no more than one fence attached.
    * This provides storage for the fences table until it grows to
    * be larger than a single element.
    */
   struct fd_fence *_inline_fence;
};

struct fd_bo *_fd_bo_new(struct fd_device *dev, uint32_t size, uint32_t flags);
void _fd_bo_set_name(struct fd_bo *bo, const char *fmt, va_list ap);

static inline void fd_bo_set_name(struct fd_bo *bo, const char *fmt, ...)
   _util_printf_format(2, 3);

static inline void
fd_bo_set_name(struct fd_bo *bo, const char *fmt, ...)
{
#ifndef NDEBUG
   va_list ap;
   va_start(ap, fmt);
   _fd_bo_set_name(bo, fmt, ap);
   va_end(ap);
#endif
}

static inline struct fd_bo *fd_bo_new(struct fd_device *dev, uint32_t size,
                                      uint32_t flags, const char *fmt, ...)
   _util_printf_format(4, 5);

static inline struct fd_bo *
fd_bo_new(struct fd_device *dev, uint32_t size, uint32_t flags, const char *fmt,
          ...)
{
   struct fd_bo *bo = _fd_bo_new(dev, size, flags);
#ifndef NDEBUG
   if (fmt) {
      va_list ap;
      va_start(ap, fmt);
      _fd_bo_set_name(bo, fmt, ap);
      va_end(ap);
   }
#endif
   return bo;
}

struct fd_bo *fd_bo_from_handle(struct fd_device *dev, uint32_t handle,
                                uint32_t size);
struct fd_bo *fd_bo_from_name(struct fd_device *dev, uint32_t name);
struct fd_bo *fd_bo_from_dmabuf(struct fd_device *dev, int fd);
void fd_bo_mark_for_dump(struct fd_bo *bo);

static inline uint64_t
fd_bo_get_iova(struct fd_bo *bo)
{
   /* ancient kernels did not support this */
   assert(bo->iova != 0);
   return bo->iova;
}

struct fd_bo *fd_bo_ref(struct fd_bo *bo);
void fd_bo_del(struct fd_bo *bo);
void fd_bo_del_array(struct fd_bo **bos, int count);
void fd_bo_del_list_nocache(struct list_head *list);
int fd_bo_get_name(struct fd_bo *bo, uint32_t *name);
uint32_t fd_bo_handle(struct fd_bo *bo);
int fd_bo_dmabuf_drm(struct fd_bo *bo);
int fd_bo_dmabuf(struct fd_bo *bo);
uint32_t fd_bo_size(struct fd_bo *bo);
void *fd_bo_map(struct fd_bo *bo);
void fd_bo_upload(struct fd_bo *bo, void *src, unsigned off, unsigned len);
bool fd_bo_prefer_upload(struct fd_bo *bo, unsigned len);
int fd_bo_cpu_prep(struct fd_bo *bo, struct fd_pipe *pipe, uint32_t op);
bool fd_bo_is_cached(struct fd_bo *bo);
void fd_bo_set_metadata(struct fd_bo *bo, void *metadata, uint32_t metadata_size);
int fd_bo_get_metadata(struct fd_bo *bo, void *metadata, uint32_t metadata_size);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* FREEDRENO_DRMIF_H_ */
