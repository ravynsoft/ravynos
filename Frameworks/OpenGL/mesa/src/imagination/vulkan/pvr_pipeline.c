/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * based in part on v3dv driver which is:
 * Copyright © 2019 Raspberry Pi
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
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "compiler/shader_enums.h"
#include "hwdef/rogue_hw_utils.h"
#include "nir/nir.h"
#include "pvr_bo.h"
#include "pvr_csb.h"
#include "pvr_csb_enum_helpers.h"
#include "pvr_hardcode.h"
#include "pvr_pds.h"
#include "pvr_private.h"
#include "pvr_robustness.h"
#include "pvr_shader.h"
#include "pvr_types.h"
#include "rogue/rogue.h"
#include "util/log.h"
#include "util/macros.h"
#include "util/ralloc.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"
#include "vk_alloc.h"
#include "vk_format.h"
#include "vk_graphics_state.h"
#include "vk_log.h"
#include "vk_object.h"
#include "vk_pipeline_cache.h"
#include "vk_render_pass.h"
#include "vk_util.h"

/*****************************************************************************
   PDS functions
*****************************************************************************/

/* If allocator == NULL, the internal one will be used. */
static VkResult pvr_pds_coeff_program_create_and_upload(
   struct pvr_device *device,
   const VkAllocationCallbacks *allocator,
   const uint32_t *fpu_iterators,
   uint32_t fpu_iterators_count,
   const uint32_t *destinations,
   struct pvr_pds_upload *const pds_upload_out,
   uint32_t *const pds_temps_count_out)
{
   struct pvr_pds_coeff_loading_program program = {
      .num_fpu_iterators = fpu_iterators_count,
   };
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   assert(fpu_iterators_count < PVR_MAXIMUM_ITERATIONS);

   /* Get the size of the program and then allocate that much memory. */
   pvr_pds_coefficient_loading(&program, NULL, PDS_GENERATE_SIZES);

   if (!program.code_size) {
      pds_upload_out->pvr_bo = NULL;
      pds_upload_out->code_size = 0;
      pds_upload_out->data_size = 0;
      *pds_temps_count_out = 0;

      return VK_SUCCESS;
   }

   staging_buffer_size = PVR_DW_TO_BYTES(program.code_size + program.data_size);

   staging_buffer = vk_alloc2(&device->vk.alloc,
                              allocator,
                              staging_buffer_size,
                              8,
                              VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* FIXME: Should we save pointers when we redesign the pds gen api ? */
   typed_memcpy(program.FPU_iterators,
                fpu_iterators,
                program.num_fpu_iterators);

   typed_memcpy(program.destination, destinations, program.num_fpu_iterators);

   /* Generate the program into is the staging_buffer. */
   pvr_pds_coefficient_loading(&program,
                               staging_buffer,
                               PDS_GENERATE_CODEDATA_SEGMENTS);

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               &staging_buffer[0],
                               program.data_size,
                               16,
                               &staging_buffer[program.data_size],
                               program.code_size,
                               16,
                               16,
                               pds_upload_out);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, allocator, staging_buffer);
      return result;
   }

   vk_free2(&device->vk.alloc, allocator, staging_buffer);

   *pds_temps_count_out = program.temps_used;

   return VK_SUCCESS;
}

/* FIXME: move this elsewhere since it's also called in pvr_pass.c? */
/* If allocator == NULL, the internal one will be used. */
VkResult pvr_pds_fragment_program_create_and_upload(
   struct pvr_device *device,
   const VkAllocationCallbacks *allocator,
   const struct pvr_suballoc_bo *fragment_shader_bo,
   uint32_t fragment_temp_count,
   enum rogue_msaa_mode msaa_mode,
   bool has_phase_rate_change,
   struct pvr_pds_upload *const pds_upload_out)
{
   const enum PVRX(PDSINST_DOUTU_SAMPLE_RATE)
      sample_rate = pvr_pdsinst_doutu_sample_rate_from_rogue(msaa_mode);
   struct pvr_pds_kickusc_program program = { 0 };
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   /* FIXME: Should it be passing in the USC offset rather than address here?
    */
   /* Note this is not strictly required to be done before calculating the
    * staging_buffer_size in this particular case. It can also be done after
    * allocating the buffer. The size from pvr_pds_kick_usc() is constant.
    */
   pvr_pds_setup_doutu(&program.usc_task_control,
                       fragment_shader_bo->dev_addr.addr,
                       fragment_temp_count,
                       sample_rate,
                       has_phase_rate_change);

   pvr_pds_kick_usc(&program, NULL, 0, false, PDS_GENERATE_SIZES);

   staging_buffer_size = PVR_DW_TO_BYTES(program.code_size + program.data_size);

   staging_buffer = vk_alloc2(&device->vk.alloc,
                              allocator,
                              staging_buffer_size,
                              8,
                              VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pvr_pds_kick_usc(&program,
                    staging_buffer,
                    0,
                    false,
                    PDS_GENERATE_CODEDATA_SEGMENTS);

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               &staging_buffer[0],
                               program.data_size,
                               16,
                               &staging_buffer[program.data_size],
                               program.code_size,
                               16,
                               16,
                               pds_upload_out);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, allocator, staging_buffer);
      return result;
   }

   vk_free2(&device->vk.alloc, allocator, staging_buffer);

   return VK_SUCCESS;
}

static inline size_t pvr_pds_get_max_vertex_program_const_map_size_in_bytes(
   const struct pvr_device_info *dev_info,
   bool robust_buffer_access)
{
   /* FIXME: Use more local variable to improve formatting. */

   /* Maximum memory allocation needed for const map entries in
    * pvr_pds_generate_vertex_primary_program().
    * When robustBufferAccess is disabled, it must be >= 410.
    * When robustBufferAccess is enabled, it must be >= 570.
    *
    * 1. Size of entry for base instance
    *        (pvr_const_map_entry_base_instance)
    *
    * 2. Max. number of vertex inputs (PVR_MAX_VERTEX_INPUT_BINDINGS) * (
    *     if (!robustBufferAccess)
    *         size of vertex attribute entry
    *             (pvr_const_map_entry_vertex_attribute_address) +
    *     else
    *         size of robust vertex attribute entry
    *             (pvr_const_map_entry_robust_vertex_attribute_address) +
    *         size of entry for max attribute index
    *             (pvr_const_map_entry_vertex_attribute_max_index) +
    *     fi
    *     size of Unified Store burst entry
    *         (pvr_const_map_entry_literal32) +
    *     size of entry for vertex stride
    *         (pvr_const_map_entry_literal32) +
    *     size of entries for DDMAD control word
    *         (num_ddmad_literals * pvr_const_map_entry_literal32))
    *
    * 3. Size of entry for DOUTW vertex/instance control word
    *     (pvr_const_map_entry_literal32)
    *
    * 4. Size of DOUTU entry (pvr_const_map_entry_doutu_address)
    */

   const size_t attribute_size =
      (!robust_buffer_access)
         ? sizeof(struct pvr_const_map_entry_vertex_attribute_address)
         : sizeof(struct pvr_const_map_entry_robust_vertex_attribute_address) +
              sizeof(struct pvr_const_map_entry_vertex_attribute_max_index);

   /* If has_pds_ddmadt the DDMAD control word is now a DDMADT control word
    * and is increased by one DWORD to contain the data for the DDMADT's
    * out-of-bounds check.
    */
   const size_t pvr_pds_const_map_vertex_entry_num_ddmad_literals =
      1U + (size_t)PVR_HAS_FEATURE(dev_info, pds_ddmadt);

   return (sizeof(struct pvr_const_map_entry_base_instance) +
           PVR_MAX_VERTEX_INPUT_BINDINGS *
              (attribute_size +
               (2 + pvr_pds_const_map_vertex_entry_num_ddmad_literals) *
                  sizeof(struct pvr_const_map_entry_literal32)) +
           sizeof(struct pvr_const_map_entry_literal32) +
           sizeof(struct pvr_const_map_entry_doutu_address));
}

/* This is a const pointer to an array of pvr_pds_vertex_dma structs.
 * The array being pointed to is of PVR_MAX_VERTEX_ATTRIB_DMAS size.
 */
typedef struct pvr_pds_vertex_dma (
      *const
         pvr_pds_attrib_dma_descriptions_array_ptr)[PVR_MAX_VERTEX_ATTRIB_DMAS];

/* dma_descriptions_out_ptr is a pointer to the array used as output.
 * The whole array might not be filled so dma_count_out indicates how many
 * elements were used.
 */
static void pvr_pds_vertex_attrib_init_dma_descriptions(
   const VkPipelineVertexInputStateCreateInfo *const vertex_input_state,
   const struct rogue_vs_build_data *vs_data,
   pvr_pds_attrib_dma_descriptions_array_ptr dma_descriptions_out_ptr,
   uint32_t *const dma_count_out)
{
   struct pvr_pds_vertex_dma *const dma_descriptions =
      *dma_descriptions_out_ptr;
   uint32_t dma_count = 0;

   if (!vertex_input_state) {
      *dma_count_out = 0;
      return;
   }

   for (uint32_t i = 0; i < vertex_input_state->vertexAttributeDescriptionCount;
        i++) {
      const VkVertexInputAttributeDescription *const attrib_desc =
         &vertex_input_state->pVertexAttributeDescriptions[i];
      const VkVertexInputBindingDescription *binding_desc = NULL;
      struct pvr_pds_vertex_dma *const dma_desc = &dma_descriptions[dma_count];
      size_t location = attrib_desc->location;

      assert(location < vs_data->inputs.num_input_vars);

      /* Finding the matching binding description. */
      for (uint32_t j = 0;
           j < vertex_input_state->vertexBindingDescriptionCount;
           j++) {
         const VkVertexInputBindingDescription *const current_binding_desc =
            &vertex_input_state->pVertexBindingDescriptions[j];

         if (current_binding_desc->binding == attrib_desc->binding) {
            binding_desc = current_binding_desc;
            break;
         }
      }

      /* From the Vulkan 1.2.195 spec for
       * VkPipelineVertexInputStateCreateInfo:
       *
       *    "For every binding specified by each element of
       *    pVertexAttributeDescriptions, a
       *    VkVertexInputBindingDescription must exist in
       *    pVertexBindingDescriptions with the same value of binding"
       */
      assert(binding_desc);

      dma_desc->offset = attrib_desc->offset;
      dma_desc->stride = binding_desc->stride;

      dma_desc->flags = 0;

      if (binding_desc->inputRate == VK_VERTEX_INPUT_RATE_INSTANCE)
         dma_desc->flags |= PVR_PDS_VERTEX_DMA_FLAGS_INSTANCE_RATE;

      dma_desc->size_in_dwords = vs_data->inputs.components[location];
      /* TODO: This will be different when other types are supported.
       * Store in vs_data with base and components?
       */
      /* TODO: Use attrib_desc->format. */
      dma_desc->component_size_in_bytes = ROGUE_REG_SIZE_BYTES;
      dma_desc->destination = vs_data->inputs.base[location];
      dma_desc->binding_index = attrib_desc->binding;
      dma_desc->divisor = 1;

      dma_desc->robustness_buffer_offset =
         pvr_get_robustness_buffer_format_offset(attrib_desc->format);

      ++dma_count;
   }

   *dma_count_out = dma_count;
}

