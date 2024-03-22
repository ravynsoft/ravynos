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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "hwdef/rogue_hw_utils.h"
#include "pvr_bo.h"
#include "pvr_border.h"
#include "pvr_device_info.h"
#include "pvr_formats.h"
#include "pvr_private.h"
#include "util/bitset.h"
#include "util/format/u_format.h"
#include "util/format/u_formats.h"
#include "util/log.h"
#include "util/macros.h"
#include "vk_format.h"
#include "vk_log.h"
#include "vk_sampler.h"

struct pvr_border_color_table_value {
   uint8_t value[16];
} PACKED;
static_assert(sizeof(struct pvr_border_color_table_value) ==
                 4 * sizeof(uint32_t),
              "struct pvr_border_color_table_value must be 4 x u32");

struct pvr_border_color_table_entry {
   struct pvr_border_color_table_value values[PVR_TEX_FORMAT_COUNT * 2];
} PACKED;

static inline void pvr_border_color_table_pack_single(
   struct pvr_border_color_table_value *const dst,
   const union pipe_color_union *const color,
   const struct pvr_tex_format_description *const format,
   const bool is_int)
{
   const enum pipe_format pipe_format = is_int ? format->pipe_format_int
                                               : format->pipe_format_float;

   if (pipe_format == PIPE_FORMAT_NONE)
      return;

   memset(dst->value, 0, sizeof(dst->value));

   if (util_format_is_depth_or_stencil(pipe_format)) {
      if (is_int) {
         const uint8_t s_color[4] = {
            color->ui[0],
            color->ui[1],
            color->ui[2],
            color->ui[3],
         };

         util_format_pack_s_8uint(pipe_format, dst->value, s_color, 1);
      } else {
         util_format_pack_z_float(pipe_format, dst->value, color->f, 1);
      }
   } else {
      util_format_pack_rgba(pipe_format, dst->value, color, 1);
   }
}

static inline void pvr_border_color_table_pack_single_compressed(
   struct pvr_border_color_table_value *const dst,
   const union pipe_color_union *const color,
   const struct pvr_tex_format_compressed_description *const format,
   const struct pvr_device_info *const dev_info)
{
   if (PVR_HAS_FEATURE(dev_info, tpu_border_colour_enhanced)) {
      const struct pvr_tex_format_description *format_simple =
         pvr_get_tex_format_description(format->tex_format_simple);

      pvr_border_color_table_pack_single(dst, color, format_simple, false);
      return;
   }

   memset(dst->value, 0, sizeof(dst->value));

   pvr_finishme("Devices without tpu_border_colour_enhanced require entries "
                "for compressed formats to be stored in the table "
                "pre-compressed.");
}

static int32_t
pvr_border_color_table_alloc_entry(struct pvr_border_color_table *const table)
{
   /* BITSET_FFS() returns a 1-indexed position or 0 if no bits are set. */
   int32_t index = BITSET_FFS(table->unused_entries);
   if (!index--)
      return -1;

   BITSET_CLEAR(table->unused_entries, index);

   return index;
}

static void
pvr_border_color_table_free_entry(struct pvr_border_color_table *const table,
                                  const uint32_t index)
{
   assert(pvr_border_color_table_is_index_valid(table, index));
   BITSET_SET(table->unused_entries, index);
}

static void
pvr_border_color_table_fill_entry(struct pvr_border_color_table *const table,
                                  const uint32_t index,
                                  const union pipe_color_union *const color,
                                  const bool is_int,
                                  const struct pvr_device_info *const dev_info)
{
   struct pvr_border_color_table_entry *const entries = table->table->bo->map;
   struct pvr_border_color_table_entry *const entry = &entries[index];
   uint32_t tex_format = 0;

   for (; tex_format < PVR_TEX_FORMAT_COUNT; tex_format++) {
      if (!pvr_tex_format_is_supported(tex_format))
         continue;

      pvr_border_color_table_pack_single(
         &entry->values[tex_format],
         color,
         pvr_get_tex_format_description(tex_format),
         is_int);
   }

   for (; tex_format < PVR_TEX_FORMAT_COUNT * 2; tex_format++) {
      if (!pvr_tex_format_compressed_is_supported(tex_format))
         continue;

      pvr_border_color_table_pack_single_compressed(
         &entry->values[tex_format],
         color,
         pvr_get_tex_format_compressed_description(tex_format),
         dev_info);
   }
}

VkResult pvr_border_color_table_init(struct pvr_border_color_table *const table,
                                     struct pvr_device *const device)
{
   const struct pvr_device_info *const dev_info = &device->pdevice->dev_info;
   const uint32_t cache_line_size = rogue_get_slc_cache_line_size(dev_info);
   const uint32_t table_size = sizeof(struct pvr_border_color_table_entry) *
                               PVR_BORDER_COLOR_TABLE_NR_ENTRIES;

   VkResult result;

   /* Initialize to ones so ffs can be used to find unused entries. */
   BITSET_ONES(table->unused_entries);

   result = pvr_bo_alloc(device,
                         device->heaps.general_heap,
                         table_size,
                         cache_line_size,
                         PVR_BO_ALLOC_FLAG_CPU_MAPPED,
                         &table->table);
   if (result != VK_SUCCESS)
      goto err_out;

   BITSET_CLEAR_RANGE_INSIDE_WORD(table->unused_entries,
                                  0,
                                  PVR_BORDER_COLOR_TABLE_NR_BUILTIN_ENTRIES -
                                     1);

   for (uint32_t i = 0; i < PVR_BORDER_COLOR_TABLE_NR_BUILTIN_ENTRIES; i++) {
      const VkClearColorValue color = vk_border_color_value(i);
      const bool is_int = vk_border_color_is_int(i);

      pvr_border_color_table_fill_entry(table,
                                        i,
                                        (const union pipe_color_union *)&color,
                                        is_int,
                                        dev_info);
   }

   pvr_bo_cpu_unmap(device, table->table);

   return VK_SUCCESS;

err_out:
   return result;
}

void pvr_border_color_table_finish(struct pvr_border_color_table *const table,
                                   struct pvr_device *const device)
{
#if defined(DEBUG)
   BITSET_SET_RANGE_INSIDE_WORD(table->unused_entries,
                                0,
                                PVR_BORDER_COLOR_TABLE_NR_BUILTIN_ENTRIES - 1);
   BITSET_NOT(table->unused_entries);
   assert(BITSET_IS_EMPTY(table->unused_entries));
#endif

   pvr_bo_free(device, table->table);
}

VkResult pvr_border_color_table_get_or_create_entry(
   UNUSED struct pvr_border_color_table *const table,
   const struct pvr_sampler *const sampler,
   uint32_t *const index_out)
{
   const VkBorderColor vk_type = sampler->vk.border_color;

   if (vk_type <= PVR_BORDER_COLOR_TABLE_NR_BUILTIN_ENTRIES) {
      *index_out = vk_type;
      return VK_SUCCESS;
   }

   pvr_finishme("VK_EXT_custom_border_color is currently unsupported.");
   return vk_error(sampler, VK_ERROR_EXTENSION_NOT_PRESENT);
}
