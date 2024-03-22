/*
 * Copyright © Microsoft Corporation
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

#include "d3d12_nir_passes.h"
#include "d3d12_compiler.h"
#include "nir_builder.h"
#include "nir_builtin_builder.h"
#include "nir_deref.h"
#include "nir_format_convert.h"
#include "program/prog_instruction.h"
#include "dxil_nir.h"

/**
 * Lower Y Flip:
 *
 * We can't do a Y flip simply by negating the viewport height,
 * so we need to lower the flip into the NIR shader.
 */

nir_def *
d3d12_get_state_var(nir_builder *b,
                    enum d3d12_state_var var_enum,
                    const char *var_name,
                    const struct glsl_type *var_type,
                    nir_variable **out_var)
{
   const gl_state_index16 tokens[STATE_LENGTH] = { STATE_INTERNAL_DRIVER, var_enum };
   if (*out_var == NULL) {
      nir_variable *var = nir_state_variable_create(b->shader, var_type,
                                                    var_name, tokens);
      var->data.how_declared = nir_var_hidden;
      *out_var = var;
   }
   return nir_load_var(b, *out_var);
}

static void
lower_pos_write(nir_builder *b, struct nir_instr *instr, nir_variable **flip)
{
   if (instr->type != nir_instr_type_intrinsic)
      return;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_store_deref)
      return;

   nir_variable *var = nir_intrinsic_get_var(intr, 0);
   if (var->data.mode != nir_var_shader_out ||
       var->data.location != VARYING_SLOT_POS)
      return;

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *pos = intr->src[1].ssa;
   nir_def *flip_y = d3d12_get_state_var(b, D3D12_STATE_VAR_Y_FLIP, "d3d12_FlipY",
                                             glsl_float_type(), flip);
   nir_def *def = nir_vec4(b,
                               nir_channel(b, pos, 0),
                               nir_fmul(b, nir_channel(b, pos, 1), flip_y),
                               nir_channel(b, pos, 2),
                               nir_channel(b, pos, 3));
   nir_src_rewrite(intr->src + 1, def);
}

void
d3d12_lower_yflip(nir_shader *nir)
{
   nir_variable *flip = NULL;

   if (nir->info.stage != MESA_SHADER_VERTEX &&
       nir->info.stage != MESA_SHADER_TESS_EVAL &&
       nir->info.stage != MESA_SHADER_GEOMETRY)
      return;

   nir_foreach_function_impl(impl, nir) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            lower_pos_write(&b, instr, &flip);
         }
      }

      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   }
}

static void
lower_pos_read(nir_builder *b, struct nir_instr *instr,
               nir_variable **depth_transform_var)
{
   if (instr->type != nir_instr_type_intrinsic)
      return;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_load_deref)
      return;

   nir_variable *var = nir_intrinsic_get_var(intr, 0);
   if (var->data.mode != nir_var_shader_in ||
       var->data.location != VARYING_SLOT_POS)
      return;

   b->cursor = nir_after_instr(instr);

   nir_def *pos = nir_instr_def(instr);
   nir_def *depth = nir_channel(b, pos, 2);

   assert(depth_transform_var);
   nir_def *depth_transform = d3d12_get_state_var(b, D3D12_STATE_VAR_DEPTH_TRANSFORM,
                                                      "d3d12_DepthTransform",
                                                      glsl_vec_type(2),
                                                      depth_transform_var);
   depth = nir_fmad(b, depth, nir_channel(b, depth_transform, 0),
                              nir_channel(b, depth_transform, 1));

   pos = nir_vector_insert_imm(b, pos, depth, 2);

   nir_def_rewrite_uses_after(&intr->def, pos,
                                  pos->parent_instr);
}

void
d3d12_lower_depth_range(nir_shader *nir)
{
   assert(nir->info.stage == MESA_SHADER_FRAGMENT);
   nir_variable *depth_transform = NULL;
   nir_foreach_function_impl(impl, nir) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            lower_pos_read(&b, instr, &depth_transform);
         }
      }

      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   }
}

struct compute_state_vars {
   nir_variable *num_workgroups;
};

static bool
lower_compute_state_vars(nir_builder *b, nir_instr *instr, void *_state)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   b->cursor = nir_after_instr(instr);
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   struct compute_state_vars *vars = _state;
   nir_def *result = NULL;
   switch (intr->intrinsic) {
   case nir_intrinsic_load_num_workgroups:
      result = d3d12_get_state_var(b, D3D12_STATE_VAR_NUM_WORKGROUPS, "d3d12_NumWorkgroups",
         glsl_vec_type(3), &vars->num_workgroups);
      break;
   default:
      return false;
   }

   nir_def_rewrite_uses(&intr->def, result);
   nir_instr_remove(instr);
   return true;
}

