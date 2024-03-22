/*
 * Copyright © 2021 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_nir.h"
#include "nir_builder.h"

/*
 * These NIR passes are used to lower NIR cross-stage I/O intrinsics into the
 * memory accesses that actually happen on the HW.
 *
 * Each input and output has a 16-byte (4 dwords) slot reserved for it, and
 * can have up to 4 components. Each component is 32 bits.
 *
 * ## VS-TCS-TES I/O - Terminology:
 *
 * * patch - Group of vertices, used instead of primitives in tessellation
 * * per-vertex - input or output which can be different for every vertex.
 * * per-patch - input output which applies to a patch (a group of vertices)
 *
 * ## VS-TCS-TES I/O - How it works:
 *
 * ```
 * SW model:    SW VS         SW TCS    tessellator    SW TES
 *                ┊             ┊             ┊          ┊
 *              ┌────┐        ┌────┐        ┌────┐    ┌─────┐
 * HW pipeline: │ LS │─╮   ╭─>│ HS │─╮   ╭─>│ FF │ ╭─>│VS/ES│
 *              └────┘ │   │  └────┘ │   │  └────┘ │  └─────┘
 * Memory:             ╰─>LDS<──╯    ╰─>VRAM───────╯
 * ```
 *
 * * SW VS runs as a HW LS (Local Shader, merged into HS on GFX9+),
 *   and SW TCS runs as HW HS (Hull Shader).
 *   SW TES runs as either HW VS or HW ES (Export Shader).
 * * LS and HS share the same LDS space.
 * * LS (SW VS) stores outputs to LDS to be read by HS (SW TCS).
 * * HS (SW TCS) stores outputs in LDS if the HS (SW TCS) reads them.
 * * HS (SW TCS) stores outputs in VRAM if the next stage (SW TES) reads them.
 *
 * Side note: some old HW supports having TES read from the same LDS space where LS/HS write, but
 * Mesa always stores HS outputs to VRAM to avoid forcing TES waves to run on the same CU as the LS/HS waves.
 *
 * ### Passing VS-TCS I/O in registers
 *
 * On GPUs that run SW VS and  SW TCS on the same HW stage (HS on GFX9+),
 * IO can be passed through registers instead of LDS when the following conditions are met:
 *
 * 1. TCS input and output patch size match
 * 2. Floating point execution modes in SW VS and SW TCS match
 * 3. The SW VS output is not written indirectly, and the corresponding SW TCS input is not read indirectly
 *
 * Some HS outputs could be passed through registers to, but this is a TODO.
 *
 * ### LDS layout used by VS-TCS:
 *
 * ```
 * TCS per-vertex inputs for patch 0  <─── 0
 * TCS per-vertex inputs for patch 1
 * TCS per-vertex inputs for patch 2  <─── hs_per_vertex_input_lds_offset (rel_patch_id = 2)
 * ...
 * TCS per-vertex outputs for patch 0 <─── output_patch0_offset
 * TCS per-patch outputs for patch 0  <─── output_patch0_patch_data_offset
 * TCS per-vertex outputs for patch 1
 * TCS per-patch outputs for patch 1
 * TCS per-vertex outputs for patch 2 <─── hs_output_lds_offset (rel_patch_id = 2, per-vertex)
 * TCS per-patch outputs for patch 2  <─── hs_output_lds_offset (rel_patch_id = 2, per-patch)
 * ...
 * ```
 *
 * ### VRAM layout used by TCS-TES I/O:
 *
 * ```
 * attr 0 of patch 0 vertex 0   <─── "off-chip LDS" offset
 * attr 0 of patch 0 vertex 1
 * attr 0 of patch 0 vertex 2
 * ...
 * attr 0 of patch 1 vertex 0
 * attr 0 of patch 1 vertex 1
 * attr 0 of patch 1 vertex 2   <─── hs_per_vertex_output_vmem_offset (attribute slot = 0, rel_patch_id = 1, vertex index = 1)
 * ...
 * attr 0 of patch 2 vertex 0
 * attr 0 of patch 2 vertex 1
 * attr 0 of patch 2 vertex 2
 * ...
 * attr 1 of patch 0 vertex 0
 * attr 1 of patch 0 vertex 1
 * attr 1 of patch 0 vertex 2
 * ...
 * ...
 * per-patch attr 0 of patch 0  <─── hs_out_patch_data_offset_amd
 * per-patch attr 0 of patch 1
 * per-patch attr 0 of patch 2  <─── hs_per_patch_output_vmem_offset (attribute slot = 0, rel_patch_id = 2)
 * ...
 * per-patch attr 1 of patch 0
 * per-patch attr 1 of patch 1
 * per-patch attr 1 of patch 2
 * ...
 * ```
 *
 */

