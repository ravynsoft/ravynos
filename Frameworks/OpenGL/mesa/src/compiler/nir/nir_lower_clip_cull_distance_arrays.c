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

#include "nir.h"
#include "nir_builder.h"
#include "nir_deref.h"

/**
 * This file contains two different lowering passes.
 *
 * 1. nir_lower_clip_cull_distance_arrays()
 *
 *    This pass combines clip and cull distance arrays in separate locations
 *    and colocates them both in VARYING_SLOT_CLIP_DIST0.  It does so by
 *    maintaining two arrays but making them compact and using location_frac
 *    to stack them on top of each other.
 *
 * 2. nir_lower_clip_cull_distance_to_vec4s()
 *
 *    This pass accounts for the difference between the way
 *    gl_ClipDistance is declared in standard GLSL (as an array of
 *    floats), and the way it is frequently implemented in hardware (as
 *    a pair of vec4s, with four clip distances packed into each).
 *
 *    The declaration of gl_ClipDistance is replaced with a declaration
 *    of gl_ClipDistanceMESA, and any references to gl_ClipDistance are
 *    translated to refer to gl_ClipDistanceMESA with the appropriate
 *    swizzling of array indices.  For instance:
 *
 *      gl_ClipDistance[i]
 *
 *    is translated into:
 *
 *      gl_ClipDistanceMESA[i>>2][i&3]
 */

#define GLSL_CLIP_VAR_NAME "gl_ClipDistanceMESA"

struct lower_distance_state {
   /**
    * Pointer to the declaration of gl_ClipDistance, if found.
    *
    * Note:
    *
    * - the in_var is for geometry and both tessellation shader inputs only.
    *
    * - since gl_ClipDistance is available in tessellation control,
    *   tessellation evaluation and geometry shaders as both an input
    *   and an output, it's possible for both old_distance_out_var
    *   and old_distance_in_var to be non-null.
    */
   nir_variable *old_distance_out_var;
   nir_variable *old_distance_in_var;

   /**
    * Pointer to the newly-created gl_ClipDistanceMESA variable.
    */
   nir_variable *new_distance_out_var;
   nir_variable *new_distance_in_var;

   /**
    * Type of shader we are compiling (e.g. MESA_SHADER_VERTEX)
    */
   gl_shader_stage shader_stage;
   const char *in_name;
   int total_size;
   int offset;
};

/**
 * Get the length of the clip/cull distance array, looking past
 * any interface block arrays.
 */
static unsigned
get_unwrapped_array_length(nir_shader *nir, nir_variable *var)
{
   if (!var)
      return 0;

   /* Unwrap GS input and TCS input/output interfaces.  We want the
    * underlying clip/cull distance array length, not the per-vertex
    * array length.
    */
   const struct glsl_type *type = var->type;
   if (nir_is_arrayed_io(var, nir->info.stage))
      type = glsl_get_array_element(type);

   if (var->data.per_view) {
      assert(glsl_type_is_array(type));
      type = glsl_get_array_element(type);
   }

   assert(glsl_type_is_array(type));

   return glsl_get_length(type);
}

/**
 * Replace any declaration of 'in_name' as an array of floats with a
 * declaration of gl_ClipDistanceMESA as an array of vec4's.
 */
