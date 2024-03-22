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

#include "sfn_scheduler.h"

#include "../r600_isa.h"

#include "amd_family.h"
#include "sfn_alu_defines.h"
#include "sfn_debug.h"
#include "sfn_instr_alugroup.h"
#include "sfn_instr_controlflow.h"
#include "sfn_instr_export.h"
#include "sfn_instr_fetch.h"
#include "sfn_instr_lds.h"
#include "sfn_instr_mem.h"
#include "sfn_instr_tex.h"

#include <algorithm>
#include <sstream>

namespace r600 {

class CollectInstructions : public InstrVisitor {

public:
   CollectInstructions(ValueFactory& vf):
       m_value_factory(vf)
   {
   }

   void visit(AluInstr *instr) override
   {
      if (instr->has_alu_flag(alu_is_trans))
         alu_trans.push_back(instr);
      else {
         if (instr->alu_slots() == 1)
            alu_vec.push_back(instr);
         else
            alu_groups.push_back(instr->split(m_value_factory));
      }
   }
   void visit(AluGroup *instr) override { alu_groups.push_back(instr); }
   void visit(TexInstr *instr) override { tex.push_back(instr); }
   void visit(ExportInstr *instr) override { exports.push_back(instr); }
   void visit(FetchInstr *instr) override { fetches.push_back(instr); }
   void visit(Block *instr) override
   {
      for (auto& i : *instr)
         i->accept(*this);
   }

   void visit(ControlFlowInstr *instr) override
   {
      assert(!m_cf_instr);
      m_cf_instr = instr;
   }

   void visit(IfInstr *instr) override
   {
      assert(!m_cf_instr);
      m_cf_instr = instr;
   }

   void visit(EmitVertexInstr *instr) override
   {
      assert(!m_cf_instr);
      m_cf_instr = instr;
   }

   void visit(ScratchIOInstr *instr) override { mem_write_instr.push_back(instr); }

   void visit(StreamOutInstr *instr) override { mem_write_instr.push_back(instr); }

   void visit(MemRingOutInstr *instr) override { mem_ring_writes.push_back(instr); }

   void visit(GDSInstr *instr) override { gds_op.push_back(instr); }

   void visit(WriteTFInstr *instr) override { write_tf.push_back(instr); }

   void visit(LDSReadInstr *instr) override
   {
      std::vector<AluInstr *> buffer;
      m_last_lds_instr = instr->split(buffer, m_last_lds_instr);
      for (auto& i : buffer) {
         i->accept(*this);
      }
   }

   void visit(LDSAtomicInstr *instr) override
   {
      std::vector<AluInstr *> buffer;
      m_last_lds_instr = instr->split(buffer, m_last_lds_instr);
      for (auto& i : buffer) {
         i->accept(*this);
      }
   }

   void visit(RatInstr *instr) override { rat_instr.push_back(instr); }

   std::list<AluInstr *> alu_trans;
   std::list<AluInstr *> alu_vec;
   std::list<TexInstr *> tex;
   std::list<AluGroup *> alu_groups;
   std::list<ExportInstr *> exports;
   std::list<FetchInstr *> fetches;
   std::list<WriteOutInstr *> mem_write_instr;
   std::list<MemRingOutInstr *> mem_ring_writes;
   std::list<GDSInstr *> gds_op;
   std::list<WriteTFInstr *> write_tf;
   std::list<RatInstr *> rat_instr;

   Instr *m_cf_instr{nullptr};
   ValueFactory& m_value_factory;

   AluInstr *m_last_lds_instr{nullptr};
};

struct ArrayChanHash
{
    std::size_t operator()(std::pair<int, int> const& s) const noexcept
    {
       return std::hash<size_t>{}((size_t(s.first) << 3) | s.second);
    }
};

using ArrayCheckSet = std::unordered_set<std::pair<int, int>, ArrayChanHash>;

class BlockScheduler {
public:
   BlockScheduler(r600_chip_class chip_class,
                  radeon_family family);

   void run(Shader *shader);

   void finalize();

private:
   void
   schedule_block(Block& in_block, Shader::ShaderBlocks& out_blocks, ValueFactory& vf);

   bool collect_ready(CollectInstructions& available);

   template <typename T>
   bool collect_ready_type(std::list<T *>& ready, std::list<T *>& orig);

   bool collect_ready_alu_vec(std::list<AluInstr *>& ready,
                              std::list<AluInstr *>& available);

   bool schedule_tex(Shader::ShaderBlocks& out_blocks);
   bool schedule_vtx(Shader::ShaderBlocks& out_blocks);

   template <typename I>
   bool schedule_gds(Shader::ShaderBlocks& out_blocks, std::list<I *>& ready_list);

   template <typename I>
   bool schedule_cf(Shader::ShaderBlocks& out_blocks, std::list<I *>& ready_list);

   bool schedule_alu(Shader::ShaderBlocks& out_blocks);
   void start_new_block(Shader::ShaderBlocks& out_blocks, Block::Type type);

   bool schedule_alu_to_group_vec(AluGroup *group);
   bool schedule_alu_to_group_trans(AluGroup *group, std::list<AluInstr *>& readylist);

