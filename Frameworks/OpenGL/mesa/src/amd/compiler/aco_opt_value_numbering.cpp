/*
 * Copyright © 2018 Valve Corporation
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
#include "aco_util.h"

#include <unordered_map>
#include <vector>

/*
 * Implements the algorithm for dominator-tree value numbering
 * from "Value Numbering" by Briggs, Cooper, and Simpson.
 */

namespace aco {
namespace {

inline uint32_t
murmur_32_scramble(uint32_t h, uint32_t k)
{
   k *= 0xcc9e2d51;
   k = (k << 15) | (k >> 17);
   h ^= k * 0x1b873593;
   h = (h << 13) | (h >> 19);
   h = h * 5 + 0xe6546b64;
   return h;
}

template <typename T>
uint32_t
hash_murmur_32(Instruction* instr)
{
   uint32_t hash = uint32_t(instr->format) << 16 | uint32_t(instr->opcode);

   for (const Operand& op : instr->operands)
      hash = murmur_32_scramble(hash, op.constantValue());

   /* skip format, opcode and pass_flags */
   for (unsigned i = 2; i < (sizeof(T) >> 2); i++) {
      uint32_t u;
      /* Accesses it though a byte array, so doesn't violate the strict aliasing rule */
      memcpy(&u, reinterpret_cast<uint8_t*>(instr) + i * 4, 4);
      hash = murmur_32_scramble(hash, u);
   }

   /* Finalize. */
   uint32_t len = instr->operands.size() + instr->definitions.size() + sizeof(T);
   hash ^= len;
   hash ^= hash >> 16;
   hash *= 0x85ebca6b;
   hash ^= hash >> 13;
   hash *= 0xc2b2ae35;
   hash ^= hash >> 16;
   return hash;
}

struct InstrHash {
   /* This hash function uses the Murmur3 algorithm written by Austin Appleby
    * https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
    *
    * In order to calculate the expression set, only the right-hand-side of an
    * instruction is used for the hash, i.e. everything except the definitions.
    */
   std::size_t operator()(Instruction* instr) const
   {
      if (instr->isDPP16())
         return hash_murmur_32<DPP16_instruction>(instr);

      if (instr->isDPP8())
         return hash_murmur_32<DPP8_instruction>(instr);

      if (instr->isSDWA())
         return hash_murmur_32<SDWA_instruction>(instr);

      if (instr->isVINTERP_INREG())
         return hash_murmur_32<VINTERP_inreg_instruction>(instr);

      if (instr->isVALU())
         return hash_murmur_32<VALU_instruction>(instr);

      switch (instr->format) {
      case Format::SMEM: return hash_murmur_32<SMEM_instruction>(instr);
      case Format::VINTRP: return hash_murmur_32<VINTRP_instruction>(instr);
      case Format::DS: return hash_murmur_32<DS_instruction>(instr);
      case Format::SOPP: return hash_murmur_32<SOPP_instruction>(instr);
      case Format::SOPK: return hash_murmur_32<SOPK_instruction>(instr);
      case Format::EXP: return hash_murmur_32<Export_instruction>(instr);
      case Format::MUBUF: return hash_murmur_32<MUBUF_instruction>(instr);
      case Format::MIMG: return hash_murmur_32<MIMG_instruction>(instr);
      case Format::MTBUF: return hash_murmur_32<MTBUF_instruction>(instr);
      case Format::FLAT: return hash_murmur_32<FLAT_instruction>(instr);
      case Format::PSEUDO_BRANCH: return hash_murmur_32<Pseudo_branch_instruction>(instr);
      case Format::PSEUDO_REDUCTION: return hash_murmur_32<Pseudo_reduction_instruction>(instr);
      default: return hash_murmur_32<Instruction>(instr);
      }
   }
};

struct InstrPred {
   bool operator()(Instruction* a, Instruction* b) const
   {
      if (a->format != b->format)
         return false;
      if (a->opcode != b->opcode)
         return false;
      if (a->operands.size() != b->operands.size() ||
          a->definitions.size() != b->definitions.size())
         return false; /* possible with pseudo-instructions */
      for (unsigned i = 0; i < a->operands.size(); i++) {
         if (a->operands[i].isConstant()) {
            if (!b->operands[i].isConstant())
               return false;
            if (a->operands[i].constantValue() != b->operands[i].constantValue())
               return false;
         } else if (a->operands[i].isTemp()) {
            if (!b->operands[i].isTemp())
               return false;
            if (a->operands[i].tempId() != b->operands[i].tempId())
               return false;
         } else if (a->operands[i].isUndefined() ^ b->operands[i].isUndefined())
            return false;
         if (a->operands[i].isFixed()) {
            if (!b->operands[i].isFixed())
               return false;
            if (a->operands[i].physReg() != b->operands[i].physReg())
               return false;
            if (a->operands[i].physReg() == exec && a->pass_flags != b->pass_flags)
               return false;
         }
      }
      for (unsigned i = 0; i < a->definitions.size(); i++) {
         if (a->definitions[i].isTemp()) {
            if (!b->definitions[i].isTemp())
               return false;
            if (a->definitions[i].regClass() != b->definitions[i].regClass())
               return false;
         }
         if (a->definitions[i].isFixed()) {
            if (!b->definitions[i].isFixed())
               return false;
            if (a->definitions[i].physReg() != b->definitions[i].physReg())
               return false;
            if (a->definitions[i].physReg() == exec)
               return false;
         }
      }

      if (a->isVALU()) {
         VALU_instruction& aV = a->valu();
         VALU_instruction& bV = b->valu();
         if (aV.abs != bV.abs || aV.neg != bV.neg || aV.clamp != bV.clamp || aV.omod != bV.omod ||
             aV.opsel != bV.opsel || aV.opsel_lo != bV.opsel_lo || aV.opsel_hi != bV.opsel_hi)
            return false;

         if (a->opcode == aco_opcode::v_permlane16_b32 ||
             a->opcode == aco_opcode::v_permlanex16_b32 ||
             a->opcode == aco_opcode::v_readfirstlane_b32)
            return aV.pass_flags == bV.pass_flags;
      }
      if (a->isDPP16()) {
         DPP16_instruction& aDPP = a->dpp16();
         DPP16_instruction& bDPP = b->dpp16();
         return aDPP.pass_flags == bDPP.pass_flags && aDPP.dpp_ctrl == bDPP.dpp_ctrl &&
                aDPP.bank_mask == bDPP.bank_mask && aDPP.row_mask == bDPP.row_mask &&
                aDPP.bound_ctrl == bDPP.bound_ctrl && aDPP.fetch_inactive == bDPP.fetch_inactive;
      }
      if (a->isDPP8()) {
         DPP8_instruction& aDPP = a->dpp8();
         DPP8_instruction& bDPP = b->dpp8();
         return aDPP.pass_flags == bDPP.pass_flags && aDPP.lane_sel == bDPP.lane_sel &&
                aDPP.fetch_inactive == bDPP.fetch_inactive;
      }
      if (a->isSDWA()) {
         SDWA_instruction& aSDWA = a->sdwa();
         SDWA_instruction& bSDWA = b->sdwa();
         return aSDWA.sel[0] == bSDWA.sel[0] && aSDWA.sel[1] == bSDWA.sel[1] &&
                aSDWA.dst_sel == bSDWA.dst_sel;
      }

      switch (a->format) {
      case Format::SOP1: {
         if (a->opcode == aco_opcode::s_sendmsg_rtn_b32 ||
             a->opcode == aco_opcode::s_sendmsg_rtn_b64)
            return false;
         return true;
      }
      case Format::SOPK: {
         if (a->opcode == aco_opcode::s_getreg_b32)
            return false;
         SOPK_instruction& aK = a->sopk();
         SOPK_instruction& bK = b->sopk();
         return aK.imm == bK.imm;
      }
      case Format::SMEM: {
         SMEM_instruction& aS = a->smem();
         SMEM_instruction& bS = b->smem();
         return aS.sync == bS.sync && aS.glc == bS.glc && aS.dlc == bS.dlc && aS.nv == bS.nv &&
                aS.disable_wqm == bS.disable_wqm;
      }
      case Format::VINTRP: {
         VINTRP_instruction& aI = a->vintrp();
         VINTRP_instruction& bI = b->vintrp();
         if (aI.attribute != bI.attribute)
            return false;
         if (aI.component != bI.component)
            return false;
         return true;
      }
      case Format::VINTERP_INREG: {
         VINTERP_inreg_instruction& aI = a->vinterp_inreg();
         VINTERP_inreg_instruction& bI = b->vinterp_inreg();
         return aI.wait_exp == bI.wait_exp;
      }
      case Format::PSEUDO_REDUCTION: {
         Pseudo_reduction_instruction& aR = a->reduction();
         Pseudo_reduction_instruction& bR = b->reduction();
         return aR.pass_flags == bR.pass_flags && aR.reduce_op == bR.reduce_op &&
                aR.cluster_size == bR.cluster_size;
      }
      case Format::DS: {
         assert(a->opcode == aco_opcode::ds_bpermute_b32 ||
                a->opcode == aco_opcode::ds_permute_b32 || a->opcode == aco_opcode::ds_swizzle_b32);
         DS_instruction& aD = a->ds();
         DS_instruction& bD = b->ds();
         return aD.sync == bD.sync && aD.pass_flags == bD.pass_flags && aD.gds == bD.gds &&
                aD.offset0 == bD.offset0 && aD.offset1 == bD.offset1;
      }
      case Format::LDSDIR: {
         LDSDIR_instruction& aD = a->ldsdir();
         LDSDIR_instruction& bD = b->ldsdir();
         return aD.sync == bD.sync && aD.attr == bD.attr && aD.attr_chan == bD.attr_chan &&
                aD.wait_vdst == bD.wait_vdst;
      }
      case Format::MTBUF: {
         MTBUF_instruction& aM = a->mtbuf();
         MTBUF_instruction& bM = b->mtbuf();
         return aM.sync == bM.sync && aM.dfmt == bM.dfmt && aM.nfmt == bM.nfmt &&
                aM.offset == bM.offset && aM.offen == bM.offen && aM.idxen == bM.idxen &&
                aM.glc == bM.glc && aM.dlc == bM.dlc && aM.slc == bM.slc && aM.tfe == bM.tfe &&
                aM.disable_wqm == bM.disable_wqm;
      }
      case Format::MUBUF: {
         MUBUF_instruction& aM = a->mubuf();
         MUBUF_instruction& bM = b->mubuf();
         return aM.sync == bM.sync && aM.offset == bM.offset && aM.offen == bM.offen &&
                aM.idxen == bM.idxen && aM.glc == bM.glc && aM.dlc == bM.dlc && aM.slc == bM.slc &&
                aM.tfe == bM.tfe && aM.lds == bM.lds && aM.disable_wqm == bM.disable_wqm;
      }
      case Format::MIMG: {
         MIMG_instruction& aM = a->mimg();
         MIMG_instruction& bM = b->mimg();
         return aM.sync == bM.sync && aM.dmask == bM.dmask && aM.unrm == bM.unrm &&
                aM.glc == bM.glc && aM.slc == bM.slc && aM.tfe == bM.tfe && aM.da == bM.da &&
                aM.lwe == bM.lwe && aM.r128 == bM.r128 && aM.a16 == bM.a16 && aM.d16 == bM.d16 &&
                aM.disable_wqm == bM.disable_wqm;
      }
      case Format::FLAT:
      case Format::GLOBAL:
      case Format::SCRATCH:
      case Format::EXP:
      case Format::SOPP:
      case Format::PSEUDO_BRANCH:
      case Format::PSEUDO_BARRIER: unreachable("unsupported instruction format");
      default: return true;
      }
   }
};

using expr_set = aco::unordered_map<Instruction*, uint32_t, InstrHash, InstrPred>;

struct vn_ctx {
   Program* program;
   monotonic_buffer_resource m;
   expr_set expr_values;
   aco::unordered_map<uint32_t, Temp> renames;

