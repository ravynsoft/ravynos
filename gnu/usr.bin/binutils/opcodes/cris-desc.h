/* CPU data header for cris.

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

#ifndef CRIS_CPU_H
#define CRIS_CPU_H

#ifdef __cplusplus
extern "C" {
#endif

#define CGEN_ARCH cris

/* Given symbol S, return cris_cgen_<S>.  */
#define CGEN_SYM(s) cris##_cgen_##s


/* Selected cpu families.  */
#define HAVE_CPU_CRISV0F
#define HAVE_CPU_CRISV3F
#define HAVE_CPU_CRISV8F
#define HAVE_CPU_CRISV10F
#define HAVE_CPU_CRISV32F

#define CGEN_INSN_LSB0_P 1

/* Minimum size of any insn (in bytes).  */
#define CGEN_MIN_INSN_SIZE 2

/* Maximum size of any insn (in bytes).  */
#define CGEN_MAX_INSN_SIZE 6

#define CGEN_INT_INSN_P 0

/* Maximum number of syntax elements in an instruction.  */
#define CGEN_ACTUAL_MAX_SYNTAX_ELEMENTS 22

/* CGEN_MNEMONIC_OPERANDS is defined if mnemonics have operands.
   e.g. In "b,a foo" the ",a" is an operand.  If mnemonics have operands
   we can't hash on everything up to the space.  */
#define CGEN_MNEMONIC_OPERANDS

/* Maximum number of fields in an instruction.  */
#define CGEN_ACTUAL_MAX_IFMT_OPERANDS 6

/* Enums.  */

/* Enum declaration for .  */
typedef enum gr_names_pcreg {
  H_GR_REAL_PC_PC = 15, H_GR_REAL_PC_SP = 14, H_GR_REAL_PC_R0 = 0, H_GR_REAL_PC_R1 = 1
 , H_GR_REAL_PC_R2 = 2, H_GR_REAL_PC_R3 = 3, H_GR_REAL_PC_R4 = 4, H_GR_REAL_PC_R5 = 5
 , H_GR_REAL_PC_R6 = 6, H_GR_REAL_PC_R7 = 7, H_GR_REAL_PC_R8 = 8, H_GR_REAL_PC_R9 = 9
 , H_GR_REAL_PC_R10 = 10, H_GR_REAL_PC_R11 = 11, H_GR_REAL_PC_R12 = 12, H_GR_REAL_PC_R13 = 13
 , H_GR_REAL_PC_R14 = 14
} GR_NAMES_PCREG;

/* Enum declaration for .  */
typedef enum gr_names_acr {
  H_GR_ACR = 15, H_GR_SP = 14, H_GR_R0 = 0, H_GR_R1 = 1
 , H_GR_R2 = 2, H_GR_R3 = 3, H_GR_R4 = 4, H_GR_R5 = 5
 , H_GR_R6 = 6, H_GR_R7 = 7, H_GR_R8 = 8, H_GR_R9 = 9
 , H_GR_R10 = 10, H_GR_R11 = 11, H_GR_R12 = 12, H_GR_R13 = 13
 , H_GR_R14 = 14
} GR_NAMES_ACR;

/* Enum declaration for .  */
typedef enum gr_names_v32 {
  H_GR_V32_ACR = 15, H_GR_V32_SP = 14, H_GR_V32_R0 = 0, H_GR_V32_R1 = 1
 , H_GR_V32_R2 = 2, H_GR_V32_R3 = 3, H_GR_V32_R4 = 4, H_GR_V32_R5 = 5
 , H_GR_V32_R6 = 6, H_GR_V32_R7 = 7, H_GR_V32_R8 = 8, H_GR_V32_R9 = 9
 , H_GR_V32_R10 = 10, H_GR_V32_R11 = 11, H_GR_V32_R12 = 12, H_GR_V32_R13 = 13
 , H_GR_V32_R14 = 14
} GR_NAMES_V32;

