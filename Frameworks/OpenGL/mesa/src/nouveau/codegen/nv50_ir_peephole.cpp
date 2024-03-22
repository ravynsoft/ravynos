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
#include "nv50_ir_target.h"
#include "nv50_ir_build_util.h"

extern "C" {
#include "util/u_math.h"
}

namespace nv50_ir {

bool
Instruction::isNop() const
{
   if (op == OP_PHI || op == OP_SPLIT || op == OP_MERGE)
      return true;
   if (terminator || join) // XXX: should terminator imply flow ?
      return false;
   if (op == OP_ATOM)
      return false;
   if (!fixed && op == OP_NOP)
      return true;

   if (defExists(0) && def(0).rep()->reg.data.id < 0) {
      for (int d = 1; defExists(d); ++d)
         if (def(d).rep()->reg.data.id >= 0)
            WARN("part of vector result is unused !\n");
      return true;
   }

   if (op == OP_MOV || op == OP_UNION) {
      if (!getDef(0)->equals(getSrc(0)))
         return false;
      if (op == OP_UNION)
         if (!getDef(0)->equals(getSrc(1)))
            return false;
      return true;
   }

   return false;
}

bool Instruction::isDead() const
{
   if (op == OP_STORE ||
       op == OP_EXPORT ||
       op == OP_ATOM ||
       op == OP_SUSTB || op == OP_SUSTP || op == OP_SUREDP || op == OP_SUREDB)
      return false;

   for (int d = 0; defExists(d); ++d)
      if (getDef(d)->refCount() || getDef(d)->reg.data.id >= 0)
         return false;

   if (terminator || asFlow())
      return false;
   if (fixed)
      return false;

   return true;
};

// =============================================================================

class CopyPropagation : public Pass
{
private:
   virtual bool visit(BasicBlock *);
};

// Propagate all MOVs forward to make subsequent optimization easier, except if
// the sources stem from a phi, in which case we don't want to mess up potential
// swaps $rX <-> $rY, i.e. do not create live range overlaps of phi src and def.
bool
CopyPropagation::visit(BasicBlock *bb)
{
   Instruction *mov, *si, *next;

   for (mov = bb->getEntry(); mov; mov = next) {
      next = mov->next;
      if (mov->op != OP_MOV || mov->fixed || !mov->getSrc(0)->asLValue())
         continue;
      if (mov->getPredicate())
         continue;
      if (mov->def(0).getFile() != mov->src(0).getFile())
         continue;
      si = mov->getSrc(0)->getInsn();
      if (mov->getDef(0)->reg.data.id < 0 && si && si->op != OP_PHI) {
         // propagate
         mov->def(0).replace(mov->getSrc(0), false);
         delete_Instruction(prog, mov);
      }
   }
   return true;
}

// =============================================================================

class MergeSplits : public Pass
{
private:
   virtual bool visit(BasicBlock *);
};

// For SPLIT / MERGE pairs that operate on the same registers, replace the
// post-merge def with the SPLIT's source.
bool
MergeSplits::visit(BasicBlock *bb)
{
   Instruction *i, *next, *si;

   for (i = bb->getEntry(); i; i = next) {
      next = i->next;
      if (i->op != OP_MERGE || typeSizeof(i->dType) != 8)
         continue;
      si = i->getSrc(0)->getInsn();
      if (si->op != OP_SPLIT || si != i->getSrc(1)->getInsn())
         continue;
      i->def(0).replace(si->getSrc(0), false);
      delete_Instruction(prog, i);
   }

   return true;
}

// =============================================================================

class LoadPropagation : public Pass
{
private:
   virtual bool visit(BasicBlock *);

   void checkSwapSrc01(Instruction *);

   bool isCSpaceLoad(Instruction *);
   bool isImmdLoad(Instruction *);
   bool isAttribOrSharedLoad(Instruction *);
};

bool
LoadPropagation::isCSpaceLoad(Instruction *ld)
{
   return ld && ld->op == OP_LOAD && ld->src(0).getFile() == FILE_MEMORY_CONST;
}

bool
LoadPropagation::isImmdLoad(Instruction *ld)
{
   if (!ld || (ld->op != OP_MOV) ||
       ((typeSizeof(ld->dType) != 4) && (typeSizeof(ld->dType) != 8)))
      return false;

   // A 0 can be replaced with a register, so it doesn't count as an immediate.
   ImmediateValue val;
   return ld->src(0).getImmediate(val) && !val.isInteger(0);
}

bool
LoadPropagation::isAttribOrSharedLoad(Instruction *ld)
{
   return ld &&
      (ld->op == OP_VFETCH ||
       (ld->op == OP_LOAD &&
        (ld->src(0).getFile() == FILE_SHADER_INPUT ||
         ld->src(0).getFile() == FILE_MEMORY_SHARED)));
}

void
LoadPropagation::checkSwapSrc01(Instruction *insn)
{
   const Target *targ = prog->getTarget();
   if (!targ->getOpInfo(insn).commutative) {
      if (insn->op != OP_SET && insn->op != OP_SLCT &&
          insn->op != OP_SUB && insn->op != OP_XMAD)
         return;
      // XMAD is only commutative if both the CBCC and MRG flags are not set.
      if (insn->op == OP_XMAD &&
          (insn->subOp & NV50_IR_SUBOP_XMAD_CMODE_MASK) == NV50_IR_SUBOP_XMAD_CBCC)
         return;
      if (insn->op == OP_XMAD && (insn->subOp & NV50_IR_SUBOP_XMAD_MRG))
         return;
   }
   if (insn->src(1).getFile() != FILE_GPR)
      return;
   // This is the special OP_SET used for alphatesting, we can't reverse its
   // arguments as that will confuse the fixup code.
   if (insn->op == OP_SET && insn->subOp)
      return;

   Instruction *i0 = insn->getSrc(0)->getInsn();
   Instruction *i1 = insn->getSrc(1)->getInsn();

   // Swap sources to inline the less frequently used source. That way,
   // optimistically, it will eventually be able to remove the instruction.
   int i0refs = insn->getSrc(0)->refCount();
   int i1refs = insn->getSrc(1)->refCount();

   if ((isCSpaceLoad(i0) || isImmdLoad(i0)) && targ->insnCanLoad(insn, 1, i0)) {
      if ((!isImmdLoad(i1) && !isCSpaceLoad(i1)) ||
          !targ->insnCanLoad(insn, 1, i1) ||
          i0refs < i1refs)
         insn->swapSources(0, 1);
      else
         return;
   } else
   if (isAttribOrSharedLoad(i1)) {
      if (!isAttribOrSharedLoad(i0))
         insn->swapSources(0, 1);
      else
         return;
   } else {
      return;
   }

   if (insn->op == OP_SET || insn->op == OP_SET_AND ||
       insn->op == OP_SET_OR || insn->op == OP_SET_XOR)
      insn->asCmp()->setCond = reverseCondCode(insn->asCmp()->setCond);
   else
   if (insn->op == OP_SLCT)
      insn->asCmp()->setCond = inverseCondCode(insn->asCmp()->setCond);
   else
   if (insn->op == OP_SUB) {
      insn->src(0).mod = insn->src(0).mod ^ Modifier(NV50_IR_MOD_NEG);
      insn->src(1).mod = insn->src(1).mod ^ Modifier(NV50_IR_MOD_NEG);
   } else
   if (insn->op == OP_XMAD) {
      // swap h1 flags
      uint16_t h1 = (insn->subOp >> 1 & NV50_IR_SUBOP_XMAD_H1(0)) |
                    (insn->subOp << 1 & NV50_IR_SUBOP_XMAD_H1(1));
      insn->subOp = (insn->subOp & ~NV50_IR_SUBOP_XMAD_H1_MASK) | h1;
   }
}

bool
LoadPropagation::visit(BasicBlock *bb)
{
   const Target *targ = prog->getTarget();
   Instruction *next;

   for (Instruction *i = bb->getEntry(); i; i = next) {
      next = i->next;

      if (i->op == OP_CALL) // calls have args as sources, they must be in regs
         continue;

      if (i->op == OP_PFETCH) // pfetch expects arg1 to be a reg
         continue;

      if (i->srcExists(1))
         checkSwapSrc01(i);

      for (int s = 0; i->srcExists(s); ++s) {
         Instruction *ld = i->getSrc(s)->getInsn();

         if (!ld || ld->fixed || (ld->op != OP_LOAD && ld->op != OP_MOV))
            continue;
         if (ld->op == OP_LOAD && ld->subOp == NV50_IR_SUBOP_LOAD_LOCKED)
            continue;
         if (!targ->insnCanLoad(i, s, ld))
            continue;

         // propagate !
         i->setSrc(s, ld->getSrc(0));
         if (ld->src(0).isIndirect(0))
            i->setIndirect(s, 0, ld->getIndirect(0, 0));

         if (ld->getDef(0)->refCount() == 0)
            delete_Instruction(prog, ld);
      }
   }
   return true;
}

// =============================================================================

class IndirectPropagation : public Pass
{
private:
   virtual bool visit(BasicBlock *);

   BuildUtil bld;
};

bool
IndirectPropagation::visit(BasicBlock *bb)
{
   const Target *targ = prog->getTarget();
   Instruction *next;

   for (Instruction *i = bb->getEntry(); i; i = next) {
      next = i->next;

      bld.setPosition(i, false);

      for (int s = 0; i->srcExists(s); ++s) {
         Instruction *insn;
         ImmediateValue imm;
         if (!i->src(s).isIndirect(0))
            continue;
         insn = i->getIndirect(s, 0)->getInsn();
         if (!insn)
            continue;
         if (insn->op == OP_ADD && !isFloatType(insn->dType)) {
            if (insn->src(0).getFile() != targ->nativeFile(FILE_ADDRESS) ||
                !insn->src(1).getImmediate(imm) ||
                !targ->insnCanLoadOffset(i, s, imm.reg.data.s32))
               continue;
            i->setIndirect(s, 0, insn->getSrc(0));
            i->setSrc(s, cloneShallow(func, i->getSrc(s)));
            i->src(s).get()->reg.data.offset += imm.reg.data.u32;
         } else if (insn->op == OP_SUB && !isFloatType(insn->dType)) {
            if (insn->src(0).getFile() != targ->nativeFile(FILE_ADDRESS) ||
                !insn->src(1).getImmediate(imm) ||
                !targ->insnCanLoadOffset(i, s, -imm.reg.data.s32))
               continue;
            i->setIndirect(s, 0, insn->getSrc(0));
            i->setSrc(s, cloneShallow(func, i->getSrc(s)));
            i->src(s).get()->reg.data.offset -= imm.reg.data.u32;
         } else if (insn->op == OP_MOV) {
            if (!insn->src(0).getImmediate(imm) ||
                !targ->insnCanLoadOffset(i, s, imm.reg.data.s32))
               continue;
            i->setIndirect(s, 0, NULL);
            i->setSrc(s, cloneShallow(func, i->getSrc(s)));
            i->src(s).get()->reg.data.offset += imm.reg.data.u32;
         } else if (insn->op == OP_SHLADD) {
            if (!insn->src(2).getImmediate(imm) ||
                !targ->insnCanLoadOffset(i, s, imm.reg.data.s32))
               continue;
            i->setIndirect(s, 0, bld.mkOp2v(
               OP_SHL, TYPE_U32, bld.getSSA(), insn->getSrc(0), insn->getSrc(1)));
            i->setSrc(s, cloneShallow(func, i->getSrc(s)));
            i->src(s).get()->reg.data.offset += imm.reg.data.u32;
         }
      }
   }
   return true;
}

// =============================================================================

// Evaluate constant expressions.
class ConstantFolding : public Pass
{
public:
   ConstantFolding() : foldCount(0) {}
   bool foldAll(Program *);

private:
   virtual bool visit(BasicBlock *);

   void expr(Instruction *, ImmediateValue&, ImmediateValue&);
   void expr(Instruction *, ImmediateValue&, ImmediateValue&, ImmediateValue&);
   /* true if i was deleted */
   bool opnd(Instruction *i, ImmediateValue&, int s);
   void opnd3(Instruction *, ImmediateValue&);

   void unary(Instruction *, const ImmediateValue&);

   void tryCollapseChainedMULs(Instruction *, const int s, ImmediateValue&);

   CmpInstruction *findOriginForTestWithZero(Value *);

   bool createMul(DataType ty, Value *def, Value *a, int64_t b, Value *c);

   unsigned int foldCount;

