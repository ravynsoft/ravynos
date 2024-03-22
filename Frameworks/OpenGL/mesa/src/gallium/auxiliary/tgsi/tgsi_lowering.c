/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "tgsi/tgsi_transform.h"
#include "tgsi/tgsi_scan.h"
#include "tgsi/tgsi_dump.h"

#include "util/compiler.h"
#include "util/u_debug.h"
#include "util/u_math.h"

#include "tgsi_lowering.h"

struct tgsi_lowering_context {
   struct tgsi_transform_context base;
   const struct tgsi_lowering_config *config;
   struct tgsi_shader_info *info;
   unsigned two_side_colors;
   unsigned two_side_idx[PIPE_MAX_SHADER_INPUTS];
   unsigned color_base;  /* base register for chosen COLOR/BCOLOR's */
   int face_idx;
   unsigned numtmp;
   struct {
      struct tgsi_full_src_register src;
      struct tgsi_full_dst_register dst;
   } tmp[2];
#define A 0
#define B 1
   struct tgsi_full_src_register imm;
   int emitted_decls;
   unsigned saturate;
};

static inline struct tgsi_lowering_context *
tgsi_lowering_context(struct tgsi_transform_context *tctx)
{
   return (struct tgsi_lowering_context *)tctx;
}

/*
 * Utility helpers:
 */

static void
reg_dst(struct tgsi_full_dst_register *dst,
	const struct tgsi_full_dst_register *orig_dst, unsigned wrmask)
{
   *dst = *orig_dst;
   dst->Register.WriteMask &= wrmask;
   assert(dst->Register.WriteMask);
}

static inline void
get_swiz(unsigned *swiz, const struct tgsi_src_register *src)
{
   swiz[0] = src->SwizzleX;
   swiz[1] = src->SwizzleY;
   swiz[2] = src->SwizzleZ;
   swiz[3] = src->SwizzleW;
}

static void
reg_src(struct tgsi_full_src_register *src,
	const struct tgsi_full_src_register *orig_src,
	unsigned sx, unsigned sy, unsigned sz, unsigned sw)
{
   unsigned swiz[4];
   get_swiz(swiz, &orig_src->Register);
   *src = *orig_src;
   src->Register.SwizzleX = swiz[sx];
   src->Register.SwizzleY = swiz[sy];
   src->Register.SwizzleZ = swiz[sz];
   src->Register.SwizzleW = swiz[sw];
}

#define TGSI_SWIZZLE__ TGSI_SWIZZLE_X  /* don't-care value! */
#define SWIZ(x,y,z,w) TGSI_SWIZZLE_ ## x, TGSI_SWIZZLE_ ## y,   \
      TGSI_SWIZZLE_ ## z, TGSI_SWIZZLE_ ## w

/*
 * if (dst.x aliases src.x) {
 *   MOV tmpA.x, src.x
 *   src = tmpA
 * }
 * COS dst.x, src.x
 * SIN dst.y, src.x
 * MOV dst.zw, imm{0.0, 1.0}
 */
static bool
aliases(const struct tgsi_full_dst_register *dst, unsigned dst_mask,
	const struct tgsi_full_src_register *src, unsigned src_mask)
{
   if ((dst->Register.File == src->Register.File) &&
       (dst->Register.Index == src->Register.Index)) {
      unsigned i, actual_mask = 0;
      unsigned swiz[4];
      get_swiz(swiz, &src->Register);
      for (i = 0; i < 4; i++)
         if (src_mask & (1 << i))
            actual_mask |= (1 << swiz[i]);
      if (actual_mask & dst_mask)
         return true;
   }
   return false;
}

static void
create_mov(struct tgsi_transform_context *tctx,
           const struct tgsi_full_dst_register *dst,
           const struct tgsi_full_src_register *src,
           unsigned mask, unsigned saturate)
{
   struct tgsi_full_instruction new_inst;

   new_inst = tgsi_default_full_instruction();
   new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
   new_inst.Instruction.Saturate = saturate;
   new_inst.Instruction.NumDstRegs = 1;
   reg_dst(&new_inst.Dst[0], dst, mask);
   new_inst.Instruction.NumSrcRegs = 1;
   reg_src(&new_inst.Src[0], src, SWIZ(X, Y, Z, W));
   tctx->emit_instruction(tctx, &new_inst);
}

/* to help calculate # of tgsi tokens for a lowering.. we assume
 * the worst case, ie. removed instructions don't have ADDR[] or
 * anything which increases the # of tokens per src/dst and the
 * inserted instructions do.
 *
 * OINST() - old instruction
 *    1         : instruction itself
 *    1         : dst
 *    1 * nargs : srcN
 *
 * NINST() - new instruction
 *    1         : instruction itself
 *    2         : dst
 *    2 * nargs : srcN
 */

#define OINST(nargs)  (1 + 1 + 1 * (nargs))
#define NINST(nargs)  (1 + 2 + 2 * (nargs))

/*
 * Lowering Translators:
 */

/* DST - Distance Vector
 *   dst.x = 1.0
 *   dst.y = src0.y \times src1.y
 *   dst.z = src0.z
 *   dst.w = src1.w
 *
 * ; note: could be more clever and use just a single temp
 * ;       if I was clever enough to re-write the swizzles.
 * ; needs: 2 tmp, imm{1.0}
 * if (dst.y aliases src0.z) {
 *   MOV tmpA.yz, src0.yz
 *   src0 = tmpA
 * }
 * if (dst.yz aliases src1.w) {
 *   MOV tmpB.yw, src1.yw
 *   src1 = tmpB
 * }
 * MUL dst.y, src0.y, src1.y
 * MOV dst.z, src0.z
 * MOV dst.w, src1.w
 * MOV dst.x, imm{1.0}
 */
#define DST_GROW (NINST(1) + NINST(1) + NINST(2) + NINST(1) + \
		NINST(1) + NINST(1) - OINST(2))
