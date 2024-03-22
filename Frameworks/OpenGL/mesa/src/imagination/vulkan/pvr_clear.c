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

#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "hwdef/rogue_hw_utils.h"
#include "pvr_clear.h"
#include "pvr_hardcode.h"
#include "pvr_pds.h"
#include "pvr_private.h"
#include "pvr_shader_factory.h"
#include "pvr_static_shaders.h"
#include "pvr_types.h"
#include "vk_alloc.h"
#include "vk_log.h"

static void pvr_device_setup_graphics_static_clear_ppp_base(
   struct pvr_static_clear_ppp_base *const base)
{
   pvr_csb_pack (&base->wclamp, TA_WCLAMP, wclamp) {
      wclamp.val = fui(0.00001f);
   }

   /* clang-format off */
   pvr_csb_pack (&base->varying_word[0], TA_STATE_VARYING0, varying0);
   pvr_csb_pack (&base->varying_word[1], TA_STATE_VARYING1, varying1);
   pvr_csb_pack (&base->varying_word[2], TA_STATE_VARYING2, varying2);
   /* clang-format on */

   pvr_csb_pack (&base->ppp_ctrl, TA_STATE_PPP_CTRL, ppp_ctrl) {
      ppp_ctrl.pretransform = true;
      ppp_ctrl.cullmode = PVRX(TA_CULLMODE_NO_CULLING);
   }

   /* clang-format off */
   pvr_csb_pack (&base->stream_out0, TA_STATE_STREAM_OUT0, stream_out0);
   /* clang-format on */
}

static void pvr_device_setup_graphics_static_clear_ppp_templates(
   struct pvr_static_clear_ppp_template
      templates[static PVR_STATIC_CLEAR_VARIANT_COUNT])
{
   for (uint32_t i = 0; i < PVR_STATIC_CLEAR_VARIANT_COUNT; i++) {
      const bool has_color = !!(i & VK_IMAGE_ASPECT_COLOR_BIT);
      const bool has_depth = !!(i & VK_IMAGE_ASPECT_DEPTH_BIT);
      const bool has_stencil = !!(i & VK_IMAGE_ASPECT_STENCIL_BIT);

      struct pvr_static_clear_ppp_template *const template = &templates[i];

      template->requires_pds_state = has_color;

      pvr_csb_pack (&template->header, TA_STATE_HEADER, header) {
         header.pres_stream_out_size = true;
         header.pres_ppp_ctrl = true;
         header.pres_varying_word2 = true;
         header.pres_varying_word1 = true;
         header.pres_varying_word0 = true;
         header.pres_outselects = true;
         header.pres_wclamp = true;
         header.pres_region_clip = true;
         header.pres_pds_state_ptr2 = template->requires_pds_state;
         header.pres_pds_state_ptr1 = template->requires_pds_state;
         header.pres_pds_state_ptr0 = template->requires_pds_state;
         header.pres_ispctl_fb = true;
         header.pres_ispctl_fa = true;
         header.pres_ispctl = true;
      }

#define CS_HEADER(cs)    \
   (struct PVRX(cs))     \
   {                     \
      pvr_cmd_header(cs) \
   }

      template->config.ispctl = CS_HEADER(TA_STATE_ISPCTL);
      template->config.ispctl.tagwritedisable = !has_color;
      template->config.ispctl.bpres = true;

      template->config.ispa = CS_HEADER(TA_STATE_ISPA);
      template->config.ispa.objtype = PVRX(TA_OBJTYPE_TRIANGLE);
      template->config.ispa.passtype = PVRX(TA_PASSTYPE_TRANSLUCENT);
      template->config.ispa.dwritedisable = !has_depth;
      template->config.ispa.dcmpmode = (i == 0) ? PVRX(TA_CMPMODE_NEVER)
                                                : PVRX(TA_CMPMODE_ALWAYS);
      template->config.ispa.sref =
         has_stencil ? PVRX(TA_STATE_ISPA_SREF_SIZE_MAX) : 0;

      pvr_csb_pack (&template->ispb, TA_STATE_ISPB, ispb) {
         ispb.scmpmode = PVRX(TA_CMPMODE_ALWAYS);
         ispb.sop1 = PVRX(TA_ISPB_STENCILOP_KEEP);
         ispb.sop2 = PVRX(TA_ISPB_STENCILOP_KEEP);

         ispb.sop3 = has_stencil ? PVRX(TA_ISPB_STENCILOP_REPLACE)
                                 : PVRX(TA_ISPB_STENCILOP_KEEP);

         ispb.swmask = has_stencil ? 0xFF : 0;
      }

      template->config.pds_state = NULL;

      template->config.region_clip0 = CS_HEADER(TA_REGION_CLIP0);
      template->config.region_clip0.mode = PVRX(TA_REGION_CLIP_MODE_OUTSIDE);
      template->config.region_clip0.left = 0;
      template->config.region_clip0.right = PVRX(TA_REGION_CLIP_MAX);

      template->config.region_clip1 = CS_HEADER(TA_REGION_CLIP1);
      template->config.region_clip1.top = 0;
      template->config.region_clip1.bottom = PVRX(TA_REGION_CLIP_MAX);

      template->config.output_sel = CS_HEADER(TA_OUTPUT_SEL);
      template->config.output_sel.vtxsize = 4;
      template->config.output_sel.rhw_pres = true;

#undef CS_HEADER
   }
}

