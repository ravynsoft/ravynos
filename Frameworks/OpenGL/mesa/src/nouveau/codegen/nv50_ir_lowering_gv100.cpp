/*
 * Copyright 2020 Red Hat Inc.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include "nv50_ir.h"
#include "nv50_ir_build_util.h"

#include "nv50_ir_target_nvc0.h"
#include "nv50_ir_lowering_gv100.h"

#include <limits>

namespace nv50_ir {

bool
GV100LegalizeSSA::handleCMP(Instruction *i)
{
   Value *pred = bld.getSSA(1, FILE_PREDICATE);

   bld.mkCmp(OP_SET, reverseCondCode(i->asCmp()->setCond), TYPE_U8, pred,
             i->sType, bld.mkImm(0), i->getSrc(2))->ftz = i->ftz;
   bld.mkOp3(OP_SELP, TYPE_U32, i->getDef(0), i->getSrc(0), i->getSrc(1), pred);
   return true;
}

// NIR deals with most of these for us, but codegen generates more in pointer
// calculations from other lowering passes.
bool
GV100LegalizeSSA::handleIADD64(Instruction *i)
{
   Value *carry = bld.getSSA(1, FILE_PREDICATE);
   Value *def[2] = { bld.getSSA(), bld.getSSA() };
   Value *src[2][2];

   for (int s = 0; s < 2; s++) {
      if (i->getSrc(s)->reg.size == 8) {
         bld.mkSplit(src[s], 4, i->getSrc(s));
      } else {
         src[s][0] = i->getSrc(s);
         src[s][1] = bld.mkImm(0);
      }
   }

   bld.mkOp2(OP_ADD, TYPE_U32, def[0], src[0][0], src[1][0])->
      setFlagsDef(1, carry);
   bld.mkOp2(OP_ADD, TYPE_U32, def[1], src[0][1], src[1][1])->
      setFlagsSrc(2, carry);
   bld.mkOp2(OP_MERGE, i->dType, i->getDef(0), def[0], def[1]);
   return true;
}

bool
GV100LegalizeSSA::handleIMAD_HIGH(Instruction *i)
{
   Value *def = bld.getSSA(8), *defs[2];
   Value *src2;

   if (i->srcExists(2) &&
       (!i->getSrc(2)->asImm() || i->getSrc(2)->asImm()->reg.data.u32)) {
      Value *src2s[2] = { bld.getSSA(), bld.getSSA() };
      bld.mkMov(src2s[0], bld.mkImm(0));
      bld.mkMov(src2s[1], i->getSrc(2));
      src2 = bld.mkOp2(OP_MERGE, TYPE_U64, bld.getSSA(8), src2s[0], src2s[1])->getDef(0);
   } else {
      src2 = bld.mkImm(0);
   }

   bld.mkOp3(OP_MAD, isSignedType(i->sType) ? TYPE_S64 : TYPE_U64, def,
             i->getSrc(0), i->getSrc(1), src2);

   bld.mkSplit(defs, 4, def);
   i->def(0).replace(defs[1], false);
   return true;
}

// XXX: We should be able to do this in GV100LoweringPass, but codegen messes
//      up somehow and swaps the condcode without swapping the sources.
//      - tests/spec/glsl-1.50/execution/geometry/primitive-id-in.shader_test
bool
GV100LegalizeSSA::handleIMNMX(Instruction *i)
{
   Value *pred = bld.getSSA(1, FILE_PREDICATE);

   bld.mkCmp(OP_SET, (i->op == OP_MIN) ? CC_LT : CC_GT, i->dType, pred,
             i->sType, i->getSrc(0), i->getSrc(1));
   bld.mkOp3(OP_SELP, i->dType, i->getDef(0), i->getSrc(0), i->getSrc(1), pred);
   return true;
}

bool
GV100LegalizeSSA::handleIMUL(Instruction *i)
{
   if (i->subOp == NV50_IR_SUBOP_MUL_HIGH)
      return handleIMAD_HIGH(i);

   bld.mkOp3(OP_MAD, i->dType, i->getDef(0), i->getSrc(0), i->getSrc(1),
             bld.mkImm(0));
   return true;
}

bool
GV100LegalizeSSA::handleLOP2(Instruction *i)
{
   uint8_t src0 = NV50_IR_SUBOP_LOP3_LUT_SRC0;
   uint8_t src1 = NV50_IR_SUBOP_LOP3_LUT_SRC1;
   uint8_t subOp;

   if (i->src(0).mod & Modifier(NV50_IR_MOD_NOT))
      src0 = ~src0;
   if (i->src(1).mod & Modifier(NV50_IR_MOD_NOT))
      src1 = ~src1;

   switch (i->op) {
   case OP_AND: subOp = src0 & src1; break;
   case OP_OR : subOp = src0 | src1; break;
   case OP_XOR: subOp = src0 ^ src1; break;
   default:
      unreachable("invalid LOP2 opcode");
   }

   bld.mkOp3(OP_LOP3_LUT, TYPE_U32, i->getDef(0), i->getSrc(0), i->getSrc(1),
             bld.mkImm(0))->subOp = subOp;
   return true;
}

bool
GV100LegalizeSSA::handleNOT(Instruction *i)
{
   bld.mkOp3(OP_LOP3_LUT, TYPE_U32, i->getDef(0), bld.mkImm(0), i->getSrc(0),
             bld.mkImm(0))->subOp = (uint8_t)~NV50_IR_SUBOP_LOP3_LUT_SRC1;
   return true;
}

bool
GV100LegalizeSSA::handlePREEX2(Instruction *i)
{
   i->def(0).replace(i->src(0), false);
   return true;
}

bool
GV100LegalizeSSA::handleQUADON(Instruction *i)
{
   bld.mkBMov(i->getDef(0), bld.mkTSVal(TS_MACTIVE));
   Instruction *b = bld.mkBMov(bld.mkTSVal(TS_PQUAD_MACTIVE), i->getDef(0));
   b->fixed = 1;
   return true;
}

bool
GV100LegalizeSSA::handleQUADPOP(Instruction *i)
{
   Instruction *b = bld.mkBMov(bld.mkTSVal(TS_MACTIVE), i->getSrc(0));
   b->fixed = 1;
   return true;
}

bool
GV100LegalizeSSA::handleSET(Instruction *i)
{
   Value *src2 = i->srcExists(2) ? i->getSrc(2) : NULL;
   Value *pred = bld.getSSA(1, FILE_PREDICATE), *met;
   Instruction *xsetp;

   if (isFloatType(i->dType)) {
      if (i->sType == TYPE_F32)
         return false; // HW has FSET.BF
      met = bld.mkImm(0x3f800000);
   } else {
      met = bld.mkImm(0xffffffff);
   }

   xsetp = bld.mkCmp(i->op, i->asCmp()->setCond, TYPE_U8, pred, i->sType,
                     i->getSrc(0), i->getSrc(1));
   xsetp->src(0).mod = i->src(0).mod;
   xsetp->src(1).mod = i->src(1).mod;
   xsetp->setSrc(2, src2);
   xsetp->ftz = i->ftz;

   i = bld.mkOp3(OP_SELP, TYPE_U32, i->getDef(0), bld.mkImm(0), met, pred);
   i->src(2).mod = Modifier(NV50_IR_MOD_NOT);
   return true;
}

bool
GV100LegalizeSSA::handleSHFL(Instruction *i)
{
   Instruction *sync = new_Instruction(func, OP_WARPSYNC, TYPE_NONE);
   sync->fixed = 1;
   sync->setSrc(0, bld.mkImm(0xffffffff));
   i->bb->insertBefore(i, sync);
   return false;
}

bool
GV100LegalizeSSA::handleShift(Instruction *i)
{
   Value *zero = bld.mkImm(0);
   Value *src1 = i->getSrc(1);
   Value *src0, *src2;
   uint8_t subOp = i->op == OP_SHL ? NV50_IR_SUBOP_SHF_L : NV50_IR_SUBOP_SHF_R;

   if (i->op == OP_SHL && i->src(0).getFile() == FILE_GPR) {
      src0 = i->getSrc(0);
      src2 = zero;
   } else {
      src0 = zero;
      src2 = i->getSrc(0);
      subOp |= NV50_IR_SUBOP_SHF_HI;
   }
   if (i->subOp & NV50_IR_SUBOP_SHIFT_WRAP)
      subOp |= NV50_IR_SUBOP_SHF_W;

   bld.mkOp3(OP_SHF, i->dType, i->getDef(0), src0, src1, src2)->subOp = subOp;
   return true;
}

bool
GV100LegalizeSSA::handleSUB(Instruction *i)
{
   Instruction *xadd =
      bld.mkOp2(OP_ADD, i->dType, i->getDef(0), i->getSrc(0), i->getSrc(1));
   xadd->src(0).mod = i->src(0).mod;
   xadd->src(1).mod = i->src(1).mod ^ Modifier(NV50_IR_MOD_NEG);
   xadd->ftz = i->ftz;
   return true;
}

bool
GV100LegalizeSSA::visit(Instruction *i)
{
   bool lowered = false;

   bld.setPosition(i, false);
   if (i->sType == TYPE_F32 && i->dType != TYPE_F16 &&
       prog->getType() != Program::TYPE_COMPUTE)
      handleFTZ(i);

   switch (i->op) {
   case OP_AND:
   case OP_OR:
   case OP_XOR:
      if (i->def(0).getFile() != FILE_PREDICATE)
         lowered = handleLOP2(i);
      break;
   case OP_NOT:
      lowered = handleNOT(i);
      break;
   case OP_SHL:
   case OP_SHR:
      lowered = handleShift(i);
      break;
   case OP_SET:
   case OP_SET_AND:
   case OP_SET_OR:
   case OP_SET_XOR:
      if (i->def(0).getFile() != FILE_PREDICATE)
         lowered = handleSET(i);
      break;
   case OP_SLCT:
      lowered = handleCMP(i);
      break;
   case OP_PREEX2:
      lowered = handlePREEX2(i);
      break;
   case OP_MUL:
      if (!isFloatType(i->dType))
         lowered = handleIMUL(i);
      break;
   case OP_MAD:
      if (!isFloatType(i->dType) && i->subOp == NV50_IR_SUBOP_MUL_HIGH)
         lowered = handleIMAD_HIGH(i);
      break;
   case OP_SHFL:
      lowered = handleSHFL(i);
      break;
   case OP_QUADON:
      lowered = handleQUADON(i);
      break;
   case OP_QUADPOP:
      lowered = handleQUADPOP(i);
      break;
   case OP_SUB:
      lowered = handleSUB(i);
      break;
   case OP_MAX:
   case OP_MIN:
      if (!isFloatType(i->dType))
         lowered = handleIMNMX(i);
      break;
   case OP_ADD:
      if (!isFloatType(i->dType) && typeSizeof(i->dType) == 8)
         lowered = handleIADD64(i);
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

   if (lowered)
      delete_Instruction(prog, i);

   return true;
}

bool
GV100LoweringPass::handleDMNMX(Instruction *i)
{
   Value *pred = bld.getSSA(1, FILE_PREDICATE);
   Value *src0[2], *src1[2], *dest[2];

   bld.mkCmp(OP_SET, (i->op == OP_MIN) ? CC_LT : CC_GT, TYPE_U32, pred,
             i->sType, i->getSrc(0), i->getSrc(1));
   bld.mkSplit(src0, 4, i->getSrc(0));
   bld.mkSplit(src1, 4, i->getSrc(1));
   bld.mkSplit(dest, 4, i->getDef(0));
   bld.mkOp3(OP_SELP, TYPE_U32, dest[0], src0[0], src1[0], pred);
   bld.mkOp3(OP_SELP, TYPE_U32, dest[1], src0[1], src1[1], pred);
   bld.mkOp2(OP_MERGE, TYPE_U64, i->getDef(0), dest[0], dest[1]);
   return true;
}

bool
GV100LoweringPass::handleEXTBF(Instruction *i)
{
   Value *bit = bld.getScratch();
   Value *cnt = bld.getScratch();
   Value *mask = bld.getScratch();
   Value *zero = bld.mkImm(0);

   bld.mkOp3(OP_PERMT, TYPE_U32, bit, i->getSrc(1), bld.mkImm(0x4440), zero);
   bld.mkOp3(OP_PERMT, TYPE_U32, cnt, i->getSrc(1), bld.mkImm(0x4441), zero);
   bld.mkOp2(OP_BMSK, TYPE_U32, mask, bit, cnt);
   bld.mkOp2(OP_AND, TYPE_U32, mask, i->getSrc(0), mask);
   bld.mkOp2(OP_SHR, TYPE_U32, i->getDef(0), mask, bit);
   if (isSignedType(i->dType))
      bld.mkOp2(OP_SGXT, TYPE_S32, i->getDef(0), i->getDef(0), cnt);

   return true;
}

bool
GV100LoweringPass::handleFLOW(Instruction *i)
{
   i->op = OP_BRA;
   return false;
}

bool
GV100LoweringPass::handleI2I(Instruction *i)
{
   bld.mkCvt(OP_CVT, TYPE_F32, i->getDef(0), i->sType, i->getSrc(0))->
      subOp = i->subOp;
   bld.mkCvt(OP_CVT, i->dType, i->getDef(0), TYPE_F32, i->getDef(0));
   return true;
}

bool
GV100LoweringPass::handleINSBF(Instruction *i)
{
   Value *bit = bld.getScratch();
   Value *cnt = bld.getScratch();
   Value *mask = bld.getScratch();
   Value *src0 = bld.getScratch();
   Value *zero = bld.mkImm(0);

   bld.mkOp3(OP_PERMT, TYPE_U32, bit, i->getSrc(1), bld.mkImm(0x4440), zero);
   bld.mkOp3(OP_PERMT, TYPE_U32, cnt, i->getSrc(1), bld.mkImm(0x4441), zero);
   bld.mkOp2(OP_BMSK, TYPE_U32, mask, zero, cnt);

   bld.mkOp2(OP_AND, TYPE_U32, src0, i->getSrc(0), mask);
   bld.mkOp2(OP_SHL, TYPE_U32, src0, src0, bit);

   bld.mkOp2(OP_SHL, TYPE_U32, mask, mask, bit);
   bld.mkOp3(OP_LOP3_LUT, TYPE_U32, i->getDef(0), src0, i->getSrc(2), mask)->
      subOp = NV50_IR_SUBOP_LOP3_LUT(a | (b & ~c));

   return true;
}

bool
GV100LoweringPass::handlePINTERP(Instruction *i)
{
   Value *src2 = i->srcExists(2) ? i->getSrc(2) : NULL;
   Instruction *ipa, *mul;

   ipa = bld.mkOp2(OP_LINTERP, TYPE_F32, i->getDef(0), i->getSrc(0), src2);
   ipa->ipa = i->ipa;
   mul = bld.mkOp2(OP_MUL, TYPE_F32, i->getDef(0), i->getDef(0), i->getSrc(1));

   if (i->getInterpMode() == NV50_IR_INTERP_SC) {
      ipa->setDef(1, bld.getSSA(1, FILE_PREDICATE));
      mul->setPredicate(CC_NOT_P, ipa->getDef(1));
   }

   return true;
}

bool
GV100LoweringPass::handlePREFLOW(Instruction *i)
{
   return true;
}

bool
GV100LoweringPass::handlePRESIN(Instruction *i)
{
   const float f = 1.0 / (2.0 * 3.14159265);
   bld.mkOp2(OP_MUL, i->dType, i->getDef(0), i->getSrc(0), bld.mkImm(f));
   return true;
}

bool
GV100LoweringPass::visit(Instruction *i)
{
   bool lowered = false;

   bld.setPosition(i, false);

   switch (i->op) {
   case OP_BREAK:
   case OP_CONT:
      lowered = handleFLOW(i);
      break;
   case OP_PREBREAK:
   case OP_PRECONT:
      lowered = handlePREFLOW(i);
      break;
   case OP_CVT:
      if (i->src(0).getFile() != FILE_PREDICATE &&
          i->def(0).getFile() != FILE_PREDICATE &&
          !isFloatType(i->dType) && !isFloatType(i->sType))
         lowered = handleI2I(i);
      break;
   case OP_EXTBF:
      lowered = handleEXTBF(i);
      break;
   case OP_INSBF:
      lowered = handleINSBF(i);
      break;
   case OP_MAX:
   case OP_MIN:
      if (i->dType == TYPE_F64)
         lowered = handleDMNMX(i);
      break;
   case OP_PINTERP:
      lowered = handlePINTERP(i);
      break;
   case OP_PRESIN:
      lowered = handlePRESIN(i);
      break;
   default:
      break;
   }

   if (lowered)
      delete_Instruction(prog, i);

   return true;
}

} // namespace nv50_ir
