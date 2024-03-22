/*
 * Copyright 2011 Christoph Bumiller
 *           2014 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nv50_ir.h"
#include "nv50_ir_build_util.h"

#include "nv50_ir_target_nvc0.h"
#include "nv50_ir_lowering_gm107.h"

#include <limits>

namespace nv50_ir {

#define QOP_ADD  0
#define QOP_SUBR 1
#define QOP_SUB  2
#define QOP_MOV2 3

//             UL UR LL LR
#define QUADOP(q, r, s, t)                      \
   ((QOP_##q << 6) | (QOP_##r << 4) |           \
    (QOP_##s << 2) | (QOP_##t << 0))

#define SHFL_BOUND_QUAD 0x1c03

void
GM107LegalizeSSA::handlePFETCH(Instruction *i)
{
   Value *src0;

   if (i->src(0).getFile() == FILE_GPR && !i->srcExists(1))
      return;

   bld.setPosition(i, false);
   src0 = bld.getSSA();

   if (i->srcExists(1))
      bld.mkOp2(OP_ADD , TYPE_U32, src0, i->getSrc(0), i->getSrc(1));
   else
      bld.mkOp1(OP_MOV , TYPE_U32, src0, i->getSrc(0));

   i->setSrc(0, src0);
   i->setSrc(1, NULL);
}

void
GM107LegalizeSSA::handleLOAD(Instruction *i)
{
   if (i->src(0).getFile() != FILE_MEMORY_CONST)
      return;
   if (i->src(0).isIndirect(0))
      return;
   if (typeSizeof(i->dType) != 4)
      return;

   i->op = OP_MOV;
}

void
GM107LegalizeSSA::handleQUADON(Instruction *i)
{
   i->setDef(0, NULL);
}

void
GM107LegalizeSSA::handleQUADPOP(Instruction *i)
{
   i->setSrc(0, NULL);
}

bool
GM107LegalizeSSA::visit(Instruction *i)
{
   switch (i->op) {
   case OP_QUADON:
      handleQUADON(i);
      break;
   case OP_QUADPOP:
      handleQUADPOP(i);
      break;
   case OP_PFETCH:
      handlePFETCH(i);
      break;
   case OP_LOAD:
      handleLOAD(i);
      break;
   default:
      break;
   }
   return true;
}

bool
GM107LoweringPass::handleManualTXD(TexInstruction *i)
{
   // See NVC0LoweringPass::handleManualTXD for rationale. This function
   // implements the same logic, but using SM50-friendly primitives.
   static const uint8_t qOps[2] =
      { QUADOP(MOV2, ADD,  MOV2, ADD),  QUADOP(MOV2, MOV2, ADD,  ADD) };
   Value *def[4][4];
   Value *crd[3], *arr, *shadow;
   Value *tmp;
   Instruction *tex, *add;
   Value *quad = bld.mkImm(SHFL_BOUND_QUAD);
   int l, c;
   const int dim = i->tex.target.getDim() + i->tex.target.isCube();
   const int array = i->tex.target.isArray();
   const int indirect = i->tex.rIndirectSrc >= 0;

   i->op = OP_TEX; // no need to clone dPdx/dPdy later

   for (c = 0; c < dim; ++c)
      crd[c] = bld.getScratch();
   arr = bld.getScratch();
   shadow = bld.getScratch();
   tmp = bld.getScratch();

   for (l = 0; l < 4; ++l) {
      Value *bar = bld.getSSA(4, FILE_BARRIER);
      Value *src[3], *val;
      Value *lane = bld.mkImm(l);
      bld.mkOp(OP_QUADON, TYPE_U32, bar);
      // Make sure lane 0 has the appropriate array/depth compare values
      if (l != 0) {
         if (array)
            bld.mkOp3(OP_SHFL, TYPE_F32, arr, i->getSrc(0), lane, quad);
         if (i->tex.target.isShadow())
            bld.mkOp3(OP_SHFL, TYPE_F32, shadow, i->getSrc(array + dim + indirect), lane, quad);
      }

      // mov coordinates from lane l to all lanes
      for (c = 0; c < dim; ++c) {
         bld.mkOp3(OP_SHFL, TYPE_F32, crd[c], i->getSrc(c + array), lane, quad);
      }

      // add dPdx from lane l to lanes dx
      for (c = 0; c < dim; ++c) {
         bld.mkOp3(OP_SHFL, TYPE_F32, tmp, i->dPdx[c].get(), lane, quad);
         add = bld.mkOp2(OP_QUADOP, TYPE_F32, crd[c], tmp, crd[c]);
         add->subOp = qOps[0];
         add->lanes = 1; /* abused for .ndv */
      }

      // add dPdy from lane l to lanes dy
      for (c = 0; c < dim; ++c) {
         bld.mkOp3(OP_SHFL, TYPE_F32, tmp, i->dPdy[c].get(), lane, quad);
         add = bld.mkOp2(OP_QUADOP, TYPE_F32, crd[c], tmp, crd[c]);
         add->subOp = qOps[1];
         add->lanes = 1; /* abused for .ndv */
      }

      // normalize cube coordinates if necessary
      if (i->tex.target.isCube()) {
         for (c = 0; c < 3; ++c)
            src[c] = bld.mkOp1v(OP_ABS, TYPE_F32, bld.getSSA(), crd[c]);
         val = bld.getScratch();
         bld.mkOp2(OP_MAX, TYPE_F32, val, src[0], src[1]);
         bld.mkOp2(OP_MAX, TYPE_F32, val, src[2], val);
         bld.mkOp1(OP_RCP, TYPE_F32, val, val);
         for (c = 0; c < 3; ++c)
            src[c] = bld.mkOp2v(OP_MUL, TYPE_F32, bld.getSSA(), crd[c], val);
      } else {
         for (c = 0; c < dim; ++c)
            src[c] = crd[c];
      }

      // texture
      bld.insert(tex = cloneForward(func, i));
      if (l != 0) {
         if (array)
            tex->setSrc(0, arr);
         if (i->tex.target.isShadow())
            tex->setSrc(array + dim + indirect, shadow);
      }
      for (c = 0; c < dim; ++c)
         tex->setSrc(c + array, src[c]);
      // broadcast results from lane 0 to all lanes
      if (l != 0)
         for (c = 0; i->defExists(c); ++c)
            bld.mkOp3(OP_SHFL, TYPE_F32, tex->getDef(c), tex->getDef(c), bld.mkImm(0), quad);
      bld.mkOp1(OP_QUADPOP, TYPE_U32, NULL, bar)->fixed = 1;

      // save results
      for (c = 0; i->defExists(c); ++c) {
         Instruction *mov;
         def[c][l] = bld.getSSA();
         mov = bld.mkMov(def[c][l], tex->getDef(c));
         mov->fixed = 1;
         mov->lanes = 1 << l;
      }
   }

   for (c = 0; i->defExists(c); ++c) {
      Instruction *u = bld.mkOp(OP_UNION, TYPE_U32, i->getDef(c));
      for (l = 0; l < 4; ++l)
         u->setSrc(l, def[c][l]);
   }

   i->bb->remove(i);
   return true;
}

