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
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_utils.h"
#include "pvr_bo.h"
#include "pvr_cdm_load_sr.h"
#include "pvr_common.h"
#include "pvr_csb.h"
#include "pvr_job_context.h"
#include "pvr_pds.h"
#include "pvr_private.h"
#include "pvr_transfer_frag_store.h"
#include "pvr_types.h"
#include "pvr_uscgen.h"
#include "pvr_vdm_load_sr.h"
#include "pvr_vdm_store_sr.h"
#include "pvr_winsys.h"
#include "util/macros.h"
#include "util/os_file.h"
#include "util/u_dynarray.h"
#include "vk_alloc.h"
#include "vk_log.h"

/* TODO: Is there some way to ensure the Vulkan driver doesn't exceed this
 * value when constructing the control stream?
 */
/* The VDM callstack is used by the hardware to implement control stream links
 * with a return, i.e. sub-control streams/subroutines. This value specifies the
 * maximum callstack depth.
 */
#define PVR_VDM_CALLSTACK_MAX_DEPTH 1U

#define ROGUE_PDS_TASK_PROGRAM_SIZE 256U

static VkResult pvr_ctx_reset_cmd_init(struct pvr_device *device,
                                       struct pvr_reset_cmd *const reset_cmd)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;

   /* The reset framework depends on compute support in the hw. */
   assert(PVR_HAS_FEATURE(dev_info, compute));

   if (PVR_HAS_QUIRK(dev_info, 51764))
      pvr_finishme("Missing reset support for brn51764");

   if (PVR_HAS_QUIRK(dev_info, 58839))
      pvr_finishme("Missing reset support for brn58839");

   return VK_SUCCESS;
}

static void pvr_ctx_reset_cmd_fini(struct pvr_device *device,
                                   struct pvr_reset_cmd *reset_cmd)

{
   /* TODO: reset command cleanup. */
}

static VkResult pvr_pds_pt_store_program_create_and_upload(
   struct pvr_device *device,
   struct pvr_bo *pt_bo,
   uint32_t pt_bo_size,
   struct pvr_pds_upload *const pds_upload_out)
{
   struct pvr_pds_stream_out_terminate_program program = { 0 };
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   const uint32_t cache_line_size = rogue_get_slc_cache_line_size(dev_info);
   size_t staging_buffer_size;
   uint32_t *staging_buffer;
   uint32_t *data_buffer;
   uint32_t *code_buffer;
   VkResult result;

   /* Check the bo size can be converted to dwords without any rounding. */
   assert(pt_bo_size % 4 == 0);

   program.pds_persistent_temp_size_to_store = pt_bo_size / 4;
   program.dev_address_for_storing_persistent_temp = pt_bo->vma->dev_addr.addr;

   pvr_pds_generate_stream_out_terminate_program(&program,
                                                 NULL,
                                                 PDS_GENERATE_SIZES,
                                                 dev_info);

   staging_buffer_size = (program.stream_out_terminate_pds_data_size +
                          program.stream_out_terminate_pds_code_size) *
                         sizeof(*staging_buffer);

   staging_buffer = vk_zalloc(&device->vk.alloc,
                              staging_buffer_size,
                              8,
                              VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   data_buffer = staging_buffer;
   code_buffer =
      pvr_pds_generate_stream_out_terminate_program(&program,
                                                    data_buffer,
                                                    PDS_GENERATE_DATA_SEGMENT,
                                                    dev_info);
   pvr_pds_generate_stream_out_terminate_program(&program,
                                                 code_buffer,
                                                 PDS_GENERATE_CODE_SEGMENT,
                                                 dev_info);

   /* This PDS program is passed to the HW via the PPP state words. These only
    * allow the data segment address to be specified and expect the code
    * segment to immediately follow. Assume the code alignment is the same as
    * the data.
    */
   result =
      pvr_gpu_upload_pds(device,
                         data_buffer,
                         program.stream_out_terminate_pds_data_size,
                         PVRX(TA_STATE_STREAM_OUT1_PDS_DATA_SIZE_UNIT_SIZE),
                         code_buffer,
                         program.stream_out_terminate_pds_code_size,
                         PVRX(TA_STATE_STREAM_OUT1_PDS_DATA_SIZE_UNIT_SIZE),
                         cache_line_size,
                         pds_upload_out);

   vk_free(&device->vk.alloc, staging_buffer);

   return result;
}

static VkResult pvr_pds_pt_resume_program_create_and_upload(
   struct pvr_device *device,
   struct pvr_bo *pt_bo,
   uint32_t pt_bo_size,
   struct pvr_pds_upload *const pds_upload_out)
{
   struct pvr_pds_stream_out_init_program program = { 0 };
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   const uint32_t cache_line_size = rogue_get_slc_cache_line_size(dev_info);
   size_t staging_buffer_size;
   uint32_t *staging_buffer;
   uint32_t *data_buffer;
   uint32_t *code_buffer;
   VkResult result;

   /* Check the bo size can be converted to dwords without any rounding. */
   assert(pt_bo_size % 4 == 0);

   program.num_buffers = 1;
   program.pds_buffer_data_size[0] = pt_bo_size / 4;
   program.dev_address_for_buffer_data[0] = pt_bo->vma->dev_addr.addr;

   pvr_pds_generate_stream_out_init_program(&program,
                                            NULL,
                                            false,
                                            PDS_GENERATE_SIZES,
                                            dev_info);

   staging_buffer_size = (program.stream_out_init_pds_data_size +
                          program.stream_out_init_pds_code_size) *
                         sizeof(*staging_buffer);

   staging_buffer = vk_zalloc(&device->vk.alloc,
                              staging_buffer_size,
                              8,
                              VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   data_buffer = staging_buffer;
   code_buffer =
      pvr_pds_generate_stream_out_init_program(&program,
                                               data_buffer,
                                               false,
                                               PDS_GENERATE_DATA_SEGMENT,
                                               dev_info);
   pvr_pds_generate_stream_out_init_program(&program,
                                            code_buffer,
                                            false,
                                            PDS_GENERATE_CODE_SEGMENT,
                                            dev_info);

   /* This PDS program is passed to the HW via the PPP state words. These only
    * allow the data segment address to be specified and expect the code
    * segment to immediately follow. Assume the code alignment is the same as
    * the data.
    */
   result =
      pvr_gpu_upload_pds(device,
                         data_buffer,
                         program.stream_out_init_pds_data_size,
                         PVRX(TA_STATE_STREAM_OUT1_PDS_DATA_SIZE_UNIT_SIZE),
                         code_buffer,
                         program.stream_out_init_pds_code_size,
                         PVRX(TA_STATE_STREAM_OUT1_PDS_DATA_SIZE_UNIT_SIZE),
                         cache_line_size,
                         pds_upload_out);

   vk_free(&device->vk.alloc, staging_buffer);

   return result;
}

static VkResult
pvr_render_job_pt_programs_setup(struct pvr_device *device,
                                 struct rogue_pt_programs *pt_programs)
{
   VkResult result;

