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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "nir.h"
#include "nir_builder.h"

#define MAX_COLORS 2 /* VARYING_SLOT_COL0/COL1 */

typedef struct {
   nir_builder b;
   nir_shader *shader;
   bool face_sysval;
   struct {
      nir_variable *front; /* COLn */
      nir_variable *back;  /* BFCn */
   } colors[MAX_COLORS];
   int colors_count;
} lower_2side_state;

/* Lowering pass for fragment shaders to emulated two-sided-color.  For
 * each COLOR input, a corresponding BCOLOR input is created, and bcsel
 * instruction used to select front or back color based on FACE.
 */

static nir_variable *
create_input(nir_shader *shader, gl_varying_slot slot,
             enum glsl_interp_mode interpolation)
{
   nir_variable *var = nir_create_variable_with_location(shader, nir_var_shader_in,
                                                         slot, glsl_vec4_type());

   var->data.index = 0;
   var->data.interpolation = interpolation;

   return var;
}

static nir_def *
load_input(nir_builder *b, nir_variable *in)
{
   return nir_load_input(b, 4, 32, nir_imm_int(b, 0),
                         .base = in->data.driver_location);
}

static int
setup_inputs(lower_2side_state *state)
{
   /* find color inputs: */
   nir_foreach_shader_in_variable(var, state->shader) {
      switch (var->data.location) {
      case VARYING_SLOT_COL0:
      case VARYING_SLOT_COL1:
         assert(state->colors_count < ARRAY_SIZE(state->colors));
         state->colors[state->colors_count].front = var;
         state->colors_count++;
         break;
      }
   }

   /* if we don't have any color inputs, nothing to do: */
   if (state->colors_count == 0)
      return -1;

   /* add required back-face color inputs: */
   for (int i = 0; i < state->colors_count; i++) {
      gl_varying_slot slot;

      if (state->colors[i].front->data.location == VARYING_SLOT_COL0)
         slot = VARYING_SLOT_BFC0;
      else
         slot = VARYING_SLOT_BFC1;

      state->colors[i].back = create_input(
         state->shader, slot,
         state->colors[i].front->data.interpolation);
   }

   return 0;
}

static bool
nir_lower_two_sided_color_instr(nir_builder *b, nir_instr *instr, void *data)
{
   lower_2side_state *state = data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   int idx;
   if (intr->intrinsic == nir_intrinsic_load_input) {
      for (idx = 0; idx < state->colors_count; idx++) {
         unsigned drvloc =
            state->colors[idx].front->data.driver_location;
         if (nir_intrinsic_base(intr) == drvloc) {
            assert(nir_src_is_const(intr->src[0]));
            break;
         }
      }
   } else if (intr->intrinsic == nir_intrinsic_load_deref) {
      nir_variable *var = nir_intrinsic_get_var(intr, 0);
      if (var->data.mode != nir_var_shader_in)
         return false;

      for (idx = 0; idx < state->colors_count; idx++) {
         unsigned loc = state->colors[idx].front->data.location;
         if (var->data.location == loc)
            break;
      }
   } else
      return false;

   if (idx == state->colors_count)
      return false;

   /* replace load_input(COLn) with
    * bcsel(load_system_value(FACE), load_input(COLn), load_input(BFCn))
    */
   b->cursor = nir_before_instr(&intr->instr);
   /* gl_FrontFace is a boolean but the intrinsic constructor creates
    * 32-bit value by default.
    */
   nir_def *face;
   if (state->face_sysval)
      face = nir_load_front_face(b, 1);
   else {
      nir_variable *var = nir_get_variable_with_location(b->shader, nir_var_shader_in,
                                                         VARYING_SLOT_FACE, glsl_bool_type());
      var->data.interpolation = INTERP_MODE_FLAT;
      face = nir_load_var(b, var);
   }

   nir_def *front, *back;
   if (intr->intrinsic == nir_intrinsic_load_deref) {
      front = nir_load_var(b, state->colors[idx].front);
      back = nir_load_var(b, state->colors[idx].back);
   } else {
      front = load_input(b, state->colors[idx].front);
      back = load_input(b, state->colors[idx].back);
   }
   nir_def *color = nir_bcsel(b, face, front, back);

   nir_def_rewrite_uses(&intr->def, color);

   return true;
}

bool
nir_lower_two_sided_color(nir_shader *shader, bool face_sysval)
{
   lower_2side_state state = {
      .shader = shader,
      .face_sysval = face_sysval,
   };

   if (shader->info.stage != MESA_SHADER_FRAGMENT)
      return false;

   if (setup_inputs(&state) != 0)
      return false;

   return nir_shader_instructions_pass(shader,
                                       nir_lower_two_sided_color_instr,
                                       nir_metadata_block_index |
                                          nir_metadata_dominance,
                                       &state);
}
