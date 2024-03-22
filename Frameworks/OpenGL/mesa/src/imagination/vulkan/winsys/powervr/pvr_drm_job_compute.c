/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <xf86drm.h>

#include "drm-uapi/pvr_drm.h"
#include "pvr_drm.h"
#include "pvr_drm_job_common.h"
#include "pvr_drm_job_compute.h"
#include "pvr_private.h"
#include "pvr_winsys.h"
#include "pvr_winsys_helper.h"
#include "util/macros.h"
#include "vk_alloc.h"
#include "vk_drm_syncobj.h"
#include "vk_log.h"

struct pvr_drm_winsys_compute_ctx {
   struct pvr_winsys_compute_ctx base;

   /* Handle to kernel context. */
   uint32_t handle;
};

#define to_pvr_drm_winsys_compute_ctx(ctx) \
   container_of(ctx, struct pvr_drm_winsys_compute_ctx, base)

static void pvr_drm_compute_ctx_static_state_init(
   const struct pvr_winsys_compute_ctx_create_info *create_info,
   uint8_t *stream_ptr_start,
   uint32_t *stream_len_ptr)
{
   const struct pvr_winsys_compute_ctx_static_state *ws_static_state =
      &create_info->static_state;
   uint64_t *stream_ptr = (uint64_t *)stream_ptr_start;

   /* Leave space for stream header. */
   stream_ptr += pvr_cmd_length(KMD_STREAM_HDR) / 2;

   *stream_ptr++ = ws_static_state->cdm_ctx_store_pds0;
   *stream_ptr++ = ws_static_state->cdm_ctx_store_pds1;
   *stream_ptr++ = ws_static_state->cdm_ctx_terminate_pds;
   *stream_ptr++ = ws_static_state->cdm_ctx_terminate_pds1;
   *stream_ptr++ = ws_static_state->cdm_ctx_resume_pds0;
   *stream_ptr++ = ws_static_state->cdm_ctx_store_pds0_b;
   *stream_ptr++ = ws_static_state->cdm_ctx_resume_pds0_b;

   *stream_len_ptr = ((uint8_t *)stream_ptr - stream_ptr_start);

   pvr_csb_pack ((uint64_t *)stream_ptr_start, KMD_STREAM_HDR, value) {
      value.length = *stream_len_ptr;
   }
}

VkResult pvr_drm_winsys_compute_ctx_create(
   struct pvr_winsys *ws,
   const struct pvr_winsys_compute_ctx_create_info *create_info,
   struct pvr_winsys_compute_ctx **const ctx_out)
{
   uint8_t static_ctx_state_fw_stream[64];
   struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ws);
   struct drm_pvr_ioctl_create_context_args ctx_args = {
      .type = DRM_PVR_CTX_TYPE_COMPUTE,
      .priority = pvr_drm_from_winsys_priority(create_info->priority),
      .static_context_state = (__u64)static_ctx_state_fw_stream,
      .static_context_state_len = (__u32)sizeof(static_ctx_state_fw_stream),
      .vm_context_handle = drm_ws->vm_context,
   };

   struct pvr_drm_winsys_compute_ctx *drm_ctx;
   VkResult result;

   drm_ctx = vk_alloc(ws->alloc,
                      sizeof(*drm_ctx),
                      8,
                      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!drm_ctx) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_out;
   }

   pvr_drm_compute_ctx_static_state_init(create_info,
                                         static_ctx_state_fw_stream,
                                         &ctx_args.static_context_state_len);

   result = pvr_ioctlf(ws->render_fd,
                       DRM_IOCTL_PVR_CREATE_CONTEXT,
                       &ctx_args,
                       VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to create compute context");
   if (result != VK_SUCCESS)
      goto err_free_ctx;

   drm_ctx->base.ws = ws;
   drm_ctx->handle = ctx_args.handle;

   *ctx_out = &drm_ctx->base;

   return VK_SUCCESS;

err_free_ctx:
   vk_free(ws->alloc, drm_ctx);

err_out:
   return result;
}