/**
 * \brief Emit geom state from a configurable template.
 *
 * Note that the state is emitted by joining the template with a base so the
 * base must have been setup before calling this.
 *
 * \param[in] csb          Control stream to emit to.
 * \param[in] template     The configured template.
 * \param[out] pvr_bo_out  Uploaded state's pvr_bo object.
 *
 * \return   VK_SUCCESS if the state was successfully uploaded.
 */
VkResult pvr_emit_ppp_from_template(
   struct pvr_csb *const csb,
   const struct pvr_static_clear_ppp_template *const template,
   struct pvr_suballoc_bo **const pvr_bo_out)
{
   const uint32_t dword_count =
      pvr_cmd_length(TA_STATE_HEADER) + pvr_cmd_length(TA_STATE_ISPCTL) +
      pvr_cmd_length(TA_STATE_ISPA) + pvr_cmd_length(TA_STATE_ISPB) +
      (template->requires_pds_state ? PVR_STATIC_CLEAR_PDS_STATE_COUNT : 0) +
      pvr_cmd_length(TA_REGION_CLIP0) + pvr_cmd_length(TA_REGION_CLIP1) +
      pvr_cmd_length(TA_WCLAMP) + pvr_cmd_length(TA_OUTPUT_SEL) +
      pvr_cmd_length(TA_STATE_VARYING0) + pvr_cmd_length(TA_STATE_VARYING1) +
      pvr_cmd_length(TA_STATE_VARYING2) + pvr_cmd_length(TA_STATE_PPP_CTRL) +
      pvr_cmd_length(TA_STATE_STREAM_OUT0);

   struct pvr_device *const device = csb->device;
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
   const struct pvr_static_clear_ppp_base *const base =
      &device->static_clear_state.ppp_base;
   struct pvr_suballoc_bo *pvr_bo;
   uint32_t *stream;
   VkResult result;

   result = pvr_bo_suballoc(&device->suballoc_general,
                            PVR_DW_TO_BYTES(dword_count),
                            cache_line_size,
                            false,
                            &pvr_bo);
   if (result != VK_SUCCESS) {
      *pvr_bo_out = NULL;
      return result;
   }

   stream = (uint32_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);

   pvr_csb_write_value(stream, TA_STATE_HEADER, template->header);
   pvr_csb_write_struct(stream, TA_STATE_ISPCTL, &template->config.ispctl);
   pvr_csb_write_struct(stream, TA_STATE_ISPA, &template->config.ispa);
   pvr_csb_write_value(stream, TA_STATE_ISPB, template->ispb);

   if (template->requires_pds_state) {
      static_assert(sizeof(*stream) == sizeof((*template->config.pds_state)[0]),
                    "Size mismatch");
      for (uint32_t i = 0; i < PVR_STATIC_CLEAR_PDS_STATE_COUNT; i++)
         *stream++ = (*template->config.pds_state)[i];
   }

   pvr_csb_write_struct(stream,
                        TA_REGION_CLIP0,
                        &template->config.region_clip0);
   pvr_csb_write_struct(stream,
                        TA_REGION_CLIP1,
                        &template->config.region_clip1);
   pvr_csb_write_value(stream, TA_WCLAMP, base->wclamp);
   pvr_csb_write_struct(stream, TA_OUTPUT_SEL, &template->config.output_sel);
   pvr_csb_write_value(stream, TA_STATE_VARYING0, base->varying_word[0]);
   pvr_csb_write_value(stream, TA_STATE_VARYING1, base->varying_word[1]);
   pvr_csb_write_value(stream, TA_STATE_VARYING2, base->varying_word[2]);
   pvr_csb_write_value(stream, TA_STATE_PPP_CTRL, base->ppp_ctrl);
   pvr_csb_write_value(stream, TA_STATE_STREAM_OUT0, base->stream_out0);

