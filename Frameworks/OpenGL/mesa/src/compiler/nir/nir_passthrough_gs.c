/*
 * Copyright © 2022 Collabora Ltc.
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

#include "util/u_memory.h"
#include "nir.h"
#include "nir_builder.h"
#include "nir_xfb_info.h"

static enum mesa_prim
gs_in_prim_for_topology(enum mesa_prim prim)
{
   switch (prim) {
   case MESA_PRIM_QUADS:
      return MESA_PRIM_LINES_ADJACENCY;
   default:
      return prim;
   }
}

static enum mesa_prim
gs_out_prim_for_topology(enum mesa_prim prim)
{
   switch (prim) {
   case MESA_PRIM_POINTS:
      return MESA_PRIM_POINTS;
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_LOOP:
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
   case MESA_PRIM_LINE_STRIP:
      return MESA_PRIM_LINE_STRIP;
   case MESA_PRIM_TRIANGLES:
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_TRIANGLE_FAN:
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
   case MESA_PRIM_POLYGON:
      return MESA_PRIM_TRIANGLE_STRIP;
   case MESA_PRIM_QUADS:
   case MESA_PRIM_QUAD_STRIP:
   case MESA_PRIM_PATCHES:
   default:
      return MESA_PRIM_QUADS;
   }
}

static unsigned int
vertices_for_prim(enum mesa_prim prim)
{
   switch (prim) {
   case MESA_PRIM_POINTS:
      return 1;
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_LOOP:
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
   case MESA_PRIM_LINE_STRIP:
      return 2;
   case MESA_PRIM_TRIANGLES:
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_TRIANGLE_FAN:
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
   case MESA_PRIM_POLYGON:
      return 3;
   case MESA_PRIM_QUADS:
   case MESA_PRIM_QUAD_STRIP:
      return 4;
   case MESA_PRIM_PATCHES:
   default:
      unreachable("unsupported primitive for gs input");
   }
}

static void
copy_vars(nir_builder *b, nir_deref_instr *dst, nir_deref_instr *src)
{
   assert(glsl_get_bare_type(dst->type) == glsl_get_bare_type(src->type));
   if (glsl_type_is_struct_or_ifc(dst->type)) {
      for (unsigned i = 0; i < glsl_get_length(dst->type); ++i) {
         copy_vars(b, nir_build_deref_struct(b, dst, i), nir_build_deref_struct(b, src, i));
      }
   } else if (glsl_type_is_array_or_matrix(dst->type)) {
      unsigned count = glsl_type_is_array(dst->type) ? glsl_array_size(dst->type) : glsl_get_matrix_columns(dst->type);
      for (unsigned i = 0; i < count; i++) {
         copy_vars(b, nir_build_deref_array_imm(b, dst, i), nir_build_deref_array_imm(b, src, i));
      }
   } else {
      nir_def *load = nir_load_deref(b, src);
      nir_store_deref(b, dst, load, BITFIELD_MASK(load->num_components));
   }
}

/*
 * A helper to create a passthrough GS shader for drivers that needs to lower
 * some rendering tasks to the GS.
 */

