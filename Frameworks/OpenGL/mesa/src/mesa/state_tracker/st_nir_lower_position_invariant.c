/*
 * Copyright © 2023 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "nir_builder.h"
#include "nir_builtin_builder.h"
#include "st_nir.h"

/**
 * Emits the implicit "gl_Position = gl_ModelViewProjection * gl_Vertex" for
 * ARB_vertex_program's ARB_position_invariant option, which must match the
 * behavior of the fixed function vertex shader.
 *
 * The "aos" flag is
 * ctx->Const.ShaderCompilerOptions[MESA_SHADER_VERTEX].OptimizeForAOS, used by
 * both FF VS and ARB_vp.
 */
bool
st_nir_lower_position_invariant(struct nir_shader *s, bool aos,
                                struct gl_program_parameter_list *paramList)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(s);
   nir_builder b = nir_builder_at(nir_before_impl(impl));

   nir_def *mvp[4];
   for (int i = 0; i < 4; i++) {
      gl_state_index16 tokens[STATE_LENGTH] = {
          aos ? STATE_MVP_MATRIX : STATE_MVP_MATRIX_TRANSPOSE, 0, i, i};
      nir_variable *var = st_nir_state_variable_create(s, glsl_vec4_type(), tokens);
      _mesa_add_state_reference(paramList, tokens);
      mvp[i] = nir_load_var(&b, var);
   }

   nir_def *result;
   nir_def *in_pos = nir_load_var(&b, nir_get_variable_with_location(s, nir_var_shader_in,
                                                                         VERT_ATTRIB_POS, glsl_vec4_type()));
   s->info.inputs_read |= VERT_BIT_POS;
   if (aos) {
      nir_def *chans[4];
      for (int i = 0; i < 4; i++)
         chans[i] = nir_fdot4(&b, mvp[i], in_pos);
      result = nir_vec4(&b, chans[0], chans[1], chans[2], chans[3]);
   } else {
      result = nir_fmul(&b, mvp[0], nir_channel(&b, in_pos, 0));
      for (int i = 1; i < 4; i++)
         result = nir_fmad(&b, mvp[i], nir_channel(&b, in_pos, i), result);
   }
   nir_store_var(&b, nir_get_variable_with_location(s, nir_var_shader_out,
                                                    VARYING_SLOT_POS, glsl_vec4_type()), result, 0xf);
   s->info.outputs_written |= VARYING_BIT_POS;

   nir_metadata_preserve(b.impl, nir_metadata_block_index |
                                 nir_metadata_dominance);

   return true;
}
