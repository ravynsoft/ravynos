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

#include "nv50_ir_target_nvc0.h"
#include "nv50_ir_lowering_nvc0.h"

#include <limits>

namespace nv50_ir {

#define QOP_ADD  0
#define QOP_SUBR 1
#define QOP_SUB  2
#define QOP_MOV2 3

//             UL UR LL LR
#define QUADOP(q, r, s, t)                      \
   ((QOP_##q << 6) | (QOP_##r << 4) |           \
    (QOP_##s << 2) | (QOP_##t << 0))

void
NVC0LegalizeSSA::handleDIV(Instruction *i)
{
   FlowInstruction *call;
   int builtin;

   bld.setPosition(i, false);

   // Generate movs to the input regs for the call we want to generate
   for (int s = 0; i->srcExists(s); ++s) {
      Instruction *ld = i->getSrc(s)->getInsn();
      // check if we are moving an immediate, propagate it in that case
      if (!ld || ld->fixed || (ld->op != OP_LOAD && ld->op != OP_MOV) ||
            !(ld->src(0).getFile() == FILE_IMMEDIATE))
         bld.mkMovToReg(s, i->getSrc(s));
      else {
         assert(ld->getSrc(0) != NULL);
         bld.mkMovToReg(s, ld->getSrc(0));
         // Clear the src, to make code elimination possible here before we
         // delete the instruction i later
         i->setSrc(s, NULL);
         if (ld->isDead())
            delete_Instruction(prog, ld);
      }
   }

   switch (i->dType) {
   case TYPE_U32: builtin = NVC0_BUILTIN_DIV_U32; break;
   case TYPE_S32: builtin = NVC0_BUILTIN_DIV_S32; break;
   default:
      return;
   }
   call = bld.mkFlow(OP_CALL, NULL, CC_ALWAYS, NULL);
   bld.mkMovFromReg(i->getDef(0), i->op == OP_DIV ? 0 : 1);
   bld.mkClobber(FILE_GPR, (i->op == OP_DIV) ? 0xe : 0xd, 2);
   bld.mkClobber(FILE_PREDICATE, (i->dType == TYPE_S32) ? 0xf : 0x3, 0);

   call->fixed = 1;
   call->absolute = call->builtin = 1;
   call->target.builtin = builtin;
   delete_Instruction(prog, i);
}

void
NVC0LegalizeSSA::handleRCPRSQLib(Instruction *i, Value *src[])
{
   FlowInstruction *call;
   Value *def[2];
   int builtin;

   def[0] = bld.mkMovToReg(0, src[0])->getDef(0);
   def[1] = bld.mkMovToReg(1, src[1])->getDef(0);

   if (i->op == OP_RCP)
      builtin = NVC0_BUILTIN_RCP_F64;
   else
      builtin = NVC0_BUILTIN_RSQ_F64;

   call = bld.mkFlow(OP_CALL, NULL, CC_ALWAYS, NULL);
   def[0] = bld.getSSA();
   def[1] = bld.getSSA();
   bld.mkMovFromReg(def[0], 0);
   bld.mkMovFromReg(def[1], 1);
   bld.mkClobber(FILE_GPR, 0x3fc, 2);
   bld.mkClobber(FILE_PREDICATE, i->op == OP_RSQ ? 0x3 : 0x1, 0);
   bld.mkOp2(OP_MERGE, TYPE_U64, i->getDef(0), def[0], def[1]);

   call->fixed = 1;
   call->absolute = call->builtin = 1;
   call->target.builtin = builtin;
   delete_Instruction(prog, i);

   prog->fp64 = true;
}

void
NVC0LegalizeSSA::handleRCPRSQ(Instruction *i)
{
   assert(i->dType == TYPE_F64);
   // There are instructions that will compute the high 32 bits of the 64-bit
   // float. We will just stick 0 in the bottom 32 bits.

   bld.setPosition(i, false);

   // 1. Take the source and it up.
   Value *src[2], *dst[2], *def = i->getDef(0);
   bld.mkSplit(src, 4, i->getSrc(0));

   int chip = prog->getTarget()->getChipset();
   if (chip >= NVISA_GK104_CHIPSET) {
      handleRCPRSQLib(i, src);
      return;
   }

   // 2. We don't care about the low 32 bits of the destination. Stick a 0 in.
   dst[0] = bld.loadImm(NULL, 0);
   dst[1] = bld.getSSA();

   // 3. The new version of the instruction takes the high 32 bits of the
   // source and outputs the high 32 bits of the destination.
   i->setSrc(0, src[1]);
   i->setDef(0, dst[1]);
   i->setType(TYPE_F32);
   i->subOp = NV50_IR_SUBOP_RCPRSQ_64H;

   // 4. Recombine the two dst pieces back into the original destination.
   bld.setPosition(i, true);
   bld.mkOp2(OP_MERGE, TYPE_U64, def, dst[0], dst[1]);
}

void
NVC0LegalizeSSA::handleFTZ(Instruction *i)
{
   // Only want to flush float inputs
   assert(i->sType == TYPE_F32);

   // If we're already flushing denorms (and NaN's) to zero, no need for this.
   if (i->dnz)
      return;

   // Only certain classes of operations can flush
   OpClass cls = prog->getTarget()->getOpClass(i->op);
   if (cls != OPCLASS_ARITH && cls != OPCLASS_COMPARE &&
       cls != OPCLASS_CONVERT)
      return;

   i->ftz = true;
}

void
NVC0LegalizeSSA::handleTEXLOD(TexInstruction *i)
{
   if (i->tex.levelZero)
      return;

   ImmediateValue lod;

   // The LOD argument comes right after the coordinates (before depth bias,
   // offsets, etc).
   int arg = i->tex.target.getArgCount();

   // SM30+ stores the indirect handle as a separate arg, which comes before
   // the LOD.
   if (prog->getTarget()->getChipset() >= NVISA_GK104_CHIPSET &&
       i->tex.rIndirectSrc >= 0)
      arg++;
   // SM20 stores indirect handle combined with array coordinate
   if (prog->getTarget()->getChipset() < NVISA_GK104_CHIPSET &&
       !i->tex.target.isArray() &&
       i->tex.rIndirectSrc >= 0)
      arg++;

   if (!i->src(arg).getImmediate(lod) || !lod.isInteger(0))
      return;

   if (i->op == OP_TXL)
      i->op = OP_TEX;
   i->tex.levelZero = true;
   i->moveSources(arg + 1, -1);
}

void
NVC0LegalizeSSA::handleShift(Instruction *lo)
{
   Value *shift = lo->getSrc(1);
   Value *dst64 = lo->getDef(0);
   Value *src[2], *dst[2];
   operation op = lo->op;

   bld.setPosition(lo, false);

   bld.mkSplit(src, 4, lo->getSrc(0));

   // SM30 and prior don't have the fancy new SHF.L/R ops. So the logic has to
   // be completely emulated. For SM35+, we can use the more directed SHF
   // operations.
   if (prog->getTarget()->getChipset() < NVISA_GK20A_CHIPSET) {
      // The strategy here is to handle shifts >= 32 and less than 32 as
      // separate parts.
      //
      // For SHL:
      // If the shift is <= 32, then
      //   (HI,LO) << x = (HI << x | (LO >> (32 - x)), LO << x)
      // If the shift is > 32, then
      //   (HI,LO) << x = (LO << (x - 32), 0)
      //
      // For SHR:
      // If the shift is <= 32, then
      //   (HI,LO) >> x = (HI >> x, (HI << (32 - x)) | LO >> x)
      // If the shift is > 32, then
      //   (HI,LO) >> x = (0, HI >> (x - 32))
      //
      // Note that on NVIDIA hardware, a shift > 32 yields a 0 value, which we
      // can use to our advantage. Also note the structural similarities
      // between the right/left cases. The main difference is swapping hi/lo
      // on input and output.

      Value *x32_minus_shift, *pred, *hi1, *hi2;
      DataType type = isSignedIntType(lo->dType) ? TYPE_S32 : TYPE_U32;
      operation antiop = op == OP_SHR ? OP_SHL : OP_SHR;
      if (op == OP_SHR)
         std::swap(src[0], src[1]);
      bld.mkOp2(OP_ADD, TYPE_U32, (x32_minus_shift = bld.getSSA()), shift, bld.mkImm(0x20))
         ->src(0).mod = Modifier(NV50_IR_MOD_NEG);
      bld.mkCmp(OP_SET, CC_LE, TYPE_U8, (pred = bld.getSSA(1, FILE_PREDICATE)),
                TYPE_U32, shift, bld.mkImm(32));
      // Compute HI (shift <= 32)
      bld.mkOp2(OP_OR, TYPE_U32, (hi1 = bld.getSSA()),
                bld.mkOp2v(op, TYPE_U32, bld.getSSA(), src[1], shift),
                bld.mkOp2v(antiop, TYPE_U32, bld.getSSA(), src[0], x32_minus_shift))
         ->setPredicate(CC_P, pred);
      // Compute LO (all shift values)
      bld.mkOp2(op, type, (dst[0] = bld.getSSA()), src[0], shift);
      // Compute HI (shift > 32)
      bld.mkOp2(op, type, (hi2 = bld.getSSA()), src[0],
                bld.mkOp1v(OP_NEG, TYPE_S32, bld.getSSA(), x32_minus_shift))
         ->setPredicate(CC_NOT_P, pred);
      bld.mkOp2(OP_UNION, TYPE_U32, (dst[1] = bld.getSSA()), hi1, hi2);
      if (op == OP_SHR)
         std::swap(dst[0], dst[1]);
      bld.mkOp2(OP_MERGE, TYPE_U64, dst64, dst[0], dst[1]);
      delete_Instruction(prog, lo);
      return;
   }

   Instruction *hi = new_Instruction(func, op, TYPE_U32);
   lo->bb->insertAfter(lo, hi);

   hi->sType = lo->sType;
   lo->dType = TYPE_U32;

   hi->setDef(0, (dst[1] = bld.getSSA()));
   if (lo->op == OP_SHR)
      hi->subOp |= NV50_IR_SUBOP_SHIFT_HIGH;
   lo->setDef(0, (dst[0] = bld.getSSA()));

   bld.setPosition(hi, true);

   if (lo->op == OP_SHL)
      std::swap(hi, lo);

   hi->setSrc(0, new_ImmediateValue(prog, 0u));
   hi->setSrc(1, shift);
   hi->setSrc(2, lo->op == OP_SHL ? src[0] : src[1]);

   lo->setSrc(0, src[0]);
   lo->setSrc(1, shift);
   lo->setSrc(2, src[1]);

   bld.mkOp2(OP_MERGE, TYPE_U64, dst64, dst[0], dst[1]);
}

void
NVC0LegalizeSSA::handleSET(CmpInstruction *cmp)
{
   DataType hTy = cmp->sType == TYPE_S64 ? TYPE_S32 : TYPE_U32;
   Value *carry;
   Value *src0[2], *src1[2];
   bld.setPosition(cmp, false);

   bld.mkSplit(src0, 4, cmp->getSrc(0));
   bld.mkSplit(src1, 4, cmp->getSrc(1));
   bld.mkOp2(OP_SUB, hTy, NULL, src0[0], src1[0])
      ->setFlagsDef(0, (carry = bld.getSSA(1, FILE_FLAGS)));
   cmp->setFlagsSrc(cmp->srcCount(), carry);
   cmp->setSrc(0, src0[1]);
   cmp->setSrc(1, src1[1]);
   cmp->sType = hTy;
}

void
NVC0LegalizeSSA::handleBREV(Instruction *i)
{
   i->op = OP_EXTBF;
   i->subOp = NV50_IR_SUBOP_EXTBF_REV;
   i->setSrc(1, bld.mkImm(0x2000));
}

bool
NVC0LegalizeSSA::visit(Function *fn)
{
   bld.setProgram(fn->getProgram());
   return true;
}

bool
NVC0LegalizeSSA::visit(BasicBlock *bb)
{
   Instruction *next;
   for (Instruction *i = bb->getEntry(); i; i = next) {
      next = i->next;

      if (i->sType == TYPE_F32 && prog->getType() != Program::TYPE_COMPUTE)
         handleFTZ(i);

      switch (i->op) {
      case OP_DIV:
      case OP_MOD:
         if (i->sType != TYPE_F32)
            handleDIV(i);
         break;
      case OP_RCP:
      case OP_RSQ:
         if (i->dType == TYPE_F64)
            handleRCPRSQ(i);
         break;
      case OP_TXL:
      case OP_TXF:
         handleTEXLOD(i->asTex());
         break;
      case OP_SHR:
      case OP_SHL:
         if (typeSizeof(i->sType) == 8)
            handleShift(i);
         break;
      case OP_SET:
      case OP_SET_AND:
      case OP_SET_OR:
      case OP_SET_XOR:
         if (typeSizeof(i->sType) == 8 && i->sType != TYPE_F64)
            handleSET(i->asCmp());
         break;
      case OP_BREV:
         handleBREV(i);
         break;
      default:
         break;
      }
   }
   return true;
}

NVC0LegalizePostRA::NVC0LegalizePostRA(const Program *prog)
   : rZero(NULL),
     carry(NULL),
     pOne(NULL),
     needTexBar(prog->getTarget()->getChipset() >= 0xe0 &&
                prog->getTarget()->getChipset() < 0x110)
{
}

bool
NVC0LegalizePostRA::insnDominatedBy(const Instruction *later,
                                    const Instruction *early) const
{
   if (early->bb == later->bb)
      return early->serial < later->serial;
   return later->bb->dominatedBy(early->bb);
}

void
NVC0LegalizePostRA::addTexUse(std::list<TexUse> &uses,
                              Instruction *usei, const Instruction *texi)
{
   bool add = true;
   bool dominated = insnDominatedBy(usei, texi);
   // Uses before the tex have to all be included. Just because an earlier
   // instruction dominates another instruction doesn't mean that there's no
   // way to get from the tex to the later instruction. For example you could
   // have nested loops, with the tex in the inner loop, and uses before it in
   // both loops - even though the outer loop's instruction would dominate the
   // inner's, we still want a texbar before the inner loop's instruction.
   //
   // However we can still use the eliding logic between uses dominated by the
   // tex instruction, as that is unambiguously correct.
   if (dominated) {
      for (std::list<TexUse>::iterator it = uses.begin(); it != uses.end();) {
         if (it->after) {
            if (insnDominatedBy(usei, it->insn)) {
               add = false;
               break;
            }
            if (insnDominatedBy(it->insn, usei)) {
               it = uses.erase(it);
               continue;
            }
         }
         ++it;
      }
   }
   if (add)
      uses.push_back(TexUse(usei, texi, dominated));
}

// While it might be tempting to use the an algorithm that just looks at tex
// uses, not all texture results are guaranteed to be used on all paths. In
// the case where along some control flow path a texture result is never used,
// we might reuse that register for something else, creating a
// write-after-write hazard. So we have to manually look through all
// instructions looking for ones that reference the registers in question.
void
NVC0LegalizePostRA::findFirstUses(
   Instruction *texi, std::list<TexUse> &uses)
{
   int minGPR = texi->def(0).rep()->reg.data.id;
   int maxGPR = minGPR + texi->def(0).rep()->reg.size / 4 - 1;

   std::unordered_set<const BasicBlock *> visited;
   findFirstUsesBB(minGPR, maxGPR, texi->next, texi, uses, visited);
}

void
NVC0LegalizePostRA::findFirstUsesBB(
   int minGPR, int maxGPR, Instruction *start,
   const Instruction *texi, std::list<TexUse> &uses,
   std::unordered_set<const BasicBlock *> &visited)
{
   const BasicBlock *bb = start->bb;

   // We don't process the whole bb the first time around. This is correct,
   // however we might be in a loop and hit this BB again, and need to process
   // the full thing. So only mark a bb as visited if we processed it from the
   // beginning.
   if (start == bb->getEntry()) {
      if (visited.find(bb) != visited.end())
         return;
      visited.insert(bb);
   }

   for (Instruction *insn = start; insn != bb->getExit(); insn = insn->next) {
      if (insn->isNop())
         continue;

      for (int d = 0; insn->defExists(d); ++d) {
         const Value *def = insn->def(d).rep();
         if (insn->def(d).getFile() != FILE_GPR ||
             def->reg.data.id + def->reg.size / 4 - 1 < minGPR ||
             def->reg.data.id > maxGPR)
            continue;
         addTexUse(uses, insn, texi);
         return;
      }

      for (int s = 0; insn->srcExists(s); ++s) {
         const Value *src = insn->src(s).rep();
         if (insn->src(s).getFile() != FILE_GPR ||
             src->reg.data.id + src->reg.size / 4 - 1 < minGPR ||
             src->reg.data.id > maxGPR)
            continue;
         addTexUse(uses, insn, texi);
         return;
      }
   }

   for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
      findFirstUsesBB(minGPR, maxGPR, BasicBlock::get(ei.getNode())->getEntry(),
                      texi, uses, visited);
   }
}

// Texture barriers:
// This pass is a bit long and ugly and can probably be optimized.
//
// 1. obtain a list of TEXes and their outputs' first use(s)
// 2. calculate the barrier level of each first use (minimal number of TEXes,
//    over all paths, between the TEX and the use in question)
// 3. for each barrier, if all paths from the source TEX to that barrier
//    contain a barrier of lesser level, it can be culled
bool
NVC0LegalizePostRA::insertTextureBarriers(Function *fn)
{
   std::list<TexUse> *uses;
   std::vector<Instruction *> texes;
   std::vector<int> bbFirstTex;
   std::vector<int> bbFirstUse;
   std::vector<int> texCounts;
   std::vector<TexUse> useVec;
   ArrayList insns;

   fn->orderInstructions(insns);

   texCounts.resize(fn->allBBlocks.getSize(), 0);
   bbFirstTex.resize(fn->allBBlocks.getSize(), insns.getSize());
   bbFirstUse.resize(fn->allBBlocks.getSize(), insns.getSize());

   // tag BB CFG nodes by their id for later
   for (ArrayList::Iterator i = fn->allBBlocks.iterator(); !i.end(); i.next()) {
      BasicBlock *bb = reinterpret_cast<BasicBlock *>(i.get());
      if (bb)
         bb->cfg.tag = bb->getId();
   }

   // gather the first uses for each TEX
   for (unsigned int i = 0; i < insns.getSize(); ++i) {
      Instruction *tex = reinterpret_cast<Instruction *>(insns.get(i));
      if (isTextureOp(tex->op)) {
         texes.push_back(tex);
         if (!texCounts.at(tex->bb->getId()))
            bbFirstTex[tex->bb->getId()] = texes.size() - 1;
         texCounts[tex->bb->getId()]++;
      }
   }
   insns.clear();
   if (texes.empty())
      return false;
   uses = new std::list<TexUse>[texes.size()];
   if (!uses)
      return false;
   for (size_t i = 0; i < texes.size(); ++i) {
      findFirstUses(texes[i], uses[i]);
   }

   // determine the barrier level at each use
   for (size_t i = 0; i < texes.size(); ++i) {
      for (std::list<TexUse>::iterator u = uses[i].begin(); u != uses[i].end();
           ++u) {
         BasicBlock *tb = texes[i]->bb;
         BasicBlock *ub = u->insn->bb;
         if (tb == ub) {
            u->level = 0;
            for (size_t j = i + 1; j < texes.size() &&
                    texes[j]->bb == tb && texes[j]->serial < u->insn->serial;
                 ++j)
               u->level++;
         } else {
            u->level = fn->cfg.findLightestPathWeight(&tb->cfg,
                                                      &ub->cfg, texCounts);
            if (u->level < 0) {
               WARN("Failed to find path TEX -> TEXBAR\n");
               u->level = 0;
               continue;
            }
            // this counted all TEXes in the origin block, correct that
            u->level -= i - bbFirstTex.at(tb->getId()) + 1 /* this TEX */;
            // and did not count the TEXes in the destination block, add those
            for (size_t j = bbFirstTex.at(ub->getId()); j < texes.size() &&
                    texes[j]->bb == ub && texes[j]->serial < u->insn->serial;
                 ++j)
               u->level++;
         }
         assert(u->level >= 0);
         useVec.push_back(*u);
      }
   }
   delete[] uses;

   // insert the barriers
   for (size_t i = 0; i < useVec.size(); ++i) {
      Instruction *prev = useVec[i].insn->prev;
      if (useVec[i].level < 0)
         continue;
      if (prev && prev->op == OP_TEXBAR) {
         if (prev->subOp > useVec[i].level)
            prev->subOp = useVec[i].level;
         prev->setSrc(prev->srcCount(), useVec[i].tex->getDef(0));
      } else {
         Instruction *bar = new_Instruction(func, OP_TEXBAR, TYPE_NONE);
         bar->fixed = 1;
         bar->subOp = useVec[i].level;
         // make use explicit to ease latency calculation
         bar->setSrc(bar->srcCount(), useVec[i].tex->getDef(0));
         useVec[i].insn->bb->insertBefore(useVec[i].insn, bar);
      }
   }

   if (fn->getProgram()->optLevel < 3)
      return true;

   std::vector<Limits> limitT, limitB, limitS; // entry, exit, single

   limitT.resize(fn->allBBlocks.getSize(), Limits(0, 0));
   limitB.resize(fn->allBBlocks.getSize(), Limits(0, 0));
   limitS.resize(fn->allBBlocks.getSize());

   // cull unneeded barriers (should do that earlier, but for simplicity)
   IteratorRef bi = fn->cfg.iteratorCFG();
   // first calculate min/max outstanding TEXes for each BB
   for (bi->reset(); !bi->end(); bi->next()) {
      Graph::Node *n = reinterpret_cast<Graph::Node *>(bi->get());
      BasicBlock *bb = BasicBlock::get(n);
      int min = 0;
      int max = std::numeric_limits<int>::max();
      for (Instruction *i = bb->getFirst(); i; i = i->next) {
         if (isTextureOp(i->op)) {
            min++;
            if (max < std::numeric_limits<int>::max())
               max++;
         } else
         if (i->op == OP_TEXBAR) {
            min = MIN2(min, i->subOp);
            max = MIN2(max, i->subOp);
         }
      }
      // limits when looking at an isolated block
      limitS[bb->getId()].min = min;
      limitS[bb->getId()].max = max;
   }
   // propagate the min/max values
   for (unsigned int l = 0; l <= fn->loopNestingBound; ++l) {
      for (bi->reset(); !bi->end(); bi->next()) {
         Graph::Node *n = reinterpret_cast<Graph::Node *>(bi->get());
         BasicBlock *bb = BasicBlock::get(n);
         const int bbId = bb->getId();
         for (Graph::EdgeIterator ei = n->incident(); !ei.end(); ei.next()) {
            BasicBlock *in = BasicBlock::get(ei.getNode());
            const int inId = in->getId();
            limitT[bbId].min = MAX2(limitT[bbId].min, limitB[inId].min);
            limitT[bbId].max = MAX2(limitT[bbId].max, limitB[inId].max);
         }
         // I just hope this is correct ...
         if (limitS[bbId].max == std::numeric_limits<int>::max()) {
            // no barrier
            limitB[bbId].min = limitT[bbId].min + limitS[bbId].min;
            limitB[bbId].max = limitT[bbId].max + limitS[bbId].min;
         } else {
            // block contained a barrier
            limitB[bbId].min = MIN2(limitS[bbId].max,
                                    limitT[bbId].min + limitS[bbId].min);
            limitB[bbId].max = MIN2(limitS[bbId].max,
                                    limitT[bbId].max + limitS[bbId].min);
         }
      }
   }
   // finally delete unnecessary barriers
   for (bi->reset(); !bi->end(); bi->next()) {
      Graph::Node *n = reinterpret_cast<Graph::Node *>(bi->get());
      BasicBlock *bb = BasicBlock::get(n);
      Instruction *prev = NULL;
      Instruction *next;
      int max = limitT[bb->getId()].max;
      for (Instruction *i = bb->getFirst(); i; i = next) {
         next = i->next;
         if (i->op == OP_TEXBAR) {
            if (i->subOp >= max) {
               delete_Instruction(prog, i);
               i = NULL;
            } else {
               max = i->subOp;
               if (prev && prev->op == OP_TEXBAR && prev->subOp >= max) {
                  delete_Instruction(prog, prev);
                  prev = NULL;
               }
            }
         } else
         if (isTextureOp(i->op)) {
            max++;
         }
         if (i && !i->isNop())
            prev = i;
      }
   }
   return true;
}

bool
NVC0LegalizePostRA::visit(Function *fn)
{
   if (needTexBar)
      insertTextureBarriers(fn);

   rZero = new_LValue(fn, FILE_GPR);
   pOne = new_LValue(fn, FILE_PREDICATE);
   carry = new_LValue(fn, FILE_FLAGS);

   rZero->reg.data.id = (prog->getTarget()->getChipset() >= NVISA_GK20A_CHIPSET) ? 255 : 63;
   carry->reg.data.id = 0;
   pOne->reg.data.id = 7;

   return true;
}

void
NVC0LegalizePostRA::replaceZero(Instruction *i)
{
   for (int s = 0; i->srcExists(s); ++s) {
      if (s == 2 && i->op == OP_SUCLAMP)
         continue;
      if (s == 1 && i->op == OP_SHLADD)
         continue;
      ImmediateValue *imm = i->getSrc(s)->asImm();
      if (imm) {
         if (i->op == OP_SELP && s == 2) {
            i->setSrc(s, pOne);
            if (imm->reg.data.u64 == 0)
               i->src(s).mod = i->src(s).mod ^ Modifier(NV50_IR_MOD_NOT);
         } else if (imm->reg.data.u64 == 0) {
            i->setSrc(s, rZero);
         }
      }
   }
}

// replace CONT with BRA for single unconditional continue
bool
NVC0LegalizePostRA::tryReplaceContWithBra(BasicBlock *bb)
{
   if (bb->cfg.incidentCount() != 2 || bb->getEntry()->op != OP_PRECONT)
      return false;
   Graph::EdgeIterator ei = bb->cfg.incident();
   if (ei.getType() != Graph::Edge::BACK)
      ei.next();
   if (ei.getType() != Graph::Edge::BACK)
      return false;
   BasicBlock *contBB = BasicBlock::get(ei.getNode());

   if (!contBB->getExit() || contBB->getExit()->op != OP_CONT ||
       contBB->getExit()->getPredicate())
      return false;
   contBB->getExit()->op = OP_BRA;
   bb->remove(bb->getEntry()); // delete PRECONT

   ei.next();
   assert(ei.end() || ei.getType() != Graph::Edge::BACK);
   return true;
}

// replace branches to join blocks with join ops
void
NVC0LegalizePostRA::propagateJoin(BasicBlock *bb)
{
   if (bb->getEntry()->op != OP_JOIN || bb->getEntry()->asFlow()->limit)
      return;
   for (Graph::EdgeIterator ei = bb->cfg.incident(); !ei.end(); ei.next()) {
      BasicBlock *in = BasicBlock::get(ei.getNode());
      Instruction *exit = in->getExit();
      if (!exit) {
         in->insertTail(new FlowInstruction(func, OP_JOIN, bb));
         // there should always be a terminator instruction
         WARN("inserted missing terminator in BB:%i\n", in->getId());
      } else
      if (exit->op == OP_BRA) {
         exit->op = OP_JOIN;
         exit->asFlow()->limit = 1; // must-not-propagate marker
      }
   }
   bb->remove(bb->getEntry());
}

// replaces instructions which would end up as f2f or i2i with faster
// alternatives:
//  - fabs(a)     -> fadd(0, abs a)
//  - fneg(a)     -> fadd(neg 0, neg a)
//  - ineg(a)     -> iadd(0, neg a)
//  - fneg(abs a) -> fadd(neg 0, neg abs a)
//  - sat(a)      -> sat add(0, a)
void
NVC0LegalizePostRA::replaceCvt(Instruction *cvt)
{
   if (!isFloatType(cvt->sType) && typeSizeof(cvt->sType) != 4)
      return;
   if (cvt->sType != cvt->dType)
      return;
   // we could make it work, but in this case we have optimizations disabled
   // and we don't really care either way.
   if (cvt->src(0).getFile() != FILE_GPR &&
       cvt->src(0).getFile() != FILE_MEMORY_CONST)
      return;

   Modifier mod0, mod1;

   switch (cvt->op) {
   case OP_ABS:
      if (cvt->src(0).mod)
         return;
      if (!isFloatType(cvt->sType))
         return;
      mod0 = 0;
      mod1 = NV50_IR_MOD_ABS;
      break;
   case OP_NEG:
      if (!isFloatType(cvt->sType) && cvt->src(0).mod)
         return;
      if (isFloatType(cvt->sType) &&
          (cvt->src(0).mod && cvt->src(0).mod != Modifier(NV50_IR_MOD_ABS)))
         return;

      mod0 = isFloatType(cvt->sType) ? NV50_IR_MOD_NEG : 0;
      mod1 = cvt->src(0).mod == Modifier(NV50_IR_MOD_ABS) ?
         NV50_IR_MOD_NEG_ABS : NV50_IR_MOD_NEG;
      break;
   case OP_SAT:
      if (!isFloatType(cvt->sType) && cvt->src(0).mod.abs())
         return;
      mod0 = 0;
      mod1 = cvt->src(0).mod;
      cvt->saturate = true;
      break;
   default:
      return;
   }

   cvt->op = OP_ADD;
   cvt->moveSources(0, 1);
   cvt->setSrc(0, rZero);
   cvt->src(0).mod = mod0;
   cvt->src(1).mod = mod1;
}

bool
NVC0LegalizePostRA::visit(BasicBlock *bb)
{
   Instruction *i, *next;

   // remove pseudo operations and non-fixed no-ops, split 64 bit operations
   for (i = bb->getFirst(); i; i = next) {
      next = i->next;
      if (i->op == OP_EMIT || i->op == OP_RESTART) {
         if (!i->getDef(0)->refCount())
            i->setDef(0, NULL);
         if (i->src(0).getFile() == FILE_IMMEDIATE)
            i->setSrc(0, rZero); // initial value must be 0
         replaceZero(i);
      } else
      if (i->isNop()) {
         bb->remove(i);
      } else
      if (i->op == OP_BAR && i->subOp == NV50_IR_SUBOP_BAR_SYNC &&
          prog->getType() != Program::TYPE_COMPUTE) {
         // It seems like barriers are never required for tessellation since
         // the warp size is 32, and there are always at most 32 tcs threads.
         bb->remove(i);
      } else
      if (i->op == OP_LOAD && i->subOp == NV50_IR_SUBOP_LDC_IS) {
         int offset = i->src(0).get()->reg.data.offset;
         if (abs(offset) >= 0x10000)
            i->src(0).get()->reg.fileIndex += offset >> 16;
         i->src(0).get()->reg.data.offset = (int)(short)offset;
      } else {
         // TODO: Move this to before register allocation for operations that
         // need the $c register !
         if (typeSizeof(i->sType) == 8 || typeSizeof(i->dType) == 8) {
            Instruction *hi;
            hi = BuildUtil::split64BitOpPostRA(func, i, rZero, carry);
            if (hi)
               next = hi;
         }

         if (i->op != OP_MOV && i->op != OP_PFETCH)
            replaceZero(i);

         if (i->op == OP_SAT || i->op == OP_NEG || i->op == OP_ABS)
            replaceCvt(i);
      }
   }
   if (!bb->getEntry())
      return true;

   if (!tryReplaceContWithBra(bb))
      propagateJoin(bb);

   return true;
}

NVC0LoweringPass::NVC0LoweringPass(Program *prog) : targ(prog->getTarget()),
   gpEmitAddress(NULL)
{
   bld.setProgram(prog);
}

bool
NVC0LoweringPass::visit(Function *fn)
{
   if (prog->getType() == Program::TYPE_GEOMETRY) {
      assert(!strncmp(fn->getName(), "MAIN", 4));
      // TODO: when we generate actual functions pass this value along somehow
      bld.setPosition(BasicBlock::get(fn->cfg.getRoot()), false);
      gpEmitAddress = bld.loadImm(NULL, 0)->asLValue();
      if (fn->cfgExit) {
         bld.setPosition(BasicBlock::get(fn->cfgExit)->getExit(), false);
         if (prog->getTarget()->getChipset() >= NVISA_GV100_CHIPSET)
            bld.mkOp1(OP_FINAL, TYPE_NONE, NULL, gpEmitAddress)->fixed = 1;
         bld.mkMovToReg(0, gpEmitAddress);
      }
   }
   return true;
}

bool
NVC0LoweringPass::visit(BasicBlock *bb)
{
   return true;
}

inline Value *
NVC0LoweringPass::loadTexHandle(Value *ptr, unsigned int slot)
{
   uint8_t b = prog->driver->io.auxCBSlot;
   uint32_t off = prog->driver->io.texBindBase + slot * 4;

   if (ptr)
      ptr = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(), ptr, bld.mkImm(2));

   return bld.
      mkLoadv(TYPE_U32, bld.mkSymbol(FILE_MEMORY_CONST, b, TYPE_U32, off), ptr);
}

// move array source to first slot, convert to u16, add indirections
bool
NVC0LoweringPass::handleTEX(TexInstruction *i)
{
   const int dim = i->tex.target.getDim() + i->tex.target.isCube();
   const int arg = i->tex.target.getArgCount() - i->tex.target.isMS();
   const int lyr = arg - 1;
   const int chipset = prog->getTarget()->getChipset();

   /* Only normalize in the non-explicit derivatives case. For explicit
    * derivatives, this is handled in handleManualTXD.
    */
   if (i->tex.target.isCube() && i->dPdx[0].get() == NULL) {
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

   // Arguments to the TEX instruction are a little insane. Even though the
   // encoding is identical between SM20 and SM30, the arguments mean
   // different things between Fermi and Kepler+. A lot of arguments are
   // optional based on flags passed to the instruction. This summarizes the
   // order of things.
   //
   // Fermi:
   //  array/indirect
   //  coords
   //  sample
   //  lod bias
   //  depth compare
   //  offsets:
   //    - tg4: 8 bits each, either 2 (1 offset reg) or 8 (2 offset reg)
   //    - other: 4 bits each, single reg
   //
   // Kepler+:
   //  indirect handle
   //  array (+ offsets for txd in upper 16 bits)
   //  coords
   //  sample
   //  lod bias
   //  depth compare
   //  offsets (same as fermi, except txd which takes it with array)
   //
   // Maxwell (tex):
   //  array
   //  coords
   //  indirect handle
   //  sample
   //  lod bias
   //  depth compare
   //  offsets
   //
   // Maxwell (txd):
   //  indirect handle
   //  coords
   //  array + offsets
   //  derivatives

   if (chipset >= NVISA_GK104_CHIPSET) {
      if (i->tex.rIndirectSrc >= 0 || i->tex.sIndirectSrc >= 0) {
         // XXX this ignores tsc, and assumes a 1:1 mapping
         assert(i->tex.rIndirectSrc >= 0);
         if (!i->tex.bindless) {
            Value *hnd = loadTexHandle(i->getIndirectR(), i->tex.r);
            i->tex.r = 0xff;
            i->tex.s = 0x1f;
            i->setIndirectR(hnd);
         }
         i->setIndirectS(NULL);
      } else if (i->tex.r == i->tex.s || i->op == OP_TXF) {
         if (i->tex.r == 0xffff)
            i->tex.r = prog->driver->io.fbtexBindBase / 4;
         else
            i->tex.r += prog->driver->io.texBindBase / 4;
         i->tex.s  = 0; // only a single cX[] value possible here
      } else {
         Value *hnd = bld.getScratch();
         Value *rHnd = loadTexHandle(NULL, i->tex.r);
         Value *sHnd = loadTexHandle(NULL, i->tex.s);

         bld.mkOp3(OP_INSBF, TYPE_U32, hnd, rHnd, bld.mkImm(0x1400), sHnd);

         i->tex.r = 0; // not used for indirect tex
         i->tex.s = 0;
         i->setIndirectR(hnd);
      }
      if (i->tex.target.isArray()) {
         LValue *layer = new_LValue(func, FILE_GPR);
         Value *src = i->getSrc(lyr);
         /* Vulkan requires that a negative index on a texelFetch() count as
          * out-of-bounds but a negative index on any other texture operation
          * gets clamped to 0.  (See the spec section entitled "(u,v,w,a) to
          * (i,j,k,l,n) Transformation And Array Layer Selection").
          *
          * For TXF, we take a U32 MAX with 0xffff, ensuring that negative
          * array indices clamp to 0xffff and will be considered out-of-bounds
          * by the hardware (there are a maximum of 2048 array indices in an
          * image descriptor).  For everything else, we use a saturating F32
          * to U16 conversion which will clamp negative array indices to 0 and
          * large positive indices to 0xffff.  The hardware will further clamp
          * positive array indices to the maximum in the image descriptor.
          */
         if (i->op == OP_TXF) {
            bld.mkOp2(OP_MIN, TYPE_U32, layer, src, bld.loadImm(NULL, 0xffff));
         } else {
            bld.mkCvt(OP_CVT, TYPE_U16, layer, TYPE_F32, src)->saturate = true;
         }
         if (i->op != OP_TXD || chipset < NVISA_GM107_CHIPSET) {
            for (int s = dim; s >= 1; --s)
               i->setSrc(s, i->getSrc(s - 1));
            i->setSrc(0, layer);
         } else {
            i->setSrc(dim, layer);
         }
      }
      // Move the indirect reference to the first place
      if (i->tex.rIndirectSrc >= 0 && (
                i->op == OP_TXD || chipset < NVISA_GM107_CHIPSET)) {
         Value *hnd = i->getIndirectR();

         i->setIndirectR(NULL);
         i->moveSources(0, 1);
         i->setSrc(0, hnd);
         i->tex.rIndirectSrc = 0;
         i->tex.sIndirectSrc = -1;
      }
      // Move the indirect reference to right after the coords
      else if (i->tex.rIndirectSrc >= 0 && chipset >= NVISA_GM107_CHIPSET) {
         Value *hnd = i->getIndirectR();

         i->setIndirectR(NULL);
         i->moveSources(arg, 1);
         i->setSrc(arg, hnd);
         i->tex.rIndirectSrc = 0;
         i->tex.sIndirectSrc = -1;
      }
   } else
   // (nvc0) generate and move the tsc/tic/array source to the front
   if (i->tex.target.isArray() || i->tex.rIndirectSrc >= 0 || i->tex.sIndirectSrc >= 0) {
      LValue *src = new_LValue(func, FILE_GPR); // 0xttxsaaaa

      Value *ticRel = i->getIndirectR();
      Value *tscRel = i->getIndirectS();

      if (i->tex.r == 0xffff) {
         i->tex.r = 0x20;
         i->tex.s = 0x10;
      }

      if (ticRel) {
         i->setSrc(i->tex.rIndirectSrc, NULL);
         if (i->tex.r)
            ticRel = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getScratch(),
                                ticRel, bld.mkImm(i->tex.r));
      }
      if (tscRel) {
         i->setSrc(i->tex.sIndirectSrc, NULL);
         if (i->tex.s)
            tscRel = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getScratch(),
                                tscRel, bld.mkImm(i->tex.s));
      }

      Value *arrayIndex = i->tex.target.isArray() ? i->getSrc(lyr) : NULL;
      if (arrayIndex) {
         for (int s = dim; s >= 1; --s)
            i->setSrc(s, i->getSrc(s - 1));
         i->setSrc(0, arrayIndex);
      } else {
         i->moveSources(0, 1);
      }

      if (arrayIndex) {
         if (i->op == OP_TXF) {
            bld.mkOp2(OP_MIN, TYPE_U32, src, arrayIndex, bld.loadImm(NULL, 0xffff));
         } else {
            bld.mkCvt(OP_CVT, TYPE_U16, src, TYPE_F32, arrayIndex)->saturate = true;
         }
      } else {
         bld.loadImm(src, 0);
      }

      if (ticRel)
         bld.mkOp3(OP_INSBF, TYPE_U32, src, ticRel, bld.mkImm(0x0917), src);
      if (tscRel)
         bld.mkOp3(OP_INSBF, TYPE_U32, src, tscRel, bld.mkImm(0x0710), src);

      i->setSrc(0, src);
   }

   // For nvc0, the sample id has to be in the second operand, as the offset
   // does. Right now we don't know how to pass both in, and this case can't
   // happen with OpenGL. On nve0, the sample id is part of the texture
   // coordinate argument.
   assert(chipset >= NVISA_GK104_CHIPSET ||
          !i->tex.useOffsets || !i->tex.target.isMS());

   // offset is between lod and dc
   if (i->tex.useOffsets) {
      int n, c;
      int s = i->srcCount(0xff, true);
      if (i->op != OP_TXD || chipset < NVISA_GK104_CHIPSET) {
         if (i->tex.target.isShadow())
            s--;
         if (i->srcExists(s)) // move potential predicate out of the way
            i->moveSources(s, 1);
         if (i->tex.useOffsets == 4 && i->srcExists(s + 1))
            i->moveSources(s + 1, 1);
      }
      if (i->op == OP_TXG) {
         // Either there is 1 offset, which goes into the 2 low bytes of the
         // first source, or there are 4 offsets, which go into 2 sources (8
         // values, 1 byte each).
         Value *offs[2] = {NULL, NULL};
         for (n = 0; n < i->tex.useOffsets; n++) {
            for (c = 0; c < 2; ++c) {
               if ((n % 2) == 0 && c == 0)
                  bld.mkMov(offs[n / 2] = bld.getScratch(), i->offset[n][c].get());
               else
                  bld.mkOp3(OP_INSBF, TYPE_U32,
                            offs[n / 2],
                            i->offset[n][c].get(),
                            bld.mkImm(0x800 | ((n * 16 + c * 8) % 32)),
                            offs[n / 2]);
            }
         }
         i->setSrc(s, offs[0]);
         if (offs[1])
            i->setSrc(s + 1, offs[1]);
      } else {
         unsigned imm = 0;
         assert(i->tex.useOffsets == 1);
         for (c = 0; c < 3; ++c) {
            ImmediateValue val;
            if (!i->offset[0][c].getImmediate(val))
               assert(!"non-immediate offset passed to non-TXG");
            imm |= (val.reg.data.u32 & 0xf) << (c * 4);
         }
         if (i->op == OP_TXD && chipset >= NVISA_GK104_CHIPSET) {
            // The offset goes into the upper 16 bits of the array index. So
            // create it if it's not already there, and INSBF it if it already
            // is.
            s = (i->tex.rIndirectSrc >= 0) ? 1 : 0;
            if (chipset >= NVISA_GM107_CHIPSET)
               s += dim;
            if (i->tex.target.isArray()) {
               Value *offset = bld.getScratch();
               bld.mkOp3(OP_INSBF, TYPE_U32, offset,
                         bld.loadImm(NULL, imm), bld.mkImm(0xc10),
                         i->getSrc(s));
               i->setSrc(s, offset);
            } else {
               i->moveSources(s, 1);
               i->setSrc(s, bld.loadImm(NULL, imm << 16));
            }
         } else {
            i->setSrc(s, bld.loadImm(NULL, imm));
         }
      }
   }

   return true;
}

bool
NVC0LoweringPass::handleManualTXD(TexInstruction *i)
{
   // Always done from the l0 perspective. This is the way that NVIDIA's
   // driver does it, and doing it from the "current" lane's perspective
   // doesn't seem to always work for reasons that aren't altogether clear,
   // even in frag shaders.
   //
   // Note that we must move not only the coordinates into lane0, but also all
   // ancillary arguments, like array indices and depth compare as they may
   // differ between lanes. Offsets for TXD are supposed to be uniform, so we
   // leave them alone.
   static const uint8_t qOps[2] =
      { QUADOP(MOV2, ADD,  MOV2, ADD),  QUADOP(MOV2, MOV2, ADD,  ADD) };

   Value *def[4][4];
   Value *crd[3], *arr[2], *shadow;
   Instruction *tex;
   Value *zero = bld.loadImm(bld.getSSA(), 0);
   int l, c;
   const int dim = i->tex.target.getDim() + i->tex.target.isCube();

   // This function is invoked after handleTEX lowering, so we have to expect
   // the arguments in the order that the hw wants them. For Fermi, array and
   // indirect are both in the leading arg, while for Kepler, array and
   // indirect are separate (and both precede the coordinates). Maxwell is
   // handled in a separate function.
   int array;
   if (targ->getChipset() < NVISA_GK104_CHIPSET)
      array = i->tex.target.isArray() || i->tex.rIndirectSrc >= 0;
   else
      array = i->tex.target.isArray() + (i->tex.rIndirectSrc >= 0);

   i->op = OP_TEX; // no need to clone dPdx/dPdy later

   for (c = 0; c < dim; ++c)
      crd[c] = bld.getScratch();
   for (c = 0; c < array; ++c)
      arr[c] = bld.getScratch();
   shadow = bld.getScratch();

   for (l = 0; l < 4; ++l) {
      Value *src[3], *val;

      bld.mkOp(OP_QUADON, TYPE_NONE, NULL);
      // we're using the texture result from lane 0 in all cases, so make sure
      // that lane 0 is pointing at the proper array index, indirect value,
      // and depth compare.
      if (l != 0) {
         for (c = 0; c < array; ++c)
            bld.mkQuadop(0x00, arr[c], l, i->getSrc(c), zero);
         if (i->tex.target.isShadow()) {
            // The next argument after coords is the depth compare
            bld.mkQuadop(0x00, shadow, l, i->getSrc(array + dim), zero);
         }
      }
      // mov position coordinates from lane l to all lanes
      for (c = 0; c < dim; ++c)
         bld.mkQuadop(0x00, crd[c], l, i->getSrc(c + array), zero);
      // add dPdx from lane l to lanes dx
      for (c = 0; c < dim; ++c)
         bld.mkQuadop(qOps[0], crd[c], l, i->dPdx[c].get(), crd[c]);
      // add dPdy from lane l to lanes dy
      for (c = 0; c < dim; ++c)
         bld.mkQuadop(qOps[1], crd[c], l, i->dPdy[c].get(), crd[c]);
      // normalize cube coordinates
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
      if (l != 0) {
         for (c = 0; c < array; ++c)
            tex->setSrc(c, arr[c]);
         if (i->tex.target.isShadow())
            tex->setSrc(array + dim, shadow);
      }
      for (c = 0; c < dim; ++c)
         tex->setSrc(c + array, src[c]);
      // broadcast results from lane 0 to all lanes so that the moves *into*
      // the target lane pick up the proper value.
      if (l != 0)
         for (c = 0; i->defExists(c); ++c)
            bld.mkQuadop(0x00, tex->getDef(c), 0, tex->getDef(c), zero);
      bld.mkOp(OP_QUADPOP, TYPE_NONE, NULL);

      // save results
      for (c = 0; i->defExists(c); ++c) {
         Instruction *mov;
         def[c][l] = bld.getSSA();
         mov = bld.mkMov(def[c][l], tex->getDef(c));
         mov->fixed = 1;
         mov->lanes = 1 << l;
      }
   }

   for (c = 0; i->defExists(c); ++c) {
      Instruction *u = bld.mkOp(OP_UNION, TYPE_U32, i->getDef(c));
      for (l = 0; l < 4; ++l)
         u->setSrc(l, def[c][l]);
   }

   i->bb->remove(i);
   return true;
}

bool
NVC0LoweringPass::handleTXD(TexInstruction *txd)
{
   int dim = txd->tex.target.getDim() + txd->tex.target.isCube();
   unsigned arg = txd->tex.target.getArgCount();
   unsigned expected_args = arg;
   const int chipset = prog->getTarget()->getChipset();

   if (chipset >= NVISA_GK104_CHIPSET) {
      if (!txd->tex.target.isArray() && txd->tex.useOffsets)
         expected_args++;
      if (txd->tex.rIndirectSrc >= 0 || txd->tex.sIndirectSrc >= 0)
         expected_args++;
   } else {
      if (txd->tex.useOffsets)
         expected_args++;
      if (!txd->tex.target.isArray() && (
                txd->tex.rIndirectSrc >= 0 || txd->tex.sIndirectSrc >= 0))
         expected_args++;
   }

   if (expected_args > 4 ||
       dim > 2 ||
       txd->tex.target.isShadow())
      txd->op = OP_TEX;

   handleTEX(txd);
   while (txd->srcExists(arg))
      ++arg;

   txd->tex.derivAll = true;
   if (txd->op == OP_TEX)
      return handleManualTXD(txd);

   assert(arg == expected_args);
   for (int c = 0; c < dim; ++c) {
      txd->setSrc(arg + c * 2 + 0, txd->dPdx[c]);
      txd->setSrc(arg + c * 2 + 1, txd->dPdy[c]);
      txd->dPdx[c].set(NULL);
      txd->dPdy[c].set(NULL);
   }

   // In this case we have fewer than 4 "real" arguments, which means that
   // handleTEX didn't apply any padding. However we have to make sure that
   // the second "group" of arguments still gets padded up to 4.
   if (chipset >= NVISA_GK104_CHIPSET) {
      int s = arg + 2 * dim;
      if (s >= 4 && s < 7) {
         if (txd->srcExists(s)) // move potential predicate out of the way
            txd->moveSources(s, 7 - s);
         while (s < 7)
            txd->setSrc(s++, bld.loadImm(NULL, 0));
      }
   }

   return true;
}

bool
NVC0LoweringPass::handleTXQ(TexInstruction *txq)
{
   const int chipset = prog->getTarget()->getChipset();
   if (chipset >= NVISA_GK104_CHIPSET && txq->tex.rIndirectSrc < 0)
      txq->tex.r += prog->driver->io.texBindBase / 4;

   if (txq->tex.rIndirectSrc < 0)
      return true;

   Value *ticRel = txq->getIndirectR();

   txq->setIndirectS(NULL);
   txq->tex.sIndirectSrc = -1;

   assert(ticRel);

   if (chipset < NVISA_GK104_CHIPSET) {
      LValue *src = new_LValue(func, FILE_GPR); // 0xttxsaaaa

      txq->setSrc(txq->tex.rIndirectSrc, NULL);
      if (txq->tex.r)
         ticRel = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getScratch(),
                             ticRel, bld.mkImm(txq->tex.r));

      bld.mkOp2(OP_SHL, TYPE_U32, src, ticRel, bld.mkImm(0x17));

      txq->moveSources(0, 1);
      txq->setSrc(0, src);
   } else {
      Value *hnd;
      if (txq->tex.bindless) {
         hnd = txq->getIndirectR();
      } else {
         hnd = loadTexHandle(txq->getIndirectR(), txq->tex.r);
         txq->tex.r = 0xff;
         txq->tex.s = 0x1f;
      }

      txq->setIndirectR(NULL);
      txq->moveSources(0, 1);
      txq->setSrc(0, hnd);
      txq->tex.rIndirectSrc = 0;
   }

