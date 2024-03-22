/*
 * Copyright (c) 2019 Zodiac Inflight Innovations
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Jonathan Marek <jonathan@marek.ca>
 */

#include "etnaviv_compiler_nir.h"
#include "util/compiler.h"

/* to map nir srcs should to etna_inst srcs */
enum {
   SRC_0_1_2 = (0 << 0) | (1 << 2) | (2 << 4),
   SRC_0_1_X = (0 << 0) | (1 << 2) | (3 << 4),
   SRC_0_X_X = (0 << 0) | (3 << 2) | (3 << 4),
   SRC_0_X_1 = (0 << 0) | (3 << 2) | (1 << 4),
   SRC_0_1_0 = (0 << 0) | (1 << 2) | (0 << 4),
   SRC_X_X_0 = (3 << 0) | (3 << 2) | (0 << 4),
   SRC_0_X_0 = (0 << 0) | (3 << 2) | (0 << 4),
};

/* info to translate a nir op to etna_inst */
struct etna_op_info {
   uint8_t opcode; /* INST_OPCODE_ */
   uint8_t src; /* SRC_ enum  */
   uint8_t cond; /* INST_CONDITION_ */
   uint8_t type; /* INST_TYPE_ */
};

static const struct etna_op_info etna_ops[] = {
   [0 ... nir_num_opcodes - 1] = {0xff},
#undef TRUE
#undef FALSE
#define OPCT(nir, op, src, cond, type) [nir_op_##nir] = { \
   INST_OPCODE_##op, \
   SRC_##src, \
   INST_CONDITION_##cond, \
   INST_TYPE_##type \
}
#define OPC(nir, op, src, cond) OPCT(nir, op, src, cond, F32)
#define IOPC(nir, op, src, cond) OPCT(nir, op, src, cond, S32)
#define UOPC(nir, op, src, cond) OPCT(nir, op, src, cond, U32)
#define OP(nir, op, src) OPC(nir, op, src, TRUE)
#define IOP(nir, op, src) IOPC(nir, op, src, TRUE)
#define UOP(nir, op, src) UOPC(nir, op, src, TRUE)
   OP(mov, MOV, X_X_0), OP(fneg, MOV, X_X_0), OP(fabs, MOV, X_X_0), OP(fsat, MOV, X_X_0),
   OP(fmul, MUL, 0_1_X), OP(fadd, ADD, 0_X_1), OP(ffma, MAD, 0_1_2),
   OP(fdot2, DP2, 0_1_X), OP(fdot3, DP3, 0_1_X), OP(fdot4, DP4, 0_1_X),
   OPC(fmin, SELECT, 0_1_0, GT), OPC(fmax, SELECT, 0_1_0, LT),
   OP(ffract, FRC, X_X_0), OP(frcp, RCP, X_X_0), OP(frsq, RSQ, X_X_0),
   OP(fsqrt, SQRT, X_X_0), OP(fsin, SIN, X_X_0), OP(fcos, COS, X_X_0),
   OP(fsign, SIGN, X_X_0), OP(ffloor, FLOOR, X_X_0), OP(fceil, CEIL, X_X_0),
   OP(flog2, LOG, X_X_0), OP(fexp2, EXP, X_X_0),
   OPC(seq, SET, 0_1_X, EQ), OPC(sne, SET, 0_1_X, NE), OPC(sge, SET, 0_1_X, GE), OPC(slt, SET, 0_1_X, LT),
   OPC(fcsel, SELECT, 0_1_2, NZ),
   OP(fdiv, DIV, 0_1_X),
   OP(fddx, DSX, 0_X_0), OP(fddy, DSY, 0_X_0),

   /* type convert */
   IOP(i2f32, I2F, 0_X_X),
   IOP(i2i32, I2I, 0_X_X),
   OPCT(i2i16, I2I, 0_X_X, TRUE, S16),
   OPCT(i2i8,  I2I, 0_X_X, TRUE, S8),
   UOP(u2f32, I2F, 0_X_X),
   UOP(u2u32, I2I, 0_X_X),
   OPCT(u2u16, I2I, 0_X_X, TRUE, U16),
   OPCT(u2u8,  I2I, 0_X_X, TRUE, U8),
   IOP(f2i32, F2I, 0_X_X),
   OPCT(f2i16, F2I, 0_X_X, TRUE, S16),
   OPCT(f2i8,  F2I, 0_X_X, TRUE, S8),
   UOP(f2u32, F2I, 0_X_X),
   OPCT(f2u16, F2I, 0_X_X, TRUE, U16),
   OPCT(f2u8,  F2I, 0_X_X, TRUE, U8),
   UOP(b2f32, AND, 0_X_X), /* AND with fui(1.0f) */
   UOP(b2i32, AND, 0_X_X), /* AND with 1 */

   /* arithmetic */
   IOP(iadd, ADD, 0_X_1),
   IOP(imul, IMULLO0, 0_1_X),
   /* IOP(imad, IMADLO0, 0_1_2), */
   IOP(ineg, ADD, X_X_0), /* ADD 0, -x */
   IOP(iabs, IABS, X_X_0),
   IOP(isign, SIGN, X_X_0),
   IOPC(imin, SELECT, 0_1_0, GT),
   IOPC(imax, SELECT, 0_1_0, LT),
   UOPC(umin, SELECT, 0_1_0, GT),
   UOPC(umax, SELECT, 0_1_0, LT),

   /* select */
   UOPC(b32csel, SELECT, 0_1_2, NZ),

   /* compare with int result */
    OPC(feq32, CMP, 0_1_X, EQ),
    OPC(fneu32, CMP, 0_1_X, NE),
    OPC(fge32, CMP, 0_1_X, GE),
    OPC(flt32, CMP, 0_1_X, LT),
   IOPC(ieq32, CMP, 0_1_X, EQ),
   IOPC(ine32, CMP, 0_1_X, NE),
   IOPC(ige32, CMP, 0_1_X, GE),
   IOPC(ilt32, CMP, 0_1_X, LT),
   UOPC(uge32, CMP, 0_1_X, GE),
   UOPC(ult32, CMP, 0_1_X, LT),

   /* bit ops */
   IOP(ior,  OR,  0_X_1),
   IOP(iand, AND, 0_X_1),
   IOP(ixor, XOR, 0_X_1),
   IOP(inot, NOT, X_X_0),
   IOP(ishl, LSHIFT, 0_X_1),
   IOP(ishr, RSHIFT, 0_X_1),
   UOP(ushr, RSHIFT, 0_X_1),
   UOP(uclz, LEADZERO, 0_X_X),
};

