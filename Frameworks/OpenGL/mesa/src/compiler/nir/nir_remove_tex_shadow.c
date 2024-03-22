/*
 * Copyright © 2023 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "nir.h"
#include "nir_builder.h"

static const struct glsl_type *
strip_shadow(const struct glsl_type *type)
{
   const struct glsl_type *new_type =
      glsl_sampler_type(
         glsl_get_sampler_dim(type),
         false, glsl_sampler_type_is_array(type),
         GLSL_TYPE_FLOAT);
   return new_type;
}

static inline const struct glsl_type *
strip_shadow_with_array(const struct glsl_type *type)
{
   return glsl_type_wrap_in_arrays(strip_shadow(glsl_without_array(type)), type);
}

static bool
change_deref_var_type(struct nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_deref)
      return false;

   nir_variable *sampler = data;
   nir_deref_instr *deref = nir_instr_as_deref(instr);
   if (deref->var == sampler) {
      deref->type = sampler->type;
      return true;
   }

   return false;
}

static bool
remove_tex_shadow(struct nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   if (!tex->is_shadow)
      return false;

   unsigned *textures_bitmask = data;

   if (BITFIELD_BIT(tex->texture_index) & ~*textures_bitmask)
      return false;

   int index = nir_tex_instr_src_index(tex, nir_tex_src_comparator);

   if (index != -1) {
      nir_deref_instr *sampler_deref = NULL;
      nir_variable *sampler = NULL;
      int sampler_src_index = nir_tex_instr_src_index(tex, nir_tex_src_sampler_deref);
      if (sampler_src_index >= 0) {
         sampler_deref = nir_instr_as_deref(tex->src[sampler_src_index].src.ssa->parent_instr);
         sampler = nir_deref_instr_get_variable(sampler_deref);
         sampler->type = strip_shadow_with_array(sampler->type);
         sampler_deref->type = sampler->type;
      } else {
         sampler = nir_find_sampler_variable_with_tex_index(b->shader,
                                                            tex->texture_index);
         sampler->type = strip_shadow_with_array(sampler->type);
      }

      nir_shader_instructions_pass(b->shader, change_deref_var_type,
                                   nir_metadata_none, sampler);
      tex->is_shadow = false;
      nir_tex_instr_remove_src(tex, index);
      return true;
   }

   return false;
}

bool
nir_remove_tex_shadow(nir_shader *shader, unsigned textures_bitmask)
{
   return nir_shader_instructions_pass(shader, remove_tex_shadow,
                                       nir_metadata_none, &textures_bitmask);
}
