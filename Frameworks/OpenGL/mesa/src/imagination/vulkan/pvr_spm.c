/*
 * Copyright © 2023 Imagination Technologies Ltd.
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
#include <stddef.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "c11/threads.h"
#include "hwdef/rogue_hw_utils.h"
#include "pvr_bo.h"
#include "pvr_csb.h"
#include "pvr_csb_enum_helpers.h"
#include "pvr_device_info.h"
#include "pvr_formats.h"
#include "pvr_hw_pass.h"
#include "pvr_job_common.h"
#include "pvr_pds.h"
#include "pvr_private.h"
#include "pvr_shader_factory.h"
#include "pvr_spm.h"
#include "pvr_static_shaders.h"
#include "pvr_tex_state.h"
#include "pvr_types.h"
#include "pvr_uscgen.h"
#include "util/bitscan.h"
#include "util/macros.h"
#include "util/simple_mtx.h"
#include "util/u_atomic.h"
#include "vk_alloc.h"
#include "vk_log.h"

struct pvr_spm_scratch_buffer {
   uint32_t ref_count;
   struct pvr_bo *bo;
   uint64_t size;
};

void pvr_spm_init_scratch_buffer_store(struct pvr_device *device)
{
   struct pvr_spm_scratch_buffer_store *store =
      &device->spm_scratch_buffer_store;

   simple_mtx_init(&store->mtx, mtx_plain);
   store->head_ref = NULL;
}

void pvr_spm_finish_scratch_buffer_store(struct pvr_device *device)
{
   struct pvr_spm_scratch_buffer_store *store =
      &device->spm_scratch_buffer_store;

   /* Either a framebuffer was never created so no scratch buffer was ever
    * created or all framebuffers have been freed so only the store's reference
    * remains.
    */
   assert(!store->head_ref || p_atomic_read(&store->head_ref->ref_count) == 1);

   simple_mtx_destroy(&store->mtx);

   if (store->head_ref) {
      pvr_bo_free(device, store->head_ref->bo);
      vk_free(&device->vk.alloc, store->head_ref);
   }
}

uint64_t
pvr_spm_scratch_buffer_calc_required_size(const struct pvr_render_pass *pass,
                                          uint32_t framebuffer_width,
                                          uint32_t framebuffer_height)
{
   uint64_t dwords_per_pixel;
   uint64_t buffer_size;

   /* If we're allocating an SPM scratch buffer we'll have a minimum of 1 output
    * reg and/or tile_buffer.
    */
   uint32_t nr_tile_buffers = 1;
   uint32_t nr_output_regs = 1;

   for (uint32_t i = 0; i < pass->hw_setup->render_count; i++) {
      const struct pvr_renderpass_hwsetup_render *hw_render =
         &pass->hw_setup->renders[i];

      nr_tile_buffers = MAX2(nr_tile_buffers, hw_render->tile_buffers_count);
      nr_output_regs = MAX2(nr_output_regs, hw_render->output_regs_count);
   }

   dwords_per_pixel =
      (uint64_t)pass->max_sample_count * nr_output_regs * nr_tile_buffers;

   buffer_size = ALIGN_POT((uint64_t)framebuffer_width,
                           PVRX(CR_PBE_WORD0_MRT0_LINESTRIDE_ALIGNMENT));
   buffer_size *=
      (uint64_t)framebuffer_height * PVR_DW_TO_BYTES(dwords_per_pixel);

   return buffer_size;
}

static VkResult
pvr_spm_scratch_buffer_alloc(struct pvr_device *device,
                             uint64_t size,
                             struct pvr_spm_scratch_buffer **const buffer_out)
{
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
   struct pvr_spm_scratch_buffer *scratch_buffer;
   struct pvr_bo *bo;
   VkResult result;

   result = pvr_bo_alloc(device,
                         device->heaps.general_heap,
                         size,
                         cache_line_size,
                         0,
                         &bo);
   if (result != VK_SUCCESS) {
      *buffer_out = NULL;
      return result;
   }

   scratch_buffer = vk_alloc(&device->vk.alloc,
                             sizeof(*scratch_buffer),
                             4,
                             VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!scratch_buffer) {
      pvr_bo_free(device, bo);
      *buffer_out = NULL;
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   *scratch_buffer = (struct pvr_spm_scratch_buffer){
      .bo = bo,
      .size = size,
   };

   *buffer_out = scratch_buffer;

   return VK_SUCCESS;
}

static void
pvr_spm_scratch_buffer_release_locked(struct pvr_device *device,
                                      struct pvr_spm_scratch_buffer *buffer)
{
   struct pvr_spm_scratch_buffer_store *store =
      &device->spm_scratch_buffer_store;

   simple_mtx_assert_locked(&store->mtx);

   if (p_atomic_dec_zero(&buffer->ref_count)) {
      pvr_bo_free(device, buffer->bo);
      vk_free(&device->vk.alloc, buffer);
   }
}

void pvr_spm_scratch_buffer_release(struct pvr_device *device,
                                    struct pvr_spm_scratch_buffer *buffer)
{
   struct pvr_spm_scratch_buffer_store *store =
      &device->spm_scratch_buffer_store;

