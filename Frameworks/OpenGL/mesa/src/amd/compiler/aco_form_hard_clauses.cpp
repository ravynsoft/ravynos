/*
 * Copyright © 2020 Valve Corporation
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

#include "aco_builder.h"
#include "aco_ir.h"

#include <vector>

namespace aco {
namespace {

/* there can also be LDS and VALU clauses, but I don't see how those are interesting */
enum clause_type {
   clause_smem,
   clause_other,
   /* GFX10: */
   clause_vmem,
   clause_flat,
   /* GFX11: */
   clause_mimg_load,
   clause_mimg_store,
   clause_mimg_atomic,
   clause_mimg_sample,
   clause_vmem_load,
   clause_vmem_store,
   clause_vmem_atomic,
   clause_flat_load,
   clause_flat_store,
   clause_flat_atomic,
   clause_bvh,
};

void
emit_clause(Builder& bld, unsigned num_instrs, aco_ptr<Instruction>* instrs)
{
   unsigned start = 0;
   unsigned end = num_instrs;

   if (bld.program->gfx_level < GFX11) {
      /* skip any stores at the start */
      for (; (start < num_instrs) && instrs[start]->definitions.empty(); start++)
         bld.insert(std::move(instrs[start]));

      for (end = start; (end < num_instrs) && !instrs[end]->definitions.empty(); end++)
         ;
   }

   unsigned clause_size = end - start;
   if (clause_size > 1)
      bld.sopp(aco_opcode::s_clause, -1, clause_size - 1);

   for (unsigned i = start; i < num_instrs; i++)
      bld.insert(std::move(instrs[i]));
}