   return true;
}

bool
NVC0LoweringPass::handleTXLQ(TexInstruction *i)
{
   /* The outputs are inverted compared to what the TGSI instruction
    * expects. Take that into account in the mask.
    */
   assert((i->tex.mask & ~3) == 0);
   if (i->tex.mask == 1)
      i->tex.mask = 2;
   else if (i->tex.mask == 2)
      i->tex.mask = 1;
   handleTEX(i);
   bld.setPosition(i, true);

   /* The returned values are not quite what we want:
    * (a) convert from s16/u16 to f32
    * (b) multiply by 1/256
    */
   for (int def = 0; def < 2; ++def) {
      if (!i->defExists(def))
         continue;
      enum DataType type = TYPE_S16;
      if (i->tex.mask == 2 || def > 0)
         type = TYPE_U16;
      bld.mkCvt(OP_CVT, TYPE_F32, i->getDef(def), type, i->getDef(def));
      bld.mkOp2(OP_MUL, TYPE_F32, i->getDef(def),
                i->getDef(def), bld.loadImm(NULL, 1.0f / 256));
   }
   if (i->tex.mask == 3) {
      LValue *t = new_LValue(func, FILE_GPR);
      bld.mkMov(t, i->getDef(0));
      bld.mkMov(i->getDef(0), i->getDef(1));
      bld.mkMov(i->getDef(1), t);
   }
   return true;
}

