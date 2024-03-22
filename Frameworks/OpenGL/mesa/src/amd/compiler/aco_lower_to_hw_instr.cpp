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

#include "aco_builder.h"
#include "aco_ir.h"

#include "common/sid.h"

#include <map>
#include <vector>

namespace aco {

struct lower_context {
   Program* program;
   Block* block;
   std::vector<aco_ptr<Instruction>> instructions;
};

/* Class for obtaining where s_sendmsg(MSG_ORDERED_PS_DONE) must be done in a Primitive Ordered
 * Pixel Shader on GFX9-10.3.
 *
 * MSG_ORDERED_PS_DONE must be sent once after the ordered section is done along all execution paths
 * from the POPS packer ID hardware register setting to s_endpgm. It is, however, also okay to send
 * it if the packer ID is not going to be set at all by the wave, so some conservativeness is fine.
 *
 * For simplicity, sending the message from top-level blocks as dominance and post-dominance
 * checking for any location in the shader is trivial in them. Also, for simplicity, sending it
 * regardless of whether the POPS packer ID hardware register has already potentially been set up.
 *
 * Note that there can be multiple interlock end instructions in the shader.
 * SPV_EXT_fragment_shader_interlock requires OpEndInvocationInterlockEXT to be executed exactly
 * once by the invocation. However, there may be, for instance, multiple ordered sections, and which
 * one will be executed may depend on divergent control flow (some lanes may execute one ordered
 * section, other lanes may execute another). MSG_ORDERED_PS_DONE, however, is sent via a scalar
 * instruction, so it must be ensured that the message is sent after the last ordered section in the
 * entire wave.
 */
class gfx9_pops_done_msg_bounds {
public:
   explicit gfx9_pops_done_msg_bounds() = default;

   explicit gfx9_pops_done_msg_bounds(const Program* const program)
   {
      /* Find the top-level location after the last ordered section end pseudo-instruction in the
       * program.
       * Consider `p_pops_gfx9_overlapped_wave_wait_done` a boundary too - make sure the message
       * isn't sent if any wait hasn't been fully completed yet (if a begin-end-begin situation
       * occurs somehow, as the location of `p_pops_gfx9_ordered_section_done` is controlled by the
       * application) for safety, assuming that waits are the only thing that need the packer
       * hardware register to be set at some point during or before them, and it won't be set
       * anymore after the last wait.
       */
      int last_top_level_block_idx = -1;
      for (int block_idx = (int)program->blocks.size() - 1; block_idx >= 0; block_idx--) {
         const Block& block = program->blocks[block_idx];
         if (block.kind & block_kind_top_level) {
            last_top_level_block_idx = block_idx;
         }
         for (size_t instr_idx = block.instructions.size() - 1; instr_idx + size_t(1) > 0;
              instr_idx--) {
            const aco_opcode opcode = block.instructions[instr_idx]->opcode;
            if (opcode == aco_opcode::p_pops_gfx9_ordered_section_done ||
                opcode == aco_opcode::p_pops_gfx9_overlapped_wave_wait_done) {
               end_block_idx_ = last_top_level_block_idx;
               /* The same block if it's already a top-level block, or the beginning of the next
                * top-level block.
                */
               instr_after_end_idx_ = block_idx == end_block_idx_ ? instr_idx + 1 : 0;
               break;
            }
         }
         if (end_block_idx_ != -1) {
            break;
         }
      }
   }

   /* If this is not -1, during the normal execution flow (not early exiting), MSG_ORDERED_PS_DONE
    * must be sent in this block.
    */
   int end_block_idx() const { return end_block_idx_; }

   /* If end_block_idx() is an existing block, during the normal execution flow (not early exiting),
    * MSG_ORDERED_PS_DONE must be sent before this instruction in the block end_block_idx().
    * If this is out of the bounds of the instructions in the end block, it must be sent in the end
    * of that block.
    */
   size_t instr_after_end_idx() const { return instr_after_end_idx_; }