   BuildUtil bld;
};

// TODO: remember generated immediates and only revisit these
bool
ConstantFolding::foldAll(Program *prog)
{
   unsigned int iterCount = 0;
   do {
      foldCount = 0;
      if (!run(prog))
         return false;
   } while (foldCount && ++iterCount < 2);
   return true;
}

bool
ConstantFolding::visit(BasicBlock *bb)
{
   Instruction *i, *next;

   for (i = bb->getEntry(); i; i = next) {
      next = i->next;
      if (i->op == OP_MOV || i->op == OP_CALL)
         continue;

      ImmediateValue src0, src1, src2;

      if (i->srcExists(2) &&
          i->src(0).getImmediate(src0) &&
          i->src(1).getImmediate(src1) &&
          i->src(2).getImmediate(src2)) {
         expr(i, src0, src1, src2);
      } else
      if (i->srcExists(1) &&
          i->src(0).getImmediate(src0) && i->src(1).getImmediate(src1)) {
         expr(i, src0, src1);
      } else
      if (i->srcExists(0) && i->src(0).getImmediate(src0)) {
         if (opnd(i, src0, 0))
            continue;
      } else
      if (i->srcExists(1) && i->src(1).getImmediate(src1)) {
         if (opnd(i, src1, 1))
            continue;
      }
      if (i->srcExists(2) && i->src(2).getImmediate(src2))
         opnd3(i, src2);
   }
   return true;
}

CmpInstruction *
ConstantFolding::findOriginForTestWithZero(Value *value)
{
   if (!value)
      return NULL;
   Instruction *insn = value->getInsn();
   if (!insn)
      return NULL;

   if (insn->asCmp() && insn->op != OP_SLCT)
      return insn->asCmp();

   /* Sometimes mov's will sneak in as a result of other folding. This gets
    * cleaned up later.
    */
   if (insn->op == OP_MOV)
      return findOriginForTestWithZero(insn->getSrc(0));

   /* Deal with AND 1.0 here since nv50 can't fold into boolean float */
   if (insn->op == OP_AND) {
      int s = 0;
      ImmediateValue imm;
      if (!insn->src(s).getImmediate(imm)) {
         s = 1;
         if (!insn->src(s).getImmediate(imm))
            return NULL;
      }
      if (imm.reg.data.f32 != 1.0f)
         return NULL;
      /* TODO: Come up with a way to handle the condition being inverted */
      if (insn->src(!s).mod != Modifier(0))
         return NULL;
      return findOriginForTestWithZero(insn->getSrc(!s));
   }

   return NULL;
}

void
Modifier::applyTo(ImmediateValue& imm) const
{
   if (!bits) // avoid failure if imm.reg.type is unhandled (e.g. b128)
      return;
   switch (imm.reg.type) {
   case TYPE_F32:
      if (bits & NV50_IR_MOD_ABS)
         imm.reg.data.f32 = fabsf(imm.reg.data.f32);
      if (bits & NV50_IR_MOD_NEG)
         imm.reg.data.f32 = -imm.reg.data.f32;
      if (bits & NV50_IR_MOD_SAT) {
         if (imm.reg.data.f32 < 0.0f)
            imm.reg.data.f32 = 0.0f;
         else
         if (imm.reg.data.f32 > 1.0f)
            imm.reg.data.f32 = 1.0f;
      }
      assert(!(bits & NV50_IR_MOD_NOT));
      break;

   case TYPE_S8: // NOTE: will be extended
   case TYPE_S16:
   case TYPE_S32:
   case TYPE_U8: // NOTE: treated as signed
   case TYPE_U16:
   case TYPE_U32:
      if (bits & NV50_IR_MOD_ABS)
         imm.reg.data.s32 = (imm.reg.data.s32 >= 0) ?
            imm.reg.data.s32 : -imm.reg.data.s32;
      if (bits & NV50_IR_MOD_NEG)
         imm.reg.data.s32 = -imm.reg.data.s32;
      if (bits & NV50_IR_MOD_NOT)
         imm.reg.data.s32 = ~imm.reg.data.s32;
      break;

   case TYPE_F64:
      if (bits & NV50_IR_MOD_ABS)
         imm.reg.data.f64 = fabs(imm.reg.data.f64);
      if (bits & NV50_IR_MOD_NEG)
         imm.reg.data.f64 = -imm.reg.data.f64;
      if (bits & NV50_IR_MOD_SAT) {
         if (imm.reg.data.f64 < 0.0)
            imm.reg.data.f64 = 0.0;
         else
         if (imm.reg.data.f64 > 1.0)
            imm.reg.data.f64 = 1.0;
      }
      assert(!(bits & NV50_IR_MOD_NOT));
      break;

   default:
      assert(!"invalid/unhandled type");
      imm.reg.data.u64 = 0;
      break;
   }
}

operation
Modifier::getOp() const
{
   switch (bits) {
   case NV50_IR_MOD_ABS: return OP_ABS;
   case NV50_IR_MOD_NEG: return OP_NEG;
   case NV50_IR_MOD_SAT: return OP_SAT;
   case NV50_IR_MOD_NOT: return OP_NOT;
   case 0:
      return OP_MOV;
   default:
      return OP_CVT;
   }
}

void
ConstantFolding::expr(Instruction *i,
                      ImmediateValue &imm0, ImmediateValue &imm1)
{
   struct Storage *const a = &imm0.reg, *const b = &imm1.reg;
   struct Storage res;
   DataType type = i->dType;

   memset(&res.data, 0, sizeof(res.data));

   switch (i->op) {
   case OP_SGXT: {
      int bits = b->data.u32;
      if (bits) {
         uint32_t data = a->data.u32 & (0xffffffff >> (32 - bits));
         if (bits < 32 && (data & (1 << (bits - 1))))
            data = data - (1 << bits);
         res.data.u32 = data;
      }
      break;
   }
   case OP_BMSK:
      res.data.u32 = ((1 << b->data.u32) - 1) << a->data.u32;
      break;
   case OP_MAD:
   case OP_FMA:
   case OP_MUL:
      if (i->dnz && i->dType == TYPE_F32) {
         if (!isfinite(a->data.f32))
            a->data.f32 = 0.0f;
         if (!isfinite(b->data.f32))
            b->data.f32 = 0.0f;
      }
      switch (i->dType) {
      case TYPE_F32:
         res.data.f32 = a->data.f32 * b->data.f32 * exp2f(i->postFactor);
         break;
      case TYPE_F64: res.data.f64 = a->data.f64 * b->data.f64; break;
      case TYPE_S32:
         if (i->subOp == NV50_IR_SUBOP_MUL_HIGH) {
            res.data.s32 = ((int64_t)a->data.s32 * b->data.s32) >> 32;
            break;
         }
         FALLTHROUGH;
      case TYPE_U32:
         if (i->subOp == NV50_IR_SUBOP_MUL_HIGH) {
            res.data.u32 = ((uint64_t)a->data.u32 * b->data.u32) >> 32;
            break;
         }
         res.data.u32 = a->data.u32 * b->data.u32; break;
      default:
         return;
      }
      break;
   case OP_DIV:
      if (b->data.u32 == 0)
         break;
      switch (i->dType) {
      case TYPE_F32: res.data.f32 = a->data.f32 / b->data.f32; break;
      case TYPE_F64: res.data.f64 = a->data.f64 / b->data.f64; break;
      case TYPE_S32: res.data.s32 = a->data.s32 / b->data.s32; break;
      case TYPE_U32: res.data.u32 = a->data.u32 / b->data.u32; break;
      default:
         return;
      }
      break;
   case OP_ADD:
      switch (i->dType) {
      case TYPE_F32: res.data.f32 = a->data.f32 + b->data.f32; break;
      case TYPE_F64: res.data.f64 = a->data.f64 + b->data.f64; break;
      case TYPE_S32:
      case TYPE_U32: res.data.u32 = a->data.u32 + b->data.u32; break;
      default:
         return;
      }
      break;
   case OP_SUB:
      switch (i->dType) {
      case TYPE_F32: res.data.f32 = a->data.f32 - b->data.f32; break;
      case TYPE_F64: res.data.f64 = a->data.f64 - b->data.f64; break;
      case TYPE_S32:
      case TYPE_U32: res.data.u32 = a->data.u32 - b->data.u32; break;
      default:
         return;
      }
      break;
   case OP_MAX:
      switch (i->dType) {
      case TYPE_F32: res.data.f32 = MAX2(a->data.f32, b->data.f32); break;
      case TYPE_F64: res.data.f64 = MAX2(a->data.f64, b->data.f64); break;
      case TYPE_S32: res.data.s32 = MAX2(a->data.s32, b->data.s32); break;
      case TYPE_U32: res.data.u32 = MAX2(a->data.u32, b->data.u32); break;
      default:
         return;
      }
      break;
   case OP_MIN:
      switch (i->dType) {
      case TYPE_F32: res.data.f32 = MIN2(a->data.f32, b->data.f32); break;
      case TYPE_F64: res.data.f64 = MIN2(a->data.f64, b->data.f64); break;
      case TYPE_S32: res.data.s32 = MIN2(a->data.s32, b->data.s32); break;
      case TYPE_U32: res.data.u32 = MIN2(a->data.u32, b->data.u32); break;
      default:
         return;
      }
      break;
   case OP_AND:
      res.data.u64 = a->data.u64 & b->data.u64;
      break;
   case OP_OR:
      res.data.u64 = a->data.u64 | b->data.u64;
      break;
   case OP_XOR:
      res.data.u64 = a->data.u64 ^ b->data.u64;
      break;
   case OP_SHL:
      res.data.u32 = a->data.u32 << b->data.u32;
      break;
   case OP_SHR:
      switch (i->dType) {
      case TYPE_S32: res.data.s32 = a->data.s32 >> b->data.u32; break;
      case TYPE_U32: res.data.u32 = a->data.u32 >> b->data.u32; break;
      default:
         return;
      }
      break;
   case OP_SLCT:
      if (a->data.u32 != b->data.u32)
         return;
      res.data.u32 = a->data.u32;
      break;
   case OP_EXTBF: {
      int offset = b->data.u32 & 0xff;
      int width = (b->data.u32 >> 8) & 0xff;
      int rshift = offset;
      int lshift = 0;
      if (width == 0) {
         res.data.u32 = 0;
         break;
      }
      if (width + offset < 32) {
         rshift = 32 - width;
         lshift = 32 - width - offset;
      }
      if (i->subOp == NV50_IR_SUBOP_EXTBF_REV)
         res.data.u32 = util_bitreverse(a->data.u32);
      else
         res.data.u32 = a->data.u32;
      switch (i->dType) {
      case TYPE_S32: res.data.s32 = (res.data.s32 << lshift) >> rshift; break;
      case TYPE_U32: res.data.u32 = (res.data.u32 << lshift) >> rshift; break;
      default:
         return;
      }
      break;
   }
   case OP_POPCNT:
      res.data.u32 = util_bitcount(a->data.u32 & b->data.u32);
      break;
   case OP_PFETCH:
      // The two arguments to pfetch are logically added together. Normally
      // the second argument will not be constant, but that can happen.
      res.data.u32 = a->data.u32 + b->data.u32;
      type = TYPE_U32;
      break;
   case OP_MERGE:
      switch (i->dType) {
      case TYPE_U64:
      case TYPE_S64:
      case TYPE_F64:
         res.data.u64 = (((uint64_t)b->data.u32) << 32) | a->data.u32;
         break;
      default:
         return;
      }
      break;
   default:
      return;
   }
   ++foldCount;

   i->src(0).mod = Modifier(0);
   i->src(1).mod = Modifier(0);
   i->postFactor = 0;

   i->setSrc(0, new_ImmediateValue(i->bb->getProgram(), res.data.u32));
   i->setSrc(1, NULL);

   i->getSrc(0)->reg.data = res.data;
   i->getSrc(0)->reg.type = type;
   i->getSrc(0)->reg.size = typeSizeof(type);

   switch (i->op) {
   case OP_MAD:
   case OP_FMA: {
      ImmediateValue src0, src1;
      src1 = *i->getSrc(0)->asImm();

      // Move the immediate into position 1, where we know it might be
      // emittable. However it might not be anyways, as there may be other
      // restrictions, so move it into a separate LValue.
      bld.setPosition(i, false);
      i->op = OP_ADD;
      i->dnz = 0;
      i->setSrc(1, bld.mkMov(bld.getSSA(type), i->getSrc(0), type)->getDef(0));
      i->setSrc(0, i->getSrc(2));
      i->src(0).mod = i->src(2).mod;
      i->setSrc(2, NULL);

      if (i->src(0).getImmediate(src0))
         expr(i, src0, src1);
      else
         opnd(i, src1, 1);
      break;
   }
   case OP_PFETCH:
      // Leave PFETCH alone... we just folded its 2 args into 1.
      break;
   default:
      i->op = i->saturate ? OP_SAT : OP_MOV;
      if (i->saturate)
         unary(i, *i->getSrc(0)->asImm());
      break;
   }
   i->subOp = 0;
}

void
ConstantFolding::expr(Instruction *i,
                      ImmediateValue &imm0,
                      ImmediateValue &imm1,
                      ImmediateValue &imm2)
{
   struct Storage *const a = &imm0.reg, *const b = &imm1.reg, *const c = &imm2.reg;
   struct Storage res;

   memset(&res.data, 0, sizeof(res.data));

   switch (i->op) {
   case OP_LOP3_LUT:
      for (int n = 0; n < 32; n++) {
         uint8_t lut = ((a->data.u32 >> n) & 1) << 2 |
                       ((b->data.u32 >> n) & 1) << 1 |
                       ((c->data.u32 >> n) & 1);
         res.data.u32 |= !!(i->subOp & (1 << lut)) << n;
      }
      break;
   case OP_PERMT:
      if (!i->subOp) {
         uint64_t input = (uint64_t)c->data.u32 << 32 | a->data.u32;
         uint16_t permt = b->data.u32;
         for (int n = 0 ; n < 4; n++, permt >>= 4)
            res.data.u32 |= ((input >> ((permt & 0xf) * 8)) & 0xff) << n * 8;
      } else
         return;
      break;
   case OP_INSBF: {
      int offset = b->data.u32 & 0xff;
      int width = (b->data.u32 >> 8) & 0xff;
      unsigned bitmask = ((1 << width) - 1) << offset;
      res.data.u32 = ((a->data.u32 << offset) & bitmask) | (c->data.u32 & ~bitmask);
      break;
   }
   case OP_MAD:
   case OP_FMA: {
      switch (i->dType) {
      case TYPE_F32:
         res.data.f32 = a->data.f32 * b->data.f32 * exp2f(i->postFactor) +
            c->data.f32;
         break;
      case TYPE_F64:
         res.data.f64 = a->data.f64 * b->data.f64 + c->data.f64;
         break;
      case TYPE_S32:
         if (i->subOp == NV50_IR_SUBOP_MUL_HIGH) {
            res.data.s32 = ((int64_t)a->data.s32 * b->data.s32 >> 32) + c->data.s32;
            break;
         }
         FALLTHROUGH;
      case TYPE_U32:
         if (i->subOp == NV50_IR_SUBOP_MUL_HIGH) {
            res.data.u32 = ((uint64_t)a->data.u32 * b->data.u32 >> 32) + c->data.u32;
            break;
         }
         res.data.u32 = a->data.u32 * b->data.u32 + c->data.u32;
         break;
      default:
         return;
      }
      break;
   }
   case OP_SHLADD:
      res.data.u32 = (a->data.u32 << b->data.u32) + c->data.u32;
      break;
   default:
      return;
   }

   ++foldCount;
   i->src(0).mod = Modifier(0);
   i->src(1).mod = Modifier(0);
   i->src(2).mod = Modifier(0);

   i->setSrc(0, new_ImmediateValue(i->bb->getProgram(), res.data.u32));
   i->setSrc(1, NULL);
   i->setSrc(2, NULL);

   i->getSrc(0)->reg.data = res.data;
   i->getSrc(0)->reg.type = i->dType;
   i->getSrc(0)->reg.size = typeSizeof(i->dType);

   i->op = OP_MOV;
}

void
ConstantFolding::unary(Instruction *i, const ImmediateValue &imm)
{
   Storage res;

   if (i->dType != TYPE_F32)
      return;
   switch (i->op) {
   case OP_NEG: res.data.f32 = -imm.reg.data.f32; break;
   case OP_ABS: res.data.f32 = fabsf(imm.reg.data.f32); break;
   case OP_SAT: res.data.f32 = SATURATE(imm.reg.data.f32); break;
   case OP_RCP: res.data.f32 = 1.0f / imm.reg.data.f32; break;
   case OP_RSQ: res.data.f32 = 1.0f / sqrtf(imm.reg.data.f32); break;
   case OP_LG2: res.data.f32 = log2f(imm.reg.data.f32); break;
   case OP_EX2: res.data.f32 = exp2f(imm.reg.data.f32); break;
   case OP_SIN: res.data.f32 = sinf(imm.reg.data.f32); break;
   case OP_COS: res.data.f32 = cosf(imm.reg.data.f32); break;
   case OP_SQRT: res.data.f32 = sqrtf(imm.reg.data.f32); break;
   case OP_PRESIN:
   case OP_PREEX2:
      // these should be handled in subsequent OP_SIN/COS/EX2
      res.data.f32 = imm.reg.data.f32;
      break;
   default:
      return;
   }
   i->op = OP_MOV;
   i->setSrc(0, new_ImmediateValue(i->bb->getProgram(), res.data.f32));
   i->src(0).mod = Modifier(0);
}

void
ConstantFolding::tryCollapseChainedMULs(Instruction *mul2,
                                        const int s, ImmediateValue& imm2)
{
   const int t = s ? 0 : 1;
   Instruction *insn;
   Instruction *mul1 = NULL; // mul1 before mul2
   int e = 0;
   float f = imm2.reg.data.f32 * exp2f(mul2->postFactor);
   ImmediateValue imm1;

   assert(mul2->op == OP_MUL && mul2->dType == TYPE_F32);

   if (mul2->getSrc(t)->refCount() == 1) {
      insn = mul2->getSrc(t)->getInsn();
      if (!mul2->src(t).mod && insn->op == OP_MUL && insn->dType == TYPE_F32)
         mul1 = insn;
      if (mul1 && !mul1->saturate) {
         int s1;

         if (mul1->src(s1 = 0).getImmediate(imm1) ||
             mul1->src(s1 = 1).getImmediate(imm1)) {
            bld.setPosition(mul1, false);
            // a = mul r, imm1
            // d = mul a, imm2 -> d = mul r, (imm1 * imm2)
            mul1->setSrc(s1, bld.loadImm(NULL, f * imm1.reg.data.f32));
            mul1->src(s1).mod = Modifier(0);
            mul2->def(0).replace(mul1->getDef(0), false);
            mul1->saturate = mul2->saturate;
         } else
         if (prog->getTarget()->isPostMultiplySupported(OP_MUL, f, e)) {
            // c = mul a, b
            // d = mul c, imm   -> d = mul_x_imm a, b
            mul1->postFactor = e;
            mul2->def(0).replace(mul1->getDef(0), false);
            if (f < 0)
               mul1->src(0).mod *= Modifier(NV50_IR_MOD_NEG);
            mul1->saturate = mul2->saturate;
         }
         return;
      }
   }
   if (mul2->getDef(0)->refCount() == 1 && !mul2->saturate) {
      // b = mul a, imm
      // d = mul b, c   -> d = mul_x_imm a, c
      int s2, t2;
      insn = (*mul2->getDef(0)->uses.begin())->getInsn();
      if (!insn)
         return;
      mul1 = mul2;
      mul2 = NULL;
      s2 = insn->getSrc(0) == mul1->getDef(0) ? 0 : 1;
      t2 = s2 ? 0 : 1;
      if (insn->op == OP_MUL && insn->dType == TYPE_F32)
         if (!insn->src(s2).mod && !insn->src(t2).getImmediate(imm1))
            mul2 = insn;
      if (mul2 && prog->getTarget()->isPostMultiplySupported(OP_MUL, f, e)) {
         mul2->postFactor = e;
         mul2->setSrc(s2, mul1->src(t));
         if (f < 0)
            mul2->src(s2).mod *= Modifier(NV50_IR_MOD_NEG);
      }
   }
}

void
ConstantFolding::opnd3(Instruction *i, ImmediateValue &imm2)
{
   switch (i->op) {
   case OP_MAD:
   case OP_FMA:
      if (imm2.isInteger(0)) {
         i->op = OP_MUL;
         i->setSrc(2, NULL);
         foldCount++;
         return;
      }
      break;
   case OP_SHLADD:
      if (imm2.isInteger(0)) {
         i->op = OP_SHL;
         i->setSrc(2, NULL);
         foldCount++;
         return;
      }
      break;
   default:
      return;
   }
}

bool
ConstantFolding::createMul(DataType ty, Value *def, Value *a, int64_t b, Value *c)
{
   const Target *target = prog->getTarget();
   int64_t absB = llabs(b);

   //a * (2^shl) -> a << shl
   if (b >= 0 && util_is_power_of_two_or_zero64(b)) {
      int shl = util_logbase2_64(b);

      Value *res = c ? bld.getSSA(typeSizeof(ty)) : def;
      bld.mkOp2(OP_SHL, ty, res, a, bld.mkImm(shl));
      if (c)
         bld.mkOp2(OP_ADD, ty, def, res, c);

      return true;
   }

   //a * (2^shl + 1) -> a << shl + a
   //a * -(2^shl + 1) -> -a << shl + a
   //a * (2^shl - 1) -> a << shl - a
   //a * -(2^shl - 1) -> -a << shl - a
   if (typeSizeof(ty) == 4 &&
       (util_is_power_of_two_or_zero64(absB - 1) ||
        util_is_power_of_two_or_zero64(absB + 1)) &&
       target->isOpSupported(OP_SHLADD, TYPE_U32)) {
      bool subA = util_is_power_of_two_or_zero64(absB + 1);
      int shl = subA ? util_logbase2_64(absB + 1) : util_logbase2_64(absB - 1);

      Value *res = c ? bld.getSSA() : def;
      Instruction *insn = bld.mkOp3(OP_SHLADD, TYPE_U32, res, a, bld.mkImm(shl), a);
      if (b < 0)
         insn->src(0).mod = Modifier(NV50_IR_MOD_NEG);
      if (subA)
         insn->src(2).mod = Modifier(NV50_IR_MOD_NEG);

      if (c)
         bld.mkOp2(OP_ADD, TYPE_U32, def, res, c);

      return true;
   }

   if (typeSizeof(ty) == 4 && b >= 0 && b <= 0xffff &&
       target->isOpSupported(OP_XMAD, TYPE_U32)) {
      Value *tmp = bld.mkOp3v(OP_XMAD, TYPE_U32, bld.getSSA(),
                              a, bld.mkImm((uint32_t)b), c ? c : bld.mkImm(0));
      bld.mkOp3(OP_XMAD, TYPE_U32, def, a, bld.mkImm((uint32_t)b), tmp)->subOp =
         NV50_IR_SUBOP_XMAD_PSL | NV50_IR_SUBOP_XMAD_H1(0);

      return true;
   }

   return false;
}

bool
ConstantFolding::opnd(Instruction *i, ImmediateValue &imm0, int s)
{
   const int t = !s;
   const operation op = i->op;
   Instruction *newi = i;
   bool deleted = false;

   switch (i->op) {
   case OP_SPLIT: {
      bld.setPosition(i, false);

      uint8_t size = i->getDef(0)->reg.size;
      uint8_t bitsize = size * 8;
      uint32_t mask = (1ULL << bitsize) - 1;
      assert(bitsize <= 32);

      uint64_t val = imm0.reg.data.u64;
      for (int8_t d = 0; i->defExists(d); ++d) {
         Value *def = i->getDef(d);
         assert(def->reg.size == size);

         newi = bld.mkMov(def, bld.mkImm((uint32_t)(val & mask)),
                          typeOfSize(size));
         val >>= bitsize;
      }
      delete_Instruction(prog, i);
      deleted = true;
      break;
   }
   case OP_MUL:
      if (i->dType == TYPE_F32 && !i->precise)
         tryCollapseChainedMULs(i, s, imm0);

      if (i->subOp == NV50_IR_SUBOP_MUL_HIGH) {
         assert(!isFloatType(i->sType));
         if (imm0.isInteger(1) && i->dType == TYPE_S32) {
            bld.setPosition(i, false);
            // Need to set to the sign value, which is a compare.
            newi = bld.mkCmp(OP_SET, CC_LT, TYPE_S32, i->getDef(0),
                             TYPE_S32, i->getSrc(t), bld.mkImm(0));
            delete_Instruction(prog, i);
            deleted = true;
         } else if (imm0.isInteger(0) || imm0.isInteger(1)) {
            // The high bits can't be set in this case (either mul by 0 or
            // unsigned by 1)
            i->op = OP_MOV;
            i->subOp = 0;
            i->setSrc(0, new_ImmediateValue(prog, 0u));
            i->src(0).mod = Modifier(0);
            i->setSrc(1, NULL);
         } else if (!imm0.isNegative() && imm0.isPow2()) {
            // Translate into a shift
            imm0.applyLog2();
            i->op = OP_SHR;
            i->subOp = 0;
            imm0.reg.data.u32 = 32 - imm0.reg.data.u32;
            i->setSrc(0, i->getSrc(t));
            i->src(0).mod = i->src(t).mod;
            i->setSrc(1, new_ImmediateValue(prog, imm0.reg.data.u32));
            i->src(1).mod = 0;
         }
      } else
      if (imm0.isInteger(0)) {
         i->dnz = 0;
         i->op = OP_MOV;
         i->setSrc(0, new_ImmediateValue(prog, 0u));
         i->src(0).mod = Modifier(0);
         i->postFactor = 0;
         i->setSrc(1, NULL);
      } else
      if (!i->postFactor && (imm0.isInteger(1) || imm0.isInteger(-1))) {
         if (imm0.isNegative())
            i->src(t).mod = i->src(t).mod ^ Modifier(NV50_IR_MOD_NEG);
         i->dnz = 0;
         i->op = i->src(t).mod.getOp();
         if (s == 0) {
            i->setSrc(0, i->getSrc(1));
            i->src(0).mod = i->src(1).mod;
            i->src(1).mod = 0;
         }
         if (i->op != OP_CVT)
            i->src(0).mod = 0;
         i->setSrc(1, NULL);
      } else
      if (!i->postFactor && (imm0.isInteger(2) || imm0.isInteger(-2))) {
         if (imm0.isNegative())
            i->src(t).mod = i->src(t).mod ^ Modifier(NV50_IR_MOD_NEG);
         i->op = OP_ADD;
         i->dnz = 0;
         i->setSrc(s, i->getSrc(t));
         i->src(s).mod = i->src(t).mod;
      } else
      if (!isFloatType(i->dType) && !i->src(t).mod) {
         bld.setPosition(i, false);
         int64_t b = typeSizeof(i->dType) == 8 ? imm0.reg.data.s64 : imm0.reg.data.s32;
         if (createMul(i->dType, i->getDef(0), i->getSrc(t), b, NULL)) {
            delete_Instruction(prog, i);
            deleted = true;
         }
      } else
      if (i->postFactor && i->sType == TYPE_F32) {
         /* Can't emit a postfactor with an immediate, have to fold it in */
         i->setSrc(s, new_ImmediateValue(
                      prog, imm0.reg.data.f32 * exp2f(i->postFactor)));
         i->postFactor = 0;
      }
      break;
   case OP_FMA:
   case OP_MAD:
      if (imm0.isInteger(0)) {
         i->setSrc(0, i->getSrc(2));
         i->src(0).mod = i->src(2).mod;
         i->setSrc(1, NULL);
         i->setSrc(2, NULL);
         i->dnz = 0;
         i->op = i->src(0).mod.getOp();
         if (i->op != OP_CVT)
            i->src(0).mod = 0;
      } else
      if (i->subOp != NV50_IR_SUBOP_MUL_HIGH &&
          (imm0.isInteger(1) || imm0.isInteger(-1))) {
         if (imm0.isNegative())
            i->src(t).mod = i->src(t).mod ^ Modifier(NV50_IR_MOD_NEG);
         if (s == 0) {
            i->setSrc(0, i->getSrc(1));
            i->src(0).mod = i->src(1).mod;
         }
         i->setSrc(1, i->getSrc(2));
         i->src(1).mod = i->src(2).mod;
         i->setSrc(2, NULL);
         i->dnz = 0;
         i->op = OP_ADD;
      } else
      if (!isFloatType(i->dType) && !i->subOp && !i->src(t).mod && !i->src(2).mod) {
         bld.setPosition(i, false);
         int64_t b = typeSizeof(i->dType) == 8 ? imm0.reg.data.s64 : imm0.reg.data.s32;
         if (createMul(i->dType, i->getDef(0), i->getSrc(t), b, i->getSrc(2))) {
            delete_Instruction(prog, i);
            deleted = true;
         }
      }
      break;
   case OP_SUB:
      if (imm0.isInteger(0) && s == 0 && typeSizeof(i->dType) == 8 &&
          !isFloatType(i->dType))
         break;
      FALLTHROUGH;
   case OP_ADD:
      if (i->usesFlags())
         break;
      if (imm0.isInteger(0)) {
         if (s == 0) {
            i->setSrc(0, i->getSrc(1));
            i->src(0).mod = i->src(1).mod;
            if (i->op == OP_SUB)
               i->src(0).mod = i->src(0).mod ^ Modifier(NV50_IR_MOD_NEG);
         }
         i->setSrc(1, NULL);
         i->op = i->src(0).mod.getOp();
         if (i->op != OP_CVT)
            i->src(0).mod = Modifier(0);
      }
      break;

   case OP_DIV:
      if (s != 1 || (i->dType != TYPE_S32 && i->dType != TYPE_U32))
         break;
      bld.setPosition(i, false);
      if (imm0.reg.data.u32 == 0) {
         break;
      } else
      if (imm0.reg.data.u32 == 1) {
         i->op = OP_MOV;
         i->setSrc(1, NULL);
      } else
      if (i->dType == TYPE_U32 && imm0.isPow2()) {
         i->op = OP_SHR;
         i->setSrc(1, bld.mkImm(util_logbase2(imm0.reg.data.u32)));
      } else
      if (i->dType == TYPE_U32) {
         Instruction *mul;
         Value *tA, *tB;
         const uint32_t d = imm0.reg.data.u32;
         uint32_t m;
         int r, s;
         uint32_t l = util_logbase2(d);
         if (((uint32_t)1 << l) < d)
            ++l;
         m = (((uint64_t)1 << 32) * (((uint64_t)1 << l) - d)) / d + 1;
         r = l ? 1 : 0;
         s = l ? (l - 1) : 0;

         tA = bld.getSSA();
         tB = bld.getSSA();
         mul = bld.mkOp2(OP_MUL, TYPE_U32, tA, i->getSrc(0),
                         bld.loadImm(NULL, m));
         mul->subOp = NV50_IR_SUBOP_MUL_HIGH;
         bld.mkOp2(OP_SUB, TYPE_U32, tB, i->getSrc(0), tA);
         tA = bld.getSSA();
         if (r)
            bld.mkOp2(OP_SHR, TYPE_U32, tA, tB, bld.mkImm(r));
         else
            tA = tB;
         tB = s ? bld.getSSA() : i->getDef(0);
         newi = bld.mkOp2(OP_ADD, TYPE_U32, tB, mul->getDef(0), tA);
         if (s)
            bld.mkOp2(OP_SHR, TYPE_U32, i->getDef(0), tB, bld.mkImm(s));

         delete_Instruction(prog, i);
         deleted = true;
      } else
      if (imm0.reg.data.s32 == -1) {
         i->op = OP_NEG;
         i->setSrc(1, NULL);
      } else {
         LValue *tA, *tB;
         LValue *tD;
         const int32_t d = imm0.reg.data.s32;
         int32_t m;
         int32_t l = util_logbase2(static_cast<unsigned>(abs(d)));
         if ((1 << l) < abs(d))
            ++l;
         if (!l)
            l = 1;
         m = ((uint64_t)1 << (32 + l - 1)) / abs(d) + 1 - ((uint64_t)1 << 32);

         tA = bld.getSSA();
         tB = bld.getSSA();
         bld.mkOp3(OP_MAD, TYPE_S32, tA, i->getSrc(0), bld.loadImm(NULL, m),
                   i->getSrc(0))->subOp = NV50_IR_SUBOP_MUL_HIGH;
         if (l > 1)
            bld.mkOp2(OP_SHR, TYPE_S32, tB, tA, bld.mkImm(l - 1));
         else
            tB = tA;
         tA = bld.getSSA();
         bld.mkCmp(OP_SET, CC_LT, TYPE_S32, tA, TYPE_S32, i->getSrc(0), bld.mkImm(0));
         tD = (d < 0) ? bld.getSSA() : i->getDef(0)->asLValue();
         newi = bld.mkOp2(OP_SUB, TYPE_U32, tD, tB, tA);
         if (d < 0)
            bld.mkOp1(OP_NEG, TYPE_S32, i->getDef(0), tB);

         delete_Instruction(prog, i);
         deleted = true;
      }
      break;

   case OP_MOD:
      if (s == 1 && imm0.isPow2()) {
         bld.setPosition(i, false);
         if (i->sType == TYPE_U32) {
            i->op = OP_AND;
            i->setSrc(1, bld.loadImm(NULL, imm0.reg.data.u32 - 1));
         } else if (i->sType == TYPE_S32) {
            // Do it on the absolute value of the input, and then restore the
            // sign. The only odd case is MIN_INT, but that should work out
            // as well, since MIN_INT mod any power of 2 is 0.
            //
            // Technically we don't have to do any of this since MOD is
            // undefined with negative arguments in GLSL, but this seems like
            // the nice thing to do.
            Value *abs = bld.mkOp1v(OP_ABS, TYPE_S32, bld.getSSA(), i->getSrc(0));
            Value *neg, *v1, *v2;
            bld.mkCmp(OP_SET, CC_LT, TYPE_S32,
                      (neg = bld.getSSA(1, prog->getTarget()->nativeFile(FILE_PREDICATE))),
                      TYPE_S32, i->getSrc(0), bld.loadImm(NULL, 0));
            Value *mod = bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(), abs,
                                    bld.loadImm(NULL, imm0.reg.data.u32 - 1));
            bld.mkOp1(OP_NEG, TYPE_S32, (v1 = bld.getSSA()), mod)
               ->setPredicate(CC_P, neg);
            bld.mkOp1(OP_MOV, TYPE_S32, (v2 = bld.getSSA()), mod)
               ->setPredicate(CC_NOT_P, neg);
            newi = bld.mkOp2(OP_UNION, TYPE_S32, i->getDef(0), v1, v2);

            delete_Instruction(prog, i);
            deleted = true;
         }
      } else if (s == 1) {
         // In this case, we still want the optimized lowering that we get
         // from having division by an immediate.
         //
         // a % b == a - (a/b) * b
         bld.setPosition(i, false);
         Value *div = bld.mkOp2v(OP_DIV, i->sType, bld.getSSA(),
                                 i->getSrc(0), i->getSrc(1));
         newi = bld.mkOp2(OP_ADD, i->sType, i->getDef(0), i->getSrc(0),
                          bld.mkOp2v(OP_MUL, i->sType, bld.getSSA(), div, i->getSrc(1)));
         // TODO: Check that target supports this. In this case, we know that
         // all backends do.
         newi->src(1).mod = Modifier(NV50_IR_MOD_NEG);

         delete_Instruction(prog, i);
         deleted = true;
      }
      break;

   case OP_SET: // TODO: SET_AND,OR,XOR
   {
      /* This optimizes the case where the output of a set is being compared
       * to zero. Since the set can only produce 0/-1 (int) or 0/1 (float), we
       * can be a lot cleverer in our comparison.
       */
      CmpInstruction *si = findOriginForTestWithZero(i->getSrc(t));
      CondCode cc, ccZ;
      if (imm0.reg.data.u32 != 0 || !si)
         return false;
      cc = si->setCond;
      ccZ = (CondCode)((unsigned int)i->asCmp()->setCond & ~CC_U);
      // We do everything assuming var (cmp) 0, reverse the condition if 0 is
      // first.
      if (s == 0)
         ccZ = reverseCondCode(ccZ);
      // If there is a negative modifier, we need to undo that, by flipping
      // the comparison to zero.
      if (i->src(t).mod.neg())
         ccZ = reverseCondCode(ccZ);
      // If this is a signed comparison, we expect the input to be a regular
      // boolean, i.e. 0/-1. However the rest of the logic assumes that true
      // is positive, so just flip the sign.
      if (i->sType == TYPE_S32) {
         assert(!isFloatType(si->dType));
         ccZ = reverseCondCode(ccZ);
      }
      switch (ccZ) {
      case CC_LT: cc = CC_FL; break; // bool < 0 -- this is never true
      case CC_GE: cc = CC_TR; break; // bool >= 0 -- this is always true
      case CC_EQ: cc = inverseCondCode(cc); break; // bool == 0 -- !bool
      case CC_LE: cc = inverseCondCode(cc); break; // bool <= 0 -- !bool
      case CC_GT: break; // bool > 0 -- bool
      case CC_NE: break; // bool != 0 -- bool
      default:
         return false;
      }

      // Update the condition of this SET to be identical to the origin set,
      // but with the updated condition code. The original SET should get
      // DCE'd, ideally.
      i->op = si->op;
      i->asCmp()->setCond = cc;
      i->setSrc(0, si->src(0));
      i->setSrc(1, si->src(1));
      if (si->srcExists(2))
         i->setSrc(2, si->src(2));
      i->sType = si->sType;
   }
      break;

   case OP_AND:
   {
      Instruction *src = i->getSrc(t)->getInsn();
      ImmediateValue imm1;
      if (imm0.reg.data.u32 == 0) {
         i->op = OP_MOV;
         i->setSrc(0, new_ImmediateValue(prog, 0u));
         i->src(0).mod = Modifier(0);
         i->setSrc(1, NULL);
      } else if (imm0.reg.data.u32 == ~0U) {
         i->op = i->src(t).mod.getOp();
         if (t) {
            i->setSrc(0, i->getSrc(t));
            i->src(0).mod = i->src(t).mod;
         }
         i->setSrc(1, NULL);
      } else if (src->asCmp()) {
         CmpInstruction *cmp = src->asCmp();
         if (!cmp || cmp->op == OP_SLCT || cmp->getDef(0)->refCount() > 1)
            return false;
         if (!prog->getTarget()->isOpSupported(cmp->op, TYPE_F32))
            return false;
         if (imm0.reg.data.f32 != 1.0)
            return false;
         if (cmp->dType != TYPE_U32)
            return false;

         cmp->dType = TYPE_F32;
         if (i->src(t).mod != Modifier(0)) {
            assert(i->src(t).mod == Modifier(NV50_IR_MOD_NOT));
            i->src(t).mod = Modifier(0);
            cmp->setCond = inverseCondCode(cmp->setCond);
         }
         i->op = OP_MOV;
         i->setSrc(s, NULL);
         if (t) {
            i->setSrc(0, i->getSrc(t));
            i->setSrc(t, NULL);
         }
      } else if (prog->getTarget()->isOpSupported(OP_EXTBF, TYPE_U32) &&
                 src->op == OP_SHR &&
                 src->src(1).getImmediate(imm1) &&
                 i->src(t).mod == Modifier(0) &&
                 util_is_power_of_two_or_zero(imm0.reg.data.u32 + 1)) {
         // low byte = offset, high byte = width
         uint32_t ext = (util_last_bit(imm0.reg.data.u32) << 8) | imm1.reg.data.u32;
         i->op = OP_EXTBF;
         i->setSrc(0, src->getSrc(0));
         i->setSrc(1, new_ImmediateValue(prog, ext));
      } else if (src->op == OP_SHL &&
                 src->src(1).getImmediate(imm1) &&
                 i->src(t).mod == Modifier(0) &&
                 util_is_power_of_two_or_zero(~imm0.reg.data.u32 + 1) &&
                 util_last_bit(~imm0.reg.data.u32) <= imm1.reg.data.u32) {
         i->op = OP_MOV;
         i->setSrc(s, NULL);
         if (t) {
            i->setSrc(0, i->getSrc(t));
            i->setSrc(t, NULL);
         }
      }
   }
      break;

   case OP_SHL:
   {
      if (s != 1 || i->src(0).mod != Modifier(0))
         break;

      if (imm0.reg.data.u32 == 0) {
         i->op = OP_MOV;
         i->setSrc(1, NULL);
         break;
      }
      // try to concatenate shifts
      Instruction *si = i->getSrc(0)->getInsn();
      if (!si)
         break;
      ImmediateValue imm1;
      switch (si->op) {
      case OP_SHL:
         if (si->src(1).getImmediate(imm1)) {
            bld.setPosition(i, false);
            i->setSrc(0, si->getSrc(0));
            i->setSrc(1, bld.loadImm(NULL, imm0.reg.data.u32 + imm1.reg.data.u32));
         }
         break;
      case OP_SHR:
         if (si->src(1).getImmediate(imm1) && imm0.reg.data.u32 == imm1.reg.data.u32) {
            bld.setPosition(i, false);
            i->op = OP_AND;
            i->setSrc(0, si->getSrc(0));
            i->setSrc(1, bld.loadImm(NULL, ~((1 << imm0.reg.data.u32) - 1)));
         }
         break;
      case OP_MUL:
         int muls;
         if (isFloatType(si->dType))
            return false;
         if (si->subOp)
            return false;
         if (si->src(1).getImmediate(imm1))
            muls = 1;
         else if (si->src(0).getImmediate(imm1))
            muls = 0;
         else
            return false;

         bld.setPosition(i, false);
         i->op = OP_MUL;
         i->subOp = 0;
         i->dType = si->dType;
         i->sType = si->sType;
         i->setSrc(0, si->getSrc(!muls));
         i->setSrc(1, bld.loadImm(NULL, imm1.reg.data.u32 << imm0.reg.data.u32));
         break;
      case OP_SUB:
      case OP_ADD:
         int adds;
         if (isFloatType(si->dType))
            return false;
         if (si->op != OP_SUB && si->src(0).getImmediate(imm1))
            adds = 0;
         else if (si->src(1).getImmediate(imm1))
            adds = 1;
         else
            return false;
         if (si->src(!adds).mod != Modifier(0))
            return false;
         // SHL(ADD(x, y), z) = ADD(SHL(x, z), SHL(y, z))

         // This is more operations, but if one of x, y is an immediate, then
         // we can get a situation where (a) we can use ISCADD, or (b)
         // propagate the add bit into an indirect load.
         bld.setPosition(i, false);
         i->op = si->op;
         i->setSrc(adds, bld.loadImm(NULL, imm1.reg.data.u32 << imm0.reg.data.u32));
         i->setSrc(!adds, bld.mkOp2v(OP_SHL, i->dType,
                                     bld.getSSA(i->def(0).getSize(), i->def(0).getFile()),
                                     si->getSrc(!adds),
                                     bld.mkImm(imm0.reg.data.u32)));
         break;
      default:
         return false;
      }
   }
      break;

   case OP_ABS:
   case OP_NEG:
   case OP_SAT:
   case OP_LG2:
   case OP_RCP:
   case OP_SQRT:
   case OP_RSQ:
   case OP_PRESIN:
   case OP_SIN:
   case OP_COS:
   case OP_PREEX2:
   case OP_EX2:
      unary(i, imm0);
      break;
   case OP_BFIND: {
      int32_t res;
      switch (i->dType) {
      case TYPE_S32: res = util_last_bit_signed(imm0.reg.data.s32) - 1; break;
      case TYPE_U32: res = util_last_bit(imm0.reg.data.u32) - 1; break;
      default:
         return false;
      }
      if (i->subOp == NV50_IR_SUBOP_BFIND_SAMT && res >= 0)
         res = 31 - res;
      bld.setPosition(i, false); /* make sure bld is init'ed */
      i->setSrc(0, bld.mkImm(res));
      i->setSrc(1, NULL);
      i->op = OP_MOV;
      i->subOp = 0;
      break;
   }
   case OP_BREV: {
      uint32_t res = util_bitreverse(imm0.reg.data.u32);
      i->setSrc(0, new_ImmediateValue(i->bb->getProgram(), res));
      i->op = OP_MOV;
      break;
   }
   case OP_POPCNT: {
      // Only deal with 1-arg POPCNT here
      if (i->srcExists(1))
         break;
      uint32_t res = util_bitcount(imm0.reg.data.u32);
      i->setSrc(0, new_ImmediateValue(i->bb->getProgram(), res));
      i->setSrc(1, NULL);
      i->op = OP_MOV;
      break;
   }
   case OP_CVT: {
      Storage res;

      // TODO: handle 64-bit values properly
      if (typeSizeof(i->dType) == 8 || typeSizeof(i->sType) == 8)
         return false;

      // TODO: handle single byte/word extractions
      if (i->subOp)
         return false;

      bld.setPosition(i, true); /* make sure bld is init'ed */

#define CASE(type, dst, fmin, fmax, imin, imax, umin, umax) \
   case type: \
      switch (i->sType) { \
      case TYPE_F64: \
         res.data.dst = util_iround(i->saturate ? \
                                    CLAMP(imm0.reg.data.f64, fmin, fmax) : \
                                    imm0.reg.data.f64); \
         break; \
      case TYPE_F32: \
         res.data.dst = util_iround(i->saturate ? \
                                    CLAMP(imm0.reg.data.f32, fmin, fmax) : \
                                    imm0.reg.data.f32); \
         break; \
      case TYPE_S32: \
         res.data.dst = i->saturate ? \
                        CLAMP(imm0.reg.data.s32, imin, imax) : \
                        imm0.reg.data.s32; \
         break; \
      case TYPE_U32: \
         res.data.dst = i->saturate ? \
                        CLAMP(imm0.reg.data.u32, umin, umax) : \
                        imm0.reg.data.u32; \
         break; \
      case TYPE_S16: \
         res.data.dst = i->saturate ? \
                        CLAMP(imm0.reg.data.s16, imin, imax) : \
                        imm0.reg.data.s16; \
         break; \
      case TYPE_U16: \
         res.data.dst = i->saturate ? \
                        CLAMP(imm0.reg.data.u16, umin, umax) : \
                        imm0.reg.data.u16; \
         break; \
      default: return false; \
      } \
      i->setSrc(0, bld.mkImm(res.data.dst)); \
      break

      switch(i->dType) {
      CASE(TYPE_U16, u16, 0, UINT16_MAX, 0, UINT16_MAX, 0, UINT16_MAX);
      CASE(TYPE_S16, s16, INT16_MIN, INT16_MAX, INT16_MIN, INT16_MAX, 0, INT16_MAX);
      CASE(TYPE_U32, u32, 0, (float)UINT32_MAX, 0, INT32_MAX, 0, UINT32_MAX);
      CASE(TYPE_S32, s32, (float)INT32_MIN, (float)INT32_MAX, INT32_MIN, INT32_MAX, 0, INT32_MAX);
      case TYPE_F32:
         switch (i->sType) {
         case TYPE_F64:
            res.data.f32 = i->saturate ?
               SATURATE(imm0.reg.data.f64) :
               imm0.reg.data.f64;
            break;
         case TYPE_F32:
            res.data.f32 = i->saturate ?
               SATURATE(imm0.reg.data.f32) :
               imm0.reg.data.f32;
            break;
         case TYPE_U16: res.data.f32 = (float) imm0.reg.data.u16; break;
         case TYPE_U32: res.data.f32 = (float) imm0.reg.data.u32; break;
         case TYPE_S16: res.data.f32 = (float) imm0.reg.data.s16; break;
         case TYPE_S32: res.data.f32 = (float) imm0.reg.data.s32; break;
         default:
            return false;
         }
         i->setSrc(0, bld.mkImm(res.data.f32));
         break;
      case TYPE_F64:
         switch (i->sType) {
         case TYPE_F64:
            res.data.f64 = i->saturate ?
               SATURATE(imm0.reg.data.f64) :
               imm0.reg.data.f64;
            break;
         case TYPE_F32:
            res.data.f64 = i->saturate ?
               SATURATE(imm0.reg.data.f32) :
               imm0.reg.data.f32;
            break;
         case TYPE_U16: res.data.f64 = (double) imm0.reg.data.u16; break;
         case TYPE_U32: res.data.f64 = (double) imm0.reg.data.u32; break;
         case TYPE_S16: res.data.f64 = (double) imm0.reg.data.s16; break;
         case TYPE_S32: res.data.f64 = (double) imm0.reg.data.s32; break;
         default:
            return false;
         }
         i->setSrc(0, bld.mkImm(res.data.f64));
         break;
      default:
         return false;
      }
#undef CASE

      i->setType(i->dType); /* Remove i->sType, which we don't need anymore */
      i->op = OP_MOV;
      i->saturate = 0;
      i->src(0).mod = Modifier(0); /* Clear the already applied modifier */
      break;
   }
   default:
      return false;
   }