bool
NVC0LoweringPass::handleBUFQ(Instruction *bufq)
{
   bufq->op = OP_MOV;
   bufq->setSrc(0, loadBufLength32(bufq->getIndirect(0, 1),
                                   bufq->getSrc(0)->reg.fileIndex * 16));
   bufq->setIndirect(0, 0, NULL);
   bufq->setIndirect(0, 1, NULL);
   return true;
}

void
NVC0LoweringPass::handleSharedATOMNVE4(Instruction *atom)
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

   CmpInstruction *pred =
      bld.mkCmp(OP_SET, CC_EQ, TYPE_U32, bld.getSSA(1, FILE_PREDICATE),
                TYPE_U32, bld.mkImm(0), bld.mkImm(1));

   bld.mkFlow(OP_BRA, tryLockBB, CC_ALWAYS, NULL);
   currBB->cfg.attach(&tryLockBB->cfg, Graph::Edge::TREE);

   bld.setPosition(tryLockBB, true);

   Instruction *ld =
      bld.mkLoad(TYPE_U32, atom->getDef(0), atom->getSrc(0)->asSym(),
                 atom->getIndirect(0, 0));
   ld->setDef(1, bld.getSSA(1, FILE_PREDICATE));
   ld->subOp = NV50_IR_SUBOP_LOAD_LOCKED;

   bld.mkFlow(OP_BRA, setAndUnlockBB, CC_P, ld->getDef(1));
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
         bld.mkCmp(OP_SET, CC_EQ, TYPE_U32, bld.getSSA(),
                   TYPE_U32, ld->getDef(0), atom->getSrc(1));

      bld.mkCmp(OP_SLCT, CC_NE, TYPE_U32, (stVal = bld.getSSA()),
                TYPE_U32, atom->getSrc(2), ld->getDef(0), set->getDef(0));
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
         return;
      }

      stVal = bld.mkOp2v(op, atom->dType, bld.getSSA(), ld->getDef(0),
                         atom->getSrc(1));
   }

   Instruction *st =
      bld.mkStore(OP_STORE, TYPE_U32, atom->getSrc(0)->asSym(),
                  atom->getIndirect(0, 0), stVal);
   st->setDef(0, pred->getDef(0));
   st->subOp = NV50_IR_SUBOP_STORE_UNLOCKED;

   bld.mkFlow(OP_BRA, failLockBB, CC_ALWAYS, NULL);
   setAndUnlockBB->cfg.attach(&failLockBB->cfg, Graph::Edge::TREE);

   // Lock until the store has not been performed.
   bld.setPosition(failLockBB, true);
   bld.mkFlow(OP_BRA, tryLockBB, CC_NOT_P, pred->getDef(0));
   bld.mkFlow(OP_BRA, joinBB, CC_ALWAYS, NULL);
   failLockBB->cfg.attach(&tryLockBB->cfg, Graph::Edge::BACK);
   failLockBB->cfg.attach(&joinBB->cfg, Graph::Edge::TREE);

   bld.setPosition(joinBB, false);
   bld.mkFlow(OP_JOIN, NULL, CC_ALWAYS, NULL)->fixed = 1;
}

