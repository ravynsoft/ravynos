/*
 * Copyright 2012 Christoph Bumiller
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

#include "nv50_ir_target_nvc0.h"

// CodeEmitter for GK110 encoding of the Fermi/Kepler ISA.

namespace nv50_ir {

class CodeEmitterGK110 : public CodeEmitter
{
public:
   CodeEmitterGK110(const TargetNVC0 *, Program::Type);

   virtual bool emitInstruction(Instruction *);
   virtual uint32_t getMinEncodingSize(const Instruction *) const;
   virtual void prepareEmission(Function *);

private:
   const TargetNVC0 *targNVC0;

   Program::Type progType;

   const bool writeIssueDelays;

private:
   void emitForm_21(const Instruction *, uint32_t opc2, uint32_t opc1);
   void emitForm_C(const Instruction *, uint32_t opc, uint8_t ctg);
   void emitForm_L(const Instruction *, uint32_t opc, uint8_t ctg, Modifier, int sCount = 3);

   void emitPredicate(const Instruction *);

   void setCAddress14(const ValueRef&);
   void setShortImmediate(const Instruction *, const int s);
   void setImmediate32(const Instruction *, const int s, Modifier);
   void setSUConst16(const Instruction *, const int s);

   void modNegAbsF32_3b(const Instruction *, const int s);

   void emitCondCode(CondCode cc, int pos, uint8_t mask);
   void emitInterpMode(const Instruction *);
   void emitLoadStoreType(DataType ty, const int pos);
   void emitCachingMode(CacheMode c, const int pos);
   void emitSUGType(DataType, const int pos);
   void emitSUCachingMode(CacheMode c);

   inline uint8_t getSRegEncoding(const ValueRef&);

   void emitRoundMode(RoundMode, const int pos, const int rintPos);
   void emitRoundModeF(RoundMode, const int pos);
   void emitRoundModeI(RoundMode, const int pos);

   void emitNegAbs12(const Instruction *);

   void emitNOP(const Instruction *);

   void emitLOAD(const Instruction *);
   void emitSTORE(const Instruction *);
   void emitMOV(const Instruction *);
   void emitATOM(const Instruction *);
   void emitCCTL(const Instruction *);

   void emitINTERP(const Instruction *);
   void emitAFETCH(const Instruction *);
   void emitPFETCH(const Instruction *);
   void emitVFETCH(const Instruction *);
   void emitEXPORT(const Instruction *);
   void emitOUT(const Instruction *);

   void emitUADD(const Instruction *);
   void emitFADD(const Instruction *);
   void emitDADD(const Instruction *);
   void emitIMUL(const Instruction *);
   void emitFMUL(const Instruction *);
   void emitDMUL(const Instruction *);
   void emitIMAD(const Instruction *);
   void emitISAD(const Instruction *);
   void emitSHLADD(const Instruction *);
   void emitFMAD(const Instruction *);
   void emitDMAD(const Instruction *);
   void emitMADSP(const Instruction *i);

   void emitNOT(const Instruction *);
   void emitLogicOp(const Instruction *, uint8_t subOp);
   void emitPOPC(const Instruction *);
   void emitINSBF(const Instruction *);
   void emitEXTBF(const Instruction *);
   void emitBFIND(const Instruction *);
   void emitPERMT(const Instruction *);
   void emitShift(const Instruction *);
   void emitShift64(const Instruction *);

   void emitSFnOp(const Instruction *, uint8_t subOp);

   void emitCVT(const Instruction *);
   void emitMINMAX(const Instruction *);
   void emitPreOp(const Instruction *);

   void emitSET(const CmpInstruction *);
   void emitSLCT(const CmpInstruction *);
   void emitSELP(const Instruction *);

   void emitTEXBAR(const Instruction *);
   void emitTEX(const TexInstruction *);
   void emitTEXCSAA(const TexInstruction *);
   void emitTXQ(const TexInstruction *);

   void emitQUADOP(const Instruction *, uint8_t qOp, uint8_t laneMask);

   void emitPIXLD(const Instruction *);

   void emitBAR(const Instruction *);
   void emitMEMBAR(const Instruction *);

   void emitFlow(const Instruction *);

   void emitSHFL(const Instruction *);

   void emitVOTE(const Instruction *);

   void emitSULDGB(const TexInstruction *);
   void emitSUSTGx(const TexInstruction *);
   void emitSUCLAMPMode(uint16_t);
   void emitSUCalc(Instruction *);

   void emitVSHL(const Instruction *);
   void emitVectorSubOp(const Instruction *);

   inline void defId(const ValueDef&, const int pos);
   inline void srcId(const ValueRef&, const int pos);
   inline void srcId(const ValueRef *, const int pos);
   inline void srcId(const Instruction *, int s, const int pos);

   inline void srcAddr32(const ValueRef&, const int pos); // address / 4

   inline bool isLIMM(const ValueRef&, DataType ty, bool mod = false);
};

#define GK110_GPR_ZERO 255

#define NEG_(b, s) \
   if (i->src(s).mod.neg()) code[(0x##b) / 32] |= 1 << ((0x##b) % 32)
#define ABS_(b, s) \
   if (i->src(s).mod.abs()) code[(0x##b) / 32] |= 1 << ((0x##b) % 32)

#define NOT_(b, s) if (i->src(s).mod & Modifier(NV50_IR_MOD_NOT))       \
   code[(0x##b) / 32] |= 1 << ((0x##b) % 32)

#define FTZ_(b) if (i->ftz) code[(0x##b) / 32] |= 1 << ((0x##b) % 32)
#define DNZ_(b) if (i->dnz) code[(0x##b) / 32] |= 1 << ((0x##b) % 32)

#define SAT_(b) if (i->saturate) code[(0x##b) / 32] |= 1 << ((0x##b) % 32)

#define RND_(b, t) emitRoundMode##t(i->rnd, 0x##b)

#define SDATA(a) ((a).rep()->reg.data)
#define DDATA(a) ((a).rep()->reg.data)

void CodeEmitterGK110::srcId(const ValueRef& src, const int pos)
{
   code[pos / 32] |= (src.get() ? SDATA(src).id : GK110_GPR_ZERO) << (pos % 32);
}

void CodeEmitterGK110::srcId(const ValueRef *src, const int pos)
{
   code[pos / 32] |= (src ? SDATA(*src).id : GK110_GPR_ZERO) << (pos % 32);
}

void CodeEmitterGK110::srcId(const Instruction *insn, int s, int pos)
{
   int r = insn->srcExists(s) ? SDATA(insn->src(s)).id : GK110_GPR_ZERO;
   code[pos / 32] |= r << (pos % 32);
}

void CodeEmitterGK110::srcAddr32(const ValueRef& src, const int pos)
{
   code[pos / 32] |= (SDATA(src).offset >> 2) << (pos % 32);
}

void CodeEmitterGK110::defId(const ValueDef& def, const int pos)
{
   code[pos / 32] |= (def.get() && def.getFile() != FILE_FLAGS ? DDATA(def).id : GK110_GPR_ZERO) << (pos % 32);
}

bool CodeEmitterGK110::isLIMM(const ValueRef& ref, DataType ty, bool mod)
{
   const ImmediateValue *imm = ref.get()->asImm();

   if (ty == TYPE_F32)
      return imm && imm->reg.data.u32 & 0xfff;
   else
      return imm && (imm->reg.data.s32 > 0x7ffff ||
                     imm->reg.data.s32 < -0x80000);
}

void
CodeEmitterGK110::emitRoundMode(RoundMode rnd, const int pos, const int rintPos)
{
   bool rint = false;
   uint8_t n;

   switch (rnd) {
   case ROUND_MI: rint = true; FALLTHROUGH; case ROUND_M: n = 1; break;
   case ROUND_PI: rint = true; FALLTHROUGH; case ROUND_P: n = 2; break;
   case ROUND_ZI: rint = true; FALLTHROUGH; case ROUND_Z: n = 3; break;
   default:
      rint = rnd == ROUND_NI;
      n = 0;
      assert(rnd == ROUND_N || rnd == ROUND_NI);
      break;
   }
   code[pos / 32] |= n << (pos % 32);
   if (rint && rintPos >= 0)
      code[rintPos / 32] |= 1 << (rintPos % 32);
}

void
CodeEmitterGK110::emitRoundModeF(RoundMode rnd, const int pos)
{
   uint8_t n;

   switch (rnd) {
   case ROUND_M: n = 1; break;
   case ROUND_P: n = 2; break;
   case ROUND_Z: n = 3; break;
   default:
      n = 0;
      assert(rnd == ROUND_N);
      break;
   }
   code[pos / 32] |= n << (pos % 32);
}

void
CodeEmitterGK110::emitRoundModeI(RoundMode rnd, const int pos)
{
   uint8_t n;

   switch (rnd) {
   case ROUND_MI: n = 1; break;
   case ROUND_PI: n = 2; break;
   case ROUND_ZI: n = 3; break;
   default:
      n = 0;
      assert(rnd == ROUND_NI);
      break;
   }
   code[pos / 32] |= n << (pos % 32);
}

void CodeEmitterGK110::emitCondCode(CondCode cc, int pos, uint8_t mask)
{
   uint8_t n;

   switch (cc) {
   case CC_FL:  n = 0x00; break;
   case CC_LT:  n = 0x01; break;
   case CC_EQ:  n = 0x02; break;
   case CC_LE:  n = 0x03; break;
   case CC_GT:  n = 0x04; break;
   case CC_NE:  n = 0x05; break;
   case CC_GE:  n = 0x06; break;
   case CC_LTU: n = 0x09; break;
   case CC_EQU: n = 0x0a; break;
   case CC_LEU: n = 0x0b; break;
   case CC_GTU: n = 0x0c; break;
   case CC_NEU: n = 0x0d; break;
   case CC_GEU: n = 0x0e; break;
   case CC_TR:  n = 0x0f; break;
   case CC_NO:  n = 0x10; break;
   case CC_NC:  n = 0x11; break;
   case CC_NS:  n = 0x12; break;
   case CC_NA:  n = 0x13; break;
   case CC_A:   n = 0x14; break;
   case CC_S:   n = 0x15; break;
   case CC_C:   n = 0x16; break;
   case CC_O:   n = 0x17; break;
   default:
      n = 0;
      assert(!"invalid condition code");
      break;
   }
   code[pos / 32] |= (n & mask) << (pos % 32);
}

void
CodeEmitterGK110::emitPredicate(const Instruction *i)
{
   if (i->predSrc >= 0) {
      srcId(i->src(i->predSrc), 18);
      if (i->cc == CC_NOT_P)
         code[0] |= 8 << 18; // negate
      assert(i->getPredicate()->reg.file == FILE_PREDICATE);
   } else {
      code[0] |= 7 << 18;
   }
}

void
CodeEmitterGK110::setCAddress14(const ValueRef& src)
{
   const Storage& res = src.get()->asSym()->reg;
   const int32_t addr = res.data.offset / 4;

   code[0] |= (addr & 0x01ff) << 23;
   code[1] |= (addr & 0x3e00) >> 9;
   code[1] |= res.fileIndex << 5;
}

void
CodeEmitterGK110::setShortImmediate(const Instruction *i, const int s)
{
   const uint32_t u32 = i->getSrc(s)->asImm()->reg.data.u32;
   const uint64_t u64 = i->getSrc(s)->asImm()->reg.data.u64;

   if (i->sType == TYPE_F32) {
      assert(!(u32 & 0x00000fff));
      code[0] |= ((u32 & 0x001ff000) >> 12) << 23;
      code[1] |= ((u32 & 0x7fe00000) >> 21);
      code[1] |= ((u32 & 0x80000000) >> 4);
   } else
   if (i->sType == TYPE_F64) {
      assert(!(u64 & 0x00000fffffffffffULL));
      code[0] |= ((u64 & 0x001ff00000000000ULL) >> 44) << 23;
      code[1] |= ((u64 & 0x7fe0000000000000ULL) >> 53);
      code[1] |= ((u64 & 0x8000000000000000ULL) >> 36);
   } else {
      assert((u32 & 0xfff80000) == 0 || (u32 & 0xfff80000) == 0xfff80000);
      code[0] |= (u32 & 0x001ff) << 23;
      code[1] |= (u32 & 0x7fe00) >> 9;
      code[1] |= (u32 & 0x80000) << 8;
   }
}

void
CodeEmitterGK110::setImmediate32(const Instruction *i, const int s,
                                 Modifier mod)
{
   uint32_t u32 = i->getSrc(s)->asImm()->reg.data.u32;

   if (mod) {
      ImmediateValue imm(i->getSrc(s)->asImm(), i->sType);
      mod.applyTo(imm);
      u32 = imm.reg.data.u32;
   }

   code[0] |= u32 << 23;
   code[1] |= u32 >> 9;
}

void
CodeEmitterGK110::emitForm_L(const Instruction *i, uint32_t opc, uint8_t ctg,
                             Modifier mod, int sCount)
{
   code[0] = ctg;
   code[1] = opc << 20;

   emitPredicate(i);

   defId(i->def(0), 2);

   for (int s = 0; s < sCount && i->srcExists(s); ++s) {
      switch (i->src(s).getFile()) {
      case FILE_GPR:
         srcId(i->src(s), s ? 42 : 10);
         break;
      case FILE_IMMEDIATE:
         setImmediate32(i, s, mod);
         break;
      default:
         break;
      }
   }
}


void
CodeEmitterGK110::emitForm_C(const Instruction *i, uint32_t opc, uint8_t ctg)
{
   code[0] = ctg;
   code[1] = opc << 20;

   emitPredicate(i);

   defId(i->def(0), 2);

   switch (i->src(0).getFile()) {
   case FILE_MEMORY_CONST:
      code[1] |= 0x4 << 28;
      setCAddress14(i->src(0));
      break;
   case FILE_GPR:
      code[1] |= 0xc << 28;
      srcId(i->src(0), 23);
      break;
   default:
      assert(0);
      break;
   }
}

// 0x2 for GPR, c[] and 0x1 for short immediate
void
CodeEmitterGK110::emitForm_21(const Instruction *i, uint32_t opc2,
                              uint32_t opc1)
{
   const bool imm = i->srcExists(1) && i->src(1).getFile() == FILE_IMMEDIATE;

   int s1 = 23;
   if (i->srcExists(2) && i->src(2).getFile() == FILE_MEMORY_CONST)
      s1 = 42;

   if (imm) {
      code[0] = 0x1;
      code[1] = opc1 << 20;
   } else {
      code[0] = 0x2;
      code[1] = (0xc << 28) | (opc2 << 20);
   }

   emitPredicate(i);

   defId(i->def(0), 2);

   for (int s = 0; s < 3 && i->srcExists(s); ++s) {
      switch (i->src(s).getFile()) {
      case FILE_MEMORY_CONST:
         code[1] &= (s == 2) ? ~(0x4 << 28) : ~(0x8 << 28);
         setCAddress14(i->src(s));
         break;
      case FILE_IMMEDIATE:
         setShortImmediate(i, s);
         break;
      case FILE_GPR:
         srcId(i->src(s), s ? ((s == 2) ? 42 : s1) : 10);
         break;
      default:
         if (i->op == OP_SELP) {
            assert(s == 2 && i->src(s).getFile() == FILE_PREDICATE);
            srcId(i->src(s), 42);
         }
         // ignore here, can be predicate or flags, but must not be address
         break;
      }
   }
   // 0x0 = invalid
   // 0xc = rrr
   // 0x8 = rrc
   // 0x4 = rcr
   assert(imm || (code[1] & (0xc << 28)));
}

inline void
CodeEmitterGK110::modNegAbsF32_3b(const Instruction *i, const int s)
{
   if (i->src(s).mod.abs()) code[1] &= ~(1 << 27);
   if (i->src(s).mod.neg()) code[1] ^=  (1 << 27);
}

void
CodeEmitterGK110::emitNOP(const Instruction *i)
{
   code[0] = 0x00003c02;
   code[1] = 0x85800000;

   if (i)
      emitPredicate(i);
   else
      code[0] = 0x001c3c02;
}

void
CodeEmitterGK110::emitFMAD(const Instruction *i)
{
   bool neg1 = (i->src(0).mod ^ i->src(1).mod).neg();

   if (isLIMM(i->src(1), TYPE_F32)) {
      assert(i->getDef(0)->reg.data.id == i->getSrc(2)->reg.data.id);

      // last source is dst, so force 2 sources
      emitForm_L(i, 0x600, 0x0, 0, 2);

      if (i->flagsDef >= 0)
         code[1] |= 1 << 23;

      SAT_(3a);
      NEG_(3c, 2);

      if (neg1) {
         code[1] |= 1 << 27;
      }
   } else {
      emitForm_21(i, 0x0c0, 0x940);

      NEG_(34, 2);
      SAT_(35);
      RND_(36, F);

      if (code[0] & 0x1) {
         if (neg1)
            code[1] ^= 1 << 27;
      } else
      if (neg1) {
         code[1] |= 1 << 19;
      }
   }

   FTZ_(38);
   DNZ_(39);
}

void
CodeEmitterGK110::emitDMAD(const Instruction *i)
{
   assert(!i->saturate);
   assert(!i->ftz);

   emitForm_21(i, 0x1b8, 0xb38);

   NEG_(34, 2);
   RND_(36, F);

   bool neg1 = (i->src(0).mod ^ i->src(1).mod).neg();

   if (code[0] & 0x1) {
      if (neg1)
         code[1] ^= 1 << 27;
   } else
   if (neg1) {
      code[1] |= 1 << 19;
   }
}

void
CodeEmitterGK110::emitMADSP(const Instruction *i)
{
   emitForm_21(i, 0x140, 0xa40);

   if (i->subOp == NV50_IR_SUBOP_MADSP_SD) {
      code[1] |= 0x00c00000;
   } else {
      code[1] |= (i->subOp & 0x00f) << 19; // imadp1
      code[1] |= (i->subOp & 0x0f0) << 20; // imadp2
      code[1] |= (i->subOp & 0x100) << 11; // imadp3
      code[1] |= (i->subOp & 0x200) << 15; // imadp3
      code[1] |= (i->subOp & 0xc00) << 12; // imadp3
   }

   if (i->flagsDef >= 0)
      code[1] |= 1 << 18;
}

void
CodeEmitterGK110::emitFMUL(const Instruction *i)
{
   bool neg = (i->src(0).mod ^ i->src(1).mod).neg();

   assert(i->postFactor >= -3 && i->postFactor <= 3);

   if (isLIMM(i->src(1), TYPE_F32)) {
      emitForm_L(i, 0x200, 0x2, Modifier(0));

      FTZ_(38);
      DNZ_(39);
      SAT_(3a);
      if (neg)
         code[1] ^= 1 << 22;

      assert(i->postFactor == 0);
   } else {
      emitForm_21(i, 0x234, 0xc34);
      code[1] |= ((i->postFactor > 0) ?
                  (7 - i->postFactor) : (0 - i->postFactor)) << 12;

      RND_(2a, F);
      FTZ_(2f);
      DNZ_(30);
      SAT_(35);

      if (code[0] & 0x1) {
         if (neg)
            code[1] ^= 1 << 27;
      } else
      if (neg) {
         code[1] |= 1 << 19;
      }
   }
}

void
CodeEmitterGK110::emitDMUL(const Instruction *i)
{
   bool neg = (i->src(0).mod ^ i->src(1).mod).neg();

   assert(!i->postFactor);
   assert(!i->saturate);
   assert(!i->ftz);
   assert(!i->dnz);

   emitForm_21(i, 0x240, 0xc40);

   RND_(2a, F);

   if (code[0] & 0x1) {
      if (neg)
         code[1] ^= 1 << 27;
   } else
   if (neg) {
      code[1] |= 1 << 19;
   }
}

void
CodeEmitterGK110::emitIMUL(const Instruction *i)
{
   assert(!i->src(0).mod.neg() && !i->src(1).mod.neg());
   assert(!i->src(0).mod.abs() && !i->src(1).mod.abs());

   if (isLIMM(i->src(1), TYPE_S32)) {
      emitForm_L(i, 0x280, 2, Modifier(0));

      if (i->subOp == NV50_IR_SUBOP_MUL_HIGH)
         code[1] |= 1 << 24;
      if (i->sType == TYPE_S32)
         code[1] |= 3 << 25;
   } else {
      emitForm_21(i, 0x21c, 0xc1c);

      if (i->subOp == NV50_IR_SUBOP_MUL_HIGH)
         code[1] |= 1 << 10;
      if (i->sType == TYPE_S32)
         code[1] |= 3 << 11;
   }
}

void
CodeEmitterGK110::emitFADD(const Instruction *i)
{
   if (isLIMM(i->src(1), TYPE_F32)) {
      assert(i->rnd == ROUND_N);
      assert(!i->saturate);

      Modifier mod = i->src(1).mod ^
         Modifier(i->op == OP_SUB ? NV50_IR_MOD_NEG : 0);

      emitForm_L(i, 0x400, 0, mod);

      FTZ_(3a);
      NEG_(3b, 0);
      ABS_(39, 0);
   } else {
      emitForm_21(i, 0x22c, 0xc2c);

      FTZ_(2f);
      RND_(2a, F);
      ABS_(31, 0);
      NEG_(33, 0);
      SAT_(35);

      if (code[0] & 0x1) {
         modNegAbsF32_3b(i, 1);
         if (i->op == OP_SUB) code[1] ^= 1 << 27;
      } else {
         ABS_(34, 1);
         NEG_(30, 1);
         if (i->op == OP_SUB) code[1] ^= 1 << 16;
      }
   }
}

void
CodeEmitterGK110::emitDADD(const Instruction *i)
{
   assert(!i->saturate);
   assert(!i->ftz);

   emitForm_21(i, 0x238, 0xc38);
   RND_(2a, F);
   ABS_(31, 0);
   NEG_(33, 0);
   if (code[0] & 0x1) {
      modNegAbsF32_3b(i, 1);
      if (i->op == OP_SUB) code[1] ^= 1 << 27;
   } else {
      NEG_(30, 1);
      ABS_(34, 1);
      if (i->op == OP_SUB) code[1] ^= 1 << 16;
   }
}

void
CodeEmitterGK110::emitUADD(const Instruction *i)
{
   uint8_t addOp = (i->src(0).mod.neg() << 1) | i->src(1).mod.neg();

   if (i->op == OP_SUB)
      addOp ^= 1;

   assert(!i->src(0).mod.abs() && !i->src(1).mod.abs());

   if (isLIMM(i->src(1), TYPE_S32)) {
      emitForm_L(i, 0x400, 1, Modifier((addOp & 1) ? NV50_IR_MOD_NEG : 0));

      if (addOp & 2)
         code[1] |= 1 << 27;

      assert(i->flagsDef < 0);
      assert(i->flagsSrc < 0);

      SAT_(39);
   } else {
      emitForm_21(i, 0x208, 0xc08);

      assert(addOp != 3); // would be add-plus-one

      code[1] |= addOp << 19;

      if (i->flagsDef >= 0)
         code[1] |= 1 << 18; // write carry
      if (i->flagsSrc >= 0)
         code[1] |= 1 << 14; // add carry

      SAT_(35);
   }
}

void
CodeEmitterGK110::emitIMAD(const Instruction *i)
{
   uint8_t addOp =
      i->src(2).mod.neg() | ((i->src(0).mod.neg() ^ i->src(1).mod.neg()) << 1);

   emitForm_21(i, 0x100, 0xa00);

   assert(addOp != 3);
   code[1] |= addOp << 26;

   if (i->sType == TYPE_S32)
      code[1] |= (1 << 19) | (1 << 24);

   if (i->subOp == NV50_IR_SUBOP_MUL_HIGH)
      code[1] |= 1 << 25;

   if (i->flagsDef >= 0) code[1] |= 1 << 18;
   if (i->flagsSrc >= 0) code[1] |= 1 << 20;

   SAT_(35);
}

void
CodeEmitterGK110::emitISAD(const Instruction *i)
{
   assert(i->dType == TYPE_S32 || i->dType == TYPE_U32);

   emitForm_21(i, 0x1f4, 0xb74);

   if (i->dType == TYPE_S32)
      code[1] |= 1 << 19;
}

void
CodeEmitterGK110::emitSHLADD(const Instruction *i)
{
   uint8_t addOp = (i->src(0).mod.neg() << 1) | i->src(2).mod.neg();
   const ImmediateValue *imm = i->src(1).get()->asImm();
   assert(imm);

   if (i->src(2).getFile() == FILE_IMMEDIATE) {
      code[0] = 0x1;
      code[1] = 0xc0c << 20;
   } else {
      code[0] = 0x2;
      code[1] = 0x20c << 20;
   }
   code[1] |= addOp << 19;

   emitPredicate(i);

   defId(i->def(0), 2);
   srcId(i->src(0), 10);

   if (i->flagsDef >= 0)
      code[1] |= 1 << 18;

   assert(!(imm->reg.data.u32 & 0xffffffe0));
   code[1] |= imm->reg.data.u32 << 10;

   switch (i->src(2).getFile()) {
   case FILE_GPR:
      assert(code[0] & 0x2);
      code[1] |= 0xc << 28;
      srcId(i->src(2), 23);
      break;
   case FILE_MEMORY_CONST:
      assert(code[0] & 0x2);
      code[1] |= 0x4 << 28;
      setCAddress14(i->src(2));
      break;
   case FILE_IMMEDIATE:
      assert(code[0] & 0x1);
      setShortImmediate(i, 2);
      break;
   default:
      assert(!"bad src2 file");
      break;
   }
}

void
CodeEmitterGK110::emitNOT(const Instruction *i)
{
   code[0] = 0x0003fc02; // logop(mov2) dst, 0, not src
   code[1] = 0x22003800;

   emitPredicate(i);

   defId(i->def(0), 2);

   switch (i->src(0).getFile()) {
   case FILE_GPR:
      code[1] |= 0xc << 28;
      srcId(i->src(0), 23);
      break;
   case FILE_MEMORY_CONST:
      code[1] |= 0x4 << 28;
      setCAddress14(i->src(0));
      break;
   default:
      assert(0);
      break;
   }
}

void
CodeEmitterGK110::emitLogicOp(const Instruction *i, uint8_t subOp)
{
   if (i->def(0).getFile() == FILE_PREDICATE) {
      code[0] = 0x00000002 | (subOp << 27);
      code[1] = 0x84800000;

      emitPredicate(i);

      defId(i->def(0), 5);
      srcId(i->src(0), 14);
      if (i->src(0).mod == Modifier(NV50_IR_MOD_NOT)) code[0] |= 1 << 17;
      srcId(i->src(1), 32);
      if (i->src(1).mod == Modifier(NV50_IR_MOD_NOT)) code[1] |= 1 << 3;

      if (i->defExists(1)) {
         defId(i->def(1), 2);
      } else {
         code[0] |= 7 << 2;
      }
      // (a OP b) OP c
      if (i->predSrc != 2 && i->srcExists(2)) {
         code[1] |= subOp << 16;
         srcId(i->src(2), 42);
         if (i->src(2).mod == Modifier(NV50_IR_MOD_NOT)) code[1] |= 1 << 13;
      } else {
         code[1] |= 7 << 10;
      }
   } else
   if (isLIMM(i->src(1), TYPE_S32)) {
      emitForm_L(i, 0x200, 0, i->src(1).mod);
      code[1] |= subOp << 24;
      NOT_(3a, 0);
   } else {
      emitForm_21(i, 0x220, 0xc20);
      code[1] |= subOp << 12;
      NOT_(2a, 0);
      NOT_(2b, 1);
   }
}

void
CodeEmitterGK110::emitPOPC(const Instruction *i)
{
   assert(!isLIMM(i->src(1), TYPE_S32, true));

   emitForm_21(i, 0x204, 0xc04);

   NOT_(2a, 0);
   if (!(code[0] & 0x1))
      NOT_(2b, 1);
}

void
CodeEmitterGK110::emitINSBF(const Instruction *i)
{
   emitForm_21(i, 0x1f8, 0xb78);
}

void
CodeEmitterGK110::emitEXTBF(const Instruction *i)
{
   emitForm_21(i, 0x600, 0xc00);

   if (i->dType == TYPE_S32)
      code[1] |= 0x80000;
   if (i->subOp == NV50_IR_SUBOP_EXTBF_REV)
      code[1] |= 0x800;
}

void
CodeEmitterGK110::emitBFIND(const Instruction *i)
{
   emitForm_C(i, 0x218, 0x2);

   if (i->dType == TYPE_S32)
      code[1] |= 0x80000;
   if (i->src(0).mod == Modifier(NV50_IR_MOD_NOT))
      code[1] |= 0x800;
   if (i->subOp == NV50_IR_SUBOP_BFIND_SAMT)
      code[1] |= 0x1000;
}

void
CodeEmitterGK110::emitPERMT(const Instruction *i)
{
   emitForm_21(i, 0x1e0, 0xb60);

   code[1] |= i->subOp << 19;
}

void
CodeEmitterGK110::emitShift(const Instruction *i)
{
   if (i->op == OP_SHR) {
      emitForm_21(i, 0x214, 0xc14);
      if (isSignedType(i->dType))
         code[1] |= 1 << 19;
   } else {
      emitForm_21(i, 0x224, 0xc24);
   }

   if (i->subOp == NV50_IR_SUBOP_SHIFT_WRAP)
      code[1] |= 1 << 10;
}

void
CodeEmitterGK110::emitShift64(const Instruction *i)
{
   if (i->op == OP_SHR) {
      emitForm_21(i, 0x27c, 0xc7c);
      if (isSignedType(i->sType))
         code[1] |= 0x100;
      if (i->subOp & NV50_IR_SUBOP_SHIFT_HIGH)
         code[1] |= 1 << 19;
   } else {
      emitForm_21(i, 0xdfc, 0xf7c);
   }
   code[1] |= 0x200;

   if (i->subOp & NV50_IR_SUBOP_SHIFT_WRAP)
      code[1] |= 1 << 21;
}

void
CodeEmitterGK110::emitPreOp(const Instruction *i)
{
   emitForm_C(i, 0x248, 0x2);

   if (i->op == OP_PREEX2)
      code[1] |= 1 << 10;

   NEG_(30, 0);
   ABS_(34, 0);
}

void
CodeEmitterGK110::emitSFnOp(const Instruction *i, uint8_t subOp)
{
   code[0] = 0x00000002 | (subOp << 23);
   code[1] = 0x84000000;

   emitPredicate(i);

   defId(i->def(0), 2);
   srcId(i->src(0), 10);

   NEG_(33, 0);
   ABS_(31, 0);
   SAT_(35);
}

void
CodeEmitterGK110::emitMINMAX(const Instruction *i)
{
   uint32_t op2, op1;

   switch (i->dType) {
   case TYPE_U32:
   case TYPE_S32:
      op2 = 0x210;
      op1 = 0xc10;
      break;
   case TYPE_F32:
      op2 = 0x230;
      op1 = 0xc30;
      break;
   case TYPE_F64:
      op2 = 0x228;
      op1 = 0xc28;
      break;
   default:
      assert(0);
      op2 = 0;
      op1 = 0;
      break;
   }
   emitForm_21(i, op2, op1);

   if (i->dType == TYPE_S32)
      code[1] |= 1 << 19;
   code[1] |= (i->op == OP_MIN) ? 0x1c00 : 0x3c00; // [!]pt
   code[1] |= i->subOp << 14;
   if (i->flagsDef >= 0)
      code[1] |= i->subOp << 18;

   FTZ_(2f);
   ABS_(31, 0);
   NEG_(33, 0);
   if (code[0] & 0x1) {
      modNegAbsF32_3b(i, 1);
   } else {
      ABS_(34, 1);
      NEG_(30, 1);
   }
}

void
CodeEmitterGK110::emitCVT(const Instruction *i)
{
   const bool f2f = isFloatType(i->dType) && isFloatType(i->sType);
   const bool f2i = !isFloatType(i->dType) && isFloatType(i->sType);
   const bool i2f = isFloatType(i->dType) && !isFloatType(i->sType);

   bool sat = i->saturate;
   bool abs = i->src(0).mod.abs();
   bool neg = i->src(0).mod.neg();

   RoundMode rnd = i->rnd;

   switch (i->op) {
   case OP_CEIL:  rnd = f2f ? ROUND_PI : ROUND_P; break;
   case OP_FLOOR: rnd = f2f ? ROUND_MI : ROUND_M; break;
   case OP_TRUNC: rnd = f2f ? ROUND_ZI : ROUND_Z; break;
   case OP_SAT: sat = true; break;
   case OP_NEG: neg = !neg; break;
   case OP_ABS: abs = true; neg = false; break;
   default:
      break;
   }

   DataType dType;

   if (i->op == OP_NEG && i->dType == TYPE_U32)
      dType = TYPE_S32;
   else
      dType = i->dType;


   uint32_t op;

   if      (f2f) op = 0x254;
   else if (f2i) op = 0x258;
   else if (i2f) op = 0x25c;
   else          op = 0x260;

   emitForm_C(i, op, 0x2);

   FTZ_(2f);
   if (neg) code[1] |= 1 << 16;
   if (abs) code[1] |= 1 << 20;
   if (sat) code[1] |= 1 << 21;

   emitRoundMode(rnd, 32 + 10, f2f ? (32 + 13) : -1);

   code[0] |= typeSizeofLog2(dType) << 10;
   code[0] |= typeSizeofLog2(i->sType) << 12;
   code[1] |= i->subOp << 12;

   if (isSignedIntType(dType))
      code[0] |= 0x4000;
   if (isSignedIntType(i->sType))
      code[0] |= 0x8000;
}

void
CodeEmitterGK110::emitSET(const CmpInstruction *i)
{
   uint16_t op1, op2;

   if (i->def(0).getFile() == FILE_PREDICATE) {
      switch (i->sType) {
      case TYPE_F32: op2 = 0x1d8; op1 = 0xb58; break;
      case TYPE_F64: op2 = 0x1c0; op1 = 0xb40; break;
      default:
         op2 = 0x1b0;
         op1 = 0xb30;
         break;
      }
      emitForm_21(i, op2, op1);

      NEG_(2e, 0);
      ABS_(9, 0);
      if (!(code[0] & 0x1)) {
         NEG_(8, 1);
         ABS_(2f, 1);
      } else {
         modNegAbsF32_3b(i, 1);
      }
      FTZ_(32);

      // normal DST field is negated predicate result
      code[0] = (code[0] & ~0xfc) | ((code[0] << 3) & 0xe0);
      if (i->defExists(1))
         defId(i->def(1), 2);
      else
         code[0] |= 0x1c;
   } else {
      switch (i->sType) {
      case TYPE_F32: op2 = 0x000; op1 = 0x800; break;
      case TYPE_F64: op2 = 0x080; op1 = 0x900; break;
      default:
         op2 = 0x1a8;
         op1 = 0xb28;
         break;
      }
      emitForm_21(i, op2, op1);

      NEG_(2e, 0);
      ABS_(39, 0);
      if (!(code[0] & 0x1)) {
         NEG_(38, 1);
         ABS_(2f, 1);
      } else {
         modNegAbsF32_3b(i, 1);
      }
      FTZ_(3a);

      if (i->dType == TYPE_F32) {
         if (isFloatType(i->sType))
            code[1] |= 1 << 23;
         else
            code[1] |= 1 << 15;
      }
   }
   if (i->sType == TYPE_S32)
      code[1] |= 1 << 19;

   if (i->op != OP_SET) {
      switch (i->op) {
      case OP_SET_AND: code[1] |= 0x0 << 16; break;
      case OP_SET_OR:  code[1] |= 0x1 << 16; break;
      case OP_SET_XOR: code[1] |= 0x2 << 16; break;
      default:
         assert(0);
         break;
      }
      srcId(i->src(2), 0x2a);
   } else {
      code[1] |= 0x7 << 10;
   }
   if (i->flagsSrc >= 0)
      code[1] |= 1 << 14;
   emitCondCode(i->setCond,
                isFloatType(i->sType) ? 0x33 : 0x34,
                isFloatType(i->sType) ? 0xf : 0x7);
}

void
CodeEmitterGK110::emitSLCT(const CmpInstruction *i)
{
   CondCode cc = i->setCond;
   if (i->src(2).mod.neg())
      cc = reverseCondCode(cc);

   if (i->dType == TYPE_F32) {
      emitForm_21(i, 0x1d0, 0xb50);
      FTZ_(32);
      emitCondCode(cc, 0x33, 0xf);
   } else {
      emitForm_21(i, 0x1a0, 0xb20);
      emitCondCode(cc, 0x34, 0x7);
      if (i->dType == TYPE_S32)
         code[1] |= 1 << 19;
   }
}

void
gk110_selpFlip(const FixupEntry *entry, uint32_t *code, const FixupData& data)
{
   int loc = entry->loc;
   bool val = false;
   switch (entry->ipa) {
   case 0:
      val = data.force_persample_interp;
      break;
   case 1:
      val = data.msaa;
      break;
   }
   if (val)
      code[loc + 1] |= 1 << 13;
   else
      code[loc + 1] &= ~(1 << 13);
}

void CodeEmitterGK110::emitSELP(const Instruction *i)
{
   emitForm_21(i, 0x250, 0x050);

   if (i->src(2).mod & Modifier(NV50_IR_MOD_NOT))
      code[1] |= 1 << 13;

   if (i->subOp >= 1) {
      addInterp(i->subOp - 1, 0, gk110_selpFlip);
   }
}

void CodeEmitterGK110::emitTEXBAR(const Instruction *i)
{
   code[0] = 0x0000003e | (i->subOp << 23);
   code[1] = 0x77000000;

   emitPredicate(i);
}

void CodeEmitterGK110::emitTEXCSAA(const TexInstruction *i)
{
   code[0] = 0x00000002;
   code[1] = 0x76c00000;

   code[1] |= i->tex.r << 9;
   // code[1] |= i->tex.s << (9 + 8);

   if (i->tex.liveOnly)
      code[0] |= 0x80000000;

   defId(i->def(0), 2);
   srcId(i->src(0), 10);
}

static inline bool
isNextIndependentTex(const TexInstruction *i)
{
   if (!i->next || !isTextureOp(i->next->op))
      return false;
   if (i->getDef(0)->interfers(i->next->getSrc(0)))
      return false;
   return !i->next->srcExists(1) || !i->getDef(0)->interfers(i->next->getSrc(1));
}

void
CodeEmitterGK110::emitTEX(const TexInstruction *i)
{
   const bool ind = i->tex.rIndirectSrc >= 0;

   if (ind) {
      code[0] = 0x00000002;
      switch (i->op) {
      case OP_TXD:
         code[1] = 0x7e000000;
         break;
      case OP_TXLQ:
         code[1] = 0x7e800000;
         break;
      case OP_TXF:
         code[1] = 0x78000000;
         break;
      case OP_TXG:
         code[1] = 0x7dc00000;
         break;
      default:
         code[1] = 0x7d800000;
         break;
      }
   } else {
      switch (i->op) {
      case OP_TXD:
         code[0] = 0x00000002;
         code[1] = 0x76000000;
         code[1] |= i->tex.r << 9;
         break;
      case OP_TXLQ:
         code[0] = 0x00000002;
         code[1] = 0x76800000;
         code[1] |= i->tex.r << 9;
         break;
      case OP_TXF:
         code[0] = 0x00000002;
         code[1] = 0x70000000;
         code[1] |= i->tex.r << 13;
         break;
      case OP_TXG:
         code[0] = 0x00000001;
         code[1] = 0x70000000;
         code[1] |= i->tex.r << 15;
         break;
      default:
         code[0] = 0x00000001;
         code[1] = 0x60000000;
         code[1] |= i->tex.r << 15;
         break;
      }
   }

   code[1] |= isNextIndependentTex(i) ? 0x1 : 0x2; // t : p mode

   if (i->tex.liveOnly)
      code[0] |= 0x80000000;

   switch (i->op) {
   case OP_TEX: break;
   case OP_TXB: code[1] |= 0x2000; break;
   case OP_TXL: code[1] |= 0x3000; break;
   case OP_TXF: break;
   case OP_TXG: break;
   case OP_TXD: break;
   case OP_TXLQ: break;
   default:
      assert(!"invalid texture op");
      break;
   }

   if (i->op == OP_TXF) {
      if (!i->tex.levelZero)
         code[1] |= 0x1000;
   } else
   if (i->tex.levelZero) {
      code[1] |= 0x1000;
   }

   if (i->op != OP_TXD && i->tex.derivAll)
      code[1] |= 0x200;

   emitPredicate(i);

   code[1] |= i->tex.mask << 2;

   const int src1 = (i->predSrc == 1) ? 2 : 1; // if predSrc == 1, !srcExists(2)

   defId(i->def(0), 2);
   srcId(i->src(0), 10);
   srcId(i, src1, 23);

   if (i->op == OP_TXG) code[1] |= i->tex.gatherComp << 13;

   // texture target:
   code[1] |= (i->tex.target.isCube() ? 3 : (i->tex.target.getDim() - 1)) << 7;
   if (i->tex.target.isArray())
      code[1] |= 0x40;
   if (i->tex.target.isShadow())
      code[1] |= 0x400;
   if (i->tex.target == TEX_TARGET_2D_MS ||
       i->tex.target == TEX_TARGET_2D_MS_ARRAY)
      code[1] |= 0x800;

   if (i->srcExists(src1) && i->src(src1).getFile() == FILE_IMMEDIATE) {
      // ?
   }

   if (i->tex.useOffsets == 1) {
      switch (i->op) {
      case OP_TXF: code[1] |= 0x200; break;
      case OP_TXD: code[1] |= 0x00400000; break;
      default: code[1] |= 0x800; break;
      }
   }
   if (i->tex.useOffsets == 4)
      code[1] |= 0x1000;
}

void
CodeEmitterGK110::emitTXQ(const TexInstruction *i)
{
   code[0] = 0x00000002;
   code[1] = 0x75400001;

   switch (i->tex.query) {
   case TXQ_DIMS:            code[0] |= 0x01 << 25; break;
   case TXQ_TYPE:            code[0] |= 0x02 << 25; break;
   case TXQ_SAMPLE_POSITION: code[0] |= 0x05 << 25; break;
   case TXQ_FILTER:          code[0] |= 0x10 << 25; break;
   case TXQ_LOD:             code[0] |= 0x12 << 25; break;
   case TXQ_BORDER_COLOUR:   code[0] |= 0x16 << 25; break;
   default:
      assert(!"invalid texture query");
      break;
   }

   code[1] |= i->tex.mask << 2;
   code[1] |= i->tex.r << 9;
   if (/*i->tex.sIndirectSrc >= 0 || */i->tex.rIndirectSrc >= 0)
      code[1] |= 0x08000000;

   defId(i->def(0), 2);
   srcId(i->src(0), 10);

   emitPredicate(i);
}

