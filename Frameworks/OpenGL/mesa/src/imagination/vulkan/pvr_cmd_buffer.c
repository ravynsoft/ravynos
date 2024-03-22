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

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_defs.h"
#include "hwdef/rogue_hw_utils.h"
#include "pvr_blit.h"
#include "pvr_bo.h"
#include "pvr_clear.h"
#include "pvr_common.h"
#include "pvr_csb.h"
#include "pvr_csb_enum_helpers.h"
#include "pvr_device_info.h"
#include "pvr_formats.h"
#include "pvr_hardcode.h"
#include "pvr_hw_pass.h"
#include "pvr_job_common.h"
#include "pvr_job_render.h"
#include "pvr_limits.h"
#include "pvr_pds.h"
#include "pvr_private.h"
#include "pvr_tex_state.h"
#include "pvr_types.h"
#include "pvr_uscgen.h"
#include "pvr_winsys.h"
#include "util/bitscan.h"
#include "util/bitset.h"
#include "util/compiler.h"
#include "util/list.h"
#include "util/macros.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"
#include "util/u_pack_color.h"
#include "vk_alloc.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_common_entrypoints.h"
#include "vk_format.h"
#include "vk_graphics_state.h"
#include "vk_log.h"
#include "vk_object.h"
#include "vk_util.h"

/* Structure used to pass data into pvr_compute_generate_control_stream()
 * function.
 */
struct pvr_compute_kernel_info {
   pvr_dev_addr_t indirect_buffer_addr;
   bool global_offsets_present;
   uint32_t usc_common_size;
   uint32_t usc_unified_size;
   uint32_t pds_temp_size;
   uint32_t pds_data_size;
   enum PVRX(CDMCTRL_USC_TARGET) usc_target;
   bool is_fence;
   uint32_t pds_data_offset;
   uint32_t pds_code_offset;
   enum PVRX(CDMCTRL_SD_TYPE) sd_type;
   bool usc_common_shared;
   uint32_t local_size[PVR_WORKGROUP_DIMENSIONS];
   uint32_t global_size[PVR_WORKGROUP_DIMENSIONS];
   uint32_t max_instances;
};

static void pvr_cmd_buffer_free_sub_cmd(struct pvr_cmd_buffer *cmd_buffer,
                                        struct pvr_sub_cmd *sub_cmd)
{
   if (sub_cmd->owned) {
      switch (sub_cmd->type) {
      case PVR_SUB_CMD_TYPE_GRAPHICS:
         util_dynarray_fini(&sub_cmd->gfx.sec_query_indices);
         pvr_csb_finish(&sub_cmd->gfx.control_stream);
         pvr_bo_free(cmd_buffer->device, sub_cmd->gfx.terminate_ctrl_stream);
         pvr_bo_suballoc_free(sub_cmd->gfx.depth_bias_bo);
         pvr_bo_suballoc_free(sub_cmd->gfx.scissor_bo);
         break;

      case PVR_SUB_CMD_TYPE_COMPUTE:
      case PVR_SUB_CMD_TYPE_OCCLUSION_QUERY:
         pvr_csb_finish(&sub_cmd->compute.control_stream);
         break;

      case PVR_SUB_CMD_TYPE_TRANSFER:
         list_for_each_entry_safe (struct pvr_transfer_cmd,
                                   transfer_cmd,
                                   sub_cmd->transfer.transfer_cmds,
                                   link) {
            list_del(&transfer_cmd->link);
            if (!transfer_cmd->is_deferred_clear)
               vk_free(&cmd_buffer->vk.pool->alloc, transfer_cmd);
         }
         break;

      case PVR_SUB_CMD_TYPE_EVENT:
         if (sub_cmd->event.type == PVR_EVENT_TYPE_WAIT)
            vk_free(&cmd_buffer->vk.pool->alloc, sub_cmd->event.wait.events);
         break;

      default:
         unreachable("Unsupported sub-command type");
      }
   }

   list_del(&sub_cmd->link);
   vk_free(&cmd_buffer->vk.pool->alloc, sub_cmd);
}

static void pvr_cmd_buffer_free_sub_cmds(struct pvr_cmd_buffer *cmd_buffer)
{
   list_for_each_entry_safe (struct pvr_sub_cmd,
                             sub_cmd,
                             &cmd_buffer->sub_cmds,
                             link) {
      pvr_cmd_buffer_free_sub_cmd(cmd_buffer, sub_cmd);
   }
}

static void pvr_cmd_buffer_free_resources(struct pvr_cmd_buffer *cmd_buffer)
{
   vk_free(&cmd_buffer->vk.pool->alloc,
           cmd_buffer->state.render_pass_info.attachments);
   vk_free(&cmd_buffer->vk.pool->alloc,
           cmd_buffer->state.render_pass_info.clear_values);

   util_dynarray_fini(&cmd_buffer->state.query_indices);

   pvr_cmd_buffer_free_sub_cmds(cmd_buffer);

   list_for_each_entry_safe (struct pvr_suballoc_bo,
                             suballoc_bo,
                             &cmd_buffer->bo_list,
                             link) {
      list_del(&suballoc_bo->link);
      pvr_bo_suballoc_free(suballoc_bo);
   }

   util_dynarray_fini(&cmd_buffer->deferred_clears);
   util_dynarray_fini(&cmd_buffer->deferred_csb_commands);
   util_dynarray_fini(&cmd_buffer->scissor_array);
   util_dynarray_fini(&cmd_buffer->depth_bias_array);
}

static void pvr_cmd_buffer_reset(struct vk_command_buffer *vk_cmd_buffer,
                                 VkCommandBufferResetFlags flags)
{
   struct pvr_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct pvr_cmd_buffer, vk);

   /* FIXME: For now we always free all resources as if
    * VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT was set.
    */
   pvr_cmd_buffer_free_resources(cmd_buffer);

   vk_command_buffer_reset(&cmd_buffer->vk);

   memset(&cmd_buffer->state, 0, sizeof(cmd_buffer->state));
   memset(&cmd_buffer->scissor_words, 0, sizeof(cmd_buffer->scissor_words));

   cmd_buffer->usage_flags = 0;
}

static void pvr_cmd_buffer_destroy(struct vk_command_buffer *vk_cmd_buffer)
{
   struct pvr_cmd_buffer *cmd_buffer =
      container_of(vk_cmd_buffer, struct pvr_cmd_buffer, vk);

   pvr_cmd_buffer_free_resources(cmd_buffer);
   vk_command_buffer_finish(&cmd_buffer->vk);
   vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer);
}

static const struct vk_command_buffer_ops cmd_buffer_ops = {
   .reset = pvr_cmd_buffer_reset,
   .destroy = pvr_cmd_buffer_destroy,
};

static VkResult pvr_cmd_buffer_create(struct pvr_device *device,
                                      struct vk_command_pool *pool,
                                      VkCommandBufferLevel level,
                                      VkCommandBuffer *pCommandBuffer)
{
   struct pvr_cmd_buffer *cmd_buffer;
   VkResult result;

   cmd_buffer = vk_zalloc(&pool->alloc,
                          sizeof(*cmd_buffer),
                          8U,
                          VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   result =
      vk_command_buffer_init(pool, &cmd_buffer->vk, &cmd_buffer_ops, level);
   if (result != VK_SUCCESS) {
      vk_free(&pool->alloc, cmd_buffer);
      return result;
   }

   cmd_buffer->device = device;

   util_dynarray_init(&cmd_buffer->depth_bias_array, NULL);
   util_dynarray_init(&cmd_buffer->scissor_array, NULL);
   util_dynarray_init(&cmd_buffer->deferred_csb_commands, NULL);
   util_dynarray_init(&cmd_buffer->deferred_clears, NULL);

   list_inithead(&cmd_buffer->sub_cmds);
   list_inithead(&cmd_buffer->bo_list);

   *pCommandBuffer = pvr_cmd_buffer_to_handle(cmd_buffer);

   return VK_SUCCESS;
}

VkResult
pvr_AllocateCommandBuffers(VkDevice _device,
                           const VkCommandBufferAllocateInfo *pAllocateInfo,
                           VkCommandBuffer *pCommandBuffers)
{
   VK_FROM_HANDLE(vk_command_pool, pool, pAllocateInfo->commandPool);
   PVR_FROM_HANDLE(pvr_device, device, _device);
   VkResult result = VK_SUCCESS;
   uint32_t i;

   for (i = 0; i < pAllocateInfo->commandBufferCount; i++) {
      result = pvr_cmd_buffer_create(device,
                                     pool,
                                     pAllocateInfo->level,
                                     &pCommandBuffers[i]);
      if (result != VK_SUCCESS)
         break;
   }

   if (result != VK_SUCCESS) {
      while (i--) {
         VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, pCommandBuffers[i]);
         pvr_cmd_buffer_destroy(cmd_buffer);
      }

      for (i = 0; i < pAllocateInfo->commandBufferCount; i++)
         pCommandBuffers[i] = VK_NULL_HANDLE;
   }

   return result;
}

static void pvr_cmd_buffer_update_barriers(struct pvr_cmd_buffer *cmd_buffer,
                                           enum pvr_sub_cmd_type type)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   uint32_t barriers;

   switch (type) {
   case PVR_SUB_CMD_TYPE_GRAPHICS:
      barriers = PVR_PIPELINE_STAGE_GEOM_BIT | PVR_PIPELINE_STAGE_FRAG_BIT;
      break;

   case PVR_SUB_CMD_TYPE_COMPUTE:
      barriers = PVR_PIPELINE_STAGE_COMPUTE_BIT;
      break;

   case PVR_SUB_CMD_TYPE_OCCLUSION_QUERY:
   case PVR_SUB_CMD_TYPE_TRANSFER:
      /* Compute jobs are used for occlusion queries but to copy the results we
       * have to sync with transfer jobs because vkCmdCopyQueryPoolResults() is
       * deemed as a transfer operation by the spec.
       */
      barriers = PVR_PIPELINE_STAGE_TRANSFER_BIT;
      break;

   case PVR_SUB_CMD_TYPE_EVENT:
      barriers = 0;
      break;

   default:
      unreachable("Unsupported sub-command type");
   }

   for (uint32_t i = 0; i < ARRAY_SIZE(state->barriers_needed); i++)
      state->barriers_needed[i] |= barriers;
}

static VkResult
pvr_cmd_buffer_upload_tables(struct pvr_device *device,
                             struct pvr_cmd_buffer *cmd_buffer,
                             struct pvr_sub_cmd_gfx *const sub_cmd)
{
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
   VkResult result;

   assert(!sub_cmd->depth_bias_bo && !sub_cmd->scissor_bo);

   if (cmd_buffer->depth_bias_array.size > 0) {
      result =
         pvr_gpu_upload(device,
                        device->heaps.general_heap,
                        util_dynarray_begin(&cmd_buffer->depth_bias_array),
                        cmd_buffer->depth_bias_array.size,
                        cache_line_size,
                        &sub_cmd->depth_bias_bo);
      if (result != VK_SUCCESS)
         return result;
   }

   if (cmd_buffer->scissor_array.size > 0) {
      result = pvr_gpu_upload(device,
                              device->heaps.general_heap,
                              util_dynarray_begin(&cmd_buffer->scissor_array),
                              cmd_buffer->scissor_array.size,
                              cache_line_size,
                              &sub_cmd->scissor_bo);
      if (result != VK_SUCCESS)
         goto err_free_depth_bias_bo;
   }

   util_dynarray_clear(&cmd_buffer->depth_bias_array);
   util_dynarray_clear(&cmd_buffer->scissor_array);

   return VK_SUCCESS;

err_free_depth_bias_bo:
   pvr_bo_suballoc_free(sub_cmd->depth_bias_bo);
   sub_cmd->depth_bias_bo = NULL;

   return result;
}

static VkResult
pvr_cmd_buffer_emit_ppp_state(const struct pvr_cmd_buffer *const cmd_buffer,
                              struct pvr_csb *const csb)
{
   const struct pvr_framebuffer *const framebuffer =
      cmd_buffer->state.render_pass_info.framebuffer;

   assert(csb->stream_type == PVR_CMD_STREAM_TYPE_GRAPHICS ||
          csb->stream_type == PVR_CMD_STREAM_TYPE_GRAPHICS_DEFERRED);

   pvr_csb_set_relocation_mark(csb);

   pvr_csb_emit (csb, VDMCTRL_PPP_STATE0, state0) {
      state0.addrmsb = framebuffer->ppp_state_bo->dev_addr;
      state0.word_count = framebuffer->ppp_state_size;
   }

   pvr_csb_emit (csb, VDMCTRL_PPP_STATE1, state1) {
      state1.addrlsb = framebuffer->ppp_state_bo->dev_addr;
   }

   pvr_csb_clear_relocation_mark(csb);

   return csb->status;
}

VkResult
pvr_cmd_buffer_upload_general(struct pvr_cmd_buffer *const cmd_buffer,
                              const void *const data,
                              const size_t size,
                              struct pvr_suballoc_bo **const pvr_bo_out)
{
   struct pvr_device *const device = cmd_buffer->device;
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
   struct pvr_suballoc_bo *suballoc_bo;
   VkResult result;

   result = pvr_gpu_upload(device,
                           device->heaps.general_heap,
                           data,
                           size,
                           cache_line_size,
                           &suballoc_bo);
   if (result != VK_SUCCESS)
      return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

   list_add(&suballoc_bo->link, &cmd_buffer->bo_list);

   *pvr_bo_out = suballoc_bo;

   return VK_SUCCESS;
}

static VkResult
pvr_cmd_buffer_upload_usc(struct pvr_cmd_buffer *const cmd_buffer,
                          const void *const code,
                          const size_t code_size,
                          uint64_t code_alignment,
                          struct pvr_suballoc_bo **const pvr_bo_out)
{
   struct pvr_device *const device = cmd_buffer->device;
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
   struct pvr_suballoc_bo *suballoc_bo;
   VkResult result;

   code_alignment = MAX2(code_alignment, cache_line_size);

   result =
      pvr_gpu_upload_usc(device, code, code_size, code_alignment, &suballoc_bo);
   if (result != VK_SUCCESS)
      return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

   list_add(&suballoc_bo->link, &cmd_buffer->bo_list);

   *pvr_bo_out = suballoc_bo;

   return VK_SUCCESS;
}

VkResult pvr_cmd_buffer_upload_pds(struct pvr_cmd_buffer *const cmd_buffer,
                                   const uint32_t *data,
                                   uint32_t data_size_dwords,
                                   uint32_t data_alignment,
                                   const uint32_t *code,
                                   uint32_t code_size_dwords,
                                   uint32_t code_alignment,
                                   uint64_t min_alignment,
                                   struct pvr_pds_upload *const pds_upload_out)
{
   struct pvr_device *const device = cmd_buffer->device;
   VkResult result;

   result = pvr_gpu_upload_pds(device,
                               data,
                               data_size_dwords,
                               data_alignment,
                               code,
                               code_size_dwords,
                               code_alignment,
                               min_alignment,
                               pds_upload_out);
   if (result != VK_SUCCESS)
      return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

   list_add(&pds_upload_out->pvr_bo->link, &cmd_buffer->bo_list);

   return VK_SUCCESS;
}

static inline VkResult
pvr_cmd_buffer_upload_pds_data(struct pvr_cmd_buffer *const cmd_buffer,
                               const uint32_t *data,
                               uint32_t data_size_dwords,
                               uint32_t data_alignment,
                               struct pvr_pds_upload *const pds_upload_out)
{
   return pvr_cmd_buffer_upload_pds(cmd_buffer,
                                    data,
                                    data_size_dwords,
                                    data_alignment,
                                    NULL,
                                    0,
                                    0,
                                    data_alignment,
                                    pds_upload_out);
}

/* pbe_cs_words must be an array of length emit_count with
 * ROGUE_NUM_PBESTATE_STATE_WORDS entries
 */
static VkResult pvr_sub_cmd_gfx_per_job_fragment_programs_create_and_upload(
   struct pvr_cmd_buffer *const cmd_buffer,
   const uint32_t emit_count,
   const uint32_t *pbe_cs_words,
   struct pvr_pds_upload *const pds_upload_out)
{
   struct pvr_pds_event_program pixel_event_program = {
      /* No data to DMA, just a DOUTU needed. */
      .num_emit_word_pairs = 0,
   };
   const uint32_t staging_buffer_size =
      PVR_DW_TO_BYTES(cmd_buffer->device->pixel_event_data_size_in_dwords);
   const VkAllocationCallbacks *const allocator = &cmd_buffer->vk.pool->alloc;
   struct pvr_device *const device = cmd_buffer->device;
   struct pvr_suballoc_bo *usc_eot_program = NULL;
   struct util_dynarray eot_program_bin;
   uint32_t *staging_buffer;
   uint32_t usc_temp_count;
   VkResult result;

   assert(emit_count > 0);

   pvr_uscgen_eot("per-job EOT",
                  emit_count,
                  pbe_cs_words,
                  &usc_temp_count,
                  &eot_program_bin);

   result = pvr_cmd_buffer_upload_usc(cmd_buffer,
                                      eot_program_bin.data,
                                      eot_program_bin.size,
                                      4,
                                      &usc_eot_program);

   util_dynarray_fini(&eot_program_bin);

   if (result != VK_SUCCESS)
      return result;

   pvr_pds_setup_doutu(&pixel_event_program.task_control,
                       usc_eot_program->dev_addr.addr,
                       usc_temp_count,
                       PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);

   /* TODO: We could skip allocating this and generate directly into the device
    * buffer thus removing one allocation and memcpy() per job. Would this
    * speed up things in a noticeable way?
    */
   staging_buffer = vk_alloc(allocator,
                             staging_buffer_size,
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_free_usc_pixel_program;
   }

   /* Generate the data segment. The code segment was uploaded earlier when
    * setting up the PDS static heap data.
    */
   pvr_pds_generate_pixel_event_data_segment(&pixel_event_program,
                                             staging_buffer,
                                             &device->pdevice->dev_info);

   result = pvr_cmd_buffer_upload_pds_data(
      cmd_buffer,
      staging_buffer,
      cmd_buffer->device->pixel_event_data_size_in_dwords,
      4,
      pds_upload_out);
   if (result != VK_SUCCESS)
      goto err_free_pixel_event_staging_buffer;

   vk_free(allocator, staging_buffer);

   return VK_SUCCESS;

err_free_pixel_event_staging_buffer:
   vk_free(allocator, staging_buffer);

err_free_usc_pixel_program:
   list_del(&usc_eot_program->link);
   pvr_bo_suballoc_free(usc_eot_program);

   return result;
}

static VkResult pvr_sub_cmd_gfx_build_terminate_ctrl_stream(
   struct pvr_device *const device,
   const struct pvr_cmd_buffer *const cmd_buffer,
   struct pvr_sub_cmd_gfx *const gfx_sub_cmd)
{
   struct list_head bo_list;
   struct pvr_csb csb;
   VkResult result;

   pvr_csb_init(device, PVR_CMD_STREAM_TYPE_GRAPHICS, &csb);

   result = pvr_cmd_buffer_emit_ppp_state(cmd_buffer, &csb);
   if (result != VK_SUCCESS)
      goto err_csb_finish;

   result = pvr_csb_emit_terminate(&csb);
   if (result != VK_SUCCESS)
      goto err_csb_finish;

   result = pvr_csb_bake(&csb, &bo_list);
   if (result != VK_SUCCESS)
      goto err_csb_finish;

   /* This is a trivial control stream, there's no reason it should ever require
    * more memory than a single bo can provide.
    */
   assert(list_is_singular(&bo_list));
   gfx_sub_cmd->terminate_ctrl_stream =
      list_first_entry(&bo_list, struct pvr_bo, link);

   return VK_SUCCESS;

err_csb_finish:
   pvr_csb_finish(&csb);

   return result;
}

static VkResult pvr_setup_texture_state_words(
   struct pvr_device *device,
   struct pvr_combined_image_sampler_descriptor *descriptor,
   const struct pvr_image_view *image_view)
{
   const struct pvr_image *image = vk_to_pvr_image(image_view->vk.image);
   struct pvr_texture_state_info info = {
      .format = image_view->vk.format,
      .mem_layout = image->memlayout,
      .type = image_view->vk.view_type,
      .is_cube = image_view->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE ||
                 image_view->vk.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
      .tex_state_type = PVR_TEXTURE_STATE_SAMPLE,
      .extent = image_view->vk.extent,
      .mip_levels = 1,
      .sample_count = image_view->vk.image->samples,
      .stride = image->physical_extent.width,
      .addr = image->dev_addr,
   };
   const uint8_t *const swizzle = pvr_get_format_swizzle(info.format);
   VkResult result;

   memcpy(&info.swizzle, swizzle, sizeof(info.swizzle));

   /* TODO: Can we use image_view->texture_state instead of generating here? */
   result = pvr_pack_tex_state(device, &info, descriptor->image);
   if (result != VK_SUCCESS)
      return result;

   descriptor->sampler = (union pvr_sampler_descriptor){ 0 };

   pvr_csb_pack (&descriptor->sampler.data.sampler_word,
                 TEXSTATE_SAMPLER,
                 sampler) {
      sampler.non_normalized_coords = true;
      sampler.addrmode_v = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
      sampler.addrmode_u = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
      sampler.minfilter = PVRX(TEXSTATE_FILTER_POINT);
      sampler.magfilter = PVRX(TEXSTATE_FILTER_POINT);
      sampler.dadjust = PVRX(TEXSTATE_DADJUST_ZERO_UINT);
   }

   return VK_SUCCESS;
}

static VkResult
pvr_load_op_constants_create_and_upload(struct pvr_cmd_buffer *cmd_buffer,
                                        const struct pvr_load_op *load_op,
                                        pvr_dev_addr_t *const addr_out)
{
   const struct pvr_render_pass_info *render_pass_info =
      &cmd_buffer->state.render_pass_info;
   const struct pvr_render_pass *pass = render_pass_info->pass;
   const struct pvr_renderpass_hwsetup_render *hw_render = load_op->hw_render;
   const struct pvr_renderpass_colorinit *color_init =
      &hw_render->color_init[0];
   const VkClearValue *clear_value =
      &render_pass_info->clear_values[color_init->index];
   struct pvr_suballoc_bo *clear_bo;
   uint32_t attachment_count;
   bool has_depth_clear;
   bool has_depth_load;
   VkResult result;

   /* These are only setup and never used for now. These will need to be
    * uploaded into a buffer based on some compiler info.
    */
   /* TODO: Remove the above comment once the compiler is hooked up and we're
    * setting up + uploading the buffer.
    */
   struct pvr_combined_image_sampler_descriptor
      texture_states[PVR_LOAD_OP_CLEARS_LOADS_MAX_RTS];
   uint32_t texture_count = 0;
   uint32_t hw_clear_value[PVR_LOAD_OP_CLEARS_LOADS_MAX_RTS *
                           PVR_CLEAR_COLOR_ARRAY_SIZE];
   uint32_t next_clear_consts = 0;

   if (load_op->is_hw_object)
      attachment_count = load_op->hw_render->color_init_count;
   else
      attachment_count = load_op->subpass->color_count;

   for (uint32_t i = 0; i < attachment_count; i++) {
      struct pvr_image_view *image_view;
      uint32_t attachment_idx;

      if (load_op->is_hw_object)
         attachment_idx = load_op->hw_render->color_init[i].index;
      else
         attachment_idx = load_op->subpass->color_attachments[i];

      image_view = render_pass_info->attachments[attachment_idx];

      assert((load_op->clears_loads_state.rt_load_mask &
              load_op->clears_loads_state.rt_clear_mask) == 0);
      if (load_op->clears_loads_state.rt_load_mask & BITFIELD_BIT(i)) {
         result = pvr_setup_texture_state_words(cmd_buffer->device,
                                                &texture_states[texture_count],
                                                image_view);
         if (result != VK_SUCCESS)
            return result;

         texture_count++;
      } else if (load_op->clears_loads_state.rt_clear_mask & BITFIELD_BIT(i)) {
         const uint32_t accum_fmt_size =
            pvr_get_pbe_accum_format_size_in_bytes(image_view->vk.format);

         assert(next_clear_consts +
                   vk_format_get_blocksize(image_view->vk.format) <=
                ARRAY_SIZE(hw_clear_value));

         /* FIXME: do this at the point we store the clear values? */
         pvr_get_hw_clear_color(image_view->vk.format,
                                clear_value->color,
                                &hw_clear_value[next_clear_consts]);

         next_clear_consts += DIV_ROUND_UP(accum_fmt_size, sizeof(uint32_t));
      }
   }

   has_depth_load = false;
   for (uint32_t i = 0;
        i < ARRAY_SIZE(load_op->clears_loads_state.dest_vk_format);
        i++) {
      if (load_op->clears_loads_state.dest_vk_format[i] ==
          VK_FORMAT_D32_SFLOAT) {
         has_depth_load = true;
         break;
      }
   }

   has_depth_clear = load_op->clears_loads_state.depth_clear_to_reg != -1;

   assert(!(has_depth_clear && has_depth_load));

   if (has_depth_load) {
      const struct pvr_render_pass_attachment *attachment;
      const struct pvr_image_view *image_view;

      assert(load_op->subpass->depth_stencil_attachment !=
             VK_ATTACHMENT_UNUSED);
      assert(!load_op->is_hw_object);
      attachment =
         &pass->attachments[load_op->subpass->depth_stencil_attachment];

      image_view = render_pass_info->attachments[attachment->index];

      result = pvr_setup_texture_state_words(cmd_buffer->device,
                                             &texture_states[texture_count],
                                             image_view);
      if (result != VK_SUCCESS)
         return result;

      texture_count++;
   } else if (has_depth_clear) {
      const struct pvr_render_pass_attachment *attachment;
      VkClearValue clear_value;

      assert(load_op->subpass->depth_stencil_attachment !=
             VK_ATTACHMENT_UNUSED);
      attachment =
         &pass->attachments[load_op->subpass->depth_stencil_attachment];

      clear_value = render_pass_info->clear_values[attachment->index];

      assert(next_clear_consts < ARRAY_SIZE(hw_clear_value));
      hw_clear_value[next_clear_consts++] = fui(clear_value.depthStencil.depth);
   }

   result = pvr_cmd_buffer_upload_general(cmd_buffer,
                                          &hw_clear_value[0],
                                          sizeof(hw_clear_value),
                                          &clear_bo);
   if (result != VK_SUCCESS)
      return result;

   *addr_out = clear_bo->dev_addr;

   return VK_SUCCESS;
}

static VkResult pvr_load_op_pds_data_create_and_upload(
   struct pvr_cmd_buffer *cmd_buffer,
   const struct pvr_load_op *load_op,
   pvr_dev_addr_t constants_addr,
   struct pvr_pds_upload *const pds_upload_out)
{
   struct pvr_device *device = cmd_buffer->device;
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   struct pvr_pds_pixel_shader_sa_program program = { 0 };
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   program.num_texture_dma_kicks = 1;

   pvr_csb_pack (&program.texture_dma_address[0],
                 PDSINST_DOUT_FIELDS_DOUTD_SRC0,
                 value) {
      value.sbase = constants_addr;
   }

   pvr_csb_pack (&program.texture_dma_control[0],
                 PDSINST_DOUT_FIELDS_DOUTD_SRC1,
                 value) {
      value.dest = PVRX(PDSINST_DOUTD_DEST_COMMON_STORE);
      value.a0 = load_op->shareds_dest_offset;
      value.bsize = load_op->shareds_count;
   }

   pvr_pds_set_sizes_pixel_shader_sa_texture_data(&program, dev_info);

   staging_buffer_size = PVR_DW_TO_BYTES(program.data_size);

   staging_buffer = vk_alloc(&cmd_buffer->vk.pool->alloc,
                             staging_buffer_size,
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pvr_pds_generate_pixel_shader_sa_texture_state_data(&program,
                                                       staging_buffer,
                                                       dev_info);

   result = pvr_cmd_buffer_upload_pds_data(cmd_buffer,
                                           staging_buffer,
                                           program.data_size,
                                           1,
                                           pds_upload_out);
   if (result != VK_SUCCESS) {
      vk_free(&cmd_buffer->vk.pool->alloc, staging_buffer);
      return result;
   }

   vk_free(&cmd_buffer->vk.pool->alloc, staging_buffer);

   return VK_SUCCESS;
}

/* FIXME: Should this function be specific to the HW background object, in
 * which case its name should be changed, or should it have the load op
 * structure passed in?
 */
static VkResult
pvr_load_op_data_create_and_upload(struct pvr_cmd_buffer *cmd_buffer,
                                   const struct pvr_load_op *load_op,
                                   struct pvr_pds_upload *const pds_upload_out)
{
   pvr_dev_addr_t constants_addr;
   VkResult result;

   result = pvr_load_op_constants_create_and_upload(cmd_buffer,
                                                    load_op,
                                                    &constants_addr);
   if (result != VK_SUCCESS)
      return result;

   return pvr_load_op_pds_data_create_and_upload(cmd_buffer,
                                                 load_op,
                                                 constants_addr,
                                                 pds_upload_out);
}

static void pvr_pds_bgnd_pack_state(
   const struct pvr_load_op *load_op,
   const struct pvr_pds_upload *load_op_program,
   uint64_t pds_reg_values[static const ROGUE_NUM_CR_PDS_BGRND_WORDS])
{
   pvr_csb_pack (&pds_reg_values[0], CR_PDS_BGRND0_BASE, value) {
      value.shader_addr = PVR_DEV_ADDR(load_op->pds_frag_prog.data_offset);
      value.texunicode_addr =
         PVR_DEV_ADDR(load_op->pds_tex_state_prog.code_offset);
   }

   pvr_csb_pack (&pds_reg_values[1], CR_PDS_BGRND1_BASE, value) {
      value.texturedata_addr = PVR_DEV_ADDR(load_op_program->data_offset);
   }

   pvr_csb_pack (&pds_reg_values[2], CR_PDS_BGRND3_SIZEINFO, value) {
      value.usc_sharedsize =
         DIV_ROUND_UP(load_op->const_shareds_count,
                      PVRX(CR_PDS_BGRND3_SIZEINFO_USC_SHAREDSIZE_UNIT_SIZE));
      value.pds_texturestatesize = DIV_ROUND_UP(
         load_op_program->data_size,
         PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_TEXTURESTATESIZE_UNIT_SIZE));
      value.pds_tempsize =
         DIV_ROUND_UP(load_op->temps_count,
                      PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_TEMPSIZE_UNIT_SIZE));
   }
}

/**
 * \brief Calculates the stride in pixels based on the pitch in bytes and pixel
 * format.
 *
 * \param[in] pitch     Width pitch in bytes.
 * \param[in] vk_format Vulkan image format.
 * \return Stride in pixels.
 */
static inline uint32_t pvr_stride_from_pitch(uint32_t pitch, VkFormat vk_format)
{
   const unsigned int cpp = vk_format_get_blocksize(vk_format);

   assert(pitch % cpp == 0);

   return pitch / cpp;
}

static void pvr_setup_pbe_state(
   const struct pvr_device_info *dev_info,
   const struct pvr_framebuffer *framebuffer,
   uint32_t mrt_index,
   const struct usc_mrt_resource *mrt_resource,
   const struct pvr_image_view *const iview,
   const VkRect2D *render_area,
   const bool down_scale,
   const uint32_t samples,
   uint32_t pbe_cs_words[static const ROGUE_NUM_PBESTATE_STATE_WORDS],
   uint64_t pbe_reg_words[static const ROGUE_NUM_PBESTATE_REG_WORDS])
{
   const struct pvr_image *image = pvr_image_view_get_image(iview);
   uint32_t level_pitch = image->mip_levels[iview->vk.base_mip_level].pitch;

   struct pvr_pbe_surf_params surface_params;
   struct pvr_pbe_render_params render_params;
   bool with_packed_usc_channel;
   const uint8_t *swizzle;
   uint32_t position;

   /* down_scale should be true when performing a resolve, in which case there
    * should be more than one sample.
    */
   assert((down_scale && samples > 1U) || (!down_scale && samples == 1U));

   /* Setup surface parameters. */

   if (PVR_HAS_FEATURE(dev_info, usc_f16sop_u8)) {
      with_packed_usc_channel = vk_format_is_unorm(iview->vk.format) ||
                                vk_format_is_snorm(iview->vk.format);
   } else {
      with_packed_usc_channel = false;
   }

   swizzle = pvr_get_format_swizzle(iview->vk.format);
   memcpy(surface_params.swizzle, swizzle, sizeof(surface_params.swizzle));

   pvr_pbe_get_src_format_and_gamma(iview->vk.format,
                                    PVR_PBE_GAMMA_NONE,
                                    with_packed_usc_channel,
                                    &surface_params.source_format,
                                    &surface_params.gamma);

   surface_params.is_normalized = vk_format_is_normalized(iview->vk.format);
   surface_params.pbe_packmode = pvr_get_pbe_packmode(iview->vk.format);
   surface_params.nr_components = vk_format_get_nr_components(iview->vk.format);

   /* FIXME: Should we have an inline function to return the address of a mip
    * level?
    */
   surface_params.addr =
      PVR_DEV_ADDR_OFFSET(image->vma->dev_addr,
                          image->mip_levels[iview->vk.base_mip_level].offset);
   surface_params.addr =
      PVR_DEV_ADDR_OFFSET(surface_params.addr,
                          iview->vk.base_array_layer * image->layer_size);

   surface_params.mem_layout = image->memlayout;
   surface_params.stride = pvr_stride_from_pitch(level_pitch, iview->vk.format);
   surface_params.depth = iview->vk.extent.depth;
   surface_params.width = iview->vk.extent.width;
   surface_params.height = iview->vk.extent.height;
   surface_params.z_only_render = false;
   surface_params.down_scale = down_scale;

   /* Setup render parameters. */

   if (mrt_resource->type == USC_MRT_RESOURCE_TYPE_MEMORY) {
      position = mrt_resource->mem.offset_dw;
   } else {
      assert(mrt_resource->type == USC_MRT_RESOURCE_TYPE_OUTPUT_REG);
      assert(mrt_resource->reg.offset == 0);

      position = mrt_resource->reg.output_reg;
   }

   assert(position <= 3 || PVR_HAS_FEATURE(dev_info, eight_output_registers));

   switch (position) {
   case 0:
   case 4:
      render_params.source_start = PVR_PBE_STARTPOS_BIT0;
      break;
   case 1:
   case 5:
      render_params.source_start = PVR_PBE_STARTPOS_BIT32;
      break;
   case 2:
   case 6:
      render_params.source_start = PVR_PBE_STARTPOS_BIT64;
      break;
   case 3:
   case 7:
      render_params.source_start = PVR_PBE_STARTPOS_BIT96;
      break;
   default:
      assert(!"Invalid output register");
      break;
   }

#define PVR_DEC_IF_NOT_ZERO(_v) (((_v) > 0) ? (_v)-1 : 0)

   render_params.min_x_clip = MAX2(0, render_area->offset.x);
   render_params.min_y_clip = MAX2(0, render_area->offset.y);
   render_params.max_x_clip = MIN2(
      framebuffer->width - 1,
      PVR_DEC_IF_NOT_ZERO(render_area->offset.x + render_area->extent.width));
   render_params.max_y_clip = MIN2(
      framebuffer->height - 1,
      PVR_DEC_IF_NOT_ZERO(render_area->offset.y + render_area->extent.height));

#undef PVR_DEC_IF_NOT_ZERO

   render_params.slice = 0;
   render_params.mrt_index = mrt_index;

   pvr_pbe_pack_state(dev_info,
                      &surface_params,
                      &render_params,
                      pbe_cs_words,
                      pbe_reg_words);
}

static struct pvr_render_target *
pvr_get_render_target(const struct pvr_render_pass *pass,
                      const struct pvr_framebuffer *framebuffer,
                      uint32_t idx)
{
   const struct pvr_renderpass_hwsetup_render *hw_render =
      &pass->hw_setup->renders[idx];
   uint32_t rt_idx = 0;

   switch (hw_render->sample_count) {
   case 1:
   case 2:
   case 4:
   case 8:
      rt_idx = util_logbase2(hw_render->sample_count);
      break;

   default:
      unreachable("Unsupported sample count");
      break;
   }

   return &framebuffer->render_targets[rt_idx];
}

