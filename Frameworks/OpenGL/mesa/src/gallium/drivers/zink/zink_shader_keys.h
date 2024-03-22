/*
 * Copyright 2020 Mike Blumenkrantz
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


/** this file exists for organization and to be included in nir_to_spirv/ without pulling in extra deps */
#ifndef ZINK_SHADER_KEYS_H
# define ZINK_SHADER_KEYS_H

#include "compiler/shader_info.h"

struct zink_vs_key_base {
   bool last_vertex_stage : 1;
   bool clip_halfz : 1;
   bool push_drawid : 1;
   bool robust_access : 1;
   uint8_t pad : 4;
};

struct zink_vs_key {
   struct zink_vs_key_base base;
   uint8_t pad;
   union {
      struct {
         uint32_t decomposed_attrs;
         uint32_t decomposed_attrs_without_w;
      } u32;
      struct {
         uint16_t decomposed_attrs;
         uint16_t decomposed_attrs_without_w;
      } u16;
      struct {
         uint8_t decomposed_attrs;
         uint8_t decomposed_attrs_without_w;
      } u8;
   };
   // not hashed
   unsigned size;
};

struct zink_gs_key {
   struct zink_vs_key_base base;
   uint8_t pad;
   bool lower_line_stipple : 1;
   bool lower_line_smooth : 1;
   bool lower_gl_point : 1;
   bool line_rectangular : 1;
   unsigned lower_pv_mode : 2;
   // not hashed
   unsigned size;
};

struct zink_zs_swizzle {
   uint8_t s[4];
};

struct zink_zs_swizzle_key {
   /* Mask of sampler views with zs_view, i.e. have swizzles other than GL_RED for depth */
   uint32_t mask;
   struct zink_zs_swizzle swizzle[32];
};

struct zink_fs_key_base {
   bool point_coord_yinvert : 1;
   bool samples : 1;
   bool force_dual_color_blend : 1;
   bool force_persample_interp : 1;
   bool fbfetch_ms : 1;
   bool shadow_needs_shader_swizzle : 1; //append zink_zs_swizzle_key after the key data
   uint8_t pad : 2;
   uint8_t coord_replace_bits;
};

struct zink_fs_key {
   struct zink_fs_key_base base;
   /* non-optimal bits after this point */
   bool lower_line_stipple : 1;
   bool lower_line_smooth : 1;
   bool lower_point_smooth : 1;
   bool robust_access : 1;
   uint16_t pad2 : 12;
};

struct zink_tcs_key {
   uint8_t patch_vertices;
};

/* when adding a new field, make sure
 * ctx->compute_pipeline_state.key.size is set in zink_context_create.
 */
struct zink_cs_key {
   bool robust_access : 1;
   uint32_t pad : 31;
};

struct zink_shader_key_base {
   bool needs_zs_shader_swizzle;
   uint32_t nonseamless_cube_mask;
   uint32_t inlined_uniform_values[MAX_INLINABLE_UNIFORMS];
};

/* a shader key is used for swapping out shader modules based on pipeline states,
 * e.g., if sampleCount changes, we must verify that the fs doesn't need a recompile
 *       to account for GL ignoring gl_SampleMask in some cases when VK will not
 * which allows us to avoid recompiling shaders when the pipeline state changes repeatedly
 */
struct zink_shader_key {
   union {
      /* reuse vs key for now with tes since we only use clip_halfz */
      struct zink_vs_key vs;
      struct zink_vs_key_base vs_base;
      struct zink_tcs_key tcs;
      struct zink_gs_key gs;
      struct zink_fs_key fs;
      struct zink_fs_key_base fs_base;
      struct zink_cs_key cs;
   } key;
   struct zink_shader_key_base base;
   unsigned inline_uniforms:1;
   uint32_t size;
};

union zink_shader_key_optimal {
   struct {
      struct zink_vs_key_base vs_base;
      struct zink_tcs_key tcs;
      struct zink_fs_key_base fs;
   };
   struct {
      uint8_t vs_bits;
      uint8_t tcs_bits;
      uint16_t fs_bits;
   };
   uint32_t val;
};

/* the default key has only last_vertex_stage set*/
#define ZINK_SHADER_KEY_OPTIMAL_DEFAULT (1<<0)
/* Ignore patch_vertices bits that would only be used if we had to generate the missing TCS */
static inline uint32_t
zink_shader_key_optimal_no_tcs(uint32_t key)
{
   union zink_shader_key_optimal k;
   k.val = key;
   k.tcs_bits = 0;
   return k.val;
}
#define ZINK_SHADER_KEY_OPTIMAL_IS_DEFAULT(key) (zink_shader_key_optimal_no_tcs(key) == ZINK_SHADER_KEY_OPTIMAL_DEFAULT)

static inline const struct zink_fs_key_base *
zink_fs_key_base(const struct zink_shader_key *key)
{
   assert(key);
   return &key->key.fs.base;
}

static inline const struct zink_fs_key *
zink_fs_key(const struct zink_shader_key *key)
{
   assert(key);
   return &key->key.fs;
}

static inline const struct zink_vs_key_base *
zink_vs_key_base(const struct zink_shader_key *key)
{
   return &key->key.vs_base;
}

static inline const struct zink_vs_key *
zink_vs_key(const struct zink_shader_key *key)
{
   assert(key);
   return &key->key.vs;
}

static inline const struct zink_gs_key *
zink_gs_key(const struct zink_shader_key *key)
{
   assert(key);
   return &key->key.gs;
}

static inline const struct zink_tcs_key *
zink_tcs_key(const struct zink_shader_key *key)
{
   assert(key);
   return &key->key.tcs;
}

static inline const struct zink_cs_key *
zink_cs_key(const struct zink_shader_key *key)
{
   assert(key);
   return &key->key.cs;
}

#endif
