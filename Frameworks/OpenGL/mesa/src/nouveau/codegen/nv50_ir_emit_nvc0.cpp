/*
 * Copyright 2011 Christoph Bumiller
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

namespace nv50_ir {

// Argh, all these assertions ...

class CodeEmitterNVC0 : public CodeEmitter
{
public:
   CodeEmitterNVC0(const TargetNVC0 *, Program::Type);

   virtual bool emitInstruction(Instruction *);
   virtual uint32_t getMinEncodingSize(const Instruction *) const;
   virtual void prepareEmission(Function *);

private:
   const TargetNVC0 *targNVC0;

   Program::Type progType;

   const bool writeIssueDelays;

private:
   void emitForm_A(const Instruction *, uint64_t);
   void emitForm_B(const Instruction *, uint64_t);
   void emitForm_S(const Instruction *, uint32_t, bool pred);

   void emitPredicate(const Instruction *);

   void setAddress16(const ValueRef&);
   void setAddress24(const ValueRef&);
   void setAddressByFile(const ValueRef&);
   void setImmediate(const Instruction *, const int s); // needs op already set
   void setImmediateS8(const ValueRef&);
   void setSUConst16(const Instruction *, const int s);
   void setSUPred(const Instruction *, const int s);
   void setPDSTL(const Instruction *, const int d);

   void emitCondCode(CondCode cc, int pos);
   void emitInterpMode(const Instruction *);
   void emitLoadStoreType(DataType ty);
   void emitSUGType(DataType);
   void emitSUAddr(const TexInstruction *);
   void emitSUDim(const TexInstruction *);
   void emitCachingMode(CacheMode c);

   void emitShortSrc2(const ValueRef&);

   inline uint8_t getSRegEncoding(const ValueRef&);

   void roundMode_A(const Instruction *);
   void roundMode_C(const Instruction *);
   void roundMode_CS(const Instruction *);

   void emitNegAbs12(const Instruction *);

   void emitNOP(const Instruction *);

   void emitLOAD(const Instruction *);
   void emitSTORE(const Instruction *);
   void emitMOV(const Instruction *);
   void emitATOM(const Instruction *);
   void emitMEMBAR(const Instruction *);
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
   void emitUMUL(const Instruction *);
   void emitFMUL(const Instruction *);
   void emitDMUL(const Instruction *);
   void emitIMAD(const Instruction *);
   void emitISAD(const Instruction *);
   void emitSHLADD(const Instruction *a);
   void emitFMAD(const Instruction *);
   void emitDMAD(const Instruction *);
   void emitMADSP(const Instruction *);

   void emitNOT(Instruction *);
   void emitLogicOp(const Instruction *, uint8_t subOp);
   void emitPOPC(const Instruction *);
   void emitINSBF(const Instruction *);
   void emitEXTBF(const Instruction *);
   void emitBFIND(const Instruction *);
   void emitPERMT(const Instruction *);
   void emitShift(const Instruction *);

   void emitSFnOp(const Instruction *, uint8_t subOp);

   void emitCVT(Instruction *);
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

   void emitFlow(const Instruction *);
   void emitBAR(const Instruction *);

   void emitSUCLAMPMode(uint16_t);
   void emitSUCalc(Instruction *);
   void emitSULDGB(const TexInstruction *);
   void emitSUSTGx(const TexInstruction *);

   void emitSULDB(const TexInstruction *);
   void emitSUSTx(const TexInstruction *);
   void emitSULEA(const TexInstruction *);

   void emitVSHL(const Instruction *);
   void emitVectorSubOp(const Instruction *);

   void emitPIXLD(const Instruction *);

   void emitSHFL(const Instruction *);

   void emitVOTE(const Instruction *);

   inline void defId(const ValueDef&, const int pos);
   inline void defId(const Instruction *, int d, const int pos);
   inline void srcId(const ValueRef&, const int pos);
   inline void srcId(const ValueRef *, const int pos);
   inline void srcId(const Instruction *, int s, const int pos);
   inline void srcAddr32(const ValueRef&, int pos, int shr);

   inline bool isLIMM(const ValueRef&, DataType ty);
};

// for better visibility
#define HEX64(h, l) 0x##h##l##ULL

#define SDATA(a) ((a).rep()->reg.data)
#define DDATA(a) ((a).rep()->reg.data)

void CodeEmitterNVC0::srcId(const ValueRef& src, const int pos)
{
   code[pos / 32] |= (src.get() ? SDATA(src).id : 63) << (pos % 32);
}

void CodeEmitterNVC0::srcId(const ValueRef *src, const int pos)
{
   code[pos / 32] |= (src ? SDATA(*src).id : 63) << (pos % 32);
}

void CodeEmitterNVC0::srcId(const Instruction *insn, int s, int pos)
{
   int r = insn->srcExists(s) ? SDATA(insn->src(s)).id : 63;
   code[pos / 32] |= r << (pos % 32);
}

void
CodeEmitterNVC0::srcAddr32(const ValueRef& src, int pos, int shr)
{
   const uint32_t offset = SDATA(src).offset >> shr;

   code[pos / 32] |= offset << (pos % 32);
   if (pos && (pos < 32))
      code[1] |= offset >> (32 - pos);
}

void CodeEmitterNVC0::defId(const ValueDef& def, const int pos)
{
   code[pos / 32] |= (def.get() && def.getFile() != FILE_FLAGS ? DDATA(def).id : 63) << (pos % 32);
}

void CodeEmitterNVC0::defId(const Instruction *insn, int d, const int pos)
{
   if (insn->defExists(d))
      defId(insn->def(d), pos);
   else
      code[pos / 32] |= 63 << (pos % 32);
}

bool CodeEmitterNVC0::isLIMM(const ValueRef& ref, DataType ty)
{
   const ImmediateValue *imm = ref.get()->asImm();

   if (ty == TYPE_F32)
      return imm && imm->reg.data.u32 & 0xfff;
   else
      return imm && (imm->reg.data.s32 > 0x7ffff ||
                     imm->reg.data.s32 < -0x80000);
}

void
CodeEmitterNVC0::roundMode_A(const Instruction *insn)
{
   switch (insn->rnd) {
   case ROUND_M: code[1] |= 1 << 23; break;
   case ROUND_P: code[1] |= 2 << 23; break;
   case ROUND_Z: code[1] |= 3 << 23; break;
   default:
      assert(insn->rnd == ROUND_N);
      break;
   }
}

void
CodeEmitterNVC0::emitNegAbs12(const Instruction *i)
{
   if (i->src(1).mod.abs()) code[0] |= 1 << 6;
   if (i->src(0).mod.abs()) code[0] |= 1 << 7;
   if (i->src(1).mod.neg()) code[0] |= 1 << 8;
   if (i->src(0).mod.neg()) code[0] |= 1 << 9;
}

void CodeEmitterNVC0::emitCondCode(CondCode cc, int pos)
{
   uint8_t val;

   switch (cc) {
   case CC_LT:  val = 0x1; break;
   case CC_LTU: val = 0x9; break;
   case CC_EQ:  val = 0x2; break;
   case CC_EQU: val = 0xa; break;
   case CC_LE:  val = 0x3; break;
   case CC_LEU: val = 0xb; break;
   case CC_GT:  val = 0x4; break;
   case CC_GTU: val = 0xc; break;
   case CC_NE:  val = 0x5; break;
   case CC_NEU: val = 0xd; break;
   case CC_GE:  val = 0x6; break;
   case CC_GEU: val = 0xe; break;
   case CC_TR:  val = 0xf; break;
   case CC_FL:  val = 0x0; break;

   case CC_A:  val = 0x14; break;
   case CC_NA: val = 0x13; break;
   case CC_S:  val = 0x15; break;
   case CC_NS: val = 0x12; break;
   case CC_C:  val = 0x16; break;
   case CC_NC: val = 0x11; break;
   case CC_O:  val = 0x17; break;
   case CC_NO: val = 0x10; break;

   default:
      val = 0;
      assert(!"invalid condition code");
      break;
   }
   code[pos / 32] |= val << (pos % 32);
}

void
CodeEmitterNVC0::emitPredicate(const Instruction *i)
{
   if (i->predSrc >= 0) {
      assert(i->getPredicate()->reg.file == FILE_PREDICATE);
      srcId(i->src(i->predSrc), 10);
      if (i->cc == CC_NOT_P)
         code[0] |= 0x2000; // negate
   } else {
      code[0] |= 0x1c00;
   }
}

void
CodeEmitterNVC0::setAddressByFile(const ValueRef& src)
{
   switch (src.getFile()) {
   case FILE_MEMORY_GLOBAL:
      srcAddr32(src, 26, 0);
      break;
   case FILE_MEMORY_LOCAL:
   case FILE_MEMORY_SHARED:
      setAddress24(src);
      break;
   default:
      assert(src.getFile() == FILE_MEMORY_CONST);
      setAddress16(src);
      break;
   }
}

void
CodeEmitterNVC0::setAddress16(const ValueRef& src)
{
   Symbol *sym = src.get()->asSym();

   assert(sym);

   code[0] |= (sym->reg.data.offset & 0x003f) << 26;
   code[1] |= (sym->reg.data.offset & 0xffc0) >> 6;
}

void
CodeEmitterNVC0::setAddress24(const ValueRef& src)
{
   Symbol *sym = src.get()->asSym();

   assert(sym);

   code[0] |= (sym->reg.data.offset & 0x00003f) << 26;
   code[1] |= (sym->reg.data.offset & 0xffffc0) >> 6;
}

void
CodeEmitterNVC0::setImmediate(const Instruction *i, const int s)
{
   const ImmediateValue *imm = i->src(s).get()->asImm();
   uint32_t u32;

   assert(imm);
   u32 = imm->reg.data.u32;

   if ((code[0] & 0xf) == 0x1) {
      // double immediate
      uint64_t u64 = imm->reg.data.u64;
      assert(!(u64 & 0x00000fffffffffffULL));
      assert(!(code[1] & 0xc000));
      code[0] |= ((u64 >> 44) & 0x3f) << 26;
      code[1] |= 0xc000 | (u64 >> 50);
   } else
   if ((code[0] & 0xf) == 0x2) {
      // LIMM
      code[0] |= (u32 & 0x3f) << 26;
      code[1] |= u32 >> 6;
   } else
   if ((code[0] & 0xf) == 0x3 || (code[0] & 0xf) == 4) {
      // integer immediate
      assert((u32 & 0xfff80000) == 0 || (u32 & 0xfff80000) == 0xfff80000);
      assert(!(code[1] & 0xc000));
      u32 &= 0xfffff;
      code[0] |= (u32 & 0x3f) << 26;
      code[1] |= 0xc000 | (u32 >> 6);
   } else {
      // float immediate
      assert(!(u32 & 0x00000fff));
      assert(!(code[1] & 0xc000));
      code[0] |= ((u32 >> 12) & 0x3f) << 26;
      code[1] |= 0xc000 | (u32 >> 18);
   }
}

void CodeEmitterNVC0::setImmediateS8(const ValueRef &ref)
{
   const ImmediateValue *imm = ref.get()->asImm();

   int8_t s8 = static_cast<int8_t>(imm->reg.data.s32);

   assert(s8 == imm->reg.data.s32);

   code[0] |= (s8 & 0x3f) << 26;
   code[0] |= (s8 >> 6) << 8;
}

void CodeEmitterNVC0::setPDSTL(const Instruction *i, const int d)
{
   assert(d < 0 || (i->defExists(d) && i->def(d).getFile() == FILE_PREDICATE));

   uint32_t pred = d >= 0 ? DDATA(i->def(d)).id : 7;

   code[0] |= (pred & 3) << 8;
   code[1] |= (pred & 4) << (26 - 2);
}

void
CodeEmitterNVC0::emitForm_A(const Instruction *i, uint64_t opc)
{
   code[0] = opc;
   code[1] = opc >> 32;

   emitPredicate(i);

   defId(i->def(0), 14);

   int s1 = 26;
   if (i->srcExists(2) && i->getSrc(2)->reg.file == FILE_MEMORY_CONST)
      s1 = 49;

   for (int s = 0; s < 3 && i->srcExists(s); ++s) {
      switch (i->getSrc(s)->reg.file) {
      case FILE_MEMORY_CONST:
         assert(!(code[1] & 0xc000));
         code[1] |= (s == 2) ? 0x8000 : 0x4000;
         code[1] |= i->getSrc(s)->reg.fileIndex << 10;
         setAddress16(i->src(s));
         break;
      case FILE_IMMEDIATE:
         assert(s == 1 ||
                i->op == OP_MOV || i->op == OP_PRESIN || i->op == OP_PREEX2);
         assert(!(code[1] & 0xc000));
         setImmediate(i, s);
         break;
      case FILE_GPR:
         if ((s == 2) && ((code[0] & 0x7) == 2)) // LIMM: 3rd src == dst
            break;
         srcId(i->src(s), s ? ((s == 2) ? 49 : s1) : 20);
         break;
      default:
         if (i->op == OP_SELP) {
            // OP_SELP is used to implement shared+atomics on Fermi.
            assert(s == 2 && i->src(s).getFile() == FILE_PREDICATE);
            srcId(i->src(s), 49);
         }
         // ignore here, can be predicate or flags, but must not be address
         break;
      }
   }
}

void
CodeEmitterNVC0::emitForm_B(const Instruction *i, uint64_t opc)
{
   code[0] = opc;
   code[1] = opc >> 32;

   emitPredicate(i);

   defId(i->def(0), 14);

   switch (i->src(0).getFile()) {
   case FILE_MEMORY_CONST:
      assert(!(code[1] & 0xc000));
      code[1] |= 0x4000 | (i->src(0).get()->reg.fileIndex << 10);
      setAddress16(i->src(0));
      break;
   case FILE_IMMEDIATE:
      assert(!(code[1] & 0xc000));
      setImmediate(i, 0);
      break;
   case FILE_GPR:
      srcId(i->src(0), 26);
      break;
   default:
      // ignore here, can be predicate or flags, but must not be address
      break;
   }
}

void
CodeEmitterNVC0::emitForm_S(const Instruction *i, uint32_t opc, bool pred)
{
   code[0] = opc;

   int ss2a = 0;
   if (opc == 0x0d || opc == 0x0e)
      ss2a = 2;

   defId(i->def(0), 14);
   srcId(i->src(0), 20);

   assert(pred || (i->predSrc < 0));
   if (pred)
      emitPredicate(i);

   for (int s = 1; s < 3 && i->srcExists(s); ++s) {
      if (i->src(s).get()->reg.file == FILE_MEMORY_CONST) {
         assert(!(code[0] & (0x300 >> ss2a)));
         switch (i->src(s).get()->reg.fileIndex) {
         case 0:  code[0] |= 0x100 >> ss2a; break;
         case 1:  code[0] |= 0x200 >> ss2a; break;
         case 16: code[0] |= 0x300 >> ss2a; break;
         default:
            ERROR("invalid c[] space for short form\n");
            break;
         }
         if (s == 1)
            code[0] |= i->getSrc(s)->reg.data.offset << 24;
         else
            code[0] |= i->getSrc(s)->reg.data.offset << 6;
      } else
      if (i->src(s).getFile() == FILE_IMMEDIATE) {
         assert(s == 1);
         setImmediateS8(i->src(s));
      } else
      if (i->src(s).getFile() == FILE_GPR) {
         srcId(i->src(s), (s == 1) ? 26 : 8);
      }
   }
}

void
CodeEmitterNVC0::emitShortSrc2(const ValueRef &src)
{
   if (src.getFile() == FILE_MEMORY_CONST) {
      switch (src.get()->reg.fileIndex) {
      case 0:  code[0] |= 0x100; break;
      case 1:  code[0] |= 0x200; break;
      case 16: code[0] |= 0x300; break;
      default:
         assert(!"unsupported file index for short op");
         break;
      }
      srcAddr32(src, 20, 2);
   } else {
      srcId(src, 20);
      assert(src.getFile() == FILE_GPR);
   }
}

void
CodeEmitterNVC0::emitNOP(const Instruction *i)
{
   code[0] = 0x000001e4;
   code[1] = 0x40000000;
   emitPredicate(i);
}

void
CodeEmitterNVC0::emitFMAD(const Instruction *i)
{
   bool neg1 = (i->src(0).mod ^ i->src(1).mod).neg();

   if (i->encSize == 8) {
      if (isLIMM(i->src(1), TYPE_F32)) {
         emitForm_A(i, HEX64(20000000, 00000002));
      } else {
         emitForm_A(i, HEX64(30000000, 00000000));

         if (i->src(2).mod.neg())
            code[0] |= 1 << 8;
      }
      roundMode_A(i);

      if (neg1)
         code[0] |= 1 << 9;

      if (i->saturate)
         code[0] |= 1 << 5;

      if (i->dnz)
         code[0] |= 1 << 7;
      else
      if (i->ftz)
         code[0] |= 1 << 6;
   } else {
      assert(!i->saturate && !i->src(2).mod.neg());
      emitForm_S(i, (i->src(2).getFile() == FILE_MEMORY_CONST) ? 0x2e : 0x0e,
                 false);
      if (neg1)
         code[0] |= 1 << 4;
   }
}

void
CodeEmitterNVC0::emitDMAD(const Instruction *i)
{
   bool neg1 = (i->src(0).mod ^ i->src(1).mod).neg();

   emitForm_A(i, HEX64(20000000, 00000001));

   if (i->src(2).mod.neg())
      code[0] |= 1 << 8;

   roundMode_A(i);

   if (neg1)
      code[0] |= 1 << 9;

   assert(!i->saturate);
   assert(!i->ftz);
}

void
CodeEmitterNVC0::emitFMUL(const Instruction *i)
{
   bool neg = (i->src(0).mod ^ i->src(1).mod).neg();

   assert(i->postFactor >= -3 && i->postFactor <= 3);

   if (i->encSize == 8) {
      if (isLIMM(i->src(1), TYPE_F32)) {
         assert(i->postFactor == 0); // constant folded, hopefully
         emitForm_A(i, HEX64(30000000, 00000002));
      } else {
         emitForm_A(i, HEX64(58000000, 00000000));
         roundMode_A(i);
         code[1] |= ((i->postFactor > 0) ?
                     (7 - i->postFactor) : (0 - i->postFactor)) << 17;
      }
      if (neg)
         code[1] ^= 1 << 25; // aliases with LIMM sign bit

      if (i->saturate)
         code[0] |= 1 << 5;

      if (i->dnz)
         code[0] |= 1 << 7;
      else
      if (i->ftz)
         code[0] |= 1 << 6;
   } else {
      assert(!neg && !i->saturate && !i->ftz && !i->postFactor);
      emitForm_S(i, 0xa8, true);
   }
}

void
CodeEmitterNVC0::emitDMUL(const Instruction *i)
{
   bool neg = (i->src(0).mod ^ i->src(1).mod).neg();

   emitForm_A(i, HEX64(50000000, 00000001));
   roundMode_A(i);

   if (neg)
      code[0] |= 1 << 9;

   assert(!i->saturate);
   assert(!i->ftz);
   assert(!i->dnz);
   assert(!i->postFactor);
}

void
CodeEmitterNVC0::emitUMUL(const Instruction *i)
{
   if (i->encSize == 8) {
      if (isLIMM(i->src(1), TYPE_U32)) {
         emitForm_A(i, HEX64(10000000, 00000002));
      } else {
         emitForm_A(i, HEX64(50000000, 00000003));
      }
      if (i->subOp == NV50_IR_SUBOP_MUL_HIGH)
         code[0] |= 1 << 6;
      if (i->sType == TYPE_S32)
         code[0] |= 1 << 5;
      if (i->dType == TYPE_S32)
         code[0] |= 1 << 7;
   } else {
      emitForm_S(i, i->src(1).getFile() == FILE_IMMEDIATE ? 0xaa : 0x2a, true);

      if (i->sType == TYPE_S32)
         code[0] |= 1 << 6;
   }
}

void
CodeEmitterNVC0::emitFADD(const Instruction *i)
{
   if (i->encSize == 8) {
      if (isLIMM(i->src(1), TYPE_F32)) {
         assert(!i->saturate);
         emitForm_A(i, HEX64(28000000, 00000002));

         code[0] |= i->src(0).mod.abs() << 7;
         code[0] |= i->src(0).mod.neg() << 9;

         if (i->src(1).mod.abs())
            code[1] &= 0xfdffffff;
         if ((i->op == OP_SUB) != static_cast<bool>(i->src(1).mod.neg()))
            code[1] ^= 0x02000000;
      } else {
         emitForm_A(i, HEX64(50000000, 00000000));

         roundMode_A(i);
         if (i->saturate)
            code[1] |= 1 << 17;

         emitNegAbs12(i);
         if (i->op == OP_SUB) code[0] ^= 1 << 8;
      }
      if (i->ftz)
         code[0] |= 1 << 5;
   } else {
      assert(!i->saturate && i->op != OP_SUB &&
             !i->src(0).mod.abs() &&
             !i->src(1).mod.neg() && !i->src(1).mod.abs());

      emitForm_S(i, 0x49, true);

      if (i->src(0).mod.neg())
         code[0] |= 1 << 7;
   }
}

void
CodeEmitterNVC0::emitDADD(const Instruction *i)
{
   assert(i->encSize == 8);
   emitForm_A(i, HEX64(48000000, 00000001));
   roundMode_A(i);
   assert(!i->saturate);
   assert(!i->ftz);
   emitNegAbs12(i);
   if (i->op == OP_SUB)
      code[0] ^= 1 << 8;
}

void
CodeEmitterNVC0::emitUADD(const Instruction *i)
{
   uint32_t addOp = 0;

   assert(!i->src(0).mod.abs() && !i->src(1).mod.abs());

   if (i->src(0).mod.neg())
      addOp |= 0x200;
   if (i->src(1).mod.neg())
      addOp |= 0x100;
   if (i->op == OP_SUB)
      addOp ^= 0x100;

   assert(addOp != 0x300); // would be add-plus-one

   if (i->encSize == 8) {
      if (isLIMM(i->src(1), TYPE_U32)) {
         emitForm_A(i, HEX64(08000000, 00000002));
         if (i->flagsDef >= 0)
            code[1] |= 1 << 26; // write carry
      } else {
         emitForm_A(i, HEX64(48000000, 00000003));
         if (i->flagsDef >= 0)
            code[1] |= 1 << 16; // write carry
      }
      code[0] |= addOp;

      if (i->saturate)
         code[0] |= 1 << 5;
      if (i->flagsSrc >= 0) // add carry
         code[0] |= 1 << 6;
   } else {
      assert(!(addOp & 0x100));
      emitForm_S(i, (addOp >> 3) |
                 ((i->src(1).getFile() == FILE_IMMEDIATE) ? 0xac : 0x2c), true);
   }
}

void
CodeEmitterNVC0::emitIMAD(const Instruction *i)
{
   uint8_t addOp =
      i->src(2).mod.neg() | ((i->src(0).mod.neg() ^ i->src(1).mod.neg()) << 1);

   assert(i->encSize == 8);
   emitForm_A(i, HEX64(20000000, 00000003));

   assert(addOp != 3);
   code[0] |= addOp << 8;

   if (isSignedType(i->dType))
      code[0] |= 1 << 7;
   if (isSignedType(i->sType))
      code[0] |= 1 << 5;

   code[1] |= i->saturate << 24;

   if (i->flagsDef >= 0) code[1] |= 1 << 16;
   if (i->flagsSrc >= 0) code[1] |= 1 << 23;

   if (i->subOp == NV50_IR_SUBOP_MUL_HIGH)
      code[0] |= 1 << 6;
}

void
CodeEmitterNVC0::emitSHLADD(const Instruction *i)
{
   uint8_t addOp = (i->src(0).mod.neg() << 1) | i->src(2).mod.neg();
   const ImmediateValue *imm = i->src(1).get()->asImm();
   assert(imm);

   code[0] = 0x00000003;
   code[1] = 0x40000000 | addOp << 23;

   emitPredicate(i);

   defId(i->def(0), 14);
   srcId(i->src(0), 20);

   if (i->flagsDef >= 0)
      code[1] |= 1 << 16;

   assert(!(imm->reg.data.u32 & 0xffffffe0));
   code[0] |= imm->reg.data.u32 << 5;

   switch (i->src(2).getFile()) {
   case FILE_GPR:
      srcId(i->src(2), 26);
      break;
   case FILE_MEMORY_CONST:
      code[1] |= 0x4000;
      code[1] |= i->getSrc(2)->reg.fileIndex << 10;
      setAddress16(i->src(2));
      break;
   case FILE_IMMEDIATE:
      setImmediate(i, 2);
      break;
   default:
      assert(!"bad src2 file");
      break;
   }
}

void
CodeEmitterNVC0::emitMADSP(const Instruction *i)
{
   assert(targ->getChipset() >= NVISA_GK104_CHIPSET);

   emitForm_A(i, HEX64(00000000, 00000003));

   if (i->subOp == NV50_IR_SUBOP_MADSP_SD) {
      code[1] |= 0x01800000;
   } else {
      code[0] |= (i->subOp & 0x00f) << 7;
      code[0] |= (i->subOp & 0x0f0) << 1;
      code[0] |= (i->subOp & 0x100) >> 3;
      code[0] |= (i->subOp & 0x200) >> 2;
      code[1] |= (i->subOp & 0xc00) << 13;
   }

   if (i->flagsDef >= 0)
      code[1] |= 1 << 16;
}

void
CodeEmitterNVC0::emitISAD(const Instruction *i)
{
   assert(i->dType == TYPE_S32 || i->dType == TYPE_U32);
   assert(i->encSize == 8);

   emitForm_A(i, HEX64(38000000, 00000003));

   if (i->dType == TYPE_S32)
      code[0] |= 1 << 5;
}

void
CodeEmitterNVC0::emitNOT(Instruction *i)
{
   assert(i->encSize == 8);
   if (i->getPredicate())
      i->moveSources(1, 1);
   i->setSrc(1, i->src(0));
   emitForm_A(i, HEX64(68000000, 000001c3));
}

void
CodeEmitterNVC0::emitLogicOp(const Instruction *i, uint8_t subOp)
{
   if (i->def(0).getFile() == FILE_PREDICATE) {
      code[0] = 0x00000004 | (subOp << 30);
      code[1] = 0x0c000000;

      emitPredicate(i);

      defId(i->def(0), 17);
      srcId(i->src(0), 20);
      if (i->src(0).mod == Modifier(NV50_IR_MOD_NOT)) code[0] |= 1 << 23;
      srcId(i->src(1), 26);
      if (i->src(1).mod == Modifier(NV50_IR_MOD_NOT)) code[0] |= 1 << 29;

      if (i->defExists(1)) {
         defId(i->def(1), 14);
      } else {
         code[0] |= 7 << 14;
      }
      // (a OP b) OP c
      if (i->predSrc != 2 && i->srcExists(2)) {
         code[1] |= subOp << 21;
         srcId(i->src(2), 49);
         if (i->src(2).mod == Modifier(NV50_IR_MOD_NOT)) code[1] |= 1 << 20;
      } else {
         code[1] |= 0x000e0000;
      }
   } else
   if (i->encSize == 8) {
      if (isLIMM(i->src(1), TYPE_U32)) {
         emitForm_A(i, HEX64(38000000, 00000002));

         if (i->flagsDef >= 0)
            code[1] |= 1 << 26;
      } else {
         emitForm_A(i, HEX64(68000000, 00000003));

         if (i->flagsDef >= 0)
            code[1] |= 1 << 16;
      }
      code[0] |= subOp << 6;

      if (i->flagsSrc >= 0) // carry
         code[0] |= 1 << 5;

      if (i->src(0).mod & Modifier(NV50_IR_MOD_NOT)) code[0] |= 1 << 9;
      if (i->src(1).mod & Modifier(NV50_IR_MOD_NOT)) code[0] |= 1 << 8;
   } else {
      emitForm_S(i, (subOp << 5) |
                 ((i->src(1).getFile() == FILE_IMMEDIATE) ? 0x1d : 0x8d), true);
   }
}

void
CodeEmitterNVC0::emitPOPC(const Instruction *i)
{
   emitForm_A(i, HEX64(54000000, 00000004));

   if (i->src(0).mod & Modifier(NV50_IR_MOD_NOT)) code[0] |= 1 << 9;
   if (i->src(1).mod & Modifier(NV50_IR_MOD_NOT)) code[0] |= 1 << 8;
}

void
CodeEmitterNVC0::emitINSBF(const Instruction *i)
{
   emitForm_A(i, HEX64(28000000, 00000003));
}

void
CodeEmitterNVC0::emitEXTBF(const Instruction *i)
{
   emitForm_A(i, HEX64(70000000, 00000003));

   if (i->dType == TYPE_S32)
      code[0] |= 1 << 5;
   if (i->subOp == NV50_IR_SUBOP_EXTBF_REV)
      code[0] |= 1 << 8;
}

void
CodeEmitterNVC0::emitBFIND(const Instruction *i)
{
   emitForm_B(i, HEX64(78000000, 00000003));

   if (i->dType == TYPE_S32)
      code[0] |= 1 << 5;
   if (i->src(0).mod == Modifier(NV50_IR_MOD_NOT))
      code[0] |= 1 << 8;
   if (i->subOp == NV50_IR_SUBOP_BFIND_SAMT)
      code[0] |= 1 << 6;
}

void
CodeEmitterNVC0::emitPERMT(const Instruction *i)
{
   emitForm_A(i, HEX64(24000000, 00000004));

   code[0] |= i->subOp << 5;
}

void
CodeEmitterNVC0::emitShift(const Instruction *i)
{
   if (i->op == OP_SHR) {
      emitForm_A(i, HEX64(58000000, 00000003)
                 | (isSignedType(i->dType) ? 0x20 : 0x00));
   } else {
      emitForm_A(i, HEX64(60000000, 00000003));
   }

   if (i->subOp == NV50_IR_SUBOP_SHIFT_WRAP)
      code[0] |= 1 << 9;
}

void
CodeEmitterNVC0::emitPreOp(const Instruction *i)
{
   if (i->encSize == 8) {
      emitForm_B(i, HEX64(60000000, 00000000));

      if (i->op == OP_PREEX2)
         code[0] |= 0x20;

      if (i->src(0).mod.abs()) code[0] |= 1 << 6;
      if (i->src(0).mod.neg()) code[0] |= 1 << 8;
   } else {
      emitForm_S(i, i->op == OP_PREEX2 ? 0x74000008 : 0x70000008, true);
   }
}

void
CodeEmitterNVC0::emitSFnOp(const Instruction *i, uint8_t subOp)
{
   if (i->encSize == 8) {
      code[0] = 0x00000000 | (subOp << 26);
      code[1] = 0xc8000000;

      emitPredicate(i);

      defId(i->def(0), 14);
      srcId(i->src(0), 20);

      assert(i->src(0).getFile() == FILE_GPR);

      if (i->saturate) code[0] |= 1 << 5;

      if (i->src(0).mod.abs()) code[0] |= 1 << 7;
      if (i->src(0).mod.neg()) code[0] |= 1 << 9;
   } else {
      emitForm_S(i, 0x80000008 | (subOp << 26), true);

      assert(!i->src(0).mod.neg());
      if (i->src(0).mod.abs()) code[0] |= 1 << 30;
   }
}

void
CodeEmitterNVC0::emitMINMAX(const Instruction *i)
{
   uint64_t op;

   assert(i->encSize == 8);

   op = (i->op == OP_MIN) ? 0x080e000000000000ULL : 0x081e000000000000ULL;

   if (i->ftz)
      op |= 1 << 5;
   else
   if (!isFloatType(i->dType)) {
      op |= isSignedType(i->dType) ? 0x23 : 0x03;
      op |= i->subOp << 6;
   }
   if (i->dType == TYPE_F64)
      op |= 0x01;

   emitForm_A(i, op);
   emitNegAbs12(i);

   if (i->flagsDef >= 0)
      code[1] |= 1 << 16;
}

void
CodeEmitterNVC0::roundMode_C(const Instruction *i)
{
   switch (i->rnd) {
   case ROUND_M:  code[1] |= 1 << 17; break;
   case ROUND_P:  code[1] |= 2 << 17; break;
   case ROUND_Z:  code[1] |= 3 << 17; break;
   case ROUND_NI: code[0] |= 1 << 7; break;
   case ROUND_MI: code[0] |= 1 << 7; code[1] |= 1 << 17; break;
   case ROUND_PI: code[0] |= 1 << 7; code[1] |= 2 << 17; break;
   case ROUND_ZI: code[0] |= 1 << 7; code[1] |= 3 << 17; break;
   case ROUND_N: break;
   default:
      assert(!"invalid round mode");
      break;
   }
}

void
CodeEmitterNVC0::roundMode_CS(const Instruction *i)
{
   switch (i->rnd) {
   case ROUND_M:
   case ROUND_MI: code[0] |= 1 << 16; break;
   case ROUND_P:
   case ROUND_PI: code[0] |= 2 << 16; break;
   case ROUND_Z:
   case ROUND_ZI: code[0] |= 3 << 16; break;
   default:
      break;
   }
}

void
CodeEmitterNVC0::emitCVT(Instruction *i)
{
   const bool f2f = isFloatType(i->dType) && isFloatType(i->sType);
   DataType dType;

   switch (i->op) {
   case OP_CEIL:  i->rnd = f2f ? ROUND_PI : ROUND_P; break;
   case OP_FLOOR: i->rnd = f2f ? ROUND_MI : ROUND_M; break;
   case OP_TRUNC: i->rnd = f2f ? ROUND_ZI : ROUND_Z; break;
   default:
      break;
   }

   const bool sat = (i->op == OP_SAT) || i->saturate;
   const bool abs = (i->op == OP_ABS) || i->src(0).mod.abs();
   const bool neg = (i->op == OP_NEG) || i->src(0).mod.neg();

   if (i->op == OP_NEG && i->dType == TYPE_U32)
      dType = TYPE_S32;
   else
      dType = i->dType;

   if (i->encSize == 8) {
      emitForm_B(i, HEX64(10000000, 00000004));

      roundMode_C(i);

      // cvt u16 f32 sets high bits to 0, so we don't have to use Value::Size()
      code[0] |= util_logbase2(typeSizeof(dType)) << 20;
      code[0] |= util_logbase2(typeSizeof(i->sType)) << 23;

      // for 8/16 source types, the byte/word is in subOp. word 1 is
      // represented as 2.
      if (!isFloatType(i->sType))
         code[1] |= i->subOp << 0x17;
      else
         code[1] |= i->subOp << 0x18;

      if (sat)
         code[0] |= 0x20;
      if (abs)
         code[0] |= 1 << 6;
      if (neg && i->op != OP_ABS)
         code[0] |= 1 << 8;

      if (i->ftz)
         code[1] |= 1 << 23;

      if (isSignedIntType(dType))
         code[0] |= 0x080;
      if (isSignedIntType(i->sType))
         code[0] |= 0x200;

      if (isFloatType(dType)) {
         if (!isFloatType(i->sType))
            code[1] |= 0x08000000;
      } else {
         if (isFloatType(i->sType))
            code[1] |= 0x04000000;
         else
            code[1] |= 0x0c000000;
      }
   } else {
      if (i->op == OP_CEIL || i->op == OP_FLOOR || i->op == OP_TRUNC) {
         code[0] = 0x298;
      } else
      if (isFloatType(dType)) {
         if (isFloatType(i->sType))
            code[0] = 0x098;
         else
            code[0] = 0x088 | (isSignedType(i->sType) ? (1 << 8) : 0);
      } else {
         assert(isFloatType(i->sType));

         code[0] = 0x288 | (isSignedType(i->sType) ? (1 << 8) : 0);
      }

      if (neg) code[0] |= 1 << 16;
      if (sat) code[0] |= 1 << 18;
      if (abs) code[0] |= 1 << 19;

      roundMode_CS(i);
   }
}

void
CodeEmitterNVC0::emitSET(const CmpInstruction *i)
{
   uint32_t hi;
   uint32_t lo = 0;

   if (i->sType == TYPE_F64)
      lo = 0x1;
   else
   if (!isFloatType(i->sType))
      lo = 0x3;

   if (isSignedIntType(i->sType))
      lo |= 0x20;
   if (isFloatType(i->dType)) {
      if (isFloatType(i->sType))
         lo |= 0x20;
      else
         lo |= 0x80;
   }

   switch (i->op) {
   case OP_SET_AND: hi = 0x10000000; break;
   case OP_SET_OR:  hi = 0x10200000; break;
   case OP_SET_XOR: hi = 0x10400000; break;
   default:
      hi = 0x100e0000;
      break;
   }
   emitForm_A(i, (static_cast<uint64_t>(hi) << 32) | lo);

   if (i->op != OP_SET)
      srcId(i->src(2), 32 + 17);

   if (i->def(0).getFile() == FILE_PREDICATE) {
      if (i->sType == TYPE_F32)
         code[1] += 0x10000000;
      else
         code[1] += 0x08000000;

      code[0] &= ~0xfc000;
      defId(i->def(0), 17);
      if (i->defExists(1))
         defId(i->def(1), 14);
      else
         code[0] |= 0x1c000;
   }

   if (i->ftz)
      code[1] |= 1 << 27;
   if (i->flagsSrc >= 0)
      code[0] |= 1 << 6;

   emitCondCode(i->setCond, 32 + 23);
   emitNegAbs12(i);
}

void
CodeEmitterNVC0::emitSLCT(const CmpInstruction *i)
{
   uint64_t op;

   switch (i->dType) {
   case TYPE_S32:
      op = HEX64(30000000, 00000023);
      break;
   case TYPE_U32:
      op = HEX64(30000000, 00000003);
      break;
   case TYPE_F32:
      op = HEX64(38000000, 00000000);
      break;
   default:
      assert(!"invalid type for SLCT");
      op = 0;
      break;
   }
   emitForm_A(i, op);

   CondCode cc = i->setCond;

   if (i->src(2).mod.neg())
      cc = reverseCondCode(cc);

   emitCondCode(cc, 32 + 23);

   if (i->ftz)
      code[0] |= 1 << 5;
}

void
nvc0_selpFlip(const FixupEntry *entry, uint32_t *code, const FixupData& data)
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
      code[loc + 1] |= 1 << 20;
   else
      code[loc + 1] &= ~(1 << 20);
}

void CodeEmitterNVC0::emitSELP(const Instruction *i)
{
   emitForm_A(i, HEX64(20000000, 00000004));

   if (i->src(2).mod & Modifier(NV50_IR_MOD_NOT))
      code[1] |= 1 << 20;

   if (i->subOp >= 1) {
      addInterp(i->subOp - 1, 0, nvc0_selpFlip);
   }
}

void CodeEmitterNVC0::emitTEXBAR(const Instruction *i)
{
   code[0] = 0x00000006 | (i->subOp << 26);
   code[1] = 0xf0000000;
   emitPredicate(i);
   emitCondCode(i->flagsSrc >= 0 ? i->cc : CC_ALWAYS, 5);
}

void CodeEmitterNVC0::emitTEXCSAA(const TexInstruction *i)
{
   code[0] = 0x00000086;
   code[1] = 0xd0000000;

   code[1] |= i->tex.r;
   code[1] |= i->tex.s << 8;

   if (i->tex.liveOnly)
      code[0] |= 1 << 9;

   defId(i->def(0), 14);
   srcId(i->src(0), 20);
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
CodeEmitterNVC0::emitTEX(const TexInstruction *i)
{
   code[0] = 0x00000006;

   if (isNextIndependentTex(i))
      code[0] |= 0x080; // t mode
   else
      code[0] |= 0x100; // p mode

   if (i->tex.liveOnly)
      code[0] |= 1 << 9;

   switch (i->op) {
   case OP_TEX: code[1] = 0x80000000; break;
   case OP_TXB: code[1] = 0x84000000; break;
   case OP_TXL: code[1] = 0x86000000; break;
   case OP_TXF: code[1] = 0x90000000; break;
   case OP_TXG: code[1] = 0xa0000000; break;
   case OP_TXLQ: code[1] = 0xb0000000; break;
   case OP_TXD: code[1] = 0xe0000000; break;
   default:
      assert(!"invalid texture op");
      break;
   }
   if (i->op == OP_TXF) {
      if (!i->tex.levelZero)
         code[1] |= 0x02000000;
   } else
   if (i->tex.levelZero) {
      code[1] |= 0x02000000;
   }

   if (i->op != OP_TXD && i->tex.derivAll)
      code[1] |= 1 << 13;

   defId(i->def(0), 14);
   srcId(i->src(0), 20);

   emitPredicate(i);

   if (i->op == OP_TXG) code[0] |= i->tex.gatherComp << 5;

   code[1] |= i->tex.mask << 14;

   code[1] |= i->tex.r;
   code[1] |= i->tex.s << 8;
   if (i->tex.rIndirectSrc >= 0 || i->tex.sIndirectSrc >= 0)
      code[1] |= 1 << 18; // in 1st source (with array index)

   // texture target:
   code[1] |= (i->tex.target.getDim() - 1) << 20;
   if (i->tex.target.isCube())
      code[1] += 2 << 20;
   if (i->tex.target.isArray())
      code[1] |= 1 << 19;
   if (i->tex.target.isShadow())
      code[1] |= 1 << 24;

   const int src1 = (i->predSrc == 1) ? 2 : 1; // if predSrc == 1, !srcExists(2)

   if (i->srcExists(src1) && i->src(src1).getFile() == FILE_IMMEDIATE) {
      // lzero
      if (i->op == OP_TXL)
         code[1] &= ~(1 << 26);
      else
      if (i->op == OP_TXF)
         code[1] &= ~(1 << 25);
   }
   if (i->tex.target == TEX_TARGET_2D_MS ||
       i->tex.target == TEX_TARGET_2D_MS_ARRAY)
      code[1] |= 1 << 23;

   if (i->tex.useOffsets == 1)
      code[1] |= 1 << 22;
   if (i->tex.useOffsets == 4)
      code[1] |= 1 << 23;

   srcId(i, src1, 26);
}

void
CodeEmitterNVC0::emitTXQ(const TexInstruction *i)
{
   code[0] = 0x00000086;
   code[1] = 0xc0000000;

   switch (i->tex.query) {
   case TXQ_DIMS:            code[1] |= 0 << 22; break;
   case TXQ_TYPE:            code[1] |= 1 << 22; break;
   case TXQ_SAMPLE_POSITION: code[1] |= 2 << 22; break;
   case TXQ_FILTER:          code[1] |= 3 << 22; break;
   case TXQ_LOD:             code[1] |= 4 << 22; break;
   case TXQ_BORDER_COLOUR:   code[1] |= 5 << 22; break;
   default:
      assert(!"invalid texture query");
      break;
   }

   code[1] |= i->tex.mask << 14;

   code[1] |= i->tex.r;
   code[1] |= i->tex.s << 8;
   if (i->tex.sIndirectSrc >= 0 || i->tex.rIndirectSrc >= 0)
      code[1] |= 1 << 18;

   const int src1 = (i->predSrc == 1) ? 2 : 1; // if predSrc == 1, !srcExists(2)

   defId(i->def(0), 14);
   srcId(i->src(0), 20);
   srcId(i, src1, 26);

   emitPredicate(i);
}

void
CodeEmitterNVC0::emitQUADOP(const Instruction *i, uint8_t qOp, uint8_t laneMask)
{
   code[0] = 0x00000200 | (laneMask << 6); // dall
   code[1] = 0x48000000 | qOp;

   defId(i->def(0), 14);
   srcId(i->src(0), 20);
   srcId((i->srcExists(1) && i->predSrc != 1) ? i->src(1) : i->src(0), 26);

   emitPredicate(i);
}

void
CodeEmitterNVC0::emitFlow(const Instruction *i)
{
   const FlowInstruction *f = i->asFlow();

   unsigned mask; // bit 0: predicate, bit 1: target

   code[0] = 0x00000007;

   switch (i->op) {
   case OP_BRA:
      code[1] = f->absolute ? 0x00000000 : 0x40000000;
      if (i->srcExists(0) && i->src(0).getFile() == FILE_MEMORY_CONST)
         code[0] |= 0x4000;
      mask = 3;
      break;
   case OP_CALL:
      code[1] = f->absolute ? 0x10000000 : 0x50000000;
      if (f->indirect)
         code[0] |= 0x4000; // indirect calls always use c[] source
      mask = 2;
      break;

   case OP_EXIT:    code[1] = 0x80000000; mask = 1; break;
   case OP_RET:     code[1] = 0x90000000; mask = 1; break;
   case OP_DISCARD: code[1] = 0x98000000; mask = 1; break;
   case OP_BREAK:   code[1] = 0xa8000000; mask = 1; break;
   case OP_CONT:    code[1] = 0xb0000000; mask = 1; break;

   case OP_JOINAT:   code[1] = 0x60000000; mask = 2; break;
   case OP_PREBREAK: code[1] = 0x68000000; mask = 2; break;
   case OP_PRECONT:  code[1] = 0x70000000; mask = 2; break;
   case OP_PRERET:   code[1] = 0x78000000; mask = 2; break;

   case OP_QUADON:  code[1] = 0xc0000000; mask = 0; break;
   case OP_QUADPOP: code[1] = 0xc8000000; mask = 0; break;
   case OP_BRKPT:   code[1] = 0xd0000000; mask = 0; break;
   default:
      assert(!"invalid flow operation");
      return;
   }

   if (mask & 1) {
      emitPredicate(i);
      if (i->flagsSrc < 0)
         code[0] |= 0x1e0;
   }

   if (!f)
      return;

   if (f->allWarp)
      code[0] |= 1 << 15;
   if (f->limit)
      code[0] |= 1 << 16;

   if (f->indirect) {
      if (code[0] & 0x4000) {
         assert(i->srcExists(0) && i->src(0).getFile() == FILE_MEMORY_CONST);
         setAddress16(i->src(0));
         code[1] |= i->getSrc(0)->reg.fileIndex << 10;
         if (f->op == OP_BRA)
            srcId(f->src(0).getIndirect(0), 20);
      } else {
         srcId(f, 0, 20);
      }
   }

   if (f->op == OP_CALL) {
      if (f->indirect) {
         // nothing
      } else
      if (f->builtin) {
         assert(f->absolute);
         uint32_t pcAbs = targNVC0->getBuiltinOffset(f->target.builtin);
         addReloc(RelocEntry::TYPE_BUILTIN, 0, pcAbs, 0xfc000000, 26);
         addReloc(RelocEntry::TYPE_BUILTIN, 1, pcAbs, 0x03ffffff, -6);
      } else {
         assert(!f->absolute);
         int32_t pcRel = f->target.fn->binPos - (codeSize + 8);
         code[0] |= (pcRel & 0x3f) << 26;
         code[1] |= (pcRel >> 6) & 0x3ffff;
      }
   } else
   if (mask & 2) {
      int32_t pcRel = f->target.bb->binPos - (codeSize + 8);
      if (writeIssueDelays && !(f->target.bb->binPos & 0x3f))
         pcRel += 8;
      // currently we don't want absolute branches
      assert(!f->absolute);
      code[0] |= (pcRel & 0x3f) << 26;
      code[1] |= (pcRel >> 6) & 0x3ffff;
   }
}

void
CodeEmitterNVC0::emitBAR(const Instruction *i)
{
   Value *rDef = NULL, *pDef = NULL;

   switch (i->subOp) {
   case NV50_IR_SUBOP_BAR_ARRIVE:   code[0] = 0x84; break;
   case NV50_IR_SUBOP_BAR_RED_AND:  code[0] = 0x24; break;
   case NV50_IR_SUBOP_BAR_RED_OR:   code[0] = 0x44; break;
   case NV50_IR_SUBOP_BAR_RED_POPC: code[0] = 0x04; break;
   default:
      code[0] = 0x04;
      assert(i->subOp == NV50_IR_SUBOP_BAR_SYNC);
      break;
   }
   code[1] = 0x50000000;

   code[0] |= 63 << 14;
   code[1] |= 7 << 21;

   emitPredicate(i);

   // barrier id
   if (i->src(0).getFile() == FILE_GPR) {
      srcId(i->src(0), 20);
   } else {
      ImmediateValue *imm = i->getSrc(0)->asImm();
      assert(imm);
      code[0] |= imm->reg.data.u32 << 20;
      code[1] |= 0x8000;
   }

   // thread count
   if (i->src(1).getFile() == FILE_GPR) {
      srcId(i->src(1), 26);
   } else {
      ImmediateValue *imm = i->getSrc(1)->asImm();
      assert(imm);
      assert(imm->reg.data.u32 <= 0xfff);
      code[0] |= imm->reg.data.u32 << 26;
      code[1] |= imm->reg.data.u32 >> 6;
      code[1] |= 0x4000;
   }

   if (i->srcExists(2) && (i->predSrc != 2)) {
      srcId(i->src(2), 32 + 17);
      if (i->src(2).mod == Modifier(NV50_IR_MOD_NOT))
         code[1] |= 1 << 20;
   } else {
      code[1] |= 7 << 17;
   }

   if (i->defExists(0)) {
      if (i->def(0).getFile() == FILE_GPR)
         rDef = i->getDef(0);
      else
         pDef = i->getDef(0);

      if (i->defExists(1)) {
         if (i->def(1).getFile() == FILE_GPR)
            rDef = i->getDef(1);
         else
            pDef = i->getDef(1);
      }
   }
   if (rDef) {
      code[0] &= ~(63 << 14);
      defId(rDef, 14);
   }
   if (pDef) {
      code[1] &= ~(7 << 21);
      defId(pDef, 32 + 21);
   }
}

void
CodeEmitterNVC0::emitAFETCH(const Instruction *i)
{
   code[0] = 0x00000006;
   code[1] = 0x0c000000 | (i->src(0).get()->reg.data.offset & 0x7ff);

   if (i->getSrc(0)->reg.file == FILE_SHADER_OUTPUT)
      code[0] |= 0x200;

   emitPredicate(i);

   defId(i->def(0), 14);
   srcId(i->src(0).getIndirect(0), 20);
}

void
CodeEmitterNVC0::emitPFETCH(const Instruction *i)
{
   uint32_t prim = i->src(0).get()->reg.data.u32;

   code[0] = 0x00000006 | ((prim & 0x3f) << 26);
   code[1] = 0x00000000 | (prim >> 6);

   emitPredicate(i);

   const int src1 = (i->predSrc == 1) ? 2 : 1; // if predSrc == 1, !srcExists(2)

   defId(i->def(0), 14);
   srcId(i, src1, 20);
}

void
CodeEmitterNVC0::emitVFETCH(const Instruction *i)
{
   code[0] = 0x00000006;
   code[1] = 0x06000000 | i->src(0).get()->reg.data.offset;

   if (i->perPatch)
      code[0] |= 0x100;
   if (i->getSrc(0)->reg.file == FILE_SHADER_OUTPUT)
      code[0] |= 0x200; // yes, TCPs can read from *outputs* of other threads

   emitPredicate(i);

   code[0] |= ((i->getDef(0)->reg.size / 4) - 1) << 5;

   defId(i->def(0), 14);
   srcId(i->src(0).getIndirect(0), 20);
   srcId(i->src(0).getIndirect(1), 26); // vertex address
}

void
CodeEmitterNVC0::emitEXPORT(const Instruction *i)
{
   unsigned int size = typeSizeof(i->dType);

   code[0] = 0x00000006 | ((size / 4 - 1) << 5);
   code[1] = 0x0a000000 | i->src(0).get()->reg.data.offset;

   assert(!(code[1] & ((size == 12) ? 15 : (size - 1))));

   if (i->perPatch)
      code[0] |= 0x100;

   emitPredicate(i);

   assert(i->src(1).getFile() == FILE_GPR);

   srcId(i->src(0).getIndirect(0), 20);
   srcId(i->src(0).getIndirect(1), 32 + 17); // vertex base address
   srcId(i->src(1), 26);
}

void
CodeEmitterNVC0::emitOUT(const Instruction *i)
{
   code[0] = 0x00000006;
   code[1] = 0x1c000000;

   emitPredicate(i);

   defId(i->def(0), 14); // new secret address
   srcId(i->src(0), 20); // old secret address, should be 0 initially

   assert(i->src(0).getFile() == FILE_GPR);

   if (i->op == OP_EMIT)
      code[0] |= 1 << 5;
   if (i->op == OP_RESTART || i->subOp == NV50_IR_SUBOP_EMIT_RESTART)
      code[0] |= 1 << 6;

   // vertex stream
   if (i->src(1).getFile() == FILE_IMMEDIATE) {
      unsigned int stream = SDATA(i->src(1)).u32;
      assert(stream < 4);
      if (stream) {
         code[1] |= 0xc000;
         code[0] |= stream << 26;
      } else {
         srcId(NULL, 26);
      }
   } else {
      srcId(i->src(1), 26);
   }
}

void
CodeEmitterNVC0::emitInterpMode(const Instruction *i)
{
   if (i->encSize == 8) {
      code[0] |= i->ipa << 6; // TODO: INTERP_SAMPLEID
   } else {
      if (i->getInterpMode() == NV50_IR_INTERP_SC)
         code[0] |= 0x80;
      assert(i->op == OP_PINTERP && i->getSampleMode() == 0);
   }
}

void
nvc0_interpApply(const FixupEntry *entry, uint32_t *code, const FixupData& data)
{
   int ipa = entry->ipa;
   int reg = entry->reg;
   int loc = entry->loc;

   if (data.flatshade &&
       (ipa & NV50_IR_INTERP_MODE_MASK) == NV50_IR_INTERP_SC) {
      ipa = NV50_IR_INTERP_FLAT;
      reg = 0x3f;
   } else if (data.force_persample_interp &&
              (ipa & NV50_IR_INTERP_SAMPLE_MASK) == NV50_IR_INTERP_DEFAULT &&
              (ipa & NV50_IR_INTERP_MODE_MASK) != NV50_IR_INTERP_FLAT) {
      ipa |= NV50_IR_INTERP_CENTROID;
   }
   code[loc + 0] &= ~(0xf << 6);
   code[loc + 0] |= ipa << 6;
   code[loc + 0] &= ~(0x3f << 26);
   code[loc + 0] |= reg << 26;
}

void
CodeEmitterNVC0::emitINTERP(const Instruction *i)
{
   const uint32_t base = i->getSrc(0)->reg.data.offset;

   if (i->encSize == 8) {
      code[0] = 0x00000000;
      code[1] = 0xc0000000 | (base & 0xffff);

      if (i->saturate)
         code[0] |= 1 << 5;

      if (i->op == OP_PINTERP) {
         srcId(i->src(1), 26);
         addInterp(i->ipa, SDATA(i->src(1)).id, nvc0_interpApply);
      } else {
         code[0] |= 0x3f << 26;
         addInterp(i->ipa, 0x3f, nvc0_interpApply);
      }

      srcId(i->src(0).getIndirect(0), 20);
   } else {
      assert(i->op == OP_PINTERP);
      code[0] = 0x00000009 | ((base & 0xc) << 6) | ((base >> 4) << 26);
      srcId(i->src(1), 20);
   }
   emitInterpMode(i);

   emitPredicate(i);
   defId(i->def(0), 14);

   if (i->getSampleMode() == NV50_IR_INTERP_OFFSET)
      srcId(i->src(i->op == OP_PINTERP ? 2 : 1), 32 + 17);
   else
      code[1] |= 0x3f << 17;
}

void
CodeEmitterNVC0::emitLoadStoreType(DataType ty)
{
   uint8_t val;

   switch (ty) {
   case TYPE_U8:
      val = 0x00;
      break;
   case TYPE_S8:
      val = 0x20;
      break;
   case TYPE_F16:
   case TYPE_U16:
      val = 0x40;
      break;
   case TYPE_S16:
      val = 0x60;
      break;
   case TYPE_F32:
   case TYPE_U32:
   case TYPE_S32:
      val = 0x80;
      break;
   case TYPE_F64:
   case TYPE_U64:
   case TYPE_S64:
      val = 0xa0;
      break;
   case TYPE_B128:
      val = 0xc0;
      break;
   default:
      val = 0x80;
      assert(!"invalid type");
      break;
   }
   code[0] |= val;
}

void
CodeEmitterNVC0::emitCachingMode(CacheMode c)
{
   uint32_t val;

   switch (c) {
   case CACHE_CA:
// case CACHE_WB:
      val = 0x000;
      break;
   case CACHE_CG:
      val = 0x100;
      break;
   case CACHE_CS:
      val = 0x200;
      break;
   case CACHE_CV:
// case CACHE_WT:
      val = 0x300;
      break;
   default:
      val = 0;
      assert(!"invalid caching mode");
      break;
   }
   code[0] |= val;
}

static inline bool
uses64bitAddress(const Instruction *ldst)
{
   return ldst->src(0).getFile() == FILE_MEMORY_GLOBAL &&
      ldst->src(0).isIndirect(0) &&
      ldst->getIndirect(0, 0)->reg.size == 8;
}

void
CodeEmitterNVC0::emitSTORE(const Instruction *i)
{
   uint32_t opc;

   switch (i->src(0).getFile()) {
   case FILE_MEMORY_GLOBAL: opc = 0x90000000; break;
   case FILE_MEMORY_LOCAL:  opc = 0xc8000000; break;
   case FILE_MEMORY_SHARED:
      if (i->subOp == NV50_IR_SUBOP_STORE_UNLOCKED) {
         if (targ->getChipset() >= NVISA_GK104_CHIPSET)
            opc = 0xb8000000;
         else
            opc = 0xcc000000;
      } else {
         opc = 0xc9000000;
      }
      break;
   default:
      assert(!"invalid memory file");
      opc = 0;
      break;
   }
   code[0] = 0x00000005;
   code[1] = opc;

   if (targ->getChipset() >= NVISA_GK104_CHIPSET) {
      // Unlocked store on shared memory can fail.
      if (i->src(0).getFile() == FILE_MEMORY_SHARED &&
          i->subOp == NV50_IR_SUBOP_STORE_UNLOCKED) {
         assert(i->defExists(0));
         setPDSTL(i, 0);
      }
   }

   setAddressByFile(i->src(0));
   srcId(i->src(1), 14);
   srcId(i->src(0).getIndirect(0), 20);
   if (uses64bitAddress(i))
      code[1] |= 1 << 26;

   emitPredicate(i);

   emitLoadStoreType(i->dType);
   emitCachingMode(i->cache);
}

void
CodeEmitterNVC0::emitLOAD(const Instruction *i)
{
   uint32_t opc;

   code[0] = 0x00000005;

   switch (i->src(0).getFile()) {
   case FILE_MEMORY_GLOBAL: opc = 0x80000000; break;
   case FILE_MEMORY_LOCAL:  opc = 0xc0000000; break;
   case FILE_MEMORY_SHARED:
      if (i->subOp == NV50_IR_SUBOP_LOAD_LOCKED) {
         if (targ->getChipset() >= NVISA_GK104_CHIPSET)
            opc = 0xa8000000;
         else
            opc = 0xc4000000;
      } else {
         opc = 0xc1000000;
      }
      break;
   case FILE_MEMORY_CONST:
      if (!i->src(0).isIndirect(0) && typeSizeof(i->dType) == 4) {
         emitMOV(i); // not sure if this is any better
         return;
      }
      opc = 0x14000000 | (i->src(0).get()->reg.fileIndex << 10);
      code[0] = 0x00000006 | (i->subOp << 8);
      break;
   default:
      assert(!"invalid memory file");
      opc = 0;
      break;
   }
   code[1] = opc;

   int r = 0, p = -1;
   if (i->src(0).getFile() == FILE_MEMORY_SHARED) {
      if (i->subOp == NV50_IR_SUBOP_LOAD_LOCKED) {
         if (i->def(0).getFile() == FILE_PREDICATE) { // p, #
            r = -1;
            p = 0;
         } else if (i->defExists(1)) { // r, p
            p = 1;
         } else {
            assert(!"Expected predicate dest for load locked");
         }
      }
   }

   if (r >= 0)
      defId(i->def(r), 14);
   else
      code[0] |= 63 << 14;

   if (p >= 0) {
      if (targ->getChipset() >= NVISA_GK104_CHIPSET)
         setPDSTL(i, p);
      else
         defId(i->def(p), 32 + 18);
   }

   setAddressByFile(i->src(0));
   srcId(i->src(0).getIndirect(0), 20);
   if (uses64bitAddress(i))
      code[1] |= 1 << 26;

   emitPredicate(i);

   emitLoadStoreType(i->dType);
   emitCachingMode(i->cache);
}

uint8_t
CodeEmitterNVC0::getSRegEncoding(const ValueRef& ref)
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
CodeEmitterNVC0::emitMOV(const Instruction *i)
{
   assert(!i->saturate);
   if (i->def(0).getFile() == FILE_PREDICATE) {
      if (i->src(0).getFile() == FILE_GPR) {
         code[0] = 0xfc01c003;
         code[1] = 0x1a8e0000;
         srcId(i->src(0), 20);
      } else {
         code[0] = 0x0001c004;
         code[1] = 0x0c0e0000;
         if (i->src(0).getFile() == FILE_IMMEDIATE) {
            code[0] |= 7 << 20;
            if (!i->getSrc(0)->reg.data.u32)
               code[0] |= 1 << 23;
         } else {
            srcId(i->src(0), 20);
         }
      }
      defId(i->def(0), 17);
      emitPredicate(i);
   } else
   if (i->src(0).getFile() == FILE_SYSTEM_VALUE) {
      uint8_t sr = getSRegEncoding(i->src(0));

      if (i->encSize == 8) {
         code[0] = 0x00000004 | (sr << 26);
         code[1] = 0x2c000000;
      } else {
         code[0] = 0x40000008 | (sr << 20);
      }
      defId(i->def(0), 14);

      emitPredicate(i);
   } else
   if (i->encSize == 8) {
      uint64_t opc;

      if (i->src(0).getFile() == FILE_IMMEDIATE)
         opc = HEX64(18000000, 000001e2);
      else
      if (i->src(0).getFile() == FILE_PREDICATE)
         opc = HEX64(080e0000, 1c000004);
      else
         opc = HEX64(28000000, 00000004);

      if (i->src(0).getFile() != FILE_PREDICATE)
         opc |= i->lanes << 5;

      emitForm_B(i, opc);

      // Explicitly emit the predicate source as emitForm_B skips it.
      if (i->src(0).getFile() == FILE_PREDICATE)
         srcId(i->src(0), 20);
   } else {
      uint32_t imm;

      if (i->src(0).getFile() == FILE_IMMEDIATE) {
         imm = SDATA(i->src(0)).u32;
         if (imm & 0xfff00000) {
            assert(!(imm & 0x000fffff));
            code[0] = 0x00000318 | imm;
         } else {
            assert(imm < 0x800 && ((int32_t)imm >= -0x800));
            code[0] = 0x00000118 | (imm << 20);
         }
      } else {
         code[0] = 0x0028;
         emitShortSrc2(i->src(0));
      }
      defId(i->def(0), 14);

      emitPredicate(i);
   }
}

void
CodeEmitterNVC0::emitATOM(const Instruction *i)
{
   const bool hasDst = i->defExists(0);
   const bool casOrExch =
      i->subOp == NV50_IR_SUBOP_ATOM_EXCH ||
      i->subOp == NV50_IR_SUBOP_ATOM_CAS;

   if (i->dType == TYPE_U64) {
      switch (i->subOp) {
      case NV50_IR_SUBOP_ATOM_ADD:
         code[0] = 0x205;
         if (hasDst)
            code[1] = 0x507e0000;
         else
            code[1] = 0x10000000;
         break;
      case NV50_IR_SUBOP_ATOM_EXCH:
         code[0] = 0x305;
         code[1] = 0x507e0000;
         break;
      case NV50_IR_SUBOP_ATOM_CAS:
         code[0] = 0x325;
         code[1] = 0x50000000;
         break;
      default:
         assert(!"invalid u64 red op");
         break;
      }
   } else
   if (i->dType == TYPE_U32) {
      switch (i->subOp) {
      case NV50_IR_SUBOP_ATOM_EXCH:
         code[0] = 0x105;
         code[1] = 0x507e0000;
         break;
      case NV50_IR_SUBOP_ATOM_CAS:
         code[0] = 0x125;
         code[1] = 0x50000000;
         break;
      default:
         code[0] = 0x5 | (i->subOp << 5);
         if (hasDst)
            code[1] = 0x507e0000;
         else
            code[1] = 0x10000000;
         break;
      }
   } else
   if (i->dType == TYPE_S32) {
      assert(i->subOp <= 2);
      code[0] = 0x205 | (i->subOp << 5);
      if (hasDst)
         code[1] = 0x587e0000;
      else
         code[1] = 0x18000000;
   } else
   if (i->dType == TYPE_F32) {
      assert(i->subOp == NV50_IR_SUBOP_ATOM_ADD);
      code[0] = 0x205;
      if (hasDst)
         code[1] = 0x687e0000;
      else
         code[1] = 0x28000000;
   }

   emitPredicate(i);

   srcId(i->src(1), 14);

   if (hasDst)
      defId(i->def(0), 32 + 11);
   else
   if (casOrExch)
      code[1] |= 63 << 11;

   if (hasDst || casOrExch) {
      const int32_t offset = SDATA(i->src(0)).offset;
      assert(offset < 0x80000 && offset >= -0x80000);
      code[0] |= offset << 26;
      code[1] |= (offset & 0x1ffc0) >> 6;
      code[1] |= (offset & 0xe0000) << 6;
   } else {
      srcAddr32(i->src(0), 26, 0);
   }
   if (i->getIndirect(0, 0)) {
      srcId(i->getIndirect(0, 0), 20);
      if (i->getIndirect(0, 0)->reg.size == 8)
         code[1] |= 1 << 26;
   } else {
      code[0] |= 63 << 20;
   }

   if (i->subOp == NV50_IR_SUBOP_ATOM_CAS) {
      assert(i->src(1).getSize() == 2 * typeSizeof(i->sType));
      code[1] |= (SDATA(i->src(1)).id + 1) << 17;
   }
}

void
CodeEmitterNVC0::emitMEMBAR(const Instruction *i)
{
   switch (NV50_IR_SUBOP_MEMBAR_SCOPE(i->subOp)) {
   case NV50_IR_SUBOP_MEMBAR_CTA: code[0] = 0x05; break;
   case NV50_IR_SUBOP_MEMBAR_GL:  code[0] = 0x25; break;
   default:
      code[0] = 0x45;
      assert(NV50_IR_SUBOP_MEMBAR_SCOPE(i->subOp) == NV50_IR_SUBOP_MEMBAR_SYS);
      break;
   }
   code[1] = 0xe0000000;

   emitPredicate(i);
}

void
CodeEmitterNVC0::emitCCTL(const Instruction *i)
{
   code[0] = 0x00000005 | (i->subOp << 5);

   if (i->src(0).getFile() == FILE_MEMORY_GLOBAL) {
      code[1] = 0x98000000;
      srcAddr32(i->src(0), 28, 2);
   } else {
      code[1] = 0xd0000000;
      setAddress24(i->src(0));
   }
   if (uses64bitAddress(i))
      code[1] |= 1 << 26;
   srcId(i->src(0).getIndirect(0), 20);

   emitPredicate(i);

   defId(i, 0, 14);
}

void
CodeEmitterNVC0::emitSUCLAMPMode(uint16_t subOp)
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
   code[0] |= m << 5;
   if (subOp & NV50_IR_SUBOP_SUCLAMP_2D)
      code[1] |= 1 << 16;
}

void
CodeEmitterNVC0::emitSUCalc(Instruction *i)
{
   ImmediateValue *imm = NULL;
   uint64_t opc;

   if (i->srcExists(2)) {
      imm = i->getSrc(2)->asImm();
      if (imm)
         i->setSrc(2, NULL); // special case, make emitForm_A not assert
   }

   switch (i->op) {
   case OP_SUCLAMP: opc = HEX64(58000000, 00000004); break;
   case OP_SUBFM: opc = HEX64(5c000000, 00000004); break;
   case OP_SUEAU: opc = HEX64(60000000, 00000004); break;
   default:
      assert(0);
      return;
   }
   emitForm_A(i, opc);

   if (i->op == OP_SUCLAMP) {
      if (i->dType == TYPE_S32)
         code[0] |= 1 << 9;
      emitSUCLAMPMode(i->subOp);
   }

   if (i->op == OP_SUBFM && i->subOp == NV50_IR_SUBOP_SUBFM_3D)
         code[1] |= 1 << 16;

   if (i->op != OP_SUEAU) {
      if (i->def(0).getFile() == FILE_PREDICATE) { // p, #
         code[0] |= 63 << 14;
         code[1] |= i->getDef(0)->reg.data.id << 23;
      } else
      if (i->defExists(1)) { // r, p
         assert(i->def(1).getFile() == FILE_PREDICATE);
         code[1] |= i->getDef(1)->reg.data.id << 23;
      } else { // r, #
         code[1] |= 7 << 23;
      }
   }
   if (imm) {
      assert(i->op == OP_SUCLAMP);
      i->setSrc(2, imm);
      code[1] |= (imm->reg.data.u32 & 0x3f) << 17; // sint6
   }
}

void
CodeEmitterNVC0::emitSUGType(DataType ty)
{
   switch (ty) {
   case TYPE_S32: code[1] |= 1 << 13; break;
   case TYPE_U8:  code[1] |= 2 << 13; break;
   case TYPE_S8:  code[1] |= 3 << 13; break;
   default:
      assert(ty == TYPE_U32);
      break;
   }
}

void
CodeEmitterNVC0::setSUConst16(const Instruction *i, const int s)
{
   const uint32_t offset = i->getSrc(s)->reg.data.offset;

   assert(i->src(s).getFile() == FILE_MEMORY_CONST);
   assert(offset == (offset & 0xfffc));

   code[1] |= 1 << 21;
   code[0] |= offset << 24;
   code[1] |= offset >> 8;
   code[1] |= i->getSrc(s)->reg.fileIndex << 8;
}

void
CodeEmitterNVC0::setSUPred(const Instruction *i, const int s)
{
   if (!i->srcExists(s) || (i->predSrc == s)) {
      code[1] |= 0x7 << 17;
   } else {
      if (i->src(s).mod == Modifier(NV50_IR_MOD_NOT))
         code[1] |= 1 << 20;
      srcId(i->src(s), 32 + 17);
   }
}

void
CodeEmitterNVC0::emitSULDGB(const TexInstruction *i)
{
   code[0] = 0x5;
   code[1] = 0xd4000000 | (i->subOp << 15);

   emitLoadStoreType(i->dType);
   emitSUGType(i->sType);
   emitCachingMode(i->cache);

   emitPredicate(i);
   defId(i->def(0), 14); // destination
   srcId(i->src(0), 20); // address
   // format
   if (i->src(1).getFile() == FILE_GPR)
      srcId(i->src(1), 26);
   else
      setSUConst16(i, 1);
   setSUPred(i, 2);
}

void
CodeEmitterNVC0::emitSUSTGx(const TexInstruction *i)
{
   code[0] = 0x5;
   code[1] = 0xdc000000 | (i->subOp << 15);

   if (i->op == OP_SUSTP)
      code[1] |= i->tex.mask << 22;
   else
      emitLoadStoreType(i->dType);
   emitSUGType(i->sType);
   emitCachingMode(i->cache);

   emitPredicate(i);
   srcId(i->src(0), 20); // address
   // format
   if (i->src(1).getFile() == FILE_GPR)
      srcId(i->src(1), 26);
   else
      setSUConst16(i, 1);
   srcId(i->src(3), 14); // values
   setSUPred(i, 2);
}

void
CodeEmitterNVC0::emitSUAddr(const TexInstruction *i)
{
   assert(targ->getChipset() < NVISA_GK104_CHIPSET);

   if (i->tex.rIndirectSrc < 0) {
      code[1] |= 0x00004000;
      code[0] |= i->tex.r << 26;
   } else {
      srcId(i, i->tex.rIndirectSrc, 26);
   }
}

void
CodeEmitterNVC0::emitSUDim(const TexInstruction *i)
{
   assert(targ->getChipset() < NVISA_GK104_CHIPSET);

   code[1] |= (i->tex.target.getDim() - 1) << 12;
   if (i->tex.target.isArray() || i->tex.target.isCube() ||
       i->tex.target.getDim() == 3) {
      // use e2d mode for 3-dim images, arrays and cubes.
      code[1] |= 3 << 12;
   }

   srcId(i->src(0), 20);
}

void
CodeEmitterNVC0::emitSULEA(const TexInstruction *i)
{
   assert(targ->getChipset() < NVISA_GK104_CHIPSET);

   code[0] = 0x5;
   code[1] = 0xf0000000;

   emitPredicate(i);
   emitLoadStoreType(i->sType);

   defId(i->def(0), 14);

   if (i->defExists(1)) {
      defId(i->def(1), 32 + 22);
   } else {
      code[1] |= 7 << 22;
   }

   emitSUAddr(i);
   emitSUDim(i);
}

void
CodeEmitterNVC0::emitSULDB(const TexInstruction *i)
{
   assert(targ->getChipset() < NVISA_GK104_CHIPSET);

   code[0] = 0x5;
   code[1] = 0xd4000000 | (i->subOp << 15);

   emitPredicate(i);
   emitLoadStoreType(i->dType);

   defId(i->def(0), 14);

   emitCachingMode(i->cache);
   emitSUAddr(i);
   emitSUDim(i);
}

void
CodeEmitterNVC0::emitSUSTx(const TexInstruction *i)
{
   assert(targ->getChipset() < NVISA_GK104_CHIPSET);

   code[0] = 0x5;
   code[1] = 0xdc000000 | (i->subOp << 15);

   if (i->op == OP_SUSTP)
      code[1] |= i->tex.mask << 17;
   else
      emitLoadStoreType(i->dType);

   emitPredicate(i);

   srcId(i->src(1), 14);

   emitCachingMode(i->cache);
   emitSUAddr(i);
   emitSUDim(i);
}

void
CodeEmitterNVC0::emitVectorSubOp(const Instruction *i)
{
   switch (NV50_IR_SUBOP_Vn(i->subOp)) {
   case 0:
      code[1] |= (i->subOp & 0x000f) << 12; // vsrc1
      code[1] |= (i->subOp & 0x00e0) >> 5;  // vsrc2
      code[1] |= (i->subOp & 0x0100) << 7;  // vsrc2
      code[1] |= (i->subOp & 0x3c00) << 13; // vdst
      break;
   case 1:
      code[1] |= (i->subOp & 0x000f) << 8;  // v2src1
      code[1] |= (i->subOp & 0x0010) << 11; // v2src1
      code[1] |= (i->subOp & 0x01e0) >> 1;  // v2src2
      code[1] |= (i->subOp & 0x0200) << 6;  // v2src2
      code[1] |= (i->subOp & 0x3c00) << 2;  // v4dst
      code[1] |= (i->mask & 0x3) << 2;
      break;
   case 2:
      code[1] |= (i->subOp & 0x000f) << 8; // v4src1
      code[1] |= (i->subOp & 0x01e0) >> 1; // v4src2
      code[1] |= (i->subOp & 0x3c00) << 2; // v4dst
      code[1] |= (i->mask & 0x3) << 2;
      code[1] |= (i->mask & 0xc) << 21;
      break;
   default:
      assert(0);
      break;
   }
}

void
CodeEmitterNVC0::emitVSHL(const Instruction *i)
{
   uint64_t opc = 0x4;

   switch (NV50_IR_SUBOP_Vn(i->subOp)) {
   case 0: opc |= 0xe8ULL << 56; break;
   case 1: opc |= 0xb4ULL << 56; break;
   case 2: opc |= 0x94ULL << 56; break;
   default:
      assert(0);
      break;
   }
   if (NV50_IR_SUBOP_Vn(i->subOp) == 1) {
      if (isSignedType(i->dType)) opc |= 1ULL << 0x2a;
      if (isSignedType(i->sType)) opc |= (1 << 6) | (1 << 5);
   } else {
      if (isSignedType(i->dType)) opc |= 1ULL << 0x39;
      if (isSignedType(i->sType)) opc |= 1 << 6;
   }
   emitForm_A(i, opc);
   emitVectorSubOp(i);

   if (i->saturate)
      code[0] |= 1 << 9;
   if (i->flagsDef >= 0)
      code[1] |= 1 << 16;
}

void
CodeEmitterNVC0::emitPIXLD(const Instruction *i)
{
   assert(i->encSize == 8);
   emitForm_A(i, HEX64(10000000, 00000006));
   code[0] |= i->subOp << 5;
   code[1] |= 0x00e00000;
}

void
CodeEmitterNVC0::emitSHFL(const Instruction *i)
{
   const ImmediateValue *imm;

   assert(targ->getChipset() >= NVISA_GK104_CHIPSET);

   code[0] = 0x00000005;
   code[1] = 0x88000000 | (i->subOp << 23);

   emitPredicate(i);

   defId(i->def(0), 14);
   srcId(i->src(0), 20);

   switch (i->src(1).getFile()) {
   case FILE_GPR:
      srcId(i->src(1), 26);
      break;
   case FILE_IMMEDIATE:
      imm = i->getSrc(1)->asImm();
      assert(imm && imm->reg.data.u32 < 0x20);
      code[0] |= imm->reg.data.u32 << 26;
      code[0] |= 1 << 5;
      break;
   default:
      assert(!"invalid src1 file");
      break;
   }

   switch (i->src(2).getFile()) {
   case FILE_GPR:
      srcId(i->src(2), 49);
      break;
   case FILE_IMMEDIATE:
      imm = i->getSrc(2)->asImm();
      assert(imm && imm->reg.data.u32 < 0x2000);
      code[1] |= imm->reg.data.u32 << 10;
      code[0] |= 1 << 6;
      break;
   default:
      assert(!"invalid src2 file");
      break;
   }

   setPDSTL(i, i->defExists(1) ? 1 : -1);
}

void
CodeEmitterNVC0::emitVOTE(const Instruction *i)
{
   const ImmediateValue *imm;
   uint32_t u32;

   code[0] = 0x00000004 | (i->subOp << 5);
   code[1] = 0x48000000;

   emitPredicate(i);

   unsigned rp = 0;
   for (int d = 0; i->defExists(d); d++) {
      if (i->def(d).getFile() == FILE_PREDICATE) {
         assert(!(rp & 2));
         rp |= 2;
         defId(i->def(d), 32 + 22);
      } else if (i->def(d).getFile() == FILE_GPR) {
         assert(!(rp & 1));
         rp |= 1;
         defId(i->def(d), 14);
      } else {
         assert(!"Unhandled def");
      }
   }
   if (!(rp & 1))
      code[0] |= 63 << 14;
   if (!(rp & 2))
      code[1] |= 7 << 22;

   switch (i->src(0).getFile()) {
   case FILE_PREDICATE:
      if (i->src(0).mod == Modifier(NV50_IR_MOD_NOT))
         code[0] |= 1 << 23;
      srcId(i->src(0), 20);
      break;
   case FILE_IMMEDIATE:
      imm = i->getSrc(0)->asImm();
      assert(imm);
      u32 = imm->reg.data.u32;
      assert(u32 == 0 || u32 == 1);
      code[0] |= (u32 == 1 ? 0x7 : 0xf) << 20;
      break;
   default:
      assert(!"Unhandled src");
      break;
   }
}

bool
CodeEmitterNVC0::emitInstruction(Instruction *insn)
{
   unsigned int size = insn->encSize;

   if (writeIssueDelays && !(codeSize & 0x3f))
      size += 8;

   if (!insn->encSize) {
      ERROR("skipping unencodable instruction: "); insn->print();
      return false;
   } else
   if (codeSize + size > codeSizeLimit) {
      ERROR("code emitter output buffer too small\n");
      return false;
   }

   if (writeIssueDelays) {
      if (!(codeSize & 0x3f)) {
         code[0] = 0x00000007; // cf issue delay "instruction"
         code[1] = 0x20000000;
         code += 2;
         codeSize += 8;
      }
      const unsigned int id = (codeSize & 0x3f) / 8 - 1;
      uint32_t *data = code - (id * 2 + 2);
      if (id <= 2) {
         data[0] |= insn->sched << (id * 8 + 4);
      } else
      if (id == 3) {
         data[0] |= insn->sched << 28;
         data[1] |= insn->sched >> 4;
      } else {
         data[1] |= insn->sched << ((id - 4) * 8 + 4);
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
   case OP_PFETCH:
      emitPFETCH(insn);
      break;
   case OP_AFETCH:
      emitAFETCH(insn);
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
         emitUMUL(insn);
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
   case OP_SUBFM:
   case OP_SUCLAMP:
   case OP_SUEAU:
      emitSUCalc(insn);
      break;
   case OP_MADSP:
      emitMADSP(insn);
      break;
   case OP_SULDB:
      if (targ->getChipset() >= NVISA_GK104_CHIPSET)
         emitSULDGB(insn->asTex());
      else
         emitSULDB(insn->asTex());
      break;
   case OP_SUSTB:
   case OP_SUSTP:
      if (targ->getChipset() >= NVISA_GK104_CHIPSET)
         emitSUSTGx(insn->asTex());
      else
         emitSUSTx(insn->asTex());
      break;
   case OP_SULEA:
      emitSULEA(insn->asTex());
      break;
   case OP_ATOM:
      emitATOM(insn);
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
   case OP_CCTL:
      emitCCTL(insn);
      break;
   case OP_VSHL:
      emitVSHL(insn);
      break;
   case OP_PIXLD:
      emitPIXLD(insn);
      break;
   case OP_SHFL:
      emitSHFL(insn);
      break;
   case OP_VOTE:
      emitVOTE(insn);
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

   if (insn->join) {
      code[0] |= 0x10;
      assert(insn->encSize == 8);
   }

   code += insn->encSize / 4;
   codeSize += insn->encSize;
   return true;
}

uint32_t
CodeEmitterNVC0::getMinEncodingSize(const Instruction *i) const
{
   const Target::OpInfo &info = targ->getOpInfo(i);

   if (writeIssueDelays || info.minEncSize == 8 || true)
      return 8;

   if (i->ftz || i->saturate || i->join)
      return 8;
   if (i->rnd != ROUND_N)
      return 8;
   if (i->predSrc >= 0 && i->op == OP_MAD)
      return 8;

   if (i->op == OP_PINTERP) {
      if (i->getSampleMode() || true) // XXX: grr, short op doesn't work
         return 8;
   } else
   if (i->op == OP_MOV && i->lanes != 0xf) {
      return 8;
   }

   for (int s = 0; i->srcExists(s); ++s) {
      if (i->src(s).isIndirect(0))
         return 8;

      if (i->src(s).getFile() == FILE_MEMORY_CONST) {
         if (SDATA(i->src(s)).offset >= 0x100)
            return 8;
         if (i->getSrc(s)->reg.fileIndex > 1 &&
             i->getSrc(s)->reg.fileIndex != 16)
             return 8;
      } else
      if (i->src(s).getFile() == FILE_IMMEDIATE) {
         if (i->dType == TYPE_F32) {
            if (SDATA(i->src(s)).u32 >= 0x100)
               return 8;
         } else {
            if (SDATA(i->src(s)).u32 > 0xff)
               return 8;
         }
      }

      if (i->op == OP_CVT)
         continue;
      if (i->src(s).mod != Modifier(0)) {
         if (i->src(s).mod == Modifier(NV50_IR_MOD_ABS))
            if (i->op != OP_RSQ)
               return 8;
         if (i->src(s).mod == Modifier(NV50_IR_MOD_NEG))
            if (i->op != OP_ADD || s != 0)
               return 8;
      }
   }

   return 4;
}

// Simplified, erring on safe side.
class SchedDataCalculator : public Pass
{
public:
   SchedDataCalculator(const Target *targ) : score(NULL), prevData(0),
      prevOp(OP_NOP), targ(targ) { }

private:
   struct RegScores
   {
      struct Resource {
         int st[DATA_FILE_COUNT]; // LD to LD delay 3
         int ld[DATA_FILE_COUNT]; // ST to ST delay 3
         int tex; // TEX to non-TEX delay 17 (0x11)
         int sfu; // SFU to SFU delay 3 (except PRE-ops)
         int imul; // integer MUL to MUL delay 3
      } res;
      struct ScoreData {
         int r[256];
         int p[8];
         int c;
      } rd, wr;
      int base;
      int regs;

      void rebase(const int base)
      {
         const int delta = this->base - base;
         if (!delta)
            return;
         this->base = 0;

         for (int i = 0; i < regs; ++i) {
            rd.r[i] += delta;
            wr.r[i] += delta;
         }
         for (int i = 0; i < 8; ++i) {
            rd.p[i] += delta;
            wr.p[i] += delta;
         }
         rd.c += delta;
         wr.c += delta;

         for (unsigned int f = 0; f < DATA_FILE_COUNT; ++f) {
            res.ld[f] += delta;
            res.st[f] += delta;
         }
         res.sfu += delta;
         res.imul += delta;
         res.tex += delta;
      }
      void wipe(int regs)
      {
         memset(&rd, 0, sizeof(rd));
         memset(&wr, 0, sizeof(wr));
         memset(&res, 0, sizeof(res));
         this->regs = regs;
      }
      int getLatest(const ScoreData& d) const
      {
         int max = 0;
         for (int i = 0; i < regs; ++i)
            if (d.r[i] > max)
               max = d.r[i];
         for (int i = 0; i < 8; ++i)
            if (d.p[i] > max)
               max = d.p[i];
         if (d.c > max)
            max = d.c;
         return max;
      }
      inline int getLatestRd() const
      {
         return getLatest(rd);
      }
      inline int getLatestWr() const
      {
         return getLatest(wr);
      }
      inline int getLatest() const
      {
         const int a = getLatestRd();
         const int b = getLatestWr();

         int max = MAX2(a, b);
         for (unsigned int f = 0; f < DATA_FILE_COUNT; ++f) {
            max = MAX2(res.ld[f], max);
            max = MAX2(res.st[f], max);
         }
         max = MAX2(res.sfu, max);
         max = MAX2(res.imul, max);
         max = MAX2(res.tex, max);
         return max;
      }
      void setMax(const RegScores *that)
      {
         for (int i = 0; i < regs; ++i) {
            rd.r[i] = MAX2(rd.r[i], that->rd.r[i]);
            wr.r[i] = MAX2(wr.r[i], that->wr.r[i]);
         }
         for (int i = 0; i < 8; ++i) {
            rd.p[i] = MAX2(rd.p[i], that->rd.p[i]);
            wr.p[i] = MAX2(wr.p[i], that->wr.p[i]);
         }
         rd.c = MAX2(rd.c, that->rd.c);
         wr.c = MAX2(wr.c, that->wr.c);

         for (unsigned int f = 0; f < DATA_FILE_COUNT; ++f) {
            res.ld[f] = MAX2(res.ld[f], that->res.ld[f]);
            res.st[f] = MAX2(res.st[f], that->res.st[f]);
         }
         res.sfu = MAX2(res.sfu, that->res.sfu);
         res.imul = MAX2(res.imul, that->res.imul);
         res.tex = MAX2(res.tex, that->res.tex);
      }
      void print(int cycle)
      {
         for (int i = 0; i < regs; ++i) {
            if (rd.r[i] > cycle)
               INFO("rd $r%i @ %i\n", i, rd.r[i]);
            if (wr.r[i] > cycle)
               INFO("wr $r%i @ %i\n", i, wr.r[i]);
         }
         for (int i = 0; i < 8; ++i) {
            if (rd.p[i] > cycle)
               INFO("rd $p%i @ %i\n", i, rd.p[i]);
            if (wr.p[i] > cycle)
               INFO("wr $p%i @ %i\n", i, wr.p[i]);
         }
         if (rd.c > cycle)
            INFO("rd $c @ %i\n", rd.c);
         if (wr.c > cycle)
            INFO("wr $c @ %i\n", wr.c);
         if (res.sfu > cycle)
            INFO("sfu @ %i\n", res.sfu);
         if (res.imul > cycle)
            INFO("imul @ %i\n", res.imul);
         if (res.tex > cycle)
            INFO("tex @ %i\n", res.tex);
      }
   };

   RegScores *score; // for current BB
   std::vector<RegScores> scoreBoards;
   int prevData;
   operation prevOp;

   const Target *targ;

   bool visit(Function *);
   bool visit(BasicBlock *);

   void commitInsn(const Instruction *, int cycle);
   int calcDelay(const Instruction *, int cycle) const;
   void setDelay(Instruction *, int delay, Instruction *next);

   void recordRd(const Value *, const int ready);
   void recordWr(const Value *, const int ready);
   void checkRd(const Value *, int cycle, int& delay) const;
   void checkWr(const Value *, int cycle, int& delay) const;

   int getCycles(const Instruction *, int origDelay) const;
};

void
SchedDataCalculator::setDelay(Instruction *insn, int delay, Instruction *next)
{
   if (insn->op == OP_EXIT || insn->op == OP_RET)
      delay = MAX2(delay, 14);

   if (insn->op == OP_TEXBAR) {
      // TODO: except if results not used before EXIT
      insn->sched = 0xc2;
   } else
   if (insn->op == OP_JOIN || insn->join) {
      insn->sched = 0x00;
   } else
   if (delay >= 0 || prevData == 0x04 ||
       !next || !targ->canDualIssue(insn, next)) {
      insn->sched = static_cast<uint8_t>(MAX2(delay, 0));
      if (prevOp == OP_EXPORT)
         insn->sched |= 0x40;
      else
         insn->sched |= 0x20;
   } else {
      insn->sched = 0x04; // dual-issue
   }

   if (prevData != 0x04 || prevOp != OP_EXPORT)
      if (insn->sched != 0x04 || insn->op == OP_EXPORT)
         prevOp = insn->op;

   prevData = insn->sched;
}

int
SchedDataCalculator::getCycles(const Instruction *insn, int origDelay) const
{
   if (insn->sched & 0x80) {
      int c = (insn->sched & 0x0f) * 2 + 1;
      if (insn->op == OP_TEXBAR && origDelay > 0)
         c += origDelay;
      return c;
   }
   if (insn->sched & 0x60)
      return (insn->sched & 0x1f) + 1;
   return (insn->sched == 0x04) ? 0 : 32;
}

bool
SchedDataCalculator::visit(Function *func)
{
   int regs = targ->getFileSize(FILE_GPR) + 1;
   scoreBoards.resize(func->cfg.getSize());
   for (size_t i = 0; i < scoreBoards.size(); ++i)
      scoreBoards[i].wipe(regs);
   return true;
}

bool
SchedDataCalculator::visit(BasicBlock *bb)
{
   Instruction *insn;
   Instruction *next = NULL;

   int cycle = 0;

   prevData = 0x00;
   prevOp = OP_NOP;
   score = &scoreBoards.at(bb->getId());

   for (Graph::EdgeIterator ei = bb->cfg.incident(); !ei.end(); ei.next()) {
      // back branches will wait until all target dependencies are satisfied
      if (ei.getType() == Graph::Edge::BACK) // sched would be uninitialized
         continue;
      BasicBlock *in = BasicBlock::get(ei.getNode());
      if (in->getExit()) {
         if (prevData != 0x04)
            prevData = in->getExit()->sched;
         prevOp = in->getExit()->op;
      }
      score->setMax(&scoreBoards.at(in->getId()));
   }
   if (bb->cfg.incidentCount() > 1)
      prevOp = OP_NOP;

#ifdef NVC0_DEBUG_SCHED_DATA
   INFO("=== BB:%i initial scores\n", bb->getId());
   score->print(cycle);
#endif

   for (insn = bb->getEntry(); insn && insn->next; insn = insn->next) {
      next = insn->next;

      commitInsn(insn, cycle);
      int delay = calcDelay(next, cycle);
      setDelay(insn, delay, next);
      cycle += getCycles(insn, delay);

#ifdef NVC0_DEBUG_SCHED_DATA
      INFO("cycle %i, sched %02x\n", cycle, insn->sched);
      insn->print();
      next->print();
#endif
   }
   if (!insn)
      return true;
   commitInsn(insn, cycle);

   int bbDelay = -1;

   for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
      BasicBlock *out = BasicBlock::get(ei.getNode());

      if (ei.getType() != Graph::Edge::BACK) {
         // only test the first instruction of the outgoing block
         next = out->getEntry();
         if (next)
            bbDelay = MAX2(bbDelay, calcDelay(next, cycle));
      } else {
         // wait until all dependencies are satisfied
         const int regsFree = score->getLatest();
         next = out->getFirst();
         for (int c = cycle; next && c < regsFree; next = next->next) {
            bbDelay = MAX2(bbDelay, calcDelay(next, c));
            c += getCycles(next, bbDelay);
         }
         next = NULL;
      }
   }
   if (bb->cfg.outgoingCount() != 1)
      next = NULL;
   setDelay(insn, bbDelay, next);
   cycle += getCycles(insn, bbDelay);

   score->rebase(cycle); // common base for initializing out blocks' scores
   return true;
}

#define NVE4_MAX_ISSUE_DELAY 0x1f
int
SchedDataCalculator::calcDelay(const Instruction *insn, int cycle) const
{
   int delay = 0, ready = cycle;

   for (int s = 0; insn->srcExists(s); ++s)
      checkRd(insn->getSrc(s), cycle, delay);
   // WAR & WAW don't seem to matter
   // for (int s = 0; insn->srcExists(s); ++s)
   //   recordRd(insn->getSrc(s), cycle);

   switch (Target::getOpClass(insn->op)) {
   case OPCLASS_SFU:
      ready = score->res.sfu;
      break;
   case OPCLASS_ARITH:
      if (insn->op == OP_MUL && !isFloatType(insn->dType))
         ready = score->res.imul;
      break;
   case OPCLASS_TEXTURE:
      ready = score->res.tex;
      break;
   case OPCLASS_LOAD:
      ready = score->res.ld[insn->src(0).getFile()];
      break;
   case OPCLASS_STORE:
      ready = score->res.st[insn->src(0).getFile()];
      break;
   default:
      break;
   }
   if (Target::getOpClass(insn->op) != OPCLASS_TEXTURE)
      ready = MAX2(ready, score->res.tex);

   delay = MAX2(delay, ready - cycle);

   // if can issue next cycle, delay is 0, not 1
   return MIN2(delay - 1, NVE4_MAX_ISSUE_DELAY);
}

void
SchedDataCalculator::commitInsn(const Instruction *insn, int cycle)
{
   const int ready = cycle + targ->getLatency(insn);

   for (int d = 0; insn->defExists(d); ++d)
      recordWr(insn->getDef(d), ready);
   // WAR & WAW don't seem to matter
   // for (int s = 0; insn->srcExists(s); ++s)
   //   recordRd(insn->getSrc(s), cycle);

   switch (Target::getOpClass(insn->op)) {
   case OPCLASS_SFU:
      score->res.sfu = cycle + 4;
      break;
   case OPCLASS_ARITH:
      if (insn->op == OP_MUL && !isFloatType(insn->dType))
         score->res.imul = cycle + 4;
      break;
   case OPCLASS_TEXTURE:
      score->res.tex = cycle + 18;
      break;
   case OPCLASS_LOAD:
      if (insn->src(0).getFile() == FILE_MEMORY_CONST)
         break;
      score->res.ld[insn->src(0).getFile()] = cycle + 4;
      score->res.st[insn->src(0).getFile()] = ready;
      break;
   case OPCLASS_STORE:
      score->res.st[insn->src(0).getFile()] = cycle + 4;
      score->res.ld[insn->src(0).getFile()] = ready;
      break;
   case OPCLASS_OTHER:
      if (insn->op == OP_TEXBAR)
         score->res.tex = cycle;
      break;
   default:
      break;
   }

#ifdef NVC0_DEBUG_SCHED_DATA
   score->print(cycle);
#endif
}

void
SchedDataCalculator::checkRd(const Value *v, int cycle, int& delay) const
{
   int ready = cycle;
   int a, b;

   switch (v->reg.file) {
   case FILE_GPR:
      a = v->reg.data.id;
      b = a + v->reg.size / 4;
      for (int r = a; r < b; ++r)
         ready = MAX2(ready, score->rd.r[r]);
      break;
   case FILE_PREDICATE:
      ready = MAX2(ready, score->rd.p[v->reg.data.id]);
      break;
   case FILE_FLAGS:
      ready = MAX2(ready, score->rd.c);
      break;
   case FILE_SHADER_INPUT:
   case FILE_SHADER_OUTPUT: // yes, TCPs can read outputs
   case FILE_MEMORY_LOCAL:
   case FILE_MEMORY_CONST:
   case FILE_MEMORY_SHARED:
   case FILE_MEMORY_GLOBAL:
   case FILE_SYSTEM_VALUE:
      // TODO: any restrictions here ?
      break;
   case FILE_IMMEDIATE:
      break;
   default:
      assert(0);
      break;
   }
   if (cycle < ready)
      delay = MAX2(delay, ready - cycle);
}

void
SchedDataCalculator::checkWr(const Value *v, int cycle, int& delay) const
{
   int ready = cycle;
   int a, b;

   switch (v->reg.file) {
   case FILE_GPR:
      a = v->reg.data.id;
      b = a + v->reg.size / 4;
      for (int r = a; r < b; ++r)
         ready = MAX2(ready, score->wr.r[r]);
      break;
   case FILE_PREDICATE:
      ready = MAX2(ready, score->wr.p[v->reg.data.id]);
      break;
   default:
      assert(v->reg.file == FILE_FLAGS);
      ready = MAX2(ready, score->wr.c);
      break;
   }
   if (cycle < ready)
      delay = MAX2(delay, ready - cycle);
}

void
SchedDataCalculator::recordWr(const Value *v, const int ready)
{
   int a = v->reg.data.id;

   if (v->reg.file == FILE_GPR) {
      int b = a + v->reg.size / 4;
      for (int r = a; r < b; ++r)
         score->rd.r[r] = ready;
   } else
   // $c, $pX: shorter issue-to-read delay (at least as exec pred and carry)
   if (v->reg.file == FILE_PREDICATE) {
      score->rd.p[a] = ready + 4;
   } else {
      assert(v->reg.file == FILE_FLAGS);
      score->rd.c = ready + 4;
   }
}

void
SchedDataCalculator::recordRd(const Value *v, const int ready)
{
   int a = v->reg.data.id;

   if (v->reg.file == FILE_GPR) {
      int b = a + v->reg.size / 4;
      for (int r = a; r < b; ++r)
         score->wr.r[r] = ready;
   } else
   if (v->reg.file == FILE_PREDICATE) {
      score->wr.p[a] = ready;
   } else
   if (v->reg.file == FILE_FLAGS) {
      score->wr.c = ready;
   }
}

bool
calculateSchedDataNVC0(const Target *targ, Function *func)
{
   SchedDataCalculator sched(targ);
   return sched.run(func, true, true);
}

void
CodeEmitterNVC0::prepareEmission(Function *func)
{
   CodeEmitter::prepareEmission(func);

   if (targ->hasSWSched)
      calculateSchedDataNVC0(targ, func);
}

CodeEmitterNVC0::CodeEmitterNVC0(const TargetNVC0 *target, Program::Type type)
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
TargetNVC0::createCodeEmitterNVC0(Program::Type type)
{
   CodeEmitterNVC0 *emit = new CodeEmitterNVC0(this, type);
   return emit;
}

CodeEmitter *
TargetNVC0::getCodeEmitter(Program::Type type)
{
   if (chipset >= NVISA_GK20A_CHIPSET)
      return createCodeEmitterGK110(type);
   return createCodeEmitterNVC0(type);
}

} // namespace nv50_ir
