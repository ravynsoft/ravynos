/* AArch64 assembler/disassembler support.

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef OPCODE_AARCH64_H
#define OPCODE_AARCH64_H

#include "bfd.h"
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "dis-asm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The offset for pc-relative addressing is currently defined to be 0.  */
#define AARCH64_PCREL_OFFSET		0

typedef uint32_t aarch64_insn;

/* The following bitmasks control CPU features.  */
#define AARCH64_FEATURE_V8	     (1ULL << 0) /* All processors.  */
#define AARCH64_FEATURE_V8_6	     (1ULL << 1) /* ARMv8.6 processors.  */
#define AARCH64_FEATURE_BFLOAT16     (1ULL << 2) /* Bfloat16 insns.  */
#define AARCH64_FEATURE_V8_A	     (1ULL << 3) /* Armv8-A processors.  */
#define AARCH64_FEATURE_SVE2	     (1ULL << 4) /* SVE2 instructions.  */
#define AARCH64_FEATURE_V8_2	     (1ULL << 5) /* ARMv8.2 processors.  */
#define AARCH64_FEATURE_V8_3	     (1ULL << 6) /* ARMv8.3 processors.  */
#define AARCH64_FEATURE_SVE2_AES     (1ULL << 7)
#define AARCH64_FEATURE_SVE2_BITPERM (1ULL << 8)
#define AARCH64_FEATURE_SVE2_SM4     (1ULL << 9)
#define AARCH64_FEATURE_SVE2_SHA3    (1ULL << 10)
#define AARCH64_FEATURE_V8_4	     (1ULL << 11) /* ARMv8.4 processors.  */
#define AARCH64_FEATURE_V8_R	     (1ULL << 12) /* Armv8-R processors.  */
#define AARCH64_FEATURE_V8_7	     (1ULL << 13) /* Armv8.7 processors.  */
#define AARCH64_FEATURE_SME	     (1ULL << 14) /* Scalable Matrix Extension.  */
#define AARCH64_FEATURE_LS64	     (1ULL << 15) /* Atomic 64-byte load/store.  */
#define AARCH64_FEATURE_PAC	     (1ULL << 16) /* v8.3 Pointer Authentication.  */
#define AARCH64_FEATURE_FP	     (1ULL << 17) /* FP instructions.  */
#define AARCH64_FEATURE_SIMD	     (1ULL << 18) /* SIMD instructions.  */
#define AARCH64_FEATURE_CRC	     (1ULL << 19) /* CRC instructions.  */
#define AARCH64_FEATURE_LSE	     (1ULL << 20) /* LSE instructions.  */
#define AARCH64_FEATURE_PAN	     (1ULL << 21) /* PAN instructions.  */
#define AARCH64_FEATURE_LOR	     (1ULL << 22) /* LOR instructions.  */
#define AARCH64_FEATURE_RDMA	     (1ULL << 23) /* v8.1 SIMD instructions.  */
#define AARCH64_FEATURE_V8_1	     (1ULL << 24) /* v8.1 features.  */
#define AARCH64_FEATURE_F16	     (1ULL << 25) /* v8.2 FP16 instructions.  */
#define AARCH64_FEATURE_RAS	     (1ULL << 26) /* RAS Extensions.  */
#define AARCH64_FEATURE_PROFILE      (1ULL << 27) /* Statistical Profiling.  */
#define AARCH64_FEATURE_SVE	     (1ULL << 28) /* SVE instructions.  */
#define AARCH64_FEATURE_RCPC	     (1ULL << 29) /* RCPC instructions.  */
#define AARCH64_FEATURE_COMPNUM      (1ULL << 30) /* Complex # instructions.  */
#define AARCH64_FEATURE_DOTPROD      (1ULL << 31) /* Dot Product instructions.  */
#define AARCH64_FEATURE_SM4	     (1ULL << 32) /* SM3 & SM4 instructions.  */
#define AARCH64_FEATURE_SHA2	     (1ULL << 33) /* SHA2 instructions.  */
#define AARCH64_FEATURE_SHA3	     (1ULL << 34) /* SHA3 instructions.  */
#define AARCH64_FEATURE_AES	     (1ULL << 35) /* AES instructions.  */
#define AARCH64_FEATURE_F16_FML      (1ULL << 36) /* v8.2 FP16FML ins.  */
#define AARCH64_FEATURE_V8_5	     (1ULL << 37) /* ARMv8.5 processors.  */
#define AARCH64_FEATURE_FLAGMANIP    (1ULL << 38) /* v8.5 Flag Manipulation version 2.  */
#define AARCH64_FEATURE_FRINTTS      (1ULL << 39) /* FRINT[32,64][Z,X] insns.  */
#define AARCH64_FEATURE_SB	     (1ULL << 40) /* SB instruction.  */
#define AARCH64_FEATURE_PREDRES      (1ULL << 41) /* Execution and Data Prediction Restriction instructions.  */
#define AARCH64_FEATURE_CVADP	     (1ULL << 42) /* DC CVADP.  */
#define AARCH64_FEATURE_RNG	     (1ULL << 43) /* Random Number instructions.  */
#define AARCH64_FEATURE_BTI	     (1ULL << 44) /* BTI instructions.  */
#define AARCH64_FEATURE_SCXTNUM      (1ULL << 45) /* SCXTNUM_ELx.  */
#define AARCH64_FEATURE_ID_PFR2      (1ULL << 46) /* ID_PFR2 instructions.  */
#define AARCH64_FEATURE_SSBS	     (1ULL << 47) /* SSBS mechanism enabled.  */
#define AARCH64_FEATURE_MEMTAG       (1ULL << 48) /* Memory Tagging Extension.  */
#define AARCH64_FEATURE_TME	     (1ULL << 49) /* Transactional Memory Extension.  */
#define AARCH64_FEATURE_MOPS	     (1ULL << 50) /* Standardization of memory operations.  */
#define AARCH64_FEATURE_HBC	     (1ULL << 51) /* Hinted conditional branches.  */
#define AARCH64_FEATURE_I8MM	     (1ULL << 52) /* Matrix Multiply instructions.  */
#define AARCH64_FEATURE_F32MM	     (1ULL << 53)
#define AARCH64_FEATURE_F64MM	     (1ULL << 54)
#define AARCH64_FEATURE_FLAGM	     (1ULL << 55) /* v8.4 Flag Manipulation.  */
#define AARCH64_FEATURE_V9	     (1ULL << 56) /* Armv9.0-A processors.  */
#define AARCH64_FEATURE_SME_F64F64   (1ULL << 57) /* SME F64F64.  */
#define AARCH64_FEATURE_SME_I16I64   (1ULL << 58) /* SME I16I64.  */
#define AARCH64_FEATURE_V8_8	     (1ULL << 59) /* Armv8.8 processors.  */
#define AARCH64_FEATURE_CSSC	     (1ULL << 60) /* Common Short Sequence Compression instructions.  */
#define AARCH64_FEATURE_SME2	     (1ULL << 61) /* SME2.  */

/* Crypto instructions are the combination of AES and SHA2.  */
#define AARCH64_FEATURE_CRYPTO	(AARCH64_FEATURE_SHA2 | AARCH64_FEATURE_AES)

#define AARCH64_ARCH_V8_FEATURES	(AARCH64_FEATURE_V8_A		\
					 | AARCH64_FEATURE_FP		\
					 | AARCH64_FEATURE_RAS		\
					 | AARCH64_FEATURE_SIMD)
#define AARCH64_ARCH_V8_1_FEATURES	(AARCH64_FEATURE_V8_1		\
					 | AARCH64_FEATURE_CRC		\
					 | AARCH64_FEATURE_LSE		\
					 | AARCH64_FEATURE_PAN		\
					 | AARCH64_FEATURE_LOR		\
					 | AARCH64_FEATURE_RDMA)
#define AARCH64_ARCH_V8_2_FEATURES	(AARCH64_FEATURE_V8_2)
#define AARCH64_ARCH_V8_3_FEATURES	(AARCH64_FEATURE_V8_3		\
					 | AARCH64_FEATURE_PAC		\
					 | AARCH64_FEATURE_RCPC		\
					 | AARCH64_FEATURE_COMPNUM)
#define AARCH64_ARCH_V8_4_FEATURES	(AARCH64_FEATURE_V8_4		\
					 | AARCH64_FEATURE_DOTPROD	\
					 | AARCH64_FEATURE_FLAGM	\
					 | AARCH64_FEATURE_F16_FML)
#define AARCH64_ARCH_V8_5_FEATURES	(AARCH64_FEATURE_V8_5		\
					 | AARCH64_FEATURE_FLAGMANIP	\
					 | AARCH64_FEATURE_FRINTTS	\
					 | AARCH64_FEATURE_SB		\
					 | AARCH64_FEATURE_PREDRES	\
					 | AARCH64_FEATURE_CVADP	\
					 | AARCH64_FEATURE_BTI		\
					 | AARCH64_FEATURE_SCXTNUM	\
					 | AARCH64_FEATURE_ID_PFR2	\
					 | AARCH64_FEATURE_SSBS)
#define AARCH64_ARCH_V8_6_FEATURES	(AARCH64_FEATURE_V8_6		\
					 | AARCH64_FEATURE_BFLOAT16	\
					 | AARCH64_FEATURE_I8MM)
#define AARCH64_ARCH_V8_7_FEATURES	(AARCH64_FEATURE_V8_7		\
					 | AARCH64_FEATURE_LS64)
#define AARCH64_ARCH_V8_8_FEATURES	(AARCH64_FEATURE_V8_8		\
					 | AARCH64_FEATURE_MOPS		\
					 | AARCH64_FEATURE_HBC)

#define AARCH64_ARCH_V9_FEATURES	(AARCH64_FEATURE_V9		\
					 | AARCH64_FEATURE_F16          \
					 | AARCH64_FEATURE_SVE		\
					 | AARCH64_FEATURE_SVE2)
#define AARCH64_ARCH_V9_1_FEATURES	(AARCH64_ARCH_V8_6_FEATURES)
#define AARCH64_ARCH_V9_2_FEATURES	(AARCH64_ARCH_V8_7_FEATURES)
#define AARCH64_ARCH_V9_3_FEATURES	(AARCH64_ARCH_V8_8_FEATURES)

/* Architectures are the sum of the base and extensions.  */
#define AARCH64_ARCH_V8		AARCH64_FEATURE (AARCH64_FEATURE_V8, \
						 AARCH64_ARCH_V8_FEATURES)
