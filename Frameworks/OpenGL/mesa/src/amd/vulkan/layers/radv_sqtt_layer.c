/*
 * Copyright © 2020 Valve Corporation
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

#include "radv_cs.h"
#include "radv_private.h"
#include "radv_shader.h"
#include "vk_common_entrypoints.h"
#include "vk_semaphore.h"
#include "wsi_common_entrypoints.h"

#include "ac_rgp.h"
#include "ac_sqtt.h"

#include "vk_pipeline.h"

void
radv_sqtt_emit_relocated_shaders(struct radv_cmd_buffer *cmd_buffer, struct radv_graphics_pipeline *pipeline)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   struct radv_sqtt_shaders_reloc *reloc = pipeline->sqtt_shaders_reloc;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint64_t va;

   radv_cs_add_buffer(cmd_buffer->device->ws, cs, reloc->bo);

   /* VS */
   if (pipeline->base.shaders[MESA_SHADER_VERTEX]) {
      struct radv_shader *vs = pipeline->base.shaders[MESA_SHADER_VERTEX];

      va = reloc->va[MESA_SHADER_VERTEX];
      if (vs->info.vs.as_ls) {
         radeon_set_sh_reg(cs, R_00B520_SPI_SHADER_PGM_LO_LS, va >> 8);
      } else if (vs->info.vs.as_es) {
         radeon_set_sh_reg_seq(cs, R_00B320_SPI_SHADER_PGM_LO_ES, 2);
         radeon_emit(cs, va >> 8);
         radeon_emit(cs, S_00B324_MEM_BASE(va >> 40));
      } else if (vs->info.is_ngg) {
         radeon_set_sh_reg(cs, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
      } else {
         radeon_set_sh_reg_seq(cs, R_00B120_SPI_SHADER_PGM_LO_VS, 2);
         radeon_emit(cs, va >> 8);
         radeon_emit(cs, S_00B124_MEM_BASE(va >> 40));
      }
   }

   /* TCS */
   if (pipeline->base.shaders[MESA_SHADER_TESS_CTRL]) {
      va = reloc->va[MESA_SHADER_TESS_CTRL];

      if (gfx_level >= GFX9) {
         if (gfx_level >= GFX10) {
            radeon_set_sh_reg(cs, R_00B520_SPI_SHADER_PGM_LO_LS, va >> 8);
         } else {
            radeon_set_sh_reg(cs, R_00B410_SPI_SHADER_PGM_LO_LS, va >> 8);
         }
      } else {
         radeon_set_sh_reg_seq(cs, R_00B420_SPI_SHADER_PGM_LO_HS, 2);
         radeon_emit(cs, va >> 8);
         radeon_emit(cs, S_00B424_MEM_BASE(va >> 40));
      }
   }

   /* TES */
   if (pipeline->base.shaders[MESA_SHADER_TESS_EVAL]) {
      struct radv_shader *tes = pipeline->base.shaders[MESA_SHADER_TESS_EVAL];

      va = reloc->va[MESA_SHADER_TESS_EVAL];
      if (tes->info.is_ngg) {
         radeon_set_sh_reg(cs, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
      } else if (tes->info.tes.as_es) {
         radeon_set_sh_reg_seq(cs, R_00B320_SPI_SHADER_PGM_LO_ES, 2);
         radeon_emit(cs, va >> 8);
         radeon_emit(cs, S_00B324_MEM_BASE(va >> 40));
      } else {
         radeon_set_sh_reg_seq(cs, R_00B120_SPI_SHADER_PGM_LO_VS, 2);
         radeon_emit(cs, va >> 8);
         radeon_emit(cs, S_00B124_MEM_BASE(va >> 40));
      }
   }

   /* GS */
   if (pipeline->base.shaders[MESA_SHADER_GEOMETRY]) {
      struct radv_shader *gs = pipeline->base.shaders[MESA_SHADER_GEOMETRY];

      va = reloc->va[MESA_SHADER_GEOMETRY];
      if (gs->info.is_ngg) {
         radeon_set_sh_reg(cs, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
      } else {
         if (gfx_level >= GFX9) {
            if (gfx_level >= GFX10) {
               radeon_set_sh_reg(cs, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
            } else {
               radeon_set_sh_reg(cs, R_00B210_SPI_SHADER_PGM_LO_ES, va >> 8);
            }
         } else {
            radeon_set_sh_reg_seq(cs, R_00B220_SPI_SHADER_PGM_LO_GS, 2);
            radeon_emit(cs, va >> 8);
            radeon_emit(cs, S_00B224_MEM_BASE(va >> 40));
         }
      }
   }

   /* FS */
   if (pipeline->base.shaders[MESA_SHADER_FRAGMENT]) {
      va = reloc->va[MESA_SHADER_FRAGMENT];

      radeon_set_sh_reg_seq(cs, R_00B020_SPI_SHADER_PGM_LO_PS, 2);
      radeon_emit(cs, va >> 8);
      radeon_emit(cs, S_00B024_MEM_BASE(va >> 40));
   }

   /* MS */
   if (pipeline->base.shaders[MESA_SHADER_MESH]) {
      va = reloc->va[MESA_SHADER_MESH];

      radeon_set_sh_reg(cs, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
   }
}

static uint64_t
radv_sqtt_shader_get_va_reloc(struct radv_pipeline *pipeline, gl_shader_stage stage)
{
   if (pipeline->type == RADV_PIPELINE_GRAPHICS) {
      struct radv_graphics_pipeline *graphics_pipeline = radv_pipeline_to_graphics(pipeline);
      struct radv_sqtt_shaders_reloc *reloc = graphics_pipeline->sqtt_shaders_reloc;
      return reloc->va[stage];
   }

   return radv_shader_get_va(pipeline->shaders[stage]);
}

static VkResult
radv_sqtt_reloc_graphics_shaders(struct radv_device *device, struct radv_graphics_pipeline *pipeline)
{
   struct radv_shader_dma_submission *submission = NULL;
   struct radv_sqtt_shaders_reloc *reloc;
   uint32_t code_size = 0;

   reloc = calloc(1, sizeof(*reloc));
   if (!reloc)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   /* Compute the total code size. */
   for (int i = 0; i < MESA_VULKAN_SHADER_STAGES; i++) {
      const struct radv_shader *shader = pipeline->base.shaders[i];
      if (!shader)
         continue;

      code_size += align(shader->code_size, RADV_SHADER_ALLOC_ALIGNMENT);
   }

   /* Allocate memory for all shader binaries. */
   reloc->alloc = radv_alloc_shader_memory(device, code_size, false, pipeline);
   if (!reloc->alloc) {
      free(reloc);
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;
   }

   reloc->bo = reloc->alloc->arena->bo;

   /* Relocate shader binaries to be contiguous in memory as requested by RGP. */
   uint64_t slab_va = radv_buffer_get_va(reloc->bo) + reloc->alloc->offset;
   char *slab_ptr = reloc->alloc->arena->ptr + reloc->alloc->offset;
   uint64_t offset = 0;

   if (device->shader_use_invisible_vram) {
      submission = radv_shader_dma_get_submission(device, reloc->bo, slab_va, code_size);
      if (!submission)
         return VK_ERROR_UNKNOWN;
   }

   for (int i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
      const struct radv_shader *shader = pipeline->base.shaders[i];
      void *dest_ptr;
      if (!shader)
         continue;

      reloc->va[i] = slab_va + offset;

      if (device->shader_use_invisible_vram)
         dest_ptr = submission->ptr + offset;
      else
         dest_ptr = slab_ptr + offset;

      memcpy(dest_ptr, shader->code, shader->code_size);

      offset += align(shader->code_size, RADV_SHADER_ALLOC_ALIGNMENT);
   }

   if (device->shader_use_invisible_vram) {
      if (!radv_shader_dma_submit(device, submission, &pipeline->base.shader_upload_seq))
         return VK_ERROR_UNKNOWN;
   }

   pipeline->sqtt_shaders_reloc = reloc;

   return VK_SUCCESS;
}

static void
radv_write_begin_general_api_marker(struct radv_cmd_buffer *cmd_buffer, enum rgp_sqtt_marker_general_api_type api_type)
{
   struct rgp_sqtt_marker_general_api marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_GENERAL_API;
   marker.api_type = api_type;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
}

static void
radv_write_end_general_api_marker(struct radv_cmd_buffer *cmd_buffer, enum rgp_sqtt_marker_general_api_type api_type)
{
   struct rgp_sqtt_marker_general_api marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_GENERAL_API;
   marker.api_type = api_type;
   marker.is_end = 1;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
}

static void
radv_write_event_marker(struct radv_cmd_buffer *cmd_buffer, enum rgp_sqtt_marker_event_type api_type,
                        uint32_t vertex_offset_user_data, uint32_t instance_offset_user_data,
                        uint32_t draw_index_user_data)
{
   struct rgp_sqtt_marker_event marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_EVENT;
   marker.api_type = api_type;
   marker.cmd_id = cmd_buffer->state.num_events++;
   marker.cb_id = cmd_buffer->sqtt_cb_id;

   if (vertex_offset_user_data == UINT_MAX || instance_offset_user_data == UINT_MAX) {
      vertex_offset_user_data = 0;
      instance_offset_user_data = 0;
   }

   if (draw_index_user_data == UINT_MAX)
      draw_index_user_data = vertex_offset_user_data;

   marker.vertex_offset_reg_idx = vertex_offset_user_data;
   marker.instance_offset_reg_idx = instance_offset_user_data;
   marker.draw_index_reg_idx = draw_index_user_data;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
}

static void
radv_write_event_with_dims_marker(struct radv_cmd_buffer *cmd_buffer, enum rgp_sqtt_marker_event_type api_type,
                                  uint32_t x, uint32_t y, uint32_t z)
{
   struct rgp_sqtt_marker_event_with_dims marker = {0};

   marker.event.identifier = RGP_SQTT_MARKER_IDENTIFIER_EVENT;
   marker.event.api_type = api_type;
   marker.event.cmd_id = cmd_buffer->state.num_events++;
   marker.event.cb_id = cmd_buffer->sqtt_cb_id;
   marker.event.has_thread_dims = 1;

   marker.thread_x = x;
   marker.thread_y = y;
   marker.thread_z = z;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
}

static void
radv_write_user_event_marker(struct radv_cmd_buffer *cmd_buffer, enum rgp_sqtt_marker_user_event_type type,
                             const char *str)
{
   if (type == UserEventPop) {
      assert(str == NULL);
      struct rgp_sqtt_marker_user_event marker = {0};
      marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_USER_EVENT;
      marker.data_type = type;

      radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
   } else {
      assert(str != NULL);
      unsigned len = strlen(str);
      struct rgp_sqtt_marker_user_event_with_length marker = {0};
      marker.user_event.identifier = RGP_SQTT_MARKER_IDENTIFIER_USER_EVENT;
      marker.user_event.data_type = type;
      marker.length = align(len, 4);

      uint8_t *buffer = alloca(sizeof(marker) + marker.length);
      memset(buffer, 0, sizeof(marker) + marker.length);
      memcpy(buffer, &marker, sizeof(marker));
      memcpy(buffer + sizeof(marker), str, len);

      radv_emit_sqtt_userdata(cmd_buffer, buffer, sizeof(marker) / 4 + marker.length / 4);
   }
}

void
radv_describe_begin_cmd_buffer(struct radv_cmd_buffer *cmd_buffer)
{
   uint64_t device_id = (uintptr_t)cmd_buffer->device;
   struct rgp_sqtt_marker_cb_start marker = {0};

   if (likely(!cmd_buffer->device->sqtt.bo))
      return;

   /* Reserve a command buffer ID for SQTT. */
   enum amd_ip_type ip_type = radv_queue_family_to_ring(cmd_buffer->device->physical_device, cmd_buffer->qf);
   union rgp_sqtt_marker_cb_id cb_id = ac_sqtt_get_next_cmdbuf_id(&cmd_buffer->device->sqtt, ip_type);
   cmd_buffer->sqtt_cb_id = cb_id.all;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_CB_START;
   marker.cb_id = cmd_buffer->sqtt_cb_id;
   marker.device_id_low = device_id;
   marker.device_id_high = device_id >> 32;
   marker.queue = cmd_buffer->qf;
   marker.queue_flags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

   if (cmd_buffer->qf == RADV_QUEUE_GENERAL)
      marker.queue_flags |= VK_QUEUE_GRAPHICS_BIT;

   if (cmd_buffer->device->instance->drirc.legacy_sparse_binding)
      marker.queue_flags |= VK_QUEUE_SPARSE_BINDING_BIT;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
}

void
radv_describe_end_cmd_buffer(struct radv_cmd_buffer *cmd_buffer)
{
   uint64_t device_id = (uintptr_t)cmd_buffer->device;
   struct rgp_sqtt_marker_cb_end marker = {0};

   if (likely(!cmd_buffer->device->sqtt.bo))
      return;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_CB_END;
   marker.cb_id = cmd_buffer->sqtt_cb_id;
   marker.device_id_low = device_id;
   marker.device_id_high = device_id >> 32;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
}

void
radv_describe_draw(struct radv_cmd_buffer *cmd_buffer)
{
   if (likely(!cmd_buffer->device->sqtt.bo))
      return;

   radv_write_event_marker(cmd_buffer, cmd_buffer->state.current_event_type, UINT_MAX, UINT_MAX, UINT_MAX);
}

void
radv_describe_dispatch(struct radv_cmd_buffer *cmd_buffer, const struct radv_dispatch_info *info)
{
   if (likely(!cmd_buffer->device->sqtt.bo))
      return;

   if (info->indirect) {
      radv_write_event_marker(cmd_buffer, cmd_buffer->state.current_event_type, UINT_MAX, UINT_MAX, UINT_MAX);
   } else {
      radv_write_event_with_dims_marker(cmd_buffer, cmd_buffer->state.current_event_type, info->blocks[0],
                                        info->blocks[1], info->blocks[2]);
   }
}

void
radv_describe_begin_render_pass_clear(struct radv_cmd_buffer *cmd_buffer, VkImageAspectFlagBits aspects)
{
   cmd_buffer->state.current_event_type =
      (aspects & VK_IMAGE_ASPECT_COLOR_BIT) ? EventRenderPassColorClear : EventRenderPassDepthStencilClear;
}

void
radv_describe_end_render_pass_clear(struct radv_cmd_buffer *cmd_buffer)
{
   cmd_buffer->state.current_event_type = EventInternalUnknown;
}

void
radv_describe_begin_render_pass_resolve(struct radv_cmd_buffer *cmd_buffer)
{
   cmd_buffer->state.current_event_type = EventRenderPassResolve;
}

void
radv_describe_end_render_pass_resolve(struct radv_cmd_buffer *cmd_buffer)
{
   cmd_buffer->state.current_event_type = EventInternalUnknown;
}

void
radv_describe_barrier_end_delayed(struct radv_cmd_buffer *cmd_buffer)
{
   struct rgp_sqtt_marker_barrier_end marker = {0};

   if (likely(!cmd_buffer->device->sqtt.bo) || !cmd_buffer->state.pending_sqtt_barrier_end)
      return;

   cmd_buffer->state.pending_sqtt_barrier_end = false;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BARRIER_END;
   marker.cb_id = cmd_buffer->sqtt_cb_id;

   marker.num_layout_transitions = cmd_buffer->state.num_layout_transitions;

   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_WAIT_ON_EOP_TS)
      marker.wait_on_eop_ts = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_VS_PARTIAL_FLUSH)
      marker.vs_partial_flush = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_PS_PARTIAL_FLUSH)
      marker.ps_partial_flush = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_CS_PARTIAL_FLUSH)
      marker.cs_partial_flush = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_PFP_SYNC_ME)
      marker.pfp_sync_me = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_SYNC_CP_DMA)
      marker.sync_cp_dma = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_VMEM_L0)
      marker.inval_tcp = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_ICACHE)
      marker.inval_sqI = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_SMEM_L0)
      marker.inval_sqK = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_FLUSH_L2)
      marker.flush_tcc = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_L2)
      marker.inval_tcc = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_FLUSH_CB)
      marker.flush_cb = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_CB)
      marker.inval_cb = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_FLUSH_DB)
      marker.flush_db = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_DB)
      marker.inval_db = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_L1)
      marker.inval_gl1 = true;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);

   cmd_buffer->state.num_layout_transitions = 0;
}