   assert((uint64_t)(stream - (uint32_t *)pvr_bo_suballoc_get_map_addr(
                                 pvr_bo)) == dword_count);

   stream = NULL;

   pvr_csb_set_relocation_mark(csb);

   pvr_csb_emit (csb, VDMCTRL_PPP_STATE0, state) {
      state.word_count = dword_count;
      state.addrmsb = pvr_bo->dev_addr;
   }

   pvr_csb_emit (csb, VDMCTRL_PPP_STATE1, state) {
      state.addrlsb = pvr_bo->dev_addr;
   }

   pvr_csb_clear_relocation_mark(csb);

   *pvr_bo_out = pvr_bo;

   return VK_SUCCESS;
}

static VkResult
pvr_device_init_clear_attachment_programs(struct pvr_device *device)
{
   const uint32_t pds_prog_alignment =
      MAX2(PVRX(TA_STATE_PDS_TEXUNICODEBASE_ADDR_ALIGNMENT),
           PVRX(TA_STATE_PDS_SHADERBASE_ADDR_ALIGNMENT));
   struct pvr_device_static_clear_state *clear_state =
      &device->static_clear_state;
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   uint32_t pds_texture_program_offsets[PVR_CLEAR_ATTACHMENT_PROGRAM_COUNT];
   uint32_t pds_pixel_program_offsets[PVR_CLEAR_ATTACHMENT_PROGRAM_COUNT];
   uint32_t usc_program_offsets[PVR_CLEAR_ATTACHMENT_PROGRAM_COUNT];
   uint64_t usc_upload_offset;
   uint64_t pds_upload_offset;
   uint32_t alloc_size = 0;
   VkResult result;
   uint8_t *ptr;

#if !defined(NDEBUG)
   uint32_t clear_attachment_info_count = 0;

   for (uint32_t i = 0; i < ARRAY_SIZE(clear_attachment_collection); i++) {
      if (!clear_attachment_collection[i].info)
         continue;

      clear_attachment_info_count++;
   }

   assert(clear_attachment_info_count == PVR_CLEAR_ATTACHMENT_PROGRAM_COUNT);
#endif

   /* Upload USC fragment shaders. */

   for (uint32_t i = 0, offset_idx = 0;
        i < ARRAY_SIZE(clear_attachment_collection);
        i++) {
      if (!clear_attachment_collection[i].info)
         continue;

      usc_program_offsets[offset_idx] = alloc_size;
      /* TODO: The compiler will likely give us a pre-aligned size for the USC
       * shader so don't bother aligning here when it's hooked up.
       */
      alloc_size += ALIGN_POT(clear_attachment_collection[i].size, 4);

      offset_idx++;
   }

   result = pvr_bo_suballoc(&device->suballoc_usc,
                            alloc_size,
                            4,
                            false,
                            &clear_state->usc_clear_attachment_programs);
   if (result != VK_SUCCESS)
      return result;

   usc_upload_offset =
      clear_state->usc_clear_attachment_programs->dev_addr.addr -
      device->heaps.usc_heap->base_addr.addr;
   ptr = (uint8_t *)pvr_bo_suballoc_get_map_addr(
      clear_state->usc_clear_attachment_programs);

   for (uint32_t i = 0, offset_idx = 0;
        i < ARRAY_SIZE(clear_attachment_collection);
        i++) {
      if (!clear_attachment_collection[i].info)
         continue;

      memcpy(ptr + usc_program_offsets[offset_idx],
             clear_attachment_collection[i].code,
             clear_attachment_collection[i].size);

      offset_idx++;
   }

   /* Upload PDS programs. */

   alloc_size = 0;

   for (uint32_t i = 0, offset_idx = 0;
        i < ARRAY_SIZE(clear_attachment_collection);
        i++) {
      struct pvr_pds_pixel_shader_sa_program texture_pds_program;
      struct pvr_pds_kickusc_program pixel_shader_pds_program;
      uint32_t program_size;

      if (!clear_attachment_collection[i].info)
         continue;

      /* Texture program to load colors. */

      texture_pds_program = (struct pvr_pds_pixel_shader_sa_program){
         .num_texture_dma_kicks = 1,
      };

      pvr_pds_set_sizes_pixel_shader_uniform_texture_code(&texture_pds_program);

      pds_texture_program_offsets[offset_idx] = alloc_size;
      alloc_size += ALIGN_POT(PVR_DW_TO_BYTES(texture_pds_program.code_size),
                              pds_prog_alignment);

      /* Pixel program to load fragment shader. */

      pixel_shader_pds_program = (struct pvr_pds_kickusc_program){ 0 };

      pvr_pds_setup_doutu(&pixel_shader_pds_program.usc_task_control,
                          usc_upload_offset + usc_program_offsets[offset_idx],
                          clear_attachment_collection[i].info->temps_required,
                          PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                          false);

      pvr_pds_set_sizes_pixel_shader(&pixel_shader_pds_program);

      program_size = pixel_shader_pds_program.code_size +
                     pixel_shader_pds_program.data_size;
      program_size = PVR_DW_TO_BYTES(program_size);

      pds_pixel_program_offsets[offset_idx] = alloc_size;
      alloc_size += ALIGN_POT(program_size, pds_prog_alignment);

      offset_idx++;
   }

   result = pvr_bo_suballoc(&device->suballoc_pds,
                            alloc_size,
                            pds_prog_alignment,
                            false,
                            &clear_state->pds_clear_attachment_programs);
   if (result != VK_SUCCESS) {
      pvr_bo_suballoc_free(clear_state->usc_clear_attachment_programs);
      return result;
   }

   pds_upload_offset =
      clear_state->pds_clear_attachment_programs->dev_addr.addr -
      device->heaps.pds_heap->base_addr.addr;
   ptr =
      pvr_bo_suballoc_get_map_addr(clear_state->pds_clear_attachment_programs);

   for (uint32_t i = 0, offset_idx = 0;
        i < ARRAY_SIZE(clear_attachment_collection);
        i++) {
      struct pvr_pds_pixel_shader_sa_program texture_pds_program;
      struct pvr_pds_kickusc_program pixel_shader_pds_program;

      if (!clear_attachment_collection[i].info) {
         clear_state->pds_clear_attachment_program_info[i] =
            (struct pvr_pds_clear_attachment_program_info){ 0 };

         continue;
      }

      /* Texture program to load colors. */

      texture_pds_program = (struct pvr_pds_pixel_shader_sa_program){
         .num_texture_dma_kicks = 1,
      };

      pvr_pds_generate_pixel_shader_sa_code_segment(
         &texture_pds_program,
         (uint32_t *)(ptr + pds_texture_program_offsets[offset_idx]));

      /* Pixel program to load fragment shader. */

      pixel_shader_pds_program = (struct pvr_pds_kickusc_program){ 0 };

      pvr_pds_setup_doutu(&pixel_shader_pds_program.usc_task_control,
                          usc_upload_offset + usc_program_offsets[offset_idx],
                          clear_attachment_collection[i].info->temps_required,
                          PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                          false);

      pvr_pds_generate_pixel_shader_program(
         &pixel_shader_pds_program,
         (uint32_t *)(ptr + pds_pixel_program_offsets[offset_idx]));

      /* Setup the PDS program info. */

      pvr_pds_set_sizes_pixel_shader_sa_texture_data(&texture_pds_program,
                                                     dev_info);

      clear_state->pds_clear_attachment_program_info[i] =
         (struct pvr_pds_clear_attachment_program_info){
            .texture_program_offset = PVR_DEV_ADDR(
               pds_upload_offset + pds_texture_program_offsets[offset_idx]),
            .pixel_program_offset = PVR_DEV_ADDR(
               pds_upload_offset + pds_pixel_program_offsets[offset_idx]),

            .texture_program_pds_temps_count = texture_pds_program.temps_used,
            .texture_program_data_size = texture_pds_program.data_size,
         };

      offset_idx++;
   }

   return VK_SUCCESS;
}

