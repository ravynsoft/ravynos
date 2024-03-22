/*
 * Copyright © 2019 Raspberry Pi Ltd
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

#include "broadcom/common/v3d_csd.h"
#include "v3dv_private.h"
#include "util/u_pack_color.h"
#include "vk_util.h"
#include "vulkan/runtime/vk_common_entrypoints.h"

void
v3dv_job_add_bo(struct v3dv_job *job, struct v3dv_bo *bo)
{
   if (!bo)
      return;

   if (job->bo_handle_mask & bo->handle_bit) {
      if (_mesa_set_search(job->bos, bo))
         return;
   }

   _mesa_set_add(job->bos, bo);
   job->bo_count++;
   job->bo_handle_mask |= bo->handle_bit;
}

void
v3dv_job_add_bo_unchecked(struct v3dv_job *job, struct v3dv_bo *bo)
{
   assert(bo);
   _mesa_set_add(job->bos, bo);
   job->bo_count++;
   job->bo_handle_mask |= bo->handle_bit;
}

static void
cmd_buffer_init(struct v3dv_cmd_buffer *cmd_buffer,
                struct v3dv_device *device)
{
   /* Do not reset the base object! If we are calling this from a command
    * buffer reset that would reset the loader's dispatch table for the
    * command buffer, and any other relevant info from vk_object_base
    */
   const uint32_t base_size = sizeof(struct vk_command_buffer);
   uint8_t *cmd_buffer_driver_start = ((uint8_t *) cmd_buffer) + base_size;
   memset(cmd_buffer_driver_start, 0, sizeof(*cmd_buffer) - base_size);

   cmd_buffer->device = device;

   list_inithead(&cmd_buffer->private_objs);
   list_inithead(&cmd_buffer->jobs);

   cmd_buffer->state.subpass_idx = -1;
   cmd_buffer->state.meta.subpass_idx = -1;

   cmd_buffer->status = V3DV_CMD_BUFFER_STATUS_INITIALIZED;
}

static VkResult
cmd_buffer_create(struct vk_command_pool *pool,
                  struct vk_command_buffer **cmd_buffer_out)
{
   struct v3dv_device *device =
      container_of(pool->base.device, struct v3dv_device, vk);

   struct v3dv_cmd_buffer *cmd_buffer;
   cmd_buffer = vk_zalloc(&pool->alloc,
                          sizeof(*cmd_buffer),
                          8,
                          VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (cmd_buffer == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* Here we pass 0 as level because this callback hook doesn't have the level
    * info, but that's fine, vk_common_AllocateCommandBuffers will fix it up
    * after creation.
    */
   VkResult result;
   result = vk_command_buffer_init(pool, &cmd_buffer->vk,
                                   &v3dv_cmd_buffer_ops, 0 /* level */);
   if (result != VK_SUCCESS) {
      vk_free(&pool->alloc, cmd_buffer);
      return result;
   }

   cmd_buffer_init(cmd_buffer, device);

   *cmd_buffer_out = &cmd_buffer->vk;

   return VK_SUCCESS;
}

static void
job_destroy_gpu_cl_resources(struct v3dv_job *job)
{
   assert(job->type == V3DV_JOB_TYPE_GPU_CL ||
          job->type == V3DV_JOB_TYPE_GPU_CL_SECONDARY);

   v3dv_cl_destroy(&job->bcl);
   v3dv_cl_destroy(&job->rcl);
   v3dv_cl_destroy(&job->indirect);

   /* Since we don't ref BOs when we add them to the command buffer, don't
    * unref them here either. Bo's will be freed when their corresponding API
    * objects are destroyed.
    */
   _mesa_set_destroy(job->bos, NULL);

   v3dv_bo_free(job->device, job->tile_alloc);
   v3dv_bo_free(job->device, job->tile_state);
}

static void
job_destroy_cloned_gpu_cl_resources(struct v3dv_job *job)
{
   assert(job->type == V3DV_JOB_TYPE_GPU_CL);

   list_for_each_entry_safe(struct v3dv_bo, bo, &job->bcl.bo_list, list_link) {
      list_del(&bo->list_link);
      vk_free(&job->device->vk.alloc, bo);
   }

   list_for_each_entry_safe(struct v3dv_bo, bo, &job->rcl.bo_list, list_link) {
      list_del(&bo->list_link);
      vk_free(&job->device->vk.alloc, bo);
   }

   list_for_each_entry_safe(struct v3dv_bo, bo, &job->indirect.bo_list, list_link) {
      list_del(&bo->list_link);
      vk_free(&job->device->vk.alloc, bo);
   }
}

static void
job_destroy_gpu_csd_resources(struct v3dv_job *job)
{
   assert(job->type == V3DV_JOB_TYPE_GPU_CSD);
   assert(job->cmd_buffer);

   v3dv_cl_destroy(&job->indirect);

   _mesa_set_destroy(job->bos, NULL);

   if (job->csd.shared_memory)
      v3dv_bo_free(job->device, job->csd.shared_memory);
}

void
v3dv_job_destroy(struct v3dv_job *job)
{
   assert(job);

   list_del(&job->list_link);

   /* Cloned jobs don't make deep copies of the original jobs, so they don't
    * own any of their resources. However, they do allocate clones of BO
    * structs, so make sure we free those.
    */
   if (!job->is_clone) {
      switch (job->type) {
      case V3DV_JOB_TYPE_GPU_CL:
      case V3DV_JOB_TYPE_GPU_CL_SECONDARY:
         job_destroy_gpu_cl_resources(job);
         break;
      case V3DV_JOB_TYPE_GPU_CSD:
         job_destroy_gpu_csd_resources(job);
         break;
      default:
         break;
      }
   } else {
      /* Cloned jobs */
      if (job->type == V3DV_JOB_TYPE_GPU_CL)
         job_destroy_cloned_gpu_cl_resources(job);
   }

   vk_free(&job->device->vk.alloc, job);
}

void
v3dv_cmd_buffer_add_private_obj(struct v3dv_cmd_buffer *cmd_buffer,
                                uint64_t obj,
                                v3dv_cmd_buffer_private_obj_destroy_cb destroy_cb)
{
   struct v3dv_cmd_buffer_private_obj *pobj =
      vk_alloc(&cmd_buffer->device->vk.alloc, sizeof(*pobj), 8,
               VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!pobj) {
      v3dv_flag_oom(cmd_buffer, NULL);
      return;
   }

   pobj->obj = obj;
   pobj->destroy_cb = destroy_cb;

   list_addtail(&pobj->list_link, &cmd_buffer->private_objs);
}

static void
cmd_buffer_destroy_private_obj(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_cmd_buffer_private_obj *pobj)
{
   assert(pobj && pobj->obj && pobj->destroy_cb);
   pobj->destroy_cb(v3dv_device_to_handle(cmd_buffer->device),
                    pobj->obj,
                    &cmd_buffer->device->vk.alloc);
   list_del(&pobj->list_link);
   vk_free(&cmd_buffer->device->vk.alloc, pobj);
}

static void
cmd_buffer_free_resources(struct v3dv_cmd_buffer *cmd_buffer)
{
   list_for_each_entry_safe(struct v3dv_job, job,
                            &cmd_buffer->jobs, list_link) {
      v3dv_job_destroy(job);
   }

   if (cmd_buffer->state.job)
      v3dv_job_destroy(cmd_buffer->state.job);

   if (cmd_buffer->state.attachments)
      vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer->state.attachments);

   if (cmd_buffer->state.query.end.alloc_count > 0)
      vk_free(&cmd_buffer->device->vk.alloc, cmd_buffer->state.query.end.states);

   if (cmd_buffer->push_constants_resource.bo)
      v3dv_bo_free(cmd_buffer->device, cmd_buffer->push_constants_resource.bo);

   list_for_each_entry_safe(struct v3dv_cmd_buffer_private_obj, pobj,
                            &cmd_buffer->private_objs, list_link) {
      cmd_buffer_destroy_private_obj(cmd_buffer, pobj);
   }

   if (cmd_buffer->state.meta.attachments) {
         assert(cmd_buffer->state.meta.attachment_alloc_count > 0);
         vk_free(&cmd_buffer->device->vk.alloc, cmd_buffer->state.meta.attachments);
   }
}

static void
cmd_buffer_destroy(struct vk_command_buffer *vk_cmd_buffer)
{
   struct v3dv_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct v3dv_cmd_buffer, vk);

   cmd_buffer_free_resources(cmd_buffer);
   vk_command_buffer_finish(&cmd_buffer->vk);
   vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer);
}

static bool
cmd_buffer_can_merge_subpass(struct v3dv_cmd_buffer *cmd_buffer,
                             uint32_t subpass_idx)
{
   const struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   assert(state->pass);

   const struct v3dv_physical_device *physical_device =
      cmd_buffer->device->pdevice;

   if (cmd_buffer->vk.level != VK_COMMAND_BUFFER_LEVEL_PRIMARY)
      return false;

   if (!cmd_buffer->state.job)
      return false;

   if (cmd_buffer->state.job->always_flush)
      return false;

   if (!physical_device->options.merge_jobs)
      return false;

   /* Each render pass starts a new job */
   if (subpass_idx == 0)
      return false;

   /* Two subpasses can be merged in the same job if we can emit a single RCL
    * for them (since the RCL includes the END_OF_RENDERING command that
    * triggers the "render job finished" interrupt). We can do this so long
    * as both subpasses render against the same attachments.
    */
   assert(state->subpass_idx == subpass_idx - 1);
   struct v3dv_subpass *prev_subpass = &state->pass->subpasses[state->subpass_idx];
   struct v3dv_subpass *subpass = &state->pass->subpasses[subpass_idx];

   if (subpass->ds_attachment.attachment !=
       prev_subpass->ds_attachment.attachment)
      return false;

   if (subpass->color_count != prev_subpass->color_count)
      return false;

   for (uint32_t i = 0; i < subpass->color_count; i++) {
      if (subpass->color_attachments[i].attachment !=
          prev_subpass->color_attachments[i].attachment) {
         return false;
      }
   }

   /* Don't merge if the subpasses have different view masks, since in that
    * case the framebuffer setup is different and we need to emit different
    * RCLs.
    */
   if (subpass->view_mask != prev_subpass->view_mask)
      return false;

   /* FIXME: Since some attachment formats can't be resolved using the TLB we
    * need to emit separate resolve jobs for them and that would not be
    * compatible with subpass merges. We could fix that by testing if any of
    * the attachments to resolve doesn't support TLB resolves.
    */
   if (prev_subpass->resolve_attachments || subpass->resolve_attachments ||
       prev_subpass->resolve_depth || prev_subpass->resolve_stencil ||
       subpass->resolve_depth || subpass->resolve_stencil) {
      return false;
   }

   return true;
}

/**
 * Computes and sets the job frame tiling information required to setup frame
 * binning and rendering.
 */
static struct v3dv_frame_tiling *
job_compute_frame_tiling(struct v3dv_job *job,
                         uint32_t width,
                         uint32_t height,
                         uint32_t layers,
                         uint32_t render_target_count,
                         uint8_t max_internal_bpp,
                         uint8_t total_color_bpp,
                         bool msaa,
                         bool double_buffer)
{
   assert(job);
   struct v3dv_frame_tiling *tiling = &job->frame_tiling;

   tiling->width = width;
   tiling->height = height;
   tiling->layers = layers;
   tiling->render_target_count = render_target_count;
   tiling->msaa = msaa;
   tiling->internal_bpp = max_internal_bpp;
   tiling->total_color_bpp = total_color_bpp;
   tiling->double_buffer = double_buffer;

   /* Double-buffer is incompatible with MSAA */
   assert(!tiling->msaa || !tiling->double_buffer);

   v3d_choose_tile_size(&job->device->devinfo,
                        render_target_count,
                        max_internal_bpp, total_color_bpp, msaa,
                        tiling->double_buffer,
                        &tiling->tile_width, &tiling->tile_height);

   tiling->draw_tiles_x = DIV_ROUND_UP(width, tiling->tile_width);
   tiling->draw_tiles_y = DIV_ROUND_UP(height, tiling->tile_height);

   /* Size up our supertiles until we get under the limit */
   const uint32_t max_supertiles = 256;
   tiling->supertile_width = 1;
   tiling->supertile_height = 1;
   for (;;) {
      tiling->frame_width_in_supertiles =
         DIV_ROUND_UP(tiling->draw_tiles_x, tiling->supertile_width);
      tiling->frame_height_in_supertiles =
         DIV_ROUND_UP(tiling->draw_tiles_y, tiling->supertile_height);
      const uint32_t num_supertiles = tiling->frame_width_in_supertiles *
                                      tiling->frame_height_in_supertiles;
      if (num_supertiles < max_supertiles)
         break;

      if (tiling->supertile_width < tiling->supertile_height)
         tiling->supertile_width++;
      else
         tiling->supertile_height++;
   }

   return tiling;
}

bool
v3dv_job_allocate_tile_state(struct v3dv_job *job)
{
   struct v3dv_frame_tiling *tiling = &job->frame_tiling;
   const uint32_t layers =
      job->allocate_tile_state_for_all_layers ? tiling->layers : 1;

   /* The PTB will request the tile alloc initial size per tile at start
    * of tile binning.
    */
   uint32_t tile_alloc_size = 64 * layers *
                              tiling->draw_tiles_x *
                              tiling->draw_tiles_y;

   /* The PTB allocates in aligned 4k chunks after the initial setup. */
   tile_alloc_size = align(tile_alloc_size, 4096);

   /* Include the first two chunk allocations that the PTB does so that
    * we definitely clear the OOM condition before triggering one (the HW
    * won't trigger OOM during the first allocations).
    */
   tile_alloc_size += 8192;

   /* For performance, allocate some extra initial memory after the PTB's
    * minimal allocations, so that we hopefully don't have to block the
    * GPU on the kernel handling an OOM signal.
    */
   tile_alloc_size += 512 * 1024;

   job->tile_alloc = v3dv_bo_alloc(job->device, tile_alloc_size,
                                   "tile_alloc", true);
   if (!job->tile_alloc) {
      v3dv_flag_oom(NULL, job);
      return false;
   }

   v3dv_job_add_bo_unchecked(job, job->tile_alloc);

   const uint32_t tsda_per_tile_size = 256;
   const uint32_t tile_state_size = layers *
                                    tiling->draw_tiles_x *
                                    tiling->draw_tiles_y *
                                    tsda_per_tile_size;
   job->tile_state = v3dv_bo_alloc(job->device, tile_state_size, "TSDA", true);
   if (!job->tile_state) {
      v3dv_flag_oom(NULL, job);
      return false;
   }

   v3dv_job_add_bo_unchecked(job, job->tile_state);
   return true;
}

void
v3dv_job_start_frame(struct v3dv_job *job,
                     uint32_t width,
                     uint32_t height,
                     uint32_t layers,
                     bool allocate_tile_state_for_all_layers,
                     bool allocate_tile_state_now,
                     uint32_t render_target_count,
                     uint8_t max_internal_bpp,
                     uint8_t total_color_bpp,
                     bool msaa)
{
   assert(job);

   /* Start by computing frame tiling spec for this job assuming that
    * double-buffer mode is disabled.
    */
   const struct v3dv_frame_tiling *tiling =
      job_compute_frame_tiling(job, width, height, layers,
                               render_target_count, max_internal_bpp,
                               total_color_bpp, msaa, false);

   v3dv_cl_ensure_space_with_branch(&job->bcl, 256);
   v3dv_return_if_oom(NULL, job);

   job->allocate_tile_state_for_all_layers = allocate_tile_state_for_all_layers;

   /* For subpass jobs we postpone tile state allocation until we are finishing
    * the job and have made a decision about double-buffer.
    */
   if (allocate_tile_state_now) {
      if (!v3dv_job_allocate_tile_state(job))
         return;
   }

   v3dv_X(job->device, job_emit_binning_prolog)(job, tiling,
      allocate_tile_state_for_all_layers ? tiling->layers : 1);

   job->ez_state = V3D_EZ_UNDECIDED;
   job->first_ez_state = V3D_EZ_UNDECIDED;
}

static bool
job_should_enable_double_buffer(struct v3dv_job *job)
{
   /* Incompatibility with double-buffer */
   if (!job->can_use_double_buffer)
      return false;

   /* Too much geometry processing */
   if (job->double_buffer_score.geom > 2000000)
      return false;

   /* Too little rendering to make up for tile store latency */
   if (job->double_buffer_score.render < 100000)
      return false;

   return true;
}

static void
cmd_buffer_end_render_pass_frame(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);


   /* For subpass jobs we always emit the RCL here */
   assert(v3dv_cl_offset(&job->rcl) == 0);

   /* Decide if we want to enable double-buffer for this job. If we do, then
    * we need to rewrite the TILE_BINNING_MODE_CFG packet in the BCL.
    */
   if (job_should_enable_double_buffer(job)) {
      assert(!job->frame_tiling.double_buffer);
      job_compute_frame_tiling(job,
                               job->frame_tiling.width,
                               job->frame_tiling.height,
                               job->frame_tiling.layers,
                               job->frame_tiling.render_target_count,
                               job->frame_tiling.internal_bpp,
                               job->frame_tiling.total_color_bpp,
                               job->frame_tiling.msaa,
                               true);

      v3dv_X(job->device, job_emit_enable_double_buffer)(job);
   }

   /* At this point we have decided whether we want to use double-buffer or
    * not and the job's frame tiling represents that decision so we can
    * allocate the tile state, which we need to do before we emit the RCL.
    */
   v3dv_job_allocate_tile_state(job);

   v3dv_X(cmd_buffer->device, cmd_buffer_emit_render_pass_rcl)(cmd_buffer);

   v3dv_X(cmd_buffer->device, job_emit_binning_flush)(job);
}

struct v3dv_job *
v3dv_cmd_buffer_create_cpu_job(struct v3dv_device *device,
                               enum v3dv_job_type type,
                               struct v3dv_cmd_buffer *cmd_buffer,
                               uint32_t subpass_idx)
{
   struct v3dv_job *job = vk_zalloc(&device->vk.alloc,
                                    sizeof(struct v3dv_job), 8,
                                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!job) {
      v3dv_flag_oom(cmd_buffer, NULL);
      return NULL;
   }

   v3dv_job_init(job, type, device, cmd_buffer, subpass_idx);
   return job;
}

static void
cmd_buffer_emit_end_query_cpu(struct v3dv_cmd_buffer *cmd_buffer,
                              struct v3dv_query_pool *pool,
                              uint32_t query, uint32_t count)
{
   assert(pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR);

   struct v3dv_job *job =
      v3dv_cmd_buffer_create_cpu_job(cmd_buffer->device,
                                     V3DV_JOB_TYPE_CPU_END_QUERY,
                                     cmd_buffer, -1);
   v3dv_return_if_oom(cmd_buffer, NULL);

   job->cpu.query_end.pool = pool;
   job->cpu.query_end.query = query;
   job->cpu.query_end.count = count;
   list_addtail(&job->list_link, &cmd_buffer->jobs);
}

static void
cmd_buffer_add_jobs_for_pending_state(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;

   if (state->query.end.used_count > 0) {
      const uint32_t count = state->query.end.used_count;
      for (uint32_t i = 0; i < count; i++) {
         assert(i < state->query.end.used_count);
         struct v3dv_end_query_info *info = &state->query.end.states[i];
          if (info->pool->query_type == VK_QUERY_TYPE_OCCLUSION) {
            v3dv_cmd_buffer_emit_set_query_availability(cmd_buffer, info->pool,
                                                        info->query, info->count, 1);
         } else {
            cmd_buffer_emit_end_query_cpu(cmd_buffer, info->pool,
                                          info->query, info->count);
         }
      }
      state->query.end.used_count = 0;
   }
}

