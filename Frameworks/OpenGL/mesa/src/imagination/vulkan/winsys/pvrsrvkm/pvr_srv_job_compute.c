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
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#include "fw-api/pvr_rogue_fwif.h"
#include "fw-api/pvr_rogue_fwif_rf.h"
#include "pvr_device_info.h"
#include "pvr_private.h"
#include "pvr_srv.h"
#include "pvr_srv_bridge.h"
#include "pvr_srv_job_common.h"
#include "pvr_srv_job_compute.h"
#include "pvr_srv_sync.h"
#include "pvr_winsys.h"
#include "util/macros.h"
#include "vk_alloc.h"
#include "vk_log.h"

struct pvr_srv_winsys_compute_ctx {
   struct pvr_winsys_compute_ctx base;

   void *handle;

   int timeline;
};

#define to_pvr_srv_winsys_compute_ctx(ctx) \
   container_of(ctx, struct pvr_srv_winsys_compute_ctx, base)

VkResult pvr_srv_winsys_compute_ctx_create(
   struct pvr_winsys *ws,
   const struct pvr_winsys_compute_ctx_create_info *create_info,
   struct pvr_winsys_compute_ctx **const ctx_out)
{
   struct rogue_fwif_static_computecontext_state static_state = {
		.ctx_switch_regs = {
			.cdm_context_pds0 = create_info->static_state.cdm_ctx_store_pds0,
			.cdm_context_pds0_b =
				create_info->static_state.cdm_ctx_store_pds0_b,
			.cdm_context_pds1 = create_info->static_state.cdm_ctx_store_pds1,

			.cdm_terminate_pds = create_info->static_state.cdm_ctx_terminate_pds,
			.cdm_terminate_pds1 =
				create_info->static_state.cdm_ctx_terminate_pds1,

			.cdm_resume_pds0 = create_info->static_state.cdm_ctx_resume_pds0,
			.cdm_resume_pds0_b = create_info->static_state.cdm_ctx_resume_pds0_b,
		},
	};

   struct rogue_fwif_rf_cmd reset_cmd = { 0 };

   struct pvr_srv_winsys *srv_ws = to_pvr_srv_winsys(ws);
   struct pvr_srv_winsys_compute_ctx *srv_ctx;
   VkResult result;

   srv_ctx = vk_alloc(ws->alloc,
                      sizeof(*srv_ctx),
                      8U,
                      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!srv_ctx)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = pvr_srv_create_timeline(ws->render_fd, &srv_ctx->timeline);
   if (result != VK_SUCCESS)
      goto err_free_srv_ctx;

   /* TODO: Add support for reset framework. Currently we subtract
    * reset_cmd.regs size from reset_cmd size to only pass empty flags field.
    */
   result = pvr_srv_rgx_create_compute_context(
      ws->render_fd,
      pvr_srv_from_winsys_priority(create_info->priority),
      sizeof(reset_cmd) - sizeof(reset_cmd.regs),
      (uint8_t *)&reset_cmd,
      srv_ws->server_memctx_data,
      sizeof(static_state),
      (uint8_t *)&static_state,
      0U,
      RGX_CONTEXT_FLAG_DISABLESLR,
      0U,
      UINT_MAX,
      &srv_ctx->handle);
   if (result != VK_SUCCESS)
      goto err_close_timeline;

   srv_ctx->base.ws = ws;

   *ctx_out = &srv_ctx->base;

   return VK_SUCCESS;

err_close_timeline:
   close(srv_ctx->timeline);

err_free_srv_ctx:
   vk_free(ws->alloc, srv_ctx);

   return result;
}

void pvr_srv_winsys_compute_ctx_destroy(struct pvr_winsys_compute_ctx *ctx)
{
   struct pvr_srv_winsys *srv_ws = to_pvr_srv_winsys(ctx->ws);
   struct pvr_srv_winsys_compute_ctx *srv_ctx =
      to_pvr_srv_winsys_compute_ctx(ctx);

   pvr_srv_rgx_destroy_compute_context(srv_ws->base.render_fd, srv_ctx->handle);
   close(srv_ctx->timeline);
   vk_free(srv_ws->base.alloc, srv_ctx);
}

static uint32_t
pvr_srv_compute_cmd_stream_load(struct rogue_fwif_cmd_compute *const cmd,
                                const uint8_t *const stream,
                                const uint32_t stream_len,
                                const struct pvr_device_info *const dev_info)
{
   const uint32_t *stream_ptr = (const uint32_t *)stream;
   struct rogue_fwif_cdm_regs *const regs = &cmd->regs;
   uint32_t main_stream_len =
      pvr_csb_unpack((uint64_t *)stream_ptr, KMD_STREAM_HDR).length;

   stream_ptr += pvr_cmd_length(KMD_STREAM_HDR);

   regs->tpu_border_colour_table = *(const uint64_t *)stream_ptr;
   stream_ptr += pvr_cmd_length(CR_TPU_BORDER_COLOUR_TABLE_CDM);

   regs->cdm_ctrl_stream_base = *(const uint64_t *)stream_ptr;
   stream_ptr += pvr_cmd_length(CR_CDM_CTRL_STREAM_BASE);

   regs->cdm_context_state_base_addr = *(const uint64_t *)stream_ptr;
   stream_ptr += pvr_cmd_length(CR_CDM_CONTEXT_STATE_BASE);

   regs->cdm_resume_pds1 = *stream_ptr;
   stream_ptr += pvr_cmd_length(CR_CDM_CONTEXT_PDS1);

   regs->cdm_item = *stream_ptr;
   stream_ptr += pvr_cmd_length(CR_CDM_ITEM);

   if (PVR_HAS_FEATURE(dev_info, cluster_grouping)) {
      regs->compute_cluster = *stream_ptr;
      stream_ptr += pvr_cmd_length(CR_COMPUTE_CLUSTER);
   }

   if (PVR_HAS_FEATURE(dev_info, gpu_multicore_support)) {
      cmd->execute_count = *stream_ptr;
      stream_ptr++;
   }

   assert((const uint8_t *)stream_ptr - stream <= stream_len);
   assert((const uint8_t *)stream_ptr - stream == main_stream_len);

   return main_stream_len;
}

