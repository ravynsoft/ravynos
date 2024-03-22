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

#include "sfn_shader_vs.h"

#include "../r600_asm.h"

#include "sfn_debug.h"
#include "sfn_instr_alugroup.h"
#include "sfn_instr_export.h"

namespace r600 {

uint32_t
VertexStageShader::enabled_stream_buffers_mask() const
{
   return m_enabled_stream_buffers_mask;
}

void
VertexStageShader::combine_enabled_stream_buffers_mask(uint32_t mask)
{
   m_enabled_stream_buffers_mask = mask;
}

bool
VertexExportStage::store_output(nir_intrinsic_instr& intr)
{
   auto index = nir_src_as_const_value(intr.src[1]);
   assert(index && "Indirect outputs not supported");

   const store_loc store_info = {nir_intrinsic_component(&intr),
                                 nir_intrinsic_io_semantics(&intr).location,
                                 (unsigned)nir_intrinsic_base(&intr) + index->u32,
                                 0};

   return do_store_output(store_info, intr);
}

VertexExportStage::VertexExportStage(VertexStageShader *parent):
    m_parent(parent)
{
}

VertexExportForFs::VertexExportForFs(VertexStageShader *parent,
                                     const pipe_stream_output_info *so_info,
                                     const r600_shader_key& key):
    VertexExportStage(parent),
    m_vs_as_gs_a(key.vs.as_gs_a),
    m_so_info(so_info)
{
}

bool
VertexExportForFs::do_store_output(const store_loc& store_info, nir_intrinsic_instr& intr)
{
   switch (store_info.location) {

   case VARYING_SLOT_PSIZ:
      m_writes_point_size = true;
      FALLTHROUGH;
   case VARYING_SLOT_POS:
      return emit_varying_pos(store_info, intr);
   case VARYING_SLOT_EDGE: {
      std::array<uint8_t, 4> swizzle_override = {7, 0, 7, 7};
      return emit_varying_pos(store_info, intr, &swizzle_override);
   }
   case VARYING_SLOT_VIEWPORT: {
      std::array<uint8_t, 4> swizzle_override = {7, 7, 7, 0};
      return emit_varying_pos(store_info, intr, &swizzle_override) &&
             emit_varying_param(store_info, intr);
   }
   case VARYING_SLOT_CLIP_VERTEX:
      return emit_clip_vertices(store_info, intr);
   case VARYING_SLOT_CLIP_DIST0:
   case VARYING_SLOT_CLIP_DIST1: {
      bool success = emit_varying_pos(store_info, intr);
      m_num_clip_dist += 4;
      if (!nir_intrinsic_io_semantics(&intr).no_varying)
         success &= emit_varying_param(store_info, intr);
      return success;
   }
   case VARYING_SLOT_LAYER: {
      m_out_misc_write = 1;
      m_vs_out_layer = 1;
      std::array<uint8_t, 4> swz = {7, 7, 0, 7};
      return emit_varying_pos(store_info, intr, &swz) &&
             emit_varying_param(store_info, intr);
   }
   case VARYING_SLOT_VIEW_INDEX:
      return emit_varying_pos(store_info, intr) && emit_varying_param(store_info, intr);

   default:
      return emit_varying_param(store_info, intr);
      return false;
   }
}

bool
VertexExportForFs::emit_clip_vertices(const store_loc& store_info,
                                      const nir_intrinsic_instr& instr)
{
   auto& vf = m_parent->value_factory();

   m_cc_dist_mask = 0xff;
   m_clip_dist_write = 0xff;

   m_clip_vertex = vf.src_vec4(instr.src[store_info.data_loc], pin_group, {0, 1, 2, 3});

   m_output_registers[nir_intrinsic_base(&instr)] = &m_clip_vertex;

   return true;
}

void
VertexExportForFs::get_shader_info(r600_shader *sh_info) const
{
   sh_info->cc_dist_mask = m_cc_dist_mask;
   sh_info->clip_dist_write = m_clip_dist_write;
   sh_info->vs_as_gs_a = m_vs_as_gs_a;
   sh_info->vs_out_edgeflag = m_out_edgeflag;
   sh_info->vs_out_viewport = m_out_viewport;
   sh_info->vs_out_misc_write = m_out_misc_write;
   sh_info->vs_out_point_size = m_out_point_size;
   sh_info->vs_out_layer = m_vs_out_layer;
}

void
VertexExportForFs::finalize()
{
   if (m_vs_as_gs_a) {
      auto primid = m_parent->value_factory().temp_vec4(pin_group, {2, 7, 7, 7});
      m_parent->emit_instruction(new AluInstr(
         op1_mov, primid[0], m_parent->primitive_id(), AluInstr::last_write));
      int param = m_last_param_export ? m_last_param_export->location() + 1 : 0;

      m_last_param_export = new ExportInstr(ExportInstr::param, param, primid);
      m_parent->emit_instruction(m_last_param_export);

      ShaderOutput output(m_parent->noutputs(), 1, VARYING_SLOT_PRIMITIVE_ID);
      output.set_export_param(param);
      m_parent->add_output(output);
   }

   if (!m_last_pos_export) {
      RegisterVec4 value(0, false, {7, 7, 7, 7});
      m_last_pos_export = new ExportInstr(ExportInstr::pos, 0, value);
      m_parent->emit_instruction(m_last_pos_export);
   }

   if (!m_last_param_export) {
      RegisterVec4 value(0, false, {7, 7, 7, 7});
      m_last_param_export = new ExportInstr(ExportInstr::param, 0, value);
      m_parent->emit_instruction(m_last_param_export);
   }

   m_last_pos_export->set_is_last_export(true);
   m_last_param_export->set_is_last_export(true);

   if (m_so_info && m_so_info->num_outputs)
      emit_stream(-1);
}

void
VertexShader::do_get_shader_info(r600_shader *sh_info)
{
   sh_info->processor_type = PIPE_SHADER_VERTEX;
   m_export_stage->get_shader_info(sh_info);
}

bool
VertexExportForFs::emit_varying_pos(const store_loc& store_info,
                                    nir_intrinsic_instr& intr,
                                    std::array<uint8_t, 4> *swizzle_override)
{
   RegisterVec4::Swizzle swizzle;
   uint32_t write_mask = 0;

   write_mask = nir_intrinsic_write_mask(&intr) << store_info.frac;

   if (!swizzle_override) {
      for (int i = 0; i < 4; ++i)
         swizzle[i] = ((1 << i) & write_mask) ? i - store_info.frac : 7;
   } else
      std::copy(swizzle_override->begin(), swizzle_override->end(), swizzle.begin());

   int export_slot = 0;

   auto in_value = m_parent->value_factory().src_vec4(intr.src[0], pin_group, swizzle);
   auto& value = in_value;
   RegisterVec4 out_value = m_parent->value_factory().temp_vec4(pin_group, swizzle);

   switch (store_info.location) {
   case VARYING_SLOT_EDGE: {
      m_out_misc_write = true;
      m_out_edgeflag = true;
      auto src = m_parent->value_factory().src(intr.src[0], 0);
      auto clamped = m_parent->value_factory().temp_register();
      m_parent->emit_instruction(
         new AluInstr(op1_mov, clamped, src, {alu_write, alu_dst_clamp, alu_last_instr}));
      auto alu =
         new AluInstr(op1_flt_to_int, out_value[1], clamped, AluInstr::last_write);
      if (m_parent->chip_class() < ISA_CC_EVERGREEN)
         alu->set_alu_flag(alu_is_trans);
      m_parent->emit_instruction(alu);

      value = out_value;
   }
      FALLTHROUGH;
   case VARYING_SLOT_PSIZ:
      m_out_misc_write = true;
      m_out_point_size = true;
      FALLTHROUGH;
   case VARYING_SLOT_LAYER:
      export_slot = 1;
      break;
   case VARYING_SLOT_VIEWPORT:
      m_out_misc_write = true;
      m_out_viewport = true;
      export_slot = 1;
      break;
   case VARYING_SLOT_POS:
      break;
   case VARYING_SLOT_CLIP_DIST0:
   case VARYING_SLOT_CLIP_DIST1:
      m_cc_dist_mask |= write_mask
                        << (4 * (store_info.location - VARYING_SLOT_CLIP_DIST0));
      m_clip_dist_write |= write_mask
                           << (4 * (store_info.location - VARYING_SLOT_CLIP_DIST0));
      export_slot = m_cur_clip_pos++;
      break;
   default:
      sfn_log << SfnLog::err << __func__ << "Unsupported location " << store_info.location
              << "\n";
      return false;
   }

   m_last_pos_export = new ExportInstr(ExportInstr::pos, export_slot, value);

   m_output_registers[nir_intrinsic_base(&intr)] = &m_last_pos_export->value();

   m_parent->emit_instruction(m_last_pos_export);

   return true;
}

bool
VertexExportForFs::emit_varying_param(const store_loc& store_info,
                                      nir_intrinsic_instr& intr)
{
   sfn_log << SfnLog::io << __func__ << ": emit DDL: " << store_info.driver_location
           << "\n";

   int write_mask = nir_intrinsic_write_mask(&intr) << store_info.frac;
   RegisterVec4::Swizzle swizzle;
   for (int i = 0; i < 4; ++i)
      swizzle[i] = ((1 << i) & write_mask) ? i - store_info.frac : 7;

   Pin pin = util_bitcount(write_mask) > 1 ? pin_group : pin_free;

   int export_slot = m_parent->output(nir_intrinsic_base(&intr)).export_param();
   assert(export_slot >= 0);
   auto value = m_parent->value_factory().temp_vec4(pin, swizzle);

   AluInstr *alu = nullptr;
   for (int i = 0; i < 4; ++i) {
      if (swizzle[i] < 4) {
         alu = new AluInstr(op1_mov,
                            value[i],
                            m_parent->value_factory().src(intr.src[0], swizzle[i]),
                            AluInstr::write);
         m_parent->emit_instruction(alu);
      }
   }
   if (alu)
      alu->set_alu_flag(alu_last_instr);

   m_last_param_export = new ExportInstr(ExportInstr::param, export_slot, value);
   m_output_registers[nir_intrinsic_base(&intr)] = &m_last_param_export->value();

   m_parent->emit_instruction(m_last_param_export);

   return true;
}

bool
VertexExportForFs::emit_stream(int stream)
{
   assert(m_so_info);
   if (m_so_info->num_outputs > PIPE_MAX_SO_OUTPUTS) {
      R600_ASM_ERR("Too many stream outputs: %d\n", m_so_info->num_outputs);
      return false;
   }
   for (unsigned i = 0; i < m_so_info->num_outputs; i++) {
      if (m_so_info->output[i].output_buffer >= 4) {
         R600_ASM_ERR("Exceeded the max number of stream output buffers, got: %d\n",
                      m_so_info->output[i].output_buffer);
         return false;
      }
   }
   const RegisterVec4 *so_gpr[PIPE_MAX_SHADER_OUTPUTS];
   unsigned start_comp[PIPE_MAX_SHADER_OUTPUTS];
   std::vector<RegisterVec4> tmp(m_so_info->num_outputs);

   /* Initialize locations where the outputs are stored. */
   for (unsigned i = 0; i < m_so_info->num_outputs; i++) {
      if (stream != -1 && stream != m_so_info->output[i].stream)
         continue;

      sfn_log << SfnLog::instr << "Emit stream " << i << " with register index "
              << m_so_info->output[i].register_index << "  so_gpr:";

      so_gpr[i] = output_register(m_so_info->output[i].register_index);

      if (!so_gpr[i]) {
         sfn_log << SfnLog::err << "\nERR: register index "
                 << m_so_info->output[i].register_index
                 << " doesn't correspond to an output register\n";
         return false;
      }
      start_comp[i] = m_so_info->output[i].start_component;
      /* Lower outputs with dst_offset < start_component.
       *
       * We can only output 4D vectors with a write mask, e.g. we can
       * only output the W component at offset 3, etc. If we want
       * to store Y, Z, or W at buffer offset 0, we need to use MOV
       * to move it to X and output X. */

      bool need_copy =
         m_so_info->output[i].dst_offset < m_so_info->output[i].start_component;

      int sc = m_so_info->output[i].start_component;
      for (int j = 0; j < m_so_info->output[i].num_components; j++) {
         if ((*so_gpr[i])[j + sc]->chan() != j + sc) {
            need_copy = true;
            break;
         }
      }
      if (need_copy) {
         RegisterVec4::Swizzle swizzle = {0, 1, 2, 3};
         for (auto j = m_so_info->output[i].num_components; j < 4; ++j)
            swizzle[j] = 7;
         tmp[i] = m_parent->value_factory().temp_vec4(pin_group, swizzle);

         AluInstr *alu = nullptr;
         for (int j = 0; j < m_so_info->output[i].num_components; j++) {
            alu = new AluInstr(op1_mov, tmp[i][j], (*so_gpr[i])[j + sc], {alu_write});
            m_parent->emit_instruction(alu);
         }
         if (alu)
            alu->set_alu_flag(alu_last_instr);

         start_comp[i] = 0;
         so_gpr[i] = &tmp[i];
      }
      sfn_log << SfnLog::instr << *so_gpr[i] << "\n";
   }

   uint32_t enabled_stream_buffers_mask = 0;
   /* Write outputs to buffers. */
   for (unsigned i = 0; i < m_so_info->num_outputs; i++) {
      sfn_log << SfnLog::instr << "Write output buffer " << i << " with register index "
              << m_so_info->output[i].register_index << "\n";

      auto out_stream =
         new StreamOutInstr(*so_gpr[i],
                            m_so_info->output[i].num_components,
                            m_so_info->output[i].dst_offset - start_comp[i],
                            ((1 << m_so_info->output[i].num_components) - 1)
                               << start_comp[i],
                            m_so_info->output[i].output_buffer,
                            m_so_info->output[i].stream);
      m_parent->emit_instruction(out_stream);
      enabled_stream_buffers_mask |= (1 << m_so_info->output[i].output_buffer)
                                     << m_so_info->output[i].stream * 4;
   }
   m_parent->combine_enabled_stream_buffers_mask(enabled_stream_buffers_mask);
   return true;
}

const RegisterVec4 *
VertexExportForFs::output_register(int loc) const
{
   const RegisterVec4 *retval = nullptr;
   auto val = m_output_registers.find(loc);
   if (val != m_output_registers.end())
      retval = val->second;
   return retval;
}

VertexShader::VertexShader(const pipe_stream_output_info *so_info,
                           r600_shader *gs_shader,
                           const r600_shader_key& key):
    VertexStageShader("VS", key.vs.first_atomic_counter),
    m_vs_as_gs_a(key.vs.as_gs_a)
{
   if (key.vs.as_es)
      m_export_stage = new VertexExportForGS(this, gs_shader);
   else if (key.vs.as_ls)
      m_export_stage = new VertexExportForTCS(this);
   else
      m_export_stage = new VertexExportForFs(this, so_info, key);
}

bool
VertexShader::do_scan_instruction(nir_instr *instr)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   auto intr = nir_instr_as_intrinsic(instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_load_input: {
      int vtx_register = nir_intrinsic_base(intr) + 1;
      if (m_last_vertex_attribute_register < vtx_register)
         m_last_vertex_attribute_register = vtx_register;
      return true;
   }
   case nir_intrinsic_store_output: {
      auto location = static_cast<gl_varying_slot>(nir_intrinsic_io_semantics(intr).location);

      if (nir_intrinsic_io_semantics(intr).no_varying &&
          (location == VARYING_SLOT_CLIP_DIST0 || location == VARYING_SLOT_CLIP_DIST1)) {
         break;
      }

      int driver_location = nir_intrinsic_base(intr);

      int write_mask =
         location == VARYING_SLOT_LAYER ? 1 << 2 : nir_intrinsic_write_mask(intr);

      ShaderOutput output(driver_location, write_mask, location);

      add_output(output);
      break;
   }
   case nir_intrinsic_load_vertex_id:
      m_sv_values.set(es_vertexid);
      break;
   case nir_intrinsic_load_instance_id:
      m_sv_values.set(es_instanceid);
      break;
   case nir_intrinsic_load_primitive_id:
      m_sv_values.set(es_primitive_id);
      break;
   case nir_intrinsic_load_tcs_rel_patch_id_r600:
      m_sv_values.set(es_rel_patch_id);
      break;
   default:
      return false;
   }