void
v3dv_cmd_buffer_finish_job(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   if (!job)
      return;

   /* Always clear BCL state after a job has been finished if we don't have
    * a pending graphics barrier that could consume it (BCL barriers only
    * apply to graphics jobs). This can happen if the application recorded
    * a barrier involving geometry stages but none of the draw calls in the
    * job actually required a binning sync.
    */
   if (!(cmd_buffer->state.barrier.dst_mask & V3DV_BARRIER_GRAPHICS_BIT)) {
      cmd_buffer->state.barrier.bcl_buffer_access = 0;
      cmd_buffer->state.barrier.bcl_image_access = 0;
   }

   if (cmd_buffer->state.oom) {
      v3dv_job_destroy(job);
      cmd_buffer->state.job = NULL;
      return;
   }

   /* If we have created a job for a command buffer then we should have
    * recorded something into it: if the job was started in a render pass, it
    * should at least have the start frame commands, otherwise, it should have
    * a transfer command. The only exception are secondary command buffers
    * inside a render pass.
    */
   assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY ||
          v3dv_cl_offset(&job->bcl) > 0);

   /* When we merge multiple subpasses into the same job we must only emit one
    * RCL, so we do that here, when we decided that we need to finish the job.
    * Any rendering that happens outside a render pass is never merged, so
    * the RCL should have been emitted by the time we got here.
    */
   assert(v3dv_cl_offset(&job->rcl) != 0 || cmd_buffer->state.pass);

   /* If we are finishing a job inside a render pass we have two scenarios:
    *
    * 1. It is a regular CL, in which case we will submit the job to the GPU,
    *    so we may need to generate an RCL and add a binning flush.
    *
    * 2. It is a partial CL recorded in a secondary command buffer, in which
    *    case we are not submitting it directly to the GPU but rather branch to
    *    it from a primary command buffer. In this case we just want to end
    *    the BCL with a RETURN_FROM_SUB_LIST and the RCL and binning flush
    *    will be the primary job that branches to this CL.
    */
   if (cmd_buffer->state.pass) {
      if (job->type == V3DV_JOB_TYPE_GPU_CL) {
         cmd_buffer_end_render_pass_frame(cmd_buffer);
      } else {
         assert(job->type == V3DV_JOB_TYPE_GPU_CL_SECONDARY);
         v3dv_X(cmd_buffer->device, cmd_buffer_end_render_pass_secondary)(cmd_buffer);
      }
   }

   list_addtail(&job->list_link, &cmd_buffer->jobs);
   cmd_buffer->state.job = NULL;

   /* If we have recorded any state with this last GPU job that requires to
    * emit jobs after the job is completed, add them now. The only exception
    * is secondary command buffers inside a render pass, because in
    * that case we want to defer this until we finish recording the primary
    * job into which we execute the secondary.
    */
   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY ||
       !cmd_buffer->state.pass) {
      cmd_buffer_add_jobs_for_pending_state(cmd_buffer);
   }
}

bool
v3dv_job_type_is_gpu(struct v3dv_job *job)
{
   switch (job->type) {
   case V3DV_JOB_TYPE_GPU_CL:
   case V3DV_JOB_TYPE_GPU_CL_SECONDARY:
   case V3DV_JOB_TYPE_GPU_TFU:
   case V3DV_JOB_TYPE_GPU_CSD:
      return true;
   default:
      return false;
   }
}

static void
cmd_buffer_serialize_job_if_needed(struct v3dv_cmd_buffer *cmd_buffer,
                                   struct v3dv_job *job)
{
   assert(cmd_buffer && job);

   /* Serialization only affects GPU jobs, CPU jobs are always automatically
    * serialized.
    */
   if (!v3dv_job_type_is_gpu(job))
      return;

   uint8_t barrier_mask = cmd_buffer->state.barrier.dst_mask;
   if (barrier_mask == 0)
      return;

   uint8_t bit = 0;
   uint8_t *src_mask;
   if (job->type == V3DV_JOB_TYPE_GPU_CSD) {
      assert(!job->is_transfer);
      bit = V3DV_BARRIER_COMPUTE_BIT;
      src_mask = &cmd_buffer->state.barrier.src_mask_compute;
   } else if (job->is_transfer) {
      assert(job->type == V3DV_JOB_TYPE_GPU_CL ||
             job->type == V3DV_JOB_TYPE_GPU_CL_SECONDARY ||
             job->type == V3DV_JOB_TYPE_GPU_TFU);
      bit = V3DV_BARRIER_TRANSFER_BIT;
      src_mask = &cmd_buffer->state.barrier.src_mask_transfer;
   } else {
      assert(job->type == V3DV_JOB_TYPE_GPU_CL ||
             job->type == V3DV_JOB_TYPE_GPU_CL_SECONDARY);
      bit = V3DV_BARRIER_GRAPHICS_BIT;
      src_mask = &cmd_buffer->state.barrier.src_mask_graphics;
   }

   if (barrier_mask & bit) {
      job->serialize = *src_mask;
      *src_mask = 0;
      cmd_buffer->state.barrier.dst_mask &= ~bit;
   }
}

void
v3dv_job_init(struct v3dv_job *job,
              enum v3dv_job_type type,
              struct v3dv_device *device,
              struct v3dv_cmd_buffer *cmd_buffer,
              int32_t subpass_idx)
{
   assert(job);

   /* Make sure we haven't made this new job current before calling here */
   assert(!cmd_buffer || cmd_buffer->state.job != job);

   job->type = type;

   job->device = device;
   job->cmd_buffer = cmd_buffer;

   list_inithead(&job->list_link);

   if (type == V3DV_JOB_TYPE_GPU_CL ||
       type == V3DV_JOB_TYPE_GPU_CL_SECONDARY ||
       type == V3DV_JOB_TYPE_GPU_CSD) {
      job->bos =
         _mesa_set_create(NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);
      job->bo_count = 0;

      v3dv_cl_init(job, &job->indirect);

      if (V3D_DBG(ALWAYS_FLUSH))
         job->always_flush = true;
   }

   if (type == V3DV_JOB_TYPE_GPU_CL ||
       type == V3DV_JOB_TYPE_GPU_CL_SECONDARY) {
      v3dv_cl_init(job, &job->bcl);
      v3dv_cl_init(job, &job->rcl);
   }

   if (cmd_buffer) {
      /* Flag all state as dirty. Generally, we need to re-emit state for each
       * new job.
       *
       * FIXME: there may be some exceptions, in which case we could skip some
       * bits.
       */
      cmd_buffer->state.dirty = ~0;
      cmd_buffer->state.dirty_descriptor_stages = ~0;

      /* Honor inheritance of occlusion queries in secondaries if requested */
      if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY &&
          cmd_buffer->state.inheritance.occlusion_query_enable) {
         cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_OCCLUSION_QUERY;
      }

      /* Keep track of the first subpass that we are recording in this new job.
       * We will use this when we emit the RCL to decide how to emit our loads
       * and stores.
       */
      if (cmd_buffer->state.pass)
         job->first_subpass = subpass_idx;

      job->is_transfer = cmd_buffer->state.is_transfer;

      cmd_buffer_serialize_job_if_needed(cmd_buffer, job);

      job->perf = cmd_buffer->state.query.active_query.perf;
   }
}

struct v3dv_job *
v3dv_cmd_buffer_start_job(struct v3dv_cmd_buffer *cmd_buffer,
                          int32_t subpass_idx,
                          enum v3dv_job_type type)
{
   /* Don't create a new job if we can merge the current subpass into
    * the current job.
    */
   if (cmd_buffer->state.pass &&
       subpass_idx != -1 &&
       cmd_buffer_can_merge_subpass(cmd_buffer, subpass_idx)) {
      cmd_buffer->state.job->is_subpass_finish = false;
      return cmd_buffer->state.job;
   }

   /* Ensure we are not starting a new job without finishing a previous one */
   if (cmd_buffer->state.job != NULL)
      v3dv_cmd_buffer_finish_job(cmd_buffer);

   assert(cmd_buffer->state.job == NULL);
   struct v3dv_job *job = vk_zalloc(&cmd_buffer->device->vk.alloc,
                                    sizeof(struct v3dv_job), 8,
                                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

   if (!job) {
      fprintf(stderr, "Error: failed to allocate CPU memory for job\n");
      v3dv_flag_oom(cmd_buffer, NULL);
      return NULL;
   }

   v3dv_job_init(job, type, cmd_buffer->device, cmd_buffer, subpass_idx);
   cmd_buffer->state.job = job;

   return job;
}

static void
cmd_buffer_reset(struct vk_command_buffer *vk_cmd_buffer,
                 VkCommandBufferResetFlags flags)
{
   struct v3dv_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct v3dv_cmd_buffer, vk);

   vk_command_buffer_reset(&cmd_buffer->vk);
   if (cmd_buffer->status != V3DV_CMD_BUFFER_STATUS_INITIALIZED) {
      struct v3dv_device *device = cmd_buffer->device;

      /* FIXME: For now we always free all resources as if
       * VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT was set.
       */
      if (cmd_buffer->status != V3DV_CMD_BUFFER_STATUS_NEW)
         cmd_buffer_free_resources(cmd_buffer);

      cmd_buffer_init(cmd_buffer, device);
   }

   assert(cmd_buffer->status == V3DV_CMD_BUFFER_STATUS_INITIALIZED);
}


static void
cmd_buffer_emit_resolve(struct v3dv_cmd_buffer *cmd_buffer,
                        uint32_t dst_attachment_idx,
                        uint32_t src_attachment_idx,
                        VkImageAspectFlagBits aspect)
{
   struct v3dv_image_view *src_iview =
      cmd_buffer->state.attachments[src_attachment_idx].image_view;
   struct v3dv_image_view *dst_iview =
      cmd_buffer->state.attachments[dst_attachment_idx].image_view;

   const VkRect2D *ra = &cmd_buffer->state.render_area;

   VkImageResolve2 region = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2,
      .srcSubresource = {
         aspect,
         src_iview->vk.base_mip_level,
         src_iview->vk.base_array_layer,
         src_iview->vk.layer_count,
      },
      .srcOffset = { ra->offset.x, ra->offset.y, 0 },
      .dstSubresource =  {
         aspect,
         dst_iview->vk.base_mip_level,
         dst_iview->vk.base_array_layer,
         dst_iview->vk.layer_count,
      },
      .dstOffset = { ra->offset.x, ra->offset.y, 0 },
      .extent = { ra->extent.width, ra->extent.height, 1 },
   };

   struct v3dv_image *src_image = (struct v3dv_image *) src_iview->vk.image;
   struct v3dv_image *dst_image = (struct v3dv_image *) dst_iview->vk.image;
   VkResolveImageInfo2 resolve_info = {
      .sType = VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2,
      .srcImage = v3dv_image_to_handle(src_image),
      .srcImageLayout = VK_IMAGE_LAYOUT_GENERAL,
      .dstImage = v3dv_image_to_handle(dst_image),
      .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
      .regionCount = 1,
      .pRegions = &region,
   };

   VkCommandBuffer cmd_buffer_handle = v3dv_cmd_buffer_to_handle(cmd_buffer);
   v3dv_CmdResolveImage2(cmd_buffer_handle, &resolve_info);
}

static void
cmd_buffer_subpass_handle_pending_resolves(struct v3dv_cmd_buffer *cmd_buffer)
{
   assert(cmd_buffer->state.subpass_idx < cmd_buffer->state.pass->subpass_count);
   const struct v3dv_render_pass *pass = cmd_buffer->state.pass;
   const struct v3dv_subpass *subpass =
      &pass->subpasses[cmd_buffer->state.subpass_idx];

   if (!subpass->resolve_attachments)
      return;

   /* At this point we have already ended the current subpass and now we are
    * about to emit vkCmdResolveImage calls to get the resolves we can't handle
    * handle in the subpass RCL.
    *
    * vkCmdResolveImage is not supposed to be called inside a render pass so
    * before we call that we need to make sure our command buffer state reflects
    * that we are no longer in a subpass by finishing the current job and
    * resetting the framebuffer and render pass state temporarily and then
    * restoring it after we are done with the resolves.
    */
   if (cmd_buffer->state.job)
      v3dv_cmd_buffer_finish_job(cmd_buffer);
   struct v3dv_framebuffer *restore_fb = cmd_buffer->state.framebuffer;
   struct v3dv_render_pass *restore_pass = cmd_buffer->state.pass;
   uint32_t restore_subpass_idx = cmd_buffer->state.subpass_idx;
   cmd_buffer->state.framebuffer = NULL;
   cmd_buffer->state.pass = NULL;
   cmd_buffer->state.subpass_idx = -1;

   for (uint32_t i = 0; i < subpass->color_count; i++) {
      const uint32_t src_attachment_idx =
         subpass->color_attachments[i].attachment;
      if (src_attachment_idx == VK_ATTACHMENT_UNUSED)
         continue;

      /* Skip if this attachment doesn't have a resolve or if it was already
       * implemented as a TLB resolve.
       */
      if (!cmd_buffer->state.attachments[src_attachment_idx].has_resolve ||
          cmd_buffer->state.attachments[src_attachment_idx].use_tlb_resolve) {
         continue;
      }

      const uint32_t dst_attachment_idx =
         subpass->resolve_attachments[i].attachment;
      assert(dst_attachment_idx != VK_ATTACHMENT_UNUSED);

      cmd_buffer_emit_resolve(cmd_buffer, dst_attachment_idx, src_attachment_idx,
                              VK_IMAGE_ASPECT_COLOR_BIT);
   }

   const uint32_t ds_src_attachment_idx =
      subpass->ds_attachment.attachment;
   if (ds_src_attachment_idx != VK_ATTACHMENT_UNUSED &&
       cmd_buffer->state.attachments[ds_src_attachment_idx].has_resolve &&
       !cmd_buffer->state.attachments[ds_src_attachment_idx].use_tlb_resolve) {
      assert(subpass->resolve_depth || subpass->resolve_stencil);
      const VkImageAspectFlags ds_aspects =
         (subpass->resolve_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) |
         (subpass->resolve_stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
      const uint32_t ds_dst_attachment_idx =
         subpass->ds_resolve_attachment.attachment;
      assert(ds_dst_attachment_idx != VK_ATTACHMENT_UNUSED);
      cmd_buffer_emit_resolve(cmd_buffer, ds_dst_attachment_idx,
                              ds_src_attachment_idx, ds_aspects);
   }

   cmd_buffer->state.framebuffer = restore_fb;
   cmd_buffer->state.pass = restore_pass;
   cmd_buffer->state.subpass_idx = restore_subpass_idx;
}

static VkResult
cmd_buffer_begin_render_pass_secondary(
   struct v3dv_cmd_buffer *cmd_buffer,
   const VkCommandBufferInheritanceInfo *inheritance_info)
{
   assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
   assert(cmd_buffer->usage_flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);
   assert(inheritance_info);

   cmd_buffer->state.pass =
      v3dv_render_pass_from_handle(inheritance_info->renderPass);
   assert(cmd_buffer->state.pass);

   cmd_buffer->state.framebuffer =
      v3dv_framebuffer_from_handle(inheritance_info->framebuffer);

   assert(inheritance_info->subpass < cmd_buffer->state.pass->subpass_count);
   cmd_buffer->state.subpass_idx = inheritance_info->subpass;

   cmd_buffer->state.inheritance.occlusion_query_enable =
      inheritance_info->occlusionQueryEnable;

   /* Secondaries that execute inside a render pass won't start subpasses
    * so we want to create a job for them here.
    */
   struct v3dv_job *job =
      v3dv_cmd_buffer_start_job(cmd_buffer, inheritance_info->subpass,
                                V3DV_JOB_TYPE_GPU_CL_SECONDARY);
   if (!job) {
      v3dv_flag_oom(cmd_buffer, NULL);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   /* Secondary command buffers don't know about the render area, but our
    * scissor setup accounts for it, so let's make sure we make it large
    * enough that it doesn't actually constrain any rendering. This should
    * be fine, since the Vulkan spec states:
    *
    *    "The application must ensure (using scissor if necessary) that all
    *     rendering is contained within the render area."
    */
   const struct v3dv_framebuffer *framebuffer = cmd_buffer->state.framebuffer;
   cmd_buffer->state.render_area.offset.x = 0;
   cmd_buffer->state.render_area.offset.y = 0;
   cmd_buffer->state.render_area.extent.width =
      framebuffer ? framebuffer->width : V3D_MAX_IMAGE_DIMENSION;
   cmd_buffer->state.render_area.extent.height =
      framebuffer ? framebuffer->height : V3D_MAX_IMAGE_DIMENSION;

   /* We only really execute double-buffer mode in primary jobs, so allow this
    * mode in render pass secondaries to keep track of the double-buffer mode
    * score in them and update the primaries accordingly when they are executed
    * into them.
    */
    job->can_use_double_buffer = true;

   return VK_SUCCESS;
}

const struct vk_command_buffer_ops v3dv_cmd_buffer_ops = {
   .create = cmd_buffer_create,
   .reset = cmd_buffer_reset,
   .destroy = cmd_buffer_destroy,
};

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_BeginCommandBuffer(VkCommandBuffer commandBuffer,
                        const VkCommandBufferBeginInfo *pBeginInfo)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   /* If this is the first vkBeginCommandBuffer, we must initialize the
    * command buffer's state. Otherwise, we must reset its state. In both
    * cases we reset it.
    */
   cmd_buffer_reset(&cmd_buffer->vk, 0);

   assert(cmd_buffer->status == V3DV_CMD_BUFFER_STATUS_INITIALIZED);

   cmd_buffer->usage_flags = pBeginInfo->flags;

   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
      if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
         VkResult result =
            cmd_buffer_begin_render_pass_secondary(cmd_buffer,
                                                   pBeginInfo->pInheritanceInfo);
         if (result != VK_SUCCESS)
            return result;
      }
   }

   cmd_buffer->status = V3DV_CMD_BUFFER_STATUS_RECORDING;

   return VK_SUCCESS;
}

static void
cmd_buffer_update_tile_alignment(struct v3dv_cmd_buffer *cmd_buffer)
{
   /* Render areas and scissor/viewport are only relevant inside render passes,
    * otherwise we are dealing with transfer operations where these elements
    * don't apply.
    */
   assert(cmd_buffer->state.pass);
   const VkRect2D *rect = &cmd_buffer->state.render_area;

   /* We should only call this at the beginning of a subpass so we should
    * always have framebuffer information available.
    */
   assert(cmd_buffer->state.framebuffer);
   cmd_buffer->state.tile_aligned_render_area =
      v3dv_subpass_area_is_tile_aligned(cmd_buffer->device, rect,
                                        cmd_buffer->state.framebuffer,
                                        cmd_buffer->state.pass,
                                        cmd_buffer->state.subpass_idx);

   if (!cmd_buffer->state.tile_aligned_render_area) {
      perf_debug("Render area for subpass %d of render pass %p doesn't "
                 "match render pass granularity.\n",
                 cmd_buffer->state.subpass_idx, cmd_buffer->state.pass);
   }
}

static void
cmd_buffer_update_attachment_resolve_state(struct v3dv_cmd_buffer *cmd_buffer)
{
   /* NOTE: This should be called after cmd_buffer_update_tile_alignment()
    * since it relies on up-to-date information about subpass tile alignment.
    */
   const struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   const struct v3dv_render_pass *pass = state->pass;
   const struct v3dv_subpass *subpass = &pass->subpasses[state->subpass_idx];

   for (uint32_t i = 0; i < subpass->color_count; i++) {
      const uint32_t attachment_idx = subpass->color_attachments[i].attachment;
      if (attachment_idx == VK_ATTACHMENT_UNUSED)
         continue;

      state->attachments[attachment_idx].has_resolve =
         subpass->resolve_attachments &&
         subpass->resolve_attachments[i].attachment != VK_ATTACHMENT_UNUSED;

      state->attachments[attachment_idx].use_tlb_resolve =
         state->attachments[attachment_idx].has_resolve &&
         state->tile_aligned_render_area &&
         pass->attachments[attachment_idx].try_tlb_resolve;
   }

   uint32_t ds_attachment_idx = subpass->ds_attachment.attachment;
   if (ds_attachment_idx != VK_ATTACHMENT_UNUSED) {
      uint32_t ds_resolve_attachment_idx =
         subpass->ds_resolve_attachment.attachment;
      state->attachments[ds_attachment_idx].has_resolve =
         ds_resolve_attachment_idx != VK_ATTACHMENT_UNUSED;

      assert(!state->attachments[ds_attachment_idx].has_resolve ||
             (subpass->resolve_depth || subpass->resolve_stencil));

      state->attachments[ds_attachment_idx].use_tlb_resolve =
         state->attachments[ds_attachment_idx].has_resolve &&
         state->tile_aligned_render_area &&
         pass->attachments[ds_attachment_idx].try_tlb_resolve;
   }
}