void
etna_emit_alu(struct etna_compile *c, nir_op op, struct etna_inst_dst dst,
              struct etna_inst_src src[3], bool saturate)
{
   struct etna_op_info ei = etna_ops[op];
   unsigned swiz_scalar = INST_SWIZ_BROADCAST(ffs(dst.write_mask) - 1);

   if (ei.opcode == 0xff)
      compile_error(c, "Unhandled ALU op: %s\n", nir_op_infos[op].name);

   struct etna_inst inst = {
      .opcode = ei.opcode,
      .type = ei.type,
      .cond = ei.cond,
      .dst = dst,
      .sat = saturate,
   };

   switch (op) {
   case nir_op_fdiv:
   case nir_op_flog2:
   case nir_op_fsin:
   case nir_op_fcos:
      if (c->specs->has_new_transcendentals)
         inst.tex.amode = 1;
      FALLTHROUGH;
   case nir_op_frsq:
   case nir_op_frcp:
   case nir_op_fexp2:
   case nir_op_fsqrt:
   case nir_op_imul:
      /* scalar instructions we want src to be in x component */
      src[0].swiz = inst_swiz_compose(src[0].swiz, swiz_scalar);
      src[1].swiz = inst_swiz_compose(src[1].swiz, swiz_scalar);
      break;
   /* deal with instructions which don't have 1:1 mapping */
   case nir_op_b2f32:
      inst.src[2] = etna_immediate_float(1.0f);
      break;
   case nir_op_b2i32:
      inst.src[2] = etna_immediate_int(1);
      break;
   case nir_op_ineg:
      inst.src[0] = etna_immediate_int(0);
      src[0].neg = 1;
      break;
   default:
      break;
   }

   /* set the "true" value for CMP instructions */
   if (inst.opcode == INST_OPCODE_CMP)
      inst.src[2] = etna_immediate_int(-1);

   for (unsigned j = 0; j < 3; j++) {
      unsigned i = ((ei.src >> j*2) & 3);
      if (i < 3)
         inst.src[j] = src[i];
   }

   emit_inst(c, &inst);
}

void
etna_emit_tex(struct etna_compile *c, nir_texop op, unsigned texid, unsigned dst_swiz,
              struct etna_inst_dst dst, struct etna_inst_src coord,
              struct etna_inst_src src1, struct etna_inst_src src2)
{
   struct etna_inst inst = {
      .dst = dst,
      .tex.id = texid + (is_fs(c) ? 0 : c->specs->vertex_sampler_offset),
      .tex.swiz = dst_swiz,
      .src[0] = coord,
   };

   if (src1.use)
      inst.src[1] = src1;

   if (src2.use)
      inst.src[2] = src2;

   switch (op) {
   case nir_texop_tex: inst.opcode = INST_OPCODE_TEXLD; break;
   case nir_texop_txb: inst.opcode = INST_OPCODE_TEXLDB; break;
   case nir_texop_txd: inst.opcode = INST_OPCODE_TEXLDD; break;
   case nir_texop_txl: inst.opcode = INST_OPCODE_TEXLDL; break;
   default:
      compile_error(c, "Unhandled NIR tex type: %d\n", op);
   }

   emit_inst(c, &inst);
}

void
etna_emit_jump(struct etna_compile *c, unsigned block, struct etna_inst_src condition)
{
   if (!condition.use) {
      emit_inst(c, &(struct etna_inst) {.opcode = INST_OPCODE_BRANCH, .imm = block });
      return;
   }

   struct etna_inst inst = {
      .opcode = INST_OPCODE_BRANCH,
      .cond = INST_CONDITION_NOT,
      .type = INST_TYPE_U32,
      .src[0] = condition,
      .imm = block,
   };
   inst.src[0].swiz = INST_SWIZ_BROADCAST(inst.src[0].swiz & 3);
   emit_inst(c, &inst);
}

void
etna_emit_discard(struct etna_compile *c, struct etna_inst_src condition)
{
   if (!condition.use) {
      emit_inst(c, &(struct etna_inst) { .opcode = INST_OPCODE_TEXKILL });
      return;
   }

   struct etna_inst inst = {
      .opcode = INST_OPCODE_TEXKILL,
      .cond = INST_CONDITION_NZ,
      .type = (c->specs->halti < 2) ? INST_TYPE_F32 : INST_TYPE_U32,
      .src[0] = condition,
   };
   inst.src[0].swiz = INST_SWIZ_BROADCAST(inst.src[0].swiz & 3);
   emit_inst(c, &inst);
}
