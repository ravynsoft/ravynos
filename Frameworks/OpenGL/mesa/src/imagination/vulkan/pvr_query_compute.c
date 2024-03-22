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
#include <string.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_utils.h"
#include "pvr_bo.h"
#include "pvr_formats.h"
#include "pvr_pds.h"
#include "pvr_private.h"
#include "pvr_shader_factory.h"
#include "pvr_static_shaders.h"
#include "pvr_tex_state.h"
#include "pvr_types.h"
#include "vk_alloc.h"
#include "vk_command_pool.h"
#include "vk_util.h"

static inline void pvr_init_primary_compute_pds_program(
   struct pvr_pds_compute_shader_program *program)
{
   pvr_pds_compute_shader_program_init(program);
   program->local_input_regs[0] = 0;
   /* Workgroup id is in reg0. */
   program->work_group_input_regs[0] = 0;
   program->flattened_work_groups = true;
   program->kick_usc = true;
}

static VkResult pvr_create_compute_secondary_prog(
   struct pvr_device *device,
   const struct pvr_shader_factory_info *shader_factory_info,
   struct pvr_compute_query_shader *query_prog)
{
   const size_t size =
      pvr_pds_get_max_descriptor_upload_const_map_size_in_bytes();
   struct pvr_pds_descriptor_program_input sec_pds_program;
   struct pvr_pds_info *info = &query_prog->info;
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   info->entries =
      vk_alloc(&device->vk.alloc, size, 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!info->entries)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   info->entries_size_in_bytes = size;

   sec_pds_program = (struct pvr_pds_descriptor_program_input){
      .buffer_count = 1,
      .buffers = {
         [0] = {
            .buffer_id = 0,
            .source_offset = 0,
            .type = PVR_BUFFER_TYPE_COMPILE_TIME,
            .size_in_dwords = shader_factory_info->const_shared_regs,
            .destination = shader_factory_info->explicit_const_start_offset,
         }
      },
   };

   pvr_pds_generate_descriptor_upload_program(&sec_pds_program, NULL, info);

   staging_buffer_size = info->code_size_in_dwords;

   staging_buffer = vk_alloc(&device->vk.alloc,
                             PVR_DW_TO_BYTES(staging_buffer_size),
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer) {
      vk_free(&device->vk.alloc, info->entries);
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   pvr_pds_generate_descriptor_upload_program(&sec_pds_program,
                                              staging_buffer,
                                              info);

   assert(info->code_size_in_dwords <= staging_buffer_size);

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               NULL,
                               0,
                               0,
                               staging_buffer,
                               info->code_size_in_dwords,
                               16,
                               16,
                               &query_prog->pds_sec_code);
   if (result != VK_SUCCESS) {
      vk_free(&device->vk.alloc, staging_buffer);
      vk_free(&device->vk.alloc, info->entries);
      return result;
   }

   vk_free(&device->vk.alloc, staging_buffer);

   return VK_SUCCESS;
}

static void
pvr_destroy_compute_secondary_prog(struct pvr_device *device,
                                   struct pvr_compute_query_shader *program)
{
   pvr_bo_suballoc_free(program->pds_sec_code.pvr_bo);
   vk_free(&device->vk.alloc, program->info.entries);
}

static VkResult pvr_create_compute_query_program(
   struct pvr_device *device,
   const struct pvr_shader_factory_info *shader_factory_info,
   struct pvr_compute_query_shader *query_prog)
{
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
   struct pvr_pds_compute_shader_program pds_primary_prog;
   VkResult result;

   /* No support for query constant calc program. */
   assert(shader_factory_info->const_calc_prog_inst_bytes == 0);
   /* No support for query coefficient update program. */
   assert(shader_factory_info->coeff_update_prog_start == PVR_INVALID_INST);

   result = pvr_gpu_upload_usc(device,
                               shader_factory_info->shader_code,
                               shader_factory_info->code_size,
                               cache_line_size,
                               &query_prog->usc_bo);
   if (result != VK_SUCCESS)
      return result;

   pvr_init_primary_compute_pds_program(&pds_primary_prog);

   pvr_pds_setup_doutu(&pds_primary_prog.usc_task_control,
                       query_prog->usc_bo->dev_addr.addr,
                       shader_factory_info->temps_required,
                       PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);

