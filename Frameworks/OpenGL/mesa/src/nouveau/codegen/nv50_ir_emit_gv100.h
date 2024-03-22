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
#ifndef __NV50_IR_EMIT_GV100_H__
#define __NV50_IR_EMIT_GV100_H__
#include "nv50_ir_target_gv100.h"

namespace nv50_ir {

class CodeEmitterGV100 : public CodeEmitter {
public:
   CodeEmitterGV100(TargetGV100 *target);

   virtual bool emitInstruction(Instruction *);
   virtual uint32_t getMinEncodingSize(const Instruction *) const { return 16; }

private:
   const Program *prog;
   const TargetGV100 *targ;
   const Instruction *insn;

   virtual void prepareEmission(Program *);
   virtual void prepareEmission(Function *);
   virtual void prepareEmission(BasicBlock *);

   inline void emitInsn(uint32_t op) {
      code[0] = op;
      code[1] = 0;
      code[2] = 0;
      code[3] = 0;
      if (insn->predSrc >= 0) {
         emitField(12, 3, insn->getSrc(insn->predSrc)->rep()->reg.data.id);
         emitField(15, 1, insn->cc == CC_NOT_P);
      } else {
         emitField(12, 3, 7);
      }
   };

   inline void emitField(int b, int s, uint64_t v) {
      if (b >= 0) {
         uint64_t m = ~0ULL >> (64 - s);
         uint64_t d = v & m;
         assert(!(v & ~m) || (v & ~m) == ~m);
         if (b < 64 && b + s > 64) {
            *(uint64_t *)&code[0] |= d << b;
            *(uint64_t *)&code[2] |= d >> (64 - b);
         } else {
            *(uint64_t *)&code[(b/64*2)] |= d << (b & 0x3f);
         }
      }
   };

   inline void emitABS(int pos, int src, bool supported)
   {
      if (insn->src(src).mod.abs()) {
         assert(supported);
         emitField(pos, 1, 1);
      }
   }

   inline void emitABS(int pos, int src)
   {
      emitABS(pos, src, true);
   }

   inline void emitNEG(int pos, int src, bool supported) {
      if (insn->src(src).mod.neg()) {
         assert(supported);
         emitField(pos, 1, 1);
      }
   }

   inline void emitNEG(int pos, int src) {
      emitNEG(pos, src, true);
   }

   inline void emitNOT(int pos) {
      emitField(pos, 1, 0);
   };

   inline void emitNOT(int pos, const ValueRef &ref) {
      emitField(pos, 1, !!(ref.mod & Modifier(NV50_IR_MOD_NOT)));
   }

   inline void emitSAT(int pos) {
      emitField(pos, 1, insn->saturate);
   }

