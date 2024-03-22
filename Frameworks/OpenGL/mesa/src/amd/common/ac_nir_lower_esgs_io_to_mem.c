/*
 * Copyright © 2021 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_nir.h"
#include "nir_builder.h"

/*
 * Lower NIR cross-stage I/O intrinsics into the memory accesses that actually happen on the HW.
 *
 * These HW stages are used only when a Geometry Shader is used.
 * Export Shader (ES) runs the SW stage before GS, can be either VS or TES.
 *
 * * GFX6-8:
 *   ES and GS are separate HW stages.
 *   I/O is passed between them through VRAM.
 * * GFX9+:
 *   ES and GS are merged into a single HW stage.
 *   I/O is passed between them through LDS.
 *
 */

typedef struct {
   /* Which hardware generation we're dealing with */
   enum amd_gfx_level gfx_level;

   /* I/O semantic -> real location used by lowering. */
   ac_nir_map_io_driver_location map_io;

   /* Stride of an ES invocation outputs in esgs ring, in bytes. */
   unsigned esgs_itemsize;

   /* Enable fix for triangle strip adjacency in geometry shader. */
   bool gs_triangle_strip_adjacency_fix;
} lower_esgs_io_state;

static nir_def *
emit_split_buffer_load(nir_builder *b, nir_def *desc, nir_def *v_off, nir_def *s_off,
                       unsigned component_stride, unsigned num_components, unsigned bit_size)
{
   unsigned total_bytes = num_components * bit_size / 8u;
   unsigned full_dwords = total_bytes / 4u;
   unsigned remaining_bytes = total_bytes - full_dwords * 4u;

   /* Accommodate max number of split 64-bit loads */
   nir_def *comps[NIR_MAX_VEC_COMPONENTS * 2u];

   /* Assume that 1x32-bit load is better than 1x16-bit + 1x8-bit */
   if (remaining_bytes == 3) {
      remaining_bytes = 0;
      full_dwords++;
   }

   nir_def *zero = nir_imm_int(b, 0);

   for (unsigned i = 0; i < full_dwords; ++i)
      comps[i] = nir_load_buffer_amd(b, 1, 32, desc, v_off, s_off, zero,
                                     .base = component_stride * i, .memory_modes = nir_var_shader_in,
                                     .access = ACCESS_COHERENT);

   if (remaining_bytes)
      comps[full_dwords] = nir_load_buffer_amd(b, 1, remaining_bytes * 8, desc, v_off, s_off, zero,
                                               .base = component_stride * full_dwords,
                                               .memory_modes = nir_var_shader_in,
                                               .access = ACCESS_COHERENT);

   return nir_extract_bits(b, comps, full_dwords + !!remaining_bytes, 0, num_components, bit_size);
}

static void
emit_split_buffer_store(nir_builder *b, nir_def *d, nir_def *desc, nir_def *v_off, nir_def *s_off,
                        unsigned component_stride, unsigned num_components, unsigned bit_size,
                        unsigned writemask, bool swizzled, bool slc)
{
   nir_def *zero = nir_imm_int(b, 0);

   while (writemask) {
      int start, count;
      u_bit_scan_consecutive_range(&writemask, &start, &count);
      assert(start >= 0 && count >= 0);

      unsigned bytes = count * bit_size / 8u;
      unsigned start_byte = start * bit_size / 8u;

      while (bytes) {
         unsigned store_bytes = MIN2(bytes, 4u);
         if ((start_byte % 4) == 1 || (start_byte % 4) == 3)
            store_bytes = MIN2(store_bytes, 1);
         else if ((start_byte % 4) == 2)
            store_bytes = MIN2(store_bytes, 2);

         nir_def *store_val = nir_extract_bits(b, &d, 1, start_byte * 8u, 1, store_bytes * 8u);
         nir_store_buffer_amd(b, store_val, desc, v_off, s_off, zero,
                              .base = start_byte, .memory_modes = nir_var_shader_out,
                              .access = ACCESS_COHERENT |
                                        (slc ? ACCESS_NON_TEMPORAL : 0) |
                                        (swizzled ? ACCESS_IS_SWIZZLED_AMD : 0));

         start_byte += store_bytes;
         bytes -= store_bytes;
      }
   }
}