typedef struct {
   /* Which hardware generation we're dealing with */
   enum amd_gfx_level gfx_level;

   /* I/O semantic -> real location used by lowering. */
   ac_nir_map_io_driver_location map_io;

   /* True if merged VS+TCS (on GFX9+) has the same number
    * of input and output patch size.
    */
   bool tcs_in_out_eq;

   /* Bit mask of TCS per-vertex inputs (VS outputs) which
    * are passed between the two stages only in temporaries (registers).
    */
   uint64_t tcs_temp_only_inputs;

   /* Bit mask of TCS outputs read by TES. */
   uint64_t tes_inputs_read;
   uint64_t tes_patch_inputs_read;

   /* Whether TES reads the tess factors. */
   bool tes_reads_tessfactors;

   unsigned tcs_num_reserved_outputs;
   unsigned tcs_num_reserved_patch_outputs;

   /* Location (slot) where tessellation levels are stored. */
   int tcs_tess_lvl_in_loc;
   int tcs_tess_lvl_out_loc;

   /* True if the output patch fits the subgroup, so all TCS outputs are always written in the same
    * subgroup that reads them.
    */
   bool tcs_out_patch_fits_subgroup;

   /* Set if all invocations will write to all tess factors, so tess factors
    * can be passed by register.
    */
   bool tcs_pass_tessfactors_by_reg;

   /* Whether all TCS inputs are accessed using gl_InvocationID and passed via VGPRs.
    * In that case, no LDS is allocated for TCS inputs.
    */
   bool tcs_no_inputs_in_lds;

   /* Save TCS tess factor for tess factor writer. */
   nir_variable *tcs_tess_level_outer;
   nir_variable *tcs_tess_level_inner;
   unsigned tcs_tess_level_outer_base;
   unsigned tcs_tess_level_outer_mask;
   unsigned tcs_tess_level_inner_base;
   unsigned tcs_tess_level_inner_mask;
} lower_tess_io_state;

static bool
match_mask(gl_shader_stage stage,
           nir_intrinsic_instr *intrin,
           uint64_t mask,
           bool match_indirect)
{
   bool indirect = !nir_src_is_const(*nir_get_io_offset_src(intrin));
   if (indirect)
      return match_indirect;

   uint64_t slot = nir_intrinsic_io_semantics(intrin).location;
   if (stage == MESA_SHADER_TESS_CTRL &&
       intrin->intrinsic != nir_intrinsic_load_per_vertex_input &&
       intrin->intrinsic != nir_intrinsic_store_per_vertex_output)
      slot -= VARYING_SLOT_PATCH0;

   return (UINT64_C(1) << slot) & mask;
}

static bool
tcs_output_needs_vmem(nir_intrinsic_instr *intrin,
                      lower_tess_io_state *st)
{
   uint64_t mask = intrin->intrinsic == nir_intrinsic_store_per_vertex_output
                   ? st->tes_inputs_read
                   : st->tes_patch_inputs_read;

   return match_mask(MESA_SHADER_TESS_CTRL, intrin, mask, true);
}

static bool
tcs_output_needs_lds(nir_intrinsic_instr *intrin,
                     nir_shader *shader)
{
   uint64_t mask = intrin->intrinsic == nir_intrinsic_store_per_vertex_output
                   ? shader->info.outputs_read
                   : shader->info.patch_outputs_read;

   return match_mask(MESA_SHADER_TESS_CTRL, intrin, mask, true);
}

static bool
lower_ls_output_store(nir_builder *b,
                      nir_intrinsic_instr *intrin,
                      void *state)
{
   if (intrin->intrinsic != nir_intrinsic_store_output)
      return false;

   /* The ARB_shader_viewport_layer_array spec contains the
    * following issue:
    *
    *    2) What happens if gl_ViewportIndex or gl_Layer is
    *    written in the vertex shader and a geometry shader is
    *    present?
    *
    *    RESOLVED: The value written by the last vertex processing
    *    stage is used. If the last vertex processing stage
    *    (vertex, tessellation evaluation or geometry) does not
    *    statically assign to gl_ViewportIndex or gl_Layer, index
    *    or layer zero is assumed.
    *
    * So writes to those outputs in VS-as-LS are simply ignored.
    */
   unsigned semantic = nir_intrinsic_io_semantics(intrin).location;
   if (semantic == VARYING_SLOT_LAYER || semantic == VARYING_SLOT_VIEWPORT) {
      nir_instr_remove(&intrin->instr);
      return true;
   }

   lower_tess_io_state *st = (lower_tess_io_state *) state;

   /* If this is a temp-only TCS input, we don't need to use shared memory at all. */
   if (match_mask(MESA_SHADER_VERTEX, intrin, st->tcs_temp_only_inputs, false))
      return false;

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *vertex_idx = nir_load_local_invocation_index(b);
   nir_def *base_off_var = nir_imul(b, vertex_idx, nir_load_lshs_vertex_stride_amd(b));

   nir_def *io_off = ac_nir_calc_io_offset(b, intrin, nir_imm_int(b, 16u), 4u, st->map_io);
   unsigned write_mask = nir_intrinsic_write_mask(intrin);

   nir_def *off = nir_iadd_nuw(b, base_off_var, io_off);
   nir_store_shared(b, intrin->src[0].ssa, off, .write_mask = write_mask);

   /* NOTE: don't remove the store_output intrinsic on GFX9+ when tcs_in_out_eq,
    * it will be used by same-invocation TCS input loads.
    */
   if (!st->tcs_in_out_eq)
      nir_instr_remove(&intrin->instr);

   return true;
}

