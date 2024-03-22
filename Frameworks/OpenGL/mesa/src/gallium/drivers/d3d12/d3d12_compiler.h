/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_COMPILER_H
#define D3D12_COMPILER_H

#include "d3d12_common.h"

#include "dxil_nir_lower_int_samplers.h"

#include "pipe/p_defines.h"
#include "pipe/p_state.h"

#include "compiler/shader_info.h"
#include "program/prog_statevars.h"

#include "nir.h"

struct pipe_screen;
struct d3d12_context;
struct d3d12_screen;

#ifdef __cplusplus
extern "C" {
#endif

enum d3d12_state_var {
   D3D12_STATE_VAR_Y_FLIP = 0,
   D3D12_STATE_VAR_PT_SPRITE,
   D3D12_STATE_VAR_DRAW_PARAMS,
   D3D12_STATE_VAR_DEPTH_TRANSFORM,
   D3D12_STATE_VAR_DEFAULT_INNER_TESS_LEVEL,
   D3D12_STATE_VAR_DEFAULT_OUTER_TESS_LEVEL,
   D3D12_STATE_VAR_PATCH_VERTICES_IN,
   D3D12_MAX_GRAPHICS_STATE_VARS,

   D3D12_STATE_VAR_NUM_WORKGROUPS = 0,
   D3D12_STATE_VAR_TRANSFORM_GENERIC0,
   D3D12_STATE_VAR_TRANSFORM_GENERIC1,
   D3D12_MAX_COMPUTE_STATE_VARS,

   D3D12_MAX_STATE_VARS = MAX2(D3D12_MAX_GRAPHICS_STATE_VARS, D3D12_MAX_COMPUTE_STATE_VARS)
};

#define D3D12_MAX_POINT_SIZE 255.0f

const void *
d3d12_get_compiler_options(struct pipe_screen *screen,
                           enum pipe_shader_ir ir,
                           enum pipe_shader_type shader);


void
d3d12_varying_cache_init(struct d3d12_screen *ctx);

void
d3d12_varying_cache_destroy(struct d3d12_screen *ctx);


struct d3d12_varying_info {
   struct {
      const struct glsl_type *types[4];
      uint8_t location_frac_mask:4;
      uint8_t patch:1;
      struct {
         unsigned interpolation:3;   // INTERP_MODE_COUNT = 5
         unsigned driver_location:6; // VARYING_SLOT_MAX = 64
         unsigned compact:1;
      } vars[4];
   } slots[VARYING_SLOT_MAX];
   uint64_t mask;
   uint32_t hash;
   uint32_t max;
};

struct d3d12_image_format_conversion_info {
   enum pipe_format view_format, emulated_format;
};
struct d3d12_image_format_conversion_info_arr {
   int n_images;
   struct d3d12_image_format_conversion_info* image_format_conversion;
};

struct d3d12_shader_key {
   uint32_t hash;
   enum pipe_shader_type stage;

   struct d3d12_varying_info *required_varying_inputs;
   struct d3d12_varying_info *required_varying_outputs;
   uint64_t next_varying_inputs;
   uint64_t prev_varying_outputs;
   union {
      struct {
         unsigned last_vertex_processing_stage : 1;
         unsigned invert_depth : 16;
         unsigned halfz : 1;
         unsigned samples_int_textures : 1;
         unsigned input_clip_size : 4;
      };
      uint32_t common_all;
   };
   unsigned tex_saturate_s : PIPE_MAX_SAMPLERS;
   unsigned tex_saturate_r : PIPE_MAX_SAMPLERS;
   unsigned tex_saturate_t : PIPE_MAX_SAMPLERS;
   union {
      struct {
         unsigned needs_format_emulation:1;
         enum pipe_format format_conversion[PIPE_MAX_ATTRIBS];
      } vs;

      union {
         struct {
            unsigned sprite_coord_enable:24;
            unsigned sprite_origin_upper_left:1;
            unsigned point_pos_stream_out:1;
            unsigned writes_psize:1;
            unsigned point_size_per_vertex:1;
            unsigned aa_point:1;
            unsigned stream_output_factor:3;
            unsigned primitive_id:1;
            unsigned triangle_strip:1;
         };
         uint64_t all;
      } gs;

      struct {
         union {
            struct {
               uint32_t next_patch_inputs;
               unsigned primitive_mode:2;
               unsigned ccw:1;
               unsigned point_mode:1;
               unsigned spacing:2;
               unsigned patch_vertices_in:5;
            };
            uint64_t all;
         };
         struct d3d12_varying_info *required_patch_outputs;
      } hs;

      struct {
         unsigned tcs_vertices_out;
         uint32_t prev_patch_outputs;
         struct d3d12_varying_info *required_patch_inputs;
      } ds;

      union {
         struct {
            unsigned missing_dual_src_outputs : 2;
            unsigned frag_result_color_lowering : 4;
            unsigned cast_to_uint : 1;
            unsigned cast_to_int : 1;
            unsigned provoking_vertex : 2;
            unsigned manual_depth_range : 1;
            unsigned polygon_stipple : 1;
            unsigned remap_front_facing : 1;
            unsigned multisample_disabled : 1;
         };
         unsigned short all;
      } fs;

      struct {
         unsigned workgroup_size[3];
      } cs;
   };

   int n_texture_states;
   dxil_wrap_sampler_state *tex_wrap_states;
   dxil_texture_swizzle_state swizzle_state[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   enum compare_func sampler_compare_funcs[PIPE_MAX_SHADER_SAMPLER_VIEWS];

   int n_images;
   struct d3d12_image_format_conversion_info image_format_conversion[PIPE_MAX_SHADER_IMAGES];
};

struct d3d12_shader {
   void *bytecode;
   size_t bytecode_length;

   nir_shader *nir;
   struct d3d12_varying_info *output_vars_gs;
   struct d3d12_varying_info *output_vars_fs;
   struct d3d12_varying_info *output_vars_default;

   struct d3d12_varying_info *input_vars_vs;
   struct d3d12_varying_info *input_vars_default;

   struct d3d12_varying_info *tess_eval_output_vars;
   struct d3d12_varying_info *tess_ctrl_input_vars;

   struct {
      unsigned binding;
   } cb_bindings[PIPE_MAX_CONSTANT_BUFFERS];
   size_t num_cb_bindings;

   struct {
      enum d3d12_state_var var;
      unsigned offset;
   } state_vars[D3D12_MAX_STATE_VARS];
   unsigned num_state_vars;
   size_t state_vars_size;
   bool state_vars_used;

   struct {
      uint32_t dimension;
   } srv_bindings[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   size_t begin_srv_binding;
   size_t end_srv_binding;

   struct {
      uint32_t dimension;
   } uav_bindings[PIPE_MAX_SHADER_IMAGES];

   bool has_default_ubo0;
   unsigned pstipple_binding;

   struct d3d12_shader_key key;
   struct d3d12_shader *next_variant;
};

struct d3d12_gs_variant_key
{
   union {
      struct {
         unsigned passthrough:1;
         unsigned provoking_vertex:3;
         unsigned alternate_tri:1;
         unsigned fill_mode:2;
         unsigned cull_mode:2;
         unsigned has_front_face:1;
         unsigned front_ccw:1;
         unsigned edge_flag_fix:1;
         unsigned flatshade_first:1;
      };
      uint64_t all;
   };
   uint64_t flat_varyings;
   struct d3d12_varying_info *varyings;
};

struct d3d12_tcs_variant_key
{
   unsigned vertices_out;
   struct d3d12_varying_info *varyings;
};

struct d3d12_shader_selector {
   enum pipe_shader_type stage;
   const nir_shader *initial;
   struct d3d12_varying_info *initial_output_vars;

   struct d3d12_shader *first;
   struct d3d12_shader *current;

   struct pipe_stream_output_info so_info;

   unsigned samples_int_textures:1;
   unsigned compare_with_lod_bias_grad:1;
   unsigned workgroup_size_variable:1;

   bool is_variant;
   union {
      struct d3d12_gs_variant_key gs_key;
      struct d3d12_tcs_variant_key tcs_key;
   };
};

struct d3d12_shader_selector *
d3d12_create_shader(struct d3d12_context *ctx,
                    enum pipe_shader_type stage,
                    const struct pipe_shader_state *shader);

struct d3d12_shader_selector *
d3d12_create_compute_shader(struct d3d12_context *ctx,
                            const struct pipe_compute_state *shader);

void
d3d12_shader_free(struct d3d12_shader_selector *shader);

void
d3d12_select_shader_variants(struct d3d12_context *ctx,
                             const struct pipe_draw_info *dinfo);

void
d3d12_select_compute_shader_variants(struct d3d12_context *ctx,
                                     const struct pipe_grid_info *info);

void
d3d12_gs_variant_cache_init(struct d3d12_context *ctx);

void
d3d12_gs_variant_cache_destroy(struct d3d12_context *ctx);

struct d3d12_shader_selector *
d3d12_get_gs_variant(struct d3d12_context *ctx, struct d3d12_gs_variant_key *key);

void
d3d12_tcs_variant_cache_init(struct d3d12_context *ctx);

void
d3d12_tcs_variant_cache_destroy(struct d3d12_context *ctx);

struct d3d12_shader_selector *
d3d12_get_tcs_variant(struct d3d12_context *ctx, struct d3d12_tcs_variant_key *key);

unsigned
missing_dual_src_outputs(struct d3d12_context* ctx);

bool
has_flat_varyings(struct d3d12_context* ctx);

bool
d3d12_compare_varying_info(const struct d3d12_varying_info *expect, const struct d3d12_varying_info *have);

bool
manual_depth_range(struct d3d12_context* ctx);

#ifdef __cplusplus
}
#endif

#endif
