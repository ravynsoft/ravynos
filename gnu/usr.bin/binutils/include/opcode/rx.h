/* Opcode decoder for the Renesas RX
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Written by DJ Delorie <dj@redhat.com>

   This file is part of GDB, the GNU Debugger and GAS, the GNU Assembler.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* The RX decoder in libopcodes is used by the simulator, gdb's
   analyzer, and the disassembler.  Given an opcode data source,
   it decodes the next opcode into the following structures.  */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  RX_AnySize = 0,
  RX_Byte, /* undefined extension */
  RX_UByte,
  RX_SByte,
  RX_Word, /* undefined extension */
  RX_UWord,
  RX_SWord,
  RX_3Byte,
  RX_Long,
  RX_Double,
  RX_Bad_Size,
  RX_MAX_SIZE
} RX_Size;

typedef enum
{
  RX_Operand_None,
  RX_Operand_Immediate,	/* #addend */
  RX_Operand_Register,	/* Rn */
  RX_Operand_Indirect,	/* [Rn + addend] */
  RX_Operand_Zero_Indirect,/* [Rn] */
  RX_Operand_Postinc,	/* [Rn+] */
  RX_Operand_Predec,	/* [-Rn] */
  RX_Operand_Condition,	/* eq, gtu, etc */
  RX_Operand_Flag,	/* [UIOSZC] */
  RX_Operand_TwoReg,	/* [Rn + scale*R2] */
  RX_Operand_DoubleReg,	/* DRn */
  RX_Operand_DoubleRegH,/* DRHn */
  RX_Operand_DoubleRegL,/* DRLn */
  RX_Operand_DoubleCReg,/* DCRxx */
  RX_Operand_DoubleCond,/* UN/EQ/LE/LT */
} RX_Operand_Type;

typedef enum
{
  RXO_unknown,
  RXO_mov,	/* d = s (signed) */
  RXO_movbi,	/* d = [s,s2] (signed) */
  RXO_movbir,	/* [s,s2] = d (signed) */
  RXO_pushm,	/* s..s2 */
  RXO_popm,	/* s..s2 */
  RXO_xchg,	/* s <-> d */
  RXO_stcc,	/* d = s if cond(s2) */
  RXO_rtsd,	/* rtsd, 1=imm, 2-0 = reg if reg type */

  /* These are all either d OP= s or, if s2 is set, d = s OP s2.  Note
     that d may be "None".  */
  RXO_and,
  RXO_or,
  RXO_xor,
  RXO_add,
  RXO_sub,
  RXO_mul,
  RXO_div,
  RXO_divu,
  RXO_shll,
  RXO_shar,
  RXO_shlr,

  RXO_adc,	/* d = d + s + carry */
  RXO_sbb,	/* d = d - s - ~carry */
  RXO_abs,	/* d = |s| */
  RXO_max,	/* d = max(d,s) */
  RXO_min,	/* d = min(d,s) */
  RXO_emul,	/* d:64 = d:32 * s */
  RXO_emulu,	/* d:64 = d:32 * s (unsigned) */

  RXO_rolc,	/* d <<= 1 through carry */
  RXO_rorc,	/* d >>= 1 through carry*/
  RXO_rotl,	/* d <<= #s without carry */
  RXO_rotr,	/* d >>= #s without carry*/
  RXO_revw,	/* d = revw(s) */
  RXO_revl,	/* d = revl(s) */
  RXO_branch,	/* pc = d if cond(s) */
  RXO_branchrel,/* pc += d if cond(s) */
  RXO_jsr,	/* pc = d */
  RXO_jsrrel,	/* pc += d */
  RXO_rts,
  RXO_nop,
  RXO_nop2,
  RXO_nop3,
  RXO_nop4,
  RXO_nop5,
  RXO_nop6,
  RXO_nop7,

  RXO_scmpu,
  RXO_smovu,
  RXO_smovb,
  RXO_suntil,
  RXO_swhile,
  RXO_smovf,
  RXO_sstr,

  RXO_rmpa,
  RXO_mulhi,
  RXO_mullo,
  RXO_machi,
  RXO_maclo,
  RXO_mvtachi,
  RXO_mvtaclo,
  RXO_mvfachi,
  RXO_mvfacmi,
  RXO_mvfaclo,
  RXO_racw,

  RXO_sat,	/* sat(d) */
  RXO_satr,

  RXO_fadd,	/* d op= s */
  RXO_fcmp,
  RXO_fsub,
  RXO_ftoi,
  RXO_fmul,
  RXO_fdiv,
  RXO_round,
  RXO_itof,

  RXO_bset,	/* d |= (1<<s) */
  RXO_bclr,	/* d &= ~(1<<s) */
  RXO_btst,	/* s & (1<<s2) */
  RXO_bnot,	/* d ^= (1<<s) */
  RXO_bmcc,	/* d<s> = cond(s2) */

  RXO_clrpsw,	/* flag index in d */
  RXO_setpsw,	/* flag index in d */
  RXO_mvtipl,	/* new IPL in s */

  RXO_rtfi,
  RXO_rte,
  RXO_rtd,	/* undocumented */
  RXO_brk,
  RXO_dbt,	/* undocumented */
  RXO_int,	/* vector id in s */
  RXO_stop,
  RXO_wait,

  RXO_sccnd,	/* d = cond(s) ? 1 : 0 */

  RXO_fsqrt,
  RXO_ftou,
  RXO_utof,
  RXO_movco,
  RXO_movli,
  RXO_emaca,
  RXO_emsba,
  RXO_emula,
  RXO_maclh,
  RXO_msbhi,
  RXO_msblh,
  RXO_msblo,
  RXO_mullh,
  RXO_mvfacgu,
  RXO_mvtacgu,
  RXO_racl,
  RXO_rdacl,
  RXO_rdacw,

  RXO_bfmov,
  RXO_bfmovz,
  RXO_rstr,
  RXO_save,
  RXO_dmov,
  RXO_dpopm,
  RXO_dpushm,
  RXO_mvfdc,
  RXO_mvfdr,
  RXO_mvtdc,
  RXO_dabs,
  RXO_dadd,
  RXO_dcmp,
  RXO_ddiv,
  RXO_dmul,
  RXO_dneg,
  RXO_dround,
  RXO_dsqrt,
  RXO_dsub,
  RXO_dtoi,
  RXO_dtof,
  RXO_dtou,
  RXO_ftod,
  RXO_itod,
  RXO_utod
} RX_Opcode_ID;