static bool
filter_load_tcs_per_vertex_input(const nir_instr *instr,
                                 UNUSED const void *state)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   lower_tess_io_state *st = (lower_tess_io_state *) state;
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   if (intrin->intrinsic != nir_intrinsic_load_per_vertex_input)
      return false;
   if (!st->tcs_in_out_eq)
      return true;

   /* tcs_in_out_eq: a same-invocation input load, without indirect offset,
    * can use temporaries, no need to use shared memory.
    */
   nir_src *off_src = nir_get_io_offset_src(intrin);
   nir_src *vertex_index_src = nir_get_io_arrayed_index_src(intrin);
   nir_instr *vertex_index_instr = vertex_index_src->ssa->parent_instr;

   bool can_use_temps = nir_src_is_const(*off_src) &&
                        vertex_index_instr->type == nir_instr_type_intrinsic &&
                        nir_instr_as_intrinsic(vertex_index_instr)->intrinsic == nir_intrinsic_load_invocation_id;

   return !can_use_temps;
}

static nir_def *
hs_per_vertex_input_lds_offset(nir_builder *b,
                               lower_tess_io_state *st,
                               nir_intrinsic_instr *instr)
{
   nir_def *tcs_in_vtxcnt = nir_load_patch_vertices_in(b);
   nir_def *rel_patch_id = nir_load_tess_rel_patch_id_amd(b);
   nir_def *vertex_index = nir_get_io_arrayed_index_src(instr)->ssa;

   nir_def *stride = nir_load_lshs_vertex_stride_amd(b);
   nir_def *tcs_in_patch_stride = nir_imul(b, tcs_in_vtxcnt, stride);
   nir_def *vertex_index_off = nir_imul(b, vertex_index, stride);

   nir_def *tcs_in_current_patch_offset = nir_imul(b, rel_patch_id, tcs_in_patch_stride);

   nir_def *io_offset = ac_nir_calc_io_offset(b, instr, nir_imm_int(b, 16u), 4u, st->map_io);

   return nir_iadd_nuw(b, nir_iadd_nuw(b, tcs_in_current_patch_offset, vertex_index_off), io_offset);
}

static nir_def *
hs_output_lds_offset(nir_builder *b,
                     lower_tess_io_state *st,
                     nir_intrinsic_instr *intrin)
{
   bool per_vertex = intrin &&
                     (intrin->intrinsic == nir_intrinsic_store_per_vertex_output ||
                      intrin->intrinsic == nir_intrinsic_load_per_vertex_output);

   unsigned output_vertex_size = st->tcs_num_reserved_outputs * 16u;
   unsigned pervertex_output_patch_size = b->shader->info.tess.tcs_vertices_out * output_vertex_size;
   unsigned output_patch_stride = pervertex_output_patch_size + st->tcs_num_reserved_patch_outputs * 16u;

   nir_def *off = intrin
                    ? ac_nir_calc_io_offset(b, intrin, nir_imm_int(b, 16u), 4u, st->map_io)
                    : nir_imm_int(b, 0);

   nir_def *rel_patch_id = nir_load_tess_rel_patch_id_amd(b);
   nir_def *patch_offset = nir_imul_imm(b, rel_patch_id, output_patch_stride);

   nir_def *output_patch_offset;
   if (st->tcs_no_inputs_in_lds)
      output_patch_offset = patch_offset;
   else {
      nir_def *tcs_in_vtxcnt = nir_load_patch_vertices_in(b);
      nir_def *tcs_num_patches = nir_load_tcs_num_patches_amd(b);
      nir_def *input_patch_size =
         nir_imul(b, tcs_in_vtxcnt, nir_load_lshs_vertex_stride_amd(b));
      nir_def *output_patch0_offset = nir_imul(b, input_patch_size, tcs_num_patches);
      output_patch_offset = nir_iadd_nuw(b, patch_offset, output_patch0_offset);
   }

   if (per_vertex) {
      nir_def *vertex_index = nir_get_io_arrayed_index_src(intrin)->ssa;
      nir_def *vertex_index_off = nir_imul_imm(b, vertex_index, output_vertex_size);

      off = nir_iadd_nuw(b, off, vertex_index_off);
      return nir_iadd_nuw(b, off, output_patch_offset);
   } else {
      off = nir_iadd_imm_nuw(b, off, pervertex_output_patch_size);
      return nir_iadd_nuw(b, off, output_patch_offset);
   }
}

