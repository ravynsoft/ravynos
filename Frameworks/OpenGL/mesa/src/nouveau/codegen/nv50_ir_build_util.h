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

#ifndef __NV50_IR_BUILD_UTIL__
#define __NV50_IR_BUILD_UTIL__

namespace nv50_ir {

class BuildUtil
{
public:
   BuildUtil();
   BuildUtil(Program *);

   inline void setProgram(Program *);
   inline Program *getProgram() const { return prog; }
   inline Function *getFunction() const { return func; }

   // keeps inserting at head/tail of block
   inline void setPosition(BasicBlock *, bool tail);
   // position advances only if @after is true
   inline void setPosition(Instruction *, bool after);

   inline BasicBlock *getBB() { return bb; }

   inline void insert(Instruction *);
   inline void remove(Instruction *i) { assert(i->bb == bb); bb->remove(i); }

   inline LValue *getScratch(int size = 4, DataFile = FILE_GPR);
   // scratch value for a single assignment:
   inline LValue *getSSA(int size = 4, DataFile = FILE_GPR);

   inline Instruction *mkOp(operation, DataType, Value *);
   Instruction *mkOp1(operation, DataType, Value *, Value *);
   Instruction *mkOp2(operation, DataType, Value *, Value *, Value *);
   Instruction *mkOp3(operation, DataType, Value *, Value *, Value *, Value *);

   LValue *mkOp1v(operation, DataType, Value *, Value *);
   LValue *mkOp2v(operation, DataType, Value *, Value *, Value *);
   LValue *mkOp3v(operation, DataType, Value *, Value *, Value *, Value *);

   Instruction *mkLoad(DataType, Value *dst, Symbol *, Value *ptr);
   Instruction *mkStore(operation, DataType, Symbol *, Value *ptr, Value *val);

   LValue *mkLoadv(DataType, Symbol *, Value *ptr);

   Instruction *mkMov(Value *, Value *, DataType = TYPE_U32);
   Instruction *mkMovToReg(int id, Value *);
   Instruction *mkMovFromReg(Value *, int id);
   inline Instruction *mkBMov(Value *, Value *);

   Instruction *mkInterp(unsigned mode, Value *, int32_t offset, Value *rel);
   Instruction *mkFetch(Value *, DataType, DataFile, int32_t offset,
                        Value *attrRel, Value *primRel);

   Instruction *mkCvt(operation, DataType, Value *, DataType, Value *);
   CmpInstruction *mkCmp(operation, CondCode, DataType,
                         Value *,
                         DataType, Value *, Value *, Value * = NULL);
   TexInstruction *mkTex(operation, TexTarget,
                         uint16_t tic, uint16_t tsc,
                         const std::vector<Value *> &def,
                         const std::vector<Value *> &src);
   Instruction *mkQuadop(uint8_t qop, Value *, uint8_t l, Value *, Value *);

   FlowInstruction *mkFlow(operation, void *target, CondCode, Value *pred);

   Instruction *mkSelect(Value *pred, Value *dst, Value *trSrc, Value *flSrc);

   Instruction *mkSplit(Value *half[2], uint8_t halfSize, Value *);

   void mkClobber(DataFile file, uint32_t regMask, int regUnitLog2);

   ImmediateValue *mkImm(float);
   ImmediateValue *mkImm(double);
   ImmediateValue *mkImm(uint16_t);
   ImmediateValue *mkImm(uint32_t);
   ImmediateValue *mkImm(uint64_t);

   ImmediateValue *mkImm(int i) { return mkImm((uint32_t)i); }

   Value *loadImm(Value *dst, float);
   Value *loadImm(Value *dst, double);
   Value *loadImm(Value *dst, uint16_t);
   Value *loadImm(Value *dst, uint32_t);
   Value *loadImm(Value *dst, uint64_t);

   Value *loadImm(Value *dst, int i) { return loadImm(dst, (uint32_t)i); }

