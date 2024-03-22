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

#ifndef PVR_CLEAR_H
#define PVR_CLEAR_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "pvr_csb.h"
#include "pvr_device_info.h"
#include "util/macros.h"

#define PVR_CLEAR_VERTEX_COUNT 4
#define PVR_CLEAR_VERTEX_COORDINATES 3

#define PVR_STATIC_CLEAR_PDS_STATE_COUNT          \
   (pvr_cmd_length(TA_STATE_PDS_SHADERBASE) +     \
    pvr_cmd_length(TA_STATE_PDS_TEXUNICODEBASE) + \
    pvr_cmd_length(TA_STATE_PDS_SIZEINFO1) +      \
    pvr_cmd_length(TA_STATE_PDS_SIZEINFO2) +      \
    pvr_cmd_length(TA_STATE_PDS_VARYINGBASE) +    \
    pvr_cmd_length(TA_STATE_PDS_TEXTUREDATABASE))

/* These can be used as offsets within a PVR_STATIC_CLEAR_PDS_STATE_COUNT dwords
 * sized array to get the respective state word.
 *
 * The values are based on the lengths of the state words.
 */
enum pvr_static_clear_ppp_pds_state_type {
   /* Words enabled by pres_pds_state_ptr0. */
   PVR_STATIC_CLEAR_PPP_PDS_TYPE_SHADERBASE = 0,
   PVR_STATIC_CLEAR_PPP_PDS_TYPE_TEXUNICODEBASE = 1,
   PVR_STATIC_CLEAR_PPP_PDS_TYPE_SIZEINFO1 = 2,
   PVR_STATIC_CLEAR_PPP_PDS_TYPE_SIZEINFO2 = 3,

   /* Word enabled by pres_pds_state_ptr1. */
   PVR_STATIC_CLEAR_PPP_PDS_TYPE_VARYINGBASE = 4,

   /* Word enabled by pres_pds_state_ptr2. */
   PVR_STATIC_CLEAR_PPP_PDS_TYPE_TEXTUREDATABASE = 5,
};

static_assert(PVR_STATIC_CLEAR_PPP_PDS_TYPE_TEXTUREDATABASE + 1 ==
                 PVR_STATIC_CLEAR_PDS_STATE_COUNT,
              "pvr_static_clear_ppp_pds_state_type might require fixing.");

#define PVR_STATIC_CLEAR_VARIANT_COUNT (VK_IMAGE_ASPECT_STENCIL_BIT << 1U)

struct pvr_bo;
struct pvr_cmd_buffer;
struct pvr_device;
struct pvr_pds_upload;
struct pvr_pds_vertex_shader_program;

struct pvr_static_clear_ppp_base {
   uint32_t wclamp;
   uint32_t varying_word[3];
   uint32_t ppp_ctrl;
   uint32_t stream_out0;
};

struct pvr_static_clear_ppp_template {
   /* Pre-packed control words. */
   uint32_t header;
   uint32_t ispb;

   bool requires_pds_state;

   /* Configurable control words.
    * These are initialized and can be modified as needed before emitting them.
    */
   struct {
      struct PVRX(TA_STATE_ISPCTL) ispctl;
      struct PVRX(TA_STATE_ISPA) ispa;

      /* In case the template requires_pds_state this needs to be a valid
       * pointer to a pre-packed PDS state before emitting.
       *
       * Note: this is a pointer to an array of const uint32_t and not an array
       * of pointers or a function pointer.
       */
      const uint32_t (*pds_state)[PVR_STATIC_CLEAR_PDS_STATE_COUNT];

      struct PVRX(TA_REGION_CLIP0) region_clip0;
      struct PVRX(TA_REGION_CLIP1) region_clip1;

      struct PVRX(TA_OUTPUT_SEL) output_sel;
   } config;
};

VkResult pvr_device_init_graphics_static_clear_state(struct pvr_device *device);
void pvr_device_finish_graphics_static_clear_state(struct pvr_device *device);

VkResult pvr_emit_ppp_from_template(
   struct pvr_csb *const csb,
   const struct pvr_static_clear_ppp_template *const template,
   struct pvr_suballoc_bo **const pvr_bo_out);

void pvr_pds_clear_vertex_shader_program_init_base(
   struct pvr_pds_vertex_shader_program *program,
   const struct pvr_suballoc_bo *usc_shader_bo);

VkResult pvr_pds_clear_vertex_shader_program_create_and_upload(
   struct pvr_pds_vertex_shader_program *program,
   struct pvr_device *device,
   const struct pvr_suballoc_bo *vertices_bo,
   struct pvr_pds_upload *const upload_out);
VkResult pvr_pds_clear_vertex_shader_program_create_and_upload_data(
   struct pvr_pds_vertex_shader_program *program,
   struct pvr_cmd_buffer *cmd_buffer,
   struct pvr_suballoc_bo *vertices_bo,
   struct pvr_pds_upload *const pds_upload_out);

void pvr_pds_clear_rta_vertex_shader_program_init_base(
   struct pvr_pds_vertex_shader_program *program,
   const struct pvr_suballoc_bo *usc_shader_bo);

/* Each code and data upload function clears the other's fields in the
 * pds_upload_out. So when uploading the code, the data fields will be 0.
 */
VkResult pvr_pds_clear_rta_vertex_shader_program_create_and_upload_code(
   struct pvr_pds_vertex_shader_program *program,
   struct pvr_cmd_buffer *cmd_buffer,
   uint32_t base_array_layer,
   struct pvr_pds_upload *const pds_upload_out);

static inline VkResult
pvr_pds_clear_rta_vertex_shader_program_create_and_upload_data(
   struct pvr_pds_vertex_shader_program *program,
   struct pvr_cmd_buffer *cmd_buffer,
   struct pvr_suballoc_bo *vertices_bo,
   struct pvr_pds_upload *const pds_upload_out)
{
   return pvr_pds_clear_vertex_shader_program_create_and_upload_data(
      program,
      cmd_buffer,
      vertices_bo,
      pds_upload_out);
}

static inline uint32_t
pvr_clear_vdm_state_get_size_in_dw(const struct pvr_device_info *const dev_info,
                                   uint32_t layer_count)
{
   uint32_t size_in_dw =
      pvr_cmd_length(VDMCTRL_VDM_STATE0) + pvr_cmd_length(VDMCTRL_VDM_STATE2) +
      pvr_cmd_length(VDMCTRL_VDM_STATE3) + pvr_cmd_length(VDMCTRL_VDM_STATE4) +
      pvr_cmd_length(VDMCTRL_VDM_STATE5) + pvr_cmd_length(VDMCTRL_INDEX_LIST0) +
      pvr_cmd_length(VDMCTRL_INDEX_LIST2);

   const bool needs_instance_count =
      !PVR_HAS_FEATURE(dev_info, gs_rta_support) && layer_count > 1;

   if (needs_instance_count)
      size_in_dw += pvr_cmd_length(VDMCTRL_INDEX_LIST3);

   return size_in_dw;
}

void pvr_pack_clear_vdm_state(const struct pvr_device_info *const dev_info,
                              const struct pvr_pds_upload *const program,
                              uint32_t temps,
                              uint32_t index_count,
                              uint32_t vs_output_size_in_bytes,
                              uint32_t layer_count,
                              uint32_t *const state_buffer);

VkResult pvr_clear_vertices_upload(struct pvr_device *device,
                                   const VkRect2D *rect,
                                   float depth,
                                   struct pvr_suballoc_bo **const pvr_bo_out);

#endif /* PVR_CLEAR_H */
