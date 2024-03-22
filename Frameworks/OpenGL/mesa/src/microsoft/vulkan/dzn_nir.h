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

#ifndef DZN_NIR_H
#define DZN_NIR_H

#define D3D12_IGNORE_SDK_LAYERS
#define COBJMACROS
#include <unknwn.h>
#include <directx/d3d12.h>

#include "nir.h"

struct dzn_indirect_draw_params {
   uint32_t vertex_count;
   uint32_t instance_count;
   uint32_t first_vertex;
   uint32_t first_instance;
};

struct dzn_indirect_indexed_draw_params {
   uint32_t index_count;
   uint32_t instance_count;
   uint32_t first_index;
   int32_t vertex_offset;
   uint32_t first_instance;
};

struct dzn_indirect_draw_rewrite_params {
   uint32_t draw_buf_stride;
};

struct dzn_indirect_draw_triangle_fan_rewrite_params {
   uint32_t draw_buf_stride;
   uint32_t triangle_fan_index_buf_stride;
   uint64_t triangle_fan_index_buf_start;
};

struct dzn_indirect_draw_triangle_fan_prim_restart_rewrite_params {
   uint32_t draw_buf_stride;
   uint32_t triangle_fan_index_buf_stride;
   uint64_t triangle_fan_index_buf_start;
   uint64_t exec_buf_start;
};

struct dzn_indirect_draw_exec_params {
   struct {
      uint32_t first_vertex;
      uint32_t base_instance;
      uint32_t draw_id;
   } sysvals;
   union {
      struct dzn_indirect_draw_params draw;
      struct dzn_indirect_indexed_draw_params indexed_draw;
   };
};

struct dzn_indirect_triangle_fan_draw_exec_params {
   D3D12_INDEX_BUFFER_VIEW ibview;
   struct {
      uint32_t first_vertex;
      uint32_t base_instance;
      uint32_t draw_id;
   } sysvals;
   union {
      struct dzn_indirect_draw_params draw;
      struct dzn_indirect_indexed_draw_params indexed_draw;
   };
};

struct dzn_triangle_fan_rewrite_index_params {
   uint32_t first_index;
};

struct dzn_triangle_fan_prim_restart_rewrite_index_params {
   uint32_t first_index;
   uint32_t index_count;
};

struct dzn_indirect_triangle_fan_rewrite_index_exec_params {
   uint64_t new_index_buf;
   struct dzn_triangle_fan_rewrite_index_params params;
   struct {
      uint32_t x, y, z;
   } group_count;
};

struct dzn_indirect_triangle_fan_prim_restart_rewrite_index_exec_params {
   uint64_t new_index_buf;
   struct dzn_triangle_fan_prim_restart_rewrite_index_params params;
   uint64_t index_count_ptr;
   struct {
      uint32_t x, y, z;
   } group_count;
};

enum dzn_indirect_draw_type {
   DZN_INDIRECT_DRAW,
   DZN_INDIRECT_DRAW_COUNT,
   DZN_INDIRECT_INDEXED_DRAW,
   DZN_INDIRECT_INDEXED_DRAW_COUNT,
   DZN_INDIRECT_DRAW_TRIANGLE_FAN,
   DZN_INDIRECT_DRAW_COUNT_TRIANGLE_FAN,
   DZN_INDIRECT_INDEXED_DRAW_TRIANGLE_FAN,
   DZN_INDIRECT_INDEXED_DRAW_COUNT_TRIANGLE_FAN,
   DZN_INDIRECT_INDEXED_DRAW_TRIANGLE_FAN_PRIM_RESTART,
   DZN_INDIRECT_INDEXED_DRAW_COUNT_TRIANGLE_FAN_PRIM_RESTART,
   DZN_NUM_INDIRECT_DRAW_TYPES,
};

nir_shader *
dzn_nir_indirect_draw_shader(enum dzn_indirect_draw_type type);

nir_shader *
dzn_nir_triangle_fan_rewrite_index_shader(uint8_t old_index_size);

nir_shader *
dzn_nir_triangle_fan_prim_restart_rewrite_index_shader(uint8_t old_index_size);

enum dzn_blit_resolve_mode {
   dzn_blit_resolve_none,
   dzn_blit_resolve_average,
   dzn_blit_resolve_min,
   dzn_blit_resolve_max,
   dzn_blit_resolve_sample_zero,
};
struct dzn_nir_blit_info {
   union {
      struct {
         uint32_t src_samples : 6;
         uint32_t loc : 4;
         uint32_t out_type : 4;
         uint32_t sampler_dim : 4;
         uint32_t src_is_array : 1;
         uint32_t resolve_mode : 3;
         uint32_t padding : 10;
      };
      const uint32_t hash_key;
   };
};

nir_shader *
dzn_nir_blit_vs(void);

nir_shader *
dzn_nir_blit_fs(const struct dzn_nir_blit_info *info);

struct dzn_nir_point_gs_info {
   unsigned cull_mode;
   bool front_ccw;
   bool depth_bias;
   bool depth_bias_dynamic;
   DXGI_FORMAT ds_fmt;
   /* Constant values */
   float constant_depth_bias;
   float slope_scaled_depth_bias;
   float depth_bias_clamp;
   /* Used for loading dynamic values */
   struct {
      uint32_t register_space;
      uint32_t base_shader_register;
   } runtime_data_cbv;
};
nir_shader *
dzn_nir_polygon_point_mode_gs(const nir_shader *vs, struct dzn_nir_point_gs_info *info);

#endif
