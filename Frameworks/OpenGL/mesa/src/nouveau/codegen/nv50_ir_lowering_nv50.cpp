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

#include "nv50_ir_target_nv50.h"

#define NV50_SU_INFO_SIZE_X   0x00
#define NV50_SU_INFO_SIZE_Y   0x04
#define NV50_SU_INFO_SIZE_Z   0x08
#define NV50_SU_INFO_BSIZE    0x0c
#define NV50_SU_INFO_STRIDE_Y 0x10
#define NV50_SU_INFO_MS_X     0x18
#define NV50_SU_INFO_MS_Y     0x1c
#define NV50_SU_INFO_TILE_SHIFT_X 0x20
#define NV50_SU_INFO_TILE_SHIFT_Y 0x24
#define NV50_SU_INFO_TILE_SHIFT_Z 0x28
#define NV50_SU_INFO_OFFSET_Z 0x2c

#define NV50_SU_INFO__STRIDE 0x30

#define NV50_SU_INFO_SIZE(i) (0x00 + (i) * 4)
#define NV50_SU_INFO_MS(i)   (0x18 + (i) * 4)
#define NV50_SU_INFO_TILE_SHIFT(i) (0x20 + (i) * 4)

namespace nv50_ir {

// nv50 doesn't support 32 bit integer multiplication
//
//       ah al * bh bl = LO32: (al * bh + ah * bl) << 16 + (al * bl)
// -------------------
//    al*bh 00           HI32: (al * bh + ah * bl) >> 16 + (ah * bh) +
// ah*bh 00 00                 (           carry1) << 16 + ( carry2)
//       al*bl
//    ah*bl 00
//
// fffe0001 + fffe0001
//
// Note that this sort of splitting doesn't work for signed values, so we
// compute the sign on those manually and then perform an unsigned multiply.
static bool
expandIntegerMUL(BuildUtil *bld, Instruction *mul)
{
   const bool highResult = mul->subOp == NV50_IR_SUBOP_MUL_HIGH;
   ImmediateValue src1;
   bool src1imm = mul->src(1).getImmediate(src1);

   DataType fTy; // full type
   switch (mul->sType) {
   case TYPE_S32: fTy = TYPE_U32; break;
   case TYPE_S64: fTy = TYPE_U64; break;
   default: fTy = mul->sType; break;
   }

   DataType hTy; // half type
   switch (fTy) {
   case TYPE_U32: hTy = TYPE_U16; break;
   case TYPE_U64: hTy = TYPE_U32; break;
   default:
      return false;
   }
   unsigned int fullSize = typeSizeof(fTy);
   unsigned int halfSize = typeSizeof(hTy);

   Instruction *i[9];

   bld->setPosition(mul, true);

   Value *s[2];
   Value *a[2], *b[2];
   Value *t[4];
   for (int j = 0; j < 4; ++j)
      t[j] = bld->getSSA(fullSize);

   if (isSignedType(mul->sType) && highResult) {
      s[0] = bld->getSSA(fullSize);
      s[1] = bld->getSSA(fullSize);
      bld->mkOp1(OP_ABS, mul->sType, s[0], mul->getSrc(0));
      bld->mkOp1(OP_ABS, mul->sType, s[1], mul->getSrc(1));
      src1.reg.data.s32 = abs(src1.reg.data.s32);
   } else {
      s[0] = mul->getSrc(0);
      s[1] = mul->getSrc(1);
   }

   // split sources into halves
   i[0] = bld->mkSplit(a, halfSize, s[0]);
   i[1] = bld->mkSplit(b, halfSize, s[1]);

   if (src1imm && (src1.reg.data.u32 & 0xffff0000) == 0) {
      i[2] = i[3] = bld->mkOp2(OP_MUL, fTy, t[1], a[1],
                               bld->mkImm(src1.reg.data.u32 & 0xffff));
   } else {
      i[2] = bld->mkOp2(OP_MUL, fTy, t[0], a[0],
                        src1imm ? bld->mkImm(src1.reg.data.u32 >> 16) : b[1]);
      if (src1imm && (src1.reg.data.u32 & 0x0000ffff) == 0) {
         i[3] = i[2];
         t[1] = t[0];
      } else {
         i[3] = bld->mkOp3(OP_MAD, fTy, t[1], a[1], b[0], t[0]);
      }
   }
   i[7] = bld->mkOp2(OP_SHL, fTy, t[2], t[1], bld->mkImm(halfSize * 8));
   if (src1imm && (src1.reg.data.u32 & 0x0000ffff) == 0) {
      i[4] = i[3];
      t[3] = t[2];
   } else {
      i[4] = bld->mkOp3(OP_MAD, fTy, t[3], a[0], b[0], t[2]);
   }

   if (highResult) {
      Value *c[2];
      Value *r[5];
      Value *imm = bld->loadImm(NULL, 1 << (halfSize * 8));
      c[0] = bld->getSSA(1, FILE_FLAGS);
      c[1] = bld->getSSA(1, FILE_FLAGS);
      for (int j = 0; j < 5; ++j)
         r[j] = bld->getSSA(fullSize);

      i[8] = bld->mkOp2(OP_SHR, fTy, r[0], t[1], bld->mkImm(halfSize * 8));
      i[6] = bld->mkOp2(OP_ADD, fTy, r[1], r[0], imm);
      bld->mkMov(r[3], r[0])->setPredicate(CC_NC, c[0]);
      bld->mkOp2(OP_UNION, TYPE_U32, r[2], r[1], r[3]);
      i[5] = bld->mkOp3(OP_MAD, fTy, r[4], a[1], b[1], r[2]);

      // set carry defs / sources
      i[3]->setFlagsDef(1, c[0]);
      // actual result required in negative case, but ignored for
      // unsigned. for some reason the compiler ends up dropping the whole
      // instruction if the destination is unused but the flags are.
      if (isSignedType(mul->sType))
         i[4]->setFlagsDef(1, c[1]);
      else
         i[4]->setFlagsDef(0, c[1]);
      i[6]->setPredicate(CC_C, c[0]);
      i[5]->setFlagsSrc(3, c[1]);

      if (isSignedType(mul->sType)) {
         Value *cc[2];
         Value *rr[7];
         Value *one = bld->getSSA(fullSize);
         bld->loadImm(one, 1);
         for (int j = 0; j < 7; j++)
            rr[j] = bld->getSSA(fullSize);

         // NOTE: this logic uses predicates because splitting basic blocks is
         // ~impossible during the SSA phase. The RA relies on a correlation
         // between edge order and phi node sources.

         // Set the sign of the result based on the inputs
         bld->mkOp2(OP_XOR, fTy, NULL, mul->getSrc(0), mul->getSrc(1))
            ->setFlagsDef(0, (cc[0] = bld->getSSA(1, FILE_FLAGS)));

         // 1s complement of 64-bit value
         bld->mkOp1(OP_NOT, fTy, rr[0], r[4])
            ->setPredicate(CC_S, cc[0]);
         bld->mkOp1(OP_NOT, fTy, rr[1], t[3])
            ->setPredicate(CC_S, cc[0]);

         // add to low 32-bits, keep track of the carry
         Instruction *n = bld->mkOp2(OP_ADD, fTy, NULL, rr[1], one);
         n->setPredicate(CC_S, cc[0]);
         n->setFlagsDef(0, (cc[1] = bld->getSSA(1, FILE_FLAGS)));

         // If there was a carry, add 1 to the upper 32 bits
         // XXX: These get executed even if they shouldn't be
         bld->mkOp2(OP_ADD, fTy, rr[2], rr[0], one)
            ->setPredicate(CC_C, cc[1]);
         bld->mkMov(rr[3], rr[0])
            ->setPredicate(CC_NC, cc[1]);
         bld->mkOp2(OP_UNION, fTy, rr[4], rr[2], rr[3]);

         // Merge the results from the negative and non-negative paths
         bld->mkMov(rr[5], rr[4])
            ->setPredicate(CC_S, cc[0]);
         bld->mkMov(rr[6], r[4])
            ->setPredicate(CC_NS, cc[0]);
         bld->mkOp2(OP_UNION, mul->sType, mul->getDef(0), rr[5], rr[6]);
      } else {
         bld->mkMov(mul->getDef(0), r[4]);
      }
   } else {
      bld->mkMov(mul->getDef(0), t[3]);
   }
   delete_Instruction(bld->getProgram(), mul);

   for (int j = 2; j <= (highResult ? 5 : 4); ++j)
      if (i[j])
         i[j]->sType = hTy;

   return true;
}

#define QOP_ADD  0
#define QOP_SUBR 1
#define QOP_SUB  2
#define QOP_MOV2 3

//             UL UR LL LR
#define QUADOP(q, r, s, t)            \
   ((QOP_##q << 6) | (QOP_##r << 4) | \
    (QOP_##s << 2) | (QOP_##t << 0))

class NV50LegalizePostRA : public Pass
{
public:
   NV50LegalizePostRA() : r63(NULL) { }

private:
   virtual bool visit(Function *);
   virtual bool visit(BasicBlock *);

   void handlePRERET(FlowInstruction *);
   void replaceZero(Instruction *);

   BuildUtil bld;

   LValue *r63;
};

bool
NV50LegalizePostRA::visit(Function *fn)
{
   Program *prog = fn->getProgram();

   r63 = new_LValue(fn, FILE_GPR);
   // GPR units on nv50 are in half-regs
   if (prog->maxGPR < 126)
      r63->reg.data.id = 63;
   else
      r63->reg.data.id = 127;

   // this is actually per-program, but we can do it all on visiting main()
   std::list<Instruction *> *outWrites =
      reinterpret_cast<std::list<Instruction *> *>(prog->targetPriv);

   if (outWrites) {
      for (std::list<Instruction *>::iterator it = outWrites->begin();
           it != outWrites->end(); ++it)
         (*it)->getSrc(1)->defs.front()->getInsn()->setDef(0, (*it)->getSrc(0));
      // instructions will be deleted on exit
      outWrites->clear();
   }

   return true;
}

void
NV50LegalizePostRA::replaceZero(Instruction *i)
{
   for (int s = 0; i->srcExists(s); ++s) {
      ImmediateValue *imm = i->getSrc(s)->asImm();
      if (imm && imm->reg.data.u64 == 0)
         i->setSrc(s, r63);
   }
}

// Emulate PRERET: jump to the target and call to the origin from there
//
// WARNING: atm only works if BBs are affected by at most a single PRERET
//
// BB:0
// preret BB:3
// (...)
// BB:3
// (...)
//             --->
// BB:0
// bra BB:3 + n0 (directly to the call; move to beginning of BB and fixate)
// (...)
// BB:3
// bra BB:3 + n1 (skip the call)
// call BB:0 + n2 (skip bra at beginning of BB:0)
// (...)
void
NV50LegalizePostRA::handlePRERET(FlowInstruction *pre)
{
   BasicBlock *bbE = pre->bb;
   BasicBlock *bbT = pre->target.bb;

   pre->subOp = NV50_IR_SUBOP_EMU_PRERET + 0;
   bbE->remove(pre);
   bbE->insertHead(pre);

   Instruction *skip = new_FlowInstruction(func, OP_PRERET, bbT);
   Instruction *call = new_FlowInstruction(func, OP_PRERET, bbE);

   bbT->insertHead(call);
   bbT->insertHead(skip);

   // NOTE: maybe split blocks to prevent the instructions from moving ?

   skip->subOp = NV50_IR_SUBOP_EMU_PRERET + 1;
   call->subOp = NV50_IR_SUBOP_EMU_PRERET + 2;
}

bool
NV50LegalizePostRA::visit(BasicBlock *bb)
{
   Instruction *i, *next;

   // remove pseudo operations and non-fixed no-ops, split 64 bit operations
   for (i = bb->getFirst(); i; i = next) {
      next = i->next;
      if (i->isNop()) {
         bb->remove(i);
      } else
      if (i->op == OP_PRERET && prog->getTarget()->getChipset() < 0xa0) {
         handlePRERET(i->asFlow());
      } else {
         // TODO: We will want to do this before register allocation,
         // since have to use a $c register for the carry flag.
         if (typeSizeof(i->dType) == 8) {
            Instruction *hi = BuildUtil::split64BitOpPostRA(func, i, r63, NULL);
            if (hi)
               next = hi;
         }

         if (i->op != OP_PFETCH && i->op != OP_BAR &&
             (!i->defExists(0) || i->def(0).getFile() != FILE_ADDRESS))
            replaceZero(i);
      }
   }
   if (!bb->getEntry())
      return true;

   return true;
}

class NV50LegalizeSSA : public Pass
{
public:
   NV50LegalizeSSA(Program *);

   virtual bool visit(BasicBlock *bb);

private:
   void propagateWriteToOutput(Instruction *);
   void handleDIV(Instruction *);
   void handleMOD(Instruction *);
   void handleMUL(Instruction *);
   void handleAddrDef(Instruction *);

   inline bool isARL(const Instruction *) const;

   BuildUtil bld;

   std::list<Instruction *> *outWrites;
};

NV50LegalizeSSA::NV50LegalizeSSA(Program *prog)
{
   bld.setProgram(prog);

   if (prog->optLevel >= 2 &&
       (prog->getType() == Program::TYPE_GEOMETRY ||
        prog->getType() == Program::TYPE_VERTEX))
      outWrites =
         reinterpret_cast<std::list<Instruction *> *>(prog->targetPriv);
   else
      outWrites = NULL;
}

void
NV50LegalizeSSA::propagateWriteToOutput(Instruction *st)
{
   if (st->src(0).isIndirect(0) || st->getSrc(1)->refCount() != 1)
      return;

   // check def instruction can store
   Instruction *di = st->getSrc(1)->defs.front()->getInsn();

   // TODO: move exports (if beneficial) in common opt pass
   if (di->isPseudo() || isTextureOp(di->op) || di->defCount(0xff, true) > 1)
      return;

   for (int s = 0; di->srcExists(s); ++s)
      if (di->src(s).getFile() == FILE_IMMEDIATE ||
          di->src(s).getFile() == FILE_MEMORY_LOCAL)
         return;

   if (prog->getType() == Program::TYPE_GEOMETRY) {
      // Only propagate output writes in geometry shaders when we can be sure
      // that we are propagating to the same output vertex.
      if (di->bb != st->bb)
         return;
      Instruction *i;
      for (i = di; i != st; i = i->next) {
         if (i->op == OP_EMIT || i->op == OP_RESTART)
            return;
      }
      assert(i); // st after di
   }

   // We cannot set defs to non-lvalues before register allocation, so
   // save & remove (to save registers) the exports and replace later.
   outWrites->push_back(st);
   st->bb->remove(st);
}

bool
NV50LegalizeSSA::isARL(const Instruction *i) const
{
   ImmediateValue imm;

   if (i->op != OP_SHL || i->src(0).getFile() != FILE_GPR)
      return false;
   if (!i->src(1).getImmediate(imm))
      return false;
   return imm.isInteger(0);
}

void
NV50LegalizeSSA::handleAddrDef(Instruction *i)
{
   Instruction *arl;

   i->getDef(0)->reg.size = 2; // $aX are only 16 bit

   // PFETCH can always write to $a
   if (i->op == OP_PFETCH)
      return;
   // only ADDR <- SHL(GPR, IMM) and ADDR <- ADD(ADDR, IMM) are valid
   if (i->srcExists(1) && i->src(1).getFile() == FILE_IMMEDIATE) {
      if (i->op == OP_SHL && i->src(0).getFile() == FILE_GPR)
         return;
      if (i->op == OP_ADD && i->src(0).getFile() == FILE_ADDRESS)
         return;
   }

   // turn $a sources into $r sources (can't operate on $a)
   for (int s = 0; i->srcExists(s); ++s) {
      Value *a = i->getSrc(s);
      Value *r;
      if (a->reg.file == FILE_ADDRESS) {
         if (a->getInsn() && isARL(a->getInsn())) {
            i->setSrc(s, a->getInsn()->getSrc(0));
         } else {
            bld.setPosition(i, false);
            r = bld.getSSA();
            bld.mkMov(r, a);
            i->setSrc(s, r);
         }
      }
   }
   if (i->op == OP_SHL && i->src(1).getFile() == FILE_IMMEDIATE)
      return;

   // turn result back into $a
   bld.setPosition(i, true);
   arl = bld.mkOp2(OP_SHL, TYPE_U32, i->getDef(0), bld.getSSA(), bld.mkImm(0));
   i->setDef(0, arl->getSrc(0));
}

void
NV50LegalizeSSA::handleMUL(Instruction *mul)
{
   if (isFloatType(mul->sType) || typeSizeof(mul->sType) <= 2)
      return;
   Value *def = mul->getDef(0);
   Value *pred = mul->getPredicate();
   CondCode cc = mul->cc;
   if (pred)
      mul->setPredicate(CC_ALWAYS, NULL);

   if (mul->op == OP_MAD) {
      Instruction *add = mul;
      bld.setPosition(add, false);
      Value *res = cloneShallow(func, mul->getDef(0));
      mul = bld.mkOp2(OP_MUL, add->sType, res, add->getSrc(0), add->getSrc(1));
      add->op = OP_ADD;
      add->setSrc(0, mul->getDef(0));
      add->setSrc(1, add->getSrc(2));
      for (int s = 2; add->srcExists(s); ++s)
         add->setSrc(s, NULL);
      mul->subOp = add->subOp;
      add->subOp = 0;
   }
   expandIntegerMUL(&bld, mul);
   if (pred)
      def->getInsn()->setPredicate(cc, pred);
}

// Use f32 division: first compute an approximate result, use it to reduce
// the dividend, which should then be representable as f32, divide the reduced
// dividend, and add the quotients.
void
NV50LegalizeSSA::handleDIV(Instruction *div)
{
   const DataType ty = div->sType;

   if (ty != TYPE_U32 && ty != TYPE_S32)
      return;

   Value *q, *q0, *qf, *aR, *aRf, *qRf, *qR, *t, *s, *m, *cond;

   bld.setPosition(div, false);

   Value *a, *af = bld.getSSA();
   Value *b, *bf = bld.getSSA();

   bld.mkCvt(OP_CVT, TYPE_F32, af, ty, div->getSrc(0));
   bld.mkCvt(OP_CVT, TYPE_F32, bf, ty, div->getSrc(1));

   if (isSignedType(ty)) {
      af->getInsn()->src(0).mod = Modifier(NV50_IR_MOD_ABS);
      bf->getInsn()->src(0).mod = Modifier(NV50_IR_MOD_ABS);
      a = bld.getSSA();
      b = bld.getSSA();
      bld.mkOp1(OP_ABS, ty, a, div->getSrc(0));
      bld.mkOp1(OP_ABS, ty, b, div->getSrc(1));
   } else {
      a = div->getSrc(0);
      b = div->getSrc(1);
   }

   bf = bld.mkOp1v(OP_RCP, TYPE_F32, bld.getSSA(), bf);
   bf = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(), bf, bld.mkImm(-2));

   bld.mkOp2(OP_MUL, TYPE_F32, (qf = bld.getSSA()), af, bf)->rnd = ROUND_Z;
   bld.mkCvt(OP_CVT, ty, (q0 = bld.getSSA()), TYPE_F32, qf)->rnd = ROUND_Z;

   // get error of 1st result
   expandIntegerMUL(&bld,
      bld.mkOp2(OP_MUL, TYPE_U32, (t = bld.getSSA()), q0, b));
   bld.mkOp2(OP_SUB, TYPE_U32, (aRf = bld.getSSA()), a, t);

   bld.mkCvt(OP_CVT, TYPE_F32, (aR = bld.getSSA()), TYPE_U32, aRf);

   bld.mkOp2(OP_MUL, TYPE_F32, (qRf = bld.getSSA()), aR, bf)->rnd = ROUND_Z;
   bld.mkCvt(OP_CVT, TYPE_U32, (qR = bld.getSSA()), TYPE_F32, qRf)
      ->rnd = ROUND_Z;
   bld.mkOp2(OP_ADD, ty, (q = bld.getSSA()), q0, qR); // add quotients

   // correction: if modulus >= divisor, add 1
   expandIntegerMUL(&bld,
      bld.mkOp2(OP_MUL, TYPE_U32, (t = bld.getSSA()), q, b));
   bld.mkOp2(OP_SUB, TYPE_U32, (m = bld.getSSA()), a, t);
   bld.mkCmp(OP_SET, CC_GE, TYPE_U32, (s = bld.getSSA()), TYPE_U32, m, b);
   if (!isSignedType(ty)) {
      div->op = OP_SUB;
      div->setSrc(0, q);
      div->setSrc(1, s);
   } else {
      t = q;
      bld.mkOp2(OP_SUB, TYPE_U32, (q = bld.getSSA()), t, s);
      s = bld.getSSA();
      t = bld.getSSA();
      // fix the sign
      bld.mkOp2(OP_XOR, TYPE_U32, NULL, div->getSrc(0), div->getSrc(1))
         ->setFlagsDef(0, (cond = bld.getSSA(1, FILE_FLAGS)));
      bld.mkOp1(OP_NEG, ty, s, q)->setPredicate(CC_S, cond);
      bld.mkOp1(OP_MOV, ty, t, q)->setPredicate(CC_NS, cond);

      div->op = OP_UNION;
      div->setSrc(0, s);
      div->setSrc(1, t);
   }
}

void
NV50LegalizeSSA::handleMOD(Instruction *mod)
{
   if (mod->dType != TYPE_U32 && mod->dType != TYPE_S32)
      return;
   bld.setPosition(mod, false);

   Value *q = bld.getSSA();
   Value *m = bld.getSSA();

   bld.mkOp2(OP_DIV, mod->dType, q, mod->getSrc(0), mod->getSrc(1));
   handleDIV(q->getInsn());

   bld.setPosition(mod, false);
   expandIntegerMUL(&bld, bld.mkOp2(OP_MUL, TYPE_U32, m, q, mod->getSrc(1)));

   mod->op = OP_SUB;
   mod->setSrc(1, m);
}

bool
NV50LegalizeSSA::visit(BasicBlock *bb)
{
   Instruction *insn, *next;
   // skipping PHIs (don't pass them to handleAddrDef) !
   for (insn = bb->getEntry(); insn; insn = next) {
      next = insn->next;

      if (insn->defExists(0) && insn->getDef(0)->reg.file == FILE_ADDRESS)
         handleAddrDef(insn);

      switch (insn->op) {
      case OP_EXPORT:
         if (outWrites)
            propagateWriteToOutput(insn);
         break;
      case OP_DIV:
         handleDIV(insn);
         break;
      case OP_MOD:
         handleMOD(insn);
         break;
      case OP_MAD:
      case OP_MUL:
         handleMUL(insn);
         break;
      default:
         break;
      }
   }
   return true;
}

class NV50LoweringPreSSA : public Pass
{
public:
   NV50LoweringPreSSA(Program *);

private:
   virtual bool visit(Instruction *);
   virtual bool visit(Function *);

   bool handleRDSV(Instruction *);

   bool handlePFETCH(Instruction *);
   bool handleEXPORT(Instruction *);
   bool handleLOAD(Instruction *);
   bool handleLDST(Instruction *);
   bool handleMEMBAR(Instruction *);
   bool handleSharedATOM(Instruction *);
   bool handleSULDP(TexInstruction *);
   bool handleSUREDP(TexInstruction *);
   bool handleSUSTP(TexInstruction *);
   Value *processSurfaceCoords(TexInstruction *);

   bool handleDIV(Instruction *);
   bool handleSQRT(Instruction *);

   bool handleSET(Instruction *);
   bool handleSLCT(CmpInstruction *);
   bool handleSELP(Instruction *);

   bool handleTEX(TexInstruction *);
   bool handleTXB(TexInstruction *); // I really
   bool handleTXL(TexInstruction *); // hate
   bool handleTXD(TexInstruction *); // these 3
   bool handleTXLQ(TexInstruction *);
   bool handleTXQ(TexInstruction *);
   bool handleSUQ(TexInstruction *);
   bool handleBUFQ(Instruction *);

   bool handleCALL(Instruction *);
   bool handlePRECONT(Instruction *);
   bool handleCONT(Instruction *);

   void checkPredicate(Instruction *);
   void loadTexMsInfo(uint32_t off, Value **ms, Value **ms_x, Value **ms_y);
   void loadMsInfo(Value *ms, Value *s, Value **dx, Value **dy);
   Value *loadSuInfo(int slot, uint32_t off);
   Value *loadSuInfo16(int slot, uint32_t off);

private:
   const Target *const targ;

   BuildUtil bld;

   Value *tid;
};

NV50LoweringPreSSA::NV50LoweringPreSSA(Program *prog) :
   targ(prog->getTarget()), tid(NULL)
{
   bld.setProgram(prog);
}

bool
NV50LoweringPreSSA::visit(Function *f)
{
   BasicBlock *root = BasicBlock::get(func->cfg.getRoot());

   if (prog->getType() == Program::TYPE_COMPUTE) {
      // Add implicit "thread id" argument in $r0 to the function
      Value *arg = new_LValue(func, FILE_GPR);
      arg->reg.data.id = 0;
      f->ins.push_back(arg);

      bld.setPosition(root, false);
      tid = bld.mkMov(bld.getScratch(), arg, TYPE_U32)->getDef(0);
   }

   return true;
}

void NV50LoweringPreSSA::loadTexMsInfo(uint32_t off, Value **ms,
                                       Value **ms_x, Value **ms_y) {
   // This loads the texture-indexed ms setting from the constant buffer
   Value *tmp = new_LValue(func, FILE_GPR);
   uint8_t b = prog->driver->io.auxCBSlot;
   off += prog->driver->io.suInfoBase;
   if (prog->getType() > Program::TYPE_VERTEX)
      off += 16 * 2 * 4;
   if (prog->getType() > Program::TYPE_GEOMETRY)
      off += 16 * 2 * 4;
   if (prog->getType() > Program::TYPE_FRAGMENT)
      off += 16 * 2 * 4;
   *ms_x = bld.mkLoadv(TYPE_U32, bld.mkSymbol(
                             FILE_MEMORY_CONST, b, TYPE_U32, off + 0), NULL);
   *ms_y = bld.mkLoadv(TYPE_U32, bld.mkSymbol(
                             FILE_MEMORY_CONST, b, TYPE_U32, off + 4), NULL);
   *ms = bld.mkOp2v(OP_ADD, TYPE_U32, tmp, *ms_x, *ms_y);
}

void NV50LoweringPreSSA::loadMsInfo(Value *ms, Value *s, Value **dx, Value **dy) {
   // Given a MS level, and a sample id, compute the delta x/y
   uint8_t b = prog->driver->io.msInfoCBSlot;
   Value *off = new_LValue(func, FILE_ADDRESS), *t = new_LValue(func, FILE_GPR);

   // The required information is at mslevel * 16 * 4 + sample * 8
   // = (mslevel * 8 + sample) * 8
   bld.mkOp2(OP_SHL,
             TYPE_U32,
             off,
             bld.mkOp2v(OP_ADD, TYPE_U32, t,
                        bld.mkOp2v(OP_SHL, TYPE_U32, t, ms, bld.mkImm(3)),
                        s),
             bld.mkImm(3));
   *dx = bld.mkLoadv(TYPE_U32, bld.mkSymbol(
                           FILE_MEMORY_CONST, b, TYPE_U32,
                           prog->driver->io.msInfoBase), off);
   *dy = bld.mkLoadv(TYPE_U32, bld.mkSymbol(
                           FILE_MEMORY_CONST, b, TYPE_U32,
                           prog->driver->io.msInfoBase + 4), off);
}

Value *
NV50LoweringPreSSA::loadSuInfo(int slot, uint32_t off)
{
   uint8_t b = prog->driver->io.auxCBSlot;
   off += prog->driver->io.bufInfoBase + slot * NV50_SU_INFO__STRIDE;
   return bld.mkLoadv(TYPE_U32, bld.mkSymbol(
                            FILE_MEMORY_CONST, b, TYPE_U32, off), NULL);
}

Value *
NV50LoweringPreSSA::loadSuInfo16(int slot, uint32_t off)
{
   uint8_t b = prog->driver->io.auxCBSlot;
   off += prog->driver->io.bufInfoBase + slot * NV50_SU_INFO__STRIDE;
   return bld.mkLoadv(TYPE_U16, bld.mkSymbol(
                            FILE_MEMORY_CONST, b, TYPE_U16, off), NULL);
}

bool
NV50LoweringPreSSA::handleTEX(TexInstruction *i)
{
   const int arg = i->tex.target.getArgCount();
   const int dref = arg;
   const int lod = i->tex.target.isShadow() ? (arg + 1) : arg;

   /* Only normalize in the non-explicit derivatives case.
    */
   if (i->tex.target.isCube() && i->op != OP_TXD) {
      Value *src[3], *val;
      int c;
      for (c = 0; c < 3; ++c)
         src[c] = bld.mkOp1v(OP_ABS, TYPE_F32, bld.getSSA(), i->getSrc(c));
      val = bld.getScratch();
      bld.mkOp2(OP_MAX, TYPE_F32, val, src[0], src[1]);
      bld.mkOp2(OP_MAX, TYPE_F32, val, src[2], val);
      bld.mkOp1(OP_RCP, TYPE_F32, val, val);
      for (c = 0; c < 3; ++c) {
         i->setSrc(c, bld.mkOp2v(OP_MUL, TYPE_F32, bld.getSSA(),
                                 i->getSrc(c), val));
      }
   }

   // handle MS, which means looking up the MS params for this texture, and
   // adjusting the input coordinates to point at the right sample.
   if (i->tex.target.isMS()) {
      Value *x = i->getSrc(0);
      Value *y = i->getSrc(1);
      Value *s = i->getSrc(arg - 1);
      Value *tx = new_LValue(func, FILE_GPR), *ty = new_LValue(func, FILE_GPR),
         *ms, *ms_x, *ms_y, *dx, *dy;

      i->tex.target.clearMS();

      loadTexMsInfo(i->tex.r * 4 * 2, &ms, &ms_x, &ms_y);
      loadMsInfo(ms, s, &dx, &dy);

      bld.mkOp2(OP_SHL, TYPE_U32, tx, x, ms_x);
      bld.mkOp2(OP_SHL, TYPE_U32, ty, y, ms_y);
      bld.mkOp2(OP_ADD, TYPE_U32, tx, tx, dx);
      bld.mkOp2(OP_ADD, TYPE_U32, ty, ty, dy);
      i->setSrc(0, tx);
      i->setSrc(1, ty);
      i->setSrc(arg - 1, bld.loadImm(NULL, 0));
   }

   // dref comes before bias/lod
   if (i->tex.target.isShadow())
      if (i->op == OP_TXB || i->op == OP_TXL)
         i->swapSources(dref, lod);

   if (i->tex.target.isArray()) {
      if (i->op != OP_TXF) {
         // array index must be converted to u32, but it's already an integer
         // for TXF
         Value *layer = i->getSrc(arg - 1);
         LValue *src = new_LValue(func, FILE_GPR);
         bld.mkCvt(OP_CVT, TYPE_U32, src, TYPE_F32, layer);
         bld.mkOp2(OP_MIN, TYPE_U32, src, src, bld.loadImm(NULL, 511));
         i->setSrc(arg - 1, src);
      }
      if (i->tex.target.isCube() && i->srcCount() > 4) {
         std::vector<Value *> acube, a2d;
         int c;

         acube.resize(4);
         for (c = 0; c < 4; ++c)
            acube[c] = i->getSrc(c);
         a2d.resize(4);
         for (c = 0; c < 3; ++c)
            a2d[c] = new_LValue(func, FILE_GPR);
         a2d[3] = NULL;

         bld.mkTex(OP_TEXPREP, TEX_TARGET_CUBE_ARRAY, i->tex.r, i->tex.s,
                   a2d, acube)->asTex()->tex.mask = 0x7;

         for (c = 0; c < 3; ++c)
            i->setSrc(c, a2d[c]);
         for (; i->srcExists(c + 1); ++c)
            i->setSrc(c, i->getSrc(c + 1));
         i->setSrc(c, NULL);
         assert(c <= 4);

         i->tex.target = i->tex.target.isShadow() ?
            TEX_TARGET_2D_ARRAY_SHADOW : TEX_TARGET_2D_ARRAY;
      }
   }

   // texel offsets are 3 immediate fields in the instruction,
   // nv50 cannot do textureGatherOffsets
   assert(i->tex.useOffsets <= 1);
   if (i->tex.useOffsets) {
      for (int c = 0; c < 3; ++c) {
         ImmediateValue val;
         if (!i->offset[0][c].getImmediate(val))
            assert(!"non-immediate offset");
         i->tex.offset[c] = val.reg.data.u32;
         i->offset[0][c].set(NULL);
      }
   }

   return true;
}

// Bias must be equal for all threads of a quad or lod calculation will fail.
//
// The lanes of a quad are grouped by the bit in the condition register they
// have set, which is selected by differing bias values.
// Move the input values for TEX into a new register set for each group and
// execute TEX only for a specific group.
// We always need to use 4 new registers for the inputs/outputs because the
// implicitly calculated derivatives must be correct.
//
// TODO: move to SSA phase so we can easily determine whether bias is constant
bool
NV50LoweringPreSSA::handleTXB(TexInstruction *i)
{
   const CondCode cc[4] = { CC_EQU, CC_S, CC_C, CC_O };
   int l, d;

   // We can't actually apply bias *and* do a compare for a cube
   // texture. Since the compare has to be done before the filtering, just
   // drop the bias on the floor.
   if (i->tex.target == TEX_TARGET_CUBE_SHADOW) {
      i->op = OP_TEX;
      i->setSrc(3, i->getSrc(4));
      i->setSrc(4, NULL);
      return handleTEX(i);
   }

   handleTEX(i);
   Value *bias = i->getSrc(i->tex.target.getArgCount());
   if (bias->isUniform())
      return true;

   Instruction *cond = bld.mkOp1(OP_UNION, TYPE_U32, bld.getScratch(),
                                 bld.loadImm(NULL, 1));
   bld.setPosition(cond, false);

   for (l = 1; l < 4; ++l) {
      const uint8_t qop = QUADOP(SUBR, SUBR, SUBR, SUBR);
      Value *bit = bld.getSSA();
      Value *pred = bld.getScratch(1, FILE_FLAGS);
      Value *imm = bld.loadImm(NULL, (1 << l));
      bld.mkQuadop(qop, pred, l, bias, bias)->flagsDef = 0;
      bld.mkMov(bit, imm)->setPredicate(CC_EQ, pred);
      cond->setSrc(l, bit);
   }
   Value *flags = bld.getScratch(1, FILE_FLAGS);
   bld.setPosition(cond, true);
   bld.mkCvt(OP_CVT, TYPE_U8, flags, TYPE_U32, cond->getDef(0))->flagsDef = 0;

   Instruction *tex[4];
   for (l = 0; l < 4; ++l) {
      (tex[l] = cloneForward(func, i))->setPredicate(cc[l], flags);
      bld.insert(tex[l]);
   }

   Value *res[4][4];
   for (d = 0; i->defExists(d); ++d)
      res[0][d] = tex[0]->getDef(d);
   for (l = 1; l < 4; ++l) {
      for (d = 0; tex[l]->defExists(d); ++d) {
         res[l][d] = cloneShallow(func, res[0][d]);
         bld.mkMov(res[l][d], tex[l]->getDef(d))->setPredicate(cc[l], flags);
      }
   }

   for (d = 0; i->defExists(d); ++d) {
      Instruction *dst = bld.mkOp(OP_UNION, TYPE_U32, i->getDef(d));
      for (l = 0; l < 4; ++l)
         dst->setSrc(l, res[l][d]);
   }
   delete_Instruction(prog, i);
   return true;
}

// LOD must be equal for all threads of a quad.
// Unlike with TXB, here we can just diverge since there's no LOD calculation
// that would require all 4 threads' sources to be set up properly.
bool
NV50LoweringPreSSA::handleTXL(TexInstruction *i)
{
   handleTEX(i);
   Value *lod = i->getSrc(i->tex.target.getArgCount());
   if (lod->isUniform())
      return true;

   BasicBlock *currBB = i->bb;
   BasicBlock *texiBB = i->bb->splitBefore(i, false);
   BasicBlock *joinBB = i->bb->splitAfter(i);

   bld.setPosition(currBB, true);
   assert(!currBB->joinAt);
   currBB->joinAt = bld.mkFlow(OP_JOINAT, joinBB, CC_ALWAYS, NULL);

   for (int l = 0; l <= 3; ++l) {
      const uint8_t qop = QUADOP(SUBR, SUBR, SUBR, SUBR);
      Value *pred = bld.getScratch(1, FILE_FLAGS);
      bld.setPosition(currBB, true);
      bld.mkQuadop(qop, pred, l, lod, lod)->flagsDef = 0;
      bld.mkFlow(OP_BRA, texiBB, CC_EQ, pred)->fixed = 1;
      currBB->cfg.attach(&texiBB->cfg, Graph::Edge::FORWARD);
      if (l <= 2) {
         BasicBlock *laneBB = new BasicBlock(func);
         currBB->cfg.attach(&laneBB->cfg, Graph::Edge::TREE);
         currBB = laneBB;
      }
   }
   bld.setPosition(joinBB, false);
   bld.mkFlow(OP_JOIN, NULL, CC_ALWAYS, NULL)->fixed = 1;
   return true;
}

bool
NV50LoweringPreSSA::handleTXD(TexInstruction *i)
{
   static const uint8_t qOps[4][2] =
   {
      { QUADOP(MOV2, ADD,  MOV2, ADD),  QUADOP(MOV2, MOV2, ADD,  ADD) }, // l0
      { QUADOP(SUBR, MOV2, SUBR, MOV2), QUADOP(MOV2, MOV2, ADD,  ADD) }, // l1
      { QUADOP(MOV2, ADD,  MOV2, ADD),  QUADOP(SUBR, SUBR, MOV2, MOV2) }, // l2
      { QUADOP(SUBR, MOV2, SUBR, MOV2), QUADOP(SUBR, SUBR, MOV2, MOV2) }, // l3
   };
   Value *def[4][4];
   Value *crd[3];
   Instruction *tex;
   Value *zero = bld.loadImm(bld.getSSA(), 0);
   int l, c;
   const int dim = i->tex.target.getDim() + i->tex.target.isCube();

   handleTEX(i);
   i->op = OP_TEX; // no need to clone dPdx/dPdy later
   i->tex.derivAll = true;

   for (c = 0; c < dim; ++c)
      crd[c] = bld.getScratch();

   bld.mkOp(OP_QUADON, TYPE_NONE, NULL);
   for (l = 0; l < 4; ++l) {
      Value *src[3], *val;
      // mov coordinates from lane l to all lanes
      for (c = 0; c < dim; ++c)
         bld.mkQuadop(0x00, crd[c], l, i->getSrc(c), zero);
      // add dPdx from lane l to lanes dx
      for (c = 0; c < dim; ++c)
         bld.mkQuadop(qOps[l][0], crd[c], l, i->dPdx[c].get(), crd[c]);
      // add dPdy from lane l to lanes dy
      for (c = 0; c < dim; ++c)
         bld.mkQuadop(qOps[l][1], crd[c], l, i->dPdy[c].get(), crd[c]);
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
      for (c = 0; c < dim; ++c)
         tex->setSrc(c, src[c]);
      // save results
      for (c = 0; i->defExists(c); ++c) {
         Instruction *mov;
         def[c][l] = bld.getSSA();
         mov = bld.mkMov(def[c][l], tex->getDef(c));
         mov->fixed = 1;
         mov->lanes = 1 << l;
      }
   }
   bld.mkOp(OP_QUADPOP, TYPE_NONE, NULL);

   for (c = 0; i->defExists(c); ++c) {
      Instruction *u = bld.mkOp(OP_UNION, TYPE_U32, i->getDef(c));
      for (l = 0; l < 4; ++l)
         u->setSrc(l, def[c][l]);
   }

   i->bb->remove(i);
   return true;
}

bool
NV50LoweringPreSSA::handleTXLQ(TexInstruction *i)
{
   handleTEX(i);
   bld.setPosition(i, true);

   /* The returned values are not quite what we want:
    * (a) convert from s32 to f32
    * (b) multiply by 1/256
    */
   for (int def = 0; def < 2; ++def) {
      if (!i->defExists(def))
         continue;
      bld.mkCvt(OP_CVT, TYPE_F32, i->getDef(def), TYPE_S32, i->getDef(def));
      bld.mkOp2(OP_MUL, TYPE_F32, i->getDef(def),
                i->getDef(def), bld.loadImm(NULL, 1.0f / 256));
   }
   return true;
}

bool
NV50LoweringPreSSA::handleTXQ(TexInstruction *i)
{
   Value *ms, *ms_x, *ms_y;
   if (i->tex.query == TXQ_DIMS) {
      if (i->tex.target.isMS()) {
         bld.setPosition(i, true);
         loadTexMsInfo(i->tex.r * 4 * 2, &ms, &ms_x, &ms_y);
         int d = 0;
         if (i->tex.mask & 1) {
            bld.mkOp2(OP_SHR, TYPE_U32, i->getDef(d), i->getDef(d), ms_x);
            d++;
         }
         if (i->tex.mask & 2) {
            bld.mkOp2(OP_SHR, TYPE_U32, i->getDef(d), i->getDef(d), ms_y);
            d++;
         }
      }
      return true;
   }
   assert(i->tex.query == TXQ_TYPE);
   assert(i->tex.mask == 4);

   loadTexMsInfo(i->tex.r * 4 * 2, &ms, &ms_x, &ms_y);
   bld.mkOp2(OP_SHL, TYPE_U32, i->getDef(0), bld.loadImm(NULL, 1), ms);
   i->bb->remove(i);

   return true;
}

bool
NV50LoweringPreSSA::handleSUQ(TexInstruction *suq)
{
   const int dim = suq->tex.target.getDim();
   const int arg = dim + (suq->tex.target.isArray() || suq->tex.target.isCube());
   int mask = suq->tex.mask;
   int slot = suq->tex.r;
   int c, d;

   for (c = 0, d = 0; c < 3; ++c, mask >>= 1) {
      if (c >= arg || !(mask & 1))
         continue;

      int offset;

      if (c == 1 && suq->tex.target == TEX_TARGET_1D_ARRAY) {
         offset = NV50_SU_INFO_SIZE(2);
      } else {
         offset = NV50_SU_INFO_SIZE(c);
      }
      bld.mkMov(suq->getDef(d++), loadSuInfo(slot, offset));
      if (c == 2 && suq->tex.target.isCube())
         bld.mkOp2(OP_DIV, TYPE_U32, suq->getDef(d - 1), suq->getDef(d - 1),
                   bld.loadImm(NULL, 6));
   }

   if (mask & 1) {
      if (suq->tex.target.isMS()) {
         Value *ms_x = loadSuInfo(slot, NV50_SU_INFO_MS(0));
         Value *ms_y = loadSuInfo(slot, NV50_SU_INFO_MS(1));
         Value *ms = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getScratch(), ms_x, ms_y);
         bld.mkOp2(OP_SHL, TYPE_U32, suq->getDef(d++), bld.loadImm(NULL, 1), ms);
      } else {
         bld.mkMov(suq->getDef(d++), bld.loadImm(NULL, 1));
      }
   }

   bld.remove(suq);
   return true;
}

bool
NV50LoweringPreSSA::handleBUFQ(Instruction *bufq)
{
   bufq->op = OP_MOV;
   bufq->setSrc(0, loadSuInfo(bufq->getSrc(0)->reg.fileIndex, NV50_SU_INFO_SIZE_X));
   bufq->setIndirect(0, 0, NULL);
   bufq->setIndirect(0, 1, NULL);
   return true;
}

bool
NV50LoweringPreSSA::handleSET(Instruction *i)
{
   if (i->dType == TYPE_F32) {
      bld.setPosition(i, true);
      i->dType = TYPE_U32;
      bld.mkOp1(OP_ABS, TYPE_S32, i->getDef(0), i->getDef(0));
      bld.mkCvt(OP_CVT, TYPE_F32, i->getDef(0), TYPE_S32, i->getDef(0));
   }
   return true;
}

bool
NV50LoweringPreSSA::handleSLCT(CmpInstruction *i)
{
   Value *src0 = bld.getSSA();
   Value *src1 = bld.getSSA();
   Value *pred = bld.getScratch(1, FILE_FLAGS);

   Value *v0 = i->getSrc(0);
   Value *v1 = i->getSrc(1);
   // XXX: these probably shouldn't be immediates in the first place ...
   if (v0->asImm())
      v0 = bld.mkMov(bld.getSSA(), v0)->getDef(0);
   if (v1->asImm())
      v1 = bld.mkMov(bld.getSSA(), v1)->getDef(0);

   bld.setPosition(i, true);
   bld.mkMov(src0, v0)->setPredicate(CC_NE, pred);
   bld.mkMov(src1, v1)->setPredicate(CC_EQ, pred);
   bld.mkOp2(OP_UNION, i->dType, i->getDef(0), src0, src1);

   bld.setPosition(i, false);
   i->op = OP_SET;
   i->setFlagsDef(0, pred);
   i->dType = TYPE_U8;
   i->setSrc(0, i->getSrc(2));
   i->setSrc(2, NULL);
   i->setSrc(1, bld.loadImm(NULL, 0));

   return true;
}

bool
NV50LoweringPreSSA::handleSELP(Instruction *i)
{
   Value *src0 = bld.getSSA();
   Value *src1 = bld.getSSA();

   Value *v0 = i->getSrc(0);
   Value *v1 = i->getSrc(1);
   if (v0->asImm())
      v0 = bld.mkMov(bld.getSSA(), v0)->getDef(0);
   if (v1->asImm())
      v1 = bld.mkMov(bld.getSSA(), v1)->getDef(0);

   bld.mkMov(src0, v0)->setPredicate(CC_NE, i->getSrc(2));
   bld.mkMov(src1, v1)->setPredicate(CC_EQ, i->getSrc(2));
   bld.mkOp2(OP_UNION, i->dType, i->getDef(0), src0, src1);
   delete_Instruction(prog, i);
   return true;
}

bool
NV50LoweringPreSSA::handleCALL(Instruction *i)
{
   if (prog->getType() == Program::TYPE_COMPUTE) {
      // Add implicit "thread id" argument in $r0 to the function
      i->setSrc(i->srcCount(), tid);
   }
   return true;
}

bool
NV50LoweringPreSSA::handlePRECONT(Instruction *i)
{
   delete_Instruction(prog, i);
   return true;
}

bool
NV50LoweringPreSSA::handleCONT(Instruction *i)
{
   i->op = OP_BRA;
   return true;
}

bool
NV50LoweringPreSSA::handleRDSV(Instruction *i)
{
   Symbol *sym = i->getSrc(0)->asSym();
   uint32_t addr = targ->getSVAddress(FILE_SHADER_INPUT, sym);
   Value *def = i->getDef(0);
   SVSemantic sv = sym->reg.data.sv.sv;
   int idx = sym->reg.data.sv.index;

   if (addr >= 0x400) // mov $sreg
      return true;

   switch (sv) {
   case SV_POSITION:
      assert(prog->getType() == Program::TYPE_FRAGMENT);
      bld.mkInterp(NV50_IR_INTERP_LINEAR, i->getDef(0), addr, NULL);
      break;
   case SV_FACE:
      bld.mkInterp(NV50_IR_INTERP_FLAT, def, addr, NULL);
      if (i->dType == TYPE_F32) {
         bld.mkOp2(OP_OR, TYPE_U32, def, def, bld.mkImm(0x00000001));
         bld.mkOp1(OP_NEG, TYPE_S32, def, def);
         bld.mkCvt(OP_CVT, TYPE_F32, def, TYPE_S32, def);
      }
      break;
   case SV_NCTAID:
   case SV_CTAID:
   case SV_NTID: {
      Value *x = bld.getSSA(2);
      bld.mkOp1(OP_LOAD, TYPE_U16, x,
                bld.mkSymbol(FILE_MEMORY_SHARED, 0, TYPE_U16, addr));
      bld.mkCvt(OP_CVT, TYPE_U32, def, TYPE_U16, x);
      break;
   }
   case SV_TID:
      if (idx == 0) {
         bld.mkOp2(OP_AND, TYPE_U32, def, tid, bld.mkImm(0x0000ffff));
      } else if (idx == 1) {
         bld.mkOp2(OP_AND, TYPE_U32, def, tid, bld.mkImm(0x03ff0000));
         bld.mkOp2(OP_SHR, TYPE_U32, def, def, bld.mkImm(16));
      } else if (idx == 2) {
         bld.mkOp2(OP_SHR, TYPE_U32, def, tid, bld.mkImm(26));
      } else {
         bld.mkMov(def, bld.mkImm(0));
      }
      break;
   case SV_COMBINED_TID:
      bld.mkMov(def, tid);
      break;
   case SV_SAMPLE_POS: {
      Value *off = new_LValue(func, FILE_ADDRESS);
      bld.mkOp1(OP_RDSV, TYPE_U32, def, bld.mkSysVal(SV_SAMPLE_INDEX, 0));
      bld.mkOp2(OP_SHL, TYPE_U32, off, def, bld.mkImm(3));
      bld.mkLoad(TYPE_F32,
                 def,
                 bld.mkSymbol(
                       FILE_MEMORY_CONST, prog->driver->io.auxCBSlot,
                       TYPE_U32, prog->driver->io.sampleInfoBase + 4 * idx),
                 off);
      break;
   }
   case SV_THREAD_KILL:
      // Not actually supported. But it's implementation-dependent, so we can
      // always just say it's not a helper.
      bld.mkMov(def, bld.loadImm(NULL, 0));
      break;
   default:
      bld.mkFetch(i->getDef(0), i->dType,
                  FILE_SHADER_INPUT, addr, i->getIndirect(0, 0), NULL);
      break;
   }
   bld.getBB()->remove(i);
   return true;
}

bool
NV50LoweringPreSSA::handleDIV(Instruction *i)
{
   if (!isFloatType(i->dType))
      return true;
   bld.setPosition(i, false);
   Instruction *rcp = bld.mkOp1(OP_RCP, i->dType, bld.getSSA(), i->getSrc(1));
   i->op = OP_MUL;
   i->setSrc(1, rcp->getDef(0));
   return true;
}

bool
NV50LoweringPreSSA::handleSQRT(Instruction *i)
{
   bld.setPosition(i, true);
   i->op = OP_RSQ;
   bld.mkOp1(OP_RCP, i->dType, i->getDef(0), i->getDef(0));

   return true;
}

bool
NV50LoweringPreSSA::handleEXPORT(Instruction *i)
{
   if (prog->getType() == Program::TYPE_FRAGMENT) {
      if (i->getIndirect(0, 0)) {
         // TODO: redirect to l[] here, load to GPRs at exit
         return false;
      } else {
         int id = i->getSrc(0)->reg.data.offset / 4; // in 32 bit reg units

         i->op = OP_MOV;
         i->subOp = NV50_IR_SUBOP_MOV_FINAL;
         i->src(0).set(i->src(1));
         i->setSrc(1, NULL);
         i->setDef(0, new_LValue(func, FILE_GPR));
         i->getDef(0)->reg.data.id = id;

         prog->maxGPR = MAX2(prog->maxGPR, id * 2);
      }
   }
   return true;
}

// Handle indirect addressing in geometry shaders:
//
// ld $r0 a[$a1][$a2+k] ->
// ld $r0 a[($a1 + $a2 * $vstride) + k], where k *= $vstride is implicit
//
bool
NV50LoweringPreSSA::handleLOAD(Instruction *i)
{
   ValueRef src = i->src(0);
   Symbol *sym = i->getSrc(0)->asSym();

   if (prog->getType() == Program::TYPE_COMPUTE) {
      if (sym->inFile(FILE_MEMORY_SHARED) ||
          sym->inFile(FILE_MEMORY_BUFFER) ||
          sym->inFile(FILE_MEMORY_GLOBAL)) {
         return handleLDST(i);
      }
   }

   if (src.isIndirect(1)) {
      assert(prog->getType() == Program::TYPE_GEOMETRY);
      Value *addr = i->getIndirect(0, 1);

      if (src.isIndirect(0)) {
         // base address is in an address register, so move to a GPR
         Value *base = bld.getScratch();
         bld.mkMov(base, addr);

         Symbol *sv = bld.mkSysVal(SV_VERTEX_STRIDE, 0);
         Value *vstride = bld.mkOp1v(OP_RDSV, TYPE_U32, bld.getSSA(), sv);
         Value *attrib = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                                    i->getIndirect(0, 0), bld.mkImm(2));

         // Calculate final address: addr = base + attr*vstride; use 16-bit
         // multiplication since 32-bit would be lowered to multiple
         // instructions, and we only need the low 16 bits of the result
         Value *a[2], *b[2];
         bld.mkSplit(a, 2, attrib);
         bld.mkSplit(b, 2, vstride);
         Value *sum = bld.mkOp3v(OP_MAD, TYPE_U16, bld.getSSA(), a[0], b[0],
                                 base);

         // move address from GPR into an address register
         addr = bld.getSSA(2, FILE_ADDRESS);
         bld.mkMov(addr, sum);
      }

      i->setIndirect(0, 1, NULL);
      i->setIndirect(0, 0, addr);
   }

   return true;
}

bool
NV50LoweringPreSSA::handleSharedATOM(Instruction *atom)
{
   assert(atom->src(0).getFile() == FILE_MEMORY_SHARED);

   BasicBlock *currBB = atom->bb;
   BasicBlock *tryLockBB = atom->bb->splitBefore(atom, false);
   BasicBlock *joinBB = atom->bb->splitAfter(atom);
   BasicBlock *setAndUnlockBB = new BasicBlock(func);
   BasicBlock *failLockBB = new BasicBlock(func);

   bld.setPosition(currBB, true);
   assert(!currBB->joinAt);
   currBB->joinAt = bld.mkFlow(OP_JOINAT, joinBB, CC_ALWAYS, NULL);

   bld.mkFlow(OP_BRA, tryLockBB, CC_ALWAYS, NULL);
   currBB->cfg.attach(&tryLockBB->cfg, Graph::Edge::TREE);

   bld.setPosition(tryLockBB, true);

   Instruction *ld =
      bld.mkLoad(TYPE_U32, atom->getDef(0), atom->getSrc(0)->asSym(),
                 atom->getIndirect(0, 0));
   Value *locked = bld.getSSA(1, FILE_FLAGS);
   if (prog->getTarget()->getChipset() >= 0xa0) {
      ld->setFlagsDef(1, locked);
      ld->subOp = NV50_IR_SUBOP_LOAD_LOCKED;
   } else {
      bld.mkMov(locked, bld.loadImm(NULL, 2))
         ->flagsDef = 0;
   }

   bld.mkFlow(OP_BRA, setAndUnlockBB, CC_LT, locked);
   bld.mkFlow(OP_BRA, failLockBB, CC_ALWAYS, NULL);
   tryLockBB->cfg.attach(&failLockBB->cfg, Graph::Edge::CROSS);
   tryLockBB->cfg.attach(&setAndUnlockBB->cfg, Graph::Edge::TREE);

   tryLockBB->cfg.detach(&joinBB->cfg);
   bld.remove(atom);

   bld.setPosition(setAndUnlockBB, true);
   Value *stVal;
   if (atom->subOp == NV50_IR_SUBOP_ATOM_EXCH) {
      // Read the old value, and write the new one.
      stVal = atom->getSrc(1);
   } else if (atom->subOp == NV50_IR_SUBOP_ATOM_CAS) {
      CmpInstruction *set =
         bld.mkCmp(OP_SET, CC_EQ, TYPE_U32, bld.getSSA(1, FILE_FLAGS),
                   TYPE_U32, ld->getDef(0), atom->getSrc(1));

      Instruction *selp =
         bld.mkOp3(OP_SELP, TYPE_U32, bld.getSSA(), atom->getSrc(2),
                   ld->getDef(0), set->getDef(0));
      stVal = selp->getDef(0);

      handleSELP(selp);
   } else {
      operation op;

      switch (atom->subOp) {
      case NV50_IR_SUBOP_ATOM_ADD:
         op = OP_ADD;
         break;
      case NV50_IR_SUBOP_ATOM_AND:
         op = OP_AND;
         break;
      case NV50_IR_SUBOP_ATOM_OR:
         op = OP_OR;
         break;
      case NV50_IR_SUBOP_ATOM_XOR:
         op = OP_XOR;
         break;
      case NV50_IR_SUBOP_ATOM_MIN:
         op = OP_MIN;
         break;
      case NV50_IR_SUBOP_ATOM_MAX:
         op = OP_MAX;
         break;
      default:
         assert(0);
         return false;
      }

      Instruction *i =
         bld.mkOp2(op, atom->dType, bld.getSSA(), ld->getDef(0),
                   atom->getSrc(1));

      stVal = i->getDef(0);
   }

   Instruction *store = bld.mkStore(OP_STORE, TYPE_U32, atom->getSrc(0)->asSym(),
               atom->getIndirect(0, 0), stVal);
   if (prog->getTarget()->getChipset() >= 0xa0) {
      store->subOp = NV50_IR_SUBOP_STORE_UNLOCKED;
   }

   bld.mkFlow(OP_BRA, failLockBB, CC_ALWAYS, NULL);
   setAndUnlockBB->cfg.attach(&failLockBB->cfg, Graph::Edge::TREE);

   // Loop until the lock is acquired.
   bld.setPosition(failLockBB, true);
   bld.mkFlow(OP_BRA, tryLockBB, CC_GEU, locked);
   bld.mkFlow(OP_BRA, joinBB, CC_ALWAYS, NULL);
   failLockBB->cfg.attach(&tryLockBB->cfg, Graph::Edge::BACK);
   failLockBB->cfg.attach(&joinBB->cfg, Graph::Edge::TREE);

   bld.setPosition(joinBB, false);
   bld.mkFlow(OP_JOIN, NULL, CC_ALWAYS, NULL)->fixed = 1;

   return true;
}

bool
NV50LoweringPreSSA::handleLDST(Instruction *i)
{
   ValueRef src = i->src(0);
   Symbol *sym = i->getSrc(0)->asSym();

   if (prog->getType() != Program::TYPE_COMPUTE) {
      return true;
   }

   // Buffers just map directly to the different global memory spaces
   if (sym->inFile(FILE_MEMORY_BUFFER)) {
      sym->reg.file = FILE_MEMORY_GLOBAL;
   }

   if (sym->inFile(FILE_MEMORY_SHARED)) {

      if (src.isIndirect(0)) {
         Value *addr = i->getIndirect(0, 0);

         if (!addr->inFile(FILE_ADDRESS)) {
            // Move address from GPR into an address register
            Value *new_addr = bld.getSSA(2, FILE_ADDRESS);
            bld.mkMov(new_addr, addr);

            i->setIndirect(0, 0, new_addr);
         }
      }

      if (i->op == OP_ATOM)
         handleSharedATOM(i);
   } else if (sym->inFile(FILE_MEMORY_GLOBAL)) {
      // All global access must be indirect. There are no instruction forms
      // with direct access.
      Value *addr = i->getIndirect(0, 0);

      Value *offset = bld.loadImm(bld.getSSA(), sym->reg.data.offset);
      Value *sum;
      if (addr != NULL)
         sum = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(), addr,
                          offset);
      else
         sum = offset;

      i->setIndirect(0, 0, sum);
      sym->reg.data.offset = 0;
   }