void
radv_describe_barrier_start(struct radv_cmd_buffer *cmd_buffer, enum rgp_barrier_reason reason)
{
   struct rgp_sqtt_marker_barrier_start marker = {0};

   if (likely(!cmd_buffer->device->sqtt.bo))
      return;

   if (cmd_buffer->state.in_barrier) {
      assert(!"attempted to start a barrier while already in a barrier");
      return;
   }

   radv_describe_barrier_end_delayed(cmd_buffer);
   cmd_buffer->state.sqtt_flush_bits = 0;
   cmd_buffer->state.in_barrier = true;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BARRIER_START;
   marker.cb_id = cmd_buffer->sqtt_cb_id;
   marker.dword02 = reason;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
}

void
radv_describe_barrier_end(struct radv_cmd_buffer *cmd_buffer)
{
   cmd_buffer->state.in_barrier = false;
   cmd_buffer->state.pending_sqtt_barrier_end = true;
}

void
radv_describe_layout_transition(struct radv_cmd_buffer *cmd_buffer, const struct radv_barrier_data *barrier)
{
   struct rgp_sqtt_marker_layout_transition marker = {0};

   if (likely(!cmd_buffer->device->sqtt.bo))
      return;

   if (!cmd_buffer->state.in_barrier) {
      assert(!"layout transition marker should be only emitted inside a barrier marker");
      return;
   }

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_LAYOUT_TRANSITION;
   marker.depth_stencil_expand = barrier->layout_transitions.depth_stencil_expand;
   marker.htile_hiz_range_expand = barrier->layout_transitions.htile_hiz_range_expand;
   marker.depth_stencil_resummarize = barrier->layout_transitions.depth_stencil_resummarize;
   marker.dcc_decompress = barrier->layout_transitions.dcc_decompress;
   marker.fmask_decompress = barrier->layout_transitions.fmask_decompress;
   marker.fast_clear_eliminate = barrier->layout_transitions.fast_clear_eliminate;
   marker.fmask_color_expand = barrier->layout_transitions.fmask_color_expand;
   marker.init_mask_ram = barrier->layout_transitions.init_mask_ram;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);

   cmd_buffer->state.num_layout_transitions++;
}

