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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "hwdef/rogue_hw_utils.h"
#include "pvr_bo.h"
#include "pvr_common.h"
#include "pvr_device_info.h"
#include "pvr_job_transfer.h"
#include "pvr_pds.h"
#include "pvr_private.h"
#include "pvr_transfer_frag_store.h"
#include "pvr_types.h"
#include "pvr_uscgen.h"
#include "util/hash_table.h"
#include "util/macros.h"
#include "util/ralloc.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"
#include "vk_log.h"

#define PVR_TRANSFER_BYTE_UNWIND_MAX 16U

struct pvr_transfer_frag_store_entry_data {
   pvr_dev_addr_t kick_usc_pds_offset;
   struct pvr_bo *kick_usc_pds_upload;

   struct pvr_suballoc_bo *usc_upload;
   struct pvr_tq_frag_sh_reg_layout sh_reg_layout;
};

#define to_pvr_entry_data(_entry) \
   _Generic((_entry), \
            struct hash_entry *: (struct pvr_transfer_frag_store_entry_data *)((_entry)->data), \
            const struct hash_entry *: (const struct pvr_transfer_frag_store_entry_data *)((_entry)->data))

VkResult pvr_transfer_frag_store_init(struct pvr_device *device,
                                      struct pvr_transfer_frag_store *store)
{
   const struct pvr_device_info *dev_info = &device->pdevice->dev_info;

   *store = (struct pvr_transfer_frag_store){
      .max_multisample = PVR_GET_FEATURE_VALUE(dev_info, max_multisample, 1U),
      .hash_table = _mesa_hash_table_create_u32_keys(NULL),
   };

   if (!store->hash_table)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   return VK_SUCCESS;
}

/**
 * \brief Returns a key based on shader properties.
 *
 * Returns a unique key that can be used to uniquely identify a transfer
 * fragment shader based on the provided shader properties.
 *
 * Make sure that the non valid parts of shader_props are memset to 0. Otherwise
 * these bits might appear in the key as uninitialized data and might not
 * match a key for the same shader.
 */
static uint32_t pvr_transfer_frag_shader_key(
   uint32_t max_multisample,
   const struct pvr_tq_shader_properties *shader_props)
{
   const struct pvr_tq_layer_properties *layer = &shader_props->layer_props;
   uint32_t resolve_op_num = max_multisample + PVR_RESOLVE_SAMPLE0;

   uint32_t num_layers_bits = util_logbase2_ceil(PVR_TRANSFER_MAX_LAYERS + 1U);
   uint32_t layer_float_bits = util_logbase2_ceil(PVR_INT_COORD_SET_FLOATS_NUM);
   uint32_t pixel_src_bits = util_logbase2_ceil(PVR_TRANSFER_PBE_PIXEL_SRC_NUM);
   uint32_t byte_unwind_bits = util_logbase2_ceil(PVR_TRANSFER_BYTE_UNWIND_MAX);
   uint32_t resolve_op_bits = util_logbase2_ceil(resolve_op_num);
   uint32_t sample_cnt_bits = util_last_bit(util_logbase2(max_multisample));
   uint32_t hash = 0U;

#if defined(DEBUG)
   uint32_t max_shift = 0U;
#   define shift_hash(hash, num)   \
      do {                         \
         max_shift += (num);       \
         assert(max_shift <= 32U); \
                                   \
         (hash) <<= (num);         \
      } while (0U)
#else
#   define shift_hash(hash, num) hash <<= (num)
#endif

   /* Hash layer info. */

   shift_hash(hash, layer_float_bits);
   hash |= (uint32_t)shader_props->layer_props.layer_floats;

   shift_hash(hash, 1U);
   hash |= layer->sample;

   shift_hash(hash, 1U);
   hash |= (uint32_t) false;

   shift_hash(hash, 1U);
   hash |= (uint32_t) false;

   shift_hash(hash, pixel_src_bits);
   hash |= (uint32_t)layer->pbe_format;

   shift_hash(hash, resolve_op_bits);
   hash |= (uint32_t)layer->resolve_op;

   assert(util_is_power_of_two_nonzero(layer->sample_count));
   shift_hash(hash, sample_cnt_bits);
   hash |= (uint32_t)util_logbase2(layer->sample_count);

   shift_hash(hash, 1U);
   hash |= (uint32_t)layer->msaa;

   shift_hash(hash, byte_unwind_bits);
   hash |= layer->byte_unwind;

   shift_hash(hash, 1U);
   hash |= (uint32_t)layer->linear;

   /* End layer info. */

   shift_hash(hash, 1U);
   hash |= (uint32_t)shader_props->full_rate;

   shift_hash(hash, 1U);
   hash |= (uint32_t)shader_props->iterated;

   shift_hash(hash, 1U);
   hash |= (uint32_t)shader_props->pick_component;

   shift_hash(hash, num_layers_bits);
   /* Just 1 layer. */
   hash |= 1;

   shift_hash(hash, 3U);
   /* alpha type none */
   hash |= 0;

#undef shift_hash

   return hash;
}