   // This can get left behind some of the optimizations which simplify
   // saturatable values.
   if (newi->op == OP_MOV && newi->saturate) {
      ImmediateValue tmp;
      newi->saturate = 0;
      newi->op = OP_SAT;
      if (newi->src(0).getImmediate(tmp))
         unary(newi, tmp);
   }

   if (newi->op != op)
      foldCount++;
   return deleted;
}

// =============================================================================

// Merge modifier operations (ABS, NEG, NOT) into ValueRefs where allowed.
class ModifierFolding : public Pass
{
private:
   virtual bool visit(BasicBlock *);
};

bool
ModifierFolding::visit(BasicBlock *bb)
{
   const Target *target = prog->getTarget();

   Instruction *i, *next, *mi;
   Modifier mod;

   for (i = bb->getEntry(); i; i = next) {
      next = i->next;

      if (false && i->op == OP_SUB) {
         // turn "sub" into "add neg" (do we really want this ?)
         i->op = OP_ADD;
         i->src(0).mod = i->src(0).mod ^ Modifier(NV50_IR_MOD_NEG);
      }

      for (int s = 0; s < 3 && i->srcExists(s); ++s) {
         mi = i->getSrc(s)->getInsn();
         if (!mi ||
             mi->predSrc >= 0 || mi->getDef(0)->refCount() > 8)
            continue;
         if (i->sType == TYPE_U32 && mi->dType == TYPE_S32) {
            if ((i->op != OP_ADD &&
                 i->op != OP_MUL) ||
                (mi->op != OP_ABS &&
                 mi->op != OP_NEG))
               continue;
         } else
         if (i->sType != mi->dType) {
            continue;
         }
         if ((mod = Modifier(mi->op)) == Modifier(0))
            continue;
         mod *= mi->src(0).mod;

         if ((i->op == OP_ABS) || i->src(s).mod.abs()) {
            // abs neg [abs] = abs
            mod = mod & Modifier(~(NV50_IR_MOD_NEG | NV50_IR_MOD_ABS));
         } else
         if ((i->op == OP_NEG) && mod.neg()) {
            assert(s == 0);
            // neg as both opcode and modifier on same insn is prohibited
            // neg neg abs = abs, neg neg = identity
            mod = mod & Modifier(~NV50_IR_MOD_NEG);
            i->op = mod.getOp();
            mod = mod & Modifier(~NV50_IR_MOD_ABS);
            if (mod == Modifier(0))
               i->op = OP_MOV;
         }

         if (target->isModSupported(i, s, mod)) {
            i->setSrc(s, mi->getSrc(0));
            i->src(s).mod *= mod;
         }
      }

      if (i->op == OP_SAT) {
         mi = i->getSrc(0)->getInsn();
         if (mi &&
             mi->getDef(0)->refCount() <= 1 && target->isSatSupported(mi)) {
            mi->saturate = 1;
            mi->setDef(0, i->getDef(0));
            delete_Instruction(prog, i);
         }
      }
   }

   return true;
}