   return true;
}

bool
NV50LoweringPreSSA::handleMEMBAR(Instruction *i)
{
   // For global memory, apparently doing a bunch of reads at different
   // addresses forces things to get sufficiently flushed.
   if (i->subOp & NV50_IR_SUBOP_MEMBAR_GL) {
      uint8_t b = prog->driver->io.auxCBSlot;
      Value *base =
         bld.mkLoadv(TYPE_U32, bld.mkSymbol(FILE_MEMORY_CONST, b, TYPE_U32,
                                            prog->driver->io.membarOffset), NULL);
      Value *physid = bld.mkOp1v(OP_RDSV, TYPE_U32, bld.getSSA(), bld.mkSysVal(SV_PHYSID, 0));
      Value *off = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                              bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(),
                                         physid, bld.loadImm(NULL, 0x1f)),
                              bld.loadImm(NULL, 2));
      base = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(), base, off);
      Symbol *gmemMembar = bld.mkSymbol(FILE_MEMORY_GLOBAL, prog->driver->io.gmemMembar, TYPE_U32, 0);
      for (int i = 0; i < 8; i++) {
         if (i != 0) {
            base = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(), base, bld.loadImm(NULL, 0x100));
         }
         bld.mkLoad(TYPE_U32, bld.getSSA(), gmemMembar, base)
            ->fixed = 1;
      }
   }

   // Both global and shared memory barriers also need a regular control bar
   // TODO: double-check this is the case
   i->op = OP_BAR;
   i->subOp = NV50_IR_SUBOP_BAR_SYNC;
   i->setSrc(0, bld.mkImm(0u));
   i->setSrc(1, bld.mkImm(0u));

   return true;
}

