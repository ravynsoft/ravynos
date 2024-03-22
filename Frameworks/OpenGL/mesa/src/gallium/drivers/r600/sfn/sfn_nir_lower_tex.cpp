/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
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

#include "sfn_nir_lower_tex.h"

#include "nir.h"
#include "nir_builder.h"
#include "nir_builtin_builder.h"

static bool
lower_coord_shift_normalized(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_before_instr(&tex->instr);

   nir_def *size = nir_i2f32(b, nir_get_texture_size(b, tex));
   nir_def *scale = nir_frcp(b, size);

   int coord_index = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   nir_def *corr = nullptr;
   if (unlikely(tex->array_is_lowered_cube)) {
      auto corr2 = nir_fadd(b,
                            nir_trim_vector(b, tex->src[coord_index].src.ssa, 2),
                            nir_fmul_imm(b, scale, -0.5f));
      corr = nir_vec3(b,
                      nir_channel(b, corr2, 0),
                      nir_channel(b, corr2, 1),
                      nir_channel(b, tex->src[coord_index].src.ssa, 2));
   } else {
      corr = nir_fadd(b,
                      nir_fmul_imm(b, scale, -0.5f),
                      tex->src[coord_index].src.ssa);
   }

   nir_src_rewrite(&tex->src[coord_index].src, corr);
   return true;
}

static bool
lower_coord_shift_unnormalized(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_before_instr(&tex->instr);
   int coord_index = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   nir_def *corr = nullptr;
   if (unlikely(tex->array_is_lowered_cube)) {
      auto corr2 = nir_fadd_imm(b,
                                nir_trim_vector(b, tex->src[coord_index].src.ssa, 2),
                                -0.5f);
      corr = nir_vec3(b,
                      nir_channel(b, corr2, 0),
                      nir_channel(b, corr2, 1),
                      nir_channel(b, tex->src[coord_index].src.ssa, 2));
   } else {
      corr = nir_fadd_imm(b, tex->src[coord_index].src.ssa, -0.5f);
   }
   nir_src_rewrite(&tex->src[coord_index].src, corr);
   return true;
}

static bool
r600_nir_lower_int_tg4_impl(nir_function_impl *impl)
{
   nir_builder b = nir_builder_create(impl);

   bool progress = false;
   nir_foreach_block(block, impl)
   {
      nir_foreach_instr_safe(instr, block)
      {
         if (instr->type == nir_instr_type_tex) {
            nir_tex_instr *tex = nir_instr_as_tex(instr);
            if (tex->op == nir_texop_tg4 && tex->sampler_dim != GLSL_SAMPLER_DIM_CUBE &&
                nir_tex_instr_src_index(tex, nir_tex_src_backend1) < 0) {
               if (nir_alu_type_get_base_type(tex->dest_type) != nir_type_float) {
                  if (tex->sampler_dim != GLSL_SAMPLER_DIM_RECT)
                     lower_coord_shift_normalized(&b, tex);
                  else
                     lower_coord_shift_unnormalized(&b, tex);
                  progress = true;
               }
            }
         }
      }
   }
   return progress;
}

/*
 * This lowering pass works around a bug in r600 when doing TG4 from
 * integral valued samplers.

 * Gather4 should follow the same rules as bilinear filtering, but the hardware
 * incorrectly forces nearest filtering if the texture format is integer.
 * The only effect it has on Gather4, which always returns 4 texels for
 * bilinear filtering, is that the final coordinates are off by 0.5 of
 * the texel size.
*/

bool
r600_nir_lower_int_tg4(nir_shader *shader)
{
   bool progress = false;
   bool need_lowering = false;

   nir_foreach_uniform_variable(var, shader)
   {
      if (glsl_type_is_sampler(var->type)) {
         if (glsl_base_type_is_integer(var->type->sampled_type)) {
            need_lowering = true;
         }
      }
   }

   if (need_lowering) {
      nir_foreach_function_impl(impl, shader)
      {
         if (r600_nir_lower_int_tg4_impl(impl))
            progress = true;
      }
   }

   return progress;
}