/* Enum declaration for .  */
typedef enum p_names_v10 {
  H_SR_PRE_V32_CCR = 5, H_SR_PRE_V32_MOF = 7, H_SR_PRE_V32_IBR = 9, H_SR_PRE_V32_IRP = 10
 , H_SR_PRE_V32_BAR = 12, H_SR_PRE_V32_DCCR = 13, H_SR_PRE_V32_BRP = 14, H_SR_PRE_V32_USP = 15
 , H_SR_PRE_V32_VR = 1, H_SR_PRE_V32_SRP = 11, H_SR_PRE_V32_P0 = 0, H_SR_PRE_V32_P1 = 1
 , H_SR_PRE_V32_P2 = 2, H_SR_PRE_V32_P3 = 3, H_SR_PRE_V32_P4 = 4, H_SR_PRE_V32_P5 = 5
 , H_SR_PRE_V32_P6 = 6, H_SR_PRE_V32_P7 = 7, H_SR_PRE_V32_P8 = 8, H_SR_PRE_V32_P9 = 9
 , H_SR_PRE_V32_P10 = 10, H_SR_PRE_V32_P11 = 11, H_SR_PRE_V32_P12 = 12, H_SR_PRE_V32_P13 = 13
 , H_SR_PRE_V32_P14 = 14
} P_NAMES_V10;

/* Enum declaration for .  */
typedef enum p_names_v32 {
  H_SR_BZ = 0, H_SR_PID = 2, H_SR_SRS = 3, H_SR_WZ = 4
 , H_SR_EXS = 5, H_SR_EDA = 6, H_SR_MOF = 7, H_SR_DZ = 8
 , H_SR_EBP = 9, H_SR_ERP = 10, H_SR_NRP = 12, H_SR_CCS = 13
 , H_SR_USP = 14, H_SR_SPC = 15, H_SR_VR = 1, H_SR_SRP = 11
 , H_SR_P0 = 0, H_SR_P1 = 1, H_SR_P2 = 2, H_SR_P3 = 3
 , H_SR_P4 = 4, H_SR_P5 = 5, H_SR_P6 = 6, H_SR_P7 = 7
 , H_SR_P8 = 8, H_SR_P9 = 9, H_SR_P10 = 10, H_SR_P11 = 11
 , H_SR_P12 = 12, H_SR_P13 = 13, H_SR_P14 = 14
} P_NAMES_V32;

/* Enum declaration for .  */
typedef enum p_names_v32_x {
  H_SR_V32_BZ = 0, H_SR_V32_PID = 2, H_SR_V32_SRS = 3, H_SR_V32_WZ = 4
 , H_SR_V32_EXS = 5, H_SR_V32_EDA = 6, H_SR_V32_MOF = 7, H_SR_V32_DZ = 8
 , H_SR_V32_EBP = 9, H_SR_V32_ERP = 10, H_SR_V32_NRP = 12, H_SR_V32_CCS = 13
 , H_SR_V32_USP = 14, H_SR_V32_SPC = 15, H_SR_V32_VR = 1, H_SR_V32_SRP = 11
 , H_SR_V32_P0 = 0, H_SR_V32_P1 = 1, H_SR_V32_P2 = 2, H_SR_V32_P3 = 3
 , H_SR_V32_P4 = 4, H_SR_V32_P5 = 5, H_SR_V32_P6 = 6, H_SR_V32_P7 = 7
 , H_SR_V32_P8 = 8, H_SR_V32_P9 = 9, H_SR_V32_P10 = 10, H_SR_V32_P11 = 11
 , H_SR_V32_P12 = 12, H_SR_V32_P13 = 13, H_SR_V32_P14 = 14
} P_NAMES_V32_X;

/* Enum declaration for Standard instruction operand size.  */
typedef enum insn_size {
  SIZE_BYTE, SIZE_WORD, SIZE_DWORD, SIZE_FIXED
} INSN_SIZE;

