/*
 * Copyright (C) 2005-2007  Brian Paul   All Rights Reserved.
 * Copyright (C) 2008  VMware, Inc.   All Rights Reserved.
 * Copyright © 2014 Intel Corporation
 * Copyright © 2017 Advanced Micro Devices, Inc.
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

/**
 * \file
 *
 * Lower sampler and image references of (non-bindless) uniforms by removing
 * struct dereferences, and synthesizing new uniform variables without structs
 * if required.
 *
 * This will allow backends to have a simple, uniform treatment of bindless and
 * non-bindless samplers and images.
 *
 * Example:
 *
 *   struct S {
 *      sampler2D tex[2];
 *      sampler2D other;
 *   };
 *   uniform S s[2];
 *
 *   tmp = texture(s[n].tex[m], coord);
 *
 * Becomes:
 *
 *   decl_var uniform INTERP_MODE_NONE sampler2D[2][2] lower@s.tex (...)
 *
 *   vec1 32 ssa_idx = $(2 * n + m)
 *   vec4 32 ssa_out = tex ssa_coord (coord), lower@s.tex[n][m] (texture), lower@s.tex[n][m] (sampler)
 *
 * and lower@s.tex has var->data.binding set to the base index as defined by
 * the opaque uniform mapping.
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_deref.h"
#include "gl_nir.h"
#include "ir_uniform.h"

#include "util/compiler.h"
#include "main/shader_types.h"

struct lower_samplers_as_deref_state {
   nir_shader *shader;
   const struct gl_shader_program *shader_program;
   struct hash_table *remap_table;
};

/* Prepare for removing struct derefs.  This pre-pass generates the name
 * of the lowered deref, and calculates the lowered type and location.
 * After that, once looking up (or creating if needed) the lowered var,
 * constructing the new chain of deref instructions is a simple loop
 * that skips the struct deref's
 *
 * path:     appended to as we descend down the chain of deref instrs
 *           and remove struct derefs
 * location: increased as we descend down and remove struct derefs
 * type:     updated as we recurse back up the chain of deref instrs
 *           with the resulting type after removing struct derefs
 */
static void
remove_struct_derefs_prep(nir_deref_instr **p, char **name,
                          unsigned *location, const struct glsl_type **type)
{
   nir_deref_instr *cur = p[0], *next = p[1];

   if (!next) {
      *type = cur->type;
      return;
   }

   switch (next->deref_type) {
   case nir_deref_type_array: {
      unsigned length = glsl_get_length(cur->type);

      remove_struct_derefs_prep(&p[1], name, location, type);

      *type = glsl_array_type(*type, length, glsl_get_explicit_stride(cur->type));
      break;
   }

   case nir_deref_type_struct: {
      *location += glsl_get_struct_location_offset(cur->type, next->strct.index);
      ralloc_asprintf_append(name, ".%s",
                             glsl_get_struct_elem_name(cur->type, next->strct.index));

      remove_struct_derefs_prep(&p[1], name, location, type);
      break;
   }

   default:
      unreachable("Invalid deref type");
      break;
   }
}

static void
record_images_used(struct shader_info *info,
                   nir_intrinsic_instr *instr)
{
   nir_variable *var = nir_intrinsic_get_var(instr, 0);

   /* Structs have been lowered already, so get_aoa_size is sufficient. */
   const unsigned size =
      glsl_type_is_array(var->type) ? glsl_get_aoa_size(var->type) : 1;

   BITSET_SET_RANGE(info->images_used, var->data.binding,
                    var->data.binding + (MAX2(size, 1) - 1));

   enum glsl_sampler_dim sampler_dim =
      glsl_get_sampler_dim(glsl_without_array(var->type));
   if (sampler_dim == GLSL_SAMPLER_DIM_BUF) {
      BITSET_SET_RANGE(info->image_buffers, var->data.binding,
                       var->data.binding + (MAX2(size, 1) - 1));
   }
   if (sampler_dim == GLSL_SAMPLER_DIM_MS) {
      BITSET_SET_RANGE(info->msaa_images, var->data.binding,
                       var->data.binding + (MAX2(size, 1) - 1));
   }
}