static void
replace_var_declaration(struct lower_distance_state *state, nir_shader *sh,
                        nir_variable *var, const char *in_name)
{
   nir_variable **old_var;
   nir_variable **new_var;

   if (!var->name || strcmp(var->name, in_name) != 0)
      return;

   assert(glsl_type_is_array(var->type));
   if (var->data.mode == nir_var_shader_out) {
      if (state->old_distance_out_var)
         return;

      old_var = &state->old_distance_out_var;
      new_var = &state->new_distance_out_var;
   } else if (var->data.mode == nir_var_shader_in) {
      if (state->old_distance_in_var)
         return;

      old_var = &state->old_distance_in_var;
      new_var = &state->new_distance_in_var;
   } else {
      unreachable("not reached");
   }

   *old_var = var;

   if (!(*new_var)) {
      unsigned new_size = (state->total_size + 3) / 4;

      *new_var = rzalloc(sh, nir_variable);
      (*new_var)->name = ralloc_strdup(*new_var, GLSL_CLIP_VAR_NAME);
      (*new_var)->data.mode = var->data.mode;
      (*new_var)->data.location = VARYING_SLOT_CLIP_DIST0;
      (*new_var)->data.assigned = true;
      (*new_var)->data.how_declared = var->data.how_declared;

      nir_shader_add_variable(sh, *new_var);

      if (!glsl_type_is_array(glsl_get_array_element(var->type))) {
         /* gl_ClipDistance (used for vertex, tessellation evaluation and
          * geometry output, and fragment input).
          */
         assert((var->data.mode == nir_var_shader_in &&
                 sh->info.stage == MESA_SHADER_FRAGMENT) ||
                (var->data.mode == nir_var_shader_out &&
                 (sh->info.stage == MESA_SHADER_VERTEX ||
                  sh->info.stage == MESA_SHADER_TESS_EVAL ||
                  sh->info.stage == MESA_SHADER_GEOMETRY)));

         assert(glsl_get_base_type(glsl_get_array_element(var->type)) ==
                GLSL_TYPE_FLOAT);

         /* And change the properties that we need to change */
         (*new_var)->type = glsl_array_type(glsl_vec4_type(), new_size, 0);
      } else {
         /* 2D gl_ClipDistance (used for tessellation control, tessellation
          * evaluation and geometry input, and tessellation control output).
          */
         assert((var->data.mode == nir_var_shader_in &&
                 (sh->info.stage == MESA_SHADER_GEOMETRY ||
                  sh->info.stage == MESA_SHADER_TESS_EVAL)) ||
                sh->info.stage == MESA_SHADER_TESS_CTRL);

         assert (glsl_get_base_type(glsl_get_array_element(glsl_get_array_element(var->type))) ==
                 GLSL_TYPE_FLOAT);

         /* And change the properties that we need to change */
         (*new_var)->type =
            glsl_array_type(glsl_array_type(glsl_vec4_type(), new_size, 0),
                            glsl_array_size(var->type), 0);
      }
   }
}

static nir_def *
interp_deref(nir_builder *b, nir_intrinsic_instr *old_intrin,
             nir_deref_instr *deref)
{
   nir_intrinsic_instr *intrin =
      nir_intrinsic_instr_create(b->shader, old_intrin->intrinsic);
   intrin->num_components = 4;
   intrin->src[0] = nir_src_for_ssa(&deref->def);

   if (intrin->intrinsic == nir_intrinsic_interp_deref_at_offset ||
       intrin->intrinsic == nir_intrinsic_interp_deref_at_sample)
      intrin->src[1] = nir_src_for_ssa(old_intrin->src[1].ssa);

   nir_def_init(&intrin->instr, &intrin->def, 4, 32);
   nir_builder_instr_insert(b, &intrin->instr);

   return &intrin->def;
}

/* Replace any expression that indexes one of the floats in gl_ClipDistance
 * with an expression that indexes into one of the vec4's in
 * gl_ClipDistanceMESA and accesses the appropriate component.
 */
static void
lower_distance_deref(struct lower_distance_state *state, nir_builder *b,
                     nir_intrinsic_instr *intrin, nir_deref_instr *deref,
                     nir_variable *new_var)
{
   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);

   assert(path.path[0]->deref_type == nir_deref_type_var);
   nir_deref_instr **p = &path.path[1];

   b->cursor = nir_before_instr(&intrin->instr);
   nir_deref_instr *deref_var = nir_build_deref_var(b, new_var);

   /* Handle 2D arrays such as Geom shader inputs */
   if (glsl_type_is_array(glsl_get_array_element(new_var->type))) {
      assert((*p)->deref_type == nir_deref_type_array);
      deref_var = nir_build_deref_array(b, deref_var, (*p)->arr.index.ssa);
      p++;
   }

   assert((*p)->deref_type == nir_deref_type_array);

   /**
    * Create the necessary values to index into gl_ClipDistanceMESA based
    * on the value previously used to index into gl_ClipDistance.
    *
    * An array index selects one of the vec4's in gl_ClipDistanceMESA
    * a swizzle then selects a component within the selected vec4.
    */
   nir_src old_index = (*p)->arr.index;
   if (nir_src_is_const(old_index)) {
      unsigned const_val = nir_src_as_uint(old_index) + state->offset;
      unsigned swizzle = const_val % 4;

      nir_deref_instr *def_arr_instr =
         nir_build_deref_array_imm(b, deref_var, const_val / 4);

      if (intrin->intrinsic == nir_intrinsic_store_deref) {
         nir_def *value = intrin->src[1].ssa;
         nir_build_write_masked_store(b, def_arr_instr, value, swizzle);
      } else {
         assert(intrin->intrinsic == nir_intrinsic_load_deref ||
                intrin->intrinsic == nir_intrinsic_interp_deref_at_centroid ||
                intrin->intrinsic == nir_intrinsic_interp_deref_at_sample ||
                intrin->intrinsic == nir_intrinsic_interp_deref_at_offset);

         nir_def *load_def;
         if (intrin->intrinsic == nir_intrinsic_load_deref)
            load_def = nir_load_deref(b, def_arr_instr);
         else
            load_def = interp_deref(b, intrin, def_arr_instr);

         nir_def *swz = nir_channel(b, load_def, swizzle);
         nir_def_rewrite_uses(&intrin->def, swz);
      }
   } else {
      nir_def *index = nir_iadd_imm(b, old_index.ssa, state->offset);
      nir_def *swizzle = nir_umod_imm(b, index, 4);
      index = nir_ishr_imm(b, index, 2); /* index / 4 */

      nir_deref_instr *def_arr_instr =
         nir_build_deref_array(b, deref_var, index);

      if (intrin->intrinsic == nir_intrinsic_store_deref) {
         nir_def *value = intrin->src[1].ssa;
         nir_build_write_masked_stores(b, def_arr_instr, value, swizzle, 0, 4);
      } else {
         assert(intrin->intrinsic == nir_intrinsic_load_deref ||
                intrin->intrinsic == nir_intrinsic_interp_deref_at_centroid ||
                intrin->intrinsic == nir_intrinsic_interp_deref_at_sample ||
                intrin->intrinsic == nir_intrinsic_interp_deref_at_offset);

         nir_def *load_def;
         if (intrin->intrinsic == nir_intrinsic_load_deref)
            load_def = nir_load_deref(b, def_arr_instr);
         else
            load_def = interp_deref(b, intrin, def_arr_instr);

         nir_def *swz = nir_vector_extract(b, load_def, swizzle);
         nir_def_rewrite_uses(&intrin->def, swz);
      }
   }

   nir_deref_path_finish(&path);
}