   result = pvr_bo_alloc(device,
                         device->heaps.pds_heap,
                         ROGUE_LLS_PDS_PERSISTENT_TEMPS_BUFFER_SIZE,
                         ROGUE_LLS_PDS_PERSISTENT_TEMPS_BUFFER_ALIGNMENT,
                         PVR_BO_ALLOC_FLAG_CPU_ACCESS,
                         &pt_programs->store_resume_state_bo);
   if (result != VK_SUCCESS)
      return result;

   result = pvr_pds_pt_store_program_create_and_upload(
      device,
      pt_programs->store_resume_state_bo,
      ROGUE_LLS_PDS_PERSISTENT_TEMPS_BUFFER_SIZE,
      &pt_programs->pds_store_program);
   if (result != VK_SUCCESS)
      goto err_free_store_resume_state_bo;

   result = pvr_pds_pt_resume_program_create_and_upload(
      device,
      pt_programs->store_resume_state_bo,
      ROGUE_LLS_PDS_PERSISTENT_TEMPS_BUFFER_SIZE,
      &pt_programs->pds_resume_program);
   if (result != VK_SUCCESS)
      goto err_free_pds_store_program;

   return VK_SUCCESS;

err_free_pds_store_program:
   pvr_bo_suballoc_free(pt_programs->pds_store_program.pvr_bo);

err_free_store_resume_state_bo:
   pvr_bo_free(device, pt_programs->store_resume_state_bo);

   return result;
}

static void
pvr_render_job_pt_programs_cleanup(struct pvr_device *device,
                                   struct rogue_pt_programs *pt_programs)
{
   pvr_bo_suballoc_free(pt_programs->pds_resume_program.pvr_bo);
   pvr_bo_suballoc_free(pt_programs->pds_store_program.pvr_bo);
   pvr_bo_free(device, pt_programs->store_resume_state_bo);
}

static void pvr_pds_ctx_sr_program_setup(
   bool cc_enable,
   uint64_t usc_program_upload_offset,
   uint8_t usc_temps,
   pvr_dev_addr_t sr_addr,
   struct pvr_pds_shared_storing_program *const program_out)
{
   /* The PDS task is the same for stores and loads. */
   *program_out = (struct pvr_pds_shared_storing_program){
		.cc_enable = cc_enable,
		.doutw_control = {
			.dest_store = PDS_UNIFIED_STORE,
			.num_const64 = 2,
			.doutw_data = {
				[0] = sr_addr.addr,
				[1] = sr_addr.addr + ROGUE_LLS_SHARED_REGS_RESERVE_SIZE,
			},
			.last_instruction = false,
		},
	};

   pvr_pds_setup_doutu(&program_out->usc_task.usc_task_control,
                       usc_program_upload_offset,
                       usc_temps,
                       PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);
}

/* Note: pvr_pds_compute_ctx_sr_program_create_and_upload() is very similar to
 * this. If there is a problem here it's likely that the same problem exists
 * there so don't forget to update the compute function.
 */
static VkResult pvr_pds_render_ctx_sr_program_create_and_upload(
   struct pvr_device *device,
   uint64_t usc_program_upload_offset,
   uint8_t usc_temps,
   pvr_dev_addr_t sr_addr,
   struct pvr_pds_upload *const pds_upload_out)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   const uint32_t cache_line_size = rogue_get_slc_cache_line_size(dev_info);
   const uint32_t pds_data_alignment =
      PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE) / 4U;

   /* FIXME: pvr_pds_generate_shared_storing_program() doesn't return the data
    * and code size when using the PDS_GENERATE_SIZES mode.
    */
   STATIC_ASSERT(ROGUE_PDS_TASK_PROGRAM_SIZE % 4 == 0);
   uint32_t staging_buffer[ROGUE_PDS_TASK_PROGRAM_SIZE / 4U] = { 0 };
   struct pvr_pds_shared_storing_program program;
   ASSERTED uint32_t *buffer_end;
   uint32_t code_offset;

   pvr_pds_ctx_sr_program_setup(false,
                                usc_program_upload_offset,
                                usc_temps,
                                sr_addr,
                                &program);

   pvr_pds_generate_shared_storing_program(&program,
                                           &staging_buffer[0],
                                           PDS_GENERATE_DATA_SEGMENT,
                                           dev_info);

   code_offset = ALIGN_POT(program.data_size, pds_data_alignment);

   buffer_end =
      pvr_pds_generate_shared_storing_program(&program,
                                              &staging_buffer[code_offset],
                                              PDS_GENERATE_CODE_SEGMENT,
                                              dev_info);

   assert((uint32_t)(buffer_end - staging_buffer) * sizeof(staging_buffer[0]) <
          ROGUE_PDS_TASK_PROGRAM_SIZE);

   return pvr_gpu_upload_pds(device,
                             &staging_buffer[0],
                             program.data_size,
                             PVRX(VDMCTRL_PDS_STATE1_PDS_DATA_ADDR_ALIGNMENT),
                             &staging_buffer[code_offset],
                             program.code_size,
                             PVRX(VDMCTRL_PDS_STATE2_PDS_CODE_ADDR_ALIGNMENT),
                             cache_line_size,
                             pds_upload_out);
}

/* Note: pvr_pds_render_ctx_sr_program_create_and_upload() is very similar to
 * this. If there is a problem here it's likely that the same problem exists
 * there so don't forget to update the render_ctx function.
 */
