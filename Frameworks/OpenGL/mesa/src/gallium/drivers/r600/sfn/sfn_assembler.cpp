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

#include "sfn_assembler.h"

#include "../eg_sq.h"
#include "../r600_asm.h"

#include "sfn_callstack.h"
#include "sfn_conditionaljumptracker.h"
#include "sfn_debug.h"
#include "sfn_instr_alugroup.h"
#include "sfn_instr_controlflow.h"
#include "sfn_instr_export.h"
#include "sfn_instr_fetch.h"
#include "sfn_instr_mem.h"
#include "sfn_instr_tex.h"

namespace r600 {
Assembler::Assembler(r600_shader *sh, const r600_shader_key& key):
    m_sh(sh),
    m_key(key)
{
}

extern const std::map<ESDOp, int> ds_opcode_map;

class AssamblerVisitor : public ConstInstrVisitor {
public:
   AssamblerVisitor(r600_shader *sh, const r600_shader_key& key, bool legacy_math_rules);

   void visit(const AluInstr& instr) override;
   void visit(const AluGroup& instr) override;
   void visit(const TexInstr& instr) override;
   void visit(const ExportInstr& instr) override;
   void visit(const FetchInstr& instr) override;
   void visit(const Block& instr) override;
   void visit(const IfInstr& instr) override;
   void visit(const ControlFlowInstr& instr) override;
   void visit(const ScratchIOInstr& instr) override;
   void visit(const StreamOutInstr& instr) override;
   void visit(const MemRingOutInstr& instr) override;
   void visit(const EmitVertexInstr& instr) override;
   void visit(const GDSInstr& instr) override;
   void visit(const WriteTFInstr& instr) override;
   void visit(const LDSAtomicInstr& instr) override;
   void visit(const LDSReadInstr& instr) override;
   void visit(const RatInstr& instr) override;

   void finalize();

   const uint32_t sf_vtx = 1;
   const uint32_t sf_tex = 2;
   const uint32_t sf_alu = 4;
   const uint32_t sf_addr_register = 8;
   const uint32_t sf_all = 0xf;

   void clear_states(const uint32_t& states);
   bool copy_dst(r600_bytecode_alu_dst& dst, const Register& d, bool write);
   PVirtualValue copy_src(r600_bytecode_alu_src& src, const VirtualValue& s);

   EBufferIndexMode emit_index_reg(const VirtualValue& addr, unsigned idx);

   void emit_endif();
   void emit_else();
   void emit_loop_begin(bool vpm);
   void emit_loop_end();
   void emit_loop_break();
   void emit_loop_cont();

   void emit_alu_op(const AluInstr& ai);
   void emit_lds_op(const AluInstr& lds);

   auto translate_for_mathrules(EAluOp op) -> EAluOp;

   void emit_wait_ack();

   /* Start initialized in constructor */
   const r600_shader_key& m_key;
   r600_shader *m_shader;
   r600_bytecode *m_bc;

   ConditionalJumpTracker m_jump_tracker;
   CallStack m_callstack;
   bool ps_alpha_to_one;
   /* End initialized in constructor */

   std::set<uint32_t> m_nliterals_in_group;
   std::set<int> vtx_fetch_results;
   std::set<int> tex_fetch_results;

   const VirtualValue *m_last_addr{nullptr};

   unsigned m_max_color_exports{0};
   int m_loop_nesting{0};

   bool m_ack_suggested{false};
   bool m_has_param_output{false};
   bool m_has_pos_output{false};
   bool m_last_op_was_barrier{false};
   bool m_result{true};
   bool m_legacy_math_rules{false};
};

bool
Assembler::lower(Shader *shader)
{
   AssamblerVisitor ass(m_sh, m_key, shader->has_flag(Shader::sh_legacy_math_rules));

   auto& blocks = shader->func();
   for (auto b : blocks) {
      b->accept(ass);
      if (!ass.m_result)
         return false;
   }

   ass.finalize();

   return ass.m_result;
}

AssamblerVisitor::AssamblerVisitor(r600_shader *sh, const r600_shader_key& key,
                                   bool legacy_math_rules):
    m_key(key),
    m_shader(sh),