   simple_mtx_lock(&store->mtx);

   pvr_spm_scratch_buffer_release_locked(device, buffer);

   simple_mtx_unlock(&store->mtx);
}

static void pvr_spm_scratch_buffer_store_set_head_ref_locked(
   struct pvr_spm_scratch_buffer_store *store,
   struct pvr_spm_scratch_buffer *buffer)
{
   simple_mtx_assert_locked(&store->mtx);
   assert(!store->head_ref);

   p_atomic_inc(&buffer->ref_count);
   store->head_ref = buffer;
}

static void pvr_spm_scratch_buffer_store_release_head_ref_locked(
   struct pvr_device *device,
   struct pvr_spm_scratch_buffer_store *store)
{
   simple_mtx_assert_locked(&store->mtx);

   pvr_spm_scratch_buffer_release_locked(device, store->head_ref);

   store->head_ref = NULL;
}

VkResult pvr_spm_scratch_buffer_get_buffer(
   struct pvr_device *device,
   uint64_t size,
   struct pvr_spm_scratch_buffer **const buffer_out)
{
   struct pvr_spm_scratch_buffer_store *store =
      &device->spm_scratch_buffer_store;
   struct pvr_spm_scratch_buffer *buffer;

   simple_mtx_lock(&store->mtx);

   /* When a render requires a PR the fw will wait for other renders to end,
    * free the PB space, unschedule any other vert/frag jobs and solely run the
    * PR on the whole device until completion.
    * Thus we can safely use the same scratch buffer across multiple
    * framebuffers as the scratch buffer is only used during PRs and only one PR
    * can ever be executed at any one time.
    */
   if (store->head_ref && store->head_ref->size <= size) {
      buffer = store->head_ref;
   } else {
      VkResult result;

      if (store->head_ref)
         pvr_spm_scratch_buffer_store_release_head_ref_locked(device, store);

      result = pvr_spm_scratch_buffer_alloc(device, size, &buffer);
      if (result != VK_SUCCESS) {
         simple_mtx_unlock(&store->mtx);
         *buffer_out = NULL;

         return result;
      }

      pvr_spm_scratch_buffer_store_set_head_ref_locked(store, buffer);
   }

   p_atomic_inc(&buffer->ref_count);
   simple_mtx_unlock(&store->mtx);
   *buffer_out = buffer;

   return VK_SUCCESS;
}

VkResult pvr_device_init_spm_load_state(struct pvr_device *device)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   uint32_t pds_texture_aligned_offsets[PVR_SPM_LOAD_PROGRAM_COUNT];
   uint32_t pds_kick_aligned_offsets[PVR_SPM_LOAD_PROGRAM_COUNT];
   uint32_t usc_aligned_offsets[PVR_SPM_LOAD_PROGRAM_COUNT];
   uint32_t pds_allocation_size = 0;
   uint32_t usc_allocation_size = 0;
   struct pvr_suballoc_bo *pds_bo;
   struct pvr_suballoc_bo *usc_bo;
   uint8_t *mem_ptr;
   VkResult result;

   static_assert(PVR_SPM_LOAD_PROGRAM_COUNT == ARRAY_SIZE(spm_load_collection),
                 "Size mismatch");

   /* TODO: We don't need to upload all the programs since the set contains
    * programs for devices with 8 output regs as well. We can save some memory
    * by not uploading them on devices without the feature.
    * It's likely that once the compiler is hooked up we'll be using the shader
    * cache and generate the shaders as needed so this todo will be unnecessary.
    */

   /* Upload USC shaders. */

   for (uint32_t i = 0; i < ARRAY_SIZE(spm_load_collection); i++) {
      usc_aligned_offsets[i] = usc_allocation_size;
      usc_allocation_size += ALIGN_POT(spm_load_collection[i].size, 4);
   }

   result = pvr_bo_suballoc(&device->suballoc_usc,
                            usc_allocation_size,
                            4,
                            false,
                            &usc_bo);
   if (result != VK_SUCCESS)
      return result;

   mem_ptr = (uint8_t *)pvr_bo_suballoc_get_map_addr(usc_bo);

   for (uint32_t i = 0; i < ARRAY_SIZE(spm_load_collection); i++) {
      memcpy(mem_ptr + usc_aligned_offsets[i],
             spm_load_collection[i].code,
             spm_load_collection[i].size);
   }

   /* Upload PDS programs. */