static void
pvr_device_finish_clear_attachment_programs(struct pvr_device *device)
{
   struct pvr_device_static_clear_state *clear_state =
      &device->static_clear_state;

   pvr_bo_suballoc_free(clear_state->usc_clear_attachment_programs);
   pvr_bo_suballoc_free(clear_state->pds_clear_attachment_programs);
}

/**
 * \brief Generate and uploads vertices required to clear the rect area.
 *
 * We use the triangle strip topology for clears so this functions generates 4
 * vertices to represent the rect. Note that the coordinates are in screen space
 * and not NDC.
 *
 * \param[in]  device      Device to upload to.
 * \param[in]  rect        Area to clear.
 * \param[in]  depth       Depth (i.e. Z coordinate) of the area to clear.
 * \param[out] pvr_bo_out  BO upload object.
 * \return VK_SUCCESS if the upload succeeded.
 */
VkResult pvr_clear_vertices_upload(struct pvr_device *device,
                                   const VkRect2D *rect,
                                   float depth,
                                   struct pvr_suballoc_bo **const pvr_bo_out)
{
   const float y1 = (float)(rect->offset.y + rect->extent.height);
   const float x1 = (float)(rect->offset.x + rect->extent.width);
   const float y0 = (float)rect->offset.y;
   const float x0 = (float)rect->offset.x;