static VkResult pvr_pds_vertex_attrib_program_create_and_upload(
   struct pvr_device *const device,
   const VkAllocationCallbacks *const allocator,
   struct pvr_pds_vertex_primary_program_input *const input,
   struct pvr_pds_attrib_program *const program_out)
{
   const size_t const_entries_size_in_bytes =
      pvr_pds_get_max_vertex_program_const_map_size_in_bytes(
         &device->pdevice->dev_info,
         device->vk.enabled_features.robustBufferAccess);
   struct pvr_pds_upload *const program = &program_out->program;
   struct pvr_pds_info *const info = &program_out->info;
   struct pvr_const_map_entry *new_entries;
   ASSERTED uint32_t code_size_in_dwords;
   size_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   memset(info, 0, sizeof(*info));

   info->entries = vk_alloc2(&device->vk.alloc,
                             allocator,
                             const_entries_size_in_bytes,
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!info->entries) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_out;
   }

   info->entries_size_in_bytes = const_entries_size_in_bytes;

   pvr_pds_generate_vertex_primary_program(
      input,
      NULL,
      info,
      device->vk.enabled_features.robustBufferAccess,
      &device->pdevice->dev_info);

   code_size_in_dwords = info->code_size_in_dwords;
   staging_buffer_size = PVR_DW_TO_BYTES(info->code_size_in_dwords);

   staging_buffer = vk_alloc2(&device->vk.alloc,
                              allocator,
                              staging_buffer_size,
                              8,
                              VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_free_entries;
   }

   /* This also fills in info->entries. */
   pvr_pds_generate_vertex_primary_program(
      input,
      staging_buffer,
      info,
      device->vk.enabled_features.robustBufferAccess,
      &device->pdevice->dev_info);

   assert(info->code_size_in_dwords <= code_size_in_dwords);

   /* FIXME: Add a vk_realloc2() ? */
   new_entries = vk_realloc((!allocator) ? &device->vk.alloc : allocator,
                            info->entries,
                            info->entries_written_size_in_bytes,
                            8,
                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!new_entries) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_free_staging_buffer;
   }

   info->entries = new_entries;
   info->entries_size_in_bytes = info->entries_written_size_in_bytes;

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               NULL,
                               0,
                               0,
                               staging_buffer,
                               info->code_size_in_dwords,
                               16,
                               16,
                               program);
   if (result != VK_SUCCESS)
      goto err_free_staging_buffer;

   vk_free2(&device->vk.alloc, allocator, staging_buffer);

   return VK_SUCCESS;

err_free_staging_buffer:
   vk_free2(&device->vk.alloc, allocator, staging_buffer);

err_free_entries:
   vk_free2(&device->vk.alloc, allocator, info->entries);

err_out:
   return result;
}

static inline void pvr_pds_vertex_attrib_program_destroy(
   struct pvr_device *const device,
   const struct VkAllocationCallbacks *const allocator,
   struct pvr_pds_attrib_program *const program)
{
   pvr_bo_suballoc_free(program->program.pvr_bo);
   vk_free2(&device->vk.alloc, allocator, program->info.entries);
}

/* This is a const pointer to an array of pvr_pds_attrib_program structs.
 * The array being pointed to is of PVR_PDS_VERTEX_ATTRIB_PROGRAM_COUNT size.
 */
typedef struct pvr_pds_attrib_program (*const pvr_pds_attrib_programs_array_ptr)
   [PVR_PDS_VERTEX_ATTRIB_PROGRAM_COUNT];

/* Indicates that the special variable is unused and has not been allocated a
 * register.
 */
#define PVR_VERTEX_SPECIAL_VAR_UNUSED (-1)

/* Each special variable gets allocated its own vtxin reg if used. */
struct pvr_vertex_special_vars {
   /* VertexIndex built-in. */
   int16_t vertex_id_offset;
   /* InstanceIndex built-in. */
   int16_t instance_id_offset;
};

/* Generate and uploads a PDS program for DMAing vertex attribs into USC vertex
 * inputs. This will bake the code segment and create a template of the data
 * segment for the command buffer to fill in.
 */
/* If allocator == NULL, the internal one will be used.
 *
 * programs_out_ptr is a pointer to the array where the outputs will be placed.
 */
static VkResult pvr_pds_vertex_attrib_programs_create_and_upload(
   struct pvr_device *device,
   const VkAllocationCallbacks *const allocator,
   const VkPipelineVertexInputStateCreateInfo *const vertex_input_state,
   uint32_t usc_temp_count,
   const struct rogue_vs_build_data *vs_data,

   /* Needed for the new path. */
   /* TODO: Remove some of the above once the compiler is hooked up. */
   const struct pvr_pds_vertex_dma
      dma_descriptions[static const PVR_MAX_VERTEX_ATTRIB_DMAS],
   uint32_t dma_count,
   const struct pvr_vertex_special_vars *special_vars_layout,

   pvr_pds_attrib_programs_array_ptr programs_out_ptr)
{
   struct pvr_pds_vertex_dma dma_descriptions_old[PVR_MAX_VERTEX_ATTRIB_DMAS];

   struct pvr_pds_attrib_program *const programs_out = *programs_out_ptr;
   struct pvr_pds_vertex_primary_program_input input = { 0 };
   VkResult result;

   const bool old_path = pvr_has_hard_coded_shaders(&device->pdevice->dev_info);

   if (old_path) {
      pvr_pds_vertex_attrib_init_dma_descriptions(vertex_input_state,
                                                  vs_data,
                                                  &dma_descriptions_old,
                                                  &input.dma_count);

      input.dma_list = dma_descriptions_old;
   } else {
      input.dma_list = dma_descriptions;
      input.dma_count = dma_count;

      if (special_vars_layout->vertex_id_offset !=
          PVR_VERTEX_SPECIAL_VAR_UNUSED) {
         /* Gets filled by the HW and copied into the appropriate reg. */
         input.flags |= PVR_PDS_VERTEX_FLAGS_VERTEX_ID_REQUIRED;
         input.vertex_id_register = special_vars_layout->vertex_id_offset;
      }

      if (special_vars_layout->instance_id_offset !=
          PVR_VERTEX_SPECIAL_VAR_UNUSED) {
         /* Gets filled by the HW and copied into the appropriate reg. */
         input.flags |= PVR_PDS_VERTEX_FLAGS_INSTANCE_ID_REQUIRED;
         input.instance_id_register = special_vars_layout->instance_id_offset;
      }
   }

   pvr_pds_setup_doutu(&input.usc_task_control,
                       0,
                       usc_temp_count,
                       PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);

   /* Note: programs_out_ptr is a pointer to an array so this is fine. See the
    * typedef.
    */
   for (uint32_t i = 0; i < ARRAY_SIZE(*programs_out_ptr); i++) {
      uint32_t extra_flags;

      switch (i) {
      case PVR_PDS_VERTEX_ATTRIB_PROGRAM_BASIC:
         extra_flags = 0;
         break;

      case PVR_PDS_VERTEX_ATTRIB_PROGRAM_BASE_INSTANCE:
         extra_flags = PVR_PDS_VERTEX_FLAGS_BASE_INSTANCE_VARIANT;
         break;

      case PVR_PDS_VERTEX_ATTRIB_PROGRAM_DRAW_INDIRECT:
         extra_flags = PVR_PDS_VERTEX_FLAGS_DRAW_INDIRECT_VARIANT;
         break;

      default:
         unreachable("Invalid vertex attrib program type.");
      }

      input.flags |= extra_flags;

      result =
         pvr_pds_vertex_attrib_program_create_and_upload(device,
                                                         allocator,
                                                         &input,
                                                         &programs_out[i]);
      if (result != VK_SUCCESS) {
         for (uint32_t j = 0; j < i; j++) {
            pvr_pds_vertex_attrib_program_destroy(device,
                                                  allocator,
                                                  &programs_out[j]);
         }

         return result;
      }

      input.flags &= ~extra_flags;
   }

   return VK_SUCCESS;
}

size_t pvr_pds_get_max_descriptor_upload_const_map_size_in_bytes(void)
{
   /* Maximum memory allocation needed for const map entries in
    * pvr_pds_generate_descriptor_upload_program().
    * It must be >= 688 bytes. This size is calculated as the sum of:
    *
    *  1. Max. number of descriptor sets (8) * (
    *         size of descriptor entry
    *             (pvr_const_map_entry_descriptor_set) +
    *         size of Common Store burst entry
    *             (pvr_const_map_entry_literal32))
    *
    *  2. Max. number of PDS program buffers (24) * (
    *         size of the largest buffer structure
    *             (pvr_const_map_entry_constant_buffer) +
    *         size of Common Store burst entry
    *             (pvr_const_map_entry_literal32)
    *
    *  3. Size of DOUTU entry (pvr_const_map_entry_doutu_address)
    *
    *  4. Max. number of PDS address literals (8) * (
    *         size of entry
    *             (pvr_const_map_entry_descriptor_set_addrs_table)
    *
    *  5. Max. number of address literals with single buffer entry to DOUTD
              size of entry
                  (pvr_pds_const_map_entry_addr_literal_buffer) +
              8 * size of entry (pvr_pds_const_map_entry_addr_literal)
    */

   /* FIXME: PVR_MAX_DESCRIPTOR_SETS is 4 and not 8. The comment above seems to
    * say that it should be 8.
    * Figure our a define for this or is the comment wrong?
    */
   return (8 * (sizeof(struct pvr_const_map_entry_descriptor_set) +
                sizeof(struct pvr_const_map_entry_literal32)) +
           PVR_PDS_MAX_BUFFERS *
              (sizeof(struct pvr_const_map_entry_constant_buffer) +
               sizeof(struct pvr_const_map_entry_literal32)) +
           sizeof(struct pvr_const_map_entry_doutu_address) +
           sizeof(struct pvr_pds_const_map_entry_addr_literal_buffer) +
           8 * sizeof(struct pvr_pds_const_map_entry_addr_literal));
}

/* This is a const pointer to an array of PVR_PDS_MAX_BUFFERS pvr_pds_buffer
 * structs.
 */
typedef struct pvr_pds_buffer (
      *const pvr_pds_descriptor_program_buffer_array_ptr)[PVR_PDS_MAX_BUFFERS];

/**
 * \brief Setup buffers for the PDS descriptor program.
 *
 * Sets up buffers required by the PDS gen api based on compiler info.
 *
 * For compile time static constants that need DMAing it uploads them and
 * returns the upload in \r static_consts_pvr_bo_out .
 */
