/*
 * Copyright © 2023 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "nir_builder.h"
#include "nir_builtin_builder.h"
#include "st_nir.h"

static nir_def *
fog_result(nir_builder *b, nir_def *color, enum gl_fog_mode fog_mode, struct gl_program_parameter_list *paramList)
{
   nir_shader *s = b->shader;
   nir_variable *fogc_var =
      nir_create_variable_with_location(s, nir_var_shader_in, VARYING_SLOT_FOGC, glsl_float_type());
   nir_def *fogc = nir_load_var(b, fogc_var);
   s->info.inputs_read |= VARYING_BIT_FOGC;

   static const gl_state_index16 fog_params_tokens[STATE_LENGTH] = {STATE_FOG_PARAMS_OPTIMIZED};
   static const gl_state_index16 fog_color_tokens[STATE_LENGTH] = {STATE_FOG_COLOR};

   nir_variable *fog_params_var = st_nir_state_variable_create(s, glsl_vec4_type(), fog_params_tokens);
   fog_params_var->data.driver_location = _mesa_add_state_reference(paramList, fog_params_tokens);
   nir_def *params = nir_load_var(b, fog_params_var);

   nir_variable *fog_color_var = st_nir_state_variable_create(s, glsl_vec4_type(), fog_color_tokens);
   fog_color_var->data.driver_location = _mesa_add_state_reference(paramList, fog_color_tokens);
   nir_def *fog_color = nir_load_var(b, fog_color_var);

   /* compute the 1 component fog factor f */
   nir_def *f = NULL;
   switch (fog_mode) {
   case FOG_LINEAR:
      /* f = (end - z) / (end - start)
       *
       * gl_MesaFogParamsOptimized gives us (-1 / (end - start)) and
       * (end / (end - start)) so we can generate a single MAD.
       */
      f = nir_fmad(b, fogc,
                   nir_channel(b, params, 0),
                   nir_channel(b, params, 1));
      break;
  case FOG_EXP:
      /* f = e^(-(density * fogcoord))
       *
       * gl_MesaFogParamsOptimized gives us density/ln(2) so we can
       * use EXP2 which is generally the native instruction without
       * having to do any further math on the fog density uniform.
       */
      f = nir_fmul(b, fogc, nir_channel(b, params, 2));
      f = nir_fexp2(b, nir_fneg(b, f));
      break;
  case FOG_EXP2:
      /* f = e^(-(density * fogcoord)^2)
       *
       * gl_MesaFogParamsOptimized gives us density/sqrt(ln(2)) so we
       * can do this like FOG_EXP but with a squaring after the
       * multiply by density.
       */
      f = nir_fmul(b, fogc, nir_channel(b, params, 3));
      f = nir_fmul(b, f, f);
      f = nir_fexp2(b, nir_fneg(b, f));
      break;
   default:
      unreachable("unsupported fog mode");
   }
   f = nir_fsat(b, f);

   /* Not using flrp because we may end up lowering fog after driver lowering
    * that meant to remove all lrps.
    */
   return nir_fmad(b, color, f, nir_fmul(b, fog_color, nir_fsub_imm(b, 1.0, f)));
}

struct lower_fog_state {
   enum gl_fog_mode fog_mode;
   struct gl_program_parameter_list *paramList;
};

static bool
st_nir_lower_fog_instr(nir_builder *b, nir_instr *instr, void *_state)
{
   const struct lower_fog_state *state = _state;

   if (instr->type != nir_instr_type_intrinsic)
      return false;
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   int loc = nir_intrinsic_io_semantics(intr).location;
   if (loc != FRAG_RESULT_COLOR && loc != FRAG_RESULT_DATA0)
      return false;

   b->cursor = nir_before_instr(instr);

   nir_def *color = intr->src[0].ssa;
   color = nir_resize_vector(b, color, 4);

   nir_def *fog = fog_result(b, color, state->fog_mode, state->paramList);

   /* retain the non-fog-blended alpha value for color */
   color = nir_vector_insert_imm(b, fog, nir_channel(b, color, 3), 3);

   nir_src_rewrite(&intr->src[0],
                   nir_resize_vector(b, color, intr->num_components));

   return true;
}

bool
st_nir_lower_fog(nir_shader *s, enum gl_fog_mode fog_mode, struct gl_program_parameter_list *paramList)
{
   if (s->info.io_lowered) {
      struct lower_fog_state state = {
         .fog_mode = fog_mode,
         .paramList = paramList,
      };
      nir_shader_instructions_pass(s, st_nir_lower_fog_instr,
                                   nir_metadata_block_index |
                                   nir_metadata_dominance,
                                   &state);
   } else {
      nir_variable *color_var = nir_find_variable_with_location(s, nir_var_shader_out, FRAG_RESULT_COLOR);
      if (!color_var) {
         color_var = nir_find_variable_with_location(s, nir_var_shader_out, FRAG_RESULT_DATA0);
         /* Fog result would be undefined if we had no output color (in ARB_fragment_program) */
         if (!color_var)
            return false;
      }

      nir_function_impl *impl = nir_shader_get_entrypoint(s);
      nir_builder b = nir_builder_at(nir_after_impl(impl));

      /* Note: while ARB_fragment_program plus ARB_draw_buffers allows an array
       * of result colors, prog_to_nir generates separate vars per slot so we
       * don't have to handle that.  Fog only applies to the first color result.
       */
      assert(!glsl_type_is_array(color_var->type));

      nir_def *color = nir_load_var(&b, color_var);
      color = fog_result(&b, color, fog_mode, paramList);
      nir_store_var(&b, color_var, color, 0x7);

      nir_metadata_preserve(b.impl, nir_metadata_block_index |
                                    nir_metadata_dominance);   }

   return true;
}