static uint32_t
pvr_pass_get_pixel_output_width(const struct pvr_render_pass *pass,
                                uint32_t idx,
                                const struct pvr_device_info *dev_info)
{
   const struct pvr_renderpass_hwsetup_render *hw_render =
      &pass->hw_setup->renders[idx];
   /* Default value based on the maximum value found in all existing cores. The
    * maximum is used as this is being treated as a lower bound, making it a
    * "safer" choice than the minimum value found in all existing cores.
    */
   const uint32_t min_output_regs =
      PVR_GET_FEATURE_VALUE(dev_info, usc_min_output_registers_per_pix, 2U);
   const uint32_t width = MAX2(hw_render->output_regs_count, min_output_regs);

   return util_next_power_of_two(width);
}

static inline bool
pvr_ds_attachment_requires_zls(const struct pvr_ds_attachment *attachment)
{
   bool zls_used;

   zls_used = attachment->load.d || attachment->load.s;
   zls_used |= attachment->store.d || attachment->store.s;

   return zls_used;
}

/**
 * \brief If depth and/or stencil attachment dimensions are not tile-aligned,
 * then we may need to insert some additional transfer subcommands.
 *
 * It's worth noting that we check whether the dimensions are smaller than a
 * tile here, rather than checking whether they're tile-aligned - this relies
 * on the assumption that we can safely use any attachment with dimensions
 * larger than a tile. If the attachment is twiddled, it will be over-allocated
 * to the nearest power-of-two (which will be tile-aligned). If the attachment
 * is not twiddled, we don't need to worry about tile-alignment at all.
 */
static bool pvr_sub_cmd_gfx_requires_ds_subtile_alignment(
   const struct pvr_device_info *dev_info,
   const struct pvr_render_job *job)
{
   const struct pvr_image *const ds_image =
      pvr_image_view_get_image(job->ds.iview);
   uint32_t zls_tile_size_x;
   uint32_t zls_tile_size_y;

   rogue_get_zls_tile_size_xy(dev_info, &zls_tile_size_x, &zls_tile_size_y);

   if (ds_image->physical_extent.width >= zls_tile_size_x &&
       ds_image->physical_extent.height >= zls_tile_size_y) {
      return false;
   }

   /* If we have the zls_subtile feature, we can skip the alignment iff:
    *  - The attachment is not multisampled, and
    *  - The depth and stencil attachments are the same.
    */
   if (PVR_HAS_FEATURE(dev_info, zls_subtile) &&
       ds_image->vk.samples == VK_SAMPLE_COUNT_1_BIT &&
       job->has_stencil_attachment == job->has_depth_attachment) {
      return false;
   }

   /* No ZLS functions enabled; nothing to do. */
   if ((!job->has_depth_attachment && !job->has_stencil_attachment) ||
       !pvr_ds_attachment_requires_zls(&job->ds)) {
      return false;
   }

   return true;
}

static VkResult
pvr_sub_cmd_gfx_align_ds_subtiles(struct pvr_cmd_buffer *const cmd_buffer,
                                  struct pvr_sub_cmd_gfx *const gfx_sub_cmd)
{
   struct pvr_sub_cmd *const prev_sub_cmd =
      container_of(gfx_sub_cmd, struct pvr_sub_cmd, gfx);
   struct pvr_ds_attachment *const ds = &gfx_sub_cmd->job.ds;
   const struct pvr_image *const ds_image = pvr_image_view_get_image(ds->iview);
   const VkFormat copy_format = pvr_get_raw_copy_format(ds_image->vk.format);

   struct pvr_suballoc_bo *buffer;
   uint32_t buffer_layer_size;
   VkBufferImageCopy2 region;
   VkExtent2D zls_tile_size;
   VkExtent2D rounded_size;
   uint32_t buffer_size;
   VkExtent2D scale;
   VkResult result;

   /* The operations below assume the last command in the buffer was the target
    * gfx subcommand. Assert that this is the case.
    */
   assert(list_last_entry(&cmd_buffer->sub_cmds, struct pvr_sub_cmd, link) ==
          prev_sub_cmd);

   if (!pvr_ds_attachment_requires_zls(ds))
      return VK_SUCCESS;

   rogue_get_zls_tile_size_xy(&cmd_buffer->device->pdevice->dev_info,
                              &zls_tile_size.width,
                              &zls_tile_size.height);
   rogue_get_isp_scale_xy_from_samples(ds_image->vk.samples,
                                       &scale.width,
                                       &scale.height);

   rounded_size = (VkExtent2D){
      .width = ALIGN_POT(ds_image->physical_extent.width, zls_tile_size.width),
      .height =
         ALIGN_POT(ds_image->physical_extent.height, zls_tile_size.height),
   };

   buffer_layer_size = vk_format_get_blocksize(ds_image->vk.format) *
                       rounded_size.width * rounded_size.height * scale.width *
                       scale.height;

   if (ds->iview->vk.layer_count > 1)
      buffer_layer_size = ALIGN_POT(buffer_layer_size, ds_image->alignment);

   buffer_size = buffer_layer_size * ds->iview->vk.layer_count;

   result = pvr_cmd_buffer_alloc_mem(cmd_buffer,
                                     cmd_buffer->device->heaps.general_heap,
                                     buffer_size,
                                     &buffer);
   if (result != VK_SUCCESS)
      return result;

   region = (VkBufferImageCopy2){
      .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
      .pNext = NULL,
      .bufferOffset = 0,
      .bufferRowLength = rounded_size.width,
      .bufferImageHeight = 0,
      .imageSubresource = {
         .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
         .mipLevel = ds->iview->vk.base_mip_level,
         .baseArrayLayer = ds->iview->vk.base_array_layer,
         .layerCount = ds->iview->vk.layer_count,
      },
      .imageOffset = { 0 },
      .imageExtent = {
         .width = ds->iview->vk.extent.width,
         .height = ds->iview->vk.extent.height,
         .depth = 1,
      },
   };

   if (ds->load.d || ds->load.s) {
      cmd_buffer->state.current_sub_cmd = NULL;

      result =
         pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_TRANSFER);
      if (result != VK_SUCCESS)
         return result;

      result = pvr_copy_image_to_buffer_region_format(cmd_buffer,
                                                      ds_image,
                                                      buffer->dev_addr,
                                                      &region,
                                                      copy_format,
                                                      copy_format);
      if (result != VK_SUCCESS)
         return result;

      cmd_buffer->state.current_sub_cmd->transfer.serialize_with_frag = true;

      result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;

      /* Now we have to fiddle with cmd_buffer to place this transfer command
       * *before* the target gfx subcommand.
       */
      list_move_to(&cmd_buffer->state.current_sub_cmd->link,
                   &prev_sub_cmd->link);

      cmd_buffer->state.current_sub_cmd = prev_sub_cmd;
   }

   if (ds->store.d || ds->store.s) {
      cmd_buffer->state.current_sub_cmd = NULL;

      result =
         pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_TRANSFER);
      if (result != VK_SUCCESS)
         return result;

      result = pvr_copy_buffer_to_image_region_format(cmd_buffer,
                                                      buffer->dev_addr,
                                                      ds_image,
                                                      &region,
                                                      copy_format,
                                                      copy_format,
                                                      0);
      if (result != VK_SUCCESS)
         return result;

      cmd_buffer->state.current_sub_cmd->transfer.serialize_with_frag = true;

      result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;

      cmd_buffer->state.current_sub_cmd = prev_sub_cmd;
   }

   /* Finally, patch up the target graphics sub_cmd to use the correctly-strided
    * buffer.
    */
   ds->has_alignment_transfers = true;
   ds->addr = buffer->dev_addr;
   ds->physical_extent = rounded_size;

   gfx_sub_cmd->wait_on_previous_transfer = true;

   return VK_SUCCESS;
}

struct pvr_emit_state {
   uint32_t pbe_cs_words[PVR_MAX_COLOR_ATTACHMENTS]
                        [ROGUE_NUM_PBESTATE_STATE_WORDS];

   uint64_t pbe_reg_words[PVR_MAX_COLOR_ATTACHMENTS]
                         [ROGUE_NUM_PBESTATE_REG_WORDS];

   uint32_t emit_count;
};

static void
pvr_setup_emit_state(const struct pvr_device_info *dev_info,
                     const struct pvr_renderpass_hwsetup_render *hw_render,
                     struct pvr_render_pass_info *render_pass_info,
                     struct pvr_emit_state *emit_state)
{
   assert(hw_render->pbe_emits <= PVR_NUM_PBE_EMIT_REGS);

   if (hw_render->eot_surface_count == 0) {
      emit_state->emit_count = 1;
      pvr_csb_pack (&emit_state->pbe_cs_words[0][1],
                    PBESTATE_STATE_WORD1,
                    state) {
         state.emptytile = true;
      }
      return;
   }

   static_assert(USC_MRT_RESOURCE_TYPE_OUTPUT_REG + 1 ==
                    USC_MRT_RESOURCE_TYPE_MEMORY,
                 "The loop below needs adjusting.");

   emit_state->emit_count = 0;
   for (uint32_t resource_type = USC_MRT_RESOURCE_TYPE_OUTPUT_REG;
        resource_type <= USC_MRT_RESOURCE_TYPE_MEMORY;
        resource_type++) {
      for (uint32_t i = 0; i < hw_render->eot_surface_count; i++) {
         const struct pvr_framebuffer *framebuffer =
            render_pass_info->framebuffer;
         const struct pvr_renderpass_hwsetup_eot_surface *surface =
            &hw_render->eot_surfaces[i];
         const struct pvr_image_view *iview =
            render_pass_info->attachments[surface->attachment_idx];
         const struct usc_mrt_resource *mrt_resource =
            &hw_render->eot_setup.mrt_resources[surface->mrt_idx];
         uint32_t samples = 1;

         if (mrt_resource->type != resource_type)
            continue;

         if (surface->need_resolve) {
            const struct pvr_image_view *resolve_src =
               render_pass_info->attachments[surface->src_attachment_idx];

            /* Attachments that are the destination of resolve operations must
             * be loaded before their next use.
             */
            render_pass_info->enable_bg_tag = true;
            render_pass_info->process_empty_tiles = true;

            if (surface->resolve_type != PVR_RESOLVE_TYPE_PBE)
               continue;

            samples = (uint32_t)resolve_src->vk.image->samples;
         }

         assert(emit_state->emit_count < ARRAY_SIZE(emit_state->pbe_cs_words));
         assert(emit_state->emit_count < ARRAY_SIZE(emit_state->pbe_reg_words));

         pvr_setup_pbe_state(dev_info,
                             framebuffer,
                             emit_state->emit_count,
                             mrt_resource,
                             iview,
                             &render_pass_info->render_area,
                             surface->need_resolve,
                             samples,
                             emit_state->pbe_cs_words[emit_state->emit_count],
                             emit_state->pbe_reg_words[emit_state->emit_count]);
         emit_state->emit_count += 1;
      }
   }

   assert(emit_state->emit_count == hw_render->pbe_emits);
}

static inline bool
pvr_is_render_area_tile_aligned(const struct pvr_cmd_buffer *cmd_buffer,
                                const struct pvr_image_view *iview)
{
   const VkRect2D *render_area =
      &cmd_buffer->state.render_pass_info.render_area;

   return render_area->offset.x == 0 && render_area->offset.y == 0 &&
          render_area->extent.height == iview->vk.extent.height &&
          render_area->extent.width == iview->vk.extent.width;
}

static VkResult pvr_sub_cmd_gfx_job_init(const struct pvr_device_info *dev_info,
                                         struct pvr_cmd_buffer *cmd_buffer,
                                         struct pvr_sub_cmd_gfx *sub_cmd)
{
   static const VkClearDepthStencilValue default_ds_clear_value = {
      .depth = 1.0f,
      .stencil = 0xFFFFFFFF,
   };

   const struct vk_dynamic_graphics_state *dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct pvr_render_pass_info *render_pass_info =
      &cmd_buffer->state.render_pass_info;
   const struct pvr_renderpass_hwsetup_render *hw_render =
      &render_pass_info->pass->hw_setup->renders[sub_cmd->hw_render_idx];
   struct pvr_render_job *job = &sub_cmd->job;
   struct pvr_pds_upload pds_pixel_event_program;
   struct pvr_framebuffer *framebuffer = render_pass_info->framebuffer;
   struct pvr_spm_bgobj_state *spm_bgobj_state =
      &framebuffer->spm_bgobj_state_per_render[sub_cmd->hw_render_idx];
   struct pvr_render_target *render_target;
   VkResult result;

   if (sub_cmd->barrier_store) {
      /* There can only ever be one frag job running on the hardware at any one
       * time, and a context switch is not allowed mid-tile, so instead of
       * allocating a new scratch buffer we can reuse the SPM scratch buffer to
       * perform the store.
       * So use the SPM EOT program with the SPM PBE reg words in order to store
       * the render to the SPM scratch buffer.
       */

      memcpy(job->pbe_reg_words,
             &framebuffer->spm_eot_state_per_render[0].pbe_reg_words,
             sizeof(job->pbe_reg_words));
      job->pds_pixel_event_data_offset =
         framebuffer->spm_eot_state_per_render[0]
            .pixel_event_program_data_offset;
   } else {
      struct pvr_emit_state emit_state = { 0 };

      pvr_setup_emit_state(dev_info, hw_render, render_pass_info, &emit_state);

      memcpy(job->pbe_reg_words,
             emit_state.pbe_reg_words,
             sizeof(job->pbe_reg_words));

      result = pvr_sub_cmd_gfx_per_job_fragment_programs_create_and_upload(
         cmd_buffer,
         emit_state.emit_count,
         emit_state.pbe_cs_words[0],
         &pds_pixel_event_program);
      if (result != VK_SUCCESS)
         return result;

      job->pds_pixel_event_data_offset = pds_pixel_event_program.data_offset;
   }

   if (sub_cmd->barrier_load) {
      job->enable_bg_tag = true;
      job->process_empty_tiles = true;

      /* Load the previously stored render from the SPM scratch buffer. */

      STATIC_ASSERT(ARRAY_SIZE(job->pds_bgnd_reg_values) ==
                    ARRAY_SIZE(spm_bgobj_state->pds_reg_values));
      typed_memcpy(job->pds_bgnd_reg_values,
                   spm_bgobj_state->pds_reg_values,
                   ARRAY_SIZE(spm_bgobj_state->pds_reg_values));
   } else if (hw_render->load_op) {
      const struct pvr_load_op *load_op = hw_render->load_op;
      struct pvr_pds_upload load_op_program;

      /* Recalculate Background Object(s). */

      /* FIXME: Should we free the PDS pixel event data or let it be freed
       * when the pool gets emptied?
       */
      result = pvr_load_op_data_create_and_upload(cmd_buffer,
                                                  load_op,
                                                  &load_op_program);
      if (result != VK_SUCCESS)
         return result;

      job->enable_bg_tag = render_pass_info->enable_bg_tag;
      job->process_empty_tiles = render_pass_info->process_empty_tiles;

      pvr_pds_bgnd_pack_state(load_op,
                              &load_op_program,
                              job->pds_bgnd_reg_values);
   }

   /* TODO: In some cases a PR can be removed by storing to the color attachment
    * and have the background object load directly from it instead of using the
    * scratch buffer. In those cases we can also set this to "false" and avoid
    * extra fw overhead.
    */
   /* The scratch buffer is always needed and allocated to avoid data loss in
    * case SPM is hit so set the flag unconditionally.
    */
   job->requires_spm_scratch_buffer = true;

   memcpy(job->pr_pbe_reg_words,
          &framebuffer->spm_eot_state_per_render[0].pbe_reg_words,
          sizeof(job->pbe_reg_words));
   job->pr_pds_pixel_event_data_offset =
      framebuffer->spm_eot_state_per_render[0].pixel_event_program_data_offset;

   STATIC_ASSERT(ARRAY_SIZE(job->pds_pr_bgnd_reg_values) ==
                 ARRAY_SIZE(spm_bgobj_state->pds_reg_values));
   typed_memcpy(job->pds_pr_bgnd_reg_values,
                spm_bgobj_state->pds_reg_values,
                ARRAY_SIZE(spm_bgobj_state->pds_reg_values));

   render_target = pvr_get_render_target(render_pass_info->pass,
                                         framebuffer,
                                         sub_cmd->hw_render_idx);
   job->rt_dataset = render_target->rt_dataset;

   job->ctrl_stream_addr = pvr_csb_get_start_address(&sub_cmd->control_stream);

   if (sub_cmd->depth_bias_bo)
      job->depth_bias_table_addr = sub_cmd->depth_bias_bo->dev_addr;
   else
      job->depth_bias_table_addr = PVR_DEV_ADDR_INVALID;

   if (sub_cmd->scissor_bo)
      job->scissor_table_addr = sub_cmd->scissor_bo->dev_addr;
   else
      job->scissor_table_addr = PVR_DEV_ADDR_INVALID;

   job->pixel_output_width =
      pvr_pass_get_pixel_output_width(render_pass_info->pass,
                                      sub_cmd->hw_render_idx,
                                      dev_info);

   /* Setup depth/stencil job information. */
   if (hw_render->ds_attach_idx != VK_ATTACHMENT_UNUSED) {
      struct pvr_image_view *ds_iview =
         render_pass_info->attachments[hw_render->ds_attach_idx];
      const struct pvr_image *ds_image = pvr_image_view_get_image(ds_iview);

      job->has_depth_attachment = vk_format_has_depth(ds_image->vk.format);
      job->has_stencil_attachment = vk_format_has_stencil(ds_image->vk.format);

      if (job->has_depth_attachment || job->has_stencil_attachment) {
         uint32_t level_pitch =
            ds_image->mip_levels[ds_iview->vk.base_mip_level].pitch;
         const bool render_area_is_tile_aligned =
            pvr_is_render_area_tile_aligned(cmd_buffer, ds_iview);
         bool store_was_optimised_out = false;
         bool d_store = false, s_store = false;
         bool d_load = false, s_load = false;

         job->ds.iview = ds_iview;
         job->ds.addr = ds_image->dev_addr;

         job->ds.stride =
            pvr_stride_from_pitch(level_pitch, ds_iview->vk.format);
         job->ds.height = ds_iview->vk.extent.height;
         job->ds.physical_extent = (VkExtent2D){
            .width = u_minify(ds_image->physical_extent.width,
                              ds_iview->vk.base_mip_level),
            .height = u_minify(ds_image->physical_extent.height,
                               ds_iview->vk.base_mip_level),
         };
         job->ds.layer_size = ds_image->layer_size;

         job->ds_clear_value = default_ds_clear_value;

         if (hw_render->ds_attach_idx < render_pass_info->clear_value_count) {
            const VkClearDepthStencilValue *const clear_values =
               &render_pass_info->clear_values[hw_render->ds_attach_idx]
                   .depthStencil;

            if (job->has_depth_attachment)
               job->ds_clear_value.depth = clear_values->depth;

            if (job->has_stencil_attachment)
               job->ds_clear_value.stencil = clear_values->stencil;
         }

         switch (ds_iview->vk.format) {
         case VK_FORMAT_D16_UNORM:
            job->ds.zls_format = PVRX(CR_ZLS_FORMAT_TYPE_16BITINT);
            break;

         case VK_FORMAT_S8_UINT:
         case VK_FORMAT_D32_SFLOAT:
            job->ds.zls_format = PVRX(CR_ZLS_FORMAT_TYPE_F32Z);
            break;

         case VK_FORMAT_D24_UNORM_S8_UINT:
            job->ds.zls_format = PVRX(CR_ZLS_FORMAT_TYPE_24BITINT);
            break;

         default:
            unreachable("Unsupported depth stencil format");
         }

         job->ds.memlayout = ds_image->memlayout;

         if (job->has_depth_attachment) {
            if (hw_render->depth_store || sub_cmd->barrier_store) {
               const bool depth_init_is_clear = hw_render->depth_init ==
                                                VK_ATTACHMENT_LOAD_OP_CLEAR;

               d_store = true;

               if (hw_render->depth_store && render_area_is_tile_aligned &&
                   !(sub_cmd->modifies_depth || depth_init_is_clear)) {
                  d_store = false;
                  store_was_optimised_out = true;
               }
            }

            if (d_store && !render_area_is_tile_aligned) {
               d_load = true;
            } else if (hw_render->depth_init == VK_ATTACHMENT_LOAD_OP_LOAD) {
               enum pvr_depth_stencil_usage depth_usage = sub_cmd->depth_usage;

               assert(depth_usage != PVR_DEPTH_STENCIL_USAGE_UNDEFINED);
               d_load = (depth_usage != PVR_DEPTH_STENCIL_USAGE_NEVER);
            } else {
               d_load = sub_cmd->barrier_load;
            }
         }

         if (job->has_stencil_attachment) {
            if (hw_render->stencil_store || sub_cmd->barrier_store) {
               const bool stencil_init_is_clear = hw_render->stencil_init ==
                                                  VK_ATTACHMENT_LOAD_OP_CLEAR;

               s_store = true;

               if (hw_render->stencil_store && render_area_is_tile_aligned &&
                   !(sub_cmd->modifies_stencil || stencil_init_is_clear)) {
                  s_store = false;
                  store_was_optimised_out = true;
               }
            }

            if (s_store && !render_area_is_tile_aligned) {
               s_load = true;
            } else if (hw_render->stencil_init == VK_ATTACHMENT_LOAD_OP_LOAD) {
               enum pvr_depth_stencil_usage stencil_usage =
                  sub_cmd->stencil_usage;

               assert(stencil_usage != PVR_DEPTH_STENCIL_USAGE_UNDEFINED);
               s_load = (stencil_usage != PVR_DEPTH_STENCIL_USAGE_NEVER);
            } else {
               s_load = sub_cmd->barrier_load;
            }
         }

         job->ds.load.d = d_load;
         job->ds.load.s = s_load;
         job->ds.store.d = d_store;
         job->ds.store.s = s_store;

         /* ZLS can't do masked writes for packed depth stencil formats so if
          * we store anything, we have to store everything.
          */
         if ((job->ds.store.d || job->ds.store.s) &&
             pvr_zls_format_type_is_packed(job->ds.zls_format)) {
            job->ds.store.d = true;
            job->ds.store.s = true;

            /* In case we are only operating on one aspect of the attachment we
             * need to load the unused one in order to preserve its contents due
             * to the forced store which might otherwise corrupt it.
             */
            if (hw_render->depth_init != VK_ATTACHMENT_LOAD_OP_CLEAR)
               job->ds.load.d = true;

            if (hw_render->stencil_init != VK_ATTACHMENT_LOAD_OP_CLEAR)
               job->ds.load.s = true;
         }

         if (pvr_ds_attachment_requires_zls(&job->ds) ||
             store_was_optimised_out) {
            job->process_empty_tiles = true;
         }

         if (pvr_sub_cmd_gfx_requires_ds_subtile_alignment(dev_info, job)) {
            result = pvr_sub_cmd_gfx_align_ds_subtiles(cmd_buffer, sub_cmd);
            if (result != VK_SUCCESS)
               return result;
         }
      }
   } else {
      job->has_depth_attachment = false;
      job->has_stencil_attachment = false;
      job->ds_clear_value = default_ds_clear_value;
   }

   if (hw_render->ds_attach_idx != VK_ATTACHMENT_UNUSED) {
      struct pvr_image_view *iview =
         render_pass_info->attachments[hw_render->ds_attach_idx];
      const struct pvr_image *image = pvr_image_view_get_image(iview);

      /* If the HW render pass has a valid depth/stencil surface, determine the
       * sample count from the attachment's image.
       */
      job->samples = image->vk.samples;
   } else if (hw_render->output_regs_count) {
      /* If the HW render pass has output registers, we have color attachments
       * to write to, so determine the sample count from the count specified for
       * every color attachment in this render.
       */
      job->samples = hw_render->sample_count;
   } else if (cmd_buffer->state.gfx_pipeline) {
      /* If the HW render pass has no color or depth/stencil attachments, we
       * determine the sample count from the count given during pipeline
       * creation.
       */
      job->samples = dynamic_state->ms.rasterization_samples;
   } else if (render_pass_info->pass->attachment_count > 0) {
      /* If we get here, we have a render pass with subpasses containing no
       * attachments. The next best thing is largest of the sample counts
       * specified by the render pass attachment descriptions.
       */
      job->samples = render_pass_info->pass->max_sample_count;
   } else {
      /* No appropriate framebuffer attachment is available. */
      mesa_logw("Defaulting render job sample count to 1.");
      job->samples = VK_SAMPLE_COUNT_1_BIT;
   }

   if (sub_cmd->max_tiles_in_flight ==
       PVR_GET_FEATURE_VALUE(dev_info, isp_max_tiles_in_flight, 1U)) {
      /* Use the default limit based on the partition store. */
      job->max_tiles_in_flight = 0U;
   } else {
      job->max_tiles_in_flight = sub_cmd->max_tiles_in_flight;
   }

   job->frag_uses_atomic_ops = sub_cmd->frag_uses_atomic_ops;
   job->disable_compute_overlap = false;
   job->max_shared_registers = cmd_buffer->state.max_shared_regs;
   job->run_frag = true;
   job->geometry_terminate = true;

   return VK_SUCCESS;
}

static void
pvr_sub_cmd_compute_job_init(const struct pvr_physical_device *pdevice,
                             struct pvr_cmd_buffer *cmd_buffer,
                             struct pvr_sub_cmd_compute *sub_cmd)
{
   sub_cmd->num_shared_regs = MAX2(cmd_buffer->device->idfwdf_state.usc_shareds,
                                   cmd_buffer->state.max_shared_regs);

   cmd_buffer->state.max_shared_regs = 0U;
}

#define PIXEL_ALLOCATION_SIZE_MAX_IN_BLOCKS \
   (1024 / PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE))

static uint32_t
pvr_compute_flat_slot_size(const struct pvr_physical_device *pdevice,
                           uint32_t coeff_regs_count,
                           bool use_barrier,
                           uint32_t total_workitems)
{
   const struct pvr_device_runtime_info *dev_runtime_info =
      &pdevice->dev_runtime_info;
   const struct pvr_device_info *dev_info = &pdevice->dev_info;
   uint32_t max_workgroups_per_task = ROGUE_CDM_MAX_PACKED_WORKGROUPS_PER_TASK;
   uint32_t max_avail_coeff_regs =
      dev_runtime_info->cdm_max_local_mem_size_regs;
   uint32_t localstore_chunks_count =
      DIV_ROUND_UP(PVR_DW_TO_BYTES(coeff_regs_count),
                   PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE));

   /* Ensure that we cannot have more workgroups in a slot than the available
    * number of coefficients allow us to have.
    */
   if (coeff_regs_count > 0U) {
      /* If the geometry or fragment jobs can overlap with the compute job, or
       * if there is a vertex shader already running then we need to consider
       * this in calculating max allowed work-groups.
       */
      if (PVR_HAS_QUIRK(dev_info, 52354) &&
          (PVR_HAS_FEATURE(dev_info, compute_overlap) ||
           PVR_HAS_FEATURE(dev_info, gs_rta_support))) {
         /* Solve for n (number of work-groups per task). All values are in
          * size of common store alloc blocks:
          *
          * n + (2n + 7) * (local_memory_size_max - 1) =
          * 	(coefficient_memory_pool_size) - (7 * pixel_allocation_size_max)
          * ==>
          * n + 2n * (local_memory_size_max - 1) =
          * 	(coefficient_memory_pool_size) - (7 * pixel_allocation_size_max)
          * 	- (7 * (local_memory_size_max - 1))
          * ==>
          * n * (1 + 2 * (local_memory_size_max - 1)) =
          * 	(coefficient_memory_pool_size) - (7 * pixel_allocation_size_max)
          * 	- (7 * (local_memory_size_max - 1))
          * ==>
          * n = ((coefficient_memory_pool_size) -
          * 	(7 * pixel_allocation_size_max) -
          * 	(7 * (local_memory_size_max - 1)) / (1 +
          * 2 * (local_memory_size_max - 1)))
          */
         uint32_t max_common_store_blocks =
            DIV_ROUND_UP(max_avail_coeff_regs * 4U,
                         PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE));

         /* (coefficient_memory_pool_size) - (7 * pixel_allocation_size_max)
          */
         max_common_store_blocks -= ROGUE_MAX_OVERLAPPED_PIXEL_TASK_INSTANCES *
                                    PIXEL_ALLOCATION_SIZE_MAX_IN_BLOCKS;

         /* - (7 * (local_memory_size_max - 1)) */
         max_common_store_blocks -= (ROGUE_MAX_OVERLAPPED_PIXEL_TASK_INSTANCES *
                                     (localstore_chunks_count - 1U));

         /* Divide by (1 + 2 * (local_memory_size_max - 1)) */
         max_workgroups_per_task = max_common_store_blocks /
                                   (1U + 2U * (localstore_chunks_count - 1U));

         max_workgroups_per_task =
            MIN2(max_workgroups_per_task,
                 ROGUE_CDM_MAX_PACKED_WORKGROUPS_PER_TASK);

      } else {
         max_workgroups_per_task =
            MIN2((max_avail_coeff_regs / coeff_regs_count),
                 max_workgroups_per_task);
      }
   }

   /* max_workgroups_per_task should at least be one. */
   assert(max_workgroups_per_task >= 1U);

   if (total_workitems >= ROGUE_MAX_INSTANCES_PER_TASK) {
      /* In this case, the work group size will have been padded up to the
       * next ROGUE_MAX_INSTANCES_PER_TASK so we just set max instances to be
       * ROGUE_MAX_INSTANCES_PER_TASK.
       */
      return ROGUE_MAX_INSTANCES_PER_TASK;
   }

   /* In this case, the number of instances in the slot must be clamped to
    * accommodate whole work-groups only.
    */
   if (PVR_HAS_QUIRK(dev_info, 49032) || use_barrier) {
      max_workgroups_per_task =
         MIN2(max_workgroups_per_task,
              ROGUE_MAX_INSTANCES_PER_TASK / total_workitems);
      return total_workitems * max_workgroups_per_task;
   }

   return MIN2(total_workitems * max_workgroups_per_task,
               ROGUE_MAX_INSTANCES_PER_TASK);
}

static void
pvr_compute_generate_control_stream(struct pvr_csb *csb,
                                    struct pvr_sub_cmd_compute *sub_cmd,
                                    const struct pvr_compute_kernel_info *info)
{
   pvr_csb_set_relocation_mark(csb);

   /* Compute kernel 0. */
   pvr_csb_emit (csb, CDMCTRL_KERNEL0, kernel0) {
      kernel0.indirect_present = !!info->indirect_buffer_addr.addr;
      kernel0.global_offsets_present = info->global_offsets_present;
      kernel0.usc_common_size = info->usc_common_size;
      kernel0.usc_unified_size = info->usc_unified_size;
      kernel0.pds_temp_size = info->pds_temp_size;
      kernel0.pds_data_size = info->pds_data_size;
      kernel0.usc_target = info->usc_target;
      kernel0.fence = info->is_fence;
   }

   /* Compute kernel 1. */
   pvr_csb_emit (csb, CDMCTRL_KERNEL1, kernel1) {
      kernel1.data_addr = PVR_DEV_ADDR(info->pds_data_offset);
      kernel1.sd_type = info->sd_type;
      kernel1.usc_common_shared = info->usc_common_shared;
   }

   /* Compute kernel 2. */
   pvr_csb_emit (csb, CDMCTRL_KERNEL2, kernel2) {
      kernel2.code_addr = PVR_DEV_ADDR(info->pds_code_offset);
   }

   if (info->indirect_buffer_addr.addr) {
      /* Compute kernel 6. */
      pvr_csb_emit (csb, CDMCTRL_KERNEL6, kernel6) {
         kernel6.indirect_addrmsb = info->indirect_buffer_addr;
      }

      /* Compute kernel 7. */
      pvr_csb_emit (csb, CDMCTRL_KERNEL7, kernel7) {
         kernel7.indirect_addrlsb = info->indirect_buffer_addr;
      }
   } else {
      /* Compute kernel 3. */
      pvr_csb_emit (csb, CDMCTRL_KERNEL3, kernel3) {
         assert(info->global_size[0U] > 0U);
         kernel3.workgroup_x = info->global_size[0U] - 1U;
      }

      /* Compute kernel 4. */
      pvr_csb_emit (csb, CDMCTRL_KERNEL4, kernel4) {
         assert(info->global_size[1U] > 0U);
         kernel4.workgroup_y = info->global_size[1U] - 1U;
      }

      /* Compute kernel 5. */
      pvr_csb_emit (csb, CDMCTRL_KERNEL5, kernel5) {
         assert(info->global_size[2U] > 0U);
         kernel5.workgroup_z = info->global_size[2U] - 1U;
      }
   }

   /* Compute kernel 8. */
   pvr_csb_emit (csb, CDMCTRL_KERNEL8, kernel8) {
      if (info->max_instances == ROGUE_MAX_INSTANCES_PER_TASK)
         kernel8.max_instances = 0U;
      else
         kernel8.max_instances = info->max_instances;

      assert(info->local_size[0U] > 0U);
      kernel8.workgroup_size_x = info->local_size[0U] - 1U;
      assert(info->local_size[1U] > 0U);
      kernel8.workgroup_size_y = info->local_size[1U] - 1U;
      assert(info->local_size[2U] > 0U);
      kernel8.workgroup_size_z = info->local_size[2U] - 1U;
   }

   pvr_csb_clear_relocation_mark(csb);

   /* Track the highest amount of shared registers usage in this dispatch.
    * This is used by the FW for context switching, so must be large enough
    * to contain all the shared registers that might be in use for this compute
    * job. Coefficients don't need to be included as the context switch will not
    * happen within the execution of a single workgroup, thus nothing needs to
    * be preserved.
    */
   if (info->usc_common_shared) {
      sub_cmd->num_shared_regs =
         MAX2(sub_cmd->num_shared_regs, info->usc_common_size);
   }
}

/* TODO: This can be pre-packed and uploaded directly. Would that provide any
 * speed up?
 */
static void
pvr_compute_generate_idfwdf(struct pvr_cmd_buffer *cmd_buffer,
                            struct pvr_sub_cmd_compute *const sub_cmd)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   bool *const is_sw_barier_required =
      &state->current_sub_cmd->compute.pds_sw_barrier_requires_clearing;
   const struct pvr_physical_device *pdevice = cmd_buffer->device->pdevice;
   struct pvr_csb *csb = &sub_cmd->control_stream;
   const struct pvr_pds_upload *program;

   if (PVR_NEED_SW_COMPUTE_PDS_BARRIER(&pdevice->dev_info) &&
       *is_sw_barier_required) {
      *is_sw_barier_required = false;
      program = &cmd_buffer->device->idfwdf_state.sw_compute_barrier_pds;
   } else {
      program = &cmd_buffer->device->idfwdf_state.pds;
   }

   struct pvr_compute_kernel_info info = {
      .indirect_buffer_addr = PVR_DEV_ADDR_INVALID,
      .global_offsets_present = false,
      .usc_common_size = DIV_ROUND_UP(
         PVR_DW_TO_BYTES(cmd_buffer->device->idfwdf_state.usc_shareds),
         PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE)),
      .usc_unified_size = 0U,
      .pds_temp_size = 0U,
      .pds_data_size =
         DIV_ROUND_UP(PVR_DW_TO_BYTES(program->data_size),
                      PVRX(CDMCTRL_KERNEL0_PDS_DATA_SIZE_UNIT_SIZE)),
      .usc_target = PVRX(CDMCTRL_USC_TARGET_ALL),
      .is_fence = false,
      .pds_data_offset = program->data_offset,
      .sd_type = PVRX(CDMCTRL_SD_TYPE_USC),
      .usc_common_shared = true,
      .pds_code_offset = program->code_offset,
      .global_size = { 1U, 1U, 1U },
      .local_size = { 1U, 1U, 1U },
   };

   /* We don't need to pad work-group size for this case. */

   info.max_instances =
      pvr_compute_flat_slot_size(pdevice,
                                 cmd_buffer->device->idfwdf_state.usc_shareds,
                                 false,
                                 1U);

   pvr_compute_generate_control_stream(csb, sub_cmd, &info);
}

