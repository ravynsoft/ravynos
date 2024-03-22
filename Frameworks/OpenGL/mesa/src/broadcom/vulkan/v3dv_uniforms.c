/*
 * Copyright © 2019 Raspberry Pi Ltd
 *
 * Based in part on v3d driver which is:
 *
 * Copyright © 2014-2017 Broadcom
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

#include "v3dv_private.h"

/* Our Vulkan resource indices represent indices in descriptor maps which
 * include all shader stages, so we need to size the arrays below
 * accordingly. For now we only support a maximum of 3 stages: VS, GS, FS.
 */
#define MAX_STAGES 3

#define MAX_TOTAL_TEXTURE_SAMPLERS (V3D_MAX_TEXTURE_SAMPLERS * MAX_STAGES)
struct texture_bo_list {
   struct v3dv_bo *tex[MAX_TOTAL_TEXTURE_SAMPLERS];
};

/* This tracks state BOs for both textures and samplers, so we
 * multiply by 2.
 */
#define MAX_TOTAL_STATES (2 * V3D_MAX_TEXTURE_SAMPLERS * MAX_STAGES)
struct state_bo_list {
   uint32_t count;
   struct v3dv_bo *states[MAX_TOTAL_STATES];
};

#define MAX_TOTAL_UNIFORM_BUFFERS ((MAX_UNIFORM_BUFFERS + \
                                    MAX_INLINE_UNIFORM_BUFFERS) * MAX_STAGES)
#define MAX_TOTAL_STORAGE_BUFFERS (MAX_STORAGE_BUFFERS * MAX_STAGES)
struct buffer_bo_list {
   struct v3dv_bo *ubo[MAX_TOTAL_UNIFORM_BUFFERS];
   struct v3dv_bo *ssbo[MAX_TOTAL_STORAGE_BUFFERS];
};

static bool
state_bo_in_list(struct state_bo_list *list, struct v3dv_bo *bo)
{
   for (int i = 0; i < list->count; i++) {
      if (list->states[i] == bo)
         return true;
   }
   return false;
}

static void
push_constants_bo_free(VkDevice _device,
                       uint64_t bo_ptr,
                       VkAllocationCallbacks *alloc)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   v3dv_bo_free(device, (struct v3dv_bo *)(uintptr_t) bo_ptr);
}

/*
 * This method checks if the ubo used for push constants is needed to be
 * updated or not.
 *
 * push constants ubo is only used for push constants accessed by a non-const
 * index.
 */
static void
check_push_constants_ubo(struct v3dv_cmd_buffer *cmd_buffer,
                         struct v3dv_pipeline *pipeline)
{
   if (!(cmd_buffer->state.dirty & V3DV_CMD_DIRTY_PUSH_CONSTANTS_UBO) ||
       pipeline->layout->push_constant_size == 0)
      return;

   if (cmd_buffer->push_constants_resource.bo == NULL) {
      cmd_buffer->push_constants_resource.bo =
         v3dv_bo_alloc(cmd_buffer->device, 4096, "push constants", true);

      v3dv_job_add_bo(cmd_buffer->state.job,
                      cmd_buffer->push_constants_resource.bo);

      if (!cmd_buffer->push_constants_resource.bo) {
         fprintf(stderr, "Failed to allocate memory for push constants\n");
         abort();
      }

      bool ok = v3dv_bo_map(cmd_buffer->device,
                            cmd_buffer->push_constants_resource.bo,
                            cmd_buffer->push_constants_resource.bo->size);
      if (!ok) {
         fprintf(stderr, "failed to map push constants buffer\n");
         abort();
      }
   } else {
      if (cmd_buffer->push_constants_resource.offset +
          cmd_buffer->state.push_constants_size <=
          cmd_buffer->push_constants_resource.bo->size) {
         cmd_buffer->push_constants_resource.offset +=
            cmd_buffer->state.push_constants_size;
      } else {
         /* We ran out of space so we'll have to allocate a new buffer but we
          * need to ensure the old one is preserved until the end of the command
          * buffer life and make sure it is eventually freed. We use the
          * private object machinery in the command buffer for this.
          */
         v3dv_cmd_buffer_add_private_obj(
            cmd_buffer, (uintptr_t) cmd_buffer->push_constants_resource.bo,
            (v3dv_cmd_buffer_private_obj_destroy_cb) push_constants_bo_free);

         /* Now call back so we create a new BO */
         cmd_buffer->push_constants_resource.bo = NULL;
         check_push_constants_ubo(cmd_buffer, pipeline);
         return;
      }
   }

