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

#include "sfn_instrfactory.h"

#include "sfn_alu_defines.h"
#include "sfn_debug.h"
#include "sfn_instr_alugroup.h"
#include "sfn_instr_controlflow.h"
#include "sfn_instr_export.h"
#include "sfn_instr_fetch.h"
#include "sfn_instr_lds.h"
#include "sfn_instr_mem.h"
#include "sfn_instr_tex.h"
#include "sfn_shader.h"

#include <sstream>
#include <string>
#include <vector>

namespace r600 {

using std::string;
using std::vector;

InstrFactory::InstrFactory():
    group(nullptr)
{
}

PInst
InstrFactory::from_string(const std::string& s, int nesting_depth, bool is_cayman)
{
   string type;
   std::istringstream is(s);

   PInst result = nullptr;

   do {
      is >> type;
   } while (type.empty() && is.good());

   if (type == "ALU_GROUP_BEGIN") {
      group = new AluGroup();
      group->set_nesting_depth(nesting_depth);
      return nullptr;
   } else if (type == "ALU_GROUP_END") {
      AluGroup *retval = group;
      group = nullptr;
      return retval;
   } else if (type == "ALU") {
      result = AluInstr::from_string(is, m_value_factory, group, is_cayman);
   } else if (type == "TEX") {
      result = TexInstr::from_string(is, m_value_factory);
   } else if (type == "EXPORT") {
      result = ExportInstr::from_string(is, m_value_factory);
   } else if (type == "EXPORT_DONE") {
      result = ExportInstr::last_from_string(is, m_value_factory);
   } else if (type == "VFETCH") {
      result = FetchInstr::from_string(is, m_value_factory);
   } else if (type == "GET_BUF_RESINFO") {
      result = QueryBufferSizeInstr::from_string(is, m_value_factory);
   } else if (type == "LOAD_BUF") {
      result = LoadFromBuffer::from_string(is, m_value_factory);
   } else if (type == "READ_SCRATCH") {
      result = LoadFromScratch::from_string(is, m_value_factory);
   } else if (type == "IF") {
      result = IfInstr::from_string(is, m_value_factory, is_cayman);
   } else if (type == "WRITE_SCRATCH") {
      result = ScratchIOInstr::from_string(is, m_value_factory);
   } else if (type == "MEM_RING") {
      result = MemRingOutInstr::from_string(is, m_value_factory);
   } else if (type == "EMIT_VERTEX") {
      result = EmitVertexInstr::from_string(is, false);
   } else if (type == "EMIT_CUT_VERTEX") {
      result = EmitVertexInstr::from_string(is, true);
   } else if (type == "LDS_READ") {
      result = LDSReadInstr::from_string(is, m_value_factory);
   } else if (type == "LDS") {
      result = LDSAtomicInstr::from_string(is, m_value_factory);
   } else if (type == "WRITE_TF") {
      result = WriteTFInstr::from_string(is, m_value_factory);
   } else
      result = ControlFlowInstr::from_string(type);

   if (!result && !group) {
      std::cerr << "Error translating '" << s << "'\n";
   }

   return result;
}

bool
InstrFactory::from_nir(nir_instr *instr, Shader& shader)
{
   switch (instr->type) {
   case nir_instr_type_alu:
      return AluInstr::from_nir(nir_instr_as_alu(instr), shader);
   case nir_instr_type_intrinsic:
      return shader.process_intrinsic(nir_instr_as_intrinsic(instr));
   case nir_instr_type_load_const:
      return load_const(nir_instr_as_load_const(instr), shader);
   case nir_instr_type_tex:
      return TexInstr::from_nir(nir_instr_as_tex(instr), shader);
   case nir_instr_type_jump:
      return process_jump(nir_instr_as_jump(instr), shader);
   case nir_instr_type_undef:
      return process_undef(nir_instr_as_undef(instr), shader);
   default:
      fprintf(stderr, "Instruction type %d not supported\n", instr->type);
      return false;
   }
}

bool
InstrFactory::load_const(nir_load_const_instr *literal, Shader& shader)
{
   AluInstr *ir = nullptr;

   if (literal->def.bit_size == 64) {
      for (int i = 0; i < literal->def.num_components; ++i) {
         auto dest0 = m_value_factory.dest(literal->def, 2 * i, pin_none);
         auto src0 = m_value_factory.literal(literal->value[i].u64 & 0xffffffff);
         shader.emit_instruction(new AluInstr(op1_mov, dest0, src0, {alu_write}));

         auto dest1 = m_value_factory.dest(literal->def, 2 * i + 1, pin_none);
         auto src1 = m_value_factory.literal((literal->value[i].u64 >> 32) & 0xffffffff);
         shader.emit_instruction(new AluInstr(op1_mov, dest1, src1, AluInstr::last_write));
      }
   } else {
      Pin pin = literal->def.num_components == 1 ? pin_free : pin_none;
      for (int i = 0; i < literal->def.num_components; ++i) {
         auto dest = m_value_factory.dest(literal->def, i, pin);
         uint32_t v = literal->value[i].i32;
         PVirtualValue src = nullptr;
         switch (v) {
         case 0:
            src = m_value_factory.zero();
            break;
         case 1:
            src = m_value_factory.one_i();
            break;
         case 0xffffffff:
            src = m_value_factory.inline_const(ALU_SRC_M_1_INT, 0);
            break;
         case 0x3f800000:
            src = m_value_factory.inline_const(ALU_SRC_1, 0);
            break;
         case 0x3f000000:
            src = m_value_factory.inline_const(ALU_SRC_0_5, 0);
            break;
         default:
            src = m_value_factory.literal(v);
         }

         ir = new AluInstr(op1_mov, dest, src, {alu_write});
         shader.emit_instruction(ir);
      }
      if (ir)
         ir->set_alu_flag(alu_last_instr);
   }
   return true;
}

bool
InstrFactory::process_jump(nir_jump_instr *instr, Shader& shader)
{
   ControlFlowInstr::CFType type;
   switch (instr->type) {
   case nir_jump_break:
      type = ControlFlowInstr::cf_loop_break;
      break;

   case nir_jump_continue:
      type = ControlFlowInstr::cf_loop_continue;
      break;

   default: {
      nir_instr *i = reinterpret_cast<nir_instr *>(instr);
      sfn_log << SfnLog::err << "Jump instrunction " << *i << " not supported\n";
      return false;
   }
   }
   shader.emit_instruction(new ControlFlowInstr(type));
   shader.start_new_block(0);

   return true;
}

bool
InstrFactory::process_undef(nir_undef_instr *undef, Shader& shader)
{
   for (int i = 0; i < undef->def.num_components; ++i) {
      auto dest = shader.value_factory().undef(undef->def.index, i);
      shader.emit_instruction(
         new AluInstr(op1_mov, dest, value_factory().zero(), AluInstr::last_write));
   }
   return true;
}

} // namespace r600
