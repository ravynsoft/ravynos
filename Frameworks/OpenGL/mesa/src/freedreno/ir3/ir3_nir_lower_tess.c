/*
 * Copyright © 2019 Google, Inc.
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

#include "compiler/nir/nir_builder.h"
#include "ir3_compiler.h"
#include "ir3_nir.h"

struct state {
   uint32_t topology;

   struct primitive_map {
      /* +POSITION, +PSIZE, ... - see shader_io_get_unique_index */
      unsigned loc[12 + 32];
      unsigned stride;
   } map;

   nir_def *header;

   nir_variable *vertex_count_var;
   nir_variable *emitted_vertex_var;
   nir_variable *vertex_flags_out;

   struct exec_list old_outputs;
   struct exec_list new_outputs;
   struct exec_list emit_outputs;

   /* tess ctrl shader on a650 gets the local primitive id at different bits: */
   unsigned local_primitive_id_start;
};

static nir_def *
bitfield_extract(nir_builder *b, nir_def *v, uint32_t start, uint32_t mask)
{
   return nir_iand_imm(b, nir_ushr_imm(b, v, start), mask);
}

static nir_def *
build_invocation_id(nir_builder *b, struct state *state)
{
   return bitfield_extract(b, state->header, 11, 31);
}

static nir_def *
build_vertex_id(nir_builder *b, struct state *state)
{
   return bitfield_extract(b, state->header, 6, 31);
}

static nir_def *
build_local_primitive_id(nir_builder *b, struct state *state)
{
   return bitfield_extract(b, state->header, state->local_primitive_id_start,
                           63);
}

static bool
is_tess_levels(gl_varying_slot slot)
{
   return (slot == VARYING_SLOT_PRIMITIVE_ID ||
           slot == VARYING_SLOT_TESS_LEVEL_OUTER ||
           slot == VARYING_SLOT_TESS_LEVEL_INNER);
}

/* Return a deterministic index for varyings. We can't rely on driver_location
 * to be correct without linking the different stages first, so we create
 * "primitive maps" where the producer decides on the location of each varying
 * slot and then exports a per-slot array to the consumer. This compacts the
 * gl_varying_slot space down a bit so that the primitive maps aren't too
 * large.
 *
 * Note: per-patch varyings are currently handled separately, without any
 * compacting.
 *
 * TODO: We could probably use the driver_location's directly in the non-SSO
 * (Vulkan) case.
 */

static unsigned
shader_io_get_unique_index(gl_varying_slot slot)
{
   switch (slot) {
   case VARYING_SLOT_POS:         return 0;
   case VARYING_SLOT_PSIZ:        return 1;
   case VARYING_SLOT_COL0:        return 2;
   case VARYING_SLOT_COL1:        return 3;
   case VARYING_SLOT_BFC0:        return 4;
   case VARYING_SLOT_BFC1:        return 5;
   case VARYING_SLOT_FOGC:        return 6;
   case VARYING_SLOT_CLIP_DIST0:  return 7;
   case VARYING_SLOT_CLIP_DIST1:  return 8;
   case VARYING_SLOT_CLIP_VERTEX: return 9;
   case VARYING_SLOT_LAYER:       return 10;
   case VARYING_SLOT_VIEWPORT:    return 11;
   case VARYING_SLOT_VAR0 ... VARYING_SLOT_VAR31: {
      struct state state = {};
      STATIC_ASSERT(ARRAY_SIZE(state.map.loc) - 1 ==
                    (12 + VARYING_SLOT_VAR31 - VARYING_SLOT_VAR0));
      struct ir3_shader_variant v = {};
      STATIC_ASSERT(ARRAY_SIZE(v.output_loc) - 1 ==
                    (12 + VARYING_SLOT_VAR31 - VARYING_SLOT_VAR0));
      return 12 + (slot - VARYING_SLOT_VAR0);
   }
   default:
      unreachable("illegal slot in get unique index\n");
   }
}

static nir_def *
build_local_offset(nir_builder *b, struct state *state, nir_def *vertex,
                   uint32_t location, uint32_t comp, nir_def *offset)
{
   nir_def *primitive_stride = nir_load_vs_primitive_stride_ir3(b);
   nir_def *primitive_offset =
      nir_imul24(b, build_local_primitive_id(b, state), primitive_stride);
   nir_def *attr_offset;
   nir_def *vertex_stride;
   unsigned index = shader_io_get_unique_index(location);

   switch (b->shader->info.stage) {
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_TESS_EVAL:
      vertex_stride = nir_imm_int(b, state->map.stride * 4);
      attr_offset = nir_imm_int(b, state->map.loc[index] + 4 * comp);
      break;
   case MESA_SHADER_TESS_CTRL:
   case MESA_SHADER_GEOMETRY:
      vertex_stride = nir_load_vs_vertex_stride_ir3(b);
      attr_offset = nir_iadd_imm(b, nir_load_primitive_location_ir3(b, index),
                                 comp * 4);
      break;
   default:
      unreachable("bad shader stage");
   }

   nir_def *vertex_offset = nir_imul24(b, vertex, vertex_stride);

   return nir_iadd(
      b, nir_iadd(b, primitive_offset, vertex_offset),
      nir_iadd(b, attr_offset, nir_ishl_imm(b, offset, 4)));
}