   result =
      pvr_pds_compute_shader_create_and_upload(device,
                                               &pds_primary_prog,
                                               &query_prog->pds_prim_code);
   if (result != VK_SUCCESS)
      goto err_free_usc_bo;

   query_prog->primary_data_size_dw = pds_primary_prog.data_size;
   query_prog->primary_num_temps = pds_primary_prog.temps_used;

   result = pvr_create_compute_secondary_prog(device,
                                              shader_factory_info,
                                              query_prog);
   if (result != VK_SUCCESS)
      goto err_free_pds_prim_code_bo;

   return VK_SUCCESS;

err_free_pds_prim_code_bo:
   pvr_bo_suballoc_free(query_prog->pds_prim_code.pvr_bo);

err_free_usc_bo:
   pvr_bo_suballoc_free(query_prog->usc_bo);

   return result;
}

/* TODO: See if we can dedup this with pvr_setup_descriptor_mappings() or
 * pvr_setup_descriptor_mappings().
 */
static VkResult pvr_write_compute_query_pds_data_section(
   struct pvr_cmd_buffer *cmd_buffer,
   const struct pvr_compute_query_shader *query_prog,
   struct pvr_private_compute_pipeline *pipeline)
{
   const struct pvr_pds_info *const info = &query_prog->info;
   struct pvr_suballoc_bo *pvr_bo;
   const uint8_t *entries;
   uint32_t *dword_buffer;
   uint64_t *qword_buffer;
   VkResult result;

   result = pvr_cmd_buffer_alloc_mem(cmd_buffer,
                                     cmd_buffer->device->heaps.pds_heap,
                                     PVR_DW_TO_BYTES(info->data_size_in_dwords),
                                     &pvr_bo);
   if (result != VK_SUCCESS)
      return result;

   dword_buffer = (uint32_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);
   qword_buffer = (uint64_t *)pvr_bo_suballoc_get_map_addr(pvr_bo);

   entries = (uint8_t *)info->entries;

   /* TODO: Remove this when we can test this path and make sure that this is
    * not needed. If it's needed we should probably be using LITERAL entries for
    * this instead.
    */
   memset(dword_buffer, 0xFE, PVR_DW_TO_BYTES(info->data_size_in_dwords));

   pipeline->pds_shared_update_data_size_dw = info->data_size_in_dwords;

   for (uint32_t i = 0; i < info->entry_count; i++) {
      const struct pvr_const_map_entry *const entry_header =
         (struct pvr_const_map_entry *)entries;

      switch (entry_header->type) {
      case PVR_PDS_CONST_MAP_ENTRY_TYPE_LITERAL32: {
         const struct pvr_const_map_entry_literal32 *const literal =
            (struct pvr_const_map_entry_literal32 *)entries;

         PVR_WRITE(dword_buffer,
                   literal->literal_value,
                   literal->const_offset,
                   info->data_size_in_dwords);

         entries += sizeof(*literal);
         break;
      }
      case PVR_PDS_CONST_MAP_ENTRY_TYPE_LITERAL64: {
         const struct pvr_const_map_entry_literal64 *const literal =
            (struct pvr_const_map_entry_literal64 *)entries;

         PVR_WRITE(qword_buffer,
                   literal->literal_value,
                   literal->const_offset,
                   info->data_size_in_dwords);

         entries += sizeof(*literal);
         break;
      }
      case PVR_PDS_CONST_MAP_ENTRY_TYPE_DOUTU_ADDRESS: {
         const struct pvr_const_map_entry_doutu_address *const doutu_addr =
            (struct pvr_const_map_entry_doutu_address *)entries;
         const pvr_dev_addr_t exec_addr =
            PVR_DEV_ADDR_OFFSET(query_prog->pds_sec_code.pvr_bo->dev_addr,
                                query_prog->pds_sec_code.code_offset);
         uint64_t addr = 0ULL;

         pvr_set_usc_execution_address64(&addr, exec_addr.addr);

         PVR_WRITE(qword_buffer,
                   addr | doutu_addr->doutu_control,
                   doutu_addr->const_offset,
                   info->data_size_in_dwords);

         entries += sizeof(*doutu_addr);
         break;
      }
      case PVR_PDS_CONST_MAP_ENTRY_TYPE_SPECIAL_BUFFER: {
         const struct pvr_const_map_entry_special_buffer *special_buff_entry =
            (struct pvr_const_map_entry_special_buffer *)entries;

         switch (special_buff_entry->buffer_type) {
         case PVR_BUFFER_TYPE_COMPILE_TIME: {
            uint64_t addr = pipeline->const_buffer_addr.addr;

            PVR_WRITE(qword_buffer,
                      addr,
                      special_buff_entry->const_offset,
                      info->data_size_in_dwords);
            break;
         }

         default:
            unreachable("Unsupported special buffer type.");
         }

         entries += sizeof(*special_buff_entry);
         break;
      }
      default:
         unreachable("Unsupported data section map");
      }
   }

