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

namespace nv50_ir {

// Converts nv50 IR generated from TGSI to SSA form.

// DominatorTree implements an algorithm for finding immediate dominators,
// as described by T. Lengauer & R. Tarjan.
class DominatorTree : public Graph
{
public:
   DominatorTree(Graph *cfg);
   ~DominatorTree() { }

   bool dominates(BasicBlock *, BasicBlock *);

   void findDominanceFrontiers();

private:
   void build();
   void buildDFS(Node *);

   void squash(int);
   inline void link(int, int);
   inline int eval(int);

   void debugPrint();

   Graph *cfg;

   Node **vert;
   int *data;
   const int count;

   #define SEMI(i)     (data[(i) + 0 * count])
   #define ANCESTOR(i) (data[(i) + 1 * count])
   #define PARENT(i)   (data[(i) + 2 * count])
   #define LABEL(i)    (data[(i) + 3 * count])
   #define DOM(i)      (data[(i) + 4 * count])
};

void DominatorTree::debugPrint()
{
   for (int i = 0; i < count; ++i) {
      INFO("SEMI(%i) = %i\n", i, SEMI(i));
      INFO("ANCESTOR(%i) = %i\n", i, ANCESTOR(i));
      INFO("PARENT(%i) = %i\n", i, PARENT(i));
      INFO("LABEL(%i) = %i\n", i, LABEL(i));
      INFO("DOM(%i) = %i\n", i, DOM(i));
   }
}

DominatorTree::DominatorTree(Graph *cfgraph) : cfg(cfgraph),
                                               count(cfg->getSize())
{
   int i = 0;

   vert = new Node * [count];
   data = new int[5 * count];

   for (IteratorRef it = cfg->iteratorDFS(true); !it->end(); it->next(), ++i) {
      vert[i] = reinterpret_cast<Node *>(it->get());
      vert[i]->tag = i;
      LABEL(i) = i;
      SEMI(i) = ANCESTOR(i) = -1;
   }
   assert(i == count);

   build();

   delete[] vert;
   delete[] data;
}

void DominatorTree::buildDFS(Graph::Node *node)
{
   SEMI(node->tag) = node->tag;

   for (Graph::EdgeIterator ei = node->outgoing(); !ei.end(); ei.next()) {
      if (SEMI(ei.getNode()->tag) < 0) {
         buildDFS(ei.getNode());
         PARENT(ei.getNode()->tag) = node->tag;
      }
   }
}

void DominatorTree::squash(int v)
{
   if (ANCESTOR(ANCESTOR(v)) >= 0) {
      squash(ANCESTOR(v));

      if (SEMI(LABEL(ANCESTOR(v))) < SEMI(LABEL(v)))
         LABEL(v) = LABEL(ANCESTOR(v));
      ANCESTOR(v) = ANCESTOR(ANCESTOR(v));
   }
}

int DominatorTree::eval(int v)
{
   if (ANCESTOR(v) < 0)
      return v;
   squash(v);
   return LABEL(v);
}

void DominatorTree::link(int v, int w)
{
   ANCESTOR(w) = v;
}

void DominatorTree::build()
{
   DLList *bucket = new DLList[count];
   Node *nv, *nw;
   int p, u, v, w;

   buildDFS(cfg->getRoot());

   for (w = count - 1; w >= 1; --w) {
      nw = vert[w];
      assert(nw->tag == w);
      for (Graph::EdgeIterator ei = nw->incident(); !ei.end(); ei.next()) {
         nv = ei.getNode();
         v = nv->tag;
         u = eval(v);
         if (SEMI(u) < SEMI(w))
            SEMI(w) = SEMI(u);
      }
      p = PARENT(w);
      bucket[SEMI(w)].insert(nw);
      link(p, w);

      for (DLList::Iterator it = bucket[p].iterator(); !it.end(); it.erase()) {
         v = reinterpret_cast<Node *>(it.get())->tag;
         u = eval(v);
         DOM(v) = (SEMI(u) < SEMI(v)) ? u : p;
      }
   }
   for (w = 1; w < count; ++w) {
      if (DOM(w) != SEMI(w))
         DOM(w) = DOM(DOM(w));
   }
   DOM(0) = 0;

   insert(&BasicBlock::get(cfg->getRoot())->dom);
   do {
      p = 0;
      for (v = 1; v < count; ++v) {
         nw = &BasicBlock::get(vert[DOM(v)])->dom;
         nv = &BasicBlock::get(vert[v])->dom;
         if (nw->getGraph() && !nv->getGraph()) {
            ++p;
            nw->attach(nv, Graph::Edge::TREE);
         }
      }
   } while (p);

   delete[] bucket;
}

#undef SEMI
#undef ANCESTOR
#undef PARENT
#undef LABEL
#undef DOM

void DominatorTree::findDominanceFrontiers()
{
   BasicBlock *bb;

   for (IteratorRef dtIt = iteratorDFS(false); !dtIt->end(); dtIt->next()) {
      EdgeIterator succIt, chldIt;

      bb = BasicBlock::get(reinterpret_cast<Node *>(dtIt->get()));
      bb->getDF().clear();

      for (succIt = bb->cfg.outgoing(); !succIt.end(); succIt.next()) {
         BasicBlock *dfLocal = BasicBlock::get(succIt.getNode());
         if (dfLocal->idom() != bb)
            bb->getDF().insert(dfLocal);
      }

      for (chldIt = bb->dom.outgoing(); !chldIt.end(); chldIt.next()) {
         BasicBlock *cb = BasicBlock::get(chldIt.getNode());

         DLList::Iterator dfIt = cb->getDF().iterator();
         for (; !dfIt.end(); dfIt.next()) {
            BasicBlock *dfUp = BasicBlock::get(dfIt);
            if (dfUp->idom() != bb)
               bb->getDF().insert(dfUp);
         }
      }
   }
}

// liveIn(bb) = usedBeforeAssigned(bb) U (liveOut(bb) - assigned(bb))
void
Function::buildLiveSetsPreSSA(BasicBlock *bb, const int seq)
{
   Function *f = bb->getFunction();
   BitSet usedBeforeAssigned(allLValues.getSize(), true);
   BitSet assigned(allLValues.getSize(), true);

   bb->liveSet.allocate(allLValues.getSize(), false);

   int n = 0;
   for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
      BasicBlock *out = BasicBlock::get(ei.getNode());
      if (out == bb)
         continue;
      if (out->cfg.visit(seq))
         buildLiveSetsPreSSA(out, seq);
      if (!n++)
         bb->liveSet = out->liveSet;
      else
         bb->liveSet |= out->liveSet;
   }
   if (!n && !bb->liveSet.marker)
      bb->liveSet.fill(0);
   bb->liveSet.marker = true;