   // returns high part of the operation
   static Instruction *split64BitOpPostRA(Function *, Instruction *,
                                          Value *zero, Value *carry);

   Symbol *mkSymbol(DataFile file, int8_t fileIndex,
                    DataType ty, uint32_t baseAddress);

   Symbol *mkSysVal(SVSemantic svName, uint32_t svIndex);
   Symbol *mkTSVal(TSSemantic tsName);

private:
   void init(Program *);
   void addImmediate(ImmediateValue *);
   inline unsigned int u32Hash(uint32_t);

protected:
   Program *prog;
   Function *func;
   Instruction *pos;
   BasicBlock *bb;
   bool tail;

#define NV50_IR_BUILD_IMM_HT_SIZE 256

   ImmediateValue *imms[NV50_IR_BUILD_IMM_HT_SIZE];
   unsigned int immCount;
};

unsigned int BuildUtil::u32Hash(uint32_t u)
{
   return (u % 273) % NV50_IR_BUILD_IMM_HT_SIZE;
}

void BuildUtil::setProgram(Program *program)
{
   prog = program;
}

void
BuildUtil::setPosition(BasicBlock *block, bool atTail)
{
   bb = block;
   prog = bb->getProgram();
   func = bb->getFunction();
   pos = NULL;
   tail = atTail;
}

void
BuildUtil::setPosition(Instruction *i, bool after)
{
   bb = i->bb;
   prog = bb->getProgram();
   func = bb->getFunction();
   pos = i;
   tail = after;
   assert(bb);
}

LValue *
BuildUtil::getScratch(int size, DataFile f)
{
   LValue *lval = new_LValue(func, f);
   lval->reg.size = size;
   return lval;
}

LValue *
BuildUtil::getSSA(int size, DataFile f)
{
   LValue *lval = new_LValue(func, f);
   lval->ssa = 1;
   lval->reg.size = size;
   return lval;
}

void BuildUtil::insert(Instruction *i)
{
   if (!pos) {
      tail ? bb->insertTail(i) : bb->insertHead(i);
   } else {
      if (tail) {
         bb->insertAfter(pos, i);
         pos = i;
      } else {
         bb->insertBefore(pos, i);
      }
   }
}

Instruction *
BuildUtil::mkOp(operation op, DataType ty, Value *dst)
{
   Instruction *insn = new_Instruction(func, op, ty);
   insn->setDef(0, dst);
   insert(insn);
   if (op == OP_DISCARD || op == OP_EXIT ||
       op == OP_JOIN ||
       op == OP_QUADON || op == OP_QUADPOP ||
       op == OP_EMIT || op == OP_RESTART)
      insn->fixed = 1;
   return insn;
}

inline LValue *
BuildUtil::mkOp1v(operation op, DataType ty, Value *dst, Value *src)
{
   mkOp1(op, ty, dst, src);
   return dst->asLValue();
}

inline LValue *
BuildUtil::mkOp2v(operation op, DataType ty, Value *dst,
                  Value *src0, Value *src1)
{
   mkOp2(op, ty, dst, src0, src1);
   return dst->asLValue();
}

inline LValue *
BuildUtil::mkOp3v(operation op, DataType ty, Value *dst,
                  Value *src0, Value *src1, Value *src2)
{
   mkOp3(op, ty, dst, src0, src1, src2);
   return dst->asLValue();
}

inline LValue *
BuildUtil::mkLoadv(DataType ty, Symbol *mem, Value *ptr)
{
   LValue *dst = getScratch(typeSizeof(ty));
   mkLoad(ty, dst, mem, ptr);
   return dst;
}

inline Instruction *
BuildUtil::mkBMov(Value *dst, Value *src)
{
   return mkCvt(OP_CVT, TYPE_U32, dst, TYPE_U32, src);
}

} // namespace nv50_ir

#endif // __NV50_IR_BUILD_UTIL_H__