bool
GM107LoweringPass::handleDFDX(Instruction *insn)
{
   Instruction *shfl;
   int qop = 0, xid = 0;

   switch (insn->op) {
   case OP_DFDX:
      qop = QUADOP(SUB, SUBR, SUB, SUBR);
      xid = 1;
      break;
   case OP_DFDY:
      qop = QUADOP(SUB, SUB, SUBR, SUBR);
      xid = 2;
      break;
   default:
      assert(!"invalid dfdx opcode");
      break;
   }

   shfl = bld.mkOp3(OP_SHFL, TYPE_F32, bld.getScratch(), insn->getSrc(0),
                    bld.mkImm(xid), bld.mkImm(SHFL_BOUND_QUAD));
   shfl->subOp = NV50_IR_SUBOP_SHFL_BFLY;
   insn->op = OP_QUADOP;
   insn->subOp = qop;
   insn->lanes = 0; /* abused for !.ndv */
   insn->setSrc(1, insn->getSrc(0));
   insn->setSrc(0, shfl->getDef(0));
   return true;
}

bool
GM107LoweringPass::handlePFETCH(Instruction *i)
{
   Value *tmp0 = bld.getScratch();
   Value *tmp1 = bld.getScratch();
   Value *tmp2 = bld.getScratch();
   bld.mkOp1(OP_RDSV, TYPE_U32, tmp0, bld.mkSysVal(SV_INVOCATION_INFO, 0));
   bld.mkOp3(OP_PERMT, TYPE_U32, tmp1, tmp0, bld.mkImm(0x4442), bld.mkImm(0));
   bld.mkOp3(OP_PERMT, TYPE_U32, tmp0, tmp0, bld.mkImm(0x4440), bld.mkImm(0));
   if (i->getSrc(1))
      bld.mkOp2(OP_ADD , TYPE_U32, tmp2, i->getSrc(0), i->getSrc(1));
   else
      bld.mkOp1(OP_MOV , TYPE_U32, tmp2, i->getSrc(0));
   bld.mkOp3(OP_MAD , TYPE_U32, tmp0, tmp0, tmp1, tmp2);
   i->setSrc(0, tmp0);
   i->setSrc(1, NULL);
   return true;
}

