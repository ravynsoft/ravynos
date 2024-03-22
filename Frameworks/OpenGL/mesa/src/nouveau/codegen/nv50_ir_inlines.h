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

#ifndef __NV50_IR_INLINES_H__
#define __NV50_IR_INLINES_H__

static inline CondCode reverseCondCode(CondCode cc)
{
   static const uint8_t ccRev[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };

   return static_cast<CondCode>(ccRev[cc & 7] | (cc & ~7));
}

static inline CondCode inverseCondCode(CondCode cc)
{
   return static_cast<CondCode>(cc ^ 7);
}

static inline bool isMemoryFile(DataFile f)
{
   return (f >= FILE_MEMORY_CONST && f <= FILE_MEMORY_LOCAL);
}

// contrary to asTex(), this will never include SULD/SUST
static inline bool isTextureOp(operation op)
{
   return (op >= OP_TEX && op <= OP_TEXPREP);
}

static inline bool isSurfaceOp(operation op)
{
   return (op >= OP_SULDB && op <= OP_SULEA) || (op == OP_SUQ);
}

static inline unsigned int typeSizeof(DataType ty)
{
   switch (ty) {
   case TYPE_U8:
   case TYPE_S8:
      return 1;
   case TYPE_F16:
   case TYPE_U16:
   case TYPE_S16:
      return 2;
   case TYPE_F32:
   case TYPE_U32:
   case TYPE_S32:
      return 4;
   case TYPE_F64:
   case TYPE_U64:
   case TYPE_S64:
      return 8;
   case TYPE_B96:
      return 12;
   case TYPE_B128:
      return 16;
   default:
      return 0;
   }
}

static inline unsigned int typeSizeofLog2(DataType ty)
{
   switch (ty) {
   case TYPE_F16:
   case TYPE_U16:
   case TYPE_S16:
      return 1;
   case TYPE_F32:
   case TYPE_U32:
   case TYPE_S32:
      return 2;
   case TYPE_F64:
   case TYPE_U64:
   case TYPE_S64:
      return 3;
   case TYPE_B96:
   case TYPE_B128:
      return 4;
   case TYPE_U8:
   case TYPE_S8:
   default:
      return 0;
   }
}

static inline DataType typeOfSize(unsigned int size,
                                  bool flt = false, bool sgn = false)
{
   switch (size) {
   case 1: return sgn ? TYPE_S8 : TYPE_U8;
   case 2: return flt ? TYPE_F16 : (sgn ? TYPE_S16 : TYPE_U16);
   case 8: return flt ? TYPE_F64 : (sgn ? TYPE_S64 : TYPE_U64);
   case 12: return TYPE_B96;
   case 16: return TYPE_B128;
   case 4:
      return flt ? TYPE_F32 : (sgn ? TYPE_S32 : TYPE_U32);
   default:
      return TYPE_NONE;
   }
}

static inline bool isFloatType(DataType ty)
{
   return (ty >= TYPE_F16 && ty <= TYPE_F64);
}

static inline bool isSignedIntType(DataType ty)
{
   return (ty == TYPE_S8 || ty == TYPE_S16 || ty == TYPE_S32 || ty == TYPE_S64);
}

static inline bool isUnsignedIntType(DataType ty)
{
   return (ty == TYPE_U8 || ty == TYPE_U16 || ty == TYPE_U32 || ty == TYPE_U64);
}

static inline bool isIntType(DataType ty)
{
   return (isSignedIntType(ty) || isUnsignedIntType(ty));
}

static inline bool isSignedType(DataType ty)
{
   switch (ty) {
   case TYPE_NONE:
   case TYPE_U8:
   case TYPE_U16:
   case TYPE_U32:
   case TYPE_U64:
   case TYPE_B96:
   case TYPE_B128:
      return false;
   default:
      return true;
   }
}

static inline DataType intTypeToSigned(DataType ty)
{
   switch (ty) {
   case TYPE_U64: return TYPE_S64;
   case TYPE_U32: return TYPE_S32;
   case TYPE_U16: return TYPE_S16;
   case TYPE_U8: return TYPE_S8;
   default:
      return ty;
   }
}

const ValueRef *ValueRef::getIndirect(int dim) const
{
   return isIndirect(dim) ? &insn->src(indirect[dim]) : NULL;
}

DataFile ValueRef::getFile() const
{
   return value ? value->reg.file : FILE_NULL;
}

unsigned int ValueRef::getSize() const
{
   return value ? value->reg.size : 0;
}

Value *ValueRef::rep() const
{
   assert(value);
   return value->join;
}

Value *ValueDef::rep() const
{
   assert(value);
   return value->join;
}

DataFile ValueDef::getFile() const
{
   return value ? value->reg.file : FILE_NULL;
}

unsigned int ValueDef::getSize() const
{
   return value ? value->reg.size : 0;
}

void ValueDef::setSSA(LValue *lval)
{
   origin = value->asLValue();
   set(lval);
}

const LValue *ValueDef::preSSA() const
{
   return origin;
}

Instruction *Value::getInsn() const
{
   return defs.empty() ? NULL : defs.front()->getInsn();
}