   for (uint32_t i = 0; i < ARRAY_SIZE(spm_load_collection); i++) {
      struct pvr_pds_pixel_shader_sa_program pds_texture_program = {
         /* DMA for clear colors and tile buffer address parts. */
         .num_texture_dma_kicks = 1,
      };
      struct pvr_pds_kickusc_program pds_kick_program = { 0 };

      /* TODO: This looks a bit odd and isn't consistent with other code where
       * we're getting the size of the PDS program. Can we improve this?
       */
      pvr_pds_set_sizes_pixel_shader_uniform_texture_code(&pds_texture_program);
      pvr_pds_set_sizes_pixel_shader_sa_texture_data(&pds_texture_program,
                                                     dev_info);

      /* TODO: Looking at the pvr_pds_generate_...() functions and the run-time
       * behavior the data size is always the same here. Should we try saving
       * some memory by adjusting things based on that?
       */
      device->spm_load_state.load_program[i].pds_texture_program_data_size =
         pds_texture_program.data_size;

      pds_texture_aligned_offsets[i] = pds_allocation_size;
      /* FIXME: Figure out the define for alignment of 16. */
      pds_allocation_size +=
         ALIGN_POT(PVR_DW_TO_BYTES(pds_texture_program.code_size), 16);

      pvr_pds_set_sizes_pixel_shader(&pds_kick_program);

      pds_kick_aligned_offsets[i] = pds_allocation_size;
      /* FIXME: Figure out the define for alignment of 16. */
      pds_allocation_size +=
         ALIGN_POT(PVR_DW_TO_BYTES(pds_kick_program.code_size +
                                   pds_kick_program.data_size),
                   16);
   }

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_bo_suballoc(&device->suballoc_pds,
                            pds_allocation_size,
                            16,
                            false,
                            &pds_bo);
   if (result != VK_SUCCESS) {
      pvr_bo_suballoc_free(usc_bo);
      return result;
   }

   mem_ptr = (uint8_t *)pvr_bo_suballoc_get_map_addr(pds_bo);

   for (uint32_t i = 0; i < ARRAY_SIZE(spm_load_collection); i++) {
      struct pvr_pds_pixel_shader_sa_program pds_texture_program = {
         /* DMA for clear colors and tile buffer address parts. */
         .num_texture_dma_kicks = 1,
      };
      const pvr_dev_addr_t usc_program_dev_addr =
         PVR_DEV_ADDR_OFFSET(usc_bo->dev_addr, usc_aligned_offsets[i]);
      struct pvr_pds_kickusc_program pds_kick_program = { 0 };
      enum PVRX(PDSINST_DOUTU_SAMPLE_RATE) sample_rate;

      pvr_pds_generate_pixel_shader_sa_code_segment(
         &pds_texture_program,
         (uint32_t *)(mem_ptr + pds_texture_aligned_offsets[i]));

      if (spm_load_collection[i].info->msaa_sample_count > 1)
         sample_rate = PVRX(PDSINST_DOUTU_SAMPLE_RATE_FULL);
      else
         sample_rate = PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE);

      pvr_pds_setup_doutu(&pds_kick_program.usc_task_control,
                          usc_program_dev_addr.addr,
                          spm_load_collection[i].info->temps_required,
                          sample_rate,
                          false);

      /* Generated both code and data. */
      pvr_pds_generate_pixel_shader_program(
         &pds_kick_program,
         (uint32_t *)(mem_ptr + pds_kick_aligned_offsets[i]));

      device->spm_load_state.load_program[i].pds_pixel_program_offset =
         PVR_DEV_ADDR_OFFSET(pds_bo->dev_addr, pds_kick_aligned_offsets[i]);
      device->spm_load_state.load_program[i].pds_uniform_program_offset =
         PVR_DEV_ADDR_OFFSET(pds_bo->dev_addr, pds_texture_aligned_offsets[i]);

      /* TODO: From looking at the pvr_pds_generate_...() functions, it seems
       * like temps_used is always 1. Should we remove this and hard code it
       * with a define in the PDS code?
       */
      device->spm_load_state.load_program[i].pds_texture_program_temps_count =
         pds_texture_program.temps_used;
   }

   device->spm_load_state.usc_programs = usc_bo;
   device->spm_load_state.pds_programs = pds_bo;

   return VK_SUCCESS;
}

void pvr_device_finish_spm_load_state(struct pvr_device *device)
{
   pvr_bo_suballoc_free(device->spm_load_state.pds_programs);
   pvr_bo_suballoc_free(device->spm_load_state.usc_programs);
}

static inline enum PVRX(PBESTATE_PACKMODE)
   pvr_spm_get_pbe_packmode(uint32_t dword_count)
{
   switch (dword_count) {
   case 1:
      return PVRX(PBESTATE_PACKMODE_U32);
   case 2:
      return PVRX(PBESTATE_PACKMODE_U32U32);
   case 3:
      return PVRX(PBESTATE_PACKMODE_U32U32U32);
   case 4:
      return PVRX(PBESTATE_PACKMODE_U32U32U32U32);
   default:
      unreachable("Unsupported dword_count");
   }
}

/**
 * \brief Sets up PBE registers and state values per a single render output.
 *
 * On a PR we want to store tile data to the scratch buffer so we need to
 * setup the Pixel Back End (PBE) to write the data to the scratch buffer. This
 * function sets up the PBE state and register values required to do so, for a
 * single resource whether it be a tile buffer or the output register set.
 *
 * \return Size of the data saved into the scratch buffer in bytes.
 */
