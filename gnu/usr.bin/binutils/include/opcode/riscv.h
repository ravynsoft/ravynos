/* riscv.h.  RISC-V opcode list for GDB, the GNU debugger.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Contributed by Andrew Waterman

   This file is part of GDB, GAS, and the GNU binutils.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version
   3, or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef _RISCV_H_
#define _RISCV_H_

#include "riscv-opc.h"
#include <stdlib.h>
#include <stdint.h>

typedef uint64_t insn_t;

static inline unsigned int riscv_insn_length (insn_t insn)
{
  if ((insn & 0x3) != 0x3) /* RVC instructions.  */
    return 2;
  if ((insn & 0x1f) != 0x1f) /* 32-bit instructions.  */
    return 4;
  if ((insn & 0x3f) == 0x1f) /* 48-bit instructions.  */
    return 6;
  if ((insn & 0x7f) == 0x3f) /* 64-bit instructions.  */
    return 8;
  /* 80- ... 176-bit instructions.  */
  if ((insn & 0x7f) == 0x7f && (insn & 0x7000) != 0x7000)
    return 10 + ((insn >> 11) & 0xe);
  /* Maximum value returned by this function.  */
#define RISCV_MAX_INSN_LEN 22
  /* Longer instructions not supported at the moment.  */
  return 2;
}

#define RVC_JUMP_BITS 11
#define RVC_JUMP_REACH ((1ULL << RVC_JUMP_BITS) * RISCV_JUMP_ALIGN)

#define RVC_BRANCH_BITS 8
#define RVC_BRANCH_REACH ((1ULL << RVC_BRANCH_BITS) * RISCV_BRANCH_ALIGN)

#define RV_X(x, s, n)  (((x) >> (s)) & ((1 << (n)) - 1))
#define RV_IMM_SIGN(x) (-(((x) >> 31) & 1))
#define RV_X_SIGNED(x, s, n) (RV_X(x, s, n) | ((-(RV_X(x, (s + n - 1), 1))) << (n)))

#define EXTRACT_ITYPE_IMM(x) \
  (RV_X(x, 20, 12) | (RV_IMM_SIGN(x) << 12))
#define EXTRACT_STYPE_IMM(x) \
  (RV_X(x, 7, 5) | (RV_X(x, 25, 7) << 5) | (RV_IMM_SIGN(x) << 12))
#define EXTRACT_BTYPE_IMM(x) \
  ((RV_X(x, 8, 4) << 1) | (RV_X(x, 25, 6) << 5) | (RV_X(x, 7, 1) << 11) | (RV_IMM_SIGN(x) << 12))
#define EXTRACT_UTYPE_IMM(x) \
  ((RV_X(x, 12, 20) << 12) | (RV_IMM_SIGN(x) << 32))
#define EXTRACT_JTYPE_IMM(x) \
  ((RV_X(x, 21, 10) << 1) | (RV_X(x, 20, 1) << 11) | (RV_X(x, 12, 8) << 12) | (RV_IMM_SIGN(x) << 20))
#define EXTRACT_CITYPE_IMM(x) \
  (RV_X(x, 2, 5) | (-RV_X(x, 12, 1) << 5))
#define EXTRACT_CITYPE_LUI_IMM(x) \
  (EXTRACT_CITYPE_IMM (x) << RISCV_IMM_BITS)
#define EXTRACT_CITYPE_ADDI16SP_IMM(x) \
  ((RV_X(x, 6, 1) << 4) | (RV_X(x, 2, 1) << 5) | (RV_X(x, 5, 1) << 6) | (RV_X(x, 3, 2) << 7) | (-RV_X(x, 12, 1) << 9))
#define EXTRACT_CITYPE_LWSP_IMM(x) \
  ((RV_X(x, 4, 3) << 2) | (RV_X(x, 12, 1) << 5) | (RV_X(x, 2, 2) << 6))
#define EXTRACT_CITYPE_LDSP_IMM(x) \
  ((RV_X(x, 5, 2) << 3) | (RV_X(x, 12, 1) << 5) | (RV_X(x, 2, 3) << 6))