#define AARCH64_ARCH_V8_1	AARCH64_FEATURE (AARCH64_ARCH_V8, \
						 AARCH64_ARCH_V8_1_FEATURES)
#define AARCH64_ARCH_V8_2	AARCH64_FEATURE (AARCH64_ARCH_V8_1,	\
						 AARCH64_ARCH_V8_2_FEATURES)
#define AARCH64_ARCH_V8_3	AARCH64_FEATURE (AARCH64_ARCH_V8_2,	\
						 AARCH64_ARCH_V8_3_FEATURES)
#define AARCH64_ARCH_V8_4	AARCH64_FEATURE (AARCH64_ARCH_V8_3,	\
						 AARCH64_ARCH_V8_4_FEATURES)
#define AARCH64_ARCH_V8_5	AARCH64_FEATURE (AARCH64_ARCH_V8_4,	\
						 AARCH64_ARCH_V8_5_FEATURES)
#define AARCH64_ARCH_V8_6	AARCH64_FEATURE (AARCH64_ARCH_V8_5,	\
						 AARCH64_ARCH_V8_6_FEATURES)
#define AARCH64_ARCH_V8_7	AARCH64_FEATURE (AARCH64_ARCH_V8_6,	\
						 AARCH64_ARCH_V8_7_FEATURES)
#define AARCH64_ARCH_V8_8	AARCH64_FEATURE (AARCH64_ARCH_V8_7,	\
						 AARCH64_ARCH_V8_8_FEATURES)
#define AARCH64_ARCH_V8_R	(AARCH64_FEATURE (AARCH64_ARCH_V8_4,	\
						 AARCH64_FEATURE_V8_R)	\
			      & ~(AARCH64_FEATURE_V8_A | AARCH64_FEATURE_LOR))

#define AARCH64_ARCH_V9		AARCH64_FEATURE (AARCH64_ARCH_V8_5,	\
						 AARCH64_ARCH_V9_FEATURES)
#define AARCH64_ARCH_V9_1	AARCH64_FEATURE (AARCH64_ARCH_V9,	\
						 AARCH64_ARCH_V9_1_FEATURES)
#define AARCH64_ARCH_V9_2	AARCH64_FEATURE (AARCH64_ARCH_V9_1,	\
						 AARCH64_ARCH_V9_2_FEATURES)
#define AARCH64_ARCH_V9_3	AARCH64_FEATURE (AARCH64_ARCH_V9_2,	\
						 AARCH64_ARCH_V9_3_FEATURES)

#define AARCH64_ARCH_NONE	AARCH64_FEATURE (0, 0)
#define AARCH64_ANY		AARCH64_FEATURE (-1, 0)	/* Any basic core.  */

/* CPU-specific features.  */
typedef unsigned long long aarch64_feature_set;

#define AARCH64_CPU_HAS_ALL_FEATURES(CPU,FEAT)	\
  ((~(CPU) & (FEAT)) == 0)

#define AARCH64_CPU_HAS_ANY_FEATURES(CPU,FEAT)	\
  (((CPU) & (FEAT)) != 0)

#define AARCH64_CPU_HAS_FEATURE(CPU,FEAT)	\
  AARCH64_CPU_HAS_ALL_FEATURES (CPU,FEAT)

#define AARCH64_MERGE_FEATURE_SETS(TARG,F1,F2)	\
  do						\
    {						\
      (TARG) = (F1) | (F2);			\
    }						\
  while (0)

#define AARCH64_CLEAR_FEATURE(TARG,F1,F2)	\
  do						\
    { 						\
      (TARG) = (F1) &~ (F2);			\
    }						\
  while (0)

#define AARCH64_FEATURE(core,coproc) ((core) | (coproc))

enum aarch64_operand_class
{
  AARCH64_OPND_CLASS_NIL,
  AARCH64_OPND_CLASS_INT_REG,
  AARCH64_OPND_CLASS_MODIFIED_REG,
  AARCH64_OPND_CLASS_FP_REG,
  AARCH64_OPND_CLASS_SIMD_REG,
  AARCH64_OPND_CLASS_SIMD_ELEMENT,
  AARCH64_OPND_CLASS_SISD_REG,
  AARCH64_OPND_CLASS_SIMD_REGLIST,
  AARCH64_OPND_CLASS_SVE_REG,
  AARCH64_OPND_CLASS_SVE_REGLIST,
  AARCH64_OPND_CLASS_PRED_REG,
  AARCH64_OPND_CLASS_ZA_ACCESS,
  AARCH64_OPND_CLASS_ADDRESS,
  AARCH64_OPND_CLASS_IMMEDIATE,
  AARCH64_OPND_CLASS_SYSTEM,
  AARCH64_OPND_CLASS_COND,
};

/* Operand code that helps both parsing and coding.
   Keep AARCH64_OPERANDS synced.  */

enum aarch64_opnd
{
  AARCH64_OPND_NIL,	/* no operand---MUST BE FIRST!*/

  AARCH64_OPND_Rd,	/* Integer register as destination.  */
  AARCH64_OPND_Rn,	/* Integer register as source.  */
  AARCH64_OPND_Rm,	/* Integer register as source.  */
  AARCH64_OPND_Rt,	/* Integer register used in ld/st instructions.  */
  AARCH64_OPND_Rt2,	/* Integer register used in ld/st pair instructions.  */
  AARCH64_OPND_Rt_LS64,	/* Integer register used in LS64 instructions.  */
  AARCH64_OPND_Rt_SP,	/* Integer Rt or SP used in STG instructions.  */
  AARCH64_OPND_Rs,	/* Integer register used in ld/st exclusive.  */
  AARCH64_OPND_Ra,	/* Integer register used in ddp_3src instructions.  */
  AARCH64_OPND_Rt_SYS,	/* Integer register used in system instructions.  */

  AARCH64_OPND_Rd_SP,	/* Integer Rd or SP.  */
  AARCH64_OPND_Rn_SP,	/* Integer Rn or SP.  */
  AARCH64_OPND_Rm_SP,	/* Integer Rm or SP.  */
  AARCH64_OPND_PAIRREG,	/* Paired register operand.  */
  AARCH64_OPND_Rm_EXT,	/* Integer Rm extended.  */
  AARCH64_OPND_Rm_SFT,	/* Integer Rm shifted.  */

  AARCH64_OPND_Fd,	/* Floating-point Fd.  */
  AARCH64_OPND_Fn,	/* Floating-point Fn.  */
  AARCH64_OPND_Fm,	/* Floating-point Fm.  */
  AARCH64_OPND_Fa,	/* Floating-point Fa.  */
  AARCH64_OPND_Ft,	/* Floating-point Ft.  */
  AARCH64_OPND_Ft2,	/* Floating-point Ft2.  */

  AARCH64_OPND_Sd,	/* AdvSIMD Scalar Sd.  */
  AARCH64_OPND_Sn,	/* AdvSIMD Scalar Sn.  */
  AARCH64_OPND_Sm,	/* AdvSIMD Scalar Sm.  */

  AARCH64_OPND_Va,	/* AdvSIMD Vector Va.  */
  AARCH64_OPND_Vd,	/* AdvSIMD Vector Vd.  */
  AARCH64_OPND_Vn,	/* AdvSIMD Vector Vn.  */
  AARCH64_OPND_Vm,	/* AdvSIMD Vector Vm.  */
  AARCH64_OPND_VdD1,	/* AdvSIMD <Vd>.D[1]; for FMOV only.  */
  AARCH64_OPND_VnD1,	/* AdvSIMD <Vn>.D[1]; for FMOV only.  */
  AARCH64_OPND_Ed,	/* AdvSIMD Vector Element Vd.  */
  AARCH64_OPND_En,	/* AdvSIMD Vector Element Vn.  */
  AARCH64_OPND_Em,	/* AdvSIMD Vector Element Vm.  */
  AARCH64_OPND_Em16,	/* AdvSIMD Vector Element Vm restricted to V0 - V15 when
			   qualifier is S_H.  */
  AARCH64_OPND_LVn,	/* AdvSIMD Vector register list used in e.g. TBL.  */
  AARCH64_OPND_LVt,	/* AdvSIMD Vector register list used in ld/st.  */
  AARCH64_OPND_LVt_AL,	/* AdvSIMD Vector register list for loading single
			   structure to all lanes.  */
  AARCH64_OPND_LEt,	/* AdvSIMD Vector Element list.  */

  AARCH64_OPND_CRn,	/* Co-processor register in CRn field.  */
  AARCH64_OPND_CRm,	/* Co-processor register in CRm field.  */

  AARCH64_OPND_IDX,	/* AdvSIMD EXT index operand.  */
  AARCH64_OPND_MASK,	/* AdvSIMD EXT index operand.  */
  AARCH64_OPND_IMM_VLSL,/* Immediate for shifting vector registers left.  */
  AARCH64_OPND_IMM_VLSR,/* Immediate for shifting vector registers right.  */
  AARCH64_OPND_SIMD_IMM,/* AdvSIMD modified immediate without shift.  */
  AARCH64_OPND_SIMD_IMM_SFT,	/* AdvSIMD modified immediate with shift.  */
  AARCH64_OPND_SIMD_FPIMM,/* AdvSIMD 8-bit fp immediate.  */
  AARCH64_OPND_SHLL_IMM,/* Immediate shift for AdvSIMD SHLL instruction
			   (no encoding).  */
  AARCH64_OPND_IMM0,	/* Immediate for #0.  */
  AARCH64_OPND_FPIMM0,	/* Immediate for #0.0.  */
  AARCH64_OPND_FPIMM,	/* Floating-point Immediate.  */
  AARCH64_OPND_IMMR,	/* Immediate #<immr> in e.g. BFM.  */
  AARCH64_OPND_IMMS,	/* Immediate #<imms> in e.g. BFM.  */
  AARCH64_OPND_WIDTH,	/* Immediate #<width> in e.g. BFI.  */
  AARCH64_OPND_IMM,	/* Immediate.  */
  AARCH64_OPND_IMM_2,	/* Immediate.  */
  AARCH64_OPND_UIMM3_OP1,/* Unsigned 3-bit immediate in the op1 field.  */
  AARCH64_OPND_UIMM3_OP2,/* Unsigned 3-bit immediate in the op2 field.  */
  AARCH64_OPND_UIMM4,	/* Unsigned 4-bit immediate in the CRm field.  */
  AARCH64_OPND_UIMM4_ADDG,/* Unsigned 4-bit immediate in addg/subg.  */
  AARCH64_OPND_UIMM7,	/* Unsigned 7-bit immediate in the CRm:op2 fields.  */
  AARCH64_OPND_UIMM10,	/* Unsigned 10-bit immediate in addg/subg.  */
  AARCH64_OPND_BIT_NUM,	/* Immediate.  */
  AARCH64_OPND_EXCEPTION,/* imm16 operand in exception instructions.  */
  AARCH64_OPND_UNDEFINED,/* imm16 operand in undefined instruction. */
  AARCH64_OPND_CCMP_IMM,/* Immediate in conditional compare instructions.  */
  AARCH64_OPND_SIMM5,	/* 5-bit signed immediate in the imm5 field.  */
  AARCH64_OPND_NZCV,	/* Flag bit specifier giving an alternative value for
			   each condition flag.  */

