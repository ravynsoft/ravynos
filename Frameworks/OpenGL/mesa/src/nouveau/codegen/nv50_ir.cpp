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
#include "nv50_ir_driver.h"

namespace nv50_ir {

Modifier::Modifier(operation op)
{
   switch (op) {
   case OP_NEG: bits = NV50_IR_MOD_NEG; break;
   case OP_ABS: bits = NV50_IR_MOD_ABS; break;
   case OP_SAT: bits = NV50_IR_MOD_SAT; break;
   case OP_NOT: bits = NV50_IR_MOD_NOT; break;
   default:
      bits = 0;
      break;
   }
}

Modifier Modifier::operator*(const Modifier m) const
{
   unsigned int a, b, c;

   b = m.bits;
   if (this->bits & NV50_IR_MOD_ABS)
      b &= ~NV50_IR_MOD_NEG;

   a = (this->bits ^ b)      & (NV50_IR_MOD_NOT | NV50_IR_MOD_NEG);
   c = (this->bits | m.bits) & (NV50_IR_MOD_ABS | NV50_IR_MOD_SAT);

   return Modifier(a | c);
}

ValueRef::ValueRef(Value *v) : value(NULL), insn(NULL)
{
   indirect[0] = -1;
   indirect[1] = -1;
   usedAsPtr = false;
   set(v);
}

ValueRef::ValueRef(const ValueRef& ref) : value(NULL), insn(ref.insn)
{
   set(ref);
   usedAsPtr = ref.usedAsPtr;
}

ValueRef::~ValueRef()
{
   this->set(NULL);
}

bool ValueRef::getImmediate(ImmediateValue &imm) const
{
   const ValueRef *src = this;
   Modifier m;
   DataType type = src->insn->sType;

   while (src) {
      if (src->mod) {
         if (src->insn->sType != type)
            break;
         m *= src->mod;
      }
      if (src->getFile() == FILE_IMMEDIATE) {
         imm = *(src->value->asImm());
         // The immediate's type isn't required to match its use, it's
         // more of a hint; applying a modifier makes use of that hint.
         imm.reg.type = type;
         m.applyTo(imm);
         return true;
      }

      Instruction *insn = src->value->getUniqueInsn();

      if (insn && insn->op == OP_MOV) {
         src = &insn->src(0);
         if (src->mod)
            WARN("OP_MOV with modifier encountered !\n");
      } else {
         src = NULL;
      }
   }
   return false;
}

ValueDef::ValueDef(Value *v) : value(NULL), origin(NULL), insn(NULL)
{
   set(v);
}

ValueDef::ValueDef(const ValueDef& def) : value(NULL), origin(NULL), insn(NULL)
{
   set(def.get());
}

ValueDef::~ValueDef()
{
   this->set(NULL);
}

void
ValueRef::set(const ValueRef &ref)
{
   this->set(ref.get());
   mod = ref.mod;
   indirect[0] = ref.indirect[0];
   indirect[1] = ref.indirect[1];
}

void
ValueRef::set(Value *refVal)
{
   if (value == refVal)
      return;
   if (value)
      value->uses.erase(this);
   if (refVal)
      refVal->uses.insert(this);

   value = refVal;
}

void
ValueDef::set(Value *defVal)
{
   if (value == defVal)
      return;
   if (value)
      value->defs.remove(this);
   if (defVal)
      defVal->defs.push_back(this);

   value = defVal;
}

// Check if we can replace this definition's value by the value in @rep,
// including the source modifiers, i.e. make sure that all uses support
// @rep.mod.
bool
ValueDef::mayReplace(const ValueRef &rep)
{
   if (!rep.mod)
      return true;

   if (!insn || !insn->bb) // Unbound instruction ?
      return false;

   const Target *target = insn->bb->getProgram()->getTarget();

   for (Value::UseIterator it = value->uses.begin(); it != value->uses.end();
        ++it) {
      Instruction *insn = (*it)->getInsn();
      int s = -1;

      for (int i = 0; insn->srcExists(i); ++i) {
         if (insn->src(i).get() == value) {
            // If there are multiple references to us we'd have to check if the
            // combination of mods is still supported, but just bail for now.
            if (&insn->src(i) != (*it))
               return false;
            s = i;
         }
      }
      assert(s >= 0); // integrity of uses list

      if (!target->isModSupported(insn, s, rep.mod))
         return false;
   }
   return true;
}

void
ValueDef::replace(const ValueRef &repVal, bool doSet)
{
   assert(mayReplace(repVal));

   if (value == repVal.get())
      return;

   while (!value->uses.empty()) {
      ValueRef *ref = *value->uses.begin();
      ref->set(repVal.get());
      ref->mod *= repVal.mod;
   }

   if (doSet)
      set(repVal.get());
}

Value::Value() : id(-1)
{
  join = this;
  memset(&reg, 0, sizeof(reg));
  reg.size = 4;
}

LValue::LValue(Function *fn, DataFile file)
{
   reg.file = file;
   reg.size = (file != FILE_PREDICATE) ? 4 : 1;
   reg.data.id = -1;

   compMask = 0;
   compound = 0;
   ssa = 0;
   fixedReg = 0;
   noSpill = 0;

   fn->add(this, this->id);
}

LValue::LValue(Function *fn, LValue *lval)
{
   assert(lval);

   reg.file = lval->reg.file;
   reg.size = lval->reg.size;
   reg.data.id = -1;

   compMask = 0;
   compound = 0;
   ssa = 0;
   fixedReg = 0;
   noSpill = 0;

   fn->add(this, this->id);
}

LValue *
LValue::clone(ClonePolicy<Function>& pol) const
{
   LValue *that = new_LValue(pol.context(), reg.file);

   pol.set<Value>(this, that);

   that->reg.size = this->reg.size;
   that->reg.type = this->reg.type;
   that->reg.data = this->reg.data;

   return that;
}

bool
LValue::isUniform() const
{
   if (defs.size() > 1)
      return false;
   Instruction *insn = getInsn();
   if (!insn)
      return false;
   // let's not try too hard here for now ...
   return !insn->srcExists(1) && insn->getSrc(0)->isUniform();
}

Symbol::Symbol(Program *prog, DataFile f, uint8_t fidx)
{
   baseSym = NULL;

   reg.file = f;
   reg.fileIndex = fidx;
   reg.data.offset = 0;

   prog->add(this, this->id);
}

Symbol *
Symbol::clone(ClonePolicy<Function>& pol) const
{
   Program *prog = pol.context()->getProgram();

   Symbol *that = new_Symbol(prog, reg.file, reg.fileIndex);

   pol.set<Value>(this, that);

   that->reg.size = this->reg.size;
   that->reg.type = this->reg.type;
   that->reg.data = this->reg.data;

   that->baseSym = this->baseSym;

   return that;
}

bool
Symbol::isUniform() const
{
   return
      reg.file != FILE_SYSTEM_VALUE &&
      reg.file != FILE_MEMORY_LOCAL &&
      reg.file != FILE_SHADER_INPUT;
}

ImmediateValue::ImmediateValue(Program *prog, uint32_t uval)
{
   memset(&reg, 0, sizeof(reg));

   reg.file = FILE_IMMEDIATE;
   reg.size = 4;
   reg.type = TYPE_U32;

   reg.data.u32 = uval;

   prog->add(this, this->id);
}

ImmediateValue::ImmediateValue(Program *prog, float fval)
{
   memset(&reg, 0, sizeof(reg));

   reg.file = FILE_IMMEDIATE;
   reg.size = 4;
   reg.type = TYPE_F32;

   reg.data.f32 = fval;

   prog->add(this, this->id);
}

ImmediateValue::ImmediateValue(Program *prog, double dval)
{
   memset(&reg, 0, sizeof(reg));

   reg.file = FILE_IMMEDIATE;
   reg.size = 8;
   reg.type = TYPE_F64;

   reg.data.f64 = dval;

   prog->add(this, this->id);
}

ImmediateValue::ImmediateValue(const ImmediateValue *proto, DataType ty)
{
   reg = proto->reg;

   reg.type = ty;
   reg.size = typeSizeof(ty);
}

ImmediateValue *
ImmediateValue::clone(ClonePolicy<Function>& pol) const
{
   Program *prog = pol.context()->getProgram();
   ImmediateValue *that = new_ImmediateValue(prog, 0u);

   pol.set<Value>(this, that);

   that->reg.size = this->reg.size;
   that->reg.type = this->reg.type;
   that->reg.data = this->reg.data;

   return that;
}

bool
ImmediateValue::isInteger(const int i) const
{
   switch (reg.type) {
   case TYPE_S8:
      return reg.data.s8 == i;
   case TYPE_U8:
      return reg.data.u8 == i;
   case TYPE_S16:
      return reg.data.s16 == i;
   case TYPE_U16:
      return reg.data.u16 == i;
   case TYPE_S32:
   case TYPE_U32:
      return reg.data.s32 == i; // as if ...
   case TYPE_S64:
   case TYPE_U64:
      return reg.data.s64 == i; // as if ...
   case TYPE_F32:
      return reg.data.f32 == static_cast<float>(i);
   case TYPE_F64:
      return reg.data.f64 == static_cast<double>(i);
   default:
      return false;
   }
}

bool
ImmediateValue::isNegative() const
{
   switch (reg.type) {
   case TYPE_S8:  return reg.data.s8 < 0;
   case TYPE_S16: return reg.data.s16 < 0;
   case TYPE_S32:
   case TYPE_U32: return reg.data.s32 < 0;
   case TYPE_F32: return reg.data.u32 & (1 << 31);
   case TYPE_F64: return reg.data.u64 & (1ULL << 63);
   default:
      return false;
   }
}

bool
ImmediateValue::isPow2() const
{
   if (reg.type == TYPE_U64 || reg.type == TYPE_S64)
      return util_is_power_of_two_or_zero64(reg.data.u64);
   else
      return util_is_power_of_two_or_zero(reg.data.u32);
}

void
ImmediateValue::applyLog2()
{
   switch (reg.type) {
   case TYPE_S8:
   case TYPE_S16:
   case TYPE_S32:
      assert(!this->isNegative());
      FALLTHROUGH;
   case TYPE_U8:
   case TYPE_U16:
   case TYPE_U32:
      reg.data.u32 = util_logbase2(reg.data.u32);
      break;
   case TYPE_S64:
      assert(!this->isNegative());
      FALLTHROUGH;
   case TYPE_U64:
      reg.data.u64 = util_logbase2_64(reg.data.u64);
      break;
   case TYPE_F32:
      reg.data.f32 = log2f(reg.data.f32);
      break;
   case TYPE_F64:
      reg.data.f64 = log2(reg.data.f64);
      break;
   default:
      assert(0);
      break;
   }
}

bool
ImmediateValue::compare(CondCode cc, float fval) const
{
   if (reg.type != TYPE_F32)
      ERROR("immediate value is not of type f32");

   switch (static_cast<CondCode>(cc & 7)) {
   case CC_TR: return true;
   case CC_FL: return false;
   case CC_LT: return reg.data.f32 <  fval;
   case CC_LE: return reg.data.f32 <= fval;
   case CC_GT: return reg.data.f32 >  fval;
   case CC_GE: return reg.data.f32 >= fval;
   case CC_EQ: return reg.data.f32 == fval;
   case CC_NE: return reg.data.f32 != fval;
   default:
      assert(0);
      return false;
   }
}

ImmediateValue&
ImmediateValue::operator=(const ImmediateValue &that)
{
   this->reg = that.reg;
   return (*this);
}

bool
Value::interfers(const Value *that) const
{
   uint32_t idA, idB;

   if (that->reg.file != reg.file || that->reg.fileIndex != reg.fileIndex)
      return false;
   if (this->asImm())
      return false;

   if (this->asSym()) {
      idA = this->join->reg.data.offset;
      idB = that->join->reg.data.offset;
   } else {
      idA = this->join->reg.data.id * MIN2(this->reg.size, 4);
      idB = that->join->reg.data.id * MIN2(that->reg.size, 4);
   }

   if (idA < idB)
      return (idA + this->reg.size > idB);
   else
   if (idA > idB)
      return (idB + that->reg.size > idA);
   else
      return (idA == idB);
}

bool
Value::equals(const Value *that, bool strict) const
{
   if (strict)
      return this == that;

   if (that->reg.file != reg.file || that->reg.fileIndex != reg.fileIndex)
      return false;
   if (that->reg.size != this->reg.size)
      return false;

   if (that->reg.data.id != this->reg.data.id)
      return false;

   return true;
}

bool
ImmediateValue::equals(const Value *that, bool strict) const
{
   const ImmediateValue *imm = that->asImm();
   if (!imm)
      return false;
   return reg.data.u64 == imm->reg.data.u64;
}

bool
Symbol::equals(const Value *that, bool strict) const
{
   if (reg.file != that->reg.file || reg.fileIndex != that->reg.fileIndex)
      return false;
   assert(that->asSym());

   if (this->baseSym != that->asSym()->baseSym)
      return false;

   if (reg.file == FILE_SYSTEM_VALUE)
      return (this->reg.data.sv.sv    == that->reg.data.sv.sv &&
              this->reg.data.sv.index == that->reg.data.sv.index);
   return this->reg.data.offset == that->reg.data.offset;
}

void Instruction::init()
{
   next = prev = 0;
   serial = 0;

   cc = CC_ALWAYS;
   rnd = ROUND_N;
   cache = CACHE_CA;
   subOp = 0;

   saturate = 0;
   join = 0;
   exit = 0;
   terminator = 0;
   ftz = 0;
   dnz = 0;
   perPatch = 0;
   fixed = 0;
   encSize = 0;
   ipa = 0;
   mask = 0;
   precise = 0;

   lanes = 0xf;

   postFactor = 0;

   predSrc = -1;
   flagsDef = -1;
   flagsSrc = -1;

   sched = 0;
   bb = NULL;
}

Instruction::Instruction()
{
   init();

   op = OP_NOP;
   dType = sType = TYPE_F32;

   id = -1;
}

Instruction::Instruction(Function *fn, operation opr, DataType ty)
{
   init();

   op = opr;
   dType = sType = ty;

   fn->add(this, id);
}

Instruction::~Instruction()
{
   if (bb) {
      Function *fn = bb->getFunction();
      bb->remove(this);
      fn->allInsns.remove(id);
   }

   for (int s = 0; srcExists(s); ++s)
      setSrc(s, NULL);
   // must unlink defs too since the list pointers will get deallocated
   for (int d = 0; defExists(d); ++d)
      setDef(d, NULL);
}

void
Instruction::setDef(int i, Value *val)
{
   int size = defs.size();
   if (i >= size) {
      defs.resize(i + 1);
      while (size <= i)
         defs[size++].setInsn(this);
   }
   defs[i].set(val);
}

void
Instruction::setSrc(int s, Value *val)
{
   int size = srcs.size();
   if (s >= size) {
      srcs.resize(s + 1);
      while (size <= s)
         srcs[size++].setInsn(this);
   }
   srcs[s].set(val);
}

void
Instruction::setSrc(int s, const ValueRef& ref)
{
   setSrc(s, ref.get());
   srcs[s].mod = ref.mod;
}

void
Instruction::swapSources(int a, int b)
{
   Value *value = srcs[a].get();
   Modifier m = srcs[a].mod;

   setSrc(a, srcs[b]);

   srcs[b].set(value);
   srcs[b].mod = m;
}

static inline void moveSourcesAdjustIndex(int8_t &index, int s, int delta)
{
   if (index >= s)
      index += delta;
   else
   if ((delta < 0) && (index >= (s + delta)))
      index = -1;
}

// Moves sources [@s,last_source] by @delta.
// If @delta < 0, sources [@s - abs(@delta), @s) are erased.
void
Instruction::moveSources(const int s, const int delta)
{
   if (delta == 0)
      return;
   assert(s + delta >= 0);

   int k;

   for (k = 0; srcExists(k); ++k) {
      for (int i = 0; i < 2; ++i)
         moveSourcesAdjustIndex(src(k).indirect[i], s, delta);
   }
   moveSourcesAdjustIndex(predSrc, s, delta);
   moveSourcesAdjustIndex(flagsSrc, s, delta);
   if (asTex()) {
      TexInstruction *tex = asTex();
      moveSourcesAdjustIndex(tex->tex.rIndirectSrc, s, delta);
      moveSourcesAdjustIndex(tex->tex.sIndirectSrc, s, delta);
   }

   if (delta > 0) {
      --k;
      for (int p = k + delta; k >= s; --k, --p)
         setSrc(p, src(k));
   } else {
      int p;
      for (p = s; p < k; ++p)
         setSrc(p + delta, src(p));
      for (; (p + delta) < k; ++p)
         setSrc(p + delta, NULL);
   }
}

void
Instruction::takeExtraSources(int s, Value *values[3])
{
   values[0] = getIndirect(s, 0);
   if (values[0])
      setIndirect(s, 0, NULL);

   values[1] = getIndirect(s, 1);
   if (values[1])
      setIndirect(s, 1, NULL);

   values[2] = getPredicate();
   if (values[2])
      setPredicate(cc, NULL);
}

void
Instruction::putExtraSources(int s, Value *values[3])
{
   if (values[0])
      setIndirect(s, 0, values[0]);
   if (values[1])
      setIndirect(s, 1, values[1]);
   if (values[2])
      setPredicate(cc, values[2]);
}

Instruction *
Instruction::clone(ClonePolicy<Function>& pol, Instruction *i) const
{
   if (!i)
      i = new_Instruction(pol.context(), op, dType);
#if !defined(NDEBUG) && defined(__cpp_rtti)
   assert(typeid(*i) == typeid(*this));
#endif

   pol.set<Instruction>(this, i);

   i->sType = sType;

   i->rnd = rnd;
   i->cache = cache;
   i->subOp = subOp;

   i->saturate = saturate;
   i->join = join;
   i->exit = exit;
   i->mask = mask;
   i->ftz = ftz;
   i->dnz = dnz;
   i->ipa = ipa;
   i->lanes = lanes;
   i->perPatch = perPatch;

   i->postFactor = postFactor;

   for (int d = 0; defExists(d); ++d)
      i->setDef(d, pol.get(getDef(d)));

   for (int s = 0; srcExists(s); ++s) {
      i->setSrc(s, pol.get(getSrc(s)));
      i->src(s).mod = src(s).mod;
   }

   i->cc = cc;
   i->predSrc = predSrc;
   i->flagsDef = flagsDef;
   i->flagsSrc = flagsSrc;

   return i;
}

unsigned int
Instruction::defCount(unsigned int mask, bool singleFile) const
{
   unsigned int i, n;

   if (singleFile) {
      unsigned int d = ffs(mask);
      if (!d)
         return 0;
      for (i = d--; defExists(i); ++i)
         if (getDef(i)->reg.file != getDef(d)->reg.file)
            mask &= ~(1 << i);
   }

   for (n = 0, i = 0; this->defExists(i); ++i, mask >>= 1)
      n += mask & 1;
   return n;
}

unsigned int
Instruction::srcCount(unsigned int mask, bool singleFile) const
{
   unsigned int i, n;

   if (singleFile) {
      unsigned int s = ffs(mask);
      if (!s)
         return 0;
      for (i = s--; srcExists(i); ++i)
         if (getSrc(i)->reg.file != getSrc(s)->reg.file)
            mask &= ~(1 << i);
   }

   for (n = 0, i = 0; this->srcExists(i); ++i, mask >>= 1)
      n += mask & 1;
   return n;
}

bool
Instruction::setIndirect(int s, int dim, Value *value)
{
   assert(this->srcExists(s));

   int p = srcs[s].indirect[dim];
   if (p < 0) {
      if (!value)
         return true;
      p = srcs.size();
      while (p > 0 && !srcExists(p - 1))
         --p;
   }
   setSrc(p, value);
   srcs[p].usedAsPtr = (value != 0);
   srcs[s].indirect[dim] = value ? p : -1;
   return true;
}

bool
Instruction::setPredicate(CondCode ccode, Value *value)
{
   cc = ccode;

   if (!value) {
      if (predSrc >= 0) {
         srcs[predSrc].set(NULL);
         predSrc = -1;
      }
      return true;
   }

   if (predSrc < 0) {
      predSrc = srcs.size();
      while (predSrc > 0 && !srcExists(predSrc - 1))
         --predSrc;
   }

   setSrc(predSrc, value);
   return true;
}

bool
Instruction::writesPredicate() const
{
   for (int d = 0; defExists(d); ++d)
      if (getDef(d)->inFile(FILE_PREDICATE) || getDef(d)->inFile(FILE_FLAGS))
         return true;
   return false;
}

bool
Instruction::canCommuteDefSrc(const Instruction *i) const
{
   for (int d = 0; defExists(d); ++d)
      for (int s = 0; i->srcExists(s); ++s)
         if (getDef(d)->interfers(i->getSrc(s)))
            return false;
   return true;
}

bool
Instruction::canCommuteDefDef(const Instruction *i) const
{
   for (int d = 0; defExists(d); ++d)
      for (int c = 0; i->defExists(c); ++c)
         if (getDef(d)->interfers(i->getDef(c)))
            return false;
   return true;
}

bool
Instruction::isCommutationLegal(const Instruction *i) const
{
   return canCommuteDefDef(i) &&
      canCommuteDefSrc(i) &&
      i->canCommuteDefSrc(this);
}

TexInstruction::TexInstruction(Function *fn, operation op)
   : Instruction(fn, op, TYPE_F32), tex()
{
   tex.rIndirectSrc = -1;
   tex.sIndirectSrc = -1;

   if (op == OP_TXF)
      sType = TYPE_U32;
}

TexInstruction::~TexInstruction()
{
   for (int c = 0; c < 3; ++c) {
      dPdx[c].set(NULL);
      dPdy[c].set(NULL);
   }
   for (int n = 0; n < 4; ++n)
      for (int c = 0; c < 3; ++c)
         offset[n][c].set(NULL);
}

TexInstruction *
TexInstruction::clone(ClonePolicy<Function>& pol, Instruction *i) const
{
   TexInstruction *tex = (i ? static_cast<TexInstruction *>(i) :
                          new_TexInstruction(pol.context(), op));

   Instruction::clone(pol, tex);

   tex->tex = this->tex;

   if (op == OP_TXD) {
      for (unsigned int c = 0; c < tex->tex.target.getDim(); ++c) {
         tex->dPdx[c].set(dPdx[c]);
         tex->dPdy[c].set(dPdy[c]);
      }
   }

   for (int n = 0; n < tex->tex.useOffsets; ++n)
      for (int c = 0; c < 3; ++c)
         tex->offset[n][c].set(offset[n][c]);

   return tex;
}

const struct TexInstruction::Target::Desc TexInstruction::Target::descTable[] =
{
   { "1D",                1, 1, false, false, false },
   { "2D",                2, 2, false, false, false },
   { "2D_MS",             2, 3, false, false, false },
   { "3D",                3, 3, false, false, false },
   { "CUBE",              2, 3, false, true,  false },
   { "1D_SHADOW",         1, 1, false, false, true  },
   { "2D_SHADOW",         2, 2, false, false, true  },
   { "CUBE_SHADOW",       2, 3, false, true,  true  },
   { "1D_ARRAY",          1, 2, true,  false, false },
   { "2D_ARRAY",          2, 3, true,  false, false },
   { "2D_MS_ARRAY",       2, 4, true,  false, false },
   { "CUBE_ARRAY",        2, 4, true,  true,  false },
   { "1D_ARRAY_SHADOW",   1, 2, true,  false, true  },
   { "2D_ARRAY_SHADOW",   2, 3, true,  false, true  },
   { "RECT",              2, 2, false, false, false },
   { "RECT_SHADOW",       2, 2, false, false, true  },
   { "CUBE_ARRAY_SHADOW", 2, 4, true,  true,  true  },
   { "BUFFER",            1, 1, false, false, false },
};

const struct TexInstruction::ImgFormatDesc TexInstruction::formatTable[] =
{
   { "RGBA32F",      4, { 32, 32, 32, 32 }, FLOAT },
   { "RGBA16F",      4, { 16, 16, 16, 16 }, FLOAT },
   { "RG32F",        2, { 32, 32,  0,  0 }, FLOAT },
   { "RG16F",        2, { 16, 16,  0,  0 }, FLOAT },
   { "R11G11B10F",   3, { 11, 11, 10,  0 }, FLOAT },
   { "R32F",         1, { 32,  0,  0,  0 }, FLOAT },
   { "R16F",         1, { 16,  0,  0,  0 }, FLOAT },

   { "RGBA32UI",     4, { 32, 32, 32, 32 },  UINT },
   { "RGBA16UI",     4, { 16, 16, 16, 16 },  UINT },
   { "RGB10A2UI",    4, { 10, 10, 10,  2 },  UINT },
   { "RGBA8UI",      4, {  8,  8,  8,  8 },  UINT },
   { "RG32UI",       2, { 32, 32,  0,  0 },  UINT },
   { "RG16UI",       2, { 16, 16,  0,  0 },  UINT },
   { "RG8UI",        2, {  8,  8,  0,  0 },  UINT },
   { "R32UI",        1, { 32,  0,  0,  0 },  UINT },
   { "R16UI",        1, { 16,  0,  0,  0 },  UINT },
   { "R8UI",         1, {  8,  0,  0,  0 },  UINT },

   { "RGBA32I",      4, { 32, 32, 32, 32 },  SINT },
   { "RGBA16I",      4, { 16, 16, 16, 16 },  SINT },
   { "RGBA8I",       4, {  8,  8,  8,  8 },  SINT },
   { "RG32I",        2, { 32, 32,  0,  0 },  SINT },
   { "RG16I",        2, { 16, 16,  0,  0 },  SINT },
   { "RG8I",         2, {  8,  8,  0,  0 },  SINT },
   { "R32I",         1, { 32,  0,  0,  0 },  SINT },
   { "R16I",         1, { 16,  0,  0,  0 },  SINT },
   { "R8I",          1, {  8,  0,  0,  0 },  SINT },

   { "RGBA16",       4, { 16, 16, 16, 16 }, UNORM },
   { "RGB10A2",      4, { 10, 10, 10,  2 }, UNORM },
   { "RGBA8",        4, {  8,  8,  8,  8 }, UNORM },
   { "RG16",         2, { 16, 16,  0,  0 }, UNORM },
   { "RG8",          2, {  8,  8,  0,  0 }, UNORM },
   { "R16",          1, { 16,  0,  0,  0 }, UNORM },
   { "R8",           1, {  8,  0,  0,  0 }, UNORM },

   { "RGBA16_SNORM", 4, { 16, 16, 16, 16 }, SNORM },
   { "RGBA8_SNORM",  4, {  8,  8,  8,  8 }, SNORM },
   { "RG16_SNORM",   2, { 16, 16,  0,  0 }, SNORM },
   { "RG8_SNORM",    2, {  8,  8,  0,  0 }, SNORM },
   { "R16_SNORM",    1, { 16,  0,  0,  0 }, SNORM },
   { "R8_SNORM",     1, {  8,  0,  0,  0 }, SNORM },

   { "BGRA8",        4, {  8,  8,  8,  8 }, UNORM, true },
};

const struct TexInstruction::ImgFormatDesc *
TexInstruction::translateImgFormat(enum pipe_format format)
{

#define FMT_CASE(a, b) \
  case PIPE_FORMAT_ ## a: return &formatTable[nv50_ir::FMT_ ## b]

   switch (format) {
   case PIPE_FORMAT_NONE: return NULL;

   FMT_CASE(R32G32B32A32_FLOAT, RGBA32F);
   FMT_CASE(R16G16B16A16_FLOAT, RGBA16F);
   FMT_CASE(R32G32_FLOAT, RG32F);
   FMT_CASE(R16G16_FLOAT, RG16F);
   FMT_CASE(R11G11B10_FLOAT, R11G11B10F);
   FMT_CASE(R32_FLOAT, R32F);
   FMT_CASE(R16_FLOAT, R16F);

   FMT_CASE(R32G32B32A32_UINT, RGBA32UI);
   FMT_CASE(R16G16B16A16_UINT, RGBA16UI);
   FMT_CASE(R10G10B10A2_UINT, RGB10A2UI);
   FMT_CASE(R8G8B8A8_UINT, RGBA8UI);
   FMT_CASE(R32G32_UINT, RG32UI);
   FMT_CASE(R16G16_UINT, RG16UI);
   FMT_CASE(R8G8_UINT, RG8UI);
   FMT_CASE(R32_UINT, R32UI);
   FMT_CASE(R16_UINT, R16UI);
   FMT_CASE(R8_UINT, R8UI);

   FMT_CASE(R32G32B32A32_SINT, RGBA32I);
   FMT_CASE(R16G16B16A16_SINT, RGBA16I);
   FMT_CASE(R8G8B8A8_SINT, RGBA8I);
   FMT_CASE(R32G32_SINT, RG32I);
   FMT_CASE(R16G16_SINT, RG16I);
   FMT_CASE(R8G8_SINT, RG8I);
   FMT_CASE(R32_SINT, R32I);
   FMT_CASE(R16_SINT, R16I);
   FMT_CASE(R8_SINT, R8I);

   FMT_CASE(R16G16B16A16_UNORM, RGBA16);
   FMT_CASE(R10G10B10A2_UNORM, RGB10A2);
   FMT_CASE(R8G8B8A8_UNORM, RGBA8);
   FMT_CASE(R16G16_UNORM, RG16);
   FMT_CASE(R8G8_UNORM, RG8);
   FMT_CASE(R16_UNORM, R16);
   FMT_CASE(R8_UNORM, R8);

   FMT_CASE(R16G16B16A16_SNORM, RGBA16_SNORM);
   FMT_CASE(R8G8B8A8_SNORM, RGBA8_SNORM);
   FMT_CASE(R16G16_SNORM, RG16_SNORM);
   FMT_CASE(R8G8_SNORM, RG8_SNORM);
   FMT_CASE(R16_SNORM, R16_SNORM);
   FMT_CASE(R8_SNORM, R8_SNORM);

   FMT_CASE(B8G8R8A8_UNORM, BGRA8);

   default:
      assert(!"Unexpected format");
      return NULL;
   }
}

void
TexInstruction::setIndirectR(Value *v)
{
   int p = ((tex.rIndirectSrc < 0) && v) ? srcs.size() : tex.rIndirectSrc;
   if (p >= 0) {
      tex.rIndirectSrc = p;
      setSrc(p, v);
      srcs[p].usedAsPtr = !!v;
   }
}

void
TexInstruction::setIndirectS(Value *v)
{
   int p = ((tex.sIndirectSrc < 0) && v) ? srcs.size() : tex.sIndirectSrc;
   if (p >= 0) {
      tex.sIndirectSrc = p;
      setSrc(p, v);
      srcs[p].usedAsPtr = !!v;
   }
}

CmpInstruction::CmpInstruction(Function *fn, operation op)
   : Instruction(fn, op, TYPE_F32)
{
   setCond = CC_ALWAYS;
}

CmpInstruction *
CmpInstruction::clone(ClonePolicy<Function>& pol, Instruction *i) const
{
   CmpInstruction *cmp = (i ? static_cast<CmpInstruction *>(i) :
                          new_CmpInstruction(pol.context(), op));
   cmp->dType = dType;
   Instruction::clone(pol, cmp);
   cmp->setCond = setCond;
   return cmp;
}

FlowInstruction::FlowInstruction(Function *fn, operation op, void *targ)
   : Instruction(fn, op, TYPE_NONE)
{
   if (op == OP_CALL)
      target.fn = reinterpret_cast<Function *>(targ);
   else
      target.bb = reinterpret_cast<BasicBlock *>(targ);

   if (op == OP_BRA ||
       op == OP_CONT || op == OP_BREAK ||
       op == OP_RET || op == OP_EXIT)
      terminator = 1;
   else
   if (op == OP_JOIN)
      terminator = targ ? 1 : 0;

   allWarp = absolute = limit = builtin = indirect = 0;
}

FlowInstruction *
FlowInstruction::clone(ClonePolicy<Function>& pol, Instruction *i) const
{
   FlowInstruction *flow = (i ? static_cast<FlowInstruction *>(i) :
                            new_FlowInstruction(pol.context(), op, NULL));

   Instruction::clone(pol, flow);
   flow->allWarp = allWarp;
   flow->absolute = absolute;
   flow->limit = limit;
   flow->builtin = builtin;

   if (builtin)
      flow->target.builtin = target.builtin;
   else
   if (op == OP_CALL)
      flow->target.fn = target.fn;
   else
   if (target.bb)
      flow->target.bb = pol.get<BasicBlock>(target.bb);

   return flow;
}

Program::Program(Type type, Target *arch)
   : progType(type),
     target(arch),
     tlsSize(0),
     mem_Instruction(sizeof(Instruction), 6),
     mem_CmpInstruction(sizeof(CmpInstruction), 4),
     mem_TexInstruction(sizeof(TexInstruction), 4),
     mem_FlowInstruction(sizeof(FlowInstruction), 4),
     mem_LValue(sizeof(LValue), 8),
     mem_Symbol(sizeof(Symbol), 7),
     mem_ImmediateValue(sizeof(ImmediateValue), 7),
     driver(NULL),
     driver_out(NULL)
{
   code = NULL;
   binSize = 0;

   maxGPR = -1;
   fp64 = false;
   persampleInvocation = false;

   main = new Function(this, "MAIN", ~0);
   calls.insert(&main->call);

   dbgFlags = 0;
   optLevel = 0;

   targetPriv = NULL;
}

Program::~Program()
{
   for (ArrayList::Iterator it = allFuncs.iterator(); !it.end(); it.next())
      delete reinterpret_cast<Function *>(it.get());

   for (ArrayList::Iterator it = allRValues.iterator(); !it.end(); it.next())
      releaseValue(reinterpret_cast<Value *>(it.get()));
}

void Program::releaseInstruction(Instruction *insn)
{
   // TODO: make this not suck so much

   insn->~Instruction();

   if (insn->asCmp())
      mem_CmpInstruction.release(insn);
   else
   if (insn->asTex())
      mem_TexInstruction.release(insn);
   else
   if (insn->asFlow())
      mem_FlowInstruction.release(insn);
   else
      mem_Instruction.release(insn);
}

void Program::releaseValue(Value *value)
{
   value->~Value();

   if (value->asLValue())
      mem_LValue.release(value);
   else
   if (value->asImm())
      mem_ImmediateValue.release(value);
   else
   if (value->asSym())
      mem_Symbol.release(value);
}


} // namespace nv50_ir