   /* The exec id should be the same on the same level of control flow depth.
    * Together with the check for dominator relations, it is safe to assume
    * that the same exec_id also means the same execution mask.
    * Discards increment the exec_id, so that it won't return to the previous value.
    */
   uint32_t exec_id = 1;

   vn_ctx(Program* program_) : program(program_), m(), expr_values(m), renames(m)
   {
      static_assert(sizeof(Temp) == 4, "Temp must fit in 32bits");
      unsigned size = 0;
      for (Block& block : program->blocks)
         size += block.instructions.size();
      expr_values.reserve(size);
   }
};

/* dominates() returns true if the parent block dominates the child block and
 * if the parent block is part of the same loop or has a smaller loop nest depth.
 */
bool
dominates(vn_ctx& ctx, uint32_t parent, uint32_t child)
{
   unsigned parent_loop_nest_depth = ctx.program->blocks[parent].loop_nest_depth;
   while (parent < child && parent_loop_nest_depth <= ctx.program->blocks[child].loop_nest_depth)
      child = ctx.program->blocks[child].logical_idom;

   return parent == child;
}

/** Returns whether this instruction can safely be removed
 *  and replaced by an equal expression.
 *  This is in particular true for ALU instructions and
 *  read-only memory instructions.
 *
 *  Note that expr_set must not be used with instructions
 *  which cannot be eliminated.
 */
bool
can_eliminate(aco_ptr<Instruction>& instr)
{
   switch (instr->format) {
   case Format::FLAT:
   case Format::GLOBAL:
   case Format::SCRATCH:
   case Format::EXP:
   case Format::SOPP:
   case Format::PSEUDO_BRANCH:
   case Format::PSEUDO_BARRIER: return false;
   case Format::DS:
      return instr->opcode == aco_opcode::ds_bpermute_b32 ||
             instr->opcode == aco_opcode::ds_permute_b32 ||
             instr->opcode == aco_opcode::ds_swizzle_b32;
   case Format::SMEM:
   case Format::MUBUF:
   case Format::MIMG:
   case Format::MTBUF:
      if (!get_sync_info(instr.get()).can_reorder())
         return false;
      break;
   default: break;
   }

   if (instr->definitions.empty() || instr->opcode == aco_opcode::p_phi ||
       instr->opcode == aco_opcode::p_linear_phi ||
       instr->opcode == aco_opcode::p_pops_gfx9_add_exiting_wave_id ||
       instr->definitions[0].isNoCSE())
      return false;

   return true;
}

void
process_block(vn_ctx& ctx, Block& block)
{
   std::vector<aco_ptr<Instruction>> new_instructions;
   new_instructions.reserve(block.instructions.size());

   for (aco_ptr<Instruction>& instr : block.instructions) {
      /* first, rename operands */
      for (Operand& op : instr->operands) {
         if (!op.isTemp())
            continue;
         auto it = ctx.renames.find(op.tempId());
         if (it != ctx.renames.end())
            op.setTemp(it->second);
      }

      if (instr->opcode == aco_opcode::p_discard_if ||
          instr->opcode == aco_opcode::p_demote_to_helper || instr->opcode == aco_opcode::p_end_wqm)
         ctx.exec_id++;

      if (!can_eliminate(instr)) {
         new_instructions.emplace_back(std::move(instr));
         continue;
      }

      /* simple copy-propagation through renaming */
      bool copy_instr =
         instr->opcode == aco_opcode::p_parallelcopy ||
         (instr->opcode == aco_opcode::p_create_vector && instr->operands.size() == 1);
      if (copy_instr && !instr->definitions[0].isFixed() && instr->operands[0].isTemp() &&
          instr->operands[0].regClass() == instr->definitions[0].regClass()) {
         ctx.renames[instr->definitions[0].tempId()] = instr->operands[0].getTemp();
         continue;
      }

      instr->pass_flags = ctx.exec_id;
      std::pair<expr_set::iterator, bool> res = ctx.expr_values.emplace(instr.get(), block.index);

      /* if there was already an expression with the same value number */
      if (!res.second) {
         Instruction* orig_instr = res.first->first;
         assert(instr->definitions.size() == orig_instr->definitions.size());
         /* check if the original instruction dominates the current one */
         if (dominates(ctx, res.first->second, block.index) &&
             ctx.program->blocks[res.first->second].fp_mode.canReplace(block.fp_mode)) {
            for (unsigned i = 0; i < instr->definitions.size(); i++) {
               assert(instr->definitions[i].regClass() == orig_instr->definitions[i].regClass());
               assert(instr->definitions[i].isTemp());
               ctx.renames[instr->definitions[i].tempId()] = orig_instr->definitions[i].getTemp();
               if (instr->definitions[i].isPrecise())
                  orig_instr->definitions[i].setPrecise(true);
               /* SPIR_V spec says that an instruction marked with NUW wrapping
                * around is undefined behaviour, so we can break additions in
                * other contexts.
                */
               if (instr->definitions[i].isNUW())
                  orig_instr->definitions[i].setNUW(true);
            }
         } else {
            ctx.expr_values.erase(res.first);
            ctx.expr_values.emplace(instr.get(), block.index);
            new_instructions.emplace_back(std::move(instr));
         }
      } else {
         new_instructions.emplace_back(std::move(instr));
      }
   }

   block.instructions = std::move(new_instructions);
}

void
rename_phi_operands(Block& block, aco::unordered_map<uint32_t, Temp>& renames)
{
   for (aco_ptr<Instruction>& phi : block.instructions) {
      if (phi->opcode != aco_opcode::p_phi && phi->opcode != aco_opcode::p_linear_phi)
         break;

      for (Operand& op : phi->operands) {
         if (!op.isTemp())
            continue;
         auto it = renames.find(op.tempId());
         if (it != renames.end())
            op.setTemp(it->second);
      }
   }
}
} /* end namespace */

void
value_numbering(Program* program)
{
   vn_ctx ctx(program);
   std::vector<unsigned> loop_headers;

   for (Block& block : program->blocks) {
      assert(ctx.exec_id > 0);
      /* decrement exec_id when leaving nested control flow */
      if (block.kind & block_kind_loop_header)
         loop_headers.push_back(block.index);
      if (block.kind & block_kind_merge) {
         ctx.exec_id--;
      } else if (block.kind & block_kind_loop_exit) {
         ctx.exec_id -= program->blocks[loop_headers.back()].linear_preds.size();
         ctx.exec_id -= block.linear_preds.size();
         loop_headers.pop_back();
      }

      if (block.logical_idom == (int)block.index)
         ctx.expr_values.clear();

      if (block.logical_idom != -1)
         process_block(ctx, block);
      else
         rename_phi_operands(block, ctx.renames);

      /* increment exec_id when entering nested control flow */
      if (block.kind & block_kind_branch || block.kind & block_kind_loop_preheader ||
          block.kind & block_kind_break || block.kind & block_kind_continue)
         ctx.exec_id++;
      else if (block.kind & block_kind_continue_or_break)
         ctx.exec_id += 2;
   }

   /* rename loop header phi operands */
   for (Block& block : program->blocks) {
      if (block.kind & block_kind_loop_header)
         rename_phi_operands(block, ctx.renames);
   }
}

} // namespace aco