   for (Instruction *i = bb->getEntry(); i; i = i->next) {
      for (int s = 0; i->srcExists(s); ++s)
         if (i->getSrc(s)->asLValue() && !assigned.test(i->getSrc(s)->id))
            usedBeforeAssigned.set(i->getSrc(s)->id);
      for (int d = 0; i->defExists(d); ++d)
         assigned.set(i->getDef(d)->id);
   }

   if (bb == BasicBlock::get(f->cfgExit)) {
      for (std::deque<ValueRef>::iterator it = f->outs.begin();
           it != f->outs.end(); ++it) {
         if (!assigned.test(it->get()->id))
            usedBeforeAssigned.set(it->get()->id);
      }
   }

   bb->liveSet.andNot(assigned);
   bb->liveSet |= usedBeforeAssigned;
}

void
Function::buildDefSetsPreSSA(BasicBlock *bb, const int seq)
{
   bb->defSet.allocate(allLValues.getSize(), !bb->liveSet.marker);
   bb->liveSet.marker = true;

   for (Graph::EdgeIterator ei = bb->cfg.incident(); !ei.end(); ei.next()) {
      BasicBlock *in = BasicBlock::get(ei.getNode());

      if (in->cfg.visit(seq))
         buildDefSetsPreSSA(in, seq);

      bb->defSet |= in->defSet;
   }

   for (Instruction *i = bb->getEntry(); i; i = i->next) {
      for (int d = 0; i->defExists(d); ++d)
         bb->defSet.set(i->getDef(d)->id);
   }
}

class RenamePass
{
public:
   RenamePass(Function *);
   ~RenamePass();

   bool run();
   void search(BasicBlock *);