static bool
lower_es_output_store(nir_builder *b,
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
    * Vulkan spec 15.7 Built-In Variables:
    *
    *   The last active pre-rasterization shader stage (in pipeline order)
    *   controls the Layer that is used. Outputs in previous shader stages
    *   are not used, even if the last stage fails to write the Layer.
    *
    *   The last active pre-rasterization shader stage (in pipeline order)
    *   controls the ViewportIndex that is used. Outputs in previous shader
    *   stages are not used, even if the last stage fails to write the
    *   ViewportIndex.
    *
    * So writes to those outputs in ES are simply ignored.
    */
   unsigned semantic = nir_intrinsic_io_semantics(intrin).location;
   if (semantic == VARYING_SLOT_LAYER || semantic == VARYING_SLOT_VIEWPORT) {
      nir_instr_remove(&intrin->instr);
      return true;
   }

   lower_esgs_io_state *st = (lower_esgs_io_state *) state;
   unsigned write_mask = nir_intrinsic_write_mask(intrin);

   b->cursor = nir_before_instr(&intrin->instr);
   nir_def *io_off = ac_nir_calc_io_offset(b, intrin, nir_imm_int(b, 16u), 4u, st->map_io);

   if (st->gfx_level <= GFX8) {
      /* GFX6-8: ES is a separate HW stage, data is passed from ES to GS in VRAM. */
      nir_def *ring = nir_load_ring_esgs_amd(b);
      nir_def *es2gs_off = nir_load_ring_es2gs_offset_amd(b);
      emit_split_buffer_store(b, intrin->src[0].ssa, ring, io_off, es2gs_off, 4u,
                              intrin->src[0].ssa->num_components, intrin->src[0].ssa->bit_size,
                              write_mask, true, true);
   } else {
      /* GFX9+: ES is merged into GS, data is passed through LDS. */
      nir_def *vertex_idx = nir_load_local_invocation_index(b);
      nir_def *off = nir_iadd(b, nir_imul_imm(b, vertex_idx, st->esgs_itemsize), io_off);
      nir_store_shared(b, intrin->src[0].ssa, off, .write_mask = write_mask);
   }

   nir_instr_remove(&intrin->instr);
   return true;
}

static nir_def *
gs_get_vertex_offset(nir_builder *b, lower_esgs_io_state *st, unsigned vertex_index)
{
   nir_def *origin = nir_load_gs_vertex_offset_amd(b, .base = vertex_index);
   if (!st->gs_triangle_strip_adjacency_fix)
      return origin;

   unsigned fixed_index;
   if (st->gfx_level < GFX9) {
      /* Rotate vertex index by 2. */
      fixed_index = (vertex_index + 4) % 6;
   } else {
      /* This issue has been fixed for GFX10+ */
      assert(st->gfx_level == GFX9);
      /* 6 vertex offset are packed to 3 vgprs for GFX9+ */
      fixed_index = (vertex_index + 2) % 3;
   }
   nir_def *fixed = nir_load_gs_vertex_offset_amd(b, .base = fixed_index);

   nir_def *prim_id = nir_load_primitive_id(b);
   /* odd primitive id use fixed offset */
   nir_def *cond = nir_i2b(b, nir_iand_imm(b, prim_id, 1));
   return nir_bcsel(b, cond, fixed, origin);
}

static nir_def *
gs_per_vertex_input_vertex_offset_gfx6(nir_builder *b, lower_esgs_io_state *st,
                                       nir_src *vertex_src)
{
   if (nir_src_is_const(*vertex_src))
      return gs_get_vertex_offset(b, st, nir_src_as_uint(*vertex_src));

   nir_def *vertex_offset = gs_get_vertex_offset(b, st, 0);

   for (unsigned i = 1; i < b->shader->info.gs.vertices_in; ++i) {
      nir_def *cond = nir_ieq_imm(b, vertex_src->ssa, i);
      nir_def *elem = gs_get_vertex_offset(b, st, i);
      vertex_offset = nir_bcsel(b, cond, elem, vertex_offset);
   }

   return vertex_offset;
}