    m_bc(&sh->bc),
    m_callstack(sh->bc),
    ps_alpha_to_one(key.ps.alpha_to_one),
    m_legacy_math_rules(legacy_math_rules)
{
   if (m_shader->processor_type == PIPE_SHADER_FRAGMENT)
      m_max_color_exports = MAX2(m_key.ps.nr_cbufs, 1);

   if (m_shader->processor_type == PIPE_SHADER_VERTEX && m_shader->ninput > 0)
      r600_bytecode_add_cfinst(m_bc, CF_OP_CALL_FS);
}

void
AssamblerVisitor::finalize()
{
   const struct cf_op_info *last = nullptr;

   if (m_bc->cf_last)
      last = r600_isa_cf(m_bc->cf_last->op);

   /* alu clause instructions don't have EOP bit, so add NOP */
   if (m_shader->bc.gfx_level < CAYMAN &&
       (!last || last->flags & CF_ALU || m_bc->cf_last->op == CF_OP_LOOP_END ||
        m_bc->cf_last->op == CF_OP_POP))
      r600_bytecode_add_cfinst(m_bc, CF_OP_NOP);

   /* A fetch shader only can't be EOP (results in hang), but we can replace
    * it by a NOP */
   else if (last && m_bc->cf_last->op == CF_OP_CALL_FS)
      m_bc->cf_last->op = CF_OP_NOP;

   if (m_shader->bc.gfx_level != CAYMAN)
      m_bc->cf_last->end_of_program = 1;
   else
      cm_bytecode_add_cf_end(m_bc);
}

extern const std::map<EAluOp, int> opcode_map;

void
AssamblerVisitor::visit(const AluInstr& ai)
{
   assert(vtx_fetch_results.empty());
   assert(tex_fetch_results.empty());

   if (unlikely(ai.has_alu_flag(alu_is_lds)))
      emit_lds_op(ai);
   else
      emit_alu_op(ai);
}

void
AssamblerVisitor::emit_lds_op(const AluInstr& lds)
{
   struct r600_bytecode_alu alu;
   memset(&alu, 0, sizeof(alu));

   alu.is_lds_idx_op = true;
   alu.op = lds.lds_opcode();

   bool has_lds_fetch = false;
   switch (alu.op) {
   case LDS_WRITE:
      alu.op = LDS_OP2_LDS_WRITE;
      break;
   case LDS_WRITE_REL:
      alu.op = LDS_OP3_LDS_WRITE_REL;
      alu.lds_idx = 1;
      break;
   case DS_OP_READ_RET:
      alu.op = LDS_OP1_LDS_READ_RET;
      FALLTHROUGH;
   case LDS_ADD_RET:
   case LDS_AND_RET:
   case LDS_OR_RET:
   case LDS_MAX_INT_RET:
   case LDS_MAX_UINT_RET:
   case LDS_MIN_INT_RET:
   case LDS_MIN_UINT_RET:
   case LDS_XOR_RET:
   case LDS_XCHG_RET:
   case LDS_CMP_XCHG_RET:
      has_lds_fetch = true;
      break;
   case LDS_ADD:
   case LDS_AND:
   case LDS_OR:
   case LDS_MAX_INT:
   case LDS_MAX_UINT:
   case LDS_MIN_INT:
   case LDS_MIN_UINT:
   case LDS_XOR:
      break;
   default:
      std::cerr << "\n R600: error op: " << lds << "\n";
      unreachable("Unhandled LDS op");
   }

   copy_src(alu.src[0], lds.src(0));

   if (lds.n_sources() > 1)
      copy_src(alu.src[1], lds.src(1));
   else
      alu.src[1].sel = V_SQ_ALU_SRC_0;

   if (lds.n_sources() > 2)
      copy_src(alu.src[2], lds.src(2));
   else
      alu.src[2].sel = V_SQ_ALU_SRC_0;

   alu.last = lds.has_alu_flag(alu_last_instr);

   int r = r600_bytecode_add_alu(m_bc, &alu);
   if (has_lds_fetch)
      m_bc->cf_last->nlds_read++;

   if (r)
      m_result = false;
}

auto AssamblerVisitor::translate_for_mathrules(EAluOp op) -> EAluOp
{
   switch (op) {
   case op2_dot_ieee: return op2_dot;
   case op2_dot4_ieee: return op2_dot4;
   case op2_mul_ieee: return op2_mul;
   case op3_muladd_ieee : return op2_mul_ieee;
   default:
      return op;
   }
}

void
AssamblerVisitor::emit_alu_op(const AluInstr& ai)
{
   sfn_log << SfnLog::assembly << "Emit ALU op " << ai << "\n";

   struct r600_bytecode_alu alu;
   memset(&alu, 0, sizeof(alu));

   auto opcode = ai.opcode();

   if (unlikely(ai.opcode() == op1_mova_int &&
                (m_bc->gfx_level < CAYMAN || alu.dst.sel == 0))) {
      m_last_addr = ai.psrc(0);
      m_bc->ar_chan = m_last_addr->chan();
      m_bc->ar_reg = m_last_addr->sel();
   }

   if (m_legacy_math_rules)
       opcode = translate_for_mathrules(opcode);

   auto hw_opcode = opcode_map.find(opcode);

   if (hw_opcode == opcode_map.end()) {
      std::cerr << "Opcode not handled for " << ai << "\n";
      m_result = false;
      return;
   }

   // skip multiple barriers
   if (m_last_op_was_barrier && opcode == op0_group_barrier)
      return;

   m_last_op_was_barrier = opcode == op0_group_barrier;

   alu.op = hw_opcode->second;

   auto dst = ai.dest();
   if (dst) {
      if (ai.opcode() != op1_mova_int) {
         if (!copy_dst(alu.dst, *dst, ai.has_alu_flag(alu_write))) {
            m_result = false;
            return;
         }

         alu.dst.write = ai.has_alu_flag(alu_write);
         alu.dst.clamp = ai.has_alu_flag(alu_dst_clamp);
         alu.dst.rel = dst->addr() ? 1 : 0;
      } else if (m_bc->gfx_level == CAYMAN && ai.dest()->sel() > 0) {
         alu.dst.sel = ai.dest()->sel() + 1;
      }
   }

   alu.is_op3 = ai.n_sources() == 3;

   EBufferIndexMode kcache_index_mode = bim_none;
   PVirtualValue buffer_offset = nullptr;

   for (unsigned i = 0; i < ai.n_sources(); ++i) {
      buffer_offset = copy_src(alu.src[i], ai.src(i));
      alu.src[i].neg = ai.has_source_mod(i, AluInstr::mod_neg);
      if (!alu.is_op3)
         alu.src[i].abs = ai.has_source_mod(i, AluInstr::mod_abs);

      if (buffer_offset && kcache_index_mode == bim_none) {
         auto idx_reg = buffer_offset->as_register();
         if (idx_reg && idx_reg->has_flag(Register::addr_or_idx)) {
            switch (idx_reg->sel()) {
            case 1: kcache_index_mode = bim_zero; break;
            case 2: kcache_index_mode = bim_one; break;
            default:
               unreachable("Unsupported index mode");
            }
         } else {
            kcache_index_mode = bim_zero;
         }
         alu.src[i].kc_rel = kcache_index_mode;
      }

      if (ai.has_lds_queue_read()) {
         assert(m_bc->cf_last->nlds_read > 0);
         m_bc->cf_last->nlds_read--;
      }
   }

   if (ai.bank_swizzle() != alu_vec_unknown)
      alu.bank_swizzle_force = ai.bank_swizzle();

   alu.last = ai.has_alu_flag(alu_last_instr);
   alu.execute_mask = ai.has_alu_flag(alu_update_exec);

   /* If the destination register is equal to the last loaded address register
    * then clear the latter one, because the values will no longer be
    * identical */
   if (m_last_addr)
      sfn_log << SfnLog::assembly << "  Current address register is " << *m_last_addr
              << "\n";

   if (dst)
      sfn_log << SfnLog::assembly << "  Current dst register is " << *dst << "\n";

   auto cf_op = ai.cf_type();

   unsigned type = 0;
   switch (cf_op) {
   case cf_alu:
      type = CF_OP_ALU;
      break;
   case cf_alu_push_before:
      type = CF_OP_ALU_PUSH_BEFORE;
      break;
   case cf_alu_pop_after:
      type = CF_OP_ALU_POP_AFTER;
      break;
   case cf_alu_pop2_after:
      type = CF_OP_ALU_POP2_AFTER;
      break;
   case cf_alu_break:
      type = CF_OP_ALU_BREAK;
      break;
   case cf_alu_else_after:
      type = CF_OP_ALU_ELSE_AFTER;
      break;
   case cf_alu_continue:
      type = CF_OP_ALU_CONTINUE;
      break;
   case cf_alu_extended:
      type = CF_OP_ALU_EXT;
      break;
   default:
      assert(0 && "cf_alu_undefined should have been replaced");
   }

   if (alu.last)
      m_nliterals_in_group.clear();

   m_result = !r600_bytecode_add_alu_type(m_bc, &alu, type);

   if (unlikely(ai.opcode() == op1_mova_int)) {
      if (m_bc->gfx_level < CAYMAN || alu.dst.sel == 0) {
         m_bc->ar_loaded = 1;
      } else if (m_bc->gfx_level == CAYMAN) {
         int idx = alu.dst.sel - 2;
         m_bc->index_loaded[idx] = 1;
         m_bc->index_reg[idx] = -1;
      }
   }

   if (alu.dst.sel >= g_clause_local_start && alu.dst.sel < g_clause_local_end) {
      int clidx = 4 * (alu.dst.sel - g_clause_local_start) + alu.dst.chan;
      m_bc->cf_last->clause_local_written |= 1 << clidx;
   }

   if (ai.opcode() == op1_set_cf_idx0) {
      m_bc->index_loaded[0] = 1;
      m_bc->index_reg[0] = -1;
   }

   if (ai.opcode() == op1_set_cf_idx1) {
      m_bc->index_loaded[1] = 1;
      m_bc->index_reg[1] = -1;
   }
}

void
AssamblerVisitor::visit(const AluGroup& group)
{
   clear_states(sf_vtx | sf_tex);

   if (group.slots() == 0)
      return;

   static const unsigned slot_limit = 256;

   if (m_bc->cf_last && !m_bc->force_add_cf) {
      if (group.has_lds_group_start()) {
         if (m_bc->cf_last->ndw + 2 * (*group.begin())->required_slots() > slot_limit) {
            assert(m_bc->cf_last->nlds_read == 0);
            assert(0 && "Not allowed to start new alu group here");
            m_bc->force_add_cf = 1;
            m_last_addr = nullptr;
         }
      } else {
         if (m_bc->cf_last->ndw + 2 * group.slots() > slot_limit) {
            std::cerr << "m_bc->cf_last->ndw = " << m_bc->cf_last->ndw
                      << " group.slots() = " << group.slots()
                      << " -> " << m_bc->cf_last->ndw + 2 * group.slots()
                      << "> slot_limit = " << slot_limit << "\n";
            assert(m_bc->cf_last->nlds_read == 0);
            assert(0 && "Not allowed to start new alu group here");
            m_bc->force_add_cf = 1;
            m_last_addr = nullptr;
         } else {
            auto instr = *group.begin();
            if (instr && !instr->has_alu_flag(alu_is_lds) &&
                instr->opcode() == op0_group_barrier && m_bc->cf_last->ndw + 14 > slot_limit) {
               assert(0 && "Not allowed to start new alu group here");
               assert(m_bc->cf_last->nlds_read == 0);
               m_bc->force_add_cf = 1;
               m_last_addr = nullptr;
            }
         }
      }
   }

   auto [addr, is_index] = group.addr();

   if (addr) {
      if (!addr->has_flag(Register::addr_or_idx)) {
         if (is_index) {
            emit_index_reg(*addr, 0);
         } else {
            auto reg = addr->as_register();
            assert(reg);
            if (!m_last_addr || !m_bc->ar_loaded || !m_last_addr->equal_to(*reg)) {
               m_last_addr = reg;
               m_bc->ar_reg = reg->sel();
               m_bc->ar_chan = reg->chan();
               m_bc->ar_loaded = 0;
               r600_load_ar(m_bc, group.addr_for_src());
            }
         }
      }
   }

   for (auto& i : group) {
      if (i)
         i->accept(*this);
   }
}

void
AssamblerVisitor::visit(const TexInstr& tex_instr)
{
   clear_states(sf_vtx | sf_alu);

   if (tex_fetch_results.find(tex_instr.src().sel()) != tex_fetch_results.end()) {
      m_bc->force_add_cf = 1;
      tex_fetch_results.clear();
   }

   r600_bytecode_tex tex;
   memset(&tex, 0, sizeof(struct r600_bytecode_tex));
   tex.op = tex_instr.opcode();
   tex.sampler_id = tex_instr.sampler_id();
   tex.resource_id = tex_instr.resource_id();
   tex.src_gpr = tex_instr.src().sel();
   tex.dst_gpr = tex_instr.dst().sel();
   tex.dst_sel_x = tex_instr.dest_swizzle(0);
   tex.dst_sel_y = tex_instr.dest_swizzle(1);
   tex.dst_sel_z = tex_instr.dest_swizzle(2);
   tex.dst_sel_w = tex_instr.dest_swizzle(3);
   tex.src_sel_x = tex_instr.src()[0]->chan();
   tex.src_sel_y = tex_instr.src()[1]->chan();
   tex.src_sel_z = tex_instr.src()[2]->chan();
   tex.src_sel_w = tex_instr.src()[3]->chan();
   tex.coord_type_x = !tex_instr.has_tex_flag(TexInstr::x_unnormalized);
   tex.coord_type_y = !tex_instr.has_tex_flag(TexInstr::y_unnormalized);
   tex.coord_type_z = !tex_instr.has_tex_flag(TexInstr::z_unnormalized);
   tex.coord_type_w = !tex_instr.has_tex_flag(TexInstr::w_unnormalized);
   tex.offset_x = tex_instr.get_offset(0);
   tex.offset_y = tex_instr.get_offset(1);
   tex.offset_z = tex_instr.get_offset(2);
   tex.resource_index_mode = tex_instr.resource_index_mode();
   tex.sampler_index_mode = tex_instr.sampler_index_mode();

   if (tex.dst_sel_x < 4 && tex.dst_sel_y < 4 && tex.dst_sel_z < 4 && tex.dst_sel_w < 4)
      tex_fetch_results.insert(tex.dst_gpr);

   if (tex_instr.opcode() == TexInstr::get_gradient_h ||
       tex_instr.opcode() == TexInstr::get_gradient_v)
      tex.inst_mod = tex_instr.has_tex_flag(TexInstr::grad_fine) ? 1 : 0;
   else
      tex.inst_mod = tex_instr.inst_mode();
   if (r600_bytecode_add_tex(m_bc, &tex)) {
      R600_ASM_ERR("shader_from_nir: Error creating tex assembly instruction\n");
      m_result = false;
   }
}

void
AssamblerVisitor::visit(const ExportInstr& exi)
{
   const auto& value = exi.value();

   r600_bytecode_output output;
   memset(&output, 0, sizeof(output));

   output.gpr = value.sel();
   output.elem_size = 3;
   output.swizzle_x = value[0]->chan();
   output.swizzle_y = value[1]->chan();
   output.swizzle_z = value[2]->chan();
   output.burst_count = 1;
   output.op = exi.is_last_export() ? CF_OP_EXPORT_DONE : CF_OP_EXPORT;
   output.type = exi.export_type();

   clear_states(sf_all);
   switch (exi.export_type()) {
   case ExportInstr::pixel:
      output.swizzle_w = ps_alpha_to_one ? 5 : exi.value()[3]->chan();
      output.array_base = exi.location();
      break;
   case ExportInstr::pos:
      output.swizzle_w = exi.value()[3]->chan();
      output.array_base = 60 + exi.location();
      break;
   case ExportInstr::param:
      output.swizzle_w = exi.value()[3]->chan();
      output.array_base = exi.location();
      break;
   default:
      R600_ASM_ERR("shader_from_nir: export %d type not yet supported\n",
                   exi.export_type());
      m_result = false;
   }

   /* If all register elements pinned to fixed values
    * we can override the gpr (the register allocator doesn't see
    * this because it doesn't take these channels into account. */
   if (output.swizzle_x > 3 && output.swizzle_y > 3 && output.swizzle_z > 3 &&
       output.swizzle_w > 3)
      output.gpr = 0;

   int r = 0;
   if ((r = r600_bytecode_add_output(m_bc, &output))) {
      R600_ASM_ERR("Error adding export at location %d : err: %d\n", exi.location(), r);
      m_result = false;
   }
}

void
AssamblerVisitor::visit(const ScratchIOInstr& instr)
{
   clear_states(sf_all);

   struct r600_bytecode_output cf;

   memset(&cf, 0, sizeof(struct r600_bytecode_output));

   cf.op = CF_OP_MEM_SCRATCH;
   cf.elem_size = 3;
   cf.gpr = instr.value().sel();
   cf.mark = !instr.is_read();
   cf.comp_mask = instr.is_read() ? 0xf : instr.write_mask();
   cf.swizzle_x = 0;
   cf.swizzle_y = 1;
   cf.swizzle_z = 2;
   cf.swizzle_w = 3;
   cf.burst_count = 1;

   assert(!instr.is_read() || m_bc->gfx_level < R700);

   if (instr.address()) {
      cf.type = instr.is_read() || m_bc->gfx_level > R600 ? 3 : 1;
      cf.index_gpr = instr.address()->sel();

      /* The docu seems to be wrong here: In indirect addressing the
       * address_base seems to be the array_size */
      cf.array_size = instr.array_size();
   } else {
      cf.type = instr.is_read() || m_bc->gfx_level > R600 ? 2 : 0;
      cf.array_base = instr.location();
   }

   if (r600_bytecode_add_output(m_bc, &cf)) {
      R600_ASM_ERR("shader_from_nir: Error creating SCRATCH_WR assembly instruction\n");
      m_result = false;
   }
}

void
AssamblerVisitor::visit(const StreamOutInstr& instr)
{
   struct r600_bytecode_output output;
   memset(&output, 0, sizeof(struct r600_bytecode_output));

   output.gpr = instr.value().sel();
   output.elem_size = instr.element_size();
   output.array_base = instr.array_base();
   output.type = V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE;
   output.burst_count = instr.burst_count();
   output.array_size = instr.array_size();
   output.comp_mask = instr.comp_mask();
   output.op = instr.op(m_shader->bc.gfx_level);

   if (r600_bytecode_add_output(m_bc, &output)) {
      R600_ASM_ERR("shader_from_nir: Error creating stream output instruction\n");
      m_result = false;
   }
}

void
AssamblerVisitor::visit(const MemRingOutInstr& instr)
{
   struct r600_bytecode_output output;
   memset(&output, 0, sizeof(struct r600_bytecode_output));

   output.gpr = instr.value().sel();
   output.type = instr.type();
   output.elem_size = 3;
   output.comp_mask = 0xf;
   output.burst_count = 1;
   output.op = instr.op();
   if (instr.type() == MemRingOutInstr::mem_write_ind ||
       instr.type() == MemRingOutInstr::mem_write_ind_ack) {
      output.index_gpr = instr.index_reg();
      output.array_size = 0xfff;
   }
   output.array_base = instr.array_base();

   if (r600_bytecode_add_output(m_bc, &output)) {
      R600_ASM_ERR("shader_from_nir: Error creating mem ring write instruction\n");
      m_result = false;
   }
}

void
AssamblerVisitor::visit(const EmitVertexInstr& instr)
{
   int r = r600_bytecode_add_cfinst(m_bc, instr.op());
   if (!r)
      m_bc->cf_last->count = instr.stream();
   else
      m_result = false;
   assert(m_bc->cf_last->count < 4);
}

void
AssamblerVisitor::visit(const FetchInstr& fetch_instr)
{
   bool use_tc =
      fetch_instr.has_fetch_flag(FetchInstr::use_tc) || (m_bc->gfx_level == CAYMAN);

   auto clear_flags = use_tc ? sf_vtx : sf_tex;

   clear_states(clear_flags | sf_alu);

   if (fetch_instr.has_fetch_flag(FetchInstr::wait_ack))
      emit_wait_ack();


   if (!use_tc &&
       vtx_fetch_results.find(fetch_instr.src().sel()) != vtx_fetch_results.end()) {
      m_bc->force_add_cf = 1;
      vtx_fetch_results.clear();
   }

   if (fetch_instr.has_fetch_flag(FetchInstr::use_tc) &&
       tex_fetch_results.find(fetch_instr.src().sel()) != tex_fetch_results.end()) {
      m_bc->force_add_cf = 1;
      tex_fetch_results.clear();
   }

   if (use_tc)
      tex_fetch_results.insert(fetch_instr.dst().sel());
   else
      vtx_fetch_results.insert(fetch_instr.dst().sel());

   struct r600_bytecode_vtx vtx;
   memset(&vtx, 0, sizeof(vtx));
   vtx.op = fetch_instr.opcode();
   vtx.buffer_id = fetch_instr.resource_id();
   vtx.fetch_type = fetch_instr.fetch_type();
   vtx.src_gpr = fetch_instr.src().sel();
   vtx.src_sel_x = fetch_instr.src().chan();
   vtx.mega_fetch_count = fetch_instr.mega_fetch_count();
   vtx.dst_gpr = fetch_instr.dst().sel();
   vtx.dst_sel_x = fetch_instr.dest_swizzle(0); /* SEL_X */
   vtx.dst_sel_y = fetch_instr.dest_swizzle(1); /* SEL_Y */
   vtx.dst_sel_z = fetch_instr.dest_swizzle(2); /* SEL_Z */
   vtx.dst_sel_w = fetch_instr.dest_swizzle(3); /* SEL_W */
   vtx.use_const_fields = fetch_instr.has_fetch_flag(FetchInstr::use_const_field);
   vtx.data_format = fetch_instr.data_format();
   vtx.num_format_all = fetch_instr.num_format(); /* NUM_FORMAT_SCALED */
   vtx.format_comp_all = fetch_instr.has_fetch_flag(FetchInstr::format_comp_signed);
   vtx.endian = fetch_instr.endian_swap();
   vtx.buffer_index_mode = fetch_instr.resource_index_mode();
   vtx.offset = fetch_instr.src_offset();
   vtx.indexed = fetch_instr.has_fetch_flag(FetchInstr::indexed);
   vtx.uncached = fetch_instr.has_fetch_flag(FetchInstr::uncached);
   vtx.elem_size = fetch_instr.elm_size();
   vtx.array_base = fetch_instr.array_base();
   vtx.array_size = fetch_instr.array_size();
   vtx.srf_mode_all = fetch_instr.has_fetch_flag(FetchInstr::srf_mode);

   if (fetch_instr.has_fetch_flag(FetchInstr::use_tc)) {
      if ((r600_bytecode_add_vtx_tc(m_bc, &vtx))) {
         R600_ASM_ERR("shader_from_nir: Error creating tex assembly instruction\n");
         m_result = false;
      }

   } else {
      if ((r600_bytecode_add_vtx(m_bc, &vtx))) {
         R600_ASM_ERR("shader_from_nir: Error creating tex assembly instruction\n");
         m_result = false;
      }
   }

   m_bc->cf_last->vpm =
      (m_bc->type == PIPE_SHADER_FRAGMENT) && fetch_instr.has_fetch_flag(FetchInstr::vpm);
   m_bc->cf_last->barrier = 1;
}

void
AssamblerVisitor::visit(const WriteTFInstr& instr)
{
   struct r600_bytecode_gds gds;

   auto& value = instr.value();

   memset(&gds, 0, sizeof(struct r600_bytecode_gds));
   gds.src_gpr = value.sel();
   gds.src_sel_x = value[0]->chan();
   gds.src_sel_y = value[1]->chan();
   gds.src_sel_z = 4;
   gds.dst_sel_x = 7;
   gds.dst_sel_y = 7;
   gds.dst_sel_z = 7;
   gds.dst_sel_w = 7;
   gds.op = FETCH_OP_TF_WRITE;

   if (r600_bytecode_add_gds(m_bc, &gds) != 0) {
      m_result = false;
      return;
   }

   if (value[2]->chan() != 7) {
      memset(&gds, 0, sizeof(struct r600_bytecode_gds));
      gds.src_gpr = value.sel();
      gds.src_sel_x = value[2]->chan();
      gds.src_sel_y = value[3]->chan();
      gds.src_sel_z = 4;
      gds.dst_sel_x = 7;
      gds.dst_sel_y = 7;
      gds.dst_sel_z = 7;
      gds.dst_sel_w = 7;
      gds.op = FETCH_OP_TF_WRITE;

      if (r600_bytecode_add_gds(m_bc, &gds)) {
         m_result = false;
         return;
      }
   }
}

void
AssamblerVisitor::visit(const RatInstr& instr)
{
   struct r600_bytecode_gds gds;

   /* The instruction writes to the retuen buffer location, and
    * the value will actually be read back, so make sure all previous writes
    * have been finished */
   if (m_ack_suggested /*&& instr.has_instr_flag(Instr::ack_rat_return_write)*/)
      emit_wait_ack();

   int rat_idx = instr.resource_id();

   memset(&gds, 0, sizeof(struct r600_bytecode_gds));

   r600_bytecode_add_cfinst(m_bc, instr.cf_opcode());
   auto cf = m_bc->cf_last;
   cf->rat.id = rat_idx + m_shader->rat_base;
   cf->rat.inst = instr.rat_op();
   cf->rat.index_mode = instr.resource_index_mode();
   cf->output.type = instr.need_ack() ? 3 : 1;
   cf->output.gpr = instr.data_gpr();
   cf->output.index_gpr = instr.index_gpr();
   cf->output.comp_mask = instr.comp_mask();
   cf->output.burst_count = instr.burst_count();
   assert(instr.data_swz(0) == PIPE_SWIZZLE_X);
   if (cf->rat.inst != RatInstr::STORE_TYPED) {
      assert(instr.data_swz(1) == PIPE_SWIZZLE_Y ||
             instr.data_swz(1) == PIPE_SWIZZLE_MAX);
      assert(instr.data_swz(2) == PIPE_SWIZZLE_Z ||
             instr.data_swz(2) == PIPE_SWIZZLE_MAX);
   }

   cf->vpm = m_bc->type == PIPE_SHADER_FRAGMENT;
   cf->barrier = 1;
   cf->mark = instr.need_ack();
   cf->output.elem_size = instr.elm_size();

   m_ack_suggested |= instr.need_ack();
}

void
AssamblerVisitor::clear_states(const uint32_t& states)
{
   if (states & sf_vtx)
      vtx_fetch_results.clear();

   if (states & sf_tex)
      tex_fetch_results.clear();

   if (states & sf_alu) {
      m_last_op_was_barrier = false;
      m_last_addr = nullptr;
   }
}

void
AssamblerVisitor::visit(const Block& block)
{
   if (block.empty())
      return;

   if (block.has_instr_flag(Instr::force_cf)) {
      m_bc->force_add_cf = 1;
      m_bc->ar_loaded = 0;
      m_last_addr = nullptr;
   }
   sfn_log << SfnLog::assembly << "Translate block  size: " << block.size()
           << " new_cf:" << m_bc->force_add_cf << "\n";

   for (const auto& i : block) {
      sfn_log << SfnLog::assembly << "Translate " << *i << " ";
      i->accept(*this);
      sfn_log << SfnLog::assembly << (m_result ? "good" : "fail") << "\n";

      if (!m_result)
         break;
   }
}

void
AssamblerVisitor::visit(const IfInstr& instr)
{
   int elems = m_callstack.push(FC_PUSH_VPM);
   bool needs_workaround = false;

   if (m_bc->gfx_level == CAYMAN && m_bc->stack.loop > 1)
      needs_workaround = true;

   if (m_bc->gfx_level == EVERGREEN && m_bc->family != CHIP_HEMLOCK &&
       m_bc->family != CHIP_CYPRESS && m_bc->family != CHIP_JUNIPER) {
      unsigned dmod1 = (elems - 1) % m_bc->stack.entry_size;
      unsigned dmod2 = (elems) % m_bc->stack.entry_size;

      if (elems && (!dmod1 || !dmod2))
         needs_workaround = true;
   }

   auto pred = instr.predicate();
   auto [addr, dummy0, dummy1] = pred->indirect_addr();
   {
   }
   assert(!dummy1);
   if (addr) {
      if (!m_last_addr || !m_bc->ar_loaded || !m_last_addr->equal_to(*addr)) {
         m_bc->ar_reg = addr->sel();
         m_bc->ar_chan = addr->chan();
         m_last_addr = addr;
         m_bc->ar_loaded = 0;

         r600_load_ar(m_bc, true);
      }
   }

   if (needs_workaround) {
      r600_bytecode_add_cfinst(m_bc, CF_OP_PUSH);
      m_bc->cf_last->cf_addr = m_bc->cf_last->id + 2;
      r600_bytecode_add_cfinst(m_bc, CF_OP_ALU);
      pred->set_cf_type(cf_alu);
   }

   clear_states(sf_tex | sf_vtx);
   pred->accept(*this);

   r600_bytecode_add_cfinst(m_bc, CF_OP_JUMP);
   clear_states(sf_all);

   m_jump_tracker.push(m_bc->cf_last, jt_if);
}

void
AssamblerVisitor::visit(const ControlFlowInstr& instr)
{
   clear_states(sf_all);
   switch (instr.cf_type()) {
   case ControlFlowInstr::cf_else:
      emit_else();
      break;
   case ControlFlowInstr::cf_endif:
      emit_endif();
      break;
   case ControlFlowInstr::cf_loop_begin: {
      bool use_vpm = m_shader->processor_type == PIPE_SHADER_FRAGMENT &&
                     instr.has_instr_flag(Instr::vpm) &&
                     !instr.has_instr_flag(Instr::helper);
      emit_loop_begin(use_vpm);
      break;
   }
   case ControlFlowInstr::cf_loop_end:
      emit_loop_end();
      break;
   case ControlFlowInstr::cf_loop_break:
      emit_loop_break();
      break;
   case ControlFlowInstr::cf_loop_continue:
      emit_loop_cont();
      break;
   case ControlFlowInstr::cf_wait_ack: {
      int r = r600_bytecode_add_cfinst(m_bc, CF_OP_WAIT_ACK);
      if (!r) {
         m_bc->cf_last->cf_addr = 0;
         m_bc->cf_last->barrier = 1;
         m_ack_suggested = false;
      } else {
         m_result = false;
      }
   } break;
   default:
      unreachable("Unknown CF instruction type");
   }
}

void
AssamblerVisitor::visit(const GDSInstr& instr)
{
   struct r600_bytecode_gds gds;

   memset(&gds, 0, sizeof(struct r600_bytecode_gds));

   gds.op = ds_opcode_map.at(instr.opcode());
   gds.uav_id = instr.resource_id();
   gds.uav_index_mode = instr.resource_index_mode();
   gds.src_gpr = instr.src().sel();

   gds.src_sel_x = instr.src()[0]->chan() < 7 ? instr.src()[0]->chan() : 4;
   gds.src_sel_y = instr.src()[1]->chan() < 7 ? instr.src()[1]->chan() : 4;
   gds.src_sel_z = instr.src()[2]->chan() < 7 ? instr.src()[2]->chan() : 4;

   gds.dst_sel_x = 7;
   gds.dst_sel_y = 7;
   gds.dst_sel_z = 7;
   gds.dst_sel_w = 7;

   if (instr.dest()) {
      gds.dst_gpr = instr.dest()->sel();
      switch (instr.dest()->chan()) {
      case 0:
         gds.dst_sel_x = 0;
         break;
      case 1:
         gds.dst_sel_y = 0;
         break;
      case 2:
         gds.dst_sel_z = 0;
         break;
      case 3:
         gds.dst_sel_w = 0;
      }
   }

   gds.src_gpr2 = 0;
   gds.alloc_consume = m_bc->gfx_level < CAYMAN ? 1 : 0; // Not Cayman

   int r = r600_bytecode_add_gds(m_bc, &gds);
   if (r) {
      m_result = false;
      return;
   }
   m_bc->cf_last->vpm = PIPE_SHADER_FRAGMENT == m_bc->type;
   m_bc->cf_last->barrier = 1;
}

void
AssamblerVisitor::visit(const LDSAtomicInstr& instr)
{
   (void)instr;
   unreachable("LDSAtomicInstr must be lowered to ALUInstr");
}

void
AssamblerVisitor::visit(const LDSReadInstr& instr)
{
   (void)instr;
   unreachable("LDSReadInstr must be lowered to ALUInstr");
}

EBufferIndexMode
AssamblerVisitor::emit_index_reg(const VirtualValue& addr, unsigned idx)
{
   assert(idx < 2);

   if (!m_bc->index_loaded[idx] || m_loop_nesting ||
       m_bc->index_reg[idx] != (unsigned)addr.sel() ||
       m_bc->index_reg_chan[idx] != (unsigned)addr.chan()) {
      struct r600_bytecode_alu alu;

      // Make sure MOVA is not last instr in clause

      if (!m_bc->cf_last || (m_bc->cf_last->ndw >> 1) >= 110)
         m_bc->force_add_cf = 1;

      if (m_bc->gfx_level != CAYMAN) {

         EAluOp idxop = idx ? op1_set_cf_idx1 : op1_set_cf_idx0;

         memset(&alu, 0, sizeof(alu));
         alu.op = opcode_map.at(op1_mova_int);
         alu.dst.chan = 0;
         alu.src[0].sel = addr.sel();
         alu.src[0].chan = addr.chan();
         alu.last = 1;
         sfn_log << SfnLog::assembly << "   mova_int, ";
         int r = r600_bytecode_add_alu(m_bc, &alu);
         if (r)
            return bim_invalid;

         alu.op = opcode_map.at(idxop);
         alu.dst.chan = 0;
         alu.src[0].sel = 0;
         alu.src[0].chan = 0;
         alu.last = 1;
         sfn_log << SfnLog::assembly << "op1_set_cf_idx" << idx;
         r = r600_bytecode_add_alu(m_bc, &alu);
         if (r)
            return bim_invalid;
      } else {
         memset(&alu, 0, sizeof(alu));
         alu.op = opcode_map.at(op1_mova_int);
         alu.dst.sel = idx == 0 ? CM_V_SQ_MOVA_DST_CF_IDX0 : CM_V_SQ_MOVA_DST_CF_IDX1;
         alu.dst.chan = 0;
         alu.src[0].sel = addr.sel();
         alu.src[0].chan = addr.chan();
         alu.last = 1;
         sfn_log << SfnLog::assembly << "   mova_int, ";
         int r = r600_bytecode_add_alu(m_bc, &alu);
         if (r)
            return bim_invalid;
      }

      m_bc->ar_loaded = 0;
      m_bc->index_reg[idx] = addr.sel();
      m_bc->index_reg_chan[idx] = addr.chan();
      m_bc->index_loaded[idx] = true;
      m_bc->force_add_cf = 1;
      sfn_log << SfnLog::assembly << "\n";
   }
   return idx == 0 ? bim_zero : bim_one;
}

void
AssamblerVisitor::emit_else()
{
   r600_bytecode_add_cfinst(m_bc, CF_OP_ELSE);
   m_bc->cf_last->pop_count = 1;
   m_result &= m_jump_tracker.add_mid(m_bc->cf_last, jt_if);
}

void
AssamblerVisitor::emit_endif()
{
   m_callstack.pop(FC_PUSH_VPM);

   unsigned force_pop = m_bc->force_add_cf;
   if (!force_pop) {
      int alu_pop = 3;
      if (m_bc->cf_last) {
         if (m_bc->cf_last->op == CF_OP_ALU)
            alu_pop = 0;
         else if (m_bc->cf_last->op == CF_OP_ALU_POP_AFTER)
            alu_pop = 1;
      }
      alu_pop += 1;
      if (alu_pop == 1) {
         m_bc->cf_last->op = CF_OP_ALU_POP_AFTER;
         m_bc->force_add_cf = 1;
      } else {
         force_pop = 1;
      }
   }

   if (force_pop) {
      r600_bytecode_add_cfinst(m_bc, CF_OP_POP);
      m_bc->cf_last->pop_count = 1;
      m_bc->cf_last->cf_addr = m_bc->cf_last->id + 2;
   }

   m_result &= m_jump_tracker.pop(m_bc->cf_last, jt_if);
}

void
AssamblerVisitor::emit_loop_begin(bool vpm)
{
   r600_bytecode_add_cfinst(m_bc, CF_OP_LOOP_START_DX10);
   m_bc->cf_last->vpm = vpm && m_bc->type == PIPE_SHADER_FRAGMENT;
   m_jump_tracker.push(m_bc->cf_last, jt_loop);
   m_callstack.push(FC_LOOP);
   ++m_loop_nesting;
}

void
AssamblerVisitor::emit_loop_end()
{
   if (m_ack_suggested) {
      emit_wait_ack();
      m_ack_suggested = false;
   }

   r600_bytecode_add_cfinst(m_bc, CF_OP_LOOP_END);
   m_callstack.pop(FC_LOOP);
   assert(m_loop_nesting);
   --m_loop_nesting;
   m_result |= m_jump_tracker.pop(m_bc->cf_last, jt_loop);
}

void
AssamblerVisitor::emit_loop_break()
{
   r600_bytecode_add_cfinst(m_bc, CF_OP_LOOP_BREAK);
   m_result |= m_jump_tracker.add_mid(m_bc->cf_last, jt_loop);
}

void
AssamblerVisitor::emit_loop_cont()
{
   r600_bytecode_add_cfinst(m_bc, CF_OP_LOOP_CONTINUE);
   m_result |= m_jump_tracker.add_mid(m_bc->cf_last, jt_loop);
}

bool
AssamblerVisitor::copy_dst(r600_bytecode_alu_dst& dst, const Register& d, bool write)
{
   if (write && d.sel() > g_clause_local_end) {
      R600_ASM_ERR("shader_from_nir: Don't support more then 123 GPRs + 4 clause "
                   "local, but try using %d\n",
                   d.sel());
      m_result = false;
      return false;
   }

   dst.sel = d.sel();
   dst.chan = d.chan();

   if (m_last_addr && m_last_addr->equal_to(d))
      m_last_addr = nullptr;

   for (int i = 0; i < 2; ++i) {
      /* Force emitting index register, if we didn't emit it yet, because
       * the register value will change now */
      if (dst.sel == m_bc->index_reg[i] && dst.chan == m_bc->index_reg_chan[i])
         m_bc->index_loaded[i] = false;
   }

   return true;
}

void
AssamblerVisitor::emit_wait_ack()
{
   int r = r600_bytecode_add_cfinst(m_bc, CF_OP_WAIT_ACK);
   if (!r) {
      m_bc->cf_last->cf_addr = 0;
      m_bc->cf_last->barrier = 1;
      m_ack_suggested = false;
   } else
      m_result = false;
}

class EncodeSourceVisitor : public ConstRegisterVisitor {
public:
   EncodeSourceVisitor(r600_bytecode_alu_src& s, r600_bytecode *bc);
   void visit(const Register& value) override;
   void visit(const LocalArray& value) override;
   void visit(const LocalArrayValue& value) override;
   void visit(const UniformValue& value) override;
   void visit(const LiteralConstant& value) override;
   void visit(const InlineConstant& value) override;