   /* Whether an instruction doing early exit (such as discard) needs to send MSG_ORDERED_PS_DONE
    * before actually ending the program.
    */
   bool early_exit_needs_done_msg(const int block_idx, const size_t instr_idx) const
   {
      return block_idx <= end_block_idx_ &&
             (block_idx != end_block_idx_ || instr_idx < instr_after_end_idx_);
   }

private:
   /* Initialize to an empty range for which "is inside" comparisons will be failing for any
    * block.
    */
   int end_block_idx_ = -1;
   size_t instr_after_end_idx_ = 0;
};

/* used by handle_operands() indirectly through Builder::copy */
uint8_t int8_mul_table[512] = {
   0, 20,  1,  1,   1,  2,   1,  3,   1,  4,   1, 5,   1,  6,   1,  7,   1,  8,   1,  9,
   1, 10,  1,  11,  1,  12,  1,  13,  1,  14,  1, 15,  1,  16,  1,  17,  1,  18,  1,  19,
   1, 20,  1,  21,  1,  22,  1,  23,  1,  24,  1, 25,  1,  26,  1,  27,  1,  28,  1,  29,
   1, 30,  1,  31,  1,  32,  1,  33,  1,  34,  1, 35,  1,  36,  1,  37,  1,  38,  1,  39,
   1, 40,  1,  41,  1,  42,  1,  43,  1,  44,  1, 45,  1,  46,  1,  47,  1,  48,  1,  49,
   1, 50,  1,  51,  1,  52,  1,  53,  1,  54,  1, 55,  1,  56,  1,  57,  1,  58,  1,  59,
   1, 60,  1,  61,  1,  62,  1,  63,  1,  64,  5, 13,  2,  33,  17, 19,  2,  34,  3,  23,
   2, 35,  11, 53,  2,  36,  7,  47,  2,  37,  3, 25,  2,  38,  7,  11,  2,  39,  53, 243,
   2, 40,  3,  27,  2,  41,  17, 35,  2,  42,  5, 17,  2,  43,  3,  29,  2,  44,  15, 23,
   2, 45,  7,  13,  2,  46,  3,  31,  2,  47,  5, 19,  2,  48,  19, 59,  2,  49,  3,  33,
   2, 50,  7,  51,  2,  51,  15, 41,  2,  52,  3, 35,  2,  53,  11, 33,  2,  54,  23, 27,
   2, 55,  3,  37,  2,  56,  9,  41,  2,  57,  5, 23,  2,  58,  3,  39,  2,  59,  7,  17,
   2, 60,  9,  241, 2,  61,  3,  41,  2,  62,  5, 25,  2,  63,  35, 245, 2,  64,  3,  43,
   5, 26,  9,  43,  3,  44,  7,  19,  10, 39,  3, 45,  4,  34,  11, 59,  3,  46,  9,  243,
   4, 35,  3,  47,  22, 53,  7,  57,  3,  48,  5, 29,  10, 245, 3,  49,  4,  37,  9,  45,
   3, 50,  7,  241, 4,  38,  3,  51,  7,  22,  5, 31,  3,  52,  7,  59,  7,  242, 3,  53,
   4, 40,  7,  23,  3,  54,  15, 45,  4,  41,  3, 55,  6,  241, 9,  47,  3,  56,  13, 13,
   5, 34,  3,  57,  4,  43,  11, 39,  3,  58,  5, 35,  4,  44,  3,  59,  6,  243, 7,  245,
   3, 60,  5,  241, 7,  26,  3,  61,  4,  46,  5, 37,  3,  62,  11, 17,  4,  47,  3,  63,
   5, 38,  5,  243, 3,  64,  7,  247, 9,  50,  5, 39,  4,  241, 33, 37,  6,  33,  13, 35,
   4, 242, 5,  245, 6,  247, 7,  29,  4,  51,  5, 41,  5,  246, 7,  249, 3,  240, 11, 19,
   5, 42,  3,  241, 4,  245, 25, 29,  3,  242, 5, 43,  4,  246, 3,  243, 17, 58,  17, 43,
   3, 244, 5,  249, 6,  37,  3,  245, 2,  240, 5, 45,  2,  241, 21, 23,  2,  242, 3,  247,
   2, 243, 5,  251, 2,  244, 29, 61,  2,  245, 3, 249, 2,  246, 17, 29,  2,  247, 9,  55,
   1, 240, 1,  241, 1,  242, 1,  243, 1,  244, 1, 245, 1,  246, 1,  247, 1,  248, 1,  249,
   1, 250, 1,  251, 1,  252, 1,  253, 1,  254, 1, 255};

aco_opcode
get_reduce_opcode(amd_gfx_level gfx_level, ReduceOp op)
{
   /* Because some 16-bit instructions are already VOP3 on GFX10, we use the
    * 32-bit opcodes (VOP2) which allows to remove the temporary VGPR and to use
    * DPP with the arithmetic instructions. This requires to sign-extend.
    */
   switch (op) {
   case iadd8:
   case iadd16:
      if (gfx_level >= GFX10) {
         return aco_opcode::v_add_u32;
      } else if (gfx_level >= GFX8) {
         return aco_opcode::v_add_u16;
      } else {
         return aco_opcode::v_add_co_u32;
      }
      break;
   case imul8:
   case imul16:
      if (gfx_level >= GFX10) {
         return aco_opcode::v_mul_lo_u16_e64;
      } else if (gfx_level >= GFX8) {
         return aco_opcode::v_mul_lo_u16;
      } else {
         return aco_opcode::v_mul_u32_u24;
      }
      break;
   case fadd16: return aco_opcode::v_add_f16;
   case fmul16: return aco_opcode::v_mul_f16;
   case imax8:
   case imax16:
      if (gfx_level >= GFX10) {
         return aco_opcode::v_max_i32;
      } else if (gfx_level >= GFX8) {
         return aco_opcode::v_max_i16;
      } else {
         return aco_opcode::v_max_i32;
      }
      break;
   case imin8:
   case imin16:
      if (gfx_level >= GFX10) {
         return aco_opcode::v_min_i32;
      } else if (gfx_level >= GFX8) {
         return aco_opcode::v_min_i16;
      } else {
         return aco_opcode::v_min_i32;
      }
      break;
   case umin8:
   case umin16:
      if (gfx_level >= GFX10) {
         return aco_opcode::v_min_u32;
      } else if (gfx_level >= GFX8) {
         return aco_opcode::v_min_u16;
      } else {
         return aco_opcode::v_min_u32;
      }
      break;
   case umax8:
   case umax16:
      if (gfx_level >= GFX10) {
         return aco_opcode::v_max_u32;
      } else if (gfx_level >= GFX8) {
         return aco_opcode::v_max_u16;
      } else {
         return aco_opcode::v_max_u32;
      }
      break;
   case fmin16: return aco_opcode::v_min_f16;
   case fmax16: return aco_opcode::v_max_f16;
   case iadd32: return gfx_level >= GFX9 ? aco_opcode::v_add_u32 : aco_opcode::v_add_co_u32;
   case imul32: return aco_opcode::v_mul_lo_u32;
   case fadd32: return aco_opcode::v_add_f32;
   case fmul32: return aco_opcode::v_mul_f32;
   case imax32: return aco_opcode::v_max_i32;
   case imin32: return aco_opcode::v_min_i32;
   case umin32: return aco_opcode::v_min_u32;
   case umax32: return aco_opcode::v_max_u32;
   case fmin32: return aco_opcode::v_min_f32;
   case fmax32: return aco_opcode::v_max_f32;
   case iand8:
   case iand16:
   case iand32: return aco_opcode::v_and_b32;
   case ixor8:
   case ixor16:
   case ixor32: return aco_opcode::v_xor_b32;
   case ior8:
   case ior16:
   case ior32: return aco_opcode::v_or_b32;
   case iadd64: return aco_opcode::num_opcodes;
   case imul64: return aco_opcode::num_opcodes;
   case fadd64: return aco_opcode::v_add_f64;
   case fmul64: return aco_opcode::v_mul_f64;
   case imin64: return aco_opcode::num_opcodes;
   case imax64: return aco_opcode::num_opcodes;
   case umin64: return aco_opcode::num_opcodes;
   case umax64: return aco_opcode::num_opcodes;
   case fmin64: return aco_opcode::v_min_f64;
   case fmax64: return aco_opcode::v_max_f64;
   case iand64: return aco_opcode::num_opcodes;
   case ior64: return aco_opcode::num_opcodes;
   case ixor64: return aco_opcode::num_opcodes;
   default: return aco_opcode::num_opcodes;
   }
}

bool
is_vop3_reduce_opcode(aco_opcode opcode)
{
   /* 64-bit reductions are VOP3. */
   if (opcode == aco_opcode::num_opcodes)
      return true;

   return instr_info.format[(int)opcode] == Format::VOP3;
}

void
emit_vadd32(Builder& bld, Definition def, Operand src0, Operand src1)
{
   Instruction* instr = bld.vadd32(def, src0, src1, false, Operand(s2), true);
   if (instr->definitions.size() >= 2) {
      assert(instr->definitions[1].regClass() == bld.lm);
      instr->definitions[1].setFixed(vcc);
   }
}

void
emit_int64_dpp_op(lower_context* ctx, PhysReg dst_reg, PhysReg src0_reg, PhysReg src1_reg,
                  PhysReg vtmp_reg, ReduceOp op, unsigned dpp_ctrl, unsigned row_mask,
                  unsigned bank_mask, bool bound_ctrl, Operand* identity = NULL)
{
   Builder bld(ctx->program, &ctx->instructions);
   Definition dst[] = {Definition(dst_reg, v1), Definition(PhysReg{dst_reg + 1}, v1)};
   Definition vtmp_def[] = {Definition(vtmp_reg, v1), Definition(PhysReg{vtmp_reg + 1}, v1)};
   Operand src0[] = {Operand(src0_reg, v1), Operand(PhysReg{src0_reg + 1}, v1)};
   Operand src1[] = {Operand(src1_reg, v1), Operand(PhysReg{src1_reg + 1}, v1)};
   Operand src1_64 = Operand(src1_reg, v2);
   Operand vtmp_op[] = {Operand(vtmp_reg, v1), Operand(PhysReg{vtmp_reg + 1}, v1)};
   Operand vtmp_op64 = Operand(vtmp_reg, v2);
   if (op == iadd64) {
      if (ctx->program->gfx_level >= GFX10) {
         if (identity)
            bld.vop1(aco_opcode::v_mov_b32, vtmp_def[0], identity[0]);
         bld.vop1_dpp(aco_opcode::v_mov_b32, vtmp_def[0], src0[0], dpp_ctrl, row_mask, bank_mask,
                      bound_ctrl);
         bld.vop3(aco_opcode::v_add_co_u32_e64, dst[0], bld.def(bld.lm, vcc), vtmp_op[0], src1[0]);
      } else {
         bld.vop2_dpp(aco_opcode::v_add_co_u32, dst[0], bld.def(bld.lm, vcc), src0[0], src1[0],
                      dpp_ctrl, row_mask, bank_mask, bound_ctrl);
      }
      bld.vop2_dpp(aco_opcode::v_addc_co_u32, dst[1], bld.def(bld.lm, vcc), src0[1], src1[1],
                   Operand(vcc, bld.lm), dpp_ctrl, row_mask, bank_mask, bound_ctrl);
   } else if (op == iand64) {
      bld.vop2_dpp(aco_opcode::v_and_b32, dst[0], src0[0], src1[0], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
      bld.vop2_dpp(aco_opcode::v_and_b32, dst[1], src0[1], src1[1], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
   } else if (op == ior64) {
      bld.vop2_dpp(aco_opcode::v_or_b32, dst[0], src0[0], src1[0], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
      bld.vop2_dpp(aco_opcode::v_or_b32, dst[1], src0[1], src1[1], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
   } else if (op == ixor64) {
      bld.vop2_dpp(aco_opcode::v_xor_b32, dst[0], src0[0], src1[0], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
      bld.vop2_dpp(aco_opcode::v_xor_b32, dst[1], src0[1], src1[1], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
   } else if (op == umin64 || op == umax64 || op == imin64 || op == imax64) {
      aco_opcode cmp = aco_opcode::num_opcodes;
      switch (op) {
      case umin64: cmp = aco_opcode::v_cmp_gt_u64; break;
      case umax64: cmp = aco_opcode::v_cmp_lt_u64; break;
      case imin64: cmp = aco_opcode::v_cmp_gt_i64; break;
      case imax64: cmp = aco_opcode::v_cmp_lt_i64; break;
      default: break;
      }

      if (identity) {
         bld.vop1(aco_opcode::v_mov_b32, vtmp_def[0], identity[0]);
         bld.vop1(aco_opcode::v_mov_b32, vtmp_def[1], identity[1]);
      }
      bld.vop1_dpp(aco_opcode::v_mov_b32, vtmp_def[0], src0[0], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
      bld.vop1_dpp(aco_opcode::v_mov_b32, vtmp_def[1], src0[1], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);

      bld.vopc(cmp, bld.def(bld.lm, vcc), vtmp_op64, src1_64);
      bld.vop2(aco_opcode::v_cndmask_b32, dst[0], vtmp_op[0], src1[0], Operand(vcc, bld.lm));
      bld.vop2(aco_opcode::v_cndmask_b32, dst[1], vtmp_op[1], src1[1], Operand(vcc, bld.lm));
   } else if (op == imul64) {
      /* t4 = dpp(x_hi)
       * t1 = umul_lo(t4, y_lo)
       * t3 = dpp(x_lo)
       * t0 = umul_lo(t3, y_hi)
       * t2 = iadd(t0, t1)
       * t5 = umul_hi(t3, y_lo)
       * res_hi = iadd(t2, t5)
       * res_lo = umul_lo(t3, y_lo)
       * Requires that res_hi != src0[0] and res_hi != src1[0]
       * and that vtmp[0] != res_hi.
       */
      if (identity)
         bld.vop1(aco_opcode::v_mov_b32, vtmp_def[0], identity[1]);
      bld.vop1_dpp(aco_opcode::v_mov_b32, vtmp_def[0], src0[1], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
      bld.vop3(aco_opcode::v_mul_lo_u32, vtmp_def[1], vtmp_op[0], src1[0]);
      if (identity)
         bld.vop1(aco_opcode::v_mov_b32, vtmp_def[0], identity[0]);
      bld.vop1_dpp(aco_opcode::v_mov_b32, vtmp_def[0], src0[0], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
      bld.vop3(aco_opcode::v_mul_lo_u32, vtmp_def[0], vtmp_op[0], src1[1]);
      emit_vadd32(bld, vtmp_def[1], vtmp_op[0], vtmp_op[1]);
      if (identity)
         bld.vop1(aco_opcode::v_mov_b32, vtmp_def[0], identity[0]);
      bld.vop1_dpp(aco_opcode::v_mov_b32, vtmp_def[0], src0[0], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
      bld.vop3(aco_opcode::v_mul_hi_u32, vtmp_def[0], vtmp_op[0], src1[0]);
      emit_vadd32(bld, dst[1], vtmp_op[1], vtmp_op[0]);
      if (identity)
         bld.vop1(aco_opcode::v_mov_b32, vtmp_def[0], identity[0]);
      bld.vop1_dpp(aco_opcode::v_mov_b32, vtmp_def[0], src0[0], dpp_ctrl, row_mask, bank_mask,
                   bound_ctrl);
      bld.vop3(aco_opcode::v_mul_lo_u32, dst[0], vtmp_op[0], src1[0]);
   }
}

void
emit_int64_op(lower_context* ctx, PhysReg dst_reg, PhysReg src0_reg, PhysReg src1_reg, PhysReg vtmp,
              ReduceOp op)
{
   Builder bld(ctx->program, &ctx->instructions);
   Definition dst[] = {Definition(dst_reg, v1), Definition(PhysReg{dst_reg + 1}, v1)};
   RegClass src0_rc = src0_reg.reg() >= 256 ? v1 : s1;
   Operand src0[] = {Operand(src0_reg, src0_rc), Operand(PhysReg{src0_reg + 1}, src0_rc)};
   Operand src1[] = {Operand(src1_reg, v1), Operand(PhysReg{src1_reg + 1}, v1)};
   Operand src0_64 = Operand(src0_reg, src0_reg.reg() >= 256 ? v2 : s2);
   Operand src1_64 = Operand(src1_reg, v2);

   if (src0_rc == s1 &&
       (op == imul64 || op == umin64 || op == umax64 || op == imin64 || op == imax64)) {
      assert(vtmp.reg() != 0);
      bld.vop1(aco_opcode::v_mov_b32, Definition(vtmp, v1), src0[0]);
      bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{vtmp + 1}, v1), src0[1]);
      src0_reg = vtmp;
      src0[0] = Operand(vtmp, v1);
      src0[1] = Operand(PhysReg{vtmp + 1}, v1);
      src0_64 = Operand(vtmp, v2);
   } else if (src0_rc == s1 && op == iadd64) {
      assert(vtmp.reg() != 0);
      bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{vtmp + 1}, v1), src0[1]);
      src0[1] = Operand(PhysReg{vtmp + 1}, v1);
   }

   if (op == iadd64) {
      if (ctx->program->gfx_level >= GFX10) {
         bld.vop3(aco_opcode::v_add_co_u32_e64, dst[0], bld.def(bld.lm, vcc), src0[0], src1[0]);
      } else {
         bld.vop2(aco_opcode::v_add_co_u32, dst[0], bld.def(bld.lm, vcc), src0[0], src1[0]);
      }
      bld.vop2(aco_opcode::v_addc_co_u32, dst[1], bld.def(bld.lm, vcc), src0[1], src1[1],
               Operand(vcc, bld.lm));
   } else if (op == iand64) {
      bld.vop2(aco_opcode::v_and_b32, dst[0], src0[0], src1[0]);
      bld.vop2(aco_opcode::v_and_b32, dst[1], src0[1], src1[1]);
   } else if (op == ior64) {
      bld.vop2(aco_opcode::v_or_b32, dst[0], src0[0], src1[0]);
      bld.vop2(aco_opcode::v_or_b32, dst[1], src0[1], src1[1]);
   } else if (op == ixor64) {
      bld.vop2(aco_opcode::v_xor_b32, dst[0], src0[0], src1[0]);
      bld.vop2(aco_opcode::v_xor_b32, dst[1], src0[1], src1[1]);
   } else if (op == umin64 || op == umax64 || op == imin64 || op == imax64) {
      aco_opcode cmp = aco_opcode::num_opcodes;
      switch (op) {
      case umin64: cmp = aco_opcode::v_cmp_gt_u64; break;
      case umax64: cmp = aco_opcode::v_cmp_lt_u64; break;
      case imin64: cmp = aco_opcode::v_cmp_gt_i64; break;
      case imax64: cmp = aco_opcode::v_cmp_lt_i64; break;
      default: break;
      }

      bld.vopc(cmp, bld.def(bld.lm, vcc), src0_64, src1_64);
      bld.vop2(aco_opcode::v_cndmask_b32, dst[0], src0[0], src1[0], Operand(vcc, bld.lm));
      bld.vop2(aco_opcode::v_cndmask_b32, dst[1], src0[1], src1[1], Operand(vcc, bld.lm));
   } else if (op == imul64) {
      if (src1_reg == dst_reg) {
         /* it's fine if src0==dst but not if src1==dst */
         std::swap(src0_reg, src1_reg);
         std::swap(src0[0], src1[0]);
         std::swap(src0[1], src1[1]);
         std::swap(src0_64, src1_64);
      }
      assert(!(src0_reg == src1_reg));
      /* t1 = umul_lo(x_hi, y_lo)
       * t0 = umul_lo(x_lo, y_hi)
       * t2 = iadd(t0, t1)
       * t5 = umul_hi(x_lo, y_lo)
       * res_hi = iadd(t2, t5)
       * res_lo = umul_lo(x_lo, y_lo)
       * assumes that it's ok to modify x_hi/y_hi, since we might not have vtmp
       */
      Definition tmp0_def(PhysReg{src0_reg + 1}, v1);
      Definition tmp1_def(PhysReg{src1_reg + 1}, v1);
      Operand tmp0_op = src0[1];
      Operand tmp1_op = src1[1];
      bld.vop3(aco_opcode::v_mul_lo_u32, tmp0_def, src0[1], src1[0]);
      bld.vop3(aco_opcode::v_mul_lo_u32, tmp1_def, src0[0], src1[1]);
      emit_vadd32(bld, tmp0_def, tmp1_op, tmp0_op);
      bld.vop3(aco_opcode::v_mul_hi_u32, tmp1_def, src0[0], src1[0]);
      emit_vadd32(bld, dst[1], tmp0_op, tmp1_op);
      bld.vop3(aco_opcode::v_mul_lo_u32, dst[0], src0[0], src1[0]);
   }
}

void
emit_dpp_op(lower_context* ctx, PhysReg dst_reg, PhysReg src0_reg, PhysReg src1_reg, PhysReg vtmp,
            ReduceOp op, unsigned size, unsigned dpp_ctrl, unsigned row_mask, unsigned bank_mask,
            bool bound_ctrl, Operand* identity = NULL) /* for VOP3 with sparse writes */
{
   Builder bld(ctx->program, &ctx->instructions);
   RegClass rc = RegClass(RegType::vgpr, size);
   Definition dst(dst_reg, rc);
   Operand src0(src0_reg, rc);
   Operand src1(src1_reg, rc);

   aco_opcode opcode = get_reduce_opcode(ctx->program->gfx_level, op);
   bool vop3 = is_vop3_reduce_opcode(opcode);

   if (!vop3) {
      if (opcode == aco_opcode::v_add_co_u32)
         bld.vop2_dpp(opcode, dst, bld.def(bld.lm, vcc), src0, src1, dpp_ctrl, row_mask, bank_mask,
                      bound_ctrl);
      else
         bld.vop2_dpp(opcode, dst, src0, src1, dpp_ctrl, row_mask, bank_mask, bound_ctrl);
      return;
   }

   if (opcode == aco_opcode::num_opcodes) {
      emit_int64_dpp_op(ctx, dst_reg, src0_reg, src1_reg, vtmp, op, dpp_ctrl, row_mask, bank_mask,
                        bound_ctrl, identity);
      return;
   }

   if (identity)
      bld.vop1(aco_opcode::v_mov_b32, Definition(vtmp, v1), identity[0]);
   if (identity && size >= 2)
      bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{vtmp + 1}, v1), identity[1]);

   for (unsigned i = 0; i < size; i++)
      bld.vop1_dpp(aco_opcode::v_mov_b32, Definition(PhysReg{vtmp + i}, v1),
                   Operand(PhysReg{src0_reg + i}, v1), dpp_ctrl, row_mask, bank_mask, bound_ctrl);

   bld.vop3(opcode, dst, Operand(vtmp, rc), src1);
}

void
emit_op(lower_context* ctx, PhysReg dst_reg, PhysReg src0_reg, PhysReg src1_reg, PhysReg vtmp,
        ReduceOp op, unsigned size)
{
   Builder bld(ctx->program, &ctx->instructions);
   RegClass rc = RegClass(RegType::vgpr, size);
   Definition dst(dst_reg, rc);
   Operand src0(src0_reg, RegClass(src0_reg.reg() >= 256 ? RegType::vgpr : RegType::sgpr, size));
   Operand src1(src1_reg, rc);

   aco_opcode opcode = get_reduce_opcode(ctx->program->gfx_level, op);
   bool vop3 = is_vop3_reduce_opcode(opcode);

   if (opcode == aco_opcode::num_opcodes) {
      emit_int64_op(ctx, dst_reg, src0_reg, src1_reg, vtmp, op);
      return;
   }

   if (vop3) {
      bld.vop3(opcode, dst, src0, src1);
   } else if (opcode == aco_opcode::v_add_co_u32) {
      bld.vop2(opcode, dst, bld.def(bld.lm, vcc), src0, src1);
   } else {
      bld.vop2(opcode, dst, src0, src1);
   }
}

void
emit_dpp_mov(lower_context* ctx, PhysReg dst, PhysReg src0, unsigned size, unsigned dpp_ctrl,
             unsigned row_mask, unsigned bank_mask, bool bound_ctrl)
{
   Builder bld(ctx->program, &ctx->instructions);
   for (unsigned i = 0; i < size; i++) {
      bld.vop1_dpp(aco_opcode::v_mov_b32, Definition(PhysReg{dst + i}, v1),
                   Operand(PhysReg{src0 + i}, v1), dpp_ctrl, row_mask, bank_mask, bound_ctrl);
   }
}

void
emit_ds_swizzle(Builder bld, PhysReg dst, PhysReg src, unsigned size, unsigned ds_pattern)
{
   for (unsigned i = 0; i < size; i++) {
      bld.ds(aco_opcode::ds_swizzle_b32, Definition(PhysReg{dst + i}, v1),
             Operand(PhysReg{src + i}, v1), ds_pattern);
   }
}

void
emit_reduction(lower_context* ctx, aco_opcode op, ReduceOp reduce_op, unsigned cluster_size,
               PhysReg tmp, PhysReg stmp, PhysReg vtmp, PhysReg sitmp, Operand src, Definition dst)
{
   assert(cluster_size == ctx->program->wave_size || op == aco_opcode::p_reduce);
   assert(cluster_size <= ctx->program->wave_size);

   Builder bld(ctx->program, &ctx->instructions);

   Operand identity[2];
   identity[0] = Operand::c32(get_reduction_identity(reduce_op, 0));
   identity[1] = Operand::c32(get_reduction_identity(reduce_op, 1));
   Operand vcndmask_identity[2] = {identity[0], identity[1]};

   /* First, copy the source to tmp and set inactive lanes to the identity */
   bld.sop1(Builder::s_or_saveexec, Definition(stmp, bld.lm), Definition(scc, s1),
            Definition(exec, bld.lm), Operand::c64(UINT64_MAX), Operand(exec, bld.lm));

   /* On GFX10+ v_writelane_b32/v_cndmask_b32_e64 can take a literal */
   if (ctx->program->gfx_level < GFX10) {
      for (unsigned i = 0; i < src.size(); i++) {
         /* p_exclusive_scan uses identity for v_writelane_b32 */
         if (identity[i].isLiteral() && op == aco_opcode::p_exclusive_scan) {
            bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg{sitmp + i}, s1), identity[i]);
            identity[i] = Operand(PhysReg{sitmp + i}, s1);

            bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{tmp + i}, v1), identity[i]);
            vcndmask_identity[i] = Operand(PhysReg{tmp + i}, v1);
         } else if (identity[i].isLiteral()) {
            bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{tmp + i}, v1), identity[i]);
            vcndmask_identity[i] = Operand(PhysReg{tmp + i}, v1);
         }
      }
   }

   for (unsigned i = 0; i < src.size(); i++) {
      bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(PhysReg{tmp + i}, v1),
                   vcndmask_identity[i], Operand(PhysReg{src.physReg() + i}, v1),
                   Operand(stmp, bld.lm));
   }

   if (src.regClass() == v1b) {
      if (ctx->program->gfx_level >= GFX8 && ctx->program->gfx_level < GFX11) {
         aco_ptr<SDWA_instruction> sdwa{create_instruction<SDWA_instruction>(
            aco_opcode::v_mov_b32, asSDWA(Format::VOP1), 1, 1)};
         sdwa->operands[0] = Operand(PhysReg{tmp}, v1);
         sdwa->definitions[0] = Definition(PhysReg{tmp}, v1);
         bool sext = reduce_op == imin8 || reduce_op == imax8;
         sdwa->sel[0] = SubdwordSel(1, 0, sext);
         sdwa->dst_sel = SubdwordSel::dword;
         bld.insert(std::move(sdwa));
      } else {
         aco_opcode opcode;

         if (reduce_op == imin8 || reduce_op == imax8)
            opcode = aco_opcode::v_bfe_i32;
         else
            opcode = aco_opcode::v_bfe_u32;

         bld.vop3(opcode, Definition(PhysReg{tmp}, v1), Operand(PhysReg{tmp}, v1), Operand::zero(),
                  Operand::c32(8u));
      }
   } else if (src.regClass() == v2b) {
      bool is_add_cmp = reduce_op == iadd16 || reduce_op == imax16 || reduce_op == imin16 ||
                        reduce_op == umin16 || reduce_op == umax16;
      if (ctx->program->gfx_level >= GFX10 && ctx->program->gfx_level < GFX11 && is_add_cmp) {
         aco_ptr<SDWA_instruction> sdwa{create_instruction<SDWA_instruction>(
            aco_opcode::v_mov_b32, asSDWA(Format::VOP1), 1, 1)};
         sdwa->operands[0] = Operand(PhysReg{tmp}, v1);
         sdwa->definitions[0] = Definition(PhysReg{tmp}, v1);
         bool sext = reduce_op == imin16 || reduce_op == imax16 || reduce_op == iadd16;
         sdwa->sel[0] = SubdwordSel(2, 0, sext);
         sdwa->dst_sel = SubdwordSel::dword;
         bld.insert(std::move(sdwa));
      } else if (ctx->program->gfx_level <= GFX7 ||
                 (ctx->program->gfx_level >= GFX11 && is_add_cmp)) {
         aco_opcode opcode;

         if (reduce_op == imin16 || reduce_op == imax16 || reduce_op == iadd16)
            opcode = aco_opcode::v_bfe_i32;
         else
            opcode = aco_opcode::v_bfe_u32;

         bld.vop3(opcode, Definition(PhysReg{tmp}, v1), Operand(PhysReg{tmp}, v1), Operand::zero(),
                  Operand::c32(16u));
      }
   }

   bool reduction_needs_last_op = false;
   switch (op) {
   case aco_opcode::p_reduce:
      if (cluster_size == 1)
         break;

      if (ctx->program->gfx_level <= GFX7) {
         reduction_needs_last_op = true;
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), (1 << 15) | dpp_quad_perm(1, 0, 3, 2));
         if (cluster_size == 2)
            break;
         emit_op(ctx, tmp, vtmp, tmp, PhysReg{0}, reduce_op, src.size());
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), (1 << 15) | dpp_quad_perm(2, 3, 0, 1));
         if (cluster_size == 4)
            break;
         emit_op(ctx, tmp, vtmp, tmp, PhysReg{0}, reduce_op, src.size());
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x1f, 0, 0x04));
         if (cluster_size == 8)
            break;
         emit_op(ctx, tmp, vtmp, tmp, PhysReg{0}, reduce_op, src.size());
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x1f, 0, 0x08));
         if (cluster_size == 16)
            break;
         emit_op(ctx, tmp, vtmp, tmp, PhysReg{0}, reduce_op, src.size());
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x1f, 0, 0x10));
         if (cluster_size == 32)
            break;
         emit_op(ctx, tmp, vtmp, tmp, PhysReg{0}, reduce_op, src.size());
         for (unsigned i = 0; i < src.size(); i++)
            bld.readlane(Definition(PhysReg{dst.physReg() + i}, s1), Operand(PhysReg{tmp + i}, v1),
                         Operand::zero());
         // TODO: it would be more effective to do the last reduction step on SALU
         emit_op(ctx, tmp, dst.physReg(), tmp, vtmp, reduce_op, src.size());
         reduction_needs_last_op = false;
         break;
      }

      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_quad_perm(1, 0, 3, 2), 0xf,
                  0xf, false);
      if (cluster_size == 2)
         break;
      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_quad_perm(2, 3, 0, 1), 0xf,
                  0xf, false);
      if (cluster_size == 4)
         break;
      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_half_mirror, 0xf, 0xf,
                  false);
      if (cluster_size == 8)
         break;
      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_mirror, 0xf, 0xf, false);
      if (cluster_size == 16)
         break;

      if (ctx->program->gfx_level >= GFX10) {
         /* GFX10+ doesn't support row_bcast15 and row_bcast31 */
         for (unsigned i = 0; i < src.size(); i++)
            bld.vop3(aco_opcode::v_permlanex16_b32, Definition(PhysReg{vtmp + i}, v1),
                     Operand(PhysReg{tmp + i}, v1), Operand::zero(), Operand::zero());

         if (cluster_size == 32) {
            reduction_needs_last_op = true;
            break;
         }

         emit_op(ctx, tmp, tmp, vtmp, PhysReg{0}, reduce_op, src.size());
         for (unsigned i = 0; i < src.size(); i++)
            bld.readlane(Definition(PhysReg{dst.physReg() + i}, s1), Operand(PhysReg{tmp + i}, v1),
                         Operand::zero());
         // TODO: it would be more effective to do the last reduction step on SALU
         emit_op(ctx, tmp, dst.physReg(), tmp, vtmp, reduce_op, src.size());
         break;
      }

      if (cluster_size == 32) {
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x1f, 0, 0x10));
         reduction_needs_last_op = true;
         break;
      }
      assert(cluster_size == 64);
      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_bcast15, 0xa, 0xf,
                  false);
      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_bcast31, 0xc, 0xf,
                  false);
      break;
   case aco_opcode::p_exclusive_scan:
      if (ctx->program->gfx_level >= GFX10) { /* gfx10 doesn't support wf_sr1, so emulate it */
         /* shift rows right */
         emit_dpp_mov(ctx, vtmp, tmp, src.size(), dpp_row_sr(1), 0xf, 0xf, true);

         /* fill in the gaps in rows 1 and 3 */
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_lo, s1), Operand::c32(0x10000u));
         if (ctx->program->wave_size == 64)
            bld.sop1(aco_opcode::s_mov_b32, Definition(exec_hi, s1), Operand::c32(0x10000u));
         for (unsigned i = 0; i < src.size(); i++) {
            Instruction* perm =
               bld.vop3(aco_opcode::v_permlanex16_b32, Definition(PhysReg{vtmp + i}, v1),
                        Operand(PhysReg{tmp + i}, v1), Operand::c32(0xffffffffu),
                        Operand::c32(0xffffffffu))
                  .instr;
            perm->valu().opsel = 1; /* FI (Fetch Inactive) */
         }
         bld.sop1(Builder::s_mov, Definition(exec, bld.lm), Operand::c64(UINT64_MAX));

         if (ctx->program->wave_size == 64) {
            /* fill in the gap in row 2 */
            for (unsigned i = 0; i < src.size(); i++) {
               bld.readlane(Definition(PhysReg{sitmp + i}, s1), Operand(PhysReg{tmp + i}, v1),
                            Operand::c32(31u));
               bld.writelane(Definition(PhysReg{vtmp + i}, v1), Operand(PhysReg{sitmp + i}, s1),
                             Operand::c32(32u), Operand(PhysReg{vtmp + i}, v1));
            }
         }
         std::swap(tmp, vtmp);
      } else if (ctx->program->gfx_level >= GFX8) {
         emit_dpp_mov(ctx, tmp, tmp, src.size(), dpp_wf_sr1, 0xf, 0xf, true);
      } else {
         // TODO: use LDS on CS with a single write and shifted read
         /* wavefront shift_right by 1 on SI/CI */
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), (1 << 15) | dpp_quad_perm(0, 0, 1, 2));
         emit_ds_swizzle(bld, tmp, tmp, src.size(),
                         ds_pattern_bitmode(0x1F, 0x00, 0x07)); /* mirror(8) */
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_lo, s1), Operand::c32(0x10101010u));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_hi, s1), Operand(exec_lo, s1));
         for (unsigned i = 0; i < src.size(); i++)
            bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{vtmp + i}, v1),
                     Operand(PhysReg{tmp + i}, v1));

         bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(UINT64_MAX));
         emit_ds_swizzle(bld, tmp, tmp, src.size(),
                         ds_pattern_bitmode(0x1F, 0x00, 0x08)); /* swap(8) */
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_lo, s1), Operand::c32(0x01000100u));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_hi, s1), Operand(exec_lo, s1));
         for (unsigned i = 0; i < src.size(); i++)
            bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{vtmp + i}, v1),
                     Operand(PhysReg{tmp + i}, v1));

         bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(UINT64_MAX));
         emit_ds_swizzle(bld, tmp, tmp, src.size(),
                         ds_pattern_bitmode(0x1F, 0x00, 0x10)); /* swap(16) */
         bld.sop2(aco_opcode::s_bfm_b32, Definition(exec_lo, s1), Operand::c32(1u),
                  Operand::c32(16u));
         bld.sop2(aco_opcode::s_bfm_b32, Definition(exec_hi, s1), Operand::c32(1u),
                  Operand::c32(16u));
         for (unsigned i = 0; i < src.size(); i++)
            bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{vtmp + i}, v1),
                     Operand(PhysReg{tmp + i}, v1));

         bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(UINT64_MAX));
         for (unsigned i = 0; i < src.size(); i++) {
            bld.writelane(Definition(PhysReg{vtmp + i}, v1), identity[i], Operand::zero(),
                          Operand(PhysReg{vtmp + i}, v1));
            bld.readlane(Definition(PhysReg{sitmp + i}, s1), Operand(PhysReg{tmp + i}, v1),
                         Operand::zero());
            bld.writelane(Definition(PhysReg{vtmp + i}, v1), Operand(PhysReg{sitmp + i}, s1),
                          Operand::c32(32u), Operand(PhysReg{vtmp + i}, v1));
            identity[i] = Operand::zero(); /* prevent further uses of identity */
         }
         std::swap(tmp, vtmp);
      }

      for (unsigned i = 0; i < src.size(); i++) {
         if (!identity[i].isConstant() ||
             identity[i].constantValue()) { /* bound_ctrl should take care of this otherwise */
            if (ctx->program->gfx_level < GFX10)
               assert((identity[i].isConstant() && !identity[i].isLiteral()) ||
                      identity[i].physReg() == PhysReg{sitmp + i});
            bld.writelane(Definition(PhysReg{tmp + i}, v1), identity[i], Operand::zero(),
                          Operand(PhysReg{tmp + i}, v1));
         }
      }
      FALLTHROUGH;
   case aco_opcode::p_inclusive_scan:
      assert(cluster_size == ctx->program->wave_size);
      if (ctx->program->gfx_level <= GFX7) {
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x1e, 0x00, 0x00));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_lo, s1), Operand::c32(0xAAAAAAAAu));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_hi, s1), Operand(exec_lo, s1));
         emit_op(ctx, tmp, tmp, vtmp, PhysReg{0}, reduce_op, src.size());

         bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(UINT64_MAX));
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x1c, 0x01, 0x00));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_lo, s1), Operand::c32(0xCCCCCCCCu));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_hi, s1), Operand(exec_lo, s1));
         emit_op(ctx, tmp, tmp, vtmp, PhysReg{0}, reduce_op, src.size());

         bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(UINT64_MAX));
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x18, 0x03, 0x00));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_lo, s1), Operand::c32(0xF0F0F0F0u));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_hi, s1), Operand(exec_lo, s1));
         emit_op(ctx, tmp, tmp, vtmp, PhysReg{0}, reduce_op, src.size());

         bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(UINT64_MAX));
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x10, 0x07, 0x00));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_lo, s1), Operand::c32(0xFF00FF00u));
         bld.sop1(aco_opcode::s_mov_b32, Definition(exec_hi, s1), Operand(exec_lo, s1));
         emit_op(ctx, tmp, tmp, vtmp, PhysReg{0}, reduce_op, src.size());

         bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(UINT64_MAX));
         emit_ds_swizzle(bld, vtmp, tmp, src.size(), ds_pattern_bitmode(0x00, 0x0f, 0x00));
         bld.sop2(aco_opcode::s_bfm_b32, Definition(exec_lo, s1), Operand::c32(16u),
                  Operand::c32(16u));
         bld.sop2(aco_opcode::s_bfm_b32, Definition(exec_hi, s1), Operand::c32(16u),
                  Operand::c32(16u));
         emit_op(ctx, tmp, tmp, vtmp, PhysReg{0}, reduce_op, src.size());

         for (unsigned i = 0; i < src.size(); i++)
            bld.readlane(Definition(PhysReg{sitmp + i}, s1), Operand(PhysReg{tmp + i}, v1),
                         Operand::c32(31u));
         bld.sop2(aco_opcode::s_bfm_b64, Definition(exec, s2), Operand::c32(32u),
                  Operand::c32(32u));
         emit_op(ctx, tmp, sitmp, tmp, vtmp, reduce_op, src.size());
         break;
      }

      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_sr(1), 0xf, 0xf, false,
                  identity);
      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_sr(2), 0xf, 0xf, false,
                  identity);
      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_sr(4), 0xf, 0xf, false,
                  identity);
      emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_sr(8), 0xf, 0xf, false,
                  identity);
      if (ctx->program->gfx_level >= GFX10) {
         if (ctx->program->wave_size == 64) {
            bld.sop1(aco_opcode::s_bitreplicate_b64_b32, Definition(exec, s2),
                     Operand::c32(0xff00ff00u));
         } else {
            bld.sop2(aco_opcode::s_bfm_b32, Definition(exec_lo, s1), Operand::c32(16u),
                     Operand::c32(16u));
         }
         for (unsigned i = 0; i < src.size(); i++) {
            Instruction* perm =
               bld.vop3(aco_opcode::v_permlanex16_b32, Definition(PhysReg{vtmp + i}, v1),
                        Operand(PhysReg{tmp + i}, v1), Operand::c32(0xffffffffu),
                        Operand::c32(0xffffffffu))
                  .instr;
            perm->valu().opsel = 1; /* FI (Fetch Inactive) */
         }
         emit_op(ctx, tmp, tmp, vtmp, PhysReg{0}, reduce_op, src.size());

         if (ctx->program->wave_size == 64) {
            bld.sop2(aco_opcode::s_bfm_b64, Definition(exec, s2), Operand::c32(32u),
                     Operand::c32(32u));
            for (unsigned i = 0; i < src.size(); i++)
               bld.readlane(Definition(PhysReg{sitmp + i}, s1), Operand(PhysReg{tmp + i}, v1),
                            Operand::c32(31u));
            emit_op(ctx, tmp, sitmp, tmp, vtmp, reduce_op, src.size());
         }
      } else {
         emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_bcast15, 0xa, 0xf,
                     false, identity);
         emit_dpp_op(ctx, tmp, tmp, tmp, vtmp, reduce_op, src.size(), dpp_row_bcast31, 0xc, 0xf,
                     false, identity);
      }
      break;
   default: unreachable("Invalid reduction mode");
   }

   if (op == aco_opcode::p_reduce) {
      if (reduction_needs_last_op && dst.regClass().type() == RegType::vgpr) {
         bld.sop1(Builder::s_mov, Definition(exec, bld.lm), Operand(stmp, bld.lm));
         emit_op(ctx, dst.physReg(), tmp, vtmp, PhysReg{0}, reduce_op, src.size());
         return;
      }

      if (reduction_needs_last_op)
         emit_op(ctx, tmp, vtmp, tmp, PhysReg{0}, reduce_op, src.size());
   }

   /* restore exec */
   bld.sop1(Builder::s_mov, Definition(exec, bld.lm), Operand(stmp, bld.lm));

   if (dst.regClass().type() == RegType::sgpr) {
      for (unsigned k = 0; k < src.size(); k++) {
         bld.readlane(Definition(PhysReg{dst.physReg() + k}, s1), Operand(PhysReg{tmp + k}, v1),
                      Operand::c32(ctx->program->wave_size - 1));
      }
   } else if (dst.physReg() != tmp) {
      for (unsigned k = 0; k < src.size(); k++) {
         bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg{dst.physReg() + k}, v1),
                  Operand(PhysReg{tmp + k}, v1));
      }
   }
}