static nir_deref_instr *
lower_deref(nir_builder *b, struct lower_samplers_as_deref_state *state,
            nir_deref_instr *deref)
{
   nir_variable *var = nir_deref_instr_get_variable(deref);
   gl_shader_stage stage = state->shader->info.stage;

   if (!(var->data.mode & (nir_var_uniform | nir_var_image)) ||
       var->data.bindless)
      return NULL;

   nir_deref_path path;
   nir_deref_path_init(&path, deref, state->remap_table);
   assert(path.path[0]->deref_type == nir_deref_type_var);

   char *name = ralloc_asprintf(state->remap_table, "lower@%s", var->name);
   unsigned location = var->data.location;
   const struct glsl_type *type = NULL;
   unsigned binding;

   /*
    * We end up needing to do this in two passes, in order to generate
    * the name of the lowered var (and detecting whether there even are
    * any struct deref's), and then the second pass to construct the
    * actual deref instructions after looking up / generating a new
    * nir_variable (since we need to construct the deref_var first)
    */

   remove_struct_derefs_prep(path.path, &name, &location, &type);

   if (state->shader_program && var->data.how_declared != nir_var_hidden) {
      /* For GLSL programs, look up the bindings in the uniform storage. */
      assert(location < state->shader_program->data->NumUniformStorage &&
             state->shader_program->data->UniformStorage[location].opaque[stage].active);

      binding = state->shader_program->data->UniformStorage[location].opaque[stage].index;
   } else {
      /* For ARB programs, built-in shaders, or internally generated sampler
       * variables in GLSL programs, assume that whoever created the shader
       * set the bindings correctly already.
       */
      assert(var->data.explicit_binding);
      binding = var->data.binding;
   }

   if (var->type == type) {
      /* Fast path: We did not encounter any struct derefs. */
      var->data.binding = binding;
      return deref;
   }

   uint32_t hash = _mesa_hash_string(name);
   struct hash_entry *h =
      _mesa_hash_table_search_pre_hashed(state->remap_table, hash, name);

   if (h) {
      var = (nir_variable *)h->data;
   } else {
      var = nir_variable_create(state->shader, var->data.mode, type, name);
      var->data.binding = binding;

      /* Don't set var->data.location.  The old structure location could be
       * used to index into gl_uniform_storage, assuming the full structure
       * was walked in order.  With the new split variables, this invariant
       * no longer holds and there's no meaningful way to start from a base
       * location and access a particular array element.  Just leave it 0.
       */

      _mesa_hash_table_insert_pre_hashed(state->remap_table, hash, name, var);
   }

   /* construct a new deref based on lowered var (skipping the struct deref's
    * from the original deref:
    */
   nir_deref_instr *new_deref = nir_build_deref_var(b, var);
   for (nir_deref_instr **p = &path.path[1]; *p; p++) {
      if ((*p)->deref_type == nir_deref_type_struct)
         continue;

      assert((*p)->deref_type == nir_deref_type_array);

      new_deref = nir_build_deref_array(b, new_deref,
                                        (*p)->arr.index.ssa);
   }

   return new_deref;
}

static void
record_textures_used(struct shader_info *info,
                     nir_deref_instr *deref,
                     nir_texop op)
{
   nir_variable *var = nir_deref_instr_get_variable(deref);

   /* Structs have been lowered already, so get_aoa_size is sufficient. */
   const unsigned size =
      glsl_type_is_array(var->type) ? glsl_get_aoa_size(var->type) : 1;

   BITSET_SET_RANGE(info->textures_used, var->data.binding,
                    var->data.binding + (MAX2(size, 1) - 1));

   if (op == nir_texop_txf ||
       op == nir_texop_txf_ms ||
       op == nir_texop_txf_ms_mcs_intel) {
      BITSET_SET_RANGE(info->textures_used_by_txf, var->data.binding,
                       var->data.binding + (MAX2(size, 1) - 1));
   }
}

static void
record_samplers_used(struct shader_info *info,
                     nir_deref_instr *deref,
                     nir_texop op)
{
   nir_variable *var = nir_deref_instr_get_variable(deref);

   /* Structs have been lowered already, so get_aoa_size is sufficient. */
   const unsigned size =
      glsl_type_is_array(var->type) ? glsl_get_aoa_size(var->type) : 1;

   BITSET_SET_RANGE(info->samplers_used, var->data.binding,
                    var->data.binding + (MAX2(size, 1) - 1));
}