#define to_hash_table_key(_key) ((void *)(uintptr_t)(_key))

static VkResult pvr_transfer_frag_store_entry_data_compile(
   struct pvr_device *device,
   struct pvr_transfer_frag_store_entry_data *const entry_data,
   const struct pvr_tq_shader_properties *shader_props,
   uint32_t *const num_usc_temps_out)
{
   const uint32_t image_desc_offset =
      offsetof(struct pvr_combined_image_sampler_descriptor, image) / 4;
   const uint32_t sampler_desc_offset =
      offsetof(struct pvr_combined_image_sampler_descriptor, sampler) / 4;

   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);

   struct pvr_tq_frag_sh_reg_layout *sh_reg_layout = &entry_data->sh_reg_layout;
   uint32_t next_free_sh_reg = 0;
   struct util_dynarray shader;
   VkResult result;

   /* TODO: Allocate all combined image samplers if needed? Otherwise change the
    * array to a single descriptor.
    */
   sh_reg_layout->combined_image_samplers.offsets[0].image =
      next_free_sh_reg + image_desc_offset;
   sh_reg_layout->combined_image_samplers.offsets[0].sampler =
      next_free_sh_reg + sampler_desc_offset;
   sh_reg_layout->combined_image_samplers.count = 1;
   next_free_sh_reg += sizeof(struct pvr_combined_image_sampler_descriptor) / 4;

   /* TODO: Handle dynamic_const_regs used for PVR_INT_COORD_SET_FLOATS_{4,6}, Z
    * position, texel unwind, etc. when compiler adds support for them.
    */
   sh_reg_layout->dynamic_consts.offset = next_free_sh_reg;
   sh_reg_layout->dynamic_consts.count = 0;

   sh_reg_layout->driver_total = next_free_sh_reg;

   pvr_uscgen_tq_frag(shader_props,
                      &entry_data->sh_reg_layout,
                      num_usc_temps_out,
                      &shader);

   result = pvr_gpu_upload_usc(device,
                               util_dynarray_begin(&shader),
                               util_dynarray_num_elements(&shader, uint8_t),
                               cache_line_size,
                               &entry_data->usc_upload);
   util_dynarray_fini(&shader);
   if (result != VK_SUCCESS)
      return result;

   return VK_SUCCESS;
}

static VkResult pvr_transfer_frag_store_entry_data_create(
   struct pvr_device *device,
   struct pvr_transfer_frag_store *store,
   const struct pvr_tq_shader_properties *shader_props,
   const struct pvr_transfer_frag_store_entry_data **const entry_data_out)
{
   struct pvr_pds_kickusc_program kick_usc_pds_prog = { 0 };
   struct pvr_transfer_frag_store_entry_data *entry_data;
   pvr_dev_addr_t dev_addr;
   uint32_t num_usc_temps;
   VkResult result;

   entry_data = ralloc(store->hash_table, __typeof__(*entry_data));
   if (!entry_data)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   result = pvr_transfer_frag_store_entry_data_compile(device,
                                                       entry_data,
                                                       shader_props,
                                                       &num_usc_temps);
   if (result != VK_SUCCESS)
      goto err_free_entry;

   dev_addr = entry_data->usc_upload->dev_addr;
   dev_addr.addr -= device->heaps.usc_heap->base_addr.addr;

   pvr_pds_setup_doutu(&kick_usc_pds_prog.usc_task_control,
                       dev_addr.addr,
                       num_usc_temps,
                       shader_props->full_rate
                          ? PVRX(PDSINST_DOUTU_SAMPLE_RATE_FULL)
                          : PVRX(PDSINST_DOUTU_SAMPLE_RATE_INSTANCE),
                       false);

   pvr_pds_kick_usc(&kick_usc_pds_prog, NULL, 0U, false, PDS_GENERATE_SIZES);

   result = pvr_bo_alloc(device,
                         device->heaps.pds_heap,
                         PVR_DW_TO_BYTES(kick_usc_pds_prog.data_size +
                                         kick_usc_pds_prog.code_size),
                         16,
                         PVR_BO_ALLOC_FLAG_CPU_MAPPED,
                         &entry_data->kick_usc_pds_upload);
   if (result != VK_SUCCESS)
      goto err_free_usc_upload;

   pvr_pds_kick_usc(&kick_usc_pds_prog,
                    entry_data->kick_usc_pds_upload->bo->map,
                    0U,
                    false,
                    PDS_GENERATE_CODEDATA_SEGMENTS);

   dev_addr = entry_data->kick_usc_pds_upload->vma->dev_addr;
   dev_addr.addr -= device->heaps.pds_heap->base_addr.addr;
   entry_data->kick_usc_pds_offset = dev_addr;

   *entry_data_out = entry_data;

   return VK_SUCCESS;

err_free_usc_upload:
   pvr_bo_suballoc_free(entry_data->usc_upload);

err_free_entry:
   ralloc_free(entry_data);

   return result;
}