static nir_def *
hs_per_vertex_output_vmem_offset(nir_builder *b,
                                 lower_tess_io_state *st,
                                 nir_intrinsic_instr *intrin)
{
   nir_def *out_vertices_per_patch = b->shader->info.stage == MESA_SHADER_TESS_CTRL
                                         ? nir_imm_int(b, b->shader->info.tess.tcs_vertices_out)
                                         : nir_load_patch_vertices_in(b);

   nir_def *tcs_num_patches = nir_load_tcs_num_patches_amd(b);
   nir_def *attr_stride = nir_imul(b, tcs_num_patches, nir_imul_imm(b, out_vertices_per_patch, 16u));
   nir_def *io_offset = ac_nir_calc_io_offset(b, intrin, attr_stride, 4u, st->map_io);

   nir_def *rel_patch_id = nir_load_tess_rel_patch_id_amd(b);
   nir_def *patch_offset = nir_imul(b, rel_patch_id, nir_imul_imm(b, out_vertices_per_patch, 16u));

   nir_def *vertex_index = nir_get_io_arrayed_index_src(intrin)->ssa;
   nir_def *vertex_index_off = nir_imul_imm(b, vertex_index, 16u);

   return nir_iadd_nuw(b, nir_iadd_nuw(b, patch_offset, vertex_index_off), io_offset);
}

static nir_def *
hs_per_patch_output_vmem_offset(nir_builder *b,
                                lower_tess_io_state *st,
                                nir_intrinsic_instr *intrin,
                                unsigned const_base_offset)
{
   nir_def *tcs_num_patches = nir_load_tcs_num_patches_amd(b);
   nir_def *per_patch_data_offset = nir_load_hs_out_patch_data_offset_amd(b);

   nir_def * off = intrin
                    ? ac_nir_calc_io_offset(b, intrin, nir_imul_imm(b, tcs_num_patches, 16u), 4u, st->map_io)
                    : nir_imm_int(b, 0);

   if (const_base_offset)
      off = nir_iadd_nuw(b, off, nir_imul_imm(b, tcs_num_patches, const_base_offset));

   nir_def *rel_patch_id = nir_load_tess_rel_patch_id_amd(b);
   nir_def *patch_offset = nir_imul_imm(b, rel_patch_id, 16u);
   off = nir_iadd_nuw(b, off, per_patch_data_offset);
   return nir_iadd_nuw(b, off, patch_offset);
}

static nir_def *
lower_hs_per_vertex_input_load(nir_builder *b,
                               nir_instr *instr,
                               void *state)
{
   lower_tess_io_state *st = (lower_tess_io_state *) state;
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   nir_def *off = hs_per_vertex_input_lds_offset(b, st, intrin);
   return nir_load_shared(b, intrin->def.num_components, intrin->def.bit_size, off);
}