static void
radv_describe_pipeline_bind(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint pipelineBindPoint,
                            struct radv_pipeline *pipeline)
{
   struct rgp_sqtt_marker_pipeline_bind marker = {0};

   if (likely(!cmd_buffer->device->sqtt.bo))
      return;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BIND_PIPELINE;
   marker.cb_id = cmd_buffer->sqtt_cb_id;
   marker.bind_point = pipelineBindPoint;
   marker.api_pso_hash[0] = pipeline->pipeline_hash;
   marker.api_pso_hash[1] = pipeline->pipeline_hash >> 32;

   radv_emit_sqtt_userdata(cmd_buffer, &marker, sizeof(marker) / 4);
}

/* Queue events */
static void
radv_describe_queue_event(struct radv_queue *queue, struct rgp_queue_event_record *record)
{
   struct radv_device *device = queue->device;
   struct ac_sqtt *sqtt = &device->sqtt;
   struct rgp_queue_event *queue_event = &sqtt->rgp_queue_event;

   simple_mtx_lock(&queue_event->lock);
   list_addtail(&record->list, &queue_event->record);
   queue_event->record_count++;
   simple_mtx_unlock(&queue_event->lock);
}

static VkResult
radv_describe_queue_present(struct radv_queue *queue, uint64_t cpu_timestamp, void *gpu_timestamp_ptr)
{
   struct rgp_queue_event_record *record;

   record = calloc(1, sizeof(struct rgp_queue_event_record));
   if (!record)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   record->event_type = SQTT_QUEUE_TIMING_EVENT_PRESENT;
   record->cpu_timestamp = cpu_timestamp;
   record->gpu_timestamps[0] = gpu_timestamp_ptr;
   record->queue_info_index = queue->vk.queue_family_index;

   radv_describe_queue_event(queue, record);

   return VK_SUCCESS;
}

static VkResult
radv_describe_queue_submit(struct radv_queue *queue, struct radv_cmd_buffer *cmd_buffer, uint32_t cmdbuf_idx,
                           uint64_t cpu_timestamp, void *pre_gpu_timestamp_ptr, void *post_gpu_timestamp_ptr)
{
   struct radv_device *device = queue->device;
   struct rgp_queue_event_record *record;

   record = calloc(1, sizeof(struct rgp_queue_event_record));
   if (!record)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   record->event_type = SQTT_QUEUE_TIMING_EVENT_CMDBUF_SUBMIT;
   record->api_id = (uintptr_t)cmd_buffer;
   record->cpu_timestamp = cpu_timestamp;
   record->frame_index = device->vk.current_frame;
   record->gpu_timestamps[0] = pre_gpu_timestamp_ptr;
   record->gpu_timestamps[1] = post_gpu_timestamp_ptr;
   record->queue_info_index = queue->vk.queue_family_index;
   record->submit_sub_index = cmdbuf_idx;

   radv_describe_queue_event(queue, record);

   return VK_SUCCESS;
}

static VkResult
radv_describe_queue_semaphore(struct radv_queue *queue, struct vk_semaphore *sync,
                              enum sqtt_queue_event_type event_type)
{
   struct rgp_queue_event_record *record;

   record = calloc(1, sizeof(struct rgp_queue_event_record));
   if (!record)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   record->event_type = event_type;
   record->api_id = (uintptr_t)sync;
   record->cpu_timestamp = os_time_get_nano();
   record->queue_info_index = queue->vk.queue_family_index;

   radv_describe_queue_event(queue, record);

   return VK_SUCCESS;
}