static void
cmd_buffer_state_set_attachment_clear_color(struct v3dv_cmd_buffer *cmd_buffer,
                                            uint32_t attachment_idx,
                                            const VkClearColorValue *color)
{
   assert(attachment_idx < cmd_buffer->state.pass->attachment_count);
   const struct v3dv_render_pass_attachment *attachment =
      &cmd_buffer->state.pass->attachments[attachment_idx];

   uint32_t internal_type, internal_bpp;
   const struct v3dv_format *format =
      v3dv_X(cmd_buffer->device, get_format)(attachment->desc.format);
   /* We don't allow multi-planar formats for render pass attachments */
   assert(format->plane_count == 1);

   v3dv_X(cmd_buffer->device, get_internal_type_bpp_for_output_format)
      (format->planes[0].rt_type, &internal_type, &internal_bpp);

   uint32_t internal_size = 4 << internal_bpp;

   struct v3dv_cmd_buffer_attachment_state *attachment_state =
      &cmd_buffer->state.attachments[attachment_idx];

   v3dv_X(cmd_buffer->device, get_hw_clear_color)
      (color, internal_type, internal_size, &attachment_state->clear_value.color[0]);

   attachment_state->vk_clear_value.color = *color;
}

static void
cmd_buffer_state_set_attachment_clear_depth_stencil(
   struct v3dv_cmd_buffer *cmd_buffer,
   uint32_t attachment_idx,
   bool clear_depth, bool clear_stencil,
   const VkClearDepthStencilValue *ds)
{
   struct v3dv_cmd_buffer_attachment_state *attachment_state =
      &cmd_buffer->state.attachments[attachment_idx];

   if (clear_depth)
      attachment_state->clear_value.z = ds->depth;

   if (clear_stencil)
      attachment_state->clear_value.s = ds->stencil;

   attachment_state->vk_clear_value.depthStencil = *ds;
}

static void
cmd_buffer_state_set_clear_values(struct v3dv_cmd_buffer *cmd_buffer,
                                  uint32_t count, const VkClearValue *values)
{
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   const struct v3dv_render_pass *pass = state->pass;

   /* There could be less clear values than attachments in the render pass, in
    * which case we only want to process as many as we have, or there could be
    * more, in which case we want to ignore those for which we don't have a
    * corresponding attachment.
    */
   count = MIN2(count, pass->attachment_count);
   for (uint32_t i = 0; i < count; i++) {
      const struct v3dv_render_pass_attachment *attachment =
         &pass->attachments[i];

      if (attachment->desc.loadOp != VK_ATTACHMENT_LOAD_OP_CLEAR)
         continue;

      VkImageAspectFlags aspects = vk_format_aspects(attachment->desc.format);
      if (aspects & VK_IMAGE_ASPECT_COLOR_BIT) {
         cmd_buffer_state_set_attachment_clear_color(cmd_buffer, i,
                                                     &values[i].color);
      } else if (aspects & (VK_IMAGE_ASPECT_DEPTH_BIT |
                            VK_IMAGE_ASPECT_STENCIL_BIT)) {
         cmd_buffer_state_set_attachment_clear_depth_stencil(
            cmd_buffer, i,
            aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
            aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
            &values[i].depthStencil);
      }
   }
}

static void
cmd_buffer_state_set_attachments(struct v3dv_cmd_buffer *cmd_buffer,
                                 const VkRenderPassBeginInfo *pRenderPassBegin)
{
   V3DV_FROM_HANDLE(v3dv_render_pass, pass, pRenderPassBegin->renderPass);
   V3DV_FROM_HANDLE(v3dv_framebuffer, framebuffer, pRenderPassBegin->framebuffer);

   const VkRenderPassAttachmentBeginInfo *attach_begin =
      vk_find_struct_const(pRenderPassBegin, RENDER_PASS_ATTACHMENT_BEGIN_INFO);

   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;

   for (uint32_t i = 0; i < pass->attachment_count; i++) {
      if (attach_begin && attach_begin->attachmentCount != 0) {
         state->attachments[i].image_view =
            v3dv_image_view_from_handle(attach_begin->pAttachments[i]);
      } else if (framebuffer) {
         state->attachments[i].image_view = framebuffer->attachments[i];
      } else {
         assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
         state->attachments[i].image_view = NULL;
      }
   }
}

static void
cmd_buffer_init_render_pass_attachment_state(struct v3dv_cmd_buffer *cmd_buffer,
                                             const VkRenderPassBeginInfo *pRenderPassBegin)
{
   cmd_buffer_state_set_clear_values(cmd_buffer,
                                     pRenderPassBegin->clearValueCount,
                                     pRenderPassBegin->pClearValues);

   cmd_buffer_state_set_attachments(cmd_buffer, pRenderPassBegin);
}

static void
cmd_buffer_ensure_render_pass_attachment_state(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   const struct v3dv_render_pass *pass = state->pass;

   if (state->attachment_alloc_count < pass->attachment_count) {
      if (state->attachments > 0) {
         assert(state->attachment_alloc_count > 0);
         vk_free(&cmd_buffer->device->vk.alloc, state->attachments);
      }

      uint32_t size = sizeof(struct v3dv_cmd_buffer_attachment_state) *
                      pass->attachment_count;
      state->attachments = vk_zalloc(&cmd_buffer->device->vk.alloc, size, 8,
                                     VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
      if (!state->attachments) {
         v3dv_flag_oom(cmd_buffer, NULL);
         return;
      }
      state->attachment_alloc_count = pass->attachment_count;
   }

   assert(state->attachment_alloc_count >= pass->attachment_count);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                         const VkRenderPassBeginInfo *pRenderPassBegin,
                         const VkSubpassBeginInfo *pSubpassBeginInfo)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_render_pass, pass, pRenderPassBegin->renderPass);
   V3DV_FROM_HANDLE(v3dv_framebuffer, framebuffer, pRenderPassBegin->framebuffer);

   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   state->pass = pass;
   state->framebuffer = framebuffer;

   cmd_buffer_ensure_render_pass_attachment_state(cmd_buffer);
   v3dv_return_if_oom(cmd_buffer, NULL);

   cmd_buffer_init_render_pass_attachment_state(cmd_buffer, pRenderPassBegin);

   state->render_area = pRenderPassBegin->renderArea;

   /* If our render area is smaller than the current clip window we will have
    * to emit a new clip window to constraint it to the render area.
    */
   uint32_t min_render_x = state->render_area.offset.x;
   uint32_t min_render_y = state->render_area.offset.y;
   uint32_t max_render_x = min_render_x + state->render_area.extent.width - 1;
   uint32_t max_render_y = min_render_y + state->render_area.extent.height - 1;
   uint32_t min_clip_x = state->clip_window.offset.x;
   uint32_t min_clip_y = state->clip_window.offset.y;
   uint32_t max_clip_x = min_clip_x + state->clip_window.extent.width - 1;
   uint32_t max_clip_y = min_clip_y + state->clip_window.extent.height - 1;
   if (min_render_x > min_clip_x || min_render_y > min_clip_y ||
       max_render_x < max_clip_x || max_render_y < max_clip_y) {
      state->dirty |= V3DV_CMD_DIRTY_SCISSOR;
   }

   /* Setup for first subpass */
   v3dv_cmd_buffer_subpass_start(cmd_buffer, 0);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdNextSubpass2(VkCommandBuffer commandBuffer,
                     const VkSubpassBeginInfo *pSubpassBeginInfo,
                     const VkSubpassEndInfo *pSubpassEndInfo)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   assert(state->subpass_idx < state->pass->subpass_count - 1);

   /* Finish the previous subpass */
   v3dv_cmd_buffer_subpass_finish(cmd_buffer);
   cmd_buffer_subpass_handle_pending_resolves(cmd_buffer);

   /* Start the next subpass */
   v3dv_cmd_buffer_subpass_start(cmd_buffer, state->subpass_idx + 1);
}

static void
cmd_buffer_emit_subpass_clears(struct v3dv_cmd_buffer *cmd_buffer)
{
   assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   assert(cmd_buffer->state.pass);
   assert(cmd_buffer->state.subpass_idx < cmd_buffer->state.pass->subpass_count);
   const struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   const struct v3dv_render_pass *pass = state->pass;
   const struct v3dv_subpass *subpass = &pass->subpasses[state->subpass_idx];

   /* We only need to emit subpass clears as draw calls when the render
    * area is not aligned to tile boundaries or for GFXH-1461.
    */
   if (cmd_buffer->state.tile_aligned_render_area &&
       !subpass->do_depth_clear_with_draw &&
       !subpass->do_depth_clear_with_draw) {
      return;
   }

   uint32_t att_count = 0;
   VkClearAttachment atts[V3D_MAX_DRAW_BUFFERS + 1]; /* +1 for D/S */

   /* We only need to emit subpass clears as draw calls for color attachments
    * if the render area is not aligned to tile boundaries.
    */
   if (!cmd_buffer->state.tile_aligned_render_area) {
      for (uint32_t i = 0; i < subpass->color_count; i++) {
         const uint32_t att_idx = subpass->color_attachments[i].attachment;
         if (att_idx == VK_ATTACHMENT_UNUSED)
            continue;

         struct v3dv_render_pass_attachment *att = &pass->attachments[att_idx];
         if (att->desc.loadOp != VK_ATTACHMENT_LOAD_OP_CLEAR)
            continue;

         if (state->subpass_idx != att->first_subpass)
            continue;

         atts[att_count].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
         atts[att_count].colorAttachment = i;
         atts[att_count].clearValue = state->attachments[att_idx].vk_clear_value;
         att_count++;
      }
   }

   /* For D/S we may also need to emit a subpass clear for GFXH-1461 */
   const uint32_t ds_att_idx = subpass->ds_attachment.attachment;
   if (ds_att_idx != VK_ATTACHMENT_UNUSED) {
      struct v3dv_render_pass_attachment *att = &pass->attachments[ds_att_idx];
      if (state->subpass_idx == att->first_subpass) {
         VkImageAspectFlags aspects = vk_format_aspects(att->desc.format);
         if (att->desc.loadOp != VK_ATTACHMENT_LOAD_OP_CLEAR ||
             (cmd_buffer->state.tile_aligned_render_area &&
              !subpass->do_depth_clear_with_draw)) {
            aspects &= ~VK_IMAGE_ASPECT_DEPTH_BIT;
         }
         if (att->desc.stencilLoadOp != VK_ATTACHMENT_LOAD_OP_CLEAR ||
             (cmd_buffer->state.tile_aligned_render_area &&
              !subpass->do_stencil_clear_with_draw)) {
            aspects &= ~VK_IMAGE_ASPECT_STENCIL_BIT;
         }
         if (aspects) {
            atts[att_count].aspectMask = aspects;
            atts[att_count].colorAttachment = 0; /* Ignored */
            atts[att_count].clearValue =
               state->attachments[ds_att_idx].vk_clear_value;
            att_count++;
         }
      }
   }

   if (att_count == 0)
      return;

   if (!cmd_buffer->state.tile_aligned_render_area) {
      perf_debug("Render area doesn't match render pass granularity, falling "
                 "back to vkCmdClearAttachments for "
                 "VK_ATTACHMENT_LOAD_OP_CLEAR.\n");
   } else if (subpass->do_depth_clear_with_draw ||
              subpass->do_stencil_clear_with_draw) {
      perf_debug("Subpass clears DEPTH but loads STENCIL (or vice versa), "
                 "falling back to vkCmdClearAttachments for "
                 "VK_ATTACHMENT_LOAD_OP_CLEAR.\n");
   }

   /* From the Vulkan 1.0 spec:
    *
    *    "VK_ATTACHMENT_LOAD_OP_CLEAR specifies that the contents within the
    *     render area will be cleared to a uniform value, which is specified
    *     when a render pass instance is begun."
    *
    * So the clear is only constrained by the render area and not by pipeline
    * state such as scissor or viewport, these are the semantics of
    * vkCmdClearAttachments as well.
    */
   VkCommandBuffer _cmd_buffer = v3dv_cmd_buffer_to_handle(cmd_buffer);
   VkClearRect rect = {
      .rect = state->render_area,
      .baseArrayLayer = 0,
      .layerCount = 1,
   };
   v3dv_CmdClearAttachments(_cmd_buffer, att_count, atts, 1, &rect);
}

bool
v3dv_cmd_buffer_check_needs_load(const struct v3dv_cmd_buffer_state *state,
                                 VkImageAspectFlags aspect,
                                 uint32_t first_subpass_idx,
                                 VkAttachmentLoadOp load_op,
                                 uint32_t last_subpass_idx,
                                 VkAttachmentStoreOp store_op)
{
   /* We call this with image->vk.aspects & aspect, so 0 means the aspect we are
    * testing does not exist in the image.
    */
   if (!aspect)
      return false;

   /* Attachment (or view) load operations apply on the first subpass that
    * uses the attachment (or view), otherwise we always need to load.
    */
   if (state->job->first_subpass > first_subpass_idx)
      return true;

   /* If the job is continuing a subpass started in another job, we always
    * need to load.
    */
   if (state->job->is_subpass_continue)
      return true;

   /* If the area is not aligned to tile boundaries and we are going to store,
    * then we need to load to preserve contents outside the render area.
    */
   if (!state->tile_aligned_render_area &&
       v3dv_cmd_buffer_check_needs_store(state, aspect, last_subpass_idx,
                                         store_op)) {
      return true;
   }

   /* The attachment load operations must be LOAD */
   return load_op == VK_ATTACHMENT_LOAD_OP_LOAD;
}

bool
v3dv_cmd_buffer_check_needs_store(const struct v3dv_cmd_buffer_state *state,
                                  VkImageAspectFlags aspect,
                                  uint32_t last_subpass_idx,
                                  VkAttachmentStoreOp store_op)
{
   /* We call this with image->vk.aspects & aspect, so 0 means the aspect we are
    * testing does not exist in the image.
    */
   if (!aspect)
      return false;

   /* Attachment (or view) store operations only apply on the last subpass
    * where the attachment (or view)  is used, in other subpasses we always
    * need to store.
    */
   if (state->subpass_idx < last_subpass_idx)
      return true;

   /* Attachment store operations only apply on the last job we emit on the the
    * last subpass where the attachment is used, otherwise we always need to
    * store.
    */
   if (!state->job->is_subpass_finish)
      return true;

   /* The attachment store operation must be STORE */
   return store_op == VK_ATTACHMENT_STORE_OP_STORE;
}

static void
cmd_buffer_subpass_check_double_buffer_mode(struct v3dv_cmd_buffer *cmd_buffer,
                                            bool msaa)
{
   const struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   job->can_use_double_buffer = false;

   /* Double-buffer can only be used if requested via V3D_DEBUG */
   if (!V3D_DBG(DOUBLE_BUFFER))
      return;

   /* Double-buffer cannot be enabled for MSAA jobs */
   if (msaa)
      return;

   const struct v3dv_render_pass *pass = state->pass;
   const struct v3dv_subpass *subpass = &pass->subpasses[state->subpass_idx];

   /* FIXME: For now we discard multiview jobs (which have an implicit geometry
    * shader) for this optimization. If we want to enable this with multiview
    * we would need to check if any view (layer) in any attachment used by the
    * job has loads and/or stores as we do below for regular attachments. Also,
    * we would want to have a heuristic that doesn't automatically disable
    * double-buffer in the presence of geometry shaders.
    */
   if (state->pass->multiview_enabled)
      return;

   /* Tile loads are serialized against stores, in which case we don't get
    * any benefits from enabling double-buffer and would just pay the price
    * of a smaller tile size instead. Similarly, we only benefit from
    * double-buffer if we have tile stores, as the point of this mode is
    * to execute rendering of a new tile while we store the previous one to
    * hide latency on the tile store operation.
    */
   bool has_stores = false;
   for (uint32_t i = 0; i < subpass->color_count; i++) {
      uint32_t attachment_idx = subpass->color_attachments[i].attachment;
      if (attachment_idx == VK_ATTACHMENT_UNUSED)
         continue;

      const struct v3dv_render_pass_attachment *attachment =
         &state->pass->attachments[attachment_idx];

      /* FIXME: This will check 'tile_aligned_render_area' but that was
       * computed with a tile size without double-buffer. That is okay
       * because if the larger tile size is aligned then we know the smaller
       * tile size for double-buffer will be as well. However, we might
       * still benefit from doing this check with the smaller tile size
       * because it can happen that the smaller size is aligned and the
       * larger size is not.
       */
      if (v3dv_cmd_buffer_check_needs_load(state,
                                           VK_IMAGE_ASPECT_COLOR_BIT,
                                           attachment->first_subpass,
                                           attachment->desc.loadOp,
                                           attachment->last_subpass,
                                           attachment->desc.storeOp)) {
         return;
      }

      if (v3dv_cmd_buffer_check_needs_store(state,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            attachment->last_subpass,
                                            attachment->desc.storeOp)) {
         has_stores = true;
      }
   }

   if (subpass->ds_attachment.attachment != VK_ATTACHMENT_UNUSED) {
      uint32_t ds_attachment_idx = subpass->ds_attachment.attachment;
      const struct v3dv_render_pass_attachment *ds_attachment =
         &state->pass->attachments[ds_attachment_idx];

      const VkImageAspectFlags ds_aspects =
         vk_format_aspects(ds_attachment->desc.format);

      if (v3dv_cmd_buffer_check_needs_load(state,
                                           ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                                           ds_attachment->first_subpass,
                                           ds_attachment->desc.loadOp,
                                           ds_attachment->last_subpass,
                                           ds_attachment->desc.storeOp)) {
         return;
      }

      if (v3dv_cmd_buffer_check_needs_load(state,
                                           ds_aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
                                           ds_attachment->first_subpass,
                                           ds_attachment->desc.stencilLoadOp,
                                           ds_attachment->last_subpass,
                                           ds_attachment->desc.stencilStoreOp)) {
         return;
      }

      has_stores |= v3dv_cmd_buffer_check_needs_store(state,
                                                      ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT,
                                                      ds_attachment->last_subpass,
                                                      ds_attachment->desc.storeOp);
      has_stores |= v3dv_cmd_buffer_check_needs_store(state,
                                                      ds_aspects & VK_IMAGE_ASPECT_STENCIL_BIT,
                                                      ds_attachment->last_subpass,
                                                      ds_attachment->desc.stencilStoreOp);
   }

   job->can_use_double_buffer = has_stores;
}

static struct v3dv_job *
cmd_buffer_subpass_create_job(struct v3dv_cmd_buffer *cmd_buffer,
                              uint32_t subpass_idx,
                              enum v3dv_job_type type)
{
   assert(type == V3DV_JOB_TYPE_GPU_CL ||
          type == V3DV_JOB_TYPE_GPU_CL_SECONDARY);

   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   assert(subpass_idx < state->pass->subpass_count);

   /* Starting a new job can trigger a finish of the current one, so don't
    * change the command buffer state for the new job until we are done creating
    * the new job.
    */
   struct v3dv_job *job =
      v3dv_cmd_buffer_start_job(cmd_buffer, subpass_idx, type);
   if (!job)
      return NULL;

   state->subpass_idx = subpass_idx;

   /* If we are starting a new job we need to setup binning. We only do this
    * for V3DV_JOB_TYPE_GPU_CL jobs because V3DV_JOB_TYPE_GPU_CL_SECONDARY
    * jobs are not submitted to the GPU directly, and are instead meant to be
    * branched to from other V3DV_JOB_TYPE_GPU_CL jobs.
    */
   if (type == V3DV_JOB_TYPE_GPU_CL &&
       job->first_subpass == state->subpass_idx) {
      const struct v3dv_subpass *subpass =
         &state->pass->subpasses[state->subpass_idx];

      const struct v3dv_framebuffer *framebuffer = state->framebuffer;

      uint8_t max_internal_bpp, total_color_bpp;
      bool msaa;
      v3dv_X(job->device, framebuffer_compute_internal_bpp_msaa)
         (framebuffer, state->attachments, subpass,
          &max_internal_bpp, &total_color_bpp, &msaa);

      /* From the Vulkan spec:
       *
       *    "If the render pass uses multiview, then layers must be one and
       *     each attachment requires a number of layers that is greater than
       *     the maximum bit index set in the view mask in the subpasses in
       *     which it is used."
       *
       * So when multiview is enabled, we take the number of layers from the
       * last bit set in the view mask.
       */
      uint32_t layers = framebuffer->layers;
      if (subpass->view_mask != 0) {
         assert(framebuffer->layers == 1);
         layers = util_last_bit(subpass->view_mask);
      }

      v3dv_job_start_frame(job,
                           framebuffer->width,
                           framebuffer->height,
                           layers,
                           true, false,
                           subpass->color_count,
                           max_internal_bpp,
                           total_color_bpp,
                           msaa);
   }