static void inline pvr_transfer_frag_store_entry_data_destroy_no_ralloc_free(
   struct pvr_device *device,
   const struct pvr_transfer_frag_store_entry_data *entry_data)
{
   pvr_bo_free(device, entry_data->kick_usc_pds_upload);
   pvr_bo_suballoc_free(entry_data->usc_upload);
}

static void inline pvr_transfer_frag_store_entry_data_destroy(
   struct pvr_device *device,
   const struct pvr_transfer_frag_store_entry_data *entry_data)
{
   pvr_transfer_frag_store_entry_data_destroy_no_ralloc_free(device,
                                                             entry_data);
   /* Casting away the const :( */
   ralloc_free((void *)entry_data);
}

static VkResult pvr_transfer_frag_store_get_entry(
   struct pvr_device *device,
   struct pvr_transfer_frag_store *store,
   const struct pvr_tq_shader_properties *shader_props,
   const struct pvr_transfer_frag_store_entry_data **const entry_data_out)
{
   const uint32_t key =
      pvr_transfer_frag_shader_key(store->max_multisample, shader_props);
   const struct hash_entry *entry;
   VkResult result;

   entry = _mesa_hash_table_search(store->hash_table, to_hash_table_key(key));
   if (!entry) {
      /* Init so that gcc stops complaining. */
      const struct pvr_transfer_frag_store_entry_data *entry_data = NULL;

      result = pvr_transfer_frag_store_entry_data_create(device,
                                                         store,
                                                         shader_props,
                                                         &entry_data);
      if (result != VK_SUCCESS)
         return result;

      assert(entry_data);

      entry = _mesa_hash_table_insert(store->hash_table,
                                      to_hash_table_key(key),
                                      (void *)entry_data);
      if (!entry) {
         pvr_transfer_frag_store_entry_data_destroy(device, entry_data);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }
   }

   *entry_data_out = to_pvr_entry_data(entry);

   return VK_SUCCESS;
}

VkResult pvr_transfer_frag_store_get_shader_info(
   struct pvr_device *device,
   struct pvr_transfer_frag_store *store,
   const struct pvr_tq_shader_properties *shader_props,
   pvr_dev_addr_t *const pds_dev_addr_out,
   const struct pvr_tq_frag_sh_reg_layout **const reg_layout_out)
{
   /* Init so that gcc stops complaining. */
   const struct pvr_transfer_frag_store_entry_data *entry_data = NULL;
   VkResult result;

   result = pvr_transfer_frag_store_get_entry(device,
                                              store,
                                              shader_props,
                                              &entry_data);
   if (result != VK_SUCCESS)
      return result;

   *pds_dev_addr_out = entry_data->kick_usc_pds_offset;
   *reg_layout_out = &entry_data->sh_reg_layout;

   return VK_SUCCESS;
}

void pvr_transfer_frag_store_fini(struct pvr_device *device,
                                  struct pvr_transfer_frag_store *store)
{
   hash_table_foreach_remove(store->hash_table, entry)
   {
      /* ralloc_free() in _mesa_hash_table_destroy() will free each entry's
       * memory so let's not waste extra time freeing them one by one and
       * unliking.
       */
      pvr_transfer_frag_store_entry_data_destroy_no_ralloc_free(
         device,
         to_pvr_entry_data(entry));
   }

   _mesa_hash_table_destroy(store->hash_table, NULL);
}