void
NVC0LoweringPass::handleSharedATOM(Instruction *atom)
{
   assert(atom->src(0).getFile() == FILE_MEMORY_SHARED);

   BasicBlock *currBB = atom->bb;
   BasicBlock *tryLockAndSetBB = atom->bb->splitBefore(atom, false);
   BasicBlock *joinBB = atom->bb->splitAfter(atom);

   bld.setPosition(currBB, true);
   assert(!currBB->joinAt);
   currBB->joinAt = bld.mkFlow(OP_JOINAT, joinBB, CC_ALWAYS, NULL);

   bld.mkFlow(OP_BRA, tryLockAndSetBB, CC_ALWAYS, NULL);
   currBB->cfg.attach(&tryLockAndSetBB->cfg, Graph::Edge::TREE);

   bld.setPosition(tryLockAndSetBB, true);

   Instruction *ld =
      bld.mkLoad(TYPE_U32, atom->getDef(0), atom->getSrc(0)->asSym(),
                 atom->getIndirect(0, 0));
   ld->setDef(1, bld.getSSA(1, FILE_PREDICATE));
   ld->subOp = NV50_IR_SUBOP_LOAD_LOCKED;

   Value *stVal;
   if (atom->subOp == NV50_IR_SUBOP_ATOM_EXCH) {
      // Read the old value, and write the new one.
      stVal = atom->getSrc(1);
   } else if (atom->subOp == NV50_IR_SUBOP_ATOM_CAS) {
      CmpInstruction *set =
         bld.mkCmp(OP_SET, CC_EQ, TYPE_U32, bld.getSSA(1, FILE_PREDICATE),
                   TYPE_U32, ld->getDef(0), atom->getSrc(1));
      set->setPredicate(CC_P, ld->getDef(1));

      Instruction *selp =
         bld.mkOp3(OP_SELP, TYPE_U32, bld.getSSA(), ld->getDef(0),
                   atom->getSrc(2), set->getDef(0));
      selp->src(2).mod = Modifier(NV50_IR_MOD_NOT);
      selp->setPredicate(CC_P, ld->getDef(1));

      stVal = selp->getDef(0);
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
         return;
      }

      Instruction *i =
         bld.mkOp2(op, atom->dType, bld.getSSA(), ld->getDef(0),
                   atom->getSrc(1));
      i->setPredicate(CC_P, ld->getDef(1));

      stVal = i->getDef(0);
   }

   Instruction *st =
      bld.mkStore(OP_STORE, TYPE_U32, atom->getSrc(0)->asSym(),
                  atom->getIndirect(0, 0), stVal);
   st->setPredicate(CC_P, ld->getDef(1));
   st->subOp = NV50_IR_SUBOP_STORE_UNLOCKED;

   // Loop until the lock is acquired.
   bld.mkFlow(OP_BRA, tryLockAndSetBB, CC_NOT_P, ld->getDef(1));
   tryLockAndSetBB->cfg.attach(&tryLockAndSetBB->cfg, Graph::Edge::BACK);
   tryLockAndSetBB->cfg.attach(&joinBB->cfg, Graph::Edge::CROSS);
   bld.mkFlow(OP_BRA, joinBB, CC_ALWAYS, NULL);

   bld.remove(atom);

   bld.setPosition(joinBB, false);
   bld.mkFlow(OP_JOIN, NULL, CC_ALWAYS, NULL)->fixed = 1;
}

bool
NVC0LoweringPass::handleATOM(Instruction *atom)
{
   SVSemantic sv;
   Value *ptr = atom->getIndirect(0, 0), *ind = atom->getIndirect(0, 1), *base;

   switch (atom->src(0).getFile()) {
   case FILE_MEMORY_LOCAL:
      sv = SV_LBASE;
      break;
   case FILE_MEMORY_SHARED:
      // For Fermi/Kepler, we have to use ld lock/st unlock to perform atomic
      // operations on shared memory. For Maxwell, ATOMS is enough.
      if (targ->getChipset() < NVISA_GK104_CHIPSET)
         handleSharedATOM(atom);
      else if (targ->getChipset() < NVISA_GM107_CHIPSET)
         handleSharedATOMNVE4(atom);
      return true;
   case FILE_MEMORY_GLOBAL:
      return true;
   default:
      assert(atom->src(0).getFile() == FILE_MEMORY_BUFFER);
      base = loadBufInfo64(ind, atom->getSrc(0)->reg.fileIndex * 16);
      assert(base->reg.size == 8);
      if (ptr)
         base = bld.mkOp2v(OP_ADD, TYPE_U64, base, base, ptr);
      assert(base->reg.size == 8);
      atom->setIndirect(0, 0, base);
      atom->getSrc(0)->reg.file = FILE_MEMORY_GLOBAL;

      // Harden against out-of-bounds accesses
      Value *offset = bld.loadImm(NULL, atom->getSrc(0)->reg.data.offset + typeSizeof(atom->sType));
      Value *length = loadBufLength32(ind, atom->getSrc(0)->reg.fileIndex * 16);
      Value *pred = new_LValue(func, FILE_PREDICATE);
      if (ptr)
         bld.mkOp2(OP_ADD, TYPE_U32, offset, offset, ptr);
      bld.mkCmp(OP_SET, CC_GT, TYPE_U32, pred, TYPE_U32, offset, length);
      atom->setPredicate(CC_NOT_P, pred);
      if (atom->defExists(0)) {
         Value *zero, *dst = atom->getDef(0);
         atom->setDef(0, bld.getSSA());

         bld.setPosition(atom, true);
         bld.mkMov((zero = bld.getSSA()), bld.mkImm(0))
            ->setPredicate(CC_P, pred);
         bld.mkOp2(OP_UNION, TYPE_U32, dst, atom->getDef(0), zero);
      }

      return true;
   }
   base =
      bld.mkOp1v(OP_RDSV, TYPE_U32, bld.getScratch(), bld.mkSysVal(sv, 0));

   atom->setSrc(0, cloneShallow(func, atom->getSrc(0)));
   atom->getSrc(0)->reg.file = FILE_MEMORY_GLOBAL;
   if (ptr)
      base = bld.mkOp2v(OP_ADD, TYPE_U32, base, base, ptr);
   atom->setIndirect(0, 1, NULL);
   atom->setIndirect(0, 0, base);

   return true;
}

bool
NVC0LoweringPass::handleATOMCctl(Instruction *atom) {
   // Flush L1 cache manually since atomics go directly to L2. This ensures
   // that any later CA reads retrieve the updated data.

   if (atom->cache != nv50_ir::CACHE_CA)
      return false;

   bld.setPosition(atom, true);

   Instruction *cctl = bld.mkOp1(OP_CCTL, TYPE_NONE, NULL, atom->getSrc(0));
   cctl->setIndirect(0, 0, atom->getIndirect(0, 0));
   cctl->fixed = 1;
   cctl->subOp = NV50_IR_SUBOP_CCTL_IV;
   if (atom->isPredicated())
      cctl->setPredicate(atom->cc, atom->getPredicate());

   return true;
}

bool
NVC0LoweringPass::handleCasExch(Instruction *cas)
{
   if (targ->getChipset() < NVISA_GM107_CHIPSET) {
      if (cas->src(0).getFile() == FILE_MEMORY_SHARED) {
         // ATOM_CAS and ATOM_EXCH are handled in handleSharedATOM().
         return false;
      }
   }

   if (cas->subOp != NV50_IR_SUBOP_ATOM_CAS &&
       cas->subOp != NV50_IR_SUBOP_ATOM_EXCH)
      return false;

   if (cas->subOp == NV50_IR_SUBOP_ATOM_CAS &&
       targ->getChipset() < NVISA_GV100_CHIPSET) {
      // CAS is crazy. It's 2nd source is a double reg, and the 3rd source
      // should be set to the high part of the double reg or bad things will
      // happen elsewhere in the universe.
      // Also, it sometimes returns the new value instead of the old one
      // under mysterious circumstances.
      DataType ty = typeOfSize(typeSizeof(cas->dType) * 2);
      Value *dreg = bld.getSSA(typeSizeof(ty));
      bld.setPosition(cas, false);
      bld.mkOp2(OP_MERGE, ty, dreg, cas->getSrc(1), cas->getSrc(2));
      cas->setSrc(1, dreg);
      cas->setSrc(2, dreg);
   }

   return true;
}

inline Value *
NVC0LoweringPass::loadResInfo32(Value *ptr, uint32_t off, uint16_t base)
{
   uint8_t b = prog->driver->io.auxCBSlot;
   off += base;

   return bld.
      mkLoadv(TYPE_U32, bld.mkSymbol(FILE_MEMORY_CONST, b, TYPE_U32, off), ptr);
}

inline Value *
NVC0LoweringPass::loadResInfo64(Value *ptr, uint32_t off, uint16_t base)
{
   uint8_t b = prog->driver->io.auxCBSlot;
   off += base;

   if (ptr)
      ptr = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getScratch(), ptr, bld.mkImm(4));

   return bld.
      mkLoadv(TYPE_U64, bld.mkSymbol(FILE_MEMORY_CONST, b, TYPE_U64, off), ptr);
}

inline Value *
NVC0LoweringPass::loadResLength32(Value *ptr, uint32_t off, uint16_t base)
{
   uint8_t b = prog->driver->io.auxCBSlot;
   off += base;

   if (ptr)
      ptr = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getScratch(), ptr, bld.mkImm(4));

   return bld.
      mkLoadv(TYPE_U32, bld.mkSymbol(FILE_MEMORY_CONST, b, TYPE_U64, off + 8), ptr);
}

inline Value *
NVC0LoweringPass::loadBufInfo64(Value *ptr, uint32_t off)
{
   return loadResInfo64(ptr, off, prog->driver->io.bufInfoBase);
}