void
CodeEmitterGK110::emitQUADOP(const Instruction *i, uint8_t qOp, uint8_t laneMask)
{
   code[0] = 0x00000002 | ((qOp & 1) << 31);
   code[1] = 0x7fc00200 | (qOp >> 1) | (laneMask << 12); // dall

   defId(i->def(0), 2);
   srcId(i->src(0), 10);
   srcId((i->srcExists(1) && i->predSrc != 1) ? i->src(1) : i->src(0), 23);

   emitPredicate(i);
}

void
CodeEmitterGK110::emitPIXLD(const Instruction *i)
{
   emitForm_L(i, 0x7f4, 2, Modifier(0));
   code[1] |= i->subOp << 2;
   code[1] |= 0x00070000;
}

void
CodeEmitterGK110::emitBAR(const Instruction *i)
{
   code[0] = 0x00000002;
   code[1] = 0x85400000;

   switch (i->subOp) {
   case NV50_IR_SUBOP_BAR_ARRIVE:   code[1] |= 0x08; break;
   case NV50_IR_SUBOP_BAR_RED_AND:  code[1] |= 0x50; break;
   case NV50_IR_SUBOP_BAR_RED_OR:   code[1] |= 0x90; break;
   case NV50_IR_SUBOP_BAR_RED_POPC: code[1] |= 0x10; break;
   default:
      assert(i->subOp == NV50_IR_SUBOP_BAR_SYNC);
      break;
   }

   emitPredicate(i);

   // barrier id
   if (i->src(0).getFile() == FILE_GPR) {
      srcId(i->src(0), 10);
   } else {
      ImmediateValue *imm = i->getSrc(0)->asImm();
      assert(imm);
      code[0] |= imm->reg.data.u32 << 10;
      code[1] |= 0x8000;
   }

   // thread count
   if (i->src(1).getFile() == FILE_GPR) {
      srcId(i->src(1), 23);
   } else {
      ImmediateValue *imm = i->getSrc(0)->asImm();
      assert(imm);
      assert(imm->reg.data.u32 <= 0xfff);
      code[0] |= imm->reg.data.u32 << 23;
      code[1] |= imm->reg.data.u32 >> 9;
      code[1] |= 0x4000;
   }

   if (i->srcExists(2) && (i->predSrc != 2)) {
      srcId(i->src(2), 32 + 10);
      if (i->src(2).mod == Modifier(NV50_IR_MOD_NOT))
         code[1] |= 1 << 13;
   } else {
      code[1] |= 7 << 10;
   }
}