// The type that bests represents how each component can be stored when packed.
static DataType
getPackedType(const TexInstruction::ImgFormatDesc *t, int c)
{
   switch (t->type) {
   case FLOAT: return t->bits[c] == 16 ? TYPE_F16 : TYPE_F32;
   case UNORM: return t->bits[c] == 8 ? TYPE_U8 : TYPE_U16;
   case SNORM: return t->bits[c] == 8 ? TYPE_S8 : TYPE_S16;
   case UINT:
      return (t->bits[c] == 8 ? TYPE_U8 :
              (t->bits[c] <= 16 ? TYPE_U16 : TYPE_U32));
   case SINT:
      return (t->bits[c] == 8 ? TYPE_S8 :
              (t->bits[c] <= 16 ? TYPE_S16 : TYPE_S32));
   }
   return TYPE_NONE;
}

// The type that the rest of the shader expects to process this image type in.
static DataType
getShaderType(const ImgType type) {
   switch (type) {
   case FLOAT:
   case UNORM:
   case SNORM:
      return TYPE_F32;
   case UINT:
      return TYPE_U32;
   case SINT:
      return TYPE_S32;
   default:
      assert(!"Impossible type");
      return TYPE_NONE;
   }
}

// Reads the raw coordinates out of the input instruction, and returns a
// single-value coordinate which is what the hardware expects to receive in a
// ld/st op.
Value *
NV50LoweringPreSSA::processSurfaceCoords(TexInstruction *su)
{
   const int slot = su->tex.r;
   const int dim = su->tex.target.getDim();
   const int arg = dim + (su->tex.target.isArray() || su->tex.target.isCube());

   const TexInstruction::ImgFormatDesc *format = su->tex.format;
   const uint16_t bytes = (format->bits[0] + format->bits[1] +
                           format->bits[2] + format->bits[3]) / 8;
   uint16_t shift = ffs(bytes) - 1;

   // Buffer sizes don't necessarily fit in 16-bit values
   if (su->tex.target == TEX_TARGET_BUFFER) {
      return bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                        su->getSrc(0), bld.loadImm(NULL, (uint32_t)shift));
   }

   // For buffers, we just need the byte offset. And for 2d buffers we want
   // the x coordinate in bytes as well.
   Value *coords[3] = {};
   for (int i = 0; i < arg; i++) {
      Value *src[2];
      bld.mkSplit(src, 2, su->getSrc(i));
      coords[i] = src[0];
      // For 1d-images, we want the y coord to be 0, which it will be here.
      if (i == 0)
         coords[1] = src[1];
   }

   coords[0] = bld.mkOp2v(OP_SHL, TYPE_U16, bld.getSSA(2),
                          coords[0], bld.loadImm(NULL, shift));

   if (su->tex.target.isMS()) {
      Value *ms_x = loadSuInfo16(slot, NV50_SU_INFO_MS(0));
      Value *ms_y = loadSuInfo16(slot, NV50_SU_INFO_MS(1));
      coords[0] = bld.mkOp2v(OP_SHL, TYPE_U16, bld.getSSA(2), coords[0], ms_x);
      coords[1] = bld.mkOp2v(OP_SHL, TYPE_U16, bld.getSSA(2), coords[1], ms_y);
   }

   // If there are more dimensions, we just want the y-offset. But that needs
   // to be adjusted up by the y-stride for array images.
   if (su->tex.target.isArray() || su->tex.target.isCube()) {
      Value *index = coords[dim];
      Value *height = loadSuInfo16(slot, NV50_SU_INFO_STRIDE_Y);
      Instruction *mul = bld.mkOp2(OP_MUL, TYPE_U32, bld.getSSA(4), index, height);
      mul->sType = TYPE_U16;
      Value *muls[2];
      bld.mkSplit(muls, 2, mul->getDef(0));
      if (dim > 1)
         coords[1] = bld.mkOp2v(OP_ADD, TYPE_U16, bld.getSSA(2), coords[1], muls[0]);
      else
         coords[1] = muls[0];
   }

   // 3d is special-cased. Note that a single "slice" of a 3d image may
   // also be attached as 2d, so we have to do the same 3d processing for
   // 2d as well, just in case. In order to remap a 3d image onto a 2d
   // image, we have to retile it "by hand".
   if (su->tex.target == TEX_TARGET_3D || su->tex.target == TEX_TARGET_2D) {
      Value *z = loadSuInfo16(slot, NV50_SU_INFO_OFFSET_Z);
      Value *y_size_aligned = loadSuInfo16(slot, NV50_SU_INFO_STRIDE_Y);
      // Add the z coordinate for actual 3d-images
      if (dim > 2)
         coords[2] = bld.mkOp2v(OP_ADD, TYPE_U16, bld.getSSA(2), z, coords[2]);
      else
         coords[2] = z;

      // Compute the surface parameters from tile shifts
      Value *tile_shift[3];
      Value *tile_size[3];
      Value *tile_mask[3];
      // We only ever use one kind of X-tiling.
      tile_shift[0] = bld.loadImm(NULL, (uint16_t)6);
      tile_size[0] = bld.loadImm(NULL, (uint16_t)64);
      tile_mask[0] = bld.loadImm(NULL, (uint16_t)63);
      // Fetch the "real" tiling parameters of the underlying surface
      for (int i = 1; i < 3; i++) {
         tile_shift[i] = loadSuInfo16(slot, NV50_SU_INFO_TILE_SHIFT(i));
         tile_size[i] = bld.mkOp2v(OP_SHL, TYPE_U16, bld.getSSA(2), bld.loadImm(NULL, (uint16_t)1), tile_shift[i]);
         tile_mask[i] = bld.mkOp2v(OP_ADD, TYPE_U16, bld.getSSA(2), tile_size[i], bld.loadImm(NULL, (uint16_t)-1));
      }

      // Compute the location of given coordinate, both inside the tile as
      // well as which (linearly-laid out) tile it's in.
      Value *coord_in_tile[3];
      Value *tile[3];
      for (int i = 0; i < 3; i++) {
         coord_in_tile[i] = bld.mkOp2v(OP_AND, TYPE_U16, bld.getSSA(2), coords[i], tile_mask[i]);
         tile[i] = bld.mkOp2v(OP_SHR, TYPE_U16, bld.getSSA(2), coords[i], tile_shift[i]);
      }

      // Based on the "real" tiling parameters, compute x/y coordinates in the
      // larger surface with 2d tiling that was supplied to the hardware. This
      // was determined and verified with the help of the tiling pseudocode in
      // the envytools docs.
      //
      // adj_x = x_coord_in_tile + x_tile * x_tile_size * z_tile_size +
      //         z_coord_in_tile * x_tile_size
      // adj_y = y_coord_in_tile + y_tile * y_tile_size +
      //         z_tile * y_tile_size * y_tiles
      //
      // Note: STRIDE_Y = y_tile_size * y_tiles

      coords[0] = bld.mkOp2v(
            OP_ADD, TYPE_U16, bld.getSSA(2),
            bld.mkOp2v(OP_ADD, TYPE_U16, bld.getSSA(2),
                       coord_in_tile[0],
                       bld.mkOp2v(OP_SHL, TYPE_U16, bld.getSSA(2),
                                  tile[0],
                                  bld.mkOp2v(OP_ADD, TYPE_U16, bld.getSSA(2),
                                             tile_shift[2], tile_shift[0]))),
            bld.mkOp2v(OP_SHL, TYPE_U16, bld.getSSA(2),
                       coord_in_tile[2], tile_shift[0]));

      Instruction *mul = bld.mkOp2(OP_MUL, TYPE_U32, bld.getSSA(4),
                                   tile[2], y_size_aligned);
      mul->sType = TYPE_U16;
      Value *muls[2];
      bld.mkSplit(muls, 2, mul->getDef(0));

      coords[1] = bld.mkOp2v(
            OP_ADD, TYPE_U16, bld.getSSA(2),
            muls[0],
            bld.mkOp2v(OP_ADD, TYPE_U16, bld.getSSA(2),
                       coord_in_tile[1],
                       bld.mkOp2v(OP_SHL, TYPE_U16, bld.getSSA(2),
                                  tile[1], tile_shift[1])));
   }

   return bld.mkOp2v(OP_MERGE, TYPE_U32, bld.getSSA(), coords[0], coords[1]);
}