void pvr_drm_winsys_compute_ctx_destroy(struct pvr_winsys_compute_ctx *ctx)
{
   struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ctx->ws);
   struct pvr_drm_winsys_compute_ctx *drm_ctx =
      to_pvr_drm_winsys_compute_ctx(ctx);
   struct drm_pvr_ioctl_destroy_context_args args = {
      .handle = drm_ctx->handle,
   };

   pvr_ioctlf(drm_ws->base.render_fd,
              DRM_IOCTL_PVR_DESTROY_CONTEXT,
              &args,
              VK_ERROR_UNKNOWN,
              "Error destroying compute context");

   vk_free(drm_ws->base.alloc, drm_ctx);
}

static uint32_t pvr_winsys_compute_flags_to_drm(
   const struct pvr_winsys_compute_submit_flags *const ws_flags)
{
   uint32_t flags = 0U;

   if (ws_flags->prevent_all_overlap)
      flags |= DRM_PVR_SUBMIT_JOB_COMPUTE_CMD_PREVENT_ALL_OVERLAP;

   if (ws_flags->use_single_core)
      flags |= DRM_PVR_SUBMIT_JOB_COMPUTE_CMD_SINGLE_CORE;

   return flags;
}

VkResult pvr_drm_winsys_compute_submit(
   const struct pvr_winsys_compute_ctx *ctx,
   const struct pvr_winsys_compute_submit_info *submit_info,
   UNUSED const struct pvr_device_info *dev_info,
   struct vk_sync *signal_sync)
{
   const struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ctx->ws);
   const struct pvr_drm_winsys_compute_ctx *drm_ctx =
      to_pvr_drm_winsys_compute_ctx(ctx);

   struct drm_pvr_sync_op sync_ops[2];
   struct drm_pvr_job job_args = {
      .type = DRM_PVR_JOB_TYPE_COMPUTE,
      .context_handle = drm_ctx->handle,
      .cmd_stream = (__u64)&submit_info->fw_stream[0],
      .cmd_stream_len = submit_info->fw_stream_len,
      /* bo_handles is unused and zeroed. */
      /* num_bo_handles is unused and zeroed. */
      .flags = pvr_winsys_compute_flags_to_drm(&submit_info->flags),
      .sync_ops = DRM_PVR_OBJ_ARRAY(0, sync_ops),
   };

   struct drm_pvr_ioctl_submit_jobs_args args = {
      .jobs = DRM_PVR_OBJ_ARRAY(1, &job_args),
   };

   if (submit_info->wait) {
      struct vk_sync *sync = submit_info->wait;

      assert(!(sync->flags & VK_SYNC_IS_TIMELINE));
      sync_ops[job_args.sync_ops.count++] = (struct drm_pvr_sync_op){
         .handle = vk_sync_as_drm_syncobj(sync)->syncobj,
         .flags = DRM_PVR_SYNC_OP_FLAG_WAIT |
                  DRM_PVR_SYNC_OP_FLAG_HANDLE_TYPE_SYNCOBJ,
         .value = 0,
      };
   }

   if (signal_sync) {
      assert(!(signal_sync->flags & VK_SYNC_IS_TIMELINE));
      sync_ops[job_args.sync_ops.count++] = (struct drm_pvr_sync_op){
         .handle = vk_sync_as_drm_syncobj(signal_sync)->syncobj,
         .flags = DRM_PVR_SYNC_OP_FLAG_SIGNAL |
                  DRM_PVR_SYNC_OP_FLAG_HANDLE_TYPE_SYNCOBJ,
         .value = 0,
      };
   }

   /* Returns VK_ERROR_OUT_OF_DEVICE_MEMORY to match pvrsrv. */
   return pvr_ioctlf(drm_ws->base.render_fd,
                     DRM_IOCTL_PVR_SUBMIT_JOBS,
                     &args,
                     VK_ERROR_OUT_OF_DEVICE_MEMORY,
                     "Failed to submit compute job");
}