static VkResult pvr_pds_compute_ctx_sr_program_create_and_upload(
   struct pvr_device *device,
   bool is_loading_program,
   uint64_t usc_program_upload_offset,
   uint8_t usc_temps,
   pvr_dev_addr_t sr_addr,
   struct pvr_pds_upload *const pds_upload_out)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   const uint32_t cache_line_size = rogue_get_slc_cache_line_size(dev_info);
   const uint32_t pds_data_alignment =
      PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE) / 4U;

   /* FIXME: pvr_pds_generate_shared_storing_program() doesn't return the data
    * and code size when using the PDS_GENERATE_SIZES mode.
    */
   STATIC_ASSERT(ROGUE_PDS_TASK_PROGRAM_SIZE % 4 == 0);
   uint32_t staging_buffer[ROGUE_PDS_TASK_PROGRAM_SIZE / 4U] = { 0 };
   struct pvr_pds_shared_storing_program program;
   uint32_t *buffer_ptr;
   uint32_t code_offset;

   pvr_pds_ctx_sr_program_setup(PVR_HAS_ERN(dev_info, 35421),
                                usc_program_upload_offset,
                                usc_temps,
                                sr_addr,
                                &program);

   if (is_loading_program && PVR_NEED_SW_COMPUTE_PDS_BARRIER(dev_info)) {
      pvr_pds_generate_compute_shared_loading_program(&program,
                                                      &staging_buffer[0],
                                                      PDS_GENERATE_DATA_SEGMENT,
                                                      dev_info);
   } else {
      pvr_pds_generate_shared_storing_program(&program,
                                              &staging_buffer[0],
                                              PDS_GENERATE_DATA_SEGMENT,
                                              dev_info);
   }

   code_offset = ALIGN_POT(program.data_size, pds_data_alignment);

   buffer_ptr =
      pvr_pds_generate_compute_barrier_conditional(&staging_buffer[code_offset],
                                                   PDS_GENERATE_CODE_SEGMENT);

   if (is_loading_program && PVR_NEED_SW_COMPUTE_PDS_BARRIER(dev_info)) {
      buffer_ptr = pvr_pds_generate_compute_shared_loading_program(
         &program,
         buffer_ptr,
         PDS_GENERATE_CODE_SEGMENT,
         dev_info);
   } else {
      buffer_ptr =
         pvr_pds_generate_shared_storing_program(&program,
                                                 buffer_ptr,
                                                 PDS_GENERATE_CODE_SEGMENT,
                                                 dev_info);
   }

   assert((uint32_t)(buffer_ptr - staging_buffer) * sizeof(staging_buffer[0]) <
          ROGUE_PDS_TASK_PROGRAM_SIZE);

   STATIC_ASSERT(PVRX(CR_CDM_CONTEXT_PDS0_DATA_ADDR_ALIGNMENT) ==
                 PVRX(CR_CDM_CONTEXT_LOAD_PDS0_DATA_ADDR_ALIGNMENT));

   STATIC_ASSERT(PVRX(CR_CDM_CONTEXT_PDS0_CODE_ADDR_ALIGNMENT) ==
                 PVRX(CR_CDM_CONTEXT_LOAD_PDS0_CODE_ADDR_ALIGNMENT));

   return pvr_gpu_upload_pds(
      device,
      &staging_buffer[0],
      program.data_size,
      PVRX(CR_CDM_CONTEXT_PDS0_DATA_ADDR_ALIGNMENT),
      &staging_buffer[code_offset],
      (uint32_t)(buffer_ptr - &staging_buffer[code_offset]),
      PVRX(CR_CDM_CONTEXT_PDS0_CODE_ADDR_ALIGNMENT),
      cache_line_size,
      pds_upload_out);
}

enum pvr_ctx_sr_program_target {
   PVR_CTX_SR_RENDER_TARGET,
   PVR_CTX_SR_COMPUTE_TARGET,
};

static VkResult pvr_ctx_sr_programs_setup(struct pvr_device *device,
                                          enum pvr_ctx_sr_program_target target,
                                          struct rogue_sr_programs *sr_programs)
{
   const uint64_t store_load_state_bo_size =
      PVRX(LLS_USC_SHARED_REGS_BUFFER_SIZE) +
      ROGUE_LLS_SHARED_REGS_RESERVE_SIZE;
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   const uint32_t cache_line_size = rogue_get_slc_cache_line_size(dev_info);
   uint64_t usc_store_program_upload_offset;
   uint64_t usc_load_program_upload_offset;
   const uint8_t *usc_load_sr_code;
   uint32_t usc_load_sr_code_size;
   VkResult result;

   /* Note that this is being used for both compute and render ctx. There is no
    * compute equivalent define for the VDMCTRL unit size.
    */
   /* 4 blocks (16 dwords / 64 bytes) in USC to prevent fragmentation. */
   sr_programs->usc.unified_size =
      DIV_ROUND_UP(64, PVRX(VDMCTRL_PDS_STATE0_USC_UNIFIED_SIZE_UNIT_SIZE));

   result = pvr_bo_alloc(device,
                         device->heaps.pds_heap,
                         store_load_state_bo_size,
                         cache_line_size,
                         PVR_WINSYS_BO_FLAG_CPU_ACCESS,
                         &sr_programs->store_load_state_bo);
   if (result != VK_SUCCESS)
      return result;

   /* USC state update: SR state store. */

   assert(sizeof(pvr_vdm_store_sr_code) < ROGUE_USC_TASK_PROGRAM_SIZE);

   result = pvr_gpu_upload_usc(device,
                               pvr_vdm_store_sr_code,
                               sizeof(pvr_vdm_store_sr_code),
                               cache_line_size,
                               &sr_programs->usc.store_program_bo);
   if (result != VK_SUCCESS)
      goto err_free_store_load_state_bo;

   usc_store_program_upload_offset =
      sr_programs->usc.store_program_bo->dev_addr.addr -
      device->heaps.usc_heap->base_addr.addr;

   /* USC state update: SR state load. */

   if (target == PVR_CTX_SR_COMPUTE_TARGET && PVR_HAS_QUIRK(dev_info, 62269)) {
      STATIC_ASSERT(sizeof(pvr_cdm_load_sr_code) < ROGUE_USC_TASK_PROGRAM_SIZE);

      usc_load_sr_code = pvr_cdm_load_sr_code;
      usc_load_sr_code_size = sizeof(pvr_cdm_load_sr_code);
   } else {
      STATIC_ASSERT(sizeof(pvr_vdm_load_sr_code) < ROGUE_USC_TASK_PROGRAM_SIZE);

      usc_load_sr_code = pvr_vdm_load_sr_code;
      usc_load_sr_code_size = sizeof(pvr_vdm_load_sr_code);
   }

   result = pvr_gpu_upload_usc(device,
                               usc_load_sr_code,
                               usc_load_sr_code_size,
                               cache_line_size,
                               &sr_programs->usc.load_program_bo);
   if (result != VK_SUCCESS)
      goto err_free_usc_store_program_bo;

   usc_load_program_upload_offset =
      sr_programs->usc.load_program_bo->dev_addr.addr -
      device->heaps.usc_heap->base_addr.addr;