#define EXTRACT_CSSTYPE_IMM(x) \
  (RV_X(x, 7, 6) << 0)
#define EXTRACT_CSSTYPE_SWSP_IMM(x) \
  ((RV_X(x, 9, 4) << 2) | (RV_X(x, 7, 2) << 6))
#define EXTRACT_CSSTYPE_SDSP_IMM(x) \
  ((RV_X(x, 10, 3) << 3) | (RV_X(x, 7, 3) << 6))
#define EXTRACT_CIWTYPE_IMM(x) \
  (RV_X(x, 5, 8))
#define EXTRACT_CIWTYPE_ADDI4SPN_IMM(x) \
  ((RV_X(x, 6, 1) << 2) | (RV_X(x, 5, 1) << 3) | (RV_X(x, 11, 2) << 4) | (RV_X(x, 7, 4) << 6))
#define EXTRACT_CLTYPE_IMM(x) \
  ((RV_X(x, 5, 2) << 0) | (RV_X(x, 10, 3) << 2))
#define EXTRACT_CLTYPE_LW_IMM(x) \
  ((RV_X(x, 6, 1) << 2) | (RV_X(x, 10, 3) << 3) | (RV_X(x, 5, 1) << 6))
#define EXTRACT_CLTYPE_LD_IMM(x) \
  ((RV_X(x, 10, 3) << 3) | (RV_X(x, 5, 2) << 6))
#define EXTRACT_CBTYPE_IMM(x) \
  ((RV_X(x, 3, 2) << 1) | (RV_X(x, 10, 2) << 3) | (RV_X(x, 2, 1) << 5) | (RV_X(x, 5, 2) << 6) | (-RV_X(x, 12, 1) << 8))
#define EXTRACT_CJTYPE_IMM(x) \
  ((RV_X(x, 3, 3) << 1) | (RV_X(x, 11, 1) << 4) | (RV_X(x, 2, 1) << 5) | (RV_X(x, 7, 1) << 6) | (RV_X(x, 6, 1) << 7) | (RV_X(x, 9, 2) << 8) | (RV_X(x, 8, 1) << 10) | (-RV_X(x, 12, 1) << 11))
#define EXTRACT_RVV_VI_IMM(x) \
  (RV_X(x, 15, 5) | (-RV_X(x, 19, 1) << 5))
#define EXTRACT_RVV_VI_UIMM(x) \
  (RV_X(x, 15, 5))
#define EXTRACT_RVV_VI_UIMM6(x) \
  (RV_X(x, 15, 5) | (RV_X(x, 26, 1) << 5))
#define EXTRACT_RVV_OFFSET(x) \
  (RV_X(x, 29, 3))
#define EXTRACT_RVV_VB_IMM(x) \
  (RV_X(x, 20, 10))
#define EXTRACT_RVV_VC_IMM(x) \
  (RV_X(x, 20, 11))

#define ENCODE_ITYPE_IMM(x) \
  (RV_X(x, 0, 12) << 20)
#define ENCODE_STYPE_IMM(x) \
  ((RV_X(x, 0, 5) << 7) | (RV_X(x, 5, 7) << 25))
#define ENCODE_BTYPE_IMM(x) \
  ((RV_X(x, 1, 4) << 8) | (RV_X(x, 5, 6) << 25) | (RV_X(x, 11, 1) << 7) | (RV_X(x, 12, 1) << 31))
#define ENCODE_UTYPE_IMM(x) \
  (RV_X(x, 12, 20) << 12)
#define ENCODE_JTYPE_IMM(x) \
  ((RV_X(x, 1, 10) << 21) | (RV_X(x, 11, 1) << 20) | (RV_X(x, 12, 8) << 12) | (RV_X(x, 20, 1) << 31))
#define ENCODE_CITYPE_IMM(x) \
  ((RV_X(x, 0, 5) << 2) | (RV_X(x, 5, 1) << 12))
#define ENCODE_CITYPE_LUI_IMM(x) \
  ENCODE_CITYPE_IMM ((x) >> RISCV_IMM_BITS)