void pvr_compute_generate_fence(struct pvr_cmd_buffer *cmd_buffer,
                                struct pvr_sub_cmd_compute *const sub_cmd,
                                bool deallocate_shareds)
{
   const struct pvr_pds_upload *program =
      &cmd_buffer->device->pds_compute_fence_program;
   const struct pvr_physical_device *pdevice = cmd_buffer->device->pdevice;
   struct pvr_csb *csb = &sub_cmd->control_stream;

   struct pvr_compute_kernel_info info = {
      .indirect_buffer_addr = PVR_DEV_ADDR_INVALID,
      .global_offsets_present = false,
      .usc_common_size = 0U,
      .usc_unified_size = 0U,
      .pds_temp_size = 0U,
      .pds_data_size =
         DIV_ROUND_UP(PVR_DW_TO_BYTES(program->data_size),
                      PVRX(CDMCTRL_KERNEL0_PDS_DATA_SIZE_UNIT_SIZE)),
      .usc_target = PVRX(CDMCTRL_USC_TARGET_ANY),
      .is_fence = true,
      .pds_data_offset = program->data_offset,
      .sd_type = PVRX(CDMCTRL_SD_TYPE_PDS),
      .usc_common_shared = deallocate_shareds,
      .pds_code_offset = program->code_offset,
      .global_size = { 1U, 1U, 1U },
      .local_size = { 1U, 1U, 1U },
   };

   /* We don't need to pad work-group size for this case. */
   /* Here we calculate the slot size. This can depend on the use of barriers,
    * local memory, BRN's or other factors.
    */
   info.max_instances = pvr_compute_flat_slot_size(pdevice, 0U, false, 1U);

   pvr_compute_generate_control_stream(csb, sub_cmd, &info);
}

static VkResult
pvr_cmd_buffer_process_deferred_clears(struct pvr_cmd_buffer *cmd_buffer)
{
   util_dynarray_foreach (&cmd_buffer->deferred_clears,
                          struct pvr_transfer_cmd,
                          transfer_cmd) {
      VkResult result;

      result = pvr_cmd_buffer_add_transfer_cmd(cmd_buffer, transfer_cmd);
      if (result != VK_SUCCESS)
         return result;

      cmd_buffer->state.current_sub_cmd->transfer.serialize_with_frag = true;
   }

   return VK_SUCCESS;
}

VkResult pvr_cmd_buffer_end_sub_cmd(struct pvr_cmd_buffer *cmd_buffer)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_sub_cmd *sub_cmd = state->current_sub_cmd;
   struct pvr_device *device = cmd_buffer->device;
   const struct pvr_query_pool *query_pool = NULL;
   struct pvr_suballoc_bo *query_bo = NULL;
   size_t query_indices_size = 0;
   VkResult result;

   /* FIXME: Is this NULL check required because this function is called from
    * pvr_resolve_unemitted_resolve_attachments()? See comment about this
    * function being called twice in a row in pvr_CmdEndRenderPass().
    */
   if (!sub_cmd)
      return VK_SUCCESS;

   if (!sub_cmd->owned) {
      state->current_sub_cmd = NULL;
      return VK_SUCCESS;
   }

   switch (sub_cmd->type) {
   case PVR_SUB_CMD_TYPE_GRAPHICS: {
      struct pvr_sub_cmd_gfx *const gfx_sub_cmd = &sub_cmd->gfx;

      query_indices_size =
         util_dynarray_num_elements(&state->query_indices, char);

      if (query_indices_size > 0) {
         const bool secondary_cont =
            cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY &&
            cmd_buffer->usage_flags &
               VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

         assert(gfx_sub_cmd->query_pool);

         if (secondary_cont) {
            util_dynarray_append_dynarray(&state->query_indices,
                                          &gfx_sub_cmd->sec_query_indices);
         } else {
            const void *data = util_dynarray_begin(&state->query_indices);

            result = pvr_cmd_buffer_upload_general(cmd_buffer,
                                                   data,
                                                   query_indices_size,
                                                   &query_bo);
            if (result != VK_SUCCESS)
               return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

            query_pool = gfx_sub_cmd->query_pool;
         }

         gfx_sub_cmd->has_occlusion_query = true;

         util_dynarray_clear(&state->query_indices);
      }

      if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
         result = pvr_csb_emit_return(&gfx_sub_cmd->control_stream);
         if (result != VK_SUCCESS)
            return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

         break;
      }

      /* TODO: Check if the sub_cmd can be skipped based on
       * sub_cmd->gfx.empty_cmd flag.
       */

      /* TODO: Set the state in the functions called with the command buffer
       * instead of here.
       */

      result = pvr_cmd_buffer_upload_tables(device, cmd_buffer, gfx_sub_cmd);
      if (result != VK_SUCCESS)
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

      result = pvr_cmd_buffer_emit_ppp_state(cmd_buffer,
                                             &gfx_sub_cmd->control_stream);
      if (result != VK_SUCCESS)
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

      result = pvr_csb_emit_terminate(&gfx_sub_cmd->control_stream);
      if (result != VK_SUCCESS)
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

      result = pvr_sub_cmd_gfx_job_init(&device->pdevice->dev_info,
                                        cmd_buffer,
                                        gfx_sub_cmd);
      if (result != VK_SUCCESS)
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

      if (pvr_sub_cmd_gfx_requires_split_submit(gfx_sub_cmd)) {
         result = pvr_sub_cmd_gfx_build_terminate_ctrl_stream(device,
                                                              cmd_buffer,
                                                              gfx_sub_cmd);
         if (result != VK_SUCCESS)
            return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
      }

      break;
   }

   case PVR_SUB_CMD_TYPE_OCCLUSION_QUERY:
   case PVR_SUB_CMD_TYPE_COMPUTE: {
      struct pvr_sub_cmd_compute *const compute_sub_cmd = &sub_cmd->compute;

      pvr_compute_generate_fence(cmd_buffer, compute_sub_cmd, true);

      result = pvr_csb_emit_terminate(&compute_sub_cmd->control_stream);
      if (result != VK_SUCCESS)
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

      pvr_sub_cmd_compute_job_init(device->pdevice,
                                   cmd_buffer,
                                   compute_sub_cmd);
      break;
   }

   case PVR_SUB_CMD_TYPE_TRANSFER:
      break;

   case PVR_SUB_CMD_TYPE_EVENT:
      break;

   default:
      unreachable("Unsupported sub-command type");
   }

   state->current_sub_cmd = NULL;

   /* pvr_cmd_buffer_process_deferred_clears() must be called with a NULL
    * current_sub_cmd.
    *
    * We can start a sub_cmd of a different type from the current sub_cmd only
    * after having ended the current sub_cmd. However, we can't end the current
    * sub_cmd if this depends on starting sub_cmd(s) of a different type. Hence,
    * don't try to start transfer sub_cmd(s) with
    * pvr_cmd_buffer_process_deferred_clears() until the current hasn't ended.
    * Failing to do so we will cause a circular dependency between
    * pvr_cmd_buffer_{end,start}_cmd and blow the stack.
    */
   if (sub_cmd->type == PVR_SUB_CMD_TYPE_GRAPHICS) {
      result = pvr_cmd_buffer_process_deferred_clears(cmd_buffer);
      if (result != VK_SUCCESS)
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
   }

   if (query_pool) {
      struct pvr_query_info query_info;

      assert(query_bo);
      assert(query_indices_size);

      query_info.type = PVR_QUERY_TYPE_AVAILABILITY_WRITE;

      /* sizeof(uint32_t) is for the size of single query. */
      query_info.availability_write.num_query_indices =
         query_indices_size / sizeof(uint32_t);
      query_info.availability_write.index_bo = query_bo;

      query_info.availability_write.num_queries = query_pool->query_count;
      query_info.availability_write.availability_bo =
         query_pool->availability_buffer;

      /* Insert a barrier after the graphics sub command and before the
       * query sub command so that the availability write program waits for the
       * fragment shader to complete.
       */

      result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_EVENT);
      if (result != VK_SUCCESS)
         return result;

      cmd_buffer->state.current_sub_cmd->event = (struct pvr_sub_cmd_event){
         .type = PVR_EVENT_TYPE_BARRIER,
         .barrier = {
            .wait_for_stage_mask = PVR_PIPELINE_STAGE_FRAG_BIT,
            .wait_at_stage_mask = PVR_PIPELINE_STAGE_OCCLUSION_QUERY_BIT,
         },
      };

      return pvr_add_query_program(cmd_buffer, &query_info);
   }

   return VK_SUCCESS;
}

void pvr_reset_graphics_dirty_state(struct pvr_cmd_buffer *const cmd_buffer,
                                    bool start_geom)
{
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;

   if (start_geom) {
      /*
       * Initial geometry phase state.
       * It's the driver's responsibility to ensure that the state of the
       * hardware is correctly initialized at the start of every geometry
       * phase. This is required to prevent stale state from a previous
       * geometry phase erroneously affecting the next geometry phase.
       *
       * If a geometry phase does not contain any geometry, this restriction
       * can be ignored. If the first draw call in a geometry phase will only
       * update the depth or stencil buffers i.e. ISP_TAGWRITEDISABLE is set
       * in the ISP State Control Word, the PDS State Pointers
       * (TA_PRES_PDSSTATEPTR*) in the first PPP State Update do not need to
       * be supplied, since they will never reach the PDS in the fragment
       * phase.
       */

      cmd_buffer->state.emit_header = (struct PVRX(TA_STATE_HEADER)){
         .pres_stream_out_size = true,
         .pres_ppp_ctrl = true,
         .pres_varying_word2 = true,
         .pres_varying_word1 = true,
         .pres_varying_word0 = true,
         .pres_outselects = true,
         .pres_wclamp = true,
         .pres_viewport = true,
         .pres_region_clip = true,
         .pres_pds_state_ptr0 = true,
         .pres_ispctl_fb = true,
         .pres_ispctl = true,
      };
   } else {
      struct PVRX(TA_STATE_HEADER) *const emit_header =
         &cmd_buffer->state.emit_header;

      emit_header->pres_ppp_ctrl = true;
      emit_header->pres_varying_word1 = true;
      emit_header->pres_varying_word0 = true;
      emit_header->pres_outselects = true;
      emit_header->pres_viewport = true;
      emit_header->pres_region_clip = true;
      emit_header->pres_pds_state_ptr0 = true;
      emit_header->pres_ispctl_fb = true;
      emit_header->pres_ispctl = true;
   }

   memset(&cmd_buffer->state.ppp_state,
          0U,
          sizeof(cmd_buffer->state.ppp_state));

   cmd_buffer->state.dirty.vertex_bindings = true;
   cmd_buffer->state.dirty.gfx_pipeline_binding = true;

   BITSET_SET(dynamic_state->dirty, MESA_VK_DYNAMIC_VP_VIEWPORTS);
   BITSET_SET(dynamic_state->dirty, MESA_VK_DYNAMIC_VP_VIEWPORT_COUNT);
}

static inline bool
pvr_cmd_uses_deferred_cs_cmds(const struct pvr_cmd_buffer *const cmd_buffer)
{
   const VkCommandBufferUsageFlags deferred_control_stream_flags =
      VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
      VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

   return cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY &&
          (cmd_buffer->usage_flags & deferred_control_stream_flags) ==
             deferred_control_stream_flags;
}

VkResult pvr_cmd_buffer_start_sub_cmd(struct pvr_cmd_buffer *cmd_buffer,
                                      enum pvr_sub_cmd_type type)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_device *device = cmd_buffer->device;
   struct pvr_sub_cmd *sub_cmd;
   VkResult result;

   /* Check the current status of the buffer. */
   if (vk_command_buffer_has_error(&cmd_buffer->vk))
      return vk_command_buffer_get_record_result(&cmd_buffer->vk);

   pvr_cmd_buffer_update_barriers(cmd_buffer, type);

   /* TODO: Add proper support for joining consecutive event sub_cmd? */
   if (state->current_sub_cmd) {
      if (state->current_sub_cmd->type == type) {
         /* Continue adding to the current sub command. */
         return VK_SUCCESS;
      }

      /* End the current sub command. */
      result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;
   }

   sub_cmd = vk_zalloc(&cmd_buffer->vk.pool->alloc,
                       sizeof(*sub_cmd),
                       8,
                       VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!sub_cmd) {
      return vk_command_buffer_set_error(&cmd_buffer->vk,
                                         VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   sub_cmd->type = type;
   sub_cmd->owned = true;

   switch (type) {
   case PVR_SUB_CMD_TYPE_GRAPHICS:
      sub_cmd->gfx.depth_usage = PVR_DEPTH_STENCIL_USAGE_UNDEFINED;
      sub_cmd->gfx.stencil_usage = PVR_DEPTH_STENCIL_USAGE_UNDEFINED;
      sub_cmd->gfx.modifies_depth = false;
      sub_cmd->gfx.modifies_stencil = false;
      sub_cmd->gfx.max_tiles_in_flight =
         PVR_GET_FEATURE_VALUE(&device->pdevice->dev_info,
                               isp_max_tiles_in_flight,
                               1);
      sub_cmd->gfx.hw_render_idx = state->render_pass_info.current_hw_subpass;
      sub_cmd->gfx.framebuffer = state->render_pass_info.framebuffer;
      sub_cmd->gfx.empty_cmd = true;

      if (state->vis_test_enabled)
         sub_cmd->gfx.query_pool = state->query_pool;

      pvr_reset_graphics_dirty_state(cmd_buffer, true);

      if (pvr_cmd_uses_deferred_cs_cmds(cmd_buffer)) {
         pvr_csb_init(device,
                      PVR_CMD_STREAM_TYPE_GRAPHICS_DEFERRED,
                      &sub_cmd->gfx.control_stream);
      } else {
         pvr_csb_init(device,
                      PVR_CMD_STREAM_TYPE_GRAPHICS,
                      &sub_cmd->gfx.control_stream);
      }

      util_dynarray_init(&sub_cmd->gfx.sec_query_indices, NULL);
      break;

   case PVR_SUB_CMD_TYPE_OCCLUSION_QUERY:
   case PVR_SUB_CMD_TYPE_COMPUTE:
      pvr_csb_init(device,
                   PVR_CMD_STREAM_TYPE_COMPUTE,
                   &sub_cmd->compute.control_stream);
      break;

   case PVR_SUB_CMD_TYPE_TRANSFER:
      sub_cmd->transfer.transfer_cmds = &sub_cmd->transfer.transfer_cmds_priv;
      list_inithead(sub_cmd->transfer.transfer_cmds);
      break;

   case PVR_SUB_CMD_TYPE_EVENT:
      break;

   default:
      unreachable("Unsupported sub-command type");
   }

   list_addtail(&sub_cmd->link, &cmd_buffer->sub_cmds);
   state->current_sub_cmd = sub_cmd;

   return VK_SUCCESS;
}

VkResult pvr_cmd_buffer_alloc_mem(struct pvr_cmd_buffer *cmd_buffer,
                                  struct pvr_winsys_heap *heap,
                                  uint64_t size,
                                  struct pvr_suballoc_bo **const pvr_bo_out)
{
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&cmd_buffer->device->pdevice->dev_info);
   struct pvr_suballoc_bo *suballoc_bo;
   struct pvr_suballocator *allocator;
   VkResult result;

   if (heap == cmd_buffer->device->heaps.general_heap)
      allocator = &cmd_buffer->device->suballoc_general;
   else if (heap == cmd_buffer->device->heaps.pds_heap)
      allocator = &cmd_buffer->device->suballoc_pds;
   else if (heap == cmd_buffer->device->heaps.transfer_frag_heap)
      allocator = &cmd_buffer->device->suballoc_transfer;
   else if (heap == cmd_buffer->device->heaps.usc_heap)
      allocator = &cmd_buffer->device->suballoc_usc;
   else
      unreachable("Unknown heap type");

   result =
      pvr_bo_suballoc(allocator, size, cache_line_size, false, &suballoc_bo);
   if (result != VK_SUCCESS)
      return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

   list_add(&suballoc_bo->link, &cmd_buffer->bo_list);

   *pvr_bo_out = suballoc_bo;

   return VK_SUCCESS;
}

static void pvr_cmd_bind_compute_pipeline(
   const struct pvr_compute_pipeline *const compute_pipeline,
   struct pvr_cmd_buffer *const cmd_buffer)
{
   cmd_buffer->state.compute_pipeline = compute_pipeline;
   cmd_buffer->state.dirty.compute_pipeline_binding = true;
}

static void pvr_cmd_bind_graphics_pipeline(
   const struct pvr_graphics_pipeline *const gfx_pipeline,
   struct pvr_cmd_buffer *const cmd_buffer)
{
   cmd_buffer->state.gfx_pipeline = gfx_pipeline;
   cmd_buffer->state.dirty.gfx_pipeline_binding = true;

   vk_cmd_set_dynamic_graphics_state(&cmd_buffer->vk,
                                     &gfx_pipeline->dynamic_state);
}

void pvr_CmdBindPipeline(VkCommandBuffer commandBuffer,
                         VkPipelineBindPoint pipelineBindPoint,
                         VkPipeline _pipeline)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_pipeline, pipeline, _pipeline);

   switch (pipelineBindPoint) {
   case VK_PIPELINE_BIND_POINT_COMPUTE:
      pvr_cmd_bind_compute_pipeline(to_pvr_compute_pipeline(pipeline),
                                    cmd_buffer);
      break;

   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      pvr_cmd_bind_graphics_pipeline(to_pvr_graphics_pipeline(pipeline),
                                     cmd_buffer);
      break;

   default:
      unreachable("Invalid bind point.");
      break;
   }
}

#if defined(DEBUG)
static void check_viewport_quirk_70165(const struct pvr_device *device,
                                       const VkViewport *pViewport)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   float min_vertex_x, max_vertex_x, min_vertex_y, max_vertex_y;
   float min_screen_space_value, max_screen_space_value;
   float sign_to_unsigned_offset, fixed_point_max;
   float guardband_width, guardband_height;

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
      /* Max representable value in 13.4 fixed point format.
       * Round-down to avoid precision issues.
       * Calculated as (2 ** 13) - 2*(2 ** -4)
       */
      fixed_point_max = 8192.0f - 2.0f / 16.0f;

      if (PVR_HAS_FEATURE(dev_info, screen_size8K)) {
         if (pViewport->width <= 4096 && pViewport->height <= 4096) {
            guardband_width = pViewport->width / 4.0f;
            guardband_height = pViewport->height / 4.0f;

            /* 2k of the range is negative */
            sign_to_unsigned_offset = 2048.0f;
         } else {
            guardband_width = 0.0f;
            guardband_height = 0.0f;

            /* For > 4k renders, the entire range is positive */
            sign_to_unsigned_offset = 0.0f;
         }
      } else {
         guardband_width = pViewport->width / 4.0f;
         guardband_height = pViewport->height / 4.0f;

         /* 2k of the range is negative */
         sign_to_unsigned_offset = 2048.0f;
      }
   } else {
      /* Max representable value in 16.8 fixed point format
       * Calculated as (2 ** 16) - (2 ** -8)
       */
      fixed_point_max = 65535.99609375f;
      guardband_width = pViewport->width / 4.0f;
      guardband_height = pViewport->height / 4.0f;

      /* 4k/20k of the range is negative */
      sign_to_unsigned_offset = (float)PVR_MAX_NEG_OFFSCREEN_OFFSET;
   }

   min_screen_space_value = -sign_to_unsigned_offset;
   max_screen_space_value = fixed_point_max - sign_to_unsigned_offset;

   min_vertex_x = pViewport->x - guardband_width;
   max_vertex_x = pViewport->x + pViewport->width + guardband_width;
   min_vertex_y = pViewport->y - guardband_height;
   max_vertex_y = pViewport->y + pViewport->height + guardband_height;
   if (min_vertex_x < min_screen_space_value ||
       max_vertex_x > max_screen_space_value ||
       min_vertex_y < min_screen_space_value ||
       max_vertex_y > max_screen_space_value) {
      mesa_logw("Viewport is affected by BRN70165, geometry outside "
                "the viewport could be corrupted");
   }
}
#endif

void pvr_CmdSetViewport(VkCommandBuffer commandBuffer,
                        uint32_t firstViewport,
                        uint32_t viewportCount,
                        const VkViewport *pViewports)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   const uint32_t total_count = firstViewport + viewportCount;

   assert(firstViewport < PVR_MAX_VIEWPORTS && viewportCount > 0);
   assert(total_count >= 1 && total_count <= PVR_MAX_VIEWPORTS);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

#if defined(DEBUG)
   if (PVR_HAS_QUIRK(&cmd_buffer->device->pdevice->dev_info, 70165)) {
      for (uint32_t viewport = 0; viewport < viewportCount; viewport++) {
         check_viewport_quirk_70165(cmd_buffer->device, &pViewports[viewport]);
      }
   }
#endif

   vk_common_CmdSetViewport(commandBuffer,
                            firstViewport,
                            viewportCount,
                            pViewports);
}

void pvr_CmdSetDepthBounds(VkCommandBuffer commandBuffer,
                           float minDepthBounds,
                           float maxDepthBounds)
{
   mesa_logd("No support for depth bounds testing.");
}

void pvr_CmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                               VkPipelineBindPoint pipelineBindPoint,
                               VkPipelineLayout _layout,
                               uint32_t firstSet,
                               uint32_t descriptorSetCount,
                               const VkDescriptorSet *pDescriptorSets,
                               uint32_t dynamicOffsetCount,
                               const uint32_t *pDynamicOffsets)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_descriptor_state *descriptor_state;

   assert(firstSet + descriptorSetCount <= PVR_MAX_DESCRIPTOR_SETS);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   switch (pipelineBindPoint) {
   case VK_PIPELINE_BIND_POINT_GRAPHICS:
   case VK_PIPELINE_BIND_POINT_COMPUTE:
      break;

   default:
      unreachable("Unsupported bind point.");
      break;
   }

   if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
      descriptor_state = &cmd_buffer->state.gfx_desc_state;
      cmd_buffer->state.dirty.gfx_desc_dirty = true;
   } else {
      descriptor_state = &cmd_buffer->state.compute_desc_state;
      cmd_buffer->state.dirty.compute_desc_dirty = true;
   }

   for (uint32_t i = 0; i < descriptorSetCount; i++) {
      PVR_FROM_HANDLE(pvr_descriptor_set, set, pDescriptorSets[i]);
      uint32_t index = firstSet + i;

      if (descriptor_state->descriptor_sets[index] != set) {
         descriptor_state->descriptor_sets[index] = set;
         descriptor_state->valid_mask |= (1u << index);
      }
   }

   if (dynamicOffsetCount > 0) {
      PVR_FROM_HANDLE(pvr_pipeline_layout, pipeline_layout, _layout);
      uint32_t set_offset = 0;

      for (uint32_t set = 0; set < firstSet; set++)
         set_offset += pipeline_layout->set_layout[set]->dynamic_buffer_count;

      assert(set_offset + dynamicOffsetCount <=
             ARRAY_SIZE(descriptor_state->dynamic_offsets));

      /* From the Vulkan 1.3.238 spec. :
       *
       *    "If any of the sets being bound include dynamic uniform or storage
       *    buffers, then pDynamicOffsets includes one element for each array
       *    element in each dynamic descriptor type binding in each set."
       *
       */
      for (uint32_t i = 0; i < dynamicOffsetCount; i++)
         descriptor_state->dynamic_offsets[set_offset + i] = pDynamicOffsets[i];
   }
}

void pvr_CmdBindVertexBuffers(VkCommandBuffer commandBuffer,
                              uint32_t firstBinding,
                              uint32_t bindingCount,
                              const VkBuffer *pBuffers,
                              const VkDeviceSize *pOffsets)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_vertex_binding *const vb = cmd_buffer->state.vertex_bindings;

   /* We have to defer setting up vertex buffer since we need the buffer
    * stride from the pipeline.
    */

   assert(firstBinding < PVR_MAX_VERTEX_INPUT_BINDINGS &&
          bindingCount <= PVR_MAX_VERTEX_INPUT_BINDINGS);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   for (uint32_t i = 0; i < bindingCount; i++) {
      vb[firstBinding + i].buffer = pvr_buffer_from_handle(pBuffers[i]);
      vb[firstBinding + i].offset = pOffsets[i];
   }

   cmd_buffer->state.dirty.vertex_bindings = true;
}

void pvr_CmdBindIndexBuffer(VkCommandBuffer commandBuffer,
                            VkBuffer buffer,
                            VkDeviceSize offset,
                            VkIndexType indexType)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_buffer, index_buffer, buffer);
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;

   assert(offset < index_buffer->vk.size);
   assert(indexType == VK_INDEX_TYPE_UINT32 ||
          indexType == VK_INDEX_TYPE_UINT16);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   state->index_buffer_binding.buffer = index_buffer;
   state->index_buffer_binding.offset = offset;
   state->index_buffer_binding.type = indexType;
   state->dirty.index_buffer_binding = true;
}

void pvr_CmdPushConstants(VkCommandBuffer commandBuffer,
                          VkPipelineLayout layout,
                          VkShaderStageFlags stageFlags,
                          uint32_t offset,
                          uint32_t size,
                          const void *pValues)
{
#if defined(DEBUG)
   const uint64_t ending = (uint64_t)offset + (uint64_t)size;
#endif

   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   pvr_assert(ending <= PVR_MAX_PUSH_CONSTANTS_SIZE);

   memcpy(&state->push_constants.data[offset], pValues, size);

   state->push_constants.dirty_stages |= stageFlags;
   state->push_constants.uploaded = false;
}

static VkResult
pvr_cmd_buffer_setup_attachments(struct pvr_cmd_buffer *cmd_buffer,
                                 const struct pvr_render_pass *pass,
                                 const struct pvr_framebuffer *framebuffer)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_render_pass_info *info = &state->render_pass_info;

   assert(pass->attachment_count == framebuffer->attachment_count);

   /* Free any previously allocated attachments. */
   vk_free(&cmd_buffer->vk.pool->alloc, state->render_pass_info.attachments);

   if (pass->attachment_count == 0) {
      info->attachments = NULL;
      return VK_SUCCESS;
   }

   info->attachments =
      vk_zalloc(&cmd_buffer->vk.pool->alloc,
                pass->attachment_count * sizeof(*info->attachments),
                8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!info->attachments) {
      return vk_command_buffer_set_error(&cmd_buffer->vk,
                                         VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   for (uint32_t i = 0; i < pass->attachment_count; i++)
      info->attachments[i] = framebuffer->attachments[i];

   return VK_SUCCESS;
}

static VkResult pvr_init_render_targets(struct pvr_device *device,
                                        struct pvr_render_pass *pass,
                                        struct pvr_framebuffer *framebuffer)
{
   for (uint32_t i = 0; i < pass->hw_setup->render_count; i++) {
      struct pvr_render_target *render_target =
         pvr_get_render_target(pass, framebuffer, i);

      pthread_mutex_lock(&render_target->mutex);

      if (!render_target->valid) {
         const struct pvr_renderpass_hwsetup_render *hw_render =
            &pass->hw_setup->renders[i];
         VkResult result;

         result = pvr_render_target_dataset_create(device,
                                                   framebuffer->width,
                                                   framebuffer->height,
                                                   hw_render->sample_count,
                                                   framebuffer->layers,
                                                   &render_target->rt_dataset);
         if (result != VK_SUCCESS) {
            pthread_mutex_unlock(&render_target->mutex);
            return result;
         }

         render_target->valid = true;
      }

      pthread_mutex_unlock(&render_target->mutex);
   }

   return VK_SUCCESS;
}

const struct pvr_renderpass_hwsetup_subpass *
pvr_get_hw_subpass(const struct pvr_render_pass *pass, const uint32_t subpass)
{
   const struct pvr_renderpass_hw_map *map =
      &pass->hw_setup->subpass_map[subpass];

   return &pass->hw_setup->renders[map->render].subpasses[map->subpass];
}

static void pvr_perform_start_of_render_attachment_clear(
   struct pvr_cmd_buffer *cmd_buffer,
   const struct pvr_framebuffer *framebuffer,
   uint32_t index,
   bool is_depth_stencil,
   uint32_t *index_list_clear_mask)
{
   ASSERTED static const VkImageAspectFlags dsc_aspect_flags =
      VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT |
      VK_IMAGE_ASPECT_COLOR_BIT;
   struct pvr_render_pass_info *info = &cmd_buffer->state.render_pass_info;
   const struct pvr_render_pass *pass = info->pass;
   const struct pvr_renderpass_hwsetup *hw_setup = pass->hw_setup;
   const struct pvr_renderpass_hwsetup_render *hw_render =
      &hw_setup->renders[hw_setup->subpass_map[info->subpass_idx].render];
   VkImageAspectFlags image_aspect;
   struct pvr_image_view *iview;
   uint32_t view_idx;

   if (is_depth_stencil) {
      bool stencil_clear;
      bool depth_clear;
      bool is_stencil;
      bool is_depth;

      assert(hw_render->ds_attach_idx != VK_ATTACHMENT_UNUSED);
      assert(index == 0);

      view_idx = hw_render->ds_attach_idx;

      is_depth = vk_format_has_depth(pass->attachments[view_idx].vk_format);
      is_stencil = vk_format_has_stencil(pass->attachments[view_idx].vk_format);
      depth_clear = hw_render->depth_init == VK_ATTACHMENT_LOAD_OP_CLEAR;
      stencil_clear = hw_render->stencil_init == VK_ATTACHMENT_LOAD_OP_CLEAR;

      /* Attempt to clear the ds attachment. Do not erroneously discard an
       * attachment that has no depth clear but has a stencil attachment.
       */
      /* if not (a ∧ c) ∨ (b ∧ d) */
      if (!((is_depth && depth_clear) || (is_stencil && stencil_clear)))
         return;
   } else if (hw_render->color_init[index].op != VK_ATTACHMENT_LOAD_OP_CLEAR) {
      return;
   } else {
      view_idx = hw_render->color_init[index].index;
   }

   iview = info->attachments[view_idx];

   /* FIXME: It would be nice if this function and pvr_sub_cmd_gfx_job_init()
    * were doing the same check (even if it's just an assert) to determine if a
    * clear is needed.
    */
   /* If this is single-layer fullscreen, we already do the clears in
    * pvr_sub_cmd_gfx_job_init().
    */
   if (pvr_is_render_area_tile_aligned(cmd_buffer, iview) &&
       framebuffer->layers == 1) {
      return;
   }

   image_aspect = vk_format_aspects(pass->attachments[view_idx].vk_format);
   assert((image_aspect & ~dsc_aspect_flags) == 0);

   if (image_aspect & VK_IMAGE_ASPECT_DEPTH_BIT &&
       hw_render->depth_init != VK_ATTACHMENT_LOAD_OP_CLEAR) {
      image_aspect &= ~VK_IMAGE_ASPECT_DEPTH_BIT;
   }

   if (image_aspect & VK_IMAGE_ASPECT_STENCIL_BIT &&
       hw_render->stencil_init != VK_ATTACHMENT_LOAD_OP_CLEAR) {
      image_aspect &= ~VK_IMAGE_ASPECT_STENCIL_BIT;
   }

   if (image_aspect != VK_IMAGE_ASPECT_NONE) {
      VkClearAttachment clear_attachment = {
         .aspectMask = image_aspect,
         .colorAttachment = index,
         .clearValue = info->clear_values[view_idx],
      };
      VkClearRect rect = {
         .rect = info->render_area,
         .baseArrayLayer = 0,
         .layerCount = info->framebuffer->layers,
      };

      assert(view_idx < info->clear_value_count);

      pvr_clear_attachments_render_init(cmd_buffer, &clear_attachment, &rect);

      *index_list_clear_mask |= (1 << index);
   }
}

static void
pvr_perform_start_of_render_clears(struct pvr_cmd_buffer *cmd_buffer)
{
   struct pvr_render_pass_info *info = &cmd_buffer->state.render_pass_info;
   const struct pvr_framebuffer *framebuffer = info->framebuffer;
   const struct pvr_render_pass *pass = info->pass;
   const struct pvr_renderpass_hwsetup *hw_setup = pass->hw_setup;
   const struct pvr_renderpass_hwsetup_render *hw_render =
      &hw_setup->renders[hw_setup->subpass_map[info->subpass_idx].render];

   /* Mask of attachment clears using index lists instead of background object
    * to clear.
    */
   uint32_t index_list_clear_mask = 0;

   for (uint32_t i = 0; i < hw_render->color_init_count; i++) {
      pvr_perform_start_of_render_attachment_clear(cmd_buffer,
                                                   framebuffer,
                                                   i,
                                                   false,
                                                   &index_list_clear_mask);
   }

   info->enable_bg_tag = !!hw_render->color_init_count;

   /* If we're not using index list for all clears/loads then we need to run
    * the background object on empty tiles.
    */
   if (hw_render->color_init_count &&
       index_list_clear_mask != ((1u << hw_render->color_init_count) - 1u)) {
      info->process_empty_tiles = true;
   } else {
      info->process_empty_tiles = false;
   }

   if (hw_render->ds_attach_idx != VK_ATTACHMENT_UNUSED) {
      uint32_t ds_index_list = 0;

      pvr_perform_start_of_render_attachment_clear(cmd_buffer,
                                                   framebuffer,
                                                   0,
                                                   true,
                                                   &ds_index_list);
   }

   if (index_list_clear_mask)
      pvr_finishme("Add support for generating loadops shaders!");
}

static void pvr_stash_depth_format(struct pvr_cmd_buffer_state *state,
                                   struct pvr_sub_cmd_gfx *const sub_cmd)
{
   const struct pvr_render_pass *pass = state->render_pass_info.pass;
   const struct pvr_renderpass_hwsetup_render *hw_render =
      &pass->hw_setup->renders[sub_cmd->hw_render_idx];

   if (hw_render->ds_attach_idx != VK_ATTACHMENT_UNUSED) {
      struct pvr_image_view **iviews = state->render_pass_info.attachments;

      state->depth_format = iviews[hw_render->ds_attach_idx]->vk.format;
   }
}

static bool pvr_loadops_contain_clear(struct pvr_renderpass_hwsetup *hw_setup)
{
   for (uint32_t i = 0; i < hw_setup->render_count; i++) {
      struct pvr_renderpass_hwsetup_render *hw_render = &hw_setup->renders[i];
      uint32_t render_targets_count = hw_render->init_setup.num_render_targets;

      for (uint32_t j = 0;
           j < (hw_render->color_init_count * render_targets_count);
           j += render_targets_count) {
         for (uint32_t k = 0; k < hw_render->init_setup.num_render_targets;
              k++) {
            if (hw_render->color_init[j + k].op ==
                VK_ATTACHMENT_LOAD_OP_CLEAR) {
               return true;
            }
         }
      }
      if (hw_render->depth_init == VK_ATTACHMENT_LOAD_OP_CLEAR ||
          hw_render->stencil_init == VK_ATTACHMENT_LOAD_OP_CLEAR) {
         return true;
      }
   }

   return false;
}

static VkResult
pvr_cmd_buffer_set_clear_values(struct pvr_cmd_buffer *cmd_buffer,
                                const VkRenderPassBeginInfo *pRenderPassBegin)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;

   /* Free any previously allocated clear values. */
   vk_free(&cmd_buffer->vk.pool->alloc, state->render_pass_info.clear_values);

   if (pRenderPassBegin->clearValueCount) {
      const size_t size = pRenderPassBegin->clearValueCount *
                          sizeof(*state->render_pass_info.clear_values);

      state->render_pass_info.clear_values =
         vk_zalloc(&cmd_buffer->vk.pool->alloc,
                   size,
                   8,
                   VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
      if (!state->render_pass_info.clear_values) {
         return vk_command_buffer_set_error(&cmd_buffer->vk,
                                            VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      memcpy(state->render_pass_info.clear_values,
             pRenderPassBegin->pClearValues,
             size);
   } else {
      state->render_pass_info.clear_values = NULL;
   }

   state->render_pass_info.clear_value_count =
      pRenderPassBegin->clearValueCount;

   return VK_SUCCESS;
}

/**
 * \brief Indicates whether to use the large or normal clear state words.
 *
 * If the current render area can fit within a quarter of the max framebuffer
 * that the device is capable of, we can use the normal clear state words,
 * otherwise the large clear state words are needed.
 *
 * The requirement of a quarter of the max framebuffer comes from the index
 * count used in the normal clear state words and the vertices uploaded at
 * device creation.
 *
 * \param[in] cmd_buffer The command buffer for the clear.
 * \return true if large clear state words are required.
 */
static bool
pvr_is_large_clear_required(const struct pvr_cmd_buffer *const cmd_buffer)
{
   const struct pvr_device_info *const dev_info =
      &cmd_buffer->device->pdevice->dev_info;
   const VkRect2D render_area = cmd_buffer->state.render_pass_info.render_area;
   const uint32_t vf_max_x = rogue_get_param_vf_max_x(dev_info);
   const uint32_t vf_max_y = rogue_get_param_vf_max_x(dev_info);

   return (render_area.extent.width > (vf_max_x / 2) - 1) ||
          (render_area.extent.height > (vf_max_y / 2) - 1);
}

static void pvr_emit_clear_words(struct pvr_cmd_buffer *const cmd_buffer,
                                 struct pvr_sub_cmd_gfx *const sub_cmd)
{
   struct pvr_device *device = cmd_buffer->device;
   struct pvr_csb *csb = &sub_cmd->control_stream;
   uint32_t vdm_state_size_in_dw;
   const uint32_t *vdm_state;
   uint32_t *stream;

   vdm_state_size_in_dw =
      pvr_clear_vdm_state_get_size_in_dw(&device->pdevice->dev_info, 1);

   pvr_csb_set_relocation_mark(csb);

   stream = pvr_csb_alloc_dwords(csb, vdm_state_size_in_dw);
   if (!stream) {
      pvr_cmd_buffer_set_error_unwarned(cmd_buffer, csb->status);
      return;
   }

   if (pvr_is_large_clear_required(cmd_buffer))
      vdm_state = device->static_clear_state.large_clear_vdm_words;
   else
      vdm_state = device->static_clear_state.vdm_words;

   memcpy(stream, vdm_state, PVR_DW_TO_BYTES(vdm_state_size_in_dw));

   pvr_csb_clear_relocation_mark(csb);
}

static VkResult pvr_cs_write_load_op(struct pvr_cmd_buffer *cmd_buffer,
                                     struct pvr_sub_cmd_gfx *sub_cmd,
                                     struct pvr_load_op *load_op,
                                     uint32_t isp_userpass)
{
   const struct pvr_device *device = cmd_buffer->device;
   struct pvr_static_clear_ppp_template template =
      device->static_clear_state.ppp_templates[VK_IMAGE_ASPECT_COLOR_BIT];
   uint32_t pds_state[PVR_STATIC_CLEAR_PDS_STATE_COUNT];
   struct pvr_pds_upload shareds_update_program;
   struct pvr_suballoc_bo *pvr_bo;
   VkResult result;

   result = pvr_load_op_data_create_and_upload(cmd_buffer,
                                               load_op,
                                               &shareds_update_program);
   if (result != VK_SUCCESS)
      return result;

   template.config.ispctl.upass = isp_userpass;

   /* It might look odd that we aren't specifying the code segment's
    * address anywhere. This is because the hardware always assumes that the
    * data size is 2 128bit words and the code segments starts after that.
    */
   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_SHADERBASE],
                 TA_STATE_PDS_SHADERBASE,
                 shaderbase) {
      shaderbase.addr = PVR_DEV_ADDR(load_op->pds_frag_prog.data_offset);
   }

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_TEXUNICODEBASE],
                 TA_STATE_PDS_TEXUNICODEBASE,
                 texunicodebase) {
      texunicodebase.addr =
         PVR_DEV_ADDR(load_op->pds_tex_state_prog.code_offset);
   }

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_SIZEINFO1],
                 TA_STATE_PDS_SIZEINFO1,
                 sizeinfo1) {
      /* Dummy coefficient loading program. */
      sizeinfo1.pds_varyingsize = 0;

      sizeinfo1.pds_texturestatesize = DIV_ROUND_UP(
         shareds_update_program.data_size,
         PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEXTURESTATESIZE_UNIT_SIZE));

      sizeinfo1.pds_tempsize =
         DIV_ROUND_UP(load_op->temps_count,
                      PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEMPSIZE_UNIT_SIZE));
   }

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_SIZEINFO2],
                 TA_STATE_PDS_SIZEINFO2,
                 sizeinfo2) {
      sizeinfo2.usc_sharedsize =
         DIV_ROUND_UP(load_op->const_shareds_count,
                      PVRX(TA_STATE_PDS_SIZEINFO2_USC_SHAREDSIZE_UNIT_SIZE));
   }

   /* Dummy coefficient loading program. */
   pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_VARYINGBASE] = 0;

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_TEXTUREDATABASE],
                 TA_STATE_PDS_TEXTUREDATABASE,
                 texturedatabase) {
      texturedatabase.addr = PVR_DEV_ADDR(shareds_update_program.data_offset);
   }

   template.config.pds_state = &pds_state;

   pvr_emit_ppp_from_template(&sub_cmd->control_stream, &template, &pvr_bo);
   list_add(&pvr_bo->link, &cmd_buffer->bo_list);

   pvr_emit_clear_words(cmd_buffer, sub_cmd);

   pvr_reset_graphics_dirty_state(cmd_buffer, false);

   return VK_SUCCESS;
}

