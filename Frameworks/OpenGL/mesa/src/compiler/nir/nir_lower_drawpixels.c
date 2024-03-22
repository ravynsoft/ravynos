/*
 * Copyright © 2015 Red Hat
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "nir.h"
#include "nir_builder.h"

/* Lower glDrawPixels().
 *
 * This is based on the logic in st_get_drawpix_shader() in TGSI compiler.
 *
 * Run before nir_lower_io.
 */

typedef struct {
   const nir_lower_drawpixels_options *options;
   nir_shader *shader;
   nir_variable *texcoord, *texcoord_const, *scale, *bias, *tex, *pixelmap;
} lower_drawpixels_state;

static nir_def *
get_texcoord(nir_builder *b, lower_drawpixels_state *state)
{
   if (state->texcoord == NULL) {
      state->texcoord = nir_get_variable_with_location(state->shader, nir_var_shader_in,
                                                       VARYING_SLOT_TEX0, glsl_vec4_type());
   }
   return nir_load_var(b, state->texcoord);
}

static nir_def *
get_scale(nir_builder *b, lower_drawpixels_state *state)
{
   if (state->scale == NULL) {
      state->scale = nir_state_variable_create(state->shader, glsl_vec4_type(), "gl_PTscale",
                                               state->options->scale_state_tokens);
   }
   return nir_load_var(b, state->scale);
}

static nir_def *
get_bias(nir_builder *b, lower_drawpixels_state *state)
{
   if (state->bias == NULL) {
      state->bias = nir_state_variable_create(state->shader, glsl_vec4_type(), "gl_PTbias",
                                              state->options->bias_state_tokens);
   }
   return nir_load_var(b, state->bias);
}

static nir_def *
get_texcoord_const(nir_builder *b, lower_drawpixels_state *state)
{
   if (state->texcoord_const == NULL) {
      state->texcoord_const = nir_state_variable_create(state->shader, glsl_vec4_type(),
                                                        "gl_MultiTexCoord0",
                                                        state->options->texcoord_state_tokens);
   }
   return nir_load_var(b, state->texcoord_const);
}