static nir_def *
lower_hs_output_store(nir_builder *b,
                      nir_intrinsic_instr *intrin,
                      lower_tess_io_state *st)
{
   assert(intrin->intrinsic == nir_intrinsic_store_per_vertex_output ||
          intrin->intrinsic == nir_intrinsic_store_output);

   nir_io_semantics semantics = nir_intrinsic_io_semantics(intrin);
   nir_def *store_val = intrin->src[0].ssa;
   unsigned base = nir_intrinsic_base(intrin);
   unsigned component = nir_intrinsic_component(intrin);
   unsigned write_mask = nir_intrinsic_write_mask(intrin);
   bool is_tess_factor = semantics.location == VARYING_SLOT_TESS_LEVEL_INNER ||
                         semantics.location == VARYING_SLOT_TESS_LEVEL_OUTER;
   bool write_to_vmem = !is_tess_factor && tcs_output_needs_vmem(intrin, st);
   bool write_to_lds = (is_tess_factor && !st->tcs_pass_tessfactors_by_reg) ||
      tcs_output_needs_lds(intrin, b->shader);

   /* Remember tess factor location so that we can load them from LDS and/or
    * store them to VMEM when hs_emit_write_tess_factors().
    */
   if (is_tess_factor) {
      unsigned mapped_location =
         st->map_io ? st->map_io(semantics.location) : nir_intrinsic_base(intrin);

      if (semantics.location == VARYING_SLOT_TESS_LEVEL_INNER)
         st->tcs_tess_lvl_in_loc = mapped_location * 16u;
      else
         st->tcs_tess_lvl_out_loc = mapped_location * 16u;
   }

   if (write_to_vmem) {
      nir_def *vmem_off = intrin->intrinsic == nir_intrinsic_store_per_vertex_output
                            ? hs_per_vertex_output_vmem_offset(b, st, intrin)
                            : hs_per_patch_output_vmem_offset(b, st, intrin, 0);

      nir_def *hs_ring_tess_offchip = nir_load_ring_tess_offchip_amd(b);
      nir_def *offchip_offset = nir_load_ring_tess_offchip_offset_amd(b);
      nir_def *zero = nir_imm_int(b, 0);
      nir_store_buffer_amd(b, store_val, hs_ring_tess_offchip, vmem_off, offchip_offset, zero,
                           .write_mask = write_mask, .memory_modes = nir_var_shader_out,
                           .access = ACCESS_COHERENT);
   }

   if (write_to_lds) {
      nir_def *lds_off = hs_output_lds_offset(b, st, intrin);
      nir_store_shared(b, store_val, lds_off, .write_mask = write_mask);
   }

   /* Save tess factor to be used by tess factor writer or reconstruct
    * store output instruction later.
    */
   if (is_tess_factor && st->tcs_pass_tessfactors_by_reg) {
      if (semantics.location == VARYING_SLOT_TESS_LEVEL_INNER) {
         st->tcs_tess_level_inner_base = base;
         st->tcs_tess_level_inner_mask |= write_mask << component;
         ac_nir_store_var_components(b, st->tcs_tess_level_inner, store_val,
                                     component, write_mask);
      } else {
         st->tcs_tess_level_outer_base = base;
         st->tcs_tess_level_outer_mask |= write_mask << component;
         ac_nir_store_var_components(b, st->tcs_tess_level_outer, store_val,
                                     component, write_mask);
      }
   }

   return NIR_LOWER_INSTR_PROGRESS_REPLACE;
}

static nir_def *
lower_hs_output_load(nir_builder *b,
                     nir_intrinsic_instr *intrin,
                     lower_tess_io_state *st)
{
   nir_def *off = hs_output_lds_offset(b, st, intrin);
   return nir_load_shared(b, intrin->def.num_components, intrin->def.bit_size, off);
}

static void
update_hs_barrier(nir_intrinsic_instr *intrin, lower_tess_io_state *st)
{
   /* Output loads and stores are lowered to shared memory access,
    * so we have to update the barriers to also reflect this.
    */
   unsigned mem_modes = nir_intrinsic_memory_modes(intrin);
   if (mem_modes & nir_var_shader_out) {
      mem_modes |= nir_var_mem_shared;
      mem_modes &= ~nir_var_shader_out;
   }
   nir_intrinsic_set_memory_modes(intrin, mem_modes);

   mesa_scope exec_scope = nir_intrinsic_execution_scope(intrin);
   if (exec_scope == SCOPE_WORKGROUP && st->tcs_out_patch_fits_subgroup)
      nir_intrinsic_set_execution_scope(intrin, SCOPE_SUBGROUP);

   mesa_scope mem_scope = nir_intrinsic_memory_scope(intrin);
   if (mem_scope == SCOPE_WORKGROUP && st->tcs_out_patch_fits_subgroup)
      nir_intrinsic_set_memory_scope(intrin, SCOPE_SUBGROUP);
}

static nir_def *
lower_hs_output_access(nir_builder *b,
                       nir_instr *instr,
                       void *state)
{
   lower_tess_io_state *st = (lower_tess_io_state *) state;
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   if (intrin->intrinsic == nir_intrinsic_store_output ||
       intrin->intrinsic == nir_intrinsic_store_per_vertex_output) {
      return lower_hs_output_store(b, intrin, st);
   } else if (intrin->intrinsic == nir_intrinsic_load_output ||
              intrin->intrinsic == nir_intrinsic_load_per_vertex_output) {
      return lower_hs_output_load(b, intrin, st);
   } else if (intrin->intrinsic == nir_intrinsic_barrier) {
      update_hs_barrier(intrin, st);
      return NIR_LOWER_INSTR_PROGRESS;
   } else {
      unreachable("intrinsic not supported by lower_hs_output_access");
   }
}