/* Enum declaration for Standard instruction addressing modes.  */
typedef enum insn_mode {
  MODE_QUICK_IMMEDIATE, MODE_REGISTER, MODE_INDIRECT, MODE_AUTOINCREMENT
} INSN_MODE;

/* Enum declaration for Whether the operand is indirect.  */
typedef enum insn_memoryness_mode {
  MODEMEMP_NO, MODEMEMP_YES
} INSN_MEMORYNESS_MODE;

/* Enum declaration for Whether the indirect operand is autoincrement.  */
typedef enum insn_memincness_mode {
  MODEINCP_NO, MODEINCP_YES
} INSN_MEMINCNESS_MODE;

/* Enum declaration for Signed instruction operand size.  */
typedef enum insn_signed_size {
  SIGNED_UNDEF_SIZE_0, SIGNED_UNDEF_SIZE_1, SIGNED_BYTE, SIGNED_WORD
} INSN_SIGNED_SIZE;

/* Enum declaration for Unsigned instruction operand size.  */
typedef enum insn_unsigned_size {
  UNSIGNED_BYTE, UNSIGNED_WORD, UNSIGNED_UNDEF_SIZE_2, UNSIGNED_UNDEF_SIZE_3
} INSN_UNSIGNED_SIZE;

/* Enum declaration for Insns for MODE_QUICK_IMMEDIATE.  */
typedef enum insn_qi_opc {
  Q_BCC_0, Q_BCC_1, Q_BCC_2, Q_BCC_3
 , Q_BDAP_0, Q_BDAP_1, Q_BDAP_2, Q_BDAP_3
 , Q_ADDQ, Q_MOVEQ, Q_SUBQ, Q_CMPQ
 , Q_ANDQ, Q_ORQ, Q_ASHQ, Q_LSHQ
} INSN_QI_OPC;

/* Enum declaration for Same as insn-qi-opc, though using only the high two bits of the opcode.  */
typedef enum insn_qihi_opc {
  QHI_BCC, QHI_BDAP, QHI_OTHER2, QHI_OTHER3
} INSN_QIHI_OPC;

/* Enum declaration for Insns for MODE_REGISTER and either SIZE_BYTE, SIZE_WORD or SIZE_DWORD.  */
typedef enum insn_r_opc {
  R_ADDX, R_MOVX, R_SUBX, R_LSL
 , R_ADDI, R_BIAP, R_NEG, R_BOUND
 , R_ADD, R_MOVE, R_SUB, R_CMP
 , R_AND, R_OR, R_ASR, R_LSR
} INSN_R_OPC;

/* Enum declaration for Insns for MODE_REGISTER and SIZE_FIXED.  */
typedef enum insn_rfix_opc {
  RFIX_ADDX, RFIX_MOVX, RFIX_SUBX, RFIX_BTST
 , RFIX_SCC, RFIX_ADDC, RFIX_SETF, RFIX_CLEARF
 , RFIX_MOVE_R_S, RFIX_MOVE_S_R, RFIX_ABS, RFIX_DSTEP
 , RFIX_LZ, RFIX_SWAP, RFIX_XOR, RFIX_MSTEP
} INSN_RFIX_OPC;

/* Enum declaration for Insns for (MODE_INDIRECT or MODE_AUTOINCREMENT) and either SIZE_BYTE, SIZE_WORD or SIZE_DWORD.  */
typedef enum insn_indir_opc {
  INDIR_ADDX, INDIR_MOVX, INDIR_SUBX, INDIR_CMPX
 , INDIR_MUL, INDIR_BDAP_M, INDIR_ADDC, INDIR_BOUND
 , INDIR_ADD, INDIR_MOVE_M_R, INDIR_SUB, INDIR_CMP
 , INDIR_AND, INDIR_OR, INDIR_TEST, INDIR_MOVE_R_M
} INSN_INDIR_OPC;