static uint64_t pvr_spm_setup_pbe_state(
   const struct pvr_device_info *dev_info,
   const VkExtent2D *framebuffer_size,
   uint32_t dword_count,
   enum pvr_pbe_source_start_pos source_start,
   uint32_t sample_count,
   pvr_dev_addr_t scratch_buffer_addr,
   uint32_t pbe_state_words_out[static const ROGUE_NUM_PBESTATE_STATE_WORDS],
   uint64_t pbe_reg_words_out[static const ROGUE_NUM_PBESTATE_REG_WORDS])
{
   const uint32_t stride =
      ALIGN_POT(framebuffer_size->width,
                PVRX(PBESTATE_REG_WORD0_LINESTRIDE_UNIT_SIZE));

   const struct pvr_pbe_surf_params surface_params = {
      .swizzle = {
         [0] = PIPE_SWIZZLE_X,
         [1] = PIPE_SWIZZLE_Y,
         [2] = PIPE_SWIZZLE_Z,
         [3] = PIPE_SWIZZLE_W,
      },
      .pbe_packmode = pvr_spm_get_pbe_packmode(dword_count),
      .source_format = PVRX(PBESTATE_SOURCE_FORMAT_8_PER_CHANNEL),
      .addr = scratch_buffer_addr,
      .mem_layout = PVR_MEMLAYOUT_LINEAR,
      .stride = stride,
   };
   const struct pvr_pbe_render_params render_params = {
      .max_x_clip = framebuffer_size->width - 1,
      .max_y_clip = framebuffer_size->height - 1,
      .source_start = source_start,
   };

   pvr_pbe_pack_state(dev_info,
                      &surface_params,
                      &render_params,
                      pbe_state_words_out,
                      pbe_reg_words_out);

   return (uint64_t)stride * framebuffer_size->height * sample_count *
          PVR_DW_TO_BYTES(dword_count);
}

static inline void pvr_set_pbe_all_valid_mask(struct usc_mrt_desc *desc)
{
   for (uint32_t i = 0; i < ARRAY_SIZE(desc->valid_mask); i++)
      desc->valid_mask[i] = ~0;
}

#define PVR_DEV_ADDR_ADVANCE(_addr, _offset) \
   _addr = PVR_DEV_ADDR_OFFSET(_addr, _offset)

/**
 * \brief Sets up PBE registers, PBE state values and MRT data per a single
 * render output requiring 8 dwords to be written.
 *
 * On a PR we want to store tile data to the scratch buffer so we need to
 * setup the Pixel Back End (PBE) to write the data to the scratch buffer, as
 * well as setup the Multiple Render Target (MRT) info so the compiler knows
 * what data needs to be stored (output regs or tile buffers) and generate the
 * appropriate EOT shader.
 *
 * This function is only available for devices with the eight_output_registers
 * feature thus requiring 8 dwords to be stored.
 *
 * \return Size of the data saved into the scratch buffer in bytes.
 */
static uint64_t pvr_spm_setup_pbe_eight_dword_write(
   const struct pvr_device_info *dev_info,
   const VkExtent2D *framebuffer_size,
   uint32_t sample_count,
   enum usc_mrt_resource_type source_type,
   uint32_t tile_buffer_idx,
   pvr_dev_addr_t scratch_buffer_addr,
   uint32_t pbe_state_word_0_out[static const ROGUE_NUM_PBESTATE_STATE_WORDS],
   uint32_t pbe_state_word_1_out[static const ROGUE_NUM_PBESTATE_STATE_WORDS],
   uint64_t pbe_reg_word_0_out[static const ROGUE_NUM_PBESTATE_REG_WORDS],
   uint64_t pbe_reg_word_1_out[static const ROGUE_NUM_PBESTATE_REG_WORDS],
   uint32_t *render_target_used_out)
{
   const uint32_t max_pbe_write_size_dw = 4;
   uint32_t render_target_used = 0;
   uint64_t mem_stored;

   assert(PVR_HAS_FEATURE(dev_info, eight_output_registers));
   assert(source_type != USC_MRT_RESOURCE_TYPE_INVALID);

   /* To store 8 dwords we need to split this into two
    * ROGUE_PBESTATE_PACKMODE_U32U32U32U32 stores with the second one using
    * PVR_PBE_STARTPOS_BIT128 as the source offset to store the last 4 dwords.
    */

   mem_stored = pvr_spm_setup_pbe_state(dev_info,
                                        framebuffer_size,
                                        max_pbe_write_size_dw,
                                        PVR_PBE_STARTPOS_BIT0,
                                        sample_count,
                                        scratch_buffer_addr,
                                        pbe_state_word_0_out,
                                        pbe_reg_word_0_out);

   PVR_DEV_ADDR_ADVANCE(scratch_buffer_addr, mem_stored);

   render_target_used++;

   mem_stored += pvr_spm_setup_pbe_state(dev_info,
                                         framebuffer_size,
                                         max_pbe_write_size_dw,
                                         PVR_PBE_STARTPOS_BIT128,
                                         sample_count,
                                         scratch_buffer_addr,
                                         pbe_state_word_1_out,
                                         pbe_reg_word_1_out);

   PVR_DEV_ADDR_ADVANCE(scratch_buffer_addr, mem_stored);

   render_target_used++;
   *render_target_used_out = render_target_used;

   return mem_stored;
}