   bool schedule_exports(Shader::ShaderBlocks& out_blocks,
                         std::list<ExportInstr *>& ready_list);

   void maybe_split_alu_block(Shader::ShaderBlocks& out_blocks);

   template <typename I> bool schedule(std::list<I *>& ready_list);

   template <typename I> bool schedule_block(std::list<I *>& ready_list);

   void update_array_writes(const AluGroup& group);
   bool check_array_reads(const AluInstr& instr);
   bool check_array_reads(const AluGroup& group);

   std::list<AluInstr *> alu_vec_ready;
   std::list<AluInstr *> alu_trans_ready;
   std::list<AluGroup *> alu_groups_ready;
   std::list<TexInstr *> tex_ready;
   std::list<ExportInstr *> exports_ready;
   std::list<FetchInstr *> fetches_ready;
   std::list<WriteOutInstr *> memops_ready;
   std::list<MemRingOutInstr *> mem_ring_writes_ready;
   std::list<GDSInstr *> gds_ready;
   std::list<WriteTFInstr *> write_tf_ready;
   std::list<RatInstr *> rat_instr_ready;

   enum {
      sched_alu,
      sched_tex,
      sched_fetch,
      sched_free,
      sched_mem_ring,
      sched_gds,
      sched_write_tf,
      sched_rat,
   } current_shed;

   ExportInstr *m_last_pos;
   ExportInstr *m_last_pixel;
   ExportInstr *m_last_param;

   Block *m_current_block;

   int m_lds_addr_count{0};
   int m_alu_groups_scheduled{0};
   r600_chip_class m_chip_class;
   radeon_family m_chip_family;
   bool m_idx0_loading{false};
   bool m_idx1_loading{false};
   bool m_idx0_pending{false};
   bool m_idx1_pending{false};

   bool m_nop_after_rel_dest{false};
   bool m_nop_befor_rel_src{false};
   uint32_t m_next_block_id{1};