/* Enum declaration for Insns for (MODE_INDIRECT or MODE_AUTOINCREMENT) and SIZE_FIXED.  */
typedef enum insn_infix_opc {
  INFIX_ADDX, INFIX_MOVX, INFIX_SUBX, INFIX_CMPX
 , INFIX_JUMP_M, INFIX_DIP, INFIX_JUMP_R, INFIX_BCC_M
 , INFIX_MOVE_M_S, INFIX_MOVE_S_M, INFIX_BMOD, INFIX_BSTORE
 , INFIX_RBF, INFIX_SBFS, INFIX_MOVEM_M_R, INFIX_MOVEM_R_M
} INSN_INFIX_OPC;

/* Attributes.  */

/* Enum declaration for machine type selection.  */
typedef enum mach_attr {
  MACH_BASE, MACH_CRISV0, MACH_CRISV3, MACH_CRISV8
 , MACH_CRISV10, MACH_CRISV32, MACH_MAX
} MACH_ATTR;

/* Enum declaration for instruction set selection.  */
typedef enum isa_attr {
  ISA_CRIS, ISA_MAX
} ISA_ATTR;

/* Number of architecture variants.  */
#define MAX_ISAS  1
#define MAX_MACHS ((int) MACH_MAX)

/* Ifield support.  */

/* Ifield attribute indices.  */

/* Enum declaration for cgen_ifld attrs.  */
typedef enum cgen_ifld_attr {
  CGEN_IFLD_VIRTUAL, CGEN_IFLD_PCREL_ADDR, CGEN_IFLD_ABS_ADDR, CGEN_IFLD_RESERVED
 , CGEN_IFLD_SIGN_OPT, CGEN_IFLD_SIGNED, CGEN_IFLD_END_BOOLS, CGEN_IFLD_START_NBOOLS = 31
 , CGEN_IFLD_MACH, CGEN_IFLD_END_NBOOLS
} CGEN_IFLD_ATTR;

/* Number of non-boolean elements in cgen_ifld_attr.  */
#define CGEN_IFLD_NBOOL_ATTRS (CGEN_IFLD_END_NBOOLS - CGEN_IFLD_START_NBOOLS - 1)

/* cgen_ifld attribute accessor macros.  */
#define CGEN_ATTR_CGEN_IFLD_MACH_VALUE(attrs) ((attrs)->nonbool[CGEN_IFLD_MACH-CGEN_IFLD_START_NBOOLS-1].nonbitset)
#define CGEN_ATTR_CGEN_IFLD_VIRTUAL_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_IFLD_VIRTUAL)) != 0)
#define CGEN_ATTR_CGEN_IFLD_PCREL_ADDR_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_IFLD_PCREL_ADDR)) != 0)
#define CGEN_ATTR_CGEN_IFLD_ABS_ADDR_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_IFLD_ABS_ADDR)) != 0)
#define CGEN_ATTR_CGEN_IFLD_RESERVED_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_IFLD_RESERVED)) != 0)
#define CGEN_ATTR_CGEN_IFLD_SIGN_OPT_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_IFLD_SIGN_OPT)) != 0)
#define CGEN_ATTR_CGEN_IFLD_SIGNED_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_IFLD_SIGNED)) != 0)

/* Enum declaration for cris ifield types.  */
typedef enum ifield_type {
  CRIS_F_NIL, CRIS_F_ANYOF, CRIS_F_OPERAND1, CRIS_F_SIZE
 , CRIS_F_OPCODE, CRIS_F_MODE, CRIS_F_OPERAND2, CRIS_F_MEMMODE
 , CRIS_F_MEMBIT, CRIS_F_B5, CRIS_F_OPCODE_HI, CRIS_F_DSTSRC
 , CRIS_F_U6, CRIS_F_S6, CRIS_F_U5, CRIS_F_U4
 , CRIS_F_S8, CRIS_F_DISP9_HI, CRIS_F_DISP9_LO, CRIS_F_DISP9
 , CRIS_F_QO, CRIS_F_INDIR_PC__BYTE, CRIS_F_INDIR_PC__WORD, CRIS_F_INDIR_PC__WORD_PCREL
 , CRIS_F_INDIR_PC__DWORD, CRIS_F_INDIR_PC__DWORD_PCREL, CRIS_F_MAX
} IFIELD_TYPE;

