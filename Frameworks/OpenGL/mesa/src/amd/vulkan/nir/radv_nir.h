/*
 * Copyright © 2023 Valve Corporation
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

#ifndef RADV_NIR_H
#define RADV_NIR_H

#include <stdbool.h>
#include <stdint.h>
#include "amd_family.h"
#include "nir.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nir_shader nir_shader;
struct radeon_info;
struct radv_pipeline_layout;
struct radv_pipeline_key;
struct radv_shader_stage;
struct radv_shader_info;
struct radv_shader_args;
struct radv_shader_layout;
struct radv_device;

void radv_nir_apply_pipeline_layout(nir_shader *shader, struct radv_device *device, const struct radv_shader_info *info,
                                    const struct radv_shader_args *args, const struct radv_shader_layout *layout);

void radv_nir_lower_abi(nir_shader *shader, enum amd_gfx_level gfx_level, const struct radv_shader_info *info,
                        const struct radv_shader_args *args, const struct radv_pipeline_key *pl_key,
                        uint32_t address32_hi);

bool radv_nir_lower_hit_attrib_derefs(nir_shader *shader);

bool radv_nir_lower_ray_queries(struct nir_shader *shader, struct radv_device *device);

bool radv_nir_lower_vs_inputs(nir_shader *shader, const struct radv_shader_stage *vs_stage,
                              const struct radv_pipeline_key *pl_key, const struct radeon_info *rad_info);

bool radv_nir_lower_primitive_shading_rate(nir_shader *nir, enum amd_gfx_level gfx_level);

bool radv_nir_lower_fs_intrinsics(nir_shader *nir, const struct radv_shader_stage *fs_stage,
                                  const struct radv_pipeline_key *key);

bool radv_nir_lower_fs_barycentric(nir_shader *shader, const struct radv_pipeline_key *key, unsigned rast_prim);

bool radv_nir_lower_intrinsics_early(nir_shader *nir, const struct radv_pipeline_key *key);

bool radv_nir_lower_view_index(nir_shader *nir, bool per_primitive);

bool radv_nir_lower_viewport_to_zero(nir_shader *nir);

bool radv_nir_export_multiview(nir_shader *nir);

void radv_nir_lower_io_to_scalar_early(nir_shader *nir, nir_variable_mode mask);

void radv_nir_lower_io(struct radv_device *device, nir_shader *nir);

bool radv_nir_lower_io_to_mem(struct radv_device *device, struct radv_shader_stage *stage);

void radv_nir_lower_poly_line_smooth(nir_shader *nir, const struct radv_pipeline_key *key);

bool radv_nir_lower_cooperative_matrix(nir_shader *shader, unsigned wave_size);

#ifdef __cplusplus
}
#endif

#endif /* RADV_NIR_H */