void pvr_CmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                             const VkRenderPassBeginInfo *pRenderPassBeginInfo,
                             const VkSubpassBeginInfo *pSubpassBeginInfo)
{
   PVR_FROM_HANDLE(pvr_framebuffer,
                   framebuffer,
                   pRenderPassBeginInfo->framebuffer);
   PVR_FROM_HANDLE(pvr_render_pass, pass, pRenderPassBeginInfo->renderPass);
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   const struct pvr_renderpass_hwsetup_subpass *hw_subpass;
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   assert(!state->render_pass_info.pass);
   assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   /* FIXME: Create a separate function for everything using pass->subpasses,
    * look at cmd_buffer_begin_subpass() for example. */
   state->render_pass_info.pass = pass;
   state->render_pass_info.framebuffer = framebuffer;
   state->render_pass_info.subpass_idx = 0;
   state->render_pass_info.render_area = pRenderPassBeginInfo->renderArea;
   state->render_pass_info.current_hw_subpass = 0;
   state->render_pass_info.pipeline_bind_point =
      pass->subpasses[0].pipeline_bind_point;
   state->render_pass_info.isp_userpass = pass->subpasses[0].isp_userpass;
   state->dirty.isp_userpass = true;

   result = pvr_cmd_buffer_setup_attachments(cmd_buffer, pass, framebuffer);
   if (result != VK_SUCCESS)
      return;

   result = pvr_init_render_targets(cmd_buffer->device, pass, framebuffer);
   if (result != VK_SUCCESS) {
      pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
      return;
   }

   result = pvr_cmd_buffer_set_clear_values(cmd_buffer, pRenderPassBeginInfo);
   if (result != VK_SUCCESS)
      return;

   assert(pass->subpasses[0].pipeline_bind_point ==
          VK_PIPELINE_BIND_POINT_GRAPHICS);

   result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_GRAPHICS);
   if (result != VK_SUCCESS)
      return;

   /* Run subpass 0 "soft" background object after the actual background
    * object.
    */
   hw_subpass = pvr_get_hw_subpass(pass, 0);
   if (hw_subpass->load_op) {
      result = pvr_cs_write_load_op(cmd_buffer,
                                    &cmd_buffer->state.current_sub_cmd->gfx,
                                    hw_subpass->load_op,
                                    0);
      if (result != VK_SUCCESS)
         return;
   }

   pvr_perform_start_of_render_clears(cmd_buffer);
   pvr_stash_depth_format(&cmd_buffer->state,
                          &cmd_buffer->state.current_sub_cmd->gfx);
}

VkResult pvr_BeginCommandBuffer(VkCommandBuffer commandBuffer,
                                const VkCommandBufferBeginInfo *pBeginInfo)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *state;
   VkResult result;

   vk_command_buffer_begin(&cmd_buffer->vk, pBeginInfo);

   cmd_buffer->usage_flags = pBeginInfo->flags;
   state = &cmd_buffer->state;

   /* VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT must be ignored for
    * primary level command buffers.
    *
    * From the Vulkan 1.0 spec:
    *
    *    VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT specifies that a
    *    secondary command buffer is considered to be entirely inside a render
    *    pass. If this is a primary command buffer, then this bit is ignored.
    */
   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
      cmd_buffer->usage_flags &=
         ~VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
   }

   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
      if (cmd_buffer->usage_flags &
          VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
         const VkCommandBufferInheritanceInfo *inheritance_info =
            pBeginInfo->pInheritanceInfo;
         struct pvr_render_pass *pass;

         pass = pvr_render_pass_from_handle(inheritance_info->renderPass);
         state->render_pass_info.pass = pass;
         state->render_pass_info.framebuffer =
            pvr_framebuffer_from_handle(inheritance_info->framebuffer);
         state->render_pass_info.subpass_idx = inheritance_info->subpass;
         state->render_pass_info.isp_userpass =
            pass->subpasses[inheritance_info->subpass].isp_userpass;

         result =
            pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_GRAPHICS);
         if (result != VK_SUCCESS)
            return result;

         state->vis_test_enabled = inheritance_info->occlusionQueryEnable;
      }

      state->dirty.isp_userpass = true;
   }

   util_dynarray_init(&state->query_indices, NULL);

   memset(state->barriers_needed,
          0xFF,
          sizeof(*state->barriers_needed) * ARRAY_SIZE(state->barriers_needed));

   return VK_SUCCESS;
}

VkResult pvr_cmd_buffer_add_transfer_cmd(struct pvr_cmd_buffer *cmd_buffer,
                                         struct pvr_transfer_cmd *transfer_cmd)
{
   struct pvr_sub_cmd_transfer *sub_cmd;
   VkResult result;

   result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_TRANSFER);
   if (result != VK_SUCCESS)
      return result;

   sub_cmd = &cmd_buffer->state.current_sub_cmd->transfer;

   list_addtail(&transfer_cmd->link, sub_cmd->transfer_cmds);

   return VK_SUCCESS;
}