/* Condition bitpatterns, as registers.  */
#define RXC_eq		0
#define RXC_z		0
#define RXC_ne		1
#define RXC_nz		1
#define RXC_c		2
#define RXC_nc		3
#define RXC_gtu		4
#define RXC_leu		5
#define RXC_pz		6
#define RXC_n		7
#define RXC_ge		8
#define RXC_lt		9
#define RXC_gt		10
#define RXC_le		11
#define RXC_o		12
#define RXC_no		13
#define RXC_always	14
#define RXC_never	15

typedef struct
{
  RX_Operand_Type  type;
  int              reg;
  int              addend;
  RX_Size          size;
} RX_Opcode_Operand;

typedef struct
{
  RX_Opcode_ID      id;
  int               n_bytes;
  int               prefix;
  char *            syntax;
  RX_Size           size;
  /* By convention, these are destination, source1, source2.  */
  RX_Opcode_Operand op[3];

  /* The logic here is:
     newflags = (oldflags & ~(int)flags_0) | flags_1 | (op_flags & flags_s)
     Only the O, S, Z, and C flags are affected.  */
  char flags_0; /* This also clears out flags-to-be-set.  */
  char flags_1;
  char flags_s;
} RX_Opcode_Decoded;

/* Within the syntax, %c-style format specifiers are as follows:

   %% = '%' character
   %0 = operand[0] (destination)
   %1 = operand[1] (source)
   %2 = operand[2] (2nd source)
   %s = operation size (b/w/l)
   %SN = operand size [N] (N=0,1,2)
   %aN = op[N] as an address (N=0,1,2)

   Register numbers 0..15 are general registers.  16..31 are control
   registers.  32..47 are condition codes.  */

int rx_decode_opcode (unsigned long, RX_Opcode_Decoded *, int (*)(void *), void *);

#ifdef __cplusplus
}
#endif