inline Value *
NVC0LoweringPass::loadBufLength32(Value *ptr, uint32_t off)
{
   return loadResLength32(ptr, off, prog->driver->io.bufInfoBase);
}

inline Value *
NVC0LoweringPass::loadUboInfo64(Value *ptr, uint32_t off)
{
   return loadResInfo64(ptr, off, prog->driver->io.uboInfoBase);
}

inline Value *
NVC0LoweringPass::loadUboLength32(Value *ptr, uint32_t off)
{
   return loadResLength32(ptr, off, prog->driver->io.uboInfoBase);
}

inline Value *
NVC0LoweringPass::loadMsInfo32(Value *ptr, uint32_t off)
{
   uint8_t b = prog->driver->io.msInfoCBSlot;
   off += prog->driver->io.msInfoBase;
   return bld.
      mkLoadv(TYPE_U32, bld.mkSymbol(FILE_MEMORY_CONST, b, TYPE_U32, off), ptr);
}

inline Value *
NVC0LoweringPass::loadSuInfo32(Value *ptr, int slot, uint32_t off, bool bindless)
{
   uint32_t base = slot * NVC0_SU_INFO__STRIDE;

   // We don't upload surface info for bindless for GM107+
   assert(!bindless || targ->getChipset() < NVISA_GM107_CHIPSET);

   if (ptr) {
      ptr = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(), ptr, bld.mkImm(slot));
      if (bindless)
         ptr = bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(), ptr, bld.mkImm(511));
      else
         ptr = bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(), ptr, bld.mkImm(7));
      ptr = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(), ptr, bld.mkImm(6));
      base = 0;
   }
   off += base;

   return loadResInfo32(ptr, off, bindless ? prog->driver->io.bindlessBase :
                        prog->driver->io.suInfoBase);
}

Value *
NVC0LoweringPass::loadMsAdjInfo32(TexInstruction::Target target, uint32_t index, int slot, Value *ind, bool bindless)
{
   if (!bindless || targ->getChipset() < NVISA_GM107_CHIPSET)
      return loadSuInfo32(ind, slot, NVC0_SU_INFO_MS(index), bindless);

   assert(bindless);

   Value *samples = bld.getSSA();
   // this shouldn't be lowered because it's being inserted before the current instruction
   TexInstruction *tex = new_TexInstruction(func, OP_TXQ);
   tex->tex.target = target;
   tex->tex.query = TXQ_TYPE;
   tex->tex.mask = 0x4;
   tex->tex.r = 0xff;
   tex->tex.s = 0x1f;
   tex->tex.rIndirectSrc = 0;
   tex->setDef(0, samples);
   tex->setSrc(0, ind);
   tex->setSrc(1, bld.loadImm(NULL, 0));
   bld.insert(tex);

   // doesn't work with sample counts other than 1/2/4/8 but they aren't supported
   switch (index) {
   case 0: {
      Value *tmp = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(), samples, bld.mkImm(2));
      return bld.mkOp2v(OP_SHR, TYPE_U32, bld.getSSA(), tmp, bld.mkImm(2));
   }
   case 1: {
      Value *tmp = bld.mkCmp(OP_SET, CC_GT, TYPE_U32, bld.getSSA(), TYPE_U32, samples, bld.mkImm(2))->getDef(0);
      return bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(), tmp, bld.mkImm(1));
   }
   default: {
      assert(false);
      return NULL;
   }
   }
}

static inline uint16_t getSuClampSubOp(const TexInstruction *su, int c)
{
   switch (su->tex.target.getEnum()) {
   case TEX_TARGET_BUFFER:      return NV50_IR_SUBOP_SUCLAMP_PL(0, 1);
   case TEX_TARGET_RECT:        return NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   case TEX_TARGET_1D:          return NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   case TEX_TARGET_1D_ARRAY:    return (c == 1) ?
                                   NV50_IR_SUBOP_SUCLAMP_PL(0, 2) :
                                   NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   case TEX_TARGET_2D:          return NV50_IR_SUBOP_SUCLAMP_BL(0, 2);
   case TEX_TARGET_2D_MS:       return NV50_IR_SUBOP_SUCLAMP_BL(0, 2);
   case TEX_TARGET_2D_ARRAY:    return NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   case TEX_TARGET_2D_MS_ARRAY: return NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   case TEX_TARGET_3D:          return NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   case TEX_TARGET_CUBE:        return NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   case TEX_TARGET_CUBE_ARRAY:  return NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   default:
      assert(0);
      return 0;
   }
}

bool
NVC0LoweringPass::handleSUQ(TexInstruction *suq)
{
   int mask = suq->tex.mask;
   int dim = suq->tex.target.getDim();
   int arg = dim + (suq->tex.target.isArray() || suq->tex.target.isCube());
   Value *ind = suq->getIndirectR();
   int slot = suq->tex.r;
   int c, d;

   for (c = 0, d = 0; c < 3; ++c, mask >>= 1) {
      if (c >= arg || !(mask & 1))
         continue;

      int offset;

      if (c == 1 && suq->tex.target == TEX_TARGET_1D_ARRAY) {
         offset = NVC0_SU_INFO_SIZE(2);
      } else {
         offset = NVC0_SU_INFO_SIZE(c);
      }
      bld.mkMov(suq->getDef(d++), loadSuInfo32(ind, slot, offset, suq->tex.bindless));
      if (c == 2 && suq->tex.target.isCube())
         bld.mkOp2(OP_DIV, TYPE_U32, suq->getDef(d - 1), suq->getDef(d - 1),
                   bld.loadImm(NULL, 6));
   }

   if (mask & 1) {
      if (suq->tex.target.isMS()) {
         Value *ms_x = loadSuInfo32(ind, slot, NVC0_SU_INFO_MS(0), suq->tex.bindless);
         Value *ms_y = loadSuInfo32(ind, slot, NVC0_SU_INFO_MS(1), suq->tex.bindless);
         Value *ms = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getScratch(), ms_x, ms_y);
         bld.mkOp2(OP_SHL, TYPE_U32, suq->getDef(d++), bld.loadImm(NULL, 1), ms);
      } else {
         bld.mkMov(suq->getDef(d++), bld.loadImm(NULL, 1));
      }
   }

   bld.remove(suq);
   return true;
}

void
NVC0LoweringPass::adjustCoordinatesMS(TexInstruction *tex)
{
   const int arg = tex->tex.target.getArgCount();
   int slot = tex->tex.r;

   if (tex->tex.target == TEX_TARGET_2D_MS)
      tex->tex.target = TEX_TARGET_2D;
   else
   if (tex->tex.target == TEX_TARGET_2D_MS_ARRAY)
      tex->tex.target = TEX_TARGET_2D_ARRAY;
   else
      return;

   Value *x = tex->getSrc(0);
   Value *y = tex->getSrc(1);
   Value *s = tex->getSrc(arg - 1);

   Value *tx = bld.getSSA(), *ty = bld.getSSA(), *ts = bld.getSSA();
   Value *ind = tex->getIndirectR();

   Value *ms_x = loadMsAdjInfo32(tex->tex.target, 0, slot, ind, tex->tex.bindless);
   Value *ms_y = loadMsAdjInfo32(tex->tex.target, 1, slot, ind, tex->tex.bindless);

   bld.mkOp2(OP_SHL, TYPE_U32, tx, x, ms_x);
   bld.mkOp2(OP_SHL, TYPE_U32, ty, y, ms_y);

   s = bld.mkOp2v(OP_AND, TYPE_U32, ts, s, bld.loadImm(NULL, 0x7));
   s = bld.mkOp2v(OP_SHL, TYPE_U32, ts, ts, bld.mkImm(3));

   Value *dx = loadMsInfo32(ts, 0x0);
   Value *dy = loadMsInfo32(ts, 0x4);

   bld.mkOp2(OP_ADD, TYPE_U32, tx, tx, dx);
   bld.mkOp2(OP_ADD, TYPE_U32, ty, ty, dy);

   tex->setSrc(0, tx);
   tex->setSrc(1, ty);
   tex->moveSources(arg, -1);
}

// Sets 64-bit "generic address", predicate and format sources for SULD/SUST.
// They're computed from the coordinates using the surface info in c[] space.
void
NVC0LoweringPass::processSurfaceCoordsNVE4(TexInstruction *su)
{
   Instruction *insn;
   const bool atom = su->op == OP_SUREDB || su->op == OP_SUREDP;
   const bool raw =
      su->op == OP_SULDB || su->op == OP_SUSTB || su->op == OP_SUREDB;
   const int slot = su->tex.r;
   const int dim = su->tex.target.getDim();
   const bool array = su->tex.target.isArray() || su->tex.target.isCube();
   const int arg = dim + array;
   int c;
   Value *zero = bld.mkImm(0);
   Value *p1 = NULL;
   Value *v;
   Value *src[3];
   Value *bf, *eau, *off;
   Value *addr, *pred;
   Value *ind = su->getIndirectR();
   Value *y, *z;

   off = bld.getScratch(4);
   bf = bld.getScratch(4);
   addr = bld.getSSA(8);
   pred = bld.getScratch(1, FILE_PREDICATE);

   bld.setPosition(su, false);

   adjustCoordinatesMS(su);

   // calculate clamped coordinates
   for (c = 0; c < arg; ++c) {
      int dimc = c;

      if (c == 1 && su->tex.target == TEX_TARGET_1D_ARRAY) {
         // The array index is stored in the Z component for 1D arrays.
         dimc = 2;
      }

      src[c] = bld.getScratch();
      if (c == 0 && raw)
         v = loadSuInfo32(ind, slot, NVC0_SU_INFO_RAW_X, su->tex.bindless);
      else
         v = loadSuInfo32(ind, slot, NVC0_SU_INFO_DIM(dimc), su->tex.bindless);
      bld.mkOp3(OP_SUCLAMP, TYPE_S32, src[c], su->getSrc(c), v, zero)
         ->subOp = getSuClampSubOp(su, dimc);
   }
   for (; c < 3; ++c)
      src[c] = zero;

   if (dim == 2 && !array) {
      v = loadSuInfo32(ind, slot, NVC0_SU_INFO_UNK1C, su->tex.bindless);
      src[2] = bld.mkOp2v(OP_SHR, TYPE_U32, bld.getSSA(),
                          v, bld.loadImm(NULL, 16));

      v = loadSuInfo32(ind, slot, NVC0_SU_INFO_DIM(2), su->tex.bindless);
      bld.mkOp3(OP_SUCLAMP, TYPE_S32, src[2], src[2], v, zero)
         ->subOp = NV50_IR_SUBOP_SUCLAMP_SD(0, 2);
   }

   // set predicate output
   if (su->tex.target == TEX_TARGET_BUFFER) {
      src[0]->getInsn()->setFlagsDef(1, pred);
   } else
   if (array) {
      p1 = bld.getSSA(1, FILE_PREDICATE);
      src[dim]->getInsn()->setFlagsDef(1, p1);
   }

   // calculate pixel offset
   if (dim == 1) {
      y = z = zero;
      if (su->tex.target != TEX_TARGET_BUFFER)
         bld.mkOp2(OP_AND, TYPE_U32, off, src[0], bld.loadImm(NULL, 0xffff));
   } else {
      y = src[1];
      z = src[2];

      v = loadSuInfo32(ind, slot, NVC0_SU_INFO_UNK1C, su->tex.bindless);
      bld.mkOp3(OP_MADSP, TYPE_U32, off, src[2], v, src[1])
         ->subOp = NV50_IR_SUBOP_MADSP(4,4,8); // u16l u16l u16l

      v = loadSuInfo32(ind, slot, NVC0_SU_INFO_PITCH, su->tex.bindless);
      bld.mkOp3(OP_MADSP, TYPE_U32, off, off, v, src[0])
         ->subOp = array ?
         NV50_IR_SUBOP_MADSP_SD : NV50_IR_SUBOP_MADSP(0,2,8); // u32 u16l u16l
   }

   // calculate effective address part 1
   if (su->tex.target == TEX_TARGET_BUFFER) {
      if (raw) {
         bf = src[0];
      } else {
         v = loadSuInfo32(ind, slot, NVC0_SU_INFO_FMT, su->tex.bindless);
         bld.mkOp3(OP_VSHL, TYPE_U32, bf, src[0], v, zero)
            ->subOp = NV50_IR_SUBOP_V1(7,6,8|2);
      }
   } else {
      uint16_t subOp = 0;

      switch (dim) {
      case 1:
         break;
      case 2:
         if (array) {
            z = off;
         } else {
            subOp = NV50_IR_SUBOP_SUBFM_3D;
         }
         break;
      default:
         subOp = NV50_IR_SUBOP_SUBFM_3D;
         assert(dim == 3);
         break;
      }
      insn = bld.mkOp3(OP_SUBFM, TYPE_U32, bf, src[0], y, z);
      insn->subOp = subOp;
      insn->setFlagsDef(1, pred);
   }

   // part 2
   v = loadSuInfo32(ind, slot, NVC0_SU_INFO_ADDR, su->tex.bindless);

   if (su->tex.target == TEX_TARGET_BUFFER) {
      eau = v;
   } else {
      eau = bld.mkOp3v(OP_SUEAU, TYPE_U32, bld.getScratch(4), off, bf, v);
   }
   // add array layer offset
   if (array) {
      v = loadSuInfo32(ind, slot, NVC0_SU_INFO_ARRAY, su->tex.bindless);
      if (dim == 1)
         bld.mkOp3(OP_MADSP, TYPE_U32, eau, src[1], v, eau)
            ->subOp = NV50_IR_SUBOP_MADSP(4,0,0); // u16 u24 u32
      else
         bld.mkOp3(OP_MADSP, TYPE_U32, eau, v, src[2], eau)
            ->subOp = NV50_IR_SUBOP_MADSP(0,0,0); // u32 u24 u32
      // combine predicates
      assert(p1);
      bld.mkOp2(OP_OR, TYPE_U8, pred, pred, p1);
   }

   if (atom) {
      Value *lo = bf;
      if (su->tex.target == TEX_TARGET_BUFFER) {
         lo = zero;
         bld.mkMov(off, bf);
      }
      //  bf == g[] address & 0xff
      // eau == g[] address >> 8
      bld.mkOp3(OP_PERMT, TYPE_U32,  bf,   lo, bld.loadImm(NULL, 0x6540), eau);
      bld.mkOp3(OP_PERMT, TYPE_U32, eau, zero, bld.loadImm(NULL, 0x0007), eau);
   } else
   if (su->op == OP_SULDP && su->tex.target == TEX_TARGET_BUFFER) {
      // Convert from u32 to u8 address format, which is what the library code
      // doing SULDP currently uses.
      // XXX: can SUEAU do this ?
      // XXX: does it matter that we don't mask high bytes in bf ?
      // Grrr.
      bld.mkOp2(OP_SHR, TYPE_U32, off, bf, bld.mkImm(8));
      bld.mkOp2(OP_ADD, TYPE_U32, eau, eau, off);
   }

   bld.mkOp2(OP_MERGE, TYPE_U64, addr, bf, eau);

   if (atom && su->tex.target == TEX_TARGET_BUFFER)
      bld.mkOp2(OP_ADD, TYPE_U64, addr, addr, off);

   // let's just set it 0 for raw access and hope it works
   v = raw ?
      bld.mkImm(0) : loadSuInfo32(ind, slot, NVC0_SU_INFO_FMT, su->tex.bindless);

   // get rid of old coordinate sources, make space for fmt info and predicate
   su->moveSources(arg, 3 - arg);
   // set 64 bit address and 32-bit format sources
   su->setSrc(0, addr);
   su->setSrc(1, v);
   su->setSrc(2, pred);
   su->setIndirectR(NULL);

   // prevent read fault when the image is not actually bound
   CmpInstruction *pred1 =
      bld.mkCmp(OP_SET, CC_EQ, TYPE_U32, bld.getSSA(1, FILE_PREDICATE),
                TYPE_U32, bld.mkImm(0),
                loadSuInfo32(ind, slot, NVC0_SU_INFO_ADDR, su->tex.bindless));

   if (su->op != OP_SUSTP && su->tex.format) {
      const TexInstruction::ImgFormatDesc *format = su->tex.format;
      int blockwidth = format->bits[0] + format->bits[1] +
                       format->bits[2] + format->bits[3];

      // make sure that the format doesn't mismatch
      assert(format->components != 0);
      bld.mkCmp(OP_SET_OR, CC_NE, TYPE_U32, pred1->getDef(0),
                TYPE_U32, bld.loadImm(NULL, blockwidth / 8),
                loadSuInfo32(ind, slot, NVC0_SU_INFO_BSIZE, su->tex.bindless),
                pred1->getDef(0));
   }
   su->setPredicate(CC_NOT_P, pred1->getDef(0));

   // TODO: initialize def values to 0 when the surface operation is not
   // performed (not needed for stores). Also, fix the "address bounds test"
   // subtests from arb_shader_image_load_store-invalid for buffers, because it
   // seems like that the predicate is not correctly set by suclamp.
}