static nir_intrinsic_instr *
replace_intrinsic(nir_builder *b, nir_intrinsic_instr *intr,
                  nir_intrinsic_op op, nir_def *src0, nir_def *src1,
                  nir_def *src2)
{
   nir_intrinsic_instr *new_intr = nir_intrinsic_instr_create(b->shader, op);

   new_intr->src[0] = nir_src_for_ssa(src0);
   if (src1)
      new_intr->src[1] = nir_src_for_ssa(src1);
   if (src2)
      new_intr->src[2] = nir_src_for_ssa(src2);

   new_intr->num_components = intr->num_components;

   if (nir_intrinsic_infos[op].has_dest)
      nir_def_init(&new_intr->instr, &new_intr->def,
                   intr->num_components, intr->def.bit_size);

   nir_builder_instr_insert(b, &new_intr->instr);

   if (nir_intrinsic_infos[op].has_dest)
      nir_def_rewrite_uses(&intr->def, &new_intr->def);

   nir_instr_remove(&intr->instr);

   return new_intr;
}

static void
build_primitive_map(nir_shader *shader, struct primitive_map *map)
{
   /* All interfaces except the TCS <-> TES interface use ldlw, which takes
    * an offset in bytes, so each vec4 slot is 16 bytes. TCS <-> TES uses
    * ldg, which takes an offset in dwords, but each per-vertex slot has
    * space for every vertex, and there's space at the beginning for
    * per-patch varyings.
    */
   unsigned slot_size = 16, start = 0;
   if (shader->info.stage == MESA_SHADER_TESS_CTRL) {
      slot_size = shader->info.tess.tcs_vertices_out * 4;
      start = util_last_bit(shader->info.patch_outputs_written) * 4;
   }

   uint64_t mask = shader->info.outputs_written;
   unsigned loc = start;
   while (mask) {
      int location = u_bit_scan64(&mask);
      if (is_tess_levels(location))
         continue;

      unsigned index = shader_io_get_unique_index(location);
      map->loc[index] = loc;
      loc += slot_size;
   }

   map->stride = loc;
   /* Use units of dwords for the stride. */
   if (shader->info.stage != MESA_SHADER_TESS_CTRL)
      map->stride /= 4;
}

/* For shader stages that receive a primitive map, calculate how big it should
 * be.
 */

static unsigned
calc_primitive_map_size(nir_shader *shader)
{
   uint64_t mask = shader->info.inputs_read;
   unsigned max_index = 0;
   while (mask) {
      int location = u_bit_scan64(&mask);

      if (is_tess_levels(location))
         continue;

      unsigned index = shader_io_get_unique_index(location);
      max_index = MAX2(max_index, index + 1);
   }

   return max_index;
}

static void
lower_block_to_explicit_output(nir_block *block, nir_builder *b,
                               struct state *state)
{
   nir_foreach_instr_safe (instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

      switch (intr->intrinsic) {
      case nir_intrinsic_store_output: {
         // src[] = { value, offset }.

         /* nir_lower_io_to_temporaries replaces all access to output
          * variables with temp variables and then emits a nir_copy_var at
          * the end of the shader.  Thus, we should always get a full wrmask
          * here.
          */
         assert(
            util_is_power_of_two_nonzero(nir_intrinsic_write_mask(intr) + 1));

         b->cursor = nir_instr_remove(&intr->instr);

         nir_def *vertex_id = build_vertex_id(b, state);
         nir_def *offset = build_local_offset(
            b, state, vertex_id, nir_intrinsic_io_semantics(intr).location,
            nir_intrinsic_component(intr), intr->src[1].ssa);

         nir_store_shared_ir3(b, intr->src[0].ssa, offset);
         break;
      }

      default:
         break;
      }
   }
}

static nir_def *
local_thread_id(nir_builder *b)
{
   return bitfield_extract(b, nir_load_gs_header_ir3(b), 16, 1023);
}