// =============================================================================

// MUL + ADD -> MAD/FMA
// MIN/MAX(a, a) -> a, etc.
// SLCT(a, b, const) -> cc(const) ? a : b
// RCP(RCP(a)) -> a
// MUL(MUL(a, b), const) -> MUL_Xconst(a, b)
// EXTBF(RDSV(COMBINED_TID)) -> RDSV(TID)
class AlgebraicOpt : public Pass
{
private:
   virtual bool visit(BasicBlock *);

   void handleABS(Instruction *);
   bool handleADD(Instruction *);
   bool tryADDToMADOrSAD(Instruction *, operation toOp);
   void handleMINMAX(Instruction *);
   void handleRCP(Instruction *);
   void handleSLCT(Instruction *);
   void handleLOGOP(Instruction *);
   void handleCVT_NEG(Instruction *);
   void handleCVT_CVT(Instruction *);
   void handleCVT_EXTBF(Instruction *);
   void handleSUCLAMP(Instruction *);
   void handleNEG(Instruction *);
   void handleEXTBF_RDSV(Instruction *);

   BuildUtil bld;
};

void
AlgebraicOpt::handleABS(Instruction *abs)
{
   Instruction *sub = abs->getSrc(0)->getInsn();
   DataType ty;
   if (!sub ||
       !prog->getTarget()->isOpSupported(OP_SAD, abs->dType))
      return;
   // hidden conversion ?
   ty = intTypeToSigned(sub->dType);
   if (abs->dType != abs->sType || ty != abs->sType)
      return;

   if ((sub->op != OP_ADD && sub->op != OP_SUB) ||
       sub->src(0).getFile() != FILE_GPR || sub->src(0).mod ||
       sub->src(1).getFile() != FILE_GPR || sub->src(1).mod)
         return;

   Value *src0 = sub->getSrc(0);
   Value *src1 = sub->getSrc(1);

   if (sub->op == OP_ADD) {
      Instruction *neg = sub->getSrc(1)->getInsn();
      if (neg && neg->op != OP_NEG) {
         neg = sub->getSrc(0)->getInsn();
         src0 = sub->getSrc(1);
      }
      if (!neg || neg->op != OP_NEG ||
          neg->dType != neg->sType || neg->sType != ty)
         return;
      src1 = neg->getSrc(0);
   }

   // found ABS(SUB))
   abs->moveSources(1, 2); // move sources >=1 up by 2
   abs->op = OP_SAD;
   abs->setType(sub->dType);
   abs->setSrc(0, src0);
   abs->setSrc(1, src1);
   bld.setPosition(abs, false);
   abs->setSrc(2, bld.loadImm(bld.getSSA(typeSizeof(ty)), 0));
}

bool
AlgebraicOpt::handleADD(Instruction *add)
{
   Value *src0 = add->getSrc(0);
   Value *src1 = add->getSrc(1);

   if (src0->reg.file != FILE_GPR || src1->reg.file != FILE_GPR)
      return false;

   bool changed = false;
   // we can't optimize to MAD if the add is precise
   if (!add->precise && prog->getTarget()->isOpSupported(OP_MAD, add->dType))
      changed = tryADDToMADOrSAD(add, OP_MAD);
   if (!changed && prog->getTarget()->isOpSupported(OP_SAD, add->dType))
      changed = tryADDToMADOrSAD(add, OP_SAD);
   return changed;
}

// ADD(SAD(a,b,0), c) -> SAD(a,b,c)
// ADD(MUL(a,b), c) -> MAD(a,b,c)
bool
AlgebraicOpt::tryADDToMADOrSAD(Instruction *add, operation toOp)
{
   Value *src0 = add->getSrc(0);
   Value *src1 = add->getSrc(1);
   Value *src;
   int s;
   const operation srcOp = toOp == OP_SAD ? OP_SAD : OP_MUL;
   const Modifier modBad = Modifier(~((toOp == OP_MAD) ? NV50_IR_MOD_NEG : 0));
   Modifier mod[4];

   if (src0->refCount() == 1 &&
       src0->getUniqueInsn() && src0->getUniqueInsn()->op == srcOp)
      s = 0;
   else
   if (src1->refCount() == 1 &&
       src1->getUniqueInsn() && src1->getUniqueInsn()->op == srcOp)
      s = 1;
   else
      return false;

   src = add->getSrc(s);

   if (src->getUniqueInsn() && src->getUniqueInsn()->bb != add->bb)
      return false;

   if (src->getInsn()->saturate || src->getInsn()->postFactor ||
       src->getInsn()->dnz || src->getInsn()->precise)
      return false;

   if (toOp == OP_SAD) {
      ImmediateValue imm;
      if (!src->getInsn()->src(2).getImmediate(imm))
         return false;
      if (!imm.isInteger(0))
         return false;
   }

   if (typeSizeof(add->dType) != typeSizeof(src->getInsn()->dType) ||
       isFloatType(add->dType) != isFloatType(src->getInsn()->dType))
      return false;

   mod[0] = add->src(0).mod;
   mod[1] = add->src(1).mod;
   mod[2] = src->getUniqueInsn()->src(0).mod;
   mod[3] = src->getUniqueInsn()->src(1).mod;

   if (((mod[0] | mod[1]) | (mod[2] | mod[3])) & modBad)
      return false;

   add->op = toOp;
   add->subOp = src->getInsn()->subOp; // potentially mul-high
   add->dnz = src->getInsn()->dnz;
   add->dType = src->getInsn()->dType; // sign matters for imad hi
   add->sType = src->getInsn()->sType;

   add->setSrc(2, add->src(s ? 0 : 1));

   add->setSrc(0, src->getInsn()->getSrc(0));
   add->src(0).mod = mod[2] ^ mod[s];
   add->setSrc(1, src->getInsn()->getSrc(1));
   add->src(1).mod = mod[3];

   return true;
}