   assert(cmd_buffer->state.push_constants_size <= MAX_PUSH_CONSTANTS_SIZE);
   memcpy(cmd_buffer->push_constants_resource.bo->map +
          cmd_buffer->push_constants_resource.offset,
          cmd_buffer->state.push_constants_data,
          cmd_buffer->state.push_constants_size);

   cmd_buffer->state.dirty &= ~V3DV_CMD_DIRTY_PUSH_CONSTANTS_UBO;
}

/** V3D 4.x TMU configuration parameter 0 (texture) */
static void
write_tmu_p0(struct v3dv_cmd_buffer *cmd_buffer,
             struct v3dv_pipeline *pipeline,
             enum broadcom_shader_stage stage,
             struct v3dv_cl_out **uniforms,
             uint32_t data,
             struct texture_bo_list *tex_bos,
             struct state_bo_list *state_bos)
{
   uint32_t texture_idx = v3d_unit_data_get_unit(data);

   struct v3dv_descriptor_state *descriptor_state =
      v3dv_cmd_buffer_get_descriptor_state(cmd_buffer, pipeline);

   /* We need to ensure that the texture bo is added to the job */
   struct v3dv_bo *texture_bo =
      v3dv_descriptor_map_get_texture_bo(descriptor_state,
                                         &pipeline->shared_data->maps[stage]->texture_map,
                                         pipeline->layout, texture_idx);
   assert(texture_bo);
   assert(texture_idx < V3D_MAX_TEXTURE_SAMPLERS);
   tex_bos->tex[texture_idx] = texture_bo;

   struct v3dv_cl_reloc state_reloc =
      v3dv_descriptor_map_get_texture_shader_state(cmd_buffer->device, descriptor_state,
                                                   &pipeline->shared_data->maps[stage]->texture_map,
                                                   pipeline->layout,
                                                   texture_idx);

   cl_aligned_u32(uniforms, state_reloc.bo->offset +
                            state_reloc.offset +
                            v3d_unit_data_get_offset(data));

   /* Texture and Sampler states are typically suballocated, so they are
    * usually the same BO: only flag them once to avoid trying to add them
    * multiple times to the job later.
    */
   if (!state_bo_in_list(state_bos, state_reloc.bo)) {
      assert(state_bos->count < 2 * V3D_MAX_TEXTURE_SAMPLERS);
      state_bos->states[state_bos->count++] = state_reloc.bo;
   }
}

/** V3D 4.x TMU configuration parameter 1 (sampler) */
static void
write_tmu_p1(struct v3dv_cmd_buffer *cmd_buffer,
             struct v3dv_pipeline *pipeline,
             enum broadcom_shader_stage stage,
             struct v3dv_cl_out **uniforms,
             uint32_t data,
             struct state_bo_list *state_bos)
{
   uint32_t sampler_idx = v3d_unit_data_get_unit(data);
   struct v3dv_descriptor_state *descriptor_state =
      v3dv_cmd_buffer_get_descriptor_state(cmd_buffer, pipeline);

   assert(sampler_idx != V3DV_NO_SAMPLER_16BIT_IDX &&
          sampler_idx != V3DV_NO_SAMPLER_32BIT_IDX);