   pipeline->pds_shared_update_data_offset =
      pvr_bo->dev_addr.addr -
      cmd_buffer->device->heaps.pds_heap->base_addr.addr;

   return VK_SUCCESS;
}

static void pvr_write_private_compute_dispatch(
   struct pvr_cmd_buffer *cmd_buffer,
   struct pvr_private_compute_pipeline *pipeline,
   uint32_t num_query_indices)
{
   struct pvr_sub_cmd *sub_cmd = cmd_buffer->state.current_sub_cmd;
   const uint32_t workgroup_size[PVR_WORKGROUP_DIMENSIONS] = {
      DIV_ROUND_UP(num_query_indices, 32),
      1,
      1,
   };

   assert(sub_cmd->type == PVR_SUB_CMD_TYPE_OCCLUSION_QUERY);

   pvr_compute_update_shared_private(cmd_buffer, &sub_cmd->compute, pipeline);
   pvr_compute_update_kernel_private(cmd_buffer,
                                     &sub_cmd->compute,
                                     pipeline,
                                     workgroup_size);
   pvr_compute_generate_fence(cmd_buffer, &sub_cmd->compute, false);
}

static void
pvr_destroy_compute_query_program(struct pvr_device *device,
                                  struct pvr_compute_query_shader *program)
{
   pvr_destroy_compute_secondary_prog(device, program);
   pvr_bo_suballoc_free(program->pds_prim_code.pvr_bo);
   pvr_bo_suballoc_free(program->usc_bo);
}

static VkResult pvr_create_multibuffer_compute_query_program(
   struct pvr_device *device,
   const struct pvr_shader_factory_info *const *shader_factory_info,
   struct pvr_compute_query_shader *query_programs)
{
   const uint32_t core_count = device->pdevice->dev_runtime_info.core_count;
   VkResult result;
   uint32_t i;

   for (i = 0; i < core_count; i++) {
      result = pvr_create_compute_query_program(device,
                                                shader_factory_info[i],
                                                &query_programs[i]);
      if (result != VK_SUCCESS)
         goto err_destroy_compute_query_program;
   }

   return VK_SUCCESS;

err_destroy_compute_query_program:
   for (uint32_t j = 0; j < i; j++)
      pvr_destroy_compute_query_program(device, &query_programs[j]);

   return result;
}

VkResult pvr_device_create_compute_query_programs(struct pvr_device *device)
{
   const uint32_t core_count = device->pdevice->dev_runtime_info.core_count;
   VkResult result;

   result = pvr_create_compute_query_program(device,
                                             &availability_query_write_info,
                                             &device->availability_shader);
   if (result != VK_SUCCESS)
      return result;

   device->copy_results_shaders =
      vk_alloc(&device->vk.alloc,
               sizeof(*device->copy_results_shaders) * core_count,
               8,
               VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!device->copy_results_shaders) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_destroy_availability_query_program;
   }

   result = pvr_create_multibuffer_compute_query_program(
      device,
      copy_query_results_collection,
      device->copy_results_shaders);
   if (result != VK_SUCCESS)
      goto err_vk_free_copy_results_shaders;

   device->reset_queries_shaders =
      vk_alloc(&device->vk.alloc,
               sizeof(*device->reset_queries_shaders) * core_count,
               8,
               VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!device->reset_queries_shaders) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_destroy_copy_results_query_programs;
   }

   result = pvr_create_multibuffer_compute_query_program(
      device,
      reset_query_collection,
      device->reset_queries_shaders);
   if (result != VK_SUCCESS)
      goto err_vk_free_reset_queries_shaders;

   return VK_SUCCESS;