  AARCH64_OPND_LIMM,	/* Logical Immediate.  */
  AARCH64_OPND_AIMM,	/* Arithmetic immediate.  */
  AARCH64_OPND_HALF,	/* #<imm16>{, LSL #<shift>} operand in move wide.  */
  AARCH64_OPND_FBITS,	/* FP #<fbits> operand in e.g. SCVTF */
  AARCH64_OPND_IMM_MOV,	/* Immediate operand for the MOV alias.  */
  AARCH64_OPND_IMM_ROT1,	/* Immediate rotate operand for FCMLA.  */
  AARCH64_OPND_IMM_ROT2,	/* Immediate rotate operand for indexed FCMLA.  */
  AARCH64_OPND_IMM_ROT3,	/* Immediate rotate operand for FCADD.  */

  AARCH64_OPND_COND,	/* Standard condition as the last operand.  */
  AARCH64_OPND_COND1,	/* Same as the above, but excluding AL and NV.  */

  AARCH64_OPND_ADDR_ADRP,	/* Memory address for ADRP */
  AARCH64_OPND_ADDR_PCREL14,	/* 14-bit PC-relative address for e.g. TBZ.  */
  AARCH64_OPND_ADDR_PCREL19,	/* 19-bit PC-relative address for e.g. LDR.  */
  AARCH64_OPND_ADDR_PCREL21,	/* 21-bit PC-relative address for e.g. ADR.  */
  AARCH64_OPND_ADDR_PCREL26,	/* 26-bit PC-relative address for e.g. BL.  */

  AARCH64_OPND_ADDR_SIMPLE,	/* Address of ld/st exclusive.  */
  AARCH64_OPND_ADDR_REGOFF,	/* Address of register offset.  */
  AARCH64_OPND_ADDR_SIMM7,	/* Address of signed 7-bit immediate.  */
  AARCH64_OPND_ADDR_SIMM9,	/* Address of signed 9-bit immediate.  */
  AARCH64_OPND_ADDR_SIMM9_2,	/* Same as the above, but the immediate is
				   negative or unaligned and there is
				   no writeback allowed.  This operand code
				   is only used to support the programmer-
				   friendly feature of using LDR/STR as the
				   the mnemonic name for LDUR/STUR instructions
				   wherever there is no ambiguity.  */
  AARCH64_OPND_ADDR_SIMM10,	/* Address of signed 10-bit immediate.  */
  AARCH64_OPND_ADDR_SIMM11,	/* Address with a signed 11-bit (multiple of
				   16) immediate.  */
  AARCH64_OPND_ADDR_UIMM12,	/* Address of unsigned 12-bit immediate.  */
  AARCH64_OPND_ADDR_SIMM13,	/* Address with a signed 13-bit (multiple of
				   16) immediate.  */
  AARCH64_OPND_SIMD_ADDR_SIMPLE,/* Address of ld/st multiple structures.  */
  AARCH64_OPND_ADDR_OFFSET,     /* Address with an optional 9-bit immediate.  */
  AARCH64_OPND_SIMD_ADDR_POST,	/* Address of ld/st multiple post-indexed.  */