void
adjust_bpermute_dst(Builder& bld, Definition dst, Operand input_data)
{
   /* RA assumes that the result is always in the low part of the register, so we have to shift,
    * if it's not there already.
    */
   if (input_data.physReg().byte()) {
      unsigned right_shift = input_data.physReg().byte() * 8;
      bld.vop2(aco_opcode::v_lshrrev_b32, dst, Operand::c32(right_shift),
               Operand(dst.physReg(), dst.regClass()));
   }
}

void
emit_bpermute_permlane(Program* program, aco_ptr<Instruction>& instr, Builder& bld)
{
   /* Emulates proper bpermute on GFX11 in wave64 mode.
    *
    * Similar to emit_gfx10_wave64_bpermute, but uses the new
    * v_permlane64_b32 instruction to swap data between lo and hi halves.
    */

   assert(program->gfx_level >= GFX11);
   assert(program->wave_size == 64);

   Definition dst = instr->definitions[0];
   Definition tmp_exec = instr->definitions[1];
   Definition clobber_scc = instr->definitions[2];
   Operand tmp_op = instr->operands[0];
   Operand index_x4 = instr->operands[1];
   Operand input_data = instr->operands[2];
   Operand same_half = instr->operands[3];

   assert(dst.regClass() == v1);
   assert(tmp_exec.regClass() == bld.lm);
   assert(clobber_scc.isFixed() && clobber_scc.physReg() == scc);
   assert(same_half.regClass() == bld.lm);
   assert(tmp_op.regClass() == v1.as_linear());
   assert(index_x4.regClass() == v1);
   assert(input_data.regClass().type() == RegType::vgpr);
   assert(input_data.bytes() <= 4);

   Definition tmp_def(tmp_op.physReg(), tmp_op.regClass());

   /* Permute the input within the same half-wave. */
   bld.ds(aco_opcode::ds_bpermute_b32, dst, index_x4, input_data);

   /* Save EXEC and enable all lanes. */
   bld.sop1(aco_opcode::s_or_saveexec_b64, tmp_exec, clobber_scc, Definition(exec, s2),
            Operand::c32(-1u), Operand(exec, s2));

   /* Copy input data from other half to current half's linear VGPR. */
   bld.vop1(aco_opcode::v_permlane64_b32, tmp_def, input_data);

   /* Permute the input from the other half-wave, write to linear VGPR. */
   bld.ds(aco_opcode::ds_bpermute_b32, tmp_def, index_x4, tmp_op);

   /* Restore saved EXEC. */
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand(tmp_exec.physReg(), s2));

   /* Select correct permute result. */
   bld.vop2_e64(aco_opcode::v_cndmask_b32, dst, tmp_op, Operand(dst.physReg(), dst.regClass()),
                same_half);

   adjust_bpermute_dst(bld, dst, input_data);
}

void
emit_bpermute_shared_vgpr(Program* program, aco_ptr<Instruction>& instr, Builder& bld)
{
   /* Emulates proper bpermute on GFX10 in wave64 mode.
    *
    * This is necessary because on GFX10 the bpermute instruction only works
    * on half waves (you can think of it as having a cluster size of 32), so we
    * manually swap the data between the two halves using two shared VGPRs.
    */

   assert(program->gfx_level >= GFX10 && program->gfx_level <= GFX10_3);
   assert(program->wave_size == 64);

   unsigned shared_vgpr_reg_0 = align(program->config->num_vgprs, 4) + 256;
   Definition dst = instr->definitions[0];
   Definition tmp_exec = instr->definitions[1];
   Definition clobber_scc = instr->definitions[2];
   Operand index_x4 = instr->operands[0];
   Operand input_data = instr->operands[1];
   Operand same_half = instr->operands[2];

   assert(dst.regClass() == v1);
   assert(tmp_exec.regClass() == bld.lm);
   assert(clobber_scc.isFixed() && clobber_scc.physReg() == scc);
   assert(same_half.regClass() == bld.lm);
   assert(index_x4.regClass() == v1);
   assert(input_data.regClass().type() == RegType::vgpr);
   assert(input_data.bytes() <= 4);
   assert(dst.physReg() != index_x4.physReg());
   assert(dst.physReg() != input_data.physReg());
   assert(tmp_exec.physReg() != same_half.physReg());

   PhysReg shared_vgpr_lo(shared_vgpr_reg_0);
   PhysReg shared_vgpr_hi(shared_vgpr_reg_0 + 1);

   /* Permute the input within the same half-wave */
   bld.ds(aco_opcode::ds_bpermute_b32, dst, index_x4, input_data);

   /* HI: Copy data from high lanes 32-63 to shared vgpr */
   bld.vop1_dpp(aco_opcode::v_mov_b32, Definition(shared_vgpr_hi, v1), input_data,
                dpp_quad_perm(0, 1, 2, 3), 0xc, 0xf, false);
   /* Save EXEC */
   bld.sop1(aco_opcode::s_mov_b64, tmp_exec, Operand(exec, s2));
   /* Set EXEC to enable LO lanes only */
   bld.sop2(aco_opcode::s_bfm_b64, Definition(exec, s2), Operand::c32(32u), Operand::zero());
   /* LO: Copy data from low lanes 0-31 to shared vgpr */
   bld.vop1(aco_opcode::v_mov_b32, Definition(shared_vgpr_lo, v1), input_data);
   /* LO: bpermute shared vgpr (high lanes' data) */
   bld.ds(aco_opcode::ds_bpermute_b32, Definition(shared_vgpr_hi, v1), index_x4,
          Operand(shared_vgpr_hi, v1));
   /* Set EXEC to enable HI lanes only */
   bld.sop2(aco_opcode::s_bfm_b64, Definition(exec, s2), Operand::c32(32u), Operand::c32(32u));
   /* HI: bpermute shared vgpr (low lanes' data) */
   bld.ds(aco_opcode::ds_bpermute_b32, Definition(shared_vgpr_lo, v1), index_x4,
          Operand(shared_vgpr_lo, v1));

   /* Only enable lanes which use the other half's data */
   bld.sop2(aco_opcode::s_andn2_b64, Definition(exec, s2), clobber_scc,
            Operand(tmp_exec.physReg(), s2), same_half);
   /* LO: Copy shared vgpr (high lanes' bpermuted data) to output vgpr */
   bld.vop1_dpp(aco_opcode::v_mov_b32, dst, Operand(shared_vgpr_hi, v1), dpp_quad_perm(0, 1, 2, 3),
                0x3, 0xf, false);
   /* HI: Copy shared vgpr (low lanes' bpermuted data) to output vgpr */
   bld.vop1_dpp(aco_opcode::v_mov_b32, dst, Operand(shared_vgpr_lo, v1), dpp_quad_perm(0, 1, 2, 3),
                0xc, 0xf, false);

   /* Restore saved EXEC */
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand(tmp_exec.physReg(), s2));

   adjust_bpermute_dst(bld, dst, input_data);
}

void
emit_bpermute_readlane(Program* program, aco_ptr<Instruction>& instr, Builder& bld)
{
   /* Emulates bpermute using readlane instructions */

   Operand index = instr->operands[0];
   Operand input = instr->operands[1];
   Definition dst = instr->definitions[0];
   Definition temp_exec = instr->definitions[1];
   Definition clobber_vcc = instr->definitions[2];

   assert(dst.regClass() == v1);
   assert(temp_exec.regClass() == bld.lm);
   assert(clobber_vcc.regClass() == bld.lm);
   assert(clobber_vcc.physReg() == vcc);
   assert(index.regClass() == v1);
   assert(index.physReg() != dst.physReg());
   assert(input.regClass().type() == RegType::vgpr);
   assert(input.bytes() <= 4);
   assert(input.physReg() != dst.physReg());

   /* Save original EXEC */
   bld.sop1(Builder::s_mov, temp_exec, Operand(exec, bld.lm));

   /* An "unrolled loop" that is executed per each lane.
    * This takes only a few instructions per lane, as opposed to a "real" loop
    * with branching, where the branch instruction alone would take 16+ cycles.
    */
   for (unsigned n = 0; n < program->wave_size; ++n) {
      /* Activate the lane which has N for its source index */
      if (program->gfx_level >= GFX10)
         bld.vopc(aco_opcode::v_cmpx_eq_u32, Definition(exec, bld.lm), Operand::c32(n), index);
      else
         bld.vopc(aco_opcode::v_cmpx_eq_u32, clobber_vcc, Definition(exec, bld.lm), Operand::c32(n),
                  index);
      /* Read the data from lane N */
      bld.readlane(Definition(vcc, s1), input, Operand::c32(n));
      /* On the active lane, move the data we read from lane N to the destination VGPR */
      bld.vop1(aco_opcode::v_mov_b32, dst, Operand(vcc, s1));
      /* Restore original EXEC */
      bld.sop1(Builder::s_mov, Definition(exec, bld.lm), Operand(temp_exec.physReg(), bld.lm));
   }

   adjust_bpermute_dst(bld, dst, input);
}