bool
d3d12_lower_compute_state_vars(nir_shader *nir)
{
   assert(nir->info.stage == MESA_SHADER_COMPUTE);
   struct compute_state_vars vars = { 0 };
   return nir_shader_instructions_pass(nir, lower_compute_state_vars,
      nir_metadata_block_index | nir_metadata_dominance, &vars);
}

static bool
is_color_output(nir_variable *var)
{
   return (var->data.mode == nir_var_shader_out &&
           (var->data.location == FRAG_RESULT_COLOR ||
            var->data.location >= FRAG_RESULT_DATA0));
}

static void
lower_uint_color_write(nir_builder *b, struct nir_instr *instr, bool is_signed)
{
   const unsigned NUM_BITS = 8;
   const unsigned bits[4] = { NUM_BITS, NUM_BITS, NUM_BITS, NUM_BITS };

   if (instr->type != nir_instr_type_intrinsic)
      return;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_store_deref)
      return;

   nir_variable *var = nir_intrinsic_get_var(intr, 0);
   if (!is_color_output(var))
      return;

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *col = intr->src[1].ssa;
   nir_def *def = is_signed ? nir_format_float_to_snorm(b, col, bits) :
                                  nir_format_float_to_unorm(b, col, bits);
   if (is_signed)
      def = nir_bcsel(b, nir_ilt_imm(b, def, 0),
                      nir_iadd_imm(b, def, 1ull << NUM_BITS),
                      def);
   nir_src_rewrite(intr->src + 1, def);
}

void
d3d12_lower_uint_cast(nir_shader *nir, bool is_signed)
{
   if (nir->info.stage != MESA_SHADER_FRAGMENT)
      return;

   nir_foreach_function_impl(impl, nir) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            lower_uint_color_write(&b, instr, is_signed);
         }
      }

      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   }
}

static bool
lower_load_draw_params(nir_builder *b, nir_intrinsic_instr *intr,
                       void *draw_params)
{
   if (intr->intrinsic != nir_intrinsic_load_first_vertex &&
       intr->intrinsic != nir_intrinsic_load_base_instance &&
       intr->intrinsic != nir_intrinsic_load_draw_id &&
       intr->intrinsic != nir_intrinsic_load_is_indexed_draw)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *load = d3d12_get_state_var(b, D3D12_STATE_VAR_DRAW_PARAMS, "d3d12_DrawParams",
                                           glsl_uvec4_type(), draw_params);
   unsigned channel = intr->intrinsic == nir_intrinsic_load_first_vertex ? 0 :
      intr->intrinsic == nir_intrinsic_load_base_instance ? 1 :
      intr->intrinsic == nir_intrinsic_load_draw_id ? 2 : 3;
   nir_def_rewrite_uses(&intr->def, nir_channel(b, load, channel));
   nir_instr_remove(&intr->instr);

   return true;
}

bool
d3d12_lower_load_draw_params(struct nir_shader *nir)
{
   nir_variable *draw_params = NULL;
   if (nir->info.stage != MESA_SHADER_VERTEX)
      return false;

   return nir_shader_intrinsics_pass(nir, lower_load_draw_params,
                                     nir_metadata_block_index | nir_metadata_dominance,
                                     &draw_params);
}

static bool
lower_load_patch_vertices_in(nir_builder *b, nir_intrinsic_instr *intr,
                             void *_state)
{
   if (intr->intrinsic != nir_intrinsic_load_patch_vertices_in)
      return false;

   b->cursor = nir_before_instr(&intr->instr);
   nir_def *load = b->shader->info.stage == MESA_SHADER_TESS_CTRL ?
      d3d12_get_state_var(b, D3D12_STATE_VAR_PATCH_VERTICES_IN, "d3d12_FirstVertex", glsl_uint_type(), _state) :
      nir_imm_int(b, b->shader->info.tess.tcs_vertices_out);
   nir_def_rewrite_uses(&intr->def, load);
   nir_instr_remove(&intr->instr);
   return true;
}

bool
d3d12_lower_load_patch_vertices_in(struct nir_shader *nir)
{
   nir_variable *var = NULL;

   if (nir->info.stage != MESA_SHADER_TESS_CTRL &&
       nir->info.stage != MESA_SHADER_TESS_EVAL)
      return false;

   return nir_shader_intrinsics_pass(nir, lower_load_patch_vertices_in,
                                     nir_metadata_block_index | nir_metadata_dominance,
                                     &var);
}

