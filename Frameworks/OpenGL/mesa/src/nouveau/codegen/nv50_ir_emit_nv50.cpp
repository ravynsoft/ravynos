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

#include "nv50_ir.h"
#include "nv50_ir_target_nv50.h"

namespace nv50_ir {

#define NV50_OP_ENC_LONG     0
#define NV50_OP_ENC_SHORT    1
#define NV50_OP_ENC_IMM      2
#define NV50_OP_ENC_LONG_ALT 3

class CodeEmitterNV50 : public CodeEmitter
{
public:
   CodeEmitterNV50(Program::Type, const TargetNV50 *);

   virtual bool emitInstruction(Instruction *);

   virtual uint32_t getMinEncodingSize(const Instruction *) const;

   virtual void prepareEmission(Function *);

private:
   Program::Type progType;

   const TargetNV50 *targNV50;

private:
   inline void defId(const ValueDef&, const int pos);
   inline void srcId(const ValueRef&, const int pos);
   inline void srcId(const ValueRef *, const int pos);

   inline void srcAddr16(const ValueRef&, bool adj, const int pos);
   inline void srcAddr8(const ValueRef&, const int pos);

   void emitFlagsRd(const Instruction *);
   void emitFlagsWr(const Instruction *);

   void emitCondCode(CondCode cc, DataType ty, int pos);

   inline void setARegBits(unsigned int);

   void setAReg16(const Instruction *, int s);
   void setImmediate(const Instruction *, int s);

   void setDst(const Value *);
   void setDst(const Instruction *, int d);
   void setSrcFileBits(const Instruction *, int enc);
   void setSrc(const Instruction *, unsigned int s, int slot);

   void emitForm_MAD(const Instruction *);
   void emitForm_ADD(const Instruction *);
   void emitForm_MUL(const Instruction *);
   void emitForm_IMM(const Instruction *);

   void emitLoadStoreSizeLG(DataType ty, int pos);
   void emitLoadStoreSizeCS(DataType ty);

   void roundMode_MAD(const Instruction *);
   void roundMode_CVT(RoundMode);

   void emitMNeg12(const Instruction *);

   void emitLOAD(const Instruction *);
   void emitSTORE(const Instruction *);
   void emitMOV(const Instruction *);
   void emitRDSV(const Instruction *);
   void emitNOP();
   void emitINTERP(const Instruction *);
   void emitPFETCH(const Instruction *);
   void emitOUT(const Instruction *);

   void emitUADD(const Instruction *);
   void emitAADD(const Instruction *);
   void emitFADD(const Instruction *);
   void emitDADD(const Instruction *);
   void emitIMUL(const Instruction *);
   void emitFMUL(const Instruction *);
   void emitDMUL(const Instruction *);
   void emitFMAD(const Instruction *);
   void emitDMAD(const Instruction *);
   void emitIMAD(const Instruction *);
   void emitISAD(const Instruction *);

   void emitMINMAX(const Instruction *);

   void emitPreOp(const Instruction *);
   void emitSFnOp(const Instruction *, uint8_t subOp);

   void emitShift(const Instruction *);
   void emitARL(const Instruction *, unsigned int shl);
   void emitLogicOp(const Instruction *);
   void emitNOT(const Instruction *);

   void emitCVT(const Instruction *);
   void emitSET(const Instruction *);

   void emitTEX(const TexInstruction *);
   void emitTXQ(const TexInstruction *);
   void emitTEXPREP(const TexInstruction *);

   void emitQUADOP(const Instruction *, uint8_t lane, uint8_t quOp);

   void emitFlow(const Instruction *, uint8_t flowOp);
   void emitPRERETEmu(const FlowInstruction *);
   void emitBAR(const Instruction *);