#define MAX_IFLD ((int) CRIS_F_MAX)

/* Hardware attribute indices.  */

/* Enum declaration for cgen_hw attrs.  */
typedef enum cgen_hw_attr {
  CGEN_HW_VIRTUAL, CGEN_HW_CACHE_ADDR, CGEN_HW_PC, CGEN_HW_PROFILE
 , CGEN_HW_END_BOOLS, CGEN_HW_START_NBOOLS = 31, CGEN_HW_MACH, CGEN_HW_END_NBOOLS
} CGEN_HW_ATTR;

/* Number of non-boolean elements in cgen_hw_attr.  */
#define CGEN_HW_NBOOL_ATTRS (CGEN_HW_END_NBOOLS - CGEN_HW_START_NBOOLS - 1)

/* cgen_hw attribute accessor macros.  */
#define CGEN_ATTR_CGEN_HW_MACH_VALUE(attrs) ((attrs)->nonbool[CGEN_HW_MACH-CGEN_HW_START_NBOOLS-1].nonbitset)
#define CGEN_ATTR_CGEN_HW_VIRTUAL_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_HW_VIRTUAL)) != 0)
#define CGEN_ATTR_CGEN_HW_CACHE_ADDR_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_HW_CACHE_ADDR)) != 0)
#define CGEN_ATTR_CGEN_HW_PC_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_HW_PC)) != 0)
#define CGEN_ATTR_CGEN_HW_PROFILE_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_HW_PROFILE)) != 0)

/* Enum declaration for cris hardware types.  */
typedef enum cgen_hw_type {
  HW_H_MEMORY, HW_H_SINT, HW_H_UINT, HW_H_ADDR
 , HW_H_IADDR, HW_H_INC, HW_H_CCODE, HW_H_SWAP
 , HW_H_FLAGBITS, HW_H_V32, HW_H_PC, HW_H_GR
 , HW_H_GR_X, HW_H_GR_REAL_PC, HW_H_RAW_GR, HW_H_SR
 , HW_H_SR_X, HW_H_SUPR, HW_H_CBIT, HW_H_CBIT_MOVE
 , HW_H_CBIT_MOVE_X, HW_H_VBIT, HW_H_VBIT_MOVE, HW_H_VBIT_MOVE_X
 , HW_H_ZBIT, HW_H_ZBIT_MOVE, HW_H_ZBIT_MOVE_X, HW_H_NBIT
 , HW_H_NBIT_MOVE, HW_H_NBIT_MOVE_X, HW_H_XBIT, HW_H_IBIT
 , HW_H_IBIT_X, HW_H_PBIT, HW_H_RBIT, HW_H_UBIT
 , HW_H_UBIT_X, HW_H_GBIT, HW_H_KERNEL_SP, HW_H_MBIT
 , HW_H_QBIT, HW_H_SBIT, HW_H_INSN_PREFIXED_P, HW_H_INSN_PREFIXED_P_X
 , HW_H_PREFIXREG, HW_MAX
} CGEN_HW_TYPE;

#define MAX_HW ((int) HW_MAX)

/* Operand attribute indices.  */

/* Enum declaration for cgen_operand attrs.  */
typedef enum cgen_operand_attr {
  CGEN_OPERAND_VIRTUAL, CGEN_OPERAND_PCREL_ADDR, CGEN_OPERAND_ABS_ADDR, CGEN_OPERAND_SIGN_OPT
 , CGEN_OPERAND_SIGNED, CGEN_OPERAND_NEGATIVE, CGEN_OPERAND_RELAX, CGEN_OPERAND_SEM_ONLY
 , CGEN_OPERAND_END_BOOLS, CGEN_OPERAND_START_NBOOLS = 31, CGEN_OPERAND_MACH, CGEN_OPERAND_END_NBOOLS
} CGEN_OPERAND_ATTR;