  AARCH64_OPND_SYSREG,		/* System register operand.  */
  AARCH64_OPND_PSTATEFIELD,	/* PSTATE field name operand.  */
  AARCH64_OPND_SYSREG_AT,	/* System register <at_op> operand.  */
  AARCH64_OPND_SYSREG_DC,	/* System register <dc_op> operand.  */
  AARCH64_OPND_SYSREG_IC,	/* System register <ic_op> operand.  */
  AARCH64_OPND_SYSREG_TLBI,	/* System register <tlbi_op> operand.  */
  AARCH64_OPND_SYSREG_SR,	/* System register RCTX operand.  */
  AARCH64_OPND_BARRIER,		/* Barrier operand.  */
  AARCH64_OPND_BARRIER_DSB_NXS,	/* Barrier operand for DSB nXS variant.  */
  AARCH64_OPND_BARRIER_ISB,	/* Barrier operand for ISB.  */
  AARCH64_OPND_PRFOP,		/* Prefetch operation.  */
  AARCH64_OPND_RPRFMOP,		/* Range prefetch operation.  */
  AARCH64_OPND_BARRIER_PSB,	/* Barrier operand for PSB.  */
  AARCH64_OPND_BTI_TARGET,	/* BTI {<target>}.  */
  AARCH64_OPND_SVE_ADDR_RI_S4x16,   /* SVE [<Xn|SP>, #<simm4>*16].  */
  AARCH64_OPND_SVE_ADDR_RI_S4x32,   /* SVE [<Xn|SP>, #<simm4>*32].  */
  AARCH64_OPND_SVE_ADDR_RI_S4xVL,   /* SVE [<Xn|SP>, #<simm4>, MUL VL].  */
  AARCH64_OPND_SVE_ADDR_RI_S4x2xVL, /* SVE [<Xn|SP>, #<simm4>*2, MUL VL].  */
  AARCH64_OPND_SVE_ADDR_RI_S4x3xVL, /* SVE [<Xn|SP>, #<simm4>*3, MUL VL].  */
  AARCH64_OPND_SVE_ADDR_RI_S4x4xVL, /* SVE [<Xn|SP>, #<simm4>*4, MUL VL].  */
  AARCH64_OPND_SVE_ADDR_RI_S6xVL,   /* SVE [<Xn|SP>, #<simm6>, MUL VL].  */
  AARCH64_OPND_SVE_ADDR_RI_S9xVL,   /* SVE [<Xn|SP>, #<simm9>, MUL VL].  */
  AARCH64_OPND_SVE_ADDR_RI_U6,	    /* SVE [<Xn|SP>, #<uimm6>].  */
  AARCH64_OPND_SVE_ADDR_RI_U6x2,    /* SVE [<Xn|SP>, #<uimm6>*2].  */
  AARCH64_OPND_SVE_ADDR_RI_U6x4,    /* SVE [<Xn|SP>, #<uimm6>*4].  */
  AARCH64_OPND_SVE_ADDR_RI_U6x8,    /* SVE [<Xn|SP>, #<uimm6>*8].  */
  AARCH64_OPND_SVE_ADDR_R,	    /* SVE [<Xn|SP>].  */
  AARCH64_OPND_SVE_ADDR_RR,	    /* SVE [<Xn|SP>, <Xm|XZR>].  */
  AARCH64_OPND_SVE_ADDR_RR_LSL1,    /* SVE [<Xn|SP>, <Xm|XZR>, LSL #1].  */
  AARCH64_OPND_SVE_ADDR_RR_LSL2,    /* SVE [<Xn|SP>, <Xm|XZR>, LSL #2].  */
  AARCH64_OPND_SVE_ADDR_RR_LSL3,    /* SVE [<Xn|SP>, <Xm|XZR>, LSL #3].  */
  AARCH64_OPND_SVE_ADDR_RR_LSL4,    /* SVE [<Xn|SP>, <Xm|XZR>, LSL #4].  */
  AARCH64_OPND_SVE_ADDR_RX,	    /* SVE [<Xn|SP>, <Xm>].  */
  AARCH64_OPND_SVE_ADDR_RX_LSL1,    /* SVE [<Xn|SP>, <Xm>, LSL #1].  */
  AARCH64_OPND_SVE_ADDR_RX_LSL2,    /* SVE [<Xn|SP>, <Xm>, LSL #2].  */
  AARCH64_OPND_SVE_ADDR_RX_LSL3,    /* SVE [<Xn|SP>, <Xm>, LSL #3].  */
  AARCH64_OPND_SVE_ADDR_ZX,	    /* SVE [Zn.<T>{, <Xm>}].  */
  AARCH64_OPND_SVE_ADDR_RZ,	    /* SVE [<Xn|SP>, Zm.D].  */
  AARCH64_OPND_SVE_ADDR_RZ_LSL1,    /* SVE [<Xn|SP>, Zm.D, LSL #1].  */
  AARCH64_OPND_SVE_ADDR_RZ_LSL2,    /* SVE [<Xn|SP>, Zm.D, LSL #2].  */
  AARCH64_OPND_SVE_ADDR_RZ_LSL3,    /* SVE [<Xn|SP>, Zm.D, LSL #3].  */
  AARCH64_OPND_SVE_ADDR_RZ_XTW_14,  /* SVE [<Xn|SP>, Zm.<T>, (S|U)XTW].
				       Bit 14 controls S/U choice.  */
  AARCH64_OPND_SVE_ADDR_RZ_XTW_22,  /* SVE [<Xn|SP>, Zm.<T>, (S|U)XTW].
				       Bit 22 controls S/U choice.  */
  AARCH64_OPND_SVE_ADDR_RZ_XTW1_14, /* SVE [<Xn|SP>, Zm.<T>, (S|U)XTW #1].
				       Bit 14 controls S/U choice.  */
  AARCH64_OPND_SVE_ADDR_RZ_XTW1_22, /* SVE [<Xn|SP>, Zm.<T>, (S|U)XTW #1].
				       Bit 22 controls S/U choice.  */
  AARCH64_OPND_SVE_ADDR_RZ_XTW2_14, /* SVE [<Xn|SP>, Zm.<T>, (S|U)XTW #2].
				       Bit 14 controls S/U choice.  */
  AARCH64_OPND_SVE_ADDR_RZ_XTW2_22, /* SVE [<Xn|SP>, Zm.<T>, (S|U)XTW #2].
				       Bit 22 controls S/U choice.  */
  AARCH64_OPND_SVE_ADDR_RZ_XTW3_14, /* SVE [<Xn|SP>, Zm.<T>, (S|U)XTW #3].
				       Bit 14 controls S/U choice.  */
  AARCH64_OPND_SVE_ADDR_RZ_XTW3_22, /* SVE [<Xn|SP>, Zm.<T>, (S|U)XTW #3].
				       Bit 22 controls S/U choice.  */
  AARCH64_OPND_SVE_ADDR_ZI_U5,	    /* SVE [Zn.<T>, #<uimm5>].  */
  AARCH64_OPND_SVE_ADDR_ZI_U5x2,    /* SVE [Zn.<T>, #<uimm5>*2].  */
  AARCH64_OPND_SVE_ADDR_ZI_U5x4,    /* SVE [Zn.<T>, #<uimm5>*4].  */
  AARCH64_OPND_SVE_ADDR_ZI_U5x8,    /* SVE [Zn.<T>, #<uimm5>*8].  */
  AARCH64_OPND_SVE_ADDR_ZZ_LSL,     /* SVE [Zn.<T>, Zm,<T>, LSL #<msz>].  */
  AARCH64_OPND_SVE_ADDR_ZZ_SXTW,    /* SVE [Zn.<T>, Zm,<T>, SXTW #<msz>].  */
  AARCH64_OPND_SVE_ADDR_ZZ_UXTW,    /* SVE [Zn.<T>, Zm,<T>, UXTW #<msz>].  */
  AARCH64_OPND_SVE_AIMM,	/* SVE unsigned arithmetic immediate.  */
  AARCH64_OPND_SVE_ASIMM,	/* SVE signed arithmetic immediate.  */
  AARCH64_OPND_SVE_FPIMM8,	/* SVE 8-bit floating-point immediate.  */
  AARCH64_OPND_SVE_I1_HALF_ONE,	/* SVE choice between 0.5 and 1.0.  */
  AARCH64_OPND_SVE_I1_HALF_TWO,	/* SVE choice between 0.5 and 2.0.  */
  AARCH64_OPND_SVE_I1_ZERO_ONE,	/* SVE choice between 0.0 and 1.0.  */
  AARCH64_OPND_SVE_IMM_ROT1,	/* SVE 1-bit rotate operand (90 or 270).  */
  AARCH64_OPND_SVE_IMM_ROT2,	/* SVE 2-bit rotate operand (N*90).  */
  AARCH64_OPND_SVE_IMM_ROT3,	/* SVE cadd 1-bit rotate (90 or 270).  */
  AARCH64_OPND_SVE_INV_LIMM,	/* SVE inverted logical immediate.  */
  AARCH64_OPND_SVE_LIMM,	/* SVE logical immediate.  */
  AARCH64_OPND_SVE_LIMM_MOV,	/* SVE logical immediate for MOV.  */
  AARCH64_OPND_SVE_PATTERN,	/* SVE vector pattern enumeration.  */
  AARCH64_OPND_SVE_PATTERN_SCALED, /* Likewise, with additional MUL factor.  */
  AARCH64_OPND_SVE_PRFOP,	/* SVE prefetch operation.  */
  AARCH64_OPND_SVE_Pd,		/* SVE p0-p15 in Pd.  */
  AARCH64_OPND_SVE_PNd,		/* SVE pn0-pn15 in Pd.  */
  AARCH64_OPND_SVE_Pg3,		/* SVE p0-p7 in Pg.  */
  AARCH64_OPND_SVE_Pg4_5,	/* SVE p0-p15 in Pg, bits [8,5].  */
  AARCH64_OPND_SVE_Pg4_10,	/* SVE p0-p15 in Pg, bits [13,10].  */
  AARCH64_OPND_SVE_PNg4_10,	/* SVE pn0-pn15 in Pg, bits [13,10].  */
  AARCH64_OPND_SVE_Pg4_16,	/* SVE p0-p15 in Pg, bits [19,16].  */
  AARCH64_OPND_SVE_Pm,		/* SVE p0-p15 in Pm.  */
  AARCH64_OPND_SVE_Pn,		/* SVE p0-p15 in Pn.  */
  AARCH64_OPND_SVE_PNn,		/* SVE pn0-pn15 in Pn.  */
  AARCH64_OPND_SVE_Pt,		/* SVE p0-p15 in Pt.  */
  AARCH64_OPND_SVE_PNt,		/* SVE pn0-pn15 in Pt.  */
  AARCH64_OPND_SVE_Rm,		/* Integer Rm or ZR, alt. SVE position.  */
  AARCH64_OPND_SVE_Rn_SP,	/* Integer Rn or SP, alt. SVE position.  */
  AARCH64_OPND_SVE_SHLIMM_PRED,	  /* SVE shift left amount (predicated).  */
  AARCH64_OPND_SVE_SHLIMM_UNPRED, /* SVE shift left amount (unpredicated).  */
  AARCH64_OPND_SVE_SHLIMM_UNPRED_22,	/* SVE 3 bit shift left unpred.  */
  AARCH64_OPND_SVE_SHRIMM_PRED,	  /* SVE shift right amount (predicated).  */
  AARCH64_OPND_SVE_SHRIMM_UNPRED, /* SVE shift right amount (unpredicated).  */
  AARCH64_OPND_SVE_SHRIMM_UNPRED_22,	/* SVE 3 bit shift right unpred.  */
  AARCH64_OPND_SVE_SIMM5,	/* SVE signed 5-bit immediate.  */
  AARCH64_OPND_SVE_SIMM5B,	/* SVE secondary signed 5-bit immediate.  */
  AARCH64_OPND_SVE_SIMM6,	/* SVE signed 6-bit immediate.  */
  AARCH64_OPND_SVE_SIMM8,	/* SVE signed 8-bit immediate.  */
  AARCH64_OPND_SVE_UIMM3,	/* SVE unsigned 3-bit immediate.  */
  AARCH64_OPND_SVE_UIMM7,	/* SVE unsigned 7-bit immediate.  */
  AARCH64_OPND_SVE_UIMM8,	/* SVE unsigned 8-bit immediate.  */
  AARCH64_OPND_SVE_UIMM8_53,	/* SVE split unsigned 8-bit immediate.  */
  AARCH64_OPND_SVE_VZn,		/* Scalar SIMD&FP register in Zn field.  */
  AARCH64_OPND_SVE_Vd,		/* Scalar SIMD&FP register in Vd.  */
  AARCH64_OPND_SVE_Vm,		/* Scalar SIMD&FP register in Vm.  */
  AARCH64_OPND_SVE_Vn,		/* Scalar SIMD&FP register in Vn.  */
  AARCH64_OPND_SVE_Za_5,	/* SVE vector register in Za, bits [9,5].  */
  AARCH64_OPND_SVE_Za_16,	/* SVE vector register in Za, bits [20,16].  */
  AARCH64_OPND_SVE_Zd,		/* SVE vector register in Zd.  */
  AARCH64_OPND_SVE_Zm_5,	/* SVE vector register in Zm, bits [9,5].  */
  AARCH64_OPND_SVE_Zm_16,	/* SVE vector register in Zm, bits [20,16].  */
  AARCH64_OPND_SVE_Zm3_INDEX,	/* z0-z7[0-3] in Zm, bits [20,16].  */
  AARCH64_OPND_SVE_Zm3_11_INDEX, /* z0-z7[0-7] in Zm3_INDEX plus bit 11.  */
  AARCH64_OPND_SVE_Zm3_19_INDEX, /* z0-z7[0-3] in Zm3_INDEX plus bit 19.  */
  AARCH64_OPND_SVE_Zm3_22_INDEX, /* z0-z7[0-7] in Zm3_INDEX plus bit 22.  */
  AARCH64_OPND_SVE_Zm4_11_INDEX, /* z0-z15[0-3] in Zm plus bit 11.  */
  AARCH64_OPND_SVE_Zm4_INDEX,	/* z0-z15[0-1] in Zm, bits [20,16].  */
  AARCH64_OPND_SVE_Zn,		/* SVE vector register in Zn.  */
  AARCH64_OPND_SVE_Zn_INDEX,	/* Indexed SVE vector register, for DUP.  */
  AARCH64_OPND_SVE_ZnxN,	/* SVE vector register list in Zn.  */
  AARCH64_OPND_SVE_Zt,		/* SVE vector register in Zt.  */
  AARCH64_OPND_SVE_ZtxN,	/* SVE vector register list in Zt.  */
  AARCH64_OPND_SME_Zdnx2,	/* SVE vector register list from [4:1]*2.  */
  AARCH64_OPND_SME_Zdnx4,	/* SVE vector register list from [4:2]*4.  */
  AARCH64_OPND_SME_Zm,		/* SVE vector register list in 4-bit Zm.  */
  AARCH64_OPND_SME_Zmx2,	/* SVE vector register list from [20:17]*2.  */
  AARCH64_OPND_SME_Zmx4,	/* SVE vector register list from [20:18]*4.  */
  AARCH64_OPND_SME_Znx2,	/* SVE vector register list from [9:6]*2.  */
  AARCH64_OPND_SME_Znx4,	/* SVE vector register list from [9:7]*4.  */
  AARCH64_OPND_SME_Ztx2_STRIDED, /* SVE vector register list in [4:0]&23.  */
  AARCH64_OPND_SME_Ztx4_STRIDED, /* SVE vector register list in [4:0]&19.  */
  AARCH64_OPND_SME_ZAda_2b,	/* SME <ZAda>.S, 2-bits.  */
  AARCH64_OPND_SME_ZAda_3b,	/* SME <ZAda>.D, 3-bits.  */
  AARCH64_OPND_SME_ZA_HV_idx_src,	/* SME source ZA tile vector.  */
  AARCH64_OPND_SME_ZA_HV_idx_srcxN,	/* SME N source ZA tile vectors.  */
  AARCH64_OPND_SME_ZA_HV_idx_dest,	/* SME destination ZA tile vector.  */
  AARCH64_OPND_SME_ZA_HV_idx_destxN,	/* SME N dest ZA tile vectors.  */
  AARCH64_OPND_SME_Pdx2,	/* Predicate register list in [3:1].  */
  AARCH64_OPND_SME_PdxN,	/* Predicate register list in [3:0].  */
  AARCH64_OPND_SME_Pm,		/* SME scalable predicate register, bits [15:13].  */
  AARCH64_OPND_SME_PNd3,	/* Predicate-as-counter register, bits [3:0].  */
  AARCH64_OPND_SME_PNg3,	/* Predicate-as-counter register, bits [12:10].  */
  AARCH64_OPND_SME_PNn,		/* Predicate-as-counter register, bits [8:5].  */
  AARCH64_OPND_SME_PNn3_INDEX1,	/* Indexed pred-as-counter reg, bits [8:5].  */
  AARCH64_OPND_SME_PNn3_INDEX2,	/* Indexed pred-as-counter reg, bits [9:5].  */
  AARCH64_OPND_SME_list_of_64bit_tiles, /* SME list of ZA tiles.  */
  AARCH64_OPND_SME_ZA_HV_idx_ldstr, /* SME destination ZA tile vector.  */
  AARCH64_OPND_SME_ZA_array_off1x4, /* SME ZA[<Wv>, #<imm1>*4:<imm1>*4+3].  */
  AARCH64_OPND_SME_ZA_array_off2x2, /* SME ZA[<Wv>, #<imm2>*2:<imm2>*2+1].  */
  AARCH64_OPND_SME_ZA_array_off2x4, /* SME ZA[<Wv>, #<imm2>*4:<imm2>*4+3].  */
  AARCH64_OPND_SME_ZA_array_off3_0, /* SME ZA[<Wv>{, #<imm3>}].  */
  AARCH64_OPND_SME_ZA_array_off3_5, /* SME ZA[<Wv>{, #<imm3>}].  */
  AARCH64_OPND_SME_ZA_array_off3x2, /* SME ZA[<Wv>, #<imm3>*2:<imm3>*2+1].  */
  AARCH64_OPND_SME_ZA_array_off4,   /* SME ZA[<Wv>{, #<imm>}].  */
  AARCH64_OPND_SME_ADDR_RI_U4xVL,   /* SME [<Xn|SP>{, #<imm>, MUL VL}].  */
  AARCH64_OPND_SME_SM_ZA,           /* SME {SM | ZA}.  */
  AARCH64_OPND_SME_PnT_Wm_imm,      /* SME <Pn>.<T>[<Wm>, #<imm>].  */
  AARCH64_OPND_SME_SHRIMM4,	    /* 4-bit right shift, bits [19:16].  */
  AARCH64_OPND_SME_SHRIMM5,	    /* size + 5-bit right shift, bits [23:22,20:16].  */
  AARCH64_OPND_SME_Zm_INDEX1,	    /* Zn.T[index], bits [19:16,10].  */
  AARCH64_OPND_SME_Zm_INDEX2,	    /* Zn.T[index], bits [19:16,11:10].  */
  AARCH64_OPND_SME_Zm_INDEX3_1,     /* Zn.T[index], bits [19:16,10,2:1].  */
  AARCH64_OPND_SME_Zm_INDEX3_2,     /* Zn.T[index], bits [19:16,11:10,2].  */
  AARCH64_OPND_SME_Zm_INDEX3_10,    /* Zn.T[index], bits [19:16,15,11:10].  */
  AARCH64_OPND_SME_Zm_INDEX4_1,     /* Zn.T[index], bits [19:16,11:10,2:1].  */
  AARCH64_OPND_SME_Zm_INDEX4_10,    /* Zn.T[index], bits [19:16,15,12:10].  */
  AARCH64_OPND_SME_Zn_INDEX1_16,    /* Zn[index], bits [9:5] and [16:16].  */
  AARCH64_OPND_SME_Zn_INDEX2_15,    /* Zn[index], bits [9:5] and [16:15].  */
  AARCH64_OPND_SME_Zn_INDEX2_16,    /* Zn[index], bits [9:5] and [17:16].  */
  AARCH64_OPND_SME_Zn_INDEX3_14,    /* Zn[index], bits [9:5] and [16:14].  */
  AARCH64_OPND_SME_Zn_INDEX3_15,    /* Zn[index], bits [9:5] and [17:15].  */
  AARCH64_OPND_SME_Zn_INDEX4_14,    /* Zn[index], bits [9:5] and [17:14].  */
  AARCH64_OPND_SME_VLxN_10,	/* VLx2 or VLx4, in bit 10.  */
  AARCH64_OPND_SME_VLxN_13,	/* VLx2 or VLx4, in bit 13.  */
  AARCH64_OPND_SME_ZT0,		/* The fixed token zt0/ZT0 (not encoded).  */
  AARCH64_OPND_SME_ZT0_INDEX,	/* ZT0[<imm>], bits [14:12].  */
  AARCH64_OPND_SME_ZT0_LIST,	/* { zt0/ZT0 } (not encoded).  */
  AARCH64_OPND_TME_UIMM16,	/* TME unsigned 16-bit immediate.  */
  AARCH64_OPND_SM3_IMM2,	/* SM3 encodes lane in bits [13, 14].  */
  AARCH64_OPND_MOPS_ADDR_Rd,	/* [Rd]!, in bits [0, 4].  */
  AARCH64_OPND_MOPS_ADDR_Rs,	/* [Rs]!, in bits [16, 20].  */
  AARCH64_OPND_MOPS_WB_Rn,	/* Rn!, in bits [5, 9].  */
  AARCH64_OPND_CSSC_SIMM8,	/* CSSC signed 8-bit immediate.  */
  AARCH64_OPND_CSSC_UIMM8,	/* CSSC unsigned 8-bit immediate.  */
};