   /* FIXME: The number of USC temps should be output alongside
    * pvr_vdm_store_sr_code rather than hard coded.
    */
   /* Create and upload the PDS load and store programs. Point them to the
    * appropriate USC load and store programs.
    */
   switch (target) {
   case PVR_CTX_SR_RENDER_TARGET:
      /* PDS state update: SR state store. */
      result = pvr_pds_render_ctx_sr_program_create_and_upload(
         device,
         usc_store_program_upload_offset,
         8,
         sr_programs->store_load_state_bo->vma->dev_addr,
         &sr_programs->pds.store_program);
      if (result != VK_SUCCESS)
         goto err_free_usc_load_program_bo;

      /* PDS state update: SR state load. */
      result = pvr_pds_render_ctx_sr_program_create_and_upload(
         device,
         usc_load_program_upload_offset,
         20,
         sr_programs->store_load_state_bo->vma->dev_addr,
         &sr_programs->pds.load_program);
      if (result != VK_SUCCESS)
         goto err_free_pds_store_program_bo;

      break;

   case PVR_CTX_SR_COMPUTE_TARGET:
      /* PDS state update: SR state store. */
      result = pvr_pds_compute_ctx_sr_program_create_and_upload(
         device,
         false,
         usc_store_program_upload_offset,
         8,
         sr_programs->store_load_state_bo->vma->dev_addr,
         &sr_programs->pds.store_program);
      if (result != VK_SUCCESS)
         goto err_free_usc_load_program_bo;

      /* PDS state update: SR state load. */
      result = pvr_pds_compute_ctx_sr_program_create_and_upload(
         device,
         true,
         usc_load_program_upload_offset,
         20,
         sr_programs->store_load_state_bo->vma->dev_addr,
         &sr_programs->pds.load_program);
      if (result != VK_SUCCESS)
         goto err_free_pds_store_program_bo;

      break;

   default:
      unreachable("Invalid target.");
      break;
   }

   return VK_SUCCESS;

err_free_pds_store_program_bo:
   pvr_bo_suballoc_free(sr_programs->pds.store_program.pvr_bo);

err_free_usc_load_program_bo:
   pvr_bo_suballoc_free(sr_programs->usc.load_program_bo);

err_free_usc_store_program_bo:
   pvr_bo_suballoc_free(sr_programs->usc.store_program_bo);

err_free_store_load_state_bo:
   pvr_bo_free(device, sr_programs->store_load_state_bo);

   return result;
}

static void pvr_ctx_sr_programs_cleanup(struct pvr_device *device,
                                        struct rogue_sr_programs *sr_programs)
{
   pvr_bo_suballoc_free(sr_programs->pds.load_program.pvr_bo);
   pvr_bo_suballoc_free(sr_programs->pds.store_program.pvr_bo);
   pvr_bo_suballoc_free(sr_programs->usc.load_program_bo);
   pvr_bo_suballoc_free(sr_programs->usc.store_program_bo);
   pvr_bo_free(device, sr_programs->store_load_state_bo);
}

static VkResult
pvr_render_ctx_switch_programs_setup(struct pvr_device *device,
                                     struct pvr_render_ctx_programs *programs)
{
   VkResult result;

   result = pvr_render_job_pt_programs_setup(device, &programs->pt);
   if (result != VK_SUCCESS)
      return result;

   result = pvr_ctx_sr_programs_setup(device,
                                      PVR_CTX_SR_RENDER_TARGET,
                                      &programs->sr);
   if (result != VK_SUCCESS)
      goto err_pt_programs_cleanup;

   return VK_SUCCESS;

err_pt_programs_cleanup:
   pvr_render_job_pt_programs_cleanup(device, &programs->pt);

   return result;
}

static void
pvr_render_ctx_switch_programs_cleanup(struct pvr_device *device,
                                       struct pvr_render_ctx_programs *programs)
{
   pvr_ctx_sr_programs_cleanup(device, &programs->sr);
   pvr_render_job_pt_programs_cleanup(device, &programs->pt);
}

static VkResult pvr_render_ctx_switch_init(struct pvr_device *device,
                                           struct pvr_render_ctx *ctx)
{
   struct pvr_render_ctx_switch *ctx_switch = &ctx->ctx_switch;
   const uint64_t vdm_state_bo_flags = PVR_BO_ALLOC_FLAG_GPU_UNCACHED |
                                       PVR_BO_ALLOC_FLAG_CPU_ACCESS;
   const uint64_t geom_state_bo_flags = PVR_BO_ALLOC_FLAG_GPU_UNCACHED |
                                        PVR_BO_ALLOC_FLAG_CPU_ACCESS;
   VkResult result;
   uint32_t i;

   result = pvr_bo_alloc(device,
                         device->heaps.general_heap,
                         ROGUE_LLS_VDM_CONTEXT_RESUME_BUFFER_SIZE,
                         ROGUE_LLS_VDM_CONTEXT_RESUME_BUFFER_ALIGNMENT,
                         vdm_state_bo_flags,
                         &ctx_switch->vdm_state_bo);
   if (result != VK_SUCCESS)
      return result;

   result = pvr_bo_alloc(device,
                         device->heaps.general_heap,
                         ROGUE_LLS_TA_STATE_BUFFER_SIZE,
                         ROGUE_LLS_TA_STATE_BUFFER_ALIGNMENT,
                         geom_state_bo_flags,
                         &ctx_switch->geom_state_bo);
   if (result != VK_SUCCESS)
      goto err_pvr_bo_free_vdm_state_bo;

   for (i = 0; i < ARRAY_SIZE(ctx_switch->programs); i++) {
      result =
         pvr_render_ctx_switch_programs_setup(device, &ctx_switch->programs[i]);
      if (result != VK_SUCCESS)
         goto err_programs_cleanup;
   }

   return VK_SUCCESS;

err_programs_cleanup:
   for (uint32_t j = 0; j < i; j++)
      pvr_render_ctx_switch_programs_cleanup(device, &ctx_switch->programs[j]);

   pvr_bo_free(device, ctx_switch->geom_state_bo);

err_pvr_bo_free_vdm_state_bo:
   pvr_bo_free(device, ctx_switch->vdm_state_bo);

   return result;
}

static void pvr_render_ctx_switch_fini(struct pvr_device *device,
                                       struct pvr_render_ctx *ctx)
{
   struct pvr_render_ctx_switch *ctx_switch = &ctx->ctx_switch;

   for (uint32_t i = 0; i < ARRAY_SIZE(ctx_switch->programs); i++)
      pvr_render_ctx_switch_programs_cleanup(device, &ctx_switch->programs[i]);

   pvr_bo_free(device, ctx_switch->geom_state_bo);
   pvr_bo_free(device, ctx_switch->vdm_state_bo);
}