   const float vertices[PVR_CLEAR_VERTEX_COUNT][PVR_CLEAR_VERTEX_COORDINATES] = {
      [0] = { [0] = x0, [1] = y0, [2] = depth },
      [1] = { [0] = x0, [1] = y1, [2] = depth },
      [2] = { [0] = x1, [1] = y0, [2] = depth },
      [3] = { [0] = x1, [1] = y1, [2] = depth }
   };

   return pvr_gpu_upload(device,
                         device->heaps.general_heap,
                         vertices,
                         sizeof(vertices),
                         4,
                         pvr_bo_out);
}

VkResult pvr_device_init_graphics_static_clear_state(struct pvr_device *device)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   const VkRect2D vf_rect = {
      .offset = { .x = 0, .y = 0 },
      .extent = { .width = rogue_get_param_vf_max_x(dev_info),
                  .height = rogue_get_param_vf_max_y(dev_info) }
   };

   const uint32_t vdm_state_size_in_dw =
      pvr_clear_vdm_state_get_size_in_dw(dev_info, 1);
   struct pvr_device_static_clear_state *state = &device->static_clear_state;
   const uint32_t cache_line_size = rogue_get_slc_cache_line_size(dev_info);
   struct pvr_pds_vertex_shader_program pds_program;
   struct util_dynarray passthrough_vert_shader;
   uint32_t *state_buffer;
   VkResult result;

   if (PVR_HAS_FEATURE(dev_info, gs_rta_support)) {
      struct util_dynarray passthrough_rta_vert_shader;

      util_dynarray_init(&passthrough_rta_vert_shader, NULL);
      pvr_hard_code_get_passthrough_rta_vertex_shader(
         dev_info,
         &passthrough_rta_vert_shader);

      result = pvr_gpu_upload_usc(device,
                                  passthrough_rta_vert_shader.data,
                                  passthrough_rta_vert_shader.size,
                                  cache_line_size,
                                  &state->usc_multi_layer_vertex_shader_bo);
      if (result != VK_SUCCESS) {
         util_dynarray_fini(&passthrough_rta_vert_shader);
         return result;
      }

      util_dynarray_fini(&passthrough_rta_vert_shader);
   } else {
      state->usc_multi_layer_vertex_shader_bo = NULL;
   }

   util_dynarray_init(&passthrough_vert_shader, NULL);
   pvr_hard_code_get_passthrough_vertex_shader(dev_info,
                                               &passthrough_vert_shader);

   result = pvr_gpu_upload_usc(device,
                               passthrough_vert_shader.data,
                               passthrough_vert_shader.size,
                               cache_line_size,
                               &state->usc_vertex_shader_bo);
   util_dynarray_fini(&passthrough_vert_shader);
   if (result != VK_SUCCESS)
      goto err_free_usc_multi_layer_shader;

   result =
      pvr_clear_vertices_upload(device, &vf_rect, 0.0f, &state->vertices_bo);
   if (result != VK_SUCCESS)
      goto err_free_usc_shader;

   pvr_pds_clear_vertex_shader_program_init_base(&pds_program,
                                                 state->usc_vertex_shader_bo);

   result =
      pvr_pds_clear_vertex_shader_program_create_and_upload(&pds_program,
                                                            device,
                                                            state->vertices_bo,
                                                            &state->pds);
   if (result != VK_SUCCESS)
      goto err_free_vertices_buffer;

   pvr_device_setup_graphics_static_clear_ppp_base(&state->ppp_base);
   pvr_device_setup_graphics_static_clear_ppp_templates(state->ppp_templates);

   assert(pds_program.code_size <= state->pds.code_size);

   state_buffer = vk_alloc(&device->vk.alloc,
                           PVR_DW_TO_BYTES(vdm_state_size_in_dw * 2),
                           8,
                           VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (state_buffer == NULL) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_free_pds_program;
   }

   /* TODO: The difference between the large and normal words is only the last
    * word. The value is 3 or 4 depending on the amount of indices. Should we
    * dedup this?
    */

   /* The large clear state words cover the max framebuffer. The normal clear
    * state words cover only half (since 3 indices are passed, forming a single
    * triangle, instead of 4) and are used when the render area fits within a
    * quarter of the max framebuffer, i.e. fit within the single triangle.
    */
   /* 4 * sizeof(uint32_t) because of the 4 pixel output regs. */
   /* TODO: Replace 4 * sizeof(uint32_t) with a defines from the compiler or
    * hook up the value directly to it using some compiler info.
    */
   pvr_pack_clear_vdm_state(&device->pdevice->dev_info,
                            &state->pds,
                            pds_program.temps_used,
                            3,
                            4 * sizeof(uint32_t),
                            1,
                            state_buffer);
   state->vdm_words = state_buffer;
   state_buffer += vdm_state_size_in_dw;

   pvr_pack_clear_vdm_state(&device->pdevice->dev_info,
                            &state->pds,
                            pds_program.temps_used,
                            4,
                            4 * sizeof(uint32_t),
                            1,
                            state_buffer);
   state->large_clear_vdm_words = state_buffer;

   result = pvr_device_init_clear_attachment_programs(device);
   if (result != VK_SUCCESS)
      goto err_free_vdm_state;

   return VK_SUCCESS;