void
ir3_nir_lower_to_explicit_output(nir_shader *shader,
                                 struct ir3_shader_variant *v,
                                 unsigned topology)
{
   struct state state = {};

   build_primitive_map(shader, &state.map);
   memcpy(v->output_loc, state.map.loc, sizeof(v->output_loc));

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);

   nir_builder b = nir_builder_at(nir_before_impl(impl));

   if (v->type == MESA_SHADER_VERTEX && topology != IR3_TESS_NONE)
      state.header = nir_load_tcs_header_ir3(&b);
   else
      state.header = nir_load_gs_header_ir3(&b);

   nir_foreach_block_safe (block, impl)
      lower_block_to_explicit_output(block, &b, &state);

   nir_metadata_preserve(impl,
                         nir_metadata_block_index | nir_metadata_dominance);

   v->output_size = state.map.stride;
}

static void
lower_block_to_explicit_input(nir_block *block, nir_builder *b,
                              struct state *state)
{
   nir_foreach_instr_safe (instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

      switch (intr->intrinsic) {
      case nir_intrinsic_load_per_vertex_input: {
         // src[] = { vertex, offset }.

         b->cursor = nir_before_instr(&intr->instr);

         nir_def *offset = build_local_offset(
            b, state,
            intr->src[0].ssa, // this is typically gl_InvocationID
            nir_intrinsic_io_semantics(intr).location,
            nir_intrinsic_component(intr), intr->src[1].ssa);

         replace_intrinsic(b, intr, nir_intrinsic_load_shared_ir3, offset, NULL,
                           NULL);
         break;
      }

      case nir_intrinsic_load_invocation_id: {
         b->cursor = nir_before_instr(&intr->instr);

         nir_def *iid = build_invocation_id(b, state);
         nir_def_rewrite_uses(&intr->def, iid);
         nir_instr_remove(&intr->instr);
         break;
      }

      default:
         break;
      }
   }
}

void
ir3_nir_lower_to_explicit_input(nir_shader *shader,
                                struct ir3_shader_variant *v)
{
   struct state state = {};

   /* when using stl/ldl (instead of stlw/ldlw) for linking VS and HS,
    * HS uses a different primitive id, which starts at bit 16 in the header
    */
   if (shader->info.stage == MESA_SHADER_TESS_CTRL &&
       v->compiler->tess_use_shared)
      state.local_primitive_id_start = 16;

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);

   nir_builder b = nir_builder_at(nir_before_impl(impl));

   if (shader->info.stage == MESA_SHADER_GEOMETRY)
      state.header = nir_load_gs_header_ir3(&b);
   else
      state.header = nir_load_tcs_header_ir3(&b);

   nir_foreach_block_safe (block, impl)
      lower_block_to_explicit_input(block, &b, &state);

   v->input_size = calc_primitive_map_size(shader);
}

static nir_def *
build_tcs_out_vertices(nir_builder *b)
{
   if (b->shader->info.stage == MESA_SHADER_TESS_CTRL)
      return nir_imm_int(b, b->shader->info.tess.tcs_vertices_out);
   else
      return nir_load_patch_vertices_in(b);
}

static nir_def *
build_per_vertex_offset(nir_builder *b, struct state *state,
                        nir_def *vertex, uint32_t location, uint32_t comp,
                        nir_def *offset)
{
   nir_def *patch_id = nir_load_rel_patch_id_ir3(b);
   nir_def *patch_stride = nir_load_hs_patch_stride_ir3(b);
   nir_def *patch_offset = nir_imul24(b, patch_id, patch_stride);
   nir_def *attr_offset;

   if (nir_src_is_const(nir_src_for_ssa(offset))) {
      location += nir_src_as_uint(nir_src_for_ssa(offset));
      offset = nir_imm_int(b, 0);
   } else {
      /* Offset is in vec4's, but we need it in unit of components for the
       * load/store_global_ir3 offset.
       */
      offset = nir_ishl_imm(b, offset, 2);
   }

   nir_def *vertex_offset;
   if (vertex) {
      unsigned index = shader_io_get_unique_index(location);
      switch (b->shader->info.stage) {
      case MESA_SHADER_TESS_CTRL:
         attr_offset = nir_imm_int(b, state->map.loc[index] + comp);
         break;
      case MESA_SHADER_TESS_EVAL:
         attr_offset = nir_iadd_imm(b, nir_load_primitive_location_ir3(b, index),
                                    comp);
         break;
      default:
         unreachable("bad shader state");
      }

      attr_offset = nir_iadd(b, attr_offset,
                             nir_imul24(b, offset, build_tcs_out_vertices(b)));
      vertex_offset = nir_ishl_imm(b, vertex, 2);
   } else {
      assert(location >= VARYING_SLOT_PATCH0 &&
             location <= VARYING_SLOT_TESS_MAX);
      unsigned index = location - VARYING_SLOT_PATCH0;
      attr_offset = nir_iadd_imm(b, offset, index * 4 + comp);
      vertex_offset = nir_imm_int(b, 0);
   }

   return nir_iadd(b, nir_iadd(b, patch_offset, attr_offset), vertex_offset);
}

