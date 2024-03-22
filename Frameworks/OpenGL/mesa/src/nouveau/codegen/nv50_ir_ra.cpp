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

#include <algorithm>
#include <stack>
#include <limits>
#include <unordered_map>

namespace nv50_ir {
namespace {

#define MAX_REGISTER_FILE_SIZE 256

class RegisterSet
{
public:
   RegisterSet(const Target *);

   void init(const Target *);
   void reset(DataFile, bool resetMax = false);

   bool assign(int32_t& reg, DataFile f, unsigned int size, unsigned int maxReg);
   void occupy(DataFile f, int32_t reg, unsigned int size);
   void occupyMask(DataFile f, int32_t reg, uint8_t mask);
   bool isOccupied(DataFile f, int32_t reg, unsigned int size) const;
   bool testOccupy(DataFile f, int32_t reg, unsigned int size);

   inline int getMaxAssigned(DataFile f) const { return fill[f]; }

   inline unsigned int getFileSize(DataFile f) const
   {
      return last[f] + 1;
   }

   inline unsigned int units(DataFile f, unsigned int size) const
   {
      return size >> unit[f];
   }
   // for regs of size >= 4, id is counted in 4-byte words (like nv50/c0 binary)
   inline unsigned int idToBytes(const Value *v) const
   {
      return v->reg.data.id * MIN2(v->reg.size, 4);
   }
   inline unsigned int idToUnits(const Value *v) const
   {
      return units(v->reg.file, idToBytes(v));
   }
   inline int bytesToId(Value *v, unsigned int bytes) const
   {
      if (v->reg.size < 4)
         return units(v->reg.file, bytes);
      return bytes / 4;
   }
   inline int unitsToId(DataFile f, int u, uint8_t size) const
   {
      if (u < 0)
         return -1;
      return (size < 4) ? u : ((u << unit[f]) / 4);
   }

   void print(DataFile f) const;

   const bool restrictedGPR16Range;

private:
   BitSet bits[LAST_REGISTER_FILE + 1];

   int unit[LAST_REGISTER_FILE + 1]; // log2 of allocation granularity

   int last[LAST_REGISTER_FILE + 1];
   int fill[LAST_REGISTER_FILE + 1];
};

void
RegisterSet::reset(DataFile f, bool resetMax)
{
   bits[f].fill(0);
   if (resetMax)
      fill[f] = -1;
}

void
RegisterSet::init(const Target *targ)
{
   for (unsigned int rf = 0; rf <= LAST_REGISTER_FILE; ++rf) {
      DataFile f = static_cast<DataFile>(rf);
      last[rf] = targ->getFileSize(f) - 1;
      unit[rf] = targ->getFileUnit(f);
      fill[rf] = -1;
      assert(last[rf] < MAX_REGISTER_FILE_SIZE);
      bits[rf].allocate(last[rf] + 1, true);
   }
}

RegisterSet::RegisterSet(const Target *targ)
  : restrictedGPR16Range(targ->getChipset() < 0xc0)
{
   init(targ);
   for (unsigned int i = 0; i <= LAST_REGISTER_FILE; ++i)
      reset(static_cast<DataFile>(i));
}

void
RegisterSet::print(DataFile f) const
{
   INFO("GPR:");
   bits[f].print();
   INFO("\n");
}

bool
RegisterSet::assign(int32_t& reg, DataFile f, unsigned int size, unsigned int maxReg)
{
   reg = bits[f].findFreeRange(size, maxReg);
   if (reg < 0)
      return false;
   fill[f] = MAX2(fill[f], (int32_t)(reg + size - 1));
   return true;
}

bool
RegisterSet::isOccupied(DataFile f, int32_t reg, unsigned int size) const
{
   return bits[f].testRange(reg, size);
}

void
RegisterSet::occupyMask(DataFile f, int32_t reg, uint8_t mask)
{
   bits[f].setMask(reg & ~31, static_cast<uint32_t>(mask) << (reg % 32));
}

void
RegisterSet::occupy(DataFile f, int32_t reg, unsigned int size)
{
   bits[f].setRange(reg, size);

   INFO_DBG(0, REG_ALLOC, "reg occupy: %u[%i] %u\n", f, reg, size);

   fill[f] = MAX2(fill[f], (int32_t)(reg + size - 1));
}

bool
RegisterSet::testOccupy(DataFile f, int32_t reg, unsigned int size)
{
   if (isOccupied(f, reg, size))
      return false;
   occupy(f, reg, size);
   return true;
}

class RegAlloc
{
public:
   RegAlloc(Program *program) : prog(program), func(NULL), sequence(0) { }

   bool exec();
   bool execFunc();

private:
   class PhiMovesPass : public Pass {
   private:
      virtual bool visit(BasicBlock *);
      inline bool needNewElseBlock(BasicBlock *b, BasicBlock *p);
      inline void splitEdges(BasicBlock *b);
   };

   class BuildIntervalsPass : public Pass {
   private:
      virtual bool visit(BasicBlock *);
      void collectLiveValues(BasicBlock *);
      void addLiveRange(Value *, const BasicBlock *, int end);
   };

   class InsertConstraintsPass : public Pass {
   public:
      InsertConstraintsPass() : targ(NULL) { }
      bool exec(Function *func);
   private:
      virtual bool visit(BasicBlock *);

      void insertConstraintMove(Instruction *, int s);
      bool insertConstraintMoves();

      void condenseDefs(Instruction *);
      void condenseDefs(Instruction *, const int first, const int last);
      void condenseSrcs(Instruction *, const int first, const int last);

      void addHazard(Instruction *i, const ValueRef *src);
      void textureMask(TexInstruction *);

      // target specific functions, TODO: put in subclass or Target
      void texConstraintNV50(TexInstruction *);
      void texConstraintNVC0(TexInstruction *);
      void texConstraintNVE0(TexInstruction *);
      void texConstraintGM107(TexInstruction *);

      bool isScalarTexGM107(TexInstruction *);
      void handleScalarTexGM107(TexInstruction *);

      std::list<Instruction *> constrList;

      const Target *targ;
   };

   bool buildLiveSets(BasicBlock *);

private:
   Program *prog;
   Function *func;

   // instructions in control flow / chronological order
   ArrayList insns;

   int sequence; // for manual passes through CFG
};

typedef std::pair<Value *, Value *> ValuePair;

class MergedDefs
{
private:
   std::list<ValueDef *>& entry(Value *val) {
      auto it = defs.find(val);

      if (it == defs.end()) {
         std::list<ValueDef *> &res = defs[val];
         res = val->defs;
         return res;
      } else {
         return (*it).second;
      }
   }

   std::unordered_map<Value *, std::list<ValueDef *> > defs;

public:
   std::list<ValueDef *>& operator()(Value *val) {
      return entry(val);
   }

   void add(Value *val, const std::list<ValueDef *> &vals) {
      assert(val);
      std::list<ValueDef *> &valdefs = entry(val);
      valdefs.insert(valdefs.end(), vals.begin(), vals.end());
   }

   void removeDefsOfInstruction(Instruction *insn) {
      for (int d = 0; insn->defExists(d); ++d) {
         ValueDef *def = &insn->def(d);
         defs.erase(def->get());
         for (auto &p : defs)
            p.second.remove(def);
      }
   }

   void merge() {
      for (auto &p : defs)
         p.first->defs = p.second;
   }
};

class SpillCodeInserter
{
public:
   SpillCodeInserter(Function *fn, MergedDefs &mergedDefs) : func(fn), mergedDefs(mergedDefs), stackSize(0), stackBase(0) { }

   bool run(const std::list<ValuePair>&);

   Symbol *assignSlot(const Interval&, const unsigned int size);
   Value *offsetSlot(Value *, const LValue *);
   inline int32_t getStackSize() const { return stackSize; }

private:
   Function *func;
   MergedDefs &mergedDefs;

   int32_t stackSize;
   int32_t stackBase;

