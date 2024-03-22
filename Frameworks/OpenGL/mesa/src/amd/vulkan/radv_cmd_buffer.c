/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
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

#include "meta/radv_meta.h"
#include "radv_cs.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "radv_radeon_winsys.h"
#include "radv_shader.h"
#include "sid.h"
#include "vk_common_entrypoints.h"
#include "vk_enum_defines.h"
#include "vk_format.h"
#include "vk_framebuffer.h"
#include "vk_render_pass.h"
#include "vk_util.h"

#include "ac_debug.h"
#include "ac_shader_args.h"

#include "aco_interface.h"

#include "util/fast_idiv_by_const.h"

enum {
   RADV_PREFETCH_VBO_DESCRIPTORS = (1 << 0),
   RADV_PREFETCH_VS = (1 << 1),
   RADV_PREFETCH_TCS = (1 << 2),
   RADV_PREFETCH_TES = (1 << 3),
   RADV_PREFETCH_GS = (1 << 4),
   RADV_PREFETCH_PS = (1 << 5),
   RADV_PREFETCH_MS = (1 << 6),
   RADV_PREFETCH_SHADERS = (RADV_PREFETCH_VS | RADV_PREFETCH_TCS | RADV_PREFETCH_TES | RADV_PREFETCH_GS |
                            RADV_PREFETCH_PS | RADV_PREFETCH_MS)
};

static void radv_handle_image_transition(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                                         VkImageLayout src_layout, VkImageLayout dst_layout, uint32_t src_family_index,
                                         uint32_t dst_family_index, const VkImageSubresourceRange *range,
                                         struct radv_sample_locations_state *sample_locs);

static void
radv_bind_dynamic_state(struct radv_cmd_buffer *cmd_buffer, const struct radv_dynamic_state *src)
{
   struct radv_dynamic_state *dest = &cmd_buffer->state.dynamic;
   uint64_t copy_mask = src->mask;
   uint64_t dest_mask = 0;

   dest->vk.dr.rectangle_count = src->vk.dr.rectangle_count;
   dest->sample_location.count = src->sample_location.count;

   if (copy_mask & RADV_DYNAMIC_VIEWPORT) {
      if (dest->vk.vp.viewport_count != src->vk.vp.viewport_count) {
         dest->vk.vp.viewport_count = src->vk.vp.viewport_count;
         dest_mask |= RADV_DYNAMIC_VIEWPORT;
      }

      if (memcmp(&dest->vk.vp.viewports, &src->vk.vp.viewports, src->vk.vp.viewport_count * sizeof(VkViewport))) {
         typed_memcpy(dest->vk.vp.viewports, src->vk.vp.viewports, src->vk.vp.viewport_count);
         typed_memcpy(dest->hw_vp.xform, src->hw_vp.xform, src->vk.vp.viewport_count);
         dest_mask |= RADV_DYNAMIC_VIEWPORT;
      }
   }

   if (copy_mask & RADV_DYNAMIC_SCISSOR) {
      if (dest->vk.vp.scissor_count != src->vk.vp.scissor_count) {
         dest->vk.vp.scissor_count = src->vk.vp.scissor_count;
         dest_mask |= RADV_DYNAMIC_SCISSOR;
      }

      if (memcmp(&dest->vk.vp.scissors, &src->vk.vp.scissors, src->vk.vp.scissor_count * sizeof(VkRect2D))) {
         typed_memcpy(dest->vk.vp.scissors, src->vk.vp.scissors, src->vk.vp.scissor_count);
         dest_mask |= RADV_DYNAMIC_SCISSOR;
      }
   }

   if (copy_mask & RADV_DYNAMIC_BLEND_CONSTANTS) {
      if (memcmp(&dest->vk.cb.blend_constants, &src->vk.cb.blend_constants, sizeof(src->vk.cb.blend_constants))) {
         typed_memcpy(dest->vk.cb.blend_constants, src->vk.cb.blend_constants, 4);
         dest_mask |= RADV_DYNAMIC_BLEND_CONSTANTS;
      }
   }

   if (copy_mask & RADV_DYNAMIC_DISCARD_RECTANGLE) {
      if (memcmp(&dest->vk.dr.rectangles, &src->vk.dr.rectangles, src->vk.dr.rectangle_count * sizeof(VkRect2D))) {
         typed_memcpy(dest->vk.dr.rectangles, src->vk.dr.rectangles, src->vk.dr.rectangle_count);
         dest_mask |= RADV_DYNAMIC_DISCARD_RECTANGLE;
      }
   }

   if (copy_mask & RADV_DYNAMIC_SAMPLE_LOCATIONS) {
      if (dest->sample_location.per_pixel != src->sample_location.per_pixel ||
          dest->sample_location.grid_size.width != src->sample_location.grid_size.width ||
          dest->sample_location.grid_size.height != src->sample_location.grid_size.height ||
          memcmp(&dest->sample_location.locations, &src->sample_location.locations,
                 src->sample_location.count * sizeof(VkSampleLocationEXT))) {
         dest->sample_location.per_pixel = src->sample_location.per_pixel;
         dest->sample_location.grid_size = src->sample_location.grid_size;
         typed_memcpy(dest->sample_location.locations, src->sample_location.locations, src->sample_location.count);
         dest_mask |= RADV_DYNAMIC_SAMPLE_LOCATIONS;
      }
   }

   if (copy_mask & RADV_DYNAMIC_COLOR_WRITE_MASK) {
      for (uint32_t i = 0; i < MAX_RTS; i++) {
         if (dest->vk.cb.attachments[i].write_mask != src->vk.cb.attachments[i].write_mask) {
            dest->vk.cb.attachments[i].write_mask = src->vk.cb.attachments[i].write_mask;
            dest_mask |= RADV_DYNAMIC_COLOR_WRITE_MASK;
         }
      }
   }

   if (copy_mask & RADV_DYNAMIC_COLOR_BLEND_ENABLE) {
      for (uint32_t i = 0; i < MAX_RTS; i++) {
         if (dest->vk.cb.attachments[i].blend_enable != src->vk.cb.attachments[i].blend_enable) {
            dest->vk.cb.attachments[i].blend_enable = src->vk.cb.attachments[i].blend_enable;
            dest_mask |= RADV_DYNAMIC_COLOR_BLEND_ENABLE;
         }
      }
   }

   if (copy_mask & RADV_DYNAMIC_COLOR_BLEND_EQUATION) {
      for (uint32_t i = 0; i < MAX_RTS; i++) {
         if (dest->vk.cb.attachments[i].src_color_blend_factor != src->vk.cb.attachments[i].src_color_blend_factor ||
             dest->vk.cb.attachments[i].dst_color_blend_factor != src->vk.cb.attachments[i].dst_color_blend_factor ||
             dest->vk.cb.attachments[i].color_blend_op != src->vk.cb.attachments[i].color_blend_op ||
             dest->vk.cb.attachments[i].src_alpha_blend_factor != src->vk.cb.attachments[i].src_alpha_blend_factor ||
             dest->vk.cb.attachments[i].dst_alpha_blend_factor != src->vk.cb.attachments[i].dst_alpha_blend_factor ||
             dest->vk.cb.attachments[i].alpha_blend_op != src->vk.cb.attachments[i].alpha_blend_op) {
            dest->vk.cb.attachments[i].src_color_blend_factor = src->vk.cb.attachments[i].src_color_blend_factor;
            dest->vk.cb.attachments[i].dst_color_blend_factor = src->vk.cb.attachments[i].dst_color_blend_factor;
            dest->vk.cb.attachments[i].color_blend_op = src->vk.cb.attachments[i].color_blend_op;
            dest->vk.cb.attachments[i].src_alpha_blend_factor = src->vk.cb.attachments[i].src_alpha_blend_factor;
            dest->vk.cb.attachments[i].dst_alpha_blend_factor = src->vk.cb.attachments[i].dst_alpha_blend_factor;
            dest->vk.cb.attachments[i].alpha_blend_op = src->vk.cb.attachments[i].alpha_blend_op;
            dest_mask |= RADV_DYNAMIC_COLOR_BLEND_EQUATION;
         }
      }
   }

#define RADV_CMP_COPY(field, flag)                                                                                     \
   if (copy_mask & flag) {                                                                                             \
      if (dest->field != src->field) {                                                                                 \
         dest->field = src->field;                                                                                     \
         dest_mask |= flag;                                                                                            \
      }                                                                                                                \
   }

   RADV_CMP_COPY(vk.ia.primitive_topology, RADV_DYNAMIC_PRIMITIVE_TOPOLOGY);
   RADV_CMP_COPY(vk.ia.primitive_restart_enable, RADV_DYNAMIC_PRIMITIVE_RESTART_ENABLE);

   RADV_CMP_COPY(vk.vp.depth_clip_negative_one_to_one, RADV_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE);

   RADV_CMP_COPY(vk.ts.patch_control_points, RADV_DYNAMIC_PATCH_CONTROL_POINTS);
   RADV_CMP_COPY(vk.ts.domain_origin, RADV_DYNAMIC_TESS_DOMAIN_ORIGIN);

   RADV_CMP_COPY(vk.rs.line.width, RADV_DYNAMIC_LINE_WIDTH);
   RADV_CMP_COPY(vk.rs.depth_bias.constant, RADV_DYNAMIC_DEPTH_BIAS);
   RADV_CMP_COPY(vk.rs.depth_bias.clamp, RADV_DYNAMIC_DEPTH_BIAS);
   RADV_CMP_COPY(vk.rs.depth_bias.slope, RADV_DYNAMIC_DEPTH_BIAS);
   RADV_CMP_COPY(vk.rs.depth_bias.representation, RADV_DYNAMIC_DEPTH_BIAS);
   RADV_CMP_COPY(vk.rs.line.stipple.factor, RADV_DYNAMIC_LINE_STIPPLE);
   RADV_CMP_COPY(vk.rs.line.stipple.pattern, RADV_DYNAMIC_LINE_STIPPLE);
   RADV_CMP_COPY(vk.rs.cull_mode, RADV_DYNAMIC_CULL_MODE);
   RADV_CMP_COPY(vk.rs.front_face, RADV_DYNAMIC_FRONT_FACE);
   RADV_CMP_COPY(vk.rs.depth_bias.enable, RADV_DYNAMIC_DEPTH_BIAS_ENABLE);
   RADV_CMP_COPY(vk.rs.rasterizer_discard_enable, RADV_DYNAMIC_RASTERIZER_DISCARD_ENABLE);
   RADV_CMP_COPY(vk.rs.polygon_mode, RADV_DYNAMIC_POLYGON_MODE);
   RADV_CMP_COPY(vk.rs.line.stipple.enable, RADV_DYNAMIC_LINE_STIPPLE_ENABLE);
   RADV_CMP_COPY(vk.rs.depth_clip_enable, RADV_DYNAMIC_DEPTH_CLIP_ENABLE);
   RADV_CMP_COPY(vk.rs.conservative_mode, RADV_DYNAMIC_CONSERVATIVE_RAST_MODE);
   RADV_CMP_COPY(vk.rs.provoking_vertex, RADV_DYNAMIC_PROVOKING_VERTEX_MODE);
   RADV_CMP_COPY(vk.rs.depth_clamp_enable, RADV_DYNAMIC_DEPTH_CLAMP_ENABLE);
   RADV_CMP_COPY(vk.rs.line.mode, RADV_DYNAMIC_LINE_RASTERIZATION_MODE);

   RADV_CMP_COPY(vk.ms.alpha_to_coverage_enable, RADV_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE);
   RADV_CMP_COPY(vk.ms.sample_mask, RADV_DYNAMIC_SAMPLE_MASK);
   RADV_CMP_COPY(vk.ms.rasterization_samples, RADV_DYNAMIC_RASTERIZATION_SAMPLES);
   RADV_CMP_COPY(vk.ms.sample_locations_enable, RADV_DYNAMIC_SAMPLE_LOCATIONS_ENABLE);

   RADV_CMP_COPY(vk.ds.depth.bounds_test.min, RADV_DYNAMIC_DEPTH_BOUNDS);
   RADV_CMP_COPY(vk.ds.depth.bounds_test.max, RADV_DYNAMIC_DEPTH_BOUNDS);
   RADV_CMP_COPY(vk.ds.stencil.front.compare_mask, RADV_DYNAMIC_STENCIL_COMPARE_MASK);
   RADV_CMP_COPY(vk.ds.stencil.back.compare_mask, RADV_DYNAMIC_STENCIL_COMPARE_MASK);
   RADV_CMP_COPY(vk.ds.stencil.front.write_mask, RADV_DYNAMIC_STENCIL_WRITE_MASK);
   RADV_CMP_COPY(vk.ds.stencil.back.write_mask, RADV_DYNAMIC_STENCIL_WRITE_MASK);
   RADV_CMP_COPY(vk.ds.stencil.front.reference, RADV_DYNAMIC_STENCIL_REFERENCE);
   RADV_CMP_COPY(vk.ds.stencil.back.reference, RADV_DYNAMIC_STENCIL_REFERENCE);
   RADV_CMP_COPY(vk.ds.depth.test_enable, RADV_DYNAMIC_DEPTH_TEST_ENABLE);
   RADV_CMP_COPY(vk.ds.depth.write_enable, RADV_DYNAMIC_DEPTH_WRITE_ENABLE);
   RADV_CMP_COPY(vk.ds.depth.compare_op, RADV_DYNAMIC_DEPTH_COMPARE_OP);
   RADV_CMP_COPY(vk.ds.depth.bounds_test.enable, RADV_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE);
   RADV_CMP_COPY(vk.ds.stencil.test_enable, RADV_DYNAMIC_STENCIL_TEST_ENABLE);
   RADV_CMP_COPY(vk.ds.stencil.front.op.fail, RADV_DYNAMIC_STENCIL_OP);
   RADV_CMP_COPY(vk.ds.stencil.front.op.pass, RADV_DYNAMIC_STENCIL_OP);
   RADV_CMP_COPY(vk.ds.stencil.front.op.depth_fail, RADV_DYNAMIC_STENCIL_OP);
   RADV_CMP_COPY(vk.ds.stencil.front.op.compare, RADV_DYNAMIC_STENCIL_OP);
   RADV_CMP_COPY(vk.ds.stencil.back.op.fail, RADV_DYNAMIC_STENCIL_OP);
   RADV_CMP_COPY(vk.ds.stencil.back.op.pass, RADV_DYNAMIC_STENCIL_OP);
   RADV_CMP_COPY(vk.ds.stencil.back.op.depth_fail, RADV_DYNAMIC_STENCIL_OP);
   RADV_CMP_COPY(vk.ds.stencil.back.op.compare, RADV_DYNAMIC_STENCIL_OP);

   RADV_CMP_COPY(vk.cb.logic_op, RADV_DYNAMIC_LOGIC_OP);
   RADV_CMP_COPY(vk.cb.color_write_enables, RADV_DYNAMIC_COLOR_WRITE_ENABLE);
   RADV_CMP_COPY(vk.cb.logic_op_enable, RADV_DYNAMIC_LOGIC_OP_ENABLE);

   RADV_CMP_COPY(vk.fsr.fragment_size.width, RADV_DYNAMIC_FRAGMENT_SHADING_RATE);
   RADV_CMP_COPY(vk.fsr.fragment_size.height, RADV_DYNAMIC_FRAGMENT_SHADING_RATE);
   RADV_CMP_COPY(vk.fsr.combiner_ops[0], RADV_DYNAMIC_FRAGMENT_SHADING_RATE);
   RADV_CMP_COPY(vk.fsr.combiner_ops[1], RADV_DYNAMIC_FRAGMENT_SHADING_RATE);

   RADV_CMP_COPY(vk.dr.enable, RADV_DYNAMIC_DISCARD_RECTANGLE_ENABLE);
   RADV_CMP_COPY(vk.dr.mode, RADV_DYNAMIC_DISCARD_RECTANGLE_MODE);

   RADV_CMP_COPY(feedback_loop_aspects, RADV_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE);

#undef RADV_CMP_COPY

   cmd_buffer->state.dirty |= dest_mask;

   /* Handle driver specific states that need to be re-emitted when PSO are bound. */
   if (dest_mask & (RADV_DYNAMIC_VIEWPORT | RADV_DYNAMIC_POLYGON_MODE | RADV_DYNAMIC_LINE_WIDTH |
                    RADV_DYNAMIC_PRIMITIVE_TOPOLOGY)) {
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_GUARDBAND;
   }

   if (cmd_buffer->device->physical_device->rad_info.rbplus_allowed && (dest_mask & RADV_DYNAMIC_COLOR_WRITE_MASK)) {
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_RBPLUS;
   }
}

bool
radv_cmd_buffer_uses_mec(struct radv_cmd_buffer *cmd_buffer)
{
   return cmd_buffer->qf == RADV_QUEUE_COMPUTE && cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7;
}

enum amd_ip_type
radv_queue_family_to_ring(const struct radv_physical_device *physical_device, enum radv_queue_family f)
{
   switch (f) {
   case RADV_QUEUE_GENERAL:
      return AMD_IP_GFX;
   case RADV_QUEUE_COMPUTE:
      return AMD_IP_COMPUTE;
   case RADV_QUEUE_TRANSFER:
      return AMD_IP_SDMA;
   case RADV_QUEUE_VIDEO_DEC:
      return physical_device->vid_decode_ip;
   case RADV_QUEUE_VIDEO_ENC:
      return AMD_IP_VCN_ENC;
   default:
      unreachable("Unknown queue family");
   }
}

static void
radv_write_data(struct radv_cmd_buffer *cmd_buffer, const unsigned engine_sel, const uint64_t va, const unsigned count,
                const uint32_t *data, const bool predicating)
{
   radv_cs_write_data(cmd_buffer->device, cmd_buffer->cs, cmd_buffer->qf, engine_sel, va, count, data, predicating);
}

static void
radv_emit_clear_data(struct radv_cmd_buffer *cmd_buffer, unsigned engine_sel, uint64_t va, unsigned size)
{
   uint32_t *zeroes = alloca(size);
   memset(zeroes, 0, size);
   radv_write_data(cmd_buffer, engine_sel, va, size / 4, zeroes, false);
}

static void
radv_cmd_buffer_finish_shader_part_cache(struct radv_cmd_buffer *cmd_buffer)
{
   ralloc_free(cmd_buffer->vs_prologs.table);
   ralloc_free(cmd_buffer->ps_epilogs.table);
   ralloc_free(cmd_buffer->tcs_epilogs.table);
}

static bool
radv_cmd_buffer_init_shader_part_cache(struct radv_device *device, struct radv_cmd_buffer *cmd_buffer)
{
   if (device->vs_prologs.ops) {
      if (!_mesa_set_init(&cmd_buffer->vs_prologs, NULL, device->vs_prologs.ops->hash, device->vs_prologs.ops->equals))
         return false;
   }
   if (device->tcs_epilogs.ops) {
      if (!_mesa_set_init(&cmd_buffer->tcs_epilogs, NULL, device->tcs_epilogs.ops->hash,
                          device->tcs_epilogs.ops->equals))
         return false;
   }
   if (device->ps_epilogs.ops) {
      if (!_mesa_set_init(&cmd_buffer->ps_epilogs, NULL, device->ps_epilogs.ops->hash, device->ps_epilogs.ops->equals))
         return false;
   }
   return true;
}

static void
radv_destroy_cmd_buffer(struct vk_command_buffer *vk_cmd_buffer)
{
   struct radv_cmd_buffer *cmd_buffer = container_of(vk_cmd_buffer, struct radv_cmd_buffer, vk);

   if (cmd_buffer->qf != RADV_QUEUE_SPARSE) {
      list_for_each_entry_safe (struct radv_cmd_buffer_upload, up, &cmd_buffer->upload.list, list) {
         radv_rmv_log_command_buffer_bo_destroy(cmd_buffer->device, up->upload_bo);
         cmd_buffer->device->ws->buffer_destroy(cmd_buffer->device->ws, up->upload_bo);
         list_del(&up->list);
         free(up);
      }

      if (cmd_buffer->upload.upload_bo) {
         radv_rmv_log_command_buffer_bo_destroy(cmd_buffer->device, cmd_buffer->upload.upload_bo);
         cmd_buffer->device->ws->buffer_destroy(cmd_buffer->device->ws, cmd_buffer->upload.upload_bo);
      }

      if (cmd_buffer->cs)
         cmd_buffer->device->ws->cs_destroy(cmd_buffer->cs);
      if (cmd_buffer->gang.cs)
         cmd_buffer->device->ws->cs_destroy(cmd_buffer->gang.cs);
      if (cmd_buffer->transfer.copy_temp)
         cmd_buffer->device->ws->buffer_destroy(cmd_buffer->device->ws, cmd_buffer->transfer.copy_temp);

      radv_cmd_buffer_finish_shader_part_cache(cmd_buffer);

      for (unsigned i = 0; i < MAX_BIND_POINTS; i++) {
         struct radv_descriptor_set_header *set = &cmd_buffer->descriptors[i].push_set.set;
         free(set->mapped_ptr);
         if (set->layout)
            vk_descriptor_set_layout_unref(&cmd_buffer->device->vk, &set->layout->vk);
         vk_object_base_finish(&set->base);
      }

      vk_object_base_finish(&cmd_buffer->meta_push_descriptors.base);
   }

   vk_command_buffer_finish(&cmd_buffer->vk);
   vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer);
}

static VkResult
radv_create_cmd_buffer(struct vk_command_pool *pool, struct vk_command_buffer **cmd_buffer_out)
{
   struct radv_device *device = container_of(pool->base.device, struct radv_device, vk);

   struct radv_cmd_buffer *cmd_buffer;
   unsigned ring;
   cmd_buffer = vk_zalloc(&pool->alloc, sizeof(*cmd_buffer), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (cmd_buffer == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkResult result = vk_command_buffer_init(pool, &cmd_buffer->vk, &radv_cmd_buffer_ops, 0);
   if (result != VK_SUCCESS) {
      vk_free(&cmd_buffer->vk.pool->alloc, cmd_buffer);
      return result;
   }

   cmd_buffer->device = device;

   cmd_buffer->qf = vk_queue_to_radv(device->physical_device, pool->queue_family_index);

   if (cmd_buffer->qf != RADV_QUEUE_SPARSE) {
      list_inithead(&cmd_buffer->upload.list);

      if (!radv_cmd_buffer_init_shader_part_cache(device, cmd_buffer)) {
         radv_destroy_cmd_buffer(&cmd_buffer->vk);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      ring = radv_queue_family_to_ring(device->physical_device, cmd_buffer->qf);

      cmd_buffer->cs =
         device->ws->cs_create(device->ws, ring, cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
      if (!cmd_buffer->cs) {
         radv_destroy_cmd_buffer(&cmd_buffer->vk);
         return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      }

      vk_object_base_init(&device->vk, &cmd_buffer->meta_push_descriptors.base, VK_OBJECT_TYPE_DESCRIPTOR_SET);

      for (unsigned i = 0; i < MAX_BIND_POINTS; i++)
         vk_object_base_init(&device->vk, &cmd_buffer->descriptors[i].push_set.set.base, VK_OBJECT_TYPE_DESCRIPTOR_SET);
   }

   *cmd_buffer_out = &cmd_buffer->vk;

   return VK_SUCCESS;
}

void
radv_cmd_buffer_reset_rendering(struct radv_cmd_buffer *cmd_buffer)
{
   memset(&cmd_buffer->state.render, 0, sizeof(cmd_buffer->state.render));
}

static void
radv_reset_cmd_buffer(struct vk_command_buffer *vk_cmd_buffer, UNUSED VkCommandBufferResetFlags flags)
{
   struct radv_cmd_buffer *cmd_buffer = container_of(vk_cmd_buffer, struct radv_cmd_buffer, vk);

   vk_command_buffer_reset(&cmd_buffer->vk);

   if (cmd_buffer->qf == RADV_QUEUE_SPARSE)
      return;

   cmd_buffer->device->ws->cs_reset(cmd_buffer->cs);
   if (cmd_buffer->gang.cs)
      cmd_buffer->device->ws->cs_reset(cmd_buffer->gang.cs);

   list_for_each_entry_safe (struct radv_cmd_buffer_upload, up, &cmd_buffer->upload.list, list) {
      radv_rmv_log_command_buffer_bo_destroy(cmd_buffer->device, up->upload_bo);
      cmd_buffer->device->ws->buffer_destroy(cmd_buffer->device->ws, up->upload_bo);
      list_del(&up->list);
      free(up);
   }

   cmd_buffer->push_constant_stages = 0;
   cmd_buffer->scratch_size_per_wave_needed = 0;
   cmd_buffer->scratch_waves_wanted = 0;
   cmd_buffer->compute_scratch_size_per_wave_needed = 0;
   cmd_buffer->compute_scratch_waves_wanted = 0;
   cmd_buffer->esgs_ring_size_needed = 0;
   cmd_buffer->gsvs_ring_size_needed = 0;
   cmd_buffer->tess_rings_needed = false;
   cmd_buffer->task_rings_needed = false;
   cmd_buffer->mesh_scratch_ring_needed = false;
   cmd_buffer->gds_needed = false;
   cmd_buffer->gds_oa_needed = false;
   cmd_buffer->sample_positions_needed = false;
   cmd_buffer->gang.sem.leader_value = 0;
   cmd_buffer->gang.sem.emitted_leader_value = 0;
   cmd_buffer->gang.sem.va = 0;
   cmd_buffer->shader_upload_seq = 0;

   if (cmd_buffer->upload.upload_bo)
      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, cmd_buffer->upload.upload_bo);
   cmd_buffer->upload.offset = 0;

   memset(cmd_buffer->vertex_binding_buffers, 0, sizeof(struct radv_buffer *) * cmd_buffer->used_vertex_bindings);
   cmd_buffer->used_vertex_bindings = 0;

   for (unsigned i = 0; i < MAX_BIND_POINTS; i++) {
      cmd_buffer->descriptors[i].dirty = 0;
      cmd_buffer->descriptors[i].valid = 0;
   }

   radv_cmd_buffer_reset_rendering(cmd_buffer);
}

const struct vk_command_buffer_ops radv_cmd_buffer_ops = {
   .create = radv_create_cmd_buffer,
   .reset = radv_reset_cmd_buffer,
   .destroy = radv_destroy_cmd_buffer,
};

static bool
radv_cmd_buffer_resize_upload_buf(struct radv_cmd_buffer *cmd_buffer, uint64_t min_needed)
{
   uint64_t new_size;
   struct radeon_winsys_bo *bo = NULL;
   struct radv_cmd_buffer_upload *upload;
   struct radv_device *device = cmd_buffer->device;

   new_size = MAX2(min_needed, 16 * 1024);
   new_size = MAX2(new_size, 2 * cmd_buffer->upload.size);

   VkResult result = device->ws->buffer_create(
      device->ws, new_size, 4096, device->ws->cs_domain(device->ws),
      RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING | RADEON_FLAG_32BIT | RADEON_FLAG_GTT_WC,
      RADV_BO_PRIORITY_UPLOAD_BUFFER, 0, &bo);

   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd_buffer->vk, result);
      return false;
   }

   radv_cs_add_buffer(device->ws, cmd_buffer->cs, bo);
   if (cmd_buffer->upload.upload_bo) {
      upload = malloc(sizeof(*upload));

      if (!upload) {
         vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
         device->ws->buffer_destroy(device->ws, bo);
         return false;
      }

      memcpy(upload, &cmd_buffer->upload, sizeof(*upload));
      list_add(&upload->list, &cmd_buffer->upload.list);
   }

   cmd_buffer->upload.upload_bo = bo;
   cmd_buffer->upload.size = new_size;
   cmd_buffer->upload.offset = 0;
   cmd_buffer->upload.map = device->ws->buffer_map(cmd_buffer->upload.upload_bo);

   if (!cmd_buffer->upload.map) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      return false;
   }
   radv_rmv_log_command_buffer_bo_create(device, cmd_buffer->upload.upload_bo, 0, cmd_buffer->upload.size, 0);

   return true;
}

bool
radv_cmd_buffer_upload_alloc_aligned(struct radv_cmd_buffer *cmd_buffer, unsigned size, unsigned alignment,
                                     unsigned *out_offset, void **ptr)
{
   assert(size % 4 == 0);

   const struct radeon_info *rad_info = &cmd_buffer->device->physical_device->rad_info;

   /* Align to the scalar cache line size if it results in this allocation
    * being placed in less of them.
    */
   unsigned offset = cmd_buffer->upload.offset;
   unsigned line_size = rad_info->gfx_level >= GFX10 ? 64 : 32;
   unsigned gap = align(offset, line_size) - offset;
   if ((size & (line_size - 1)) > gap)
      offset = align(offset, line_size);

   if (alignment)
      offset = align(offset, alignment);
   if (offset + size > cmd_buffer->upload.size) {
      if (!radv_cmd_buffer_resize_upload_buf(cmd_buffer, size))
         return false;
      offset = 0;
   }

   *out_offset = offset;
   *ptr = cmd_buffer->upload.map + offset;

   cmd_buffer->upload.offset = offset + size;
   return true;
}

bool
radv_cmd_buffer_upload_alloc(struct radv_cmd_buffer *cmd_buffer, unsigned size, unsigned *out_offset, void **ptr)
{
   return radv_cmd_buffer_upload_alloc_aligned(cmd_buffer, size, 0, out_offset, ptr);
}

bool
radv_cmd_buffer_upload_data(struct radv_cmd_buffer *cmd_buffer, unsigned size, const void *data, unsigned *out_offset)
{
   uint8_t *ptr;

   if (!radv_cmd_buffer_upload_alloc(cmd_buffer, size, out_offset, (void **)&ptr))
      return false;
   assert(ptr);

   memcpy(ptr, data, size);
   return true;
}

void
radv_cmd_buffer_trace_emit(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_device *device = cmd_buffer->device;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint64_t va;

   if (cmd_buffer->qf != RADV_QUEUE_GENERAL && cmd_buffer->qf != RADV_QUEUE_COMPUTE)
      return;

   va = radv_buffer_get_va(device->trace_bo);
   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
      va += 4;

   ++cmd_buffer->state.trace_id;
   radv_write_data(cmd_buffer, V_370_ME, va, 1, &cmd_buffer->state.trace_id, false);

   radeon_check_space(cmd_buffer->device->ws, cs, 2);

   radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
   radeon_emit(cs, AC_ENCODE_TRACE_POINT(cmd_buffer->state.trace_id));
}

static void
radv_gang_barrier(struct radv_cmd_buffer *cmd_buffer, VkPipelineStageFlags2 src_stage_mask,
                  VkPipelineStageFlags2 dst_stage_mask)
{
   /* Update flush bits from the main cmdbuf, except the stage flush. */
   cmd_buffer->gang.flush_bits |=
      cmd_buffer->state.flush_bits & RADV_CMD_FLUSH_ALL_COMPUTE & ~RADV_CMD_FLAG_CS_PARTIAL_FLUSH;

   /* Add stage flush only when necessary. */
   if (src_stage_mask & (VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT |
                         VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT))
      cmd_buffer->gang.flush_bits |= RADV_CMD_FLAG_CS_PARTIAL_FLUSH;

   /* Block task shaders when we have to wait for CP DMA on the GFX cmdbuf. */
   if (src_stage_mask &
       (VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_CLEAR_BIT | VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT |
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT))
      dst_stage_mask |= cmd_buffer->state.dma_is_busy ? VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT : 0;

   /* Increment the GFX/ACE semaphore when task shaders are blocked. */
   if (dst_stage_mask & (VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT |
                         VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT))
      cmd_buffer->gang.sem.leader_value++;
}

void
radv_gang_cache_flush(struct radv_cmd_buffer *cmd_buffer)
{
   struct radeon_cmdbuf *ace_cs = cmd_buffer->gang.cs;
   const uint32_t flush_bits = cmd_buffer->gang.flush_bits;
   enum rgp_flush_bits sqtt_flush_bits = 0;

   radv_cs_emit_cache_flush(cmd_buffer->device->ws, ace_cs, cmd_buffer->device->physical_device->rad_info.gfx_level,
                            NULL, 0, RADV_QUEUE_COMPUTE, flush_bits, &sqtt_flush_bits, 0);

   cmd_buffer->gang.flush_bits = 0;
}

static bool
radv_gang_sem_init(struct radv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->gang.sem.va)
      return true;

   /* DWORD 0: GFX->ACE semaphore (GFX blocks ACE, ie. ACE waits for GFX)
    * DWORD 1: ACE->GFX semaphore
    */
   uint64_t sem_init = 0;
   uint32_t va_off = 0;
   if (!radv_cmd_buffer_upload_data(cmd_buffer, sizeof(uint64_t), &sem_init, &va_off)) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
      return false;
   }

   cmd_buffer->gang.sem.va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + va_off;
   return true;
}

static bool
radv_gang_leader_sem_dirty(const struct radv_cmd_buffer *cmd_buffer)
{
   return cmd_buffer->gang.sem.leader_value != cmd_buffer->gang.sem.emitted_leader_value;
}

static bool
radv_gang_follower_sem_dirty(const struct radv_cmd_buffer *cmd_buffer)
{
   return cmd_buffer->gang.sem.follower_value != cmd_buffer->gang.sem.emitted_follower_value;
}

ALWAYS_INLINE static bool
radv_flush_gang_semaphore(struct radv_cmd_buffer *cmd_buffer, struct radeon_cmdbuf *cs, const enum radv_queue_family qf,
                          const uint32_t va_off, const uint32_t value)
{
   if (!radv_gang_sem_init(cmd_buffer))
      return false;

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cs, 12);

   radv_cs_emit_write_event_eop(cs, cmd_buffer->device->physical_device->rad_info.gfx_level, qf,
                                V_028A90_BOTTOM_OF_PIPE_TS, 0, EOP_DST_SEL_MEM, EOP_DATA_SEL_VALUE_32BIT,
                                cmd_buffer->gang.sem.va + va_off, value, cmd_buffer->gfx9_eop_bug_va);

   assert(cmd_buffer->cs->cdw <= cdw_max);
   return true;
}

ALWAYS_INLINE static bool
radv_flush_gang_leader_semaphore(struct radv_cmd_buffer *cmd_buffer)
{
   if (!radv_gang_leader_sem_dirty(cmd_buffer))
      return false;

   /* Gang leader writes a value to the semaphore which the follower can wait for. */
   cmd_buffer->gang.sem.emitted_leader_value = cmd_buffer->gang.sem.leader_value;
   return radv_flush_gang_semaphore(cmd_buffer, cmd_buffer->cs, cmd_buffer->qf, 0, cmd_buffer->gang.sem.leader_value);
}

ALWAYS_INLINE static bool
radv_flush_gang_follower_semaphore(struct radv_cmd_buffer *cmd_buffer)
{
   if (!radv_gang_follower_sem_dirty(cmd_buffer))
      return false;

   /* Follower writes a value to the semaphore which the gang leader can wait for. */
   cmd_buffer->gang.sem.emitted_follower_value = cmd_buffer->gang.sem.follower_value;
   return radv_flush_gang_semaphore(cmd_buffer, cmd_buffer->gang.cs, RADV_QUEUE_COMPUTE, 4,
                                    cmd_buffer->gang.sem.follower_value);
}

ALWAYS_INLINE static void
radv_wait_gang_semaphore(struct radv_cmd_buffer *cmd_buffer, struct radeon_cmdbuf *cs, const enum radv_queue_family qf,
                         const uint32_t va_off, const uint32_t value)
{
   assert(cmd_buffer->gang.sem.va);
   radeon_check_space(cmd_buffer->device->ws, cs, 7);
   radv_cp_wait_mem(cs, qf, WAIT_REG_MEM_GREATER_OR_EQUAL, cmd_buffer->gang.sem.va + va_off, value, 0xffffffff);
}

ALWAYS_INLINE static void
radv_wait_gang_leader(struct radv_cmd_buffer *cmd_buffer)
{
   /* Follower waits for the semaphore which the gang leader wrote. */
   radv_wait_gang_semaphore(cmd_buffer, cmd_buffer->gang.cs, RADV_QUEUE_COMPUTE, 0, cmd_buffer->gang.sem.leader_value);
}

ALWAYS_INLINE static void
radv_wait_gang_follower(struct radv_cmd_buffer *cmd_buffer)
{
   /* Gang leader waits for the semaphore which the follower wrote. */
   radv_wait_gang_semaphore(cmd_buffer, cmd_buffer->cs, cmd_buffer->qf, 4, cmd_buffer->gang.sem.follower_value);
}

bool
radv_gang_init(struct radv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->gang.cs)
      return true;

   struct radv_device *device = cmd_buffer->device;
   struct radeon_cmdbuf *ace_cs =
      device->ws->cs_create(device->ws, AMD_IP_COMPUTE, cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);

   if (!ace_cs) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      return false;
   }

   cmd_buffer->gang.cs = ace_cs;
   return true;
}

static VkResult
radv_gang_finalize(struct radv_cmd_buffer *cmd_buffer)
{
   assert(cmd_buffer->gang.cs);
   struct radv_device *device = cmd_buffer->device;
   struct radeon_cmdbuf *ace_cs = cmd_buffer->gang.cs;

   /* Emit pending cache flush. */
   radv_gang_cache_flush(cmd_buffer);

   /* Clear the leader<->follower semaphores if they exist.
    * This is necessary in case the same cmd buffer is submitted again in the future.
    */
   if (cmd_buffer->gang.sem.va) {
      uint64_t leader2follower_va = cmd_buffer->gang.sem.va;
      uint64_t follower2leader_va = cmd_buffer->gang.sem.va + 4;
      const uint32_t zero = 0;

      /* Follower: write 0 to the leader->follower semaphore. */
      radv_cs_write_data(device, ace_cs, RADV_QUEUE_COMPUTE, V_370_ME, leader2follower_va, 1, &zero, false);

      /* Leader: write 0 to the follower->leader semaphore. */
      radv_write_data(cmd_buffer, V_370_ME, follower2leader_va, 1, &zero, false);
   }

   return device->ws->cs_finalize(ace_cs);
}

static void
radv_cmd_buffer_after_draw(struct radv_cmd_buffer *cmd_buffer, enum radv_cmd_flush_bits flags, bool dgc)
{
   const struct radv_device *device = cmd_buffer->device;
   if (unlikely(device->sqtt.bo) && !dgc) {
      radeon_check_space(device->ws, cmd_buffer->cs, 2);

      radeon_emit(cmd_buffer->cs, PKT3(PKT3_EVENT_WRITE, 0, cmd_buffer->state.predicating));
      radeon_emit(cmd_buffer->cs, EVENT_TYPE(V_028A90_THREAD_TRACE_MARKER) | EVENT_INDEX(0));
   }

   if (device->instance->debug_flags & RADV_DEBUG_SYNC_SHADERS) {
      enum rgp_flush_bits sqtt_flush_bits = 0;
      assert(flags & (RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_CS_PARTIAL_FLUSH));

      /* Force wait for graphics or compute engines to be idle. */
      radv_cs_emit_cache_flush(device->ws, cmd_buffer->cs, device->physical_device->rad_info.gfx_level,
                               &cmd_buffer->gfx9_fence_idx, cmd_buffer->gfx9_fence_va, cmd_buffer->qf, flags,
                               &sqtt_flush_bits, cmd_buffer->gfx9_eop_bug_va);

      if ((flags & RADV_CMD_FLAG_PS_PARTIAL_FLUSH) && radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TASK)) {
         /* Force wait for compute engines to be idle on the internal cmdbuf. */
         radv_cs_emit_cache_flush(device->ws, cmd_buffer->gang.cs, device->physical_device->rad_info.gfx_level, NULL, 0,
                                  RADV_QUEUE_COMPUTE, RADV_CMD_FLAG_CS_PARTIAL_FLUSH, &sqtt_flush_bits, 0);
      }
   }

   if (unlikely(device->trace_bo))
      radv_cmd_buffer_trace_emit(cmd_buffer);
}

static void
radv_save_pipeline(struct radv_cmd_buffer *cmd_buffer, struct radv_pipeline *pipeline)
{
   struct radv_device *device = cmd_buffer->device;
   enum amd_ip_type ring;
   uint32_t data[2];
   uint64_t va;

   va = radv_buffer_get_va(device->trace_bo);

   ring = radv_queue_family_to_ring(device->physical_device, cmd_buffer->qf);

   switch (ring) {
   case AMD_IP_GFX:
      va += 8;
      break;
   case AMD_IP_COMPUTE:
      va += 16;
      break;
   default:
      assert(!"invalid IP type");
   }

   uint64_t pipeline_address = (uintptr_t)pipeline;
   data[0] = pipeline_address;
   data[1] = pipeline_address >> 32;

   radv_write_data(cmd_buffer, V_370_ME, va, 2, data, false);
}

static void
radv_save_vertex_descriptors(struct radv_cmd_buffer *cmd_buffer, uint64_t vb_ptr)
{
   struct radv_device *device = cmd_buffer->device;
   uint32_t data[2];
   uint64_t va;

   va = radv_buffer_get_va(device->trace_bo);
   va += 24;

   data[0] = vb_ptr;
   data[1] = vb_ptr >> 32;

   radv_write_data(cmd_buffer, V_370_ME, va, 2, data, false);
}

static void
radv_save_vs_prolog(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader_part *prolog)
{
   struct radv_device *device = cmd_buffer->device;
   uint32_t data[2];
   uint64_t va;

   va = radv_buffer_get_va(device->trace_bo);
   va += 32;

   uint64_t prolog_address = (uintptr_t)prolog;
   data[0] = prolog_address;
   data[1] = prolog_address >> 32;

   radv_write_data(cmd_buffer, V_370_ME, va, 2, data, false);
}

void
radv_set_descriptor_set(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point,
                        struct radv_descriptor_set *set, unsigned idx)
{
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);

   descriptors_state->sets[idx] = set;

   descriptors_state->valid |= (1u << idx); /* active descriptors */
   descriptors_state->dirty |= (1u << idx);
}

static void
radv_save_descriptors(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point)
{
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);
   struct radv_device *device = cmd_buffer->device;
   uint32_t data[MAX_SETS * 2] = {0};
   uint64_t va;
   va = radv_buffer_get_va(device->trace_bo) + 40;

   u_foreach_bit (i, descriptors_state->valid) {
      struct radv_descriptor_set *set = descriptors_state->sets[i];
      data[i * 2] = (uint64_t)(uintptr_t)set;
      data[i * 2 + 1] = (uint64_t)(uintptr_t)set >> 32;
   }

   radv_write_data(cmd_buffer, V_370_ME, va, MAX_SETS * 2, data, false);
}

const struct radv_userdata_info *
radv_get_user_sgpr(const struct radv_shader *shader, int idx)
{
   return &shader->info.user_sgprs_locs.shader_data[idx];
}

static void
radv_emit_userdata_address(struct radv_device *device, struct radeon_cmdbuf *cs, struct radv_shader *shader,
                           uint32_t base_reg, int idx, uint64_t va)
{
   const struct radv_userdata_info *loc = &shader->info.user_sgprs_locs.shader_data[idx];

   if (loc->sgpr_idx == -1)
      return;

   assert(loc->num_sgprs == 1);

   radv_emit_shader_pointer(device, cs, base_reg + loc->sgpr_idx * 4, va, false);
}

static uint64_t
radv_descriptor_get_va(const struct radv_descriptor_state *descriptors_state, unsigned set_idx)
{
   struct radv_descriptor_set *set = descriptors_state->sets[set_idx];
   uint64_t va;

   if (set) {
      va = set->header.va;
   } else {
      va = descriptors_state->descriptor_buffers[set_idx];
   }

   return va;
}

static void
radv_emit_descriptor_pointers(struct radv_device *device, struct radeon_cmdbuf *cs, struct radv_shader *shader,
                              uint32_t sh_base, struct radv_descriptor_state *descriptors_state)
{
   struct radv_userdata_locations *locs = &shader->info.user_sgprs_locs;
   unsigned mask = locs->descriptor_sets_enabled;

   mask &= descriptors_state->dirty & descriptors_state->valid;

   while (mask) {
      int start, count;

      u_bit_scan_consecutive_range(&mask, &start, &count);

      struct radv_userdata_info *loc = &locs->descriptor_sets[start];
      unsigned sh_offset = sh_base + loc->sgpr_idx * 4;

      radv_emit_shader_pointer_head(cs, sh_offset, count, true);
      for (int i = 0; i < count; i++) {
         uint64_t va = radv_descriptor_get_va(descriptors_state, start + i);

         radv_emit_shader_pointer_body(device, cs, va, true);
      }
   }
}

static unsigned
radv_get_rasterization_prim(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *last_vgt_shader = cmd_buffer->state.last_vgt_shader;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   if (cmd_buffer->state.active_stages &
       (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
        VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_MESH_BIT_EXT)) {
      /* Ignore dynamic primitive topology for TES/GS/MS stages. */
      return cmd_buffer->state.rast_prim;
   }

   return radv_conv_prim_to_gs_out(d->vk.ia.primitive_topology, last_vgt_shader->info.is_ngg);
}

static ALWAYS_INLINE unsigned
radv_get_rasterization_samples(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   if (d->vk.rs.line.mode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
       radv_rast_prim_is_line(radv_get_rasterization_prim(cmd_buffer))) {
      /* From the Vulkan spec 1.3.221:
       *
       * "When Bresenham lines are being rasterized, sample locations may all be treated as being at
       * the pixel center (this may affect attribute and depth interpolation)."
       *
       * "One consequence of this is that Bresenham lines cover the same pixels regardless of the
       * number of rasterization samples, and cover all samples in those pixels (unless masked out
       * or killed)."
       */
      return 1;
   }

   if (d->vk.rs.line.mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
       radv_rast_prim_is_line(radv_get_rasterization_prim(cmd_buffer))) {
      return RADV_NUM_SMOOTH_AA_SAMPLES;
   }

   return MAX2(1, d->vk.ms.rasterization_samples);
}

static ALWAYS_INLINE unsigned
radv_get_ps_iter_samples(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   unsigned ps_iter_samples = 1;

   if (cmd_buffer->state.ms.sample_shading_enable) {
      unsigned rasterization_samples = radv_get_rasterization_samples(cmd_buffer);
      unsigned color_samples = MAX2(render->color_samples, rasterization_samples);

      ps_iter_samples = ceilf(cmd_buffer->state.ms.min_sample_shading * color_samples);
      ps_iter_samples = util_next_power_of_two(ps_iter_samples);
   }

   return ps_iter_samples;
}

/**
 * Convert the user sample locations to hardware sample locations (the values
 * that will be emitted by PA_SC_AA_SAMPLE_LOCS_PIXEL_*).
 */
static void
radv_convert_user_sample_locs(const struct radv_sample_locations_state *state, uint32_t x, uint32_t y,
                              VkOffset2D *sample_locs)
{
   uint32_t x_offset = x % state->grid_size.width;
   uint32_t y_offset = y % state->grid_size.height;
   uint32_t num_samples = (uint32_t)state->per_pixel;
   uint32_t pixel_offset;

   pixel_offset = (x_offset + y_offset * state->grid_size.width) * num_samples;

   assert(pixel_offset <= MAX_SAMPLE_LOCATIONS);
   const VkSampleLocationEXT *user_locs = &state->locations[pixel_offset];

   for (uint32_t i = 0; i < num_samples; i++) {
      float shifted_pos_x = user_locs[i].x - 0.5;
      float shifted_pos_y = user_locs[i].y - 0.5;

      int32_t scaled_pos_x = floorf(shifted_pos_x * 16);
      int32_t scaled_pos_y = floorf(shifted_pos_y * 16);

      sample_locs[i].x = CLAMP(scaled_pos_x, -8, 7);
      sample_locs[i].y = CLAMP(scaled_pos_y, -8, 7);
   }
}

/**
 * Compute the PA_SC_AA_SAMPLE_LOCS_PIXEL_* mask based on hardware sample
 * locations.
 */
static void
radv_compute_sample_locs_pixel(uint32_t num_samples, VkOffset2D *sample_locs, uint32_t *sample_locs_pixel)
{
   for (uint32_t i = 0; i < num_samples; i++) {
      uint32_t sample_reg_idx = i / 4;
      uint32_t sample_loc_idx = i % 4;
      int32_t pos_x = sample_locs[i].x;
      int32_t pos_y = sample_locs[i].y;

      uint32_t shift_x = 8 * sample_loc_idx;
      uint32_t shift_y = shift_x + 4;

      sample_locs_pixel[sample_reg_idx] |= (pos_x & 0xf) << shift_x;
      sample_locs_pixel[sample_reg_idx] |= (pos_y & 0xf) << shift_y;
   }
}

/**
 * Compute the PA_SC_CENTROID_PRIORITY_* mask based on the top left hardware
 * sample locations.
 */
static uint64_t
radv_compute_centroid_priority(struct radv_cmd_buffer *cmd_buffer, VkOffset2D *sample_locs, uint32_t num_samples)
{
   uint32_t *centroid_priorities = alloca(num_samples * sizeof(*centroid_priorities));
   uint32_t sample_mask = num_samples - 1;
   uint32_t *distances = alloca(num_samples * sizeof(*distances));
   uint64_t centroid_priority = 0;

   /* Compute the distances from center for each sample. */
   for (int i = 0; i < num_samples; i++) {
      distances[i] = (sample_locs[i].x * sample_locs[i].x) + (sample_locs[i].y * sample_locs[i].y);
   }

   /* Compute the centroid priorities by looking at the distances array. */
   for (int i = 0; i < num_samples; i++) {
      uint32_t min_idx = 0;

      for (int j = 1; j < num_samples; j++) {
         if (distances[j] < distances[min_idx])
            min_idx = j;
      }

      centroid_priorities[i] = min_idx;
      distances[min_idx] = 0xffffffff;
   }

   /* Compute the final centroid priority. */
   for (int i = 0; i < 8; i++) {
      centroid_priority |= centroid_priorities[i & sample_mask] << (i * 4);
   }

   return centroid_priority << 32 | centroid_priority;
}

/**
 * Emit the sample locations that are specified with VK_EXT_sample_locations.
 */
static void
radv_emit_sample_locations(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   uint32_t num_samples = (uint32_t)d->sample_location.per_pixel;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint32_t sample_locs_pixel[4][2] = {0};
   VkOffset2D sample_locs[4][8]; /* 8 is the max. sample count supported */
   uint64_t centroid_priority;

   if (!d->sample_location.count || !d->vk.ms.sample_locations_enable)
      return;

   /* Convert the user sample locations to hardware sample locations. */
   radv_convert_user_sample_locs(&d->sample_location, 0, 0, sample_locs[0]);
   radv_convert_user_sample_locs(&d->sample_location, 1, 0, sample_locs[1]);
   radv_convert_user_sample_locs(&d->sample_location, 0, 1, sample_locs[2]);
   radv_convert_user_sample_locs(&d->sample_location, 1, 1, sample_locs[3]);

   /* Compute the PA_SC_AA_SAMPLE_LOCS_PIXEL_* mask. */
   for (uint32_t i = 0; i < 4; i++) {
      radv_compute_sample_locs_pixel(num_samples, sample_locs[i], sample_locs_pixel[i]);
   }

   /* Compute the PA_SC_CENTROID_PRIORITY_* mask. */
   centroid_priority = radv_compute_centroid_priority(cmd_buffer, sample_locs[0], num_samples);

   /* Emit the specified user sample locations. */
   switch (num_samples) {
   case 2:
   case 4:
      radeon_set_context_reg(cs, R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, sample_locs_pixel[0][0]);
      radeon_set_context_reg(cs, R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0, sample_locs_pixel[1][0]);
      radeon_set_context_reg(cs, R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0, sample_locs_pixel[2][0]);
      radeon_set_context_reg(cs, R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0, sample_locs_pixel[3][0]);
      break;
   case 8:
      radeon_set_context_reg(cs, R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, sample_locs_pixel[0][0]);
      radeon_set_context_reg(cs, R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0, sample_locs_pixel[1][0]);
      radeon_set_context_reg(cs, R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0, sample_locs_pixel[2][0]);
      radeon_set_context_reg(cs, R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0, sample_locs_pixel[3][0]);
      radeon_set_context_reg(cs, R_028BFC_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_1, sample_locs_pixel[0][1]);
      radeon_set_context_reg(cs, R_028C0C_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_1, sample_locs_pixel[1][1]);
      radeon_set_context_reg(cs, R_028C1C_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_1, sample_locs_pixel[2][1]);
      radeon_set_context_reg(cs, R_028C2C_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_1, sample_locs_pixel[3][1]);
      break;
   default:
      unreachable("invalid number of samples");
   }

   radeon_set_context_reg_seq(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0, 2);
   radeon_emit(cs, centroid_priority);
   radeon_emit(cs, centroid_priority >> 32);
}

static void
radv_emit_inline_push_consts(struct radv_device *device, struct radeon_cmdbuf *cs, const struct radv_shader *shader,
                             uint32_t base_reg, int idx, uint32_t *values)
{
   const struct radv_userdata_info *loc = &shader->info.user_sgprs_locs.shader_data[idx];

   if (loc->sgpr_idx == -1)
      return;

   radeon_check_space(device->ws, cs, 2 + loc->num_sgprs);

   radeon_set_sh_reg_seq(cs, base_reg + loc->sgpr_idx * 4, loc->num_sgprs);
   radeon_emit_array(cs, values, loc->num_sgprs);
}

struct radv_bin_size_entry {
   unsigned bpp;
   VkExtent2D extent;
};

static VkExtent2D
radv_gfx10_compute_bin_size(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   VkExtent2D extent = {512, 512};

   const unsigned db_tag_size = 64;
   const unsigned db_tag_count = 312;
   const unsigned color_tag_size = 1024;
   const unsigned color_tag_count = 31;
   const unsigned fmask_tag_size = 256;
   const unsigned fmask_tag_count = 44;

   const unsigned rb_count = pdevice->rad_info.max_render_backends;
   const unsigned pipe_count = MAX2(rb_count, pdevice->rad_info.num_tcc_blocks);

   const unsigned db_tag_part = (db_tag_count * rb_count / pipe_count) * db_tag_size * pipe_count;
   const unsigned color_tag_part = (color_tag_count * rb_count / pipe_count) * color_tag_size * pipe_count;
   const unsigned fmask_tag_part = (fmask_tag_count * rb_count / pipe_count) * fmask_tag_size * pipe_count;

   const unsigned total_samples = radv_get_rasterization_samples(cmd_buffer);
   const unsigned samples_log = util_logbase2_ceil(total_samples);

   unsigned color_bytes_per_pixel = 0;
   unsigned fmask_bytes_per_pixel = 0;

   for (unsigned i = 0; i < render->color_att_count; ++i) {
      struct radv_image_view *iview = render->color_att[i].iview;

      if (!iview)
         continue;

      if (!d->vk.cb.attachments[i].write_mask)
         continue;

      color_bytes_per_pixel += vk_format_get_blocksize(render->color_att[i].format);

      if (total_samples > 1) {
         assert(samples_log <= 3);
         const unsigned fmask_array[] = {0, 1, 1, 4};
         fmask_bytes_per_pixel += fmask_array[samples_log];
      }
   }

   color_bytes_per_pixel *= total_samples;
   color_bytes_per_pixel = MAX2(color_bytes_per_pixel, 1);

   const unsigned color_pixel_count_log = util_logbase2(color_tag_part / color_bytes_per_pixel);
   extent.width = 1ull << ((color_pixel_count_log + 1) / 2);
   extent.height = 1ull << (color_pixel_count_log / 2);

   if (fmask_bytes_per_pixel) {
      const unsigned fmask_pixel_count_log = util_logbase2(fmask_tag_part / fmask_bytes_per_pixel);

      const VkExtent2D fmask_extent = (VkExtent2D){.width = 1ull << ((fmask_pixel_count_log + 1) / 2),
                                                   .height = 1ull << (color_pixel_count_log / 2)};

      if (fmask_extent.width * fmask_extent.height < extent.width * extent.height)
         extent = fmask_extent;
   }

   if (render->ds_att.iview) {
      /* Coefficients taken from AMDVLK */
      unsigned depth_coeff = vk_format_has_depth(render->ds_att.format) ? 5 : 0;
      unsigned stencil_coeff = vk_format_has_stencil(render->ds_att.format) ? 1 : 0;
      unsigned db_bytes_per_pixel = (depth_coeff + stencil_coeff) * total_samples;

      const unsigned db_pixel_count_log = util_logbase2(db_tag_part / db_bytes_per_pixel);

      const VkExtent2D db_extent =
         (VkExtent2D){.width = 1ull << ((db_pixel_count_log + 1) / 2), .height = 1ull << (color_pixel_count_log / 2)};

      if (db_extent.width * db_extent.height < extent.width * extent.height)
         extent = db_extent;
   }

   extent.width = MAX2(extent.width, 128);
   extent.height = MAX2(extent.width, 64);

   return extent;
}

static VkExtent2D
radv_gfx9_compute_bin_size(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   static const struct radv_bin_size_entry color_size_table[][3][9] = {
      {
         /* One RB / SE */
         {
            /* One shader engine */
            {0, {128, 128}},
            {1, {64, 128}},
            {2, {32, 128}},
            {3, {16, 128}},
            {17, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            /* Two shader engines */
            {0, {128, 128}},
            {2, {64, 128}},
            {3, {32, 128}},
            {5, {16, 128}},
            {17, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            /* Four shader engines */
            {0, {128, 128}},
            {3, {64, 128}},
            {5, {16, 128}},
            {17, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
      },
      {
         /* Two RB / SE */
         {
            /* One shader engine */
            {0, {128, 128}},
            {2, {64, 128}},
            {3, {32, 128}},
            {5, {16, 128}},
            {33, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            /* Two shader engines */
            {0, {128, 128}},
            {3, {64, 128}},
            {5, {32, 128}},
            {9, {16, 128}},
            {33, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            /* Four shader engines */
            {0, {256, 256}},
            {2, {128, 256}},
            {3, {128, 128}},
            {5, {64, 128}},
            {9, {16, 128}},
            {33, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
      },
      {
         /* Four RB / SE */
         {
            /* One shader engine */
            {0, {128, 256}},
            {2, {128, 128}},
            {3, {64, 128}},
            {5, {32, 128}},
            {9, {16, 128}},
            {33, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            /* Two shader engines */
            {0, {256, 256}},
            {2, {128, 256}},
            {3, {128, 128}},
            {5, {64, 128}},
            {9, {32, 128}},
            {17, {16, 128}},
            {33, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            /* Four shader engines */
            {0, {256, 512}},
            {2, {256, 256}},
            {3, {128, 256}},
            {5, {128, 128}},
            {9, {64, 128}},
            {17, {16, 128}},
            {33, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
      },
   };
   static const struct radv_bin_size_entry ds_size_table[][3][9] = {
      {
         // One RB / SE
         {
            // One shader engine
            {0, {128, 256}},
            {2, {128, 128}},
            {4, {64, 128}},
            {7, {32, 128}},
            {13, {16, 128}},
            {49, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            // Two shader engines
            {0, {256, 256}},
            {2, {128, 256}},
            {4, {128, 128}},
            {7, {64, 128}},
            {13, {32, 128}},
            {25, {16, 128}},
            {49, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            // Four shader engines
            {0, {256, 512}},
            {2, {256, 256}},
            {4, {128, 256}},
            {7, {128, 128}},
            {13, {64, 128}},
            {25, {16, 128}},
            {49, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
      },
      {
         // Two RB / SE
         {
            // One shader engine
            {0, {256, 256}},
            {2, {128, 256}},
            {4, {128, 128}},
            {7, {64, 128}},
            {13, {32, 128}},
            {25, {16, 128}},
            {97, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            // Two shader engines
            {0, {256, 512}},
            {2, {256, 256}},
            {4, {128, 256}},
            {7, {128, 128}},
            {13, {64, 128}},
            {25, {32, 128}},
            {49, {16, 128}},
            {97, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
         {
            // Four shader engines
            {0, {512, 512}},
            {2, {256, 512}},
            {4, {256, 256}},
            {7, {128, 256}},
            {13, {128, 128}},
            {25, {64, 128}},
            {49, {16, 128}},
            {97, {0, 0}},
            {UINT_MAX, {0, 0}},
         },
      },
      {
         // Four RB / SE
         {
            // One shader engine
            {0, {256, 512}},
            {2, {256, 256}},
            {4, {128, 256}},
            {7, {128, 128}},
            {13, {64, 128}},
            {25, {32, 128}},
            {49, {16, 128}},
            {UINT_MAX, {0, 0}},
         },
         {
            // Two shader engines
            {0, {512, 512}},
            {2, {256, 512}},
            {4, {256, 256}},
            {7, {128, 256}},
            {13, {128, 128}},
            {25, {64, 128}},
            {49, {32, 128}},
            {97, {16, 128}},
            {UINT_MAX, {0, 0}},
         },
         {
            // Four shader engines
            {0, {512, 512}},
            {4, {256, 512}},
            {7, {256, 256}},
            {13, {128, 256}},
            {25, {128, 128}},
            {49, {64, 128}},
            {97, {16, 128}},
            {UINT_MAX, {0, 0}},
         },
      },
   };

   VkExtent2D extent = {512, 512};

   unsigned log_num_rb_per_se = util_logbase2_ceil(pdevice->rad_info.max_render_backends / pdevice->rad_info.max_se);
   unsigned log_num_se = util_logbase2_ceil(pdevice->rad_info.max_se);

   unsigned total_samples = radv_get_rasterization_samples(cmd_buffer);
   unsigned ps_iter_samples = radv_get_ps_iter_samples(cmd_buffer);
   unsigned effective_samples = total_samples;
   unsigned color_bytes_per_pixel = 0;

   for (unsigned i = 0; i < render->color_att_count; ++i) {
      struct radv_image_view *iview = render->color_att[i].iview;

      if (!iview)
         continue;

      if (!d->vk.cb.attachments[i].write_mask)
         continue;

      color_bytes_per_pixel += vk_format_get_blocksize(render->color_att[i].format);
   }

   /* MSAA images typically don't use all samples all the time. */
   if (effective_samples >= 2 && ps_iter_samples <= 1)
      effective_samples = 2;
   color_bytes_per_pixel *= effective_samples;

   const struct radv_bin_size_entry *color_entry = color_size_table[log_num_rb_per_se][log_num_se];
   while (color_entry[1].bpp <= color_bytes_per_pixel)
      ++color_entry;

   extent = color_entry->extent;

   if (render->ds_att.iview) {
      /* Coefficients taken from AMDVLK */
      unsigned depth_coeff = vk_format_has_depth(render->ds_att.format) ? 5 : 0;
      unsigned stencil_coeff = vk_format_has_stencil(render->ds_att.format) ? 1 : 0;
      unsigned ds_bytes_per_pixel = 4 * (depth_coeff + stencil_coeff) * total_samples;

      const struct radv_bin_size_entry *ds_entry = ds_size_table[log_num_rb_per_se][log_num_se];
      while (ds_entry[1].bpp <= ds_bytes_per_pixel)
         ++ds_entry;

      if (ds_entry->extent.width * ds_entry->extent.height < extent.width * extent.height)
         extent = ds_entry->extent;
   }

   return extent;
}

static unsigned
radv_get_disabled_binning_state(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   uint32_t pa_sc_binner_cntl_0;

   if (pdevice->rad_info.gfx_level >= GFX10) {
      unsigned min_bytes_per_pixel = 0;

      for (unsigned i = 0; i < render->color_att_count; ++i) {
         struct radv_image_view *iview = render->color_att[i].iview;

         if (!iview)
            continue;

         if (!d->vk.cb.attachments[i].write_mask)
            continue;

         unsigned bytes = vk_format_get_blocksize(render->color_att[i].format);
         if (!min_bytes_per_pixel || bytes < min_bytes_per_pixel)
            min_bytes_per_pixel = bytes;
      }

      pa_sc_binner_cntl_0 = S_028C44_BINNING_MODE(V_028C44_DISABLE_BINNING_USE_NEW_SC) | S_028C44_BIN_SIZE_X(0) |
                            S_028C44_BIN_SIZE_Y(0) | S_028C44_BIN_SIZE_X_EXTEND(2) |       /* 128 */
                            S_028C44_BIN_SIZE_Y_EXTEND(min_bytes_per_pixel <= 4 ? 2 : 1) | /* 128 or 64 */
                            S_028C44_DISABLE_START_OF_PRIM(1) | S_028C44_FLUSH_ON_BINNING_TRANSITION(1);
   } else {
      pa_sc_binner_cntl_0 = S_028C44_BINNING_MODE(V_028C44_DISABLE_BINNING_USE_LEGACY_SC) |
                            S_028C44_DISABLE_START_OF_PRIM(1) |
                            S_028C44_FLUSH_ON_BINNING_TRANSITION(pdevice->rad_info.family == CHIP_VEGA12 ||
                                                                 pdevice->rad_info.family == CHIP_VEGA20 ||
                                                                 pdevice->rad_info.family >= CHIP_RAVEN2);
   }

   return pa_sc_binner_cntl_0;
}

static unsigned
radv_get_binning_state(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_device *device = cmd_buffer->device;
   unsigned pa_sc_binner_cntl_0;
   VkExtent2D bin_size;

   if (device->physical_device->rad_info.gfx_level >= GFX10) {
      bin_size = radv_gfx10_compute_bin_size(cmd_buffer);
   } else {
      assert(device->physical_device->rad_info.gfx_level == GFX9);
      bin_size = radv_gfx9_compute_bin_size(cmd_buffer);
   }

   if (device->pbb_allowed && bin_size.width && bin_size.height) {
      struct radv_binning_settings *settings = &device->physical_device->binning_settings;

      pa_sc_binner_cntl_0 =
         S_028C44_BINNING_MODE(V_028C44_BINNING_ALLOWED) | S_028C44_BIN_SIZE_X(bin_size.width == 16) |
         S_028C44_BIN_SIZE_Y(bin_size.height == 16) |
         S_028C44_BIN_SIZE_X_EXTEND(util_logbase2(MAX2(bin_size.width, 32)) - 5) |
         S_028C44_BIN_SIZE_Y_EXTEND(util_logbase2(MAX2(bin_size.height, 32)) - 5) |
         S_028C44_CONTEXT_STATES_PER_BIN(settings->context_states_per_bin - 1) |
         S_028C44_PERSISTENT_STATES_PER_BIN(settings->persistent_states_per_bin - 1) |
         S_028C44_DISABLE_START_OF_PRIM(1) | S_028C44_FPOVS_PER_BATCH(settings->fpovs_per_batch) |
         S_028C44_OPTIMAL_BIN_SELECTION(1) |
         S_028C44_FLUSH_ON_BINNING_TRANSITION(device->physical_device->rad_info.family == CHIP_VEGA12 ||
                                              device->physical_device->rad_info.family == CHIP_VEGA20 ||
                                              device->physical_device->rad_info.family >= CHIP_RAVEN2);
   } else {
      pa_sc_binner_cntl_0 = radv_get_disabled_binning_state(cmd_buffer);
   }

   return pa_sc_binner_cntl_0;
}

static void
radv_emit_binning_state(struct radv_cmd_buffer *cmd_buffer)
{
   unsigned pa_sc_binner_cntl_0;

   if (cmd_buffer->device->physical_device->rad_info.gfx_level < GFX9)
      return;

   pa_sc_binner_cntl_0 = radv_get_binning_state(cmd_buffer);

   if (pa_sc_binner_cntl_0 == cmd_buffer->state.last_pa_sc_binner_cntl_0)
      return;

   radeon_set_context_reg(cmd_buffer->cs, R_028C44_PA_SC_BINNER_CNTL_0, pa_sc_binner_cntl_0);

   cmd_buffer->state.last_pa_sc_binner_cntl_0 = pa_sc_binner_cntl_0;
}

static void
radv_emit_shader_prefetch(struct radv_cmd_buffer *cmd_buffer, struct radv_shader *shader)
{
   uint64_t va;

   if (!shader)
      return;

   va = radv_shader_get_va(shader);

   radv_cp_dma_prefetch(cmd_buffer, va, shader->code_size);
}

ALWAYS_INLINE static void
radv_emit_prefetch_L2(struct radv_cmd_buffer *cmd_buffer, bool first_stage_only)
{
   struct radv_cmd_state *state = &cmd_buffer->state;
   uint32_t mask = state->prefetch_L2_mask;

   /* Fast prefetch path for starting draws as soon as possible. */
   if (first_stage_only)
      mask &= RADV_PREFETCH_VS | RADV_PREFETCH_VBO_DESCRIPTORS | RADV_PREFETCH_MS;

   if (mask & RADV_PREFETCH_VS)
      radv_emit_shader_prefetch(cmd_buffer, cmd_buffer->state.shaders[MESA_SHADER_VERTEX]);

   if (mask & RADV_PREFETCH_MS)
      radv_emit_shader_prefetch(cmd_buffer, cmd_buffer->state.shaders[MESA_SHADER_MESH]);

   if (mask & RADV_PREFETCH_VBO_DESCRIPTORS)
      radv_cp_dma_prefetch(cmd_buffer, state->vb_va, state->vb_size);

   if (mask & RADV_PREFETCH_TCS)
      radv_emit_shader_prefetch(cmd_buffer, cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL]);

   if (mask & RADV_PREFETCH_TES)
      radv_emit_shader_prefetch(cmd_buffer, cmd_buffer->state.shaders[MESA_SHADER_TESS_EVAL]);

   if (mask & RADV_PREFETCH_GS) {
      radv_emit_shader_prefetch(cmd_buffer, cmd_buffer->state.shaders[MESA_SHADER_GEOMETRY]);
      if (cmd_buffer->state.gs_copy_shader)
         radv_emit_shader_prefetch(cmd_buffer, cmd_buffer->state.gs_copy_shader);
   }

   if (mask & RADV_PREFETCH_PS) {
      radv_emit_shader_prefetch(cmd_buffer, cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT]);
   }

   state->prefetch_L2_mask &= ~mask;
}

static void
radv_emit_rbplus_state(struct radv_cmd_buffer *cmd_buffer)
{
   assert(cmd_buffer->device->physical_device->rad_info.rbplus_allowed);

   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   struct radv_rendering_state *render = &cmd_buffer->state.render;

   unsigned sx_ps_downconvert = 0;
   unsigned sx_blend_opt_epsilon = 0;
   unsigned sx_blend_opt_control = 0;

   for (unsigned i = 0; i < render->color_att_count; i++) {
      unsigned format, swap;
      bool has_alpha, has_rgb;
      if (render->color_att[i].iview == NULL) {
         /* We don't set the DISABLE bits, because the HW can't have holes,
          * so the SPI color format is set to 32-bit 1-component. */
         sx_ps_downconvert |= V_028754_SX_RT_EXPORT_32_R << (i * 4);
         continue;
      }

      struct radv_color_buffer_info *cb = &render->color_att[i].cb;

      format = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11
                  ? G_028C70_FORMAT_GFX11(cb->cb_color_info)
                  : G_028C70_FORMAT_GFX6(cb->cb_color_info);
      swap = G_028C70_COMP_SWAP(cb->cb_color_info);
      has_alpha = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11
                     ? !G_028C74_FORCE_DST_ALPHA_1_GFX11(cb->cb_color_attrib)
                     : !G_028C74_FORCE_DST_ALPHA_1_GFX6(cb->cb_color_attrib);

      uint32_t spi_format = (cmd_buffer->state.col_format_non_compacted >> (i * 4)) & 0xf;
      uint32_t colormask = d->vk.cb.attachments[i].write_mask;

      if (format == V_028C70_COLOR_8 || format == V_028C70_COLOR_16 || format == V_028C70_COLOR_32)
         has_rgb = !has_alpha;
      else
         has_rgb = true;

      /* Check the colormask and export format. */
      if (!(colormask & 0x7))
         has_rgb = false;
      if (!(colormask & 0x8))
         has_alpha = false;

      if (spi_format == V_028714_SPI_SHADER_ZERO) {
         has_rgb = false;
         has_alpha = false;
      }

      /* The HW doesn't quite blend correctly with rgb9e5 if we disable the alpha
       * optimization, even though it has no alpha. */
      if (has_rgb && format == V_028C70_COLOR_5_9_9_9)
         has_alpha = true;

      /* Disable value checking for disabled channels. */
      if (!has_rgb)
         sx_blend_opt_control |= S_02875C_MRT0_COLOR_OPT_DISABLE(1) << (i * 4);
      if (!has_alpha)
         sx_blend_opt_control |= S_02875C_MRT0_ALPHA_OPT_DISABLE(1) << (i * 4);

      /* Enable down-conversion for 32bpp and smaller formats. */
      switch (format) {
      case V_028C70_COLOR_8:
      case V_028C70_COLOR_8_8:
      case V_028C70_COLOR_8_8_8_8:
         /* For 1 and 2-channel formats, use the superset thereof. */
         if (spi_format == V_028714_SPI_SHADER_FP16_ABGR || spi_format == V_028714_SPI_SHADER_UINT16_ABGR ||
             spi_format == V_028714_SPI_SHADER_SINT16_ABGR) {
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_8_8_8_8 << (i * 4);

            if (G_028C70_NUMBER_TYPE(cb->cb_color_info) != V_028C70_NUMBER_SRGB)
               sx_blend_opt_epsilon |= V_028758_8BIT_FORMAT_0_5 << (i * 4);
         }
         break;

      case V_028C70_COLOR_5_6_5:
         if (spi_format == V_028714_SPI_SHADER_FP16_ABGR) {
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_5_6_5 << (i * 4);
            sx_blend_opt_epsilon |= V_028758_6BIT_FORMAT_0_5 << (i * 4);
         }
         break;

      case V_028C70_COLOR_1_5_5_5:
         if (spi_format == V_028714_SPI_SHADER_FP16_ABGR) {
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_1_5_5_5 << (i * 4);
            sx_blend_opt_epsilon |= V_028758_5BIT_FORMAT_0_5 << (i * 4);
         }
         break;

      case V_028C70_COLOR_4_4_4_4:
         if (spi_format == V_028714_SPI_SHADER_FP16_ABGR) {
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_4_4_4_4 << (i * 4);
            sx_blend_opt_epsilon |= V_028758_4BIT_FORMAT_0_5 << (i * 4);
         }
         break;

      case V_028C70_COLOR_32:
         if (swap == V_028C70_SWAP_STD && spi_format == V_028714_SPI_SHADER_32_R)
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_32_R << (i * 4);
         else if (swap == V_028C70_SWAP_ALT_REV && spi_format == V_028714_SPI_SHADER_32_AR)
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_32_A << (i * 4);
         break;

      case V_028C70_COLOR_16:
      case V_028C70_COLOR_16_16:
         /* For 1-channel formats, use the superset thereof. */
         if (spi_format == V_028714_SPI_SHADER_UNORM16_ABGR || spi_format == V_028714_SPI_SHADER_SNORM16_ABGR ||
             spi_format == V_028714_SPI_SHADER_UINT16_ABGR || spi_format == V_028714_SPI_SHADER_SINT16_ABGR) {
            if (swap == V_028C70_SWAP_STD || swap == V_028C70_SWAP_STD_REV)
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_16_16_GR << (i * 4);
            else
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_16_16_AR << (i * 4);
         }
         break;

      case V_028C70_COLOR_10_11_11:
         if (spi_format == V_028714_SPI_SHADER_FP16_ABGR)
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_10_11_11 << (i * 4);
         break;

      case V_028C70_COLOR_2_10_10_10:
         if (spi_format == V_028714_SPI_SHADER_FP16_ABGR) {
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_2_10_10_10 << (i * 4);
            sx_blend_opt_epsilon |= V_028758_10BIT_FORMAT_0_5 << (i * 4);
         }
         break;
      case V_028C70_COLOR_5_9_9_9:
         if (spi_format == V_028714_SPI_SHADER_FP16_ABGR)
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_9_9_9_E5 << (i * 4);
         break;
      }
   }

   /* Do not set the DISABLE bits for the unused attachments, as that
    * breaks dual source blending in SkQP and does not seem to improve
    * performance. */

   if (sx_ps_downconvert != cmd_buffer->state.last_sx_ps_downconvert ||
       sx_blend_opt_epsilon != cmd_buffer->state.last_sx_blend_opt_epsilon ||
       sx_blend_opt_control != cmd_buffer->state.last_sx_blend_opt_control) {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028754_SX_PS_DOWNCONVERT, 3);
      radeon_emit(cmd_buffer->cs, sx_ps_downconvert);
      radeon_emit(cmd_buffer->cs, sx_blend_opt_epsilon);
      radeon_emit(cmd_buffer->cs, sx_blend_opt_control);

      cmd_buffer->state.last_sx_ps_downconvert = sx_ps_downconvert;
      cmd_buffer->state.last_sx_blend_opt_epsilon = sx_blend_opt_epsilon;
      cmd_buffer->state.last_sx_blend_opt_control = sx_blend_opt_control;
   }

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_RBPLUS;
}

static void
radv_emit_ps_epilog_state(struct radv_cmd_buffer *cmd_buffer, struct radv_shader_part *ps_epilog)
{
   struct radv_shader *ps_shader = cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT];
   const struct radv_device *device = cmd_buffer->device;

   if (cmd_buffer->state.emitted_ps_epilog == ps_epilog)
      return;

   uint32_t col_format = radv_compact_spi_shader_col_format(ps_shader, ps_epilog->spi_shader_col_format);

   bool need_null_export_workaround =
      radv_needs_null_export_workaround(device, ps_shader, cmd_buffer->state.custom_blend_mode);
   if (need_null_export_workaround && !col_format)
      col_format = V_028714_SPI_SHADER_32_R;
   radeon_set_context_reg(cmd_buffer->cs, R_028714_SPI_SHADER_COL_FORMAT, col_format);
   radeon_set_context_reg(cmd_buffer->cs, R_02823C_CB_SHADER_MASK,
                          ac_get_cb_shader_mask(ps_epilog->spi_shader_col_format));

   if (ps_epilog->spi_shader_z_format)
      radeon_set_context_reg(cmd_buffer->cs, R_028710_SPI_SHADER_Z_FORMAT, ps_epilog->spi_shader_z_format);

   assert(ps_shader->config.num_shared_vgprs == 0);
   if (G_00B848_VGPRS(ps_epilog->rsrc1) > G_00B848_VGPRS(ps_shader->config.rsrc1)) {
      uint32_t rsrc1 = ps_shader->config.rsrc1;
      rsrc1 = (rsrc1 & C_00B848_VGPRS) | (ps_epilog->rsrc1 & ~C_00B848_VGPRS);
      radeon_set_sh_reg(cmd_buffer->cs, R_00B028_SPI_SHADER_PGM_RSRC1_PS, rsrc1);
   }

   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, ps_epilog->bo);

   assert((ps_epilog->va >> 32) == cmd_buffer->device->physical_device->rad_info.address32_hi);

   struct radv_userdata_info *loc = &ps_shader->info.user_sgprs_locs.shader_data[AC_UD_PS_EPILOG_PC];
   uint32_t base_reg = ps_shader->info.user_data_0;
   assert(loc->sgpr_idx != -1);
   assert(loc->num_sgprs == 1);
   radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, ps_epilog->va, false);

   cmd_buffer->shader_upload_seq = MAX2(cmd_buffer->shader_upload_seq, ps_epilog->upload_seq);

   cmd_buffer->state.emitted_ps_epilog = ps_epilog;
}

static void
radv_emit_tcs_epilog_state(struct radv_cmd_buffer *cmd_buffer, struct radv_shader_part *tcs_epilog)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   struct radv_shader *tcs = cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL];

   if (cmd_buffer->state.emitted_tcs_epilog == tcs_epilog)
      return;

   assert(tcs->config.num_shared_vgprs == 0);
   uint32_t rsrc1 = tcs->config.rsrc1;
   if (G_00B848_VGPRS(tcs_epilog->rsrc1) > G_00B848_VGPRS(tcs->config.rsrc1))
      rsrc1 = (rsrc1 & C_00B848_VGPRS) | (tcs_epilog->rsrc1 & ~C_00B848_VGPRS);
   if (gfx_level < GFX10 && G_00B228_SGPRS(tcs_epilog->rsrc1) > G_00B228_SGPRS(tcs->config.rsrc1))
      rsrc1 = (rsrc1 & C_00B228_SGPRS) | (tcs_epilog->rsrc1 & ~C_00B228_SGPRS);
   if (rsrc1 != tcs->config.rsrc1)
      radeon_set_sh_reg(cmd_buffer->cs, R_00B428_SPI_SHADER_PGM_RSRC1_HS, rsrc1);

   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, tcs_epilog->bo);

   assert((tcs_epilog->va >> 32) == cmd_buffer->device->physical_device->rad_info.address32_hi);

   struct radv_userdata_info *loc = &tcs->info.user_sgprs_locs.shader_data[AC_UD_TCS_EPILOG_PC];
   uint32_t base_reg = tcs->info.user_data_0;
   assert(loc->sgpr_idx != -1);
   assert(loc->num_sgprs == 1);
   radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, tcs_epilog->va, false);

   cmd_buffer->shader_upload_seq = MAX2(cmd_buffer->shader_upload_seq, tcs_epilog->upload_seq);

   cmd_buffer->state.emitted_tcs_epilog = tcs_epilog;
}

static void
radv_emit_graphics_pipeline(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_graphics_pipeline *pipeline = cmd_buffer->state.graphics_pipeline;
   const struct radv_device *device = cmd_buffer->device;

   if (cmd_buffer->state.emitted_graphics_pipeline == pipeline)
      return;

   if (cmd_buffer->state.emitted_graphics_pipeline) {
      if (radv_rast_prim_is_points_or_lines(cmd_buffer->state.emitted_graphics_pipeline->rast_prim) !=
          radv_rast_prim_is_points_or_lines(pipeline->rast_prim))
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_GUARDBAND;

      if (cmd_buffer->state.emitted_graphics_pipeline->custom_blend_mode != pipeline->custom_blend_mode)
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_LOGIC_OP | RADV_CMD_DIRTY_DYNAMIC_LOGIC_OP_ENABLE;

      if (cmd_buffer->state.emitted_graphics_pipeline->ms.min_sample_shading != pipeline->ms.min_sample_shading ||
          cmd_buffer->state.emitted_graphics_pipeline->uses_out_of_order_rast != pipeline->uses_out_of_order_rast ||
          cmd_buffer->state.emitted_graphics_pipeline->uses_vrs_attachment != pipeline->uses_vrs_attachment ||
          cmd_buffer->state.emitted_graphics_pipeline->rast_prim != pipeline->rast_prim)

         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES;

      if (cmd_buffer->state.emitted_graphics_pipeline->ms.sample_shading_enable != pipeline->ms.sample_shading_enable) {
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES;
         if (device->physical_device->rad_info.gfx_level >= GFX10_3)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_FRAGMENT_SHADING_RATE;
      }

      if (cmd_buffer->state.emitted_graphics_pipeline->db_render_control != pipeline->db_render_control)
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_FRAMEBUFFER;
   }

   radeon_emit_array(cmd_buffer->cs, pipeline->base.cs.buf, pipeline->base.cs.cdw);

   if (!cmd_buffer->state.emitted_graphics_pipeline ||
       cmd_buffer->state.emitted_graphics_pipeline->base.ctx_cs.cdw != pipeline->base.ctx_cs.cdw ||
       cmd_buffer->state.emitted_graphics_pipeline->base.ctx_cs_hash != pipeline->base.ctx_cs_hash ||
       memcmp(cmd_buffer->state.emitted_graphics_pipeline->base.ctx_cs.buf, pipeline->base.ctx_cs.buf,
              pipeline->base.ctx_cs.cdw * 4)) {
      radeon_emit_array(cmd_buffer->cs, pipeline->base.ctx_cs.buf, pipeline->base.ctx_cs.cdw);
   }

   if (device->pbb_allowed) {
      struct radv_binning_settings *settings = &device->physical_device->binning_settings;

      if ((!cmd_buffer->state.emitted_graphics_pipeline ||
           cmd_buffer->state.emitted_graphics_pipeline->base.shaders[MESA_SHADER_FRAGMENT] !=
              cmd_buffer->state.graphics_pipeline->base.shaders[MESA_SHADER_FRAGMENT]) &&
          (settings->context_states_per_bin > 1 || settings->persistent_states_per_bin > 1)) {
         /* Break the batch on PS changes. */
         radeon_emit(cmd_buffer->cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cmd_buffer->cs, EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
      }
   }

   if (pipeline->sqtt_shaders_reloc) {
      /* Emit shaders relocation because RGP requires them to be contiguous in memory. */
      radv_sqtt_emit_relocated_shaders(cmd_buffer, pipeline);
   }

   for (unsigned s = 0; s < MESA_VULKAN_SHADER_STAGES; s++) {
      struct radv_shader *shader = cmd_buffer->state.shaders[s];

      if (!shader)
         continue;

      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, shader->bo);
   }

   if (cmd_buffer->state.gs_copy_shader) {
      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, cmd_buffer->state.gs_copy_shader->bo);
   }

   struct radv_shader *task_shader = cmd_buffer->state.shaders[MESA_SHADER_TASK];
   if (task_shader) {
      radv_emit_compute_shader(device->physical_device, cmd_buffer->gang.cs, task_shader);

      /* Relocate the task shader because RGP requires shaders to be contiguous in memory. */
      if (pipeline->sqtt_shaders_reloc) {
         const struct radv_sqtt_shaders_reloc *reloc = pipeline->sqtt_shaders_reloc;
         const uint64_t va = reloc->va[MESA_SHADER_TASK];

         radeon_set_sh_reg(cmd_buffer->gang.cs, R_00B830_COMPUTE_PGM_LO, va >> 8);
      }
   }

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_save_pipeline(cmd_buffer, &pipeline->base);

   cmd_buffer->state.emitted_graphics_pipeline = pipeline;

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_PIPELINE;
}

static bool
radv_get_depth_clip_enable(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   return d->vk.rs.depth_clip_enable == VK_MESA_DEPTH_CLIP_ENABLE_TRUE ||
          (d->vk.rs.depth_clip_enable == VK_MESA_DEPTH_CLIP_ENABLE_NOT_CLAMP && !d->vk.rs.depth_clamp_enable);
}

enum radv_depth_clamp_mode {
   RADV_DEPTH_CLAMP_MODE_VIEWPORT = 0,    /* Clamp to the viewport min/max depth bounds */
   RADV_DEPTH_CLAMP_MODE_ZERO_TO_ONE = 1, /* Clamp between 0.0f and 1.0f */
   RADV_DEPTH_CLAMP_MODE_DISABLED = 2,    /* Disable depth clamping */
};

static enum radv_depth_clamp_mode
radv_get_depth_clamp_mode(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   bool depth_clip_enable = radv_get_depth_clip_enable(cmd_buffer);
   const struct radv_device *device = cmd_buffer->device;
   enum radv_depth_clamp_mode mode;

   mode = RADV_DEPTH_CLAMP_MODE_VIEWPORT;
   if (!d->vk.rs.depth_clamp_enable) {
      /* For optimal performance, depth clamping should always be enabled except if the application
       * disables clamping explicitly or uses depth values outside of the [0.0, 1.0] range.
       */
      if (!depth_clip_enable || device->vk.enabled_extensions.EXT_depth_range_unrestricted) {
         mode = RADV_DEPTH_CLAMP_MODE_DISABLED;
      } else {
         mode = RADV_DEPTH_CLAMP_MODE_ZERO_TO_ONE;
      }
   }

   return mode;
}

static void
radv_emit_viewport(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   enum radv_depth_clamp_mode depth_clamp_mode = radv_get_depth_clamp_mode(cmd_buffer);

   assert(d->vk.vp.viewport_count);
   radeon_set_context_reg_seq(cmd_buffer->cs, R_02843C_PA_CL_VPORT_XSCALE, d->vk.vp.viewport_count * 6);

   for (unsigned i = 0; i < d->vk.vp.viewport_count; i++) {
      radeon_emit(cmd_buffer->cs, fui(d->hw_vp.xform[i].scale[0]));
      radeon_emit(cmd_buffer->cs, fui(d->hw_vp.xform[i].translate[0]));
      radeon_emit(cmd_buffer->cs, fui(d->hw_vp.xform[i].scale[1]));
      radeon_emit(cmd_buffer->cs, fui(d->hw_vp.xform[i].translate[1]));

      double scale_z, translate_z;
      if (d->vk.vp.depth_clip_negative_one_to_one) {
         scale_z = d->hw_vp.xform[i].scale[2] * 0.5f;
         translate_z = (d->hw_vp.xform[i].translate[2] + d->vk.vp.viewports[i].maxDepth) * 0.5f;
      } else {
         scale_z = d->hw_vp.xform[i].scale[2];
         translate_z = d->hw_vp.xform[i].translate[2];
      }
      radeon_emit(cmd_buffer->cs, fui(scale_z));
      radeon_emit(cmd_buffer->cs, fui(translate_z));
   }

   radeon_set_context_reg_seq(cmd_buffer->cs, R_0282D0_PA_SC_VPORT_ZMIN_0, d->vk.vp.viewport_count * 2);
   for (unsigned i = 0; i < d->vk.vp.viewport_count; i++) {
      float zmin, zmax;

      if (depth_clamp_mode == RADV_DEPTH_CLAMP_MODE_ZERO_TO_ONE) {
         zmin = 0.0f;
         zmax = 1.0f;
      } else {
         zmin = MIN2(d->vk.vp.viewports[i].minDepth, d->vk.vp.viewports[i].maxDepth);
         zmax = MAX2(d->vk.vp.viewports[i].minDepth, d->vk.vp.viewports[i].maxDepth);
      }

      radeon_emit(cmd_buffer->cs, fui(zmin));
      radeon_emit(cmd_buffer->cs, fui(zmax));
   }
}

static void
radv_emit_scissor(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   radv_write_scissors(cmd_buffer->cs, d->vk.vp.scissor_count, d->vk.vp.scissors, d->vk.vp.viewports);
}

static void
radv_emit_discard_rectangle(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   uint32_t cliprect_rule = 0;

   if (!d->vk.dr.enable) {
      cliprect_rule = 0xffff;
   } else {
      for (unsigned i = 0; i < (1u << MAX_DISCARD_RECTANGLES); ++i) {
         /* Interpret i as a bitmask, and then set the bit in
          * the mask if that combination of rectangles in which
          * the pixel is contained should pass the cliprect
          * test.
          */
         unsigned relevant_subset = i & ((1u << d->vk.dr.rectangle_count) - 1);

         if (d->vk.dr.mode == VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT && !relevant_subset)
            continue;

         if (d->vk.dr.mode == VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT && relevant_subset)
            continue;

         cliprect_rule |= 1u << i;
      }

      radeon_set_context_reg_seq(cmd_buffer->cs, R_028210_PA_SC_CLIPRECT_0_TL, d->vk.dr.rectangle_count * 2);
      for (unsigned i = 0; i < d->vk.dr.rectangle_count; ++i) {
         VkRect2D rect = d->vk.dr.rectangles[i];
         radeon_emit(cmd_buffer->cs, S_028210_TL_X(rect.offset.x) | S_028210_TL_Y(rect.offset.y));
         radeon_emit(cmd_buffer->cs, S_028214_BR_X(rect.offset.x + rect.extent.width) |
                                        S_028214_BR_Y(rect.offset.y + rect.extent.height));
      }
   }

   radeon_set_context_reg(cmd_buffer->cs, R_02820C_PA_SC_CLIPRECT_RULE, cliprect_rule);
}

static void
radv_emit_line_width(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   radeon_set_context_reg(cmd_buffer->cs, R_028A08_PA_SU_LINE_CNTL,
                          S_028A08_WIDTH(CLAMP(d->vk.rs.line.width * 8, 0, 0xFFFF)));
}

static void
radv_emit_blend_constants(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   radeon_set_context_reg_seq(cmd_buffer->cs, R_028414_CB_BLEND_RED, 4);
   radeon_emit_array(cmd_buffer->cs, (uint32_t *)d->vk.cb.blend_constants, 4);
}

static void
radv_emit_stencil(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   radeon_set_context_reg_seq(cmd_buffer->cs, R_028430_DB_STENCILREFMASK, 2);
   radeon_emit(cmd_buffer->cs, S_028430_STENCILTESTVAL(d->vk.ds.stencil.front.reference) |
                                  S_028430_STENCILMASK(d->vk.ds.stencil.front.compare_mask) |
                                  S_028430_STENCILWRITEMASK(d->vk.ds.stencil.front.write_mask) |
                                  S_028430_STENCILOPVAL(1));
   radeon_emit(cmd_buffer->cs, S_028434_STENCILTESTVAL_BF(d->vk.ds.stencil.back.reference) |
                                  S_028434_STENCILMASK_BF(d->vk.ds.stencil.back.compare_mask) |
                                  S_028434_STENCILWRITEMASK_BF(d->vk.ds.stencil.back.write_mask) |
                                  S_028434_STENCILOPVAL_BF(1));
}

static void
radv_emit_depth_bounds(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   radeon_set_context_reg_seq(cmd_buffer->cs, R_028020_DB_DEPTH_BOUNDS_MIN, 2);
   radeon_emit(cmd_buffer->cs, fui(d->vk.ds.depth.bounds_test.min));
   radeon_emit(cmd_buffer->cs, fui(d->vk.ds.depth.bounds_test.max));
}

static void
radv_emit_depth_bias(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   struct radv_rendering_state *render = &cmd_buffer->state.render;
   unsigned slope = fui(d->vk.rs.depth_bias.slope * 16.0f);
   unsigned pa_su_poly_offset_db_fmt_cntl = 0;

   if (vk_format_has_depth(render->ds_att.format) &&
       d->vk.rs.depth_bias.representation != VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT) {
      VkFormat format = vk_format_depth_only(render->ds_att.format);

      if (format == VK_FORMAT_D16_UNORM) {
         pa_su_poly_offset_db_fmt_cntl = S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS(-16);
      } else {
         assert(format == VK_FORMAT_D32_SFLOAT);
         if (d->vk.rs.depth_bias.representation ==
             VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT) {
            pa_su_poly_offset_db_fmt_cntl = S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS(-24);
         } else {
            pa_su_poly_offset_db_fmt_cntl =
               S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS(-23) | S_028B78_POLY_OFFSET_DB_IS_FLOAT_FMT(1);
         }
      }
   }

   radeon_set_context_reg_seq(cmd_buffer->cs, R_028B7C_PA_SU_POLY_OFFSET_CLAMP, 5);
   radeon_emit(cmd_buffer->cs, fui(d->vk.rs.depth_bias.clamp));    /* CLAMP */
   radeon_emit(cmd_buffer->cs, slope);                             /* FRONT SCALE */
   radeon_emit(cmd_buffer->cs, fui(d->vk.rs.depth_bias.constant)); /* FRONT OFFSET */
   radeon_emit(cmd_buffer->cs, slope);                             /* BACK SCALE */
   radeon_emit(cmd_buffer->cs, fui(d->vk.rs.depth_bias.constant)); /* BACK OFFSET */

   radeon_set_context_reg(cmd_buffer->cs, R_028B78_PA_SU_POLY_OFFSET_DB_FMT_CNTL, pa_su_poly_offset_db_fmt_cntl);
}

static void
radv_emit_line_stipple(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   /* GFX9 chips fail linestrip CTS tests unless this is set to 0 = no reset */
   uint32_t auto_reset_cntl = (gfx_level == GFX9) ? 0 : 2;

   if (radv_primitive_topology_is_line_list(d->vk.ia.primitive_topology))
      auto_reset_cntl = 1;

   radeon_set_context_reg(cmd_buffer->cs, R_028A0C_PA_SC_LINE_STIPPLE,
                          S_028A0C_LINE_PATTERN(d->vk.rs.line.stipple.pattern) |
                             S_028A0C_REPEAT_COUNT(d->vk.rs.line.stipple.factor - 1) |
                             S_028A0C_AUTO_RESET_CNTL(auto_reset_cntl));
}

static uint32_t
radv_get_pa_su_sc_mode_cntl(const struct radv_cmd_buffer *cmd_buffer)
{
   enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned pa_su_sc_mode_cntl;

   pa_su_sc_mode_cntl =
      S_028814_CULL_FRONT(!!(d->vk.rs.cull_mode & VK_CULL_MODE_FRONT_BIT)) |
      S_028814_CULL_BACK(!!(d->vk.rs.cull_mode & VK_CULL_MODE_BACK_BIT)) | S_028814_FACE(d->vk.rs.front_face) |
      S_028814_POLY_OFFSET_FRONT_ENABLE(d->vk.rs.depth_bias.enable) |
      S_028814_POLY_OFFSET_BACK_ENABLE(d->vk.rs.depth_bias.enable) |
      S_028814_POLY_OFFSET_PARA_ENABLE(d->vk.rs.depth_bias.enable) |
      S_028814_POLY_MODE(d->vk.rs.polygon_mode != V_028814_X_DRAW_TRIANGLES) |
      S_028814_POLYMODE_FRONT_PTYPE(d->vk.rs.polygon_mode) | S_028814_POLYMODE_BACK_PTYPE(d->vk.rs.polygon_mode) |
      S_028814_PROVOKING_VTX_LAST(d->vk.rs.provoking_vertex == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT);

   if (gfx_level >= GFX10) {
      /* Ensure that SC processes the primitive group in the same order as PA produced them.  Needed
       * when either POLY_MODE or PERPENDICULAR_ENDCAP_ENA is set.
       */
      pa_su_sc_mode_cntl |=
         S_028814_KEEP_TOGETHER_ENABLE(d->vk.rs.polygon_mode != V_028814_X_DRAW_TRIANGLES ||
                                       d->vk.rs.line.mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT);
   }

   return pa_su_sc_mode_cntl;
}

static void
radv_emit_culling(struct radv_cmd_buffer *cmd_buffer)
{
   unsigned pa_su_sc_mode_cntl = radv_get_pa_su_sc_mode_cntl(cmd_buffer);

   radeon_set_context_reg(cmd_buffer->cs, R_028814_PA_SU_SC_MODE_CNTL, pa_su_sc_mode_cntl);
}

static void
radv_emit_provoking_vertex_mode(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *last_vgt_shader = cmd_buffer->state.last_vgt_shader;
   const unsigned stage = last_vgt_shader->info.stage;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   const struct radv_userdata_info *loc = radv_get_user_sgpr(last_vgt_shader, AC_UD_NGG_PROVOKING_VTX);
   unsigned provoking_vtx = 0;
   uint32_t base_reg;

   if (loc->sgpr_idx == -1)
      return;

   if (d->vk.rs.provoking_vertex == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT) {
      if (stage == MESA_SHADER_VERTEX) {
         provoking_vtx = radv_conv_prim_to_gs_out(d->vk.ia.primitive_topology, last_vgt_shader->info.is_ngg);
      } else {
         assert(stage == MESA_SHADER_GEOMETRY);
         provoking_vtx = last_vgt_shader->info.gs.vertices_in - 1;
      }
   }

   base_reg = last_vgt_shader->info.user_data_0;
   radeon_set_sh_reg(cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, provoking_vtx);
}

static void
radv_emit_primitive_topology(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *last_vgt_shader = cmd_buffer->state.last_vgt_shader;
   const struct radv_userdata_info *loc = radv_get_user_sgpr(last_vgt_shader, AC_UD_NUM_VERTS_PER_PRIM);
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   uint32_t base_reg;

   assert(!cmd_buffer->state.mesh_shading);

   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7) {
      radeon_set_uconfig_reg_idx(cmd_buffer->device->physical_device, cmd_buffer->cs, R_030908_VGT_PRIMITIVE_TYPE, 1,
                                 d->vk.ia.primitive_topology);
   } else {
      radeon_set_config_reg(cmd_buffer->cs, R_008958_VGT_PRIMITIVE_TYPE, d->vk.ia.primitive_topology);
   }

   if (loc->sgpr_idx == -1)
      return;

   base_reg = last_vgt_shader->info.user_data_0;
   radeon_set_sh_reg(cmd_buffer->cs, base_reg + loc->sgpr_idx * 4,
                     radv_conv_prim_to_gs_out(d->vk.ia.primitive_topology, last_vgt_shader->info.is_ngg) + 1);
}

static void
radv_emit_depth_control(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   const bool stencil_test_enable =
      d->vk.ds.stencil.test_enable && (render->ds_att_aspects & VK_IMAGE_ASPECT_STENCIL_BIT);

   radeon_set_context_reg(
      cmd_buffer->cs, R_028800_DB_DEPTH_CONTROL,
      S_028800_Z_ENABLE(d->vk.ds.depth.test_enable ? 1 : 0) |
         S_028800_Z_WRITE_ENABLE(d->vk.ds.depth.write_enable ? 1 : 0) | S_028800_ZFUNC(d->vk.ds.depth.compare_op) |
         S_028800_DEPTH_BOUNDS_ENABLE(d->vk.ds.depth.bounds_test.enable ? 1 : 0) |
         S_028800_STENCIL_ENABLE(stencil_test_enable) | S_028800_BACKFACE_ENABLE(stencil_test_enable) |
         S_028800_STENCILFUNC(d->vk.ds.stencil.front.op.compare) |
         S_028800_STENCILFUNC_BF(d->vk.ds.stencil.back.op.compare));
}

static void
radv_emit_stencil_control(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   radeon_set_context_reg(cmd_buffer->cs, R_02842C_DB_STENCIL_CONTROL,
                          S_02842C_STENCILFAIL(radv_translate_stencil_op(d->vk.ds.stencil.front.op.fail)) |
                             S_02842C_STENCILZPASS(radv_translate_stencil_op(d->vk.ds.stencil.front.op.pass)) |
                             S_02842C_STENCILZFAIL(radv_translate_stencil_op(d->vk.ds.stencil.front.op.depth_fail)) |
                             S_02842C_STENCILFAIL_BF(radv_translate_stencil_op(d->vk.ds.stencil.back.op.fail)) |
                             S_02842C_STENCILZPASS_BF(radv_translate_stencil_op(d->vk.ds.stencil.back.op.pass)) |
                             S_02842C_STENCILZFAIL_BF(radv_translate_stencil_op(d->vk.ds.stencil.back.op.depth_fail)));
}

static bool
radv_should_force_vrs1x1(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const struct radv_shader *ps = cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT];

   return pdevice->rad_info.gfx_level >= GFX10_3 &&
          (cmd_buffer->state.ms.sample_shading_enable || (ps && ps->info.ps.force_sample_iter_shading_rate));
}

static void
radv_emit_fragment_shading_rate(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   /* When per-vertex VRS is forced and the dynamic fragment shading rate is a no-op, ignore
    * it. This is needed for vkd3d-proton because it always declares per-draw VRS as dynamic.
    */
   if (cmd_buffer->device->force_vrs != RADV_FORCE_VRS_1x1 && d->vk.fsr.fragment_size.width == 1 &&
       d->vk.fsr.fragment_size.height == 1 &&
       d->vk.fsr.combiner_ops[0] == VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
       d->vk.fsr.combiner_ops[1] == VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR)
      return;

   uint32_t rate_x = MIN2(2, d->vk.fsr.fragment_size.width) - 1;
   uint32_t rate_y = MIN2(2, d->vk.fsr.fragment_size.height) - 1;
   uint32_t pipeline_comb_mode = d->vk.fsr.combiner_ops[0];
   uint32_t htile_comb_mode = d->vk.fsr.combiner_ops[1];
   uint32_t pa_cl_vrs_cntl = 0;

   assert(cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10_3);

   if (!cmd_buffer->state.render.vrs_att.iview) {
      /* When the current subpass has no VRS attachment, the VRS rates are expected to be 1x1, so we
       * can cheat by tweaking the different combiner modes.
       */
      switch (htile_comb_mode) {
      case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MIN_KHR:
         /* The result of min(A, 1x1) is always 1x1. */
         FALLTHROUGH;
      case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR:
         /* Force the per-draw VRS rate to 1x1. */
         rate_x = rate_y = 0;

         /* As the result of min(A, 1x1) or replace(A, 1x1) are always 1x1, set the vertex rate
          * combiner mode as passthrough.
          */
         pipeline_comb_mode = V_028848_SC_VRS_COMB_MODE_PASSTHRU;
         break;
      case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR:
         /* The result of max(A, 1x1) is always A. */
         FALLTHROUGH;
      case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR:
         /* Nothing to do here because the SAMPLE_ITER combiner mode should already be passthrough. */
         break;
      default:
         break;
      }
   }

   /* Emit per-draw VRS rate which is the first combiner. */
   radeon_set_uconfig_reg(cmd_buffer->cs, R_03098C_GE_VRS_RATE, S_03098C_RATE_X(rate_x) | S_03098C_RATE_Y(rate_y));

   /* Disable VRS and use the rates from PS_ITER_SAMPLES if:
    *
    * 1) sample shading is enabled or per-sample interpolation is used by the fragment shader
    * 2) the fragment shader requires 1x1 shading rate for some other reason
    */
   if (radv_should_force_vrs1x1(cmd_buffer)) {
      pa_cl_vrs_cntl |= S_028848_SAMPLE_ITER_COMBINER_MODE(V_028848_SC_VRS_COMB_MODE_OVERRIDE);
   }

   /* VERTEX_RATE_COMBINER_MODE controls the combiner mode between the
    * draw rate and the vertex rate.
    */
   if (cmd_buffer->state.mesh_shading) {
      pa_cl_vrs_cntl |= S_028848_VERTEX_RATE_COMBINER_MODE(V_028848_SC_VRS_COMB_MODE_PASSTHRU) |
                        S_028848_PRIMITIVE_RATE_COMBINER_MODE(pipeline_comb_mode);
   } else {
      pa_cl_vrs_cntl |= S_028848_VERTEX_RATE_COMBINER_MODE(pipeline_comb_mode) |
                        S_028848_PRIMITIVE_RATE_COMBINER_MODE(V_028848_SC_VRS_COMB_MODE_PASSTHRU);
   }

   /* HTILE_RATE_COMBINER_MODE controls the combiner mode between the primitive rate and the HTILE
    * rate.
    */
   pa_cl_vrs_cntl |= S_028848_HTILE_RATE_COMBINER_MODE(htile_comb_mode);

   radeon_set_context_reg(cmd_buffer->cs, R_028848_PA_CL_VRS_CNTL, pa_cl_vrs_cntl);
}

static uint32_t
radv_get_primitive_reset_index(const struct radv_cmd_buffer *cmd_buffer)
{
   const uint32_t index_type = G_028A7C_INDEX_TYPE(cmd_buffer->state.index_type);
   switch (index_type) {
   case V_028A7C_VGT_INDEX_8:
      return 0xffu;
   case V_028A7C_VGT_INDEX_16:
      return 0xffffu;
   case V_028A7C_VGT_INDEX_32:
      return 0xffffffffu;
   default:
      unreachable("invalid index type");
   }
}

static void
radv_emit_primitive_restart_enable(struct radv_cmd_buffer *cmd_buffer)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   const struct radv_dynamic_state *const d = &cmd_buffer->state.dynamic;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const bool en = d->vk.ia.primitive_restart_enable;

   if (gfx_level >= GFX11) {
      radeon_set_uconfig_reg(cs, R_03092C_GE_MULTI_PRIM_IB_RESET_EN,
                             S_03092C_RESET_EN(en) |
                                /* This disables primitive restart for non-indexed draws.
                                 * By keeping this set, we don't have to unset RESET_EN
                                 * for non-indexed draws. */
                                S_03092C_DISABLE_FOR_AUTO_INDEX(1));
   } else if (gfx_level >= GFX9) {
      radeon_set_uconfig_reg(cs, R_03092C_VGT_MULTI_PRIM_IB_RESET_EN, en);
   } else {
      radeon_set_context_reg(cs, R_028A94_VGT_MULTI_PRIM_IB_RESET_EN, en);
   }

   /* GFX6-7: All 32 bits are compared.
    * GFX8: Only index type bits are compared.
    * GFX9+: Default is same as GFX8, MATCH_ALL_BITS=1 selects GFX6-7 behavior
    */
   if (en && gfx_level <= GFX7) {
      const uint32_t primitive_reset_index = radv_get_primitive_reset_index(cmd_buffer);

      if (primitive_reset_index != cmd_buffer->state.last_primitive_reset_index) {
         radeon_set_context_reg(cs, R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX, primitive_reset_index);
         cmd_buffer->state.last_primitive_reset_index = primitive_reset_index;
      }
   }
}

static void
radv_emit_clipping(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   bool depth_clip_enable = radv_get_depth_clip_enable(cmd_buffer);

   radeon_set_context_reg(
      cmd_buffer->cs, R_028810_PA_CL_CLIP_CNTL,
      S_028810_DX_RASTERIZATION_KILL(d->vk.rs.rasterizer_discard_enable) |
         S_028810_ZCLIP_NEAR_DISABLE(!depth_clip_enable) | S_028810_ZCLIP_FAR_DISABLE(!depth_clip_enable) |
         S_028810_DX_CLIP_SPACE_DEF(!d->vk.vp.depth_clip_negative_one_to_one) | S_028810_DX_LINEAR_ATTR_CLIP_ENA(1));
}

static bool
radv_is_mrt0_dual_src(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   if (!d->vk.cb.attachments[0].write_mask || !d->vk.cb.attachments[0].blend_enable)
      return false;

   return radv_can_enable_dual_src(&d->vk.cb.attachments[0]);
}

static void
radv_emit_logic_op(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned cb_color_control = 0;

   if (d->vk.cb.logic_op_enable) {
      cb_color_control |= S_028808_ROP3(d->vk.cb.logic_op);
   } else {
      cb_color_control |= S_028808_ROP3(V_028808_ROP3_COPY);
   }

   if (cmd_buffer->device->physical_device->rad_info.has_rbplus) {
      /* RB+ doesn't work with dual source blending, logic op and CB_RESOLVE. */
      bool mrt0_is_dual_src = radv_is_mrt0_dual_src(cmd_buffer);

      cb_color_control |= S_028808_DISABLE_DUAL_QUAD(mrt0_is_dual_src || d->vk.cb.logic_op_enable ||
                                                     cmd_buffer->state.custom_blend_mode == V_028808_CB_RESOLVE);
   }

   if (cmd_buffer->state.custom_blend_mode) {
      cb_color_control |= S_028808_MODE(cmd_buffer->state.custom_blend_mode);
   } else {
      bool color_write_enabled = false;

      for (unsigned i = 0; i < MAX_RTS; i++) {
         if (d->vk.cb.attachments[i].write_mask) {
            color_write_enabled = true;
            break;
         }
      }

      if (color_write_enabled) {
         cb_color_control |= S_028808_MODE(V_028808_CB_NORMAL);
      } else {
         cb_color_control |= S_028808_MODE(V_028808_CB_DISABLE);
      }
   }

   radeon_set_context_reg(cmd_buffer->cs, R_028808_CB_COLOR_CONTROL, cb_color_control);
}

static void
radv_emit_color_write(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_device *device = cmd_buffer->device;
   const struct radv_binning_settings *settings = &device->physical_device->binning_settings;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   uint32_t color_write_enable = 0, color_write_mask = 0;

   u_foreach_bit (i, d->vk.cb.color_write_enables) {
      color_write_enable |= 0xfu << (i * 4);
   }

   for (unsigned i = 0; i < MAX_RTS; i++) {
      color_write_mask |= d->vk.cb.attachments[i].write_mask << (4 * i);
   }

   if (device->pbb_allowed && settings->context_states_per_bin > 1) {
      /* Flush DFSM on CB_TARGET_MASK changes. */
      radeon_emit(cmd_buffer->cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cmd_buffer->cs, EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
   }

   radeon_set_context_reg(cmd_buffer->cs, R_028238_CB_TARGET_MASK, color_write_mask & color_write_enable);
}

static void
radv_emit_patch_control_points(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const struct radv_shader *vs = radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_VERTEX);
   const struct radv_shader *tcs = cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL];
   const struct radv_shader *tes = radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_TESS_EVAL);
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned ls_hs_config, base_reg;

   /* Compute tessellation info that depends on the number of patch control points when this state
    * is dynamic.
    */
   if (cmd_buffer->state.uses_dynamic_patch_control_points) {
      /* Compute the number of patches. */
      cmd_buffer->state.tess_num_patches = get_tcs_num_patches(
         d->vk.ts.patch_control_points, tcs->info.tcs.tcs_vertices_out, vs->info.vs.num_linked_outputs,
         tcs->info.tcs.num_linked_outputs, tcs->info.tcs.num_linked_patch_outputs,
         pdevice->hs.tess_offchip_block_dw_size, pdevice->rad_info.gfx_level, pdevice->rad_info.family);

      /* Compute the LDS size. */
      cmd_buffer->state.tess_lds_size = calculate_tess_lds_size(
         pdevice->rad_info.gfx_level, d->vk.ts.patch_control_points, tcs->info.tcs.tcs_vertices_out,
         vs->info.vs.num_linked_outputs, cmd_buffer->state.tess_num_patches, tcs->info.tcs.num_linked_outputs,
         tcs->info.tcs.num_linked_patch_outputs);
   }

   ls_hs_config = S_028B58_NUM_PATCHES(cmd_buffer->state.tess_num_patches) |
                  S_028B58_HS_NUM_INPUT_CP(d->vk.ts.patch_control_points) |
                  S_028B58_HS_NUM_OUTPUT_CP(tcs->info.tcs.tcs_vertices_out);

   if (pdevice->rad_info.gfx_level >= GFX7) {
      radeon_set_context_reg_idx(cmd_buffer->cs, R_028B58_VGT_LS_HS_CONFIG, 2, ls_hs_config);
   } else {
      radeon_set_context_reg(cmd_buffer->cs, R_028B58_VGT_LS_HS_CONFIG, ls_hs_config);
   }

   if (pdevice->rad_info.gfx_level >= GFX9) {
      unsigned hs_rsrc2 = tcs->config.rsrc2;

      if (pdevice->rad_info.gfx_level >= GFX10) {
         hs_rsrc2 |= S_00B42C_LDS_SIZE_GFX10(cmd_buffer->state.tess_lds_size);
      } else {
         hs_rsrc2 |= S_00B42C_LDS_SIZE_GFX9(cmd_buffer->state.tess_lds_size);
      }

      radeon_set_sh_reg(cmd_buffer->cs, R_00B42C_SPI_SHADER_PGM_RSRC2_HS, hs_rsrc2);
   } else {
      unsigned ls_rsrc2 = vs->config.rsrc2 | S_00B52C_LDS_SIZE(cmd_buffer->state.tess_lds_size);

      radeon_set_sh_reg(cmd_buffer->cs, R_00B52C_SPI_SHADER_PGM_RSRC2_LS, ls_rsrc2);
   }

   /* Emit user SGPRs for dynamic patch control points. */
   const struct radv_userdata_info *offchip = radv_get_user_sgpr(tcs, AC_UD_TCS_OFFCHIP_LAYOUT);
   if (offchip->sgpr_idx == -1)
      return;
   assert(offchip->num_sgprs == 1);

   unsigned tcs_offchip_layout =
      SET_SGPR_FIELD(TCS_OFFCHIP_LAYOUT_PATCH_CONTROL_POINTS, d->vk.ts.patch_control_points) |
      SET_SGPR_FIELD(TCS_OFFCHIP_LAYOUT_NUM_PATCHES, cmd_buffer->state.tess_num_patches) |
      SET_SGPR_FIELD(TCS_OFFCHIP_LAYOUT_LSHS_VERTEX_STRIDE,
                     get_tcs_input_vertex_stride(vs->info.vs.num_linked_outputs) / 4);

   base_reg = tcs->info.user_data_0;
   radeon_set_sh_reg(cmd_buffer->cs, base_reg + offchip->sgpr_idx * 4, tcs_offchip_layout);

   const struct radv_userdata_info *num_patches = radv_get_user_sgpr(tes, AC_UD_TES_STATE);
   assert(num_patches->sgpr_idx != -1 && num_patches->num_sgprs == 1);

   const unsigned tes_state = SET_SGPR_FIELD(TES_STATE_NUM_PATCHES, cmd_buffer->state.tess_num_patches) |
                              SET_SGPR_FIELD(TES_STATE_TCS_VERTICES_OUT, tcs->info.tcs.tcs_vertices_out) |
                              SET_SGPR_FIELD(TES_STATE_NUM_TCS_OUTPUTS, tcs->info.tcs.num_linked_outputs);

   base_reg = tes->info.user_data_0;
   radeon_set_sh_reg(cmd_buffer->cs, base_reg + num_patches->sgpr_idx * 4, tes_state);
}

static void
radv_emit_conservative_rast_mode(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   if (pdevice->rad_info.gfx_level >= GFX9) {
      uint32_t pa_sc_conservative_rast;

      if (d->vk.rs.conservative_mode != VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT) {
         const struct radv_shader *ps = cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT];
         const bool uses_inner_coverage = ps && ps->info.ps.reads_fully_covered;

         pa_sc_conservative_rast =
            S_028C4C_PREZ_AA_MASK_ENABLE(1) | S_028C4C_POSTZ_AA_MASK_ENABLE(1) | S_028C4C_CENTROID_SAMPLE_OVERRIDE(1);

         /* Inner coverage requires underestimate conservative rasterization. */
         if (d->vk.rs.conservative_mode == VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT &&
             !uses_inner_coverage) {
            pa_sc_conservative_rast |= S_028C4C_OVER_RAST_ENABLE(1) | S_028C4C_UNDER_RAST_SAMPLE_SELECT(1) |
                                       S_028C4C_PBB_UNCERTAINTY_REGION_ENABLE(1);
         } else {
            pa_sc_conservative_rast |= S_028C4C_OVER_RAST_SAMPLE_SELECT(1) | S_028C4C_UNDER_RAST_ENABLE(1);
         }
      } else {
         pa_sc_conservative_rast = S_028C4C_NULL_SQUAD_AA_MASK_ENABLE(1);
      }

      radeon_set_context_reg(cmd_buffer->cs, R_028C4C_PA_SC_CONSERVATIVE_RASTERIZATION_CNTL, pa_sc_conservative_rast);
   }
}

static void
radv_emit_depth_clamp_enable(struct radv_cmd_buffer *cmd_buffer)
{
   enum radv_depth_clamp_mode mode = radv_get_depth_clamp_mode(cmd_buffer);

   radeon_set_context_reg(cmd_buffer->cs, R_02800C_DB_RENDER_OVERRIDE,
                          S_02800C_FORCE_HIS_ENABLE0(V_02800C_FORCE_DISABLE) |
                             S_02800C_FORCE_HIS_ENABLE1(V_02800C_FORCE_DISABLE) |
                             S_02800C_DISABLE_VIEWPORT_CLAMP(mode == RADV_DEPTH_CLAMP_MODE_DISABLED));
}

static void
radv_emit_rasterization_samples(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   unsigned rasterization_samples = radv_get_rasterization_samples(cmd_buffer);
   unsigned ps_iter_samples = radv_get_ps_iter_samples(cmd_buffer);
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned spi_baryc_cntl = S_0286E0_FRONT_FACE_ALL_BITS(1);
   unsigned pa_sc_mode_cntl_1;

   pa_sc_mode_cntl_1 =
      S_028A4C_WALK_FENCE_ENABLE(1) | // TODO linear dst fixes
      S_028A4C_WALK_FENCE_SIZE(pdevice->rad_info.num_tile_pipes == 2 ? 2 : 3) |
      S_028A4C_OUT_OF_ORDER_PRIMITIVE_ENABLE(cmd_buffer->state.uses_out_of_order_rast) |
      S_028A4C_OUT_OF_ORDER_WATER_MARK(0x7) |
      /* always 1: */
      S_028A4C_SUPERTILE_WALK_ORDER_ENABLE(1) | S_028A4C_TILE_WALK_ORDER_ENABLE(1) |
      S_028A4C_MULTI_SHADER_ENGINE_PRIM_DISCARD_ENABLE(1) | S_028A4C_FORCE_EOV_CNTDWN_ENABLE(1) |
      S_028A4C_FORCE_EOV_REZ_ENABLE(1) |
      /* This should only be set when VRS surfaces aren't enabled on GFX11, otherwise the GPU might
       * hang.
       */
      S_028A4C_WALK_ALIGN8_PRIM_FITS_ST(pdevice->rad_info.gfx_level < GFX11 || !cmd_buffer->state.uses_vrs_attachment);

   if (!d->sample_location.count)
      radv_emit_default_sample_locations(cmd_buffer->cs, rasterization_samples);

   if (ps_iter_samples > 1) {
      spi_baryc_cntl |= S_0286E0_POS_FLOAT_LOCATION(2);
      pa_sc_mode_cntl_1 |= S_028A4C_PS_ITER_SAMPLE(1);
   }

   if (radv_should_force_vrs1x1(cmd_buffer)) {
      /* Make sure sample shading is enabled even if only MSAA1x is used because the SAMPLE_ITER
       * combiner is in passthrough mode if PS_ITER_SAMPLE is 0, and it uses the per-draw rate. The
       * default VRS rate when sample shading is enabled is 1x1.
       */
      if (!G_028A4C_PS_ITER_SAMPLE(pa_sc_mode_cntl_1))
         pa_sc_mode_cntl_1 |= S_028A4C_PS_ITER_SAMPLE(1);
   }

   radeon_set_context_reg(cmd_buffer->cs, R_0286E0_SPI_BARYC_CNTL, spi_baryc_cntl);
   radeon_set_context_reg(cmd_buffer->cs, R_028A4C_PA_SC_MODE_CNTL_1, pa_sc_mode_cntl_1);
}

static void
radv_emit_fb_color_state(struct radv_cmd_buffer *cmd_buffer, int index, struct radv_color_buffer_info *cb,
                         struct radv_image_view *iview, VkImageLayout layout)
{
   bool is_vi = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX8;
   uint32_t cb_fdcc_control = cb->cb_dcc_control;
   uint32_t cb_color_info = cb->cb_color_info;
   struct radv_image *image = iview->image;

   if (!radv_layout_dcc_compressed(cmd_buffer->device, image, iview->vk.base_mip_level, layout,
                                   radv_image_queue_family_mask(image, cmd_buffer->qf, cmd_buffer->qf))) {
      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
         cb_fdcc_control &= C_028C78_FDCC_ENABLE;
      } else {
         cb_color_info &= C_028C70_DCC_ENABLE;
      }
   }

   const enum radv_fmask_compression fmask_comp = radv_layout_fmask_compression(
      cmd_buffer->device, image, layout, radv_image_queue_family_mask(image, cmd_buffer->qf, cmd_buffer->qf));
   if (fmask_comp == RADV_FMASK_COMPRESSION_NONE) {
      cb_color_info &= C_028C70_COMPRESSION;
   }

   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028C6C_CB_COLOR0_VIEW + index * 0x3c, 4);
      radeon_emit(cmd_buffer->cs, cb->cb_color_view);   /* CB_COLOR0_VIEW */
      radeon_emit(cmd_buffer->cs, cb->cb_color_info);   /* CB_COLOR0_INFO */
      radeon_emit(cmd_buffer->cs, cb->cb_color_attrib); /* CB_COLOR0_ATTRIB */
      radeon_emit(cmd_buffer->cs, cb_fdcc_control);     /* CB_COLOR0_FDCC_CONTROL */

      radeon_set_context_reg(cmd_buffer->cs, R_028C60_CB_COLOR0_BASE + index * 0x3c, cb->cb_color_base);
      radeon_set_context_reg(cmd_buffer->cs, R_028E40_CB_COLOR0_BASE_EXT + index * 4, cb->cb_color_base >> 32);
      radeon_set_context_reg(cmd_buffer->cs, R_028C94_CB_COLOR0_DCC_BASE + index * 0x3c, cb->cb_dcc_base);
      radeon_set_context_reg(cmd_buffer->cs, R_028EA0_CB_COLOR0_DCC_BASE_EXT + index * 4, cb->cb_dcc_base >> 32);
      radeon_set_context_reg(cmd_buffer->cs, R_028EC0_CB_COLOR0_ATTRIB2 + index * 4, cb->cb_color_attrib2);
      radeon_set_context_reg(cmd_buffer->cs, R_028EE0_CB_COLOR0_ATTRIB3 + index * 4, cb->cb_color_attrib3);
   } else if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10) {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028C60_CB_COLOR0_BASE + index * 0x3c, 11);
      radeon_emit(cmd_buffer->cs, cb->cb_color_base);
      radeon_emit(cmd_buffer->cs, 0);
      radeon_emit(cmd_buffer->cs, 0);
      radeon_emit(cmd_buffer->cs, cb->cb_color_view);
      radeon_emit(cmd_buffer->cs, cb_color_info);
      radeon_emit(cmd_buffer->cs, cb->cb_color_attrib);
      radeon_emit(cmd_buffer->cs, cb->cb_dcc_control);
      radeon_emit(cmd_buffer->cs, cb->cb_color_cmask);
      radeon_emit(cmd_buffer->cs, 0);
      radeon_emit(cmd_buffer->cs, cb->cb_color_fmask);
      radeon_emit(cmd_buffer->cs, 0);

      radeon_set_context_reg(cmd_buffer->cs, R_028C94_CB_COLOR0_DCC_BASE + index * 0x3c, cb->cb_dcc_base);

      radeon_set_context_reg(cmd_buffer->cs, R_028E40_CB_COLOR0_BASE_EXT + index * 4, cb->cb_color_base >> 32);
      radeon_set_context_reg(cmd_buffer->cs, R_028E60_CB_COLOR0_CMASK_BASE_EXT + index * 4, cb->cb_color_cmask >> 32);
      radeon_set_context_reg(cmd_buffer->cs, R_028E80_CB_COLOR0_FMASK_BASE_EXT + index * 4, cb->cb_color_fmask >> 32);
      radeon_set_context_reg(cmd_buffer->cs, R_028EA0_CB_COLOR0_DCC_BASE_EXT + index * 4, cb->cb_dcc_base >> 32);
      radeon_set_context_reg(cmd_buffer->cs, R_028EC0_CB_COLOR0_ATTRIB2 + index * 4, cb->cb_color_attrib2);
      radeon_set_context_reg(cmd_buffer->cs, R_028EE0_CB_COLOR0_ATTRIB3 + index * 4, cb->cb_color_attrib3);
   } else if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX9) {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028C60_CB_COLOR0_BASE + index * 0x3c, 11);
      radeon_emit(cmd_buffer->cs, cb->cb_color_base);
      radeon_emit(cmd_buffer->cs, S_028C64_BASE_256B(cb->cb_color_base >> 32));
      radeon_emit(cmd_buffer->cs, cb->cb_color_attrib2);
      radeon_emit(cmd_buffer->cs, cb->cb_color_view);
      radeon_emit(cmd_buffer->cs, cb_color_info);
      radeon_emit(cmd_buffer->cs, cb->cb_color_attrib);
      radeon_emit(cmd_buffer->cs, cb->cb_dcc_control);
      radeon_emit(cmd_buffer->cs, cb->cb_color_cmask);
      radeon_emit(cmd_buffer->cs, S_028C80_BASE_256B(cb->cb_color_cmask >> 32));
      radeon_emit(cmd_buffer->cs, cb->cb_color_fmask);
      radeon_emit(cmd_buffer->cs, S_028C88_BASE_256B(cb->cb_color_fmask >> 32));

      radeon_set_context_reg_seq(cmd_buffer->cs, R_028C94_CB_COLOR0_DCC_BASE + index * 0x3c, 2);
      radeon_emit(cmd_buffer->cs, cb->cb_dcc_base);
      radeon_emit(cmd_buffer->cs, S_028C98_BASE_256B(cb->cb_dcc_base >> 32));

      radeon_set_context_reg(cmd_buffer->cs, R_0287A0_CB_MRT0_EPITCH + index * 4, cb->cb_mrt_epitch);
   } else {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028C60_CB_COLOR0_BASE + index * 0x3c, 11);
      radeon_emit(cmd_buffer->cs, cb->cb_color_base);
      radeon_emit(cmd_buffer->cs, cb->cb_color_pitch);
      radeon_emit(cmd_buffer->cs, cb->cb_color_slice);
      radeon_emit(cmd_buffer->cs, cb->cb_color_view);
      radeon_emit(cmd_buffer->cs, cb_color_info);
      radeon_emit(cmd_buffer->cs, cb->cb_color_attrib);
      radeon_emit(cmd_buffer->cs, cb->cb_dcc_control);
      radeon_emit(cmd_buffer->cs, cb->cb_color_cmask);
      radeon_emit(cmd_buffer->cs, cb->cb_color_cmask_slice);
      radeon_emit(cmd_buffer->cs, cb->cb_color_fmask);
      radeon_emit(cmd_buffer->cs, cb->cb_color_fmask_slice);

      if (is_vi) { /* DCC BASE */
         radeon_set_context_reg(cmd_buffer->cs, R_028C94_CB_COLOR0_DCC_BASE + index * 0x3c, cb->cb_dcc_base);
      }
   }

   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11 ? G_028C78_FDCC_ENABLE(cb_fdcc_control)
                                                                        : G_028C70_DCC_ENABLE(cb_color_info)) {
      /* Drawing with DCC enabled also compresses colorbuffers. */
      VkImageSubresourceRange range = {
         .aspectMask = iview->vk.aspects,
         .baseMipLevel = iview->vk.base_mip_level,
         .levelCount = iview->vk.level_count,
         .baseArrayLayer = iview->vk.base_array_layer,
         .layerCount = iview->vk.layer_count,
      };

      radv_update_dcc_metadata(cmd_buffer, image, &range, true);
   }
}

static void
radv_update_zrange_precision(struct radv_cmd_buffer *cmd_buffer, struct radv_ds_buffer_info *ds,
                             const struct radv_image_view *iview, bool requires_cond_exec)
{
   const struct radv_image *image = iview->image;
   uint32_t db_z_info = ds->db_z_info;
   uint32_t db_z_info_reg;

   if (!cmd_buffer->device->physical_device->rad_info.has_tc_compat_zrange_bug || !radv_image_is_tc_compat_htile(image))
      return;

   db_z_info &= C_028040_ZRANGE_PRECISION;

   if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX9) {
      db_z_info_reg = R_028038_DB_Z_INFO;
   } else {
      db_z_info_reg = R_028040_DB_Z_INFO;
   }

   /* When we don't know the last fast clear value we need to emit a
    * conditional packet that will eventually skip the following
    * SET_CONTEXT_REG packet.
    */
   if (requires_cond_exec) {
      uint64_t va = radv_get_tc_compat_zrange_va(image, iview->vk.base_mip_level);

      radeon_emit(cmd_buffer->cs, PKT3(PKT3_COND_EXEC, 3, 0));
      radeon_emit(cmd_buffer->cs, va);
      radeon_emit(cmd_buffer->cs, va >> 32);
      radeon_emit(cmd_buffer->cs, 0);
      radeon_emit(cmd_buffer->cs, 3); /* SET_CONTEXT_REG size */
   }

   radeon_set_context_reg(cmd_buffer->cs, db_z_info_reg, db_z_info);
}

static struct radv_image *
radv_cmd_buffer_get_vrs_image(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_device *device = cmd_buffer->device;

   if (!device->vrs.image) {
      VkResult result;

      /* The global VRS state is initialized on-demand to avoid wasting VRAM. */
      result = radv_device_init_vrs_state(device);
      if (result != VK_SUCCESS) {
         vk_command_buffer_set_error(&cmd_buffer->vk, result);
         return NULL;
      }
   }

   return device->vrs.image;
}

static void
radv_emit_fb_ds_state(struct radv_cmd_buffer *cmd_buffer, struct radv_ds_buffer_info *ds, struct radv_image_view *iview,
                      bool depth_compressed, bool stencil_compressed)
{
   uint64_t db_htile_data_base = ds->db_htile_data_base;
   uint32_t db_htile_surface = ds->db_htile_surface;
   uint32_t db_render_control = ds->db_render_control | cmd_buffer->state.db_render_control;
   uint32_t db_z_info = ds->db_z_info;

   if (!depth_compressed)
      db_render_control |= S_028000_DEPTH_COMPRESS_DISABLE(1);
   if (!stencil_compressed)
      db_render_control |= S_028000_STENCIL_COMPRESS_DISABLE(1);

   if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX10_3) {
      if (!cmd_buffer->state.render.vrs_att.iview) {
         db_htile_surface &= C_028ABC_VRS_HTILE_ENCODING;
      } else {
         /* On GFX10.3, when a subpass uses VRS attachment but HTILE can't be enabled, we fallback to
          * our internal HTILE buffer.
          */
         if (!radv_htile_enabled(iview->image, iview->vk.base_mip_level) && radv_cmd_buffer_get_vrs_image(cmd_buffer)) {
            struct radv_buffer *htile_buffer = cmd_buffer->device->vrs.buffer;

            assert(!G_028038_TILE_SURFACE_ENABLE(db_z_info) && !db_htile_data_base && !db_htile_surface);
            db_z_info |= S_028038_TILE_SURFACE_ENABLE(1);
            db_htile_data_base = radv_buffer_get_va(htile_buffer->bo) >> 8;
            db_htile_surface = S_028ABC_FULL_CACHE(1) | S_028ABC_PIPE_ALIGNED(1) |
                               S_028ABC_VRS_HTILE_ENCODING(V_028ABC_VRS_HTILE_4BIT_ENCODING);
         }
      }
   }

   radeon_set_context_reg(cmd_buffer->cs, R_028000_DB_RENDER_CONTROL, db_render_control);
   radeon_set_context_reg(cmd_buffer->cs, R_028008_DB_DEPTH_VIEW, ds->db_depth_view);
   radeon_set_context_reg(cmd_buffer->cs, R_028010_DB_RENDER_OVERRIDE2, ds->db_render_override2);
   radeon_set_context_reg(cmd_buffer->cs, R_028ABC_DB_HTILE_SURFACE, db_htile_surface);

   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10) {
      radeon_set_context_reg(cmd_buffer->cs, R_028014_DB_HTILE_DATA_BASE, db_htile_data_base);
      radeon_set_context_reg(cmd_buffer->cs, R_02801C_DB_DEPTH_SIZE_XY, ds->db_depth_size);

      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
         radeon_set_context_reg_seq(cmd_buffer->cs, R_028040_DB_Z_INFO, 6);
      } else {
         radeon_set_context_reg_seq(cmd_buffer->cs, R_02803C_DB_DEPTH_INFO, 7);
         radeon_emit(cmd_buffer->cs, S_02803C_RESOURCE_LEVEL(1));
      }
      radeon_emit(cmd_buffer->cs, db_z_info);
      radeon_emit(cmd_buffer->cs, ds->db_stencil_info);
      radeon_emit(cmd_buffer->cs, ds->db_z_read_base);
      radeon_emit(cmd_buffer->cs, ds->db_stencil_read_base);
      radeon_emit(cmd_buffer->cs, ds->db_z_read_base);
      radeon_emit(cmd_buffer->cs, ds->db_stencil_read_base);

      radeon_set_context_reg_seq(cmd_buffer->cs, R_028068_DB_Z_READ_BASE_HI, 5);
      radeon_emit(cmd_buffer->cs, ds->db_z_read_base >> 32);
      radeon_emit(cmd_buffer->cs, ds->db_stencil_read_base >> 32);
      radeon_emit(cmd_buffer->cs, ds->db_z_read_base >> 32);
      radeon_emit(cmd_buffer->cs, ds->db_stencil_read_base >> 32);
      radeon_emit(cmd_buffer->cs, db_htile_data_base >> 32);
   } else if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX9) {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028014_DB_HTILE_DATA_BASE, 3);
      radeon_emit(cmd_buffer->cs, db_htile_data_base);
      radeon_emit(cmd_buffer->cs, S_028018_BASE_HI(db_htile_data_base >> 32));
      radeon_emit(cmd_buffer->cs, ds->db_depth_size);

      radeon_set_context_reg_seq(cmd_buffer->cs, R_028038_DB_Z_INFO, 10);
      radeon_emit(cmd_buffer->cs, db_z_info);                                         /* DB_Z_INFO */
      radeon_emit(cmd_buffer->cs, ds->db_stencil_info);                               /* DB_STENCIL_INFO */
      radeon_emit(cmd_buffer->cs, ds->db_z_read_base);                                /* DB_Z_READ_BASE */
      radeon_emit(cmd_buffer->cs, S_028044_BASE_HI(ds->db_z_read_base >> 32));        /* DB_Z_READ_BASE_HI */
      radeon_emit(cmd_buffer->cs, ds->db_stencil_read_base);                          /* DB_STENCIL_READ_BASE */
      radeon_emit(cmd_buffer->cs, S_02804C_BASE_HI(ds->db_stencil_read_base >> 32));  /* DB_STENCIL_READ_BASE_HI */
      radeon_emit(cmd_buffer->cs, ds->db_z_write_base);                               /* DB_Z_WRITE_BASE */
      radeon_emit(cmd_buffer->cs, S_028054_BASE_HI(ds->db_z_write_base >> 32));       /* DB_Z_WRITE_BASE_HI */
      radeon_emit(cmd_buffer->cs, ds->db_stencil_write_base);                         /* DB_STENCIL_WRITE_BASE */
      radeon_emit(cmd_buffer->cs, S_02805C_BASE_HI(ds->db_stencil_write_base >> 32)); /* DB_STENCIL_WRITE_BASE_HI */

      radeon_set_context_reg_seq(cmd_buffer->cs, R_028068_DB_Z_INFO2, 2);
      radeon_emit(cmd_buffer->cs, ds->db_z_info2);
      radeon_emit(cmd_buffer->cs, ds->db_stencil_info2);
   } else {
      radeon_set_context_reg(cmd_buffer->cs, R_028014_DB_HTILE_DATA_BASE, db_htile_data_base);

      radeon_set_context_reg_seq(cmd_buffer->cs, R_02803C_DB_DEPTH_INFO, 9);
      radeon_emit(cmd_buffer->cs, ds->db_depth_info);         /* R_02803C_DB_DEPTH_INFO */
      radeon_emit(cmd_buffer->cs, db_z_info);                 /* R_028040_DB_Z_INFO */
      radeon_emit(cmd_buffer->cs, ds->db_stencil_info);       /* R_028044_DB_STENCIL_INFO */
      radeon_emit(cmd_buffer->cs, ds->db_z_read_base);        /* R_028048_DB_Z_READ_BASE */
      radeon_emit(cmd_buffer->cs, ds->db_stencil_read_base);  /* R_02804C_DB_STENCIL_READ_BASE */
      radeon_emit(cmd_buffer->cs, ds->db_z_write_base);       /* R_028050_DB_Z_WRITE_BASE */
      radeon_emit(cmd_buffer->cs, ds->db_stencil_write_base); /* R_028054_DB_STENCIL_WRITE_BASE */
      radeon_emit(cmd_buffer->cs, ds->db_depth_size);         /* R_028058_DB_DEPTH_SIZE */
      radeon_emit(cmd_buffer->cs, ds->db_depth_slice);        /* R_02805C_DB_DEPTH_SLICE */
   }

   /* Update the ZRANGE_PRECISION value for the TC-compat bug. */
   radv_update_zrange_precision(cmd_buffer, ds, iview, true);
}

static void
radv_emit_null_ds_state(struct radv_cmd_buffer *cmd_buffer)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   unsigned db_render_control = 0;
   unsigned num_samples = 0;

   /* On GFX11, DB_Z_INFO.NUM_SAMPLES should always match MSAA_EXPOSED_SAMPLES. It affects VRS,
    * occlusion queries and Primitive Ordered Pixel Shading if depth and stencil are not bound.
    */
   if (gfx_level == GFX11) {
      num_samples = util_logbase2(radv_get_rasterization_samples(cmd_buffer));
      radv_gfx11_set_db_render_control(cmd_buffer->device, 1, &db_render_control);
   }

   if (gfx_level == GFX9) {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028038_DB_Z_INFO, 2);
   } else {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028040_DB_Z_INFO, 2);
   }

   radeon_emit(cmd_buffer->cs, S_028040_FORMAT(V_028040_Z_INVALID) | S_028040_NUM_SAMPLES(num_samples));
   radeon_emit(cmd_buffer->cs, S_028044_FORMAT(V_028044_STENCIL_INVALID));

   radeon_set_context_reg(cmd_buffer->cs, R_028000_DB_RENDER_CONTROL, db_render_control);
   radeon_set_context_reg(cmd_buffer->cs, R_028010_DB_RENDER_OVERRIDE2,
                          S_028010_CENTROID_COMPUTATION_MODE(gfx_level >= GFX10_3));
}
/**
 * Update the fast clear depth/stencil values if the image is bound as a
 * depth/stencil buffer.
 */
static void
radv_update_bound_fast_clear_ds(struct radv_cmd_buffer *cmd_buffer, const struct radv_image_view *iview,
                                VkClearDepthStencilValue ds_clear_value, VkImageAspectFlags aspects)
{
   const struct radv_image *image = iview->image;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (cmd_buffer->state.render.ds_att.iview == NULL || cmd_buffer->state.render.ds_att.iview->image != image)
      return;

   if (aspects == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
      radeon_set_context_reg_seq(cs, R_028028_DB_STENCIL_CLEAR, 2);
      radeon_emit(cs, ds_clear_value.stencil);
      radeon_emit(cs, fui(ds_clear_value.depth));
   } else if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT) {
      radeon_set_context_reg(cs, R_02802C_DB_DEPTH_CLEAR, fui(ds_clear_value.depth));
   } else {
      assert(aspects == VK_IMAGE_ASPECT_STENCIL_BIT);
      radeon_set_context_reg(cs, R_028028_DB_STENCIL_CLEAR, ds_clear_value.stencil);
   }

   /* Update the ZRANGE_PRECISION value for the TC-compat bug. This is
    * only needed when clearing Z to 0.0.
    */
   if ((aspects & VK_IMAGE_ASPECT_DEPTH_BIT) && ds_clear_value.depth == 0.0) {
      radv_update_zrange_precision(cmd_buffer, &cmd_buffer->state.render.ds_att.ds, iview, false);
   }

   cmd_buffer->state.context_roll_without_scissor_emitted = true;
}

/**
 * Set the clear depth/stencil values to the image's metadata.
 */
static void
radv_set_ds_clear_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                           const VkImageSubresourceRange *range, VkClearDepthStencilValue ds_clear_value,
                           VkImageAspectFlags aspects)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint32_t level_count = vk_image_subresource_level_count(&image->vk, range);

   if (aspects == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
      uint64_t va = radv_get_ds_clear_value_va(image, range->baseMipLevel);

      /* Use the fastest way when both aspects are used. */
      ASSERTED unsigned cdw_end = radv_cs_write_data_head(cmd_buffer->device, cmd_buffer->cs, cmd_buffer->qf, V_370_PFP,
                                                          va, 2 * level_count, cmd_buffer->state.predicating);

      for (uint32_t l = 0; l < level_count; l++) {
         radeon_emit(cs, ds_clear_value.stencil);
         radeon_emit(cs, fui(ds_clear_value.depth));
      }

      assert(cmd_buffer->cs->cdw == cdw_end);
   } else {
      /* Otherwise we need one WRITE_DATA packet per level. */
      for (uint32_t l = 0; l < level_count; l++) {
         uint64_t va = radv_get_ds_clear_value_va(image, range->baseMipLevel + l);
         unsigned value;

         if (aspects == VK_IMAGE_ASPECT_DEPTH_BIT) {
            value = fui(ds_clear_value.depth);
            va += 4;
         } else {
            assert(aspects == VK_IMAGE_ASPECT_STENCIL_BIT);
            value = ds_clear_value.stencil;
         }

         radv_write_data(cmd_buffer, V_370_PFP, va, 1, &value, cmd_buffer->state.predicating);
      }
   }
}

/**
 * Update the TC-compat metadata value for this image.
 */
static void
radv_set_tc_compat_zrange_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                                   const VkImageSubresourceRange *range, uint32_t value)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (!cmd_buffer->device->physical_device->rad_info.has_tc_compat_zrange_bug)
      return;

   uint64_t va = radv_get_tc_compat_zrange_va(image, range->baseMipLevel);
   uint32_t level_count = vk_image_subresource_level_count(&image->vk, range);

   ASSERTED unsigned cdw_end = radv_cs_write_data_head(cmd_buffer->device, cmd_buffer->cs, cmd_buffer->qf, V_370_PFP,
                                                       va, level_count, cmd_buffer->state.predicating);

   for (uint32_t l = 0; l < level_count; l++)
      radeon_emit(cs, value);

   assert(cmd_buffer->cs->cdw == cdw_end);
}

static void
radv_update_tc_compat_zrange_metadata(struct radv_cmd_buffer *cmd_buffer, const struct radv_image_view *iview,
                                      VkClearDepthStencilValue ds_clear_value)
{
   VkImageSubresourceRange range = {
      .aspectMask = iview->vk.aspects,
      .baseMipLevel = iview->vk.base_mip_level,
      .levelCount = iview->vk.level_count,
      .baseArrayLayer = iview->vk.base_array_layer,
      .layerCount = iview->vk.layer_count,
   };
   uint32_t cond_val;

   /* Conditionally set DB_Z_INFO.ZRANGE_PRECISION to 0 when the last
    * depth clear value is 0.0f.
    */
   cond_val = ds_clear_value.depth == 0.0f ? UINT_MAX : 0;

   radv_set_tc_compat_zrange_metadata(cmd_buffer, iview->image, &range, cond_val);
}

/**
 * Update the clear depth/stencil values for this image.
 */
void
radv_update_ds_clear_metadata(struct radv_cmd_buffer *cmd_buffer, const struct radv_image_view *iview,
                              VkClearDepthStencilValue ds_clear_value, VkImageAspectFlags aspects)
{
   VkImageSubresourceRange range = {
      .aspectMask = iview->vk.aspects,
      .baseMipLevel = iview->vk.base_mip_level,
      .levelCount = iview->vk.level_count,
      .baseArrayLayer = iview->vk.base_array_layer,
      .layerCount = iview->vk.layer_count,
   };
   struct radv_image *image = iview->image;

   assert(radv_htile_enabled(image, range.baseMipLevel));

   radv_set_ds_clear_metadata(cmd_buffer, iview->image, &range, ds_clear_value, aspects);

   if (radv_image_is_tc_compat_htile(image) && (aspects & VK_IMAGE_ASPECT_DEPTH_BIT)) {
      radv_update_tc_compat_zrange_metadata(cmd_buffer, iview, ds_clear_value);
   }

   radv_update_bound_fast_clear_ds(cmd_buffer, iview, ds_clear_value, aspects);
}

/**
 * Load the clear depth/stencil values from the image's metadata.
 */
static void
radv_load_ds_clear_metadata(struct radv_cmd_buffer *cmd_buffer, const struct radv_image_view *iview)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const struct radv_image *image = iview->image;
   VkImageAspectFlags aspects = vk_format_aspects(image->vk.format);
   uint64_t va = radv_get_ds_clear_value_va(image, iview->vk.base_mip_level);
   unsigned reg_offset = 0, reg_count = 0;

   assert(radv_image_has_htile(image));

   if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      ++reg_count;
   } else {
      ++reg_offset;
      va += 4;
   }
   if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT)
      ++reg_count;

   uint32_t reg = R_028028_DB_STENCIL_CLEAR + 4 * reg_offset;

   if (cmd_buffer->device->physical_device->rad_info.has_load_ctx_reg_pkt) {
      radeon_emit(cs, PKT3(PKT3_LOAD_CONTEXT_REG_INDEX, 3, 0));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, (reg - SI_CONTEXT_REG_OFFSET) >> 2);
      radeon_emit(cs, reg_count);
   } else {
      radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_SRC_MEM) | COPY_DATA_DST_SEL(COPY_DATA_REG) |
                         (reg_count == 2 ? COPY_DATA_COUNT_SEL : 0));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, reg >> 2);
      radeon_emit(cs, 0);

      radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      radeon_emit(cs, 0);
   }
}

/*
 * With DCC some colors don't require CMASK elimination before being
 * used as a texture. This sets a predicate value to determine if the
 * cmask eliminate is required.
 */
void
radv_update_fce_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                         const VkImageSubresourceRange *range, bool value)
{
   if (!image->fce_pred_offset)
      return;

   uint64_t pred_val = value;
   uint64_t va = radv_image_get_fce_pred_va(image, range->baseMipLevel);
   uint32_t level_count = vk_image_subresource_level_count(&image->vk, range);

   ASSERTED unsigned cdw_end = radv_cs_write_data_head(cmd_buffer->device, cmd_buffer->cs, cmd_buffer->qf, V_370_PFP,
                                                       va, 2 * level_count, false);

   for (uint32_t l = 0; l < level_count; l++) {
      radeon_emit(cmd_buffer->cs, pred_val);
      radeon_emit(cmd_buffer->cs, pred_val >> 32);
   }

   assert(cmd_buffer->cs->cdw == cdw_end);
}

/**
 * Update the DCC predicate to reflect the compression state.
 */
void
radv_update_dcc_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                         const VkImageSubresourceRange *range, bool value)
{
   if (image->dcc_pred_offset == 0)
      return;

   uint64_t pred_val = value;
   uint64_t va = radv_image_get_dcc_pred_va(image, range->baseMipLevel);
   uint32_t level_count = vk_image_subresource_level_count(&image->vk, range);

   assert(radv_dcc_enabled(image, range->baseMipLevel));

   ASSERTED unsigned cdw_end = radv_cs_write_data_head(cmd_buffer->device, cmd_buffer->cs, cmd_buffer->qf, V_370_PFP,
                                                       va, 2 * level_count, false);

   for (uint32_t l = 0; l < level_count; l++) {
      radeon_emit(cmd_buffer->cs, pred_val);
      radeon_emit(cmd_buffer->cs, pred_val >> 32);
   }

   assert(cmd_buffer->cs->cdw == cdw_end);
}

/**
 * Update the fast clear color values if the image is bound as a color buffer.
 */
static void
radv_update_bound_fast_clear_color(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, int cb_idx,
                                   uint32_t color_values[2])
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (cb_idx >= cmd_buffer->state.render.color_att_count || cmd_buffer->state.render.color_att[cb_idx].iview == NULL ||
       cmd_buffer->state.render.color_att[cb_idx].iview->image != image)
      return;

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 4);

   radeon_set_context_reg_seq(cs, R_028C8C_CB_COLOR0_CLEAR_WORD0 + cb_idx * 0x3c, 2);
   radeon_emit(cs, color_values[0]);
   radeon_emit(cs, color_values[1]);

   assert(cmd_buffer->cs->cdw <= cdw_max);

   cmd_buffer->state.context_roll_without_scissor_emitted = true;
}

/**
 * Set the clear color values to the image's metadata.
 */
static void
radv_set_color_clear_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                              const VkImageSubresourceRange *range, uint32_t color_values[2])
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint32_t level_count = vk_image_subresource_level_count(&image->vk, range);

   assert(radv_image_has_cmask(image) || radv_dcc_enabled(image, range->baseMipLevel));

   if (radv_image_has_clear_value(image)) {
      uint64_t va = radv_image_get_fast_clear_va(image, range->baseMipLevel);

      ASSERTED unsigned cdw_end = radv_cs_write_data_head(cmd_buffer->device, cmd_buffer->cs, cmd_buffer->qf, V_370_PFP,
                                                          va, 2 * level_count, cmd_buffer->state.predicating);

      for (uint32_t l = 0; l < level_count; l++) {
         radeon_emit(cs, color_values[0]);
         radeon_emit(cs, color_values[1]);
      }

      assert(cmd_buffer->cs->cdw == cdw_end);
   } else {
      /* Some default value we can set in the update. */
      assert(color_values[0] == 0 && color_values[1] == 0);
   }
}

/**
 * Update the clear color values for this image.
 */
void
radv_update_color_clear_metadata(struct radv_cmd_buffer *cmd_buffer, const struct radv_image_view *iview, int cb_idx,
                                 uint32_t color_values[2])
{
   struct radv_image *image = iview->image;
   VkImageSubresourceRange range = {
      .aspectMask = iview->vk.aspects,
      .baseMipLevel = iview->vk.base_mip_level,
      .levelCount = iview->vk.level_count,
      .baseArrayLayer = iview->vk.base_array_layer,
      .layerCount = iview->vk.layer_count,
   };

   assert(radv_image_has_cmask(image) || radv_dcc_enabled(image, iview->vk.base_mip_level));

   /* Do not need to update the clear value for images that are fast cleared with the comp-to-single
    * mode because the hardware gets the value from the image directly.
    */
   if (iview->image->support_comp_to_single)
      return;

   radv_set_color_clear_metadata(cmd_buffer, image, &range, color_values);

   radv_update_bound_fast_clear_color(cmd_buffer, image, cb_idx, color_values);
}

/**
 * Load the clear color values from the image's metadata.
 */
static void
radv_load_color_clear_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *iview, int cb_idx)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   struct radv_image *image = iview->image;

   if (!radv_image_has_cmask(image) && !radv_dcc_enabled(image, iview->vk.base_mip_level))
      return;

   if (iview->image->support_comp_to_single)
      return;

   if (!radv_image_has_clear_value(image)) {
      uint32_t color_values[2] = {0, 0};
      radv_update_bound_fast_clear_color(cmd_buffer, image, cb_idx, color_values);
      return;
   }

   uint64_t va = radv_image_get_fast_clear_va(image, iview->vk.base_mip_level);
   uint32_t reg = R_028C8C_CB_COLOR0_CLEAR_WORD0 + cb_idx * 0x3c;

   if (cmd_buffer->device->physical_device->rad_info.has_load_ctx_reg_pkt) {
      radeon_emit(cs, PKT3(PKT3_LOAD_CONTEXT_REG_INDEX, 3, cmd_buffer->state.predicating));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, (reg - SI_CONTEXT_REG_OFFSET) >> 2);
      radeon_emit(cs, 2);
   } else {
      radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, cmd_buffer->state.predicating));
      radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_SRC_MEM) | COPY_DATA_DST_SEL(COPY_DATA_REG) | COPY_DATA_COUNT_SEL);
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, reg >> 2);
      radeon_emit(cs, 0);

      radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, cmd_buffer->state.predicating));
      radeon_emit(cs, 0);
   }
}

/* GFX9+ metadata cache flushing workaround. metadata cache coherency is
 * broken if the CB caches data of multiple mips of the same image at the
 * same time.
 *
 * Insert some flushes to avoid this.
 */
static void
radv_emit_fb_mip_change_flush(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_rendering_state *render = &cmd_buffer->state.render;
   bool color_mip_changed = false;

   /* Entire workaround is not applicable before GFX9 */
   if (cmd_buffer->device->physical_device->rad_info.gfx_level < GFX9)
      return;

   for (int i = 0; i < render->color_att_count; ++i) {
      struct radv_image_view *iview = render->color_att[i].iview;
      if (!iview)
         continue;

      if ((radv_image_has_cmask(iview->image) || radv_dcc_enabled(iview->image, iview->vk.base_mip_level) ||
           radv_dcc_enabled(iview->image, cmd_buffer->state.cb_mip[i])) &&
          cmd_buffer->state.cb_mip[i] != iview->vk.base_mip_level)
         color_mip_changed = true;

      cmd_buffer->state.cb_mip[i] = iview->vk.base_mip_level;
   }

   if (color_mip_changed) {
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_CB_META;
   }

   const struct radv_image_view *iview = render->ds_att.iview;
   if (iview) {
      if ((radv_htile_enabled(iview->image, iview->vk.base_mip_level) ||
           radv_htile_enabled(iview->image, cmd_buffer->state.ds_mip)) &&
          cmd_buffer->state.ds_mip != iview->vk.base_mip_level) {
         cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB | RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;
      }

      cmd_buffer->state.ds_mip = iview->vk.base_mip_level;
   }
}

/* This function does the flushes for mip changes if the levels are not zero for
 * all render targets. This way we can assume at the start of the next cmd_buffer
 * that rendering to mip 0 doesn't need any flushes. As that is the most common
 * case that saves some flushes. */
static void
radv_emit_mip_change_flush_default(struct radv_cmd_buffer *cmd_buffer)
{
   /* Entire workaround is not applicable before GFX9 */
   if (cmd_buffer->device->physical_device->rad_info.gfx_level < GFX9)
      return;

   bool need_color_mip_flush = false;
   for (unsigned i = 0; i < 8; ++i) {
      if (cmd_buffer->state.cb_mip[i]) {
         need_color_mip_flush = true;
         break;
      }
   }

   if (need_color_mip_flush) {
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_CB_META;
   }

   if (cmd_buffer->state.ds_mip) {
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB | RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;
   }

   memset(cmd_buffer->state.cb_mip, 0, sizeof(cmd_buffer->state.cb_mip));
   cmd_buffer->state.ds_mip = 0;
}

static void
radv_emit_framebuffer_state(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_rendering_state *render = &cmd_buffer->state.render;
   int i;
   bool disable_constant_encode_ac01 = false;
   unsigned color_invalid = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11
                               ? S_028C70_FORMAT_GFX11(V_028C70_COLOR_INVALID)
                               : S_028C70_FORMAT_GFX6(V_028C70_COLOR_INVALID);
   VkExtent2D extent = {MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT};

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 51 + MAX_RTS * 70);

   for (i = 0; i < render->color_att_count; ++i) {
      struct radv_image_view *iview = render->color_att[i].iview;
      if (!iview) {
         radeon_set_context_reg(cmd_buffer->cs, R_028C70_CB_COLOR0_INFO + i * 0x3C, color_invalid);
         continue;
      }

      VkImageLayout layout = render->color_att[i].layout;

      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, iview->image->bindings[0].bo);

      assert(iview->vk.aspects & (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT |
                                  VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT));

      if (iview->image->disjoint && iview->vk.aspects == VK_IMAGE_ASPECT_COLOR_BIT) {
         for (uint32_t plane_id = 0; plane_id < iview->image->plane_count; plane_id++) {
            radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, iview->image->bindings[plane_id].bo);
         }
      } else {
         uint32_t plane_id = iview->image->disjoint ? iview->plane_id : 0;
         radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, iview->image->bindings[plane_id].bo);
      }

      radv_emit_fb_color_state(cmd_buffer, i, &render->color_att[i].cb, iview, layout);

      radv_load_color_clear_metadata(cmd_buffer, iview, i);

      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9 && iview->image->dcc_sign_reinterpret) {
         /* Disable constant encoding with the clear value of "1" with different DCC signedness
          * because the hardware will fill "1" instead of the clear value.
          */
         disable_constant_encode_ac01 = true;
      }

      extent.width = MIN2(extent.width, iview->vk.extent.width);
      extent.height = MIN2(extent.height, iview->vk.extent.height);
   }
   for (; i < cmd_buffer->state.last_subpass_color_count; i++) {
      radeon_set_context_reg(cmd_buffer->cs, R_028C70_CB_COLOR0_INFO + i * 0x3C, color_invalid);
   }
   cmd_buffer->state.last_subpass_color_count = render->color_att_count;

   if (render->ds_att.iview) {
      struct radv_image_view *iview = render->ds_att.iview;
      const struct radv_image *image = iview->image;
      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, image->bindings[0].bo);

      uint32_t qf_mask = radv_image_queue_family_mask(image, cmd_buffer->qf, cmd_buffer->qf);
      bool depth_compressed =
         radv_layout_is_htile_compressed(cmd_buffer->device, image, render->ds_att.layout, qf_mask);
      bool stencil_compressed =
         radv_layout_is_htile_compressed(cmd_buffer->device, image, render->ds_att.stencil_layout, qf_mask);

      radv_emit_fb_ds_state(cmd_buffer, &render->ds_att.ds, iview, depth_compressed, stencil_compressed);

      if (depth_compressed || stencil_compressed) {
         /* Only load the depth/stencil fast clear values when
          * compressed rendering is enabled.
          */
         radv_load_ds_clear_metadata(cmd_buffer, iview);
      }

      extent.width = MIN2(extent.width, iview->vk.extent.width);
      extent.height = MIN2(extent.height, iview->vk.extent.height);
   } else if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX10_3 && render->vrs_att.iview &&
              radv_cmd_buffer_get_vrs_image(cmd_buffer)) {
      /* When a subpass uses a VRS attachment without binding a depth/stencil attachment, we have to
       * bind our internal depth buffer that contains the VRS data as part of HTILE.
       */
      VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      struct radv_buffer *htile_buffer = cmd_buffer->device->vrs.buffer;
      struct radv_image *image = cmd_buffer->device->vrs.image;
      struct radv_ds_buffer_info ds;
      struct radv_image_view iview;

      radv_image_view_init(&iview, cmd_buffer->device,
                           &(VkImageViewCreateInfo){
                              .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                              .image = radv_image_to_handle(image),
                              .viewType = radv_meta_get_view_type(image),
                              .format = image->vk.format,
                              .subresourceRange =
                                 {
                                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1,
                                 },
                           },
                           0, NULL);

      radv_initialise_vrs_surface(image, htile_buffer, &ds);

      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, htile_buffer->bo);

      bool depth_compressed = radv_layout_is_htile_compressed(
         cmd_buffer->device, image, layout, radv_image_queue_family_mask(image, cmd_buffer->qf, cmd_buffer->qf));
      radv_emit_fb_ds_state(cmd_buffer, &ds, &iview, depth_compressed, false);

      radv_image_view_finish(&iview);
   } else {
      radv_emit_null_ds_state(cmd_buffer);
   }

   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
      bool vrs_surface_enable = render->vrs_att.iview != NULL;
      unsigned xmax = 0, ymax = 0;
      uint64_t va = 0;

      if (vrs_surface_enable) {
         struct radv_image *vrs_image = render->vrs_att.iview->image;

         va = radv_buffer_get_va(vrs_image->bindings[0].bo) + vrs_image->bindings[0].offset;
         va |= vrs_image->planes[0].surface.tile_swizzle << 8;

         xmax = vrs_image->vk.extent.width - 1;
         ymax = vrs_image->vk.extent.height - 1;
      }

      radeon_set_context_reg_seq(cmd_buffer->cs, R_0283F0_PA_SC_VRS_RATE_BASE, 3);
      radeon_emit(cmd_buffer->cs, va >> 8);
      radeon_emit(cmd_buffer->cs, S_0283F4_BASE_256B(va >> 40));
      radeon_emit(cmd_buffer->cs, S_0283F8_X_MAX(xmax) | S_0283F8_Y_MAX(ymax));

      radeon_set_context_reg(cmd_buffer->cs, R_0283D0_PA_SC_VRS_OVERRIDE_CNTL,
                             S_0283D0_VRS_SURFACE_ENABLE(vrs_surface_enable));
   }

   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX8) {
      bool disable_constant_encode = cmd_buffer->device->physical_device->rad_info.has_dcc_constant_encode;
      enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;

      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
         radeon_set_context_reg(cmd_buffer->cs, R_028424_CB_FDCC_CONTROL, S_028424_SAMPLE_MASK_TRACKER_WATERMARK(0));
      } else {
         uint8_t watermark = gfx_level >= GFX10 ? 6 : 4;

         radeon_set_context_reg(cmd_buffer->cs, R_028424_CB_DCC_CONTROL,
                                S_028424_OVERWRITE_COMBINER_MRT_SHARING_DISABLE(gfx_level <= GFX9) |
                                   S_028424_OVERWRITE_COMBINER_WATERMARK(watermark) |
                                   S_028424_DISABLE_CONSTANT_ENCODE_AC01(disable_constant_encode_ac01) |
                                   S_028424_DISABLE_CONSTANT_ENCODE_REG(disable_constant_encode));
      }
   }

   radeon_set_context_reg(cmd_buffer->cs, R_028034_PA_SC_SCREEN_SCISSOR_BR,
                          S_028034_BR_X(extent.width) | S_028034_BR_Y(extent.height));

   assert(cmd_buffer->cs->cdw <= cdw_max);

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_FRAMEBUFFER;
}

static void
radv_emit_guardband_state(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned rast_prim = radv_get_rasterization_prim(cmd_buffer);

   radv_write_guardband(cmd_buffer->cs, d->vk.vp.viewport_count, d->vk.vp.viewports, rast_prim, d->vk.rs.polygon_mode,
                        d->vk.rs.line.width);

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_GUARDBAND;
}

/* Bind an internal index buffer for GPUs that hang with 0-sized index buffers to handle robustness2
 * which requires 0 for out-of-bounds access.
 */
static void
radv_handle_zero_index_buffer_bug(struct radv_cmd_buffer *cmd_buffer, uint64_t *index_va, uint32_t *remaining_indexes)
{
   const uint32_t zero = 0;
   uint32_t offset;

   if (!radv_cmd_buffer_upload_data(cmd_buffer, sizeof(uint32_t), &zero, &offset)) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
      return;
   }

   *index_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + offset;
   *remaining_indexes = 1;
}

static void
radv_emit_index_buffer(struct radv_cmd_buffer *cmd_buffer)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   struct radv_cmd_state *state = &cmd_buffer->state;
   uint32_t max_index_count = state->max_index_count;
   uint64_t index_va = state->index_va;

   /* With indirect generated commands the index buffer bind may be part of the
    * indirect command buffer, in which case the app may not have bound any yet. */
   if (state->index_type < 0)
      return;

   /* Handle indirect draw calls with NULL index buffer if the GPU doesn't support them. */
   if (!max_index_count && cmd_buffer->device->physical_device->rad_info.has_zero_index_buffer_bug) {
      radv_handle_zero_index_buffer_bug(cmd_buffer, &index_va, &max_index_count);
   }

   radeon_emit(cs, PKT3(PKT3_INDEX_BASE, 1, 0));
   radeon_emit(cs, index_va);
   radeon_emit(cs, index_va >> 32);

   radeon_emit(cs, PKT3(PKT3_INDEX_BUFFER_SIZE, 0, 0));
   radeon_emit(cs, max_index_count);

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_INDEX_BUFFER;
}

static void
radv_flush_occlusion_query_state(struct radv_cmd_buffer *cmd_buffer)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   const bool enable_occlusion_queries =
      cmd_buffer->state.active_occlusion_queries || cmd_buffer->state.inherited_occlusion_queries;
   uint32_t db_count_control;

   if (!enable_occlusion_queries) {
      db_count_control = S_028004_ZPASS_INCREMENT_DISABLE(gfx_level < GFX11);
   } else {
      uint32_t sample_rate = util_logbase2(cmd_buffer->state.render.max_samples);
      bool gfx10_perfect =
         gfx_level >= GFX10 && (cmd_buffer->state.perfect_occlusion_queries_enabled ||
                                cmd_buffer->state.inherited_query_control_flags & VK_QUERY_CONTROL_PRECISE_BIT);

      if (gfx_level >= GFX7) {
         /* Always enable PERFECT_ZPASS_COUNTS due to issues with partially
          * covered tiles, discards, and early depth testing. For more details,
          * see https://gitlab.freedesktop.org/mesa/mesa/-/issues/3218 */
         db_count_control = S_028004_PERFECT_ZPASS_COUNTS(1) |
                            S_028004_DISABLE_CONSERVATIVE_ZPASS_COUNTS(gfx10_perfect) |
                            S_028004_SAMPLE_RATE(sample_rate) | S_028004_ZPASS_ENABLE(1) |
                            S_028004_SLICE_EVEN_ENABLE(1) | S_028004_SLICE_ODD_ENABLE(1);
      } else {
         db_count_control = S_028004_PERFECT_ZPASS_COUNTS(1) | S_028004_SAMPLE_RATE(sample_rate);
      }
   }

   if (db_count_control != cmd_buffer->state.last_db_count_control) {
      radeon_set_context_reg(cmd_buffer->cs, R_028004_DB_COUNT_CONTROL, db_count_control);

      cmd_buffer->state.context_roll_without_scissor_emitted = true;

      cmd_buffer->state.last_db_count_control = db_count_control;
   }

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_OCCLUSION_QUERY;
}

unsigned
radv_instance_rate_prolog_index(unsigned num_attributes, uint32_t instance_rate_inputs)
{
   /* instance_rate_vs_prologs is a flattened array of array of arrays of different sizes, or a
    * single array sorted in ascending order using:
    * - total number of attributes
    * - number of instanced attributes
    * - index of first instanced attribute
    */

   /* From total number of attributes to offset. */
   static const uint16_t total_to_offset[16] = {0, 1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364, 455, 560, 680};
   unsigned start_index = total_to_offset[num_attributes - 1];

   /* From number of instanced attributes to offset. This would require a different LUT depending on
    * the total number of attributes, but we can exploit a pattern to use just the LUT for 16 total
    * attributes.
    */
   static const uint8_t count_to_offset_total16[16] = {0,   16,  31,  45,  58,  70,  81,  91,
                                                       100, 108, 115, 121, 126, 130, 133, 135};
   unsigned count = util_bitcount(instance_rate_inputs);
   unsigned offset_from_start_index = count_to_offset_total16[count - 1] - ((16 - num_attributes) * (count - 1));

   unsigned first = ffs(instance_rate_inputs) - 1;
   return start_index + offset_from_start_index + first;
}

static struct radv_shader_part *
lookup_vs_prolog(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *vs_shader, uint32_t *nontrivial_divisors)
{
   assert(vs_shader->info.vs.dynamic_inputs);

   const struct radv_vs_input_state *state = &cmd_buffer->state.dynamic_vs_input;
   struct radv_device *device = cmd_buffer->device;

   unsigned num_attributes = util_last_bit(vs_shader->info.vs.vb_desc_usage_mask);
   uint32_t attribute_mask = BITFIELD_MASK(num_attributes);

   uint32_t instance_rate_inputs = state->instance_rate_inputs & attribute_mask;
   uint32_t zero_divisors = state->zero_divisors & attribute_mask;
   *nontrivial_divisors = state->nontrivial_divisors & attribute_mask;
   uint32_t misaligned_mask = cmd_buffer->state.vbo_misaligned_mask;
   if (cmd_buffer->state.vbo_misaligned_mask_invalid) {
      assert(device->physical_device->rad_info.gfx_level == GFX6 ||
             device->physical_device->rad_info.gfx_level >= GFX10);

      u_foreach_bit (index, cmd_buffer->state.vbo_misaligned_mask_invalid & attribute_mask) {
         uint8_t binding = state->bindings[index];
         if (!(cmd_buffer->state.vbo_bound_mask & BITFIELD_BIT(binding)))
            continue;

         uint8_t req = state->format_align_req_minus_1[index];
         uint64_t vb_offset = cmd_buffer->vertex_bindings[binding].offset;
         uint64_t vb_stride;

         if (cmd_buffer->state.uses_dynamic_vertex_binding_stride) {
            vb_stride = cmd_buffer->vertex_bindings[binding].stride;
         } else {
            vb_stride = cmd_buffer->state.graphics_pipeline->binding_stride[binding];
         }

         VkDeviceSize offset = vb_offset + state->offsets[index];
         if ((offset & req) || (vb_stride & req))
            misaligned_mask |= BITFIELD_BIT(index);
      }
      cmd_buffer->state.vbo_misaligned_mask = misaligned_mask;
      cmd_buffer->state.vbo_misaligned_mask_invalid &= ~attribute_mask;
   }
   misaligned_mask |= state->nontrivial_formats;
   misaligned_mask &= attribute_mask;

   const bool can_use_simple_input =
      cmd_buffer->state.shaders[MESA_SHADER_VERTEX] &&
      !cmd_buffer->state.shaders[MESA_SHADER_VERTEX]->info.merged_shader_compiled_separately &&
      cmd_buffer->state.shaders[MESA_SHADER_VERTEX]->info.is_ngg == device->physical_device->use_ngg &&
      cmd_buffer->state.shaders[MESA_SHADER_VERTEX]->info.wave_size == device->physical_device->ge_wave_size;

   /* The instance ID input VGPR is placed differently when as_ls=true. as_ls is also needed to
    * workaround the LS VGPR initialization bug.
    */
   bool as_ls =
      vs_shader->info.vs.as_ls && (instance_rate_inputs || device->physical_device->rad_info.has_ls_vgpr_init_bug);

   /* try to use a pre-compiled prolog first */
   struct radv_shader_part *prolog = NULL;
   if (can_use_simple_input && !as_ls && !misaligned_mask && !state->alpha_adjust_lo && !state->alpha_adjust_hi) {
      if (!instance_rate_inputs) {
         prolog = device->simple_vs_prologs[num_attributes - 1];
      } else if (num_attributes <= 16 && !*nontrivial_divisors && !zero_divisors &&
                 util_bitcount(instance_rate_inputs) ==
                    (util_last_bit(instance_rate_inputs) - ffs(instance_rate_inputs) + 1)) {
         unsigned index = radv_instance_rate_prolog_index(num_attributes, instance_rate_inputs);
         prolog = device->instance_rate_vs_prologs[index];
      }
   }
   if (prolog)
      return prolog;

   struct radv_vs_prolog_key key;
   memset(&key, 0, sizeof(key));
   key.instance_rate_inputs = instance_rate_inputs;
   key.nontrivial_divisors = *nontrivial_divisors;
   key.zero_divisors = zero_divisors;
   /* If the attribute is aligned, post shuffle is implemented using DST_SEL instead. */
   key.post_shuffle = state->post_shuffle & misaligned_mask;
   key.alpha_adjust_hi = state->alpha_adjust_hi & attribute_mask;
   key.alpha_adjust_lo = state->alpha_adjust_lo & attribute_mask;
   u_foreach_bit (index, misaligned_mask)
      key.formats[index] = state->formats[index];
   key.num_attributes = num_attributes;
   key.misaligned_mask = misaligned_mask;
   key.as_ls = as_ls;
   key.is_ngg = vs_shader->info.is_ngg;
   key.wave32 = vs_shader->info.wave_size == 32;

   if (vs_shader->info.merged_shader_compiled_separately) {
      assert(vs_shader->info.next_stage == MESA_SHADER_TESS_CTRL || vs_shader->info.next_stage == MESA_SHADER_GEOMETRY);
      key.next_stage = vs_shader->info.next_stage;
   } else {
      key.next_stage = vs_shader->info.stage;
   }

   return radv_shader_part_cache_get(device, &device->vs_prologs, &cmd_buffer->vs_prologs, &key);
}

static void
emit_prolog_regs(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *vs_shader,
                 const struct radv_shader_part *prolog)
{
   /* no need to re-emit anything in this case */
   if (cmd_buffer->state.emitted_vs_prolog == prolog)
      return;

   enum amd_gfx_level chip = cmd_buffer->device->physical_device->rad_info.gfx_level;

   assert(cmd_buffer->state.emitted_graphics_pipeline == cmd_buffer->state.graphics_pipeline);

   uint32_t rsrc1 = vs_shader->config.rsrc1;
   if (chip < GFX10 && G_00B228_SGPRS(prolog->rsrc1) > G_00B228_SGPRS(vs_shader->config.rsrc1))
      rsrc1 = (rsrc1 & C_00B228_SGPRS) | (prolog->rsrc1 & ~C_00B228_SGPRS);

   /* The main shader must not use less VGPRs than the prolog, otherwise shared vgprs might not
    * work.
    */
   assert(G_00B848_VGPRS(vs_shader->config.rsrc1) >= G_00B848_VGPRS(prolog->rsrc1));

   unsigned pgm_lo_reg = R_00B120_SPI_SHADER_PGM_LO_VS;
   unsigned rsrc1_reg = R_00B128_SPI_SHADER_PGM_RSRC1_VS;
   if (vs_shader->info.is_ngg || cmd_buffer->state.shaders[MESA_SHADER_GEOMETRY] == vs_shader ||
       (vs_shader->info.merged_shader_compiled_separately && vs_shader->info.next_stage == MESA_SHADER_GEOMETRY)) {
      pgm_lo_reg = chip >= GFX10 ? R_00B320_SPI_SHADER_PGM_LO_ES : R_00B210_SPI_SHADER_PGM_LO_ES;
      rsrc1_reg = R_00B228_SPI_SHADER_PGM_RSRC1_GS;
   } else if (cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL] == vs_shader ||
              (vs_shader->info.merged_shader_compiled_separately &&
               vs_shader->info.next_stage == MESA_SHADER_TESS_CTRL)) {
      pgm_lo_reg = chip >= GFX10 ? R_00B520_SPI_SHADER_PGM_LO_LS : R_00B410_SPI_SHADER_PGM_LO_LS;
      rsrc1_reg = R_00B428_SPI_SHADER_PGM_RSRC1_HS;
   } else if (vs_shader->info.vs.as_ls) {
      pgm_lo_reg = R_00B520_SPI_SHADER_PGM_LO_LS;
      rsrc1_reg = R_00B528_SPI_SHADER_PGM_RSRC1_LS;
   } else if (vs_shader->info.vs.as_es) {
      pgm_lo_reg = R_00B320_SPI_SHADER_PGM_LO_ES;
      rsrc1_reg = R_00B328_SPI_SHADER_PGM_RSRC1_ES;
   }

   radeon_set_sh_reg(cmd_buffer->cs, pgm_lo_reg, prolog->va >> 8);

   if (chip < GFX10)
      radeon_set_sh_reg(cmd_buffer->cs, rsrc1_reg, rsrc1);
   else
      assert(rsrc1 == vs_shader->config.rsrc1);

   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, prolog->bo);
}

static void
emit_prolog_inputs(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *vs_shader,
                   uint32_t nontrivial_divisors)
{
   /* no need to re-emit anything in this case */
   if (!nontrivial_divisors && cmd_buffer->state.emitted_vs_prolog &&
       !cmd_buffer->state.emitted_vs_prolog->nontrivial_divisors)
      return;

   const struct radv_vs_input_state *state = &cmd_buffer->state.dynamic_vs_input;
   uint64_t input_va = radv_shader_get_va(vs_shader);

   if (nontrivial_divisors) {
      unsigned inputs_offset;
      uint32_t *inputs;
      unsigned size = 8 + util_bitcount(nontrivial_divisors) * 8;
      if (!radv_cmd_buffer_upload_alloc(cmd_buffer, size, &inputs_offset, (void **)&inputs))
         return;

      *(inputs++) = input_va;
      *(inputs++) = input_va >> 32;

      u_foreach_bit (index, nontrivial_divisors) {
         uint32_t div = state->divisors[index];
         if (div == 0) {
            *(inputs++) = 0;
            *(inputs++) = 1;
         } else if (util_is_power_of_two_or_zero(div)) {
            *(inputs++) = util_logbase2(div) | (1 << 8);
            *(inputs++) = 0xffffffffu;
         } else {
            struct util_fast_udiv_info info = util_compute_fast_udiv_info(div, 32, 32);
            *(inputs++) = info.pre_shift | (info.increment << 8) | (info.post_shift << 16);
            *(inputs++) = info.multiplier;
         }
      }

      input_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + inputs_offset;
   }

   const struct radv_userdata_info *loc = &vs_shader->info.user_sgprs_locs.shader_data[AC_UD_VS_PROLOG_INPUTS];
   uint32_t base_reg = vs_shader->info.user_data_0;
   assert(loc->sgpr_idx != -1);
   assert(loc->num_sgprs == 2);
   radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, input_va, true);
}

static void
radv_emit_vertex_input(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *vs_shader = radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_VERTEX);

   assert(!cmd_buffer->state.mesh_shading);

   if (!vs_shader->info.vs.has_prolog)
      return;

   uint32_t nontrivial_divisors;
   struct radv_shader_part *prolog = lookup_vs_prolog(cmd_buffer, vs_shader, &nontrivial_divisors);
   if (!prolog) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
      return;
   }
   emit_prolog_regs(cmd_buffer, vs_shader, prolog);
   emit_prolog_inputs(cmd_buffer, vs_shader, nontrivial_divisors);

   cmd_buffer->shader_upload_seq = MAX2(cmd_buffer->shader_upload_seq, prolog->upload_seq);

   cmd_buffer->state.emitted_vs_prolog = prolog;

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_save_vs_prolog(cmd_buffer, prolog);
}

static void
radv_emit_tess_domain_origin(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const struct radv_shader *tes = radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_TESS_EVAL);
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned type = 0, partitioning = 0, distribution_mode = 0;
   unsigned topology;

   switch (tes->info.tes._primitive_mode) {
   case TESS_PRIMITIVE_TRIANGLES:
      type = V_028B6C_TESS_TRIANGLE;
      break;
   case TESS_PRIMITIVE_QUADS:
      type = V_028B6C_TESS_QUAD;
      break;
   case TESS_PRIMITIVE_ISOLINES:
      type = V_028B6C_TESS_ISOLINE;
      break;
   default:
      unreachable("Invalid tess primitive type");
   }

   switch (tes->info.tes.spacing) {
   case TESS_SPACING_EQUAL:
      partitioning = V_028B6C_PART_INTEGER;
      break;
   case TESS_SPACING_FRACTIONAL_ODD:
      partitioning = V_028B6C_PART_FRAC_ODD;
      break;
   case TESS_SPACING_FRACTIONAL_EVEN:
      partitioning = V_028B6C_PART_FRAC_EVEN;
      break;
   default:
      unreachable("Invalid tess spacing type");
   }

   if (pdevice->rad_info.has_distributed_tess) {
      if (pdevice->rad_info.family == CHIP_FIJI || pdevice->rad_info.family >= CHIP_POLARIS10)
         distribution_mode = V_028B6C_TRAPEZOIDS;
      else
         distribution_mode = V_028B6C_DONUTS;
   } else {
      distribution_mode = V_028B6C_NO_DIST;
   }

   if (tes->info.tes.point_mode) {
      topology = V_028B6C_OUTPUT_POINT;
   } else if (tes->info.tes._primitive_mode == TESS_PRIMITIVE_ISOLINES) {
      topology = V_028B6C_OUTPUT_LINE;
   } else {
      bool ccw = tes->info.tes.ccw;

      if (d->vk.ts.domain_origin != VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT) {
         ccw = !ccw;
      }

      topology = ccw ? V_028B6C_OUTPUT_TRIANGLE_CCW : V_028B6C_OUTPUT_TRIANGLE_CW;
   }

   radeon_set_context_reg(cmd_buffer->cs, R_028B6C_VGT_TF_PARAM,
                          S_028B6C_TYPE(type) | S_028B6C_PARTITIONING(partitioning) | S_028B6C_TOPOLOGY(topology) |
                             S_028B6C_DISTRIBUTION_MODE(distribution_mode));
}

static void
radv_emit_alpha_to_coverage_enable(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned db_alpha_to_mask = 0;

   if (cmd_buffer->device->instance->debug_flags & RADV_DEBUG_NO_ATOC_DITHERING) {
      db_alpha_to_mask = S_028B70_ALPHA_TO_MASK_OFFSET0(2) | S_028B70_ALPHA_TO_MASK_OFFSET1(2) |
                         S_028B70_ALPHA_TO_MASK_OFFSET2(2) | S_028B70_ALPHA_TO_MASK_OFFSET3(2) |
                         S_028B70_OFFSET_ROUND(0);
   } else {
      db_alpha_to_mask = S_028B70_ALPHA_TO_MASK_OFFSET0(3) | S_028B70_ALPHA_TO_MASK_OFFSET1(1) |
                         S_028B70_ALPHA_TO_MASK_OFFSET2(0) | S_028B70_ALPHA_TO_MASK_OFFSET3(2) |
                         S_028B70_OFFSET_ROUND(1);
   }

   db_alpha_to_mask |= S_028B70_ALPHA_TO_MASK_ENABLE(d->vk.ms.alpha_to_coverage_enable);

   radeon_set_context_reg(cmd_buffer->cs, R_028B70_DB_ALPHA_TO_MASK, db_alpha_to_mask);
}

static void
radv_emit_sample_mask(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   radeon_set_context_reg_seq(cmd_buffer->cs, R_028C38_PA_SC_AA_MASK_X0Y0_X1Y0, 2);
   radeon_emit(cmd_buffer->cs, d->vk.ms.sample_mask | ((uint32_t)d->vk.ms.sample_mask << 16));
   radeon_emit(cmd_buffer->cs, d->vk.ms.sample_mask | ((uint32_t)d->vk.ms.sample_mask << 16));
}

static void
radv_emit_color_blend(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const enum amd_gfx_level gfx_level = pdevice->rad_info.gfx_level;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned cb_blend_control[MAX_RTS], sx_mrt_blend_opt[MAX_RTS];
   bool mrt0_is_dual_src = radv_is_mrt0_dual_src(cmd_buffer);

   for (unsigned i = 0; i < MAX_RTS; i++) {
      VkBlendOp eqRGB = d->vk.cb.attachments[i].color_blend_op;
      VkBlendFactor srcRGB = d->vk.cb.attachments[i].src_color_blend_factor;
      VkBlendFactor dstRGB = d->vk.cb.attachments[i].dst_color_blend_factor;
      VkBlendOp eqA = d->vk.cb.attachments[i].alpha_blend_op;
      VkBlendFactor srcA = d->vk.cb.attachments[i].src_alpha_blend_factor;
      VkBlendFactor dstA = d->vk.cb.attachments[i].dst_alpha_blend_factor;
      unsigned srcRGB_opt, dstRGB_opt, srcA_opt, dstA_opt;
      unsigned blend_cntl = 0;

      cb_blend_control[i] = sx_mrt_blend_opt[i] = 0;

      /* Ignore other blend targets if dual-source blending is enabled to prevent wrong behaviour.
       */
      if (i > 0 && mrt0_is_dual_src)
         continue;

      if (!d->vk.cb.attachments[i].blend_enable) {
         sx_mrt_blend_opt[i] |= S_028760_COLOR_COMB_FCN(V_028760_OPT_COMB_BLEND_DISABLED) |
                                S_028760_ALPHA_COMB_FCN(V_028760_OPT_COMB_BLEND_DISABLED);
         continue;
      }

      radv_normalize_blend_factor(eqRGB, &srcRGB, &dstRGB);
      radv_normalize_blend_factor(eqA, &srcA, &dstA);

      /* Blending optimizations for RB+.
       * These transformations don't change the behavior.
       *
       * First, get rid of DST in the blend factors:
       *    func(src * DST, dst * 0) ---> func(src * 0, dst * SRC)
       */
      radv_blend_remove_dst(&eqRGB, &srcRGB, &dstRGB, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_SRC_COLOR);

      radv_blend_remove_dst(&eqA, &srcA, &dstA, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_SRC_COLOR);

      radv_blend_remove_dst(&eqA, &srcA, &dstA, VK_BLEND_FACTOR_DST_ALPHA, VK_BLEND_FACTOR_SRC_ALPHA);

      /* Look up the ideal settings from tables. */
      srcRGB_opt = radv_translate_blend_opt_factor(srcRGB, false);
      dstRGB_opt = radv_translate_blend_opt_factor(dstRGB, false);
      srcA_opt = radv_translate_blend_opt_factor(srcA, true);
      dstA_opt = radv_translate_blend_opt_factor(dstA, true);

      /* Handle interdependencies. */
      if (radv_blend_factor_uses_dst(srcRGB))
         dstRGB_opt = V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_NONE;
      if (radv_blend_factor_uses_dst(srcA))
         dstA_opt = V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_NONE;

      if (srcRGB == VK_BLEND_FACTOR_SRC_ALPHA_SATURATE &&
          (dstRGB == VK_BLEND_FACTOR_ZERO || dstRGB == VK_BLEND_FACTOR_SRC_ALPHA ||
           dstRGB == VK_BLEND_FACTOR_SRC_ALPHA_SATURATE))
         dstRGB_opt = V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_A0;

      /* Set the final value. */
      sx_mrt_blend_opt[i] = S_028760_COLOR_SRC_OPT(srcRGB_opt) | S_028760_COLOR_DST_OPT(dstRGB_opt) |
                            S_028760_COLOR_COMB_FCN(radv_translate_blend_opt_function(eqRGB)) |
                            S_028760_ALPHA_SRC_OPT(srcA_opt) | S_028760_ALPHA_DST_OPT(dstA_opt) |
                            S_028760_ALPHA_COMB_FCN(radv_translate_blend_opt_function(eqA));

      blend_cntl |= S_028780_ENABLE(1);
      blend_cntl |= S_028780_COLOR_COMB_FCN(radv_translate_blend_function(eqRGB));
      blend_cntl |= S_028780_COLOR_SRCBLEND(radv_translate_blend_factor(gfx_level, srcRGB));
      blend_cntl |= S_028780_COLOR_DESTBLEND(radv_translate_blend_factor(gfx_level, dstRGB));
      if (srcA != srcRGB || dstA != dstRGB || eqA != eqRGB) {
         blend_cntl |= S_028780_SEPARATE_ALPHA_BLEND(1);
         blend_cntl |= S_028780_ALPHA_COMB_FCN(radv_translate_blend_function(eqA));
         blend_cntl |= S_028780_ALPHA_SRCBLEND(radv_translate_blend_factor(gfx_level, srcA));
         blend_cntl |= S_028780_ALPHA_DESTBLEND(radv_translate_blend_factor(gfx_level, dstA));
      }
      cb_blend_control[i] = blend_cntl;
   }

   if (pdevice->rad_info.has_rbplus) {
      /* Disable RB+ blend optimizations for dual source blending. */
      if (mrt0_is_dual_src) {
         for (unsigned i = 0; i < MAX_RTS; i++) {
            sx_mrt_blend_opt[i] =
               S_028760_COLOR_COMB_FCN(V_028760_OPT_COMB_NONE) | S_028760_ALPHA_COMB_FCN(V_028760_OPT_COMB_NONE);
         }
      }

      /* Disable RB+ blend optimizations on GFX11 when alpha-to-coverage is enabled. */
      if (gfx_level >= GFX11 && d->vk.ms.alpha_to_coverage_enable) {
         sx_mrt_blend_opt[0] =
            S_028760_COLOR_COMB_FCN(V_028760_OPT_COMB_NONE) | S_028760_ALPHA_COMB_FCN(V_028760_OPT_COMB_NONE);
      }
   }

   radeon_set_context_reg_seq(cmd_buffer->cs, R_028780_CB_BLEND0_CONTROL, MAX_RTS);
   radeon_emit_array(cmd_buffer->cs, cb_blend_control, MAX_RTS);

   if (pdevice->rad_info.has_rbplus) {
      radeon_set_context_reg_seq(cmd_buffer->cs, R_028760_SX_MRT0_BLEND_OPT, MAX_RTS);
      radeon_emit_array(cmd_buffer->cs, sx_mrt_blend_opt, MAX_RTS);
   }
}

static struct radv_shader_part *
lookup_ps_epilog(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *ps = cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT];
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   struct radv_device *device = cmd_buffer->device;
   struct radv_ps_epilog_state state = {0};

   state.color_attachment_count = render->color_att_count;
   for (unsigned i = 0; i < render->color_att_count; ++i) {
      state.color_attachment_formats[i] = render->color_att[i].format;
   }

   for (unsigned i = 0; i < MAX_RTS; i++) {
      VkBlendOp eqRGB = d->vk.cb.attachments[i].color_blend_op;
      VkBlendFactor srcRGB = d->vk.cb.attachments[i].src_color_blend_factor;
      VkBlendFactor dstRGB = d->vk.cb.attachments[i].dst_color_blend_factor;

      state.color_write_mask |= d->vk.cb.attachments[i].write_mask << (4 * i);
      state.color_blend_enable |= d->vk.cb.attachments[i].blend_enable << (4 * i);

      radv_normalize_blend_factor(eqRGB, &srcRGB, &dstRGB);

      if (srcRGB == VK_BLEND_FACTOR_SRC_ALPHA || dstRGB == VK_BLEND_FACTOR_SRC_ALPHA ||
          srcRGB == VK_BLEND_FACTOR_SRC_ALPHA_SATURATE || dstRGB == VK_BLEND_FACTOR_SRC_ALPHA_SATURATE ||
          srcRGB == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA || dstRGB == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
         state.need_src_alpha |= 1 << i;
   }

   state.mrt0_is_dual_src = radv_is_mrt0_dual_src(cmd_buffer);

   if (d->vk.ms.alpha_to_coverage_enable) {
      /* Select a color export format with alpha when alpha to coverage is enabled. */
      state.need_src_alpha |= 0x1;
   }

   if (ps) {
      state.colors_written = ps->info.ps.colors_written;

      if (ps->info.ps.exports_mrtz_via_epilog) {
         assert(device->physical_device->rad_info.gfx_level >= GFX11);
         state.export_depth = ps->info.ps.writes_z;
         state.export_stencil = ps->info.ps.writes_stencil;
         state.export_sample_mask = ps->info.ps.writes_sample_mask;
         state.alpha_to_coverage_via_mrtz = d->vk.ms.alpha_to_coverage_enable;
      }
   }

   struct radv_ps_epilog_key key = radv_generate_ps_epilog_key(device, &state);

   /* Clear color attachments that aren't exported by the FS to match IO shader arguments. */
   key.spi_shader_col_format &= ps->info.ps.colors_written;

   return radv_shader_part_cache_get(device, &device->ps_epilogs, &cmd_buffer->ps_epilogs, &key);
}

static struct radv_shader_part *
lookup_tcs_epilog(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *tcs = cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL];
   const struct radv_shader *tes = radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_TESS_EVAL);
   struct radv_device *device = cmd_buffer->device;

   struct radv_tcs_epilog_key key = {
      .primitive_mode = tes->info.tes._primitive_mode,
      .tes_reads_tessfactors = tes->info.tes.reads_tess_factors,
      .tcs_out_patch_fits_subgroup = tcs->info.wave_size % tcs->info.tcs.tcs_vertices_out == 0,
   };

   return radv_shader_part_cache_get(device, &device->tcs_epilogs, &cmd_buffer->tcs_epilogs, &key);
}

static void
radv_emit_msaa_state(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   const struct radv_shader *ps = cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT];
   unsigned rasterization_samples = radv_get_rasterization_samples(cmd_buffer);
   const struct radv_rendering_state *render = &cmd_buffer->state.render;
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   unsigned log_samples = util_logbase2(rasterization_samples);
   unsigned pa_sc_aa_config = 0;
   unsigned max_sample_dist = 0;
   unsigned db_eqaa;

   db_eqaa = S_028804_HIGH_QUALITY_INTERSECTIONS(1) | S_028804_INCOHERENT_EQAA_READS(1) |
             S_028804_STATIC_ANCHOR_ASSOCIATIONS(1);

   if (pdevice->rad_info.gfx_level >= GFX9 &&
       d->vk.rs.conservative_mode != VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT) {
      /* Adjust MSAA state if conservative rasterization is enabled. */
      db_eqaa |= S_028804_OVERRASTERIZATION_AMOUNT(4);
      pa_sc_aa_config |= S_028BE0_AA_MASK_CENTROID_DTMN(1);
   }

   if (!d->sample_location.count) {
      max_sample_dist = radv_get_default_max_sample_dist(log_samples);
   } else {
      uint32_t num_samples = (uint32_t)d->sample_location.per_pixel;
      VkOffset2D sample_locs[4][8]; /* 8 is the max. sample count supported */

      /* Convert the user sample locations to hardware sample locations. */
      radv_convert_user_sample_locs(&d->sample_location, 0, 0, sample_locs[0]);
      radv_convert_user_sample_locs(&d->sample_location, 1, 0, sample_locs[1]);
      radv_convert_user_sample_locs(&d->sample_location, 0, 1, sample_locs[2]);
      radv_convert_user_sample_locs(&d->sample_location, 1, 1, sample_locs[3]);

      /* Compute the maximum sample distance from the specified locations. */
      for (unsigned i = 0; i < 4; ++i) {
         for (uint32_t j = 0; j < num_samples; j++) {
            VkOffset2D offset = sample_locs[i][j];
            max_sample_dist = MAX2(max_sample_dist, MAX2(abs(offset.x), abs(offset.y)));
         }
      }
   }

   if (rasterization_samples > 1) {
      unsigned z_samples = MAX2(render->ds_samples, rasterization_samples);
      unsigned ps_iter_samples = radv_get_ps_iter_samples(cmd_buffer);
      unsigned log_z_samples = util_logbase2(z_samples);
      unsigned log_ps_iter_samples = util_logbase2(ps_iter_samples);
      bool uses_underestimate = d->vk.rs.conservative_mode == VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT;

      db_eqaa |= S_028804_MAX_ANCHOR_SAMPLES(log_z_samples) | S_028804_PS_ITER_SAMPLES(log_ps_iter_samples) |
                 S_028804_MASK_EXPORT_NUM_SAMPLES(log_samples) | S_028804_ALPHA_TO_MASK_NUM_SAMPLES(log_samples);

      pa_sc_aa_config |= S_028BE0_MSAA_NUM_SAMPLES(uses_underestimate ? 0 : log_samples) |
                         S_028BE0_MAX_SAMPLE_DIST(max_sample_dist) | S_028BE0_MSAA_EXPOSED_SAMPLES(log_samples) |
                         S_028BE0_COVERED_CENTROID_IS_CENTER(pdevice->rad_info.gfx_level >= GFX10_3);

      if (d->vk.rs.line.mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT)
         db_eqaa |= S_028804_OVERRASTERIZATION_AMOUNT(log_samples);
   }

   pa_sc_aa_config |= S_028BE0_COVERAGE_TO_SHADER_SELECT(ps && ps->info.ps.reads_fully_covered);

   /* On GFX11, DB_Z_INFO.NUM_SAMPLES should always match MSAA_EXPOSED_SAMPLES. It affects VRS,
    * occlusion queries and Primitive Ordered Pixel Shading if depth and stencil are not bound.
    * This is normally emitted as framebuffer state, but if no attachments are bound the sample
    * count is independent of the framebuffer state and hence may need to be updated with MSAA
    * state.
    * Checking the format, not the image view, because the latter may not exist in a secondary
    * command buffer.
    */
   if (pdevice->rad_info.gfx_level == GFX11 && render->ds_att.format == VK_FORMAT_UNDEFINED) {
      assert(!render->ds_att.iview);
      radeon_set_context_reg(cmd_buffer->cs, R_028040_DB_Z_INFO,
                             S_028040_FORMAT(V_028040_Z_INVALID) | S_028040_NUM_SAMPLES(log_samples));
   }
   radeon_set_context_reg(cmd_buffer->cs, R_028804_DB_EQAA, db_eqaa);
   radeon_set_context_reg(cmd_buffer->cs, R_028BE0_PA_SC_AA_CONFIG, pa_sc_aa_config);
   radeon_set_context_reg(
      cmd_buffer->cs, R_028A48_PA_SC_MODE_CNTL_0,
      S_028A48_ALTERNATE_RBS_PER_TILE(pdevice->rad_info.gfx_level >= GFX9) | S_028A48_VPORT_SCISSOR_ENABLE(1) |
         S_028A48_LINE_STIPPLE_ENABLE(d->vk.rs.line.stipple.enable) | S_028A48_MSAA_ENABLE(rasterization_samples > 1));
}

static void
radv_emit_line_rasterization_mode(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   /* The DX10 diamond test is unnecessary with Vulkan and it decreases line rasterization
    * performance.
    */
   radeon_set_context_reg(
      cmd_buffer->cs, R_028BDC_PA_SC_LINE_CNTL,
      S_028BDC_PERPENDICULAR_ENDCAP_ENA(d->vk.rs.line.mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT));
}

static void
radv_cmd_buffer_flush_dynamic_state(struct radv_cmd_buffer *cmd_buffer, const uint64_t states)
{
   if (states & (RADV_CMD_DIRTY_DYNAMIC_VIEWPORT | RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_ENABLE |
                 RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE | RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLAMP_ENABLE))
      radv_emit_viewport(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_SCISSOR | RADV_CMD_DIRTY_DYNAMIC_VIEWPORT) &&
       !cmd_buffer->device->physical_device->rad_info.has_gfx9_scissor_bug)
      radv_emit_scissor(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_LINE_WIDTH)
      radv_emit_line_width(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_BLEND_CONSTANTS)
      radv_emit_blend_constants(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_STENCIL_REFERENCE | RADV_CMD_DIRTY_DYNAMIC_STENCIL_WRITE_MASK |
                 RADV_CMD_DIRTY_DYNAMIC_STENCIL_COMPARE_MASK))
      radv_emit_stencil(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_DEPTH_BOUNDS)
      radv_emit_depth_bounds(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_DEPTH_BIAS)
      radv_emit_depth_bias(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE | RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE_ENABLE |
                 RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE_MODE))
      radv_emit_discard_rectangle(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_CONSERVATIVE_RAST_MODE)
      radv_emit_conservative_rast_mode(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_SAMPLE_LOCATIONS)
      radv_emit_sample_locations(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_LINE_STIPPLE)
      radv_emit_line_stipple(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_CULL_MODE | RADV_CMD_DIRTY_DYNAMIC_FRONT_FACE |
                 RADV_CMD_DIRTY_DYNAMIC_DEPTH_BIAS_ENABLE | RADV_CMD_DIRTY_DYNAMIC_POLYGON_MODE |
                 RADV_CMD_DIRTY_DYNAMIC_PROVOKING_VERTEX_MODE | RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE))
      radv_emit_culling(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_PROVOKING_VERTEX_MODE | RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_TOPOLOGY))
      radv_emit_provoking_vertex_mode(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_TOPOLOGY)
      radv_emit_primitive_topology(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_DEPTH_TEST_ENABLE | RADV_CMD_DIRTY_DYNAMIC_DEPTH_WRITE_ENABLE |
                 RADV_CMD_DIRTY_DYNAMIC_DEPTH_COMPARE_OP | RADV_CMD_DIRTY_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE |
                 RADV_CMD_DIRTY_DYNAMIC_STENCIL_TEST_ENABLE | RADV_CMD_DIRTY_DYNAMIC_STENCIL_OP))
      radv_emit_depth_control(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_STENCIL_OP)
      radv_emit_stencil_control(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_FRAGMENT_SHADING_RATE)
      radv_emit_fragment_shading_rate(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_RESTART_ENABLE)
      radv_emit_primitive_restart_enable(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_RASTERIZER_DISCARD_ENABLE | RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_ENABLE |
                 RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE | RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLAMP_ENABLE))
      radv_emit_clipping(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_LOGIC_OP | RADV_CMD_DIRTY_DYNAMIC_LOGIC_OP_ENABLE |
                 RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_MASK | RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_ENABLE |
                 RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_EQUATION))
      radv_emit_logic_op(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_ENABLE | RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_MASK))
      radv_emit_color_write(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_VERTEX_INPUT)
      radv_emit_vertex_input(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_PATCH_CONTROL_POINTS)
      radv_emit_patch_control_points(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_TESS_DOMAIN_ORIGIN)
      radv_emit_tess_domain_origin(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE)
      radv_emit_alpha_to_coverage_enable(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_SAMPLE_MASK)
      radv_emit_sample_mask(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLAMP_ENABLE | RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_ENABLE))
      radv_emit_depth_clamp_enable(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_ENABLE | RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_MASK |
                 RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_EQUATION | RADV_CMD_DIRTY_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE))
      radv_emit_color_blend(cmd_buffer);

   if (states & RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE)
      radv_emit_line_rasterization_mode(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES | RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE))
      radv_emit_rasterization_samples(cmd_buffer);

   if (states & (RADV_CMD_DIRTY_DYNAMIC_LINE_STIPPLE_ENABLE | RADV_CMD_DIRTY_DYNAMIC_CONSERVATIVE_RAST_MODE |
                 RADV_CMD_DIRTY_DYNAMIC_SAMPLE_LOCATIONS | RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES |
                 RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE))
      radv_emit_msaa_state(cmd_buffer);

   /* RADV_CMD_DIRTY_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE is handled by radv_emit_db_shader_control. */

   cmd_buffer->state.dirty &= ~states;
}

static void
radv_flush_push_descriptors(struct radv_cmd_buffer *cmd_buffer, struct radv_descriptor_state *descriptors_state)
{
   struct radv_descriptor_set *set = (struct radv_descriptor_set *)&descriptors_state->push_set.set;
   unsigned bo_offset;

   if (!radv_cmd_buffer_upload_data(cmd_buffer, set->header.size, set->header.mapped_ptr, &bo_offset))
      return;

   set->header.va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
   set->header.va += bo_offset;
}

static void
radv_flush_indirect_descriptor_sets(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point)
{
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);
   uint32_t size = MAX_SETS * 4;
   uint32_t offset;
   void *ptr;

   if (!radv_cmd_buffer_upload_alloc(cmd_buffer, size, &offset, &ptr))
      return;

   for (unsigned i = 0; i < MAX_SETS; i++) {
      uint32_t *uptr = ((uint32_t *)ptr) + i;
      uint64_t set_va = 0;
      if (descriptors_state->valid & (1u << i))
         set_va = radv_descriptor_get_va(descriptors_state, i);

      uptr[0] = set_va & 0xffffffff;
   }

   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   struct radv_device *device = cmd_buffer->device;
   uint64_t va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
   va += offset;

   ASSERTED unsigned cdw_max = radeon_check_space(device->ws, cs, MESA_VULKAN_SHADER_STAGES * 3);

   if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
      for (unsigned s = MESA_SHADER_VERTEX; s <= MESA_SHADER_FRAGMENT; s++)
         if (radv_cmdbuf_has_stage(cmd_buffer, s))
            radv_emit_userdata_address(device, cs, cmd_buffer->state.shaders[s],
                                       cmd_buffer->state.shaders[s]->info.user_data_0, AC_UD_INDIRECT_DESCRIPTOR_SETS,
                                       va);

      if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_MESH))
         radv_emit_userdata_address(device, cs, cmd_buffer->state.shaders[MESA_SHADER_MESH],
                                    cmd_buffer->state.shaders[MESA_SHADER_MESH]->info.user_data_0,
                                    AC_UD_INDIRECT_DESCRIPTOR_SETS, va);

      if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TASK)) {
         radeon_check_space(device->ws, cmd_buffer->gang.cs, 3);
         radv_emit_userdata_address(device, cmd_buffer->gang.cs, cmd_buffer->state.shaders[MESA_SHADER_TASK],
                                    cmd_buffer->state.shaders[MESA_SHADER_TASK]->info.user_data_0,
                                    AC_UD_INDIRECT_DESCRIPTOR_SETS, va);
      }
   } else {
      struct radv_shader *compute_shader = bind_point == VK_PIPELINE_BIND_POINT_COMPUTE
                                              ? cmd_buffer->state.shaders[MESA_SHADER_COMPUTE]
                                              : cmd_buffer->state.rt_prolog;

      radv_emit_userdata_address(device, cs, compute_shader, compute_shader->info.user_data_0,
                                 AC_UD_INDIRECT_DESCRIPTOR_SETS, va);
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);
}

ALWAYS_INLINE static void
radv_flush_descriptors(struct radv_cmd_buffer *cmd_buffer, VkShaderStageFlags stages, VkPipelineBindPoint bind_point)
{
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);
   struct radv_device *device = cmd_buffer->device;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   bool flush_indirect_descriptors;

   if (!descriptors_state->dirty)
      return;

   flush_indirect_descriptors = descriptors_state->need_indirect_descriptor_sets;

   if (flush_indirect_descriptors)
      radv_flush_indirect_descriptor_sets(cmd_buffer, bind_point);

   ASSERTED unsigned cdw_max = radeon_check_space(device->ws, cs, MAX_SETS * MESA_VULKAN_SHADER_STAGES * 4);

   if (stages & VK_SHADER_STAGE_COMPUTE_BIT) {
      struct radv_shader *compute_shader = bind_point == VK_PIPELINE_BIND_POINT_COMPUTE
                                              ? cmd_buffer->state.shaders[MESA_SHADER_COMPUTE]
                                              : cmd_buffer->state.rt_prolog;

      radv_emit_descriptor_pointers(device, cs, compute_shader, compute_shader->info.user_data_0, descriptors_state);
   } else {
      radv_foreach_stage(stage, stages & ~VK_SHADER_STAGE_TASK_BIT_EXT)
      {
         if (!cmd_buffer->state.shaders[stage])
            continue;

         radv_emit_descriptor_pointers(device, cs, cmd_buffer->state.shaders[stage],
                                       cmd_buffer->state.shaders[stage]->info.user_data_0, descriptors_state);
      }

      if (stages & VK_SHADER_STAGE_TASK_BIT_EXT) {
         radv_emit_descriptor_pointers(device, cmd_buffer->gang.cs, cmd_buffer->state.shaders[MESA_SHADER_TASK],
                                       cmd_buffer->state.shaders[MESA_SHADER_TASK]->info.user_data_0,
                                       descriptors_state);
      }
   }

   descriptors_state->dirty = 0;

   assert(cmd_buffer->cs->cdw <= cdw_max);

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_save_descriptors(cmd_buffer, bind_point);
}

static void
radv_emit_all_inline_push_consts(struct radv_device *device, struct radeon_cmdbuf *cs, struct radv_shader *shader,
                                 uint32_t base_reg, uint32_t *values, bool *need_push_constants)
{
   if (radv_get_user_sgpr(shader, AC_UD_PUSH_CONSTANTS)->sgpr_idx != -1)
      *need_push_constants |= true;

   const uint64_t mask = shader->info.inline_push_constant_mask;
   if (!mask)
      return;

   const uint8_t base = ffs(mask) - 1;
   if (mask == u_bit_consecutive64(base, util_last_bit64(mask) - base)) {
      /* consecutive inline push constants */
      radv_emit_inline_push_consts(device, cs, shader, base_reg, AC_UD_INLINE_PUSH_CONSTANTS, values + base);
   } else {
      /* sparse inline push constants */
      uint32_t consts[AC_MAX_INLINE_PUSH_CONSTS];
      unsigned num_consts = 0;
      u_foreach_bit64 (idx, mask)
         consts[num_consts++] = values[idx];
      radv_emit_inline_push_consts(device, cs, shader, base_reg, AC_UD_INLINE_PUSH_CONSTANTS, consts);
   }
}

ALWAYS_INLINE static VkShaderStageFlags
radv_must_flush_constants(const struct radv_cmd_buffer *cmd_buffer, VkShaderStageFlags stages,
                          VkPipelineBindPoint bind_point)
{
   const struct radv_push_constant_state *push_constants = radv_get_push_constants_state(cmd_buffer, bind_point);

   if (push_constants->size || push_constants->dynamic_offset_count)
      return stages & cmd_buffer->push_constant_stages;

   return 0;
}

static void
radv_flush_constants(struct radv_cmd_buffer *cmd_buffer, VkShaderStageFlags stages, VkPipelineBindPoint bind_point)
{
   struct radv_device *device = cmd_buffer->device;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);
   const struct radv_push_constant_state *push_constants = radv_get_push_constants_state(cmd_buffer, bind_point);
   struct radv_shader *shader, *prev_shader;
   bool need_push_constants = false;
   unsigned offset;
   void *ptr;
   uint64_t va;
   uint32_t internal_stages = stages;
   uint32_t dirty_stages = 0;

   switch (bind_point) {
   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      break;
   case VK_PIPELINE_BIND_POINT_COMPUTE:
      dirty_stages = RADV_RT_STAGE_BITS;
      break;
   case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
      internal_stages = VK_SHADER_STAGE_COMPUTE_BIT;
      dirty_stages = VK_SHADER_STAGE_COMPUTE_BIT;
      break;
   default:
      unreachable("Unhandled bind point");
   }

   if (internal_stages & VK_SHADER_STAGE_COMPUTE_BIT) {
      struct radv_shader *compute_shader = bind_point == VK_PIPELINE_BIND_POINT_COMPUTE
                                              ? cmd_buffer->state.shaders[MESA_SHADER_COMPUTE]
                                              : cmd_buffer->state.rt_prolog;

      radv_emit_all_inline_push_consts(device, cs, compute_shader, compute_shader->info.user_data_0,
                                       (uint32_t *)cmd_buffer->push_constants, &need_push_constants);
   } else {
      radv_foreach_stage(stage, internal_stages & ~VK_SHADER_STAGE_TASK_BIT_EXT)
      {
         shader = radv_get_shader(cmd_buffer->state.shaders, stage);

         if (!shader)
            continue;

         radv_emit_all_inline_push_consts(device, cs, shader, shader->info.user_data_0,
                                          (uint32_t *)cmd_buffer->push_constants, &need_push_constants);
      }

      if (internal_stages & VK_SHADER_STAGE_TASK_BIT_EXT) {
         radv_emit_all_inline_push_consts(device, cmd_buffer->gang.cs, cmd_buffer->state.shaders[MESA_SHADER_TASK],
                                          cmd_buffer->state.shaders[MESA_SHADER_TASK]->info.user_data_0,
                                          (uint32_t *)cmd_buffer->push_constants, &need_push_constants);
      }
   }

   if (need_push_constants) {
      if (!radv_cmd_buffer_upload_alloc(cmd_buffer, push_constants->size + 16 * push_constants->dynamic_offset_count,
                                        &offset, &ptr))
         return;

      memcpy(ptr, cmd_buffer->push_constants, push_constants->size);
      memcpy((char *)ptr + push_constants->size, descriptors_state->dynamic_buffers,
             16 * push_constants->dynamic_offset_count);

      va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
      va += offset;

      ASSERTED unsigned cdw_max =
         radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, MESA_VULKAN_SHADER_STAGES * 4);

      if (internal_stages & VK_SHADER_STAGE_COMPUTE_BIT) {
         struct radv_shader *compute_shader = bind_point == VK_PIPELINE_BIND_POINT_COMPUTE
                                                 ? cmd_buffer->state.shaders[MESA_SHADER_COMPUTE]
                                                 : cmd_buffer->state.rt_prolog;

         radv_emit_userdata_address(device, cs, compute_shader, compute_shader->info.user_data_0, AC_UD_PUSH_CONSTANTS,
                                    va);
      } else {
         prev_shader = NULL;
         radv_foreach_stage(stage, internal_stages & ~VK_SHADER_STAGE_TASK_BIT_EXT)
         {
            shader = radv_get_shader(cmd_buffer->state.shaders, stage);

            /* Avoid redundantly emitting the address for merged stages. */
            if (shader && shader != prev_shader) {
               radv_emit_userdata_address(device, cs, shader, shader->info.user_data_0, AC_UD_PUSH_CONSTANTS, va);

               prev_shader = shader;
            }
         }

         if (internal_stages & VK_SHADER_STAGE_TASK_BIT_EXT) {
            radv_emit_userdata_address(device, cmd_buffer->gang.cs, cmd_buffer->state.shaders[MESA_SHADER_TASK],
                                       cmd_buffer->state.shaders[MESA_SHADER_TASK]->info.user_data_0,
                                       AC_UD_PUSH_CONSTANTS, va);
         }
      }

      assert(cmd_buffer->cs->cdw <= cdw_max);
   }

   cmd_buffer->push_constant_stages &= ~stages;
   cmd_buffer->push_constant_stages |= dirty_stages;
}

void
radv_write_vertex_descriptors(const struct radv_cmd_buffer *cmd_buffer, const struct radv_graphics_pipeline *pipeline,
                              bool full_null_descriptors, void *vb_ptr)
{
   struct radv_shader *vs_shader = radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_VERTEX);
   enum amd_gfx_level chip = cmd_buffer->device->physical_device->rad_info.gfx_level;
   enum radeon_family family = cmd_buffer->device->physical_device->rad_info.family;
   unsigned desc_index = 0;
   uint32_t mask = vs_shader->info.vs.vb_desc_usage_mask;
   uint64_t va;
   const struct radv_vs_input_state *vs_state =
      vs_shader->info.vs.dynamic_inputs ? &cmd_buffer->state.dynamic_vs_input : NULL;
   assert(!vs_state || vs_shader->info.vs.use_per_attribute_vb_descs);

   const struct ac_vtx_format_info *vtx_info_table = vs_state ? ac_get_vtx_format_info_table(chip, family) : NULL;

   while (mask) {
      unsigned i = u_bit_scan(&mask);
      uint32_t *desc = &((uint32_t *)vb_ptr)[desc_index++ * 4];
      uint32_t offset, rsrc_word3;

      if (vs_state && !(vs_state->attribute_mask & BITFIELD_BIT(i))) {
         /* No vertex attribute description given: assume that the shader doesn't use this
          * location (vb_desc_usage_mask can be larger than attribute usage) and use a null
          * descriptor to avoid hangs (prologs load all attributes, even if there are holes).
          */
         memset(desc, 0, 4 * 4);
         continue;
      }

      unsigned binding = vs_state ? cmd_buffer->state.dynamic_vs_input.bindings[i]
                                  : (vs_shader->info.vs.use_per_attribute_vb_descs ? pipeline->attrib_bindings[i] : i);
      struct radv_buffer *buffer = cmd_buffer->vertex_binding_buffers[binding];
      unsigned num_records;
      unsigned stride;

      if (vs_state && !(vs_state->nontrivial_formats & BITFIELD_BIT(i))) {
         const struct ac_vtx_format_info *vtx_info = &vtx_info_table[vs_state->formats[i]];
         unsigned hw_format = vtx_info->hw_format[vtx_info->num_channels - 1];

         if (chip >= GFX10) {
            rsrc_word3 = vtx_info->dst_sel | S_008F0C_FORMAT(hw_format);
         } else {
            rsrc_word3 =
               vtx_info->dst_sel | S_008F0C_NUM_FORMAT((hw_format >> 4) & 0x7) | S_008F0C_DATA_FORMAT(hw_format & 0xf);
         }
      } else {
         rsrc_word3 = S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) | S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
                      S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) | S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W);
         if (chip >= GFX10)
            rsrc_word3 |= S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_UINT);
         else
            rsrc_word3 |=
               S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_UINT) | S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);
      }

      if (cmd_buffer->state.uses_dynamic_vertex_binding_stride) {
         stride = cmd_buffer->vertex_bindings[binding].stride;
      } else {
         stride = pipeline->binding_stride[binding];
      }

      if (!buffer) {
         if (full_null_descriptors) {
            /* Put all the info in for the DGC generation shader in case the VBO gets overridden. */
            desc[0] = 0;
            desc[1] = S_008F04_STRIDE(stride);
            desc[2] = 0;
            desc[3] = rsrc_word3;
         } else if (vs_state) {
            /* Stride needs to be non-zero on GFX9, or else bounds checking is disabled. We need
             * to include the format/word3 so that the alpha channel is 1 for formats without an
             * alpha channel.
             */
            desc[0] = 0;
            desc[1] = S_008F04_STRIDE(16);
            desc[2] = 0;
            desc[3] = rsrc_word3;
         } else {
            memset(desc, 0, 4 * 4);
         }

         continue;
      }

      va = radv_buffer_get_va(buffer->bo);

      offset = cmd_buffer->vertex_bindings[binding].offset;
      va += offset + buffer->offset;
      if (vs_state)
         va += vs_state->offsets[i];

      if (cmd_buffer->vertex_bindings[binding].size) {
         num_records = cmd_buffer->vertex_bindings[binding].size;
      } else {
         num_records = vk_buffer_range(&buffer->vk, offset, VK_WHOLE_SIZE);
      }

      if (vs_shader->info.vs.use_per_attribute_vb_descs) {
         uint32_t attrib_end = vs_state ? vs_state->offsets[i] + vs_state->format_sizes[i] : pipeline->attrib_ends[i];

         if (num_records < attrib_end) {
            num_records = 0; /* not enough space for one vertex */
         } else if (stride == 0) {
            num_records = 1; /* only one vertex */
         } else {
            num_records = (num_records - attrib_end) / stride + 1;
            /* If attrib_offset>stride, then the compiler will increase the vertex index by
             * attrib_offset/stride and decrease the offset by attrib_offset%stride. This is
             * only allowed with static strides.
             */
            num_records += pipeline->attrib_index_offset[i];
         }

         /* GFX10 uses OOB_SELECT_RAW if stride==0, so convert num_records from elements into
          * into bytes in that case. GFX8 always uses bytes.
          */
         if (num_records && (chip == GFX8 || (chip != GFX9 && !stride))) {
            num_records = (num_records - 1) * stride + attrib_end;
         } else if (!num_records) {
            /* On GFX9, it seems bounds checking is disabled if both
             * num_records and stride are zero. This doesn't seem necessary on GFX8, GFX10 and
             * GFX10.3 but it doesn't hurt.
             */
            if (full_null_descriptors) {
               /* Put all the info in for the DGC generation shader in case the VBO gets overridden.
                */
               desc[0] = 0;
               desc[1] = S_008F04_STRIDE(stride);
               desc[2] = 0;
               desc[3] = rsrc_word3;
            } else if (vs_state) {
               desc[0] = 0;
               desc[1] = S_008F04_STRIDE(16);
               desc[2] = 0;
               desc[3] = rsrc_word3;
            } else {
               memset(desc, 0, 16);
            }

            continue;
         }
      } else {
         if (chip != GFX8 && stride)
            num_records = DIV_ROUND_UP(num_records, stride);
      }

      if (chip >= GFX10) {
         /* OOB_SELECT chooses the out-of-bounds check:
          * - 1: index >= NUM_RECORDS (Structured)
          * - 3: offset >= NUM_RECORDS (Raw)
          */
         int oob_select = stride ? V_008F0C_OOB_SELECT_STRUCTURED : V_008F0C_OOB_SELECT_RAW;
         rsrc_word3 |= S_008F0C_OOB_SELECT(oob_select) | S_008F0C_RESOURCE_LEVEL(chip < GFX11);
      }

      desc[0] = va;
      desc[1] = S_008F04_BASE_ADDRESS_HI(va >> 32) | S_008F04_STRIDE(stride);
      desc[2] = num_records;
      desc[3] = rsrc_word3;
   }
}

static void
radv_flush_vertex_descriptors(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_shader *vs = radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_VERTEX);

   if (!vs->info.vs.vb_desc_usage_mask)
      return;

   /* Mesh shaders don't have vertex descriptors. */
   assert(!cmd_buffer->state.mesh_shading);

   struct radv_graphics_pipeline *pipeline = cmd_buffer->state.graphics_pipeline;
   unsigned vb_desc_alloc_size = util_bitcount(vs->info.vs.vb_desc_usage_mask) * 16;
   unsigned vb_offset;
   void *vb_ptr;
   uint64_t va;

   /* allocate some descriptor state for vertex buffers */
   if (!radv_cmd_buffer_upload_alloc(cmd_buffer, vb_desc_alloc_size, &vb_offset, &vb_ptr))
      return;

   radv_write_vertex_descriptors(cmd_buffer, pipeline, false, vb_ptr);

   va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
   va += vb_offset;

   radv_emit_userdata_address(cmd_buffer->device, cmd_buffer->cs, vs, vs->info.user_data_0, AC_UD_VS_VERTEX_BUFFERS,
                              va);

   cmd_buffer->state.vb_va = va;
   cmd_buffer->state.vb_size = vb_desc_alloc_size;
   cmd_buffer->state.prefetch_L2_mask |= RADV_PREFETCH_VBO_DESCRIPTORS;

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_save_vertex_descriptors(cmd_buffer, (uintptr_t)vb_ptr);

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_VERTEX_BUFFER;
}

static void
radv_emit_streamout_buffers(struct radv_cmd_buffer *cmd_buffer, uint64_t va)
{
   const struct radv_shader *last_vgt_shader = cmd_buffer->state.last_vgt_shader;
   const struct radv_userdata_info *loc = radv_get_user_sgpr(last_vgt_shader, AC_UD_STREAMOUT_BUFFERS);
   uint32_t base_reg;

   if (loc->sgpr_idx == -1)
      return;

   base_reg = last_vgt_shader->info.user_data_0;

   radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, va, false);

   if (cmd_buffer->state.gs_copy_shader) {
      loc = &cmd_buffer->state.gs_copy_shader->info.user_sgprs_locs.shader_data[AC_UD_STREAMOUT_BUFFERS];
      if (loc->sgpr_idx != -1) {
         base_reg = R_00B130_SPI_SHADER_USER_DATA_VS_0;

         radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, va, false);
      }
   }
}

static void
radv_flush_streamout_descriptors(struct radv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->state.dirty & RADV_CMD_DIRTY_STREAMOUT_BUFFER) {
      struct radv_streamout_binding *sb = cmd_buffer->streamout_bindings;
      struct radv_streamout_state *so = &cmd_buffer->state.streamout;
      unsigned so_offset;
      uint64_t desc_va;
      void *so_ptr;

      /* Allocate some descriptor state for streamout buffers. */
      if (!radv_cmd_buffer_upload_alloc(cmd_buffer, MAX_SO_BUFFERS * 16, &so_offset, &so_ptr))
         return;

      for (uint32_t i = 0; i < MAX_SO_BUFFERS; i++) {
         struct radv_buffer *buffer = sb[i].buffer;
         uint32_t *desc = &((uint32_t *)so_ptr)[i * 4];
         uint32_t size = 0;
         uint64_t va = 0;

         if (so->enabled_mask & (1 << i)) {
            va = radv_buffer_get_va(buffer->bo) + buffer->offset;

            va += sb[i].offset;

            /* Set the descriptor.
             *
             * On GFX8, the format must be non-INVALID, otherwise
             * the buffer will be considered not bound and store
             * instructions will be no-ops.
             */
            size = 0xffffffff;

            if (cmd_buffer->device->physical_device->use_ngg_streamout) {
               /* With NGG streamout, the buffer size is used to determine the max emit per buffer
                * and also acts as a disable bit when it's 0.
                */
               size = radv_is_streamout_enabled(cmd_buffer) ? sb[i].size : 0;
            }
         }

         uint32_t rsrc_word3 = S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) | S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
                               S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) | S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W);

         if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
            rsrc_word3 |=
               S_008F0C_FORMAT(V_008F0C_GFX11_FORMAT_32_FLOAT) | S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW);
         } else if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10) {
            rsrc_word3 |= S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_FLOAT) |
                          S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW) | S_008F0C_RESOURCE_LEVEL(1);
         } else {
            rsrc_word3 |= S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);
         }

         desc[0] = va;
         desc[1] = S_008F04_BASE_ADDRESS_HI(va >> 32);
         desc[2] = size;
         desc[3] = rsrc_word3;
      }

      desc_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
      desc_va += so_offset;

      radv_emit_streamout_buffers(cmd_buffer, desc_va);
   }

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_STREAMOUT_BUFFER;
}

static void
radv_flush_shader_query_state_gfx(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *last_vgt_shader = cmd_buffer->state.last_vgt_shader;
   const struct radv_userdata_info *loc = radv_get_user_sgpr(last_vgt_shader, AC_UD_SHADER_QUERY_STATE);
   enum radv_shader_query_state shader_query_state = radv_shader_query_none;
   uint32_t base_reg;

   if (loc->sgpr_idx == -1)
      return;

   assert(last_vgt_shader->info.is_ngg || last_vgt_shader->info.stage == MESA_SHADER_GEOMETRY);

   /* By default shader queries are disabled but they are enabled if the command buffer has active GDS
    * queries or if it's a secondary command buffer that inherits the number of generated
    * primitives.
    */
   if (cmd_buffer->state.active_pipeline_gds_queries || (cmd_buffer->state.inherited_pipeline_statistics &
                                                         (VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT |
                                                          VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
                                                          VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT)))
      shader_query_state |= radv_shader_query_pipeline_stat;

   if (cmd_buffer->state.active_prims_gen_gds_queries)
      shader_query_state |= radv_shader_query_prim_gen;

   if (cmd_buffer->state.active_prims_xfb_gds_queries && radv_is_streamout_enabled(cmd_buffer)) {
      shader_query_state |= radv_shader_query_prim_xfb | radv_shader_query_prim_gen;
   }

   base_reg = last_vgt_shader->info.user_data_0;
   assert(loc->sgpr_idx != -1);

   radeon_set_sh_reg(cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, shader_query_state);
}

static void
radv_flush_shader_query_state_ace(struct radv_cmd_buffer *cmd_buffer, struct radv_shader *task_shader)
{
   const struct radv_userdata_info *loc = radv_get_user_sgpr(task_shader, AC_UD_SHADER_QUERY_STATE);
   enum radv_shader_query_state shader_query_state = radv_shader_query_none;
   uint32_t base_reg;

   if (loc->sgpr_idx == -1)
      return;

   /* By default shader queries are disabled but they are enabled if the command buffer has active ACE
    * queries or if it's a secondary command buffer that inherits the number of task shader
    * invocations query.
    */
   if (cmd_buffer->state.active_pipeline_ace_queries ||
       (cmd_buffer->state.inherited_pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT))
      shader_query_state |= radv_shader_query_pipeline_stat;

   base_reg = task_shader->info.user_data_0;
   assert(loc->sgpr_idx != -1);

   radeon_set_sh_reg(cmd_buffer->gang.cs, base_reg + loc->sgpr_idx * 4, shader_query_state);
}

static void
radv_flush_shader_query_state(struct radv_cmd_buffer *cmd_buffer)
{
   radv_flush_shader_query_state_gfx(cmd_buffer);

   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TASK))
      radv_flush_shader_query_state_ace(cmd_buffer, cmd_buffer->state.shaders[MESA_SHADER_TASK]);

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_SHADER_QUERY;
}

static void
radv_flush_force_vrs_state(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *last_vgt_shader = cmd_buffer->state.last_vgt_shader;

   if (!last_vgt_shader->info.force_vrs_per_vertex) {
      /* Un-set the SGPR index so we know to re-emit it later. */
      cmd_buffer->state.last_vrs_rates_sgpr_idx = -1;
      return;
   }

   const struct radv_userdata_info *loc;
   uint32_t base_reg;

   if (cmd_buffer->state.gs_copy_shader) {
      loc = &cmd_buffer->state.gs_copy_shader->info.user_sgprs_locs.shader_data[AC_UD_FORCE_VRS_RATES];
      base_reg = R_00B130_SPI_SHADER_USER_DATA_VS_0;
   } else {
      loc = radv_get_user_sgpr(last_vgt_shader, AC_UD_FORCE_VRS_RATES);
      base_reg = last_vgt_shader->info.user_data_0;
   }

   assert(loc->sgpr_idx != -1);

   enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   uint32_t vrs_rates = 0;

   switch (cmd_buffer->device->force_vrs) {
   case RADV_FORCE_VRS_2x2:
      vrs_rates = gfx_level >= GFX11 ? V_0283D0_VRS_SHADING_RATE_2X2 : (1u << 2) | (1u << 4);
      break;
   case RADV_FORCE_VRS_2x1:
      vrs_rates = gfx_level >= GFX11 ? V_0283D0_VRS_SHADING_RATE_2X1 : (1u << 2) | (0u << 4);
      break;
   case RADV_FORCE_VRS_1x2:
      vrs_rates = gfx_level >= GFX11 ? V_0283D0_VRS_SHADING_RATE_1X2 : (0u << 2) | (1u << 4);
      break;
   default:
      break;
   }

   if (cmd_buffer->state.last_vrs_rates != vrs_rates || cmd_buffer->state.last_vrs_rates_sgpr_idx != loc->sgpr_idx) {
      radeon_set_sh_reg(cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, vrs_rates);
   }

   cmd_buffer->state.last_vrs_rates = vrs_rates;
   cmd_buffer->state.last_vrs_rates_sgpr_idx = loc->sgpr_idx;
}

static void
radv_upload_graphics_shader_descriptors(struct radv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->state.dirty & RADV_CMD_DIRTY_VERTEX_BUFFER)
      radv_flush_vertex_descriptors(cmd_buffer);

   radv_flush_streamout_descriptors(cmd_buffer);

   VkShaderStageFlags stages = VK_SHADER_STAGE_ALL_GRAPHICS;
   radv_flush_descriptors(cmd_buffer, stages, VK_PIPELINE_BIND_POINT_GRAPHICS);

   const VkShaderStageFlags pc_stages = radv_must_flush_constants(cmd_buffer, stages, VK_PIPELINE_BIND_POINT_GRAPHICS);
   if (pc_stages)
      radv_flush_constants(cmd_buffer, pc_stages, VK_PIPELINE_BIND_POINT_GRAPHICS);

   radv_flush_force_vrs_state(cmd_buffer);
}

struct radv_draw_info {
   /**
    * Number of vertices.
    */
   uint32_t count;

   /**
    * First instance id.
    */
   uint32_t first_instance;

   /**
    * Number of instances.
    */
   uint32_t instance_count;

   /**
    * Whether it's an indexed draw.
    */
   bool indexed;

   /**
    * Indirect draw parameters resource.
    */
   struct radv_buffer *indirect;
   uint64_t indirect_offset;
   uint32_t stride;

   /**
    * Draw count parameters resource.
    */
   struct radv_buffer *count_buffer;
   uint64_t count_buffer_offset;

   /**
    * Stream output parameters resource.
    */
   struct radv_buffer *strmout_buffer;
   uint64_t strmout_buffer_offset;
};

static void
radv_emit_ia_multi_vgt_param(struct radv_cmd_buffer *cmd_buffer, bool instanced_draw, bool indirect_draw,
                             bool count_from_stream_output, uint32_t draw_vertex_count)
{
   const struct radeon_info *info = &cmd_buffer->device->physical_device->rad_info;
   struct radv_cmd_state *state = &cmd_buffer->state;
   const unsigned patch_control_points = state->dynamic.vk.ts.patch_control_points;
   const unsigned topology = state->dynamic.vk.ia.primitive_topology;
   const bool prim_restart_enable = state->dynamic.vk.ia.primitive_restart_enable;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   unsigned ia_multi_vgt_param;

   ia_multi_vgt_param = radv_get_ia_multi_vgt_param(cmd_buffer, instanced_draw, indirect_draw, count_from_stream_output,
                                                    draw_vertex_count, topology, prim_restart_enable,
                                                    patch_control_points, state->tess_num_patches);

   if (state->last_ia_multi_vgt_param != ia_multi_vgt_param) {
      if (info->gfx_level == GFX9) {
         radeon_set_uconfig_reg_idx(cmd_buffer->device->physical_device, cs, R_030960_IA_MULTI_VGT_PARAM, 4,
                                    ia_multi_vgt_param);
      } else if (info->gfx_level >= GFX7) {
         radeon_set_context_reg_idx(cs, R_028AA8_IA_MULTI_VGT_PARAM, 1, ia_multi_vgt_param);
      } else {
         radeon_set_context_reg(cs, R_028AA8_IA_MULTI_VGT_PARAM, ia_multi_vgt_param);
      }
      state->last_ia_multi_vgt_param = ia_multi_vgt_param;
   }
}

static void
gfx10_emit_ge_cntl(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *last_vgt_shader = cmd_buffer->state.last_vgt_shader;
   struct radv_cmd_state *state = &cmd_buffer->state;
   bool break_wave_at_eoi = false;
   unsigned primgroup_size;
   unsigned ge_cntl;

   if (last_vgt_shader->info.is_ngg)
      return;

   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TESS_CTRL)) {
      primgroup_size = state->tess_num_patches;

      if (cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL]->info.uses_prim_id ||
          radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_TESS_EVAL)->info.uses_prim_id) {
         break_wave_at_eoi = true;
      }
   } else if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_GEOMETRY)) {
      const struct radv_legacy_gs_info *gs_state = &cmd_buffer->state.shaders[MESA_SHADER_GEOMETRY]->info.gs_ring_info;
      primgroup_size = G_028A44_GS_PRIMS_PER_SUBGRP(gs_state->vgt_gs_onchip_cntl);
   } else {
      primgroup_size = 128; /* recommended without a GS and tess */
   }

   ge_cntl = S_03096C_PRIM_GRP_SIZE_GFX10(primgroup_size) | S_03096C_VERT_GRP_SIZE(256) | /* disable vertex grouping */
             S_03096C_PACKET_TO_ONE_PA(0) /* this should only be set if LINE_STIPPLE_TEX_ENA == 1 */ |
             S_03096C_BREAK_WAVE_AT_EOI(break_wave_at_eoi);

   if (state->last_ge_cntl != ge_cntl) {
      radeon_set_uconfig_reg(cmd_buffer->cs, R_03096C_GE_CNTL, ge_cntl);
      state->last_ge_cntl = ge_cntl;
   }
}

static void
radv_emit_draw_registers(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *draw_info)
{
   const struct radeon_info *info = &cmd_buffer->device->physical_device->rad_info;
   struct radv_cmd_state *state = &cmd_buffer->state;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint32_t topology = state->dynamic.vk.ia.primitive_topology;
   bool disable_instance_packing = false;

   /* Draw state. */
   if (info->gfx_level >= GFX10) {
      gfx10_emit_ge_cntl(cmd_buffer);
   } else {
      radv_emit_ia_multi_vgt_param(cmd_buffer, draw_info->instance_count > 1, draw_info->indirect,
                                   !!draw_info->strmout_buffer, draw_info->indirect ? 0 : draw_info->count);
   }

   /* RDNA2 is affected by a hardware bug when instance packing is enabled for adjacent primitive
    * topologies and instance_count > 1, pipeline stats generated by GE are incorrect. It needs to
    * be applied for indexed and non-indexed draws.
    */
   if (info->gfx_level == GFX10_3 && state->active_pipeline_queries > 0 &&
       (draw_info->instance_count > 1 || draw_info->indirect) &&
       (topology == V_008958_DI_PT_LINELIST_ADJ || topology == V_008958_DI_PT_LINESTRIP_ADJ ||
        topology == V_008958_DI_PT_TRILIST_ADJ || topology == V_008958_DI_PT_TRISTRIP_ADJ)) {
      disable_instance_packing = true;
   }

   if ((draw_info->indexed && state->index_type != state->last_index_type) ||
       (info->gfx_level == GFX10_3 &&
        (state->last_index_type == -1 ||
         disable_instance_packing != G_028A7C_DISABLE_INSTANCE_PACKING(state->last_index_type)))) {
      uint32_t index_type = state->index_type | S_028A7C_DISABLE_INSTANCE_PACKING(disable_instance_packing);

      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9) {
         radeon_set_uconfig_reg_idx(cmd_buffer->device->physical_device, cs, R_03090C_VGT_INDEX_TYPE, 2, index_type);
      } else {
         radeon_emit(cs, PKT3(PKT3_INDEX_TYPE, 0, 0));
         radeon_emit(cs, index_type);
      }

      state->last_index_type = index_type;
   }
}

static void
radv_stage_flush(struct radv_cmd_buffer *cmd_buffer, VkPipelineStageFlags2 src_stage_mask)
{
   /* For simplicity, if the barrier wants to wait for the task shader,
    * just make it wait for the mesh shader too.
    */
   if (src_stage_mask & VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT)
      src_stage_mask |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;

   if (src_stage_mask & (VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_RESOLVE_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT |
                         VK_PIPELINE_STAGE_2_CLEAR_BIT)) {
      /* Be conservative for now. */
      src_stage_mask |= VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
   }

   if (src_stage_mask &
       (VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT |
        VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV | VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR |
        VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR |
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)) {
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_CS_PARTIAL_FLUSH;
   }

   if (src_stage_mask & (VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                         VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT |
                         VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT |
                         VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)) {
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_PS_PARTIAL_FLUSH;
   } else if (src_stage_mask &
              (VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT |
               VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT |
               VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT |
               VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT |
               VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT)) {
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_VS_PARTIAL_FLUSH;
   }
}

static bool
can_skip_buffer_l2_flushes(struct radv_device *device)
{
   return device->physical_device->rad_info.gfx_level == GFX9 ||
          (device->physical_device->rad_info.gfx_level >= GFX10 &&
           !device->physical_device->rad_info.tcc_rb_non_coherent);
}

/*
 * In vulkan barriers have two kinds of operations:
 *
 * - visibility (implemented with radv_src_access_flush)
 * - availability (implemented with radv_dst_access_flush)
 *
 * for a memory operation to observe the result of a previous memory operation
 * one needs to do a visibility operation from the source memory and then an
 * availability operation to the target memory.
 *
 * The complication is the availability and visibility operations do not need to
 * be in the same barrier.
 *
 * The cleanest way to implement this is to define the visibility operation to
 * bring the caches to a "state of rest", which none of the caches below that
 * level dirty.
 *
 * For GFX8 and earlier this would be VRAM/GTT with none of the caches dirty.
 *
 * For GFX9+ we can define the state at rest to be L2 instead of VRAM for all
 * buffers and for images marked as coherent, and VRAM/GTT for non-coherent
 * images. However, given the existence of memory barriers which do not specify
 * the image/buffer it often devolves to just VRAM/GTT anyway.
 *
 * To help reducing the invalidations for GPUs that have L2 coherency between the
 * RB and the shader caches, we always invalidate L2 on the src side, as we can
 * use our knowledge of past usage to optimize flushes away.
 */

enum radv_cmd_flush_bits
radv_src_access_flush(struct radv_cmd_buffer *cmd_buffer, VkAccessFlags2 src_flags, const struct radv_image *image)
{
   bool has_CB_meta = true, has_DB_meta = true;
   bool image_is_coherent = image ? image->l2_coherent : false;
   enum radv_cmd_flush_bits flush_bits = 0;

   if (image) {
      if (!radv_image_has_CB_metadata(image))
         has_CB_meta = false;
      if (!radv_image_has_htile(image))
         has_DB_meta = false;
   }

   u_foreach_bit64 (b, src_flags) {
      switch ((VkAccessFlags2)BITFIELD64_BIT(b)) {
      case VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV:
         flush_bits |= RADV_CMD_FLAG_INV_L2;
         break;
      case VK_ACCESS_2_SHADER_WRITE_BIT:
      case VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT:
         /* since the STORAGE bit isn't set we know that this is a meta operation.
          * on the dst flush side we skip CB/DB flushes without the STORAGE bit, so
          * set it here. */
         if (image && !(image->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
            if (vk_format_is_depth_or_stencil(image->vk.format)) {
               flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB;
            } else {
               flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB;
            }
         }

         if (!image_is_coherent)
            flush_bits |= RADV_CMD_FLAG_INV_L2;
         break;
      case VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR:
      case VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT:
      case VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT:
         if (!image_is_coherent)
            flush_bits |= RADV_CMD_FLAG_WB_L2;
         break;
      case VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT:
         flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB;
         if (has_CB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB_META;
         break;
      case VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
         flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB;
         if (has_DB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;
         break;
      case VK_ACCESS_2_TRANSFER_WRITE_BIT:
         flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB;

         if (!image_is_coherent)
            flush_bits |= RADV_CMD_FLAG_INV_L2;
         if (has_CB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB_META;
         if (has_DB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;
         break;
      case VK_ACCESS_2_MEMORY_WRITE_BIT:
         flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB;

         if (!image_is_coherent)
            flush_bits |= RADV_CMD_FLAG_INV_L2;
         if (has_CB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB_META;
         if (has_DB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;
         break;
      default:
         break;
      }
   }
   return flush_bits;
}

enum radv_cmd_flush_bits
radv_dst_access_flush(struct radv_cmd_buffer *cmd_buffer, VkAccessFlags2 dst_flags, const struct radv_image *image)
{
   bool has_CB_meta = true, has_DB_meta = true;
   enum radv_cmd_flush_bits flush_bits = 0;
   bool flush_CB = true, flush_DB = true;
   bool image_is_coherent = image ? image->l2_coherent : false;

   if (image) {
      if (!(image->vk.usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
         flush_CB = false;
         flush_DB = false;
      }

      if (!radv_image_has_CB_metadata(image))
         has_CB_meta = false;
      if (!radv_image_has_htile(image))
         has_DB_meta = false;
   }

   /* All the L2 invalidations below are not the CB/DB. So if there are no incoherent images
    * in the L2 cache in CB/DB mode then they are already usable from all the other L2 clients. */
   image_is_coherent |= can_skip_buffer_l2_flushes(cmd_buffer->device) && !cmd_buffer->state.rb_noncoherent_dirty;

   u_foreach_bit64 (b, dst_flags) {
      switch ((VkAccessFlags2)BITFIELD64_BIT(b)) {
      case VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT:
         /* SMEM loads are used to read compute dispatch size in shaders */
         if (!cmd_buffer->device->load_grid_size_from_user_sgpr)
            flush_bits |= RADV_CMD_FLAG_INV_SCACHE;

         /* Ensure the DGC meta shader can read the commands. */
         if (cmd_buffer->device->uses_device_generated_commands) {
            flush_bits |= RADV_CMD_FLAG_INV_SCACHE | RADV_CMD_FLAG_INV_VCACHE;

            if (cmd_buffer->device->physical_device->rad_info.gfx_level < GFX9)
               flush_bits |= RADV_CMD_FLAG_INV_L2;
         }

         break;
      case VK_ACCESS_2_INDEX_READ_BIT:
      case VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT:
         break;
      case VK_ACCESS_2_UNIFORM_READ_BIT:
         flush_bits |= RADV_CMD_FLAG_INV_VCACHE | RADV_CMD_FLAG_INV_SCACHE;
         break;
      case VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT:
      case VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT:
      case VK_ACCESS_2_TRANSFER_READ_BIT:
      case VK_ACCESS_2_TRANSFER_WRITE_BIT:
         flush_bits |= RADV_CMD_FLAG_INV_VCACHE;

         if (has_CB_meta || has_DB_meta)
            flush_bits |= RADV_CMD_FLAG_INV_L2_METADATA;
         if (!image_is_coherent)
            flush_bits |= RADV_CMD_FLAG_INV_L2;
         break;
      case VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT:
         flush_bits |= RADV_CMD_FLAG_INV_SCACHE;
         break;
      case VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR:
      case VK_ACCESS_2_SHADER_READ_BIT:
      case VK_ACCESS_2_SHADER_STORAGE_READ_BIT:
         /* Unlike LLVM, ACO uses SMEM for SSBOs and we have to
          * invalidate the scalar cache. */
         if (!cmd_buffer->device->physical_device->use_llvm && !image)
            flush_bits |= RADV_CMD_FLAG_INV_SCACHE;
         FALLTHROUGH;
      case VK_ACCESS_2_SHADER_SAMPLED_READ_BIT:
         flush_bits |= RADV_CMD_FLAG_INV_VCACHE;
         if (has_CB_meta || has_DB_meta)
            flush_bits |= RADV_CMD_FLAG_INV_L2_METADATA;
         if (!image_is_coherent)
            flush_bits |= RADV_CMD_FLAG_INV_L2;
         break;
      case VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_NV:
      case VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR:
         flush_bits |= RADV_CMD_FLAG_INV_VCACHE;
         if (cmd_buffer->device->physical_device->rad_info.gfx_level < GFX9)
            flush_bits |= RADV_CMD_FLAG_INV_L2;
         break;
      case VK_ACCESS_2_SHADER_WRITE_BIT:
      case VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT:
      case VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR:
         break;
      case VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT:
      case VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT:
         if (flush_CB)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB;
         if (has_CB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB_META;
         break;
      case VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
      case VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
         if (flush_DB)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB;
         if (has_DB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;
         break;
      case VK_ACCESS_2_MEMORY_READ_BIT:
      case VK_ACCESS_2_MEMORY_WRITE_BIT:
         flush_bits |= RADV_CMD_FLAG_INV_VCACHE | RADV_CMD_FLAG_INV_SCACHE;
         if (!image_is_coherent)
            flush_bits |= RADV_CMD_FLAG_INV_L2;
         if (flush_CB)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB;
         if (has_CB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB_META;
         if (flush_DB)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB;
         if (has_DB_meta)
            flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;
         break;
      default:
         break;
      }
   }
   return flush_bits;
}

void
radv_emit_resolve_barrier(struct radv_cmd_buffer *cmd_buffer, const struct radv_resolve_barrier *barrier)
{
   struct radv_rendering_state *render = &cmd_buffer->state.render;

   for (uint32_t i = 0; i < render->color_att_count; i++) {
      struct radv_image_view *iview = render->color_att[i].iview;
      if (!iview)
         continue;

      cmd_buffer->state.flush_bits |= radv_src_access_flush(cmd_buffer, barrier->src_access_mask, iview->image);
   }
   if (render->ds_att.iview) {
      cmd_buffer->state.flush_bits |=
         radv_src_access_flush(cmd_buffer, barrier->src_access_mask, render->ds_att.iview->image);
   }

   radv_stage_flush(cmd_buffer, barrier->src_stage_mask);

   for (uint32_t i = 0; i < render->color_att_count; i++) {
      struct radv_image_view *iview = render->color_att[i].iview;
      if (!iview)
         continue;

      cmd_buffer->state.flush_bits |= radv_dst_access_flush(cmd_buffer, barrier->dst_access_mask, iview->image);
   }
   if (render->ds_att.iview) {
      cmd_buffer->state.flush_bits |=
         radv_dst_access_flush(cmd_buffer, barrier->dst_access_mask, render->ds_att.iview->image);
   }

   radv_gang_barrier(cmd_buffer, barrier->src_stage_mask, barrier->dst_stage_mask);
}

static void
radv_handle_image_transition_separate(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                                      VkImageLayout src_layout, VkImageLayout dst_layout,
                                      VkImageLayout src_stencil_layout, VkImageLayout dst_stencil_layout,
                                      uint32_t src_family_index, uint32_t dst_family_index,
                                      const VkImageSubresourceRange *range,
                                      struct radv_sample_locations_state *sample_locs)
{
   /* If we have a stencil layout that's different from depth, we need to
    * perform the stencil transition separately.
    */
   if ((range->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) &&
       (src_layout != src_stencil_layout || dst_layout != dst_stencil_layout)) {
      VkImageSubresourceRange aspect_range = *range;
      /* Depth-only transitions. */
      if (range->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
         aspect_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
         radv_handle_image_transition(cmd_buffer, image, src_layout, dst_layout, src_family_index, dst_family_index,
                                      &aspect_range, sample_locs);
      }

      /* Stencil-only transitions. */
      aspect_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
      radv_handle_image_transition(cmd_buffer, image, src_stencil_layout, dst_stencil_layout, src_family_index,
                                   dst_family_index, &aspect_range, sample_locs);
   } else {
      radv_handle_image_transition(cmd_buffer, image, src_layout, dst_layout, src_family_index, dst_family_index, range,
                                   sample_locs);
   }
}

static void
radv_handle_rendering_image_transition(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *view,
                                       uint32_t layer_count, uint32_t view_mask, VkImageLayout initial_layout,
                                       VkImageLayout initial_stencil_layout, VkImageLayout final_layout,
                                       VkImageLayout final_stencil_layout,
                                       struct radv_sample_locations_state *sample_locs)
{
   VkImageSubresourceRange range;
   range.aspectMask = view->image->vk.aspects;
   range.baseMipLevel = view->vk.base_mip_level;
   range.levelCount = 1;

   if (view_mask) {
      while (view_mask) {
         int start, count;
         u_bit_scan_consecutive_range(&view_mask, &start, &count);

         range.baseArrayLayer = view->vk.base_array_layer + start;
         range.layerCount = count;

         radv_handle_image_transition_separate(cmd_buffer, view->image, initial_layout, final_layout,
                                               initial_stencil_layout, final_stencil_layout, 0, 0, &range, sample_locs);
      }
   } else {
      range.baseArrayLayer = view->vk.base_array_layer;
      range.layerCount = layer_count;
      radv_handle_image_transition_separate(cmd_buffer, view->image, initial_layout, final_layout,
                                            initial_stencil_layout, final_stencil_layout, 0, 0, &range, sample_locs);
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   VkResult result = VK_SUCCESS;

   vk_command_buffer_begin(&cmd_buffer->vk, pBeginInfo);

   if (cmd_buffer->qf == RADV_QUEUE_SPARSE)
      return result;

   memset(&cmd_buffer->state, 0, sizeof(cmd_buffer->state));
   cmd_buffer->state.last_index_type = -1;
   cmd_buffer->state.last_num_instances = -1;
   cmd_buffer->state.last_vertex_offset_valid = false;
   cmd_buffer->state.last_first_instance = -1;
   cmd_buffer->state.last_drawid = -1;
   cmd_buffer->state.last_subpass_color_count = MAX_RTS;
   cmd_buffer->state.predication_type = -1;
   cmd_buffer->state.last_sx_ps_downconvert = -1;
   cmd_buffer->state.last_sx_blend_opt_epsilon = -1;
   cmd_buffer->state.last_sx_blend_opt_control = -1;
   cmd_buffer->state.mesh_shading = false;
   cmd_buffer->state.last_vrs_rates = -1;
   cmd_buffer->state.last_vrs_rates_sgpr_idx = -1;
   cmd_buffer->state.last_pa_sc_binner_cntl_0 = -1;
   cmd_buffer->state.last_db_count_control = -1;
   cmd_buffer->state.last_db_shader_control = -1;
   cmd_buffer->usage_flags = pBeginInfo->flags;

   cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_ALL | RADV_CMD_DIRTY_GUARDBAND | RADV_CMD_DIRTY_OCCLUSION_QUERY |
                              RADV_CMD_DIRTY_DB_SHADER_CONTROL;

   if (cmd_buffer->qf == RADV_QUEUE_COMPUTE || cmd_buffer->device->vk.enabled_features.taskShader) {
      uint32_t pred_value = 0;
      uint32_t pred_offset;
      if (!radv_cmd_buffer_upload_data(cmd_buffer, 4, &pred_value, &pred_offset))
         vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);

      cmd_buffer->mec_inv_pred_emitted = false;
      cmd_buffer->mec_inv_pred_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + pred_offset;
   }

   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9 && cmd_buffer->qf == RADV_QUEUE_GENERAL) {
      unsigned num_db = cmd_buffer->device->physical_device->rad_info.max_render_backends;
      unsigned fence_offset, eop_bug_offset;
      void *fence_ptr;

      radv_cmd_buffer_upload_alloc(cmd_buffer, 8, &fence_offset, &fence_ptr);
      memset(fence_ptr, 0, 8);

      cmd_buffer->gfx9_fence_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
      cmd_buffer->gfx9_fence_va += fence_offset;

      radv_emit_clear_data(cmd_buffer, V_370_PFP, cmd_buffer->gfx9_fence_va, 8);

      if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX9) {
         /* Allocate a buffer for the EOP bug on GFX9. */
         radv_cmd_buffer_upload_alloc(cmd_buffer, 16 * num_db, &eop_bug_offset, &fence_ptr);
         memset(fence_ptr, 0, 16 * num_db);
         cmd_buffer->gfx9_eop_bug_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
         cmd_buffer->gfx9_eop_bug_va += eop_bug_offset;

         radv_emit_clear_data(cmd_buffer, V_370_PFP, cmd_buffer->gfx9_eop_bug_va, 16 * num_db);
      }
   }

   if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY &&
       (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {

      char gcbiar_data[VK_GCBIARR_DATA_SIZE(MAX_RTS)];
      const VkRenderingInfo *resume_info =
         vk_get_command_buffer_inheritance_as_rendering_resume(cmd_buffer->vk.level, pBeginInfo, gcbiar_data);
      if (resume_info) {
         radv_CmdBeginRendering(commandBuffer, resume_info);
      } else {
         const VkCommandBufferInheritanceRenderingInfo *inheritance_info =
            vk_get_command_buffer_inheritance_rendering_info(cmd_buffer->vk.level, pBeginInfo);

         radv_cmd_buffer_reset_rendering(cmd_buffer);
         struct radv_rendering_state *render = &cmd_buffer->state.render;
         render->active = true;
         render->view_mask = inheritance_info->viewMask;
         render->max_samples = inheritance_info->rasterizationSamples;
         render->color_att_count = inheritance_info->colorAttachmentCount;
         for (uint32_t i = 0; i < render->color_att_count; i++) {
            render->color_att[i] = (struct radv_attachment){
               .format = inheritance_info->pColorAttachmentFormats[i],
            };
         }
         assert(inheritance_info->depthAttachmentFormat == VK_FORMAT_UNDEFINED ||
                inheritance_info->stencilAttachmentFormat == VK_FORMAT_UNDEFINED ||
                inheritance_info->depthAttachmentFormat == inheritance_info->stencilAttachmentFormat);
         render->ds_att = (struct radv_attachment){.iview = NULL};
         if (inheritance_info->depthAttachmentFormat != VK_FORMAT_UNDEFINED)
            render->ds_att.format = inheritance_info->depthAttachmentFormat;
         if (inheritance_info->stencilAttachmentFormat != VK_FORMAT_UNDEFINED)
            render->ds_att.format = inheritance_info->stencilAttachmentFormat;

         if (vk_format_has_depth(render->ds_att.format))
            render->ds_att_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
         if (vk_format_has_stencil(render->ds_att.format))
            render->ds_att_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }

      cmd_buffer->state.inherited_pipeline_statistics = pBeginInfo->pInheritanceInfo->pipelineStatistics;

      if (cmd_buffer->state.inherited_pipeline_statistics &
          (VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT |
           VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT))
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;

      cmd_buffer->state.inherited_occlusion_queries = pBeginInfo->pInheritanceInfo->occlusionQueryEnable;
      cmd_buffer->state.inherited_query_control_flags = pBeginInfo->pInheritanceInfo->queryFlags;
      if (cmd_buffer->state.inherited_occlusion_queries)
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_OCCLUSION_QUERY;
   }

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_cmd_buffer_trace_emit(cmd_buffer);

   radv_describe_begin_cmd_buffer(cmd_buffer);

   return result;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                           const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                           const VkDeviceSize *pStrides)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_vertex_binding *vb = cmd_buffer->vertex_bindings;
   const struct radv_vs_input_state *state = &cmd_buffer->state.dynamic_vs_input;

   /* We have to defer setting up vertex buffer since we need the buffer
    * stride from the pipeline. */

   assert(firstBinding + bindingCount <= MAX_VBS);
   enum amd_gfx_level chip = cmd_buffer->device->physical_device->rad_info.gfx_level;

   if (firstBinding + bindingCount > cmd_buffer->used_vertex_bindings)
      cmd_buffer->used_vertex_bindings = firstBinding + bindingCount;

   uint32_t misaligned_mask_invalid = 0;

   for (uint32_t i = 0; i < bindingCount; i++) {
      RADV_FROM_HANDLE(radv_buffer, buffer, pBuffers[i]);
      uint32_t idx = firstBinding + i;
      VkDeviceSize size = pSizes ? pSizes[i] : 0;
      /* if pStrides=NULL, it shouldn't overwrite the strides specified by CmdSetVertexInputEXT */
      VkDeviceSize stride = pStrides ? pStrides[i] : vb[idx].stride;

      if (!!cmd_buffer->vertex_binding_buffers[idx] != !!buffer ||
          (buffer && ((vb[idx].offset & 0x3) != (pOffsets[i] & 0x3) || (vb[idx].stride & 0x3) != (stride & 0x3)))) {
         misaligned_mask_invalid |= state->bindings_match_attrib ? BITFIELD_BIT(idx) : 0xffffffff;
      }

      cmd_buffer->vertex_binding_buffers[idx] = buffer;
      vb[idx].offset = pOffsets[i];
      vb[idx].size = buffer ? vk_buffer_range(&buffer->vk, pOffsets[i], size) : size;
      vb[idx].stride = stride;

      uint32_t bit = BITFIELD_BIT(idx);
      if (buffer) {
         radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, cmd_buffer->vertex_binding_buffers[idx]->bo);
         cmd_buffer->state.vbo_bound_mask |= bit;
      } else {
         cmd_buffer->state.vbo_bound_mask &= ~bit;
      }
   }

   if ((chip == GFX6 || chip >= GFX10) && misaligned_mask_invalid) {
      cmd_buffer->state.vbo_misaligned_mask_invalid = misaligned_mask_invalid;
      cmd_buffer->state.vbo_misaligned_mask &= ~misaligned_mask_invalid;
   }

   cmd_buffer->state.dirty |= RADV_CMD_DIRTY_VERTEX_BUFFER | RADV_CMD_DIRTY_DYNAMIC_VERTEX_INPUT;
}

static uint32_t
vk_to_index_type(VkIndexType type)
{
   switch (type) {
   case VK_INDEX_TYPE_UINT8_EXT:
      return V_028A7C_VGT_INDEX_8;
   case VK_INDEX_TYPE_UINT16:
      return V_028A7C_VGT_INDEX_16;
   case VK_INDEX_TYPE_UINT32:
      return V_028A7C_VGT_INDEX_32;
   default:
      unreachable("invalid index type");
   }
}

uint32_t
radv_get_vgt_index_size(uint32_t type)
{
   uint32_t index_type = G_028A7C_INDEX_TYPE(type);
   switch (index_type) {
   case V_028A7C_VGT_INDEX_8:
      return 1;
   case V_028A7C_VGT_INDEX_16:
      return 2;
   case V_028A7C_VGT_INDEX_32:
      return 4;
   default:
      unreachable("invalid index type");
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                            VkIndexType indexType)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, index_buffer, buffer);

   cmd_buffer->state.index_type = vk_to_index_type(indexType);

   if (index_buffer) {
      cmd_buffer->state.index_va = radv_buffer_get_va(index_buffer->bo);
      cmd_buffer->state.index_va += index_buffer->offset + offset;

      int index_size = radv_get_vgt_index_size(vk_to_index_type(indexType));
      cmd_buffer->state.max_index_count = (vk_buffer_range(&index_buffer->vk, offset, size)) / index_size;
      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, index_buffer->bo);
   } else {
      cmd_buffer->state.index_va = 0;
      cmd_buffer->state.max_index_count = 0;
   }

   cmd_buffer->state.dirty |= RADV_CMD_DIRTY_INDEX_BUFFER;

   /* Primitive restart state depends on the index type. */
   if (cmd_buffer->state.dynamic.vk.ia.primitive_restart_enable)
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_RESTART_ENABLE;
}

static void
radv_bind_descriptor_set(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point,
                         struct radv_descriptor_set *set, unsigned idx)
{
   struct radeon_winsys *ws = cmd_buffer->device->ws;

   radv_set_descriptor_set(cmd_buffer, bind_point, set, idx);

   assert(set);
   assert(!(set->header.layout->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR));

   if (!cmd_buffer->device->use_global_bo_list) {
      for (unsigned j = 0; j < set->header.buffer_count; ++j)
         if (set->descriptors[j])
            radv_cs_add_buffer(ws, cmd_buffer->cs, set->descriptors[j]);
   }

   if (set->header.bo)
      radv_cs_add_buffer(ws, cmd_buffer->cs, set->header.bo);
}

static void
radv_bind_descriptor_sets(struct radv_cmd_buffer *cmd_buffer,
                          const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo, VkPipelineBindPoint bind_point)
{
   RADV_FROM_HANDLE(radv_pipeline_layout, layout, pBindDescriptorSetsInfo->layout);
   const bool no_dynamic_bounds = cmd_buffer->device->instance->debug_flags & RADV_DEBUG_NO_DYNAMIC_BOUNDS;
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);
   unsigned dyn_idx = 0;

   for (unsigned i = 0; i < pBindDescriptorSetsInfo->descriptorSetCount; ++i) {
      unsigned set_idx = i + pBindDescriptorSetsInfo->firstSet;
      RADV_FROM_HANDLE(radv_descriptor_set, set, pBindDescriptorSetsInfo->pDescriptorSets[i]);

      if (!set)
         continue;

      /* If the set is already bound we only need to update the
       * (potentially changed) dynamic offsets. */
      if (descriptors_state->sets[set_idx] != set || !(descriptors_state->valid & (1u << set_idx))) {
         radv_bind_descriptor_set(cmd_buffer, bind_point, set, set_idx);
      }

      for (unsigned j = 0; j < set->header.layout->dynamic_offset_count; ++j, ++dyn_idx) {
         unsigned idx = j + layout->set[i + pBindDescriptorSetsInfo->firstSet].dynamic_offset_start;
         uint32_t *dst = descriptors_state->dynamic_buffers + idx * 4;
         assert(dyn_idx < pBindDescriptorSetsInfo->dynamicOffsetCount);

         struct radv_descriptor_range *range = set->header.dynamic_descriptors + j;

         if (!range->va) {
            memset(dst, 0, 4 * 4);
         } else {
            uint64_t va = range->va + pBindDescriptorSetsInfo->pDynamicOffsets[dyn_idx];
            dst[0] = va;
            dst[1] = S_008F04_BASE_ADDRESS_HI(va >> 32);
            dst[2] = no_dynamic_bounds ? 0xffffffffu : range->size;
            dst[3] = S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) | S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
                     S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) | S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W);

            if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
               dst[3] |= S_008F0C_FORMAT(V_008F0C_GFX11_FORMAT_32_FLOAT) | S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW);
            } else if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10) {
               dst[3] |= S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_FLOAT) |
                         S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW) | S_008F0C_RESOURCE_LEVEL(1);
            } else {
               dst[3] |= S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_FLOAT) |
                         S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);
            }
         }

         cmd_buffer->push_constant_stages |= set->header.layout->dynamic_shader_stages;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                               const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   if (pBindDescriptorSetsInfo->stageFlags & VK_SHADER_STAGE_COMPUTE_BIT) {
      radv_bind_descriptor_sets(cmd_buffer, pBindDescriptorSetsInfo, VK_PIPELINE_BIND_POINT_COMPUTE);
   }

   if (pBindDescriptorSetsInfo->stageFlags & RADV_GRAPHICS_STAGE_BITS) {
      radv_bind_descriptor_sets(cmd_buffer, pBindDescriptorSetsInfo, VK_PIPELINE_BIND_POINT_GRAPHICS);
   }

   if (pBindDescriptorSetsInfo->stageFlags & RADV_RT_STAGE_BITS) {
      radv_bind_descriptor_sets(cmd_buffer, pBindDescriptorSetsInfo, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
   }
}

static bool
radv_init_push_descriptor_set(struct radv_cmd_buffer *cmd_buffer, struct radv_descriptor_set *set,
                              struct radv_descriptor_set_layout *layout, VkPipelineBindPoint bind_point)
{
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);
   set->header.size = layout->size;

   if (set->header.layout != layout) {
      if (set->header.layout)
         vk_descriptor_set_layout_unref(&cmd_buffer->device->vk, &set->header.layout->vk);
      vk_descriptor_set_layout_ref(&layout->vk);
      set->header.layout = layout;
   }

   if (descriptors_state->push_set.capacity < set->header.size) {
      size_t new_size = MAX2(set->header.size, 1024);
      new_size = MAX2(new_size, 2 * descriptors_state->push_set.capacity);
      new_size = MIN2(new_size, 96 * MAX_PUSH_DESCRIPTORS);

      free(set->header.mapped_ptr);
      set->header.mapped_ptr = malloc(new_size);

      if (!set->header.mapped_ptr) {
         descriptors_state->push_set.capacity = 0;
         vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
         return false;
      }

      descriptors_state->push_set.capacity = new_size;
   }

   return true;
}

void
radv_meta_push_descriptor_set(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint pipelineBindPoint,
                              VkPipelineLayout _layout, uint32_t set, uint32_t descriptorWriteCount,
                              const VkWriteDescriptorSet *pDescriptorWrites)
{
   RADV_FROM_HANDLE(radv_pipeline_layout, layout, _layout);
   struct radv_descriptor_set *push_set = (struct radv_descriptor_set *)&cmd_buffer->meta_push_descriptors;
   unsigned bo_offset;

   assert(set == 0);
   assert(layout->set[set].layout->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

   push_set->header.size = layout->set[set].layout->size;
   push_set->header.layout = layout->set[set].layout;

   if (!radv_cmd_buffer_upload_alloc(cmd_buffer, push_set->header.size, &bo_offset,
                                     (void **)&push_set->header.mapped_ptr))
      return;

   push_set->header.va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
   push_set->header.va += bo_offset;

   radv_cmd_update_descriptor_sets(cmd_buffer->device, cmd_buffer, radv_descriptor_set_to_handle(push_set),
                                   descriptorWriteCount, pDescriptorWrites, 0, NULL);

   radv_set_descriptor_set(cmd_buffer, pipelineBindPoint, push_set, set);
}

static void
radv_push_descriptor_set(struct radv_cmd_buffer *cmd_buffer, const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                         VkPipelineBindPoint bind_point)
{
   RADV_FROM_HANDLE(radv_pipeline_layout, layout, pPushDescriptorSetInfo->layout);
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);
   struct radv_descriptor_set *push_set = (struct radv_descriptor_set *)&descriptors_state->push_set.set;

   assert(layout->set[pPushDescriptorSetInfo->set].layout->flags &
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

   if (!radv_init_push_descriptor_set(cmd_buffer, push_set, layout->set[pPushDescriptorSetInfo->set].layout,
                                      bind_point))
      return;

   /* Check that there are no inline uniform block updates when calling vkCmdPushDescriptorSetKHR()
    * because it is invalid, according to Vulkan spec.
    */
   for (int i = 0; i < pPushDescriptorSetInfo->descriptorWriteCount; i++) {
      ASSERTED const VkWriteDescriptorSet *writeset = &pPushDescriptorSetInfo->pDescriptorWrites[i];
      assert(writeset->descriptorType != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK);
   }

   radv_cmd_update_descriptor_sets(cmd_buffer->device, cmd_buffer, radv_descriptor_set_to_handle(push_set),
                                   pPushDescriptorSetInfo->descriptorWriteCount,
                                   pPushDescriptorSetInfo->pDescriptorWrites, 0, NULL);

   radv_set_descriptor_set(cmd_buffer, bind_point, push_set, pPushDescriptorSetInfo->set);

   radv_flush_push_descriptors(cmd_buffer, descriptors_state);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   if (pPushDescriptorSetInfo->stageFlags & VK_SHADER_STAGE_COMPUTE_BIT) {
      radv_push_descriptor_set(cmd_buffer, pPushDescriptorSetInfo, VK_PIPELINE_BIND_POINT_COMPUTE);
   }

   if (pPushDescriptorSetInfo->stageFlags & RADV_GRAPHICS_STAGE_BITS) {
      radv_push_descriptor_set(cmd_buffer, pPushDescriptorSetInfo, VK_PIPELINE_BIND_POINT_GRAPHICS);
   }

   if (pPushDescriptorSetInfo->stageFlags & RADV_RT_STAGE_BITS) {
      radv_push_descriptor_set(cmd_buffer, pPushDescriptorSetInfo, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdPushDescriptorSetWithTemplate2KHR(
   VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR *pPushDescriptorSetWithTemplateInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_pipeline_layout, layout, pPushDescriptorSetWithTemplateInfo->layout);
   RADV_FROM_HANDLE(radv_descriptor_update_template, templ,
                    pPushDescriptorSetWithTemplateInfo->descriptorUpdateTemplate);
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, templ->bind_point);
   struct radv_descriptor_set *push_set = (struct radv_descriptor_set *)&descriptors_state->push_set.set;

   assert(layout->set[pPushDescriptorSetWithTemplateInfo->set].layout->flags &
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

   if (!radv_init_push_descriptor_set(cmd_buffer, push_set, layout->set[pPushDescriptorSetWithTemplateInfo->set].layout,
                                      templ->bind_point))
      return;

   radv_cmd_update_descriptor_set_with_template(cmd_buffer->device, cmd_buffer, push_set,
                                                pPushDescriptorSetWithTemplateInfo->descriptorUpdateTemplate,
                                                pPushDescriptorSetWithTemplateInfo->pData);

   radv_set_descriptor_set(cmd_buffer, templ->bind_point, push_set, pPushDescriptorSetWithTemplateInfo->set);

   radv_flush_push_descriptors(cmd_buffer, descriptors_state);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfoKHR *pPushConstantsInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   memcpy(cmd_buffer->push_constants + pPushConstantsInfo->offset, pPushConstantsInfo->pValues,
          pPushConstantsInfo->size);
   cmd_buffer->push_constant_stages |= pPushConstantsInfo->stageFlags;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_EndCommandBuffer(VkCommandBuffer commandBuffer)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   if (cmd_buffer->qf == RADV_QUEUE_SPARSE)
      return vk_command_buffer_end(&cmd_buffer->vk);

   radv_emit_mip_change_flush_default(cmd_buffer);

   const bool is_gfx_or_ace = cmd_buffer->qf == RADV_QUEUE_GENERAL || cmd_buffer->qf == RADV_QUEUE_COMPUTE;

   if (is_gfx_or_ace) {
      if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX6)
         cmd_buffer->state.flush_bits |=
            RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_WB_L2;

      /* Make sure to sync all pending active queries at the end of
       * command buffer.
       */
      cmd_buffer->state.flush_bits |= cmd_buffer->active_query_flush_bits;

      /* Flush noncoherent images on GFX9+ so we can assume they're clean on the start of a
       * command buffer.
       */
      if (cmd_buffer->state.rb_noncoherent_dirty && !can_skip_buffer_l2_flushes(cmd_buffer->device))
         cmd_buffer->state.flush_bits |= radv_src_access_flush(
            cmd_buffer, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, NULL);

      /* Since NGG streamout uses GDS, we need to make GDS idle when
       * we leave the IB, otherwise another process might overwrite
       * it while our shaders are busy.
       */
      if (cmd_buffer->gds_needed)
         cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_PS_PARTIAL_FLUSH;
   }

   /* Finalize the internal compute command stream, if it exists. */
   if (cmd_buffer->gang.cs) {
      VkResult result = radv_gang_finalize(cmd_buffer);
      if (result != VK_SUCCESS)
         return vk_error(cmd_buffer, result);
   }

   if (is_gfx_or_ace) {
      radv_emit_cache_flush(cmd_buffer);

      /* Make sure CP DMA is idle at the end of IBs because the kernel
       * doesn't wait for it.
       */
      radv_cp_dma_wait_for_idle(cmd_buffer);
   }

   radv_describe_end_cmd_buffer(cmd_buffer);

   VkResult result = cmd_buffer->device->ws->cs_finalize(cmd_buffer->cs);
   if (result != VK_SUCCESS)
      return vk_error(cmd_buffer, result);

   return vk_command_buffer_end(&cmd_buffer->vk);
}

static void
radv_emit_compute_pipeline(struct radv_cmd_buffer *cmd_buffer, struct radv_compute_pipeline *pipeline)
{
   if (pipeline == cmd_buffer->state.emitted_compute_pipeline)
      return;

   assert(!pipeline->base.ctx_cs.cdw);

   cmd_buffer->state.emitted_compute_pipeline = pipeline;

   radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, pipeline->base.cs.cdw);
   radeon_emit_array(cmd_buffer->cs, pipeline->base.cs.buf, pipeline->base.cs.cdw);

   if (pipeline->base.type == RADV_PIPELINE_COMPUTE) {
      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, cmd_buffer->state.shaders[MESA_SHADER_COMPUTE]->bo);
   } else {
      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, cmd_buffer->state.rt_prolog->bo);

      if (cmd_buffer->state.shaders[MESA_SHADER_INTERSECTION])
         radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs,
                            cmd_buffer->state.shaders[MESA_SHADER_INTERSECTION]->bo);

      struct radv_ray_tracing_pipeline *rt_pipeline = radv_pipeline_to_ray_tracing(&pipeline->base);
      for (unsigned i = 0; i < rt_pipeline->stage_count; ++i) {
         struct radv_shader *shader = rt_pipeline->stages[i].shader;
         if (shader)
            radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, shader->bo);
      }
   }

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_save_pipeline(cmd_buffer, &pipeline->base);
}

static void
radv_mark_descriptor_sets_dirty(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point)
{
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);

   descriptors_state->dirty |= descriptors_state->valid;
}

static void
radv_bind_vs_input_state(struct radv_cmd_buffer *cmd_buffer, const struct radv_graphics_pipeline *pipeline)
{
   const struct radv_shader *vs_shader = radv_get_shader(cmd_buffer->state.shaders, MESA_SHADER_VERTEX);
   const struct radv_vs_input_state *src = &pipeline->vs_input_state;

   /* Bind the vertex input state from the pipeline when the VS has a prolog and the state isn't
    * dynamic. This can happen when the pre-rasterization stages and the vertex input state are from
    * two different libraries. Otherwise, if the VS has a prolog, the state is dynamic and there is
    * nothing to bind.
    */
   if (!vs_shader || !vs_shader->info.vs.has_prolog || (pipeline->dynamic_states & RADV_DYNAMIC_VERTEX_INPUT))
      return;

   cmd_buffer->state.dynamic_vs_input = *src;

   if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX6 ||
       cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10) {
      cmd_buffer->state.vbo_misaligned_mask = 0;
      cmd_buffer->state.vbo_misaligned_mask_invalid = src->attribute_mask;
   }

   cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_VERTEX_INPUT;
}

static void
radv_bind_multisample_state(struct radv_cmd_buffer *cmd_buffer, const struct radv_multisample_state *ms)
{
   if (ms->sample_shading_enable) {
      cmd_buffer->state.ms.sample_shading_enable = true;
      cmd_buffer->state.ms.min_sample_shading = ms->min_sample_shading;
   }
}

static void
radv_bind_pre_rast_shader(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *shader)
{
   bool mesh_shading = shader->info.stage == MESA_SHADER_MESH;
   const struct radv_userdata_info *loc;

   assert(shader->info.stage == MESA_SHADER_VERTEX || shader->info.stage == MESA_SHADER_TESS_CTRL ||
          shader->info.stage == MESA_SHADER_TESS_EVAL || shader->info.stage == MESA_SHADER_GEOMETRY ||
          shader->info.stage == MESA_SHADER_MESH);

   if (radv_get_user_sgpr(shader, AC_UD_NGG_PROVOKING_VTX)->sgpr_idx != -1) {
      /* Re-emit the provoking vertex mode state because the SGPR idx can be different. */
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_PROVOKING_VERTEX_MODE;
   }

   if (radv_get_user_sgpr(shader, AC_UD_STREAMOUT_BUFFERS)->sgpr_idx != -1) {
      /* Re-emit the streamout buffers because the SGPR idx can be different and with NGG streamout
       * they always need to be emitted because a buffer size of 0 is used to disable streamout.
       */
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_STREAMOUT_BUFFER;

      if (cmd_buffer->device->physical_device->use_ngg_streamout) {
         /* GFX11 needs GDS OA for streamout. */
         cmd_buffer->gds_oa_needed = true;
      }
   }

   if (radv_get_user_sgpr(shader, AC_UD_NUM_VERTS_PER_PRIM)->sgpr_idx != -1) {
      /* Re-emit the primitive topology because the SGPR idx can be different. */
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_TOPOLOGY;
   }

   if (radv_get_user_sgpr(shader, AC_UD_SHADER_QUERY_STATE)->sgpr_idx != -1) {
      /* Re-emit shader query state when SGPR exists but location potentially changed. */
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;
   }

   loc = radv_get_user_sgpr(shader, AC_UD_VS_BASE_VERTEX_START_INSTANCE);
   if (loc->sgpr_idx != -1) {
      cmd_buffer->state.vtx_base_sgpr = shader->info.user_data_0 + loc->sgpr_idx * 4;
      cmd_buffer->state.vtx_emit_num = loc->num_sgprs;
      cmd_buffer->state.uses_drawid = shader->info.vs.needs_draw_id;
      cmd_buffer->state.uses_baseinstance = shader->info.vs.needs_base_instance;

      /* Re-emit some vertex states because the SGPR idx can be different. */
      cmd_buffer->state.last_first_instance = -1;
      cmd_buffer->state.last_vertex_offset_valid = false;
      cmd_buffer->state.last_drawid = -1;
   }

   if (mesh_shading != cmd_buffer->state.mesh_shading) {
      /* Re-emit VRS state because the combiner is different (vertex vs primitive). Re-emit
       * primitive topology because the mesh shading pipeline clobbered it.
       */
      cmd_buffer->state.dirty |=
         RADV_CMD_DIRTY_DYNAMIC_FRAGMENT_SHADING_RATE | RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_TOPOLOGY;
   }

   cmd_buffer->state.mesh_shading = mesh_shading;
}

static void
radv_bind_vertex_shader(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *vs)
{
   radv_bind_pre_rast_shader(cmd_buffer, vs);

   /* Can't put anything else here due to merged shaders */
}

static void
radv_bind_tess_ctrl_shader(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *tcs)
{
   radv_bind_pre_rast_shader(cmd_buffer, tcs);

   cmd_buffer->tess_rings_needed = true;

   /* Always re-emit patch control points/domain origin when a new pipeline with tessellation is
    * bound because a bunch of parameters (user SGPRs, TCS vertices out, ccw, etc) can be different.
    */
   cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_PATCH_CONTROL_POINTS | RADV_CMD_DIRTY_DYNAMIC_TESS_DOMAIN_ORIGIN;
}

static void
radv_bind_tess_eval_shader(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *tes)
{
   radv_bind_pre_rast_shader(cmd_buffer, tes);

   /* Can't put anything else here due to merged shaders */
}

static void
radv_bind_geometry_shader(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *gs)
{
   radv_bind_pre_rast_shader(cmd_buffer, gs);

   cmd_buffer->esgs_ring_size_needed = MAX2(cmd_buffer->esgs_ring_size_needed, gs->info.gs_ring_info.esgs_ring_size);
   cmd_buffer->gsvs_ring_size_needed = MAX2(cmd_buffer->gsvs_ring_size_needed, gs->info.gs_ring_info.gsvs_ring_size);
}

static void
radv_bind_mesh_shader(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *ms)
{
   radv_bind_pre_rast_shader(cmd_buffer, ms);

   cmd_buffer->mesh_scratch_ring_needed |= ms->info.ms.needs_ms_scratch_ring;
}

static void
radv_bind_fragment_shader(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *ps)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   const struct radv_shader *previous_ps = cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT];
   const float min_sample_shading = 1.0f;

   if (ps->info.ps.needs_sample_positions) {
      cmd_buffer->sample_positions_needed = true;
   }

   /* Re-emit the FS state because the SGPR idx can be different. */
   if (radv_get_user_sgpr(ps, AC_UD_PS_STATE)->sgpr_idx != -1) {
      cmd_buffer->state.dirty |=
         RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES | RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE;
   }

   /* Re-emit the conservative rasterization mode because inner coverage is different. */
   if (!previous_ps || previous_ps->info.ps.reads_fully_covered != ps->info.ps.reads_fully_covered)
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_CONSERVATIVE_RAST_MODE;

   if (gfx_level >= GFX10_3 && (!previous_ps || previous_ps->info.ps.force_sample_iter_shading_rate !=
                                                   ps->info.ps.force_sample_iter_shading_rate))
      cmd_buffer->state.dirty |=
         RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES | RADV_CMD_DIRTY_DYNAMIC_FRAGMENT_SHADING_RATE;

   if (cmd_buffer->state.ms.sample_shading_enable != ps->info.ps.uses_sample_shading) {
      cmd_buffer->state.ms.sample_shading_enable = ps->info.ps.uses_sample_shading;
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES;

      if (gfx_level >= GFX10_3)
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_FRAGMENT_SHADING_RATE;
   }

   if (cmd_buffer->state.ms.min_sample_shading != min_sample_shading) {
      cmd_buffer->state.ms.min_sample_shading = min_sample_shading;
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES;
   }

   if (!previous_ps || previous_ps->info.ps.db_shader_control != ps->info.ps.db_shader_control ||
       previous_ps->info.ps.pops_is_per_sample != ps->info.ps.pops_is_per_sample)
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DB_SHADER_CONTROL;

   /* Re-emit the PS epilog when a new fragment shader is bound. */
   if (ps->info.has_epilog)
      cmd_buffer->state.emitted_ps_epilog = NULL;
}

static void
radv_bind_task_shader(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *ts)
{
   if (!radv_gang_init(cmd_buffer))
      return;

   cmd_buffer->task_rings_needed = true;
}

/* This function binds/unbinds a shader to the cmdbuffer state. */
static void
radv_bind_shader(struct radv_cmd_buffer *cmd_buffer, struct radv_shader *shader, gl_shader_stage stage)
{
   const struct radv_device *device = cmd_buffer->device;

   if (!shader) {
      cmd_buffer->state.shaders[stage] = NULL;
      cmd_buffer->state.active_stages &= ~mesa_to_vk_shader_stage(stage);

      /* Reset some dynamic states when a shader stage is unbound. */
      switch (stage) {
      case MESA_SHADER_FRAGMENT:
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_CONSERVATIVE_RAST_MODE |
                                    RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES |
                                    RADV_CMD_DIRTY_DYNAMIC_FRAGMENT_SHADING_RATE | RADV_CMD_DIRTY_DB_SHADER_CONTROL;
         break;
      default:
         break;
      }
      return;
   }

   switch (stage) {
   case MESA_SHADER_VERTEX:
      radv_bind_vertex_shader(cmd_buffer, shader);
      break;
   case MESA_SHADER_TESS_CTRL:
      radv_bind_tess_ctrl_shader(cmd_buffer, shader);
      break;
   case MESA_SHADER_TESS_EVAL:
      radv_bind_tess_eval_shader(cmd_buffer, shader);
      break;
   case MESA_SHADER_GEOMETRY:
      radv_bind_geometry_shader(cmd_buffer, shader);
      break;
   case MESA_SHADER_FRAGMENT:
      radv_bind_fragment_shader(cmd_buffer, shader);
      break;
   case MESA_SHADER_MESH:
      radv_bind_mesh_shader(cmd_buffer, shader);
      break;
   case MESA_SHADER_TASK:
      radv_bind_task_shader(cmd_buffer, shader);
      break;
   case MESA_SHADER_COMPUTE: {
      cmd_buffer->compute_scratch_size_per_wave_needed =
         MAX2(cmd_buffer->compute_scratch_size_per_wave_needed, shader->config.scratch_bytes_per_wave);

      const unsigned max_stage_waves = radv_get_max_scratch_waves(device, shader);
      cmd_buffer->compute_scratch_waves_wanted = MAX2(cmd_buffer->compute_scratch_waves_wanted, max_stage_waves);
      break;
   }
   case MESA_SHADER_INTERSECTION:
      /* no-op */
      break;
   default:
      unreachable("invalid shader stage");
   }

   cmd_buffer->state.shaders[stage] = shader;
   cmd_buffer->state.active_stages |= mesa_to_vk_shader_stage(stage);

   if (mesa_to_vk_shader_stage(stage) & RADV_GRAPHICS_STAGE_BITS) {
      cmd_buffer->scratch_size_per_wave_needed =
         MAX2(cmd_buffer->scratch_size_per_wave_needed, shader->config.scratch_bytes_per_wave);

      const unsigned max_stage_waves = radv_get_max_scratch_waves(device, shader);
      cmd_buffer->scratch_waves_wanted = MAX2(cmd_buffer->scratch_waves_wanted, max_stage_waves);
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline _pipeline)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_pipeline, pipeline, _pipeline);

   switch (pipelineBindPoint) {
   case VK_PIPELINE_BIND_POINT_COMPUTE: {
      struct radv_compute_pipeline *compute_pipeline = radv_pipeline_to_compute(pipeline);

      if (cmd_buffer->state.compute_pipeline == compute_pipeline)
         return;
      radv_mark_descriptor_sets_dirty(cmd_buffer, pipelineBindPoint);

      radv_bind_shader(cmd_buffer, compute_pipeline->base.shaders[MESA_SHADER_COMPUTE], MESA_SHADER_COMPUTE);

      cmd_buffer->state.compute_pipeline = compute_pipeline;
      cmd_buffer->push_constant_stages |= VK_SHADER_STAGE_COMPUTE_BIT;
      break;
   }
   case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR: {
      struct radv_ray_tracing_pipeline *rt_pipeline = radv_pipeline_to_ray_tracing(pipeline);

      if (cmd_buffer->state.rt_pipeline == rt_pipeline)
         return;
      radv_mark_descriptor_sets_dirty(cmd_buffer, pipelineBindPoint);

      radv_bind_shader(cmd_buffer, rt_pipeline->base.base.shaders[MESA_SHADER_INTERSECTION], MESA_SHADER_INTERSECTION);
      cmd_buffer->state.rt_prolog = rt_pipeline->prolog;

      cmd_buffer->state.rt_pipeline = rt_pipeline;
      cmd_buffer->push_constant_stages |= RADV_RT_STAGE_BITS;

      /* Bind the stack size when it's not dynamic. */
      if (rt_pipeline->stack_size != -1u)
         cmd_buffer->state.rt_stack_size = rt_pipeline->stack_size;

      const unsigned max_scratch_waves = radv_get_max_scratch_waves(cmd_buffer->device, rt_pipeline->prolog);
      cmd_buffer->compute_scratch_waves_wanted = MAX2(cmd_buffer->compute_scratch_waves_wanted, max_scratch_waves);
      break;
   }
   case VK_PIPELINE_BIND_POINT_GRAPHICS: {
      struct radv_graphics_pipeline *graphics_pipeline = radv_pipeline_to_graphics(pipeline);

      /* Bind the non-dynamic graphics state from the pipeline unconditionally because some PSO
       * might have been overwritten between two binds of the same pipeline.
       */
      radv_bind_dynamic_state(cmd_buffer, &graphics_pipeline->dynamic_state);

      if (cmd_buffer->state.graphics_pipeline == graphics_pipeline)
         return;
      radv_mark_descriptor_sets_dirty(cmd_buffer, pipelineBindPoint);

      radv_foreach_stage(
         stage, (cmd_buffer->state.active_stages | graphics_pipeline->active_stages) & RADV_GRAPHICS_STAGE_BITS)
      {
         radv_bind_shader(cmd_buffer, graphics_pipeline->base.shaders[stage], stage);
      }

      cmd_buffer->state.gs_copy_shader = graphics_pipeline->base.gs_copy_shader;
      cmd_buffer->state.last_vgt_shader = graphics_pipeline->base.shaders[graphics_pipeline->last_vgt_api_stage];

      cmd_buffer->state.graphics_pipeline = graphics_pipeline;

      cmd_buffer->state.has_nggc = graphics_pipeline->has_ngg_culling;
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_PIPELINE;
      cmd_buffer->push_constant_stages |= graphics_pipeline->active_stages;

      /* Prefetch all pipeline shaders at first draw time. */
      cmd_buffer->state.prefetch_L2_mask |= RADV_PREFETCH_SHADERS;

      if (cmd_buffer->device->physical_device->rad_info.has_vgt_flush_ngg_legacy_bug &&
          cmd_buffer->state.emitted_graphics_pipeline && cmd_buffer->state.emitted_graphics_pipeline->is_ngg &&
          !cmd_buffer->state.graphics_pipeline->is_ngg) {
         /* Transitioning from NGG to legacy GS requires
          * VGT_FLUSH on GFX10 and Navi21. VGT_FLUSH
          * is also emitted at the beginning of IBs when legacy
          * GS ring pointers are set.
          */
         cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_VGT_FLUSH;
      }

      cmd_buffer->state.uses_dynamic_patch_control_points =
         !!(graphics_pipeline->dynamic_states & RADV_DYNAMIC_PATCH_CONTROL_POINTS);

      if (graphics_pipeline->active_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
         if (!cmd_buffer->state.uses_dynamic_patch_control_points) {
            /* Bind the tessellation state from the pipeline when it's not dynamic. */
            struct radv_shader *tcs = cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL];

            cmd_buffer->state.tess_num_patches = tcs->info.num_tess_patches;
            cmd_buffer->state.tess_lds_size = tcs->info.tcs.num_lds_blocks;
         }
      }

      const struct radv_shader *vs = radv_get_shader(graphics_pipeline->base.shaders, MESA_SHADER_VERTEX);
      if (vs) {
         /* Re-emit the VS prolog when a new vertex shader is bound. */
         if (vs->info.vs.has_prolog) {
            cmd_buffer->state.emitted_vs_prolog = NULL;
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_VERTEX_INPUT;
         }

         /* Re-emit the vertex buffer descriptors because they are really tied to the pipeline. */
         if (vs->info.vs.vb_desc_usage_mask) {
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_VERTEX_BUFFER;
         }
      }

      if (cmd_buffer->device->physical_device->rad_info.rbplus_allowed &&
          (!cmd_buffer->state.emitted_graphics_pipeline ||
           cmd_buffer->state.col_format_non_compacted != graphics_pipeline->col_format_non_compacted)) {
         cmd_buffer->state.col_format_non_compacted = graphics_pipeline->col_format_non_compacted;
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_RBPLUS;
      }

      radv_bind_vs_input_state(cmd_buffer, graphics_pipeline);

      radv_bind_multisample_state(cmd_buffer, &graphics_pipeline->ms);

      cmd_buffer->state.custom_blend_mode = graphics_pipeline->custom_blend_mode;
      cmd_buffer->state.db_render_control = graphics_pipeline->db_render_control;

      cmd_buffer->state.rast_prim = graphics_pipeline->rast_prim;

      cmd_buffer->state.ia_multi_vgt_param = graphics_pipeline->ia_multi_vgt_param;

      cmd_buffer->state.uses_out_of_order_rast = graphics_pipeline->uses_out_of_order_rast;
      cmd_buffer->state.uses_vrs_attachment = graphics_pipeline->uses_vrs_attachment;
      cmd_buffer->state.uses_dynamic_vertex_binding_stride =
         !!(graphics_pipeline->dynamic_states & (RADV_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE | RADV_DYNAMIC_VERTEX_INPUT));
      break;
   }
   default:
      assert(!"invalid bind point");
      break;
   }

   cmd_buffer->push_constant_state[vk_to_bind_point(pipelineBindPoint)].size = pipeline->push_constant_size;
   cmd_buffer->push_constant_state[vk_to_bind_point(pipelineBindPoint)].dynamic_offset_count =
      pipeline->dynamic_offset_count;
   cmd_buffer->descriptors[vk_to_bind_point(pipelineBindPoint)].need_indirect_descriptor_sets =
      pipeline->need_indirect_descriptor_sets;

   if (cmd_buffer->device->shader_use_invisible_vram)
      cmd_buffer->shader_upload_seq = MAX2(cmd_buffer->shader_upload_seq, pipeline->shader_upload_seq);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                    const VkViewport *pViewports)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;
   ASSERTED const uint32_t total_count = firstViewport + viewportCount;

   assert(firstViewport < MAX_VIEWPORTS);
   assert(total_count >= 1 && total_count <= MAX_VIEWPORTS);

   if (state->dynamic.vk.vp.viewport_count < total_count)
      state->dynamic.vk.vp.viewport_count = total_count;

   memcpy(state->dynamic.vk.vp.viewports + firstViewport, pViewports, viewportCount * sizeof(*pViewports));
   for (unsigned i = 0; i < viewportCount; i++) {
      radv_get_viewport_xform(&pViewports[i], state->dynamic.hw_vp.xform[i + firstViewport].scale,
                              state->dynamic.hw_vp.xform[i + firstViewport].translate);
   }

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_VIEWPORT | RADV_CMD_DIRTY_GUARDBAND;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                   const VkRect2D *pScissors)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;
   ASSERTED const uint32_t total_count = firstScissor + scissorCount;

   assert(firstScissor < MAX_SCISSORS);
   assert(total_count >= 1 && total_count <= MAX_SCISSORS);

   if (state->dynamic.vk.vp.scissor_count < total_count)
      state->dynamic.vk.vp.scissor_count = total_count;

   memcpy(state->dynamic.vk.vp.scissors + firstScissor, pScissors, scissorCount * sizeof(*pScissors));

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_SCISSOR;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.line.width = lineWidth;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_LINE_WIDTH | RADV_CMD_DIRTY_GUARDBAND;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   memcpy(state->dynamic.vk.cb.blend_constants, blendConstants, sizeof(float) * 4);

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_BLEND_CONSTANTS;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ds.depth.bounds_test.min = minDepthBounds;
   state->dynamic.vk.ds.depth.bounds_test.max = maxDepthBounds;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_BOUNDS;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      state->dynamic.vk.ds.stencil.front.compare_mask = compareMask;
   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      state->dynamic.vk.ds.stencil.back.compare_mask = compareMask;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_STENCIL_COMPARE_MASK;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      state->dynamic.vk.ds.stencil.front.write_mask = writeMask;
   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      state->dynamic.vk.ds.stencil.back.write_mask = writeMask;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_STENCIL_WRITE_MASK;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      state->dynamic.vk.ds.stencil.front.reference = reference;
   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      state->dynamic.vk.ds.stencil.back.reference = reference;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_STENCIL_REFERENCE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                               uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;
   ASSERTED const uint32_t total_count = firstDiscardRectangle + discardRectangleCount;

   assert(firstDiscardRectangle < MAX_DISCARD_RECTANGLES);
   assert(total_count >= 1 && total_count <= MAX_DISCARD_RECTANGLES);

   typed_memcpy(&state->dynamic.vk.dr.rectangles[firstDiscardRectangle], pDiscardRectangles, discardRectangleCount);

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT *pSampleLocationsInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   assert(pSampleLocationsInfo->sampleLocationsCount <= MAX_SAMPLE_LOCATIONS);

   state->dynamic.sample_location.per_pixel = pSampleLocationsInfo->sampleLocationsPerPixel;
   state->dynamic.sample_location.grid_size = pSampleLocationsInfo->sampleLocationGridSize;
   state->dynamic.sample_location.count = pSampleLocationsInfo->sampleLocationsCount;
   typed_memcpy(&state->dynamic.sample_location.locations[0], pSampleLocationsInfo->pSampleLocations,
                pSampleLocationsInfo->sampleLocationsCount);

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_SAMPLE_LOCATIONS;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.line.stipple.factor = lineStippleFactor;
   state->dynamic.vk.rs.line.stipple.pattern = lineStipplePattern;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_LINE_STIPPLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.cull_mode = cullMode;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_CULL_MODE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.front_face = frontFace;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_FRONT_FACE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;
   unsigned primitive_topology = radv_translate_prim(primitiveTopology);

   if (radv_primitive_topology_is_line_list(state->dynamic.vk.ia.primitive_topology) !=
       radv_primitive_topology_is_line_list(primitive_topology))
      state->dirty |= RADV_CMD_DIRTY_DYNAMIC_LINE_STIPPLE;

   if (radv_prim_is_points_or_lines(state->dynamic.vk.ia.primitive_topology) !=
       radv_prim_is_points_or_lines(primitive_topology))
      state->dirty |= RADV_CMD_DIRTY_GUARDBAND;

   state->dynamic.vk.ia.primitive_topology = primitive_topology;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_TOPOLOGY;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports)
{
   radv_CmdSetViewport(commandBuffer, 0, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors)
{
   radv_CmdSetScissor(commandBuffer, 0, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable)

{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ds.depth.test_enable = depthTestEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_TEST_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ds.depth.write_enable = depthWriteEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_WRITE_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ds.depth.compare_op = depthCompareOp;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_COMPARE_OP;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ds.depth.bounds_test.enable = depthBoundsTestEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ds.stencil.test_enable = stencilTestEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_STENCIL_TEST_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp,
                     VkStencilOp depthFailOp, VkCompareOp compareOp)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT) {
      state->dynamic.vk.ds.stencil.front.op.fail = failOp;
      state->dynamic.vk.ds.stencil.front.op.pass = passOp;
      state->dynamic.vk.ds.stencil.front.op.depth_fail = depthFailOp;
      state->dynamic.vk.ds.stencil.front.op.compare = compareOp;
   }

   if (faceMask & VK_STENCIL_FACE_BACK_BIT) {
      state->dynamic.vk.ds.stencil.back.op.fail = failOp;
      state->dynamic.vk.ds.stencil.back.op.pass = passOp;
      state->dynamic.vk.ds.stencil.back.op.depth_fail = depthFailOp;
      state->dynamic.vk.ds.stencil.back.op.compare = compareOp;
   }

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_STENCIL_OP;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D *pFragmentSize,
                                  const VkFragmentShadingRateCombinerOpKHR combinerOps[2])
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.fsr.fragment_size = *pFragmentSize;
   for (unsigned i = 0; i < 2; i++)
      state->dynamic.vk.fsr.combiner_ops[i] = combinerOps[i];

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_FRAGMENT_SHADING_RATE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.depth_bias.enable = depthBiasEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_BIAS_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ia.primitive_restart_enable = primitiveRestartEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_RESTART_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.rasterizer_discard_enable = rasterizerDiscardEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_RASTERIZER_DISCARD_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ts.patch_control_points = patchControlPoints;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_PATCH_CONTROL_POINTS;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;
   unsigned logic_op = radv_translate_blend_logic_op(logicOp);

   state->dynamic.vk.cb.logic_op = logic_op;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_LOGIC_OP;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                               const VkBool32 *pColorWriteEnables)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;
   uint8_t color_write_enable = 0;

   assert(attachmentCount <= MAX_RTS);

   for (uint32_t i = 0; i < attachmentCount; i++) {
      if (pColorWriteEnables[i]) {
         color_write_enable |= BITFIELD_BIT(i);
      }
   }

   state->dynamic.vk.cb.color_write_enables = color_write_enable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                          const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
                          uint32_t vertexAttributeDescriptionCount,
                          const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;
   struct radv_vs_input_state *vs_state = &state->dynamic_vs_input;

   const VkVertexInputBindingDescription2EXT *bindings[MAX_VBS];
   for (unsigned i = 0; i < vertexBindingDescriptionCount; i++)
      bindings[pVertexBindingDescriptions[i].binding] = &pVertexBindingDescriptions[i];

   state->vbo_misaligned_mask = 0;
   state->vbo_misaligned_mask_invalid = 0;

   vs_state->attribute_mask = 0;
   vs_state->instance_rate_inputs = 0;
   vs_state->nontrivial_divisors = 0;
   vs_state->zero_divisors = 0;
   vs_state->post_shuffle = 0;
   vs_state->alpha_adjust_lo = 0;
   vs_state->alpha_adjust_hi = 0;
   vs_state->nontrivial_formats = 0;
   vs_state->bindings_match_attrib = true;

   enum amd_gfx_level chip = cmd_buffer->device->physical_device->rad_info.gfx_level;
   enum radeon_family family = cmd_buffer->device->physical_device->rad_info.family;
   const struct ac_vtx_format_info *vtx_info_table = ac_get_vtx_format_info_table(chip, family);

   for (unsigned i = 0; i < vertexAttributeDescriptionCount; i++) {
      const VkVertexInputAttributeDescription2EXT *attrib = &pVertexAttributeDescriptions[i];
      const VkVertexInputBindingDescription2EXT *binding = bindings[attrib->binding];
      unsigned loc = attrib->location;

      vs_state->attribute_mask |= 1u << loc;
      vs_state->bindings[loc] = attrib->binding;
      if (attrib->binding != loc)
         vs_state->bindings_match_attrib = false;
      if (binding->inputRate == VK_VERTEX_INPUT_RATE_INSTANCE) {
         vs_state->instance_rate_inputs |= 1u << loc;
         vs_state->divisors[loc] = binding->divisor;
         if (binding->divisor == 0) {
            vs_state->zero_divisors |= 1u << loc;
         } else if (binding->divisor > 1) {
            vs_state->nontrivial_divisors |= 1u << loc;
         }
      }
      cmd_buffer->vertex_bindings[attrib->binding].stride = binding->stride;
      vs_state->offsets[loc] = attrib->offset;

      enum pipe_format format = vk_format_map[attrib->format];
      const struct ac_vtx_format_info *vtx_info = &vtx_info_table[format];

      vs_state->formats[loc] = format;
      uint8_t align_req_minus_1 = vtx_info->chan_byte_size >= 4 ? 3 : (vtx_info->element_size - 1);
      vs_state->format_align_req_minus_1[loc] = align_req_minus_1;
      vs_state->format_sizes[loc] = vtx_info->element_size;
      vs_state->alpha_adjust_lo |= (vtx_info->alpha_adjust & 0x1) << loc;
      vs_state->alpha_adjust_hi |= (vtx_info->alpha_adjust >> 1) << loc;
      if (G_008F0C_DST_SEL_X(vtx_info->dst_sel) == V_008F0C_SQ_SEL_Z)
         vs_state->post_shuffle |= BITFIELD_BIT(loc);

      if (!(vtx_info->has_hw_format & BITFIELD_BIT(vtx_info->num_channels - 1)))
         vs_state->nontrivial_formats |= BITFIELD_BIT(loc);

      if ((chip == GFX6 || chip >= GFX10) && state->vbo_bound_mask & BITFIELD_BIT(attrib->binding)) {
         if (binding->stride & align_req_minus_1) {
            state->vbo_misaligned_mask |= BITFIELD_BIT(loc);
         } else if ((cmd_buffer->vertex_bindings[attrib->binding].offset + vs_state->offsets[loc]) &
                    align_req_minus_1) {
            state->vbo_misaligned_mask |= BITFIELD_BIT(loc);
         }
      }
   }

   state->dirty |= RADV_CMD_DIRTY_VERTEX_BUFFER | RADV_CMD_DIRTY_DYNAMIC_VERTEX_INPUT;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;
   unsigned polygon_mode = radv_translate_fill(polygonMode);

   if (radv_polygon_mode_is_points_or_lines(state->dynamic.vk.rs.polygon_mode) !=
       radv_polygon_mode_is_points_or_lines(polygon_mode))
      state->dirty |= RADV_CMD_DIRTY_GUARDBAND;

   state->dynamic.vk.rs.polygon_mode = polygon_mode;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_POLYGON_MODE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ts.domain_origin = domainOrigin;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_TESS_DOMAIN_ORIGIN;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.cb.logic_op_enable = logicOpEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_LOGIC_OP_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.line.stipple.enable = stippledLineEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_LINE_STIPPLE_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ms.alpha_to_coverage_enable = alphaToCoverageEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples, const VkSampleMask *pSampleMask)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ms.sample_mask = pSampleMask[0] & 0xffff;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_SAMPLE_MASK;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.depth_clip_enable = depthClipEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                            VkConservativeRasterizationModeEXT conservativeRasterizationMode)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.conservative_mode = conservativeRasterizationMode;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_CONSERVATIVE_RAST_MODE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.vp.depth_clip_negative_one_to_one = negativeOneToOne;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.provoking_vertex = provokingVertexMode;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_PROVOKING_VERTEX_MODE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.depth_clamp_enable = depthClampEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_CLAMP_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                             const VkColorComponentFlags *pColorWriteMasks)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   assert(firstAttachment + attachmentCount <= MAX_RTS);

   for (uint32_t i = 0; i < attachmentCount; i++) {
      uint32_t idx = firstAttachment + i;

      state->dynamic.vk.cb.attachments[idx].write_mask = pColorWriteMasks[i];
   }

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_MASK;

   if (cmd_buffer->device->physical_device->rad_info.rbplus_allowed)
      state->dirty |= RADV_CMD_DIRTY_RBPLUS;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                               const VkBool32 *pColorBlendEnables)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   assert(firstAttachment + attachmentCount <= MAX_RTS);

   for (uint32_t i = 0; i < attachmentCount; i++) {
      uint32_t idx = firstAttachment + i;

      state->dynamic.vk.cb.attachments[idx].blend_enable = pColorBlendEnables[i];
   }

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ms.rasterization_samples = rasterizationSamples;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.rs.line.mode = lineRasterizationMode;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                 const VkColorBlendEquationEXT *pColorBlendEquations)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   assert(firstAttachment + attachmentCount <= MAX_RTS);
   for (uint32_t i = 0; i < attachmentCount; i++) {
      unsigned idx = firstAttachment + i;

      state->dynamic.vk.cb.attachments[idx].src_color_blend_factor = pColorBlendEquations[i].srcColorBlendFactor;
      state->dynamic.vk.cb.attachments[idx].dst_color_blend_factor = pColorBlendEquations[i].dstColorBlendFactor;
      state->dynamic.vk.cb.attachments[idx].color_blend_op = pColorBlendEquations[i].colorBlendOp;
      state->dynamic.vk.cb.attachments[idx].src_alpha_blend_factor = pColorBlendEquations[i].srcAlphaBlendFactor;
      state->dynamic.vk.cb.attachments[idx].dst_alpha_blend_factor = pColorBlendEquations[i].dstAlphaBlendFactor;
      state->dynamic.vk.cb.attachments[idx].alpha_blend_op = pColorBlendEquations[i].alphaBlendOp;
   }

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_EQUATION;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.ms.sample_locations_enable = sampleLocationsEnable;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_SAMPLE_LOCATIONS_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.dr.enable = discardRectangleEnable;
   state->dynamic.vk.dr.rectangle_count = discardRectangleEnable ? MAX_DISCARD_RECTANGLES : 0;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.vk.dr.mode = discardRectangleMode;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DISCARD_RECTANGLE_MODE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   state->dynamic.feedback_loop_aspects = aspectMask;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT *pDepthBiasInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_cmd_state *state = &cmd_buffer->state;

   const VkDepthBiasRepresentationInfoEXT *dbr_info =
      vk_find_struct_const(pDepthBiasInfo->pNext, DEPTH_BIAS_REPRESENTATION_INFO_EXT);

   state->dynamic.vk.rs.depth_bias.constant = pDepthBiasInfo->depthBiasConstantFactor;
   state->dynamic.vk.rs.depth_bias.clamp = pDepthBiasInfo->depthBiasClamp;
   state->dynamic.vk.rs.depth_bias.slope = pDepthBiasInfo->depthBiasSlopeFactor;
   state->dynamic.vk.rs.depth_bias.representation =
      dbr_info ? dbr_info->depthBiasRepresentation : VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORMAT_EXT;

   state->dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_BIAS;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer *pCmdBuffers)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, primary, commandBuffer);

   assert(commandBufferCount > 0);

   radv_emit_mip_change_flush_default(primary);

   /* Emit pending flushes on primary prior to executing secondary */
   radv_emit_cache_flush(primary);

   /* Make sure CP DMA is idle on primary prior to executing secondary. */
   radv_cp_dma_wait_for_idle(primary);

   for (uint32_t i = 0; i < commandBufferCount; i++) {
      RADV_FROM_HANDLE(radv_cmd_buffer, secondary, pCmdBuffers[i]);

      /* Do not launch an IB2 for secondary command buffers that contain
       * DRAW_{INDEX}_INDIRECT_{MULTI} on GFX6-7 because it's illegal and hangs the GPU.
       */
      const bool allow_ib2 =
         !secondary->state.uses_draw_indirect || secondary->device->physical_device->rad_info.gfx_level >= GFX8;

      primary->scratch_size_per_wave_needed =
         MAX2(primary->scratch_size_per_wave_needed, secondary->scratch_size_per_wave_needed);
      primary->scratch_waves_wanted = MAX2(primary->scratch_waves_wanted, secondary->scratch_waves_wanted);
      primary->compute_scratch_size_per_wave_needed =
         MAX2(primary->compute_scratch_size_per_wave_needed, secondary->compute_scratch_size_per_wave_needed);
      primary->compute_scratch_waves_wanted =
         MAX2(primary->compute_scratch_waves_wanted, secondary->compute_scratch_waves_wanted);

      if (secondary->esgs_ring_size_needed > primary->esgs_ring_size_needed)
         primary->esgs_ring_size_needed = secondary->esgs_ring_size_needed;
      if (secondary->gsvs_ring_size_needed > primary->gsvs_ring_size_needed)
         primary->gsvs_ring_size_needed = secondary->gsvs_ring_size_needed;
      if (secondary->tess_rings_needed)
         primary->tess_rings_needed = true;
      if (secondary->task_rings_needed)
         primary->task_rings_needed = true;
      if (secondary->mesh_scratch_ring_needed)
         primary->mesh_scratch_ring_needed = true;
      if (secondary->sample_positions_needed)
         primary->sample_positions_needed = true;
      if (secondary->gds_needed)
         primary->gds_needed = true;
      if (secondary->gds_oa_needed)
         primary->gds_oa_needed = true;

      primary->shader_upload_seq = MAX2(primary->shader_upload_seq, secondary->shader_upload_seq);

      if (!secondary->state.render.has_image_views && primary->state.render.active &&
          (primary->state.dirty & RADV_CMD_DIRTY_FRAMEBUFFER)) {
         /* Emit the framebuffer state from primary if secondary
          * has been recorded without a framebuffer, otherwise
          * fast color/depth clears can't work.
          */
         radv_emit_framebuffer_state(primary);
      }

      if (secondary->gang.cs) {
         if (!radv_gang_init(primary))
            return;

         struct radeon_cmdbuf *ace_primary = primary->gang.cs;
         struct radeon_cmdbuf *ace_secondary = secondary->gang.cs;

         /* Emit pending flushes on primary prior to executing secondary. */
         radv_gang_cache_flush(primary);

         /* Wait for gang semaphores, if necessary. */
         if (radv_flush_gang_leader_semaphore(primary))
            radv_wait_gang_leader(primary);
         if (radv_flush_gang_follower_semaphore(primary))
            radv_wait_gang_follower(primary);

         /* Execute the secondary compute cmdbuf.
          * Don't use IB2 packets because they are not supported on compute queues.
          */
         primary->device->ws->cs_execute_secondary(ace_primary, ace_secondary, false);
      }

      /* Update pending ACE internal flush bits from the secondary cmdbuf */
      primary->gang.flush_bits |= secondary->gang.flush_bits;

      /* Increment gang semaphores if secondary was dirty.
       * This happens when the secondary cmdbuf has a barrier which
       * isn't consumed by a draw call.
       */
      if (radv_gang_leader_sem_dirty(secondary))
         primary->gang.sem.leader_value++;
      if (radv_gang_follower_sem_dirty(secondary))
         primary->gang.sem.follower_value++;

      primary->device->ws->cs_execute_secondary(primary->cs, secondary->cs, allow_ib2);

      /* When the secondary command buffer is compute only we don't
       * need to re-emit the current graphics pipeline.
       */
      if (secondary->state.emitted_graphics_pipeline) {
         primary->state.emitted_graphics_pipeline = secondary->state.emitted_graphics_pipeline;
      }

      /* When the secondary command buffer is graphics only we don't
       * need to re-emit the current compute pipeline.
       */
      if (secondary->state.emitted_compute_pipeline) {
         primary->state.emitted_compute_pipeline = secondary->state.emitted_compute_pipeline;
      }

      if (secondary->state.last_primitive_reset_index) {
         primary->state.last_primitive_reset_index = secondary->state.last_primitive_reset_index;
      }

      if (secondary->state.last_ia_multi_vgt_param) {
         primary->state.last_ia_multi_vgt_param = secondary->state.last_ia_multi_vgt_param;
      }

      if (secondary->state.last_ge_cntl) {
         primary->state.last_ge_cntl = secondary->state.last_ge_cntl;
      }

      primary->state.last_num_instances = secondary->state.last_num_instances;
      primary->state.last_subpass_color_count = secondary->state.last_subpass_color_count;
      primary->state.last_sx_ps_downconvert = secondary->state.last_sx_ps_downconvert;
      primary->state.last_sx_blend_opt_epsilon = secondary->state.last_sx_blend_opt_epsilon;
      primary->state.last_sx_blend_opt_control = secondary->state.last_sx_blend_opt_control;

      if (secondary->state.last_index_type != -1) {
         primary->state.last_index_type = secondary->state.last_index_type;
      }

      primary->state.last_vrs_rates = secondary->state.last_vrs_rates;
      primary->state.last_vrs_rates_sgpr_idx = secondary->state.last_vrs_rates_sgpr_idx;

      primary->state.last_pa_sc_binner_cntl_0 = secondary->state.last_pa_sc_binner_cntl_0;

      primary->state.last_db_shader_control = secondary->state.last_db_shader_control;

      primary->state.rb_noncoherent_dirty |= secondary->state.rb_noncoherent_dirty;
   }

   /* After executing commands from secondary buffers we have to dirty
    * some states.
    */
   primary->state.dirty |= RADV_CMD_DIRTY_PIPELINE | RADV_CMD_DIRTY_INDEX_BUFFER | RADV_CMD_DIRTY_GUARDBAND |
                           RADV_CMD_DIRTY_DYNAMIC_ALL | RADV_CMD_DIRTY_SHADER_QUERY | RADV_CMD_DIRTY_OCCLUSION_QUERY |
                           RADV_CMD_DIRTY_DB_SHADER_CONTROL;
   radv_mark_descriptor_sets_dirty(primary, VK_PIPELINE_BIND_POINT_GRAPHICS);
   radv_mark_descriptor_sets_dirty(primary, VK_PIPELINE_BIND_POINT_COMPUTE);

   primary->state.last_first_instance = -1;
   primary->state.last_drawid = -1;
   primary->state.last_vertex_offset_valid = false;
   primary->state.last_db_count_control = -1;
}

static void
radv_mark_noncoherent_rb(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_rendering_state *render = &cmd_buffer->state.render;

   /* Have to be conservative in cmdbuffers with inherited attachments. */
   if (!render->has_image_views) {
      cmd_buffer->state.rb_noncoherent_dirty = true;
      return;
   }

   for (uint32_t i = 0; i < render->color_att_count; i++) {
      if (render->color_att[i].iview && !render->color_att[i].iview->image->l2_coherent) {
         cmd_buffer->state.rb_noncoherent_dirty = true;
         return;
      }
   }
   if (render->ds_att.iview && !render->ds_att.iview->image->l2_coherent)
      cmd_buffer->state.rb_noncoherent_dirty = true;
}

static VkImageLayout
attachment_initial_layout(const VkRenderingAttachmentInfo *att)
{
   const VkRenderingAttachmentInitialLayoutInfoMESA *layout_info =
      vk_find_struct_const(att->pNext, RENDERING_ATTACHMENT_INITIAL_LAYOUT_INFO_MESA);
   if (layout_info != NULL)
      return layout_info->initialLayout;

   return att->imageLayout;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   const struct VkSampleLocationsInfoEXT *sample_locs_info =
      vk_find_struct_const(pRenderingInfo->pNext, SAMPLE_LOCATIONS_INFO_EXT);

   struct radv_sample_locations_state sample_locations = {
      .count = 0,
   };
   if (sample_locs_info) {
      sample_locations = (struct radv_sample_locations_state){
         .per_pixel = sample_locs_info->sampleLocationsPerPixel,
         .grid_size = sample_locs_info->sampleLocationGridSize,
         .count = sample_locs_info->sampleLocationsCount,
      };
      typed_memcpy(sample_locations.locations, sample_locs_info->pSampleLocations,
                   sample_locs_info->sampleLocationsCount);
   }

   /* Dynamic rendering does not have implicit transitions, so limit the marker to
    * when a render pass is used.
    * Additionally, some internal meta operations called inside a barrier may issue
    * render calls (with dynamic rendering), so this makes sure those case don't
    * create a nested barrier scope.
    */
   if (cmd_buffer->vk.render_pass)
      radv_describe_barrier_start(cmd_buffer, RGP_BARRIER_EXTERNAL_RENDER_PASS_SYNC);
   uint32_t color_samples = 0, ds_samples = 0;
   struct radv_attachment color_att[MAX_RTS];
   for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; i++) {
      const VkRenderingAttachmentInfo *att_info = &pRenderingInfo->pColorAttachments[i];

      color_att[i] = (struct radv_attachment){.iview = NULL};
      if (att_info->imageView == VK_NULL_HANDLE)
         continue;

      VK_FROM_HANDLE(radv_image_view, iview, att_info->imageView);
      color_att[i].format = iview->vk.format;
      color_att[i].iview = iview;
      color_att[i].layout = att_info->imageLayout;
      radv_initialise_color_surface(cmd_buffer->device, &color_att[i].cb, iview);

      if (att_info->resolveMode != VK_RESOLVE_MODE_NONE && att_info->resolveImageView != VK_NULL_HANDLE) {
         color_att[i].resolve_mode = att_info->resolveMode;
         color_att[i].resolve_iview = radv_image_view_from_handle(att_info->resolveImageView);
         color_att[i].resolve_layout = att_info->resolveImageLayout;
      }

      color_samples = MAX2(color_samples, color_att[i].iview->vk.image->samples);

      VkImageLayout initial_layout = attachment_initial_layout(att_info);
      if (initial_layout != color_att[i].layout) {
         assert(!(pRenderingInfo->flags & VK_RENDERING_RESUMING_BIT));
         radv_handle_rendering_image_transition(cmd_buffer, color_att[i].iview, pRenderingInfo->layerCount,
                                                pRenderingInfo->viewMask, initial_layout, VK_IMAGE_LAYOUT_UNDEFINED,
                                                color_att[i].layout, VK_IMAGE_LAYOUT_UNDEFINED, &sample_locations);
      }
   }

   struct radv_attachment ds_att = {.iview = NULL};
   VkImageAspectFlags ds_att_aspects = 0;
   const VkRenderingAttachmentInfo *d_att_info = pRenderingInfo->pDepthAttachment;
   const VkRenderingAttachmentInfo *s_att_info = pRenderingInfo->pStencilAttachment;
   if ((d_att_info != NULL && d_att_info->imageView != VK_NULL_HANDLE) ||
       (s_att_info != NULL && s_att_info->imageView != VK_NULL_HANDLE)) {
      struct radv_image_view *d_iview = NULL, *s_iview = NULL;
      struct radv_image_view *d_res_iview = NULL, *s_res_iview = NULL;
      VkImageLayout initial_depth_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkImageLayout initial_stencil_layout = VK_IMAGE_LAYOUT_UNDEFINED;

      if (d_att_info != NULL && d_att_info->imageView != VK_NULL_HANDLE) {
         d_iview = radv_image_view_from_handle(d_att_info->imageView);
         initial_depth_layout = attachment_initial_layout(d_att_info);
         ds_att.layout = d_att_info->imageLayout;

         if (d_att_info->resolveMode != VK_RESOLVE_MODE_NONE && d_att_info->resolveImageView != VK_NULL_HANDLE) {
            d_res_iview = radv_image_view_from_handle(d_att_info->resolveImageView);
            ds_att.resolve_mode = d_att_info->resolveMode;
            ds_att.resolve_layout = d_att_info->resolveImageLayout;
         }
      }

      if (s_att_info != NULL && s_att_info->imageView != VK_NULL_HANDLE) {
         s_iview = radv_image_view_from_handle(s_att_info->imageView);
         initial_stencil_layout = attachment_initial_layout(s_att_info);
         ds_att.stencil_layout = s_att_info->imageLayout;

         if (s_att_info->resolveMode != VK_RESOLVE_MODE_NONE && s_att_info->resolveImageView != VK_NULL_HANDLE) {
            s_res_iview = radv_image_view_from_handle(s_att_info->resolveImageView);
            ds_att.stencil_resolve_mode = s_att_info->resolveMode;
            ds_att.stencil_resolve_layout = s_att_info->resolveImageLayout;
         }
      }

      assert(d_iview == NULL || s_iview == NULL || d_iview == s_iview);
      ds_att.iview = d_iview ? d_iview : s_iview, ds_att.format = ds_att.iview->vk.format;

      if (d_iview && s_iview) {
         ds_att_aspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      } else if (d_iview) {
         ds_att_aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
      } else {
         ds_att_aspects = VK_IMAGE_ASPECT_STENCIL_BIT;
      }

      radv_initialise_ds_surface(cmd_buffer->device, &ds_att.ds, ds_att.iview, ds_att_aspects);

      assert(d_res_iview == NULL || s_res_iview == NULL || d_res_iview == s_res_iview);
      ds_att.resolve_iview = d_res_iview ? d_res_iview : s_res_iview;

      ds_samples = ds_att.iview->vk.image->samples;

      if (initial_depth_layout != ds_att.layout || initial_stencil_layout != ds_att.stencil_layout) {
         assert(!(pRenderingInfo->flags & VK_RENDERING_RESUMING_BIT));
         radv_handle_rendering_image_transition(cmd_buffer, ds_att.iview, pRenderingInfo->layerCount,
                                                pRenderingInfo->viewMask, initial_depth_layout, initial_stencil_layout,
                                                ds_att.layout, ds_att.stencil_layout, &sample_locations);
      }
   }
   if (cmd_buffer->vk.render_pass)
      radv_describe_barrier_end(cmd_buffer);

   const VkRenderingFragmentShadingRateAttachmentInfoKHR *fsr_info =
      vk_find_struct_const(pRenderingInfo->pNext, RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR);
   struct radv_attachment vrs_att = {.iview = NULL};
   VkExtent2D vrs_texel_size = {.width = 0};
   if (fsr_info && fsr_info->imageView) {
      VK_FROM_HANDLE(radv_image_view, iview, fsr_info->imageView);
      vrs_att = (struct radv_attachment){
         .format = iview->vk.format,
         .iview = iview,
         .layout = fsr_info->imageLayout,
      };
      vrs_texel_size = fsr_info->shadingRateAttachmentTexelSize;
   }

   /* Now that we've done any layout transitions which may invoke meta, we can
    * fill out the actual rendering info and set up for the client's render pass.
    */
   radv_cmd_buffer_reset_rendering(cmd_buffer);

   struct radv_rendering_state *render = &cmd_buffer->state.render;
   render->active = true;
   render->has_image_views = true;
   render->area = pRenderingInfo->renderArea;
   render->view_mask = pRenderingInfo->viewMask;
   render->layer_count = pRenderingInfo->layerCount;
   render->color_samples = color_samples;
   render->ds_samples = ds_samples;
   render->max_samples = MAX2(color_samples, ds_samples);
   render->sample_locations = sample_locations;
   render->color_att_count = pRenderingInfo->colorAttachmentCount;
   typed_memcpy(render->color_att, color_att, render->color_att_count);
   render->ds_att = ds_att;
   render->ds_att_aspects = ds_att_aspects;
   render->vrs_att = vrs_att;
   render->vrs_texel_size = vrs_texel_size;
   cmd_buffer->state.dirty |= RADV_CMD_DIRTY_FRAMEBUFFER;

   if (cmd_buffer->device->physical_device->rad_info.rbplus_allowed)
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_RBPLUS;

   cmd_buffer->state.dirty |= RADV_CMD_DIRTY_DYNAMIC_DEPTH_BIAS | RADV_CMD_DIRTY_DYNAMIC_STENCIL_TEST_ENABLE;

   if (render->vrs_att.iview && cmd_buffer->device->physical_device->rad_info.gfx_level == GFX10_3) {
      if (render->ds_att.iview &&
          radv_htile_enabled(render->ds_att.iview->image, render->ds_att.iview->vk.base_mip_level)) {
         /* When we have a VRS attachment and a depth/stencil attachment, we just need to copy the
          * VRS rates to the HTILE buffer of the attachment.
          */
         struct radv_image_view *ds_iview = render->ds_att.iview;
         struct radv_image *ds_image = ds_iview->image;
         uint32_t level = ds_iview->vk.base_mip_level;

         /* HTILE buffer */
         uint64_t htile_offset = ds_image->bindings[0].offset + ds_image->planes[0].surface.meta_offset +
                                 ds_image->planes[0].surface.u.gfx9.meta_levels[level].offset;
         uint64_t htile_size = ds_image->planes[0].surface.u.gfx9.meta_levels[level].size;
         struct radv_buffer htile_buffer;

         radv_buffer_init(&htile_buffer, cmd_buffer->device, ds_image->bindings[0].bo, htile_size, htile_offset);

         assert(render->area.offset.x + render->area.extent.width <= ds_image->vk.extent.width &&
                render->area.offset.x + render->area.extent.height <= ds_image->vk.extent.height);

         /* Copy the VRS rates to the HTILE buffer. */
         radv_copy_vrs_htile(cmd_buffer, render->vrs_att.iview->image, &render->area, ds_image, &htile_buffer, true);

         radv_buffer_finish(&htile_buffer);
      } else {
         /* When a subpass uses a VRS attachment without binding a depth/stencil attachment, or when
          * HTILE isn't enabled, we use a fallback that copies the VRS rates to our internal HTILE buffer.
          */
         struct radv_image *ds_image = radv_cmd_buffer_get_vrs_image(cmd_buffer);

         if (ds_image && render->area.offset.x < ds_image->vk.extent.width &&
             render->area.offset.y < ds_image->vk.extent.height) {
            /* HTILE buffer */
            struct radv_buffer *htile_buffer = cmd_buffer->device->vrs.buffer;

            VkRect2D area = render->area;
            area.extent.width = MIN2(area.extent.width, ds_image->vk.extent.width - area.offset.x);
            area.extent.height = MIN2(area.extent.height, ds_image->vk.extent.height - area.offset.y);

            /* Copy the VRS rates to the HTILE buffer. */
            radv_copy_vrs_htile(cmd_buffer, render->vrs_att.iview->image, &area, ds_image, htile_buffer, false);
         }
      }
   }

   radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 6);
   radeon_set_context_reg(cmd_buffer->cs, R_028204_PA_SC_WINDOW_SCISSOR_TL,
                          S_028204_TL_X(render->area.offset.x) | S_028204_TL_Y(render->area.offset.y));
   radeon_set_context_reg(cmd_buffer->cs, R_028208_PA_SC_WINDOW_SCISSOR_BR,
                          S_028208_BR_X(render->area.offset.x + render->area.extent.width) |
                             S_028208_BR_Y(render->area.offset.y + render->area.extent.height));

   radv_emit_fb_mip_change_flush(cmd_buffer);

   if (!(pRenderingInfo->flags & VK_RENDERING_RESUMING_BIT))
      radv_cmd_buffer_clear_rendering(cmd_buffer, pRenderingInfo);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdEndRendering(VkCommandBuffer commandBuffer)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   radv_mark_noncoherent_rb(cmd_buffer);
   radv_cmd_buffer_resolve_rendering(cmd_buffer);
   radv_cmd_buffer_reset_rendering(cmd_buffer);
}

static void
radv_emit_view_index_per_stage(struct radeon_cmdbuf *cs, const struct radv_shader *shader, uint32_t base_reg,
                               unsigned index)
{
   const struct radv_userdata_info *loc = radv_get_user_sgpr(shader, AC_UD_VIEW_INDEX);

   if (loc->sgpr_idx == -1)
      return;

   radeon_set_sh_reg(cs, base_reg + loc->sgpr_idx * 4, index);
}

static void
radv_emit_view_index(struct radv_cmd_buffer *cmd_buffer, unsigned index)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   radv_foreach_stage(stage, cmd_buffer->state.active_stages & ~VK_SHADER_STAGE_TASK_BIT_EXT)
   {
      const struct radv_shader *shader = radv_get_shader(cmd_buffer->state.shaders, stage);

      radv_emit_view_index_per_stage(cs, shader, shader->info.user_data_0, index);
   }

   if (cmd_buffer->state.gs_copy_shader) {
      radv_emit_view_index_per_stage(cs, cmd_buffer->state.gs_copy_shader, R_00B130_SPI_SHADER_USER_DATA_VS_0, index);
   }

   if (cmd_buffer->state.active_stages & VK_SHADER_STAGE_TASK_BIT_EXT) {
      radv_emit_view_index_per_stage(cmd_buffer->gang.cs, cmd_buffer->state.shaders[MESA_SHADER_TASK],
                                     cmd_buffer->state.shaders[MESA_SHADER_TASK]->info.user_data_0, index);
   }
}

/**
 * Emulates predication for MEC using COND_EXEC.
 * When the current command buffer is predicating, emit a COND_EXEC packet
 * so that the MEC skips the next few dwords worth of packets.
 *
 * To make it work with inverted conditional rendering, we allocate
 * space in the upload BO and emit some packets to invert the condition.
 */
static void
radv_cs_emit_compute_predication(const struct radv_device *device, struct radv_cmd_state *state,
                                 struct radeon_cmdbuf *cs, uint64_t inv_va, bool *inv_emitted, unsigned dwords)
{
   if (!state->predicating)
      return;

   uint64_t va = state->predication_va;

   if (!state->predication_type) {
      /* Invert the condition the first time it is needed. */
      if (!*inv_emitted) {
         const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;

         *inv_emitted = true;

         /* Write 1 to the inverted predication VA. */
         radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
         radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_IMM) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) |
                            COPY_DATA_WR_CONFIRM | (gfx_level == GFX6 ? COPY_DATA_ENGINE_PFP : 0));
         radeon_emit(cs, 1);
         radeon_emit(cs, 0);
         radeon_emit(cs, inv_va);
         radeon_emit(cs, inv_va >> 32);

         /* If the API predication VA == 0, skip next command. */
         if (device->physical_device->rad_info.gfx_level >= GFX7) {
            radeon_emit(cs, PKT3(PKT3_COND_EXEC, 3, 0));
            radeon_emit(cs, va);
            radeon_emit(cs, va >> 32);
            radeon_emit(cs, 0);
            radeon_emit(cs, 6); /* 1x COPY_DATA size */
         } else {
            radeon_emit(cs, PKT3(PKT3_COND_EXEC, 2, 0));
            radeon_emit(cs, va);
            radeon_emit(cs, va >> 32);
            radeon_emit(cs, 6); /* 1x COPY_DATA size */
         }

         /* Write 0 to the new predication VA (when the API condition != 0) */
         radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
         radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_IMM) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) |
                            COPY_DATA_WR_CONFIRM | (gfx_level == GFX6 ? COPY_DATA_ENGINE_PFP : 0));
         radeon_emit(cs, 0);
         radeon_emit(cs, 0);
         radeon_emit(cs, inv_va);
         radeon_emit(cs, inv_va >> 32);
      }

      va = inv_va;
   }

   if (device->physical_device->rad_info.gfx_level >= GFX7) {
      radeon_emit(cs, PKT3(PKT3_COND_EXEC, 3, 0));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, 0);      /* Cache policy */
      radeon_emit(cs, dwords); /* Size of the predicated packet(s) in DWORDs. */
   } else {
      radeon_emit(cs, PKT3(PKT3_COND_EXEC, 2, 0));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, dwords); /* Size of the predicated packet(s) in DWORDs. */
   }
}

static void
radv_cs_emit_draw_packet(struct radv_cmd_buffer *cmd_buffer, uint32_t vertex_count, uint32_t use_opaque)
{
   radeon_emit(cmd_buffer->cs, PKT3(PKT3_DRAW_INDEX_AUTO, 1, cmd_buffer->state.predicating));
   radeon_emit(cmd_buffer->cs, vertex_count);
   radeon_emit(cmd_buffer->cs, V_0287F0_DI_SRC_SEL_AUTO_INDEX | use_opaque);
}

/**
 * Emit a PKT3_DRAW_INDEX_2 packet to render "index_count` vertices.
 *
 * The starting address "index_va" may point anywhere within the index buffer. The number of
 * indexes allocated in the index buffer *past that point* is specified by "max_index_count".
 * Hardware uses this information to return 0 for out-of-bounds reads.
 */
static void
radv_cs_emit_draw_indexed_packet(struct radv_cmd_buffer *cmd_buffer, uint64_t index_va, uint32_t max_index_count,
                                 uint32_t index_count, bool not_eop)
{
   radeon_emit(cmd_buffer->cs, PKT3(PKT3_DRAW_INDEX_2, 4, cmd_buffer->state.predicating));
   radeon_emit(cmd_buffer->cs, max_index_count);
   radeon_emit(cmd_buffer->cs, index_va);
   radeon_emit(cmd_buffer->cs, index_va >> 32);
   radeon_emit(cmd_buffer->cs, index_count);
   /* NOT_EOP allows merging multiple draws into 1 wave, but only user VGPRs
    * can be changed between draws and GS fast launch must be disabled.
    * NOT_EOP doesn't work on gfx9 and older.
    */
   radeon_emit(cmd_buffer->cs, V_0287F0_DI_SRC_SEL_DMA | S_0287F0_NOT_EOP(not_eop));
}

/* MUST inline this function to avoid massive perf loss in drawoverhead */
ALWAYS_INLINE static void
radv_cs_emit_indirect_draw_packet(struct radv_cmd_buffer *cmd_buffer, bool indexed, uint32_t draw_count,
                                  uint64_t count_va, uint32_t stride)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const unsigned di_src_sel = indexed ? V_0287F0_DI_SRC_SEL_DMA : V_0287F0_DI_SRC_SEL_AUTO_INDEX;
   bool draw_id_enable = cmd_buffer->state.uses_drawid;
   uint32_t base_reg = cmd_buffer->state.vtx_base_sgpr;
   uint32_t vertex_offset_reg, start_instance_reg = 0, draw_id_reg = 0;
   bool predicating = cmd_buffer->state.predicating;
   bool mesh = cmd_buffer->state.mesh_shading;
   assert(base_reg);

   /* just reset draw state for vertex data */
   cmd_buffer->state.last_first_instance = -1;
   cmd_buffer->state.last_num_instances = -1;
   cmd_buffer->state.last_drawid = -1;
   cmd_buffer->state.last_vertex_offset_valid = false;

   vertex_offset_reg = (base_reg - SI_SH_REG_OFFSET) >> 2;
   if (cmd_buffer->state.uses_baseinstance)
      start_instance_reg = ((base_reg + (draw_id_enable ? 8 : 4)) - SI_SH_REG_OFFSET) >> 2;
   if (draw_id_enable)
      draw_id_reg = ((base_reg + mesh * 12 + 4) - SI_SH_REG_OFFSET) >> 2;

   if (draw_count == 1 && !count_va && !draw_id_enable) {
      radeon_emit(cs, PKT3(indexed ? PKT3_DRAW_INDEX_INDIRECT : PKT3_DRAW_INDIRECT, 3, predicating));
      radeon_emit(cs, 0);
      radeon_emit(cs, vertex_offset_reg);
      radeon_emit(cs, start_instance_reg);
      radeon_emit(cs, di_src_sel);
   } else {
      radeon_emit(cs, PKT3(indexed ? PKT3_DRAW_INDEX_INDIRECT_MULTI : PKT3_DRAW_INDIRECT_MULTI, 8, predicating));
      radeon_emit(cs, 0);
      radeon_emit(cs, vertex_offset_reg);
      radeon_emit(cs, start_instance_reg);
      radeon_emit(cs, draw_id_reg | S_2C3_DRAW_INDEX_ENABLE(draw_id_enable) | S_2C3_COUNT_INDIRECT_ENABLE(!!count_va));
      radeon_emit(cs, draw_count); /* count */
      radeon_emit(cs, count_va);   /* count_addr */
      radeon_emit(cs, count_va >> 32);
      radeon_emit(cs, stride); /* stride */
      radeon_emit(cs, di_src_sel);
   }

   cmd_buffer->state.uses_draw_indirect = true;
}

ALWAYS_INLINE static void
radv_cs_emit_indirect_mesh_draw_packet(struct radv_cmd_buffer *cmd_buffer, uint32_t draw_count, uint64_t count_va,
                                       uint32_t stride)
{
   const struct radv_shader *mesh_shader = cmd_buffer->state.shaders[MESA_SHADER_MESH];
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint32_t base_reg = cmd_buffer->state.vtx_base_sgpr;
   bool predicating = cmd_buffer->state.predicating;
   assert(base_reg || (!cmd_buffer->state.uses_drawid && !mesh_shader->info.cs.uses_grid_size));

   /* Reset draw state. */
   cmd_buffer->state.last_first_instance = -1;
   cmd_buffer->state.last_num_instances = -1;
   cmd_buffer->state.last_drawid = -1;
   cmd_buffer->state.last_vertex_offset_valid = false;

   uint32_t xyz_dim_enable = mesh_shader->info.cs.uses_grid_size;
   uint32_t xyz_dim_reg = (base_reg - SI_SH_REG_OFFSET) >> 2;
   uint32_t draw_id_reg = xyz_dim_reg + (xyz_dim_enable ? 3 : 0);

   uint32_t draw_id_enable = !!cmd_buffer->state.uses_drawid;
   uint32_t mode1_enable = !cmd_buffer->device->mesh_fast_launch_2;

   radeon_emit(cs, PKT3(PKT3_DISPATCH_MESH_INDIRECT_MULTI, 7, predicating) | PKT3_RESET_FILTER_CAM_S(1));
   radeon_emit(cs, 0); /* data_offset */
   radeon_emit(cs, S_4C1_XYZ_DIM_REG(xyz_dim_reg) | S_4C1_DRAW_INDEX_REG(draw_id_reg));
   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11)
      radeon_emit(cs, S_4C2_DRAW_INDEX_ENABLE(draw_id_enable) | S_4C2_COUNT_INDIRECT_ENABLE(!!count_va) |
                         S_4C2_XYZ_DIM_ENABLE(xyz_dim_enable) | S_4C2_MODE1_ENABLE(mode1_enable));
   else
      radeon_emit(cs, S_4C2_DRAW_INDEX_ENABLE(draw_id_enable) | S_4C2_COUNT_INDIRECT_ENABLE(!!count_va));
   radeon_emit(cs, draw_count);
   radeon_emit(cs, count_va & 0xFFFFFFFF);
   radeon_emit(cs, count_va >> 32);
   radeon_emit(cs, stride);
   radeon_emit(cs, V_0287F0_DI_SRC_SEL_AUTO_INDEX);
}

ALWAYS_INLINE static void
radv_cs_emit_dispatch_taskmesh_direct_ace_packet(struct radv_cmd_buffer *cmd_buffer, const uint32_t x, const uint32_t y,
                                                 const uint32_t z)
{
   struct radv_shader *task_shader = cmd_buffer->state.shaders[MESA_SHADER_TASK];
   struct radeon_cmdbuf *cs = cmd_buffer->gang.cs;
   const bool predicating = cmd_buffer->state.predicating;
   const uint32_t dispatch_initiator =
      cmd_buffer->device->dispatch_initiator_task | S_00B800_CS_W32_EN(task_shader->info.wave_size == 32);

   const struct radv_userdata_info *ring_entry_loc = radv_get_user_sgpr(task_shader, AC_UD_TASK_RING_ENTRY);
   assert(ring_entry_loc && ring_entry_loc->sgpr_idx != -1 && ring_entry_loc->num_sgprs == 1);

   uint32_t ring_entry_reg = (R_00B900_COMPUTE_USER_DATA_0 + ring_entry_loc->sgpr_idx * 4 - SI_SH_REG_OFFSET) >> 2;

   radeon_emit(cs, PKT3(PKT3_DISPATCH_TASKMESH_DIRECT_ACE, 4, predicating) | PKT3_SHADER_TYPE_S(1));
   radeon_emit(cs, x);
   radeon_emit(cs, y);
   radeon_emit(cs, z);
   radeon_emit(cs, dispatch_initiator);
   radeon_emit(cs, ring_entry_reg & 0xFFFF);
}

ALWAYS_INLINE static void
radv_cs_emit_dispatch_taskmesh_indirect_multi_ace_packet(struct radv_cmd_buffer *cmd_buffer, uint64_t data_va,
                                                         uint32_t draw_count, uint64_t count_va, uint32_t stride)
{
   assert((data_va & 0x03) == 0);
   assert((count_va & 0x03) == 0);

   struct radv_shader *task_shader = cmd_buffer->state.shaders[MESA_SHADER_TASK];
   struct radeon_cmdbuf *cs = cmd_buffer->gang.cs;

   const uint32_t xyz_dim_enable = task_shader->info.cs.uses_grid_size;
   const uint32_t draw_id_enable = task_shader->info.vs.needs_draw_id;
   const uint32_t dispatch_initiator =
      cmd_buffer->device->dispatch_initiator_task | S_00B800_CS_W32_EN(task_shader->info.wave_size == 32);

   const struct radv_userdata_info *ring_entry_loc = radv_get_user_sgpr(task_shader, AC_UD_TASK_RING_ENTRY);
   const struct radv_userdata_info *xyz_dim_loc = radv_get_user_sgpr(task_shader, AC_UD_CS_GRID_SIZE);
   const struct radv_userdata_info *draw_id_loc = radv_get_user_sgpr(task_shader, AC_UD_CS_TASK_DRAW_ID);

   assert(ring_entry_loc->sgpr_idx != -1 && ring_entry_loc->num_sgprs == 1);
   assert(!xyz_dim_enable || (xyz_dim_loc->sgpr_idx != -1 && xyz_dim_loc->num_sgprs == 3));
   assert(!draw_id_enable || (draw_id_loc->sgpr_idx != -1 && draw_id_loc->num_sgprs == 1));

   const uint32_t ring_entry_reg =
      (R_00B900_COMPUTE_USER_DATA_0 + ring_entry_loc->sgpr_idx * 4 - SI_SH_REG_OFFSET) >> 2;
   const uint32_t xyz_dim_reg =
      !xyz_dim_enable ? 0 : (R_00B900_COMPUTE_USER_DATA_0 + xyz_dim_loc->sgpr_idx * 4 - SI_SH_REG_OFFSET) >> 2;
   const uint32_t draw_id_reg =
      !draw_id_enable ? 0 : (R_00B900_COMPUTE_USER_DATA_0 + draw_id_loc->sgpr_idx * 4 - SI_SH_REG_OFFSET) >> 2;

   radeon_emit(cs, PKT3(PKT3_DISPATCH_TASKMESH_INDIRECT_MULTI_ACE, 9, 0) | PKT3_SHADER_TYPE_S(1));
   radeon_emit(cs, data_va);
   radeon_emit(cs, data_va >> 32);
   radeon_emit(cs, S_AD2_RING_ENTRY_REG(ring_entry_reg));
   radeon_emit(cs, S_AD3_COUNT_INDIRECT_ENABLE(!!count_va) | S_AD3_DRAW_INDEX_ENABLE(draw_id_enable) |
                      S_AD3_XYZ_DIM_ENABLE(xyz_dim_enable) | S_AD3_DRAW_INDEX_REG(draw_id_reg));
   radeon_emit(cs, S_AD4_XYZ_DIM_REG(xyz_dim_reg));
   radeon_emit(cs, draw_count);
   radeon_emit(cs, count_va);
   radeon_emit(cs, count_va >> 32);
   radeon_emit(cs, stride);
   radeon_emit(cs, dispatch_initiator);
}

ALWAYS_INLINE static void
radv_cs_emit_dispatch_taskmesh_gfx_packet(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *mesh_shader = cmd_buffer->state.shaders[MESA_SHADER_MESH];
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   bool predicating = cmd_buffer->state.predicating;

   const struct radv_userdata_info *ring_entry_loc =
      radv_get_user_sgpr(cmd_buffer->state.last_vgt_shader, AC_UD_TASK_RING_ENTRY);

   assert(ring_entry_loc->sgpr_idx != -1);

   uint32_t xyz_dim_reg = (cmd_buffer->state.vtx_base_sgpr - SI_SH_REG_OFFSET) >> 2;
   uint32_t ring_entry_reg = ((mesh_shader->info.user_data_0 - SI_SH_REG_OFFSET) >> 2) + ring_entry_loc->sgpr_idx;
   uint32_t xyz_dim_en = mesh_shader->info.cs.uses_grid_size;
   uint32_t mode1_en = !cmd_buffer->device->mesh_fast_launch_2;
   uint32_t linear_dispatch_en = cmd_buffer->state.shaders[MESA_SHADER_TASK]->info.cs.linear_taskmesh_dispatch;
   const bool sqtt_en = !!cmd_buffer->device->sqtt.bo;

   radeon_emit(cs, PKT3(PKT3_DISPATCH_TASKMESH_GFX, 2, predicating) | PKT3_RESET_FILTER_CAM_S(1));
   radeon_emit(cs, S_4D0_RING_ENTRY_REG(ring_entry_reg) | S_4D0_XYZ_DIM_REG(xyz_dim_reg));
   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11)
      radeon_emit(cs, S_4D1_XYZ_DIM_ENABLE(xyz_dim_en) | S_4D1_MODE1_ENABLE(mode1_en) |
                         S_4D1_LINEAR_DISPATCH_ENABLE(linear_dispatch_en) | S_4D1_THREAD_TRACE_MARKER_ENABLE(sqtt_en));
   else
      radeon_emit(cs, S_4D1_THREAD_TRACE_MARKER_ENABLE(sqtt_en));
   radeon_emit(cs, V_0287F0_DI_SRC_SEL_AUTO_INDEX);
}

ALWAYS_INLINE static void
radv_emit_userdata_vertex_internal(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info,
                                   const uint32_t vertex_offset)
{
   struct radv_cmd_state *state = &cmd_buffer->state;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const bool uses_baseinstance = state->uses_baseinstance;
   const bool uses_drawid = state->uses_drawid;

   radeon_set_sh_reg_seq(cs, state->vtx_base_sgpr, state->vtx_emit_num);

   radeon_emit(cs, vertex_offset);
   state->last_vertex_offset_valid = true;
   state->last_vertex_offset = vertex_offset;
   if (uses_drawid) {
      radeon_emit(cs, 0);
      state->last_drawid = 0;
   }
   if (uses_baseinstance) {
      radeon_emit(cs, info->first_instance);
      state->last_first_instance = info->first_instance;
   }
}

ALWAYS_INLINE static void
radv_emit_userdata_vertex(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info,
                          const uint32_t vertex_offset)
{
   const struct radv_cmd_state *state = &cmd_buffer->state;
   const bool uses_baseinstance = state->uses_baseinstance;
   const bool uses_drawid = state->uses_drawid;

   if (!state->last_vertex_offset_valid || vertex_offset != state->last_vertex_offset ||
       (uses_drawid && 0 != state->last_drawid) ||
       (uses_baseinstance && info->first_instance != state->last_first_instance))
      radv_emit_userdata_vertex_internal(cmd_buffer, info, vertex_offset);
}

ALWAYS_INLINE static void
radv_emit_userdata_vertex_drawid(struct radv_cmd_buffer *cmd_buffer, uint32_t vertex_offset, uint32_t drawid)
{
   struct radv_cmd_state *state = &cmd_buffer->state;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   radeon_set_sh_reg_seq(cs, state->vtx_base_sgpr, 1 + !!drawid);
   radeon_emit(cs, vertex_offset);
   state->last_vertex_offset_valid = true;
   state->last_vertex_offset = vertex_offset;
   if (drawid)
      radeon_emit(cs, drawid);
}

ALWAYS_INLINE static void
radv_emit_userdata_mesh(struct radv_cmd_buffer *cmd_buffer, const uint32_t x, const uint32_t y, const uint32_t z)
{
   struct radv_cmd_state *state = &cmd_buffer->state;
   const struct radv_shader *mesh_shader = state->shaders[MESA_SHADER_MESH];
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const bool uses_drawid = state->uses_drawid;
   const bool uses_grid_size = mesh_shader->info.cs.uses_grid_size;

   if (!uses_drawid && !uses_grid_size)
      return;

   radeon_set_sh_reg_seq(cs, state->vtx_base_sgpr, state->vtx_emit_num);
   if (uses_grid_size) {
      radeon_emit(cs, x);
      radeon_emit(cs, y);
      radeon_emit(cs, z);
   }
   if (uses_drawid) {
      radeon_emit(cs, 0);
      state->last_drawid = 0;
   }
}

ALWAYS_INLINE static void
radv_emit_userdata_task(struct radv_cmd_buffer *cmd_buffer, uint32_t x, uint32_t y, uint32_t z, uint32_t draw_id)
{
   struct radv_shader *task_shader = cmd_buffer->state.shaders[MESA_SHADER_TASK];
   struct radeon_cmdbuf *cs = cmd_buffer->gang.cs;

   const struct radv_userdata_info *xyz_loc = radv_get_user_sgpr(task_shader, AC_UD_CS_GRID_SIZE);
   const struct radv_userdata_info *draw_id_loc = radv_get_user_sgpr(task_shader, AC_UD_CS_TASK_DRAW_ID);

   if (xyz_loc->sgpr_idx != -1) {
      assert(xyz_loc->num_sgprs == 3);
      unsigned xyz_reg = R_00B900_COMPUTE_USER_DATA_0 + xyz_loc->sgpr_idx * 4;

      radeon_set_sh_reg_seq(cs, xyz_reg, 3);
      radeon_emit(cs, x);
      radeon_emit(cs, y);
      radeon_emit(cs, z);
   }

   if (draw_id_loc->sgpr_idx != -1) {
      assert(draw_id_loc->num_sgprs == 1);
      unsigned draw_id_reg = R_00B900_COMPUTE_USER_DATA_0 + draw_id_loc->sgpr_idx * 4;

      radeon_set_sh_reg_seq(cs, draw_id_reg, 1);
      radeon_emit(cs, draw_id);
   }
}

ALWAYS_INLINE static void
radv_emit_draw_packets_indexed(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info,
                               uint32_t drawCount, const VkMultiDrawIndexedInfoEXT *minfo, uint32_t stride,
                               const int32_t *vertexOffset)

{
   struct radv_cmd_state *state = &cmd_buffer->state;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const int index_size = radv_get_vgt_index_size(state->index_type);
   unsigned i = 0;
   const bool uses_drawid = state->uses_drawid;
   const bool can_eop = !uses_drawid && cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10;

   if (uses_drawid) {
      if (vertexOffset) {
         radv_emit_userdata_vertex(cmd_buffer, info, *vertexOffset);
         vk_foreach_multi_draw_indexed (draw, i, minfo, drawCount, stride) {
            uint32_t remaining_indexes = MAX2(state->max_index_count, draw->firstIndex) - draw->firstIndex;
            uint64_t index_va = state->index_va + draw->firstIndex * index_size;

            /* Handle draw calls with 0-sized index buffers if the GPU can't support them. */
            if (!remaining_indexes && cmd_buffer->device->physical_device->rad_info.has_zero_index_buffer_bug)
               radv_handle_zero_index_buffer_bug(cmd_buffer, &index_va, &remaining_indexes);

            if (i > 0)
               radeon_set_sh_reg(cs, state->vtx_base_sgpr + sizeof(uint32_t), i);

            if (!state->render.view_mask) {
               radv_cs_emit_draw_indexed_packet(cmd_buffer, index_va, remaining_indexes, draw->indexCount, false);
            } else {
               u_foreach_bit (view, state->render.view_mask) {
                  radv_emit_view_index(cmd_buffer, view);

                  radv_cs_emit_draw_indexed_packet(cmd_buffer, index_va, remaining_indexes, draw->indexCount, false);
               }
            }
         }
      } else {
         vk_foreach_multi_draw_indexed (draw, i, minfo, drawCount, stride) {
            uint32_t remaining_indexes = MAX2(state->max_index_count, draw->firstIndex) - draw->firstIndex;
            uint64_t index_va = state->index_va + draw->firstIndex * index_size;

            /* Handle draw calls with 0-sized index buffers if the GPU can't support them. */
            if (!remaining_indexes && cmd_buffer->device->physical_device->rad_info.has_zero_index_buffer_bug)
               radv_handle_zero_index_buffer_bug(cmd_buffer, &index_va, &remaining_indexes);

            if (i > 0) {
               assert(state->last_vertex_offset_valid);
               if (state->last_vertex_offset != draw->vertexOffset)
                  radv_emit_userdata_vertex_drawid(cmd_buffer, draw->vertexOffset, i);
               else
                  radeon_set_sh_reg(cs, state->vtx_base_sgpr + sizeof(uint32_t), i);
            } else
               radv_emit_userdata_vertex(cmd_buffer, info, draw->vertexOffset);

            if (!state->render.view_mask) {
               radv_cs_emit_draw_indexed_packet(cmd_buffer, index_va, remaining_indexes, draw->indexCount, false);
            } else {
               u_foreach_bit (view, state->render.view_mask) {
                  radv_emit_view_index(cmd_buffer, view);

                  radv_cs_emit_draw_indexed_packet(cmd_buffer, index_va, remaining_indexes, draw->indexCount, false);
               }
            }
         }
      }
      if (drawCount > 1) {
         state->last_drawid = drawCount - 1;
      }
   } else {
      if (vertexOffset) {
         if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX10) {
            /* GFX10 has a bug that consecutive draw packets with NOT_EOP must not have
             * count == 0 for the last draw that doesn't have NOT_EOP.
             */
            while (drawCount > 1) {
               const VkMultiDrawIndexedInfoEXT *last =
                  (const VkMultiDrawIndexedInfoEXT *)(((const uint8_t *)minfo) + (drawCount - 1) * stride);
               if (last->indexCount)
                  break;
               drawCount--;
            }
         }

         radv_emit_userdata_vertex(cmd_buffer, info, *vertexOffset);
         vk_foreach_multi_draw_indexed (draw, i, minfo, drawCount, stride) {
            uint32_t remaining_indexes = MAX2(state->max_index_count, draw->firstIndex) - draw->firstIndex;
            uint64_t index_va = state->index_va + draw->firstIndex * index_size;

            /* Handle draw calls with 0-sized index buffers if the GPU can't support them. */
            if (!remaining_indexes && cmd_buffer->device->physical_device->rad_info.has_zero_index_buffer_bug)
               radv_handle_zero_index_buffer_bug(cmd_buffer, &index_va, &remaining_indexes);

            if (!state->render.view_mask) {
               radv_cs_emit_draw_indexed_packet(cmd_buffer, index_va, remaining_indexes, draw->indexCount,
                                                can_eop && i < drawCount - 1);
            } else {
               u_foreach_bit (view, state->render.view_mask) {
                  radv_emit_view_index(cmd_buffer, view);

                  radv_cs_emit_draw_indexed_packet(cmd_buffer, index_va, remaining_indexes, draw->indexCount, false);
               }
            }
         }
      } else {
         vk_foreach_multi_draw_indexed (draw, i, minfo, drawCount, stride) {
            uint32_t remaining_indexes = MAX2(state->max_index_count, draw->firstIndex) - draw->firstIndex;
            uint64_t index_va = state->index_va + draw->firstIndex * index_size;

            /* Handle draw calls with 0-sized index buffers if the GPU can't support them. */
            if (!remaining_indexes && cmd_buffer->device->physical_device->rad_info.has_zero_index_buffer_bug)
               radv_handle_zero_index_buffer_bug(cmd_buffer, &index_va, &remaining_indexes);

            const VkMultiDrawIndexedInfoEXT *next =
               (const VkMultiDrawIndexedInfoEXT *)(i < drawCount - 1 ? ((uint8_t *)draw + stride) : NULL);
            const bool offset_changes = next && next->vertexOffset != draw->vertexOffset;
            radv_emit_userdata_vertex(cmd_buffer, info, draw->vertexOffset);

            if (!state->render.view_mask) {
               radv_cs_emit_draw_indexed_packet(cmd_buffer, index_va, remaining_indexes, draw->indexCount,
                                                can_eop && !offset_changes && i < drawCount - 1);
            } else {
               u_foreach_bit (view, state->render.view_mask) {
                  radv_emit_view_index(cmd_buffer, view);

                  radv_cs_emit_draw_indexed_packet(cmd_buffer, index_va, remaining_indexes, draw->indexCount, false);
               }
            }
         }
      }
      if (drawCount > 1) {
         state->last_drawid = drawCount - 1;
      }
   }
}

ALWAYS_INLINE static void
radv_emit_direct_draw_packets(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info, uint32_t drawCount,
                              const VkMultiDrawInfoEXT *minfo, uint32_t use_opaque, uint32_t stride)
{
   unsigned i = 0;
   const uint32_t view_mask = cmd_buffer->state.render.view_mask;
   const bool uses_drawid = cmd_buffer->state.uses_drawid;
   uint32_t last_start = 0;

   vk_foreach_multi_draw (draw, i, minfo, drawCount, stride) {
      if (!i)
         radv_emit_userdata_vertex(cmd_buffer, info, draw->firstVertex);
      else
         radv_emit_userdata_vertex_drawid(cmd_buffer, draw->firstVertex, uses_drawid ? i : 0);

      if (!view_mask) {
         radv_cs_emit_draw_packet(cmd_buffer, draw->vertexCount, use_opaque);
      } else {
         u_foreach_bit (view, view_mask) {
            radv_emit_view_index(cmd_buffer, view);
            radv_cs_emit_draw_packet(cmd_buffer, draw->vertexCount, use_opaque);
         }
      }
      last_start = draw->firstVertex;
   }
   if (drawCount > 1) {
      struct radv_cmd_state *state = &cmd_buffer->state;
      assert(state->last_vertex_offset_valid);
      state->last_vertex_offset = last_start;
      if (uses_drawid)
         state->last_drawid = drawCount - 1;
   }
}

static void
radv_cs_emit_mesh_dispatch_packet(struct radv_cmd_buffer *cmd_buffer, uint32_t x, uint32_t y, uint32_t z)
{
   radeon_emit(cmd_buffer->cs, PKT3(PKT3_DISPATCH_MESH_DIRECT, 3, cmd_buffer->state.predicating));
   radeon_emit(cmd_buffer->cs, x);
   radeon_emit(cmd_buffer->cs, y);
   radeon_emit(cmd_buffer->cs, z);
   radeon_emit(cmd_buffer->cs, S_0287F0_SOURCE_SELECT(V_0287F0_DI_SRC_SEL_AUTO_INDEX));
}

ALWAYS_INLINE static void
radv_emit_direct_mesh_draw_packet(struct radv_cmd_buffer *cmd_buffer, uint32_t x, uint32_t y, uint32_t z)
{
   const uint32_t view_mask = cmd_buffer->state.render.view_mask;

   radv_emit_userdata_mesh(cmd_buffer, x, y, z);

   if (cmd_buffer->device->mesh_fast_launch_2) {
      if (!view_mask) {
         radv_cs_emit_mesh_dispatch_packet(cmd_buffer, x, y, z);
      } else {
         u_foreach_bit (view, view_mask) {
            radv_emit_view_index(cmd_buffer, view);
            radv_cs_emit_mesh_dispatch_packet(cmd_buffer, x, y, z);
         }
      }
   } else {
      const uint32_t count = x * y * z;
      if (!view_mask) {
         radv_cs_emit_draw_packet(cmd_buffer, count, 0);
      } else {
         u_foreach_bit (view, view_mask) {
            radv_emit_view_index(cmd_buffer, view);
            radv_cs_emit_draw_packet(cmd_buffer, count, 0);
         }
      }
   }
}

ALWAYS_INLINE static void
radv_emit_indirect_mesh_draw_packets(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info)
{
   const struct radv_cmd_state *state = &cmd_buffer->state;
   struct radeon_winsys *ws = cmd_buffer->device->ws;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const uint64_t va = radv_buffer_get_va(info->indirect->bo) + info->indirect->offset + info->indirect_offset;
   const uint64_t count_va = !info->count_buffer ? 0
                                                 : radv_buffer_get_va(info->count_buffer->bo) +
                                                      info->count_buffer->offset + info->count_buffer_offset;

   radv_cs_add_buffer(ws, cs, info->indirect->bo);

   if (info->count_buffer) {
      radv_cs_add_buffer(ws, cs, info->count_buffer->bo);
   }

   radeon_emit(cs, PKT3(PKT3_SET_BASE, 2, 0));
   radeon_emit(cs, 1);
   radeon_emit(cs, va);
   radeon_emit(cs, va >> 32);

   if (state->uses_drawid) {
      const struct radv_shader *mesh_shader = state->shaders[MESA_SHADER_MESH];
      unsigned reg = state->vtx_base_sgpr + (mesh_shader->info.cs.uses_grid_size ? 12 : 0);
      radeon_set_sh_reg_seq(cs, reg, 1);
      radeon_emit(cs, 0);
   }

   if (!state->render.view_mask) {
      radv_cs_emit_indirect_mesh_draw_packet(cmd_buffer, info->count, count_va, info->stride);
   } else {
      u_foreach_bit (i, state->render.view_mask) {
         radv_emit_view_index(cmd_buffer, i);
         radv_cs_emit_indirect_mesh_draw_packet(cmd_buffer, info->count, count_va, info->stride);
      }
   }
}

ALWAYS_INLINE static void
radv_emit_direct_taskmesh_draw_packets(struct radv_cmd_buffer *cmd_buffer, uint32_t x, uint32_t y, uint32_t z)
{
   const uint32_t view_mask = cmd_buffer->state.render.view_mask;
   const unsigned num_views = MAX2(1, util_bitcount(view_mask));
   unsigned ace_predication_size = num_views * 6; /* DISPATCH_TASKMESH_DIRECT_ACE size */

   if (num_views > 1)
      ace_predication_size += num_views * 3; /* SET_SH_REG size (view index SGPR) */

   radv_emit_userdata_task(cmd_buffer, x, y, z, 0);
   radv_cs_emit_compute_predication(cmd_buffer->device, &cmd_buffer->state, cmd_buffer->gang.cs,
                                    cmd_buffer->mec_inv_pred_va, &cmd_buffer->mec_inv_pred_emitted,
                                    ace_predication_size);

   if (!view_mask) {
      radv_cs_emit_dispatch_taskmesh_direct_ace_packet(cmd_buffer, x, y, z);
      radv_cs_emit_dispatch_taskmesh_gfx_packet(cmd_buffer);
   } else {
      u_foreach_bit (view, view_mask) {
         radv_emit_view_index(cmd_buffer, view);
         radv_cs_emit_dispatch_taskmesh_direct_ace_packet(cmd_buffer, x, y, z);
         radv_cs_emit_dispatch_taskmesh_gfx_packet(cmd_buffer);
      }
   }
}

static void
radv_emit_indirect_taskmesh_draw_packets(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info)
{
   const uint32_t view_mask = cmd_buffer->state.render.view_mask;
   struct radeon_winsys *ws = cmd_buffer->device->ws;
   const unsigned num_views = MAX2(1, util_bitcount(view_mask));
   unsigned ace_predication_size = num_views * 11; /* DISPATCH_TASKMESH_INDIRECT_MULTI_ACE size */
   struct radeon_cmdbuf *ace_cs = cmd_buffer->gang.cs;

   const uint64_t va = radv_buffer_get_va(info->indirect->bo) + info->indirect->offset + info->indirect_offset;
   const uint64_t count_va = !info->count_buffer ? 0
                                                 : radv_buffer_get_va(info->count_buffer->bo) +
                                                      info->count_buffer->offset + info->count_buffer_offset;
   uint64_t workaround_cond_va = 0;

   if (num_views > 1)
      ace_predication_size += num_views * 3; /* SET_SH_REG size (view index SGPR) */

   if (count_va)
      radv_cs_add_buffer(ws, cmd_buffer->gang.cs, info->count_buffer->bo);

   if (cmd_buffer->device->physical_device->rad_info.has_taskmesh_indirect0_bug && count_va) {
      /* MEC firmware bug workaround.
       * When the count buffer contains zero, DISPATCH_TASKMESH_INDIRECT_MULTI_ACE hangs.
       * - We must ensure that DISPATCH_TASKMESH_INDIRECT_MULTI_ACE
       *   is only executed when the count buffer contains non-zero.
       * - Furthermore, we must also ensure that each DISPATCH_TASKMESH_GFX packet
       *   has a matching ACE packet.
       *
       * As a workaround:
       * - Reserve a dword in the upload buffer and initialize it to 1 for the workaround
       * - When count != 0, write 0 to the workaround BO and execute the indirect dispatch
       * - When workaround BO != 0 (count was 0), execute an empty direct dispatch
       */

      uint32_t workaround_cond_init = 0;
      uint32_t workaround_cond_off;
      if (!radv_cmd_buffer_upload_data(cmd_buffer, 4, &workaround_cond_init, &workaround_cond_off))
         vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);

      workaround_cond_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + workaround_cond_off;

      radeon_emit(ace_cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(ace_cs,
                  COPY_DATA_SRC_SEL(COPY_DATA_IMM) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) | COPY_DATA_WR_CONFIRM);
      radeon_emit(ace_cs, 1);
      radeon_emit(ace_cs, 0);
      radeon_emit(ace_cs, workaround_cond_va);
      radeon_emit(ace_cs, workaround_cond_va >> 32);

      /* 2x COND_EXEC + 1x COPY_DATA + Nx DISPATCH_TASKMESH_DIRECT_ACE */
      ace_predication_size += 2 * 5 + 6 + 6 * num_views;
   }

   radv_cs_add_buffer(ws, cmd_buffer->gang.cs, info->indirect->bo);
   radv_cs_emit_compute_predication(cmd_buffer->device, &cmd_buffer->state, cmd_buffer->gang.cs,
                                    cmd_buffer->mec_inv_pred_va, &cmd_buffer->mec_inv_pred_emitted,
                                    ace_predication_size);

   if (workaround_cond_va) {
      radeon_emit(ace_cs, PKT3(PKT3_COND_EXEC, 3, 0));
      radeon_emit(ace_cs, count_va);
      radeon_emit(ace_cs, count_va >> 32);
      radeon_emit(ace_cs, 0);
      radeon_emit(ace_cs, 6 + 11 * num_views); /* 1x COPY_DATA + Nx DISPATCH_TASKMESH_INDIRECT_MULTI_ACE */

      radeon_emit(ace_cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(ace_cs,
                  COPY_DATA_SRC_SEL(COPY_DATA_IMM) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) | COPY_DATA_WR_CONFIRM);
      radeon_emit(ace_cs, 0);
      radeon_emit(ace_cs, 0);
      radeon_emit(ace_cs, workaround_cond_va);
      radeon_emit(ace_cs, workaround_cond_va >> 32);
   }

   if (!view_mask) {
      radv_cs_emit_dispatch_taskmesh_indirect_multi_ace_packet(cmd_buffer, va, info->count, count_va, info->stride);
      radv_cs_emit_dispatch_taskmesh_gfx_packet(cmd_buffer);
   } else {
      u_foreach_bit (view, view_mask) {
         radv_emit_view_index(cmd_buffer, view);
         radv_cs_emit_dispatch_taskmesh_indirect_multi_ace_packet(cmd_buffer, va, info->count, count_va, info->stride);
         radv_cs_emit_dispatch_taskmesh_gfx_packet(cmd_buffer);
      }
   }

   if (workaround_cond_va) {
      radeon_emit(ace_cs, PKT3(PKT3_COND_EXEC, 3, 0));
      radeon_emit(ace_cs, workaround_cond_va);
      radeon_emit(ace_cs, workaround_cond_va >> 32);
      radeon_emit(ace_cs, 0);
      radeon_emit(ace_cs, 6 * num_views); /* Nx DISPATCH_TASKMESH_DIRECT_ACE */

      for (unsigned v = 0; v < num_views; ++v) {
         radv_cs_emit_dispatch_taskmesh_direct_ace_packet(cmd_buffer, 0, 0, 0);
      }
   }
}

static void
radv_emit_indirect_draw_packets(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info)
{
   const struct radv_cmd_state *state = &cmd_buffer->state;
   struct radeon_winsys *ws = cmd_buffer->device->ws;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const uint64_t va = radv_buffer_get_va(info->indirect->bo) + info->indirect->offset + info->indirect_offset;
   const uint64_t count_va = info->count_buffer ? radv_buffer_get_va(info->count_buffer->bo) +
                                                     info->count_buffer->offset + info->count_buffer_offset
                                                : 0;

   radv_cs_add_buffer(ws, cs, info->indirect->bo);

   radeon_emit(cs, PKT3(PKT3_SET_BASE, 2, 0));
   radeon_emit(cs, 1);
   radeon_emit(cs, va);
   radeon_emit(cs, va >> 32);

   if (info->count_buffer) {
      radv_cs_add_buffer(ws, cs, info->count_buffer->bo);
   }

   if (!state->render.view_mask) {
      radv_cs_emit_indirect_draw_packet(cmd_buffer, info->indexed, info->count, count_va, info->stride);
   } else {
      u_foreach_bit (i, state->render.view_mask) {
         radv_emit_view_index(cmd_buffer, i);

         radv_cs_emit_indirect_draw_packet(cmd_buffer, info->indexed, info->count, count_va, info->stride);
      }
   }
}

/*
 * Vega and raven have a bug which triggers if there are multiple context
 * register contexts active at the same time with different scissor values.
 *
 * There are two possible workarounds:
 * 1) Wait for PS_PARTIAL_FLUSH every time the scissor is changed. That way
 *    there is only ever 1 active set of scissor values at the same time.
 *
 * 2) Whenever the hardware switches contexts we have to set the scissor
 *    registers again even if it is a noop. That way the new context gets
 *    the correct scissor values.
 *
 * This implements option 2. radv_need_late_scissor_emission needs to
 * return true on affected HW if radv_emit_all_graphics_states sets
 * any context registers.
 */
static bool
radv_need_late_scissor_emission(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info)
{
   if (cmd_buffer->state.context_roll_without_scissor_emitted || info->strmout_buffer)
      return true;

   uint64_t used_states = cmd_buffer->state.graphics_pipeline->needed_dynamic_state | ~RADV_CMD_DIRTY_DYNAMIC_ALL;

   /* Index, vertex and streamout buffers don't change context regs.
    * We assume that any other dirty flag causes context rolls.
    */
   used_states &= ~(RADV_CMD_DIRTY_INDEX_BUFFER | RADV_CMD_DIRTY_VERTEX_BUFFER | RADV_CMD_DIRTY_DYNAMIC_VERTEX_INPUT |
                    RADV_CMD_DIRTY_STREAMOUT_BUFFER);

   return cmd_buffer->state.dirty & used_states;
}

ALWAYS_INLINE static uint32_t
radv_get_ngg_culling_settings(struct radv_cmd_buffer *cmd_buffer, bool vp_y_inverted)
{
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;

   /* Disable shader culling entirely when conservative overestimate is used.
    * The face culling algorithm can delete very tiny triangles (even if unintended).
    */
   if (d->vk.rs.conservative_mode == VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT)
      return radv_nggc_none;

   /* With graphics pipeline library, NGG culling is unconditionally compiled into shaders
    * because we don't know the primitive topology at compile time, so we should
    * disable it dynamically for points or lines.
    */
   const unsigned num_vertices_per_prim = radv_conv_prim_to_gs_out(d->vk.ia.primitive_topology, true) + 1;
   if (num_vertices_per_prim != 3)
      return radv_nggc_none;

   /* Cull every triangle when rasterizer discard is enabled. */
   if (d->vk.rs.rasterizer_discard_enable)
      return radv_nggc_front_face | radv_nggc_back_face;

   uint32_t nggc_settings = radv_nggc_none;

   /* The culling code needs to know whether face is CW or CCW. */
   bool ccw = d->vk.rs.front_face == VK_FRONT_FACE_COUNTER_CLOCKWISE;

   /* Take inverted viewport into account. */
   ccw ^= vp_y_inverted;

   if (ccw)
      nggc_settings |= radv_nggc_face_is_ccw;

   /* Face culling settings. */
   if (d->vk.rs.cull_mode & VK_CULL_MODE_FRONT_BIT)
      nggc_settings |= radv_nggc_front_face;
   if (d->vk.rs.cull_mode & VK_CULL_MODE_BACK_BIT)
      nggc_settings |= radv_nggc_back_face;

   /* Small primitive culling assumes a sample position at (0.5, 0.5)
    * so don't enable it with user sample locations.
    */
   if (!d->vk.ms.sample_locations_enable) {
      nggc_settings |= radv_nggc_small_primitives;

      /* small_prim_precision = num_samples / 2^subpixel_bits
       * num_samples is also always a power of two, so the small prim precision can only be
       * a power of two between 2^-2 and 2^-6, therefore it's enough to remember the exponent.
       */
      unsigned rasterization_samples = radv_get_rasterization_samples(cmd_buffer);
      unsigned subpixel_bits = 256;
      int32_t small_prim_precision_log2 = util_logbase2(rasterization_samples) - util_logbase2(subpixel_bits);
      nggc_settings |= ((uint32_t)small_prim_precision_log2 << 24u);
   }

   return nggc_settings;
}

static void
radv_emit_ngg_culling_state(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *last_vgt_shader = cmd_buffer->state.last_vgt_shader;
   const uint32_t base_reg = last_vgt_shader->info.user_data_0;

   /* Get viewport transform. */
   float vp_scale[2], vp_translate[2];
   memcpy(vp_scale, cmd_buffer->state.dynamic.hw_vp.xform[0].scale, 2 * sizeof(float));
   memcpy(vp_translate, cmd_buffer->state.dynamic.hw_vp.xform[0].translate, 2 * sizeof(float));
   bool vp_y_inverted = (-vp_scale[1] + vp_translate[1]) > (vp_scale[1] + vp_translate[1]);

   /* Get current culling settings. */
   uint32_t nggc_settings = radv_get_ngg_culling_settings(cmd_buffer, vp_y_inverted);

   if (cmd_buffer->state.dirty &
       (RADV_CMD_DIRTY_PIPELINE | RADV_CMD_DIRTY_DYNAMIC_VIEWPORT | RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES)) {
      /* Correction for inverted Y */
      if (vp_y_inverted) {
         vp_scale[1] = -vp_scale[1];
         vp_translate[1] = -vp_translate[1];
      }

      /* Correction for number of samples per pixel. */
      for (unsigned i = 0; i < 2; ++i) {
         vp_scale[i] *= (float)cmd_buffer->state.dynamic.vk.ms.rasterization_samples;
         vp_translate[i] *= (float)cmd_buffer->state.dynamic.vk.ms.rasterization_samples;
      }

      uint32_t vp_reg_values[4] = {fui(vp_scale[0]), fui(vp_scale[1]), fui(vp_translate[0]), fui(vp_translate[1])};
      const int8_t vp_sgpr_idx = radv_get_user_sgpr(last_vgt_shader, AC_UD_NGG_VIEWPORT)->sgpr_idx;
      assert(vp_sgpr_idx != -1);
      radeon_set_sh_reg_seq(cmd_buffer->cs, base_reg + vp_sgpr_idx * 4, 4);
      radeon_emit_array(cmd_buffer->cs, vp_reg_values, 4);
   }

   const int8_t nggc_sgpr_idx = radv_get_user_sgpr(last_vgt_shader, AC_UD_NGG_CULLING_SETTINGS)->sgpr_idx;
   assert(nggc_sgpr_idx != -1);

   radeon_set_sh_reg(cmd_buffer->cs, base_reg + nggc_sgpr_idx * 4, nggc_settings);
}

static void
radv_emit_fs_state(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radv_shader *ps = cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT];
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   const struct radv_userdata_info *loc;

   if (!ps)
      return;

   loc = radv_get_user_sgpr(ps, AC_UD_PS_STATE);
   if (loc->sgpr_idx == -1)
      return;
   assert(loc->num_sgprs == 1);

   const unsigned rasterization_samples = radv_get_rasterization_samples(cmd_buffer);
   const unsigned ps_iter_samples = radv_get_ps_iter_samples(cmd_buffer);
   const uint16_t ps_iter_mask = ac_get_ps_iter_mask(ps_iter_samples);
   const unsigned rast_prim = radv_get_rasterization_prim(cmd_buffer);
   const uint32_t base_reg = ps->info.user_data_0;
   const unsigned ps_state = SET_SGPR_FIELD(PS_STATE_NUM_SAMPLES, rasterization_samples) |
                             SET_SGPR_FIELD(PS_STATE_PS_ITER_MASK, ps_iter_mask) |
                             SET_SGPR_FIELD(PS_STATE_LINE_RAST_MODE, d->vk.rs.line.mode) |
                             SET_SGPR_FIELD(PS_STATE_RAST_PRIM, rast_prim);

   radeon_set_sh_reg(cmd_buffer->cs, base_reg + loc->sgpr_idx * 4, ps_state);
}

static void
radv_emit_db_shader_control(struct radv_cmd_buffer *cmd_buffer)
{
   const struct radeon_info *rad_info = &cmd_buffer->device->physical_device->rad_info;
   const struct radv_shader *ps = cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT];
   const struct radv_dynamic_state *d = &cmd_buffer->state.dynamic;
   const bool uses_ds_feedback_loop =
      !!(d->feedback_loop_aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
   const unsigned rasterization_samples = radv_get_rasterization_samples(cmd_buffer);

   uint32_t db_shader_control;

   if (ps) {
      db_shader_control = ps->info.ps.db_shader_control;
   } else {
      db_shader_control = S_02880C_CONSERVATIVE_Z_EXPORT(V_02880C_EXPORT_ANY_Z) |
                          S_02880C_Z_ORDER(V_02880C_EARLY_Z_THEN_LATE_Z) |
                          S_02880C_DUAL_QUAD_DISABLE(rad_info->has_rbplus && !rad_info->rbplus_allowed);
   }

   /* When a depth/stencil attachment is used inside feedback loops, use LATE_Z to make sure shader invocations read the
    * correct value.
    * Also apply the bug workaround for smoothing (overrasterization) on GFX6.
    */
   if (uses_ds_feedback_loop ||
       (rad_info->gfx_level == GFX6 && d->vk.rs.line.mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT))
      db_shader_control = (db_shader_control & C_02880C_Z_ORDER) | S_02880C_Z_ORDER(V_02880C_LATE_Z);

   if (ps && ps->info.ps.pops) {
      /* POPS_OVERLAP_NUM_SAMPLES (OVERRIDE_INTRINSIC_RATE on GFX11, must always be enabled for POPS) controls the
       * interlock granularity.
       * PixelInterlock: 1x.
       * SampleInterlock: MSAA_EXPOSED_SAMPLES (much faster at common edges of adjacent primitives with MSAA).
       */
      if (rad_info->gfx_level >= GFX11) {
         db_shader_control |= S_02880C_OVERRIDE_INTRINSIC_RATE_ENABLE(1);
         if (ps->info.ps.pops_is_per_sample)
            db_shader_control |= S_02880C_OVERRIDE_INTRINSIC_RATE(util_logbase2(rasterization_samples));
      } else {
         if (ps->info.ps.pops_is_per_sample)
            db_shader_control |= S_02880C_POPS_OVERLAP_NUM_SAMPLES(util_logbase2(rasterization_samples));

         if (rad_info->has_pops_missed_overlap_bug) {
            radeon_set_context_reg(cmd_buffer->cs, R_028060_DB_DFSM_CONTROL,
                                   S_028060_PUNCHOUT_MODE(V_028060_FORCE_OFF) |
                                      S_028060_POPS_DRAIN_PS_ON_OVERLAP(rasterization_samples >= 8));
         }
      }
   } else if (rad_info->has_export_conflict_bug && rasterization_samples == 1) {
      for (uint32_t i = 0; i < MAX_RTS; i++) {
         if (d->vk.cb.attachments[i].write_mask && d->vk.cb.attachments[i].blend_enable) {
            db_shader_control |= S_02880C_OVERRIDE_INTRINSIC_RATE_ENABLE(1) | S_02880C_OVERRIDE_INTRINSIC_RATE(2);
            break;
         }
      }
   }

   if (db_shader_control != cmd_buffer->state.last_db_shader_control) {
      radeon_set_context_reg(cmd_buffer->cs, R_02880C_DB_SHADER_CONTROL, db_shader_control);

      cmd_buffer->state.last_db_shader_control = db_shader_control;
   }

   cmd_buffer->state.dirty &= ~RADV_CMD_DIRTY_DB_SHADER_CONTROL;
}

static void
radv_emit_all_graphics_states(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info)
{
   const struct radv_device *device = cmd_buffer->device;
   struct radv_shader_part *tcs_epilog = NULL, *ps_epilog = NULL;

   if (cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT] &&
       cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT]->info.has_epilog) {
      if ((cmd_buffer->state.emitted_graphics_pipeline != cmd_buffer->state.graphics_pipeline ||
           (cmd_buffer->state.dirty &
            (RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_MASK | RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_ENABLE |
             RADV_CMD_DIRTY_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE | RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_EQUATION)))) {
         ps_epilog = lookup_ps_epilog(cmd_buffer);
         if (!ps_epilog) {
            vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
            return;
         }

         cmd_buffer->state.col_format_non_compacted = ps_epilog->spi_shader_col_format;

         bool need_null_export_workaround = radv_needs_null_export_workaround(
            device, cmd_buffer->state.shaders[MESA_SHADER_FRAGMENT], cmd_buffer->state.custom_blend_mode);

         if (need_null_export_workaround && !cmd_buffer->state.col_format_non_compacted)
            cmd_buffer->state.col_format_non_compacted = V_028714_SPI_SHADER_32_R;
         if (device->physical_device->rad_info.rbplus_allowed)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_RBPLUS;
      }
   }

   if (cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL] &&
       cmd_buffer->state.shaders[MESA_SHADER_TESS_CTRL]->info.has_epilog) {
      tcs_epilog = lookup_tcs_epilog(cmd_buffer);
      if (!tcs_epilog) {
         vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
         return;
      }
   }

   /* Determine whether GFX9 late scissor workaround should be applied based on:
    * 1. radv_need_late_scissor_emission
    * 2. any dirty dynamic flags that may cause context rolls
    */
   const bool late_scissor_emission = cmd_buffer->device->physical_device->rad_info.has_gfx9_scissor_bug
                                         ? radv_need_late_scissor_emission(cmd_buffer, info)
                                         : false;

   if (cmd_buffer->state.dirty & RADV_CMD_DIRTY_RBPLUS)
      radv_emit_rbplus_state(cmd_buffer);

   if (cmd_buffer->state.dirty & RADV_CMD_DIRTY_SHADER_QUERY)
      radv_flush_shader_query_state(cmd_buffer);

   if (cmd_buffer->state.dirty & RADV_CMD_DIRTY_OCCLUSION_QUERY)
      radv_flush_occlusion_query_state(cmd_buffer);

   if ((cmd_buffer->state.dirty &
        (RADV_CMD_DIRTY_PIPELINE | RADV_CMD_DIRTY_DYNAMIC_CULL_MODE | RADV_CMD_DIRTY_DYNAMIC_FRONT_FACE |
         RADV_CMD_DIRTY_DYNAMIC_RASTERIZER_DISCARD_ENABLE | RADV_CMD_DIRTY_DYNAMIC_VIEWPORT |
         RADV_CMD_DIRTY_DYNAMIC_CONSERVATIVE_RAST_MODE | RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES |
         RADV_CMD_DIRTY_DYNAMIC_PRIMITIVE_TOPOLOGY | RADV_CMD_DIRTY_DYNAMIC_SAMPLE_LOCATIONS_ENABLE)) &&
       cmd_buffer->state.has_nggc)
      radv_emit_ngg_culling_state(cmd_buffer);

   if (cmd_buffer->state.dirty &
       (RADV_CMD_DIRTY_FRAMEBUFFER | RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_MASK |
        RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES | RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE))
      radv_emit_binning_state(cmd_buffer);

   if (cmd_buffer->state.dirty & RADV_CMD_DIRTY_PIPELINE)
      radv_emit_graphics_pipeline(cmd_buffer);

   if (ps_epilog)
      radv_emit_ps_epilog_state(cmd_buffer, ps_epilog);

   if (tcs_epilog)
      radv_emit_tcs_epilog_state(cmd_buffer, tcs_epilog);

   if (cmd_buffer->state.dirty & RADV_CMD_DIRTY_FRAMEBUFFER)
      radv_emit_framebuffer_state(cmd_buffer);

   if (cmd_buffer->state.dirty & RADV_CMD_DIRTY_GUARDBAND)
      radv_emit_guardband_state(cmd_buffer);

   if (cmd_buffer->state.dirty &
       (RADV_CMD_DIRTY_DB_SHADER_CONTROL | RADV_CMD_DIRTY_DYNAMIC_COLOR_WRITE_MASK |
        RADV_CMD_DIRTY_DYNAMIC_COLOR_BLEND_ENABLE | RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES |
        RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE | RADV_CMD_DIRTY_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE))
      radv_emit_db_shader_control(cmd_buffer);

   if (info->indexed && info->indirect && cmd_buffer->state.dirty & RADV_CMD_DIRTY_INDEX_BUFFER)
      radv_emit_index_buffer(cmd_buffer);

   const uint64_t dynamic_states =
      cmd_buffer->state.dirty & cmd_buffer->state.emitted_graphics_pipeline->needed_dynamic_state;
   if (dynamic_states) {
      radv_cmd_buffer_flush_dynamic_state(cmd_buffer, dynamic_states);

      if (dynamic_states &
          (RADV_CMD_DIRTY_DYNAMIC_RASTERIZATION_SAMPLES | RADV_CMD_DIRTY_DYNAMIC_LINE_RASTERIZATION_MODE))
         radv_emit_fs_state(cmd_buffer);
   }

   radv_emit_draw_registers(cmd_buffer, info);

   if (late_scissor_emission) {
      radv_emit_scissor(cmd_buffer);
      cmd_buffer->state.context_roll_without_scissor_emitted = false;
   }
}

/* MUST inline this function to avoid massive perf loss in drawoverhead */
ALWAYS_INLINE static bool
radv_before_draw(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info, uint32_t drawCount, bool dgc)
{
   const bool has_prefetch = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7;

   ASSERTED const unsigned cdw_max =
      radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 4096 + 128 * (drawCount - 1));

   if (likely(!info->indirect)) {
      /* GFX6-GFX7 treat instance_count==0 as instance_count==1. There is
       * no workaround for indirect draws, but we can at least skip
       * direct draws.
       */
      if (unlikely(!info->instance_count))
         return false;

      /* Handle count == 0. */
      if (unlikely(!info->count && !info->strmout_buffer))
         return false;
   }

   if (!info->indexed && cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7) {
      /* On GFX7 and later, non-indexed draws overwrite VGT_INDEX_TYPE,
       * so the state must be re-emitted before the next indexed
       * draw.
       */
      cmd_buffer->state.last_index_type = -1;
   }

   /* Use optimal packet order based on whether we need to sync the
    * pipeline.
    */
   if (cmd_buffer->state.flush_bits & (RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB |
                                       RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_CS_PARTIAL_FLUSH)) {
      /* If we have to wait for idle, set all states first, so that
       * all SET packets are processed in parallel with previous draw
       * calls. Then upload descriptors, set shader pointers, and
       * draw, and prefetch at the end. This ensures that the time
       * the CUs are idle is very short. (there are only SET_SH
       * packets between the wait and the draw)
       */
      radv_emit_all_graphics_states(cmd_buffer, info);
      radv_emit_cache_flush(cmd_buffer);
      /* <-- CUs are idle here --> */

      radv_upload_graphics_shader_descriptors(cmd_buffer);
   } else {
      const bool need_prefetch = has_prefetch && cmd_buffer->state.prefetch_L2_mask;

      /* If we don't wait for idle, start prefetches first, then set
       * states, and draw at the end.
       */
      radv_emit_cache_flush(cmd_buffer);

      if (need_prefetch) {
         /* Only prefetch the vertex shader and VBO descriptors
          * in order to start the draw as soon as possible.
          */
         radv_emit_prefetch_L2(cmd_buffer, true);
      }

      radv_upload_graphics_shader_descriptors(cmd_buffer);

      radv_emit_all_graphics_states(cmd_buffer, info);
   }

   if (!dgc)
      radv_describe_draw(cmd_buffer);
   if (likely(!info->indirect)) {
      struct radv_cmd_state *state = &cmd_buffer->state;
      struct radeon_cmdbuf *cs = cmd_buffer->cs;
      assert(state->vtx_base_sgpr);
      if (state->last_num_instances != info->instance_count) {
         radeon_emit(cs, PKT3(PKT3_NUM_INSTANCES, 0, false));
         radeon_emit(cs, info->instance_count);
         state->last_num_instances = info->instance_count;
      }
   }
   assert(cmd_buffer->cs->cdw <= cdw_max);

   return true;
}

ALWAYS_INLINE static bool
radv_before_taskmesh_draw(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *info, uint32_t drawCount,
                          bool dgc)
{
   /* For direct draws, this makes sure we don't draw anything.
    * For indirect draws, this is necessary to prevent a GPU hang (on MEC version < 100).
    */
   if (unlikely(!info->count))
      return false;

   struct radeon_cmdbuf *ace_cs = cmd_buffer->gang.cs;
   struct radv_shader *task_shader = cmd_buffer->state.shaders[MESA_SHADER_TASK];

   assert(!task_shader || ace_cs);

   const VkShaderStageFlags stages =
      VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT | (task_shader ? VK_SHADER_STAGE_TASK_BIT_EXT : 0);
   const bool need_task_semaphore = task_shader && radv_flush_gang_leader_semaphore(cmd_buffer);

   ASSERTED const unsigned cdw_max =
      radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 4096 + 128 * (drawCount - 1));
   ASSERTED const unsigned ace_cdw_max =
      !ace_cs ? 0 : radeon_check_space(cmd_buffer->device->ws, ace_cs, 4096 + 128 * (drawCount - 1));

   radv_emit_all_graphics_states(cmd_buffer, info);

   radv_emit_cache_flush(cmd_buffer);

   if (task_shader) {
      radv_gang_cache_flush(cmd_buffer);

      if (need_task_semaphore) {
         radv_wait_gang_leader(cmd_buffer);
      }
   }

   radv_flush_descriptors(cmd_buffer, stages, VK_PIPELINE_BIND_POINT_GRAPHICS);

   const VkShaderStageFlags pc_stages = radv_must_flush_constants(cmd_buffer, stages, VK_PIPELINE_BIND_POINT_GRAPHICS);
   if (pc_stages)
      radv_flush_constants(cmd_buffer, pc_stages, VK_PIPELINE_BIND_POINT_GRAPHICS);

   if (!dgc)
      radv_describe_draw(cmd_buffer);
   if (likely(!info->indirect)) {
      struct radv_cmd_state *state = &cmd_buffer->state;
      if (unlikely(state->last_num_instances != 1)) {
         struct radeon_cmdbuf *cs = cmd_buffer->cs;
         radeon_emit(cs, PKT3(PKT3_NUM_INSTANCES, 0, false));
         radeon_emit(cs, 1);
         state->last_num_instances = 1;
      }
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);
   assert(!ace_cs || ace_cs->cdw <= ace_cdw_max);

   cmd_buffer->state.last_index_type = -1;

   return true;
}

ALWAYS_INLINE static void
radv_after_draw(struct radv_cmd_buffer *cmd_buffer, bool dgc)
{
   const struct radeon_info *rad_info = &cmd_buffer->device->physical_device->rad_info;
   bool has_prefetch = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7;
   /* Start prefetches after the draw has been started. Both will
    * run in parallel, but starting the draw first is more
    * important.
    */
   if (has_prefetch && cmd_buffer->state.prefetch_L2_mask) {
      radv_emit_prefetch_L2(cmd_buffer, false);
   }

   /* Workaround for a VGT hang when streamout is enabled.
    * It must be done after drawing.
    */
   if (radv_is_streamout_enabled(cmd_buffer) &&
       (rad_info->family == CHIP_HAWAII || rad_info->family == CHIP_TONGA || rad_info->family == CHIP_FIJI)) {
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_VGT_STREAMOUT_SYNC;
   }

   radv_cmd_buffer_after_draw(cmd_buffer, RADV_CMD_FLAG_PS_PARTIAL_FLUSH, dgc);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
             uint32_t firstInstance)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_draw_info info;

   info.count = vertexCount;
   info.instance_count = instanceCount;
   info.first_instance = firstInstance;
   info.strmout_buffer = NULL;
   info.indirect = NULL;
   info.indexed = false;

   if (!radv_before_draw(cmd_buffer, &info, 1, false))
      return;
   const VkMultiDrawInfoEXT minfo = {firstVertex, vertexCount};
   radv_emit_direct_draw_packets(cmd_buffer, &info, 1, &minfo, 0, 0);
   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT *pVertexInfo,
                     uint32_t instanceCount, uint32_t firstInstance, uint32_t stride)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_draw_info info;

   if (!drawCount)
      return;

   info.count = pVertexInfo->vertexCount;
   info.instance_count = instanceCount;
   info.first_instance = firstInstance;
   info.strmout_buffer = NULL;
   info.indirect = NULL;
   info.indexed = false;

   if (!radv_before_draw(cmd_buffer, &info, drawCount, false))
      return;
   radv_emit_direct_draw_packets(cmd_buffer, &info, drawCount, pVertexInfo, 0, stride);
   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                    int32_t vertexOffset, uint32_t firstInstance)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_draw_info info;

   info.indexed = true;
   info.count = indexCount;
   info.instance_count = instanceCount;
   info.first_instance = firstInstance;
   info.strmout_buffer = NULL;
   info.indirect = NULL;

   if (!radv_before_draw(cmd_buffer, &info, 1, false))
      return;
   const VkMultiDrawIndexedInfoEXT minfo = {firstIndex, indexCount, vertexOffset};
   radv_emit_draw_packets_indexed(cmd_buffer, &info, 1, &minfo, 0, NULL);
   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                            const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount, uint32_t firstInstance,
                            uint32_t stride, const int32_t *pVertexOffset)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_draw_info info;

   if (!drawCount)
      return;

   const VkMultiDrawIndexedInfoEXT *minfo = pIndexInfo;
   info.indexed = true;
   info.count = minfo->indexCount;
   info.instance_count = instanceCount;
   info.first_instance = firstInstance;
   info.strmout_buffer = NULL;
   info.indirect = NULL;

   if (!radv_before_draw(cmd_buffer, &info, drawCount, false))
      return;
   radv_emit_draw_packets_indexed(cmd_buffer, &info, drawCount, pIndexInfo, stride, pVertexOffset);
   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer _buffer, VkDeviceSize offset, uint32_t drawCount,
                     uint32_t stride)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, _buffer);
   struct radv_draw_info info;

   info.count = drawCount;
   info.indirect = buffer;
   info.indirect_offset = offset;
   info.stride = stride;
   info.strmout_buffer = NULL;
   info.count_buffer = NULL;
   info.indexed = false;
   info.instance_count = 0;

   if (!radv_before_draw(cmd_buffer, &info, 1, false))
      return;
   radv_emit_indirect_draw_packets(cmd_buffer, &info);
   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer _buffer, VkDeviceSize offset, uint32_t drawCount,
                            uint32_t stride)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, _buffer);
   struct radv_draw_info info;

   info.indexed = true;
   info.count = drawCount;
   info.indirect = buffer;
   info.indirect_offset = offset;
   info.stride = stride;
   info.count_buffer = NULL;
   info.strmout_buffer = NULL;
   info.instance_count = 0;

   if (!radv_before_draw(cmd_buffer, &info, 1, false))
      return;
   radv_emit_indirect_draw_packets(cmd_buffer, &info);
   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer _buffer, VkDeviceSize offset, VkBuffer _countBuffer,
                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, _buffer);
   RADV_FROM_HANDLE(radv_buffer, count_buffer, _countBuffer);
   struct radv_draw_info info;

   info.count = maxDrawCount;
   info.indirect = buffer;
   info.indirect_offset = offset;
   info.count_buffer = count_buffer;
   info.count_buffer_offset = countBufferOffset;
   info.stride = stride;
   info.strmout_buffer = NULL;
   info.indexed = false;
   info.instance_count = 0;

   if (!radv_before_draw(cmd_buffer, &info, 1, false))
      return;
   radv_emit_indirect_draw_packets(cmd_buffer, &info);
   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer _buffer, VkDeviceSize offset,
                                 VkBuffer _countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                 uint32_t stride)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, _buffer);
   RADV_FROM_HANDLE(radv_buffer, count_buffer, _countBuffer);
   struct radv_draw_info info;

   info.indexed = true;
   info.count = maxDrawCount;
   info.indirect = buffer;
   info.indirect_offset = offset;
   info.count_buffer = count_buffer;
   info.count_buffer_offset = countBufferOffset;
   info.stride = stride;
   info.strmout_buffer = NULL;
   info.instance_count = 0;

   if (!radv_before_draw(cmd_buffer, &info, 1, false))
      return;
   radv_emit_indirect_draw_packets(cmd_buffer, &info);
   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_draw_info info;

   info.count = x * y * z;
   info.instance_count = 1;
   info.first_instance = 0;
   info.stride = 0;
   info.indexed = false;
   info.strmout_buffer = NULL;
   info.count_buffer = NULL;
   info.indirect = NULL;

   if (!radv_before_taskmesh_draw(cmd_buffer, &info, 1, false))
      return;

   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TASK)) {
      radv_emit_direct_taskmesh_draw_packets(cmd_buffer, x, y, z);
   } else {
      radv_emit_direct_mesh_draw_packet(cmd_buffer, x, y, z);
   }

   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer _buffer, VkDeviceSize offset,
                                 uint32_t drawCount, uint32_t stride)
{
   if (!drawCount)
      return;

   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, _buffer);

   struct radv_draw_info info;

   info.indirect = buffer;
   info.indirect_offset = offset;
   info.stride = stride;
   info.count = drawCount;
   info.strmout_buffer = NULL;
   info.count_buffer = NULL;
   info.indexed = false;
   info.instance_count = 0;

   if (!radv_before_taskmesh_draw(cmd_buffer, &info, drawCount, false))
      return;

   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TASK)) {
      radv_emit_indirect_taskmesh_draw_packets(cmd_buffer, &info);
   } else {
      radv_emit_indirect_mesh_draw_packets(cmd_buffer, &info);
   }

   radv_after_draw(cmd_buffer, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer _buffer, VkDeviceSize offset,
                                      VkBuffer _countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                      uint32_t stride)
{

   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, _buffer);
   RADV_FROM_HANDLE(radv_buffer, count_buffer, _countBuffer);

   struct radv_draw_info info;

   info.indirect = buffer;
   info.indirect_offset = offset;
   info.stride = stride;
   info.count = maxDrawCount;
   info.strmout_buffer = NULL;
   info.count_buffer = count_buffer;
   info.count_buffer_offset = countBufferOffset;
   info.indexed = false;
   info.instance_count = 0;

   if (!radv_before_taskmesh_draw(cmd_buffer, &info, maxDrawCount, false))
      return;

   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TASK)) {
      radv_emit_indirect_taskmesh_draw_packets(cmd_buffer, &info);
   } else {
      radv_emit_indirect_mesh_draw_packets(cmd_buffer, &info);
   }

   radv_after_draw(cmd_buffer, false);
}

/* TODO: Use these functions with the normal dispatch path. */
static void radv_dgc_before_dispatch(struct radv_cmd_buffer *cmd_buffer);
static void radv_dgc_after_dispatch(struct radv_cmd_buffer *cmd_buffer);

VKAPI_ATTR void VKAPI_CALL
radv_CmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                   const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo)
{
   VK_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   VK_FROM_HANDLE(radv_indirect_command_layout, layout, pGeneratedCommandsInfo->indirectCommandsLayout);
   VK_FROM_HANDLE(radv_pipeline, pipeline, pGeneratedCommandsInfo->pipeline);
   VK_FROM_HANDLE(radv_buffer, prep_buffer, pGeneratedCommandsInfo->preprocessBuffer);
   const bool compute = layout->pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE;
   const bool use_predication = radv_use_dgc_predication(cmd_buffer, pGeneratedCommandsInfo);
   const struct radv_device *device = cmd_buffer->device;

   /* Secondary command buffers are needed for the full extension but can't use
    * PKT3_INDIRECT_BUFFER.
    */
   assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY);

   if (use_predication) {
      VK_FROM_HANDLE(radv_buffer, seq_count_buffer, pGeneratedCommandsInfo->sequencesCountBuffer);
      const uint64_t va = radv_buffer_get_va(seq_count_buffer->bo) + seq_count_buffer->offset +
                          pGeneratedCommandsInfo->sequencesCountOffset;

      radv_begin_conditional_rendering(cmd_buffer, va, true);
      cmd_buffer->state.predicating = true;
   }

   if (!radv_dgc_can_preprocess(layout, pipeline)) {
      radv_prepare_dgc(cmd_buffer, pGeneratedCommandsInfo);

      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_INV_VCACHE | RADV_CMD_FLAG_INV_L2;
   }

   if (compute) {
      radv_dgc_before_dispatch(cmd_buffer);
   } else {
      struct radv_graphics_pipeline *graphics_pipeline = radv_pipeline_to_graphics(pipeline);
      struct radv_draw_info info;

      info.count = pGeneratedCommandsInfo->sequencesCount;
      info.indirect = prep_buffer; /* We're not really going use it this way, but a good signal
                                   that this is not direct. */
      info.indirect_offset = 0;
      info.stride = 0;
      info.strmout_buffer = NULL;
      info.count_buffer = NULL;
      info.indexed = layout->indexed;
      info.instance_count = 0;

      if (radv_pipeline_has_stage(graphics_pipeline, MESA_SHADER_MESH)) {
         if (!radv_before_taskmesh_draw(cmd_buffer, &info, 1, true))
            return;
      } else {
         if (!radv_before_draw(cmd_buffer, &info, 1, true))
            return;
      }
   }

   uint32_t cmdbuf_size = radv_get_indirect_cmdbuf_size(pGeneratedCommandsInfo);
   struct radeon_winsys_bo *ib_bo = prep_buffer->bo;
   const uint64_t ib_offset = prep_buffer->offset + pGeneratedCommandsInfo->preprocessOffset;
   const uint32_t view_mask = cmd_buffer->state.render.view_mask;

   if (!radv_cmd_buffer_uses_mec(cmd_buffer)) {
      radeon_emit(cmd_buffer->cs, PKT3(PKT3_PFP_SYNC_ME, 0, cmd_buffer->state.predicating));
      radeon_emit(cmd_buffer->cs, 0);
   }

   if (compute || !view_mask) {
      device->ws->cs_execute_ib(cmd_buffer->cs, ib_bo, ib_offset, cmdbuf_size >> 2, cmd_buffer->state.predicating);
   } else {
      u_foreach_bit (view, view_mask) {
         radv_emit_view_index(cmd_buffer, view);

         device->ws->cs_execute_ib(cmd_buffer->cs, ib_bo, ib_offset, cmdbuf_size >> 2, cmd_buffer->state.predicating);
      }
   }

   if (compute) {
      cmd_buffer->push_constant_stages |= VK_SHADER_STAGE_COMPUTE_BIT;

      radv_dgc_after_dispatch(cmd_buffer);
   } else {
      struct radv_graphics_pipeline *graphics_pipeline = radv_pipeline_to_graphics(pipeline);

      if (layout->binds_index_buffer) {
         cmd_buffer->state.last_index_type = -1;
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_INDEX_BUFFER;
      }

      if (layout->bind_vbo_mask)
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_VERTEX_BUFFER;

      cmd_buffer->push_constant_stages |= graphics_pipeline->active_stages;

      if (!layout->indexed && cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7) {
         /* On GFX7 and later, non-indexed draws overwrite VGT_INDEX_TYPE, so the state must be
          * re-emitted before the next indexed draw.
          */
         cmd_buffer->state.last_index_type = -1;
      }

      cmd_buffer->state.last_num_instances = -1;
      cmd_buffer->state.last_vertex_offset_valid = false;
      cmd_buffer->state.last_first_instance = -1;
      cmd_buffer->state.last_drawid = -1;

      radv_after_draw(cmd_buffer, true);
   }

   if (use_predication) {
      cmd_buffer->state.predicating = false;
      radv_end_conditional_rendering(cmd_buffer);
   }
}

static void
radv_emit_dispatch_packets(struct radv_cmd_buffer *cmd_buffer, const struct radv_shader *compute_shader,
                           const struct radv_dispatch_info *info)
{
   unsigned dispatch_initiator = cmd_buffer->device->dispatch_initiator;
   struct radeon_winsys *ws = cmd_buffer->device->ws;
   bool predicating = cmd_buffer->state.predicating;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const struct radv_userdata_info *loc = radv_get_user_sgpr(compute_shader, AC_UD_CS_GRID_SIZE);

   radv_describe_dispatch(cmd_buffer, info);

   ASSERTED unsigned cdw_max = radeon_check_space(ws, cs, 30);

   if (compute_shader->info.wave_size == 32) {
      assert(cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10);
      dispatch_initiator |= S_00B800_CS_W32_EN(1);
   }

   if (info->ordered)
      dispatch_initiator &= ~S_00B800_ORDER_MODE(1);

   if (info->va) {
      if (info->indirect)
         radv_cs_add_buffer(ws, cs, info->indirect);

      if (info->unaligned) {
         radeon_set_sh_reg_seq(cs, R_00B81C_COMPUTE_NUM_THREAD_X, 3);
         radeon_emit(cs, S_00B81C_NUM_THREAD_FULL(compute_shader->info.cs.block_size[0]));
         radeon_emit(cs, S_00B81C_NUM_THREAD_FULL(compute_shader->info.cs.block_size[1]));
         radeon_emit(cs, S_00B81C_NUM_THREAD_FULL(compute_shader->info.cs.block_size[2]));

         dispatch_initiator |= S_00B800_USE_THREAD_DIMENSIONS(1);
      }

      if (loc->sgpr_idx != -1) {
         unsigned reg = R_00B900_COMPUTE_USER_DATA_0 + loc->sgpr_idx * 4;

         if (cmd_buffer->device->load_grid_size_from_user_sgpr) {
            assert(cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10_3);
            radeon_emit(cs, PKT3(PKT3_LOAD_SH_REG_INDEX, 3, 0));
            radeon_emit(cs, info->va);
            radeon_emit(cs, info->va >> 32);
            radeon_emit(cs, (reg - SI_SH_REG_OFFSET) >> 2);
            radeon_emit(cs, 3);
         } else {
            radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, reg, info->va, true);
         }
      }

      if (radv_cmd_buffer_uses_mec(cmd_buffer)) {
         uint64_t indirect_va = info->va;
         const bool needs_align32_workaround =
            cmd_buffer->device->physical_device->rad_info.has_async_compute_align32_bug &&
            cmd_buffer->qf == RADV_QUEUE_COMPUTE && !radv_is_aligned(indirect_va, 32);
         const unsigned ace_predication_size =
            4 /* DISPATCH_INDIRECT */ + (needs_align32_workaround ? 6 * 3 /* 3x COPY_DATA */ : 0);

         radv_cs_emit_compute_predication(cmd_buffer->device, &cmd_buffer->state, cs, cmd_buffer->mec_inv_pred_va,
                                          &cmd_buffer->mec_inv_pred_emitted, ace_predication_size);

         if (needs_align32_workaround) {
            const uint64_t unaligned_va = indirect_va;
            UNUSED void *ptr;
            uint32_t offset;

            if (!radv_cmd_buffer_upload_alloc_aligned(cmd_buffer, sizeof(VkDispatchIndirectCommand), 32, &offset, &ptr))
               return;

            indirect_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + offset;

            for (uint32_t i = 0; i < 3; i++) {
               const uint64_t src_va = unaligned_va + i * 4;
               const uint64_t dst_va = indirect_va + i * 4;

               radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
               radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_SRC_MEM) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) |
                                  COPY_DATA_WR_CONFIRM);
               radeon_emit(cs, src_va);
               radeon_emit(cs, src_va >> 32);
               radeon_emit(cs, dst_va);
               radeon_emit(cs, dst_va >> 32);
            }
         }

         radeon_emit(cs, PKT3(PKT3_DISPATCH_INDIRECT, 2, 0) | PKT3_SHADER_TYPE_S(1));
         radeon_emit(cs, indirect_va);
         radeon_emit(cs, indirect_va >> 32);
         radeon_emit(cs, dispatch_initiator);
      } else {
         radeon_emit(cs, PKT3(PKT3_SET_BASE, 2, 0) | PKT3_SHADER_TYPE_S(1));
         radeon_emit(cs, 1);
         radeon_emit(cs, info->va);
         radeon_emit(cs, info->va >> 32);

         if (cmd_buffer->qf == RADV_QUEUE_COMPUTE) {
            radv_cs_emit_compute_predication(cmd_buffer->device, &cmd_buffer->state, cs,
                                             cmd_buffer->mec_inv_pred_va, &cmd_buffer->mec_inv_pred_emitted,
                                             3 /* PKT3_DISPATCH_INDIRECT */);
            predicating = false;
         }

         radeon_emit(cs, PKT3(PKT3_DISPATCH_INDIRECT, 1, predicating) | PKT3_SHADER_TYPE_S(1));
         radeon_emit(cs, 0);
         radeon_emit(cs, dispatch_initiator);
      }
   } else {
      const unsigned *cs_block_size = compute_shader->info.cs.block_size;
      unsigned blocks[3] = {info->blocks[0], info->blocks[1], info->blocks[2]};
      unsigned offsets[3] = {info->offsets[0], info->offsets[1], info->offsets[2]};

      if (info->unaligned) {
         unsigned remainder[3];

         /* If aligned, these should be an entire block size,
          * not 0.
          */
         remainder[0] = blocks[0] + cs_block_size[0] - align_u32_npot(blocks[0], cs_block_size[0]);
         remainder[1] = blocks[1] + cs_block_size[1] - align_u32_npot(blocks[1], cs_block_size[1]);
         remainder[2] = blocks[2] + cs_block_size[2] - align_u32_npot(blocks[2], cs_block_size[2]);

         blocks[0] = DIV_ROUND_UP(blocks[0], cs_block_size[0]);
         blocks[1] = DIV_ROUND_UP(blocks[1], cs_block_size[1]);
         blocks[2] = DIV_ROUND_UP(blocks[2], cs_block_size[2]);

         for (unsigned i = 0; i < 3; ++i) {
            assert(offsets[i] % cs_block_size[i] == 0);
            offsets[i] /= cs_block_size[i];
         }

         radeon_set_sh_reg_seq(cs, R_00B81C_COMPUTE_NUM_THREAD_X, 3);
         radeon_emit(cs, S_00B81C_NUM_THREAD_FULL(cs_block_size[0]) | S_00B81C_NUM_THREAD_PARTIAL(remainder[0]));
         radeon_emit(cs, S_00B81C_NUM_THREAD_FULL(cs_block_size[1]) | S_00B81C_NUM_THREAD_PARTIAL(remainder[1]));
         radeon_emit(cs, S_00B81C_NUM_THREAD_FULL(cs_block_size[2]) | S_00B81C_NUM_THREAD_PARTIAL(remainder[2]));

         dispatch_initiator |= S_00B800_PARTIAL_TG_EN(1);
      }

      if (loc->sgpr_idx != -1) {
         if (cmd_buffer->device->load_grid_size_from_user_sgpr) {
            assert(loc->num_sgprs == 3);

            radeon_set_sh_reg_seq(cs, R_00B900_COMPUTE_USER_DATA_0 + loc->sgpr_idx * 4, 3);
            radeon_emit(cs, blocks[0]);
            radeon_emit(cs, blocks[1]);
            radeon_emit(cs, blocks[2]);
         } else {
            uint32_t offset;
            if (!radv_cmd_buffer_upload_data(cmd_buffer, 12, blocks, &offset))
               return;

            uint64_t va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + offset;
            radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs,
                                     R_00B900_COMPUTE_USER_DATA_0 + loc->sgpr_idx * 4, va, true);
         }
      }

      if (offsets[0] || offsets[1] || offsets[2]) {
         radeon_set_sh_reg_seq(cs, R_00B810_COMPUTE_START_X, 3);
         radeon_emit(cs, offsets[0]);
         radeon_emit(cs, offsets[1]);
         radeon_emit(cs, offsets[2]);

         /* The blocks in the packet are not counts but end values. */
         for (unsigned i = 0; i < 3; ++i)
            blocks[i] += offsets[i];
      } else {
         dispatch_initiator |= S_00B800_FORCE_START_AT_000(1);
      }

      if (cmd_buffer->qf == RADV_QUEUE_COMPUTE) {
         radv_cs_emit_compute_predication(cmd_buffer->device, &cmd_buffer->state, cs, cmd_buffer->mec_inv_pred_va,
                                          &cmd_buffer->mec_inv_pred_emitted, 5 /* DISPATCH_DIRECT size */);
         predicating = false;
      }

      if (cmd_buffer->device->physical_device->rad_info.has_async_compute_threadgroup_bug &&
          cmd_buffer->qf == RADV_QUEUE_COMPUTE) {
         for (unsigned i = 0; i < 3; i++) {
            if (info->unaligned) {
               /* info->blocks is already in thread dimensions for unaligned dispatches. */
               blocks[i] = info->blocks[i];
            } else {
               /* Force the async compute dispatch to be in "thread" dim mode to workaround a hw bug. */
               blocks[i] *= cs_block_size[i];
            }

            dispatch_initiator |= S_00B800_USE_THREAD_DIMENSIONS(1);
         }
      }

      radeon_emit(cs, PKT3(PKT3_DISPATCH_DIRECT, 3, predicating) | PKT3_SHADER_TYPE_S(1));
      radeon_emit(cs, blocks[0]);
      radeon_emit(cs, blocks[1]);
      radeon_emit(cs, blocks[2]);
      radeon_emit(cs, dispatch_initiator);
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);
}

static void
radv_upload_compute_shader_descriptors(struct radv_cmd_buffer *cmd_buffer, VkPipelineBindPoint bind_point)
{
   radv_flush_descriptors(cmd_buffer, VK_SHADER_STAGE_COMPUTE_BIT, bind_point);
   const VkShaderStageFlags stages =
      bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR ? RADV_RT_STAGE_BITS : VK_SHADER_STAGE_COMPUTE_BIT;
   const VkShaderStageFlags pc_stages = radv_must_flush_constants(cmd_buffer, stages, bind_point);
   if (pc_stages)
      radv_flush_constants(cmd_buffer, pc_stages, bind_point);
}

static void
radv_emit_rt_stack_size(struct radv_cmd_buffer *cmd_buffer)
{
   unsigned rsrc2 = cmd_buffer->state.rt_prolog->config.rsrc2;
   if (cmd_buffer->state.rt_stack_size)
      rsrc2 |= S_00B12C_SCRATCH_EN(1);

   radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 3);
   radeon_set_sh_reg(cmd_buffer->cs, R_00B84C_COMPUTE_PGM_RSRC2, rsrc2);
}

static void
radv_dispatch(struct radv_cmd_buffer *cmd_buffer, const struct radv_dispatch_info *info,
              struct radv_compute_pipeline *pipeline, struct radv_shader *compute_shader,
              VkPipelineBindPoint bind_point)
{
   bool has_prefetch = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7;
   bool pipeline_is_dirty = pipeline != cmd_buffer->state.emitted_compute_pipeline;

   if (compute_shader->info.cs.regalloc_hang_bug)
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_CS_PARTIAL_FLUSH;

   if (cmd_buffer->state.flush_bits & (RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB |
                                       RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_CS_PARTIAL_FLUSH)) {
      /* If we have to wait for idle, set all states first, so that
       * all SET packets are processed in parallel with previous draw
       * calls. Then upload descriptors, set shader pointers, and
       * dispatch, and prefetch at the end. This ensures that the
       * time the CUs are idle is very short. (there are only SET_SH
       * packets between the wait and the draw)
       */
      radv_emit_compute_pipeline(cmd_buffer, pipeline);
      if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
         radv_emit_rt_stack_size(cmd_buffer);
      radv_emit_cache_flush(cmd_buffer);
      /* <-- CUs are idle here --> */

      radv_upload_compute_shader_descriptors(cmd_buffer, bind_point);

      radv_emit_dispatch_packets(cmd_buffer, compute_shader, info);
      /* <-- CUs are busy here --> */

      /* Start prefetches after the dispatch has been started. Both
       * will run in parallel, but starting the dispatch first is
       * more important.
       */
      if (has_prefetch && pipeline_is_dirty) {
         radv_emit_shader_prefetch(cmd_buffer, compute_shader);
      }
   } else {
      /* If we don't wait for idle, start prefetches first, then set
       * states, and dispatch at the end.
       */
      radv_emit_cache_flush(cmd_buffer);

      if (has_prefetch && pipeline_is_dirty) {
         radv_emit_shader_prefetch(cmd_buffer, compute_shader);
      }

      radv_upload_compute_shader_descriptors(cmd_buffer, bind_point);

      radv_emit_compute_pipeline(cmd_buffer, pipeline);
      if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
         radv_emit_rt_stack_size(cmd_buffer);
      radv_emit_dispatch_packets(cmd_buffer, compute_shader, info);
   }

   if (pipeline_is_dirty) {
      /* Raytracing uses compute shaders but has separate bind points and pipelines.
       * So if we set compute userdata & shader registers we should dirty the raytracing
       * ones and the other way around.
       *
       * We only need to do this when the pipeline is dirty because when we switch between
       * the two we always need to switch pipelines.
       */
      radv_mark_descriptor_sets_dirty(cmd_buffer, bind_point == VK_PIPELINE_BIND_POINT_COMPUTE
                                                     ? VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
                                                     : VK_PIPELINE_BIND_POINT_COMPUTE);
   }

   if (compute_shader->info.cs.regalloc_hang_bug)
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_CS_PARTIAL_FLUSH;

   radv_cmd_buffer_after_draw(cmd_buffer, RADV_CMD_FLAG_CS_PARTIAL_FLUSH, false);
}

static void
radv_dgc_before_dispatch(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_compute_pipeline *pipeline = cmd_buffer->state.compute_pipeline;
   struct radv_shader *compute_shader = cmd_buffer->state.shaders[MESA_SHADER_COMPUTE];
   bool pipeline_is_dirty = pipeline != cmd_buffer->state.emitted_compute_pipeline;

   /* We will have run the DGC patch shaders before, so we can assume that there is something to
    * flush. Otherwise, we just split radv_dispatch in two. One pre-dispatch and another one
    * post-dispatch. */

   if (compute_shader->info.cs.regalloc_hang_bug)
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_CS_PARTIAL_FLUSH;

   radv_emit_compute_pipeline(cmd_buffer, pipeline);
   radv_emit_cache_flush(cmd_buffer);

   radv_upload_compute_shader_descriptors(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE);

   if (pipeline_is_dirty) {
      const bool has_prefetch = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7;

      if (has_prefetch)
         radv_emit_shader_prefetch(cmd_buffer, compute_shader);

      /* Raytracing uses compute shaders but has separate bind points and pipelines.
       * So if we set compute userdata & shader registers we should dirty the raytracing
       * ones and the other way around.
       *
       * We only need to do this when the pipeline is dirty because when we switch between
       * the two we always need to switch pipelines.
       */
      radv_mark_descriptor_sets_dirty(cmd_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
   }
}

static void
radv_dgc_after_dispatch(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_shader *compute_shader = cmd_buffer->state.shaders[MESA_SHADER_COMPUTE];

   if (compute_shader->info.cs.regalloc_hang_bug)
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_CS_PARTIAL_FLUSH;

   radv_cmd_buffer_after_draw(cmd_buffer, RADV_CMD_FLAG_CS_PARTIAL_FLUSH, true);
}

void
radv_compute_dispatch(struct radv_cmd_buffer *cmd_buffer, const struct radv_dispatch_info *info)
{
   radv_dispatch(cmd_buffer, info, cmd_buffer->state.compute_pipeline, cmd_buffer->state.shaders[MESA_SHADER_COMPUTE],
                 VK_PIPELINE_BIND_POINT_COMPUTE);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t base_x, uint32_t base_y, uint32_t base_z, uint32_t x,
                     uint32_t y, uint32_t z)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_dispatch_info info = {0};

   info.blocks[0] = x;
   info.blocks[1] = y;
   info.blocks[2] = z;

   info.offsets[0] = base_x;
   info.offsets[1] = base_y;
   info.offsets[2] = base_z;
   radv_compute_dispatch(cmd_buffer, &info);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer _buffer, VkDeviceSize offset)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, _buffer);
   struct radv_dispatch_info info = {0};

   info.indirect = buffer->bo;
   info.va = radv_buffer_get_va(buffer->bo) + buffer->offset + offset;

   radv_compute_dispatch(cmd_buffer, &info);
}

void
radv_unaligned_dispatch(struct radv_cmd_buffer *cmd_buffer, uint32_t x, uint32_t y, uint32_t z)
{
   struct radv_dispatch_info info = {0};

   info.blocks[0] = x;
   info.blocks[1] = y;
   info.blocks[2] = z;
   info.unaligned = 1;

   radv_compute_dispatch(cmd_buffer, &info);
}

void
radv_indirect_dispatch(struct radv_cmd_buffer *cmd_buffer, struct radeon_winsys_bo *bo, uint64_t va)
{
   struct radv_dispatch_info info = {0};

   info.indirect = bo;
   info.va = va;

   radv_compute_dispatch(cmd_buffer, &info);
}

enum radv_rt_mode {
   radv_rt_mode_direct,
   radv_rt_mode_indirect,
   radv_rt_mode_indirect2,
};

static void
radv_upload_trace_rays_params(struct radv_cmd_buffer *cmd_buffer, VkTraceRaysIndirectCommand2KHR *tables,
                              enum radv_rt_mode mode, uint64_t *launch_size_va, uint64_t *sbt_va)
{
   uint32_t upload_size = mode == radv_rt_mode_direct ? sizeof(VkTraceRaysIndirectCommand2KHR)
                                                      : offsetof(VkTraceRaysIndirectCommand2KHR, width);

   uint32_t offset;
   if (!radv_cmd_buffer_upload_data(cmd_buffer, upload_size, tables, &offset))
      return;

   uint64_t upload_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + offset;

   if (mode == radv_rt_mode_direct)
      *launch_size_va = upload_va + offsetof(VkTraceRaysIndirectCommand2KHR, width);
   if (sbt_va)
      *sbt_va = upload_va;
}

static void
radv_trace_rays(struct radv_cmd_buffer *cmd_buffer, VkTraceRaysIndirectCommand2KHR *tables, uint64_t indirect_va,
                enum radv_rt_mode mode)
{
   if (cmd_buffer->device->instance->debug_flags & RADV_DEBUG_NO_RT)
      return;

   struct radv_compute_pipeline *pipeline = &cmd_buffer->state.rt_pipeline->base;
   struct radv_shader *rt_prolog = cmd_buffer->state.rt_prolog;
   uint32_t base_reg = rt_prolog->info.user_data_0;

   /* Reserve scratch for stacks manually since it is not handled by the compute path. */
   uint32_t scratch_bytes_per_wave = rt_prolog->config.scratch_bytes_per_wave;
   uint32_t wave_size = rt_prolog->info.wave_size;

   /* The hardware register is specified as a multiple of 64 or 256 DWORDS. */
   unsigned scratch_alloc_granule = cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11 ? 256 : 1024;
   scratch_bytes_per_wave += align(cmd_buffer->state.rt_stack_size * wave_size, scratch_alloc_granule);

   cmd_buffer->compute_scratch_size_per_wave_needed =
      MAX2(cmd_buffer->compute_scratch_size_per_wave_needed, scratch_bytes_per_wave);

   /* Since the workgroup size is 8x4 (or 8x8), 1D dispatches can only fill 8 threads per wave at most. To increase
    * occupancy, it's beneficial to convert to a 2D dispatch in these cases. */
   if (tables && tables->height == 1 && tables->width >= cmd_buffer->state.rt_prolog->info.cs.block_size[0])
      tables->height = ACO_RT_CONVERTED_2D_LAUNCH_SIZE;

   struct radv_dispatch_info info = {0};
   info.unaligned = true;

   uint64_t launch_size_va = 0;
   uint64_t sbt_va = 0;

   if (mode != radv_rt_mode_indirect2) {
      launch_size_va = indirect_va;
      radv_upload_trace_rays_params(cmd_buffer, tables, mode, &launch_size_va, &sbt_va);
   } else {
      launch_size_va = indirect_va + offsetof(VkTraceRaysIndirectCommand2KHR, width);
      sbt_va = indirect_va;
   }

   uint32_t remaining_ray_count = 0;

   if (mode == radv_rt_mode_direct) {
      info.blocks[0] = tables->width;
      info.blocks[1] = tables->height;
      info.blocks[2] = tables->depth;

      if (tables->height == ACO_RT_CONVERTED_2D_LAUNCH_SIZE) {
         /* We need the ray count for the 2D dispatch to be a multiple of the y block size for the division to work, and
          * a multiple of the x block size because the invocation offset must be a multiple of the block size when
          * dispatching the remaining rays. Fortunately, the x block size is itself a multiple of the y block size, so
          * we only need to ensure that the ray count is a multiple of the x block size. */
         remaining_ray_count = tables->width % rt_prolog->info.cs.block_size[0];

         uint32_t ray_count = tables->width - remaining_ray_count;
         info.blocks[0] = ray_count / rt_prolog->info.cs.block_size[1];
         info.blocks[1] = rt_prolog->info.cs.block_size[1];
      }
   } else
      info.va = launch_size_va;

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 15);

   const struct radv_userdata_info *desc_loc = radv_get_user_sgpr(rt_prolog, AC_UD_CS_SBT_DESCRIPTORS);
   if (desc_loc->sgpr_idx != -1) {
      radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + desc_loc->sgpr_idx * 4, sbt_va, true);
   }

   const struct radv_userdata_info *size_loc = radv_get_user_sgpr(rt_prolog, AC_UD_CS_RAY_LAUNCH_SIZE_ADDR);
   if (size_loc->sgpr_idx != -1) {
      radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + size_loc->sgpr_idx * 4, launch_size_va,
                               true);
   }

   const struct radv_userdata_info *base_loc = radv_get_user_sgpr(rt_prolog, AC_UD_CS_RAY_DYNAMIC_CALLABLE_STACK_BASE);
   if (base_loc->sgpr_idx != -1) {
      const struct radv_shader_info *cs_info = &rt_prolog->info;
      radeon_set_sh_reg(cmd_buffer->cs, R_00B900_COMPUTE_USER_DATA_0 + base_loc->sgpr_idx * 4,
                        rt_prolog->config.scratch_bytes_per_wave / cs_info->wave_size);
   }

   const struct radv_userdata_info *shader_loc = radv_get_user_sgpr(rt_prolog, AC_UD_CS_TRAVERSAL_SHADER_ADDR);
   struct radv_shader *traversal_shader = cmd_buffer->state.shaders[MESA_SHADER_INTERSECTION];
   if (shader_loc->sgpr_idx != -1 && traversal_shader) {
      uint64_t traversal_va = traversal_shader->va | radv_rt_priority_traversal;
      radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + shader_loc->sgpr_idx * 4, traversal_va,
                               true);
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);

   radv_dispatch(cmd_buffer, &info, pipeline, rt_prolog, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

   if (remaining_ray_count) {
      info.blocks[0] = remaining_ray_count;
      info.blocks[1] = 1;
      info.offsets[0] = tables->width - remaining_ray_count;

      /* Reset the ray launch size so the prolog doesn't think this is a converted dispatch */
      tables->height = 1;
      radv_upload_trace_rays_params(cmd_buffer, tables, mode, &launch_size_va, NULL);
      if (size_loc->sgpr_idx != -1) {
         radv_emit_shader_pointer(cmd_buffer->device, cmd_buffer->cs, base_reg + size_loc->sgpr_idx * 4, launch_size_va,
                                  true);
      }

      radv_dispatch(cmd_buffer, &info, pipeline, rt_prolog, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                     const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                     const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                     const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                     uint32_t height, uint32_t depth)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   VkTraceRaysIndirectCommand2KHR tables = {
      .raygenShaderRecordAddress = pRaygenShaderBindingTable->deviceAddress,
      .raygenShaderRecordSize = pRaygenShaderBindingTable->size,
      .missShaderBindingTableAddress = pMissShaderBindingTable->deviceAddress,
      .missShaderBindingTableSize = pMissShaderBindingTable->size,
      .missShaderBindingTableStride = pMissShaderBindingTable->stride,
      .hitShaderBindingTableAddress = pHitShaderBindingTable->deviceAddress,
      .hitShaderBindingTableSize = pHitShaderBindingTable->size,
      .hitShaderBindingTableStride = pHitShaderBindingTable->stride,
      .callableShaderBindingTableAddress = pCallableShaderBindingTable->deviceAddress,
      .callableShaderBindingTableSize = pCallableShaderBindingTable->size,
      .callableShaderBindingTableStride = pCallableShaderBindingTable->stride,
      .width = width,
      .height = height,
      .depth = depth,
   };

   radv_trace_rays(cmd_buffer, &tables, 0, radv_rt_mode_direct);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                             const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                             const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                             VkDeviceAddress indirectDeviceAddress)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   assert(cmd_buffer->device->use_global_bo_list);

   VkTraceRaysIndirectCommand2KHR tables = {
      .raygenShaderRecordAddress = pRaygenShaderBindingTable->deviceAddress,
      .raygenShaderRecordSize = pRaygenShaderBindingTable->size,
      .missShaderBindingTableAddress = pMissShaderBindingTable->deviceAddress,
      .missShaderBindingTableSize = pMissShaderBindingTable->size,
      .missShaderBindingTableStride = pMissShaderBindingTable->stride,
      .hitShaderBindingTableAddress = pHitShaderBindingTable->deviceAddress,
      .hitShaderBindingTableSize = pHitShaderBindingTable->size,
      .hitShaderBindingTableStride = pHitShaderBindingTable->stride,
      .callableShaderBindingTableAddress = pCallableShaderBindingTable->deviceAddress,
      .callableShaderBindingTableSize = pCallableShaderBindingTable->size,
      .callableShaderBindingTableStride = pCallableShaderBindingTable->stride,
   };

   radv_trace_rays(cmd_buffer, &tables, indirectDeviceAddress, radv_rt_mode_indirect);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   assert(cmd_buffer->device->use_global_bo_list);

   radv_trace_rays(cmd_buffer, NULL, indirectDeviceAddress, radv_rt_mode_indirect2);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t size)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   cmd_buffer->state.rt_stack_size = size;
}

/*
 * For HTILE we have the following interesting clear words:
 *   0xfffff30f: Uncompressed, full depth range, for depth+stencil HTILE
 *   0xfffc000f: Uncompressed, full depth range, for depth only HTILE.
 *   0xfffffff0: Clear depth to 1.0
 *   0x00000000: Clear depth to 0.0
 */
static void
radv_initialize_htile(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                      const VkImageSubresourceRange *range)
{
   struct radv_cmd_state *state = &cmd_buffer->state;
   uint32_t htile_value = radv_get_htile_initial_value(cmd_buffer->device, image);
   VkClearDepthStencilValue value = {0};
   struct radv_barrier_data barrier = {0};

   barrier.layout_transitions.init_mask_ram = 1;
   radv_describe_layout_transition(cmd_buffer, &barrier);

   /* Transitioning from LAYOUT_UNDEFINED layout not everyone is consistent
    * in considering previous rendering work for WAW hazards. */
   state->flush_bits |= radv_src_access_flush(cmd_buffer, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, image);

   if (image->planes[0].surface.has_stencil &&
       !(range->aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
      /* Flush caches before performing a separate aspect initialization because it's a
       * read-modify-write operation.
       */
      state->flush_bits |= radv_dst_access_flush(cmd_buffer, VK_ACCESS_2_SHADER_READ_BIT, image);
   }

   state->flush_bits |= radv_clear_htile(cmd_buffer, image, range, htile_value);

   radv_set_ds_clear_metadata(cmd_buffer, image, range, value, range->aspectMask);

   if (radv_image_is_tc_compat_htile(image) && (range->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)) {
      /* Initialize the TC-compat metada value to 0 because by
       * default DB_Z_INFO.RANGE_PRECISION is set to 1, and we only
       * need have to conditionally update its value when performing
       * a fast depth clear.
       */
      radv_set_tc_compat_zrange_metadata(cmd_buffer, image, range, 0);
   }
}

static void
radv_handle_depth_image_transition(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                                   VkImageLayout src_layout, VkImageLayout dst_layout, unsigned src_queue_mask,
                                   unsigned dst_queue_mask, const VkImageSubresourceRange *range,
                                   struct radv_sample_locations_state *sample_locs)
{
   struct radv_device *device = cmd_buffer->device;

   if (!radv_htile_enabled(image, range->baseMipLevel))
      return;

   if (src_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
      radv_initialize_htile(cmd_buffer, image, range);
   } else if (!radv_layout_is_htile_compressed(device, image, src_layout, src_queue_mask) &&
              radv_layout_is_htile_compressed(device, image, dst_layout, dst_queue_mask)) {
      radv_initialize_htile(cmd_buffer, image, range);
   } else if (radv_layout_is_htile_compressed(device, image, src_layout, src_queue_mask) &&
              !radv_layout_is_htile_compressed(device, image, dst_layout, dst_queue_mask)) {
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB | RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;

      radv_expand_depth_stencil(cmd_buffer, image, range, sample_locs);

      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_DB | RADV_CMD_FLAG_FLUSH_AND_INV_DB_META;
   }
}

static uint32_t
radv_init_cmask(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, const VkImageSubresourceRange *range,
                uint32_t value)
{
   struct radv_barrier_data barrier = {0};

   barrier.layout_transitions.init_mask_ram = 1;
   radv_describe_layout_transition(cmd_buffer, &barrier);

   return radv_clear_cmask(cmd_buffer, image, range, value);
}

uint32_t
radv_init_fmask(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, const VkImageSubresourceRange *range)
{
   static const uint32_t fmask_clear_values[4] = {0x00000000, 0x02020202, 0xE4E4E4E4, 0x76543210};
   uint32_t log2_samples = util_logbase2(image->vk.samples);
   uint32_t value = fmask_clear_values[log2_samples];
   struct radv_barrier_data barrier = {0};

   barrier.layout_transitions.init_mask_ram = 1;
   radv_describe_layout_transition(cmd_buffer, &barrier);

   return radv_clear_fmask(cmd_buffer, image, range, value);
}

uint32_t
radv_init_dcc(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, const VkImageSubresourceRange *range,
              uint32_t value)
{
   struct radv_barrier_data barrier = {0};
   uint32_t flush_bits = 0;
   unsigned size = 0;

   barrier.layout_transitions.init_mask_ram = 1;
   radv_describe_layout_transition(cmd_buffer, &barrier);

   flush_bits |= radv_clear_dcc(cmd_buffer, image, range, value);

   if (cmd_buffer->device->physical_device->rad_info.gfx_level == GFX8) {
      /* When DCC is enabled with mipmaps, some levels might not
       * support fast clears and we have to initialize them as "fully
       * expanded".
       */
      /* Compute the size of all fast clearable DCC levels. */
      for (unsigned i = 0; i < image->planes[0].surface.num_meta_levels; i++) {
         struct legacy_surf_dcc_level *dcc_level = &image->planes[0].surface.u.legacy.color.dcc_level[i];
         unsigned dcc_fast_clear_size = dcc_level->dcc_slice_fast_clear_size * image->vk.array_layers;

         if (!dcc_fast_clear_size)
            break;

         size = dcc_level->dcc_offset + dcc_fast_clear_size;
      }

      /* Initialize the mipmap levels without DCC. */
      if (size != image->planes[0].surface.meta_size) {
         flush_bits |= radv_fill_buffer(cmd_buffer, image, image->bindings[0].bo,
                                        radv_buffer_get_va(image->bindings[0].bo) + image->bindings[0].offset +
                                           image->planes[0].surface.meta_offset + size,
                                        image->planes[0].surface.meta_size - size, 0xffffffff);
      }
   }

   return flush_bits;
}

/**
 * Initialize DCC/FMASK/CMASK metadata for a color image.
 */
static void
radv_init_color_image_metadata(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, VkImageLayout src_layout,
                               VkImageLayout dst_layout, unsigned src_queue_mask, unsigned dst_queue_mask,
                               const VkImageSubresourceRange *range)
{
   uint32_t flush_bits = 0;

   /* Transitioning from LAYOUT_UNDEFINED layout not everyone is
    * consistent in considering previous rendering work for WAW hazards.
    */
   cmd_buffer->state.flush_bits |= radv_src_access_flush(cmd_buffer, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, image);

   if (radv_image_has_cmask(image)) {
      static const uint32_t cmask_clear_values[4] = {0xffffffff, 0xdddddddd, 0xeeeeeeee, 0xffffffff};
      uint32_t log2_samples = util_logbase2(image->vk.samples);

      flush_bits |= radv_init_cmask(cmd_buffer, image, range, cmask_clear_values[log2_samples]);
   }

   if (radv_image_has_fmask(image)) {
      flush_bits |= radv_init_fmask(cmd_buffer, image, range);
   }

   if (radv_dcc_enabled(image, range->baseMipLevel)) {
      uint32_t value = 0xffffffffu; /* Fully expanded mode. */

      if (radv_layout_dcc_compressed(cmd_buffer->device, image, range->baseMipLevel, dst_layout, dst_queue_mask)) {
         value = 0u;
      }

      flush_bits |= radv_init_dcc(cmd_buffer, image, range, value);
   }

   if (radv_image_has_cmask(image) || radv_dcc_enabled(image, range->baseMipLevel)) {
      radv_update_fce_metadata(cmd_buffer, image, range, false);

      uint32_t color_values[2] = {0};
      radv_set_color_clear_metadata(cmd_buffer, image, range, color_values);
   }

   cmd_buffer->state.flush_bits |= flush_bits;
}

static void
radv_retile_transition(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, VkImageLayout src_layout,
                       VkImageLayout dst_layout, unsigned dst_queue_mask)
{
   /* If the image is read-only, we don't have to retile DCC because it can't change. */
   if (!(image->vk.usage & RADV_IMAGE_USAGE_WRITE_BITS))
      return;

   if (src_layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
       (dst_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR || (dst_queue_mask & (1u << RADV_QUEUE_FOREIGN))))
      radv_retile_dcc(cmd_buffer, image);
}

static bool
radv_image_need_retile(const struct radv_cmd_buffer *cmd_buffer, const struct radv_image *image)
{
   return cmd_buffer->qf != RADV_QUEUE_TRANSFER && image->planes[0].surface.display_dcc_offset &&
          image->planes[0].surface.display_dcc_offset != image->planes[0].surface.meta_offset;
}

/**
 * Handle color image transitions for DCC/FMASK/CMASK.
 */
static void
radv_handle_color_image_transition(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                                   VkImageLayout src_layout, VkImageLayout dst_layout, unsigned src_queue_mask,
                                   unsigned dst_queue_mask, const VkImageSubresourceRange *range)
{
   bool dcc_decompressed = false, fast_clear_flushed = false;

   if (!radv_image_has_cmask(image) && !radv_image_has_fmask(image) && !radv_dcc_enabled(image, range->baseMipLevel))
      return;

   if (src_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
      radv_init_color_image_metadata(cmd_buffer, image, src_layout, dst_layout, src_queue_mask, dst_queue_mask, range);

      if (radv_image_need_retile(cmd_buffer, image))
         radv_retile_transition(cmd_buffer, image, src_layout, dst_layout, dst_queue_mask);
      return;
   }

   if (radv_dcc_enabled(image, range->baseMipLevel)) {
      if (src_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
         cmd_buffer->state.flush_bits |= radv_init_dcc(cmd_buffer, image, range, 0xffffffffu);
      } else if (radv_layout_dcc_compressed(cmd_buffer->device, image, range->baseMipLevel, src_layout,
                                            src_queue_mask) &&
                 !radv_layout_dcc_compressed(cmd_buffer->device, image, range->baseMipLevel, dst_layout,
                                             dst_queue_mask)) {
         radv_decompress_dcc(cmd_buffer, image, range);
         dcc_decompressed = true;
      } else if (radv_layout_can_fast_clear(cmd_buffer->device, image, range->baseMipLevel, src_layout,
                                            src_queue_mask) &&
                 !radv_layout_can_fast_clear(cmd_buffer->device, image, range->baseMipLevel, dst_layout,
                                             dst_queue_mask)) {
         radv_fast_clear_flush_image_inplace(cmd_buffer, image, range);
         fast_clear_flushed = true;
      }

      if (radv_image_need_retile(cmd_buffer, image))
         radv_retile_transition(cmd_buffer, image, src_layout, dst_layout, dst_queue_mask);
   } else if (radv_image_has_cmask(image) || radv_image_has_fmask(image)) {
      if (radv_layout_can_fast_clear(cmd_buffer->device, image, range->baseMipLevel, src_layout, src_queue_mask) &&
          !radv_layout_can_fast_clear(cmd_buffer->device, image, range->baseMipLevel, dst_layout, dst_queue_mask)) {
         radv_fast_clear_flush_image_inplace(cmd_buffer, image, range);
         fast_clear_flushed = true;
      }
   }

   /* MSAA color decompress. */
   const enum radv_fmask_compression src_fmask_comp =
      radv_layout_fmask_compression(cmd_buffer->device, image, src_layout, src_queue_mask);
   const enum radv_fmask_compression dst_fmask_comp =
      radv_layout_fmask_compression(cmd_buffer->device, image, dst_layout, dst_queue_mask);
   if (src_fmask_comp <= dst_fmask_comp)
      return;

   if (src_fmask_comp == RADV_FMASK_COMPRESSION_FULL) {
      if (radv_dcc_enabled(image, range->baseMipLevel) && !radv_image_use_dcc_image_stores(cmd_buffer->device, image) &&
          !dcc_decompressed) {
         /* A DCC decompress is required before expanding FMASK
          * when DCC stores aren't supported to avoid being in
          * a state where DCC is compressed and the main
          * surface is uncompressed.
          */
         radv_decompress_dcc(cmd_buffer, image, range);
      } else if (!fast_clear_flushed) {
         /* A FMASK decompress is required before expanding
          * FMASK.
          */
         radv_fast_clear_flush_image_inplace(cmd_buffer, image, range);
      }
   }

   if (dst_fmask_comp == RADV_FMASK_COMPRESSION_NONE) {
      struct radv_barrier_data barrier = {0};
      barrier.layout_transitions.fmask_color_expand = 1;
      radv_describe_layout_transition(cmd_buffer, &barrier);

      radv_expand_fmask_image_inplace(cmd_buffer, image, range);
   }
}

static void
radv_handle_image_transition(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, VkImageLayout src_layout,
                             VkImageLayout dst_layout, uint32_t src_family_index, uint32_t dst_family_index,
                             const VkImageSubresourceRange *range, struct radv_sample_locations_state *sample_locs)
{
   enum radv_queue_family src_qf = vk_queue_to_radv(cmd_buffer->device->physical_device, src_family_index);
   enum radv_queue_family dst_qf = vk_queue_to_radv(cmd_buffer->device->physical_device, dst_family_index);
   if (image->exclusive && src_family_index != dst_family_index) {
      /* This is an acquire or a release operation and there will be
       * a corresponding release/acquire. Do the transition in the
       * most flexible queue. */

      assert(src_qf == cmd_buffer->qf || dst_qf == cmd_buffer->qf);

      if (src_family_index == VK_QUEUE_FAMILY_EXTERNAL || src_family_index == VK_QUEUE_FAMILY_FOREIGN_EXT)
         return;

      if (cmd_buffer->qf == RADV_QUEUE_TRANSFER)
         return;

      if (cmd_buffer->qf == RADV_QUEUE_COMPUTE && (src_qf == RADV_QUEUE_GENERAL || dst_qf == RADV_QUEUE_GENERAL))
         return;
   }

   unsigned src_queue_mask = radv_image_queue_family_mask(image, src_qf, cmd_buffer->qf);
   unsigned dst_queue_mask = radv_image_queue_family_mask(image, dst_qf, cmd_buffer->qf);

   if (src_layout == dst_layout && src_queue_mask == dst_queue_mask)
      return;

   if (image->vk.aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
      radv_handle_depth_image_transition(cmd_buffer, image, src_layout, dst_layout, src_queue_mask, dst_queue_mask,
                                         range, sample_locs);
   } else {
      radv_handle_color_image_transition(cmd_buffer, image, src_layout, dst_layout, src_queue_mask, dst_queue_mask,
                                         range);
   }
}

static void
radv_cp_dma_wait_for_stages(struct radv_cmd_buffer *cmd_buffer, VkPipelineStageFlags2 stage_mask)
{
   /* Make sure CP DMA is idle because the driver might have performed a DMA operation for copying a
    * buffer (or a MSAA image using FMASK). Note that updating a buffer is considered a clear
    * operation but it might also use a CP DMA copy in some rare situations. Other operations using
    * a CP DMA clear are implicitly synchronized (see CP_DMA_SYNC).
    */
   if (stage_mask &
       (VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_CLEAR_BIT | VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT |
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT))
      radv_cp_dma_wait_for_idle(cmd_buffer);
}

static void
radv_barrier(struct radv_cmd_buffer *cmd_buffer, const VkDependencyInfo *dep_info, enum rgp_barrier_reason reason)
{
   enum radv_cmd_flush_bits src_flush_bits = 0;
   enum radv_cmd_flush_bits dst_flush_bits = 0;
   VkPipelineStageFlags2 src_stage_mask = 0;
   VkPipelineStageFlags2 dst_stage_mask = 0;

   if (cmd_buffer->state.render.active)
      radv_mark_noncoherent_rb(cmd_buffer);

   radv_describe_barrier_start(cmd_buffer, reason);

   for (uint32_t i = 0; i < dep_info->memoryBarrierCount; i++) {
      src_stage_mask |= dep_info->pMemoryBarriers[i].srcStageMask;
      src_flush_bits |= radv_src_access_flush(cmd_buffer, dep_info->pMemoryBarriers[i].srcAccessMask, NULL);
      dst_stage_mask |= dep_info->pMemoryBarriers[i].dstStageMask;
      dst_flush_bits |= radv_dst_access_flush(cmd_buffer, dep_info->pMemoryBarriers[i].dstAccessMask, NULL);
   }

   for (uint32_t i = 0; i < dep_info->bufferMemoryBarrierCount; i++) {
      src_stage_mask |= dep_info->pBufferMemoryBarriers[i].srcStageMask;
      src_flush_bits |= radv_src_access_flush(cmd_buffer, dep_info->pBufferMemoryBarriers[i].srcAccessMask, NULL);
      dst_stage_mask |= dep_info->pBufferMemoryBarriers[i].dstStageMask;
      dst_flush_bits |= radv_dst_access_flush(cmd_buffer, dep_info->pBufferMemoryBarriers[i].dstAccessMask, NULL);
   }

   for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; i++) {
      RADV_FROM_HANDLE(radv_image, image, dep_info->pImageMemoryBarriers[i].image);

      src_stage_mask |= dep_info->pImageMemoryBarriers[i].srcStageMask;
      src_flush_bits |= radv_src_access_flush(cmd_buffer, dep_info->pImageMemoryBarriers[i].srcAccessMask, image);
      dst_stage_mask |= dep_info->pImageMemoryBarriers[i].dstStageMask;
      dst_flush_bits |= radv_dst_access_flush(cmd_buffer, dep_info->pImageMemoryBarriers[i].dstAccessMask, image);
   }

   /* The Vulkan spec 1.1.98 says:
    *
    * "An execution dependency with only
    *  VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT in the destination stage mask
    *  will only prevent that stage from executing in subsequently
    *  submitted commands. As this stage does not perform any actual
    *  execution, this is not observable - in effect, it does not delay
    *  processing of subsequent commands. Similarly an execution dependency
    *  with only VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT in the source stage mask
    *  will effectively not wait for any prior commands to complete."
    */
   if (dst_stage_mask != VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT)
      radv_stage_flush(cmd_buffer, src_stage_mask);
   cmd_buffer->state.flush_bits |= src_flush_bits;

   radv_gang_barrier(cmd_buffer, src_stage_mask, 0);

   for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; i++) {
      RADV_FROM_HANDLE(radv_image, image, dep_info->pImageMemoryBarriers[i].image);

      const struct VkSampleLocationsInfoEXT *sample_locs_info =
         vk_find_struct_const(dep_info->pImageMemoryBarriers[i].pNext, SAMPLE_LOCATIONS_INFO_EXT);
      struct radv_sample_locations_state sample_locations;

      if (sample_locs_info) {
         assert(image->vk.create_flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT);
         sample_locations.per_pixel = sample_locs_info->sampleLocationsPerPixel;
         sample_locations.grid_size = sample_locs_info->sampleLocationGridSize;
         sample_locations.count = sample_locs_info->sampleLocationsCount;
         typed_memcpy(&sample_locations.locations[0], sample_locs_info->pSampleLocations,
                      sample_locs_info->sampleLocationsCount);
      }

      radv_handle_image_transition(
         cmd_buffer, image, dep_info->pImageMemoryBarriers[i].oldLayout, dep_info->pImageMemoryBarriers[i].newLayout,
         dep_info->pImageMemoryBarriers[i].srcQueueFamilyIndex, dep_info->pImageMemoryBarriers[i].dstQueueFamilyIndex,
         &dep_info->pImageMemoryBarriers[i].subresourceRange, sample_locs_info ? &sample_locations : NULL);
   }

   radv_gang_barrier(cmd_buffer, 0, dst_stage_mask);

   if (cmd_buffer->qf == RADV_QUEUE_TRANSFER) {
      /* SDMA NOP packet waits for all pending SDMA operations to complete.
       * Note that GFX9+ is supposed to have RAW dependency tracking, but it's buggy
       * so we can't rely on it fow now.
       */
      radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 1);
      radeon_emit(cmd_buffer->cs, SDMA_PACKET(SDMA_OPCODE_NOP, 0, 0));
   } else {
      const bool is_gfx_or_ace = cmd_buffer->qf == RADV_QUEUE_GENERAL || cmd_buffer->qf == RADV_QUEUE_COMPUTE;
      if (is_gfx_or_ace)
         radv_cp_dma_wait_for_stages(cmd_buffer, src_stage_mask);
   }

   cmd_buffer->state.flush_bits |= dst_flush_bits;

   radv_describe_barrier_end(cmd_buffer);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   enum rgp_barrier_reason barrier_reason;

   if (cmd_buffer->vk.runtime_rp_barrier) {
      barrier_reason = RGP_BARRIER_EXTERNAL_RENDER_PASS_SYNC;
   } else {
      barrier_reason = RGP_BARRIER_EXTERNAL_CMD_PIPELINE_BARRIER;
   }

   radv_barrier(cmd_buffer, pDependencyInfo, barrier_reason);
}

static void
write_event(struct radv_cmd_buffer *cmd_buffer, struct radv_event *event, VkPipelineStageFlags2 stageMask,
            unsigned value)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint64_t va = radv_buffer_get_va(event->bo);

   if (cmd_buffer->qf == RADV_QUEUE_VIDEO_DEC)
      return;

   radv_emit_cache_flush(cmd_buffer);

   radv_cs_add_buffer(cmd_buffer->device->ws, cs, event->bo);

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cs, 28);

   if (stageMask & (VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_RESOLVE_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT |
                    VK_PIPELINE_STAGE_2_CLEAR_BIT)) {
      /* Be conservative for now. */
      stageMask |= VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
   }

   /* Flags that only require a top-of-pipe event. */
   VkPipelineStageFlags2 top_of_pipe_flags = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;

   /* Flags that only require a post-index-fetch event. */
   VkPipelineStageFlags2 post_index_fetch_flags =
      top_of_pipe_flags | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;

   /* Flags that only require signaling post PS. */
   VkPipelineStageFlags2 post_ps_flags =
      post_index_fetch_flags | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
      VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT |
      VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT |
      VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT | VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT |
      VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
      VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

   /* Flags that only require signaling post CS. */
   VkPipelineStageFlags2 post_cs_flags = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

   radv_cp_dma_wait_for_stages(cmd_buffer, stageMask);

   if (!(stageMask & ~top_of_pipe_flags)) {
      /* Just need to sync the PFP engine. */
      radv_write_data(cmd_buffer, V_370_PFP, va, 1, &value, false);
   } else if (!(stageMask & ~post_index_fetch_flags)) {
      /* Sync ME because PFP reads index and indirect buffers. */
      radv_write_data(cmd_buffer, V_370_ME, va, 1, &value, false);
   } else {
      unsigned event_type;

      if (!(stageMask & ~post_ps_flags)) {
         /* Sync previous fragment shaders. */
         event_type = V_028A90_PS_DONE;
      } else if (!(stageMask & ~post_cs_flags)) {
         /* Sync previous compute shaders. */
         event_type = V_028A90_CS_DONE;
      } else {
         /* Otherwise, sync all prior GPU work. */
         event_type = V_028A90_BOTTOM_OF_PIPE_TS;
      }

      radv_cs_emit_write_event_eop(cs, cmd_buffer->device->physical_device->rad_info.gfx_level, cmd_buffer->qf,
                                   event_type, 0, EOP_DST_SEL_MEM, EOP_DATA_SEL_VALUE_32BIT, va, value,
                                   cmd_buffer->gfx9_eop_bug_va);
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent _event, const VkDependencyInfo *pDependencyInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_event, event, _event);
   VkPipelineStageFlags2 src_stage_mask = 0;

   for (uint32_t i = 0; i < pDependencyInfo->memoryBarrierCount; i++)
      src_stage_mask |= pDependencyInfo->pMemoryBarriers[i].srcStageMask;
   for (uint32_t i = 0; i < pDependencyInfo->bufferMemoryBarrierCount; i++)
      src_stage_mask |= pDependencyInfo->pBufferMemoryBarriers[i].srcStageMask;
   for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; i++)
      src_stage_mask |= pDependencyInfo->pImageMemoryBarriers[i].srcStageMask;

   write_event(cmd_buffer, event, src_stage_mask, 1);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent _event, VkPipelineStageFlags2 stageMask)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_event, event, _event);

   write_event(cmd_buffer, event, stageMask, 0);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                    const VkDependencyInfo *pDependencyInfos)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (cmd_buffer->qf == RADV_QUEUE_VIDEO_DEC)
      return;

   for (unsigned i = 0; i < eventCount; ++i) {
      RADV_FROM_HANDLE(radv_event, event, pEvents[i]);
      uint64_t va = radv_buffer_get_va(event->bo);

      radv_cs_add_buffer(cmd_buffer->device->ws, cs, event->bo);

      ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cs, 7);

      radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_EQUAL, va, 1, 0xffffffff);
      assert(cmd_buffer->cs->cdw <= cdw_max);
   }

   radv_barrier(cmd_buffer, pDependencyInfos, RGP_BARRIER_EXTERNAL_CMD_WAIT_EVENTS);
}

void
radv_begin_conditional_rendering(struct radv_cmd_buffer *cmd_buffer, uint64_t va, bool draw_visible)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   unsigned pred_op = PREDICATION_OP_BOOL32;

   radv_emit_cache_flush(cmd_buffer);

   if (cmd_buffer->qf == RADV_QUEUE_GENERAL) {
      if (!cmd_buffer->device->physical_device->rad_info.has_32bit_predication) {
         uint64_t pred_value = 0, pred_va;
         unsigned pred_offset;

         /* From the Vulkan spec 1.1.107:
          *
          * "If the 32-bit value at offset in buffer memory is zero,
          *  then the rendering commands are discarded, otherwise they
          *  are executed as normal. If the value of the predicate in
          *  buffer memory changes while conditional rendering is
          *  active, the rendering commands may be discarded in an
          *  implementation-dependent way. Some implementations may
          *  latch the value of the predicate upon beginning conditional
          *  rendering while others may read it before every rendering
          *  command."
          *
          * But, the AMD hardware treats the predicate as a 64-bit
          * value which means we need a workaround in the driver.
          * Luckily, it's not required to support if the value changes
          * when predication is active.
          *
          * The workaround is as follows:
          * 1) allocate a 64-value in the upload BO and initialize it
          *    to 0
          * 2) copy the 32-bit predicate value to the upload BO
          * 3) use the new allocated VA address for predication
          *
          * Based on the conditionalrender demo, it's faster to do the
          * COPY_DATA in ME  (+ sync PFP) instead of PFP.
          */
         radv_cmd_buffer_upload_data(cmd_buffer, 8, &pred_value, &pred_offset);

         pred_va = radv_buffer_get_va(cmd_buffer->upload.upload_bo) + pred_offset;

         radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 8);

         radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
         radeon_emit(
            cs, COPY_DATA_SRC_SEL(COPY_DATA_SRC_MEM) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) | COPY_DATA_WR_CONFIRM);
         radeon_emit(cs, va);
         radeon_emit(cs, va >> 32);
         radeon_emit(cs, pred_va);
         radeon_emit(cs, pred_va >> 32);

         radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
         radeon_emit(cs, 0);

         va = pred_va;
         pred_op = PREDICATION_OP_BOOL64;
      }

      radv_emit_set_predication_state(cmd_buffer, draw_visible, pred_op, va);
   } else {
      /* Compute queue doesn't support predication and it's emulated elsewhere. */
   }
}

void
radv_end_conditional_rendering(struct radv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->qf == RADV_QUEUE_GENERAL) {
      radv_emit_set_predication_state(cmd_buffer, false, 0, 0);
   } else {
      /* Compute queue doesn't support predication, no need to emit anything here. */
   }
}

/* VK_EXT_conditional_rendering */
VKAPI_ATTR void VKAPI_CALL
radv_CmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                     const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, pConditionalRenderingBegin->buffer);
   unsigned pred_op = PREDICATION_OP_BOOL32;
   bool draw_visible = true;
   uint64_t va;

   va = radv_buffer_get_va(buffer->bo) + buffer->offset + pConditionalRenderingBegin->offset;

   /* By default, if the 32-bit value at offset in buffer memory is zero,
    * then the rendering commands are discarded, otherwise they are
    * executed as normal. If the inverted flag is set, all commands are
    * discarded if the value is non zero.
    */
   if (pConditionalRenderingBegin->flags & VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT) {
      draw_visible = false;
   }

   radv_begin_conditional_rendering(cmd_buffer, va, draw_visible);

   /* Store conditional rendering user info. */
   cmd_buffer->state.predicating = true;
   cmd_buffer->state.predication_type = draw_visible;
   cmd_buffer->state.predication_op = pred_op;
   cmd_buffer->state.predication_va = va;
   cmd_buffer->mec_inv_pred_emitted = false;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   radv_end_conditional_rendering(cmd_buffer);

   /* Reset conditional rendering user info. */
   cmd_buffer->state.predicating = false;
   cmd_buffer->state.predication_type = -1;
   cmd_buffer->state.predication_op = 0;
   cmd_buffer->state.predication_va = 0;
   cmd_buffer->mec_inv_pred_emitted = false;
}

/* VK_EXT_transform_feedback */
VKAPI_ATTR void VKAPI_CALL
radv_CmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                        const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                        const VkDeviceSize *pSizes)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_streamout_binding *sb = cmd_buffer->streamout_bindings;
   uint8_t enabled_mask = 0;

   assert(firstBinding + bindingCount <= MAX_SO_BUFFERS);
   for (uint32_t i = 0; i < bindingCount; i++) {
      uint32_t idx = firstBinding + i;

      sb[idx].buffer = radv_buffer_from_handle(pBuffers[i]);
      sb[idx].offset = pOffsets[i];

      if (!pSizes || pSizes[i] == VK_WHOLE_SIZE) {
         sb[idx].size = sb[idx].buffer->vk.size - sb[idx].offset;
      } else {
         sb[idx].size = pSizes[i];
      }

      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, sb[idx].buffer->bo);

      enabled_mask |= 1 << idx;
   }

   cmd_buffer->state.streamout.enabled_mask |= enabled_mask;

   cmd_buffer->state.dirty |= RADV_CMD_DIRTY_STREAMOUT_BUFFER;
}

void
radv_emit_streamout_enable(struct radv_cmd_buffer *cmd_buffer)
{
   struct radv_streamout_state *so = &cmd_buffer->state.streamout;
   bool streamout_enabled = radv_is_streamout_enabled(cmd_buffer);
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint32_t enabled_stream_buffers_mask = 0;

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 4);

   if (cmd_buffer->state.last_vgt_shader) {
      enabled_stream_buffers_mask = cmd_buffer->state.last_vgt_shader->info.so.enabled_stream_buffers_mask;
   }

   radeon_set_context_reg_seq(cs, R_028B94_VGT_STRMOUT_CONFIG, 2);
   radeon_emit(cs, S_028B94_STREAMOUT_0_EN(streamout_enabled) | S_028B94_RAST_STREAM(0) |
                      S_028B94_STREAMOUT_1_EN(streamout_enabled) | S_028B94_STREAMOUT_2_EN(streamout_enabled) |
                      S_028B94_STREAMOUT_3_EN(streamout_enabled));
   radeon_emit(cs, so->hw_enabled_mask & enabled_stream_buffers_mask);

   cmd_buffer->state.context_roll_without_scissor_emitted = true;

   assert(cs->cdw <= cdw_max);
}

static void
radv_set_streamout_enable(struct radv_cmd_buffer *cmd_buffer, bool enable)
{
   struct radv_streamout_state *so = &cmd_buffer->state.streamout;
   bool old_streamout_enabled = radv_is_streamout_enabled(cmd_buffer);
   uint32_t old_hw_enabled_mask = so->hw_enabled_mask;

   so->streamout_enabled = enable;

   so->hw_enabled_mask =
      so->enabled_mask | (so->enabled_mask << 4) | (so->enabled_mask << 8) | (so->enabled_mask << 12);

   if (!cmd_buffer->device->physical_device->use_ngg_streamout &&
       ((old_streamout_enabled != radv_is_streamout_enabled(cmd_buffer)) ||
        (old_hw_enabled_mask != so->hw_enabled_mask)))
      radv_emit_streamout_enable(cmd_buffer);

   if (cmd_buffer->device->physical_device->use_ngg_streamout) {
      /* Re-emit streamout desciptors because with NGG streamout, a buffer size of 0 acts like a
       * disable bit and this is needed when streamout needs to be ignored in shaders.
       */
      cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY | RADV_CMD_DIRTY_STREAMOUT_BUFFER;
   }
}

static void
radv_flush_vgt_streamout(struct radv_cmd_buffer *cmd_buffer)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   unsigned reg_strmout_cntl;

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 14);

   /* The register is at different places on different ASICs. */
   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9) {
      reg_strmout_cntl = R_0300FC_CP_STRMOUT_CNTL;
      radeon_emit(cs, PKT3(PKT3_WRITE_DATA, 3, 0));
      radeon_emit(cs, S_370_DST_SEL(V_370_MEM_MAPPED_REGISTER) | S_370_ENGINE_SEL(V_370_ME));
      radeon_emit(cs, R_0300FC_CP_STRMOUT_CNTL >> 2);
      radeon_emit(cs, 0);
      radeon_emit(cs, 0);
   } else if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX7) {
      reg_strmout_cntl = R_0300FC_CP_STRMOUT_CNTL;
      radeon_set_uconfig_reg(cs, reg_strmout_cntl, 0);
   } else {
      reg_strmout_cntl = R_0084FC_CP_STRMOUT_CNTL;
      radeon_set_config_reg(cs, reg_strmout_cntl, 0);
   }

   radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
   radeon_emit(cs, EVENT_TYPE(V_028A90_SO_VGTSTREAMOUT_FLUSH) | EVENT_INDEX(0));

   radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0));
   radeon_emit(cs, WAIT_REG_MEM_EQUAL);    /* wait until the register is equal to the reference value */
   radeon_emit(cs, reg_strmout_cntl >> 2); /* register */
   radeon_emit(cs, 0);
   radeon_emit(cs, S_0084FC_OFFSET_UPDATE_DONE(1)); /* reference value */
   radeon_emit(cs, S_0084FC_OFFSET_UPDATE_DONE(1)); /* mask */
   radeon_emit(cs, 4);                              /* poll interval */

   assert(cs->cdw <= cdw_max);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                  uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                  const VkDeviceSize *pCounterBufferOffsets)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_streamout_binding *sb = cmd_buffer->streamout_bindings;
   struct radv_streamout_state *so = &cmd_buffer->state.streamout;
   struct radv_shader_info *info = &cmd_buffer->state.last_vgt_shader->info;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   assert(firstCounterBuffer + counterBufferCount <= MAX_SO_BUFFERS);
   if (!cmd_buffer->device->physical_device->use_ngg_streamout)
      radv_flush_vgt_streamout(cmd_buffer);

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, MAX_SO_BUFFERS * 10);

   u_foreach_bit (i, so->enabled_mask) {
      int32_t counter_buffer_idx = i - firstCounterBuffer;
      if (counter_buffer_idx >= 0 && counter_buffer_idx >= counterBufferCount)
         counter_buffer_idx = -1;

      bool append = counter_buffer_idx >= 0 && pCounterBuffers && pCounterBuffers[counter_buffer_idx];
      uint64_t va = 0;

      if (append) {
         RADV_FROM_HANDLE(radv_buffer, buffer, pCounterBuffers[counter_buffer_idx]);
         uint64_t counter_buffer_offset = 0;

         if (pCounterBufferOffsets)
            counter_buffer_offset = pCounterBufferOffsets[counter_buffer_idx];

         va += radv_buffer_get_va(buffer->bo);
         va += buffer->offset + counter_buffer_offset;

         radv_cs_add_buffer(cmd_buffer->device->ws, cs, buffer->bo);
      }

      if (cmd_buffer->device->physical_device->use_ngg_streamout) {
         if (append) {
            radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
            radeon_emit(cs,
                        COPY_DATA_SRC_SEL(COPY_DATA_SRC_MEM) | COPY_DATA_DST_SEL(COPY_DATA_REG) | COPY_DATA_WR_CONFIRM);
            radeon_emit(cs, va);
            radeon_emit(cs, va >> 32);
            radeon_emit(cs, (R_031088_GDS_STRMOUT_DWORDS_WRITTEN_0 >> 2) + i);
            radeon_emit(cs, 0);
         } else {
            /* The PKT3 CAM bit workaround seems needed for initializing this GDS register to zero. */
            radeon_set_perfctr_reg(cmd_buffer->device->physical_device->rad_info.gfx_level, cmd_buffer->qf, cs,
                                   R_031088_GDS_STRMOUT_DWORDS_WRITTEN_0 + i * 4, 0);
         }
      } else {
         /* AMD GCN binds streamout buffers as shader resources.
          * VGT only counts primitives and tells the shader through
          * SGPRs what to do.
          */
         radeon_set_context_reg_seq(cs, R_028AD0_VGT_STRMOUT_BUFFER_SIZE_0 + 16 * i, 2);
         radeon_emit(cs, sb[i].size >> 2);     /* BUFFER_SIZE (in DW) */
         radeon_emit(cs, info->so.strides[i]); /* VTX_STRIDE (in DW) */

         cmd_buffer->state.context_roll_without_scissor_emitted = true;

         if (append) {
            radeon_emit(cs, PKT3(PKT3_STRMOUT_BUFFER_UPDATE, 4, 0));
            radeon_emit(cs, STRMOUT_SELECT_BUFFER(i) | STRMOUT_DATA_TYPE(1) |   /* offset in bytes */
                               STRMOUT_OFFSET_SOURCE(STRMOUT_OFFSET_FROM_MEM)); /* control */
            radeon_emit(cs, 0);                                                 /* unused */
            radeon_emit(cs, 0);                                                 /* unused */
            radeon_emit(cs, va);                                                /* src address lo */
            radeon_emit(cs, va >> 32);                                          /* src address hi */
         } else {
            /* Start from the beginning. */
            radeon_emit(cs, PKT3(PKT3_STRMOUT_BUFFER_UPDATE, 4, 0));
            radeon_emit(cs, STRMOUT_SELECT_BUFFER(i) | STRMOUT_DATA_TYPE(1) |      /* offset in bytes */
                               STRMOUT_OFFSET_SOURCE(STRMOUT_OFFSET_FROM_PACKET)); /* control */
            radeon_emit(cs, 0);                                                    /* unused */
            radeon_emit(cs, 0);                                                    /* unused */
            radeon_emit(cs, 0);                                                    /* unused */
            radeon_emit(cs, 0);                                                    /* unused */
         }
      }
   }

   assert(cs->cdw <= cdw_max);

   radv_set_streamout_enable(cmd_buffer, true);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount,
                                const VkBuffer *pCounterBuffers, const VkDeviceSize *pCounterBufferOffsets)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_streamout_state *so = &cmd_buffer->state.streamout;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   assert(firstCounterBuffer + counterBufferCount <= MAX_SO_BUFFERS);

   if (cmd_buffer->device->physical_device->use_ngg_streamout) {
      /* Wait for streamout to finish before reading GDS_STRMOUT registers. */
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_VS_PARTIAL_FLUSH;
      radv_emit_cache_flush(cmd_buffer);
   } else {
      radv_flush_vgt_streamout(cmd_buffer);
   }

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, MAX_SO_BUFFERS * 12);

   u_foreach_bit (i, so->enabled_mask) {
      int32_t counter_buffer_idx = i - firstCounterBuffer;
      if (counter_buffer_idx >= 0 && counter_buffer_idx >= counterBufferCount)
         counter_buffer_idx = -1;

      bool append = counter_buffer_idx >= 0 && pCounterBuffers && pCounterBuffers[counter_buffer_idx];
      uint64_t va = 0;

      if (append) {
         RADV_FROM_HANDLE(radv_buffer, buffer, pCounterBuffers[counter_buffer_idx]);
         uint64_t counter_buffer_offset = 0;

         if (pCounterBufferOffsets)
            counter_buffer_offset = pCounterBufferOffsets[counter_buffer_idx];

         va += radv_buffer_get_va(buffer->bo);
         va += buffer->offset + counter_buffer_offset;

         radv_cs_add_buffer(cmd_buffer->device->ws, cs, buffer->bo);
      }

      if (cmd_buffer->device->physical_device->use_ngg_streamout) {
         if (append) {
            radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
            radeon_emit(cs,
                        COPY_DATA_SRC_SEL(COPY_DATA_REG) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) | COPY_DATA_WR_CONFIRM);
            radeon_emit(cs, (R_031088_GDS_STRMOUT_DWORDS_WRITTEN_0 >> 2) + i);
            radeon_emit(cs, 0);
            radeon_emit(cs, va);
            radeon_emit(cs, va >> 32);
         }
      } else {
         if (append) {
            radeon_emit(cs, PKT3(PKT3_STRMOUT_BUFFER_UPDATE, 4, 0));
            radeon_emit(cs, STRMOUT_SELECT_BUFFER(i) | STRMOUT_DATA_TYPE(1) | /* offset in bytes */
                               STRMOUT_OFFSET_SOURCE(STRMOUT_OFFSET_NONE) |
                               STRMOUT_STORE_BUFFER_FILLED_SIZE); /* control */
            radeon_emit(cs, va);                                  /* dst address lo */
            radeon_emit(cs, va >> 32);                            /* dst address hi */
            radeon_emit(cs, 0);                                   /* unused */
            radeon_emit(cs, 0);                                   /* unused */
         }

         /* Deactivate transform feedback by zeroing the buffer size.
          * The counters (primitives generated, primitives emitted) may
          * be enabled even if there is not buffer bound. This ensures
          * that the primitives-emitted query won't increment.
          */
         radeon_set_context_reg(cs, R_028AD0_VGT_STRMOUT_BUFFER_SIZE_0 + 16 * i, 0);

         cmd_buffer->state.context_roll_without_scissor_emitted = true;
      }
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);

   radv_set_streamout_enable(cmd_buffer, false);
}

static void
radv_emit_strmout_buffer(struct radv_cmd_buffer *cmd_buffer, const struct radv_draw_info *draw_info)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   uint64_t va = radv_buffer_get_va(draw_info->strmout_buffer->bo);
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   va += draw_info->strmout_buffer->offset + draw_info->strmout_buffer_offset;

   radeon_set_context_reg(cs, R_028B30_VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE, draw_info->stride);

   if (gfx_level >= GFX10) {
      /* Emitting a COPY_DATA packet should be enough because RADV doesn't support preemption
       * (shadow memory) but for unknown reasons, it can lead to GPU hangs on GFX10+.
       */
      radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      radeon_emit(cs, 0);

      radeon_emit(cs, PKT3(PKT3_LOAD_CONTEXT_REG_INDEX, 3, 0));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, (R_028B2C_VGT_STRMOUT_DRAW_OPAQUE_BUFFER_FILLED_SIZE - SI_CONTEXT_REG_OFFSET) >> 2);
      radeon_emit(cs, 1); /* 1 DWORD */
   } else {
      radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_SRC_MEM) | COPY_DATA_DST_SEL(COPY_DATA_REG) | COPY_DATA_WR_CONFIRM);
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, R_028B2C_VGT_STRMOUT_DRAW_OPAQUE_BUFFER_FILLED_SIZE >> 2);
      radeon_emit(cs, 0); /* unused */
   }

   radv_cs_add_buffer(cmd_buffer->device->ws, cs, draw_info->strmout_buffer->bo);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                 VkBuffer _counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                 uint32_t vertexStride)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, counterBuffer, _counterBuffer);
   struct radv_draw_info info;

   info.count = 0;
   info.instance_count = instanceCount;
   info.first_instance = firstInstance;
   info.strmout_buffer = counterBuffer;
   info.strmout_buffer_offset = counterBufferOffset;
   info.stride = vertexStride;
   info.indexed = false;
   info.indirect = NULL;

   if (!radv_before_draw(cmd_buffer, &info, 1, false))
      return;
   struct VkMultiDrawInfoEXT minfo = {0, 0};
   radv_emit_strmout_buffer(cmd_buffer, &info);
   radv_emit_direct_draw_packets(cmd_buffer, &info, 1, &minfo, S_0287F0_USE_OPAQUE(1), 0);
   radv_after_draw(cmd_buffer, false);
}

/* VK_AMD_buffer_marker */
VKAPI_ATTR void VKAPI_CALL
radv_CmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                              VkDeviceSize dstOffset, uint32_t marker)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_buffer, buffer, dstBuffer);
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const uint64_t va = radv_buffer_get_va(buffer->bo) + buffer->offset + dstOffset;

   if (cmd_buffer->qf == RADV_QUEUE_TRANSFER) {
      radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 4);
      radeon_emit(cmd_buffer->cs, SDMA_PACKET(SDMA_OPCODE_FENCE, 0, SDMA_FENCE_MTYPE_UC));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, marker);
      return;
   }

   radv_emit_cache_flush(cmd_buffer);

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 12);

   if (!(stage & ~VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT)) {
      radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_IMM) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) | COPY_DATA_WR_CONFIRM);
      radeon_emit(cs, marker);
      radeon_emit(cs, 0);
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
   } else {
      radv_cs_emit_write_event_eop(cs, cmd_buffer->device->physical_device->rad_info.gfx_level, cmd_buffer->qf,
                                   V_028A90_BOTTOM_OF_PIPE_TS, 0, EOP_DST_SEL_MEM, EOP_DATA_SEL_VALUE_32BIT, va, marker,
                                   cmd_buffer->gfx9_eop_bug_va);
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                  VkPipeline pipeline, uint32_t groupIndex)
{
   fprintf(stderr, "radv: unimplemented vkCmdBindPipelineShaderGroupNV\n");
   abort();
}

/* VK_NV_device_generated_commands_compute */
VKAPI_ATTR void VKAPI_CALL
radv_CmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                       VkPipeline pipeline)
{
   unreachable("radv: unimplemented vkCmdUpdatePipelineIndirectBufferNV");
}

/* VK_EXT_descriptor_buffer */
VKAPI_ATTR void VKAPI_CALL
radv_CmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                 const VkDescriptorBufferBindingInfoEXT *pBindingInfos)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   for (uint32_t i = 0; i < bufferCount; i++) {
      cmd_buffer->descriptor_buffers[i] = pBindingInfos[i].address;
   }
}

static void
radv_set_descriptor_buffer_offsets(struct radv_cmd_buffer *cmd_buffer,
                                   const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo,
                                   VkPipelineBindPoint bind_point)
{
   struct radv_descriptor_state *descriptors_state = radv_get_descriptors_state(cmd_buffer, bind_point);

   for (unsigned i = 0; i < pSetDescriptorBufferOffsetsInfo->setCount; i++) {
      const uint32_t buffer_idx = pSetDescriptorBufferOffsetsInfo->pBufferIndices[i];
      const uint64_t offset = pSetDescriptorBufferOffsetsInfo->pOffsets[i];
      unsigned idx = i + pSetDescriptorBufferOffsetsInfo->firstSet;

      descriptors_state->descriptor_buffers[idx] = cmd_buffer->descriptor_buffers[buffer_idx] + offset;

      radv_set_descriptor_set(cmd_buffer, bind_point, NULL, idx);
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer,
                                       const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);

   if (pSetDescriptorBufferOffsetsInfo->stageFlags & VK_SHADER_STAGE_COMPUTE_BIT) {
      radv_set_descriptor_buffer_offsets(cmd_buffer, pSetDescriptorBufferOffsetsInfo, VK_PIPELINE_BIND_POINT_COMPUTE);
   }

   if (pSetDescriptorBufferOffsetsInfo->stageFlags & RADV_GRAPHICS_STAGE_BITS) {
      radv_set_descriptor_buffer_offsets(cmd_buffer, pSetDescriptorBufferOffsetsInfo, VK_PIPELINE_BIND_POINT_GRAPHICS);
   }

   if (pSetDescriptorBufferOffsetsInfo->stageFlags & RADV_RT_STAGE_BITS) {
      radv_set_descriptor_buffer_offsets(cmd_buffer, pSetDescriptorBufferOffsetsInfo,
                                         VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBindDescriptorBufferEmbeddedSamplers2EXT(
   VkCommandBuffer commandBuffer,
   const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo)
{
   /* This is a no-op because embedded samplers are inlined at compile time. */
}