   struct v3dv_cl_reloc sampler_state_reloc =
      v3dv_descriptor_map_get_sampler_state(cmd_buffer->device, descriptor_state,
                                            &pipeline->shared_data->maps[stage]->sampler_map,
                                            pipeline->layout, sampler_idx);

   const struct v3dv_sampler *sampler =
      v3dv_descriptor_map_get_sampler(descriptor_state,
                                      &pipeline->shared_data->maps[stage]->sampler_map,
                                      pipeline->layout, sampler_idx);
   assert(sampler);

   /* Set unnormalized coordinates flag from sampler object */
   uint32_t p1_packed = v3d_unit_data_get_offset(data);
   if (sampler->unnormalized_coordinates) {
      v3d_pack_unnormalized_coordinates(&cmd_buffer->device->devinfo, &p1_packed,
                                        sampler->unnormalized_coordinates);
   }

   cl_aligned_u32(uniforms, sampler_state_reloc.bo->offset +
                            sampler_state_reloc.offset +
                            p1_packed);

   /* Texture and Sampler states are typically suballocated, so they are
    * usually the same BO: only flag them once to avoid trying to add them
    * multiple times to the job later.
    */
   if (!state_bo_in_list(state_bos, sampler_state_reloc.bo)) {
      assert(state_bos->count < 2 * V3D_MAX_TEXTURE_SAMPLERS);
      state_bos->states[state_bos->count++] = sampler_state_reloc.bo;
   }
}

static void
write_ubo_ssbo_uniforms(struct v3dv_cmd_buffer *cmd_buffer,
                        struct v3dv_pipeline *pipeline,
                        enum broadcom_shader_stage stage,
                        struct v3dv_cl_out **uniforms,
                        enum quniform_contents content,
                        uint32_t data,
                        struct buffer_bo_list *buffer_bos)
{
   struct v3dv_descriptor_state *descriptor_state =
      v3dv_cmd_buffer_get_descriptor_state(cmd_buffer, pipeline);

   struct v3dv_descriptor_map *map =
      content == QUNIFORM_UBO_ADDR || content == QUNIFORM_GET_UBO_SIZE ?
      &pipeline->shared_data->maps[stage]->ubo_map :
      &pipeline->shared_data->maps[stage]->ssbo_map;

   uint32_t offset =
      content == QUNIFORM_UBO_ADDR ?
      v3d_unit_data_get_offset(data) :
      0;

   uint32_t dynamic_offset = 0;

   /* For ubos, index is shifted, as 0 is reserved for push constants
    * and 1..MAX_INLINE_UNIFORM_BUFFERS are reserved for inline uniform
    * buffers.
    */
   uint32_t index = v3d_unit_data_get_unit(data);
   if (content == QUNIFORM_UBO_ADDR && index == 0) {
      /* Ensure the push constants UBO is created and updated. This also
       * adds the BO to the job so we don't need to track it in buffer_bos.
       */
      check_push_constants_ubo(cmd_buffer, pipeline);

      struct v3dv_cl_reloc *resource =
         &cmd_buffer->push_constants_resource;
      assert(resource->bo);

      cl_aligned_u32(uniforms, resource->bo->offset +
                               resource->offset +
                               offset + dynamic_offset);
   } else {
      if (content == QUNIFORM_UBO_ADDR) {
         /* We reserve UBO index 0 for push constants in Vulkan (and for the
          * constant buffer in GL) so the compiler always adds one to all UBO
          * indices, fix it up before we access the descriptor map, since
          * indices start from 0 there.
          */
         assert(index > 0);
         index--;
      } else {
         index = data;
      }

      struct v3dv_descriptor *descriptor =
         v3dv_descriptor_map_get_descriptor(descriptor_state, map,
                                            pipeline->layout,
                                            index, &dynamic_offset);

      /* Inline UBO descriptors store UBO data in descriptor pool memory,
       * instead of an external buffer.
       */
      assert(descriptor);

      if (content == QUNIFORM_GET_SSBO_SIZE ||
          content == QUNIFORM_GET_UBO_SIZE) {
         cl_aligned_u32(uniforms, descriptor->range);
      } else {
         /* Inline uniform buffers store their contents in pool memory instead
          * of an external buffer.
          */
         struct v3dv_bo *bo;
         uint32_t addr;
         if (descriptor->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            assert(dynamic_offset == 0);
            struct v3dv_cl_reloc reloc =
               v3dv_descriptor_map_get_descriptor_bo(cmd_buffer->device,
                                                     descriptor_state, map,
                                                     pipeline->layout, index,
                                                     NULL);
            bo = reloc.bo;
            addr = reloc.bo->offset + reloc.offset + offset;
         } else {
            assert(descriptor->buffer);
            assert(descriptor->buffer->mem);
            assert(descriptor->buffer->mem->bo);

            bo = descriptor->buffer->mem->bo;
            addr = bo->offset +
                   descriptor->buffer->mem_offset +
                   descriptor->offset +
                   offset + dynamic_offset;
         }

         cl_aligned_u32(uniforms, addr);

         if (content == QUNIFORM_UBO_ADDR) {
            assert(index < MAX_TOTAL_UNIFORM_BUFFERS);
            buffer_bos->ubo[index] = bo;
         } else {
            assert(index < MAX_TOTAL_STORAGE_BUFFERS);
            buffer_bos->ssbo[index] = bo;
         }
      }
   }
}

