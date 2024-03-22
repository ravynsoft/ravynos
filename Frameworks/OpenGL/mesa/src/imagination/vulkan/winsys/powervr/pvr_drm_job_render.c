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
#include "pvr_drm_bo.h"
#include "pvr_drm_job_common.h"
#include "pvr_drm_job_render.h"
#include "pvr_private.h"
#include "pvr_winsys.h"
#include "pvr_winsys_helper.h"
#include "util/macros.h"
#include "vk_alloc.h"
#include "vk_drm_syncobj.h"
#include "vk_log.h"
#include "vk_util.h"
#include "vk_sync.h"

#define PVR_DRM_FREE_LIST_LOCAL 0U
#define PVR_DRM_FREE_LIST_GLOBAL 1U
#define PVR_DRM_FREE_LIST_MAX 2U

struct pvr_drm_winsys_free_list {
   struct pvr_winsys_free_list base;

   uint32_t handle;

   struct pvr_drm_winsys_free_list *parent;
};

#define to_pvr_drm_winsys_free_list(free_list) \
   container_of(free_list, struct pvr_drm_winsys_free_list, base)

struct pvr_drm_winsys_rt_dataset {
   struct pvr_winsys_rt_dataset base;
   uint32_t handle;
};

#define to_pvr_drm_winsys_rt_dataset(rt_dataset) \
   container_of(rt_dataset, struct pvr_drm_winsys_rt_dataset, base)

VkResult pvr_drm_winsys_free_list_create(
   struct pvr_winsys *const ws,
   struct pvr_winsys_vma *const free_list_vma,
   uint32_t initial_num_pages,
   uint32_t max_num_pages,
   uint32_t grow_num_pages,
   uint32_t grow_threshold,
   struct pvr_winsys_free_list *const parent_free_list,
   struct pvr_winsys_free_list **const free_list_out)
{
   struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ws);
   struct drm_pvr_ioctl_create_free_list_args free_list_args = {
      .free_list_gpu_addr = free_list_vma->dev_addr.addr,
      .initial_num_pages = initial_num_pages,
      .max_num_pages = max_num_pages,
      .grow_num_pages = grow_num_pages,
      .grow_threshold = grow_threshold,
      .vm_context_handle = drm_ws->vm_context,
   };
   struct pvr_drm_winsys_free_list *drm_free_list;
   VkResult result;

   drm_free_list = vk_zalloc(ws->alloc,
                             sizeof(*drm_free_list),
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!drm_free_list) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_out;
   }

   drm_free_list->base.ws = ws;

   if (parent_free_list)
      drm_free_list->parent = to_pvr_drm_winsys_free_list(parent_free_list);

   /* Returns VK_ERROR_INITIALIZATION_FAILED to match pvrsrv. */
   result = pvr_ioctlf(ws->render_fd,
                       DRM_IOCTL_PVR_CREATE_FREE_LIST,
                       &free_list_args,
                       VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to create free list");
   if (result != VK_SUCCESS)
      goto err_free_free_list;

   drm_free_list->handle = free_list_args.handle;

   *free_list_out = &drm_free_list->base;

   return VK_SUCCESS;

err_free_free_list:
   vk_free(ws->alloc, drm_free_list);

err_out:
   return result;
}

void pvr_drm_winsys_free_list_destroy(struct pvr_winsys_free_list *free_list)
{
   struct pvr_drm_winsys_free_list *const drm_free_list =
      to_pvr_drm_winsys_free_list(free_list);
   struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(free_list->ws);
   struct drm_pvr_ioctl_destroy_free_list_args args = {
      .handle = drm_free_list->handle,
   };

   pvr_ioctlf(drm_ws->base.render_fd,
              DRM_IOCTL_PVR_DESTROY_FREE_LIST,
              &args,
              VK_ERROR_UNKNOWN,
              "Error destroying free list");

   vk_free(drm_ws->base.alloc, free_list);
}