static bool
lower_txl_txf_array_or_cube(nir_builder *b, nir_tex_instr *tex)
{
   assert(tex->op == nir_texop_txb || tex->op == nir_texop_txl);
   assert(nir_tex_instr_src_index(tex, nir_tex_src_ddx) < 0);
   assert(nir_tex_instr_src_index(tex, nir_tex_src_ddy) < 0);

   b->cursor = nir_before_instr(&tex->instr);

   int lod_idx = nir_tex_instr_src_index(tex, nir_tex_src_lod);
   int bias_idx = nir_tex_instr_src_index(tex, nir_tex_src_bias);
   int min_lod_idx = nir_tex_instr_src_index(tex, nir_tex_src_min_lod);
   assert(lod_idx >= 0 || bias_idx >= 0);

   nir_def *size = nir_i2f32(b, nir_get_texture_size(b, tex));
   nir_def *lod = (lod_idx >= 0) ? tex->src[lod_idx].src.ssa
                                     : nir_get_texture_lod(b, tex);

   if (bias_idx >= 0)
      lod = nir_fadd(b, lod, tex->src[bias_idx].src.ssa);

   if (min_lod_idx >= 0)
      lod = nir_fmax(b, lod, tex->src[min_lod_idx].src.ssa);

   /* max lod? */

   nir_def *lambda_exp = nir_fexp2(b, lod);
   nir_def *scale = NULL;

   if (tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
      unsigned int swizzle[NIR_MAX_VEC_COMPONENTS] = {0, 0, 0, 0};
      scale = nir_frcp(b, nir_channels(b, size, 1));
      scale = nir_swizzle(b, scale, swizzle, 3);
   } else if (tex->is_array) {
      int cmp_mask = (1 << (size->num_components - 1)) - 1;
      scale = nir_frcp(b, nir_channels(b, size, (nir_component_mask_t)cmp_mask));
   }

   nir_def *grad = nir_fmul(b, lambda_exp, scale);

   if (lod_idx >= 0)
      nir_tex_instr_remove_src(tex, lod_idx);
   if (bias_idx >= 0)
      nir_tex_instr_remove_src(tex, bias_idx);
   if (min_lod_idx >= 0)
      nir_tex_instr_remove_src(tex, min_lod_idx);
   nir_tex_instr_add_src(tex, nir_tex_src_ddx, grad);
   nir_tex_instr_add_src(tex, nir_tex_src_ddy, grad);

   tex->op = nir_texop_txd;
   return true;
}

static bool
r600_nir_lower_txl_txf_array_or_cube_impl(nir_function_impl *impl)
{
   nir_builder b = nir_builder_create(impl);

   bool progress = false;
   nir_foreach_block(block, impl)
   {
      nir_foreach_instr_safe(instr, block)
      {
         if (instr->type == nir_instr_type_tex) {
            nir_tex_instr *tex = nir_instr_as_tex(instr);

            if (tex->is_shadow &&
                (tex->op == nir_texop_txl || tex->op == nir_texop_txb) &&
                (tex->is_array || tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE))
               progress |= lower_txl_txf_array_or_cube(&b, tex);
         }
      }
   }
   return progress;
}

bool
r600_nir_lower_txl_txf_array_or_cube(nir_shader *shader)
{
   bool progress = false;
   nir_foreach_function_impl(impl, shader)
   {
      if (r600_nir_lower_txl_txf_array_or_cube_impl(impl))
         progress = true;
   }
   return progress;
}

static bool
r600_nir_lower_cube_to_2darray_filer(const nir_instr *instr, const void *_options)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   auto tex = nir_instr_as_tex(instr);
   if (tex->sampler_dim != GLSL_SAMPLER_DIM_CUBE)
      return false;

   switch (tex->op) {
   case nir_texop_tex:
   case nir_texop_txb:
   case nir_texop_txf:
   case nir_texop_txl:
   case nir_texop_lod:
   case nir_texop_tg4:
   case nir_texop_txd:
      return true;
   default:
      return false;
   }
}

static nir_def *
r600_nir_lower_cube_to_2darray_impl(nir_builder *b, nir_instr *instr, void *_options)
{
   b->cursor = nir_before_instr(instr);

   auto tex = nir_instr_as_tex(instr);
   int coord_idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   assert(coord_idx >= 0);

   auto cubed = nir_cube_amd(b,
                             nir_trim_vector(b, tex->src[coord_idx].src.ssa, 3));
   auto xy = nir_fmad(b,
                      nir_vec2(b, nir_channel(b, cubed, 1), nir_channel(b, cubed, 0)),
                      nir_frcp(b, nir_fabs(b, nir_channel(b, cubed, 2))),
                      nir_imm_float(b, 1.5));

   nir_def *z = nir_channel(b, cubed, 3);
   if (tex->is_array && tex->op != nir_texop_lod) {
      auto slice = nir_fround_even(b, nir_channel(b, tex->src[coord_idx].src.ssa, 3));
      z =
         nir_fmad(b, nir_fmax(b, slice, nir_imm_float(b, 0.0)), nir_imm_float(b, 8.0), z);
   }

   if (tex->op == nir_texop_txd) {
      int ddx_idx = nir_tex_instr_src_index(tex, nir_tex_src_ddx);
      nir_src_rewrite(&tex->src[ddx_idx].src,
                      nir_fmul_imm(b, tex->src[ddx_idx].src.ssa, 0.5));

      int ddy_idx = nir_tex_instr_src_index(tex, nir_tex_src_ddy);
      nir_src_rewrite(&tex->src[ddy_idx].src,
                      nir_fmul_imm(b, tex->src[ddy_idx].src.ssa, 0.5));
   }

   auto new_coord = nir_vec3(b, nir_channel(b, xy, 0), nir_channel(b, xy, 1), z);
   nir_src_rewrite(&tex->src[coord_idx].src, new_coord);
   tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
   tex->is_array = true;
   tex->array_is_lowered_cube = true;

   tex->coord_components = 3;

   return NIR_LOWER_INSTR_PROGRESS;
}

bool
r600_nir_lower_cube_to_2darray(nir_shader *shader)
{
   return nir_shader_lower_instructions(shader,
                                        r600_nir_lower_cube_to_2darray_filer,
                                        r600_nir_lower_cube_to_2darray_impl,
                                        nullptr);
}