struct invert_depth_state
{
   unsigned viewport_mask;
   bool clip_halfz;
   nir_def *viewport_index;
   nir_instr *store_pos_instr;
};

static void
invert_depth_impl(nir_builder *b, struct invert_depth_state *state)
{
   assert(state->store_pos_instr);

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(state->store_pos_instr);
   if (state->viewport_index) {
      /* Cursor is assigned before calling. Make sure that storing pos comes
       * after computing the viewport.
       */
      nir_instr_move(b->cursor, &intr->instr);
   }

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *pos = intr->src[1].ssa;

   if (state->viewport_index) {
      nir_push_if(b, nir_test_mask(b, nir_ishl(b, nir_imm_int(b, 1), state->viewport_index), state->viewport_mask));
   }
   nir_def *old_depth = nir_channel(b, pos, 2);
   nir_def *new_depth = nir_fneg(b, old_depth);
   if (state->clip_halfz)
      new_depth = nir_fadd_imm(b, new_depth, 1.0);
   nir_def *def = nir_vec4(b,
                               nir_channel(b, pos, 0),
                               nir_channel(b, pos, 1),
                               new_depth,
                               nir_channel(b, pos, 3));
   if (state->viewport_index) {
      nir_pop_if(b, NULL);
      def = nir_if_phi(b, def, pos);
   }
   nir_src_rewrite(intr->src + 1, def);

   state->viewport_index = NULL;
   state->store_pos_instr = NULL;
}

static void
invert_depth_instr(nir_builder *b, struct nir_instr *instr, struct invert_depth_state *state)
{
   if (instr->type != nir_instr_type_intrinsic)
      return;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic == nir_intrinsic_store_deref) {
      nir_variable *var = nir_intrinsic_get_var(intr, 0);
      if (var->data.mode != nir_var_shader_out)
         return;

      if (var->data.location == VARYING_SLOT_VIEWPORT)
         state->viewport_index = intr->src[1].ssa;
      if (var->data.location == VARYING_SLOT_POS)
         state->store_pos_instr = instr;
   } else if (intr->intrinsic == nir_intrinsic_emit_vertex) {
      b->cursor = nir_before_instr(instr);
      invert_depth_impl(b, state);
   }
}

/* In OpenGL the windows space depth value z_w is evaluated according to "s * z_d + b"
 * with  "s = (far - near) / 2" (depth clip:minus_one_to_one) [OpenGL 3.3, 2.13.1].
 * When we switch the far and near value to satisfy DirectX requirements we have
 * to compensate by inverting "z_d' = -z_d" with this lowering pass.
 * When depth clip is set zero_to_one, we compensate with "z_d' = 1.0f - z_d" instead.
 */
void
d3d12_nir_invert_depth(nir_shader *shader, unsigned viewport_mask, bool clip_halfz)
{
   if (shader->info.stage != MESA_SHADER_VERTEX &&
       shader->info.stage != MESA_SHADER_TESS_EVAL &&
       shader->info.stage != MESA_SHADER_GEOMETRY)
      return;

   struct invert_depth_state state = { viewport_mask, clip_halfz };
   nir_foreach_function_impl(impl, shader) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            invert_depth_instr(&b, instr, &state);
         }
      }

      if (state.store_pos_instr) {
         b.cursor = nir_after_block(impl->end_block);
         invert_depth_impl(&b, &state);
      }

      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   }
}


/**
 * Lower State Vars:
 *
 * All uniforms related to internal D3D12 variables are
 * condensed into a UBO that is appended at the end of the
 * current ones.
 */

static unsigned
get_state_var_offset(struct d3d12_shader *shader, enum d3d12_state_var var)
{
   for (unsigned i = 0; i < shader->num_state_vars; ++i) {
      if (shader->state_vars[i].var == var)
         return shader->state_vars[i].offset;
   }

   unsigned offset = shader->state_vars_size;
   shader->state_vars[shader->num_state_vars].offset = offset;
   shader->state_vars[shader->num_state_vars].var = var;
   shader->state_vars_size += 4; /* Use 4-words slots no matter the variable size */
   shader->num_state_vars++;

   return offset;
}

static bool
lower_instr(nir_intrinsic_instr *instr, nir_builder *b,
            struct d3d12_shader *shader, unsigned binding)
{
   nir_variable *variable = NULL;
   nir_deref_instr *deref = NULL;