static bool
replace_with_derefs_to_vec4(nir_builder *b, nir_intrinsic_instr *intr,
                            void *cb_data)
{
   struct lower_distance_state *state =
      (struct lower_distance_state *) cb_data;
   nir_variable_mode mask = nir_var_shader_in | nir_var_shader_out;

   /* Copy deref lowering is expected to happen before we get here */
   assert(intr->intrinsic != nir_intrinsic_copy_deref);
   assert(intr->intrinsic != nir_intrinsic_interp_deref_at_vertex);

   if (intr->intrinsic != nir_intrinsic_load_deref &&
       intr->intrinsic != nir_intrinsic_store_deref &&
       intr->intrinsic != nir_intrinsic_interp_deref_at_centroid &&
       intr->intrinsic != nir_intrinsic_interp_deref_at_sample &&
       intr->intrinsic != nir_intrinsic_interp_deref_at_offset)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
   if (!nir_deref_mode_is_one_of(deref, mask))
      return false;

   nir_variable *var = nir_deref_instr_get_variable(deref);

   /* The var has already been lowered to a temp so the derefs have already
    * been replaced. We can end up here when a shader has both clip and cull
    * arrays.
    */
   if (var->data.mode != nir_var_shader_in &&
       var->data.mode != nir_var_shader_out)
      return false;

   if (var->data.mode == nir_var_shader_out &&
      var != state->old_distance_out_var)
      return false;

   if (var->data.mode == nir_var_shader_in &&
       var != state->old_distance_in_var)
      return false;

   nir_variable *new_var = var->data.mode == nir_var_shader_in ?
      state->new_distance_in_var : state->new_distance_out_var;

   lower_distance_deref(state, b, intr, deref, new_var);

   return true;
}

static void
lower_distance_to_vec4(nir_shader *shader, struct lower_distance_state *state)
{
   /* Replace declarations */
   nir_foreach_variable_with_modes_safe(var, shader,
                                        nir_var_shader_in | nir_var_shader_out) {
      replace_var_declaration(state, shader, var, state->in_name);
   }

   if (!state->old_distance_in_var && !state->old_distance_out_var)
      return;

   /* Replace derefs */
   nir_shader_intrinsics_pass(shader, replace_with_derefs_to_vec4,
                              nir_metadata_block_index |
                              nir_metadata_dominance, state);

   /* Mark now lowered vars as ordinary globals to be dead code eliminated.
    * Also clear the compact flag to avoid issues with validation.
    */
   if (state->old_distance_out_var) {
      state->old_distance_out_var->data.mode = nir_var_shader_temp;
      state->old_distance_out_var->data.compact = false;
   }

   if (state->old_distance_in_var) {
      state->old_distance_in_var->data.mode = nir_var_shader_temp;
      state->old_distance_in_var->data.compact = false;
   }
}