err_vk_free_reset_queries_shaders:
   vk_free(&device->vk.alloc, device->reset_queries_shaders);

err_destroy_copy_results_query_programs:
   for (uint32_t i = 0; i < core_count; i++) {
      pvr_destroy_compute_query_program(device,
                                        &device->copy_results_shaders[i]);
   }

err_vk_free_copy_results_shaders:
   vk_free(&device->vk.alloc, device->copy_results_shaders);

err_destroy_availability_query_program:
   pvr_destroy_compute_query_program(device, &device->availability_shader);

   return result;
}

void pvr_device_destroy_compute_query_programs(struct pvr_device *device)
{
   const uint32_t core_count = device->pdevice->dev_runtime_info.core_count;

   pvr_destroy_compute_query_program(device, &device->availability_shader);

   for (uint32_t i = 0; i < core_count; i++) {
      pvr_destroy_compute_query_program(device,
                                        &device->copy_results_shaders[i]);
      pvr_destroy_compute_query_program(device,
                                        &device->reset_queries_shaders[i]);
   }

   vk_free(&device->vk.alloc, device->copy_results_shaders);
   vk_free(&device->vk.alloc, device->reset_queries_shaders);
}

static void pvr_init_tex_info(const struct pvr_device_info *dev_info,
                              struct pvr_texture_state_info *tex_info,
                              uint32_t width,
                              pvr_dev_addr_t addr)
{
   const uint8_t *swizzle_arr = pvr_get_format_swizzle(tex_info->format);
   bool is_view_1d = !PVR_HAS_FEATURE(dev_info, tpu_extended_integer_lookup) &&
                     !PVR_HAS_FEATURE(dev_info, tpu_image_state_v2);

   *tex_info = (struct pvr_texture_state_info){
      .format = VK_FORMAT_R32_UINT,
      .mem_layout = PVR_MEMLAYOUT_LINEAR,
      .flags = PVR_TEXFLAGS_INDEX_LOOKUP,
      .type = is_view_1d ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_2D,
      .is_cube = false,
      .tex_state_type = PVR_TEXTURE_STATE_SAMPLE,
      .extent = { .width = width, .height = 1, .depth = 0 },
      .array_size = 1,
      .base_level = 0,
      .mip_levels = 1,
      .mipmaps_present = false,
      .sample_count = 1,
      .stride = width,
      .offset = 0,
      .swizzle = { [0] = swizzle_arr[0],
                   [1] = swizzle_arr[1],
                   [2] = swizzle_arr[2],
                   [3] = swizzle_arr[3] },
      .addr = addr,

   };
}

