/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
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

#include "sfn_shader_gs.h"

#include "sfn_debug.h"
#include "sfn_instr_fetch.h"

namespace r600 {

GeometryShader::GeometryShader(const r600_shader_key& key):
    Shader("GS", key.gs.first_atomic_counter),
    m_tri_strip_adj_fix(key.gs.tri_strip_adj_fix)
{
}

bool
GeometryShader::do_scan_instruction(nir_instr *instr)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *ii = nir_instr_as_intrinsic(instr);

   switch (ii->intrinsic) {
   case nir_intrinsic_store_output:
      return process_store_output(ii);
   case nir_intrinsic_load_per_vertex_input:
      return process_load_input(ii);
   default:
      return false;
   }
}

bool
GeometryShader::process_store_output(nir_intrinsic_instr *instr)
{
   auto location = static_cast<gl_varying_slot>(nir_intrinsic_io_semantics(instr).location);
   auto index = nir_src_as_const_value(instr->src[1]);
   assert(index);

   auto driver_location = nir_intrinsic_base(instr) + index->u32;

   if (location == VARYING_SLOT_COL0 || location == VARYING_SLOT_COL1 ||
       (location >= VARYING_SLOT_VAR0 && location <= VARYING_SLOT_VAR31) ||
       (location >= VARYING_SLOT_TEX0 && location <= VARYING_SLOT_TEX7) ||
       location == VARYING_SLOT_BFC0 || location == VARYING_SLOT_BFC1 ||
       location == VARYING_SLOT_PNTC || location == VARYING_SLOT_CLIP_VERTEX ||
       location == VARYING_SLOT_CLIP_DIST0 || location == VARYING_SLOT_CLIP_DIST1 ||
       location == VARYING_SLOT_PRIMITIVE_ID || location == VARYING_SLOT_POS ||
       location == VARYING_SLOT_PSIZ || location == VARYING_SLOT_LAYER ||
       location == VARYING_SLOT_VIEWPORT || location == VARYING_SLOT_FOGC) {

      auto write_mask = nir_intrinsic_write_mask(instr);
      ShaderOutput output(driver_location, write_mask, location);

      if (nir_intrinsic_io_semantics(instr).no_varying)
         output.set_no_varying(true);
      if (nir_intrinsic_io_semantics(instr).location != VARYING_SLOT_CLIP_VERTEX)
         add_output(output);

      if (location == VARYING_SLOT_VIEWPORT) {
         m_out_viewport = true;
         m_out_misc_write = true;
      }

      if (location == VARYING_SLOT_CLIP_DIST0 || location == VARYING_SLOT_CLIP_DIST1) {
         auto write_mask = nir_intrinsic_write_mask(instr);
         m_cc_dist_mask |= write_mask << (4 * (location - VARYING_SLOT_CLIP_DIST0));
         m_clip_dist_write |= write_mask << (4 * (location - VARYING_SLOT_CLIP_DIST0));
      }

      if (m_noutputs <= driver_location &&
          nir_intrinsic_io_semantics(instr).location != VARYING_SLOT_CLIP_VERTEX)
         m_noutputs = driver_location + 1;

      return true;
   }
   return false;
}

bool
GeometryShader::process_load_input(nir_intrinsic_instr *instr)
{
   auto location = static_cast<gl_varying_slot>(nir_intrinsic_io_semantics(instr).location);
   auto index = nir_src_as_const_value(instr->src[1]);
   assert(index);

   auto driver_location = nir_intrinsic_base(instr) + index->u32;

   if (location == VARYING_SLOT_POS || location == VARYING_SLOT_PSIZ ||
       location == VARYING_SLOT_FOGC || location == VARYING_SLOT_CLIP_VERTEX ||
       location == VARYING_SLOT_CLIP_DIST0 || location == VARYING_SLOT_CLIP_DIST1 ||
       location == VARYING_SLOT_COL0 || location == VARYING_SLOT_COL1 ||
       location == VARYING_SLOT_BFC0 || location == VARYING_SLOT_BFC1 ||
       location == VARYING_SLOT_PNTC ||
       (location >= VARYING_SLOT_VAR0 && location <= VARYING_SLOT_VAR31) ||
       (location >= VARYING_SLOT_TEX0 && location <= VARYING_SLOT_TEX7)) {

      uint64_t bit = 1ull << location;
      if (!(bit & m_input_mask)) {
         ShaderInput input(driver_location, location);
         input.set_ring_offset(16 * driver_location);
         add_input(input);
         m_next_input_ring_offset += 16;
         m_input_mask |= bit;
      }
      return true;
   }
   return false;
}