   b->cursor = nir_before_instr(&instr->instr);

   if (instr->intrinsic == nir_intrinsic_load_uniform) {
      nir_foreach_variable_with_modes(var, b->shader, nir_var_uniform) {
         if (var->data.driver_location == nir_intrinsic_base(instr)) {
            variable = var;
            break;
         }
      }
   } else if (instr->intrinsic == nir_intrinsic_load_deref) {
      deref = nir_src_as_deref(instr->src[0]);
      variable = nir_intrinsic_get_var(instr, 0);
   }

   if (variable == NULL ||
       variable->num_state_slots != 1 ||
       variable->state_slots[0].tokens[0] != STATE_INTERNAL_DRIVER)
      return false;

   enum d3d12_state_var var = variable->state_slots[0].tokens[1];
   nir_def *ubo_idx = nir_imm_int(b, binding);
   nir_def *ubo_offset =  nir_imm_int(b, get_state_var_offset(shader, var) * 4);
   nir_def *load =
      nir_load_ubo(b, instr->num_components, instr->def.bit_size,
                   ubo_idx, ubo_offset,
                   .align_mul = 16,
                   .align_offset = 0,
                   .range_base = 0,
                   .range = ~0,
                   );

   nir_def_rewrite_uses(&instr->def, load);

   /* Remove the old load_* instruction and any parent derefs */
   nir_instr_remove(&instr->instr);
   for (nir_deref_instr *d = deref; d; d = nir_deref_instr_parent(d)) {
      /* If anyone is using this deref, leave it alone */
      if (!list_is_empty(&d->def.uses))
         break;

      nir_instr_remove(&d->instr);
   }

   return true;
}

bool
d3d12_lower_state_vars(nir_shader *nir, struct d3d12_shader *shader)
{
   bool progress = false;

   /* The state var UBO is added after all the other UBOs if it already
    * exists it will be replaced by using the same binding.
    * In the event there are no other UBO's, use binding slot 1 to
    * be consistent with other non-default UBO's */
   unsigned binding = MAX2(nir->info.num_ubos, 1);

   nir_foreach_variable_with_modes_safe(var, nir, nir_var_uniform) {
      if (var->num_state_slots == 1 &&
          var->state_slots[0].tokens[0] == STATE_INTERNAL_DRIVER) {
         if (var->data.mode == nir_var_mem_ubo) {
            binding = var->data.binding;
         }
      }
   }

   nir_foreach_function_impl(impl, nir) {
      nir_builder builder = nir_builder_create(impl);
      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type == nir_instr_type_intrinsic)
               progress |= lower_instr(nir_instr_as_intrinsic(instr),
                                       &builder,
                                       shader,
                                       binding);
         }
      }

      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   }

   if (progress) {
      assert(shader->num_state_vars > 0);

      shader->state_vars_used = true;

      /* Remove state variables */
      nir_foreach_variable_with_modes_safe(var, nir, nir_var_uniform) {
         if (var->num_state_slots == 1 &&
             var->state_slots[0].tokens[0] == STATE_INTERNAL_DRIVER) {
            exec_node_remove(&var->node);
            nir->num_uniforms--;
         }
      }

      const gl_state_index16 tokens[STATE_LENGTH] = { STATE_INTERNAL_DRIVER };
      const struct glsl_type *type = glsl_array_type(glsl_vec4_type(),
                                                     shader->state_vars_size / 4, 0);
      nir_variable *ubo = nir_variable_create(nir, nir_var_mem_ubo, type,
                                                  "d3d12_state_vars");
      if (binding >= nir->info.num_ubos)
         nir->info.num_ubos = binding + 1;
      ubo->data.binding = binding;
      ubo->num_state_slots = 1;
      ubo->state_slots = ralloc_array(ubo, nir_state_slot, 1);
      memcpy(ubo->state_slots[0].tokens, tokens,
              sizeof(ubo->state_slots[0].tokens));

      struct glsl_struct_field field = {
          .type = type,
          .name = "data",
          .location = -1,
      };
      ubo->interface_type =
              glsl_interface_type(&field, 1, GLSL_INTERFACE_PACKING_STD430,
                                  false, "__d3d12_state_vars_interface");
   }

   return progress;
}