nir_shader *
nir_create_passthrough_gs(const nir_shader_compiler_options *options,
                          const nir_shader *prev_stage,
                          enum mesa_prim primitive_type,
                          enum mesa_prim output_primitive_type,
                          bool emulate_edgeflags,
                          bool force_line_strip_out)
{
   unsigned int vertices_out = vertices_for_prim(primitive_type);
   emulate_edgeflags = emulate_edgeflags && (prev_stage->info.outputs_written & VARYING_BIT_EDGE);
   enum mesa_prim original_our_prim = gs_out_prim_for_topology(output_primitive_type);
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_GEOMETRY,
                                                  options,
                                                  "gs passthrough");

   bool output_lines = force_line_strip_out || original_our_prim == MESA_PRIM_LINE_STRIP;
   bool needs_closing = (force_line_strip_out || (emulate_edgeflags && output_lines)) && vertices_out >= 3;

   nir_shader *nir = b.shader;
   nir->info.gs.input_primitive = gs_in_prim_for_topology(primitive_type);
   nir->info.gs.output_primitive = force_line_strip_out ? MESA_PRIM_LINE_STRIP : original_our_prim;
   nir->info.gs.vertices_in = mesa_vertices_per_prim(primitive_type);
   nir->info.gs.vertices_out = needs_closing ? vertices_out + 1 : vertices_out;
   nir->info.gs.invocations = 1;
   nir->info.gs.active_stream_mask = 1;

   nir->info.has_transform_feedback_varyings = prev_stage->info.has_transform_feedback_varyings;
   memcpy(nir->info.xfb_stride, prev_stage->info.xfb_stride, sizeof(prev_stage->info.xfb_stride));
   if (prev_stage->xfb_info) {
      nir->xfb_info = mem_dup(prev_stage->xfb_info, nir_xfb_info_size(prev_stage->xfb_info->output_count));
   }

   bool handle_flat = output_lines && nir->info.gs.output_primitive != gs_out_prim_for_topology(primitive_type);
   nir_variable *in_vars[VARYING_SLOT_MAX * 4];
   nir_variable *out_vars[VARYING_SLOT_MAX * 4];
   unsigned num_inputs = 0, num_outputs = 0;

   /* Create input/output variables. */
   nir_foreach_shader_out_variable(var, prev_stage) {
      assert(!var->data.patch);

      /* input vars can't be created for those */
      if (var->data.location == VARYING_SLOT_LAYER ||
          var->data.location == VARYING_SLOT_VIEW_INDEX)
         continue;

      char name[100];
      if (var->name)
         snprintf(name, sizeof(name), "in_%s", var->name);
      else
         snprintf(name, sizeof(name), "in_%d", var->data.driver_location);

      nir_variable *in = nir_variable_clone(var, nir);
      ralloc_free(in->name);
      in->name = ralloc_strdup(in, name);
      in->type = glsl_array_type(var->type, 6, false);
      in->data.mode = nir_var_shader_in;
      nir_shader_add_variable(nir, in);

      in_vars[num_inputs++] = in;

      nir->num_inputs++;
      if (in->data.location == VARYING_SLOT_EDGE)
         continue;

      if (var->data.location != VARYING_SLOT_POS)
         nir->num_outputs++;

      if (var->name)
         snprintf(name, sizeof(name), "out_%s", var->name);
      else
         snprintf(name, sizeof(name), "out_%d", var->data.driver_location);

      nir_variable *out = nir_variable_clone(var, nir);
      ralloc_free(out->name);
      out->name = ralloc_strdup(out, name);
      out->data.mode = nir_var_shader_out;
      nir_shader_add_variable(nir, out);

      out_vars[num_outputs++] = out;
   }

   unsigned int start_vert = 0;
   unsigned int end_vert = vertices_out;
   unsigned int vert_step = 1;
   switch (primitive_type) {
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      start_vert = 1;
      end_vert += 1;
      break;
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      end_vert = 5;
      vert_step = 2;
      break;
   default:
      break;
   }

   nir_variable *edge_var = nir_find_variable_with_location(nir, nir_var_shader_in, VARYING_SLOT_EDGE);
   nir_def *flat_interp_mask_def = nir_load_flat_mask(&b);
   nir_def *last_pv_vert_def = nir_load_provoking_last(&b);
   last_pv_vert_def = nir_ine_imm(&b, last_pv_vert_def, 0);
   nir_def *start_vert_index = nir_imm_int(&b, start_vert);
   nir_def *end_vert_index = nir_imm_int(&b, end_vert - 1);
   nir_def *pv_vert_index = nir_bcsel(&b, last_pv_vert_def, end_vert_index, start_vert_index);
   for (unsigned i = start_vert; i < end_vert || needs_closing; i += vert_step) {
      int idx = i < end_vert ? i : start_vert;
      /* Copy inputs to outputs. */
      for (unsigned j = 0, oj = 0; j < num_inputs; ++j) {
         if (in_vars[j]->data.location == VARYING_SLOT_EDGE) {
            continue;
         }
         /* no need to use copy_var to save a lower pass */
         nir_def *index;
         if (in_vars[j]->data.location == VARYING_SLOT_POS || !handle_flat)
            index = nir_imm_int(&b, idx);
         else {
            uint64_t mask = BITFIELD64_BIT(in_vars[j]->data.location);
            index = nir_bcsel(&b, nir_ieq_imm(&b, nir_iand_imm(&b, flat_interp_mask_def, mask), 0), nir_imm_int(&b, idx), pv_vert_index);
         }
         nir_deref_instr *value = nir_build_deref_array(&b, nir_build_deref_var(&b, in_vars[j]), index);
         copy_vars(&b, nir_build_deref_var(&b, out_vars[oj]), value);
         ++oj;
      }

      if (emulate_edgeflags && !output_lines) {
         nir_def *edge_value = nir_channel(&b, nir_load_array_var_imm(&b, edge_var, idx), 0);
         nir_push_if(&b, nir_feq_imm(&b, edge_value, 1.0));
      }

      nir_emit_vertex(&b, 0);

      if (emulate_edgeflags) {
         if (nir->info.gs.output_primitive == MESA_PRIM_LINE_STRIP) {
            nir_def *edge_value = nir_channel(&b, nir_load_array_var_imm(&b, edge_var, idx), 0);
            nir_push_if(&b, nir_fneu_imm(&b, edge_value, 1.0));
            nir_end_primitive(&b, 0);
            
         }
         nir_pop_if(&b, NULL);
      }

      if (i >= end_vert)
         break;
   }

   nir_end_primitive(&b, 0);
   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));
   nir_validate_shader(nir, "in nir_create_passthrough_gs");

   return nir;
}