static nir_def *
build_patch_offset(nir_builder *b, struct state *state, uint32_t base,
                   uint32_t comp, nir_def *offset)
{
   return build_per_vertex_offset(b, state, NULL, base, comp, offset);
}

static void
tess_level_components(struct state *state, uint32_t *inner, uint32_t *outer)
{
   switch (state->topology) {
   case IR3_TESS_TRIANGLES:
      *inner = 1;
      *outer = 3;
      break;
   case IR3_TESS_QUADS:
      *inner = 2;
      *outer = 4;
      break;
   case IR3_TESS_ISOLINES:
      *inner = 0;
      *outer = 2;
      break;
   default:
      unreachable("bad");
   }
}

static nir_def *
build_tessfactor_base(nir_builder *b, gl_varying_slot slot, uint32_t comp,
                      struct state *state)
{
   uint32_t inner_levels, outer_levels;
   tess_level_components(state, &inner_levels, &outer_levels);

   const uint32_t patch_stride = 1 + inner_levels + outer_levels;

   nir_def *patch_id = nir_load_rel_patch_id_ir3(b);

   nir_def *patch_offset =
      nir_imul24(b, patch_id, nir_imm_int(b, patch_stride));

   uint32_t offset;
   switch (slot) {
   case VARYING_SLOT_PRIMITIVE_ID:
      offset = 0;
      break;
   case VARYING_SLOT_TESS_LEVEL_OUTER:
      offset = 1;
      break;
   case VARYING_SLOT_TESS_LEVEL_INNER:
      offset = 1 + outer_levels;
      break;
   default:
      unreachable("bad");
   }

   return nir_iadd_imm(b, patch_offset, offset + comp);
}

static void
lower_tess_ctrl_block(nir_block *block, nir_builder *b, struct state *state)
{
   nir_foreach_instr_safe (instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

      switch (intr->intrinsic) {
      case nir_intrinsic_load_per_vertex_output: {
         // src[] = { vertex, offset }.

         b->cursor = nir_before_instr(&intr->instr);

         nir_def *address = nir_load_tess_param_base_ir3(b);
         nir_def *offset = build_per_vertex_offset(
            b, state, intr->src[0].ssa,
            nir_intrinsic_io_semantics(intr).location,
            nir_intrinsic_component(intr), intr->src[1].ssa);

         replace_intrinsic(b, intr, nir_intrinsic_load_global_ir3, address,
                           offset, NULL);
         break;
      }

      case nir_intrinsic_store_per_vertex_output: {
         // src[] = { value, vertex, offset }.

         b->cursor = nir_before_instr(&intr->instr);

         /* sparse writemask not supported */
         assert(
            util_is_power_of_two_nonzero(nir_intrinsic_write_mask(intr) + 1));

         nir_def *value = intr->src[0].ssa;
         nir_def *address = nir_load_tess_param_base_ir3(b);
         nir_def *offset = build_per_vertex_offset(
            b, state, intr->src[1].ssa,
            nir_intrinsic_io_semantics(intr).location,
            nir_intrinsic_component(intr), intr->src[2].ssa);

         replace_intrinsic(b, intr, nir_intrinsic_store_global_ir3, value,
                           address, offset);

         break;
      }

      case nir_intrinsic_load_output: {
         // src[] = { offset }.

         b->cursor = nir_before_instr(&intr->instr);

         nir_def *address, *offset;

         /* note if vectorization of the tess level loads ever happens:
          * "ldg" across 16-byte boundaries can behave incorrectly if results
          * are never used. most likely some issue with (sy) not properly
          * syncing with values coming from a second memory transaction.
          */
         gl_varying_slot location = nir_intrinsic_io_semantics(intr).location;
         if (is_tess_levels(location)) {
            assert(intr->def.num_components == 1);
            address = nir_load_tess_factor_base_ir3(b);
            offset = build_tessfactor_base(
               b, location, nir_intrinsic_component(intr), state);
         } else {
            address = nir_load_tess_param_base_ir3(b);
            offset = build_patch_offset(b, state, location,
                                        nir_intrinsic_component(intr),
                                        intr->src[0].ssa);
         }

         replace_intrinsic(b, intr, nir_intrinsic_load_global_ir3, address,
                           offset, NULL);
         break;
      }

      case nir_intrinsic_store_output: {
         // src[] = { value, offset }.

         /* write patch output to bo */

         b->cursor = nir_before_instr(&intr->instr);

         /* sparse writemask not supported */
         assert(
            util_is_power_of_two_nonzero(nir_intrinsic_write_mask(intr) + 1));

         gl_varying_slot location = nir_intrinsic_io_semantics(intr).location;
         if (is_tess_levels(location)) {
            uint32_t inner_levels, outer_levels, levels;
            tess_level_components(state, &inner_levels, &outer_levels);

            assert(intr->src[0].ssa->num_components == 1);

            nir_if *nif = NULL;
            if (location != VARYING_SLOT_PRIMITIVE_ID) {
               /* with tess levels are defined as float[4] and float[2],
                * but tess factor BO has smaller sizes for tris/isolines,
                * so we have to discard any writes beyond the number of
                * components for inner/outer levels
                */
               if (location == VARYING_SLOT_TESS_LEVEL_OUTER)
                  levels = outer_levels;
               else
                  levels = inner_levels;

               nir_def *offset = nir_iadd_imm(
                  b, intr->src[1].ssa, nir_intrinsic_component(intr));
               nif = nir_push_if(b, nir_ult_imm(b, offset, levels));
            }

            nir_def *offset = build_tessfactor_base(
               b, location, nir_intrinsic_component(intr), state);

            replace_intrinsic(b, intr, nir_intrinsic_store_global_ir3,
                              intr->src[0].ssa,
                              nir_load_tess_factor_base_ir3(b),
                              nir_iadd(b, intr->src[1].ssa, offset));

            if (location != VARYING_SLOT_PRIMITIVE_ID) {
               nir_pop_if(b, nif);
            }
         } else {
            nir_def *address = nir_load_tess_param_base_ir3(b);
            nir_def *offset = build_patch_offset(
               b, state, location, nir_intrinsic_component(intr),
               intr->src[1].ssa);

            replace_intrinsic(b, intr, nir_intrinsic_store_global_ir3,
                              intr->src[0].ssa, address, offset);
         }
         break;
      }

      default:
         break;
      }
   }
}