static void
pvr_rogue_get_vdmctrl_pds_state_words(struct pvr_pds_upload *pds_program,
                                      enum PVRX(VDMCTRL_USC_TARGET) usc_target,
                                      uint8_t usc_unified_size,
                                      uint32_t *const state0_out,
                                      uint32_t *const state1_out)
{
   pvr_csb_pack (state0_out, VDMCTRL_PDS_STATE0, state) {
      /* Convert the data size from dwords to bytes. */
      const uint32_t pds_data_size = PVR_DW_TO_BYTES(pds_program->data_size);

      state.dm_target = PVRX(VDMCTRL_DM_TARGET_VDM);
      state.usc_target = usc_target;
      state.usc_common_size = 0;
      state.usc_unified_size = usc_unified_size;
      state.pds_temp_size = 0;

      assert(pds_data_size % PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE) ==
             0);
      state.pds_data_size =
         pds_data_size / PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE);
   };

   pvr_csb_pack (state1_out, VDMCTRL_PDS_STATE1, state) {
      state.pds_data_addr = PVR_DEV_ADDR(pds_program->data_offset);
      state.sd_type = PVRX(VDMCTRL_SD_TYPE_PDS);
      state.sd_next_type = PVRX(VDMCTRL_SD_TYPE_PDS);
   }
}

static void
pvr_rogue_get_geom_state_stream_out_words(struct pvr_pds_upload *pds_program,
                                          uint32_t *const stream_out1_out,
                                          uint32_t *const stream_out2_out)
{
   pvr_csb_pack (stream_out1_out, TA_STATE_STREAM_OUT1, state) {
      /* Convert the data size from dwords to bytes. */
      const uint32_t pds_data_size = PVR_DW_TO_BYTES(pds_program->data_size);

      state.sync = true;

      assert(pds_data_size %
                PVRX(TA_STATE_STREAM_OUT1_PDS_DATA_SIZE_UNIT_SIZE) ==
             0);
      state.pds_data_size =
         pds_data_size / PVRX(TA_STATE_STREAM_OUT1_PDS_DATA_SIZE_UNIT_SIZE);

      state.pds_temp_size = 0;
   }

   pvr_csb_pack (stream_out2_out, TA_STATE_STREAM_OUT2, state) {
      state.pds_data_addr = PVR_DEV_ADDR(pds_program->data_offset);
   }
}

static void pvr_render_ctx_ws_static_state_init(
   struct pvr_render_ctx *ctx,
   struct pvr_winsys_render_ctx_static_state *static_state)
{
   uint64_t *q_dst;
   uint32_t *d_dst;

   q_dst = &static_state->vdm_ctx_state_base_addr;
   pvr_csb_pack (q_dst, CR_VDM_CONTEXT_STATE_BASE, base) {
      base.addr = ctx->ctx_switch.vdm_state_bo->vma->dev_addr;
   }

   q_dst = &static_state->geom_ctx_state_base_addr;
   pvr_csb_pack (q_dst, CR_TA_CONTEXT_STATE_BASE, base) {
      base.addr = ctx->ctx_switch.geom_state_bo->vma->dev_addr;
   }

   for (uint32_t i = 0; i < ARRAY_SIZE(ctx->ctx_switch.programs); i++) {
      struct rogue_pt_programs *pt_prog = &ctx->ctx_switch.programs[i].pt;
      struct rogue_sr_programs *sr_prog = &ctx->ctx_switch.programs[i].sr;

      /* Context store state. */
      q_dst = &static_state->geom_state[i].vdm_ctx_store_task0;
      pvr_csb_pack (q_dst, CR_VDM_CONTEXT_STORE_TASK0, task0) {
         pvr_rogue_get_vdmctrl_pds_state_words(&sr_prog->pds.store_program,
                                               PVRX(VDMCTRL_USC_TARGET_ANY),
                                               sr_prog->usc.unified_size,
                                               &task0.pds_state0,
                                               &task0.pds_state1);
      }

      d_dst = &static_state->geom_state[i].vdm_ctx_store_task1;
      pvr_csb_pack (d_dst, CR_VDM_CONTEXT_STORE_TASK1, task1) {
         pvr_csb_pack (&task1.pds_state2, VDMCTRL_PDS_STATE2, state) {
            state.pds_code_addr =
               PVR_DEV_ADDR(sr_prog->pds.store_program.code_offset);
         }
      }

      q_dst = &static_state->geom_state[i].vdm_ctx_store_task2;
      pvr_csb_pack (q_dst, CR_VDM_CONTEXT_STORE_TASK2, task2) {
         pvr_rogue_get_geom_state_stream_out_words(&pt_prog->pds_store_program,
                                                   &task2.stream_out1,
                                                   &task2.stream_out2);
      }

      /* Context resume state. */
      q_dst = &static_state->geom_state[i].vdm_ctx_resume_task0;
      pvr_csb_pack (q_dst, CR_VDM_CONTEXT_RESUME_TASK0, task0) {
         pvr_rogue_get_vdmctrl_pds_state_words(&sr_prog->pds.load_program,
                                               PVRX(VDMCTRL_USC_TARGET_ALL),
                                               sr_prog->usc.unified_size,
                                               &task0.pds_state0,
                                               &task0.pds_state1);
      }

      d_dst = &static_state->geom_state[i].vdm_ctx_resume_task1;
      pvr_csb_pack (d_dst, CR_VDM_CONTEXT_RESUME_TASK1, task1) {
         pvr_csb_pack (&task1.pds_state2, VDMCTRL_PDS_STATE2, state) {
            state.pds_code_addr =
               PVR_DEV_ADDR(sr_prog->pds.load_program.code_offset);
         }
      }

      q_dst = &static_state->geom_state[i].vdm_ctx_resume_task2;
      pvr_csb_pack (q_dst, CR_VDM_CONTEXT_RESUME_TASK2, task2) {
         pvr_rogue_get_geom_state_stream_out_words(&pt_prog->pds_resume_program,
                                                   &task2.stream_out1,
                                                   &task2.stream_out2);
      }
   }
}

static void pvr_render_ctx_ws_create_info_init(
   struct pvr_render_ctx *ctx,
   enum pvr_winsys_ctx_priority priority,
   struct pvr_winsys_render_ctx_create_info *create_info)
{
   create_info->priority = priority;
   create_info->vdm_callstack_addr = ctx->vdm_callstack_bo->vma->dev_addr;

   pvr_render_ctx_ws_static_state_init(ctx, &create_info->static_state);
}

VkResult pvr_render_ctx_create(struct pvr_device *device,
                               enum pvr_winsys_ctx_priority priority,
                               struct pvr_render_ctx **const ctx_out)
{
   const uint64_t vdm_callstack_size =
      sizeof(uint64_t) * PVR_VDM_CALLSTACK_MAX_DEPTH;
   struct pvr_winsys_render_ctx_create_info create_info;
   struct pvr_render_ctx *ctx;
   VkResult result;

   ctx = vk_alloc(&device->vk.alloc,
                  sizeof(*ctx),
                  8,
                  VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!ctx)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   ctx->device = device;