   inline void emitRND(int rmp, RoundMode rnd, int rip) {
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

   inline void emitRND(int pos) {
      emitRND(pos, insn->rnd, -1);
   }

   inline void emitFMZ(int pos, int len) {
      emitField(pos, len, insn->dnz << 1 | insn->ftz);
   }

   inline void emitPDIV(int pos) {
      emitField(pos, 3, insn->postFactor + 4);
   }

   inline void emitO(int pos) {
      emitField(pos, 1, insn->getSrc(0)->reg.file == FILE_SHADER_OUTPUT);
   }

   inline void emitP(int pos) {
      emitField(pos, 1, insn->perPatch);
   }

   inline void emitCond3(int pos, CondCode code) {
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

   inline void emitCond4(int pos, CondCode code) {
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

   inline void emitSYS(int pos, const Value *val) {
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

   inline void emitSYS(int pos, const ValueRef &ref) {
      emitSYS(pos, ref.get() ? ref.rep() : (const Value *)NULL);
   }

   inline void emitBTS(int pos, const Value *val) {
      if (val->inFile(FILE_THREAD_STATE)) {
         TSSemantic ts = val->reg.data.ts == TS_PQUAD_MACTIVE ? TS_MACTIVE : val->reg.data.ts;
         emitField(pos, 5, ts | 0x10);
      } else {
         emitField(pos, 5, val->reg.data.id);
      }
   }

   inline void emitBTS(int pos, const ValueRef &ref) {
      emitBTS(pos, ref.get() ? ref.rep() : (const Value *)NULL);
   }

   inline void emitBTS(int pos, const ValueDef &def) {
      emitBTS(pos, def.get() ? def.rep() : (const Value *)NULL);
   }

   inline void emitGPR(int pos, const Value *val, int off) {
      emitField(pos, 8, val && !val->inFile(FILE_FLAGS) ?
                val->reg.data.id + off: 255);
   }

   inline void emitGPR(int pos, const Value *v) {
      emitGPR(pos, v, 0);
   }

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

   inline void emitGPR(int pos, const ValueDef &def, int off) {
      emitGPR(pos, def.get() ? def.rep() : (const Value *)NULL, off);
   }

   inline void emitPRED(int pos, const Value *val) {
      emitField(pos, 3, val ? val->reg.data.id : 7);
   };

   inline void emitPRED(int pos) {
      emitPRED(pos, (const Value *)NULL);
   }

   inline void emitPRED(int pos, const ValueRef &ref) {
      emitPRED(pos, ref.get() ? ref.rep() : (const Value *)NULL);
   }

   inline void emitPRED(int pos, const ValueDef &def) {
      emitPRED(pos, def.get() ? def.rep() : (const Value *)NULL);
   }

   inline void emitCBUF(int buf, int gpr, int off, int len, int align,
                        const ValueRef &ref) {
      const Value *v = ref.get();
      const Symbol *s = v->asSym();

      assert(!(s->reg.data.offset & ((1 << align) - 1)));

      emitField(buf,  5, v->reg.fileIndex);
      if (gpr >= 0)
         emitGPR(gpr, ref.getIndirect(0));
      emitField(off, 16, s->reg.data.offset);
   }

   inline void emitIMMD(int pos, int len, const ValueRef &ref) {
      const ImmediateValue *imm = ref.get()->asImm();
      uint32_t val = imm->reg.data.u32;

      if (insn->sType == TYPE_F64) {
         assert(!(imm->reg.data.u64 & 0x00000000ffffffffULL));
         val = imm->reg.data.u64 >> 32;
      }

      emitField(pos, len, val);
   }

   inline void emitADDR(int gpr, int off, int len, int shr,
                        const ValueRef &ref) {
      const Value *v = ref.get();
      assert(!(v->reg.data.offset & ((1 << shr) - 1)));
      if (gpr >= 0)
         emitGPR(gpr, ref.getIndirect(0));
      emitField(off, len, v->reg.data.offset >> shr);
   }

   inline void emitFormA(uint16_t op, uint8_t forms, int src0, int src1, int src2);
   inline void emitFormA_RRR(uint16_t op, int src1, int src2);
   inline void emitFormA_RRI(uint16_t op, int src1, int src2);
   inline void emitFormA_RRC(uint16_t op, int src1, int src2);
   inline void emitFormA_I32(int src);

   void emitBRA();
   void emitEXIT();
   void emitKILL();
   void emitNOP();
   void emitWARPSYNC();

   void emitCS2R();
   void emitF2F();
   void emitF2I();
   void emitFRND();
   void emitI2F();
   void emitMOV();
   void emitPRMT();
   void emitS2R();
   void emitSEL();
   void emitSHFL();

   void emitFADD();
   void emitFFMA();
   void emitFMNMX();
   void emitFMUL();
   void emitFSET_BF();
   void emitFSETP();
   void emitFSWZADD();
   void emitMUFU();

   void emitDADD();
   void emitDFMA();
   void emitDMUL();
   void emitDSETP();

   void emitBMSK();
   void emitBREV();
   void emitFLO();
   void emitIABS();
   void emitIADD3();
   void emitIMAD();
   void emitIMAD_WIDE();
   void emitISETP();
   void emitLEA();
   void emitLOP3_LUT();
   void emitPOPC();
   void emitSGXT();
   void emitSHF();

   void emitALD();
   void emitAST();
   void emitATOM();
   void emitATOMS();
   void emitIPA();
   void emitISBERD();
   void emitLDSTc(int, int);
   void emitLDSTs(int, DataType);
   void emitLD();
   void emitLDC();
   void emitLDL();
   void emitLDS();
   void emitOUT();
   void emitRED();
   void emitST();
   void emitSTL();
   void emitSTS();

   void emitTEXs(int);
   void emitTEX();
   void emitTLD();
   void emitTLD4();
   void emitTMML();
   void emitTXD();
   void emitTXQ();

   void emitSUHandle(const int);
   void emitSUTarget();
   void emitSUATOM();
   void emitSULD();
   void emitSUST();

   void emitAL2P();
   void emitBAR();
   void emitCCTL();
   void emitMEMBAR();
   void emitPIXLD();
   void emitPLOP3_LUT();
   void emitVOTE();
};

};
#endif
