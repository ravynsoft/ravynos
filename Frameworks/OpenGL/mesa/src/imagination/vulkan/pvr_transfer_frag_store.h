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

#ifndef PVR_TRANSFER_FRAG_STORE_H
#define PVR_TRANSFER_FRAG_STORE_H

#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "pvr_device_info.h"
#include "pvr_uscgen.h"
#include "pvr_types.h"
#include "util/hash_table.h"

struct pvr_device;

struct pvr_transfer_frag_store {
   uint32_t max_multisample;
   /* Hash table mapping keys, produced by pvr_transfer_frag_shader_key(), to
    * pvr_transfer_frag_store_entry_data entries.
    */
   struct hash_table *hash_table;
};

VkResult pvr_transfer_frag_store_init(struct pvr_device *device,
                                      struct pvr_transfer_frag_store *store);
void pvr_transfer_frag_store_fini(struct pvr_device *device,
                                  struct pvr_transfer_frag_store *store);

VkResult pvr_transfer_frag_store_get_shader_info(
   struct pvr_device *device,
   struct pvr_transfer_frag_store *store,
   const struct pvr_tq_shader_properties *shader_props,
   pvr_dev_addr_t *const pds_dev_addr_out,
   const struct pvr_tq_frag_sh_reg_layout **const reg_layout_out);

#endif /* PVR_TRANSFER_FRAG_STORE_H */