static void
radv_handle_sqtt(VkQueue _queue)
{
   RADV_FROM_HANDLE(radv_queue, queue, _queue);

   bool trigger = queue->device->sqtt_triggered;
   queue->device->sqtt_triggered = false;

   if (queue->device->sqtt_enabled) {
      struct ac_sqtt_trace sqtt_trace = {0};

      radv_end_sqtt(queue);
      queue->device->sqtt_enabled = false;

      /* TODO: Do something better than this whole sync. */
      queue->device->vk.dispatch_table.QueueWaitIdle(_queue);

      if (radv_get_sqtt_trace(queue, &sqtt_trace)) {
         struct ac_spm_trace spm_trace;

         if (queue->device->spm.bo)
            ac_spm_get_trace(&queue->device->spm, &spm_trace);

         ac_dump_rgp_capture(&queue->device->physical_device->rad_info, &sqtt_trace,
                             queue->device->spm.bo ? &spm_trace : NULL);
      } else {
         /* Trigger a new capture if the driver failed to get
          * the trace because the buffer was too small.
          */
         trigger = true;
      }

      /* Clear resources used for this capture. */
      radv_reset_sqtt_trace(queue->device);
   }

   if (trigger) {
      if (ac_check_profile_state(&queue->device->physical_device->rad_info)) {
         fprintf(stderr, "radv: Canceling RGP trace request as a hang condition has been "
                         "detected. Force the GPU into a profiling mode with e.g. "
                         "\"echo profile_peak  > "
                         "/sys/class/drm/card0/device/power_dpm_force_performance_level\"\n");
         return;
      }

      /* Sample CPU/GPU clocks before starting the trace. */
      if (!radv_sqtt_sample_clocks(queue->device)) {
         fprintf(stderr, "radv: Failed to sample clocks\n");
      }

      radv_begin_sqtt(queue);
      assert(!queue->device->sqtt_enabled);
      queue->device->sqtt_enabled = true;
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
sqtt_QueuePresentKHR(VkQueue _queue, const VkPresentInfoKHR *pPresentInfo)
{
   RADV_FROM_HANDLE(radv_queue, queue, _queue);
   VkResult result;

   queue->sqtt_present = true;

   result = queue->device->layer_dispatch.rgp.QueuePresentKHR(_queue, pPresentInfo);
   if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      return result;

   queue->sqtt_present = false;

   radv_handle_sqtt(_queue);

   return VK_SUCCESS;
}

static VkResult
radv_sqtt_wsi_submit(VkQueue _queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence _fence)
{
   RADV_FROM_HANDLE(radv_queue, queue, _queue);
   struct radv_device *device = queue->device;
   VkCommandBufferSubmitInfo *new_cmdbufs = NULL;
   struct radeon_winsys_bo *gpu_timestamp_bo;
   uint32_t gpu_timestamp_offset;
   VkCommandBuffer timed_cmdbuf;
   void *gpu_timestamp_ptr;
   uint64_t cpu_timestamp;
   VkResult result = VK_SUCCESS;

   assert(submitCount <= 1 && pSubmits != NULL);

   for (uint32_t i = 0; i < submitCount; i++) {
      const VkSubmitInfo2 *pSubmit = &pSubmits[i];
      VkSubmitInfo2 sqtt_submit = *pSubmit;

      assert(sqtt_submit.commandBufferInfoCount <= 1);

      /* Command buffers */
      uint32_t new_cmdbuf_count = sqtt_submit.commandBufferInfoCount + 1;

      new_cmdbufs = malloc(new_cmdbuf_count * sizeof(*new_cmdbufs));
      if (!new_cmdbufs)
         return VK_ERROR_OUT_OF_HOST_MEMORY;

      /* Sample the current CPU time before building the GPU timestamp cmdbuf. */
      cpu_timestamp = os_time_get_nano();

      result = radv_sqtt_acquire_gpu_timestamp(device, &gpu_timestamp_bo, &gpu_timestamp_offset, &gpu_timestamp_ptr);
      if (result != VK_SUCCESS)
         goto fail;

      result = radv_sqtt_get_timed_cmdbuf(queue, gpu_timestamp_bo, gpu_timestamp_offset,
                                          VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, &timed_cmdbuf);
      if (result != VK_SUCCESS)
         goto fail;

      new_cmdbufs[0] = (VkCommandBufferSubmitInfo){
         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
         .commandBuffer = timed_cmdbuf,
      };

      if (sqtt_submit.commandBufferInfoCount == 1)
         new_cmdbufs[1] = sqtt_submit.pCommandBufferInfos[0];

      sqtt_submit.commandBufferInfoCount = new_cmdbuf_count;
      sqtt_submit.pCommandBufferInfos = new_cmdbufs;

      radv_describe_queue_present(queue, cpu_timestamp, gpu_timestamp_ptr);

      result = queue->device->layer_dispatch.rgp.QueueSubmit2(_queue, 1, &sqtt_submit, _fence);
      if (result != VK_SUCCESS)
         goto fail;

      FREE(new_cmdbufs);
   }

   return result;

fail:
   FREE(new_cmdbufs);
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
sqtt_QueueSubmit2(VkQueue _queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence _fence)
{
   RADV_FROM_HANDLE(radv_queue, queue, _queue);
   const bool is_gfx_or_ace = queue->state.qf == RADV_QUEUE_GENERAL || queue->state.qf == RADV_QUEUE_COMPUTE;
   struct radv_device *device = queue->device;
   VkCommandBufferSubmitInfo *new_cmdbufs = NULL;
   VkResult result = VK_SUCCESS;

   /* Only consider queue events on graphics/compute when enabled. */
   if (!device->sqtt_enabled || !radv_sqtt_queue_events_enabled() || !is_gfx_or_ace)
      return queue->device->layer_dispatch.rgp.QueueSubmit2(_queue, submitCount, pSubmits, _fence);

   for (uint32_t i = 0; i < submitCount; i++) {
      const VkSubmitInfo2 *pSubmit = &pSubmits[i];

      /* Wait semaphores */
      for (uint32_t j = 0; j < pSubmit->waitSemaphoreInfoCount; j++) {
         const VkSemaphoreSubmitInfo *pWaitSemaphoreInfo = &pSubmit->pWaitSemaphoreInfos[j];
         VK_FROM_HANDLE(vk_semaphore, sem, pWaitSemaphoreInfo->semaphore);
         radv_describe_queue_semaphore(queue, sem, SQTT_QUEUE_TIMING_EVENT_WAIT_SEMAPHORE);
      }
   }

   if (queue->sqtt_present)
      return radv_sqtt_wsi_submit(_queue, submitCount, pSubmits, _fence);

   for (uint32_t i = 0; i < submitCount; i++) {
      const VkSubmitInfo2 *pSubmit = &pSubmits[i];
      VkSubmitInfo2 sqtt_submit = *pSubmit;

      /* Command buffers */
      uint32_t new_cmdbuf_count = sqtt_submit.commandBufferInfoCount * 3;
      uint32_t cmdbuf_idx = 0;

      new_cmdbufs = malloc(new_cmdbuf_count * sizeof(*new_cmdbufs));
      if (!new_cmdbufs)
         return VK_ERROR_OUT_OF_HOST_MEMORY;

      for (uint32_t j = 0; j < sqtt_submit.commandBufferInfoCount; j++) {
         const VkCommandBufferSubmitInfo *pCommandBufferInfo = &sqtt_submit.pCommandBufferInfos[j];
         struct radeon_winsys_bo *gpu_timestamps_bo[2];
         uint32_t gpu_timestamps_offset[2];
         VkCommandBuffer pre_timed_cmdbuf, post_timed_cmdbuf;
         void *gpu_timestamps_ptr[2];
         uint64_t cpu_timestamp;

         /* Sample the current CPU time before building the timed cmdbufs. */
         cpu_timestamp = os_time_get_nano();

         result = radv_sqtt_acquire_gpu_timestamp(queue->device, &gpu_timestamps_bo[0], &gpu_timestamps_offset[0],
                                                  &gpu_timestamps_ptr[0]);
         if (result != VK_SUCCESS)
            goto fail;

         result = radv_sqtt_get_timed_cmdbuf(queue, gpu_timestamps_bo[0], gpu_timestamps_offset[0],
                                             VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, &pre_timed_cmdbuf);
         if (result != VK_SUCCESS)
            goto fail;

         new_cmdbufs[cmdbuf_idx++] = (VkCommandBufferSubmitInfo){
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = pre_timed_cmdbuf,
         };

         new_cmdbufs[cmdbuf_idx++] = *pCommandBufferInfo;

         result = radv_sqtt_acquire_gpu_timestamp(queue->device, &gpu_timestamps_bo[1], &gpu_timestamps_offset[1],
                                                  &gpu_timestamps_ptr[1]);
         if (result != VK_SUCCESS)
            goto fail;

         result = radv_sqtt_get_timed_cmdbuf(queue, gpu_timestamps_bo[1], gpu_timestamps_offset[1],
                                             VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, &post_timed_cmdbuf);
         if (result != VK_SUCCESS)
            goto fail;

         new_cmdbufs[cmdbuf_idx++] = (VkCommandBufferSubmitInfo){
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = post_timed_cmdbuf,
         };

         RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, pCommandBufferInfo->commandBuffer);
         radv_describe_queue_submit(queue, cmd_buffer, j, cpu_timestamp, gpu_timestamps_ptr[0], gpu_timestamps_ptr[1]);
      }

      sqtt_submit.commandBufferInfoCount = new_cmdbuf_count;
      sqtt_submit.pCommandBufferInfos = new_cmdbufs;

      result = queue->device->layer_dispatch.rgp.QueueSubmit2(_queue, 1, &sqtt_submit, _fence);
      if (result != VK_SUCCESS)
         goto fail;

      /* Signal semaphores */
      for (uint32_t j = 0; j < sqtt_submit.signalSemaphoreInfoCount; j++) {
         const VkSemaphoreSubmitInfo *pSignalSemaphoreInfo = &sqtt_submit.pSignalSemaphoreInfos[j];
         VK_FROM_HANDLE(vk_semaphore, sem, pSignalSemaphoreInfo->semaphore);
         radv_describe_queue_semaphore(queue, sem, SQTT_QUEUE_TIMING_EVENT_SIGNAL_SEMAPHORE);
      }

      FREE(new_cmdbufs);
   }

   return result;

fail:
   FREE(new_cmdbufs);
   return result;
}

#define EVENT_MARKER_BASE(cmd_name, api_name, event_name, ...)                                                         \
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);                                                       \
   radv_write_begin_general_api_marker(cmd_buffer, ApiCmd##api_name);                                                  \
   cmd_buffer->state.current_event_type = EventCmd##event_name;                                                        \
   cmd_buffer->device->layer_dispatch.rgp.Cmd##cmd_name(__VA_ARGS__);                                                  \
   cmd_buffer->state.current_event_type = EventInternalUnknown;                                                        \
   radv_write_end_general_api_marker(cmd_buffer, ApiCmd##api_name);

#define EVENT_MARKER_ALIAS(cmd_name, api_name, ...) EVENT_MARKER_BASE(cmd_name, api_name, api_name, __VA_ARGS__);

#define EVENT_MARKER(cmd_name, ...) EVENT_MARKER_ALIAS(cmd_name, cmd_name, __VA_ARGS__);

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
             uint32_t firstInstance)
{
   EVENT_MARKER(Draw, commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                    int32_t vertexOffset, uint32_t firstInstance)
{
   EVENT_MARKER(DrawIndexed, commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                     uint32_t stride)
{
   EVENT_MARKER(DrawIndirect, commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                            uint32_t stride)
{
   EVENT_MARKER(DrawIndexedIndirect, commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
{
   EVENT_MARKER(DrawIndirectCount, commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                 uint32_t stride)
{
   EVENT_MARKER(DrawIndexedIndirectCount, commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                stride);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
   EVENT_MARKER_ALIAS(DispatchBase, Dispatch, commandBuffer, 0, 0, 0, x, y, z);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
   EVENT_MARKER(DispatchIndirect, commandBuffer, buffer, offset);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo)
{
   EVENT_MARKER_ALIAS(CopyBuffer2, CopyBuffer, commandBuffer, pCopyBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize fillSize,
                   uint32_t data)
{
   EVENT_MARKER(FillBuffer, commandBuffer, dstBuffer, dstOffset, fillSize, data);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize,
                     const void *pData)
{
   EVENT_MARKER(UpdateBuffer, commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo)
{
   EVENT_MARKER_ALIAS(CopyImage2, CopyImage, commandBuffer, pCopyImageInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo)
{
   EVENT_MARKER_ALIAS(CopyBufferToImage2, CopyBufferToImage, commandBuffer, pCopyBufferToImageInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo)
{
   EVENT_MARKER_ALIAS(CopyImageToBuffer2, CopyImageToBuffer, commandBuffer, pCopyImageToBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo)
{
   EVENT_MARKER_ALIAS(BlitImage2, BlitImage, commandBuffer, pBlitImageInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image_h, VkImageLayout imageLayout,
                        const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
   EVENT_MARKER(ClearColorImage, commandBuffer, image_h, imageLayout, pColor, rangeCount, pRanges);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image_h, VkImageLayout imageLayout,
                               const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                               const VkImageSubresourceRange *pRanges)
{
   EVENT_MARKER(ClearDepthStencilImage, commandBuffer, image_h, imageLayout, pDepthStencil, rangeCount, pRanges);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment *pAttachments,
                         uint32_t rectCount, const VkClearRect *pRects)
{
   EVENT_MARKER(ClearAttachments, commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo)
{
   EVENT_MARKER_ALIAS(ResolveImage2, ResolveImage, commandBuffer, pResolveImageInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                    const VkDependencyInfo *pDependencyInfos)
{
   EVENT_MARKER_ALIAS(WaitEvents2, WaitEvents, commandBuffer, eventCount, pEvents, pDependencyInfos);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo)
{
   EVENT_MARKER_ALIAS(PipelineBarrier2, PipelineBarrier, commandBuffer, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
   EVENT_MARKER(ResetQueryPool, commandBuffer, queryPool, firstQuery, queryCount);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                             uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                             VkQueryResultFlags flags)
{
   EVENT_MARKER(CopyQueryPoolResults, commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride,
                flags);
}

#define EVENT_RT_MARKER(cmd_name, flags, ...) EVENT_MARKER_BASE(cmd_name, Dispatch, cmd_name | flags, __VA_ARGS__);

#define EVENT_RT_MARKER_ALIAS(cmd_name, event_name, flags, ...)                                                        \
   EVENT_MARKER_BASE(cmd_name, Dispatch, event_name | flags, __VA_ARGS__);

static uint32_t
radv_get_ray_tracing_type(VkCommandBuffer commandBuffer)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   struct radv_ray_tracing_pipeline *pipeline = cmd_buffer->state.rt_pipeline;

   bool monolithic = true;
   for (uint32_t i = 0; i < pipeline->stage_count; i++)
      monolithic &= pipeline->stages[i].can_inline;

   return monolithic ? 0 : ApiRayTracingSeparateCompiled;
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                     const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                     const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                     const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                     uint32_t height, uint32_t depth)
{
   EVENT_RT_MARKER(TraceRaysKHR, radv_get_ray_tracing_type(commandBuffer), commandBuffer, pRaygenShaderBindingTable,
                   pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                             const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                             VkDeviceAddress indirectDeviceAddress)
{
   EVENT_RT_MARKER(TraceRaysIndirectKHR, radv_get_ray_tracing_type(commandBuffer), commandBuffer,
                   pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable,
                   pCallableShaderBindingTable, indirectDeviceAddress);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress)
{
   EVENT_RT_MARKER_ALIAS(TraceRaysIndirect2KHR, TraceRaysIndirectKHR, radv_get_ray_tracing_type(commandBuffer),
                         commandBuffer, indirectDeviceAddress);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                       const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                       const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos)
{
   EVENT_RT_MARKER(BuildAccelerationStructuresKHR, 0, commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo)
{
   EVENT_RT_MARKER(CopyAccelerationStructureKHR, 0, commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                             const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo)
{
   EVENT_RT_MARKER(CopyAccelerationStructureToMemoryKHR, 0, commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                             const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo)
{
   EVENT_RT_MARKER(CopyMemoryToAccelerationStructureKHR, 0, commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
   EVENT_MARKER(DrawMeshTasksEXT, commandBuffer, x, y, z);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                 uint32_t drawCount, uint32_t stride)
{
   EVENT_MARKER(DrawMeshTasksIndirectEXT, commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                      uint32_t stride)
{
   EVENT_MARKER(DrawMeshTasksIndirectCountEXT, commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                maxDrawCount, stride);
}

#undef EVENT_RT_MARKER_ALIAS
#undef EVENT_RT_MARKER

#undef EVENT_MARKER
#undef EVENT_MARKER_ALIAS
#undef EVENT_MARKER_BASE

#define API_MARKER_ALIAS(cmd_name, api_name, ...)                                                                      \
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);                                                       \
   radv_write_begin_general_api_marker(cmd_buffer, ApiCmd##api_name);                                                  \
   cmd_buffer->device->layer_dispatch.rgp.Cmd##cmd_name(__VA_ARGS__);                                                  \
   radv_write_end_general_api_marker(cmd_buffer, ApiCmd##api_name);

#define API_MARKER(cmd_name, ...) API_MARKER_ALIAS(cmd_name, cmd_name, __VA_ARGS__);

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline _pipeline)
{
   RADV_FROM_HANDLE(radv_pipeline, pipeline, _pipeline);

   API_MARKER(BindPipeline, commandBuffer, pipelineBindPoint, _pipeline);

   if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
      /* RGP seems to expect a compute bind point to detect and report RT pipelines, which makes
       * sense somehow given that RT shaders are compiled to an unified compute shader.
       */
      radv_describe_pipeline_bind(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
   } else {
      radv_describe_pipeline_bind(cmd_buffer, pipelineBindPoint, pipeline);
   }
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                           VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                           const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                           const uint32_t *pDynamicOffsets)
{
   API_MARKER(BindDescriptorSets, commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
              pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
   API_MARKER(BindIndexBuffer, commandBuffer, buffer, offset, indexType);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                           const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                           const VkDeviceSize *pStrides)
{
   API_MARKER_ALIAS(BindVertexBuffers2, BindVertexBuffers, commandBuffer, firstBinding, bindingCount, pBuffers,
                    pOffsets, pSizes, pStrides);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
   API_MARKER(BeginQuery, commandBuffer, queryPool, query, flags);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
   API_MARKER(EndQuery, commandBuffer, queryPool, query);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                        uint32_t query)
{
   API_MARKER_ALIAS(WriteTimestamp2, WriteTimestamp, commandBuffer, stage, queryPool, query);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                      uint32_t offset, uint32_t size, const void *pValues)
{
   API_MARKER(PushConstants, commandBuffer, layout, stageFlags, offset, size, pValues);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo)
{
   API_MARKER_ALIAS(BeginRendering, BeginRenderPass, commandBuffer, pRenderingInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdEndRendering(VkCommandBuffer commandBuffer)
{
   API_MARKER_ALIAS(EndRendering, EndRenderPass, commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer *pCmdBuffers)
{
   API_MARKER(ExecuteCommands, commandBuffer, commandBufferCount, pCmdBuffers);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                   const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo)
{
   /* There is no ExecuteIndirect Vulkan event in RGP yet. */
   API_MARKER_ALIAS(ExecuteGeneratedCommandsNV, ExecuteCommands, commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                    const VkViewport *pViewports)
{
   API_MARKER(SetViewport, commandBuffer, firstViewport, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                   const VkRect2D *pScissors)
{
   API_MARKER(SetScissor, commandBuffer, firstScissor, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
   API_MARKER(SetLineWidth, commandBuffer, lineWidth);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                     float depthBiasSlopeFactor)
{
   API_MARKER(SetDepthBias, commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
   API_MARKER(SetBlendConstants, commandBuffer, blendConstants);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
   API_MARKER(SetDepthBounds, commandBuffer, minDepthBounds, maxDepthBounds);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
   API_MARKER(SetStencilCompareMask, commandBuffer, faceMask, compareMask);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
   API_MARKER(SetStencilWriteMask, commandBuffer, faceMask, writeMask);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
   API_MARKER(SetStencilReference, commandBuffer, faceMask, reference);
}

/* VK_EXT_debug_marker */
VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventPush, pMarkerInfo->pMarkerName);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventPop, NULL);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventTrigger, pMarkerInfo->pMarkerName);
}

VKAPI_ATTR VkResult VKAPI_CALL
sqtt_DebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo)
{
   /* no-op */
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
sqtt_DebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo)
{
   /* no-op */
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventPush, pLabelInfo->pLabelName);

   cmd_buffer->device->layer_dispatch.rgp.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventPop, NULL);

   cmd_buffer->device->layer_dispatch.rgp.CmdEndDebugUtilsLabelEXT(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL
sqtt_CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventTrigger, pLabelInfo->pLabelName);

   cmd_buffer->device->layer_dispatch.rgp.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

/* Pipelines */
static enum rgp_hardware_stages
radv_get_rgp_shader_stage(struct radv_shader *shader)
{
   switch (shader->info.stage) {
   case MESA_SHADER_VERTEX:
      if (shader->info.vs.as_ls)
         return RGP_HW_STAGE_LS;
      else if (shader->info.vs.as_es)
         return RGP_HW_STAGE_ES;
      else if (shader->info.is_ngg)
         return RGP_HW_STAGE_GS;
      else
         return RGP_HW_STAGE_VS;
   case MESA_SHADER_TESS_CTRL:
      return RGP_HW_STAGE_HS;
   case MESA_SHADER_TESS_EVAL:
      if (shader->info.tes.as_es)
         return RGP_HW_STAGE_ES;
      else if (shader->info.is_ngg)
         return RGP_HW_STAGE_GS;
      else
         return RGP_HW_STAGE_VS;
   case MESA_SHADER_MESH:
   case MESA_SHADER_GEOMETRY:
      return RGP_HW_STAGE_GS;
   case MESA_SHADER_FRAGMENT:
      return RGP_HW_STAGE_PS;
   case MESA_SHADER_TASK:
   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_RAYGEN:
   case MESA_SHADER_CLOSEST_HIT:
   case MESA_SHADER_ANY_HIT:
   case MESA_SHADER_INTERSECTION:
   case MESA_SHADER_MISS:
   case MESA_SHADER_CALLABLE:
      return RGP_HW_STAGE_CS;
   default:
      unreachable("invalid mesa shader stage");
   }
}

static void
radv_fill_code_object_record(struct radv_device *device, struct rgp_shader_data *shader_data,
                             struct radv_shader *shader, uint64_t va)
{
   struct radv_physical_device *pdevice = device->physical_device;
   unsigned lds_increment = pdevice->rad_info.gfx_level >= GFX11 && shader->info.stage == MESA_SHADER_FRAGMENT
                               ? 1024
                               : pdevice->rad_info.lds_encode_granularity;

   memset(shader_data->rt_shader_name, 0, sizeof(shader_data->rt_shader_name));
   shader_data->hash[0] = (uint64_t)(uintptr_t)shader;
   shader_data->hash[1] = (uint64_t)(uintptr_t)shader >> 32;
   shader_data->code_size = shader->code_size;
   shader_data->code = shader->code;
   shader_data->vgpr_count = shader->config.num_vgprs;
   shader_data->sgpr_count = shader->config.num_sgprs;
   shader_data->scratch_memory_size = shader->config.scratch_bytes_per_wave;
   shader_data->lds_size = shader->config.lds_size * lds_increment;
   shader_data->wavefront_size = shader->info.wave_size;
   shader_data->base_address = va & 0xffffffffffff;
   shader_data->elf_symbol_offset = 0;
   shader_data->hw_stage = radv_get_rgp_shader_stage(shader);
   shader_data->is_combined = false;
}

static VkResult
radv_add_code_object(struct radv_device *device, struct radv_pipeline *pipeline)
{
   struct ac_sqtt *sqtt = &device->sqtt;
   struct rgp_code_object *code_object = &sqtt->rgp_code_object;
   struct rgp_code_object_record *record;

   record = malloc(sizeof(struct rgp_code_object_record));
   if (!record)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   record->shader_stages_mask = 0;
   record->num_shaders_combined = 0;
   record->pipeline_hash[0] = pipeline->pipeline_hash;
   record->pipeline_hash[1] = pipeline->pipeline_hash;
   record->is_rt = false;

   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; i++) {
      struct radv_shader *shader = pipeline->shaders[i];

      if (!shader)
         continue;

      radv_fill_code_object_record(device, &record->shader_data[i], shader, radv_sqtt_shader_get_va_reloc(pipeline, i));

      record->shader_stages_mask |= (1 << i);
      record->num_shaders_combined++;
   }

   simple_mtx_lock(&code_object->lock);
   list_addtail(&record->list, &code_object->record);
   code_object->record_count++;
   simple_mtx_unlock(&code_object->lock);

   return VK_SUCCESS;
}

static VkResult
radv_add_rt_record(struct radv_device *device, struct rgp_code_object *code_object,
                   struct radv_ray_tracing_pipeline *pipeline, struct radv_shader *shader, uint32_t stack_size,
                   uint32_t index, uint64_t hash)
{
   struct rgp_code_object_record *record = malloc(sizeof(struct rgp_code_object_record));
   if (!record)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   struct rgp_shader_data *shader_data = &record->shader_data[shader->info.stage];

   record->shader_stages_mask = 0;
   record->num_shaders_combined = 0;
   record->pipeline_hash[0] = hash;
   record->pipeline_hash[1] = hash;

   radv_fill_code_object_record(device, shader_data, shader, shader->va);
   shader_data->rt_stack_size = stack_size;

   record->shader_stages_mask |= (1 << shader->info.stage);
   record->is_rt = true;
   switch (shader->info.stage) {
   case MESA_SHADER_RAYGEN:
      snprintf(shader_data->rt_shader_name, sizeof(shader_data->rt_shader_name), "rgen_%d", index);
      break;
   case MESA_SHADER_CLOSEST_HIT:
      snprintf(shader_data->rt_shader_name, sizeof(shader_data->rt_shader_name), "chit_%d", index);
      break;
   case MESA_SHADER_MISS:
      snprintf(shader_data->rt_shader_name, sizeof(shader_data->rt_shader_name), "miss_%d", index);
      break;
   case MESA_SHADER_INTERSECTION:
      snprintf(shader_data->rt_shader_name, sizeof(shader_data->rt_shader_name), "traversal");
      break;
   case MESA_SHADER_CALLABLE:
      snprintf(shader_data->rt_shader_name, sizeof(shader_data->rt_shader_name), "call_%d", index);
      break;
   case MESA_SHADER_COMPUTE:
      snprintf(shader_data->rt_shader_name, sizeof(shader_data->rt_shader_name), "_amdgpu_cs_main");
      break;
   default:
      unreachable("invalid rt stage");
   }
   record->num_shaders_combined = 1;

   simple_mtx_lock(&code_object->lock);
   list_addtail(&record->list, &code_object->record);
   code_object->record_count++;
   simple_mtx_unlock(&code_object->lock);

   return VK_SUCCESS;
}

static void
compute_unique_rt_sha(uint64_t pipeline_hash, unsigned index, unsigned char sha1[SHA1_DIGEST_LENGTH])
{
   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);
   _mesa_sha1_update(&ctx, &pipeline_hash, sizeof(pipeline_hash));
   _mesa_sha1_update(&ctx, &index, sizeof(index));
   _mesa_sha1_final(&ctx, sha1);
}

static VkResult
radv_register_rt_stage(struct radv_device *device, struct radv_ray_tracing_pipeline *pipeline, uint32_t index,
                       uint32_t stack_size, struct radv_shader *shader)
{
   unsigned char sha1[SHA1_DIGEST_LENGTH];
   VkResult result;

   compute_unique_rt_sha(pipeline->base.base.pipeline_hash, index, sha1);

   result = ac_sqtt_add_pso_correlation(&device->sqtt, *(uint64_t *)sha1, pipeline->base.base.pipeline_hash);
   if (!result)
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   result = ac_sqtt_add_code_object_loader_event(&device->sqtt, *(uint64_t *)sha1, shader->va);
   if (!result)
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   result =
      radv_add_rt_record(device, &device->sqtt.rgp_code_object, pipeline, shader, stack_size, index, *(uint64_t *)sha1);
   return result;
}

static VkResult
radv_register_rt_pipeline(struct radv_device *device, struct radv_ray_tracing_pipeline *pipeline)
{
   VkResult result = VK_SUCCESS;

   uint32_t max_any_hit_stack_size = 0;
   uint32_t max_intersection_stack_size = 0;

   for (unsigned i = 0; i < pipeline->stage_count; i++) {
      struct radv_ray_tracing_stage *stage = &pipeline->stages[i];
      if (stage->stage == MESA_SHADER_ANY_HIT)
         max_any_hit_stack_size = MAX2(max_any_hit_stack_size, stage->stack_size);
      else if (stage->stage == MESA_SHADER_INTERSECTION)
         max_intersection_stack_size = MAX2(max_intersection_stack_size, stage->stack_size);

      if (!pipeline->stages[i].shader)
         continue;

      result = radv_register_rt_stage(device, pipeline, i, stage->stack_size, stage->shader);
      if (result != VK_SUCCESS)
         return result;
   }

   uint32_t idx = pipeline->stage_count;

   /* Combined traversal shader */
   if (pipeline->base.base.shaders[MESA_SHADER_INTERSECTION]) {
      result = radv_register_rt_stage(device, pipeline, idx++, max_any_hit_stack_size + max_intersection_stack_size,
                                      pipeline->base.base.shaders[MESA_SHADER_INTERSECTION]);
      if (result != VK_SUCCESS)
         return result;
   }

   /* Prolog */
   result = radv_register_rt_stage(device, pipeline, idx++, 0, pipeline->prolog);

   return result;
}

static VkResult
radv_register_pipeline(struct radv_device *device, struct radv_pipeline *pipeline)
{
   bool result;
   uint64_t base_va = ~0;

   result = ac_sqtt_add_pso_correlation(&device->sqtt, pipeline->pipeline_hash, pipeline->pipeline_hash);
   if (!result)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   /* Find the lowest shader BO VA. */
   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; i++) {
      struct radv_shader *shader = pipeline->shaders[i];
      uint64_t va;

      if (!shader)
         continue;

      va = radv_sqtt_shader_get_va_reloc(pipeline, i);
      base_va = MIN2(base_va, va);
   }

   result = ac_sqtt_add_code_object_loader_event(&device->sqtt, pipeline->pipeline_hash, base_va);
   if (!result)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   result = radv_add_code_object(device, pipeline);
   if (result != VK_SUCCESS)
      return result;

   return VK_SUCCESS;
}

static void
radv_unregister_records(struct radv_device *device, uint64_t hash)
{
   struct ac_sqtt *sqtt = &device->sqtt;
   struct rgp_pso_correlation *pso_correlation = &sqtt->rgp_pso_correlation;
   struct rgp_loader_events *loader_events = &sqtt->rgp_loader_events;
   struct rgp_code_object *code_object = &sqtt->rgp_code_object;

   /* Destroy the PSO correlation record. */
   simple_mtx_lock(&pso_correlation->lock);
   list_for_each_entry_safe (struct rgp_pso_correlation_record, record, &pso_correlation->record, list) {
      if (record->pipeline_hash[0] == hash) {
         pso_correlation->record_count--;
         list_del(&record->list);
         free(record);
         break;
      }
   }
   simple_mtx_unlock(&pso_correlation->lock);

   /* Destroy the code object loader record. */
   simple_mtx_lock(&loader_events->lock);
   list_for_each_entry_safe (struct rgp_loader_events_record, record, &loader_events->record, list) {
      if (record->code_object_hash[0] == hash) {
         loader_events->record_count--;
         list_del(&record->list);
         free(record);
         break;
      }
   }
   simple_mtx_unlock(&loader_events->lock);

   /* Destroy the code object record. */
   simple_mtx_lock(&code_object->lock);
   list_for_each_entry_safe (struct rgp_code_object_record, record, &code_object->record, list) {
      if (record->pipeline_hash[0] == hash) {
         code_object->record_count--;
         list_del(&record->list);
         free(record);
         break;
      }
   }
   simple_mtx_unlock(&code_object->lock);
}

VKAPI_ATTR VkResult VKAPI_CALL
sqtt_CreateGraphicsPipelines(VkDevice _device, VkPipelineCache pipelineCache, uint32_t count,
                             const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                             VkPipeline *pPipelines)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   VkResult result;

   result = device->layer_dispatch.rgp.CreateGraphicsPipelines(_device, pipelineCache, count, pCreateInfos, pAllocator,
                                                               pPipelines);
   if (result != VK_SUCCESS)
      return result;

   for (unsigned i = 0; i < count; i++) {
      RADV_FROM_HANDLE(radv_pipeline, pipeline, pPipelines[i]);

      if (!pipeline)
         continue;

      const VkPipelineCreateFlagBits2KHR create_flags = vk_graphics_pipeline_create_flags(&pCreateInfos[i]);
      if (create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR)
         continue;

      result = radv_sqtt_reloc_graphics_shaders(device, radv_pipeline_to_graphics(pipeline));
      if (result != VK_SUCCESS)
         goto fail;

      result = radv_register_pipeline(device, pipeline);
      if (result != VK_SUCCESS)
         goto fail;
   }

   return VK_SUCCESS;

fail:
   for (unsigned i = 0; i < count; i++) {
      sqtt_DestroyPipeline(_device, pPipelines[i], pAllocator);
      pPipelines[i] = VK_NULL_HANDLE;
   }
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
sqtt_CreateComputePipelines(VkDevice _device, VkPipelineCache pipelineCache, uint32_t count,
                            const VkComputePipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                            VkPipeline *pPipelines)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   VkResult result;

   result = device->layer_dispatch.rgp.CreateComputePipelines(_device, pipelineCache, count, pCreateInfos, pAllocator,
                                                              pPipelines);
   if (result != VK_SUCCESS)
      return result;

   for (unsigned i = 0; i < count; i++) {
      RADV_FROM_HANDLE(radv_pipeline, pipeline, pPipelines[i]);

      if (!pipeline)
         continue;

      result = radv_register_pipeline(device, pipeline);
      if (result != VK_SUCCESS)
         goto fail;
   }

   return VK_SUCCESS;

fail:
   for (unsigned i = 0; i < count; i++) {
      sqtt_DestroyPipeline(_device, pPipelines[i], pAllocator);
      pPipelines[i] = VK_NULL_HANDLE;
   }
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
sqtt_CreateRayTracingPipelinesKHR(VkDevice _device, VkDeferredOperationKHR deferredOperation,
                                  VkPipelineCache pipelineCache, uint32_t count,
                                  const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   VkResult result;

   result = device->layer_dispatch.rgp.CreateRayTracingPipelinesKHR(_device, deferredOperation, pipelineCache, count,
                                                                    pCreateInfos, pAllocator, pPipelines);
   if (result != VK_SUCCESS)
      return result;

   for (unsigned i = 0; i < count; i++) {
      RADV_FROM_HANDLE(radv_pipeline, pipeline, pPipelines[i]);

      if (!pipeline)
         continue;

      const VkPipelineCreateFlagBits2KHR create_flags = vk_rt_pipeline_create_flags(&pCreateInfos[i]);
      if (create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR)
         continue;

      result = radv_register_rt_pipeline(device, radv_pipeline_to_ray_tracing(pipeline));
      if (result != VK_SUCCESS)
         goto fail;
   }

   return VK_SUCCESS;

fail:
   for (unsigned i = 0; i < count; i++) {
      sqtt_DestroyPipeline(_device, pPipelines[i], pAllocator);
      pPipelines[i] = VK_NULL_HANDLE;
   }
   return result;
}

VKAPI_ATTR void VKAPI_CALL
sqtt_DestroyPipeline(VkDevice _device, VkPipeline _pipeline, const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_pipeline, pipeline, _pipeline);

   if (!_pipeline)
      return;

   /* Ray tracing pipelines have multiple records, each with their own hash */
   if (pipeline->type == RADV_PIPELINE_RAY_TRACING) {
      /* We have one record for each stage, plus one for the traversal shader and one for the prolog */
      uint32_t record_count = radv_pipeline_to_ray_tracing(pipeline)->stage_count + 2;
      unsigned char sha1[SHA1_DIGEST_LENGTH];
      for (uint32_t i = 0; i < record_count; ++i) {
         compute_unique_rt_sha(pipeline->pipeline_hash, i, sha1);
         radv_unregister_records(device, *(uint64_t *)sha1);
      }
   } else
      radv_unregister_records(device, pipeline->pipeline_hash);

   if (pipeline->type == RADV_PIPELINE_GRAPHICS) {
      struct radv_graphics_pipeline *graphics_pipeline = radv_pipeline_to_graphics(pipeline);
      struct radv_sqtt_shaders_reloc *reloc = graphics_pipeline->sqtt_shaders_reloc;

      radv_free_shader_memory(device, reloc->alloc);
      free(reloc);
   }

   device->layer_dispatch.rgp.DestroyPipeline(_device, _pipeline, pAllocator);
}

#undef API_MARKER
