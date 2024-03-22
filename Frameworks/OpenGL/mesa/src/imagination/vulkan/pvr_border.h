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

#ifndef PVR_BORDER_H
#define PVR_BORDER_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "pvr_csb.h"
#include "util/bitset.h"

#define PVR_BORDER_COLOR_TABLE_NR_ENTRIES \
   (PVRX(TEXSTATE_SAMPLER_BORDERCOLOR_INDEX_MAX_SIZE) + 1)

#define PVR_BORDER_COLOR_TABLE_NR_BUILTIN_ENTRIES \
   (VK_BORDER_COLOR_INT_OPAQUE_WHITE + 1)

#define PVR_BORDER_COLOR_TABLE_NR_CUSTOM_ENTRIES \
   (PVR_BORDER_COLOR_TABLE_NR_ENTRIES -          \
    PVR_BORDER_COLOR_TABLE_NR_BUILTIN_ENTRIES)

/* Forward declaration from "pvr_common.h" */
struct pvr_sampler;

/* Forward declaration from "pvr_bo.h" */
struct pvr_bo;

/* Forward declaration from "pvr_private.h" */
struct pvr_device;

struct pvr_border_color_table {
   BITSET_DECLARE(unused_entries, PVR_BORDER_COLOR_TABLE_NR_ENTRIES);

   /* Contains an array of:
    * PVR_BORDER_COLOR_TABLE_NR_ENTRIES x struct pvr_border_color_table_entry
    */
   struct pvr_bo *table;
};

VkResult pvr_border_color_table_init(struct pvr_border_color_table *table,
                                     struct pvr_device *device);
void pvr_border_color_table_finish(struct pvr_border_color_table *table,
                                   struct pvr_device *device);

VkResult
pvr_border_color_table_get_or_create_entry(struct pvr_border_color_table *table,
                                           const struct pvr_sampler *sampler,
                                           uint32_t *index_out);

static inline bool pvr_border_color_table_is_index_valid(
   const struct pvr_border_color_table *const table,
   const uint32_t index)
{
   return !BITSET_TEST(table->unused_entries, index);
}

#endif /* PVR_BORDER_H */
