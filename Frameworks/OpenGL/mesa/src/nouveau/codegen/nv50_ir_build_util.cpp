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
#include "nv50_ir_build_util.h"

namespace nv50_ir {

BuildUtil::BuildUtil()
{
   init(NULL);
}

BuildUtil::BuildUtil(Program *prog)
{
   init(prog);
}

void
BuildUtil::init(Program *prog)
{
   this->prog = prog;

   func = NULL;
   bb = NULL;
   pos = NULL;

   tail = false;

   memset(imms, 0, sizeof(imms));
   immCount = 0;
}

void
BuildUtil::addImmediate(ImmediateValue *imm)
{
   if (immCount > (NV50_IR_BUILD_IMM_HT_SIZE * 3) / 4)
      return;

   unsigned int pos = u32Hash(imm->reg.data.u32);

   while (imms[pos])
      pos = (pos + 1) % NV50_IR_BUILD_IMM_HT_SIZE;
   imms[pos] = imm;
   immCount++;
}

Instruction *
BuildUtil::mkOp1(operation op, DataType ty, Value *dst, Value *src)
{
   Instruction *insn = new_Instruction(func, op, ty);

   insn->setDef(0, dst);
   insn->setSrc(0, src);

   insert(insn);
   return insn;
}

Instruction *
BuildUtil::mkOp2(operation op, DataType ty, Value *dst,
                 Value *src0, Value *src1)
{
   Instruction *insn = new_Instruction(func, op, ty);

   insn->setDef(0, dst);
   insn->setSrc(0, src0);
   insn->setSrc(1, src1);

   insert(insn);
   return insn;
}

Instruction *
BuildUtil::mkOp3(operation op, DataType ty, Value *dst,
                 Value *src0, Value *src1, Value *src2)
{
   Instruction *insn = new_Instruction(func, op, ty);

   insn->setDef(0, dst);
   insn->setSrc(0, src0);
   insn->setSrc(1, src1);
   insn->setSrc(2, src2);

   insert(insn);
   return insn;
}

Instruction *
BuildUtil::mkLoad(DataType ty, Value *dst, Symbol *mem, Value *ptr)
{
   Instruction *insn = new_Instruction(func, OP_LOAD, ty);

   insn->setDef(0, dst);
   insn->setSrc(0, mem);
   if (ptr)
      insn->setIndirect(0, 0, ptr);

   insert(insn);
   return insn;
}

Instruction *
BuildUtil::mkStore(operation op, DataType ty, Symbol *mem, Value *ptr,
                   Value *stVal)
{
   Instruction *insn = new_Instruction(func, op, ty);

   insn->setSrc(0, mem);
   insn->setSrc(1, stVal);
   if (ptr)
      insn->setIndirect(0, 0, ptr);

   insert(insn);
   return insn;
}

Instruction *
BuildUtil::mkFetch(Value *dst, DataType ty, DataFile file, int32_t offset,
                   Value *attrRel, Value *primRel)
{
   Symbol *sym = mkSymbol(file, 0, ty, offset);

   Instruction *insn = mkOp1(OP_VFETCH, ty, dst, sym);

   insn->setIndirect(0, 0, attrRel);
   insn->setIndirect(0, 1, primRel);

   // already inserted
   return insn;
}

Instruction *
BuildUtil::mkInterp(unsigned mode, Value *dst, int32_t offset, Value *rel)
{
   operation op = OP_LINTERP;
   DataType ty = TYPE_F32;

   if ((mode & NV50_IR_INTERP_MODE_MASK) == NV50_IR_INTERP_FLAT)
      ty = TYPE_U32;
   else
   if ((mode & NV50_IR_INTERP_MODE_MASK) == NV50_IR_INTERP_PERSPECTIVE)
      op = OP_PINTERP;

   Symbol *sym = mkSymbol(FILE_SHADER_INPUT, 0, ty, offset);

   Instruction *insn = mkOp1(op, ty, dst, sym);
   insn->setIndirect(0, 0, rel);
   insn->setInterpolate(mode);
   return insn;
}

Instruction *
BuildUtil::mkMov(Value *dst, Value *src, DataType ty)
{
   Instruction *insn = new_Instruction(func, OP_MOV, ty);

   insn->setDef(0, dst);
   insn->setSrc(0, src);

   insert(insn);
   return insn;
}

Instruction *
BuildUtil::mkMovToReg(int id, Value *src)
{
   Instruction *insn = new_Instruction(func, OP_MOV, typeOfSize(src->reg.size));

   insn->setDef(0, new_LValue(func, FILE_GPR));
   insn->getDef(0)->reg.data.id = id;
   insn->setSrc(0, src);

   insert(insn);
   return insn;
}

Instruction *
BuildUtil::mkMovFromReg(Value *dst, int id)
{
   Instruction *insn = new_Instruction(func, OP_MOV, typeOfSize(dst->reg.size));

   insn->setDef(0, dst);
   insn->setSrc(0, new_LValue(func, FILE_GPR));
   insn->getSrc(0)->reg.data.id = id;

   insert(insn);
   return insn;
}

Instruction *
BuildUtil::mkCvt(operation op,
                 DataType dstTy, Value *dst, DataType srcTy, Value *src)
{
   Instruction *insn = new_Instruction(func, op, dstTy);

   insn->setType(dstTy, srcTy);
   insn->setDef(0, dst);
   insn->setSrc(0, src);

   insert(insn);
   return insn;
}

CmpInstruction *
BuildUtil::mkCmp(operation op, CondCode cc, DataType dstTy, Value *dst,
                 DataType srcTy, Value *src0, Value *src1, Value *src2)
{
   CmpInstruction *insn = new_CmpInstruction(func, op);

   insn->setType((dst->reg.file == FILE_PREDICATE ||
                  dst->reg.file == FILE_FLAGS) ? TYPE_U8 : dstTy, srcTy);
   insn->setCondition(cc);
   insn->setDef(0, dst);
   insn->setSrc(0, src0);
   insn->setSrc(1, src1);
   if (src2)
      insn->setSrc(2, src2);

   if (dst->reg.file == FILE_FLAGS)
      insn->flagsDef = 0;

   insert(insn);
   return insn;
}

TexInstruction *
BuildUtil::mkTex(operation op, TexTarget targ,
                 uint16_t tic, uint16_t tsc,
                 const std::vector<Value *> &def,
                 const std::vector<Value *> &src)
{
   TexInstruction *tex = new_TexInstruction(func, op);

   for (size_t d = 0; d < def.size() && def[d]; ++d)
      tex->setDef(d, def[d]);
   for (size_t s = 0; s < src.size() && src[s]; ++s)
      tex->setSrc(s, src[s]);

   tex->setTexture(targ, tic, tsc);

   insert(tex);
   return tex;
}

Instruction *
BuildUtil::mkQuadop(uint8_t q, Value *def, uint8_t l, Value *src0, Value *src1)
{
   Instruction *quadop = mkOp2(OP_QUADOP, TYPE_F32, def, src0, src1);
   quadop->subOp = q;
   quadop->lanes = l;
   return quadop;
}

Instruction *
BuildUtil::mkSelect(Value *pred, Value *dst, Value *trSrc, Value *flSrc)
{
   LValue *def0 = getSSA();
   LValue *def1 = getSSA();

   mkMov(def0, trSrc)->setPredicate(CC_P, pred);
   mkMov(def1, flSrc)->setPredicate(CC_NOT_P, pred);

   return mkOp2(OP_UNION, typeOfSize(dst->reg.size), dst, def0, def1);
}

Instruction *
BuildUtil::mkSplit(Value *h[2], uint8_t halfSize, Value *val)
{
   Instruction *insn = NULL;

   const DataType fTy = typeOfSize(halfSize * 2);

   if (val->reg.file == FILE_IMMEDIATE)
      val = mkMov(getSSA(halfSize * 2), val, fTy)->getDef(0);

   if (isMemoryFile(val->reg.file)) {
      h[0] = cloneShallow(getFunction(), val);
      h[1] = cloneShallow(getFunction(), val);
      h[0]->reg.size = halfSize;
      h[1]->reg.size = halfSize;
      h[1]->reg.data.offset += halfSize;
   } else {
      // The value might already be the result of a split, and further
      // splitting it can lead to issues regarding spill-offsets computations
      // among others. By forcing a move between the two splits, this can be
      // avoided.
      Instruction* valInsn = val->getInsn();
      if (valInsn && valInsn->op == OP_SPLIT)
         val = mkMov(getSSA(halfSize * 2), val, fTy)->getDef(0);

      h[0] = getSSA(halfSize, val->reg.file);
      h[1] = getSSA(halfSize, val->reg.file);
      insn = mkOp1(OP_SPLIT, fTy, h[0], val);
      insn->setDef(1, h[1]);
   }
   return insn;
}

FlowInstruction *
BuildUtil::mkFlow(operation op, void *targ, CondCode cc, Value *pred)
{
   FlowInstruction *insn = new_FlowInstruction(func, op, targ);

   if (pred)
      insn->setPredicate(cc, pred);

   insert(insn);
   return insn;
}

void
BuildUtil::mkClobber(DataFile f, uint32_t rMask, int unit)
{
   static const uint16_t baseSize2[16] =
   {
      0x0000, 0x0010, 0x0011, 0x0020, 0x0012, 0x1210, 0x1211, 0x1220,
      0x0013, 0x1310, 0x1311, 0x1320, 0x0022, 0x2210, 0x2211, 0x0040,
   };

   int base = 0;

   for (; rMask; rMask >>= 4, base += 4) {
      const uint32_t mask = rMask & 0xf;
      if (!mask)
         continue;
      int base1 = (baseSize2[mask] >>  0) & 0xf;
      int size1 = (baseSize2[mask] >>  4) & 0xf;
      int base2 = (baseSize2[mask] >>  8) & 0xf;
      int size2 = (baseSize2[mask] >> 12) & 0xf;
      Instruction *insn = mkOp(OP_NOP, TYPE_NONE, NULL);
      if (true) { // size1 can't be 0
         LValue *reg = new_LValue(func, f);
         reg->reg.size = size1 << unit;
         reg->reg.data.id = base + base1;
         insn->setDef(0, reg);
      }
      if (size2) {
         LValue *reg = new_LValue(func, f);
         reg->reg.size = size2 << unit;
         reg->reg.data.id = base + base2;
         insn->setDef(1, reg);
      }
   }
}

ImmediateValue *
BuildUtil::mkImm(uint16_t u)
{
   ImmediateValue *imm = new_ImmediateValue(prog, (uint32_t)0);

   imm->reg.size = 2;
   imm->reg.type = TYPE_U16;
   imm->reg.data.u32 = u;

   return imm;
}

ImmediateValue *
BuildUtil::mkImm(uint32_t u)
{
   unsigned int pos = u32Hash(u);

   while (imms[pos] && imms[pos]->reg.data.u32 != u)
      pos = (pos + 1) % NV50_IR_BUILD_IMM_HT_SIZE;

   ImmediateValue *imm = imms[pos];
   if (!imm) {
      imm = new_ImmediateValue(prog, u);
      addImmediate(imm);
   }
   return imm;
}

ImmediateValue *
BuildUtil::mkImm(uint64_t u)
{
   ImmediateValue *imm = new_ImmediateValue(prog, (uint32_t)0);

   imm->reg.size = 8;
   imm->reg.type = TYPE_U64;
   imm->reg.data.u64 = u;

   return imm;
}

ImmediateValue *
BuildUtil::mkImm(float f)
{
   union {
      float f32;
      uint32_t u32;
   } u;
   u.f32 = f;
   return mkImm(u.u32);
}

ImmediateValue *
BuildUtil::mkImm(double d)
{
   return new_ImmediateValue(prog, d);
}

Value *
BuildUtil::loadImm(Value *dst, float f)
{
   return mkOp1v(OP_MOV, TYPE_F32, dst ? dst : getScratch(), mkImm(f));
}

Value *
BuildUtil::loadImm(Value *dst, double d)
{
   return mkOp1v(OP_MOV, TYPE_F64, dst ? dst : getScratch(8), mkImm(d));
}

Value *
BuildUtil::loadImm(Value *dst, uint16_t u)
{
   return mkOp1v(OP_MOV, TYPE_U16, dst ? dst : getScratch(2), mkImm(u));
}

Value *
BuildUtil::loadImm(Value *dst, uint32_t u)
{
   return mkOp1v(OP_MOV, TYPE_U32, dst ? dst : getScratch(), mkImm(u));
}

Value *
BuildUtil::loadImm(Value *dst, uint64_t u)
{
   return mkOp1v(OP_MOV, TYPE_U64, dst ? dst : getScratch(8), mkImm(u));
}

Symbol *
BuildUtil::mkSymbol(DataFile file, int8_t fileIndex, DataType ty,
                    uint32_t baseAddr)
{
   Symbol *sym = new_Symbol(prog, file, fileIndex);

   sym->setOffset(baseAddr);
   sym->reg.type = ty;
   sym->reg.size = typeSizeof(ty);

   return sym;
}

Symbol *
BuildUtil::mkSysVal(SVSemantic svName, uint32_t svIndex)
{
   Symbol *sym = new_Symbol(prog, FILE_SYSTEM_VALUE, 0);

   assert(svIndex < 4 || svName == SV_CLIP_DISTANCE);

   switch (svName) {
   case SV_POSITION:
   case SV_FACE:
   case SV_YDIR:
   case SV_POINT_SIZE:
   case SV_POINT_COORD:
   case SV_CLIP_DISTANCE:
   case SV_TESS_OUTER:
   case SV_TESS_INNER:
   case SV_TESS_COORD:
      sym->reg.type = TYPE_F32;
      break;
   default:
      sym->reg.type = TYPE_U32;
      break;
   }
   sym->reg.size = typeSizeof(sym->reg.type);

   sym->reg.data.sv.sv = svName;
   sym->reg.data.sv.index = svIndex;

   return sym;
}

Symbol *
BuildUtil::mkTSVal(TSSemantic tsName)
{
   Symbol *sym = new_Symbol(prog, FILE_THREAD_STATE, 0);
   sym->reg.type = TYPE_U32;
   sym->reg.size = typeSizeof(sym->reg.type);
   sym->reg.data.ts = tsName;
   return sym;
}


Instruction *
BuildUtil::split64BitOpPostRA(Function *fn, Instruction *i,
                              Value *zero,
                              Value *carry)
{
   DataType hTy;
   int srcNr;

   switch (i->dType) {
   case TYPE_U64: hTy = TYPE_U32; break;
   case TYPE_S64: hTy = TYPE_S32; break;
   case TYPE_F64:
      if (i->op == OP_MOV) {
         hTy = TYPE_U32;
         break;
      }
      FALLTHROUGH;
   default:
      return NULL;
   }

   switch (i->op) {
   case OP_MOV: srcNr = 1; break;
   case OP_ADD:
   case OP_SUB:
      if (!carry)
         return NULL;
      srcNr = 2;
      break;
   case OP_SELP: srcNr = 3; break;
   default:
      // TODO when needed
      return NULL;
   }

   i->setType(hTy);
   i->setDef(0, cloneShallow(fn, i->getDef(0)));
   i->getDef(0)->reg.size = 4;
   Instruction *lo = i;
   Instruction *hi = cloneForward(fn, i);
   lo->bb->insertAfter(lo, hi);

   hi->getDef(0)->reg.data.id++;

   for (int s = 0; s < srcNr; ++s) {
      if (lo->getSrc(s)->reg.size < 8) {
         if (s == 2)
            hi->setSrc(s, lo->getSrc(s));
         else
            hi->setSrc(s, zero);
      } else {
         if (lo->getSrc(s)->refCount() > 1)
            lo->setSrc(s, cloneShallow(fn, lo->getSrc(s)));
         lo->getSrc(s)->reg.size /= 2;
         hi->setSrc(s, cloneShallow(fn, lo->getSrc(s)));

         switch (hi->src(s).getFile()) {
         case FILE_IMMEDIATE:
            hi->getSrc(s)->reg.data.u64 >>= 32;
            break;
         case FILE_MEMORY_CONST:
         case FILE_MEMORY_SHARED:
         case FILE_SHADER_INPUT:
         case FILE_SHADER_OUTPUT:
            hi->getSrc(s)->reg.data.offset += 4;
            break;
         default:
            assert(hi->src(s).getFile() == FILE_GPR);
            hi->getSrc(s)->reg.data.id++;
            break;
         }
      }
   }
   if (srcNr == 2) {
      lo->setFlagsDef(1, carry);
      hi->setFlagsSrc(hi->srcCount(), carry);
   }
   return hi;
}

} // namespace nv50_ir