clause_type
get_type(Program* program, aco_ptr<Instruction>& instr)
{
   if (instr->isSMEM() && !instr->operands.empty())
      return clause_smem;

   if (program->gfx_level >= GFX11) {
      if (instr->isMIMG()) {
         switch (instr->opcode) {
         case aco_opcode::image_bvh_intersect_ray:
         case aco_opcode::image_bvh64_intersect_ray: return clause_bvh;
         case aco_opcode::image_atomic_swap:
         case aco_opcode::image_atomic_cmpswap:
         case aco_opcode::image_atomic_add:
         case aco_opcode::image_atomic_sub:
         case aco_opcode::image_atomic_rsub:
         case aco_opcode::image_atomic_smin:
         case aco_opcode::image_atomic_umin:
         case aco_opcode::image_atomic_smax:
         case aco_opcode::image_atomic_umax:
         case aco_opcode::image_atomic_and:
         case aco_opcode::image_atomic_or:
         case aco_opcode::image_atomic_xor:
         case aco_opcode::image_atomic_inc:
         case aco_opcode::image_atomic_dec:
         case aco_opcode::image_atomic_fcmpswap:
         case aco_opcode::image_atomic_fmin:
         case aco_opcode::image_atomic_fmax: return clause_mimg_atomic;
         default:
            if (instr->definitions.empty())
               return clause_mimg_store;
            else
               return !instr->operands[1].isUndefined() && instr->operands[1].regClass() == s4
                         ? clause_mimg_sample
                         : clause_mimg_load;
         }
      } else if (instr->isMTBUF() || instr->isScratch()) {
         return instr->definitions.empty() ? clause_vmem_store : clause_vmem_load;
      } else if (instr->isMUBUF()) {
         switch (instr->opcode) {
         case aco_opcode::buffer_atomic_add:
         case aco_opcode::buffer_atomic_and_x2:
         case aco_opcode::buffer_atomic_rsub:
         case aco_opcode::buffer_atomic_umax:
         case aco_opcode::buffer_atomic_dec:
         case aco_opcode::buffer_atomic_smax:
         case aco_opcode::buffer_atomic_fmax:
         case aco_opcode::buffer_atomic_rsub_x2:
         case aco_opcode::buffer_atomic_smin:
         case aco_opcode::buffer_atomic_sub:
         case aco_opcode::buffer_atomic_sub_x2:
         case aco_opcode::buffer_atomic_xor_x2:
         case aco_opcode::buffer_atomic_add_f32:
         case aco_opcode::buffer_atomic_inc:
         case aco_opcode::buffer_atomic_swap_x2:
         case aco_opcode::buffer_atomic_cmpswap:
         case aco_opcode::buffer_atomic_fmin_x2:
         case aco_opcode::buffer_atomic_umin:
         case aco_opcode::buffer_atomic_or:
         case aco_opcode::buffer_atomic_umax_x2:
         case aco_opcode::buffer_atomic_smin_x2:
         case aco_opcode::buffer_atomic_umin_x2:
         case aco_opcode::buffer_atomic_cmpswap_x2:
         case aco_opcode::buffer_atomic_add_x2:
         case aco_opcode::buffer_atomic_swap:
         case aco_opcode::buffer_atomic_and:
         case aco_opcode::buffer_atomic_fmin:
         case aco_opcode::buffer_atomic_fcmpswap_x2:
         case aco_opcode::buffer_atomic_or_x2:
         case aco_opcode::buffer_atomic_fcmpswap:
         case aco_opcode::buffer_atomic_xor:
         case aco_opcode::buffer_atomic_dec_x2:
         case aco_opcode::buffer_atomic_fmax_x2:
         case aco_opcode::buffer_atomic_csub:
         case aco_opcode::buffer_atomic_inc_x2:
         case aco_opcode::buffer_atomic_smax_x2: return clause_vmem_atomic;
         default: return instr->definitions.empty() ? clause_vmem_store : clause_vmem_load;
         }
      } else if (instr->isGlobal()) {
         switch (instr->opcode) {
         case aco_opcode::global_atomic_swap:
         case aco_opcode::global_atomic_umax:
         case aco_opcode::global_atomic_cmpswap:
         case aco_opcode::global_atomic_and_x2:
         case aco_opcode::global_atomic_fmax:
         case aco_opcode::global_atomic_smax_x2:
         case aco_opcode::global_atomic_fmax_x2:
         case aco_opcode::global_atomic_dec:
         case aco_opcode::global_atomic_dec_x2:
         case aco_opcode::global_atomic_umin:
         case aco_opcode::global_atomic_fcmpswap_x2:
         case aco_opcode::global_atomic_inc:
         case aco_opcode::global_atomic_and:
         case aco_opcode::global_atomic_fmin:
         case aco_opcode::global_atomic_fcmpswap:
         case aco_opcode::global_atomic_or_x2:
         case aco_opcode::global_atomic_smax:
         case aco_opcode::global_atomic_sub:
         case aco_opcode::global_atomic_xor:
         case aco_opcode::global_atomic_swap_x2:
         case aco_opcode::global_atomic_umax_x2:
         case aco_opcode::global_atomic_umin_x2:
         case aco_opcode::global_atomic_xor_x2:
         case aco_opcode::global_atomic_inc_x2:
         case aco_opcode::global_atomic_fmin_x2:
         case aco_opcode::global_atomic_add_f32:
         case aco_opcode::global_atomic_add:
         case aco_opcode::global_atomic_or:
         case aco_opcode::global_atomic_add_x2:
         case aco_opcode::global_atomic_smin_x2:
         case aco_opcode::global_atomic_smin:
         case aco_opcode::global_atomic_csub:
         case aco_opcode::global_atomic_sub_x2:
         case aco_opcode::global_atomic_cmpswap_x2: return clause_vmem_atomic;
         default: return instr->definitions.empty() ? clause_vmem_store : clause_vmem_load;
         }
      } else if (instr->isFlat()) {
         switch (instr->opcode) {
         case aco_opcode::flat_atomic_smax:
         case aco_opcode::flat_atomic_fcmpswap_x2:
         case aco_opcode::flat_atomic_inc_x2:
         case aco_opcode::flat_atomic_dec:
         case aco_opcode::flat_atomic_fmin:
         case aco_opcode::flat_atomic_umax_x2:
         case aco_opcode::flat_atomic_add_f32:
         case aco_opcode::flat_atomic_or:
         case aco_opcode::flat_atomic_smax_x2:
         case aco_opcode::flat_atomic_umin:
         case aco_opcode::flat_atomic_sub:
         case aco_opcode::flat_atomic_swap:
         case aco_opcode::flat_atomic_swap_x2:
         case aco_opcode::flat_atomic_cmpswap_x2:
         case aco_opcode::flat_atomic_fcmpswap:
         case aco_opcode::flat_atomic_add:
         case aco_opcode::flat_atomic_umin_x2:
         case aco_opcode::flat_atomic_xor_x2:
         case aco_opcode::flat_atomic_smin:
         case aco_opcode::flat_atomic_fmax_x2:
         case aco_opcode::flat_atomic_cmpswap:
         case aco_opcode::flat_atomic_dec_x2:
         case aco_opcode::flat_atomic_sub_x2:
         case aco_opcode::flat_atomic_add_x2:
         case aco_opcode::flat_atomic_umax:
         case aco_opcode::flat_atomic_xor:
         case aco_opcode::flat_atomic_and_x2:
         case aco_opcode::flat_atomic_inc:
         case aco_opcode::flat_atomic_and:
         case aco_opcode::flat_atomic_fmin_x2:
         case aco_opcode::flat_atomic_smin_x2:
         case aco_opcode::flat_atomic_or_x2:
         case aco_opcode::flat_atomic_fmax: return clause_flat_atomic;
         default: return instr->definitions.empty() ? clause_flat_store : clause_flat_load;
         }
      }
   } else {
      if (instr->isVMEM() && !instr->operands.empty()) {
         if (program->gfx_level == GFX10 && instr->isMIMG() && get_mimg_nsa_dwords(instr.get()) > 0)
            return clause_other;
         else
            return clause_vmem;
      } else if (instr->isScratch() || instr->isGlobal()) {
         return clause_vmem;
      } else if (instr->isFlat()) {
         return clause_flat;
      }
   }
   return clause_other;
}

} /* end namespace */

void
form_hard_clauses(Program* program)
{
   /* The ISA documentation says 63 is the maximum for GFX11/12, but according to
    * LLVM there are HW bugs with more than 32 instructions.
    */
   const unsigned max_clause_length = program->gfx_level >= GFX11 ? 32 : 63;
   for (Block& block : program->blocks) {
      unsigned num_instrs = 0;
      aco_ptr<Instruction> current_instrs[63];
      clause_type current_type = clause_other;

      std::vector<aco_ptr<Instruction>> new_instructions;
      new_instructions.reserve(block.instructions.size());
      Builder bld(program, &new_instructions);

      for (unsigned i = 0; i < block.instructions.size(); i++) {
         aco_ptr<Instruction>& instr = block.instructions[i];

         clause_type type = get_type(program, instr);
         if (type != current_type || num_instrs == max_clause_length ||
             (num_instrs && !should_form_clause(current_instrs[0].get(), instr.get()))) {
            emit_clause(bld, num_instrs, current_instrs);
            num_instrs = 0;
            current_type = type;
         }

         if (type == clause_other) {
            bld.insert(std::move(instr));
            continue;
         }

         current_instrs[num_instrs++] = std::move(instr);
      }

      emit_clause(bld, num_instrs, current_instrs);

      block.instructions = std::move(new_instructions);
   }
}
} // namespace aco