static bool
lower_color(nir_builder *b, lower_drawpixels_state *state, nir_intrinsic_instr *intr)
{
   nir_def *texcoord;
   nir_tex_instr *tex;
   nir_def *def;

   b->cursor = nir_before_instr(&intr->instr);

   texcoord = get_texcoord(b, state);

   const struct glsl_type *sampler2D =
      glsl_sampler_type(GLSL_SAMPLER_DIM_2D, false, false, GLSL_TYPE_FLOAT);

   if (!state->tex) {
      state->tex =
         nir_variable_create(b->shader, nir_var_uniform, sampler2D, "drawpix");
      state->tex->data.binding = state->options->drawpix_sampler;
      state->tex->data.explicit_binding = true;
      state->tex->data.how_declared = nir_var_hidden;
   }

   nir_deref_instr *tex_deref = nir_build_deref_var(b, state->tex);

   /* replace load_var(gl_Color) w/ texture sample:
    *   TEX def, texcoord, drawpix_sampler, 2D
    */
   tex = nir_tex_instr_create(state->shader, 3);
   tex->op = nir_texop_tex;
   tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
   tex->coord_components = 2;
   tex->dest_type = nir_type_float32;
   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_texture_deref,
                                     &tex_deref->def);
   tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_sampler_deref,
                                     &tex_deref->def);
   tex->src[2] =
      nir_tex_src_for_ssa(nir_tex_src_coord,
                          nir_trim_vector(b, texcoord, tex->coord_components));

   nir_def_init(&tex->instr, &tex->def, 4, 32);
   nir_builder_instr_insert(b, &tex->instr);
   def = &tex->def;

   /* Apply the scale and bias. */
   if (state->options->scale_and_bias) {
      /* MAD def, def, scale, bias; */
      def = nir_ffma(b, def, get_scale(b, state), get_bias(b, state));
   }

   if (state->options->pixel_maps) {
      if (!state->pixelmap) {
         state->pixelmap = nir_variable_create(b->shader, nir_var_uniform,
                                               sampler2D, "pixelmap");
         state->pixelmap->data.binding = state->options->pixelmap_sampler;
         state->pixelmap->data.explicit_binding = true;
         state->pixelmap->data.how_declared = nir_var_hidden;
      }

      nir_deref_instr *pixelmap_deref =
         nir_build_deref_var(b, state->pixelmap);

      /* do four pixel map look-ups with two TEX instructions: */
      nir_def *def_xy, *def_zw;

      /* TEX def.xy, def.xyyy, pixelmap_sampler, 2D; */
      tex = nir_tex_instr_create(state->shader, 3);
      tex->op = nir_texop_tex;
      tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
      tex->coord_components = 2;
      tex->sampler_index = state->options->pixelmap_sampler;
      tex->texture_index = state->options->pixelmap_sampler;
      tex->dest_type = nir_type_float32;
      tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_texture_deref,
                                        &pixelmap_deref->def);
      tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_sampler_deref,
                                        &pixelmap_deref->def);
      tex->src[2] = nir_tex_src_for_ssa(nir_tex_src_coord,
                                        nir_trim_vector(b, def, 2));

      nir_def_init(&tex->instr, &tex->def, 4, 32);
      nir_builder_instr_insert(b, &tex->instr);
      def_xy = &tex->def;

      /* TEX def.zw, def.zwww, pixelmap_sampler, 2D; */
      tex = nir_tex_instr_create(state->shader, 1);
      tex->op = nir_texop_tex;
      tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
      tex->coord_components = 2;
      tex->sampler_index = state->options->pixelmap_sampler;
      tex->dest_type = nir_type_float32;
      tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord,
                                        nir_channels(b, def, 0xc));

      nir_def_init(&tex->instr, &tex->def, 4, 32);
      nir_builder_instr_insert(b, &tex->instr);
      def_zw = &tex->def;

      /* def = vec4(def.xy, def.zw); */
      def = nir_vec4(b,
                     nir_channel(b, def_xy, 0),
                     nir_channel(b, def_xy, 1),
                     nir_channel(b, def_zw, 0),
                     nir_channel(b, def_zw, 1));
   }

   nir_def_rewrite_uses(&intr->def, def);
   return true;
}

static bool
lower_texcoord(nir_builder *b, lower_drawpixels_state *state, nir_intrinsic_instr *intr)
{
   b->cursor = nir_before_instr(&intr->instr);

   nir_def *texcoord_const = get_texcoord_const(b, state);
   nir_def_rewrite_uses(&intr->def, texcoord_const);
   return true;
}

static bool
lower_drawpixels_instr(nir_builder *b, nir_instr *instr, void *cb_data)
{
   lower_drawpixels_state *state = cb_data;
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_load_deref: {
      nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
      nir_variable *var = nir_deref_instr_get_variable(deref);

      if (var->data.location == VARYING_SLOT_COL0) {
         /* gl_Color should not have array/struct derefs: */
         assert(deref->deref_type == nir_deref_type_var);
         return lower_color(b, state, intr);
      } else if (var->data.location == VARYING_SLOT_TEX0) {
         /* gl_TexCoord should not have array/struct derefs: */
         assert(deref->deref_type == nir_deref_type_var);
         return lower_texcoord(b, state, intr);
      }
      break;
   }

   case nir_intrinsic_load_color0:
      return lower_color(b, state, intr);

   case nir_intrinsic_load_interpolated_input:
   case nir_intrinsic_load_input: {
      if (nir_intrinsic_io_semantics(intr).location == VARYING_SLOT_TEX0)
         return lower_texcoord(b, state, intr);
      break;
   }
   default:
      break;
   }

   return false;
}

void
nir_lower_drawpixels(nir_shader *shader,
                     const nir_lower_drawpixels_options *options)
{
   lower_drawpixels_state state = {
      .options = options,
      .shader = shader,
   };

   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   nir_shader_instructions_pass(shader, lower_drawpixels_instr,
                                nir_metadata_block_index |
                                   nir_metadata_dominance,
                                &state);
}