   LValue *unspill(Instruction *usei, LValue *, Value *slot);
   void spill(Instruction *defi, Value *slot, LValue *);
};

void
RegAlloc::BuildIntervalsPass::addLiveRange(Value *val,
                                           const BasicBlock *bb,
                                           int end)
{
   Instruction *insn = val->getUniqueInsn();

   if (!insn)
      insn = bb->getFirst();

   assert(bb->getFirst()->serial <= bb->getExit()->serial);
   assert(bb->getExit()->serial + 1 >= end);

   int begin = insn->serial;
   if (begin < bb->getEntry()->serial || begin > bb->getExit()->serial)
      begin = bb->getEntry()->serial;

   INFO_DBG(prog->dbgFlags, REG_ALLOC, "%%%i <- live range [%i(%i), %i)\n",
            val->id, begin, insn->serial, end);

   if (begin != end) // empty ranges are only added as hazards for fixed regs
      val->livei.extend(begin, end);
}

bool
RegAlloc::PhiMovesPass::needNewElseBlock(BasicBlock *b, BasicBlock *p)
{
   if (b->cfg.incidentCount() <= 1)
      return false;

   int n = 0;
   for (Graph::EdgeIterator ei = p->cfg.outgoing(); !ei.end(); ei.next())
      if (ei.getType() == Graph::Edge::TREE ||
          ei.getType() == Graph::Edge::FORWARD)
         ++n;
   return (n == 2);
}

struct PhiMapHash {
   size_t operator()(const std::pair<Instruction *, BasicBlock *>& val) const {
      return std::hash<Instruction*>()(val.first) * 31 +
         std::hash<BasicBlock*>()(val.second);
   }
};

typedef std::unordered_map<
   std::pair<Instruction *, BasicBlock *>, Value *, PhiMapHash> PhiMap;

// Critical edges need to be split up so that work can be inserted along
// specific edge transitions. Unfortunately manipulating incident edges into a
// BB invalidates all the PHI nodes since their sources are implicitly ordered
// by incident edge order.
//
// TODO: Make it so that that is not the case, and PHI nodes store pointers to
// the original BBs.
void
RegAlloc::PhiMovesPass::splitEdges(BasicBlock *bb)
{
   BasicBlock *pb, *pn;
   Instruction *phi;
   Graph::EdgeIterator ei;
   std::stack<BasicBlock *> stack;
   int j = 0;

   for (ei = bb->cfg.incident(); !ei.end(); ei.next()) {
      pb = BasicBlock::get(ei.getNode());
      assert(pb);
      if (needNewElseBlock(bb, pb))
         stack.push(pb);
   }

   // No critical edges were found, no need to perform any work.
   if (stack.empty())
      return;

   // We're about to, potentially, reorder the inbound edges. This means that
   // we need to hold on to the (phi, bb) -> src mapping, and fix up the phi
   // nodes after the graph has been modified.
   PhiMap phis;

   j = 0;
   for (ei = bb->cfg.incident(); !ei.end(); ei.next(), j++) {
      pb = BasicBlock::get(ei.getNode());
      for (phi = bb->getPhi(); phi && phi->op == OP_PHI; phi = phi->next)
         phis.insert(std::make_pair(std::make_pair(phi, pb), phi->getSrc(j)));
   }

   while (!stack.empty()) {
      pb = stack.top();
      pn = new BasicBlock(func);
      stack.pop();

      pb->cfg.detach(&bb->cfg);
      pb->cfg.attach(&pn->cfg, Graph::Edge::TREE);
      pn->cfg.attach(&bb->cfg, Graph::Edge::FORWARD);

      assert(pb->getExit()->op != OP_CALL);
      if (pb->getExit()->asFlow()->target.bb == bb)
         pb->getExit()->asFlow()->target.bb = pn;

      for (phi = bb->getPhi(); phi && phi->op == OP_PHI; phi = phi->next) {
         PhiMap::iterator it = phis.find(std::make_pair(phi, pb));
         assert(it != phis.end());
         phis.insert(std::make_pair(std::make_pair(phi, pn), it->second));
         phis.erase(it);
      }
   }

   // Now go through and fix up all of the phi node sources.
   j = 0;
   for (ei = bb->cfg.incident(); !ei.end(); ei.next(), j++) {
      pb = BasicBlock::get(ei.getNode());
      for (phi = bb->getPhi(); phi && phi->op == OP_PHI; phi = phi->next) {
         PhiMap::const_iterator it = phis.find(std::make_pair(phi, pb));
         assert(it != phis.end());

         phi->setSrc(j, it->second);
      }
   }
}

// For each operand of each PHI in b, generate a new value by inserting a MOV
// at the end of the block it is coming from and replace the operand with its
// result. This eliminates liveness conflicts and enables us to let values be
// copied to the right register if such a conflict exists nonetheless.
//
// These MOVs are also crucial in making sure the live intervals of phi srces
// are extended until the end of the loop, since they are not included in the
// live-in sets.
bool
RegAlloc::PhiMovesPass::visit(BasicBlock *bb)
{
   Instruction *phi, *mov;

   splitEdges(bb);

   // insert MOVs (phi->src(j) should stem from j-th in-BB)
   int j = 0;
   for (Graph::EdgeIterator ei = bb->cfg.incident(); !ei.end(); ei.next()) {
      BasicBlock *pb = BasicBlock::get(ei.getNode());
      if (!pb->isTerminated())
         pb->insertTail(new_FlowInstruction(func, OP_BRA, bb));

      for (phi = bb->getPhi(); phi && phi->op == OP_PHI; phi = phi->next) {
         LValue *tmp = new_LValue(func, phi->getDef(0)->asLValue());
         mov = new_Instruction(func, OP_MOV, typeOfSize(tmp->reg.size));

         mov->setSrc(0, phi->getSrc(j));
         mov->setDef(0, tmp);
         phi->setSrc(j, tmp);

         pb->insertBefore(pb->getExit(), mov);
      }
      ++j;
   }

   return true;
}

// Build the set of live-in variables of bb.
bool
RegAlloc::buildLiveSets(BasicBlock *bb)
{
   Function *f = bb->getFunction();
   BasicBlock *bn;
   Instruction *i;
   unsigned int s, d;

   INFO_DBG(prog->dbgFlags, REG_ALLOC, "buildLiveSets(BB:%i)\n", bb->getId());

   bb->liveSet.allocate(func->allLValues.getSize(), false);

   int n = 0;
   for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
      bn = BasicBlock::get(ei.getNode());
      if (bn == bb)
         continue;
      if (bn->cfg.visit(sequence))
         if (!buildLiveSets(bn))
            return false;
      if (n++ || bb->liveSet.marker)
         bb->liveSet |= bn->liveSet;
      else
         bb->liveSet = bn->liveSet;
   }
   if (!n && !bb->liveSet.marker)
      bb->liveSet.fill(0);
   bb->liveSet.marker = true;

   if (prog->dbgFlags & NV50_IR_DEBUG_REG_ALLOC) {
      INFO("BB:%i live set of out blocks:\n", bb->getId());
      bb->liveSet.print();
   }

   // if (!bb->getEntry())
   //   return true;

   if (bb == BasicBlock::get(f->cfgExit)) {
      for (std::deque<ValueRef>::iterator it = f->outs.begin();
           it != f->outs.end(); ++it) {
         assert(it->get()->asLValue());
         bb->liveSet.set(it->get()->id);
      }
   }

   for (i = bb->getExit(); i && i != bb->getEntry()->prev; i = i->prev) {
      for (d = 0; i->defExists(d); ++d)
         bb->liveSet.clr(i->getDef(d)->id);
      for (s = 0; i->srcExists(s); ++s)
         if (i->getSrc(s)->asLValue())
            bb->liveSet.set(i->getSrc(s)->id);
   }
   for (i = bb->getPhi(); i && i->op == OP_PHI; i = i->next)
      bb->liveSet.clr(i->getDef(0)->id);

   if (prog->dbgFlags & NV50_IR_DEBUG_REG_ALLOC) {
      INFO("BB:%i live set after propagation:\n", bb->getId());
      bb->liveSet.print();
   }

   return true;
}

void
RegAlloc::BuildIntervalsPass::collectLiveValues(BasicBlock *bb)
{
   BasicBlock *bbA = NULL, *bbB = NULL;

   if (bb->cfg.outgoingCount()) {
      // trickery to save a loop of OR'ing liveSets
      // aliasing works fine with BitSet::setOr
      for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
         if (bbA) {
            bb->liveSet.setOr(&bbA->liveSet, &bbB->liveSet);
            bbA = bb;
         } else {
            bbA = bbB;
         }
         bbB = BasicBlock::get(ei.getNode());
      }
      bb->liveSet.setOr(&bbB->liveSet, bbA ? &bbA->liveSet : NULL);
   } else
   if (bb->cfg.incidentCount()) {
      bb->liveSet.fill(0);
   }
}

bool
RegAlloc::BuildIntervalsPass::visit(BasicBlock *bb)
{
   collectLiveValues(bb);

   INFO_DBG(prog->dbgFlags, REG_ALLOC, "BuildIntervals(BB:%i)\n", bb->getId());

   // go through out blocks and delete phi sources that do not originate from
   // the current block from the live set
   for (Graph::EdgeIterator ei = bb->cfg.outgoing(); !ei.end(); ei.next()) {
      BasicBlock *out = BasicBlock::get(ei.getNode());

      for (Instruction *i = out->getPhi(); i && i->op == OP_PHI; i = i->next) {
         bb->liveSet.clr(i->getDef(0)->id);

         for (int s = 0; i->srcExists(s); ++s) {
            assert(i->src(s).getInsn());
            if (i->getSrc(s)->getUniqueInsn()->bb == bb) // XXX: reachableBy ?
               bb->liveSet.set(i->getSrc(s)->id);
            else
               bb->liveSet.clr(i->getSrc(s)->id);
         }
      }
   }

   // remaining live-outs are live until end
   if (bb->getExit()) {
      for (unsigned int j = 0; j < bb->liveSet.getSize(); ++j)
         if (bb->liveSet.test(j))
            addLiveRange(func->getLValue(j), bb, bb->getExit()->serial + 1);
   }

   for (Instruction *i = bb->getExit(); i && i->op != OP_PHI; i = i->prev) {
      for (int d = 0; i->defExists(d); ++d) {
         bb->liveSet.clr(i->getDef(d)->id);
         if (i->getDef(d)->reg.data.id >= 0) // add hazard for fixed regs
            i->getDef(d)->livei.extend(i->serial, i->serial);
      }

      for (int s = 0; i->srcExists(s); ++s) {
         if (!i->getSrc(s)->asLValue())
            continue;
         if (!bb->liveSet.test(i->getSrc(s)->id)) {
            bb->liveSet.set(i->getSrc(s)->id);
            addLiveRange(i->getSrc(s), bb, i->serial);
         }
      }
   }

   if (bb == BasicBlock::get(func->cfg.getRoot())) {
      for (std::deque<ValueDef>::iterator it = func->ins.begin();
           it != func->ins.end(); ++it) {
         if (it->get()->reg.data.id >= 0) // add hazard for fixed regs
            it->get()->livei.extend(0, 1);
      }
   }

   return true;
}


#define JOIN_MASK_PHI        (1 << 0)
#define JOIN_MASK_UNION      (1 << 1)
#define JOIN_MASK_MOV        (1 << 2)
#define JOIN_MASK_TEX        (1 << 3)

class GCRA
{
public:
   GCRA(Function *, SpillCodeInserter&, MergedDefs&);
   ~GCRA();

   bool allocateRegisters(ArrayList& insns);

   void printNodeInfo() const;

private:
   class RIG_Node : public Graph::Node
   {
   public:
      RIG_Node();

      void init(const RegisterSet&, LValue *);

      void addInterference(RIG_Node *);
      void addRegPreference(RIG_Node *);