static VkResult pvr_pds_descriptor_program_setup_buffers(
   struct pvr_device *device,
   bool robust_buffer_access,
   const struct rogue_compile_time_consts_data *compile_time_consts_data,
   const struct rogue_ubo_data *ubo_data,
   pvr_pds_descriptor_program_buffer_array_ptr buffers_out_ptr,
   uint32_t *const buffer_count_out,
   struct pvr_suballoc_bo **const static_consts_pvr_bo_out)
{
   struct pvr_pds_buffer *const buffers = *buffers_out_ptr;
   uint32_t buffer_count = 0;

   for (size_t i = 0; i < ubo_data->num_ubo_entries; i++) {
      struct pvr_pds_buffer *current_buffer = &buffers[buffer_count];

      /* This is fine since buffers_out_ptr is a pointer to an array. */
      assert(buffer_count < ARRAY_SIZE(*buffers_out_ptr));

      current_buffer->type = PVR_BUFFER_TYPE_UBO;
      current_buffer->size_in_dwords = ubo_data->size[i];
      current_buffer->destination = ubo_data->dest[i];

      current_buffer->buffer_id = buffer_count;
      current_buffer->desc_set = ubo_data->desc_set[i];
      current_buffer->binding = ubo_data->binding[i];
      /* TODO: Is this always the case?
       * E.g. can multiple UBOs have the same base buffer?
       */
      current_buffer->source_offset = 0;

      buffer_count++;
   }

   if (compile_time_consts_data->static_consts.num > 0) {
      VkResult result;

      assert(compile_time_consts_data->static_consts.num <=
             ARRAY_SIZE(compile_time_consts_data->static_consts.value));

      /* This is fine since buffers_out_ptr is a pointer to an array. */
      assert(buffer_count < ARRAY_SIZE(*buffers_out_ptr));

      /* TODO: Is it possible to have multiple static consts buffer where the
       * destination is not adjoining? If so we need to handle that.
       * Currently we're only setting up a single buffer.
       */
      buffers[buffer_count++] = (struct pvr_pds_buffer){
         .type = PVR_BUFFER_TYPE_COMPILE_TIME,
         .size_in_dwords = compile_time_consts_data->static_consts.num,
         .destination = compile_time_consts_data->static_consts.dest,
      };

      result = pvr_gpu_upload(device,
                              device->heaps.general_heap,
                              compile_time_consts_data->static_consts.value,
                              compile_time_consts_data->static_consts.num *
                                 ROGUE_REG_SIZE_BYTES,
                              ROGUE_REG_SIZE_BYTES,
                              static_consts_pvr_bo_out);
      if (result != VK_SUCCESS)
         return result;
   } else {
      *static_consts_pvr_bo_out = NULL;
   }

   *buffer_count_out = buffer_count;

   return VK_SUCCESS;
}

static VkResult pvr_pds_descriptor_program_create_and_upload(
   struct pvr_device *const device,
   const VkAllocationCallbacks *const allocator,
   const struct rogue_compile_time_consts_data *const compile_time_consts_data,
   const struct rogue_ubo_data *const ubo_data,
   const struct pvr_explicit_constant_usage *const explicit_const_usage,
   const struct pvr_pipeline_layout *const layout,
   enum pvr_stage_allocation stage,
   const struct pvr_sh_reg_layout *sh_reg_layout,
   struct pvr_stage_allocation_descriptor_state *const descriptor_state)
{
   const size_t const_entries_size_in_bytes =
      pvr_pds_get_max_descriptor_upload_const_map_size_in_bytes();
   struct pvr_pds_info *const pds_info = &descriptor_state->pds_info;
   struct pvr_pds_descriptor_program_input program = { 0 };
   struct pvr_const_map_entry *new_entries;
   ASSERTED uint32_t code_size_in_dwords;
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   const bool old_path = pvr_has_hard_coded_shaders(&device->pdevice->dev_info);

   assert(stage != PVR_STAGE_ALLOCATION_COUNT);

   *pds_info = (struct pvr_pds_info){ 0 };

   if (old_path) {
      result = pvr_pds_descriptor_program_setup_buffers(
         device,
         device->vk.enabled_features.robustBufferAccess,
         compile_time_consts_data,
         ubo_data,
         &program.buffers,
         &program.buffer_count,
         &descriptor_state->static_consts);
      if (result != VK_SUCCESS)
         return result;

      if (layout->per_stage_reg_info[stage].primary_dynamic_size_in_dwords)
         assert(!"Unimplemented");

      for (uint32_t set_num = 0; set_num < layout->set_count; set_num++) {
         const struct pvr_descriptor_set_layout_mem_layout *const reg_layout =
            &layout->register_layout_in_dwords_per_stage[stage][set_num];
         const uint32_t start_offset = explicit_const_usage->start_offset;

         /* TODO: Use compiler usage info to optimize this? */

         /* Only dma primaries if they are actually required. */
         if (reg_layout->primary_size) {
            program.descriptor_sets[program.descriptor_set_count++] =
               (struct pvr_pds_descriptor_set){
                  .descriptor_set = set_num,
                  .size_in_dwords = reg_layout->primary_size,
                  .destination = reg_layout->primary_offset + start_offset,
                  .primary = true,
               };
         }

         /* Only dma secondaries if they are actually required. */
         if (!reg_layout->secondary_size)
            continue;

         program.descriptor_sets[program.descriptor_set_count++] =
            (struct pvr_pds_descriptor_set){
               .descriptor_set = set_num,
               .size_in_dwords = reg_layout->secondary_size,
               .destination = reg_layout->secondary_offset + start_offset,
            };
      }
   } else {
      uint32_t addr_literals = 0;

      if (sh_reg_layout->descriptor_set_addrs_table.present) {
         program.addr_literals[addr_literals] = (struct pvr_pds_addr_literal){
            .type = PVR_PDS_ADDR_LITERAL_DESC_SET_ADDRS_TABLE,
            .destination = sh_reg_layout->descriptor_set_addrs_table.offset,
         };
         addr_literals++;
      }

      if (sh_reg_layout->push_consts.present) {
         program.addr_literals[addr_literals] = (struct pvr_pds_addr_literal){
            .type = PVR_PDS_ADDR_LITERAL_PUSH_CONSTS,
            .destination = sh_reg_layout->push_consts.offset,
         };
         addr_literals++;
      }

      if (sh_reg_layout->blend_consts.present) {
         program.addr_literals[addr_literals] = (struct pvr_pds_addr_literal){
            .type = PVR_PDS_ADDR_LITERAL_BLEND_CONSTANTS,
            .destination = sh_reg_layout->blend_consts.offset,
         };
         addr_literals++;
      }

      program.addr_literal_count = addr_literals;
   }

   pds_info->entries = vk_alloc2(&device->vk.alloc,
                                 allocator,
                                 const_entries_size_in_bytes,
                                 8,
                                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!pds_info->entries) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_free_static_consts;
   }

   pds_info->entries_size_in_bytes = const_entries_size_in_bytes;

   pvr_pds_generate_descriptor_upload_program(&program, NULL, pds_info);

   code_size_in_dwords = pds_info->code_size_in_dwords;
   staging_buffer_size = PVR_DW_TO_BYTES(pds_info->code_size_in_dwords);

   if (!staging_buffer_size) {
      vk_free2(&device->vk.alloc, allocator, pds_info->entries);

      *descriptor_state = (struct pvr_stage_allocation_descriptor_state){ 0 };

      return VK_SUCCESS;
   }

   staging_buffer = vk_alloc2(&device->vk.alloc,
                              allocator,
                              staging_buffer_size,
                              8,
                              VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_free_entries;
   }

   pvr_pds_generate_descriptor_upload_program(&program,
                                              staging_buffer,
                                              pds_info);

   assert(pds_info->code_size_in_dwords <= code_size_in_dwords);

   /* FIXME: use vk_realloc2() ? */
   new_entries = vk_realloc((!allocator) ? &device->vk.alloc : allocator,
                            pds_info->entries,
                            pds_info->entries_written_size_in_bytes,
                            8,
                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!new_entries) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto err_free_staging_buffer;
   }

   pds_info->entries = new_entries;
   pds_info->entries_size_in_bytes = pds_info->entries_written_size_in_bytes;

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               NULL,
                               0,
                               0,
                               staging_buffer,
                               pds_info->code_size_in_dwords,
                               16,
                               16,
                               &descriptor_state->pds_code);
   if (result != VK_SUCCESS)
      goto err_free_staging_buffer;

   vk_free2(&device->vk.alloc, allocator, staging_buffer);

   return VK_SUCCESS;

err_free_staging_buffer:
   vk_free2(&device->vk.alloc, allocator, staging_buffer);

err_free_entries:
   vk_free2(&device->vk.alloc, allocator, pds_info->entries);

err_free_static_consts:
   pvr_bo_suballoc_free(descriptor_state->static_consts);

   return result;
}

static void pvr_pds_descriptor_program_destroy(
   struct pvr_device *const device,
   const struct VkAllocationCallbacks *const allocator,
   struct pvr_stage_allocation_descriptor_state *const descriptor_state)
{
   if (!descriptor_state)
      return;

   pvr_bo_suballoc_free(descriptor_state->pds_code.pvr_bo);
   vk_free2(&device->vk.alloc, allocator, descriptor_state->pds_info.entries);
   pvr_bo_suballoc_free(descriptor_state->static_consts);
}

static void pvr_pds_compute_program_setup(
   const struct pvr_device_info *dev_info,
   const uint32_t local_input_regs[static const PVR_WORKGROUP_DIMENSIONS],
   const uint32_t work_group_input_regs[static const PVR_WORKGROUP_DIMENSIONS],
   uint32_t barrier_coefficient,
   bool add_base_workgroup,
   uint32_t usc_temps,
   pvr_dev_addr_t usc_shader_dev_addr,
   struct pvr_pds_compute_shader_program *const program)
{
   pvr_pds_compute_shader_program_init(program);
   program->local_input_regs[0] = local_input_regs[0];
   program->local_input_regs[1] = local_input_regs[1];
   program->local_input_regs[2] = local_input_regs[2];
   program->work_group_input_regs[0] = work_group_input_regs[0];
   program->work_group_input_regs[1] = work_group_input_regs[1];
   program->work_group_input_regs[2] = work_group_input_regs[2];
   program->barrier_coefficient = barrier_coefficient;
   program->add_base_workgroup = add_base_workgroup;
   program->flattened_work_groups = true;
   program->kick_usc = true;

   STATIC_ASSERT(ARRAY_SIZE(program->local_input_regs) ==
                 PVR_WORKGROUP_DIMENSIONS);
   STATIC_ASSERT(ARRAY_SIZE(program->work_group_input_regs) ==
                 PVR_WORKGROUP_DIMENSIONS);
   STATIC_ASSERT(ARRAY_SIZE(program->global_input_regs) ==
                 PVR_WORKGROUP_DIMENSIONS);