   inline LValue *getStackTop(Value *);

   LValue *mkUndefined(Value *);

private:
   Stack *stack;
   Function *func;
   Program *prog;
};

bool
Program::convertToSSA()
{
   for (ArrayList::Iterator fi = allFuncs.iterator(); !fi.end(); fi.next()) {
      Function *fn = reinterpret_cast<Function *>(fi.get());
      if (!fn->convertToSSA())
         return false;
   }
   return true;
}

// XXX: add edge from entry to exit ?

// Efficiently Computing Static Single Assignment Form and
//  the Control Dependence Graph,
// R. Cytron, J. Ferrante, B. K. Rosen, M. N. Wegman, F. K. Zadeck
bool
Function::convertToSSA()
{
   // 0. calculate live in variables (for pruned SSA)
   buildLiveSets();

   // 1. create the dominator tree
   domTree = new DominatorTree(&cfg);
   reinterpret_cast<DominatorTree *>(domTree)->findDominanceFrontiers();

   // 2. insert PHI functions
   DLList workList;
   LValue *lval;
   BasicBlock *bb;
   unsigned int var;
   int iterCount = 0;
   int *hasAlready = new int[allBBlocks.getSize() * 2];
   int *work = &hasAlready[allBBlocks.getSize()];

   memset(hasAlready, 0, allBBlocks.getSize() * 2 * sizeof(int));

   // for each variable
   for (var = 0; var < allLValues.getSize(); ++var) {
      if (!allLValues.get(var))
         continue;
      lval = reinterpret_cast<Value *>(allLValues.get(var))->asLValue();
      if (!lval || lval->defs.empty())
         continue;
      ++iterCount;

      // TODO: don't add phi functions for values that aren't used outside
      //  the BB they're defined in

      // gather blocks with assignments to lval in workList
      for (Value::DefIterator d = lval->defs.begin();
           d != lval->defs.end(); ++d) {
         bb = ((*d)->getInsn() ? (*d)->getInsn()->bb : NULL);
         if (!bb)
            continue; // instruction likely been removed but not XXX deleted

         if (work[bb->getId()] == iterCount)
            continue;
         work[bb->getId()] = iterCount;
         workList.insert(bb);
      }

      // for each block in workList, insert a phi for lval in the block's
      //  dominance frontier (if we haven't already done so)
      for (DLList::Iterator wI = workList.iterator(); !wI.end(); wI.erase()) {
         bb = BasicBlock::get(wI);

         DLList::Iterator dfIter = bb->getDF().iterator();
         for (; !dfIter.end(); dfIter.next()) {
            Instruction *phi;
            BasicBlock *dfBB = BasicBlock::get(dfIter);

            if (hasAlready[dfBB->getId()] >= iterCount)
               continue;
            hasAlready[dfBB->getId()] = iterCount;

            // pruned SSA: don't need a phi if the value is not live-in
            if (!dfBB->liveSet.test(lval->id))
               continue;

            phi = new_Instruction(this, OP_PHI, typeOfSize(lval->reg.size));
            dfBB->insertTail(phi);

            phi->setDef(0, lval);
            for (int s = 0; s < dfBB->cfg.incidentCount(); ++s)
               phi->setSrc(s, lval);

            if (work[dfBB->getId()] < iterCount) {
               work[dfBB->getId()] = iterCount;
               wI.insert(dfBB);
            }
         }
      }
   }
   delete[] hasAlready;

   RenamePass rename(this);
   return rename.run();
}

RenamePass::RenamePass(Function *fn) : func(fn), prog(fn->getProgram())
{
   stack = new Stack[func->allLValues.getSize()];
}

RenamePass::~RenamePass()
{
   if (stack)
      delete[] stack;
}

LValue *
RenamePass::getStackTop(Value *val)
{
   if (!stack[val->id].getSize())
      return 0;
   return reinterpret_cast<LValue *>(stack[val->id].peek().u.p);
}

LValue *
RenamePass::mkUndefined(Value *val)
{
   LValue *lval = val->asLValue();
   assert(lval);
   LValue *ud = new_LValue(func, lval);
   Instruction *nop = new_Instruction(func, OP_NOP, typeOfSize(lval->reg.size));
   nop->setDef(0, ud);
   BasicBlock::get(func->cfg.getRoot())->insertHead(nop);
   return ud;
}