void
AlgebraicOpt::handleMINMAX(Instruction *minmax)
{
   Value *src0 = minmax->getSrc(0);
   Value *src1 = minmax->getSrc(1);

   if (src0 != src1 || src0->reg.file != FILE_GPR)
      return;
   if (minmax->src(0).mod == minmax->src(1).mod) {
      if (minmax->def(0).mayReplace(minmax->src(0))) {
         minmax->def(0).replace(minmax->src(0), false);
         delete_Instruction(prog, minmax);
      } else {
         minmax->op = OP_CVT;
         minmax->setSrc(1, NULL);
      }
   } else {
      // TODO:
      // min(x, -x) = -abs(x)
      // min(x, -abs(x)) = -abs(x)
      // min(x, abs(x)) = x
      // max(x, -abs(x)) = x
      // max(x, abs(x)) = abs(x)
      // max(x, -x) = abs(x)
   }
}

// rcp(rcp(a)) = a
// rcp(sqrt(a)) = rsq(a)
void
AlgebraicOpt::handleRCP(Instruction *rcp)
{
   Instruction *si = rcp->getSrc(0)->getUniqueInsn();

   if (!si)
      return;

   if (si->op == OP_RCP) {
      Modifier mod = rcp->src(0).mod * si->src(0).mod;
      rcp->op = mod.getOp();
      rcp->setSrc(0, si->getSrc(0));
   } else if (si->op == OP_SQRT) {
      rcp->op = OP_RSQ;
      rcp->setSrc(0, si->getSrc(0));
      rcp->src(0).mod = rcp->src(0).mod * si->src(0).mod;
   }
}

void
AlgebraicOpt::handleSLCT(Instruction *slct)
{
   if (slct->getSrc(2)->reg.file == FILE_IMMEDIATE) {
      if (slct->getSrc(2)->asImm()->compare(slct->asCmp()->setCond, 0.0f))
         slct->setSrc(0, slct->getSrc(1));
   } else
   if (slct->getSrc(0) != slct->getSrc(1)) {
      return;
   }
   slct->op = OP_MOV;
   slct->setSrc(1, NULL);
   slct->setSrc(2, NULL);
}

void
AlgebraicOpt::handleLOGOP(Instruction *logop)
{
   Value *src0 = logop->getSrc(0);
   Value *src1 = logop->getSrc(1);

   if (src0->reg.file != FILE_GPR || src1->reg.file != FILE_GPR)
      return;

   if (src0 == src1) {
      if ((logop->op == OP_AND || logop->op == OP_OR) &&
          logop->def(0).mayReplace(logop->src(0))) {
         logop->def(0).replace(logop->src(0), false);
         delete_Instruction(prog, logop);
      }
   } else {
      // try AND(SET, SET) -> SET_AND(SET)
      Instruction *set0 = src0->getInsn();
      Instruction *set1 = src1->getInsn();

      if (!set0 || set0->fixed || !set1 || set1->fixed)
         return;
      if (set1->op != OP_SET) {
         Instruction *xchg = set0;
         set0 = set1;
         set1 = xchg;
         if (set1->op != OP_SET)
            return;
      }
      operation redOp = (logop->op == OP_AND ? OP_SET_AND :
                         logop->op == OP_XOR ? OP_SET_XOR : OP_SET_OR);
      if (!prog->getTarget()->isOpSupported(redOp, set1->sType))
         return;
      if (set0->op != OP_SET &&
          set0->op != OP_SET_AND &&
          set0->op != OP_SET_OR &&
          set0->op != OP_SET_XOR)
         return;
      if (set0->getDef(0)->refCount() > 1 &&
          set1->getDef(0)->refCount() > 1)
         return;
      if (set0->getPredicate() || set1->getPredicate())
         return;
      // check that they don't source each other
      for (int s = 0; s < 2; ++s)
         if (set0->getSrc(s) == set1->getDef(0) ||
             set1->getSrc(s) == set0->getDef(0))
            return;

      set0 = cloneForward(func, set0);
      set1 = cloneShallow(func, set1);
      logop->bb->insertAfter(logop, set1);
      logop->bb->insertAfter(logop, set0);

      set0->dType = TYPE_U8;
      set0->getDef(0)->reg.file = FILE_PREDICATE;
      set0->getDef(0)->reg.size = 1;
      set1->setSrc(2, set0->getDef(0));
      set1->op = redOp;
      set1->setDef(0, logop->getDef(0));
      delete_Instruction(prog, logop);
   }
}

// F2I(NEG(SET with result 1.0f/0.0f)) -> SET with result -1/0
// nv50:
//  F2I(NEG(I2F(ABS(SET))))
void
AlgebraicOpt::handleCVT_NEG(Instruction *cvt)
{
   Instruction *insn = cvt->getSrc(0)->getInsn();
   if (cvt->sType != TYPE_F32 ||
       cvt->dType != TYPE_S32 || cvt->src(0).mod != Modifier(0))
      return;
   if (!insn || insn->op != OP_NEG || insn->dType != TYPE_F32)
      return;
   if (insn->src(0).mod != Modifier(0))
      return;
   insn = insn->getSrc(0)->getInsn();

   // check for nv50 SET(-1,0) -> SET(1.0f/0.0f) chain and nvc0's f32 SET
   if (insn && insn->op == OP_CVT &&
       insn->dType == TYPE_F32 &&
       insn->sType == TYPE_S32) {
      insn = insn->getSrc(0)->getInsn();
      if (!insn || insn->op != OP_ABS || insn->sType != TYPE_S32 ||
          insn->src(0).mod)
         return;
      insn = insn->getSrc(0)->getInsn();
      if (!insn || insn->op != OP_SET || insn->dType != TYPE_U32)
         return;
   } else
   if (!insn || insn->op != OP_SET || insn->dType != TYPE_F32) {
      return;
   }

   Instruction *bset = cloneShallow(func, insn);
   bset->dType = TYPE_U32;
   bset->setDef(0, cvt->getDef(0));
   cvt->bb->insertAfter(cvt, bset);
   delete_Instruction(prog, cvt);
}

// F2I(TRUNC()) and so on can be expressed as a single CVT. If the earlier CVT
// does a type conversion, this becomes trickier as there might be range
// changes/etc. We could handle those in theory as long as the range was being
// reduced or kept the same.
void
AlgebraicOpt::handleCVT_CVT(Instruction *cvt)
{
   Instruction *insn = cvt->getSrc(0)->getInsn();

   if (!insn ||
       insn->saturate ||
       insn->subOp ||
       insn->dType != insn->sType ||
       insn->dType != cvt->sType)
      return;

   RoundMode rnd = insn->rnd;
   switch (insn->op) {
   case OP_CEIL:
      rnd = ROUND_PI;
      break;
   case OP_FLOOR:
      rnd = ROUND_MI;
      break;
   case OP_TRUNC:
      rnd = ROUND_ZI;
      break;
   case OP_CVT:
      break;
   default:
      return;
   }

   if (!isFloatType(cvt->dType) || !isFloatType(insn->sType))
      rnd = (RoundMode)(rnd & 3);

   cvt->rnd = rnd;
   cvt->setSrc(0, insn->getSrc(0));
   cvt->src(0).mod *= insn->src(0).mod;
   cvt->sType = insn->sType;
}

// Some shaders extract packed bytes out of words and convert them to
// e.g. float. The Fermi+ CVT instruction can extract those directly, as can
// nv50 for word sizes.
//
// CVT(EXTBF(x, byte/word))
// CVT(AND(bytemask, x))
// CVT(AND(bytemask, SHR(x, 8/16/24)))
// CVT(SHR(x, 16/24))
void
AlgebraicOpt::handleCVT_EXTBF(Instruction *cvt)
{
   Instruction *insn = cvt->getSrc(0)->getInsn();
   ImmediateValue imm;
   Value *arg = NULL;
   unsigned width, offset = 0;
   if ((cvt->sType != TYPE_U32 && cvt->sType != TYPE_S32) || !insn)
      return;
   if (insn->op == OP_EXTBF && insn->src(1).getImmediate(imm)) {
      width = (imm.reg.data.u32 >> 8) & 0xff;
      offset = imm.reg.data.u32 & 0xff;
      arg = insn->getSrc(0);

      if (width != 8 && width != 16)
         return;
      if (width == 8 && offset & 0x7)
         return;
      if (width == 16 && offset & 0xf)
         return;
   } else if (insn->op == OP_AND) {
      int s;
      if (insn->src(0).getImmediate(imm))
         s = 0;
      else if (insn->src(1).getImmediate(imm))
         s = 1;
      else
         return;

      if (imm.reg.data.u32 == 0xff)
         width = 8;
      else if (imm.reg.data.u32 == 0xffff)
         width = 16;
      else
         return;

      arg = insn->getSrc(!s);
      Instruction *shift = arg->getInsn();

      if (shift && shift->op == OP_SHR &&
          shift->sType == cvt->sType &&
          shift->src(1).getImmediate(imm) &&
          ((width == 8 && (imm.reg.data.u32 & 0x7) == 0) ||
           (width == 16 && (imm.reg.data.u32 & 0xf) == 0))) {
         arg = shift->getSrc(0);
         offset = imm.reg.data.u32;
      }
      // We just AND'd the high bits away, which means this is effectively an
      // unsigned value.
      cvt->sType = TYPE_U32;
   } else if (insn->op == OP_SHR &&
              insn->sType == cvt->sType &&
              insn->src(1).getImmediate(imm)) {
      arg = insn->getSrc(0);
      if (imm.reg.data.u32 == 24) {
         width = 8;
         offset = 24;
      } else if (imm.reg.data.u32 == 16) {
         width = 16;
         offset = 16;
      } else {
         return;
      }
   }

   if (!arg)
      return;

   // Irrespective of what came earlier, we can undo a shift on the argument
   // by adjusting the offset.
   Instruction *shift = arg->getInsn();
   if (shift && shift->op == OP_SHL &&
       shift->src(1).getImmediate(imm) &&
       ((width == 8 && (imm.reg.data.u32 & 0x7) == 0) ||
        (width == 16 && (imm.reg.data.u32 & 0xf) == 0)) &&
       imm.reg.data.u32 <= offset) {
      arg = shift->getSrc(0);
      offset -= imm.reg.data.u32;
   }

   // The unpackSnorm lowering still leaves a few shifts behind, but it's too
   // annoying to detect them.

   if (width == 8) {
      cvt->sType = cvt->sType == TYPE_U32 ? TYPE_U8 : TYPE_S8;
   } else {
      assert(width == 16);
      cvt->sType = cvt->sType == TYPE_U32 ? TYPE_U16 : TYPE_S16;
   }
   cvt->setSrc(0, arg);
   cvt->subOp = offset >> 3;
}

// SUCLAMP dst, (ADD b imm), k, 0 -> SUCLAMP dst, b, k, imm (if imm fits s6)
void
AlgebraicOpt::handleSUCLAMP(Instruction *insn)
{
   ImmediateValue imm;
   int32_t val = insn->getSrc(2)->asImm()->reg.data.s32;
   int s;
   Instruction *add;

   assert(insn->srcExists(0) && insn->src(0).getFile() == FILE_GPR);

   // look for ADD (TODO: only count references by non-SUCLAMP)
   if (insn->getSrc(0)->refCount() > 1)
      return;
   add = insn->getSrc(0)->getInsn();
   if (!add || add->op != OP_ADD ||
       (add->dType != TYPE_U32 &&
        add->dType != TYPE_S32))
      return;

   // look for immediate
   for (s = 0; s < 2; ++s)
      if (add->src(s).getImmediate(imm))
         break;
   if (s >= 2)
      return;
   s = s ? 0 : 1;
   // determine if immediate fits
   val += imm.reg.data.s32;
   if (val > 31 || val < -32)
      return;
   // determine if other addend fits
   if (add->src(s).getFile() != FILE_GPR || add->src(s).mod != Modifier(0))
      return;

   bld.setPosition(insn, false); // make sure bld is init'ed
   // replace sources
   insn->setSrc(2, bld.mkImm(val));
   insn->setSrc(0, add->getSrc(s));
}

// NEG(AND(SET, 1)) -> SET
void
AlgebraicOpt::handleNEG(Instruction *i) {
   Instruction *src = i->getSrc(0)->getInsn();
   ImmediateValue imm;
   int b;

   if (isFloatType(i->sType) || !src || src->op != OP_AND)
      return;

   if (src->src(0).getImmediate(imm))
      b = 1;
   else if (src->src(1).getImmediate(imm))
      b = 0;
   else
      return;

   if (!imm.isInteger(1))
      return;

   Instruction *set = src->getSrc(b)->getInsn();
   if ((set->op == OP_SET || set->op == OP_SET_AND ||
       set->op == OP_SET_OR || set->op == OP_SET_XOR) &&
       !isFloatType(set->dType)) {
      i->def(0).replace(set->getDef(0), false);
   }
}

// EXTBF(RDSV(COMBINED_TID)) -> RDSV(TID)
void
AlgebraicOpt::handleEXTBF_RDSV(Instruction *i)
{
   Instruction *rdsv = i->getSrc(0)->getUniqueInsn();
   if (rdsv->op != OP_RDSV ||
       rdsv->getSrc(0)->asSym()->reg.data.sv.sv != SV_COMBINED_TID)
      return;
   // Avoid creating more RDSV instructions
   if (rdsv->getDef(0)->refCount() > 1)
      return;

   ImmediateValue imm;
   if (!i->src(1).getImmediate(imm))
      return;

   int index;
   if (imm.isInteger(0x1000))
      index = 0;
   else
   if (imm.isInteger(0x0a10))
      index = 1;
   else
   if (imm.isInteger(0x061a))
      index = 2;
   else
      return;

   bld.setPosition(i, false);

   i->op = OP_RDSV;
   i->setSrc(0, bld.mkSysVal(SV_TID, index));
   i->setSrc(1, NULL);
}

bool
AlgebraicOpt::visit(BasicBlock *bb)
{
   Instruction *next;
   for (Instruction *i = bb->getEntry(); i; i = next) {
      next = i->next;
      switch (i->op) {
      case OP_ABS:
         handleABS(i);
         break;
      case OP_ADD:
         handleADD(i);
         break;
      case OP_RCP:
         handleRCP(i);
         break;
      case OP_MIN:
      case OP_MAX:
         handleMINMAX(i);
         break;
      case OP_SLCT:
         handleSLCT(i);
         break;
      case OP_AND:
      case OP_OR:
      case OP_XOR:
         handleLOGOP(i);
         break;
      case OP_CVT:
         handleCVT_NEG(i);
         handleCVT_CVT(i);
         if (prog->getTarget()->isOpSupported(OP_EXTBF, TYPE_U32))
             handleCVT_EXTBF(i);
         break;
      case OP_SUCLAMP:
         handleSUCLAMP(i);
         break;
      case OP_NEG:
         handleNEG(i);
         break;
      case OP_EXTBF:
         handleEXTBF_RDSV(i);
         break;
      default:
         break;
      }
   }

   return true;
}

// =============================================================================

// ADD(SHL(a, b), c) -> SHLADD(a, b, c)
// MUL(a, b) -> a few XMADs
// MAD/FMA(a, b, c) -> a few XMADs
class LateAlgebraicOpt : public Pass
{
private:
   virtual bool visit(Instruction *);

   void handleADD(Instruction *);
   void handleMULMAD(Instruction *);
   bool tryADDToSHLADD(Instruction *);

   BuildUtil bld;
};

void
LateAlgebraicOpt::handleADD(Instruction *add)
{
   Value *src0 = add->getSrc(0);
   Value *src1 = add->getSrc(1);

   if (src0->reg.file != FILE_GPR || src1->reg.file != FILE_GPR)
      return;

   if (prog->getTarget()->isOpSupported(OP_SHLADD, add->dType))
      tryADDToSHLADD(add);
}