void CodeEmitterGK110::emitMEMBAR(const Instruction *i)
{
   code[0] = 0x00000002 | NV50_IR_SUBOP_MEMBAR_SCOPE(i->subOp) << 8;
   code[1] = 0x7cc00000;

   emitPredicate(i);
}

void
CodeEmitterGK110::emitFlow(const Instruction *i)
{
   const FlowInstruction *f = i->asFlow();

   unsigned mask; // bit 0: predicate, bit 1: target

   code[0] = 0x00000000;

   switch (i->op) {
   case OP_BRA:
      code[1] = f->absolute ? 0x10800000 : 0x12000000;
      if (i->srcExists(0) && i->src(0).getFile() == FILE_MEMORY_CONST)
         code[0] |= 0x80;
      mask = 3;
      break;
   case OP_CALL:
      code[1] = f->absolute ? 0x11000000 : 0x13000000;
      if (i->srcExists(0) && i->src(0).getFile() == FILE_MEMORY_CONST)
         code[0] |= 0x80;
      mask = 2;
      break;

   case OP_EXIT:    code[1] = 0x18000000; mask = 1; break;
   case OP_RET:     code[1] = 0x19000000; mask = 1; break;
   case OP_DISCARD: code[1] = 0x19800000; mask = 1; break;
   case OP_BREAK:   code[1] = 0x1a000000; mask = 1; break;
   case OP_CONT:    code[1] = 0x1a800000; mask = 1; break;

   case OP_JOINAT:   code[1] = 0x14800000; mask = 2; break;
   case OP_PREBREAK: code[1] = 0x15000000; mask = 2; break;
   case OP_PRECONT:  code[1] = 0x15800000; mask = 2; break;
   case OP_PRERET:   code[1] = 0x13800000; mask = 2; break;

   case OP_QUADON:  code[1] = 0x1b800000; mask = 0; break;
   case OP_QUADPOP: code[1] = 0x1c000000; mask = 0; break;
   case OP_BRKPT:   code[1] = 0x00000000; mask = 0; break;
   default:
      assert(!"invalid flow operation");
      return;
   }

   if (mask & 1) {
      emitPredicate(i);
      if (i->flagsSrc < 0)
         code[0] |= 0x3c;
   }

   if (!f)
      return;

   if (f->allWarp)
      code[0] |= 1 << 9;
   if (f->limit)
      code[0] |= 1 << 8;

   if (f->op == OP_CALL) {
      if (f->builtin) {
         assert(f->absolute);
         uint32_t pcAbs = targNVC0->getBuiltinOffset(f->target.builtin);
         addReloc(RelocEntry::TYPE_BUILTIN, 0, pcAbs, 0xff800000, 23);
         addReloc(RelocEntry::TYPE_BUILTIN, 1, pcAbs, 0x007fffff, -9);
      } else {
         assert(!f->absolute);
         int32_t pcRel = f->target.fn->binPos - (codeSize + 8);
         code[0] |= (pcRel & 0x1ff) << 23;
         code[1] |= (pcRel >> 9) & 0x7fff;
      }
   } else
   if (mask & 2) {
      int32_t pcRel = f->target.bb->binPos - (codeSize + 8);
      if (writeIssueDelays && !(f->target.bb->binPos & 0x3f))
         pcRel += 8;
      // currently we don't want absolute branches
      assert(!f->absolute);
      code[0] |= (pcRel & 0x1ff) << 23;
      code[1] |= (pcRel >> 9) & 0x7fff;
   }
}