bool RenamePass::run()
{
   if (!stack)
      return false;
   search(BasicBlock::get(func->domTree->getRoot()));

   return true;
}

// Go through BBs in dominance order, create new values for each definition,
// and replace all sources with their current new values.
//
// NOTE: The values generated for function inputs/outputs have no connection
// to their corresponding outputs/inputs in other functions. Only allocation
// of physical registers will establish this connection.
//
void RenamePass::search(BasicBlock *bb)
{
   LValue *lval, *ssa;
   int d, s;
   const Target *targ = prog->getTarget();

   // Put current definitions for function inputs values on the stack.
   // They can be used before any redefinitions are pushed.
   if (bb == BasicBlock::get(func->cfg.getRoot())) {
      for (std::deque<ValueDef>::iterator it = func->ins.begin();
           it != func->ins.end(); ++it) {
         lval = it->get()->asLValue();
         assert(lval);

         ssa = new_LValue(func, targ->nativeFile(lval->reg.file));
         ssa->reg.size = lval->reg.size;
         ssa->reg.data.id = lval->reg.data.id;

         it->setSSA(ssa);
         stack[lval->id].push(ssa);
      }
   }

   for (Instruction *stmt = bb->getFirst(); stmt; stmt = stmt->next) {
      // PHI sources get definitions from the passes through the incident BBs,
      // so skip them here.
      if (stmt->op != OP_PHI) {
         for (s = 0; stmt->srcExists(s); ++s) {
            lval = stmt->getSrc(s)->asLValue();
            if (!lval)
               continue;
            // Values on the stack created in previously visited blocks, and
            // function inputs, will be valid because they dominate this one.
            lval = getStackTop(lval);
            if (!lval)
               lval = mkUndefined(stmt->getSrc(s));
            stmt->setSrc(s, lval);
         }
      }
      for (d = 0; stmt->defExists(d); ++d) {
         lval = stmt->def(d).get()->asLValue();
         assert(lval);
         stmt->def(d).setSSA(
            new_LValue(func, targ->nativeFile(lval->reg.file)));
         stmt->def(d).get()->reg.size = lval->reg.size;
         stmt->def(d).get()->reg.data.id = lval->reg.data.id;
         stack[lval->id].push(stmt->def(d).get());
      }
   }

   // Update sources of PHI ops corresponding to this BB in outgoing BBs.
   for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
      Instruction *phi;
      int p = 0;
      BasicBlock *sb = BasicBlock::get(ei.getNode());

      // which predecessor of sb is bb ?
      for (Graph::EdgeIterator ei = sb->cfg.incident(); !ei.end(); ei.next()) {
         if (ei.getNode() == &bb->cfg)
            break;
         ++p;
      }
      assert(p < sb->cfg.incidentCount());

      for (phi = sb->getPhi(); phi && phi->op == OP_PHI; phi = phi->next) {
         lval = getStackTop(phi->getSrc(p));
         if (!lval)
            lval = mkUndefined(phi->getSrc(p));
         phi->setSrc(p, lval);
      }
   }

   // Visit the BBs we dominate.
   for (Graph::EdgeIterator ei = bb->dom.outgoing(); !ei.end(); ei.next())
      search(BasicBlock::get(ei.getNode()));

   // Update function outputs to the last definitions of their pre-SSA values.
   // I hope they're unique, i.e. that we get PHIs for all of them ...
   if (bb == BasicBlock::get(func->cfgExit)) {
      for (std::deque<ValueRef>::iterator it = func->outs.begin();
           it != func->outs.end(); ++it) {
         lval = it->get()->asLValue();
         if (!lval)
            continue;
         lval = getStackTop(lval);
         if (!lval)
            lval = mkUndefined(it->get());
         it->set(lval);
      }
   }

   // Pop the values we created in this block from the stack because we will
   // return to blocks that we do not dominate.
   for (Instruction *stmt = bb->getFirst(); stmt; stmt = stmt->next) {
      if (stmt->op == OP_NOP)
         continue;
      for (d = 0; stmt->defExists(d); ++d)
         stack[stmt->def(d).preSSA()->id].pop();
   }
}

} // namespace nv50_ir