   result = pvr_bo_alloc(device,
                         device->heaps.general_heap,
                         vdm_callstack_size,
                         PVRX(CR_VDM_CALL_STACK_POINTER_ADDR_ALIGNMENT),
                         0,
                         &ctx->vdm_callstack_bo);
   if (result != VK_SUCCESS)
      goto err_vk_free_ctx;

   result = pvr_render_ctx_switch_init(device, ctx);
   if (result != VK_SUCCESS)
      goto err_free_vdm_callstack_bo;

   result = pvr_ctx_reset_cmd_init(device, &ctx->reset_cmd);
   if (result != VK_SUCCESS)
      goto err_render_ctx_switch_fini;

   /* ctx must be fully initialized by this point since
    * pvr_render_ctx_ws_create_info_init() depends on this.
    */
   pvr_render_ctx_ws_create_info_init(ctx, priority, &create_info);

   result = device->ws->ops->render_ctx_create(device->ws,
                                               &create_info,
                                               &ctx->ws_ctx);
   if (result != VK_SUCCESS)
      goto err_render_ctx_reset_cmd_fini;

   *ctx_out = ctx;

   return VK_SUCCESS;

err_render_ctx_reset_cmd_fini:
   pvr_ctx_reset_cmd_fini(device, &ctx->reset_cmd);

err_render_ctx_switch_fini:
   pvr_render_ctx_switch_fini(device, ctx);

err_free_vdm_callstack_bo:
   pvr_bo_free(device, ctx->vdm_callstack_bo);

err_vk_free_ctx:
   vk_free(&device->vk.alloc, ctx);

   return result;
}

void pvr_render_ctx_destroy(struct pvr_render_ctx *ctx)
{
   struct pvr_device *device = ctx->device;

   device->ws->ops->render_ctx_destroy(ctx->ws_ctx);

   pvr_ctx_reset_cmd_fini(device, &ctx->reset_cmd);
   pvr_render_ctx_switch_fini(device, ctx);
   pvr_bo_free(device, ctx->vdm_callstack_bo);
   vk_free(&device->vk.alloc, ctx);
}

static VkResult pvr_pds_sr_fence_terminate_program_create_and_upload(
   struct pvr_device *device,
   struct pvr_pds_upload *const pds_upload_out)
{
   const uint32_t pds_data_alignment =
      PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE) / 4U;
   const struct pvr_device_runtime_info *dev_runtime_info =
      &device->pdevice->dev_runtime_info;
   ASSERTED const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   uint32_t staging_buffer[PVRX(PDS_TASK_PROGRAM_SIZE) >> 2U];
   struct pvr_pds_fence_program program = { 0 };
   ASSERTED uint32_t *buffer_end;
   uint32_t code_offset;
   uint32_t data_size;

   /* SW_COMPUTE_PDS_BARRIER is not supported with 2 or more phantoms. */
   assert(!(PVR_NEED_SW_COMPUTE_PDS_BARRIER(dev_info) &&
            dev_runtime_info->num_phantoms >= 2));

   pvr_pds_generate_fence_terminate_program(&program,
                                            staging_buffer,
                                            PDS_GENERATE_DATA_SEGMENT,
                                            &device->pdevice->dev_info);

   /* FIXME: pvr_pds_generate_fence_terminate_program() zeros out the data_size
    * when we generate the code segment. Implement
    * PDS_GENERATE_CODEDATA_SEGMENTS? Or wait for the pds gen api to change?
    * This behavior doesn't seem consistent with the rest of the api. For now
    * we store the size in a variable.
    */
   data_size = program.data_size;
   code_offset = ALIGN_POT(program.data_size, pds_data_alignment);

   buffer_end =
      pvr_pds_generate_fence_terminate_program(&program,
                                               &staging_buffer[code_offset],
                                               PDS_GENERATE_CODE_SEGMENT,
                                               &device->pdevice->dev_info);

   assert((uint64_t)(buffer_end - staging_buffer) * sizeof(staging_buffer[0]) <
          ROGUE_PDS_TASK_PROGRAM_SIZE);

   return pvr_gpu_upload_pds(device,
                             staging_buffer,
                             data_size,
                             PVRX(CR_CDM_TERMINATE_PDS_DATA_ADDR_ALIGNMENT),
                             &staging_buffer[code_offset],
                             program.code_size,
                             PVRX(CR_CDM_TERMINATE_PDS_CODE_ADDR_ALIGNMENT),
                             0,
                             pds_upload_out);
}

static void pvr_compute_ctx_ws_static_state_init(
   const struct pvr_device_info *const dev_info,
   const struct pvr_compute_ctx *const ctx,
   struct pvr_winsys_compute_ctx_static_state *const static_state)
{
   const struct pvr_compute_ctx_switch *const ctx_switch = &ctx->ctx_switch;

   /* CR_CDM_CONTEXT_... use state store program info. */

   pvr_csb_pack (&static_state->cdm_ctx_store_pds0,
                 CR_CDM_CONTEXT_PDS0,
                 state) {
      state.data_addr =
         PVR_DEV_ADDR(ctx_switch->sr[0].pds.store_program.data_offset);
      state.code_addr =
         PVR_DEV_ADDR(ctx_switch->sr[0].pds.store_program.code_offset);
   }

   pvr_csb_pack (&static_state->cdm_ctx_store_pds0_b,
                 CR_CDM_CONTEXT_PDS0,
                 state) {
      state.data_addr =
         PVR_DEV_ADDR(ctx_switch->sr[1].pds.store_program.data_offset);
      state.code_addr =
         PVR_DEV_ADDR(ctx_switch->sr[1].pds.store_program.code_offset);
   }