void
CodeEmitterGK110::emitSHFL(const Instruction *i)
{
   const ImmediateValue *imm;

   code[0] = 0x00000002;
   code[1] = 0x78800000 | (i->subOp << 1);

   emitPredicate(i);

   defId(i->def(0), 2);
   srcId(i->src(0), 10);

   switch (i->src(1).getFile()) {
   case FILE_GPR:
      srcId(i->src(1), 23);
      break;
   case FILE_IMMEDIATE:
      imm = i->getSrc(1)->asImm();
      assert(imm && imm->reg.data.u32 < 0x20);
      code[0] |= imm->reg.data.u32 << 23;
      code[0] |= 1 << 31;
      break;
   default:
      assert(!"invalid src1 file");
      break;
   }

   switch (i->src(2).getFile()) {
   case FILE_GPR:
      srcId(i->src(2), 42);
      break;
   case FILE_IMMEDIATE:
      imm = i->getSrc(2)->asImm();
      assert(imm && imm->reg.data.u32 < 0x2000);
      code[1] |= imm->reg.data.u32 << 5;
      code[1] |= 1;
      break;
   default:
      assert(!"invalid src2 file");
      break;
   }

   if (!i->defExists(1))
      code[1] |= 7 << 19;
   else {
      assert(i->def(1).getFile() == FILE_PREDICATE);
      defId(i->def(1), 51);
   }
}