struct copy_operation {
   Operand op;
   Definition def;
   unsigned bytes;
   union {
      uint8_t uses[8];
      uint64_t is_used = 0;
   };
};

void
split_copy(lower_context* ctx, unsigned offset, Definition* def, Operand* op,
           const copy_operation& src, bool ignore_uses, unsigned max_size)
{
   PhysReg def_reg = src.def.physReg();
   PhysReg op_reg = src.op.physReg();
   def_reg.reg_b += offset;
   op_reg.reg_b += offset;

   /* 64-bit VGPR copies (implemented with v_lshrrev_b64) are slow before GFX10, and on GFX11
    * v_lshrrev_b64 doesn't get dual issued. */
   if ((ctx->program->gfx_level < GFX10 || ctx->program->gfx_level >= GFX11) &&
       src.def.regClass().type() == RegType::vgpr)
      max_size = MIN2(max_size, 4);
   unsigned max_align = src.def.regClass().type() == RegType::vgpr ? 4 : 16;

   /* make sure the size is a power of two and reg % bytes == 0 */
   unsigned bytes = 1;
   for (; bytes <= max_size; bytes *= 2) {
      unsigned next = bytes * 2u;
      bool can_increase = def_reg.reg_b % MIN2(next, max_align) == 0 &&
                          offset + next <= src.bytes && next <= max_size;
      if (!src.op.isConstant() && can_increase)
         can_increase = op_reg.reg_b % MIN2(next, max_align) == 0;
      for (unsigned i = 0; !ignore_uses && can_increase && (i < bytes); i++)
         can_increase = (src.uses[offset + bytes + i] == 0) == (src.uses[offset] == 0);
      if (!can_increase)
         break;
   }

   *def = Definition(src.def.tempId(), def_reg, src.def.regClass().resize(bytes));
   if (src.op.isConstant()) {
      assert(bytes >= 1 && bytes <= 8);
      uint64_t val = src.op.constantValue64() >> (offset * 8u);
      *op = Operand::get_const(ctx->program->gfx_level, val, bytes);
   } else {
      RegClass op_cls = src.op.regClass().resize(bytes);
      *op = Operand(op_reg, op_cls);
      op->setTemp(Temp(src.op.tempId(), op_cls));
   }
}

uint32_t
get_intersection_mask(int a_start, int a_size, int b_start, int b_size)
{
   int intersection_start = MAX2(b_start - a_start, 0);
   int intersection_end = MAX2(b_start + b_size - a_start, 0);
   if (intersection_start >= a_size || intersection_end == 0)
      return 0;

   uint32_t mask = u_bit_consecutive(0, a_size);
   return u_bit_consecutive(intersection_start, intersection_end - intersection_start) & mask;
}

/* src1 are bytes 0-3. dst/src0 are bytes 4-7. */
void
create_bperm(Builder& bld, uint8_t swiz[4], Definition dst, Operand src1,
             Operand src0 = Operand(v1))
{
   uint32_t swiz_packed =
      swiz[0] | ((uint32_t)swiz[1] << 8) | ((uint32_t)swiz[2] << 16) | ((uint32_t)swiz[3] << 24);

   dst = Definition(PhysReg(dst.physReg().reg()), v1);
   if (!src1.isConstant())
      src1 = Operand(PhysReg(src1.physReg().reg()), v1);
   if (src0.isUndefined())
      src0 = Operand(dst.physReg(), v1);
   else if (!src0.isConstant())
      src0 = Operand(PhysReg(src0.physReg().reg()), v1);
   bld.vop3(aco_opcode::v_perm_b32, dst, src0, src1, Operand::c32(swiz_packed));
}

void
emit_v_mov_b16(Builder& bld, Definition dst, Operand op)
{
   /* v_mov_b16 uses 32bit inline constants. */
   if (op.isConstant()) {
      if (!op.isLiteral() && op.physReg() >= 240) {
         /* v_add_f16 is smaller because it can use 16bit fp inline constants. */
         Instruction* instr = bld.vop2_e64(aco_opcode::v_add_f16, dst, op, Operand::zero());
         instr->valu().opsel[3] = dst.physReg().byte() == 2;
         return;
      }
      op = Operand::c32((int32_t)(int16_t)op.constantValue());
   }

   Instruction* instr = bld.vop1(aco_opcode::v_mov_b16, dst, op);
   instr->valu().opsel[0] = op.physReg().byte() == 2;
   instr->valu().opsel[3] = dst.physReg().byte() == 2;
}

void
copy_constant(lower_context* ctx, Builder& bld, Definition dst, Operand op)
{
   assert(op.bytes() == dst.bytes());

   if (dst.bytes() == 4 && op.isLiteral()) {
      uint32_t imm = op.constantValue();
      if (dst.regClass() == s1 && (imm >= 0xffff8000 || imm <= 0x7fff)) {
         bld.sopk(aco_opcode::s_movk_i32, dst, imm & 0xFFFFu);
         return;
      } else if (util_bitreverse(imm) <= 64 || util_bitreverse(imm) >= 0xFFFFFFF0) {
         uint32_t rev = util_bitreverse(imm);
         if (dst.regClass() == s1)
            bld.sop1(aco_opcode::s_brev_b32, dst, Operand::c32(rev));
         else
            bld.vop1(aco_opcode::v_bfrev_b32, dst, Operand::c32(rev));
         return;
      } else if (dst.regClass() == s1) {
         unsigned start = (ffs(imm) - 1) & 0x1f;
         unsigned size = util_bitcount(imm) & 0x1f;
         if (BITFIELD_RANGE(start, size) == imm) {
            bld.sop2(aco_opcode::s_bfm_b32, dst, Operand::c32(size), Operand::c32(start));
            return;
         }
         if (ctx->program->gfx_level >= GFX9) {
            Operand op_lo = Operand::c32(int32_t(int16_t(imm)));
            Operand op_hi = Operand::c32(int32_t(int16_t(imm >> 16)));
            if (!op_lo.isLiteral() && !op_hi.isLiteral()) {
               bld.sop2(aco_opcode::s_pack_ll_b32_b16, dst, op_lo, op_hi);
               return;
            }
         }
      }
   }

   if (op.bytes() == 4 && op.constantEquals(0x3e22f983) && ctx->program->gfx_level >= GFX8)
      op.setFixed(PhysReg{248}); /* it can be an inline constant on GFX8+ */

   if (dst.regClass() == s1) {
      bld.sop1(aco_opcode::s_mov_b32, dst, op);
   } else if (dst.regClass() == s2) {
      /* s_ashr_i64 writes SCC, so we can't use it */
      assert(Operand::is_constant_representable(op.constantValue64(), 8, true, false));
      uint64_t imm = op.constantValue64();
      if (op.isLiteral()) {
         unsigned start = (ffsll(imm) - 1) & 0x3f;
         unsigned size = util_bitcount64(imm) & 0x3f;
         if (BITFIELD64_RANGE(start, size) == imm) {
            bld.sop2(aco_opcode::s_bfm_b64, dst, Operand::c32(size), Operand::c32(start));
            return;
         }
      }
      bld.sop1(aco_opcode::s_mov_b64, dst, op);
   } else if (dst.regClass() == v2) {
      if (Operand::is_constant_representable(op.constantValue64(), 8, true, false)) {
         bld.vop3(aco_opcode::v_lshrrev_b64, dst, Operand::zero(), op);
      } else {
         assert(Operand::is_constant_representable(op.constantValue64(), 8, false, true));
         bld.vop3(aco_opcode::v_ashrrev_i64, dst, Operand::zero(), op);
      }
   } else if (dst.regClass() == v1) {
      bld.vop1(aco_opcode::v_mov_b32, dst, op);
   } else {
      assert(dst.regClass() == v1b || dst.regClass() == v2b);

      bool use_sdwa = ctx->program->gfx_level >= GFX9 && ctx->program->gfx_level < GFX11;
      /* We need the v_perm_b32 (VOP3) to be able to take literals, and that's a GFX10+ feature. */
      bool can_use_perm = ctx->program->gfx_level >= GFX10 &&
                          (op.constantEquals(0) || op.constantEquals(0xff) ||
                           op.constantEquals(0xffff) || op.constantEquals(0xff00));
      if (dst.regClass() == v1b && use_sdwa) {
         uint8_t val = op.constantValue();
         Operand op32 = Operand::c32((uint32_t)val | (val & 0x80u ? 0xffffff00u : 0u));
         if (op32.isLiteral()) {
            uint32_t a = (uint32_t)int8_mul_table[val * 2];
            uint32_t b = (uint32_t)int8_mul_table[val * 2 + 1];
            bld.vop2_sdwa(aco_opcode::v_mul_u32_u24, dst,
                          Operand::c32(a | (a & 0x80u ? 0xffffff00u : 0x0u)),
                          Operand::c32(b | (b & 0x80u ? 0xffffff00u : 0x0u)));
         } else {
            bld.vop1_sdwa(aco_opcode::v_mov_b32, dst, op32);
         }
      } else if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX11) {
         emit_v_mov_b16(bld, dst, op);
      } else if (dst.regClass() == v2b && use_sdwa && !op.isLiteral()) {
         if (op.constantValue() >= 0xfff0 || op.constantValue() <= 64) {
            /* use v_mov_b32 to avoid possible issues with denormal flushing or
             * NaN. v_add_f16 is still needed for float constants. */
            uint32_t val32 = (int32_t)(int16_t)op.constantValue();
            bld.vop1_sdwa(aco_opcode::v_mov_b32, dst, Operand::c32(val32));
         } else {
            bld.vop2_sdwa(aco_opcode::v_add_f16, dst, op, Operand::zero());
         }
      } else if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10 &&
                 (ctx->block->fp_mode.denorm16_64 & fp_denorm_keep_in)) {
         if (dst.physReg().byte() == 2) {
            Operand def_lo(dst.physReg().advance(-2), v2b);
            Instruction* instr = bld.vop3(aco_opcode::v_pack_b32_f16, dst, def_lo, op);
            instr->valu().opsel = 0;
         } else {
            assert(dst.physReg().byte() == 0);
            Operand def_hi(dst.physReg().advance(2), v2b);
            Instruction* instr = bld.vop3(aco_opcode::v_pack_b32_f16, dst, op, def_hi);
            instr->valu().opsel = 2;
         }
      } else if (can_use_perm) {
         uint8_t swiz[] = {4, 5, 6, 7};
         swiz[dst.physReg().byte()] = op.constantValue() & 0xff ? bperm_255 : bperm_0;
         if (dst.bytes() == 2)
            swiz[dst.physReg().byte() + 1] = op.constantValue() >> 8 ? bperm_255 : bperm_0;
         create_bperm(bld, swiz, dst, Operand::zero());
      } else {
         uint32_t offset = dst.physReg().byte() * 8u;
         uint32_t mask = ((1u << (dst.bytes() * 8)) - 1) << offset;
         uint32_t val = (op.constantValue() << offset) & mask;
         dst = Definition(PhysReg(dst.physReg().reg()), v1);
         Operand def_op(dst.physReg(), v1);
         if (val != mask)
            bld.vop2(aco_opcode::v_and_b32, dst, Operand::c32(~mask), def_op);
         if (val != 0)
            bld.vop2(aco_opcode::v_or_b32, dst, Operand::c32(val), def_op);
      }
   }
}

void
copy_linear_vgpr(Builder& bld, Definition def, Operand op, bool preserve_scc, PhysReg scratch_sgpr)
{
   if (preserve_scc)
      bld.sop1(aco_opcode::s_mov_b32, Definition(scratch_sgpr, s1), Operand(scc, s1));

   for (unsigned i = 0; i < 2; i++) {
      if (def.size() == 2)
         bld.vop3(aco_opcode::v_lshrrev_b64, def, Operand::zero(), op);
      else
         bld.vop1(aco_opcode::v_mov_b32, def, op);

      bld.sop1(Builder::s_not, Definition(exec, bld.lm), Definition(scc, s1),
               Operand(exec, bld.lm));
   }

   if (preserve_scc)
      bld.sopc(aco_opcode::s_cmp_lg_i32, Definition(scc, s1), Operand(scratch_sgpr, s1),
               Operand::zero());
}

void
swap_linear_vgpr(Builder& bld, Definition def, Operand op, bool preserve_scc, PhysReg scratch_sgpr)
{
   if (preserve_scc)
      bld.sop1(aco_opcode::s_mov_b32, Definition(scratch_sgpr, s1), Operand(scc, s1));

   Operand def_as_op = Operand(def.physReg(), def.regClass());
   Definition op_as_def = Definition(op.physReg(), op.regClass());

   for (unsigned i = 0; i < 2; i++) {
      if (bld.program->gfx_level >= GFX9) {
         bld.vop1(aco_opcode::v_swap_b32, def, op_as_def, op, def_as_op);
      } else {
         bld.vop2(aco_opcode::v_xor_b32, op_as_def, op, def_as_op);
         bld.vop2(aco_opcode::v_xor_b32, def, op, def_as_op);
         bld.vop2(aco_opcode::v_xor_b32, op_as_def, op, def_as_op);
      }

      bld.sop1(Builder::s_not, Definition(exec, bld.lm), Definition(scc, s1),
               Operand(exec, bld.lm));
   }

   if (preserve_scc)
      bld.sopc(aco_opcode::s_cmp_lg_i32, Definition(scc, s1), Operand(scratch_sgpr, s1),
               Operand::zero());
}

void
addsub_subdword_gfx11(Builder& bld, Definition dst, Operand src0, Operand src1, bool sub)
{
   Instruction* instr =
      bld.vop3(sub ? aco_opcode::v_sub_u16_e64 : aco_opcode::v_add_u16_e64, dst, src0, src1).instr;
   if (src0.physReg().byte() == 2)
      instr->valu().opsel |= 0x1;
   if (src1.physReg().byte() == 2)
      instr->valu().opsel |= 0x2;
   if (dst.physReg().byte() == 2)
      instr->valu().opsel |= 0x8;
}

bool
do_copy(lower_context* ctx, Builder& bld, const copy_operation& copy, bool* preserve_scc,
        PhysReg scratch_sgpr)
{
   bool did_copy = false;
   for (unsigned offset = 0; offset < copy.bytes;) {
      if (copy.uses[offset]) {
         offset++;
         continue;
      }

      Definition def;
      Operand op;
      split_copy(ctx, offset, &def, &op, copy, false, 8);

      if (def.physReg() == scc) {
         bld.sopc(aco_opcode::s_cmp_lg_i32, def, op, Operand::zero());
         *preserve_scc = true;
      } else if (op.isConstant()) {
         copy_constant(ctx, bld, def, op);
      } else if (def.regClass().is_linear_vgpr()) {
         copy_linear_vgpr(bld, def, op, *preserve_scc, scratch_sgpr);
      } else if (def.regClass() == v1) {
         bld.vop1(aco_opcode::v_mov_b32, def, op);
      } else if (def.regClass() == v2) {
         bld.vop3(aco_opcode::v_lshrrev_b64, def, Operand::zero(), op);
      } else if (def.regClass() == s1) {
         bld.sop1(aco_opcode::s_mov_b32, def, op);
      } else if (def.regClass() == s2) {
         bld.sop1(aco_opcode::s_mov_b64, def, op);
      } else if (def.regClass().is_subdword() && ctx->program->gfx_level < GFX8) {
         if (op.physReg().byte()) {
            assert(def.physReg().byte() == 0);
            bld.vop2(aco_opcode::v_lshrrev_b32, def, Operand::c32(op.physReg().byte() * 8), op);
         } else if (def.physReg().byte()) {
            assert(op.physReg().byte() == 0);
            /* preserve the target's lower half */
            uint32_t bits = def.physReg().byte() * 8;
            PhysReg lo_reg = PhysReg(def.physReg().reg());
            Definition lo_half =
               Definition(lo_reg, RegClass::get(RegType::vgpr, def.physReg().byte()));
            Definition dst =
               Definition(lo_reg, RegClass::get(RegType::vgpr, lo_half.bytes() + op.bytes()));

            if (def.physReg().reg() == op.physReg().reg()) {
               bld.vop2(aco_opcode::v_and_b32, lo_half, Operand::c32((1 << bits) - 1u),
                        Operand(lo_reg, lo_half.regClass()));
               if (def.physReg().byte() == 1) {
                  bld.vop2(aco_opcode::v_mul_u32_u24, dst, Operand::c32((1 << bits) + 1u), op);
               } else if (def.physReg().byte() == 2) {
                  bld.vop2(aco_opcode::v_cvt_pk_u16_u32, dst, Operand(lo_reg, v2b), op);
               } else if (def.physReg().byte() == 3) {
                  bld.sop1(aco_opcode::s_mov_b32, Definition(scratch_sgpr, s1),
                           Operand::c32((1 << bits) + 1u));
                  bld.vop3(aco_opcode::v_mul_lo_u32, dst, Operand(scratch_sgpr, s1), op);
               }
            } else {
               lo_half.setFixed(lo_half.physReg().advance(4 - def.physReg().byte()));
               bld.vop2(aco_opcode::v_lshlrev_b32, lo_half, Operand::c32(32 - bits),
                        Operand(lo_reg, lo_half.regClass()));
               bld.vop3(aco_opcode::v_alignbyte_b32, dst, op,
                        Operand(lo_half.physReg(), lo_half.regClass()),
                        Operand::c32(4 - def.physReg().byte()));
            }
         } else {
            bld.vop1(aco_opcode::v_mov_b32, def, op);
         }
      } else if (def.regClass() == v1b && ctx->program->gfx_level >= GFX11) {
         uint8_t swiz[] = {4, 5, 6, 7};
         swiz[def.physReg().byte()] = op.physReg().byte();
         create_bperm(bld, swiz, def, op);
      } else if (def.regClass() == v2b && ctx->program->gfx_level >= GFX11) {
         emit_v_mov_b16(bld, def, op);
      } else if (def.regClass().is_subdword()) {
         bld.vop1_sdwa(aco_opcode::v_mov_b32, def, op);
      } else {
         unreachable("unsupported copy");
      }

      did_copy = true;
      offset += def.bytes();
   }
   return did_copy;
}