static void
hs_emit_write_tess_factors(nir_shader *shader,
                           lower_tess_io_state *st)
{
   unsigned outer_comps;
   unsigned inner_comps;

   switch (shader->info.tess._primitive_mode) {
   case TESS_PRIMITIVE_ISOLINES:
      outer_comps = 2;
      inner_comps = 0;
      break;
   case TESS_PRIMITIVE_TRIANGLES:
      outer_comps = 3;
      inner_comps = 1;
      break;
   case TESS_PRIMITIVE_QUADS:
      outer_comps = 4;
      inner_comps = 2;
      break;
   default:
      unreachable("invalid primitive mode");
      return;
   }

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);
   nir_block *last_block = nir_impl_last_block(impl);
   assert(last_block);

   /* We assume there is always a single end block in the shader. */

   nir_builder builder = nir_builder_at(nir_after_block(last_block));
   nir_builder *b = &builder; /* This is to avoid the & */

   /* If tess factors are load from LDS, wait previous LDS stores done. */
   if (!st->tcs_pass_tessfactors_by_reg) {
      mesa_scope scope = st->tcs_out_patch_fits_subgroup ?
                        SCOPE_SUBGROUP : SCOPE_WORKGROUP;

      nir_barrier(b, .execution_scope = scope, .memory_scope = scope,
                         .memory_semantics = NIR_MEMORY_ACQ_REL, .memory_modes = nir_var_mem_shared);
   }

   nir_def *invocation_id = nir_load_invocation_id(b);

   /* Only the 1st invocation of each patch needs to do this. */
   nir_if *invocation_id_zero = nir_push_if(b, nir_ieq_imm(b, invocation_id, 0));

   /* When the output patch size is <= 32 then we can flatten the branch here
    * because we know for sure that at least 1 invocation in all waves will
    * take the branch.
    */
   if (shader->info.tess.tcs_vertices_out <= 32)
      invocation_id_zero->control = nir_selection_control_divergent_always_taken;

   const bool tess_lvl_in_written = st->tcs_tess_lvl_in_loc >= 0;
   const bool tess_lvl_out_written = st->tcs_tess_lvl_out_loc >= 0;

   nir_def *tessfactors_outer = NULL;
   nir_def *tessfactors_inner = NULL;
   if (st->tcs_pass_tessfactors_by_reg) {
      if (tess_lvl_out_written) {
         tessfactors_outer = nir_load_var(b, st->tcs_tess_level_outer);
         tessfactors_outer = nir_trim_vector(b, tessfactors_outer, outer_comps);
      }

      if (inner_comps && tess_lvl_in_written) {
         tessfactors_inner = nir_load_var(b, st->tcs_tess_level_inner);
         tessfactors_inner = nir_trim_vector(b, tessfactors_inner, inner_comps);
      }
   } else {
      /* Base LDS address of per-patch outputs in the current patch. */
      nir_def *lds_base = hs_output_lds_offset(b, st, NULL);

      /* Load all tessellation factors (aka. tess levels) from LDS. */
      if (tess_lvl_out_written) {
         tessfactors_outer = nir_load_shared(b, outer_comps, 32, lds_base,
                                             .base = st->tcs_tess_lvl_out_loc);
      }

      if (inner_comps && tess_lvl_in_written) {
         tessfactors_inner = nir_load_shared(b, inner_comps, 32, lds_base,
                                             .base = st->tcs_tess_lvl_in_loc);
      }
   }

   /* Set tess factor to be zero if user did not write them. */
   if (!tessfactors_outer)
      tessfactors_outer = nir_imm_zero(b, outer_comps, 32);
   if (inner_comps && !tessfactors_inner)
      tessfactors_inner = nir_imm_zero(b, inner_comps, 32);

   /* The descriptor where tess factors have to be stored by the shader. */
   nir_def *tessfactor_ring = nir_load_ring_tess_factors_amd(b);

   nir_def *zero = nir_imm_int(b, 0);
   nir_def *rel_patch_id = nir_load_tess_rel_patch_id_amd(b);
   nir_def *tess_factors_base = nir_load_ring_tess_factors_offset_amd(b);
   nir_def *tess_factors_offset = nir_imul_imm(b, rel_patch_id, (inner_comps + outer_comps) * 4u);
   unsigned tess_factors_const_offset = 0;

   if (st->gfx_level <= GFX8) {
      /* Store the dynamic HS control word. */
      nir_if *rel_patch_id_zero = nir_push_if(b, nir_ieq_imm(b, rel_patch_id, 0));
      nir_def *ctrlw = nir_imm_int(b, 0x80000000u);
      nir_store_buffer_amd(b, ctrlw, tessfactor_ring, zero, tess_factors_base, zero,
                           .access = ACCESS_COHERENT);
      tess_factors_const_offset += 4;
      nir_pop_if(b, rel_patch_id_zero);
   }

   /* Store tess factors for the tessellator */
   if (shader->info.tess._primitive_mode == TESS_PRIMITIVE_ISOLINES) {
      /* LINES reversal */
      nir_def *t = nir_vec2(b, nir_channel(b, tessfactors_outer, 1), nir_channel(b, tessfactors_outer, 0));
      nir_store_buffer_amd(b, t, tessfactor_ring, tess_factors_offset, tess_factors_base, zero,
                           .base = tess_factors_const_offset, .access = ACCESS_COHERENT);
   } else if (shader->info.tess._primitive_mode == TESS_PRIMITIVE_TRIANGLES) {
      nir_def *t = nir_vec4(b, nir_channel(b, tessfactors_outer, 0), nir_channel(b, tessfactors_outer, 1),
                                nir_channel(b, tessfactors_outer, 2), nir_channel(b, tessfactors_inner, 0));
      nir_store_buffer_amd(b, t, tessfactor_ring, tess_factors_offset, tess_factors_base, zero,
                           .base = tess_factors_const_offset, .access = ACCESS_COHERENT);
   } else {
      nir_store_buffer_amd(b, tessfactors_outer, tessfactor_ring, tess_factors_offset, tess_factors_base, zero,
                           .base = tess_factors_const_offset, .access = ACCESS_COHERENT);
      nir_store_buffer_amd(b, tessfactors_inner, tessfactor_ring, tess_factors_offset, tess_factors_base, zero,
                           .base = tess_factors_const_offset + 4u * outer_comps, .access = ACCESS_COHERENT);
   }

   if (st->tes_reads_tessfactors) {
      /* Store to offchip for TES to read - only if TES actually reads them */
      nir_def *hs_ring_tess_offchip = nir_load_ring_tess_offchip_amd(b);
      nir_def *offchip_offset = nir_load_ring_tess_offchip_offset_amd(b);

      if (tess_lvl_out_written) {
         nir_def *vmem_off_outer =
            hs_per_patch_output_vmem_offset(b, st, NULL, st->tcs_tess_lvl_out_loc);

         nir_store_buffer_amd(b, tessfactors_outer, hs_ring_tess_offchip,
                              vmem_off_outer, offchip_offset, zero,
                              .memory_modes = nir_var_shader_out,
                              .access = ACCESS_COHERENT);
      }

      if (inner_comps && tess_lvl_in_written) {
         nir_def *vmem_off_inner =
            hs_per_patch_output_vmem_offset(b, st, NULL, st->tcs_tess_lvl_in_loc);

         nir_store_buffer_amd(b, tessfactors_inner, hs_ring_tess_offchip,
                              vmem_off_inner, offchip_offset, zero,
                              .memory_modes = nir_var_shader_out,
                              .access = ACCESS_COHERENT);
      }
   }

   nir_pop_if(b, invocation_id_zero);

   nir_metadata_preserve(impl, nir_metadata_none);
}