static void
write_inline_uniform(struct v3dv_cl_out **uniforms,
                     uint32_t index,
                     uint32_t offset,
                     struct v3dv_cmd_buffer *cmd_buffer,
                     struct v3dv_pipeline *pipeline,
                     enum broadcom_shader_stage stage)
{
   assert(index < MAX_INLINE_UNIFORM_BUFFERS);

   struct v3dv_descriptor_state *descriptor_state =
      v3dv_cmd_buffer_get_descriptor_state(cmd_buffer, pipeline);

   struct v3dv_descriptor_map *map =
      &pipeline->shared_data->maps[stage]->ubo_map;

   struct v3dv_cl_reloc reloc =
      v3dv_descriptor_map_get_descriptor_bo(cmd_buffer->device,
                                            descriptor_state, map,
                                            pipeline->layout, index,
                                            NULL);

   /* Offset comes in 32-bit units */
   uint32_t *addr = reloc.bo->map + reloc.offset + 4 * offset;
   cl_aligned_u32(uniforms, *addr);
}

static uint32_t
get_texture_size_from_image_view(struct v3dv_image_view *image_view,
                                 enum quniform_contents contents,
                                 uint32_t data)
{
   switch(contents) {
   case QUNIFORM_IMAGE_WIDTH:
   case QUNIFORM_TEXTURE_WIDTH:
      /* We don't u_minify the values, as we are using the image_view
       * extents
       */
      return image_view->vk.extent.width;
   case QUNIFORM_IMAGE_HEIGHT:
   case QUNIFORM_TEXTURE_HEIGHT:
      return image_view->vk.extent.height;
   case QUNIFORM_IMAGE_DEPTH:
   case QUNIFORM_TEXTURE_DEPTH:
      return image_view->vk.extent.depth;
   case QUNIFORM_IMAGE_ARRAY_SIZE:
   case QUNIFORM_TEXTURE_ARRAY_SIZE:
      if (image_view->vk.view_type != VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
         return image_view->vk.layer_count;
      } else {
         assert(image_view->vk.layer_count % 6 == 0);
         return image_view->vk.layer_count / 6;
      }
   case QUNIFORM_TEXTURE_LEVELS:
      return image_view->vk.level_count;
   case QUNIFORM_TEXTURE_SAMPLES:
      assert(image_view->vk.image);
      return image_view->vk.image->samples;
   default:
      unreachable("Bad texture size field");
   }
}


