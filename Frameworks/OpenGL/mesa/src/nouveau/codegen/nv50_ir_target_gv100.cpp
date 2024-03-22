/*
 * Copyright 2020 Red Hat Inc.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include "nv50_ir_target_gv100.h"
#include "nv50_ir_lowering_gv100.h"
#include "nv50_ir_emit_gv100.h"

namespace nv50_ir {

void
TargetGV100::initOpInfo()
{
   unsigned int i, j;

   static const operation commutative[] =
   {
      OP_ADD, OP_MUL, OP_MAD, OP_FMA, OP_MAX, OP_MIN,
      OP_SET_AND, OP_SET_OR, OP_SET_XOR, OP_SET, OP_SELP, OP_SLCT
   };

   static const operation noDest[] =
   {
      OP_EXIT
   };

   static const operation noPred[] =
   {
   };

   for (i = 0; i < DATA_FILE_COUNT; ++i)
      nativeFileMap[i] = (DataFile)i;
   nativeFileMap[FILE_ADDRESS] = FILE_GPR;
   nativeFileMap[FILE_FLAGS] = FILE_PREDICATE;

   for (i = 0; i < OP_LAST; ++i) {
      opInfo[i].variants = NULL;
      opInfo[i].op = (operation)i;
      opInfo[i].srcTypes = 1 << (int)TYPE_F32;
      opInfo[i].dstTypes = 1 << (int)TYPE_F32;
      opInfo[i].immdBits = 0;
      opInfo[i].srcNr = operationSrcNr[i];

      for (j = 0; j < opInfo[i].srcNr; ++j) {
         opInfo[i].srcMods[j] = 0;
         opInfo[i].srcFiles[j] = 1 << (int)FILE_GPR;
      }
      opInfo[i].dstMods = 0;
      opInfo[i].dstFiles = 1 << (int)FILE_GPR;

      opInfo[i].hasDest = 1;
      opInfo[i].vector = (i >= OP_TEX && i <= OP_TEXCSAA);
      opInfo[i].commutative = false; /* set below */
      opInfo[i].pseudo = (i < OP_MOV);
      opInfo[i].predicate = !opInfo[i].pseudo;
      opInfo[i].flow = (i >= OP_BRA && i <= OP_JOIN);
      opInfo[i].minEncSize = 16;
   }
   for (i = 0; i < ARRAY_SIZE(commutative); ++i)
      opInfo[commutative[i]].commutative = true;
   for (i = 0; i < ARRAY_SIZE(noDest); ++i)
      opInfo[noDest[i]].hasDest = 0;
   for (i = 0; i < ARRAY_SIZE(noPred); ++i)
      opInfo[noPred[i]].predicate = 0;
}

struct opInfo {
   struct {
      uint8_t files;
      uint8_t mods;
   } src[3];
};

#define SRC_NONE 0
#define SRC_R    (1 << FILE_GPR)
#define SRC_I    (1 << FILE_MEMORY_CONST)
#define SRC_C    (1 << FILE_IMMEDIATE)
#define SRC_RC   (SRC_R |         SRC_C)
#define SRC_RI   (SRC_R | SRC_I        )
#define SRC_RIC  (SRC_R | SRC_I | SRC_C)

#define MOD_NONE 0
#define MOD_NEG  NV50_IR_MOD_NEG
#define MOD_ABS  NV50_IR_MOD_ABS
#define MOD_NOT  NV50_IR_MOD_NOT
#define MOD_NA   (MOD_NEG | MOD_ABS)