#define DST_TMP  2
static void
transform_dst(struct tgsi_transform_context *tctx,
              struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst  = &inst->Dst[0];
   struct tgsi_full_src_register *src0 = &inst->Src[0];
   struct tgsi_full_src_register *src1 = &inst->Src[1];
   struct tgsi_full_instruction new_inst;

   if (aliases(dst, TGSI_WRITEMASK_Y, src0, TGSI_WRITEMASK_Z)) {
      create_mov(tctx, &ctx->tmp[A].dst, src0, TGSI_WRITEMASK_YZ, 0);
      src0 = &ctx->tmp[A].src;
   }

   if (aliases(dst, TGSI_WRITEMASK_YZ, src1, TGSI_WRITEMASK_W)) {
      create_mov(tctx, &ctx->tmp[B].dst, src1, TGSI_WRITEMASK_YW, 0);
      src1 = &ctx->tmp[B].src;
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_Y) {
      /* MUL dst.y, src0.y, src1.y */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MUL;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_Y);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src0, SWIZ(_, Y, _, _));
      reg_src(&new_inst.Src[1], src1, SWIZ(_, Y, _, _));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_Z) {
      /* MOV dst.z, src0.z */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_Z);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], src0, SWIZ(_, _, Z, _));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_W) {
      /* MOV dst.w, src1.w */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_W);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], src1, SWIZ(_, _, _, W));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_X) {
      /* MOV dst.x, imm{1.0} */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_X);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->imm, SWIZ(Y, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* LRP - Linear Interpolate
 *  dst.x = src0.x \times src1.x + (1.0 - src0.x) \times src2.x
 *  dst.y = src0.y \times src1.y + (1.0 - src0.y) \times src2.y
 *  dst.z = src0.z \times src1.z + (1.0 - src0.z) \times src2.z
 *  dst.w = src0.w \times src1.w + (1.0 - src0.w) \times src2.w
 *
 * This becomes: src0 \times src1 + src2 - src0 \times src2, which
 * can then become: src0 \times src1 - (src0 \times src2 - src2)
 *
 * ; needs: 1 tmp
 * MAD tmpA, src0, src2, -src2
 * MAD dst, src0, src1, -tmpA
 */
#define LRP_GROW (NINST(3) + NINST(3) - OINST(3))
#define LRP_TMP  1
static void
transform_lrp(struct tgsi_transform_context *tctx,
              struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst  = &inst->Dst[0];
   struct tgsi_full_src_register *src0 = &inst->Src[0];
   struct tgsi_full_src_register *src1 = &inst->Src[1];
   struct tgsi_full_src_register *src2 = &inst->Src[2];
   struct tgsi_full_instruction new_inst;

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XYZW) {
      /* MAD tmpA, src0, src2, -src2 */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MAD;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_XYZW);
      new_inst.Instruction.NumSrcRegs = 3;
      reg_src(&new_inst.Src[0], src0, SWIZ(X, Y, Z, W));
      reg_src(&new_inst.Src[1], src2, SWIZ(X, Y, Z, W));
      reg_src(&new_inst.Src[2], src2, SWIZ(X, Y, Z, W));
      new_inst.Src[2].Register.Negate = !new_inst.Src[2].Register.Negate;
      tctx->emit_instruction(tctx, &new_inst);

      /* MAD dst, src0, src1, -tmpA */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MAD;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_XYZW);
      new_inst.Instruction.NumSrcRegs = 3;
      reg_src(&new_inst.Src[0], src0, SWIZ(X, Y, Z, W));
      reg_src(&new_inst.Src[1], src1, SWIZ(X, Y, Z, W));
      reg_src(&new_inst.Src[2], &ctx->tmp[A].src, SWIZ(X, Y, Z, W));
      new_inst.Src[2].Register.Negate = true;
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* FRC - Fraction
 *  dst.x = src.x - \lfloor src.x\rfloor
 *  dst.y = src.y - \lfloor src.y\rfloor
 *  dst.z = src.z - \lfloor src.z\rfloor
 *  dst.w = src.w - \lfloor src.w\rfloor
 *
 * ; needs: 1 tmp
 * FLR tmpA, src
 * SUB dst, src, tmpA
 */
#define FRC_GROW (NINST(1) + NINST(2) - OINST(1))
#define FRC_TMP  1
static void
transform_frc(struct tgsi_transform_context *tctx,
              struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst = &inst->Dst[0];
   struct tgsi_full_src_register *src = &inst->Src[0];
   struct tgsi_full_instruction new_inst;

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XYZW) {
      /* FLR tmpA, src */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_FLR;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_XYZW);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], src, SWIZ(X, Y, Z, W));
      tctx->emit_instruction(tctx, &new_inst);

      /* SUB dst, src, tmpA */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_ADD;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_XYZW);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src, SWIZ(X, Y, Z, W));
      reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(X, Y, Z, W));
      new_inst.Src[1].Register.Negate = 1;
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* POW - Power
 *  dst.x = src0.x^{src1.x}
 *  dst.y = src0.x^{src1.x}
 *  dst.z = src0.x^{src1.x}
 *  dst.w = src0.x^{src1.x}
 *
 * ; needs: 1 tmp
 * LG2 tmpA.x, src0.x
 * MUL tmpA.x, src1.x, tmpA.x
 * EX2 dst, tmpA.x
 */
#define POW_GROW (NINST(1) + NINST(2) + NINST(1) - OINST(2))
#define POW_TMP  1
static void
transform_pow(struct tgsi_transform_context *tctx,
              struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst  = &inst->Dst[0];
   struct tgsi_full_src_register *src0 = &inst->Src[0];
   struct tgsi_full_src_register *src1 = &inst->Src[1];
   struct tgsi_full_instruction new_inst;

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XYZW) {
      /* LG2 tmpA.x, src0.x */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_LG2;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], src0, SWIZ(X, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* MUL tmpA.x, src1.x, tmpA.x */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MUL;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src1, SWIZ(X, _, _, _));
      reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(X, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* EX2 dst, tmpA.x */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_EX2;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_XYZW);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(X, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* LIT - Light Coefficients
 *  dst.x = 1.0
 *  dst.y = max(src.x, 0.0)
 *  dst.z = (src.x > 0.0) ? max(src.y, 0.0)^{clamp(src.w, -128.0, 128.0))} : 0
 *  dst.w = 1.0
 *
 * ; needs: 1 tmp, imm{0.0}, imm{1.0}, imm{128.0}
 * MAX tmpA.xy, src.xy, imm{0.0}
 * CLAMP tmpA.z, src.w, -imm{128.0}, imm{128.0}
 * LG2 tmpA.y, tmpA.y
 * MUL tmpA.y, tmpA.z, tmpA.y
 * EX2 tmpA.y, tmpA.y
 * CMP tmpA.y, -src.x, tmpA.y, imm{0.0}
 * MOV dst.yz, tmpA.xy
 * MOV dst.xw, imm{1.0}
 */
#define LIT_GROW (NINST(1) + NINST(3) + NINST(1) + NINST(2) + \
		NINST(1) + NINST(3) + NINST(1) + NINST(1) - OINST(1))