// This is largely a copy of NVC0LoweringPass::convertSurfaceFormat, but
// adjusted to make use of 16-bit math where possible.
bool
NV50LoweringPreSSA::handleSULDP(TexInstruction *su)
{
   const int slot = su->tex.r;
   assert(!su->getIndirectR());

   bld.setPosition(su, false);

   const TexInstruction::ImgFormatDesc *format = su->tex.format;
   const int bytes = (su->tex.format->bits[0] +
                      su->tex.format->bits[1] +
                      su->tex.format->bits[2] +
                      su->tex.format->bits[3]) / 8;
   DataType ty = typeOfSize(bytes);

   Value *coord = processSurfaceCoords(su);

   Value *untypedDst[4] = {};
   Value *typedDst[4] = {};
   int i;
   for (i = 0; i < bytes / 4; i++)
      untypedDst[i] = bld.getSSA();
   if (bytes < 4)
      untypedDst[0] = bld.getSSA();

   for (i = 0; i < 4; i++)
      typedDst[i] = su->getDef(i);

   Instruction *load = bld.mkLoad(ty, NULL, bld.mkSymbol(FILE_MEMORY_GLOBAL, slot, ty, 0), coord);
   for (i = 0; i < 4 && untypedDst[i]; i++)
      load->setDef(i, untypedDst[i]);

   // Unpack each component into the typed dsts
   int bits = 0;
   for (int i = 0; i < 4; bits += format->bits[i], i++) {
      if (!typedDst[i])
         continue;

      if (i >= format->components) {
         if (format->type == FLOAT ||
             format->type == UNORM ||
             format->type == SNORM)
            bld.loadImm(typedDst[i], i == 3 ? 1.0f : 0.0f);
         else
            bld.loadImm(typedDst[i], i == 3 ? 1 : 0);
         continue;
      }

      // Get just that component's data into the relevant place
      if (format->bits[i] == 32)
         bld.mkMov(typedDst[i], untypedDst[i]);
      else if (format->bits[i] == 16) {
         // We can always convert directly from the appropriate half of the
         // loaded value into the typed result.
         Value *src[2];
         bld.mkSplit(src, 2, untypedDst[i / 2]);
         bld.mkCvt(OP_CVT, getShaderType(format->type), typedDst[i],
                   getPackedType(format, i), src[i & 1]);
      }
      else if (format->bits[i] == 8) {
         // Same approach as for 16 bits, but we have to massage the value a
         // bit more, since we have to get the appropriate 8 bits from the
         // half-register. In all cases, we can CVT from a 8-bit source, so we
         // only have to shift when we want the upper 8 bits.
         Value *src[2], *shifted;
         bld.mkSplit(src, 2, untypedDst[0]);
         DataType packedType = getPackedType(format, i);
         if (i & 1)
            shifted = bld.mkOp2v(OP_SHR, TYPE_U16, bld.getSSA(2), src[!!(i & 2)], bld.loadImm(NULL, (uint16_t)8));
         else
            shifted = src[!!(i & 2)];

         bld.mkCvt(OP_CVT, getShaderType(format->type), typedDst[i],
                   packedType, shifted);
      }
      else {
         // The options are 10, 11, and 2. Get it into a 32-bit reg, then
         // shift/mask. That's where it'll have to end up anyways. For signed,
         // we have to make sure to get sign-extension, so we actually have to
         // shift *up* first, and then shift down. There's no advantage to
         // AND'ing, so we don't.
         DataType ty = TYPE_U32;
         if (format->type == SNORM || format->type == SINT) {
            ty = TYPE_S32;
         }

         // Poor man's EXTBF
         bld.mkOp2(
               OP_SHR, ty, typedDst[i],
               bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(), untypedDst[0], bld.loadImm(NULL, 32 - bits - format->bits[i])),
               bld.loadImm(NULL, 32 - format->bits[i]));

         // If the stored data is already in the appropriate type, we don't
         // have to do anything. Convert to float for the *NORM formats.
         if (format->type == UNORM || format->type == SNORM)
            bld.mkCvt(OP_CVT, TYPE_F32, typedDst[i], TYPE_U32, typedDst[i]);
      }

      // Normalize / convert as necessary
      if (format->type == UNORM)
         bld.mkOp2(OP_MUL, TYPE_F32, typedDst[i], typedDst[i], bld.loadImm(NULL, 1.0f / ((1 << format->bits[i]) - 1)));
      else if (format->type == SNORM)
         bld.mkOp2(OP_MUL, TYPE_F32, typedDst[i], typedDst[i], bld.loadImm(NULL, 1.0f / ((1 << (format->bits[i] - 1)) - 1)));
      else if (format->type == FLOAT && format->bits[i] < 16) {
         // We expect the value to be in the low bits of the register, so we
         // have to shift back up.
         bld.mkOp2(OP_SHL, TYPE_U32, typedDst[i], typedDst[i], bld.loadImm(NULL, 15 - format->bits[i]));
         Value *src[2];
         bld.mkSplit(src, 2, typedDst[i]);
         bld.mkCvt(OP_CVT, TYPE_F32, typedDst[i], TYPE_F16, src[0]);
      }
   }

   if (format->bgra) {
      std::swap(typedDst[0], typedDst[2]);
   }

   bld.getBB()->remove(su);
   return true;
}

