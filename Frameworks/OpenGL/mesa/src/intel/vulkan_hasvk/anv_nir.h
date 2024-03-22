/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef ANV_NIR_H
#define ANV_NIR_H

#include "nir/nir.h"
#include "anv_private.h"

#ifdef __cplusplus
extern "C" {
#endif

bool anv_nir_lower_multiview(nir_shader *shader, uint32_t view_mask);

bool anv_nir_lower_ycbcr_textures(nir_shader *shader,
                                  const struct anv_pipeline_layout *layout);

static inline nir_address_format
anv_nir_ssbo_addr_format(const struct anv_physical_device *pdevice,
                         enum brw_robustness_flags robust_flags)
{
   if (pdevice->has_a64_buffer_access) {
      if (robust_flags & BRW_ROBUSTNESS_SSBO)
         return nir_address_format_64bit_bounded_global;
      else
         return nir_address_format_64bit_global_32bit_offset;
   } else {
      return nir_address_format_32bit_index_offset;
   }
}

static inline nir_address_format
anv_nir_ubo_addr_format(const struct anv_physical_device *pdevice,
                        enum brw_robustness_flags robust_flags)
{
   if (pdevice->has_a64_buffer_access) {
      if (robust_flags & BRW_ROBUSTNESS_UBO)
         return nir_address_format_64bit_bounded_global;
      else
         return nir_address_format_64bit_global_32bit_offset;
   } else {
      return nir_address_format_32bit_index_offset;
   }
}

bool anv_nir_lower_ubo_loads(nir_shader *shader);

void anv_nir_apply_pipeline_layout(nir_shader *shader,
                                   const struct anv_physical_device *pdevice,
                                   enum brw_robustness_flags robust_flags,
                                   const struct anv_pipeline_layout *layout,
                                   struct anv_pipeline_bind_map *map);

void anv_nir_compute_push_layout(nir_shader *nir,
                                 const struct anv_physical_device *pdevice,
                                 enum brw_robustness_flags robust_flags,
                                 struct brw_stage_prog_data *prog_data,
                                 struct anv_pipeline_bind_map *map,
                                 void *mem_ctx);

void anv_nir_validate_push_layout(struct brw_stage_prog_data *prog_data,
                                  struct anv_pipeline_bind_map *map);

bool anv_nir_add_base_work_group_id(nir_shader *shader);

#ifdef __cplusplus
}
#endif

#endif /* ANV_NIR_H */