static bool
lower_sampler(nir_tex_instr *instr, struct lower_samplers_as_deref_state *state,
              nir_builder *b)
{
   int texture_idx =
      nir_tex_instr_src_index(instr, nir_tex_src_texture_deref);
   int sampler_idx =
      nir_tex_instr_src_index(instr, nir_tex_src_sampler_deref);

   b->cursor = nir_before_instr(&instr->instr);

   if (texture_idx >= 0) {
      nir_deref_instr *texture_deref =
         lower_deref(b, state, nir_src_as_deref(instr->src[texture_idx].src));
      /* only lower non-bindless: */
      if (texture_deref) {
         nir_src_rewrite(&instr->src[texture_idx].src, &texture_deref->def);
         record_textures_used(&b->shader->info, texture_deref, instr->op);
      }
   }

   if (sampler_idx >= 0) {
      nir_deref_instr *sampler_deref =
         lower_deref(b, state, nir_src_as_deref(instr->src[sampler_idx].src));
      /* only lower non-bindless: */
      if (sampler_deref) {
         nir_src_rewrite(&instr->src[sampler_idx].src, &sampler_deref->def);
         record_samplers_used(&b->shader->info, sampler_deref, instr->op);
      }
   }

   return true;
}

static bool
lower_intrinsic(nir_intrinsic_instr *instr,
                struct lower_samplers_as_deref_state *state,
                nir_builder *b)
{
   if (instr->intrinsic == nir_intrinsic_image_deref_load ||
       instr->intrinsic == nir_intrinsic_image_deref_store ||
       instr->intrinsic == nir_intrinsic_image_deref_atomic ||
       instr->intrinsic == nir_intrinsic_image_deref_atomic_swap ||
       instr->intrinsic == nir_intrinsic_image_deref_size ||
       instr->intrinsic == nir_intrinsic_image_deref_samples_identical ||
       instr->intrinsic == nir_intrinsic_image_deref_descriptor_amd ||
       instr->intrinsic == nir_intrinsic_image_deref_samples) {

      b->cursor = nir_before_instr(&instr->instr);
      nir_deref_instr *deref =
         lower_deref(b, state, nir_src_as_deref(instr->src[0]));

      record_images_used(&state->shader->info, instr);

      /* don't lower bindless: */
      if (!deref)
         return false;
      nir_src_rewrite(&instr->src[0], &deref->def);
      return true;
   }
   if (instr->intrinsic == nir_intrinsic_image_deref_order ||
       instr->intrinsic == nir_intrinsic_image_deref_format)
      unreachable("how did you even manage this?");

   return false;
}

static bool
lower_instr(nir_builder *b, nir_instr *instr, void *cb_data)
{
   struct lower_samplers_as_deref_state *state = cb_data;

   if (instr->type == nir_instr_type_tex)
      return lower_sampler(nir_instr_as_tex(instr), state, b);

   if (instr->type == nir_instr_type_intrinsic)
      return lower_intrinsic(nir_instr_as_intrinsic(instr), state, b);

   return false;
}

bool
gl_nir_lower_samplers_as_deref(nir_shader *shader,
                               const struct gl_shader_program *shader_program)
{
   struct lower_samplers_as_deref_state state;

   state.shader = shader;
   state.shader_program = shader_program;
   state.remap_table = _mesa_hash_table_create(NULL, _mesa_hash_string,
                                               _mesa_key_string_equal);

   bool progress = nir_shader_instructions_pass(shader, lower_instr,
                                                nir_metadata_block_index |
                                                nir_metadata_dominance,
                                                &state);

   if (progress) {
      nir_remove_dead_derefs(shader);
      if (!shader->info.internal && shader_program) {
         /* try to apply bindings for unused samplers to avoid index zero clobbering in backends */
         nir_foreach_uniform_variable(var, shader) {
            /* ignore hidden variables */
            if (!glsl_type_is_sampler(glsl_without_array(var->type)) ||
                var->data.how_declared == nir_var_hidden)
               continue;
            bool found = false;
            hash_table_foreach(state.remap_table, entry) {
               if (var == entry->data) {
                  found = true;
                  break;
               }
            }
            if (!found) {
               /* same as lower_deref() */
               var->data.binding = shader_program->data->UniformStorage[var->data.location].opaque[shader->info.stage].index;
            }
         }
      }
   }

   /* keys are freed automatically by ralloc */
   _mesa_hash_table_destroy(state.remap_table, NULL);

   return progress;
}