static nir_def *
lower_tes_input_load(nir_builder *b,
                     nir_instr *instr,
                     void *state)
{
   lower_tess_io_state *st = (lower_tess_io_state *) state;
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   nir_def *offchip_ring = nir_load_ring_tess_offchip_amd(b);
   nir_def *offchip_offset = nir_load_ring_tess_offchip_offset_amd(b);
   nir_def *off = intrin->intrinsic == nir_intrinsic_load_per_vertex_input
                    ? hs_per_vertex_output_vmem_offset(b, st, intrin)
                    : hs_per_patch_output_vmem_offset(b, st, intrin, 0);

   nir_def *zero = nir_imm_int(b, 0);

   return nir_load_buffer_amd(b, intrin->def.num_components,
                              intrin->def.bit_size, offchip_ring,
                              off, offchip_offset, zero,
                              .access = ACCESS_COHERENT);
}

static bool
filter_hs_output_access(const nir_instr *instr,
                         UNUSED const void *st)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   return intrin->intrinsic == nir_intrinsic_store_output ||
          intrin->intrinsic == nir_intrinsic_store_per_vertex_output ||
          intrin->intrinsic == nir_intrinsic_load_output ||
          intrin->intrinsic == nir_intrinsic_load_per_vertex_output ||
          intrin->intrinsic == nir_intrinsic_barrier;
}

static bool
filter_any_input_access(const nir_instr *instr,
                        UNUSED const void *st)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   return intrin->intrinsic == nir_intrinsic_load_input ||
          intrin->intrinsic == nir_intrinsic_load_per_vertex_input;
}

void
ac_nir_lower_ls_outputs_to_mem(nir_shader *shader,
                               ac_nir_map_io_driver_location map,
                               bool tcs_in_out_eq,
                               uint64_t tcs_temp_only_inputs)
{
   assert(shader->info.stage == MESA_SHADER_VERTEX);

   lower_tess_io_state state = {
      .tcs_in_out_eq = tcs_in_out_eq,
      .tcs_temp_only_inputs = tcs_in_out_eq ? tcs_temp_only_inputs : 0,
      .map_io = map,
   };

   nir_shader_intrinsics_pass(shader, lower_ls_output_store,
                                nir_metadata_block_index | nir_metadata_dominance,
                                &state);
}