void
swap_subdword_gfx11(Builder& bld, Definition def, Operand op)
{
   if (def.physReg().reg() == op.physReg().reg()) {
      assert(def.bytes() != 2); /* handled by caller */
      uint8_t swiz[] = {4, 5, 6, 7};
      std::swap(swiz[def.physReg().byte()], swiz[op.physReg().byte()]);
      create_bperm(bld, swiz, def, Operand::zero());
      return;
   }

   if (def.bytes() == 2) {
      Operand def_as_op = Operand(def.physReg(), def.regClass());
      Definition op_as_def = Definition(op.physReg(), op.regClass());
      addsub_subdword_gfx11(bld, def, def_as_op, op, false);
      addsub_subdword_gfx11(bld, op_as_def, def_as_op, op, true);
      addsub_subdword_gfx11(bld, def, def_as_op, op, true);
   } else {
      PhysReg op_half = op.physReg();
      op_half.reg_b &= ~1;

      PhysReg def_other_half = def.physReg();
      def_other_half.reg_b &= ~1;
      def_other_half.reg_b ^= 2;

      /* We can only swap individual bytes within a single VGPR, so temporarily move both bytes
       * into the same VGPR.
       */
      swap_subdword_gfx11(bld, Definition(def_other_half, v2b), Operand(op_half, v2b));
      swap_subdword_gfx11(bld, def, Operand(def_other_half.advance(op.physReg().byte() & 1), v1b));
      swap_subdword_gfx11(bld, Definition(def_other_half, v2b), Operand(op_half, v2b));
   }
}

void
do_swap(lower_context* ctx, Builder& bld, const copy_operation& copy, bool preserve_scc,
        Pseudo_instruction* pi)
{
   unsigned offset = 0;

   if (copy.bytes == 3 && (copy.def.physReg().reg_b % 4 <= 1) &&
       (copy.def.physReg().reg_b % 4) == (copy.op.physReg().reg_b % 4)) {
      /* instead of doing a 2-byte and 1-byte swap, do a 4-byte swap and then fixup with a 1-byte
       * swap */
      PhysReg op = copy.op.physReg();
      PhysReg def = copy.def.physReg();
      op.reg_b &= ~0x3;
      def.reg_b &= ~0x3;

      copy_operation tmp;
      tmp.op = Operand(op, v1);
      tmp.def = Definition(def, v1);
      tmp.bytes = 4;
      memset(tmp.uses, 1, 4);
      do_swap(ctx, bld, tmp, preserve_scc, pi);

      op.reg_b += copy.def.physReg().reg_b % 4 == 0 ? 3 : 0;
      def.reg_b += copy.def.physReg().reg_b % 4 == 0 ? 3 : 0;
      tmp.op = Operand(op, v1b);
      tmp.def = Definition(def, v1b);
      tmp.bytes = 1;
      tmp.uses[0] = 1;
      do_swap(ctx, bld, tmp, preserve_scc, pi);

      offset = copy.bytes;
   }

   for (; offset < copy.bytes;) {
      Definition def;
      Operand op;
      unsigned max_size = copy.def.regClass().type() == RegType::vgpr ? 4 : 8;
      split_copy(ctx, offset, &def, &op, copy, true, max_size);

      assert(op.regClass() == def.regClass());
      Operand def_as_op = Operand(def.physReg(), def.regClass());
      Definition op_as_def = Definition(op.physReg(), op.regClass());
      if (def.regClass().is_linear_vgpr()) {
         swap_linear_vgpr(bld, def, op, preserve_scc, pi->scratch_sgpr);
      } else if (ctx->program->gfx_level >= GFX9 && def.regClass() == v1) {
         bld.vop1(aco_opcode::v_swap_b32, def, op_as_def, op, def_as_op);
      } else if (def.regClass() == v1) {
         assert(def.physReg().byte() == 0 && op.physReg().byte() == 0);
         bld.vop2(aco_opcode::v_xor_b32, op_as_def, op, def_as_op);
         bld.vop2(aco_opcode::v_xor_b32, def, op, def_as_op);
         bld.vop2(aco_opcode::v_xor_b32, op_as_def, op, def_as_op);
      } else if (op.physReg() == scc || def.physReg() == scc) {
         /* we need to swap scc and another sgpr */
         assert(!preserve_scc);

         PhysReg other = op.physReg() == scc ? def.physReg() : op.physReg();

         bld.sop1(aco_opcode::s_mov_b32, Definition(pi->scratch_sgpr, s1), Operand(scc, s1));
         bld.sopc(aco_opcode::s_cmp_lg_i32, Definition(scc, s1), Operand(other, s1),
                  Operand::zero());
         bld.sop1(aco_opcode::s_mov_b32, Definition(other, s1), Operand(pi->scratch_sgpr, s1));
      } else if (def.regClass() == s1) {
         if (preserve_scc) {
            bld.sop1(aco_opcode::s_mov_b32, Definition(pi->scratch_sgpr, s1), op);
            bld.sop1(aco_opcode::s_mov_b32, op_as_def, def_as_op);
            bld.sop1(aco_opcode::s_mov_b32, def, Operand(pi->scratch_sgpr, s1));
         } else {
            bld.sop2(aco_opcode::s_xor_b32, op_as_def, Definition(scc, s1), op, def_as_op);
            bld.sop2(aco_opcode::s_xor_b32, def, Definition(scc, s1), op, def_as_op);
            bld.sop2(aco_opcode::s_xor_b32, op_as_def, Definition(scc, s1), op, def_as_op);
         }
      } else if (def.regClass() == s2) {
         if (preserve_scc)
            bld.sop1(aco_opcode::s_mov_b32, Definition(pi->scratch_sgpr, s1), Operand(scc, s1));
         bld.sop2(aco_opcode::s_xor_b64, op_as_def, Definition(scc, s1), op, def_as_op);
         bld.sop2(aco_opcode::s_xor_b64, def, Definition(scc, s1), op, def_as_op);
         bld.sop2(aco_opcode::s_xor_b64, op_as_def, Definition(scc, s1), op, def_as_op);
         if (preserve_scc)
            bld.sopc(aco_opcode::s_cmp_lg_i32, Definition(scc, s1), Operand(pi->scratch_sgpr, s1),
                     Operand::zero());
      } else if (def.bytes() == 2 && def.physReg().reg() == op.physReg().reg()) {
         bld.vop3(aco_opcode::v_alignbyte_b32, Definition(def.physReg(), v1), def_as_op, op,
                  Operand::c32(2u));
      } else {
         assert(def.regClass().is_subdword());
         if (ctx->program->gfx_level >= GFX11) {
            swap_subdword_gfx11(bld, def, op);
         } else {
            bld.vop2_sdwa(aco_opcode::v_xor_b32, op_as_def, op, def_as_op);
            bld.vop2_sdwa(aco_opcode::v_xor_b32, def, op, def_as_op);
            bld.vop2_sdwa(aco_opcode::v_xor_b32, op_as_def, op, def_as_op);
         }
      }

      offset += def.bytes();
   }

   if (ctx->program->gfx_level <= GFX7)
      return;

   /* fixup in case we swapped bytes we shouldn't have */
   copy_operation tmp_copy = copy;
   tmp_copy.op.setFixed(copy.def.physReg());
   tmp_copy.def.setFixed(copy.op.physReg());
   do_copy(ctx, bld, tmp_copy, &preserve_scc, pi->scratch_sgpr);
}

void
do_pack_2x16(lower_context* ctx, Builder& bld, Definition def, Operand lo, Operand hi)
{
   if (lo.isConstant() && hi.isConstant()) {
      copy_constant(ctx, bld, def, Operand::c32(lo.constantValue() | (hi.constantValue() << 16)));
      return;
   }

   bool can_use_pack = (ctx->block->fp_mode.denorm16_64 & fp_denorm_keep_in) &&
                       (ctx->program->gfx_level >= GFX10 ||
                        (ctx->program->gfx_level >= GFX9 && !lo.isLiteral() && !hi.isLiteral()));

   if (can_use_pack) {
      Instruction* instr = bld.vop3(aco_opcode::v_pack_b32_f16, def, lo, hi);
      /* opsel: 0 = select low half, 1 = select high half. [0] = src0, [1] = src1 */
      instr->valu().opsel = hi.physReg().byte() | (lo.physReg().byte() >> 1);
      return;
   }

   /* a single alignbyte can be sufficient: hi can be a 32-bit integer constant */
   if (lo.physReg().byte() == 2 && hi.physReg().byte() == 0 &&
       (!hi.isConstant() || (hi.constantValue() && (!Operand::c32(hi.constantValue()).isLiteral() ||
                                                    ctx->program->gfx_level >= GFX10)))) {
      if (hi.isConstant())
         bld.vop3(aco_opcode::v_alignbyte_b32, def, Operand::c32(hi.constantValue()), lo,
                  Operand::c32(2u));
      else
         bld.vop3(aco_opcode::v_alignbyte_b32, def, hi, lo, Operand::c32(2u));
      return;
   }

   Definition def_lo = Definition(def.physReg(), v2b);
   Definition def_hi = Definition(def.physReg().advance(2), v2b);

   if (lo.isConstant()) {
      /* move hi and zero low bits */
      if (hi.physReg().byte() == 0)
         bld.vop2(aco_opcode::v_lshlrev_b32, def_hi, Operand::c32(16u), hi);
      else
         bld.vop2(aco_opcode::v_and_b32, def_hi, Operand::c32(~0xFFFFu), hi);
      if (lo.constantValue())
         bld.vop2(aco_opcode::v_or_b32, def, Operand::c32(lo.constantValue()),
                  Operand(def.physReg(), v1));
      return;
   }
   if (hi.isConstant()) {
      /* move lo and zero high bits */
      if (lo.physReg().byte() == 2)
         bld.vop2(aco_opcode::v_lshrrev_b32, def_lo, Operand::c32(16u), lo);
      else if (ctx->program->gfx_level >= GFX11)
         bld.vop1(aco_opcode::v_cvt_u32_u16, def, lo);
      else
         bld.vop2(aco_opcode::v_and_b32, def_lo, Operand::c32(0xFFFFu), lo);
      if (hi.constantValue())
         bld.vop2(aco_opcode::v_or_b32, def, Operand::c32(hi.constantValue() << 16u),
                  Operand(def.physReg(), v1));
      return;
   }

   if (lo.physReg().reg() == def.physReg().reg()) {
      /* lo is in the high bits of def */
      assert(lo.physReg().byte() == 2);
      bld.vop2(aco_opcode::v_lshrrev_b32, def_lo, Operand::c32(16u), lo);
      lo.setFixed(def.physReg());
   } else if (hi.physReg() == def.physReg()) {
      /* hi is in the low bits of def */
      assert(hi.physReg().byte() == 0);
      bld.vop2(aco_opcode::v_lshlrev_b32, def_hi, Operand::c32(16u), hi);
      hi.setFixed(def.physReg().advance(2));
   } else if (ctx->program->gfx_level >= GFX8) {
      /* Either lo or hi can be placed with just a v_mov. SDWA is not needed, because
       * op.physReg().byte()==def.physReg().byte() and the other half will be overwritten.
       */
      assert(lo.physReg().byte() == 0 || hi.physReg().byte() == 2);
      Operand& op = lo.physReg().byte() == 0 ? lo : hi;
      PhysReg reg = def.physReg().advance(op.physReg().byte());
      bld.vop1(aco_opcode::v_mov_b32, Definition(reg, v2b), op);
      op.setFixed(reg);
   }

   /* either hi or lo are already placed correctly */
   if (ctx->program->gfx_level >= GFX11) {
      if (lo.physReg().reg() == def.physReg().reg())
         emit_v_mov_b16(bld, def_hi, hi);
      else
         emit_v_mov_b16(bld, def_lo, lo);
      return;
   } else if (ctx->program->gfx_level >= GFX8) {
      if (lo.physReg().reg() == def.physReg().reg())
         bld.vop1_sdwa(aco_opcode::v_mov_b32, def_hi, hi);
      else
         bld.vop1_sdwa(aco_opcode::v_mov_b32, def_lo, lo);
      return;
   }

   /* alignbyte needs the operands in the following way:
    * | xx hi | lo xx | >> 2 byte */
   if (lo.physReg().byte() != hi.physReg().byte()) {
      /* | xx lo | hi xx | => | lo hi | lo hi | */
      assert(lo.physReg().byte() == 0 && hi.physReg().byte() == 2);
      bld.vop3(aco_opcode::v_alignbyte_b32, def, lo, hi, Operand::c32(2u));
      lo = Operand(def_hi.physReg(), v2b);
      hi = Operand(def_lo.physReg(), v2b);
   } else if (lo.physReg().byte() == 0) {
      /* | xx hi | xx lo | => | xx hi | lo 00 | */
      bld.vop2(aco_opcode::v_lshlrev_b32, def_hi, Operand::c32(16u), lo);
      lo = Operand(def_hi.physReg(), v2b);
   } else {
      /* | hi xx | lo xx | => | 00 hi | lo xx | */
      assert(hi.physReg().byte() == 2);
      bld.vop2(aco_opcode::v_lshrrev_b32, def_lo, Operand::c32(16u), hi);
      hi = Operand(def_lo.physReg(), v2b);
   }
   /* perform the alignbyte */
   bld.vop3(aco_opcode::v_alignbyte_b32, def, hi, lo, Operand::c32(2u));
}

void
try_coalesce_copies(lower_context* ctx, std::map<PhysReg, copy_operation>& copy_map,
                    copy_operation& copy)
{
   // TODO try more relaxed alignment for subdword copies
   unsigned next_def_align = util_next_power_of_two(copy.bytes + 1);
   unsigned next_op_align = next_def_align;
   if (copy.def.regClass().type() == RegType::vgpr)
      next_def_align = MIN2(next_def_align, 4);
   if (copy.op.regClass().type() == RegType::vgpr)
      next_op_align = MIN2(next_op_align, 4);

   if (copy.bytes >= 8 || copy.def.physReg().reg_b % next_def_align ||
       (!copy.op.isConstant() && copy.op.physReg().reg_b % next_op_align))
      return;

   auto other = copy_map.find(copy.def.physReg().advance(copy.bytes));
   if (other == copy_map.end() || copy.bytes + other->second.bytes > 8 ||
       copy.op.isConstant() != other->second.op.isConstant())
      return;

   if (other->second.def.regClass().is_linear_vgpr() != copy.def.regClass().is_linear_vgpr())
      return;

   /* don't create 64-bit copies before GFX10 */
   if (copy.bytes >= 4 && copy.def.regClass().type() == RegType::vgpr &&
       ctx->program->gfx_level < GFX10)
      return;

   unsigned new_size = copy.bytes + other->second.bytes;
   if (copy.op.isConstant()) {
      uint64_t val =
         copy.op.constantValue64() | (other->second.op.constantValue64() << (copy.bytes * 8u));
      if (!util_is_power_of_two_or_zero(new_size))
         return;
      if (!Operand::is_constant_representable(val, new_size, true,
                                              copy.def.regClass().type() == RegType::vgpr))
         return;
      copy.op = Operand::get_const(ctx->program->gfx_level, val, new_size);
   } else {
      if (other->second.op.physReg() != copy.op.physReg().advance(copy.bytes))
         return;
      copy.op = Operand(copy.op.physReg(), copy.op.regClass().resize(new_size));
   }

   copy.bytes = new_size;
   copy.def = Definition(copy.def.physReg(), copy.def.regClass().resize(copy.bytes));
   copy_map.erase(other);
}