static nir_def *
gs_per_vertex_input_vertex_offset_gfx9(nir_builder *b, lower_esgs_io_state *st,
                                       nir_src *vertex_src)
{
   if (nir_src_is_const(*vertex_src)) {
      unsigned vertex = nir_src_as_uint(*vertex_src);
      return nir_ubfe_imm(b, gs_get_vertex_offset(b, st, vertex / 2u),
                          (vertex & 1u) * 16u, 16u);
   }

   nir_def *vertex_offset = gs_get_vertex_offset(b, st, 0);

   for (unsigned i = 1; i < b->shader->info.gs.vertices_in; i++) {
      nir_def *cond = nir_ieq_imm(b, vertex_src->ssa, i);
      nir_def *elem = gs_get_vertex_offset(b, st, i / 2u * 2u);
      if (i % 2u)
         elem = nir_ishr_imm(b, elem, 16u);

      vertex_offset = nir_bcsel(b, cond, elem, vertex_offset);
   }

   return nir_iand_imm(b, vertex_offset, 0xffffu);
}

static nir_def *
gs_per_vertex_input_offset(nir_builder *b,
                           lower_esgs_io_state *st,
                           nir_intrinsic_instr *instr)
{
   nir_src *vertex_src = nir_get_io_arrayed_index_src(instr);
   nir_def *vertex_offset = st->gfx_level >= GFX9
      ? gs_per_vertex_input_vertex_offset_gfx9(b, st, vertex_src)
      : gs_per_vertex_input_vertex_offset_gfx6(b, st, vertex_src);

   /* Gfx6-8 can't emulate VGT_ESGS_RING_ITEMSIZE because it uses the register to determine
    * the allocation size of the ESGS ring buffer in memory.
    */
   if (st->gfx_level >= GFX9)
      vertex_offset = nir_imul(b, vertex_offset, nir_load_esgs_vertex_stride_amd(b));

   unsigned base_stride = st->gfx_level >= GFX9 ? 1 : 64 /* Wave size on GFX6-8 */;
   nir_def *io_off = ac_nir_calc_io_offset(b, instr, nir_imm_int(b, base_stride * 4u), base_stride, st->map_io);
   nir_def *off = nir_iadd(b, io_off, vertex_offset);
   return nir_imul_imm(b, off, 4u);
}

static nir_def *
lower_gs_per_vertex_input_load(nir_builder *b,
                               nir_instr *instr,
                               void *state)
{
   lower_esgs_io_state *st = (lower_esgs_io_state *) state;
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   nir_def *off = gs_per_vertex_input_offset(b, st, intrin);

   if (st->gfx_level >= GFX9)
      return nir_load_shared(b, intrin->def.num_components, intrin->def.bit_size, off);

   unsigned wave_size = 64u; /* GFX6-8 only support wave64 */
   nir_def *ring = nir_load_ring_esgs_amd(b);
   return emit_split_buffer_load(b, ring, off, nir_imm_zero(b, 1, 32), 4u * wave_size,
                                 intrin->def.num_components, intrin->def.bit_size);
}

static bool
filter_load_per_vertex_input(const nir_instr *instr, UNUSED const void *state)
{
   return instr->type == nir_instr_type_intrinsic && nir_instr_as_intrinsic(instr)->intrinsic == nir_intrinsic_load_per_vertex_input;
}

void
ac_nir_lower_es_outputs_to_mem(nir_shader *shader,
                               ac_nir_map_io_driver_location map,
                               enum amd_gfx_level gfx_level,
                               unsigned esgs_itemsize)
{
   lower_esgs_io_state state = {
      .gfx_level = gfx_level,
      .esgs_itemsize = esgs_itemsize,
      .map_io = map,
   };

   nir_shader_intrinsics_pass(shader, lower_es_output_store,
                                nir_metadata_block_index | nir_metadata_dominance,
                                &state);
}

void
ac_nir_lower_gs_inputs_to_mem(nir_shader *shader,
                              ac_nir_map_io_driver_location map,
                              enum amd_gfx_level gfx_level,
                              bool triangle_strip_adjacency_fix)
{
   lower_esgs_io_state state = {
      .gfx_level = gfx_level,
      .map_io = map,
      .gs_triangle_strip_adjacency_fix = triangle_strip_adjacency_fix,
   };

   nir_shader_lower_instructions(shader,
                                 filter_load_per_vertex_input,
                                 lower_gs_per_vertex_input_load,
                                 &state);
}