static DataType
getSrcType(const TexInstruction::ImgFormatDesc *t, int c)
{
   switch (t->type) {
   case FLOAT: return t->bits[c] == 16 ? TYPE_F16 : TYPE_F32;
   case UNORM: return t->bits[c] == 8 ? TYPE_U8 : TYPE_U16;
   case SNORM: return t->bits[c] == 8 ? TYPE_S8 : TYPE_S16;
   case UINT:
      return (t->bits[c] == 8 ? TYPE_U8 :
              (t->bits[c] == 16 ? TYPE_U16 : TYPE_U32));
   case SINT:
      return (t->bits[c] == 8 ? TYPE_S8 :
              (t->bits[c] == 16 ? TYPE_S16 : TYPE_S32));
   }
   return TYPE_NONE;
}

static DataType
getDestType(const ImgType type) {
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

void
NVC0LoweringPass::convertSurfaceFormat(TexInstruction *su, Instruction **loaded)
{
   const TexInstruction::ImgFormatDesc *format = su->tex.format;
   int width = format->bits[0] + format->bits[1] +
      format->bits[2] + format->bits[3];
   Value *untypedDst[4] = {};
   Value *typedDst[4] = {};

   // We must convert this to a generic load.
   su->op = OP_SULDB;

   su->dType = typeOfSize(width / 8);
   su->sType = TYPE_U8;

   for (int i = 0; i < width / 32; i++)
      untypedDst[i] = bld.getSSA();
   if (width < 32)
      untypedDst[0] = bld.getSSA();

   if (loaded && loaded[0]) {
      for (int i = 0; i < 4; i++) {
         if (loaded[i])
            typedDst[i] = loaded[i]->getDef(0);
      }
   } else {
      for (int i = 0; i < 4; i++) {
         typedDst[i] = su->getDef(i);
      }
   }

   // Set the untyped dsts as the su's destinations
   if (loaded && loaded[0]) {
      for (int i = 0; i < 4; i++)
         if (loaded[i])
            loaded[i]->setDef(0, untypedDst[i]);
   } else {
      for (int i = 0; i < 4; i++)
         su->setDef(i, untypedDst[i]);

      bld.setPosition(su, true);
   }

   // Unpack each component into the typed dsts
   int bits = 0;
   for (int i = 0; i < 4; bits += format->bits[i], i++) {
      if (!typedDst[i])
         continue;

      if (loaded && loaded[0])
         bld.setPosition(loaded[i], true);

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
      else if (format->bits[i] == 16)
         bld.mkCvt(OP_CVT, getDestType(format->type), typedDst[i],
                   getSrcType(format, i), untypedDst[i / 2])
         ->subOp = (i & 1) << (format->type == FLOAT ? 0 : 1);
      else if (format->bits[i] == 8)
         bld.mkCvt(OP_CVT, getDestType(format->type), typedDst[i],
                   getSrcType(format, i), untypedDst[0])->subOp = i;
      else {
         bld.mkOp2(OP_EXTBF, TYPE_U32, typedDst[i], untypedDst[bits / 32],
                   bld.mkImm((bits % 32) | (format->bits[i] << 8)));
         if (format->type == UNORM || format->type == SNORM)
            bld.mkCvt(OP_CVT, TYPE_F32, typedDst[i], getSrcType(format, i), typedDst[i]);
      }

      // Normalize / convert as necessary
      if (format->type == UNORM)
         bld.mkOp2(OP_MUL, TYPE_F32, typedDst[i], typedDst[i], bld.loadImm(NULL, 1.0f / ((1 << format->bits[i]) - 1)));
      else if (format->type == SNORM)
         bld.mkOp2(OP_MUL, TYPE_F32, typedDst[i], typedDst[i], bld.loadImm(NULL, 1.0f / ((1 << (format->bits[i] - 1)) - 1)));
      else if (format->type == FLOAT && format->bits[i] < 16) {
         bld.mkOp2(OP_SHL, TYPE_U32, typedDst[i], typedDst[i], bld.loadImm(NULL, 15 - format->bits[i]));
         bld.mkCvt(OP_CVT, TYPE_F32, typedDst[i], TYPE_F16, typedDst[i]);
      }
   }

   if (format->bgra) {
      std::swap(typedDst[0], typedDst[2]);
   }
}

void
NVC0LoweringPass::insertOOBSurfaceOpResult(TexInstruction *su)
{
   if (!su->getPredicate())
      return;

   bld.setPosition(su, true);

   for (unsigned i = 0; su->defExists(i); ++i) {
      Value *def = su->getDef(i);
      Value *newDef = bld.getSSA();
      su->setDef(i, newDef);

      Instruction *mov = bld.mkMov(bld.getSSA(), bld.loadImm(NULL, 0));
      assert(su->cc == CC_NOT_P);
      mov->setPredicate(CC_P, su->getPredicate());
      Instruction *uni = bld.mkOp2(OP_UNION, TYPE_U32, bld.getSSA(), newDef, mov->getDef(0));
      bld.mkMov(def, uni->getDef(0));
   }
}

void
NVC0LoweringPass::handleSurfaceOpNVE4(TexInstruction *su)
{
   processSurfaceCoordsNVE4(su);

   if (su->op == OP_SULDP && su->tex.format) {
      convertSurfaceFormat(su, NULL);
      insertOOBSurfaceOpResult(su);
   }

   if (su->op == OP_SUREDB || su->op == OP_SUREDP) {
      assert(su->getPredicate());
      Value *pred =
         bld.mkOp2v(OP_OR, TYPE_U8, bld.getScratch(1, FILE_PREDICATE),
                    su->getPredicate(), su->getSrc(2));

      Instruction *red = bld.mkOp(OP_ATOM, su->dType, bld.getSSA());
      red->subOp = su->subOp;
      red->setSrc(0, bld.mkSymbol(FILE_MEMORY_GLOBAL, 0, TYPE_U32, 0));
      red->setSrc(1, su->getSrc(3));
      if (su->subOp == NV50_IR_SUBOP_ATOM_CAS)
         red->setSrc(2, su->getSrc(4));
      red->setIndirect(0, 0, su->getSrc(0));

      // make sure to initialize dst value when the atomic operation is not
      // performed
      Instruction *mov = bld.mkMov(bld.getSSA(), bld.loadImm(NULL, 0));

      assert(su->cc == CC_NOT_P);
      red->setPredicate(su->cc, pred);
      mov->setPredicate(CC_P, pred);

      bld.mkOp2(OP_UNION, TYPE_U32, su->getDef(0),
                red->getDef(0), mov->getDef(0));

      delete_Instruction(bld.getProgram(), su);

      handleATOMCctl(red);
      handleCasExch(red);
   }

   if (su->op == OP_SUSTB || su->op == OP_SUSTP)
      su->sType = (su->tex.target == TEX_TARGET_BUFFER) ? TYPE_U32 : TYPE_U8;
}

void
NVC0LoweringPass::processSurfaceCoordsNVC0(TexInstruction *su)
{
   const int slot = su->tex.r;
   const int dim = su->tex.target.getDim();
   const int arg = dim + (su->tex.target.isArray() || su->tex.target.isCube());
   int c;
   Value *zero = bld.mkImm(0);
   Value *src[3];
   Value *v;
   Value *ind = su->getIndirectR();

   bld.setPosition(su, false);

   adjustCoordinatesMS(su);

   if (ind) {
      Value *ptr;
      ptr = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(), ind, bld.mkImm(su->tex.r));
      ptr = bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(), ptr, bld.mkImm(7));
      su->setIndirectR(ptr);
   }

   // get surface coordinates
   for (c = 0; c < arg; ++c)
      src[c] = su->getSrc(c);
   for (; c < 3; ++c)
      src[c] = zero;

   // calculate pixel offset
   if (su->op == OP_SULDP || su->op == OP_SUREDP) {
      v = loadSuInfo32(ind, slot, NVC0_SU_INFO_BSIZE, su->tex.bindless);
      su->setSrc(0, (src[0] = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(), src[0], v)));
   }

   // add array layer offset
   if (su->tex.target.isArray() || su->tex.target.isCube()) {
      v = loadSuInfo32(ind, slot, NVC0_SU_INFO_ARRAY, su->tex.bindless);
      assert(dim > 1);
      su->setSrc(2, (src[2] = bld.mkOp2v(OP_MUL, TYPE_U32, bld.getSSA(), src[2], v)));
   }

   // 3d is special-cased. Note that a single "slice" of a 3d image may
   // also be attached as 2d, so we have to do the same 3d processing for
   // 2d as well, just in case. In order to remap a 3d image onto a 2d
   // image, we have to retile it "by hand".
   if (su->tex.target == TEX_TARGET_3D || su->tex.target == TEX_TARGET_2D) {
      Value *z = loadSuInfo32(ind, slot, NVC0_SU_INFO_UNK1C, su->tex.bindless);
      Value *y_size_aligned =
         bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(),
                    loadSuInfo32(ind, slot, NVC0_SU_INFO_DIM_Y, su->tex.bindless),
                    bld.loadImm(NULL, 0x0000ffff));
      // Add the z coordinate for actual 3d-images
      if (dim > 2)
         src[2] = bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(), z, src[2]);
      else
         src[2] = z;

      // Compute the surface parameters from tile shifts
      Value *tile_shift[3];
      Value *tile_extbf[3];
      // Fetch the "real" tiling parameters of the underlying surface
      for (int i = 0; i < 3; i++) {
         tile_extbf[i] =
            bld.mkOp2v(OP_SHR, TYPE_U32, bld.getSSA(),
                       loadSuInfo32(ind, slot, NVC0_SU_INFO_DIM(i), su->tex.bindless),
                       bld.loadImm(NULL, 16));
         tile_shift[i] =
            bld.mkOp2v(OP_SHR, TYPE_U32, bld.getSSA(),
                       loadSuInfo32(ind, slot, NVC0_SU_INFO_DIM(i), su->tex.bindless),
                       bld.loadImm(NULL, 24));
      }

      // However for load/atomics, we use byte-indexing. And for byte
      // indexing, the X tile size is always the same. This leads to slightly
      // better code.
      if (su->op == OP_SULDP || su->op == OP_SUREDP) {
         tile_extbf[0] = bld.loadImm(NULL, 0x600);
         tile_shift[0] = bld.loadImm(NULL, 6);
      }

      // Compute the location of given coordinate, both inside the tile as
      // well as which (linearly-laid out) tile it's in.
      Value *coord_in_tile[3];
      Value *tile[3];
      for (int i = 0; i < 3; i++) {
         coord_in_tile[i] = bld.mkOp2v(OP_EXTBF, TYPE_U32, bld.getSSA(), src[i], tile_extbf[i]);
         tile[i] = bld.mkOp2v(OP_SHR, TYPE_U32, bld.getSSA(), src[i], tile_shift[i]);
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

      su->setSrc(0, bld.mkOp2v(
            OP_ADD, TYPE_U32, bld.getSSA(),
            bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(),
                       coord_in_tile[0],
                       bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                                  tile[0],
                                  bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(),
                                             tile_shift[2], tile_shift[0]))),
            bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                       coord_in_tile[2], tile_shift[0])));

      su->setSrc(1, bld.mkOp2v(
            OP_ADD, TYPE_U32, bld.getSSA(),
            bld.mkOp2v(OP_MUL, TYPE_U32, bld.getSSA(),
                       tile[2], y_size_aligned),
            bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(),
                       coord_in_tile[1],
                       bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                                  tile[1], tile_shift[1]))));

      if (su->tex.target == TEX_TARGET_3D) {
         su->moveSources(3, -1);
         su->tex.target = TEX_TARGET_2D;
      }
   }

   // prevent read fault when the image is not actually bound
   CmpInstruction *pred =
      bld.mkCmp(OP_SET, CC_EQ, TYPE_U32, bld.getSSA(1, FILE_PREDICATE),
                TYPE_U32, bld.mkImm(0),
                loadSuInfo32(ind, slot, NVC0_SU_INFO_ADDR, su->tex.bindless));
   if (su->op != OP_SUSTP && su->tex.format) {
      const TexInstruction::ImgFormatDesc *format = su->tex.format;
      int blockwidth = format->bits[0] + format->bits[1] +
                       format->bits[2] + format->bits[3];

      assert(format->components != 0);
      // make sure that the format doesn't mismatch when it's not FMT_NONE
      bld.mkCmp(OP_SET_OR, CC_NE, TYPE_U32, pred->getDef(0),
                TYPE_U32, bld.loadImm(NULL, ffs(blockwidth / 8) - 1),
                loadSuInfo32(ind, slot, NVC0_SU_INFO_BSIZE, su->tex.bindless),
                pred->getDef(0));
   }
   su->setPredicate(CC_NOT_P, pred->getDef(0));
}

void
NVC0LoweringPass::handleSurfaceOpNVC0(TexInstruction *su)
{
   if (su->tex.target == TEX_TARGET_1D_ARRAY) {
      /* As 1d arrays also need 3 coordinates, switching to TEX_TARGET_2D_ARRAY
       * will simplify the lowering pass and the texture constraints. */
      su->moveSources(1, 1);
      su->setSrc(1, bld.loadImm(NULL, 0));
      su->tex.target = TEX_TARGET_2D_ARRAY;
   }

   processSurfaceCoordsNVC0(su);

   if (su->op == OP_SULDP && su->tex.format) {
      convertSurfaceFormat(su, NULL);
      insertOOBSurfaceOpResult(su);
   }

   if (su->op == OP_SUREDB || su->op == OP_SUREDP) {
      const int dim = su->tex.target.getDim();
      const int arg = dim + (su->tex.target.isArray() || su->tex.target.isCube());
      LValue *addr = bld.getSSA(8);
      Value *def = su->getDef(0);

      su->op = OP_SULEA;

      // Set the destination to the address
      su->dType = TYPE_U64;
      su->setDef(0, addr);
      su->setDef(1, su->getPredicate());

      bld.setPosition(su, true);

      // Perform the atomic op
      Instruction *red = bld.mkOp(OP_ATOM, su->sType, bld.getSSA());
      red->subOp = su->subOp;
      red->setSrc(0, bld.mkSymbol(FILE_MEMORY_GLOBAL, 0, su->sType, 0));
      red->setSrc(1, su->getSrc(arg));
      if (red->subOp == NV50_IR_SUBOP_ATOM_CAS)
         red->setSrc(2, su->getSrc(arg + 1));
      red->setIndirect(0, 0, addr);

      // make sure to initialize dst value when the atomic operation is not
      // performed
      Instruction *mov = bld.mkMov(bld.getSSA(), bld.loadImm(NULL, 0));

      assert(su->cc == CC_NOT_P);
      red->setPredicate(su->cc, su->getPredicate());
      mov->setPredicate(CC_P, su->getPredicate());

      bld.mkOp2(OP_UNION, TYPE_U32, def, red->getDef(0), mov->getDef(0));

      handleCasExch(red);
   }
}