void
CodeEmitterGK110::emitVOTE(const Instruction *i)
{
   const ImmediateValue *imm;
   uint32_t u32;

   code[0] = 0x00000002;
   code[1] = 0x86c00000 | (i->subOp << 19);

   emitPredicate(i);

   unsigned rp = 0;
   for (int d = 0; i->defExists(d); d++) {
      if (i->def(d).getFile() == FILE_PREDICATE) {
         assert(!(rp & 2));
         rp |= 2;
         defId(i->def(d), 48);
      } else if (i->def(d).getFile() == FILE_GPR) {
         assert(!(rp & 1));
         rp |= 1;
         defId(i->def(d), 2);
      } else {
         assert(!"Unhandled def");
      }
   }
   if (!(rp & 1))
      code[0] |= 255 << 2;
   if (!(rp & 2))
      code[1] |= 7 << 16;

   switch (i->src(0).getFile()) {
   case FILE_PREDICATE:
      if (i->src(0).mod == Modifier(NV50_IR_MOD_NOT))
         code[0] |= 1 << 13;
      srcId(i->src(0), 42);
      break;
   case FILE_IMMEDIATE:
      imm = i->getSrc(0)->asImm();
      assert(imm);
      u32 = imm->reg.data.u32;
      assert(u32 == 0 || u32 == 1);
      code[1] |= (u32 == 1 ? 0x7 : 0xf) << 10;
      break;
   default:
      assert(!"Unhandled src");
      break;
   }
}

void
CodeEmitterGK110::emitSUGType(DataType ty, const int pos)
{
   uint8_t n = 0;

   switch (ty) {
   case TYPE_S32: n = 1; break;
   case TYPE_U8:  n = 2; break;
   case TYPE_S8:  n = 3; break;
   default:
      assert(ty == TYPE_U32);
      break;
   }
   code[pos / 32] |= n << (pos % 32);
}

void
CodeEmitterGK110::emitSUCachingMode(CacheMode c)
{
   uint8_t n = 0;

   switch (c) {
   case CACHE_CA:
// case CACHE_WB:
      n = 0;
      break;
   case CACHE_CG:
      n = 1;
      break;
   case CACHE_CS:
      n = 2;
      break;
   case CACHE_CV:
// case CACHE_WT:
      n = 3;
      break;
   default:
      assert(!"invalid caching mode");
      break;
   }
   code[0] |= (n & 1) << 31;
   code[1] |= (n & 2) >> 1;
}

void
CodeEmitterGK110::setSUConst16(const Instruction *i, const int s)
{
   const uint32_t offset = i->getSrc(s)->reg.data.offset;

   assert(offset == (offset & 0xfffc));

   code[0] |= offset << 21;
   code[1] |= offset >> 11;
   code[1] |= i->getSrc(s)->reg.fileIndex << 5;
}

void
CodeEmitterGK110::emitSULDGB(const TexInstruction *i)
{
   code[0] = 0x00000002;
   code[1] = 0x30000000 | (i->subOp << 14);

   if (i->src(1).getFile() == FILE_MEMORY_CONST) {
      emitLoadStoreType(i->dType, 0x38);
      emitCachingMode(i->cache, 0x36);

      // format
      setSUConst16(i, 1);
   } else {
      assert(i->src(1).getFile() == FILE_GPR);
      code[1] |= 0x49800000;

      emitLoadStoreType(i->dType, 0x21);
      emitSUCachingMode(i->cache);

      srcId(i->src(1), 23);
   }

   emitSUGType(i->sType, 0x34);

   emitPredicate(i);
   defId(i->def(0), 2); // destination
   srcId(i->src(0), 10); // address

   // surface predicate
   if (!i->srcExists(2) || (i->predSrc == 2)) {
      code[1] |= 0x7 << 10;
   } else {
      if (i->src(2).mod == Modifier(NV50_IR_MOD_NOT))
         code[1] |= 1 << 13;
      srcId(i->src(2), 32 + 10);
   }
}