/* Qualifier constrains an operand.  It either specifies a variant of an
   operand type or limits values available to an operand type.

   N.B. Order is important; keep aarch64_opnd_qualifiers synced.  */

enum aarch64_opnd_qualifier
{
  /* Indicating no further qualification on an operand.  */
  AARCH64_OPND_QLF_NIL,

  /* Qualifying an operand which is a general purpose (integer) register;
     indicating the operand data size or a specific register.  */
  AARCH64_OPND_QLF_W,	/* Wn, WZR or WSP.  */
  AARCH64_OPND_QLF_X,	/* Xn, XZR or XSP.  */
  AARCH64_OPND_QLF_WSP,	/* WSP.  */
  AARCH64_OPND_QLF_SP,	/* SP.  */

  /* Qualifying an operand which is a floating-point register, a SIMD
     vector element or a SIMD vector element list; indicating operand data
     size or the size of each SIMD vector element in the case of a SIMD
     vector element list.
     These qualifiers are also used to qualify an address operand to
     indicate the size of data element a load/store instruction is
     accessing.
     They are also used for the immediate shift operand in e.g. SSHR.  Such
     a use is only for the ease of operand encoding/decoding and qualifier
     sequence matching; such a use should not be applied widely; use the value
     constraint qualifiers for immediate operands wherever possible.  */
  AARCH64_OPND_QLF_S_B,
  AARCH64_OPND_QLF_S_H,
  AARCH64_OPND_QLF_S_S,
  AARCH64_OPND_QLF_S_D,
  AARCH64_OPND_QLF_S_Q,
  /* These type qualifiers have a special meaning in that they mean 4 x 1 byte
     or 2 x 2 byte are selected by the instruction.  Other than that they have
     no difference with AARCH64_OPND_QLF_S_B in encoding.  They are here purely
     for syntactical reasons and is an exception from normal AArch64
     disassembly scheme.  */
  AARCH64_OPND_QLF_S_4B,
  AARCH64_OPND_QLF_S_2H,

  /* Qualifying an operand which is a SIMD vector register or a SIMD vector
     register list; indicating register shape.
     They are also used for the immediate shift operand in e.g. SSHR.  Such
     a use is only for the ease of operand encoding/decoding and qualifier
     sequence matching; such a use should not be applied widely; use the value
     constraint qualifiers for immediate operands wherever possible.  */
  AARCH64_OPND_QLF_V_4B,
  AARCH64_OPND_QLF_V_8B,
  AARCH64_OPND_QLF_V_16B,
  AARCH64_OPND_QLF_V_2H,
  AARCH64_OPND_QLF_V_4H,
  AARCH64_OPND_QLF_V_8H,
  AARCH64_OPND_QLF_V_2S,
  AARCH64_OPND_QLF_V_4S,
  AARCH64_OPND_QLF_V_1D,
  AARCH64_OPND_QLF_V_2D,
  AARCH64_OPND_QLF_V_1Q,

  AARCH64_OPND_QLF_P_Z,
  AARCH64_OPND_QLF_P_M,

  /* Used in scaled signed immediate that are scaled by a Tag granule
     like in stg, st2g, etc.   */
  AARCH64_OPND_QLF_imm_tag,

  /* Constraint on value.  */
  AARCH64_OPND_QLF_CR,		/* CRn, CRm. */
  AARCH64_OPND_QLF_imm_0_7,
  AARCH64_OPND_QLF_imm_0_15,
  AARCH64_OPND_QLF_imm_0_31,
  AARCH64_OPND_QLF_imm_0_63,
  AARCH64_OPND_QLF_imm_1_32,
  AARCH64_OPND_QLF_imm_1_64,

  /* Indicate whether an AdvSIMD modified immediate operand is shift-zeros
     or shift-ones.  */
  AARCH64_OPND_QLF_LSL,
  AARCH64_OPND_QLF_MSL,

  /* Special qualifier helping retrieve qualifier information during the
     decoding time (currently not in use).  */
  AARCH64_OPND_QLF_RETRIEVE,
};

/* Instruction class.  */

