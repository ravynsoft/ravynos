/*
 * Copyright © 2021 Valve Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "aco_ir.h"

#include <vector>

namespace aco {
namespace {

struct idx_ctx {
   std::vector<RegClass> temp_rc = {s1};
   std::vector<uint32_t> renames;
};

inline void
reindex_defs(idx_ctx& ctx, aco_ptr<Instruction>& instr)
{
   for (Definition& def : instr->definitions) {
      if (!def.isTemp())
         continue;
      uint32_t new_id = ctx.temp_rc.size();
      RegClass rc = def.regClass();
      ctx.renames[def.tempId()] = new_id;
      ctx.temp_rc.emplace_back(rc);
      def.setTemp(Temp(new_id, rc));
   }
}

inline void
reindex_ops(idx_ctx& ctx, aco_ptr<Instruction>& instr)
{
   for (Operand& op : instr->operands) {
      if (!op.isTemp())
         continue;
      uint32_t new_id = ctx.renames[op.tempId()];
      assert(op.regClass() == ctx.temp_rc[new_id]);
      op.setTemp(Temp(new_id, op.regClass()));
   }
}

void
reindex_program(idx_ctx& ctx, Program* program)
{
   ctx.renames.resize(program->peekAllocationId());

   for (Block& block : program->blocks) {
      auto it = block.instructions.begin();
      /* for phis, only reindex the definitions */
      while (is_phi(*it)) {
         reindex_defs(ctx, *it++);
      }
      /* reindex all other instructions */
      while (it != block.instructions.end()) {
         reindex_defs(ctx, *it);
         reindex_ops(ctx, *it);
         ++it;
      }
   }
   /* update the phi operands */
   for (Block& block : program->blocks) {
      auto it = block.instructions.begin();
      while (is_phi(*it)) {
         reindex_ops(ctx, *it++);
      }
   }

   /* update program members */
   program->private_segment_buffer = Temp(ctx.renames[program->private_segment_buffer.id()],
                                          program->private_segment_buffer.regClass());
   program->scratch_offset =
      Temp(ctx.renames[program->scratch_offset.id()], program->scratch_offset.regClass());
   program->temp_rc = ctx.temp_rc;
}

void
update_live_out(idx_ctx& ctx, std::vector<IDSet>& live_out)
{
   for (IDSet& set : live_out) {
      IDSet new_set;
      for (uint32_t id : set)
         new_set.insert(ctx.renames[id]);
      set = new_set;
   }
}

} /* end namespace */

void
reindex_ssa(Program* program)
{
   idx_ctx ctx;
   reindex_program(ctx, program);

   program->allocationID = program->temp_rc.size();
}

void
reindex_ssa(Program* program, std::vector<IDSet>& live_out)
{
   idx_ctx ctx;
   reindex_program(ctx, program);
   update_live_out(ctx, live_out);

   program->allocationID = program->temp_rc.size();
}

} // namespace aco
