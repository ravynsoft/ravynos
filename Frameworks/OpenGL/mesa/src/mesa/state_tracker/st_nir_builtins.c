/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "st_nir.h"
#include "st_program.h"

#include "compiler/nir/nir_builder.h"
#include "compiler/glsl/gl_nir.h"
#include "compiler/glsl/gl_nir_linker.h"

void
st_nir_finish_builtin_nir(struct st_context *st, nir_shader *nir)
{
   struct pipe_screen *screen = st->screen;
   gl_shader_stage stage = nir->info.stage;

   MESA_TRACE_FUNC();

   nir->info.separate_shader = true;
   if (stage == MESA_SHADER_FRAGMENT)
      nir->info.fs.untyped_color_outputs = true;

   NIR_PASS_V(nir, nir_lower_global_vars_to_local);
   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);
   NIR_PASS_V(nir, nir_lower_system_values);
   NIR_PASS_V(nir, nir_lower_compute_system_values, NULL);

   if (nir->options->lower_to_scalar) {
      nir_variable_mode mask =
          (stage > MESA_SHADER_VERTEX ? nir_var_shader_in : 0) |
          (stage < MESA_SHADER_FRAGMENT ? nir_var_shader_out : 0);

      NIR_PASS_V(nir, nir_lower_io_to_scalar_early, mask);
   }

   if (st->lower_rect_tex) {
      const struct nir_lower_tex_options opts = { .lower_rect = true, };
      NIR_PASS_V(nir, nir_lower_tex, &opts);
   }

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   st_nir_assign_vs_in_locations(nir);
   st_nir_assign_varying_locations(st, nir);

   st_nir_lower_samplers(screen, nir, NULL, NULL);
   st_nir_lower_uniforms(st, nir);
   if (!screen->get_param(screen, PIPE_CAP_NIR_IMAGES_AS_DEREF))
      NIR_PASS_V(nir, gl_nir_lower_images, false);

   if (screen->finalize_nir) {
      char *msg = screen->finalize_nir(screen, nir);
      free(msg);
   } else {
      gl_nir_opts(nir);
   }
}

struct pipe_shader_state *
st_nir_finish_builtin_shader(struct st_context *st,
                             nir_shader *nir)
{
   st_nir_finish_builtin_nir(st, nir);

   struct pipe_shader_state state = {
      .type = PIPE_SHADER_IR_NIR,
      .ir.nir = nir,
   };

   return st_create_nir_shader(st, &state);
}

/**
 * Make a simple shader that copies inputs to corresponding outputs.
 */
struct pipe_shader_state *
st_nir_make_passthrough_shader(struct st_context *st,
                               const char *shader_name,
                               gl_shader_stage stage,
                               unsigned num_vars,
                               const unsigned *input_locations,
                               const gl_varying_slot *output_locations,
                               unsigned *interpolation_modes,
                               unsigned sysval_mask)
{
   const struct glsl_type *vec4 = glsl_vec4_type();
   const nir_shader_compiler_options *options =
      st_get_nir_compiler_options(st, stage);

   nir_builder b = nir_builder_init_simple_shader(stage, options,
                                                  "%s", shader_name);

   for (unsigned i = 0; i < num_vars; i++) {
      nir_variable *in;
      if (sysval_mask & (1 << i)) {
         in = nir_create_variable_with_location(b.shader, nir_var_system_value,
                                                input_locations[i],
                                  glsl_int_type());
      } else {
         in = nir_create_variable_with_location(b.shader, nir_var_shader_in,
                                                input_locations[i], vec4);
      }
      if (interpolation_modes)
         in->data.interpolation = interpolation_modes[i];

      nir_variable *out =
         nir_create_variable_with_location(b.shader, nir_var_shader_out,
                                           output_locations[i], in->type);
      out->data.interpolation = in->data.interpolation;

      nir_copy_var(&b, out, in);
   }

   return st_nir_finish_builtin_shader(st, b.shader);
}

/**
 * Make a simple shader that reads color value from a constant buffer
 * and uses it to clear all color buffers.
 */
struct pipe_shader_state *
st_nir_make_clearcolor_shader(struct st_context *st)
{
   const nir_shader_compiler_options *options =
      st_get_nir_compiler_options(st, MESA_SHADER_FRAGMENT);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT, options,
                                                  "clear color FS");
   b.shader->info.num_ubos = 1;
   b.shader->num_outputs = 1;
   b.shader->num_uniforms = 1;

   /* Read clear color from constant buffer */
   nir_def *clear_color = nir_load_uniform(&b, 4, 32, nir_imm_int(&b,0),
                                               .range = 16,
                                               .dest_type = nir_type_float32);

   nir_variable *color_out = nir_create_variable_with_location(b.shader, nir_var_shader_out,
                                                               FRAG_RESULT_COLOR, glsl_vec4_type());

   /* Write out the color */
   nir_store_var(&b, color_out, clear_color, 0xf);

   return st_nir_finish_builtin_shader(st, b.shader);
}