TexInstruction *
NVC0LoweringPass::processSurfaceCoordsGM107(TexInstruction *su, Instruction *ret[4])
{
   const int slot = su->tex.r;
   const int dim = su->tex.target.getDim();
   const bool array = su->tex.target.isArray() || su->tex.target.isCube();
   const int arg = dim + array;
   Value *ind = su->getIndirectR();
   Value *handle;
   Instruction *pred = NULL, *pred2d = NULL;
   int pos = 0;

   bld.setPosition(su, false);

   adjustCoordinatesMS(su);

   // add texture handle
   switch (su->op) {
   case OP_SUSTP:
      pos = 4;
      break;
   case OP_SUREDP:
      pos = (su->subOp == NV50_IR_SUBOP_ATOM_CAS) ? 2 : 1;
      break;
   default:
      assert(pos == 0);
      break;
   }

   if (dim == 2 && !array) {
      // This might be a 2d slice of a 3d texture, try to load the z
      // coordinate in.
      Value *v;
      if (!su->tex.bindless)
         v = loadSuInfo32(ind, slot, NVC0_SU_INFO_UNK1C, su->tex.bindless);
      else
         v = bld.mkOp2v(OP_SHR, TYPE_U32, bld.getSSA(), ind, bld.mkImm(11));
      Value *is_3d = bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(), v, bld.mkImm(1));
      pred2d = bld.mkCmp(OP_SET, CC_EQ, TYPE_U32, bld.getSSA(1, FILE_PREDICATE),
                         TYPE_U32, bld.mkImm(0), is_3d);

      bld.mkOp2(OP_SHR, TYPE_U32, v, v, bld.loadImm(NULL, 16));
      su->moveSources(dim, 1);
      su->setSrc(dim, v);
      su->tex.target = nv50_ir::TEX_TARGET_3D;
      pos++;
   }

   if (su->tex.bindless)
      handle = bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(), ind, bld.mkImm(2047));
   else
      handle = loadTexHandle(ind, slot + 32);

   su->setSrc(arg + pos, handle);

   // The address check doesn't make sense here. The format check could make
   // sense but it's a bit of a pain.
   if (!su->tex.bindless) {
      // prevent read fault when the image is not actually bound
      pred =
         bld.mkCmp(OP_SET, CC_EQ, TYPE_U32, bld.getSSA(1, FILE_PREDICATE),
                   TYPE_U32, bld.mkImm(0),
                   loadSuInfo32(ind, slot, NVC0_SU_INFO_ADDR, su->tex.bindless));
      if (su->op != OP_SUSTP && su->tex.format) {
         const TexInstruction::ImgFormatDesc *format = su->tex.format;
         int blockwidth = format->bits[0] + format->bits[1] +
            format->bits[2] + format->bits[3];

         assert(format->components != 0);
         // make sure that the format doesn't mismatch when it's not FMT_NONE
         bld.mkCmp(OP_SET_OR, CC_NE, TYPE_U32, pred->getDef(0),
                   TYPE_U32, bld.loadImm(NULL, blockwidth / 8),
                   loadSuInfo32(ind, slot, NVC0_SU_INFO_BSIZE, su->tex.bindless),
                   pred->getDef(0));
      }
   }

   // Now we have "pred" which (optionally) contains whether to do the surface
   // op at all, and a "pred2d" which indicates that, in case of doing the
   // surface op, we have to create a 2d and 3d version, conditioned on pred2d.
   TexInstruction *su2d = NULL;
   if (pred2d) {
      su2d = cloneForward(func, su)->asTex();
      for (unsigned i = 0; su->defExists(i); ++i)
         su2d->setDef(i, bld.getSSA());
      su2d->moveSources(dim + 1, -1);
      su2d->tex.target = nv50_ir::TEX_TARGET_2D;
   }
   if (pred2d && pred) {
      Instruction *pred3d = bld.mkOp2(OP_AND, TYPE_U8,
                                      bld.getSSA(1, FILE_PREDICATE),
                                      pred->getDef(0), pred2d->getDef(0));
      pred3d->src(0).mod = Modifier(NV50_IR_MOD_NOT);
      pred3d->src(1).mod = Modifier(NV50_IR_MOD_NOT);
      su->setPredicate(CC_P, pred3d->getDef(0));
      pred2d = bld.mkOp2(OP_AND, TYPE_U8, bld.getSSA(1, FILE_PREDICATE),
                         pred->getDef(0), pred2d->getDef(0));
      pred2d->src(0).mod = Modifier(NV50_IR_MOD_NOT);
   } else if (pred) {
      su->setPredicate(CC_NOT_P, pred->getDef(0));
   } else if (pred2d) {
      su->setPredicate(CC_NOT_P, pred2d->getDef(0));
   }
   if (su2d) {
      su2d->setPredicate(CC_P, pred2d->getDef(0));
      bld.insert(su2d);

      // Create a UNION so that RA assigns the same registers
      bld.setPosition(su, true);
      for (unsigned i = 0; su->defExists(i); ++i) {
         assert(i < 4);

         Value *def = su->getDef(i);
         Value *newDef = bld.getSSA();
         ValueDef &def2 = su2d->def(i);
         Instruction *mov = NULL;

         su->setDef(i, newDef);
         if (pred) {
            mov = bld.mkMov(bld.getSSA(), bld.loadImm(NULL, 0));
            mov->setPredicate(CC_P, pred->getDef(0));
         }

         Instruction *uni = ret[i] = bld.mkOp2(OP_UNION, TYPE_U32,
                                      bld.getSSA(),
                                      newDef, def2.get());
         if (mov)
            uni->setSrc(2, mov->getDef(0));
         bld.mkMov(def, uni->getDef(0));
      }
   } else if (pred) {
      // Create a UNION so that RA assigns the same registers
      bld.setPosition(su, true);
      for (unsigned i = 0; su->defExists(i); ++i) {
         assert(i < 4);

         Value *def = su->getDef(i);
         Value *newDef = bld.getSSA();
         su->setDef(i, newDef);

         Instruction *mov = bld.mkMov(bld.getSSA(), bld.loadImm(NULL, 0));
         mov->setPredicate(CC_P, pred->getDef(0));

         Instruction *uni = ret[i] = bld.mkOp2(OP_UNION, TYPE_U32,
                                      bld.getSSA(),
                                      newDef, mov->getDef(0));
         bld.mkMov(def, uni->getDef(0));
      }
   }

   return su2d;
}

void
NVC0LoweringPass::handleSurfaceOpGM107(TexInstruction *su)
{
   // processSurfaceCoords also takes care of fixing up the outputs and
   // union'ing them with 0 as necessary. Additionally it may create a second
   // surface which needs some of the similar fixups.

   Instruction *loaded[4] = {};
   TexInstruction *su2 = processSurfaceCoordsGM107(su, loaded);

   if (su->op == OP_SULDP && su->tex.format) {
      convertSurfaceFormat(su, loaded);
   }

   if (su->op == OP_SUREDP) {
      su->op = OP_SUREDB;
   }

   // If we fixed up the type of the regular surface load instruction, we also
   // have to fix up the copy.
   if (su2) {
      su2->op = su->op;
      su2->dType = su->dType;
      su2->sType = su->sType;
   }
}

void
NVC0LoweringPass::handleLDST(Instruction *i)
{
   if (i->src(0).getFile() == FILE_SHADER_INPUT) {
      if (prog->getType() == Program::TYPE_COMPUTE) {
         i->getSrc(0)->reg.file = FILE_MEMORY_CONST;
         i->getSrc(0)->reg.fileIndex = 0;
      } else
      if (prog->getType() == Program::TYPE_GEOMETRY &&
          i->src(0).isIndirect(0)) {
         // XXX: this assumes vec4 units
         Value *ptr = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                                 i->getIndirect(0, 0), bld.mkImm(4));
         i->setIndirect(0, 0, ptr);
         i->op = OP_VFETCH;
      } else {
         i->op = OP_VFETCH;
         assert(prog->getType() != Program::TYPE_FRAGMENT); // INTERP
      }
   } else if (i->src(0).getFile() == FILE_MEMORY_CONST) {
      int8_t fileIndex = i->getSrc(0)->reg.fileIndex - 1;
      Value *ind = i->getIndirect(0, 1);

      if (targ->getChipset() >= NVISA_GK104_CHIPSET &&
          prog->getType() == Program::TYPE_COMPUTE &&
          (fileIndex >= 6 || ind)) {
         // The launch descriptor only allows to set up 8 CBs, but OpenGL
         // requires at least 12 UBOs. To bypass this limitation, for constant
         // buffers 7+, we store the addrs into the driver constbuf and we
         // directly load from the global memory.
         if (ind) {
            // Clamp the UBO index when an indirect access is used to avoid
            // loading information from the wrong place in the driver cb.
            // TODO - synchronize the max with the driver.
            ind = bld.mkOp2v(OP_MIN, TYPE_U32, bld.getSSA(),
                             bld.mkOp2v(OP_ADD, TYPE_U32, bld.getSSA(),
                                        ind, bld.loadImm(NULL, fileIndex)),
                             bld.loadImm(NULL, 13));
            fileIndex = 0;
         }

         Value *offset = bld.loadImm(NULL, i->getSrc(0)->reg.data.offset + typeSizeof(i->sType));
         Value *ptr = loadUboInfo64(ind, fileIndex * 16);
         Value *length = loadUboLength32(ind, fileIndex * 16);
         Value *pred = new_LValue(func, FILE_PREDICATE);
         if (i->src(0).isIndirect(0)) {
            bld.mkOp2(OP_ADD, TYPE_U64, ptr, ptr, i->getIndirect(0, 0));
            bld.mkOp2(OP_ADD, TYPE_U32, offset, offset, i->getIndirect(0, 0));
         }
         i->getSrc(0)->reg.file = FILE_MEMORY_GLOBAL;
         i->setIndirect(0, 1, NULL);
         i->setIndirect(0, 0, ptr);
         bld.mkCmp(OP_SET, CC_GT, TYPE_U32, pred, TYPE_U32, offset, length);
         i->setPredicate(CC_NOT_P, pred);
         Value *zero, *dst = i->getDef(0);
         i->setDef(0, bld.getSSA());

         bld.setPosition(i, true);
         bld.mkMov((zero = bld.getSSA()), bld.mkImm(0))
            ->setPredicate(CC_P, pred);
         bld.mkOp2(OP_UNION, TYPE_U32, dst, i->getDef(0), zero);
      } else if (i->src(0).isIndirect(1)) {
         Value *ptr;
         if (i->src(0).isIndirect(0))
            ptr = bld.mkOp3v(OP_INSBF, TYPE_U32, bld.getSSA(),
                             i->getIndirect(0, 1), bld.mkImm(0x1010),
                             i->getIndirect(0, 0));
         else
            ptr = bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                             i->getIndirect(0, 1), bld.mkImm(16));
         i->setIndirect(0, 1, NULL);
         i->setIndirect(0, 0, ptr);
         i->subOp = NV50_IR_SUBOP_LDC_IS;
      }
   } else if (i->src(0).getFile() == FILE_SHADER_OUTPUT) {
      assert(prog->getType() == Program::TYPE_TESSELLATION_CONTROL);
      i->op = OP_VFETCH;
   } else if (i->src(0).getFile() == FILE_MEMORY_BUFFER) {
      Value *ind = i->getIndirect(0, 1);
      Value *ptr = loadBufInfo64(ind, i->getSrc(0)->reg.fileIndex * 16);
      // XXX come up with a way not to do this for EVERY little access but
      // rather to batch these up somehow. Unfortunately we've lost the
      // information about the field width by the time we get here.
      Value *offset = bld.loadImm(NULL, i->getSrc(0)->reg.data.offset + typeSizeof(i->sType));
      Value *length = loadBufLength32(ind, i->getSrc(0)->reg.fileIndex * 16);
      Value *pred = new_LValue(func, FILE_PREDICATE);
      if (i->src(0).isIndirect(0)) {
         bld.mkOp2(OP_ADD, TYPE_U64, ptr, ptr, i->getIndirect(0, 0));
         bld.mkOp2(OP_ADD, TYPE_U32, offset, offset, i->getIndirect(0, 0));
      }
      i->setIndirect(0, 1, NULL);
      i->setIndirect(0, 0, ptr);
      i->getSrc(0)->reg.file = FILE_MEMORY_GLOBAL;
      bld.mkCmp(OP_SET, CC_GT, TYPE_U32, pred, TYPE_U32, offset, length);
      i->setPredicate(CC_NOT_P, pred);
      if (i->defExists(0)) {
         Value *zero, *dst = i->getDef(0);
         uint8_t size = dst->reg.size;
         i->setDef(0, bld.getSSA(size));

         bld.setPosition(i, true);
         bld.mkMov((zero = bld.getSSA(size)), bld.mkImm(0), i->dType)
            ->setPredicate(CC_P, pred);
         bld.mkOp2(OP_UNION, i->dType, dst, i->getDef(0), zero);
      }
   }
}

void
NVC0LoweringPass::readTessCoord(LValue *dst, int c)
{
   // In case of SPIRV the domain can be specified in the tesc shader,
   // but this should be passed to tese shader by merge_tess_info.
   const uint8_t domain = prog->driver_out->prop.tp.domain;
   assert(
      domain == MESA_PRIM_LINES ||
      domain == MESA_PRIM_TRIANGLES ||
      domain == MESA_PRIM_QUADS);

   Value *laneid = bld.getSSA();
   Value *x, *y;

   bld.mkOp1(OP_RDSV, TYPE_U32, laneid, bld.mkSysVal(SV_LANEID, 0));

   if (c == 0) {
      x = dst;
      y = NULL;
   } else
   if (c == 1) {
      x = NULL;
      y = dst;
   } else {
      assert(c == 2);
      if (domain != MESA_PRIM_TRIANGLES) {
         // optimize out tesscoord.z
         bld.mkMov(dst, bld.loadImm(NULL, 0));
         return;
      }
      x = bld.getSSA();
      y = bld.getSSA();
   }
   if (x)
      bld.mkFetch(x, TYPE_F32, FILE_SHADER_OUTPUT, 0x2f0, NULL, laneid);
   if (y)
      bld.mkFetch(y, TYPE_F32, FILE_SHADER_OUTPUT, 0x2f4, NULL, laneid);

   if (c == 2) {
      // compute tesscoord.z from x, y
      bld.mkOp2(OP_ADD, TYPE_F32, dst, x, y);
      bld.mkOp2(OP_SUB, TYPE_F32, dst, bld.loadImm(NULL, 1.0f), dst);
   }
}