/**
 * \brief Create and upload the EOT PDS program.
 *
 * Essentially DOUTU the USC EOT shader.
 */
/* TODO: See if we can dedup this with
 * pvr_sub_cmd_gfx_per_job_fragment_programs_create_and_upload().
 */
static VkResult pvr_pds_pixel_event_program_create_and_upload(
   struct pvr_device *device,
   const struct pvr_suballoc_bo *usc_eot_program,
   uint32_t usc_temp_count,
   struct pvr_pds_upload *const pds_upload_out)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   struct pvr_pds_event_program program = { 0 };
   uint32_t *staging_buffer;
   VkResult result;

   pvr_pds_setup_doutu(&program.task_control,
                       usc_eot_program->dev_addr.addr,
                       usc_temp_count,
                       PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);

   staging_buffer =
      vk_alloc(&device->vk.alloc,
               PVR_DW_TO_BYTES(device->pixel_event_data_size_in_dwords),
               8,
               VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pvr_pds_generate_pixel_event_data_segment(&program,
                                             staging_buffer,
                                             dev_info);

   result = pvr_gpu_upload_pds(device,
                               staging_buffer,
                               device->pixel_event_data_size_in_dwords,
                               4,
                               NULL,
                               0,
                               0,
                               4,
                               pds_upload_out);
   vk_free(&device->vk.alloc, staging_buffer);
   return result;
}

/**
 * \brief Sets up the End of Tile (EOT) program for SPM.
 *
 * This sets up an EOT program to store the render pass'es on-chip and
 * off-chip tile data to the SPM scratch buffer on the EOT event.
 */
VkResult
pvr_spm_init_eot_state(struct pvr_device *device,
                       struct pvr_spm_eot_state *spm_eot_state,
                       const struct pvr_framebuffer *framebuffer,
                       const struct pvr_renderpass_hwsetup_render *hw_render,
                       uint32_t *emit_count_out)
{
   const VkExtent2D framebuffer_size = {
      .width = framebuffer->width,
      .height = framebuffer->height,
   };
   uint32_t pbe_state_words[PVR_MAX_COLOR_ATTACHMENTS]
                           [ROGUE_NUM_PBESTATE_STATE_WORDS];
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   uint32_t total_render_target_used = 0;
   struct pvr_pds_upload pds_eot_program;
   struct util_dynarray usc_shader_binary;
   uint32_t usc_temp_count;
   VkResult result;

   pvr_dev_addr_t next_scratch_buffer_addr =
      framebuffer->scratch_buffer->bo->vma->dev_addr;
   uint64_t mem_stored;

   /* TODO: See if instead of having a separate path for devices with 8 output
    * regs we can instead do this in a loop and dedup some stuff.
    */
   assert(util_is_power_of_two_or_zero(hw_render->output_regs_count) &&
          hw_render->output_regs_count <= 8);
   if (hw_render->output_regs_count == 8) {
      uint32_t render_targets_used;

      /* Store on-chip tile data (i.e. output regs). */

      mem_stored = pvr_spm_setup_pbe_eight_dword_write(
         dev_info,
         &framebuffer_size,
         hw_render->sample_count,
         USC_MRT_RESOURCE_TYPE_OUTPUT_REG,
         0,
         next_scratch_buffer_addr,
         pbe_state_words[total_render_target_used],
         pbe_state_words[total_render_target_used + 1],
         spm_eot_state->pbe_reg_words[total_render_target_used],
         spm_eot_state->pbe_reg_words[total_render_target_used + 1],
         &render_targets_used);

      PVR_DEV_ADDR_ADVANCE(next_scratch_buffer_addr, mem_stored);
      total_render_target_used += render_targets_used;

      /* Store off-chip tile data (i.e. tile buffers). */

      for (uint32_t i = 0; i < hw_render->tile_buffers_count; i++) {
         assert(!"Add support for tile buffers in EOT");
         pvr_finishme("Add support for tile buffers in EOT");

         /* `+ 1` since we have 2 emits per tile buffer. */
         assert(total_render_target_used + 1 < PVR_MAX_COLOR_ATTACHMENTS);

         mem_stored = pvr_spm_setup_pbe_eight_dword_write(
            dev_info,
            &framebuffer_size,
            hw_render->sample_count,
            USC_MRT_RESOURCE_TYPE_MEMORY,
            i,
            next_scratch_buffer_addr,
            pbe_state_words[total_render_target_used],
            pbe_state_words[total_render_target_used + 1],
            spm_eot_state->pbe_reg_words[total_render_target_used],
            spm_eot_state->pbe_reg_words[total_render_target_used + 1],
            &render_targets_used);

         PVR_DEV_ADDR_ADVANCE(next_scratch_buffer_addr, mem_stored);
         total_render_target_used += render_targets_used;
      }
   } else {
      /* Store on-chip tile data (i.e. output regs). */

      mem_stored = pvr_spm_setup_pbe_state(
         dev_info,
         &framebuffer_size,
         hw_render->output_regs_count,
         PVR_PBE_STARTPOS_BIT0,
         hw_render->sample_count,
         next_scratch_buffer_addr,
         pbe_state_words[total_render_target_used],
         spm_eot_state->pbe_reg_words[total_render_target_used]);

      PVR_DEV_ADDR_ADVANCE(next_scratch_buffer_addr, mem_stored);

      total_render_target_used++;

      /* Store off-chip tile data (i.e. tile buffers). */

      for (uint32_t i = 0; i < hw_render->tile_buffers_count; i++) {
         assert(!"Add support for tile buffers in EOT");
         pvr_finishme("Add support for tile buffers in EOT");

         assert(total_render_target_used < PVR_MAX_COLOR_ATTACHMENTS);

         mem_stored = pvr_spm_setup_pbe_state(
            dev_info,
            &framebuffer_size,
            hw_render->output_regs_count,
            PVR_PBE_STARTPOS_BIT0,
            hw_render->sample_count,
            next_scratch_buffer_addr,
            pbe_state_words[total_render_target_used],
            spm_eot_state->pbe_reg_words[total_render_target_used]);

         PVR_DEV_ADDR_ADVANCE(next_scratch_buffer_addr, mem_stored);

         total_render_target_used++;
      }
   }

   pvr_uscgen_eot("SPM EOT",
                  total_render_target_used,
                  pbe_state_words[0],
                  &usc_temp_count,
                  &usc_shader_binary);

   /* TODO: Create a #define in the compiler code to replace the 16. */
   result = pvr_gpu_upload_usc(device,
                               usc_shader_binary.data,
                               usc_shader_binary.size,
                               16,
                               &spm_eot_state->usc_eot_program);

   util_dynarray_fini(&usc_shader_binary);

   if (result != VK_SUCCESS)
      return result;

   result = pvr_pds_pixel_event_program_create_and_upload(
      device,
      spm_eot_state->usc_eot_program,
      usc_temp_count,
      &pds_eot_program);
   if (result != VK_SUCCESS) {
      pvr_bo_suballoc_free(spm_eot_state->usc_eot_program);
      return result;
   }

   spm_eot_state->pixel_event_program_data_upload = pds_eot_program.pvr_bo;
   spm_eot_state->pixel_event_program_data_offset = pds_eot_program.data_offset;

   *emit_count_out = total_render_target_used;

   return VK_SUCCESS;
}