bool
NV50LoweringPreSSA::handleSUREDP(TexInstruction *su)
{
   const int slot = su->tex.r;
   const int dim = su->tex.target.getDim();
   const int arg = dim + (su->tex.target.isArray() || su->tex.target.isCube());
   assert(!su->getIndirectR());

   bld.setPosition(su, false);

   Value *coord = processSurfaceCoords(su);

   // This is guaranteed to be a 32-bit format. So there's nothing to
   // pack/unpack.
   Instruction *atom = bld.mkOp2(
         OP_ATOM, su->dType, su->getDef(0),
         bld.mkSymbol(FILE_MEMORY_GLOBAL, slot, TYPE_U32, 0), su->getSrc(arg));
   if (su->subOp == NV50_IR_SUBOP_ATOM_CAS)
      atom->setSrc(2, su->getSrc(arg + 1));
   atom->setIndirect(0, 0, coord);
   atom->subOp = su->subOp;

   bld.getBB()->remove(su);
   return true;
}

bool
NV50LoweringPreSSA::handleSUSTP(TexInstruction *su)
{
   const int slot = su->tex.r;
   const int dim = su->tex.target.getDim();
   const int arg = dim + (su->tex.target.isArray() || su->tex.target.isCube());
   assert(!su->getIndirectR());

   bld.setPosition(su, false);

   const TexInstruction::ImgFormatDesc *format = su->tex.format;
   const int bytes = (su->tex.format->bits[0] +
                      su->tex.format->bits[1] +
                      su->tex.format->bits[2] +
                      su->tex.format->bits[3]) / 8;
   DataType ty = typeOfSize(bytes);

   Value *coord = processSurfaceCoords(su);

   // The packed values we will eventually store into memory
   Value *untypedDst[4] = {};
   // Each component's packed representation, in 16-bit registers (only used
   // where appropriate)
   Value *untypedDst16[4] = {};
   // The original values that are being packed
   Value *typedDst[4] = {};
   int i;

   for (i = 0; i < bytes / 4; i++)
      untypedDst[i] = bld.getSSA();
   for (i = 0; i < format->components; i++)
      untypedDst16[i] = bld.getSSA(2);
   // Make sure we get at least one of each value allocated for the
   // super-narrow formats.
   if (bytes < 4)
      untypedDst[0] = bld.getSSA();
   if (bytes < 2)
      untypedDst16[0] = bld.getSSA(2);

   for (i = 0; i < 4; i++) {
      typedDst[i] = bld.getSSA();
      bld.mkMov(typedDst[i], su->getSrc(arg + i));
   }

   if (format->bgra) {
      std::swap(typedDst[0], typedDst[2]);
   }

   // Pack each component into the untyped dsts.
   int bits = 0;
   for (int i = 0; i < format->components; bits += format->bits[i], i++) {
      // Un-normalize / convert as necessary
      if (format->type == UNORM)
         bld.mkOp2(OP_MUL, TYPE_F32, typedDst[i], typedDst[i], bld.loadImm(NULL, 1.0f * ((1 << format->bits[i]) - 1)));
      else if (format->type == SNORM)
         bld.mkOp2(OP_MUL, TYPE_F32, typedDst[i], typedDst[i], bld.loadImm(NULL, 1.0f * ((1 << (format->bits[i] - 1)) - 1)));

      // There is nothing to convert/pack for 32-bit values
      if (format->bits[i] == 32) {
         bld.mkMov(untypedDst[i], typedDst[i]);
         continue;
      }

      // The remainder of the cases will naturally want to deal in 16-bit
      // registers. We will put these into untypedDst16 and then merge them
      // together later.
      if (format->type == FLOAT && format->bits[i] < 16) {
         bld.mkCvt(OP_CVT, TYPE_F16, untypedDst16[i], TYPE_F32, typedDst[i]);
         bld.mkOp2(OP_SHR, TYPE_U16, untypedDst16[i], untypedDst16[i], bld.loadImm(NULL, (uint16_t)(15 - format->bits[i])));

         // For odd bit sizes, it's easier to pack it into the final
         // destination directly.
         Value *tmp = bld.getSSA();
         bld.mkCvt(OP_CVT, TYPE_U32, tmp, TYPE_U16, untypedDst16[i]);
         if (i == 0) {
            untypedDst[0] = tmp;
         } else {
            bld.mkOp2(OP_SHL, TYPE_U32, tmp, tmp, bld.loadImm(NULL, bits));
            bld.mkOp2(OP_OR, TYPE_U32, untypedDst[0], untypedDst[0], tmp);
         }
      } else if (format->bits[i] == 16) {
         // We can always convert the shader value into the packed value
         // directly here
         bld.mkCvt(OP_CVT, getPackedType(format, i), untypedDst16[i],
                   getShaderType(format->type), typedDst[i]);
      } else if (format->bits[i] < 16) {
         DataType packedType = getPackedType(format, i);
         DataType shaderType = getShaderType(format->type);
         // We can't convert F32 to U8/S8 directly, so go to U16/S16 first.
         if (shaderType == TYPE_F32 && typeSizeof(packedType) == 1) {
            packedType = format->type == SNORM ? TYPE_S16 : TYPE_U16;
         }
         bld.mkCvt(OP_CVT, packedType, untypedDst16[i], shaderType, typedDst[i]);
         // TODO: clamp for 10- and 2-bit sizes. Also, due to the oddness of
         // the size, it's easier to dump them into a 32-bit value and OR
         // everything later.
         if (format->bits[i] != 8) {
            // Restrict value to the appropriate bits (although maybe supposed
            // to clamp instead?)
            bld.mkOp2(OP_AND, TYPE_U16, untypedDst16[i], untypedDst16[i], bld.loadImm(NULL, (uint16_t)((1 << format->bits[i]) - 1)));
            // And merge into final packed value
            Value *tmp = bld.getSSA();
            bld.mkCvt(OP_CVT, TYPE_U32, tmp, TYPE_U16, untypedDst16[i]);
            if (i == 0) {
               untypedDst[0] = tmp;
            } else {
               bld.mkOp2(OP_SHL, TYPE_U32, tmp, tmp, bld.loadImm(NULL, bits));
               bld.mkOp2(OP_OR, TYPE_U32, untypedDst[0], untypedDst[0], tmp);
            }
         } else if (i & 1) {
            // Shift the 8-bit value up (so that it can be OR'd later)
            bld.mkOp2(OP_SHL, TYPE_U16, untypedDst16[i], untypedDst16[i], bld.loadImm(NULL, (uint16_t)(bits % 16)));
         } else if (packedType != TYPE_U8) {
            // S8 (or the *16 if converted from float) will all have high bits
            // set, so AND them out.
            bld.mkOp2(OP_AND, TYPE_U16, untypedDst16[i], untypedDst16[i], bld.loadImm(NULL, (uint16_t)0xff));
         }
      }
   }

   // OR pairs of 8-bit values together (into the even value)
   if (format->bits[0] == 8) {
      for (i = 0; i < 2 && untypedDst16[2 * i] && untypedDst16[2 * i + 1]; i++)
         bld.mkOp2(OP_OR, TYPE_U16, untypedDst16[2 * i], untypedDst16[2 * i], untypedDst16[2 * i + 1]);
   }

   // We'll always want to have at least a 32-bit source register for the store
   Instruction *merge = bld.mkOp(OP_MERGE, bytes < 4 ? TYPE_U32 : ty, bld.getSSA(bytes < 4 ? 4 : bytes));
   if (format->bits[0] == 32) {
      for (i = 0; i < 4 && untypedDst[i]; i++)
         merge->setSrc(i, untypedDst[i]);
   } else if (format->bits[0] == 16) {
      for (i = 0; i < 4 && untypedDst16[i]; i++)
         merge->setSrc(i, untypedDst16[i]);
      if (i == 1)
         merge->setSrc(i, bld.getSSA(2));
   } else if (format->bits[0] == 8) {
      for (i = 0; i < 2 && untypedDst16[2 * i]; i++)
         merge->setSrc(i, untypedDst16[2 * i]);
      if (i == 1)
         merge->setSrc(i, bld.getSSA(2));
   } else {
      merge->setSrc(0, untypedDst[0]);
   }

   bld.mkStore(OP_STORE, ty, bld.mkSymbol(FILE_MEMORY_GLOBAL, slot, TYPE_U32, 0), coord, merge->getDef(0));

   bld.getBB()->remove(su);
   return true;
}