#define LIT_TMP  1
static void
transform_lit(struct tgsi_transform_context *tctx,
              struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst = &inst->Dst[0];
   struct tgsi_full_src_register *src = &inst->Src[0];
   struct tgsi_full_instruction new_inst;

   if (dst->Register.WriteMask & TGSI_WRITEMASK_YZ) {
      /* MAX tmpA.xy, src.xy, imm{0.0} */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MAX;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_XY);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src, SWIZ(X, Y, _, _));
      reg_src(&new_inst.Src[1], &ctx->imm, SWIZ(X, X, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* MIN tmpA.z, src.w, imm{128.0} */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MIN;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Z);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src, SWIZ(_, _, W, _));
      reg_src(&new_inst.Src[1], &ctx->imm, SWIZ(_, _, Z, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* MAX tmpA.z, tmpA.z, -imm{128.0} */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MAX;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Z);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(_, _, Z, _));
      reg_src(&new_inst.Src[1], &ctx->imm, SWIZ(_, _, Z, _));
      new_inst.Src[1].Register.Negate = true;
      tctx->emit_instruction(tctx, &new_inst);

      /* LG2 tmpA.y, tmpA.y */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_LG2;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Y);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(Y, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* MUL tmpA.y, tmpA.z, tmpA.y */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MUL;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Y);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(_, Z, _, _));
      reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(_, Y, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* EX2 tmpA.y, tmpA.y */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_EX2;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Y);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(Y, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* CMP tmpA.y, -src.x, tmpA.y, imm{0.0} */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_CMP;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Y);
      new_inst.Instruction.NumSrcRegs = 3;
      reg_src(&new_inst.Src[0], src, SWIZ(_, X, _, _));
      new_inst.Src[0].Register.Negate = true;
      reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(_, Y, _, _));
      reg_src(&new_inst.Src[2], &ctx->imm, SWIZ(_, X, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* MOV dst.yz, tmpA.xy */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_YZ);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(_, X, Y, _));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XW) {
      /* MOV dst.xw, imm{1.0} */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_XW);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->imm, SWIZ(Y, _, _, Y));
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* EXP - Approximate Exponential Base 2
 *  dst.x = 2^{\lfloor src.x\rfloor}
 *  dst.y = src.x - \lfloor src.x\rfloor
 *  dst.z = 2^{src.x}
 *  dst.w = 1.0
 *
 * ; needs: 1 tmp, imm{1.0}
 * if (lowering FLR) {
 *   FRC tmpA.x, src.x
 *   SUB tmpA.x, src.x, tmpA.x
 * } else {
 *   FLR tmpA.x, src.x
 * }
 * EX2 tmpA.y, src.x
 * SUB dst.y, src.x, tmpA.x
 * EX2 dst.x, tmpA.x
 * MOV dst.z, tmpA.y
 * MOV dst.w, imm{1.0}
 */
#define EXP_GROW (NINST(1) + NINST(2) + NINST(1) + NINST(2) + NINST(1) + \
		NINST(1)+ NINST(1) - OINST(1))
#define EXP_TMP  1
static void
transform_exp(struct tgsi_transform_context *tctx,
              struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst = &inst->Dst[0];
   struct tgsi_full_src_register *src = &inst->Src[0];
   struct tgsi_full_instruction new_inst;

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XY) {
      if (ctx->config->lower_FLR) {
         /* FRC tmpA.x, src.x */
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_FRC;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
         new_inst.Instruction.NumSrcRegs = 1;
         reg_src(&new_inst.Src[0], src, SWIZ(X, _, _, _));
         tctx->emit_instruction(tctx, &new_inst);

         /* SUB tmpA.x, src.x, tmpA.x */
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_ADD;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
         new_inst.Instruction.NumSrcRegs = 2;
         reg_src(&new_inst.Src[0], src, SWIZ(X, _, _, _));
         reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(X, _, _, _));
         new_inst.Src[1].Register.Negate = 1;
         tctx->emit_instruction(tctx, &new_inst);
     } else {
         /* FLR tmpA.x, src.x */
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_FLR;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
         new_inst.Instruction.NumSrcRegs = 1;
         reg_src(&new_inst.Src[0], src, SWIZ(X, _, _, _));
         tctx->emit_instruction(tctx, &new_inst);
      }
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_Z) {
      /* EX2 tmpA.y, src.x */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_EX2;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Y);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], src, SWIZ(X, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_Y) {
      /* SUB dst.y, src.x, tmpA.x */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_ADD;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_Y);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src, SWIZ(_, X, _, _));
      reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(_, X, _, _));
      new_inst.Src[1].Register.Negate = 1;
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_X) {
      /* EX2 dst.x, tmpA.x */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_EX2;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_X);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(X, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_Z) {
      /* MOV dst.z, tmpA.y */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_Z);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(_, _, Y, _));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_W) {
      /* MOV dst.w, imm{1.0} */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_W);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->imm, SWIZ(_, _, _, Y));
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* LOG - Approximate Logarithm Base 2
 *  dst.x = \lfloor\log_2{|src.x|}\rfloor
 *  dst.y = \frac{|src.x|}{2^{\lfloor\log_2{|src.x|}\rfloor}}
 *  dst.z = \log_2{|src.x|}
 *  dst.w = 1.0
 *
 * ; needs: 1 tmp, imm{1.0}
 * LG2 tmpA.x, |src.x|
 * if (lowering FLR) {
 *   FRC tmpA.y, tmpA.x
 *   SUB tmpA.y, tmpA.x, tmpA.y
 * } else {
 *   FLR tmpA.y, tmpA.x
 * }
 * EX2 tmpA.z, tmpA.y
 * RCP tmpA.z, tmpA.z
 * MUL dst.y, |src.x|, tmpA.z
 * MOV dst.xz, tmpA.yx
 * MOV dst.w, imm{1.0}
 */
#define LOG_GROW (NINST(1) + NINST(1) + NINST(2) + NINST(1) + NINST(1) + \
		NINST(2) + NINST(1) + NINST(1) - OINST(1))