void
ac_nir_lower_hs_inputs_to_mem(nir_shader *shader,
                              ac_nir_map_io_driver_location map,
                              bool tcs_in_out_eq)
{
   assert(shader->info.stage == MESA_SHADER_TESS_CTRL);

   lower_tess_io_state state = {
      .tcs_in_out_eq = tcs_in_out_eq,
      .map_io = map,
   };

   nir_shader_lower_instructions(shader,
                                 filter_load_tcs_per_vertex_input,
                                 lower_hs_per_vertex_input_load,
                                 &state);
}

static void
reconstruct_tess_factor_outputs(nir_shader *shader, lower_tess_io_state *st)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   nir_builder builder = nir_builder_create(impl);
   nir_builder *b = &builder;
   b->cursor = nir_after_impl(impl);

   if (st->tcs_tess_level_outer_mask) {
      nir_def *val = nir_load_var(b, st->tcs_tess_level_outer);
      nir_store_output(b, val, nir_imm_int(b, 0),
                       .base = st->tcs_tess_level_outer_base,
                       .write_mask = st->tcs_tess_level_outer_mask,
                       .io_semantics.location = VARYING_SLOT_TESS_LEVEL_OUTER);
   }

   if (st->tcs_tess_level_inner_mask) {
      nir_def *val = nir_load_var(b, st->tcs_tess_level_inner);
      nir_store_output(b, val, nir_imm_int(b, 0),
                       .base = st->tcs_tess_level_inner_base,
                       .write_mask = st->tcs_tess_level_inner_mask,
                       .io_semantics.location = VARYING_SLOT_TESS_LEVEL_INNER);
   }
}

void
ac_nir_lower_hs_outputs_to_mem(nir_shader *shader,
                               ac_nir_map_io_driver_location map,
                               enum amd_gfx_level gfx_level,
                               bool tes_reads_tessfactors,
                               uint64_t tes_inputs_read,
                               uint64_t tes_patch_inputs_read,
                               unsigned num_reserved_tcs_outputs,
                               unsigned num_reserved_tcs_patch_outputs,
                               unsigned wave_size,
                               bool no_inputs_in_lds,
                               bool pass_tessfactors_by_reg,
                               bool emit_tess_factor_write)
{
   assert(shader->info.stage == MESA_SHADER_TESS_CTRL);

   lower_tess_io_state state = {
      .gfx_level = gfx_level,
      .tes_reads_tessfactors = tes_reads_tessfactors,
      .tes_inputs_read = tes_inputs_read,
      .tes_patch_inputs_read = tes_patch_inputs_read,
      .tcs_num_reserved_outputs = num_reserved_tcs_outputs,
      .tcs_num_reserved_patch_outputs = num_reserved_tcs_patch_outputs,
      .tcs_out_patch_fits_subgroup = wave_size % shader->info.tess.tcs_vertices_out == 0,
      .tcs_pass_tessfactors_by_reg = pass_tessfactors_by_reg,
      .tcs_no_inputs_in_lds = no_inputs_in_lds,
      .tcs_tess_lvl_in_loc = -1,
      .tcs_tess_lvl_out_loc = -1,
      .map_io = map,
   };

   if (pass_tessfactors_by_reg) {
      nir_function_impl *impl = nir_shader_get_entrypoint(shader);
      state.tcs_tess_level_outer =
         nir_local_variable_create(impl, glsl_vec4_type(), "tess outer");
      state.tcs_tess_level_inner =
         nir_local_variable_create(impl, glsl_vec4_type(), "tess inner");
   }

   nir_shader_lower_instructions(shader,
                                 filter_hs_output_access,
                                 lower_hs_output_access,
                                 &state);

   if (emit_tess_factor_write) {
      hs_emit_write_tess_factors(shader, &state);
   } else if (pass_tessfactors_by_reg) {
      /* Reconstruct tess factor nir_store_output instruction if it's going to be passed
       * by reg instead of LDS and we use a compiler backend TCS epilog.
       *
       * TCS does not call nir_lower_io_to_temporaries(). It's not a problem when LLVM
       * because LLVM support variable. But ACO does not support variable, so we do similar
       * thing as nir_lower_io_to_temporaries() to move store output instruction out of
       * control flow.
       */
      reconstruct_tess_factor_outputs(shader, &state);
   }
}

void
ac_nir_lower_tes_inputs_to_mem(nir_shader *shader,
                               ac_nir_map_io_driver_location map)
{
   assert(shader->info.stage == MESA_SHADER_TESS_EVAL);

   lower_tess_io_state state = {
      .map_io = map,
   };

   nir_shader_lower_instructions(shader,
                                 filter_any_input_access,
                                 lower_tes_input_load,
                                 &state);
}