static void pvr_drm_render_ctx_static_state_init(
   struct pvr_winsys_render_ctx_create_info *create_info,
   uint8_t *stream_ptr_start,
   uint32_t *stream_len_ptr)
{
   struct pvr_winsys_render_ctx_static_state *ws_static_state =
      &create_info->static_state;
   uint64_t *stream_ptr = (uint64_t *)stream_ptr_start;

   /* Leave space for stream header. */
   stream_ptr += pvr_cmd_length(KMD_STREAM_HDR) / 2;

   *stream_ptr++ = ws_static_state->vdm_ctx_state_base_addr;
   /* geom_reg_vdm_context_state_resume_addr is unused and zeroed. */
   *stream_ptr++ = 0;
   *stream_ptr++ = ws_static_state->geom_ctx_state_base_addr;

   for (uint32_t i = 0; i < ARRAY_SIZE(ws_static_state->geom_state); i++) {
      *stream_ptr++ = ws_static_state->geom_state[i].vdm_ctx_store_task0;
      *stream_ptr++ = ws_static_state->geom_state[i].vdm_ctx_store_task1;
      *stream_ptr++ = ws_static_state->geom_state[i].vdm_ctx_store_task2;
      /* {store, resume}_task{3, 4} are unused and zeroed. */
      *stream_ptr++ = 0;
      *stream_ptr++ = 0;

      *stream_ptr++ = ws_static_state->geom_state[i].vdm_ctx_resume_task0;
      *stream_ptr++ = ws_static_state->geom_state[i].vdm_ctx_resume_task1;
      *stream_ptr++ = ws_static_state->geom_state[i].vdm_ctx_resume_task2;
      /* {store, resume}_task{3, 4} are unused and zeroed. */
      *stream_ptr++ = 0;
      *stream_ptr++ = 0;
   }

   *stream_len_ptr = ((uint8_t *)stream_ptr - stream_ptr_start);

   pvr_csb_pack ((uint64_t *)stream_ptr_start, KMD_STREAM_HDR, value) {
      value.length = *stream_len_ptr;
   }
}

struct pvr_drm_winsys_render_ctx {
   struct pvr_winsys_render_ctx base;

   /* Handle to kernel context. */
   uint32_t handle;

   uint32_t geom_to_pr_syncobj;
};

#define to_pvr_drm_winsys_render_ctx(ctx) \
   container_of(ctx, struct pvr_drm_winsys_render_ctx, base)

VkResult pvr_drm_winsys_render_ctx_create(
   struct pvr_winsys *ws,
   struct pvr_winsys_render_ctx_create_info *create_info,
   struct pvr_winsys_render_ctx **const ctx_out)
{
   uint8_t static_ctx_state_fw_stream[192];
   struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ws);
   struct drm_pvr_ioctl_create_context_args ctx_args = {
      .type = DRM_PVR_CTX_TYPE_RENDER,
      .priority = pvr_drm_from_winsys_priority(create_info->priority),
      .static_context_state = (uint64_t)&static_ctx_state_fw_stream,
      .callstack_addr = create_info->vdm_callstack_addr.addr,
      .vm_context_handle = drm_ws->vm_context,
   };

   struct pvr_drm_winsys_render_ctx *drm_ctx;
   uint32_t geom_to_pr_syncobj;
   VkResult result;
   int ret;

   drm_ctx = vk_alloc(ws->alloc,
                      sizeof(*drm_ctx),
                      8,
                      VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!drm_ctx) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_out;
   }

   ret = drmSyncobjCreate(ws->render_fd, 0, &geom_to_pr_syncobj);
   if (ret < 0) {
      result = vk_errorf(NULL,
                         VK_ERROR_OUT_OF_HOST_MEMORY,
                         "DRM_IOCTL_SYNCOBJ_CREATE failed: %s",
                         strerror(errno));
      goto err_free_ctx;
   }

   pvr_drm_render_ctx_static_state_init(create_info,
                                        static_ctx_state_fw_stream,
                                        &ctx_args.static_context_state_len);

   result = pvr_ioctlf(ws->render_fd,
                       DRM_IOCTL_PVR_CREATE_CONTEXT,
                       &ctx_args,
                       VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to create render context");
   if (result != VK_SUCCESS)
      goto err_destroy_syncobj;

   *drm_ctx = (struct pvr_drm_winsys_render_ctx) {
      .base = {
         .ws = ws,
      },
      .handle = ctx_args.handle,
      .geom_to_pr_syncobj = geom_to_pr_syncobj,
   };

   *ctx_out = &drm_ctx->base;

   return VK_SUCCESS;

err_destroy_syncobj:
   ret = drmSyncobjDestroy(ws->render_fd, geom_to_pr_syncobj);
   if (ret < 0) {
      mesa_loge("DRM_IOCTL_SYNCOBJ_DESTROY failed: %s - leaking it",
                strerror(errno));
   }

err_free_ctx:
   vk_free(ws->alloc, drm_ctx);

err_out:
   return result;
}