Instruction *Value::getUniqueInsn() const
{
   if (defs.empty())
      return NULL;

   // after regalloc, the definitions of coalesced values are linked
   if (join != this) {
      for (DefCIterator it = defs.begin(); it != defs.end(); ++it)
         if ((*it)->get() == this)
            return (*it)->getInsn();
      // should be unreachable and trigger assertion at the end
   }
#ifndef NDEBUG
   if (reg.data.id < 0) {
      int n = 0;
      for (DefCIterator it = defs.begin(); n < 2 && it != defs.end(); ++it)
         if ((*it)->get() == this) // don't count joined values
            ++n;
      if (n > 1)
         WARN("value %%%i not uniquely defined\n", id); // return NULL ?
   }
#endif
   assert(defs.front()->get() == this);
   return defs.front()->getInsn();
}

inline bool Instruction::constrainedDefs() const
{
   return defExists(1) || op == OP_UNION;
}

Value *Instruction::getIndirect(int s, int dim) const
{
   return srcs[s].isIndirect(dim) ? getSrc(srcs[s].indirect[dim]) : NULL;
}

Value *Instruction::getPredicate() const
{
   return (predSrc >= 0) ? getSrc(predSrc) : NULL;
}

void Instruction::setFlagsDef(int d, Value *val)
{
   if (val) {
      if (flagsDef < 0)
         flagsDef = d;
      setDef(flagsDef, val);
   } else {
      if (flagsDef >= 0) {
         setDef(flagsDef, NULL);
         flagsDef = -1;
      }
   }
}

void Instruction::setFlagsSrc(int s, Value *val)
{
   flagsSrc = s;
   setSrc(flagsSrc, val);
}

Value *TexInstruction::getIndirectR() const
{
   return tex.rIndirectSrc >= 0 ? getSrc(tex.rIndirectSrc) : NULL;
}

Value *TexInstruction::getIndirectS() const
{
   return tex.rIndirectSrc >= 0 ? getSrc(tex.rIndirectSrc) : NULL;
}

CmpInstruction *Instruction::asCmp()
{
   if (op >= OP_SET_AND && op <= OP_SLCT && op != OP_SELP)
      return static_cast<CmpInstruction *>(this);
   return NULL;
}

const CmpInstruction *Instruction::asCmp() const
{
   if (op >= OP_SET_AND && op <= OP_SLCT && op != OP_SELP)
      return static_cast<const CmpInstruction *>(this);
   return NULL;
}

FlowInstruction *Instruction::asFlow()
{
   if (op >= OP_BRA && op <= OP_JOIN)
      return static_cast<FlowInstruction *>(this);
   return NULL;
}

const FlowInstruction *Instruction::asFlow() const
{
   if (op >= OP_BRA && op <= OP_JOIN)
      return static_cast<const FlowInstruction *>(this);
   return NULL;
}

TexInstruction *Instruction::asTex()
{
   if ((op >= OP_TEX && op <= OP_SULEA) || op == OP_SUQ)
      return static_cast<TexInstruction *>(this);
   return NULL;
}

const TexInstruction *Instruction::asTex() const
{
   if ((op >= OP_TEX && op <= OP_SULEA) || op == OP_SUQ)
      return static_cast<const TexInstruction *>(this);
   return NULL;
}

static inline Instruction *cloneForward(Function *ctx, Instruction *obj)
{
   DeepClonePolicy<Function> pol(ctx);

   for (int i = 0; obj->srcExists(i); ++i)
      pol.set(obj->getSrc(i), obj->getSrc(i));

   return obj->clone(pol);
}

// XXX: use a virtual function so we're really really safe ?
LValue *Value::asLValue()
{
   if (reg.file >= FILE_GPR && reg.file <= LAST_REGISTER_FILE)
      return static_cast<LValue *>(this);
   return NULL;
}

Symbol *Value::asSym()
{
   if (reg.file >= FILE_MEMORY_CONST)
      return static_cast<Symbol *>(this);
   return NULL;
}

const Symbol *Value::asSym() const
{
   if (reg.file >= FILE_MEMORY_CONST)
      return static_cast<const Symbol *>(this);
   return NULL;
}

void Symbol::setOffset(int32_t offset)
{
   reg.data.offset = offset;
}

void Symbol::setAddress(Symbol *base, int32_t offset)
{
   baseSym = base;
   reg.data.offset = offset;
}

void Symbol::setSV(SVSemantic sv, uint32_t index)
{
   reg.data.sv.sv = sv;
   reg.data.sv.index = index;
}

ImmediateValue *Value::asImm()
{
   if (reg.file == FILE_IMMEDIATE)
      return static_cast<ImmediateValue *>(this);
   return NULL;
}

const ImmediateValue *Value::asImm() const
{
   if (reg.file == FILE_IMMEDIATE)
      return static_cast<const ImmediateValue *>(this);
   return NULL;
}

Value *Value::get(Iterator &it)
{
   return reinterpret_cast<Value *>(it.get());
}

bool BasicBlock::reachableBy(const BasicBlock *by, const BasicBlock *term)
{
   return cfg.reachableBy(&by->cfg, &term->cfg);
}

BasicBlock *BasicBlock::get(Iterator &iter)
{
   return reinterpret_cast<BasicBlock *>(iter.get());
}

BasicBlock *BasicBlock::get(Graph::Node *node)
{
   assert(node);
   return reinterpret_cast<BasicBlock *>(node->data);
}

Function *Function::get(Graph::Node *node)
{
   assert(node);
   return reinterpret_cast<Function *>(node->data);
}

LValue *Function::getLValue(int id)
{
   assert((unsigned int)id < (unsigned int)allLValues.getSize());
   return reinterpret_cast<LValue *>(allLValues.get(id));
}

#endif // __NV50_IR_INLINES_H__