static void
emit_tess_epilouge(nir_builder *b, struct state *state)
{
   /* Insert endpatch instruction:
    *
    * TODO we should re-work this to use normal flow control.
    */

   nir_end_patch_ir3(b);
}

void
ir3_nir_lower_tess_ctrl(nir_shader *shader, struct ir3_shader_variant *v,
                        unsigned topology)
{
   struct state state = {.topology = topology};

   if (shader_debug_enabled(shader->info.stage, shader->info.internal)) {
      mesa_logi("NIR (before tess lowering) for %s shader:",
                _mesa_shader_stage_to_string(shader->info.stage));
      nir_log_shaderi(shader);
   }

   build_primitive_map(shader, &state.map);
   memcpy(v->output_loc, state.map.loc, sizeof(v->output_loc));
   v->output_size = state.map.stride;

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);

   nir_builder b = nir_builder_at(nir_before_impl(impl));

   state.header = nir_load_tcs_header_ir3(&b);

   /* If required, store gl_PrimitiveID. */
   if (v->key.tcs_store_primid) {
      b.cursor = nir_after_impl(impl);

      nir_store_output(&b, nir_load_primitive_id(&b), nir_imm_int(&b, 0),
                       .io_semantics = {
                           .location = VARYING_SLOT_PRIMITIVE_ID,
                           .num_slots = 1
                        });

      b.cursor = nir_before_impl(impl);
   }

   nir_foreach_block_safe (block, impl)
      lower_tess_ctrl_block(block, &b, &state);

   /* Now move the body of the TCS into a conditional:
    *
    *   if (gl_InvocationID < num_vertices)
    *     // body
    *
    */

   nir_cf_list body;
   nir_cf_extract(&body, nir_before_impl(impl),
                  nir_after_impl(impl));

   b.cursor = nir_after_impl(impl);

   /* Re-emit the header, since the old one got moved into the if branch */
   state.header = nir_load_tcs_header_ir3(&b);
   nir_def *iid = build_invocation_id(&b, &state);

   const uint32_t nvertices = shader->info.tess.tcs_vertices_out;
   nir_def *cond = nir_ult_imm(&b, iid, nvertices);

   nir_if *nif = nir_push_if(&b, cond);

   nir_cf_reinsert(&body, b.cursor);

   b.cursor = nir_after_cf_list(&nif->then_list);

   /* Insert conditional exit for threads invocation id != 0 */
   nir_def *iid0_cond = nir_ieq_imm(&b, iid, 0);
   nir_cond_end_ir3(&b, iid0_cond);

   emit_tess_epilouge(&b, &state);

   nir_pop_if(&b, nif);

   nir_metadata_preserve(impl, nir_metadata_none);
}