void
d3d12_add_missing_dual_src_target(struct nir_shader *s,
                                  unsigned missing_mask)
{
   assert(missing_mask != 0);
   nir_builder b;
   nir_function_impl *impl = nir_shader_get_entrypoint(s);
   b = nir_builder_at(nir_before_impl(impl));

   nir_def *zero = nir_imm_zero(&b, 4, 32);
   for (unsigned i = 0; i < 2; ++i) {

      if (!(missing_mask & (1u << i)))
         continue;

      const char *name = i == 0 ? "gl_FragData[0]" :
                                  "gl_SecondaryFragDataEXT[0]";
      nir_variable *out = nir_variable_create(s, nir_var_shader_out,
                                              glsl_vec4_type(), name);
      out->data.location = FRAG_RESULT_DATA0;
      out->data.driver_location = i;
      out->data.index = i;

      nir_store_var(&b, out, zero, 0xf);
   }
   nir_metadata_preserve(impl, nir_metadata_block_index |
                               nir_metadata_dominance);
}

void
d3d12_lower_primitive_id(nir_shader *shader)
{
   nir_builder b;
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   nir_def *primitive_id;
   b = nir_builder_create(impl);

   nir_variable *primitive_id_var = nir_variable_create(shader, nir_var_shader_out,
                                                        glsl_uint_type(), "primitive_id");
   primitive_id_var->data.location = VARYING_SLOT_PRIMITIVE_ID;
   primitive_id_var->data.interpolation = INTERP_MODE_FLAT;

   nir_foreach_block(block, impl) {
      b.cursor = nir_before_block(block);
      primitive_id = nir_load_primitive_id(&b);

      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic ||
             nir_instr_as_intrinsic(instr)->intrinsic != nir_intrinsic_emit_vertex)
            continue;

         b.cursor = nir_before_instr(instr);
         nir_store_var(&b, primitive_id_var, primitive_id, 0x1);
      }
   }

   nir_metadata_preserve(impl, nir_metadata_none);
}

static void
lower_triangle_strip_store(nir_builder *b, nir_intrinsic_instr *intr,
                           nir_variable *vertex_count_var,
                           nir_variable **varyings)
{
   /**
    * tmp_varying[slot][min(vertex_count, 2)] = src
    */
   nir_def *vertex_count = nir_load_var(b, vertex_count_var);
   nir_def *index = nir_imin(b, vertex_count, nir_imm_int(b, 2));
   nir_variable *var = nir_intrinsic_get_var(intr, 0);

   if (var->data.mode != nir_var_shader_out)
      return;

   nir_deref_instr *deref = nir_build_deref_array(b, nir_build_deref_var(b, varyings[var->data.location]), index);
   nir_def *value = intr->src[1].ssa;
   nir_store_deref(b, deref, value, 0xf);
   nir_instr_remove(&intr->instr);
}

static void
lower_triangle_strip_emit_vertex(nir_builder *b, nir_intrinsic_instr *intr,
                                 nir_variable *vertex_count_var,
                                 nir_variable **varyings,
                                 nir_variable **out_varyings)
{
   // TODO xfb + flat shading + last_pv
   /**
    * if (vertex_count >= 2) {
    *    for (i = 0; i < 3; i++) {
    *       foreach(slot)
    *          out[slot] = tmp_varying[slot][i];
    *       EmitVertex();
    *    }
    *    EndPrimitive();
    *    foreach(slot)
    *       tmp_varying[slot][vertex_count % 2] = tmp_varying[slot][2];
    * }
    * vertex_count++;
    */

   nir_def *two = nir_imm_int(b, 2);
   nir_def *vertex_count = nir_load_var(b, vertex_count_var);
   nir_def *count_cmp = nir_uge(b, vertex_count, two);
   nir_if *count_check = nir_push_if(b, count_cmp);

   for (int j = 0; j < 3; ++j) {
      for (int i = 0; i < VARYING_SLOT_MAX; ++i) {
         if (!varyings[i])
            continue;
         nir_copy_deref(b, nir_build_deref_var(b, out_varyings[i]),
                        nir_build_deref_array_imm(b, nir_build_deref_var(b, varyings[i]), j));
      }
      nir_emit_vertex(b, 0);
   }

   for (int i = 0; i < VARYING_SLOT_MAX; ++i) {
      if (!varyings[i])
         continue;
      nir_copy_deref(b, nir_build_deref_array(b, nir_build_deref_var(b, varyings[i]), nir_umod(b, vertex_count, two)),
                        nir_build_deref_array(b, nir_build_deref_var(b, varyings[i]), two));
   }

   nir_end_primitive(b, .stream_id = 0);

   nir_pop_if(b, count_check);

   vertex_count = nir_iadd_imm(b, vertex_count, 1);
   nir_store_var(b, vertex_count_var, vertex_count, 0x1);

   nir_instr_remove(&intr->instr);
}