   pvr_pds_setup_doutu(&program->usc_task_control,
                       usc_shader_dev_addr.addr,
                       usc_temps,
                       PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);

   pvr_pds_compute_shader(program, NULL, PDS_GENERATE_SIZES, dev_info);
}

/* FIXME: See if pvr_device_init_compute_pds_program() and this could be merged.
 */
static VkResult pvr_pds_compute_program_create_and_upload(
   struct pvr_device *const device,
   const VkAllocationCallbacks *const allocator,
   const uint32_t local_input_regs[static const PVR_WORKGROUP_DIMENSIONS],
   const uint32_t work_group_input_regs[static const PVR_WORKGROUP_DIMENSIONS],
   uint32_t barrier_coefficient,
   uint32_t usc_temps,
   pvr_dev_addr_t usc_shader_dev_addr,
   struct pvr_pds_upload *const pds_upload_out,
   struct pvr_pds_info *const pds_info_out)
{
   struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   struct pvr_pds_compute_shader_program program;
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   pvr_pds_compute_program_setup(dev_info,
                                 local_input_regs,
                                 work_group_input_regs,
                                 barrier_coefficient,
                                 false,
                                 usc_temps,
                                 usc_shader_dev_addr,
                                 &program);

   /* FIXME: According to pvr_device_init_compute_pds_program() the code size
    * is in bytes. Investigate this.
    */
   staging_buffer_size = PVR_DW_TO_BYTES(program.code_size + program.data_size);

   staging_buffer = vk_alloc2(&device->vk.alloc,
                              allocator,
                              staging_buffer_size,
                              8,
                              VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* FIXME: pvr_pds_compute_shader doesn't implement
    * PDS_GENERATE_CODEDATA_SEGMENTS.
    */
   pvr_pds_compute_shader(&program,
                          &staging_buffer[0],
                          PDS_GENERATE_CODE_SEGMENT,
                          dev_info);

   pvr_pds_compute_shader(&program,
                          &staging_buffer[program.code_size],
                          PDS_GENERATE_DATA_SEGMENT,
                          dev_info);

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               &staging_buffer[program.code_size],
                               program.data_size,
                               16,
                               &staging_buffer[0],
                               program.code_size,
                               16,
                               16,
                               pds_upload_out);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, allocator, staging_buffer);
      return result;
   }

   *pds_info_out = (struct pvr_pds_info){
      .temps_required = program.highest_temp,
      .code_size_in_dwords = program.code_size,
      .data_size_in_dwords = program.data_size,
   };

   vk_free2(&device->vk.alloc, allocator, staging_buffer);

   return VK_SUCCESS;
};

static void pvr_pds_compute_program_destroy(
   struct pvr_device *const device,
   const struct VkAllocationCallbacks *const allocator,
   struct pvr_pds_upload *const pds_program,
   struct pvr_pds_info *const pds_info)
{
   /* We don't allocate an entries buffer so we don't need to free it */
   pvr_bo_suballoc_free(pds_program->pvr_bo);
}

/* This only uploads the code segment. The data segment will need to be patched
 * with the base workgroup before uploading.
 */
static VkResult pvr_pds_compute_base_workgroup_variant_program_init(
   struct pvr_device *const device,
   const VkAllocationCallbacks *const allocator,
   const uint32_t local_input_regs[static const PVR_WORKGROUP_DIMENSIONS],
   const uint32_t work_group_input_regs[static const PVR_WORKGROUP_DIMENSIONS],
   uint32_t barrier_coefficient,
   uint32_t usc_temps,
   pvr_dev_addr_t usc_shader_dev_addr,
   struct pvr_pds_base_workgroup_program *program_out)
{
   struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   struct pvr_pds_compute_shader_program program;
   uint32_t buffer_size;
   uint32_t *buffer;
   VkResult result;

   pvr_pds_compute_program_setup(dev_info,
                                 local_input_regs,
                                 work_group_input_regs,
                                 barrier_coefficient,
                                 true,
                                 usc_temps,
                                 usc_shader_dev_addr,
                                 &program);

   /* FIXME: According to pvr_device_init_compute_pds_program() the code size
    * is in bytes. Investigate this.
    */
   buffer_size = PVR_DW_TO_BYTES(MAX2(program.code_size, program.data_size));

   buffer = vk_alloc2(&device->vk.alloc,
                      allocator,
                      buffer_size,
                      8,
                      VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pvr_pds_compute_shader(&program,
                          &buffer[0],
                          PDS_GENERATE_CODE_SEGMENT,
                          dev_info);

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               NULL,
                               0,
                               0,
                               buffer,
                               program.code_size,
                               16,
                               16,
                               &program_out->code_upload);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, allocator, buffer);
      return result;
   }

   pvr_pds_compute_shader(&program, buffer, PDS_GENERATE_DATA_SEGMENT, dev_info);

   program_out->data_section = buffer;

   /* We'll need to patch the base workgroup in the PDS data section before
    * dispatch so we save the offsets at which to patch. We only need to save
    * the offset for the first workgroup id since the workgroup ids are stored
    * contiguously in the data segment.
    */
   program_out->base_workgroup_data_patching_offset =
      program.base_workgroup_constant_offset_in_dwords[0];

   program_out->info = (struct pvr_pds_info){
      .temps_required = program.highest_temp,
      .code_size_in_dwords = program.code_size,
      .data_size_in_dwords = program.data_size,
   };

   return VK_SUCCESS;
}

static void pvr_pds_compute_base_workgroup_variant_program_finish(
   struct pvr_device *device,
   const VkAllocationCallbacks *const allocator,
   struct pvr_pds_base_workgroup_program *const state)
{
   pvr_bo_suballoc_free(state->code_upload.pvr_bo);
   vk_free2(&device->vk.alloc, allocator, state->data_section);
}

/******************************************************************************
   Generic pipeline functions
 ******************************************************************************/

static void pvr_pipeline_init(struct pvr_device *device,
                              enum pvr_pipeline_type type,
                              struct pvr_pipeline *const pipeline)
{
   assert(!pipeline->layout);

   vk_object_base_init(&device->vk, &pipeline->base, VK_OBJECT_TYPE_PIPELINE);

   pipeline->type = type;
}

static void pvr_pipeline_finish(struct pvr_pipeline *pipeline)
{
   vk_object_base_finish(&pipeline->base);
}

/* How many shared regs it takes to store a pvr_dev_addr_t.
 * Each shared reg is 32 bits.
 */
#define PVR_DEV_ADDR_SIZE_IN_SH_REGS \
   DIV_ROUND_UP(sizeof(pvr_dev_addr_t), sizeof(uint32_t))

/**
 * \brief Allocates shared registers.
 *
 * \return How many sh regs are required.
 */
static uint32_t
pvr_pipeline_alloc_shareds(const struct pvr_device *device,
                           const struct pvr_pipeline_layout *layout,
                           enum pvr_stage_allocation stage,
                           struct pvr_sh_reg_layout *const sh_reg_layout_out)
{
   ASSERTED const uint64_t reserved_shared_size =
      device->pdevice->dev_runtime_info.reserved_shared_size;
   ASSERTED const uint64_t max_coeff =
      device->pdevice->dev_runtime_info.max_coeffs;

   struct pvr_sh_reg_layout reg_layout = { 0 };
   uint32_t next_free_sh_reg = 0;

   reg_layout.descriptor_set_addrs_table.present =
      !!(layout->shader_stage_mask & BITFIELD_BIT(stage));

   if (reg_layout.descriptor_set_addrs_table.present) {
      reg_layout.descriptor_set_addrs_table.offset = next_free_sh_reg;
      next_free_sh_reg += PVR_DEV_ADDR_SIZE_IN_SH_REGS;
   }

   reg_layout.push_consts.present =
      !!(layout->push_constants_shader_stages & BITFIELD_BIT(stage));

   if (reg_layout.push_consts.present) {
      reg_layout.push_consts.offset = next_free_sh_reg;
      next_free_sh_reg += PVR_DEV_ADDR_SIZE_IN_SH_REGS;
   }

   *sh_reg_layout_out = reg_layout;

   /* FIXME: We might need to take more things into consideration.
    * See pvr_calc_fscommon_size_and_tiles_in_flight().
    */
   assert(next_free_sh_reg <= reserved_shared_size - max_coeff);

   return next_free_sh_reg;
}

/******************************************************************************
   Compute pipeline functions
 ******************************************************************************/

/* Compiles and uploads shaders and PDS programs. */
static VkResult pvr_compute_pipeline_compile(
   struct pvr_device *const device,
   struct vk_pipeline_cache *cache,
   const VkComputePipelineCreateInfo *pCreateInfo,
   const VkAllocationCallbacks *const allocator,
   struct pvr_compute_pipeline *const compute_pipeline)
{
   struct pvr_pipeline_layout *layout = compute_pipeline->base.layout;
   struct pvr_sh_reg_layout *sh_reg_layout =
      &layout->sh_reg_layout_per_stage[PVR_STAGE_ALLOCATION_COMPUTE];
   struct rogue_compile_time_consts_data compile_time_consts_data;
   uint32_t work_group_input_regs[PVR_WORKGROUP_DIMENSIONS];
   struct pvr_explicit_constant_usage explicit_const_usage;
   uint32_t local_input_regs[PVR_WORKGROUP_DIMENSIONS];
   struct rogue_ubo_data ubo_data;
   uint32_t barrier_coefficient;
   uint32_t usc_temps;
   VkResult result;

   if (pvr_has_hard_coded_shaders(&device->pdevice->dev_info)) {
      struct pvr_hard_code_compute_build_info build_info;

      result = pvr_hard_code_compute_pipeline(device,
                                              &compute_pipeline->shader_state,
                                              &build_info);
      if (result != VK_SUCCESS)
         return result;

      ubo_data = build_info.ubo_data;
      compile_time_consts_data = build_info.compile_time_consts_data;

      /* We make sure that the compiler's unused reg value is compatible with
       * the pds api.
       */
      STATIC_ASSERT(ROGUE_REG_UNUSED == PVR_PDS_COMPUTE_INPUT_REG_UNUSED);

      barrier_coefficient = build_info.barrier_reg;

      /* TODO: Maybe change the pds api to use pointers so we avoid the copy. */
      local_input_regs[0] = build_info.local_invocation_regs[0];
      local_input_regs[1] = build_info.local_invocation_regs[1];
      /* This is not a mistake. We want to assign element 1 to 2. */
      local_input_regs[2] = build_info.local_invocation_regs[1];

      STATIC_ASSERT(
         __same_type(work_group_input_regs, build_info.work_group_regs));
      typed_memcpy(work_group_input_regs,
                   build_info.work_group_regs,
                   PVR_WORKGROUP_DIMENSIONS);

      usc_temps = build_info.usc_temps;

      explicit_const_usage = build_info.explicit_conts_usage;

   } else {
      uint32_t sh_count;
      sh_count = pvr_pipeline_alloc_shareds(device,
                                            layout,
                                            PVR_STAGE_ALLOCATION_COMPUTE,
                                            sh_reg_layout);

      compute_pipeline->shader_state.const_shared_reg_count = sh_count;

      /* FIXME: Compile and upload the shader. */
      /* FIXME: Initialize the shader state and setup build info. */
      abort();
   };