/* Number of non-boolean elements in cgen_operand_attr.  */
#define CGEN_OPERAND_NBOOL_ATTRS (CGEN_OPERAND_END_NBOOLS - CGEN_OPERAND_START_NBOOLS - 1)

/* cgen_operand attribute accessor macros.  */
#define CGEN_ATTR_CGEN_OPERAND_MACH_VALUE(attrs) ((attrs)->nonbool[CGEN_OPERAND_MACH-CGEN_OPERAND_START_NBOOLS-1].nonbitset)
#define CGEN_ATTR_CGEN_OPERAND_VIRTUAL_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_OPERAND_VIRTUAL)) != 0)
#define CGEN_ATTR_CGEN_OPERAND_PCREL_ADDR_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_OPERAND_PCREL_ADDR)) != 0)
#define CGEN_ATTR_CGEN_OPERAND_ABS_ADDR_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_OPERAND_ABS_ADDR)) != 0)
#define CGEN_ATTR_CGEN_OPERAND_SIGN_OPT_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_OPERAND_SIGN_OPT)) != 0)
#define CGEN_ATTR_CGEN_OPERAND_SIGNED_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_OPERAND_SIGNED)) != 0)
#define CGEN_ATTR_CGEN_OPERAND_NEGATIVE_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_OPERAND_NEGATIVE)) != 0)
#define CGEN_ATTR_CGEN_OPERAND_RELAX_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_OPERAND_RELAX)) != 0)
#define CGEN_ATTR_CGEN_OPERAND_SEM_ONLY_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_OPERAND_SEM_ONLY)) != 0)

/* Enum declaration for cris operand types.  */
typedef enum cgen_operand_type {
  CRIS_OPERAND_PC, CRIS_OPERAND_CBIT, CRIS_OPERAND_CBIT_MOVE, CRIS_OPERAND_VBIT
 , CRIS_OPERAND_VBIT_MOVE, CRIS_OPERAND_ZBIT, CRIS_OPERAND_ZBIT_MOVE, CRIS_OPERAND_NBIT
 , CRIS_OPERAND_NBIT_MOVE, CRIS_OPERAND_XBIT, CRIS_OPERAND_IBIT, CRIS_OPERAND_UBIT
 , CRIS_OPERAND_PBIT, CRIS_OPERAND_RBIT, CRIS_OPERAND_SBIT, CRIS_OPERAND_MBIT
 , CRIS_OPERAND_QBIT, CRIS_OPERAND_PREFIX_SET, CRIS_OPERAND_PREFIXREG, CRIS_OPERAND_RS
 , CRIS_OPERAND_INC, CRIS_OPERAND_PS, CRIS_OPERAND_SS, CRIS_OPERAND_SD
 , CRIS_OPERAND_I, CRIS_OPERAND_J, CRIS_OPERAND_C, CRIS_OPERAND_QO
 , CRIS_OPERAND_RD, CRIS_OPERAND_SCONST8, CRIS_OPERAND_UCONST8, CRIS_OPERAND_SCONST16
 , CRIS_OPERAND_UCONST16, CRIS_OPERAND_CONST32, CRIS_OPERAND_CONST32_PCREL, CRIS_OPERAND_PD
 , CRIS_OPERAND_O, CRIS_OPERAND_O_PCREL, CRIS_OPERAND_O_WORD_PCREL, CRIS_OPERAND_CC
 , CRIS_OPERAND_N, CRIS_OPERAND_SWAPOPTION, CRIS_OPERAND_LIST_OF_FLAGS, CRIS_OPERAND_MAX
} CGEN_OPERAND_TYPE;

/* Number of operands types.  */
#define MAX_OPERANDS 43

/* Maximum number of operands referenced by any insn.  */
#define MAX_OPERAND_INSTANCES 8

/* Insn attribute indices.  */