static void
lower_triangle_strip_end_primitive(nir_builder *b, nir_intrinsic_instr *intr,
                                   nir_variable *vertex_count_var)
{
   /**
    * vertex_count = 0;
    */
   nir_store_var(b, vertex_count_var, nir_imm_int(b, 0), 0x1);
   nir_instr_remove(&intr->instr);
}

void
d3d12_lower_triangle_strip(nir_shader *shader)
{
   nir_builder b;
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   nir_variable *tmp_vars[VARYING_SLOT_MAX] = {0};
   nir_variable *out_vars[VARYING_SLOT_MAX] = {0};
   b = nir_builder_create(impl);

   shader->info.gs.vertices_out = (shader->info.gs.vertices_out - 2) * 3;

   nir_variable *vertex_count_var =
      nir_local_variable_create(impl, glsl_uint_type(), "vertex_count");

   nir_block *first = nir_start_block(impl);
   b.cursor = nir_before_block(first);
   nir_foreach_variable_with_modes(var, shader, nir_var_shader_out) {
      const struct glsl_type *type = glsl_array_type(var->type, 3, 0);
      tmp_vars[var->data.location] =  nir_local_variable_create(impl, type, "tmp_var");
      out_vars[var->data.location] = var;
   }
   nir_store_var(&b, vertex_count_var, nir_imm_int(&b, 0), 1);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         switch (intrin->intrinsic) {
         case nir_intrinsic_store_deref:
            b.cursor = nir_before_instr(instr);
            lower_triangle_strip_store(&b, intrin, vertex_count_var, tmp_vars);
            break;
         case nir_intrinsic_emit_vertex_with_counter:
         case nir_intrinsic_emit_vertex:
            b.cursor = nir_before_instr(instr);
            lower_triangle_strip_emit_vertex(&b, intrin, vertex_count_var,
                                             tmp_vars, out_vars);
            break;
         case nir_intrinsic_end_primitive:
         case nir_intrinsic_end_primitive_with_counter:
            b.cursor = nir_before_instr(instr);
            lower_triangle_strip_end_primitive(&b, intrin, vertex_count_var);
            break;
         default:
            break;
         }
      }
   }

   nir_metadata_preserve(impl, nir_metadata_none);
   NIR_PASS_V(shader, nir_lower_var_copies);
}

static bool
is_multisampling_instr(const nir_instr *instr, const void *_data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic == nir_intrinsic_store_output) {
      nir_io_semantics semantics = nir_intrinsic_io_semantics(intr);
      return semantics.location == FRAG_RESULT_SAMPLE_MASK;
   } else if (intr->intrinsic == nir_intrinsic_store_deref) {
      nir_variable *var = nir_intrinsic_get_var(intr, 0);
      return var->data.location == FRAG_RESULT_SAMPLE_MASK;
   } else if (intr->intrinsic == nir_intrinsic_load_sample_id ||
              intr->intrinsic == nir_intrinsic_load_sample_mask_in)
      return true;
   return false;
}

static nir_def *
lower_multisampling_instr(nir_builder *b, nir_instr *instr, void *_data)
{
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   switch (intr->intrinsic) {
   case nir_intrinsic_store_output:
   case nir_intrinsic_store_deref:
      return NIR_LOWER_INSTR_PROGRESS_REPLACE;
   case nir_intrinsic_load_sample_id:
      return nir_imm_int(b, 0);
   case nir_intrinsic_load_sample_mask_in:
      return nir_b2i32(b, nir_ine_imm(b, &intr->def, 0));
   default:
      unreachable("Invalid intrinsic");
   }
}

bool
d3d12_disable_multisampling(nir_shader *s)
{
   if (s->info.stage != MESA_SHADER_FRAGMENT)
      return false;
   bool progress = nir_shader_lower_instructions(s, is_multisampling_instr, lower_multisampling_instr, NULL);

   nir_foreach_variable_with_modes_safe(var, s, nir_var_shader_out) {
      if (var->data.location == FRAG_RESULT_SAMPLE_MASK) {
         exec_node_remove(&var->node);
         s->info.outputs_written &= ~(1ull << FRAG_RESULT_SAMPLE_MASK);
         progress = true;
      }
   }
   nir_foreach_variable_with_modes_safe(var, s, nir_var_system_value) {
      if (var->data.location == SYSTEM_VALUE_SAMPLE_MASK_IN ||
          var->data.location == SYSTEM_VALUE_SAMPLE_ID) {
         exec_node_remove(&var->node);
         progress = true;
      }
      var->data.sample = false;
   }
   BITSET_CLEAR(s->info.system_values_read, SYSTEM_VALUE_SAMPLE_ID);
   s->info.fs.uses_sample_qualifier = false;
   s->info.fs.uses_sample_shading = false;
   return progress;
}