bool
NV50LoweringPreSSA::handlePFETCH(Instruction *i)
{
   assert(prog->getType() == Program::TYPE_GEOMETRY);

   // NOTE: cannot use getImmediate here, not in SSA form yet, move to
   // later phase if that assertion ever triggers:

   ImmediateValue *imm = i->getSrc(0)->asImm();
   assert(imm);

   assert(imm->reg.data.u32 <= 127); // TODO: use address reg if that happens

   if (i->srcExists(1)) {
      // indirect addressing of vertex in primitive space

      LValue *val = bld.getScratch();
      Value *ptr = bld.getSSA(2, FILE_ADDRESS);
      bld.mkOp2v(OP_SHL, TYPE_U32, ptr, i->getSrc(1), bld.mkImm(2));
      bld.mkOp2v(OP_PFETCH, TYPE_U32, val, imm, ptr);

      // NOTE: PFETCH directly to an $aX only works with direct addressing
      i->op = OP_SHL;
      i->setSrc(0, val);
      i->setSrc(1, bld.mkImm(0));
   }

   return true;
}

// Set flags according to predicate and make the instruction read $cX.
void
NV50LoweringPreSSA::checkPredicate(Instruction *insn)
{
   Value *pred = insn->getPredicate();
   Value *cdst;

   // FILE_PREDICATE will simply be changed to FLAGS on conversion to SSA
   if (!pred ||
       pred->reg.file == FILE_FLAGS || pred->reg.file == FILE_PREDICATE)
      return;

   cdst = bld.getSSA(1, FILE_FLAGS);

   bld.mkCmp(OP_SET, CC_NEU, insn->dType, cdst, insn->dType, bld.loadImm(NULL, 0), pred);

   insn->setPredicate(insn->cc, cdst);
}