static void pvr_srv_compute_cmd_ext_stream_load(
   struct rogue_fwif_cmd_compute *const cmd,
   const uint8_t *const stream,
   const uint32_t stream_len,
   const uint32_t ext_stream_offset,
   const struct pvr_device_info *const dev_info)
{
   const uint32_t *ext_stream_ptr =
      (const uint32_t *)((uint8_t *)stream + ext_stream_offset);
   struct rogue_fwif_cdm_regs *const regs = &cmd->regs;

   struct PVRX(KMD_STREAM_EXTHDR_COMPUTE0) header0;

   header0 = pvr_csb_unpack(ext_stream_ptr, KMD_STREAM_EXTHDR_COMPUTE0);
   ext_stream_ptr += pvr_cmd_length(KMD_STREAM_EXTHDR_COMPUTE0);

   assert(PVR_HAS_QUIRK(dev_info, 49927) == header0.has_brn49927);
   if (header0.has_brn49927) {
      regs->tpu = *ext_stream_ptr;
      ext_stream_ptr += pvr_cmd_length(CR_TPU);
   }

   assert((const uint8_t *)ext_stream_ptr - stream == stream_len);
}

static void pvr_srv_compute_cmd_init(
   const struct pvr_winsys_compute_submit_info *submit_info,
   struct rogue_fwif_cmd_compute *cmd,
   const struct pvr_device_info *const dev_info)
{
   uint32_t ext_stream_offset;

   memset(cmd, 0, sizeof(*cmd));

   cmd->cmn.frame_num = submit_info->frame_num;

   ext_stream_offset =
      pvr_srv_compute_cmd_stream_load(cmd,
                                      submit_info->fw_stream,
                                      submit_info->fw_stream_len,
                                      dev_info);

   if (ext_stream_offset < submit_info->fw_stream_len) {
      pvr_srv_compute_cmd_ext_stream_load(cmd,
                                          submit_info->fw_stream,
                                          submit_info->fw_stream_len,
                                          ext_stream_offset,
                                          dev_info);
   }

   if (submit_info->flags.prevent_all_overlap)
      cmd->flags |= ROGUE_FWIF_COMPUTE_FLAG_PREVENT_ALL_OVERLAP;

   if (submit_info->flags.use_single_core)
      cmd->flags |= ROGUE_FWIF_COMPUTE_FLAG_SINGLE_CORE;
}

VkResult pvr_srv_winsys_compute_submit(
   const struct pvr_winsys_compute_ctx *ctx,
   const struct pvr_winsys_compute_submit_info *submit_info,
   const struct pvr_device_info *const dev_info,
   struct vk_sync *signal_sync)
{
   const struct pvr_srv_winsys_compute_ctx *srv_ctx =
      to_pvr_srv_winsys_compute_ctx(ctx);
   const struct pvr_srv_winsys *srv_ws = to_pvr_srv_winsys(ctx->ws);
   struct rogue_fwif_cmd_compute compute_cmd;
   struct pvr_srv_sync *srv_signal_sync;
   VkResult result;
   int in_fd = -1;
   int fence;

   pvr_srv_compute_cmd_init(submit_info, &compute_cmd, dev_info);

   if (submit_info->wait) {
      struct pvr_srv_sync *srv_wait_sync = to_srv_sync(submit_info->wait);

      if (srv_wait_sync->fd >= 0) {
         in_fd = dup(srv_wait_sync->fd);
         if (in_fd == -1) {
            return vk_errorf(NULL,
                             VK_ERROR_OUT_OF_HOST_MEMORY,
                             "dup called on wait sync failed, Errno: %s",
                             strerror(errno));
         }
      }
   }

   do {
      result = pvr_srv_rgx_kick_compute2(srv_ws->base.render_fd,
                                         srv_ctx->handle,
                                         0U,
                                         NULL,
                                         NULL,
                                         NULL,
                                         in_fd,
                                         srv_ctx->timeline,
                                         sizeof(compute_cmd),
                                         (uint8_t *)&compute_cmd,
                                         submit_info->job_num,
                                         0,
                                         NULL,
                                         NULL,
                                         0U,
                                         0U,
                                         0U,
                                         0U,
                                         "COMPUTE",
                                         &fence);
   } while (result == VK_NOT_READY);

   if (result != VK_SUCCESS)
      goto end_close_in_fd;

   if (signal_sync) {
      srv_signal_sync = to_srv_sync(signal_sync);
      pvr_srv_set_sync_payload(srv_signal_sync, fence);
   } else if (fence != -1) {
      close(fence);
   }

end_close_in_fd:
   if (in_fd >= 0)
      close(in_fd);

   return result;
}