bool
NVC0LoweringPass::handleRDSV(Instruction *i)
{
   Symbol *sym = i->getSrc(0)->asSym();
   const SVSemantic sv = sym->reg.data.sv.sv;
   Value *vtx = NULL;
   Instruction *ld;
   uint32_t addr = targ->getSVAddress(FILE_SHADER_INPUT, sym);

   if (addr >= 0x400) {
      // mov $sreg
      if (sym->reg.data.sv.index == 3) {
         // TGSI backend may use 4th component of TID,NTID,CTAID,NCTAID
         i->op = OP_MOV;
         i->setSrc(0, bld.mkImm((sv == SV_NTID || sv == SV_NCTAID) ? 1 : 0));
      } else
      if (sv == SV_TID) {
         // Help CSE combine TID fetches
         Value *tid = bld.mkOp1v(OP_RDSV, TYPE_U32, bld.getScratch(),
                                 bld.mkSysVal(SV_COMBINED_TID, 0));
         i->op = OP_EXTBF;
         i->setSrc(0, tid);
         switch (sym->reg.data.sv.index) {
         case 0: i->setSrc(1, bld.mkImm(0x1000)); break;
         case 1: i->setSrc(1, bld.mkImm(0x0a10)); break;
         case 2: i->setSrc(1, bld.mkImm(0x061a)); break;
         }
      }
      if (sv == SV_VERTEX_COUNT) {
         bld.setPosition(i, true);
         bld.mkOp2(OP_EXTBF, TYPE_U32, i->getDef(0), i->getDef(0), bld.mkImm(0x808));
      }
      return true;
   }

   switch (sv) {
   case SV_POSITION:
      assert(prog->getType() == Program::TYPE_FRAGMENT);
      if (i->srcExists(1)) {
         // Pass offset through to the interpolation logic
         ld = bld.mkInterp(NV50_IR_INTERP_LINEAR | NV50_IR_INTERP_OFFSET,
                           i->getDef(0), addr, NULL);
         ld->setSrc(1, i->getSrc(1));
      } else {
         bld.mkInterp(NV50_IR_INTERP_LINEAR, i->getDef(0), addr, NULL);
      }
      break;
   case SV_FACE:
   {
      Value *face = i->getDef(0);
      bld.mkInterp(NV50_IR_INTERP_FLAT, face, addr, NULL);
      if (i->dType == TYPE_F32) {
         bld.mkOp2(OP_OR, TYPE_U32, face, face, bld.mkImm(0x00000001));
         bld.mkOp1(OP_NEG, TYPE_S32, face, face);
         bld.mkCvt(OP_CVT, TYPE_F32, face, TYPE_S32, face);
      }
   }
      break;
   case SV_TESS_COORD:
      assert(prog->getType() == Program::TYPE_TESSELLATION_EVAL);
      readTessCoord(i->getDef(0)->asLValue(), i->getSrc(0)->reg.data.sv.index);
      break;
   case SV_NTID:
   case SV_NCTAID:
   case SV_GRIDID:
      assert(targ->getChipset() >= NVISA_GK104_CHIPSET); // mov $sreg otherwise
      if (sym->reg.data.sv.index == 3) {
         i->op = OP_MOV;
         i->setSrc(0, bld.mkImm(sv == SV_GRIDID ? 0 : 1));
         return true;
      }
      FALLTHROUGH;
   case SV_WORK_DIM:
      addr += prog->driver->prop.cp.gridInfoBase;
      bld.mkLoad(TYPE_U32, i->getDef(0),
                 bld.mkSymbol(FILE_MEMORY_CONST, prog->driver->io.auxCBSlot,
                              TYPE_U32, addr), NULL);
      break;
   case SV_SAMPLE_INDEX:
      // TODO: Properly pass source as an address in the PIX address space
      // (which can be of the form [r0+offset]). But this is currently
      // unnecessary.
      ld = bld.mkOp1(OP_PIXLD, TYPE_U32, i->getDef(0), bld.mkImm(0));
      ld->subOp = NV50_IR_SUBOP_PIXLD_SAMPLEID;
      break;
   case SV_SAMPLE_POS: {
      Value *sampleID = bld.getScratch();
      ld = bld.mkOp1(OP_PIXLD, TYPE_U32, sampleID, bld.mkImm(0));
      ld->subOp = NV50_IR_SUBOP_PIXLD_SAMPLEID;
      Value *offset = calculateSampleOffset(sampleID);

      assert(prog->driver_out->prop.fp.readsSampleLocations);

      if (targ->getChipset() >= NVISA_GM200_CHIPSET) {
         bld.mkLoad(TYPE_F32,
                    i->getDef(0),
                    bld.mkSymbol(
                          FILE_MEMORY_CONST, prog->driver->io.auxCBSlot,
                          TYPE_U32, prog->driver->io.sampleInfoBase),
                    offset);
         bld.mkOp2(OP_EXTBF, TYPE_U32, i->getDef(0), i->getDef(0),
                   bld.mkImm(0x040c + sym->reg.data.sv.index * 16));
         bld.mkCvt(OP_CVT, TYPE_F32, i->getDef(0), TYPE_U32, i->getDef(0));
         bld.mkOp2(OP_MUL, TYPE_F32, i->getDef(0), i->getDef(0), bld.mkImm(1.0f / 16.0f));
      } else {
         bld.mkLoad(TYPE_F32,
                    i->getDef(0),
                    bld.mkSymbol(
                          FILE_MEMORY_CONST, prog->driver->io.auxCBSlot,
                          TYPE_U32, prog->driver->io.sampleInfoBase +
                          4 * sym->reg.data.sv.index),
                    offset);
      }
      break;
   }
   case SV_SAMPLE_MASK: {
      ld = bld.mkOp1(OP_PIXLD, TYPE_U32, i->getDef(0), bld.mkImm(0));
      ld->subOp = NV50_IR_SUBOP_PIXLD_COVMASK;
      Instruction *sampleid =
         bld.mkOp1(OP_PIXLD, TYPE_U32, bld.getSSA(), bld.mkImm(0));
      sampleid->subOp = NV50_IR_SUBOP_PIXLD_SAMPLEID;
      Value *masked =
         bld.mkOp2v(OP_AND, TYPE_U32, bld.getSSA(), ld->getDef(0),
                    bld.mkOp2v(OP_SHL, TYPE_U32, bld.getSSA(),
                               bld.loadImm(NULL, 1), sampleid->getDef(0)));
      if (prog->persampleInvocation) {
         bld.mkMov(i->getDef(0), masked);
      } else {
         bld.mkOp3(OP_SELP, TYPE_U32, i->getDef(0), ld->getDef(0), masked,
                   bld.mkImm(0))
            ->subOp = 1;
      }
      break;
   }
   case SV_BASEVERTEX:
   case SV_BASEINSTANCE:
   case SV_DRAWID:
      ld = bld.mkLoad(TYPE_U32, i->getDef(0),
                      bld.mkSymbol(FILE_MEMORY_CONST,
                                   prog->driver->io.auxCBSlot,
                                   TYPE_U32,
                                   prog->driver->io.drawInfoBase +
                                   4 * (sv - SV_BASEVERTEX)),
                      NULL);
      break;
   default:
      if (prog->getType() == Program::TYPE_TESSELLATION_EVAL && !i->perPatch)
         vtx = bld.mkOp1v(OP_PFETCH, TYPE_U32, bld.getSSA(), bld.mkImm(0));
      if (prog->getType() == Program::TYPE_FRAGMENT) {
         bld.mkInterp(NV50_IR_INTERP_FLAT, i->getDef(0), addr, NULL);
      } else {
         ld = bld.mkFetch(i->getDef(0), i->dType,
                          FILE_SHADER_INPUT, addr, i->getIndirect(0, 0), vtx);
         ld->perPatch = i->perPatch;
      }
      break;
   }
   bld.getBB()->remove(i);
   return true;
}

bool
NVC0LoweringPass::handleDIV(Instruction *i)
{
   if (!isFloatType(i->dType))
      return true;
   bld.setPosition(i, false);
   Instruction *rcp = bld.mkOp1(OP_RCP, i->dType, bld.getSSA(typeSizeof(i->dType)), i->getSrc(1));
   i->op = OP_MUL;
   i->setSrc(1, rcp->getDef(0));
   return true;
}

bool
NVC0LoweringPass::handleMOD(Instruction *i)
{
   if (!isFloatType(i->dType))
      return true;
   LValue *value = bld.getScratch(typeSizeof(i->dType));
   bld.mkOp1(OP_RCP, i->dType, value, i->getSrc(1));
   bld.mkOp2(OP_MUL, i->dType, value, i->getSrc(0), value);
   bld.mkOp1(OP_TRUNC, i->dType, value, value);
   bld.mkOp2(OP_MUL, i->dType, value, i->getSrc(1), value);
   i->op = OP_SUB;
   i->setSrc(1, value);
   return true;
}

bool
NVC0LoweringPass::handleSQRT(Instruction *i)
{
   if (targ->isOpSupported(OP_SQRT, i->dType))
      return true;

   if (i->dType == TYPE_F64) {
      Value *pred = bld.getSSA(1, FILE_PREDICATE);
      Value *zero = bld.loadImm(NULL, 0.0);
      Value *dst = bld.getSSA(8);
      bld.mkOp1(OP_RSQ, i->dType, dst, i->getSrc(0));
      bld.mkCmp(OP_SET, CC_LE, i->dType, pred, i->dType, i->getSrc(0), zero);
      bld.mkOp3(OP_SELP, TYPE_U64, dst, zero, dst, pred);
      i->op = OP_MUL;
      i->setSrc(1, dst);
      // TODO: Handle this properly with a library function
   } else {
      bld.setPosition(i, true);
      i->op = OP_RSQ;
      bld.mkOp1(OP_RCP, i->dType, i->getDef(0), i->getDef(0));
   }

   return true;
}

bool
NVC0LoweringPass::handleEXPORT(Instruction *i)
{
   if (prog->getType() == Program::TYPE_FRAGMENT) {
      int id = i->getSrc(0)->reg.data.offset / 4;

      if (i->src(0).isIndirect(0)) // TODO, ugly
         return false;
      i->op = OP_MOV;
      i->subOp = NV50_IR_SUBOP_MOV_FINAL;
      i->src(0).set(i->src(1));
      i->setSrc(1, NULL);
      i->setDef(0, new_LValue(func, FILE_GPR));
      i->getDef(0)->reg.data.id = id;

      prog->maxGPR = MAX2(prog->maxGPR, id);
   } else
   if (prog->getType() == Program::TYPE_GEOMETRY) {
      i->setIndirect(0, 1, gpEmitAddress);
   }
   return true;
}

bool
NVC0LoweringPass::handleOUT(Instruction *i)
{
   Instruction *prev = i->prev;
   ImmediateValue stream, prevStream;

   // Only merge if the stream ids match. Also, note that the previous
   // instruction would have already been lowered, so we take arg1 from it.
   if (i->op == OP_RESTART && prev && prev->op == OP_EMIT &&
       i->src(0).getImmediate(stream) &&
       prev->src(1).getImmediate(prevStream) &&
       stream.reg.data.u32 == prevStream.reg.data.u32) {
      i->prev->subOp = NV50_IR_SUBOP_EMIT_RESTART;
      delete_Instruction(prog, i);
   } else {
      assert(gpEmitAddress);
      i->setDef(0, gpEmitAddress);
      i->setSrc(1, i->getSrc(0));
      i->setSrc(0, gpEmitAddress);
   }
   return true;
}

Value *
NVC0LoweringPass::calculateSampleOffset(Value *sampleID)
{
   Value *offset = bld.getScratch();
   if (targ->getChipset() >= NVISA_GM200_CHIPSET) {
      // Sample location offsets (in bytes) are calculated like so:
      // offset = (SV_POSITION.y % 4 * 2) + (SV_POSITION.x % 2)
      // offset = offset * 32 + sampleID % 8 * 4;
      // which is equivalent to:
      // offset = (SV_POSITION.y & 0x3) << 6 + (SV_POSITION.x & 0x1) << 5;
      // offset += sampleID << 2

      // The second operand (src1) of the INSBF instructions are like so:
      // 0xssll where ss is the size and ll is the offset.
      // so: dest = src2 | (src0 & (1 << ss - 1)) << ll

      // Add sample ID (offset = (sampleID & 0x7) << 2)
      bld.mkOp3(OP_INSBF, TYPE_U32, offset, sampleID, bld.mkImm(0x0302), bld.mkImm(0x0));

      Symbol *xSym = bld.mkSysVal(SV_POSITION, 0);
      Symbol *ySym = bld.mkSysVal(SV_POSITION, 1);
      Value *coord = bld.getScratch();

      // Add X coordinate (offset |= (SV_POSITION.x & 0x1) << 5)
      bld.mkInterp(NV50_IR_INTERP_LINEAR, coord,
                   targ->getSVAddress(FILE_SHADER_INPUT, xSym), NULL);
      bld.mkCvt(OP_CVT, TYPE_U32, coord, TYPE_F32, coord)
         ->rnd = ROUND_ZI;
      bld.mkOp3(OP_INSBF, TYPE_U32, offset, coord, bld.mkImm(0x0105), offset);

      // Add Y coordinate (offset |= (SV_POSITION.y & 0x3) << 6)
      bld.mkInterp(NV50_IR_INTERP_LINEAR, coord,
                   targ->getSVAddress(FILE_SHADER_INPUT, ySym), NULL);
      bld.mkCvt(OP_CVT, TYPE_U32, coord, TYPE_F32, coord)
         ->rnd = ROUND_ZI;
      bld.mkOp3(OP_INSBF, TYPE_U32, offset, coord, bld.mkImm(0x0206), offset);
   } else {
      bld.mkOp2(OP_SHL, TYPE_U32, offset, sampleID, bld.mkImm(3));
   }
   return offset;
}

// Handle programmable sample locations for GM20x+
void
NVC0LoweringPass::handlePIXLD(Instruction *i)
{
   if (i->subOp != NV50_IR_SUBOP_PIXLD_OFFSET)
      return;
   if (targ->getChipset() < NVISA_GM200_CHIPSET)
      return;

   assert(prog->driver_out->prop.fp.readsSampleLocations);

   bld.mkLoad(TYPE_F32,
              i->getDef(0),
              bld.mkSymbol(
                    FILE_MEMORY_CONST, prog->driver->io.auxCBSlot,
                    TYPE_U32, prog->driver->io.sampleInfoBase),
              calculateSampleOffset(i->getSrc(0)));

   bld.getBB()->remove(i);
}

// Generate a binary predicate if an instruction is predicated by
// e.g. an f32 value.
void
NVC0LoweringPass::checkPredicate(Instruction *insn)
{
   Value *pred = insn->getPredicate();
   Value *pdst;

   if (!pred || pred->reg.file == FILE_PREDICATE)
      return;
   pdst = new_LValue(func, FILE_PREDICATE);

   // CAUTION: don't use pdst->getInsn, the definition might not be unique,
   //  delay turning PSET(FSET(x,y),0) into PSET(x,y) to a later pass

   bld.mkCmp(OP_SET, CC_NEU, insn->dType, pdst, insn->dType, bld.mkImm(0), pred);

   insn->setPredicate(insn->cc, pdst);
}

//
// - add quadop dance for texturing
// - put FP outputs in GPRs
// - convert instruction sequences
//
bool
NVC0LoweringPass::visit(Instruction *i)
{
   bool ret = true;
   bld.setPosition(i, false);

   if (i->cc != CC_ALWAYS)
      checkPredicate(i);

   switch (i->op) {
   case OP_TEX:
   case OP_TXB:
   case OP_TXL:
   case OP_TXF:
   case OP_TXG:
      return handleTEX(i->asTex());
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
   case OP_DIV:
      return handleDIV(i);
   case OP_MOD:
      return handleMOD(i);
   case OP_SQRT:
      return handleSQRT(i);
   case OP_EXPORT:
      ret = handleEXPORT(i);
      break;
   case OP_EMIT:
   case OP_RESTART:
      return handleOUT(i);
   case OP_RDSV:
      return handleRDSV(i);
   case OP_STORE:
   case OP_LOAD:
      handleLDST(i);
      break;
   case OP_ATOM:
   {
      const bool cctl = i->src(0).getFile() == FILE_MEMORY_BUFFER;
      handleATOM(i);
      if (cctl)
         handleATOMCctl(i);
      handleCasExch(i);
   }
      break;
   case OP_SULDB:
   case OP_SULDP:
   case OP_SUSTB:
   case OP_SUSTP:
   case OP_SUREDB:
   case OP_SUREDP:
      if (targ->getChipset() >= NVISA_GM107_CHIPSET)
         handleSurfaceOpGM107(i->asTex());
      else if (targ->getChipset() >= NVISA_GK104_CHIPSET)
         handleSurfaceOpNVE4(i->asTex());
      else
         handleSurfaceOpNVC0(i->asTex());
      break;
   case OP_SUQ:
      handleSUQ(i->asTex());
      break;
   case OP_BUFQ:
      handleBUFQ(i);
      break;
   case OP_PIXLD:
      handlePIXLD(i);
      break;
   default:
      break;
   }

   /* Kepler+ has a special opcode to compute a new base address to be used
    * for indirect loads.
    *
    * Maxwell+ has an additional similar requirement for indirect
    * interpolation ops in frag shaders.
    */
   bool doAfetch = false;
   if (targ->getChipset() >= NVISA_GK104_CHIPSET &&
       !i->perPatch &&
       (i->op == OP_VFETCH || i->op == OP_EXPORT) &&
       i->src(0).isIndirect(0)) {
      doAfetch = true;
   }
   if (targ->getChipset() >= NVISA_GM107_CHIPSET &&
       (i->op == OP_LINTERP || i->op == OP_PINTERP) &&
       i->src(0).isIndirect(0)) {
      doAfetch = true;
   }

   if (doAfetch) {
      Value *addr = cloneShallow(func, i->getSrc(0));
      Instruction *afetch = bld.mkOp1(OP_AFETCH, TYPE_U32, bld.getSSA(),
                                      i->getSrc(0));
      afetch->setIndirect(0, 0, i->getIndirect(0, 0));
      addr->reg.data.offset = 0;
      i->setSrc(0, addr);
      i->setIndirect(0, 0, afetch->getDef(0));
      i->subOp = NV50_IR_SUBOP_VFETCH_PHYS;
   }

   return ret;
}

bool
TargetNVC0::runLegalizePass(Program *prog, CGStage stage) const
{
   if (stage == CG_STAGE_PRE_SSA) {
      NVC0LoweringPass pass(prog);
      return pass.run(prog, false, true);
   } else
   if (stage == CG_STAGE_POST_RA) {
      NVC0LegalizePostRA pass(prog);
      return pass.run(prog, false, true);
   } else
   if (stage == CG_STAGE_SSA) {
      NVC0LegalizeSSA pass;
      return pass.run(prog, false, true);
   }
   return false;
}

} // namespace nv50_ir
