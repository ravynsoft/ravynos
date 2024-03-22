/*
 * Copyright (C) 2018 Rob Clark <robclark@freedesktop.org>
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

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>

#include "util/os_file.h"

#include "drm/freedreno_ringbuffer_sp.h"
#include "msm_priv.h"

static int
flush_submit_list(struct list_head *submit_list)
{
   struct fd_submit_sp *fd_submit = to_fd_submit_sp(last_submit(submit_list));
   struct fd_pipe *pipe = fd_submit->base.pipe;
   struct msm_pipe *msm_pipe = to_msm_pipe(pipe);
   struct drm_msm_gem_submit req = {
      .flags = msm_pipe->pipe,
      .queueid = msm_pipe->queue_id,
   };
   int ret;

   unsigned nr_cmds = 0;

   MESA_TRACE_FUNC();

   /* Determine the number of extra cmds's from deferred submits that
    * we will be merging in:
    */
   foreach_submit (submit, submit_list) {
      assert(submit->pipe == &msm_pipe->base);
      nr_cmds += to_fd_ringbuffer_sp(submit->primary)->u.nr_cmds;
   }

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

   if (fd_submit->in_fence_fd != -1) {
      req.flags |= MSM_SUBMIT_FENCE_FD_IN;
      req.fence_fd = fd_submit->in_fence_fd;
   }

   if (pipe->no_implicit_sync) {
      req.flags |= MSM_SUBMIT_NO_IMPLICIT;
   }

   if (fd_submit->out_fence->use_fence_fd) {
      req.flags |= MSM_SUBMIT_FENCE_FD_OUT;
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
   if (bos_on_stack) {
      submit_bos = _submit_bos;
   } else {
      submit_bos = malloc(fd_submit->nr_bos * sizeof(submit_bos[0]));
   }

   for (unsigned i = 0; i < fd_submit->nr_bos; i++) {
      submit_bos[i].flags = fd_submit->bos[i]->reloc_flags;
      submit_bos[i].handle = fd_submit->bos[i]->handle;
      submit_bos[i].presumed = 0;
   }

   req.bos = VOID2U64(submit_bos);
   req.nr_bos = fd_submit->nr_bos;
   req.cmds = VOID2U64(cmds);
   req.nr_cmds = nr_cmds;

   DEBUG_MSG("nr_cmds=%u, nr_bos=%u", req.nr_cmds, req.nr_bos);

   ret = drmCommandWriteRead(msm_pipe->base.dev->fd, DRM_MSM_GEM_SUBMIT, &req,
                             sizeof(req));
   if (ret) {
      ERROR_MSG("submit failed: %d (%s)", ret, strerror(errno));
      msm_dump_submit(&req);
   } else if (!ret) {
      fd_submit->out_fence->kfence = req.fence;
      fd_submit->out_fence->fence_fd = req.fence_fd;
   }

   if (!bos_on_stack)
      free(submit_bos);

   if (fd_submit->in_fence_fd != -1)
      close(fd_submit->in_fence_fd);

   return ret;
}

struct fd_submit *
msm_submit_sp_new(struct fd_pipe *pipe)
{
   /* We don't do any translation from internal FD_RELOC flags to MSM flags. */
   STATIC_ASSERT(FD_RELOC_READ == MSM_SUBMIT_BO_READ);
   STATIC_ASSERT(FD_RELOC_WRITE == MSM_SUBMIT_BO_WRITE);
   STATIC_ASSERT(FD_RELOC_DUMP == MSM_SUBMIT_BO_DUMP);

   return fd_submit_sp_new(pipe, flush_submit_list);
}