static VkResult
pvr_setup_vertex_buffers(struct pvr_cmd_buffer *cmd_buffer,
                         const struct pvr_graphics_pipeline *const gfx_pipeline)
{
   const struct pvr_vertex_shader_state *const vertex_state =
      &gfx_pipeline->shader_state.vertex;
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   const struct pvr_pds_info *const pds_info = state->pds_shader.info;
   struct pvr_suballoc_bo *pvr_bo;
   const uint8_t *entries;
   uint32_t *dword_buffer;
   uint64_t *qword_buffer;
   VkResult result;

   result =
      pvr_cmd_buffer_alloc_mem(cmd_buffer,
                               cmd_buffer->device->heaps.pds_heap,
                               PVR_DW_TO_BYTES(pds_info->data_size_in_dwords),
                               &pvr_bo);
   if (result != VK_SUCCESS)
      return result;

   dword_buffer = (uint32_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);
   qword_buffer = (uint64_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);

   entries = (uint8_t *)pds_info->entries;

   for (uint32_t i = 0; i < pds_info->entry_count; i++) {
      const struct pvr_const_map_entry *const entry_header =
         (struct pvr_const_map_entry *)entries;

      switch (entry_header->type) {
      case PVR_PDS_CONST_MAP_ENTRY_TYPE_LITERAL32: {
         const struct pvr_const_map_entry_literal32 *const literal =
            (struct pvr_const_map_entry_literal32 *)entries;

         PVR_WRITE(dword_buffer,
                   literal->literal_value,
                   literal->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*literal);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_DOUTU_ADDRESS: {
         const struct pvr_const_map_entry_doutu_address *const doutu_addr =
            (struct pvr_const_map_entry_doutu_address *)entries;
         const pvr_dev_addr_t exec_addr =
            PVR_DEV_ADDR_OFFSET(vertex_state->bo->dev_addr,
                                vertex_state->entry_offset);
         uint64_t addr = 0ULL;

         pvr_set_usc_execution_address64(&addr, exec_addr.addr);

         PVR_WRITE(qword_buffer,
                   addr | doutu_addr->doutu_control,
                   doutu_addr->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*doutu_addr);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_BASE_INSTANCE: {
         const struct pvr_const_map_entry_base_instance *const base_instance =
            (struct pvr_const_map_entry_base_instance *)entries;

         PVR_WRITE(dword_buffer,
                   state->draw_state.base_instance,
                   base_instance->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*base_instance);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_BASE_VERTEX: {
         const struct pvr_const_map_entry_base_instance *const base_instance =
            (struct pvr_const_map_entry_base_instance *)entries;

         PVR_WRITE(dword_buffer,
                   state->draw_state.base_vertex,
                   base_instance->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*base_instance);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_VERTEX_ATTRIBUTE_ADDRESS: {
         const struct pvr_const_map_entry_vertex_attribute_address
            *const attribute =
               (struct pvr_const_map_entry_vertex_attribute_address *)entries;
         const struct pvr_vertex_binding *const binding =
            &state->vertex_bindings[attribute->binding_index];
         /* In relation to the Vulkan spec. 22.4. Vertex Input Address
          * Calculation:
          *    Adding binding->offset corresponds to calculating the
          *    `bufferBindingAddress`. Adding attribute->offset corresponds to
          *    adding the `attribDesc.offset`. The `effectiveVertexOffset` is
          *    taken care by the PDS program itself with a DDMAD which will
          *    multiply the vertex/instance idx with the binding's stride and
          *    add that to the address provided here.
          */
         const pvr_dev_addr_t addr =
            PVR_DEV_ADDR_OFFSET(binding->buffer->dev_addr,
                                binding->offset + attribute->offset);

         PVR_WRITE(qword_buffer,
                   addr.addr,
                   attribute->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*attribute);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_ROBUST_VERTEX_ATTRIBUTE_ADDRESS: {
         const struct pvr_const_map_entry_robust_vertex_attribute_address
            *const attribute =
               (struct pvr_const_map_entry_robust_vertex_attribute_address *)
                  entries;
         const struct pvr_vertex_binding *const binding =
            &state->vertex_bindings[attribute->binding_index];
         pvr_dev_addr_t addr;

         if (binding->buffer->vk.size <
             (attribute->offset + attribute->component_size_in_bytes)) {
            /* Replace with load from robustness buffer when no attribute is in
             * range
             */
            addr = PVR_DEV_ADDR_OFFSET(
               cmd_buffer->device->robustness_buffer->vma->dev_addr,
               attribute->robustness_buffer_offset);
         } else {
            addr = PVR_DEV_ADDR_OFFSET(binding->buffer->dev_addr,
                                       binding->offset + attribute->offset);
         }

         PVR_WRITE(qword_buffer,
                   addr.addr,
                   attribute->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*attribute);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_VERTEX_ATTRIBUTE_MAX_INDEX: {
         const struct pvr_const_map_entry_vertex_attribute_max_index *attribute =
            (struct pvr_const_map_entry_vertex_attribute_max_index *)entries;
         const struct pvr_vertex_binding *const binding =
            &state->vertex_bindings[attribute->binding_index];
         const uint64_t bound_size = binding->buffer->vk.size - binding->offset;
         const uint32_t attribute_end =
            attribute->offset + attribute->component_size_in_bytes;
         uint32_t max_index;

         if (PVR_HAS_FEATURE(&cmd_buffer->device->pdevice->dev_info,
                             pds_ddmadt)) {
            /* TODO: PVR_PDS_CONST_MAP_ENTRY_TYPE_VERTEX_ATTRIBUTE_MAX_INDEX
             * has the same define value as
             * PVR_PDS_CONST_MAP_ENTRY_TYPE_VERTEX_ATTR_DDMADT_OOB_BUFFER_SIZE
             * so maybe we want to remove one of the defines or change the
             * values.
             */
            pvr_finishme("Unimplemented robust buffer access with DDMADT");
            assert(false);
         }

         /* If the stride is 0 then all attributes use the same single element
          * from the binding so the index can only be up to 0.
          */
         if (bound_size < attribute_end || attribute->stride == 0) {
            max_index = 0;
         } else {
            max_index = (uint32_t)(bound_size / attribute->stride) - 1;

            /* There's one last attribute that can fit in. */
            if (bound_size % attribute->stride >= attribute_end)
               max_index++;
         }

         PVR_WRITE(dword_buffer,
                   max_index,
                   attribute->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*attribute);
         break;
      }

      default:
         unreachable("Unsupported data section map");
         break;
      }
   }

   state->pds_vertex_attrib_offset =
      pvr_bo->dev_addr.addr -
      cmd_buffer->device->heaps.pds_heap->base_addr.addr;

   return VK_SUCCESS;
}

static VkResult pvr_setup_descriptor_mappings_old(
   struct pvr_cmd_buffer *const cmd_buffer,
   enum pvr_stage_allocation stage,
   const struct pvr_stage_allocation_descriptor_state *descriptor_state,
   const pvr_dev_addr_t *const num_worgroups_buff_addr,
   uint32_t *const descriptor_data_offset_out)
{
   const struct pvr_pds_info *const pds_info = &descriptor_state->pds_info;
   const struct pvr_descriptor_state *desc_state;
   struct pvr_suballoc_bo *pvr_bo;
   const uint8_t *entries;
   uint32_t *dword_buffer;
   uint64_t *qword_buffer;
   VkResult result;

   if (!pds_info->data_size_in_dwords)
      return VK_SUCCESS;

   result =
      pvr_cmd_buffer_alloc_mem(cmd_buffer,
                               cmd_buffer->device->heaps.pds_heap,
                               PVR_DW_TO_BYTES(pds_info->data_size_in_dwords),
                               &pvr_bo);
   if (result != VK_SUCCESS)
      return result;

   dword_buffer = (uint32_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);
   qword_buffer = (uint64_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);

   entries = (uint8_t *)pds_info->entries;

   switch (stage) {
   case PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY:
   case PVR_STAGE_ALLOCATION_FRAGMENT:
      desc_state = &cmd_buffer->state.gfx_desc_state;
      break;

   case PVR_STAGE_ALLOCATION_COMPUTE:
      desc_state = &cmd_buffer->state.compute_desc_state;
      break;

   default:
      unreachable("Unsupported stage.");
      break;
   }

   for (uint32_t i = 0; i < pds_info->entry_count; i++) {
      const struct pvr_const_map_entry *const entry_header =
         (struct pvr_const_map_entry *)entries;

      switch (entry_header->type) {
      case PVR_PDS_CONST_MAP_ENTRY_TYPE_LITERAL32: {
         const struct pvr_const_map_entry_literal32 *const literal =
            (struct pvr_const_map_entry_literal32 *)entries;

         PVR_WRITE(dword_buffer,
                   literal->literal_value,
                   literal->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*literal);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_CONSTANT_BUFFER: {
         const struct pvr_const_map_entry_constant_buffer *const_buffer_entry =
            (struct pvr_const_map_entry_constant_buffer *)entries;
         const uint32_t desc_set = const_buffer_entry->desc_set;
         const uint32_t binding = const_buffer_entry->binding;
         const struct pvr_descriptor_set *descriptor_set;
         const struct pvr_descriptor *descriptor;
         pvr_dev_addr_t buffer_addr;

         assert(desc_set < PVR_MAX_DESCRIPTOR_SETS);
         descriptor_set = desc_state->descriptor_sets[desc_set];

         /* TODO: Handle dynamic buffers. */
         descriptor = &descriptor_set->descriptors[binding];
         assert(descriptor->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

         assert(descriptor->buffer_desc_range ==
                PVR_DW_TO_BYTES(const_buffer_entry->size_in_dwords));
         assert(descriptor->buffer_whole_range ==
                PVR_DW_TO_BYTES(const_buffer_entry->size_in_dwords));

         buffer_addr =
            PVR_DEV_ADDR_OFFSET(descriptor->buffer_dev_addr,
                                const_buffer_entry->offset * sizeof(uint32_t));

         PVR_WRITE(qword_buffer,
                   buffer_addr.addr,
                   const_buffer_entry->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*const_buffer_entry);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_DESCRIPTOR_SET: {
         const struct pvr_const_map_entry_descriptor_set *desc_set_entry =
            (struct pvr_const_map_entry_descriptor_set *)entries;
         const uint32_t desc_set_num = desc_set_entry->descriptor_set;
         const struct pvr_descriptor_set *descriptor_set;
         pvr_dev_addr_t desc_set_addr;
         uint64_t desc_portion_offset;

         assert(desc_set_num < PVR_MAX_DESCRIPTOR_SETS);

         /* TODO: Remove this when the compiler provides us with usage info?
          */
         /* We skip DMAing unbound descriptor sets. */
         if (!(desc_state->valid_mask & BITFIELD_BIT(desc_set_num))) {
            const struct pvr_const_map_entry_literal32 *literal;
            uint32_t zero_literal_value;

            /* The code segment contains a DOUT instructions so in the data
             * section we have to write a DOUTD_SRC0 and DOUTD_SRC1.
             * We'll write 0 for DOUTD_SRC0 since we don't have a buffer to DMA.
             * We're expecting a LITERAL32 entry containing the value for
             * DOUTD_SRC1 next so let's make sure we get it and write it
             * with BSIZE to 0 disabling the DMA operation.
             * We don't want the LITERAL32 to be processed as normal otherwise
             * we'd be DMAing from an address of 0.
             */

            entries += sizeof(*desc_set_entry);
            literal = (struct pvr_const_map_entry_literal32 *)entries;

            assert(literal->type == PVR_PDS_CONST_MAP_ENTRY_TYPE_LITERAL32);

            zero_literal_value =
               literal->literal_value &
               PVR_ROGUE_PDSINST_DOUT_FIELDS_DOUTD_SRC1_BSIZE_CLRMSK;

            PVR_WRITE(qword_buffer,
                      UINT64_C(0),
                      desc_set_entry->const_offset,
                      pds_info->data_size_in_dwords);

            PVR_WRITE(dword_buffer,
                      zero_literal_value,
                      desc_set_entry->const_offset,
                      pds_info->data_size_in_dwords);

            entries += sizeof(*literal);
            i++;
            continue;
         }

         descriptor_set = desc_state->descriptor_sets[desc_set_num];

         desc_set_addr = descriptor_set->pvr_bo->dev_addr;

         if (desc_set_entry->primary) {
            desc_portion_offset =
               descriptor_set->layout->memory_layout_in_dwords_per_stage[stage]
                  .primary_offset;
         } else {
            desc_portion_offset =
               descriptor_set->layout->memory_layout_in_dwords_per_stage[stage]
                  .secondary_offset;
         }
         desc_portion_offset = PVR_DW_TO_BYTES(desc_portion_offset);

         desc_set_addr =
            PVR_DEV_ADDR_OFFSET(desc_set_addr, desc_portion_offset);

         desc_set_addr = PVR_DEV_ADDR_OFFSET(
            desc_set_addr,
            PVR_DW_TO_BYTES((uint64_t)desc_set_entry->offset_in_dwords));

         PVR_WRITE(qword_buffer,
                   desc_set_addr.addr,
                   desc_set_entry->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*desc_set_entry);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_SPECIAL_BUFFER: {
         const struct pvr_const_map_entry_special_buffer *special_buff_entry =
            (struct pvr_const_map_entry_special_buffer *)entries;

         switch (special_buff_entry->buffer_type) {
         case PVR_BUFFER_TYPE_COMPILE_TIME: {
            uint64_t addr = descriptor_state->static_consts->dev_addr.addr;

            PVR_WRITE(qword_buffer,
                      addr,
                      special_buff_entry->const_offset,
                      pds_info->data_size_in_dwords);
            break;
         }

         case PVR_BUFFER_TYPE_BLEND_CONSTS:
            /* TODO: See if instead of reusing the blend constant buffer type
             * entry, we can setup a new buffer type specifically for
             * num_workgroups or other built-in variables. The mappings are
             * setup at pipeline creation when creating the descriptor program.
             */
            if (stage == PVR_STAGE_ALLOCATION_COMPUTE) {
               assert(num_worgroups_buff_addr->addr);

               /* TODO: Check if we need to offset this (e.g. for just y and z),
                * or cope with any reordering?
                */
               PVR_WRITE(qword_buffer,
                         num_worgroups_buff_addr->addr,
                         special_buff_entry->const_offset,
                         pds_info->data_size_in_dwords);
            } else {
               pvr_finishme("Add blend constants support.");
            }
            break;

         default:
            unreachable("Unsupported special buffer type.");
         }

         entries += sizeof(*special_buff_entry);
         break;
      }

      default:
         unreachable("Unsupported map entry type.");
      }
   }

   *descriptor_data_offset_out =
      pvr_bo->dev_addr.addr -
      cmd_buffer->device->heaps.pds_heap->base_addr.addr;

   return VK_SUCCESS;
}

/* Note that the descriptor set doesn't have any space for dynamic buffer
 * descriptors so this works on the assumption that you have a buffer with space
 * for them at the end.
 */
static uint16_t pvr_get_dynamic_descriptor_primary_offset(
   const struct pvr_device *device,
   const struct pvr_descriptor_set_layout *layout,
   const struct pvr_descriptor_set_layout_binding *binding,
   const uint32_t stage,
   const uint32_t desc_idx)
{
   struct pvr_descriptor_size_info size_info;
   uint32_t offset;

   assert(binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
          binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
   assert(desc_idx < binding->descriptor_count);

   pvr_descriptor_size_info_init(device, binding->type, &size_info);

   offset = layout->total_size_in_dwords;
   offset += binding->per_stage_offset_in_dwords[stage].primary;
   offset += (desc_idx * size_info.primary);

   /* Offset must be less than * 16bits. */
   assert(offset < UINT16_MAX);

   return (uint16_t)offset;
}

/* Note that the descriptor set doesn't have any space for dynamic buffer
 * descriptors so this works on the assumption that you have a buffer with space
 * for them at the end.
 */
static uint16_t pvr_get_dynamic_descriptor_secondary_offset(
   const struct pvr_device *device,
   const struct pvr_descriptor_set_layout *layout,
   const struct pvr_descriptor_set_layout_binding *binding,
   const uint32_t stage,
   const uint32_t desc_idx)
{
   struct pvr_descriptor_size_info size_info;
   uint32_t offset;

   assert(binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
          binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
   assert(desc_idx < binding->descriptor_count);

   pvr_descriptor_size_info_init(device, binding->type, &size_info);

   offset = layout->total_size_in_dwords;
   offset +=
      layout->memory_layout_in_dwords_per_stage[stage].primary_dynamic_size;
   offset += binding->per_stage_offset_in_dwords[stage].secondary;
   offset += (desc_idx * size_info.secondary);

   /* Offset must be less than * 16bits. */
   assert(offset < UINT16_MAX);

   return (uint16_t)offset;
}

/**
 * \brief Upload a copy of the descriptor set with dynamic buffer offsets
 * applied.
 */
/* TODO: We should probably make the compiler aware of the dynamic descriptors.
 * We could use push constants like Anv seems to do. This would avoid having to
 * duplicate all sets containing dynamic descriptors each time the offsets are
 * updated.
 */
static VkResult pvr_cmd_buffer_upload_patched_desc_set(
   struct pvr_cmd_buffer *cmd_buffer,
   const struct pvr_descriptor_set *desc_set,
   const uint32_t *dynamic_offsets,
   struct pvr_suballoc_bo **const bo_out)
{
   const struct pvr_descriptor_set_layout *layout = desc_set->layout;
   const uint64_t normal_desc_set_size =
      PVR_DW_TO_BYTES(layout->total_size_in_dwords);
   const uint64_t dynamic_descs_size =
      PVR_DW_TO_BYTES(layout->total_dynamic_size_in_dwords);
   struct pvr_descriptor_size_info dynamic_uniform_buffer_size_info;
   struct pvr_descriptor_size_info dynamic_storage_buffer_size_info;
   struct pvr_device *device = cmd_buffer->device;
   struct pvr_suballoc_bo *patched_desc_set_bo;
   uint32_t *src_mem_ptr, *dst_mem_ptr;
   uint32_t desc_idx_offset = 0;
   VkResult result;

   assert(desc_set->layout->dynamic_buffer_count > 0);

   pvr_descriptor_size_info_init(device,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                 &dynamic_uniform_buffer_size_info);
   pvr_descriptor_size_info_init(device,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                 &dynamic_storage_buffer_size_info);

   /* TODO: In the descriptor set we don't account for dynamic buffer
    * descriptors and take care of them in the pipeline layout. The pipeline
    * layout allocates them at the beginning but let's put them at the end just
    * because it makes things a bit easier. Ideally we should be using the
    * pipeline layout and use the offsets from the pipeline layout to patch
    * descriptors.
    */
   result = pvr_cmd_buffer_alloc_mem(cmd_buffer,
                                     cmd_buffer->device->heaps.general_heap,
                                     normal_desc_set_size + dynamic_descs_size,
                                     &patched_desc_set_bo);
   if (result != VK_SUCCESS)
      return result;

   src_mem_ptr = (uint32_t *)pvr_bo_suballoc_get_map_addr(desc_set->pvr_bo);
   dst_mem_ptr = (uint32_t *)pvr_bo_suballoc_get_map_addr(patched_desc_set_bo);

   memcpy(dst_mem_ptr, src_mem_ptr, normal_desc_set_size);

   for (uint32_t i = 0; i < desc_set->layout->binding_count; i++) {
      const struct pvr_descriptor_set_layout_binding *binding =
         &desc_set->layout->bindings[i];
      const struct pvr_descriptor *descriptors =
         &desc_set->descriptors[binding->descriptor_index];
      const struct pvr_descriptor_size_info *size_info;

      if (binding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
         size_info = &dynamic_uniform_buffer_size_info;
      else if (binding->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
         size_info = &dynamic_storage_buffer_size_info;
      else
         continue;

      for (uint32_t stage = 0; stage < PVR_STAGE_ALLOCATION_COUNT; stage++) {
         uint32_t primary_offset;
         uint32_t secondary_offset;

         if (!(binding->shader_stage_mask & BITFIELD_BIT(stage)))
            continue;

         /* Get the offsets for the first dynamic descriptor in the current
          * binding.
          */
         primary_offset =
            pvr_get_dynamic_descriptor_primary_offset(device,
                                                      desc_set->layout,
                                                      binding,
                                                      stage,
                                                      0);
         secondary_offset =
            pvr_get_dynamic_descriptor_secondary_offset(device,
                                                        desc_set->layout,
                                                        binding,
                                                        stage,
                                                        0);

         /* clang-format off */
         for (uint32_t desc_idx = 0;
              desc_idx < binding->descriptor_count;
              desc_idx++) {
            /* clang-format on */
            const pvr_dev_addr_t addr =
               PVR_DEV_ADDR_OFFSET(descriptors[desc_idx].buffer_dev_addr,
                                   dynamic_offsets[desc_idx + desc_idx_offset]);
            const VkDeviceSize range =
               MIN2(descriptors[desc_idx].buffer_desc_range,
                    descriptors[desc_idx].buffer_whole_range -
                       dynamic_offsets[desc_idx]);

#if defined(DEBUG)
            uint32_t desc_primary_offset;
            uint32_t desc_secondary_offset;

            desc_primary_offset =
               pvr_get_dynamic_descriptor_primary_offset(device,
                                                         desc_set->layout,
                                                         binding,
                                                         stage,
                                                         desc_idx);
            desc_secondary_offset =
               pvr_get_dynamic_descriptor_secondary_offset(device,
                                                           desc_set->layout,
                                                           binding,
                                                           stage,
                                                           desc_idx);

            /* Check the assumption that the descriptors within a binding, for
             * a particular stage, are allocated consecutively.
             */
            assert(desc_primary_offset ==
                   primary_offset + size_info->primary * desc_idx);
            assert(desc_secondary_offset ==
                   secondary_offset + size_info->secondary * desc_idx);
#endif

            assert(descriptors[desc_idx].type == binding->type);

            memcpy(dst_mem_ptr + primary_offset + size_info->primary * desc_idx,
                   &addr.addr,
                   PVR_DW_TO_BYTES(size_info->primary));
            memcpy(dst_mem_ptr + secondary_offset +
                      size_info->secondary * desc_idx,
                   &range,
                   PVR_DW_TO_BYTES(size_info->secondary));
         }
      }

      desc_idx_offset += binding->descriptor_count;
   }

   *bo_out = patched_desc_set_bo;

   return VK_SUCCESS;
}

#define PVR_SELECT(_geom, _frag, _compute)         \
   (stage == PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY) \
      ? (_geom)                                    \
      : (stage == PVR_STAGE_ALLOCATION_FRAGMENT) ? (_frag) : (_compute)

static VkResult
pvr_cmd_buffer_upload_desc_set_table(struct pvr_cmd_buffer *const cmd_buffer,
                                     enum pvr_stage_allocation stage,
                                     pvr_dev_addr_t *addr_out)
{
   uint64_t bound_desc_sets[PVR_MAX_DESCRIPTOR_SETS];
   const struct pvr_descriptor_state *desc_state;
   struct pvr_suballoc_bo *suballoc_bo;
   uint32_t dynamic_offset_idx = 0;
   VkResult result;

   switch (stage) {
   case PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY:
   case PVR_STAGE_ALLOCATION_FRAGMENT:
   case PVR_STAGE_ALLOCATION_COMPUTE:
      break;

   default:
      unreachable("Unsupported stage.");
      break;
   }

   desc_state = PVR_SELECT(&cmd_buffer->state.gfx_desc_state,
                           &cmd_buffer->state.gfx_desc_state,
                           &cmd_buffer->state.compute_desc_state);

   for (uint32_t set = 0; set < ARRAY_SIZE(bound_desc_sets); set++)
      bound_desc_sets[set] = ~0;

   assert(util_last_bit(desc_state->valid_mask) <= ARRAY_SIZE(bound_desc_sets));
   for (uint32_t set = 0; set < util_last_bit(desc_state->valid_mask); set++) {
      const struct pvr_descriptor_set *desc_set;

      if (!(desc_state->valid_mask & BITFIELD_BIT(set))) {
         const struct pvr_pipeline_layout *pipeline_layout =
            PVR_SELECT(cmd_buffer->state.gfx_pipeline->base.layout,
                       cmd_buffer->state.gfx_pipeline->base.layout,
                       cmd_buffer->state.compute_pipeline->base.layout);
         const struct pvr_descriptor_set_layout *set_layout;

         assert(set <= pipeline_layout->set_count);

         set_layout = pipeline_layout->set_layout[set];
         dynamic_offset_idx += set_layout->dynamic_buffer_count;

         continue;
      }

      desc_set = desc_state->descriptor_sets[set];

      /* TODO: Is it better if we don't set the valid_mask for empty sets? */
      if (desc_set->layout->descriptor_count == 0)
         continue;

      if (desc_set->layout->dynamic_buffer_count > 0) {
         struct pvr_suballoc_bo *new_desc_set_bo;

         assert(dynamic_offset_idx + desc_set->layout->dynamic_buffer_count <=
                ARRAY_SIZE(desc_state->dynamic_offsets));

         result = pvr_cmd_buffer_upload_patched_desc_set(
            cmd_buffer,
            desc_set,
            &desc_state->dynamic_offsets[dynamic_offset_idx],
            &new_desc_set_bo);
         if (result != VK_SUCCESS)
            return result;

         dynamic_offset_idx += desc_set->layout->dynamic_buffer_count;

         bound_desc_sets[set] = new_desc_set_bo->dev_addr.addr;
      } else {
         bound_desc_sets[set] = desc_set->pvr_bo->dev_addr.addr;
      }
   }

   result = pvr_cmd_buffer_upload_general(cmd_buffer,
                                          bound_desc_sets,
                                          sizeof(bound_desc_sets),
                                          &suballoc_bo);
   if (result != VK_SUCCESS)
      return result;

   *addr_out = suballoc_bo->dev_addr;
   return VK_SUCCESS;
}

static VkResult
pvr_process_addr_literal(struct pvr_cmd_buffer *cmd_buffer,
                         enum pvr_pds_addr_literal_type addr_literal_type,
                         enum pvr_stage_allocation stage,
                         pvr_dev_addr_t *addr_out)
{
   VkResult result;

   switch (addr_literal_type) {
   case PVR_PDS_ADDR_LITERAL_DESC_SET_ADDRS_TABLE: {
      /* TODO: Maybe we want to free pvr_bo? And only when the data
       * section is written successfully we link all bos to the command
       * buffer.
       */
      result =
         pvr_cmd_buffer_upload_desc_set_table(cmd_buffer, stage, addr_out);
      if (result != VK_SUCCESS)
         return result;

      break;
   }

   case PVR_PDS_ADDR_LITERAL_PUSH_CONSTS: {
      const struct pvr_pipeline_layout *layout =
         PVR_SELECT(cmd_buffer->state.gfx_pipeline->base.layout,
                    cmd_buffer->state.gfx_pipeline->base.layout,
                    cmd_buffer->state.compute_pipeline->base.layout);
      const uint32_t push_constants_offset =
         PVR_SELECT(layout->vert_push_constants_offset,
                    layout->frag_push_constants_offset,
                    layout->compute_push_constants_offset);

      *addr_out = PVR_DEV_ADDR_OFFSET(cmd_buffer->state.push_constants.dev_addr,
                                      push_constants_offset);
      break;
   }

   case PVR_PDS_ADDR_LITERAL_BLEND_CONSTANTS: {
      float *blend_consts =
         cmd_buffer->vk.dynamic_graphics_state.cb.blend_constants;
      size_t size =
         sizeof(cmd_buffer->vk.dynamic_graphics_state.cb.blend_constants);
      struct pvr_suballoc_bo *blend_consts_bo;

      result = pvr_cmd_buffer_upload_general(cmd_buffer,
                                             blend_consts,
                                             size,
                                             &blend_consts_bo);
      if (result != VK_SUCCESS)
         return result;

      *addr_out = blend_consts_bo->dev_addr;

      break;
   }

   default:
      unreachable("Invalid add literal type.");
   }

   return VK_SUCCESS;
}

#undef PVR_SELECT

static VkResult pvr_setup_descriptor_mappings_new(
   struct pvr_cmd_buffer *const cmd_buffer,
   enum pvr_stage_allocation stage,
   const struct pvr_stage_allocation_descriptor_state *descriptor_state,
   uint32_t *const descriptor_data_offset_out)
{
   const struct pvr_pds_info *const pds_info = &descriptor_state->pds_info;
   struct pvr_suballoc_bo *pvr_bo;
   const uint8_t *entries;
   uint32_t *dword_buffer;
   uint64_t *qword_buffer;
   VkResult result;

   if (!pds_info->data_size_in_dwords)
      return VK_SUCCESS;

   result =
      pvr_cmd_buffer_alloc_mem(cmd_buffer,
                               cmd_buffer->device->heaps.pds_heap,
                               PVR_DW_TO_BYTES(pds_info->data_size_in_dwords),
                               &pvr_bo);
   if (result != VK_SUCCESS)
      return result;

   dword_buffer = (uint32_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);
   qword_buffer = (uint64_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);

   entries = (uint8_t *)pds_info->entries;

   switch (stage) {
   case PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY:
   case PVR_STAGE_ALLOCATION_FRAGMENT:
   case PVR_STAGE_ALLOCATION_COMPUTE:
      break;

   default:
      unreachable("Unsupported stage.");
      break;
   }

   for (uint32_t i = 0; i < pds_info->entry_count; i++) {
      const struct pvr_const_map_entry *const entry_header =
         (struct pvr_const_map_entry *)entries;

      switch (entry_header->type) {
      case PVR_PDS_CONST_MAP_ENTRY_TYPE_LITERAL32: {
         const struct pvr_const_map_entry_literal32 *const literal =
            (struct pvr_const_map_entry_literal32 *)entries;

         PVR_WRITE(dword_buffer,
                   literal->literal_value,
                   literal->const_offset,
                   pds_info->data_size_in_dwords);

         entries += sizeof(*literal);
         break;
      }

      case PVR_PDS_CONST_MAP_ENTRY_TYPE_ADDR_LITERAL_BUFFER: {
         const struct pvr_pds_const_map_entry_addr_literal_buffer
            *const addr_literal_buffer_entry =
               (struct pvr_pds_const_map_entry_addr_literal_buffer *)entries;
         struct pvr_device *device = cmd_buffer->device;
         struct pvr_suballoc_bo *addr_literal_buffer_bo;
         uint32_t addr_literal_count = 0;
         uint64_t *addr_literal_buffer;

         result = pvr_cmd_buffer_alloc_mem(cmd_buffer,
                                           device->heaps.general_heap,
                                           addr_literal_buffer_entry->size,
                                           &addr_literal_buffer_bo);
         if (result != VK_SUCCESS)
            return result;

         addr_literal_buffer =
            (uint64_t *)pvr_bo_suballoc_get_map_addr(addr_literal_buffer_bo);

         entries += sizeof(*addr_literal_buffer_entry);

         PVR_WRITE(qword_buffer,
                   addr_literal_buffer_bo->dev_addr.addr,
                   addr_literal_buffer_entry->const_offset,
                   pds_info->data_size_in_dwords);

         for (uint32_t j = i + 1; j < pds_info->entry_count; j++) {
            const struct pvr_const_map_entry *const entry_header =
               (struct pvr_const_map_entry *)entries;
            const struct pvr_pds_const_map_entry_addr_literal *addr_literal;
            pvr_dev_addr_t dev_addr;

            if (entry_header->type != PVR_PDS_CONST_MAP_ENTRY_TYPE_ADDR_LITERAL)
               break;

            addr_literal =
               (struct pvr_pds_const_map_entry_addr_literal *)entries;

            result = pvr_process_addr_literal(cmd_buffer,
                                              addr_literal->addr_type,
                                              stage,
                                              &dev_addr);
            if (result != VK_SUCCESS)
               return result;

            addr_literal_buffer[addr_literal_count++] = dev_addr.addr;

            entries += sizeof(*addr_literal);
         }

         assert(addr_literal_count * sizeof(uint64_t) ==
                addr_literal_buffer_entry->size);

         i += addr_literal_count;

         break;
      }

      default:
         unreachable("Unsupported map entry type.");
      }
   }

   *descriptor_data_offset_out =
      pvr_bo->dev_addr.addr -
      cmd_buffer->device->heaps.pds_heap->base_addr.addr;

   return VK_SUCCESS;
}

static VkResult pvr_setup_descriptor_mappings(
   struct pvr_cmd_buffer *const cmd_buffer,
   enum pvr_stage_allocation stage,
   const struct pvr_stage_allocation_descriptor_state *descriptor_state,
   const pvr_dev_addr_t *const num_worgroups_buff_addr,
   uint32_t *const descriptor_data_offset_out)
{
   const bool old_path =
      pvr_has_hard_coded_shaders(&cmd_buffer->device->pdevice->dev_info);

   if (old_path) {
      return pvr_setup_descriptor_mappings_old(cmd_buffer,
                                               stage,
                                               descriptor_state,
                                               num_worgroups_buff_addr,
                                               descriptor_data_offset_out);
   }

   return pvr_setup_descriptor_mappings_new(cmd_buffer,
                                            stage,
                                            descriptor_state,
                                            descriptor_data_offset_out);
}

static void pvr_compute_update_shared(struct pvr_cmd_buffer *cmd_buffer,
                                      struct pvr_sub_cmd_compute *const sub_cmd)
{
   const struct pvr_device *device = cmd_buffer->device;
   const struct pvr_physical_device *pdevice = device->pdevice;
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_csb *csb = &sub_cmd->control_stream;
   const struct pvr_compute_pipeline *pipeline = state->compute_pipeline;
   const uint32_t const_shared_regs =
      pipeline->shader_state.const_shared_reg_count;
   struct pvr_compute_kernel_info info;

   /* No shared regs, no need to use an allocation kernel. */
   if (!const_shared_regs)
      return;

   /* Accumulate the MAX number of shared registers across the kernels in this
    * dispatch. This is used by the FW for context switching, so must be large
    * enough to contain all the shared registers that might be in use for this
    * compute job. Coefficients don't need to be included as the context switch
    * will not happen within the execution of a single workgroup, thus nothing
    * needs to be preserved.
    */
   state->max_shared_regs = MAX2(state->max_shared_regs, const_shared_regs);

   info = (struct pvr_compute_kernel_info){
      .indirect_buffer_addr = PVR_DEV_ADDR_INVALID,
      .sd_type = PVRX(CDMCTRL_SD_TYPE_NONE),

      .usc_target = PVRX(CDMCTRL_USC_TARGET_ALL),
      .usc_common_shared = true,
      .usc_common_size =
         DIV_ROUND_UP(const_shared_regs,
                      PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE)),

      .local_size = { 1, 1, 1 },
      .global_size = { 1, 1, 1 },
   };

   /* Sometimes we don't have a secondary program if there were no constants to
    * write, but we still need to run a PDS program to accomplish the
    * allocation of the local/common store shared registers. Use the
    * pre-uploaded empty PDS program in this instance.
    */
   if (pipeline->descriptor_state.pds_info.code_size_in_dwords) {
      uint32_t pds_data_size_in_dwords =
         pipeline->descriptor_state.pds_info.data_size_in_dwords;

      info.pds_data_offset = state->pds_compute_descriptor_data_offset;
      info.pds_data_size =
         DIV_ROUND_UP(PVR_DW_TO_BYTES(pds_data_size_in_dwords),
                      PVRX(CDMCTRL_KERNEL0_PDS_DATA_SIZE_UNIT_SIZE));

      /* Check that we have upload the code section. */
      assert(pipeline->descriptor_state.pds_code.code_size);
      info.pds_code_offset = pipeline->descriptor_state.pds_code.code_offset;
   } else {
      const struct pvr_pds_upload *program = &device->pds_compute_empty_program;

      info.pds_data_offset = program->data_offset;
      info.pds_data_size =
         DIV_ROUND_UP(PVR_DW_TO_BYTES(program->data_size),
                      PVRX(CDMCTRL_KERNEL0_PDS_DATA_SIZE_UNIT_SIZE));
      info.pds_code_offset = program->code_offset;
   }

   /* We don't need to pad the workgroup size. */

   info.max_instances =
      pvr_compute_flat_slot_size(pdevice, const_shared_regs, false, 1U);

   pvr_compute_generate_control_stream(csb, sub_cmd, &info);
}

void pvr_compute_update_shared_private(
   struct pvr_cmd_buffer *cmd_buffer,
   struct pvr_sub_cmd_compute *const sub_cmd,
   struct pvr_private_compute_pipeline *pipeline)
{
   const struct pvr_physical_device *pdevice = cmd_buffer->device->pdevice;
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   const uint32_t const_shared_regs = pipeline->const_shared_regs_count;
   struct pvr_csb *csb = &sub_cmd->control_stream;
   struct pvr_compute_kernel_info info;

   /* No shared regs, no need to use an allocation kernel. */
   if (!const_shared_regs)
      return;

   /* See comment in pvr_compute_update_shared() for details on this. */
   state->max_shared_regs = MAX2(state->max_shared_regs, const_shared_regs);

   info = (struct pvr_compute_kernel_info){
      .indirect_buffer_addr = PVR_DEV_ADDR_INVALID,
      .usc_common_size =
         DIV_ROUND_UP(const_shared_regs,
                      PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE)),
      .pds_data_size =
         DIV_ROUND_UP(PVR_DW_TO_BYTES(pipeline->pds_shared_update_data_size_dw),
                      PVRX(CDMCTRL_KERNEL0_PDS_DATA_SIZE_UNIT_SIZE)),
      .usc_target = PVRX(CDMCTRL_USC_TARGET_ALL),
      .pds_data_offset = pipeline->pds_shared_update_data_offset,
      .pds_code_offset = pipeline->pds_shared_update_code_offset,
      .sd_type = PVRX(CDMCTRL_SD_TYPE_NONE),
      .usc_common_shared = true,
      .local_size = { 1, 1, 1 },
      .global_size = { 1, 1, 1 },
   };

   /* We don't need to pad the workgroup size. */

   info.max_instances =
      pvr_compute_flat_slot_size(pdevice, const_shared_regs, false, 1U);

   pvr_compute_generate_control_stream(csb, sub_cmd, &info);
}

static uint32_t
pvr_compute_flat_pad_workgroup_size(const struct pvr_physical_device *pdevice,
                                    uint32_t workgroup_size,
                                    uint32_t coeff_regs_count)
{
   const struct pvr_device_runtime_info *dev_runtime_info =
      &pdevice->dev_runtime_info;
   const struct pvr_device_info *dev_info = &pdevice->dev_info;
   uint32_t max_avail_coeff_regs =
      dev_runtime_info->cdm_max_local_mem_size_regs;
   uint32_t coeff_regs_count_aligned =
      ALIGN_POT(coeff_regs_count,
                PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE) >> 2U);

   /* If the work group size is > ROGUE_MAX_INSTANCES_PER_TASK. We now *always*
    * pad the work group size to the next multiple of
    * ROGUE_MAX_INSTANCES_PER_TASK.
    *
    * If we use more than 1/8th of the max coefficient registers then we round
    * work group size up to the next multiple of ROGUE_MAX_INSTANCES_PER_TASK
    */
   /* TODO: See if this can be optimized. */
   if (workgroup_size > ROGUE_MAX_INSTANCES_PER_TASK ||
       coeff_regs_count_aligned > (max_avail_coeff_regs / 8)) {
      assert(workgroup_size < rogue_get_compute_max_work_group_size(dev_info));

      return ALIGN_POT(workgroup_size, ROGUE_MAX_INSTANCES_PER_TASK);
   }

   return workgroup_size;
}

void pvr_compute_update_kernel_private(
   struct pvr_cmd_buffer *cmd_buffer,
   struct pvr_sub_cmd_compute *const sub_cmd,
   struct pvr_private_compute_pipeline *pipeline,
   const uint32_t global_workgroup_size[static const PVR_WORKGROUP_DIMENSIONS])
{
   const struct pvr_physical_device *pdevice = cmd_buffer->device->pdevice;
   const struct pvr_device_runtime_info *dev_runtime_info =
      &pdevice->dev_runtime_info;
   struct pvr_csb *csb = &sub_cmd->control_stream;

   struct pvr_compute_kernel_info info = {
      .indirect_buffer_addr = PVR_DEV_ADDR_INVALID,
      .usc_target = PVRX(CDMCTRL_USC_TARGET_ANY),
      .pds_temp_size =
         DIV_ROUND_UP(pipeline->pds_temps_used << 2U,
                      PVRX(CDMCTRL_KERNEL0_PDS_TEMP_SIZE_UNIT_SIZE)),

      .pds_data_size =
         DIV_ROUND_UP(PVR_DW_TO_BYTES(pipeline->pds_data_size_dw),
                      PVRX(CDMCTRL_KERNEL0_PDS_DATA_SIZE_UNIT_SIZE)),
      .pds_data_offset = pipeline->pds_data_offset,
      .pds_code_offset = pipeline->pds_code_offset,

      .sd_type = PVRX(CDMCTRL_SD_TYPE_NONE),

      .usc_unified_size =
         DIV_ROUND_UP(pipeline->unified_store_regs_count << 2U,
                      PVRX(CDMCTRL_KERNEL0_USC_UNIFIED_SIZE_UNIT_SIZE)),

      /* clang-format off */
      .global_size = {
         global_workgroup_size[0],
         global_workgroup_size[1],
         global_workgroup_size[2]
      },
      /* clang-format on */
   };

   uint32_t work_size = pipeline->workgroup_size.width *
                        pipeline->workgroup_size.height *
                        pipeline->workgroup_size.depth;
   uint32_t coeff_regs;

   if (work_size > ROGUE_MAX_INSTANCES_PER_TASK) {
      /* Enforce a single workgroup per cluster through allocation starvation.
       */
      coeff_regs = dev_runtime_info->cdm_max_local_mem_size_regs;
   } else {
      coeff_regs = pipeline->coeff_regs_count;
   }

   info.usc_common_size =
      DIV_ROUND_UP(PVR_DW_TO_BYTES(coeff_regs),
                   PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE));

   /* Use a whole slot per workgroup. */
   work_size = MAX2(work_size, ROGUE_MAX_INSTANCES_PER_TASK);

   coeff_regs += pipeline->const_shared_regs_count;

   if (pipeline->const_shared_regs_count > 0)
      info.sd_type = PVRX(CDMCTRL_SD_TYPE_USC);

   work_size =
      pvr_compute_flat_pad_workgroup_size(pdevice, work_size, coeff_regs);

   info.local_size[0] = work_size;
   info.local_size[1] = 1U;
   info.local_size[2] = 1U;

   info.max_instances =
      pvr_compute_flat_slot_size(pdevice, coeff_regs, false, work_size);

   pvr_compute_generate_control_stream(csb, sub_cmd, &info);
}

/* TODO: Wire up the base_workgroup variant program when implementing
 * VK_KHR_device_group. The values will also need patching into the program.
 */
static void pvr_compute_update_kernel(
   struct pvr_cmd_buffer *cmd_buffer,
   struct pvr_sub_cmd_compute *const sub_cmd,
   pvr_dev_addr_t indirect_addr,
   const uint32_t global_workgroup_size[static const PVR_WORKGROUP_DIMENSIONS])
{
   const struct pvr_physical_device *pdevice = cmd_buffer->device->pdevice;
   const struct pvr_device_runtime_info *dev_runtime_info =
      &pdevice->dev_runtime_info;
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_csb *csb = &sub_cmd->control_stream;
   const struct pvr_compute_pipeline *pipeline = state->compute_pipeline;
   const struct pvr_compute_shader_state *shader_state =
      &pipeline->shader_state;
   const struct pvr_pds_info *program_info = &pipeline->primary_program_info;

   struct pvr_compute_kernel_info info = {
      .indirect_buffer_addr = indirect_addr,
      .usc_target = PVRX(CDMCTRL_USC_TARGET_ANY),
      .pds_temp_size =
         DIV_ROUND_UP(program_info->temps_required << 2U,
                      PVRX(CDMCTRL_KERNEL0_PDS_TEMP_SIZE_UNIT_SIZE)),

      .pds_data_size =
         DIV_ROUND_UP(PVR_DW_TO_BYTES(program_info->data_size_in_dwords),
                      PVRX(CDMCTRL_KERNEL0_PDS_DATA_SIZE_UNIT_SIZE)),
      .pds_data_offset = pipeline->primary_program.data_offset,
      .pds_code_offset = pipeline->primary_program.code_offset,

      .sd_type = PVRX(CDMCTRL_SD_TYPE_NONE),

      .usc_unified_size =
         DIV_ROUND_UP(shader_state->input_register_count << 2U,
                      PVRX(CDMCTRL_KERNEL0_USC_UNIFIED_SIZE_UNIT_SIZE)),

      /* clang-format off */
      .global_size = {
         global_workgroup_size[0],
         global_workgroup_size[1],
         global_workgroup_size[2]
      },
      /* clang-format on */
   };

   uint32_t work_size = shader_state->work_size;
   uint32_t coeff_regs;

   if (work_size > ROGUE_MAX_INSTANCES_PER_TASK) {
      /* Enforce a single workgroup per cluster through allocation starvation.
       */
      coeff_regs = dev_runtime_info->cdm_max_local_mem_size_regs;
   } else {
      coeff_regs = shader_state->coefficient_register_count;
   }

   info.usc_common_size =
      DIV_ROUND_UP(PVR_DW_TO_BYTES(coeff_regs),
                   PVRX(CDMCTRL_KERNEL0_USC_COMMON_SIZE_UNIT_SIZE));

   /* Use a whole slot per workgroup. */
   work_size = MAX2(work_size, ROGUE_MAX_INSTANCES_PER_TASK);

   coeff_regs += shader_state->const_shared_reg_count;

   if (shader_state->const_shared_reg_count > 0)
      info.sd_type = PVRX(CDMCTRL_SD_TYPE_USC);

   work_size =
      pvr_compute_flat_pad_workgroup_size(pdevice, work_size, coeff_regs);

   info.local_size[0] = work_size;
   info.local_size[1] = 1U;
   info.local_size[2] = 1U;

   info.max_instances =
      pvr_compute_flat_slot_size(pdevice, coeff_regs, false, work_size);

   pvr_compute_generate_control_stream(csb, sub_cmd, &info);
}

static VkResult pvr_cmd_upload_push_consts(struct pvr_cmd_buffer *cmd_buffer)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_suballoc_bo *suballoc_bo;
   VkResult result;

   /* TODO: Here are some possible optimizations/things to consider:
    *
    *    - Currently we upload maxPushConstantsSize. The application might only
    *      be using a portion of that so we might end up with unused memory.
    *      Should we be smarter about this. If we intend to upload the push
    *      consts into shareds, we definitely want to do avoid reserving unused
    *      regs.
    *
    *    - For now we have to upload to a new buffer each time since the shaders
    *      access the push constants from memory. If we were to reuse the same
    *      buffer we might update the contents out of sync with job submission
    *      and the shaders will see the updated contents while the command
    *      buffer was still being recorded and not yet submitted.
    *      If we were to upload the push constants directly to shared regs we
    *      could reuse the same buffer (avoiding extra allocation overhead)
    *      since the contents will be DMAed only on job submission when the
    *      control stream is processed and the PDS program is executed. This
    *      approach would also allow us to avoid regenerating the PDS data
    *      section in some cases since the buffer address will be constants.
    */

   if (cmd_buffer->state.push_constants.uploaded)
      return VK_SUCCESS;

   result = pvr_cmd_buffer_upload_general(cmd_buffer,
                                          state->push_constants.data,
                                          sizeof(state->push_constants.data),
                                          &suballoc_bo);
   if (result != VK_SUCCESS)
      return result;

   cmd_buffer->state.push_constants.dev_addr = suballoc_bo->dev_addr;
   cmd_buffer->state.push_constants.uploaded = true;

   return VK_SUCCESS;
}

static void pvr_cmd_dispatch(
   struct pvr_cmd_buffer *const cmd_buffer,
   const pvr_dev_addr_t indirect_addr,
   const uint32_t workgroup_size[static const PVR_WORKGROUP_DIMENSIONS])
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   const struct pvr_compute_pipeline *compute_pipeline =
      state->compute_pipeline;
   struct pvr_sub_cmd_compute *sub_cmd;
   VkResult result;

   pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_COMPUTE);

   sub_cmd = &state->current_sub_cmd->compute;
   sub_cmd->uses_atomic_ops |= compute_pipeline->shader_state.uses_atomic_ops;
   sub_cmd->uses_barrier |= compute_pipeline->shader_state.uses_barrier;

   if (state->push_constants.dirty_stages & VK_SHADER_STAGE_COMPUTE_BIT) {
      result = pvr_cmd_upload_push_consts(cmd_buffer);
      if (result != VK_SUCCESS)
         return;

      /* Regenerate the PDS program to use the new push consts buffer. */
      state->dirty.compute_desc_dirty = true;

      state->push_constants.dirty_stages &= ~VK_SHADER_STAGE_COMPUTE_BIT;
   }

   if (compute_pipeline->shader_state.uses_num_workgroups) {
      pvr_dev_addr_t descriptor_data_offset_out;

      if (indirect_addr.addr) {
         descriptor_data_offset_out = indirect_addr;
      } else {
         struct pvr_suballoc_bo *num_workgroups_bo;

         result = pvr_cmd_buffer_upload_general(cmd_buffer,
                                                workgroup_size,
                                                sizeof(*workgroup_size) *
                                                   PVR_WORKGROUP_DIMENSIONS,
                                                &num_workgroups_bo);
         if (result != VK_SUCCESS)
            return;

         descriptor_data_offset_out = num_workgroups_bo->dev_addr;
      }

      result = pvr_setup_descriptor_mappings(
         cmd_buffer,
         PVR_STAGE_ALLOCATION_COMPUTE,
         &compute_pipeline->descriptor_state,
         &descriptor_data_offset_out,
         &state->pds_compute_descriptor_data_offset);
      if (result != VK_SUCCESS)
         return;
   } else if ((compute_pipeline->base.layout
                  ->per_stage_descriptor_masks[PVR_STAGE_ALLOCATION_COMPUTE] &&
               state->dirty.compute_desc_dirty) ||
              state->dirty.compute_pipeline_binding) {
      result = pvr_setup_descriptor_mappings(
         cmd_buffer,
         PVR_STAGE_ALLOCATION_COMPUTE,
         &compute_pipeline->descriptor_state,
         NULL,
         &state->pds_compute_descriptor_data_offset);
      if (result != VK_SUCCESS)
         return;
   }

   pvr_compute_update_shared(cmd_buffer, sub_cmd);
   pvr_compute_update_kernel(cmd_buffer, sub_cmd, indirect_addr, workgroup_size);
}

void pvr_CmdDispatch(VkCommandBuffer commandBuffer,
                     uint32_t groupCountX,
                     uint32_t groupCountY,
                     uint32_t groupCountZ)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   if (!groupCountX || !groupCountY || !groupCountZ)
      return;

   pvr_cmd_dispatch(cmd_buffer,
                    PVR_DEV_ADDR_INVALID,
                    (uint32_t[]){ groupCountX, groupCountY, groupCountZ });
}

void pvr_CmdDispatchIndirect(VkCommandBuffer commandBuffer,
                             VkBuffer _buffer,
                             VkDeviceSize offset)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_buffer, buffer, _buffer);

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   pvr_cmd_dispatch(cmd_buffer,
                    PVR_DEV_ADDR_OFFSET(buffer->dev_addr, offset),
                    (uint32_t[]){ 1, 1, 1 });
}

static void
pvr_update_draw_state(struct pvr_cmd_buffer_state *const state,
                      const struct pvr_cmd_buffer_draw_state *const draw_state)
{
   /* We don't have a state to tell us that base_instance is being used so it
    * gets used as a boolean - 0 means we'll use a pds program that skips the
    * base instance addition. If the base_instance gets used (and the last
    * draw's base_instance was 0) then we switch to the BASE_INSTANCE attrib
    * program.
    *
    * If base_instance changes then we only need to update the data section.
    *
    * The only draw call state that doesn't really matter is the start vertex
    * as that is handled properly in the VDM state in all cases.
    */
   if ((state->draw_state.draw_indexed != draw_state->draw_indexed) ||
       (state->draw_state.draw_indirect != draw_state->draw_indirect) ||
       (state->draw_state.base_instance == 0 &&
        draw_state->base_instance != 0)) {
      state->dirty.draw_variant = true;
   } else if (state->draw_state.base_instance != draw_state->base_instance) {
      state->dirty.draw_base_instance = true;
   }

   state->draw_state = *draw_state;
}

static uint32_t pvr_calc_shared_regs_count(
   const struct pvr_graphics_pipeline *const gfx_pipeline)
{
   const struct pvr_pipeline_stage_state *const vertex_state =
      &gfx_pipeline->shader_state.vertex.stage_state;

   uint32_t shared_regs = vertex_state->const_shared_reg_count +
                          vertex_state->const_shared_reg_offset;

   if (gfx_pipeline->shader_state.fragment.bo) {
      const struct pvr_pipeline_stage_state *const fragment_state =
         &gfx_pipeline->shader_state.fragment.stage_state;

      uint32_t fragment_regs = fragment_state->const_shared_reg_count +
                               fragment_state->const_shared_reg_offset;

      shared_regs = MAX2(shared_regs, fragment_regs);
   }

   return shared_regs;
}

static void
pvr_emit_dirty_pds_state(const struct pvr_cmd_buffer *const cmd_buffer,
                         struct pvr_sub_cmd_gfx *const sub_cmd,
                         const uint32_t pds_vertex_descriptor_data_offset)
{
   const struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   const struct pvr_stage_allocation_descriptor_state
      *const vertex_descriptor_state =
         &state->gfx_pipeline->shader_state.vertex.descriptor_state;
   const struct pvr_pipeline_stage_state *const vertex_stage_state =
      &state->gfx_pipeline->shader_state.vertex.stage_state;
   struct pvr_csb *const csb = &sub_cmd->control_stream;

   if (!vertex_descriptor_state->pds_info.code_size_in_dwords)
      return;

   pvr_csb_set_relocation_mark(csb);

   pvr_csb_emit (csb, VDMCTRL_PDS_STATE0, state0) {
      state0.usc_target = PVRX(VDMCTRL_USC_TARGET_ALL);

      state0.usc_common_size =
         DIV_ROUND_UP(vertex_stage_state->const_shared_reg_count << 2,
                      PVRX(VDMCTRL_PDS_STATE0_USC_COMMON_SIZE_UNIT_SIZE));

      state0.pds_data_size = DIV_ROUND_UP(
         PVR_DW_TO_BYTES(vertex_descriptor_state->pds_info.data_size_in_dwords),
         PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE));
   }

   pvr_csb_emit (csb, VDMCTRL_PDS_STATE1, state1) {
      state1.pds_data_addr = PVR_DEV_ADDR(pds_vertex_descriptor_data_offset);
      state1.sd_type = PVRX(VDMCTRL_SD_TYPE_NONE);
   }

   pvr_csb_emit (csb, VDMCTRL_PDS_STATE2, state2) {
      state2.pds_code_addr =
         PVR_DEV_ADDR(vertex_descriptor_state->pds_code.code_offset);
   }

   pvr_csb_clear_relocation_mark(csb);
}

static void pvr_setup_output_select(struct pvr_cmd_buffer *const cmd_buffer)
{
   const struct pvr_graphics_pipeline *const gfx_pipeline =
      cmd_buffer->state.gfx_pipeline;
   const struct pvr_vertex_shader_state *const vertex_state =
      &gfx_pipeline->shader_state.vertex;
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct PVRX(TA_STATE_HEADER) *const header = &cmd_buffer->state.emit_header;
   struct pvr_ppp_state *const ppp_state = &cmd_buffer->state.ppp_state;
   uint32_t output_selects;

   /* TODO: Handle vertex and fragment shader state flags. */

   pvr_csb_pack (&output_selects, TA_OUTPUT_SEL, state) {
      state.rhw_pres = true;
      state.vtxsize = DIV_ROUND_UP(vertex_state->vertex_output_size, 4U);
      state.psprite_size_pres = (dynamic_state->ia.primitive_topology ==
                                 VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
   }

   if (ppp_state->output_selects != output_selects) {
      ppp_state->output_selects = output_selects;
      header->pres_outselects = true;
   }

   if (ppp_state->varying_word[0] != vertex_state->varying[0]) {
      ppp_state->varying_word[0] = vertex_state->varying[0];
      header->pres_varying_word0 = true;
   }

   if (ppp_state->varying_word[1] != vertex_state->varying[1]) {
      ppp_state->varying_word[1] = vertex_state->varying[1];
      header->pres_varying_word1 = true;
   }
}

static void
pvr_setup_isp_faces_and_control(struct pvr_cmd_buffer *const cmd_buffer,
                                struct PVRX(TA_STATE_ISPA) *const ispa_out)
{
   struct PVRX(TA_STATE_HEADER) *const header = &cmd_buffer->state.emit_header;
   const struct pvr_fragment_shader_state *const fragment_shader_state =
      &cmd_buffer->state.gfx_pipeline->shader_state.fragment;
   const struct pvr_render_pass_info *const pass_info =
      &cmd_buffer->state.render_pass_info;
   struct vk_dynamic_graphics_state *dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct pvr_ppp_state *const ppp_state = &cmd_buffer->state.ppp_state;

   const bool rasterizer_discard = dynamic_state->rs.rasterizer_discard_enable;
   const uint32_t subpass_idx = pass_info->subpass_idx;
   const uint32_t depth_stencil_attachment_idx =
      pass_info->pass->subpasses[subpass_idx].depth_stencil_attachment;
   const struct pvr_render_pass_attachment *const attachment =
      depth_stencil_attachment_idx != VK_ATTACHMENT_UNUSED
         ? &pass_info->pass->attachments[depth_stencil_attachment_idx]
         : NULL;

   const enum PVRX(TA_OBJTYPE)
      obj_type = pvr_ta_objtype(dynamic_state->ia.primitive_topology);

   const VkImageAspectFlags ds_aspects =
      (!rasterizer_discard && attachment)
         ? vk_format_aspects(attachment->vk_format) &
              (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
         : VK_IMAGE_ASPECT_NONE;

   /* This is deliberately a full copy rather than a pointer because
    * vk_optimize_depth_stencil_state() can only be run once against any given
    * instance of vk_depth_stencil_state.
    */
   struct vk_depth_stencil_state ds_state = dynamic_state->ds;

   uint32_t ispb_stencil_off;
   bool is_two_sided = false;
   uint32_t isp_control;

   uint32_t line_width;
   uint32_t common_a;
   uint32_t front_a;
   uint32_t front_b;
   uint32_t back_a;
   uint32_t back_b;

   vk_optimize_depth_stencil_state(&ds_state, ds_aspects, true);

   /* Convert to 4.4 fixed point format. */
   line_width = util_unsigned_fixed(dynamic_state->rs.line.width, 4);

   /* Subtract 1 to shift values from range [0=0,256=16] to [0=1/16,255=16].
    * If 0 it stays at 0, otherwise we subtract 1.
    */
   line_width = (!!line_width) * (line_width - 1);

   line_width = MIN2(line_width, PVRX(TA_STATE_ISPA_POINTLINEWIDTH_SIZE_MAX));

   /* TODO: Part of the logic in this function is duplicated in another part
    * of the code. E.g. the dcmpmode, and sop1/2/3. Could we do this earlier?
    */

   pvr_csb_pack (&common_a, TA_STATE_ISPA, ispa) {
      ispa.pointlinewidth = line_width;

      ispa.dcmpmode = pvr_ta_cmpmode(ds_state.depth.compare_op);
      ispa.dwritedisable = !ds_state.depth.write_enable;

      ispa.passtype = fragment_shader_state->pass_type;

      ispa.objtype = obj_type;

      /* Return unpacked ispa structure. dcmpmode, dwritedisable, passtype and
       * objtype are needed by pvr_setup_triangle_merging_flag.
       */
      if (ispa_out)
         *ispa_out = ispa;
   }

   /* TODO: Does this actually represent the ispb control word on stencil off?
    * If not, rename the variable.
    */
   pvr_csb_pack (&ispb_stencil_off, TA_STATE_ISPB, ispb) {
      ispb.sop3 = PVRX(TA_ISPB_STENCILOP_KEEP);
      ispb.sop2 = PVRX(TA_ISPB_STENCILOP_KEEP);
      ispb.sop1 = PVRX(TA_ISPB_STENCILOP_KEEP);
      ispb.scmpmode = PVRX(TA_CMPMODE_ALWAYS);
   }

   /* FIXME: This logic should be redone and improved. Can we also get rid of
    * the front and back variants?
    */

   front_a = common_a;
   back_a = common_a;

   if (ds_state.stencil.test_enable) {
      uint32_t front_a_sref;
      uint32_t back_a_sref;

      pvr_csb_pack (&front_a_sref, TA_STATE_ISPA, ispa) {
         ispa.sref = ds_state.stencil.front.reference;
      }
      front_a |= front_a_sref;

      pvr_csb_pack (&back_a_sref, TA_STATE_ISPA, ispa) {
         ispa.sref = ds_state.stencil.back.reference;
      }
      back_a |= back_a_sref;

      pvr_csb_pack (&front_b, TA_STATE_ISPB, ispb) {
         const struct vk_stencil_test_face_state *const front =
            &ds_state.stencil.front;

         if (ds_state.stencil.write_enable)
            ispb.swmask = front->write_mask;

         ispb.scmpmask = front->compare_mask;

         ispb.sop3 = pvr_ta_stencilop(front->op.pass);
         ispb.sop2 = pvr_ta_stencilop(front->op.depth_fail);
         ispb.sop1 = pvr_ta_stencilop(front->op.fail);
         ispb.scmpmode = pvr_ta_cmpmode(front->op.compare);
      }

      pvr_csb_pack (&back_b, TA_STATE_ISPB, ispb) {
         const struct vk_stencil_test_face_state *const back =
            &ds_state.stencil.back;

         if (ds_state.stencil.write_enable)
            ispb.swmask = back->write_mask;

         ispb.scmpmask = back->compare_mask;

         ispb.sop3 = pvr_ta_stencilop(back->op.pass);
         ispb.sop2 = pvr_ta_stencilop(back->op.depth_fail);
         ispb.sop1 = pvr_ta_stencilop(back->op.fail);
         ispb.scmpmode = pvr_ta_cmpmode(back->op.compare);
      }
   } else {
      front_b = ispb_stencil_off;
      back_b = ispb_stencil_off;
   }

   if (front_a != back_a || front_b != back_b) {
      if (dynamic_state->rs.cull_mode & VK_CULL_MODE_BACK_BIT) {
         /* Single face, using front state. */
      } else if (dynamic_state->rs.cull_mode & VK_CULL_MODE_FRONT_BIT) {
         /* Single face, using back state. */

         front_a = back_a;
         front_b = back_b;
      } else {
         /* Both faces. */

         header->pres_ispctl_ba = is_two_sided = true;

         if (dynamic_state->rs.front_face == VK_FRONT_FACE_COUNTER_CLOCKWISE) {
            uint32_t tmp = front_a;

            front_a = back_a;
            back_a = tmp;

            tmp = front_b;
            front_b = back_b;
            back_b = tmp;
         }

         /* HW defaults to stencil off. */
         if (back_b != ispb_stencil_off) {
            header->pres_ispctl_fb = true;
            header->pres_ispctl_bb = true;
         }
      }
   }

   if (ds_state.stencil.test_enable && front_b != ispb_stencil_off)
      header->pres_ispctl_fb = true;

   pvr_csb_pack (&isp_control, TA_STATE_ISPCTL, ispctl) {
      ispctl.upass = pass_info->isp_userpass;

      /* TODO: is bo ever NULL? Figure out what to do. */
      ispctl.tagwritedisable = rasterizer_discard || !fragment_shader_state->bo;

      ispctl.two_sided = is_two_sided;
      ispctl.bpres = header->pres_ispctl_fb || header->pres_ispctl_bb;

      ispctl.dbenable = !rasterizer_discard &&
                        dynamic_state->rs.depth_bias.enable &&
                        obj_type == PVRX(TA_OBJTYPE_TRIANGLE);
      if (!rasterizer_discard && cmd_buffer->state.vis_test_enabled) {
         ispctl.vistest = true;
         ispctl.visreg = cmd_buffer->state.vis_reg;
      }

      ispctl.scenable = !rasterizer_discard;

      ppp_state->isp.control_struct = ispctl;
   }

   header->pres_ispctl = true;

   ppp_state->isp.control = isp_control;
   ppp_state->isp.front_a = front_a;
   ppp_state->isp.front_b = front_b;
   ppp_state->isp.back_a = back_a;
   ppp_state->isp.back_b = back_b;
}

static float
pvr_calculate_final_depth_bias_contant_factor(struct pvr_device_info *dev_info,
                                              VkFormat format,
                                              float depth_bias)
{
   /* Information for future modifiers of these depth bias calculations.
    * ==================================================================
    * Specified depth bias equations scale the specified constant factor by a
    * value 'r' that is guaranteed to cause a resolvable difference in depth
    * across the entire range of depth values.
    * For floating point depth formats 'r' is calculated by taking the maximum
    * exponent across the triangle.
    * For UNORM formats 'r' is constant.
    * Here 'n' is the number of mantissa bits stored in the floating point
    * representation (23 for F32).
    *
    *    UNORM Format -> z += dbcf * r + slope
    *    FLOAT Format -> z += dbcf * 2^(e-n) + slope
    *
    * HW Variations.
    * ==============
    * The HW either always performs the F32 depth bias equation (exponent based
    * r), or in the case of HW that correctly supports the integer depth bias
    * equation for UNORM depth formats, we can select between both equations
    * using the ROGUE_CR_ISP_CTL.dbias_is_int flag - this is required to
    * correctly perform Vulkan UNORM depth bias (constant r).
    *
    *    if ern42307:
    *       if DBIAS_IS_INT_EN:
    *          z += dbcf + slope
    *       else:
    *          z += dbcf * 2^(e-n) + slope
    *    else:
    *       z += dbcf * 2^(e-n) + slope
    *
    */

   float nudge_factor;

   if (PVR_HAS_ERN(dev_info, 42307)) {
      switch (format) {
      case VK_FORMAT_D16_UNORM:
         return depth_bias / (1 << 15);

      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_X8_D24_UNORM_PACK32:
         return depth_bias / (1 << 23);

      default:
         return depth_bias;
      }
   }

   /* The reasoning behind clamping/nudging the value here is because UNORM
    * depth formats can have higher precision over our underlying D32F
    * representation for some depth ranges.
    *
    * When the HW scales the depth bias value by 2^(e-n) [The 'r' term'] a depth
    * bias of 1 can result in a value smaller than one F32 ULP, which will get
    * quantized to 0 - resulting in no bias.
    *
    * Biasing small values away from zero will ensure that small depth biases of
    * 1 still yield a result and overcome Z-fighting.
    */
   switch (format) {
   case VK_FORMAT_D16_UNORM:
      depth_bias *= 512.0f;
      nudge_factor = 1.0f;
      break;

   case VK_FORMAT_D24_UNORM_S8_UINT:
   case VK_FORMAT_X8_D24_UNORM_PACK32:
      depth_bias *= 2.0f;
      nudge_factor = 2.0f;
      break;

   default:
      nudge_factor = 0.0f;
      break;
   }

   if (nudge_factor != 0.0f) {
      if (depth_bias < 0.0f && depth_bias > -nudge_factor)
         depth_bias -= nudge_factor;
      else if (depth_bias > 0.0f && depth_bias < nudge_factor)
         depth_bias += nudge_factor;
   }

   return depth_bias;
}

static void pvr_get_viewport_scissor_overlap(const VkViewport *const viewport,
                                             const VkRect2D *const scissor,
                                             VkRect2D *const rect_out)
{
   /* TODO: See if we can remove this struct. */
   struct pvr_rect {
      int32_t x0, y0;
      int32_t x1, y1;
   };

   /* TODO: Worry about overflow? */
   const struct pvr_rect scissor_rect = {
      .x0 = scissor->offset.x,
      .y0 = scissor->offset.y,
      .x1 = scissor->offset.x + scissor->extent.width,
      .y1 = scissor->offset.y + scissor->extent.height
   };
   struct pvr_rect viewport_rect = { 0 };

   assert(viewport->width >= 0.0f);
   assert(scissor_rect.x0 >= 0);
   assert(scissor_rect.y0 >= 0);

   if (scissor->extent.width == 0 || scissor->extent.height == 0) {
      *rect_out = (VkRect2D){ 0 };
      return;
   }

   viewport_rect.x0 = (int32_t)viewport->x;
   viewport_rect.x1 = (int32_t)viewport->x + (int32_t)viewport->width;

   /* TODO: Is there a mathematical way of doing all this and then clamp at
    * the end?
    */
   /* We flip the y0 and y1 when height is negative. */
   viewport_rect.y0 = (int32_t)viewport->y + MIN2(0, (int32_t)viewport->height);
   viewport_rect.y1 = (int32_t)viewport->y + MAX2(0, (int32_t)viewport->height);

   if (scissor_rect.x1 <= viewport_rect.x0 ||
       scissor_rect.y1 <= viewport_rect.y0 ||
       scissor_rect.x0 >= viewport_rect.x1 ||
       scissor_rect.y0 >= viewport_rect.y1) {
      *rect_out = (VkRect2D){ 0 };
      return;
   }

   /* Determine the overlapping rectangle. */
   viewport_rect.x0 = MAX2(viewport_rect.x0, scissor_rect.x0);
   viewport_rect.y0 = MAX2(viewport_rect.y0, scissor_rect.y0);
   viewport_rect.x1 = MIN2(viewport_rect.x1, scissor_rect.x1);
   viewport_rect.y1 = MIN2(viewport_rect.y1, scissor_rect.y1);

   /* TODO: Is this conversion safe? Is this logic right? */
   rect_out->offset.x = (uint32_t)viewport_rect.x0;
   rect_out->offset.y = (uint32_t)viewport_rect.y0;
   rect_out->extent.height = (uint32_t)(viewport_rect.y1 - viewport_rect.y0);
   rect_out->extent.width = (uint32_t)(viewport_rect.x1 - viewport_rect.x0);
}

static inline uint32_t
pvr_get_geom_region_clip_align_size(struct pvr_device_info *const dev_info)
{
   /* TODO: This should come from rogue_ppp.xml. */
   return 16U + 16U * (!PVR_HAS_FEATURE(dev_info, tile_size_16x16));
}

static void
pvr_setup_isp_depth_bias_scissor_state(struct pvr_cmd_buffer *const cmd_buffer)
{
   struct PVRX(TA_STATE_HEADER) *const header = &cmd_buffer->state.emit_header;
   struct pvr_ppp_state *const ppp_state = &cmd_buffer->state.ppp_state;
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   const struct PVRX(TA_STATE_ISPCTL) *const ispctl =
      &ppp_state->isp.control_struct;
   struct pvr_device_info *const dev_info =
      &cmd_buffer->device->pdevice->dev_info;

   if (ispctl->dbenable &&
       (BITSET_TEST(dynamic_state->dirty,
                    MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS) ||
        cmd_buffer->depth_bias_array.size == 0)) {
      struct pvr_depth_bias_state depth_bias = {
         .constant_factor = pvr_calculate_final_depth_bias_contant_factor(
            dev_info,
            cmd_buffer->state.depth_format,
            dynamic_state->rs.depth_bias.constant),
         .slope_factor = dynamic_state->rs.depth_bias.slope,
         .clamp = dynamic_state->rs.depth_bias.clamp,
      };

      ppp_state->depthbias_scissor_indices.depthbias_index =
         util_dynarray_num_elements(&cmd_buffer->depth_bias_array,
                                    __typeof__(depth_bias));

      util_dynarray_append(&cmd_buffer->depth_bias_array,
                           __typeof__(depth_bias),
                           depth_bias);

      header->pres_ispctl_dbsc = true;
   }

   if (ispctl->scenable) {
      const uint32_t region_clip_align_size =
         pvr_get_geom_region_clip_align_size(dev_info);
      const VkViewport *const viewport = &dynamic_state->vp.viewports[0];
      const VkRect2D *const scissor = &dynamic_state->vp.scissors[0];
      struct pvr_scissor_words scissor_words;
      VkRect2D overlap_rect;
      uint32_t height;
      uint32_t width;
      uint32_t x;
      uint32_t y;

      /* For region clip. */
      uint32_t bottom;
      uint32_t right;
      uint32_t left;
      uint32_t top;

      /* We don't support multiple viewport calculations. */
      assert(dynamic_state->vp.viewport_count == 1);
      /* We don't support multiple scissor calculations. */
      assert(dynamic_state->vp.scissor_count == 1);

      pvr_get_viewport_scissor_overlap(viewport, scissor, &overlap_rect);

      x = overlap_rect.offset.x;
      y = overlap_rect.offset.y;
      width = overlap_rect.extent.width;
      height = overlap_rect.extent.height;

      pvr_csb_pack (&scissor_words.w0, IPF_SCISSOR_WORD_0, word0) {
         word0.scw0_xmax = x + width;
         word0.scw0_xmin = x;
      }

      pvr_csb_pack (&scissor_words.w1, IPF_SCISSOR_WORD_1, word1) {
         word1.scw1_ymax = y + height;
         word1.scw1_ymin = y;
      }

      if (cmd_buffer->scissor_array.size &&
          cmd_buffer->scissor_words.w0 == scissor_words.w0 &&
          cmd_buffer->scissor_words.w1 == scissor_words.w1) {
         return;
      }

      cmd_buffer->scissor_words = scissor_words;

      /* Calculate region clip. */

      left = x / region_clip_align_size;
      top = y / region_clip_align_size;

      /* We prevent right=-1 with the multiplication. */
      /* TODO: Is there a better way of doing this? */
      if ((x + width) != 0U)
         right = DIV_ROUND_UP(x + width, region_clip_align_size) - 1;
      else
         right = 0;

      if ((y + height) != 0U)
         bottom = DIV_ROUND_UP(y + height, region_clip_align_size) - 1;
      else
         bottom = 0U;

      /* Setup region clip to clip everything outside what was calculated. */

      /* FIXME: Should we mask to prevent writing over other words? */
      pvr_csb_pack (&ppp_state->region_clipping.word0, TA_REGION_CLIP0, word0) {
         word0.right = right;
         word0.left = left;
         word0.mode = PVRX(TA_REGION_CLIP_MODE_OUTSIDE);
      }

      pvr_csb_pack (&ppp_state->region_clipping.word1, TA_REGION_CLIP1, word1) {
         word1.bottom = bottom;
         word1.top = top;
      }

      ppp_state->depthbias_scissor_indices.scissor_index =
         util_dynarray_num_elements(&cmd_buffer->scissor_array,
                                    struct pvr_scissor_words);

      util_dynarray_append(&cmd_buffer->scissor_array,
                           struct pvr_scissor_words,
                           cmd_buffer->scissor_words);

      header->pres_ispctl_dbsc = true;
      header->pres_region_clip = true;
   }
}

static void
pvr_setup_triangle_merging_flag(struct pvr_cmd_buffer *const cmd_buffer,
                                struct PVRX(TA_STATE_ISPA) * ispa)
{
   struct PVRX(TA_STATE_HEADER) *const header = &cmd_buffer->state.emit_header;
   struct pvr_ppp_state *const ppp_state = &cmd_buffer->state.ppp_state;
   uint32_t merge_word;
   uint32_t mask;

   pvr_csb_pack (&merge_word, TA_STATE_PDS_SIZEINFO2, size_info) {
      /* Disable for lines or punch-through or for DWD and depth compare
       * always.
       */
      if (ispa->objtype == PVRX(TA_OBJTYPE_LINE) ||
          ispa->passtype == PVRX(TA_PASSTYPE_PUNCH_THROUGH) ||
          (ispa->dwritedisable && ispa->dcmpmode == PVRX(TA_CMPMODE_ALWAYS))) {
         size_info.pds_tri_merge_disable = true;
      }
   }

   pvr_csb_pack (&mask, TA_STATE_PDS_SIZEINFO2, size_info) {
      size_info.pds_tri_merge_disable = true;
   }

   merge_word |= ppp_state->pds.size_info2 & ~mask;

   if (merge_word != ppp_state->pds.size_info2) {
      ppp_state->pds.size_info2 = merge_word;
      header->pres_pds_state_ptr0 = true;
   }
}

static void
pvr_setup_fragment_state_pointers(struct pvr_cmd_buffer *const cmd_buffer,
                                  struct pvr_sub_cmd_gfx *const sub_cmd)
{
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;

   const struct pvr_fragment_shader_state *const fragment =
      &state->gfx_pipeline->shader_state.fragment;
   const struct pvr_stage_allocation_descriptor_state *descriptor_shader_state =
      &fragment->descriptor_state;
   const struct pvr_pipeline_stage_state *fragment_state =
      &fragment->stage_state;
   const struct pvr_pds_upload *pds_coeff_program =
      &fragment->pds_coeff_program;

   const struct pvr_physical_device *pdevice = cmd_buffer->device->pdevice;
   struct PVRX(TA_STATE_HEADER) *const header = &state->emit_header;
   struct pvr_ppp_state *const ppp_state = &state->ppp_state;

   const uint32_t pds_uniform_size =
      DIV_ROUND_UP(descriptor_shader_state->pds_info.data_size_in_dwords,
                   PVRX(TA_STATE_PDS_SIZEINFO1_PDS_UNIFORMSIZE_UNIT_SIZE));

   const uint32_t pds_varying_state_size =
      DIV_ROUND_UP(pds_coeff_program->data_size,
                   PVRX(TA_STATE_PDS_SIZEINFO1_PDS_VARYINGSIZE_UNIT_SIZE));

   const uint32_t usc_varying_size =
      DIV_ROUND_UP(fragment_state->coefficient_size,
                   PVRX(TA_STATE_PDS_SIZEINFO1_USC_VARYINGSIZE_UNIT_SIZE));

   const uint32_t pds_temp_size =
      DIV_ROUND_UP(fragment_state->pds_temps_count,
                   PVRX(TA_STATE_PDS_SIZEINFO1_PDS_TEMPSIZE_UNIT_SIZE));

   const uint32_t usc_shared_size =
      DIV_ROUND_UP(fragment_state->const_shared_reg_count,
                   PVRX(TA_STATE_PDS_SIZEINFO2_USC_SHAREDSIZE_UNIT_SIZE));

   const uint32_t max_tiles_in_flight =
      pvr_calc_fscommon_size_and_tiles_in_flight(
         &pdevice->dev_info,
         &pdevice->dev_runtime_info,
         usc_shared_size *
            PVRX(TA_STATE_PDS_SIZEINFO2_USC_SHAREDSIZE_UNIT_SIZE),
         1);
   uint32_t size_info_mask;
   uint32_t size_info2;

   if (max_tiles_in_flight < sub_cmd->max_tiles_in_flight)
      sub_cmd->max_tiles_in_flight = max_tiles_in_flight;

   pvr_csb_pack (&ppp_state->pds.pixel_shader_base,
                 TA_STATE_PDS_SHADERBASE,
                 shader_base) {
      const struct pvr_pds_upload *const pds_upload =
         &fragment->pds_fragment_program;

      shader_base.addr = PVR_DEV_ADDR(pds_upload->data_offset);
   }

   if (descriptor_shader_state->pds_code.pvr_bo) {
      pvr_csb_pack (&ppp_state->pds.texture_uniform_code_base,
                    TA_STATE_PDS_TEXUNICODEBASE,
                    tex_base) {
         tex_base.addr =
            PVR_DEV_ADDR(descriptor_shader_state->pds_code.code_offset);
      }
   } else {
      ppp_state->pds.texture_uniform_code_base = 0U;
   }

   pvr_csb_pack (&ppp_state->pds.size_info1, TA_STATE_PDS_SIZEINFO1, info1) {
      info1.pds_uniformsize = pds_uniform_size;
      info1.pds_texturestatesize = 0U;
      info1.pds_varyingsize = pds_varying_state_size;
      info1.usc_varyingsize = usc_varying_size;
      info1.pds_tempsize = pds_temp_size;
   }

   pvr_csb_pack (&size_info_mask, TA_STATE_PDS_SIZEINFO2, mask) {
      mask.pds_tri_merge_disable = true;
   }

   ppp_state->pds.size_info2 &= size_info_mask;

   pvr_csb_pack (&size_info2, TA_STATE_PDS_SIZEINFO2, info2) {
      info2.usc_sharedsize = usc_shared_size;
   }

   ppp_state->pds.size_info2 |= size_info2;

   if (pds_coeff_program->pvr_bo) {
      header->pres_pds_state_ptr1 = true;

      pvr_csb_pack (&ppp_state->pds.varying_base,
                    TA_STATE_PDS_VARYINGBASE,
                    base) {
         base.addr = PVR_DEV_ADDR(pds_coeff_program->data_offset);
      }
   } else {
      ppp_state->pds.varying_base = 0U;
   }

   pvr_csb_pack (&ppp_state->pds.uniform_state_data_base,
                 TA_STATE_PDS_UNIFORMDATABASE,
                 base) {
      base.addr = PVR_DEV_ADDR(state->pds_fragment_descriptor_data_offset);
   }

   header->pres_pds_state_ptr0 = true;
   header->pres_pds_state_ptr3 = true;
}

static void pvr_setup_viewport(struct pvr_cmd_buffer *const cmd_buffer)
{
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   struct PVRX(TA_STATE_HEADER) *const header = &state->emit_header;
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct pvr_ppp_state *const ppp_state = &state->ppp_state;

   if (ppp_state->viewport_count != dynamic_state->vp.viewport_count) {
      ppp_state->viewport_count = dynamic_state->vp.viewport_count;
      header->pres_viewport = true;
   }

   if (dynamic_state->rs.rasterizer_discard_enable) {
      /* We don't want to emit any viewport data as it'll just get thrown
       * away. It's after the previous condition because we still want to
       * stash the viewport_count as it's our trigger for when
       * rasterizer discard gets disabled.
       */
      header->pres_viewport = false;
      return;
   }

   for (uint32_t i = 0; i < ppp_state->viewport_count; i++) {
      VkViewport *viewport = &dynamic_state->vp.viewports[i];
      uint32_t x_scale = fui(viewport->width * 0.5f);
      uint32_t y_scale = fui(viewport->height * 0.5f);
      uint32_t z_scale = fui(viewport->maxDepth - viewport->minDepth);
      uint32_t x_center = fui(viewport->x + viewport->width * 0.5f);
      uint32_t y_center = fui(viewport->y + viewport->height * 0.5f);
      uint32_t z_center = fui(viewport->minDepth);

      if (ppp_state->viewports[i].a0 != x_center ||
          ppp_state->viewports[i].m0 != x_scale ||
          ppp_state->viewports[i].a1 != y_center ||
          ppp_state->viewports[i].m1 != y_scale ||
          ppp_state->viewports[i].a2 != z_center ||
          ppp_state->viewports[i].m2 != z_scale) {
         ppp_state->viewports[i].a0 = x_center;
         ppp_state->viewports[i].m0 = x_scale;
         ppp_state->viewports[i].a1 = y_center;
         ppp_state->viewports[i].m1 = y_scale;
         ppp_state->viewports[i].a2 = z_center;
         ppp_state->viewports[i].m2 = z_scale;

         header->pres_viewport = true;
      }
   }
}

static void pvr_setup_ppp_control(struct pvr_cmd_buffer *const cmd_buffer)
{
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   const VkPrimitiveTopology topology = dynamic_state->ia.primitive_topology;
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   struct PVRX(TA_STATE_HEADER) *const header = &state->emit_header;
   struct pvr_ppp_state *const ppp_state = &state->ppp_state;
   uint32_t ppp_control;

   pvr_csb_pack (&ppp_control, TA_STATE_PPP_CTRL, control) {
      control.drawclippededges = true;
      control.wclampen = true;

      if (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
         control.flatshade_vtx = PVRX(TA_FLATSHADE_VTX_VERTEX_1);
      else
         control.flatshade_vtx = PVRX(TA_FLATSHADE_VTX_VERTEX_0);

      if (dynamic_state->rs.depth_clamp_enable)
         control.clip_mode = PVRX(TA_CLIP_MODE_NO_FRONT_OR_REAR);
      else
         control.clip_mode = PVRX(TA_CLIP_MODE_FRONT_REAR);

      /* +--- FrontIsCCW?
       * | +--- Cull Front?
       * v v
       * 0|0 CULLMODE_CULL_CCW,
       * 0|1 CULLMODE_CULL_CW,
       * 1|0 CULLMODE_CULL_CW,
       * 1|1 CULLMODE_CULL_CCW,
       */
      switch (dynamic_state->rs.cull_mode) {
      case VK_CULL_MODE_BACK_BIT:
      case VK_CULL_MODE_FRONT_BIT:
         if ((dynamic_state->rs.front_face == VK_FRONT_FACE_COUNTER_CLOCKWISE) ^
             (dynamic_state->rs.cull_mode == VK_CULL_MODE_FRONT_BIT)) {
            control.cullmode = PVRX(TA_CULLMODE_CULL_CW);
         } else {
            control.cullmode = PVRX(TA_CULLMODE_CULL_CCW);
         }

         break;

      case VK_CULL_MODE_FRONT_AND_BACK:
      case VK_CULL_MODE_NONE:
         control.cullmode = PVRX(TA_CULLMODE_NO_CULLING);
         break;

      default:
         unreachable("Unsupported cull mode!");
      }
   }

   if (ppp_control != ppp_state->ppp_control) {
      ppp_state->ppp_control = ppp_control;
      header->pres_ppp_ctrl = true;
   }
}

/* Largest valid PPP State update in words = 31
 * 1 - Header
 * 3 - Stream Out Config words 0, 1 and 2
 * 1 - PPP Control word
 * 3 - Varying Config words 0, 1 and 2
 * 1 - Output Select
 * 1 - WClamp
 * 6 - Viewport Transform words
 * 2 - Region Clip words
 * 3 - PDS State for fragment phase (PDSSTATEPTR 1-3)
 * 4 - PDS State for fragment phase (PDSSTATEPTR0)
 * 6 - ISP Control Words
 */
#define PVR_MAX_PPP_STATE_DWORDS 31

static VkResult pvr_emit_ppp_state(struct pvr_cmd_buffer *const cmd_buffer,
                                   struct pvr_sub_cmd_gfx *const sub_cmd)
{
   const bool deferred_secondary = pvr_cmd_uses_deferred_cs_cmds(cmd_buffer);
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   struct PVRX(TA_STATE_HEADER) *const header = &state->emit_header;
   struct pvr_csb *const control_stream = &sub_cmd->control_stream;
   struct pvr_ppp_state *const ppp_state = &state->ppp_state;
   uint32_t ppp_state_words[PVR_MAX_PPP_STATE_DWORDS];
   const bool emit_dbsc = header->pres_ispctl_dbsc;
   uint32_t *buffer_ptr = ppp_state_words;
   uint32_t dbsc_patching_offset = 0;
   uint32_t ppp_state_words_count;
   struct pvr_suballoc_bo *pvr_bo;
   VkResult result;

#if !defined(NDEBUG)
   struct PVRX(TA_STATE_HEADER) emit_mask = *header;
   uint32_t packed_emit_mask;

   static_assert(pvr_cmd_length(TA_STATE_HEADER) == 1,
                 "EMIT_MASK_IS_CLEAR assumes 1 dword sized header.");

#   define EMIT_MASK_GET(field) (emit_mask.field)
#   define EMIT_MASK_SET(field, value) (emit_mask.field = (value))
#   define EMIT_MASK_IS_CLEAR                                        \
      (pvr_cmd_pack(TA_STATE_HEADER)(&packed_emit_mask, &emit_mask), \
       packed_emit_mask == 0)
#else
#   define EMIT_MASK_GET(field)
#   define EMIT_MASK_SET(field, value)
#endif

   header->view_port_count =
      (ppp_state->viewport_count == 0) ? 0U : (ppp_state->viewport_count - 1);
   header->pres_ispctl_fa = header->pres_ispctl;

   /* If deferred_secondary is true then we do a separate state update
    * which gets patched in vkCmdExecuteCommands().
    */
   header->pres_ispctl_dbsc &= !deferred_secondary;

   pvr_csb_write_struct(buffer_ptr, TA_STATE_HEADER, header);

   static_assert(pvr_cmd_length(TA_STATE_HEADER) == 1,
                 "Following header check assumes 1 dword sized header.");
   /* If the header is empty we exit early and prevent a bo alloc of 0 size. */
   if (ppp_state_words[0] == 0)
      return VK_SUCCESS;

   if (header->pres_ispctl) {
      pvr_csb_write_value(buffer_ptr, TA_STATE_ISPCTL, ppp_state->isp.control);

      assert(header->pres_ispctl_fa);
      /* This is not a mistake. FA, BA have the ISPA format, and FB, BB have the
       * ISPB format.
       */
      pvr_csb_write_value(buffer_ptr, TA_STATE_ISPA, ppp_state->isp.front_a);
      EMIT_MASK_SET(pres_ispctl_fa, false);

      if (header->pres_ispctl_fb) {
         pvr_csb_write_value(buffer_ptr, TA_STATE_ISPB, ppp_state->isp.front_b);
         EMIT_MASK_SET(pres_ispctl_fb, false);
      }

      if (header->pres_ispctl_ba) {
         pvr_csb_write_value(buffer_ptr, TA_STATE_ISPA, ppp_state->isp.back_a);
         EMIT_MASK_SET(pres_ispctl_ba, false);
      }

      if (header->pres_ispctl_bb) {
         pvr_csb_write_value(buffer_ptr, TA_STATE_ISPB, ppp_state->isp.back_b);
         EMIT_MASK_SET(pres_ispctl_bb, false);
      }

      EMIT_MASK_SET(pres_ispctl, false);
   }

   if (header->pres_ispctl_dbsc) {
      assert(!deferred_secondary);

      dbsc_patching_offset = buffer_ptr - ppp_state_words;

      pvr_csb_pack (buffer_ptr, TA_STATE_ISPDBSC, ispdbsc) {
         ispdbsc.dbindex = ppp_state->depthbias_scissor_indices.depthbias_index;
         ispdbsc.scindex = ppp_state->depthbias_scissor_indices.scissor_index;
      }
      buffer_ptr += pvr_cmd_length(TA_STATE_ISPDBSC);

      EMIT_MASK_SET(pres_ispctl_dbsc, false);
   }

   if (header->pres_pds_state_ptr0) {
      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_PDS_SHADERBASE,
                          ppp_state->pds.pixel_shader_base);

      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_PDS_TEXUNICODEBASE,
                          ppp_state->pds.texture_uniform_code_base);

      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_PDS_SIZEINFO1,
                          ppp_state->pds.size_info1);
      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_PDS_SIZEINFO2,
                          ppp_state->pds.size_info2);

      EMIT_MASK_SET(pres_pds_state_ptr0, false);
   }

   if (header->pres_pds_state_ptr1) {
      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_PDS_VARYINGBASE,
                          ppp_state->pds.varying_base);
      EMIT_MASK_SET(pres_pds_state_ptr1, false);
   }

   /* We don't use pds_state_ptr2 (texture state programs) control word, but
    * this doesn't mean we need to set it to 0. This is because the hardware
    * runs the texture state program only when
    * ROGUE_TA_STATE_PDS_SIZEINFO1.pds_texturestatesize is non-zero.
    */
   assert(pvr_csb_unpack(&ppp_state->pds.size_info1, TA_STATE_PDS_SIZEINFO1)
             .pds_texturestatesize == 0);

   if (header->pres_pds_state_ptr3) {
      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_PDS_UNIFORMDATABASE,
                          ppp_state->pds.uniform_state_data_base);
      EMIT_MASK_SET(pres_pds_state_ptr3, false);
   }

   if (header->pres_region_clip) {
      pvr_csb_write_value(buffer_ptr,
                          TA_REGION_CLIP0,
                          ppp_state->region_clipping.word0);
      pvr_csb_write_value(buffer_ptr,
                          TA_REGION_CLIP1,
                          ppp_state->region_clipping.word1);

      EMIT_MASK_SET(pres_region_clip, false);
   }

   if (header->pres_viewport) {
      const uint32_t viewports = MAX2(1, ppp_state->viewport_count);
      EMIT_MASK_SET(view_port_count, viewports);

      for (uint32_t i = 0; i < viewports; i++) {
         /* These don't have any definitions in the csbgen xml files and none
          * will be added.
          */
         *buffer_ptr++ = ppp_state->viewports[i].a0;
         *buffer_ptr++ = ppp_state->viewports[i].m0;
         *buffer_ptr++ = ppp_state->viewports[i].a1;
         *buffer_ptr++ = ppp_state->viewports[i].m1;
         *buffer_ptr++ = ppp_state->viewports[i].a2;
         *buffer_ptr++ = ppp_state->viewports[i].m2;

         EMIT_MASK_SET(view_port_count, EMIT_MASK_GET(view_port_count) - 1);
      }

      EMIT_MASK_SET(pres_viewport, false);
   }

   if (header->pres_wclamp) {
      pvr_csb_pack (buffer_ptr, TA_WCLAMP, wclamp) {
         wclamp.val = fui(0.00001f);
      }
      buffer_ptr += pvr_cmd_length(TA_WCLAMP);
      EMIT_MASK_SET(pres_wclamp, false);
   }

   if (header->pres_outselects) {
      pvr_csb_write_value(buffer_ptr, TA_OUTPUT_SEL, ppp_state->output_selects);
      EMIT_MASK_SET(pres_outselects, false);
   }

   if (header->pres_varying_word0) {
      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_VARYING0,
                          ppp_state->varying_word[0]);
      EMIT_MASK_SET(pres_varying_word0, false);
   }

   if (header->pres_varying_word1) {
      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_VARYING1,
                          ppp_state->varying_word[1]);
      EMIT_MASK_SET(pres_varying_word1, false);
   }

   /* We only emit this on the first draw of a render job to prevent us from
    * inheriting a non-zero value set elsewhere.
    */
   if (header->pres_varying_word2) {
      pvr_csb_write_value(buffer_ptr, TA_STATE_VARYING2, 0);
      EMIT_MASK_SET(pres_varying_word2, false);
   }

   if (header->pres_ppp_ctrl) {
      pvr_csb_write_value(buffer_ptr,
                          TA_STATE_PPP_CTRL,
                          ppp_state->ppp_control);
      EMIT_MASK_SET(pres_ppp_ctrl, false);
   }

   /* We only emit this on the first draw of a render job to prevent us from
    * inheriting a non-zero value set elsewhere.
    */
   if (header->pres_stream_out_size) {
      pvr_csb_write_value(buffer_ptr, TA_STATE_STREAM_OUT0, 0);
      EMIT_MASK_SET(pres_stream_out_size, false);
   }

   assert(EMIT_MASK_IS_CLEAR);