void
nir_lower_clip_cull_distance_to_vec4s(nir_shader *shader)
{
   int clip_size = 0;
   int cull_size = 0;

   nir_variable_mode mode = nir_var_shader_in | nir_var_shader_out;
   nir_foreach_variable_with_modes(var, shader, mode) {
      if ((var->data.mode == nir_var_shader_in &&
           shader->info.stage == MESA_SHADER_VERTEX) ||
          (var->data.mode == nir_var_shader_out &&
           shader->info.stage == MESA_SHADER_FRAGMENT) ||
          shader->info.stage == MESA_SHADER_COMPUTE)
         continue;


      if (var->data.location == VARYING_SLOT_CLIP_DIST0)
         clip_size = MAX2(clip_size, get_unwrapped_array_length(shader, var));

      if (var->data.location == VARYING_SLOT_CULL_DIST0)
         cull_size = MAX2(cull_size, get_unwrapped_array_length(shader, var));
   }

   if (clip_size == 0 && cull_size == 0)
      return;

   struct lower_distance_state state;
   state.old_distance_out_var = NULL;
   state.old_distance_in_var = NULL;
   state.new_distance_out_var = NULL;
   state.new_distance_in_var = NULL;
   state.shader_stage = shader->info.stage;
   state.in_name = "gl_ClipDistance";
   state.total_size = clip_size + cull_size;
   state.offset = 0;
   lower_distance_to_vec4(shader, &state);

   state.old_distance_out_var = NULL;
   state.old_distance_in_var = NULL;
   state.in_name ="gl_CullDistance";
   state.offset = clip_size;
   lower_distance_to_vec4(shader, &state);

   nir_fixup_deref_modes(shader);
}

static bool
combine_clip_cull(nir_shader *nir,
                  nir_variable_mode mode,
                  bool store_info)
{
   nir_variable *cull = NULL;
   nir_variable *clip = NULL;

   nir_foreach_variable_with_modes(var, nir, mode) {
      if (var->data.location == VARYING_SLOT_CLIP_DIST0)
         clip = var;

      if (var->data.location == VARYING_SLOT_CULL_DIST0)
         cull = var;
   }

   if (!cull && !clip) {
      /* If this is run after optimizations and the variables have been
       * eliminated, we should update the shader info, because no other
       * place does that.
       */
      if (store_info) {
         nir->info.clip_distance_array_size = 0;
         nir->info.cull_distance_array_size = 0;
      }
      return false;
   }

   if (!cull && clip) {
      /* The GLSL IR lowering pass must have converted these to vectors */
      if (!clip->data.compact)
         return false;

      /* If this pass has already run, don't repeat.  We would think that
       * the combined clip/cull distance array was clip-only and mess up.
       */
      if (clip->data.how_declared == nir_var_hidden)
         return false;
   }

   const unsigned clip_array_size = get_unwrapped_array_length(nir, clip);
   const unsigned cull_array_size = get_unwrapped_array_length(nir, cull);

   if (store_info) {
      nir->info.clip_distance_array_size = clip_array_size;
      nir->info.cull_distance_array_size = cull_array_size;
   }

   if (clip) {
      assert(clip->data.compact);
      clip->data.how_declared = nir_var_hidden;
   }

   if (cull) {
      assert(cull->data.compact);
      cull->data.how_declared = nir_var_hidden;
      cull->data.location = VARYING_SLOT_CLIP_DIST0 + clip_array_size / 4;
      cull->data.location_frac = clip_array_size % 4;
   }

   return true;
}

bool
nir_lower_clip_cull_distance_arrays(nir_shader *nir)
{
   bool progress = false;

   if (nir->info.stage <= MESA_SHADER_GEOMETRY ||
       nir->info.stage == MESA_SHADER_MESH)
      progress |= combine_clip_cull(nir, nir_var_shader_out, true);

   if (nir->info.stage > MESA_SHADER_VERTEX &&
       nir->info.stage <= MESA_SHADER_FRAGMENT) {
      progress |= combine_clip_cull(nir, nir_var_shader_in,
                                    nir->info.stage == MESA_SHADER_FRAGMENT);
   }

   nir_foreach_function_impl(impl, nir) {
      if (progress) {
         nir_metadata_preserve(impl,
                               nir_metadata_block_index |
                                  nir_metadata_dominance |
                                  nir_metadata_live_defs |
                                  nir_metadata_loop_analysis);
      } else {
         nir_metadata_preserve(impl, nir_metadata_all);
      }
   }

   return progress;
}