void pvr_drm_winsys_render_ctx_destroy(struct pvr_winsys_render_ctx *ctx)
{
   struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ctx->ws);
   struct pvr_drm_winsys_render_ctx *drm_ctx =
      to_pvr_drm_winsys_render_ctx(ctx);
   struct drm_pvr_ioctl_destroy_context_args args = {
      .handle = drm_ctx->handle,
   };
   int ret;

   ret = drmSyncobjDestroy(ctx->ws->render_fd, drm_ctx->geom_to_pr_syncobj);
   if (ret < 0) {
      mesa_loge("DRM_IOCTL_SYNCOBJ_DESTROY failed: %s - leaking it",
                strerror(errno));
   }

   pvr_ioctlf(drm_ws->base.render_fd,
              DRM_IOCTL_PVR_DESTROY_CONTEXT,
              &args,
              VK_ERROR_UNKNOWN,
              "Error destroying render context");

   vk_free(drm_ws->base.alloc, drm_ctx);
}

VkResult pvr_drm_render_target_dataset_create(
   struct pvr_winsys *const ws,
   const struct pvr_winsys_rt_dataset_create_info *const create_info,
   UNUSED const struct pvr_device_info *dev_info,
   struct pvr_winsys_rt_dataset **const rt_dataset_out)
{
   struct pvr_drm_winsys_free_list *drm_free_list =
      to_pvr_drm_winsys_free_list(create_info->local_free_list);

   /* 0 is just a placeholder. It doesn't indicate an invalid handle. */
   uint32_t parent_free_list_handle =
      drm_free_list->parent ? drm_free_list->parent->handle : 0;

   struct drm_pvr_ioctl_create_hwrt_dataset_args args = {
      .geom_data_args = {
         .tpc_dev_addr = create_info->tpc_dev_addr.addr,
         .tpc_size = create_info->tpc_size,
         .tpc_stride = create_info->tpc_stride,
         .vheap_table_dev_addr = create_info->vheap_table_dev_addr.addr,
         .rtc_dev_addr = create_info->rtc_dev_addr.addr,
      },

      .rt_data_args = {
         [0] = {
            .pm_mlist_dev_addr =
               create_info->rt_datas[0].pm_mlist_dev_addr.addr,
            .macrotile_array_dev_addr =
               create_info->rt_datas[0].macrotile_array_dev_addr.addr,
            .region_header_dev_addr =
               create_info->rt_datas[0].rgn_header_dev_addr.addr,
         },
         [1] = {
            .pm_mlist_dev_addr =
               create_info->rt_datas[1].pm_mlist_dev_addr.addr,
            .macrotile_array_dev_addr =
               create_info->rt_datas[1].macrotile_array_dev_addr.addr,
            .region_header_dev_addr =
               create_info->rt_datas[1].rgn_header_dev_addr.addr,
         },
      },

      .free_list_handles = {
         [PVR_DRM_FREE_LIST_LOCAL] = drm_free_list->handle,
         [PVR_DRM_FREE_LIST_GLOBAL] = parent_free_list_handle,
      },

      .width = create_info->width,
      .height = create_info->height,
      .samples = create_info->samples,
      .layers = create_info->layers,