   pvr_csb_pack (&static_state->cdm_ctx_store_pds1,
                 CR_CDM_CONTEXT_PDS1,
                 state) {
      const uint32_t store_program_data_size =
         PVR_DW_TO_BYTES(ctx_switch->sr[0].pds.store_program.data_size);

      state.pds_seq_dep = true;
      state.usc_seq_dep = false;
      state.target = true;
      state.unified_size = ctx_switch->sr[0].usc.unified_size;
      state.common_shared = false;
      state.common_size = 0;
      state.temp_size = 0;

      assert(store_program_data_size %
                PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE) ==
             0);
      state.data_size = store_program_data_size /
                        PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE);

      state.fence = true;
   }

   /* CR_CDM_TERMINATE_... use fence terminate info. */

   pvr_csb_pack (&static_state->cdm_ctx_terminate_pds,
                 CR_CDM_TERMINATE_PDS,
                 state) {
      state.data_addr =
         PVR_DEV_ADDR(ctx_switch->sr_fence_terminate_program.data_offset);
      state.code_addr =
         PVR_DEV_ADDR(ctx_switch->sr_fence_terminate_program.code_offset);
   }

   pvr_csb_pack (&static_state->cdm_ctx_terminate_pds1,
                 CR_CDM_TERMINATE_PDS1,
                 state) {
      /* Convert the data size from dwords to bytes. */
      const uint32_t fence_terminate_program_data_size =
         PVR_DW_TO_BYTES(ctx_switch->sr_fence_terminate_program.data_size);

      state.pds_seq_dep = true;
      state.usc_seq_dep = false;
      state.target = !PVR_HAS_FEATURE(dev_info, compute_morton_capable);
      state.unified_size = 0;
      /* Common store is for shareds -- this will free the partitions. */
      state.common_shared = true;
      state.common_size = 0;
      state.temp_size = 0;

      assert(fence_terminate_program_data_size %
                PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE) ==
             0);
      state.data_size = fence_terminate_program_data_size /
                        PVRX(VDMCTRL_PDS_STATE0_PDS_DATA_SIZE_UNIT_SIZE);
      state.fence = true;
   }

   /* CR_CDM_RESUME_... use state load program info. */

   pvr_csb_pack (&static_state->cdm_ctx_resume_pds0,
                 CR_CDM_CONTEXT_LOAD_PDS0,
                 state) {
      state.data_addr =
         PVR_DEV_ADDR(ctx_switch->sr[0].pds.load_program.data_offset);
      state.code_addr =
         PVR_DEV_ADDR(ctx_switch->sr[0].pds.load_program.code_offset);
   }

   pvr_csb_pack (&static_state->cdm_ctx_resume_pds0_b,
                 CR_CDM_CONTEXT_LOAD_PDS0,
                 state) {
      state.data_addr =
         PVR_DEV_ADDR(ctx_switch->sr[1].pds.load_program.data_offset);
      state.code_addr =
         PVR_DEV_ADDR(ctx_switch->sr[1].pds.load_program.code_offset);
   }
}

static void pvr_compute_ctx_ws_create_info_init(
   const struct pvr_compute_ctx *const ctx,
   enum pvr_winsys_ctx_priority priority,
   struct pvr_winsys_compute_ctx_create_info *const create_info)
{
   create_info->priority = priority;

   pvr_compute_ctx_ws_static_state_init(&ctx->device->pdevice->dev_info,
                                        ctx,
                                        &create_info->static_state);
}

VkResult pvr_compute_ctx_create(struct pvr_device *const device,
                                enum pvr_winsys_ctx_priority priority,
                                struct pvr_compute_ctx **const ctx_out)
{
   struct pvr_winsys_compute_ctx_create_info create_info;
   struct pvr_compute_ctx *ctx;
   VkResult result;

   ctx = vk_alloc(&device->vk.alloc,
                  sizeof(*ctx),
                  8,
                  VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!ctx)
      return vk_error(device->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   ctx->device = device;

   result = pvr_bo_alloc(
      device,
      device->heaps.general_heap,
      rogue_get_cdm_context_resume_buffer_size(&device->pdevice->dev_info),
      rogue_get_cdm_context_resume_buffer_alignment(&device->pdevice->dev_info),
      PVR_WINSYS_BO_FLAG_CPU_ACCESS | PVR_WINSYS_BO_FLAG_GPU_UNCACHED,
      &ctx->ctx_switch.compute_state_bo);
   if (result != VK_SUCCESS)
      goto err_free_ctx;

   /* TODO: Change this so that enabling storage to B doesn't change the array
    * size. Instead of looping we could unroll this and have the second
    * programs setup depending on the B enable. Doing it that way would make
    * things more obvious.
    */
   for (uint32_t i = 0; i < ARRAY_SIZE(ctx->ctx_switch.sr); i++) {
      result = pvr_ctx_sr_programs_setup(device,
                                         PVR_CTX_SR_COMPUTE_TARGET,
                                         &ctx->ctx_switch.sr[i]);
      if (result != VK_SUCCESS) {
         for (uint32_t j = 0; j < i; j++)
            pvr_ctx_sr_programs_cleanup(device, &ctx->ctx_switch.sr[j]);

         goto err_free_state_buffer;
      }
   }

   result = pvr_pds_sr_fence_terminate_program_create_and_upload(
      device,
      &ctx->ctx_switch.sr_fence_terminate_program);
   if (result != VK_SUCCESS)
      goto err_free_sr_programs;

   pvr_compute_ctx_ws_create_info_init(ctx, priority, &create_info);

   result = pvr_ctx_reset_cmd_init(device, &ctx->reset_cmd);
   if (result != VK_SUCCESS)
      goto err_free_pds_fence_terminate_program;

   result = device->ws->ops->compute_ctx_create(device->ws,
                                                &create_info,
                                                &ctx->ws_ctx);
   if (result != VK_SUCCESS)
      goto err_fini_reset_cmd;

   *ctx_out = ctx;

   return VK_SUCCESS;

err_fini_reset_cmd:
   pvr_ctx_reset_cmd_fini(device, &ctx->reset_cmd);

err_free_pds_fence_terminate_program:
   pvr_bo_suballoc_free(ctx->ctx_switch.sr_fence_terminate_program.pvr_bo);

err_free_sr_programs:
   for (uint32_t i = 0; i < ARRAY_SIZE(ctx->ctx_switch.sr); ++i)
      pvr_ctx_sr_programs_cleanup(device, &ctx->ctx_switch.sr[i]);

err_free_state_buffer:
   pvr_bo_free(device, ctx->ctx_switch.compute_state_bo);

err_free_ctx:
   vk_free(&device->vk.alloc, ctx);

   return result;
}

void pvr_compute_ctx_destroy(struct pvr_compute_ctx *const ctx)
{
   struct pvr_device *device = ctx->device;

   device->ws->ops->compute_ctx_destroy(ctx->ws_ctx);

   pvr_ctx_reset_cmd_fini(device, &ctx->reset_cmd);

   pvr_bo_suballoc_free(ctx->ctx_switch.sr_fence_terminate_program.pvr_bo);
   for (uint32_t i = 0; i < ARRAY_SIZE(ctx->ctx_switch.sr); ++i)
      pvr_ctx_sr_programs_cleanup(device, &ctx->ctx_switch.sr[i]);

   pvr_bo_free(device, ctx->ctx_switch.compute_state_bo);

   vk_free(&device->vk.alloc, ctx);
}

static void pvr_transfer_ctx_ws_create_info_init(
   enum pvr_winsys_ctx_priority priority,
   struct pvr_winsys_transfer_ctx_create_info *const create_info)
{
   create_info->priority = priority;
}

static VkResult pvr_transfer_eot_shaders_init(struct pvr_device *device,
                                              struct pvr_transfer_ctx *ctx)
{
   uint64_t rt_pbe_regs[PVR_TRANSFER_MAX_RENDER_TARGETS];

