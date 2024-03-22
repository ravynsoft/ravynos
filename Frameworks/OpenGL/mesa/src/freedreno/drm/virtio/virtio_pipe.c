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

#include "util/libsync.h"
#include "util/slab.h"

#include "freedreno_ringbuffer_sp.h"
#include "virtio_priv.h"

static int
query_param(struct fd_pipe *pipe, uint32_t param, uint64_t *value)
{
   struct virtio_pipe *virtio_pipe = to_virtio_pipe(pipe);
   struct drm_msm_param req = {
      .pipe = virtio_pipe->pipe,
      .param = param,
   };
   int ret;

   ret = virtio_simple_ioctl(pipe->dev, DRM_IOCTL_MSM_GET_PARAM, &req);
   if (ret)
      return ret;

   *value = req.value;

   return 0;
}

static int
query_faults(struct fd_pipe *pipe, uint64_t *value)
{
   struct virtio_device *virtio_dev = to_virtio_device(pipe->dev);
   uint32_t async_error = 0;
   uint64_t global_faults;

   if (vdrm_shmem_has_field(virtio_dev->shmem, async_error))
      async_error = virtio_dev->shmem->async_error;

   if (vdrm_shmem_has_field(virtio_dev->shmem, global_faults)) {
      global_faults = virtio_dev->shmem->global_faults;
   } else {
      int ret = query_param(pipe, MSM_PARAM_FAULTS, &global_faults);
      if (ret)
         return ret;
   }

   *value = global_faults + async_error;

   return 0;
}

static int
virtio_pipe_get_param(struct fd_pipe *pipe, enum fd_param_id param,
                   uint64_t *value)
{
   struct virtio_pipe *virtio_pipe = to_virtio_pipe(pipe);
   struct virtio_device *virtio_dev = to_virtio_device(pipe->dev);

   switch (param) {
   case FD_DEVICE_ID: // XXX probably get rid of this..
   case FD_GPU_ID:
      *value = virtio_pipe->gpu_id;
      return 0;
   case FD_GMEM_SIZE:
      *value = virtio_pipe->gmem;
      return 0;
   case FD_GMEM_BASE:
      *value = virtio_pipe->gmem_base;
      return 0;
   case FD_CHIP_ID:
      *value = virtio_pipe->chip_id;
      return 0;
   case FD_MAX_FREQ:
      *value = virtio_dev->vdrm->caps.u.msm.max_freq;
      return 0;
   case FD_TIMESTAMP:
      return query_param(pipe, MSM_PARAM_TIMESTAMP, value);
   case FD_NR_PRIORITIES:
      *value = virtio_dev->vdrm->caps.u.msm.priorities;
      return 0;
   case FD_CTX_FAULTS:
   case FD_GLOBAL_FAULTS:
      return query_faults(pipe, value);
   case FD_SUSPEND_COUNT:
      return query_param(pipe, MSM_PARAM_SUSPENDS, value);
   case FD_VA_SIZE:
      *value = virtio_dev->vdrm->caps.u.msm.va_size;
      return 0;
   default:
      ERROR_MSG("invalid param id: %d", param);
      return -1;
   }
}

static int
virtio_pipe_wait(struct fd_pipe *pipe, const struct fd_fence *fence, uint64_t timeout)
{
   MESA_TRACE_FUNC();
   struct vdrm_device *vdrm = to_virtio_device(pipe->dev)->vdrm;
   struct msm_ccmd_wait_fence_req req = {
         .hdr = MSM_CCMD(WAIT_FENCE, sizeof(req)),
         .queue_id = to_virtio_pipe(pipe)->queue_id,
         .fence = fence->kfence,
   };
   struct msm_ccmd_submitqueue_query_rsp *rsp;
   int64_t end_time = os_time_get_nano() + timeout;
   int ret;

   /* Do a non-blocking wait to trigger host-side wait-boost,
    * if the host kernel is new enough
    */
   rsp = vdrm_alloc_rsp(vdrm, &req.hdr, sizeof(*rsp));
   ret = vdrm_send_req(vdrm, &req.hdr, false);
   if (ret)
      goto out;

   vdrm_flush(vdrm);

   if (fence->use_fence_fd)
      return sync_wait(fence->fence_fd, timeout / 1000000);

   do {
      rsp = vdrm_alloc_rsp(vdrm, &req.hdr, sizeof(*rsp));

      ret = vdrm_send_req(vdrm, &req.hdr, true);
      if (ret)
         goto out;

      if ((timeout != OS_TIMEOUT_INFINITE) &&
          (os_time_get_nano() >= end_time))
         break;

      ret = rsp->ret;
   } while (ret == -ETIMEDOUT);

out:
   return ret;
}