      .isp_merge_lower_x = create_info->isp_merge_lower_x,
      .isp_merge_lower_y = create_info->isp_merge_lower_y,
      .isp_merge_scale_x = create_info->isp_merge_scale_x,
      .isp_merge_scale_y = create_info->isp_merge_scale_y,
      .isp_merge_upper_x = create_info->isp_merge_upper_x,
      .isp_merge_upper_y = create_info->isp_merge_upper_y,

      .region_header_size = create_info->rgn_header_size,
   };

   struct pvr_drm_winsys_rt_dataset *drm_rt_dataset;
   VkResult result;

   STATIC_ASSERT(ARRAY_SIZE(args.rt_data_args) ==
                 ARRAY_SIZE(create_info->rt_datas));

   drm_rt_dataset = vk_zalloc(ws->alloc,
                              sizeof(*drm_rt_dataset),
                              8,
                              VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!drm_rt_dataset) {
      result = vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_out;
   }

   /* Returns VK_ERROR_INITIALIZATION_FAILED to match pvrsrv. */
   result = pvr_ioctlf(ws->render_fd,
                       DRM_IOCTL_PVR_CREATE_HWRT_DATASET,
                       &args,
                       VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to create render target dataset");
   if (result != VK_SUCCESS)
      goto err_free_dataset;

   drm_rt_dataset->handle = args.handle;
   drm_rt_dataset->base.ws = ws;

   *rt_dataset_out = &drm_rt_dataset->base;

   return VK_SUCCESS;

err_free_dataset:
   vk_free(ws->alloc, drm_rt_dataset);

err_out:
   return result;
}

void pvr_drm_render_target_dataset_destroy(
   struct pvr_winsys_rt_dataset *const rt_dataset)
{
   struct pvr_drm_winsys_rt_dataset *const drm_rt_dataset =
      to_pvr_drm_winsys_rt_dataset(rt_dataset);
   struct pvr_drm_winsys *const drm_ws = to_pvr_drm_winsys(rt_dataset->ws);
   struct drm_pvr_ioctl_destroy_hwrt_dataset_args args = {
      .handle = drm_rt_dataset->handle,
   };

   pvr_ioctlf(drm_ws->base.render_fd,
              DRM_IOCTL_PVR_DESTROY_HWRT_DATASET,
              &args,
              VK_ERROR_UNKNOWN,
              "Error destroying render target dataset");

   vk_free(drm_ws->base.alloc, drm_rt_dataset);
}

static uint32_t pvr_winsys_geom_flags_to_drm(
   const struct pvr_winsys_geometry_state_flags *const ws_flags)
{
   uint32_t flags = 0U;

   if (ws_flags->is_first_geometry)
      flags |= DRM_PVR_SUBMIT_JOB_GEOM_CMD_FIRST;

   if (ws_flags->is_last_geometry)
      flags |= DRM_PVR_SUBMIT_JOB_GEOM_CMD_LAST;

   if (ws_flags->use_single_core)
      flags |= DRM_PVR_SUBMIT_JOB_GEOM_CMD_SINGLE_CORE;

   return flags;
}

static uint32_t pvr_winsys_frag_flags_to_drm(
   const struct pvr_winsys_fragment_state_flags *const ws_flags)
{
   uint32_t flags = 0U;

   if (ws_flags->use_single_core)
      flags |= DRM_PVR_SUBMIT_JOB_FRAG_CMD_SINGLE_CORE;

   if (ws_flags->has_depth_buffer)
      flags |= DRM_PVR_SUBMIT_JOB_FRAG_CMD_DEPTHBUFFER;

   if (ws_flags->has_stencil_buffer)
      flags |= DRM_PVR_SUBMIT_JOB_FRAG_CMD_STENCILBUFFER;

   if (ws_flags->prevent_cdm_overlap)
      flags |= DRM_PVR_SUBMIT_JOB_FRAG_CMD_PREVENT_CDM_OVERLAP;

   if (ws_flags->get_vis_results)
      flags |= DRM_PVR_SUBMIT_JOB_FRAG_CMD_GET_VIS_RESULTS;

   if (ws_flags->has_spm_scratch_buffer)
      flags |= DRM_PVR_SUBMIT_JOB_FRAG_CMD_SCRATCHBUFFER;

   return flags;
}

VkResult pvr_drm_winsys_render_submit(
   const struct pvr_winsys_render_ctx *ctx,
   const struct pvr_winsys_render_submit_info *submit_info,
   UNUSED const struct pvr_device_info *dev_info,
   struct vk_sync *signal_sync_geom,
   struct vk_sync *signal_sync_frag)

{
   const struct pvr_drm_winsys *drm_ws = to_pvr_drm_winsys(ctx->ws);
   const struct pvr_drm_winsys_render_ctx *drm_ctx =
      to_pvr_drm_winsys_render_ctx(ctx);
   const struct pvr_winsys_geometry_state *const geom_state =
      &submit_info->geometry;
   const struct pvr_winsys_fragment_state *const frag_state =
      &submit_info->fragment;
   const struct pvr_winsys_fragment_state *const pr_state =
      &submit_info->fragment_pr;
   const struct pvr_drm_winsys_rt_dataset *drm_rt_dataset =
      to_pvr_drm_winsys_rt_dataset(submit_info->rt_dataset);