      inline LValue *getValue() const
      {
         return reinterpret_cast<LValue *>(data);
      }
      inline void setValue(LValue *lval) { data = lval; }

      inline uint8_t getCompMask() const
      {
         return ((1 << colors) - 1) << (reg & 7);
      }

      static inline RIG_Node *get(const Graph::EdgeIterator& ei)
      {
         return static_cast<RIG_Node *>(ei.getNode());
      }

   public:
      uint32_t degree;
      uint16_t degreeLimit; // if deg < degLimit, node is trivially colourable
      uint16_t maxReg;
      uint16_t colors;

      DataFile f;
      int32_t reg;

      float weight;

      // list pointers for simplify() phase
      RIG_Node *next;
      RIG_Node *prev;

      // union of the live intervals of all coalesced values (we want to retain
      //  the separate intervals for testing interference of compound values)
      Interval livei;

      std::list<RIG_Node *> prefRegs;
   };

private:
   inline RIG_Node *getNode(const LValue *v) const { return &nodes[v->id]; }

   void buildRIG(ArrayList&);
   bool coalesce(ArrayList&);
   bool doCoalesce(ArrayList&, unsigned int mask);
   void calculateSpillWeights();
   bool simplify();
   bool selectRegisters();
   void cleanup(const bool success);

   void simplifyEdge(RIG_Node *, RIG_Node *);
   void simplifyNode(RIG_Node *);

   void copyCompound(Value *dst, Value *src);
   bool coalesceValues(Value *, Value *, bool force);
   void resolveSplitsAndMerges();
   void makeCompound(Instruction *, bool isSplit);

   inline void checkInterference(const RIG_Node *, Graph::EdgeIterator&);

   inline void insertOrderedTail(std::list<RIG_Node *>&, RIG_Node *);
   void checkList(std::list<RIG_Node *>&);

private:
   std::stack<uint32_t> stack;

   // list headers for simplify() phase
   RIG_Node lo[2];
   RIG_Node hi;

   Graph RIG;
   RIG_Node *nodes;
   unsigned int nodeCount;

   Function *func;
   Program *prog;

   struct RelDegree {
      uint8_t data[17][17];

      RelDegree() {
         for (int i = 1; i <= 16; ++i)
            for (int j = 1; j <= 16; ++j)
               data[i][j] = j * ((i + j - 1) / j);
      }

      const uint8_t* operator[](std::size_t i) const {
         return data[i];
      }
   };

   static const RelDegree relDegree;

   RegisterSet regs;

   // need to fixup register id for participants of OP_MERGE/SPLIT
   std::list<Instruction *> merges;
   std::list<Instruction *> splits;

   SpillCodeInserter& spill;
   std::list<ValuePair> mustSpill;

   MergedDefs &mergedDefs;
};

const GCRA::RelDegree GCRA::relDegree;

GCRA::RIG_Node::RIG_Node() : Node(NULL), degree(0), degreeLimit(0), maxReg(0),
   colors(0), f(FILE_NULL), reg(0), weight(0), next(this), prev(this)
{
}

void
GCRA::printNodeInfo() const
{
   for (unsigned int i = 0; i < nodeCount; ++i) {
      if (!nodes[i].colors)
         continue;
      INFO("RIG_Node[%%%i]($[%u]%i): %u colors, weight %f, deg %u/%u\n X",
           i,
           nodes[i].f,nodes[i].reg,nodes[i].colors,
           nodes[i].weight,
           nodes[i].degree, nodes[i].degreeLimit);

      for (Graph::EdgeIterator ei = nodes[i].outgoing(); !ei.end(); ei.next())
         INFO(" %%%i", RIG_Node::get(ei)->getValue()->id);
      for (Graph::EdgeIterator ei = nodes[i].incident(); !ei.end(); ei.next())
         INFO(" %%%i", RIG_Node::get(ei)->getValue()->id);
      INFO("\n");
   }
}

static bool
isShortRegOp(Instruction *insn)
{
   // Immediates are always in src1 (except zeroes, which end up getting
   // replaced with a zero reg). Every other situation can be resolved by
   // using a long encoding.
   return insn->srcExists(1) && insn->src(1).getFile() == FILE_IMMEDIATE &&
      insn->getSrc(1)->reg.data.u64;
}

// Check if this LValue is ever used in an instruction that can't be encoded
// with long registers (i.e. > r63)
static bool
isShortRegVal(LValue *lval)
{
   if (lval->getInsn() == NULL)
      return false;
   for (Value::DefCIterator def = lval->defs.begin();
        def != lval->defs.end(); ++def)
      if (isShortRegOp((*def)->getInsn()))
         return true;
   for (Value::UseCIterator use = lval->uses.begin();
        use != lval->uses.end(); ++use)
      if (isShortRegOp((*use)->getInsn()))
         return true;
   return false;
}

void
GCRA::RIG_Node::init(const RegisterSet& regs, LValue *lval)
{
   setValue(lval);
   if (lval->reg.data.id >= 0)
      lval->noSpill = lval->fixedReg = 1;

   colors = regs.units(lval->reg.file, lval->reg.size);
   f = lval->reg.file;
   reg = -1;
   if (lval->reg.data.id >= 0)
      reg = regs.idToUnits(lval);

   weight = std::numeric_limits<float>::infinity();
   degree = 0;
   maxReg = regs.getFileSize(f);
   // On nv50, we lose a bit of gpr encoding when there's an embedded
   // immediate.
   if (regs.restrictedGPR16Range && f == FILE_GPR && (lval->reg.size == 2 || isShortRegVal(lval)))
      maxReg /= 2;
   degreeLimit = maxReg;
   degreeLimit -= relDegree[1][colors] - 1;

   livei.insert(lval->livei);
}

// Used when coalescing moves. The non-compound value will become one, e.g.:
// mov b32 $r0 $r2            / merge b64 $r0d { $r0 $r1 }
// split b64 { $r0 $r1 } $r0d / mov b64 $r0d f64 $r2d
void
GCRA::copyCompound(Value *dst, Value *src)
{
   LValue *ldst = dst->asLValue();
   LValue *lsrc = src->asLValue();

   if (ldst->compound && !lsrc->compound) {
      LValue *swap = lsrc;
      lsrc = ldst;
      ldst = swap;
   }

   assert(!ldst->compound);

   if (lsrc->compound) {
      for (ValueDef *d : mergedDefs(ldst->join)) {
         LValue *ldst = d->get()->asLValue();
         if (!ldst->compound)
            ldst->compMask = 0xff;
         ldst->compound = 1;
         ldst->compMask &= lsrc->compMask;
      }
   }
}

bool
GCRA::coalesceValues(Value *dst, Value *src, bool force)
{
   LValue *rep = dst->join->asLValue();
   LValue *val = src->join->asLValue();

   if (!force && val->reg.data.id >= 0) {
      rep = src->join->asLValue();
      val = dst->join->asLValue();
   }
   RIG_Node *nRep = &nodes[rep->id];
   RIG_Node *nVal = &nodes[val->id];

   if (src->reg.file != dst->reg.file) {
      if (!force)
         return false;
      WARN("forced coalescing of values in different files !\n");
   }
   if (!force && dst->reg.size != src->reg.size)
      return false;

   if ((rep->reg.data.id >= 0) && (rep->reg.data.id != val->reg.data.id)) {
      if (force) {
         if (val->reg.data.id >= 0)
            WARN("forced coalescing of values in different fixed regs !\n");
      } else {
         if (val->reg.data.id >= 0)
            return false;
         // make sure that there is no overlap with the fixed register of rep
         for (ArrayList::Iterator it = func->allLValues.iterator();
              !it.end(); it.next()) {
            Value *reg = reinterpret_cast<Value *>(it.get())->asLValue();
            assert(reg);
            if (reg->interfers(rep) && reg->livei.overlaps(nVal->livei))
               return false;
         }
      }
   }

   if (!force && nRep->livei.overlaps(nVal->livei))
      return false;

   // TODO: Handle this case properly.
   if (!force && rep->compound && val->compound)
      return false;

   INFO_DBG(prog->dbgFlags, REG_ALLOC, "joining %%%i($%i) <- %%%i\n",
            rep->id, rep->reg.data.id, val->id);

   if (!force)
      copyCompound(dst, src);

   // set join pointer of all values joined with val
   const std::list<ValueDef *> &defs = mergedDefs(val);
   for (ValueDef *def : defs)
      def->get()->join = rep;
   assert(rep->join == rep && val->join == rep);

   // add val's definitions to rep and extend the live interval of its RIG node
   mergedDefs.add(rep, defs);
   nRep->livei.unify(nVal->livei);
   nRep->degreeLimit = MIN2(nRep->degreeLimit, nVal->degreeLimit);
   nRep->maxReg = MIN2(nRep->maxReg, nVal->maxReg);
   return true;
}

bool
GCRA::coalesce(ArrayList& insns)
{
   bool ret = doCoalesce(insns, JOIN_MASK_PHI);
   if (!ret)
      return false;
   switch (func->getProgram()->getTarget()->getChipset() & ~0xf) {
   case 0x50:
   case 0x80:
   case 0x90:
   case 0xa0:
      ret = doCoalesce(insns, JOIN_MASK_UNION | JOIN_MASK_TEX);
      break;
   case 0xc0:
   case 0xd0:
   case 0xe0:
   case 0xf0:
   case 0x100:
   case 0x110:
   case 0x120:
   case 0x130:
   case 0x140:
   case 0x160:
   case 0x170:
   case 0x190:
      ret = doCoalesce(insns, JOIN_MASK_UNION);
      break;
   default:
      break;
   }
   if (!ret)
      return false;
   return doCoalesce(insns, JOIN_MASK_MOV);
}