#define LOG_TMP  1
static void
transform_log(struct tgsi_transform_context *tctx,
              struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst = &inst->Dst[0];
   struct tgsi_full_src_register *src = &inst->Src[0];
   struct tgsi_full_instruction new_inst;

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XYZ) {
      /* LG2 tmpA.x, |src.x| */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_LG2;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], src, SWIZ(X, _, _, _));
      new_inst.Src[0].Register.Absolute = true;
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XY) {
      if (ctx->config->lower_FLR) {
         /* FRC tmpA.y, tmpA.x */
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_FRC;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Y);
         new_inst.Instruction.NumSrcRegs = 1;
         reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(_, X, _, _));
         tctx->emit_instruction(tctx, &new_inst);

         /* SUB tmpA.y, tmpA.x, tmpA.y */
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_ADD;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Y);
         new_inst.Instruction.NumSrcRegs = 2;
         reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(_, X, _, _));
         reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(_, Y, _, _));
         new_inst.Src[1].Register.Negate = 1;
         tctx->emit_instruction(tctx, &new_inst);
      } else {
         /* FLR tmpA.y, tmpA.x */
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_FLR;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Y);
         new_inst.Instruction.NumSrcRegs = 1;
         reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(_, X, _, _));
         tctx->emit_instruction(tctx, &new_inst);
      }
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_Y) {
      /* EX2 tmpA.z, tmpA.y */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_EX2;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Z);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(Y, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* RCP tmpA.z, tmpA.z */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_RCP;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_Z);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(Z, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* MUL dst.y, |src.x|, tmpA.z */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MUL;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_Y);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src, SWIZ(_, X, _, _));
      new_inst.Src[0].Register.Absolute = true;
      reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(_, Z, _, _));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XZ) {
      /* MOV dst.xz, tmpA.yx */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_XZ);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(Y, _, X, _));
      tctx->emit_instruction(tctx, &new_inst);
   }

   if (dst->Register.WriteMask & TGSI_WRITEMASK_W) {
      /* MOV dst.w, imm{1.0} */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_W);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->imm, SWIZ(_, _, _, Y));
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* DP4 - 4-component Dot Product
 *   dst = src0.x \times src1.x + src0.y \times src1.y + src0.z \times src1.z + src0.w \times src1.w
 *
 * DP3 - 3-component Dot Product
 *   dst = src0.x \times src1.x + src0.y \times src1.y + src0.z \times src1.z
 *
 * DP2 - 2-component Dot Product
 *   dst = src0.x \times src1.x + src0.y \times src1.y
 *
 * NOTE: these are translated into sequence of MUL/MAD(/ADD) scalar
 * operations, which is what you'd prefer for a ISA that is natively
 * scalar.  Probably a native vector ISA would at least already have
 * DP4/DP3 instructions, but perhaps there is room for an alternative
 * translation for DP2 using vector instructions.
 *
 * ; needs: 1 tmp
 * MUL tmpA.x, src0.x, src1.x
 * MAD tmpA.x, src0.y, src1.y, tmpA.x
 * if (DP3 || DP4) {
 *   MAD tmpA.x, src0.z, src1.z, tmpA.x
 *   if (DP4) {
 *     MAD tmpA.x, src0.w, src1.w, tmpA.x
 *   }
 * }
 * ; fixup last instruction to replicate into dst
 */
#define DP4_GROW  (NINST(2) + NINST(3) + NINST(3) + NINST(3) - OINST(2))
#define DP3_GROW  (NINST(2) + NINST(3) + NINST(3) - OINST(2))
#define DP2_GROW  (NINST(2) + NINST(3) - OINST(2))
#define DOTP_TMP  1
static void
transform_dotp(struct tgsi_transform_context *tctx,
               struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst  = &inst->Dst[0];
   struct tgsi_full_src_register *src0 = &inst->Src[0];
   struct tgsi_full_src_register *src1 = &inst->Src[1];
   struct tgsi_full_instruction new_inst;
   enum tgsi_opcode opcode = inst->Instruction.Opcode;

   /* NOTE: any potential last instruction must replicate src on all
    * components (since it could be re-written to write to final dst)
    */

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XYZW) {
      /* MUL tmpA.x, src0.x, src1.x */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MUL;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src0, SWIZ(X, _, _, _));
      reg_src(&new_inst.Src[1], src1, SWIZ(X, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* MAD tmpA.x, src0.y, src1.y, tmpA.x */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MAD;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
      new_inst.Instruction.NumSrcRegs = 3;
      reg_src(&new_inst.Src[0], src0, SWIZ(Y, Y, Y, Y));
      reg_src(&new_inst.Src[1], src1, SWIZ(Y, Y, Y, Y));
      reg_src(&new_inst.Src[2], &ctx->tmp[A].src, SWIZ(X, X, X, X));

      if ((opcode == TGSI_OPCODE_DP3) ||
          (opcode == TGSI_OPCODE_DP4)) {
         tctx->emit_instruction(tctx, &new_inst);

         /* MAD tmpA.x, src0.z, src1.z, tmpA.x */
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_MAD;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
         new_inst.Instruction.NumSrcRegs = 3;
         reg_src(&new_inst.Src[0], src0, SWIZ(Z, Z, Z, Z));
         reg_src(&new_inst.Src[1], src1, SWIZ(Z, Z, Z, Z));
         reg_src(&new_inst.Src[2], &ctx->tmp[A].src, SWIZ(X, X, X, X));

         if (opcode == TGSI_OPCODE_DP4) {
            tctx->emit_instruction(tctx, &new_inst);

            /* MAD tmpA.x, src0.w, src1.w, tmpA.x */
            new_inst = tgsi_default_full_instruction();
            new_inst.Instruction.Opcode = TGSI_OPCODE_MAD;
            new_inst.Instruction.NumDstRegs = 1;
            reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_X);
            new_inst.Instruction.NumSrcRegs = 3;
            reg_src(&new_inst.Src[0], src0, SWIZ(W, W, W, W));
            reg_src(&new_inst.Src[1], src1, SWIZ(W, W, W, W));
            reg_src(&new_inst.Src[2], &ctx->tmp[A].src, SWIZ(X, X, X, X));
         }
      }

      /* fixup last instruction to write to dst: */
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_XYZW);

      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* FLR - floor, CEIL - ceil
 * ; needs: 1 tmp
 * if (CEIL) {
 *   FRC tmpA, -src
 *   ADD dst, src, tmpA
 * } else {
 *   FRC tmpA, src
 *   SUB dst, src, tmpA
 * }
 */