   struct drm_pvr_sync_op geom_sync_ops[2], pr_sync_ops[1], frag_sync_ops[3];
   unsigned num_geom_syncs = 0, num_pr_syncs = 0, num_frag_syncs = 0;
   uint32_t geom_to_pr_syncobj;

   struct drm_pvr_job jobs_args[3] = {
      [0] = {
         .type = DRM_PVR_JOB_TYPE_GEOMETRY,
         .cmd_stream = (__u64)&geom_state->fw_stream[0],
         .cmd_stream_len = geom_state->fw_stream_len,
         .context_handle = drm_ctx->handle,
         .flags = pvr_winsys_geom_flags_to_drm(&geom_state->flags),
         .sync_ops = DRM_PVR_OBJ_ARRAY(0, geom_sync_ops),
         .hwrt = {
            .set_handle = drm_rt_dataset->handle,
            .data_index = submit_info->rt_data_idx,
         },
      },
      [1] = {
         .type = DRM_PVR_JOB_TYPE_FRAGMENT,
         .cmd_stream = (__u64)&pr_state->fw_stream[0],
         .cmd_stream_len = pr_state->fw_stream_len,
         .context_handle = drm_ctx->handle,
         .flags = DRM_PVR_SUBMIT_JOB_FRAG_CMD_PARTIAL_RENDER |
                  pvr_winsys_frag_flags_to_drm(&pr_state->flags),
         .sync_ops = DRM_PVR_OBJ_ARRAY(0, pr_sync_ops),
         .hwrt = {
            .set_handle = drm_rt_dataset->handle,
            .data_index = submit_info->rt_data_idx,
         },
      }
   };

   struct drm_pvr_ioctl_submit_jobs_args args = {
      .jobs = DRM_PVR_OBJ_ARRAY(2, jobs_args),
   };

   /* Geom syncs */

   if (submit_info->geometry.wait) {
      struct vk_sync *sync = submit_info->geometry.wait;

      assert(!(sync->flags & VK_SYNC_IS_TIMELINE));
      geom_sync_ops[num_geom_syncs++] = (struct drm_pvr_sync_op){
         .handle = vk_sync_as_drm_syncobj(sync)->syncobj,
         .flags = DRM_PVR_SYNC_OP_FLAG_WAIT |
                  DRM_PVR_SYNC_OP_FLAG_HANDLE_TYPE_SYNCOBJ,
         .value = 0,
      };
   }

   if (signal_sync_geom) {
      assert(!(signal_sync_geom->flags & VK_SYNC_IS_TIMELINE));
      geom_sync_ops[num_geom_syncs++] = (struct drm_pvr_sync_op){
         .handle = vk_sync_as_drm_syncobj(signal_sync_geom)->syncobj,
         .flags = DRM_PVR_SYNC_OP_FLAG_SIGNAL |
                  DRM_PVR_SYNC_OP_FLAG_HANDLE_TYPE_SYNCOBJ,
         .value = 0,
      };
   }