void pvr_spm_finish_eot_state(struct pvr_device *device,
                              struct pvr_spm_eot_state *spm_eot_state)
{
   pvr_bo_suballoc_free(spm_eot_state->pixel_event_program_data_upload);
   pvr_bo_suballoc_free(spm_eot_state->usc_eot_program);
}

static VkFormat pvr_get_format_from_dword_count(uint32_t dword_count)
{
   switch (dword_count) {
   case 1:
      return VK_FORMAT_R32_UINT;
   case 2:
      return VK_FORMAT_R32G32_UINT;
   case 4:
      return VK_FORMAT_R32G32B32A32_UINT;
   default:
      unreachable("Invalid dword_count");
   }
}

static VkResult pvr_spm_setup_texture_state_words(
   struct pvr_device *device,
   uint32_t dword_count,
   const VkExtent2D framebuffer_size,
   uint32_t sample_count,
   pvr_dev_addr_t scratch_buffer_addr,
   uint64_t image_descriptor[static const ROGUE_NUM_TEXSTATE_IMAGE_WORDS],
   uint64_t *mem_used_out)
{
   /* We can ignore the framebuffer's layer count since we only support
    * writing to layer 0.
    */
   struct pvr_texture_state_info info = {
      .format = pvr_get_format_from_dword_count(dword_count),
      .mem_layout = PVR_MEMLAYOUT_LINEAR,

      .type = VK_IMAGE_VIEW_TYPE_2D,
      .tex_state_type = PVR_TEXTURE_STATE_STORAGE,
      .extent = {
         .width = framebuffer_size.width,
         .height = framebuffer_size.height,
      },

      .mip_levels = 1,

      .sample_count = sample_count,
      .stride = framebuffer_size.width,

      .addr = scratch_buffer_addr,
   };
   const uint64_t aligned_fb_width =
      ALIGN_POT(framebuffer_size.width,
                PVRX(CR_PBE_WORD0_MRT0_LINESTRIDE_ALIGNMENT));
   const uint64_t fb_area = aligned_fb_width * framebuffer_size.height;
   const uint8_t *format_swizzle;
   VkResult result;

   format_swizzle = pvr_get_format_swizzle(info.format);
   memcpy(info.swizzle, format_swizzle, sizeof(info.swizzle));

   result = pvr_pack_tex_state(device, &info, image_descriptor);
   if (result != VK_SUCCESS)
      return result;

   *mem_used_out = fb_area * PVR_DW_TO_BYTES(dword_count) * sample_count;

   return VK_SUCCESS;
}