#define ENCODE_CITYPE_ADDI16SP_IMM(x) \
  ((RV_X(x, 4, 1) << 6) | (RV_X(x, 5, 1) << 2) | (RV_X(x, 6, 1) << 5) | (RV_X(x, 7, 2) << 3) | (RV_X(x, 9, 1) << 12))
#define ENCODE_CITYPE_LWSP_IMM(x) \
  ((RV_X(x, 2, 3) << 4) | (RV_X(x, 5, 1) << 12) | (RV_X(x, 6, 2) << 2))
#define ENCODE_CITYPE_LDSP_IMM(x) \
  ((RV_X(x, 3, 2) << 5) | (RV_X(x, 5, 1) << 12) | (RV_X(x, 6, 3) << 2))
#define ENCODE_CSSTYPE_IMM(x) \
  (RV_X(x, 0, 6) << 7)
#define ENCODE_CSSTYPE_SWSP_IMM(x) \
  ((RV_X(x, 2, 4) << 9) | (RV_X(x, 6, 2) << 7))
#define ENCODE_CSSTYPE_SDSP_IMM(x) \
  ((RV_X(x, 3, 3) << 10) | (RV_X(x, 6, 3) << 7))
#define ENCODE_CIWTYPE_IMM(x) \
  (RV_X(x, 0, 8) << 5)
#define ENCODE_CIWTYPE_ADDI4SPN_IMM(x) \
  ((RV_X(x, 2, 1) << 6) | (RV_X(x, 3, 1) << 5) | (RV_X(x, 4, 2) << 11) | (RV_X(x, 6, 4) << 7))
#define ENCODE_CLTYPE_IMM(x) \
  ((RV_X(x, 0, 2) << 5) | (RV_X(x, 2, 3) << 10))
#define ENCODE_CLTYPE_LW_IMM(x) \
  ((RV_X(x, 2, 1) << 6) | (RV_X(x, 3, 3) << 10) | (RV_X(x, 6, 1) << 5))
#define ENCODE_CLTYPE_LD_IMM(x) \
  ((RV_X(x, 3, 3) << 10) | (RV_X(x, 6, 2) << 5))
#define ENCODE_CBTYPE_IMM(x) \
  ((RV_X(x, 1, 2) << 3) | (RV_X(x, 3, 2) << 10) | (RV_X(x, 5, 1) << 2) | (RV_X(x, 6, 2) << 5) | (RV_X(x, 8, 1) << 12))
#define ENCODE_CJTYPE_IMM(x) \
  ((RV_X(x, 1, 3) << 3) | (RV_X(x, 4, 1) << 11) | (RV_X(x, 5, 1) << 2) | (RV_X(x, 6, 1) << 7) | (RV_X(x, 7, 1) << 6) | (RV_X(x, 8, 2) << 9) | (RV_X(x, 10, 1) << 8) | (RV_X(x, 11, 1) << 12))
#define ENCODE_RVV_VB_IMM(x) \
  (RV_X(x, 0, 10) << 20)
#define ENCODE_RVV_VC_IMM(x) \
  (RV_X(x, 0, 11) << 20)
#define ENCODE_RVV_VI_UIMM6(x) \
  (RV_X(x, 0, 5) << 15 | RV_X(x, 5, 1) << 26)

#define VALID_ITYPE_IMM(x) (EXTRACT_ITYPE_IMM(ENCODE_ITYPE_IMM(x)) == (x))
#define VALID_STYPE_IMM(x) (EXTRACT_STYPE_IMM(ENCODE_STYPE_IMM(x)) == (x))
#define VALID_BTYPE_IMM(x) (EXTRACT_BTYPE_IMM(ENCODE_BTYPE_IMM(x)) == (x))
#define VALID_UTYPE_IMM(x) (EXTRACT_UTYPE_IMM(ENCODE_UTYPE_IMM(x)) == (x))
#define VALID_JTYPE_IMM(x) (EXTRACT_JTYPE_IMM(ENCODE_JTYPE_IMM(x)) == (x))
#define VALID_CITYPE_IMM(x) (EXTRACT_CITYPE_IMM(ENCODE_CITYPE_IMM(x)) == (x))
#define VALID_CITYPE_LUI_IMM(x) (ENCODE_CITYPE_LUI_IMM(x) != 0 \
				 && EXTRACT_CITYPE_LUI_IMM(ENCODE_CITYPE_LUI_IMM(x)) == (x))
