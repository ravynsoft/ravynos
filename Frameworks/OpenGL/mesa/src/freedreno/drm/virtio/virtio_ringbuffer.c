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

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>

#include "util/libsync.h"
#include "util/os_file.h"

#include "drm/freedreno_ringbuffer_sp.h"
#include "virtio_priv.h"

static void
retire_execute(void *job, void *gdata, int thread_index)
{
   struct fd_submit_sp *fd_submit = job;

   MESA_TRACE_FUNC();

   fd_fence_wait(fd_submit->out_fence);
}

static void
retire_cleanup(void *job, void *gdata, int thread_index)
{
   struct fd_submit_sp *fd_submit = job;
   fd_submit_del(&fd_submit->base);
}

static int
flush_submit_list(struct list_head *submit_list)
{
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(last_submit(submit_list));
   struct virtio_pipe *virtio_pipe = to_virtio_pipe(fd_submit->base.pipe);
   struct fd_pipe *pipe = &virtio_pipe->base;
   struct fd_device *dev = pipe->dev;

   unsigned nr_cmds = 0;

   MESA_TRACE_FUNC();

   /* Determine the number of extra cmds's from deferred submits that
    * we will be merging in:
    */
   foreach_submit (submit, submit_list) {
      assert(submit->pipe == &virtio_pipe->base);
      nr_cmds += to_fd_ringbuffer_sp(submit->primary)->u.nr_cmds;
   }

   /* TODO we can get rid of the extra copy into the req by just
    * assuming the max amount that nr->bos will grow is by the
    * nr_cmds, and just over-allocate a bit.
    */

   struct drm_msm_gem_submit_cmd cmds[nr_cmds];

   unsigned cmd_idx = 0;

   /* Build up the table of cmds, and for all but the last submit in the
    * list, merge their bo tables into the last submit.
    */
   foreach_submit_safe (submit, submit_list) {
      struct fd_ringbuffer_sp *deferred_primary =
         to_fd_ringbuffer_sp(submit->primary);

      for (unsigned i = 0; i < deferred_primary->u.nr_cmds; i++) {
         struct fd_bo *ring_bo = deferred_primary->u.cmds[i].ring_bo;
         cmds[cmd_idx].type = MSM_SUBMIT_CMD_BUF;
         cmds[cmd_idx].submit_idx = fd_submit_append_bo(fd_submit, ring_bo);
         cmds[cmd_idx].submit_offset = submit_offset(ring_bo, deferred_primary->offset);
         cmds[cmd_idx].size = deferred_primary->u.cmds[i].size;
         cmds[cmd_idx].pad = 0;
         cmds[cmd_idx].nr_relocs = 0;

         cmd_idx++;
      }

      /* We are merging all the submits in the list into the last submit,
       * so the remainder of the loop body doesn't apply to the last submit
       */
      if (submit == last_submit(submit_list)) {
         DEBUG_MSG("merged %u submits", cmd_idx);
         break;
      }

      struct fd_submit_sp *fd_deferred_submit = to_fd_submit_sp(submit);
      for (unsigned i = 0; i < fd_deferred_submit->nr_bos; i++) {
         /* Note: if bo is used in both the current submit and the deferred
          * submit being merged, we expect to hit the fast-path as we add it
          * to the current submit:
          */
         fd_submit_append_bo(fd_submit, fd_deferred_submit->bos[i]);
      }

      /* Now that the cmds/bos have been transfered over to the current submit,
       * we can remove the deferred submit from the list and drop it's reference
       */
      list_del(&submit->node);
      fd_submit_del(submit);
   }

   /* Needs to be after get_cmd() as that could create bos/cmds table:
    *
    * NOTE allocate on-stack in the common case, but with an upper-
    * bound to limit on-stack allocation to 4k:
    */
   const unsigned bo_limit = 4096 / sizeof(struct drm_msm_gem_submit_bo);
   bool bos_on_stack = fd_submit->nr_bos < bo_limit;
   struct drm_msm_gem_submit_bo
      _submit_bos[bos_on_stack ? fd_submit->nr_bos : 0];
   struct drm_msm_gem_submit_bo *submit_bos;
   uint32_t _guest_handles[bos_on_stack ? fd_submit->nr_bos : 0];
   uint32_t *guest_handles;
   if (bos_on_stack) {
      submit_bos = _submit_bos;
      guest_handles = _guest_handles;
   } else {
      submit_bos = malloc(fd_submit->nr_bos * sizeof(submit_bos[0]));
      guest_handles = malloc(fd_submit->nr_bos * sizeof(guest_handles[0]));
   }

   uint32_t nr_guest_handles = 0;
   for (unsigned i = 0; i < fd_submit->nr_bos; i++) {
      struct virtio_bo *virtio_bo = to_virtio_bo(fd_submit->bos[i]);

      if (virtio_bo->base.alloc_flags & FD_BO_SHARED)
         guest_handles[nr_guest_handles++] = virtio_bo->base.handle;

      submit_bos[i].flags = fd_submit->bos[i]->reloc_flags;
      submit_bos[i].handle = virtio_bo->res_id;
      submit_bos[i].presumed = 0;
   }

   if (virtio_pipe->next_submit_fence <= 0)
      virtio_pipe->next_submit_fence = 1;

   uint32_t kfence = virtio_pipe->next_submit_fence++;

   /* TODO avoid extra memcpy, and populate bo's and cmds directly
    * into the req msg
    */
   unsigned bos_len = fd_submit->nr_bos * sizeof(struct drm_msm_gem_submit_bo);
   unsigned cmd_len = nr_cmds * sizeof(struct drm_msm_gem_submit_cmd);
   unsigned req_len = sizeof(struct msm_ccmd_gem_submit_req) + bos_len + cmd_len;
   struct msm_ccmd_gem_submit_req *req = malloc(req_len);

   req->hdr      = MSM_CCMD(GEM_SUBMIT, req_len);
   req->flags    = virtio_pipe->pipe;
   req->queue_id = virtio_pipe->queue_id;
   req->nr_bos   = fd_submit->nr_bos;
   req->nr_cmds  = nr_cmds;
   req->fence    = kfence;

   memcpy(req->payload, submit_bos, bos_len);
   memcpy(req->payload + bos_len, cmds, cmd_len);

   struct fd_fence *out_fence = fd_submit->out_fence;

   out_fence->kfence = kfence;

   /* Even if gallium driver hasn't requested a fence-fd, request one.
    * This way, if we have to block waiting for the fence, we can do
    * it in the guest, rather than in the single-threaded host.
    */
   out_fence->use_fence_fd = true;

   if (pipe->no_implicit_sync) {
      req->flags |= MSM_SUBMIT_NO_IMPLICIT;
      nr_guest_handles = 0;
   }

   struct vdrm_execbuf_params p = {
      .req = &req->hdr,
      .handles = guest_handles,
      .num_handles = nr_guest_handles,
      .has_in_fence_fd = !!(fd_submit->in_fence_fd != -1),
      .needs_out_fence_fd = true,
      .fence_fd = fd_submit->in_fence_fd,
      .ring_idx = virtio_pipe->ring_idx,
   };
   vdrm_execbuf(to_virtio_device(dev)->vdrm, &p);

   out_fence->fence_fd = p.fence_fd;

   free(req);

   if (!bos_on_stack) {
      free(submit_bos);
      free(guest_handles);
   }

   if (fd_submit->in_fence_fd != -1)
      close(fd_submit->in_fence_fd);

   fd_submit_ref(&fd_submit->base);

   util_queue_fence_init(&fd_submit->retire_fence);

   util_queue_add_job(&virtio_pipe->retire_queue,
                      fd_submit, &fd_submit->retire_fence,
                      retire_execute,
                      retire_cleanup,
                      0);

   return 0;
}

struct fd_submit *
virtio_submit_new(struct fd_pipe *pipe)
{
   /* We don't do any translation from internal FD_RELOC flags to MSM flags. */
   STATIC_ASSERT(FD_RELOC_READ == MSM_SUBMIT_BO_READ);
   STATIC_ASSERT(FD_RELOC_WRITE == MSM_SUBMIT_BO_WRITE);
   STATIC_ASSERT(FD_RELOC_DUMP == MSM_SUBMIT_BO_DUMP);

   return fd_submit_sp_new(pipe, flush_submit_list);
}
