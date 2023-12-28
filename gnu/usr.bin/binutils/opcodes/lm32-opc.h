/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Instruction opcode header for lm32.

THIS FILE IS MACHINE GENERATED WITH CGEN.

Copyright (C) 1996-2023 Free Software Foundation, Inc.

This file is part of the GNU Binutils and/or GDB, the GNU debugger.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.

*/

#ifndef LM32_OPC_H
#define LM32_OPC_H

#ifdef __cplusplus
extern "C" {
#endif

/* -- opc.h */

/* Allows reason codes to be output when assembler errors occur.  */
#define CGEN_VERBOSE_ASSEMBLER_ERRORS

#define CGEN_DIS_HASH_SIZE 64
#define CGEN_DIS_HASH(buf,value) ((value >> 26) & 0x3f)

/* -- asm.c */
/* Enum declaration for lm32 instruction types.  */
typedef enum cgen_insn_type {
  LM32_INSN_INVALID, LM32_INSN_ADD, LM32_INSN_ADDI, LM32_INSN_AND
 , LM32_INSN_ANDI, LM32_INSN_ANDHII, LM32_INSN_B, LM32_INSN_BI
 , LM32_INSN_BE, LM32_INSN_BG, LM32_INSN_BGE, LM32_INSN_BGEU
 , LM32_INSN_BGU, LM32_INSN_BNE, LM32_INSN_CALL, LM32_INSN_CALLI
 , LM32_INSN_CMPE, LM32_INSN_CMPEI, LM32_INSN_CMPG, LM32_INSN_CMPGI
 , LM32_INSN_CMPGE, LM32_INSN_CMPGEI, LM32_INSN_CMPGEU, LM32_INSN_CMPGEUI
 , LM32_INSN_CMPGU, LM32_INSN_CMPGUI, LM32_INSN_CMPNE, LM32_INSN_CMPNEI
 , LM32_INSN_DIVU, LM32_INSN_LB, LM32_INSN_LBU, LM32_INSN_LH
 , LM32_INSN_LHU, LM32_INSN_LW, LM32_INSN_MODU, LM32_INSN_MUL
 , LM32_INSN_MULI, LM32_INSN_NOR, LM32_INSN_NORI, LM32_INSN_OR
 , LM32_INSN_ORI, LM32_INSN_ORHII, LM32_INSN_RCSR, LM32_INSN_SB
 , LM32_INSN_SEXTB, LM32_INSN_SEXTH, LM32_INSN_SH, LM32_INSN_SL
 , LM32_INSN_SLI, LM32_INSN_SR, LM32_INSN_SRI, LM32_INSN_SRU
 , LM32_INSN_SRUI, LM32_INSN_SUB, LM32_INSN_SW, LM32_INSN_USER
 , LM32_INSN_WCSR, LM32_INSN_XOR, LM32_INSN_XORI, LM32_INSN_XNOR
 , LM32_INSN_XNORI, LM32_INSN_BREAK, LM32_INSN_SCALL, LM32_INSN_BRET
 , LM32_INSN_ERET, LM32_INSN_RET, LM32_INSN_MV, LM32_INSN_MVI
 , LM32_INSN_MVUI, LM32_INSN_MVHI, LM32_INSN_MVA, LM32_INSN_NOT
 , LM32_INSN_NOP, LM32_INSN_LBGPREL, LM32_INSN_LBUGPREL, LM32_INSN_LHGPREL
 , LM32_INSN_LHUGPREL, LM32_INSN_LWGPREL, LM32_INSN_SBGPREL, LM32_INSN_SHGPREL
 , LM32_INSN_SWGPREL, LM32_INSN_LWGOTREL, LM32_INSN_ORHIGOTOFFI, LM32_INSN_ADDGOTOFF
 , LM32_INSN_SWGOTOFF, LM32_INSN_LWGOTOFF, LM32_INSN_SHGOTOFF, LM32_INSN_LHGOTOFF
 , LM32_INSN_LHUGOTOFF, LM32_INSN_SBGOTOFF, LM32_INSN_LBGOTOFF, LM32_INSN_LBUGOTOFF
} CGEN_INSN_TYPE;

/* Index of `invalid' insn place holder.  */
#define CGEN_INSN_INVALID LM32_INSN_INVALID

/* Total number of insns in table.  */
#define MAX_INSNS ((int) LM32_INSN_LBUGOTOFF + 1)

/* This struct records data prior to insertion or after extraction.  */
struct cgen_fields
{
  int length;
  long f_nil;
  long f_anyof;
  long f_opcode;
  long f_r0;
  long f_r1;
  long f_r2;
  long f_resv0;
  long f_shift;
  long f_imm;
  long f_uimm;
  long f_csr;
  long f_user;
  long f_exception;
  long f_branch;
  long f_call;
};

#define CGEN_INIT_PARSE(od) \
{\
}
#define CGEN_INIT_INSERT(od) \
{\
}
#define CGEN_INIT_EXTRACT(od) \
{\
}
#define CGEN_INIT_PRINT(od) \
{\
}


   #ifdef __cplusplus
   }
   #endif

#endif /* LM32_OPC_H */