   return true;
}

bool
VertexShader::load_input(nir_intrinsic_instr *intr)
{
   unsigned driver_location = nir_intrinsic_base(intr);
   unsigned location = nir_intrinsic_io_semantics(intr).location;
   auto& vf = value_factory();

   AluInstr *ir = nullptr;
   if (location < VERT_ATTRIB_MAX) {
      for (unsigned i = 0; i < intr->def.num_components; ++i) {
         auto src = vf.allocate_pinned_register(driver_location + 1, i);
         src->set_flag(Register::ssa);
         vf.inject_value(intr->def, i, src);
      }
      if (ir)
         ir->set_alu_flag(alu_last_instr);

      ShaderInput input(driver_location);
      input.set_gpr(driver_location + 1);
      add_input(input);
      return true;
   }
   fprintf(stderr, "r600-NIR: Unimplemented load_deref for %d\n", location);
   return false;
}

int
VertexShader::do_allocate_reserved_registers()
{
   if (m_sv_values.test(es_vertexid)) {
      m_vertex_id = value_factory().allocate_pinned_register(0, 0);
   }

   if (m_sv_values.test(es_instanceid)) {
      m_instance_id = value_factory().allocate_pinned_register(0, 3);
   }

   if (m_sv_values.test(es_primitive_id) || m_vs_as_gs_a) {
      auto primitive_id = value_factory().allocate_pinned_register(0, 2);
      set_primitive_id(primitive_id);
   }

   if (m_sv_values.test(es_rel_patch_id)) {
      m_rel_vertex_id = value_factory().allocate_pinned_register(0, 1);
   }

   return m_last_vertex_attribute_register + 1;
}