   return job;
}

struct v3dv_job *
v3dv_cmd_buffer_subpass_start(struct v3dv_cmd_buffer *cmd_buffer,
                              uint32_t subpass_idx)
{
   assert(cmd_buffer->state.pass);
   assert(subpass_idx < cmd_buffer->state.pass->subpass_count);

   struct v3dv_job *job =
      cmd_buffer_subpass_create_job(cmd_buffer, subpass_idx,
                                    V3DV_JOB_TYPE_GPU_CL);
   if (!job)
      return NULL;

   /* Check if our render area is aligned to tile boundaries. We have to do
    * this in each subpass because the subset of attachments used can change
    * and with that the tile size selected by the hardware can change too.
    */
   cmd_buffer_update_tile_alignment(cmd_buffer);

   /* Decide if we can use double-buffer for this subpass job */
   cmd_buffer_subpass_check_double_buffer_mode(cmd_buffer, job->frame_tiling.msaa);

   cmd_buffer_update_attachment_resolve_state(cmd_buffer);

   /* If we can't use TLB clears then we need to emit draw clears for any
    * LOAD_OP_CLEAR attachments in this subpass now. We might also need to emit
    * Depth/Stencil clears if we hit GFXH-1461.
    *
    * Secondary command buffers don't start subpasses (and may not even have
    * framebuffer state), so we only care about this in primaries. The only
    * exception could be a secondary running inside a subpass that needs to
    * record a meta operation (with its own render pass) that relies on
    * attachment load clears, but we don't have any instances of that right
    * now.
    */
   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
      cmd_buffer_emit_subpass_clears(cmd_buffer);

   return job;
}

struct v3dv_job *
v3dv_cmd_buffer_subpass_resume(struct v3dv_cmd_buffer *cmd_buffer,
                               uint32_t subpass_idx)
{
   assert(cmd_buffer->state.pass);
   assert(subpass_idx < cmd_buffer->state.pass->subpass_count);

   struct v3dv_job *job;
   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
      job = cmd_buffer_subpass_create_job(cmd_buffer, subpass_idx,
                                          V3DV_JOB_TYPE_GPU_CL);
   } else {
      assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
      job = cmd_buffer_subpass_create_job(cmd_buffer, subpass_idx,
                                          V3DV_JOB_TYPE_GPU_CL_SECONDARY);
   }

   if (!job)
      return NULL;

   job->is_subpass_continue = true;

   return job;
}

void
v3dv_cmd_buffer_subpass_finish(struct v3dv_cmd_buffer *cmd_buffer)
{
   /* We can end up here without a job if the last command recorded into the
    * subpass already finished the job (for example a pipeline barrier). In
    * that case we miss to set the is_subpass_finish flag, but that is not
    * required for proper behavior.
    */
   struct v3dv_job *job = cmd_buffer->state.job;
   if (job)
      job->is_subpass_finish = true;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdEndRenderPass2(VkCommandBuffer commandBuffer,
                       const VkSubpassEndInfo *pSubpassEndInfo)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   /* Finalize last subpass */
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   assert(state->subpass_idx == state->pass->subpass_count - 1);
   v3dv_cmd_buffer_subpass_finish(cmd_buffer);
   v3dv_cmd_buffer_finish_job(cmd_buffer);

   cmd_buffer_subpass_handle_pending_resolves(cmd_buffer);

   /* We are no longer inside a render pass */
   state->framebuffer = NULL;
   state->pass = NULL;
   state->subpass_idx = -1;
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_EndCommandBuffer(VkCommandBuffer commandBuffer)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   if (cmd_buffer->state.oom)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   /* Primaries should have ended any recording jobs by the time they hit
    * vkEndRenderPass (if we are inside a render pass). Commands outside
    * a render pass instance (for both primaries and secondaries) spawn
    * complete jobs too. So the only case where we can get here without
    * finishing a recording job is when we are recording a secondary
    * inside a render pass.
    */
   if (cmd_buffer->state.job) {
      assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY &&
             cmd_buffer->state.pass);
      v3dv_cmd_buffer_finish_job(cmd_buffer);
   }

   cmd_buffer->status = V3DV_CMD_BUFFER_STATUS_EXECUTABLE;

   return VK_SUCCESS;
}

static void
clone_bo_list(struct v3dv_cmd_buffer *cmd_buffer,
              struct list_head *dst,
              struct list_head *src)
{
   assert(cmd_buffer);

   list_inithead(dst);
   list_for_each_entry(struct v3dv_bo, bo, src, list_link) {
      struct v3dv_bo *clone_bo =
         vk_alloc(&cmd_buffer->device->vk.alloc, sizeof(struct v3dv_bo), 8,
                  VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
      if (!clone_bo) {
         v3dv_flag_oom(cmd_buffer, NULL);
         return;
      }

      *clone_bo = *bo;
      list_addtail(&clone_bo->list_link, dst);
   }
}

/* Clones a job for inclusion in the given command buffer. Note that this
 * doesn't make a deep copy so the cloned job it doesn't own any resources.
 * Useful when we need to have a job in more than one list, which happens
 * for jobs recorded in secondary command buffers when we want to execute
 * them in primaries.
 */
struct v3dv_job *
v3dv_job_clone_in_cmd_buffer(struct v3dv_job *job,
                             struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *clone_job = vk_alloc(&job->device->vk.alloc,
                                         sizeof(struct v3dv_job), 8,
                                         VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!clone_job) {
      v3dv_flag_oom(cmd_buffer, NULL);
      return NULL;
   }

   /* Cloned jobs don't duplicate resources! */
   *clone_job = *job;
   clone_job->is_clone = true;
   clone_job->cmd_buffer = cmd_buffer;
   list_addtail(&clone_job->list_link, &cmd_buffer->jobs);

   /* We need to regen the BO lists so that they point to the BO list in the
    * cloned job. Otherwise functions like list_length() will loop forever.
    */
   if (job->type == V3DV_JOB_TYPE_GPU_CL) {
      clone_bo_list(cmd_buffer, &clone_job->bcl.bo_list, &job->bcl.bo_list);
      clone_bo_list(cmd_buffer, &clone_job->rcl.bo_list, &job->rcl.bo_list);
      clone_bo_list(cmd_buffer, &clone_job->indirect.bo_list,
                    &job->indirect.bo_list);
   }

   return clone_job;
}

void
v3dv_cmd_buffer_merge_barrier_state(struct v3dv_barrier_state *dst,
                                    struct v3dv_barrier_state *src)
{
   dst->dst_mask |= src->dst_mask;

   dst->src_mask_graphics |= src->src_mask_graphics;
   dst->src_mask_compute  |= src->src_mask_compute;
   dst->src_mask_transfer |= src->src_mask_transfer;

   dst->bcl_buffer_access |= src->bcl_buffer_access;
   dst->bcl_image_access  |= src->bcl_image_access;
}

static void
cmd_buffer_execute_outside_pass(struct v3dv_cmd_buffer *primary,
                                uint32_t cmd_buffer_count,
                                const VkCommandBuffer *cmd_buffers)
{
   struct v3dv_barrier_state pending_barrier = { 0 };
   for (uint32_t i = 0; i < cmd_buffer_count; i++) {
      V3DV_FROM_HANDLE(v3dv_cmd_buffer, secondary, cmd_buffers[i]);

      assert(!(secondary->usage_flags &
               VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT));

      /* Secondary command buffers that execute outside a render pass create
       * complete jobs with an RCL and tile setup, so we simply want to merge
       * their job list into the primary's. However, because they may be
       * executed into multiple primaries at the same time and we only have a
       * single list_link in each job, we can't just add then to the primary's
       * job list and we instead have to clone them first.
       *
       * Alternatively, we could create a "execute secondary" CPU job that
       * when executed in a queue, would submit all the jobs in the referenced
       * secondary command buffer. However, this would raise some challenges
       * to make it work with the implementation of wait threads in the queue
       * which we use for event waits, for example.
       */
      list_for_each_entry(struct v3dv_job, secondary_job,
                          &secondary->jobs, list_link) {
         /* These can only happen inside a render pass */
         assert(secondary_job->type != V3DV_JOB_TYPE_GPU_CL_SECONDARY);
         struct v3dv_job *job = v3dv_job_clone_in_cmd_buffer(secondary_job, primary);
         if (!job)
            return;

         if (pending_barrier.dst_mask) {
            /* FIXME: do the same we do for primaries and only choose the
             * relevant src masks.
             */
            job->serialize = pending_barrier.src_mask_graphics |
                             pending_barrier.src_mask_transfer |
                             pending_barrier.src_mask_compute;
            if (pending_barrier.bcl_buffer_access ||
                pending_barrier.bcl_image_access) {
               job->needs_bcl_sync = true;
            }
            memset(&pending_barrier, 0, sizeof(pending_barrier));
         }
      }

      /* If this secondary had any pending barrier state we will need that
       * barrier state consumed with whatever comes after it (first job in
       * the next secondary or the primary, if this was the last secondary).
       */
      assert(secondary->state.barrier.dst_mask ||
             (!secondary->state.barrier.bcl_buffer_access &&
              !secondary->state.barrier.bcl_image_access));
      pending_barrier = secondary->state.barrier;
   }

   if (pending_barrier.dst_mask) {
      v3dv_cmd_buffer_merge_barrier_state(&primary->state.barrier,
                                          &pending_barrier);
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdExecuteCommands(VkCommandBuffer commandBuffer,
                        uint32_t commandBufferCount,
                        const VkCommandBuffer *pCommandBuffers)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, primary, commandBuffer);

   if (primary->state.pass != NULL) {
      v3dv_X(primary->device, cmd_buffer_execute_inside_pass)
         (primary, commandBufferCount, pCommandBuffers);
   } else {
      cmd_buffer_execute_outside_pass(primary,
                                      commandBufferCount, pCommandBuffers);
   }
}

/* This goes though the list of possible dynamic states in the pipeline and,
 * for those that are not configured as dynamic, copies relevant state into
 * the command buffer.
 */
static void
cmd_buffer_bind_pipeline_static_state(struct v3dv_cmd_buffer *cmd_buffer,
                                      const struct v3dv_dynamic_state *src)
{
   struct v3dv_dynamic_state *dest = &cmd_buffer->state.dynamic;
   uint32_t dynamic_mask = src->mask;
   uint32_t dirty = 0;

   if (!(dynamic_mask & V3DV_DYNAMIC_VIEWPORT)) {
      dest->viewport.count = src->viewport.count;
      if (memcmp(&dest->viewport.viewports, &src->viewport.viewports,
                 src->viewport.count * sizeof(VkViewport))) {
         typed_memcpy(dest->viewport.viewports,
                      src->viewport.viewports,
                      src->viewport.count);
         typed_memcpy(dest->viewport.scale, src->viewport.scale,
                      src->viewport.count);
         typed_memcpy(dest->viewport.translate, src->viewport.translate,
                      src->viewport.count);
         dirty |= V3DV_CMD_DIRTY_VIEWPORT;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_SCISSOR)) {
      dest->scissor.count = src->scissor.count;
      if (memcmp(&dest->scissor.scissors, &src->scissor.scissors,
                 src->scissor.count * sizeof(VkRect2D))) {
         typed_memcpy(dest->scissor.scissors,
                      src->scissor.scissors, src->scissor.count);
         dirty |= V3DV_CMD_DIRTY_SCISSOR;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_STENCIL_COMPARE_MASK)) {
      if (memcmp(&dest->stencil_compare_mask, &src->stencil_compare_mask,
                 sizeof(src->stencil_compare_mask))) {
         dest->stencil_compare_mask = src->stencil_compare_mask;
         dirty |= V3DV_CMD_DIRTY_STENCIL_COMPARE_MASK;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_STENCIL_WRITE_MASK)) {
      if (memcmp(&dest->stencil_write_mask, &src->stencil_write_mask,
                 sizeof(src->stencil_write_mask))) {
         dest->stencil_write_mask = src->stencil_write_mask;
         dirty |= V3DV_CMD_DIRTY_STENCIL_WRITE_MASK;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_STENCIL_REFERENCE)) {
      if (memcmp(&dest->stencil_reference, &src->stencil_reference,
                 sizeof(src->stencil_reference))) {
         dest->stencil_reference = src->stencil_reference;
         dirty |= V3DV_CMD_DIRTY_STENCIL_REFERENCE;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_BLEND_CONSTANTS)) {
      if (memcmp(dest->blend_constants, src->blend_constants,
                 sizeof(src->blend_constants))) {
         memcpy(dest->blend_constants, src->blend_constants,
                sizeof(src->blend_constants));
         dirty |= V3DV_CMD_DIRTY_BLEND_CONSTANTS;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_DEPTH_BIAS)) {
      if (memcmp(&dest->depth_bias, &src->depth_bias,
                 sizeof(src->depth_bias))) {
         memcpy(&dest->depth_bias, &src->depth_bias, sizeof(src->depth_bias));
         dirty |= V3DV_CMD_DIRTY_DEPTH_BIAS;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_DEPTH_BOUNDS)) {
      if (memcmp(&dest->depth_bounds, &src->depth_bounds,
                 sizeof(src->depth_bounds))) {
         memcpy(&dest->depth_bounds, &src->depth_bounds, sizeof(src->depth_bounds));
         dirty |= V3DV_CMD_DIRTY_DEPTH_BOUNDS;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_LINE_WIDTH)) {
      if (dest->line_width != src->line_width) {
         dest->line_width = src->line_width;
         dirty |= V3DV_CMD_DIRTY_LINE_WIDTH;
      }
   }

   if (!(dynamic_mask & V3DV_DYNAMIC_COLOR_WRITE_ENABLE)) {
      if (dest->color_write_enable != src->color_write_enable) {
         dest->color_write_enable = src->color_write_enable;
         dirty |= V3DV_CMD_DIRTY_COLOR_WRITE_ENABLE;
      }
   }

   cmd_buffer->state.dynamic.mask = dynamic_mask;
   cmd_buffer->state.dirty |= dirty;
}

static void
bind_graphics_pipeline(struct v3dv_cmd_buffer *cmd_buffer,
                       struct v3dv_pipeline *pipeline)
{
   assert(pipeline && !(pipeline->active_stages & VK_SHADER_STAGE_COMPUTE_BIT));
   if (cmd_buffer->state.gfx.pipeline == pipeline)
      return;

   cmd_buffer->state.gfx.pipeline = pipeline;

   cmd_buffer_bind_pipeline_static_state(cmd_buffer, &pipeline->dynamic_state);

   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_PIPELINE;
}

static void
bind_compute_pipeline(struct v3dv_cmd_buffer *cmd_buffer,
                      struct v3dv_pipeline *pipeline)
{
   assert(pipeline && pipeline->active_stages == VK_SHADER_STAGE_COMPUTE_BIT);

   if (cmd_buffer->state.compute.pipeline == pipeline)
      return;

   cmd_buffer->state.compute.pipeline = pipeline;
   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_COMPUTE_PIPELINE;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdBindPipeline(VkCommandBuffer commandBuffer,
                     VkPipelineBindPoint pipelineBindPoint,
                     VkPipeline _pipeline)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_pipeline, pipeline, _pipeline);

   switch (pipelineBindPoint) {
   case VK_PIPELINE_BIND_POINT_COMPUTE:
      bind_compute_pipeline(cmd_buffer, pipeline);
      break;

   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      bind_graphics_pipeline(cmd_buffer, pipeline);
      break;

   default:
      assert(!"invalid bind point");
      break;
   }
}

/* Considers the pipeline's negative_one_to_one state and applies it to the
 * current viewport transform if needed to produce the resulting Z translate
 * and scale parameters.
 */
void
v3dv_cmd_buffer_state_get_viewport_z_xform(struct v3dv_cmd_buffer_state *state,
                                           uint32_t vp_idx,
                                           float *translate_z, float *scale_z)
{
   const struct v3dv_viewport_state *vp_state = &state->dynamic.viewport;

   float t = vp_state->translate[vp_idx][2];
   float s = vp_state->scale[vp_idx][2];

   assert(state->gfx.pipeline);
   if (state->gfx.pipeline->negative_one_to_one) {
      t = (t + vp_state->viewports[vp_idx].maxDepth) * 0.5f;
      s *= 0.5f;
   }

   if (translate_z)
      *translate_z = t;

   if (scale_z)
      *scale_z = s;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetViewport(VkCommandBuffer commandBuffer,
                    uint32_t firstViewport,
                    uint32_t viewportCount,
                    const VkViewport *pViewports)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   const uint32_t total_count = firstViewport + viewportCount;

   assert(firstViewport < MAX_VIEWPORTS);
   assert(total_count >= 1 && total_count <= MAX_VIEWPORTS);

   if (state->dynamic.viewport.count < total_count)
      state->dynamic.viewport.count = total_count;

   if (!memcmp(state->dynamic.viewport.viewports + firstViewport,
               pViewports, viewportCount * sizeof(*pViewports))) {
      return;
   }

   memcpy(state->dynamic.viewport.viewports + firstViewport, pViewports,
          viewportCount * sizeof(*pViewports));

   for (uint32_t i = firstViewport; i < total_count; i++) {
      v3dv_X(cmd_buffer->device, viewport_compute_xform)
         (&state->dynamic.viewport.viewports[i],
          state->dynamic.viewport.scale[i],
          state->dynamic.viewport.translate[i]);
   }

   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_VIEWPORT;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetScissor(VkCommandBuffer commandBuffer,
                   uint32_t firstScissor,
                   uint32_t scissorCount,
                   const VkRect2D *pScissors)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;

   assert(firstScissor < MAX_SCISSORS);
   assert(firstScissor + scissorCount >= 1 &&
          firstScissor + scissorCount <= MAX_SCISSORS);

   if (state->dynamic.scissor.count < firstScissor + scissorCount)
      state->dynamic.scissor.count = firstScissor + scissorCount;

   if (!memcmp(state->dynamic.scissor.scissors + firstScissor,
               pScissors, scissorCount * sizeof(*pScissors))) {
      return;
   }

   memcpy(state->dynamic.scissor.scissors + firstScissor, pScissors,
          scissorCount * sizeof(*pScissors));

   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_SCISSOR;
}

static void
emit_scissor(struct v3dv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->state.dynamic.viewport.count == 0)
      return;

   struct v3dv_dynamic_state *dynamic = &cmd_buffer->state.dynamic;

   /* FIXME: right now we only support one viewport. viewporst[0] would work
    * now, but would need to change if we allow multiple viewports.
    */
   float *vptranslate = dynamic->viewport.translate[0];
   float *vpscale = dynamic->viewport.scale[0];
   assert(vpscale[0] >= 0);

   float vp_minx = vptranslate[0] - vpscale[0];
   float vp_maxx = vptranslate[0] + vpscale[0];

   /* With KHR_maintenance1 viewport may have negative Y */
   float vp_miny = vptranslate[1] - fabsf(vpscale[1]);
   float vp_maxy = vptranslate[1] + fabsf(vpscale[1]);

   /* Quoting from v3dx_emit:
    * "Clip to the scissor if it's enabled, but still clip to the
    * drawable regardless since that controls where the binner
    * tries to put things.
    *
    * Additionally, always clip the rendering to the viewport,
    * since the hardware does guardband clipping, meaning
    * primitives would rasterize outside of the view volume."
    */
   uint32_t minx, miny, maxx, maxy;

   /* From the Vulkan spec:
    *
    * "The application must ensure (using scissor if necessary) that all
    *  rendering is contained within the render area. The render area must be
    *  contained within the framebuffer dimensions."
    *
    * So it is the application's responsibility to ensure this. Still, we can
    * help by automatically restricting the scissor rect to the render area.
    */
   minx = MAX2(vp_minx, cmd_buffer->state.render_area.offset.x);
   miny = MAX2(vp_miny, cmd_buffer->state.render_area.offset.y);
   maxx = MIN2(vp_maxx, cmd_buffer->state.render_area.offset.x +
                        cmd_buffer->state.render_area.extent.width);
   maxy = MIN2(vp_maxy, cmd_buffer->state.render_area.offset.y +
                        cmd_buffer->state.render_area.extent.height);

   /* Clip against user provided scissor if needed.
    *
    * FIXME: right now we only allow one scissor. Below would need to be
    * updated if we support more
    */
   if (dynamic->scissor.count > 0) {
      VkRect2D *scissor = &dynamic->scissor.scissors[0];
      minx = MAX2(minx, scissor->offset.x);
      miny = MAX2(miny, scissor->offset.y);
      maxx = MIN2(maxx, scissor->offset.x + scissor->extent.width);
      maxy = MIN2(maxy, scissor->offset.y + scissor->extent.height);
   }

   /* If the scissor is outside the viewport area we end up with
    * min{x,y} > max{x,y}.
    */
   if (minx > maxx)
      maxx = minx;
   if (miny > maxy)
      maxy = miny;

   cmd_buffer->state.clip_window.offset.x = minx;
   cmd_buffer->state.clip_window.offset.y = miny;
   cmd_buffer->state.clip_window.extent.width = maxx - minx;
   cmd_buffer->state.clip_window.extent.height = maxy - miny;

   v3dv_X(cmd_buffer->device, job_emit_clip_window)
      (cmd_buffer->state.job, &cmd_buffer->state.clip_window);

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_SCISSOR;
}