   r600_bytecode_alu_src& src;
   r600_bytecode *m_bc;
   PVirtualValue m_buffer_offset{nullptr};
};

PVirtualValue
AssamblerVisitor::copy_src(r600_bytecode_alu_src& src, const VirtualValue& s)
{

   EncodeSourceVisitor visitor(src, m_bc);
   src.sel = s.sel();
   src.chan = s.chan();

   if (s.sel() >= g_clause_local_start && s.sel() < g_clause_local_end ) {
      assert(m_bc->cf_last);
      int clidx = 4 * (s.sel() - g_clause_local_start) + s.chan();
      /* Ensure that the clause local register was already written */
      assert(m_bc->cf_last->clause_local_written & (1 << clidx));
   }

   s.accept(visitor);
   return visitor.m_buffer_offset;
}

EncodeSourceVisitor::EncodeSourceVisitor(r600_bytecode_alu_src& s, r600_bytecode *bc):
    src(s),
    m_bc(bc)
{
}

void
EncodeSourceVisitor::visit(const Register& value)
{
   assert(value.sel() < g_clause_local_end && "Only have 123 reisters + 4 clause local");
}

void
EncodeSourceVisitor::visit(const LocalArray& value)
{
   (void)value;
   unreachable("An array can't be a source register");
}

void
EncodeSourceVisitor::visit(const LocalArrayValue& value)
{
   src.rel = value.addr() ? 1 : 0;
}

void
EncodeSourceVisitor::visit(const UniformValue& value)
{
   assert(value.sel() >= 512 && "Uniform values must have a sel >= 512");
   m_buffer_offset = value.buf_addr();
   src.kc_bank = value.kcache_bank();
}

void
EncodeSourceVisitor::visit(const LiteralConstant& value)
{
   src.value = value.value();
}

void
EncodeSourceVisitor::visit(const InlineConstant& value)
{
   (void)value;
}

const std::map<EAluOp, int> opcode_map = {

   {op2_add,                       ALU_OP2_ADD                      },
   {op2_mul,                       ALU_OP2_MUL                      },
   {op2_mul_ieee,                  ALU_OP2_MUL_IEEE                 },
   {op2_max,                       ALU_OP2_MAX                      },
   {op2_min,                       ALU_OP2_MIN                      },
   {op2_max_dx10,                  ALU_OP2_MAX_DX10                 },
   {op2_min_dx10,                  ALU_OP2_MIN_DX10                 },
   {op2_sete,                      ALU_OP2_SETE                     },
   {op2_setgt,                     ALU_OP2_SETGT                    },
   {op2_setge,                     ALU_OP2_SETGE                    },
   {op2_setne,                     ALU_OP2_SETNE                    },
   {op2_sete_dx10,                 ALU_OP2_SETE_DX10                },
   {op2_setgt_dx10,                ALU_OP2_SETGT_DX10               },
   {op2_setge_dx10,                ALU_OP2_SETGE_DX10               },
   {op2_setne_dx10,                ALU_OP2_SETNE_DX10               },
   {op1_fract,                     ALU_OP1_FRACT                    },
   {op1_trunc,                     ALU_OP1_TRUNC                    },
   {op1_ceil,                      ALU_OP1_CEIL                     },
   {op1_rndne,                     ALU_OP1_RNDNE                    },
   {op1_floor,                     ALU_OP1_FLOOR                    },
   {op2_ashr_int,                  ALU_OP2_ASHR_INT                 },
   {op2_lshr_int,                  ALU_OP2_LSHR_INT                 },
   {op2_lshl_int,                  ALU_OP2_LSHL_INT                 },
   {op1_mov,                       ALU_OP1_MOV                      },
   {op0_nop,                       ALU_OP0_NOP                      },
   {op2_mul_64,                    ALU_OP2_MUL_64                   },
   {op1v_flt64_to_flt32,           ALU_OP1_FLT64_TO_FLT32           },
   {op1v_flt32_to_flt64,           ALU_OP1_FLT32_TO_FLT64           },
   {op2_prede_int,                 ALU_OP2_PRED_SETE_INT            },
   {op2_pred_setne_int,            ALU_OP2_PRED_SETNE_INT           },
   {op2_pred_setge_int,            ALU_OP2_PRED_SETGE_INT           },
   {op2_pred_setgt_int,            ALU_OP2_PRED_SETGT_INT           },
   {op2_pred_setgt_uint,           ALU_OP2_PRED_SETGT_UINT          },
   {op2_pred_setge_uint,           ALU_OP2_PRED_SETGE_UINT          },
   {op2_pred_sete,                 ALU_OP2_PRED_SETE                },
   {op2_pred_setgt,                ALU_OP2_PRED_SETGT               },
   {op2_pred_setge,                ALU_OP2_PRED_SETGE               },
   {op2_pred_setne,                ALU_OP2_PRED_SETNE               },
   {op0_pred_set_clr,              ALU_OP0_PRED_SET_CLR             },
   {op1_pred_set_restore,          ALU_OP1_PRED_SET_RESTORE         },
   {op2_pred_sete_push,            ALU_OP2_PRED_SETE_PUSH           },
   {op2_pred_setgt_push,           ALU_OP2_PRED_SETGT_PUSH          },
   {op2_pred_setge_push,           ALU_OP2_PRED_SETGE_PUSH          },
   {op2_pred_setne_push,           ALU_OP2_PRED_SETNE_PUSH          },
   {op2_kille,                     ALU_OP2_KILLE                    },
   {op2_killgt,                    ALU_OP2_KILLGT                   },
   {op2_killge,                    ALU_OP2_KILLGE                   },
   {op2_killne,                    ALU_OP2_KILLNE                   },
   {op2_and_int,                   ALU_OP2_AND_INT                  },
   {op2_or_int,                    ALU_OP2_OR_INT                   },
   {op2_xor_int,                   ALU_OP2_XOR_INT                  },
   {op1_not_int,                   ALU_OP1_NOT_INT                  },
   {op2_add_int,                   ALU_OP2_ADD_INT                  },
   {op2_sub_int,                   ALU_OP2_SUB_INT                  },
   {op2_max_int,                   ALU_OP2_MAX_INT                  },
   {op2_min_int,                   ALU_OP2_MIN_INT                  },
   {op2_max_uint,                  ALU_OP2_MAX_UINT                 },
   {op2_min_uint,                  ALU_OP2_MIN_UINT                 },
   {op2_sete_int,                  ALU_OP2_SETE_INT                 },
   {op2_setgt_int,                 ALU_OP2_SETGT_INT                },
   {op2_setge_int,                 ALU_OP2_SETGE_INT                },
   {op2_setne_int,                 ALU_OP2_SETNE_INT                },
   {op2_setgt_uint,                ALU_OP2_SETGT_UINT               },
   {op2_setge_uint,                ALU_OP2_SETGE_UINT               },
   {op2_killgt_uint,               ALU_OP2_KILLGT_UINT              },
   {op2_killge_uint,               ALU_OP2_KILLGE_UINT              },
   {op2_pred_setgt_int,            ALU_OP2_PRED_SETGT_INT           },
   {op2_pred_setge_int,            ALU_OP2_PRED_SETGE_INT           },
   {op2_pred_setne_int,            ALU_OP2_PRED_SETNE_INT           },
   {op2_kille_int,                 ALU_OP2_KILLE_INT                },
   {op2_killgt_int,                ALU_OP2_KILLGT_INT               },
   {op2_killge_int,                ALU_OP2_KILLGE_INT               },
   {op2_killne_int,                ALU_OP2_KILLNE_INT               },
   {op2_pred_sete_push_int,        ALU_OP2_PRED_SETE_PUSH_INT       },
   {op2_pred_setgt_push_int,       ALU_OP2_PRED_SETGT_PUSH_INT      },
   {op2_pred_setge_push_int,       ALU_OP2_PRED_SETGE_PUSH_INT      },
   {op2_pred_setne_push_int,       ALU_OP2_PRED_SETNE_PUSH_INT      },
   {op2_pred_setlt_push_int,       ALU_OP2_PRED_SETLT_PUSH_INT      },
   {op2_pred_setle_push_int,       ALU_OP2_PRED_SETLE_PUSH_INT      },
   {op1_flt_to_int,                ALU_OP1_FLT_TO_INT               },
   {op1_bfrev_int,                 ALU_OP1_BFREV_INT                },
   {op2_addc_uint,                 ALU_OP2_ADDC_UINT                },
   {op2_subb_uint,                 ALU_OP2_SUBB_UINT                },
   {op0_group_barrier,             ALU_OP0_GROUP_BARRIER            },
   {op0_group_seq_begin,           ALU_OP0_GROUP_SEQ_BEGIN          },
   {op0_group_seq_end,             ALU_OP0_GROUP_SEQ_END            },
   {op2_set_mode,                  ALU_OP2_SET_MODE                 },
   {op1_set_cf_idx0,               ALU_OP0_SET_CF_IDX0              },
   {op1_set_cf_idx1,               ALU_OP0_SET_CF_IDX1              },
   {op2_set_lds_size,              ALU_OP2_SET_LDS_SIZE             },
   {op1_exp_ieee,                  ALU_OP1_EXP_IEEE                 },
   {op1_log_clamped,               ALU_OP1_LOG_CLAMPED              },
   {op1_log_ieee,                  ALU_OP1_LOG_IEEE                 },
   {op1_recip_clamped,             ALU_OP1_RECIP_CLAMPED            },
   {op1_recip_ff,                  ALU_OP1_RECIP_FF                 },
   {op1_recip_ieee,                ALU_OP1_RECIP_IEEE               },
   {op1_recipsqrt_clamped,         ALU_OP1_RECIPSQRT_CLAMPED        },
   {op1_recipsqrt_ff,              ALU_OP1_RECIPSQRT_FF             },
   {op1_recipsqrt_ieee1,           ALU_OP1_RECIPSQRT_IEEE           },
   {op1_sqrt_ieee,                 ALU_OP1_SQRT_IEEE                },
   {op1_sin,                       ALU_OP1_SIN                      },
   {op1_cos,                       ALU_OP1_COS                      },
   {op2_mullo_int,                 ALU_OP2_MULLO_INT                },
   {op2_mulhi_int,                 ALU_OP2_MULHI_INT                },
   {op2_mullo_uint,                ALU_OP2_MULLO_UINT               },
   {op2_mulhi_uint,                ALU_OP2_MULHI_UINT               },
   {op1_recip_int,                 ALU_OP1_RECIP_INT                },
   {op1_recip_uint,                ALU_OP1_RECIP_UINT               },
   {op1_recip_64,                  ALU_OP2_RECIP_64                 },
   {op1_recip_clamped_64,          ALU_OP2_RECIP_CLAMPED_64         },
   {op1_recipsqrt_64,              ALU_OP2_RECIPSQRT_64             },
   {op1_recipsqrt_clamped_64,      ALU_OP2_RECIPSQRT_CLAMPED_64     },
   {op1_sqrt_64,                   ALU_OP2_SQRT_64                  },
   {op1_flt_to_uint,               ALU_OP1_FLT_TO_UINT              },
   {op1_int_to_flt,                ALU_OP1_INT_TO_FLT               },
   {op1_uint_to_flt,               ALU_OP1_UINT_TO_FLT              },
   {op2_bfm_int,                   ALU_OP2_BFM_INT                  },
   {op1_flt32_to_flt16,            ALU_OP1_FLT32_TO_FLT16           },
   {op1_flt16_to_flt32,            ALU_OP1_FLT16_TO_FLT32           },
   {op1_ubyte0_flt,                ALU_OP1_UBYTE0_FLT               },
   {op1_ubyte1_flt,                ALU_OP1_UBYTE1_FLT               },
   {op1_ubyte2_flt,                ALU_OP1_UBYTE2_FLT               },
   {op1_ubyte3_flt,                ALU_OP1_UBYTE3_FLT               },
   {op1_bcnt_int,                  ALU_OP1_BCNT_INT                 },
   {op1_ffbh_uint,                 ALU_OP1_FFBH_UINT                },
   {op1_ffbl_int,                  ALU_OP1_FFBL_INT                 },
   {op1_ffbh_int,                  ALU_OP1_FFBH_INT                 },
   {op1_flt_to_uint4,              ALU_OP1_FLT_TO_UINT4             },
   {op2_dot_ieee,                  ALU_OP2_DOT_IEEE                 },
   {op1_flt_to_int_rpi,            ALU_OP1_FLT_TO_INT_RPI           },
   {op1_flt_to_int_floor,          ALU_OP1_FLT_TO_INT_FLOOR         },
   {op2_mulhi_uint24,              ALU_OP2_MULHI_UINT24             },
   {op1_mbcnt_32hi_int,            ALU_OP1_MBCNT_32HI_INT           },
   {op1_offset_to_flt,             ALU_OP1_OFFSET_TO_FLT            },
   {op2_mul_uint24,                ALU_OP2_MUL_UINT24               },
   {op1_bcnt_accum_prev_int,       ALU_OP1_BCNT_ACCUM_PREV_INT      },
   {op1_mbcnt_32lo_accum_prev_int, ALU_OP1_MBCNT_32LO_ACCUM_PREV_INT},
   {op2_sete_64,                   ALU_OP2_SETE_64                  },
   {op2_setne_64,                  ALU_OP2_SETNE_64                 },
   {op2_setgt_64,                  ALU_OP2_SETGT_64                 },
   {op2_setge_64,                  ALU_OP2_SETGE_64                 },
   {op2_min_64,                    ALU_OP2_MIN_64                   },
   {op2_max_64,                    ALU_OP2_MAX_64                   },
   {op2_dot4,                      ALU_OP2_DOT4                     },
   {op2_dot4_ieee,                 ALU_OP2_DOT4_IEEE                },
   {op2_cube,                      ALU_OP2_CUBE                     },
   {op1_max4,                      ALU_OP1_MAX4                     },
   {op1_frexp_64,                  ALU_OP1_FREXP_64                 },
   {op1_ldexp_64,                  ALU_OP2_LDEXP_64                 },
   {op1_fract_64,                  ALU_OP1_FRACT_64                 },
   {op2_pred_setgt_64,             ALU_OP2_PRED_SETGT_64            },
   {op2_pred_sete_64,              ALU_OP2_PRED_SETE_64             },
   {op2_pred_setge_64,             ALU_OP2_PRED_SETGE_64            },
   {op2_add_64,                    ALU_OP2_ADD_64                   },
   {op1_mova_int,                  ALU_OP1_MOVA_INT                 },
   {op1v_flt64_to_flt32,           ALU_OP1_FLT64_TO_FLT32           },
   {op1_flt32_to_flt64,            ALU_OP1_FLT32_TO_FLT64           },
   {op2_sad_accum_prev_uint,       ALU_OP2_SAD_ACCUM_PREV_UINT      },
   {op2_dot,                       ALU_OP2_DOT                      },
   {op1_mul_prev,                  ALU_OP1_MUL_PREV                 },
   {op1_mul_ieee_prev,             ALU_OP1_MUL_IEEE_PREV            },
   {op1_add_prev,                  ALU_OP1_ADD_PREV                 },
   {op2_muladd_prev,               ALU_OP2_MULADD_PREV              },
   {op2_muladd_ieee_prev,          ALU_OP2_MULADD_IEEE_PREV         },
   {op2_interp_xy,                 ALU_OP2_INTERP_XY                },
   {op2_interp_zw,                 ALU_OP2_INTERP_ZW                },
   {op2_interp_x,                  ALU_OP2_INTERP_X                 },
   {op2_interp_z,                  ALU_OP2_INTERP_Z                 },
   {op0_store_flags,               ALU_OP1_STORE_FLAGS              },
   {op1_load_store_flags,          ALU_OP1_LOAD_STORE_FLAGS         },
   {op0_lds_1a,                    ALU_OP2_LDS_1A                   },
   {op0_lds_1a1d,                  ALU_OP2_LDS_1A1D                 },
   {op0_lds_2a,                    ALU_OP2_LDS_2A                   },
   {op1_interp_load_p0,            ALU_OP1_INTERP_LOAD_P0           },
   {op1_interp_load_p10,           ALU_OP1_INTERP_LOAD_P10          },
   {op1_interp_load_p20,           ALU_OP1_INTERP_LOAD_P20          },
   {op3_bfe_uint,                  ALU_OP3_BFE_UINT                 },
   {op3_bfe_int,                   ALU_OP3_BFE_INT                  },
   {op3_bfi_int,                   ALU_OP3_BFI_INT                  },
   {op3_fma,                       ALU_OP3_FMA                      },
   {op3_cndne_64,                  ALU_OP3_CNDNE_64                 },
   {op3_fma_64,                    ALU_OP3_FMA_64                   },
   {op3_lerp_uint,                 ALU_OP3_LERP_UINT                },
   {op3_bit_align_int,             ALU_OP3_BIT_ALIGN_INT            },
   {op3_byte_align_int,            ALU_OP3_BYTE_ALIGN_INT           },
   {op3_sad_accum_uint,            ALU_OP3_SAD_ACCUM_UINT           },
   {op3_sad_accum_hi_uint,         ALU_OP3_SAD_ACCUM_HI_UINT        },
   {op3_muladd_uint24,             ALU_OP3_MULADD_UINT24            },
   {op3_lds_idx_op,                ALU_OP3_LDS_IDX_OP               },
   {op3_muladd,                    ALU_OP3_MULADD                   },
   {op3_muladd_m2,                 ALU_OP3_MULADD_M2                },
   {op3_muladd_m4,                 ALU_OP3_MULADD_M4                },
   {op3_muladd_d2,                 ALU_OP3_MULADD_D2                },
   {op3_muladd_ieee,               ALU_OP3_MULADD_IEEE              },
   {op3_cnde,                      ALU_OP3_CNDE                     },
   {op3_cndgt,                     ALU_OP3_CNDGT                    },
   {op3_cndge,                     ALU_OP3_CNDGE                    },
   {op3_cnde_int,                  ALU_OP3_CNDE_INT                 },
   {op3_cndgt_int,                 ALU_OP3_CNDGT_INT                },
   {op3_cndge_int,                 ALU_OP3_CNDGE_INT                },
   {op3_mul_lit,                   ALU_OP3_MUL_LIT                  },
};

const std::map<ESDOp, int> ds_opcode_map = {
   {DS_OP_ADD,                      FETCH_OP_GDS_ADD                 },
   {DS_OP_SUB,                      FETCH_OP_GDS_SUB                 },
   {DS_OP_RSUB,                     FETCH_OP_GDS_RSUB                },
   {DS_OP_INC,                      FETCH_OP_GDS_INC                 },
   {DS_OP_DEC,                      FETCH_OP_GDS_DEC                 },
   {DS_OP_MIN_INT,                  FETCH_OP_GDS_MIN_INT             },
   {DS_OP_MAX_INT,                  FETCH_OP_GDS_MAX_INT             },
   {DS_OP_MIN_UINT,                 FETCH_OP_GDS_MIN_UINT            },
   {DS_OP_MAX_UINT,                 FETCH_OP_GDS_MAX_UINT            },
   {DS_OP_AND,                      FETCH_OP_GDS_AND                 },
   {DS_OP_OR,                       FETCH_OP_GDS_OR                  },
   {DS_OP_XOR,                      FETCH_OP_GDS_XOR                 },
   {DS_OP_MSKOR,                    FETCH_OP_GDS_MSKOR               },
   {DS_OP_WRITE,                    FETCH_OP_GDS_WRITE               },
   {DS_OP_WRITE_REL,                FETCH_OP_GDS_WRITE_REL           },
   {DS_OP_WRITE2,                   FETCH_OP_GDS_WRITE2              },
   {DS_OP_CMP_STORE,                FETCH_OP_GDS_CMP_STORE           },
   {DS_OP_CMP_STORE_SPF,            FETCH_OP_GDS_CMP_STORE_SPF       },
   {DS_OP_BYTE_WRITE,               FETCH_OP_GDS_BYTE_WRITE          },
   {DS_OP_SHORT_WRITE,              FETCH_OP_GDS_SHORT_WRITE         },
   {DS_OP_ADD_RET,                  FETCH_OP_GDS_ADD_RET             },
   {DS_OP_SUB_RET,                  FETCH_OP_GDS_SUB_RET             },
   {DS_OP_RSUB_RET,                 FETCH_OP_GDS_RSUB_RET            },
   {DS_OP_INC_RET,                  FETCH_OP_GDS_INC_RET             },
   {DS_OP_DEC_RET,                  FETCH_OP_GDS_DEC_RET             },
   {DS_OP_MIN_INT_RET,              FETCH_OP_GDS_MIN_INT_RET         },
   {DS_OP_MAX_INT_RET,              FETCH_OP_GDS_MAX_INT_RET         },
   {DS_OP_MIN_UINT_RET,             FETCH_OP_GDS_MIN_UINT_RET        },
   {DS_OP_MAX_UINT_RET,             FETCH_OP_GDS_MAX_UINT_RET        },
   {DS_OP_AND_RET,                  FETCH_OP_GDS_AND_RET             },
   {DS_OP_OR_RET,                   FETCH_OP_GDS_OR_RET              },
   {DS_OP_XOR_RET,                  FETCH_OP_GDS_XOR_RET             },
   {DS_OP_MSKOR_RET,                FETCH_OP_GDS_MSKOR_RET           },
   {DS_OP_XCHG_RET,                 FETCH_OP_GDS_XCHG_RET            },
   {DS_OP_XCHG_REL_RET,             FETCH_OP_GDS_XCHG_REL_RET        },
   {DS_OP_XCHG2_RET,                FETCH_OP_GDS_XCHG2_RET           },
   {DS_OP_CMP_XCHG_RET,             FETCH_OP_GDS_CMP_XCHG_RET        },
   {DS_OP_CMP_XCHG_SPF_RET,         FETCH_OP_GDS_CMP_XCHG_SPF_RET    },
   {DS_OP_READ_RET,                 FETCH_OP_GDS_READ_RET            },
   {DS_OP_READ_REL_RET,             FETCH_OP_GDS_READ_REL_RET        },
   {DS_OP_READ2_RET,                FETCH_OP_GDS_READ2_RET           },
   {DS_OP_READWRITE_RET,            FETCH_OP_GDS_READWRITE_RET       },
   {DS_OP_BYTE_READ_RET,            FETCH_OP_GDS_BYTE_READ_RET       },
   {DS_OP_UBYTE_READ_RET,           FETCH_OP_GDS_UBYTE_READ_RET      },
   {DS_OP_SHORT_READ_RET,           FETCH_OP_GDS_SHORT_READ_RET      },
   {DS_OP_USHORT_READ_RET,          FETCH_OP_GDS_USHORT_READ_RET     },
   {DS_OP_ATOMIC_ORDERED_ALLOC_RET, FETCH_OP_GDS_ATOMIC_ORDERED_ALLOC},
   {DS_OP_INVALID,                  0                                },
};

} // namespace r600