static void
lower_tess_eval_block(nir_block *block, nir_builder *b, struct state *state)
{
   nir_foreach_instr_safe (instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

      switch (intr->intrinsic) {
      case nir_intrinsic_load_per_vertex_input: {
         // src[] = { vertex, offset }.

         b->cursor = nir_before_instr(&intr->instr);

         nir_def *address = nir_load_tess_param_base_ir3(b);
         nir_def *offset = build_per_vertex_offset(
            b, state, intr->src[0].ssa,
            nir_intrinsic_io_semantics(intr).location,
            nir_intrinsic_component(intr), intr->src[1].ssa);

         replace_intrinsic(b, intr, nir_intrinsic_load_global_ir3, address,
                           offset, NULL);
         break;
      }

      case nir_intrinsic_load_input: {
         // src[] = { offset }.

         b->cursor = nir_before_instr(&intr->instr);

         nir_def *address, *offset;

         /* note if vectorization of the tess level loads ever happens:
          * "ldg" across 16-byte boundaries can behave incorrectly if results
          * are never used. most likely some issue with (sy) not properly
          * syncing with values coming from a second memory transaction.
          */
         gl_varying_slot location = nir_intrinsic_io_semantics(intr).location;
         if (is_tess_levels(location)) {
            assert(intr->def.num_components == 1);
            address = nir_load_tess_factor_base_ir3(b);
            offset = build_tessfactor_base(
               b, location, nir_intrinsic_component(intr), state);
         } else {
            address = nir_load_tess_param_base_ir3(b);
            offset = build_patch_offset(b, state, location,
                                        nir_intrinsic_component(intr),
                                        intr->src[0].ssa);
         }

         replace_intrinsic(b, intr, nir_intrinsic_load_global_ir3, address,
                           offset, NULL);
         break;
      }

      default:
         break;
      }
   }
}

void
ir3_nir_lower_tess_eval(nir_shader *shader, struct ir3_shader_variant *v,
                        unsigned topology)
{
   struct state state = {.topology = topology};

   if (shader_debug_enabled(shader->info.stage, shader->info.internal)) {
      mesa_logi("NIR (before tess lowering) for %s shader:",
                _mesa_shader_stage_to_string(shader->info.stage));
      nir_log_shaderi(shader);
   }

   NIR_PASS_V(shader, nir_lower_tess_coord_z, topology == IR3_TESS_TRIANGLES);

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);

   nir_builder b = nir_builder_create(impl);

   nir_foreach_block_safe (block, impl)
      lower_tess_eval_block(block, &b, &state);

   v->input_size = calc_primitive_map_size(shader);

   nir_metadata_preserve(impl, nir_metadata_none);
}

/* The hardware does not support incomplete primitives in multiple streams at
 * once or ending the "wrong" stream, but Vulkan allows this. That is,
 * EmitStreamVertex(N) followed by EmitStreamVertex(M) or EndStreamPrimitive(M)
 * where N != M and there isn't a call to EndStreamPrimitive(N) in between isn't
 * supported by the hardware. Fix this up by duplicating the entire shader per
 * stream, removing EmitStreamVertex/EndStreamPrimitive calls for streams other
 * than the current one.
 */