   result = pvr_pds_descriptor_program_create_and_upload(
      device,
      allocator,
      &compile_time_consts_data,
      &ubo_data,
      &explicit_const_usage,
      layout,
      PVR_STAGE_ALLOCATION_COMPUTE,
      sh_reg_layout,
      &compute_pipeline->descriptor_state);
   if (result != VK_SUCCESS)
      goto err_free_shader;

   result = pvr_pds_compute_program_create_and_upload(
      device,
      allocator,
      local_input_regs,
      work_group_input_regs,
      barrier_coefficient,
      usc_temps,
      compute_pipeline->shader_state.bo->dev_addr,
      &compute_pipeline->primary_program,
      &compute_pipeline->primary_program_info);
   if (result != VK_SUCCESS)
      goto err_free_descriptor_program;

   /* If the workgroup ID is required, then we require the base workgroup
    * variant of the PDS compute program as well.
    */
   compute_pipeline->flags.base_workgroup =
      work_group_input_regs[0] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED ||
      work_group_input_regs[1] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED ||
      work_group_input_regs[2] != PVR_PDS_COMPUTE_INPUT_REG_UNUSED;

   if (compute_pipeline->flags.base_workgroup) {
      result = pvr_pds_compute_base_workgroup_variant_program_init(
         device,
         allocator,
         local_input_regs,
         work_group_input_regs,
         barrier_coefficient,
         usc_temps,
         compute_pipeline->shader_state.bo->dev_addr,
         &compute_pipeline->primary_base_workgroup_variant_program);
      if (result != VK_SUCCESS)
         goto err_destroy_compute_program;
   }

   return VK_SUCCESS;

err_destroy_compute_program:
   pvr_pds_compute_program_destroy(device,
                                   allocator,
                                   &compute_pipeline->primary_program,
                                   &compute_pipeline->primary_program_info);

err_free_descriptor_program:
   pvr_pds_descriptor_program_destroy(device,
                                      allocator,
                                      &compute_pipeline->descriptor_state);

err_free_shader:
   pvr_bo_suballoc_free(compute_pipeline->shader_state.bo);

   return result;
}

static VkResult
pvr_compute_pipeline_init(struct pvr_device *device,
                          struct vk_pipeline_cache *cache,
                          const VkComputePipelineCreateInfo *pCreateInfo,
                          const VkAllocationCallbacks *allocator,
                          struct pvr_compute_pipeline *compute_pipeline)
{
   VkResult result;

   pvr_pipeline_init(device,
                     PVR_PIPELINE_TYPE_COMPUTE,
                     &compute_pipeline->base);

   compute_pipeline->base.layout =
      pvr_pipeline_layout_from_handle(pCreateInfo->layout);

   result = pvr_compute_pipeline_compile(device,
                                         cache,
                                         pCreateInfo,
                                         allocator,
                                         compute_pipeline);
   if (result != VK_SUCCESS) {
      pvr_pipeline_finish(&compute_pipeline->base);
      return result;
   }

   return VK_SUCCESS;
}

static VkResult
pvr_compute_pipeline_create(struct pvr_device *device,
                            struct vk_pipeline_cache *cache,
                            const VkComputePipelineCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *allocator,
                            VkPipeline *const pipeline_out)
{
   struct pvr_compute_pipeline *compute_pipeline;
   VkResult result;

   compute_pipeline = vk_zalloc2(&device->vk.alloc,
                                 allocator,
                                 sizeof(*compute_pipeline),
                                 8,
                                 VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!compute_pipeline)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* Compiles and uploads shaders and PDS programs. */
   result = pvr_compute_pipeline_init(device,
                                      cache,
                                      pCreateInfo,
                                      allocator,
                                      compute_pipeline);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, allocator, compute_pipeline);
      return result;
   }

   *pipeline_out = pvr_pipeline_to_handle(&compute_pipeline->base);

   return VK_SUCCESS;
}

static void pvr_compute_pipeline_destroy(
   struct pvr_device *const device,
   const VkAllocationCallbacks *const allocator,
   struct pvr_compute_pipeline *const compute_pipeline)
{
   if (compute_pipeline->flags.base_workgroup) {
      pvr_pds_compute_base_workgroup_variant_program_finish(
         device,
         allocator,
         &compute_pipeline->primary_base_workgroup_variant_program);
   }

   pvr_pds_compute_program_destroy(device,
                                   allocator,
                                   &compute_pipeline->primary_program,
                                   &compute_pipeline->primary_program_info);
   pvr_pds_descriptor_program_destroy(device,
                                      allocator,
                                      &compute_pipeline->descriptor_state);
   pvr_bo_suballoc_free(compute_pipeline->shader_state.bo);

   pvr_pipeline_finish(&compute_pipeline->base);

   vk_free2(&device->vk.alloc, allocator, compute_pipeline);
}

VkResult
pvr_CreateComputePipelines(VkDevice _device,
                           VkPipelineCache pipelineCache,
                           uint32_t createInfoCount,
                           const VkComputePipelineCreateInfo *pCreateInfos,
                           const VkAllocationCallbacks *pAllocator,
                           VkPipeline *pPipelines)
{
   VK_FROM_HANDLE(vk_pipeline_cache, cache, pipelineCache);
   PVR_FROM_HANDLE(pvr_device, device, _device);
   VkResult result = VK_SUCCESS;

   for (uint32_t i = 0; i < createInfoCount; i++) {
      const VkResult local_result =
         pvr_compute_pipeline_create(device,
                                     cache,
                                     &pCreateInfos[i],
                                     pAllocator,
                                     &pPipelines[i]);
      if (local_result != VK_SUCCESS) {
         result = local_result;
         pPipelines[i] = VK_NULL_HANDLE;
      }
   }

   return result;
}

/******************************************************************************
   Graphics pipeline functions
 ******************************************************************************/

static void
pvr_graphics_pipeline_destroy(struct pvr_device *const device,
                              const VkAllocationCallbacks *const allocator,
                              struct pvr_graphics_pipeline *const gfx_pipeline)
{
   const uint32_t num_vertex_attrib_programs =
      ARRAY_SIZE(gfx_pipeline->shader_state.vertex.pds_attrib_programs);

   pvr_pds_descriptor_program_destroy(
      device,
      allocator,
      &gfx_pipeline->shader_state.fragment.descriptor_state);

   pvr_pds_descriptor_program_destroy(
      device,
      allocator,
      &gfx_pipeline->shader_state.vertex.descriptor_state);

   for (uint32_t i = 0; i < num_vertex_attrib_programs; i++) {
      struct pvr_pds_attrib_program *const attrib_program =
         &gfx_pipeline->shader_state.vertex.pds_attrib_programs[i];

      pvr_pds_vertex_attrib_program_destroy(device, allocator, attrib_program);
   }

   pvr_bo_suballoc_free(
      gfx_pipeline->shader_state.fragment.pds_fragment_program.pvr_bo);
   pvr_bo_suballoc_free(
      gfx_pipeline->shader_state.fragment.pds_coeff_program.pvr_bo);

   pvr_bo_suballoc_free(gfx_pipeline->shader_state.fragment.bo);
   pvr_bo_suballoc_free(gfx_pipeline->shader_state.vertex.bo);

   pvr_pipeline_finish(&gfx_pipeline->base);

   vk_free2(&device->vk.alloc, allocator, gfx_pipeline);
}

static void
pvr_vertex_state_init(struct pvr_graphics_pipeline *gfx_pipeline,
                      const struct rogue_common_build_data *common_data,
                      uint32_t vtxin_regs_used,
                      const struct rogue_vs_build_data *vs_data)
{
   struct pvr_vertex_shader_state *vertex_state =
      &gfx_pipeline->shader_state.vertex;

   /* TODO: Hard coding these for now. These should be populated based on the
    * information returned by the compiler.
    */
   vertex_state->stage_state.const_shared_reg_count = common_data->shareds;
   vertex_state->stage_state.const_shared_reg_offset = 0;
   vertex_state->stage_state.coefficient_size = common_data->coeffs;
   vertex_state->stage_state.uses_atomic_ops = false;
   vertex_state->stage_state.uses_texture_rw = false;
   vertex_state->stage_state.uses_barrier = false;
   vertex_state->stage_state.has_side_effects = false;
   vertex_state->stage_state.empty_program = false;

   /* This ends up unused since we'll use the temp_usage for the PDS program we
    * end up selecting, and the descriptor PDS program doesn't use any temps.
    * Let's set it to ~0 in case it ever gets used.
    */
   vertex_state->stage_state.pds_temps_count = ~0;

   vertex_state->vertex_input_size = vtxin_regs_used;
   vertex_state->vertex_output_size =
      vs_data->num_vertex_outputs * ROGUE_REG_SIZE_BYTES;
   vertex_state->user_clip_planes_mask = 0;
   vertex_state->entry_offset = 0;

   /* TODO: The number of varyings should be checked against the fragment
    * shader inputs and assigned in the place where that happens.
    * There will also be an opportunity to cull unused fs inputs/vs outputs.
    */
   pvr_csb_pack (&gfx_pipeline->shader_state.vertex.varying[0],
                 TA_STATE_VARYING0,
                 varying0) {
      varying0.f32_linear = vs_data->num_varyings;
      varying0.f32_flat = 0;
      varying0.f32_npc = 0;
   }

   pvr_csb_pack (&gfx_pipeline->shader_state.vertex.varying[1],
                 TA_STATE_VARYING1,
                 varying1) {
      varying1.f16_linear = 0;
      varying1.f16_flat = 0;
      varying1.f16_npc = 0;
   }
}

static void
pvr_fragment_state_init(struct pvr_graphics_pipeline *gfx_pipeline,
                        const struct rogue_common_build_data *common_data)
{
   struct pvr_fragment_shader_state *fragment_state =
      &gfx_pipeline->shader_state.fragment;

   /* TODO: Hard coding these for now. These should be populated based on the
    * information returned by the compiler.
    */
   fragment_state->stage_state.const_shared_reg_count = 0;
   fragment_state->stage_state.const_shared_reg_offset = 0;
   fragment_state->stage_state.coefficient_size = common_data->coeffs;
   fragment_state->stage_state.uses_atomic_ops = false;
   fragment_state->stage_state.uses_texture_rw = false;
   fragment_state->stage_state.uses_barrier = false;
   fragment_state->stage_state.has_side_effects = false;
   fragment_state->stage_state.empty_program = false;

   fragment_state->pass_type = PVRX(TA_PASSTYPE_OPAQUE);
   fragment_state->entry_offset = 0;

   /* We can't initialize it yet since we still need to generate the PDS
    * programs so set it to `~0` to make sure that we set this up later on.
    */
   fragment_state->stage_state.pds_temps_count = ~0;
}