static void
update_gfx_uniform_state(struct v3dv_cmd_buffer *cmd_buffer,
                         uint32_t dirty_uniform_state)
{
   /* We need to update uniform streams if any piece of state that is passed
    * to the shader as a uniform may have changed.
    *
    * If only descriptor sets are dirty then we can safely ignore updates
    * for shader stages that don't access descriptors.
    */

   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   assert(pipeline);

   const bool has_new_pipeline = dirty_uniform_state & V3DV_CMD_DIRTY_PIPELINE;
   const bool has_new_viewport = dirty_uniform_state & V3DV_CMD_DIRTY_VIEWPORT;
   const bool has_new_push_constants = dirty_uniform_state & V3DV_CMD_DIRTY_PUSH_CONSTANTS;
   const bool has_new_descriptors = dirty_uniform_state & V3DV_CMD_DIRTY_DESCRIPTOR_SETS;
   const bool has_new_view_index = dirty_uniform_state & V3DV_CMD_DIRTY_VIEW_INDEX;
   const bool has_new_draw_id = dirty_uniform_state & V3DV_CMD_DIRTY_DRAW_ID;

   /* VK_SHADER_STAGE_FRAGMENT_BIT */
   const bool has_new_descriptors_fs =
      has_new_descriptors &&
      (cmd_buffer->state.dirty_descriptor_stages & VK_SHADER_STAGE_FRAGMENT_BIT);

   const bool has_new_push_constants_fs =
      has_new_push_constants &&
      (cmd_buffer->state.dirty_push_constants_stages & VK_SHADER_STAGE_FRAGMENT_BIT);

   const bool needs_fs_update = has_new_pipeline ||
                                has_new_view_index ||
                                has_new_push_constants_fs ||
                                has_new_descriptors_fs;

   if (needs_fs_update) {
      struct v3dv_shader_variant *fs_variant =
         pipeline->shared_data->variants[BROADCOM_SHADER_FRAGMENT];

      cmd_buffer->state.uniforms.fs =
         v3dv_write_uniforms(cmd_buffer, pipeline, fs_variant);
   }

   /* VK_SHADER_STAGE_GEOMETRY_BIT */
   if (pipeline->has_gs) {
      const bool has_new_descriptors_gs =
         has_new_descriptors &&
         (cmd_buffer->state.dirty_descriptor_stages &
          VK_SHADER_STAGE_GEOMETRY_BIT);

      const bool has_new_push_constants_gs =
         has_new_push_constants &&
         (cmd_buffer->state.dirty_push_constants_stages &
          VK_SHADER_STAGE_GEOMETRY_BIT);

      const bool needs_gs_update = has_new_viewport ||
                                   has_new_view_index ||
                                   has_new_pipeline ||
                                   has_new_push_constants_gs ||
                                   has_new_descriptors_gs;

      if (needs_gs_update) {
         struct v3dv_shader_variant *gs_variant =
            pipeline->shared_data->variants[BROADCOM_SHADER_GEOMETRY];

          struct v3dv_shader_variant *gs_bin_variant =
            pipeline->shared_data->variants[BROADCOM_SHADER_GEOMETRY_BIN];

         cmd_buffer->state.uniforms.gs =
            v3dv_write_uniforms(cmd_buffer, pipeline, gs_variant);

         cmd_buffer->state.uniforms.gs_bin =
            v3dv_write_uniforms(cmd_buffer, pipeline, gs_bin_variant);
      }
   }

   /* VK_SHADER_STAGE_VERTEX_BIT */
   const bool has_new_descriptors_vs =
      has_new_descriptors &&
      (cmd_buffer->state.dirty_descriptor_stages & VK_SHADER_STAGE_VERTEX_BIT);

   const bool has_new_push_constants_vs =
      has_new_push_constants &&
      (cmd_buffer->state.dirty_push_constants_stages & VK_SHADER_STAGE_VERTEX_BIT);

   const bool needs_vs_update = has_new_viewport ||
                                has_new_view_index ||
                                has_new_draw_id ||
                                has_new_pipeline ||
                                has_new_push_constants_vs ||
                                has_new_descriptors_vs;

   if (needs_vs_update) {
      struct v3dv_shader_variant *vs_variant =
         pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX];

       struct v3dv_shader_variant *vs_bin_variant =
         pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX_BIN];

      cmd_buffer->state.uniforms.vs =
         v3dv_write_uniforms(cmd_buffer, pipeline, vs_variant);

      cmd_buffer->state.uniforms.vs_bin =
         v3dv_write_uniforms(cmd_buffer, pipeline, vs_bin_variant);
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_VIEW_INDEX;
   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_DRAW_ID;
}

/* This stores command buffer state that we might be about to stomp for
 * a meta operation.
 */
void
v3dv_cmd_buffer_meta_state_push(struct v3dv_cmd_buffer *cmd_buffer,
                                bool push_descriptor_state)
{
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;

   /* Attachment state.
    *
    * We store this state even if we are not currently in a subpass
    * (subpass_idx != -1) because we may get here to implement subpass
    * resolves via vkCmdResolveImage from
    * cmd_buffer_subpass_handle_pending_resolves. In that scenario we pretend
    * we are no longer in a subpass because Vulkan disallows image resolves
    * via vkCmdResolveImage during subpasses, but we still need to preserve
    * attachment state because we may have more subpasses to go through
    * after processing resolves in the current subass.
    */
   const uint32_t attachment_state_item_size =
      sizeof(struct v3dv_cmd_buffer_attachment_state);
   const uint32_t attachment_state_total_size =
      attachment_state_item_size * state->attachment_alloc_count;
   if (state->meta.attachment_alloc_count < state->attachment_alloc_count) {
      if (state->meta.attachment_alloc_count > 0)
         vk_free(&cmd_buffer->device->vk.alloc, state->meta.attachments);

      state->meta.attachments = vk_zalloc(&cmd_buffer->device->vk.alloc,
                                          attachment_state_total_size, 8,
                                          VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
      if (!state->meta.attachments) {
         v3dv_flag_oom(cmd_buffer, NULL);
         return;
      }
      state->meta.attachment_alloc_count = state->attachment_alloc_count;
   }
   state->meta.attachment_count = state->attachment_alloc_count;
   memcpy(state->meta.attachments, state->attachments,
          attachment_state_total_size);

   if (state->subpass_idx != -1) {
      state->meta.subpass_idx = state->subpass_idx;
      state->meta.framebuffer = v3dv_framebuffer_to_handle(state->framebuffer);
      state->meta.pass = v3dv_render_pass_to_handle(state->pass);

      state->meta.tile_aligned_render_area = state->tile_aligned_render_area;
      memcpy(&state->meta.render_area, &state->render_area, sizeof(VkRect2D));
   }

   /* We expect that meta operations are graphics-only, so we only take into
    * account the graphics pipeline, and the graphics state
    */
   state->meta.gfx.pipeline = state->gfx.pipeline;
   memcpy(&state->meta.dynamic, &state->dynamic, sizeof(state->dynamic));

   struct v3dv_descriptor_state *gfx_descriptor_state =
      &cmd_buffer->state.gfx.descriptor_state;

   if (push_descriptor_state) {
      if (gfx_descriptor_state->valid != 0) {
         memcpy(&state->meta.gfx.descriptor_state, gfx_descriptor_state,
                sizeof(state->gfx.descriptor_state));
      }
      state->meta.has_descriptor_state = true;
   } else {
      state->meta.has_descriptor_state = false;
   }

   if (cmd_buffer->state.push_constants_size > 0) {
      state->meta.push_constants_size = cmd_buffer->state.push_constants_size;
      memcpy(state->meta.push_constants, cmd_buffer->state.push_constants_data,
             cmd_buffer->state.push_constants_size);
      cmd_buffer->state.push_constants_size = 0;
   }
}

/* This restores command buffer state after a meta operation
 */
void
v3dv_cmd_buffer_meta_state_pop(struct v3dv_cmd_buffer *cmd_buffer,
                               bool needs_subpass_resume)
{
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;

   /* Attachment state */
   assert(state->meta.attachment_count <= state->attachment_alloc_count);
   const uint32_t attachment_state_item_size =
      sizeof(struct v3dv_cmd_buffer_attachment_state);
   const uint32_t attachment_state_total_size =
      attachment_state_item_size * state->meta.attachment_count;
   memcpy(state->attachments, state->meta.attachments,
          attachment_state_total_size);

   if (state->meta.subpass_idx != -1) {
      state->pass = v3dv_render_pass_from_handle(state->meta.pass);
      state->framebuffer = v3dv_framebuffer_from_handle(state->meta.framebuffer);

      state->tile_aligned_render_area = state->meta.tile_aligned_render_area;
      memcpy(&state->render_area, &state->meta.render_area, sizeof(VkRect2D));

      /* Is needs_subpass_resume is true it means that the emitted the meta
       * operation in its own job (possibly with an RT config that is
       * incompatible with the current subpass), so resuming subpass execution
       * after it requires that we create a new job with the subpass RT setup.
       */
      if (needs_subpass_resume)
         v3dv_cmd_buffer_subpass_resume(cmd_buffer, state->meta.subpass_idx);
   } else {
      state->subpass_idx = -1;
   }

   if (state->meta.gfx.pipeline != NULL) {
      struct v3dv_pipeline *pipeline = state->meta.gfx.pipeline;
      VkPipelineBindPoint pipeline_binding =
         v3dv_pipeline_get_binding_point(pipeline);
      v3dv_CmdBindPipeline(v3dv_cmd_buffer_to_handle(cmd_buffer),
                           pipeline_binding,
                           v3dv_pipeline_to_handle(state->meta.gfx.pipeline));
   } else {
      state->gfx.pipeline = NULL;
   }

   /* Restore dynamic state */
   memcpy(&state->dynamic, &state->meta.dynamic, sizeof(state->dynamic));
   state->dirty = ~0;

   if (state->meta.has_descriptor_state) {
      if (state->meta.gfx.descriptor_state.valid != 0) {
         memcpy(&state->gfx.descriptor_state, &state->meta.gfx.descriptor_state,
                sizeof(state->gfx.descriptor_state));
      } else {
         state->gfx.descriptor_state.valid = 0;
      }
   }

   /* We only need to restore push constant data if we had any data in the
    * original command buffer and the meta operation wrote new push constant
    * data.
    */
   if (state->meta.push_constants_size > 0 &&
       cmd_buffer->state.push_constants_size > 0) {
      memcpy(cmd_buffer->state.push_constants_data, state->meta.push_constants,
             state->meta.push_constants_size);
   }
   cmd_buffer->state.push_constants_size = state->meta.push_constants_size;

   state->meta.gfx.pipeline = NULL;
   state->meta.framebuffer = VK_NULL_HANDLE;
   state->meta.pass = VK_NULL_HANDLE;
   state->meta.subpass_idx = -1;
   state->meta.has_descriptor_state = false;
   state->meta.push_constants_size = 0;
}

static struct v3dv_job *
cmd_buffer_pre_draw_split_job(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);

   /* If the job has been flagged with 'always_flush' and it has already
    * recorded any draw calls then we need to start a new job for it.
    */
   if (job->always_flush && job->draw_count > 0) {
      assert(cmd_buffer->state.pass);
      /* First, flag the current job as not being the last in the
       * current subpass
       */
      job->is_subpass_finish = false;

      /* Now start a new job in the same subpass and flag it as continuing
       * the current subpass.
       */
      job = v3dv_cmd_buffer_subpass_resume(cmd_buffer,
                                           cmd_buffer->state.subpass_idx);
      assert(job->draw_count == 0);

      /* Inherit the 'always flush' behavior */
      job->always_flush = true;
   }

   assert(job->draw_count == 0 || !job->always_flush);
   return job;
}

/**
 * The Vulkan spec states:
 *
 *   "It is legal for a subpass to use no color or depth/stencil
 *    attachments (...)  This kind of subpass can use shader side effects such
 *    as image stores and atomics to produce an output. In this case, the
 *    subpass continues to use the width, height, and layers of the framebuffer
 *    to define the dimensions of the rendering area, and the
 *    rasterizationSamples from each pipeline’s
 *    VkPipelineMultisampleStateCreateInfo to define the number of samples used
 *    in rasterization."
 *
 * We need to enable MSAA in the TILE_BINNING_MODE_CFG packet, which we
 * emit when we start a new frame at the beginning of a subpass. At that point,
 * if the framebuffer doesn't have any attachments we won't enable MSAA and
 * the job won't be valid in the scenario described by the spec.
 *
 * This function is intended to be called before a draw call and will test if
 * we are in that scenario, in which case, it will restart the current job
 * with MSAA enabled.
 */
static void
cmd_buffer_restart_job_for_msaa_if_needed(struct v3dv_cmd_buffer *cmd_buffer)
{
   assert(cmd_buffer->state.job);

   /* We don't support variableMultisampleRate so we know that all pipelines
    * bound in the same subpass must have matching number of samples, so we
    * can do this check only on the first draw call.
    */
   if (cmd_buffer->state.job->draw_count > 0)
      return;

   /* We only need to restart the frame if the pipeline requires MSAA but
    * our frame tiling didn't enable it.
    */
   if (!cmd_buffer->state.gfx.pipeline->msaa ||
       cmd_buffer->state.job->frame_tiling.msaa) {
      return;
   }

   /* FIXME: Secondary command buffers don't start frames. Instead, they are
    * recorded into primary jobs that start them. For secondaries, we should
    * still handle this scenario, but we should do that when we record them
    * into primaries by testing if any of the secondaries has multisampled
    * draw calls in them, and then using that info to decide if we need to
    * restart the primary job into which they are being recorded.
    */
   if (cmd_buffer->vk.level != VK_COMMAND_BUFFER_LEVEL_PRIMARY)
      return;

   /* Drop the current job and restart it with MSAA enabled */
   struct v3dv_job *old_job = cmd_buffer->state.job;
   cmd_buffer->state.job = NULL;

   struct v3dv_job *job = vk_zalloc(&cmd_buffer->device->vk.alloc,
                                    sizeof(struct v3dv_job), 8,
                                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!job) {
      v3dv_flag_oom(cmd_buffer, NULL);
      return;
   }

   v3dv_job_init(job, V3DV_JOB_TYPE_GPU_CL, cmd_buffer->device, cmd_buffer,
                 cmd_buffer->state.subpass_idx);
   cmd_buffer->state.job = job;

   v3dv_job_start_frame(job,
                        old_job->frame_tiling.width,
                        old_job->frame_tiling.height,
                        old_job->frame_tiling.layers,
                        true, false,
                        old_job->frame_tiling.render_target_count,
                        old_job->frame_tiling.internal_bpp,
                        old_job->frame_tiling.total_color_bpp,
                        true /* msaa */);

   v3dv_job_destroy(old_job);
}

static bool
cmd_buffer_binning_sync_required(struct v3dv_cmd_buffer *cmd_buffer,
                                 struct v3dv_pipeline *pipeline,
                                 bool indexed, bool indirect)
{
   const struct v3dv_descriptor_maps *vs_bin_maps =
      pipeline->shared_data->maps[BROADCOM_SHADER_VERTEX_BIN];

   const struct v3dv_descriptor_maps *gs_bin_maps =
      pipeline->shared_data->maps[BROADCOM_SHADER_GEOMETRY_BIN];

  VkAccessFlags buffer_access =
      cmd_buffer->state.barrier.bcl_buffer_access;
   if (buffer_access) {
      /* Index buffer read */
      if (indexed && (buffer_access & (VK_ACCESS_2_INDEX_READ_BIT |
                                       VK_ACCESS_2_MEMORY_READ_BIT))) {
         return true;
      }

      /* Indirect buffer read */
      if (indirect && (buffer_access & (VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT |
                                        VK_ACCESS_2_MEMORY_READ_BIT))) {
         return true;
      }

      /* Attribute read */
      if (buffer_access & (VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT |
                          VK_ACCESS_2_MEMORY_READ_BIT)) {
         const struct v3d_vs_prog_data *prog_data =
            pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX_BIN]->prog_data.vs;

         for (int i = 0; i < ARRAY_SIZE(prog_data->vattr_sizes); i++) {
            if (prog_data->vattr_sizes[i] > 0)
               return true;
         }
      }

      /* UBO / SSBO read */
      if (buffer_access & (VK_ACCESS_2_UNIFORM_READ_BIT |
                           VK_ACCESS_2_SHADER_READ_BIT |
                           VK_ACCESS_2_MEMORY_READ_BIT |
                           VK_ACCESS_2_SHADER_STORAGE_READ_BIT)) {

         if (vs_bin_maps->ubo_map.num_desc > 0 ||
             vs_bin_maps->ssbo_map.num_desc > 0) {
            return true;
         }

         if (gs_bin_maps && (gs_bin_maps->ubo_map.num_desc > 0 ||
                             gs_bin_maps->ssbo_map.num_desc > 0)) {
            return true;
         }
      }

      /* SSBO write */
      if (buffer_access & (VK_ACCESS_2_SHADER_WRITE_BIT |
                           VK_ACCESS_2_MEMORY_WRITE_BIT |
                           VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT)) {
         if (vs_bin_maps->ssbo_map.num_desc > 0)
            return true;

         if (gs_bin_maps && gs_bin_maps->ssbo_map.num_desc > 0)
            return true;
      }

      /* Texel Buffer read */
      if (buffer_access & (VK_ACCESS_2_SHADER_SAMPLED_READ_BIT |
                           VK_ACCESS_2_MEMORY_READ_BIT)) {
         if (vs_bin_maps->texture_map.num_desc > 0)
            return true;

         if (gs_bin_maps && gs_bin_maps->texture_map.num_desc > 0)
            return true;
      }
   }

   VkAccessFlags image_access =
      cmd_buffer->state.barrier.bcl_image_access;
   if (image_access) {
      /* Image load / store */
      if (image_access & (VK_ACCESS_2_SHADER_READ_BIT |
                          VK_ACCESS_2_SHADER_WRITE_BIT |
                          VK_ACCESS_2_SHADER_SAMPLED_READ_BIT |
                          VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                          VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT |
                          VK_ACCESS_2_MEMORY_READ_BIT |
                          VK_ACCESS_2_MEMORY_WRITE_BIT)) {
         if (vs_bin_maps->texture_map.num_desc > 0 ||
             vs_bin_maps->sampler_map.num_desc > 0) {
            return true;
         }

         if (gs_bin_maps && (gs_bin_maps->texture_map.num_desc > 0 ||
                             gs_bin_maps->sampler_map.num_desc > 0)) {
            return true;
         }
      }
   }

   return false;
}

void
v3dv_cmd_buffer_consume_bcl_sync(struct v3dv_cmd_buffer *cmd_buffer,
                                 struct v3dv_job *job)
{
   job->needs_bcl_sync = true;
   cmd_buffer->state.barrier.bcl_buffer_access = 0;
   cmd_buffer->state.barrier.bcl_image_access = 0;
}

static inline uint32_t
compute_prog_score(struct v3dv_shader_variant *vs)
{
   const uint32_t inst_count = vs->qpu_insts_size / sizeof(uint64_t);
   const uint32_t tmu_count = vs->prog_data.base->tmu_count +
                              vs->prog_data.base->tmu_spills +
                              vs->prog_data.base->tmu_fills;
   return inst_count + 4 * tmu_count;
}

