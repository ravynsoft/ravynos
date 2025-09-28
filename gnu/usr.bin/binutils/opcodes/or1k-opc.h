/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Instruction opcode header for or1k.

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

#ifndef OR1K_OPC_H
#define OR1K_OPC_H

#ifdef __cplusplus
extern "C" {
#endif

/* -- opc.h */

#undef  CGEN_DIS_HASH_SIZE
#define CGEN_DIS_HASH_SIZE 256
#undef  CGEN_DIS_HASH
#define CGEN_DIS_HASH(buffer, value) (((unsigned char *) (buffer))[0] >> 2)

/* Check applicability of instructions against machines.  */
#define CGEN_VALIDATE_INSN_SUPPORTED

extern int or1k_cgen_insn_supported (CGEN_CPU_DESC, const CGEN_INSN *);

/* -- */
/* Enum declaration for or1k instruction types.  */
typedef enum cgen_insn_type {
  OR1K_INSN_INVALID, OR1K_INSN_L_J, OR1K_INSN_L_ADRP, OR1K_INSN_L_JAL
 , OR1K_INSN_L_JR, OR1K_INSN_L_JALR, OR1K_INSN_L_BNF, OR1K_INSN_L_BF
 , OR1K_INSN_L_TRAP, OR1K_INSN_L_SYS, OR1K_INSN_L_MSYNC, OR1K_INSN_L_PSYNC
 , OR1K_INSN_L_CSYNC, OR1K_INSN_L_RFE, OR1K_INSN_L_NOP_IMM, OR1K_INSN_L_NOP
 , OR1K_INSN_L_MOVHI, OR1K_INSN_L_MACRC, OR1K_INSN_L_MFSPR, OR1K_INSN_L_MTSPR
 , OR1K_INSN_L_LWZ, OR1K_INSN_L_LWS, OR1K_INSN_L_LWA, OR1K_INSN_L_LBZ
 , OR1K_INSN_L_LBS, OR1K_INSN_L_LHZ, OR1K_INSN_L_LHS, OR1K_INSN_L_SW
 , OR1K_INSN_L_SB, OR1K_INSN_L_SH, OR1K_INSN_L_SWA, OR1K_INSN_L_SLL
 , OR1K_INSN_L_SLLI, OR1K_INSN_L_SRL, OR1K_INSN_L_SRLI, OR1K_INSN_L_SRA
 , OR1K_INSN_L_SRAI, OR1K_INSN_L_ROR, OR1K_INSN_L_RORI, OR1K_INSN_L_AND
 , OR1K_INSN_L_OR, OR1K_INSN_L_XOR, OR1K_INSN_L_ADD, OR1K_INSN_L_SUB
 , OR1K_INSN_L_ADDC, OR1K_INSN_L_MUL, OR1K_INSN_L_MULD, OR1K_INSN_L_MULU
 , OR1K_INSN_L_MULDU, OR1K_INSN_L_DIV, OR1K_INSN_L_DIVU, OR1K_INSN_L_FF1
 , OR1K_INSN_L_FL1, OR1K_INSN_L_ANDI, OR1K_INSN_L_ORI, OR1K_INSN_L_XORI
 , OR1K_INSN_L_ADDI, OR1K_INSN_L_ADDIC, OR1K_INSN_L_MULI, OR1K_INSN_L_EXTHS
 , OR1K_INSN_L_EXTBS, OR1K_INSN_L_EXTHZ, OR1K_INSN_L_EXTBZ, OR1K_INSN_L_EXTWS
 , OR1K_INSN_L_EXTWZ, OR1K_INSN_L_CMOV, OR1K_INSN_L_SFGTS, OR1K_INSN_L_SFGTSI
 , OR1K_INSN_L_SFGTU, OR1K_INSN_L_SFGTUI, OR1K_INSN_L_SFGES, OR1K_INSN_L_SFGESI
 , OR1K_INSN_L_SFGEU, OR1K_INSN_L_SFGEUI, OR1K_INSN_L_SFLTS, OR1K_INSN_L_SFLTSI
 , OR1K_INSN_L_SFLTU, OR1K_INSN_L_SFLTUI, OR1K_INSN_L_SFLES, OR1K_INSN_L_SFLESI
 , OR1K_INSN_L_SFLEU, OR1K_INSN_L_SFLEUI, OR1K_INSN_L_SFEQ, OR1K_INSN_L_SFEQI
 , OR1K_INSN_L_SFNE, OR1K_INSN_L_SFNEI, OR1K_INSN_L_MAC, OR1K_INSN_L_MACI
 , OR1K_INSN_L_MACU, OR1K_INSN_L_MSB, OR1K_INSN_L_MSBU, OR1K_INSN_L_CUST1
 , OR1K_INSN_L_CUST2, OR1K_INSN_L_CUST3, OR1K_INSN_L_CUST4, OR1K_INSN_L_CUST5
 , OR1K_INSN_L_CUST6, OR1K_INSN_L_CUST7, OR1K_INSN_L_CUST8, OR1K_INSN_LF_ADD_S
 , OR1K_INSN_LF_ADD_D32, OR1K_INSN_LF_SUB_S, OR1K_INSN_LF_SUB_D32, OR1K_INSN_LF_MUL_S
 , OR1K_INSN_LF_MUL_D32, OR1K_INSN_LF_DIV_S, OR1K_INSN_LF_DIV_D32, OR1K_INSN_LF_REM_S
 , OR1K_INSN_LF_REM_D32, OR1K_INSN_LF_ITOF_S, OR1K_INSN_LF_ITOF_D32, OR1K_INSN_LF_FTOI_S
 , OR1K_INSN_LF_FTOI_D32, OR1K_INSN_LF_SFEQ_S, OR1K_INSN_LF_SFEQ_D32, OR1K_INSN_LF_SFNE_S
 , OR1K_INSN_LF_SFNE_D32, OR1K_INSN_LF_SFGE_S, OR1K_INSN_LF_SFGE_D32, OR1K_INSN_LF_SFGT_S
 , OR1K_INSN_LF_SFGT_D32, OR1K_INSN_LF_SFLT_S, OR1K_INSN_LF_SFLT_D32, OR1K_INSN_LF_SFLE_S
 , OR1K_INSN_LF_SFLE_D32, OR1K_INSN_LF_SFUEQ_S, OR1K_INSN_LF_SFUEQ_D32, OR1K_INSN_LF_SFUNE_S
 , OR1K_INSN_LF_SFUNE_D32, OR1K_INSN_LF_SFUGT_S, OR1K_INSN_LF_SFUGT_D32, OR1K_INSN_LF_SFUGE_S
 , OR1K_INSN_LF_SFUGE_D32, OR1K_INSN_LF_SFULT_S, OR1K_INSN_LF_SFULT_D32, OR1K_INSN_LF_SFULE_S
 , OR1K_INSN_LF_SFULE_D32, OR1K_INSN_LF_SFUN_S, OR1K_INSN_LF_SFUN_D32, OR1K_INSN_LF_MADD_S
 , OR1K_INSN_LF_MADD_D32, OR1K_INSN_LF_CUST1_S, OR1K_INSN_LF_CUST1_D32
} CGEN_INSN_TYPE;

/* Index of `invalid' insn place holder.  */
#define CGEN_INSN_INVALID OR1K_INSN_INVALID

/* Total number of insns in table.  */
#define MAX_INSNS ((int) OR1K_INSN_LF_CUST1_D32 + 1)

/* This struct records data prior to insertion or after extraction.  */
struct cgen_fields
{
  int length;
  long f_nil;
  long f_anyof;
  long f_opcode;
  long f_r1;
  long f_r2;
  long f_r3;
  long f_op_25_2;
  long f_op_25_5;
  long f_op_16_1;
  long f_op_7_4;
  long f_op_3_4;
  long f_op_9_2;
  long f_op_9_4;
  long f_op_7_8;
  long f_op_7_2;
  long f_resv_25_26;
  long f_resv_25_10;
  long f_resv_25_5;
  long f_resv_23_8;
  long f_resv_20_21;
  long f_resv_20_5;
  long f_resv_20_4;
  long f_resv_15_8;
  long f_resv_15_6;
  long f_resv_10_11;
  long f_resv_10_7;
  long f_resv_10_3;
  long f_resv_10_1;
  long f_resv_8_1;
  long f_resv_7_4;
  long f_resv_5_2;
  long f_imm16_25_5;
  long f_imm16_10_11;
  long f_disp26;
  long f_disp21;
  long f_uimm16;
  long f_simm16;
  long f_uimm6;
  long f_uimm16_split;
  long f_simm16_split;
  long f_rdoff_10_1;
  long f_raoff_9_1;
  long f_rboff_8_1;
  long f_rdd32;
  long f_rad32;
  long f_rbd32;
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

#endif /* OR1K_OPC_H */