static inline uint8_t makeCompMask(int compSize, int base, int size)
{
   uint8_t m = ((1 << size) - 1) << base;

   switch (compSize) {
   case 1:
      return 0xff;
   case 2:
      m |= (m << 2);
      return (m << 4) | m;
   case 3:
   case 4:
      return (m << 4) | m;
   default:
      assert(compSize <= 8);
      return m;
   }
}

void
GCRA::makeCompound(Instruction *insn, bool split)
{
   LValue *rep = (split ? insn->getSrc(0) : insn->getDef(0))->asLValue();

   if (prog->dbgFlags & NV50_IR_DEBUG_REG_ALLOC) {
      INFO("makeCompound(split = %i): ", split);
      insn->print();
   }

   const unsigned int size = getNode(rep)->colors;
   unsigned int base = 0;

   if (!rep->compound)
      rep->compMask = 0xff;
   rep->compound = 1;

   for (int c = 0; split ? insn->defExists(c) : insn->srcExists(c); ++c) {
      LValue *val = (split ? insn->getDef(c) : insn->getSrc(c))->asLValue();

      val->compound = 1;
      if (!val->compMask)
         val->compMask = 0xff;
      val->compMask &= makeCompMask(size, base, getNode(val)->colors);
      assert(val->compMask);

      INFO_DBG(prog->dbgFlags, REG_ALLOC, "compound: %%%i:%02x <- %%%i:%02x\n",
           rep->id, rep->compMask, val->id, val->compMask);

      base += getNode(val)->colors;
   }
   assert(base == size);
}

bool
GCRA::doCoalesce(ArrayList& insns, unsigned int mask)
{
   int c;

   for (unsigned int n = 0; n < insns.getSize(); ++n) {
      Instruction *i;
      Instruction *insn = reinterpret_cast<Instruction *>(insns.get(n));

      switch (insn->op) {
      case OP_PHI:
         if (!(mask & JOIN_MASK_PHI))
            break;
         for (c = 0; insn->srcExists(c); ++c)
            if (!coalesceValues(insn->getDef(0), insn->getSrc(c), false)) {
               // this is bad
               ERROR("failed to coalesce phi operands\n");
               return false;
            }
         break;
      case OP_UNION:
      case OP_MERGE:
         if (!(mask & JOIN_MASK_UNION))
            break;
         for (c = 0; insn->srcExists(c); ++c)
            coalesceValues(insn->getDef(0), insn->getSrc(c), true);
         if (insn->op == OP_MERGE) {
            merges.push_back(insn);
            if (insn->srcExists(1))
               makeCompound(insn, false);
         }
         break;
      case OP_SPLIT:
         if (!(mask & JOIN_MASK_UNION))
            break;
         splits.push_back(insn);
         for (c = 0; insn->defExists(c); ++c)
            coalesceValues(insn->getSrc(0), insn->getDef(c), true);
         makeCompound(insn, true);
         break;
      case OP_MOV:
         if (!(mask & JOIN_MASK_MOV))
            break;
         i = NULL;
         if (!insn->getDef(0)->uses.empty())
            i = (*insn->getDef(0)->uses.begin())->getInsn();
         // if this is a contraint-move there will only be a single use
         if (i && i->op == OP_MERGE) // do we really still need this ?
            break;
         i = insn->getSrc(0)->getUniqueInsn();
         if (i && !i->constrainedDefs()) {
            coalesceValues(insn->getDef(0), insn->getSrc(0), false);
         }
         break;
      case OP_TEX:
      case OP_TXB:
      case OP_TXL:
      case OP_TXF:
      case OP_TXQ:
      case OP_TXD:
      case OP_TXG:
      case OP_TXLQ:
      case OP_TEXCSAA:
      case OP_TEXPREP:
         if (!(mask & JOIN_MASK_TEX))
            break;
         for (c = 0; insn->srcExists(c) && c != insn->predSrc; ++c)
            coalesceValues(insn->getDef(c), insn->getSrc(c), true);
         break;
      default:
         break;
      }
   }
   return true;
}

void
GCRA::RIG_Node::addInterference(RIG_Node *node)
{
   this->degree += relDegree[node->colors][colors];
   node->degree += relDegree[colors][node->colors];

   this->attach(node, Graph::Edge::CROSS);
}

void
GCRA::RIG_Node::addRegPreference(RIG_Node *node)
{
   prefRegs.push_back(node);
}

GCRA::GCRA(Function *fn, SpillCodeInserter& spill, MergedDefs& mergedDefs) :
   nodes(NULL),
   nodeCount(0),
   func(fn),
   regs(fn->getProgram()->getTarget()),
   spill(spill),
   mergedDefs(mergedDefs)
{
   prog = func->getProgram();
}

GCRA::~GCRA()
{
   if (nodes)
      delete[] nodes;
}

void
GCRA::checkList(std::list<RIG_Node *>& lst)
{
   GCRA::RIG_Node *prev = NULL;

   for (std::list<RIG_Node *>::iterator it = lst.begin();
        it != lst.end();
        ++it) {
      assert((*it)->getValue()->join == (*it)->getValue());
      if (prev)
         assert(prev->livei.begin() <= (*it)->livei.begin());
      prev = *it;
   }
}

void
GCRA::insertOrderedTail(std::list<RIG_Node *>& list, RIG_Node *node)
{
   if (node->livei.isEmpty())
      return;
   // only the intervals of joined values don't necessarily arrive in order
   std::list<RIG_Node *>::iterator prev, it;
   for (it = list.end(); it != list.begin(); it = prev) {
      prev = it;
      --prev;
      if ((*prev)->livei.begin() <= node->livei.begin())
         break;
   }
   list.insert(it, node);
}

void
GCRA::buildRIG(ArrayList& insns)
{
   std::list<RIG_Node *> values, active;

   for (std::deque<ValueDef>::iterator it = func->ins.begin();
        it != func->ins.end(); ++it)
      insertOrderedTail(values, getNode(it->get()->asLValue()));

   for (unsigned int i = 0; i < insns.getSize(); ++i) {
      Instruction *insn = reinterpret_cast<Instruction *>(insns.get(i));
      for (int d = 0; insn->defExists(d); ++d)
         if (insn->getDef(d)->reg.file <= LAST_REGISTER_FILE &&
             insn->getDef(d)->rep() == insn->getDef(d))
            insertOrderedTail(values, getNode(insn->getDef(d)->asLValue()));
   }
   checkList(values);

   while (!values.empty()) {
      RIG_Node *cur = values.front();

      for (std::list<RIG_Node *>::iterator it = active.begin();
           it != active.end();) {
         RIG_Node *node = *it;

         if (node->livei.end() <= cur->livei.begin()) {
            it = active.erase(it);
         } else {
            if (node->f == cur->f && node->livei.overlaps(cur->livei))
               cur->addInterference(node);
            ++it;
         }
      }
      values.pop_front();
      active.push_back(cur);
   }
}

void
GCRA::calculateSpillWeights()
{
   for (unsigned int i = 0; i < nodeCount; ++i) {
      RIG_Node *const n = &nodes[i];
      if (!nodes[i].colors || nodes[i].livei.isEmpty())
         continue;
      if (nodes[i].reg >= 0) {
         // update max reg
         regs.occupy(n->f, n->reg, n->colors);
         continue;
      }
      LValue *val = nodes[i].getValue();

      if (!val->noSpill) {
         int rc = 0;
         for (ValueDef *def : mergedDefs(val))
            rc += def->get()->refCount();

         nodes[i].weight =
            (float)rc * (float)rc / (float)nodes[i].livei.extent();
      }

      if (nodes[i].degree < nodes[i].degreeLimit) {
         int l = 0;
         if (val->reg.size > 4)
            l = 1;
         DLLIST_ADDHEAD(&lo[l], &nodes[i]);
      } else {
         DLLIST_ADDHEAD(&hi, &nodes[i]);
      }
   }
   if (prog->dbgFlags & NV50_IR_DEBUG_REG_ALLOC)
      printNodeInfo();
}

void
GCRA::simplifyEdge(RIG_Node *a, RIG_Node *b)
{
   bool move = b->degree >= b->degreeLimit;

   INFO_DBG(prog->dbgFlags, REG_ALLOC,
            "edge: (%%%i, deg %u/%u) >-< (%%%i, deg %u/%u)\n",
            a->getValue()->id, a->degree, a->degreeLimit,
            b->getValue()->id, b->degree, b->degreeLimit);

   b->degree -= relDegree[a->colors][b->colors];

   move = move && b->degree < b->degreeLimit;
   if (move && !DLLIST_EMPTY(b)) {
      int l = (b->getValue()->reg.size > 4) ? 1 : 0;
      DLLIST_DEL(b);
      DLLIST_ADDTAIL(&lo[l], b);
   }
}

void
GCRA::simplifyNode(RIG_Node *node)
{
   for (Graph::EdgeIterator ei = node->outgoing(); !ei.end(); ei.next())
      simplifyEdge(node, RIG_Node::get(ei));

   for (Graph::EdgeIterator ei = node->incident(); !ei.end(); ei.next())
      simplifyEdge(node, RIG_Node::get(ei));

   DLLIST_DEL(node);
   stack.push(node->getValue()->id);

   INFO_DBG(prog->dbgFlags, REG_ALLOC, "SIMPLIFY: pushed %%%i%s\n",
            node->getValue()->id,
            (node->degree < node->degreeLimit) ? "" : "(spill)");
}

bool
GCRA::simplify()
{
   for (;;) {
      if (!DLLIST_EMPTY(&lo[0])) {
         do {
            simplifyNode(lo[0].next);
         } while (!DLLIST_EMPTY(&lo[0]));
      } else
      if (!DLLIST_EMPTY(&lo[1])) {
         simplifyNode(lo[1].next);
      } else
      if (!DLLIST_EMPTY(&hi)) {
         RIG_Node *best = hi.next;
         unsigned bestMaxReg = best->maxReg;
         float bestScore = best->weight / (float)best->degree;
         // Spill candidate. First go through the ones with the highest max
         // register, then the ones with lower. That way the ones with the
         // lowest requirement will be allocated first, since it's a stack.
         for (RIG_Node *it = best->next; it != &hi; it = it->next) {
            float score = it->weight / (float)it->degree;
            if (score < bestScore || it->maxReg > bestMaxReg) {
               best = it;
               bestScore = score;
               bestMaxReg = it->maxReg;
            }
         }
         if (isinf(bestScore)) {
            ERROR("no viable spill candidates left\n");
            return false;
         }
         simplifyNode(best);
      } else {
         return true;
      }
   }
}