enum aarch64_insn_class
{
  aarch64_misc,
  addsub_carry,
  addsub_ext,
  addsub_imm,
  addsub_shift,
  asimdall,
  asimddiff,
  asimdelem,
  asimdext,
  asimdimm,
  asimdins,
  asimdmisc,
  asimdperm,
  asimdsame,
  asimdshf,
  asimdtbl,
  asisddiff,
  asisdelem,
  asisdlse,
  asisdlsep,
  asisdlso,
  asisdlsop,
  asisdmisc,
  asisdone,
  asisdpair,
  asisdsame,
  asisdshf,
  bitfield,
  branch_imm,
  branch_reg,
  compbranch,
  condbranch,
  condcmp_imm,
  condcmp_reg,
  condsel,
  cryptoaes,
  cryptosha2,
  cryptosha3,
  dp_1src,
  dp_2src,
  dp_3src,
  exception,
  extract,
  float2fix,
  float2int,
  floatccmp,
  floatcmp,
  floatdp1,
  floatdp2,
  floatdp3,
  floatimm,
  floatsel,
  ldst_immpost,
  ldst_immpre,
  ldst_imm9,	/* immpost or immpre */
  ldst_imm10,	/* LDRAA/LDRAB */
  ldst_pos,
  ldst_regoff,
  ldst_unpriv,
  ldst_unscaled,
  ldstexcl,
  ldstnapair_offs,
  ldstpair_off,
  ldstpair_indexed,
  loadlit,
  log_imm,
  log_shift,
  lse_atomic,
  movewide,
  pcreladdr,
  ic_system,
  sme_fp_sd,
  sme_int_sd,
  sme_misc,
  sme_mov,
  sme_ldr,
  sme_psel,
  sme_shift,
  sme_size_12_bhs,
  sme_size_12_hs,
  sme_size_22,
  sme_size_22_hsd,
  sme_sz_23,
  sme_str,
  sme_start,
  sme_stop,
  sme2_mov,
  sve_cpy,
  sve_index,
  sve_limm,
  sve_misc,
  sve_movprfx,
  sve_pred_zm,
  sve_shift_pred,
  sve_shift_unpred,
  sve_size_bhs,
  sve_size_bhsd,
  sve_size_hsd,
  sve_size_hsd2,
  sve_size_sd,
  sve_size_bh,
  sve_size_sd2,
  sve_size_13,
  sve_shift_tsz_hsd,
  sve_shift_tsz_bhsd,
  sve_size_tsz_bhs,
  testbranch,
  cryptosm3,
  cryptosm4,
  dotproduct,
  bfloat16,
  cssc,
};

/* Opcode enumerators.  */

enum aarch64_op
{
  OP_NIL,
  OP_STRB_POS,
  OP_LDRB_POS,
  OP_LDRSB_POS,
  OP_STRH_POS,
  OP_LDRH_POS,
  OP_LDRSH_POS,
  OP_STR_POS,
  OP_LDR_POS,
  OP_STRF_POS,
  OP_LDRF_POS,
  OP_LDRSW_POS,
  OP_PRFM_POS,

  OP_STURB,
  OP_LDURB,
  OP_LDURSB,
  OP_STURH,
  OP_LDURH,
  OP_LDURSH,
  OP_STUR,
  OP_LDUR,
  OP_STURV,
  OP_LDURV,
  OP_LDURSW,
  OP_PRFUM,

  OP_LDR_LIT,
  OP_LDRV_LIT,
  OP_LDRSW_LIT,
  OP_PRFM_LIT,

  OP_ADD,
  OP_B,
  OP_BL,

  OP_MOVN,
  OP_MOVZ,
  OP_MOVK,

  OP_MOV_IMM_LOG,	/* MOV alias for moving bitmask immediate.  */
  OP_MOV_IMM_WIDE,	/* MOV alias for moving wide immediate.  */
  OP_MOV_IMM_WIDEN,	/* MOV alias for moving wide immediate (negated).  */

  OP_MOV_V,		/* MOV alias for moving vector register.  */

  OP_ASR_IMM,
  OP_LSR_IMM,
  OP_LSL_IMM,

  OP_BIC,

  OP_UBFX,
  OP_BFXIL,
  OP_SBFX,
  OP_SBFIZ,
  OP_BFI,
  OP_BFC,		/* ARMv8.2.  */
  OP_UBFIZ,
  OP_UXTB,
  OP_UXTH,
  OP_UXTW,

  OP_CINC,
  OP_CINV,
  OP_CNEG,
  OP_CSET,
  OP_CSETM,

  OP_FCVT,
  OP_FCVTN,
  OP_FCVTN2,
  OP_FCVTL,
  OP_FCVTL2,
  OP_FCVTXN_S,		/* Scalar version.  */

  OP_ROR_IMM,

  OP_SXTL,
  OP_SXTL2,
  OP_UXTL,
  OP_UXTL2,

  OP_MOV_P_P,
  OP_MOV_PN_PN,
  OP_MOV_Z_P_Z,
  OP_MOV_Z_V,
  OP_MOV_Z_Z,
  OP_MOV_Z_Zi,
  OP_MOVM_P_P_P,
  OP_MOVS_P_P,
  OP_MOVZS_P_P_P,
  OP_MOVZ_P_P_P,
  OP_NOTS_P_P_P_Z,
  OP_NOT_P_P_P_Z,

  OP_FCMLA_ELEM,	/* ARMv8.3, indexed element version.  */

  OP_TOTAL_NUM,		/* Pseudo.  */
};

/* Error types.  */
enum err_type
{
  ERR_OK,
  ERR_UND,
  ERR_UNP,
  ERR_NYI,
  ERR_VFI,
  ERR_NR_ENTRIES
};

/* Maximum number of operands an instruction can have.  */
#define AARCH64_MAX_OPND_NUM 6
/* Maximum number of qualifier sequences an instruction can have.  */
#define AARCH64_MAX_QLF_SEQ_NUM 10
/* Operand qualifier typedef; optimized for the size.  */
typedef unsigned char aarch64_opnd_qualifier_t;
/* Operand qualifier sequence typedef.  */
typedef aarch64_opnd_qualifier_t	\
	  aarch64_opnd_qualifier_seq_t [AARCH64_MAX_OPND_NUM];

/* FIXME: improve the efficiency.  */
static inline bool
empty_qualifier_sequence_p (const aarch64_opnd_qualifier_t *qualifiers)
{
  int i;
  for (i = 0; i < AARCH64_MAX_OPND_NUM; ++i)
    if (qualifiers[i] != AARCH64_OPND_QLF_NIL)
      return false;
  return true;
}

/*  Forward declare error reporting type.  */
typedef struct aarch64_operand_error aarch64_operand_error;
/* Forward declare instruction sequence type.  */
typedef struct aarch64_instr_sequence aarch64_instr_sequence;
/* Forward declare instruction definition.  */
typedef struct aarch64_inst aarch64_inst;

/* This structure holds information for a particular opcode.  */

struct aarch64_opcode
{
  /* The name of the mnemonic.  */
  const char *name;

  /* The opcode itself.  Those bits which will be filled in with
     operands are zeroes.  */
  aarch64_insn opcode;

  /* The opcode mask.  This is used by the disassembler.  This is a
     mask containing ones indicating those bits which must match the
     opcode field, and zeroes indicating those bits which need not
     match (and are presumably filled in by operands).  */
  aarch64_insn mask;

  /* Instruction class.  */
  enum aarch64_insn_class iclass;

  /* Enumerator identifier.  */
  enum aarch64_op op;

  /* Which architecture variant provides this instruction.  */
  const aarch64_feature_set *avariant;

  /* An array of operand codes.  Each code is an index into the
     operand table.  They appear in the order which the operands must
     appear in assembly code, and are terminated by a zero.  */
  enum aarch64_opnd operands[AARCH64_MAX_OPND_NUM];

  /* A list of operand qualifier code sequence.  Each operand qualifier
     code qualifies the corresponding operand code.  Each operand
     qualifier sequence specifies a valid opcode variant and related
     constraint on operands.  */
  aarch64_opnd_qualifier_seq_t qualifiers_list[AARCH64_MAX_QLF_SEQ_NUM];

  /* Flags providing information about this instruction */
  uint64_t flags;

  /* Extra constraints on the instruction that the verifier checks.  */
  uint32_t constraints;

  /* If nonzero, this operand and operand 0 are both registers and
     are required to have the same register number.  */
  unsigned char tied_operand;

  /* If non-NULL, a function to verify that a given instruction is valid.  */
  enum err_type (* verifier) (const struct aarch64_inst *, const aarch64_insn,
			      bfd_vma, bool, aarch64_operand_error *,
			      struct aarch64_instr_sequence *);
};

typedef struct aarch64_opcode aarch64_opcode;

/* Table describing all the AArch64 opcodes.  */
extern const aarch64_opcode aarch64_opcode_table[];

/* Opcode flags.  */
#define F_ALIAS (1 << 0)
#define F_HAS_ALIAS (1 << 1)
/* Disassembly preference priority 1-3 (the larger the higher).  If nothing
   is specified, it is the priority 0 by default, i.e. the lowest priority.  */
#define F_P1 (1 << 2)
#define F_P2 (2 << 2)
#define F_P3 (3 << 2)
/* Flag an instruction that is truly conditional executed, e.g. b.cond.  */
#define F_COND (1 << 4)
/* Instruction has the field of 'sf'.  */
#define F_SF (1 << 5)
/* Instruction has the field of 'size:Q'.  */
#define F_SIZEQ (1 << 6)
/* Floating-point instruction has the field of 'type'.  */
#define F_FPTYPE (1 << 7)
/* AdvSIMD scalar instruction has the field of 'size'.  */
#define F_SSIZE (1 << 8)
/* AdvSIMD vector register arrangement specifier encoded in "imm5<3:0>:Q".  */
#define F_T (1 << 9)
/* Size of GPR operand in AdvSIMD instructions encoded in Q.  */
#define F_GPRSIZE_IN_Q (1 << 10)
/* Size of Rt load signed instruction encoded in opc[0], i.e. bit 22.  */
#define F_LDS_SIZE (1 << 11)
/* Optional operand; assume maximum of 1 operand can be optional.  */
#define F_OPD0_OPT (1 << 12)
#define F_OPD1_OPT (2 << 12)
#define F_OPD2_OPT (3 << 12)
#define F_OPD3_OPT (4 << 12)
#define F_OPD4_OPT (5 << 12)
/* Default value for the optional operand when omitted from the assembly.  */
#define F_DEFAULT(X) (((X) & 0x1f) << 15)
/* Instruction that is an alias of another instruction needs to be
   encoded/decoded by converting it to/from the real form, followed by
   the encoding/decoding according to the rules of the real opcode.
   This compares to the direct coding using the alias's information.
   N.B. this flag requires F_ALIAS to be used together.  */
#define F_CONV (1 << 20)
/* Use together with F_ALIAS to indicate an alias opcode is a programmer
   friendly pseudo instruction available only in the assembly code (thus will
   not show up in the disassembly).  */