#define FLR_GROW (NINST(1) + NINST(2) - OINST(1))
#define CEIL_GROW (NINST(1) + NINST(2) - OINST(1))
#define FLR_TMP 1
#define CEIL_TMP 1
static void
transform_flr_ceil(struct tgsi_transform_context *tctx,
                   struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst  = &inst->Dst[0];
   struct tgsi_full_src_register *src0 = &inst->Src[0];
   struct tgsi_full_instruction new_inst;
   enum tgsi_opcode opcode = inst->Instruction.Opcode;

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XYZW) {
      /* FLR: FRC tmpA, src  CEIL: FRC tmpA, -src */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_FRC;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_XYZW);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], src0, SWIZ(X, Y, Z, W));

      if (opcode == TGSI_OPCODE_CEIL)
         new_inst.Src[0].Register.Negate = !new_inst.Src[0].Register.Negate;
      tctx->emit_instruction(tctx, &new_inst);

      /* FLR: SUB dst, src, tmpA  CEIL: ADD dst, src, tmpA */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_ADD;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_XYZW);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], src0, SWIZ(X, Y, Z, W));
      reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(X, Y, Z, W));
      if (opcode == TGSI_OPCODE_FLR)
         new_inst.Src[1].Register.Negate = 1;
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* TRUNC - truncate off fractional part
 *  dst.x = trunc(src.x)
 *  dst.y = trunc(src.y)
 *  dst.z = trunc(src.z)
 *  dst.w = trunc(src.w)
 *
 * ; needs: 1 tmp
 * if (lower FLR) {
 *   FRC tmpA, |src|
 *   SUB tmpA, |src|, tmpA
 * } else {
 *   FLR tmpA, |src|
 * }
 * CMP dst, src, -tmpA, tmpA
 */
#define TRUNC_GROW (NINST(1) + NINST(2) + NINST(3) - OINST(1))
#define TRUNC_TMP 1
static void
transform_trunc(struct tgsi_transform_context *tctx,
                struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_dst_register *dst  = &inst->Dst[0];
   struct tgsi_full_src_register *src0 = &inst->Src[0];
   struct tgsi_full_instruction new_inst;

   if (dst->Register.WriteMask & TGSI_WRITEMASK_XYZW) {
      if (ctx->config->lower_FLR) {
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_FRC;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_XYZW);
         new_inst.Instruction.NumSrcRegs = 1;
         reg_src(&new_inst.Src[0], src0, SWIZ(X, Y, Z, W));
         new_inst.Src[0].Register.Absolute = true;
         new_inst.Src[0].Register.Negate = false;
         tctx->emit_instruction(tctx, &new_inst);

         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_ADD;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_XYZW);
         new_inst.Instruction.NumSrcRegs = 2;
         reg_src(&new_inst.Src[0], src0, SWIZ(X, Y, Z, W));
         new_inst.Src[0].Register.Absolute = true;
         new_inst.Src[0].Register.Negate = false;
         reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(X, Y, Z, W));
         new_inst.Src[1].Register.Negate = 1;
         tctx->emit_instruction(tctx, &new_inst);
      } else {
         new_inst = tgsi_default_full_instruction();
         new_inst.Instruction.Opcode = TGSI_OPCODE_FLR;
         new_inst.Instruction.NumDstRegs = 1;
         reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, TGSI_WRITEMASK_XYZW);
         new_inst.Instruction.NumSrcRegs = 1;
         reg_src(&new_inst.Src[0], src0, SWIZ(X, Y, Z, W));
         new_inst.Src[0].Register.Absolute = true;
         new_inst.Src[0].Register.Negate = false;
         tctx->emit_instruction(tctx, &new_inst);
      }

      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_CMP;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], dst, TGSI_WRITEMASK_XYZW);
      new_inst.Instruction.NumSrcRegs = 3;
      reg_src(&new_inst.Src[0], src0, SWIZ(X, Y, Z, W));
      reg_src(&new_inst.Src[1], &ctx->tmp[A].src, SWIZ(X, Y, Z, W));
      new_inst.Src[1].Register.Negate = true;
      reg_src(&new_inst.Src[2], &ctx->tmp[A].src, SWIZ(X, Y, Z, W));
      tctx->emit_instruction(tctx, &new_inst);
   }
}

/* Inserts a MOV_SAT for the needed components of tex coord.  Note that
 * in the case of TXP, the clamping must happen *after* projection, so
 * we need to lower TXP to TEX.
 *
 *   MOV tmpA, src0
 *   if (opc == TXP) {
 *     ; do perspective division manually before clamping:
 *     RCP tmpB, tmpA.w
 *     MUL tmpB.<pmask>, tmpA, tmpB.xxxx
 *     opc = TEX;
 *   }
 *   MOV_SAT tmpA.<mask>, tmpA  ; <mask> is the clamped s/t/r coords
 *   <opc> dst, tmpA, ...
 */
#define SAMP_GROW (NINST(1) + NINST(1) + NINST(2) + NINST(1))
#define SAMP_TMP  2
static int
transform_samp(struct tgsi_transform_context *tctx,
               struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_full_src_register *coord = &inst->Src[0];
   struct tgsi_full_src_register *samp;
   struct tgsi_full_instruction new_inst;
   /* mask is clamped coords, pmask is all coords (for projection): */
   unsigned mask = 0, pmask = 0, smask;
   unsigned tex = inst->Texture.Texture;
   enum tgsi_opcode opcode = inst->Instruction.Opcode;
   bool lower_txp = (opcode == TGSI_OPCODE_TXP) &&
		   (ctx->config->lower_TXP & (1 << tex));

   if (opcode == TGSI_OPCODE_TXB2) {
      samp = &inst->Src[2];
   } else {
      samp = &inst->Src[1];
   }

   /* convert sampler # to bitmask to test: */
   smask = 1 << samp->Register.Index;

   /* check if we actually need to lower this one: */
   if (!(ctx->saturate & smask) && !lower_txp)
      return -1;

   /* figure out which coordinates need saturating:
    *   - RECT textures should not get saturated
    *   - array index coords should not get saturated
    */
   switch (tex) {
   case TGSI_TEXTURE_3D:
   case TGSI_TEXTURE_CUBE:
   case TGSI_TEXTURE_CUBE_ARRAY:
   case TGSI_TEXTURE_SHADOWCUBE:
   case TGSI_TEXTURE_SHADOWCUBE_ARRAY:
      if (ctx->config->saturate_r & smask)
         mask |= TGSI_WRITEMASK_Z;
      pmask |= TGSI_WRITEMASK_Z;
      FALLTHROUGH;

   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_2D_ARRAY:
   case TGSI_TEXTURE_SHADOW2D:
   case TGSI_TEXTURE_SHADOW2D_ARRAY:
   case TGSI_TEXTURE_2D_MSAA:
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      if (ctx->config->saturate_t & smask)
         mask |= TGSI_WRITEMASK_Y;
      pmask |= TGSI_WRITEMASK_Y;
      FALLTHROUGH;

   case TGSI_TEXTURE_1D:
   case TGSI_TEXTURE_1D_ARRAY:
   case TGSI_TEXTURE_SHADOW1D:
   case TGSI_TEXTURE_SHADOW1D_ARRAY:
      if (ctx->config->saturate_s & smask)
         mask |= TGSI_WRITEMASK_X;
      pmask |= TGSI_WRITEMASK_X;
      break;

   case TGSI_TEXTURE_RECT:
   case TGSI_TEXTURE_SHADOWRECT:
      /* we don't saturate, but in case of lower_txp we
       * still need to do the perspective divide:
       */
       pmask = TGSI_WRITEMASK_XY;
       break;
   }

   /* sanity check.. driver could be asking to saturate a non-
    * existent coordinate component:
    */
   if (!mask && !lower_txp)
      return -1;

   /* MOV tmpA, src0 */
   create_mov(tctx, &ctx->tmp[A].dst, coord, TGSI_WRITEMASK_XYZW, 0);

   /* This is a bit sad.. we need to clamp *after* the coords
    * are projected, which means lowering TXP to TEX and doing
    * the projection ourself.  But since I haven't figured out
    * how to make the lowering code deliver an electric shock
    * to anyone using GL_CLAMP, we must do this instead:
    */
   if (opcode == TGSI_OPCODE_TXP) {
      /* RCP tmpB.x tmpA.w */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_RCP;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[B].dst, TGSI_WRITEMASK_X);
      new_inst.Instruction.NumSrcRegs = 1;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(W, _, _, _));
      tctx->emit_instruction(tctx, &new_inst);

      /* MUL tmpA.mask, tmpA, tmpB.xxxx */
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_MUL;
      new_inst.Instruction.NumDstRegs = 1;
      reg_dst(&new_inst.Dst[0], &ctx->tmp[A].dst, pmask);
      new_inst.Instruction.NumSrcRegs = 2;
      reg_src(&new_inst.Src[0], &ctx->tmp[A].src, SWIZ(X, Y, Z, W));
      reg_src(&new_inst.Src[1], &ctx->tmp[B].src, SWIZ(X, X, X, X));
      tctx->emit_instruction(tctx, &new_inst);

      opcode = TGSI_OPCODE_TEX;
   }

   /* MOV_SAT tmpA.<mask>, tmpA */
   if (mask) {
      create_mov(tctx, &ctx->tmp[A].dst, &ctx->tmp[A].src, mask, 1);
   }

   /* modify the texture samp instruction to take fixed up coord: */
   new_inst = *inst;
   new_inst.Instruction.Opcode = opcode;
   new_inst.Src[0] = ctx->tmp[A].src;
   tctx->emit_instruction(tctx, &new_inst);

   return 0;
}