err_free_vdm_state:
   /* Cast away the const :( */
   vk_free(&device->vk.alloc, (void *)state->vdm_words);

err_free_pds_program:
   pvr_bo_suballoc_free(state->pds.pvr_bo);

err_free_vertices_buffer:
   pvr_bo_suballoc_free(state->vertices_bo);

err_free_usc_shader:
   pvr_bo_suballoc_free(state->usc_vertex_shader_bo);

err_free_usc_multi_layer_shader:
   pvr_bo_suballoc_free(state->usc_multi_layer_vertex_shader_bo);

   return result;
}

void pvr_device_finish_graphics_static_clear_state(struct pvr_device *device)
{
   struct pvr_device_static_clear_state *state = &device->static_clear_state;

   pvr_device_finish_clear_attachment_programs(device);

   /* Don't free `large_clear_vdm_words` since it was allocated together with
    * `vdm_words`.
    */
   /* Cast away the const :( */
   vk_free(&device->vk.alloc, (void *)state->vdm_words);

   pvr_bo_suballoc_free(state->pds.pvr_bo);
   pvr_bo_suballoc_free(state->vertices_bo);
   pvr_bo_suballoc_free(state->usc_vertex_shader_bo);
   pvr_bo_suballoc_free(state->usc_multi_layer_vertex_shader_bo);
}

void pvr_pds_clear_vertex_shader_program_init_base(
   struct pvr_pds_vertex_shader_program *program,
   const struct pvr_suballoc_bo *usc_shader_bo)
{
   *program = (struct pvr_pds_vertex_shader_program){
      .num_streams = 1,
      .streams = {
         [0] = {
            /* We'll get this from this interface's client when generating the
             * data segment. This will be the address of the vertex buffer.
             */
            .address = 0,
            .stride = PVR_CLEAR_VERTEX_COORDINATES * sizeof(uint32_t),
            .num_elements = 1,
            .elements = {
               [0] = {
                  .size = PVR_CLEAR_VERTEX_COUNT * PVR_CLEAR_VERTEX_COORDINATES,
               },
            },
         },
      },
   };

   pvr_pds_setup_doutu(&program->usc_task_control,
                       usc_shader_bo->dev_addr.addr,
                       0,
                       PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);
}

VkResult pvr_pds_clear_vertex_shader_program_create_and_upload(
   struct pvr_pds_vertex_shader_program *program,
   struct pvr_device *device,
   const struct pvr_suballoc_bo *vertices_bo,
   struct pvr_pds_upload *const upload_out)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   program->streams[0].address = vertices_bo->dev_addr.addr;

   pvr_pds_vertex_shader(program, NULL, PDS_GENERATE_SIZES, dev_info);

   staging_buffer_size =
      PVR_DW_TO_BYTES(program->code_size + program->data_size);

   staging_buffer = vk_alloc(&device->vk.alloc,
                             staging_buffer_size,
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_exit;
   }

   pvr_pds_vertex_shader(program,
                         staging_buffer,
                         PDS_GENERATE_DATA_SEGMENT,
                         dev_info);
   pvr_pds_vertex_shader(program,
                         &staging_buffer[program->data_size],
                         PDS_GENERATE_CODE_SEGMENT,
                         dev_info);

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               &staging_buffer[0],
                               program->data_size,
                               16,
                               &staging_buffer[program->data_size],
                               program->code_size,
                               16,
                               16,
                               upload_out);
   if (result != VK_SUCCESS)
      goto err_free_staging_buffer;

   vk_free(&device->vk.alloc, staging_buffer);
   return VK_SUCCESS;

err_free_staging_buffer:
   vk_free(&device->vk.alloc, staging_buffer);

