/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef D3D12_NIR_PASSES_H
#define D3D12_NIR_PASSES_H

#include "nir.h"
#include "nir_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

struct d3d12_shader;
struct d3d12_image_format_conversion_info;
struct d3d12_image_format_conversion_info_arr;
enum d3d12_state_var;

nir_def *
d3d12_get_state_var(nir_builder *b,
                    enum d3d12_state_var var_enum,
                    const char *var_name,
                    const struct glsl_type *var_type,
                    nir_variable **out_var);

nir_def *
d3d12_get_state_var(nir_builder *b,
                    enum d3d12_state_var var_enum,
                    const char *var_name,
                    const struct glsl_type *var_type,
                    nir_variable **out_var);

bool
d3d12_lower_point_sprite(nir_shader *shader,
                         bool sprite_origin_lower_left,
                         bool point_size_per_vertex,
                         unsigned point_coord_enable,
                         uint64_t next_inputs_read);

bool
d3d12_lower_state_vars(struct nir_shader *s, struct d3d12_shader *shader);

void
d3d12_lower_yflip(nir_shader *s);

void
d3d12_lower_depth_range(nir_shader *nir);

bool
d3d12_lower_load_draw_params(nir_shader *nir);

bool
d3d12_lower_load_patch_vertices_in(nir_shader *nir);

bool
d3d12_lower_compute_state_vars(nir_shader *nir);

void
d3d12_lower_uint_cast(nir_shader *nir, bool is_signed);

void
d3d12_add_missing_dual_src_target(struct nir_shader *s,
                                  unsigned missing_mask);

bool
d3d12_fix_io_uint_type(struct nir_shader *s, uint64_t in_mask, uint64_t out_mask);

void
d3d12_nir_invert_depth(nir_shader *s, unsigned viewport_mask, bool clip_halfz);

void
d3d12_lower_primitive_id(nir_shader *shader);

void
d3d12_lower_triangle_strip(nir_shader *shader);

bool
d3d12_lower_image_casts(nir_shader *s, struct d3d12_image_format_conversion_info_arr *info);

bool
d3d12_disable_multisampling(nir_shader *s);

bool
d3d12_split_needed_varyings(nir_shader *s);

void
d3d12_write_0_to_new_varying(nir_shader *s, nir_variable *var);

#ifdef __cplusplus
}
#endif

#endif // D3D12_NIR_PASSES_H