// ADD(SHL(a, b), c) -> SHLADD(a, b, c)
bool
LateAlgebraicOpt::tryADDToSHLADD(Instruction *add)
{
   Value *src0 = add->getSrc(0);
   Value *src1 = add->getSrc(1);
   ImmediateValue imm;
   Instruction *shl;
   Value *src;
   int s;

   if (add->saturate || add->usesFlags() || typeSizeof(add->dType) == 8
       || isFloatType(add->dType))
      return false;

   if (src0->getUniqueInsn() && src0->getUniqueInsn()->op == OP_SHL)
      s = 0;
   else
   if (src1->getUniqueInsn() && src1->getUniqueInsn()->op == OP_SHL)
      s = 1;
   else
      return false;

   src = add->getSrc(s);
   shl = src->getUniqueInsn();

   if (shl->bb != add->bb || shl->usesFlags() || shl->subOp || shl->src(0).mod)
      return false;

   if (!shl->src(1).getImmediate(imm))
      return false;

   add->op = OP_SHLADD;
   add->setSrc(2, add->src(!s));
   // SHL can't have any modifiers, but the ADD source may have had
   // one. Preserve it.
   add->setSrc(0, shl->getSrc(0));
   if (s == 1)
      add->src(0).mod = add->src(1).mod;
   add->setSrc(1, new_ImmediateValue(shl->bb->getProgram(), imm.reg.data.u32));
   add->src(1).mod = Modifier(0);

   return true;
}

// MUL(a, b) -> a few XMADs
// MAD/FMA(a, b, c) -> a few XMADs
void
LateAlgebraicOpt::handleMULMAD(Instruction *i)
{
   // TODO: handle NV50_IR_SUBOP_MUL_HIGH
   if (!prog->getTarget()->isOpSupported(OP_XMAD, TYPE_U32))
      return;
   if (isFloatType(i->dType) || typeSizeof(i->dType) != 4)
      return;
   if (i->subOp || i->usesFlags() || i->flagsDef >= 0)
      return;

   assert(!i->src(0).mod);
   assert(!i->src(1).mod);
   assert(i->op == OP_MUL ? 1 : !i->src(2).mod);

   bld.setPosition(i, false);

   Value *a = i->getSrc(0);
   Value *b = i->getSrc(1);
   Value *c = i->op == OP_MUL ? bld.mkImm(0) : i->getSrc(2);

   Value *tmp0 = bld.getSSA();
   Value *tmp1 = bld.getSSA();

   Instruction *insn = bld.mkOp3(OP_XMAD, TYPE_U32, tmp0, b, a, c);
   insn->setPredicate(i->cc, i->getPredicate());

   insn = bld.mkOp3(OP_XMAD, TYPE_U32, tmp1, b, a, bld.mkImm(0));
   insn->setPredicate(i->cc, i->getPredicate());
   insn->subOp = NV50_IR_SUBOP_XMAD_MRG | NV50_IR_SUBOP_XMAD_H1(1);

   Value *pred = i->getPredicate();
   i->setPredicate(i->cc, NULL);

   i->op = OP_XMAD;
   i->setSrc(0, b);
   i->setSrc(1, tmp1);
   i->setSrc(2, tmp0);
   i->subOp = NV50_IR_SUBOP_XMAD_PSL | NV50_IR_SUBOP_XMAD_CBCC;
   i->subOp |= NV50_IR_SUBOP_XMAD_H1(0) | NV50_IR_SUBOP_XMAD_H1(1);

   i->setPredicate(i->cc, pred);
}

bool
LateAlgebraicOpt::visit(Instruction *i)
{
   switch (i->op) {
   case OP_ADD:
      handleADD(i);
      break;
   case OP_MUL:
   case OP_MAD:
   case OP_FMA:
      handleMULMAD(i);
      break;
   default:
      break;
   }

   return true;
}

// =============================================================================

// Split 64-bit MUL and MAD
class Split64BitOpPreRA : public Pass
{
private:
   virtual bool visit(BasicBlock *);
   void split64MulMad(Function *, Instruction *, DataType);

   BuildUtil bld;
};

bool
Split64BitOpPreRA::visit(BasicBlock *bb)
{
   Instruction *i, *next;
   Modifier mod;

   for (i = bb->getEntry(); i; i = next) {
      next = i->next;

      DataType hTy;
      switch (i->dType) {
      case TYPE_U64: hTy = TYPE_U32; break;
      case TYPE_S64: hTy = TYPE_S32; break;
      default:
         continue;
      }

      if (i->op == OP_MAD || i->op == OP_MUL)
         split64MulMad(func, i, hTy);
   }

   return true;
}

void
Split64BitOpPreRA::split64MulMad(Function *fn, Instruction *i, DataType hTy)
{
   assert(i->op == OP_MAD || i->op == OP_MUL);
   assert(!isFloatType(i->dType) && !isFloatType(i->sType));
   assert(typeSizeof(hTy) == 4);

   bld.setPosition(i, true);

   Value *zero = bld.mkImm(0u);
   Value *carry = bld.getSSA(1, FILE_FLAGS);

   // We want to compute `d = a * b (+ c)?`, where a, b, c and d are 64-bit
   // values (a, b and c might be 32-bit values), using 32-bit operations. This
   // gives the following operations:
   // * `d.low = low(a.low * b.low) (+ c.low)?`
   // * `d.high = low(a.high * b.low) + low(a.low * b.high)
   //           + high(a.low * b.low) (+ c.high)?`
   //
   // To compute the high bits, we can split in the following operations:
   // * `tmp1   = low(a.high * b.low) (+ c.high)?`
   // * `tmp2   = low(a.low * b.high) + tmp1`
   // * `d.high = high(a.low * b.low) + tmp2`
   //
   // mkSplit put lower bits at index 0 and higher bits at index 1

   Value *op1[2];
   if (i->getSrc(0)->reg.size == 8)
      bld.mkSplit(op1, 4, i->getSrc(0));
   else {
      op1[0] = i->getSrc(0);
      op1[1] = zero;
   }
   Value *op2[2];
   if (i->getSrc(1)->reg.size == 8)
      bld.mkSplit(op2, 4, i->getSrc(1));
   else {
      op2[0] = i->getSrc(1);
      op2[1] = zero;
   }

   Value *op3[2] = { NULL, NULL };
   if (i->op == OP_MAD) {
      if (i->getSrc(2)->reg.size == 8)
         bld.mkSplit(op3, 4, i->getSrc(2));
      else {
         op3[0] = i->getSrc(2);
         op3[1] = zero;
      }
   }

   Value *tmpRes1Hi = bld.getSSA();
   if (i->op == OP_MAD)
      bld.mkOp3(OP_MAD, hTy, tmpRes1Hi, op1[1], op2[0], op3[1]);
   else
      bld.mkOp2(OP_MUL, hTy, tmpRes1Hi, op1[1], op2[0]);

   Value *tmpRes2Hi = bld.mkOp3v(OP_MAD, hTy, bld.getSSA(), op1[0], op2[1], tmpRes1Hi);

   Value *def[2] = { bld.getSSA(), bld.getSSA() };

   // If it was a MAD, add the carry from the low bits
   // It is not needed if it was a MUL, since we added high(a.low * b.low) to
   // d.high
   if (i->op == OP_MAD)
      bld.mkOp3(OP_MAD, hTy, def[0], op1[0], op2[0], op3[0])->setFlagsDef(1, carry);
   else
      bld.mkOp2(OP_MUL, hTy, def[0], op1[0], op2[0]);

   Instruction *hiPart3 = bld.mkOp3(OP_MAD, hTy, def[1], op1[0], op2[0], tmpRes2Hi);
   hiPart3->subOp = NV50_IR_SUBOP_MUL_HIGH;
   if (i->op == OP_MAD)
      hiPart3->setFlagsSrc(3, carry);

   bld.mkOp2(OP_MERGE, i->dType, i->getDef(0), def[0], def[1]);

   delete_Instruction(fn->getProgram(), i);
}

// =============================================================================

static inline void
updateLdStOffset(Instruction *ldst, int32_t offset, Function *fn)
{
   if (offset != ldst->getSrc(0)->reg.data.offset) {
      if (ldst->getSrc(0)->refCount() > 1)
         ldst->setSrc(0, cloneShallow(fn, ldst->getSrc(0)));
      ldst->getSrc(0)->reg.data.offset = offset;
   }
}

// Combine loads and stores, forward stores to loads where possible.
class MemoryOpt : public Pass
{
private:
   class Record
   {
   public:
      Record *next;
      Instruction *insn;
      const Value *rel[2];
      const Value *base;
      int32_t offset;
      int8_t fileIndex;
      uint8_t size;
      bool locked;
      Record *prev;

      bool overlaps(const Instruction *ldst) const;

      inline void link(Record **);
      inline void unlink(Record **);
      inline void set(const Instruction *ldst);
   };

public:
   MemoryOpt();

   Record *loads[DATA_FILE_COUNT];
   Record *stores[DATA_FILE_COUNT];

   MemoryPool recordPool;

private:
   virtual bool visit(BasicBlock *);
   bool runOpt(BasicBlock *);

   Record **getList(const Instruction *);

   Record *findRecord(const Instruction *, bool load, bool& isAdjacent) const;

   // merge @insn into load/store instruction from @rec
   bool combineLd(Record *rec, Instruction *ld);
   bool combineSt(Record *rec, Instruction *st);

   bool replaceLdFromLd(Instruction *ld, Record *ldRec);
   bool replaceLdFromSt(Instruction *ld, Record *stRec);
   bool replaceStFromSt(Instruction *restrict st, Record *stRec);

   void addRecord(Instruction *ldst);
   void purgeRecords(Instruction *const st, DataFile);
   void lockStores(Instruction *const ld);
   void reset();

private:
   Record *prevRecord;
};

MemoryOpt::MemoryOpt() : recordPool(sizeof(MemoryOpt::Record), 6)
{
   for (int i = 0; i < DATA_FILE_COUNT; ++i) {
      loads[i] = NULL;
      stores[i] = NULL;
   }
   prevRecord = NULL;
}

void
MemoryOpt::reset()
{
   for (unsigned int i = 0; i < DATA_FILE_COUNT; ++i) {
      Record *it, *next;
      for (it = loads[i]; it; it = next) {
         next = it->next;
         recordPool.release(it);
      }
      loads[i] = NULL;
      for (it = stores[i]; it; it = next) {
         next = it->next;
         recordPool.release(it);
      }
      stores[i] = NULL;
   }
}

bool
MemoryOpt::combineLd(Record *rec, Instruction *ld)
{
   int32_t offRc = rec->offset;
   int32_t offLd = ld->getSrc(0)->reg.data.offset;
   int sizeRc = rec->size;
   int sizeLd = typeSizeof(ld->dType);
   int size = sizeRc + sizeLd;
   int d, j;

   if (!prog->getTarget()->
       isAccessSupported(ld->getSrc(0)->reg.file, typeOfSize(size)))
      return false;
   // no unaligned loads
   if (((size == 0x8) && (MIN2(offLd, offRc) & 0x7)) ||
       ((size == 0xc) && (MIN2(offLd, offRc) & 0xf)))
      return false;
   // for compute indirect loads are not guaranteed to be aligned
   if (prog->getType() == Program::TYPE_COMPUTE && rec->rel[0])
      return false;

   assert(sizeRc + sizeLd <= 16 && offRc != offLd);

   // lock any stores that overlap with the load being merged into the
   // existing record.
   lockStores(ld);

   for (j = 0; sizeRc; sizeRc -= rec->insn->getDef(j)->reg.size, ++j);

   if (offLd < offRc) {
      int sz;
      for (sz = 0, d = 0; sz < sizeLd; sz += ld->getDef(d)->reg.size, ++d);
      // d: nr of definitions in ld
      // j: nr of definitions in rec->insn, move:
      for (d = d + j - 1; j > 0; --j, --d)
         rec->insn->setDef(d, rec->insn->getDef(j - 1));

      if (rec->insn->getSrc(0)->refCount() > 1)
         rec->insn->setSrc(0, cloneShallow(func, rec->insn->getSrc(0)));
      rec->offset = rec->insn->getSrc(0)->reg.data.offset = offLd;

      d = 0;
   } else {
      d = j;
   }
   // move definitions of @ld to @rec->insn
   for (j = 0; sizeLd; ++j, ++d) {
      sizeLd -= ld->getDef(j)->reg.size;
      rec->insn->setDef(d, ld->getDef(j));
   }

   rec->size = size;
   rec->insn->getSrc(0)->reg.size = size;
   rec->insn->setType(typeOfSize(size));

   delete_Instruction(prog, ld);

   return true;
}

bool
MemoryOpt::combineSt(Record *rec, Instruction *st)
{
   int32_t offRc = rec->offset;
   int32_t offSt = st->getSrc(0)->reg.data.offset;
   int sizeRc = rec->size;
   int sizeSt = typeSizeof(st->dType);
   int s = sizeSt / 4;
   int size = sizeRc + sizeSt;
   int j, k;
   Value *src[4]; // no modifiers in ValueRef allowed for st
   Value *extra[3];

   if (!prog->getTarget()->
       isAccessSupported(st->getSrc(0)->reg.file, typeOfSize(size)))
      return false;
   // no unaligned stores
   if (size == 8 && MIN2(offRc, offSt) & 0x7)
      return false;
   // for compute indirect stores are not guaranteed to be aligned
   if (prog->getType() == Program::TYPE_COMPUTE && rec->rel[0])
      return false;

   // There's really no great place to put this in a generic manner. Seemingly
   // wide stores at 0x60 don't work in GS shaders on SM50+. Don't combine
   // those.
   if (prog->getTarget()->getChipset() >= NVISA_GM107_CHIPSET &&
       prog->getType() == Program::TYPE_GEOMETRY &&
       st->getSrc(0)->reg.file == FILE_SHADER_OUTPUT &&
       rec->rel[0] == NULL &&
       MIN2(offRc, offSt) == 0x60)
      return false;

   // remove any existing load/store records for the store being merged into
   // the existing record.
   purgeRecords(st, DATA_FILE_COUNT);

   st->takeExtraSources(0, extra); // save predicate and indirect address

   if (offRc < offSt) {
      // save values from @st
      for (s = 0; sizeSt; ++s) {
         sizeSt -= st->getSrc(s + 1)->reg.size;
         src[s] = st->getSrc(s + 1);
      }
      // set record's values as low sources of @st
      for (j = 1; sizeRc; ++j) {
         sizeRc -= rec->insn->getSrc(j)->reg.size;
         st->setSrc(j, rec->insn->getSrc(j));
      }
      // set saved values as high sources of @st
      for (k = j, j = 0; j < s; ++j)
         st->setSrc(k++, src[j]);

      updateLdStOffset(st, offRc, func);
   } else {
      for (j = 1; sizeSt; ++j)
         sizeSt -= st->getSrc(j)->reg.size;
      for (s = 1; sizeRc; ++j, ++s) {
         sizeRc -= rec->insn->getSrc(s)->reg.size;
         st->setSrc(j, rec->insn->getSrc(s));
      }
      rec->offset = offSt;
   }
   st->putExtraSources(0, extra); // restore pointer and predicate

   delete_Instruction(prog, rec->insn);
   rec->insn = st;
   rec->size = size;
   rec->insn->getSrc(0)->reg.size = size;
   rec->insn->setType(typeOfSize(size));
   return true;
}

void
MemoryOpt::Record::set(const Instruction *ldst)
{
   const Symbol *mem = ldst->getSrc(0)->asSym();
   fileIndex = mem->reg.fileIndex;
   rel[0] = ldst->getIndirect(0, 0);
   rel[1] = ldst->getIndirect(0, 1);
   offset = mem->reg.data.offset;
   base = mem->getBase();
   size = typeSizeof(ldst->sType);
}

void
MemoryOpt::Record::link(Record **list)
{
   next = *list;
   if (next)
      next->prev = this;
   prev = NULL;
   *list = this;
}

void
MemoryOpt::Record::unlink(Record **list)
{
   if (next)
      next->prev = prev;
   if (prev)
      prev->next = next;
   else
      *list = next;
}

MemoryOpt::Record **
MemoryOpt::getList(const Instruction *insn)
{
   if (insn->op == OP_LOAD || insn->op == OP_VFETCH)
      return &loads[insn->src(0).getFile()];
   return &stores[insn->src(0).getFile()];
}

void
MemoryOpt::addRecord(Instruction *i)
{
   Record **list = getList(i);
   Record *it = reinterpret_cast<Record *>(recordPool.allocate());

   it->link(list);
   it->set(i);
   it->insn = i;
   it->locked = false;
}

MemoryOpt::Record *
MemoryOpt::findRecord(const Instruction *insn, bool load, bool& isAdj) const
{
   const Symbol *sym = insn->getSrc(0)->asSym();
   const int size = typeSizeof(insn->sType);
   Record *rec = NULL;
   Record *it = load ? loads[sym->reg.file] : stores[sym->reg.file];

   for (; it; it = it->next) {
      if (it->locked && insn->op != OP_LOAD && insn->op != OP_VFETCH)
         continue;
      if ((it->offset >> 4) != (sym->reg.data.offset >> 4) ||
          it->rel[0] != insn->getIndirect(0, 0) ||
          it->fileIndex != sym->reg.fileIndex ||
          it->rel[1] != insn->getIndirect(0, 1))
         continue;

      if (it->offset < sym->reg.data.offset) {
         if (it->offset + it->size >= sym->reg.data.offset) {
            isAdj = (it->offset + it->size == sym->reg.data.offset);
            if (!isAdj)
               return it;
            if (!(it->offset & 0x7))
               rec = it;
         }
      } else {
         isAdj = it->offset != sym->reg.data.offset;
         if (size <= it->size && !isAdj)
            return it;
         else
         if (!(sym->reg.data.offset & 0x7))
            if (it->offset - size <= sym->reg.data.offset)
               rec = it;
      }
   }
   return rec;
}