static uint32_t
get_texture_size_from_buffer_view(struct v3dv_buffer_view *buffer_view,
                                  enum quniform_contents contents,
                                  uint32_t data)
{
   switch(contents) {
   case QUNIFORM_IMAGE_WIDTH:
   case QUNIFORM_TEXTURE_WIDTH:
      return buffer_view->num_elements;
   /* Only size can be queried for texel buffers  */
   default:
      unreachable("Bad texture size field for texel buffers");
   }
}

static uint32_t
get_texture_size(struct v3dv_cmd_buffer *cmd_buffer,
                 struct v3dv_pipeline *pipeline,
                 enum broadcom_shader_stage stage,
                 enum quniform_contents contents,
                 uint32_t data)
{
   uint32_t texture_idx = data;

   struct v3dv_descriptor_state *descriptor_state =
      v3dv_cmd_buffer_get_descriptor_state(cmd_buffer, pipeline);

   struct v3dv_descriptor *descriptor =
      v3dv_descriptor_map_get_descriptor(descriptor_state,
                                         &pipeline->shared_data->maps[stage]->texture_map,
                                         pipeline->layout,
                                         texture_idx, NULL);

   assert(descriptor);

   switch (descriptor->type) {
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return get_texture_size_from_image_view(descriptor->image_view,
                                              contents, data);
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return get_texture_size_from_buffer_view(descriptor->buffer_view,
                                               contents, data);
   default:
      unreachable("Wrong descriptor for getting texture size");
   }
}