#undef EMIT_MASK_GET
#undef EMIT_MASK_SET
#if !defined(NDEBUG)
#   undef EMIT_MASK_IS_CLEAR
#endif

   ppp_state_words_count = buffer_ptr - ppp_state_words;
   assert(ppp_state_words_count <= PVR_MAX_PPP_STATE_DWORDS);

   result = pvr_cmd_buffer_alloc_mem(cmd_buffer,
                                     cmd_buffer->device->heaps.general_heap,
                                     PVR_DW_TO_BYTES(ppp_state_words_count),
                                     &pvr_bo);
   if (result != VK_SUCCESS)
      return result;

   memcpy(pvr_bo_suballoc_get_map_addr(pvr_bo),
          ppp_state_words,
          PVR_DW_TO_BYTES(ppp_state_words_count));

   pvr_csb_set_relocation_mark(control_stream);

   /* Write the VDM state update into the VDM control stream. */
   pvr_csb_emit (control_stream, VDMCTRL_PPP_STATE0, state0) {
      state0.word_count = ppp_state_words_count;
      state0.addrmsb = pvr_bo->dev_addr;
   }

   pvr_csb_emit (control_stream, VDMCTRL_PPP_STATE1, state1) {
      state1.addrlsb = pvr_bo->dev_addr;
   }

   pvr_csb_clear_relocation_mark(control_stream);

   if (emit_dbsc && cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
      struct pvr_deferred_cs_command cmd;

      if (deferred_secondary) {
         const uint32_t num_dwords = pvr_cmd_length(VDMCTRL_PPP_STATE0) +
                                     pvr_cmd_length(VDMCTRL_PPP_STATE1);
         uint32_t *vdm_state;

         pvr_csb_set_relocation_mark(control_stream);

         vdm_state = pvr_csb_alloc_dwords(control_stream, num_dwords);
         if (!vdm_state) {
            result = pvr_csb_get_status(control_stream);
            return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
         }

         pvr_csb_clear_relocation_mark(control_stream);

         cmd = (struct pvr_deferred_cs_command){
            .type = PVR_DEFERRED_CS_COMMAND_TYPE_DBSC,
            .dbsc = {
               .state = ppp_state->depthbias_scissor_indices,
               .vdm_state = vdm_state,
            },
         };
      } else {
         cmd = (struct pvr_deferred_cs_command){
            .type = PVR_DEFERRED_CS_COMMAND_TYPE_DBSC2,
            .dbsc2 = {
               .state = ppp_state->depthbias_scissor_indices,
               .ppp_cs_bo = pvr_bo,
               .patch_offset = dbsc_patching_offset,
            },
         };
      }

      util_dynarray_append(&cmd_buffer->deferred_csb_commands,
                           struct pvr_deferred_cs_command,
                           cmd);
   }

   state->emit_header = (struct PVRX(TA_STATE_HEADER)){ 0 };

   return VK_SUCCESS;
}

static inline bool
pvr_ppp_state_update_required(const struct pvr_cmd_buffer *cmd_buffer)
{
   const BITSET_WORD *const dynamic_dirty =
      cmd_buffer->vk.dynamic_graphics_state.dirty;
   const struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   const struct PVRX(TA_STATE_HEADER) *const header = &state->emit_header;

   /* For push constants we only need to worry if they are updated for the
    * fragment stage since we're only updating the pds programs used in the
    * fragment stage.
    */

   return header->pres_ppp_ctrl || header->pres_ispctl ||
          header->pres_ispctl_fb || header->pres_ispctl_ba ||
          header->pres_ispctl_bb || header->pres_ispctl_dbsc ||
          header->pres_pds_state_ptr0 || header->pres_pds_state_ptr1 ||
          header->pres_pds_state_ptr2 || header->pres_pds_state_ptr3 ||
          header->pres_region_clip || header->pres_viewport ||
          header->pres_wclamp || header->pres_outselects ||
          header->pres_varying_word0 || header->pres_varying_word1 ||
          header->pres_varying_word2 || header->pres_stream_out_program ||
          state->dirty.fragment_descriptors || state->dirty.vis_test ||
          state->dirty.gfx_pipeline_binding || state->dirty.isp_userpass ||
          state->push_constants.dirty_stages & VK_SHADER_STAGE_FRAGMENT_BIT ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_DS_STENCIL_COMPARE_MASK) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_DS_STENCIL_REFERENCE) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_ENABLE) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_RS_LINE_WIDTH) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_VP_SCISSORS) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_VP_SCISSOR_COUNT) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_VP_VIEWPORTS) ||
          BITSET_TEST(dynamic_dirty, MESA_VK_DYNAMIC_VP_VIEWPORT_COUNT);
}

static VkResult
pvr_emit_dirty_ppp_state(struct pvr_cmd_buffer *const cmd_buffer,
                         struct pvr_sub_cmd_gfx *const sub_cmd)
{
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   VkResult result;

   /* TODO: The emit_header will be dirty only if
    * pvr_reset_graphics_dirty_state() was called before this (so when command
    * buffer begins recording or when it's reset). Otherwise it will have been
    * zeroed out by the previous pvr_emit_ppp_state(). We can probably set a
    * flag in there and check it here instead of checking the header.
    * Check if this is true and implement the flag.
    */
   if (!pvr_ppp_state_update_required(cmd_buffer))
      return VK_SUCCESS;