/* TODO: Split this function into per program type functions. */
VkResult pvr_add_query_program(struct pvr_cmd_buffer *cmd_buffer,
                               const struct pvr_query_info *query_info)
{
   struct pvr_device *device = cmd_buffer->device;
   const uint32_t core_count = device->pdevice->dev_runtime_info.core_count;
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   const struct pvr_shader_factory_info *shader_factory_info;
   uint64_t sampler_state[ROGUE_NUM_TEXSTATE_SAMPLER_WORDS];
   const struct pvr_compute_query_shader *query_prog;
   struct pvr_private_compute_pipeline pipeline;
   const uint32_t buffer_count = core_count;
   struct pvr_texture_state_info tex_info;
   uint32_t num_query_indices;
   uint32_t *const_buffer;
   struct pvr_suballoc_bo *pvr_bo;
   VkResult result;

   pvr_csb_pack (&sampler_state[0U], TEXSTATE_SAMPLER, reg) {
      reg.addrmode_u = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
      reg.addrmode_v = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
      reg.addrmode_w = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
      reg.minfilter = PVRX(TEXSTATE_FILTER_POINT);
      reg.magfilter = PVRX(TEXSTATE_FILTER_POINT);
      reg.non_normalized_coords = true;
      reg.dadjust = PVRX(TEXSTATE_DADJUST_ZERO_UINT);
   }

   /* clang-format off */
   pvr_csb_pack (&sampler_state[1], TEXSTATE_SAMPLER_WORD1, sampler_word1) {}
   /* clang-format on */

   switch (query_info->type) {
   case PVR_QUERY_TYPE_AVAILABILITY_WRITE:
      /* Adds a compute shader (fenced on the last 3D) that writes a non-zero
       * value in availability_bo at every index in index_bo.
       */
      query_prog = &device->availability_shader;
      shader_factory_info = &availability_query_write_info;
      num_query_indices = query_info->availability_write.num_query_indices;
      break;

   case PVR_QUERY_TYPE_COPY_QUERY_RESULTS:
      /* Adds a compute shader to copy availability and query value data. */
      query_prog = &device->copy_results_shaders[buffer_count - 1];
      shader_factory_info = copy_query_results_collection[buffer_count - 1];
      num_query_indices = query_info->copy_query_results.query_count;
      break;

   case PVR_QUERY_TYPE_RESET_QUERY_POOL:
      /* Adds a compute shader to reset availability and query value data. */
      query_prog = &device->reset_queries_shaders[buffer_count - 1];
      shader_factory_info = reset_query_collection[buffer_count - 1];
      num_query_indices = query_info->reset_query_pool.query_count;
      break;

   default:
      unreachable("Invalid query type");
   }

   result = pvr_cmd_buffer_start_sub_cmd(cmd_buffer,
                                         PVR_SUB_CMD_TYPE_OCCLUSION_QUERY);
   if (result != VK_SUCCESS)
      return result;

   pipeline.pds_code_offset = query_prog->pds_prim_code.code_offset;
   pipeline.pds_data_offset = query_prog->pds_prim_code.data_offset;

   pipeline.pds_shared_update_code_offset =
      query_prog->pds_sec_code.code_offset;
   pipeline.pds_data_size_dw = query_prog->primary_data_size_dw;
   pipeline.pds_temps_used = query_prog->primary_num_temps;

   pipeline.coeff_regs_count = shader_factory_info->coeff_regs;
   pipeline.unified_store_regs_count = shader_factory_info->input_regs;
   pipeline.const_shared_regs_count = shader_factory_info->const_shared_regs;

   const_buffer =
      vk_alloc(&cmd_buffer->vk.pool->alloc,
               PVR_DW_TO_BYTES(shader_factory_info->const_shared_regs),
               8,
               VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!const_buffer) {
      return vk_command_buffer_set_error(&cmd_buffer->vk,
                                         VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   /* clang-format off */
#define DRIVER_CONST(index)                                            \
   assert(shader_factory_info->driver_const_location_map[index] <      \
          shader_factory_info->const_shared_regs);                     \
   const_buffer[shader_factory_info->driver_const_location_map[index]]
   /* clang-format on */

   switch (query_info->type) {
   case PVR_QUERY_TYPE_AVAILABILITY_WRITE: {
      uint64_t image_sampler_state[3][ROGUE_NUM_TEXSTATE_SAMPLER_WORDS];
      uint32_t image_sampler_idx = 0;

      memcpy(&image_sampler_state[image_sampler_idx][0],
             &sampler_state[0],
             sizeof(sampler_state));
      image_sampler_idx++;

      pvr_init_tex_info(dev_info,
                        &tex_info,
                        num_query_indices,
                        query_info->availability_write.index_bo->dev_addr);

      result = pvr_pack_tex_state(device,
                                  &tex_info,
                                  &image_sampler_state[image_sampler_idx][0]);
      if (result != VK_SUCCESS) {
         vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
      }

      image_sampler_idx++;

      pvr_init_tex_info(
         dev_info,
         &tex_info,
         query_info->availability_write.num_queries,
         query_info->availability_write.availability_bo->dev_addr);

      result = pvr_pack_tex_state(device,
                                  &tex_info,
                                  &image_sampler_state[image_sampler_idx][0]);
      if (result != VK_SUCCESS) {
         vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
      }

      image_sampler_idx++;

      memcpy(&const_buffer[0],
             &image_sampler_state[0][0],
             sizeof(image_sampler_state));

      /* Only PVR_QUERY_AVAILABILITY_WRITE_COUNT driver consts allowed. */
      assert(shader_factory_info->num_driver_consts ==
             PVR_QUERY_AVAILABILITY_WRITE_COUNT);

      DRIVER_CONST(PVR_QUERY_AVAILABILITY_WRITE_INDEX_COUNT) =
         num_query_indices;
      break;
   }

   case PVR_QUERY_TYPE_COPY_QUERY_RESULTS: {
      PVR_FROM_HANDLE(pvr_query_pool,
                      pool,
                      query_info->copy_query_results.query_pool);
      PVR_FROM_HANDLE(pvr_buffer,
                      buffer,
                      query_info->copy_query_results.dst_buffer);
      const uint32_t image_sampler_state_arr_size =
         (buffer_count + 2) * ROGUE_NUM_TEXSTATE_SAMPLER_WORDS;
      uint32_t image_sampler_idx = 0;
      pvr_dev_addr_t addr;
      uint64_t offset;

      STACK_ARRAY(uint64_t, image_sampler_state, image_sampler_state_arr_size);
      if (!image_sampler_state) {
         vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);

         return vk_command_buffer_set_error(&cmd_buffer->vk,
                                            VK_ERROR_OUT_OF_HOST_MEMORY);
      }

#define SAMPLER_ARR_2D(_arr, _i, _j) \
   _arr[_i * ROGUE_NUM_TEXSTATE_SAMPLER_WORDS + _j]

      memcpy(&SAMPLER_ARR_2D(image_sampler_state, image_sampler_idx, 0),
             &sampler_state[0],
             sizeof(sampler_state));
      image_sampler_idx++;

      offset = query_info->copy_query_results.first_query * sizeof(uint32_t);

      addr = PVR_DEV_ADDR_OFFSET(pool->availability_buffer->dev_addr, offset);

      pvr_init_tex_info(dev_info, &tex_info, num_query_indices, addr);

      result = pvr_pack_tex_state(
         device,
         &tex_info,
         &SAMPLER_ARR_2D(image_sampler_state, image_sampler_idx, 0));
      if (result != VK_SUCCESS) {
         vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
      }

      image_sampler_idx++;

      for (uint32_t i = 0; i < buffer_count; i++) {
         addr = PVR_DEV_ADDR_OFFSET(pool->result_buffer->dev_addr,
                                    offset + i * pool->result_stride);

         pvr_init_tex_info(dev_info, &tex_info, num_query_indices, addr);

         result = pvr_pack_tex_state(
            device,
            &tex_info,
            &SAMPLER_ARR_2D(image_sampler_state, image_sampler_idx, 0));
         if (result != VK_SUCCESS) {
            vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);
            return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
         }

         image_sampler_idx++;
      }

      memcpy(&const_buffer[0],
             &SAMPLER_ARR_2D(image_sampler_state, 0, 0),
             image_sampler_state_arr_size * sizeof(image_sampler_state[0]));

      STACK_ARRAY_FINISH(image_sampler_state);

      /* Only PVR_COPY_QUERY_POOL_RESULTS_COUNT driver consts allowed. */
      assert(shader_factory_info->num_driver_consts ==
             PVR_COPY_QUERY_POOL_RESULTS_COUNT);

      /* Assert if no memory is bound to destination buffer. */
      assert(buffer->dev_addr.addr);

      addr = buffer->dev_addr;
      addr.addr += query_info->copy_query_results.dst_offset;
      addr.addr += query_info->copy_query_results.first_query *
                   query_info->copy_query_results.stride;

      DRIVER_CONST(PVR_COPY_QUERY_POOL_RESULTS_INDEX_COUNT) = num_query_indices;
      DRIVER_CONST(PVR_COPY_QUERY_POOL_RESULTS_BASE_ADDRESS_LOW) = addr.addr &
                                                                   0xFFFFFFFF;
      DRIVER_CONST(PVR_COPY_QUERY_POOL_RESULTS_BASE_ADDRESS_HIGH) = addr.addr >>
                                                                    32;
      DRIVER_CONST(PVR_COPY_QUERY_POOL_RESULTS_DEST_STRIDE) =
         query_info->copy_query_results.stride;
      DRIVER_CONST(PVR_COPY_QUERY_POOL_RESULTS_PARTIAL_RESULT_FLAG) =
         query_info->copy_query_results.flags & VK_QUERY_RESULT_PARTIAL_BIT;
      DRIVER_CONST(PVR_COPY_QUERY_POOL_RESULTS_64_BIT_FLAG) =
         query_info->copy_query_results.flags & VK_QUERY_RESULT_64_BIT;
      DRIVER_CONST(PVR_COPY_QUERY_POOL_RESULTS_WITH_AVAILABILITY_FLAG) =
         query_info->copy_query_results.flags &
         VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
      break;
   }

   case PVR_QUERY_TYPE_RESET_QUERY_POOL: {
      PVR_FROM_HANDLE(pvr_query_pool,
                      pool,
                      query_info->reset_query_pool.query_pool);
      const uint32_t image_sampler_state_arr_size =
         (buffer_count + 2) * ROGUE_NUM_TEXSTATE_SAMPLER_WORDS;
      uint32_t image_sampler_idx = 0;
      pvr_dev_addr_t addr;
      uint64_t offset;

      STACK_ARRAY(uint64_t, image_sampler_state, image_sampler_state_arr_size);
      if (!image_sampler_state) {
         vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);

         return vk_command_buffer_set_error(&cmd_buffer->vk,
                                            VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      memcpy(&SAMPLER_ARR_2D(image_sampler_state, image_sampler_idx, 0),
             &sampler_state[0],
             sizeof(sampler_state));
      image_sampler_idx++;

      offset = query_info->reset_query_pool.first_query * sizeof(uint32_t);

      for (uint32_t i = 0; i < buffer_count; i++) {
         addr = PVR_DEV_ADDR_OFFSET(pool->result_buffer->dev_addr,
                                    offset + i * pool->result_stride);

         pvr_init_tex_info(dev_info, &tex_info, num_query_indices, addr);

         result = pvr_pack_tex_state(
            device,
            &tex_info,
            &SAMPLER_ARR_2D(image_sampler_state, image_sampler_idx, 0));
         if (result != VK_SUCCESS) {
            vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);
            return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
         }

         image_sampler_idx++;
      }

      addr = PVR_DEV_ADDR_OFFSET(pool->availability_buffer->dev_addr, offset);

      pvr_init_tex_info(dev_info, &tex_info, num_query_indices, addr);

      result = pvr_pack_tex_state(
         device,
         &tex_info,
         &SAMPLER_ARR_2D(image_sampler_state, image_sampler_idx, 0));
      if (result != VK_SUCCESS) {
         vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);
         return pvr_cmd_buffer_set_error_unwarned(cmd_buffer, result);
      }

      image_sampler_idx++;

#undef SAMPLER_ARR_2D

      memcpy(&const_buffer[0],
             &image_sampler_state[0],
             image_sampler_state_arr_size * sizeof(image_sampler_state[0]));

      STACK_ARRAY_FINISH(image_sampler_state);

      /* Only PVR_RESET_QUERY_POOL_COUNT driver consts allowed. */
      assert(shader_factory_info->num_driver_consts ==
             PVR_RESET_QUERY_POOL_COUNT);

      DRIVER_CONST(PVR_RESET_QUERY_POOL_INDEX_COUNT) = num_query_indices;
      break;
   }

   default:
      unreachable("Invalid query type");
   }