/* Two-sided color emulation:
 * For each COLOR input, create a corresponding BCOLOR input, plus
 * CMP instruction to select front or back color based on FACE
 */
#define TWOSIDE_GROW(n)  (                      \
      2 +         /* FACE */                    \
      ((n) * 3) + /* IN[], BCOLOR[n], <intrp> */\
      ((n) * 1) + /* TEMP[] */                  \
      ((n) * NINST(3))   /* CMP instr */        \
      )

static void
emit_twoside(struct tgsi_transform_context *tctx)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_shader_info *info = ctx->info;
   struct tgsi_full_declaration decl;
   struct tgsi_full_instruction new_inst;
   unsigned inbase, tmpbase;
   unsigned i;

   inbase  = info->file_max[TGSI_FILE_INPUT] + 1;
   tmpbase = info->file_max[TGSI_FILE_TEMPORARY] + 1;

   /* additional inputs for BCOLOR's */
   for (i = 0; i < ctx->two_side_colors; i++) {
      unsigned in_idx = ctx->two_side_idx[i];
      decl = tgsi_default_full_declaration();
      decl.Declaration.File = TGSI_FILE_INPUT;
      decl.Declaration.Semantic = true;
      decl.Range.First = decl.Range.Last = inbase + i;
      decl.Semantic.Name = TGSI_SEMANTIC_BCOLOR;
      decl.Semantic.Index = info->input_semantic_index[in_idx];
      decl.Declaration.Interpolate = true;
      decl.Interp.Interpolate = info->input_interpolate[in_idx];
      decl.Interp.Location = info->input_interpolate_loc[in_idx];
      tctx->emit_declaration(tctx, &decl);
   }

   /* additional input for FACE */
   if (ctx->two_side_colors && (ctx->face_idx == -1)) {
      decl = tgsi_default_full_declaration();
      decl.Declaration.File = TGSI_FILE_INPUT;
      decl.Declaration.Semantic = true;
      decl.Range.First = decl.Range.Last = inbase + ctx->two_side_colors;
      decl.Semantic.Name = TGSI_SEMANTIC_FACE;
      decl.Semantic.Index = 0;
      tctx->emit_declaration(tctx, &decl);

      ctx->face_idx = decl.Range.First;
   }

   /* additional temps for COLOR/BCOLOR selection: */
   for (i = 0; i < ctx->two_side_colors; i++) {
      decl = tgsi_default_full_declaration();
      decl.Declaration.File = TGSI_FILE_TEMPORARY;
      decl.Range.First = decl.Range.Last = tmpbase + ctx->numtmp + i;
      tctx->emit_declaration(tctx, &decl);
   }

   /* and finally additional instructions to select COLOR/BCOLOR: */
   for (i = 0; i < ctx->two_side_colors; i++) {
      new_inst = tgsi_default_full_instruction();
      new_inst.Instruction.Opcode = TGSI_OPCODE_CMP;

      new_inst.Instruction.NumDstRegs = 1;
      new_inst.Dst[0].Register.File  = TGSI_FILE_TEMPORARY;
      new_inst.Dst[0].Register.Index = tmpbase + ctx->numtmp + i;
      new_inst.Dst[0].Register.WriteMask = TGSI_WRITEMASK_XYZW;

      new_inst.Instruction.NumSrcRegs = 3;
      new_inst.Src[0].Register.File  = TGSI_FILE_INPUT;
      new_inst.Src[0].Register.Index = ctx->face_idx;
      new_inst.Src[0].Register.SwizzleX = TGSI_SWIZZLE_X;
      new_inst.Src[0].Register.SwizzleY = TGSI_SWIZZLE_X;
      new_inst.Src[0].Register.SwizzleZ = TGSI_SWIZZLE_X;
      new_inst.Src[0].Register.SwizzleW = TGSI_SWIZZLE_X;
      new_inst.Src[1].Register.File  = TGSI_FILE_INPUT;
      new_inst.Src[1].Register.Index = inbase + i;
      new_inst.Src[1].Register.SwizzleX = TGSI_SWIZZLE_X;
      new_inst.Src[1].Register.SwizzleY = TGSI_SWIZZLE_Y;
      new_inst.Src[1].Register.SwizzleZ = TGSI_SWIZZLE_Z;
      new_inst.Src[1].Register.SwizzleW = TGSI_SWIZZLE_W;
      new_inst.Src[2].Register.File  = TGSI_FILE_INPUT;
      new_inst.Src[2].Register.Index = ctx->two_side_idx[i];
      new_inst.Src[2].Register.SwizzleX = TGSI_SWIZZLE_X;
      new_inst.Src[2].Register.SwizzleY = TGSI_SWIZZLE_Y;
      new_inst.Src[2].Register.SwizzleZ = TGSI_SWIZZLE_Z;
      new_inst.Src[2].Register.SwizzleW = TGSI_SWIZZLE_W;

      tctx->emit_instruction(tctx, &new_inst);
   }
}