bool
VertexShader::store_output(nir_intrinsic_instr *intr)
{
   return m_export_stage->store_output(*intr);
}

bool
VertexShader::process_stage_intrinsic(nir_intrinsic_instr *intr)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_load_vertex_id:
      return emit_simple_mov(intr->def, 0, m_vertex_id);
   case nir_intrinsic_load_instance_id:
      return emit_simple_mov(intr->def, 0, m_instance_id);
   case nir_intrinsic_load_primitive_id:
      return emit_simple_mov(intr->def, 0, primitive_id());
   case nir_intrinsic_load_tcs_rel_patch_id_r600:
      return emit_simple_mov(intr->def, 0, m_rel_vertex_id);
   default:
      return false;
   }
}

void
VertexShader::do_finalize()
{
   m_export_stage->finalize();
}

bool
VertexShader::read_prop(std::istream& is)
{
   (void)is;
   return false;
}

void
VertexShader::do_print_properties(std::ostream& os) const
{
   (void)os;
}

VertexExportForGS::VertexExportForGS(VertexStageShader *parent,
                                     const r600_shader *gs_shader):
    VertexExportStage(parent),
    m_gs_shader(gs_shader)
{
}

bool
VertexExportForGS::do_store_output(const store_loc& store_info,
                                   nir_intrinsic_instr& instr)
{
   int ring_offset = -1;
   auto out_io = m_parent->output(store_info.driver_location);

   sfn_log << SfnLog::io << "check output " << store_info.driver_location
           << " varying_slot=" << static_cast<int>(out_io.varying_slot()) << "\n";

   for (unsigned k = 0; k < m_gs_shader->ninput; ++k) {
      auto& in_io = m_gs_shader->input[k];
      sfn_log << SfnLog::io << "  against  " << k
              << " varying_slot=" << static_cast<int>(in_io.varying_slot) << "\n";

      if (in_io.varying_slot == out_io.varying_slot()) {
         ring_offset = in_io.ring_offset;
         break;
      }
   }

   if (store_info.location == VARYING_SLOT_VIEWPORT) {
      m_vs_out_viewport = 1;
      m_vs_out_misc_write = 1;
      return true;
   }

   if (ring_offset == -1) {
      sfn_log << SfnLog::warn << "VS defines output at "
              << store_info.driver_location
              << " varying_slot=" << static_cast<int>(out_io.varying_slot())
              << " that is not consumed as GS input\n";
      return true;
   }

   RegisterVec4::Swizzle src_swz = {7, 7, 7, 7};
   for (int i = 0; i < 4; ++i)
      src_swz[i] = i < instr.num_components ? i : 7;

   auto value = m_parent->value_factory().temp_vec4(pin_chgr, src_swz);

   AluInstr *ir = nullptr;
   for (unsigned int i = 0; i < instr.num_components; ++i) {
      ir = new AluInstr(op1_mov,
                        value[i],
                        m_parent->value_factory().src(instr.src[store_info.data_loc], i),
                        AluInstr::write);
      m_parent->emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);

   m_parent->emit_instruction(new MemRingOutInstr(
      cf_mem_ring, MemRingOutInstr::mem_write, value, ring_offset >> 2, 4, nullptr));

   if (store_info.location == VARYING_SLOT_CLIP_DIST0 ||
       store_info.location == VARYING_SLOT_CLIP_DIST1)
      m_num_clip_dist += 4;

   return true;
}

void
VertexExportForGS::finalize()
{
}

void
VertexExportForGS::get_shader_info(r600_shader *sh_info) const
{
   sh_info->vs_out_viewport = m_vs_out_viewport;
   sh_info->vs_out_misc_write = m_vs_out_misc_write;
   sh_info->vs_as_es = true;
}

VertexExportForTCS::VertexExportForTCS(VertexStageShader *parent):
    VertexExportStage(parent)
{
}

void
VertexExportForTCS::finalize()
{
}

void
VertexExportForTCS::get_shader_info(r600_shader *sh_info) const
{
   sh_info->vs_as_ls = 1;
}

bool
VertexExportForTCS::do_store_output(const store_loc& store_info,
                                    nir_intrinsic_instr& intr)
{
   (void)store_info;
   (void)intr;
   return true;
}

} // namespace r600