void
GCRA::checkInterference(const RIG_Node *node, Graph::EdgeIterator& ei)
{
   const RIG_Node *intf = RIG_Node::get(ei);

   if (intf->reg < 0)
      return;
   LValue *vA = node->getValue();
   LValue *vB = intf->getValue();

   const uint8_t intfMask = ((1 << intf->colors) - 1) << (intf->reg & 7);

   if (vA->compound | vB->compound) {
      // NOTE: this only works for >aligned< register tuples !
      for (const ValueDef *D : mergedDefs(vA)) {
      for (const ValueDef *d : mergedDefs(vB)) {
         const LValue *vD = D->get()->asLValue();
         const LValue *vd = d->get()->asLValue();

         if (!vD->livei.overlaps(vd->livei)) {
            INFO_DBG(prog->dbgFlags, REG_ALLOC, "(%%%i) X (%%%i): no overlap\n",
                     vD->id, vd->id);
            continue;
         }

         uint8_t mask = vD->compound ? vD->compMask : ~0;
         if (vd->compound) {
            assert(vB->compound);
            mask &= vd->compMask & vB->compMask;
         } else {
            mask &= intfMask;
         }

         INFO_DBG(prog->dbgFlags, REG_ALLOC,
                  "(%%%i)%02x X (%%%i)%02x & %02x: $r%i.%02x\n",
                  vD->id,
                  vD->compound ? vD->compMask : 0xff,
                  vd->id,
                  vd->compound ? vd->compMask : intfMask,
                  vB->compMask, intf->reg & ~7, mask);
         if (mask)
            regs.occupyMask(node->f, intf->reg & ~7, mask);
      }
      }
   } else {
      INFO_DBG(prog->dbgFlags, REG_ALLOC,
               "(%%%i) X (%%%i): $r%i + %u\n",
               vA->id, vB->id, intf->reg, intf->colors);
      regs.occupy(node->f, intf->reg, intf->colors);
   }
}

bool
GCRA::selectRegisters()
{
   INFO_DBG(prog->dbgFlags, REG_ALLOC, "\nSELECT phase\n");

   while (!stack.empty()) {
      RIG_Node *node = &nodes[stack.top()];
      stack.pop();

      regs.reset(node->f);

      INFO_DBG(prog->dbgFlags, REG_ALLOC, "\nNODE[%%%i, %u colors]\n",
               node->getValue()->id, node->colors);

      for (Graph::EdgeIterator ei = node->outgoing(); !ei.end(); ei.next())
         checkInterference(node, ei);
      for (Graph::EdgeIterator ei = node->incident(); !ei.end(); ei.next())
         checkInterference(node, ei);

      if (!node->prefRegs.empty()) {
         for (std::list<RIG_Node *>::const_iterator it = node->prefRegs.begin();
              it != node->prefRegs.end();
              ++it) {
            if ((*it)->reg >= 0 &&
                regs.testOccupy(node->f, (*it)->reg, node->colors)) {
               node->reg = (*it)->reg;
               break;
            }
         }
      }
      if (node->reg >= 0)
         continue;
      LValue *lval = node->getValue();
      if (prog->dbgFlags & NV50_IR_DEBUG_REG_ALLOC)
         regs.print(node->f);
      bool ret = regs.assign(node->reg, node->f, node->colors, node->maxReg);
      if (ret) {
         INFO_DBG(prog->dbgFlags, REG_ALLOC, "assigned reg %i\n", node->reg);
         lval->compMask = node->getCompMask();
      } else {
         INFO_DBG(prog->dbgFlags, REG_ALLOC, "must spill: %%%i (size %u)\n",
                  lval->id, lval->reg.size);
         Symbol *slot = NULL;
         if (lval->reg.file == FILE_GPR)
            slot = spill.assignSlot(node->livei, lval->reg.size);
         mustSpill.push_back(ValuePair(lval, slot));
      }
   }
   if (!mustSpill.empty())
      return false;
   for (unsigned int i = 0; i < nodeCount; ++i) {
      LValue *lval = nodes[i].getValue();
      if (nodes[i].reg >= 0 && nodes[i].colors > 0)
         lval->reg.data.id =
            regs.unitsToId(nodes[i].f, nodes[i].reg, lval->reg.size);
   }
   return true;
}

bool
GCRA::allocateRegisters(ArrayList& insns)
{
   bool ret;

   INFO_DBG(prog->dbgFlags, REG_ALLOC,
            "allocateRegisters to %u instructions\n", insns.getSize());

   nodeCount = func->allLValues.getSize();
   nodes = new RIG_Node[nodeCount];
   if (!nodes)
      return false;
   for (unsigned int i = 0; i < nodeCount; ++i) {
      LValue *lval = reinterpret_cast<LValue *>(func->allLValues.get(i));
      if (lval) {
         nodes[i].init(regs, lval);
         RIG.insert(&nodes[i]);

         if (lval->inFile(FILE_GPR) && lval->getInsn() != NULL) {
            Instruction *insn = lval->getInsn();
            if (insn->op != OP_MAD && insn->op != OP_FMA && insn->op != OP_SAD)
               continue;
            // For both of the cases below, we only want to add the preference
            // if all arguments are in registers.
            if (insn->src(0).getFile() != FILE_GPR ||
                insn->src(1).getFile() != FILE_GPR ||
                insn->src(2).getFile() != FILE_GPR)
               continue;
            if (prog->getTarget()->getChipset() < 0xc0) {
               // Outputting a flag is not supported with short encodings nor
               // with immediate arguments.
               // See handleMADforNV50.
               if (insn->flagsDef >= 0)
                  continue;
            } else {
               // We can only fold immediate arguments if dst == src2. This
               // only matters if one of the first two arguments is an
               // immediate. This form is also only supported for floats.
               // See handleMADforNVC0.
               ImmediateValue imm;
               if (insn->dType != TYPE_F32)
                  continue;
               if (!insn->src(0).getImmediate(imm) &&
                   !insn->src(1).getImmediate(imm))
                  continue;
            }

            nodes[i].addRegPreference(getNode(insn->getSrc(2)->asLValue()));
         }
      }
   }

   // coalesce first, we use only 1 RIG node for a group of joined values
   ret = coalesce(insns);
   if (!ret)
      goto out;

   if (func->getProgram()->dbgFlags & NV50_IR_DEBUG_REG_ALLOC)
      func->printLiveIntervals();

   buildRIG(insns);
   calculateSpillWeights();
   ret = simplify();
   if (!ret)
      goto out;

   ret = selectRegisters();
   if (!ret) {
      INFO_DBG(prog->dbgFlags, REG_ALLOC,
               "selectRegisters failed, inserting spill code ...\n");
      regs.reset(FILE_GPR, true);
      spill.run(mustSpill);
      if (prog->dbgFlags & NV50_IR_DEBUG_REG_ALLOC)
         func->print();
   } else {
      mergedDefs.merge();
      prog->maxGPR = std::max(prog->maxGPR, regs.getMaxAssigned(FILE_GPR));
   }

out:
   cleanup(ret);
   return ret;
}

void
GCRA::cleanup(const bool success)
{
   mustSpill.clear();

   for (ArrayList::Iterator it = func->allLValues.iterator();
        !it.end(); it.next()) {
      LValue *lval =  reinterpret_cast<LValue *>(it.get());

      lval->livei.clear();

      lval->compound = 0;
      lval->compMask = 0;

      if (lval->join == lval)
         continue;

      if (success)
         lval->reg.data.id = lval->join->reg.data.id;
      else
         lval->join = lval;
   }

   if (success)
      resolveSplitsAndMerges();
   splits.clear(); // avoid duplicate entries on next coalesce pass
   merges.clear();

   delete[] nodes;
   nodes = NULL;
   hi.next = hi.prev = &hi;
   lo[0].next = lo[0].prev = &lo[0];
   lo[1].next = lo[1].prev = &lo[1];
}

Symbol *
SpillCodeInserter::assignSlot(const Interval &livei, const unsigned int size)
{
   int32_t address = align(stackSize + func->tlsBase, size);

   Symbol *sym = new_Symbol(func->getProgram(), FILE_MEMORY_LOCAL);
   sym->setAddress(NULL, address);
   sym->reg.size = size;

   stackSize = address + size - func->tlsBase;

   return sym;
}

Value *
SpillCodeInserter::offsetSlot(Value *base, const LValue *lval)
{
   if (!lval->compound || (lval->compMask & 0x1))
      return base;
   Value *slot = cloneShallow(func, base);

   const unsigned int unit = func->getProgram()->getTarget()->getFileUnit(lval->reg.file);
   slot->reg.data.offset += (ffs(lval->compMask) - 1) << unit;
   assert((slot->reg.data.offset % lval->reg.size) == 0);
   slot->reg.size = lval->reg.size;

   return slot;
}