struct var_split_subvar_state {
   nir_variable *var;
   uint8_t stream;
   uint8_t num_components;
};
struct var_split_var_state {
   unsigned num_subvars;
   struct var_split_subvar_state subvars[4];
};
struct var_split_state {
   struct var_split_var_state vars[2][VARYING_SLOT_MAX];
};

static bool
split_varying_accesses(nir_builder *b, nir_intrinsic_instr *intr,
                                 void *_state)
{
   if (intr->intrinsic != nir_intrinsic_store_deref &&
       intr->intrinsic != nir_intrinsic_load_deref)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
   if (!nir_deref_mode_is(deref, nir_var_shader_out) &&
       !nir_deref_mode_is(deref, nir_var_shader_in))
      return false;

   nir_variable *var = nir_deref_instr_get_variable(deref);
   if (!var)
      return false;

   uint32_t mode_index = deref->modes == nir_var_shader_out ? 0 : 1;

   struct var_split_state *state = _state;
   struct var_split_var_state *var_state = &state->vars[mode_index][var->data.location];
   if (var_state->num_subvars <= 1)
      return false;

   nir_deref_path path;
   nir_deref_path_init(&path, deref, b->shader);
   assert(path.path[0]->deref_type == nir_deref_type_var && path.path[0]->var == var);
   
   unsigned first_channel = 0;
   nir_def *loads[2];
   for (unsigned subvar = 0; subvar < var_state->num_subvars; ++subvar) {
      b->cursor = nir_after_instr(&path.path[0]->instr);
      nir_deref_instr *new_path = nir_build_deref_var(b, var_state->subvars[subvar].var);

      for (unsigned i = 1; path.path[i]; ++i) {
         b->cursor = nir_after_instr(&path.path[i]->instr);
         new_path = nir_build_deref_follower(b, new_path, path.path[i]);
      }

      b->cursor = nir_before_instr(&intr->instr);
      if (intr->intrinsic == nir_intrinsic_store_deref) {
         unsigned mask_num_channels = (1 << var_state->subvars[subvar].num_components) - 1;
         unsigned orig_write_mask = nir_intrinsic_write_mask(intr);
         nir_def *sub_value = nir_channels(b, intr->src[1].ssa, mask_num_channels << first_channel);

         first_channel += var_state->subvars[subvar].num_components;

         unsigned new_write_mask = (orig_write_mask >> first_channel) & mask_num_channels;
         nir_build_store_deref(b, &new_path->def, sub_value, new_write_mask, nir_intrinsic_access(intr));
      } else {
         /* The load path only handles splitting dvec3/dvec4 */
         assert(subvar == 0 || subvar == 1);
         assert(intr->def.num_components >= 3);
         loads[subvar] = nir_build_load_deref(b, var_state->subvars[subvar].num_components, intr->def.bit_size, &new_path->def);
      }
   }

   nir_deref_path_finish(&path);
   if (intr->intrinsic == nir_intrinsic_load_deref) {
      nir_def *result = nir_extract_bits(b, loads, 2, 0, intr->def.num_components, intr->def.bit_size);
      nir_def_rewrite_uses(&intr->def, result);
   }
   nir_instr_free_and_dce(&intr->instr);
   return true;
}