err_exit:
   *upload_out = (struct pvr_pds_upload){ 0 };
   return result;
}

VkResult pvr_pds_clear_vertex_shader_program_create_and_upload_data(
   struct pvr_pds_vertex_shader_program *program,
   struct pvr_cmd_buffer *cmd_buffer,
   struct pvr_suballoc_bo *vertices_bo,
   struct pvr_pds_upload *const pds_upload_out)
{
   struct pvr_device_info *dev_info = &cmd_buffer->device->pdevice->dev_info;
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   program->streams[0].address = vertices_bo->dev_addr.addr;

   pvr_pds_vertex_shader(program, NULL, PDS_GENERATE_SIZES, dev_info);

   staging_buffer_size = PVR_DW_TO_BYTES(program->data_size);

   staging_buffer = vk_alloc(&cmd_buffer->device->vk.alloc,
                             staging_buffer_size,
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer) {
      *pds_upload_out = (struct pvr_pds_upload){ 0 };

      return vk_command_buffer_set_error(&cmd_buffer->vk,
                                         VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   pvr_pds_vertex_shader(program,
                         staging_buffer,
                         PDS_GENERATE_DATA_SEGMENT,
                         dev_info);

   result = pvr_cmd_buffer_upload_pds(cmd_buffer,
                                      staging_buffer,
                                      program->data_size,
                                      4,
                                      NULL,
                                      0,
                                      0,
                                      4,
                                      pds_upload_out);
   if (result != VK_SUCCESS) {
      vk_free(&cmd_buffer->device->vk.alloc, staging_buffer);

      *pds_upload_out = (struct pvr_pds_upload){ 0 };

      return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
   }

   vk_free(&cmd_buffer->device->vk.alloc, staging_buffer);

   return VK_SUCCESS;
}

void pvr_pds_clear_rta_vertex_shader_program_init_base(
   struct pvr_pds_vertex_shader_program *program,
   const struct pvr_suballoc_bo *usc_shader_bo)
{
   pvr_pds_clear_vertex_shader_program_init_base(program, usc_shader_bo);

   /* We'll set the render target index to be the instance id + base array
    * layer. Since the base array layer can change in between clear rects, we
    * don't set it here and ask for it when generating the code and data
    * section.
    */
   /* This is 3 because the instance id register will follow the xyz coordinate
    * registers in the register file.
    * TODO: Maybe we want this to be hooked up to the compiler?
    */
   program->iterate_instance_id = true;
   program->instance_id_register = 3;
}

VkResult pvr_pds_clear_rta_vertex_shader_program_create_and_upload_code(
   struct pvr_pds_vertex_shader_program *program,
   struct pvr_cmd_buffer *cmd_buffer,
   uint32_t base_array_layer,
   struct pvr_pds_upload *const pds_upload_out)
{
   struct pvr_device_info *dev_info = &cmd_buffer->device->pdevice->dev_info;
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   program->instance_id_modifier = base_array_layer;

   pvr_pds_vertex_shader(program, NULL, PDS_GENERATE_SIZES, dev_info);

   staging_buffer_size = PVR_DW_TO_BYTES(program->code_size);

   staging_buffer = vk_alloc(&cmd_buffer->device->vk.alloc,
                             staging_buffer_size,
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer) {
      *pds_upload_out = (struct pvr_pds_upload){ 0 };

      return vk_command_buffer_set_error(&cmd_buffer->vk,
                                         VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   pvr_pds_vertex_shader(program,
                         staging_buffer,
                         PDS_GENERATE_CODE_SEGMENT,
                         dev_info);

   result = pvr_cmd_buffer_upload_pds(cmd_buffer,
                                      NULL,
                                      0,
                                      0,
                                      staging_buffer,
                                      program->code_size,
                                      4,
                                      4,
                                      pds_upload_out);
   if (result != VK_SUCCESS) {
      vk_free(&cmd_buffer->device->vk.alloc, staging_buffer);

      *pds_upload_out = (struct pvr_pds_upload){ 0 };

      return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
   }

   vk_free(&cmd_buffer->device->vk.alloc, staging_buffer);

   return VK_SUCCESS;
}

/**
 * Pack VDM control stream words for clear.
 *
 * The size of the `state_buffer` provided is expected to point to a buffer of
 * size equal to what is returned by `pvr_clear_vdm_state_get_size_in_dw()`.
 */
void pvr_pack_clear_vdm_state(const struct pvr_device_info *const dev_info,
                              const struct pvr_pds_upload *const program,
                              uint32_t temps,
                              uint32_t index_count,
                              uint32_t vs_output_size_in_bytes,
                              uint32_t layer_count,
                              uint32_t *const state_buffer)
{
   const uint32_t vs_output_size =
      DIV_ROUND_UP(vs_output_size_in_bytes,
                   PVRX(VDMCTRL_VDM_STATE4_VS_OUTPUT_SIZE_UNIT_SIZE));
   const bool needs_instance_count =
      !PVR_HAS_FEATURE(dev_info, gs_rta_support) && layer_count > 1;
   uint32_t *stream = state_buffer;
   uint32_t max_instances;
   uint32_t cam_size;

   /* The layer count should at least be 1. For vkCmdClearAttachment() the spec.
    * guarantees that the layer count is not 0.
    */
   assert(layer_count != 0);

   pvr_calculate_vertex_cam_size(dev_info,
                                 vs_output_size,
                                 true,
                                 &cam_size,
                                 &max_instances);

   pvr_csb_pack (stream, VDMCTRL_VDM_STATE0, state0) {
      state0.vs_data_addr_present = true;
      state0.vs_other_present = true;
      state0.cam_size = cam_size;
      state0.uvs_scratch_size_select =
         PVRX(VDMCTRL_UVS_SCRATCH_SIZE_SELECT_FIVE);
      state0.flatshade_control = PVRX(VDMCTRL_FLATSHADE_CONTROL_VERTEX_0);
   }
   stream += pvr_cmd_length(VDMCTRL_VDM_STATE0);

   pvr_csb_pack (stream, VDMCTRL_VDM_STATE2, state2) {
      state2.vs_pds_data_base_addr = PVR_DEV_ADDR(program->data_offset);
   }
   stream += pvr_cmd_length(VDMCTRL_VDM_STATE2);

   pvr_csb_pack (stream, VDMCTRL_VDM_STATE3, state3) {
      state3.vs_pds_code_base_addr = PVR_DEV_ADDR(program->code_offset);
   }
   stream += pvr_cmd_length(VDMCTRL_VDM_STATE3);

   pvr_csb_pack (stream, VDMCTRL_VDM_STATE4, state4) {
      state4.vs_output_size = vs_output_size;
   }
   stream += pvr_cmd_length(VDMCTRL_VDM_STATE4);

   pvr_csb_pack (stream, VDMCTRL_VDM_STATE5, state5) {
      state5.vs_max_instances = max_instances;
      /* This is the size of the input vertex. The hw manages the USC
       * temporaries separately so we don't need to include them here.
       */
      state5.vs_usc_unified_size =
         DIV_ROUND_UP(PVR_CLEAR_VERTEX_COORDINATES * sizeof(uint32_t),
                      PVRX(VDMCTRL_VDM_STATE5_VS_USC_UNIFIED_SIZE_UNIT_SIZE));
      state5.vs_pds_temp_size =
         DIV_ROUND_UP(temps,
                      PVRX(VDMCTRL_VDM_STATE5_VS_PDS_TEMP_SIZE_UNIT_SIZE));
      state5.vs_pds_data_size =
         DIV_ROUND_UP(PVR_DW_TO_BYTES(program->data_size),
                      PVRX(VDMCTRL_VDM_STATE5_VS_PDS_DATA_SIZE_UNIT_SIZE));
   }
   stream += pvr_cmd_length(VDMCTRL_VDM_STATE5);

   /* TODO: Here we're doing another state update. If emitting directly to the
    * control stream, we don't mark them as separate state updates by setting
    * the relocation mark so we might be wasting a little bit of memory. See if
    * it's worth changing the code to use the relocation mark.
    */

   pvr_csb_pack (stream, VDMCTRL_INDEX_LIST0, index_list0) {
      index_list0.index_count_present = true;
      index_list0.index_instance_count_present = needs_instance_count;
      index_list0.primitive_topology =
         PVRX(VDMCTRL_PRIMITIVE_TOPOLOGY_TRI_STRIP);
   }
   stream += pvr_cmd_length(VDMCTRL_INDEX_LIST0);

   pvr_csb_pack (stream, VDMCTRL_INDEX_LIST2, index_list3) {
      index_list3.index_count = index_count;
   }
   stream += pvr_cmd_length(VDMCTRL_INDEX_LIST2);

   if (needs_instance_count) {
      pvr_csb_pack (stream, VDMCTRL_INDEX_LIST3, index_list3) {
         index_list3.instance_count = layer_count - 1;
      }
      stream += pvr_cmd_length(VDMCTRL_INDEX_LIST3);
   }

   assert((uint64_t)(stream - state_buffer) ==
          pvr_clear_vdm_state_get_size_in_dw(dev_info, layer_count));
}