void
CodeEmitterGK110::emitSUSTGx(const TexInstruction *i)
{
   assert(i->op == OP_SUSTP);

   code[0] = 0x00000002;
   code[1] = 0x38000000;

   if (i->src(1).getFile() == FILE_MEMORY_CONST) {
      code[0] |= i->subOp << 2;

      if (i->op == OP_SUSTP)
         code[0] |= i->tex.mask << 4;

      emitSUGType(i->sType, 0x8);
      emitCachingMode(i->cache, 0x36);

      // format
      setSUConst16(i, 1);
   } else {
      assert(i->src(1).getFile() == FILE_GPR);

      code[0] |= i->subOp << 23;
      code[1] |= 0x41c00000;

      if (i->op == OP_SUSTP)
         code[0] |= i->tex.mask << 25;

      emitSUGType(i->sType, 0x1d);
      emitSUCachingMode(i->cache);

      srcId(i->src(1), 2);
   }

   emitPredicate(i);
   srcId(i->src(0), 10); // address
   srcId(i->src(3), 42); // values

   // surface predicate
   if (!i->srcExists(2) || (i->predSrc == 2)) {
      code[1] |= 0x7 << 18;
   } else {
      if (i->src(2).mod == Modifier(NV50_IR_MOD_NOT))
         code[1] |= 1 << 21;
      srcId(i->src(2), 32 + 18);
   }
}

void
CodeEmitterGK110::emitSUCLAMPMode(uint16_t subOp)
{
   uint8_t m;
   switch (subOp & ~NV50_IR_SUBOP_SUCLAMP_2D) {
   case NV50_IR_SUBOP_SUCLAMP_SD(0, 1): m = 0; break;
   case NV50_IR_SUBOP_SUCLAMP_SD(1, 1): m = 1; break;
   case NV50_IR_SUBOP_SUCLAMP_SD(2, 1): m = 2; break;
   case NV50_IR_SUBOP_SUCLAMP_SD(3, 1): m = 3; break;
   case NV50_IR_SUBOP_SUCLAMP_SD(4, 1): m = 4; break;
   case NV50_IR_SUBOP_SUCLAMP_PL(0, 1): m = 5; break;
   case NV50_IR_SUBOP_SUCLAMP_PL(1, 1): m = 6; break;
   case NV50_IR_SUBOP_SUCLAMP_PL(2, 1): m = 7; break;
   case NV50_IR_SUBOP_SUCLAMP_PL(3, 1): m = 8; break;
   case NV50_IR_SUBOP_SUCLAMP_PL(4, 1): m = 9; break;
   case NV50_IR_SUBOP_SUCLAMP_BL(0, 1): m = 10; break;
   case NV50_IR_SUBOP_SUCLAMP_BL(1, 1): m = 11; break;
   case NV50_IR_SUBOP_SUCLAMP_BL(2, 1): m = 12; break;
   case NV50_IR_SUBOP_SUCLAMP_BL(3, 1): m = 13; break;
   case NV50_IR_SUBOP_SUCLAMP_BL(4, 1): m = 14; break;
   default:
      return;
   }
   code[1] |= m << 20;
   if (subOp & NV50_IR_SUBOP_SUCLAMP_2D)
      code[1] |= 1 << 24;
}

void
CodeEmitterGK110::emitSUCalc(Instruction *i)
{
   ImmediateValue *imm = NULL;
   uint64_t opc1, opc2;

   if (i->srcExists(2)) {
      imm = i->getSrc(2)->asImm();
      if (imm)
         i->setSrc(2, NULL); // special case, make emitForm_21 not assert
   }

   switch (i->op) {
   case OP_SUCLAMP:  opc1 = 0xb00; opc2 = 0x580; break;
   case OP_SUBFM:    opc1 = 0xb68; opc2 = 0x1e8; break;
   case OP_SUEAU:    opc1 = 0xb6c; opc2 = 0x1ec; break;
   default:
      assert(0);
      return;
   }
   emitForm_21(i, opc2, opc1);

   if (i->op == OP_SUCLAMP) {
      if (i->dType == TYPE_S32)
         code[1] |= 1 << 19;
      emitSUCLAMPMode(i->subOp);
   }

   if (i->op == OP_SUBFM && i->subOp == NV50_IR_SUBOP_SUBFM_3D)
      code[1] |= 1 << 18;

   if (i->op != OP_SUEAU) {
      const uint8_t pos = i->op == OP_SUBFM ? 19 : 16;
      if (i->def(0).getFile() == FILE_PREDICATE) { // p, #
         code[0] |= 255 << 2;
         code[1] |= i->getDef(1)->reg.data.id << pos;
      } else
      if (i->defExists(1)) { // r, p
         assert(i->def(1).getFile() == FILE_PREDICATE);
         code[1] |= i->getDef(1)->reg.data.id << pos;
      } else { // r, #
         code[1] |= 7 << pos;
      }
   }

   if (imm) {
      assert(i->op == OP_SUCLAMP);
      i->setSrc(2, imm);
      code[1] |= (imm->reg.data.u32 & 0x3f) << 10; // sint6
   }
}


void
CodeEmitterGK110::emitVectorSubOp(const Instruction *i)
{
   switch (NV50_IR_SUBOP_Vn(i->subOp)) {
   case 0:
      code[1] |= (i->subOp & 0x000f) << 7;  // vsrc1
      code[1] |= (i->subOp & 0x00e0) >> 6;  // vsrc2
      code[1] |= (i->subOp & 0x0100) << 13; // vsrc2
      code[1] |= (i->subOp & 0x3c00) << 12; // vdst
      break;
   default:
      assert(0);
      break;
   }
}

void
CodeEmitterGK110::emitVSHL(const Instruction *i)
{
   code[0] = 0x00000002;
   code[1] = 0xb8000000;

   assert(NV50_IR_SUBOP_Vn(i->subOp) == 0);

   if (isSignedType(i->dType)) code[1] |= 1 << 25;
   if (isSignedType(i->sType)) code[1] |= 1 << 19;

   emitVectorSubOp(i);

   emitPredicate(i);
   defId(i->def(0), 2);
   srcId(i->src(0), 10);

   if (i->getSrc(1)->reg.file == FILE_IMMEDIATE) {
      ImmediateValue *imm = i->getSrc(1)->asImm();
      assert(imm);
      code[0] |= (imm->reg.data.u32 & 0x01ff) << 23;
      code[1] |= (imm->reg.data.u32 & 0xfe00) >> 9;
   } else {
      assert(i->getSrc(1)->reg.file == FILE_GPR);
      code[1] |= 1 << 21;
      srcId(i->src(1), 23);
   }
   srcId(i->src(2), 42);

   if (i->saturate)
      code[0] |= 1 << 22;
   if (i->flagsDef >= 0)
      code[1] |= 1 << 18;
}

void
CodeEmitterGK110::emitAFETCH(const Instruction *i)
{
   uint32_t offset = i->src(0).get()->reg.data.offset & 0x7ff;

   code[0] = 0x00000002 | (offset << 23);
   code[1] = 0x7d000000 | (offset >> 9);

   if (i->getSrc(0)->reg.file == FILE_SHADER_OUTPUT)
      code[1] |= 0x8;

   emitPredicate(i);

   defId(i->def(0), 2);
   srcId(i->src(0).getIndirect(0), 10);
}

void
CodeEmitterGK110::emitPFETCH(const Instruction *i)
{
   uint32_t prim = i->src(0).get()->reg.data.u32;

   code[0] = 0x00000002 | ((prim & 0xff) << 23);
   code[1] = 0x7f800000;

   emitPredicate(i);

   const int src1 = (i->predSrc == 1) ? 2 : 1; // if predSrc == 1, !srcExists(2)

   defId(i->def(0), 2);
   srcId(i, src1, 10);
}

void
CodeEmitterGK110::emitVFETCH(const Instruction *i)
{
   unsigned int size = typeSizeof(i->dType);
   uint32_t offset = i->src(0).get()->reg.data.offset;

   code[0] = 0x00000002 | (offset << 23);
   code[1] = 0x7ec00000 | (offset >> 9);
   code[1] |= (size / 4 - 1) << 18;

   if (i->perPatch)
      code[1] |= 0x4;
   if (i->getSrc(0)->reg.file == FILE_SHADER_OUTPUT)
      code[1] |= 0x8; // yes, TCPs can read from *outputs* of other threads

   emitPredicate(i);

   defId(i->def(0), 2);
   srcId(i->src(0).getIndirect(0), 10);
   srcId(i->src(0).getIndirect(1), 32 + 10); // vertex address
}

void
CodeEmitterGK110::emitEXPORT(const Instruction *i)
{
   unsigned int size = typeSizeof(i->dType);
   uint32_t offset = i->src(0).get()->reg.data.offset;

   code[0] = 0x00000002 | (offset << 23);
   code[1] = 0x7f000000 | (offset >> 9);
   code[1] |= (size / 4 - 1) << 18;

   if (i->perPatch)
      code[1] |= 0x4;

   emitPredicate(i);

   assert(i->src(1).getFile() == FILE_GPR);

   srcId(i->src(0).getIndirect(0), 10);
   srcId(i->src(0).getIndirect(1), 32 + 10); // vertex base address
   srcId(i->src(1), 2);
}

void
CodeEmitterGK110::emitOUT(const Instruction *i)
{
   assert(i->src(0).getFile() == FILE_GPR);

   emitForm_21(i, 0x1f0, 0xb70);

   if (i->op == OP_EMIT)
      code[1] |= 1 << 10;
   if (i->op == OP_RESTART || i->subOp == NV50_IR_SUBOP_EMIT_RESTART)
      code[1] |= 1 << 11;
}

void
CodeEmitterGK110::emitInterpMode(const Instruction *i)
{
   code[1] |= (i->ipa & 0x3) << 21; // TODO: INTERP_SAMPLEID
   code[1] |= (i->ipa & 0xc) << (19 - 2);
}

void
gk110_interpApply(const struct FixupEntry *entry, uint32_t *code, const FixupData& data)
{
   int ipa = entry->ipa;
   int reg = entry->reg;
   int loc = entry->loc;

   if (data.flatshade &&
       (ipa & NV50_IR_INTERP_MODE_MASK) == NV50_IR_INTERP_SC) {
      ipa = NV50_IR_INTERP_FLAT;
      reg = 0xff;
   } else if (data.force_persample_interp &&
              (ipa & NV50_IR_INTERP_SAMPLE_MASK) == NV50_IR_INTERP_DEFAULT &&
              (ipa & NV50_IR_INTERP_MODE_MASK) != NV50_IR_INTERP_FLAT) {
      ipa |= NV50_IR_INTERP_CENTROID;
   }
   code[loc + 1] &= ~(0xf << 19);
   code[loc + 1] |= (ipa & 0x3) << 21;
   code[loc + 1] |= (ipa & 0xc) << (19 - 2);
   code[loc + 0] &= ~(0xff << 23);
   code[loc + 0] |= reg << 23;
}