#define OPINFO(O,SA,MA,SB,MB,SC,MC)                                            \
static struct opInfo                                                           \
opInfo_##O = {                                                                 \
   .src = { { SRC_##SA, MOD_##MA },                                            \
            { SRC_##SB, MOD_##MB },                                            \
            { SRC_##SC, MOD_##MC }},                                           \
};


/* Handled by GV100LegalizeSSA. */
OPINFO(FABS     , RIC , NA  , NONE, NONE, NONE, NONE);
OPINFO(FCMP     , R   , NONE, RIC , NONE, RIC , NONE); //XXX: use FSEL for mods
OPINFO(FNEG     , RIC , NA  , NONE, NONE, NONE, NONE);
OPINFO(FSET     , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(ICMP     , R   , NONE, RIC , NONE, RIC , NONE);
OPINFO(IMUL     , R   , NONE, RIC , NONE, NONE, NONE);
OPINFO(INEG     , RIC , NEG , NONE, NONE, NONE, NONE);
OPINFO(ISET     , R   , NONE, RIC , NONE, NONE, NONE);
OPINFO(LOP2     , R   , NOT , RIC , NOT , NONE, NONE);
OPINFO(NOT      , RIC , NONE, NONE, NONE, NONE, NONE);
OPINFO(SAT      , RIC , NA  , NONE, NONE, NONE, NONE);
OPINFO(SHL      , RIC , NONE, RIC , NONE, NONE, NONE);
OPINFO(SHR      , RIC , NONE, RIC , NONE, NONE, NONE);
OPINFO(SUB      , R   , NONE, RIC , NEG , NONE, NONE);
OPINFO(IMNMX    , R   , NONE, RIC , NONE, NONE, NONE);

/* Handled by CodeEmitterGV100. */
OPINFO(AL2P     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(ALD      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(AST      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(ATOM     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(ATOMS    , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(BAR      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(BRA      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(BMSK     , R   , NONE, RIC , NONE, NONE, NONE);
OPINFO(BREV     , RIC , NONE, NONE, NONE, NONE, NONE);
OPINFO(CCTL     , NONE, NONE, NONE, NONE, NONE, NONE);
//OPINFO(CS2R     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(DADD     , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(DFMA     , R   , NA  , RIC , NA  , RIC , NA  );
OPINFO(DMUL     , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(DSETP    , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(EXIT     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(F2F      , RIC , NA  , NONE, NONE, NONE, NONE);
OPINFO(F2I      , RIC , NA  , NONE, NONE, NONE, NONE);
OPINFO(FADD     , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(FFMA     , R   , NA  , RIC , NA  , RIC , NA  );
OPINFO(FLO      , RIC , NOT , NONE, NONE, NONE, NONE);
OPINFO(FMNMX    , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(FMUL     , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(FRND     , RIC , NA  , NONE, NONE, NONE, NONE);
OPINFO(FSET_BF  , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(FSETP    , R   , NA  , RIC , NA  , NONE, NONE);
OPINFO(FSWZADD  , R   , NONE, R   , NONE, NONE, NONE);
OPINFO(I2F      , RIC , NONE, NONE, NONE, NONE, NONE);
OPINFO(IABS     , RIC , NONE, NONE, NONE, NONE, NONE);
OPINFO(IADD3    , R   , NEG , RIC , NEG , R   , NEG );
OPINFO(IMAD     , R   , NONE, RIC , NONE, RIC , NEG );
OPINFO(IMAD_WIDE, R   , NONE, RIC , NONE, RC  , NEG );
OPINFO(IPA      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(ISBERD   , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(ISETP    , R   , NONE, RIC , NONE, NONE, NONE);
OPINFO(KILL     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(LD       , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(LDC      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(LDL      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(LDS      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(LEA      , R   , NEG , I   , NONE, RIC , NEG );
OPINFO(LOP3_LUT , R   , NONE, RIC , NONE, R   , NONE);
OPINFO(MEMBAR   , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(MOV      , RIC , NONE, NONE, NONE, NONE, NONE);
OPINFO(MUFU     , RIC , NA  , NONE, NONE, NONE, NONE);
OPINFO(NOP      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(OUT      , R   , NONE, RI  , NONE, NONE, NONE);
OPINFO(PIXLD    , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(PLOP3_LUT, NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(POPC     , RIC , NOT , NONE, NONE, NONE, NONE);
OPINFO(PRMT     , R   , NONE, RIC , NONE, RIC , NONE);
OPINFO(RED      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(SGXT     , R   , NONE, RIC , NONE, NONE, NONE);
OPINFO(S2R      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(SEL      , R   , NONE, RIC , NONE, NONE, NONE);
OPINFO(SHF      , R   , NONE, RIC , NONE, RIC , NONE);
OPINFO(SHFL     , R   , NONE, R   , NONE, R   , NONE);
OPINFO(ST       , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(STL      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(STS      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(SUATOM   , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(SULD     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(SUST     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(TEX      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(TLD      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(TLD4     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(TMML     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(TXD      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(TXQ      , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(VOTE     , NONE, NONE, NONE, NONE, NONE, NONE);
OPINFO(WARPSYNC , R   , NONE, NONE, NONE, NONE, NONE);

static const struct opInfo *
getOpInfo(const Instruction *i)
{
   switch (i->op) {
   case OP_ABS:
      if (isFloatType(i->dType))
         return &opInfo_FABS;
      return &opInfo_IABS;
   case OP_ADD:
      if (isFloatType(i->dType)) {
         if (i->dType == TYPE_F32)
            return &opInfo_FADD;
         else
            return &opInfo_DADD;
      } else {
         return &opInfo_IADD3;
      }
      break;
   case OP_AFETCH: return &opInfo_AL2P;
   case OP_AND:
   case OP_OR:
   case OP_XOR:
      if (i->def(0).getFile() == FILE_PREDICATE)
         return &opInfo_PLOP3_LUT;
      return &opInfo_LOP2;
   case OP_ATOM:
      if (i->src(0).getFile() == FILE_MEMORY_SHARED)
         return &opInfo_ATOMS;
      else
         if (!i->defExists(0) && i->subOp < NV50_IR_SUBOP_ATOM_CAS)
            return &opInfo_RED;
         else
            return &opInfo_ATOM;
      break;
   case OP_BAR: return &opInfo_BAR;
   case OP_BFIND: return &opInfo_FLO;
   case OP_BMSK: return &opInfo_BMSK;
   case OP_BREV: return &opInfo_BREV;
   case OP_BRA:
   case OP_JOIN: return &opInfo_BRA; //XXX
   case OP_CCTL: return &opInfo_CCTL;
   case OP_CEIL:
   case OP_CVT:
   case OP_FLOOR:
   case OP_TRUNC:
      if (i->op == OP_CVT && (i->def(0).getFile() == FILE_PREDICATE ||
                                 i->src(0).getFile() == FILE_PREDICATE)) {
         return &opInfo_MOV;
      } else if (isFloatType(i->dType)) {
         if (isFloatType(i->sType)) {
            if (i->sType == i->dType)
               return &opInfo_FRND;
            else
               return &opInfo_F2F;
         } else {
            return &opInfo_I2F;
         }
      } else {
         if (isFloatType(i->sType))
            return &opInfo_F2I;
      }
      break;
   case OP_COS:
   case OP_EX2:
   case OP_LG2:
   case OP_RCP:
   case OP_RSQ:
   case OP_SIN:
   case OP_SQRT: return &opInfo_MUFU;
   case OP_DISCARD: return &opInfo_KILL;
   case OP_EMIT:
   case OP_FINAL:
   case OP_RESTART: return &opInfo_OUT;
   case OP_EXIT: return &opInfo_EXIT;
   case OP_EXPORT: return &opInfo_AST;
   case OP_FMA:
   case OP_MAD:
      if (isFloatType(i->dType)) {
         if (i->dType == TYPE_F32)
            return &opInfo_FFMA;
         else
            return &opInfo_DFMA;
      } else {
         if (typeSizeof(i->dType) != 8)
            return &opInfo_IMAD;
         else
            return &opInfo_IMAD_WIDE;
      }
      break;
   case OP_JOINAT: return &opInfo_NOP; //XXX
   case OP_LINTERP: return &opInfo_IPA;
   case OP_LOAD:
      switch (i->src(0).getFile()) {
      case FILE_MEMORY_CONST : return &opInfo_LDC;
      case FILE_MEMORY_LOCAL : return &opInfo_LDL;
      case FILE_MEMORY_SHARED: return &opInfo_LDS;
      case FILE_MEMORY_GLOBAL: return &opInfo_LD;
      default:
         break;
      }
      break;
   case OP_LOP3_LUT: return &opInfo_LOP3_LUT;
   case OP_MAX:
   case OP_MIN:
      if (isFloatType(i->dType)) {
         if (i->dType == TYPE_F32)
            return &opInfo_FMNMX;
      } else {
         return &opInfo_IMNMX;
      }
      break;
   case OP_MEMBAR: return &opInfo_MEMBAR;
   case OP_MOV: return &opInfo_MOV;
   case OP_MUL:
      if (isFloatType(i->dType)) {
         if (i->dType == TYPE_F32)
            return &opInfo_FMUL;
         else
            return &opInfo_DMUL;
      }
      return &opInfo_IMUL;
   case OP_NEG:
      if (isFloatType(i->dType))
         return &opInfo_FNEG;
      return &opInfo_INEG;
   case OP_NOT: return &opInfo_NOT;
   case OP_PERMT: return &opInfo_PRMT;
   case OP_PFETCH: return &opInfo_ISBERD;
   case OP_PIXLD: return &opInfo_PIXLD;
   case OP_POPCNT: return &opInfo_POPC;
   case OP_QUADOP: return &opInfo_FSWZADD;
   case OP_RDSV:
#if 0
      if (targ->isCS2RSV(i->getSrc(0)->reg.data.sv.sv))
         return &opInfo_CS2R;
#endif
      return &opInfo_S2R;
   case OP_SAT: return &opInfo_SAT;
   case OP_SELP: return &opInfo_SEL;
   case OP_SET:
   case OP_SET_AND:
   case OP_SET_OR:
   case OP_SET_XOR:
      if (i->def(0).getFile() != FILE_PREDICATE) {
         if (isFloatType(i->dType)) {
            if (i->dType == TYPE_F32)
               return &opInfo_FSET_BF;
         } else {
            if (isFloatType(i->sType))
                  return &opInfo_FSET;
            return &opInfo_ISET;
         }
      } else {
         if (isFloatType(i->sType))
            if (i->sType == TYPE_F64)
               return &opInfo_DSETP;
            else
               return &opInfo_FSETP;
         else
            return &opInfo_ISETP;
      }
      break;
   case OP_SGXT: return &opInfo_SGXT;
   case OP_SHF: return &opInfo_SHF;
   case OP_SHFL: return &opInfo_SHFL;
   case OP_SHL: return &opInfo_SHL;
   case OP_SHLADD: return &opInfo_LEA;
   case OP_SHR: return &opInfo_SHR;
   case OP_SLCT:
      if (isFloatType(i->sType))
         return &opInfo_FCMP;
      return &opInfo_ICMP;
   case OP_STORE:
      switch (i->src(0).getFile()) {
      case FILE_MEMORY_LOCAL : return &opInfo_STL;
      case FILE_MEMORY_SHARED: return &opInfo_STS;
      case FILE_MEMORY_GLOBAL: return &opInfo_ST;
      default:
         break;
      }
      break;
   case OP_SUB: return &opInfo_SUB;
   case OP_SULDB:
   case OP_SULDP: return &opInfo_SULD;
   case OP_SUREDB:
   case OP_SUREDP: return &opInfo_SUATOM;
   case OP_SUSTB:
   case OP_SUSTP: return &opInfo_SUST;
   case OP_TEX:
   case OP_TXB:
   case OP_TXL: return &opInfo_TEX;
   case OP_TXD: return &opInfo_TXD;
   case OP_TXF: return &opInfo_TLD;
   case OP_TXG: return &opInfo_TLD4;
   case OP_TXLQ: return &opInfo_TMML;
   case OP_TXQ: return &opInfo_TXQ;
   case OP_VFETCH: return &opInfo_ALD;
   case OP_VOTE: return &opInfo_VOTE;
   case OP_WARPSYNC: return &opInfo_WARPSYNC;
   default:
      break;
   }
   return NULL;
}

bool
TargetGV100::isSatSupported(const Instruction *i) const
{
   switch (i->dType) {
   case TYPE_F32:
      switch (i->op) {
      case OP_ADD:
      case OP_FMA:
      case OP_MAD:
      case OP_MUL: return true;
      default:
         break;
      }
      break;
   default:
      break;
   }
   return false;
}

bool
TargetGV100::isModSupported(const Instruction *i, int s, Modifier mod) const
{
   const struct opInfo *info = nv50_ir::getOpInfo(i);
   uint8_t mods = 0;
   if (info && s < (int)ARRAY_SIZE(info->src))
      mods = info->src[s].mods;
   return (mod & Modifier(mods)) == mod;
}

bool
TargetGV100::isOpSupported(operation op, DataType ty) const
{
   if (op == OP_MAD || op == OP_FMA)
      return true;
   if (ty == TYPE_F32) {
      if (op == OP_MAX)
         return true;
   }
   if (op == OP_RSQ)
      return true;
   if (op == OP_SET ||
       op == OP_SET_AND ||
       op == OP_SET_OR ||
       op == OP_SET_XOR)
      return true;
   if (op == OP_SHLADD)
      return true;
   return false;
}

bool
TargetGV100::isBarrierRequired(const Instruction *i) const
{
   switch (i->op) {
   case OP_BREV:
      return true;
   default:
      break;
   }

   return TargetGM107::isBarrierRequired(i);
}

bool
TargetGV100::insnCanLoad(const Instruction *i, int s,
                         const Instruction *ld) const
{
   const struct opInfo *info = nv50_ir::getOpInfo(i);
   uint16_t files = 0;

   if (ld->src(0).getFile() == FILE_IMMEDIATE && ld->getSrc(0)->reg.data.u64 == 0)
      return (!i->isPseudo() &&
              !i->asTex() &&
              i->op != OP_EXPORT && i->op != OP_STORE);

   if (ld->src(0).isIndirect(0))
      return false;

   if (info && s < (int)ARRAY_SIZE(info->src)) {
      files = info->src[s].files;
      if ((s == 1 && i->srcExists(2) && i->src(2).getFile() != FILE_GPR) ||
          (s == 2 && i->srcExists(1) && i->src(1).getFile() != FILE_GPR)) {
         files &= ~(1 << FILE_MEMORY_CONST);
         files &= ~(1 << FILE_IMMEDIATE);
      } else
      if ((i->op == OP_SHL || i->op == OP_SHR) &&
          ((s == 0 && i->srcExists(1) && i->src(1).getFile() != FILE_GPR) ||
           (s == 1 && i->srcExists(0) && i->src(0).getFile() != FILE_GPR))) {
         files &= ~(1 << FILE_MEMORY_CONST);
         files &= ~(1 << FILE_IMMEDIATE);
      }
   }

   if (ld->src(0).getFile() == FILE_IMMEDIATE) {
      if (i->sType == TYPE_F64) {
         if (ld->getSrc(0)->asImm()->reg.data.u64 & 0x00000000ffffffff)
            return false;
      }
   }

   return (files & (1 << ld->src(0).getFile()));
}

void
TargetGV100::getBuiltinCode(const uint32_t **code, uint32_t *size) const
{
   //XXX: find out why gv100 (tu1xx is fine) hangs without this
   static uint32_t builtin[] = {
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
      0x0000794d, 0x00000000, 0x03800000, 0x03ffde00,
   };
   *code = builtin;
   *size = sizeof(builtin);
}

uint32_t
TargetGV100::getBuiltinOffset(int builtin) const
{
   return 0;
}

bool
TargetGV100::runLegalizePass(Program *prog, CGStage stage) const
{
   if (stage == CG_STAGE_PRE_SSA) {
      GM107LoweringPass pass1(prog);
      GV100LoweringPass pass2(prog);
      pass1.run(prog, false, true);
      pass2.run(prog, false, true);
      return true;
   } else
   if (stage == CG_STAGE_SSA) {
      GV100LegalizeSSA pass(prog);
      return pass.run(prog, false, true);
   } else
   if (stage == CG_STAGE_POST_RA) {
      NVC0LegalizePostRA pass(prog);
      return pass.run(prog, false, true);
   }
   return false;
}

CodeEmitter *
TargetGV100::getCodeEmitter(Program::Type type)
{
   return new CodeEmitterGV100(this);
}

TargetGV100::TargetGV100(unsigned int chipset)
   : TargetGM107(chipset)
{
   initOpInfo();
};

Target *getTargetGV100(unsigned int chipset)
{
   return new TargetGV100(chipset);
}

};