/* Enum declaration for cgen_insn attrs.  */
typedef enum cgen_insn_attr {
  CGEN_INSN_ALIAS, CGEN_INSN_VIRTUAL, CGEN_INSN_UNCOND_CTI, CGEN_INSN_COND_CTI
 , CGEN_INSN_SKIP_CTI, CGEN_INSN_DELAY_SLOT, CGEN_INSN_RELAXABLE, CGEN_INSN_RELAXED
 , CGEN_INSN_NO_DIS, CGEN_INSN_PBB, CGEN_INSN_END_BOOLS, CGEN_INSN_START_NBOOLS = 31
 , CGEN_INSN_MACH, CGEN_INSN_END_NBOOLS
} CGEN_INSN_ATTR;

/* Number of non-boolean elements in cgen_insn_attr.  */
#define CGEN_INSN_NBOOL_ATTRS (CGEN_INSN_END_NBOOLS - CGEN_INSN_START_NBOOLS - 1)

/* cgen_insn attribute accessor macros.  */
#define CGEN_ATTR_CGEN_INSN_MACH_VALUE(attrs) ((attrs)->nonbool[CGEN_INSN_MACH-CGEN_INSN_START_NBOOLS-1].nonbitset)
#define CGEN_ATTR_CGEN_INSN_ALIAS_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_ALIAS)) != 0)
#define CGEN_ATTR_CGEN_INSN_VIRTUAL_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_VIRTUAL)) != 0)
#define CGEN_ATTR_CGEN_INSN_UNCOND_CTI_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_UNCOND_CTI)) != 0)
#define CGEN_ATTR_CGEN_INSN_COND_CTI_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_COND_CTI)) != 0)
#define CGEN_ATTR_CGEN_INSN_SKIP_CTI_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_SKIP_CTI)) != 0)
#define CGEN_ATTR_CGEN_INSN_DELAY_SLOT_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_DELAY_SLOT)) != 0)
#define CGEN_ATTR_CGEN_INSN_RELAXABLE_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_RELAXABLE)) != 0)
#define CGEN_ATTR_CGEN_INSN_RELAXED_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_RELAXED)) != 0)
#define CGEN_ATTR_CGEN_INSN_NO_DIS_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_NO_DIS)) != 0)
#define CGEN_ATTR_CGEN_INSN_PBB_VALUE(attrs) (((attrs)->bool_ & (1 << CGEN_INSN_PBB)) != 0)

/* cgen.h uses things we just defined.  */
#include "opcode/cgen.h"

extern const struct cgen_ifld cris_cgen_ifld_table[];

/* Attributes.  */
extern const CGEN_ATTR_TABLE cris_cgen_hardware_attr_table[];
extern const CGEN_ATTR_TABLE cris_cgen_ifield_attr_table[];
extern const CGEN_ATTR_TABLE cris_cgen_operand_attr_table[];
extern const CGEN_ATTR_TABLE cris_cgen_insn_attr_table[];

/* Hardware decls.  */

extern CGEN_KEYWORD cris_cgen_opval_h_inc;
extern CGEN_KEYWORD cris_cgen_opval_h_ccode;
extern CGEN_KEYWORD cris_cgen_opval_h_swap;
extern CGEN_KEYWORD cris_cgen_opval_h_flagbits;
extern CGEN_KEYWORD cris_cgen_opval_gr_names_pcreg;
extern CGEN_KEYWORD cris_cgen_opval_gr_names_pcreg;
extern CGEN_KEYWORD cris_cgen_opval_gr_names_acr;
extern CGEN_KEYWORD cris_cgen_opval_p_names_v10;
extern CGEN_KEYWORD cris_cgen_opval_p_names_v10;
extern CGEN_KEYWORD cris_cgen_opval_p_names_v10;
extern CGEN_KEYWORD cris_cgen_opval_p_names_v10;
extern CGEN_KEYWORD cris_cgen_opval_p_names_v32;
extern CGEN_KEYWORD cris_cgen_opval_h_supr;

extern const CGEN_HW_ENTRY cris_cgen_hw_table[];



   #ifdef __cplusplus
   }
   #endif

#endif /* CRIS_CPU_H */