static bool pvr_blend_factor_requires_consts(VkBlendFactor factor)
{
   switch (factor) {
   case VK_BLEND_FACTOR_CONSTANT_COLOR:
   case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
   case VK_BLEND_FACTOR_CONSTANT_ALPHA:
   case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
      return true;

   default:
      return false;
   }
}

/**
 * \brief Indicates whether dynamic blend constants are needed.
 *
 * If the user has specified the blend constants to be dynamic, they might not
 * necessarily be using them. This function makes sure that they are being used
 * in order to determine whether we need to upload them later on for the shader
 * to access them.
 */
static bool pvr_graphics_pipeline_requires_dynamic_blend_consts(
   const struct pvr_graphics_pipeline *gfx_pipeline)
{
   const struct vk_dynamic_graphics_state *const state =
      &gfx_pipeline->dynamic_state;

   if (BITSET_TEST(state->set, MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS))
      return false;

   for (uint32_t i = 0; i < state->cb.attachment_count; i++) {
      const struct vk_color_blend_attachment_state *attachment =
         &state->cb.attachments[i];

      const bool has_color_write =
         attachment->write_mask &
         (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT);
      const bool has_alpha_write = attachment->write_mask &
                                   VK_COLOR_COMPONENT_A_BIT;

      if (!attachment->blend_enable || attachment->write_mask == 0)
         continue;

      if (has_color_write) {
         const uint8_t src_color_blend_factor =
            attachment->src_color_blend_factor;
         const uint8_t dst_color_blend_factor =
            attachment->dst_color_blend_factor;

         if (pvr_blend_factor_requires_consts(src_color_blend_factor) ||
             pvr_blend_factor_requires_consts(dst_color_blend_factor)) {
            return true;
         }
      }

      if (has_alpha_write) {
         const uint8_t src_alpha_blend_factor =
            attachment->src_alpha_blend_factor;
         const uint8_t dst_alpha_blend_factor =
            attachment->dst_alpha_blend_factor;

         if (pvr_blend_factor_requires_consts(src_alpha_blend_factor) ||
             pvr_blend_factor_requires_consts(dst_alpha_blend_factor)) {
            return true;
         }
      }
   }

   return false;
}

static uint32_t pvr_graphics_pipeline_alloc_shareds(
   const struct pvr_device *device,
   const struct pvr_graphics_pipeline *gfx_pipeline,
   enum pvr_stage_allocation stage,
   struct pvr_sh_reg_layout *const sh_reg_layout_out)
{
   ASSERTED const uint64_t reserved_shared_size =
      device->pdevice->dev_runtime_info.reserved_shared_size;
   ASSERTED const uint64_t max_coeff =
      device->pdevice->dev_runtime_info.max_coeffs;

   const struct pvr_pipeline_layout *layout = gfx_pipeline->base.layout;
   struct pvr_sh_reg_layout reg_layout = { 0 };
   uint32_t next_free_sh_reg = 0;

   next_free_sh_reg =
      pvr_pipeline_alloc_shareds(device, layout, stage, &reg_layout);

   reg_layout.blend_consts.present =
      (stage == PVR_STAGE_ALLOCATION_FRAGMENT &&
       pvr_graphics_pipeline_requires_dynamic_blend_consts(gfx_pipeline));
   if (reg_layout.blend_consts.present) {
      reg_layout.blend_consts.offset = next_free_sh_reg;
      next_free_sh_reg += PVR_DEV_ADDR_SIZE_IN_SH_REGS;
   }

   *sh_reg_layout_out = reg_layout;

   /* FIXME: We might need to take more things into consideration.
    * See pvr_calc_fscommon_size_and_tiles_in_flight().
    */
   assert(next_free_sh_reg <= reserved_shared_size - max_coeff);

   return next_free_sh_reg;
}

#undef PVR_DEV_ADDR_SIZE_IN_SH_REGS

static void pvr_graphics_pipeline_alloc_vertex_inputs(
   const VkPipelineVertexInputStateCreateInfo *const vs_data,
   rogue_vertex_inputs *const vertex_input_layout_out,
   unsigned *num_vertex_input_regs_out,
   pvr_pds_attrib_dma_descriptions_array_ptr dma_descriptions_out_ptr,
   uint32_t *const dma_count_out)
{
   const VkVertexInputBindingDescription
      *sorted_bindings[PVR_MAX_VERTEX_INPUT_BINDINGS] = { 0 };
   const VkVertexInputAttributeDescription
      *sorted_attributes[PVR_MAX_VERTEX_INPUT_BINDINGS] = { 0 };

   rogue_vertex_inputs build_data = {
      .num_input_vars = vs_data->vertexAttributeDescriptionCount,
   };
   uint32_t next_reg_offset = 0;

   struct pvr_pds_vertex_dma *const dma_descriptions =
      *dma_descriptions_out_ptr;
   uint32_t dma_count = 0;

   /* Vertex attributes map to the `layout(location = x)` annotation in the
    * shader where `x` is the attribute's location.
    * Vertex bindings have NO relation to the shader. They have nothing to do
    * with the `layout(set = x, binding = y)` notation. They instead indicate
    * where the data for a collection of vertex attributes comes from. The
    * application binds a VkBuffer with vkCmdBindVertexBuffers() to a specific
    * binding number and based on that we'll know which buffer to DMA the data
    * from, to fill in the collection of vertex attributes.
    */

   for (uint32_t i = 0; i < vs_data->vertexBindingDescriptionCount; i++) {
      const VkVertexInputBindingDescription *binding_desc =
         &vs_data->pVertexBindingDescriptions[i];

      sorted_bindings[binding_desc->binding] = binding_desc;
   }

   for (uint32_t i = 0; i < vs_data->vertexAttributeDescriptionCount; i++) {
      const VkVertexInputAttributeDescription *attribute_desc =
         &vs_data->pVertexAttributeDescriptions[i];

      sorted_attributes[attribute_desc->location] = attribute_desc;
   }

   for (uint32_t i = 0, j = 0; i < ARRAY_SIZE(sorted_attributes); i++) {
      if (sorted_attributes[i])
         sorted_attributes[j++] = sorted_attributes[i];
   }

   for (uint32_t i = 0; i < vs_data->vertexAttributeDescriptionCount; i++) {
      const VkVertexInputAttributeDescription *attribute = sorted_attributes[i];
      const VkVertexInputBindingDescription *binding =
         sorted_bindings[attribute->binding];
      const struct util_format_description *fmt_description =
         vk_format_description(attribute->format);
      struct pvr_pds_vertex_dma *dma_desc = &dma_descriptions[dma_count];
      unsigned vtxin_reg_offset;

      /* Reg allocation. */

      vtxin_reg_offset = next_reg_offset;
      build_data.base[i] = vtxin_reg_offset;

      if (fmt_description->colorspace != UTIL_FORMAT_COLORSPACE_RGB ||
          fmt_description->layout != UTIL_FORMAT_LAYOUT_PLAIN ||
          fmt_description->block.bits % 32 != 0 || !fmt_description->is_array) {
         /* For now we only support formats with 32 bit components since we
          * don't need to pack/unpack them.
          */
         /* TODO: Support any other format with VERTEX_BUFFER_BIT set that
          * doesn't have 32 bit components if we're advertising any.
          */
         assert(false);
      }

      /* TODO: Check if this is fine with the compiler. Does it want the amount
       * of components or does it want a size in dwords to figure out how many
       * vtxin regs are covered. For formats with 32 bit components the
       * distinction doesn't change anything.
       */
      build_data.components[i] =
         util_format_get_nr_components(fmt_description->format);

      next_reg_offset += build_data.components[i];

      /* DMA setup. */

      /* The PDS program sets up DDMADs to DMA attributes into vtxin regs.
       *
       * DDMAD -> Multiply, add, and DOUTD (i.e. DMA from that address).
       *          DMA source addr = src0 * src1 + src2
       *          DMA params = src3
       *
       * In the PDS program we setup src0 with the binding's stride and src1
       * with either the instance id or vertex id (both of which get filled by
       * the hardware). We setup src2 later on once we know which VkBuffer to
       * DMA the data from so it's saved for later when we patch the data
       * section.
       */

      /* TODO: Right now we're setting up a DMA per attribute. In a case where
       * there are multiple attributes packed into a single binding with
       * adjacent locations we'd still be DMAing them separately. This is not
       * great so the DMA setup should be smarter and could do with some
       * optimization.
       */

      *dma_desc = (struct pvr_pds_vertex_dma){ 0 };

      /* In relation to the Vulkan spec. 22.4. Vertex Input Address Calculation
       * this corresponds to `attribDesc.offset`.
       * The PDS program doesn't do anything with it but just save it in the
       * PDS program entry.
       */
      dma_desc->offset = attribute->offset;

      /* In relation to the Vulkan spec. 22.4. Vertex Input Address Calculation
       * this corresponds to `bindingDesc.stride`.
       * The PDS program will calculate the `effectiveVertexOffset` with this
       * and add it to the address provided in the patched data segment.
       */
      dma_desc->stride = binding->stride;

      if (binding->inputRate == VK_VERTEX_INPUT_RATE_INSTANCE)
         dma_desc->flags = PVR_PDS_VERTEX_DMA_FLAGS_INSTANCE_RATE;
      else
         dma_desc->flags = 0;

      /* Size to DMA per vertex attribute. Used to setup src3 in the DDMAD. */
      assert(fmt_description->block.bits != 0); /* Likely an unsupported fmt. */
      dma_desc->size_in_dwords = fmt_description->block.bits / 32;

      /* Vtxin reg offset to start DMAing into. */
      dma_desc->destination = vtxin_reg_offset;

      /* Will be used by the driver to figure out buffer address to patch in the
       * data section. I.e. which binding we should DMA from.
       */
      dma_desc->binding_index = attribute->binding;

      /* We don't currently support VK_EXT_vertex_attribute_divisor so no
       * repeating of instance-rate vertex attributes needed. We should always
       * move on to the next vertex attribute.
       */
      dma_desc->divisor = 1;

      /* Will be used to generate PDS code that takes care of robust buffer
       * access, and later on by the driver to write the correct robustness
       * buffer address to DMA the fallback values from.
       */
      dma_desc->robustness_buffer_offset =
         pvr_get_robustness_buffer_format_offset(attribute->format);

      /* Used by later on by the driver to figure out if the buffer is being
       * accessed out of bounds, for robust buffer access.
       */
      dma_desc->component_size_in_bytes =
         fmt_description->block.bits / fmt_description->nr_channels / 8;

      dma_count++;
   };

   *vertex_input_layout_out = build_data;
   *num_vertex_input_regs_out = next_reg_offset;
   *dma_count_out = dma_count;
}