#define F_PSEUDO (1 << 21)
/* Instruction has miscellaneous encoding/decoding rules.  */
#define F_MISC (1 << 22)
/* Instruction has the field of 'N'; used in conjunction with F_SF.  */
#define F_N (1 << 23)
/* Opcode dependent field.  */
#define F_OD(X) (((X) & 0x7) << 24)
/* Instruction has the field of 'sz'.  */
#define F_LSE_SZ (1 << 27)
/* Require an exact qualifier match, even for NIL qualifiers.  */
#define F_STRICT (1ULL << 28)
/* This system instruction is used to read system registers.  */
#define F_SYS_READ (1ULL << 29)
/* This system instruction is used to write system registers.  */
#define F_SYS_WRITE (1ULL << 30)
/* This instruction has an extra constraint on it that imposes a requirement on
   subsequent instructions.  */
#define F_SCAN (1ULL << 31)
/* Next bit is 32.  */

/* Instruction constraints.  */
/* This instruction has a predication constraint on the instruction at PC+4.  */
#define C_SCAN_MOVPRFX (1U << 0)
/* This instruction's operation width is determined by the operand with the
   largest element size.  */
#define C_MAX_ELEM (1U << 1)
#define C_SCAN_MOPS_P (1U << 2)
#define C_SCAN_MOPS_M (2U << 2)
#define C_SCAN_MOPS_E (3U << 2)
#define C_SCAN_MOPS_PME (3U << 2)
/* Next bit is 4.  */

static inline bool
alias_opcode_p (const aarch64_opcode *opcode)
{
  return (opcode->flags & F_ALIAS) != 0;
}

static inline bool
opcode_has_alias (const aarch64_opcode *opcode)
{
  return (opcode->flags & F_HAS_ALIAS) != 0;
}

/* Priority for disassembling preference.  */
static inline int
opcode_priority (const aarch64_opcode *opcode)
{
  return (opcode->flags >> 2) & 0x3;
}

static inline bool
pseudo_opcode_p (const aarch64_opcode *opcode)
{
  return (opcode->flags & F_PSEUDO) != 0lu;
}

static inline bool
optional_operand_p (const aarch64_opcode *opcode, unsigned int idx)
{
  return ((opcode->flags >> 12) & 0x7) == idx + 1;
}

static inline aarch64_insn
get_optional_operand_default_value (const aarch64_opcode *opcode)
{
  return (opcode->flags >> 15) & 0x1f;
}

static inline unsigned int
get_opcode_dependent_value (const aarch64_opcode *opcode)
{
  return (opcode->flags >> 24) & 0x7;
}

static inline bool
opcode_has_special_coder (const aarch64_opcode *opcode)
{
  return (opcode->flags & (F_SF | F_LSE_SZ | F_SIZEQ | F_FPTYPE | F_SSIZE | F_T
	  | F_GPRSIZE_IN_Q | F_LDS_SIZE | F_MISC | F_N | F_COND)) != 0;
}

struct aarch64_name_value_pair
{
  const char *  name;
  aarch64_insn	value;
};

extern const struct aarch64_name_value_pair aarch64_operand_modifiers [];
extern const struct aarch64_name_value_pair aarch64_barrier_options [16];
extern const struct aarch64_name_value_pair aarch64_barrier_dsb_nxs_options [4];
extern const struct aarch64_name_value_pair aarch64_prfops [32];
extern const struct aarch64_name_value_pair aarch64_hint_options [];

#define AARCH64_MAX_SYSREG_NAME_LEN 32

typedef struct
{
  const char *  name;
  aarch64_insn	value;
  uint32_t	flags;

  /* A set of features, all of which are required for this system register to be
     available.  */
  aarch64_feature_set features;
} aarch64_sys_reg;

extern const aarch64_sys_reg aarch64_sys_regs [];
extern const aarch64_sys_reg aarch64_pstatefields [];
extern bool aarch64_sys_reg_deprecated_p (const uint32_t);
extern bool aarch64_pstatefield_supported_p (const aarch64_feature_set,
					     const aarch64_sys_reg *);

typedef struct
{
  const char *name;
  uint32_t value;
  uint32_t flags ;
} aarch64_sys_ins_reg;

extern bool aarch64_sys_ins_reg_has_xt (const aarch64_sys_ins_reg *);
extern bool
aarch64_sys_ins_reg_supported_p (const aarch64_feature_set,
				 const char *reg_name, aarch64_insn,
                                 uint32_t, aarch64_feature_set);

extern const aarch64_sys_ins_reg aarch64_sys_regs_ic [];
extern const aarch64_sys_ins_reg aarch64_sys_regs_dc [];
extern const aarch64_sys_ins_reg aarch64_sys_regs_at [];
extern const aarch64_sys_ins_reg aarch64_sys_regs_tlbi [];
extern const aarch64_sys_ins_reg aarch64_sys_regs_sr [];

/* Shift/extending operator kinds.
   N.B. order is important; keep aarch64_operand_modifiers synced.  */
enum aarch64_modifier_kind
{
  AARCH64_MOD_NONE,
  AARCH64_MOD_MSL,
  AARCH64_MOD_ROR,
  AARCH64_MOD_ASR,
  AARCH64_MOD_LSR,
  AARCH64_MOD_LSL,
  AARCH64_MOD_UXTB,
  AARCH64_MOD_UXTH,
  AARCH64_MOD_UXTW,
  AARCH64_MOD_UXTX,
  AARCH64_MOD_SXTB,
  AARCH64_MOD_SXTH,
  AARCH64_MOD_SXTW,
  AARCH64_MOD_SXTX,
  AARCH64_MOD_MUL,
  AARCH64_MOD_MUL_VL,
};

bool
aarch64_extend_operator_p (enum aarch64_modifier_kind);

enum aarch64_modifier_kind
aarch64_get_operand_modifier (const struct aarch64_name_value_pair *);
/* Condition.  */

typedef struct
{
  /* A list of names with the first one as the disassembly preference;
     terminated by NULL if fewer than 3.  */
  const char *names[4];
  aarch64_insn value;
} aarch64_cond;

extern const aarch64_cond aarch64_conds[16];

const aarch64_cond* get_cond_from_value (aarch64_insn value);
const aarch64_cond* get_inverted_cond (const aarch64_cond *cond);

/* Information about a reference to part of ZA.  */
struct aarch64_indexed_za
{
  /* Which tile is being accessed.  Unused (and 0) for an index into ZA.  */
  int regno;

  struct
  {
    /* The 32-bit index register.  */
    int regno;

    /* The first (or only) immediate offset.  */
    int64_t imm;

    /* The last immediate offset minus the first immediate offset.
       Unlike the range size, this is guaranteed not to overflow
       when the end offset > the start offset.  */
    uint64_t countm1;
  } index;

  /* The vector group size, or 0 if none.  */
  unsigned group_size : 8;

  /* True if a tile access is vertical, false if it is horizontal.
     Unused (and 0) for an index into ZA.  */
  unsigned v : 1;
};

/* Information about a list of registers.  */
struct aarch64_reglist
{
  unsigned first_regno : 8;
  unsigned num_regs : 8;
  /* The difference between the nth and the n+1th register.  */
  unsigned stride : 8;
  /* 1 if it is a list of reg element.  */
  unsigned has_index : 1;
  /* Lane index; valid only when has_index is 1.  */
  int64_t index;
};

/* Structure representing an operand.  */

struct aarch64_opnd_info
{
  enum aarch64_opnd type;
  aarch64_opnd_qualifier_t qualifier;
  int idx;

  union
    {
      struct
	{
	  unsigned regno;
	} reg;
      struct
	{
	  unsigned int regno;
	  int64_t index;
	} reglane;
      /* e.g. LVn.  */
      struct aarch64_reglist reglist;
      /* e.g. immediate or pc relative address offset.  */
      struct
	{
	  int64_t value;
	  unsigned is_fp : 1;
	} imm;
      /* e.g. address in STR (register offset).  */
      struct
	{
	  unsigned base_regno;
	  struct
	    {
	      union
		{
		  int imm;
		  unsigned regno;
		};
	      unsigned is_reg;
	    } offset;
	  unsigned pcrel : 1;		/* PC-relative.  */
	  unsigned writeback : 1;
	  unsigned preind : 1;		/* Pre-indexed.  */
	  unsigned postind : 1;		/* Post-indexed.  */
	} addr;

      struct
	{
	  /* The encoding of the system register.  */
	  aarch64_insn value;

	  /* The system register flags.  */
	  uint32_t flags;
	} sysreg;

      /* ZA tile vector, e.g. <ZAn><HV>.D[<Wv>{, <imm>}]  */
      struct aarch64_indexed_za indexed_za;

      const aarch64_cond *cond;
      /* The encoding of the PSTATE field.  */
      aarch64_insn pstatefield;
      const aarch64_sys_ins_reg *sysins_op;
      const struct aarch64_name_value_pair *barrier;
      const struct aarch64_name_value_pair *hint_option;
      const struct aarch64_name_value_pair *prfop;
    };

  /* Operand shifter; in use when the operand is a register offset address,
     add/sub extended reg, etc. e.g. <R><m>{, <extend> {#<amount>}}.  */
  struct
    {
      enum aarch64_modifier_kind kind;
      unsigned operator_present: 1;	/* Only valid during encoding.  */
      /* Value of the 'S' field in ld/st reg offset; used only in decoding.  */
      unsigned amount_present: 1;
      int64_t amount;
    } shifter;

  unsigned skip:1;	/* Operand is not completed if there is a fixup needed
			   to be done on it.  In some (but not all) of these
			   cases, we need to tell libopcodes to skip the
			   constraint checking and the encoding for this
			   operand, so that the libopcodes can pick up the
			   right opcode before the operand is fixed-up.  This
			   flag should only be used during the
			   assembling/encoding.  */
  unsigned present:1;	/* Whether this operand is present in the assembly
			   line; not used during the disassembly.  */
};

typedef struct aarch64_opnd_info aarch64_opnd_info;

/* Structure representing an instruction.

   It is used during both the assembling and disassembling.  The assembler
   fills an aarch64_inst after a successful parsing and then passes it to the
   encoding routine to do the encoding.  During the disassembling, the
   disassembler calls the decoding routine to decode a binary instruction; on a
   successful return, such a structure will be filled with information of the
   instruction; then the disassembler uses the information to print out the
   instruction.  */

struct aarch64_inst
{
  /* The value of the binary instruction.  */
  aarch64_insn value;

  /* Corresponding opcode entry.  */
  const aarch64_opcode *opcode;

  /* Condition for a truly conditional-executed instrutions, e.g. b.cond.  */
  const aarch64_cond *cond;