void
SpillCodeInserter::spill(Instruction *defi, Value *slot, LValue *lval)
{
   const DataType ty = typeOfSize(lval->reg.size);

   slot = offsetSlot(slot, lval);

   Instruction *st;
   if (slot->reg.file == FILE_MEMORY_LOCAL) {
      lval->noSpill = 1;
      if (ty != TYPE_B96) {
         st = new_Instruction(func, OP_STORE, ty);
         st->setSrc(0, slot);
         st->setSrc(1, lval);
      } else {
         st = new_Instruction(func, OP_SPLIT, ty);
         st->setSrc(0, lval);
         for (int d = 0; d < lval->reg.size / 4; ++d)
            st->setDef(d, new_LValue(func, FILE_GPR));

         for (int d = lval->reg.size / 4 - 1; d >= 0; --d) {
            Value *tmp = cloneShallow(func, slot);
            tmp->reg.size = 4;
            tmp->reg.data.offset += 4 * d;

            Instruction *s = new_Instruction(func, OP_STORE, TYPE_U32);
            s->setSrc(0, tmp);
            s->setSrc(1, st->getDef(d));
            defi->bb->insertAfter(defi, s);
         }
      }
   } else {
      st = new_Instruction(func, OP_CVT, ty);
      st->setDef(0, slot);
      st->setSrc(0, lval);
      if (lval->reg.file == FILE_FLAGS)
         st->flagsSrc = 0;
   }
   defi->bb->insertAfter(defi, st);
}

LValue *
SpillCodeInserter::unspill(Instruction *usei, LValue *lval, Value *slot)
{
   const DataType ty = typeOfSize(lval->reg.size);

   slot = offsetSlot(slot, lval);
   lval = cloneShallow(func, lval);

   Instruction *ld;
   if (slot->reg.file == FILE_MEMORY_LOCAL) {
      lval->noSpill = 1;
      if (ty != TYPE_B96) {
         ld = new_Instruction(func, OP_LOAD, ty);
      } else {
         ld = new_Instruction(func, OP_MERGE, ty);
         for (int d = 0; d < lval->reg.size / 4; ++d) {
            Value *tmp = cloneShallow(func, slot);
            LValue *val;
            tmp->reg.size = 4;
            tmp->reg.data.offset += 4 * d;

            Instruction *l = new_Instruction(func, OP_LOAD, TYPE_U32);
            l->setDef(0, (val = new_LValue(func, FILE_GPR)));
            l->setSrc(0, tmp);
            usei->bb->insertBefore(usei, l);
            ld->setSrc(d, val);
            val->noSpill = 1;
         }
         ld->setDef(0, lval);
         usei->bb->insertBefore(usei, ld);
         return lval;
      }
   } else {
      ld = new_Instruction(func, OP_CVT, ty);
   }
   ld->setDef(0, lval);
   ld->setSrc(0, slot);
   if (lval->reg.file == FILE_FLAGS)
      ld->flagsDef = 0;

   usei->bb->insertBefore(usei, ld);
   return lval;
}

static bool
value_cmp(ValueRef *a, ValueRef *b) {
   Instruction *ai = a->getInsn(), *bi = b->getInsn();
   if (ai->bb != bi->bb)
      return ai->bb->getId() < bi->bb->getId();
   return ai->serial < bi->serial;
}

// For each value that is to be spilled, go through all its definitions.
// A value can have multiple definitions if it has been coalesced before.
// For each definition, first go through all its uses and insert an unspill
// instruction before it, then replace the use with the temporary register.
// Unspill can be either a load from memory or simply a move to another
// register file.
// For "Pseudo" instructions (like PHI, SPLIT, MERGE) we can erase the use
// if we have spilled to a memory location, or simply with the new register.
// No load or conversion instruction should be needed.
bool
SpillCodeInserter::run(const std::list<ValuePair>& lst)
{
   for (std::list<ValuePair>::const_iterator it = lst.begin(); it != lst.end();
        ++it) {
      LValue *lval = it->first->asLValue();
      Symbol *mem = it->second ? it->second->asSym() : NULL;

      // Keep track of which instructions to delete later. Deleting them
      // inside the loop is unsafe since a single instruction may have
      // multiple destinations that all need to be spilled (like OP_SPLIT).
      std::unordered_set<Instruction *> to_del;

      std::list<ValueDef *> &defs = mergedDefs(lval);
      for (Value::DefIterator d = defs.begin(); d != defs.end();
           ++d) {
         Value *slot = mem ?
            static_cast<Value *>(mem) : new_LValue(func, FILE_GPR);
         Value *tmp = NULL;
         Instruction *last = NULL;

         LValue *dval = (*d)->get()->asLValue();
         Instruction *defi = (*d)->getInsn();

         // Sort all the uses by BB/instruction so that we don't unspill
         // multiple times in a row, and also remove a source of
         // non-determinism.
         std::vector<ValueRef *> refs(dval->uses.begin(), dval->uses.end());
         std::sort(refs.begin(), refs.end(), value_cmp);

         // Unspill at each use *before* inserting spill instructions,
         // we don't want to have the spill instructions in the use list here.
         for (std::vector<ValueRef*>::const_iterator it = refs.begin();
              it != refs.end(); ++it) {
            ValueRef *u = *it;
            Instruction *usei = u->getInsn();
            assert(usei);
            if (usei->isPseudo()) {
               tmp = (slot->reg.file == FILE_MEMORY_LOCAL) ? NULL : slot;
               last = NULL;
            } else {
               if (!last || (usei != last->next && usei != last))
                  tmp = unspill(usei, dval, slot);
               last = usei;
            }
            u->set(tmp);
         }

         assert(defi);
         if (defi->isPseudo()) {
            d = defs.erase(d);
            --d;
            if (slot->reg.file == FILE_MEMORY_LOCAL)
               to_del.insert(defi);
            else
               defi->setDef(0, slot);
         } else {
            spill(defi, slot, dval);
         }
      }

      for (std::unordered_set<Instruction *>::const_iterator it = to_del.begin();
           it != to_del.end(); ++it) {
         mergedDefs.removeDefsOfInstruction(*it);
         delete_Instruction(func->getProgram(), *it);
      }
   }

   stackBase = stackSize;
   return true;
}

bool
RegAlloc::exec()
{
   for (IteratorRef it = prog->calls.iteratorDFS(false);
        !it->end(); it->next()) {
      func = Function::get(reinterpret_cast<Graph::Node *>(it->get()));

      func->tlsBase = prog->tlsSize;
      if (!execFunc())
         return false;
      prog->tlsSize += func->tlsSize;
   }
   return true;
}

bool
RegAlloc::execFunc()
{
   MergedDefs mergedDefs;
   InsertConstraintsPass insertConstr;
   PhiMovesPass insertPhiMoves;
   BuildIntervalsPass buildIntervals;
   SpillCodeInserter insertSpills(func, mergedDefs);

   GCRA gcra(func, insertSpills, mergedDefs);

   unsigned int i, retries;
   bool ret;

   if (!func->ins.empty()) {
      // Insert a nop at the entry so inputs only used by the first instruction
      // don't count as having an empty live range.
      Instruction *nop = new_Instruction(func, OP_NOP, TYPE_NONE);
      BasicBlock::get(func->cfg.getRoot())->insertHead(nop);
   }

   ret = insertConstr.exec(func);
   if (!ret)
      goto out;

   ret = insertPhiMoves.run(func);
   if (!ret)
      goto out;

   // TODO: need to fix up spill slot usage ranges to support > 1 retry
   for (retries = 0; retries < 3; ++retries) {
      if (retries && (prog->dbgFlags & NV50_IR_DEBUG_REG_ALLOC))
         INFO("Retry: %i\n", retries);
      if (prog->dbgFlags & NV50_IR_DEBUG_REG_ALLOC)
         func->print();

      // spilling to registers may add live ranges, need to rebuild everything
      ret = true;
      for (sequence = func->cfg.nextSequence(), i = 0;
           ret && i <= func->loopNestingBound;
           sequence = func->cfg.nextSequence(), ++i)
         ret = buildLiveSets(BasicBlock::get(func->cfg.getRoot()));
      // reset marker
      for (ArrayList::Iterator bi = func->allBBlocks.iterator();
           !bi.end(); bi.next())
         BasicBlock::get(bi)->liveSet.marker = false;
      if (!ret)
         break;
      func->orderInstructions(this->insns);

      ret = buildIntervals.run(func);
      if (!ret)
         break;
      ret = gcra.allocateRegisters(insns);
      if (ret)
         break; // success
   }
   INFO_DBG(prog->dbgFlags, REG_ALLOC, "RegAlloc done: %i\n", ret);

   func->tlsSize = insertSpills.getStackSize();
out:
   return ret;
}

// TODO: check if modifying Instruction::join here breaks anything
void
GCRA::resolveSplitsAndMerges()
{
   for (std::list<Instruction *>::iterator it = splits.begin();
        it != splits.end();
        ++it) {
      Instruction *split = *it;
      unsigned int reg = regs.idToBytes(split->getSrc(0));
      for (int d = 0; split->defExists(d); ++d) {
         Value *v = split->getDef(d);
         v->reg.data.id = regs.bytesToId(v, reg);
         v->join = v;
         reg += v->reg.size;
      }
   }
   splits.clear();

   for (std::list<Instruction *>::iterator it = merges.begin();
        it != merges.end();
        ++it) {
      Instruction *merge = *it;
      unsigned int reg = regs.idToBytes(merge->getDef(0));
      for (int s = 0; merge->srcExists(s); ++s) {
         Value *v = merge->getSrc(s);
         v->reg.data.id = regs.bytesToId(v, reg);
         v->join = v;
         // If the value is defined by a phi/union node, we also need to
         // perform the same fixup on that node's sources, since after RA
         // their registers should be identical.
         if (v->getInsn()->op == OP_PHI || v->getInsn()->op == OP_UNION) {
            Instruction *phi = v->getInsn();
            for (int phis = 0; phi->srcExists(phis); ++phis) {
               phi->getSrc(phis)->join = v;
               phi->getSrc(phis)->reg.data.id = v->reg.data.id;
            }
         }
         reg += v->reg.size;
      }
   }
   merges.clear();
}

bool
RegAlloc::InsertConstraintsPass::exec(Function *ir)
{
   constrList.clear();

   bool ret = run(ir, true, true);
   if (ret)
      ret = insertConstraintMoves();
   return ret;
}