static void pvr_graphics_pipeline_alloc_vertex_special_vars(
   unsigned *num_vertex_input_regs,
   struct pvr_vertex_special_vars *special_vars_layout_out)
{
   unsigned next_free_reg = *num_vertex_input_regs;
   struct pvr_vertex_special_vars layout;

   /* We don't support VK_KHR_shader_draw_parameters or Vulkan 1.1 so no
    * BaseInstance, BaseVertex, DrawIndex.
    */

   /* TODO: The shader might not necessarily be using this so we'd just be
    * wasting regs. Get the info from the compiler about whether or not the
    * shader uses them and allocate them accordingly. For now we'll set them up
    * regardless.
    */

   layout.vertex_id_offset = (int16_t)next_free_reg;
   next_free_reg++;

   layout.instance_id_offset = (int16_t)next_free_reg;
   next_free_reg++;

   *num_vertex_input_regs = next_free_reg;
   *special_vars_layout_out = layout;
}

/* Compiles and uploads shaders and PDS programs. */
static VkResult
pvr_graphics_pipeline_compile(struct pvr_device *const device,
                              struct vk_pipeline_cache *cache,
                              const VkGraphicsPipelineCreateInfo *pCreateInfo,
                              const VkAllocationCallbacks *const allocator,
                              struct pvr_graphics_pipeline *const gfx_pipeline)
{
   /* FIXME: Remove this hard coding. */
   struct pvr_explicit_constant_usage vert_explicit_const_usage = {
      .start_offset = 16,
   };
   struct pvr_explicit_constant_usage frag_explicit_const_usage = {
      .start_offset = 0,
   };
   static uint32_t hard_code_pipeline_n = 0;

   struct pvr_pipeline_layout *layout = gfx_pipeline->base.layout;
   struct pvr_sh_reg_layout *sh_reg_layout_vert =
      &layout->sh_reg_layout_per_stage[PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY];
   struct pvr_sh_reg_layout *sh_reg_layout_frag =
      &layout->sh_reg_layout_per_stage[PVR_STAGE_ALLOCATION_FRAGMENT];
   const VkPipelineVertexInputStateCreateInfo *const vertex_input_state =
      pCreateInfo->pVertexInputState;
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
   struct rogue_compiler *compiler = device->pdevice->compiler;
   struct rogue_build_ctx *ctx;
   VkResult result;

   const bool old_path = pvr_has_hard_coded_shaders(&device->pdevice->dev_info);

   /* Vars needed for the new path. */
   struct pvr_pds_vertex_dma vtx_dma_descriptions[PVR_MAX_VERTEX_ATTRIB_DMAS];
   uint32_t vtx_dma_count = 0;
   rogue_vertex_inputs *vertex_input_layout;
   unsigned *vertex_input_reg_count;

   /* TODO: The compiler should be making use of this to determine where
    * specific special variables are located in the vtxin reg set.
    */
   struct pvr_vertex_special_vars special_vars_layout = { 0 };

   uint32_t sh_count[PVR_STAGE_ALLOCATION_COUNT] = { 0 };

   /* Setup shared build context. */
   ctx = rogue_build_context_create(compiler, layout);
   if (!ctx)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vertex_input_layout = &ctx->stage_data.vs.inputs;
   vertex_input_reg_count = &ctx->stage_data.vs.num_vertex_input_regs;

   if (!old_path) {
      pvr_graphics_pipeline_alloc_vertex_inputs(vertex_input_state,
                                                vertex_input_layout,
                                                vertex_input_reg_count,
                                                &vtx_dma_descriptions,
                                                &vtx_dma_count);

      pvr_graphics_pipeline_alloc_vertex_special_vars(vertex_input_reg_count,
                                                      &special_vars_layout);

      for (enum pvr_stage_allocation pvr_stage =
              PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY;
           pvr_stage < PVR_STAGE_ALLOCATION_COMPUTE;
           ++pvr_stage)
         sh_count[pvr_stage] = pvr_pipeline_alloc_shareds(
            device,
            layout,
            pvr_stage,
            &layout->sh_reg_layout_per_stage[pvr_stage]);
   }

   /* NIR middle-end translation. */
   for (gl_shader_stage stage = MESA_SHADER_FRAGMENT; stage > MESA_SHADER_NONE;
        stage--) {
      const VkPipelineShaderStageCreateInfo *create_info;
      size_t stage_index = gfx_pipeline->stage_indices[stage];

      if (pvr_has_hard_coded_shaders(&device->pdevice->dev_info)) {
         if (pvr_hard_code_graphics_get_flags(&device->pdevice->dev_info) &
             BITFIELD_BIT(stage)) {
            continue;
         }
      }

      /* Skip unused/inactive stages. */
      if (stage_index == ~0)
         continue;

      create_info = &pCreateInfo->pStages[stage_index];

      /* SPIR-V to NIR. */
      ctx->nir[stage] = pvr_spirv_to_nir(ctx, stage, create_info);
      if (!ctx->nir[stage]) {
         ralloc_free(ctx);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }
   }

   /* Pre-back-end analysis and optimization, driver data extraction. */
   /* TODO: Analyze and cull unused I/O between stages. */
   /* TODO: Allocate UBOs between stages;
    * pipeline->layout->set_{count,layout}.
    */

   /* Back-end translation. */
   for (gl_shader_stage stage = MESA_SHADER_FRAGMENT; stage > MESA_SHADER_NONE;
        stage--) {
      if (pvr_has_hard_coded_shaders(&device->pdevice->dev_info) &&
          pvr_hard_code_graphics_get_flags(&device->pdevice->dev_info) &
             BITFIELD_BIT(stage)) {
         const struct pvr_device_info *const dev_info =
            &device->pdevice->dev_info;
         struct pvr_explicit_constant_usage *explicit_const_usage;

         switch (stage) {
         case MESA_SHADER_VERTEX:
            explicit_const_usage = &vert_explicit_const_usage;
            break;

         case MESA_SHADER_FRAGMENT:
            explicit_const_usage = &frag_explicit_const_usage;
            break;

         default:
            unreachable("Unsupported stage.");
         }

         pvr_hard_code_graphics_shader(dev_info,
                                       hard_code_pipeline_n,
                                       stage,
                                       &ctx->binary[stage]);

         pvr_hard_code_graphics_get_build_info(dev_info,
                                               hard_code_pipeline_n,
                                               stage,
                                               &ctx->common_data[stage],
                                               &ctx->stage_data,
                                               explicit_const_usage);

         continue;
      }

      if (!ctx->nir[stage])
         continue;

      ctx->rogue[stage] = pvr_nir_to_rogue(ctx, ctx->nir[stage]);
      if (!ctx->rogue[stage]) {
         ralloc_free(ctx);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      pvr_rogue_to_binary(ctx, ctx->rogue[stage], &ctx->binary[stage]);
      if (!ctx->binary[stage].size) {
         ralloc_free(ctx);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }
   }

   if (pvr_has_hard_coded_shaders(&device->pdevice->dev_info) &&
       pvr_hard_code_graphics_get_flags(&device->pdevice->dev_info) &
          BITFIELD_BIT(MESA_SHADER_VERTEX)) {
      pvr_hard_code_graphics_vertex_state(&device->pdevice->dev_info,
                                          hard_code_pipeline_n,
                                          &gfx_pipeline->shader_state.vertex);
   } else {
      pvr_vertex_state_init(gfx_pipeline,
                            &ctx->common_data[MESA_SHADER_VERTEX],
                            *vertex_input_reg_count,
                            &ctx->stage_data.vs);

      if (!old_path) {
         struct pvr_vertex_shader_state *vertex_state =
            &gfx_pipeline->shader_state.vertex;

         /* FIXME: For now we just overwrite it but the compiler shouldn't be
          * returning the sh count since the driver is in charge of allocating
          * them.
          */
         vertex_state->stage_state.const_shared_reg_count =
            sh_count[PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY];

         gfx_pipeline->shader_state.vertex.vertex_input_size =
            ctx->stage_data.vs.num_vertex_input_regs;
      }
   }

   result =
      pvr_gpu_upload_usc(device,
                         util_dynarray_begin(&ctx->binary[MESA_SHADER_VERTEX]),
                         ctx->binary[MESA_SHADER_VERTEX].size,
                         cache_line_size,
                         &gfx_pipeline->shader_state.vertex.bo);
   if (result != VK_SUCCESS)
      goto err_free_build_context;

   if (ctx->nir[MESA_SHADER_FRAGMENT]) {
      struct pvr_fragment_shader_state *fragment_state =
         &gfx_pipeline->shader_state.fragment;

      if (pvr_has_hard_coded_shaders(&device->pdevice->dev_info) &&
          pvr_hard_code_graphics_get_flags(&device->pdevice->dev_info) &
             BITFIELD_BIT(MESA_SHADER_FRAGMENT)) {
         pvr_hard_code_graphics_fragment_state(
            &device->pdevice->dev_info,
            hard_code_pipeline_n,
            &gfx_pipeline->shader_state.fragment);
      } else {
         pvr_fragment_state_init(gfx_pipeline,
                                 &ctx->common_data[MESA_SHADER_FRAGMENT]);

         if (!old_path) {
            /* FIXME: For now we just overwrite it but the compiler shouldn't be
             * returning the sh count since the driver is in charge of
             * allocating them.
             */
            fragment_state->stage_state.const_shared_reg_count =
               sh_count[PVR_STAGE_ALLOCATION_FRAGMENT];
         }
      }

      result = pvr_gpu_upload_usc(
         device,
         util_dynarray_begin(&ctx->binary[MESA_SHADER_FRAGMENT]),
         ctx->binary[MESA_SHADER_FRAGMENT].size,
         cache_line_size,
         &gfx_pipeline->shader_state.fragment.bo);
      if (result != VK_SUCCESS)
         goto err_free_vertex_bo;

      /* TODO: powervr has an optimization where it attempts to recompile
       * shaders. See PipelineCompileNoISPFeedbackFragmentStage. Unimplemented
       * since in our case the optimization doesn't happen.
       */

      result = pvr_pds_coeff_program_create_and_upload(
         device,
         allocator,
         ctx->stage_data.fs.iterator_args.fpu_iterators,
         ctx->stage_data.fs.iterator_args.num_fpu_iterators,
         ctx->stage_data.fs.iterator_args.destination,
         &fragment_state->pds_coeff_program,
         &fragment_state->stage_state.pds_temps_count);
      if (result != VK_SUCCESS)
         goto err_free_fragment_bo;

      result = pvr_pds_fragment_program_create_and_upload(
         device,
         allocator,
         gfx_pipeline->shader_state.fragment.bo,
         ctx->common_data[MESA_SHADER_FRAGMENT].temps,
         ctx->stage_data.fs.msaa_mode,
         ctx->stage_data.fs.phas,
         &fragment_state->pds_fragment_program);
      if (result != VK_SUCCESS)
         goto err_free_coeff_program;

      /* FIXME: For now we pass in the same explicit_const_usage since it
       * contains all invalid entries. Fix this by hooking it up to the
       * compiler.
       */
      result = pvr_pds_descriptor_program_create_and_upload(
         device,
         allocator,
         &ctx->common_data[MESA_SHADER_FRAGMENT].compile_time_consts_data,
         &ctx->common_data[MESA_SHADER_FRAGMENT].ubo_data,
         &frag_explicit_const_usage,
         layout,
         PVR_STAGE_ALLOCATION_FRAGMENT,
         sh_reg_layout_frag,
         &fragment_state->descriptor_state);
      if (result != VK_SUCCESS)
         goto err_free_frag_program;

      /* If not, we need to MAX2() and set
       * `fragment_state->stage_state.pds_temps_count` appropriately.
       */
      assert(fragment_state->descriptor_state.pds_info.temps_required == 0);
   }

   result = pvr_pds_vertex_attrib_programs_create_and_upload(
      device,
      allocator,
      vertex_input_state,
      ctx->common_data[MESA_SHADER_VERTEX].temps,
      &ctx->stage_data.vs,
      vtx_dma_descriptions,
      vtx_dma_count,
      &special_vars_layout,
      &gfx_pipeline->shader_state.vertex.pds_attrib_programs);
   if (result != VK_SUCCESS)
      goto err_free_frag_descriptor_program;

   result = pvr_pds_descriptor_program_create_and_upload(
      device,
      allocator,
      &ctx->common_data[MESA_SHADER_VERTEX].compile_time_consts_data,
      &ctx->common_data[MESA_SHADER_VERTEX].ubo_data,
      &vert_explicit_const_usage,
      layout,
      PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY,
      sh_reg_layout_vert,
      &gfx_pipeline->shader_state.vertex.descriptor_state);
   if (result != VK_SUCCESS)
      goto err_free_vertex_attrib_program;

   /* FIXME: When the temp_buffer_total_size is non-zero we need to allocate a
    * scratch buffer for both vertex and fragment stage.
    * Figure out the best place to do this.
    */
   /* assert(pvr_pds_descriptor_program_variables.temp_buff_total_size == 0); */
   /* TODO: Implement spilling with the above. */

   ralloc_free(ctx);

   hard_code_pipeline_n++;

   return VK_SUCCESS;

err_free_vertex_attrib_program:
   for (uint32_t i = 0;
        i < ARRAY_SIZE(gfx_pipeline->shader_state.vertex.pds_attrib_programs);
        i++) {
      struct pvr_pds_attrib_program *const attrib_program =
         &gfx_pipeline->shader_state.vertex.pds_attrib_programs[i];

      pvr_pds_vertex_attrib_program_destroy(device, allocator, attrib_program);
   }
err_free_frag_descriptor_program:
   pvr_pds_descriptor_program_destroy(
      device,
      allocator,
      &gfx_pipeline->shader_state.fragment.descriptor_state);
err_free_frag_program:
   pvr_bo_suballoc_free(
      gfx_pipeline->shader_state.fragment.pds_fragment_program.pvr_bo);
err_free_coeff_program:
   pvr_bo_suballoc_free(
      gfx_pipeline->shader_state.fragment.pds_coeff_program.pvr_bo);
err_free_fragment_bo:
   pvr_bo_suballoc_free(gfx_pipeline->shader_state.fragment.bo);
err_free_vertex_bo:
   pvr_bo_suballoc_free(gfx_pipeline->shader_state.vertex.bo);
err_free_build_context:
   ralloc_free(ctx);
   return result;
}

static struct vk_render_pass_state
pvr_create_renderpass_state(const VkGraphicsPipelineCreateInfo *const info)
{
   PVR_FROM_HANDLE(pvr_render_pass, pass, info->renderPass);
   const struct pvr_render_subpass *const subpass =
      &pass->subpasses[info->subpass];

   VkImageAspectFlags attachment_aspects = VK_IMAGE_ASPECT_NONE;

   assert(info->subpass < pass->subpass_count);

   for (uint32_t i = 0; i < subpass->color_count; i++) {
      attachment_aspects |=
         pass->attachments[subpass->color_attachments[i]].aspects;
   }

   if (subpass->depth_stencil_attachment != VK_ATTACHMENT_UNUSED) {
      attachment_aspects |=
         pass->attachments[subpass->depth_stencil_attachment].aspects;
   }

   return (struct vk_render_pass_state){
      .attachment_aspects = attachment_aspects,

      /* TODO: This is only needed for VK_KHR_create_renderpass2 (or core 1.2),
       * which is not currently supported.
       */
      .view_mask = 0,
   };
}

static VkResult
pvr_graphics_pipeline_init(struct pvr_device *device,
                           struct vk_pipeline_cache *cache,
                           const VkGraphicsPipelineCreateInfo *pCreateInfo,
                           const VkAllocationCallbacks *allocator,
                           struct pvr_graphics_pipeline *gfx_pipeline)
{
   struct vk_dynamic_graphics_state *const dynamic_state =
      &gfx_pipeline->dynamic_state;
   const struct vk_render_pass_state rp_state =
      pvr_create_renderpass_state(pCreateInfo);