//
// - add quadop dance for texturing
// - put FP outputs in GPRs
// - convert instruction sequences
//
bool
NV50LoweringPreSSA::visit(Instruction *i)
{
   bld.setPosition(i, false);

   if (i->cc != CC_ALWAYS)
      checkPredicate(i);

   switch (i->op) {
   case OP_TEX:
   case OP_TXF:
   case OP_TXG:
      return handleTEX(i->asTex());
   case OP_TXB:
      return handleTXB(i->asTex());
   case OP_TXL:
      return handleTXL(i->asTex());
   case OP_TXD:
      return handleTXD(i->asTex());
   case OP_TXLQ:
      return handleTXLQ(i->asTex());
   case OP_TXQ:
      return handleTXQ(i->asTex());
   case OP_EX2:
      bld.mkOp1(OP_PREEX2, TYPE_F32, i->getDef(0), i->getSrc(0));
      i->setSrc(0, i->getDef(0));
      break;
   case OP_SET:
      return handleSET(i);
   case OP_SLCT:
      return handleSLCT(i->asCmp());
   case OP_SELP:
      return handleSELP(i);
   case OP_DIV:
      return handleDIV(i);
   case OP_SQRT:
      return handleSQRT(i);
   case OP_EXPORT:
      return handleEXPORT(i);
   case OP_LOAD:
      return handleLOAD(i);
   case OP_MEMBAR:
      return handleMEMBAR(i);
   case OP_ATOM:
   case OP_STORE:
      return handleLDST(i);
   case OP_SULDP:
      return handleSULDP(i->asTex());
   case OP_SUSTP:
      return handleSUSTP(i->asTex());
   case OP_SUREDP:
      return handleSUREDP(i->asTex());
   case OP_SUQ:
      return handleSUQ(i->asTex());
   case OP_BUFQ:
      return handleBUFQ(i);
   case OP_RDSV:
      return handleRDSV(i);
   case OP_CALL:
      return handleCALL(i);
   case OP_PRECONT:
      return handlePRECONT(i);
   case OP_CONT:
      return handleCONT(i);
   case OP_PFETCH:
      return handlePFETCH(i);
   default:
      break;
   }
   return true;
}

bool
TargetNV50::runLegalizePass(Program *prog, CGStage stage) const
{
   bool ret = false;

   if (stage == CG_STAGE_PRE_SSA) {
      NV50LoweringPreSSA pass(prog);
      ret = pass.run(prog, false, true);
   } else
   if (stage == CG_STAGE_SSA) {
      if (!prog->targetPriv)
         prog->targetPriv = new std::list<Instruction *>();
      NV50LegalizeSSA pass(prog);
      ret = pass.run(prog, false, true);
   } else
   if (stage == CG_STAGE_POST_RA) {
      NV50LegalizePostRA pass;
      ret = pass.run(prog, false, true);
      if (prog->targetPriv)
         delete reinterpret_cast<std::list<Instruction *> *>(prog->targetPriv);
   }
   return ret;
}

} // namespace nv50_ir
