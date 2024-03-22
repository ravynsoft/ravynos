/*
 * Copyright © 2022 Google, Inc.
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
 */

#ifndef VIRTIO_PRIV_H_
#define VIRTIO_PRIV_H_

#include <poll.h>

#include "freedreno_priv.h"

#include "util/perf/cpu_trace.h"
#include "util/u_atomic.h"
#include "util/slab.h"
#include "util/timespec.h"
#include "util/vma.h"

#include "drm-uapi/virtgpu_drm.h"
/* We also use some types/defines from the host drm/msm uabi: */
#include "drm-uapi/msm_drm.h"

#define VIRGL_RENDERER_UNSTABLE_APIS 1
#include "virglrenderer_hw.h"
#include "msm_proto.h"

#include "vdrm.h"

struct virtio_device {
   struct fd_device base;

   struct vdrm_device *vdrm;

   uint32_t next_blob_id;
   struct msm_shmem *shmem;

   /*
    * Notes on address space allocation:
    *
    * In both the import (GEM_INFO) and new (GEM_NEW) path we allocate
    * the iova.  Since the iova (vma on kernel side) is local to the
    * address space, and that is 1:1 with drm fd (which is 1:1 with
    * virtio_device and therefore address_space) which is not shared
    * with anything outside of the driver, and because of the handle
    * de-duplication, we can safely assume that an iova has not yet
    * been set on imported buffers.
    *
    * The other complication with userspace allocated iova is that
    * the kernel holds on to a reference to the bo (and the GPU is
    * still using it's iova) until the submit retires.  So a per-pipe
    * retire_queue is used to hold an extra reference to the submit
    * (and indirectly all the bo's referenced) until the out-fence is
    * signaled.
    */
   struct util_vma_heap address_space;
   simple_mtx_t address_space_lock;
};
FD_DEFINE_CAST(fd_device, virtio_device);

struct fd_device *virtio_device_new(int fd, drmVersionPtr version);

static inline void
virtio_dev_free_iova(struct fd_device *dev, uint64_t iova, uint32_t size)
{
   struct virtio_device *virtio_dev = to_virtio_device(dev);

   simple_mtx_lock(&virtio_dev->address_space_lock);
   util_vma_heap_free(&virtio_dev->address_space, iova, size);
   simple_mtx_unlock(&virtio_dev->address_space_lock);
}

static inline uint64_t
virtio_dev_alloc_iova(struct fd_device *dev, uint32_t size)
{
   struct virtio_device *virtio_dev = to_virtio_device(dev);
   uint64_t iova;

   simple_mtx_lock(&virtio_dev->address_space_lock);
   iova = util_vma_heap_alloc(&virtio_dev->address_space, size, 0x1000);
   simple_mtx_unlock(&virtio_dev->address_space_lock);

   return iova;
}

struct virtio_pipe {
   struct fd_pipe base;
   uint32_t pipe;
   uint32_t gpu_id;
   uint64_t chip_id;
   uint64_t gmem_base;
   uint32_t gmem;
   uint32_t queue_id;
   uint32_t ring_idx;
   struct slab_parent_pool ring_pool;

   /**
    * We know that the kernel allocated fence seqno's sequentially per-
    * submitqueue in a range 1..INT_MAX, which is incremented *after* any
    * point where the submit ioctl could be restarted.  So we just *guess*
    * what the next seqno fence will be to avoid having to synchronize the
    * submit with the host.
    *
    * TODO maybe we need version_minor bump so we can make the 1..INT_MAX
    * assumption.. it is only really true after:
    *
    *   ca3ffcbeb0c8 ("drm/msm/gpu: Don't allow zero fence_id")
    */
   int32_t next_submit_fence;

   /**
    * When userspace allocates iova, we need to defer deleting bo's (and
    * therefore releasing their address) until submits referencing them
    * have completed.  This is accomplished by enqueueing a job, holding
    * a reference to the submit, that waits on the submit's out-fence
    * before dropping the reference to the submit.  The submit holds a
    * reference to the associated ring buffers, which in turn hold a ref
    * to the associated bo's.
    */
   struct util_queue retire_queue;
};
FD_DEFINE_CAST(fd_pipe, virtio_pipe);

struct fd_pipe *virtio_pipe_new(struct fd_device *dev, enum fd_pipe_id id,
                                uint32_t prio);

struct fd_submit *virtio_submit_new(struct fd_pipe *pipe);

struct virtio_bo {
   struct fd_bo base;
   uint64_t alloc_time_ns;
   uint64_t offset;
   uint32_t res_id;
   uint32_t blob_id;
   uint32_t upload_seqno;
   bool has_upload_seqno;
};
FD_DEFINE_CAST(fd_bo, virtio_bo);

struct fd_bo *virtio_bo_new(struct fd_device *dev, uint32_t size, uint32_t flags);
struct fd_bo *virtio_bo_from_handle(struct fd_device *dev, uint32_t size,
                                    uint32_t handle);

/*
 * Internal helpers:
 */
int virtio_simple_ioctl(struct fd_device *dev, unsigned cmd, void *req);

#endif /* VIRTIO_PRIV_H_ */
