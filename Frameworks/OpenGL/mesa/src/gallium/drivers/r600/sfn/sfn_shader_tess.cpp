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

#include "sfn_shader_tess.h"

#include "sfn_instr_export.h"
#include "sfn_shader_vs.h"

#include <sstream>

namespace r600 {

using std::string;

TCSShader::TCSShader(const r600_shader_key& key):
    Shader("TCS", key.tcs.first_atomic_counter),
    m_tcs_prim_mode(key.tcs.prim_mode)
{
}

bool
TCSShader::do_scan_instruction(nir_instr *instr)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *ii = nir_instr_as_intrinsic(instr);

   switch (ii->intrinsic) {
   case nir_intrinsic_load_primitive_id:
      m_sv_values.set(es_primitive_id);
      break;
   case nir_intrinsic_load_invocation_id:
      m_sv_values.set(es_invocation_id);
      break;
   case nir_intrinsic_load_tcs_rel_patch_id_r600:
      m_sv_values.set(es_rel_patch_id);
      break;
   case nir_intrinsic_load_tcs_tess_factor_base_r600:
      m_sv_values.set(es_tess_factor_base);
      break;
   default:
      return false;
      ;
   }
   return true;
}

int
TCSShader::do_allocate_reserved_registers()
{
   if (m_sv_values.test(es_primitive_id)) {
      m_primitive_id = value_factory().allocate_pinned_register(0, 0);
   }

   if (m_sv_values.test(es_invocation_id)) {
      m_invocation_id = value_factory().allocate_pinned_register(0, 2);
   }

   if (m_sv_values.test(es_rel_patch_id)) {
      m_rel_patch_id = value_factory().allocate_pinned_register(0, 1);
   }

   if (m_sv_values.test(es_tess_factor_base)) {
      m_tess_factor_base = value_factory().allocate_pinned_register(0, 3);
   }

   return value_factory().next_register_index();
   ;
}

bool
TCSShader::process_stage_intrinsic(nir_intrinsic_instr *instr)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_load_tcs_rel_patch_id_r600:
      return emit_simple_mov(instr->def, 0, m_rel_patch_id);
   case nir_intrinsic_load_invocation_id:
      return emit_simple_mov(instr->def, 0, m_invocation_id);
   case nir_intrinsic_load_primitive_id:
      return emit_simple_mov(instr->def, 0, m_primitive_id);
   case nir_intrinsic_load_tcs_tess_factor_base_r600:
      return emit_simple_mov(instr->def, 0, m_tess_factor_base);
   case nir_intrinsic_store_tf_r600:
      return store_tess_factor(instr);
   default:
      return false;
   }
}

bool
TCSShader::store_tess_factor(nir_intrinsic_instr *instr)
{
   auto value0 = value_factory().src_vec4(instr->src[0], pin_group, {0, 1, 7, 7});
   emit_instruction(new WriteTFInstr(value0));
   return true;
}

void
TCSShader::do_get_shader_info(r600_shader *sh_info)
{
   sh_info->processor_type = PIPE_SHADER_TESS_CTRL;
   sh_info->tcs_prim_mode = m_tcs_prim_mode;
}

bool
TCSShader::read_prop(std::istream& is)
{
   string value;
   is >> value;

   ASSERTED auto splitpos = value.find(':');
   assert(splitpos != string::npos);

   std::istringstream ival(value);
   string name;
   string val;

   std::getline(ival, name, ':');

   if (name == "TCS_PRIM_MODE")
      ival >> m_tcs_prim_mode;
   else
      return false;
   return true;
}

void
TCSShader::do_print_properties(std::ostream& os) const
{
   os << "PROP TCS_PRIM_MODE:" << m_tcs_prim_mode << "\n";
}

TESShader::TESShader(const pipe_stream_output_info *so_info,
                     const r600_shader *gs_shader,
                     const r600_shader_key& key):
    VertexStageShader("TES", key.tes.first_atomic_counter),
    m_vs_as_gs_a(key.vs.as_gs_a),
    m_tes_as_es(key.tes.as_es)
{
   if (key.tes.as_es)
      m_export_processor = new VertexExportForGS(this, gs_shader);
   else
      m_export_processor = new VertexExportForFs(this, so_info, key);
}

bool
TESShader::do_scan_instruction(nir_instr *instr)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   auto intr = nir_instr_as_intrinsic(instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_load_tess_coord_xy:
      m_sv_values.set(es_tess_coord);
      break;
   case nir_intrinsic_load_primitive_id:
      m_sv_values.set(es_primitive_id);
      break;
   case nir_intrinsic_load_tcs_rel_patch_id_r600:
      m_sv_values.set(es_rel_patch_id);
      break;
   case nir_intrinsic_store_output: {
      int driver_location = nir_intrinsic_base(intr);
      auto location = static_cast<gl_varying_slot>(nir_intrinsic_io_semantics(intr).location);
      auto write_mask = nir_intrinsic_write_mask(intr);

      if (location == VARYING_SLOT_LAYER)
         write_mask = 4;

      ShaderOutput output(driver_location, write_mask, location);

      add_output(output);
      break;
   }
   default:
      return false;
   }
   return true;
}

int
TESShader::do_allocate_reserved_registers()
{
   if (m_sv_values.test(es_tess_coord)) {
      m_tess_coord[0] = value_factory().allocate_pinned_register(0, 0);
      m_tess_coord[1] = value_factory().allocate_pinned_register(0, 1);
   }

   if (m_sv_values.test(es_rel_patch_id)) {
      m_rel_patch_id = value_factory().allocate_pinned_register(0, 2);
   }

   if (m_sv_values.test(es_primitive_id) || m_vs_as_gs_a) {
      m_primitive_id = value_factory().allocate_pinned_register(0, 3);
   }
   return value_factory().next_register_index();
}

bool
TESShader::process_stage_intrinsic(nir_intrinsic_instr *intr)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_load_tess_coord_xy:
      return emit_simple_mov(intr->def, 0, m_tess_coord[0], pin_none) &&
             emit_simple_mov(intr->def, 1, m_tess_coord[1], pin_none);
   case nir_intrinsic_load_primitive_id:
      return emit_simple_mov(intr->def, 0, m_primitive_id);
   case nir_intrinsic_load_tcs_rel_patch_id_r600:
      return emit_simple_mov(intr->def, 0, m_rel_patch_id);
   case nir_intrinsic_store_output:
      return m_export_processor->store_output(*intr);
   default:
      return false;
   }
}

void
TESShader::do_get_shader_info(r600_shader *sh_info)
{
   sh_info->processor_type = PIPE_SHADER_TESS_EVAL;
   m_export_processor->get_shader_info(sh_info);
}

void
TESShader::do_finalize()
{
   m_export_processor->finalize();
}

bool
TESShader::TESShader::read_prop(std::istream& is)
{
   (void)is;
   return true;
}

void
TESShader::do_print_properties(std::ostream& os) const
{
   (void)os;
}

} // namespace r600