int
GeometryShader::do_allocate_reserved_registers()
{
   const int sel[6] = {0, 0, 0, 1, 1, 1};
   const int chan[6] = {0, 1, 3, 0, 1, 2};

   /* Reserve registers used by the shaders (should check how many
    * components are actually used */
   for (int i = 0; i < 6; ++i) {
      m_per_vertex_offsets[i] = value_factory().allocate_pinned_register(sel[i], chan[i]);
   }

   m_primitive_id = value_factory().allocate_pinned_register(0, 2);
   m_invocation_id = value_factory().allocate_pinned_register(1, 3);

   value_factory().set_virtual_register_base(2);

   auto zero = value_factory().inline_const(ALU_SRC_0, 0);

   for (int i = 0; i < 4; ++i) {
      m_export_base[i] = value_factory().temp_register(0, false);
      emit_instruction(
         new AluInstr(op1_mov, m_export_base[i], zero, AluInstr::last_write));
   }

   m_ring_item_sizes[0] = m_next_input_ring_offset;

   /* GS thread with no output workaround - emit a cut at start of GS */
   if (chip_class() == ISA_CC_R600) {
      emit_instruction(new EmitVertexInstr(0, true));
      start_new_block(0);
   }

   if (m_tri_strip_adj_fix)
      emit_adj_fix();

   return value_factory().next_register_index();
}

bool
GeometryShader::process_stage_intrinsic(nir_intrinsic_instr *intr)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_emit_vertex:
      return emit_vertex(intr, false);
   case nir_intrinsic_end_primitive:
      return emit_vertex(intr, true);
   case nir_intrinsic_load_primitive_id:
      return emit_simple_mov(intr->def, 0, m_primitive_id);
   case nir_intrinsic_load_invocation_id:
      return emit_simple_mov(intr->def, 0, m_invocation_id);
   case nir_intrinsic_load_per_vertex_input:
      return emit_load_per_vertex_input(intr);
   default:;
   }
   return false;
}

bool
GeometryShader::emit_vertex(nir_intrinsic_instr *instr, bool cut)
{
   int stream = nir_intrinsic_stream_id(instr);
   assert(stream < 4);

   auto cut_instr = new EmitVertexInstr(stream, cut);

   for (auto v : m_streamout_data) {
      if (stream == 0 || v.first != VARYING_SLOT_POS) {
         v.second->patch_ring(stream, m_export_base[stream]);
         cut_instr->add_required_instr(v.second);
         emit_instruction(v.second);
      } else
         delete v.second;
   }
   m_streamout_data.clear();

   emit_instruction(cut_instr);
   start_new_block(0);

   if (!cut) {
      auto ir = new AluInstr(op2_add_int,
                             m_export_base[stream],
                             m_export_base[stream],
                             value_factory().literal(m_noutputs),
                             AluInstr::last_write);
      emit_instruction(ir);
   }

   return true;
}

bool
GeometryShader::store_output(nir_intrinsic_instr *instr)
{
   if (nir_intrinsic_io_semantics(instr).location == VARYING_SLOT_CLIP_VERTEX)
      return true;

   auto location = nir_intrinsic_io_semantics(instr).location;
   auto index = nir_src_as_const_value(instr->src[1]);
   assert(index);
   auto driver_location = nir_intrinsic_base(instr) + index->u32;

   uint32_t write_mask = nir_intrinsic_write_mask(instr);
   uint32_t shift = nir_intrinsic_component(instr);

   RegisterVec4::Swizzle src_swz{7, 7, 7, 7};
   for (unsigned i = shift; i < 4; ++i) {
      src_swz[i] = (1 << i) & (write_mask << shift) ? i - shift : 7;
   }

   auto out_value = value_factory().src_vec4(instr->src[0], pin_free, src_swz);

   AluInstr *ir = nullptr;
   if (m_streamout_data[location]) {
      const auto& value = m_streamout_data[location]->value();
      auto tmp = value_factory().temp_vec4(pin_chgr);
      for (unsigned i = 0; i < 4 - shift; ++i) {
         if (!(write_mask & (1 << i)))
            continue;
         if (out_value[i + shift]->chan() < 4) {
            ir = new AluInstr(op1_mov,
                              tmp[i + shift],
                              out_value[i + shift],
                              AluInstr::write);
         } else if (value[i]->chan() < 4) {
            ir = new AluInstr(op1_mov, tmp[i + shift], value[i], AluInstr::write);
         } else
            continue;
         emit_instruction(ir);
      }
      ir->set_alu_flag(alu_last_instr);
      m_streamout_data[location] = new MemRingOutInstr(cf_mem_ring,
                                                       MemRingOutInstr::mem_write_ind,
                                                       tmp,
                                                       4 * driver_location,
                                                       instr->num_components,
                                                       m_export_base[0]);
   } else {

      sfn_log << SfnLog::io << "None-streamout ";
      bool need_copy = shift != 0;
      if (!need_copy) {
         for (int i = 0; i < 4; ++i) {
            if ((write_mask & (1 << i)) && (out_value[i]->chan() != i)) {
               need_copy = true;
               break;
            }
         }
      }

      if (need_copy) {
         auto tmp = value_factory().temp_vec4(pin_chgr);
         for (unsigned i = 0; i < 4 - shift; ++i) {
            if (out_value[i]->chan() < 4) {
               ir = new AluInstr(op1_mov, tmp[i], out_value[i], AluInstr::write);
               emit_instruction(ir);
            }
         }
         ir->set_alu_flag(alu_last_instr);
         m_streamout_data[location] = new MemRingOutInstr(cf_mem_ring,
                                                          MemRingOutInstr::mem_write_ind,
                                                          tmp,
                                                          4 * driver_location,
                                                          instr->num_components,
                                                          m_export_base[0]);
      } else {
         for (auto i = 0; i < 4; ++i)
            out_value[i]->set_pin(pin_chgr);
         m_streamout_data[location] = new MemRingOutInstr(cf_mem_ring,
                                                          MemRingOutInstr::mem_write_ind,
                                                          out_value,
                                                          4 * driver_location,
                                                          instr->num_components,
                                                          m_export_base[0]);
      }
   }

   return true;
}