// TODO: make part of texture insn
void
RegAlloc::InsertConstraintsPass::textureMask(TexInstruction *tex)
{
   Value *def[4];
   int c, k, d;
   uint8_t mask = 0;

   for (d = 0, k = 0, c = 0; c < 4; ++c) {
      if (!(tex->tex.mask & (1 << c)))
         continue;
      if (tex->getDef(k)->refCount()) {
         mask |= 1 << c;
         def[d++] = tex->getDef(k);
      }
      ++k;
   }
   tex->tex.mask = mask;

   for (c = 0; c < d; ++c)
      tex->setDef(c, def[c]);
   for (; c < 4; ++c)
      tex->setDef(c, NULL);
}

// Add a dummy use of the pointer source of >= 8 byte loads after the load
// to prevent it from being assigned a register which overlapping the load's
// destination, which would produce random corruptions.
void
RegAlloc::InsertConstraintsPass::addHazard(Instruction *i, const ValueRef *src)
{
   Instruction *hzd = new_Instruction(func, OP_NOP, TYPE_NONE);
   hzd->setSrc(0, src->get());
   i->bb->insertAfter(i, hzd);

}

// b32 { %r0 %r1 %r2 %r3 } -> b128 %r0q
void
RegAlloc::InsertConstraintsPass::condenseDefs(Instruction *insn)
{
   int n;
   for (n = 0; insn->defExists(n) && insn->def(n).getFile() == FILE_GPR; ++n);
   condenseDefs(insn, 0, n - 1);
}

void
RegAlloc::InsertConstraintsPass::condenseDefs(Instruction *insn,
                                              const int a, const int b)
{
   uint8_t size = 0;
   if (a >= b)
      return;
   for (int s = a; s <= b; ++s)
      size += insn->getDef(s)->reg.size;
   if (!size)
      return;

   LValue *lval = new_LValue(func, FILE_GPR);
   lval->reg.size = size;

   Instruction *split = new_Instruction(func, OP_SPLIT, typeOfSize(size));
   split->setSrc(0, lval);
   for (int d = a; d <= b; ++d) {
      split->setDef(d - a, insn->getDef(d));
      insn->setDef(d, NULL);
   }
   insn->setDef(a, lval);

   for (int k = a + 1, d = b + 1; insn->defExists(d); ++d, ++k) {
      insn->setDef(k, insn->getDef(d));
      insn->setDef(d, NULL);
   }
   // carry over predicate if any (mainly for OP_UNION uses)
   split->setPredicate(insn->cc, insn->getPredicate());

   insn->bb->insertAfter(insn, split);
   constrList.push_back(split);
}

void
RegAlloc::InsertConstraintsPass::condenseSrcs(Instruction *insn,
                                              const int a, const int b)
{
   uint8_t size = 0;
   if (a >= b)
      return;
   for (int s = a; s <= b; ++s)
      size += insn->getSrc(s)->reg.size;
   if (!size)
      return;
   LValue *lval = new_LValue(func, FILE_GPR);
   lval->reg.size = size;

   Value *save[3];
   insn->takeExtraSources(0, save);

   Instruction *merge = new_Instruction(func, OP_MERGE, typeOfSize(size));
   merge->setDef(0, lval);
   for (int s = a, i = 0; s <= b; ++s, ++i) {
      merge->setSrc(i, insn->getSrc(s));
   }
   insn->moveSources(b + 1, a - b);
   insn->setSrc(a, lval);
   insn->bb->insertBefore(insn, merge);

   insn->putExtraSources(0, save);

   constrList.push_back(merge);
}

bool
RegAlloc::InsertConstraintsPass::isScalarTexGM107(TexInstruction *tex)
{
   if (tex->tex.sIndirectSrc >= 0 ||
       tex->tex.rIndirectSrc >= 0 ||
       tex->tex.derivAll)
      return false;

   if (tex->tex.mask == 5 || tex->tex.mask == 6)
      return false;

   switch (tex->op) {
   case OP_TEX:
   case OP_TXF:
   case OP_TXG:
   case OP_TXL:
      break;
   default:
      return false;
   }

   // legal variants:
   // TEXS.1D.LZ
   // TEXS.2D
   // TEXS.2D.LZ
   // TEXS.2D.LL
   // TEXS.2D.DC
   // TEXS.2D.LL.DC
   // TEXS.2D.LZ.DC
   // TEXS.A2D
   // TEXS.A2D.LZ
   // TEXS.A2D.LZ.DC
   // TEXS.3D
   // TEXS.3D.LZ
   // TEXS.CUBE
   // TEXS.CUBE.LL

   // TLDS.1D.LZ
   // TLDS.1D.LL
   // TLDS.2D.LZ
   // TLSD.2D.LZ.AOFFI
   // TLDS.2D.LZ.MZ
   // TLDS.2D.LL
   // TLDS.2D.LL.AOFFI
   // TLDS.A2D.LZ
   // TLDS.3D.LZ

   // TLD4S: all 2D/RECT variants and only offset

   switch (tex->op) {
   case OP_TEX:
      if (tex->tex.useOffsets)
         return false;

      switch (tex->tex.target.getEnum()) {
      case TEX_TARGET_1D:
      case TEX_TARGET_2D_ARRAY_SHADOW:
         return tex->tex.levelZero;
      case TEX_TARGET_CUBE:
         return !tex->tex.levelZero;
      case TEX_TARGET_2D:
      case TEX_TARGET_2D_ARRAY:
      case TEX_TARGET_2D_SHADOW:
      case TEX_TARGET_3D:
      case TEX_TARGET_RECT:
      case TEX_TARGET_RECT_SHADOW:
         return true;
      default:
         return false;
      }

   case OP_TXL:
      if (tex->tex.useOffsets)
         return false;

      switch (tex->tex.target.getEnum()) {
      case TEX_TARGET_2D:
      case TEX_TARGET_2D_SHADOW:
      case TEX_TARGET_RECT:
      case TEX_TARGET_RECT_SHADOW:
      case TEX_TARGET_CUBE:
         return true;
      default:
         return false;
      }

   case OP_TXF:
      switch (tex->tex.target.getEnum()) {
      case TEX_TARGET_1D:
         return !tex->tex.useOffsets;
      case TEX_TARGET_2D:
      case TEX_TARGET_RECT:
         return true;
      case TEX_TARGET_2D_ARRAY:
      case TEX_TARGET_2D_MS:
      case TEX_TARGET_3D:
         return !tex->tex.useOffsets && tex->tex.levelZero;
      default:
         return false;
      }

   case OP_TXG:
      if (tex->tex.useOffsets > 1)
         return false;
      if (tex->tex.mask != 0x3 && tex->tex.mask != 0xf)
         return false;

      switch (tex->tex.target.getEnum()) {
      case TEX_TARGET_2D:
      case TEX_TARGET_2D_MS:
      case TEX_TARGET_2D_SHADOW:
      case TEX_TARGET_RECT:
      case TEX_TARGET_RECT_SHADOW:
         return true;
      default:
         return false;
      }

   default:
      return false;
   }
}

void
RegAlloc::InsertConstraintsPass::handleScalarTexGM107(TexInstruction *tex)
{
   int defCount = tex->defCount(0xff);
   int srcCount = tex->srcCount(0xff);

   tex->tex.scalar = true;

   // 1. handle defs
   if (defCount > 3)
      condenseDefs(tex, 2, 3);
   if (defCount > 1)
      condenseDefs(tex, 0, 1);

   // 2. handle srcs
   // special case for TXF.A2D
   if (tex->op == OP_TXF && tex->tex.target == TEX_TARGET_2D_ARRAY) {
      assert(srcCount >= 3);
      condenseSrcs(tex, 1, 2);
   } else {
      if (srcCount > 3)
         condenseSrcs(tex, 2, 3);
      // only if we have more than 2 sources
      if (srcCount > 2)
         condenseSrcs(tex, 0, 1);
   }

   assert(!tex->defExists(2) && !tex->srcExists(2));
}

void
RegAlloc::InsertConstraintsPass::texConstraintGM107(TexInstruction *tex)
{
   int n, s;

   if (isTextureOp(tex->op))
      textureMask(tex);

   if (targ->getChipset() < NVISA_GV100_CHIPSET) {
      if (isScalarTexGM107(tex)) {
         handleScalarTexGM107(tex);
         return;
      }

      assert(!tex->tex.scalar);
      condenseDefs(tex);
   } else {
      if (isTextureOp(tex->op)) {
         int defCount = tex->defCount(0xff);
         if (defCount > 3)
            condenseDefs(tex, 2, 3);
         if (defCount > 1)
            condenseDefs(tex, 0, 1);
      } else {
         condenseDefs(tex);
      }
   }

   if (isSurfaceOp(tex->op)) {
      int s = tex->tex.target.getDim() +
         (tex->tex.target.isArray() || tex->tex.target.isCube());
      int n = 0;

      switch (tex->op) {
      case OP_SUSTB:
      case OP_SUSTP:
         n = 4;
         break;
      case OP_SUREDB:
      case OP_SUREDP:
         if (tex->subOp == NV50_IR_SUBOP_ATOM_CAS)
            n = 2;
         break;
      default:
         break;
      }

      if (s > 1)
         condenseSrcs(tex, 0, s - 1);
      if (n > 1)
         condenseSrcs(tex, 1, n); // do not condense the tex handle
   } else
   if (isTextureOp(tex->op)) {
      if (tex->op != OP_TXQ) {
         s = tex->tex.target.getArgCount() - tex->tex.target.isMS();
         if (tex->op == OP_TXD) {
            // Indirect handle belongs in the first arg
            if (tex->tex.rIndirectSrc >= 0)
               s++;
            if (!tex->tex.target.isArray() && tex->tex.useOffsets)
               s++;
         }
         n = tex->srcCount(0xff, true) - s;
         // TODO: Is this necessary? Perhaps just has to be aligned to the
         // level that the first arg is, not necessarily to 4. This
         // requirement has not been rigorously verified, as it has been on
         // Kepler.
         if (n > 0 && n < 3) {
            if (tex->srcExists(n + s)) // move potential predicate out of the way
               tex->moveSources(n + s, 3 - n);
            while (n < 3)
               tex->setSrc(s + n++, new_LValue(func, FILE_GPR));
         }
      } else {
         s = tex->srcCount(0xff, true);
         n = 0;
      }

      if (s > 1)
         condenseSrcs(tex, 0, s - 1);
      if (n > 1) // NOTE: first call modified positions already
         condenseSrcs(tex, 1, n);
   }
}