/* FIXME: Can we dedup this with pvr_load_op_pds_data_create_and_upload() ? */
static VkResult pvr_pds_bgnd_program_create_and_upload(
   struct pvr_device *device,
   uint32_t texture_program_data_size_in_dwords,
   const struct pvr_bo *consts_buffer,
   uint32_t const_shared_regs,
   struct pvr_pds_upload *pds_upload_out)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;
   struct pvr_pds_pixel_shader_sa_program texture_program = { 0 };
   uint32_t staging_buffer_size;
   uint32_t *staging_buffer;
   VkResult result;

   pvr_csb_pack (&texture_program.texture_dma_address[0],
                 PDSINST_DOUT_FIELDS_DOUTD_SRC0,
                 doutd_src0) {
      doutd_src0.sbase = consts_buffer->vma->dev_addr;
   }

   pvr_csb_pack (&texture_program.texture_dma_control[0],
                 PDSINST_DOUT_FIELDS_DOUTD_SRC1,
                 doutd_src1) {
      doutd_src1.dest = PVRX(PDSINST_DOUTD_DEST_COMMON_STORE);
      doutd_src1.bsize = const_shared_regs;
   }

   texture_program.num_texture_dma_kicks += 1;

#if defined(DEBUG)
   pvr_pds_set_sizes_pixel_shader_sa_texture_data(&texture_program, dev_info);
   assert(texture_program_data_size_in_dwords == texture_program.data_size);
#endif

   staging_buffer_size = PVR_DW_TO_BYTES(texture_program_data_size_in_dwords);

   staging_buffer = vk_alloc(&device->vk.alloc,
                             staging_buffer_size,
                             8,
                             VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!staging_buffer)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pvr_pds_generate_pixel_shader_sa_texture_state_data(&texture_program,
                                                       staging_buffer,
                                                       dev_info);

   /* FIXME: Figure out the define for alignment of 16. */
   result = pvr_gpu_upload_pds(device,
                               &staging_buffer[0],
                               texture_program_data_size_in_dwords,
                               16,
                               NULL,
                               0,
                               0,
                               16,
                               pds_upload_out);
   if (result != VK_SUCCESS) {
      vk_free(&device->vk.alloc, staging_buffer);
      return result;
   }

   vk_free(&device->vk.alloc, staging_buffer);

   return VK_SUCCESS;
}