   void emitATOM(const Instruction *);
};

#define SDATA(a) ((a).rep()->reg.data)
#define DDATA(a) ((a).rep()->reg.data)

void CodeEmitterNV50::srcId(const ValueRef& src, const int pos)
{
   assert(src.get());
   code[pos / 32] |= SDATA(src).id << (pos % 32);
}

void CodeEmitterNV50::srcId(const ValueRef *src, const int pos)
{
   assert(src->get());
   code[pos / 32] |= SDATA(*src).id << (pos % 32);
}

void CodeEmitterNV50::srcAddr16(const ValueRef& src, bool adj, const int pos)
{
   assert(src.get());

   int32_t offset = SDATA(src).offset;

   assert(!adj || src.get()->reg.size <= 4);
   if (adj)
      offset /= src.get()->reg.size;

   assert(offset <= 0x7fff && offset >= (int32_t)-0x8000 && (pos % 32) <= 16);

   if (offset < 0)
      offset &= adj ? (0xffff >> (src.get()->reg.size >> 1)) : 0xffff;

   code[pos / 32] |= offset << (pos % 32);
}

void CodeEmitterNV50::srcAddr8(const ValueRef& src, const int pos)
{
   assert(src.get());

   uint32_t offset = SDATA(src).offset;

   assert((offset <= 0x1fc || offset == 0x3fc) && !(offset & 0x3));

   code[pos / 32] |= (offset >> 2) << (pos % 32);
}

void CodeEmitterNV50::defId(const ValueDef& def, const int pos)
{
   assert(def.get() && def.getFile() != FILE_SHADER_OUTPUT);

   code[pos / 32] |= DDATA(def).id << (pos % 32);
}

void
CodeEmitterNV50::roundMode_MAD(const Instruction *insn)
{
   switch (insn->rnd) {
   case ROUND_M: code[1] |= 1 << 22; break;
   case ROUND_P: code[1] |= 2 << 22; break;
   case ROUND_Z: code[1] |= 3 << 22; break;
   default:
      assert(insn->rnd == ROUND_N);
      break;
   }
}

void
CodeEmitterNV50::emitMNeg12(const Instruction *i)
{
   code[1] |= i->src(0).mod.neg() << 26;
   code[1] |= i->src(1).mod.neg() << 27;
}

void CodeEmitterNV50::emitCondCode(CondCode cc, DataType ty, int pos)
{
   uint8_t enc;

   assert(pos >= 32 || pos <= 27);

   switch (cc) {
   case CC_LT:  enc = 0x1; break;
   case CC_LTU: enc = 0x9; break;
   case CC_EQ:  enc = 0x2; break;
   case CC_EQU: enc = 0xa; break;
   case CC_LE:  enc = 0x3; break;
   case CC_LEU: enc = 0xb; break;
   case CC_GT:  enc = 0x4; break;
   case CC_GTU: enc = 0xc; break;
   case CC_NE:  enc = 0x5; break;
   case CC_NEU: enc = 0xd; break;
   case CC_GE:  enc = 0x6; break;
   case CC_GEU: enc = 0xe; break;
   case CC_TR:  enc = 0xf; break;
   case CC_FL:  enc = 0x0; break;

   case CC_O:  enc = 0x10; break;
   case CC_C:  enc = 0x11; break;
   case CC_A:  enc = 0x12; break;
   case CC_S:  enc = 0x13; break;
   case CC_NS: enc = 0x1c; break;
   case CC_NA: enc = 0x1d; break;
   case CC_NC: enc = 0x1e; break;
   case CC_NO: enc = 0x1f; break;

   default:
      enc = 0;
      assert(!"invalid condition code");
      break;
   }
   if (ty != TYPE_NONE && !isFloatType(ty))
      enc &= ~0x8; // unordered only exists for float types

   code[pos / 32] |= enc << (pos % 32);
}

void
CodeEmitterNV50::emitFlagsRd(const Instruction *i)
{
   int s = (i->flagsSrc >= 0) ? i->flagsSrc : i->predSrc;

   assert(!(code[1] & 0x00003f80));

   if (s >= 0) {
      assert(i->getSrc(s)->reg.file == FILE_FLAGS);
      emitCondCode(i->cc, TYPE_NONE, 32 + 7);
      srcId(i->src(s), 32 + 12);
   } else {
      code[1] |= 0x0780;
   }
}

void
CodeEmitterNV50::emitFlagsWr(const Instruction *i)
{
   assert(!(code[1] & 0x70));

   int flagsDef = i->flagsDef;

   // find flags definition and check that it is the last def
   if (flagsDef < 0) {
      for (int d = 0; i->defExists(d); ++d)
         if (i->def(d).getFile() == FILE_FLAGS)
            flagsDef = d;
      if (flagsDef >= 0 && false) // TODO: enforce use of flagsDef at some point
         WARN("Instruction::flagsDef was not set properly\n");
   }
   if (flagsDef == 0 && i->defExists(1))
      WARN("flags def should not be the primary definition\n");

   if (flagsDef >= 0)
      code[1] |= (DDATA(i->def(flagsDef)).id << 4) | 0x40;

}

void
CodeEmitterNV50::setARegBits(unsigned int u)
{
   code[0] |= (u & 3) << 26;
   code[1] |= (u & 4);
}

void
CodeEmitterNV50::setAReg16(const Instruction *i, int s)
{
   if (i->srcExists(s)) {
      s = i->src(s).indirect[0];
      if (s >= 0)
         setARegBits(SDATA(i->src(s)).id + 1);
   }
}

void
CodeEmitterNV50::setImmediate(const Instruction *i, int s)
{
   const ImmediateValue *imm = i->src(s).get()->asImm();
   assert(imm);

   uint32_t u = imm->reg.data.u32;

   if (i->src(s).mod & Modifier(NV50_IR_MOD_NOT))
      u = ~u;

   code[1] |= 3;
   code[0] |= (u & 0x3f) << 16;
   code[1] |= (u >> 6) << 2;
}

void
CodeEmitterNV50::setDst(const Value *dst)
{
   const Storage *reg = &dst->join->reg;

   assert(reg->file != FILE_ADDRESS);

   if (reg->data.id < 0 || reg->file == FILE_FLAGS) {
      code[0] |= (127 << 2) | 1;
      code[1] |= 8;
   } else {
      int id;
      if (reg->file == FILE_SHADER_OUTPUT) {
         code[1] |= 8;
         id = reg->data.offset / 4;
      } else {
         id = reg->data.id;
      }
      code[0] |= id << 2;
   }
}

void
CodeEmitterNV50::setDst(const Instruction *i, int d)
{
   if (i->defExists(d)) {
      setDst(i->getDef(d));
   } else
   if (!d) {
      code[0] |= 0x01fc; // bit bucket
      code[1] |= 0x0008;
   }
}

// 3 * 2 bits:
// 0: r
// 1: a/s
// 2: c
// 3: i
void
CodeEmitterNV50::setSrcFileBits(const Instruction *i, int enc)
{
   uint8_t mode = 0;

   for (unsigned int s = 0; s < Target::operationSrcNr[i->op]; ++s) {
      switch (i->src(s).getFile()) {
      case FILE_GPR:
         break;
      case FILE_MEMORY_SHARED:
      case FILE_SHADER_INPUT:
         mode |= 1 << (s * 2);
         break;
      case FILE_MEMORY_CONST:
         mode |= 2 << (s * 2);
         break;
      case FILE_IMMEDIATE:
         mode |= 3 << (s * 2);
         break;
      default:
         ERROR("invalid file on source %i: %u\n", s, i->src(s).getFile());
         assert(0);
         break;
      }
   }
   switch (mode) {
   case 0x00: // rrr
      break;
   case 0x01: // arr/grr
      if (progType == Program::TYPE_GEOMETRY && i->src(0).isIndirect(0)) {
         code[0] |= 0x01800000;
         if (enc == NV50_OP_ENC_LONG || enc == NV50_OP_ENC_LONG_ALT)
            code[1] |= 0x00200000;
      } else {
         if (enc == NV50_OP_ENC_SHORT)
            code[0] |= 0x01000000;
         else
            code[1] |= 0x00200000;
      }
      break;
   case 0x03: // irr
      assert(i->op == OP_MOV);
      return;
   case 0x0c: // rir
      break;
   case 0x0d: // gir
      assert(progType == Program::TYPE_GEOMETRY ||
             progType == Program::TYPE_COMPUTE);
      code[0] |= 0x01000000;
      if (progType == Program::TYPE_GEOMETRY && i->src(0).isIndirect(0)) {
         int reg = i->src(0).getIndirect(0)->rep()->reg.data.id;
         assert(reg < 3);
         code[0] |= (reg + 1) << 26;
      }
      break;
   case 0x08: // rcr
      code[0] |= (enc == NV50_OP_ENC_LONG_ALT) ? 0x01000000 : 0x00800000;
      code[1] |= (i->getSrc(1)->reg.fileIndex << 22);
      break;
   case 0x09: // acr/gcr
      if (progType == Program::TYPE_GEOMETRY && i->src(0).isIndirect(0)) {
         code[0] |= 0x01800000;
      } else {
         code[0] |= (enc == NV50_OP_ENC_LONG_ALT) ? 0x01000000 : 0x00800000;
         code[1] |= 0x00200000;
      }
      code[1] |= (i->getSrc(1)->reg.fileIndex << 22);
      break;
   case 0x20: // rrc
      code[0] |= 0x01000000;
      code[1] |= (i->getSrc(2)->reg.fileIndex << 22);
      break;
   case 0x21: // arc
      code[0] |= 0x01000000;
      code[1] |= 0x00200000 | (i->getSrc(2)->reg.fileIndex << 22);
      assert(progType != Program::TYPE_GEOMETRY);
      break;
   default:
      ERROR("not encodable: %x\n", mode);
      assert(0);
      break;
   }
   if (progType != Program::TYPE_COMPUTE)
      return;

   if ((mode & 3) == 1) {
      const int pos = ((mode >> 2) & 3) == 3 ? 13 : 14;

      switch (i->sType) {
      case TYPE_U8:
         break;
      case TYPE_U16:
         code[0] |= 1 << pos;
         break;
      case TYPE_S16:
         code[0] |= 2 << pos;
         break;
      default:
         code[0] |= 3 << pos;
         assert(i->getSrc(0)->reg.size == 4);
         break;
      }
   }
}

void
CodeEmitterNV50::setSrc(const Instruction *i, unsigned int s, int slot)
{
   if (Target::operationSrcNr[i->op] <= s)
      return;
   const Storage *reg = &i->src(s).rep()->reg;

   unsigned int id = (reg->file == FILE_GPR) ?
      reg->data.id :
      reg->data.offset >> (reg->size >> 1); // no > 4 byte sources here

   switch (slot) {
   case 0: code[0] |= id << 9; break;
   case 1: code[0] |= id << 16; break;
   case 2: code[1] |= id << 14; break;
   default:
      assert(0);
      break;
   }
}

// the default form:
//  - long instruction
//  - 1 to 3 sources in slots 0, 1, 2 (rrr, arr, rcr, acr, rrc, arc, gcr, grr)
//  - address & flags
void
CodeEmitterNV50::emitForm_MAD(const Instruction *i)
{
   assert(i->encSize == 8);
   code[0] |= 1;

   emitFlagsRd(i);
   emitFlagsWr(i);

   setDst(i, 0);

   setSrcFileBits(i, NV50_OP_ENC_LONG);
   setSrc(i, 0, 0);
   setSrc(i, 1, 1);
   setSrc(i, 2, 2);

   if (i->getIndirect(0, 0)) {
      assert(!i->srcExists(1) || !i->getIndirect(1, 0));
      assert(!i->srcExists(2) || !i->getIndirect(2, 0));
      setAReg16(i, 0);
   } else if (i->srcExists(1) && i->getIndirect(1, 0)) {
      assert(!i->srcExists(2) || !i->getIndirect(2, 0));
      setAReg16(i, 1);
   } else {
      setAReg16(i, 2);
   }
}

// like default form, but 2nd source in slot 2, and no 3rd source
void
CodeEmitterNV50::emitForm_ADD(const Instruction *i)
{
   assert(i->encSize == 8);
   code[0] |= 1;

   emitFlagsRd(i);
   emitFlagsWr(i);

   setDst(i, 0);

   setSrcFileBits(i, NV50_OP_ENC_LONG_ALT);
   setSrc(i, 0, 0);
   if (i->predSrc != 1)
      setSrc(i, 1, 2);

   if (i->getIndirect(0, 0)) {
      assert(!i->getIndirect(1, 0));
      setAReg16(i, 0);
   } else {
      setAReg16(i, 1);
   }
}

// default short form (rr, ar, rc, gr)
void
CodeEmitterNV50::emitForm_MUL(const Instruction *i)
{
   assert(i->encSize == 4 && !(code[0] & 1));
   assert(i->defExists(0));
   assert(!i->getPredicate());

   setDst(i, 0);

   setSrcFileBits(i, NV50_OP_ENC_SHORT);
   setSrc(i, 0, 0);
   setSrc(i, 1, 1);
}

// usual immediate form
// - 1 to 3 sources where second is immediate (rir, gir)
// - no address or predicate possible
void
CodeEmitterNV50::emitForm_IMM(const Instruction *i)
{
   assert(i->encSize == 8);
   code[0] |= 1;

   assert(i->defExists(0) && i->srcExists(0));

   setDst(i, 0);

   setSrcFileBits(i, NV50_OP_ENC_IMM);
   if (Target::operationSrcNr[i->op] > 1) {
      setSrc(i, 0, 0);
      setImmediate(i, 1);
      // If there is another source, it has to be the same as the dest reg.
   } else {
      setImmediate(i, 0);
   }
}

void
CodeEmitterNV50::emitLoadStoreSizeLG(DataType ty, int pos)
{
   uint8_t enc;

   switch (ty) {
   case TYPE_F32: // fall through
   case TYPE_S32: // fall through
   case TYPE_U32:  enc = 0x6; break;
   case TYPE_B128: enc = 0x5; break;
   case TYPE_F64: // fall through
   case TYPE_S64: // fall through
   case TYPE_U64:  enc = 0x4; break;
   case TYPE_S16:  enc = 0x3; break;
   case TYPE_U16:  enc = 0x2; break;
   case TYPE_S8:   enc = 0x1; break;
   case TYPE_U8:   enc = 0x0; break;
   default:
      enc = 0;
      assert(!"invalid load/store type");
      break;
   }
   code[pos / 32] |= enc << (pos % 32);
}

void
CodeEmitterNV50::emitLoadStoreSizeCS(DataType ty)
{
   switch (ty) {
   case TYPE_U8: break;
   case TYPE_U16: code[1] |= 0x4000; break;
   case TYPE_S16: code[1] |= 0x8000; break;
   case TYPE_F32:
   case TYPE_S32:
   case TYPE_U32: code[1] |= 0xc000; break;
   default:
      assert(0);
      break;
   }
}

void
CodeEmitterNV50::emitLOAD(const Instruction *i)
{
   DataFile sf = i->src(0).getFile();
   ASSERTED int32_t offset = i->getSrc(0)->reg.data.offset;

   switch (sf) {
   case FILE_SHADER_INPUT:
      if (progType == Program::TYPE_GEOMETRY && i->src(0).isIndirect(0))
         code[0] = 0x11800001;
      else
         // use 'mov' where we can
         code[0] = i->src(0).isIndirect(0) ? 0x00000001 : 0x10000001;
      code[1] = 0x00200000 | (i->lanes << 14);
      if (typeSizeof(i->dType) == 4)
         code[1] |= 0x04000000;
      break;
   case FILE_MEMORY_SHARED:
      if (targ->getChipset() >= 0x84) {
         assert(offset <= (int32_t)(0x3fff * typeSizeof(i->sType)));
         code[0] = 0x10000001;
         code[1] = 0x40000000;

         if (typeSizeof(i->dType) == 4)
            code[1] |= 0x04000000;

         emitLoadStoreSizeCS(i->sType);

         if (i->subOp == NV50_IR_SUBOP_LOAD_LOCKED)
            code[1] |= 0x00800000;
      } else {
         assert(offset <= (int32_t)(0x1f * typeSizeof(i->sType)));
         code[0] = 0x10000001;
         code[1] = 0x00200000 | (i->lanes << 14);
         emitLoadStoreSizeCS(i->sType);
      }
      break;
   case FILE_MEMORY_CONST:
      code[0] = 0x10000001;
      code[1] = 0x20000000 | (i->getSrc(0)->reg.fileIndex << 22);
      if (typeSizeof(i->dType) == 4)
         code[1] |= 0x04000000;
      emitLoadStoreSizeCS(i->sType);
      break;
   case FILE_MEMORY_LOCAL:
      code[0] = 0xd0000001;
      code[1] = 0x40000000;
      break;
   case FILE_MEMORY_GLOBAL:
      code[0] = 0xd0000001 | (i->getSrc(0)->reg.fileIndex << 16);
      code[1] = 0x80000000;
      break;
   default:
      assert(!"invalid load source file");
      break;
   }
   if (sf == FILE_MEMORY_LOCAL ||
       sf == FILE_MEMORY_GLOBAL)
      emitLoadStoreSizeLG(i->sType, 21 + 32);

   setDst(i, 0);

   emitFlagsRd(i);
   emitFlagsWr(i);

   if (i->src(0).getFile() == FILE_MEMORY_GLOBAL) {
      srcId(*i->src(0).getIndirect(0), 9);
   } else {
      setAReg16(i, 0);
      srcAddr16(i->src(0), i->src(0).getFile() != FILE_MEMORY_LOCAL, 9);
   }
}

void
CodeEmitterNV50::emitSTORE(const Instruction *i)
{
   DataFile f = i->getSrc(0)->reg.file;
   int32_t offset = i->getSrc(0)->reg.data.offset;

   switch (f) {
   case FILE_SHADER_OUTPUT:
      code[0] = 0x00000001 | ((offset >> 2) << 9);
      code[1] = 0x80c00000;
      srcId(i->src(1), 32 + 14);
      break;
   case FILE_MEMORY_GLOBAL:
      code[0] = 0xd0000001 | (i->getSrc(0)->reg.fileIndex << 16);
      code[1] = 0xa0000000;
      emitLoadStoreSizeLG(i->dType, 21 + 32);
      srcId(i->src(1), 2);
      break;
   case FILE_MEMORY_LOCAL:
      code[0] = 0xd0000001;
      code[1] = 0x60000000;
      emitLoadStoreSizeLG(i->dType, 21 + 32);
      srcId(i->src(1), 2);
      break;
   case FILE_MEMORY_SHARED:
      code[0] = 0x00000001;
      code[1] = 0xe0000000;
      if (i->subOp == NV50_IR_SUBOP_STORE_UNLOCKED)
         code[1] |= 0x00800000;
      switch (typeSizeof(i->dType)) {
      case 1:
         code[0] |= offset << 9;
         code[1] |= 0x00400000;
         break;
      case 2:
         code[0] |= (offset >> 1) << 9;
         break;
      case 4:
         code[0] |= (offset >> 2) << 9;
         code[1] |= 0x04200000;
         break;
      default:
         assert(0);
         break;
      }
      srcId(i->src(1), 32 + 14);
      break;
   default:
      assert(!"invalid store destination file");
      break;
   }

   if (f == FILE_MEMORY_GLOBAL)
      srcId(*i->src(0).getIndirect(0), 9);
   else
      setAReg16(i, 0);

   if (f == FILE_MEMORY_LOCAL)
      srcAddr16(i->src(0), false, 9);

   emitFlagsRd(i);
}

void
CodeEmitterNV50::emitMOV(const Instruction *i)
{
   DataFile sf = i->getSrc(0)->reg.file;
   DataFile df = i->getDef(0)->reg.file;

   assert(sf == FILE_GPR || df == FILE_GPR);

   if (sf == FILE_FLAGS) {
      assert(i->flagsSrc >= 0);
      code[0] = 0x00000001;
      code[1] = 0x20000000;
      defId(i->def(0), 2);
      emitFlagsRd(i);
   } else
   if (sf == FILE_ADDRESS) {
      code[0] = 0x00000001;
      code[1] = 0x40000000;
      defId(i->def(0), 2);
      setARegBits(SDATA(i->src(0)).id + 1);
      emitFlagsRd(i);
   } else
   if (df == FILE_FLAGS) {
      assert(i->flagsDef >= 0);
      code[0] = 0x00000001;
      code[1] = 0xa0000000;
      srcId(i->src(0), 9);
      emitFlagsRd(i);
      emitFlagsWr(i);
   } else
   if (sf == FILE_IMMEDIATE) {
      code[0] = 0x10000001;
      code[1] = 0x00000003;
      emitForm_IMM(i);

      code[0] |= (typeSizeof(i->dType) == 2) ? 0 : 0x00008000;
   } else {
      if (i->encSize == 4) {
         code[0] = 0x10000000;
         code[0] |= (typeSizeof(i->dType) == 2) ? 0 : 0x00008000;
         defId(i->def(0), 2);
      } else {
         code[0] = 0x10000001;
         code[1] = (typeSizeof(i->dType) == 2) ? 0 : 0x04000000;
         code[1] |= (i->lanes << 14);
         setDst(i, 0);
         emitFlagsRd(i);
      }
      srcId(i->src(0), 9);
   }
   if (df == FILE_SHADER_OUTPUT) {
      assert(i->encSize == 8);
      code[1] |= 0x8;
   }
}

static inline uint8_t getSRegEncoding(const ValueRef &ref)
{
   switch (SDATA(ref).sv.sv) {
   case SV_PHYSID:        return 0;
   case SV_CLOCK:         return 1;
   case SV_VERTEX_STRIDE: return 3;
// case SV_PM_COUNTER:    return 4 + SDATA(ref).sv.index;
   case SV_SAMPLE_INDEX:  return 8;
   default:
      assert(!"no sreg for system value");
      return 0;
   }
}

void
CodeEmitterNV50::emitRDSV(const Instruction *i)
{
   code[0] = 0x00000001;
   code[1] = 0x60000000 | (getSRegEncoding(i->src(0)) << 14);
   defId(i->def(0), 2);
   emitFlagsRd(i);
}

void
CodeEmitterNV50::emitNOP()
{
   code[0] = 0xf0000001;
   code[1] = 0xe0000000;
}

void
CodeEmitterNV50::emitQUADOP(const Instruction *i, uint8_t lane, uint8_t quOp)
{
   code[0] = 0xc0000000 | (lane << 16);
   code[1] = 0x80000000;

   code[0] |= (quOp & 0x03) << 20;
   code[1] |= (quOp & 0xfc) << 20;

   emitForm_ADD(i);

   if (!i->srcExists(1) || i->predSrc == 1)
      srcId(i->src(0), 32 + 14);
}

/* NOTE: This returns the base address of a vertex inside the primitive.
 * src0 is an immediate, the index (not offset) of the vertex
 * inside the primitive. XXX: signed or unsigned ?
 * src1 (may be NULL) should use whatever units the hardware requires
 * (on nv50 this is bytes, so, relative index * 4; signed 16 bit value).
 */
void
CodeEmitterNV50::emitPFETCH(const Instruction *i)
{
   const uint32_t prim = i->src(0).get()->reg.data.u32;
   assert(prim <= 127);

   if (i->def(0).getFile() == FILE_ADDRESS) {
      // shl $aX a[] 0
      code[0] = 0x00000001 | ((DDATA(i->def(0)).id + 1) << 2);
      code[1] = 0xc0200000;
      code[0] |= prim << 9;
      assert(!i->srcExists(1));
   } else
   if (i->srcExists(1)) {
      // ld b32 $rX a[$aX+base]
      code[0] = 0x00000001;
      code[1] = 0x04200000 | (0xf << 14);
      defId(i->def(0), 2);
      code[0] |= prim << 9;
      setARegBits(SDATA(i->src(1)).id + 1);
   } else {
      // mov b32 $rX a[]
      code[0] = 0x10000001;
      code[1] = 0x04200000 | (0xf << 14);
      defId(i->def(0), 2);
      code[0] |= prim << 9;
   }
   emitFlagsRd(i);
}

void
nv50_interpApply(const FixupEntry *entry, uint32_t *code, const FixupData& data)
{
   int ipa = entry->ipa;
   int encSize = entry->reg;
   int loc = entry->loc;

   if ((ipa & NV50_IR_INTERP_SAMPLE_MASK) == NV50_IR_INTERP_DEFAULT &&
       (ipa & NV50_IR_INTERP_MODE_MASK) != NV50_IR_INTERP_FLAT) {
      if (data.force_persample_interp) {
         if (encSize == 8)
            code[loc + 1] |= 1 << 16;
         else
            code[loc + 0] |= 1 << 24;
      } else {
         if (encSize == 8)
            code[loc + 1] &= ~(1 << 16);
         else
            code[loc + 0] &= ~(1 << 24);
      }
   }
}

void
CodeEmitterNV50::emitINTERP(const Instruction *i)
{
   code[0] = 0x80000000;

   defId(i->def(0), 2);
   srcAddr8(i->src(0), 16);
   setAReg16(i, 0);

   if (i->encSize != 8 && i->getInterpMode() == NV50_IR_INTERP_FLAT) {
      code[0] |= 1 << 8;
   } else {
      if (i->op == OP_PINTERP) {
         code[0] |= 1 << 25;
         srcId(i->src(1), 9);
      }
      if (i->getSampleMode() == NV50_IR_INTERP_CENTROID)
         code[0] |= 1 << 24;
   }

   if (i->encSize == 8) {
      if (i->getInterpMode() == NV50_IR_INTERP_FLAT)
         code[1] = 4 << 16;
      else
         code[1] = (code[0] & (3 << 24)) >> (24 - 16);
      code[0] &= ~0x03000000;
      code[0] |= 1;
      emitFlagsRd(i);
   }

   addInterp(i->ipa, i->encSize, nv50_interpApply);
}

void
CodeEmitterNV50::emitMINMAX(const Instruction *i)
{
   if (i->dType == TYPE_F64) {
      code[0] = 0xe0000000;
      code[1] = (i->op == OP_MIN) ? 0xa0000000 : 0xc0000000;
   } else {
      code[0] = 0x30000000;
      code[1] = 0x80000000;
      if (i->op == OP_MIN)
         code[1] |= 0x20000000;

      switch (i->dType) {
      case TYPE_F32: code[0] |= 0x80000000; break;
      case TYPE_S32: code[1] |= 0x8c000000; break;
      case TYPE_U32: code[1] |= 0x84000000; break;
      case TYPE_S16: code[1] |= 0x80000000; break;
      case TYPE_U16: break;
      default:
         assert(0);
         break;
      }
   }

   code[1] |= i->src(0).mod.abs() << 20;
   code[1] |= i->src(0).mod.neg() << 26;
   code[1] |= i->src(1).mod.abs() << 19;
   code[1] |= i->src(1).mod.neg() << 27;

   emitForm_MAD(i);
}

void
CodeEmitterNV50::emitFMAD(const Instruction *i)
{
   const int neg_mul = i->src(0).mod.neg() ^ i->src(1).mod.neg();
   const int neg_add = i->src(2).mod.neg();

   code[0] = 0xe0000000;

   if (i->src(1).getFile() == FILE_IMMEDIATE) {
      code[1] = 0;
      emitForm_IMM(i);
      code[0] |= neg_mul << 15;
      code[0] |= neg_add << 22;
      if (i->saturate)
         code[0] |= 1 << 8;
   } else
   if (i->encSize == 4) {
      emitForm_MUL(i);
      code[0] |= neg_mul << 15;
      code[0] |= neg_add << 22;
      if (i->saturate)
         code[0] |= 1 << 8;
   } else {
      code[1]  = neg_mul << 26;
      code[1] |= neg_add << 27;
      if (i->saturate)
         code[1] |= 1 << 29;
      emitForm_MAD(i);
   }
}

void
CodeEmitterNV50::emitDMAD(const Instruction *i)
{
   const int neg_mul = i->src(0).mod.neg() ^ i->src(1).mod.neg();
   const int neg_add = i->src(2).mod.neg();

   assert(i->encSize == 8);
   assert(!i->saturate);

   code[1] = 0x40000000;
   code[0] = 0xe0000000;

   code[1] |= neg_mul << 26;
   code[1] |= neg_add << 27;

   roundMode_MAD(i);

   emitForm_MAD(i);
}

void
CodeEmitterNV50::emitFADD(const Instruction *i)
{
   const int neg0 = i->src(0).mod.neg();
   const int neg1 = i->src(1).mod.neg() ^ ((i->op == OP_SUB) ? 1 : 0);

   code[0] = 0xb0000000;

   assert(!(i->src(0).mod | i->src(1).mod).abs());

   if (i->src(1).getFile() == FILE_IMMEDIATE) {
      code[1] = 0;
      emitForm_IMM(i);
      code[0] |= neg0 << 15;
      code[0] |= neg1 << 22;
      if (i->saturate)
         code[0] |= 1 << 8;
   } else
   if (i->encSize == 8) {
      code[1] = 0;
      emitForm_ADD(i);
      code[1] |= neg0 << 26;
      code[1] |= neg1 << 27;
      if (i->saturate)
         code[1] |= 1 << 29;
   } else {
      emitForm_MUL(i);
      code[0] |= neg0 << 15;
      code[0] |= neg1 << 22;
      if (i->saturate)
         code[0] |= 1 << 8;
   }
}

void
CodeEmitterNV50::emitDADD(const Instruction *i)
{
   const int neg0 = i->src(0).mod.neg();
   const int neg1 = i->src(1).mod.neg() ^ ((i->op == OP_SUB) ? 1 : 0);

   assert(!(i->src(0).mod | i->src(1).mod).abs());
   assert(!i->saturate);
   assert(i->encSize == 8);

   code[1] = 0x60000000;
   code[0] = 0xe0000000;

   emitForm_ADD(i);

   code[1] |= neg0 << 26;
   code[1] |= neg1 << 27;
}

void
CodeEmitterNV50::emitUADD(const Instruction *i)
{
   const int neg0 = i->src(0).mod.neg();
   const int neg1 = i->src(1).mod.neg() ^ ((i->op == OP_SUB) ? 1 : 0);

   code[0] = 0x20000000;

   if (i->src(1).getFile() == FILE_IMMEDIATE) {
      code[0] |= (typeSizeof(i->dType) == 2) ? 0 : 0x00008000;
      code[1] = 0;
      emitForm_IMM(i);
   } else
   if (i->encSize == 8) {
      code[1] = (typeSizeof(i->dType) == 2) ? 0 : 0x04000000;
      emitForm_ADD(i);
   } else {
      code[0] |= (typeSizeof(i->dType) == 2) ? 0 : 0x00008000;
      emitForm_MUL(i);
   }
   assert(!(neg0 && neg1));
   code[0] |= neg0 << 28;
   code[0] |= neg1 << 22;

   if (i->flagsSrc >= 0) {
      // addc == sub | subr
      assert(!(code[0] & 0x10400000) && !i->getPredicate());
      code[0] |= 0x10400000;
      srcId(i->src(i->flagsSrc), 32 + 12);
   }
}

void
CodeEmitterNV50::emitAADD(const Instruction *i)
{
   const int s = (i->op == OP_MOV) ? 0 : 1;

   code[0] = 0xd0000001 | (i->getSrc(s)->reg.data.u16 << 9);
   code[1] = 0x20000000;

   code[0] |= (DDATA(i->def(0)).id + 1) << 2;

   emitFlagsRd(i);

   if (s && i->srcExists(0))
      setARegBits(SDATA(i->src(0)).id + 1);
}

void
CodeEmitterNV50::emitIMUL(const Instruction *i)
{
   code[0] = 0x40000000;

   if (i->src(1).getFile() == FILE_IMMEDIATE) {
      if (i->sType == TYPE_S16)
         code[0] |= 0x8100;
      code[1] = 0;
      emitForm_IMM(i);
   } else
   if (i->encSize == 8) {
      code[1] = (i->sType == TYPE_S16) ? (0x8000 | 0x4000) : 0x0000;
      emitForm_MAD(i);
   } else {
      if (i->sType == TYPE_S16)
         code[0] |= 0x8100;
      emitForm_MUL(i);
   }
}

void
CodeEmitterNV50::emitFMUL(const Instruction *i)
{
   const int neg = (i->src(0).mod ^ i->src(1).mod).neg();

   code[0] = 0xc0000000;

   if (i->src(1).getFile() == FILE_IMMEDIATE) {
      code[1] = 0;
      emitForm_IMM(i);
      if (neg)
         code[0] |= 0x8000;
      if (i->saturate)
         code[0] |= 1 << 8;
   } else
   if (i->encSize == 8) {
      code[1] = i->rnd == ROUND_Z ? 0x0000c000 : 0;
      if (neg)
         code[1] |= 0x08000000;
      if (i->saturate)
         code[1] |= 1 << 20;
      emitForm_MAD(i);
   } else {
      emitForm_MUL(i);
      if (neg)
         code[0] |= 0x8000;
      if (i->saturate)
         code[0] |= 1 << 8;
   }
}

void
CodeEmitterNV50::emitDMUL(const Instruction *i)
{
   const int neg = (i->src(0).mod ^ i->src(1).mod).neg();

   assert(!i->saturate);
   assert(i->encSize == 8);

   code[1] = 0x80000000;
   code[0] = 0xe0000000;

   if (neg)
      code[1] |= 0x08000000;

   roundMode_CVT(i->rnd);

   emitForm_MAD(i);
}

void
CodeEmitterNV50::emitIMAD(const Instruction *i)
{
   int mode;
   code[0] = 0x60000000;

   assert(!i->src(0).mod && !i->src(1).mod && !i->src(2).mod);
   if (!isSignedType(i->sType))
      mode = 0;
   else if (i->saturate)
      mode = 2;
   else
      mode = 1;

   if (i->src(1).getFile() == FILE_IMMEDIATE) {
      code[1] = 0;
      emitForm_IMM(i);
      code[0] |= (mode & 1) << 8 | (mode & 2) << 14;
      if (i->flagsSrc >= 0) {
         assert(!(code[0] & 0x10400000));
         assert(SDATA(i->src(i->flagsSrc)).id == 0);
         code[0] |= 0x10400000;
      }
   } else
   if (i->encSize == 4) {
      emitForm_MUL(i);
      code[0] |= (mode & 1) << 8 | (mode & 2) << 14;
      if (i->flagsSrc >= 0) {
         assert(!(code[0] & 0x10400000));
         assert(SDATA(i->src(i->flagsSrc)).id == 0);
         code[0] |= 0x10400000;
      }
   } else {
      code[1] = mode << 29;
      emitForm_MAD(i);

      if (i->flagsSrc >= 0) {
         // add with carry from $cX
         assert(!(code[1] & 0x0c000000) && !i->getPredicate());
         code[1] |= 0xc << 24;
         srcId(i->src(i->flagsSrc), 32 + 12);
      }
   }
}

void
CodeEmitterNV50::emitISAD(const Instruction *i)
{
   if (i->encSize == 8) {
      code[0] = 0x50000000;
      switch (i->sType) {
      case TYPE_U32: code[1] = 0x04000000; break;
      case TYPE_S32: code[1] = 0x0c000000; break;
      case TYPE_U16: code[1] = 0x00000000; break;
      case TYPE_S16: code[1] = 0x08000000; break;
      default:
         assert(0);
         break;
      }
      emitForm_MAD(i);
   } else {
      switch (i->sType) {
      case TYPE_U32: code[0] = 0x50008000; break;
      case TYPE_S32: code[0] = 0x50008100; break;
      case TYPE_U16: code[0] = 0x50000000; break;
      case TYPE_S16: code[0] = 0x50000100; break;
      default:
         assert(0);
         break;
      }
      emitForm_MUL(i);
   }
}

static void
alphatestSet(const FixupEntry *entry, uint32_t *code, const FixupData& data)
{
   int loc = entry->loc;
   int enc;

   switch (data.alphatest) {
   case PIPE_FUNC_NEVER: enc = 0x0; break;
   case PIPE_FUNC_LESS: enc = 0x1; break;
   case PIPE_FUNC_EQUAL: enc = 0x2; break;
   case PIPE_FUNC_LEQUAL: enc = 0x3; break;
   case PIPE_FUNC_GREATER: enc = 0x4; break;
   case PIPE_FUNC_NOTEQUAL: enc = 0x5; break;
   case PIPE_FUNC_GEQUAL: enc = 0x6; break;
   default:
   case PIPE_FUNC_ALWAYS: enc = 0xf; break;
   }

   code[loc + 1] &= ~(0x1f << 14);
   code[loc + 1] |= enc << 14;
}

void
CodeEmitterNV50::emitSET(const Instruction *i)
{
   code[0] = 0x30000000;
   code[1] = 0x60000000;

   switch (i->sType) {
   case TYPE_F64:
      code[0] = 0xe0000000;
      code[1] = 0xe0000000;
      break;
   case TYPE_F32: code[0] |= 0x80000000; break;
   case TYPE_S32: code[1] |= 0x0c000000; break;
   case TYPE_U32: code[1] |= 0x04000000; break;
   case TYPE_S16: code[1] |= 0x08000000; break;
   case TYPE_U16: break;
   default:
      assert(0);
      break;
   }

   emitCondCode(i->asCmp()->setCond, i->sType, 32 + 14);

   if (i->src(0).mod.neg()) code[1] |= 0x04000000;
   if (i->src(1).mod.neg()) code[1] |= 0x08000000;
   if (i->src(0).mod.abs()) code[1] |= 0x00100000;
   if (i->src(1).mod.abs()) code[1] |= 0x00080000;

   emitForm_MAD(i);

   if (i->subOp == 1) {
      addInterp(0, 0, alphatestSet);
   }
}

void
CodeEmitterNV50::roundMode_CVT(RoundMode rnd)
{
   switch (rnd) {
   case ROUND_NI: code[1] |= 0x08000000; break;
   case ROUND_M:  code[1] |= 0x00020000; break;
   case ROUND_MI: code[1] |= 0x08020000; break;
   case ROUND_P:  code[1] |= 0x00040000; break;
   case ROUND_PI: code[1] |= 0x08040000; break;
   case ROUND_Z:  code[1] |= 0x00060000; break;
   case ROUND_ZI: code[1] |= 0x08060000; break;
   default:
      assert(rnd == ROUND_N);
      break;
   }
}

void
CodeEmitterNV50::emitCVT(const Instruction *i)
{
   const bool f2f = isFloatType(i->dType) && isFloatType(i->sType);
   RoundMode rnd;
   DataType dType;

   switch (i->op) {
   case OP_CEIL:  rnd = f2f ? ROUND_PI : ROUND_P; break;
   case OP_FLOOR: rnd = f2f ? ROUND_MI : ROUND_M; break;
   case OP_TRUNC: rnd = f2f ? ROUND_ZI : ROUND_Z; break;
   default:
      rnd = i->rnd;
      break;
   }

   if (i->op == OP_NEG && i->dType == TYPE_U32)
      dType = TYPE_S32;
   else
      dType = i->dType;

   code[0] = 0xa0000000;

   switch (dType) {
   case TYPE_F64:
      switch (i->sType) {
      case TYPE_F64: code[1] = 0xc4404000; break;
      case TYPE_S64: code[1] = 0x44414000; break;
      case TYPE_U64: code[1] = 0x44404000; break;
      case TYPE_F32: code[1] = 0xc4400000; break;
      case TYPE_S32: code[1] = 0x44410000; break;
      case TYPE_U32: code[1] = 0x44400000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_S64:
      switch (i->sType) {
      case TYPE_F64: code[1] = 0x8c404000; break;
      case TYPE_F32: code[1] = 0x8c400000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_U64:
      switch (i->sType) {
      case TYPE_F64: code[1] = 0x84404000; break;
      case TYPE_F32: code[1] = 0x84400000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_F32:
      switch (i->sType) {
      case TYPE_F64: code[1] = 0xc0404000; break;
      case TYPE_S64: code[1] = 0x40414000; break;
      case TYPE_U64: code[1] = 0x40404000; break;
      case TYPE_F32: code[1] = 0xc4004000; break;
      case TYPE_S32: code[1] = 0x44014000; break;
      case TYPE_U32: code[1] = 0x44004000; break;
      case TYPE_F16: code[1] = 0xc4000000; break;
      case TYPE_U16: code[1] = 0x44000000; break;
      case TYPE_S16: code[1] = 0x44010000; break;
      case TYPE_S8:  code[1] = 0x44018000; break;
      case TYPE_U8:  code[1] = 0x44008000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_S32:
      switch (i->sType) {
      case TYPE_F64: code[1] = 0x88404000; break;
      case TYPE_F32: code[1] = 0x8c004000; break;
      case TYPE_S32: code[1] = 0x0c014000; break;
      case TYPE_U32: code[1] = 0x0c004000; break;
      case TYPE_F16: code[1] = 0x8c000000; break;
      case TYPE_S16: code[1] = 0x0c010000; break;
      case TYPE_U16: code[1] = 0x0c000000; break;
      case TYPE_S8:  code[1] = 0x0c018000; break;
      case TYPE_U8:  code[1] = 0x0c008000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_U32:
      switch (i->sType) {
      case TYPE_F64: code[1] = 0x80404000; break;
      case TYPE_F32: code[1] = 0x84004000; break;
      case TYPE_S32: code[1] = 0x04014000; break;
      case TYPE_U32: code[1] = 0x04004000; break;
      case TYPE_F16: code[1] = 0x84000000; break;
      case TYPE_S16: code[1] = 0x04010000; break;
      case TYPE_U16: code[1] = 0x04000000; break;
      case TYPE_S8:  code[1] = 0x04018000; break;
      case TYPE_U8:  code[1] = 0x04008000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_F16:
      switch (i->sType) {
      case TYPE_F16: code[1] = 0xc0000000; break;
      case TYPE_F32: code[1] = 0xc0004000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_S16:
      switch (i->sType) {
      case TYPE_F32: code[1] = 0x88004000; break;
      case TYPE_S32: code[1] = 0x08014000; break;
      case TYPE_U32: code[1] = 0x08004000; break;
      case TYPE_F16: code[1] = 0x88000000; break;
      case TYPE_S16: code[1] = 0x08010000; break;
      case TYPE_U16: code[1] = 0x08000000; break;
      case TYPE_S8:  code[1] = 0x08018000; break;
      case TYPE_U8:  code[1] = 0x08008000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_U16:
      switch (i->sType) {
      case TYPE_F32: code[1] = 0x80004000; break;
      case TYPE_S32: code[1] = 0x00014000; break;
      case TYPE_U32: code[1] = 0x00004000; break;
      case TYPE_F16: code[1] = 0x80000000; break;
      case TYPE_S16: code[1] = 0x00010000; break;
      case TYPE_U16: code[1] = 0x00000000; break;
      case TYPE_S8:  code[1] = 0x00018000; break;
      case TYPE_U8:  code[1] = 0x00008000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_S8:
      switch (i->sType) {
      case TYPE_S32: code[1] = 0x08094000; break;
      case TYPE_U32: code[1] = 0x08084000; break;
      case TYPE_F16: code[1] = 0x88080000; break;
      case TYPE_S16: code[1] = 0x08090000; break;
      case TYPE_U16: code[1] = 0x08080000; break;
      case TYPE_S8:  code[1] = 0x08098000; break;
      case TYPE_U8:  code[1] = 0x08088000; break;
      default:
         assert(0);
         break;
      }
      break;
   case TYPE_U8:
      switch (i->sType) {
      case TYPE_S32: code[1] = 0x00094000; break;
      case TYPE_U32: code[1] = 0x00084000; break;
      case TYPE_F16: code[1] = 0x80080000; break;
      case TYPE_S16: code[1] = 0x00090000; break;
      case TYPE_U16: code[1] = 0x00080000; break;
      case TYPE_S8:  code[1] = 0x00098000; break;
      case TYPE_U8:  code[1] = 0x00088000; break;
      default:
         assert(0);
         break;
      }
      break;
   default:
      assert(0);
      break;
   }
   if (typeSizeof(i->sType) == 1 && i->getSrc(0)->reg.size == 4)
      code[1] |= 0x00004000;

   roundMode_CVT(rnd);

   switch (i->op) {
   case OP_ABS: code[1] |= 1 << 20; break;
   case OP_SAT: code[1] |= 1 << 19; break;
   case OP_NEG: code[1] |= 1 << 29; break;
   default:
      break;
   }
   code[1] ^= i->src(0).mod.neg() << 29;
   code[1] |= i->src(0).mod.abs() << 20;
   if (i->saturate)
      code[1] |= 1 << 19;

   assert(i->op != OP_ABS || !i->src(0).mod.neg());

   emitForm_MAD(i);
}

void
CodeEmitterNV50::emitPreOp(const Instruction *i)
{
   code[0] = 0xb0000000;
   code[1] = (i->op == OP_PREEX2) ? 0xc0004000 : 0xc0000000;

   code[1] |= i->src(0).mod.abs() << 20;
   code[1] |= i->src(0).mod.neg() << 26;

   emitForm_MAD(i);
}

void
CodeEmitterNV50::emitSFnOp(const Instruction *i, uint8_t subOp)
{
   code[0] = 0x90000000;

   if (i->encSize == 4) {
      assert(i->op == OP_RCP);
      assert(!i->saturate);
      code[0] |= i->src(0).mod.abs() << 15;
      code[0] |= i->src(0).mod.neg() << 22;
      emitForm_MUL(i);
   } else {
      code[1] = subOp << 29;
      code[1] |= i->src(0).mod.abs() << 20;
      code[1] |= i->src(0).mod.neg() << 26;
      if (i->saturate) {
         assert(subOp == 6 && i->op == OP_EX2);
         code[1] |= 1 << 27;
      }
      emitForm_MAD(i);
   }
}

void
CodeEmitterNV50::emitNOT(const Instruction *i)
{
   code[0] = 0xd0000000;
   code[1] = 0x0002c000;

   switch (i->sType) {
   case TYPE_U32:
   case TYPE_S32:
      code[1] |= 0x04000000;
      break;
   default:
      break;
   }
   emitForm_MAD(i);
   setSrc(i, 0, 1);
}

void
CodeEmitterNV50::emitLogicOp(const Instruction *i)
{
   code[0] = 0xd0000000;
   code[1] = 0;

   if (i->src(1).getFile() == FILE_IMMEDIATE) {
      switch (i->op) {
      case OP_OR:  code[0] |= 0x0100; break;
      case OP_XOR: code[0] |= 0x8000; break;
      default:
         assert(i->op == OP_AND);
         break;
      }
      if (i->src(0).mod & Modifier(NV50_IR_MOD_NOT))
         code[0] |= 1 << 22;

      emitForm_IMM(i);
   } else {
      switch (i->op) {
      case OP_AND: code[1] = 0x00000000; break;
      case OP_OR:  code[1] = 0x00004000; break;
      case OP_XOR: code[1] = 0x00008000; break;
      default:
         assert(0);
         break;
      }
      if (typeSizeof(i->dType) == 4)
         code[1] |= 0x04000000;
      if (i->src(0).mod & Modifier(NV50_IR_MOD_NOT))
         code[1] |= 1 << 16;
      if (i->src(1).mod & Modifier(NV50_IR_MOD_NOT))
         code[1] |= 1 << 17;

      emitForm_MAD(i);
   }
}

void
CodeEmitterNV50::emitARL(const Instruction *i, unsigned int shl)
{
   code[0] = 0x00000001 | (shl << 16);
   code[1] = 0xc0000000;

   code[0] |= (DDATA(i->def(0)).id + 1) << 2;

   setSrcFileBits(i, NV50_OP_ENC_IMM);
   setSrc(i, 0, 0);
   emitFlagsRd(i);
}

void
CodeEmitterNV50::emitShift(const Instruction *i)
{
   if (i->def(0).getFile() == FILE_ADDRESS) {
      assert(i->srcExists(1) && i->src(1).getFile() == FILE_IMMEDIATE);
      emitARL(i, i->getSrc(1)->reg.data.u32 & 0x3f);
   } else {
      code[0] = 0x30000001;
      code[1] = (i->op == OP_SHR) ? 0xe0000000 : 0xc0000000;
      if (typeSizeof(i->dType) == 4)
         code[1] |= 0x04000000;
      if (i->op == OP_SHR && isSignedType(i->sType))
          code[1] |= 1 << 27;

      if (i->src(1).getFile() == FILE_IMMEDIATE) {
         code[1] |= 1 << 20;
         code[0] |= (i->getSrc(1)->reg.data.u32 & 0x7f) << 16;
         defId(i->def(0), 2);
         srcId(i->src(0), 9);
         emitFlagsRd(i);
      } else {
         emitForm_MAD(i);
      }
   }
}

void
CodeEmitterNV50::emitOUT(const Instruction *i)
{
   code[0] = (i->op == OP_EMIT) ? 0xf0000201 : 0xf0000401;
   code[1] = 0xc0000000;

   emitFlagsRd(i);
}

void
CodeEmitterNV50::emitTEX(const TexInstruction *i)
{
   code[0] = 0xf0000001;
   code[1] = 0x00000000;

   switch (i->op) {
   case OP_TXB:
      code[1] = 0x20000000;
      break;
   case OP_TXL:
      code[1] = 0x40000000;
      break;
   case OP_TXF:
      code[0] |= 0x01000000;
      break;
   case OP_TXG:
      code[0] |= 0x01000000;
      code[1] = 0x80000000;
      break;
   case OP_TXLQ:
      code[1] = 0x60020000;
      break;
   default:
      assert(i->op == OP_TEX);
      break;
   }

   code[0] |= i->tex.r << 9;
   code[0] |= i->tex.s << 17;

   int argc = i->tex.target.getArgCount();

   if (i->op == OP_TXB || i->op == OP_TXL || i->op == OP_TXF)
      argc += 1;
   if (i->tex.target.isShadow())
      argc += 1;
   assert(argc <= 4);

   code[0] |= (argc - 1) << 22;

   if (i->tex.target.isCube()) {
      code[0] |= 0x08000000;
   } else
   if (i->tex.useOffsets) {
      code[1] |= (i->tex.offset[0] & 0xf) << 24;
      code[1] |= (i->tex.offset[1] & 0xf) << 20;
      code[1] |= (i->tex.offset[2] & 0xf) << 16;
   }

   code[0] |= (i->tex.mask & 0x3) << 25;
   code[1] |= (i->tex.mask & 0xc) << 12;

   if (i->tex.liveOnly)
      code[1] |= 1 << 2;
   if (i->tex.derivAll)
      code[1] |= 1 << 3;

   defId(i->def(0), 2);

   emitFlagsRd(i);
}

void
CodeEmitterNV50::emitTXQ(const TexInstruction *i)
{
   assert(i->tex.query == TXQ_DIMS);

   code[0] = 0xf0000001;
   code[1] = 0x60000000;

   code[0] |= i->tex.r << 9;
   code[0] |= i->tex.s << 17;

   code[0] |= (i->tex.mask & 0x3) << 25;
   code[1] |= (i->tex.mask & 0xc) << 12;

   defId(i->def(0), 2);

   emitFlagsRd(i);
}

void
CodeEmitterNV50::emitTEXPREP(const TexInstruction *i)
{
   code[0] = 0xf8000001 | (3 << 22) | (i->tex.s << 17) | (i->tex.r << 9);
   code[1] = 0x60010000;

   code[0] |= (i->tex.mask & 0x3) << 25;
   code[1] |= (i->tex.mask & 0xc) << 12;
   defId(i->def(0), 2);

   emitFlagsRd(i);
}

void
CodeEmitterNV50::emitPRERETEmu(const FlowInstruction *i)
{
   uint32_t pos = i->target.bb->binPos + 8; // +8 to skip an op */

   code[0] = 0x10000003; // bra
   code[1] = 0x00000780; // always

   switch (i->subOp) {
   case NV50_IR_SUBOP_EMU_PRERET + 0: // bra to the call
      break;
   case NV50_IR_SUBOP_EMU_PRERET + 1: // bra to skip the call
      pos += 8;
      break;
   default:
      assert(i->subOp == (NV50_IR_SUBOP_EMU_PRERET + 2));
      code[0] = 0x20000003; // call
      code[1] = 0x00000000; // no predicate
      break;
   }
   addReloc(RelocEntry::TYPE_CODE, 0, pos, 0x07fff800, 9);
   addReloc(RelocEntry::TYPE_CODE, 1, pos, 0x000fc000, -4);
}

void
CodeEmitterNV50::emitFlow(const Instruction *i, uint8_t flowOp)
{
   const FlowInstruction *f = i->asFlow();
   bool hasPred = false;
   bool hasTarg = false;

   code[0] = 0x00000003 | (flowOp << 28);
   code[1] = 0x00000000;

   switch (i->op) {
   case OP_BRA:
      hasPred = true;
      hasTarg = true;
      break;
   case OP_BREAK:
   case OP_BRKPT:
   case OP_DISCARD:
   case OP_RET:
      hasPred = true;
      break;
   case OP_CALL:
   case OP_PREBREAK:
   case OP_JOINAT:
      hasTarg = true;
      break;
   case OP_PRERET:
      hasTarg = true;
      if (i->subOp >= NV50_IR_SUBOP_EMU_PRERET) {
         emitPRERETEmu(f);
         return;
      }
      break;
   default:
      break;
   }

   if (hasPred)
      emitFlagsRd(i);

   if (hasTarg && f) {
      uint32_t pos;

      if (f->op == OP_CALL) {
         if (f->builtin) {
            pos = targNV50->getBuiltinOffset(f->target.builtin);
         } else {
            pos = f->target.fn->binPos;
         }
      } else {
         pos = f->target.bb->binPos;
      }

      code[0] |= ((pos >>  2) & 0xffff) << 11;
      code[1] |= ((pos >> 18) & 0x003f) << 14;

      RelocEntry::Type relocTy;

      relocTy = f->builtin ? RelocEntry::TYPE_BUILTIN : RelocEntry::TYPE_CODE;

      addReloc(relocTy, 0, pos, 0x07fff800, 9);
      addReloc(relocTy, 1, pos, 0x000fc000, -4);
   }
}

void
CodeEmitterNV50::emitBAR(const Instruction *i)
{
   ImmediateValue *barId = i->getSrc(0)->asImm();
   assert(barId);

   code[0] = 0x82000003 | (barId->reg.data.u32 << 21);
   code[1] = 0x00004000;

   if (i->subOp == NV50_IR_SUBOP_BAR_SYNC)
      code[0] |= 1 << 26;
}

void
CodeEmitterNV50::emitATOM(const Instruction *i)
{
   uint8_t subOp;
   switch (i->subOp) {
   case NV50_IR_SUBOP_ATOM_ADD:  subOp = 0x0; break;
   case NV50_IR_SUBOP_ATOM_MIN:  subOp = 0x7; break;
   case NV50_IR_SUBOP_ATOM_MAX:  subOp = 0x6; break;
   case NV50_IR_SUBOP_ATOM_INC:  subOp = 0x4; break;
   case NV50_IR_SUBOP_ATOM_DEC:  subOp = 0x5; break;
   case NV50_IR_SUBOP_ATOM_AND:  subOp = 0xa; break;
   case NV50_IR_SUBOP_ATOM_OR:   subOp = 0xb; break;
   case NV50_IR_SUBOP_ATOM_XOR:  subOp = 0xc; break;
   case NV50_IR_SUBOP_ATOM_CAS:  subOp = 0x2; break;
   case NV50_IR_SUBOP_ATOM_EXCH: subOp = 0x1; break;
   default:
      assert(!"invalid subop");
      return;
   }
   code[0] = 0xd0000001;
   code[1] = 0xc0c00000 | (subOp << 2);
   if (isSignedType(i->dType))
      code[1] |= 1 << 21;

   // args
   emitFlagsRd(i);
   if (i->subOp == NV50_IR_SUBOP_ATOM_EXCH ||
       i->subOp == NV50_IR_SUBOP_ATOM_CAS ||
       i->defExists(0)) {
      code[1] |= 0x20000000;
      setDst(i, 0);
      setSrc(i, 1, 1);
      // g[] pointer
      code[0] |= i->getSrc(0)->reg.fileIndex << 23;
   } else {
      srcId(i->src(1), 2);
      // g[] pointer
      code[0] |= i->getSrc(0)->reg.fileIndex << 16;
   }
   if (i->subOp == NV50_IR_SUBOP_ATOM_CAS)
      setSrc(i, 2, 2);

   srcId(i->getIndirect(0, 0), 9);
}

bool
CodeEmitterNV50::emitInstruction(Instruction *insn)
{
   if (!insn->encSize) {
      ERROR("skipping unencodable instruction: "); insn->print();
      return false;
   } else
   if (codeSize + insn->encSize > codeSizeLimit) {
      ERROR("code emitter output buffer too small\n");
      return false;
   }

   if (insn->bb->getProgram()->dbgFlags & NV50_IR_DEBUG_BASIC) {
      INFO("EMIT: "); insn->print();
   }

   switch (insn->op) {
   case OP_MOV:
      emitMOV(insn);
      break;
   case OP_EXIT:
   case OP_NOP:
   case OP_JOIN:
      emitNOP();
      break;
   case OP_VFETCH:
   case OP_LOAD:
      emitLOAD(insn);
      break;
   case OP_EXPORT:
   case OP_STORE:
      emitSTORE(insn);
      break;
   case OP_PFETCH:
      emitPFETCH(insn);
      break;
   case OP_RDSV:
      emitRDSV(insn);
      break;
   case OP_LINTERP:
   case OP_PINTERP:
      emitINTERP(insn);
      break;
   case OP_ADD:
   case OP_SUB:
      if (insn->dType == TYPE_F64)
         emitDADD(insn);
      else if (isFloatType(insn->dType))
         emitFADD(insn);
      else if (insn->getDef(0)->reg.file == FILE_ADDRESS)
         emitAADD(insn);
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
   case OP_SAD:
      emitISAD(insn);
      break;
   case OP_NOT:
      emitNOT(insn);
      break;
   case OP_AND:
   case OP_OR:
   case OP_XOR:
      emitLogicOp(insn);
      break;
   case OP_SHL:
   case OP_SHR:
      emitShift(insn);
      break;
   case OP_SET:
      emitSET(insn);
      break;
   case OP_MIN:
   case OP_MAX:
      emitMINMAX(insn);
      break;
   case OP_CEIL:
   case OP_FLOOR:
   case OP_TRUNC:
   case OP_ABS:
   case OP_NEG:
   case OP_SAT:
      emitCVT(insn);
      break;
   case OP_CVT:
      if (insn->def(0).getFile() == FILE_ADDRESS)
         emitARL(insn, 0);
      else
      if (insn->def(0).getFile() == FILE_FLAGS ||
          insn->src(0).getFile() == FILE_FLAGS ||
          insn->src(0).getFile() == FILE_ADDRESS)
         emitMOV(insn);
      else
         emitCVT(insn);
      break;
   case OP_RCP:
      emitSFnOp(insn, 0);
      break;
   case OP_RSQ:
      emitSFnOp(insn, 2);
      break;
   case OP_LG2:
      emitSFnOp(insn, 3);
      break;
   case OP_SIN:
      emitSFnOp(insn, 4);
      break;
   case OP_COS:
      emitSFnOp(insn, 5);
      break;
   case OP_EX2:
      emitSFnOp(insn, 6);
      break;
   case OP_PRESIN:
   case OP_PREEX2:
      emitPreOp(insn);
      break;
   case OP_TEX:
   case OP_TXB:
   case OP_TXL:
   case OP_TXF:
   case OP_TXG:
   case OP_TXLQ:
      emitTEX(insn->asTex());
      break;
   case OP_TXQ:
      emitTXQ(insn->asTex());
      break;
   case OP_TEXPREP:
      emitTEXPREP(insn->asTex());
      break;
   case OP_EMIT:
   case OP_RESTART:
      emitOUT(insn);
      break;
   case OP_DISCARD:
      emitFlow(insn, 0x0);
      break;
   case OP_BRA:
      emitFlow(insn, 0x1);
      break;
   case OP_CALL:
      emitFlow(insn, 0x2);
      break;
   case OP_RET:
      emitFlow(insn, 0x3);
      break;
   case OP_PREBREAK:
      emitFlow(insn, 0x4);
      break;
   case OP_BREAK:
      emitFlow(insn, 0x5);
      break;
   case OP_QUADON:
      emitFlow(insn, 0x6);
      break;
   case OP_QUADPOP:
      emitFlow(insn, 0x7);
      break;
   case OP_JOINAT:
      emitFlow(insn, 0xa);
      break;
   case OP_PRERET:
      emitFlow(insn, 0xd);
      break;
   case OP_QUADOP:
      emitQUADOP(insn, insn->lanes, insn->subOp);
      break;
   case OP_DFDX:
      emitQUADOP(insn, 4, insn->src(0).mod.neg() ? 0x66 : 0x99);
      break;
   case OP_DFDY:
      emitQUADOP(insn, 5, insn->src(0).mod.neg() ? 0x5a : 0xa5);
      break;
   case OP_ATOM:
      emitATOM(insn);
      break;
   case OP_BAR:
      emitBAR(insn);
      break;
   case OP_PHI:
   case OP_UNION:
      ERROR("operation should have been eliminated\n");
      return false;
   case OP_SQRT:
   case OP_SELP:
   case OP_SLCT:
   case OP_TXD:
   case OP_PRECONT:
   case OP_CONT:
   case OP_POPCNT:
   case OP_INSBF:
   case OP_EXTBF:
      ERROR("operation should have been lowered\n");
      return false;
   default:
      ERROR("unknown op: %u\n", insn->op);
      return false;
   }
   if (insn->join || insn->op == OP_JOIN)
      code[1] |= 0x2;
   else
   if (insn->exit || insn->op == OP_EXIT)
      code[1] |= 0x1;

   assert((insn->encSize == 8) == (code[0] & 1));

   code += insn->encSize / 4;
   codeSize += insn->encSize;
   return true;
}

uint32_t
CodeEmitterNV50::getMinEncodingSize(const Instruction *i) const
{
   const Target::OpInfo &info = targ->getOpInfo(i);

   if (info.minEncSize > 4 || i->dType == TYPE_F64)
      return 8;

   // check constraints on dst and src operands
   for (int d = 0; i->defExists(d); ++d) {
      if (i->def(d).rep()->reg.data.id > 63 ||
          i->def(d).rep()->reg.file != FILE_GPR)
         return 8;
   }

   for (int s = 0; i->srcExists(s); ++s) {
      DataFile sf = i->src(s).getFile();
      if (sf != FILE_GPR)
         if (sf != FILE_SHADER_INPUT || progType != Program::TYPE_FRAGMENT)
            return 8;
      if (i->src(s).rep()->reg.data.id > 63)
         return 8;
   }

   // check modifiers & rounding
   if (i->join || i->lanes != 0xf || i->exit)
      return 8;
   if (i->op == OP_MUL && i->rnd != ROUND_N)
      return 8;

   if (i->asTex())
      return 8; // TODO: short tex encoding

   // check constraints on short MAD
   if (info.srcNr >= 2 && i->srcExists(2)) {
      if (!i->defExists(0) ||
          (i->flagsSrc >= 0 && SDATA(i->src(i->flagsSrc)).id > 0) ||
          DDATA(i->def(0)).id != SDATA(i->src(2)).id)
         return 8;
   }

   return info.minEncSize;
}

// Change the encoding size of an instruction after BBs have been scheduled.
static void
makeInstructionLong(Instruction *insn)
{
   if (insn->encSize == 8)
      return;
   Function *fn = insn->bb->getFunction();
   int n = 0;
   int adj = 4;

   for (Instruction *i = insn->next; i && i->encSize == 4; ++n, i = i->next);

   if (n & 1) {
      adj = 8;
      insn->next->encSize = 8;
   } else
   if (insn->prev && insn->prev->encSize == 4) {
      adj = 8;
      insn->prev->encSize = 8;
   }
   insn->encSize = 8;

   for (int i = fn->bbCount - 1; i >= 0 && fn->bbArray[i] != insn->bb; --i) {
      fn->bbArray[i]->binPos += adj;
   }
   fn->binSize += adj;
   insn->bb->binSize += adj;
}

static bool
trySetExitModifier(Instruction *insn)
{
   if (insn->op == OP_DISCARD ||
       insn->op == OP_QUADON ||
       insn->op == OP_QUADPOP)
      return false;
   for (int s = 0; insn->srcExists(s); ++s)
      if (insn->src(s).getFile() == FILE_IMMEDIATE)
         return false;
   if (insn->asFlow()) {
      if (insn->op == OP_CALL) // side effects !
         return false;
      if (insn->getPredicate()) // cannot do conditional exit (or can we ?)
         return false;
      insn->op = OP_EXIT;
   }
   insn->exit = 1;
   makeInstructionLong(insn);
   return true;
}

static void
replaceExitWithModifier(Function *func)
{
   BasicBlock *epilogue = BasicBlock::get(func->cfgExit);

   if (!epilogue->getExit() ||
       epilogue->getExit()->op != OP_EXIT) // only main will use OP_EXIT
      return;

   if (epilogue->getEntry()->op != OP_EXIT) {
      Instruction *insn = epilogue->getExit()->prev;
      if (!insn || !trySetExitModifier(insn))
         return;
      insn->exit = 1;
   } else {
      for (Graph::EdgeIterator ei = func->cfgExit->incident();
           !ei.end(); ei.next()) {
         BasicBlock *bb = BasicBlock::get(ei.getNode());
         Instruction *i = bb->getExit();

         if (!i || !trySetExitModifier(i))
            return;
      }
   }

   int adj = epilogue->getExit()->encSize;
   epilogue->binSize -= adj;
   func->binSize -= adj;
   delete_Instruction(func->getProgram(), epilogue->getExit());

   // There may be BB's that are laid out after the exit block
   for (int i = func->bbCount - 1; i >= 0 && func->bbArray[i] != epilogue; --i) {
      func->bbArray[i]->binPos -= adj;
   }
}

void
CodeEmitterNV50::prepareEmission(Function *func)
{
   CodeEmitter::prepareEmission(func);

   replaceExitWithModifier(func);
}

CodeEmitterNV50::CodeEmitterNV50(Program::Type type, const TargetNV50 *target) :
   CodeEmitter(target), progType(type), targNV50(target)
{
   targ = target; // specialized
   code = NULL;
   codeSize = codeSizeLimit = 0;
   relocInfo = NULL;
}

CodeEmitter *
TargetNV50::getCodeEmitter(Program::Type type)
{
   CodeEmitterNV50 *emit = new CodeEmitterNV50(type, this);
   return emit;
}

} // namespace nv50_ir