   /* PR syncs */

   if (signal_sync_geom) {
      assert(!(signal_sync_geom->flags & VK_SYNC_IS_TIMELINE));
      geom_to_pr_syncobj = vk_sync_as_drm_syncobj(signal_sync_geom)->syncobj;
   } else {
      geom_to_pr_syncobj = drm_ctx->geom_to_pr_syncobj;

      geom_sync_ops[num_geom_syncs++] = (struct drm_pvr_sync_op){
         .handle = geom_to_pr_syncobj,
         .flags = DRM_PVR_SYNC_OP_FLAG_SIGNAL |
                  DRM_PVR_SYNC_OP_FLAG_HANDLE_TYPE_SYNCOBJ,
         .value = 0,
      };
   }

   pr_sync_ops[num_pr_syncs++] = (struct drm_pvr_sync_op){
      .handle = geom_to_pr_syncobj,
      .flags = DRM_PVR_SYNC_OP_FLAG_WAIT |
               DRM_PVR_SYNC_OP_FLAG_HANDLE_TYPE_SYNCOBJ,
      .value = 0,
   };

   /* Frag job */

   if (submit_info->has_fragment_job) {
      jobs_args[args.jobs.count++] = (struct drm_pvr_job) {
         .type = DRM_PVR_JOB_TYPE_FRAGMENT,
         .cmd_stream = (__u64)&frag_state->fw_stream[0],
         .cmd_stream_len = frag_state->fw_stream_len,
         .context_handle = drm_ctx->handle,
         .flags = pvr_winsys_frag_flags_to_drm(&frag_state->flags),
         .sync_ops = DRM_PVR_OBJ_ARRAY(0, frag_sync_ops),
         .hwrt = {
            .set_handle = drm_rt_dataset->handle,
            .data_index = submit_info->rt_data_idx,
         },
      };

      /* There's no need to setup a geom -> frag dependency here, as we always
       * setup a geom -> pr dependency (a PR just being a frag job) and the KMD
       * respects submission order for jobs of the same type.
       *
       * Note that, in the case where PRs aren't needed, because we didn't run
       * out of PB space during the geometry phase, the PR job will still be
       * scheduled after the geometry job, but no PRs will be performed, as
       * they aren't needed.
       */

      if (submit_info->fragment.wait) {
         struct vk_sync *sync = submit_info->fragment.wait;

         assert(!(sync->flags & VK_SYNC_IS_TIMELINE));
         frag_sync_ops[num_frag_syncs++] = (struct drm_pvr_sync_op){
            .handle = vk_sync_as_drm_syncobj(sync)->syncobj,
            .flags = DRM_PVR_SYNC_OP_FLAG_WAIT |
                     DRM_PVR_SYNC_OP_FLAG_HANDLE_TYPE_SYNCOBJ,
            .value = 0,
         };
      }

      if (signal_sync_frag) {
         assert(!(signal_sync_frag->flags & VK_SYNC_IS_TIMELINE));
         frag_sync_ops[num_frag_syncs++] = (struct drm_pvr_sync_op){
            .handle = vk_sync_as_drm_syncobj(signal_sync_frag)->syncobj,
            .flags = DRM_PVR_SYNC_OP_FLAG_SIGNAL |
                     DRM_PVR_SYNC_OP_FLAG_HANDLE_TYPE_SYNCOBJ,
            .value = 0,
         };
      }
   }

   jobs_args[0].sync_ops.count = num_geom_syncs;
   jobs_args[1].sync_ops.count = num_pr_syncs;

   if (submit_info->has_fragment_job)
      jobs_args[2].sync_ops.count = num_frag_syncs;

   /* Returns VK_ERROR_OUT_OF_DEVICE_MEMORY to match pvrsrv. */
   return pvr_ioctlf(drm_ws->base.render_fd,
                     DRM_IOCTL_PVR_SUBMIT_JOBS,
                     &args,
                     VK_ERROR_OUT_OF_DEVICE_MEMORY,
                     "Failed to submit render job");
}