bool
GeometryShader::emit_load_per_vertex_input(nir_intrinsic_instr *instr)
{
   auto dest = value_factory().dest_vec4(instr->def, pin_group);

   RegisterVec4::Swizzle dest_swz{7, 7, 7, 7};
   for (unsigned i = 0; i < instr->def.num_components; ++i) {
      dest_swz[i] = i + nir_intrinsic_component(instr);
   }

   auto literal_index = nir_src_as_const_value(instr->src[0]);

   if (!literal_index) {
      sfn_log << SfnLog::err << "GS: Indirect input addressing not (yet) supported\n";
      return false;
   }
   assert(literal_index->u32 < 6);
   assert(nir_intrinsic_io_semantics(instr).num_slots == 1);

   EVTXDataFormat fmt =
      chip_class() >= ISA_CC_EVERGREEN ? fmt_invalid : fmt_32_32_32_32_float;

   auto addr = m_per_vertex_offsets[literal_index->u32];
   auto fetch = new LoadFromBuffer(dest,
                                   dest_swz,
                                   addr,
                                   16 * nir_intrinsic_base(instr),
                                   R600_GS_RING_CONST_BUFFER,
                                   nullptr,
                                   fmt);

   if (chip_class() >= ISA_CC_EVERGREEN)
      fetch->set_fetch_flag(FetchInstr::use_const_field);

   fetch->set_num_format(vtx_nf_norm);
   fetch->reset_fetch_flag(FetchInstr::format_comp_signed);

   emit_instruction(fetch);
   return true;
}

void
GeometryShader::do_finalize()
{
}

void
GeometryShader::do_get_shader_info(r600_shader *sh_info)
{
   sh_info->processor_type = PIPE_SHADER_GEOMETRY;
   sh_info->ring_item_sizes[0] = m_ring_item_sizes[0];
   sh_info->cc_dist_mask = m_cc_dist_mask;
   sh_info->clip_dist_write = m_clip_dist_write;
}

bool
GeometryShader::read_prop(std::istream& is)
{
   (void)is;
   return true;
}

void
GeometryShader::do_print_properties(std::ostream& os) const
{
   (void)os;
}

void
GeometryShader::emit_adj_fix()
{
   auto adjhelp0 = value_factory().temp_register();

   emit_instruction(new AluInstr(op2_and_int,
                                 adjhelp0,
                                 m_primitive_id,
                                 value_factory().one_i(),
                                 AluInstr::last_write));

   int reg_indices[6];
   int rotate_indices[6] = {4, 5, 0, 1, 2, 3};

   reg_indices[0] = reg_indices[1] = reg_indices[2] = m_export_base[1]->sel();
   reg_indices[3] = reg_indices[4] = reg_indices[5] = m_export_base[2]->sel();

   std::array<PRegister, 6> adjhelp;

   AluInstr *ir = nullptr;
   for (int i = 0; i < 6; i++) {
      adjhelp[i] = value_factory().temp_register();
      ir = new AluInstr(op3_cnde_int,
                        adjhelp[i],
                        adjhelp0,
                        m_per_vertex_offsets[i],
                        m_per_vertex_offsets[rotate_indices[i]],
                        AluInstr::write);

      emit_instruction(ir);
   }
   ir->set_alu_flag(alu_last_instr);

   for (int i = 0; i < 6; i++)
      m_per_vertex_offsets[i] = adjhelp[i];
}

} // namespace r600