   struct vk_graphics_pipeline_all_state all_state;
   struct vk_graphics_pipeline_state state = { 0 };

   VkResult result;

   pvr_pipeline_init(device, PVR_PIPELINE_TYPE_GRAPHICS, &gfx_pipeline->base);

   result = vk_graphics_pipeline_state_fill(&device->vk,
                                            &state,
                                            pCreateInfo,
                                            &rp_state,
                                            0,
                                            &all_state,
                                            NULL,
                                            0,
                                            NULL);
   if (result != VK_SUCCESS)
      goto err_pipeline_finish;

   vk_dynamic_graphics_state_init(dynamic_state);

   /* Load static state into base dynamic state holder. */
   vk_dynamic_graphics_state_fill(dynamic_state, &state);

   /* The value of ms.rasterization_samples is undefined when
    * rasterizer_discard_enable is set, but we need a specific value.
    * Fill that in here.
    */
   if (state.rs->rasterizer_discard_enable)
      dynamic_state->ms.rasterization_samples = VK_SAMPLE_COUNT_1_BIT;

   memset(gfx_pipeline->stage_indices, ~0, sizeof(gfx_pipeline->stage_indices));

   for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
      VkShaderStageFlagBits vk_stage = pCreateInfo->pStages[i].stage;
      gl_shader_stage gl_stage = vk_to_mesa_shader_stage(vk_stage);
      /* From the Vulkan 1.2.192 spec for VkPipelineShaderStageCreateInfo:
       *
       *    "stage must not be VK_SHADER_STAGE_ALL_GRAPHICS,
       *    or VK_SHADER_STAGE_ALL."
       *
       * So we don't handle that.
       *
       * We also don't handle VK_SHADER_STAGE_TESSELLATION_* and
       * VK_SHADER_STAGE_GEOMETRY_BIT stages as 'tessellationShader' and
       * 'geometryShader' are set to false in the VkPhysicalDeviceFeatures
       * structure returned by the driver.
       */
      switch (pCreateInfo->pStages[i].stage) {
      case VK_SHADER_STAGE_VERTEX_BIT:
      case VK_SHADER_STAGE_FRAGMENT_BIT:
         gfx_pipeline->stage_indices[gl_stage] = i;
         break;
      default:
         unreachable("Unsupported stage.");
      }
   }

   gfx_pipeline->base.layout =
      pvr_pipeline_layout_from_handle(pCreateInfo->layout);

   /* Compiles and uploads shaders and PDS programs. */
   result = pvr_graphics_pipeline_compile(device,
                                          cache,
                                          pCreateInfo,
                                          allocator,
                                          gfx_pipeline);
   if (result != VK_SUCCESS)
      goto err_pipeline_finish;

   return VK_SUCCESS;

err_pipeline_finish:
   pvr_pipeline_finish(&gfx_pipeline->base);

   return result;
}

/* If allocator == NULL, the internal one will be used. */
static VkResult
pvr_graphics_pipeline_create(struct pvr_device *device,
                             struct vk_pipeline_cache *cache,
                             const VkGraphicsPipelineCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *allocator,
                             VkPipeline *const pipeline_out)
{
   struct pvr_graphics_pipeline *gfx_pipeline;
   VkResult result;

   gfx_pipeline = vk_zalloc2(&device->vk.alloc,
                             allocator,
                             sizeof(*gfx_pipeline),
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!gfx_pipeline)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* Compiles and uploads shaders and PDS programs too. */
   result = pvr_graphics_pipeline_init(device,
                                       cache,
                                       pCreateInfo,
                                       allocator,
                                       gfx_pipeline);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, allocator, gfx_pipeline);
      return result;
   }

   *pipeline_out = pvr_pipeline_to_handle(&gfx_pipeline->base);

   return VK_SUCCESS;
}

VkResult
pvr_CreateGraphicsPipelines(VkDevice _device,
                            VkPipelineCache pipelineCache,
                            uint32_t createInfoCount,
                            const VkGraphicsPipelineCreateInfo *pCreateInfos,
                            const VkAllocationCallbacks *pAllocator,
                            VkPipeline *pPipelines)
{
   VK_FROM_HANDLE(vk_pipeline_cache, cache, pipelineCache);
   PVR_FROM_HANDLE(pvr_device, device, _device);
   VkResult result = VK_SUCCESS;

   for (uint32_t i = 0; i < createInfoCount; i++) {
      const VkResult local_result =
         pvr_graphics_pipeline_create(device,
                                      cache,
                                      &pCreateInfos[i],
                                      pAllocator,
                                      &pPipelines[i]);
      if (local_result != VK_SUCCESS) {
         result = local_result;
         pPipelines[i] = VK_NULL_HANDLE;
      }
   }

   return result;
}

/*****************************************************************************
   Other functions
*****************************************************************************/

void pvr_DestroyPipeline(VkDevice _device,
                         VkPipeline _pipeline,
                         const VkAllocationCallbacks *pAllocator)
{
   PVR_FROM_HANDLE(pvr_pipeline, pipeline, _pipeline);
   PVR_FROM_HANDLE(pvr_device, device, _device);

   if (!pipeline)
      return;

   switch (pipeline->type) {
   case PVR_PIPELINE_TYPE_GRAPHICS: {
      struct pvr_graphics_pipeline *const gfx_pipeline =
         to_pvr_graphics_pipeline(pipeline);

      pvr_graphics_pipeline_destroy(device, pAllocator, gfx_pipeline);
      break;
   }

   case PVR_PIPELINE_TYPE_COMPUTE: {
      struct pvr_compute_pipeline *const compute_pipeline =
         to_pvr_compute_pipeline(pipeline);

      pvr_compute_pipeline_destroy(device, pAllocator, compute_pipeline);
      break;
   }

   default:
      unreachable("Unknown pipeline type.");
   }
}