   if (state->dirty.gfx_pipeline_binding) {
      struct PVRX(TA_STATE_ISPA) ispa;

      pvr_setup_output_select(cmd_buffer);
      pvr_setup_isp_faces_and_control(cmd_buffer, &ispa);
      pvr_setup_triangle_merging_flag(cmd_buffer, &ispa);
   } else if (BITSET_TEST(dynamic_state->dirty,
                          MESA_VK_DYNAMIC_DS_STENCIL_COMPARE_MASK) ||
              BITSET_TEST(dynamic_state->dirty,
                          MESA_VK_DYNAMIC_DS_STENCIL_REFERENCE) ||
              BITSET_TEST(dynamic_state->dirty,
                          MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK) ||
              BITSET_TEST(dynamic_state->dirty,
                          MESA_VK_DYNAMIC_RS_LINE_WIDTH) ||
              state->dirty.isp_userpass || state->dirty.vis_test) {
      pvr_setup_isp_faces_and_control(cmd_buffer, NULL);
   }

   if (!dynamic_state->rs.rasterizer_discard_enable &&
       state->dirty.fragment_descriptors &&
       state->gfx_pipeline->shader_state.fragment.bo) {
      pvr_setup_fragment_state_pointers(cmd_buffer, sub_cmd);
   }

   pvr_setup_isp_depth_bias_scissor_state(cmd_buffer);

   if (BITSET_TEST(dynamic_state->dirty, MESA_VK_DYNAMIC_VP_VIEWPORTS) ||
       BITSET_TEST(dynamic_state->dirty, MESA_VK_DYNAMIC_VP_VIEWPORT_COUNT))
      pvr_setup_viewport(cmd_buffer);

   pvr_setup_ppp_control(cmd_buffer);

   /* The hardware doesn't have an explicit mode for this so we use a
    * negative viewport to make sure all objects are culled out early.
    */
   if (dynamic_state->rs.cull_mode == VK_CULL_MODE_FRONT_AND_BACK) {
      /* Shift the viewport out of the guard-band culling everything. */
      const uint32_t negative_vp_val = fui(-2.0f);

      state->ppp_state.viewports[0].a0 = negative_vp_val;
      state->ppp_state.viewports[0].m0 = 0;
      state->ppp_state.viewports[0].a1 = negative_vp_val;
      state->ppp_state.viewports[0].m1 = 0;
      state->ppp_state.viewports[0].a2 = negative_vp_val;
      state->ppp_state.viewports[0].m2 = 0;

      state->ppp_state.viewport_count = 1;

      state->emit_header.pres_viewport = true;
   }

   result = pvr_emit_ppp_state(cmd_buffer, sub_cmd);
   if (result != VK_SUCCESS)
      return result;

   return VK_SUCCESS;
}

void pvr_calculate_vertex_cam_size(const struct pvr_device_info *dev_info,
                                   const uint32_t vs_output_size,
                                   const bool raster_enable,
                                   uint32_t *const cam_size_out,
                                   uint32_t *const vs_max_instances_out)
{
   /* First work out the size of a vertex in the UVS and multiply by 4 for
    * column ordering.
    */
   const uint32_t uvs_vertex_vector_size_in_dwords =
      (vs_output_size + 1U + raster_enable * 4U) * 4U;
   const uint32_t vdm_cam_size =
      PVR_GET_FEATURE_VALUE(dev_info, vdm_cam_size, 32U);

   /* This is a proxy for 8XE. */
   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format) &&
       vdm_cam_size < 96U) {
      /* Comparisons are based on size including scratch per vertex vector. */
      if (uvs_vertex_vector_size_in_dwords < (14U * 4U)) {
         *cam_size_out = MIN2(31U, vdm_cam_size - 1U);
         *vs_max_instances_out = 16U;
      } else if (uvs_vertex_vector_size_in_dwords < (20U * 4U)) {
         *cam_size_out = 15U;
         *vs_max_instances_out = 16U;
      } else if (uvs_vertex_vector_size_in_dwords < (28U * 4U)) {
         *cam_size_out = 11U;
         *vs_max_instances_out = 12U;
      } else if (uvs_vertex_vector_size_in_dwords < (44U * 4U)) {
         *cam_size_out = 7U;
         *vs_max_instances_out = 8U;
      } else if (PVR_HAS_FEATURE(dev_info,
                                 simple_internal_parameter_format_v2) ||
                 uvs_vertex_vector_size_in_dwords < (64U * 4U)) {
         *cam_size_out = 7U;
         *vs_max_instances_out = 4U;
      } else {
         *cam_size_out = 3U;
         *vs_max_instances_out = 2U;
      }
   } else {
      /* Comparisons are based on size including scratch per vertex vector. */
      if (uvs_vertex_vector_size_in_dwords <= (32U * 4U)) {
         /* output size <= 27 + 5 scratch. */
         *cam_size_out = MIN2(95U, vdm_cam_size - 1U);
         *vs_max_instances_out = 0U;
      } else if (uvs_vertex_vector_size_in_dwords <= 48U * 4U) {
         /* output size <= 43 + 5 scratch */
         *cam_size_out = 63U;
         if (PVR_GET_FEATURE_VALUE(dev_info, uvs_vtx_entries, 144U) < 288U)
            *vs_max_instances_out = 16U;
         else
            *vs_max_instances_out = 0U;
      } else if (uvs_vertex_vector_size_in_dwords <= 64U * 4U) {
         /* output size <= 59 + 5 scratch. */
         *cam_size_out = 31U;
         if (PVR_GET_FEATURE_VALUE(dev_info, uvs_vtx_entries, 144U) < 288U)
            *vs_max_instances_out = 16U;
         else
            *vs_max_instances_out = 0U;
      } else {
         *cam_size_out = 15U;
         *vs_max_instances_out = 16U;
      }
   }
}

static void pvr_emit_dirty_vdm_state(struct pvr_cmd_buffer *const cmd_buffer,
                                     struct pvr_sub_cmd_gfx *const sub_cmd)
{
   /* FIXME: Assume all state is dirty for the moment. */
   struct pvr_device_info *const dev_info =
      &cmd_buffer->device->pdevice->dev_info;
   ASSERTED const uint32_t max_user_vertex_output_components =
      pvr_get_max_user_vertex_output_components(dev_info);
   struct PVRX(VDMCTRL_VDM_STATE0)
      header = { pvr_cmd_header(VDMCTRL_VDM_STATE0) };
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   const struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   const struct pvr_vertex_shader_state *const vertex_shader_state =
      &state->gfx_pipeline->shader_state.vertex;
   struct pvr_csb *const csb = &sub_cmd->control_stream;
   uint32_t vs_output_size;
   uint32_t max_instances;
   uint32_t cam_size;

   /* CAM Calculations and HW state take vertex size aligned to DWORDS. */
   vs_output_size =
      DIV_ROUND_UP(vertex_shader_state->vertex_output_size,
                   PVRX(VDMCTRL_VDM_STATE4_VS_OUTPUT_SIZE_UNIT_SIZE));

   assert(vs_output_size <= max_user_vertex_output_components);

   pvr_calculate_vertex_cam_size(dev_info,
                                 vs_output_size,
                                 true,
                                 &cam_size,
                                 &max_instances);

   pvr_csb_set_relocation_mark(csb);

   pvr_csb_emit (csb, VDMCTRL_VDM_STATE0, state0) {
      state0.cam_size = cam_size;

      if (dynamic_state->ia.primitive_restart_enable) {
         state0.cut_index_enable = true;
         state0.cut_index_present = true;
      }

      switch (dynamic_state->ia.primitive_topology) {
      case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
         state0.flatshade_control = PVRX(VDMCTRL_FLATSHADE_CONTROL_VERTEX_1);
         break;

      default:
         state0.flatshade_control = PVRX(VDMCTRL_FLATSHADE_CONTROL_VERTEX_0);
         break;
      }

      /* If we've bound a different vertex buffer, or this draw-call requires
       * a different PDS attrib data-section from the last draw call (changed
       * base_instance) then we need to specify a new data section. This is
       * also the case if we've switched pipeline or attrib program as the
       * data-section layout will be different.
       */
      state0.vs_data_addr_present =
         state->dirty.gfx_pipeline_binding || state->dirty.vertex_bindings ||
         state->dirty.draw_base_instance || state->dirty.draw_variant;

      /* Need to specify new PDS Attrib program if we've bound a different
       * pipeline or we needed a different PDS Attrib variant for this
       * draw-call.
       */
      state0.vs_other_present = state->dirty.gfx_pipeline_binding ||
                                state->dirty.draw_variant;

      /* UVB_SCRATCH_SELECT_ONE with no rasterization is only valid when
       * stream output is enabled. We use UVB_SCRATCH_SELECT_FIVE because
       * Vulkan doesn't support stream output and the vertex position is
       * always emitted to the UVB.
       */
      state0.uvs_scratch_size_select =
         PVRX(VDMCTRL_UVS_SCRATCH_SIZE_SELECT_FIVE);

      header = state0;
   }

   if (header.cut_index_present) {
      pvr_csb_emit (csb, VDMCTRL_VDM_STATE1, state1) {
         switch (state->index_buffer_binding.type) {
         case VK_INDEX_TYPE_UINT32:
            /* FIXME: Defines for these? These seem to come from the Vulkan
             * spec. for VkPipelineInputAssemblyStateCreateInfo
             * primitiveRestartEnable.
             */
            state1.cut_index = 0xFFFFFFFF;
            break;

         case VK_INDEX_TYPE_UINT16:
            state1.cut_index = 0xFFFF;
            break;

         default:
            unreachable("Invalid index type");
         }
      }
   }

   if (header.vs_data_addr_present) {
      pvr_csb_emit (csb, VDMCTRL_VDM_STATE2, state2) {
         state2.vs_pds_data_base_addr =
            PVR_DEV_ADDR(state->pds_vertex_attrib_offset);
      }
   }

   if (header.vs_other_present) {
      const uint32_t usc_unified_store_size_in_bytes =
         vertex_shader_state->vertex_input_size << 2;

      pvr_csb_emit (csb, VDMCTRL_VDM_STATE3, state3) {
         state3.vs_pds_code_base_addr =
            PVR_DEV_ADDR(state->pds_shader.code_offset);
      }

      pvr_csb_emit (csb, VDMCTRL_VDM_STATE4, state4) {
         state4.vs_output_size = vs_output_size;
      }

      pvr_csb_emit (csb, VDMCTRL_VDM_STATE5, state5) {
         state5.vs_max_instances = max_instances;
         state5.vs_usc_common_size = 0U;
         state5.vs_usc_unified_size = DIV_ROUND_UP(
            usc_unified_store_size_in_bytes,
            PVRX(VDMCTRL_VDM_STATE5_VS_USC_UNIFIED_SIZE_UNIT_SIZE));
         state5.vs_pds_temp_size =
            DIV_ROUND_UP(state->pds_shader.info->temps_required << 2,
                         PVRX(VDMCTRL_VDM_STATE5_VS_PDS_TEMP_SIZE_UNIT_SIZE));
         state5.vs_pds_data_size = DIV_ROUND_UP(
            PVR_DW_TO_BYTES(state->pds_shader.info->data_size_in_dwords),
            PVRX(VDMCTRL_VDM_STATE5_VS_PDS_DATA_SIZE_UNIT_SIZE));
      }
   }

   pvr_csb_clear_relocation_mark(csb);
}

static VkResult pvr_validate_draw_state(struct pvr_cmd_buffer *cmd_buffer)
{
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   const struct pvr_graphics_pipeline *const gfx_pipeline = state->gfx_pipeline;
   const struct pvr_pipeline_stage_state *const fragment_state =
      &gfx_pipeline->shader_state.fragment.stage_state;
   const struct pvr_pipeline_stage_state *const vertex_state =
      &gfx_pipeline->shader_state.vertex.stage_state;
   const struct pvr_pipeline_layout *const pipeline_layout =
      gfx_pipeline->base.layout;
   struct pvr_sub_cmd_gfx *sub_cmd;
   bool fstencil_writemask_zero;
   bool bstencil_writemask_zero;
   bool fstencil_keep;
   bool bstencil_keep;
   VkResult result;

   pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_GRAPHICS);

   sub_cmd = &state->current_sub_cmd->gfx;
   sub_cmd->empty_cmd = false;

   /* Determine pipeline depth/stencil usage. If a pipeline uses depth or
    * stencil testing, those attachments are using their loaded values, and
    * the loadOps cannot be optimized out.
    */
   /* Pipeline uses depth testing. */
   if (sub_cmd->depth_usage == PVR_DEPTH_STENCIL_USAGE_UNDEFINED &&
       dynamic_state->ds.depth.compare_op != VK_COMPARE_OP_ALWAYS) {
      sub_cmd->depth_usage = PVR_DEPTH_STENCIL_USAGE_NEEDED;
   }

   /* Pipeline uses stencil testing. */
   if (sub_cmd->stencil_usage == PVR_DEPTH_STENCIL_USAGE_UNDEFINED &&
       (dynamic_state->ds.stencil.front.op.compare != VK_COMPARE_OP_ALWAYS ||
        dynamic_state->ds.stencil.back.op.compare != VK_COMPARE_OP_ALWAYS)) {
      sub_cmd->stencil_usage = PVR_DEPTH_STENCIL_USAGE_NEEDED;
   }

   if (PVR_HAS_FEATURE(&cmd_buffer->device->pdevice->dev_info,
                       compute_overlap)) {
      uint32_t coefficient_size =
         DIV_ROUND_UP(fragment_state->coefficient_size,
                      PVRX(TA_STATE_PDS_SIZEINFO1_USC_VARYINGSIZE_UNIT_SIZE));

      if (coefficient_size >
          PVRX(TA_STATE_PDS_SIZEINFO1_USC_VARYINGSIZE_MAX_SIZE))
         sub_cmd->disable_compute_overlap = true;
   }

   sub_cmd->frag_uses_atomic_ops |= fragment_state->uses_atomic_ops;
   sub_cmd->frag_has_side_effects |= fragment_state->has_side_effects;
   sub_cmd->frag_uses_texture_rw |= fragment_state->uses_texture_rw;
   sub_cmd->vertex_uses_texture_rw |= vertex_state->uses_texture_rw;

   sub_cmd->job.get_vis_results = state->vis_test_enabled;

   fstencil_keep =
      (dynamic_state->ds.stencil.front.op.fail == VK_STENCIL_OP_KEEP) &&
      (dynamic_state->ds.stencil.front.op.pass == VK_STENCIL_OP_KEEP);
   bstencil_keep =
      (dynamic_state->ds.stencil.back.op.fail == VK_STENCIL_OP_KEEP) &&
      (dynamic_state->ds.stencil.back.op.pass == VK_STENCIL_OP_KEEP);
   fstencil_writemask_zero = (dynamic_state->ds.stencil.front.write_mask == 0);
   bstencil_writemask_zero = (dynamic_state->ds.stencil.back.write_mask == 0);

   /* Set stencil modified flag if:
    * - Neither front nor back-facing stencil has a fail_op/pass_op of KEEP.
    * - Neither front nor back-facing stencil has a write_mask of zero.
    */
   if (!(fstencil_keep && bstencil_keep) &&
       !(fstencil_writemask_zero && bstencil_writemask_zero)) {
      sub_cmd->modifies_stencil = true;
   }

   /* Set depth modified flag if depth write is enabled. */
   if (dynamic_state->ds.depth.write_enable)
      sub_cmd->modifies_depth = true;

   /* If either the data or code changes for pds vertex attribs, regenerate the
    * data segment.
    */
   if (state->dirty.vertex_bindings || state->dirty.gfx_pipeline_binding ||
       state->dirty.draw_variant || state->dirty.draw_base_instance) {
      enum pvr_pds_vertex_attrib_program_type prog_type;
      const struct pvr_pds_attrib_program *program;

      if (state->draw_state.draw_indirect)
         prog_type = PVR_PDS_VERTEX_ATTRIB_PROGRAM_DRAW_INDIRECT;
      else if (state->draw_state.base_instance)
         prog_type = PVR_PDS_VERTEX_ATTRIB_PROGRAM_BASE_INSTANCE;
      else
         prog_type = PVR_PDS_VERTEX_ATTRIB_PROGRAM_BASIC;

      program =
         &gfx_pipeline->shader_state.vertex.pds_attrib_programs[prog_type];
      state->pds_shader.info = &program->info;
      state->pds_shader.code_offset = program->program.code_offset;

      state->max_shared_regs =
         MAX2(state->max_shared_regs, pvr_calc_shared_regs_count(gfx_pipeline));

      pvr_setup_vertex_buffers(cmd_buffer, gfx_pipeline);
   }

   if (state->push_constants.dirty_stages & VK_SHADER_STAGE_ALL_GRAPHICS) {
      result = pvr_cmd_upload_push_consts(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;
   }

   state->dirty.vertex_descriptors = state->dirty.gfx_pipeline_binding;
   state->dirty.fragment_descriptors = state->dirty.vertex_descriptors;

   /* Account for dirty descriptor set. */
   state->dirty.vertex_descriptors |=
      state->dirty.gfx_desc_dirty &&
      pipeline_layout
         ->per_stage_descriptor_masks[PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY];
   state->dirty.fragment_descriptors |=
      state->dirty.gfx_desc_dirty &&
      pipeline_layout->per_stage_descriptor_masks[PVR_STAGE_ALLOCATION_FRAGMENT];

   if (BITSET_TEST(dynamic_state->dirty, MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS))
      state->dirty.fragment_descriptors = true;

   state->dirty.vertex_descriptors |=
      state->push_constants.dirty_stages &
      (VK_SHADER_STAGE_ALL_GRAPHICS & ~VK_SHADER_STAGE_FRAGMENT_BIT);
   state->dirty.fragment_descriptors |= state->push_constants.dirty_stages &
                                        VK_SHADER_STAGE_FRAGMENT_BIT;

   if (state->dirty.fragment_descriptors) {
      result = pvr_setup_descriptor_mappings(
         cmd_buffer,
         PVR_STAGE_ALLOCATION_FRAGMENT,
         &state->gfx_pipeline->shader_state.fragment.descriptor_state,
         NULL,
         &state->pds_fragment_descriptor_data_offset);
      if (result != VK_SUCCESS) {
         mesa_loge("Could not setup fragment descriptor mappings.");
         return result;
      }
   }

   if (state->dirty.vertex_descriptors) {
      uint32_t pds_vertex_descriptor_data_offset;

      result = pvr_setup_descriptor_mappings(
         cmd_buffer,
         PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY,
         &state->gfx_pipeline->shader_state.vertex.descriptor_state,
         NULL,
         &pds_vertex_descriptor_data_offset);
      if (result != VK_SUCCESS) {
         mesa_loge("Could not setup vertex descriptor mappings.");
         return result;
      }

      pvr_emit_dirty_pds_state(cmd_buffer,
                               sub_cmd,
                               pds_vertex_descriptor_data_offset);
   }

   pvr_emit_dirty_ppp_state(cmd_buffer, sub_cmd);
   pvr_emit_dirty_vdm_state(cmd_buffer, sub_cmd);

   vk_dynamic_graphics_state_clear_dirty(dynamic_state);
   state->dirty.gfx_desc_dirty = false;
   state->dirty.draw_base_instance = false;
   state->dirty.draw_variant = false;
   state->dirty.fragment_descriptors = false;
   state->dirty.gfx_pipeline_binding = false;
   state->dirty.isp_userpass = false;
   state->dirty.vertex_bindings = false;
   state->dirty.vis_test = false;

   state->push_constants.dirty_stages &= ~VK_SHADER_STAGE_ALL_GRAPHICS;

   return VK_SUCCESS;
}

static uint32_t pvr_get_hw_primitive_topology(VkPrimitiveTopology topology)
{
   switch (topology) {
   case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_POINT_LIST);
   case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_LINE_LIST);
   case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_LINE_STRIP);
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_TRI_LIST);
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_TRI_STRIP);
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_TRI_FAN);
   case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_LINE_LIST_ADJ);
   case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJ);
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_TRI_LIST_ADJ);
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_TRI_STRIP_ADJ);
   case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
      return PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_PATCH_LIST);
   default:
      unreachable("Undefined primitive topology");
   }
}

/* TODO: Rewrite this in terms of ALIGN_POT() and pvr_cmd_length(). */
/* Aligned to 128 bit for PDS loads / stores */
#define DUMMY_VDM_CONTROL_STREAM_BLOCK_SIZE 8

static VkResult
pvr_write_draw_indirect_vdm_stream(struct pvr_cmd_buffer *cmd_buffer,
                                   struct pvr_csb *const csb,
                                   pvr_dev_addr_t idx_buffer_addr,
                                   uint32_t idx_stride,
                                   struct PVRX(VDMCTRL_INDEX_LIST0) * list_hdr,
                                   struct pvr_buffer *buffer,
                                   VkDeviceSize offset,
                                   uint32_t count,
                                   uint32_t stride)
{
   struct pvr_pds_drawindirect_program pds_prog = { 0 };
   uint32_t word0;

   /* Draw indirect always has index offset and instance count. */
   list_hdr->index_offset_present = true;
   list_hdr->index_instance_count_present = true;

   pvr_cmd_pack(VDMCTRL_INDEX_LIST0)(&word0, list_hdr);

   pds_prog.support_base_instance = true;
   pds_prog.arg_buffer = buffer->dev_addr.addr + offset;
   pds_prog.index_buffer = idx_buffer_addr.addr;
   pds_prog.index_block_header = word0;
   pds_prog.index_stride = idx_stride;
   pds_prog.num_views = 1U;

   /* TODO: See if we can pre-upload the code section of all the pds programs
    * and reuse them here.
    */
   /* Generate and upload the PDS programs (code + data). */
   for (uint32_t i = 0U; i < count; i++) {
      const struct pvr_device_info *dev_info =
         &cmd_buffer->device->pdevice->dev_info;
      struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
      struct pvr_suballoc_bo *dummy_bo;
      struct pvr_suballoc_bo *pds_bo;
      uint32_t *dummy_stream;
      uint32_t *pds_base;
      uint32_t pds_size;
      VkResult result;

      /* TODO: Move this outside the loop and allocate all of them in one go? */
      result = pvr_cmd_buffer_alloc_mem(cmd_buffer,
                                        cmd_buffer->device->heaps.general_heap,
                                        DUMMY_VDM_CONTROL_STREAM_BLOCK_SIZE,
                                        &dummy_bo);
      if (result != VK_SUCCESS)
         return result;

      pds_prog.increment_draw_id = (i != 0);
      pds_prog.index_list_addr_buffer = dummy_bo->dev_addr.addr;

      if (state->draw_state.draw_indexed) {
         pvr_pds_generate_draw_elements_indirect(&pds_prog,
                                                 0,
                                                 PDS_GENERATE_SIZES,
                                                 dev_info);
      } else {
         pvr_pds_generate_draw_arrays_indirect(&pds_prog,
                                               0,
                                               PDS_GENERATE_SIZES,
                                               dev_info);
      }

      pds_size = PVR_DW_TO_BYTES(pds_prog.program.data_size_aligned +
                                 pds_prog.program.code_size_aligned);

      result = pvr_cmd_buffer_alloc_mem(cmd_buffer,
                                        cmd_buffer->device->heaps.pds_heap,
                                        pds_size,
                                        &pds_bo);
      if (result != VK_SUCCESS)
         return result;

      pds_base = pvr_bo_suballoc_get_map_addr(pds_bo);
      memcpy(pds_base,
             pds_prog.program.code,
             PVR_DW_TO_BYTES(pds_prog.program.code_size_aligned));

      if (state->draw_state.draw_indexed) {
         pvr_pds_generate_draw_elements_indirect(
            &pds_prog,
            pds_base + pds_prog.program.code_size_aligned,
            PDS_GENERATE_DATA_SEGMENT,
            dev_info);
      } else {
         pvr_pds_generate_draw_arrays_indirect(
            &pds_prog,
            pds_base + pds_prog.program.code_size_aligned,
            PDS_GENERATE_DATA_SEGMENT,
            dev_info);
      }

      pvr_csb_set_relocation_mark(csb);

      pvr_csb_emit (csb, VDMCTRL_PDS_STATE0, state0) {
         state0.usc_target = PVRX(VDMCTRL_USC_TARGET_ANY);

         state0.pds_temp_size =
            DIV_ROUND_UP(PVR_DW_TO_BYTES(pds_prog.program.temp_size_aligned),
                         PVRX(VDMCTRL_PDS_STATE0_PDS_TEMP_SIZE_UNIT_SIZE));

         state0.pds_data_size =
            DIV_ROUND_UP(PVR_DW_TO_BYTES(pds_prog.program.data_size_aligned),
                         PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE));
      }

      pvr_csb_emit (csb, VDMCTRL_PDS_STATE1, state1) {
         const uint32_t data_offset =
            pds_bo->dev_addr.addr +
            PVR_DW_TO_BYTES(pds_prog.program.code_size_aligned) -
            cmd_buffer->device->heaps.pds_heap->base_addr.addr;

         state1.pds_data_addr = PVR_DEV_ADDR(data_offset);
         state1.sd_type = PVRX(VDMCTRL_SD_TYPE_PDS);
         state1.sd_next_type = PVRX(VDMCTRL_SD_TYPE_NONE);
      }

      pvr_csb_emit (csb, VDMCTRL_PDS_STATE2, state2) {
         const uint32_t code_offset =
            pds_bo->dev_addr.addr -
            cmd_buffer->device->heaps.pds_heap->base_addr.addr;

         state2.pds_code_addr = PVR_DEV_ADDR(code_offset);
      }

      pvr_csb_clear_relocation_mark(csb);

      /* We don't really need to set the relocation mark since the following
       * state update is just one emit but let's be nice and use it.
       */
      pvr_csb_set_relocation_mark(csb);

      /* Sync task to ensure the VDM doesn't start reading the dummy blocks
       * before they are ready.
       */
      pvr_csb_emit (csb, VDMCTRL_INDEX_LIST0, list0) {
         list0.primitive_topology = PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_TRI_LIST);
      }

      pvr_csb_clear_relocation_mark(csb);

      dummy_stream = pvr_bo_suballoc_get_map_addr(dummy_bo);

      /* For indexed draw cmds fill in the dummy's header (as it won't change
       * based on the indirect args) and increment by the in-use size of each
       * dummy block.
       */
      if (!state->draw_state.draw_indexed) {
         dummy_stream[0] = word0;
         dummy_stream += 4;
      } else {
         dummy_stream += 5;
      }

      /* clang-format off */
      pvr_csb_pack (dummy_stream, VDMCTRL_STREAM_RETURN, word);
      /* clang-format on */

      pvr_csb_set_relocation_mark(csb);

      /* Stream link to the first dummy which forces the VDM to discard any
       * prefetched (dummy) control stream.
       */
      pvr_csb_emit (csb, VDMCTRL_STREAM_LINK0, link) {
         link.with_return = true;
         link.link_addrmsb = dummy_bo->dev_addr;
      }

      pvr_csb_emit (csb, VDMCTRL_STREAM_LINK1, link) {
         link.link_addrlsb = dummy_bo->dev_addr;
      }

      pvr_csb_clear_relocation_mark(csb);

      /* Point the pds program to the next argument buffer and the next VDM
       * dummy buffer.
       */
      pds_prog.arg_buffer += stride;
   }

   return VK_SUCCESS;
}

#undef DUMMY_VDM_CONTROL_STREAM_BLOCK_SIZE

static void pvr_emit_vdm_index_list(struct pvr_cmd_buffer *cmd_buffer,
                                    struct pvr_sub_cmd_gfx *const sub_cmd,
                                    VkPrimitiveTopology topology,
                                    uint32_t index_offset,
                                    uint32_t first_index,
                                    uint32_t index_count,
                                    uint32_t instance_count,
                                    struct pvr_buffer *buffer,
                                    VkDeviceSize offset,
                                    uint32_t count,
                                    uint32_t stride)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   const bool vertex_shader_has_side_effects =
      state->gfx_pipeline->shader_state.vertex.stage_state.has_side_effects;
   struct PVRX(VDMCTRL_INDEX_LIST0)
      list_hdr = { pvr_cmd_header(VDMCTRL_INDEX_LIST0) };
   pvr_dev_addr_t index_buffer_addr = PVR_DEV_ADDR_INVALID;
   struct pvr_csb *const csb = &sub_cmd->control_stream;
   unsigned int index_stride = 0;

   list_hdr.primitive_topology = pvr_get_hw_primitive_topology(topology);

   /* firstInstance is not handled here in the VDM state, it's implemented as
    * an addition in the PDS vertex fetch using
    * PVR_PDS_CONST_MAP_ENTRY_TYPE_BASE_INSTANCE entry type.
    */

   list_hdr.index_count_present = true;

   if (instance_count > 1)
      list_hdr.index_instance_count_present = true;

   if (index_offset)
      list_hdr.index_offset_present = true;

   if (state->draw_state.draw_indexed) {
      switch (state->index_buffer_binding.type) {
      case VK_INDEX_TYPE_UINT32:
         list_hdr.index_size = PVRX(VDMCTRL_INDEX_SIZE_B32);
         index_stride = 4;
         break;

      case VK_INDEX_TYPE_UINT16:
         list_hdr.index_size = PVRX(VDMCTRL_INDEX_SIZE_B16);
         index_stride = 2;
         break;

      default:
         unreachable("Invalid index type");
      }

      index_buffer_addr = PVR_DEV_ADDR_OFFSET(
         state->index_buffer_binding.buffer->dev_addr,
         state->index_buffer_binding.offset + first_index * index_stride);

      list_hdr.index_addr_present = true;
      list_hdr.index_base_addrmsb = index_buffer_addr;
   }

   list_hdr.degen_cull_enable =
      PVR_HAS_FEATURE(&cmd_buffer->device->pdevice->dev_info,
                      vdm_degenerate_culling) &&
      !vertex_shader_has_side_effects;

   if (state->draw_state.draw_indirect) {
      assert(buffer);
      pvr_write_draw_indirect_vdm_stream(cmd_buffer,
                                         csb,
                                         index_buffer_addr,
                                         index_stride,
                                         &list_hdr,
                                         buffer,
                                         offset,
                                         count,
                                         stride);
      return;
   }

   pvr_csb_set_relocation_mark(csb);

   pvr_csb_emit (csb, VDMCTRL_INDEX_LIST0, list0) {
      list0 = list_hdr;
   }

   if (list_hdr.index_addr_present) {
      pvr_csb_emit (csb, VDMCTRL_INDEX_LIST1, list1) {
         list1.index_base_addrlsb = index_buffer_addr;
      }
   }

   if (list_hdr.index_count_present) {
      pvr_csb_emit (csb, VDMCTRL_INDEX_LIST2, list2) {
         list2.index_count = index_count;
      }
   }

   if (list_hdr.index_instance_count_present) {
      pvr_csb_emit (csb, VDMCTRL_INDEX_LIST3, list3) {
         list3.instance_count = instance_count - 1;
      }
   }

   if (list_hdr.index_offset_present) {
      pvr_csb_emit (csb, VDMCTRL_INDEX_LIST4, list4) {
         list4.index_offset = index_offset;
      }
   }

   pvr_csb_clear_relocation_mark(csb);
}

void pvr_CmdDraw(VkCommandBuffer commandBuffer,
                 uint32_t vertexCount,
                 uint32_t instanceCount,
                 uint32_t firstVertex,
                 uint32_t firstInstance)
{
   const struct pvr_cmd_buffer_draw_state draw_state = {
      .base_vertex = firstVertex,
      .base_instance = firstInstance,
   };

   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   pvr_update_draw_state(state, &draw_state);

   result = pvr_validate_draw_state(cmd_buffer);
   if (result != VK_SUCCESS)
      return;

   /* Write the VDM control stream for the primitive. */
   pvr_emit_vdm_index_list(cmd_buffer,
                           &state->current_sub_cmd->gfx,
                           dynamic_state->ia.primitive_topology,
                           firstVertex,
                           0U,
                           vertexCount,
                           instanceCount,
                           NULL,
                           0U,
                           0U,
                           0U);
}

void pvr_CmdDrawIndexed(VkCommandBuffer commandBuffer,
                        uint32_t indexCount,
                        uint32_t instanceCount,
                        uint32_t firstIndex,
                        int32_t vertexOffset,
                        uint32_t firstInstance)
{
   const struct pvr_cmd_buffer_draw_state draw_state = {
      .base_vertex = vertexOffset,
      .base_instance = firstInstance,
      .draw_indexed = true,
   };

   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   pvr_update_draw_state(state, &draw_state);

   result = pvr_validate_draw_state(cmd_buffer);
   if (result != VK_SUCCESS)
      return;

   /* Write the VDM control stream for the primitive. */
   pvr_emit_vdm_index_list(cmd_buffer,
                           &state->current_sub_cmd->gfx,
                           dynamic_state->ia.primitive_topology,
                           vertexOffset,
                           firstIndex,
                           indexCount,
                           instanceCount,
                           NULL,
                           0U,
                           0U,
                           0U);
}

void pvr_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer,
                                VkBuffer _buffer,
                                VkDeviceSize offset,
                                uint32_t drawCount,
                                uint32_t stride)
{
   const struct pvr_cmd_buffer_draw_state draw_state = {
      .draw_indirect = true,
      .draw_indexed = true,
   };

   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   PVR_FROM_HANDLE(pvr_buffer, buffer, _buffer);
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   pvr_update_draw_state(state, &draw_state);

   result = pvr_validate_draw_state(cmd_buffer);
   if (result != VK_SUCCESS)
      return;

   /* Write the VDM control stream for the primitive. */
   pvr_emit_vdm_index_list(cmd_buffer,
                           &state->current_sub_cmd->gfx,
                           dynamic_state->ia.primitive_topology,
                           0U,
                           0U,
                           0U,
                           0U,
                           buffer,
                           offset,
                           drawCount,
                           stride);
}

void pvr_CmdDrawIndirect(VkCommandBuffer commandBuffer,
                         VkBuffer _buffer,
                         VkDeviceSize offset,
                         uint32_t drawCount,
                         uint32_t stride)
{
   const struct pvr_cmd_buffer_draw_state draw_state = {
      .draw_indirect = true,
   };

   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   PVR_FROM_HANDLE(pvr_buffer, buffer, _buffer);
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   pvr_update_draw_state(state, &draw_state);

   result = pvr_validate_draw_state(cmd_buffer);
   if (result != VK_SUCCESS)
      return;

   /* Write the VDM control stream for the primitive. */
   pvr_emit_vdm_index_list(cmd_buffer,
                           &state->current_sub_cmd->gfx,
                           dynamic_state->ia.primitive_topology,
                           0U,
                           0U,
                           0U,
                           0U,
                           buffer,
                           offset,
                           drawCount,
                           stride);
}

static VkResult
pvr_resolve_unemitted_resolve_attachments(struct pvr_cmd_buffer *cmd_buffer,
                                          struct pvr_render_pass_info *info)
{
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   const struct pvr_renderpass_hwsetup_render *hw_render =
      &state->render_pass_info.pass->hw_setup->renders[info->current_hw_subpass];

   for (uint32_t i = 0U; i < hw_render->eot_surface_count; i++) {
      const struct pvr_renderpass_hwsetup_eot_surface *surface =
         &hw_render->eot_surfaces[i];
      const uint32_t color_attach_idx = surface->src_attachment_idx;
      const uint32_t resolve_attach_idx = surface->attachment_idx;
      VkImageSubresourceLayers src_subresource;
      VkImageSubresourceLayers dst_subresource;
      struct pvr_image_view *dst_view;
      struct pvr_image_view *src_view;
      VkFormat src_format;
      VkFormat dst_format;
      VkImageCopy2 region;
      VkResult result;

      if (!surface->need_resolve ||
          surface->resolve_type != PVR_RESOLVE_TYPE_TRANSFER)
         continue;

      dst_view = info->attachments[resolve_attach_idx];
      src_view = info->attachments[color_attach_idx];

      src_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      src_subresource.mipLevel = src_view->vk.base_mip_level;
      src_subresource.baseArrayLayer = src_view->vk.base_array_layer;
      src_subresource.layerCount = src_view->vk.layer_count;

      dst_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      dst_subresource.mipLevel = dst_view->vk.base_mip_level;
      dst_subresource.baseArrayLayer = dst_view->vk.base_array_layer;
      dst_subresource.layerCount = dst_view->vk.layer_count;

      region.srcOffset = (VkOffset3D){ info->render_area.offset.x,
                                       info->render_area.offset.y,
                                       0 };
      region.dstOffset = (VkOffset3D){ info->render_area.offset.x,
                                       info->render_area.offset.y,
                                       0 };
      region.extent = (VkExtent3D){ info->render_area.extent.width,
                                    info->render_area.extent.height,
                                    1 };

      region.srcSubresource = src_subresource;
      region.dstSubresource = dst_subresource;

      /* TODO: if ERN_46863 is supported, Depth and stencil are sampled
       * separately from images with combined depth+stencil. Add logic here to
       * handle it using appropriate format from image view.
       */
      src_format = src_view->vk.image->format;
      dst_format = dst_view->vk.image->format;
      src_view->vk.image->format = src_view->vk.format;
      dst_view->vk.image->format = dst_view->vk.format;

      result = pvr_copy_or_resolve_color_image_region(
         cmd_buffer,
         vk_to_pvr_image(src_view->vk.image),
         vk_to_pvr_image(dst_view->vk.image),
         &region);

      src_view->vk.image->format = src_format;
      dst_view->vk.image->format = dst_format;

      state->current_sub_cmd->transfer.serialize_with_frag = true;

      if (result != VK_SUCCESS)
         return result;
   }