static void
lower_mixed_streams(nir_shader *nir)
{
   /* We don't have to do anything for points because there is only one vertex
    * per primitive and therefore no possibility of mixing.
    */
   if (nir->info.gs.output_primitive == MESA_PRIM_POINTS)
      return;

   nir_function_impl *entrypoint = nir_shader_get_entrypoint(nir);

   uint8_t stream_mask = 0;

   nir_foreach_block (block, entrypoint) {
      nir_foreach_instr (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         if (intrin->intrinsic == nir_intrinsic_emit_vertex ||
             intrin->intrinsic == nir_intrinsic_end_primitive)
            stream_mask |= 1 << nir_intrinsic_stream_id(intrin);
      }
   }

   if (util_is_power_of_two_or_zero(stream_mask))
      return;

   nir_cf_list body;
   nir_cf_list_extract(&body, &entrypoint->body);

   nir_builder b = nir_builder_create(entrypoint);

   u_foreach_bit (stream, stream_mask) {
      b.cursor = nir_after_impl(entrypoint);
      
      /* Inserting the cloned body invalidates any cursor not using an
       * instruction, so we need to emit this to keep track of where the new
       * body is to iterate over it.
       */
      nir_instr *anchor = &nir_nop(&b)->instr;

      nir_cf_list_clone_and_reinsert(&body, &entrypoint->cf_node, b.cursor, NULL);

      /* We need to iterate over all instructions after the anchor, which is a
       * bit tricky to do so we do it manually.
       */
      for (nir_block *block = anchor->block; block != NULL;
           block = nir_block_cf_tree_next(block)) {
         for (nir_instr *instr = 
               (block == anchor->block) ? anchor : nir_block_first_instr(block),
               *next = instr ? nir_instr_next(instr) : NULL;
              instr != NULL; instr = next, next = next ? nir_instr_next(next) : NULL) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            if ((intrin->intrinsic == nir_intrinsic_emit_vertex ||
                 intrin->intrinsic == nir_intrinsic_end_primitive) &&
                nir_intrinsic_stream_id(intrin) != stream) {
               nir_instr_remove(instr);
            }
         }
      }

      nir_instr_remove(anchor);

      /* The user can omit the last EndStreamPrimitive(), so add an extra one
       * here before potentially adding other copies of the body that emit to
       * different streams. Our lowering means that redundant calls to
       * EndStreamPrimitive are safe and should be optimized out.
       */
      b.cursor = nir_after_impl(entrypoint);
      nir_end_primitive(&b, .stream_id = stream);
   }

   nir_cf_delete(&body);
}

static void
copy_vars(nir_builder *b, struct exec_list *dests, struct exec_list *srcs)
{
   foreach_two_lists (dest_node, dests, src_node, srcs) {
      nir_variable *dest = exec_node_data(nir_variable, dest_node, node);
      nir_variable *src = exec_node_data(nir_variable, src_node, node);
      nir_copy_var(b, dest, src);
   }
}

static void
lower_gs_block(nir_block *block, nir_builder *b, struct state *state)
{
   nir_foreach_instr_safe (instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

      switch (intr->intrinsic) {
      case nir_intrinsic_end_primitive: {
         /* The HW will use the stream from the preceding emitted vertices,
          * which thanks to the lower_mixed_streams is the same as the stream
          * for this instruction, so we can ignore it here.
          */
         b->cursor = nir_before_instr(&intr->instr);
         nir_store_var(b, state->vertex_flags_out, nir_imm_int(b, 4), 0x1);
         nir_instr_remove(&intr->instr);
         break;
      }

      case nir_intrinsic_emit_vertex: {
         /* Load the vertex count */
         b->cursor = nir_before_instr(&intr->instr);
         nir_def *count = nir_load_var(b, state->vertex_count_var);

         nir_push_if(b, nir_ieq(b, count, local_thread_id(b)));

         unsigned stream = nir_intrinsic_stream_id(intr);
         /* vertex_flags_out |= stream */
         nir_store_var(b, state->vertex_flags_out,
                       nir_ior_imm(b, nir_load_var(b, state->vertex_flags_out),
                                   stream),
                       0x1 /* .x */);

         copy_vars(b, &state->emit_outputs, &state->old_outputs);

         nir_instr_remove(&intr->instr);

         nir_store_var(b, state->emitted_vertex_var,
                       nir_iadd_imm(b,
                                    nir_load_var(b,
                                                 state->emitted_vertex_var),
                                                 1),
                       0x1);

         nir_pop_if(b, NULL);

         /* Increment the vertex count by 1 */
         nir_store_var(b, state->vertex_count_var,
                       nir_iadd_imm(b, count, 1), 0x1); /* .x */
         nir_store_var(b, state->vertex_flags_out, nir_imm_int(b, 0), 0x1);

         break;
      }

      default:
         break;
      }
   }
}