static void
job_update_double_buffer_score(struct v3dv_job *job,
                               struct v3dv_pipeline *pipeline,
                               uint32_t vertex_count,
                               VkExtent2D *render_area)
{
   /* FIXME: assume anything with GS workloads is too expensive */
   struct v3dv_shader_variant *gs_bin =
      pipeline->shared_data->variants[BROADCOM_SHADER_GEOMETRY_BIN];
   if (gs_bin) {
      job->can_use_double_buffer = false;
      return;
   }

   /* Keep track of vertex processing: too much geometry processing would not
    * be good for double-buffer.
    */
   struct v3dv_shader_variant *vs_bin =
      pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX_BIN];
   assert(vs_bin);
   uint32_t geom_score = vertex_count * compute_prog_score(vs_bin);

   struct v3dv_shader_variant *vs =
      pipeline->shared_data->variants[BROADCOM_SHADER_VERTEX];
   assert(vs);
   uint32_t vs_score = vertex_count * compute_prog_score(vs);
   geom_score += vs_score;

   job->double_buffer_score.geom += geom_score;

   /* Compute pixel rendering cost.
    *
    * We estimate that on average a draw would render 0.2% of the pixels in
    * the render area. That would be a 64x64 region in a 1920x1080 area.
    */
   struct v3dv_shader_variant *fs =
      pipeline->shared_data->variants[BROADCOM_SHADER_FRAGMENT];
   assert(fs);
   uint32_t pixel_count = 0.002f * render_area->width * render_area->height;
   uint32_t render_score = vs_score + pixel_count * compute_prog_score(fs);

   job->double_buffer_score.render += render_score;
}

void
v3dv_cmd_buffer_emit_pre_draw(struct v3dv_cmd_buffer *cmd_buffer,
                              bool indexed, bool indirect,
                              uint32_t vertex_count)
{
   assert(cmd_buffer->state.gfx.pipeline);
   assert(!(cmd_buffer->state.gfx.pipeline->active_stages & VK_SHADER_STAGE_COMPUTE_BIT));

   /* If we emitted a pipeline barrier right before this draw we won't have
    * an active job. In that case, create a new job continuing the current
    * subpass.
    */
   if (!cmd_buffer->state.job) {
      v3dv_cmd_buffer_subpass_resume(cmd_buffer,
                                     cmd_buffer->state.subpass_idx);
   }

   /* Restart single sample job for MSAA pipeline if needed */
   cmd_buffer_restart_job_for_msaa_if_needed(cmd_buffer);

   /* If the job is configured to flush on every draw call we need to create
    * a new job now.
    */
   struct v3dv_job *job = cmd_buffer_pre_draw_split_job(cmd_buffer);
   job->draw_count++;

   /* Track VK_KHR_buffer_device_address usage in the job */
   struct v3dv_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   job->uses_buffer_device_address |= pipeline->uses_buffer_device_address;

   /* If this job is serialized (has consumed a barrier) then check if we need
    * to sync at the binning stage by testing if the binning shaders involved
    * with the draw call require access to external resources.
    */
   if (job->serialize && (cmd_buffer->state.barrier.bcl_buffer_access ||
                          cmd_buffer->state.barrier.bcl_image_access)) {
      assert(!job->needs_bcl_sync);
      if (cmd_buffer_binning_sync_required(cmd_buffer, pipeline,
                                           indexed, indirect)) {
         v3dv_cmd_buffer_consume_bcl_sync(cmd_buffer, job);
      }
   }

   /* GL shader state binds shaders, uniform and vertex attribute state. The
    * compiler injects uniforms to handle some descriptor types (such as
    * textures), so we need to regen that when descriptor state changes.
    *
    * We also need to emit new shader state if we have a dirty viewport since
    * that will require that we new uniform state for QUNIFORM_VIEWPORT_*.
    */
   uint32_t *dirty = &cmd_buffer->state.dirty;

   const uint32_t dirty_uniform_state =
      *dirty & (V3DV_CMD_DIRTY_PIPELINE |
                V3DV_CMD_DIRTY_PUSH_CONSTANTS |
                V3DV_CMD_DIRTY_DESCRIPTOR_SETS |
                V3DV_CMD_DIRTY_VIEWPORT |
                V3DV_CMD_DIRTY_VIEW_INDEX |
                V3DV_CMD_DIRTY_DRAW_ID);

   if (dirty_uniform_state)
      update_gfx_uniform_state(cmd_buffer, dirty_uniform_state);

   struct v3dv_device *device = cmd_buffer->device;

   if (dirty_uniform_state || (*dirty & V3DV_CMD_DIRTY_VERTEX_BUFFER))
      v3dv_X(device, cmd_buffer_emit_gl_shader_state)(cmd_buffer);

   if (*dirty & (V3DV_CMD_DIRTY_PIPELINE)) {
      v3dv_X(device, cmd_buffer_emit_configuration_bits)(cmd_buffer);
      v3dv_X(device, cmd_buffer_emit_varyings_state)(cmd_buffer);
   }

   if (*dirty & (V3DV_CMD_DIRTY_VIEWPORT | V3DV_CMD_DIRTY_SCISSOR)) {
      emit_scissor(cmd_buffer);
   }

   if (*dirty & V3DV_CMD_DIRTY_VIEWPORT) {
      v3dv_X(device, cmd_buffer_emit_viewport)(cmd_buffer);
   }

   if (*dirty & V3DV_CMD_DIRTY_INDEX_BUFFER)
      v3dv_X(device, cmd_buffer_emit_index_buffer)(cmd_buffer);

   const uint32_t dynamic_stencil_dirty_flags =
      V3DV_CMD_DIRTY_STENCIL_COMPARE_MASK |
      V3DV_CMD_DIRTY_STENCIL_WRITE_MASK |
      V3DV_CMD_DIRTY_STENCIL_REFERENCE;
   if (*dirty & (V3DV_CMD_DIRTY_PIPELINE | dynamic_stencil_dirty_flags))
      v3dv_X(device, cmd_buffer_emit_stencil)(cmd_buffer);

   if (*dirty & (V3DV_CMD_DIRTY_PIPELINE | V3DV_CMD_DIRTY_DEPTH_BIAS))
      v3dv_X(device, cmd_buffer_emit_depth_bias)(cmd_buffer);

   if (*dirty & V3DV_CMD_DIRTY_DEPTH_BOUNDS)
      v3dv_X(device, cmd_buffer_emit_depth_bounds)(cmd_buffer);

   if (*dirty & (V3DV_CMD_DIRTY_PIPELINE | V3DV_CMD_DIRTY_BLEND_CONSTANTS))
      v3dv_X(device, cmd_buffer_emit_blend)(cmd_buffer);

   if (*dirty & V3DV_CMD_DIRTY_OCCLUSION_QUERY)
      v3dv_X(device, cmd_buffer_emit_occlusion_query)(cmd_buffer);

   if (*dirty & V3DV_CMD_DIRTY_LINE_WIDTH)
      v3dv_X(device, cmd_buffer_emit_line_width)(cmd_buffer);

   if (*dirty & V3DV_CMD_DIRTY_PIPELINE)
      v3dv_X(device, cmd_buffer_emit_sample_state)(cmd_buffer);

   if (*dirty & (V3DV_CMD_DIRTY_PIPELINE | V3DV_CMD_DIRTY_COLOR_WRITE_ENABLE))
      v3dv_X(device, cmd_buffer_emit_color_write_mask)(cmd_buffer);

   /* We disable double-buffer mode if indirect draws are used because in that
    * case we don't know the vertex count.
    */
   if (indirect) {
      job->can_use_double_buffer = false;
   } else if (job->can_use_double_buffer) {
      job_update_double_buffer_score(job, pipeline, vertex_count,
                                      &cmd_buffer->state.render_area.extent);
   }

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_PIPELINE;
}

static inline void
cmd_buffer_set_view_index(struct v3dv_cmd_buffer *cmd_buffer,
                          uint32_t view_index)
{
   cmd_buffer->state.view_index = view_index;
   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_VIEW_INDEX;
}

static void
cmd_buffer_draw(struct v3dv_cmd_buffer *cmd_buffer,
                struct v3dv_draw_info *info)
{
   uint32_t vertex_count =
      info->vertex_count * info->instance_count;

   struct v3dv_render_pass *pass = cmd_buffer->state.pass;
   if (likely(!pass->multiview_enabled)) {
      v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, false, false, vertex_count);
      v3dv_X(cmd_buffer->device, cmd_buffer_emit_draw)(cmd_buffer, info);
      return;
   }

   uint32_t view_mask = pass->subpasses[cmd_buffer->state.subpass_idx].view_mask;
   while (view_mask) {
      cmd_buffer_set_view_index(cmd_buffer, u_bit_scan(&view_mask));
      v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, false, false, vertex_count);
      v3dv_X(cmd_buffer->device, cmd_buffer_emit_draw)(cmd_buffer, info);
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdDraw(VkCommandBuffer commandBuffer,
             uint32_t vertexCount,
             uint32_t instanceCount,
             uint32_t firstVertex,
             uint32_t firstInstance)
{
   if (vertexCount == 0 || instanceCount == 0)
      return;

   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   struct v3dv_draw_info info = {};
   info.vertex_count = vertexCount;
   info.instance_count = instanceCount;
   info.first_instance = firstInstance;
   info.first_vertex = firstVertex;

   cmd_buffer_draw(cmd_buffer, &info);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdDrawMultiEXT(VkCommandBuffer commandBuffer,
                     uint32_t drawCount,
                     const VkMultiDrawInfoEXT *pVertexInfo,
                     uint32_t instanceCount,
                     uint32_t firstInstance,
                     uint32_t stride)

{
   if (drawCount == 0 || instanceCount == 0)
      return;

   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   uint32_t i = 0;
   vk_foreach_multi_draw(draw, i, pVertexInfo, drawCount, stride) {
      cmd_buffer->state.draw_id = i;
      cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_DRAW_ID;

      struct v3dv_draw_info info = {};
      info.vertex_count = draw->vertexCount;
      info.instance_count = instanceCount;
      info.first_instance = firstInstance;
      info.first_vertex = draw->firstVertex;

      cmd_buffer_draw(cmd_buffer, &info);
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdDrawIndexed(VkCommandBuffer commandBuffer,
                    uint32_t indexCount,
                    uint32_t instanceCount,
                    uint32_t firstIndex,
                    int32_t vertexOffset,
                    uint32_t firstInstance)
{
   if (indexCount == 0 || instanceCount == 0)
      return;

   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   uint32_t vertex_count = indexCount * instanceCount;

   struct v3dv_render_pass *pass = cmd_buffer->state.pass;
   if (likely(!pass->multiview_enabled)) {
      v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, true, false, vertex_count);
      v3dv_X(cmd_buffer->device, cmd_buffer_emit_draw_indexed)
         (cmd_buffer, indexCount, instanceCount,
          firstIndex, vertexOffset, firstInstance);
      return;
   }

   uint32_t view_mask = pass->subpasses[cmd_buffer->state.subpass_idx].view_mask;
   while (view_mask) {
      cmd_buffer_set_view_index(cmd_buffer, u_bit_scan(&view_mask));
      v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, true, false, vertex_count);
      v3dv_X(cmd_buffer->device, cmd_buffer_emit_draw_indexed)
         (cmd_buffer, indexCount, instanceCount,
          firstIndex, vertexOffset, firstInstance);
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer,
                            uint32_t drawCount,
                            const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                            uint32_t instanceCount,
                            uint32_t firstInstance,
                            uint32_t stride,
                            const int32_t *pVertexOffset)
{
   if (drawCount == 0 || instanceCount == 0)
      return;

   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   uint32_t i = 0;
   vk_foreach_multi_draw_indexed(draw, i, pIndexInfo, drawCount, stride) {
      uint32_t vertex_count = draw->indexCount * instanceCount;
      int32_t vertexOffset = pVertexOffset ? *pVertexOffset : draw->vertexOffset;

      cmd_buffer->state.draw_id = i;
      cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_DRAW_ID;

      struct v3dv_render_pass *pass = cmd_buffer->state.pass;
      if (likely(!pass->multiview_enabled)) {
         v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, true, false, vertex_count);
         v3dv_X(cmd_buffer->device, cmd_buffer_emit_draw_indexed)
            (cmd_buffer, draw->indexCount, instanceCount,
             draw->firstIndex, vertexOffset, firstInstance);
         continue;
      }

      uint32_t view_mask = pass->subpasses[cmd_buffer->state.subpass_idx].view_mask;
      while (view_mask) {
         cmd_buffer_set_view_index(cmd_buffer, u_bit_scan(&view_mask));
         v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, true, false, vertex_count);
         v3dv_X(cmd_buffer->device, cmd_buffer_emit_draw_indexed)
            (cmd_buffer, draw->indexCount, instanceCount,
             draw->firstIndex, vertexOffset, firstInstance);
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdDrawIndirect(VkCommandBuffer commandBuffer,
                     VkBuffer _buffer,
                     VkDeviceSize offset,
                     uint32_t drawCount,
                     uint32_t stride)
{
   /* drawCount is the number of draws to execute, and can be zero. */
   if (drawCount == 0)
      return;

   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_buffer, buffer, _buffer);

   struct v3dv_render_pass *pass = cmd_buffer->state.pass;
   if (likely(!pass->multiview_enabled)) {
      v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, false, true, 0);
      v3dv_X(cmd_buffer->device, cmd_buffer_emit_draw_indirect)
         (cmd_buffer, buffer, offset, drawCount, stride);
      return;
   }

   uint32_t view_mask = pass->subpasses[cmd_buffer->state.subpass_idx].view_mask;
   while (view_mask) {
      cmd_buffer_set_view_index(cmd_buffer, u_bit_scan(&view_mask));
      v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, false, true, 0);
      v3dv_X(cmd_buffer->device, cmd_buffer_emit_draw_indirect)
         (cmd_buffer, buffer, offset, drawCount, stride);
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer,
                            VkBuffer _buffer,
                            VkDeviceSize offset,
                            uint32_t drawCount,
                            uint32_t stride)
{
   /* drawCount is the number of draws to execute, and can be zero. */
   if (drawCount == 0)
      return;

   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_buffer, buffer, _buffer);

   struct v3dv_render_pass *pass = cmd_buffer->state.pass;
   if (likely(!pass->multiview_enabled)) {
      v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, true, true, 0);
      v3dv_X(cmd_buffer->device, cmd_buffer_emit_indexed_indirect)
         (cmd_buffer, buffer, offset, drawCount, stride);
      return;
   }

   uint32_t view_mask = pass->subpasses[cmd_buffer->state.subpass_idx].view_mask;
   while (view_mask) {
      cmd_buffer_set_view_index(cmd_buffer, u_bit_scan(&view_mask));
      v3dv_cmd_buffer_emit_pre_draw(cmd_buffer, true, true, 0);
      v3dv_X(cmd_buffer->device, cmd_buffer_emit_indexed_indirect)
         (cmd_buffer, buffer, offset, drawCount, stride);
   }
}

static void
handle_barrier(VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
               VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
               bool is_image_barrier, bool is_buffer_barrier,
               struct v3dv_barrier_state *state)
{
   /* We only care about barriers between GPU jobs */
   if (srcStageMask == VK_PIPELINE_STAGE_2_HOST_BIT ||
       dstStageMask == VK_PIPELINE_STAGE_2_HOST_BIT) {
      return;
   }

   /* Track source of the barrier */
   uint8_t src_mask = 0;

   const VkPipelineStageFlags2 compute_mask =
      VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
   if (srcStageMask & (compute_mask | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT))
      src_mask |= V3DV_BARRIER_COMPUTE_BIT;

   const VkPipelineStageFlags2 transfer_mask =
      VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT |
      VK_PIPELINE_STAGE_2_COPY_BIT |
      VK_PIPELINE_STAGE_2_BLIT_BIT |
      VK_PIPELINE_STAGE_2_CLEAR_BIT;
   if (srcStageMask & (transfer_mask | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT))
      src_mask |= V3DV_BARRIER_TRANSFER_BIT;

   const VkPipelineStageFlags2 graphics_mask = ~(compute_mask | transfer_mask);
   if (srcStageMask & (graphics_mask | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT))
      src_mask |= V3DV_BARRIER_GRAPHICS_BIT;

   /* Track consumer of the barrier */
   if (dstStageMask & (compute_mask | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)) {
      state->dst_mask |= V3DV_BARRIER_COMPUTE_BIT;
      state->src_mask_compute |= src_mask;
   }

   if (dstStageMask & (transfer_mask | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)) {
      state->dst_mask |= V3DV_BARRIER_TRANSFER_BIT;
      state->src_mask_transfer |= src_mask;
   }

   if (dstStageMask & (graphics_mask | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)) {
      state->dst_mask |= V3DV_BARRIER_GRAPHICS_BIT;
      state->src_mask_graphics |= src_mask;

      if (dstStageMask & (VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT |
                          VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT |
                          VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT |
                          VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT |
                          VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
                          VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT |
                          VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT |
                          VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT |
                          VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT |
                          VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT |
                          VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT |
                          VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)) {
         if (is_image_barrier)
            state->bcl_image_access |= dstAccessMask;

         if (is_buffer_barrier)
            state->bcl_buffer_access |= dstAccessMask;
      }
   }
}

void
v3dv_cmd_buffer_emit_pipeline_barrier(struct v3dv_cmd_buffer *cmd_buffer,
                                      const VkDependencyInfo *info)
{
   uint32_t imageBarrierCount = info->imageMemoryBarrierCount;
   const VkImageMemoryBarrier2 *pImageBarriers = info->pImageMemoryBarriers;

   uint32_t bufferBarrierCount = info->bufferMemoryBarrierCount;
   const VkBufferMemoryBarrier2 *pBufferBarriers = info->pBufferMemoryBarriers;

   uint32_t memoryBarrierCount = info->memoryBarrierCount;
   const VkMemoryBarrier2 *pMemoryBarriers = info->pMemoryBarriers;

   struct v3dv_barrier_state state = { 0 };
   for (uint32_t i = 0; i < imageBarrierCount; i++) {
      /* We can safely skip barriers for image layout transitions from UNDEFINED
       * layout.
       *
       * Notice that KHR_synchronization2 allows to specify barriers that don't
       * involve a layout transition by making oldLayout and newLayout the same,
       * including UNDEFINED.
       */
      if (pImageBarriers[i].oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
          pImageBarriers[i].oldLayout != pImageBarriers[i].newLayout) {
         continue;
      }

      handle_barrier(pImageBarriers[i].srcStageMask,
                     pImageBarriers[i].srcAccessMask,
                     pImageBarriers[i].dstStageMask,
                     pImageBarriers[i].dstAccessMask,
                     true, false, &state);
   }

   for (uint32_t i = 0; i < bufferBarrierCount; i++) {
      handle_barrier(pBufferBarriers[i].srcStageMask,
                     pBufferBarriers[i].srcAccessMask,
                     pBufferBarriers[i].dstStageMask,
                     pBufferBarriers[i].dstAccessMask,
                     false, true, &state);
   }

   for (uint32_t i = 0; i < memoryBarrierCount; i++) {
      handle_barrier(pMemoryBarriers[i].srcStageMask,
                     pMemoryBarriers[i].srcAccessMask,
                     pMemoryBarriers[i].dstStageMask,
                     pMemoryBarriers[i].dstAccessMask,
                     true, true, &state);
   }

   /* Bail if we don't relevant barriers */
   if (!state.dst_mask)
      return;

   /* If we have a recording job, finish it here */
   if (cmd_buffer->state.job)
      v3dv_cmd_buffer_finish_job(cmd_buffer);

   /* Update barrier state in the command buffer */
   v3dv_cmd_buffer_merge_barrier_state(&cmd_buffer->state.barrier, &state);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                         const VkDependencyInfo *pDependencyInfo)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   v3dv_cmd_buffer_emit_pipeline_barrier(cmd_buffer, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdBindVertexBuffers(VkCommandBuffer commandBuffer,
                          uint32_t firstBinding,
                          uint32_t bindingCount,
                          const VkBuffer *pBuffers,
                          const VkDeviceSize *pOffsets)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   struct v3dv_vertex_binding *vb = cmd_buffer->state.vertex_bindings;

   /* We have to defer setting up vertex buffer since we need the buffer
    * stride from the pipeline.
    */

   assert(firstBinding + bindingCount <= MAX_VBS);
   bool vb_state_changed = false;
   for (uint32_t i = 0; i < bindingCount; i++) {
      if (vb[firstBinding + i].buffer != v3dv_buffer_from_handle(pBuffers[i])) {
         vb[firstBinding + i].buffer = v3dv_buffer_from_handle(pBuffers[i]);
         vb_state_changed = true;
      }
      if (vb[firstBinding + i].offset != pOffsets[i]) {
         vb[firstBinding + i].offset = pOffsets[i];
         vb_state_changed = true;
      }
   }

   if (vb_state_changed)
      cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_VERTEX_BUFFER;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdBindIndexBuffer(VkCommandBuffer commandBuffer,
                        VkBuffer buffer,
                        VkDeviceSize offset,
                        VkIndexType indexType)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   const uint32_t index_size = vk_index_type_to_bytes(indexType);
   if (buffer == cmd_buffer->state.index_buffer.buffer &&
       offset == cmd_buffer->state.index_buffer.offset &&
       index_size == cmd_buffer->state.index_buffer.index_size) {
      return;
   }

   cmd_buffer->state.index_buffer.buffer = buffer;
   cmd_buffer->state.index_buffer.offset = offset;
   cmd_buffer->state.index_buffer.index_size = index_size;
   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_INDEX_BUFFER;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
                              VkStencilFaceFlags faceMask,
                              uint32_t compareMask)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      cmd_buffer->state.dynamic.stencil_compare_mask.front = compareMask & 0xff;
   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      cmd_buffer->state.dynamic.stencil_compare_mask.back = compareMask & 0xff;

   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_STENCIL_COMPARE_MASK;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer,
                            VkStencilFaceFlags faceMask,
                            uint32_t writeMask)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      cmd_buffer->state.dynamic.stencil_write_mask.front = writeMask & 0xff;
   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      cmd_buffer->state.dynamic.stencil_write_mask.back = writeMask & 0xff;

   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_STENCIL_WRITE_MASK;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetStencilReference(VkCommandBuffer commandBuffer,
                            VkStencilFaceFlags faceMask,
                            uint32_t reference)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      cmd_buffer->state.dynamic.stencil_reference.front = reference & 0xff;
   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      cmd_buffer->state.dynamic.stencil_reference.back = reference & 0xff;

   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_STENCIL_REFERENCE;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetDepthBias(VkCommandBuffer commandBuffer,
                     float depthBiasConstantFactor,
                     float depthBiasClamp,
                     float depthBiasSlopeFactor)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   cmd_buffer->state.dynamic.depth_bias.constant_factor = depthBiasConstantFactor;
   cmd_buffer->state.dynamic.depth_bias.depth_bias_clamp = depthBiasClamp;
   cmd_buffer->state.dynamic.depth_bias.slope_factor = depthBiasSlopeFactor;
   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_DEPTH_BIAS;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetDepthBounds(VkCommandBuffer commandBuffer,
                       float minDepthBounds,
                       float maxDepthBounds)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   cmd_buffer->state.dynamic.depth_bounds.min = minDepthBounds;
   cmd_buffer->state.dynamic.depth_bounds.max = maxDepthBounds;
   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_DEPTH_BOUNDS;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetLineStippleEXT(VkCommandBuffer commandBuffer,
                          uint32_t lineStippleFactor,
                          uint16_t lineStipplePattern)
{
   /* We do not support stippled line rasterization so we just ignore this. */
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetLineWidth(VkCommandBuffer commandBuffer,
                     float lineWidth)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   cmd_buffer->state.dynamic.line_width = lineWidth;
   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_LINE_WIDTH;
}