extern "C" {

static void
nv50_ir_init_prog_info(struct nv50_ir_prog_info *info,
                       struct nv50_ir_prog_info_out *info_out)
{
   info_out->target = info->target;
   info_out->type = info->type;
   if (info->type == PIPE_SHADER_TESS_CTRL || info->type == PIPE_SHADER_TESS_EVAL) {
      info_out->prop.tp.domain = MESA_PRIM_COUNT;
      info_out->prop.tp.outputPrim = MESA_PRIM_COUNT;
   }
   if (info->type == PIPE_SHADER_GEOMETRY) {
      info_out->prop.gp.instanceCount = 1;
      info_out->prop.gp.maxVertices = 1;
   }
   if (info->type == PIPE_SHADER_COMPUTE) {
      info->prop.cp.numThreads[0] =
      info->prop.cp.numThreads[1] =
      info->prop.cp.numThreads[2] = 1;
   }
   info_out->bin.smemSize = info->bin.smemSize;
   info_out->io.instanceId = 0xff;
   info_out->io.vertexId = 0xff;
   info_out->io.edgeFlagIn = 0xff;
   info_out->io.edgeFlagOut = 0xff;
   info_out->io.fragDepth = 0xff;
   info_out->io.sampleMask = 0xff;
}

int
nv50_ir_generate_code(struct nv50_ir_prog_info *info,
                      struct nv50_ir_prog_info_out *info_out)
{
   int ret = 0;

   nv50_ir::Program::Type type;

   nv50_ir_init_prog_info(info, info_out);

#define PROG_TYPE_CASE(a, b)                                      \
   case PIPE_SHADER_##a: type = nv50_ir::Program::TYPE_##b; break

   switch (info->type) {
   PROG_TYPE_CASE(VERTEX, VERTEX);
   PROG_TYPE_CASE(TESS_CTRL, TESSELLATION_CONTROL);
   PROG_TYPE_CASE(TESS_EVAL, TESSELLATION_EVAL);
   PROG_TYPE_CASE(GEOMETRY, GEOMETRY);
   PROG_TYPE_CASE(FRAGMENT, FRAGMENT);
   PROG_TYPE_CASE(COMPUTE, COMPUTE);
   default:
      INFO_DBG(info->dbgFlags, VERBOSE, "unsupported program type %u\n", info->type);
      return -1;
   }
   INFO_DBG(info->dbgFlags, VERBOSE, "translating program of type %u\n", type);

   nv50_ir::Target *targ = nv50_ir::Target::create(info->target);
   if (!targ)
      return -1;

   nv50_ir::Program *prog = new nv50_ir::Program(type, targ);
   if (!prog) {
      nv50_ir::Target::destroy(targ);
      return -1;
   }
   prog->driver = info;
   prog->driver_out = info_out;
   prog->dbgFlags = info->dbgFlags;
   prog->optLevel = info->optLevel;

   ret = prog->makeFromNIR(info, info_out) ? 0 : -2;
   if (ret < 0)
      goto out;
   if (prog->dbgFlags & NV50_IR_DEBUG_VERBOSE)
      prog->print();

   targ->parseDriverInfo(info, info_out);
   prog->getTarget()->runLegalizePass(prog, nv50_ir::CG_STAGE_PRE_SSA);

   prog->convertToSSA();

   if (prog->dbgFlags & NV50_IR_DEBUG_VERBOSE)
      prog->print();

   prog->optimizeSSA(info->optLevel);
   prog->getTarget()->runLegalizePass(prog, nv50_ir::CG_STAGE_SSA);

   if (prog->dbgFlags & NV50_IR_DEBUG_BASIC)
      prog->print();

   if (!prog->registerAllocation()) {
      ret = -4;
      goto out;
   }
   prog->getTarget()->runLegalizePass(prog, nv50_ir::CG_STAGE_POST_RA);

   prog->optimizePostRA(info->optLevel);

   if (!prog->emitBinary(info_out)) {
      ret = -5;
      goto out;
   }

out:
   INFO_DBG(prog->dbgFlags, VERBOSE, "nv50_ir_generate_code: ret = %i\n", ret);

   info_out->bin.maxGPR = prog->maxGPR;
   info_out->bin.code = prog->code;
   info_out->bin.codeSize = prog->binSize;
   info_out->bin.tlsSpace = ALIGN(prog->tlsSize, 0x10);

   delete prog;
   nv50_ir::Target::destroy(targ);

   return ret;
}

} // extern "C"
