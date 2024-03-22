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

#include "sfn_shader_cs.h"

#include "sfn_instr_fetch.h"

namespace r600 {

ComputeShader::ComputeShader(UNUSED const r600_shader_key& key, int num_samplers):
    Shader("CS", 0),
    m_image_size_const_offset(num_samplers)
{
}

bool
ComputeShader::do_scan_instruction(UNUSED nir_instr *instr)
{
   return false;
}

int
ComputeShader::do_allocate_reserved_registers()
{
   auto& vf = value_factory();

   const int thread_id_sel = 0;
   const int wg_id_sel = 1;

   for (int i = 0; i < 3; ++i) {
      m_local_invocation_id[i] = vf.allocate_pinned_register(thread_id_sel, i);
      m_local_invocation_id[i]->set_flag(Register::pin_end);
      m_workgroup_id[i] = vf.allocate_pinned_register(wg_id_sel, i);
      m_workgroup_id[i]->set_flag(Register::pin_end);
   }
   return 2;
}

bool
ComputeShader::process_stage_intrinsic(nir_intrinsic_instr *instr)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_load_local_invocation_id:
      return emit_load_3vec(instr, m_local_invocation_id);
   case nir_intrinsic_load_workgroup_id:
      return emit_load_3vec(instr, m_workgroup_id);
   case nir_intrinsic_load_workgroup_size:
      return emit_load_from_info_buffer(instr, 0);
   case nir_intrinsic_load_num_workgroups:
      return emit_load_from_info_buffer(instr, 16);
   default:
      return false;
   }
}

void
ComputeShader::do_get_shader_info(r600_shader *sh_info)
{
   sh_info->processor_type = PIPE_SHADER_COMPUTE;
}

bool
ComputeShader::read_prop(UNUSED std::istream& is)
{
   return true;
}

void
ComputeShader::do_print_properties(UNUSED std::ostream& os) const
{
}

bool
ComputeShader::emit_load_from_info_buffer(nir_intrinsic_instr *instr, int offset)
{
   if (!m_zero_register) {
      m_zero_register = value_factory().temp_register();
      emit_instruction(new AluInstr(op1_mov,
                                    m_zero_register,
                                    value_factory().inline_const(ALU_SRC_0, 0),
                                    AluInstr::last_write));
   }

   auto dest = value_factory().dest_vec4(instr->def, pin_group);

   auto ir = new LoadFromBuffer(dest,
                                {0, 1, 2, 7},
                                m_zero_register,
                                offset,
                                R600_BUFFER_INFO_CONST_BUFFER,
                                nullptr,
                                fmt_32_32_32_32);

   ir->set_fetch_flag(LoadFromBuffer::srf_mode);
   ir->reset_fetch_flag(LoadFromBuffer::format_comp_signed);
   ir->set_num_format(vtx_nf_int);
   emit_instruction(ir);
   return true;
}

bool
ComputeShader::emit_load_3vec(nir_intrinsic_instr *instr,
                              const std::array<PRegister, 3>& src)
{
   auto& vf = value_factory();

   for (int i = 0; i < 3; ++i) {
      auto dest = vf.dest(instr->def, i, pin_none);
      emit_instruction(new AluInstr(
         op1_mov, dest, src[i], i == 2 ? AluInstr::last_write : AluInstr::write));
   }
   return true;
}

} // namespace r600