#define VALID_CITYPE_ADDI16SP_IMM(x) (ENCODE_CITYPE_ADDI16SP_IMM(x) != 0 \
				      && EXTRACT_CITYPE_ADDI16SP_IMM(ENCODE_CITYPE_ADDI16SP_IMM(x)) == (x))
#define VALID_CITYPE_LWSP_IMM(x) (EXTRACT_CITYPE_LWSP_IMM(ENCODE_CITYPE_LWSP_IMM(x)) == (x))
#define VALID_CITYPE_LDSP_IMM(x) (EXTRACT_CITYPE_LDSP_IMM(ENCODE_CITYPE_LDSP_IMM(x)) == (x))
#define VALID_CSSTYPE_IMM(x) (EXTRACT_CSSTYPE_IMM(ENCODE_CSSTYPE_IMM(x)) == (x))
#define VALID_CSSTYPE_SWSP_IMM(x) (EXTRACT_CSSTYPE_SWSP_IMM(ENCODE_CSSTYPE_SWSP_IMM(x)) == (x))
#define VALID_CSSTYPE_SDSP_IMM(x) (EXTRACT_CSSTYPE_SDSP_IMM(ENCODE_CSSTYPE_SDSP_IMM(x)) == (x))
#define VALID_CIWTYPE_IMM(x) (EXTRACT_CIWTYPE_IMM(ENCODE_CIWTYPE_IMM(x)) == (x))
#define VALID_CIWTYPE_ADDI4SPN_IMM(x) (EXTRACT_CIWTYPE_ADDI4SPN_IMM(ENCODE_CIWTYPE_ADDI4SPN_IMM(x)) == (x))
#define VALID_CLTYPE_IMM(x) (EXTRACT_CLTYPE_IMM(ENCODE_CLTYPE_IMM(x)) == (x))
#define VALID_CLTYPE_LW_IMM(x) (EXTRACT_CLTYPE_LW_IMM(ENCODE_CLTYPE_LW_IMM(x)) == (x))
#define VALID_CLTYPE_LD_IMM(x) (EXTRACT_CLTYPE_LD_IMM(ENCODE_CLTYPE_LD_IMM(x)) == (x))
#define VALID_CBTYPE_IMM(x) (EXTRACT_CBTYPE_IMM(ENCODE_CBTYPE_IMM(x)) == (x))
#define VALID_CJTYPE_IMM(x) (EXTRACT_CJTYPE_IMM(ENCODE_CJTYPE_IMM(x)) == (x))
#define VALID_RVV_VB_IMM(x) (EXTRACT_RVV_VB_IMM(ENCODE_RVV_VB_IMM(x)) == (x))
#define VALID_RVV_VC_IMM(x) (EXTRACT_RVV_VC_IMM(ENCODE_RVV_VC_IMM(x)) == (x))