bool
MemoryOpt::replaceLdFromSt(Instruction *ld, Record *rec)
{
   Instruction *st = rec->insn;
   int32_t offSt = rec->offset;
   int32_t offLd = ld->getSrc(0)->reg.data.offset;
   int d, s;

   for (s = 1; offSt != offLd && st->srcExists(s); ++s)
      offSt += st->getSrc(s)->reg.size;
   if (offSt != offLd)
      return false;

   for (d = 0; ld->defExists(d) && st->srcExists(s); ++d, ++s) {
      if (ld->getDef(d)->reg.size != st->getSrc(s)->reg.size)
         return false;
      if (st->getSrc(s)->reg.file != FILE_GPR)
         return false;
      ld->def(d).replace(st->src(s), false);
   }
   ld->bb->remove(ld);
   return true;
}

bool
MemoryOpt::replaceLdFromLd(Instruction *ldE, Record *rec)
{
   Instruction *ldR = rec->insn;
   int32_t offR = rec->offset;
   int32_t offE = ldE->getSrc(0)->reg.data.offset;
   int dR, dE;

   assert(offR <= offE);
   for (dR = 0; offR < offE && ldR->defExists(dR); ++dR)
      offR += ldR->getDef(dR)->reg.size;
   if (offR != offE)
      return false;

   for (dE = 0; ldE->defExists(dE) && ldR->defExists(dR); ++dE, ++dR) {
      if (ldE->getDef(dE)->reg.size != ldR->getDef(dR)->reg.size)
         return false;
      ldE->def(dE).replace(ldR->getDef(dR), false);
   }

   delete_Instruction(prog, ldE);
   return true;
}

bool
MemoryOpt::replaceStFromSt(Instruction *restrict st, Record *rec)
{
   const Instruction *const ri = rec->insn;
   Value *extra[3];

   int32_t offS = st->getSrc(0)->reg.data.offset;
   int32_t offR = rec->offset;
   int32_t endS = offS + typeSizeof(st->dType);
   int32_t endR = offR + typeSizeof(ri->dType);

   rec->size = MAX2(endS, endR) - MIN2(offS, offR);

   st->takeExtraSources(0, extra);

   if (offR < offS) {
      Value *vals[10];
      int s, n;
      int k = 0;
      // get non-replaced sources of ri
      for (s = 1; offR < offS; offR += ri->getSrc(s)->reg.size, ++s)
         vals[k++] = ri->getSrc(s);
      n = s;
      // get replaced sources of st
      for (s = 1; st->srcExists(s); offS += st->getSrc(s)->reg.size, ++s)
         vals[k++] = st->getSrc(s);
      // skip replaced sources of ri
      for (s = n; offR < endS; offR += ri->getSrc(s)->reg.size, ++s);
      // get non-replaced sources after values covered by st
      for (; offR < endR; offR += ri->getSrc(s)->reg.size, ++s)
         vals[k++] = ri->getSrc(s);
      assert((unsigned int)k <= ARRAY_SIZE(vals));
      for (s = 0; s < k; ++s)
         st->setSrc(s + 1, vals[s]);
      st->setSrc(0, ri->getSrc(0));
   } else
   if (endR > endS) {
      int j, s;
      for (j = 1; offR < endS; offR += ri->getSrc(j++)->reg.size);
      for (s = 1; offS < endS; offS += st->getSrc(s++)->reg.size);
      for (; offR < endR; offR += ri->getSrc(j++)->reg.size)
         st->setSrc(s++, ri->getSrc(j));
   }
   st->putExtraSources(0, extra);

   delete_Instruction(prog, rec->insn);

   rec->insn = st;
   rec->offset = st->getSrc(0)->reg.data.offset;

   st->setType(typeOfSize(rec->size));

   return true;
}

bool
MemoryOpt::Record::overlaps(const Instruction *ldst) const
{
   Record that;
   that.set(ldst);

   // This assumes that images/buffers can't overlap. They can.
   // TODO: Plumb the restrict logic through, and only skip when it's a
   // restrict situation, or there can implicitly be no writes.
   if (this->fileIndex != that.fileIndex && this->rel[1] == that.rel[1])
      return false;

   if (this->rel[0] || that.rel[0])
      return this->base == that.base;

   return
      (this->offset < that.offset + that.size) &&
      (this->offset + this->size > that.offset);
}

// We must not eliminate stores that affect the result of @ld if
// we find later stores to the same location, and we may no longer
// merge them with later stores.
// The stored value can, however, still be used to determine the value
// returned by future loads.
void
MemoryOpt::lockStores(Instruction *const ld)
{
   for (Record *r = stores[ld->src(0).getFile()]; r; r = r->next)
      if (!r->locked && r->overlaps(ld))
         r->locked = true;
}

// Prior loads from the location of @st are no longer valid.
// Stores to the location of @st may no longer be used to derive
// the value at it nor be coalesced into later stores.
void
MemoryOpt::purgeRecords(Instruction *const st, DataFile f)
{
   if (st)
      f = st->src(0).getFile();

   for (Record *r = loads[f]; r; r = r->next)
      if (!st || r->overlaps(st))
         r->unlink(&loads[f]);

   for (Record *r = stores[f]; r; r = r->next)
      if (!st || r->overlaps(st))
         r->unlink(&stores[f]);
}

bool
MemoryOpt::visit(BasicBlock *bb)
{
   bool ret = runOpt(bb);
   // Run again, one pass won't combine 4 32 bit ld/st to a single 128 bit ld/st
   // where 96 bit memory operations are forbidden.
   if (ret)
      ret = runOpt(bb);
   return ret;
}

bool
MemoryOpt::runOpt(BasicBlock *bb)
{
   Instruction *ldst, *next;
   Record *rec;
   bool isAdjacent = true;

   for (ldst = bb->getEntry(); ldst; ldst = next) {
      bool keep = true;
      bool isLoad = true;
      next = ldst->next;

      // TODO: Handle combining sub 4-bytes loads/stores.
      if (ldst->op == OP_STORE && typeSizeof(ldst->dType) < 4) {
         purgeRecords(ldst, ldst->src(0).getFile());
         continue;
      }

      if (ldst->op == OP_LOAD || ldst->op == OP_VFETCH) {
         if (ldst->subOp == NV50_IR_SUBOP_LOAD_LOCKED) {
            purgeRecords(ldst, ldst->src(0).getFile());
            continue;
         }
         if (ldst->isDead()) {
            // might have been produced by earlier optimization
            delete_Instruction(prog, ldst);
            continue;
         }
      } else
      if (ldst->op == OP_STORE || ldst->op == OP_EXPORT) {
         if (ldst->subOp == NV50_IR_SUBOP_STORE_UNLOCKED) {
            purgeRecords(ldst, ldst->src(0).getFile());
            continue;
         }
         if (typeSizeof(ldst->dType) == 4 &&
             ldst->src(1).getFile() == FILE_GPR &&
             ldst->getSrc(1)->getInsn()->op == OP_NOP) {
            delete_Instruction(prog, ldst);
            continue;
         }
         isLoad = false;
      } else {
         // TODO: maybe have all fixed ops act as barrier ?
         if (ldst->op == OP_CALL ||
             ldst->op == OP_BAR ||
             ldst->op == OP_MEMBAR) {
            purgeRecords(NULL, FILE_MEMORY_LOCAL);
            purgeRecords(NULL, FILE_MEMORY_GLOBAL);
            purgeRecords(NULL, FILE_MEMORY_SHARED);
            purgeRecords(NULL, FILE_SHADER_OUTPUT);
         } else
         if (ldst->op == OP_ATOM || ldst->op == OP_CCTL) {
            if (ldst->src(0).getFile() == FILE_MEMORY_GLOBAL) {
               purgeRecords(NULL, FILE_MEMORY_LOCAL);
               purgeRecords(NULL, FILE_MEMORY_GLOBAL);
               purgeRecords(NULL, FILE_MEMORY_SHARED);
            } else {
               purgeRecords(NULL, ldst->src(0).getFile());
            }
         } else
         if (ldst->op == OP_EMIT || ldst->op == OP_RESTART) {
            purgeRecords(NULL, FILE_SHADER_OUTPUT);
         }
         continue;
      }
      if (ldst->getPredicate()) // TODO: handle predicated ld/st
         continue;
      if (ldst->perPatch) // TODO: create separate per-patch lists
         continue;

      if (isLoad) {
         DataFile file = ldst->src(0).getFile();

         // if ld l[]/g[] look for previous store to eliminate the reload
         if (file == FILE_MEMORY_GLOBAL || file == FILE_MEMORY_LOCAL) {
            // TODO: shared memory ?
            rec = findRecord(ldst, false, isAdjacent);
            if (rec && !isAdjacent)
               keep = !replaceLdFromSt(ldst, rec);
         }

         // or look for ld from the same location and replace this one
         rec = keep ? findRecord(ldst, true, isAdjacent) : NULL;
         if (rec) {
            if (!isAdjacent)
               keep = !replaceLdFromLd(ldst, rec);
            else
               // or combine a previous load with this one
               keep = !combineLd(rec, ldst);
         }
         if (keep)
            lockStores(ldst);
      } else {
         rec = findRecord(ldst, false, isAdjacent);
         if (rec) {
            if (!isAdjacent)
               keep = !replaceStFromSt(ldst, rec);
            else
               keep = !combineSt(rec, ldst);
         }
         if (keep)
            purgeRecords(ldst, DATA_FILE_COUNT);
      }
      if (keep)
         addRecord(ldst);
   }
   reset();

   return true;
}

// =============================================================================

// Turn control flow into predicated instructions (after register allocation !).
// TODO:
// Could move this to before register allocation on NVC0 and also handle nested
// constructs.
class FlatteningPass : public Pass
{
public:
   FlatteningPass() : gpr_unit(0) {}

private:
   virtual bool visit(Function *);
   virtual bool visit(BasicBlock *);

   bool tryPredicateConditional(BasicBlock *);
   void predicateInstructions(BasicBlock *, Value *pred, CondCode cc);
   void tryPropagateBranch(BasicBlock *);
   inline bool isConstantCondition(Value *pred);
   inline bool mayPredicate(const Instruction *, const Value *pred) const;
   inline void removeFlow(Instruction *);

   uint8_t gpr_unit;
};

bool
FlatteningPass::isConstantCondition(Value *pred)
{
   Instruction *insn = pred->getUniqueInsn();
   assert(insn);
   if (insn->op != OP_SET || insn->srcExists(2))
      return false;

   for (int s = 0; s < 2 && insn->srcExists(s); ++s) {
      Instruction *ld = insn->getSrc(s)->getUniqueInsn();
      DataFile file;
      if (ld) {
         if (ld->op != OP_MOV && ld->op != OP_LOAD)
            return false;
         if (ld->src(0).isIndirect(0))
            return false;
         file = ld->src(0).getFile();
      } else {
         file = insn->src(s).getFile();
         // catch $r63 on NVC0 and $r63/$r127 on NV50. Unfortunately maxGPR is
         // in register "units", which can vary between targets.
         if (file == FILE_GPR) {
            Value *v = insn->getSrc(s);
            int bytes = v->reg.data.id * MIN2(v->reg.size, 4);
            int units = bytes >> gpr_unit;
            if (units > prog->maxGPR)
               file = FILE_IMMEDIATE;
         }
      }
      if (file != FILE_IMMEDIATE && file != FILE_MEMORY_CONST)
         return false;
   }
   return true;
}

void
FlatteningPass::removeFlow(Instruction *insn)
{
   FlowInstruction *term = insn ? insn->asFlow() : NULL;
   if (!term)
      return;
   Graph::Edge::Type ty = term->bb->cfg.outgoing().getType();

   if (term->op == OP_BRA) {
      // TODO: this might get more difficult when we get arbitrary BRAs
      if (ty == Graph::Edge::CROSS || ty == Graph::Edge::BACK)
         return;
   } else
   if (term->op != OP_JOIN)
      return;

   Value *pred = term->getPredicate();

   delete_Instruction(prog, term);

   if (pred && pred->refCount() == 0) {
      Instruction *pSet = pred->getUniqueInsn();
      pred->join->reg.data.id = -1; // deallocate
      if (pSet->isDead())
         delete_Instruction(prog, pSet);
   }
}

void
FlatteningPass::predicateInstructions(BasicBlock *bb, Value *pred, CondCode cc)
{
   for (Instruction *i = bb->getEntry(); i; i = i->next) {
      if (i->isNop())
         continue;
      assert(!i->getPredicate());
      i->setPredicate(cc, pred);
   }
   removeFlow(bb->getExit());
}

bool
FlatteningPass::mayPredicate(const Instruction *insn, const Value *pred) const
{
   if (insn->isPseudo())
      return true;
   // TODO: calls where we don't know which registers are modified

   if (!prog->getTarget()->mayPredicate(insn, pred))
      return false;
   for (int d = 0; insn->defExists(d); ++d)
      if (insn->getDef(d)->equals(pred))
         return false;
   return true;
}

// If we jump to BRA/RET/EXIT, replace the jump with it.
// NOTE: We do not update the CFG anymore here !
//
// TODO: Handle cases where we skip over a branch (maybe do that elsewhere ?):
//  BB:0
//   @p0 bra BB:2 -> @!p0 bra BB:3 iff (!) BB:2 immediately adjoins BB:1
//  BB1:
//   bra BB:3
//  BB2:
//   ...
//  BB3:
//   ...
void
FlatteningPass::tryPropagateBranch(BasicBlock *bb)
{
   for (Instruction *i = bb->getExit(); i && i->op == OP_BRA; i = i->prev) {
      BasicBlock *bf = i->asFlow()->target.bb;

      if (bf->getInsnCount() != 1)
         continue;

      FlowInstruction *bra = i->asFlow();
      FlowInstruction *rep = bf->getExit()->asFlow();

      if (!rep || rep->getPredicate())
         continue;
      if (rep->op != OP_BRA &&
          rep->op != OP_JOIN &&
          rep->op != OP_EXIT)
         continue;

      // TODO: If there are multiple branches to @rep, only the first would
      // be replaced, so only remove them after this pass is done ?
      // Also, need to check all incident blocks for fall-through exits and
      // add the branch there.
      bra->op = rep->op;
      bra->target.bb = rep->target.bb;
      if (bf->cfg.incidentCount() == 1)
         bf->remove(rep);
   }
}

bool
FlatteningPass::visit(Function *fn)
{
   gpr_unit = prog->getTarget()->getFileUnit(FILE_GPR);

   return true;
}

bool
FlatteningPass::visit(BasicBlock *bb)
{
   if (tryPredicateConditional(bb))
      return true;

   // try to attach join to previous instruction
   if (prog->getTarget()->hasJoin) {
      Instruction *insn = bb->getExit();
      if (insn && insn->op == OP_JOIN && !insn->getPredicate()) {
         insn = insn->prev;
         if (insn && !insn->getPredicate() &&
             !insn->asFlow() &&
             insn->op != OP_DISCARD &&
             insn->op != OP_TEXBAR &&
             !isTextureOp(insn->op) && // probably just nve4
             !isSurfaceOp(insn->op) && // not confirmed
             insn->op != OP_LINTERP && // probably just nve4
             insn->op != OP_PINTERP && // probably just nve4
             ((insn->op != OP_LOAD && insn->op != OP_STORE && insn->op != OP_ATOM) ||
              (typeSizeof(insn->dType) <= 4 && !insn->src(0).isIndirect(0))) &&
             !insn->isNop()) {
            insn->join = 1;
            bb->remove(bb->getExit());
            return true;
         }
      }
   }

   tryPropagateBranch(bb);

   return true;
}

bool
FlatteningPass::tryPredicateConditional(BasicBlock *bb)
{
   BasicBlock *bL = NULL, *bR = NULL;
   unsigned int nL = 0, nR = 0, limit = 12;
   Instruction *insn;
   unsigned int mask;

   mask = bb->initiatesSimpleConditional();
   if (!mask)
      return false;

   assert(bb->getExit());
   Value *pred = bb->getExit()->getPredicate();
   assert(pred);

   if (isConstantCondition(pred))
      limit = 4;

   Graph::EdgeIterator ei = bb->cfg.outgoing();

   if (mask & 1) {
      bL = BasicBlock::get(ei.getNode());
      for (insn = bL->getEntry(); insn; insn = insn->next, ++nL)
         if (!mayPredicate(insn, pred))
            return false;
      if (nL > limit)
         return false; // too long, do a real branch
   }
   ei.next();

   if (mask & 2) {
      bR = BasicBlock::get(ei.getNode());
      for (insn = bR->getEntry(); insn; insn = insn->next, ++nR)
         if (!mayPredicate(insn, pred))
            return false;
      if (nR > limit)
         return false; // too long, do a real branch
   }

   if (bL)
      predicateInstructions(bL, pred, bb->getExit()->cc);
   if (bR)
      predicateInstructions(bR, pred, inverseCondCode(bb->getExit()->cc));

   if (bb->joinAt) {
      bb->remove(bb->joinAt);
      bb->joinAt = NULL;
   }
   removeFlow(bb->getExit()); // delete the branch/join at the fork point

   // remove potential join operations at the end of the conditional
   if (prog->getTarget()->joinAnterior) {
      bb = BasicBlock::get((bL ? bL : bR)->cfg.outgoing().getNode());
      if (bb->getEntry() && bb->getEntry()->op == OP_JOIN)
         removeFlow(bb->getEntry());
   }

   return true;
}