void
CodeEmitterGK110::emitINTERP(const Instruction *i)
{
   const uint32_t base = i->getSrc(0)->reg.data.offset;

   code[0] = 0x00000002 | (base << 31);
   code[1] = 0x74800000 | (base >> 1);

   if (i->saturate)
      code[1] |= 1 << 18;

   if (i->op == OP_PINTERP) {
      srcId(i->src(1), 23);
      addInterp(i->ipa, SDATA(i->src(1)).id, gk110_interpApply);
   } else {
      code[0] |= 0xff << 23;
      addInterp(i->ipa, 0xff, gk110_interpApply);
   }

   srcId(i->src(0).getIndirect(0), 10);
   emitInterpMode(i);

   emitPredicate(i);
   defId(i->def(0), 2);

   if (i->getSampleMode() == NV50_IR_INTERP_OFFSET)
      srcId(i->src(i->op == OP_PINTERP ? 2 : 1), 32 + 10);
   else
      code[1] |= 0xff << 10;
}

void
CodeEmitterGK110::emitLoadStoreType(DataType ty, const int pos)
{
   uint8_t n;

   switch (ty) {
   case TYPE_U8:
      n = 0;
      break;
   case TYPE_S8:
      n = 1;
      break;
   case TYPE_U16:
      n = 2;
      break;
   case TYPE_S16:
      n = 3;
      break;
   case TYPE_F32:
   case TYPE_U32:
   case TYPE_S32:
      n = 4;
      break;
   case TYPE_F64:
   case TYPE_U64:
   case TYPE_S64:
      n = 5;
      break;
   case TYPE_B128:
      n = 6;
      break;
   default:
      n = 0;
      assert(!"invalid ld/st type");
      break;
   }
   code[pos / 32] |= n << (pos % 32);
}

void
CodeEmitterGK110::emitCachingMode(CacheMode c, const int pos)
{
   uint8_t n;

   switch (c) {
   case CACHE_CA:
// case CACHE_WB:
      n = 0;
      break;
   case CACHE_CG:
      n = 1;
      break;
   case CACHE_CS:
      n = 2;
      break;
   case CACHE_CV:
// case CACHE_WT:
      n = 3;
      break;
   default:
      n = 0;
      assert(!"invalid caching mode");
      break;
   }
   code[pos / 32] |= n << (pos % 32);
}

void
CodeEmitterGK110::emitSTORE(const Instruction *i)
{
   int32_t offset = SDATA(i->src(0)).offset;

   switch (i->src(0).getFile()) {
   case FILE_MEMORY_GLOBAL: code[1] = 0xe0000000; code[0] = 0x00000000; break;
   case FILE_MEMORY_LOCAL:  code[1] = 0x7a800000; code[0] = 0x00000002; break;
   case FILE_MEMORY_SHARED:
      code[0] = 0x00000002;
      if (i->subOp == NV50_IR_SUBOP_STORE_UNLOCKED)
         code[1] = 0x78400000;
      else
         code[1] = 0x7ac00000;
      break;
   default:
      assert(!"invalid memory file");
      break;
   }

   if (code[0] & 0x2) {
      offset &= 0xffffff;
      emitLoadStoreType(i->dType, 0x33);
      if (i->src(0).getFile() == FILE_MEMORY_LOCAL)
         emitCachingMode(i->cache, 0x2f);
   } else {
      emitLoadStoreType(i->dType, 0x38);
      emitCachingMode(i->cache, 0x3b);
   }
   code[0] |= offset << 23;
   code[1] |= offset >> 9;

   // Unlocked store on shared memory can fail.
   if (i->src(0).getFile() == FILE_MEMORY_SHARED &&
       i->subOp == NV50_IR_SUBOP_STORE_UNLOCKED) {
      assert(i->defExists(0));
      defId(i->def(0), 32 + 16);
   }

   emitPredicate(i);

   srcId(i->src(1), 2);
   srcId(i->src(0).getIndirect(0), 10);
   if (i->src(0).getFile() == FILE_MEMORY_GLOBAL &&
       i->src(0).isIndirect(0) &&
       i->getIndirect(0, 0)->reg.size == 8)
      code[1] |= 1 << 23;
}

void
CodeEmitterGK110::emitLOAD(const Instruction *i)
{
   int32_t offset = SDATA(i->src(0)).offset;

   switch (i->src(0).getFile()) {
   case FILE_MEMORY_GLOBAL: code[1] = 0xc0000000; code[0] = 0x00000000; break;
   case FILE_MEMORY_LOCAL:  code[1] = 0x7a000000; code[0] = 0x00000002; break;
   case FILE_MEMORY_SHARED:
      code[0] = 0x00000002;
      if (i->subOp == NV50_IR_SUBOP_LOAD_LOCKED)
         code[1] = 0x77400000;
      else
         code[1] = 0x7a400000;
      break;
   case FILE_MEMORY_CONST:
      if (!i->src(0).isIndirect(0) && typeSizeof(i->dType) == 4) {
         emitMOV(i);
         return;
      }
      offset &= 0xffff;
      code[0] = 0x00000002;
      code[1] = 0x7c800000 | (i->src(0).get()->reg.fileIndex << 7);
      code[1] |= i->subOp << 15;
      break;
   default:
      assert(!"invalid memory file");
      break;
   }

   if (code[0] & 0x2) {
      offset &= 0xffffff;
      emitLoadStoreType(i->dType, 0x33);
      if (i->src(0).getFile() == FILE_MEMORY_LOCAL)
         emitCachingMode(i->cache, 0x2f);
   } else {
      emitLoadStoreType(i->dType, 0x38);
      emitCachingMode(i->cache, 0x3b);
   }
   code[0] |= offset << 23;
   code[1] |= offset >> 9;

   // Locked store on shared memory can fail.
   int r = 0, p = -1;
   if (i->src(0).getFile() == FILE_MEMORY_SHARED &&
       i->subOp == NV50_IR_SUBOP_LOAD_LOCKED) {
      if (i->def(0).getFile() == FILE_PREDICATE) { // p, #
         r = -1;
         p = 0;
      } else if (i->defExists(1)) { // r, p
         p = 1;
      } else {
         assert(!"Expected predicate dest for load locked");
      }
   }

   emitPredicate(i);

   if (r >= 0)
      defId(i->def(r), 2);
   else
      code[0] |= 255 << 2;

   if (p >= 0)
      defId(i->def(p), 32 + 16);

   if (i->getIndirect(0, 0)) {
      srcId(i->src(0).getIndirect(0), 10);
      if (i->getIndirect(0, 0)->reg.size == 8)
         code[1] |= 1 << 23;
   } else {
      code[0] |= 255 << 10;
   }
}

uint8_t
CodeEmitterGK110::getSRegEncoding(const ValueRef& ref)
{
   switch (SDATA(ref).sv.sv) {
   case SV_LANEID:        return 0x00;
   case SV_PHYSID:        return 0x03;
   case SV_VERTEX_COUNT:  return 0x10;
   case SV_INVOCATION_ID: return 0x11;
   case SV_YDIR:          return 0x12;
   case SV_THREAD_KILL:   return 0x13;
   case SV_COMBINED_TID:  return 0x20;
   case SV_TID:           return 0x21 + SDATA(ref).sv.index;
   case SV_CTAID:         return 0x25 + SDATA(ref).sv.index;
   case SV_NTID:          return 0x29 + SDATA(ref).sv.index;
   case SV_GRIDID:        return 0x2c;
   case SV_NCTAID:        return 0x2d + SDATA(ref).sv.index;
   case SV_LBASE:         return 0x34;
   case SV_SBASE:         return 0x30;
   case SV_LANEMASK_EQ:   return 0x38;
   case SV_LANEMASK_LT:   return 0x39;
   case SV_LANEMASK_LE:   return 0x3a;
   case SV_LANEMASK_GT:   return 0x3b;
   case SV_LANEMASK_GE:   return 0x3c;
   case SV_CLOCK:         return 0x50 + SDATA(ref).sv.index;
   default:
      assert(!"no sreg for system value");
      return 0;
   }
}

void
CodeEmitterGK110::emitMOV(const Instruction *i)
{
   if (i->def(0).getFile() == FILE_PREDICATE) {
      if (i->src(0).getFile() == FILE_GPR) {
         // Use ISETP.NE.AND dst, PT, src, RZ, PT
         code[0] = 0x00000002;
         code[1] = 0xdb500000;

         code[0] |= 0x7 << 2;
         code[0] |= 0xff << 23;
         code[1] |= 0x7 << 10;
         srcId(i->src(0), 10);
      } else
      if (i->src(0).getFile() == FILE_PREDICATE) {
         // Use PSETP.AND.AND dst, PT, src, PT, PT
         code[0] = 0x00000002;
         code[1] = 0x84800000;

         code[0] |= 0x7 << 2;
         code[1] |= 0x7 << 0;
         code[1] |= 0x7 << 10;

         srcId(i->src(0), 14);
      } else {
         assert(!"Unexpected source for predicate destination");
         emitNOP(i);
      }
      emitPredicate(i);
      defId(i->def(0), 5);
   } else
   if (i->src(0).getFile() == FILE_SYSTEM_VALUE) {
      code[0] = 0x00000002 | (getSRegEncoding(i->src(0)) << 23);
      code[1] = 0x86400000;
      emitPredicate(i);
      defId(i->def(0), 2);
   } else
   if (i->src(0).getFile() == FILE_IMMEDIATE) {
      code[0] = 0x00000002 | (i->lanes << 14);
      code[1] = 0x74000000;
      emitPredicate(i);
      defId(i->def(0), 2);
      setImmediate32(i, 0, Modifier(0));
   } else
   if (i->src(0).getFile() == FILE_PREDICATE) {
      code[0] = 0x00000002;
      code[1] = 0x84401c07;
      emitPredicate(i);
      defId(i->def(0), 2);
      srcId(i->src(0), 14);
   } else {
      emitForm_C(i, 0x24c, 2);
      code[1] |= i->lanes << 10;
   }
}

static inline bool
uses64bitAddress(const Instruction *ldst)
{
   return ldst->src(0).getFile() == FILE_MEMORY_GLOBAL &&
      ldst->src(0).isIndirect(0) &&
      ldst->getIndirect(0, 0)->reg.size == 8;
}