bool
d3d12_split_needed_varyings(nir_shader *s)
{
   struct var_split_state state;
   memset(&state, 0, sizeof(state));

   bool progress = false;
   nir_foreach_variable_with_modes_safe(var, s, nir_var_shader_out | nir_var_shader_in) {
      uint32_t mode_index = var->data.mode == nir_var_shader_out ? 0 : 1;
      struct var_split_var_state *var_state = &state.vars[mode_index][var->data.location];
      struct var_split_subvar_state *subvars = var_state->subvars;
      if ((var->data.stream & NIR_STREAM_PACKED) != 0 &&
          s->info.stage == MESA_SHADER_GEOMETRY &&
          var->data.mode == nir_var_shader_out) {
         for (unsigned i = 0; i < glsl_get_vector_elements(var->type); ++i) {
            unsigned stream = (var->data.stream >> (2 * (i + var->data.location_frac))) & 0x3;
            if (var_state->num_subvars == 0 || stream != subvars[var_state->num_subvars - 1].stream) {
               subvars[var_state->num_subvars].stream = stream;
               subvars[var_state->num_subvars].num_components = 1;
               var_state->num_subvars++;
            } else {
               subvars[var_state->num_subvars - 1].num_components++;
            }
         }

         var->data.stream = subvars[0].stream;
         if (var_state->num_subvars == 1)
            continue;

         progress = true;

         subvars[0].var = var;
         var->type = glsl_vector_type(glsl_get_base_type(var->type), subvars[0].num_components);
         unsigned location_frac = var->data.location_frac + subvars[0].num_components;
         for (unsigned subvar = 1; subvar < var_state->num_subvars; ++subvar) {
            char *name = ralloc_asprintf(s, "unpacked:%s_stream%d", var->name, subvars[subvar].stream);
            nir_variable *new_var = nir_variable_create(s, nir_var_shader_out,
                                                        glsl_vector_type(glsl_get_base_type(var->type), subvars[subvar].num_components),
                                                        name);

            new_var->data = var->data;
            new_var->data.stream = subvars[subvar].stream;
            new_var->data.location_frac = location_frac;
            location_frac += subvars[subvar].num_components;
            subvars[subvar].var = new_var;
         }
      } else if (glsl_type_is_64bit(glsl_without_array(var->type)) &&
                 glsl_get_components(glsl_without_array(var->type)) >= 3) {
         progress = true;
         assert(var->data.location_frac == 0);
         uint32_t components = glsl_get_components(glsl_without_array(var->type));
         var_state->num_subvars = 2;
         subvars[0].var = var;
         subvars[0].num_components = 2;
         subvars[0].stream = var->data.stream;
         const struct glsl_type *base_type = glsl_without_array(var->type);
         var->type = glsl_type_wrap_in_arrays(glsl_vector_type(glsl_get_base_type(base_type), 2), var->type);

         subvars[1].var = nir_variable_clone(var, s);
         subvars[1].num_components = components - 2;
         subvars[1].stream = var->data.stream;
         exec_node_insert_after(&var->node, &subvars[1].var->node);
         subvars[1].var->type = glsl_type_wrap_in_arrays(glsl_vector_type(glsl_get_base_type(base_type), components - 2), var->type);
         subvars[1].var->data.location++;
         subvars[1].var->data.driver_location++;
      }
   }

   if (progress) {
      nir_shader_intrinsics_pass(s, split_varying_accesses,
                                 nir_metadata_block_index | nir_metadata_dominance,
                                 &state);
   } else {
      nir_shader_preserve_all_metadata(s);
   }

   return progress;
}

static void
write_0(nir_builder *b, nir_deref_instr *deref)
{
   if (glsl_type_is_array_or_matrix(deref->type)) {
      for (unsigned i = 0; i < glsl_get_length(deref->type); ++i)
         write_0(b, nir_build_deref_array_imm(b, deref, i));
   } else if (glsl_type_is_struct(deref->type)) {
      for (unsigned i = 0; i < glsl_get_length(deref->type); ++i)
         write_0(b, nir_build_deref_struct(b, deref, i));
   } else {
      nir_def *scalar = nir_imm_intN_t(b, 0, glsl_get_bit_size(deref->type));
      nir_def *scalar_arr[NIR_MAX_VEC_COMPONENTS];
      unsigned num_comps = glsl_get_components(deref->type);
      unsigned writemask = (1 << num_comps) - 1;
      for (unsigned i = 0; i < num_comps; ++i)
         scalar_arr[i] = scalar;
      nir_def *zero_val = nir_vec(b, scalar_arr, num_comps);
      nir_store_deref(b, deref, zero_val, writemask);
   }
}

void
d3d12_write_0_to_new_varying(nir_shader *s, nir_variable *var)
{
   /* Skip per-vertex HS outputs */
   if (s->info.stage == MESA_SHADER_TESS_CTRL && !var->data.patch)
      return;

   nir_foreach_function_impl(impl, s) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl) {
         b.cursor = nir_before_block(block);
         if (s->info.stage != MESA_SHADER_GEOMETRY) {
            write_0(&b, nir_build_deref_var(&b, var));
            break;
         }

         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_emit_vertex)
               continue;

            b.cursor = nir_before_instr(instr);
            write_0(&b, nir_build_deref_var(&b, var));
         }
      }

      nir_metadata_preserve(impl, nir_metadata_block_index | nir_metadata_dominance);
   }
}