// =============================================================================

// Fold Immediate into MAD; must be done after register allocation due to
// constraint SDST == SSRC2
// TODO:
// Does NVC0+ have other situations where this pass makes sense?
class PostRaLoadPropagation : public Pass
{
private:
   virtual bool visit(Instruction *);

   void handleMADforNV50(Instruction *);
   void handleMADforNVC0(Instruction *);
};

static bool
post_ra_dead(Instruction *i)
{
   for (int d = 0; i->defExists(d); ++d)
      if (i->getDef(d)->refCount())
         return false;
   return true;
}

// Fold Immediate into MAD; must be done after register allocation due to
// constraint SDST == SSRC2
void
PostRaLoadPropagation::handleMADforNV50(Instruction *i)
{
   if (i->def(0).getFile() != FILE_GPR ||
       i->src(0).getFile() != FILE_GPR ||
       i->src(1).getFile() != FILE_GPR ||
       i->src(2).getFile() != FILE_GPR ||
       i->getDef(0)->reg.data.id != i->getSrc(2)->reg.data.id)
      return;

   if (i->getDef(0)->reg.data.id >= 64 ||
       i->getSrc(0)->reg.data.id >= 64)
      return;

   if (i->flagsSrc >= 0 && i->getSrc(i->flagsSrc)->reg.data.id != 0)
      return;

   if (i->getPredicate())
      return;

   Value *vtmp;
   Instruction *def = i->getSrc(1)->getInsn();

   if (def && def->op == OP_SPLIT && typeSizeof(def->sType) == 4)
      def = def->getSrc(0)->getInsn();
   if (def && def->op == OP_MOV && def->src(0).getFile() == FILE_IMMEDIATE) {
      vtmp = i->getSrc(1);
      if (isFloatType(i->sType)) {
         i->setSrc(1, def->getSrc(0));
      } else {
         ImmediateValue val;
         // getImmediate() has side-effects on the argument so this *shouldn't*
         // be folded into the assert()
         ASSERTED bool ret = def->src(0).getImmediate(val);
         assert(ret);
         if (i->getSrc(1)->reg.data.id & 1)
            val.reg.data.u32 >>= 16;
         val.reg.data.u32 &= 0xffff;
         i->setSrc(1, new_ImmediateValue(prog, val.reg.data.u32));
      }

      /* There's no post-RA dead code elimination, so do it here
       * XXX: if we add more code-removing post-RA passes, we might
       *      want to create a post-RA dead-code elim pass */
      if (post_ra_dead(vtmp->getInsn())) {
         Value *src = vtmp->getInsn()->getSrc(0);
         // Careful -- splits will have already been removed from the
         // functions. Don't double-delete.
         if (vtmp->getInsn()->bb)
            delete_Instruction(prog, vtmp->getInsn());
         if (src->getInsn() && post_ra_dead(src->getInsn()))
            delete_Instruction(prog, src->getInsn());
      }
   }
}

void
PostRaLoadPropagation::handleMADforNVC0(Instruction *i)
{
   if (i->def(0).getFile() != FILE_GPR ||
       i->src(0).getFile() != FILE_GPR ||
       i->src(1).getFile() != FILE_GPR ||
       i->src(2).getFile() != FILE_GPR ||
       i->getDef(0)->reg.data.id != i->getSrc(2)->reg.data.id)
      return;

   // TODO: gm107 can also do this for S32, maybe other chipsets as well
   if (i->dType != TYPE_F32)
      return;

   if ((i->src(2).mod | Modifier(NV50_IR_MOD_NEG)) != Modifier(NV50_IR_MOD_NEG))
      return;

   ImmediateValue val;
   int s;

   if (i->src(0).getImmediate(val))
      s = 1;
   else if (i->src(1).getImmediate(val))
      s = 0;
   else
      return;

   if ((i->src(s).mod | Modifier(NV50_IR_MOD_NEG)) != Modifier(NV50_IR_MOD_NEG))
      return;

   if (s == 1)
      i->swapSources(0, 1);

   Instruction *imm = i->getSrc(1)->getInsn();
   i->setSrc(1, imm->getSrc(0));
   if (post_ra_dead(imm))
      delete_Instruction(prog, imm);
}

bool
PostRaLoadPropagation::visit(Instruction *i)
{
   switch (i->op) {
   case OP_FMA:
   case OP_MAD:
      if (prog->getTarget()->getChipset() < 0xc0)
         handleMADforNV50(i);
      else
         handleMADforNVC0(i);
      break;
   default:
      break;
   }

   return true;
}

// =============================================================================

// Common subexpression elimination. Stupid O^2 implementation.
class LocalCSE : public Pass
{
private:
   virtual bool visit(BasicBlock *);

   inline bool tryReplace(Instruction **, Instruction *);

   DLList ops[OP_LAST + 1];
};

class GlobalCSE : public Pass
{
private:
   virtual bool visit(BasicBlock *);
};

bool
Instruction::isActionEqual(const Instruction *that) const
{
   if (this->op != that->op ||
       this->dType != that->dType ||
       this->sType != that->sType)
      return false;
   if (this->cc != that->cc)
      return false;

   if (this->asTex()) {
      if (memcmp(&this->asTex()->tex,
                 &that->asTex()->tex,
                 sizeof(this->asTex()->tex)))
         return false;
   } else
   if (this->asCmp()) {
      if (this->asCmp()->setCond != that->asCmp()->setCond)
         return false;
   } else
   if (this->asFlow()) {
      return false;
   } else
   if (this->op == OP_PHI && this->bb != that->bb) {
      /* TODO: we could probably be a bit smarter here by following the
       * control flow, but honestly, it is quite painful to check */
      return false;
   } else {
      if (this->ipa != that->ipa ||
          this->lanes != that->lanes ||
          this->perPatch != that->perPatch)
         return false;
      if (this->postFactor != that->postFactor)
         return false;
   }

   if (this->subOp != that->subOp ||
       this->saturate != that->saturate ||
       this->rnd != that->rnd ||
       this->ftz != that->ftz ||
       this->dnz != that->dnz ||
       this->cache != that->cache ||
       this->mask != that->mask)
      return false;

   return true;
}

bool
Instruction::isResultEqual(const Instruction *that) const
{
   unsigned int d, s;

   // NOTE: location of discard only affects tex with liveOnly and quadops
   if (!this->defExists(0) && this->op != OP_DISCARD)
      return false;

   if (!isActionEqual(that))
      return false;

   if (this->predSrc != that->predSrc)
      return false;

   for (d = 0; this->defExists(d); ++d) {
      if (!that->defExists(d) ||
          !this->getDef(d)->equals(that->getDef(d), false))
         return false;
   }
   if (that->defExists(d))
      return false;

   for (s = 0; this->srcExists(s); ++s) {
      if (!that->srcExists(s))
         return false;
      if (this->src(s).mod != that->src(s).mod)
         return false;
      if (!this->getSrc(s)->equals(that->getSrc(s), true))
         return false;
   }
   if (that->srcExists(s))
      return false;

   if (op == OP_LOAD || op == OP_VFETCH || op == OP_ATOM) {
      switch (src(0).getFile()) {
      case FILE_MEMORY_CONST:
      case FILE_SHADER_INPUT:
         return true;
      case FILE_SHADER_OUTPUT:
         return bb->getProgram()->getType() == Program::TYPE_TESSELLATION_EVAL;
      default:
         return false;
      }
   }

   return true;
}

// pull through common expressions from different in-blocks
bool
GlobalCSE::visit(BasicBlock *bb)
{
   Instruction *phi, *next, *ik;
   int s;

   // TODO: maybe do this with OP_UNION, too

   for (phi = bb->getPhi(); phi && phi->op == OP_PHI; phi = next) {
      next = phi->next;
      if (phi->getSrc(0)->refCount() > 1)
         continue;
      ik = phi->getSrc(0)->getInsn();
      if (!ik)
         continue; // probably a function input
      if (ik->defCount(0xff) > 1)
         continue; // too painful to check if we can really push this forward
      for (s = 1; phi->srcExists(s); ++s) {
         if (phi->getSrc(s)->refCount() > 1)
            break;
         if (!phi->getSrc(s)->getInsn() ||
             !phi->getSrc(s)->getInsn()->isResultEqual(ik))
            break;
      }
      if (!phi->srcExists(s)) {
         assert(ik->op != OP_PHI);
         Instruction *entry = bb->getEntry();
         ik->bb->remove(ik);
         if (!entry || entry->op != OP_JOIN)
            bb->insertHead(ik);
         else
            bb->insertAfter(entry, ik);
         ik->setDef(0, phi->getDef(0));
         delete_Instruction(prog, phi);
      }
   }

   return true;
}

bool
LocalCSE::tryReplace(Instruction **ptr, Instruction *i)
{
   Instruction *old = *ptr;

   // TODO: maybe relax this later (causes trouble with OP_UNION)
   if (i->isPredicated())
      return false;

   if (!old->isResultEqual(i))
      return false;

   for (int d = 0; old->defExists(d); ++d)
      old->def(d).replace(i->getDef(d), false);
   delete_Instruction(prog, old);
   *ptr = NULL;
   return true;
}

bool
LocalCSE::visit(BasicBlock *bb)
{
   unsigned int replaced;

   do {
      Instruction *ir, *next;

      replaced = 0;

      // will need to know the order of instructions
      int serial = 0;
      for (ir = bb->getFirst(); ir; ir = ir->next)
         ir->serial = serial++;

      for (ir = bb->getFirst(); ir; ir = next) {
         int s;
         Value *src = NULL;

         next = ir->next;

         if (ir->fixed) {
            ops[ir->op].insert(ir);
            continue;
         }

         for (s = 0; ir->srcExists(s); ++s)
            if (ir->getSrc(s)->asLValue())
               if (!src || ir->getSrc(s)->refCount() < src->refCount())
                  src = ir->getSrc(s);

         if (src) {
            for (Value::UseIterator it = src->uses.begin();
                 it != src->uses.end(); ++it) {
               Instruction *ik = (*it)->getInsn();
               if (ik && ik->bb == ir->bb && ik->serial < ir->serial)
                  if (tryReplace(&ir, ik))
                     break;
            }
         } else {
            DLLIST_FOR_EACH(&ops[ir->op], iter)
            {
               Instruction *ik = reinterpret_cast<Instruction *>(iter.get());
               if (tryReplace(&ir, ik))
                  break;
            }
         }

         if (ir)
            ops[ir->op].insert(ir);
         else
            ++replaced;
      }
      for (unsigned int i = 0; i <= OP_LAST; ++i)
         ops[i].clear();

   } while (replaced);

   return true;
}

// =============================================================================

// Remove computations of unused values.
class DeadCodeElim : public Pass
{
public:
   DeadCodeElim() : deadCount(0) {}
   bool buryAll(Program *);

private:
   virtual bool visit(BasicBlock *);

   void checkSplitLoad(Instruction *ld); // for partially dead loads

   unsigned int deadCount;
};

bool
DeadCodeElim::buryAll(Program *prog)
{
   do {
      deadCount = 0;
      if (!this->run(prog, false, false))
         return false;
   } while (deadCount);

   return true;
}

bool
DeadCodeElim::visit(BasicBlock *bb)
{
   Instruction *prev;

   for (Instruction *i = bb->getExit(); i; i = prev) {
      prev = i->prev;
      if (i->isDead()) {
         ++deadCount;
         delete_Instruction(prog, i);
      } else
      if (i->defExists(1) &&
          i->subOp == 0 &&
          (i->op == OP_VFETCH || i->op == OP_LOAD)) {
         checkSplitLoad(i);
      } else
      if (i->defExists(0) && !i->getDef(0)->refCount()) {
         if (i->op == OP_ATOM ||
             i->op == OP_SUREDP ||
             i->op == OP_SUREDB) {
            const Target *targ = prog->getTarget();
            if (targ->getChipset() >= NVISA_GF100_CHIPSET ||
                i->subOp != NV50_IR_SUBOP_ATOM_CAS)
               i->setDef(0, NULL);
            if (i->op == OP_ATOM && i->subOp == NV50_IR_SUBOP_ATOM_EXCH) {
               i->cache = CACHE_CV;
               i->op = OP_STORE;
               i->subOp = 0;
            }
         } else if (i->op == OP_LOAD && i->subOp == NV50_IR_SUBOP_LOAD_LOCKED) {
            i->setDef(0, i->getDef(1));
            i->setDef(1, NULL);
         }
      }
   }
   return true;
}

// Each load can go into up to 4 destinations, any of which might potentially
// be dead (i.e. a hole). These can always be split into 2 loads, independent
// of where the holes are. We find the first contiguous region, put it into
// the first load, and then put the second contiguous region into the second
// load. There can be at most 2 contiguous regions.
//
// Note that there are some restrictions, for example it's not possible to do
// a 64-bit load that's not 64-bit aligned, so such a load has to be split
// up. Also hardware doesn't support 96-bit loads, so those also have to be
// split into a 64-bit and 32-bit load.
void
DeadCodeElim::checkSplitLoad(Instruction *ld1)
{
   Instruction *ld2 = NULL; // can get at most 2 loads
   Value *def1[4];
   Value *def2[4];
   int32_t addr1, addr2;
   int32_t size1, size2;
   int d, n1, n2;
   uint32_t mask = 0xffffffff;

   for (d = 0; ld1->defExists(d); ++d)
      if (!ld1->getDef(d)->refCount() && ld1->getDef(d)->reg.data.id < 0)
         mask &= ~(1 << d);
   if (mask == 0xffffffff)
      return;

   addr1 = ld1->getSrc(0)->reg.data.offset;
   n1 = n2 = 0;
   size1 = size2 = 0;

   // Compute address/width for first load
   for (d = 0; ld1->defExists(d); ++d) {
      if (mask & (1 << d)) {
         if (size1 && (addr1 & 0x7))
            break;
         def1[n1] = ld1->getDef(d);
         size1 += def1[n1++]->reg.size;
      } else
      if (!n1) {
         addr1 += ld1->getDef(d)->reg.size;
      } else {
         break;
      }
   }

   // Scale back the size of the first load until it can be loaded. This
   // typically happens for TYPE_B96 loads.
   while (n1 &&
          !prog->getTarget()->isAccessSupported(ld1->getSrc(0)->reg.file,
                                                typeOfSize(size1))) {
      size1 -= def1[--n1]->reg.size;
      d--;
   }

   // Compute address/width for second load
   for (addr2 = addr1 + size1; ld1->defExists(d); ++d) {
      if (mask & (1 << d)) {
         assert(!size2 || !(addr2 & 0x7));
         def2[n2] = ld1->getDef(d);
         size2 += def2[n2++]->reg.size;
      } else if (!n2) {
         assert(!n2);
         addr2 += ld1->getDef(d)->reg.size;
      } else {
         break;
      }
   }

   // Make sure that we've processed all the values
   for (; ld1->defExists(d); ++d)
      assert(!(mask & (1 << d)));

   updateLdStOffset(ld1, addr1, func);
   ld1->setType(typeOfSize(size1));
   for (d = 0; d < 4; ++d)
      ld1->setDef(d, (d < n1) ? def1[d] : NULL);

   if (!n2)
      return;

   ld2 = cloneShallow(func, ld1);
   updateLdStOffset(ld2, addr2, func);
   ld2->setType(typeOfSize(size2));
   for (d = 0; d < 4; ++d)
      ld2->setDef(d, (d < n2) ? def2[d] : NULL);

   ld1->bb->insertAfter(ld1, ld2);
}

// =============================================================================

#define RUN_PASS(l, n, f)                       \
   if (level >= (l)) {                          \
      if (dbgFlags & NV50_IR_DEBUG_VERBOSE)     \
         INFO("PEEPHOLE: %s\n", #n);            \
      n pass;                                   \
      if (!pass.f(this))                        \
         return false;                          \
   }

bool
Program::optimizeSSA(int level)
{
   RUN_PASS(1, DeadCodeElim, buryAll);
   RUN_PASS(1, CopyPropagation, run);
   RUN_PASS(1, MergeSplits, run);
   RUN_PASS(2, GlobalCSE, run);
   RUN_PASS(1, LocalCSE, run);
   RUN_PASS(2, AlgebraicOpt, run);
   RUN_PASS(2, ModifierFolding, run); // before load propagation -> less checks
   RUN_PASS(1, ConstantFolding, foldAll);
   RUN_PASS(0, Split64BitOpPreRA, run);
   RUN_PASS(2, LateAlgebraicOpt, run);
   RUN_PASS(1, LoadPropagation, run);
   RUN_PASS(1, IndirectPropagation, run);
   RUN_PASS(4, MemoryOpt, run);
   RUN_PASS(2, LocalCSE, run);
   RUN_PASS(0, DeadCodeElim, buryAll);

   return true;
}

bool
Program::optimizePostRA(int level)
{
   RUN_PASS(2, FlatteningPass, run);
   RUN_PASS(2, PostRaLoadPropagation, run);

   return true;
}

}