   /* Setup start indexes of the shared registers that will contain the PBE
    * state words for each render target. These must match the indexes used in
    * pvr_pds_generate_pixel_event(), which is used to generate the
    * corresponding PDS program in pvr_pbe_setup_emit() via
    * pvr_pds_generate_pixel_event_data_segment() and
    * pvr_pds_generate_pixel_event_code_segment().
    */
   /* TODO: store the shared register information somewhere so that it can be
    * shared with pvr_pbe_setup_emit() rather than having the shared register
    * indexes and number of shared registers hard coded in
    * pvr_pds_generate_pixel_event().
    */
   for (uint32_t i = 0; i < ARRAY_SIZE(rt_pbe_regs); i++)
      rt_pbe_regs[i] = i * PVR_STATE_PBE_DWORDS;

   STATIC_ASSERT(ARRAY_SIZE(rt_pbe_regs) == ARRAY_SIZE(ctx->usc_eot_bos));

   for (uint32_t i = 0; i < ARRAY_SIZE(ctx->usc_eot_bos); i++) {
      const uint32_t cache_line_size =
         rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
      const unsigned rt_count = i + 1;
      struct util_dynarray eot_bin;
      VkResult result;

      pvr_uscgen_tq_eot(rt_count, rt_pbe_regs, &eot_bin);

      result = pvr_gpu_upload_usc(device,
                                  util_dynarray_begin(&eot_bin),
                                  eot_bin.size,
                                  cache_line_size,
                                  &ctx->usc_eot_bos[i]);
      util_dynarray_fini(&eot_bin);
      if (result != VK_SUCCESS) {
         for (uint32_t j = 0; j < i; j++)
            pvr_bo_suballoc_free(ctx->usc_eot_bos[j]);

         return result;
      }
   }

   return VK_SUCCESS;
}

static void pvr_transfer_eot_shaders_fini(struct pvr_device *device,
                                          struct pvr_transfer_ctx *ctx)
{
   for (uint32_t i = 0; i < ARRAY_SIZE(ctx->usc_eot_bos); i++)
      pvr_bo_suballoc_free(ctx->usc_eot_bos[i]);
}

static VkResult pvr_transfer_ctx_shaders_init(struct pvr_device *device,
                                              struct pvr_transfer_ctx *ctx)
{
   VkResult result;

   result = pvr_transfer_frag_store_init(device, &ctx->frag_store);
   if (result != VK_SUCCESS)
      goto err_out;

   result = pvr_transfer_eot_shaders_init(device, ctx);
   if (result != VK_SUCCESS)
      goto err_frag_store_fini;

   return VK_SUCCESS;

err_frag_store_fini:
   pvr_transfer_frag_store_fini(device, &ctx->frag_store);

err_out:
   return result;
}

static void pvr_transfer_ctx_shaders_fini(struct pvr_device *device,
                                          struct pvr_transfer_ctx *ctx)
{
   pvr_transfer_eot_shaders_fini(device, ctx);
   pvr_transfer_frag_store_fini(device, &ctx->frag_store);
}

VkResult pvr_transfer_ctx_create(struct pvr_device *const device,
                                 enum pvr_winsys_ctx_priority priority,
                                 struct pvr_transfer_ctx **const ctx_out)
{
   struct pvr_winsys_transfer_ctx_create_info create_info;
   struct pvr_transfer_ctx *ctx;
   VkResult result;

   ctx = vk_zalloc(&device->vk.alloc,
                   sizeof(*ctx),
                   8U,
                   VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!ctx)
      return vk_error(device->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   ctx->device = device;

   result = pvr_ctx_reset_cmd_init(device, &ctx->reset_cmd);
   if (result != VK_SUCCESS)
      goto err_free_ctx;

   pvr_transfer_ctx_ws_create_info_init(priority, &create_info);

   result = device->ws->ops->transfer_ctx_create(device->ws,
                                                 &create_info,
                                                 &ctx->ws_ctx);
   if (result != VK_SUCCESS)
      goto err_fini_reset_cmd;

   result = pvr_transfer_ctx_shaders_init(device, ctx);
   if (result != VK_SUCCESS)
      goto err_destroy_transfer_ctx;

   /* Create the PDS Uniform/Tex state code segment array. */
   for (uint32_t i = 0U; i < ARRAY_SIZE(ctx->pds_unitex_code); i++) {
      for (uint32_t j = 0U; j < ARRAY_SIZE(ctx->pds_unitex_code[0U]); j++) {
         if (i == 0U && j == 0U)
            continue;

         result = pvr_pds_unitex_state_program_create_and_upload(
            device,
            NULL,
            i,
            j,
            &ctx->pds_unitex_code[i][j]);
         if (result != VK_SUCCESS) {
            goto err_free_pds_unitex_bos;
         }
      }
   }

   *ctx_out = ctx;

   return VK_SUCCESS;

err_free_pds_unitex_bos:
   for (uint32_t i = 0U; i < ARRAY_SIZE(ctx->pds_unitex_code); i++) {
      for (uint32_t j = 0U; j < ARRAY_SIZE(ctx->pds_unitex_code[0U]); j++) {
         if (!ctx->pds_unitex_code[i][j].pvr_bo)
            continue;

         pvr_bo_suballoc_free(ctx->pds_unitex_code[i][j].pvr_bo);
      }
   }

   pvr_transfer_ctx_shaders_fini(device, ctx);

err_destroy_transfer_ctx:
   device->ws->ops->transfer_ctx_destroy(ctx->ws_ctx);

err_fini_reset_cmd:
   pvr_ctx_reset_cmd_fini(device, &ctx->reset_cmd);

err_free_ctx:
   vk_free(&device->vk.alloc, ctx);

   return result;
}

void pvr_transfer_ctx_destroy(struct pvr_transfer_ctx *const ctx)
{
   struct pvr_device *device = ctx->device;

   for (uint32_t i = 0U; i < ARRAY_SIZE(ctx->pds_unitex_code); i++) {
      for (uint32_t j = 0U; j < ARRAY_SIZE(ctx->pds_unitex_code[0U]); j++) {
         if (!ctx->pds_unitex_code[i][j].pvr_bo)
            continue;

         pvr_bo_suballoc_free(ctx->pds_unitex_code[i][j].pvr_bo);
      }
   }

   pvr_transfer_ctx_shaders_fini(device, ctx);
   device->ws->ops->transfer_ctx_destroy(ctx->ws_ctx);
   pvr_ctx_reset_cmd_fini(device, &ctx->reset_cmd);
   vk_free(&device->vk.alloc, ctx);
}