static void
emit_decls(struct tgsi_transform_context *tctx)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);
   struct tgsi_shader_info *info = ctx->info;
   struct tgsi_full_declaration decl;
   struct tgsi_full_immediate immed;
   unsigned tmpbase;
   unsigned i;

   tmpbase = info->file_max[TGSI_FILE_TEMPORARY] + 1;

   ctx->color_base = tmpbase + ctx->numtmp;

   /* declare immediate: */
   immed = tgsi_default_full_immediate();
   immed.Immediate.NrTokens = 1 + 4; /* one for the token itself */
   immed.u[0].Float = 0.0;
   immed.u[1].Float = 1.0;
   immed.u[2].Float = 128.0;
   immed.u[3].Float = 0.0;
   tctx->emit_immediate(tctx, &immed);

   ctx->imm.Register.File = TGSI_FILE_IMMEDIATE;
   ctx->imm.Register.Index = info->immediate_count;
   ctx->imm.Register.SwizzleX = TGSI_SWIZZLE_X;
   ctx->imm.Register.SwizzleY = TGSI_SWIZZLE_Y;
   ctx->imm.Register.SwizzleZ = TGSI_SWIZZLE_Z;
   ctx->imm.Register.SwizzleW = TGSI_SWIZZLE_W;

   /* declare temp regs: */
   for (i = 0; i < ctx->numtmp; i++) {
      decl = tgsi_default_full_declaration();
      decl.Declaration.File = TGSI_FILE_TEMPORARY;
      decl.Range.First = decl.Range.Last = tmpbase + i;
      tctx->emit_declaration(tctx, &decl);

      ctx->tmp[i].src.Register.File  = TGSI_FILE_TEMPORARY;
      ctx->tmp[i].src.Register.Index = tmpbase + i;
      ctx->tmp[i].src.Register.SwizzleX = TGSI_SWIZZLE_X;
      ctx->tmp[i].src.Register.SwizzleY = TGSI_SWIZZLE_Y;
      ctx->tmp[i].src.Register.SwizzleZ = TGSI_SWIZZLE_Z;
      ctx->tmp[i].src.Register.SwizzleW = TGSI_SWIZZLE_W;

      ctx->tmp[i].dst.Register.File  = TGSI_FILE_TEMPORARY;
      ctx->tmp[i].dst.Register.Index = tmpbase + i;
      ctx->tmp[i].dst.Register.WriteMask = TGSI_WRITEMASK_XYZW;
   }

   if (ctx->two_side_colors)
      emit_twoside(tctx);
}

static void
rename_color_inputs(struct tgsi_lowering_context *ctx,
                    struct tgsi_full_instruction *inst)
{
   unsigned i, j;
   for (i = 0; i < inst->Instruction.NumSrcRegs; i++) {
      struct tgsi_src_register *src = &inst->Src[i].Register;
      if (src->File == TGSI_FILE_INPUT) {
         for (j = 0; j < ctx->two_side_colors; j++) {
	    if (src->Index == (int)ctx->two_side_idx[j]) {
               src->File = TGSI_FILE_TEMPORARY;
               src->Index = ctx->color_base + j;
               break;
            }
         }
      }
   }

}

static void
transform_instr(struct tgsi_transform_context *tctx,
		struct tgsi_full_instruction *inst)
{
   struct tgsi_lowering_context *ctx = tgsi_lowering_context(tctx);

   if (!ctx->emitted_decls) {
      emit_decls(tctx);
      ctx->emitted_decls = 1;
   }

   /* if emulating two-sided-color, we need to re-write some
    * src registers:
    */
   if (ctx->two_side_colors)
      rename_color_inputs(ctx, inst);

   switch (inst->Instruction.Opcode) {
   case TGSI_OPCODE_DST:
      if (!ctx->config->lower_DST)
         goto skip;
      transform_dst(tctx, inst);
      break;
   case TGSI_OPCODE_LRP:
      if (!ctx->config->lower_LRP)
         goto skip;
      transform_lrp(tctx, inst);
      break;
   case TGSI_OPCODE_FRC:
      if (!ctx->config->lower_FRC)
         goto skip;
      transform_frc(tctx, inst);
      break;
   case TGSI_OPCODE_POW:
      if (!ctx->config->lower_POW)
         goto skip;
      transform_pow(tctx, inst);
      break;
   case TGSI_OPCODE_LIT:
      if (!ctx->config->lower_LIT)
         goto skip;
      transform_lit(tctx, inst);
      break;
   case TGSI_OPCODE_EXP:
      if (!ctx->config->lower_EXP)
         goto skip;
      transform_exp(tctx, inst);
      break;
   case TGSI_OPCODE_LOG:
      if (!ctx->config->lower_LOG)
         goto skip;
      transform_log(tctx, inst);
      break;
   case TGSI_OPCODE_DP4:
      if (!ctx->config->lower_DP4)
         goto skip;
      transform_dotp(tctx, inst);
      break;
   case TGSI_OPCODE_DP3:
      if (!ctx->config->lower_DP3)
         goto skip;
      transform_dotp(tctx, inst);
      break;
   case TGSI_OPCODE_DP2:
      if (!ctx->config->lower_DP2)
         goto skip;
      transform_dotp(tctx, inst);
      break;
   case TGSI_OPCODE_FLR:
      if (!ctx->config->lower_FLR)
         goto skip;
      transform_flr_ceil(tctx, inst);
      break;
   case TGSI_OPCODE_CEIL:
      if (!ctx->config->lower_CEIL)
         goto skip;
      transform_flr_ceil(tctx, inst);
      break;
   case TGSI_OPCODE_TRUNC:
      if (!ctx->config->lower_TRUNC)
         goto skip;
      transform_trunc(tctx, inst);
      break;
   case TGSI_OPCODE_TEX:
   case TGSI_OPCODE_TXP:
   case TGSI_OPCODE_TXB:
   case TGSI_OPCODE_TXB2:
   case TGSI_OPCODE_TXL:
      if (transform_samp(tctx, inst))
         goto skip;
      break;
   default:
   skip:
      tctx->emit_instruction(tctx, inst);
      break;
   }
}