   ArrayCheckSet m_last_indirect_array_write;
   ArrayCheckSet m_last_direct_array_write;
};

Shader *
schedule(Shader *original)
{
   Block::set_chipclass(original->chip_class());
   AluGroup::set_chipclass(original->chip_class());

   sfn_log << SfnLog::schedule << "Original shader\n";
   if (sfn_log.has_debug_flag(SfnLog::schedule)) {
      std::stringstream ss;
      original->print(ss);
      sfn_log << ss.str() << "\n\n";
   }

   // TODO later it might be necessary to clone the shader
   // to be able to re-start scheduling

   auto scheduled_shader = original;

   BlockScheduler s(original->chip_class(), original->chip_family());

   s.run(scheduled_shader);
   s.finalize();

   sfn_log << SfnLog::schedule << "Scheduled shader\n";
   if (sfn_log.has_debug_flag(SfnLog::schedule)) {
      std::stringstream ss;
      scheduled_shader->print(ss);
      sfn_log << ss.str() << "\n\n";
   }

   return scheduled_shader;
}

BlockScheduler::BlockScheduler(r600_chip_class chip_class,
                               radeon_family chip_family):
    current_shed(sched_alu),
    m_last_pos(nullptr),
    m_last_pixel(nullptr),
    m_last_param(nullptr),
    m_current_block(nullptr),
    m_chip_class(chip_class),
    m_chip_family(chip_family)
{
   m_nop_after_rel_dest = chip_family == CHIP_RV770;

   m_nop_befor_rel_src = m_chip_class == ISA_CC_R600 &&
                         chip_family != CHIP_RV670 &&
                         chip_family != CHIP_RS780 &&
                         chip_family != CHIP_RS880;
}

void
BlockScheduler::run(Shader *shader)
{
   Shader::ShaderBlocks scheduled_blocks;

   for (auto& block : shader->func()) {
      sfn_log << SfnLog::schedule << "Process block " << block->id() << "\n";
      if (sfn_log.has_debug_flag(SfnLog::schedule)) {
         std::stringstream ss;
         block->print(ss);
         sfn_log << ss.str() << "\n";
      }
      schedule_block(*block, scheduled_blocks, shader->value_factory());
   }

   shader->reset_function(scheduled_blocks);
}

void
BlockScheduler::schedule_block(Block& in_block,
                              Shader::ShaderBlocks& out_blocks,
                              ValueFactory& vf)
{

   assert(in_block.id() >= 0);

   current_shed = sched_fetch;
   auto last_shed = sched_fetch;

   CollectInstructions cir(vf);
   in_block.accept(cir);

   bool have_instr = collect_ready(cir);

   m_current_block = new Block(in_block.nesting_depth(), m_next_block_id++);
   m_current_block->set_instr_flag(Instr::force_cf);
   assert(m_current_block->id() >= 0);

   while (have_instr) {

      sfn_log << SfnLog::schedule << "Have ready instructions\n";

      if (alu_vec_ready.size())
         sfn_log << SfnLog::schedule << "  ALU V:" << alu_vec_ready.size() << "\n";

      if (alu_trans_ready.size())
         sfn_log << SfnLog::schedule << "  ALU T:" << alu_trans_ready.size() << "\n";

      if (alu_groups_ready.size())
         sfn_log << SfnLog::schedule << "  ALU G:" << alu_groups_ready.size() << "\n";

      if (exports_ready.size())
         sfn_log << SfnLog::schedule << "  EXP:" << exports_ready.size() << "\n";
      if (tex_ready.size())
         sfn_log << SfnLog::schedule << "  TEX:" << tex_ready.size() << "\n";
      if (fetches_ready.size())
         sfn_log << SfnLog::schedule << "  FETCH:" << fetches_ready.size() << "\n";
      if (mem_ring_writes_ready.size())
         sfn_log << SfnLog::schedule << "  MEM_RING:" << mem_ring_writes_ready.size()
                 << "\n";
      if (memops_ready.size())
         sfn_log << SfnLog::schedule << "  MEM_OPS:" << mem_ring_writes_ready.size()
                 << "\n";

      if (!m_current_block->lds_group_active() &&
          m_current_block->expected_ar_uses() == 0) {
         if (last_shed != sched_free && memops_ready.size() > 8)
            current_shed = sched_free;
         else if (mem_ring_writes_ready.size() > 15)
            current_shed = sched_mem_ring;
         else if (rat_instr_ready.size() > 3)
            current_shed = sched_rat;
         else if (tex_ready.size() > (m_chip_class >= ISA_CC_EVERGREEN ? 15 : 7))
            current_shed = sched_tex;
      }

      switch (current_shed) {
      case sched_alu:
         if (!schedule_alu(out_blocks)) {
            assert(!m_current_block->lds_group_active());
            current_shed = sched_tex;
            continue;
         }
         last_shed = current_shed;
         break;
      case sched_tex:
         if (tex_ready.empty() || !schedule_tex(out_blocks)) {
            current_shed = sched_fetch;
            continue;
         }
         last_shed = current_shed;
         break;
      case sched_fetch:
         if (!fetches_ready.empty()) {
            schedule_vtx(out_blocks);
            last_shed = current_shed;
         }
         current_shed = sched_gds;
         continue;
      case sched_gds:
         if (!gds_ready.empty()) {
            schedule_gds(out_blocks, gds_ready);
            last_shed = current_shed;
         }
         current_shed = sched_mem_ring;
         continue;
      case sched_mem_ring:
         if (mem_ring_writes_ready.empty() ||
             !schedule_cf(out_blocks, mem_ring_writes_ready)) {
            current_shed = sched_write_tf;
            continue;
         }
         last_shed = current_shed;
         break;
      case sched_write_tf:
         if (write_tf_ready.empty() || !schedule_gds(out_blocks, write_tf_ready)) {
            current_shed = sched_rat;
            continue;
         }
         last_shed = current_shed;
         break;
      case sched_rat:
         if (rat_instr_ready.empty() || !schedule_cf(out_blocks, rat_instr_ready)) {
            current_shed = sched_free;
            continue;
         }
         last_shed = current_shed;
         break;
      case sched_free:
         if (memops_ready.empty() || !schedule_cf(out_blocks, memops_ready)) {
            current_shed = sched_alu;
            break;
         }
         last_shed = current_shed;
      }

      have_instr = collect_ready(cir);
   }

   /* Emit exports always at end of a block */
   while (collect_ready_type(exports_ready, cir.exports))
      schedule_exports(out_blocks, exports_ready);

   ASSERTED bool fail = false;

   if (!cir.alu_groups.empty()) {
      std::cerr << "Unscheduled ALU groups:\n";
      for (auto& a : cir.alu_groups) {
         std::cerr << "   " << *a << "\n";
      }
      fail = true;
   }

   if (!cir.alu_vec.empty()) {
      std::cerr << "Unscheduled ALU vec ops:\n";
      for (auto& a : cir.alu_vec) {
         std::cerr << "   [" << a->block_id() << ":"
                   << a->index() <<"]:" << *a << "\n";
         for (auto& d : a->required_instr())
            std::cerr << "      R["<< d->block_id() << ":" << d->index() <<"]:"
                      << *d << "\n";
      }
      fail = true;
   }

   if (!cir.alu_trans.empty()) {
      std::cerr << "Unscheduled ALU trans ops:\n";
      for (auto& a : cir.alu_trans) {
         std::cerr << "   " << "   [" << a->block_id() << ":"
                   << a->index() <<"]:" << *a << "\n";
         for (auto& d : a->required_instr())
            std::cerr << "      R:" << *d << "\n";
      }
      fail = true;
   }
   if (!cir.mem_write_instr.empty()) {
      std::cerr << "Unscheduled MEM ops:\n";
      for (auto& a : cir.mem_write_instr) {
         std::cerr << "   " << *a << "\n";
      }
      fail = true;
   }

   if (!cir.fetches.empty()) {
      std::cerr << "Unscheduled Fetch ops:\n";
      for (auto& a : cir.fetches) {
         std::cerr << "   " << *a << "\n";
      }
      fail = true;
   }

   if (!cir.tex.empty()) {
      std::cerr << "Unscheduled Tex ops:\n";
      for (auto& a : cir.tex) {
         std::cerr << "   " << *a << "\n";
      }
      fail = true;
   }

   if (fail) {
      std::cerr << "Failing block:\n";
      for (auto& i : in_block)
         std::cerr << "[" << i->block_id() << ":" << i->index() << "] "
                   << (i->is_scheduled() ? "S " : "")
                   << *i << "\n";
      std::cerr << "\nSo far scheduled: ";

      for (auto i : *m_current_block)
         std::cerr << "[" << i->block_id() << ":" << i->index() << "] " << *i << "\n";
      std::cerr << "\n\n: ";
   }

   assert(cir.tex.empty());
   assert(cir.exports.empty());
   assert(cir.fetches.empty());
   assert(cir.alu_vec.empty());
   assert(cir.mem_write_instr.empty());
   assert(cir.mem_ring_writes.empty());

   assert(!fail);

   if (cir.m_cf_instr) {
      // Assert that if condition is ready
      if (m_current_block->type() != Block::alu) {
         start_new_block(out_blocks, Block::alu);
      }
      m_current_block->push_back(cir.m_cf_instr);
      cir.m_cf_instr->set_scheduled();
   }

   if (m_current_block->type() == Block::alu)
      maybe_split_alu_block(out_blocks);
   else
      out_blocks.push_back(m_current_block);
}

void
BlockScheduler::finalize()
{
   if (m_last_pos)
      m_last_pos->set_is_last_export(true);
   if (m_last_pixel)
      m_last_pixel->set_is_last_export(true);
   if (m_last_param)
      m_last_param->set_is_last_export(true);
}

bool
BlockScheduler::schedule_alu(Shader::ShaderBlocks& out_blocks)
{
   bool success = false;
   AluGroup *group = nullptr;

   sfn_log << SfnLog::schedule << "Schedule alu with " <<
              m_current_block->expected_ar_uses()
           << " pending AR loads\n";

   bool has_alu_ready = !alu_vec_ready.empty() || !alu_trans_ready.empty();

   bool has_lds_ready =
      !alu_vec_ready.empty() && (*alu_vec_ready.begin())->has_lds_access();

   bool has_ar_read_ready = !alu_vec_ready.empty() &&
                            std::get<0>((*alu_vec_ready.begin())->indirect_addr());

   /* If we have ready ALU instructions we have to start a new ALU block */
   if (has_alu_ready || !alu_groups_ready.empty()) {
      if (m_current_block->type() != Block::alu) {
         start_new_block(out_blocks, Block::alu);
         m_alu_groups_scheduled = 0;
      }
   }

   /* Schedule groups first. unless we have a pending LDS instruction
    * We don't want the LDS instructions to be too far apart because the
    * fetch + read from queue has to be in the same ALU CF block */
   if (!alu_groups_ready.empty() && !has_lds_ready && !has_ar_read_ready) {
      group = *alu_groups_ready.begin();

      if (!check_array_reads(*group)) {


         sfn_log << SfnLog::schedule << "try schedule " <<
                    *group << "\n";

         /* Only start a new CF if we have no pending AR reads */
         if (m_current_block->try_reserve_kcache(*group)) {
            alu_groups_ready.erase(alu_groups_ready.begin());
            success = true;
         } else {
            if (m_current_block->expected_ar_uses() == 0) {
               start_new_block(out_blocks, Block::alu);

               if (!m_current_block->try_reserve_kcache(*group))
                  unreachable("Scheduling a group in a new block should always succeed");
               alu_groups_ready.erase(alu_groups_ready.begin());
               sfn_log << SfnLog::schedule << "Schedule ALU group\n";
               success = true;
            } else {
               sfn_log << SfnLog::schedule << "Don't add group because of " <<
                          m_current_block->expected_ar_uses()
                       << "pending AR loads\n";
               group = nullptr;
            }
         }
      }
   }

   if (!group && has_alu_ready) {
      group = new AluGroup();
      sfn_log << SfnLog::schedule << "START new ALU group\n";
   } else if (!success) {
      return false;
   }

   assert(group);

   int free_slots = group->free_slots();

   while (free_slots && has_alu_ready) {
      if (!alu_vec_ready.empty())
         success |= schedule_alu_to_group_vec(group);

      /* Apparently one can't schedule a t-slot if there is already
       * and LDS instruction scheduled.
       * TODO: check whether this is only relevant for actual LDS instructions
       * or also for instructions that read from the LDS return value queue */

      if (free_slots & 0x10 && !has_lds_ready) {
         sfn_log << SfnLog::schedule << "Try schedule TRANS channel\n";
         if (!alu_trans_ready.empty())
            success |= schedule_alu_to_group_trans(group, alu_trans_ready);
         if (!alu_vec_ready.empty())
            success |= schedule_alu_to_group_trans(group, alu_vec_ready);
      }

      if (success) {
         ++m_alu_groups_scheduled;
         break;
      } else if (m_current_block->kcache_reservation_failed()) {
         // LDS read groups should not lead to impossible
         // kcache constellations
         assert(!m_current_block->lds_group_active());

         // AR is loaded but not all uses are done, we don't want
         // to start a new CF here
         assert(m_current_block->expected_ar_uses() ==0);

         // kcache reservation failed, so we have to start a new CF
         start_new_block(out_blocks, Block::alu);
      } else {
         // Ready is not empty, but we didn't schedule anything, this
         // means we had a indirect array read or write conflict that we
         // can resolve with an extra group that has a NOP instruction
         if (!alu_trans_ready.empty()  || !alu_vec_ready.empty()) {
            group->add_vec_instructions(new AluInstr(op0_nop, 0));
            break;
         } else {
            return false;
         }
      }
   }



   sfn_log << SfnLog::schedule << "Finalize ALU group\n";
   group->set_scheduled();
   group->fix_last_flag();
   group->set_nesting_depth(m_current_block->nesting_depth());

   auto [addr, is_index] = group->addr();
   if (is_index) {
      if (addr->sel() == AddressRegister::idx0 && m_idx0_pending) {
         assert(!group->has_lds_group_start());
         assert(m_current_block->expected_ar_uses() == 0);
         start_new_block(out_blocks, Block::alu);
         m_current_block->try_reserve_kcache(*group);
      }
      if (addr->sel() == AddressRegister::idx1 && m_idx1_pending) {
         assert(!group->has_lds_group_start());
         assert(m_current_block->expected_ar_uses() == 0);
         start_new_block(out_blocks, Block::alu);
         m_current_block->try_reserve_kcache(*group);
      }
   }

   m_current_block->push_back(group);

   update_array_writes(*group);

   m_idx0_pending |= m_idx0_loading;
   m_idx0_loading = false;

   m_idx1_pending |= m_idx1_loading;
   m_idx1_loading = false;

   if (!m_current_block->lds_group_active() &&
       m_current_block->expected_ar_uses() == 0 &&
       (!addr || is_index)) {
      group->set_instr_flag(Instr::no_lds_or_addr_group);
   }

   if (group->has_lds_group_start())
      m_current_block->lds_group_start(*group->begin());

   if (group->has_lds_group_end())
      m_current_block->lds_group_end();

   if (group->has_kill_op()) {
      assert(!group->has_lds_group_start());
      assert(m_current_block->expected_ar_uses() == 0);
      start_new_block(out_blocks, Block::alu);
   }

   return success;
}

bool
BlockScheduler::schedule_tex(Shader::ShaderBlocks& out_blocks)
{
   if (m_current_block->type() != Block::tex || m_current_block->remaining_slots() == 0) {
      start_new_block(out_blocks, Block::tex);
      m_current_block->set_instr_flag(Instr::force_cf);
   }

   if (!tex_ready.empty() && m_current_block->remaining_slots() > 0) {
      auto ii = tex_ready.begin();
      sfn_log << SfnLog::schedule << "Schedule: " << **ii << "\n";

      if ((unsigned)m_current_block->remaining_slots() < 1 + (*ii)->prepare_instr().size())
         start_new_block(out_blocks, Block::tex);

      for (auto prep : (*ii)->prepare_instr()) {
         prep->set_scheduled();
         m_current_block->push_back(prep);
      }

      (*ii)->set_scheduled();
      m_current_block->push_back(*ii);
      tex_ready.erase(ii);
      return true;
   }
   return false;
}

bool
BlockScheduler::schedule_vtx(Shader::ShaderBlocks& out_blocks)
{
   if (m_current_block->type() != Block::vtx || m_current_block->remaining_slots() == 0) {
      start_new_block(out_blocks, Block::vtx);
      m_current_block->set_instr_flag(Instr::force_cf);
   }
   return schedule_block(fetches_ready);
}

template <typename I>
bool
BlockScheduler::schedule_gds(Shader::ShaderBlocks& out_blocks, std::list<I *>& ready_list)
{
   bool was_full = m_current_block->remaining_slots() == 0;
   if (m_current_block->type() != Block::gds || was_full) {
      start_new_block(out_blocks, Block::gds);
      if (was_full)
         m_current_block->set_instr_flag(Instr::force_cf);
   }
   return schedule_block(ready_list);
}

void
BlockScheduler::start_new_block(Shader::ShaderBlocks& out_blocks, Block::Type type)
{
   if (!m_current_block->empty()) {
      sfn_log << SfnLog::schedule << "Start new block\n";
      assert(!m_current_block->lds_group_active());

      if (m_current_block->type() != Block::alu)
         out_blocks.push_back(m_current_block);
      else
         maybe_split_alu_block(out_blocks);
      m_current_block = new Block(m_current_block->nesting_depth(), m_next_block_id++);
      m_current_block->set_instr_flag(Instr::force_cf);
      m_idx0_pending = m_idx1_pending = false;

   }
   m_current_block->set_type(type, m_chip_class);
}

void BlockScheduler::maybe_split_alu_block(Shader::ShaderBlocks& out_blocks)
{
   // TODO: needs fixing
   if (m_current_block->remaining_slots() > 0) {
      out_blocks.push_back(m_current_block);
      return;
   }

   int used_slots = 0;
   int pending_slots = 0;

   Instr *next_block_start = nullptr;
   for (auto cur_group : *m_current_block) {
      /* This limit is a bit fishy, it should be 128 */
      if (used_slots + pending_slots + cur_group->slots() < 128) {
         if (cur_group->can_start_alu_block()) {
            next_block_start = cur_group;
            used_slots += pending_slots;
            pending_slots = cur_group->slots();
         } else {
            pending_slots += cur_group->slots();
         }
      } else {
         assert(next_block_start);
         next_block_start->set_instr_flag(Instr::force_cf);
         used_slots = pending_slots;
         pending_slots = cur_group->slots();
      }
   }

   Block *sub_block = new Block(m_current_block->nesting_depth(),
                                m_next_block_id++);
   sub_block->set_type(Block::alu, m_chip_class);
   sub_block->set_instr_flag(Instr::force_cf);

   for (auto instr : *m_current_block) {
      auto group = instr->as_alu_group();
      if (!group) {
            sub_block->push_back(instr);
            continue;
      }

      if (group->group_force_alu_cf()) {
         assert(!sub_block->lds_group_active());
         out_blocks.push_back(sub_block);
         sub_block = new Block(m_current_block->nesting_depth(),
                                         m_next_block_id++);
         sub_block->set_type(Block::alu, m_chip_class);
         sub_block->set_instr_flag(Instr::force_cf);
      }
      sub_block->push_back(group);
      if (group->has_lds_group_start())
         sub_block->lds_group_start(*group->begin());

      if (group->has_lds_group_end())
         sub_block->lds_group_end();

   }
   if (!sub_block->empty())
      out_blocks.push_back(sub_block);
}

template <typename I>
bool
BlockScheduler::schedule_cf(Shader::ShaderBlocks& out_blocks, std::list<I *>& ready_list)
{
   if (ready_list.empty())
      return false;
   if (m_current_block->type() != Block::cf)
      start_new_block(out_blocks, Block::cf);
   return schedule(ready_list);
}

bool
BlockScheduler::schedule_alu_to_group_vec(AluGroup *group)
{
   assert(group);
   assert(!alu_vec_ready.empty());

   bool success = false;
   auto i = alu_vec_ready.begin();
   auto e = alu_vec_ready.end();
   while (i != e) {
      sfn_log << SfnLog::schedule << "Try schedule to vec " << **i;

      if (check_array_reads(**i)) {
         ++i;
         continue;
      }

      // precausion: don't kill while we hae LDS queue reads in the pipeline
      if ((*i)->is_kill() && m_current_block->lds_group_active())
         continue;

      if (!m_current_block->try_reserve_kcache(**i)) {
         sfn_log << SfnLog::schedule << " failed (kcache)\n";
         ++i;
         continue;
      }

      if (group->add_vec_instructions(*i)) {
         auto old_i = i;
         ++i;
         if ((*old_i)->has_alu_flag(alu_is_lds)) {
            --m_lds_addr_count;
         }

         if ((*old_i)->num_ar_uses())
            m_current_block->set_expected_ar_uses((*old_i)->num_ar_uses());
         auto addr = std::get<0>((*old_i)->indirect_addr());
         bool has_indirect_reg_load = addr != nullptr && addr->has_flag(Register::addr_or_idx);

         bool is_idx_load_on_eg = false;
         if (!(*old_i)->has_alu_flag(alu_is_lds)) {
            bool load_idx0_eg = (*old_i)->opcode() == op1_set_cf_idx0;
            bool load_idx0_ca = ((*old_i)->opcode() == op1_mova_int &&
                                 (*old_i)->dest()->sel() == AddressRegister::idx0);

            bool load_idx1_eg = (*old_i)->opcode() == op1_set_cf_idx1;
            bool load_idx1_ca = ((*old_i)->opcode() == op1_mova_int &&
                                 (*old_i)->dest()->sel() == AddressRegister::idx1);

            is_idx_load_on_eg = load_idx0_eg || load_idx1_eg;

            bool load_idx0 = load_idx0_eg || load_idx0_ca;
            bool load_idx1 = load_idx1_eg || load_idx1_ca;


            assert(!m_idx0_pending || !load_idx0);
            assert(!m_idx1_pending || !load_idx1);

            m_idx0_loading |= load_idx0;
            m_idx1_loading |= load_idx1;
         }

         if (has_indirect_reg_load || is_idx_load_on_eg)
            m_current_block->dec_expected_ar_uses();

         alu_vec_ready.erase(old_i);
         success = true;
         sfn_log << SfnLog::schedule << " success\n";
      } else {
         ++i;
         sfn_log << SfnLog::schedule << " failed\n";
      }
   }
   return success;
}

bool
BlockScheduler::schedule_alu_to_group_trans(AluGroup *group,
                                           std::list<AluInstr *>& readylist)
{
   assert(group);

   bool success = false;
   auto i = readylist.begin();
   auto e = readylist.end();
   while (i != e) {

      if (check_array_reads(**i)) {
         ++i;
         continue;
      }

      sfn_log << SfnLog::schedule << "Try schedule to trans " << **i;
      if (!m_current_block->try_reserve_kcache(**i)) {
         sfn_log << SfnLog::schedule << " failed (kcache)\n";
         ++i;
         continue;
      }

      if (group->add_trans_instructions(*i)) {
         auto old_i = i;
         ++i;
         auto addr = std::get<0>((*old_i)->indirect_addr());
         if (addr && addr->has_flag(Register::addr_or_idx))
            m_current_block->dec_expected_ar_uses();

         readylist.erase(old_i);
         success = true;
         sfn_log << SfnLog::schedule << " success\n";
         break;
      } else {
         ++i;
         sfn_log << SfnLog::schedule << " failed\n";
      }
   }
   return success;
}

template <typename I>
bool
BlockScheduler::schedule(std::list<I *>& ready_list)
{
   if (!ready_list.empty() && m_current_block->remaining_slots() > 0) {
      auto ii = ready_list.begin();
      sfn_log << SfnLog::schedule << "Schedule: " << **ii << "\n";
      (*ii)->set_scheduled();
      m_current_block->push_back(*ii);
      ready_list.erase(ii);
      return true;
   }
   return false;
}

template <typename I>
bool
BlockScheduler::schedule_block(std::list<I *>& ready_list)
{
   bool success = false;
   while (!ready_list.empty() && m_current_block->remaining_slots() > 0) {
      auto ii = ready_list.begin();
      sfn_log << SfnLog::schedule << "Schedule: " << **ii << " "
              << m_current_block->remaining_slots() << "\n";
      (*ii)->set_scheduled();
      m_current_block->push_back(*ii);
      ready_list.erase(ii);
      success = true;
   }
   return success;
}

bool
BlockScheduler::schedule_exports(Shader::ShaderBlocks& out_blocks,
                                std::list<ExportInstr *>& ready_list)
{
   if (m_current_block->type() != Block::cf)
      start_new_block(out_blocks, Block::cf);

   if (!ready_list.empty()) {
      auto ii = ready_list.begin();
      sfn_log << SfnLog::schedule << "Schedule: " << **ii << "\n";
      (*ii)->set_scheduled();
      m_current_block->push_back(*ii);
      switch ((*ii)->export_type()) {
      case ExportInstr::pos:
         m_last_pos = *ii;
         break;
      case ExportInstr::param:
         m_last_param = *ii;
         break;
      case ExportInstr::pixel:
         m_last_pixel = *ii;
         break;
      }
      (*ii)->set_is_last_export(false);
      ready_list.erase(ii);
      return true;
   }
   return false;
}

bool
BlockScheduler::collect_ready(CollectInstructions& available)
{
   sfn_log << SfnLog::schedule << "Ready instructions\n";
   bool result = false;
   result |= collect_ready_alu_vec(alu_vec_ready, available.alu_vec);
   result |= collect_ready_type(alu_trans_ready, available.alu_trans);
   result |= collect_ready_type(alu_groups_ready, available.alu_groups);
   result |= collect_ready_type(gds_ready, available.gds_op);
   result |= collect_ready_type(tex_ready, available.tex);
   result |= collect_ready_type(fetches_ready, available.fetches);
   result |= collect_ready_type(memops_ready, available.mem_write_instr);
   result |= collect_ready_type(mem_ring_writes_ready, available.mem_ring_writes);
   result |= collect_ready_type(write_tf_ready, available.write_tf);
   result |= collect_ready_type(rat_instr_ready, available.rat_instr);

   sfn_log << SfnLog::schedule << "\n";
   return result;
}

bool
BlockScheduler::collect_ready_alu_vec(std::list<AluInstr *>& ready,
                                     std::list<AluInstr *>& available)
{
   auto i = available.begin();
   auto e = available.end();

   for (auto alu : ready) {
      alu->add_priority(100 * alu->register_priority());
   }

   int max_check = 0;
   while (i != e && max_check++ < 64) {
      if (ready.size() < 64 && (*i)->ready()) {

         int priority = 0;
         /* LDS fetches that use static offsets are usually ready ery fast,
          * so that they would get schedules early, and this leaves the
          * problem that we allocate too many registers with just constant
          * values, and this will make problems with RA. So limit the number of
          * LDS address registers.
          */
         if ((*i)->has_alu_flag(alu_lds_address)) {
            if (m_lds_addr_count > 64) {
               ++i;
               continue;
            } else {
               ++m_lds_addr_count;
            }
         }

         /* LDS instructions are scheduled with high priority.
          * instractions that can go into the t slot and don't have
          * indirect access are put in last, so that they don't block
          * vec-only instructions when scheduling to the vector slots
          * for everything else we look at the register use */

         auto [addr, dummy1, dummy2] = (*i)->indirect_addr();

         if ((*i)->has_lds_access()) {
            priority = 100000;
            if ((*i)->has_alu_flag(alu_is_lds))
               priority += 100000;
         } else if (addr) {
            priority = 10000;
         } else if (AluGroup::has_t()) {
            auto opinfo = alu_ops.find((*i)->opcode());
            assert(opinfo != alu_ops.end());
            if (opinfo->second.can_channel(AluOp::t, m_chip_class))
               priority = -1;
         }

         priority += 100 * (*i)->register_priority();

         (*i)->add_priority(priority);
         ready.push_back(*i);

         auto old_i = i;
         ++i;
         available.erase(old_i);
      } else
         ++i;
   }

   for (auto& i : ready)
      sfn_log << SfnLog::schedule << "V:  " << *i << "\n";

   ready.sort([](const AluInstr *lhs, const AluInstr *rhs) {
      return lhs->priority() > rhs->priority();
   });

   for (auto& i : ready)
      sfn_log << SfnLog::schedule << "V (S):  " << i->priority() << " " << *i << "\n";

   return !ready.empty();
}

template <typename T> struct type_char {
};

template <> struct type_char<AluInstr> {
   static char value() { return 'A';};
};

template <> struct type_char<AluGroup> {
   static char value() { return 'G';};
};

template <> struct type_char<ExportInstr> {
   static char value() { return 'E';};
};

template <> struct type_char<TexInstr> {
   static char value() { return 'T';};
};

template <> struct type_char<FetchInstr> {
   static char value() { return 'F';};
};

template <> struct type_char<WriteOutInstr> {
   static char value() { return 'M';};
};

template <> struct type_char<MemRingOutInstr> {
   static char value() { return 'R';};
};

template <> struct type_char<WriteTFInstr> {
   static char value() { return 'X';};
};

template <> struct type_char<GDSInstr> {
   static char value() { return 'S';};
};

template <> struct type_char<RatInstr> {
   static char value() { return 'I';};
};

template <typename T>
bool
BlockScheduler::collect_ready_type(std::list<T *>& ready, std::list<T *>& available)
{
   auto i = available.begin();
   auto e = available.end();

   int lookahead = 16;
   while (i != e && ready.size() < 16 && lookahead-- > 0) {
      if ((*i)->ready()) {
         ready.push_back(*i);
         auto old_i = i;
         ++i;
         available.erase(old_i);
      } else
         ++i;
   }

   for (auto& i : ready)
      sfn_log << SfnLog::schedule << type_char<T>::value() << ";  " << *i << "\n";

   return !ready.empty();
}

class CheckArrayAccessVisitor : public  ConstRegisterVisitor {
public:
   using ConstRegisterVisitor::visit;
   void visit(const Register& value) override {(void)value;}
   void visit(const LocalArray& value) override {(void)value;}
   void visit(const UniformValue& value) override {(void)value;}
   void visit(const LiteralConstant& value) override {(void)value;}
   void visit(const InlineConstant& value) override {(void)value;}
};

class UpdateArrayWrite : public CheckArrayAccessVisitor {
public:
   UpdateArrayWrite(ArrayCheckSet& indirect_arrays,
                    ArrayCheckSet& direct_arrays,
                    bool tdw):
      last_indirect_array_write(indirect_arrays),
      last_direct_array_write(direct_arrays),
      track_direct_writes(tdw)
   {
   }

