/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 1999-2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include <stdio.h>
#include <assert.h>

#include "util/glheader.h"
#include "prog_instruction.h"
#include "prog_parameter.h"


/**
 * Initialize program instruction fields to defaults.
 * \param inst  first instruction to initialize
 * \param count  number of instructions to initialize
 */
void
_mesa_init_instructions(struct prog_instruction *inst, GLuint count)
{
   GLuint i;

   memset(inst, 0, count * sizeof(struct prog_instruction));

   for (i = 0; i < count; i++) {
      inst[i].SrcReg[0].File = PROGRAM_UNDEFINED;
      inst[i].SrcReg[0].Swizzle = SWIZZLE_NOOP;
      inst[i].SrcReg[1].File = PROGRAM_UNDEFINED;
      inst[i].SrcReg[1].Swizzle = SWIZZLE_NOOP;
      inst[i].SrcReg[2].File = PROGRAM_UNDEFINED;
      inst[i].SrcReg[2].Swizzle = SWIZZLE_NOOP;

      inst[i].DstReg.File = PROGRAM_UNDEFINED;
      inst[i].DstReg.WriteMask = WRITEMASK_XYZW;

      inst[i].Saturate = GL_FALSE;
   }
}


/**
 * Basic info about each instruction
 */
struct instruction_info
{
   enum prog_opcode Opcode;
   const char *Name;
   GLuint NumSrcRegs;
   GLuint NumDstRegs;
};

/**
 * Instruction info
 * \note Opcode should equal array index!
 */
static const struct instruction_info InstInfo[MAX_OPCODE] = {
   { OPCODE_NOP,    "NOP",     0, 0 },
   { OPCODE_ABS,    "ABS",     1, 1 },
   { OPCODE_ADD,    "ADD",     2, 1 },
   { OPCODE_ARL,    "ARL",     1, 1 },
   { OPCODE_CMP,    "CMP",     3, 1 },
   { OPCODE_COS,    "COS",     1, 1 },
   { OPCODE_DDX,    "DDX",     1, 1 },
   { OPCODE_DDY,    "DDY",     1, 1 },
   { OPCODE_DP2,    "DP2",     2, 1 },
   { OPCODE_DP3,    "DP3",     2, 1 },
   { OPCODE_DP4,    "DP4",     2, 1 },
   { OPCODE_DPH,    "DPH",     2, 1 },
   { OPCODE_DST,    "DST",     2, 1 },
   { OPCODE_END,    "END",     0, 0 },
   { OPCODE_EX2,    "EX2",     1, 1 },
   { OPCODE_EXP,    "EXP",     1, 1 },
   { OPCODE_FLR,    "FLR",     1, 1 },
   { OPCODE_FRC,    "FRC",     1, 1 },
   { OPCODE_KIL,    "KIL",     1, 0 },
   { OPCODE_LG2,    "LG2",     1, 1 },
   { OPCODE_LIT,    "LIT",     1, 1 },
   { OPCODE_LOG,    "LOG",     1, 1 },
   { OPCODE_LRP,    "LRP",     3, 1 },
   { OPCODE_MAD,    "MAD",     3, 1 },
   { OPCODE_MAX,    "MAX",     2, 1 },
   { OPCODE_MIN,    "MIN",     2, 1 },
   { OPCODE_MOV,    "MOV",     1, 1 },
   { OPCODE_MUL,    "MUL",     2, 1 },
   { OPCODE_POW,    "POW",     2, 1 },
   { OPCODE_RCP,    "RCP",     1, 1 },
   { OPCODE_RSQ,    "RSQ",     1, 1 },
   { OPCODE_SCS,    "SCS",     1, 1 },
   { OPCODE_SGE,    "SGE",     2, 1 },
   { OPCODE_SIN,    "SIN",     1, 1 },
   { OPCODE_SLT,    "SLT",     2, 1 },
   { OPCODE_SSG,    "SSG",     1, 1 },
   { OPCODE_SUB,    "SUB",     2, 1 },
   { OPCODE_SWZ,    "SWZ",     1, 1 },
   { OPCODE_TEX,    "TEX",     1, 1 },
   { OPCODE_TXB,    "TXB",     1, 1 },
   { OPCODE_TXD,    "TXD",     3, 1 },
   { OPCODE_TXL,    "TXL",     1, 1 },
   { OPCODE_TXP,    "TXP",     1, 1 },
   { OPCODE_XPD,    "XPD",     2, 1 }
};


/**
 * Return the number of src registers for the given instruction/opcode.
 */
GLuint
_mesa_num_inst_src_regs(enum prog_opcode opcode)
{
   assert(opcode < MAX_OPCODE);
   assert(opcode == InstInfo[opcode].Opcode);
   assert(OPCODE_XPD == InstInfo[OPCODE_XPD].Opcode);
   return InstInfo[opcode].NumSrcRegs;
}


/**
 * Return the number of dst registers for the given instruction/opcode.
 */
GLuint
_mesa_num_inst_dst_regs(enum prog_opcode opcode)
{
   assert(opcode < MAX_OPCODE);
   assert(opcode == InstInfo[opcode].Opcode);
   assert(OPCODE_XPD == InstInfo[OPCODE_XPD].Opcode);
   return InstInfo[opcode].NumDstRegs;
}


/**
 * Return string name for given program opcode.
 */
const char *
_mesa_opcode_string(enum prog_opcode opcode)
{
   if (opcode < MAX_OPCODE)
      return InstInfo[opcode].Name;
   else {
      static char s[20];
      snprintf(s, sizeof(s), "OP%u", opcode);
      return s;
   }
}