  /* Operands information.  */
  aarch64_opnd_info operands[AARCH64_MAX_OPND_NUM];
};

/* Defining the HINT #imm values for the aarch64_hint_options.  */
#define HINT_OPD_CSYNC	0x11
#define HINT_OPD_C	0x22
#define HINT_OPD_J	0x24
#define HINT_OPD_JC	0x26
#define HINT_OPD_NULL	0x00


/* Diagnosis related declaration and interface.  */

/* Operand error kind enumerators.

   AARCH64_OPDE_RECOVERABLE
     Less severe error found during the parsing, very possibly because that
     GAS has picked up a wrong instruction template for the parsing.

   AARCH64_OPDE_A_SHOULD_FOLLOW_B
     The instruction forms (or is expected to form) part of a sequence,
     but the preceding instruction in the sequence wasn't the expected one.
     The message refers to two strings: the name of the current instruction,
     followed by the name of the expected preceding instruction.

   AARCH64_OPDE_EXPECTED_A_AFTER_B
     Same as AARCH64_OPDE_A_SHOULD_FOLLOW_B, but shifting the focus
     so that the current instruction is assumed to be the incorrect one:
     "since the previous instruction was B, the current one should be A".

   AARCH64_OPDE_SYNTAX_ERROR
     General syntax error; it can be either a user error, or simply because
     that GAS is trying a wrong instruction template.

   AARCH64_OPDE_FATAL_SYNTAX_ERROR
     Definitely a user syntax error.

   AARCH64_OPDE_INVALID_VARIANT
     No syntax error, but the operands are not a valid combination, e.g.
     FMOV D0,S0

   The following errors are only reported against an asm string that is
   syntactically valid and that has valid operand qualifiers.

   AARCH64_OPDE_INVALID_VG_SIZE
     Error about a "VGx<n>" modifier in a ZA index not having the
     correct <n>.  This error effectively forms a pair with
     AARCH64_OPDE_REG_LIST_LENGTH, since both errors relate to the number
     of vectors that an instruction operates on.  However, the "VGx<n>"
     modifier is optional, whereas a register list always has a known
     and explicit length.  It therefore seems better to place more
     importance on the register list length when selecting an opcode table
     entry.  This in turn means that having an incorrect register length
     should be more severe than having an incorrect "VGx<n>".

   AARCH64_OPDE_REG_LIST_LENGTH
     Error about a register list operand having an unexpected number of
     registers.  This error is low severity because there might be another
     opcode entry that supports the given number of registers.

   AARCH64_OPDE_REG_LIST_STRIDE
     Error about a register list operand having the correct number
     (and type) of registers, but an unexpected stride.  This error is
     more severe than AARCH64_OPDE_REG_LIST_LENGTH because it implies
     that the length is known to be correct.  However, it is lower than
     many other errors, since some instructions have forms that share
     the same number of registers but have different strides.

   AARCH64_OPDE_UNTIED_IMMS
     The asm failed to use the same immediate for a destination operand
     and a tied source operand.

   AARCH64_OPDE_UNTIED_OPERAND
     The asm failed to use the same register for a destination operand
     and a tied source operand.

   AARCH64_OPDE_OUT_OF_RANGE
     Error about some immediate value out of a valid range.

   AARCH64_OPDE_UNALIGNED
     Error about some immediate value not properly aligned (i.e. not being a
     multiple times of a certain value).

   AARCH64_OPDE_OTHER_ERROR
     Error of the highest severity and used for any severe issue that does not
     fall into any of the above categories.

   AARCH64_OPDE_INVALID_REGNO
     A register was syntactically valid and had the right type, but it was
     outside the range supported by the associated operand field.  This is
     a high severity error because there are currently no instructions that
     would accept the operands that precede the erroneous one (if any) and
     yet still accept a wider range of registers.

   AARCH64_OPDE_RECOVERABLE, AARCH64_OPDE_SYNTAX_ERROR and
   AARCH64_OPDE_FATAL_SYNTAX_ERROR are only deteced by GAS while the
   AARCH64_OPDE_INVALID_VARIANT error can only be spotted by libopcodes as
   only libopcodes has the information about the valid variants of each
   instruction.

   The enumerators have an increasing severity.  This is helpful when there are
   multiple instruction templates available for a given mnemonic name (e.g.
   FMOV); this mechanism will help choose the most suitable template from which
   the generated diagnostics can most closely describe the issues, if any.

   This enum needs to be kept up-to-date with operand_mismatch_kind_names
   in tc-aarch64.c.  */

enum aarch64_operand_error_kind
{
  AARCH64_OPDE_NIL,
  AARCH64_OPDE_RECOVERABLE,
  AARCH64_OPDE_A_SHOULD_FOLLOW_B,
  AARCH64_OPDE_EXPECTED_A_AFTER_B,
  AARCH64_OPDE_SYNTAX_ERROR,
  AARCH64_OPDE_FATAL_SYNTAX_ERROR,
  AARCH64_OPDE_INVALID_VARIANT,
  AARCH64_OPDE_INVALID_VG_SIZE,
  AARCH64_OPDE_REG_LIST_LENGTH,
  AARCH64_OPDE_REG_LIST_STRIDE,
  AARCH64_OPDE_UNTIED_IMMS,
  AARCH64_OPDE_UNTIED_OPERAND,
  AARCH64_OPDE_OUT_OF_RANGE,
  AARCH64_OPDE_UNALIGNED,
  AARCH64_OPDE_OTHER_ERROR,
  AARCH64_OPDE_INVALID_REGNO
};

/* N.B. GAS assumes that this structure work well with shallow copy.  */
struct aarch64_operand_error
{
  enum aarch64_operand_error_kind kind;
  int index;
  const char *error;
  /* Some data for extra information.  */
  union {
    int i;
    const char *s;
  } data[3];
  bool non_fatal;
};

/* AArch64 sequence structure used to track instructions with F_SCAN
   dependencies for both assembler and disassembler.  */
struct aarch64_instr_sequence
{
  /* The instructions in the sequence, starting with the one that
     caused it to be opened.  */
  aarch64_inst *instr;
  /* The number of instructions already in the sequence.  */
  int num_added_insns;
  /* The number of instructions allocated to the sequence.  */
  int num_allocated_insns;
};

/* Encoding entrypoint.  */

extern bool
aarch64_opcode_encode (const aarch64_opcode *, const aarch64_inst *,
		       aarch64_insn *, aarch64_opnd_qualifier_t *,
		       aarch64_operand_error *, aarch64_instr_sequence *);

extern const aarch64_opcode *
aarch64_replace_opcode (struct aarch64_inst *,
			const aarch64_opcode *);

/* Given the opcode enumerator OP, return the pointer to the corresponding
   opcode entry.  */

extern const aarch64_opcode *
aarch64_get_opcode (enum aarch64_op);

/* An instance of this structure is passed to aarch64_print_operand, and
   the callback within this structure is used to apply styling to the
   disassembler output.  This structure encapsulates the callback and a
   state pointer.  */

struct aarch64_styler
{
  /* The callback used to apply styling.  Returns a string created from FMT
     and ARGS with STYLE applied to the string.  STYLER is a pointer back
     to this object so that the callback can access the state member.

     The string returned from this callback must remain valid until the
     call to aarch64_print_operand has completed.  */
  const char *(*apply_style) (struct aarch64_styler *styler,
			      enum disassembler_style style,
			      const char *fmt,
			      va_list args);

  /* A pointer to a state object which can be used by the apply_style
     callback function.  */
  void *state;
};

/* Generate the string representation of an operand.  */
extern void
aarch64_print_operand (char *, size_t, bfd_vma, const aarch64_opcode *,
		       const aarch64_opnd_info *, int, int *, bfd_vma *,
		       char **, char *, size_t,
		       aarch64_feature_set features,
		       struct aarch64_styler *styler);

/* Miscellaneous interface.  */

extern int
aarch64_operand_index (const enum aarch64_opnd *, enum aarch64_opnd);

extern aarch64_opnd_qualifier_t
aarch64_get_expected_qualifier (const aarch64_opnd_qualifier_seq_t *, int,
				const aarch64_opnd_qualifier_t, int);

extern bool
aarch64_is_destructive_by_operands (const aarch64_opcode *);

extern int
aarch64_num_of_operands (const aarch64_opcode *);

extern int
aarch64_stack_pointer_p (const aarch64_opnd_info *);

extern int
aarch64_zero_register_p (const aarch64_opnd_info *);

extern enum err_type
aarch64_decode_insn (aarch64_insn, aarch64_inst *, bool,
		     aarch64_operand_error *);

extern void
init_insn_sequence (const struct aarch64_inst *, aarch64_instr_sequence *);

/* Given an operand qualifier, return the expected data element size
   of a qualified operand.  */
extern unsigned char
aarch64_get_qualifier_esize (aarch64_opnd_qualifier_t);

extern enum aarch64_operand_class
aarch64_get_operand_class (enum aarch64_opnd);

extern const char *
aarch64_get_operand_name (enum aarch64_opnd);

extern const char *
aarch64_get_operand_desc (enum aarch64_opnd);

extern bool
aarch64_sve_dupm_mov_immediate_p (uint64_t, int);

extern bool
aarch64_cpu_supports_inst_p (uint64_t, aarch64_inst *);

#ifdef DEBUG_AARCH64
extern int debug_dump;

extern void
aarch64_verbose (const char *, ...) __attribute__ ((format (printf, 1, 2)));

#define DEBUG_TRACE(M, ...)					\
  {								\
    if (debug_dump)						\
      aarch64_verbose ("%s: " M ".", __func__, ##__VA_ARGS__);	\
  }

#define DEBUG_TRACE_IF(C, M, ...)				\
  {								\
    if (debug_dump && (C))					\
      aarch64_verbose ("%s: " M ".", __func__, ##__VA_ARGS__);	\
  }
#else  /* !DEBUG_AARCH64 */
#define DEBUG_TRACE(M, ...) ;
#define DEBUG_TRACE_IF(C, M, ...) ;
#endif /* DEBUG_AARCH64 */

extern const char *const aarch64_sve_pattern_array[32];
extern const char *const aarch64_sve_prfop_array[16];
extern const char *const aarch64_rprfmop_array[64];
extern const char *const aarch64_sme_vlxn_array[2];

#ifdef __cplusplus
}
#endif

#endif /* OPCODE_AARCH64_H */