   return pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
}

void pvr_CmdEndRenderPass2(VkCommandBuffer commandBuffer,
                           const VkSubpassEndInfo *pSubpassEndInfo)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_image_view **attachments;
   VkClearValue *clear_values;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   assert(state->render_pass_info.pass);
   assert(state->render_pass_info.framebuffer);

   result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
   if (result != VK_SUCCESS)
      return;

   result = pvr_resolve_unemitted_resolve_attachments(cmd_buffer,
                                                      &state->render_pass_info);
   if (result != VK_SUCCESS)
      return;

   /* Save the required fields before clearing render_pass_info struct. */
   attachments = state->render_pass_info.attachments;
   clear_values = state->render_pass_info.clear_values;

   memset(&state->render_pass_info, 0, sizeof(state->render_pass_info));

   state->render_pass_info.attachments = attachments;
   state->render_pass_info.clear_values = clear_values;
}

static VkResult
pvr_execute_deferred_cmd_buffer(struct pvr_cmd_buffer *cmd_buffer,
                                const struct pvr_cmd_buffer *sec_cmd_buffer)
{
   struct vk_dynamic_graphics_state *const dynamic_state =
      &cmd_buffer->vk.dynamic_graphics_state;
   const uint32_t prim_db_elems =
      util_dynarray_num_elements(&cmd_buffer->depth_bias_array,
                                 struct pvr_depth_bias_state);
   const uint32_t prim_scissor_elems =
      util_dynarray_num_elements(&cmd_buffer->scissor_array,
                                 struct pvr_scissor_words);

   util_dynarray_foreach (&sec_cmd_buffer->deferred_csb_commands,
                          struct pvr_deferred_cs_command,
                          cmd) {
      switch (cmd->type) {
      case PVR_DEFERRED_CS_COMMAND_TYPE_DBSC: {
         const uint32_t scissor_idx =
            prim_scissor_elems + cmd->dbsc.state.scissor_index;
         const uint32_t db_idx =
            prim_db_elems + cmd->dbsc.state.depthbias_index;
         const uint32_t num_dwords =
            pvr_cmd_length(TA_STATE_HEADER) + pvr_cmd_length(TA_STATE_ISPDBSC);
         struct pvr_suballoc_bo *suballoc_bo;
         uint32_t ppp_state[num_dwords];
         VkResult result;

         pvr_csb_pack (&ppp_state[0], TA_STATE_HEADER, header) {
            header.pres_ispctl_dbsc = true;
         }

         pvr_csb_pack (&ppp_state[1], TA_STATE_ISPDBSC, ispdbsc) {
            ispdbsc.dbindex = db_idx;
            ispdbsc.scindex = scissor_idx;
         }

         result = pvr_cmd_buffer_upload_general(cmd_buffer,
                                                &ppp_state[0],
                                                sizeof(ppp_state),
                                                &suballoc_bo);
         if (result != VK_SUCCESS)
            return result;

         pvr_csb_pack (&cmd->dbsc.vdm_state[0], VDMCTRL_PPP_STATE0, state) {
            state.word_count = num_dwords;
            state.addrmsb = suballoc_bo->dev_addr;
         }

         pvr_csb_pack (&cmd->dbsc.vdm_state[1], VDMCTRL_PPP_STATE1, state) {
            state.addrlsb = suballoc_bo->dev_addr;
         }

         break;
      }

      case PVR_DEFERRED_CS_COMMAND_TYPE_DBSC2: {
         const uint32_t scissor_idx =
            prim_scissor_elems + cmd->dbsc2.state.scissor_index;
         const uint32_t db_idx =
            prim_db_elems + cmd->dbsc2.state.depthbias_index;

         uint32_t *const addr =
            (uint32_t *)pvr_bo_suballoc_get_map_addr(cmd->dbsc2.ppp_cs_bo) +
            cmd->dbsc2.patch_offset;

         assert(pvr_bo_suballoc_get_map_addr(cmd->dbsc2.ppp_cs_bo));

         pvr_csb_pack (addr, TA_STATE_ISPDBSC, ispdbsc) {
            ispdbsc.dbindex = db_idx;
            ispdbsc.scindex = scissor_idx;
         }

         break;
      }

      default:
         unreachable("Invalid deferred control stream command type.");
         break;
      }
   }

   util_dynarray_append_dynarray(&cmd_buffer->depth_bias_array,
                                 &sec_cmd_buffer->depth_bias_array);

   util_dynarray_append_dynarray(&cmd_buffer->scissor_array,
                                 &sec_cmd_buffer->scissor_array);

   BITSET_SET(dynamic_state->dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS);
   cmd_buffer->scissor_words = (struct pvr_scissor_words){ 0 };

   return VK_SUCCESS;
}

/* Caller needs to make sure that it ends the current sub_cmd. This function
 * only creates a copy of sec_sub_cmd and links it to the cmd_buffer's
 * sub_cmd list.
 */
static VkResult pvr_execute_sub_cmd(struct pvr_cmd_buffer *cmd_buffer,
                                    struct pvr_sub_cmd *sec_sub_cmd)
{
   struct pvr_sub_cmd *primary_sub_cmd =
      vk_zalloc(&cmd_buffer->vk.pool->alloc,
                sizeof(*primary_sub_cmd),
                8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!primary_sub_cmd) {
      return vk_command_buffer_set_error(&cmd_buffer->vk,
                                         VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   primary_sub_cmd->type = sec_sub_cmd->type;
   primary_sub_cmd->owned = false;

   list_addtail(&primary_sub_cmd->link, &cmd_buffer->sub_cmds);

   switch (sec_sub_cmd->type) {
   case PVR_SUB_CMD_TYPE_GRAPHICS:
      primary_sub_cmd->gfx = sec_sub_cmd->gfx;
      break;

   case PVR_SUB_CMD_TYPE_OCCLUSION_QUERY:
   case PVR_SUB_CMD_TYPE_COMPUTE:
      primary_sub_cmd->compute = sec_sub_cmd->compute;
      break;

   case PVR_SUB_CMD_TYPE_TRANSFER:
      primary_sub_cmd->transfer = sec_sub_cmd->transfer;
      break;

   case PVR_SUB_CMD_TYPE_EVENT:
      primary_sub_cmd->event = sec_sub_cmd->event;
      break;

   default:
      unreachable("Unsupported sub-command type");
   }

   return VK_SUCCESS;
}

static VkResult
pvr_execute_graphics_cmd_buffer(struct pvr_cmd_buffer *cmd_buffer,
                                const struct pvr_cmd_buffer *sec_cmd_buffer)
{
   const struct pvr_device_info *dev_info =
      &cmd_buffer->device->pdevice->dev_info;
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_sub_cmd *primary_sub_cmd = state->current_sub_cmd;
   struct pvr_sub_cmd *first_sec_cmd;
   VkResult result;

   /* Inherited queries are not supported. */
   assert(!state->vis_test_enabled);

   if (list_is_empty(&sec_cmd_buffer->sub_cmds))
      return VK_SUCCESS;

   first_sec_cmd =
      list_first_entry(&sec_cmd_buffer->sub_cmds, struct pvr_sub_cmd, link);

   /* Kick a render if we have a new base address. */
   if (primary_sub_cmd->gfx.query_pool && first_sec_cmd->gfx.query_pool &&
       primary_sub_cmd->gfx.query_pool != first_sec_cmd->gfx.query_pool) {
      state->current_sub_cmd->gfx.barrier_store = true;

      result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
      if (result != VK_SUCCESS)
         return result;

      result =
         pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_GRAPHICS);
      if (result != VK_SUCCESS)
         return result;

      primary_sub_cmd = state->current_sub_cmd;

      /* Use existing render setup, but load color attachments from HW
       * Background object.
       */
      primary_sub_cmd->gfx.barrier_load = true;
      primary_sub_cmd->gfx.barrier_store = false;
   }

   list_for_each_entry (struct pvr_sub_cmd,
                        sec_sub_cmd,
                        &sec_cmd_buffer->sub_cmds,
                        link) {
      /* Only graphics secondary execution supported within a renderpass. */
      assert(sec_sub_cmd->type == PVR_SUB_CMD_TYPE_GRAPHICS);

      if (!sec_sub_cmd->gfx.empty_cmd)
         primary_sub_cmd->gfx.empty_cmd = false;

      if (sec_sub_cmd->gfx.query_pool) {
         primary_sub_cmd->gfx.query_pool = sec_sub_cmd->gfx.query_pool;

         util_dynarray_append_dynarray(&state->query_indices,
                                       &sec_sub_cmd->gfx.sec_query_indices);
      }

      if (pvr_cmd_uses_deferred_cs_cmds(sec_cmd_buffer)) {
         /* TODO: In case if secondary buffer is created with
          * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, then we patch the
          * stream and copy it to primary stream using pvr_csb_copy below.
          * This will need locking if the same secondary command buffer is
          * executed in multiple primary buffers at the same time.
          */
         result = pvr_execute_deferred_cmd_buffer(cmd_buffer, sec_cmd_buffer);
         if (result != VK_SUCCESS)
            return result;

         result = pvr_csb_copy(&primary_sub_cmd->gfx.control_stream,
                               &sec_sub_cmd->gfx.control_stream);
         if (result != VK_SUCCESS)
            return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
      } else {
         result = pvr_execute_deferred_cmd_buffer(cmd_buffer, sec_cmd_buffer);
         if (result != VK_SUCCESS)
            return result;

         pvr_csb_emit_link(
            &primary_sub_cmd->gfx.control_stream,
            pvr_csb_get_start_address(&sec_sub_cmd->gfx.control_stream),
            true);
      }

      if (PVR_HAS_FEATURE(&cmd_buffer->device->pdevice->dev_info,
                          compute_overlap)) {
         primary_sub_cmd->gfx.job.disable_compute_overlap |=
            sec_sub_cmd->gfx.job.disable_compute_overlap;
      }

      primary_sub_cmd->gfx.max_tiles_in_flight =
         MIN2(primary_sub_cmd->gfx.max_tiles_in_flight,
              sec_sub_cmd->gfx.max_tiles_in_flight);

      /* Pass loaded depth/stencil usage from secondary command buffer. */
      if (sec_sub_cmd->gfx.depth_usage == PVR_DEPTH_STENCIL_USAGE_NEEDED)
         primary_sub_cmd->gfx.depth_usage = PVR_DEPTH_STENCIL_USAGE_NEEDED;

      if (sec_sub_cmd->gfx.stencil_usage == PVR_DEPTH_STENCIL_USAGE_NEEDED)
         primary_sub_cmd->gfx.stencil_usage = PVR_DEPTH_STENCIL_USAGE_NEEDED;

      /* Pass depth/stencil modification state from secondary command buffer. */
      if (sec_sub_cmd->gfx.modifies_depth)
         primary_sub_cmd->gfx.modifies_depth = true;

      if (sec_sub_cmd->gfx.modifies_stencil)
         primary_sub_cmd->gfx.modifies_stencil = true;

      if (sec_sub_cmd->gfx.barrier_store) {
         struct pvr_sub_cmd *sec_next =
            list_entry(sec_sub_cmd->link.next, struct pvr_sub_cmd, link);

         /* This shouldn't be the last sub cmd. There should be a barrier load
          * subsequent to the barrier store.
          */
         assert(list_last_entry(&sec_cmd_buffer->sub_cmds,
                                struct pvr_sub_cmd,
                                link) != sec_sub_cmd);

         /* Kick render to store stencil. */
         state->current_sub_cmd->gfx.barrier_store = true;
         state->current_sub_cmd->gfx.empty_cmd = false;

         result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
         if (result != VK_SUCCESS)
            return result;

         result =
            pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_GRAPHICS);
         if (result != VK_SUCCESS)
            return result;

         primary_sub_cmd = state->current_sub_cmd;

         /* Use existing render setup, but load color attachments from HW
          * Background object.
          */
         primary_sub_cmd->gfx.barrier_load = sec_next->gfx.barrier_load;
         primary_sub_cmd->gfx.barrier_store = sec_next->gfx.barrier_store;
         primary_sub_cmd->gfx.empty_cmd = false;
      }

      if (!PVR_HAS_FEATURE(dev_info, gs_rta_support)) {
         util_dynarray_append_dynarray(&cmd_buffer->deferred_clears,
                                       &sec_cmd_buffer->deferred_clears);
      }
   }

   return VK_SUCCESS;
}

void pvr_CmdExecuteCommands(VkCommandBuffer commandBuffer,
                            uint32_t commandBufferCount,
                            const VkCommandBuffer *pCommandBuffers)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_cmd_buffer *last_cmd_buffer;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   /* Reset the CPU copy of the most recent PPP state of the primary command
    * buffer.
    *
    * The next draw call in the primary after CmdExecuteCommands may send
    * redundant state, if it all goes in the same geom job.
    *
    * Can't just copy state from the secondary because the recording state of
    * the secondary command buffers would have been deleted at this point.
    */
   pvr_reset_graphics_dirty_state(cmd_buffer, false);

   if (state->current_sub_cmd &&
       state->current_sub_cmd->type == PVR_SUB_CMD_TYPE_GRAPHICS) {
      for (uint32_t i = 0; i < commandBufferCount; i++) {
         PVR_FROM_HANDLE(pvr_cmd_buffer, sec_cmd_buffer, pCommandBuffers[i]);

         assert(sec_cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);

         result = pvr_execute_graphics_cmd_buffer(cmd_buffer, sec_cmd_buffer);
         if (result != VK_SUCCESS)
            return;
      }

      last_cmd_buffer =
         pvr_cmd_buffer_from_handle(pCommandBuffers[commandBufferCount - 1]);

      /* Set barriers from final command secondary command buffer. */
      for (uint32_t i = 0; i != PVR_NUM_SYNC_PIPELINE_STAGES; i++) {
         state->barriers_needed[i] |=
            last_cmd_buffer->state.barriers_needed[i] &
            PVR_PIPELINE_STAGE_ALL_GRAPHICS_BITS;
      }
   } else {
      for (uint32_t i = 0; i < commandBufferCount; i++) {
         PVR_FROM_HANDLE(pvr_cmd_buffer, sec_cmd_buffer, pCommandBuffers[i]);

         assert(sec_cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);

         result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
         if (result != VK_SUCCESS)
            return;

         list_for_each_entry_safe (struct pvr_sub_cmd,
                                   sec_sub_cmd,
                                   &sec_cmd_buffer->sub_cmds,
                                   link) {
            result = pvr_execute_sub_cmd(cmd_buffer, sec_sub_cmd);
            if (result != VK_SUCCESS)
               return;
         }
      }

      last_cmd_buffer =
         pvr_cmd_buffer_from_handle(pCommandBuffers[commandBufferCount - 1]);

      memcpy(state->barriers_needed,
             last_cmd_buffer->state.barriers_needed,
             sizeof(state->barriers_needed));
   }
}

static void pvr_insert_transparent_obj(struct pvr_cmd_buffer *const cmd_buffer,
                                       struct pvr_sub_cmd_gfx *const sub_cmd)
{
   struct pvr_device *const device = cmd_buffer->device;
   /* Yes we want a copy. The user could be recording multiple command buffers
    * in parallel so writing the template in place could cause problems.
    */
   struct pvr_static_clear_ppp_template clear =
      device->static_clear_state.ppp_templates[VK_IMAGE_ASPECT_COLOR_BIT];
   uint32_t pds_state[PVR_STATIC_CLEAR_PDS_STATE_COUNT] = { 0 };
   struct pvr_csb *csb = &sub_cmd->control_stream;
   struct pvr_suballoc_bo *ppp_bo;

   assert(clear.requires_pds_state);

   /* Patch the template. */

   pvr_csb_pack (&pds_state[PVR_STATIC_CLEAR_PPP_PDS_TYPE_SHADERBASE],
                 TA_STATE_PDS_SHADERBASE,
                 shaderbase) {
      shaderbase.addr = PVR_DEV_ADDR(device->nop_program.pds.data_offset);
   }

   clear.config.pds_state = &pds_state;

   clear.config.ispctl.upass = cmd_buffer->state.render_pass_info.isp_userpass;

   /* Emit PPP state from template. */

   pvr_emit_ppp_from_template(csb, &clear, &ppp_bo);
   list_add(&ppp_bo->link, &cmd_buffer->bo_list);

   /* Emit VDM state. */

   pvr_emit_clear_words(cmd_buffer, sub_cmd);

   /* Reset graphics state. */
   pvr_reset_graphics_dirty_state(cmd_buffer, false);
}

static inline struct pvr_render_subpass *
pvr_get_current_subpass(const struct pvr_cmd_buffer_state *const state)
{
   const uint32_t subpass_idx = state->render_pass_info.subpass_idx;

   return &state->render_pass_info.pass->subpasses[subpass_idx];
}

void pvr_CmdNextSubpass2(VkCommandBuffer commandBuffer,
                         const VkSubpassBeginInfo *pSubpassBeginInfo,
                         const VkSubpassEndInfo *pSubpassEndInfo)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   struct pvr_render_pass_info *rp_info = &state->render_pass_info;
   const struct pvr_renderpass_hwsetup_subpass *hw_subpass;
   struct pvr_renderpass_hwsetup_render *next_hw_render;
   const struct pvr_render_pass *pass = rp_info->pass;
   const struct pvr_renderpass_hw_map *current_map;
   const struct pvr_renderpass_hw_map *next_map;
   struct pvr_load_op *hw_subpass_load_op;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   current_map = &pass->hw_setup->subpass_map[rp_info->subpass_idx];
   next_map = &pass->hw_setup->subpass_map[rp_info->subpass_idx + 1];
   next_hw_render = &pass->hw_setup->renders[next_map->render];

   if (current_map->render != next_map->render) {
      result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
      if (result != VK_SUCCESS)
         return;

      result = pvr_resolve_unemitted_resolve_attachments(cmd_buffer, rp_info);
      if (result != VK_SUCCESS)
         return;

      rp_info->current_hw_subpass = next_map->render;

      result =
         pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_GRAPHICS);
      if (result != VK_SUCCESS)
         return;

      rp_info->enable_bg_tag = false;
      rp_info->process_empty_tiles = false;

      /* If this subpass contains any load ops the HW Background Object must be
       * run to do the clears/loads.
       */
      if (next_hw_render->color_init_count > 0) {
         rp_info->enable_bg_tag = true;

         for (uint32_t i = 0; i < next_hw_render->color_init_count; i++) {
            /* Empty tiles need to be cleared too. */
            if (next_hw_render->color_init[i].op ==
                VK_ATTACHMENT_LOAD_OP_CLEAR) {
               rp_info->process_empty_tiles = true;
               break;
            }
         }
      }

      /* Set isp_userpass to zero for new hw_render. This will be used to set
       * ROGUE_CR_ISP_CTL::upass_start.
       */
      rp_info->isp_userpass = 0;
   }

   hw_subpass = &next_hw_render->subpasses[next_map->subpass];
   hw_subpass_load_op = hw_subpass->load_op;

   if (hw_subpass_load_op) {
      result = pvr_cs_write_load_op(cmd_buffer,
                                    &state->current_sub_cmd->gfx,
                                    hw_subpass_load_op,
                                    rp_info->isp_userpass);
   }

   /* Pipelines are created for a particular subpass so unbind but leave the
    * vertex and descriptor bindings intact as they are orthogonal to the
    * subpass.
    */
   state->gfx_pipeline = NULL;

   /* User-pass spawn is 4 bits so if the driver has to wrap it, it will emit a
    * full screen transparent object to flush all tags up until now, then the
    * user-pass spawn value will implicitly be reset to 0 because
    * pvr_render_subpass::isp_userpass values are stored ANDed with
    * ROGUE_CR_ISP_CTL_UPASS_START_SIZE_MAX.
    */
   /* If hw_subpass_load_op is valid then pvr_write_load_op_control_stream
    * has already done a full-screen transparent object.
    */
   if (rp_info->isp_userpass == PVRX(CR_ISP_CTL_UPASS_START_SIZE_MAX) &&
       !hw_subpass_load_op) {
      pvr_insert_transparent_obj(cmd_buffer, &state->current_sub_cmd->gfx);
   }

   rp_info->subpass_idx++;

   rp_info->isp_userpass = pass->subpasses[rp_info->subpass_idx].isp_userpass;
   state->dirty.isp_userpass = true;

   rp_info->pipeline_bind_point =
      pass->subpasses[rp_info->subpass_idx].pipeline_bind_point;

   pvr_stash_depth_format(state, &state->current_sub_cmd->gfx);
}

static bool
pvr_stencil_has_self_dependency(const struct pvr_cmd_buffer_state *const state)
{
   const struct pvr_render_subpass *const current_subpass =
      pvr_get_current_subpass(state);
   const uint32_t *const input_attachments = current_subpass->input_attachments;

   if (current_subpass->depth_stencil_attachment == VK_ATTACHMENT_UNUSED)
      return false;

   /* We only need to check the current software subpass as we don't support
    * merging to/from a subpass with self-dep stencil.
    */

   for (uint32_t i = 0; i < current_subpass->input_count; i++) {
      if (input_attachments[i] == current_subpass->depth_stencil_attachment)
         return true;
   }

   return false;
}

static bool pvr_is_stencil_store_load_needed(
   const struct pvr_cmd_buffer *const cmd_buffer,
   VkPipelineStageFlags2 vk_src_stage_mask,
   VkPipelineStageFlags2 vk_dst_stage_mask,
   uint32_t memory_barrier_count,
   const VkMemoryBarrier2 *const memory_barriers,
   uint32_t image_barrier_count,
   const VkImageMemoryBarrier2 *const image_barriers)
{
   const struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   const uint32_t fragment_test_stages =
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
   const struct pvr_render_pass *const pass = state->render_pass_info.pass;
   const struct pvr_renderpass_hwsetup_render *hw_render;
   struct pvr_image_view **const attachments =
      state->render_pass_info.attachments;
   const struct pvr_image_view *attachment;
   uint32_t hw_render_idx;

   if (!pass)
      return false;

   hw_render_idx = state->current_sub_cmd->gfx.hw_render_idx;
   hw_render = &pass->hw_setup->renders[hw_render_idx];

   if (hw_render->ds_attach_idx == VK_ATTACHMENT_UNUSED)
      return false;

   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
      attachment = attachments[hw_render->ds_attach_idx];
   } else {
      assert(!attachments);
      attachment = NULL;
   }

   if (!(vk_src_stage_mask & fragment_test_stages) &&
       vk_dst_stage_mask & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
      return false;

   for (uint32_t i = 0; i < memory_barrier_count; i++) {
      const uint32_t stencil_write_bit =
         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      const uint32_t input_attachment_read_bit =
         VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

      if (!(memory_barriers[i].srcAccessMask & stencil_write_bit))
         continue;

      if (!(memory_barriers[i].dstAccessMask & input_attachment_read_bit))
         continue;

      return pvr_stencil_has_self_dependency(state);
   }

   for (uint32_t i = 0; i < image_barrier_count; i++) {
      PVR_FROM_HANDLE(pvr_image, image, image_barriers[i].image);
      const uint32_t stencil_bit = VK_IMAGE_ASPECT_STENCIL_BIT;

      if (!(image_barriers[i].subresourceRange.aspectMask & stencil_bit))
         continue;

      if (attachment && image != vk_to_pvr_image(attachment->vk.image))
         continue;

      if (!vk_format_has_stencil(image->vk.format))
         continue;

      return pvr_stencil_has_self_dependency(state);
   }

   return false;
}

static VkResult
pvr_cmd_buffer_insert_mid_frag_barrier_event(struct pvr_cmd_buffer *cmd_buffer,
                                             uint32_t src_stage_mask,
                                             uint32_t dst_stage_mask)
{
   VkResult result;

   assert(cmd_buffer->state.current_sub_cmd->type == PVR_SUB_CMD_TYPE_GRAPHICS);

   cmd_buffer->state.current_sub_cmd->gfx.empty_cmd = false;

   /* Submit graphics job to store stencil. */
   cmd_buffer->state.current_sub_cmd->gfx.barrier_store = true;

   pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
   result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_EVENT);
   if (result != VK_SUCCESS)
      return result;

   cmd_buffer->state.current_sub_cmd->event = (struct pvr_sub_cmd_event){
      .type = PVR_EVENT_TYPE_BARRIER,
      .barrier = {
         .in_render_pass = true,
         .wait_for_stage_mask = src_stage_mask,
         .wait_at_stage_mask = dst_stage_mask,
      },
   };

   pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
   pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_GRAPHICS);

   /* Use existing render setup, but load color attachments from HW BGOBJ */
   cmd_buffer->state.current_sub_cmd->gfx.barrier_load = true;
   cmd_buffer->state.current_sub_cmd->gfx.barrier_store = false;

   return VK_SUCCESS;
}

static VkResult
pvr_cmd_buffer_insert_barrier_event(struct pvr_cmd_buffer *cmd_buffer,
                                    uint32_t src_stage_mask,
                                    uint32_t dst_stage_mask)
{
   VkResult result;

   result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_EVENT);
   if (result != VK_SUCCESS)
      return result;

   cmd_buffer->state.current_sub_cmd->event = (struct pvr_sub_cmd_event){
      .type = PVR_EVENT_TYPE_BARRIER,
      .barrier = {
         .wait_for_stage_mask = src_stage_mask,
         .wait_at_stage_mask = dst_stage_mask,
      },
   };

   return pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
}

/* This is just enough to handle vkCmdPipelineBarrier().
 * TODO: Complete?
 */
void pvr_CmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                             const VkDependencyInfo *pDependencyInfo)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *const state = &cmd_buffer->state;
   const struct pvr_render_pass *const render_pass =
      state->render_pass_info.pass;
   VkPipelineStageFlags vk_src_stage_mask = 0U;
   VkPipelineStageFlags vk_dst_stage_mask = 0U;
   bool is_stencil_store_load_needed;
   uint32_t required_stage_mask = 0U;
   uint32_t src_stage_mask;
   uint32_t dst_stage_mask;
   bool is_barrier_needed;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   for (uint32_t i = 0; i < pDependencyInfo->memoryBarrierCount; i++) {
      vk_src_stage_mask |= pDependencyInfo->pMemoryBarriers[i].srcStageMask;
      vk_dst_stage_mask |= pDependencyInfo->pMemoryBarriers[i].dstStageMask;
   }

   for (uint32_t i = 0; i < pDependencyInfo->bufferMemoryBarrierCount; i++) {
      vk_src_stage_mask |=
         pDependencyInfo->pBufferMemoryBarriers[i].srcStageMask;
      vk_dst_stage_mask |=
         pDependencyInfo->pBufferMemoryBarriers[i].dstStageMask;
   }

   for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; i++) {
      vk_src_stage_mask |=
         pDependencyInfo->pImageMemoryBarriers[i].srcStageMask;
      vk_dst_stage_mask |=
         pDependencyInfo->pImageMemoryBarriers[i].dstStageMask;
   }

   src_stage_mask = pvr_stage_mask_src(vk_src_stage_mask);
   dst_stage_mask = pvr_stage_mask_dst(vk_dst_stage_mask);

   for (uint32_t stage = 0U; stage != PVR_NUM_SYNC_PIPELINE_STAGES; stage++) {
      if (!(dst_stage_mask & BITFIELD_BIT(stage)))
         continue;

      required_stage_mask |= state->barriers_needed[stage];
   }

   src_stage_mask &= required_stage_mask;
   for (uint32_t stage = 0U; stage != PVR_NUM_SYNC_PIPELINE_STAGES; stage++) {
      if (!(dst_stage_mask & BITFIELD_BIT(stage)))
         continue;

      state->barriers_needed[stage] &= ~src_stage_mask;
   }

   if (src_stage_mask == 0 || dst_stage_mask == 0) {
      is_barrier_needed = false;
   } else if (src_stage_mask == PVR_PIPELINE_STAGE_GEOM_BIT &&
              dst_stage_mask == PVR_PIPELINE_STAGE_FRAG_BIT) {
      /* This is implicit so no need to barrier. */
      is_barrier_needed = false;
   } else if (src_stage_mask == dst_stage_mask &&
              util_bitcount(src_stage_mask) == 1) {
      struct pvr_sub_cmd *const current_sub_cmd = state->current_sub_cmd;

      switch (src_stage_mask) {
      case PVR_PIPELINE_STAGE_FRAG_BIT:
         is_barrier_needed = !render_pass;

         if (is_barrier_needed)
            break;

         assert(current_sub_cmd->type == PVR_SUB_CMD_TYPE_GRAPHICS);

         /* Flush all fragment work up to this point. */
         pvr_insert_transparent_obj(cmd_buffer, &current_sub_cmd->gfx);
         break;

      case PVR_PIPELINE_STAGE_COMPUTE_BIT:
         is_barrier_needed = false;

         if (!current_sub_cmd ||
             current_sub_cmd->type != PVR_SUB_CMD_TYPE_COMPUTE) {
            break;
         }

         /* Multiple dispatches can be merged into a single job. When back to
          * back dispatches have a sequential dependency (Compute -> compute
          * pipeline barrier) we need to do the following.
          *   - Dispatch a kernel which fences all previous memory writes and
          *     flushes the MADD cache.
          *   - Issue a compute fence which ensures all previous tasks emitted
          *     by the compute data master are completed before starting
          *     anything new.
          */

         /* Issue Data Fence, Wait for Data Fence (IDFWDF) makes the PDS wait
          * for data.
          */
         pvr_compute_generate_idfwdf(cmd_buffer, &current_sub_cmd->compute);

         pvr_compute_generate_fence(cmd_buffer,
                                    &current_sub_cmd->compute,
                                    false);
         break;

      default:
         is_barrier_needed = false;
         break;
      };
   } else {
      is_barrier_needed = true;
   }

   is_stencil_store_load_needed =
      pvr_is_stencil_store_load_needed(cmd_buffer,
                                       vk_src_stage_mask,
                                       vk_dst_stage_mask,
                                       pDependencyInfo->memoryBarrierCount,
                                       pDependencyInfo->pMemoryBarriers,
                                       pDependencyInfo->imageMemoryBarrierCount,
                                       pDependencyInfo->pImageMemoryBarriers);

   if (is_stencil_store_load_needed) {
      VkResult result;

      result = pvr_cmd_buffer_insert_mid_frag_barrier_event(cmd_buffer,
                                                            src_stage_mask,
                                                            dst_stage_mask);
      if (result != VK_SUCCESS)
         mesa_loge("Failed to insert mid frag barrier event.");
   } else {
      if (is_barrier_needed) {
         VkResult result;

         result = pvr_cmd_buffer_insert_barrier_event(cmd_buffer,
                                                      src_stage_mask,
                                                      dst_stage_mask);
         if (result != VK_SUCCESS)
            mesa_loge("Failed to insert pipeline barrier event.");
      }
   }
}

void pvr_CmdResetEvent2(VkCommandBuffer commandBuffer,
                        VkEvent _event,
                        VkPipelineStageFlags2 stageMask)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_event, event, _event);
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_EVENT);
   if (result != VK_SUCCESS)
      return;

   cmd_buffer->state.current_sub_cmd->event = (struct pvr_sub_cmd_event){
      .type = PVR_EVENT_TYPE_RESET,
      .set_reset = {
         .event = event,
         .wait_for_stage_mask = pvr_stage_mask_src(stageMask),
      },
   };

   pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
}

void pvr_CmdSetEvent2(VkCommandBuffer commandBuffer,
                      VkEvent _event,
                      const VkDependencyInfo *pDependencyInfo)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   PVR_FROM_HANDLE(pvr_event, event, _event);
   VkPipelineStageFlags2 stage_mask = 0;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_EVENT);
   if (result != VK_SUCCESS)
      return;

   for (uint32_t i = 0; i < pDependencyInfo->memoryBarrierCount; i++)
      stage_mask |= pDependencyInfo->pMemoryBarriers[i].srcStageMask;

   for (uint32_t i = 0; i < pDependencyInfo->bufferMemoryBarrierCount; i++)
      stage_mask |= pDependencyInfo->pBufferMemoryBarriers[i].srcStageMask;

   for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; i++)
      stage_mask |= pDependencyInfo->pImageMemoryBarriers[i].srcStageMask;

   cmd_buffer->state.current_sub_cmd->event = (struct pvr_sub_cmd_event){
      .type = PVR_EVENT_TYPE_SET,
      .set_reset = {
         .event = event,
         .wait_for_stage_mask = pvr_stage_mask_dst(stage_mask),
      },
   };

   pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
}

void pvr_CmdWaitEvents2(VkCommandBuffer commandBuffer,
                        uint32_t eventCount,
                        const VkEvent *pEvents,
                        const VkDependencyInfo *pDependencyInfos)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_event **events_array;
   uint32_t *stage_masks;
   VkResult result;

   PVR_CHECK_COMMAND_BUFFER_BUILDING_STATE(cmd_buffer);

   VK_MULTIALLOC(ma);
   vk_multialloc_add(&ma, &events_array, __typeof__(*events_array), eventCount);
   vk_multialloc_add(&ma, &stage_masks, __typeof__(*stage_masks), eventCount);

   if (!vk_multialloc_alloc(&ma,
                            &cmd_buffer->vk.pool->alloc,
                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT)) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
      return;
   }

   result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer, PVR_SUB_CMD_TYPE_EVENT);
   if (result != VK_SUCCESS) {
      vk_free(&cmd_buffer->vk.pool->alloc, events_array);
      return;
   }

   memcpy(events_array, pEvents, sizeof(*events_array) * eventCount);

   for (uint32_t i = 0; i < eventCount; i++) {
      const VkDependencyInfo *info = &pDependencyInfos[i];
      VkPipelineStageFlags2 mask = 0;

      for (uint32_t j = 0; j < info->memoryBarrierCount; j++)
         mask |= info->pMemoryBarriers[j].dstStageMask;

      for (uint32_t j = 0; j < info->bufferMemoryBarrierCount; j++)
         mask |= info->pBufferMemoryBarriers[j].dstStageMask;

      for (uint32_t j = 0; j < info->imageMemoryBarrierCount; j++)
         mask |= info->pImageMemoryBarriers[j].dstStageMask;

      stage_masks[i] = pvr_stage_mask_dst(mask);
   }

   cmd_buffer->state.current_sub_cmd->event = (struct pvr_sub_cmd_event){
      .type = PVR_EVENT_TYPE_WAIT,
      .wait = {
         .count = eventCount,
         .events = events_array,
         .wait_at_stage_masks = stage_masks,
      },
   };

   pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
}

void pvr_CmdWriteTimestamp2(VkCommandBuffer commandBuffer,
                               VkPipelineStageFlags2 stage,
                               VkQueryPool queryPool,
                               uint32_t query)
{
   unreachable("Timestamp queries are not supported.");
}

VkResult pvr_EndCommandBuffer(VkCommandBuffer commandBuffer)
{
   PVR_FROM_HANDLE(pvr_cmd_buffer, cmd_buffer, commandBuffer);
   struct pvr_cmd_buffer_state *state = &cmd_buffer->state;
   VkResult result;

   if (vk_command_buffer_has_error(&cmd_buffer->vk))
      return vk_command_buffer_end(&cmd_buffer->vk);

   /* TODO: We should be freeing all the resources, allocated for recording,
    * here.
    */
   util_dynarray_fini(&state->query_indices);

   result = pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
   if (result != VK_SUCCESS)
      pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);

   return vk_command_buffer_end(&cmd_buffer->vk);
}