void
RegAlloc::InsertConstraintsPass::texConstraintNVE0(TexInstruction *tex)
{
   if (isTextureOp(tex->op))
      textureMask(tex);
   condenseDefs(tex);

   if (tex->op == OP_SUSTB || tex->op == OP_SUSTP) {
      condenseSrcs(tex, 3, 6);
   } else
   if (isTextureOp(tex->op)) {
      int n = tex->srcCount(0xff, true);
      int s = n > 4 ? 4 : n;
      if (n > 4 && n < 7) {
         if (tex->srcExists(n)) // move potential predicate out of the way
            tex->moveSources(n, 7 - n);

         while (n < 7)
            tex->setSrc(n++, new_LValue(func, FILE_GPR));
      }
      if (s > 1)
         condenseSrcs(tex, 0, s - 1);
      if (n > 4)
         condenseSrcs(tex, 1, n - s);
   }
}

void
RegAlloc::InsertConstraintsPass::texConstraintNVC0(TexInstruction *tex)
{
   int n, s;

   if (isTextureOp(tex->op))
      textureMask(tex);

   if (tex->op == OP_TXQ) {
      s = tex->srcCount(0xff);
      n = 0;
   } else if (isSurfaceOp(tex->op)) {
      s = tex->tex.target.getDim() + (tex->tex.target.isArray() || tex->tex.target.isCube());
      if (tex->op == OP_SUSTB || tex->op == OP_SUSTP)
         n = 4;
      else
         n = 0;
   } else {
      s = tex->tex.target.getArgCount() - tex->tex.target.isMS();
      if (!tex->tex.target.isArray() &&
          (tex->tex.rIndirectSrc >= 0 || tex->tex.sIndirectSrc >= 0))
         ++s;
      if (tex->op == OP_TXD && tex->tex.useOffsets)
         ++s;
      n = tex->srcCount(0xff) - s;
      assert(n <= 4);
   }

   if (s > 1)
      condenseSrcs(tex, 0, s - 1);
   if (n > 1) // NOTE: first call modified positions already
      condenseSrcs(tex, 1, n);

   condenseDefs(tex);
}

void
RegAlloc::InsertConstraintsPass::texConstraintNV50(TexInstruction *tex)
{
   Value *pred = tex->getPredicate();
   if (pred)
      tex->setPredicate(tex->cc, NULL);

   textureMask(tex);

   assert(tex->defExists(0) && tex->srcExists(0));
   // make src and def count match
   int c;
   for (c = 0; tex->srcExists(c) || tex->defExists(c); ++c) {
      if (!tex->srcExists(c))
         tex->setSrc(c, new_LValue(func, tex->getSrc(0)->asLValue()));
      else
         insertConstraintMove(tex, c);
      if (!tex->defExists(c))
         tex->setDef(c, new_LValue(func, tex->getDef(0)->asLValue()));
   }
   if (pred)
      tex->setPredicate(tex->cc, pred);
   condenseDefs(tex);
   condenseSrcs(tex, 0, c - 1);
}

// Insert constraint markers for instructions whose multiple sources must be
// located in consecutive registers.
bool
RegAlloc::InsertConstraintsPass::visit(BasicBlock *bb)
{
   TexInstruction *tex;
   Instruction *next;
   int s, size;

   targ = bb->getProgram()->getTarget();

   for (Instruction *i = bb->getEntry(); i; i = next) {
      next = i->next;

      if ((tex = i->asTex())) {
         switch (targ->getChipset() & ~0xf) {
         case 0x50:
         case 0x80:
         case 0x90:
         case 0xa0:
            texConstraintNV50(tex);
            break;
         case 0xc0:
         case 0xd0:
            texConstraintNVC0(tex);
            break;
         case 0xe0:
         case 0xf0:
         case 0x100:
            texConstraintNVE0(tex);
            break;
         case 0x110:
         case 0x120:
         case 0x130:
         case 0x140:
         case 0x160:
         case 0x170:
         case 0x190:
            texConstraintGM107(tex);
            break;
         default:
            break;
         }
      } else
      if (i->op == OP_EXPORT || i->op == OP_STORE) {
         for (size = typeSizeof(i->dType), s = 1; size > 0; ++s) {
            assert(i->srcExists(s));
            size -= i->getSrc(s)->reg.size;
         }
         condenseSrcs(i, 1, s - 1);
      } else
      if (i->op == OP_LOAD || i->op == OP_VFETCH) {
         condenseDefs(i);
         if (i->src(0).isIndirect(0) && typeSizeof(i->dType) >= 8)
            addHazard(i, i->src(0).getIndirect(0));
         if (i->src(0).isIndirect(1) && typeSizeof(i->dType) >= 8)
            addHazard(i, i->src(0).getIndirect(1));
         if (i->op == OP_LOAD && i->fixed && targ->getChipset() < 0xc0) {
            // Add a hazard to make sure we keep the op around. These are used
            // for membars.
            Instruction *nop = new_Instruction(func, OP_NOP, i->dType);
            nop->setSrc(0, i->getDef(0));
            i->bb->insertAfter(i, nop);
         }
      } else
      if (i->op == OP_UNION ||
          i->op == OP_MERGE ||
          i->op == OP_SPLIT) {
         constrList.push_back(i);
      } else
      if (i->op == OP_ATOM && i->subOp == NV50_IR_SUBOP_ATOM_CAS &&
          targ->getChipset() < 0xc0) {
         // Like a hazard, but for a def.
         Instruction *nop = new_Instruction(func, OP_NOP, i->dType);
         nop->setSrc(0, i->getDef(0));
         i->bb->insertAfter(i, nop);
      }
   }
   return true;
}

void
RegAlloc::InsertConstraintsPass::insertConstraintMove(Instruction *cst, int s)
{
   const uint8_t size = cst->src(s).getSize();

   assert(cst->getSrc(s)->defs.size() == 1); // still SSA

   Instruction *defi = cst->getSrc(s)->defs.front()->getInsn();

   bool imm = defi->op == OP_MOV &&
      defi->src(0).getFile() == FILE_IMMEDIATE;
   bool load = defi->op == OP_LOAD &&
      defi->src(0).getFile() == FILE_MEMORY_CONST &&
      !defi->src(0).isIndirect(0);
   // catch some cases where don't really need MOVs
   if (cst->getSrc(s)->refCount() == 1 && !defi->constrainedDefs()
       && defi->op != OP_MERGE && defi->op != OP_SPLIT) {
      if (imm || load) {
         // Move the defi right before the cst. No point in expanding
         // the range.
         defi->bb->remove(defi);
         cst->bb->insertBefore(cst, defi);
      }
      return;
   }

   LValue *lval = new_LValue(func, cst->src(s).getFile());
   lval->reg.size = size;

   Instruction *mov = new_Instruction(func, OP_MOV, typeOfSize(size));
   mov->setDef(0, lval);
   mov->setSrc(0, cst->getSrc(s));

   if (load) {
      mov->op = OP_LOAD;
      mov->setSrc(0, defi->getSrc(0));
   } else if (imm) {
      mov->setSrc(0, defi->getSrc(0));
   }

   if (defi->getPredicate())
      mov->setPredicate(defi->cc, defi->getPredicate());

   cst->setSrc(s, mov->getDef(0));
   cst->bb->insertBefore(cst, mov);

   cst->getDef(0)->asLValue()->noSpill = 1; // doesn't help
}

// Insert extra moves so that, if multiple register constraints on a value are
// in conflict, these conflicts can be resolved.
bool
RegAlloc::InsertConstraintsPass::insertConstraintMoves()
{
   for (std::list<Instruction *>::iterator it = constrList.begin();
        it != constrList.end();
        ++it) {
      Instruction *cst = *it;
      Instruction *mov;

      if (cst->op == OP_SPLIT && false) {
         // spilling splits is annoying, just make sure they're separate
         for (int d = 0; cst->defExists(d); ++d) {
            if (!cst->getDef(d)->refCount())
               continue;
            LValue *lval = new_LValue(func, cst->def(d).getFile());
            const uint8_t size = cst->def(d).getSize();
            lval->reg.size = size;

            mov = new_Instruction(func, OP_MOV, typeOfSize(size));
            mov->setSrc(0, lval);
            mov->setDef(0, cst->getDef(d));
            cst->setDef(d, mov->getSrc(0));
            cst->bb->insertAfter(cst, mov);

            cst->getSrc(0)->asLValue()->noSpill = 1;
            mov->getSrc(0)->asLValue()->noSpill = 1;
         }
      } else
      if (cst->op == OP_MERGE || cst->op == OP_UNION) {
         for (int s = 0; cst->srcExists(s); ++s) {
            const uint8_t size = cst->src(s).getSize();

            if (!cst->getSrc(s)->defs.size()) {
               mov = new_Instruction(func, OP_NOP, typeOfSize(size));
               mov->setDef(0, cst->getSrc(s));
               cst->bb->insertBefore(cst, mov);
               continue;
            }

            insertConstraintMove(cst, s);
         }
      }
   }

   return true;
}

} // anonymous namespace

bool Program::registerAllocation()
{
   RegAlloc ra(this);
   return ra.exec();
}

} // namespace nv50_ir