struct v3dv_cl_reloc
v3dv_write_uniforms_wg_offsets(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_pipeline *pipeline,
                               struct v3dv_shader_variant *variant,
                               uint32_t **wg_count_offsets)
{
   struct v3d_uniform_list *uinfo =
      &variant->prog_data.base->uniforms;
   struct v3dv_dynamic_state *dynamic = &cmd_buffer->state.dynamic;

   struct v3dv_job *job = cmd_buffer->state.job;
   assert(job);
   assert(job->cmd_buffer == cmd_buffer);

   struct texture_bo_list tex_bos = { 0 };
   struct state_bo_list state_bos = { 0 };
   struct buffer_bo_list buffer_bos = { 0 };

   /* The hardware always pre-fetches the next uniform (also when there
    * aren't any), so we always allocate space for an extra slot. This
    * fixes MMU exceptions reported since Linux kernel 5.4 when the
    * uniforms fill up the tail bytes of a page in the indirect
    * BO. In that scenario, when the hardware pre-fetches after reading
    * the last uniform it will read beyond the end of the page and trigger
    * the MMU exception.
    */
   v3dv_cl_ensure_space(&job->indirect, (uinfo->count + 1) * 4, 4);

   struct v3dv_cl_reloc uniform_stream = v3dv_cl_get_address(&job->indirect);

   struct v3dv_cl_out *uniforms = cl_start(&job->indirect);
   for (int i = 0; i < uinfo->count; i++) {
      uint32_t data = uinfo->data[i];

      switch (uinfo->contents[i]) {
      case QUNIFORM_CONSTANT:
         cl_aligned_u32(&uniforms, data);
         break;

      case QUNIFORM_UNIFORM:
         cl_aligned_u32(&uniforms, cmd_buffer->state.push_constants_data[data]);
         break;

      case QUNIFORM_INLINE_UBO_0:
      case QUNIFORM_INLINE_UBO_1:
      case QUNIFORM_INLINE_UBO_2:
      case QUNIFORM_INLINE_UBO_3:
         write_inline_uniform(&uniforms,
                              uinfo->contents[i] - QUNIFORM_INLINE_UBO_0, data,
                              cmd_buffer, pipeline, variant->stage);
         break;

      case QUNIFORM_VIEWPORT_X_SCALE: {
         float clipper_xy_granularity = V3DV_X(cmd_buffer->device, CLIPPER_XY_GRANULARITY);
         cl_aligned_f(&uniforms, dynamic->viewport.scale[0][0] * clipper_xy_granularity);
         break;
      }

      case QUNIFORM_VIEWPORT_Y_SCALE: {
         float clipper_xy_granularity = V3DV_X(cmd_buffer->device, CLIPPER_XY_GRANULARITY);
         cl_aligned_f(&uniforms, dynamic->viewport.scale[0][1] * clipper_xy_granularity);
         break;
      }

      case QUNIFORM_VIEWPORT_Z_OFFSET: {
         float translate_z;
         v3dv_cmd_buffer_state_get_viewport_z_xform(&cmd_buffer->state, 0,
                                                    &translate_z, NULL);
         cl_aligned_f(&uniforms, translate_z);
         break;
      }

      case QUNIFORM_VIEWPORT_Z_SCALE: {
         float scale_z;
         v3dv_cmd_buffer_state_get_viewport_z_xform(&cmd_buffer->state, 0,
                                                    NULL, &scale_z);
         cl_aligned_f(&uniforms, scale_z);
         break;
      }

      case QUNIFORM_SSBO_OFFSET:
      case QUNIFORM_UBO_ADDR:
      case QUNIFORM_GET_SSBO_SIZE:
      case QUNIFORM_GET_UBO_SIZE:
         write_ubo_ssbo_uniforms(cmd_buffer, pipeline, variant->stage, &uniforms,
                                 uinfo->contents[i], data, &buffer_bos);

        break;

      case QUNIFORM_IMAGE_TMU_CONFIG_P0:
      case QUNIFORM_TMU_CONFIG_P0:
         write_tmu_p0(cmd_buffer, pipeline, variant->stage,
                      &uniforms, data, &tex_bos, &state_bos);
         break;

      case QUNIFORM_TMU_CONFIG_P1:
         write_tmu_p1(cmd_buffer, pipeline, variant->stage,
                      &uniforms, data, &state_bos);
         break;

      case QUNIFORM_IMAGE_WIDTH:
      case QUNIFORM_IMAGE_HEIGHT:
      case QUNIFORM_IMAGE_DEPTH:
      case QUNIFORM_IMAGE_ARRAY_SIZE:
      case QUNIFORM_TEXTURE_WIDTH:
      case QUNIFORM_TEXTURE_HEIGHT:
      case QUNIFORM_TEXTURE_DEPTH:
      case QUNIFORM_TEXTURE_ARRAY_SIZE:
      case QUNIFORM_TEXTURE_LEVELS:
      case QUNIFORM_TEXTURE_SAMPLES:
         cl_aligned_u32(&uniforms,
                        get_texture_size(cmd_buffer,
                                         pipeline,
                                         variant->stage,
                                         uinfo->contents[i],
                                         data));
         break;

      /* We generate this from geometry shaders to cap the generated gl_Layer
       * to be within the number of layers of the framebuffer so we prevent the
       * binner from trying to access tile state memory out of bounds (for
       * layers that don't exist).
       *
       * Unfortunately, for secondary command buffers we may not know the
       * number of layers in the framebuffer at this stage. Since we are
       * only using this to sanitize the shader and it should not have any
       * impact on correct shaders that emit valid values for gl_Layer,
       * we just work around it by using the largest number of layers we
       * support.
       *
       * FIXME: we could do better than this by recording in the job that
       * the value at this uniform offset is not correct, and patch it when
       * we execute the secondary command buffer into a primary, since we do
       * have the correct number of layers at that point, but again, since this
       * is only for sanityzing the shader and it only affects the specific case
       * of secondary command buffers without framebuffer info available it
       * might not be worth the trouble.
       *
       * With multiview the number of layers is dictated by the view mask
       * and not by the framebuffer layers. We do set the job's frame tiling
       * information correctly from the view mask in that case, however,
       * secondary command buffers may not have valid frame tiling data,
       * so when multiview is enabled, we always set the number of layers
       * from the subpass view mask.
       */
      case QUNIFORM_FB_LAYERS: {
         const struct v3dv_cmd_buffer_state *state = &job->cmd_buffer->state;
         const uint32_t view_mask =
            state->pass->subpasses[state->subpass_idx].view_mask;

         uint32_t num_layers;
         if (view_mask != 0) {
            num_layers = util_last_bit(view_mask);
         } else if (job->frame_tiling.layers != 0) {
            num_layers = job->frame_tiling.layers;
         } else if (cmd_buffer->state.framebuffer) {
            num_layers = cmd_buffer->state.framebuffer->layers;
         } else {
            assert(cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
            num_layers = 2048;
#if DEBUG
            fprintf(stderr, "Skipping gl_LayerID shader sanity check for "
                            "secondary command buffer\n");
#endif
         }
         cl_aligned_u32(&uniforms, num_layers);
         break;
      }

      case QUNIFORM_VIEW_INDEX:
         cl_aligned_u32(&uniforms, job->cmd_buffer->state.view_index);
         break;

      case QUNIFORM_NUM_WORK_GROUPS:
         assert(job->type == V3DV_JOB_TYPE_GPU_CSD);
         assert(job->csd.wg_count[data] > 0);
         if (wg_count_offsets)
            wg_count_offsets[data] = (uint32_t *) uniforms;
         cl_aligned_u32(&uniforms, job->csd.wg_count[data]);
         break;

      case QUNIFORM_WORK_GROUP_BASE:
         assert(job->type == V3DV_JOB_TYPE_GPU_CSD);
         cl_aligned_u32(&uniforms, job->csd.wg_base[data]);
         break;

      case QUNIFORM_SHARED_OFFSET:
         assert(job->type == V3DV_JOB_TYPE_GPU_CSD);
         assert(job->csd.shared_memory);
         cl_aligned_u32(&uniforms, job->csd.shared_memory->offset);
         break;

      case QUNIFORM_SPILL_OFFSET:
         assert(pipeline->spill.bo);
         cl_aligned_u32(&uniforms, pipeline->spill.bo->offset);
         break;

      case QUNIFORM_SPILL_SIZE_PER_THREAD:
         assert(pipeline->spill.size_per_thread > 0);
         cl_aligned_u32(&uniforms, pipeline->spill.size_per_thread);
         break;

      case QUNIFORM_DRAW_ID:
         cl_aligned_u32(&uniforms, job->cmd_buffer->state.draw_id);
         break;

      default:
         unreachable("unsupported quniform_contents uniform type\n");
      }
   }

   cl_end(&job->indirect, uniforms);

   for (int i = 0; i < MAX_TOTAL_TEXTURE_SAMPLERS; i++) {
      if (tex_bos.tex[i])
         v3dv_job_add_bo(job, tex_bos.tex[i]);
   }

   for (int i = 0; i < state_bos.count; i++)
      v3dv_job_add_bo(job, state_bos.states[i]);

   for (int i = 0; i < MAX_TOTAL_UNIFORM_BUFFERS; i++) {
      if (buffer_bos.ubo[i])
         v3dv_job_add_bo(job, buffer_bos.ubo[i]);
   }

   for (int i = 0; i < MAX_TOTAL_STORAGE_BUFFERS; i++) {
      if (buffer_bos.ssbo[i])
         v3dv_job_add_bo(job, buffer_bos.ssbo[i]);
   }

   if (job->csd.shared_memory)
      v3dv_job_add_bo(job, job->csd.shared_memory);

   if (pipeline->spill.bo)
      v3dv_job_add_bo(job, pipeline->spill.bo);

   return uniform_stream;
}

struct v3dv_cl_reloc
v3dv_write_uniforms(struct v3dv_cmd_buffer *cmd_buffer,
                    struct v3dv_pipeline *pipeline,
                    struct v3dv_shader_variant *variant)
{
   return v3dv_write_uniforms_wg_offsets(cmd_buffer, pipeline, variant, NULL);
}