#undef DRIVER_CONST

   for (uint32_t i = 0; i < shader_factory_info->num_static_const; i++) {
      const struct pvr_static_buffer *load =
         &shader_factory_info->static_const_buffer[i];

      /* Assert if static const is out of range. */
      assert(load->dst_idx < shader_factory_info->const_shared_regs);
      const_buffer[load->dst_idx] = load->value;
   }

   result = pvr_cmd_buffer_upload_general(
      cmd_buffer,
      const_buffer,
      PVR_DW_TO_BYTES(shader_factory_info->const_shared_regs),
      &pvr_bo);
   if (result != VK_SUCCESS) {
      vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);

      return result;
   }

   pipeline.const_buffer_addr = pvr_bo->dev_addr;

   vk_free(&cmd_buffer->vk.pool->alloc, const_buffer);

   /* PDS data section for the secondary/constant upload. */
   result = pvr_write_compute_query_pds_data_section(cmd_buffer,
                                                     query_prog,
                                                     &pipeline);
   if (result != VK_SUCCESS)
      return result;

   pipeline.workgroup_size.width = ROGUE_MAX_INSTANCES_PER_TASK;
   pipeline.workgroup_size.height = 1;
   pipeline.workgroup_size.depth = 1;

   pvr_write_private_compute_dispatch(cmd_buffer, &pipeline, num_query_indices);

   return pvr_cmd_buffer_end_sub_cmd(cmd_buffer);
}