void
CodeEmitterGK110::emitATOM(const Instruction *i)
{
   const bool hasDst = i->defExists(0);
   const bool exch = i->subOp == NV50_IR_SUBOP_ATOM_EXCH;

   code[0] = 0x00000002;
   if (i->subOp == NV50_IR_SUBOP_ATOM_CAS)
      code[1] = 0x77800000;
   else
      code[1] = 0x68000000;

   switch (i->subOp) {
   case NV50_IR_SUBOP_ATOM_CAS: break;
   case NV50_IR_SUBOP_ATOM_EXCH: code[1] |= 0x04000000; break;
   default: code[1] |= i->subOp << 23; break;
   }

   switch (i->dType) {
   case TYPE_U32: break;
   case TYPE_S32: code[1] |= 0x00100000; break;
   case TYPE_U64: code[1] |= 0x00200000; break;
   case TYPE_F32: code[1] |= 0x00300000; break;
   case TYPE_B128: code[1] |= 0x00400000; break; /* TODO: U128 */
   case TYPE_S64: code[1] |= 0x00500000; break;
   default: assert(!"unsupported type"); break;
   }

   emitPredicate(i);

   /* TODO: cas: check that src regs line up */
   /* TODO: cas: flip bits if $r255 is used */
   srcId(i->src(1), 23);

   if (hasDst) {
      defId(i->def(0), 2);
   } else
   if (!exch) {
      code[0] |= 255 << 2;
   }

   if (hasDst || !exch) {
      const int32_t offset = SDATA(i->src(0)).offset;
      assert(offset < 0x80000 && offset >= -0x80000);
      code[0] |= (offset & 1) << 31;
      code[1] |= (offset & 0xffffe) >> 1;
   } else {
      srcAddr32(i->src(0), 31);
   }

   if (i->getIndirect(0, 0)) {
      srcId(i->getIndirect(0, 0), 10);
      if (i->getIndirect(0, 0)->reg.size == 8)
         code[1] |= 1 << 19;
   } else {
      code[0] |= 255 << 10;
   }
}

void
CodeEmitterGK110::emitCCTL(const Instruction *i)
{
   int32_t offset = SDATA(i->src(0)).offset;

   code[0] = 0x00000002 | (i->subOp << 2);

   if (i->src(0).getFile() == FILE_MEMORY_GLOBAL) {
      code[1] = 0x7b000000;
   } else {
      code[1] = 0x7c000000;
      offset &= 0xffffff;
   }
   code[0] |= offset << 23;
   code[1] |= offset >> 9;

   if (uses64bitAddress(i))
      code[1] |= 1 << 23;
   srcId(i->src(0).getIndirect(0), 10);

   emitPredicate(i);
}

bool
CodeEmitterGK110::emitInstruction(Instruction *insn)
{
   const unsigned int size = (writeIssueDelays && !(codeSize & 0x3f)) ? 16 : 8;

   if (insn->encSize != 8) {
      ERROR("skipping unencodable instruction: ");
      insn->print();
      return false;
   } else
   if (codeSize + size > codeSizeLimit) {
      ERROR("code emitter output buffer too small\n");
      return false;
   }

   if (writeIssueDelays) {
      int id = (codeSize & 0x3f) / 8 - 1;
      if (id < 0) {
         id += 1;
         code[0] = 0x00000000; // cf issue delay "instruction"
         code[1] = 0x08000000;
         code += 2;
         codeSize += 8;
      }
      uint32_t *data = code - (id * 2 + 2);

      switch (id) {
      case 0: data[0] |= insn->sched << 2; break;
      case 1: data[0] |= insn->sched << 10; break;
      case 2: data[0] |= insn->sched << 18; break;
      case 3: data[0] |= insn->sched << 26; data[1] |= insn->sched >> 6; break;
      case 4: data[1] |= insn->sched << 2; break;
      case 5: data[1] |= insn->sched << 10; break;
      case 6: data[1] |= insn->sched << 18; break;
      default:
         assert(0);
         break;
      }
   }

   // assert that instructions with multiple defs don't corrupt registers
   for (int d = 0; insn->defExists(d); ++d)
      assert(insn->asTex() || insn->def(d).rep()->reg.data.id >= 0);

   switch (insn->op) {
   case OP_MOV:
   case OP_RDSV:
      emitMOV(insn);
      break;
   case OP_NOP:
      break;
   case OP_LOAD:
      emitLOAD(insn);
      break;
   case OP_STORE:
      emitSTORE(insn);
      break;
   case OP_LINTERP:
   case OP_PINTERP:
      emitINTERP(insn);
      break;
   case OP_VFETCH:
      emitVFETCH(insn);
      break;
   case OP_EXPORT:
      emitEXPORT(insn);
      break;
   case OP_AFETCH:
      emitAFETCH(insn);
      break;
   case OP_PFETCH:
      emitPFETCH(insn);
      break;
   case OP_EMIT:
   case OP_RESTART:
      emitOUT(insn);
      break;
   case OP_ADD:
   case OP_SUB:
      if (insn->dType == TYPE_F64)
         emitDADD(insn);
      else if (isFloatType(insn->dType))
         emitFADD(insn);
      else
         emitUADD(insn);
      break;
   case OP_MUL:
      if (insn->dType == TYPE_F64)
         emitDMUL(insn);
      else if (isFloatType(insn->dType))
         emitFMUL(insn);
      else
         emitIMUL(insn);
      break;
   case OP_MAD:
   case OP_FMA:
      if (insn->dType == TYPE_F64)
         emitDMAD(insn);
      else if (isFloatType(insn->dType))
         emitFMAD(insn);
      else
         emitIMAD(insn);
      break;
   case OP_MADSP:
      emitMADSP(insn);
      break;
   case OP_SAD:
      emitISAD(insn);
      break;
   case OP_SHLADD:
      emitSHLADD(insn);
      break;
   case OP_NOT:
      emitNOT(insn);
      break;
   case OP_AND:
      emitLogicOp(insn, 0);
      break;
   case OP_OR:
      emitLogicOp(insn, 1);
      break;
   case OP_XOR:
      emitLogicOp(insn, 2);
      break;
   case OP_SHL:
   case OP_SHR:
      if (typeSizeof(insn->sType) == 8)
         emitShift64(insn);
      else
         emitShift(insn);
      break;
   case OP_SET:
   case OP_SET_AND:
   case OP_SET_OR:
   case OP_SET_XOR:
      emitSET(insn->asCmp());
      break;
   case OP_SELP:
      emitSELP(insn);
      break;
   case OP_SLCT:
      emitSLCT(insn->asCmp());
      break;
   case OP_MIN:
   case OP_MAX:
      emitMINMAX(insn);
      break;
   case OP_ABS:
   case OP_NEG:
   case OP_CEIL:
   case OP_FLOOR:
   case OP_TRUNC:
   case OP_SAT:
      emitCVT(insn);
      break;
   case OP_CVT:
      if (insn->def(0).getFile() == FILE_PREDICATE ||
          insn->src(0).getFile() == FILE_PREDICATE)
         emitMOV(insn);
      else
         emitCVT(insn);
      break;
   case OP_RSQ:
      emitSFnOp(insn, 5 + 2 * insn->subOp);
      break;
   case OP_RCP:
      emitSFnOp(insn, 4 + 2 * insn->subOp);
      break;
   case OP_LG2:
      emitSFnOp(insn, 3);
      break;
   case OP_EX2:
      emitSFnOp(insn, 2);
      break;
   case OP_SIN:
      emitSFnOp(insn, 1);
      break;
   case OP_COS:
      emitSFnOp(insn, 0);
      break;
   case OP_PRESIN:
   case OP_PREEX2:
      emitPreOp(insn);
      break;
   case OP_TEX:
   case OP_TXB:
   case OP_TXL:
   case OP_TXD:
   case OP_TXF:
   case OP_TXG:
   case OP_TXLQ:
      emitTEX(insn->asTex());
      break;
   case OP_TXQ:
      emitTXQ(insn->asTex());
      break;
   case OP_TEXBAR:
      emitTEXBAR(insn);
      break;
   case OP_PIXLD:
      emitPIXLD(insn);
      break;
   case OP_BRA:
   case OP_CALL:
   case OP_PRERET:
   case OP_RET:
   case OP_DISCARD:
   case OP_EXIT:
   case OP_PRECONT:
   case OP_CONT:
   case OP_PREBREAK:
   case OP_BREAK:
   case OP_JOINAT:
   case OP_BRKPT:
   case OP_QUADON:
   case OP_QUADPOP:
      emitFlow(insn);
      break;
   case OP_QUADOP:
      emitQUADOP(insn, insn->subOp, insn->lanes);
      break;
   case OP_DFDX:
      emitQUADOP(insn, insn->src(0).mod.neg() ? 0x66 : 0x99, 0x4);
      break;
   case OP_DFDY:
      emitQUADOP(insn, insn->src(0).mod.neg() ? 0x5a : 0xa5, 0x5);
      break;
   case OP_POPCNT:
      emitPOPC(insn);
      break;
   case OP_INSBF:
      emitINSBF(insn);
      break;
   case OP_EXTBF:
      emitEXTBF(insn);
      break;
   case OP_BFIND:
      emitBFIND(insn);
      break;
   case OP_PERMT:
      emitPERMT(insn);
      break;
   case OP_JOIN:
      emitNOP(insn);
      insn->join = 1;
      break;
   case OP_BAR:
      emitBAR(insn);
      break;
   case OP_MEMBAR:
      emitMEMBAR(insn);
      break;
   case OP_ATOM:
      emitATOM(insn);
      break;
   case OP_CCTL:
      emitCCTL(insn);
      break;
   case OP_SHFL:
      emitSHFL(insn);
      break;
   case OP_VOTE:
      emitVOTE(insn);
      break;
   case OP_SULDB:
      emitSULDGB(insn->asTex());
      break;
   case OP_SUSTB:
   case OP_SUSTP:
      emitSUSTGx(insn->asTex());
      break;
   case OP_SUBFM:
   case OP_SUCLAMP:
   case OP_SUEAU:
      emitSUCalc(insn);
      break;
   case OP_VSHL:
      emitVSHL(insn);
      break;
   case OP_PHI:
   case OP_UNION:
      ERROR("operation should have been eliminated");
      return false;
   case OP_SQRT:
      ERROR("operation should have been lowered\n");
      return false;
   default:
      ERROR("unknown op: %u\n", insn->op);
      return false;
   }

   if (insn->join)
      code[0] |= 1 << 22;

   code += 2;
   codeSize += 8;
   return true;
}

uint32_t
CodeEmitterGK110::getMinEncodingSize(const Instruction *i) const
{
   // No more short instruction encodings.
   return 8;
}

void
CodeEmitterGK110::prepareEmission(Function *func)
{
   const Target *targ = func->getProgram()->getTarget();

   CodeEmitter::prepareEmission(func);

   if (targ->hasSWSched)
      calculateSchedDataNVC0(targ, func);
}

CodeEmitterGK110::CodeEmitterGK110(const TargetNVC0 *target, Program::Type type)
   : CodeEmitter(target),
     targNVC0(target),
     progType(type),
     writeIssueDelays(target->hasSWSched)
{
   code = NULL;
   codeSize = codeSizeLimit = 0;
   relocInfo = NULL;
}

CodeEmitter *
TargetNVC0::createCodeEmitterGK110(Program::Type type)
{
   CodeEmitterGK110 *emit = new CodeEmitterGK110(this, type);
   return emit;
}

} // namespace nv50_ir
