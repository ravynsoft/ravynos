/*
 * Copyright 2014 Red Hat Inc.
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
 *
 * Authors: Ben Skeggs <bskeggs@redhat.com>
 */

#include "nv50_ir_target_gm107.h"
#include "nv50_ir_sched_gm107.h"

//#define GM107_DEBUG_SCHED_DATA

namespace nv50_ir {

class CodeEmitterGM107 : public CodeEmitter
{
public:
   CodeEmitterGM107(const TargetGM107 *);

   virtual bool emitInstruction(Instruction *);
   virtual uint32_t getMinEncodingSize(const Instruction *) const;

   virtual void prepareEmission(Program *);
   virtual void prepareEmission(Function *);

   inline void setProgramType(Program::Type pType) { progType = pType; }

private:
   const TargetGM107 *targGM107;

   Program::Type progType;

   const Instruction *insn;
   const bool writeIssueDelays;
   uint32_t *data;

private:
   inline void emitField(uint32_t *, int, int, uint32_t);
   inline void emitField(int b, int s, uint32_t v) { emitField(code, b, s, v); }

   inline void emitInsn(uint32_t, bool);
   inline void emitInsn(uint32_t o) { emitInsn(o, true); }
   inline void emitPred();
   inline void emitGPR(int, const Value *);
   inline void emitGPR(int pos) {
      emitGPR(pos, (const Value *)NULL);
   }
   inline void emitGPR(int pos, const ValueRef &ref) {
      emitGPR(pos, ref.get() ? ref.rep() : (const Value *)NULL);
   }
   inline void emitGPR(int pos, const ValueRef *ref) {
      emitGPR(pos, ref ? ref->rep() : (const Value *)NULL);
   }
   inline void emitGPR(int pos, const ValueDef &def) {
      emitGPR(pos, def.get() ? def.rep() : (const Value *)NULL);
   }
   inline void emitSYS(int, const Value *);
   inline void emitSYS(int pos, const ValueRef &ref) {
      emitSYS(pos, ref.get() ? ref.rep() : (const Value *)NULL);
   }
   inline void emitPRED(int, const Value *);
   inline void emitPRED(int pos) {
      emitPRED(pos, (const Value *)NULL);
   }
   inline void emitPRED(int pos, const ValueRef &ref) {
      emitPRED(pos, ref.get() ? ref.rep() : (const Value *)NULL);
   }
   inline void emitPRED(int pos, const ValueDef &def) {
      emitPRED(pos, def.get() ? def.rep() : (const Value *)NULL);
   }
   inline void emitADDR(int, int, int, int, const ValueRef &);
   inline void emitCBUF(int, int, int, int, int, const ValueRef &);
   inline bool longIMMD(const ValueRef &);
   inline void emitIMMD(int, int, const ValueRef &);

   void emitCond3(int, CondCode);
   void emitCond4(int, CondCode);
   void emitCond5(int pos, CondCode cc) { emitCond4(pos, cc); }
   inline void emitO(int);
   inline void emitP(int);
   inline void emitSAT(int);
   inline void emitCC(int);
   inline void emitX(int);
   inline void emitABS(int, const ValueRef &);
   inline void emitNEG(int, const ValueRef &);
   inline void emitNEG2(int, const ValueRef &, const ValueRef &);
   inline void emitFMZ(int, int);
   inline void emitRND(int, RoundMode, int);
   inline void emitRND(int pos) {
      emitRND(pos, insn->rnd, -1);
   }
   inline void emitPDIV(int);
   inline void emitINV(int, const ValueRef &);

   void emitEXIT();
   void emitBRA();
   void emitCAL();
   void emitPCNT();
   void emitCONT();
   void emitPBK();
   void emitBRK();
   void emitPRET();
   void emitRET();
   void emitSSY();
   void emitSYNC();
   void emitSAM();
   void emitRAM();

   void emitPSETP();

   void emitMOV();
   void emitS2R();
   void emitCS2R();
   void emitF2F();
   void emitF2I();
   void emitI2F();
   void emitI2I();
   void emitSEL();
   void emitSHFL();

   void emitDADD();
   void emitDMUL();
   void emitDFMA();
   void emitDMNMX();
   void emitDSET();
   void emitDSETP();

   void emitFADD();
   void emitFMUL();
   void emitFFMA();
   void emitMUFU();
   void emitFMNMX();
   void emitRRO();
   void emitFCMP();
   void emitFSET();
   void emitFSETP();
   void emitFSWZADD();

   void emitLOP();
   void emitNOT();
   void emitIADD();
   void emitIMUL();
   void emitIMAD();
   void emitISCADD();
   void emitXMAD();
   void emitIMNMX();
   void emitICMP();
   void emitISET();
   void emitISETP();
   void emitSHL();
   void emitSHR();
   void emitSHF();
   void emitPOPC();
   void emitBFI();
   void emitBFE();
   void emitFLO();
   void emitPRMT();

   void emitLDSTs(int, DataType);
   void emitLDSTc(int);
   void emitLDC();
   void emitLDL();
   void emitLDS();
   void emitLD();
   void emitSTL();
   void emitSTS();
   void emitST();
   void emitALD();
   void emitAST();
   void emitISBERD();
   void emitAL2P();
   void emitIPA();
   void emitATOM();
   void emitATOMS();
   void emitRED();
   void emitCCTL();

   void emitPIXLD();

   void emitTEXs(int);
   void emitTEX();
   void emitTEXS();
   void emitTLD();
   void emitTLD4();
   void emitTXD();
   void emitTXQ();
   void emitTMML();
   void emitDEPBAR();

   void emitNOP();
   void emitKIL();
   void emitOUT();

   void emitBAR();
   void emitMEMBAR();

   void emitVOTE();

   void emitSUTarget();
   void emitSUHandle(const int s);
   void emitSUSTx();
   void emitSULDx();
   void emitSUREDx();
};

/*******************************************************************************
 * general instruction layout/fields
 ******************************************************************************/

void
CodeEmitterGM107::emitField(uint32_t *data, int b, int s, uint32_t v)
{
   if (b >= 0) {
      uint32_t m = ((1ULL << s) - 1);
      uint64_t d = (uint64_t)(v & m) << b;
      assert(!(v & ~m) || (v & ~m) == ~m);
      data[1] |= d >> 32;
      data[0] |= d;
   }
}

void
CodeEmitterGM107::emitPred()
{
   if (insn->predSrc >= 0) {
      emitField(16, 3, insn->getSrc(insn->predSrc)->rep()->reg.data.id);
      emitField(19, 1, insn->cc == CC_NOT_P);
   } else {
      emitField(16, 3, 7);
   }
}

void
CodeEmitterGM107::emitInsn(uint32_t hi, bool pred)
{
   code[0] = 0x00000000;
   code[1] = hi;
   if (pred)
      emitPred();
}

void
CodeEmitterGM107::emitGPR(int pos, const Value *val)
{
   emitField(pos, 8, val && !val->inFile(FILE_FLAGS) ?
             val->reg.data.id : 255);
}

void
CodeEmitterGM107::emitSYS(int pos, const Value *val)
{
   int id = val ? val->reg.data.id : -1;

   switch (id) {
   case SV_LANEID         : id = 0x00; break;
   case SV_VERTEX_COUNT   : id = 0x10; break;
   case SV_INVOCATION_ID  : id = 0x11; break;
   case SV_THREAD_KILL    : id = 0x13; break;
   case SV_INVOCATION_INFO: id = 0x1d; break;
   case SV_COMBINED_TID   : id = 0x20; break;
   case SV_TID            : id = 0x21 + val->reg.data.sv.index; break;
   case SV_CTAID          : id = 0x25 + val->reg.data.sv.index; break;
   case SV_LANEMASK_EQ    : id = 0x38; break;
   case SV_LANEMASK_LT    : id = 0x39; break;
   case SV_LANEMASK_LE    : id = 0x3a; break;
   case SV_LANEMASK_GT    : id = 0x3b; break;
   case SV_LANEMASK_GE    : id = 0x3c; break;
   case SV_CLOCK          : id = 0x50 + val->reg.data.sv.index; break;
   default:
      assert(!"invalid system value");
      id = 0;
      break;
   }

   emitField(pos, 8, id);
}

void
CodeEmitterGM107::emitPRED(int pos, const Value *val)
{
   emitField(pos, 3, val ? val->reg.data.id : 7);
}

void
CodeEmitterGM107::emitADDR(int gpr, int off, int len, int shr,
                           const ValueRef &ref)
{
   const Value *v = ref.get();
   assert(!(v->reg.data.offset & ((1 << shr) - 1)));
   if (gpr >= 0)
      emitGPR(gpr, ref.getIndirect(0));
   emitField(off, len, v->reg.data.offset >> shr);
}

void
CodeEmitterGM107::emitCBUF(int buf, int gpr, int off, int len, int shr,
                           const ValueRef &ref)
{
   const Value *v = ref.get();
   const Symbol *s = v->asSym();

   assert(!(s->reg.data.offset & ((1 << shr) - 1)));

   emitField(buf,  5, v->reg.fileIndex);
   if (gpr >= 0)
      emitGPR(gpr, ref.getIndirect(0));
   emitField(off, 16, s->reg.data.offset >> shr);
}

bool
CodeEmitterGM107::longIMMD(const ValueRef &ref)
{
   if (ref.getFile() == FILE_IMMEDIATE) {
      const ImmediateValue *imm = ref.get()->asImm();
      if (isFloatType(insn->sType))
         return imm->reg.data.u32 & 0xfff;
      else
         return imm->reg.data.s32 > 0x7ffff || imm->reg.data.s32 < -0x80000;
   }
   return false;
}

void
CodeEmitterGM107::emitIMMD(int pos, int len, const ValueRef &ref)
{
   const ImmediateValue *imm = ref.get()->asImm();
   uint32_t val = imm->reg.data.u32;

   if (len == 19) {
      if (insn->sType == TYPE_F32 || insn->sType == TYPE_F16) {
         assert(!(val & 0x00000fff));
         val >>= 12;
      } else if (insn->sType == TYPE_F64) {
         assert(!(imm->reg.data.u64 & 0x00000fffffffffffULL));
         val = imm->reg.data.u64 >> 44;
      } else {
         assert(!(val & 0xfff80000) || (val & 0xfff80000) == 0xfff80000);
      }
      emitField( 56,   1, (val & 0x80000) >> 19);
      emitField(pos, len, (val & 0x7ffff));
   } else {
      emitField(pos, len, val);
   }
}

/*******************************************************************************
 * modifiers
 ******************************************************************************/

void
CodeEmitterGM107::emitCond3(int pos, CondCode code)
{
   int data = 0;

   switch (code) {
   case CC_FL : data = 0x00; break;
   case CC_LTU:
   case CC_LT : data = 0x01; break;
   case CC_EQU:
   case CC_EQ : data = 0x02; break;
   case CC_LEU:
   case CC_LE : data = 0x03; break;
   case CC_GTU:
   case CC_GT : data = 0x04; break;
   case CC_NEU:
   case CC_NE : data = 0x05; break;
   case CC_GEU:
   case CC_GE : data = 0x06; break;
   case CC_TR : data = 0x07; break;
   default:
      assert(!"invalid cond3");
      break;
   }

   emitField(pos, 3, data);
}

void
CodeEmitterGM107::emitCond4(int pos, CondCode code)
{
   int data = 0;

   switch (code) {
   case CC_FL: data = 0x00; break;
   case CC_LT: data = 0x01; break;
   case CC_EQ: data = 0x02; break;
   case CC_LE: data = 0x03; break;
   case CC_GT: data = 0x04; break;
   case CC_NE: data = 0x05; break;
   case CC_GE: data = 0x06; break;
//   case CC_NUM: data = 0x07; break;
//   case CC_NAN: data = 0x08; break;
   case CC_LTU: data = 0x09; break;
   case CC_EQU: data = 0x0a; break;
   case CC_LEU: data = 0x0b; break;
   case CC_GTU: data = 0x0c; break;
   case CC_NEU: data = 0x0d; break;
   case CC_GEU: data = 0x0e; break;
   case CC_TR:  data = 0x0f; break;
   default:
      assert(!"invalid cond4");
      break;
   }

   emitField(pos, 4, data);
}

void
CodeEmitterGM107::emitO(int pos)
{
   emitField(pos, 1, insn->getSrc(0)->reg.file == FILE_SHADER_OUTPUT);
}

void
CodeEmitterGM107::emitP(int pos)
{
   emitField(pos, 1, insn->perPatch);
}

void
CodeEmitterGM107::emitSAT(int pos)
{
   emitField(pos, 1, insn->saturate);
}

void
CodeEmitterGM107::emitCC(int pos)
{
   emitField(pos, 1, insn->flagsDef >= 0);
}

void
CodeEmitterGM107::emitX(int pos)
{
   emitField(pos, 1, insn->flagsSrc >= 0);
}

void
CodeEmitterGM107::emitABS(int pos, const ValueRef &ref)
{
   emitField(pos, 1, ref.mod.abs());
}

void
CodeEmitterGM107::emitNEG(int pos, const ValueRef &ref)
{
   emitField(pos, 1, ref.mod.neg());
}

void
CodeEmitterGM107::emitNEG2(int pos, const ValueRef &a, const ValueRef &b)
{
   emitField(pos, 1, a.mod.neg() ^ b.mod.neg());
}

void
CodeEmitterGM107::emitFMZ(int pos, int len)
{
   emitField(pos, len, insn->dnz << 1 | insn->ftz);
}

void
CodeEmitterGM107::emitRND(int rmp, RoundMode rnd, int rip)
{
   int rm = 0, ri = 0;
   switch (rnd) {
   case ROUND_NI: ri = 1;
   case ROUND_N : rm = 0; break;
   case ROUND_MI: ri = 1;
   case ROUND_M : rm = 1; break;
   case ROUND_PI: ri = 1;
   case ROUND_P : rm = 2; break;
   case ROUND_ZI: ri = 1;
   case ROUND_Z : rm = 3; break;
   default:
      assert(!"invalid round mode");
      break;
   }
   emitField(rip, 1, ri);
   emitField(rmp, 2, rm);
}

void
CodeEmitterGM107::emitPDIV(int pos)
{
   assert(insn->postFactor >= -3 && insn->postFactor <= 3);
   if (insn->postFactor > 0)
      emitField(pos, 3, 7 - insn->postFactor);
   else
      emitField(pos, 3, 0 - insn->postFactor);
}

void
CodeEmitterGM107::emitINV(int pos, const ValueRef &ref)
{
   emitField(pos, 1, !!(ref.mod & Modifier(NV50_IR_MOD_NOT)));
}

/*******************************************************************************
 * control flow
 ******************************************************************************/

void
CodeEmitterGM107::emitEXIT()
{
   emitInsn (0xe3000000);
   emitCond5(0x00, CC_TR);
}

void
CodeEmitterGM107::emitBRA()
{
   const FlowInstruction *insn = this->insn->asFlow();
   int gpr = -1;

   if (insn->indirect) {
      if (insn->absolute)
         emitInsn(0xe2000000); // JMX
      else
         emitInsn(0xe2500000); // BRX
      gpr = 0x08;
   } else {
      if (insn->absolute)
         emitInsn(0xe2100000); // JMP
      else
         emitInsn(0xe2400000); // BRA
      emitField(0x07, 1, insn->allWarp);
   }

   emitField(0x06, 1, insn->limit);
   emitCond5(0x00, CC_TR);

   if (!insn->srcExists(0) || insn->src(0).getFile() != FILE_MEMORY_CONST) {
      int32_t pos = insn->target.bb->binPos;
      if (writeIssueDelays && !(pos & 0x1f))
         pos += 8;
      if (!insn->absolute)
         emitField(0x14, 24, pos - (codeSize + 8));
      else
         emitField(0x14, 32, pos);
   } else {
      emitCBUF (0x24, gpr, 20, 16, 0, insn->src(0));
      emitField(0x05, 1, 1);
   }
}

void
CodeEmitterGM107::emitCAL()
{
   const FlowInstruction *insn = this->insn->asFlow();

   if (insn->absolute) {
      emitInsn(0xe2200000, false); // JCAL
   } else {
      emitInsn(0xe2600000, false); // CAL
   }

   if (!insn->srcExists(0) || insn->src(0).getFile() != FILE_MEMORY_CONST) {
      if (!insn->absolute)
         emitField(0x14, 24, insn->target.bb->binPos - (codeSize + 8));
      else {
         if (insn->builtin) {
            int pcAbs = targGM107->getBuiltinOffset(insn->target.builtin);
            addReloc(RelocEntry::TYPE_BUILTIN, 0, pcAbs, 0xfff00000,  20);
            addReloc(RelocEntry::TYPE_BUILTIN, 1, pcAbs, 0x000fffff, -12);
         } else {
            emitField(0x14, 32, insn->target.bb->binPos);
         }
      }
   } else {
      emitCBUF (0x24, -1, 20, 16, 0, insn->src(0));
      emitField(0x05, 1, 1);
   }
}

void
CodeEmitterGM107::emitPCNT()
{
   const FlowInstruction *insn = this->insn->asFlow();

   emitInsn(0xe2b00000, false);

   if (!insn->srcExists(0) || insn->src(0).getFile() != FILE_MEMORY_CONST) {
      emitField(0x14, 24, insn->target.bb->binPos - (codeSize + 8));
   } else {
      emitCBUF (0x24, -1, 20, 16, 0, insn->src(0));
      emitField(0x05, 1, 1);
   }
}

void
CodeEmitterGM107::emitCONT()
{
   emitInsn (0xe3500000);
   emitCond5(0x00, CC_TR);
}

void
CodeEmitterGM107::emitPBK()
{
   const FlowInstruction *insn = this->insn->asFlow();

   emitInsn(0xe2a00000, false);

   if (!insn->srcExists(0) || insn->src(0).getFile() != FILE_MEMORY_CONST) {
      emitField(0x14, 24, insn->target.bb->binPos - (codeSize + 8));
   } else {
      emitCBUF (0x24, -1, 20, 16, 0, insn->src(0));
      emitField(0x05, 1, 1);
   }
}

void
CodeEmitterGM107::emitBRK()
{
   emitInsn (0xe3400000);
   emitCond5(0x00, CC_TR);
}

void
CodeEmitterGM107::emitPRET()
{
   const FlowInstruction *insn = this->insn->asFlow();

   emitInsn(0xe2700000, false);

   if (!insn->srcExists(0) || insn->src(0).getFile() != FILE_MEMORY_CONST) {
      emitField(0x14, 24, insn->target.bb->binPos - (codeSize + 8));
   } else {
      emitCBUF (0x24, -1, 20, 16, 0, insn->src(0));
      emitField(0x05, 1, 1);
   }
}

void
CodeEmitterGM107::emitRET()
{
   emitInsn (0xe3200000);
   emitCond5(0x00, CC_TR);
}

void
CodeEmitterGM107::emitSSY()
{
   const FlowInstruction *insn = this->insn->asFlow();

   emitInsn(0xe2900000, false);

   if (!insn->srcExists(0) || insn->src(0).getFile() != FILE_MEMORY_CONST) {
      emitField(0x14, 24, insn->target.bb->binPos - (codeSize + 8));
   } else {
      emitCBUF (0x24, -1, 20, 16, 0, insn->src(0));
      emitField(0x05, 1, 1);
   }
}

void
CodeEmitterGM107::emitSYNC()
{
   emitInsn (0xf0f80000);
   emitCond5(0x00, CC_TR);
}

void
CodeEmitterGM107::emitSAM()
{
   emitInsn(0xe3700000, false);
}

void
CodeEmitterGM107::emitRAM()
{
   emitInsn(0xe3800000, false);
}

/*******************************************************************************
 * predicate/cc
 ******************************************************************************/

void
CodeEmitterGM107::emitPSETP()
{

   emitInsn(0x50900000);

   switch (insn->op) {
   case OP_AND: emitField(0x18, 3, 0); break;
   case OP_OR:  emitField(0x18, 3, 1); break;
   case OP_XOR: emitField(0x18, 3, 2); break;
   default:
      assert(!"unexpected operation");
      break;
   }

   // emitINV (0x2a);
   emitPRED(0x27); // TODO: support 3-arg
   emitINV (0x20, insn->src(1));
   emitPRED(0x1d, insn->src(1));
   emitINV (0x0f, insn->src(0));
   emitPRED(0x0c, insn->src(0));
   emitPRED(0x03, insn->def(0));
   emitPRED(0x00);
}

/*******************************************************************************
 * movement / conversion
 ******************************************************************************/

void
CodeEmitterGM107::emitMOV()
{
   if (insn->src(0).getFile() != FILE_IMMEDIATE) {
      switch (insn->src(0).getFile()) {
      case FILE_GPR:
         if (insn->def(0).getFile() == FILE_PREDICATE) {
            emitInsn(0x5b6a0000);
            emitGPR (0x08);
         } else {
            emitInsn(0x5c980000);
         }
         emitGPR (0x14, insn->src(0));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4c980000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x38980000);
         emitIMMD(0x14, 19, insn->src(0));
         break;
      case FILE_PREDICATE:
         emitInsn(0x50880000);
         emitPRED(0x0c, insn->src(0));
         emitPRED(0x1d);
         emitPRED(0x27);
         break;
      default:
         assert(!"bad src file");
         break;
      }
      if (insn->def(0).getFile() != FILE_PREDICATE &&
          insn->src(0).getFile() != FILE_PREDICATE)
         emitField(0x27, 4, insn->lanes);
   } else {
      emitInsn (0x01000000);
      emitIMMD (0x14, 32, insn->src(0));
      emitField(0x0c, 4, insn->lanes);
   }