void
ir3_nir_lower_gs(nir_shader *shader)
{
   struct state state = {};

   /* Don't lower multiple times: */
   nir_foreach_shader_out_variable (var, shader)
      if (var->data.location == VARYING_SLOT_GS_VERTEX_FLAGS_IR3)
         return;

   if (shader_debug_enabled(shader->info.stage, shader->info.internal)) {
      mesa_logi("NIR (before gs lowering):");
      nir_log_shaderi(shader);
   }

   lower_mixed_streams(shader);

   /* Create an output var for vertex_flags. This will be shadowed below,
    * same way regular outputs get shadowed, and this variable will become a
    * temporary.
    */
   state.vertex_flags_out = nir_variable_create(
      shader, nir_var_shader_out, glsl_uint_type(), "vertex_flags");
   state.vertex_flags_out->data.driver_location = shader->num_outputs++;
   state.vertex_flags_out->data.location = VARYING_SLOT_GS_VERTEX_FLAGS_IR3;
   state.vertex_flags_out->data.interpolation = INTERP_MODE_NONE;

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);

   nir_builder b = nir_builder_at(nir_before_impl(impl));

   state.header = nir_load_gs_header_ir3(&b);

   /* Generate two set of shadow vars for the output variables.  The first
    * set replaces the real outputs and the second set (emit_outputs) we'll
    * assign in the emit_vertex conditionals.  Then at the end of the shader
    * we copy the emit_outputs to the real outputs, so that we get
    * store_output in uniform control flow.
    */
   exec_list_make_empty(&state.old_outputs);
   nir_foreach_shader_out_variable_safe (var, shader) {
      exec_node_remove(&var->node);
      exec_list_push_tail(&state.old_outputs, &var->node);
   }
   exec_list_make_empty(&state.new_outputs);
   exec_list_make_empty(&state.emit_outputs);
   nir_foreach_variable_in_list (var, &state.old_outputs) {
      /* Create a new output var by cloning the original output var and
       * stealing the name.
       */
      nir_variable *output = nir_variable_clone(var, shader);
      exec_list_push_tail(&state.new_outputs, &output->node);

      /* Rewrite the original output to be a shadow variable. */
      var->name = ralloc_asprintf(var, "%s@gs-temp", output->name);
      var->data.mode = nir_var_shader_temp;

      /* Clone the shadow variable to create the emit shadow variable that
       * we'll assign in the emit conditionals.
       */
      nir_variable *emit_output = nir_variable_clone(var, shader);
      emit_output->name = ralloc_asprintf(var, "%s@emit-temp", output->name);
      exec_list_push_tail(&state.emit_outputs, &emit_output->node);
   }

   /* During the shader we'll keep track of which vertex we're currently
    * emitting for the EmitVertex test and how many vertices we emitted so we
    * know to discard if didn't emit any.  In most simple shaders, this can
    * all be statically determined and gets optimized away.
    */
   state.vertex_count_var =
      nir_local_variable_create(impl, glsl_uint_type(), "vertex_count");
   state.emitted_vertex_var =
      nir_local_variable_create(impl, glsl_uint_type(), "emitted_vertex");

   /* Initialize to 0. */
   b.cursor = nir_before_impl(impl);
   nir_store_var(&b, state.vertex_count_var, nir_imm_int(&b, 0), 0x1);
   nir_store_var(&b, state.emitted_vertex_var, nir_imm_int(&b, 0), 0x1);
   nir_store_var(&b, state.vertex_flags_out, nir_imm_int(&b, 4), 0x1);

   nir_foreach_block_safe (block, impl)
      lower_gs_block(block, &b, &state);

   /* Note: returns are lowered, so there should be only one block before the
    * end block.  If we had real returns, we would probably want to redirect
    * them to this new if statement, rather than emitting this code at every
    * return statement.
    */
   assert(impl->end_block->predecessors->entries == 1);
   nir_block *block = nir_impl_last_block(impl);
   b.cursor = nir_after_block_before_jump(block);

   /* If we haven't emitted any vertex we need to copy the shadow (old)
    * outputs to emit outputs here.
    *
    * Also some piglit GS tests[1] don't have EndPrimitive() so throw
    * in an extra vertex_flags write for good measure.  If unneeded it
    * will be optimized out.
    *
    * [1] ex, tests/spec/glsl-1.50/execution/compatibility/clipping/gs-clip-vertex-const-accept.shader_test
    */
   nir_def *cond =
      nir_ieq_imm(&b, nir_load_var(&b, state.emitted_vertex_var), 0);
   nir_push_if(&b, cond);
   nir_store_var(&b, state.vertex_flags_out, nir_imm_int(&b, 4), 0x1);
   copy_vars(&b, &state.emit_outputs, &state.old_outputs);
   nir_pop_if(&b, NULL);

   nir_discard_if(&b, cond);

   copy_vars(&b, &state.new_outputs, &state.emit_outputs);

   exec_list_append(&shader->variables, &state.old_outputs);
   exec_list_append(&shader->variables, &state.emit_outputs);
   exec_list_append(&shader->variables, &state.new_outputs);

   nir_metadata_preserve(impl, nir_metadata_none);

   nir_lower_global_vars_to_local(shader);
   nir_split_var_copies(shader);
   nir_lower_var_copies(shader);

   nir_fixup_deref_modes(shader);

   if (shader_debug_enabled(shader->info.stage, shader->info.internal)) {
      mesa_logi("NIR (after gs lowering):");
      nir_log_shaderi(shader);
   }
}