VkResult
pvr_spm_init_bgobj_state(struct pvr_device *device,
                         struct pvr_spm_bgobj_state *spm_bgobj_state,
                         const struct pvr_framebuffer *framebuffer,
                         const struct pvr_renderpass_hwsetup_render *hw_render,
                         uint32_t emit_count)
{
   const uint32_t spm_load_program_idx =
      pvr_get_spm_load_program_index(hw_render->sample_count,
                                     hw_render->tile_buffers_count,
                                     hw_render->output_regs_count);
   const VkExtent2D framebuffer_size = {
      .width = framebuffer->width,
      .height = framebuffer->height,
   };
   pvr_dev_addr_t next_scratch_buffer_addr =
      framebuffer->scratch_buffer->bo->vma->dev_addr;
   struct pvr_spm_per_load_program_state *load_program_state;
   struct pvr_pds_upload pds_texture_data_upload;
   const struct pvr_shader_factory_info *info;
   union pvr_sampler_descriptor *descriptor;
   uint64_t consts_buffer_size;
   uint32_t dword_count;
   uint32_t *mem_ptr;
   VkResult result;

   assert(spm_load_program_idx < ARRAY_SIZE(spm_load_collection));
   info = spm_load_collection[spm_load_program_idx].info;

   consts_buffer_size = PVR_DW_TO_BYTES(info->const_shared_regs);

   /* TODO: Remove this check, along with the pvr_finishme(), once the zeroed
    * shaders are replaced by the real shaders.
    */
   if (!consts_buffer_size)
      return VK_SUCCESS;

   pvr_finishme("Remove consts buffer size check");

   result = pvr_bo_alloc(device,
                         device->heaps.general_heap,
                         consts_buffer_size,
                         sizeof(uint32_t),
                         PVR_BO_ALLOC_FLAG_CPU_MAPPED,
                         &spm_bgobj_state->consts_buffer);
   if (result != VK_SUCCESS)
      return result;

   mem_ptr = spm_bgobj_state->consts_buffer->bo->map;

   if (info->driver_const_location_map) {
      const uint32_t *const const_map = info->driver_const_location_map;

      for (uint32_t i = 0; i < PVR_SPM_LOAD_CONST_COUNT; i += 2) {
         pvr_dev_addr_t tile_buffer_addr;

         if (const_map[i] == PVR_SPM_LOAD_DEST_UNUSED) {
#if defined(DEBUG)
            for (uint32_t j = i; j < PVR_SPM_LOAD_CONST_COUNT; j++)
               assert(const_map[j] == PVR_SPM_LOAD_DEST_UNUSED);
#endif
            break;
         }

         tile_buffer_addr =
            device->tile_buffer_state.buffers[i / 2]->vma->dev_addr;

         assert(const_map[i] == const_map[i + 1] + 1);
         mem_ptr[const_map[i]] = tile_buffer_addr.addr >> 32;
         mem_ptr[const_map[i + 1]] = (uint32_t)tile_buffer_addr.addr;
      }
   }

   /* TODO: The 32 comes from how the shaders are compiled. We should
    * unhardcode it when this is hooked up to the compiler.
    */
   descriptor = (union pvr_sampler_descriptor *)(mem_ptr + 32);
   *descriptor = (union pvr_sampler_descriptor){ 0 };

   pvr_csb_pack (&descriptor->data.sampler_word, TEXSTATE_SAMPLER, sampler) {
      sampler.non_normalized_coords = true;
      sampler.addrmode_v = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
      sampler.addrmode_u = PVRX(TEXSTATE_ADDRMODE_CLAMP_TO_EDGE);
      sampler.minfilter = PVRX(TEXSTATE_FILTER_POINT);
      sampler.magfilter = PVRX(TEXSTATE_FILTER_POINT);
      sampler.maxlod = PVRX(TEXSTATE_CLAMP_MIN);
      sampler.minlod = PVRX(TEXSTATE_CLAMP_MIN);
      sampler.dadjust = PVRX(TEXSTATE_DADJUST_ZERO_UINT);
   }

   /* Even if we might have 8 output regs we can only pack and write 4 dwords
    * using R32G32B32A32_UINT.
    */
   if (hw_render->tile_buffers_count > 0)
      dword_count = 4;
   else
      dword_count = MIN2(hw_render->output_regs_count, 4);

   for (uint32_t i = 0; i < emit_count; i++) {
      uint64_t *mem_ptr_u64 = (uint64_t *)mem_ptr;
      uint64_t mem_used = 0;

      STATIC_ASSERT(ROGUE_NUM_TEXSTATE_IMAGE_WORDS * sizeof(uint64_t) /
                       sizeof(uint32_t) ==
                    PVR_IMAGE_DESCRIPTOR_SIZE);
      mem_ptr_u64 += i * ROGUE_NUM_TEXSTATE_IMAGE_WORDS;

      result = pvr_spm_setup_texture_state_words(device,
                                                 dword_count,
                                                 framebuffer_size,
                                                 hw_render->sample_count,
                                                 next_scratch_buffer_addr,
                                                 mem_ptr_u64,
                                                 &mem_used);
      if (result != VK_SUCCESS)
         goto err_free_consts_buffer;

      PVR_DEV_ADDR_ADVANCE(next_scratch_buffer_addr, mem_used);
   }

   assert(spm_load_program_idx <
          ARRAY_SIZE(device->spm_load_state.load_program));
   load_program_state =
      &device->spm_load_state.load_program[spm_load_program_idx];

   result = pvr_pds_bgnd_program_create_and_upload(
      device,
      load_program_state->pds_texture_program_data_size,
      spm_bgobj_state->consts_buffer,
      info->const_shared_regs,
      &pds_texture_data_upload);
   if (result != VK_SUCCESS)
      goto err_free_consts_buffer;

   spm_bgobj_state->pds_texture_data_upload = pds_texture_data_upload.pvr_bo;

   /* TODO: Is it worth to dedup this with pvr_pds_bgnd_pack_state() ? */

   /* clang-format off */
   pvr_csb_pack (&spm_bgobj_state->pds_reg_values[0],
                 CR_PDS_BGRND0_BASE,
                 value) {
      /* clang-format on */
      value.shader_addr = load_program_state->pds_pixel_program_offset;
      value.texunicode_addr = load_program_state->pds_uniform_program_offset;
   }

   /* clang-format off */
   pvr_csb_pack (&spm_bgobj_state->pds_reg_values[1],
                 CR_PDS_BGRND1_BASE,
                 value) {
      /* clang-format on */
      value.texturedata_addr =
         PVR_DEV_ADDR(pds_texture_data_upload.data_offset);
   }

   /* clang-format off */
   pvr_csb_pack (&spm_bgobj_state->pds_reg_values[2],
                 CR_PDS_BGRND3_SIZEINFO,
                 value) {
      /* clang-format on */
      value.usc_sharedsize =
         DIV_ROUND_UP(info->const_shared_regs,
                      PVRX(CR_PDS_BGRND3_SIZEINFO_USC_SHAREDSIZE_UNIT_SIZE));
      value.pds_texturestatesize = DIV_ROUND_UP(
         pds_texture_data_upload.data_size,
         PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_TEXTURESTATESIZE_UNIT_SIZE));
      value.pds_tempsize =
         DIV_ROUND_UP(load_program_state->pds_texture_program_temps_count,
                      PVRX(CR_PDS_BGRND3_SIZEINFO_PDS_TEMPSIZE_UNIT_SIZE));
   }

   return VK_SUCCESS;

err_free_consts_buffer:
   pvr_bo_free(device, spm_bgobj_state->consts_buffer);

   return result;
}

void pvr_spm_finish_bgobj_state(struct pvr_device *device,
                                struct pvr_spm_bgobj_state *spm_bgobj_state)
{
   pvr_bo_suballoc_free(spm_bgobj_state->pds_texture_data_upload);
   pvr_bo_free(device, spm_bgobj_state->consts_buffer);
}

#undef PVR_DEV_ADDR_ADVANCE
