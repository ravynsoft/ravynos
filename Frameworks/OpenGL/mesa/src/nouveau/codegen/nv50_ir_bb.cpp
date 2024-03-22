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

namespace nv50_ir {

Function::Function(Program *p, const char *fnName, uint32_t label)
   : call(this),
     label(label),
     name(fnName),
     prog(p)
{
   cfgExit = NULL;
   domTree = NULL;

   bbArray = NULL;
   bbCount = 0;
   loopNestingBound = 0;
   regClobberMax = 0;

   binPos = 0;
   binSize = 0;

   tlsBase = 0;
   tlsSize = 0;

   prog->add(this, id);
}

Function::~Function()
{
   prog->del(this, id);

   if (domTree)
      delete domTree;
   if (bbArray)
      delete[] bbArray;

   // clear value refs and defs
   ins.clear();
   outs.clear();

   for (ArrayList::Iterator it = allInsns.iterator(); !it.end(); it.next())
      delete_Instruction(prog, reinterpret_cast<Instruction *>(it.get()));

   for (ArrayList::Iterator it = allLValues.iterator(); !it.end(); it.next())
      delete_Value(prog, reinterpret_cast<LValue *>(it.get()));

   for (ArrayList::Iterator BBs = allBBlocks.iterator(); !BBs.end(); BBs.next())
      delete reinterpret_cast<BasicBlock *>(BBs.get());
}

BasicBlock::BasicBlock(Function *fn) : cfg(this), dom(this), func(fn)
{
   program = func->getProgram();

   joinAt = phi = entry = exit = NULL;

   numInsns = 0;
   binPos = 0;
   binSize = 0;

   explicitCont = false;

   func->add(this, this->id);
}

BasicBlock::~BasicBlock()
{
   // nothing yet
}

BasicBlock *
BasicBlock::clone(ClonePolicy<Function>& pol) const
{
   BasicBlock *bb = new BasicBlock(pol.context());

   pol.set(this, bb);

   for (Instruction *i = getFirst(); i; i = i->next)
      bb->insertTail(i->clone(pol));

   pol.context()->cfg.insert(&bb->cfg);

   for (Graph::EdgeIterator it = cfg.outgoing(); !it.end(); it.next()) {
      BasicBlock *obb = BasicBlock::get(it.getNode());
      bb->cfg.attach(&pol.get(obb)->cfg, it.getType());
   }

   return bb;
}

BasicBlock *
BasicBlock::idom() const
{
   Graph::Node *dn = dom.parent();
   return dn ? BasicBlock::get(dn) : NULL;
}

void
BasicBlock::insertHead(Instruction *inst)
{
   assert(inst->next == 0 && inst->prev == 0);

   if (inst->op == OP_PHI) {
      if (phi) {
         insertBefore(phi, inst);
      } else {
         if (entry) {
            insertBefore(entry, inst);
         } else {
            assert(!exit);
            phi = exit = inst;
            inst->bb = this;
            ++numInsns;
         }
      }
   } else {
      if (entry) {
         insertBefore(entry, inst);
      } else {
         if (phi) {
            insertAfter(exit, inst); // after last phi
         } else {
            assert(!exit);
            entry = exit = inst;
            inst->bb = this;
            ++numInsns;
         }
      }
   }
}

void
BasicBlock::insertTail(Instruction *inst)
{
   assert(inst->next == 0 && inst->prev == 0);

   if (inst->op == OP_PHI) {
      if (entry) {
         insertBefore(entry, inst);
      } else
      if (exit) {
         assert(phi);
         insertAfter(exit, inst);
      } else {
         assert(!phi);
         phi = exit = inst;
         inst->bb = this;
         ++numInsns;
      }
   } else {
      if (exit) {
         insertAfter(exit, inst);
      } else {
         assert(!phi);
         entry = exit = inst;
         inst->bb = this;
         ++numInsns;
      }
   }
}

void
BasicBlock::insertBefore(Instruction *q, Instruction *p)
{
   assert(p && q);

   assert(p->next == 0 && p->prev == 0);

   if (q == entry) {
      if (p->op == OP_PHI) {
         if (!phi)
            phi = p;
      } else {
         entry = p;
      }
   } else
   if (q == phi) {
      assert(p->op == OP_PHI);
      phi = p;
   }

   p->next = q;
   p->prev = q->prev;
   if (p->prev)
      p->prev->next = p;
   q->prev = p;

   p->bb = this;
   ++numInsns;
}

void
BasicBlock::insertAfter(Instruction *p, Instruction *q)
{
   assert(p && q);
   assert(q->op != OP_PHI || p->op == OP_PHI);

   assert(q->next == 0 && q->prev == 0);

   if (p == exit)
      exit = q;
   if (p->op == OP_PHI && q->op != OP_PHI)
      entry = q;

   q->prev = p;
   q->next = p->next;
   if (q->next)
      q->next->prev = q;
   p->next = q;

   q->bb = this;
   ++numInsns;
}

void
BasicBlock::remove(Instruction *insn)
{
   assert(insn->bb == this);

   if (insn->prev)
      insn->prev->next = insn->next;

   if (insn->next)
      insn->next->prev = insn->prev;
   else
      exit = insn->prev;

   if (insn == entry) {
      if (insn->next)
         entry = insn->next;
      else
      if (insn->prev && insn->prev->op != OP_PHI)
         entry = insn->prev;
      else
         entry = NULL;
   }

   if (insn == phi)
      phi = (insn->next && insn->next->op == OP_PHI) ? insn->next : 0;

   --numInsns;
   insn->bb = NULL;
   insn->next =
   insn->prev = NULL;
}

void BasicBlock::permuteAdjacent(Instruction *a, Instruction *b)
{
   assert(a->bb == b->bb);

   if (a->next != b) {
      Instruction *i = a;
      a = b;
      b = i;
   }
   assert(a->next == b);
   assert(a->op != OP_PHI && b->op != OP_PHI);

   if (b == exit)
      exit = a;
   if (a == entry)
      entry = b;

   b->prev = a->prev;
   a->next = b->next;
   b->next = a;
   a->prev = b;

   if (b->prev)
      b->prev->next = b;
   if (a->next)
      a->next->prev = a;
}

void
BasicBlock::splitCommon(Instruction *insn, BasicBlock *bb, bool attach)
{
   bb->entry = insn;

   if (insn) {
      exit = insn->prev;
      insn->prev = NULL;
   }

   if (exit)
      exit->next = NULL;
   else
      entry = NULL;

   while (!cfg.outgoing(true).end()) {
      Graph::Edge *e = cfg.outgoing(true).getEdge();
      bb->cfg.attach(e->getTarget(), e->getType());
      this->cfg.detach(e->getTarget());
   }

   for (; insn; insn = insn->next) {
      this->numInsns--;
      bb->numInsns++;
      insn->bb = bb;
      bb->exit = insn;
   }
   if (attach)
      this->cfg.attach(&bb->cfg, Graph::Edge::TREE);
}

BasicBlock *
BasicBlock::splitBefore(Instruction *insn, bool attach)
{
   BasicBlock *bb = new BasicBlock(func);
   assert(!insn || insn->op != OP_PHI);

   bb->joinAt = joinAt;
   joinAt = NULL;

   splitCommon(insn, bb, attach);
   return bb;
}

BasicBlock *
BasicBlock::splitAfter(Instruction *insn, bool attach)
{
   BasicBlock *bb = new BasicBlock(func);
   assert(!insn || insn->op != OP_PHI);

   bb->joinAt = joinAt;
   joinAt = NULL;

   splitCommon(insn ? insn->next : NULL, bb, attach);
   return bb;
}

bool
BasicBlock::dominatedBy(BasicBlock *that)
{
   Graph::Node *bn = &that->dom;
   Graph::Node *dn = &this->dom;

   while (dn && dn != bn)
      dn = dn->parent();

   return dn != NULL;
}

unsigned int
BasicBlock::initiatesSimpleConditional() const
{
   Graph::Node *out[2];
   int n;
   Graph::Edge::Type eR;

   if (cfg.outgoingCount() != 2) // -> if and -> else/endif
      return false;

   n = 0;
   for (Graph::EdgeIterator ei = cfg.outgoing(); !ei.end(); ei.next())
      out[n++] = ei.getNode();
   eR = out[1]->outgoing().getType();

   // IF block is out edge to the right
   if (eR == Graph::Edge::CROSS || eR == Graph::Edge::BACK)
      return 0x2;

   if (out[1]->outgoingCount() != 1) // 0 is IF { RET; }, >1 is more divergence
      return 0x0;
   // do they reconverge immediately ?
   if (out[1]->outgoing().getNode() == out[0])
      return 0x1;
   if (out[0]->outgoingCount() == 1)
      if (out[0]->outgoing().getNode() == out[1]->outgoing().getNode())
         return 0x3;

   return 0x0;
}

bool
Function::setEntry(BasicBlock *bb)
{
   if (cfg.getRoot())
      return false;
   cfg.insert(&bb->cfg);
   return true;
}

bool
Function::setExit(BasicBlock *bb)
{
   if (cfgExit)
      return false;
   cfgExit = &bb->cfg;
   return true;
}

unsigned int
Function::orderInstructions(ArrayList &result)
{
   result.clear();

   for (IteratorRef it = cfg.iteratorCFG(); !it->end(); it->next()) {
      BasicBlock *bb =
         BasicBlock::get(reinterpret_cast<Graph::Node *>(it->get()));

      for (Instruction *insn = bb->getFirst(); insn; insn = insn->next)
         result.insert(insn, insn->serial);
   }

   return result.getSize();
}

void
Function::buildLiveSets()
{
   for (unsigned i = 0; i <= loopNestingBound; ++i)
      buildLiveSetsPreSSA(BasicBlock::get(cfg.getRoot()), cfg.nextSequence());

   for (ArrayList::Iterator bi = allBBlocks.iterator(); !bi.end(); bi.next())
      BasicBlock::get(bi)->liveSet.marker = false;
}

bool
Pass::run(Program *prog, bool ordered, bool skipPhi)
{
   this->prog = prog;
   err = false;
   return doRun(prog, ordered, skipPhi);
}

bool
Pass::doRun(Program *prog, bool ordered, bool skipPhi)
{
   for (IteratorRef it = prog->calls.iteratorDFS(false);
        !it->end(); it->next()) {
      Graph::Node *n = reinterpret_cast<Graph::Node *>(it->get());
      if (!doRun(Function::get(n), ordered, skipPhi))
         return false;
   }
   return !err;
}

bool
Pass::run(Function *func, bool ordered, bool skipPhi)
{
   prog = func->getProgram();
   err = false;
   return doRun(func, ordered, skipPhi);
}

bool
Pass::doRun(Function *func, bool ordered, bool skipPhi)
{
   IteratorRef bbIter;
   BasicBlock *bb;
   Instruction *insn, *next;

   this->func = func;
   if (!visit(func))
      return false;

   bbIter = ordered ? func->cfg.iteratorCFG() : func->cfg.iteratorDFS();

   for (; !bbIter->end(); bbIter->next()) {
      bb = BasicBlock::get(reinterpret_cast<Graph::Node *>(bbIter->get()));
      if (!visit(bb))
         break;
      for (insn = skipPhi ? bb->getEntry() : bb->getFirst(); insn != NULL;
           insn = next) {
         next = insn->next;
         if (!visit(insn))
            break;
      }
   }

   return !err;
}

void
Function::printCFGraph(const char *filePath)
{
   FILE *out = fopen(filePath, "a");
   if (!out) {
      ERROR("failed to open file: %s\n", filePath);
      return;
   }
   INFO("printing control flow graph to: %s\n", filePath);

   fprintf(out, "digraph G {\n");

   for (IteratorRef it = cfg.iteratorDFS(); !it->end(); it->next()) {
      BasicBlock *bb = BasicBlock::get(
         reinterpret_cast<Graph::Node *>(it->get()));
      int idA = bb->getId();
      for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
         int idB = BasicBlock::get(ei.getNode())->getId();
         switch (ei.getType()) {
         case Graph::Edge::TREE:
            fprintf(out, "\t%i -> %i;\n", idA, idB);
            break;
         case Graph::Edge::FORWARD:
            fprintf(out, "\t%i -> %i [color=green];\n", idA, idB);
            break;
         case Graph::Edge::CROSS:
            fprintf(out, "\t%i -> %i [color=red];\n", idA, idB);
            break;
         case Graph::Edge::BACK:
            fprintf(out, "\t%i -> %i;\n", idA, idB);
            break;
         default:
            assert(0);
            break;
         }
      }
   }

   fprintf(out, "}\n");
   fclose(out);
}

} // namespace nv50_ir