static int
open_submitqueue(struct fd_pipe *pipe, uint32_t prio)
{
   struct virtio_pipe *virtio_pipe = to_virtio_pipe(pipe);

   struct drm_msm_submitqueue req = {
      .flags = 0,
      .prio = prio,
   };
   uint64_t nr_prio = 1;
   int ret;

   virtio_pipe_get_param(pipe, FD_NR_PRIORITIES, &nr_prio);

   req.prio = MIN2(req.prio, MAX2(nr_prio, 1) - 1);

   ret = virtio_simple_ioctl(pipe->dev, DRM_IOCTL_MSM_SUBMITQUEUE_NEW, &req);
   if (ret) {
      ERROR_MSG("could not create submitqueue! %d (%s)", ret, strerror(errno));
      return ret;
   }

   virtio_pipe->queue_id = req.id;
   virtio_pipe->ring_idx = req.prio + 1;

   return 0;
}

static void
close_submitqueue(struct fd_pipe *pipe, uint32_t queue_id)
{
   virtio_simple_ioctl(pipe->dev, DRM_IOCTL_MSM_SUBMITQUEUE_CLOSE, &queue_id);
}

static void
virtio_pipe_destroy(struct fd_pipe *pipe)
{
   struct virtio_pipe *virtio_pipe = to_virtio_pipe(pipe);

   if (util_queue_is_initialized(&virtio_pipe->retire_queue))
      util_queue_destroy(&virtio_pipe->retire_queue);

   close_submitqueue(pipe, virtio_pipe->queue_id);
   fd_pipe_sp_ringpool_fini(pipe);
   free(virtio_pipe);
}

static const struct fd_pipe_funcs funcs = {
   .ringbuffer_new_object = fd_ringbuffer_sp_new_object,
   .submit_new = virtio_submit_new,
   .flush = fd_pipe_sp_flush,
   .get_param = virtio_pipe_get_param,
   .wait = virtio_pipe_wait,
   .destroy = virtio_pipe_destroy,
};

struct fd_pipe *
virtio_pipe_new(struct fd_device *dev, enum fd_pipe_id id, uint32_t prio)
{
   static const uint32_t pipe_id[] = {
      [FD_PIPE_3D] = MSM_PIPE_3D0,
      [FD_PIPE_2D] = MSM_PIPE_2D0,
   };
   struct virtio_device *virtio_dev = to_virtio_device(dev);
   struct vdrm_device *vdrm = virtio_dev->vdrm;
   struct virtio_pipe *virtio_pipe = NULL;
   struct fd_pipe *pipe = NULL;

   virtio_pipe = calloc(1, sizeof(*virtio_pipe));
   if (!virtio_pipe) {
      ERROR_MSG("allocation failed");
      goto fail;
   }

   pipe = &virtio_pipe->base;

   pipe->funcs = &funcs;

   /* initialize before get_param(): */
   pipe->dev = dev;
   virtio_pipe->pipe = pipe_id[id];

   virtio_pipe->gpu_id = vdrm->caps.u.msm.gpu_id;
   virtio_pipe->gmem = vdrm->caps.u.msm.gmem_size;
   virtio_pipe->gmem_base = vdrm->caps.u.msm.gmem_base;
   virtio_pipe->chip_id = vdrm->caps.u.msm.chip_id;


   if (!(virtio_pipe->gpu_id || virtio_pipe->chip_id))
      goto fail;

   util_queue_init(&virtio_pipe->retire_queue, "rq", 8, 1,
                   UTIL_QUEUE_INIT_RESIZE_IF_FULL, NULL);

   INFO_MSG("Pipe Info:");
   INFO_MSG(" GPU-id:          %d", virtio_pipe->gpu_id);
   INFO_MSG(" Chip-id:         0x%016"PRIx64, virtio_pipe->chip_id);
   INFO_MSG(" GMEM size:       0x%08x", virtio_pipe->gmem);

   if (open_submitqueue(pipe, prio))
      goto fail;

   fd_pipe_sp_ringpool_init(pipe);

   return pipe;
fail:
   if (pipe)
      fd_pipe_del(pipe);
   return NULL;
}