void
handle_operands(std::map<PhysReg, copy_operation>& copy_map, lower_context* ctx,
                amd_gfx_level gfx_level, Pseudo_instruction* pi)
{
   Builder bld(ctx->program, &ctx->instructions);
   unsigned num_instructions_before = ctx->instructions.size();
   aco_ptr<Instruction> mov;
   bool writes_scc = false;

   /* count the number of uses for each dst reg */
   for (auto it = copy_map.begin(); it != copy_map.end();) {

      if (it->second.def.physReg() == scc)
         writes_scc = true;

      assert(!pi->tmp_in_scc || !(it->second.def.physReg() == pi->scratch_sgpr));

      /* if src and dst reg are the same, remove operation */
      if (it->first == it->second.op.physReg()) {
         it = copy_map.erase(it);
         continue;
      }

      /* split large copies */
      if (it->second.bytes > 8) {
         assert(!it->second.op.isConstant());
         assert(!it->second.def.regClass().is_subdword());
         RegClass rc = it->second.def.regClass().resize(it->second.def.bytes() - 8);
         Definition hi_def = Definition(PhysReg{it->first + 2}, rc);
         rc = it->second.op.regClass().resize(it->second.op.bytes() - 8);
         Operand hi_op = Operand(PhysReg{it->second.op.physReg() + 2}, rc);
         copy_operation copy = {hi_op, hi_def, it->second.bytes - 8};
         copy_map[hi_def.physReg()] = copy;
         assert(it->second.op.physReg().byte() == 0 && it->second.def.physReg().byte() == 0);
         it->second.op = Operand(it->second.op.physReg(), it->second.op.regClass().resize(8));
         it->second.def = Definition(it->second.def.physReg(), it->second.def.regClass().resize(8));
         it->second.bytes = 8;
      }

      try_coalesce_copies(ctx, copy_map, it->second);

      /* check if the definition reg is used by another copy operation */
      for (std::pair<const PhysReg, copy_operation>& copy : copy_map) {
         if (copy.second.op.isConstant())
            continue;
         for (uint16_t i = 0; i < it->second.bytes; i++) {
            /* distance might underflow */
            unsigned distance = it->first.reg_b + i - copy.second.op.physReg().reg_b;
            if (distance < copy.second.bytes)
               it->second.uses[i] += 1;
         }
      }

      ++it;
   }

   /* first, handle paths in the location transfer graph */
   bool preserve_scc = pi->tmp_in_scc && !writes_scc;
   bool skip_partial_copies = true;
   for (auto it = copy_map.begin();;) {
      if (copy_map.empty()) {
         ctx->program->statistics[aco_statistic_copies] +=
            ctx->instructions.size() - num_instructions_before;
         return;
      }
      if (it == copy_map.end()) {
         if (!skip_partial_copies)
            break;
         skip_partial_copies = false;
         it = copy_map.begin();
      }

      /* check if we can pack one register at once */
      if (it->first.byte() == 0 && it->second.bytes == 2) {
         PhysReg reg_hi = it->first.advance(2);
         std::map<PhysReg, copy_operation>::iterator other = copy_map.find(reg_hi);
         if (other != copy_map.end() && other->second.bytes == 2) {
            /* check if the target register is otherwise unused */
            bool unused_lo = !it->second.is_used || (it->second.is_used == 0x0101 &&
                                                     other->second.op.physReg() == it->first);
            bool unused_hi = !other->second.is_used ||
                             (other->second.is_used == 0x0101 && it->second.op.physReg() == reg_hi);
            if (unused_lo && unused_hi) {
               Operand lo = it->second.op;
               Operand hi = other->second.op;
               do_pack_2x16(ctx, bld, Definition(it->first, v1), lo, hi);
               copy_map.erase(it);
               copy_map.erase(other);

               for (std::pair<const PhysReg, copy_operation>& other2 : copy_map) {
                  for (uint16_t i = 0; i < other2.second.bytes; i++) {
                     /* distance might underflow */
                     unsigned distance_lo = other2.first.reg_b + i - lo.physReg().reg_b;
                     unsigned distance_hi = other2.first.reg_b + i - hi.physReg().reg_b;
                     if (distance_lo < 2 || distance_hi < 2)
                        other2.second.uses[i] -= 1;
                  }
               }
               it = copy_map.begin();
               continue;
            }
         }
      }

      /* on GFX6/7, we need some small workarounds as there is no
       * SDWA instruction to do partial register writes */
      if (ctx->program->gfx_level < GFX8 && it->second.bytes < 4) {
         if (it->first.byte() == 0 && it->second.op.physReg().byte() == 0 && !it->second.is_used &&
             pi->opcode == aco_opcode::p_split_vector) {
            /* Other operations might overwrite the high bits, so change all users
             * of the high bits to the new target where they are still available.
             * This mechanism depends on also emitting dead definitions. */
            PhysReg reg_hi = it->second.op.physReg().advance(it->second.bytes);
            while (reg_hi != PhysReg(it->second.op.physReg().reg() + 1)) {
               std::map<PhysReg, copy_operation>::iterator other = copy_map.begin();
               for (other = copy_map.begin(); other != copy_map.end(); other++) {
                  /* on GFX6/7, if the high bits are used as operand, they cannot be a target */
                  if (other->second.op.physReg() == reg_hi) {
                     other->second.op.setFixed(it->first.advance(reg_hi.byte()));
                     break; /* break because an operand can only be used once */
                  }
               }
               reg_hi = reg_hi.advance(it->second.bytes);
            }
         } else if (it->first.byte()) {
            assert(pi->opcode == aco_opcode::p_create_vector);
            /* on GFX6/7, if we target an upper half where the lower half hasn't yet been handled,
             * move to the target operand's high bits. This is save to do as it cannot be an operand
             */
            PhysReg lo = PhysReg(it->first.reg());
            std::map<PhysReg, copy_operation>::iterator other = copy_map.find(lo);
            if (other != copy_map.end()) {
               assert(other->second.bytes == it->first.byte());
               PhysReg new_reg_hi = other->second.op.physReg().advance(it->first.byte());
               it->second.def = Definition(new_reg_hi, it->second.def.regClass());
               it->second.is_used = 0;
               other->second.bytes += it->second.bytes;
               other->second.def.setTemp(Temp(other->second.def.tempId(),
                                              RegClass::get(RegType::vgpr, other->second.bytes)));
               other->second.op.setTemp(Temp(other->second.op.tempId(),
                                             RegClass::get(RegType::vgpr, other->second.bytes)));
               /* if the new target's high bits are also a target, change uses */
               std::map<PhysReg, copy_operation>::iterator target = copy_map.find(new_reg_hi);
               if (target != copy_map.end()) {
                  for (unsigned i = 0; i < it->second.bytes; i++)
                     target->second.uses[i]++;
               }
            }
         }
      }

      /* find portions where the target reg is not used as operand for any other copy */
      if (it->second.is_used) {
         if (it->second.op.isConstant() || skip_partial_copies) {
            /* we have to skip constants until is_used=0.
             * we also skip partial copies at the beginning to help coalescing */
            ++it;
            continue;
         }

         unsigned has_zero_use_bytes = 0;
         for (unsigned i = 0; i < it->second.bytes; i++)
            has_zero_use_bytes |= (it->second.uses[i] == 0) << i;

         if (has_zero_use_bytes) {
            /* Skipping partial copying and doing a v_swap_b32 and then fixup
             * copies is usually beneficial for sub-dword copies, but if doing
             * a partial copy allows further copies, it should be done instead. */
            bool partial_copy = (has_zero_use_bytes == 0xf) || (has_zero_use_bytes == 0xf0);
            for (std::pair<const PhysReg, copy_operation>& copy : copy_map) {
               /* on GFX6/7, we can only do copies with full registers */
               if (partial_copy || ctx->program->gfx_level <= GFX7)
                  break;
               for (uint16_t i = 0; i < copy.second.bytes; i++) {
                  /* distance might underflow */
                  unsigned distance = copy.first.reg_b + i - it->second.op.physReg().reg_b;
                  if (distance < it->second.bytes && copy.second.uses[i] == 1 &&
                      !it->second.uses[distance])
                     partial_copy = true;
               }
            }

            if (!partial_copy) {
               ++it;
               continue;
            }
         } else {
            /* full target reg is used: register swapping needed */
            ++it;
            continue;
         }
      }

      bool did_copy = do_copy(ctx, bld, it->second, &preserve_scc, pi->scratch_sgpr);
      skip_partial_copies = did_copy;
      std::pair<PhysReg, copy_operation> copy = *it;

      if (it->second.is_used == 0) {
         /* the target reg is not used as operand for any other copy, so we
          * copied to all of it */
         copy_map.erase(it);
         it = copy_map.begin();
      } else {
         /* we only performed some portions of this copy, so split it to only
          * leave the portions that still need to be done */
         copy_operation original = it->second; /* the map insertion below can overwrite this */
         copy_map.erase(it);
         for (unsigned offset = 0; offset < original.bytes;) {
            if (original.uses[offset] == 0) {
               offset++;
               continue;
            }
            Definition def;
            Operand op;
            split_copy(ctx, offset, &def, &op, original, false, 8);

            copy_operation new_copy = {op, def, def.bytes()};
            for (unsigned i = 0; i < new_copy.bytes; i++)
               new_copy.uses[i] = original.uses[i + offset];
            copy_map[def.physReg()] = new_copy;

            offset += def.bytes();
         }

         it = copy_map.begin();
      }

      /* Reduce the number of uses of the operand reg by one. Do this after
       * splitting the copy or removing it in case the copy writes to it's own
       * operand (for example, v[7:8] = v[8:9]) */
      if (did_copy && !copy.second.op.isConstant()) {
         for (std::pair<const PhysReg, copy_operation>& other : copy_map) {
            for (uint16_t i = 0; i < other.second.bytes; i++) {
               /* distance might underflow */
               unsigned distance = other.first.reg_b + i - copy.second.op.physReg().reg_b;
               if (distance < copy.second.bytes && !copy.second.uses[distance])
                  other.second.uses[i] -= 1;
            }
         }
      }
   }

   /* all target regs are needed as operand somewhere which means, all entries are part of a cycle */
   unsigned largest = 0;
   for (const std::pair<const PhysReg, copy_operation>& op : copy_map)
      largest = MAX2(largest, op.second.bytes);

   while (!copy_map.empty()) {

      /* Perform larger swaps first, because larger swaps swaps can make other
       * swaps unnecessary. */
      auto it = copy_map.begin();
      for (auto it2 = copy_map.begin(); it2 != copy_map.end(); ++it2) {
         if (it2->second.bytes > it->second.bytes) {
            it = it2;
            if (it->second.bytes == largest)
               break;
         }
      }

      /* should already be done */
      assert(!it->second.op.isConstant());

      assert(it->second.op.isFixed());
      assert(it->second.def.regClass() == it->second.op.regClass());

      if (it->first == it->second.op.physReg()) {
         copy_map.erase(it);
         continue;
      }

      if (preserve_scc && it->second.def.getTemp().type() == RegType::sgpr)
         assert(!(it->second.def.physReg() == pi->scratch_sgpr));

      /* to resolve the cycle, we have to swap the src reg with the dst reg */
      copy_operation swap = it->second;

      /* if this is self-intersecting, we have to split it because
       * self-intersecting swaps don't make sense */
      PhysReg src = swap.op.physReg(), dst = swap.def.physReg();
      if (abs((int)src.reg_b - (int)dst.reg_b) < (int)swap.bytes) {
         unsigned offset = abs((int)src.reg_b - (int)dst.reg_b);

         copy_operation remaining;
         src.reg_b += offset;
         dst.reg_b += offset;
         remaining.bytes = swap.bytes - offset;
         memcpy(remaining.uses, swap.uses + offset, remaining.bytes);
         remaining.op = Operand(src, swap.def.regClass().resize(remaining.bytes));
         remaining.def = Definition(dst, swap.def.regClass().resize(remaining.bytes));
         copy_map[dst] = remaining;

         memset(swap.uses + offset, 0, swap.bytes - offset);
         swap.bytes = offset;
      }

      /* GFX6-7 can only swap full registers */
      if (ctx->program->gfx_level <= GFX7)
         swap.bytes = align(swap.bytes, 4);

      do_swap(ctx, bld, swap, preserve_scc, pi);

      /* remove from map */
      copy_map.erase(it);

      /* change the operand reg of the target's uses and split uses if needed */
      uint32_t bytes_left = u_bit_consecutive(0, swap.bytes);
      for (auto target = copy_map.begin(); target != copy_map.end(); ++target) {
         if (target->second.op.physReg() == swap.def.physReg() &&
             swap.bytes == target->second.bytes) {
            target->second.op.setFixed(swap.op.physReg());
            break;
         }

         uint32_t imask =
            get_intersection_mask(swap.def.physReg().reg_b, swap.bytes,
                                  target->second.op.physReg().reg_b, target->second.bytes);

         if (!imask)
            continue;

         int offset = (int)target->second.op.physReg().reg_b - (int)swap.def.physReg().reg_b;

         /* split and update the middle (the portion that reads the swap's
          * definition) to read the swap's operand instead */
         int target_op_end = target->second.op.physReg().reg_b + target->second.bytes;
         int swap_def_end = swap.def.physReg().reg_b + swap.bytes;
         int before_bytes = MAX2(-offset, 0);
         int after_bytes = MAX2(target_op_end - swap_def_end, 0);
         int middle_bytes = target->second.bytes - before_bytes - after_bytes;

         if (after_bytes) {
            unsigned after_offset = before_bytes + middle_bytes;
            assert(after_offset > 0);
            copy_operation copy;
            copy.bytes = after_bytes;
            memcpy(copy.uses, target->second.uses + after_offset, copy.bytes);
            RegClass rc = target->second.op.regClass().resize(after_bytes);
            copy.op = Operand(target->second.op.physReg().advance(after_offset), rc);
            copy.def = Definition(target->second.def.physReg().advance(after_offset), rc);
            copy_map[copy.def.physReg()] = copy;
         }

         if (middle_bytes) {
            copy_operation copy;
            copy.bytes = middle_bytes;
            memcpy(copy.uses, target->second.uses + before_bytes, copy.bytes);
            RegClass rc = target->second.op.regClass().resize(middle_bytes);
            copy.op = Operand(swap.op.physReg().advance(MAX2(offset, 0)), rc);
            copy.def = Definition(target->second.def.physReg().advance(before_bytes), rc);
            copy_map[copy.def.physReg()] = copy;
         }

         if (before_bytes) {
            copy_operation copy;
            target->second.bytes = before_bytes;
            RegClass rc = target->second.op.regClass().resize(before_bytes);
            target->second.op = Operand(target->second.op.physReg(), rc);
            target->second.def = Definition(target->second.def.physReg(), rc);
            memset(target->second.uses + target->second.bytes, 0, 8 - target->second.bytes);
         }

         /* break early since we know each byte of the swap's definition is used
          * at most once */
         bytes_left &= ~imask;
         if (!bytes_left)
            break;
      }
   }
   ctx->program->statistics[aco_statistic_copies] +=
      ctx->instructions.size() - num_instructions_before;
}

void
emit_set_mode(Builder& bld, float_mode new_mode, bool set_round, bool set_denorm)
{
   if (bld.program->gfx_level >= GFX10) {
      if (set_round)
         bld.sopp(aco_opcode::s_round_mode, -1, new_mode.round);
      if (set_denorm)
         bld.sopp(aco_opcode::s_denorm_mode, -1, new_mode.denorm);
   } else if (set_round || set_denorm) {
      /* "((size - 1) << 11) | register" (MODE is encoded as register 1) */
      bld.sopk(aco_opcode::s_setreg_imm32_b32, Operand::literal32(new_mode.val), (7 << 11) | 1);
   }
}

void
emit_set_mode_from_block(Builder& bld, Program& program, Block* block, bool always_set)
{
   float_mode config_mode;
   config_mode.val = program.config->float_mode;

   bool set_round = always_set && block->fp_mode.round != config_mode.round;
   bool set_denorm = always_set && block->fp_mode.denorm != config_mode.denorm;
   if (block->kind & block_kind_top_level) {
      for (unsigned pred : block->linear_preds) {
         if (program.blocks[pred].fp_mode.round != block->fp_mode.round)
            set_round = true;
         if (program.blocks[pred].fp_mode.denorm != block->fp_mode.denorm)
            set_denorm = true;
      }
   }
   /* only allow changing modes at top-level blocks so this doesn't break
    * the "jump over empty blocks" optimization */
   assert((!set_round && !set_denorm) || (block->kind & block_kind_top_level));
   emit_set_mode(bld, block->fp_mode, set_round, set_denorm);
}

void
hw_init_scratch(Builder& bld, Definition def, Operand scratch_addr, Operand scratch_offset)
{
   /* Since we know what the high 16 bits of scratch_hi is, we can set all the high 16
    * bits in the same instruction that we add the carry.
    */
   Operand hi_add = Operand::c32(0xffff0000 - S_008F04_SWIZZLE_ENABLE_GFX6(1));
   Operand scratch_addr_lo(scratch_addr.physReg(), s1);
   Operand scratch_addr_hi(scratch_addr_lo.physReg().advance(4), s1);

   if (bld.program->gfx_level >= GFX10) {
      PhysReg scratch_lo = def.physReg();
      PhysReg scratch_hi = def.physReg().advance(4);

      bld.sop2(aco_opcode::s_add_u32, Definition(scratch_lo, s1), Definition(scc, s1),
               scratch_addr_lo, scratch_offset);
      bld.sop2(aco_opcode::s_addc_u32, Definition(scratch_hi, s1), Definition(scc, s1),
               scratch_addr_hi, hi_add, Operand(scc, s1));

      /* "((size - 1) << 11) | register" (FLAT_SCRATCH_LO/HI is encoded as register
       * 20/21) */
      bld.sopk(aco_opcode::s_setreg_b32, Operand(scratch_lo, s1), (31 << 11) | 20);
      bld.sopk(aco_opcode::s_setreg_b32, Operand(scratch_hi, s1), (31 << 11) | 21);
   } else {
      bld.sop2(aco_opcode::s_add_u32, Definition(flat_scr_lo, s1), Definition(scc, s1),
               scratch_addr_lo, scratch_offset);
      bld.sop2(aco_opcode::s_addc_u32, Definition(flat_scr_hi, s1), Definition(scc, s1),
               scratch_addr_hi, hi_add, Operand(scc, s1));
   }
}

void
lower_image_sample(lower_context* ctx, aco_ptr<Instruction>& instr)
{
   Operand linear_vgpr = instr->operands[3];

   unsigned nsa_size = ctx->program->dev.max_nsa_vgprs;
   unsigned vaddr_size = linear_vgpr.size();
   unsigned num_copied_vgprs = instr->operands.size() - 4;
   nsa_size = num_copied_vgprs > 0 && (ctx->program->gfx_level >= GFX11 || vaddr_size <= nsa_size)
                 ? nsa_size
                 : 0;

   Operand vaddr[16];
   unsigned num_vaddr = 0;

   if (nsa_size) {
      assert(num_copied_vgprs <= nsa_size);
      for (unsigned i = 0; i < num_copied_vgprs; i++)
         vaddr[num_vaddr++] = instr->operands[4 + i];
      for (unsigned i = num_copied_vgprs; i < std::min(vaddr_size, nsa_size); i++)
         vaddr[num_vaddr++] = Operand(linear_vgpr.physReg().advance(i * 4), v1);
      if (vaddr_size > nsa_size) {
         RegClass rc = RegClass::get(RegType::vgpr, (vaddr_size - nsa_size) * 4);
         vaddr[num_vaddr++] = Operand(PhysReg(linear_vgpr.physReg().advance(nsa_size * 4)), rc);
      }
   } else {
      PhysReg reg = linear_vgpr.physReg();
      std::map<PhysReg, copy_operation> copy_operations;
      for (unsigned i = 4; i < instr->operands.size(); i++) {
         Operand arg = instr->operands[i];
         Definition def(reg, RegClass::get(RegType::vgpr, arg.bytes()));
         copy_operations[def.physReg()] = {arg, def, def.bytes()};
         reg = reg.advance(arg.bytes());
      }
      vaddr[num_vaddr++] = linear_vgpr;

      Pseudo_instruction pi = {};
      handle_operands(copy_operations, ctx, ctx->program->gfx_level, &pi);
   }

   instr->mimg().strict_wqm = false;

   if ((3 + num_vaddr) > instr->operands.size()) {
      MIMG_instruction* new_instr = create_instruction<MIMG_instruction>(
         instr->opcode, Format::MIMG, 3 + num_vaddr, instr->definitions.size());
      std::copy(instr->definitions.cbegin(), instr->definitions.cend(),
                new_instr->definitions.begin());
      new_instr->operands[0] = instr->operands[0];
      new_instr->operands[1] = instr->operands[1];
      new_instr->operands[2] = instr->operands[2];
      memcpy((uint8_t*)new_instr + sizeof(Instruction), (uint8_t*)instr.get() + sizeof(Instruction),
             sizeof(MIMG_instruction) - sizeof(Instruction));
      instr.reset(new_instr);
   } else {
      while (instr->operands.size() > (3 + num_vaddr))
         instr->operands.pop_back();
   }
   std::copy(vaddr, vaddr + num_vaddr, std::next(instr->operands.begin(), 3));
}