/**
 * This checks a descriptor set to see if are binding any descriptors that would
 * involve sampling from a linear image (the hardware only supports this for
 * 1D images), and if so, attempts to create a tiled copy of the linear image
 * and rewrite the descriptor set to use that instead.
 *
 * This was added to support a scenario with Android where some part of the UI
 * wanted to show previews of linear swapchain images. For more details:
 * https://gitlab.freedesktop.org/mesa/mesa/-/issues/9712
 *
 * Currently this only supports a linear sampling from a simple 2D image, but
 * it could be extended to support more cases if necessary.
 */
static void
handle_sample_from_linear_image(struct v3dv_cmd_buffer *cmd_buffer,
                                struct v3dv_descriptor_set *set,
                                bool is_compute)
{
   for (int32_t i = 0; i < set->layout->binding_count; i++) {
      const struct v3dv_descriptor_set_binding_layout *blayout =
         &set->layout->binding[i];
      if (blayout->type != VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE &&
          blayout->type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
         continue;

      struct v3dv_descriptor *desc = &set->descriptors[blayout->descriptor_index];
      if (!desc->image_view)
         continue;

      struct v3dv_image *image = (struct v3dv_image *) desc->image_view->vk.image;
      struct v3dv_image_view *view = (struct v3dv_image_view *) desc->image_view;
      if (image->tiled || view->vk.view_type == VK_IMAGE_VIEW_TYPE_1D ||
                          view->vk.view_type == VK_IMAGE_VIEW_TYPE_1D_ARRAY) {
         continue;
      }

      /* FIXME: we can probably handle most of these restrictions too with
       * a bit of extra effort.
       */
      if (view->vk.view_type != VK_IMAGE_VIEW_TYPE_2D ||
          view->vk.level_count != 1 || view->vk.layer_count != 1 ||
          blayout->array_size != 1) {
         fprintf(stderr, "Sampling from linear image is not supported. "
                 "Expect corruption.\n");
         continue;
      }

      /* We are sampling from a linear image. V3D doesn't support this
       * so we create a tiled copy of the image and rewrite the descriptor
       * to read from it instead.
       */
      perf_debug("Sampling from linear image is not supported natively and "
                 "requires a copy.\n");

      struct v3dv_device *device = cmd_buffer->device;
      VkDevice vk_device = v3dv_device_to_handle(device);

      /* Allocate shadow tiled image if needed, we only do this once for
       * each image, on the first sampling attempt. We need to take a lock
       * since we may be trying to do the same in another command buffer in
       * a separate thread.
       */
      mtx_lock(&device->meta.mtx);
      VkResult result;
      VkImage tiled_image;
      if (image->shadow) {
         tiled_image = v3dv_image_to_handle(image->shadow);
      } else {
         VkImageCreateInfo image_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = image->vk.create_flags,
            .imageType = image->vk.image_type,
            .format = image->vk.format,
            .extent = {
               image->vk.extent.width,
               image->vk.extent.height,
               image->vk.extent.depth,
            },
            .mipLevels = image->vk.mip_levels,
            .arrayLayers = image->vk.array_layers,
            .samples = image->vk.samples,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = image->vk.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
         };
         result = v3dv_CreateImage(vk_device, &image_info,
                                   &device->vk.alloc, &tiled_image);
         if (result != VK_SUCCESS) {
            fprintf(stderr, "Failed to copy linear 2D image for sampling."
                    "Expect corruption.\n");
            mtx_unlock(&device->meta.mtx);
            continue;
         }

         bool disjoint = image->vk.create_flags & VK_IMAGE_CREATE_DISJOINT_BIT;
         VkImageMemoryRequirementsInfo2 reqs_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
            .image = tiled_image,
         };

         assert(image->plane_count <= V3DV_MAX_PLANE_COUNT);
         for (int p = 0; p < (disjoint ? image->plane_count : 1); p++) {
            VkImageAspectFlagBits plane_aspect = VK_IMAGE_ASPECT_PLANE_0_BIT << p;
            VkImagePlaneMemoryRequirementsInfo plane_info = {
               .sType = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO,
               .planeAspect = plane_aspect,
            };
            if (disjoint)
               reqs_info.pNext = &plane_info;

            VkMemoryRequirements2 reqs = {
               .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
            };
            v3dv_GetImageMemoryRequirements2(vk_device, &reqs_info, &reqs);

            VkDeviceMemory mem;
            VkMemoryAllocateInfo alloc_info = {
               .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .allocationSize = reqs.memoryRequirements.size,
               .memoryTypeIndex = 0,
            };
            result = v3dv_AllocateMemory(vk_device, &alloc_info,
                                         &device->vk.alloc, &mem);
            if (result != VK_SUCCESS) {
               fprintf(stderr, "Failed to copy linear 2D image for sampling."
                       "Expect corruption.\n");
               v3dv_DestroyImage(vk_device, tiled_image, &device->vk.alloc);
               mtx_unlock(&device->meta.mtx);
               continue;
            }

            VkBindImageMemoryInfo bind_info = {
               .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
               .image = tiled_image,
               .memory = mem,
               .memoryOffset = 0,
            };
            VkBindImagePlaneMemoryInfo plane_bind_info = {
               .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO,
               .planeAspect = plane_aspect,
            };
            if (disjoint)
               bind_info.pNext = &plane_bind_info;
            result = v3dv_BindImageMemory2(vk_device, 1, &bind_info);
            if (result != VK_SUCCESS) {
               fprintf(stderr, "Failed to copy linear 2D image for sampling."
                       "Expect corruption.\n");
               v3dv_DestroyImage(vk_device, tiled_image, &device->vk.alloc);
               v3dv_FreeMemory(vk_device, mem, &device->vk.alloc);
               mtx_unlock(&device->meta.mtx);
               continue;
            }
         }

         image->shadow = v3dv_image_from_handle(tiled_image);
      }

      /* Create a shadow view that refers to the tiled image if needed */
      VkImageView tiled_view;
      if (view->shadow) {
         tiled_view = v3dv_image_view_to_handle(view->shadow);
      } else {
         VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .flags = view->vk.create_flags,
            .image = tiled_image,
            .viewType = view->vk.view_type,
            .format = view->vk.format,
            .components = view->vk.swizzle,
            .subresourceRange = {
               .aspectMask = view->vk.aspects,
               .baseMipLevel = view->vk.base_mip_level,
               .levelCount = view->vk.level_count,
               .baseArrayLayer = view->vk.base_array_layer,
               .layerCount = view->vk.layer_count,
            },
         };
         result = v3dv_create_image_view(device, &view_info, &tiled_view);
         if (result != VK_SUCCESS) {
            fprintf(stderr, "Failed to copy linear 2D image for sampling."
                    "Expect corruption.\n");
            mtx_unlock(&device->meta.mtx);
            continue;
         }
      }

      view->shadow = v3dv_image_view_from_handle(tiled_view);

      mtx_unlock(&device->meta.mtx);

      /* Rewrite the descriptor to use the shadow view */
      VkDescriptorImageInfo desc_image_info = {
         .sampler = v3dv_sampler_to_handle(desc->sampler),
         .imageView = tiled_view,
         .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
      };
      VkWriteDescriptorSet write = {
         .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = v3dv_descriptor_set_to_handle(set),
         .dstBinding = i,
         .dstArrayElement = 0, /* Assumes array_size is 1 */
         .descriptorCount = 1,
         .descriptorType = desc->type,
         .pImageInfo = &desc_image_info,
      };
      v3dv_UpdateDescriptorSets(vk_device, 1, &write, 0, NULL);

      /* Now we need to actually copy the pixel data from the linear image
       * into the tiled image storage to ensure it is up-to-date.
       *
       * FIXME: ideally we would track if the linear image is dirty and skip
       * this step otherwise, but that would be a bit of a pain.
       *
       * Note that we need to place the copy job *before* the current job in
       * the command buffer state so we have the tiled image ready to process
       * an upcoming draw call in the current job that samples from it.
       *
       * Also, we need to use the TFU path for this copy, as any other path
       * will use the tile buffer and would require a new framebuffer setup,
       * thus requiring extra work to stop and resume any in-flight render
       * pass. Since we are converting a full 2D texture here the TFU should
       * be able to handle this.
       */
      for (int p = 0; p < image->plane_count; p++) {
         VkImageAspectFlagBits plane_aspect = VK_IMAGE_ASPECT_PLANE_0_BIT << p;
         struct VkImageCopy2 copy_region = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
            .srcSubresource = {
               .aspectMask = image->plane_count == 1 ?
                  view->vk.aspects : (view->vk.aspects & plane_aspect),
               .mipLevel = view->vk.base_mip_level,
               .baseArrayLayer = view->vk.base_array_layer,
               .layerCount = view->vk.layer_count,
            },
            .srcOffset = {0, 0, 0 },
            .dstSubresource = {
               .aspectMask = image->plane_count == 1 ?
                  view->vk.aspects : (view->vk.aspects & plane_aspect),
               .mipLevel = view->vk.base_mip_level,
               .baseArrayLayer = view->vk.base_array_layer,
               .layerCount = view->vk.layer_count,
            },
            .dstOffset = { 0, 0, 0},
            .extent = {
               image->planes[p].width,
               image->planes[p].height,
               1,
            },
         };
         struct v3dv_image *copy_src = image;
         struct v3dv_image *copy_dst = v3dv_image_from_handle(tiled_image);
         bool ok = v3dv_cmd_buffer_copy_image_tfu(cmd_buffer, copy_dst, copy_src,
                                                  &copy_region);
         if (ok) {
            /* This will emit the TFU job right before the current in-flight
             * job (if any), since in-fight jobs are only added to the list
             * when finished.
             */
            struct v3dv_job *tfu_job =
               list_last_entry(&cmd_buffer->jobs, struct v3dv_job, list_link);
            assert(tfu_job->type == V3DV_JOB_TYPE_GPU_TFU);
            /* Serialize the copy since we don't know who is producing the linear
             * image and we need the image to be ready by the time the copy
             * executes.
             */
            tfu_job->serialize = V3DV_BARRIER_ALL;

            /* Also, we need to ensure the TFU copy job completes before anyhing
             * else coming after that may be using the tiled shadow copy.
             */
            if (cmd_buffer->state.job) {
               /* If we already had an in-flight job (i.e. we are in a render
                * pass) make sure the job waits for the TFU copy.
                */
               cmd_buffer->state.job->serialize |= V3DV_BARRIER_TRANSFER_BIT;
            } else {
               /* Otherwise, make the the follow-up job syncs with the TFU
                * job we just added when it is created by adding the
                * corresponding barrier state.
                */
               if (!is_compute) {
                  cmd_buffer->state.barrier.dst_mask |= V3DV_BARRIER_GRAPHICS_BIT;
                  cmd_buffer->state.barrier.src_mask_graphics |= V3DV_BARRIER_TRANSFER_BIT;
               } else {
                  cmd_buffer->state.barrier.dst_mask |= V3DV_BARRIER_COMPUTE_BIT;
                  cmd_buffer->state.barrier.src_mask_compute |= V3DV_BARRIER_TRANSFER_BIT;
               }
            }
         } else {
            fprintf(stderr, "Failed to copy linear 2D image for sampling."
                    "TFU doesn't support copy. Expect corruption.\n");
         }
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                           VkPipelineBindPoint pipelineBindPoint,
                           VkPipelineLayout _layout,
                           uint32_t firstSet,
                           uint32_t descriptorSetCount,
                           const VkDescriptorSet *pDescriptorSets,
                           uint32_t dynamicOffsetCount,
                           const uint32_t *pDynamicOffsets)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_pipeline_layout, layout, _layout);

   uint32_t dyn_index = 0;

   assert(firstSet + descriptorSetCount <= MAX_SETS);

   struct v3dv_descriptor_state *descriptor_state =
      pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE ?
      &cmd_buffer->state.compute.descriptor_state :
      &cmd_buffer->state.gfx.descriptor_state;

   VkShaderStageFlags dirty_stages = 0;
   bool descriptor_state_changed = false;
   for (uint32_t i = 0; i < descriptorSetCount; i++) {
      V3DV_FROM_HANDLE(v3dv_descriptor_set, set, pDescriptorSets[i]);
      uint32_t index = firstSet + i;

      descriptor_state->valid |= (1u << index);
      if (descriptor_state->descriptor_sets[index] != set) {
         descriptor_state->descriptor_sets[index] = set;
         dirty_stages |= set->layout->shader_stages;
         descriptor_state_changed = true;

         /* Check if we are sampling from a linear 2D image. This is not
          * supported in hardware, but may be required for some applications
          * so we will transparently convert to tiled at the expense of
          * performance.
          */
         handle_sample_from_linear_image(cmd_buffer, set,
                                         pipelineBindPoint ==
                                         VK_PIPELINE_BIND_POINT_COMPUTE);
      }

      for (uint32_t j = 0; j < set->layout->dynamic_offset_count; j++, dyn_index++) {
         uint32_t idx = j + layout->set[i + firstSet].dynamic_offset_start;

         if (descriptor_state->dynamic_offsets[idx] != pDynamicOffsets[dyn_index]) {
            descriptor_state->dynamic_offsets[idx] = pDynamicOffsets[dyn_index];
            dirty_stages |= set->layout->shader_stages;
            descriptor_state_changed = true;
         }
      }
   }

   if (descriptor_state_changed) {
      if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
         cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_DESCRIPTOR_SETS;
         cmd_buffer->state.dirty_descriptor_stages |= dirty_stages & VK_SHADER_STAGE_ALL_GRAPHICS;
      } else {
         cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_COMPUTE_DESCRIPTOR_SETS;
         cmd_buffer->state.dirty_descriptor_stages |= VK_SHADER_STAGE_COMPUTE_BIT;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdPushConstants(VkCommandBuffer commandBuffer,
                      VkPipelineLayout layout,
                      VkShaderStageFlags stageFlags,
                      uint32_t offset,
                      uint32_t size,
                      const void *pValues)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   if (!memcmp((uint8_t *) cmd_buffer->state.push_constants_data + offset,
               pValues, size)) {
      return;
   }

   memcpy((uint8_t *) cmd_buffer->state.push_constants_data + offset,
           pValues, size);
   cmd_buffer->state.push_constants_size =
      MAX2(offset + size, cmd_buffer->state.push_constants_size);

   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_PUSH_CONSTANTS |
                              V3DV_CMD_DIRTY_PUSH_CONSTANTS_UBO;
   cmd_buffer->state.dirty_push_constants_stages |= stageFlags;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetBlendConstants(VkCommandBuffer commandBuffer,
                          const float blendConstants[4])
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;

   if (!memcmp(state->dynamic.blend_constants, blendConstants,
               sizeof(state->dynamic.blend_constants))) {
      return;
   }

   memcpy(state->dynamic.blend_constants, blendConstants,
          sizeof(state->dynamic.blend_constants));

   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_BLEND_CONSTANTS;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer,
                               uint32_t attachmentCount,
                               const VkBool32 *pColorWriteEnables)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   uint32_t color_write_enable = 0;

   for (uint32_t i = 0; i < attachmentCount; i++)
      color_write_enable |= pColorWriteEnables[i] ? (0xfu << (i * 4)) : 0;

   if (state->dynamic.color_write_enable == color_write_enable)
      return;

   state->dynamic.color_write_enable = color_write_enable;

   state->dirty |= V3DV_CMD_DIRTY_COLOR_WRITE_ENABLE;
}

void
v3dv_cmd_buffer_ensure_array_state(struct v3dv_cmd_buffer *cmd_buffer,
                                   uint32_t slot_size,
                                   uint32_t used_count,
                                   uint32_t *alloc_count,
                                   void **ptr)
{
   if (used_count >= *alloc_count) {
      const uint32_t prev_slot_count = *alloc_count;
      void *old_buffer = *ptr;

      const uint32_t new_slot_count = MAX2(*alloc_count * 2, 4);
      const uint32_t bytes = new_slot_count * slot_size;
      *ptr = vk_alloc(&cmd_buffer->device->vk.alloc, bytes, 8,
                      VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
      if (*ptr == NULL) {
         fprintf(stderr, "Error: failed to allocate CPU buffer for query.\n");
         v3dv_flag_oom(cmd_buffer, NULL);
         return;
      }

      memcpy(*ptr, old_buffer, prev_slot_count * slot_size);
      *alloc_count = new_slot_count;
   }
   assert(used_count < *alloc_count);
}

void
v3dv_cmd_buffer_begin_query(struct v3dv_cmd_buffer *cmd_buffer,
                            struct v3dv_query_pool *pool,
                            uint32_t query,
                            VkQueryControlFlags flags)
{
   assert(query < pool->query_count);
   switch (pool->query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
      /* FIXME: we only support one active occlusion query for now */
      assert(cmd_buffer->state.query.active_query.bo == NULL);

      cmd_buffer->state.query.active_query.bo = pool->occlusion.bo;
      cmd_buffer->state.query.active_query.offset =
         pool->queries[query].occlusion.offset;
      cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_OCCLUSION_QUERY;
      break;
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      assert(cmd_buffer->state.query.active_query.perf == NULL);
      if (cmd_buffer->state.pass)
         v3dv_cmd_buffer_subpass_finish(cmd_buffer);

      cmd_buffer->state.query.active_query.perf =
         &pool->queries[query].perf;

      if (cmd_buffer->state.pass) {
         v3dv_cmd_buffer_subpass_resume(cmd_buffer,
            cmd_buffer->state.subpass_idx);
      }
      break;
   }
   default:
      unreachable("Unsupported query type");
   }
}