#define RISCV_RTYPE(insn, rd, rs1, rs2) \
  ((MATCH_ ## insn) | ((rd) << OP_SH_RD) | ((rs1) << OP_SH_RS1) | ((rs2) << OP_SH_RS2))
#define RISCV_ITYPE(insn, rd, rs1, imm) \
  ((MATCH_ ## insn) | ((rd) << OP_SH_RD) | ((rs1) << OP_SH_RS1) | ENCODE_ITYPE_IMM(imm))
#define RISCV_STYPE(insn, rs1, rs2, imm) \
  ((MATCH_ ## insn) | ((rs1) << OP_SH_RS1) | ((rs2) << OP_SH_RS2) | ENCODE_STYPE_IMM(imm))
#define RISCV_BTYPE(insn, rs1, rs2, target) \
  ((MATCH_ ## insn) | ((rs1) << OP_SH_RS1) | ((rs2) << OP_SH_RS2) | ENCODE_BTYPE_IMM(target))
#define RISCV_UTYPE(insn, rd, bigimm) \
  ((MATCH_ ## insn) | ((rd) << OP_SH_RD) | ENCODE_UTYPE_IMM(bigimm))
#define RISCV_JTYPE(insn, rd, target) \
  ((MATCH_ ## insn) | ((rd) << OP_SH_RD) | ENCODE_JTYPE_IMM(target))

#define RISCV_NOP RISCV_ITYPE(ADDI, 0, 0, 0)
#define RVC_NOP MATCH_C_ADDI

#define RISCV_CONST_HIGH_PART(VALUE) \
  (((VALUE) + (RISCV_IMM_REACH/2)) & ~(RISCV_IMM_REACH-1))
#define RISCV_CONST_LOW_PART(VALUE) ((VALUE) - RISCV_CONST_HIGH_PART (VALUE))
#define RISCV_PCREL_HIGH_PART(VALUE, PC) RISCV_CONST_HIGH_PART((VALUE) - (PC))
#define RISCV_PCREL_LOW_PART(VALUE, PC) RISCV_CONST_LOW_PART((VALUE) - (PC))

#define RISCV_JUMP_BITS RISCV_BIGIMM_BITS
#define RISCV_JUMP_ALIGN_BITS 1
#define RISCV_JUMP_ALIGN (1 << RISCV_JUMP_ALIGN_BITS)
#define RISCV_JUMP_REACH ((1ULL << RISCV_JUMP_BITS) * RISCV_JUMP_ALIGN)

#define RISCV_IMM_BITS 12
#define RISCV_BIGIMM_BITS (32 - RISCV_IMM_BITS)
#define RISCV_IMM_REACH (1LL << RISCV_IMM_BITS)
#define RISCV_BIGIMM_REACH (1LL << RISCV_BIGIMM_BITS)
#define RISCV_RVC_IMM_REACH (1LL << 6)
#define RISCV_BRANCH_BITS RISCV_IMM_BITS
#define RISCV_BRANCH_ALIGN_BITS RISCV_JUMP_ALIGN_BITS
#define RISCV_BRANCH_ALIGN (1 << RISCV_BRANCH_ALIGN_BITS)
#define RISCV_BRANCH_REACH (RISCV_IMM_REACH * RISCV_BRANCH_ALIGN)

/* RV fields.  */

#define OP_MASK_OP		0x7f
#define OP_SH_OP		0
#define OP_MASK_RS2		0x1f
#define OP_SH_RS2		20
#define OP_MASK_RS1		0x1f
#define OP_SH_RS1		15
#define OP_MASK_RS3		0x1fU
#define OP_SH_RS3		27
#define OP_MASK_RD		0x1f
#define OP_SH_RD		7
#define OP_MASK_SHAMT		0x3f
#define OP_SH_SHAMT		20
#define OP_MASK_SHAMTW		0x1f
#define OP_SH_SHAMTW		20
#define OP_MASK_RM		0x7
#define OP_SH_RM		12
#define OP_MASK_PRED		0xf
#define OP_SH_PRED		24
#define OP_MASK_SUCC		0xf
#define OP_SH_SUCC		20
#define OP_MASK_AQ		0x1
#define OP_SH_AQ		26
#define OP_MASK_RL		0x1
#define OP_SH_RL		25

#define OP_MASK_CSR		0xfffU
#define OP_SH_CSR		20

#define OP_MASK_FUNCT3		0x7
#define OP_SH_FUNCT3		12
#define OP_MASK_FUNCT7		0x7fU
#define OP_SH_FUNCT7		25
#define OP_MASK_FUNCT2		0x3
#define OP_SH_FUNCT2		25

/* RVC fields.  */

#define OP_MASK_OP2		0x3
#define OP_SH_OP2		0

#define OP_MASK_CRS2		0x1f
#define OP_SH_CRS2		2
#define OP_MASK_CRS1S		0x7
#define OP_SH_CRS1S		7
#define OP_MASK_CRS2S		0x7
#define OP_SH_CRS2S		2

#define OP_MASK_CFUNCT6		0x3f
#define OP_SH_CFUNCT6		10
#define OP_MASK_CFUNCT4		0xf
#define OP_SH_CFUNCT4		12
#define OP_MASK_CFUNCT3		0x7
#define OP_SH_CFUNCT3		13
#define OP_MASK_CFUNCT2		0x3
#define OP_SH_CFUNCT2		5

/* Scalar crypto fields. */

#define OP_SH_BS        30
#define OP_MASK_BS      3
#define OP_SH_RNUM      20
#define OP_MASK_RNUM    0xf

/* RVV fields.  */

#define OP_MASK_VD		0x1f
#define OP_SH_VD		7
#define OP_MASK_VS1		0x1f
#define OP_SH_VS1		15
#define OP_MASK_VS2		0x1f
#define OP_SH_VS2		20
#define OP_MASK_VIMM		0x1f
#define OP_SH_VIMM		15
#define OP_MASK_VMASK		0x1
#define OP_SH_VMASK		25
#define OP_MASK_VFUNCT6		0x3f
#define OP_SH_VFUNCT6		26
#define OP_MASK_VLMUL		0x7
#define OP_SH_VLMUL		0
#define OP_MASK_VSEW		0x7
#define OP_SH_VSEW		3
#define OP_MASK_VTA		0x1
#define OP_SH_VTA		6
#define OP_MASK_VMA		0x1
#define OP_SH_VMA		7
#define OP_MASK_VWD		0x1
#define OP_SH_VWD		26

#define NVECR 32
#define NVECM 1

/* ABI names for selected x-registers.  */

#define X_RA 1
#define X_SP 2
#define X_GP 3
#define X_TP 4
#define X_T0 5
#define X_T1 6
#define X_T2 7
#define X_T3 28

#define NGPR 32
#define NFPR 32

/* These fake label defines are use by both the assembler, and
   libopcodes.  The assembler uses this when it needs to generate a fake
   label, and libopcodes uses it to hide the fake labels in its output.  */
#define RISCV_FAKE_LABEL_NAME ".L0 "
#define RISCV_FAKE_LABEL_CHAR ' '

/* Replace bits MASK << SHIFT of STRUCT with the equivalent bits in
   VALUE << SHIFT.  VALUE is evaluated exactly once.  */
#define INSERT_BITS(STRUCT, VALUE, MASK, SHIFT) \
  (STRUCT) = (((STRUCT) & ~((insn_t)(MASK) << (SHIFT))) \
	      | ((insn_t)((VALUE) & (MASK)) << (SHIFT)))

/* Extract bits MASK << SHIFT from STRUCT and shift them right
   SHIFT places.  */
#define EXTRACT_BITS(STRUCT, MASK, SHIFT) \
  (((STRUCT) >> (SHIFT)) & (MASK))

/* Extract the operand given by FIELD from integer INSN.  */
#define EXTRACT_OPERAND(FIELD, INSN) \
  EXTRACT_BITS ((INSN), OP_MASK_##FIELD, OP_SH_##FIELD)

/* Extract an unsigned immediate operand on position s with n bits.  */
#define EXTRACT_U_IMM(n, s, l) \
  RV_X (l, s, n)

/* Extract an signed immediate operand on position s with n bits.  */
#define EXTRACT_S_IMM(n, s, l) \
  RV_X_SIGNED (l, s, n)

/* Validate that unsigned n-bit immediate is within bounds.  */
#define VALIDATE_U_IMM(v, n) \
  ((unsigned long) v < (1UL << n))

/* Validate that signed n-bit immediate is within bounds.  */
#define VALIDATE_S_IMM(v, n) \
  (v < (long) (1UL << (n-1)) && v >= -(offsetT) (1UL << (n-1)))

/* The maximal number of subset can be required.  */
#define MAX_SUBSET_NUM 4

/* All RISC-V instructions belong to at least one of these classes.  */
enum riscv_insn_class
{
  INSN_CLASS_NONE,

  INSN_CLASS_I,
  INSN_CLASS_C,
  INSN_CLASS_A,
  INSN_CLASS_M,
  INSN_CLASS_F,
  INSN_CLASS_D,
  INSN_CLASS_Q,
  INSN_CLASS_F_AND_C,
  INSN_CLASS_D_AND_C,
  INSN_CLASS_ZICOND,
  INSN_CLASS_ZICSR,
  INSN_CLASS_ZIFENCEI,
  INSN_CLASS_ZIHINTPAUSE,
  INSN_CLASS_ZMMUL,
  INSN_CLASS_ZAWRS,
  INSN_CLASS_F_INX,
  INSN_CLASS_D_INX,
  INSN_CLASS_Q_INX,
  INSN_CLASS_ZFH_INX,
  INSN_CLASS_ZFHMIN,
  INSN_CLASS_ZFHMIN_INX,
  INSN_CLASS_ZFHMIN_AND_D_INX,
  INSN_CLASS_ZFHMIN_AND_Q_INX,
  INSN_CLASS_ZFA,
  INSN_CLASS_D_AND_ZFA,
  INSN_CLASS_Q_AND_ZFA,
  INSN_CLASS_ZFH_AND_ZFA,
  INSN_CLASS_ZBA,
  INSN_CLASS_ZBB,
  INSN_CLASS_ZBC,
  INSN_CLASS_ZBS,
  INSN_CLASS_ZBKB,
  INSN_CLASS_ZBKC,
  INSN_CLASS_ZBKX,
  INSN_CLASS_ZKND,
  INSN_CLASS_ZKNE,
  INSN_CLASS_ZKNH,
  INSN_CLASS_ZKSED,
  INSN_CLASS_ZKSH,
  INSN_CLASS_ZBB_OR_ZBKB,
  INSN_CLASS_ZBC_OR_ZBKC,
  INSN_CLASS_ZKND_OR_ZKNE,
  INSN_CLASS_V,
  INSN_CLASS_ZVEF,
  INSN_CLASS_ZVBB,
  INSN_CLASS_ZVBC,
  INSN_CLASS_ZVKG,
  INSN_CLASS_ZVKNED,
  INSN_CLASS_ZVKNHA,
  INSN_CLASS_ZVKNHB,
  INSN_CLASS_ZVKNHA_OR_ZVKNHB,
  INSN_CLASS_ZVKSED,
  INSN_CLASS_ZVKSH,
  INSN_CLASS_SVINVAL,
  INSN_CLASS_ZICBOM,
  INSN_CLASS_ZICBOP,
  INSN_CLASS_ZICBOZ,
  INSN_CLASS_H,
  INSN_CLASS_XTHEADBA,
  INSN_CLASS_XTHEADBB,
  INSN_CLASS_XTHEADBS,
  INSN_CLASS_XTHEADCMO,
  INSN_CLASS_XTHEADCONDMOV,
  INSN_CLASS_XTHEADFMEMIDX,
  INSN_CLASS_XTHEADFMV,
  INSN_CLASS_XTHEADINT,
  INSN_CLASS_XTHEADMAC,
  INSN_CLASS_XTHEADMEMIDX,
  INSN_CLASS_XTHEADMEMPAIR,
  INSN_CLASS_XTHEADSYNC,
  INSN_CLASS_XVENTANACONDOPS,
};

/* This structure holds information for a particular instruction.  */
struct riscv_opcode
{
  /* The name of the instruction.  */
  const char *name;

  /* The requirement of xlen for the instruction, 0 if no requirement.  */
  unsigned xlen_requirement;

  /* Class to which this instruction belongs.  Used to decide whether or
     not this instruction is legal in the current -march context.  */
  enum riscv_insn_class insn_class;

  /* A string describing the arguments for this instruction.  */
  const char *args;

  /* The basic opcode for the instruction.  When assembling, this
     opcode is modified by the arguments to produce the actual opcode
     that is used.  If pinfo is INSN_MACRO, then this is 0.  */
  insn_t match;

  /* If pinfo is not INSN_MACRO, then this is a bit mask for the
     relevant portions of the opcode when disassembling.  If the
     actual opcode anded with the match field equals the opcode field,
     then we have found the correct instruction.  If pinfo is
     INSN_MACRO, then this field is the macro identifier.  */
  insn_t mask;

  /* A function to determine if a word corresponds to this instruction.
     Usually, this computes ((word & mask) == match).  */
  int (*match_func) (const struct riscv_opcode *op, insn_t word);

  /* For a macro, this is INSN_MACRO.  Otherwise, it is a collection
     of bits describing the instruction, notably any relevant hazard
     information.  */
  unsigned long pinfo;
};

/* Instruction is a simple alias (e.g. "mv" for "addi").  */
#define	INSN_ALIAS		0x00000001

/* These are for setting insn_info fields.

   Nonbranch is the default.  Noninsn is used only if there is no match.
   There are no condjsr or dref2 instructions.  So that leaves condbranch,
   branch, jsr, and dref that we need to handle here, encoded in 3 bits.  */
#define INSN_TYPE		0x0000000e

/* Instruction is an unconditional branch.  */
#define INSN_BRANCH		0x00000002
/* Instruction is a conditional branch.  */
#define INSN_CONDBRANCH		0x00000004
/* Instruction is a jump to subroutine.  */
#define INSN_JSR		0x00000006
/* Instruction is a data reference.  */
#define INSN_DREF		0x00000008
/* Instruction is allowed when eew >= 64.  */
#define INSN_V_EEW64		0x10000000

/* We have 5 data reference sizes, which we can encode in 3 bits.  */
#define INSN_DATA_SIZE		0x00000070
#define INSN_DATA_SIZE_SHIFT	4
#define INSN_1_BYTE		0x00000010
#define INSN_2_BYTE		0x00000020
#define INSN_4_BYTE		0x00000030
#define INSN_8_BYTE		0x00000040
#define INSN_16_BYTE		0x00000050

/* Instruction is actually a macro.  It should be ignored by the
   disassembler, and requires special treatment by the assembler.  */
#define INSN_MACRO		0xffffffff

/* This is a list of macro expanded instructions.  */
enum
{
  M_LA,
  M_LLA,
  M_LGA,
  M_LA_TLS_GD,
  M_LA_TLS_IE,
  M_LB,
  M_LBU,
  M_LH,
  M_LHU,
  M_LW,
  M_LWU,
  M_LD,
  M_SB,
  M_SH,
  M_SW,
  M_SD,
  M_FLW,
  M_FLD,
  M_FLQ,
  M_FSW,
  M_FSD,
  M_FSQ,
  M_CALL,
  M_J,
  M_LI,
  M_ZEXTH,
  M_ZEXTW,
  M_SEXTB,
  M_SEXTH,
  M_VMSGE,
  M_VMSGEU,
  M_FLH,
  M_FSH,
  M_NUM_MACROS
};

/* The mapping symbol states.  */
enum riscv_seg_mstate
{
  MAP_NONE = 0,		/* Must be zero, for seginfo in new sections.  */
  MAP_DATA,		/* Data.  */
  MAP_INSN,		/* Instructions.  */
};

extern const char * const riscv_gpr_names_numeric[NGPR];
extern const char * const riscv_gpr_names_abi[NGPR];
extern const char * const riscv_fpr_names_numeric[NFPR];
extern const char * const riscv_fpr_names_abi[NFPR];
extern const char * const riscv_rm[8];
extern const char * const riscv_pred_succ[16];
extern const char * const riscv_vecr_names_numeric[NVECR];
extern const char * const riscv_vecm_names_numeric[NVECM];
extern const char * const riscv_vsew[8];
extern const char * const riscv_vlmul[8];
extern const char * const riscv_vta[2];
extern const char * const riscv_vma[2];
extern const char * const riscv_fli_symval[32];
extern const float riscv_fli_numval[32];

extern const struct riscv_opcode riscv_opcodes[];
extern const struct riscv_opcode riscv_insn_types[];

#endif /* _RISCV_H_ */