/* returns NULL if no lowering required, else returns the new
 * tokens (which caller is required to free()).  In either case
 * returns the current info.
 */
const struct tgsi_token *
tgsi_transform_lowering(const struct tgsi_lowering_config *config,
                        const struct tgsi_token *tokens,
                        struct tgsi_shader_info *info)
{
   struct tgsi_lowering_context ctx;
   struct tgsi_token *newtoks;
   int newlen, numtmp;

   /* sanity check in case limit is ever increased: */
   STATIC_ASSERT((sizeof(config->saturate_s) * 8) >= PIPE_MAX_SAMPLERS);

   /* sanity check the lowering */
   assert(!(config->lower_FRC && (config->lower_FLR || config->lower_CEIL)));
   assert(!(config->lower_FRC && config->lower_TRUNC));

   memset(&ctx, 0, sizeof(ctx));
   ctx.base.transform_instruction = transform_instr;
   ctx.info = info;
   ctx.config = config;

   tgsi_scan_shader(tokens, info);

   /* if we are adding fragment shader support to emulate two-sided
    * color, then figure out the number of additional inputs we need
    * to create for BCOLOR's..
    */
   if ((info->processor == PIPE_SHADER_FRAGMENT) &&
       config->color_two_side) {
      int i;
      ctx.face_idx = -1;
      for (i = 0; i <= info->file_max[TGSI_FILE_INPUT]; i++) {
         if (info->input_semantic_name[i] == TGSI_SEMANTIC_COLOR)
            ctx.two_side_idx[ctx.two_side_colors++] = i;
         if (info->input_semantic_name[i] == TGSI_SEMANTIC_FACE)
            ctx.face_idx = i;
      }
   }

   ctx.saturate = config->saturate_r | config->saturate_s | config->saturate_t;

#define OPCS(x) ((config->lower_ ## x) ? info->opcode_count[TGSI_OPCODE_ ## x] : 0)
   /* if there are no instructions to lower, then we are done: */
   if (!(OPCS(DST) ||
         OPCS(LRP) ||
         OPCS(FRC) ||
         OPCS(POW) ||
         OPCS(LIT) ||
         OPCS(EXP) ||
         OPCS(LOG) ||
         OPCS(DP4) ||
         OPCS(DP3) ||
         OPCS(DP2) ||
         OPCS(FLR) ||
         OPCS(CEIL) ||
         OPCS(TRUNC) ||
         OPCS(TXP) ||
         ctx.two_side_colors ||
         ctx.saturate))
      return NULL;

#if 0  /* debug */
   _debug_printf("BEFORE:");
   tgsi_dump(tokens, 0);
#endif

   numtmp = 0;
   newlen = tgsi_num_tokens(tokens);
   if (OPCS(DST)) {
      newlen += DST_GROW * OPCS(DST);
      numtmp = MAX2(numtmp, DST_TMP);
   }
   if (OPCS(LRP)) {
      newlen += LRP_GROW * OPCS(LRP);
      numtmp = MAX2(numtmp, LRP_TMP);
   }
   if (OPCS(FRC)) {
      newlen += FRC_GROW * OPCS(FRC);
      numtmp = MAX2(numtmp, FRC_TMP);
   }
   if (OPCS(POW)) {
      newlen += POW_GROW * OPCS(POW);
      numtmp = MAX2(numtmp, POW_TMP);
   }
   if (OPCS(LIT)) {
      newlen += LIT_GROW * OPCS(LIT);
      numtmp = MAX2(numtmp, LIT_TMP);
   }
   if (OPCS(EXP)) {
      newlen += EXP_GROW * OPCS(EXP);
      numtmp = MAX2(numtmp, EXP_TMP);
   }
   if (OPCS(LOG)) {
      newlen += LOG_GROW * OPCS(LOG);
      numtmp = MAX2(numtmp, LOG_TMP);
   }
   if (OPCS(DP4)) {
      newlen += DP4_GROW * OPCS(DP4);
      numtmp = MAX2(numtmp, DOTP_TMP);
   }
   if (OPCS(DP3)) {
      newlen += DP3_GROW * OPCS(DP3);
      numtmp = MAX2(numtmp, DOTP_TMP);
   }
   if (OPCS(DP2)) {
      newlen += DP2_GROW * OPCS(DP2);
      numtmp = MAX2(numtmp, DOTP_TMP);
   }
   if (OPCS(FLR)) {
      newlen += FLR_GROW * OPCS(FLR);
      numtmp = MAX2(numtmp, FLR_TMP);
   }
   if (OPCS(CEIL)) {
      newlen += CEIL_GROW * OPCS(CEIL);
      numtmp = MAX2(numtmp, CEIL_TMP);
   }
   if (OPCS(TRUNC)) {
      newlen += TRUNC_GROW * OPCS(TRUNC);
      numtmp = MAX2(numtmp, TRUNC_TMP);
   }
   if (ctx.saturate || config->lower_TXP) {
      int n = 0;

      if (ctx.saturate) {
         n = info->opcode_count[TGSI_OPCODE_TEX] +
            info->opcode_count[TGSI_OPCODE_TXP] +
            info->opcode_count[TGSI_OPCODE_TXB] +
            info->opcode_count[TGSI_OPCODE_TXB2] +
            info->opcode_count[TGSI_OPCODE_TXL];
      } else if (config->lower_TXP) {
          n = info->opcode_count[TGSI_OPCODE_TXP];
      }

      newlen += SAMP_GROW * n;
      numtmp = MAX2(numtmp, SAMP_TMP);
   }

   /* specifically don't include two_side_colors temps in the count: */
   ctx.numtmp = numtmp;

   if (ctx.two_side_colors) {
      newlen += TWOSIDE_GROW(ctx.two_side_colors);
      /* note: we permanently consume temp regs, re-writing references
       * to IN.COLOR[n] to TEMP[m] (holding the output of of the CMP
       * instruction that selects which varying to use):
       */
      numtmp += ctx.two_side_colors;
   }

   newlen += 2 * numtmp;
   newlen += 5;        /* immediate */

   newtoks = tgsi_transform_shader(tokens, newlen, &ctx.base);
   if (!newtoks)
      return NULL;

   tgsi_scan_shader(newtoks, info);

#if 0  /* debug */
   _debug_printf("AFTER:");
   tgsi_dump(newtoks, 0);
#endif

   return newtoks;
}