   if (insn->def(0).getFile() == FILE_PREDICATE) {
      emitPRED(0x27);
      emitPRED(0x03, insn->def(0));
      emitPRED(0x00);
   } else {
      emitGPR(0x00, insn->def(0));
   }
}

void
CodeEmitterGM107::emitS2R()
{
   emitInsn(0xf0c80000);
   emitSYS (0x14, insn->src(0));
   emitGPR (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitCS2R()
{
   emitInsn(0x50c80000);
   emitSYS (0x14, insn->src(0));
   emitGPR (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitF2F()
{
   RoundMode rnd = insn->rnd;

   switch (insn->op) {
   case OP_FLOOR: rnd = ROUND_MI; break;
   case OP_CEIL : rnd = ROUND_PI; break;
   case OP_TRUNC: rnd = ROUND_ZI; break;
   default:
      break;
   }

   switch (insn->src(0).getFile()) {
   case FILE_GPR:
      emitInsn(0x5ca80000);
      emitGPR (0x14, insn->src(0));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4ca80000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38a80000);
      emitIMMD(0x14, 19, insn->src(0));
      break;
   default:
      assert(!"bad src0 file");
      break;
   }

   emitField(0x32, 1, (insn->op == OP_SAT) || insn->saturate);
   emitField(0x31, 1, (insn->op == OP_ABS) || insn->src(0).mod.abs());
   emitCC   (0x2f);
   emitField(0x2d, 1, (insn->op == OP_NEG) || insn->src(0).mod.neg());
   emitFMZ  (0x2c, 1);
   emitField(0x29, 1, insn->subOp);
   emitRND  (0x27, rnd, 0x2a);
   emitField(0x0a, 2, util_logbase2(typeSizeof(insn->sType)));
   emitField(0x08, 2, util_logbase2(typeSizeof(insn->dType)));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitF2I()
{
   RoundMode rnd = insn->rnd;

   switch (insn->op) {
   case OP_FLOOR: rnd = ROUND_M; break;
   case OP_CEIL : rnd = ROUND_P; break;
   case OP_TRUNC: rnd = ROUND_Z; break;
   default:
      break;
   }

   switch (insn->src(0).getFile()) {
   case FILE_GPR:
      emitInsn(0x5cb00000);
      emitGPR (0x14, insn->src(0));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4cb00000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38b00000);
      emitIMMD(0x14, 19, insn->src(0));
      break;
   default:
      assert(!"bad src0 file");
      break;
   }

   emitField(0x31, 1, (insn->op == OP_ABS) || insn->src(0).mod.abs());
   emitCC   (0x2f);
   emitField(0x2d, 1, (insn->op == OP_NEG) || insn->src(0).mod.neg());
   emitFMZ  (0x2c, 1);
   emitRND  (0x27, rnd, 0x2a);
   emitField(0x0c, 1, isSignedType(insn->dType));
   emitField(0x0a, 2, util_logbase2(typeSizeof(insn->sType)));
   emitField(0x08, 2, util_logbase2(typeSizeof(insn->dType)));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitI2F()
{
   RoundMode rnd = insn->rnd;

   switch (insn->op) {
   case OP_FLOOR: rnd = ROUND_M; break;
   case OP_CEIL : rnd = ROUND_P; break;
   case OP_TRUNC: rnd = ROUND_Z; break;
   default:
      break;
   }

   switch (insn->src(0).getFile()) {
   case FILE_GPR:
      emitInsn(0x5cb80000);
      emitGPR (0x14, insn->src(0));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4cb80000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38b80000);
      emitIMMD(0x14, 19, insn->src(0));
      break;
   default:
      assert(!"bad src0 file");
      break;
   }

   emitField(0x31, 1, (insn->op == OP_ABS) || insn->src(0).mod.abs());
   emitCC   (0x2f);
   emitField(0x2d, 1, (insn->op == OP_NEG) || insn->src(0).mod.neg());
   emitField(0x29, 2, insn->subOp);
   emitRND  (0x27, rnd, -1);
   emitField(0x0d, 1, isSignedType(insn->sType));
   emitField(0x0a, 2, util_logbase2(typeSizeof(insn->sType)));
   emitField(0x08, 2, util_logbase2(typeSizeof(insn->dType)));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitI2I()
{
   switch (insn->src(0).getFile()) {
   case FILE_GPR:
      emitInsn(0x5ce00000);
      emitGPR (0x14, insn->src(0));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4ce00000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38e00000);
      emitIMMD(0x14, 19, insn->src(0));
      break;
   default:
      assert(!"bad src0 file");
      break;
   }

   emitSAT  (0x32);
   emitField(0x31, 1, (insn->op == OP_ABS) || insn->src(0).mod.abs());
   emitCC   (0x2f);
   emitField(0x2d, 1, (insn->op == OP_NEG) || insn->src(0).mod.neg());
   emitField(0x29, 2, insn->subOp);
   emitField(0x0d, 1, isSignedType(insn->sType));
   emitField(0x0c, 1, isSignedType(insn->dType));
   emitField(0x0a, 2, util_logbase2(typeSizeof(insn->sType)));
   emitField(0x08, 2, util_logbase2(typeSizeof(insn->dType)));
   emitGPR  (0x00, insn->def(0));
}

void
gm107_selpFlip(const FixupEntry *entry, uint32_t *code, const FixupData& data)
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
      code[loc + 1] |= 1 << 10;
   else
      code[loc + 1] &= ~(1 << 10);
}

void
CodeEmitterGM107::emitSEL()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5ca00000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4ca00000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38a00000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitINV (0x2a, insn->src(2));
   emitPRED(0x27, insn->src(2));
   emitGPR (0x08, insn->src(0));
   emitGPR (0x00, insn->def(0));

   if (insn->subOp >= 1) {
      addInterp(insn->subOp - 1, 0, gm107_selpFlip);
   }
}

void
CodeEmitterGM107::emitSHFL()
{
   int type = 0;

   emitInsn (0xef100000);

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitGPR(0x14, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitIMMD(0x14, 5, insn->src(1));
      type |= 1;
      break;
   default:
      assert(!"invalid src1 file");
      break;
   }

   switch (insn->src(2).getFile()) {
   case FILE_GPR:
      emitGPR(0x27, insn->src(2));
      break;
   case FILE_IMMEDIATE:
      emitIMMD(0x22, 13, insn->src(2));
      type |= 2;
      break;
   default:
      assert(!"invalid src2 file");
      break;
   }

   if (!insn->defExists(1))
      emitPRED(0x30);
   else {
      assert(insn->def(1).getFile() == FILE_PREDICATE);
      emitPRED(0x30, insn->def(1));
   }

   emitField(0x1e, 2, insn->subOp);
   emitField(0x1c, 2, type);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

/*******************************************************************************
 * double
 ******************************************************************************/

void
CodeEmitterGM107::emitDADD()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c700000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c700000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38700000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }
   emitABS(0x31, insn->src(1));
   emitNEG(0x30, insn->src(0));
   emitCC (0x2f);
   emitABS(0x2e, insn->src(0));
   emitNEG(0x2d, insn->src(1));

   if (insn->op == OP_SUB)
      code[1] ^= 0x00002000;

   emitGPR(0x08, insn->src(0));
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitDMUL()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c800000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c800000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38800000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitNEG2(0x30, insn->src(0), insn->src(1));
   emitCC  (0x2f);
   emitRND (0x27);
   emitGPR (0x08, insn->src(0));
   emitGPR (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitDFMA()
{
   switch(insn->src(2).getFile()) {
   case FILE_GPR:
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5b700000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4b700000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x36700000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitGPR (0x27, insn->src(2));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x53700000);
      emitGPR (0x27, insn->src(1));
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(2));
      break;
   default:
      assert(!"bad src2 file");
      break;
   }

   emitRND (0x32);
   emitNEG (0x31, insn->src(2));
   emitNEG2(0x30, insn->src(0), insn->src(1));
   emitCC  (0x2f);
   emitGPR (0x08, insn->src(0));
   emitGPR (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitDMNMX()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c500000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c500000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38500000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitABS  (0x31, insn->src(1));
   emitNEG  (0x30, insn->src(0));
   emitCC   (0x2f);
   emitABS  (0x2e, insn->src(0));
   emitNEG  (0x2d, insn->src(1));
   emitField(0x2a, 1, insn->op == OP_MAX);
   emitPRED (0x27);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitDSET()
{
   const CmpInstruction *insn = this->insn->asCmp();

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x59000000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x49000000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x32000000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   if (insn->op != OP_SET) {
      switch (insn->op) {
      case OP_SET_AND: emitField(0x2d, 2, 0); break;
      case OP_SET_OR : emitField(0x2d, 2, 1); break;
      case OP_SET_XOR: emitField(0x2d, 2, 2); break;
      default:
         assert(!"invalid set op");
         break;
      }
      emitPRED(0x27, insn->src(2));
   } else {
      emitPRED(0x27);
   }

   emitABS  (0x36, insn->src(0));
   emitNEG  (0x35, insn->src(1));
   emitField(0x34, 1, insn->dType == TYPE_F32);
   emitCond4(0x30, insn->setCond);
   emitCC   (0x2f);
   emitABS  (0x2c, insn->src(1));
   emitNEG  (0x2b, insn->src(0));
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitDSETP()
{
   const CmpInstruction *insn = this->insn->asCmp();

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5b800000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4b800000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x36800000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   if (insn->op != OP_SET) {
      switch (insn->op) {
      case OP_SET_AND: emitField(0x2d, 2, 0); break;
      case OP_SET_OR : emitField(0x2d, 2, 1); break;
      case OP_SET_XOR: emitField(0x2d, 2, 2); break;
      default:
         assert(!"invalid set op");
         break;
      }
      emitPRED(0x27, insn->src(2));
   } else {
      emitPRED(0x27);
   }

   emitCond4(0x30, insn->setCond);
   emitABS  (0x2c, insn->src(1));
   emitNEG  (0x2b, insn->src(0));
   emitGPR  (0x08, insn->src(0));
   emitABS  (0x07, insn->src(0));
   emitNEG  (0x06, insn->src(1));
   emitPRED (0x03, insn->def(0));
   if (insn->defExists(1))
      emitPRED(0x00, insn->def(1));
   else
      emitPRED(0x00);
}

/*******************************************************************************
 * float
 ******************************************************************************/

void
CodeEmitterGM107::emitFADD()
{
   if (!longIMMD(insn->src(1))) {
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5c580000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4c580000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x38580000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitSAT(0x32);
      emitABS(0x31, insn->src(1));
      emitNEG(0x30, insn->src(0));
      emitCC (0x2f);
      emitABS(0x2e, insn->src(0));
      emitNEG(0x2d, insn->src(1));
      emitFMZ(0x2c, 1);

      if (insn->op == OP_SUB)
         code[1] ^= 0x00002000;
   } else {
      emitInsn(0x08000000);
      emitABS(0x39, insn->src(1));
      emitNEG(0x38, insn->src(0));
      emitFMZ(0x37, 1);
      emitABS(0x36, insn->src(0));
      emitNEG(0x35, insn->src(1));
      emitCC  (0x34);
      emitIMMD(0x14, 32, insn->src(1));

      if (insn->op == OP_SUB)
         code[1] ^= 0x00080000;
   }

   emitGPR(0x08, insn->src(0));
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitFMUL()
{
   if (!longIMMD(insn->src(1))) {
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5c680000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4c680000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x38680000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitSAT (0x32);
      emitNEG2(0x30, insn->src(0), insn->src(1));
      emitCC  (0x2f);
      emitFMZ (0x2c, 2);
      emitPDIV(0x29);
      emitRND (0x27);
   } else {
      emitInsn(0x1e000000);
      emitSAT (0x37);
      emitFMZ (0x35, 2);
      emitCC  (0x34);
      emitIMMD(0x14, 32, insn->src(1));
      if (insn->src(0).mod.neg() ^ insn->src(1).mod.neg())
         code[1] ^= 0x00080000; /* flip immd sign bit */
   }

   emitGPR(0x08, insn->src(0));
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitFFMA()
{
   bool isLongIMMD = false;
   switch(insn->src(2).getFile()) {
   case FILE_GPR:
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x59800000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x49800000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         if (longIMMD(insn->getSrc(1))) {
            assert(insn->getDef(0)->reg.data.id == insn->getSrc(2)->reg.data.id);
            isLongIMMD = true;
            emitInsn(0x0c000000);
            emitIMMD(0x14, 32, insn->src(1));
         } else {
            emitInsn(0x32800000);
            emitIMMD(0x14, 19, insn->src(1));
         }
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      if (!isLongIMMD)
         emitGPR (0x27, insn->src(2));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x51800000);
      emitGPR (0x27, insn->src(1));
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(2));
      break;
   default:
      assert(!"bad src2 file");
      break;
   }

   if (isLongIMMD) {
      emitNEG (0x39, insn->src(2));
      emitNEG2(0x38, insn->src(0), insn->src(1));
      emitSAT (0x37);
      emitCC  (0x34);
   } else {
      emitRND (0x33);
      emitSAT (0x32);
      emitNEG (0x31, insn->src(2));
      emitNEG2(0x30, insn->src(0), insn->src(1));
      emitCC  (0x2f);
   }

   emitFMZ(0x35, 2);
   emitGPR(0x08, insn->src(0));
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitMUFU()
{
   int mufu = 0;

   switch (insn->op) {
   case OP_COS: mufu = 0; break;
   case OP_SIN: mufu = 1; break;
   case OP_EX2: mufu = 2; break;
   case OP_LG2: mufu = 3; break;
   case OP_RCP: mufu = 4 + 2 * insn->subOp; break;
   case OP_RSQ: mufu = 5 + 2 * insn->subOp; break;
   case OP_SQRT: mufu = 8; break;
   default:
      assert(!"invalid mufu");
      break;
   }

   emitInsn (0x50800000);
   emitSAT  (0x32);
   emitNEG  (0x30, insn->src(0));
   emitABS  (0x2e, insn->src(0));
   emitField(0x14, 4, mufu);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitFMNMX()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c600000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c600000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38600000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitField(0x2a, 1, insn->op == OP_MAX);
   emitPRED (0x27);

   emitABS(0x31, insn->src(1));
   emitNEG(0x30, insn->src(0));
   emitCC (0x2f);
   emitABS(0x2e, insn->src(0));
   emitNEG(0x2d, insn->src(1));
   emitFMZ(0x2c, 1);
   emitGPR(0x08, insn->src(0));
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitRRO()
{
   switch (insn->src(0).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c900000);
      emitGPR (0x14, insn->src(0));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c900000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38900000);
      emitIMMD(0x14, 19, insn->src(0));
      break;
   default:
      assert(!"bad src file");
      break;
   }

   emitABS  (0x31, insn->src(0));
   emitNEG  (0x2d, insn->src(0));
   emitField(0x27, 1, insn->op == OP_PREEX2);
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitFCMP()
{
   const CmpInstruction *insn = this->insn->asCmp();
   CondCode cc = insn->setCond;

   if (insn->src(2).mod.neg())
      cc = reverseCondCode(cc);

   switch(insn->src(2).getFile()) {
   case FILE_GPR:
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5ba00000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4ba00000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x36a00000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitGPR (0x27, insn->src(2));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x53a00000);
      emitGPR (0x27, insn->src(1));
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(2));
      break;
   default:
      assert(!"bad src2 file");
      break;
   }

   emitCond4(0x30, cc);
   emitFMZ  (0x2f, 1);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitFSET()
{
   const CmpInstruction *insn = this->insn->asCmp();

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x58000000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x48000000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x30000000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   if (insn->op != OP_SET) {
      switch (insn->op) {
      case OP_SET_AND: emitField(0x2d, 2, 0); break;
      case OP_SET_OR : emitField(0x2d, 2, 1); break;
      case OP_SET_XOR: emitField(0x2d, 2, 2); break;
      default:
         assert(!"invalid set op");
         break;
      }
      emitPRED(0x27, insn->src(2));
   } else {
      emitPRED(0x27);
   }

   emitFMZ  (0x37, 1);
   emitABS  (0x36, insn->src(0));
   emitNEG  (0x35, insn->src(1));
   emitField(0x34, 1, insn->dType == TYPE_F32);
   emitCond4(0x30, insn->setCond);
   emitCC   (0x2f);
   emitABS  (0x2c, insn->src(1));
   emitNEG  (0x2b, insn->src(0));
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitFSETP()
{
   const CmpInstruction *insn = this->insn->asCmp();

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5bb00000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4bb00000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x36b00000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   if (insn->op != OP_SET) {
      switch (insn->op) {
      case OP_SET_AND: emitField(0x2d, 2, 0); break;
      case OP_SET_OR : emitField(0x2d, 2, 1); break;
      case OP_SET_XOR: emitField(0x2d, 2, 2); break;
      default:
         assert(!"invalid set op");
         break;
      }
      emitPRED(0x27, insn->src(2));
   } else {
      emitPRED(0x27);
   }

   emitCond4(0x30, insn->setCond);
   emitFMZ  (0x2f, 1);
   emitABS  (0x2c, insn->src(1));
   emitNEG  (0x2b, insn->src(0));
   emitGPR  (0x08, insn->src(0));
   emitABS  (0x07, insn->src(0));
   emitNEG  (0x06, insn->src(1));
   emitPRED (0x03, insn->def(0));
   if (insn->defExists(1))
      emitPRED(0x00, insn->def(1));
   else
      emitPRED(0x00);
}

void
CodeEmitterGM107::emitFSWZADD()
{
   emitInsn (0x50f80000);
   emitCC   (0x2f);
   emitFMZ  (0x2c, 1);
   emitRND  (0x27);
   emitField(0x26, 1, insn->lanes); /* abused for .ndv */
   emitField(0x1c, 8, insn->subOp);
   if (insn->predSrc != 1)
      emitGPR  (0x14, insn->src(1));
   else
      emitGPR  (0x14);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

/*******************************************************************************
 * integer
 ******************************************************************************/

void
CodeEmitterGM107::emitLOP()
{
   int lop = 0;

   switch (insn->op) {
   case OP_AND: lop = 0; break;
   case OP_OR : lop = 1; break;
   case OP_XOR: lop = 2; break;
   default:
      assert(!"invalid lop");
      break;
   }

   if (!longIMMD(insn->src(1))) {
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5c400000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4c400000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x38400000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitPRED (0x30);
      emitCC   (0x2f);
      emitX    (0x2b);
      emitField(0x29, 2, lop);
      emitINV  (0x28, insn->src(1));
      emitINV  (0x27, insn->src(0));
   } else {
      emitInsn (0x04000000);
      emitX    (0x39);
      emitINV  (0x38, insn->src(1));
      emitINV  (0x37, insn->src(0));
      emitField(0x35, 2, lop);
      emitCC   (0x34);
      emitIMMD (0x14, 32, insn->src(1));
   }

   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

/* special-case of emitLOP(): lop pass_b dst 0 ~src */
void
CodeEmitterGM107::emitNOT()
{
   if (!longIMMD(insn->src(0))) {
      switch (insn->src(0).getFile()) {
      case FILE_GPR:
         emitInsn(0x5c400700);
         emitGPR (0x14, insn->src(0));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4c400700);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x38400700);
         emitIMMD(0x14, 19, insn->src(0));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitPRED (0x30);
   } else {
      emitInsn (0x05600000);
      emitIMMD (0x14, 32, insn->src(1));
   }

   emitGPR(0x08);
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitIADD()
{
   if (!longIMMD(insn->src(1))) {
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5c100000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4c100000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x38100000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitSAT(0x32);
      emitNEG(0x31, insn->src(0));
      emitNEG(0x30, insn->src(1));
      emitCC (0x2f);
      emitX  (0x2b);
   } else {
      emitInsn(0x1c000000);
      emitNEG (0x38, insn->src(0));
      emitSAT (0x36);
      emitX   (0x35);
      emitCC  (0x34);
      emitIMMD(0x14, 32, insn->src(1));
   }

   if (insn->op == OP_SUB)
      code[1] ^= 0x00010000;

   emitGPR(0x08, insn->src(0));
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitIMUL()
{
   if (!longIMMD(insn->src(1))) {
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5c380000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4c380000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x38380000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitCC   (0x2f);
      emitField(0x29, 1, isSignedType(insn->sType));
      emitField(0x28, 1, isSignedType(insn->dType));
      emitField(0x27, 1, insn->subOp == NV50_IR_SUBOP_MUL_HIGH);
   } else {
      emitInsn (0x1f000000);
      emitField(0x37, 1, isSignedType(insn->sType));
      emitField(0x36, 1, isSignedType(insn->dType));
      emitField(0x35, 1, insn->subOp == NV50_IR_SUBOP_MUL_HIGH);
      emitCC   (0x34);
      emitIMMD (0x14, 32, insn->src(1));
   }

   emitGPR(0x08, insn->src(0));
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitIMAD()
{
   /*XXX: imad32i exists, but not using it as third src overlaps dst */
   switch(insn->src(2).getFile()) {
   case FILE_GPR:
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5a000000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4a000000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x34000000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitGPR (0x27, insn->src(2));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x52000000);
      emitGPR (0x27, insn->src(1));
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(2));
      break;
   default:
      assert(!"bad src2 file");
      break;
   }

   emitField(0x36, 1, insn->subOp == NV50_IR_SUBOP_MUL_HIGH);
   emitField(0x35, 1, isSignedType(insn->sType));
   emitNEG  (0x34, insn->src(2));
   emitNEG2 (0x33, insn->src(0), insn->src(1));
   emitSAT  (0x32);
   emitX    (0x31);
   emitField(0x30, 1, isSignedType(insn->dType));
   emitCC   (0x2f);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitISCADD()
{
   assert(insn->src(1).get()->asImm());

   switch (insn->src(2).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c180000);
      emitGPR (0x14, insn->src(2));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c180000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(2));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38180000);
      emitIMMD(0x14, 19, insn->src(2));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }
   emitNEG (0x31, insn->src(0));
   emitNEG (0x30, insn->src(2));
   emitCC  (0x2f);
   emitIMMD(0x27, 5, insn->src(1));
   emitGPR (0x08, insn->src(0));
   emitGPR (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitXMAD()
{
   assert(insn->src(0).getFile() == FILE_GPR);

   bool constbuf = false;
   bool psl_mrg = true;
   bool immediate = false;
   if (insn->src(2).getFile() == FILE_MEMORY_CONST) {
      assert(insn->src(1).getFile() == FILE_GPR);
      constbuf = true;
      psl_mrg = false;
      emitInsn(0x51000000);
      emitGPR(0x27, insn->src(1));
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(2));
   } else if (insn->src(1).getFile() == FILE_MEMORY_CONST) {
      assert(insn->src(2).getFile() == FILE_GPR);
      constbuf = true;
      emitInsn(0x4e000000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      emitGPR(0x27, insn->src(2));
   } else if (insn->src(1).getFile() == FILE_IMMEDIATE) {
      assert(insn->src(2).getFile() == FILE_GPR);
      assert(!(insn->subOp & NV50_IR_SUBOP_XMAD_H1(1)));
      immediate = true;
      emitInsn(0x36000000);
      emitIMMD(0x14, 16, insn->src(1));
      emitGPR(0x27, insn->src(2));
   } else {
      assert(insn->src(1).getFile() == FILE_GPR);
      assert(insn->src(2).getFile() == FILE_GPR);
      emitInsn(0x5b000000);
      emitGPR(0x14, insn->src(1));
      emitGPR(0x27, insn->src(2));
   }

   if (psl_mrg)
      emitField(constbuf ? 0x37 : 0x24, 2, insn->subOp & 0x3);

   unsigned cmode = (insn->subOp & NV50_IR_SUBOP_XMAD_CMODE_MASK);
   cmode >>= NV50_IR_SUBOP_XMAD_CMODE_SHIFT;
   emitField(0x32, constbuf ? 2 : 3, cmode);

   emitX(constbuf ? 0x36 : 0x26);
   emitCC(0x2f);

   emitGPR(0x0, insn->def(0));
   emitGPR(0x8, insn->src(0));

   // source flags
   if (isSignedType(insn->sType)) {
      uint16_t h1s = insn->subOp & NV50_IR_SUBOP_XMAD_H1_MASK;
      emitField(0x30, 2, h1s >> NV50_IR_SUBOP_XMAD_H1_SHIFT);
   }
   emitField(0x35, 1, insn->subOp & NV50_IR_SUBOP_XMAD_H1(0) ? 1 : 0);
   if (!immediate) {
      bool h1 = insn->subOp & NV50_IR_SUBOP_XMAD_H1(1);
      emitField(constbuf ? 0x34 : 0x23, 1, h1);
   }
}

void
CodeEmitterGM107::emitIMNMX()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c200000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c200000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38200000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitField(0x30, 1, isSignedType(insn->dType));
   emitCC   (0x2f);
   emitField(0x2b, 2, insn->subOp);
   emitField(0x2a, 1, insn->op == OP_MAX);
   emitPRED (0x27);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitICMP()
{
   const CmpInstruction *insn = this->insn->asCmp();
   CondCode cc = insn->setCond;

   if (insn->src(2).mod.neg())
      cc = reverseCondCode(cc);

   switch(insn->src(2).getFile()) {
   case FILE_GPR:
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5b400000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4b400000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x36400000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitGPR (0x27, insn->src(2));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x53400000);
      emitGPR (0x27, insn->src(1));
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(2));
      break;
   default:
      assert(!"bad src2 file");
      break;
   }

   emitCond3(0x31, cc);
   emitField(0x30, 1, isSignedType(insn->sType));
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitISET()
{
   const CmpInstruction *insn = this->insn->asCmp();

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5b500000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4b500000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x36500000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   if (insn->op != OP_SET) {
      switch (insn->op) {
      case OP_SET_AND: emitField(0x2d, 2, 0); break;
      case OP_SET_OR : emitField(0x2d, 2, 1); break;
      case OP_SET_XOR: emitField(0x2d, 2, 2); break;
      default:
         assert(!"invalid set op");
         break;
      }
      emitPRED(0x27, insn->src(2));
   } else {
      emitPRED(0x27);
   }

   emitCond3(0x31, insn->setCond);
   emitField(0x30, 1, isSignedType(insn->sType));
   emitCC   (0x2f);
   emitField(0x2c, 1, insn->dType == TYPE_F32);
   emitX    (0x2b);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitISETP()
{
   const CmpInstruction *insn = this->insn->asCmp();

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5b600000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4b600000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x36600000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   if (insn->op != OP_SET) {
      switch (insn->op) {
      case OP_SET_AND: emitField(0x2d, 2, 0); break;
      case OP_SET_OR : emitField(0x2d, 2, 1); break;
      case OP_SET_XOR: emitField(0x2d, 2, 2); break;
      default:
         assert(!"invalid set op");
         break;
      }
      emitPRED(0x27, insn->src(2));
   } else {
      emitPRED(0x27);
   }

   emitCond3(0x31, insn->setCond);
   emitField(0x30, 1, isSignedType(insn->sType));
   emitX    (0x2b);
   emitGPR  (0x08, insn->src(0));
   emitPRED (0x03, insn->def(0));
   if (insn->defExists(1))
      emitPRED(0x00, insn->def(1));
   else
      emitPRED(0x00);
}

void
CodeEmitterGM107::emitSHL()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c480000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c480000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38480000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitCC   (0x2f);
   emitX    (0x2b);
   emitField(0x27, 1, insn->subOp == NV50_IR_SUBOP_SHIFT_WRAP);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitSHR()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c280000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c280000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38280000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitField(0x30, 1, isSignedType(insn->dType));
   emitCC   (0x2f);
   emitX    (0x2c);
   emitField(0x27, 1, insn->subOp == NV50_IR_SUBOP_SHIFT_WRAP);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitSHF()
{
   unsigned type;

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(insn->op == OP_SHL ? 0x5bf80000 : 0x5cf80000);
      emitGPR(0x14, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(insn->op == OP_SHL ? 0x36f80000 : 0x38f80000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   switch (insn->sType) {
   case TYPE_U64:
      type = 2;
      break;
   case TYPE_S64:
      type = 3;
      break;
   default:
      type = 0;
      break;
   }

   emitField(0x32, 1, !!(insn->subOp & NV50_IR_SUBOP_SHIFT_WRAP));
   emitX    (0x31);
   emitField(0x30, 1, !!(insn->subOp & NV50_IR_SUBOP_SHIFT_HIGH));
   emitCC   (0x2f);
   emitGPR  (0x27, insn->src(2));
   emitField(0x25, 2, type);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitPOPC()
{
   switch (insn->src(0).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c080000);
      emitGPR (0x14, insn->src(0));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c080000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38080000);
      emitIMMD(0x14, 19, insn->src(0));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitINV(0x28, insn->src(0));
   emitGPR(0x00, insn->def(0));
}

void
CodeEmitterGM107::emitBFI()
{
   switch(insn->src(2).getFile()) {
   case FILE_GPR:
      switch (insn->src(1).getFile()) {
      case FILE_GPR:
         emitInsn(0x5bf00000);
         emitGPR (0x14, insn->src(1));
         break;
      case FILE_MEMORY_CONST:
         emitInsn(0x4bf00000);
         emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
         break;
      case FILE_IMMEDIATE:
         emitInsn(0x36f00000);
         emitIMMD(0x14, 19, insn->src(1));
         break;
      default:
         assert(!"bad src1 file");
         break;
      }
      emitGPR (0x27, insn->src(2));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x53f00000);
      emitGPR (0x27, insn->src(1));
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(2));
      break;
   default:
      assert(!"bad src2 file");
      break;
   }

   emitCC   (0x2f);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitBFE()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c000000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c000000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38000000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitField(0x30, 1, isSignedType(insn->dType));
   emitCC   (0x2f);
   emitField(0x28, 1, insn->subOp == NV50_IR_SUBOP_EXTBF_REV);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitFLO()
{
   switch (insn->src(0).getFile()) {
   case FILE_GPR:
      emitInsn(0x5c300000);
      emitGPR (0x14, insn->src(0));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4c300000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(0));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x38300000);
      emitIMMD(0x14, 19, insn->src(0));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitField(0x30, 1, isSignedType(insn->dType));
   emitCC   (0x2f);
   emitField(0x29, 1, insn->subOp == NV50_IR_SUBOP_BFIND_SAMT);
   emitINV  (0x28, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitPRMT()
{
   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0x5bc00000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0x4bc00000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0x36c00000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitField(0x30, 3, insn->subOp);
   emitGPR  (0x27, insn->src(2));
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

/*******************************************************************************
 * memory
 ******************************************************************************/

void
CodeEmitterGM107::emitLDSTs(int pos, DataType type)
{
   int data = 0;

   switch (typeSizeof(type)) {
   case  1: data = isSignedType(type) ? 1 : 0; break;
   case  2: data = isSignedType(type) ? 3 : 2; break;
   case  4: data = 4; break;
   case  8: data = 5; break;
   case 16: data = 6; break;
   default:
      assert(!"bad type");
      break;
   }

   emitField(pos, 3, data);
}

void
CodeEmitterGM107::emitLDSTc(int pos)
{
   int mode = 0;

   switch (insn->cache) {
   case CACHE_CA: mode = 0; break;
   case CACHE_CG: mode = 1; break;
   case CACHE_CS: mode = 2; break;
   case CACHE_CV: mode = 3; break;
   default:
      assert(!"invalid caching mode");
      break;
   }

   emitField(pos, 2, mode);
}

void
CodeEmitterGM107::emitLDC()
{
   emitInsn (0xef900000);
   emitLDSTs(0x30, insn->dType);
   emitField(0x2c, 2, insn->subOp);
   emitCBUF (0x24, 0x08, 0x14, 16, 0, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitLDL()
{
   emitInsn (0xef400000);
   emitLDSTs(0x30, insn->dType);
   emitLDSTc(0x2c);
   emitADDR (0x08, 0x14, 24, 0, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitLDS()
{
   emitInsn (0xef480000);
   emitLDSTs(0x30, insn->dType);
   emitADDR (0x08, 0x14, 24, 0, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitLD()
{
   emitInsn (0x80000000);
   emitPRED (0x3a);
   emitLDSTc(0x38);
   emitLDSTs(0x35, insn->dType);
   emitField(0x34, 1, insn->src(0).getIndirect(0)->getSize() == 8);
   emitADDR (0x08, 0x14, 32, 0, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitSTL()
{
   emitInsn (0xef500000);
   emitLDSTs(0x30, insn->dType);
   emitLDSTc(0x2c);
   emitADDR (0x08, 0x14, 24, 0, insn->src(0));
   emitGPR  (0x00, insn->src(1));
}

void
CodeEmitterGM107::emitSTS()
{
   emitInsn (0xef580000);
   emitLDSTs(0x30, insn->dType);
   emitADDR (0x08, 0x14, 24, 0, insn->src(0));
   emitGPR  (0x00, insn->src(1));
}

void
CodeEmitterGM107::emitST()
{
   emitInsn (0xa0000000);
   emitPRED (0x3a);
   emitLDSTc(0x38);
   emitLDSTs(0x35, insn->dType);
   emitField(0x34, 1, insn->src(0).getIndirect(0)->getSize() == 8);
   emitADDR (0x08, 0x14, 32, 0, insn->src(0));
   emitGPR  (0x00, insn->src(1));
}

void
CodeEmitterGM107::emitALD()
{
   emitInsn (0xefd80000);
   emitField(0x2f, 2, (insn->getDef(0)->reg.size / 4) - 1);
   emitGPR  (0x27, insn->src(0).getIndirect(1));
   emitO    (0x20);
   emitP    (0x1f);
   emitADDR (0x08, 20, 10, 0, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitAST()
{
   emitInsn (0xeff00000);
   emitField(0x2f, 2, (typeSizeof(insn->dType) / 4) - 1);
   emitGPR  (0x27, insn->src(0).getIndirect(1));
   emitP    (0x1f);
   emitADDR (0x08, 20, 10, 0, insn->src(0));
   emitGPR  (0x00, insn->src(1));
}

void
CodeEmitterGM107::emitISBERD()
{
   emitInsn(0xefd00000);
   emitGPR (0x08, insn->src(0));
   emitGPR (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitAL2P()
{
   emitInsn (0xefa00000);
   emitField(0x2f, 2, (insn->getDef(0)->reg.size / 4) - 1);
   emitPRED (0x2c);
   emitO    (0x20);
   emitField(0x14, 11, insn->src(0).get()->reg.data.offset);
   emitGPR  (0x08, insn->src(0).getIndirect(0));
   emitGPR  (0x00, insn->def(0));
}

void
gm107_interpApply(const FixupEntry *entry, uint32_t *code, const FixupData& data)
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
   code[loc + 1] &= ~(0xf << 0x14);
   code[loc + 1] |= (ipa & 0x3) << 0x16;
   code[loc + 1] |= (ipa & 0xc) << (0x14 - 2);
   code[loc + 0] &= ~(0xff << 0x14);
   code[loc + 0] |= reg << 0x14;
}

void
CodeEmitterGM107::emitIPA()
{
   int ipam = 0, ipas = 0;

   switch (insn->getInterpMode()) {
   case NV50_IR_INTERP_LINEAR     : ipam = 0; break;
   case NV50_IR_INTERP_PERSPECTIVE: ipam = 1; break;
   case NV50_IR_INTERP_FLAT       : ipam = 2; break;
   case NV50_IR_INTERP_SC         : ipam = 3; break;
   default:
      assert(!"invalid ipa mode");
      break;
   }

   switch (insn->getSampleMode()) {
   case NV50_IR_INTERP_DEFAULT : ipas = 0; break;
   case NV50_IR_INTERP_CENTROID: ipas = 1; break;
   case NV50_IR_INTERP_OFFSET  : ipas = 2; break;
   default:
      assert(!"invalid ipa sample mode");
      break;
   }

   emitInsn (0xe0000000);
   emitField(0x36, 2, ipam);
   emitField(0x34, 2, ipas);
   emitSAT  (0x33);
   emitField(0x2f, 3, 7);
   emitADDR (0x08, 0x1c, 10, 0, insn->src(0));
   if ((code[0] & 0x0000ff00) != 0x0000ff00)
      code[1] |= 0x00000040; /* .idx */
   emitGPR(0x00, insn->def(0));

   if (insn->op == OP_PINTERP) {
      emitGPR(0x14, insn->src(1));
      if (insn->getSampleMode() == NV50_IR_INTERP_OFFSET)
         emitGPR(0x27, insn->src(2));
      addInterp(insn->ipa, insn->getSrc(1)->reg.data.id, gm107_interpApply);
   } else {
      if (insn->getSampleMode() == NV50_IR_INTERP_OFFSET)
         emitGPR(0x27, insn->src(1));
      emitGPR(0x14);
      addInterp(insn->ipa, 0xff, gm107_interpApply);
   }

   if (insn->getSampleMode() != NV50_IR_INTERP_OFFSET)
      emitGPR(0x27);
}

void
CodeEmitterGM107::emitATOM()
{
   unsigned dType, subOp;

   if (insn->subOp == NV50_IR_SUBOP_ATOM_CAS) {
      switch (insn->dType) {
      case TYPE_U32: dType = 0; break;
      case TYPE_U64: dType = 1; break;
      default: assert(!"unexpected dType"); dType = 0; break;
      }
      subOp = 15;

      emitInsn (0xee000000);
   } else {
      switch (insn->dType) {
      case TYPE_U32: dType = 0; break;
      case TYPE_S32: dType = 1; break;
      case TYPE_U64: dType = 2; break;
      case TYPE_F32: dType = 3; break;
      case TYPE_B128: dType = 4; break;
      case TYPE_S64: dType = 5; break;
      default: assert(!"unexpected dType"); dType = 0; break;
      }
      if (insn->subOp == NV50_IR_SUBOP_ATOM_EXCH)
         subOp = 8;
      else
         subOp = insn->subOp;

      emitInsn (0xed000000);
   }

   emitField(0x34, 4, subOp);
   emitField(0x31, 3, dType);
   emitField(0x30, 1, insn->src(0).getIndirect(0)->getSize() == 8);
   emitGPR  (0x14, insn->src(1));
   emitADDR (0x08, 0x1c, 20, 0, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitATOMS()
{
   unsigned dType, subOp;

   if (insn->subOp == NV50_IR_SUBOP_ATOM_CAS) {
      switch (insn->dType) {
      case TYPE_U32: dType = 0; break;
      case TYPE_U64: dType = 1; break;
      default: assert(!"unexpected dType"); dType = 0; break;
      }
      subOp = 4;

      emitInsn (0xee000000);
      emitField(0x34, 1, dType);
   } else {
      switch (insn->dType) {
      case TYPE_U32: dType = 0; break;
      case TYPE_S32: dType = 1; break;
      case TYPE_U64: dType = 2; break;
      case TYPE_S64: dType = 3; break;
      default: assert(!"unexpected dType"); dType = 0; break;
      }

      if (insn->subOp == NV50_IR_SUBOP_ATOM_EXCH)
         subOp = 8;
      else
         subOp = insn->subOp;

      emitInsn (0xec000000);
      emitField(0x1c, 3, dType);
   }

   emitField(0x34, 4, subOp);
   emitGPR  (0x14, insn->src(1));
   emitADDR (0x08, 0x1e, 22, 2, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitRED()
{
   unsigned dType;

   switch (insn->dType) {
   case TYPE_U32: dType = 0; break;
   case TYPE_S32: dType = 1; break;
   case TYPE_U64: dType = 2; break;
   case TYPE_F32: dType = 3; break;
   case TYPE_B128: dType = 4; break;
   case TYPE_S64: dType = 5; break;
   default: assert(!"unexpected dType"); dType = 0; break;
   }

   emitInsn (0xebf80000);
   emitField(0x30, 1, insn->src(0).getIndirect(0)->getSize() == 8);
   emitField(0x17, 3, insn->subOp);
   emitField(0x14, 3, dType);
   emitADDR (0x08, 0x1c, 20, 0, insn->src(0));
   emitGPR  (0x00, insn->src(1));
}

void
CodeEmitterGM107::emitCCTL()
{
   unsigned width;
   if (insn->src(0).getFile() == FILE_MEMORY_GLOBAL) {
      emitInsn(0xef600000);
      width = 30;
   } else {
      emitInsn(0xef800000);
      width = 22;
   }
   emitField(0x34, 1, insn->src(0).getIndirect(0)->getSize() == 8);
   emitADDR (0x08, 0x16, width, 2, insn->src(0));
   emitField(0x00, 4, insn->subOp);
}

/*******************************************************************************
 * surface
 ******************************************************************************/

void
CodeEmitterGM107::emitPIXLD()
{
   emitInsn (0xefe80000);
   emitPRED (0x2d);
   emitField(0x1f, 3, insn->subOp);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

/*******************************************************************************
 * texture
 ******************************************************************************/

void
CodeEmitterGM107::emitTEXs(int pos)
{
   int src1 = insn->predSrc == 1 ? 2 : 1;
   if (insn->srcExists(src1))
      emitGPR(pos, insn->src(src1));
   else
      emitGPR(pos);
}

static uint8_t
getTEXSMask(uint8_t mask)
{
   switch (mask) {
   case 0x1: return 0x0;
   case 0x2: return 0x1;
   case 0x3: return 0x4;
   case 0x4: return 0x2;
   case 0x7: return 0x0;
   case 0x8: return 0x3;
   case 0x9: return 0x5;
   case 0xa: return 0x6;
   case 0xb: return 0x1;
   case 0xc: return 0x7;
   case 0xd: return 0x2;
   case 0xe: return 0x3;
   case 0xf: return 0x4;
   default:
      assert(!"invalid mask");
      return 0;
   }
}

static uint8_t
getTEXSTarget(const TexInstruction *tex)
{
   assert(tex->op == OP_TEX || tex->op == OP_TXL);

   switch (tex->tex.target.getEnum()) {
   case TEX_TARGET_1D:
      assert(tex->tex.levelZero);
      return 0x0;
   case TEX_TARGET_2D:
   case TEX_TARGET_RECT:
      if (tex->tex.levelZero)
         return 0x2;
      if (tex->op == OP_TXL)
         return 0x3;
      return 0x1;
   case TEX_TARGET_2D_SHADOW:
   case TEX_TARGET_RECT_SHADOW:
      if (tex->tex.levelZero)
         return 0x6;
      if (tex->op == OP_TXL)
         return 0x5;
      return 0x4;
   case TEX_TARGET_2D_ARRAY:
      if (tex->tex.levelZero)
         return 0x8;
      return 0x7;
   case TEX_TARGET_2D_ARRAY_SHADOW:
      assert(tex->tex.levelZero);
      return 0x9;
   case TEX_TARGET_3D:
      if (tex->tex.levelZero)
         return 0xb;
      assert(tex->op != OP_TXL);
      return 0xa;
   case TEX_TARGET_CUBE:
      assert(!tex->tex.levelZero);
      if (tex->op == OP_TXL)
         return 0xd;
      return 0xc;
   default:
      assert(false);
      return 0x0;
   }
}

static uint8_t
getTLDSTarget(const TexInstruction *tex)
{
   switch (tex->tex.target.getEnum()) {
   case TEX_TARGET_1D:
      if (tex->tex.levelZero)
         return 0x0;
      return 0x1;
   case TEX_TARGET_2D:
   case TEX_TARGET_RECT:
      if (tex->tex.levelZero)
         return tex->tex.useOffsets ? 0x4 : 0x2;
      return tex->tex.useOffsets ? 0xc : 0x5;
   case TEX_TARGET_2D_MS:
      assert(tex->tex.levelZero);
      return 0x6;
   case TEX_TARGET_3D:
      assert(tex->tex.levelZero);
      return 0x7;
   case TEX_TARGET_2D_ARRAY:
      assert(tex->tex.levelZero);
      return 0x8;

   default:
      assert(false);
      return 0x0;
   }
}

void
CodeEmitterGM107::emitTEX()
{
   const TexInstruction *insn = this->insn->asTex();
   int lodm = 0;

   if (!insn->tex.levelZero) {
      switch (insn->op) {
      case OP_TEX: lodm = 0; break;
      case OP_TXB: lodm = 2; break;
      case OP_TXL: lodm = 3; break;
      default:
         assert(!"invalid tex op");
         break;
      }
   } else {
      lodm = 1;
   }

   if (insn->tex.rIndirectSrc >= 0) {
      emitInsn (0xdeb80000);
      emitField(0x25, 2, lodm);
      emitField(0x24, 1, insn->tex.useOffsets == 1);
   } else {
      emitInsn (0xc0380000);
      emitField(0x37, 2, lodm);
      emitField(0x36, 1, insn->tex.useOffsets == 1);
      emitField(0x24, 13, insn->tex.r);
   }

   emitField(0x32, 1, insn->tex.target.isShadow());
   emitField(0x31, 1, insn->tex.liveOnly);
   emitField(0x23, 1, insn->tex.derivAll);
   emitField(0x1f, 4, insn->tex.mask);
   emitField(0x1d, 2, insn->tex.target.isCube() ? 3 :
                      insn->tex.target.getDim() - 1);
   emitField(0x1c, 1, insn->tex.target.isArray());
   emitTEXs (0x14);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitTEXS()
{
   const TexInstruction *insn = this->insn->asTex();
   assert(!insn->tex.derivAll);

   switch (insn->op) {
   case OP_TEX:
   case OP_TXL:
      emitInsn (0xd8000000);
      emitField(0x35, 4, getTEXSTarget(insn));
      emitField(0x32, 3, getTEXSMask(insn->tex.mask));
      break;
   case OP_TXF:
      emitInsn (0xda000000);
      emitField(0x35, 4, getTLDSTarget(insn));
      emitField(0x32, 3, getTEXSMask(insn->tex.mask));
      break;
   case OP_TXG:
      assert(insn->tex.useOffsets != 4);
      emitInsn (0xdf000000);
      emitField(0x34, 2, insn->tex.gatherComp);
      emitField(0x33, 1, insn->tex.useOffsets == 1);
      emitField(0x32, 1, insn->tex.target.isShadow());
      break;
   default:
      unreachable("unknown op in emitTEXS()");
      break;
   }

   emitField(0x31, 1, insn->tex.liveOnly);
   emitField(0x24, 13, insn->tex.r);
   if (insn->defExists(1))
      emitGPR(0x1c, insn->def(1));
   else
      emitGPR(0x1c);
   if (insn->srcExists(1))
      emitGPR(0x14, insn->getSrc(1));
   else
      emitGPR(0x14);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitTLD()
{
   const TexInstruction *insn = this->insn->asTex();

   if (insn->tex.rIndirectSrc >= 0) {
      emitInsn (0xdd380000);
   } else {
      emitInsn (0xdc380000);
      emitField(0x24, 13, insn->tex.r);
   }

   emitField(0x37, 1, insn->tex.levelZero == 0);
   emitField(0x32, 1, insn->tex.target.isMS());
   emitField(0x31, 1, insn->tex.liveOnly);
   emitField(0x23, 1, insn->tex.useOffsets == 1);
   emitField(0x1f, 4, insn->tex.mask);
   emitField(0x1d, 2, insn->tex.target.isCube() ? 3 :
                      insn->tex.target.getDim() - 1);
   emitField(0x1c, 1, insn->tex.target.isArray());
   emitTEXs (0x14);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitTLD4()
{
   const TexInstruction *insn = this->insn->asTex();

   if (insn->tex.rIndirectSrc >= 0) {
      emitInsn (0xdef80000);
      emitField(0x26, 2, insn->tex.gatherComp);
      emitField(0x25, 2, insn->tex.useOffsets == 4);
      emitField(0x24, 2, insn->tex.useOffsets == 1);
   } else {
      emitInsn (0xc8380000);
      emitField(0x38, 2, insn->tex.gatherComp);
      emitField(0x37, 2, insn->tex.useOffsets == 4);
      emitField(0x36, 2, insn->tex.useOffsets == 1);
      emitField(0x24, 13, insn->tex.r);
   }

   emitField(0x32, 1, insn->tex.target.isShadow());
   emitField(0x31, 1, insn->tex.liveOnly);
   emitField(0x23, 1, insn->tex.derivAll);
   emitField(0x1f, 4, insn->tex.mask);
   emitField(0x1d, 2, insn->tex.target.isCube() ? 3 :
                      insn->tex.target.getDim() - 1);
   emitField(0x1c, 1, insn->tex.target.isArray());
   emitTEXs (0x14);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitTXD()
{
   const TexInstruction *insn = this->insn->asTex();

   if (insn->tex.rIndirectSrc >= 0) {
      emitInsn (0xde780000);
   } else {
      emitInsn (0xde380000);
      emitField(0x24, 13, insn->tex.r);
   }

   emitField(0x31, 1, insn->tex.liveOnly);
   emitField(0x23, 1, insn->tex.useOffsets == 1);
   emitField(0x1f, 4, insn->tex.mask);
   emitField(0x1d, 2, insn->tex.target.isCube() ? 3 :
                      insn->tex.target.getDim() - 1);
   emitField(0x1c, 1, insn->tex.target.isArray());
   emitTEXs (0x14);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitTMML()
{
   const TexInstruction *insn = this->insn->asTex();

   if (insn->tex.rIndirectSrc >= 0) {
      emitInsn (0xdf600000);
   } else {
      emitInsn (0xdf580000);
      emitField(0x24, 13, insn->tex.r);
   }

   emitField(0x31, 1, insn->tex.liveOnly);
   emitField(0x23, 1, insn->tex.derivAll);
   emitField(0x1f, 4, insn->tex.mask);
   emitField(0x1d, 2, insn->tex.target.isCube() ? 3 :
                      insn->tex.target.getDim() - 1);
   emitField(0x1c, 1, insn->tex.target.isArray());
   emitTEXs (0x14);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitTXQ()
{
   const TexInstruction *insn = this->insn->asTex();
   int type = 0;

   switch (insn->tex.query) {
   case TXQ_DIMS           : type = 0x01; break;
   case TXQ_TYPE           : type = 0x02; break;
   case TXQ_SAMPLE_POSITION: type = 0x05; break;
   case TXQ_FILTER         : type = 0x10; break;
   case TXQ_LOD            : type = 0x12; break;
   case TXQ_WRAP           : type = 0x14; break;
   case TXQ_BORDER_COLOUR  : type = 0x16; break;
   default:
      assert(!"invalid txq query");
      break;
   }

   if (insn->tex.rIndirectSrc >= 0) {
      emitInsn (0xdf500000);
   } else {
      emitInsn (0xdf480000);
      emitField(0x24, 13, insn->tex.r);
   }

   emitField(0x31, 1, insn->tex.liveOnly);
   emitField(0x1f, 4, insn->tex.mask);
   emitField(0x16, 6, type);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitDEPBAR()
{
   emitInsn (0xf0f00000);
   emitField(0x1d, 1, 1); /* le */
   emitField(0x1a, 3, 5);
   emitField(0x14, 6, insn->subOp);
   emitField(0x00, 6, insn->subOp);
}

/*******************************************************************************
 * misc
 ******************************************************************************/

void
CodeEmitterGM107::emitNOP()
{
   emitInsn(0x50b00000);
}

void
CodeEmitterGM107::emitKIL()
{
   emitInsn (0xe3300000);
   emitCond5(0x00, CC_TR);
}

void
CodeEmitterGM107::emitOUT()
{
   const int cut  = insn->op == OP_RESTART || insn->subOp;
   const int emit = insn->op == OP_EMIT;

   switch (insn->src(1).getFile()) {
   case FILE_GPR:
      emitInsn(0xfbe00000);
      emitGPR (0x14, insn->src(1));
      break;
   case FILE_IMMEDIATE:
      emitInsn(0xf6e00000);
      emitIMMD(0x14, 19, insn->src(1));
      break;
   case FILE_MEMORY_CONST:
      emitInsn(0xebe00000);
      emitCBUF(0x22, -1, 0x14, 16, 2, insn->src(1));
      break;
   default:
      assert(!"bad src1 file");
      break;
   }

   emitField(0x27, 2, (cut << 1) | emit);
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));
}

void
CodeEmitterGM107::emitBAR()
{
   uint8_t subop;

   emitInsn (0xf0a80000);

   switch (insn->subOp) {
   case NV50_IR_SUBOP_BAR_RED_POPC: subop = 0x02; break;
   case NV50_IR_SUBOP_BAR_RED_AND:  subop = 0x0a; break;
   case NV50_IR_SUBOP_BAR_RED_OR:   subop = 0x12; break;
   case NV50_IR_SUBOP_BAR_ARRIVE:   subop = 0x81; break;
   default:
      subop = 0x80;
      assert(insn->subOp == NV50_IR_SUBOP_BAR_SYNC);
      break;
   }

   emitField(0x20, 8, subop);

   // barrier id
   if (insn->src(0).getFile() == FILE_GPR) {
      emitGPR(0x08, insn->src(0));
   } else {
      ImmediateValue *imm = insn->getSrc(0)->asImm();
      assert(imm);
      emitField(0x08, 8, imm->reg.data.u32);
      emitField(0x2b, 1, 1);
   }

   // thread count
   if (insn->src(1).getFile() == FILE_GPR) {
      emitGPR(0x14, insn->src(1));
   } else {
      ImmediateValue *imm = insn->getSrc(0)->asImm();
      assert(imm);
      emitField(0x14, 12, imm->reg.data.u32);
      emitField(0x2c, 1, 1);
   }

   if (insn->srcExists(2) && (insn->predSrc != 2)) {
      emitPRED (0x27, insn->src(2));
      emitField(0x2a, 1, insn->src(2).mod == Modifier(NV50_IR_MOD_NOT));
   } else {
      emitField(0x27, 3, 7);
   }
}

void
CodeEmitterGM107::emitMEMBAR()
{
   emitInsn (0xef980000);
   emitField(0x08, 2, insn->subOp >> 2);
}

void
CodeEmitterGM107::emitVOTE()
{
   const ImmediateValue *imm;
   uint32_t u32;

   int r = -1, p = -1;
   for (int i = 0; insn->defExists(i); i++) {
      if (insn->def(i).getFile() == FILE_GPR)
         r = i;
      else if (insn->def(i).getFile() == FILE_PREDICATE)
         p = i;
   }

   emitInsn (0x50d80000);
   emitField(0x30, 2, insn->subOp);
   if (r >= 0)
      emitGPR  (0x00, insn->def(r));
   else
      emitGPR  (0x00);
   if (p >= 0)
      emitPRED (0x2d, insn->def(p));
   else
      emitPRED (0x2d);

   switch (insn->src(0).getFile()) {
   case FILE_PREDICATE:
      emitField(0x2a, 1, insn->src(0).mod == Modifier(NV50_IR_MOD_NOT));
      emitPRED (0x27, insn->src(0));
      break;
   case FILE_IMMEDIATE:
      imm = insn->getSrc(0)->asImm();
      assert(imm);
      u32 = imm->reg.data.u32;
      assert(u32 == 0 || u32 == 1);
      emitPRED(0x27);
      emitField(0x2a, 1, u32 == 0);
      break;
   default:
      assert(!"Unhandled src");
      break;
   }
}

void
CodeEmitterGM107::emitSUTarget()
{
   const TexInstruction *insn = this->insn->asTex();
   int target = 0;

   assert(insn->op >= OP_SULDB && insn->op <= OP_SUREDP);

   if (insn->tex.target == TEX_TARGET_BUFFER) {
      target = 2;
   } else if (insn->tex.target == TEX_TARGET_1D_ARRAY) {
      target = 4;
   } else if (insn->tex.target == TEX_TARGET_2D ||
              insn->tex.target == TEX_TARGET_RECT) {
      target = 6;
   } else if (insn->tex.target == TEX_TARGET_2D_ARRAY ||
              insn->tex.target == TEX_TARGET_CUBE ||
              insn->tex.target == TEX_TARGET_CUBE_ARRAY) {
      target = 8;
   } else if (insn->tex.target == TEX_TARGET_3D) {
      target = 10;
   } else {
      assert(insn->tex.target == TEX_TARGET_1D);
   }
   emitField(0x20, 4, target);
}

void
CodeEmitterGM107::emitSUHandle(const int s)
{
   const TexInstruction *insn = this->insn->asTex();

   assert(insn->op >= OP_SULDB && insn->op <= OP_SUREDP);

   if (insn->src(s).getFile() == FILE_GPR) {
      emitGPR(0x27, insn->src(s));
   } else {
      ImmediateValue *imm = insn->getSrc(s)->asImm();
      assert(imm);
      emitField(0x33, 1, 1);
      emitField(0x24, 13, imm->reg.data.u32);
   }
}

void
CodeEmitterGM107::emitSUSTx()
{
   const TexInstruction *insn = this->insn->asTex();

   emitInsn(0xeb200000);
   if (insn->op == OP_SUSTB)
      emitField(0x34, 1, 1);
   emitSUTarget();

   emitLDSTc(0x18);
   emitField(0x14, 4, 0xf); // rgba
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->src(1));

   emitSUHandle(2);
}

void
CodeEmitterGM107::emitSULDx()
{
   const TexInstruction *insn = this->insn->asTex();

   emitInsn(0xeb000000);

   if (insn->op == OP_SULDB) {
      int type = 0;
      emitField(0x34, 1, 1);
      switch (insn->dType) {
      case TYPE_S8:   type = 1; break;
      case TYPE_U16:  type = 2; break;
      case TYPE_S16:  type = 3; break;
      case TYPE_U32:  type = 4; break;
      case TYPE_U64:  type = 5; break;
      case TYPE_B128: type = 6; break;
      default:
         assert(insn->dType == TYPE_U8);
         break;
      }
      emitField(0x14, 3, type);
   } else {
      emitField(0x14, 4, 0xf); // rgba
   }

   emitSUTarget();
   emitLDSTc(0x18);
   emitGPR  (0x00, insn->def(0));
   emitGPR  (0x08, insn->src(0));

   emitSUHandle(1);
}

void
CodeEmitterGM107::emitSUREDx()
{
   const TexInstruction *insn = this->insn->asTex();
   uint8_t type = 0, subOp;

   if (insn->subOp == NV50_IR_SUBOP_ATOM_CAS)
      emitInsn(0xeac00000);
   else
      emitInsn(0xea600000);

   if (insn->op == OP_SUREDB)
      emitField(0x34, 1, 1);
   emitSUTarget();

   // destination type
   switch (insn->dType) {
   case TYPE_S32: type = 1; break;
   case TYPE_U64: type = 2; break;
   case TYPE_F32: type = 3; break;
   case TYPE_S64: type = 5; break;
   default:
      assert(insn->dType == TYPE_U32);
      break;
   }

   // atomic operation
   if (insn->subOp == NV50_IR_SUBOP_ATOM_CAS) {
      subOp = 0;
   } else if (insn->subOp == NV50_IR_SUBOP_ATOM_EXCH) {
      subOp = 8;
   } else {
      subOp = insn->subOp;
   }

   emitField(0x24, 3, type);
   emitField(0x1d, 4, subOp);
   emitGPR  (0x14, insn->src(1));
   emitGPR  (0x08, insn->src(0));
   emitGPR  (0x00, insn->def(0));

   emitSUHandle(2);
}

/*******************************************************************************
 * assembler front-end
 ******************************************************************************/

bool
CodeEmitterGM107::emitInstruction(Instruction *i)
{
   const unsigned int size = (writeIssueDelays && !(codeSize & 0x1f)) ? 16 : 8;
   bool ret = true;

   insn = i;

   if (insn->encSize != 8) {
      ERROR("skipping undecodable instruction: "); insn->print();
      return false;
   } else
   if (codeSize + size > codeSizeLimit) {
      ERROR("code emitter output buffer too small\n");
      return false;
   }

   if (writeIssueDelays) {
      int n = ((codeSize & 0x1f) / 8) - 1;
      if (n < 0) {
         data = code;
         data[0] = 0x00000000;
         data[1] = 0x00000000;
         code += 2;
         codeSize += 8;
         n++;
      }

      emitField(data, n * 21, 21, insn->sched);
   }

   switch (insn->op) {
   case OP_EXIT:
      emitEXIT();
      break;
   case OP_BRA:
      emitBRA();
      break;
   case OP_CALL:
      emitCAL();
      break;
   case OP_PRECONT:
      emitPCNT();
      break;
   case OP_CONT:
      emitCONT();
      break;
   case OP_PREBREAK:
      emitPBK();
      break;
   case OP_BREAK:
      emitBRK();
      break;
   case OP_PRERET:
      emitPRET();
      break;
   case OP_RET:
      emitRET();
      break;
   case OP_JOINAT:
      emitSSY();
      break;
   case OP_JOIN:
      emitSYNC();
      break;
   case OP_QUADON:
      emitSAM();
      break;
   case OP_QUADPOP:
      emitRAM();
      break;
   case OP_MOV:
      emitMOV();
      break;
   case OP_RDSV:
      if (targGM107->isCS2RSV(insn->getSrc(0)->reg.data.sv.sv))
         emitCS2R();
      else
         emitS2R();
      break;
   case OP_ABS:
   case OP_NEG:
   case OP_SAT:
   case OP_FLOOR:
   case OP_CEIL:
   case OP_TRUNC:
   case OP_CVT:
      if (insn->op == OP_CVT && (insn->def(0).getFile() == FILE_PREDICATE ||
                                 insn->src(0).getFile() == FILE_PREDICATE)) {
         emitMOV();
      } else if (isFloatType(insn->dType)) {
         if (isFloatType(insn->sType))
            emitF2F();
         else
            emitI2F();
      } else {
         if (isFloatType(insn->sType))
            emitF2I();
         else
            emitI2I();
      }
      break;
   case OP_SHFL:
      emitSHFL();
      break;
   case OP_ADD:
   case OP_SUB:
      if (isFloatType(insn->dType)) {
         if (insn->dType == TYPE_F64)
            emitDADD();
         else
            emitFADD();
      } else {
         emitIADD();
      }
      break;
   case OP_MUL:
      if (isFloatType(insn->dType)) {
         if (insn->dType == TYPE_F64)
            emitDMUL();
         else
            emitFMUL();
      } else {
         emitIMUL();
      }
      break;
   case OP_MAD:
   case OP_FMA:
      if (isFloatType(insn->dType)) {
         if (insn->dType == TYPE_F64)
            emitDFMA();
         else
            emitFFMA();
      } else {
         emitIMAD();
      }
      break;
   case OP_SHLADD:
      emitISCADD();
      break;
   case OP_XMAD:
      emitXMAD();
      break;
   case OP_MIN:
   case OP_MAX:
      if (isFloatType(insn->dType)) {
         if (insn->dType == TYPE_F64)
            emitDMNMX();
         else
            emitFMNMX();
      } else {
         emitIMNMX();
      }
      break;
   case OP_SHL:
      if (typeSizeof(insn->sType) == 8)
         emitSHF();
      else
         emitSHL();
      break;
   case OP_SHR:
      if (typeSizeof(insn->sType) == 8)
         emitSHF();
      else
         emitSHR();
      break;
   case OP_POPCNT:
      emitPOPC();
      break;
   case OP_INSBF:
      emitBFI();
      break;
   case OP_EXTBF:
      emitBFE();
      break;
   case OP_BFIND:
      emitFLO();
      break;
   case OP_PERMT:
      emitPRMT();
      break;
   case OP_SLCT:
      if (isFloatType(insn->dType))
         emitFCMP();
      else
         emitICMP();
      break;
   case OP_SET:
   case OP_SET_AND:
   case OP_SET_OR:
   case OP_SET_XOR:
      if (insn->def(0).getFile() != FILE_PREDICATE) {
         if (isFloatType(insn->sType))
            if (insn->sType == TYPE_F64)
               emitDSET();
            else
               emitFSET();
         else
            emitISET();
      } else {
         if (isFloatType(insn->sType))
            if (insn->sType == TYPE_F64)
               emitDSETP();
            else
               emitFSETP();
         else
            emitISETP();
      }
      break;
   case OP_SELP:
      emitSEL();
      break;
   case OP_PRESIN:
   case OP_PREEX2:
      emitRRO();
      break;
   case OP_COS:
   case OP_SIN:
   case OP_EX2:
   case OP_LG2:
   case OP_RCP:
   case OP_RSQ:
   case OP_SQRT:
      emitMUFU();
      break;
   case OP_AND:
   case OP_OR:
   case OP_XOR:
      switch (insn->def(0).getFile()) {
      case FILE_GPR: emitLOP(); break;
      case FILE_PREDICATE: emitPSETP(); break;
      default:
         assert(!"invalid bool op");
      }
      break;
   case OP_NOT:
      emitNOT();
      break;
   case OP_LOAD:
      switch (insn->src(0).getFile()) {
      case FILE_MEMORY_CONST : emitLDC(); break;
      case FILE_MEMORY_LOCAL : emitLDL(); break;
      case FILE_MEMORY_SHARED: emitLDS(); break;
      case FILE_MEMORY_GLOBAL: emitLD(); break;
      default:
         assert(!"invalid load");
         emitNOP();
         break;
      }
      break;
   case OP_STORE:
      switch (insn->src(0).getFile()) {
      case FILE_MEMORY_LOCAL : emitSTL(); break;
      case FILE_MEMORY_SHARED: emitSTS(); break;
      case FILE_MEMORY_GLOBAL: emitST(); break;
      default:
         assert(!"invalid store");
         emitNOP();
         break;
      }
      break;
   case OP_ATOM:
      if (insn->src(0).getFile() == FILE_MEMORY_SHARED)
         emitATOMS();
      else
         if (!insn->defExists(0) && insn->subOp < NV50_IR_SUBOP_ATOM_CAS)
            emitRED();
         else
            emitATOM();
      break;
   case OP_CCTL:
      emitCCTL();
      break;
   case OP_VFETCH:
      emitALD();
      break;
   case OP_EXPORT:
      emitAST();
      break;
   case OP_PFETCH:
      emitISBERD();
      break;
   case OP_AFETCH:
      emitAL2P();
      break;
   case OP_LINTERP:
   case OP_PINTERP:
      emitIPA();
      break;
   case OP_PIXLD:
      emitPIXLD();
      break;
   case OP_TEX:
   case OP_TXL:
      if (insn->asTex()->tex.scalar)
         emitTEXS();
      else
         emitTEX();
      break;
   case OP_TXB:
      emitTEX();
      break;
   case OP_TXF:
      if (insn->asTex()->tex.scalar)
         emitTEXS();
      else
         emitTLD();
      break;
   case OP_TXG:
      if (insn->asTex()->tex.scalar)
         emitTEXS();
      else
         emitTLD4();
      break;
   case OP_TXD:
      emitTXD();
      break;
   case OP_TXQ:
      emitTXQ();
      break;
   case OP_TXLQ:
      emitTMML();
      break;
   case OP_TEXBAR:
      emitDEPBAR();
      break;
   case OP_QUADOP:
      emitFSWZADD();
      break;
   case OP_NOP:
      emitNOP();
      break;
   case OP_DISCARD:
      emitKIL();
      break;
   case OP_EMIT:
   case OP_RESTART:
      emitOUT();
      break;
   case OP_BAR:
      emitBAR();
      break;
   case OP_MEMBAR:
      emitMEMBAR();
      break;
   case OP_VOTE:
      emitVOTE();
      break;
   case OP_SUSTB:
   case OP_SUSTP:
      emitSUSTx();
      break;
   case OP_SULDB:
   case OP_SULDP:
      emitSULDx();
      break;
   case OP_SUREDB:
   case OP_SUREDP:
      emitSUREDx();
      break;
   default:
      assert(!"invalid opcode");
      emitNOP();
      ret = false;
      break;
   }

   if (insn->join) {
      /*XXX*/
   }

   code += 2;
   codeSize += 8;
   return ret;
}

uint32_t
CodeEmitterGM107::getMinEncodingSize(const Instruction *i) const
{
   return 8;
}

/*******************************************************************************
 * sched data calculator
 ******************************************************************************/

inline void
SchedDataCalculatorGM107::emitStall(Instruction *insn, uint8_t cnt)
{
   assert(cnt < 16);
   insn->sched |= cnt;
}

inline void
SchedDataCalculatorGM107::emitYield(Instruction *insn)
{
   insn->sched |= 1 << 4;
}

inline void
SchedDataCalculatorGM107::emitWrDepBar(Instruction *insn, uint8_t id)
{
   assert(id < 6);
   if ((insn->sched & 0xe0) == 0xe0)
      insn->sched ^= 0xe0;
   insn->sched |= id << 5;
}

inline void
SchedDataCalculatorGM107::emitRdDepBar(Instruction *insn, uint8_t id)
{
   assert(id < 6);
   if ((insn->sched & 0x700) == 0x700)
      insn->sched ^= 0x700;
   insn->sched |= id << 8;
}

inline void
SchedDataCalculatorGM107::emitWtDepBar(Instruction *insn, uint8_t id)
{
   assert(id < 6);
   insn->sched |= 1 << (11 + id);
}

inline void
SchedDataCalculatorGM107::emitReuse(Instruction *insn, uint8_t id)
{
   assert(id < 4);
   insn->sched |= 1 << (17 + id);
}

inline void
SchedDataCalculatorGM107::printSchedInfo(int cycle,
                                         const Instruction *insn) const
{
   uint8_t st, yl, wr, rd, wt, ru;

   st = (insn->sched & 0x00000f) >> 0;
   yl = (insn->sched & 0x000010) >> 4;
   wr = (insn->sched & 0x0000e0) >> 5;
   rd = (insn->sched & 0x000700) >> 8;
   wt = (insn->sched & 0x01f800) >> 11;
   ru = (insn->sched & 0x1e0000) >> 17;

   INFO("cycle %i, (st 0x%x, yl 0x%x, wr 0x%x, rd 0x%x, wt 0x%x, ru 0x%x)\n",
        cycle, st, yl, wr, rd, wt, ru);
}

inline int
SchedDataCalculatorGM107::getStall(const Instruction *insn) const
{
   return insn->sched & 0xf;
}

inline int
SchedDataCalculatorGM107::getWrDepBar(const Instruction *insn) const
{
   return (insn->sched & 0x0000e0) >> 5;
}

inline int
SchedDataCalculatorGM107::getRdDepBar(const Instruction *insn) const
{
   return (insn->sched & 0x000700) >> 8;
}

inline int
SchedDataCalculatorGM107::getWtDepBar(const Instruction *insn) const
{
   return (insn->sched & 0x01f800) >> 11;
}

// Emit the reuse flag which allows to make use of the new memory hierarchy
// introduced since Maxwell, the operand reuse cache.
//
// It allows to reduce bank conflicts by caching operands. Each time you issue
// an instruction, that flag can tell the hw which operands are going to be
// re-used by the next instruction. Note that the next instruction has to use
// the same GPR id in the same operand slot.
void
SchedDataCalculatorGM107::setReuseFlag(Instruction *insn)
{
   Instruction *next = insn->next;
   BitSet defs(255, true);

   if (!targ->isReuseSupported(insn))
      return;

   for (int d = 0; insn->defExists(d); ++d) {
      const Value *def = insn->def(d).rep();
      if (insn->def(d).getFile() != FILE_GPR)
         continue;
      if (typeSizeof(insn->dType) != 4 || def->reg.data.id == 255)
         continue;
      defs.set(def->reg.data.id);
   }

   for (int s = 0; insn->srcExists(s); s++) {
      const Value *src = insn->src(s).rep();
      if (insn->src(s).getFile() != FILE_GPR)
         continue;
      if (typeSizeof(insn->sType) != 4 || src->reg.data.id == 255)
         continue;
      if (defs.test(src->reg.data.id))
         continue;
      if (!next->srcExists(s) || next->src(s).getFile() != FILE_GPR)
         continue;
      if (src->reg.data.id != next->getSrc(s)->reg.data.id)
         continue;
      assert(s < 4);
      emitReuse(insn, s);
   }
}

void
SchedDataCalculatorGM107::recordWr(const Value *v, int cycle, int ready)
{
   int a = v->reg.data.id, b;

   switch (v->reg.file) {
   case FILE_GPR:
      b = a + v->reg.size / 4;
      for (int r = a; r < b; ++r)
         score->rd.r[r] = ready;
      break;
   case FILE_PREDICATE:
      // To immediately use a predicate set by any instructions, the minimum
      // number of stall counts is 13.
      score->rd.p[a] = cycle + 13;
      break;
   case FILE_FLAGS:
      score->rd.c = ready;
      break;
   default:
      break;
   }
}

void
SchedDataCalculatorGM107::checkRd(const Value *v, int cycle, int &delay) const
{
   int a = v->reg.data.id, b;
   int ready = cycle;

   switch (v->reg.file) {
   case FILE_GPR:
      b = a + v->reg.size / 4;
      for (int r = a; r < b; ++r)
         ready = MAX2(ready, score->rd.r[r]);
      break;
   case FILE_PREDICATE:
      ready = MAX2(ready, score->rd.p[a]);
      break;
   case FILE_FLAGS:
      ready = MAX2(ready, score->rd.c);
      break;
   default:
      break;
   }
   if (cycle < ready)
      delay = MAX2(delay, ready - cycle);
}

void
SchedDataCalculatorGM107::commitInsn(const Instruction *insn, int cycle)
{
   const int ready = cycle + targ->getLatency(insn);

   for (int d = 0; insn->defExists(d); ++d)
      recordWr(insn->getDef(d), cycle, ready);

#ifdef GM107_DEBUG_SCHED_DATA
   score->print(cycle);
#endif
}

#define GM107_MIN_ISSUE_DELAY 0x1
#define GM107_MAX_ISSUE_DELAY 0xf

int
SchedDataCalculatorGM107::calcDelay(const Instruction *insn, int cycle) const
{
   int delay = 0, ready = cycle;

   for (int s = 0; insn->srcExists(s); ++s)
      checkRd(insn->getSrc(s), cycle, delay);

   // TODO: make use of getReadLatency()!

   return MAX2(delay, ready - cycle);
}

void
SchedDataCalculatorGM107::setDelay(Instruction *insn, int delay,
                                   const Instruction *next)
{
   const OpClass cl = targ->getOpClass(insn->op);
   int wr, rd;

   if (insn->op == OP_EXIT ||
       insn->op == OP_BAR ||
       insn->op == OP_MEMBAR) {
      delay = GM107_MAX_ISSUE_DELAY;
   } else
   if (insn->op == OP_QUADON ||
       insn->op == OP_QUADPOP) {
      delay = 0xd;
   } else
   if (cl == OPCLASS_FLOW || insn->join) {
      delay = 0xd;
   }

   if (!next || !targ->canDualIssue(insn, next)) {
      delay = CLAMP(delay, GM107_MIN_ISSUE_DELAY, GM107_MAX_ISSUE_DELAY);
   } else {
      delay = 0x0; // dual-issue
   }

   wr = getWrDepBar(insn);
   rd = getRdDepBar(insn);

   if (delay == GM107_MIN_ISSUE_DELAY && (wr & rd) != 7) {
      // Barriers take one additional clock cycle to become active on top of
      // the clock consumed by the instruction producing it.
      if (!next || insn->bb != next->bb) {
         delay = 0x2;
      } else {
         int wt = getWtDepBar(next);
         if ((wt & (1 << wr)) | (wt & (1 << rd)))
            delay = 0x2;
      }
   }

   emitStall(insn, delay);
}


// Return true when the given instruction needs to emit a read dependency
// barrier (for WaR hazards) because it doesn't operate at a fixed latency, and
// setting the maximum number of stall counts is not enough.
bool
SchedDataCalculatorGM107::needRdDepBar(const Instruction *insn) const
{
   BitSet srcs(255, true), defs(255, true);
   int a, b;

   if (!targ->isBarrierRequired(insn))
      return false;

   // Do not emit a read dependency barrier when the instruction doesn't use
   // any GPR (like st s[0x4] 0x0) as input because it's unnecessary.
   for (int s = 0; insn->srcExists(s); ++s) {
      const Value *src = insn->src(s).rep();
      if (insn->src(s).getFile() != FILE_GPR)
         continue;
      if (src->reg.data.id == 255)
         continue;

      a = src->reg.data.id;
      b = a + src->reg.size / 4;
      for (int r = a; r < b; ++r)
         srcs.set(r);
   }

   if (!srcs.popCount())
      return false;

   // Do not emit a read dependency barrier when the output GPRs are equal to
   // the input GPRs (like rcp $r0 $r0) because a write dependency barrier will
   // be produced and WaR hazards are prevented.
   for (int d = 0; insn->defExists(d); ++d) {
      const Value *def = insn->def(d).rep();
      if (insn->def(d).getFile() != FILE_GPR)
         continue;
      if (def->reg.data.id == 255)
         continue;

      a = def->reg.data.id;
      b = a + def->reg.size / 4;
      for (int r = a; r < b; ++r)
         defs.set(r);
   }

   srcs.andNot(defs);
   if (!srcs.popCount())
      return false;

   return true;
}

// Return true when the given instruction needs to emit a write dependency
// barrier (for RaW hazards) because it doesn't operate at a fixed latency, and
// setting the maximum number of stall counts is not enough. This is only legal
// if the instruction output something.
bool
SchedDataCalculatorGM107::needWrDepBar(const Instruction *insn) const
{
   if (!targ->isBarrierRequired(insn))
      return false;

   for (int d = 0; insn->defExists(d); ++d) {
      if (insn->def(d).getFile() == FILE_GPR ||
          insn->def(d).getFile() == FILE_FLAGS ||
          insn->def(d).getFile() == FILE_PREDICATE)
         return true;
   }
   return false;
}

// Helper function for findFirstUse() and findFirstDef()
bool
SchedDataCalculatorGM107::doesInsnWriteTo(const Instruction *insn,
                                          const Value *val) const
{
   if (val->reg.file != FILE_GPR &&
       val->reg.file != FILE_PREDICATE &&
       val->reg.file != FILE_FLAGS)
      return false;

   for (int d = 0; insn->defExists(d); ++d) {
      const Value* def = insn->getDef(d);
      int minGPR = def->reg.data.id;
      int maxGPR = minGPR + def->reg.size / 4 - 1;

      if (def->reg.file != val->reg.file)
         continue;

      if (def->reg.file == FILE_GPR) {
         if (val->reg.data.id + val->reg.size / 4 - 1 < minGPR ||
             val->reg.data.id > maxGPR)
            continue;
         return true;
      } else
      if (def->reg.file == FILE_PREDICATE) {
         if (val->reg.data.id != minGPR)
            continue;
         return true;
      } else
      if (def->reg.file == FILE_FLAGS) {
         if (val->reg.data.id != minGPR)
            continue;
         return true;
      }
   }

   return false;
}

// Find the next instruction inside the same basic block which uses (reads or
// writes from) the output of the given instruction in order to avoid RaW and
// WaW hazards.
Instruction *
SchedDataCalculatorGM107::findFirstUse(const Instruction *bari) const
{
   Instruction *insn, *next;

   if (!bari->defExists(0))
      return NULL;

   for (insn = bari->next; insn != NULL; insn = next) {
      next = insn->next;

      for (int s = 0; insn->srcExists(s); ++s)
         if (doesInsnWriteTo(bari, insn->getSrc(s)))
            return insn;

      for (int d = 0; insn->defExists(d); ++d)
         if (doesInsnWriteTo(bari, insn->getDef(d)))
            return insn;
   }
   return NULL;
}

// Find the next instruction inside the same basic block which overwrites, at
// least, one source of the given instruction in order to avoid WaR hazards.
Instruction *
SchedDataCalculatorGM107::findFirstDef(const Instruction *bari) const
{
   Instruction *insn, *next;

   if (!bari->srcExists(0))
      return NULL;

   for (insn = bari->next; insn != NULL; insn = next) {
      next = insn->next;

      for (int s = 0; bari->srcExists(s); ++s)
         if (doesInsnWriteTo(insn, bari->getSrc(s)))
            return insn;
   }
   return NULL;
}

// Dependency barriers:
// This pass is a bit ugly and could probably be improved by performing a
// better allocation.
//
// The main idea is to avoid WaR and RaW hazards by emitting read/write
// dependency barriers using the control codes.
bool
SchedDataCalculatorGM107::insertBarriers(BasicBlock *bb)
{
   std::list<LiveBarUse> live_uses;
   std::list<LiveBarDef> live_defs;
   Instruction *insn, *next;
   BitSet bars(6, true);
   int bar_id;

   for (insn = bb->getEntry(); insn != NULL; insn = next) {
      Instruction *usei = NULL, *defi = NULL;
      bool need_wr_bar, need_rd_bar;

      next = insn->next;

      // Expire old barrier uses.
      for (std::list<LiveBarUse>::iterator it = live_uses.begin();
           it != live_uses.end();) {
         if (insn->serial >= it->usei->serial) {
            int wr = getWrDepBar(it->insn);
            emitWtDepBar(insn, wr);
            bars.clr(wr); // free barrier
            it = live_uses.erase(it);
            continue;
         }
         ++it;
      }

      // Expire old barrier defs.
      for (std::list<LiveBarDef>::iterator it = live_defs.begin();
           it != live_defs.end();) {
         if (insn->serial >= it->defi->serial) {
            int rd = getRdDepBar(it->insn);
            emitWtDepBar(insn, rd);
            bars.clr(rd); // free barrier
            it = live_defs.erase(it);
            continue;
         }
         ++it;
      }

      need_wr_bar = needWrDepBar(insn);
      need_rd_bar = needRdDepBar(insn);

      if (need_wr_bar) {
         // When the instruction requires to emit a write dependency barrier
         // (all which write something at a variable latency), find the next
         // instruction which reads the outputs (or writes to them, potentially
         // completing before this insn.
         usei = findFirstUse(insn);

         // Allocate and emit a new barrier.
         bar_id = bars.findFreeRange(1);
         if (bar_id == -1)
            bar_id = 5;
         bars.set(bar_id);
         emitWrDepBar(insn, bar_id);
         if (usei)
            live_uses.push_back(LiveBarUse(insn, usei));
      }

      if (need_rd_bar) {
         // When the instruction requires to emit a read dependency barrier
         // (all which read something at a variable latency), find the next
         // instruction which will write the inputs.
         defi = findFirstDef(insn);

         if (usei && defi && usei->serial <= defi->serial)
            continue;

         // Allocate and emit a new barrier.
         bar_id = bars.findFreeRange(1);
         if (bar_id == -1)
            bar_id = 5;
         bars.set(bar_id);
         emitRdDepBar(insn, bar_id);
         if (defi)
            live_defs.push_back(LiveBarDef(insn, defi));
      }
   }

   // Remove unnecessary barrier waits.
   BitSet alive_bars(6, true);
   for (insn = bb->getEntry(); insn != NULL; insn = next) {
      int wr, rd, wt;

      next = insn->next;

      wr = getWrDepBar(insn);
      rd = getRdDepBar(insn);
      wt = getWtDepBar(insn);

      for (int idx = 0; idx < 6; ++idx) {
         if (!(wt & (1 << idx)))
            continue;
         if (!alive_bars.test(idx)) {
            insn->sched &= ~(1 << (11  + idx));
         } else {
            alive_bars.clr(idx);
         }
      }

      if (wr < 6)
         alive_bars.set(wr);
      if (rd < 6)
         alive_bars.set(rd);
   }

   return true;
}

bool
SchedDataCalculatorGM107::visit(Function *func)
{
   ArrayList insns;

   func->orderInstructions(insns);

   scoreBoards.resize(func->cfg.getSize());
   for (size_t i = 0; i < scoreBoards.size(); ++i)
      scoreBoards[i].wipe();
   return true;
}

bool
SchedDataCalculatorGM107::visit(BasicBlock *bb)
{
   Instruction *insn, *next = NULL;
   int cycle = 0;

   for (Instruction *insn = bb->getEntry(); insn; insn = insn->next) {
      /*XXX*/
      insn->sched = 0x7e0;
   }

   if (!debug_get_bool_option("NV50_PROG_SCHED", true))
      return true;

   // Insert read/write dependency barriers for instructions which don't
   // operate at a fixed latency.
   insertBarriers(bb);

   score = &scoreBoards.at(bb->getId());

   for (Graph::EdgeIterator ei = bb->cfg.incident(); !ei.end(); ei.next()) {
      // back branches will wait until all target dependencies are satisfied
      if (ei.getType() == Graph::Edge::BACK) // sched would be uninitialized
         continue;
      BasicBlock *in = BasicBlock::get(ei.getNode());
      score->setMax(&scoreBoards.at(in->getId()));
   }

#ifdef GM107_DEBUG_SCHED_DATA
   INFO("=== BB:%i initial scores\n", bb->getId());
   score->print(cycle);
#endif

   // Because barriers are allocated locally (intra-BB), we have to make sure
   // that all produced barriers have been consumed before entering inside a
   // new basic block. The best way is to do a global allocation pre RA but
   // it's really more difficult, especially because of the phi nodes. Anyways,
   // it seems like that waiting on a barrier which has already been consumed
   // doesn't add any additional cost, it's just not elegant!
   Instruction *start = bb->getEntry();
   if (start && bb->cfg.incidentCount() > 0) {
      for (int b = 0; b < 6; b++)
         emitWtDepBar(start, b);
   }

   for (insn = bb->getEntry(); insn && insn->next; insn = insn->next) {
      next = insn->next;

      commitInsn(insn, cycle);
      int delay = calcDelay(next, cycle);
      setDelay(insn, delay, next);
      cycle += getStall(insn);

      setReuseFlag(insn);

      // XXX: The yield flag seems to destroy a bunch of things when it is
      // set on every instruction, need investigation.
      //emitYield(insn);

#ifdef GM107_DEBUG_SCHED_DATA
      printSchedInfo(cycle, insn);
      insn->print();
      next->print();
#endif
   }

   if (!insn)
      return true;
   commitInsn(insn, cycle);

   int bbDelay = -1;

#ifdef GM107_DEBUG_SCHED_DATA
   fprintf(stderr, "last instruction is : ");
   insn->print();
   fprintf(stderr, "cycle=%d\n", cycle);
#endif

   for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
      BasicBlock *out = BasicBlock::get(ei.getNode());

      if (ei.getType() != Graph::Edge::BACK) {
         // Only test the first instruction of the outgoing block.
         next = out->getEntry();
         if (next) {
            bbDelay = MAX2(bbDelay, calcDelay(next, cycle));
         } else {
            // When the outgoing BB is empty, make sure to set the number of
            // stall counts needed by the instruction because we don't know the
            // next instruction.
            bbDelay = MAX2(bbDelay, targ->getLatency(insn));
         }
      } else {
         // Wait until all dependencies are satisfied.
         const int regsFree = score->getLatest();
         next = out->getFirst();
         for (int c = cycle; next && c < regsFree; next = next->next) {
            bbDelay = MAX2(bbDelay, calcDelay(next, c));
            c += getStall(next);
         }
         next = NULL;
      }
   }
   if (bb->cfg.outgoingCount() != 1)
      next = NULL;
   setDelay(insn, bbDelay, next);
   cycle += getStall(insn);

   score->rebase(cycle); // common base for initializing out blocks' scores
   return true;
}

/*******************************************************************************
 * main
 ******************************************************************************/

void
CodeEmitterGM107::prepareEmission(Function *func)
{
   SchedDataCalculatorGM107 sched(targGM107);
   CodeEmitter::prepareEmission(func);
   sched.run(func, true, true);
}

static inline uint32_t sizeToBundlesGM107(uint32_t size)
{
   return (size + 23) / 24;
}

void
CodeEmitterGM107::prepareEmission(Program *prog)
{
   for (ArrayList::Iterator fi = prog->allFuncs.iterator();
        !fi.end(); fi.next()) {
      Function *func = reinterpret_cast<Function *>(fi.get());
      func->binPos = prog->binSize;
      prepareEmission(func);

      // adjust sizes & positions for schedulding info:
      if (prog->getTarget()->hasSWSched) {
         uint32_t adjPos = func->binPos;
         BasicBlock *bb = NULL;
         for (int i = 0; i < func->bbCount; ++i) {
            bb = func->bbArray[i];
            int32_t adjSize = bb->binSize;
            if (adjPos % 32) {
               adjSize -= 32 - adjPos % 32;
               if (adjSize < 0)
                  adjSize = 0;
            }
            adjSize = bb->binSize + sizeToBundlesGM107(adjSize) * 8;
            bb->binPos = adjPos;
            bb->binSize = adjSize;
            adjPos += adjSize;
         }
         if (bb)
            func->binSize = adjPos - func->binPos;
      }

      prog->binSize += func->binSize;
   }
}

CodeEmitterGM107::CodeEmitterGM107(const TargetGM107 *target)
   : CodeEmitter(target),
     targGM107(target),
     progType(Program::TYPE_VERTEX),
     insn(NULL),
     writeIssueDelays(target->hasSWSched),
     data(NULL)
{
   code = NULL;
   codeSize = codeSizeLimit = 0;
   relocInfo = NULL;
}

CodeEmitter *
TargetGM107::createCodeEmitterGM107(Program::Type type)
{
   CodeEmitterGM107 *emit = new CodeEmitterGM107(this);
   emit->setProgramType(type);
   return emit;
}

} // namespace nv50_ir