bool
GM107LoweringPass::handlePOPCNT(Instruction *i)
{
   Value *tmp = bld.mkOp2v(OP_AND, i->sType, bld.getScratch(),
                           i->getSrc(0), i->getSrc(1));
   i->setSrc(0, tmp);
   i->setSrc(1, NULL);
   return true;
}

bool
GM107LoweringPass::handleSUQ(TexInstruction *suq)
{
   Value *ind = suq->getIndirectR();
   Value *handle;
   const int slot = suq->tex.r;
   const int mask = suq->tex.mask;

   if (suq->tex.bindless)
      handle = ind;
   else
      handle = loadTexHandle(ind, slot + 32);

   suq->tex.r = 0xff;
   suq->tex.s = 0x1f;

   suq->setIndirectR(NULL);
   suq->setSrc(0, handle);
   suq->tex.rIndirectSrc = 0;
   suq->setSrc(1, bld.loadImm(NULL, 0));
   suq->tex.query = TXQ_DIMS;
   suq->op = OP_TXQ;

   // We store CUBE / CUBE_ARRAY as a 2D ARRAY. Make sure that depth gets
   // divided by 6.
   if (mask & 0x4 && suq->tex.target.isCube()) {
      int d = util_bitcount(mask & 0x3);
      bld.setPosition(suq, true);
      bld.mkOp2(OP_DIV, TYPE_U32, suq->getDef(d), suq->getDef(d),
                bld.loadImm(NULL, 6));
   }

   // Samples come from a different query. If we want both samples and dims,
   // create a second suq.
   if (mask & 0x8) {
      int d = util_bitcount(mask & 0x7);
      Value *dst = suq->getDef(d);
      TexInstruction *samples = suq;
      assert(dst);

      if (mask != 0x8) {
         suq->setDef(d, NULL);
         suq->tex.mask &= 0x7;
         samples = cloneShallow(func, suq);
         for (int i = 0; i < d; i++)
            samples->setDef(d, NULL);
         samples->setDef(0, dst);
         suq->bb->insertAfter(suq, samples);
      }
      samples->tex.mask = 0x4;
      samples->tex.query = TXQ_TYPE;
   }

   if (suq->tex.target.isMS()) {
      bld.setPosition(suq, true);

      if (mask & 0x1)
         bld.mkOp2(OP_SHR, TYPE_U32, suq->getDef(0), suq->getDef(0),
                   loadMsAdjInfo32(suq->tex.target, 0, slot, ind, suq->tex.bindless));
      if (mask & 0x2) {
         int d = util_bitcount(mask & 0x1);
         bld.mkOp2(OP_SHR, TYPE_U32, suq->getDef(d), suq->getDef(d),
                   loadMsAdjInfo32(suq->tex.target, 1, slot, ind, suq->tex.bindless));
      }
   }

   return true;
}

//
// - add quadop dance for texturing
// - put FP outputs in GPRs
// - convert instruction sequences
//
bool
GM107LoweringPass::visit(Instruction *i)
{
   bld.setPosition(i, false);

   if (i->cc != CC_ALWAYS)
      checkPredicate(i);

   switch (i->op) {
   case OP_PFETCH:
      return handlePFETCH(i);
   case OP_DFDX:
   case OP_DFDY:
      return handleDFDX(i);
   case OP_POPCNT:
      return handlePOPCNT(i);
   case OP_SUQ:
      return handleSUQ(i->asTex());
   default:
      return NVC0LoweringPass::visit(i);
   }
}

} // namespace nv50_ir