void
v3dv_cmd_buffer_pause_occlusion_query(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   struct v3dv_bo *occlusion_query_bo = state->query.active_query.bo;
   if (occlusion_query_bo) {
      assert(!state->query.active_query.paused_bo);
      state->query.active_query.paused_bo = occlusion_query_bo;
      state->query.active_query.bo = NULL;
      state->dirty |= V3DV_CMD_DIRTY_OCCLUSION_QUERY;
   }
}

void
v3dv_cmd_buffer_resume_occlusion_query(struct v3dv_cmd_buffer *cmd_buffer)
{
   struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
   struct v3dv_bo *occlusion_query_bo = state->query.active_query.paused_bo;
   if (occlusion_query_bo) {
      assert(!state->query.active_query.bo);
      state->query.active_query.bo = occlusion_query_bo;
      state->query.active_query.paused_bo = NULL;
      state->dirty |= V3DV_CMD_DIRTY_OCCLUSION_QUERY;
   }
}

static void
v3dv_cmd_buffer_schedule_end_query(struct v3dv_cmd_buffer *cmd_buffer,
                                   struct v3dv_query_pool *pool,
                                   uint32_t query)
{
   assert(query < pool->query_count);
   assert(pool->query_type == VK_QUERY_TYPE_OCCLUSION ||
          pool->query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR);

   /* For occlusion queries in the middle of a render pass we don't want to
    * split the current job at the EndQuery just to emit query availability,
    * instead we queue this state in the command buffer and we emit it when
    * we finish the current job.
    */
   if  (cmd_buffer->state.pass &&
        pool->query_type == VK_QUERY_TYPE_OCCLUSION) {
      struct v3dv_cmd_buffer_state *state = &cmd_buffer->state;
      v3dv_cmd_buffer_ensure_array_state(cmd_buffer,
                                         sizeof(struct v3dv_end_query_info),
                                         state->query.end.used_count,
                                         &state->query.end.alloc_count,
                                         (void **) &state->query.end.states);
      v3dv_return_if_oom(cmd_buffer, NULL);

      struct v3dv_end_query_info *info =
         &state->query.end.states[state->query.end.used_count++];

      info->pool = pool;
      info->query = query;

      /* From the Vulkan spec:
       *
       *   "If queries are used while executing a render pass instance that has
       *    multiview enabled, the query uses N consecutive query indices in
       *    the query pool (starting at query) where N is the number of bits set
       *    in the view mask in the subpass the query is used in. How the
       *    numerical results of the query are distributed among the queries is
       *    implementation-dependent."
       *
       * In our case, only the first query is used but this means we still need
       * to flag the other queries as available so we don't emit errors when
       * the applications attempt to retrieve values from them.
       */
      struct v3dv_render_pass *pass = cmd_buffer->state.pass;
      if (!pass->multiview_enabled) {
         info->count = 1;
      } else {
         struct v3dv_subpass *subpass = &pass->subpasses[state->subpass_idx];
         info->count = util_bitcount(subpass->view_mask);
      }
   } else {
      /* Otherwise, schedule the end query job immediately.
       *
       * Multiview queries cannot cross subpass boundaries, so query count is
       * always 1.
       */
       if (pool->query_type == VK_QUERY_TYPE_OCCLUSION)
         v3dv_cmd_buffer_emit_set_query_availability(cmd_buffer, pool, query, 1, 1);
       else
         cmd_buffer_emit_end_query_cpu(cmd_buffer, pool, query, 1);
   }
}

static void
v3dv_cmd_buffer_end_occlusion_query(struct v3dv_cmd_buffer *cmd_buffer,
                                    struct v3dv_query_pool *pool,
                                    uint32_t query)
{
   assert(query < pool->query_count);
   assert(cmd_buffer->state.query.active_query.bo != NULL);

   v3dv_cmd_buffer_schedule_end_query(cmd_buffer, pool, query);

   cmd_buffer->state.query.active_query.bo = NULL;
   cmd_buffer->state.dirty |= V3DV_CMD_DIRTY_OCCLUSION_QUERY;
}

static void
v3dv_cmd_buffer_end_performance_query(struct v3dv_cmd_buffer *cmd_buffer,
                                      struct v3dv_query_pool *pool,
                                      uint32_t query)
{
   assert(query < pool->query_count);
   assert(cmd_buffer->state.query.active_query.perf != NULL);

   if (cmd_buffer->state.pass)
      v3dv_cmd_buffer_subpass_finish(cmd_buffer);

   v3dv_cmd_buffer_schedule_end_query(cmd_buffer, pool, query);

   cmd_buffer->state.query.active_query.perf = NULL;

   if (cmd_buffer->state.pass)
      v3dv_cmd_buffer_subpass_resume(cmd_buffer, cmd_buffer->state.subpass_idx);
}

void v3dv_cmd_buffer_end_query(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_query_pool *pool,
                               uint32_t query)
{
   switch (pool->query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
      v3dv_cmd_buffer_end_occlusion_query(cmd_buffer, pool, query);
      break;
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
      v3dv_cmd_buffer_end_performance_query(cmd_buffer, pool, query);
      break;
   default:
      unreachable("Unsupported query type");
   }
}

void
v3dv_cmd_buffer_add_tfu_job(struct v3dv_cmd_buffer *cmd_buffer,
                            struct drm_v3d_submit_tfu *tfu)
{
   struct v3dv_device *device = cmd_buffer->device;
   struct v3dv_job *job = vk_zalloc(&device->vk.alloc,
                                    sizeof(struct v3dv_job), 8,
                                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!job) {
      v3dv_flag_oom(cmd_buffer, NULL);
      return;
   }

   v3dv_job_init(job, V3DV_JOB_TYPE_GPU_TFU, device, cmd_buffer, -1);
   job->tfu = *tfu;
   list_addtail(&job->list_link, &cmd_buffer->jobs);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdWriteTimestamp2(VkCommandBuffer commandBuffer,
                        VkPipelineStageFlags2 stage,
                        VkQueryPool queryPool,
                        uint32_t query)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, query_pool, queryPool);

   /* If this is called inside a render pass we need to finish the current
    * job here...
    */
   struct v3dv_render_pass *pass = cmd_buffer->state.pass;
   if (pass)
      v3dv_cmd_buffer_finish_job(cmd_buffer);

   struct v3dv_job *job =
      v3dv_cmd_buffer_create_cpu_job(cmd_buffer->device,
                                     V3DV_JOB_TYPE_CPU_TIMESTAMP_QUERY,
                                     cmd_buffer, -1);
   v3dv_return_if_oom(cmd_buffer, NULL);

   job->cpu.query_timestamp.pool = query_pool;
   job->cpu.query_timestamp.query = query;

   if (!pass || !pass->multiview_enabled) {
      job->cpu.query_timestamp.count = 1;
   } else {
      struct v3dv_subpass *subpass =
         &pass->subpasses[cmd_buffer->state.subpass_idx];
      job->cpu.query_timestamp.count = util_bitcount(subpass->view_mask);
   }

   list_addtail(&job->list_link, &cmd_buffer->jobs);
   cmd_buffer->state.job = NULL;

   /* ...and resume the subpass after the timestamp */
   if (cmd_buffer->state.pass)
      v3dv_cmd_buffer_subpass_resume(cmd_buffer, cmd_buffer->state.subpass_idx);
}

static void
cmd_buffer_emit_pre_dispatch(struct v3dv_cmd_buffer *cmd_buffer)
{
   assert(cmd_buffer->state.compute.pipeline);
   assert(cmd_buffer->state.compute.pipeline->active_stages ==
          VK_SHADER_STAGE_COMPUTE_BIT);

   cmd_buffer->state.dirty &= ~(V3DV_CMD_DIRTY_COMPUTE_PIPELINE |
                                V3DV_CMD_DIRTY_COMPUTE_DESCRIPTOR_SETS);
   cmd_buffer->state.dirty_descriptor_stages &= ~VK_SHADER_STAGE_COMPUTE_BIT;
   cmd_buffer->state.dirty_push_constants_stages &= ~VK_SHADER_STAGE_COMPUTE_BIT;
}

void
v3dv_cmd_buffer_rewrite_indirect_csd_job(
   struct v3dv_device *device,
   struct v3dv_csd_indirect_cpu_job_info *info,
   const uint32_t *wg_counts)
{
   assert(info->csd_job);
   struct v3dv_job *job = info->csd_job;

   assert(job->type == V3DV_JOB_TYPE_GPU_CSD);
   assert(wg_counts[0] > 0 && wg_counts[1] > 0 && wg_counts[2] > 0);

   struct drm_v3d_submit_csd *submit = &job->csd.submit;

   job->csd.wg_count[0] = wg_counts[0];
   job->csd.wg_count[1] = wg_counts[1];
   job->csd.wg_count[2] = wg_counts[2];

   submit->cfg[0] = wg_counts[0] << V3D_CSD_CFG012_WG_COUNT_SHIFT;
   submit->cfg[1] = wg_counts[1] << V3D_CSD_CFG012_WG_COUNT_SHIFT;
   submit->cfg[2] = wg_counts[2] << V3D_CSD_CFG012_WG_COUNT_SHIFT;

   uint32_t num_batches = DIV_ROUND_UP(info->wg_size, 16) *
                          (wg_counts[0] * wg_counts[1] * wg_counts[2]);
   /* V3D 7.1.6 and later don't subtract 1 from the number of batches */
   if (device->devinfo.ver < 71 ||
       (device->devinfo.ver == 71 && device->devinfo.rev < 6)) {
      submit->cfg[4] = num_batches - 1;
   } else {
      submit->cfg[4] = num_batches;
   }
   assert(submit->cfg[4] != ~0);

   if (info->needs_wg_uniform_rewrite) {
      /* Make sure the GPU is not currently accessing the indirect CL for this
       * job, since we are about to overwrite some of the uniform data.
       */
      v3dv_bo_wait(job->device, job->indirect.bo, OS_TIMEOUT_INFINITE);

      for (uint32_t i = 0; i < 3; i++) {
         if (info->wg_uniform_offsets[i]) {
            /* Sanity check that our uniform pointers are within the allocated
             * BO space for our indirect CL.
             */
            assert(info->wg_uniform_offsets[i] >= (uint32_t *) job->indirect.base);
            assert(info->wg_uniform_offsets[i] < (uint32_t *) job->indirect.next);
            *(info->wg_uniform_offsets[i]) = wg_counts[i];
         }
      }
   }
}

static struct v3dv_job *
cmd_buffer_create_csd_job(struct v3dv_cmd_buffer *cmd_buffer,
                          uint32_t base_offset_x,
                          uint32_t base_offset_y,
                          uint32_t base_offset_z,
                          uint32_t group_count_x,
                          uint32_t group_count_y,
                          uint32_t group_count_z,
                          uint32_t **wg_uniform_offsets_out,
                          uint32_t *wg_size_out)
{
   struct v3dv_device *device = cmd_buffer->device;
   struct v3dv_pipeline *pipeline = cmd_buffer->state.compute.pipeline;
   assert(pipeline && pipeline->shared_data->variants[BROADCOM_SHADER_COMPUTE]);
   struct v3dv_shader_variant *cs_variant =
      pipeline->shared_data->variants[BROADCOM_SHADER_COMPUTE];

   struct v3dv_job *job = vk_zalloc(&cmd_buffer->device->vk.alloc,
                                    sizeof(struct v3dv_job), 8,
                                    VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!job) {
      v3dv_flag_oom(cmd_buffer, NULL);
      return NULL;
   }

   v3dv_job_init(job, V3DV_JOB_TYPE_GPU_CSD, cmd_buffer->device, cmd_buffer, -1);
   cmd_buffer->state.job = job;

   struct drm_v3d_submit_csd *submit = &job->csd.submit;

   job->csd.wg_count[0] = group_count_x;
   job->csd.wg_count[1] = group_count_y;
   job->csd.wg_count[2] = group_count_z;

   job->csd.wg_base[0] = base_offset_x;
   job->csd.wg_base[1] = base_offset_y;
   job->csd.wg_base[2] = base_offset_z;

   submit->cfg[0] |= group_count_x << V3D_CSD_CFG012_WG_COUNT_SHIFT;
   submit->cfg[1] |= group_count_y << V3D_CSD_CFG012_WG_COUNT_SHIFT;
   submit->cfg[2] |= group_count_z << V3D_CSD_CFG012_WG_COUNT_SHIFT;

   const struct v3d_compute_prog_data *cpd =
      cs_variant->prog_data.cs;

   const uint32_t num_wgs = group_count_x * group_count_y * group_count_z;
   const uint32_t wg_size = cpd->local_size[0] *
                            cpd->local_size[1] *
                            cpd->local_size[2];

   uint32_t wgs_per_sg =
      v3d_csd_choose_workgroups_per_supergroup(
         &cmd_buffer->device->devinfo,
         cs_variant->prog_data.cs->has_subgroups,
         cs_variant->prog_data.cs->base.has_control_barrier,
         cs_variant->prog_data.cs->base.threads,
         num_wgs, wg_size);

   uint32_t batches_per_sg = DIV_ROUND_UP(wgs_per_sg * wg_size, 16);
   uint32_t whole_sgs = num_wgs / wgs_per_sg;
   uint32_t rem_wgs = num_wgs - whole_sgs * wgs_per_sg;
   uint32_t num_batches = batches_per_sg * whole_sgs +
                          DIV_ROUND_UP(rem_wgs * wg_size, 16);

   submit->cfg[3] |= (wgs_per_sg & 0xf) << V3D_CSD_CFG3_WGS_PER_SG_SHIFT;
   submit->cfg[3] |= (batches_per_sg - 1) << V3D_CSD_CFG3_BATCHES_PER_SG_M1_SHIFT;
   submit->cfg[3] |= (wg_size & 0xff) << V3D_CSD_CFG3_WG_SIZE_SHIFT;
   if (wg_size_out)
      *wg_size_out = wg_size;

   /* V3D 7.1.6 and later don't subtract 1 from the number of batches */
   if (device->devinfo.ver < 71 ||
       (device->devinfo.ver == 71 && device->devinfo.rev < 6)) {
      submit->cfg[4] = num_batches - 1;
   } else {
      submit->cfg[4] = num_batches;
   }
   assert(submit->cfg[4] != ~0);

   assert(pipeline->shared_data->assembly_bo);
   struct v3dv_bo *cs_assembly_bo = pipeline->shared_data->assembly_bo;

   submit->cfg[5] = cs_assembly_bo->offset + cs_variant->assembly_offset;
   if (cs_variant->prog_data.base->single_seg)
      submit->cfg[5] |= V3D_CSD_CFG5_SINGLE_SEG;
   if (cs_variant->prog_data.base->threads == 4)
      submit->cfg[5] |= V3D_CSD_CFG5_THREADING;
   /* V3D 7.x has made the PROPAGATE_NANS bit in CFG5 reserved  */
   if (device->devinfo.ver < 71)
      submit->cfg[5] |= V3D_CSD_CFG5_PROPAGATE_NANS;

   if (cs_variant->prog_data.cs->shared_size > 0) {
      job->csd.shared_memory =
         v3dv_bo_alloc(cmd_buffer->device,
                       cs_variant->prog_data.cs->shared_size * num_wgs,
                       "shared_vars", true);
      if (!job->csd.shared_memory) {
         v3dv_flag_oom(cmd_buffer, NULL);
         return job;
      }
   }

   v3dv_job_add_bo_unchecked(job, cs_assembly_bo);
   struct v3dv_cl_reloc uniforms =
      v3dv_write_uniforms_wg_offsets(cmd_buffer, pipeline,
                                     cs_variant,
                                     wg_uniform_offsets_out);
   submit->cfg[6] = uniforms.bo->offset + uniforms.offset;


   /* Track VK_KHR_buffer_device_address usage in the job */
   job->uses_buffer_device_address |= pipeline->uses_buffer_device_address;

   v3dv_job_add_bo(job, uniforms.bo);

   return job;
}

static void
cmd_buffer_dispatch(struct v3dv_cmd_buffer *cmd_buffer,
                    uint32_t base_offset_x,
                    uint32_t base_offset_y,
                    uint32_t base_offset_z,
                    uint32_t group_count_x,
                    uint32_t group_count_y,
                    uint32_t group_count_z)
{
   if (group_count_x == 0 || group_count_y == 0 || group_count_z == 0)
      return;

   struct v3dv_job *job =
      cmd_buffer_create_csd_job(cmd_buffer,
                                base_offset_x,
                                base_offset_y,
                                base_offset_z,
                                group_count_x,
                                group_count_y,
                                group_count_z,
                                NULL, NULL);

   list_addtail(&job->list_link, &cmd_buffer->jobs);
   cmd_buffer->state.job = NULL;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdDispatchBase(VkCommandBuffer commandBuffer,
                     uint32_t baseGroupX,
                     uint32_t baseGroupY,
                     uint32_t baseGroupZ,
                     uint32_t groupCountX,
                     uint32_t groupCountY,
                     uint32_t groupCountZ)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);

   cmd_buffer_emit_pre_dispatch(cmd_buffer);
   cmd_buffer_dispatch(cmd_buffer,
                       baseGroupX, baseGroupY, baseGroupZ,
                       groupCountX, groupCountY, groupCountZ);
}


static void
cmd_buffer_dispatch_indirect(struct v3dv_cmd_buffer *cmd_buffer,
                             struct v3dv_buffer *buffer,
                             uint32_t offset)
{
   /* We can't do indirect dispatches, so instead we record a CPU job that,
    * when executed in the queue, will map the indirect buffer, read the
    * dispatch parameters, and submit a regular dispatch.
    */
   struct v3dv_job *job =
      v3dv_cmd_buffer_create_cpu_job(cmd_buffer->device,
                                     V3DV_JOB_TYPE_CPU_CSD_INDIRECT,
                                     cmd_buffer, -1);
   v3dv_return_if_oom(cmd_buffer, NULL);

   /* We need to create a CSD job now, even if we still don't know the actual
    * dispatch parameters, because the job setup needs to be done using the
    * current command buffer state (i.e. pipeline, descriptor sets, push
    * constants, etc.). So we create the job with default dispatch parameters
    * and we will rewrite the parts we need at submit time if the indirect
    * parameters don't match the ones we used to setup the job.
    */
   struct v3dv_job *csd_job =
      cmd_buffer_create_csd_job(cmd_buffer,
                                0, 0, 0,
                                1, 1, 1,
                                &job->cpu.csd_indirect.wg_uniform_offsets[0],
                                &job->cpu.csd_indirect.wg_size);
   v3dv_return_if_oom(cmd_buffer, NULL);
   assert(csd_job);

   job->cpu.csd_indirect.buffer = buffer;
   job->cpu.csd_indirect.offset = offset;
   job->cpu.csd_indirect.csd_job = csd_job;

   /* If the compute shader reads the workgroup sizes we will also need to
    * rewrite the corresponding uniforms.
    */
   job->cpu.csd_indirect.needs_wg_uniform_rewrite =
      job->cpu.csd_indirect.wg_uniform_offsets[0] ||
      job->cpu.csd_indirect.wg_uniform_offsets[1] ||
      job->cpu.csd_indirect.wg_uniform_offsets[2];

   list_addtail(&job->list_link, &cmd_buffer->jobs);

   /* If we have a CPU queue we submit the CPU job directly to the
    * queue and the CSD job will be dispatched from within the kernel
    * queue, otherwise we will have to dispatch the CSD job manually
    * right after the CPU job by adding it to the list of jobs in the
    * command buffer.
    */
   if (!cmd_buffer->device->pdevice->caps.cpu_queue)
      list_addtail(&csd_job->list_link, &cmd_buffer->jobs);

   cmd_buffer->state.job = NULL;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdDispatchIndirect(VkCommandBuffer commandBuffer,
                         VkBuffer _buffer,
                         VkDeviceSize offset)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_buffer, buffer, _buffer);

   assert(offset <= UINT32_MAX);

   cmd_buffer_emit_pre_dispatch(cmd_buffer);
   cmd_buffer_dispatch_indirect(cmd_buffer, buffer, offset);
}