   void visit(const LocalArrayValue& value) override {
      int array_base = value.array().base_sel();
      auto entry = std::make_pair(array_base, value.chan());
      if (value.addr())
         last_indirect_array_write.insert(entry);
      else if (track_direct_writes)
         last_direct_array_write.insert(entry);
   }
private:
   ArrayCheckSet& last_indirect_array_write;
   ArrayCheckSet& last_direct_array_write;
   bool track_direct_writes {false};
};


void BlockScheduler::update_array_writes(const AluGroup& group)
{
   if (m_nop_after_rel_dest || m_nop_befor_rel_src) {
      m_last_direct_array_write.clear();
      m_last_indirect_array_write.clear();

      UpdateArrayWrite visitor(m_last_indirect_array_write,
                               m_last_direct_array_write,
                               m_nop_befor_rel_src);

      for (auto alu : group) {
         if (alu && alu->dest())
            alu->dest()->accept(visitor);
      }
   }
}

class CheckArrayRead : public CheckArrayAccessVisitor {
public:
   CheckArrayRead(const ArrayCheckSet& indirect_arrays,
                  const ArrayCheckSet& direct_arrays):
      last_indirect_array_write(indirect_arrays),
      last_direct_array_write(direct_arrays)
   {
   }

   void visit(const LocalArrayValue& value) override {
      int array_base = value.array().base_sel();
      auto entry = std::make_pair(array_base, value.chan());

      if (last_indirect_array_write.find(entry) !=
          last_indirect_array_write.end())
         need_extra_group = true;

      if (value.addr() && last_direct_array_write.find(entry) !=
          last_direct_array_write.end()) {
         need_extra_group = true;
      }
   }

   const ArrayCheckSet& last_indirect_array_write;
   const ArrayCheckSet& last_direct_array_write;
   bool need_extra_group {false};
};


bool BlockScheduler::check_array_reads(const AluInstr& instr)
{
   if (m_nop_after_rel_dest || m_nop_befor_rel_src) {

      CheckArrayRead visitor(m_last_indirect_array_write,
                             m_last_direct_array_write);

      for (auto& s : instr.sources()) {
         s->accept(visitor);
      }
      return visitor.need_extra_group;
   }
   return false;
}

bool BlockScheduler::check_array_reads(const AluGroup& group)
{
   if (m_nop_after_rel_dest || m_nop_befor_rel_src) {

      CheckArrayRead visitor(m_last_indirect_array_write,
                             m_last_direct_array_write);

      for (auto alu : group) {
         if (!alu)
            continue;
         for (auto& s : alu->sources()) {
            s->accept(visitor);
         }
      }
      return visitor.need_extra_group;
   }
   return false;
}


} // namespace r600