void
lower_to_hw_instr(Program* program)
{
   gfx9_pops_done_msg_bounds pops_done_msg_bounds;
   if (program->has_pops_overlapped_waves_wait && program->gfx_level < GFX11) {
      pops_done_msg_bounds = gfx9_pops_done_msg_bounds(program);
   }

   Block* discard_exit_block = NULL;
   Block* discard_pops_done_and_exit_block = NULL;

   int end_with_regs_block_index = -1;

   bool should_dealloc_vgprs = dealloc_vgprs(program);

   for (int block_idx = program->blocks.size() - 1; block_idx >= 0; block_idx--) {
      Block* block = &program->blocks[block_idx];
      lower_context ctx;
      ctx.program = program;
      ctx.block = block;
      ctx.instructions.reserve(block->instructions.size());
      Builder bld(program, &ctx.instructions);

      emit_set_mode_from_block(bld, *program, block, (block_idx == 0));

      for (size_t instr_idx = 0; instr_idx < block->instructions.size(); instr_idx++) {
         aco_ptr<Instruction>& instr = block->instructions[instr_idx];

         /* Send the ordered section done message from the middle of the block if needed (if the
          * ordered section is ended by an instruction inside this block).
          * Also make sure the done message is sent if it's needed in case early exit happens for
          * any reason.
          */
         if ((block_idx == pops_done_msg_bounds.end_block_idx() &&
              instr_idx == pops_done_msg_bounds.instr_after_end_idx()) ||
             (instr->opcode == aco_opcode::s_endpgm &&
              pops_done_msg_bounds.early_exit_needs_done_msg(block_idx, instr_idx))) {
            bld.sopp(aco_opcode::s_sendmsg, -1, sendmsg_ordered_ps_done);
         }

         aco_ptr<Instruction> mov;
         if (instr->isPseudo() && instr->opcode != aco_opcode::p_unit_test) {
            Pseudo_instruction* pi = &instr->pseudo();

            switch (instr->opcode) {
            case aco_opcode::p_extract_vector: {
               PhysReg reg = instr->operands[0].physReg();
               Definition& def = instr->definitions[0];
               reg.reg_b += instr->operands[1].constantValue() * def.bytes();

               if (reg == def.physReg())
                  break;

               RegClass op_rc = def.regClass().is_subdword()
                                   ? def.regClass()
                                   : RegClass(instr->operands[0].getTemp().type(), def.size());
               std::map<PhysReg, copy_operation> copy_operations;
               copy_operations[def.physReg()] = {Operand(reg, op_rc), def, def.bytes()};
               handle_operands(copy_operations, &ctx, program->gfx_level, pi);
               break;
            }
            case aco_opcode::p_create_vector: {
               std::map<PhysReg, copy_operation> copy_operations;
               PhysReg reg = instr->definitions[0].physReg();

               for (const Operand& op : instr->operands) {
                  if (op.isConstant()) {
                     const Definition def = Definition(
                        reg, instr->definitions[0].getTemp().regClass().resize(op.bytes()));
                     copy_operations[reg] = {op, def, op.bytes()};
                     reg.reg_b += op.bytes();
                     continue;
                  }
                  if (op.isUndefined()) {
                     // TODO: coalesce subdword copies if dst byte is 0
                     reg.reg_b += op.bytes();
                     continue;
                  }

                  RegClass rc_def =
                     op.regClass().is_subdword()
                        ? op.regClass()
                        : instr->definitions[0].getTemp().regClass().resize(op.bytes());
                  const Definition def = Definition(reg, rc_def);
                  copy_operations[def.physReg()] = {op, def, op.bytes()};
                  reg.reg_b += op.bytes();
               }
               handle_operands(copy_operations, &ctx, program->gfx_level, pi);
               break;
            }
            case aco_opcode::p_split_vector: {
               std::map<PhysReg, copy_operation> copy_operations;
               PhysReg reg = instr->operands[0].physReg();

               for (const Definition& def : instr->definitions) {
                  RegClass rc_op = def.regClass().is_subdword()
                                      ? def.regClass()
                                      : instr->operands[0].getTemp().regClass().resize(def.bytes());
                  const Operand op = Operand(reg, rc_op);
                  copy_operations[def.physReg()] = {op, def, def.bytes()};
                  reg.reg_b += def.bytes();
               }
               handle_operands(copy_operations, &ctx, program->gfx_level, pi);
               break;
            }
            case aco_opcode::p_parallelcopy: {
               std::map<PhysReg, copy_operation> copy_operations;
               for (unsigned j = 0; j < instr->operands.size(); j++) {
                  assert(instr->definitions[j].bytes() == instr->operands[j].bytes());
                  copy_operations[instr->definitions[j].physReg()] = {
                     instr->operands[j], instr->definitions[j], instr->operands[j].bytes()};
               }
               handle_operands(copy_operations, &ctx, program->gfx_level, pi);
               break;
            }
            case aco_opcode::p_start_linear_vgpr: {
               if (instr->operands.empty())
                  break;

               Definition def(instr->definitions[0].physReg(),
                              RegClass::get(RegType::vgpr, instr->definitions[0].bytes()));

               std::map<PhysReg, copy_operation> copy_operations;
               copy_operations[def.physReg()] = {instr->operands[0], def,
                                                 instr->operands[0].bytes()};
               handle_operands(copy_operations, &ctx, program->gfx_level, pi);
               break;
            }
            case aco_opcode::p_exit_early_if: {
               /* don't bother with an early exit near the end of the program */
               if ((block->instructions.size() - 1 - instr_idx) <= 4 &&
                   block->instructions.back()->opcode == aco_opcode::s_endpgm) {
                  unsigned null_exp_dest =
                     program->gfx_level >= GFX11 ? V_008DFC_SQ_EXP_MRT : V_008DFC_SQ_EXP_NULL;
                  bool ignore_early_exit = true;

                  for (unsigned k = instr_idx + 1; k < block->instructions.size(); ++k) {
                     const aco_ptr<Instruction>& instr2 = block->instructions[k];
                     if (instr2->opcode == aco_opcode::s_endpgm ||
                         instr2->opcode == aco_opcode::p_logical_end)
                        continue;
                     else if (instr2->opcode == aco_opcode::exp &&
                              instr2->exp().dest == null_exp_dest &&
                              instr2->exp().enabled_mask == 0)
                        continue;
                     else if (instr2->opcode == aco_opcode::p_parallelcopy &&
                              instr2->definitions[0].isFixed() &&
                              instr2->definitions[0].physReg() == exec)
                        continue;

                     ignore_early_exit = false;
                  }

                  if (ignore_early_exit)
                     break;
               }

               const bool discard_sends_pops_done =
                  pops_done_msg_bounds.early_exit_needs_done_msg(block_idx, instr_idx);

               Block* discard_block =
                  discard_sends_pops_done ? discard_pops_done_and_exit_block : discard_exit_block;
               if (!discard_block) {
                  discard_block = program->create_and_insert_block();
                  discard_block->kind = block_kind_discard_early_exit;
                  if (discard_sends_pops_done) {
                     discard_pops_done_and_exit_block = discard_block;
                  } else {
                     discard_exit_block = discard_block;
                  }
                  block = &program->blocks[block_idx];

                  bld.reset(discard_block);
                  if (program->has_pops_overlapped_waves_wait &&
                      (program->gfx_level >= GFX11 || discard_sends_pops_done)) {
                     /* If this discard early exit potentially exits the POPS ordered section, do
                      * the waitcnt necessary before resuming overlapping waves as the normal
                      * waitcnt insertion doesn't work in a discard early exit block.
                      */
                     if (program->gfx_level >= GFX10)
                        bld.sopk(aco_opcode::s_waitcnt_vscnt, Operand(sgpr_null, s1), 0);
                     wait_imm pops_exit_wait_imm;
                     pops_exit_wait_imm.vm = 0;
                     if (program->has_smem_buffer_or_global_loads)
                        pops_exit_wait_imm.lgkm = 0;
                     bld.sopp(aco_opcode::s_waitcnt, -1,
                              pops_exit_wait_imm.pack(program->gfx_level));
                  }
                  if (discard_sends_pops_done)
                     bld.sopp(aco_opcode::s_sendmsg, -1, sendmsg_ordered_ps_done);
                  unsigned target = V_008DFC_SQ_EXP_NULL;
                  if (program->gfx_level >= GFX11)
                     target =
                        program->has_color_exports ? V_008DFC_SQ_EXP_MRT : V_008DFC_SQ_EXP_MRTZ;
                  if (program->stage == fragment_fs)
                     bld.exp(aco_opcode::exp, Operand(v1), Operand(v1), Operand(v1), Operand(v1), 0,
                             target, false, true, true);
                  if (should_dealloc_vgprs)
                     bld.sopp(aco_opcode::s_sendmsg, -1, sendmsg_dealloc_vgprs);
                  bld.sopp(aco_opcode::s_endpgm);

                  bld.reset(&ctx.instructions);
               }

               assert(instr->operands[0].physReg() == scc);
               bld.sopp(aco_opcode::s_cbranch_scc0, instr->operands[0], discard_block->index);

               discard_block->linear_preds.push_back(block->index);
               block->linear_succs.push_back(discard_block->index);
               break;
            }
            case aco_opcode::p_spill: {
               assert(instr->operands[0].regClass() == v1.as_linear());
               for (unsigned i = 0; i < instr->operands[2].size(); i++) {
                  Operand src =
                     instr->operands[2].isConstant()
                        ? Operand::c32(uint32_t(instr->operands[2].constantValue64() >> (32 * i)))
                        : Operand(PhysReg{instr->operands[2].physReg() + i}, s1);
                  bld.writelane(bld.def(v1, instr->operands[0].physReg()), src,
                                Operand::c32(instr->operands[1].constantValue() + i),
                                instr->operands[0]);
               }
               break;
            }
            case aco_opcode::p_reload: {
               assert(instr->operands[0].regClass() == v1.as_linear());
               for (unsigned i = 0; i < instr->definitions[0].size(); i++)
                  bld.readlane(bld.def(s1, PhysReg{instr->definitions[0].physReg() + i}),
                               instr->operands[0],
                               Operand::c32(instr->operands[1].constantValue() + i));
               break;
            }
            case aco_opcode::p_as_uniform: {
               if (instr->operands[0].isConstant() ||
                   instr->operands[0].regClass().type() == RegType::sgpr) {
                  std::map<PhysReg, copy_operation> copy_operations;
                  copy_operations[instr->definitions[0].physReg()] = {
                     instr->operands[0], instr->definitions[0], instr->definitions[0].bytes()};
                  handle_operands(copy_operations, &ctx, program->gfx_level, pi);
               } else {
                  assert(instr->operands[0].regClass().type() == RegType::vgpr);
                  assert(instr->definitions[0].regClass().type() == RegType::sgpr);
                  assert(instr->operands[0].size() == instr->definitions[0].size());
                  for (unsigned i = 0; i < instr->definitions[0].size(); i++) {
                     bld.vop1(aco_opcode::v_readfirstlane_b32,
                              bld.def(s1, PhysReg{instr->definitions[0].physReg() + i}),
                              Operand(PhysReg{instr->operands[0].physReg() + i}, v1));
                  }
               }
               break;
            }
            case aco_opcode::p_pops_gfx9_add_exiting_wave_id: {
               bld.sop2(aco_opcode::s_add_i32, instr->definitions[0], instr->definitions[1],
                        Operand(pops_exiting_wave_id, s1), instr->operands[0]);
               break;
            }
            case aco_opcode::p_bpermute_readlane: {
               emit_bpermute_readlane(program, instr, bld);
               break;
            }
            case aco_opcode::p_bpermute_shared_vgpr: {
               emit_bpermute_shared_vgpr(program, instr, bld);
               break;
            }
            case aco_opcode::p_bpermute_permlane: {
               emit_bpermute_permlane(program, instr, bld);
               break;
            }
            case aco_opcode::p_constaddr: {
               unsigned id = instr->definitions[0].tempId();
               PhysReg reg = instr->definitions[0].physReg();
               bld.sop1(aco_opcode::p_constaddr_getpc, instr->definitions[0], Operand::c32(id));
               bld.sop2(aco_opcode::p_constaddr_addlo, Definition(reg, s1), bld.def(s1, scc),
                        Operand(reg, s1), instr->operands[0], Operand::c32(id));
               /* s_addc_u32 not needed because the program is in a 32-bit VA range */
               break;
            }
            case aco_opcode::p_resume_shader_address: {
               /* Find index of resume block. */
               unsigned resume_idx = instr->operands[0].constantValue();
               unsigned resume_block_idx = 0;
               for (Block& resume_block : program->blocks) {
                  if (resume_block.kind & block_kind_resume) {
                     if (resume_idx == 0) {
                        resume_block_idx = resume_block.index;
                        break;
                     }
                     resume_idx--;
                  }
               }
               assert(resume_block_idx != 0);
               unsigned id = instr->definitions[0].tempId();
               PhysReg reg = instr->definitions[0].physReg();
               bld.sop1(aco_opcode::p_resumeaddr_getpc, instr->definitions[0], Operand::c32(id));
               bld.sop2(aco_opcode::p_resumeaddr_addlo, Definition(reg, s1), bld.def(s1, scc),
                        Operand(reg, s1), Operand::c32(resume_block_idx), Operand::c32(id));
               /* s_addc_u32 not needed because the program is in a 32-bit VA range */
               break;
            }
            case aco_opcode::p_extract: {
               assert(instr->operands[1].isConstant());
               assert(instr->operands[2].isConstant());
               assert(instr->operands[3].isConstant());
               if (instr->definitions[0].regClass() == s1)
                  assert(instr->definitions.size() >= 2 && instr->definitions[1].physReg() == scc);
               Definition dst = instr->definitions[0];
               Operand op = instr->operands[0];
               unsigned bits = instr->operands[2].constantValue();
               unsigned index = instr->operands[1].constantValue();
               unsigned offset = index * bits;
               bool signext = !instr->operands[3].constantEquals(0);

               if (dst.regClass() == s1) {
                  if (offset == (32 - bits)) {
                     bld.sop2(signext ? aco_opcode::s_ashr_i32 : aco_opcode::s_lshr_b32, dst,
                              bld.def(s1, scc), op, Operand::c32(offset));
                  } else if (offset == 0 && signext && (bits == 8 || bits == 16)) {
                     bld.sop1(bits == 8 ? aco_opcode::s_sext_i32_i8 : aco_opcode::s_sext_i32_i16,
                              dst, op);
                  } else if (ctx.program->gfx_level >= GFX9 && offset == 0 && bits == 16) {
                     bld.sop2(aco_opcode::s_pack_ll_b32_b16, dst, op, Operand::zero());
                  } else {
                     bld.sop2(signext ? aco_opcode::s_bfe_i32 : aco_opcode::s_bfe_u32, dst,
                              bld.def(s1, scc), op, Operand::c32((bits << 16) | offset));
                  }
               } else if ((dst.regClass() == v1 && op.physReg().byte() == 0) ||
                          ctx.program->gfx_level <= GFX7) {
                  assert(op.physReg().byte() == 0 && dst.physReg().byte() == 0);
                  if (offset == (32 - bits) && op.regClass() != s1) {
                     bld.vop2(signext ? aco_opcode::v_ashrrev_i32 : aco_opcode::v_lshrrev_b32, dst,
                              Operand::c32(offset), op);
                  } else if (offset == 0 && bits == 16 && ctx.program->gfx_level >= GFX11) {
                     bld.vop1(signext ? aco_opcode::v_cvt_i32_i16 : aco_opcode::v_cvt_u32_u16, dst,
                              op);
                  } else {
                     bld.vop3(signext ? aco_opcode::v_bfe_i32 : aco_opcode::v_bfe_u32, dst, op,
                              Operand::c32(offset), Operand::c32(bits));
                  }
               } else {
                  assert(dst.regClass() == v2b || dst.regClass() == v1b || op.regClass() == v2b ||
                         op.regClass() == v1b);
                  if (ctx.program->gfx_level >= GFX11) {
                     unsigned op_vgpr_byte = op.physReg().byte() + offset / 8;
                     unsigned sign_byte = op_vgpr_byte + bits / 8 - 1;

                     uint8_t swiz[4] = {4, 5, 6, 7};
                     swiz[dst.physReg().byte()] = op_vgpr_byte;
                     if (bits == 16)
                        swiz[dst.physReg().byte() + 1] = op_vgpr_byte + 1;
                     for (unsigned i = bits / 8; i < dst.bytes(); i++) {
                        uint8_t ext = bperm_0;
                        if (signext) {
                           if (sign_byte == 1)
                              ext = bperm_b1_sign;
                           else if (sign_byte == 3)
                              ext = bperm_b3_sign;
                           else /* replicate so sign-extension can be done later */
                              ext = sign_byte;
                        }
                        swiz[dst.physReg().byte() + i] = ext;
                     }
                     create_bperm(bld, swiz, dst, op);

                     if (signext && sign_byte != 3 && sign_byte != 1) {
                        assert(bits == 8);
                        assert(dst.regClass() == v2b || dst.regClass() == v1);
                        uint8_t ext_swiz[4] = {4, 5, 6, 7};
                        uint8_t ext = dst.physReg().byte() == 2 ? bperm_b7_sign : bperm_b5_sign;
                        memset(ext_swiz + dst.physReg().byte() + 1, ext, dst.bytes() - 1);
                        create_bperm(bld, ext_swiz, dst, Operand::zero());
                     }
                  } else {
                     SDWA_instruction& sdwa = bld.vop1_sdwa(aco_opcode::v_mov_b32, dst, op)->sdwa();
                     sdwa.sel[0] = SubdwordSel(bits / 8, offset / 8, signext);
                  }
               }
               break;
            }
            case aco_opcode::p_insert: {
               assert(instr->operands[1].isConstant());
               assert(instr->operands[2].isConstant());
               if (instr->definitions[0].regClass() == s1)
                  assert(instr->definitions.size() >= 2 && instr->definitions[1].physReg() == scc);
               Definition dst = instr->definitions[0];
               Operand op = instr->operands[0];
               unsigned bits = instr->operands[2].constantValue();
               unsigned index = instr->operands[1].constantValue();
               unsigned offset = index * bits;

               bool has_sdwa = program->gfx_level >= GFX8 && program->gfx_level < GFX11;
               if (dst.regClass() == s1) {
                  if (offset == (32 - bits)) {
                     bld.sop2(aco_opcode::s_lshl_b32, dst, bld.def(s1, scc), op,
                              Operand::c32(offset));
                  } else if (offset == 0) {
                     bld.sop2(aco_opcode::s_bfe_u32, dst, bld.def(s1, scc), op,
                              Operand::c32(bits << 16));
                  } else {
                     bld.sop2(aco_opcode::s_bfe_u32, dst, bld.def(s1, scc), op,
                              Operand::c32(bits << 16));
                     bld.sop2(aco_opcode::s_lshl_b32, dst, bld.def(s1, scc),
                              Operand(dst.physReg(), s1), Operand::c32(offset));
                  }
               } else if (dst.regClass() == v1 || !has_sdwa) {
                  if (offset == (dst.bytes() * 8u - bits) &&
                      (dst.regClass() == v1 || program->gfx_level <= GFX7)) {
                     bld.vop2(aco_opcode::v_lshlrev_b32, dst, Operand::c32(offset), op);
                  } else if (offset == 0 && (dst.regClass() == v1 || program->gfx_level <= GFX7)) {
                     bld.vop3(aco_opcode::v_bfe_u32, dst, op, Operand::zero(), Operand::c32(bits));
                  } else if (has_sdwa && (op.regClass() != s1 || program->gfx_level >= GFX9)) {
                     bld.vop1_sdwa(aco_opcode::v_mov_b32, dst, op)->sdwa().dst_sel =
                        SubdwordSel(bits / 8, offset / 8, false);
                  } else if (program->gfx_level >= GFX11) {
                     uint8_t swiz[] = {4, 5, 6, 7};
                     for (unsigned i = 0; i < dst.bytes(); i++)
                        swiz[dst.physReg().byte() + i] = bperm_0;
                     for (unsigned i = 0; i < bits / 8; i++)
                        swiz[dst.physReg().byte() + i + offset / 8] = op.physReg().byte() + i;
                     create_bperm(bld, swiz, dst, op);
                  } else {
                     bld.vop3(aco_opcode::v_bfe_u32, dst, op, Operand::zero(), Operand::c32(bits));
                     bld.vop2(aco_opcode::v_lshlrev_b32, dst, Operand::c32(offset),
                              Operand(dst.physReg(), v1));
                  }
               } else {
                  assert(dst.regClass() == v2b);
                  bld.vop2_sdwa(aco_opcode::v_lshlrev_b32, dst, Operand::c32(offset), op)
                     ->sdwa()
                     .sel[1] = SubdwordSel::ubyte;
               }
               break;
            }
            case aco_opcode::p_init_scratch: {
               assert(program->gfx_level >= GFX8 && program->gfx_level <= GFX10_3);
               if (!program->config->scratch_bytes_per_wave)
                  break;

               Operand scratch_addr = instr->operands[0];
               if (scratch_addr.isUndefined()) {
                  PhysReg reg = instr->definitions[0].physReg();
                  bld.sop1(aco_opcode::p_load_symbol, Definition(reg, s1),
                           Operand::c32(aco_symbol_scratch_addr_lo));
                  bld.sop1(aco_opcode::p_load_symbol, Definition(reg.advance(4), s1),
                           Operand::c32(aco_symbol_scratch_addr_hi));
                  scratch_addr.setFixed(reg);
               } else if (program->stage.hw != AC_HW_COMPUTE_SHADER) {
                  bld.smem(aco_opcode::s_load_dwordx2, instr->definitions[0], scratch_addr,
                           Operand::zero());
                  scratch_addr.setFixed(instr->definitions[0].physReg());
               }

               hw_init_scratch(bld, instr->definitions[0], scratch_addr, instr->operands[1]);
               break;
            }
            case aco_opcode::p_jump_to_epilog: {
               if (pops_done_msg_bounds.early_exit_needs_done_msg(block_idx, instr_idx)) {
                  bld.sopp(aco_opcode::s_sendmsg, -1, sendmsg_ordered_ps_done);
               }
               bld.sop1(aco_opcode::s_setpc_b64, instr->operands[0]);
               break;
            }
            case aco_opcode::p_interp_gfx11: {
               assert(instr->definitions[0].regClass() == v1 ||
                      instr->definitions[0].regClass() == v2b);
               assert(instr->operands[0].regClass() == v1.as_linear());
               assert(instr->operands[1].isConstant());
               assert(instr->operands[2].isConstant());
               assert(instr->operands.back().physReg() == m0);
               Definition dst = instr->definitions[0];
               PhysReg lin_vgpr = instr->operands[0].physReg();
               unsigned attribute = instr->operands[1].constantValue();
               unsigned component = instr->operands[2].constantValue();
               uint16_t dpp_ctrl = 0;
               Operand coord1, coord2;
               if (instr->operands.size() == 6) {
                  assert(instr->operands[3].regClass() == v1);
                  assert(instr->operands[4].regClass() == v1);
                  coord1 = instr->operands[3];
                  coord2 = instr->operands[4];
               } else {
                  assert(instr->operands[3].isConstant());
                  dpp_ctrl = instr->operands[3].constantValue();
               }

               bld.ldsdir(aco_opcode::lds_param_load, Definition(lin_vgpr, v1), Operand(m0, s1),
                          attribute, component);

               Operand p(lin_vgpr, v1);
               Operand dst_op(dst.physReg(), v1);
               if (instr->operands.size() == 5) {
                  bld.vop1_dpp(aco_opcode::v_mov_b32, Definition(dst), p, dpp_ctrl);
               } else if (dst.regClass() == v2b) {
                  bld.vinterp_inreg(aco_opcode::v_interp_p10_f16_f32_inreg, Definition(dst), p,
                                    coord1, p);
                  bld.vinterp_inreg(aco_opcode::v_interp_p2_f16_f32_inreg, Definition(dst), p,
                                    coord2, dst_op);
               } else {
                  bld.vinterp_inreg(aco_opcode::v_interp_p10_f32_inreg, Definition(dst), p, coord1,
                                    p);
                  bld.vinterp_inreg(aco_opcode::v_interp_p2_f32_inreg, Definition(dst), p, coord2,
                                    dst_op);
               }
               break;
            }
            case aco_opcode::p_dual_src_export_gfx11: {
               PhysReg dst0 = instr->definitions[0].physReg();
               PhysReg dst1 = instr->definitions[1].physReg();
               Definition exec_tmp = instr->definitions[2];
               Definition not_vcc_tmp = instr->definitions[3];
               Definition clobber_vcc = instr->definitions[4];
               Definition clobber_scc = instr->definitions[5];

               assert(exec_tmp.regClass() == bld.lm);
               assert(not_vcc_tmp.regClass() == bld.lm);
               assert(clobber_vcc.regClass() == bld.lm && clobber_vcc.physReg() == vcc);
               assert(clobber_scc.isFixed() && clobber_scc.physReg() == scc);

               bld.sop1(Builder::s_mov, Definition(exec_tmp.physReg(), bld.lm),
                        Operand(exec, bld.lm));
               bld.sop1(Builder::s_wqm, Definition(exec, bld.lm), clobber_scc,
                        Operand(exec, bld.lm));

               uint8_t enabled_channels = 0;
               Operand mrt0[4], mrt1[4];

               bld.sop1(aco_opcode::s_mov_b32, Definition(clobber_vcc.physReg(), s1),
                        Operand::c32(0x55555555));
               if (ctx.program->wave_size == 64)
                  bld.sop1(aco_opcode::s_mov_b32, Definition(clobber_vcc.physReg().advance(4), s1),
                           Operand::c32(0x55555555));

               Operand src_even = Operand(clobber_vcc.physReg(), bld.lm);

               bld.sop1(Builder::s_not, not_vcc_tmp, clobber_scc, src_even);

               Operand src_odd = Operand(not_vcc_tmp.physReg(), bld.lm);

               for (unsigned i = 0; i < 4; i++) {
                  if (instr->operands[i].isUndefined() && instr->operands[i + 4].isUndefined()) {
                     mrt0[i] = instr->operands[i];
                     mrt1[i] = instr->operands[i + 4];
                     continue;
                  }

                  Operand src0 = instr->operands[i];
                  Operand src1 = instr->operands[i + 4];

                  /*      | even lanes | odd lanes
                   * mrt0 | src0 even  | src1 even
                   * mrt1 | src0 odd   | src1 odd
                   */
                  bld.vop2_dpp(aco_opcode::v_cndmask_b32, Definition(dst0, v1), src1, src0,
                               src_even, dpp_row_xmask(1));
                  bld.vop2_e64_dpp(aco_opcode::v_cndmask_b32, Definition(dst1, v1), src0, src1,
                                   src_odd, dpp_row_xmask(1));

                  mrt0[i] = Operand(dst0, v1);
                  mrt1[i] = Operand(dst1, v1);

                  enabled_channels |= 1 << i;

                  dst0 = dst0.advance(4);
                  dst1 = dst1.advance(4);
               }

               bld.sop1(Builder::s_mov, Definition(exec, bld.lm),
                        Operand(exec_tmp.physReg(), bld.lm));

               /* Force export all channels when everything is undefined. */
               if (!enabled_channels)
                  enabled_channels = 0xf;

               bld.exp(aco_opcode::exp, mrt0[0], mrt0[1], mrt0[2], mrt0[3], enabled_channels,
                       V_008DFC_SQ_EXP_MRT + 21, false);
               bld.exp(aco_opcode::exp, mrt1[0], mrt1[1], mrt1[2], mrt1[3], enabled_channels,
                       V_008DFC_SQ_EXP_MRT + 22, false);
               break;
            }
            case aco_opcode::p_end_with_regs: {
               end_with_regs_block_index = block->index;
               break;
            }
            default: break;
            }
         } else if (instr->isBranch()) {
            Pseudo_branch_instruction* branch = &instr->branch();
            const uint32_t target = branch->target[0];
            const bool uniform_branch = !((branch->opcode == aco_opcode::p_cbranch_z ||
                                           branch->opcode == aco_opcode::p_cbranch_nz) &&
                                          branch->operands[0].physReg() == exec);

            /* Check if the branch instruction can be removed.
             * This is beneficial when executing the next block with an empty exec mask
             * is faster than the branch instruction itself.
             *
             * Override this judgement when:
             * - The application prefers to remove control flow
             * - The compiler stack knows that it's a divergent branch always taken
             */
            const bool prefer_remove =
               branch->selection_control_remove && ctx.program->gfx_level >= GFX10;
            bool can_remove = block->index < target;
            unsigned num_scalar = 0;
            unsigned num_vector = 0;

            /* Check the instructions between branch and target */
            for (unsigned i = block->index + 1; i < branch->target[0]; i++) {
               /* Uniform conditional branches must not be ignored if they
                * are about to jump over actual instructions */
               if (uniform_branch && !program->blocks[i].instructions.empty())
                  can_remove = false;

               if (!can_remove)
                  break;

               for (aco_ptr<Instruction>& inst : program->blocks[i].instructions) {
                  if (inst->isSOPP()) {
                     /* Discard early exits and loop breaks and continues should work fine with an
                      * empty exec mask.
                      */
                     bool is_break_continue =
                        program->blocks[i].kind & (block_kind_break | block_kind_continue);
                     bool discard_early_exit =
                        inst->sopp().block != -1 &&
                        (program->blocks[inst->sopp().block].kind & block_kind_discard_early_exit);
                     if ((inst->opcode != aco_opcode::s_cbranch_scc0 &&
                          inst->opcode != aco_opcode::s_cbranch_scc1) ||
                         (!discard_early_exit && !is_break_continue))
                        can_remove = false;
                  } else if (inst->isSALU()) {
                     num_scalar++;
                  } else if (inst->isVALU() || inst->isVINTRP()) {
                     if (instr->opcode == aco_opcode::v_writelane_b32 ||
                         instr->opcode == aco_opcode::v_writelane_b32_e64) {
                        /* writelane ignores exec, writing inactive lanes results in UB. */
                        can_remove = false;
                     }
                     num_vector++;
                     /* VALU which writes SGPRs are always executed on GFX10+ */
                     if (ctx.program->gfx_level >= GFX10) {
                        for (Definition& def : inst->definitions) {
                           if (def.regClass().type() == RegType::sgpr)
                              num_scalar++;
                        }
                     }
                  } else if (inst->isEXP()) {
                     /* Export instructions with exec=0 can hang some GFX10+ (unclear on old GPUs). */
                     can_remove = false;
                  } else if (inst->isVMEM() || inst->isFlatLike() || inst->isDS() ||
                             inst->isLDSDIR()) {
                     // TODO: GFX6-9 can use vskip
                     can_remove = prefer_remove;
                  } else if (inst->isSMEM()) {
                     /* SMEM are at least as expensive as branches */
                     can_remove = prefer_remove;
                  } else if (inst->isBarrier()) {
                     can_remove = prefer_remove;
                  } else {
                     can_remove = false;
                     assert(false && "Pseudo instructions should be lowered by this point.");
                  }

                  if (!prefer_remove) {
                     /* Under these conditions, we shouldn't remove the branch.
                      * Don't care about the estimated cycles when the shader prefers flattening.
                      */
                     unsigned est_cycles;
                     if (ctx.program->gfx_level >= GFX10)
                        est_cycles = num_scalar * 2 + num_vector;
                     else
                        est_cycles = num_scalar * 4 + num_vector * 4;

                     if (est_cycles > 16)
                        can_remove = false;
                  }

                  if (!can_remove)
                     break;
               }
            }

            if (can_remove)
               continue;

            /* emit branch instruction */
            switch (instr->opcode) {
            case aco_opcode::p_branch:
               assert(block->linear_succs[0] == target);
               bld.sopp(aco_opcode::s_branch, branch->definitions[0], target);
               break;
            case aco_opcode::p_cbranch_nz:
               assert(block->linear_succs[1] == target);
               if (branch->operands[0].physReg() == exec)
                  bld.sopp(aco_opcode::s_cbranch_execnz, branch->definitions[0], target);
               else if (branch->operands[0].physReg() == vcc)
                  bld.sopp(aco_opcode::s_cbranch_vccnz, branch->definitions[0], target);
               else {
                  assert(branch->operands[0].physReg() == scc);
                  bld.sopp(aco_opcode::s_cbranch_scc1, branch->definitions[0], target);
               }
               break;
            case aco_opcode::p_cbranch_z:
               assert(block->linear_succs[1] == target);
               if (branch->operands[0].physReg() == exec)
                  bld.sopp(aco_opcode::s_cbranch_execz, branch->definitions[0], target);
               else if (branch->operands[0].physReg() == vcc)
                  bld.sopp(aco_opcode::s_cbranch_vccz, branch->definitions[0], target);
               else {
                  assert(branch->operands[0].physReg() == scc);
                  bld.sopp(aco_opcode::s_cbranch_scc0, branch->definitions[0], target);
               }
               break;
            default: unreachable("Unknown Pseudo branch instruction!");
            }

         } else if (instr->isReduction()) {
            Pseudo_reduction_instruction& reduce = instr->reduction();
            emit_reduction(&ctx, reduce.opcode, reduce.reduce_op, reduce.cluster_size,
                           reduce.operands[1].physReg(),    // tmp
                           reduce.definitions[1].physReg(), // stmp
                           reduce.operands[2].physReg(),    // vtmp
                           reduce.definitions[2].physReg(), // sitmp
                           reduce.operands[0], reduce.definitions[0]);
         } else if (instr->isBarrier()) {
            Pseudo_barrier_instruction& barrier = instr->barrier();

            /* Anything larger than a workgroup isn't possible. Anything
             * smaller requires no instructions and this pseudo instruction
             * would only be included to control optimizations. */
            bool emit_s_barrier = barrier.exec_scope == scope_workgroup &&
                                  program->workgroup_size > program->wave_size;

            bld.insert(std::move(instr));
            if (emit_s_barrier)
               bld.sopp(aco_opcode::s_barrier);
         } else if (instr->opcode == aco_opcode::p_cvt_f16_f32_rtne) {
            float_mode new_mode = block->fp_mode;
            new_mode.round16_64 = fp_round_ne;
            bool set_round = new_mode.round != block->fp_mode.round;

            emit_set_mode(bld, new_mode, set_round, false);

            instr->opcode = aco_opcode::v_cvt_f16_f32;
            ctx.instructions.emplace_back(std::move(instr));

            emit_set_mode(bld, block->fp_mode, set_round, false);
         } else if (instr->isMIMG() && instr->mimg().strict_wqm) {
            lower_image_sample(&ctx, instr);
            ctx.instructions.emplace_back(std::move(instr));
         } else {
            ctx.instructions.emplace_back(std::move(instr));
         }
      }

      /* Send the ordered section done message from this block if it's needed in this block, but
       * instr_after_end_idx() points beyond the end of its instructions. This may commonly happen
       * if the common post-dominator of multiple end locations turns out to be an empty block.
       */
      if (block_idx == pops_done_msg_bounds.end_block_idx() &&
          pops_done_msg_bounds.instr_after_end_idx() >= block->instructions.size()) {
         bld.sopp(aco_opcode::s_sendmsg, -1, sendmsg_ordered_ps_done);
      }

      block->instructions = std::move(ctx.instructions);
   }

   /* If block with p_end_with_regs is not the last block (i.e. p_exit_early_if may append exit
    * block at last), create an exit block for it to branch to.
    */
   int last_block_index = program->blocks.size() - 1;
   if (end_with_regs_block_index >= 0 && end_with_regs_block_index != last_block_index) {
      Block* exit_block = program->create_and_insert_block();
      Block* end_with_regs_block = &program->blocks[end_with_regs_block_index];
      exit_block->linear_preds.push_back(end_with_regs_block->index);
      end_with_regs_block->linear_succs.push_back(exit_block->index);

      Builder bld(program, end_with_regs_block);
      bld.sopp(aco_opcode::s_branch, exit_block->index);

      /* For insert waitcnt pass to add waitcnt in exit block, otherwise waitcnt will be added
       * after the s_branch which won't be executed.
       */
      end_with_regs_block->kind &= ~block_kind_end_with_regs;
      exit_block->kind |= block_kind_end_with_regs;
   }
}

} // namespace aco
