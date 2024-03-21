/* Instruction printing code for the ARM
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Contributed by Richard Earnshaw (rwe@pegasus.esprit.ec.org)
   Modification by James G. Smith (jsmith@cygnus.co.uk)

   This file is part of libopcodes.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <assert.h>

#include "disassemble.h"
#include "opcode/arm.h"
#include "opintl.h"
#include "safe-ctype.h"
#include "libiberty.h"
#include "floatformat.h"

/* FIXME: This shouldn't be done here.  */
#include "coff/internal.h"
#include "libcoff.h"
#include "bfd.h"
#include "elf-bfd.h"
#include "elf/internal.h"
#include "elf/arm.h"
#include "mach-o.h"

/* Cached mapping symbol state.  */
enum map_type
{
  MAP_ARM,
  MAP_THUMB,
  MAP_DATA
};

struct arm_private_data
{
  /* The features to use when disassembling optional instructions.  */
  arm_feature_set features;

  /* Track the last type (although this doesn't seem to be useful) */
  enum map_type last_type;

  /* Tracking symbol table information */
  int last_mapping_sym;

  /* The end range of the current range being disassembled.  */
  bfd_vma last_stop_offset;
  bfd_vma last_mapping_addr;
};

enum mve_instructions
{
  MVE_VPST,
  MVE_VPT_FP_T1,
  MVE_VPT_FP_T2,
  MVE_VPT_VEC_T1,
  MVE_VPT_VEC_T2,
  MVE_VPT_VEC_T3,
  MVE_VPT_VEC_T4,
  MVE_VPT_VEC_T5,
  MVE_VPT_VEC_T6,
  MVE_VCMP_FP_T1,
  MVE_VCMP_FP_T2,
  MVE_VCMP_VEC_T1,
  MVE_VCMP_VEC_T2,
  MVE_VCMP_VEC_T3,
  MVE_VCMP_VEC_T4,
  MVE_VCMP_VEC_T5,
  MVE_VCMP_VEC_T6,
  MVE_VDUP,
  MVE_VEOR,
  MVE_VFMAS_FP_SCALAR,
  MVE_VFMA_FP_SCALAR,
  MVE_VFMA_FP,
  MVE_VFMS_FP,
  MVE_VHADD_T1,
  MVE_VHADD_T2,
  MVE_VHSUB_T1,
  MVE_VHSUB_T2,
  MVE_VRHADD,
  MVE_VLD2,
  MVE_VLD4,
  MVE_VST2,
  MVE_VST4,
  MVE_VLDRB_T1,
  MVE_VLDRH_T2,
  MVE_VLDRB_T5,
  MVE_VLDRH_T6,
  MVE_VLDRW_T7,
  MVE_VSTRB_T1,
  MVE_VSTRH_T2,
  MVE_VSTRB_T5,
  MVE_VSTRH_T6,
  MVE_VSTRW_T7,
  MVE_VLDRB_GATHER_T1,
  MVE_VLDRH_GATHER_T2,
  MVE_VLDRW_GATHER_T3,
  MVE_VLDRD_GATHER_T4,
  MVE_VLDRW_GATHER_T5,
  MVE_VLDRD_GATHER_T6,
  MVE_VSTRB_SCATTER_T1,
  MVE_VSTRH_SCATTER_T2,
  MVE_VSTRW_SCATTER_T3,
  MVE_VSTRD_SCATTER_T4,
  MVE_VSTRW_SCATTER_T5,
  MVE_VSTRD_SCATTER_T6,
  MVE_VCVT_FP_FIX_VEC,
  MVE_VCVT_BETWEEN_FP_INT,
  MVE_VCVT_FP_HALF_FP,
  MVE_VCVT_FROM_FP_TO_INT,
  MVE_VRINT_FP,
  MVE_VMOV_HFP_TO_GP,
  MVE_VMOV_GP_TO_VEC_LANE,
  MVE_VMOV_IMM_TO_VEC,
  MVE_VMOV_VEC_TO_VEC,
  MVE_VMOV2_VEC_LANE_TO_GP,
  MVE_VMOV2_GP_TO_VEC_LANE,
  MVE_VMOV_VEC_LANE_TO_GP,
  MVE_VMVN_IMM,
  MVE_VMVN_REG,
  MVE_VORR_IMM,
  MVE_VORR_REG,
  MVE_VORN,
  MVE_VBIC_IMM,
  MVE_VBIC_REG,
  MVE_VMOVX,
  MVE_VMOVL,
  MVE_VMOVN,
  MVE_VMULL_INT,
  MVE_VMULL_POLY,
  MVE_VQDMULL_T1,
  MVE_VQDMULL_T2,
  MVE_VQMOVN,
  MVE_VQMOVUN,
  MVE_VADDV,
  MVE_VMLADAV_T1,
  MVE_VMLADAV_T2,
  MVE_VMLALDAV,
  MVE_VMLAS,
  MVE_VADDLV,
  MVE_VMLSDAV_T1,
  MVE_VMLSDAV_T2,
  MVE_VMLSLDAV,
  MVE_VRMLALDAVH,
  MVE_VRMLSLDAVH,
  MVE_VQDMLADH,
  MVE_VQRDMLADH,
  MVE_VQDMLAH,
  MVE_VQRDMLAH,
  MVE_VQDMLASH,
  MVE_VQRDMLASH,
  MVE_VQDMLSDH,
  MVE_VQRDMLSDH,
  MVE_VQDMULH_T1,
  MVE_VQRDMULH_T2,
  MVE_VQDMULH_T3,
  MVE_VQRDMULH_T4,
  MVE_VDDUP,
  MVE_VDWDUP,
  MVE_VIWDUP,
  MVE_VIDUP,
  MVE_VCADD_FP,
  MVE_VCADD_VEC,
  MVE_VHCADD,
  MVE_VCMLA_FP,
  MVE_VCMUL_FP,
  MVE_VQRSHL_T1,
  MVE_VQRSHL_T2,
  MVE_VQRSHRN,
  MVE_VQRSHRUN,
  MVE_VQSHL_T1,
  MVE_VQSHL_T2,
  MVE_VQSHLU_T3,
  MVE_VQSHL_T4,
  MVE_VQSHRN,
  MVE_VQSHRUN,
  MVE_VRSHL_T1,
  MVE_VRSHL_T2,
  MVE_VRSHR,
  MVE_VRSHRN,
  MVE_VSHL_T1,
  MVE_VSHL_T2,
  MVE_VSHL_T3,
  MVE_VSHLC,
  MVE_VSHLL_T1,
  MVE_VSHLL_T2,
  MVE_VSHR,
  MVE_VSHRN,
  MVE_VSLI,
  MVE_VSRI,
  MVE_VADC,
  MVE_VABAV,
  MVE_VABD_FP,
  MVE_VABD_VEC,
  MVE_VABS_FP,
  MVE_VABS_VEC,
  MVE_VADD_FP_T1,
  MVE_VADD_FP_T2,
  MVE_VADD_VEC_T1,
  MVE_VADD_VEC_T2,
  MVE_VSBC,
  MVE_VSUB_FP_T1,
  MVE_VSUB_FP_T2,
  MVE_VSUB_VEC_T1,
  MVE_VSUB_VEC_T2,
  MVE_VAND,
  MVE_VBRSR,
  MVE_VCLS,
  MVE_VCLZ,
  MVE_VCTP,
  MVE_VMAX,
  MVE_VMAXA,
  MVE_VMAXNM_FP,
  MVE_VMAXNMA_FP,
  MVE_VMAXNMV_FP,
  MVE_VMAXNMAV_FP,
  MVE_VMAXV,
  MVE_VMAXAV,
  MVE_VMIN,
  MVE_VMINA,
  MVE_VMINNM_FP,
  MVE_VMINNMA_FP,
  MVE_VMINNMV_FP,
  MVE_VMINNMAV_FP,
  MVE_VMINV,
  MVE_VMINAV,
  MVE_VMLA,
  MVE_VMUL_FP_T1,
  MVE_VMUL_FP_T2,
  MVE_VMUL_VEC_T1,
  MVE_VMUL_VEC_T2,
  MVE_VMULH,
  MVE_VRMULH,
  MVE_VNEG_FP,
  MVE_VNEG_VEC,
  MVE_VPNOT,
  MVE_VPSEL,
  MVE_VQABS,
  MVE_VQADD_T1,
  MVE_VQADD_T2,
  MVE_VQSUB_T1,
  MVE_VQSUB_T2,
  MVE_VQNEG,
  MVE_VREV16,
  MVE_VREV32,
  MVE_VREV64,
  MVE_LSLL,
  MVE_LSLLI,
  MVE_LSRL,
  MVE_ASRL,
  MVE_ASRLI,
  MVE_SQRSHRL,
  MVE_SQRSHR,
  MVE_UQRSHL,
  MVE_UQRSHLL,
  MVE_UQSHL,
  MVE_UQSHLL,
  MVE_URSHRL,
  MVE_URSHR,
  MVE_SRSHRL,
  MVE_SRSHR,
  MVE_SQSHLL,
  MVE_SQSHL,
  MVE_CINC,
  MVE_CINV,
  MVE_CNEG,
  MVE_CSINC,
  MVE_CSINV,
  MVE_CSET,
  MVE_CSETM,
  MVE_CSNEG,
  MVE_CSEL,
  MVE_NONE
};

enum mve_unpredictable
{
  UNPRED_IT_BLOCK,		/* Unpredictable because mve insn in it block.
				 */
  UNPRED_FCA_0_FCB_1,		/* Unpredictable because fcA = 0 and
				   fcB = 1 (vpt).  */
  UNPRED_R13,			/* Unpredictable because r13 (sp) or
				   r15 (sp) used.  */
  UNPRED_R15,			/* Unpredictable because r15 (pc) is used.  */
  UNPRED_Q_GT_4,		/* Unpredictable because
				   vec reg start > 4 (vld4/st4).  */
  UNPRED_Q_GT_6,		/* Unpredictable because
				   vec reg start > 6 (vld2/st2).  */
  UNPRED_R13_AND_WB,		/* Unpredictable becase gp reg = r13
				   and WB bit = 1.  */
  UNPRED_Q_REGS_EQUAL,		/* Unpredictable because vector registers are
				   equal.  */
  UNPRED_OS,			/* Unpredictable because offset scaled == 1.  */
  UNPRED_GP_REGS_EQUAL,		/* Unpredictable because gp registers are the
				   same.  */
  UNPRED_Q_REGS_EQ_AND_SIZE_1,	/* Unpredictable because q regs equal and
				   size = 1.  */
  UNPRED_Q_REGS_EQ_AND_SIZE_2,	/* Unpredictable because q regs equal and
				   size = 2.  */
  UNPRED_NONE			/* No unpredictable behavior.  */
};

enum mve_undefined
{
  UNDEF_SIZE,			/* undefined size.  */
  UNDEF_SIZE_0,			/* undefined because size == 0.  */
  UNDEF_SIZE_2,			/* undefined because size == 2.  */
  UNDEF_SIZE_3,			/* undefined because size == 3.  */
  UNDEF_SIZE_LE_1,		/* undefined because size <= 1.  */
  UNDEF_SIZE_NOT_0,		/* undefined because size != 0.  */
  UNDEF_SIZE_NOT_2,		/* undefined because size != 2.  */
  UNDEF_SIZE_NOT_3,		/* undefined because size != 3.  */
  UNDEF_NOT_UNS_SIZE_0,		/* undefined because U == 0 and
				   size == 0.  */
  UNDEF_NOT_UNS_SIZE_1,		/* undefined because U == 0 and
				   size == 1.  */
  UNDEF_NOT_UNSIGNED,		/* undefined because U == 0.  */
  UNDEF_VCVT_IMM6,		/* imm6 < 32.  */
  UNDEF_VCVT_FSI_IMM6,		/* fsi = 0 and 32 >= imm6 <= 47.  */
  UNDEF_BAD_OP1_OP2,		/* undefined with op2 = 2 and
				   op1 == (0 or 1).  */
  UNDEF_BAD_U_OP1_OP2,		/* undefined with U = 1 and
				   op2 == 0 and op1 == (0 or 1).  */
  UNDEF_OP_0_BAD_CMODE,		/* undefined because op == 0 and cmode
				   in {0xx1, x0x1}.  */
  UNDEF_XCHG_UNS,		/* undefined because X == 1 and U == 1.  */
  UNDEF_NONE			/* no undefined behavior.  */
};

struct opcode32
{
  arm_feature_set arch;		/* Architecture defining this insn.  */
  unsigned long value;		/* If arch is 0 then value is a sentinel.  */
  unsigned long mask;		/* Recognise insn if (op & mask) == value.  */
  const char *  assembler;	/* How to disassemble this insn.  */
};

struct cdeopcode32
{
  arm_feature_set arch;		/* Architecture defining this insn.  */
  uint8_t coproc_shift;		/* coproc is this far into op.  */
  uint16_t coproc_mask;		/* Length of coproc field in op.  */
  unsigned long value;		/* If arch is 0 then value is a sentinel.  */
  unsigned long mask;		/* Recognise insn if (op & mask) == value.  */
  const char *  assembler;	/* How to disassemble this insn.  */
};

/* MVE opcodes.  */

struct mopcode32
{
  arm_feature_set arch;		/* Architecture defining this insn.  */
  enum mve_instructions mve_op;  /* Specific mve instruction for faster
				    decoding.  */
  unsigned long value;		/* If arch is 0 then value is a sentinel.  */
  unsigned long mask;		/* Recognise insn if (op & mask) == value.  */
  const char *  assembler;	/* How to disassemble this insn.  */
};

enum isa {
  ANY,
  T32,
  ARM
};


/* Shared (between Arm and Thumb mode) opcode.  */
struct sopcode32
{
  enum isa isa;			/* Execution mode instruction availability.  */
  arm_feature_set arch;		/* Architecture defining this insn.  */
  unsigned long value;		/* If arch is 0 then value is a sentinel.  */
  unsigned long mask;		/* Recognise insn if (op & mask) == value.  */
  const char *  assembler;	/* How to disassemble this insn.  */
};

struct opcode16
{
  arm_feature_set arch;		/* Architecture defining this insn.  */
  unsigned short value, mask;	/* Recognise insn if (op & mask) == value.  */
  const char *assembler;	/* How to disassemble this insn.  */
};

/* print_insn_coprocessor recognizes the following format control codes:

   %%			%

   %c			print condition code (always bits 28-31 in ARM mode)
   %b			print condition code allowing cp_num == 9
   %q			print shifter argument
   %u			print condition code (unconditional in ARM mode,
                          UNPREDICTABLE if not AL in Thumb)
   %A			print address for ldc/stc/ldf/stf instruction
   %B			print vstm/vldm register list
   %C			print vscclrm register list
   %I                   print cirrus signed shift immediate: bits 0..3|4..6
   %J			print register for VLDR instruction
   %K			print address for VLDR instruction
   %F			print the COUNT field of a LFM/SFM instruction.
   %P			print floating point precision in arithmetic insn
   %Q			print floating point precision in ldf/stf insn
   %R			print floating point rounding mode

   %<bitfield>c		print as a condition code (for vsel)
   %<bitfield>r		print as an ARM register
   %<bitfield>R		as %<>r but r15 is UNPREDICTABLE
   %<bitfield>ru        as %<>r but each u register must be unique.
   %<bitfield>d		print the bitfield in decimal
   %<bitfield>k		print immediate for VFPv3 conversion instruction
   %<bitfield>x		print the bitfield in hex
   %<bitfield>X		print the bitfield as 1 hex digit without leading "0x"
   %<bitfield>f		print a floating point constant if >7 else a
			floating point register
   %<bitfield>w         print as an iWMMXt width field - [bhwd]ss/us
   %<bitfield>g         print as an iWMMXt 64-bit register
   %<bitfield>G         print as an iWMMXt general purpose or control register
   %<bitfield>D		print as a NEON D register
   %<bitfield>Q		print as a NEON Q register
   %<bitfield>V		print as a NEON D or Q register
   %<bitfield>E		print a quarter-float immediate value

   %y<code>		print a single precision VFP reg.
			  Codes: 0=>Sm, 1=>Sd, 2=>Sn, 3=>multi-list, 4=>Sm pair
   %z<code>		print a double precision VFP reg
			  Codes: 0=>Dm, 1=>Dd, 2=>Dn, 3=>multi-list

   %<bitfield>'c	print specified char iff bitfield is all ones
   %<bitfield>`c	print specified char iff bitfield is all zeroes
   %<bitfield>?ab...    select from array of values in big endian order

   %L			print as an iWMMXt N/M width field.
   %Z			print the Immediate of a WSHUFH instruction.
   %l			like 'A' except use byte offsets for 'B' & 'H'
			versions.
   %i			print 5-bit immediate in bits 8,3..0
			(print "32" when 0)
   %r			print register offset address for wldt/wstr instruction.  */

enum opcode_sentinel_enum
{
  SENTINEL_IWMMXT_START = 1,
  SENTINEL_IWMMXT_END,
  SENTINEL_GENERIC_START
} opcode_sentinels;

#define UNDEFINED_INSTRUCTION      "\t\t@ <UNDEFINED> instruction: %0-31x"
#define UNKNOWN_INSTRUCTION_32BIT  "\t\t@ <UNDEFINED> instruction: %08x"
#define UNKNOWN_INSTRUCTION_16BIT  "\t\t@ <UNDEFINED> instruction: %04x"
#define UNPREDICTABLE_INSTRUCTION  "\t@ <UNPREDICTABLE>"

/* Common coprocessor opcodes shared between Arm and Thumb-2.  */

/* print_insn_cde recognizes the following format control codes:

   %%			%

   %a			print 'a' iff bit 28 is 1
   %p			print bits 8-10 as coprocessor
   %<bitfield>d		print as decimal
   %<bitfield>r		print as an ARM register
   %<bitfield>n		print as an ARM register but r15 is APSR_nzcv
   %<bitfield>T		print as an ARM register + 1
   %<bitfield>R		as %r but r13 is UNPREDICTABLE
   %<bitfield>S		as %r but rX where X > 10 is UNPREDICTABLE
   %j			print immediate taken from bits (16..21,7,0..5)
   %k			print immediate taken from bits (20..21,7,0..5).
   %l			print immediate taken from bits (20..22,7,4..5).  */

/* At the moment there is only one valid position for the coprocessor number,
   and hence that's encoded in the macro below.  */
#define CDE_OPCODE(ARCH, VALUE, MASK, ASM) \
  { ARCH, 8, 7, VALUE, MASK, ASM }
static const struct cdeopcode32 cde_opcodes[] =
{
  /* Custom Datapath Extension instructions.  */
  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xee000000, 0xefc00840,
	      "cx1%a\t%p, %12-15n, %{I:#%0-5,7,16-21d%}"),
  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xee000040, 0xefc00840,
	      "cx1d%a\t%p, %12-15S, %12-15T, %{I:#%0-5,7,16-21d%}"),

  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xee400000, 0xefc00840,
	      "cx2%a\t%p, %12-15n, %16-19n, %{I:#%0-5,7,20-21d%}"),
  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xee400040, 0xefc00840,
	      "cx2d%a\t%p, %12-15S, %12-15T, %16-19n, %{I:#%0-5,7,20-21d%}"),

  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xee800000, 0xef800840,
	      "cx3%a\t%p, %0-3n, %16-19n, %12-15n, %{I:#%4-5,7,20-22d%}"),
  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xee800040, 0xef800840,
	     "cx3d%a\t%p, %0-3S, %0-3T, %16-19n, %12-15n, %{I:#%4-5,7,20-22d%}"),

  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xec200000, 0xeeb00840,
	      "vcx1%a\t%p, %12-15,22V, %{I:#%0-5,7,16-19d%}"),
  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xec200040, 0xeeb00840,
	      "vcx1%a\t%p, %12-15,22V, %{I:#%0-5,7,16-19,24d%}"),

  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xec300000, 0xeeb00840,
	      "vcx2%a\t%p, %12-15,22V, %0-3,5V, %{I:#%4,7,16-19d%}"),
  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xec300040, 0xeeb00840,
	      "vcx2%a\t%p, %12-15,22V, %0-3,5V, %{I:#%4,7,16-19,24d%}"),

  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xec800000, 0xee800840,
	      "vcx3%a\t%p, %12-15,22V, %16-19,7V, %0-3,5V, %{I:#%4,20-21d%}"),
  CDE_OPCODE (ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE),
	      0xec800040, 0xee800840,
	      "vcx3%a\t%p, %12-15,22V, %16-19,7V, %0-3,5V, %{I:#%4,20-21,24d%}"),

  CDE_OPCODE (ARM_FEATURE_CORE_LOW (0), 0, 0, 0)

};

static const struct sopcode32 coprocessor_opcodes[] =
{
  /* XScale instructions.  */
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e200010, 0x0fff0ff0,
    "mia%c\t%{R:acc0%}, %0-3r, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e280010, 0x0fff0ff0,
    "miaph%c\t%{R:acc0%}, %0-3r, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e2c0010, 0x0ffc0ff0, "mia%17'T%17`B%16'T%16`B%c\t%{R:acc0%}, %0-3r, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0c400000, 0x0ff00fff, "mar%c\t%{R:acc0%}, %12-15r, %16-19r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0c500000, 0x0ff00fff, "mra%c\t%12-15r, %16-19r, %{R:acc0%}"},

  /* Intel Wireless MMX technology instructions.  */
  {ANY, ARM_FEATURE_CORE_LOW (0), SENTINEL_IWMMXT_START, 0, "" },
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT),
    0x0e130130, 0x0f3f0fff, "tandc%22-23w%c\t%12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e400010, 0x0ff00f3f, "tbcst%6-7w%c\t%16-19g, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e130170, 0x0f3f0ff8, "textrc%22-23w%c\t%12-15r, %{I:#%0-2d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100070, 0x0f300ff0, "textrm%3?su%22-23w%c\t%12-15r, %16-19g, %{I:#%0-2d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e600010, 0x0ff00f38, "tinsr%6-7w%c\t%16-19g, %12-15r, %{I:#%0-2d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000110, 0x0ff00fff, "tmcr%c\t%16-19G, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0c400000, 0x0ff00ff0, "tmcrr%c\t%0-3g, %12-15r, %16-19r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e2c0010, 0x0ffc0e10, "tmia%17?tb%16?tb%c\t%5-8g, %0-3r, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e200010, 0x0fff0e10, "tmia%c\t%5-8g, %0-3r, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e280010, 0x0fff0e10, "tmiaph%c\t%5-8g, %0-3r, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100030, 0x0f300fff, "tmovmsk%22-23w%c\t%12-15r, %16-19g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100110, 0x0ff00ff0, "tmrc%c\t%12-15r, %16-19G"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0c500000, 0x0ff00ff0, "tmrrc%c\t%12-15r, %16-19r, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e130150, 0x0f3f0fff, "torc%22-23w%c\t%12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e120190, 0x0f3f0fff, "torvsc%22-23w%c\t%12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e2001c0, 0x0f300fff, "wabs%22-23w%c\t%12-15g, %16-19g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e0001c0, 0x0f300fff, "wacc%22-23w%c\t%12-15g, %16-19g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000180, 0x0f000ff0, "wadd%20-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e2001a0, 0x0fb00ff0, "waddbhus%22?ml%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0ea001a0, 0x0ff00ff0, "waddsubhx%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000020, 0x0f800ff0, "waligni%c\t%12-15g, %16-19g, %0-3g, %{I:#%20-22d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e800020, 0x0fc00ff0, "walignr%20-21d%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e200000, 0x0fe00ff0, "wand%20'n%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e800000, 0x0fa00ff0, "wavg2%22?hb%20'r%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e400000, 0x0fe00ff0, "wavg4%20'r%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000060, 0x0f300ff0, "wcmpeq%22-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100060, 0x0f100ff0, "wcmpgt%21?su%22-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0xfc500100, 0xfe500f00, "wldrd\t%12-15g, %r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0xfc100100, 0xfe500f00, "wldrw\t%12-15G, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0c100000, 0x0e100e00, "wldr%L%c\t%12-15g, %l"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e400100, 0x0fc00ff0, "wmac%21?su%20'z%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e800100, 0x0fc00ff0, "wmadd%21?su%20'x%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0ec00100, 0x0fd00ff0, "wmadd%21?sun%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000160, 0x0f100ff0, "wmax%21?su%22-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000080, 0x0f100fe0, "wmerge%c\t%12-15g, %16-19g, %0-3g, %{I:#%21-23d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e0000a0, 0x0f800ff0, "wmia%21?tb%20?tb%22'n%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e800120, 0x0f800ff0,
    "wmiaw%21?tb%20?tb%22'n%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100160, 0x0f100ff0, "wmin%21?su%22-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000100, 0x0fc00ff0, "wmul%21?su%20?ml%23'r%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0ed00100, 0x0fd00ff0, "wmul%21?sumr%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0ee000c0, 0x0fe00ff0, "wmulwsm%20`r%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0ec000c0, 0x0fe00ff0, "wmulwum%20`r%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0eb000c0, 0x0ff00ff0, "wmulwl%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e8000a0, 0x0f800ff0,
    "wqmia%21?tb%20?tb%22'n%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100080, 0x0fd00ff0, "wqmulm%21'r%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0ec000e0, 0x0fd00ff0, "wqmulwm%21'r%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000000, 0x0ff00ff0, "wor%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000080, 0x0f000ff0, "wpack%20-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0xfe300040, 0xff300ef0, "wror%22-23w\t%12-15g, %16-19g, %{I:#%i%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e300040, 0x0f300ff0, "wror%22-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e300140, 0x0f300ff0, "wror%22-23wg%c\t%12-15g, %16-19g, %0-3G"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000120, 0x0fa00ff0, "wsad%22?hb%20'z%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e0001e0, 0x0f000ff0, "wshufh%c\t%12-15g, %16-19g, %{I:#%Z%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0xfe100040, 0xff300ef0, "wsll%22-23w\t%12-15g, %16-19g, %{I:#%i%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100040, 0x0f300ff0, "wsll%22-23w%8'g%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100148, 0x0f300ffc, "wsll%22-23w%8'g%c\t%12-15g, %16-19g, %0-3G"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0xfe000040, 0xff300ef0, "wsra%22-23w\t%12-15g, %16-19g, %{I:#%i%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000040, 0x0f300ff0, "wsra%22-23w%8'g%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e000148, 0x0f300ffc, "wsra%22-23w%8'g%c\t%12-15g, %16-19g, %0-3G"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0xfe200040, 0xff300ef0, "wsrl%22-23w\t%12-15g, %16-19g, %{I:#%i%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e200040, 0x0f300ff0, "wsrl%22-23w%8'g%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e200148, 0x0f300ffc, "wsrl%22-23w%8'g%c\t%12-15g, %16-19g, %0-3G"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0xfc400100, 0xfe500f00, "wstrd\t%12-15g, %r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0xfc000100, 0xfe500f00, "wstrw\t%12-15G, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0c000000, 0x0e100e00, "wstr%L%c\t%12-15g, %l"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e0001a0, 0x0f000ff0, "wsub%20-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0ed001c0, 0x0ff00ff0, "wsubaddhx%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e1001c0, 0x0f300ff0, "wabsdiff%22-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e0000c0, 0x0fd00fff, "wunpckeh%21?sub%c\t%12-15g, %16-19g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e4000c0, 0x0fd00fff, "wunpckeh%21?suh%c\t%12-15g, %16-19g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e8000c0, 0x0fd00fff, "wunpckeh%21?suw%c\t%12-15g, %16-19g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e0000e0, 0x0f100fff, "wunpckel%21?su%22-23w%c\t%12-15g, %16-19g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e1000c0, 0x0f300ff0, "wunpckih%22-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e1000e0, 0x0f300ff0, "wunpckil%22-23w%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
    0x0e100000, 0x0ff00ff0, "wxor%c\t%12-15g, %16-19g, %0-3g"},
  {ANY, ARM_FEATURE_CORE_LOW (0),
    SENTINEL_IWMMXT_END, 0, "" },

  /* Floating point coprocessor (FPA) instructions.  */
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e000100, 0x0ff08f10, "adf%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e100100, 0x0ff08f10, "muf%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e200100, 0x0ff08f10, "suf%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e300100, 0x0ff08f10, "rsf%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e400100, 0x0ff08f10, "dvf%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e500100, 0x0ff08f10, "rdf%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e600100, 0x0ff08f10, "pow%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e700100, 0x0ff08f10, "rpw%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e800100, 0x0ff08f10, "rmf%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e900100, 0x0ff08f10, "fml%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ea00100, 0x0ff08f10, "fdv%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0eb00100, 0x0ff08f10, "frd%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ec00100, 0x0ff08f10, "pol%c%P%R\t%12-14f, %16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e008100, 0x0ff08f10, "mvf%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e108100, 0x0ff08f10, "mnf%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e208100, 0x0ff08f10, "abs%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e308100, 0x0ff08f10, "rnd%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e408100, 0x0ff08f10, "sqt%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e508100, 0x0ff08f10, "log%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e608100, 0x0ff08f10, "lgn%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e708100, 0x0ff08f10, "exp%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e808100, 0x0ff08f10, "sin%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e908100, 0x0ff08f10, "cos%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ea08100, 0x0ff08f10, "tan%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0eb08100, 0x0ff08f10, "asn%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ec08100, 0x0ff08f10, "acs%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ed08100, 0x0ff08f10, "atn%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ee08100, 0x0ff08f10, "urd%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ef08100, 0x0ff08f10, "nrm%c%P%R\t%12-14f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e000110, 0x0ff00f1f, "flt%c%P%R\t%16-18f, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e100110, 0x0fff0f98, "fix%c%R\t%12-15r, %0-2f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e200110, 0x0fff0fff, "wfs%c\t%12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e300110, 0x0fff0fff, "rfs%c\t%12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e400110, 0x0fff0fff, "wfc%c\t%12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e500110, 0x0fff0fff, "rfc%c\t%12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0e90f110, 0x0ff8fff0, "cmf%c\t%16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0eb0f110, 0x0ff8fff0, "cnf%c\t%16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ed0f110, 0x0ff8fff0, "cmfe%c\t%16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0ef0f110, 0x0ff8fff0, "cnfe%c\t%16-18f, %0-3f"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0c000100, 0x0e100f00, "stf%c%Q\t%12-14f, %A"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V1),
    0x0c100100, 0x0e100f00, "ldf%c%Q\t%12-14f, %A"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V2),
    0x0c000200, 0x0e100f00, "sfm%c\t%12-14f, %F, %A"},
  {ANY, ARM_FEATURE_COPROC (FPU_FPA_EXT_V2),
    0x0c100200, 0x0e100f00, "lfm%c\t%12-14f, %F, %A"},

  /* Armv8.1-M Mainline instructions.  */
  {T32, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xec9f0b00, 0xffbf0f01, "vscclrm%c\t%C"},
  {T32, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xec9f0a00, 0xffbf0f00, "vscclrm%c\t%C"},

  /* ARMv8-M Mainline Security Extensions instructions.  */
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M_MAIN),
    0xec300a00, 0xfff0ffff, "vlldm\t%16-19r"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M_MAIN),
    0xec200a00, 0xfff0ffff, "vlstm\t%16-19r"},

  /* Register load/store.  */
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD | FPU_NEON_EXT_V1),
    0x0d2d0b00, 0x0fbf0f01, "vpush%c\t%B"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD | FPU_NEON_EXT_V1),
    0x0d200b00, 0x0fb00f01, "vstmdb%c\t%16-19r!, %B"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD | FPU_NEON_EXT_V1),
    0x0d300b00, 0x0fb00f01, "vldmdb%c\t%16-19r!, %B"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD | FPU_NEON_EXT_V1),
    0x0c800b00, 0x0f900f01, "vstmia%c\t%16-19r%21'!, %B"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD | FPU_NEON_EXT_V1),
    0x0cbd0b00, 0x0fbf0f01, "vpop%c\t%B"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD | FPU_NEON_EXT_V1),
    0x0c900b00, 0x0f900f01, "vldmia%c\t%16-19r%21'!, %B"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD | FPU_NEON_EXT_V1),
    0x0d000b00, 0x0f300f00, "vstr%c\t%12-15,22D, %A"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD | FPU_NEON_EXT_V1),
    0x0d100b00, 0x0f300f00, "vldr%c\t%12-15,22D, %A"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0d2d0a00, 0x0fbf0f00, "vpush%c\t%y3"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0d200a00, 0x0fb00f00, "vstmdb%c\t%16-19r!, %y3"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0d300a00, 0x0fb00f00, "vldmdb%c\t%16-19r!, %y3"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0c800a00, 0x0f900f00, "vstmia%c\t%16-19r%21'!, %y3"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0cbd0a00, 0x0fbf0f00, "vpop%c\t%y3"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0c900a00, 0x0f900f00, "vldmia%c\t%16-19r%21'!, %y3"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0d000a00, 0x0f300f00, "vstr%c\t%y1, %A"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0d100a00, 0x0f300f00, "vldr%c\t%y1, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_EXT2_V8_1M_MAIN),
    0xec100f80, 0xfe101f80, "vldr%c\t%J, %K"},
  {ANY, ARM_FEATURE_COPROC (ARM_EXT2_V8_1M_MAIN),
    0xec000f80, 0xfe101f80, "vstr%c\t%J, %K"},

  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0d200b01, 0x0fb00f01, "fstmdbx%c\t%16-19r!, %z3\t@ Deprecated"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0d300b01, 0x0fb00f01, "fldmdbx%c\t%16-19r!, %z3\t@ Deprecated"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0c800b01, 0x0f900f01, "fstmiax%c\t%16-19r%21'!, %z3\t@ Deprecated"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0c900b01, 0x0f900f01, "fldmiax%c\t%16-19r%21'!, %z3\t@ Deprecated"},

  /* Data transfer between ARM and NEON registers.  */
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0c400b10, 0x0ff00fd0, "vmov%c\t%0-3,5D, %12-15r, %16-19r"},
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0c500b10, 0x0ff00fd0, "vmov%c\t%12-15r, %16-19r, %0-3,5D"},
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0e000b10, 0x0fd00f70, "vmov%c.32\t%{R:%16-19,7D[%21d]%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0e100b10, 0x0f500f70, "vmov%c.32\t%12-15r, %{R:%16-19,7D[%21d]%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0e000b30, 0x0fd00f30, "vmov%c.16\t%{R:%16-19,7D[%6,21d]%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0e100b30, 0x0f500f30, "vmov%c.%23?us16\t%12-15r, %{R:%16-19,7D[%6,21d]%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0e400b10, 0x0fd00f10, "vmov%c.8\t%{R:%16-19,7D[%5,6,21d]%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0e500b10, 0x0f500f10, "vmov%c.%23?us8\t%12-15r, %{R:%16-19,7D[%5,6,21d]%}"},
  /* Half-precision conversion instructions.  */
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0x0eb20b40, 0x0fbf0f50, "vcvt%7?tb%c.f64.f16\t%z1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0x0eb30b40, 0x0fbf0f50, "vcvt%7?tb%c.f16.f64\t%y1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FP16),
    0x0eb20a40, 0x0fbf0f50, "vcvt%7?tb%c.f32.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FP16),
    0x0eb30a40, 0x0fbf0f50, "vcvt%7?tb%c.f16.f32\t%y1, %y0"},

  /* Floating point coprocessor (VFP) instructions.  */
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ee00a10, 0x0fff0fff, "vmsr%c\t%{R:fpsid%}, %12-15r"},
  {ANY, ARM_FEATURE (0, ARM_EXT2_V8_1M_MAIN, FPU_VFP_EXT_V1xD),
    0x0ee10a10, 0x0fff0fff, "vmsr%c\t%{R:fpscr%}, %12-15r"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0x0ee20a10, 0x0fff0fff, "vmsr%c\t%{R:fpscr_nzcvqc%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ee60a10, 0x0fff0fff, "vmsr%c\t%{R:mvfr1%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ee70a10, 0x0fff0fff, "vmsr%c\t%{R:mvfr0%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0x0ee50a10, 0x0fff0fff, "vmsr%c\t%{R:mvfr2%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ee80a10, 0x0fff0fff, "vmsr%c\t%{R:fpexc%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ee90a10, 0x0fff0fff, "vmsr%c\t%{R:fpinst%}, %12-15r\t@ Impl def"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0eea0a10, 0x0fff0fff, "vmsr%c\t%{R:fpinst2%}, %12-15r\t@ Impl def"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
    0x0eec0a10, 0x0fff0fff, "vmsr%c\t%{R:vpr%}, %12-15r"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
    0x0eed0a10, 0x0fff0fff, "vmsr%c\t%{R:p0%}, %12-15r"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0x0eee0a10, 0x0fff0fff, "vmsr%c\t%{R:fpcxt_ns%}, %12-15r"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0x0eef0a10, 0x0fff0fff, "vmsr%c\t%{R:fpcxt_s%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ef00a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:fpsid%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ef1fa10, 0x0fffffff, "vmrs%c\t%{R:APSR_nzcv%}, %{R:fpscr%}"},
  {ANY, ARM_FEATURE (0, ARM_EXT2_V8_1M_MAIN, FPU_VFP_EXT_V1xD),
    0x0ef10a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:fpscr%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0x0ef20a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:fpscr_nzcvqc%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0x0ef50a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:mvfr2%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ef60a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:mvfr1%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ef70a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:mvfr0%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ef80a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:fpexc%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ef90a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:fpinst%}\t@ Impl def"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0efa0a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:fpinst2%}\t@ Impl def"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
    0x0efc0a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:vpr%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
    0x0efd0a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:p0%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0x0efe0a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:fpcxt_ns%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0x0eff0a10, 0x0fff0fff, "vmrs%c\t%12-15r, %{R:fpcxt_s%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e000b10, 0x0fd00fff, "vmov%c.32\t%z2[%{I:%21d%}], %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e100b10, 0x0fd00fff, "vmov%c.32\t%12-15r, %z2[%{I:%21d%}]"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ee00a10, 0x0ff00fff, "vmsr%c\t<impl def %16-19x>, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ef00a10, 0x0ff00fff, "vmrs%c\t%12-15r, <impl def %16-19x>"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e000a10, 0x0ff00f7f, "vmov%c\t%y2, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e100a10, 0x0ff00f7f, "vmov%c\t%12-15r, %y2"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0eb50a40, 0x0fbf0f70, "vcmp%7'e%c.f32\t%y1, %{I:#0.0%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb50b40, 0x0fbf0f70, "vcmp%7'e%c.f64\t%z1, %{I:#0.0%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0eb00a40, 0x0fbf0fd0, "vmov%c.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0eb00ac0, 0x0fbf0fd0, "vabs%c.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb00b40, 0x0fbf0fd0, "vmov%c.f64\t%z1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb00bc0, 0x0fbf0fd0, "vabs%c.f64\t%z1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0eb10a40, 0x0fbf0fd0, "vneg%c.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0eb10ac0, 0x0fbf0fd0, "vsqrt%c.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb10b40, 0x0fbf0fd0, "vneg%c.f64\t%z1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb10bc0, 0x0fbf0fd0, "vsqrt%c.f64\t%z1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb70ac0, 0x0fbf0fd0, "vcvt%c.f64.f32\t%z1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb70bc0, 0x0fbf0fd0, "vcvt%c.f32.f64\t%y1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0eb80a40, 0x0fbf0f50, "vcvt%c.f32.%7?su32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb80b40, 0x0fbf0f50, "vcvt%c.f64.%7?su32\t%z1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0eb40a40, 0x0fbf0f50, "vcmp%7'e%c.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0eb40b40, 0x0fbf0f50, "vcmp%7'e%c.f64\t%z1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V3xD),
    0x0eba0a40, 0x0fbe0f50, "vcvt%c.f32.%16?us%7?31%7?26\t%y1, %y1, %{I:#%5,0-3k%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V3),
    0x0eba0b40, 0x0fbe0f50, "vcvt%c.f64.%16?us%7?31%7?26\t%z1, %z1, %{I:#%5,0-3k%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0ebc0a40, 0x0fbe0f50, "vcvt%7`r%c.%16?su32.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0ebc0b40, 0x0fbe0f50, "vcvt%7`r%c.%16?su32.f64\t%y1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V3xD),
    0x0ebe0a40, 0x0fbe0f50, "vcvt%c.%16?us%7?31%7?26.f32\t%y1, %y1, %{I:#%5,0-3k%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V3),
    0x0ebe0b40, 0x0fbe0f50, "vcvt%c.%16?us%7?31%7?26.f64\t%z1, %z1, %{I:#%5,0-3k%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0c500b10, 0x0fb00ff0, "vmov%c\t%12-15r, %16-19r, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V3xD),
    0x0eb00a00, 0x0fb00ff0, "vmov%c.f32\t%y1, %{I:#%0-3,16-19E%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V3),
    0x0eb00b00, 0x0fb00ff0, "vmov%c.f64\t%z1, %{I:#%0-3,16-19E%}"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V2),
    0x0c400a10, 0x0ff00fd0, "vmov%c\t%y4, %12-15r, %16-19r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V2),
    0x0c400b10, 0x0ff00fd0, "vmov%c\t%z0, %12-15r, %16-19r"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V2),
    0x0c500a10, 0x0ff00fd0, "vmov%c\t%12-15r, %16-19r, %y4"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e000a00, 0x0fb00f50, "vmla%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e000a40, 0x0fb00f50, "vmls%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e000b00, 0x0fb00f50, "vmla%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e000b40, 0x0fb00f50, "vmls%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e100a00, 0x0fb00f50, "vnmls%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e100a40, 0x0fb00f50, "vnmla%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e100b00, 0x0fb00f50, "vnmls%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e100b40, 0x0fb00f50, "vnmla%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e200a00, 0x0fb00f50, "vmul%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e200a40, 0x0fb00f50, "vnmul%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e200b00, 0x0fb00f50, "vmul%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e200b40, 0x0fb00f50, "vnmul%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e300a00, 0x0fb00f50, "vadd%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e300a40, 0x0fb00f50, "vsub%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e300b00, 0x0fb00f50, "vadd%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e300b40, 0x0fb00f50, "vsub%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD),
    0x0e800a00, 0x0fb00f50, "vdiv%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_V1),
    0x0e800b00, 0x0fb00f50, "vdiv%c.f64\t%z1, %z2, %z0"},

  /* Cirrus coprocessor instructions.  */
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0d100400, 0x0f500f00, "cfldrs%c\t%{R:mvf%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0c100400, 0x0f500f00, "cfldrs%c\t%{R:mvf%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0d500400, 0x0f500f00, "cfldrd%c\t%{R:mvd%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0c500400, 0x0f500f00, "cfldrd%c\t%{R:mvd%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0d100500, 0x0f500f00, "cfldr32%c\t%{R:mvfx%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0c100500, 0x0f500f00, "cfldr32%c\t%{R:mvfx%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0d500500, 0x0f500f00, "cfldr64%c\t%{R:mvdx%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0c500500, 0x0f500f00, "cfldr64%c\t%{R:mvdx%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0d000400, 0x0f500f00, "cfstrs%c\t%{R:mvf%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0c000400, 0x0f500f00, "cfstrs%c\t%{R:mvf%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0d400400, 0x0f500f00, "cfstrd%c\t%{R:mvd%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0c400400, 0x0f500f00, "cfstrd%c\t%{R:mvd%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0d000500, 0x0f500f00, "cfstr32%c\t%{R:mvfx%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0c000500, 0x0f500f00, "cfstr32%c\t%{R:mvfx%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0d400500, 0x0f500f00, "cfstr64%c\t%{R:mvdx%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0c400500, 0x0f500f00, "cfstr64%c\t%{R:mvdx%12-15d%}, %A"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000450, 0x0ff00ff0, "cfmvsr%c\t%{R:mvf%16-19d%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100450, 0x0ff00ff0, "cfmvrs%c\t%12-15r, %{R:mvf%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000410, 0x0ff00ff0, "cfmvdlr%c\t%{R:mvd%16-19d%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100410, 0x0ff00ff0, "cfmvrdl%c\t%12-15r, %{R:mvd%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000430, 0x0ff00ff0, "cfmvdhr%c\t%{R:mvd%16-19d%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100430, 0x0ff00fff, "cfmvrdh%c\t%12-15r, %{R:mvd%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000510, 0x0ff00fff, "cfmv64lr%c\t%{R:mvdx%16-19d%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100510, 0x0ff00fff, "cfmvr64l%c\t%12-15r, %{R:mvdx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000530, 0x0ff00fff, "cfmv64hr%c\t%{R:mvdx%16-19d%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100530, 0x0ff00fff, "cfmvr64h%c\t%12-15r, %{R:mvdx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e200440, 0x0ff00fff, "cfmval32%c\t%{R:mvax%12-15d%}, %{R:mvfx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100440, 0x0ff00fff, "cfmv32al%c\t%{R:mvfx%12-15d%}, %{R:mvax%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e200460, 0x0ff00fff, "cfmvam32%c\t%{R:mvax%12-15d%}, %{R:mvfx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100460, 0x0ff00fff, "cfmv32am%c\t%{R:mvfx%12-15d%}, %{R:mvax%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e200480, 0x0ff00fff, "cfmvah32%c\t%{R:mvax%12-15d%}, %{R:mvfx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100480, 0x0ff00fff, "cfmv32ah%c\t%{R:mvfx%12-15d%}, %{R:mvax%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e2004a0, 0x0ff00fff, "cfmva32%c\t%{R:mvax%12-15d%}, %{R:mvfx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e1004a0, 0x0ff00fff, "cfmv32a%c\t%{R:mvfx%12-15d%}, %{R:mvax%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e2004c0, 0x0ff00fff, "cfmva64%c\t%{R:mvax%12-15d%}, %{R:mvdx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e1004c0, 0x0ff00fff, "cfmv64a%c\t%{R:mvdx%12-15d%}, %{R:mvax%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e2004e0, 0x0fff0fff, "cfmvsc32%c\t%{R:dspsc%}, %{R:mvdx%12-15d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e1004e0, 0x0fff0fff, "cfmv32sc%c\t%{R:mvdx%12-15d%}, %{R:dspsc%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000400, 0x0ff00fff, "cfcpys%c\t%{R:mvf%12-15d%}, %{R:mvf%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000420, 0x0ff00fff, "cfcpyd%c\t%{R:mvd%12-15d%}, %{R:mvd%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000460, 0x0ff00fff, "cfcvtsd%c\t%{R:mvd%12-15d%}, %{R:mvf%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000440, 0x0ff00fff, "cfcvtds%c\t%{R:mvf%12-15d%}, %{R:mvd%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000480, 0x0ff00fff, "cfcvt32s%c\t%{R:mvf%12-15d%}, %{R:mvfx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e0004a0, 0x0ff00fff, "cfcvt32d%c\t%{R:mvd%12-15d%}, %{R:mvfx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e0004c0, 0x0ff00fff, "cfcvt64s%c\t%{R:mvf%12-15d%}, %{R:mvdx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e0004e0, 0x0ff00fff, "cfcvt64d%c\t%{R:mvd%12-15d%}, %{R:mvdx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100580, 0x0ff00fff, "cfcvts32%c\t%{R:mvfx%12-15d%}, %{R:mvf%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e1005a0, 0x0ff00fff, "cfcvtd32%c\t%{R:mvfx%12-15d%}, %{R:mvd%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e1005c0, 0x0ff00fff, "cftruncs32%c\t%{R:mvfx%12-15d%}, %{R:mvf%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e1005e0, 0x0ff00fff, "cftruncd32%c\t%{R:mvfx%12-15d%}, %{R:mvd%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000550, 0x0ff00ff0, "cfrshl32%c\t%{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000570, 0x0ff00ff0, "cfrshl64%c\t%{R:mvdx%16-19d%}, %{R:mvdx%0-3d%}, %12-15r"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000500, 0x0ff00f10, "cfsh32%c\t%{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}, %{I:#%I%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e200500, 0x0ff00f10, "cfsh64%c\t%{R:mvdx%12-15d%}, %{R:mvdx%16-19d%}, %{I:#%I%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100490, 0x0ff00ff0, "cfcmps%c\t%12-15r, %{R:mvf%16-19d%}, %{R:mvf%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e1004b0, 0x0ff00ff0, "cfcmpd%c\t%12-15r, %{R:mvd%16-19d%}, %{R:mvd%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100590, 0x0ff00ff0, "cfcmp32%c\t%12-15r, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e1005b0, 0x0ff00ff0, "cfcmp64%c\t%12-15r, %{R:mvdx%16-19d%}, %{R:mvdx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300400, 0x0ff00fff, "cfabss%c\t%{R:mvf%12-15d%}, %{R:mvf%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300420, 0x0ff00fff, "cfabsd%c\t%{R:mvd%12-15d%}, %{R:mvd%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300440, 0x0ff00fff, "cfnegs%c\t%{R:mvf%12-15d%}, %{R:mvf%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300460, 0x0ff00fff, "cfnegd%c\t%{R:mvd%12-15d%}, %{R:mvd%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300480, 0x0ff00ff0, "cfadds%c\t%{R:mvf%12-15d%}, %{R:mvf%16-19d%}, %{R:mvf%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e3004a0, 0x0ff00ff0, "cfaddd%c\t%{R:mvd%12-15d%}, %{R:mvd%16-19d%}, %{R:mvd%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e3004c0, 0x0ff00ff0, "cfsubs%c\t%{R:mvf%12-15d%}, %{R:mvf%16-19d%}, %{R:mvf%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e3004e0, 0x0ff00ff0, "cfsubd%c\t%{R:mvd%12-15d%}, %{R:mvd%16-19d%}, %{R:mvd%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100400, 0x0ff00ff0, "cfmuls%c\t%{R:mvf%12-15d%}, %{R:mvf%16-19d%}, %{R:mvf%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100420, 0x0ff00ff0, "cfmuld%c\t%{R:mvd%12-15d%}, %{R:mvd%16-19d%}, %{R:mvd%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300500, 0x0ff00fff, "cfabs32%c\t%{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300520, 0x0ff00fff, "cfabs64%c\t%{R:mvdx%12-15d%}, %{R:mvdx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300540, 0x0ff00fff, "cfneg32%c\t%{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300560, 0x0ff00fff, "cfneg64%c\t%{R:mvdx%12-15d%}, %{R:mvdx%16-19d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300580, 0x0ff00ff0, "cfadd32%c\t%{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e3005a0, 0x0ff00ff0, "cfadd64%c\t%{R:mvdx%12-15d%}, %{R:mvdx%16-19d%}, %{R:mvdx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e3005c0, 0x0ff00ff0, "cfsub32%c\t%{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e3005e0, 0x0ff00ff0, "cfsub64%c\t%{R:mvdx%12-15d%}, %{R:mvdx%16-19d%}, %{R:mvdx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100500, 0x0ff00ff0, "cfmul32%c\t%{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100520, 0x0ff00ff0, "cfmul64%c\t%{R:mvdx%12-15d%}, %{R:mvdx%16-19d%}, %{R:mvdx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100540, 0x0ff00ff0, "cfmac32%c\t%{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100560, 0x0ff00ff0, "cfmsc32%c\t%{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e000600, 0x0ff00f10,
    "cfmadd32%c\t%{R:mvax%5-7d%}, %{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e100600, 0x0ff00f10,
    "cfmsub32%c\t%{R:mvax%5-7d%}, %{R:mvfx%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e200600, 0x0ff00f10,
    "cfmadda32%c\t%{R:mvax%5-7d%}, %{R:mvax%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},
  {ANY, ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
    0x0e300600, 0x0ff00f10,
    "cfmsuba32%c\t%{R:mvax%5-7d%}, %{R:mvax%12-15d%}, %{R:mvfx%16-19d%}, %{R:mvfx%0-3d%}"},

  /* VFP Fused multiply add instructions.  */
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA),
    0x0ea00a00, 0x0fb00f50, "vfma%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA),
    0x0ea00b00, 0x0fb00f50, "vfma%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA),
    0x0ea00a40, 0x0fb00f50, "vfms%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA),
    0x0ea00b40, 0x0fb00f50, "vfms%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA),
    0x0e900a40, 0x0fb00f50, "vfnma%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA),
    0x0e900b40, 0x0fb00f50, "vfnma%c.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA),
    0x0e900a00, 0x0fb00f50, "vfnms%c.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA),
    0x0e900b00, 0x0fb00f50, "vfnms%c.f64\t%z1, %z2, %z0"},

  /* FP v5.  */
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfe000a00, 0xff800f50, "vsel%20-21c%u.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfe000b00, 0xff800f50, "vsel%20-21c%u.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfe800a00, 0xffb00f50, "vmaxnm%u.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfe800b00, 0xffb00f50, "vmaxnm%u.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfe800a40, 0xffb00f50, "vminnm%u.f32\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfe800b40, 0xffb00f50, "vminnm%u.f64\t%z1, %z2, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfebc0a40, 0xffbc0f50, "vcvt%16-17?mpna%u.%7?su32.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfebc0b40, 0xffbc0f50, "vcvt%16-17?mpna%u.%7?su32.f64\t%y1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0x0eb60a40, 0x0fbe0f50, "vrint%7,16??xzr%c.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0x0eb60b40, 0x0fbe0f50, "vrint%7,16??xzr%c.f64\t%z1, %z0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfeb80a40, 0xffbc0fd0, "vrint%16-17?mpna%u.f32\t%y1, %y0"},
  {ANY, ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8),
    0xfeb80b40, 0xffbc0fd0, "vrint%16-17?mpna%u.f64\t%z1, %z0"},

  {ANY, ARM_FEATURE_CORE_LOW (0), SENTINEL_GENERIC_START, 0, "" },
  /* ARMv8.3 AdvSIMD instructions in the space of coprocessor 8.  */
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfc800800, 0xfeb00f10, "vcadd%c.f16\t%12-15,22V, %16-19,7V, %0-3,5V, %{I:#%24?29%24'70%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfc900800, 0xfeb00f10, "vcadd%c.f32\t%12-15,22V, %16-19,7V, %0-3,5V, %{I:#%24?29%24'70%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfc200800, 0xff300f10, "vcmla%c.f16\t%12-15,22V, %16-19,7V, %0-3,5V, %{I:#%23'90%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfd200800, 0xff300f10, "vcmla%c.f16\t%12-15,22V, %16-19,7V, %0-3,5V, %{I:#%23?21%23?780%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfc300800, 0xff300f10, "vcmla%c.f32\t%12-15,22V, %16-19,7V, %0-3,5V, %{I:#%23'90%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfd300800, 0xff300f10, "vcmla%c.f32\t%12-15,22V, %16-19,7V, %0-3,5V, %{I:#%23?21%23?780%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfe000800, 0xffa00f10, "vcmla%c.f16\t%12-15,22V, %16-19,7V, %{R:%0-3D[%5?10]%}, %{I:#%20'90%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfe200800, 0xffa00f10, "vcmla%c.f16\t%12-15,22V, %16-19,7V, %{R:%0-3D[%5?10]%}, %{I:#%20?21%20?780%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfe800800, 0xffa00f10, "vcmla%c.f32\t%12-15,22V, %16-19,7V, %{R:%0-3,5D[0]%}, %{I:#%20'90%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0xfea00800, 0xffa00f10, "vcmla%c.f32\t%12-15,22V, %16-19,7V, %{R:%0-3,5D[0]%}, %{I:#%20?21%20?780%}"},

  /* BFloat16 instructions.  */
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16),
    0x0eb30940, 0x0fbf0f50, "vcvt%7?tb%b.bf16.f32\t%y1, %y0"},

  /* Dot Product instructions in the space of coprocessor 13.  */
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_DOTPROD),
    0xfc200d00, 0xffb00f00, "v%4?usdot.%4?us8\t%12-15,22V, %16-19,7V, %0-3,5V"},
  {ANY, ARM_FEATURE_COPROC (FPU_NEON_EXT_DOTPROD),
    0xfe200d00, 0xff200f00, "v%4?usdot.%4?us8\t%12-15,22V, %16-19,7V, %{R:%0-3D[%5?10]%}"},

  /* ARMv8.2 FMAC Long instructions in the space of coprocessor 8.  */
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_V8_2A),
    0xfc200810, 0xffb00f50, "vfmal.f16\t%12-15,22D, %{R:s%7,16-19d%}, %{R:s%5,0-3d%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_V8_2A),
    0xfca00810, 0xffb00f50, "vfmsl.f16\t%12-15,22D, %{R:s%7,16-19d%}, %{R:s%5,0-3d%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_V8_2A),
    0xfc200850, 0xffb00f50, "vfmal.f16\t%12-15,22Q, %{R:d%16-19,7d%}, %{R:d%0-3,5d%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_V8_2A),
    0xfca00850, 0xffb00f50, "vfmsl.f16\t%12-15,22Q, %{R:d%16-19,7d%}, %{R:d%0-3,5d%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_V8_2A),
    0xfe000810, 0xffb00f50, "vfmal.f16\t%12-15,22D, %{R:s%7,16-19d%}, %{R:s%5,0-2d[%3d]%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_V8_2A),
    0xfe100810, 0xffb00f50, "vfmsl.f16\t%12-15,22D, %{R:s%7,16-19d%}, %{R:s%5,0-2d[%3d]%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_V8_2A),
    0xfe000850, 0xffb00f50, "vfmal.f16\t%12-15,22Q, %{R:d%16-19,7d%}, %{R:d%0-2d[%3,5d]%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_V8_2A),
    0xfe100850, 0xffb00f50, "vfmsl.f16\t%12-15,22Q, %{R:d%16-19,7d%}, %{R:d%0-2d[%3,5d]%}"},

  /* ARMv8.2 half-precision Floating point coprocessor 9 (VFP) instructions.
     cp_num: bit <11:8> == 0b1001.
     cond: bit <31:28> == 0b1110, otherwise, it's UNPREDICTABLE.  */
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0eb009c0, 0x0fbf0fd0, "vabs%c.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e300900, 0x0fb00f50, "vadd%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0eb40940, 0x0fbf0f50, "vcmp%7'e%c.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0eb50940, 0x0fbf0f70, "vcmp%7'e%c.f16\t%y1, %{I:#0.0%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0eba09c0, 0x0fbe0fd0, "vcvt%c.f16.%16?us%7?31%7?26\t%y1, %y1, %{I:#%5,0-3k%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0ebe09c0, 0x0fbe0fd0, "vcvt%c.%16?us%7?31%7?26.f16\t%y1, %y1, %{I:#%5,0-3k%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0ebc0940, 0x0fbe0f50, "vcvt%7`r%c.%16?su32.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0eb80940, 0x0fbf0f50, "vcvt%c.f16.%7?su32\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xfebc0940, 0xffbc0f50, "vcvt%16-17?mpna%u.%7?su32.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e800900, 0x0fb00f50, "vdiv%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0ea00900, 0x0fb00f50, "vfma%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0ea00940, 0x0fb00f50, "vfms%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e900940, 0x0fb00f50, "vfnma%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e900900, 0x0fb00f50, "vfnms%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xfeb00ac0, 0xffbf0fd0, "vins.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xfeb00a40, 0xffbf0fd0, "vmovx%c.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0d100900, 0x0f300f00, "vldr%c.16\t%y1, %A"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0d000900, 0x0f300f00, "vstr%c.16\t%y1, %A"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xfe800900, 0xffb00f50, "vmaxnm%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xfe800940, 0xffb00f50, "vminnm%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e000900, 0x0fb00f50, "vmla%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e000940, 0x0fb00f50, "vmls%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e100910, 0x0ff00f7f, "vmov%c.f16\t%12-15r, %y2"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e000910, 0x0ff00f7f, "vmov%c.f16\t%y2, %12-15r"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xeb00900, 0x0fb00ff0, "vmov%c.f16\t%y1, %{I:#%0-3,16-19E%}"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e200900, 0x0fb00f50, "vmul%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0eb10940, 0x0fbf0fd0, "vneg%c.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e100940, 0x0fb00f50, "vnmla%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e100900, 0x0fb00f50, "vnmls%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e200940, 0x0fb00f50, "vnmul%c.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0eb60940, 0x0fbe0f50, "vrint%7,16??xzr%c.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xfeb80940, 0xffbc0fd0, "vrint%16-17?mpna%u.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xfe000900, 0xff800f50, "vsel%20-21c%u.f16\t%y1, %y2, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0eb109c0, 0x0fbf0fd0, "vsqrt%c.f16\t%y1, %y0"},
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0x0e300940, 0x0fb00f50, "vsub%c.f16\t%y1, %y2, %y0"},

  /* ARMv8.3 javascript conversion instruction.  */
  {ANY, ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A),
    0x0eb90bc0, 0x0fbf0fd0, "vjcvt%c.s32.f64\t%y1, %z0"},

  {ANY, ARM_FEATURE_CORE_LOW (0), 0, 0, 0}
};

/* Generic coprocessor instructions.  These are only matched if a more specific
   SIMD or co-processor instruction does not match first.  */

static const struct sopcode32 generic_coprocessor_opcodes[] =
{
  /* Generic coprocessor instructions.  */
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V5E),
    0x0c400000, 0x0ff00000, "mcrr%c\t%{I:%8-11d%}, %{I:%4-7d%}, %12-15R, %16-19r, %{R:cr%0-3d%}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V5E),
    0x0c500000, 0x0ff00000,
    "mrrc%c\t%{I:%8-11d%}, %{I:%4-7d%}, %12-15Ru, %16-19Ru, %{R:cr%0-3d%}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V2),
    0x0e000000, 0x0f000010,
    "cdp%c\t%{I:%8-11d%}, %{I:%20-23d%}, %{R:cr%12-15d%}, %{R:cr%16-19d%}, %{R:cr%0-3d%}, {%{I:%5-7d%}}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V2),
    0x0e10f010, 0x0f10f010,
    "mrc%c\t%{I:%8-11d%}, %{I:%21-23d%}, %{R:APSR_nzcv%}, %{R:cr%16-19d%}, %{R:cr%0-3d%}, {%{I:%5-7d%}}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V2),
    0x0e100010, 0x0f100010,
    "mrc%c\t%{I:%8-11d%}, %{I:%21-23d%}, %12-15r, %{R:cr%16-19d%}, %{R:cr%0-3d%}, {%{I:%5-7d%}}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V2),
    0x0e000010, 0x0f100010,
    "mcr%c\t%{I:%8-11d%}, %{I:%21-23d%}, %12-15R, %{R:cr%16-19d%}, %{R:cr%0-3d%}, {%{I:%5-7d%}}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V2),
    0x0c000000, 0x0e100000, "stc%22'l%c\t%{I:%8-11d%}, %{R:cr%12-15d%}, %A"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V2),
    0x0c100000, 0x0e100000, "ldc%22'l%c\t%{I:%8-11d%}, %{R:cr%12-15d%}, %A"},

  /* V6 coprocessor instructions.  */
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xfc500000, 0xfff00000,
    "mrrc2%c\t%{I:%8-11d%}, %{I:%4-7d%}, %12-15Ru, %16-19Ru, %{R:cr%0-3d%}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xfc400000, 0xfff00000,
    "mcrr2%c\t%{I:%8-11d%}, %{I:%4-7d%}, %12-15R, %16-19R, %{R:cr%0-3d%}"},

  /* V5 coprocessor instructions.  */
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0xfc100000, 0xfe100000, "ldc2%22'l%c\t%{I:%8-11d%}, %{R:cr%12-15d%}, %A"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0xfc000000, 0xfe100000, "stc2%22'l%c\t%{I:%8-11d%}, %{R:cr%12-15d%}, %A"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0xfe000000, 0xff000010,
    "cdp2%c\t%{I:%8-11d%}, %{I:%20-23d%}, %{R:cr%12-15d%}, %{R:cr%16-19d%}, %{R:cr%0-3d%}, {%{I:%5-7d%}}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0xfe000010, 0xff100010,
    "mcr2%c\t%{I:%8-11d%}, %{I:%21-23d%}, %12-15R, %{R:cr%16-19d%}, %{R:cr%0-3d%}, {%{I:%5-7d%}}"},
  {ANY, ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0xfe100010, 0xff100010,
    "mrc2%c\t%{I:%8-11d%}, %{I:%21-23d%}, %12-15r, %{R:cr%16-19d%}, %{R:cr%0-3d%}, {%{I:%5-7d%}}"},

  {ANY, ARM_FEATURE_CORE_LOW (0), 0, 0, 0}
};

/* Neon opcode table:  This does not encode the top byte -- that is
   checked by the print_insn_neon routine, as it depends on whether we are
   doing thumb32 or arm32 disassembly.  */

/* print_insn_neon recognizes the following format control codes:

   %%			%

   %c			print condition code
   %u			print condition code (unconditional in ARM mode,
                          UNPREDICTABLE if not AL in Thumb)
   %A			print v{st,ld}[1234] operands
   %B			print v{st,ld}[1234] any one operands
   %C			print v{st,ld}[1234] single->all operands
   %D			print scalar
   %E			print vmov, vmvn, vorr, vbic encoded constant
   %F			print vtbl,vtbx register list

   %<bitfield>r		print as an ARM register
   %<bitfield>d		print the bitfield in decimal
   %<bitfield>e         print the 2^N - bitfield in decimal
   %<bitfield>D		print as a NEON D register
   %<bitfield>Q		print as a NEON Q register
   %<bitfield>R		print as a NEON D or Q register
   %<bitfield>Sn	print byte scaled width limited by n
   %<bitfield>Tn	print short scaled width limited by n
   %<bitfield>Un	print long scaled width limited by n

   %<bitfield>'c	print specified char iff bitfield is all ones
   %<bitfield>`c	print specified char iff bitfield is all zeroes
   %<bitfield>?ab...    select from array of values in big endian order.  */

static const struct opcode32 neon_opcodes[] =
{
  /* Extract.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2b00840, 0xffb00850,
    "vext%c.8\t%12-15,22R, %16-19,7R, %0-3,5R, %{I:#%8-11d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2b00000, 0xffb00810,
    "vext%c.8\t%12-15,22R, %16-19,7R, %0-3,5R, %{I:#%8-11d%}"},

  /* Data transfer between ARM and NEON registers.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0e800b10, 0x0ff00f70, "vdup%c.32\t%16-19,7D, %12-15r"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0e800b30, 0x0ff00f70, "vdup%c.16\t%16-19,7D, %12-15r"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0ea00b10, 0x0ff00f70, "vdup%c.32\t%16-19,7Q, %12-15r"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0ea00b30, 0x0ff00f70, "vdup%c.16\t%16-19,7Q, %12-15r"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0ec00b10, 0x0ff00f70, "vdup%c.8\t%16-19,7D, %12-15r"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0x0ee00b10, 0x0ff00f70, "vdup%c.8\t%16-19,7Q, %12-15r"},

  /* Move data element to all lanes.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b40c00, 0xffb70f90, "vdup%c.32\t%12-15,22R, %{R:%0-3,5D[%19d]%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20c00, 0xffb30f90, "vdup%c.16\t%12-15,22R, %{R:%0-3,5D[%18-19d]%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b10c00, 0xffb10f90, "vdup%c.8\t%12-15,22R, %{R:%0-3,5D[%17-19d]%}"},

  /* Table lookup.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00800, 0xffb00c50, "vtbl%c.8\t%12-15,22D, %F, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00840, 0xffb00c50, "vtbx%c.8\t%12-15,22D, %F, %0-3,5D"},

  /* Half-precision conversions.  */
  {ARM_FEATURE_COPROC (FPU_VFP_EXT_FP16),
    0xf3b60600, 0xffbf0fd0, "vcvt%c.f16.f32\t%12-15,22D, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_VFP_EXT_FP16),
    0xf3b60700, 0xffbf0fd0, "vcvt%c.f32.f16\t%12-15,22Q, %0-3,5D"},

  /* NEON fused multiply add instructions.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_FMA),
    0xf2000c10, 0xffb00f10, "vfma%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2100c10, 0xffb00f10, "vfma%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_FMA),
    0xf2200c10, 0xffb00f10, "vfms%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2300c10, 0xffb00f10, "vfms%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},

  /* BFloat16 instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16),
    0xfc000d00, 0xffb00f10, "vdot.bf16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16),
    0xfe000d00, 0xffb00f10, "vdot.bf16\t%12-15,22R, %16-19,7R, %{R:d%0-3d[%5d]%}"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16),
    0xfc000c40, 0xffb00f50, "vmmla.bf16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16),
    0xf3b60640, 0xffbf0fd0, "vcvt%c.bf16.f32\t%12-15,22D, %0-3,5Q"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16),
    0xfc300810, 0xffb00f10, "vfma%6?tb.bf16\t%12-15,22Q, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16),
    0xfe300810, 0xffb00f10, "vfma%6?tb.bf16\t%12-15,22Q, %16-19,7Q, %{R:%0-2D[%3,5d]%}"},

  /* Matrix Multiply instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM),
    0xfc200c40, 0xffb00f50, "vsmmla.s8\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM),
    0xfc200c50, 0xffb00f50, "vummla.u8\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM),
    0xfca00c40, 0xffb00f50, "vusmmla.s8\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM),
    0xfca00d00, 0xffb00f10, "vusdot.s8\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM),
    0xfe800d00, 0xffb00f10, "vusdot.s8\t%12-15,22R, %16-19,7R, %{R:d%0-3d[%5d]%}"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM),
    0xfe800d10, 0xffb00f10, "vsudot.u8\t%12-15,22R, %16-19,7R, %{R:d%0-3d[%5d]%}"},

  /* Two registers, miscellaneous.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_ARMV8),
    0xf3ba0400, 0xffbf0c10, "vrint%7-9?p?m?zaxn%u.f32\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3b60400, 0xffbf0c10, "vrint%7-9?p?m?zaxn%u.f16\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_ARMV8),
    0xf3bb0000, 0xffbf0c10, "vcvt%8-9?mpna%u.%7?us32.f32\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3b70000, 0xffbf0c10, "vcvt%8-9?mpna%u.%7?us16.f16\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3b00300, 0xffbf0fd0, "aese%u.8\t%12-15,22Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3b00340, 0xffbf0fd0, "aesd%u.8\t%12-15,22Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3b00380, 0xffbf0fd0, "aesmc%u.8\t%12-15,22Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3b003c0, 0xffbf0fd0, "aesimc%u.8\t%12-15,22Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3b902c0, 0xffbf0fd0, "sha1h%u.32\t%12-15,22Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3ba0380, 0xffbf0fd0, "sha1su1%u.32\t%12-15,22Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3ba03c0, 0xffbf0fd0, "sha256su0%u.32\t%12-15,22Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880a10, 0xfebf0fd0, "vmovl%c.%24?us8\t%12-15,22Q, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900a10, 0xfebf0fd0, "vmovl%c.%24?us16\t%12-15,22Q, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00a10, 0xfebf0fd0, "vmovl%c.%24?us32\t%12-15,22Q, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00500, 0xffbf0f90, "vcnt%c.8\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00580, 0xffbf0f90, "vmvn%c\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20000, 0xffbf0f90, "vswp%c\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20200, 0xffb30fd0, "vmovn%c.i%18-19T2\t%12-15,22D, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20240, 0xffb30fd0, "vqmovun%c.s%18-19T2\t%12-15,22D, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20280, 0xffb30fd0, "vqmovn%c.s%18-19T2\t%12-15,22D, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b202c0, 0xffb30fd0, "vqmovn%c.u%18-19T2\t%12-15,22D, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20300, 0xffb30fd0,
    "vshll%c.i%18-19S2\t%12-15,22Q, %0-3,5D, %{I:#%18-19S2%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3bb0400, 0xffbf0e90, "vrecpe%c.%8?fu%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3b70400, 0xffbf0e90, "vrecpe%c.%8?fu16\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3bb0480, 0xffbf0e90, "vrsqrte%c.%8?fu%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3b70480, 0xffbf0e90, "vrsqrte%c.%8?fu16\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00000, 0xffb30f90, "vrev64%c.%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00080, 0xffb30f90, "vrev32%c.%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00100, 0xffb30f90, "vrev16%c.%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00400, 0xffb30f90, "vcls%c.s%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00480, 0xffb30f90, "vclz%c.i%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00700, 0xffb30f90, "vqabs%c.s%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00780, 0xffb30f90, "vqneg%c.s%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20080, 0xffb30f90, "vtrn%c.%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20100, 0xffb30f90, "vuzp%c.%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b20180, 0xffb30f90, "vzip%c.%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b10000, 0xffb30b90, "vcgt%c.%10?fs%18-19S2\t%12-15,22R, %0-3,5R, %{I:#0%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b10080, 0xffb30b90, "vcge%c.%10?fs%18-19S2\t%12-15,22R, %0-3,5R, %{I:#0%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b10100, 0xffb30b90, "vceq%c.%10?fi%18-19S2\t%12-15,22R, %0-3,5R, %{I:#0%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b10180, 0xffb30b90, "vcle%c.%10?fs%18-19S2\t%12-15,22R, %0-3,5R, %{I:#0%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b10200, 0xffb30b90, "vclt%c.%10?fs%18-19S2\t%12-15,22R, %0-3,5R, %{I:#0%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b10300, 0xffb30b90, "vabs%c.%10?fs%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b10380, 0xffb30b90, "vneg%c.%10?fs%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00200, 0xffb30f10, "vpaddl%c.%7?us%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3b00600, 0xffb30f10, "vpadal%c.%7?us%18-19S2\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3bb0600, 0xffbf0e10,
    "vcvt%c.%7-8?usff%18-19Sa.%7-8?ffus%18-19Sa\t%12-15,22R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3b70600, 0xffbf0e10,
    "vcvt%c.%7-8?usff16.%7-8?ffus16\t%12-15,22R, %0-3,5R"},

  /* Three registers of the same length.  */
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf2000c40, 0xffb00f50, "sha1c%u.32\t%12-15,22Q, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf2100c40, 0xffb00f50, "sha1p%u.32\t%12-15,22Q, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf2200c40, 0xffb00f50, "sha1m%u.32\t%12-15,22Q, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf2300c40, 0xffb00f50, "sha1su0%u.32\t%12-15,22Q, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3000c40, 0xffb00f50, "sha256h%u.32\t%12-15,22Q, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3100c40, 0xffb00f50, "sha256h2%u.32\t%12-15,22Q, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf3200c40, 0xffb00f50, "sha256su1%u.32\t%12-15,22Q, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_ARMV8),
    0xf3000f10, 0xffb00f10, "vmaxnm%u.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3100f10, 0xffb00f10, "vmaxnm%u.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_ARMV8),
    0xf3200f10, 0xffb00f10, "vminnm%u.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3300f10, 0xffb00f10, "vminnm%u.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000110, 0xffb00f10, "vand%c\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2100110, 0xffb00f10, "vbic%c\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2200110, 0xffb00f10, "vorr%c\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2300110, 0xffb00f10, "vorn%c\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000110, 0xffb00f10, "veor%c\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3100110, 0xffb00f10, "vbsl%c\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3200110, 0xffb00f10, "vbit%c\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3300110, 0xffb00f10, "vbif%c\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000d00, 0xffb00f10, "vadd%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2100d00, 0xffb00f10, "vadd%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000d10, 0xffb00f10, "vmla%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2100d10, 0xffb00f10, "vmla%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000e00, 0xffb00f10, "vceq%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2100e00, 0xffb00f10, "vceq%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000f00, 0xffb00f10, "vmax%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2100f00, 0xffb00f10, "vmax%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000f10, 0xffb00f10, "vrecps%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2100f10, 0xffb00f10, "vrecps%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2200d00, 0xffb00f10, "vsub%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2300d00, 0xffb00f10, "vsub%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2200d10, 0xffb00f10, "vmls%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2300d10, 0xffb00f10, "vmls%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2200f00, 0xffb00f10, "vmin%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2300f00, 0xffb00f10, "vmin%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2200f10, 0xffb00f10, "vrsqrts%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2300f10, 0xffb00f10, "vrsqrts%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000d00, 0xffb00f10, "vpadd%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3100d00, 0xffb00f10, "vpadd%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000d10, 0xffb00f10, "vmul%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3100d10, 0xffb00f10, "vmul%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000e00, 0xffb00f10, "vcge%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3100e00, 0xffb00f10, "vcge%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000e10, 0xffb00f10, "vacge%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3100e10, 0xffb00f10, "vacge%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000f00, 0xffb00f10, "vpmax%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3100f00, 0xffb00f10, "vpmax%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3200d00, 0xffb00f10, "vabd%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3300d00, 0xffb00f10, "vabd%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3200e00, 0xffb00f10, "vcgt%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3300e00, 0xffb00f10, "vcgt%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3200e10, 0xffb00f10, "vacgt%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3300e10, 0xffb00f10, "vacgt%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3200f00, 0xffb00f10, "vpmin%c.f32\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf3300f00, 0xffb00f10, "vpmin%c.f16\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000800, 0xff800f10, "vadd%c.i%20-21S3\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000810, 0xff800f10, "vtst%c.%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000900, 0xff800f10, "vmla%c.i%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000b00, 0xff800f10,
    "vqdmulh%c.s%20-21S6\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000b10, 0xff800f10,
    "vpadd%c.i%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000800, 0xff800f10, "vsub%c.i%20-21S3\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000810, 0xff800f10, "vceq%c.i%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000900, 0xff800f10, "vmls%c.i%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3000b00, 0xff800f10,
    "vqrdmulh%c.s%20-21S6\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000000, 0xfe800f10,
    "vhadd%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000010, 0xfe800f10,
    "vqadd%c.%24?us%20-21S3\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000100, 0xfe800f10,
    "vrhadd%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000200, 0xfe800f10,
    "vhsub%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000210, 0xfe800f10,
    "vqsub%c.%24?us%20-21S3\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000300, 0xfe800f10,
    "vcgt%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000310, 0xfe800f10,
    "vcge%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000400, 0xfe800f10,
    "vshl%c.%24?us%20-21S3\t%12-15,22R, %0-3,5R, %16-19,7R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000410, 0xfe800f10,
    "vqshl%c.%24?us%20-21S3\t%12-15,22R, %0-3,5R, %16-19,7R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000500, 0xfe800f10,
    "vrshl%c.%24?us%20-21S3\t%12-15,22R, %0-3,5R, %16-19,7R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000510, 0xfe800f10,
    "vqrshl%c.%24?us%20-21S3\t%12-15,22R, %0-3,5R, %16-19,7R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000600, 0xfe800f10,
    "vmax%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000610, 0xfe800f10,
    "vmin%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000700, 0xfe800f10,
    "vabd%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000710, 0xfe800f10,
    "vaba%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000910, 0xfe800f10,
    "vmul%c.%24?pi%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000a00, 0xfe800f10,
    "vpmax%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2000a10, 0xfe800f10,
    "vpmin%c.%24?us%20-21S2\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_RDMA),
    0xf3000b10, 0xff800f10,
    "vqrdmlah%c.s%20-21S6\t%12-15,22R, %16-19,7R, %0-3,5R"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_RDMA),
    0xf3000c10, 0xff800f10,
    "vqrdmlsh%c.s%20-21S6\t%12-15,22R, %16-19,7R, %0-3,5R"},

  /* One register and an immediate value.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800e10, 0xfeb80fb0, "vmov%c.i8\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800e30, 0xfeb80fb0, "vmov%c.i64\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800f10, 0xfeb80fb0, "vmov%c.f32\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800810, 0xfeb80db0, "vmov%c.i16\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800830, 0xfeb80db0, "vmvn%c.i16\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800910, 0xfeb80db0, "vorr%c.i16\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800930, 0xfeb80db0, "vbic%c.i16\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800c10, 0xfeb80eb0, "vmov%c.i32\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800c30, 0xfeb80eb0, "vmvn%c.i32\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800110, 0xfeb809b0, "vorr%c.i32\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800130, 0xfeb809b0, "vbic%c.i32\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800010, 0xfeb808b0, "vmov%c.i32\t%12-15,22R, %E"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800030, 0xfeb808b0, "vmvn%c.i32\t%12-15,22R, %E"},

  /* Two registers and a shift amount.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880810, 0xffb80fd0, "vshrn%c.i16\t%12-15,22D, %0-3,5Q, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880850, 0xffb80fd0, "vrshrn%c.i16\t%12-15,22D, %0-3,5Q, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880810, 0xfeb80fd0, "vqshrun%c.s16\t%12-15,22D, %0-3,5Q, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880850, 0xfeb80fd0, "vqrshrun%c.s16\t%12-15,22D, %0-3,5Q, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880910, 0xfeb80fd0, "vqshrn%c.%24?us16\t%12-15,22D, %0-3,5Q, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880950, 0xfeb80fd0,
    "vqrshrn%c.%24?us16\t%12-15,22D, %0-3,5Q, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880a10, 0xfeb80fd0, "vshll%c.%24?us8\t%12-15,22Q, %0-3,5D, %{I:#%16-18d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900810, 0xffb00fd0, "vshrn%c.i32\t%12-15,22D, %0-3,5Q, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900850, 0xffb00fd0, "vrshrn%c.i32\t%12-15,22D, %0-3,5Q, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880510, 0xffb80f90, "vshl%c.%24?us8\t%12-15,22R, %0-3,5R, %{I:#%16-18d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3880410, 0xffb80f90, "vsri%c.8\t%12-15,22R, %0-3,5R, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3880510, 0xffb80f90, "vsli%c.8\t%12-15,22R, %0-3,5R, %{I:#%16-18d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3880610, 0xffb80f90, "vqshlu%c.s8\t%12-15,22R, %0-3,5R, %{I:#%16-18d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900810, 0xfeb00fd0, "vqshrun%c.s32\t%12-15,22D, %0-3,5Q, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900850, 0xfeb00fd0, "vqrshrun%c.s32\t%12-15,22D, %0-3,5Q, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900910, 0xfeb00fd0, "vqshrn%c.%24?us32\t%12-15,22D, %0-3,5Q, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900950, 0xfeb00fd0,
    "vqrshrn%c.%24?us32\t%12-15,22D, %0-3,5Q, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900a10, 0xfeb00fd0, "vshll%c.%24?us16\t%12-15,22Q, %0-3,5D, %{I:#%16-19d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880010, 0xfeb80f90, "vshr%c.%24?us8\t%12-15,22R, %0-3,5R, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880110, 0xfeb80f90, "vsra%c.%24?us8\t%12-15,22R, %0-3,5R, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880210, 0xfeb80f90, "vrshr%c.%24?us8\t%12-15,22R, %0-3,5R, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880310, 0xfeb80f90, "vrsra%c.%24?us8\t%12-15,22R, %0-3,5R, %{I:#%16-18e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2880710, 0xfeb80f90, "vqshl%c.%24?us8\t%12-15,22R, %0-3,5R, %{I:#%16-18d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00810, 0xffa00fd0, "vshrn%c.i64\t%12-15,22D, %0-3,5Q, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00850, 0xffa00fd0, "vrshrn%c.i64\t%12-15,22D, %0-3,5Q, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900510, 0xffb00f90, "vshl%c.%24?us16\t%12-15,22R, %0-3,5R, %{I:#%16-19d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3900410, 0xffb00f90, "vsri%c.16\t%12-15,22R, %0-3,5R, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3900510, 0xffb00f90, "vsli%c.16\t%12-15,22R, %0-3,5R, %{I:#%16-19d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3900610, 0xffb00f90, "vqshlu%c.s16\t%12-15,22R, %0-3,5R, %{I:#%16-19d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00a10, 0xfea00fd0, "vshll%c.%24?us32\t%12-15,22Q, %0-3,5D, %{I:#%16-20d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900010, 0xfeb00f90, "vshr%c.%24?us16\t%12-15,22R, %0-3,5R, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900110, 0xfeb00f90, "vsra%c.%24?us16\t%12-15,22R, %0-3,5R, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900210, 0xfeb00f90, "vrshr%c.%24?us16\t%12-15,22R, %0-3,5R, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900310, 0xfeb00f90, "vrsra%c.%24?us16\t%12-15,22R, %0-3,5R, %{I:#%16-19e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2900710, 0xfeb00f90, "vqshl%c.%24?us16\t%12-15,22R, %0-3,5R, %{I:#%16-19d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00810, 0xfea00fd0, "vqshrun%c.s64\t%12-15,22D, %0-3,5Q, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00850, 0xfea00fd0, "vqrshrun%c.s64\t%12-15,22D, %0-3,5Q, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00910, 0xfea00fd0, "vqshrn%c.%24?us64\t%12-15,22D, %0-3,5Q, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00950, 0xfea00fd0,
    "vqrshrn%c.%24?us64\t%12-15,22D, %0-3,5Q, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00510, 0xffa00f90, "vshl%c.%24?us32\t%12-15,22R, %0-3,5R, %{I:#%16-20d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3a00410, 0xffa00f90, "vsri%c.32\t%12-15,22R, %0-3,5R, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3a00510, 0xffa00f90, "vsli%c.32\t%12-15,22R, %0-3,5R, %{I:#%16-20d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3a00610, 0xffa00f90, "vqshlu%c.s32\t%12-15,22R, %0-3,5R, %{I:#%16-20d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00010, 0xfea00f90, "vshr%c.%24?us32\t%12-15,22R, %0-3,5R, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00110, 0xfea00f90, "vsra%c.%24?us32\t%12-15,22R, %0-3,5R, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00210, 0xfea00f90, "vrshr%c.%24?us32\t%12-15,22R, %0-3,5R, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00310, 0xfea00f90, "vrsra%c.%24?us32\t%12-15,22R, %0-3,5R, %{I:#%16-20e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00710, 0xfea00f90, "vqshl%c.%24?us32\t%12-15,22R, %0-3,5R, %{I:#%16-20d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800590, 0xff800f90, "vshl%c.%24?us64\t%12-15,22R, %0-3,5R, %{I:#%16-21d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800490, 0xff800f90, "vsri%c.64\t%12-15,22R, %0-3,5R, %{I:#%16-21e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800590, 0xff800f90, "vsli%c.64\t%12-15,22R, %0-3,5R, %{I:#%16-21d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800690, 0xff800f90, "vqshlu%c.s64\t%12-15,22R, %0-3,5R, %{I:#%16-21d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800090, 0xfe800f90, "vshr%c.%24?us64\t%12-15,22R, %0-3,5R, %{I:#%16-21e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800190, 0xfe800f90, "vsra%c.%24?us64\t%12-15,22R, %0-3,5R, %{I:#%16-21e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800290, 0xfe800f90, "vrshr%c.%24?us64\t%12-15,22R, %0-3,5R, %{I:#%16-21e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800390, 0xfe800f90, "vrsra%c.%24?us64\t%12-15,22R, %0-3,5R, %{I:#%16-21e%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800790, 0xfe800f90, "vqshl%c.%24?us64\t%12-15,22R, %0-3,5R, %{I:#%16-21d%}"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2a00e10, 0xfea00e90,
    "vcvt%c.%24,8?usff32.%24,8?ffus32\t%12-15,22R, %0-3,5R, %{I:#%16-20e%}"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
    0xf2a00c10, 0xfea00e90,
    "vcvt%c.%24,8?usff16.%24,8?ffus16\t%12-15,22R, %0-3,5R, %{I:#%16-20e%}"},

  /* Three registers of different lengths.  */
  {ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8),
    0xf2a00e00, 0xfeb00f50, "vmull%c.p64\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800e00, 0xfea00f50, "vmull%c.p%20S0\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800400, 0xff800f50,
    "vaddhn%c.i%20-21T2\t%12-15,22D, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800600, 0xff800f50,
    "vsubhn%c.i%20-21T2\t%12-15,22D, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800900, 0xff800f50,
    "vqdmlal%c.s%20-21S6\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800b00, 0xff800f50,
    "vqdmlsl%c.s%20-21S6\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800d00, 0xff800f50,
    "vqdmull%c.s%20-21S6\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800400, 0xff800f50,
    "vraddhn%c.i%20-21T2\t%12-15,22D, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800600, 0xff800f50,
    "vrsubhn%c.i%20-21T2\t%12-15,22D, %16-19,7Q, %0-3,5Q"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800000, 0xfe800f50,
    "vaddl%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800100, 0xfe800f50,
    "vaddw%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7Q, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800200, 0xfe800f50,
    "vsubl%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800300, 0xfe800f50,
    "vsubw%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7Q, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800500, 0xfe800f50,
    "vabal%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800700, 0xfe800f50,
    "vabdl%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800800, 0xfe800f50,
    "vmlal%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800a00, 0xfe800f50,
    "vmlsl%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7D, %0-3,5D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800c00, 0xfe800f50,
    "vmull%c.%24?us%20-21S2\t%12-15,22Q, %16-19,7D, %0-3,5D"},

  /* Two registers and a scalar.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800040, 0xff800f50, "vmla%c.i%20-21S6\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800140, 0xff900f50, "vmla%c.f%20-21Sa\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (ARM_EXT2_FP16_INST),
    0xf2900140, 0xffb00f50, "vmla%c.f16\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800340, 0xff800f50, "vqdmlal%c.s%20-21S6\t%12-15,22Q, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800440, 0xff800f50, "vmls%c.i%20-21S6\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800540, 0xff900f50, "vmls%c.f%20-21S6\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (ARM_EXT2_FP16_INST),
    0xf2900540, 0xffb00f50, "vmls%c.f16\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800740, 0xff800f50, "vqdmlsl%c.s%20-21S6\t%12-15,22Q, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800840, 0xff800f50, "vmul%c.i%20-21S6\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800940, 0xff900f50, "vmul%c.f%20-21Sa\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (ARM_EXT2_FP16_INST),
    0xf2900940, 0xffb00f50, "vmul%c.f16\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800b40, 0xff800f50, "vqdmull%c.s%20-21S6\t%12-15,22Q, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800c40, 0xff800f50, "vqdmulh%c.s%20-21S6\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800d40, 0xff800f50, "vqrdmulh%c.s%20-21S6\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800040, 0xff800f50, "vmla%c.i%20-21S6\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800140, 0xff900f50, "vmla%c.f%20-21Sa\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (ARM_EXT2_FP16_INST),
    0xf3900140, 0xffb00f50, "vmla%c.f16\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800440, 0xff800f50, "vmls%c.i%20-21S6\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800540, 0xff900f50, "vmls%c.f%20-21Sa\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (ARM_EXT2_FP16_INST),
    0xf3900540, 0xffb00f50, "vmls%c.f16\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800840, 0xff800f50, "vmul%c.i%20-21S6\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800940, 0xff900f50, "vmul%c.f%20-21Sa\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (ARM_EXT2_FP16_INST),
    0xf3900940, 0xffb00f50, "vmul%c.f16\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800c40, 0xff800f50, "vqdmulh%c.s%20-21S6\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf3800d40, 0xff800f50, "vqrdmulh%c.s%20-21S6\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800240, 0xfe800f50,
    "vmlal%c.%24?us%20-21S6\t%12-15,22Q, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800640, 0xfe800f50,
    "vmlsl%c.%24?us%20-21S6\t%12-15,22Q, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf2800a40, 0xfe800f50,
    "vmull%c.%24?us%20-21S6\t%12-15,22Q, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_RDMA),
    0xf2800e40, 0xff800f50,
   "vqrdmlah%c.s%20-21S6\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_RDMA),
    0xf2800f40, 0xff800f50,
   "vqrdmlsh%c.s%20-21S6\t%12-15,22D, %16-19,7D, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_RDMA),
    0xf3800e40, 0xff800f50,
   "vqrdmlah%c.s%20-21S6\t%12-15,22Q, %16-19,7Q, %D"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_RDMA),
    0xf3800f40, 0xff800f50,
   "vqrdmlsh%c.s%20-21S6\t%12-15,22Q, %16-19,7Q, %D"
  },

  /* Element and structure load/store.  */
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4a00fc0, 0xffb00fc0, "vld4%c.32\t%C"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4a00c00, 0xffb00f00, "vld1%c.%6-7S2\t%C"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4a00d00, 0xffb00f00, "vld2%c.%6-7S2\t%C"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4a00e00, 0xffb00f00, "vld3%c.%6-7S2\t%C"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4a00f00, 0xffb00f00, "vld4%c.%6-7S2\t%C"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000200, 0xff900f00, "v%21?ls%21?dt1%c.%6-7S3\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000300, 0xff900f00, "v%21?ls%21?dt2%c.%6-7S2\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000400, 0xff900f00, "v%21?ls%21?dt3%c.%6-7S2\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000500, 0xff900f00, "v%21?ls%21?dt3%c.%6-7S2\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000600, 0xff900f00, "v%21?ls%21?dt1%c.%6-7S3\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000700, 0xff900f00, "v%21?ls%21?dt1%c.%6-7S3\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000800, 0xff900f00, "v%21?ls%21?dt2%c.%6-7S2\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000900, 0xff900f00, "v%21?ls%21?dt2%c.%6-7S2\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000a00, 0xff900f00, "v%21?ls%21?dt1%c.%6-7S3\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4000000, 0xff900e00, "v%21?ls%21?dt4%c.%6-7S2\t%A"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4800000, 0xff900300, "v%21?ls%21?dt1%c.%10-11S2\t%B"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4800100, 0xff900300, "v%21?ls%21?dt2%c.%10-11S2\t%B"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4800200, 0xff900300, "v%21?ls%21?dt3%c.%10-11S2\t%B"},
  {ARM_FEATURE_COPROC (FPU_NEON_EXT_V1),
    0xf4800300, 0xff900300, "v%21?ls%21?dt4%c.%10-11S2\t%B"},

  {ARM_FEATURE_CORE_LOW (0), 0 ,0, 0}
};

/* mve opcode table.  */

/* print_insn_mve recognizes the following format control codes:

   %%			%

   %a			print '+' or '-' or imm offset in vldr[bhwd] and
			vstr[bhwd]
   %c			print condition code
   %d			print addr mode of MVE vldr[bhw] and vstr[bhw]
   %u			print 'U' (unsigned) or 'S' for various mve instructions
   %i			print MVE predicate(s) for vpt and vpst
   %j			print a 5-bit immediate from hw2[14:12,7:6]
   %k			print 48 if the 7th position bit is set else print 64.
   %m			print rounding mode for vcvt and vrint
   %n			print vector comparison code for predicated instruction
   %s			print size for various vcvt instructions
   %v			print vector predicate for instruction in predicated
			block
   %o			print offset scaled for vldr[hwd] and vstr[hwd]
   %w			print writeback mode for MVE v{st,ld}[24]
   %B			print v{st,ld}[24] any one operands
   %E			print vmov, vmvn, vorr, vbic encoded constant
   %N			print generic index for vmov
   %T			print bottom ('b') or top ('t') of source register
   %X			print exchange field in vmla* instructions

   %<bitfield>r		print as an ARM register
   %<bitfield>d		print the bitfield in decimal
   %<bitfield>A		print accumulate or not
   %<bitfield>c		print bitfield as a condition code
   %<bitfield>C		print bitfield as an inverted condition code
   %<bitfield>Q		print as a MVE Q register
   %<bitfield>F		print as a MVE S register
   %<bitfield>Z		as %<>r but r15 is ZR instead of PC and r13 is
			UNPREDICTABLE

   %<bitfield>S		as %<>r but r15 or r13 is UNPREDICTABLE
   %<bitfield>s		print size for vector predicate & non VMOV instructions
   %<bitfield>I		print carry flag or not
   %<bitfield>i		print immediate for vstr/vldr reg +/- imm
   %<bitfield>h		print high half of 64-bit destination reg
   %<bitfield>k		print immediate for vector conversion instruction
   %<bitfield>l		print low half of 64-bit destination reg
   %<bitfield>o		print rotate value for vcmul
   %<bitfield>u		print immediate value for vddup/vdwdup
   %<bitfield>x		print the bitfield in hex.
  */

static const struct mopcode32 mve_opcodes[] =
{
  /* MVE.  */

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPST,
   0xfe310f4d, 0xffbf1fff,
   "vpst%i"
  },

  /* Floating point VPT T1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VPT_FP_T1,
   0xee310f00, 0xefb10f50,
   "vpt%i.f%28s\t%n, %17-19Q, %1-3,5Q"},
  /* Floating point VPT T2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VPT_FP_T2,
   0xee310f40, 0xefb10f50,
   "vpt%i.f%28s\t%n, %17-19Q, %0-3Z"},

  /* Vector VPT T1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPT_VEC_T1,
   0xfe010f00, 0xff811f51,
   "vpt%i.i%20-21s\t%n, %17-19Q, %1-3,5Q"},
  /* Vector VPT T2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPT_VEC_T2,
   0xfe010f01, 0xff811f51,
   "vpt%i.u%20-21s\t%n, %17-19Q, %1-3,5Q"},
  /* Vector VPT T3.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPT_VEC_T3,
   0xfe011f00, 0xff811f50,
   "vpt%i.s%20-21s\t%n, %17-19Q, %1-3,5Q"},
  /* Vector VPT T4.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPT_VEC_T4,
   0xfe010f40, 0xff811f70,
   "vpt%i.i%20-21s\t%n, %17-19Q, %0-3Z"},
  /* Vector VPT T5.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPT_VEC_T5,
   0xfe010f60, 0xff811f70,
   "vpt%i.u%20-21s\t%n, %17-19Q, %0-3Z"},
  /* Vector VPT T6.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPT_VEC_T6,
   0xfe011f40, 0xff811f50,
   "vpt%i.s%20-21s\t%n, %17-19Q, %0-3Z"},

  /* Vector VBIC immediate.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VBIC_IMM,
   0xef800070, 0xefb81070,
   "vbic%v.i%8-11s\t%13-15,22Q, %E"},

  /* Vector VBIC register.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VBIC_REG,
   0xef100150, 0xffb11f51,
   "vbic%v\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VABAV.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VABAV,
   0xee800f01, 0xefc10f51,
   "vabav%v.%u%20-21s\t%12-15r, %17-19,7Q, %1-3,5Q"},

  /* Vector VABD floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VABD_FP,
   0xff200d40, 0xffa11f51,
   "vabd%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VABD.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VABD_VEC,
   0xef000740, 0xef811f51,
   "vabd%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VABS floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VABS_FP,
   0xFFB10740, 0xFFB31FD1,
   "vabs%v.f%18-19s\t%13-15,22Q, %1-3,5Q"},
  /* Vector VABS.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VABS_VEC,
   0xffb10340, 0xffb31fd1,
   "vabs%v.s%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VADD floating point T1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VADD_FP_T1,
   0xef000d40, 0xffa11f51,
   "vadd%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},
  /* Vector VADD floating point T2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VADD_FP_T2,
   0xee300f40, 0xefb11f70,
   "vadd%v.f%28s\t%13-15,22Q, %17-19,7Q, %0-3r"},
  /* Vector VADD T1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VADD_VEC_T1,
   0xef000840, 0xff811f51,
   "vadd%v.i%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},
  /* Vector VADD T2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VADD_VEC_T2,
   0xee010f40, 0xff811f70,
   "vadd%v.i%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VADDLV.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VADDLV,
   0xee890f00, 0xef8f1fd1,
   "vaddlv%5A%v.%u32\t%13-15l, %20-22h, %1-3Q"},

  /* Vector VADDV.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VADDV,
   0xeef10f00, 0xeff31fd1,
   "vaddv%5A%v.%u%18-19s\t%13-15l, %1-3Q"},

  /* Vector VADC.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VADC,
   0xee300f00, 0xffb10f51,
   "vadc%12I%v.i32\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VAND.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VAND,
   0xef000150, 0xffb11f51,
   "vand%v\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VBRSR register.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VBRSR,
   0xfe011e60, 0xff811f70,
   "vbrsr%v.%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VCADD floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCADD_FP,
   0xfc800840, 0xfea11f51,
   "vcadd%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q, %{I:#%24o%}"},

  /* Vector VCADD.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCADD_VEC,
   0xfe000f00, 0xff810f51,
   "vcadd%v.i%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q, %{I:#%12o%}"},

  /* Vector VCLS.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCLS,
   0xffb00440, 0xffb31fd1,
   "vcls%v.s%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VCLZ.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCLZ,
   0xffb004c0, 0xffb31fd1,
   "vclz%v.i%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VCMLA.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCMLA_FP,
   0xfc200840, 0xfe211f51,
   "vcmla%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q, %{I:#%23-24o%}"},

  /* Vector VCMP floating point T1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCMP_FP_T1,
   0xee310f00, 0xeff1ef50,
   "vcmp%v.f%28s\t%n, %17-19Q, %1-3,5Q"},

  /* Vector VCMP floating point T2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCMP_FP_T2,
   0xee310f40, 0xeff1ef50,
   "vcmp%v.f%28s\t%n, %17-19Q, %0-3Z"},

  /* Vector VCMP T1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCMP_VEC_T1,
   0xfe010f00, 0xffc1ff51,
   "vcmp%v.i%20-21s\t%n, %17-19Q, %1-3,5Q"},
  /* Vector VCMP T2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCMP_VEC_T2,
   0xfe010f01, 0xffc1ff51,
   "vcmp%v.u%20-21s\t%n, %17-19Q, %1-3,5Q"},
  /* Vector VCMP T3.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCMP_VEC_T3,
   0xfe011f00, 0xffc1ff50,
   "vcmp%v.s%20-21s\t%n, %17-19Q, %1-3,5Q"},
  /* Vector VCMP T4.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCMP_VEC_T4,
   0xfe010f40, 0xffc1ff70,
   "vcmp%v.i%20-21s\t%n, %17-19Q, %0-3Z"},
  /* Vector VCMP T5.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCMP_VEC_T5,
   0xfe010f60, 0xffc1ff70,
   "vcmp%v.u%20-21s\t%n, %17-19Q, %0-3Z"},
  /* Vector VCMP T6.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCMP_VEC_T6,
   0xfe011f40, 0xffc1ff50,
   "vcmp%v.s%20-21s\t%n, %17-19Q, %0-3Z"},

  /* Vector VDUP.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VDUP,
   0xeea00b10, 0xffb10f5f,
   "vdup%v.%5,22s\t%17-19,7Q, %12-15r"},

  /* Vector VEOR.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VEOR,
   0xff000150, 0xffd11f51,
   "veor%v\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VFMA, vector * scalar.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VFMA_FP_SCALAR,
   0xee310e40, 0xefb11f70,
   "vfma%v.f%28s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VFMA floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VFMA_FP,
   0xef000c50, 0xffa11f51,
   "vfma%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VFMS floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VFMS_FP,
   0xef200c50, 0xffa11f51,
   "vfms%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VFMAS, vector * scalar.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VFMAS_FP_SCALAR,
   0xee311e40, 0xefb11f70,
   "vfmas%v.f%28s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VHADD T1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VHADD_T1,
   0xef000040, 0xef811f51,
   "vhadd%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VHADD T2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VHADD_T2,
   0xee000f40, 0xef811f70,
   "vhadd%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VHSUB T1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VHSUB_T1,
   0xef000240, 0xef811f51,
   "vhsub%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VHSUB T2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VHSUB_T2,
   0xee001f40, 0xef811f70,
   "vhsub%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VCMUL.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCMUL_FP,
   0xee300e00, 0xefb10f50,
   "vcmul%v.f%28s\t%13-15,22Q, %17-19,7Q, %1-3,5Q, %{I:#%0,12o%}"},

   /* Vector VCTP.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VCTP,
   0xf000e801, 0xffc0ffff,
   "vctp%v.%20-21s\t%16-19r"},

  /* Vector VDUP.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VDUP,
   0xeea00b10, 0xffb10f5f,
   "vdup%v.%5,22s\t%17-19,7Q, %12-15r"},

  /* Vector VRHADD.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRHADD,
   0xef000140, 0xef811f51,
   "vrhadd%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VCVT.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCVT_FP_FIX_VEC,
   0xef800c50, 0xef801cd1,
   "vcvt%v.%s\t%13-15,22Q, %1-3,5Q, %{I:#%16-21k%}"},

  /* Vector VCVT.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCVT_BETWEEN_FP_INT,
   0xffb30640, 0xffb31e51,
   "vcvt%v.%s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VCVT between single and half-precision float, bottom half.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCVT_FP_HALF_FP,
   0xee3f0e01, 0xefbf1fd1,
   "vcvtb%v.%s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VCVT between single and half-precision float, top half.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCVT_FP_HALF_FP,
   0xee3f1e01, 0xefbf1fd1,
   "vcvtt%v.%s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VCVT.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VCVT_FROM_FP_TO_INT,
   0xffb30040, 0xffb31c51,
   "vcvt%m%v.%s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VDDUP.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VDDUP,
   0xee011f6e, 0xff811f7e,
   "vddup%v.u%20-21s\t%13-15,22Q, %17-19l, %{I:#%0,7u%}"},

  /* Vector VDWDUP.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VDWDUP,
   0xee011f60, 0xff811f70,
   "vdwdup%v.u%20-21s\t%13-15,22Q, %17-19l, %1-3h, %{I:#%0,7u%}"},

  /* Vector VHCADD.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VHCADD,
   0xee000f00, 0xff810f51,
   "vhcadd%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q, %{I:#%12o%}"},

  /* Vector VIWDUP.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VIWDUP,
   0xee010f60, 0xff811f70,
   "viwdup%v.u%20-21s\t%13-15,22Q, %17-19l, %1-3h, %{I:#%0,7u%}"},

  /* Vector VIDUP.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VIDUP,
   0xee010f6e, 0xff811f7e,
   "vidup%v.u%20-21s\t%13-15,22Q, %17-19l, %{I:#%0,7u%}"},

  /* Vector VLD2.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLD2,
   0xfc901e00, 0xff901e5f,
   "vld2%5d.%7-8s\t%B, [%16-19r]%w"},

  /* Vector VLD4.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLD4,
   0xfc901e01, 0xff901e1f,
   "vld4%5-6d.%7-8s\t%B, [%16-19r]%w"},

  /* Vector VLDRB gather load.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRB_GATHER_T1,
   0xec900e00, 0xefb01e50,
   "vldrb%v.%u%7-8s\t%13-15,22Q, [%16-19r, %1-3,5Q]"},

  /* Vector VLDRH gather load.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRH_GATHER_T2,
   0xec900e10, 0xefb01e50,
   "vldrh%v.%u%7-8s\t%13-15,22Q, [%16-19r, %1-3,5Q%o]"},

  /* Vector VLDRW gather load.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRW_GATHER_T3,
   0xfc900f40, 0xffb01fd0,
   "vldrw%v.u32\t%13-15,22Q, [%16-19r, %1-3,5Q%o]"},

  /* Vector VLDRD gather load.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRD_GATHER_T4,
   0xec900fd0, 0xefb01fd0,
   "vldrd%v.u64\t%13-15,22Q, [%16-19r, %1-3,5Q%o]"},

  /* Vector VLDRW gather load.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRW_GATHER_T5,
   0xfd101e00, 0xff111f00,
   "vldrw%v.u32\t%13-15,22Q, [%17-19,7Q, %{I:#%a%0-6i%}]%w"},

  /* Vector VLDRD gather load, variant T6.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRD_GATHER_T6,
   0xfd101f00, 0xff111f00,
   "vldrd%v.u64\t%13-15,22Q, [%17-19,7Q, %{I:#%a%0-6i%}]%w"},

  /* Vector VLDRB.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRB_T1,
   0xec100e00, 0xee581e00,
   "vldrb%v.%u%7-8s\t%13-15Q, %d"},

  /* Vector VLDRH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRH_T2,
   0xec180e00, 0xee581e00,
   "vldrh%v.%u%7-8s\t%13-15Q, %d"},

  /* Vector VLDRB unsigned, variant T5.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRB_T5,
   0xec101e00, 0xfe101f80,
   "vldrb%v.u8\t%13-15,22Q, %d"},

  /* Vector VLDRH unsigned, variant T6.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRH_T6,
   0xec101e80, 0xfe101f80,
   "vldrh%v.u16\t%13-15,22Q, %d"},

  /* Vector VLDRW unsigned, variant T7.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VLDRW_T7,
   0xec101f00, 0xfe101f80,
   "vldrw%v.u32\t%13-15,22Q, %d"},

  /* Vector VMAX.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMAX,
   0xef000640, 0xef811f51,
   "vmax%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMAXA.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMAXA,
   0xee330e81, 0xffb31fd1,
   "vmaxa%v.s%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VMAXNM floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMAXNM_FP,
   0xff000f50, 0xffa11f51,
   "vmaxnm%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMAXNMA floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMAXNMA_FP,
   0xee3f0e81, 0xefbf1fd1,
   "vmaxnma%v.f%28s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VMAXNMV floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMAXNMV_FP,
   0xeeee0f00, 0xefff0fd1,
   "vmaxnmv%v.f%28s\t%12-15r, %1-3,5Q"},

  /* Vector VMAXNMAV floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMAXNMAV_FP,
   0xeeec0f00, 0xefff0fd1,
   "vmaxnmav%v.f%28s\t%12-15r, %1-3,5Q"},

  /* Vector VMAXV.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMAXV,
   0xeee20f00, 0xeff30fd1,
   "vmaxv%v.%u%18-19s\t%12-15r, %1-3,5Q"},

  /* Vector VMAXAV.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMAXAV,
   0xeee00f00, 0xfff30fd1,
   "vmaxav%v.s%18-19s\t%12-15r, %1-3,5Q"},

  /* Vector VMIN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMIN,
   0xef000650, 0xef811f51,
   "vmin%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMINA.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMINA,
   0xee331e81, 0xffb31fd1,
   "vmina%v.s%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VMINNM floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMINNM_FP,
   0xff200f50, 0xffa11f51,
   "vminnm%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMINNMA floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMINNMA_FP,
   0xee3f1e81, 0xefbf1fd1,
   "vminnma%v.f%28s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VMINNMV floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMINNMV_FP,
   0xeeee0f80, 0xefff0fd1,
   "vminnmv%v.f%28s\t%12-15r, %1-3,5Q"},

  /* Vector VMINNMAV floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMINNMAV_FP,
   0xeeec0f80, 0xefff0fd1,
   "vminnmav%v.f%28s\t%12-15r, %1-3,5Q"},

  /* Vector VMINV.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMINV,
   0xeee20f80, 0xeff30fd1,
   "vminv%v.%u%18-19s\t%12-15r, %1-3,5Q"},

  /* Vector VMINAV.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMINAV,
   0xeee00f80, 0xfff30fd1,
   "vminav%v.s%18-19s\t%12-15r, %1-3,5Q"},

  /* Vector VMLA.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLA,
   0xee010e40, 0xef811f70,
   "vmla%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VMLALDAV.  Note must appear before VMLADAV due to instruction
     opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLALDAV,
   0xee801e00, 0xef801f51,
   "vmlaldav%5Ax%v.%u%16s\t%13-15l, %20-22h, %17-19,7Q, %1-3Q"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLALDAV,
   0xee800e00, 0xef801f51,
   "vmlalv%5A%v.%u%16s\t%13-15l, %20-22h, %17-19,7Q, %1-3Q"},

  /* Vector VMLAV T1 variant, same as VMLADAV but with X == 0.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLADAV_T1,
   0xeef00e00, 0xeff01f51,
   "vmlav%5A%v.%u%16s\t%13-15l, %17-19,7Q, %1-3Q"},

  /* Vector VMLAV T2 variant, same as VMLADAV but with X == 0.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLADAV_T2,
   0xeef00f00, 0xeff11f51,
   "vmlav%5A%v.%u8\t%13-15l, %17-19,7Q, %1-3Q"},

  /* Vector VMLADAV T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLADAV_T1,
   0xeef01e00, 0xeff01f51,
   "vmladav%5Ax%v.%u%16s\t%13-15l, %17-19,7Q, %1-3Q"},

  /* Vector VMLADAV T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLADAV_T2,
   0xeef01f00, 0xeff11f51,
   "vmladav%5Ax%v.%u8\t%13-15l, %17-19,7Q, %1-3Q"},

  /* Vector VMLAS.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLAS,
   0xee011e40, 0xef811f70,
   "vmlas%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VRMLSLDAVH.  Note must appear before VMLSDAV due to instruction
     opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRMLSLDAVH,
   0xfe800e01, 0xff810f51,
   "vrmlsldavh%5A%X%v.s32\t%13-15l, %20-22h, %17-19,7Q, %1-3Q"},

  /* Vector VMLSLDAV.  Note must appear before VMLSDAV due to instruction
     opcdoe aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLSLDAV,
   0xee800e01, 0xff800f51,
   "vmlsldav%5A%X%v.%u%16s\t%13-15l, %20-22h, %17-19,7Q, %1-3Q"},

  /* Vector VMLSDAV T1 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLSDAV_T1,
   0xeef00e01, 0xfff00f51,
   "vmlsdav%5A%X%v.s%16s\t%13-15l, %17-19,7Q, %1-3Q"},

  /* Vector VMLSDAV T2 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMLSDAV_T2,
   0xfef00e01, 0xfff10f51,
   "vmlsdav%5A%X%v.s8\t%13-15l, %17-19,7Q, %1-3Q"},

  /* Vector VMOV between gpr and half precision register, op == 0.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMOV_HFP_TO_GP,
   0xee000910, 0xfff00f7f,
   "vmov.f16\t%7,16-19F, %12-15r"},

  /* Vector VMOV between gpr and half precision register, op == 1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMOV_HFP_TO_GP,
   0xee100910, 0xfff00f7f,
   "vmov.f16\t%12-15r, %7,16-19F"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMOV_GP_TO_VEC_LANE,
   0xee000b10, 0xff900f1f,
   "vmov%c.%5-6,21-22s\t%{R:%17-19,7Q[%N]%}, %12-15r"},

  /* Vector VORR immediate to vector.
     NOTE: MVE_VORR_IMM must appear in the table
     before MVE_VMOV_IMM_TO_VEC due to opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VORR_IMM,
   0xef800050, 0xefb810f0,
   "vorr%v.i%8-11s\t%13-15,22Q, %E"},

  /* Vector VQSHL T2 Variant.
     NOTE: MVE_VQSHL_T2 must appear in the table before
     before MVE_VMOV_IMM_TO_VEC due to opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQSHL_T2,
   0xef800750, 0xef801fd1,
   "vqshl%v.%u%19-21s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VQSHLU T3 Variant
     NOTE: MVE_VQSHL_T2 must appear in the table before
     before MVE_VMOV_IMM_TO_VEC due to opcode aliasing.  */

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQSHLU_T3,
   0xff800650, 0xff801fd1,
   "vqshlu%v.s%19-21s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VRSHR
     NOTE: MVE_VRSHR must appear in the table before
     before MVE_VMOV_IMM_TO_VEC due to opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRSHR,
   0xef800250, 0xef801fd1,
   "vrshr%v.%u%19-21s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VSHL.
     NOTE: MVE_VSHL must appear in the table before
     before MVE_VMOV_IMM_TO_VEC due to opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSHL_T1,
   0xef800550, 0xff801fd1,
   "vshl%v.i%19-21s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VSHR
     NOTE: MVE_VSHR must appear in the table before
     before MVE_VMOV_IMM_TO_VEC due to opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSHR,
   0xef800050, 0xef801fd1,
   "vshr%v.%u%19-21s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VSLI
     NOTE: MVE_VSLI must appear in the table before
     before MVE_VMOV_IMM_TO_VEC due to opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSLI,
   0xff800550, 0xff801fd1,
   "vsli%v.%19-21s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VSRI
     NOTE: MVE_VSRI must appear in the table before
     before MVE_VMOV_IMM_TO_VEC due to opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSRI,
   0xff800450, 0xff801fd1,
   "vsri%v.%19-21s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VMOV immediate to vector,
     undefinded for cmode == 1111 */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMVN_IMM, 0xef800f70, 0xefb81ff0, UNDEFINED_INSTRUCTION},

  /* Vector VMOV immediate to vector,
     cmode == 1101 */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOV_IMM_TO_VEC, 0xef800d50, 0xefb81fd0,
   "vmov%v.%5,8-11s\t%13-15,22Q, %E"},

  /* Vector VMOV immediate to vector.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOV_IMM_TO_VEC,
   0xef800050, 0xefb810d0,
   "vmov%v.%5,8-11s\t%13-15,22Q, %E"},

  /* Vector VMOV two 32-bit lanes to two gprs, idx = 0.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOV2_VEC_LANE_TO_GP,
   0xec000f00, 0xffb01ff0,
   "vmov%c\t%0-3r, %16-19r, %{R:%13-15,22Q[2]%}, %{R:%13-15,22Q[0]%}"},

  /* Vector VMOV two 32-bit lanes to two gprs, idx = 1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOV2_VEC_LANE_TO_GP,
   0xec000f10, 0xffb01ff0,
   "vmov%c\t%0-3r, %16-19r, %{R:%13-15,22Q[3]%}, %{R:%13-15,22Q[1]%}"},

  /* Vector VMOV Two gprs to two 32-bit lanes, idx = 0.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOV2_GP_TO_VEC_LANE,
   0xec100f00, 0xffb01ff0,
   "vmov%c\t%{R:%13-15,22Q[2]%}, %{R:%13-15,22Q[0]%}, %0-3r, %16-19r"},

  /* Vector VMOV Two gprs to two 32-bit lanes, idx = 1.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOV2_GP_TO_VEC_LANE,
   0xec100f10, 0xffb01ff0,
   "vmov%c\t%{R:%13-15,22Q[3]%}, %{R:%13-15,22Q[1]%}, %0-3r, %16-19r"},

  /* Vector VMOV Vector lane to gpr.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMOV_VEC_LANE_TO_GP,
   0xee100b10, 0xff100f1f,
   "vmov%c.%u%5-6,21-22s\t%12-15r, %{R:%17-19,7Q[%N]%}"},

  /* Vector VSHLL T1 Variant.  Note: VSHLL T1 must appear before MVE_VMOVL due
     to instruction opcode aliasing.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSHLL_T1,
   0xeea00f40, 0xefa00fd1,
   "vshll%T%v.%u%19-20s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VMOVL long.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOVL,
   0xeea00f40, 0xefa70fd1,
   "vmovl%T%v.%u%19-20s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VMOV and narrow.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOVN,
   0xfe310e81, 0xffb30fd1,
   "vmovn%T%v.i%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Floating point move extract.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMOVX,
   0xfeb00a40, 0xffbf0fd0,
   "vmovx.f16\t%22,12-15F, %5,0-3F"},

  /* Vector VMUL floating-point T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMUL_FP_T1,
   0xff000d50, 0xffa11f51,
   "vmul%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMUL floating-point T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VMUL_FP_T2,
   0xee310e60, 0xefb11f70,
   "vmul%v.f%28s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VMUL T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMUL_VEC_T1,
   0xef000950, 0xff811f51,
   "vmul%v.i%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMUL T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMUL_VEC_T2,
   0xee011e60, 0xff811f70,
   "vmul%v.i%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VMULH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMULH,
   0xee010e01, 0xef811f51,
   "vmulh%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VRMULH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRMULH,
   0xee011e01, 0xef811f51,
   "vrmulh%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMULL integer.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMULL_INT,
   0xee010e00, 0xef810f51,
   "vmull%T%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMULL polynomial.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMULL_POLY,
   0xee310e00, 0xefb10f51,
   "vmull%T%v.%28s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMVN immediate to vector.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMVN_IMM,
   0xef800070, 0xefb810f0,
   "vmvn%v.i%8-11s\t%13-15,22Q, %E"},

  /* Vector VMVN register.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMVN_REG,
   0xffb005c0, 0xffbf1fd1,
   "vmvn%v\t%13-15,22Q, %1-3,5Q"},

  /* Vector VNEG floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VNEG_FP,
   0xffb107c0, 0xffb31fd1,
   "vneg%v.f%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VNEG.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VNEG_VEC,
   0xffb103c0, 0xffb31fd1,
   "vneg%v.s%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VORN, vector bitwise or not.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VORN,
   0xef300150, 0xffb11f51,
   "vorn%v\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VORR register.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VORR_REG,
   0xef200150, 0xffb11f51,
   "vorr%v\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VMOV, vector to vector move. While decoding MVE_VORR_REG if
     "Qm==Qn", VORR should replaced by its alias VMOV. For that to happen
     MVE_VMOV_VEC_TO_VEC need to placed after MVE_VORR_REG in this mve_opcodes
     array.  */

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VMOV_VEC_TO_VEC,
   0xef200150, 0xffb11f51,
   "vmov%v\t%13-15,22Q, %17-19,7Q"},

  /* Vector VQDMULL T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQDMULL_T1,
   0xee300f01, 0xefb10f51,
   "vqdmull%T%v.s%28s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VPNOT.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPNOT,
   0xfe310f4d, 0xffffffff,
   "vpnot%v"},

  /* Vector VPSEL.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VPSEL,
   0xfe310f01, 0xffb11f51,
   "vpsel%v\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQABS.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQABS,
   0xffb00740, 0xffb31fd1,
   "vqabs%v.s%18-19s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQADD T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQADD_T1,
   0xef000050, 0xef811f51,
   "vqadd%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQADD T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQADD_T2,
   0xee000f60, 0xef811f70,
   "vqadd%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VQDMULL T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQDMULL_T2,
   0xee300f60, 0xefb10f70,
   "vqdmull%T%v.s%28s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VQMOVN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQMOVN,
   0xee330e01, 0xefb30fd1,
   "vqmovn%T%v.%u%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VQMOVUN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQMOVUN,
   0xee310e81, 0xffb30fd1,
   "vqmovun%T%v.s%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VQDMLADH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQDMLADH,
   0xee000e00, 0xff810f51,
   "vqdmladh%X%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQRDMLADH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRDMLADH,
   0xee000e01, 0xff810f51,
   "vqrdmladh%X%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQDMLAH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQDMLAH,
   0xee000e60, 0xff811f70,
   "vqdmlah%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VQRDMLAH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRDMLAH,
   0xee000e40, 0xff811f70,
   "vqrdmlah%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VQDMLASH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQDMLASH,
   0xee001e60, 0xff811f70,
   "vqdmlash%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VQRDMLASH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRDMLASH,
   0xee001e40, 0xff811f70,
   "vqrdmlash%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VQDMLSDH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQDMLSDH,
   0xfe000e00, 0xff810f51,
   "vqdmlsdh%X%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQRDMLSDH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRDMLSDH,
   0xfe000e01, 0xff810f51,
   "vqrdmlsdh%X%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQDMULH T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQDMULH_T1,
   0xef000b40, 0xff811f51,
   "vqdmulh%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQRDMULH T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRDMULH_T2,
   0xff000b40, 0xff811f51,
   "vqrdmulh%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQDMULH T3 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQDMULH_T3,
   0xee010e60, 0xff811f70,
   "vqdmulh%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VQRDMULH T4 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRDMULH_T4,
   0xfe010e60, 0xff811f70,
   "vqrdmulh%v.s%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VQNEG.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQNEG,
   0xffb007c0, 0xffb31fd1,
   "vqneg%v.s%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VQRSHL T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRSHL_T1,
   0xef000550, 0xef811f51,
   "vqrshl%v.%u%20-21s\t%13-15,22Q, %1-3,5Q, %17-19,7Q"},

  /* Vector VQRSHL T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRSHL_T2,
   0xee331ee0, 0xefb31ff0,
   "vqrshl%v.%u%18-19s\t%13-15,22Q, %0-3r"},

  /* Vector VQRSHRN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRSHRN,
   0xee800f41, 0xefa00fd1,
   "vqrshrn%T%v.%u%19-20s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VQRSHRUN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQRSHRUN,
   0xfe800fc0, 0xffa00fd1,
   "vqrshrun%T%v.s%19-20s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VQSHL T1 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQSHL_T1,
   0xee311ee0, 0xefb31ff0,
   "vqshl%v.%u%18-19s\t%13-15,22Q, %0-3r"},

  /* Vector VQSHL T4 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQSHL_T4,
   0xef000450, 0xef811f51,
   "vqshl%v.%u%20-21s\t%13-15,22Q, %1-3,5Q, %17-19,7Q"},

  /* Vector VQSHRN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQSHRN,
   0xee800f40, 0xefa00fd1,
   "vqshrn%T%v.%u%19-20s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VQSHRUN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQSHRUN,
   0xee800fc0, 0xffa00fd1,
   "vqshrun%T%v.s%19-20s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VQSUB T1 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQSUB_T1,
   0xef000250, 0xef811f51,
   "vqsub%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VQSUB T2 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VQSUB_T2,
   0xee001f60, 0xef811f70,
   "vqsub%v.%u%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VREV16.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VREV16,
   0xffb00140, 0xffb31fd1,
   "vrev16%v.8\t%13-15,22Q, %1-3,5Q"},

  /* Vector VREV32.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VREV32,
   0xffb000c0, 0xffb31fd1,
   "vrev32%v.%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VREV64.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VREV64,
   0xffb00040, 0xffb31fd1,
   "vrev64%v.%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VRINT floating point.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VRINT_FP,
   0xffb20440, 0xffb31c51,
   "vrint%m%v.f%18-19s\t%13-15,22Q, %1-3,5Q"},

  /* Vector VRMLALDAVH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRMLALDAVH,
   0xee800f00, 0xef811f51,
   "vrmlalvh%5A%v.%u32\t%13-15l, %20-22h, %17-19,7Q, %1-3Q"},

  /* Vector VRMLALDAVH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRMLALDAVH,
   0xee801f00, 0xef811f51,
   "vrmlaldavh%5Ax%v.%u32\t%13-15l, %20-22h, %17-19,7Q, %1-3Q"},

  /* Vector VRSHL T1 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRSHL_T1,
   0xef000540, 0xef811f51,
   "vrshl%v.%u%20-21s\t%13-15,22Q, %1-3,5Q, %17-19,7Q"},

  /* Vector VRSHL T2 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRSHL_T2,
   0xee331e60, 0xefb31ff0,
   "vrshl%v.%u%18-19s\t%13-15,22Q, %0-3r"},

  /* Vector VRSHRN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VRSHRN,
   0xfe800fc1, 0xffa00fd1,
   "vrshrn%T%v.i%19-20s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VSBC.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSBC,
   0xfe300f00, 0xffb10f51,
   "vsbc%12I%v.i32\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VSHL T2 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSHL_T2,
   0xee311e60, 0xefb31ff0,
   "vshl%v.%u%18-19s\t%13-15,22Q, %0-3r"},

  /* Vector VSHL T3 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSHL_T3,
   0xef000440, 0xef811f51,
   "vshl%v.%u%20-21s\t%13-15,22Q, %1-3,5Q, %17-19,7Q"},

  /* Vector VSHLC.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSHLC,
   0xeea00fc0, 0xffa01ff0,
   "vshlc%v\t%13-15,22Q, %0-3r, %{I:#%16-20d%}"},

  /* Vector VSHLL T2 Variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSHLL_T2,
   0xee310e01, 0xefb30fd1,
   "vshll%T%v.%u%18-19s\t%13-15,22Q, %1-3,5Q, %{I:#%18-19d%}"},

  /* Vector VSHRN.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSHRN,
   0xee800fc1, 0xffa00fd1,
   "vshrn%T%v.i%19-20s\t%13-15,22Q, %1-3,5Q, %{I:#%16-18d%}"},

  /* Vector VST2 no writeback.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VST2,
   0xfc801e00, 0xffb01e5f,
   "vst2%5d.%7-8s\t%B, [%16-19r]"},

  /* Vector VST2 writeback.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VST2,
   0xfca01e00, 0xffb01e5f,
   "vst2%5d.%7-8s\t%B, [%16-19r]!"},

  /* Vector VST4 no writeback.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VST4,
   0xfc801e01, 0xffb01e1f,
   "vst4%5-6d.%7-8s\t%B, [%16-19r]"},

  /* Vector VST4 writeback.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VST4,
   0xfca01e01, 0xffb01e1f,
   "vst4%5-6d.%7-8s\t%B, [%16-19r]!"},

  /* Vector VSTRB scatter store, T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRB_SCATTER_T1,
   0xec800e00, 0xffb01e50,
   "vstrb%v.%7-8s\t%13-15,22Q, [%16-19r, %1-3,5Q]"},

  /* Vector VSTRH scatter store, T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRH_SCATTER_T2,
   0xec800e10, 0xffb01e50,
   "vstrh%v.%7-8s\t%13-15,22Q, [%16-19r, %1-3,5Q%o]"},

  /* Vector VSTRW scatter store, T3 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRW_SCATTER_T3,
   0xec800e40, 0xffb01e50,
   "vstrw%v.%7-8s\t%13-15,22Q, [%16-19r, %1-3,5Q%o]"},

  /* Vector VSTRD scatter store, T4 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRD_SCATTER_T4,
   0xec800fd0, 0xffb01fd0,
   "vstrd%v.64\t%13-15,22Q, [%16-19r, %1-3,5Q%o]"},

  /* Vector VSTRW scatter store, T5 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRW_SCATTER_T5,
   0xfd001e00, 0xff111f00,
   "vstrw%v.32\t%13-15,22Q, [%17-19,7Q, %{I:#%a%0-6i%}]%w"},

  /* Vector VSTRD scatter store, T6 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRD_SCATTER_T6,
   0xfd001f00, 0xff111f00,
   "vstrd%v.64\t%13-15,22Q, [%17-19,7Q, %{I:#%a%0-6i%}]%w"},

  /* Vector VSTRB.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRB_T1,
   0xec000e00, 0xfe581e00,
   "vstrb%v.%7-8s\t%13-15Q, %d"},

  /* Vector VSTRH.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRH_T2,
   0xec080e00, 0xfe581e00,
   "vstrh%v.%7-8s\t%13-15Q, %d"},

  /* Vector VSTRB variant T5.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRB_T5,
   0xec001e00, 0xfe101f80,
   "vstrb%v.8\t%13-15,22Q, %d"},

  /* Vector VSTRH variant T6.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRH_T6,
   0xec001e80, 0xfe101f80,
   "vstrh%v.16\t%13-15,22Q, %d"},

  /* Vector VSTRW variant T7.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSTRW_T7,
   0xec001f00, 0xfe101f80,
   "vstrw%v.32\t%13-15,22Q, %d"},

  /* Vector VSUB floating point T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VSUB_FP_T1,
   0xef200d40, 0xffa11f51,
   "vsub%v.f%20s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VSUB floating point T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP),
   MVE_VSUB_FP_T2,
   0xee301f40, 0xefb11f70,
   "vsub%v.f%28s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  /* Vector VSUB T1 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSUB_VEC_T1,
   0xff000840, 0xff811f51,
   "vsub%v.i%20-21s\t%13-15,22Q, %17-19,7Q, %1-3,5Q"},

  /* Vector VSUB T2 variant.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_VSUB_VEC_T2,
   0xee011f40, 0xff811f70,
   "vsub%v.i%20-21s\t%13-15,22Q, %17-19,7Q, %0-3r"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_ASRLI,
   0xea50012f, 0xfff1813f,
   "asrl%c\t%17-19l, %9-11h, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_ASRL,
   0xea50012d, 0xfff101ff,
   "asrl%c\t%17-19l, %9-11h, %12-15S"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_LSLLI,
   0xea50010f, 0xfff1813f,
   "lsll%c\t%17-19l, %9-11h, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_LSLL,
   0xea50010d, 0xfff101ff,
   "lsll%c\t%17-19l, %9-11h, %12-15S"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_LSRL,
   0xea50011f, 0xfff1813f,
   "lsrl%c\t%17-19l, %9-11h, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_SQRSHRL,
   0xea51012d, 0xfff1017f,
   "sqrshrl%c\t%17-19l, %9-11h, %k, %12-15S"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_SQRSHR,
   0xea500f2d, 0xfff00fff,
   "sqrshr%c\t%16-19S, %12-15S"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_SQSHLL,
   0xea51013f, 0xfff1813f,
   "sqshll%c\t%17-19l, %9-11h, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_SQSHL,
   0xea500f3f, 0xfff08f3f,
   "sqshl%c\t%16-19S, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_SRSHRL,
   0xea51012f, 0xfff1813f,
   "srshrl%c\t%17-19l, %9-11h, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_SRSHR,
   0xea500f2f, 0xfff08f3f,
   "srshr%c\t%16-19S, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_UQRSHLL,
   0xea51010d, 0xfff1017f,
   "uqrshll%c\t%17-19l, %9-11h, %k, %12-15S"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_UQRSHL,
   0xea500f0d, 0xfff00fff,
   "uqrshl%c\t%16-19S, %12-15S"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_UQSHLL,
    0xea51010f, 0xfff1813f,
   "uqshll%c\t%17-19l, %9-11h, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_UQSHL,
   0xea500f0f, 0xfff08f3f,
   "uqshl%c\t%16-19S, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_URSHRL,
    0xea51011f, 0xfff1813f,
   "urshrl%c\t%17-19l, %9-11h, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE),
   MVE_URSHR,
   0xea500f1f, 0xfff08f3f,
   "urshr%c\t%16-19S, %j"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CSINC,
   0xea509000, 0xfff0f000,
   "csinc\t%8-11S, %16-19Z, %0-3Z, %4-7c"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CSINV,
   0xea50a000, 0xfff0f000,
   "csinv\t%8-11S, %16-19Z, %0-3Z, %4-7c"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CSET,
   0xea5f900f, 0xfffff00f,
   "cset\t%8-11S, %4-7C"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CSETM,
   0xea5fa00f, 0xfffff00f,
   "csetm\t%8-11S, %4-7C"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CSEL,
   0xea508000, 0xfff0f000,
   "csel\t%8-11S, %16-19Z, %0-3Z, %4-7c"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CSNEG,
   0xea50b000, 0xfff0f000,
   "csneg\t%8-11S, %16-19Z, %0-3Z, %4-7c"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CINC,
   0xea509000, 0xfff0f000,
   "cinc\t%8-11S, %16-19Z, %4-7C"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CINV,
   0xea50a000, 0xfff0f000,
   "cinv\t%8-11S, %16-19Z, %4-7C"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   MVE_CNEG,
   0xea50b000, 0xfff0f000,
   "cneg\t%8-11S, %16-19Z, %4-7C"},

  {ARM_FEATURE_CORE_LOW (0),
   MVE_NONE,
   0x00000000, 0x00000000, 0}
};

/* Opcode tables: ARM, 16-bit Thumb, 32-bit Thumb.  All three are partially
   ordered: they must be searched linearly from the top to obtain a correct
   match.  */

/* print_insn_arm recognizes the following format control codes:

   %%			%

   %a			print address for ldr/str instruction
   %s                   print address for ldr/str halfword/signextend instruction
   %S                   like %s but allow UNPREDICTABLE addressing
   %b			print branch destination
   %c			print condition code (always bits 28-31)
   %m			print register mask for ldm/stm instruction
   %o			print operand2 (immediate or register + shift)
   %p			print 'p' iff bits 12-15 are 15
   %t			print 't' iff bit 21 set and bit 24 clear
   %B			print arm BLX(1) destination
   %C			print the PSR sub type.
   %U			print barrier type.
   %P			print address for pli instruction.

   %<bitfield>r		print as an ARM register
   %<bitfield>T		print as an ARM register + 1
   %<bitfield>R		as %r but r15 is UNPREDICTABLE
   %<bitfield>{r|R}u    as %{r|R} but if matches the other %u field then is UNPREDICTABLE
   %<bitfield>{r|R}U    as %{r|R} but if matches the other %U field then is UNPREDICTABLE
   %<bitfield>d		print the bitfield in decimal
   %<bitfield>W         print the bitfield plus one in decimal
   %<bitfield>x		print the bitfield in hex
   %<bitfield>X		print the bitfield as 1 hex digit without leading "0x"

   %<bitfield>'c	print specified char iff bitfield is all ones
   %<bitfield>`c	print specified char iff bitfield is all zeroes
   %<bitfield>?ab...    select from array of values in big endian order

   %e                   print arm SMI operand (bits 0..7,8..19).
   %E			print the LSB and WIDTH fields of a BFI or BFC instruction.
   %V                   print the 16-bit immediate field of a MOVT or MOVW instruction.
   %R			print the SPSR/CPSR or banked register of an MRS.  */

static const struct opcode32 arm_opcodes[] =
{
  /* ARM instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0xe1a00000, 0xffffffff, "nop\t\t\t@ (mov r0, r0)"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0xe7f000f0, 0xfff000f0, "udf\t%{I:#%e%}"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T | ARM_EXT_V5),
    0x012FFF10, 0x0ffffff0, "bx%c\t%0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V2),
    0x00000090, 0x0fe000f0, "mul%20's%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V2),
    0x00200090, 0x0fe000f0, "mla%20's%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V2S),
    0x01000090, 0x0fb00ff0, "swp%22'b%c\t%12-15RU, %0-3Ru, [%16-19RuU]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V3M),
    0x00800090, 0x0fa000f0,
    "%22?sumull%20's%c\t%12-15Ru, %16-19Ru, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V3M),
    0x00a00090, 0x0fa000f0,
    "%22?sumlal%20's%c\t%12-15Ru, %16-19Ru, %0-3R, %8-11R"},

  /* V8.2 RAS extension instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_RAS),
    0xe320f010, 0xffffffff, "esb"},

  /* V8-R instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8R),
    0xf57ff04c, 0xffffffff, "dfb"},

  /* V8 instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0x0320f005, 0x0fffffff, "sevl"},
  /* Defined in V8 but is in NOP space so available to all arch.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0xe1000070, 0xfff000f0, "hlt\t%{I:0x%16-19X%12-15X%8-11X%0-3X%}"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_ATOMICS),
    0x01800e90, 0x0ff00ff0, "stlex%c\t%12-15r, %0-3r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01900e9f, 0x0ff00fff, "ldaex%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0x01a00e90, 0x0ff00ff0, "stlexd%c\t%12-15r, %0-3r, %0-3T, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0x01b00e9f, 0x0ff00fff, "ldaexd%c\t%12-15r, %12-15T, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01c00e90, 0x0ff00ff0, "stlexb%c\t%12-15r, %0-3r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01d00e9f, 0x0ff00fff, "ldaexb%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01e00e90, 0x0ff00ff0, "stlexh%c\t%12-15r, %0-3r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01f00e9f, 0x0ff00fff, "ldaexh%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x0180fc90, 0x0ff0fff0, "stl%c\t%0-3r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01900c9f, 0x0ff00fff, "lda%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01c0fc90, 0x0ff0fff0, "stlb%c\t%0-3r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01d00c9f, 0x0ff00fff, "ldab%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01e0fc90, 0x0ff0fff0, "stlh%c\t%0-3r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT2_ATOMICS),
    0x01f00c9f, 0x0ff00fff, "ldah%c\t%12-15r, [%16-19R]"},
  /* CRC32 instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xe1000040, 0xfff00ff0, "crc32b\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xe1200040, 0xfff00ff0, "crc32h\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xe1400040, 0xfff00ff0, "crc32w\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xe1000240, 0xfff00ff0, "crc32cb\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xe1200240, 0xfff00ff0, "crc32ch\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xe1400240, 0xfff00ff0, "crc32cw\t%12-15R, %16-19R, %0-3R"},

  /* Privileged Access Never extension instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_PAN),
    0xf1100000, 0xfffffdff, "setpan\t%{I:#%9-9d%}"},

  /* Virtualization Extension instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_VIRT), 0x0160006e, 0x0fffffff, "eret%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_VIRT), 0x01400070, 0x0ff000f0, "hvc%c\t%e"},

  /* Integer Divide Extension instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV),
    0x0710f010, 0x0ff0f0f0, "sdiv%c\t%16-19r, %0-3r, %8-11r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV),
    0x0730f010, 0x0ff0f0f0, "udiv%c\t%16-19r, %0-3r, %8-11r"},

  /* MP Extension instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_MP), 0xf410f000, 0xfc70f000, "pldw\t%a"},

  /* Speculation Barriers.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V3), 0xe320f014, 0xffffffff, "csdb"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V3), 0xf57ff040, 0xffffffff, "ssbb"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V3), 0xf57ff044, 0xffffffff, "pssbb"},

  /* V7 instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf450f000, 0xfd70f000, "pli\t%P"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0x0320f0f0, 0x0ffffff0, "dbg%c\t%{I:#%0-3d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8), 0xf57ff051, 0xfffffff3, "dmb\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8), 0xf57ff041, 0xfffffff3, "dsb\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf57ff050, 0xfffffff0, "dmb\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf57ff040, 0xfffffff0, "dsb\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf57ff060, 0xfffffff0, "isb\t%U"},
   {ARM_FEATURE_CORE_LOW (ARM_EXT_V7),
    0x0320f000, 0x0fffffff, "nop%c\t{%{I:%0-7d%}}"},

  /* ARM V6T2 instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0x07c0001f, 0x0fe0007f, "bfc%c\t%12-15R, %E"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0x07c00010, 0x0fe00070, "bfi%c\t%12-15R, %0-3r, %E"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0x00600090, 0x0ff000f0, "mls%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0x002000b0, 0x0f3000f0, "strht%c\t%12-15R, %S"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0x00300090, 0x0f3000f0, UNDEFINED_INSTRUCTION },
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0x00300090, 0x0f300090, "ldr%6's%5?hbt%c\t%12-15R, %S"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0x03000000, 0x0ff00000, "movw%c\t%12-15R, %V"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0x03400000, 0x0ff00000, "movt%c\t%12-15R, %V"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0x06ff0f30, 0x0fff0ff0, "rbit%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0x07a00050, 0x0fa00070, "%22?usbfx%c\t%12-15r, %0-3r, %{I:#%7-11d%}, %{I:#%16-20W%}"},

  /* ARM Security extension instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_SEC),
    0x01600070, 0x0ff000f0, "smc%c\t%e"},

  /* ARM V6K instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0xf57ff01f, 0xffffffff, "clrex"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x01d00f9f, 0x0ff00fff, "ldrexb%c\t%12-15R, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x01b00f9f, 0x0ff00fff, "ldrexd%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x01f00f9f, 0x0ff00fff, "ldrexh%c\t%12-15R, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x01c00f90, 0x0ff00ff0, "strexb%c\t%12-15R, %0-3R, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x01a00f90, 0x0ff00ff0, "strexd%c\t%12-15R, %0-3r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x01e00f90, 0x0ff00ff0, "strexh%c\t%12-15R, %0-3R, [%16-19R]"},

  /* ARMv8.5-A instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB), 0xf57ff070, 0xffffffff, "sb"},

  /* ARM V6K NOP hints.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x0320f001, 0x0fffffff, "yield%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x0320f002, 0x0fffffff, "wfe%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x0320f003, 0x0fffffff, "wfi%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x0320f004, 0x0fffffff, "sev%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
    0x0320f000, 0x0fffff00, "nop%c\t{%{I:%0-7d%}}"},

  /* ARM V6 instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xf1080000, 0xfffffe3f, "cpsie\t%{B:%8'a%7'i%6'f%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xf10a0000, 0xfffffe20, "cpsie\t%{B:%8'a%7'i%6'f%}, %{I:#%0-4d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xf10C0000, 0xfffffe3f, "cpsid\t%{B:%8'a%7'i%6'f%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xf10e0000, 0xfffffe20, "cpsid\t%{B:%8'a%7'i%6'f%}, %{I:#%0-4d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xf1000000, 0xfff1fe20, "cps\t%{I:#%0-4d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800010, 0x0ff00ff0, "pkhbt%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800010, 0x0ff00070, "pkhbt%c\t%12-15R, %16-19R, %0-3R, %{B:lsl%} %{I:#%7-11d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800050, 0x0ff00ff0, "pkhtb%c\t%12-15R, %16-19R, %0-3R, %{B:asr%} %{I:#32%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800050, 0x0ff00070, "pkhtb%c\t%12-15R, %16-19R, %0-3R, %{B:asr%} %{I:#%7-11d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x01900f9f, 0x0ff00fff, "ldrex%c\t%{R:r%12-15d%}, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06200f10, 0x0ff00ff0, "qadd16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06200f90, 0x0ff00ff0, "qadd8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06200f30, 0x0ff00ff0, "qasx%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06200f70, 0x0ff00ff0, "qsub16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06200ff0, 0x0ff00ff0, "qsub8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06200f50, 0x0ff00ff0, "qsax%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06100f10, 0x0ff00ff0, "sadd16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06100f90, 0x0ff00ff0, "sadd8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06100f30, 0x0ff00ff0, "sasx%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06300f10, 0x0ff00ff0, "shadd16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06300f90, 0x0ff00ff0, "shadd8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06300f30, 0x0ff00ff0, "shasx%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06300f70, 0x0ff00ff0, "shsub16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06300ff0, 0x0ff00ff0, "shsub8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06300f50, 0x0ff00ff0, "shsax%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06100f70, 0x0ff00ff0, "ssub16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06100ff0, 0x0ff00ff0, "ssub8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06100f50, 0x0ff00ff0, "ssax%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06500f10, 0x0ff00ff0, "uadd16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06500f90, 0x0ff00ff0, "uadd8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06500f30, 0x0ff00ff0, "uasx%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06700f10, 0x0ff00ff0, "uhadd16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06700f90, 0x0ff00ff0, "uhadd8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06700f30, 0x0ff00ff0, "uhasx%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06700f70, 0x0ff00ff0, "uhsub16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06700ff0, 0x0ff00ff0, "uhsub8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06700f50, 0x0ff00ff0, "uhsax%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06600f10, 0x0ff00ff0, "uqadd16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06600f90, 0x0ff00ff0, "uqadd8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06600f30, 0x0ff00ff0, "uqasx%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06600f70, 0x0ff00ff0, "uqsub16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06600ff0, 0x0ff00ff0, "uqsub8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06600f50, 0x0ff00ff0, "uqsax%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06500f70, 0x0ff00ff0, "usub16%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06500ff0, 0x0ff00ff0, "usub8%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06500f50, 0x0ff00ff0, "usax%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06bf0f30, 0x0fff0ff0, "rev%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06bf0fb0, 0x0fff0ff0, "rev16%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ff0fb0, 0x0fff0ff0, "revsh%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xf8100a00, 0xfe50ffff, "rfe%23?id%24?ba\t%16-19r%21'!"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06bf0070, 0x0fff0ff0, "sxth%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06bf0470, 0x0fff0ff0, "sxth%c\t%12-15R, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06bf0870, 0x0fff0ff0, "sxth%c\t%12-15R, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06bf0c70, 0x0fff0ff0, "sxth%c\t%12-15R, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x068f0070, 0x0fff0ff0, "sxtb16%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x068f0470, 0x0fff0ff0, "sxtb16%c\t%12-15R, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x068f0870, 0x0fff0ff0, "sxtb16%c\t%12-15R, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x068f0c70, 0x0fff0ff0, "sxtb16%c\t%12-15R, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06af0070, 0x0fff0ff0, "sxtb%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06af0470, 0x0fff0ff0, "sxtb%c\t%12-15R, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06af0870, 0x0fff0ff0, "sxtb%c\t%12-15R, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06af0c70, 0x0fff0ff0, "sxtb%c\t%12-15R, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ff0070, 0x0fff0ff0, "uxth%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ff0470, 0x0fff0ff0, "uxth%c\t%12-15R, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ff0870, 0x0fff0ff0, "uxth%c\t%12-15R, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ff0c70, 0x0fff0ff0, "uxth%c\t%12-15R, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06cf0070, 0x0fff0ff0, "uxtb16%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06cf0470, 0x0fff0ff0, "uxtb16%c\t%12-15R, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06cf0870, 0x0fff0ff0, "uxtb16%c\t%12-15R, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06cf0c70, 0x0fff0ff0, "uxtb16%c\t%12-15R, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ef0070, 0x0fff0ff0, "uxtb%c\t%12-15R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ef0470, 0x0fff0ff0, "uxtb%c\t%12-15R, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ef0870, 0x0fff0ff0, "uxtb%c\t%12-15R, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06ef0c70, 0x0fff0ff0, "uxtb%c\t%12-15R, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06b00070, 0x0ff00ff0, "sxtah%c\t%12-15R, %16-19r, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06b00470, 0x0ff00ff0, "sxtah%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06b00870, 0x0ff00ff0, "sxtah%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06b00c70, 0x0ff00ff0, "sxtah%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800070, 0x0ff00ff0, "sxtab16%c\t%12-15R, %16-19r, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800470, 0x0ff00ff0, "sxtab16%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800870, 0x0ff00ff0, "sxtab16%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800c70, 0x0ff00ff0, "sxtab16%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06a00070, 0x0ff00ff0, "sxtab%c\t%12-15R, %16-19r, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06a00470, 0x0ff00ff0, "sxtab%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06a00870, 0x0ff00ff0, "sxtab%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06a00c70, 0x0ff00ff0, "sxtab%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06f00070, 0x0ff00ff0, "uxtah%c\t%12-15R, %16-19r, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06f00470, 0x0ff00ff0, "uxtah%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06f00870, 0x0ff00ff0, "uxtah%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06f00c70, 0x0ff00ff0, "uxtah%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06c00070, 0x0ff00ff0, "uxtab16%c\t%12-15R, %16-19r, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06c00470, 0x0ff00ff0, "uxtab16%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06c00870, 0x0ff00ff0, "uxtab16%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06c00c70, 0x0ff00ff0, "uxtab16%c\t%12-15R, %16-19r, %0-3R, ROR %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06e00070, 0x0ff00ff0, "uxtab%c\t%12-15R, %16-19r, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06e00470, 0x0ff00ff0, "uxtab%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#8%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06e00870, 0x0ff00ff0, "uxtab%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#16%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06e00c70, 0x0ff00ff0, "uxtab%c\t%12-15R, %16-19r, %0-3R, %{B:ror%} %{I:#24%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06800fb0, 0x0ff00ff0, "sel%c\t%12-15R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xf1010000, 0xfffffc00, "setend\t%{B:%9?ble%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x0700f010, 0x0ff0f0d0, "smuad%5'x%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x0700f050, 0x0ff0f0d0, "smusd%5'x%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x07000010, 0x0ff000d0, "smlad%5'x%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x07400010, 0x0ff000d0, "smlald%5'x%c\t%12-15Ru, %16-19Ru, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x07000050, 0x0ff000d0, "smlsd%5'x%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x07400050, 0x0ff000d0, "smlsld%5'x%c\t%12-15Ru, %16-19Ru, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x0750f010, 0x0ff0f0d0, "smmul%5'r%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x07500010, 0x0ff000d0, "smmla%5'r%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x075000d0, 0x0ff000d0, "smmls%5'r%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0xf84d0500, 0xfe5fffe0, "srs%23?id%24?ba\t%16-19r%21'!, %{I:#%0-4d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06a00010, 0x0fe00ff0, "ssat%c\t%12-15R, %{I:#%16-20W%}, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06a00010, 0x0fe00070, "ssat%c\t%12-15R, %{I:#%16-20W%}, %0-3R, %{B:lsl%} %{I:#%7-11d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06a00050, 0x0fe00070, "ssat%c\t%12-15R, %{I:#%16-20W%}, %0-3R, %{B:asr%} %{I:#%7-11d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06a00f30, 0x0ff00ff0, "ssat16%c\t%12-15r, %{I:#%16-19W%}, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x01800f90, 0x0ff00ff0, "strex%c\t%12-15R, %0-3R, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x00400090, 0x0ff000f0, "umaal%c\t%12-15R, %16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x0780f010, 0x0ff0f0f0, "usad8%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x07800010, 0x0ff000f0, "usada8%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06e00010, 0x0fe00ff0, "usat%c\t%12-15R, %{I:#%16-20d%}, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06e00010, 0x0fe00070, "usat%c\t%12-15R, %{I:#%16-20d%}, %0-3R, %{B:lsl%} %{I:#%7-11d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06e00050, 0x0fe00070, "usat%c\t%12-15R, %{I:#%16-20d%}, %0-3R, %{B:asr%} %{I:#%7-11d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6),
    0x06e00f30, 0x0ff00ff0, "usat16%c\t%12-15R, %{I:#%16-19d%}, %0-3R"},

  /* V5J instruction.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5J),
    0x012fff20, 0x0ffffff0, "bxj%c\t%0-3R"},

  /* V5 Instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0xe1200070, 0xfff000f0,
    "bkpt\t%{I:0x%16-19X%12-15X%8-11X%0-3X%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0xfa000000, 0xfe000000, "blx\t%B"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0x012fff30, 0x0ffffff0, "blx%c\t%0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5),
    0x016f0f10, 0x0fff0ff0, "clz%c\t%12-15R, %0-3R"},

  /* V5E "El Segundo" Instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5E),
    0x000000d0, 0x0e1000f0, "ldrd%c\t%12-15r, %s"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5E),
    0x000000f0, 0x0e1000f0, "strd%c\t%12-15r, %s"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5E),
    0xf450f000, 0xfc70f000, "pld\t%a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x01000080, 0x0ff000f0, "smlabb%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x010000a0, 0x0ff000f0, "smlatb%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x010000c0, 0x0ff000f0, "smlabt%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x010000e0, 0x0ff000f0, "smlatt%c\t%16-19r, %0-3r, %8-11R, %12-15R"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x01200080, 0x0ff000f0, "smlawb%c\t%16-19R, %0-3R, %8-11R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x012000c0, 0x0ff000f0, "smlawt%c\t%16-19R, %0-3r, %8-11R, %12-15R"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x01400080, 0x0ff000f0, "smlalbb%c\t%12-15Ru, %16-19Ru, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x014000a0, 0x0ff000f0, "smlaltb%c\t%12-15Ru, %16-19Ru, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x014000c0, 0x0ff000f0, "smlalbt%c\t%12-15Ru, %16-19Ru, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x014000e0, 0x0ff000f0, "smlaltt%c\t%12-15Ru, %16-19Ru, %0-3R, %8-11R"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x01600080, 0x0ff0f0f0, "smulbb%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x016000a0, 0x0ff0f0f0, "smultb%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x016000c0, 0x0ff0f0f0, "smulbt%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x016000e0, 0x0ff0f0f0, "smultt%c\t%16-19R, %0-3R, %8-11R"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x012000a0, 0x0ff0f0f0, "smulwb%c\t%16-19R, %0-3R, %8-11R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x012000e0, 0x0ff0f0f0, "smulwt%c\t%16-19R, %0-3R, %8-11R"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x01000050, 0x0ff00ff0,  "qadd%c\t%12-15R, %0-3R, %16-19R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x01400050, 0x0ff00ff0, "qdadd%c\t%12-15R, %0-3R, %16-19R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x01200050, 0x0ff00ff0,  "qsub%c\t%12-15R, %0-3R, %16-19R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP),
    0x01600050, 0x0ff00ff0, "qdsub%c\t%12-15R, %0-3R, %16-19R"},

  /* ARM Instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x052d0004, 0x0fff0fff, "push%c\t{%12-15r}\t\t@ (str%c %12-15r, %a)"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x04400000, 0x0e500000, "strb%t%c\t%12-15R, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x04000000, 0x0e500000, "str%t%c\t%12-15r, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x06400000, 0x0e500ff0, "strb%t%c\t%12-15R, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x06000000, 0x0e500ff0, "str%t%c\t%12-15r, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x04400000, 0x0c500010, "strb%t%c\t%12-15R, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x04000000, 0x0c500010, "str%t%c\t%12-15r, %a"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x04400000, 0x0e500000, "strb%c\t%12-15R, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x06400000, 0x0e500010, "strb%c\t%12-15R, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x004000b0, 0x0e5000f0, "strh%c\t%12-15R, %s"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x000000b0, 0x0e500ff0, "strh%c\t%12-15R, %s"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00500090, 0x0e5000f0, UNDEFINED_INSTRUCTION},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00500090, 0x0e500090, "ldr%6's%5?hb%c\t%12-15R, %s"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00100090, 0x0e500ff0, UNDEFINED_INSTRUCTION},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00100090, 0x0e500f90, "ldr%6's%5?hb%c\t%12-15R, %s"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x02000000, 0x0fe00000, "and%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00000000, 0x0fe00010, "and%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00000010, 0x0fe00090, "and%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x02200000, 0x0fe00000, "eor%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00200000, 0x0fe00010, "eor%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00200010, 0x0fe00090, "eor%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x02400000, 0x0fe00000, "sub%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00400000, 0x0fe00010, "sub%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00400010, 0x0fe00090, "sub%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x02600000, 0x0fe00000, "rsb%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00600000, 0x0fe00010, "rsb%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00600010, 0x0fe00090, "rsb%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x02800000, 0x0fe00000, "add%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00800000, 0x0fe00010, "add%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00800010, 0x0fe00090, "add%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x02a00000, 0x0fe00000, "adc%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00a00000, 0x0fe00010, "adc%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00a00010, 0x0fe00090, "adc%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x02c00000, 0x0fe00000, "sbc%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00c00000, 0x0fe00010, "sbc%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00c00010, 0x0fe00090, "sbc%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x02e00000, 0x0fe00000, "rsc%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00e00000, 0x0fe00010, "rsc%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00e00010, 0x0fe00090, "rsc%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_VIRT),
    0x0120f200, 0x0fb0f200, "msr%c\t%C, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V3),
    0x0120f000, 0x0db0f000, "msr%c\t%C, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V3),
    0x01000000, 0x0fb00cff, "mrs%c\t%12-15R, %R"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x03000000, 0x0fe00000, "tst%p%c\t%16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01000000, 0x0fe00010, "tst%p%c\t%16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01000010, 0x0fe00090, "tst%p%c\t%16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x03300000, 0x0ff00000, "teq%p%c\t%16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01300000, 0x0ff00010, "teq%p%c\t%16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01300010, 0x0ff00010, "teq%p%c\t%16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x03400000, 0x0fe00000, "cmp%p%c\t%16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01400000, 0x0fe00010, "cmp%p%c\t%16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01400010, 0x0fe00090, "cmp%p%c\t%16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x03600000, 0x0fe00000, "cmn%p%c\t%16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01600000, 0x0fe00010, "cmn%p%c\t%16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01600010, 0x0fe00090, "cmn%p%c\t%16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x03800000, 0x0fe00000, "orr%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01800000, 0x0fe00010, "orr%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01800010, 0x0fe00090, "orr%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x03a00000, 0x0fef0000, "mov%20's%c\t%12-15r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01a00000, 0x0def0ff0, "mov%20's%c\t%12-15r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01a00000, 0x0def0060, "lsl%20's%c\t%12-15R, %q"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01a00020, 0x0def0060, "lsr%20's%c\t%12-15R, %q"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01a00040, 0x0def0060, "asr%20's%c\t%12-15R, %q"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01a00060, 0x0def0ff0, "rrx%20's%c\t%12-15r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01a00060, 0x0def0060, "ror%20's%c\t%12-15R, %q"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x03c00000, 0x0fe00000, "bic%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01c00000, 0x0fe00010, "bic%20's%c\t%12-15r, %16-19r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01c00010, 0x0fe00090, "bic%20's%c\t%12-15R, %16-19R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x03e00000, 0x0fe00000, "mvn%20's%c\t%12-15r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01e00000, 0x0fe00010, "mvn%20's%c\t%12-15r, %o"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x01e00010, 0x0fe00090, "mvn%20's%c\t%12-15R, %o"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x06000010, 0x0e000010, UNDEFINED_INSTRUCTION},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x049d0004, 0x0fff0fff, "pop%c\t{%12-15r}\t\t@ (ldr%c %12-15r, %a)"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x04500000, 0x0c500000, "ldrb%t%c\t%12-15R, %a"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x04300000, 0x0d700000, "ldrt%c\t%12-15R, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x04100000, 0x0c500000, "ldr%c\t%12-15r, %a"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0001, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0002, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0004, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0008, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0010, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0020, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0040, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0080, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0100, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0200, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0400, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0800, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d1000, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d2000, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d4000, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d8000, 0x0fffffff, "stmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x092d0000, 0x0fff0000, "push%c\t%m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08800000, 0x0ff00000, "stm%c\t%16-19R%21'!, %m%22'^"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08000000, 0x0e100000, "stm%23?id%24?ba%c\t%16-19R%21'!, %m%22'^"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0001, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0002, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0004, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0008, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0010, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0020, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0040, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0080, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0100, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0200, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0400, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0800, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd1000, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd2000, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd4000, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd8000, 0x0fffffff, "ldmfd%c\t%16-19R!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08bd0000, 0x0fff0000, "pop%c\t%m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08900000, 0x0f900000, "ldm%c\t%16-19R%21'!, %m%22'^"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x08100000, 0x0e100000, "ldm%23?id%24?ba%c\t%16-19R%21'!, %m%22'^"},

  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x0a000000, 0x0e000000, "b%24'l%c\t%b"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x0f000000, 0x0f000000, "svc%c\t%0-23x"},

  /* The rest.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7),
    0x03200000, 0x0fff00ff, "nop%c\t{%{I:%0-7d%}}" UNPREDICTABLE_INSTRUCTION},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
    0x00000000, 0x00000000, UNDEFINED_INSTRUCTION},
  {ARM_FEATURE_CORE_LOW (0),
    0x00000000, 0x00000000, 0}
};

/* print_insn_thumb16 recognizes the following format control codes:

   %S                   print Thumb register (bits 3..5 as high number if bit 6 set)
   %D                   print Thumb register (bits 0..2 as high number if bit 7 set)
   %<bitfield>I         print bitfield as a signed decimal
   				(top bit of range being the sign bit)
   %N                   print Thumb register mask (with LR)
   %O                   print Thumb register mask (with PC)
   %M                   print Thumb register mask
   %b			print CZB's 6-bit unsigned branch destination
   %s			print Thumb right-shift immediate (6..10; 0 == 32).
   %c			print the condition code
   %C			print the condition code, or "s" if not conditional
   %x			print warning if conditional an not at end of IT block"
   %X			print "\t@ unpredictable <IT:code>" if conditional
   %I			print IT instruction suffix and operands
   %W			print Thumb Writeback indicator for LDMIA
   %<bitfield>r		print bitfield as an ARM register
   %<bitfield>d		print bitfield as a decimal
   %<bitfield>H         print (bitfield * 2) as a decimal
   %<bitfield>W         print (bitfield * 4) as a decimal
   %<bitfield>a         print (bitfield * 4) as a pc-rel offset + decoded symbol
   %<bitfield>B         print Thumb branch destination (signed displacement)
   %<bitfield>c         print bitfield as a condition code
   %<bitnum>'c		print specified char iff bit is one
   %<bitnum>?ab		print a if bit is one else print b.  */

static const struct opcode16 thumb_opcodes[] =
{
  /* Thumb instructions.  */

  /* ARMv8-M Security Extensions instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M), 0x4784, 0xff87, "blxns\t%3-6r"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M), 0x4704, 0xff87, "bxns\t%3-6r"},

  /* ARM V8 instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),  0xbf50, 0xffff, "sevl%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),  0xba80, 0xffc0, "hlt\t%0-5x"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_PAN),  0xb610, 0xfff7, "setpan\t%{I:#%3-3d%}"},

  /* ARM V6K no-argument instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K), 0xbf00, 0xffff, "nop%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K), 0xbf10, 0xffff, "yield%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K), 0xbf20, 0xffff, "wfe%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K), 0xbf30, 0xffff, "wfi%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K), 0xbf40, 0xffff, "sev%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6K), 0xbf00, 0xff0f, "nop%c\t{%4-7d}"},

  /* ARM V6T2 instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xb900, 0xfd00, "cbnz\t%0-2r, %b%X"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xb100, 0xfd00, "cbz\t%0-2r, %b%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xbf00, 0xff00, "it%I%X"},

  /* ARM V6.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xb660, 0xfff8, "cpsie\t%{B:%2'a%1'i%0'f%}%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xb670, 0xfff8, "cpsid\t%{B:%2'a%1'i%0'f%}%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0x4600, 0xffc0, "mov%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xba00, 0xffc0, "rev%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xba40, 0xffc0, "rev16%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xbac0, 0xffc0, "revsh%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xb650, 0xfff7, "setend\t%{B:%3?ble%}%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xb200, 0xffc0, "sxth%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xb240, 0xffc0, "sxtb%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xb280, 0xffc0, "uxth%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6), 0xb2c0, 0xffc0, "uxtb%c\t%0-2r, %3-5r"},

  /* ARM V5 ISA extends Thumb.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5T),
    0xbe00, 0xff00, "bkpt\t%0-7x"}, /* Is always unconditional.  */
  /* This is BLX(2).  BLX(1) is a 32-bit instruction.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V5T),
    0x4780, 0xff87, "blx%c\t%3-6r%x"},	/* note: 4 bit register number.  */
  /* ARM V4T ISA (Thumb v1).  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x46C0, 0xFFFF, "nop%c\t\t\t@ (mov r8, r8)"},
  /* Format 4.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4000, 0xFFC0, "and%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4040, 0xFFC0, "eor%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4080, 0xFFC0, "lsl%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x40C0, 0xFFC0, "lsr%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4100, 0xFFC0, "asr%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4140, 0xFFC0, "adc%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4180, 0xFFC0, "sbc%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x41C0, 0xFFC0, "ror%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4200, 0xFFC0, "tst%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4240, 0xFFC0, "neg%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4280, 0xFFC0, "cmp%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x42C0, 0xFFC0, "cmn%c\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4300, 0xFFC0, "orr%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4340, 0xFFC0, "mul%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4380, 0xFFC0, "bic%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x43C0, 0xFFC0, "mvn%C\t%0-2r, %3-5r"},
  /* format 13 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xB000, 0xFF80, "add%c\t%{R:sp%}, %{I:#%0-6W%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xB080, 0xFF80, "sub%c\t%{R:sp%}, %{I:#%0-6W%}"},
  /* format 5 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4700, 0xFF80, "bx%c\t%S%x"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4400, 0xFF00, "add%c\t%D, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4500, 0xFF00, "cmp%c\t%D, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x4600, 0xFF00, "mov%c\t%D, %S"},
  /* format 14 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xB400, 0xFE00, "push%c\t%N"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xBC00, 0xFE00, "pop%c\t%O"},
  /* format 2 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x1800, 0xFE00, "add%C\t%0-2r, %3-5r, %6-8r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x1A00, 0xFE00, "sub%C\t%0-2r, %3-5r, %6-8r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x1C00, 0xFE00, "add%C\t%0-2r, %3-5r, %{I:#%6-8d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x1E00, 0xFE00, "sub%C\t%0-2r, %3-5r, %{I:#%6-8d%}"},
  /* format 8 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x5200, 0xFE00, "strh%c\t%0-2r, [%3-5r, %6-8r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x5A00, 0xFE00, "ldrh%c\t%0-2r, [%3-5r, %6-8r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x5600, 0xF600, "ldrs%11?hb%c\t%0-2r, [%3-5r, %6-8r]"},
  /* format 7 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x5000, 0xFA00, "str%10'b%c\t%0-2r, [%3-5r, %6-8r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x5800, 0xFA00, "ldr%10'b%c\t%0-2r, [%3-5r, %6-8r]"},
  /* format 1 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x0000, 0xFFC0, "mov%C\t%0-2r, %3-5r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x0000, 0xF800, "lsl%C\t%0-2r, %3-5r, %{I:#%6-10d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x0800, 0xF800, "lsr%C\t%0-2r, %3-5r, %s"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x1000, 0xF800, "asr%C\t%0-2r, %3-5r, %s"},
  /* format 3 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x2000, 0xF800, "mov%C\t%8-10r, %{I:#%0-7d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x2800, 0xF800, "cmp%c\t%8-10r, %{I:#%0-7d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x3000, 0xF800, "add%C\t%8-10r, %{I:#%0-7d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0x3800, 0xF800, "sub%C\t%8-10r, %{I:#%0-7d%}"},
  /* format 6 */
  /* TODO: Disassemble PC relative "LDR rD,=<symbolic>" */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x4800, 0xF800,
    "ldr%c\t%8-10r, [%{R:pc%}, %{I:#%0-7W%}]\t@ (%0-7a)"},
  /* format 9 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x6000, 0xF800, "str%c\t%0-2r, [%3-5r, %{I:#%6-10W%}]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x6800, 0xF800, "ldr%c\t%0-2r, [%3-5r, %{I:#%6-10W%}]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x7000, 0xF800, "strb%c\t%0-2r, [%3-5r, %{I:#%6-10d%}]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x7800, 0xF800, "ldrb%c\t%0-2r, [%3-5r, %{I:#%6-10d%}]"},
  /* format 10 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x8000, 0xF800, "strh%c\t%0-2r, [%3-5r, %{I:#%6-10H%}]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x8800, 0xF800, "ldrh%c\t%0-2r, [%3-5r, %{I:#%6-10H%}]"},
  /* format 11 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x9000, 0xF800, "str%c\t%8-10r, [%{R:sp%}, %{I:#%0-7W%}]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0x9800, 0xF800, "ldr%c\t%8-10r, [%{R:sp%}, %{I:#%0-7W%}]"},
  /* format 12 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0xA000, 0xF800, "add%c\t%8-10r, %{R:pc%}, %{I:#%0-7W%}\t@ (adr %8-10r, %0-7a)"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
    0xA800, 0xF800, "add%c\t%8-10r, %{R:sp%}, %{I:#%0-7W%}"},
  /* format 15 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xC000, 0xF800, "stmia%c\t%8-10r!, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xC800, 0xF800, "ldmia%c\t%8-10r%W, %M"},
  /* format 17 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xDF00, 0xFF00, "svc%c\t%0-7d"},
  /* format 16 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xDE00, 0xFF00, "udf%c\t%{I:#%0-7d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xDE00, 0xFE00, UNDEFINED_INSTRUCTION},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xD000, 0xF000, "b%8-11c.n\t%0-7B%X"},
  /* format 18 */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T), 0xE000, 0xF800, "b%c.n\t%0-10B%x"},

  /* The E800 .. FFFF range is unconditionally redirected to the
     32-bit table, because even in pre-V6T2 ISAs, BL and BLX(1) pairs
     are processed via that table.  Thus, we can never encounter a
     bare "second half of BL/BLX(1)" instruction here.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),  0x0000, 0x0000, UNDEFINED_INSTRUCTION},
  {ARM_FEATURE_CORE_LOW (0), 0, 0, 0}
};

/* Thumb32 opcodes use the same table structure as the ARM opcodes.
   We adopt the convention that hw1 is the high 16 bits of .value and
   .mask, hw2 the low 16 bits.

   print_insn_thumb32 recognizes the following format control codes:

       %%		%

       %I		print a 12-bit immediate from hw1[10],hw2[14:12,7:0]
       %M		print a modified 12-bit immediate (same location)
       %J		print a 16-bit immediate from hw1[3:0,10],hw2[14:12,7:0]
       %K		print a 16-bit immediate from hw2[3:0],hw1[3:0],hw2[11:4]
       %H		print a 16-bit immediate from hw2[3:0],hw1[11:0]
       %S		print a possibly-shifted Rm

       %L		print address for a ldrd/strd instruction
       %a		print the address of a plain load/store
       %w		print the width and signedness of a core load/store
       %m		print register mask for ldm/stm
       %n		print register mask for clrm

       %E		print the lsb and width fields of a bfc/bfi instruction
       %F		print the lsb and width fields of a sbfx/ubfx instruction
       %G		print a fallback offset for Branch Future instructions
       %W		print an offset for BF instruction
       %Y		print an offset for BFL instruction
       %Z		print an offset for BFCSEL instruction
       %Q		print an offset for Low Overhead Loop instructions
       %P		print an offset for Low Overhead Loop end instructions
       %b		print a conditional branch offset
       %B		print an unconditional branch offset
       %s		print the shift field of an SSAT instruction
       %R		print the rotation field of an SXT instruction
       %U		print barrier type.
       %P		print address for pli instruction.
       %c		print the condition code
       %x		print warning if conditional an not at end of IT block"
       %X		print "\t@ unpredictable <IT:code>" if conditional

       %<bitfield>d	print bitfield in decimal
       %<bitfield>D     print bitfield plus one in decimal
       %<bitfield>W	print bitfield*4 in decimal
       %<bitfield>r	print bitfield as an ARM register
       %<bitfield>R	as %<>r but r15 is UNPREDICTABLE
       %<bitfield>S	as %<>r but r13 and r15 is UNPREDICTABLE
       %<bitfield>c	print bitfield as a condition code

       %<bitfield>'c	print specified char iff bitfield is all ones
       %<bitfield>`c	print specified char iff bitfield is all zeroes
       %<bitfield>?ab... select from array of values in big endian order

   With one exception at the bottom (done because BL and BLX(1) need
   to come dead last), this table was machine-sorted first in
   decreasing order of number of bits set in the mask, then in
   increasing numeric order of mask, then in increasing numeric order
   of opcode.  This order is not the clearest for a human reader, but
   is guaranteed never to catch a special-case bit pattern with a more
   general mask, which is important, because this instruction encoding
   makes heavy use of special-case bit patterns.  */
static const struct opcode32 thumb32_opcodes[] =
{
  /* Arm v8.1-M Mainline Pointer Authentication and Branch Target
     Identification Extension.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   0xf3af802d, 0xffffffff, "aut\t%{R:r12%}, %{R:lr%}, %{R:sp%}"},
  {ARM_FEATURE_CORE_HIGH_HIGH (ARM_EXT3_PACBTI),
   0xfb500f00, 0xfff00ff0, "autg%c\t%12-15r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   0xf3af800f, 0xffffffff, "bti"},
  {ARM_FEATURE_CORE_HIGH_HIGH (ARM_EXT3_PACBTI),
   0xfb500f10, 0xfff00ff0, "bxaut%c\t%12-15r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   0xf3af801d, 0xffffffff, "pac\t%{R:r12%}, %{R:lr%}, %{R:sp%}"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
   0xf3af800d, 0xffffffff, "pacbti\t%{R:r12%}, %{R:lr%}, %{R:sp%}"},
  {ARM_FEATURE_CORE_HIGH_HIGH (ARM_EXT3_PACBTI),
   0xfb60f000, 0xfff0f0f0, "pacg%c\t%8-11r, %16-19r, %0-3r"},

  /* Armv8.1-M Mainline and Armv8.1-M Mainline Security Extensions
     instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf00fe001, 0xffffffff, "lctp%c"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf02fc001, 0xfffff001, "le\t%P"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf00fc001, 0xfffff001, "le\t%{R:lr%}, %P"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf01fc001, 0xfffff001, "letp\t%{R:lr%}, %P"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf040c001, 0xfff0f001, "wls\t%{R:lr%}, %16-19S, %Q"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf000c001, 0xffc0f001, "wlstp.%20-21s\t%{R:lr%}, %16-19S, %Q"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf040e001, 0xfff0ffff, "dls\t%{R:lr%}, %16-19S"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf000e001, 0xffc0ffff, "dlstp.%20-21s\t%{R:lr%}, %16-19S"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf040e001, 0xf860f001, "bf%c\t%G, %W"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf060e001, 0xf8f0f001, "bfx%c\t%G, %16-19S"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf000c001, 0xf800f001, "bfl%c\t%G, %Y"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf070e001, 0xf8f0f001, "bflx%c\t%G, %16-19S"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xf000e001, 0xf840f001, "bfcsel\t%G, %Z, %{B:%18-21c%}"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN),
    0xe89f0000, 0xffff2000, "clrm%c\t%n"},

  /* ARMv8-M and ARMv8-M Security Extensions instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M), 0xe97fe97f, 0xffffffff, "sg"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M),
    0xe840f000, 0xfff0f0ff, "tt\t%8-11r, %16-19r"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M),
    0xe840f040, 0xfff0f0ff, "ttt\t%8-11r, %16-19r"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M),
    0xe840f080, 0xfff0f0ff, "tta\t%8-11r, %16-19r"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M),
    0xe840f0c0, 0xfff0f0ff, "ttat\t%8-11r, %16-19r"},

  /* ARM V8.2 RAS extension instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_RAS),
    0xf3af8010, 0xffffffff, "esb"},

  /* V8 instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xf3af8005, 0xffffffff, "sevl%c.w"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xf78f8000, 0xfffffffc, "dcps%0-1d"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8c00f8f, 0xfff00fff, "stlb%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8c00f9f, 0xfff00fff, "stlh%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8c00faf, 0xfff00fff, "stl%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8c00fc0, 0xfff00ff0, "stlexb%c\t%0-3r, %12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8c00fd0, 0xfff00ff0, "stlexh%c\t%0-3r, %12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8c00fe0, 0xfff00ff0, "stlex%c\t%0-3r, %12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8c000f0, 0xfff000f0, "stlexd%c\t%0-3r, %12-15r, %8-11r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8d00f8f, 0xfff00fff, "ldab%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8d00f9f, 0xfff00fff, "ldah%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8d00faf, 0xfff00fff, "lda%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8d00fcf, 0xfff00fff, "ldaexb%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8d00fdf, 0xfff00fff, "ldaexh%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8d00fef, 0xfff00fff, "ldaex%c\t%12-15r, [%16-19R]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8),
    0xe8d000ff, 0xfff000ff, "ldaexd%c\t%12-15r, %8-11r, [%16-19R]"},

  /* V8-R instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8R),
    0xf3bf8f4c, 0xffffffff, "dfb%c"},

  /* CRC32 instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xfac0f080, 0xfff0f0f0, "crc32b\t%8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xfac0f090, 0xfff0f0f0, "crc32h\t%9-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xfac0f0a0, 0xfff0f0f0, "crc32w\t%8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xfad0f080, 0xfff0f0f0, "crc32cb\t%8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xfad0f090, 0xfff0f0f0, "crc32ch\t%8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
    0xfad0f0a0, 0xfff0f0f0, "crc32cw\t%8-11R, %16-19R, %0-3R"},

  /* Speculation Barriers.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf3af8014, 0xffffffff, "csdb"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf3bf8f40, 0xffffffff, "ssbb"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf3bf8f44, 0xffffffff, "pssbb"},

  /* V7 instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf910f000, 0xff70f000, "pli%c\t%a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf3af80f0, 0xfffffff0, "dbg%c\t%{I:#%0-3d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8), 0xf3bf8f51, 0xfffffff3, "dmb%c\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V8), 0xf3bf8f41, 0xfffffff3, "dsb%c\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf3bf8f50, 0xfffffff0, "dmb%c\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf3bf8f40, 0xfffffff0, "dsb%c\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V7), 0xf3bf8f60, 0xfffffff0, "isb%c\t%U"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_DIV),
    0xfb90f0f0, 0xfff0f0f0, "sdiv%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_DIV),
    0xfbb0f0f0, 0xfff0f0f0, "udiv%c\t%8-11r, %16-19r, %0-3r"},

  /* Virtualization Extension instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_VIRT), 0xf7e08000, 0xfff0f000, "hvc%c\t%V"},
  /* We skip ERET as that is SUBS pc, lr, #0.  */

  /* MP Extension instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_MP),   0xf830f000, 0xff70f000, "pldw%c\t%a"},

  /* Security extension instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_SEC),  0xf7f08000, 0xfff0f000, "smc%c\t%K"},

  /* ARMv8.5-A instructions.  */
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB), 0xf3bf8f70, 0xffffffff, "sb"},

  /* Instructions defined in the basic V6T2 set.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf3af8000, 0xffffffff, "nop%c.w"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf3af8001, 0xffffffff, "yield%c.w"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf3af8002, 0xffffffff, "wfe%c.w"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf3af8003, 0xffffffff, "wfi%c.w"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf3af8004, 0xffffffff, "sev%c.w"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3af8000, 0xffffff00, "nop%c.w\t{%{I:%0-7d%}}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2), 0xf7f0a000, 0xfff0f000, "udf%c.w\t%H"},

  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xf3bf8f2f, 0xffffffff, "clrex%c"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3af8400, 0xffffff1f, "cpsie.w\t%{B:%7'a%6'i%5'f%}%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3af8600, 0xffffff1f, "cpsid.w\t%{B:%7'a%6'i%5'f%}%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3c08f00, 0xfff0ffff, "bxj%c\t%16-19r%x"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe810c000, 0xffd0ffff, "rfedb%c\t%16-19r%21'!"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe990c000, 0xffd0ffff, "rfeia%c\t%16-19r%21'!"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3e08000, 0xffe0f000, "mrs%c\t%8-11r, %D"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3af8100, 0xffffffe0, "cps\t%{I:#%0-4d%}%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe8d0f000, 0xfff0fff0, "tbb%c\t[%16-19r, %0-3r]%x"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe8d0f010, 0xfff0fff0, "tbh%c\t[%16-19r, %0-3r, %{B:lsl%} %{I:#1%}]%x"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3af8500, 0xffffff00, "cpsie\t%{B:%7'a%6'i%5'f%}, %{I:#%0-4d%}%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3af8700, 0xffffff00, "cpsid\t%{B:%7'a%6'i%5'f%}, %{I:#%0-4d%}%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3de8f00, 0xffffff00, "subs%c\t%{R:pc%}, %{R:lr%}, %{I:#%0-7d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3808000, 0xffe0f000, "msr%c\t%C, %16-19r"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xe8500f00, 0xfff00fff, "ldrex%c\t%12-15r, [%16-19r]"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xe8d00f4f, 0xfff00fef, "ldrex%4?hb%c\t%12-15r, [%16-19r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe800c000, 0xffd0ffe0, "srsdb%c\t%16-19r%21'!, %{I:#%0-4d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe980c000, 0xffd0ffe0, "srsia%c\t%16-19r%21'!, %{I:#%0-4d%}"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa0ff080, 0xfffff0c0, "sxth%c.w\t%8-11r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa1ff080, 0xfffff0c0, "uxth%c.w\t%8-11r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa2ff080, 0xfffff0c0, "sxtb16%c\t%8-11r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa3ff080, 0xfffff0c0, "uxtb16%c\t%8-11r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa4ff080, 0xfffff0c0, "sxtb%c.w\t%8-11r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa5ff080, 0xfffff0c0, "uxtb%c.w\t%8-11r, %0-3r%R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xe8400000, 0xfff000ff, "strex%c\t%8-11r, %12-15r, [%16-19r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe8d0007f, 0xfff000ff, "ldrexd%c\t%12-15r, %8-11r, [%16-19r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f000, 0xfff0f0f0, "sadd8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f010, 0xfff0f0f0, "qadd8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f020, 0xfff0f0f0, "shadd8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f040, 0xfff0f0f0, "uadd8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f050, 0xfff0f0f0, "uqadd8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f060, 0xfff0f0f0, "uhadd8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f080, 0xfff0f0f0, "qadd%c\t%8-11r, %0-3r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f090, 0xfff0f0f0, "qdadd%c\t%8-11r, %0-3r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f0a0, 0xfff0f0f0, "qsub%c\t%8-11r, %0-3r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa80f0b0, 0xfff0f0f0, "qdsub%c\t%8-11r, %0-3r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f000, 0xfff0f0f0, "sadd16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f010, 0xfff0f0f0, "qadd16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f020, 0xfff0f0f0, "shadd16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f040, 0xfff0f0f0, "uadd16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f050, 0xfff0f0f0, "uqadd16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f060, 0xfff0f0f0, "uhadd16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f080, 0xfff0f0f0, "rev%c.w\t%8-11r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f090, 0xfff0f0f0, "rev16%c.w\t%8-11r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f0a0, 0xfff0f0f0, "rbit%c\t%8-11r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa90f0b0, 0xfff0f0f0, "revsh%c.w\t%8-11r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfaa0f000, 0xfff0f0f0, "sasx%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfaa0f010, 0xfff0f0f0, "qasx%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfaa0f020, 0xfff0f0f0, "shasx%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfaa0f040, 0xfff0f0f0, "uasx%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfaa0f050, 0xfff0f0f0, "uqasx%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfaa0f060, 0xfff0f0f0, "uhasx%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfaa0f080, 0xfff0f0f0, "sel%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfab0f080, 0xfff0f0f0, "clz%c\t%8-11r, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfac0f000, 0xfff0f0f0, "ssub8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfac0f010, 0xfff0f0f0, "qsub8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfac0f020, 0xfff0f0f0, "shsub8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfac0f040, 0xfff0f0f0, "usub8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfac0f050, 0xfff0f0f0, "uqsub8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfac0f060, 0xfff0f0f0, "uhsub8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfad0f000, 0xfff0f0f0, "ssub16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfad0f010, 0xfff0f0f0, "qsub16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfad0f020, 0xfff0f0f0, "shsub16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfad0f040, 0xfff0f0f0, "usub16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfad0f050, 0xfff0f0f0, "uqsub16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfad0f060, 0xfff0f0f0, "uhsub16%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfae0f000, 0xfff0f0f0, "ssax%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfae0f010, 0xfff0f0f0, "qsax%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfae0f020, 0xfff0f0f0, "shsax%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfae0f040, 0xfff0f0f0, "usax%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfae0f050, 0xfff0f0f0, "uqsax%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfae0f060, 0xfff0f0f0, "uhsax%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb00f000, 0xfff0f0f0, "mul%c.w\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb70f000, 0xfff0f0f0, "usad8%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa00f000, 0xffe0f0f0, "lsl%20's%c.w\t%8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa20f000, 0xffe0f0f0, "lsr%20's%c.w\t%8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa40f000, 0xffe0f0f0, "asr%20's%c.w\t%8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa60f000, 0xffe0f0f0, "ror%20's%c.w\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xe8c00f40, 0xfff00fe0, "strex%4?hb%c\t%0-3r, %12-15r, [%16-19r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3200000, 0xfff0f0e0, "ssat16%c\t%8-11r, %{I:#%0-4D%}, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3a00000, 0xfff0f0e0, "usat16%c\t%8-11r, %{I:#%0-4d%}, %16-19r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb20f000, 0xfff0f0e0, "smuad%4'x%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb30f000, 0xfff0f0e0, "smulw%4?tb%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb40f000, 0xfff0f0e0, "smusd%4'x%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb50f000, 0xfff0f0e0, "smmul%4'r%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa00f080, 0xfff0f0c0, "sxtah%c\t%8-11r, %16-19r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa10f080, 0xfff0f0c0, "uxtah%c\t%8-11r, %16-19r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa20f080, 0xfff0f0c0, "sxtab16%c\t%8-11r, %16-19r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa30f080, 0xfff0f0c0, "uxtab16%c\t%8-11r, %16-19r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa40f080, 0xfff0f0c0, "sxtab%c\t%8-11r, %16-19r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfa50f080, 0xfff0f0c0, "uxtab%c\t%8-11r, %16-19r, %0-3r%R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb10f000, 0xfff0f0c0, "smul%5?tb%4?tb%c\t%8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf36f0000, 0xffff8020, "bfc%c\t%8-11r, %E"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea100f00, 0xfff08f00, "tst%c.w\t%16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea900f00, 0xfff08f00, "teq%c\t%16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xeb100f00, 0xfff08f00, "cmn%c.w\t%16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xebb00f00, 0xfff08f00, "cmp%c.w\t%16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0100f00, 0xfbf08f00, "tst%c.w\t%16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0900f00, 0xfbf08f00, "teq%c\t%16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf1100f00, 0xfbf08f00, "cmn%c.w\t%16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf1b00f00, 0xfbf08f00, "cmp%c.w\t%16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea4f0000, 0xffef8000, "mov%20's%c.w\t%8-11r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea6f0000, 0xffef8000, "mvn%20's%c.w\t%8-11r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe8c00070, 0xfff000f0, "strexd%c\t%0-3r, %12-15r, %8-11r, [%16-19r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb000000, 0xfff000f0, "mla%c\t%8-11r, %16-19r, %0-3r, %12-15r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb000010, 0xfff000f0, "mls%c\t%8-11r, %16-19r, %0-3r, %12-15r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb700000, 0xfff000f0, "usada8%c\t%8-11R, %16-19R, %0-3R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb800000, 0xfff000f0, "smull%c\t%12-15R, %8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfba00000, 0xfff000f0, "umull%c\t%12-15R, %8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfbc00000, 0xfff000f0, "smlal%c\t%12-15R, %8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfbe00000, 0xfff000f0, "umlal%c\t%12-15R, %8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfbe00060, 0xfff000f0, "umaal%c\t%12-15R, %8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xe8500f00, 0xfff00f00, "ldrex%c\t%12-15r, [%16-19r, %{I:#%0-7W%}]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf04f0000, 0xfbef8000, "mov%20's%c.w\t%8-11r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf06f0000, 0xfbef8000, "mvn%20's%c.w\t%8-11r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf810f000, 0xff70f000, "pld%c\t%a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb200000, 0xfff000e0, "smlad%4'x%c\t%8-11R, %16-19R, %0-3R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb300000, 0xfff000e0, "smlaw%4?tb%c\t%8-11R, %16-19R, %0-3R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb400000, 0xfff000e0, "smlsd%4'x%c\t%8-11R, %16-19R, %0-3R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb500000, 0xfff000e0, "smmla%4'r%c\t%8-11R, %16-19R, %0-3R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb600000, 0xfff000e0, "smmls%4'r%c\t%8-11R, %16-19R, %0-3R, %12-15R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfbc000c0, 0xfff000e0, "smlald%4'x%c\t%12-15R, %8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfbd000c0, 0xfff000e0, "smlsld%4'x%c\t%12-15R, %8-11R, %16-19R, %0-3R"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xeac00000, 0xfff08030, "pkhbt%c\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xeac00020, 0xfff08030, "pkhtb%c\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3400000, 0xfff08020, "sbfx%c\t%8-11r, %16-19r, %F"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3c00000, 0xfff08020, "ubfx%c\t%8-11r, %16-19r, %F"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf8000e00, 0xff900f00, "str%wt%c\t%12-15r, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfb100000, 0xfff000c0,
    "smla%5?tb%4?tb%c\t%8-11r, %16-19r, %0-3r, %12-15r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xfbc00080, 0xfff000c0,
    "smlal%5?tb%4?tb%c\t%12-15r, %8-11r, %16-19r, %0-3r"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3600000, 0xfff08020, "bfi%c\t%8-11r, %16-19r, %E"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf8100e00, 0xfe900f00, "ldr%wt%c\t%12-15r, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3000000, 0xffd08020, "ssat%c\t%8-11r, %{I:#%0-4D%}, %16-19r%s"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3800000, 0xffd08020, "usat%c\t%8-11r, %{I:#%0-4d%}, %16-19r%s"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf2000000, 0xfbf08000, "addw%c\t%8-11r, %16-19r, %I"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xf2400000, 0xfbf08000, "movw%c\t%8-11r, %J"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf2a00000, 0xfbf08000, "subw%c\t%8-11r, %16-19r, %I"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xf2c00000, 0xfbf08000, "movt%c\t%8-11r, %J"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea000000, 0xffe08000, "and%20's%c.w\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea200000, 0xffe08000, "bic%20's%c.w\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea400000, 0xffe08000, "orr%20's%c.w\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea600000, 0xffe08000, "orn%20's%c\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xea800000, 0xffe08000, "eor%20's%c.w\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xeb000000, 0xffe08000, "add%20's%c.w\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xeb400000, 0xffe08000, "adc%20's%c.w\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xeb600000, 0xffe08000, "sbc%20's%c.w\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xeba00000, 0xffe08000, "sub%20's%c.w\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xebc00000, 0xffe08000, "rsb%20's%c\t%8-11r, %16-19r, %S"},
  {ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M),
    0xe8400000, 0xfff00000, "strex%c\t%8-11r, %12-15r, [%16-19r, %{I:#%0-7W%}]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0000000, 0xfbe08000, "and%20's%c.w\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0200000, 0xfbe08000, "bic%20's%c.w\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0400000, 0xfbe08000, "orr%20's%c.w\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0600000, 0xfbe08000, "orn%20's%c\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0800000, 0xfbe08000, "eor%20's%c.w\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf1000000, 0xfbe08000, "add%20's%c.w\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf1400000, 0xfbe08000, "adc%20's%c.w\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf1600000, 0xfbe08000, "sbc%20's%c.w\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf1a00000, 0xfbe08000, "sub%20's%c.w\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf1c00000, 0xfbe08000, "rsb%20's%c\t%8-11r, %16-19r, %M"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe8800000, 0xffd00000, "stmia%c.w\t%16-19r%21'!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe8900000, 0xffd00000, "ldmia%c.w\t%16-19r%21'!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe9000000, 0xffd00000, "stmdb%c\t%16-19r%21'!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe9100000, 0xffd00000, "ldmdb%c\t%16-19r%21'!, %m"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe9c00000, 0xffd000ff, "strd%c\t%12-15r, %8-11r, [%16-19r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe9d00000, 0xffd000ff, "ldrd%c\t%12-15r, %8-11r, [%16-19r]"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe9400000, 0xff500000,
    "strd%c\t%12-15r, %8-11r, [%16-19r, %{I:#%23`-%0-7W%}]%21'!%L"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe9500000, 0xff500000,
    "ldrd%c\t%12-15r, %8-11r, [%16-19r, %{I:#%23`-%0-7W%}]%21'!%L"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe8600000, 0xff700000,
    "strd%c\t%12-15r, %8-11r, [%16-19r], %{I:#%23`-%0-7W%}%L"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xe8700000, 0xff700000,
    "ldrd%c\t%12-15r, %8-11r, [%16-19r], %{I:#%23`-%0-7W%}%L"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf8000000, 0xff100000, "str%w%c.w\t%12-15r, %a"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf8100000, 0xfe100000, "ldr%w%c.w\t%12-15r, %a"},

  /* Filter out Bcc with cond=E or F, which are used for other instructions.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3c08000, 0xfbc0d000, "undefined (bcc, cond=0xF)"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf3808000, 0xfbc0d000, "undefined (bcc, cond=0xE)"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0008000, 0xf800d000, "b%22-25c.w\t%b%X"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2),
    0xf0009000, 0xf800d000, "b%c.w\t%B%x"},

  /* These have been 32-bit since the invention of Thumb.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
     0xf000c000, 0xf800d001, "blx%c\t%B%x"},
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V4T),
     0xf000d000, 0xf800d000, "bl%c\t%B%x"},

  /* Fallback.  */
  {ARM_FEATURE_CORE_LOW (ARM_EXT_V1),
      0x00000000, 0x00000000, UNDEFINED_INSTRUCTION},
  {ARM_FEATURE_CORE_LOW (0), 0, 0, 0}
};

static const char *const arm_conditional[] =
{"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
 "hi", "ls", "ge", "lt", "gt", "le", "al", "<und>", ""};

static const char *const arm_fp_const[] =
{"0.0", "1.0", "2.0", "3.0", "4.0", "5.0", "0.5", "10.0"};

static const char *const arm_shift[] =
{"lsl", "lsr", "asr", "ror"};

typedef struct
{
  const char *name;
  const char *description;
  const char *reg_names[16];
}
arm_regname;

static const arm_regname regnames[] =
{
  { "reg-names-raw", N_("Select raw register names"),
    { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"}},
  { "reg-names-gcc", N_("Select register names used by GCC"),
    { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "sl",  "fp",  "ip",  "sp",  "lr",  "pc" }},
  { "reg-names-std", N_("Select register names used in ARM's ISA documentation"),
    { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "sp",  "lr",  "pc" }},
  { "force-thumb", N_("Assume all insns are Thumb insns"), {NULL} },
  { "no-force-thumb", N_("Examine preceding label to determine an insn's type"), {NULL} },
  { "reg-names-apcs", N_("Select register names used in the APCS"),
    { "a1", "a2", "a3", "a4", "v1", "v2", "v3", "v4", "v5", "v6", "sl",  "fp",  "ip",  "sp",  "lr",  "pc" }},
  { "reg-names-atpcs", N_("Select register names used in the ATPCS"),
    { "a1", "a2", "a3", "a4", "v1", "v2", "v3", "v4", "v5", "v6", "v7",  "v8",  "IP",  "SP",  "LR",  "PC" }},
  { "reg-names-special-atpcs", N_("Select special register names used in the ATPCS"),
    { "a1", "a2", "a3", "a4", "v1", "v2", "v3", "WR", "v5", "SB", "SL",  "FP",  "IP",  "SP",  "LR",  "PC" }},
  { "coproc<N>=(cde|generic)", N_("Enable CDE extensions for coprocessor N space"), { NULL } }
};

static const char *const iwmmxt_wwnames[] =
{"b", "h", "w", "d"};

static const char *const iwmmxt_wwssnames[] =
{"b", "bus", "bc", "bss",
 "h", "hus", "hc", "hss",
 "w", "wus", "wc", "wss",
 "d", "dus", "dc", "dss"
};

static const char *const iwmmxt_regnames[] =
{ "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7",
  "wr8", "wr9", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15"
};

static const char *const iwmmxt_cregnames[] =
{ "wcid", "wcon", "wcssf", "wcasf", "reserved", "reserved", "reserved", "reserved",
  "wcgr0", "wcgr1", "wcgr2", "wcgr3", "reserved", "reserved", "reserved", "reserved"
};

static const char *const vec_condnames[] =
{ "eq", "ne", "cs", "hi", "ge", "lt", "gt", "le"
};

static const char *const mve_predicatenames[] =
{ "", "ttt", "tt", "tte", "t", "tee", "te", "tet", "",
  "eee", "ee", "eet", "e", "ett", "et", "ete"
};

/* Names for 2-bit size field for mve vector isntructions.  */
static const char *const mve_vec_sizename[] =
  { "8", "16", "32", "64"};

/* Indicates whether we are processing a then predicate,
   else predicate or none at all.  */
enum vpt_pred_state
{
  PRED_NONE,
  PRED_THEN,
  PRED_ELSE
};

/* Information used to process a vpt block and subsequent instructions.  */
struct vpt_block
{
  /* Are we in a vpt block.  */
  bool in_vpt_block;

  /* Next predicate state if in vpt block.  */
  enum vpt_pred_state next_pred_state;

  /* Mask from vpt/vpst instruction.  */
  long predicate_mask;

  /* Instruction number in vpt block.  */
  long current_insn_num;

  /* Number of instructions in vpt block..   */
  long num_pred_insn;
};

static struct vpt_block vpt_block_state =
{
  false,
  PRED_NONE,
  0,
  0,
  0
};

/* Default to GCC register name set.  */
static unsigned int regname_selected = 1;

#define NUM_ARM_OPTIONS   ARRAY_SIZE (regnames)
#define arm_regnames      regnames[regname_selected].reg_names

static bool force_thumb = false;
static uint16_t cde_coprocs = 0;

/* Current IT instruction state.  This contains the same state as the IT
   bits in the CPSR.  */
static unsigned int ifthen_state;
/* IT state for the next instruction.  */
static unsigned int ifthen_next_state;
/* The address of the insn for which the IT state is valid.  */
static bfd_vma ifthen_address;
#define IFTHEN_COND ((ifthen_state >> 4) & 0xf)
/* Indicates that the current Conditional state is unconditional or outside
   an IT block.  */
#define COND_UNCOND 16


/* Functions.  */
/* Extract the predicate mask for a VPT or VPST instruction.
   The mask is composed of bits 13-15 (Mkl) and bit 22 (Mkh).  */

static long
mve_extract_pred_mask (long given)
{
  return ((given & 0x00400000) >> 19) | ((given & 0xe000) >> 13);
}

/* Return the number of instructions in a MVE predicate block.  */
static long
num_instructions_vpt_block (long given)
{
  long mask = mve_extract_pred_mask (given);
  if (mask == 0)
    return 0;

  if (mask == 8)
    return 1;

  if ((mask & 7) == 4)
    return 2;

  if ((mask & 3) == 2)
    return 3;

  if ((mask & 1) == 1)
    return 4;

  return 0;
}

static void
mark_outside_vpt_block (void)
{
  vpt_block_state.in_vpt_block = false;
  vpt_block_state.next_pred_state = PRED_NONE;
  vpt_block_state.predicate_mask = 0;
  vpt_block_state.current_insn_num = 0;
  vpt_block_state.num_pred_insn = 0;
}

static void
mark_inside_vpt_block (long given)
{
  vpt_block_state.in_vpt_block = true;
  vpt_block_state.next_pred_state = PRED_THEN;
  vpt_block_state.predicate_mask = mve_extract_pred_mask (given);
  vpt_block_state.current_insn_num = 0;
  vpt_block_state.num_pred_insn = num_instructions_vpt_block (given);
  assert (vpt_block_state.num_pred_insn >= 1);
}

static enum vpt_pred_state
invert_next_predicate_state (enum vpt_pred_state astate)
{
  if (astate == PRED_THEN)
    return PRED_ELSE;
  else if (astate == PRED_ELSE)
    return PRED_THEN;
  else
    return PRED_NONE;
}

static enum vpt_pred_state
update_next_predicate_state (void)
{
  long pred_mask = vpt_block_state.predicate_mask;
  long mask_for_insn = 0;

  switch (vpt_block_state.current_insn_num)
    {
    case 1:
      mask_for_insn = 8;
      break;

    case 2:
      mask_for_insn = 4;
      break;

    case 3:
      mask_for_insn = 2;
      break;

    case 4:
      return PRED_NONE;
    }

  if (pred_mask & mask_for_insn)
    return invert_next_predicate_state (vpt_block_state.next_pred_state);
  else
    return vpt_block_state.next_pred_state;
}

static void
update_vpt_block_state (void)
{
  vpt_block_state.current_insn_num++;
  if (vpt_block_state.current_insn_num == vpt_block_state.num_pred_insn)
    {
      /* No more instructions to process in vpt block.  */
      mark_outside_vpt_block ();
      return;
    }

  vpt_block_state.next_pred_state = update_next_predicate_state ();
}

/* Decode a bitfield of the form matching regexp (N(-N)?,)*N(-N)?.
   Returns pointer to following character of the format string and
   fills in *VALUEP and *WIDTHP with the extracted value and number of
   bits extracted.  WIDTHP can be NULL.  */

static const char *
arm_decode_bitfield (const char *ptr,
		     unsigned long insn,
		     unsigned long *valuep,
		     int *widthp)
{
  unsigned long value = 0;
  int width = 0;

  do
    {
      int start, end;
      int bits;

      for (start = 0; *ptr >= '0' && *ptr <= '9'; ptr++)
	start = start * 10 + *ptr - '0';
      if (*ptr == '-')
	for (end = 0, ptr++; *ptr >= '0' && *ptr <= '9'; ptr++)
	  end = end * 10 + *ptr - '0';
      else
	end = start;
      bits = end - start;
      if (bits < 0)
	abort ();
      value |= ((insn >> start) & ((2ul << bits) - 1)) << width;
      width += bits + 1;
    }
  while (*ptr++ == ',');
  *valuep = value;
  if (widthp)
    *widthp = width;
  return ptr - 1;
}

static void
arm_decode_shift (long given, fprintf_styled_ftype func, void *stream,
		  bool print_shift)
{
  func (stream, dis_style_register, "%s", arm_regnames[given & 0xf]);

  if ((given & 0xff0) != 0)
    {
      if ((given & 0x10) == 0)
	{
	  int amount = (given & 0xf80) >> 7;
	  int shift = (given & 0x60) >> 5;

	  if (amount == 0)
	    {
	      if (shift == 3)
		{
		  func (stream, dis_style_text, ", ");
		  func (stream, dis_style_sub_mnemonic, "rrx");
		  return;
		}

	      amount = 32;
	    }

	  if (print_shift)
	    {
	      func (stream, dis_style_text, ", ");
	      func (stream, dis_style_sub_mnemonic, "%s ", arm_shift[shift]);
	      func (stream, dis_style_immediate, "#%d", amount);
	    }
	  else
	    {
	      func (stream, dis_style_text, ", ");
	      func (stream, dis_style_immediate, "#%d", amount);
	    }
	}
      else if ((given & 0x80) == 0x80)
	func (stream, dis_style_comment_start,
	      "\t@ <illegal shifter operand>");
      else if (print_shift)
	{
	  func (stream, dis_style_text, ", ");
	  func (stream, dis_style_sub_mnemonic, "%s ",
		arm_shift[(given & 0x60) >> 5]);
	  func (stream, dis_style_register, "%s",
		arm_regnames[(given & 0xf00) >> 8]);
	}
      else
	{
	  func (stream, dis_style_text, ", ");
	  func (stream, dis_style_register, "%s",
		arm_regnames[(given & 0xf00) >> 8]);
	}
    }
}

/* Return TRUE if the MATCHED_INSN can be inside an IT block.  */

static bool
is_mve_okay_in_it (enum mve_instructions matched_insn)
{
  switch (matched_insn)
    {
    case MVE_VMOV_GP_TO_VEC_LANE:
    case MVE_VMOV2_VEC_LANE_TO_GP:
    case MVE_VMOV2_GP_TO_VEC_LANE:
    case MVE_VMOV_VEC_LANE_TO_GP:
    case MVE_LSLL:
    case MVE_LSLLI:
    case MVE_LSRL:
    case MVE_ASRL:
    case MVE_ASRLI:
    case MVE_SQRSHRL:
    case MVE_SQRSHR:
    case MVE_UQRSHL:
    case MVE_UQRSHLL:
    case MVE_UQSHL:
    case MVE_UQSHLL:
    case MVE_URSHRL:
    case MVE_URSHR:
    case MVE_SRSHRL:
    case MVE_SRSHR:
    case MVE_SQSHLL:
    case MVE_SQSHL:
      return true;
    default:
      return false;
    }
}

static bool
is_mve_architecture (struct disassemble_info *info)
{
  struct arm_private_data *private_data = info->private_data;
  arm_feature_set allowed_arches = private_data->features;

  arm_feature_set arm_ext_v8_1m_main
    = ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN);

  if (ARM_CPU_HAS_FEATURE (arm_ext_v8_1m_main, allowed_arches)
      && !ARM_CPU_IS_ANY (allowed_arches))
    return true;
  else
    return false;
}

static bool
is_vpt_instruction (long given)
{

  /* If mkh:mkl is '0000' then its not a vpt/vpst instruction.  */
  if ((given & 0x0040e000) == 0)
    return false;

  /* VPT floating point T1 variant.  */
  if (((given & 0xefb10f50) == 0xee310f00 && ((given & 0x1001) != 0x1))
  /* VPT floating point T2 variant.  */
      || ((given & 0xefb10f50) == 0xee310f40)
  /* VPT vector T1 variant.  */
      || ((given & 0xff811f51) == 0xfe010f00)
  /* VPT vector T2 variant.  */
      || ((given & 0xff811f51) == 0xfe010f01
	  && ((given & 0x300000) != 0x300000))
  /* VPT vector T3 variant.  */
      || ((given & 0xff811f50) == 0xfe011f00)
  /* VPT vector T4 variant.  */
      || ((given & 0xff811f70) == 0xfe010f40)
  /* VPT vector T5 variant.  */
      || ((given & 0xff811f70) == 0xfe010f60)
  /* VPT vector T6 variant.  */
      || ((given & 0xff811f50) == 0xfe011f40)
  /* VPST vector T variant.  */
      || ((given & 0xffbf1fff) == 0xfe310f4d))
    return true;
  else
    return false;
}

/* Decode a bitfield from opcode GIVEN, with starting bitfield = START
   and ending bitfield = END.  END must be greater than START.  */

static unsigned long
arm_decode_field (unsigned long given, unsigned int start, unsigned int end)
{
  int bits = end - start;

  if (bits < 0)
    abort ();

  return ((given >> start) & ((2ul << bits) - 1));
}

/* Decode a bitfield from opcode GIVEN, with multiple bitfields:
   START:END and START2:END2.  END/END2 must be greater than
   START/START2.  */

static unsigned long
arm_decode_field_multiple (unsigned long given, unsigned int start,
			   unsigned int end, unsigned int start2,
			   unsigned int end2)
{
  int bits = end - start;
  int bits2 = end2 - start2;
  unsigned long value = 0;
  int width = 0;

  if (bits2 < 0)
    abort ();

  value = arm_decode_field (given, start, end);
  width += bits + 1;

  value |= ((given >> start2) & ((2ul << bits2) - 1)) << width;
  return value;
}

/* Return TRUE if the GIVEN encoding should not be decoded as MATCHED_INSN.
   This helps us decode instructions that change mnemonic depending on specific
   operand values/encodings.  */

static bool
is_mve_encoding_conflict (unsigned long given,
			  enum mve_instructions matched_insn)
{
  switch (matched_insn)
    {
    case MVE_VPST:
      if (arm_decode_field_multiple (given, 13, 15, 22, 22) == 0)
	return true;
      else
	return false;

    case MVE_VPT_FP_T1:
      if (arm_decode_field_multiple (given, 13, 15, 22, 22) == 0)
	return true;
      if ((arm_decode_field (given, 12, 12) == 0)
	  && (arm_decode_field (given, 0, 0) == 1))
	return true;
      return false;

    case MVE_VPT_FP_T2:
      if (arm_decode_field_multiple (given, 13, 15, 22, 22) == 0)
	return true;
      if (arm_decode_field (given, 0, 3) == 0xd)
	return true;
      return false;

    case MVE_VPT_VEC_T1:
    case MVE_VPT_VEC_T2:
    case MVE_VPT_VEC_T3:
    case MVE_VPT_VEC_T4:
    case MVE_VPT_VEC_T5:
    case MVE_VPT_VEC_T6:
      if (arm_decode_field_multiple (given, 13, 15, 22, 22) == 0)
	return true;
      if (arm_decode_field (given, 20, 21) == 3)
	return true;
      return false;

    case MVE_VCMP_FP_T1:
      if ((arm_decode_field (given, 12, 12) == 0)
	  && (arm_decode_field (given, 0, 0) == 1))
	return true;
      else
	return false;

    case MVE_VCMP_FP_T2:
      if (arm_decode_field (given, 0, 3) == 0xd)
	return true;
      else
	return false;

    case MVE_VQADD_T2:
    case MVE_VQSUB_T2:
    case MVE_VMUL_VEC_T2:
    case MVE_VMULH:
    case MVE_VRMULH:
    case MVE_VMLA:
    case MVE_VMAX:
    case MVE_VMIN:
    case MVE_VBRSR:
    case MVE_VADD_VEC_T2:
    case MVE_VSUB_VEC_T2:
    case MVE_VABAV:
    case MVE_VQRSHL_T1:
    case MVE_VQSHL_T4:
    case MVE_VRSHL_T1:
    case MVE_VSHL_T3:
    case MVE_VCADD_VEC:
    case MVE_VHCADD:
    case MVE_VDDUP:
    case MVE_VIDUP:
    case MVE_VQRDMLADH:
    case MVE_VQDMLAH:
    case MVE_VQRDMLAH:
    case MVE_VQDMLASH:
    case MVE_VQRDMLASH:
    case MVE_VQDMLSDH:
    case MVE_VQRDMLSDH:
    case MVE_VQDMULH_T3:
    case MVE_VQRDMULH_T4:
    case MVE_VQDMLADH:
    case MVE_VMLAS:
    case MVE_VMULL_INT:
    case MVE_VHADD_T2:
    case MVE_VHSUB_T2:
    case MVE_VCMP_VEC_T1:
    case MVE_VCMP_VEC_T2:
    case MVE_VCMP_VEC_T3:
    case MVE_VCMP_VEC_T4:
    case MVE_VCMP_VEC_T5:
    case MVE_VCMP_VEC_T6:
      if (arm_decode_field (given, 20, 21) == 3)
	return true;
      else
	return false;

    case MVE_VLD2:
    case MVE_VLD4:
    case MVE_VST2:
    case MVE_VST4:
      if (arm_decode_field (given, 7, 8) == 3)
	return true;
      else
	return false;

    case MVE_VSTRB_T1:
    case MVE_VSTRH_T2:
      if ((arm_decode_field (given, 24, 24) == 0)
	  && (arm_decode_field (given, 21, 21) == 0))
	{
	    return true;
	}
      else if ((arm_decode_field (given, 7, 8) == 3))
	return true;
      else
	return false;

    case MVE_VLDRB_T1:
    case MVE_VLDRH_T2:
    case MVE_VLDRW_T7:
    case MVE_VSTRB_T5:
    case MVE_VSTRH_T6:
    case MVE_VSTRW_T7:
      if ((arm_decode_field (given, 24, 24) == 0)
	  && (arm_decode_field (given, 21, 21) == 0))
	{
	    return true;
	}
      else
	return false;

    case MVE_VCVT_FP_FIX_VEC:
      return (arm_decode_field (given, 16, 21) & 0x38) == 0;

    case MVE_VBIC_IMM:
    case MVE_VORR_IMM:
      {
	unsigned long cmode = arm_decode_field (given, 8, 11);

	if ((cmode & 1) == 0)
	  return true;
	else if ((cmode & 0xc) == 0xc)
	  return true;
	else
	  return false;
      }

    case MVE_VMVN_IMM:
      {
	unsigned long cmode = arm_decode_field (given, 8, 11);

	if (cmode == 0xe)
	  return true;
	else if ((cmode & 0x9) == 1)
	  return true;
	else if ((cmode & 0xd) == 9)
	  return true;
	else
	  return false;
      }

    case MVE_VMOV_IMM_TO_VEC:
      if ((arm_decode_field (given, 5, 5) == 1)
	  && (arm_decode_field (given, 8, 11) != 0xe))
	return true;
      else
	return false;

    case MVE_VMOVL:
      {
	unsigned long size = arm_decode_field (given, 19, 20);
	if ((size == 0) || (size == 3))
	  return true;
	else
	  return false;
      }

    case MVE_VMAXA:
    case MVE_VMINA:
    case MVE_VMAXV:
    case MVE_VMAXAV:
    case MVE_VMINV:
    case MVE_VMINAV:
    case MVE_VQRSHL_T2:
    case MVE_VQSHL_T1:
    case MVE_VRSHL_T2:
    case MVE_VSHL_T2:
    case MVE_VSHLL_T2:
    case MVE_VADDV:
    case MVE_VMOVN:
    case MVE_VQMOVUN:
    case MVE_VQMOVN:
      if (arm_decode_field (given, 18, 19) == 3)
	return true;
      else
	return false;

    case MVE_VMLSLDAV:
    case MVE_VRMLSLDAVH:
    case MVE_VMLALDAV:
    case MVE_VADDLV:
      if (arm_decode_field (given, 20, 22) == 7)
	return true;
      else
	return false;

    case MVE_VRMLALDAVH:
      if ((arm_decode_field (given, 20, 22) & 6) == 6)
	return true;
      else
	return false;

    case MVE_VDWDUP:
    case MVE_VIWDUP:
      if ((arm_decode_field (given, 20, 21) == 3)
	  || (arm_decode_field (given, 1, 3) == 7))
	return true;
      else
	return false;


    case MVE_VSHLL_T1:
      if (arm_decode_field (given, 16, 18) == 0)
	{
	  unsigned long sz = arm_decode_field (given, 19, 20);

	  if ((sz == 1) || (sz == 2))
	    return true;
	  else
	    return false;
	}
      else
	return false;

    case MVE_VQSHL_T2:
    case MVE_VQSHLU_T3:
    case MVE_VRSHR:
    case MVE_VSHL_T1:
    case MVE_VSHR:
    case MVE_VSLI:
    case MVE_VSRI:
      if (arm_decode_field (given, 19, 21) == 0)
	return true;
      else
	return false;

    case MVE_VCTP:
    if (arm_decode_field (given, 16, 19) == 0xf)
      return true;
    else
      return false;

    case MVE_ASRLI:
    case MVE_ASRL:
    case MVE_LSLLI:
    case MVE_LSLL:
    case MVE_LSRL:
    case MVE_SQRSHRL:
    case MVE_SQSHLL:
    case MVE_SRSHRL:
    case MVE_UQRSHLL:
    case MVE_UQSHLL:
    case MVE_URSHRL:
      if (arm_decode_field (given, 9, 11) == 0x7)
	return true;
      else
	return false;

    case MVE_CSINC:
    case MVE_CSINV:
      {
	unsigned long rm, rn;
	rm = arm_decode_field (given, 0, 3);
	rn = arm_decode_field (given, 16, 19);
	/* CSET/CSETM.  */
	if (rm == 0xf && rn == 0xf)
	  return true;
	/* CINC/CINV.  */
	else if (rn == rm && rn != 0xf)
	  return true;
      }
    /* Fall through.  */
    case MVE_CSEL:
    case MVE_CSNEG:
      if (arm_decode_field (given, 0, 3) == 0xd)
	return true;
      /* CNEG.  */
      else if (matched_insn == MVE_CSNEG)
	if (arm_decode_field (given, 0, 3) == arm_decode_field (given, 16, 19))
	  return true;
      return false;

    default:
    case MVE_VADD_FP_T1:
    case MVE_VADD_FP_T2:
    case MVE_VADD_VEC_T1:
      return false;

    }
}

static void
print_mve_vld_str_addr (struct disassemble_info *info,
			unsigned long given,
			enum mve_instructions matched_insn)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  unsigned long p, w, gpr, imm, add, mod_imm;

  imm = arm_decode_field (given, 0, 6);
  mod_imm = imm;

  switch (matched_insn)
    {
    case MVE_VLDRB_T1:
    case MVE_VSTRB_T1:
      gpr = arm_decode_field (given, 16, 18);
      break;

    case MVE_VLDRH_T2:
    case MVE_VSTRH_T2:
      gpr = arm_decode_field (given, 16, 18);
      mod_imm = imm << 1;
      break;

    case MVE_VLDRH_T6:
    case MVE_VSTRH_T6:
      gpr = arm_decode_field (given, 16, 19);
      mod_imm = imm << 1;
      break;

    case MVE_VLDRW_T7:
    case MVE_VSTRW_T7:
      gpr = arm_decode_field (given, 16, 19);
      mod_imm = imm << 2;
      break;

    case MVE_VLDRB_T5:
    case MVE_VSTRB_T5:
      gpr = arm_decode_field (given, 16, 19);
      break;

    default:
      return;
    }

  p = arm_decode_field (given, 24, 24);
  w = arm_decode_field (given, 21, 21);

  add = arm_decode_field (given, 23, 23);

  char * add_sub;

  /* Don't print anything for '+' as it is implied.  */
  if (add == 1)
    add_sub = "";
  else
    add_sub = "-";

  func (stream, dis_style_text, "[");
  func (stream, dis_style_register, "%s", arm_regnames[gpr]);
  if (p == 1)
    {
      func (stream, dis_style_text, ", ");
      func (stream, dis_style_immediate, "#%s%lu", add_sub, mod_imm);
      /* Offset mode.  */
      if (w == 0)
	func (stream, dis_style_text, "]");
      /* Pre-indexed mode.  */
      else
	func (stream, dis_style_text, "]!");
    }
  else if ((p == 0) && (w == 1))
    {
      /* Post-index mode.  */
      func (stream, dis_style_text, "], ");
      func (stream, dis_style_immediate, "#%s%lu", add_sub, mod_imm);
    }
}

/* Return FALSE if GIVEN is not an undefined encoding for MATCHED_INSN.
   Otherwise, return TRUE and set UNDEFINED_CODE to give a reason as to why
   this encoding is undefined.  */

static bool
is_mve_undefined (unsigned long given, enum mve_instructions matched_insn,
		  enum mve_undefined *undefined_code)
{
  *undefined_code = UNDEF_NONE;

  switch (matched_insn)
    {
    case MVE_VDUP:
      if (arm_decode_field_multiple (given, 5, 5, 22, 22) == 3)
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}
      else
	return false;

    case MVE_VQADD_T1:
    case MVE_VQSUB_T1:
    case MVE_VMUL_VEC_T1:
    case MVE_VABD_VEC:
    case MVE_VADD_VEC_T1:
    case MVE_VSUB_VEC_T1:
    case MVE_VQDMULH_T1:
    case MVE_VQRDMULH_T2:
    case MVE_VRHADD:
    case MVE_VHADD_T1:
    case MVE_VHSUB_T1:
      if (arm_decode_field (given, 20, 21) == 3)
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}
      else
	return false;

    case MVE_VLDRB_T1:
      if (arm_decode_field (given, 7, 8) == 3)
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}
      else
	return false;

    case MVE_VLDRH_T2:
      if (arm_decode_field (given, 7, 8) <= 1)
	{
	  *undefined_code = UNDEF_SIZE_LE_1;
	  return true;
	}
      else
	return false;

    case MVE_VSTRB_T1:
      if ((arm_decode_field (given, 7, 8) == 0))
	{
	  *undefined_code = UNDEF_SIZE_0;
	  return true;
	}
      else
	return false;

    case MVE_VSTRH_T2:
      if ((arm_decode_field (given, 7, 8) <= 1))
	{
	  *undefined_code = UNDEF_SIZE_LE_1;
	  return true;
	}
      else
	return false;

    case MVE_VLDRB_GATHER_T1:
      if (arm_decode_field (given, 7, 8) == 3)
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}
      else if ((arm_decode_field (given, 28, 28) == 0)
	       && (arm_decode_field (given, 7, 8) == 0))
	{
	  *undefined_code = UNDEF_NOT_UNS_SIZE_0;
	  return true;
	}
      else
	return false;

    case MVE_VLDRH_GATHER_T2:
      if (arm_decode_field (given, 7, 8) == 3)
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}
      else if ((arm_decode_field (given, 28, 28) == 0)
	       && (arm_decode_field (given, 7, 8) == 1))
	{
	  *undefined_code = UNDEF_NOT_UNS_SIZE_1;
	  return true;
	}
      else if (arm_decode_field (given, 7, 8) == 0)
	{
	  *undefined_code = UNDEF_SIZE_0;
	  return true;
	}
      else
	return false;

    case MVE_VLDRW_GATHER_T3:
      if (arm_decode_field (given, 7, 8) != 2)
	{
	  *undefined_code = UNDEF_SIZE_NOT_2;
	  return true;
	}
      else if (arm_decode_field (given, 28, 28) == 0)
	{
	  *undefined_code = UNDEF_NOT_UNSIGNED;
	  return true;
	}
      else
	return false;

    case MVE_VLDRD_GATHER_T4:
      if (arm_decode_field (given, 7, 8) != 3)
	{
	  *undefined_code = UNDEF_SIZE_NOT_3;
	  return true;
	}
      else if (arm_decode_field (given, 28, 28) == 0)
	{
	  *undefined_code = UNDEF_NOT_UNSIGNED;
	  return true;
	}
      else
	return false;

    case MVE_VSTRB_SCATTER_T1:
      if (arm_decode_field (given, 7, 8) == 3)
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}
      else
	return false;

    case MVE_VSTRH_SCATTER_T2:
      {
	unsigned long size = arm_decode_field (given, 7, 8);
	if (size == 3)
	  {
	    *undefined_code = UNDEF_SIZE_3;
	    return true;
	  }
	else if (size == 0)
	  {
	    *undefined_code = UNDEF_SIZE_0;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VSTRW_SCATTER_T3:
      if (arm_decode_field (given, 7, 8) != 2)
	{
	  *undefined_code = UNDEF_SIZE_NOT_2;
	  return true;
	}
      else
	return false;

    case MVE_VSTRD_SCATTER_T4:
      if (arm_decode_field (given, 7, 8) != 3)
	{
	  *undefined_code = UNDEF_SIZE_NOT_3;
	  return true;
	}
      else
	return false;

    case MVE_VCVT_FP_FIX_VEC:
      {
	unsigned long imm6 = arm_decode_field (given, 16, 21);
	if ((imm6 & 0x20) == 0)
	  {
	    *undefined_code = UNDEF_VCVT_IMM6;
	    return true;
	  }

	if ((arm_decode_field (given, 9, 9) == 0)
	    && ((imm6 & 0x30) == 0x20))
	  {
	    *undefined_code = UNDEF_VCVT_FSI_IMM6;
	    return true;
	  }

	return false;
      }

    case MVE_VNEG_FP:
    case MVE_VABS_FP:
    case MVE_VCVT_BETWEEN_FP_INT:
    case MVE_VCVT_FROM_FP_TO_INT:
      {
	unsigned long size = arm_decode_field (given, 18, 19);
	if (size == 0)
	  {
	    *undefined_code = UNDEF_SIZE_0;
	    return true;
	  }
	else if (size == 3)
	  {
	    *undefined_code = UNDEF_SIZE_3;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VMOV_VEC_LANE_TO_GP:
      {
	unsigned long op1 = arm_decode_field (given, 21, 22);
	unsigned long op2 = arm_decode_field (given, 5, 6);
	unsigned long u = arm_decode_field (given, 23, 23);

	if ((op2 == 0) && (u == 1))
	  {
	    if ((op1 == 0) || (op1 == 1))
	      {
		*undefined_code = UNDEF_BAD_U_OP1_OP2;
		return true;
	      }
	    else
	      return false;
	  }
	else if (op2 == 2)
	  {
	    if ((op1 == 0) || (op1 == 1))
	      {
		*undefined_code = UNDEF_BAD_OP1_OP2;
		return true;
	      }
	    else
	      return false;
	  }

	return false;
      }

    case MVE_VMOV_GP_TO_VEC_LANE:
      if (arm_decode_field (given, 5, 6) == 2)
	{
	  unsigned long op1 = arm_decode_field (given, 21, 22);
	  if ((op1 == 0) || (op1 == 1))
	    {
	      *undefined_code = UNDEF_BAD_OP1_OP2;
	      return true;
	    }
	  else
	    return false;
	}
      else
	return false;

    case MVE_VMOV_VEC_TO_VEC:
      if ((arm_decode_field (given, 5, 5) == 1)
	  || (arm_decode_field (given, 22, 22) == 1))
	  return true;
      return false;

    case MVE_VMOV_IMM_TO_VEC:
      if (arm_decode_field (given, 5, 5) == 0)
      {
	unsigned long cmode = arm_decode_field (given, 8, 11);

	if (((cmode & 9) == 1) || ((cmode & 5) == 1))
	  {
	    *undefined_code = UNDEF_OP_0_BAD_CMODE;
	    return true;
	  }
	else
	  return false;
      }
      else
	return false;

    case MVE_VSHLL_T2:
    case MVE_VMOVN:
      if (arm_decode_field (given, 18, 19) == 2)
	{
	  *undefined_code = UNDEF_SIZE_2;
	  return true;
	}
      else
	return false;

    case MVE_VRMLALDAVH:
    case MVE_VMLADAV_T1:
    case MVE_VMLADAV_T2:
    case MVE_VMLALDAV:
      if ((arm_decode_field (given, 28, 28) == 1)
	  && (arm_decode_field (given, 12, 12) == 1))
	{
	  *undefined_code = UNDEF_XCHG_UNS;
	  return true;
	}
      else
	return false;

    case MVE_VQSHRN:
    case MVE_VQSHRUN:
    case MVE_VSHLL_T1:
    case MVE_VSHRN:
      {
	unsigned long sz = arm_decode_field (given, 19, 20);
	if (sz == 1)
	  return false;
	else if ((sz & 2) == 2)
	  return false;
	else
	  {
	    *undefined_code = UNDEF_SIZE;
	    return true;
	  }
      }
      break;

    case MVE_VQSHL_T2:
    case MVE_VQSHLU_T3:
    case MVE_VRSHR:
    case MVE_VSHL_T1:
    case MVE_VSHR:
    case MVE_VSLI:
    case MVE_VSRI:
      {
	unsigned long sz = arm_decode_field (given, 19, 21);
	if ((sz & 7) == 1)
	  return false;
	else if ((sz & 6) == 2)
	  return false;
	else if ((sz & 4) == 4)
	  return false;
	else
	  {
	    *undefined_code = UNDEF_SIZE;
	    return true;
	  }
      }

    case MVE_VQRSHRN:
    case MVE_VQRSHRUN:
      if (arm_decode_field (given, 19, 20) == 0)
	{
	  *undefined_code = UNDEF_SIZE_0;
	  return true;
	}
      else
	return false;

    case MVE_VABS_VEC:
	if (arm_decode_field (given, 18, 19) == 3)
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}
	else
	  return false;

    case MVE_VQNEG:
    case MVE_VQABS:
    case MVE_VNEG_VEC:
    case MVE_VCLS:
    case MVE_VCLZ:
      if (arm_decode_field (given, 18, 19) == 3)
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}
      else
	return false;

    case MVE_VREV16:
      if (arm_decode_field (given, 18, 19) == 0)
	return false;
      else
	{
	  *undefined_code = UNDEF_SIZE_NOT_0;
	  return true;
	}

    case MVE_VREV32:
      {
	unsigned long size = arm_decode_field (given, 18, 19);
	if ((size & 2) == 2)
	  {
	    *undefined_code = UNDEF_SIZE_2;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VREV64:
      if (arm_decode_field (given, 18, 19) != 3)
	return false;
      else
	{
	  *undefined_code = UNDEF_SIZE_3;
	  return true;
	}

    default:
      return false;
    }
}

/* Return FALSE if GIVEN is not an unpredictable encoding for MATCHED_INSN.
   Otherwise, return TRUE and set UNPREDICTABLE_CODE to give a reason as to
   why this encoding is unpredictable.  */

static bool
is_mve_unpredictable (unsigned long given, enum mve_instructions matched_insn,
		      enum mve_unpredictable *unpredictable_code)
{
  *unpredictable_code = UNPRED_NONE;

  switch (matched_insn)
    {
    case MVE_VCMP_FP_T2:
    case MVE_VPT_FP_T2:
      if ((arm_decode_field (given, 12, 12) == 0)
	  && (arm_decode_field (given, 5, 5) == 1))
	{
	  *unpredictable_code = UNPRED_FCA_0_FCB_1;
	  return true;
	}
      else
	return false;

    case MVE_VPT_VEC_T4:
    case MVE_VPT_VEC_T5:
    case MVE_VPT_VEC_T6:
    case MVE_VCMP_VEC_T4:
    case MVE_VCMP_VEC_T5:
    case MVE_VCMP_VEC_T6:
      if (arm_decode_field (given, 0, 3) == 0xd)
	{
	  *unpredictable_code = UNPRED_R13;
	  return true;
	}
      else
	return false;

    case MVE_VDUP:
      {
	unsigned long gpr = arm_decode_field (given, 12, 15);
	if (gpr == 0xd)
	  {
	    *unpredictable_code = UNPRED_R13;
	    return true;
	  }
	else if (gpr == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }

	return false;
      }

    case MVE_VQADD_T2:
    case MVE_VQSUB_T2:
    case MVE_VMUL_FP_T2:
    case MVE_VMUL_VEC_T2:
    case MVE_VMLA:
    case MVE_VBRSR:
    case MVE_VADD_FP_T2:
    case MVE_VSUB_FP_T2:
    case MVE_VADD_VEC_T2:
    case MVE_VSUB_VEC_T2:
    case MVE_VQRSHL_T2:
    case MVE_VQSHL_T1:
    case MVE_VRSHL_T2:
    case MVE_VSHL_T2:
    case MVE_VSHLC:
    case MVE_VQDMLAH:
    case MVE_VQRDMLAH:
    case MVE_VQDMLASH:
    case MVE_VQRDMLASH:
    case MVE_VQDMULH_T3:
    case MVE_VQRDMULH_T4:
    case MVE_VMLAS:
    case MVE_VFMA_FP_SCALAR:
    case MVE_VFMAS_FP_SCALAR:
    case MVE_VHADD_T2:
    case MVE_VHSUB_T2:
      {
	unsigned long gpr = arm_decode_field (given, 0, 3);
	if (gpr == 0xd)
	  {
	    *unpredictable_code = UNPRED_R13;
	    return true;
	  }
	else if (gpr == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }

	return false;
      }

    case MVE_VLD2:
    case MVE_VST2:
      {
	unsigned long rn = arm_decode_field (given, 16, 19);

	if ((rn == 0xd) && (arm_decode_field (given, 21, 21) == 1))
	  {
	    *unpredictable_code = UNPRED_R13_AND_WB;
	    return true;
	  }

	if (rn == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }

	if (arm_decode_field_multiple (given, 13, 15, 22, 22) > 6)
	  {
	    *unpredictable_code = UNPRED_Q_GT_6;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VLD4:
    case MVE_VST4:
      {
	unsigned long rn = arm_decode_field (given, 16, 19);

	if ((rn == 0xd) && (arm_decode_field (given, 21, 21) == 1))
	  {
	    *unpredictable_code = UNPRED_R13_AND_WB;
	    return true;
	  }

	if (rn == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }

	if (arm_decode_field_multiple (given, 13, 15, 22, 22) > 4)
	  {
	    *unpredictable_code = UNPRED_Q_GT_4;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VLDRB_T5:
    case MVE_VLDRH_T6:
    case MVE_VLDRW_T7:
    case MVE_VSTRB_T5:
    case MVE_VSTRH_T6:
    case MVE_VSTRW_T7:
      {
	unsigned long rn = arm_decode_field (given, 16, 19);

	if ((rn == 0xd) && (arm_decode_field (given, 21, 21) == 1))
	  {
	    *unpredictable_code = UNPRED_R13_AND_WB;
	    return true;
	  }
	else if (rn == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VLDRB_GATHER_T1:
      if (arm_decode_field (given, 0, 0) == 1)
	{
	  *unpredictable_code = UNPRED_OS;
	  return true;
	}

      /*  fall through.  */
      /* To handle common code with T2-T4 variants.  */
    case MVE_VLDRH_GATHER_T2:
    case MVE_VLDRW_GATHER_T3:
    case MVE_VLDRD_GATHER_T4:
      {
	unsigned long qd = arm_decode_field_multiple (given, 13, 15, 22, 22);
	unsigned long qm = arm_decode_field_multiple (given, 1, 3, 5, 5);

	if (qd == qm)
	  {
	    *unpredictable_code = UNPRED_Q_REGS_EQUAL;
	    return true;
	  }

	if (arm_decode_field (given, 16, 19) == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }

	return false;
      }

    case MVE_VLDRW_GATHER_T5:
    case MVE_VLDRD_GATHER_T6:
      {
	unsigned long qd = arm_decode_field_multiple (given, 13, 15, 22, 22);
	unsigned long qm = arm_decode_field_multiple (given, 17, 19, 7, 7);

	if (qd == qm)
	  {
	    *unpredictable_code = UNPRED_Q_REGS_EQUAL;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VSTRB_SCATTER_T1:
      if (arm_decode_field (given, 16, 19) == 0xf)
	{
	  *unpredictable_code = UNPRED_R15;
	  return true;
	}
      else if (arm_decode_field (given, 0, 0) == 1)
	{
	  *unpredictable_code = UNPRED_OS;
	  return true;
	}
      else
	return false;

    case MVE_VSTRH_SCATTER_T2:
    case MVE_VSTRW_SCATTER_T3:
    case MVE_VSTRD_SCATTER_T4:
      if (arm_decode_field (given, 16, 19) == 0xf)
	{
	  *unpredictable_code = UNPRED_R15;
	  return true;
	}
      else
	return false;

    case MVE_VMOV2_VEC_LANE_TO_GP:
    case MVE_VMOV2_GP_TO_VEC_LANE:
    case MVE_VCVT_BETWEEN_FP_INT:
    case MVE_VCVT_FROM_FP_TO_INT:
      {
	unsigned long rt = arm_decode_field (given, 0, 3);
	unsigned long rt2 = arm_decode_field (given, 16, 19);

	if ((rt == 0xd) || (rt2 == 0xd))
	  {
	    *unpredictable_code = UNPRED_R13;
	    return true;
	  }
	else if ((rt == 0xf) || (rt2 == 0xf))
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }
	else if (rt == rt2 && matched_insn != MVE_VMOV2_GP_TO_VEC_LANE)
	  {
	    *unpredictable_code = UNPRED_GP_REGS_EQUAL;
	    return true;
	  }

	return false;
      }

    case MVE_VMAXV:
    case MVE_VMAXAV:
    case MVE_VMAXNMV_FP:
    case MVE_VMAXNMAV_FP:
    case MVE_VMINNMV_FP:
    case MVE_VMINNMAV_FP:
    case MVE_VMINV:
    case MVE_VMINAV:
    case MVE_VABAV:
    case MVE_VMOV_HFP_TO_GP:
    case MVE_VMOV_GP_TO_VEC_LANE:
    case MVE_VMOV_VEC_LANE_TO_GP:
      {
	unsigned long rda = arm_decode_field (given, 12, 15);
	if (rda == 0xd)
	  {
	    *unpredictable_code = UNPRED_R13;
	    return true;
	  }
	else if (rda == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }

	return false;
      }

    case MVE_VMULL_INT:
      {
	unsigned long Qd;
	unsigned long Qm;
	unsigned long Qn;

	if (arm_decode_field (given, 20, 21) == 2)
	  {
	    Qd = arm_decode_field_multiple (given, 13, 15, 22, 22);
	    Qm = arm_decode_field_multiple (given, 1, 3, 5, 5);
	    Qn = arm_decode_field_multiple (given, 17, 19, 7, 7);

	    if ((Qd == Qn) || (Qd == Qm))
	      {
		*unpredictable_code = UNPRED_Q_REGS_EQ_AND_SIZE_2;
		return true;
	      }
	    else
	      return false;
	  }
	else
	  return false;
      }

    case MVE_VCMUL_FP:
    case MVE_VQDMULL_T1:
      {
	unsigned long Qd;
	unsigned long Qm;
	unsigned long Qn;

	if (arm_decode_field (given, 28, 28) == 1)
	  {
	    Qd = arm_decode_field_multiple (given, 13, 15, 22, 22);
	    Qm = arm_decode_field_multiple (given, 1, 3, 5, 5);
	    Qn = arm_decode_field_multiple (given, 17, 19, 7, 7);

	    if ((Qd == Qn) || (Qd == Qm))
	      {
		*unpredictable_code = UNPRED_Q_REGS_EQ_AND_SIZE_1;
		return true;
	      }
	    else
	      return false;
	  }
	else
	  return false;
      }

    case MVE_VQDMULL_T2:
      {
	unsigned long gpr = arm_decode_field (given, 0, 3);
	if (gpr == 0xd)
	  {
	    *unpredictable_code = UNPRED_R13;
	    return true;
	  }
	else if (gpr == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }

	if (arm_decode_field (given, 28, 28) == 1)
	  {
	    unsigned long Qd
	      = arm_decode_field_multiple (given, 13, 15, 22, 22);
	    unsigned long Qn = arm_decode_field_multiple (given, 17, 19, 7, 7);

	    if (Qd == Qn)
	      {
		*unpredictable_code = UNPRED_Q_REGS_EQ_AND_SIZE_1;
		return true;
	      }
	    else
	      return false;
	  }

	return false;
      }

    case MVE_VMLSLDAV:
    case MVE_VRMLSLDAVH:
    case MVE_VMLALDAV:
    case MVE_VADDLV:
      if (arm_decode_field (given, 20, 22) == 6)
	{
	  *unpredictable_code = UNPRED_R13;
	  return true;
	}
      else
	return false;

    case MVE_VDWDUP:
    case MVE_VIWDUP:
      if (arm_decode_field (given, 1, 3) == 6)
	{
	  *unpredictable_code = UNPRED_R13;
	  return true;
	}
      else
	return false;

    case MVE_VCADD_VEC:
    case MVE_VHCADD:
      {
	unsigned long Qd = arm_decode_field_multiple (given, 13, 15, 22, 22);
	unsigned long Qm = arm_decode_field_multiple (given, 1, 3, 5, 5);
	if ((Qd == Qm) && arm_decode_field (given, 20, 21) == 2)
	  {
	    *unpredictable_code = UNPRED_Q_REGS_EQ_AND_SIZE_2;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VCADD_FP:
      {
	unsigned long Qd = arm_decode_field_multiple (given, 13, 15, 22, 22);
	unsigned long Qm = arm_decode_field_multiple (given, 1, 3, 5, 5);
	if ((Qd == Qm) && arm_decode_field (given, 20, 20) == 1)
	  {
	    *unpredictable_code = UNPRED_Q_REGS_EQ_AND_SIZE_1;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_VCMLA_FP:
      {
	unsigned long Qda;
	unsigned long Qm;
	unsigned long Qn;

	if (arm_decode_field (given, 20, 20) == 1)
	  {
	    Qda = arm_decode_field_multiple (given, 13, 15, 22, 22);
	    Qm = arm_decode_field_multiple (given, 1, 3, 5, 5);
	    Qn = arm_decode_field_multiple (given, 17, 19, 7, 7);

	    if ((Qda == Qn) || (Qda == Qm))
	      {
		*unpredictable_code = UNPRED_Q_REGS_EQ_AND_SIZE_1;
		return true;
	      }
	    else
	      return false;
	  }
	else
	  return false;

      }

    case MVE_VCTP:
      if (arm_decode_field (given, 16, 19) == 0xd)
	{
	  *unpredictable_code = UNPRED_R13;
	  return true;
	}
      else
	return false;

    case MVE_VREV64:
      {
	unsigned long qd = arm_decode_field_multiple (given, 13, 15, 22, 22);
	unsigned long qm = arm_decode_field_multiple (given, 1, 3, 6, 6);

	if (qd == qm)
	  {
	    *unpredictable_code = UNPRED_Q_REGS_EQUAL;
	    return true;
	  }
	else
	  return false;
      }

    case MVE_LSLL:
    case MVE_LSLLI:
    case MVE_LSRL:
    case MVE_ASRL:
    case MVE_ASRLI:
    case MVE_UQSHLL:
    case MVE_UQRSHLL:
    case MVE_URSHRL:
    case MVE_SRSHRL:
    case MVE_SQSHLL:
    case MVE_SQRSHRL:
      {
	unsigned long gpr = arm_decode_field (given, 9, 11);
	gpr = ((gpr << 1) | 1);
	if (gpr == 0xd)
	  {
	    *unpredictable_code = UNPRED_R13;
	    return true;
	  }
	else if (gpr == 0xf)
	  {
	    *unpredictable_code = UNPRED_R15;
	    return true;
	  }

	return false;
      }

    default:
      return false;
    }
}

static void
print_mve_vmov_index (struct disassemble_info *info, unsigned long given)
{
  unsigned long op1 = arm_decode_field (given, 21, 22);
  unsigned long op2 = arm_decode_field (given, 5, 6);
  unsigned long h = arm_decode_field (given, 16, 16);
  unsigned long index_operand, esize, targetBeat, idx;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  if ((op1 & 0x2) == 0x2)
    {
      index_operand = op2;
      esize = 8;
    }
  else if (((op1 & 0x2) == 0x0) && ((op2 & 0x1) == 0x1))
    {
      index_operand = op2  >> 1;
      esize = 16;
    }
  else if (((op1 & 0x2) == 0) && ((op2 & 0x3) == 0))
    {
      index_operand = 0;
      esize = 32;
    }
  else
    {
      func (stream, dis_style_text, "<undefined index>");
      return;
    }

  targetBeat =  (op1 & 0x1) | (h << 1);
  idx = index_operand + targetBeat * (32/esize);

  func (stream, dis_style_immediate, "%lu", idx);
}

/* Print neon and mve 8-bit immediate that can be a 8, 16, 32, or 64-bits
   in length and integer of floating-point type.  */
static void
print_simd_imm8 (struct disassemble_info *info, unsigned long given,
		 unsigned int ibit_loc, const struct mopcode32 *insn)
{
  int bits = 0;
  int cmode = (given >> 8) & 0xf;
  int op = (given >> 5) & 0x1;
  unsigned long value = 0, hival = 0;
  unsigned shift;
  int size = 0;
  int isfloat = 0;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  /* On Neon the 'i' bit is at bit 24, on mve it is
     at bit 28.  */
  bits |= ((given >> ibit_loc) & 1) << 7;
  bits |= ((given >> 16) & 7) << 4;
  bits |= ((given >> 0) & 15) << 0;

  if (cmode < 8)
    {
      shift = (cmode >> 1) & 3;
      value = (unsigned long) bits << (8 * shift);
      size = 32;
    }
  else if (cmode < 12)
    {
      shift = (cmode >> 1) & 1;
      value = (unsigned long) bits << (8 * shift);
      size = 16;
    }
  else if (cmode < 14)
    {
      shift = (cmode & 1) + 1;
      value = (unsigned long) bits << (8 * shift);
      value |= (1ul << (8 * shift)) - 1;
      size = 32;
    }
  else if (cmode == 14)
    {
      if (op)
	{
	  /* Bit replication into bytes.  */
	  int ix;
	  unsigned long mask;

	  value = 0;
	  hival = 0;
	  for (ix = 7; ix >= 0; ix--)
	    {
	      mask = ((bits >> ix) & 1) ? 0xff : 0;
	      if (ix <= 3)
		value = (value << 8) | mask;
	      else
		hival = (hival << 8) | mask;
	    }
	  size = 64;
	}
      else
	{
	  /* Byte replication.  */
	  value = (unsigned long) bits;
	  size = 8;
	}
    }
  else if (!op)
    {
      /* Floating point encoding.  */
      int tmp;

      value = (unsigned long)  (bits & 0x7f) << 19;
      value |= (unsigned long) (bits & 0x80) << 24;
      tmp = bits & 0x40 ? 0x3c : 0x40;
      value |= (unsigned long) tmp << 24;
      size = 32;
      isfloat = 1;
    }
  else
    {
      func (stream, dis_style_text, "<illegal constant %.8x:%x:%x>",
	    bits, cmode, op);
      size = 32;
      return;
    }

  /* printU determines whether the immediate value should be printed as
     unsigned.  */
  unsigned printU = 0;
  switch (insn->mve_op)
    {
    default:
      break;
    /* We want this for instructions that don't have a 'signed' type.  */
    case MVE_VBIC_IMM:
    case MVE_VORR_IMM:
    case MVE_VMVN_IMM:
    case MVE_VMOV_IMM_TO_VEC:
      printU = 1;
      break;
    }
  switch (size)
    {
    case 8:
      func (stream, dis_style_immediate, "#%ld", value);
      func (stream, dis_style_comment_start, "\t@ 0x%.2lx", value);
      break;

    case 16:
      func (stream, dis_style_immediate, printU ? "#%lu" : "#%ld", value);
      func (stream, dis_style_comment_start, "\t@ 0x%.4lx", value);
      break;

    case 32:
      if (isfloat)
	{
	  unsigned char valbytes[4];
	  double fvalue;

	  /* Do this a byte at a time so we don't have to
	     worry about the host's endianness.  */
	  valbytes[0] = value & 0xff;
	  valbytes[1] = (value >> 8) & 0xff;
	  valbytes[2] = (value >> 16) & 0xff;
	  valbytes[3] = (value >> 24) & 0xff;

	  floatformat_to_double
	    (& floatformat_ieee_single_little, valbytes,
	     & fvalue);

	  func (stream, dis_style_immediate, "#%.7g", fvalue);
	  func (stream, dis_style_comment_start, "\t@ 0x%.8lx", value);
	}
      else
	{
	  func (stream, dis_style_immediate,
		printU ? "#%lu" : "#%ld",
		(long) (((value & 0x80000000L) != 0)
			&& !printU
			? value | ~0xffffffffL : value));
	  func (stream, dis_style_comment_start, "\t@ 0x%.8lx", value);
	}
      break;

    case 64:
      func (stream, dis_style_immediate, "#0x%.8lx%.8lx", hival, value);
      break;

    default:
      abort ();
    }

}

static void
print_mve_undefined (struct disassemble_info *info,
		     enum mve_undefined undefined_code)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  /* Initialize REASON to avoid compiler warning about uninitialized
     usage, though such usage should be impossible.  */
  const char *reason = "??";

  switch (undefined_code)
    {
    case UNDEF_SIZE:
      reason = "illegal size";
      break;

    case UNDEF_SIZE_0:
      reason = "size equals zero";
      break;

    case UNDEF_SIZE_2:
      reason = "size equals two";
      break;

    case UNDEF_SIZE_3:
      reason = "size equals three";
      break;

    case UNDEF_SIZE_LE_1:
      reason = "size <= 1";
      break;

    case UNDEF_SIZE_NOT_0:
      reason = "size not equal to 0";
      break;

    case UNDEF_SIZE_NOT_2:
      reason = "size not equal to 2";
      break;

    case UNDEF_SIZE_NOT_3:
      reason = "size not equal to 3";
      break;

    case UNDEF_NOT_UNS_SIZE_0:
      reason = "not unsigned and size = zero";
      break;

    case UNDEF_NOT_UNS_SIZE_1:
      reason = "not unsigned and size = one";
      break;

    case UNDEF_NOT_UNSIGNED:
      reason = "not unsigned";
      break;

    case UNDEF_VCVT_IMM6:
      reason = "invalid imm6";
      break;

    case UNDEF_VCVT_FSI_IMM6:
      reason = "fsi = 0 and invalid imm6";
      break;

    case UNDEF_BAD_OP1_OP2:
      reason = "bad size with op2 = 2 and op1 = 0 or 1";
      break;

    case UNDEF_BAD_U_OP1_OP2:
      reason = "unsigned with op2 = 0 and op1 = 0 or 1";
      break;

    case UNDEF_OP_0_BAD_CMODE:
      reason = "op field equal 0 and bad cmode";
      break;

    case UNDEF_XCHG_UNS:
      reason = "exchange and unsigned together";
      break;

    case UNDEF_NONE:
      reason = "";
      break;
    }

  func (stream, dis_style_text, "\t\tundefined instruction: %s", reason);
}

static void
print_mve_unpredictable (struct disassemble_info *info,
			 enum mve_unpredictable unpredict_code)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  /* Initialize REASON to avoid compiler warning about uninitialized
     usage, though such usage should be impossible.  */
  const char *reason = "??";

  switch (unpredict_code)
    {
    case UNPRED_IT_BLOCK:
      reason = "mve instruction in it block";
      break;

    case UNPRED_FCA_0_FCB_1:
      reason = "condition bits, fca = 0 and fcb = 1";
      break;

    case UNPRED_R13:
      reason = "use of r13 (sp)";
      break;

    case UNPRED_R15:
      reason = "use of r15 (pc)";
      break;

    case UNPRED_Q_GT_4:
      reason = "start register block > r4";
      break;

    case UNPRED_Q_GT_6:
      reason = "start register block > r6";
      break;

    case UNPRED_R13_AND_WB:
      reason = "use of r13 and write back";
      break;

    case UNPRED_Q_REGS_EQUAL:
      reason = "same vector register used for destination and other operand";
      break;

    case UNPRED_OS:
      reason = "use of offset scaled";
      break;

    case UNPRED_GP_REGS_EQUAL:
      reason = "same general-purpose register used for both operands";
      break;

    case UNPRED_Q_REGS_EQ_AND_SIZE_1:
      reason = "use of identical q registers and size = 1";
      break;

    case UNPRED_Q_REGS_EQ_AND_SIZE_2:
      reason = "use of identical q registers and size = 1";
      break;

    case UNPRED_NONE:
      reason = "";
      break;
    }

  func (stream, dis_style_comment_start, "%s: %s",
	UNPREDICTABLE_INSTRUCTION, reason);
}

/* Print register block operand for mve vld2/vld4/vst2/vld4.  */

static void
print_mve_register_blocks (struct disassemble_info *info,
			   unsigned long given,
			   enum mve_instructions matched_insn)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  unsigned long q_reg_start = arm_decode_field_multiple (given,
							 13, 15,
							 22, 22);
  switch (matched_insn)
    {
    case MVE_VLD2:
    case MVE_VST2:
      if (q_reg_start <= 6)
	{
	  func (stream, dis_style_text, "{");
	  func (stream, dis_style_register, "q%ld", q_reg_start);
	  func (stream, dis_style_text, ", ");
	  func (stream, dis_style_register, "q%ld", q_reg_start + 1);
	  func (stream, dis_style_text, "}");
	}
      else
	func (stream, dis_style_text, "<illegal reg q%ld>", q_reg_start);
      break;

    case MVE_VLD4:
    case MVE_VST4:
      if (q_reg_start <= 4)
	{
	  func (stream, dis_style_text, "{");
	  func (stream, dis_style_register, "q%ld", q_reg_start);
	  func (stream, dis_style_text, ", ");
	  func (stream, dis_style_register, "q%ld", q_reg_start + 1);
	  func (stream, dis_style_text, ", ");
	  func (stream, dis_style_register, "q%ld", q_reg_start + 2);
	  func (stream, dis_style_text, ", ");
	  func (stream, dis_style_register, "q%ld", q_reg_start + 3);
	  func (stream, dis_style_text, "}");
	}
      else
	func (stream, dis_style_text, "<illegal reg q%ld>", q_reg_start);
      break;

    default:
      break;
    }
}

static void
print_mve_rounding_mode (struct disassemble_info *info,
			 unsigned long given,
			 enum mve_instructions matched_insn)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  switch (matched_insn)
    {
    case MVE_VCVT_FROM_FP_TO_INT:
      {
	switch (arm_decode_field (given, 8, 9))
	  {
	  case 0:
	    func (stream, dis_style_mnemonic, "a");
	    break;

	  case 1:
	    func (stream, dis_style_mnemonic, "n");
	    break;

	  case 2:
	    func (stream, dis_style_mnemonic, "p");
	    break;

	  case 3:
	    func (stream, dis_style_mnemonic, "m");
	    break;

	  default:
	    break;
	  }
      }
      break;

    case MVE_VRINT_FP:
      {
	switch (arm_decode_field (given, 7, 9))
	  {
	  case 0:
	    func (stream, dis_style_mnemonic, "n");
	    break;

	  case 1:
	    func (stream, dis_style_mnemonic, "x");
	    break;

	  case 2:
	    func (stream, dis_style_mnemonic, "a");
	    break;

	  case 3:
	    func (stream, dis_style_mnemonic, "z");
	    break;

	  case 5:
	    func (stream, dis_style_mnemonic, "m");
	    break;

	  case 7:
	    func (stream, dis_style_mnemonic, "p");

	  case 4:
	  case 6:
	  default:
	    break;
	  }
      }
      break;

    default:
      break;
    }
}

static void
print_mve_vcvt_size (struct disassemble_info *info,
		     unsigned long given,
		     enum mve_instructions matched_insn)
{
  unsigned long mode = 0;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  switch (matched_insn)
    {
    case MVE_VCVT_FP_FIX_VEC:
      {
	mode = (((given & 0x200) >> 7)
		| ((given & 0x10000000) >> 27)
		| ((given & 0x100) >> 8));

	switch (mode)
	  {
	  case 0:
	    func (stream, dis_style_mnemonic, "f16.s16");
	    break;

	  case 1:
	    func (stream, dis_style_mnemonic, "s16.f16");
	    break;

	  case 2:
	    func (stream, dis_style_mnemonic, "f16.u16");
	    break;

	  case 3:
	    func (stream, dis_style_mnemonic, "u16.f16");
	    break;

	  case 4:
	    func (stream, dis_style_mnemonic, "f32.s32");
	    break;

	  case 5:
	    func (stream, dis_style_mnemonic, "s32.f32");
	    break;

	  case 6:
	    func (stream, dis_style_mnemonic, "f32.u32");
	    break;

	  case 7:
	    func (stream, dis_style_mnemonic, "u32.f32");
	    break;

	  default:
	    break;
	  }
	break;
      }
    case MVE_VCVT_BETWEEN_FP_INT:
      {
	unsigned long size = arm_decode_field (given, 18, 19);
	unsigned long op = arm_decode_field (given, 7, 8);

	if (size == 1)
	  {
	    switch (op)
	      {
	      case 0:
		func (stream, dis_style_mnemonic, "f16.s16");
		break;

	      case 1:
		func (stream, dis_style_mnemonic, "f16.u16");
		break;

	      case 2:
		func (stream, dis_style_mnemonic, "s16.f16");
		break;

	      case 3:
		func (stream, dis_style_mnemonic, "u16.f16");
		break;

	      default:
		break;
	      }
	  }
	else if (size == 2)
	  {
	    switch (op)
	      {
	      case 0:
		func (stream, dis_style_mnemonic, "f32.s32");
		break;

	      case 1:
		func (stream, dis_style_mnemonic, "f32.u32");
		break;

	      case 2:
		func (stream, dis_style_mnemonic, "s32.f32");
		break;

	      case 3:
		func (stream, dis_style_mnemonic, "u32.f32");
		break;
	      }
	  }
      }
      break;

    case MVE_VCVT_FP_HALF_FP:
      {
	unsigned long op = arm_decode_field (given, 28, 28);
	if (op == 0)
	  func (stream, dis_style_mnemonic, "f16.f32");
	else if (op == 1)
	  func (stream, dis_style_mnemonic, "f32.f16");
      }
      break;

    case MVE_VCVT_FROM_FP_TO_INT:
      {
	unsigned long size = arm_decode_field_multiple (given, 7, 7, 18, 19);

	switch (size)
	  {
	  case 2:
	    func (stream, dis_style_mnemonic, "s16.f16");
	    break;

	  case 3:
	    func (stream, dis_style_mnemonic, "u16.f16");
	    break;

	  case 4:
	    func (stream, dis_style_mnemonic, "s32.f32");
	    break;

	  case 5:
	    func (stream, dis_style_mnemonic, "u32.f32");
	    break;

	  default:
	    break;
	  }
      }
      break;

    default:
      break;
    }
}

static void
print_mve_rotate (struct disassemble_info *info, unsigned long rot,
		  unsigned long rot_width)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  if (rot_width == 1)
    {
      switch (rot)
	{
	case 0:
	  func (stream, dis_style_immediate, "90");
	  break;
	case 1:
	  func (stream, dis_style_immediate, "270");
	  break;
	default:
	  break;
	}
    }
  else if (rot_width == 2)
    {
      switch (rot)
	{
	case 0:
	  func (stream, dis_style_immediate, "0");
	  break;
	case 1:
	  func (stream, dis_style_immediate, "90");
	  break;
	case 2:
	  func (stream, dis_style_immediate, "180");
	  break;
	case 3:
	  func (stream, dis_style_immediate, "270");
	  break;
	default:
	  break;
	}
    }
}

static void
print_instruction_predicate (struct disassemble_info *info)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  if (vpt_block_state.next_pred_state == PRED_THEN)
    func (stream, dis_style_mnemonic, "t");
  else if (vpt_block_state.next_pred_state == PRED_ELSE)
    func (stream, dis_style_mnemonic, "e");
}

static void
print_mve_size (struct disassemble_info *info,
		unsigned long size,
		enum mve_instructions matched_insn)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  switch (matched_insn)
    {
    case MVE_VABAV:
    case MVE_VABD_VEC:
    case MVE_VABS_FP:
    case MVE_VABS_VEC:
    case MVE_VADD_VEC_T1:
    case MVE_VADD_VEC_T2:
    case MVE_VADDV:
    case MVE_VBRSR:
    case MVE_VCADD_VEC:
    case MVE_VCLS:
    case MVE_VCLZ:
    case MVE_VCMP_VEC_T1:
    case MVE_VCMP_VEC_T2:
    case MVE_VCMP_VEC_T3:
    case MVE_VCMP_VEC_T4:
    case MVE_VCMP_VEC_T5:
    case MVE_VCMP_VEC_T6:
    case MVE_VCTP:
    case MVE_VDDUP:
    case MVE_VDWDUP:
    case MVE_VHADD_T1:
    case MVE_VHADD_T2:
    case MVE_VHCADD:
    case MVE_VHSUB_T1:
    case MVE_VHSUB_T2:
    case MVE_VIDUP:
    case MVE_VIWDUP:
    case MVE_VLD2:
    case MVE_VLD4:
    case MVE_VLDRB_GATHER_T1:
    case MVE_VLDRH_GATHER_T2:
    case MVE_VLDRW_GATHER_T3:
    case MVE_VLDRD_GATHER_T4:
    case MVE_VLDRB_T1:
    case MVE_VLDRH_T2:
    case MVE_VMAX:
    case MVE_VMAXA:
    case MVE_VMAXV:
    case MVE_VMAXAV:
    case MVE_VMIN:
    case MVE_VMINA:
    case MVE_VMINV:
    case MVE_VMINAV:
    case MVE_VMLA:
    case MVE_VMLAS:
    case MVE_VMUL_VEC_T1:
    case MVE_VMUL_VEC_T2:
    case MVE_VMULH:
    case MVE_VRMULH:
    case MVE_VMULL_INT:
    case MVE_VNEG_FP:
    case MVE_VNEG_VEC:
    case MVE_VPT_VEC_T1:
    case MVE_VPT_VEC_T2:
    case MVE_VPT_VEC_T3:
    case MVE_VPT_VEC_T4:
    case MVE_VPT_VEC_T5:
    case MVE_VPT_VEC_T6:
    case MVE_VQABS:
    case MVE_VQADD_T1:
    case MVE_VQADD_T2:
    case MVE_VQDMLADH:
    case MVE_VQRDMLADH:
    case MVE_VQDMLAH:
    case MVE_VQRDMLAH:
    case MVE_VQDMLASH:
    case MVE_VQRDMLASH:
    case MVE_VQDMLSDH:
    case MVE_VQRDMLSDH:
    case MVE_VQDMULH_T1:
    case MVE_VQRDMULH_T2:
    case MVE_VQDMULH_T3:
    case MVE_VQRDMULH_T4:
    case MVE_VQNEG:
    case MVE_VQRSHL_T1:
    case MVE_VQRSHL_T2:
    case MVE_VQSHL_T1:
    case MVE_VQSHL_T4:
    case MVE_VQSUB_T1:
    case MVE_VQSUB_T2:
    case MVE_VREV32:
    case MVE_VREV64:
    case MVE_VRHADD:
    case MVE_VRINT_FP:
    case MVE_VRSHL_T1:
    case MVE_VRSHL_T2:
    case MVE_VSHL_T2:
    case MVE_VSHL_T3:
    case MVE_VSHLL_T2:
    case MVE_VST2:
    case MVE_VST4:
    case MVE_VSTRB_SCATTER_T1:
    case MVE_VSTRH_SCATTER_T2:
    case MVE_VSTRW_SCATTER_T3:
    case MVE_VSTRB_T1:
    case MVE_VSTRH_T2:
    case MVE_VSUB_VEC_T1:
    case MVE_VSUB_VEC_T2:
      if (size <= 3)
	func (stream, dis_style_mnemonic, "%s", mve_vec_sizename[size]);
      else
	func (stream, dis_style_text, "<undef size>");
      break;

    case MVE_VABD_FP:
    case MVE_VADD_FP_T1:
    case MVE_VADD_FP_T2:
    case MVE_VSUB_FP_T1:
    case MVE_VSUB_FP_T2:
    case MVE_VCMP_FP_T1:
    case MVE_VCMP_FP_T2:
    case MVE_VFMA_FP_SCALAR:
    case MVE_VFMA_FP:
    case MVE_VFMS_FP:
    case MVE_VFMAS_FP_SCALAR:
    case MVE_VMAXNM_FP:
    case MVE_VMAXNMA_FP:
    case MVE_VMAXNMV_FP:
    case MVE_VMAXNMAV_FP:
    case MVE_VMINNM_FP:
    case MVE_VMINNMA_FP:
    case MVE_VMINNMV_FP:
    case MVE_VMINNMAV_FP:
    case MVE_VMUL_FP_T1:
    case MVE_VMUL_FP_T2:
    case MVE_VPT_FP_T1:
    case MVE_VPT_FP_T2:
      if (size == 0)
	func (stream, dis_style_mnemonic, "32");
      else if (size == 1)
	func (stream, dis_style_mnemonic, "16");
      break;

    case MVE_VCADD_FP:
    case MVE_VCMLA_FP:
    case MVE_VCMUL_FP:
    case MVE_VMLADAV_T1:
    case MVE_VMLALDAV:
    case MVE_VMLSDAV_T1:
    case MVE_VMLSLDAV:
    case MVE_VMOVN:
    case MVE_VQDMULL_T1:
    case MVE_VQDMULL_T2:
    case MVE_VQMOVN:
    case MVE_VQMOVUN:
      if (size == 0)
	func (stream, dis_style_mnemonic, "16");
      else if (size == 1)
	func (stream, dis_style_mnemonic, "32");
      break;

    case MVE_VMOVL:
      if (size == 1)
	func (stream, dis_style_mnemonic, "8");
      else if (size == 2)
	func (stream, dis_style_mnemonic, "16");
      break;

    case MVE_VDUP:
      switch (size)
	{
	case 0:
	  func (stream, dis_style_mnemonic, "32");
	  break;
	case 1:
	  func (stream, dis_style_mnemonic, "16");
	  break;
	case 2:
	  func (stream, dis_style_mnemonic, "8");
	  break;
	default:
	  break;
	}
      break;

    case MVE_VMOV_GP_TO_VEC_LANE:
    case MVE_VMOV_VEC_LANE_TO_GP:
      switch (size)
	{
	case 0: case 4:
	  func (stream, dis_style_mnemonic, "32");
	  break;

	case 1: case 3:
	case 5: case 7:
	  func (stream, dis_style_mnemonic, "16");
	  break;

	case 8: case 9: case 10: case 11:
	case 12: case 13: case 14: case 15:
	  func (stream, dis_style_mnemonic, "8");
	  break;

	default:
	  break;
	}
      break;

    case MVE_VMOV_IMM_TO_VEC:
      switch (size)
	{
	case 0: case 4: case 8:
	case 12: case 24: case 26:
	  func (stream, dis_style_mnemonic, "i32");
	  break;
	case 16: case 20:
	  func (stream, dis_style_mnemonic, "i16");
	  break;
	case 28:
	  func (stream, dis_style_mnemonic, "i8");
	  break;
	case 29:
	  func (stream, dis_style_mnemonic, "i64");
	  break;
	case 30:
	  func (stream, dis_style_mnemonic, "f32");
	  break;
	default:
	  break;
	}
      break;

    case MVE_VMULL_POLY:
      if (size == 0)
	func (stream, dis_style_mnemonic, "p8");
      else if (size == 1)
	func (stream, dis_style_mnemonic, "p16");
      break;

    case MVE_VMVN_IMM:
      switch (size)
	{
	case 0: case 2: case 4:
	case 6: case 12: case 13:
	  func (stream, dis_style_mnemonic, "32");
	  break;

	case 8: case 10:
	  func (stream, dis_style_mnemonic, "16");
	  break;

	default:
	  break;
	}
      break;

    case MVE_VBIC_IMM:
    case MVE_VORR_IMM:
      switch (size)
	{
	case 1: case 3:
	case 5: case 7:
	  func (stream, dis_style_mnemonic, "32");
	  break;

	case 9: case 11:
	  func (stream, dis_style_mnemonic, "16");
	  break;

	default:
	  break;
	}
      break;

    case MVE_VQSHRN:
    case MVE_VQSHRUN:
    case MVE_VQRSHRN:
    case MVE_VQRSHRUN:
    case MVE_VRSHRN:
    case MVE_VSHRN:
      {
	switch (size)
	{
	case 1:
	  func (stream, dis_style_mnemonic, "16");
	  break;

	case 2: case 3:
	  func (stream, dis_style_mnemonic, "32");
	  break;

	default:
	  break;
	}
      }
      break;

    case MVE_VQSHL_T2:
    case MVE_VQSHLU_T3:
    case MVE_VRSHR:
    case MVE_VSHL_T1:
    case MVE_VSHLL_T1:
    case MVE_VSHR:
    case MVE_VSLI:
    case MVE_VSRI:
      {
	switch (size)
	{
	case 1:
	  func (stream, dis_style_mnemonic, "8");
	  break;

	case 2: case 3:
	  func (stream, dis_style_mnemonic, "16");
	  break;

	case 4: case 5: case 6: case 7:
	  func (stream, dis_style_mnemonic, "32");
	  break;

	default:
	  break;
	}
      }
      break;

    default:
      break;
    }
}

static void
print_mve_shift_n (struct disassemble_info *info, long given,
		   enum mve_instructions matched_insn)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;

  int startAt0
    = matched_insn == MVE_VQSHL_T2
      || matched_insn == MVE_VQSHLU_T3
      || matched_insn == MVE_VSHL_T1
      || matched_insn == MVE_VSHLL_T1
      || matched_insn == MVE_VSLI;

  unsigned imm6 = (given & 0x3f0000) >> 16;

  if (matched_insn == MVE_VSHLL_T1)
    imm6 &= 0x1f;

  unsigned shiftAmount = 0;
  if ((imm6 & 0x20) != 0)
    shiftAmount = startAt0 ? imm6 - 32 : 64 - imm6;
  else if ((imm6 & 0x10) != 0)
    shiftAmount = startAt0 ? imm6 - 16 : 32 - imm6;
  else if ((imm6 & 0x08) != 0)
    shiftAmount = startAt0 ? imm6 - 8 : 16 - imm6;
  else
    print_mve_undefined (info, UNDEF_SIZE_0);

  func (stream, dis_style_immediate, "%u", shiftAmount);
}

static void
print_vec_condition (struct disassemble_info *info, long given,
		     enum mve_instructions matched_insn)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  long vec_cond = 0;

  switch (matched_insn)
    {
    case MVE_VPT_FP_T1:
    case MVE_VCMP_FP_T1:
      vec_cond = (((given & 0x1000) >> 10)
		  | ((given & 1) << 1)
		  | ((given & 0x0080) >> 7));
      func (stream, dis_style_sub_mnemonic, "%s", vec_condnames[vec_cond]);
      break;

    case MVE_VPT_FP_T2:
    case MVE_VCMP_FP_T2:
      vec_cond = (((given & 0x1000) >> 10)
		  | ((given & 0x0020) >> 4)
		  | ((given & 0x0080) >> 7));
      func (stream, dis_style_sub_mnemonic, "%s", vec_condnames[vec_cond]);
      break;

    case MVE_VPT_VEC_T1:
    case MVE_VCMP_VEC_T1:
      vec_cond = (given & 0x0080) >> 7;
      func (stream, dis_style_sub_mnemonic, "%s", vec_condnames[vec_cond]);
      break;

    case MVE_VPT_VEC_T2:
    case MVE_VCMP_VEC_T2:
      vec_cond = 2 | ((given & 0x0080) >> 7);
      func (stream, dis_style_sub_mnemonic, "%s", vec_condnames[vec_cond]);
      break;

    case MVE_VPT_VEC_T3:
    case MVE_VCMP_VEC_T3:
      vec_cond = 4 | ((given & 1) << 1) | ((given & 0x0080) >> 7);
      func (stream, dis_style_sub_mnemonic, "%s", vec_condnames[vec_cond]);
      break;

    case MVE_VPT_VEC_T4:
    case MVE_VCMP_VEC_T4:
      vec_cond = (given & 0x0080) >> 7;
      func (stream, dis_style_sub_mnemonic, "%s", vec_condnames[vec_cond]);
      break;

    case MVE_VPT_VEC_T5:
    case MVE_VCMP_VEC_T5:
      vec_cond = 2 | ((given & 0x0080) >> 7);
      func (stream, dis_style_sub_mnemonic, "%s", vec_condnames[vec_cond]);
      break;

    case MVE_VPT_VEC_T6:
    case MVE_VCMP_VEC_T6:
      vec_cond = 4 | ((given & 0x0020) >> 4) | ((given & 0x0080) >> 7);
      func (stream, dis_style_sub_mnemonic, "%s", vec_condnames[vec_cond]);
      break;

    case MVE_NONE:
    case MVE_VPST:
    default:
      break;
    }
}

#define W_BIT 21
#define I_BIT 22
#define U_BIT 23
#define P_BIT 24

#define WRITEBACK_BIT_SET (given & (1 << W_BIT))
#define IMMEDIATE_BIT_SET (given & (1 << I_BIT))
#define NEGATIVE_BIT_SET  ((given & (1 << U_BIT)) == 0)
#define PRE_BIT_SET	  (given & (1 << P_BIT))

/* The assembler string for an instruction can include %{X:...%} patterns,
   where the 'X' is one of the characters understood by this function.

   This function takes the X character, and returns a new style.  This new
   style will be used by the caller to temporarily change the current base
   style.  */

static enum disassembler_style
decode_base_style (const char x)
{
  switch (x)
    {
    case 'A': return dis_style_address;
    case 'B': return dis_style_sub_mnemonic;
    case 'C': return dis_style_comment_start;
    case 'D': return dis_style_assembler_directive;
    case 'I': return dis_style_immediate;
    case 'M': return dis_style_mnemonic;
    case 'O': return dis_style_address_offset;
    case 'R': return dis_style_register;
    case 'S': return dis_style_symbol;
    case 'T': return dis_style_text;
    default:
      abort ();
    }
}

/* Print one coprocessor instruction on INFO->STREAM.
   Return TRUE if the instuction matched, FALSE if this is not a
   recognised coprocessor instruction.  */

static bool
print_insn_coprocessor_1 (const struct sopcode32 *opcodes,
			  bfd_vma pc,
			  struct disassemble_info *info,
			  long given,
			  bool thumb)
{
  const struct sopcode32 *insn;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  unsigned long mask;
  unsigned long value = 0;
  int cond;
  int cp_num;
  struct arm_private_data *private_data = info->private_data;
  arm_feature_set allowed_arches = ARM_ARCH_NONE;
  arm_feature_set arm_ext_v8_1m_main =
    ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN);
  enum disassembler_style base_style = dis_style_mnemonic;
  enum disassembler_style old_base_style = base_style;

  allowed_arches = private_data->features;

  for (insn = opcodes; insn->assembler; insn++)
    {
      unsigned long u_reg = 16;
      bool is_unpredictable = false;
      signed long value_in_comment = 0;
      const char *c;

      if (ARM_FEATURE_ZERO (insn->arch))
	switch (insn->value)
	  {
	  case SENTINEL_IWMMXT_START:
	    if (info->mach != bfd_mach_arm_XScale
		&& info->mach != bfd_mach_arm_iWMMXt
		&& info->mach != bfd_mach_arm_iWMMXt2)
	      do
		insn++;
	      while ((! ARM_FEATURE_ZERO (insn->arch))
		     && insn->value != SENTINEL_IWMMXT_END);
	    continue;

	  case SENTINEL_IWMMXT_END:
	    continue;

	  case SENTINEL_GENERIC_START:
	    allowed_arches = private_data->features;
	    continue;

	  default:
	    abort ();
	  }

      mask = insn->mask;
      value = insn->value;
      cp_num = (given >> 8) & 0xf;

      if (thumb)
	{
	  /* The high 4 bits are 0xe for Arm conditional instructions, and
	     0xe for arm unconditional instructions.  The rest of the
	     encoding is the same.  */
	  mask |= 0xf0000000;
	  value |= 0xe0000000;
	  if (ifthen_state)
	    cond = IFTHEN_COND;
	  else
	    cond = COND_UNCOND;
	}
      else
	{
	  /* Only match unconditional instuctions against unconditional
	     patterns.  */
	  if ((given & 0xf0000000) == 0xf0000000)
	    {
	      mask |= 0xf0000000;
	      cond = COND_UNCOND;
	    }
	  else
	    {
	      cond = (given >> 28) & 0xf;
	      if (cond == 0xe)
		cond = COND_UNCOND;
	    }
	}

      if ((insn->isa == T32 && !thumb)
	  || (insn->isa == ARM && thumb))
	continue;

      if ((given & mask) != value)
	continue;

      if (! ARM_CPU_HAS_FEATURE (insn->arch, allowed_arches))
	continue;

      if (insn->value == 0xfe000010     /* mcr2  */
	  || insn->value == 0xfe100010  /* mrc2  */
	  || insn->value == 0xfc100000  /* ldc2  */
	  || insn->value == 0xfc000000) /* stc2  */
	{
	  if (cp_num == 9 || cp_num == 10 || cp_num == 11)
	    is_unpredictable = true;

	  /* Armv8.1-M Mainline FP & MVE instructions.  */
	  if (ARM_CPU_HAS_FEATURE (arm_ext_v8_1m_main, allowed_arches)
	      && !ARM_CPU_IS_ANY (allowed_arches)
	      && (cp_num == 8 || cp_num == 14 || cp_num == 15))
	    continue;

	}
      else if (insn->value == 0x0e000000     /* cdp  */
	       || insn->value == 0xfe000000  /* cdp2  */
	       || insn->value == 0x0e000010  /* mcr  */
	       || insn->value == 0x0e100010  /* mrc  */
	       || insn->value == 0x0c100000  /* ldc  */
	       || insn->value == 0x0c000000) /* stc  */
	{
	  /* Floating-point instructions.  */
	  if (cp_num == 9 || cp_num == 10 || cp_num == 11)
	    continue;

	  /* Armv8.1-M Mainline FP & MVE instructions.  */
	  if (ARM_CPU_HAS_FEATURE (arm_ext_v8_1m_main, allowed_arches)
	      && !ARM_CPU_IS_ANY (allowed_arches)
	      && (cp_num == 8 || cp_num == 14 || cp_num == 15))
	    continue;
	}
      else if ((insn->value == 0xec100f80      /* vldr (system register) */
		|| insn->value == 0xec000f80)  /* vstr (system register) */
	       && arm_decode_field (given, 24, 24) == 0
	       && arm_decode_field (given, 21, 21) == 0)
	/* If the P and W bits are both 0 then these encodings match the MVE
	   VLDR and VSTR instructions, these are in a different table, so we
	   don't let it match here.  */
	continue;

      for (c = insn->assembler; *c; c++)
	{
	  if (*c == '%')
	    {
	      const char mod = *++c;

	      switch (mod)
		{
		case '{':
		  ++c;
		  if (*c == '\0')
		    abort ();
		  old_base_style = base_style;
		  base_style = decode_base_style (*c);
		  ++c;
		  if (*c != ':')
		    abort ();
		  break;

		case '}':
		  base_style = old_base_style;
		  break;

		case '%':
		  func (stream, base_style, "%%");
		  break;

		case 'A':
		case 'K':
		  {
		    int rn = (given >> 16) & 0xf;
		    bfd_vma offset = given & 0xff;

		    if (mod == 'K')
		      offset = given & 0x7f;

		    func (stream, dis_style_text, "[");
		    func (stream, dis_style_register, "%s",
			  arm_regnames [(given >> 16) & 0xf]);

		    if (PRE_BIT_SET || WRITEBACK_BIT_SET)
		      {
			/* Not unindexed.  The offset is scaled.  */
			if (cp_num == 9)
			  /* vldr.16/vstr.16 will shift the address
			     left by 1 bit only.  */
			  offset = offset * 2;
			else
			  offset = offset * 4;

			if (NEGATIVE_BIT_SET)
			  offset = - offset;
			if (rn != 15)
			  value_in_comment = offset;
		      }

		    if (PRE_BIT_SET)
		      {
			if (offset)
			  {
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_immediate, "#%d",
				  (int) offset);
			    func (stream, dis_style_text, "]%s",
				  WRITEBACK_BIT_SET ? "!" : "");
			  }
			else if (NEGATIVE_BIT_SET)
			  {
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_immediate, "#-0");
			    func (stream, dis_style_text, "]");
			  }
			else
			  func (stream, dis_style_text, "]");
		      }
		    else
		      {
			func (stream, dis_style_text, "]");

			if (WRITEBACK_BIT_SET)
			  {
			    if (offset)
			      {
				func (stream, dis_style_text, ", ");
				func (stream, dis_style_immediate,
				      "#%d", (int) offset);
			      }
			    else if (NEGATIVE_BIT_SET)
			      {
				func (stream, dis_style_text, ", ");
				func (stream, dis_style_immediate, "#-0");
			      }
			  }
			else
			  {
			    func (stream, dis_style_text, ", {");
			    func (stream, dis_style_immediate, "%s%d",
				  (NEGATIVE_BIT_SET && !offset) ? "-" : "",
				  (int) offset);
			    func (stream, dis_style_text, "}");
			    value_in_comment = offset;
			  }
		      }
		    if (rn == 15 && (PRE_BIT_SET || WRITEBACK_BIT_SET))
		      {
			func (stream, dis_style_comment_start, "\t@ ");
			/* For unaligned PCs, apply off-by-alignment
			   correction.  */
			info->print_address_func (offset + pc
						  + info->bytes_per_chunk * 2
						  - (pc & 3),
						  info);
		      }
		  }
		  break;

		case 'B':
		  {
		    int regno = ((given >> 12) & 0xf) | ((given >> (22 - 4)) & 0x10);
		    int offset = (given >> 1) & 0x3f;

		    func (stream, dis_style_text, "{");
		    if (offset == 1)
		      func (stream, dis_style_register, "d%d", regno);
		    else if (regno + offset > 32)
		      {
			func (stream, dis_style_register, "d%d", regno);
			func (stream, dis_style_text, "-<overflow reg d%d>",
			      regno + offset - 1);
		      }
		    else
		      {
			func (stream, dis_style_register, "d%d", regno);
			func (stream, dis_style_text, "-");
			func (stream, dis_style_register, "d%d",
			      regno + offset - 1);
		      }
		    func (stream, dis_style_text, "}");
		  }
		  break;

		case 'C':
		  {
		    bool single = ((given >> 8) & 1) == 0;
		    char reg_prefix = single ? 's' : 'd';
		    int Dreg = (given >> 22) & 0x1;
		    int Vdreg = (given >> 12) & 0xf;
		    int reg = single ? ((Vdreg << 1) | Dreg)
				     : ((Dreg << 4) | Vdreg);
		    int num = (given >> (single ? 0 : 1)) & 0x7f;
		    int maxreg = single ? 31 : 15;
		    int topreg = reg + num - 1;

		    func (stream, dis_style_text, "{");
		    if (!num)
		      {
			/* Nothing.  */
		      }
		    else if (num == 1)
		      {
			func (stream, dis_style_register,
			      "%c%d", reg_prefix, reg);
			func (stream, dis_style_text, ", ");
		      }
		    else if (topreg > maxreg)
		      {
			func (stream, dis_style_register, "%c%d",
			      reg_prefix, reg);
			func (stream, dis_style_text, "-<overflow reg d%d, ",
			      single ? topreg >> 1 : topreg);
		      }
		    else
		      {
			func (stream, dis_style_register,
			      "%c%d", reg_prefix, reg);
			func (stream, dis_style_text, "-");
			func (stream, dis_style_register, "%c%d",
			      reg_prefix, topreg);
			func (stream, dis_style_text, ", ");
		      }
		    func (stream, dis_style_register, "VPR");
		    func (stream, dis_style_text, "}");
		  }
		  break;

		case 'u':
		  if (cond != COND_UNCOND)
		    is_unpredictable = true;

		  /* Fall through.  */
		case 'c':
		  if (cond != COND_UNCOND && cp_num == 9)
		    is_unpredictable = true;

		  /* Fall through.  */
		case 'b':
		  func (stream, dis_style_mnemonic, "%s",
			arm_conditional[cond]);
		  break;

		case 'I':
		  /* Print a Cirrus/DSP shift immediate.  */
		  /* Immediates are 7bit signed ints with bits 0..3 in
		     bits 0..3 of opcode and bits 4..6 in bits 5..7
		     of opcode.  */
		  {
		    int imm;

		    imm = (given & 0xf) | ((given & 0xe0) >> 1);

		    /* Is ``imm'' a negative number?  */
		    if (imm & 0x40)
		      imm -= 0x80;

		    func (stream, dis_style_immediate, "%d", imm);
		  }

		  break;

		case 'J':
		  {
		    unsigned long regno
		      = arm_decode_field_multiple (given, 13, 15, 22, 22);

		    switch (regno)
		      {
		      case 0x1:
			func (stream, dis_style_register, "FPSCR");
			break;
		      case 0x2:
			func (stream, dis_style_register, "FPSCR_nzcvqc");
			break;
		      case 0xc:
			func (stream, dis_style_register, "VPR");
			break;
		      case 0xd:
			func (stream, dis_style_register, "P0");
			break;
		      case 0xe:
			func (stream, dis_style_register, "FPCXTNS");
			break;
		      case 0xf:
			func (stream, dis_style_register, "FPCXTS");
			break;
		      default:
			func (stream, dis_style_text, "<invalid reg %lu>",
			      regno);
			break;
		      }
		  }
		  break;

		case 'F':
		  switch (given & 0x00408000)
		    {
		    case 0:
		      func (stream, dis_style_immediate, "4");
		      break;
		    case 0x8000:
		      func (stream, dis_style_immediate, "1");
		      break;
		    case 0x00400000:
		      func (stream, dis_style_immediate, "2");
		      break;
		    default:
		      func (stream, dis_style_immediate, "3");
		    }
		  break;

		case 'P':
		  switch (given & 0x00080080)
		    {
		    case 0:
		      func (stream, dis_style_mnemonic, "s");
		      break;
		    case 0x80:
		      func (stream, dis_style_mnemonic, "d");
		      break;
		    case 0x00080000:
		      func (stream, dis_style_mnemonic, "e");
		      break;
		    default:
		      func (stream, dis_style_text, _("<illegal precision>"));
		      break;
		    }
		  break;

		case 'Q':
		  switch (given & 0x00408000)
		    {
		    case 0:
		      func (stream, dis_style_mnemonic, "s");
		      break;
		    case 0x8000:
		      func (stream, dis_style_mnemonic, "d");
		      break;
		    case 0x00400000:
		      func (stream, dis_style_mnemonic, "e");
		      break;
		    default:
		      func (stream, dis_style_mnemonic, "p");
		      break;
		    }
		  break;

		case 'R':
		  switch (given & 0x60)
		    {
		    case 0:
		      break;
		    case 0x20:
		      func (stream, dis_style_mnemonic, "p");
		      break;
		    case 0x40:
		      func (stream, dis_style_mnemonic, "m");
		      break;
		    default:
		      func (stream, dis_style_mnemonic, "z");
		      break;
		    }
		  break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		  {
		    int width;

		    c = arm_decode_bitfield (c, given, &value, &width);

		    switch (*c)
		      {
		      case 'R':
			if (value == 15)
			  is_unpredictable = true;
			/* Fall through.  */
		      case 'r':
			if (c[1] == 'u')
			  {
			    /* Eat the 'u' character.  */
			    ++ c;

			    if (u_reg == value)
			      is_unpredictable = true;
			    u_reg = value;
			  }
			func (stream, dis_style_register, "%s",
			      arm_regnames[value]);
			break;
		      case 'V':
			if (given & (1 << 6))
			  goto Q;
			/* FALLTHROUGH */
		      case 'D':
			func (stream, dis_style_register, "d%ld", value);
			break;
		      case 'Q':
		      Q:
			if (value & 1)
			  func (stream, dis_style_text,
				"<illegal reg q%ld.5>", value >> 1);
			else
			  func (stream, dis_style_register,
				"q%ld", value >> 1);
			break;
		      case 'd':
			func (stream, base_style, "%ld", value);
			value_in_comment = value;
			break;
		      case 'E':
                        {
			  /* Converts immediate 8 bit back to float value.  */
			  unsigned floatVal = (value & 0x80) << 24
			    | (value & 0x3F) << 19
			    | ((value & 0x40) ? (0xF8 << 22) : (1 << 30));

			  /* Quarter float have a maximum value of 31.0.
			     Get floating point value multiplied by 1e7.
			     The maximum value stays in limit of a 32-bit int.  */
			  unsigned decVal =
			    (78125 << (((floatVal >> 23) & 0xFF) - 124)) *
			    (16 + (value & 0xF));

			  if (!(decVal % 1000000))
			    {
			      func (stream, dis_style_immediate, "%ld", value);
			      func (stream, dis_style_comment_start,
				    "\t@ 0x%08x %c%u.%01u",
				    floatVal, value & 0x80 ? '-' : ' ',
				    decVal / 10000000,
				    decVal % 10000000 / 1000000);
			    }
			  else if (!(decVal % 10000))
			    {
			      func (stream, dis_style_immediate, "%ld", value);
			      func (stream, dis_style_comment_start,
				    "\t@ 0x%08x %c%u.%03u",
				    floatVal, value & 0x80 ? '-' : ' ',
				    decVal / 10000000,
				    decVal % 10000000 / 10000);
			    }
			  else
			    {
			      func (stream, dis_style_immediate, "%ld", value);
			      func (stream, dis_style_comment_start,
				    "\t@ 0x%08x %c%u.%07u",
				    floatVal, value & 0x80 ? '-' : ' ',
				    decVal / 10000000, decVal % 10000000);
			    }
			  break;
			}
		      case 'k':
			{
			  int from = (given & (1 << 7)) ? 32 : 16;
			  func (stream, dis_style_immediate, "%ld",
				from - value);
			}
			break;

		      case 'f':
			if (value > 7)
			  func (stream, dis_style_immediate, "#%s",
				arm_fp_const[value & 7]);
			else
			  func (stream, dis_style_register, "f%ld", value);
			break;

		      case 'w':
			if (width == 2)
			  func (stream, dis_style_mnemonic, "%s",
				iwmmxt_wwnames[value]);
			else
			  func (stream, dis_style_mnemonic, "%s",
				iwmmxt_wwssnames[value]);
			break;

		      case 'g':
			func (stream, dis_style_register, "%s",
			      iwmmxt_regnames[value]);
			break;
		      case 'G':
			func (stream, dis_style_register, "%s",
			      iwmmxt_cregnames[value]);
			break;

		      case 'x':
			func (stream, dis_style_immediate, "0x%lx",
			      (value & 0xffffffffUL));
			break;

		      case 'c':
			switch (value)
			  {
			  case 0:
			    func (stream, dis_style_mnemonic, "eq");
			    break;

			  case 1:
			    func (stream, dis_style_mnemonic, "vs");
			    break;

			  case 2:
			    func (stream, dis_style_mnemonic, "ge");
			    break;

			  case 3:
			    func (stream, dis_style_mnemonic, "gt");
			    break;

			  default:
			    func (stream, dis_style_text, "??");
			    break;
			  }
			break;

		      case '`':
			c++;
			if (value == 0)
			  func (stream, dis_style_mnemonic, "%c", *c);
			break;
		      case '\'':
			c++;
			if (value == ((1ul << width) - 1))
			  func (stream, base_style, "%c", *c);
			break;
		      case '?':
			func (stream, base_style, "%c",
			      c[(1 << width) - (int) value]);
			c += 1 << width;
			break;
		      default:
			abort ();
		      }
		  }
		  break;

		case 'y':
		case 'z':
		  {
		    int single = *c++ == 'y';
		    int regno;

		    switch (*c)
		      {
		      case '4': /* Sm pair */
		      case '0': /* Sm, Dm */
			regno = given & 0x0000000f;
			if (single)
			  {
			    regno <<= 1;
			    regno += (given >> 5) & 1;
			  }
			else
			  regno += ((given >> 5) & 1) << 4;
			break;

		      case '1': /* Sd, Dd */
			regno = (given >> 12) & 0x0000000f;
			if (single)
			  {
			    regno <<= 1;
			    regno += (given >> 22) & 1;
			  }
			else
			  regno += ((given >> 22) & 1) << 4;
			break;

		      case '2': /* Sn, Dn */
			regno = (given >> 16) & 0x0000000f;
			if (single)
			  {
			    regno <<= 1;
			    regno += (given >> 7) & 1;
			  }
			else
			  regno += ((given >> 7) & 1) << 4;
			break;

		      case '3': /* List */
			func (stream, dis_style_text, "{");
			regno = (given >> 12) & 0x0000000f;
			if (single)
			  {
			    regno <<= 1;
			    regno += (given >> 22) & 1;
			  }
			else
			  regno += ((given >> 22) & 1) << 4;
			break;

		      default:
			abort ();
		      }

		    func (stream, dis_style_register, "%c%d",
			  single ? 's' : 'd', regno);

		    if (*c == '3')
		      {
			int count = given & 0xff;

			if (single == 0)
			  count >>= 1;

			if (--count)
			  {
			    func (stream, dis_style_text, "-");
			    func (stream, dis_style_register, "%c%d",
				  single ? 's' : 'd',
				  regno + count);
			  }

			func (stream, dis_style_text, "}");
		      }
		    else if (*c == '4')
		      {
			func (stream, dis_style_text, ", ");
			func (stream, dis_style_register, "%c%d",
			      single ? 's' : 'd', regno + 1);
		      }
		  }
		  break;

		case 'L':
		  switch (given & 0x00400100)
		    {
		    case 0x00000000:
		      func (stream, dis_style_mnemonic, "b");
		      break;
		    case 0x00400000:
		      func (stream, dis_style_mnemonic, "h");
		      break;
		    case 0x00000100:
		      func (stream, dis_style_mnemonic, "w");
		      break;
		    case 0x00400100:
		      func (stream, dis_style_mnemonic, "d");
		      break;
		    default:
		      break;
		    }
		  break;

		case 'Z':
		  {
		    /* given (20, 23) | given (0, 3) */
		    value = ((given >> 16) & 0xf0) | (given & 0xf);
		    func (stream, dis_style_immediate, "%d", (int) value);
		  }
		  break;

		case 'l':
		  /* This is like the 'A' operator, except that if
		     the width field "M" is zero, then the offset is
		     *not* multiplied by four.  */
		  {
		    int offset = given & 0xff;
		    int multiplier = (given & 0x00000100) ? 4 : 1;

		    func (stream, dis_style_text, "[");
		    func (stream, dis_style_register, "%s",
			  arm_regnames [(given >> 16) & 0xf]);

		    if (multiplier > 1)
		      {
			value_in_comment = offset * multiplier;
			if (NEGATIVE_BIT_SET)
			  value_in_comment = - value_in_comment;
		      }

		    if (offset)
		      {
			if (PRE_BIT_SET)
			  {
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_immediate, "#%s%d",
				  NEGATIVE_BIT_SET ? "-" : "",
				  offset * multiplier);
			    func (stream, dis_style_text, "]%s",
				  WRITEBACK_BIT_SET ? "!" : "");
			  }
			else
			  {
			    func (stream, dis_style_text, "], ");
			    func (stream, dis_style_immediate, "#%s%d",
				  NEGATIVE_BIT_SET ? "-" : "",
				  offset * multiplier);
			  }
		      }
		    else
		      func (stream, dis_style_text, "]");
		  }
		  break;

		case 'r':
		  {
		    int imm4 = (given >> 4) & 0xf;
		    int puw_bits = ((given >> 22) & 6) | ((given >> W_BIT) & 1);
		    int ubit = ! NEGATIVE_BIT_SET;
		    const char *rm = arm_regnames [given & 0xf];
		    const char *rn = arm_regnames [(given >> 16) & 0xf];

		    switch (puw_bits)
		      {
		      case 1:
		      case 3:
			func (stream, dis_style_text, "[");
			func (stream, dis_style_register, "%s", rn);
			func (stream, dis_style_text, "], ");
			func (stream, dis_style_text, "%c", ubit ? '+' : '-');
			func (stream, dis_style_register, "%s", rm);
			if (imm4)
			  {
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_sub_mnemonic, "lsl ");
			    func (stream, dis_style_immediate, "#%d", imm4);
			  }
			break;

		      case 4:
		      case 5:
		      case 6:
		      case 7:
			func (stream, dis_style_text, "[");
			func (stream, dis_style_register, "%s", rn);
			func (stream, dis_style_text, ", ");
			func (stream, dis_style_text, "%c", ubit ? '+' : '-');
			func (stream, dis_style_register, "%s", rm);
			if (imm4 > 0)
			  {
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_sub_mnemonic, "lsl ");
			    func (stream, dis_style_immediate, "#%d", imm4);
			  }
			func (stream, dis_style_text, "]");
			if (puw_bits == 5 || puw_bits == 7)
			  func (stream, dis_style_text, "!");
			break;

		      default:
			func (stream, dis_style_text, "INVALID");
		      }
		  }
		  break;

		case 'i':
		  {
		    long imm5;
		    imm5 = ((given & 0x100) >> 4) | (given & 0xf);
		    func (stream, dis_style_immediate, "%ld",
			  (imm5 == 0) ? 32 : imm5);
		  }
		  break;

		default:
		  abort ();
		}
	    }
	  else
	    {
	      if (*c == '@')
		base_style = dis_style_comment_start;

	      if (*c == '\t')
		base_style = dis_style_text;

	      func (stream, base_style, "%c", *c);
	    }
	}

      if (value_in_comment > 32 || value_in_comment < -16)
	func (stream, dis_style_comment_start, "\t@ 0x%lx",
	      (value_in_comment & 0xffffffffUL));

      if (is_unpredictable)
	func (stream, dis_style_comment_start, UNPREDICTABLE_INSTRUCTION);

      return true;
    }
  return false;
}

static bool
print_insn_coprocessor (bfd_vma pc,
			struct disassemble_info *info,
			long given,
			bool thumb)
{
  return print_insn_coprocessor_1 (coprocessor_opcodes,
				   pc, info, given, thumb);
}

static bool
print_insn_generic_coprocessor (bfd_vma pc,
				struct disassemble_info *info,
				long given,
				bool thumb)
{
  return print_insn_coprocessor_1 (generic_coprocessor_opcodes,
				   pc, info, given, thumb);
}

/* Decodes and prints ARM addressing modes.  Returns the offset
   used in the address, if any, if it is worthwhile printing the
   offset as a hexadecimal value in a comment at the end of the
   line of disassembly.  */

static signed long
print_arm_address (bfd_vma pc, struct disassemble_info *info, long given)
{
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  bfd_vma offset = 0;

  if (((given & 0x000f0000) == 0x000f0000)
      && ((given & 0x02000000) == 0))
    {
      offset = given & 0xfff;

      func (stream, dis_style_text, "[");
      func (stream, dis_style_register, "pc");

      if (PRE_BIT_SET)
	{
	  /* Pre-indexed.  Elide offset of positive zero when
	     non-writeback.  */
	  if (WRITEBACK_BIT_SET || NEGATIVE_BIT_SET || offset)
	    {
	      func (stream, dis_style_text, ", ");
	      func (stream, dis_style_immediate, "#%s%d",
		    NEGATIVE_BIT_SET ? "-" : "", (int) offset);
	    }

	  if (NEGATIVE_BIT_SET)
	    offset = -offset;

	  offset += pc + 8;

	  /* Cope with the possibility of write-back
	     being used.  Probably a very dangerous thing
	     for the programmer to do, but who are we to
	     argue ?  */
	  func (stream, dis_style_text, "]%s", WRITEBACK_BIT_SET ? "!" : "");
	}
      else  /* Post indexed.  */
	{
	  func (stream, dis_style_text, "], ");
	  func (stream, dis_style_immediate, "#%s%d",
		NEGATIVE_BIT_SET ? "-" : "", (int) offset);

	  /* Ie ignore the offset.  */
	  offset = pc + 8;
	}

      func (stream, dis_style_comment_start, "\t@ ");
      info->print_address_func (offset, info);
      offset = 0;
    }
  else
    {
      func (stream, dis_style_text, "[");
      func (stream, dis_style_register, "%s",
	    arm_regnames[(given >> 16) & 0xf]);

      if (PRE_BIT_SET)
	{
	  if ((given & 0x02000000) == 0)
	    {
	      /* Elide offset of positive zero when non-writeback.  */
	      offset = given & 0xfff;
	      if (WRITEBACK_BIT_SET || NEGATIVE_BIT_SET || offset)
		{
		  func (stream, dis_style_text, ", ");
		  func (stream, dis_style_immediate, "#%s%d",
			NEGATIVE_BIT_SET ? "-" : "", (int) offset);
		}
	    }
	  else
	    {
	      func (stream, dis_style_text, ", %s",
		    NEGATIVE_BIT_SET ? "-" : "");
	      arm_decode_shift (given, func, stream, true);
	    }

	  func (stream, dis_style_text, "]%s",
		WRITEBACK_BIT_SET ? "!" : "");
	}
      else
	{
	  if ((given & 0x02000000) == 0)
	    {
	      /* Always show offset.  */
	      offset = given & 0xfff;
	      func (stream, dis_style_text, "], ");
	      func (stream, dis_style_immediate, "#%s%d",
		    NEGATIVE_BIT_SET ? "-" : "", (int) offset);
	    }
	  else
	    {
	      func (stream, dis_style_text, "], %s",
		    NEGATIVE_BIT_SET ? "-" : "");
	      arm_decode_shift (given, func, stream, true);
	    }
	}
      if (NEGATIVE_BIT_SET)
	offset = -offset;
    }

  return (signed long) offset;
}


/* Print one cde instruction on INFO->STREAM.
   Return TRUE if the instuction matched, FALSE if this is not a
   recognised cde instruction.  */
static bool
print_insn_cde (struct disassemble_info *info, long given, bool thumb)
{
  const struct cdeopcode32 *insn;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  enum disassembler_style base_style = dis_style_mnemonic;
  enum disassembler_style old_base_style = base_style;

  if (thumb)
  {
    /* Manually extract the coprocessor code from a known point.
       This position is the same across all CDE instructions.  */
    for (insn = cde_opcodes; insn->assembler; insn++)
    {
      uint16_t coproc = (given >> insn->coproc_shift) & insn->coproc_mask;
      uint16_t coproc_mask = 1 << coproc;
      if (! (coproc_mask & cde_coprocs))
	continue;

      if ((given & insn->mask) == insn->value)
      {
	bool is_unpredictable = false;
	const char *c;

	for (c = insn->assembler; *c; c++)
	{
	  if (*c == '%')
	  {
	    switch (*++c)
	    {
	      case '{':
		++c;
		if (*c == '\0')
		  abort ();
		old_base_style = base_style;
		base_style = decode_base_style (*c);
		++c;
		if (*c != ':')
		  abort ();
		break;

	      case '}':
		base_style = old_base_style;
		break;

	      case '%':
		func (stream, base_style, "%%");
		break;

	      case '0': case '1': case '2': case '3': case '4':
	      case '5': case '6': case '7': case '8': case '9':
	      {
		int width;
		unsigned long value;

		c = arm_decode_bitfield (c, given, &value, &width);

		switch (*c)
		{
		  case 'S':
		    if (value > 10)
		      is_unpredictable = true;
		    /* Fall through.  */
		  case 'R':
		    if (value == 13)
		      is_unpredictable = true;
		    /* Fall through.  */
		  case 'r':
		    func (stream, dis_style_register, "%s",
			  arm_regnames[value]);
		    break;

		  case 'n':
		    if (value == 15)
		      func (stream, dis_style_register, "%s", "APSR_nzcv");
		    else
		      func (stream, dis_style_register, "%s",
			    arm_regnames[value]);
		    break;

		  case 'T':
		    func (stream, dis_style_register, "%s",
			  arm_regnames[(value + 1) & 15]);
		    break;

		  case 'd':
		    func (stream, dis_style_immediate, "%ld", value);
		    break;

		  case 'V':
		    if (given & (1 << 6))
		      func (stream, dis_style_register, "q%ld", value >> 1);
		    else if (given & (1 << 24))
		      func (stream, dis_style_register, "d%ld", value);
		    else
		      {
			/* Encoding for S register is different than for D and
			   Q registers.  S registers are encoded using the top
			   single bit in position 22 as the lowest bit of the
			   register number, while for Q and D it represents the
			   highest bit of the register number.  */
			uint8_t top_bit = (value >> 4) & 1;
			uint8_t tmp = (value << 1) & 0x1e;
			uint8_t res = tmp | top_bit;
			func (stream, dis_style_register, "s%u", res);
		      }
		    break;

		default:
		  abort ();
		}
	      }
	    break;

	    case 'p':
	      {
		uint8_t proc_number = (given >> 8) & 0x7;
		func (stream, dis_style_register, "p%u", proc_number);
		break;
	      }

	    case 'a':
	      {
		uint8_t a_offset = 28;
		if (given & (1 << a_offset))
		  func (stream, dis_style_mnemonic, "a");
		break;
	      }
	  default:
	    abort ();
	  }
	}
	else
	  {
	    if (*c == '@')
	      base_style = dis_style_comment_start;
	    if (*c == '\t')
	      base_style = dis_style_text;

	    func (stream, base_style, "%c", *c);
	  }
      }

      if (is_unpredictable)
	func (stream, dis_style_comment_start, UNPREDICTABLE_INSTRUCTION);

      return true;
      }
    }
    return false;
  }
  else
    return false;
}


/* Print one neon instruction on INFO->STREAM.
   Return TRUE if the instuction matched, FALSE if this is not a
   recognised neon instruction.  */

static bool
print_insn_neon (struct disassemble_info *info, long given, bool thumb)
{
  const struct opcode32 *insn;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  enum disassembler_style base_style = dis_style_mnemonic;
  enum disassembler_style old_base_style = base_style;

  if (thumb)
    {
      if ((given & 0xef000000) == 0xef000000)
	{
	  /* Move bit 28 to bit 24 to translate Thumb2 to ARM encoding.  */
	  unsigned long bit28 = given & (1 << 28);

	  given &= 0x00ffffff;
	  if (bit28)
            given |= 0xf3000000;
          else
	    given |= 0xf2000000;
	}
      else if ((given & 0xff000000) == 0xf9000000)
	given ^= 0xf9000000 ^ 0xf4000000;
      /* BFloat16 neon instructions without special top byte handling.  */
      else if ((given & 0xff000000) == 0xfe000000
	       || (given & 0xff000000) == 0xfc000000)
	;
      /* vdup is also a valid neon instruction.  */
      else if ((given & 0xff900f5f) != 0xee800b10)
	return false;
    }

  for (insn = neon_opcodes; insn->assembler; insn++)
    {
      unsigned long cond_mask = insn->mask;
      unsigned long cond_value = insn->value;
      int cond;

      if (thumb)
        {
          if ((cond_mask & 0xf0000000) == 0) {
              /* For the entries in neon_opcodes, an opcode mask/value with
                 the high 4 bits equal to 0 indicates a conditional
                 instruction. For thumb however, we need to include those
                 bits in the instruction matching.  */
              cond_mask |= 0xf0000000;
              /* Furthermore, the thumb encoding of a conditional instruction
                 will have the high 4 bits equal to 0xe.  */
              cond_value |= 0xe0000000;
          }
          if (ifthen_state)
            cond = IFTHEN_COND;
          else
            cond = COND_UNCOND;
        }
      else
        {
          if ((given & 0xf0000000) == 0xf0000000)
            {
              /* If the instruction is unconditional, update the mask to only
                 match against unconditional opcode values.  */
              cond_mask |= 0xf0000000;
              cond = COND_UNCOND;
            }
          else
            {
              cond = (given >> 28) & 0xf;
              if (cond == 0xe)
                cond = COND_UNCOND;
            }
        }

      if ((given & cond_mask) == cond_value)
	{
	  signed long value_in_comment = 0;
	  bool is_unpredictable = false;
	  const char *c;

	  for (c = insn->assembler; *c; c++)
	    {
	      if (*c == '%')
		{
		  switch (*++c)
		    {
		    case '{':
		      ++c;
		      if (*c == '\0')
			abort ();
		      old_base_style = base_style;
		      base_style = decode_base_style (*c);
		      ++c;
		      if (*c != ':')
			abort ();
		      break;

		    case '}':
		      base_style = old_base_style;
		      break;

		    case '%':
		      func (stream, base_style, "%%");
		      break;

		    case 'u':
		      if (thumb && ifthen_state)
			is_unpredictable = true;

		      /* Fall through.  */
		    case 'c':
		      func (stream, dis_style_mnemonic, "%s",
			    arm_conditional[cond]);
		      break;

		    case 'A':
		      {
			static const unsigned char enc[16] =
			{
			  0x4, 0x14, /* st4 0,1 */
			  0x4, /* st1 2 */
			  0x4, /* st2 3 */
			  0x3, /* st3 4 */
			  0x13, /* st3 5 */
			  0x3, /* st1 6 */
			  0x1, /* st1 7 */
			  0x2, /* st2 8 */
			  0x12, /* st2 9 */
			  0x2, /* st1 10 */
			  0, 0, 0, 0, 0
			};
			int rd = ((given >> 12) & 0xf) | (((given >> 22) & 1) << 4);
			int rn = ((given >> 16) & 0xf);
			int rm = ((given >> 0) & 0xf);
			int align = ((given >> 4) & 0x3);
			int type = ((given >> 8) & 0xf);
			int n = enc[type] & 0xf;
			int stride = (enc[type] >> 4) + 1;
			int ix;

			func (stream, dis_style_text, "{");
			if (stride > 1)
			  for (ix = 0; ix != n; ix++)
			    {
			      if (ix > 0)
				func (stream, dis_style_text, ",");
			      func (stream, dis_style_register, "d%d",
				    rd + ix * stride);
			    }
			else if (n == 1)
			  func (stream, dis_style_register, "d%d", rd);
			else
			  {
			    func (stream, dis_style_register, "d%d", rd);
			    func (stream, dis_style_text, "-");
			    func (stream, dis_style_register, "d%d",
				  rd + n - 1);
			  }
			func (stream, dis_style_text, "}, [");
			func (stream, dis_style_register, "%s",
			      arm_regnames[rn]);
			if (align)
			  {
			    func (stream, dis_style_text, " :");
			    func (stream, dis_style_immediate, "%d",
				  32 << align);
			  }
			func (stream, dis_style_text, "]");
			if (rm == 0xd)
			  func (stream, dis_style_text, "!");
			else if (rm != 0xf)
			  {
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_register, "%s",
				  arm_regnames[rm]);
			  }
		      }
		      break;

		    case 'B':
		      {
			int rd = ((given >> 12) & 0xf) | (((given >> 22) & 1) << 4);
			int rn = ((given >> 16) & 0xf);
			int rm = ((given >> 0) & 0xf);
			int idx_align = ((given >> 4) & 0xf);
                        int align = 0;
			int size = ((given >> 10) & 0x3);
			int idx = idx_align >> (size + 1);
                        int length = ((given >> 8) & 3) + 1;
                        int stride = 1;
                        int i;

                        if (length > 1 && size > 0)
                          stride = (idx_align & (1 << size)) ? 2 : 1;

                        switch (length)
                          {
                          case 1:
                            {
                              int amask = (1 << size) - 1;
                              if ((idx_align & (1 << size)) != 0)
                                return false;
                              if (size > 0)
                                {
                                  if ((idx_align & amask) == amask)
                                    align = 8 << size;
                                  else if ((idx_align & amask) != 0)
                                    return false;
                                }
                              }
                            break;

                          case 2:
                            if (size == 2 && (idx_align & 2) != 0)
                              return false;
                            align = (idx_align & 1) ? 16 << size : 0;
                            break;

                          case 3:
                            if ((size == 2 && (idx_align & 3) != 0)
                                || (idx_align & 1) != 0)
                              return false;
                            break;

                          case 4:
                            if (size == 2)
                              {
                                if ((idx_align & 3) == 3)
                                  return false;
                                align = (idx_align & 3) * 64;
                              }
                            else
                              align = (idx_align & 1) ? 32 << size : 0;
                            break;

                          default:
                            abort ();
                          }

			func (stream, dis_style_text, "{");
                        for (i = 0; i < length; i++)
			  {
			    if (i > 0)
			      func (stream, dis_style_text, ",");
			    func (stream, dis_style_register, "d%d[%d]",
				  rd + i * stride, idx);
			  }
			func (stream, dis_style_text, "}, [");
			func (stream, dis_style_register, "%s",
			      arm_regnames[rn]);
			if (align)
			  {
			    func (stream, dis_style_text, " :");
			    func (stream, dis_style_immediate, "%d", align);
			  }
			func (stream, dis_style_text, "]");
			if (rm == 0xd)
			  func (stream, dis_style_text, "!");
			else if (rm != 0xf)
			  {
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_register, "%s",
				  arm_regnames[rm]);
			  }
		      }
		      break;

		    case 'C':
		      {
			int rd = ((given >> 12) & 0xf) | (((given >> 22) & 1) << 4);
			int rn = ((given >> 16) & 0xf);
			int rm = ((given >> 0) & 0xf);
			int align = ((given >> 4) & 0x1);
			int size = ((given >> 6) & 0x3);
			int type = ((given >> 8) & 0x3);
			int n = type + 1;
			int stride = ((given >> 5) & 0x1);
			int ix;

			if (stride && (n == 1))
			  n++;
			else
			  stride++;

			func (stream, dis_style_text, "{");
			if (stride > 1)
			  for (ix = 0; ix != n; ix++)
			    {
			      if (ix > 0)
				func (stream, dis_style_text, ",");
			      func (stream, dis_style_register, "d%d[]",
				    rd + ix * stride);
			    }
			else if (n == 1)
			  func (stream, dis_style_register, "d%d[]", rd);
			else
			  {
			    func (stream, dis_style_register, "d%d[]", rd);
			    func (stream, dis_style_text, "-");
			    func (stream, dis_style_register, "d%d[]",
				  rd + n - 1);
			  }
			func (stream, dis_style_text, "}, [");
			func (stream, dis_style_register, "%s",
			      arm_regnames[rn]);
			if (align)
			  {
                            align = (8 * (type + 1)) << size;
                            if (type == 3)
                              align = (size > 1) ? align >> 1 : align;
			    if (type == 2 || (type == 0 && !size))
			      func (stream, dis_style_text,
				    " :<bad align %d>", align);
			    else
			      {
				func (stream, dis_style_text, " :");
				func (stream, dis_style_immediate,
				      "%d", align);
			      }
			  }
			func (stream, dis_style_text, "]");
			if (rm == 0xd)
			  func (stream, dis_style_text, "!");
			else if (rm != 0xf)
			  {
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_register, "%s",
				  arm_regnames[rm]);
			  }
		      }
		      break;

		    case 'D':
		      {
			int raw_reg = (given & 0xf) | ((given >> 1) & 0x10);
			int size = (given >> 20) & 3;
			int reg = raw_reg & ((4 << size) - 1);
			int ix = raw_reg >> size >> 2;

			func (stream, dis_style_register, "d%d[%d]", reg, ix);
		      }
		      break;

		    case 'E':
		      /* Neon encoded constant for mov, mvn, vorr, vbic.  */
		      {
			int bits = 0;
			int cmode = (given >> 8) & 0xf;
			int op = (given >> 5) & 0x1;
			unsigned long value = 0, hival = 0;
			unsigned shift;
                        int size = 0;
                        int isfloat = 0;

			bits |= ((given >> 24) & 1) << 7;
			bits |= ((given >> 16) & 7) << 4;
			bits |= ((given >> 0) & 15) << 0;

			if (cmode < 8)
			  {
			    shift = (cmode >> 1) & 3;
			    value = (unsigned long) bits << (8 * shift);
                            size = 32;
			  }
			else if (cmode < 12)
			  {
			    shift = (cmode >> 1) & 1;
			    value = (unsigned long) bits << (8 * shift);
                            size = 16;
			  }
			else if (cmode < 14)
			  {
			    shift = (cmode & 1) + 1;
			    value = (unsigned long) bits << (8 * shift);
			    value |= (1ul << (8 * shift)) - 1;
                            size = 32;
			  }
			else if (cmode == 14)
			  {
			    if (op)
			      {
				/* Bit replication into bytes.  */
				int ix;
				unsigned long mask;

				value = 0;
                                hival = 0;
				for (ix = 7; ix >= 0; ix--)
				  {
				    mask = ((bits >> ix) & 1) ? 0xff : 0;
                                    if (ix <= 3)
				      value = (value << 8) | mask;
                                    else
                                      hival = (hival << 8) | mask;
				  }
                                size = 64;
			      }
                            else
                              {
                                /* Byte replication.  */
                                value = (unsigned long) bits;
                                size = 8;
                              }
			  }
			else if (!op)
			  {
			    /* Floating point encoding.  */
			    int tmp;

			    value = (unsigned long)  (bits & 0x7f) << 19;
			    value |= (unsigned long) (bits & 0x80) << 24;
			    tmp = bits & 0x40 ? 0x3c : 0x40;
			    value |= (unsigned long) tmp << 24;
                            size = 32;
                            isfloat = 1;
			  }
			else
			  {
			    func (stream, dis_style_text,
				  "<illegal constant %.8x:%x:%x>",
                                  bits, cmode, op);
                            size = 32;
			    break;
			  }
                        switch (size)
                          {
                          case 8:
			    func (stream, dis_style_immediate, "#%ld", value);
			    func (stream, dis_style_comment_start,
				  "\t@ 0x%.2lx", value);
                            break;

                          case 16:
			    func (stream, dis_style_immediate, "#%ld", value);
			    func (stream, dis_style_comment_start,
				  "\t@ 0x%.4lx", value);
                            break;

                          case 32:
                            if (isfloat)
                              {
                                unsigned char valbytes[4];
                                double fvalue;

                                /* Do this a byte at a time so we don't have to
                                   worry about the host's endianness.  */
                                valbytes[0] = value & 0xff;
                                valbytes[1] = (value >> 8) & 0xff;
                                valbytes[2] = (value >> 16) & 0xff;
                                valbytes[3] = (value >> 24) & 0xff;

                                floatformat_to_double
                                  (& floatformat_ieee_single_little, valbytes,
                                  & fvalue);

				func (stream, dis_style_immediate,
				      "#%.7g", fvalue);
				func (stream, dis_style_comment_start,
				      "\t@ 0x%.8lx", value);
                              }
                            else
			      {
				func (stream, dis_style_immediate, "#%ld",
				      (long) (((value & 0x80000000L) != 0)
					      ? value | ~0xffffffffL : value));
				func (stream, dis_style_comment_start,
				      "\t@ 0x%.8lx", value);
			      }
                            break;

                          case 64:
			    func (stream, dis_style_immediate,
				  "#0x%.8lx%.8lx", hival, value);
                            break;

                          default:
                            abort ();
                          }
		      }
		      break;

		    case 'F':
		      {
			int regno = ((given >> 16) & 0xf) | ((given >> (7 - 4)) & 0x10);
			int num = (given >> 8) & 0x3;

			func (stream, dis_style_text, "{");
			if (!num)
			  func (stream, dis_style_register, "d%d", regno);
			else if (num + regno >= 32)
			  {
			    func (stream, dis_style_register, "d%d", regno);
			    func (stream, dis_style_text, "-<overflow reg d%d",
				  regno + num);
			  }
			else
			  {
			    func (stream, dis_style_register, "d%d", regno);
			    func (stream, dis_style_text, "-");
			    func (stream, dis_style_register, "d%d",
				  regno + num);
			  }
			func (stream, dis_style_text, "}");
		      }
		      break;


		    case '0': case '1': case '2': case '3': case '4':
		    case '5': case '6': case '7': case '8': case '9':
		      {
			int width;
			unsigned long value;

			c = arm_decode_bitfield (c, given, &value, &width);

			switch (*c)
			  {
			  case 'r':
			    func (stream, dis_style_register, "%s",
				  arm_regnames[value]);
			    break;
			  case 'd':
			    func (stream, base_style, "%ld", value);
			    value_in_comment = value;
			    break;
			  case 'e':
			    func (stream, dis_style_immediate, "%ld",
				  (1ul << width) - value);
			    break;

			  case 'S':
			  case 'T':
			  case 'U':
			    /* Various width encodings.  */
			    {
			      int base = 8 << (*c - 'S'); /* 8,16 or 32 */
			      int limit;
			      unsigned low, high;

			      c++;
			      if (*c >= '0' && *c <= '9')
				limit = *c - '0';
			      else if (*c >= 'a' && *c <= 'f')
				limit = *c - 'a' + 10;
			      else
				abort ();
			      low = limit >> 2;
			      high = limit & 3;

			      if (value < low || value > high)
				func (stream, dis_style_text,
				      "<illegal width %d>", base << value);
			      else
				func (stream, base_style, "%d",
				      base << value);
			    }
			    break;
			  case 'R':
			    if (given & (1 << 6))
			      goto Q;
			    /* FALLTHROUGH */
			  case 'D':
			    func (stream, dis_style_register, "d%ld", value);
			    break;
			  case 'Q':
			  Q:
			    if (value & 1)
			      func (stream, dis_style_text,
				    "<illegal reg q%ld.5>", value >> 1);
			    else
			      func (stream, dis_style_register,
				    "q%ld", value >> 1);
			    break;

			  case '`':
			    c++;
			    if (value == 0)
			      func (stream, dis_style_text, "%c", *c);
			    break;
			  case '\'':
			    c++;
			    if (value == ((1ul << width) - 1))
			      func (stream, dis_style_text, "%c", *c);
			    break;
			  case '?':
			    func (stream, dis_style_mnemonic, "%c",
				  c[(1 << width) - (int) value]);
			    c += 1 << width;
			    break;
			  default:
			    abort ();
			  }
		      }
		      break;

		    default:
		      abort ();
		    }
		}
	      else
		{
		  if (*c == '@')
		    base_style = dis_style_comment_start;

		  if (*c == '\t')
		    base_style = dis_style_text;

		  func (stream, base_style, "%c", *c);

		}
	    }

	  if (value_in_comment > 32 || value_in_comment < -16)
	    func (stream, dis_style_comment_start, "\t@ 0x%lx",
		  value_in_comment);

	  if (is_unpredictable)
	    func (stream, dis_style_comment_start, UNPREDICTABLE_INSTRUCTION);

	  return true;
	}
    }
  return false;
}

/* Print one mve instruction on INFO->STREAM.
   Return TRUE if the instuction matched, FALSE if this is not a
   recognised mve instruction.  */

static bool
print_insn_mve (struct disassemble_info *info, long given)
{
  const struct mopcode32 *insn;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  enum disassembler_style base_style = dis_style_mnemonic;
  enum disassembler_style old_base_style = base_style;

  for (insn = mve_opcodes; insn->assembler; insn++)
    {
      if (((given & insn->mask) == insn->value)
	  && !is_mve_encoding_conflict (given, insn->mve_op))
	{
	  signed long value_in_comment = 0;
	  bool is_unpredictable = false;
	  bool is_undefined = false;
	  const char *c;
	  enum mve_unpredictable unpredictable_cond = UNPRED_NONE;
	  enum mve_undefined undefined_cond = UNDEF_NONE;

	  /* Most vector mve instruction are illegal in a it block.
	     There are a few exceptions; check for them.  */
	  if (ifthen_state && !is_mve_okay_in_it (insn->mve_op))
	    {
	      is_unpredictable = true;
	      unpredictable_cond = UNPRED_IT_BLOCK;
	    }
	  else if (is_mve_unpredictable (given, insn->mve_op,
					 &unpredictable_cond))
	    is_unpredictable = true;

	  if (is_mve_undefined (given, insn->mve_op, &undefined_cond))
	    is_undefined = true;

	  /* In "VORR Qd, Qm, Qn", if Qm==Qn, VORR is nothing but VMOV,
	     i.e "VMOV Qd, Qm".  */
	  if ((insn->mve_op == MVE_VORR_REG)
	      && (arm_decode_field (given, 1, 3)
		  == arm_decode_field (given, 17, 19)))
	    continue;

	  for (c = insn->assembler; *c; c++)
	    {
	      if (*c == '%')
		{
		  switch (*++c)
		    {
		    case '{':
		      ++c;
		      if (*c == '\0')
			abort ();
		      old_base_style = base_style;
		      base_style = decode_base_style (*c);
		      ++c;
		      if (*c != ':')
			abort ();
		      break;

		    case '}':
		      base_style = old_base_style;
		      break;

		    case '%':
		      func (stream, base_style, "%%");
		      break;

		    case 'a':
		      /* Don't print anything for '+' as it is implied.  */
		      if (arm_decode_field (given, 23, 23) == 0)
			func (stream, dis_style_immediate, "-");
		      break;

		    case 'c':
		      if (ifthen_state)
			func (stream, dis_style_mnemonic, "%s",
			      arm_conditional[IFTHEN_COND]);
		      break;

		    case 'd':
		      print_mve_vld_str_addr (info, given, insn->mve_op);
		      break;

		    case 'i':
		      {
			long mve_mask = mve_extract_pred_mask (given);
			func (stream, dis_style_mnemonic, "%s",
			      mve_predicatenames[mve_mask]);
		      }
		      break;

		    case 'j':
		      {
			unsigned int imm5 = 0;
			imm5 |= arm_decode_field (given, 6, 7);
			imm5 |= (arm_decode_field (given, 12, 14) << 2);
			func (stream, dis_style_immediate, "#%u",
			      (imm5 == 0) ? 32 : imm5);
		      }
		      break;

		    case 'k':
		      func (stream, dis_style_immediate, "#%u",
			    (arm_decode_field (given, 7, 7) == 0) ? 64 : 48);
		      break;

		    case 'n':
		      print_vec_condition (info, given, insn->mve_op);
		      break;

		    case 'o':
		      if (arm_decode_field (given, 0, 0) == 1)
			{
			  unsigned long size
			    = arm_decode_field (given, 4, 4)
			      | (arm_decode_field (given, 6, 6) << 1);

			  func (stream, dis_style_text, ", ");
			  func (stream, dis_style_sub_mnemonic, "uxtw ");
			  func (stream, dis_style_immediate, "#%lu", size);
			}
		      break;

		    case 'm':
		      print_mve_rounding_mode (info, given, insn->mve_op);
		      break;

		    case 's':
		      print_mve_vcvt_size (info, given, insn->mve_op);
		      break;

		    case 'u':
		      {
			unsigned long op1 = arm_decode_field (given, 21, 22);

			if ((insn->mve_op == MVE_VMOV_VEC_LANE_TO_GP))
			  {
			    /* Check for signed.  */
			    if (arm_decode_field (given, 23, 23) == 0)
			      {
				/* We don't print 's' for S32.  */
				if ((arm_decode_field (given, 5, 6) == 0)
				    && ((op1 == 0) || (op1 == 1)))
				  ;
				else
				  func (stream, dis_style_mnemonic, "s");
			      }
			    else
			      func (stream, dis_style_mnemonic, "u");
			  }
			else
			  {
			    if (arm_decode_field (given, 28, 28) == 0)
			      func (stream, dis_style_mnemonic, "s");
			    else
			      func (stream, dis_style_mnemonic, "u");
			  }
		      }
		      break;

		    case 'v':
		      print_instruction_predicate (info);
		      break;

		    case 'w':
		      if (arm_decode_field (given, 21, 21) == 1)
			func (stream, dis_style_text, "!");
		      break;

		    case 'B':
		      print_mve_register_blocks (info, given, insn->mve_op);
		      break;

		    case 'E':
		      /* SIMD encoded constant for mov, mvn, vorr, vbic.  */

		      print_simd_imm8 (info, given, 28, insn);
		      break;

		    case 'N':
		      print_mve_vmov_index (info, given);
		      break;

		    case 'T':
		      if (arm_decode_field (given, 12, 12) == 0)
			func (stream, dis_style_mnemonic, "b");
		      else
			func (stream, dis_style_mnemonic, "t");
		      break;

		    case 'X':
		      if (arm_decode_field (given, 12, 12) == 1)
			func (stream, dis_style_mnemonic, "x");
		      break;

		    case '0': case '1': case '2': case '3': case '4':
		    case '5': case '6': case '7': case '8': case '9':
		      {
			int width;
			unsigned long value;

			c = arm_decode_bitfield (c, given, &value, &width);

			switch (*c)
			  {
			  case 'Z':
			    if (value == 13)
			      is_unpredictable = true;
			    else if (value == 15)
			      func (stream, dis_style_register, "zr");
			    else
			      func (stream, dis_style_register, "%s",
				    arm_regnames[value]);
			    break;

			  case 'c':
			    func (stream, dis_style_sub_mnemonic, "%s",
				  arm_conditional[value]);
			    break;

			  case 'C':
			    value ^= 1;
			    func (stream, dis_style_sub_mnemonic, "%s",
				  arm_conditional[value]);
			    break;

			  case 'S':
			    if (value == 13 || value == 15)
			      is_unpredictable = true;
			    else
			      func (stream, dis_style_register, "%s",
				    arm_regnames[value]);
			    break;

			  case 's':
			    print_mve_size (info,
					    value,
					    insn->mve_op);
			    break;
			  case 'I':
			    if (value == 1)
			      func (stream, dis_style_mnemonic, "i");
			    break;
			  case 'A':
			    if (value == 1)
			      func (stream, dis_style_mnemonic, "a");
			    break;
			  case 'h':
			    {
			      unsigned int odd_reg = (value << 1) | 1;
			      func (stream, dis_style_register, "%s",
				    arm_regnames[odd_reg]);
			    }
			    break;
			  case 'i':
			    {
			      unsigned long imm
				= arm_decode_field (given, 0, 6);
			      unsigned long mod_imm = imm;

			      switch (insn->mve_op)
				{
				case MVE_VLDRW_GATHER_T5:
				case MVE_VSTRW_SCATTER_T5:
				  mod_imm = mod_imm << 2;
				  break;
				case MVE_VSTRD_SCATTER_T6:
				case MVE_VLDRD_GATHER_T6:
				  mod_imm = mod_imm << 3;
				  break;

				default:
				  break;
				}

			      func (stream, dis_style_immediate, "%lu",
				    mod_imm);
			    }
			    break;
			  case 'k':
			    func (stream, dis_style_immediate, "%lu",
				  64 - value);
			    break;
			  case 'l':
			    {
			      unsigned int even_reg = value << 1;
			      func (stream, dis_style_register, "%s",
				    arm_regnames[even_reg]);
			    }
			    break;
			  case 'u':
			    switch (value)
			      {
			      case 0:
				func (stream, dis_style_immediate, "1");
				break;
			      case 1:
				func (stream, dis_style_immediate, "2");
				break;
			      case 2:
				func (stream, dis_style_immediate, "4");
				break;
			      case 3:
				func (stream, dis_style_immediate, "8");
				break;
			      default:
				break;
			      }
			    break;
			  case 'o':
			    print_mve_rotate (info, value, width);
			    break;
			  case 'r':
			    func (stream, dis_style_register, "%s",
				  arm_regnames[value]);
			    break;
			  case 'd':
			    if (insn->mve_op == MVE_VQSHL_T2
				|| insn->mve_op == MVE_VQSHLU_T3
				|| insn->mve_op == MVE_VRSHR
				|| insn->mve_op == MVE_VRSHRN
				|| insn->mve_op == MVE_VSHL_T1
				|| insn->mve_op == MVE_VSHLL_T1
				|| insn->mve_op == MVE_VSHR
				|| insn->mve_op == MVE_VSHRN
				|| insn->mve_op == MVE_VSLI
				|| insn->mve_op == MVE_VSRI)
			      print_mve_shift_n (info, given, insn->mve_op);
			    else if (insn->mve_op == MVE_VSHLL_T2)
			      {
				switch (value)
				  {
				  case 0x00:
				    func (stream, dis_style_immediate, "8");
				    break;
				  case 0x01:
				    func (stream, dis_style_immediate, "16");
				    break;
				  case 0x10:
				    print_mve_undefined (info, UNDEF_SIZE_0);
				    break;
				  default:
				    assert (0);
				    break;
				  }
			      }
			    else
			      {
				if (insn->mve_op == MVE_VSHLC && value == 0)
				  value = 32;
				func (stream, base_style, "%ld", value);
				value_in_comment = value;
			      }
			    break;
			  case 'F':
			    func (stream, dis_style_register, "s%ld", value);
			    break;
			  case 'Q':
			    if (value & 0x8)
			      func (stream, dis_style_text,
				    "<illegal reg q%ld.5>", value);
			    else
			      func (stream, dis_style_register, "q%ld", value);
			    break;
			  case 'x':
			    func (stream, dis_style_immediate,
				  "0x%08lx", value);
			    break;
			  default:
			    abort ();
			  }
			break;
		      default:
			abort ();
		      }
		    }
		}
	      else
		{
		  if (*c == '@')
		    base_style = dis_style_comment_start;

		  if (*c == '\t')
		    base_style = dis_style_text;

		  func (stream, base_style, "%c", *c);
		}
	    }

	  if (value_in_comment > 32 || value_in_comment < -16)
	    func (stream, dis_style_comment_start, "\t@ 0x%lx",
		  value_in_comment);

	  if (is_unpredictable)
	    print_mve_unpredictable (info, unpredictable_cond);

	  if (is_undefined)
	    print_mve_undefined (info, undefined_cond);

	  if (!vpt_block_state.in_vpt_block
	      && !ifthen_state
	      && is_vpt_instruction (given))
	    mark_inside_vpt_block (given);
	  else if (vpt_block_state.in_vpt_block)
	    update_vpt_block_state ();

	  return true;
	}
    }
  return false;
}


/* Return the name of a v7A special register.  */

static const char *
banked_regname (unsigned reg)
{
  switch (reg)
    {
      case 15: return "CPSR";
      case 32: return "R8_usr";
      case 33: return "R9_usr";
      case 34: return "R10_usr";
      case 35: return "R11_usr";
      case 36: return "R12_usr";
      case 37: return "SP_usr";
      case 38: return "LR_usr";
      case 40: return "R8_fiq";
      case 41: return "R9_fiq";
      case 42: return "R10_fiq";
      case 43: return "R11_fiq";
      case 44: return "R12_fiq";
      case 45: return "SP_fiq";
      case 46: return "LR_fiq";
      case 48: return "LR_irq";
      case 49: return "SP_irq";
      case 50: return "LR_svc";
      case 51: return "SP_svc";
      case 52: return "LR_abt";
      case 53: return "SP_abt";
      case 54: return "LR_und";
      case 55: return "SP_und";
      case 60: return "LR_mon";
      case 61: return "SP_mon";
      case 62: return "ELR_hyp";
      case 63: return "SP_hyp";
      case 79: return "SPSR";
      case 110: return "SPSR_fiq";
      case 112: return "SPSR_irq";
      case 114: return "SPSR_svc";
      case 116: return "SPSR_abt";
      case 118: return "SPSR_und";
      case 124: return "SPSR_mon";
      case 126: return "SPSR_hyp";
      default: return NULL;
    }
}

/* Return the name of the DMB/DSB option.  */
static const char *
data_barrier_option (unsigned option)
{
  switch (option & 0xf)
    {
    case 0xf: return "sy";
    case 0xe: return "st";
    case 0xd: return "ld";
    case 0xb: return "ish";
    case 0xa: return "ishst";
    case 0x9: return "ishld";
    case 0x7: return "un";
    case 0x6: return "unst";
    case 0x5: return "nshld";
    case 0x3: return "osh";
    case 0x2: return "oshst";
    case 0x1: return "oshld";
    default:  return NULL;
    }
}

/* Print one ARM instruction from PC on INFO->STREAM.  */

static void
print_insn_arm (bfd_vma pc, struct disassemble_info *info, long given)
{
  const struct opcode32 *insn;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  struct arm_private_data *private_data = info->private_data;
  enum disassembler_style base_style = dis_style_mnemonic;
  enum disassembler_style old_base_style = base_style;

  if (print_insn_coprocessor (pc, info, given, false))
    return;

  if (print_insn_neon (info, given, false))
    return;

  if (print_insn_generic_coprocessor (pc, info, given, false))
    return;

  for (insn = arm_opcodes; insn->assembler; insn++)
    {
      if ((given & insn->mask) != insn->value)
	continue;

      if (! ARM_CPU_HAS_FEATURE (insn->arch, private_data->features))
	continue;

      /* Special case: an instruction with all bits set in the condition field
	 (0xFnnn_nnnn) is only matched if all those bits are set in insn->mask,
	 or by the catchall at the end of the table.  */
      if ((given & 0xF0000000) != 0xF0000000
	  || (insn->mask & 0xF0000000) == 0xF0000000
	  || (insn->mask == 0 && insn->value == 0))
	{
	  unsigned long u_reg = 16;
	  unsigned long U_reg = 16;
	  bool is_unpredictable = false;
	  signed long value_in_comment = 0;
	  const char *c;

	  for (c = insn->assembler; *c; c++)
	    {
	      if (*c == '%')
		{
		  bool allow_unpredictable = false;

		  switch (*++c)
		    {
		    case '{':
		      ++c;
		      if (*c == '\0')
			abort ();
		      old_base_style = base_style;
		      base_style = decode_base_style (*c);
		      ++c;
		      if (*c != ':')
			abort ();
		      break;

		    case '}':
		      base_style = old_base_style;
		      break;

		    case '%':
		      func (stream, base_style, "%%");
		      break;

		    case 'a':
		      value_in_comment = print_arm_address (pc, info, given);
		      break;

		    case 'P':
		      /* Set P address bit and use normal address
			 printing routine.  */
		      value_in_comment = print_arm_address (pc, info, given | (1 << P_BIT));
		      break;

		    case 'S':
		      allow_unpredictable = true;
		      /* Fall through.  */
		    case 's':
                      if ((given & 0x004f0000) == 0x004f0000)
			{
                          /* PC relative with immediate offset.  */
			  bfd_vma offset = ((given & 0xf00) >> 4) | (given & 0xf);

			  if (PRE_BIT_SET)
			    {
			      /* Elide positive zero offset.  */
			      if (offset || NEGATIVE_BIT_SET)
				{
				  func (stream, dis_style_text, "[");
				  func (stream, dis_style_register, "pc");
				  func (stream, dis_style_text, ", ");
				  func (stream, dis_style_immediate, "#%s%d",
					(NEGATIVE_BIT_SET ? "-" : ""),
					(int) offset);
				  func (stream, dis_style_text, "]");
				}
			      else
				{
				  func (stream, dis_style_text, "[");
				  func (stream, dis_style_register, "pc");
				  func (stream, dis_style_text, "]");
				}
			      if (NEGATIVE_BIT_SET)
				offset = -offset;
			      func (stream, dis_style_comment_start, "\t@ ");
			      info->print_address_func (offset + pc + 8, info);
			    }
			  else
			    {
			      /* Always show the offset.  */
			      func (stream, dis_style_text, "[");
			      func (stream, dis_style_register, "pc");
			      func (stream, dis_style_text, "], ");
			      func (stream, dis_style_immediate, "#%s%d",
				    NEGATIVE_BIT_SET ? "-" : "", (int) offset);
			      if (! allow_unpredictable)
				is_unpredictable = true;
			    }
			}
		      else
			{
			  int offset = ((given & 0xf00) >> 4) | (given & 0xf);

			  func (stream, dis_style_text, "[");
			  func (stream, dis_style_register, "%s",
				arm_regnames[(given >> 16) & 0xf]);

			  if (PRE_BIT_SET)
			    {
			      if (IMMEDIATE_BIT_SET)
				{
				  /* Elide offset for non-writeback
				     positive zero.  */
				  if (WRITEBACK_BIT_SET || NEGATIVE_BIT_SET
				      || offset)
				    {
				      func (stream, dis_style_text, ", ");
				      func (stream, dis_style_immediate,
					    "#%s%d",
					    (NEGATIVE_BIT_SET ? "-" : ""),
					    offset);
				    }

				  if (NEGATIVE_BIT_SET)
				    offset = -offset;

				  value_in_comment = offset;
				}
			      else
				{
				  /* Register Offset or Register Pre-Indexed.  */
				  func (stream, dis_style_text, ", %s",
					NEGATIVE_BIT_SET ? "-" : "");
				  func (stream, dis_style_register, "%s",
					arm_regnames[given & 0xf]);

				  /* Writing back to the register that is the source/
				     destination of the load/store is unpredictable.  */
				  if (! allow_unpredictable
				      && WRITEBACK_BIT_SET
				      && ((given & 0xf) == ((given >> 12) & 0xf)))
				    is_unpredictable = true;
				}

			      func (stream, dis_style_text, "]%s",
				    WRITEBACK_BIT_SET ? "!" : "");
			    }
			  else
			    {
			      if (IMMEDIATE_BIT_SET)
				{
				  /* Immediate Post-indexed.  */
				  /* PR 10924: Offset must be printed, even if it is zero.  */
				  func (stream, dis_style_text, "], ");
				  func (stream, dis_style_immediate, "#%s%d",
					NEGATIVE_BIT_SET ? "-" : "", offset);
				  if (NEGATIVE_BIT_SET)
				    offset = -offset;
				  value_in_comment = offset;
				}
			      else
				{
				  /* Register Post-indexed.  */
				  func (stream, dis_style_text, "], %s",
					NEGATIVE_BIT_SET ? "-" : "");
				  func (stream, dis_style_register, "%s",
					arm_regnames[given & 0xf]);

				  /* Writing back to the register that is the source/
				     destination of the load/store is unpredictable.  */
				  if (! allow_unpredictable
				      && (given & 0xf) == ((given >> 12) & 0xf))
				    is_unpredictable = true;
				}

			      if (! allow_unpredictable)
				{
				  /* Writeback is automatically implied by post- addressing.
				     Setting the W bit is unnecessary and ARM specify it as
				     being unpredictable.  */
				  if (WRITEBACK_BIT_SET
				      /* Specifying the PC register as the post-indexed
					 registers is also unpredictable.  */
				      || (! IMMEDIATE_BIT_SET && ((given & 0xf) == 0xf)))
				    is_unpredictable = true;
				}
			    }
			}
		      break;

		    case 'b':
		      {
			bfd_vma disp = (((given & 0xffffff) ^ 0x800000) - 0x800000);
			bfd_vma target = disp * 4 + pc + 8;
			info->print_address_func (target, info);

			/* Fill in instruction information.  */
			info->insn_info_valid = 1;
			info->insn_type = dis_branch;
			info->target = target;
		      }
		      break;

		    case 'c':
		      if (((given >> 28) & 0xf) != 0xe)
			func (stream, dis_style_mnemonic, "%s",
			      arm_conditional [(given >> 28) & 0xf]);
		      break;

		    case 'm':
		      {
			int started = 0;
			int reg;

			func (stream, dis_style_text, "{");
			for (reg = 0; reg < 16; reg++)
			  if ((given & (1 << reg)) != 0)
			    {
			      if (started)
				func (stream, dis_style_text, ", ");
			      started = 1;
			      func (stream, dis_style_register, "%s",
				    arm_regnames[reg]);
			    }
			func (stream, dis_style_text, "}");
			if (! started)
			  is_unpredictable = true;
		      }
		      break;

		    case 'q':
		      arm_decode_shift (given, func, stream, false);
		      break;

		    case 'o':
		      if ((given & 0x02000000) != 0)
			{
			  unsigned int rotate = (given & 0xf00) >> 7;
			  unsigned int immed = (given & 0xff);
			  unsigned int a, i;

			  a = (immed << ((32 - rotate) & 31)
			       | immed >> rotate) & 0xffffffff;
			  /* If there is another encoding with smaller rotate,
			     the rotate should be specified directly.  */
			  for (i = 0; i < 32; i += 2)
			    if ((a << i | a >> ((32 - i) & 31)) <= 0xff)
			      break;

			  if (i != rotate)
			    {
			      func (stream, dis_style_immediate, "#%d", immed);
			      func (stream, dis_style_text, ", ");
			      func (stream, dis_style_immediate, "%d", rotate);
			    }
			  else
			    func (stream, dis_style_immediate, "#%d", a);
			  value_in_comment = a;
			}
		      else
			arm_decode_shift (given, func, stream, true);
		      break;

		    case 'p':
		      if ((given & 0x0000f000) == 0x0000f000)
			{
			  arm_feature_set arm_ext_v6 =
			    ARM_FEATURE_CORE_LOW (ARM_EXT_V6);

			  /* The p-variants of tst/cmp/cmn/teq are the pre-V6
			     mechanism for setting PSR flag bits.  They are
			     obsolete in V6 onwards.  */
			  if (! ARM_CPU_HAS_FEATURE (private_data->features, \
						     arm_ext_v6))
			    func (stream, dis_style_mnemonic, "p");
			  else
			    is_unpredictable = true;
			}
		      break;

		    case 't':
		      if ((given & 0x01200000) == 0x00200000)
			func (stream, dis_style_mnemonic, "t");
		      break;

		    case 'A':
		      {
			int offset = given & 0xff;

			value_in_comment = offset * 4;
			if (NEGATIVE_BIT_SET)
			  value_in_comment = - value_in_comment;

			func (stream, dis_style_text, "[%s",
			      arm_regnames [(given >> 16) & 0xf]);

			if (PRE_BIT_SET)
			  {
			    if (offset)
			      func (stream, dis_style_text, ", #%d]%s",
				    (int) value_in_comment,
				    WRITEBACK_BIT_SET ? "!" : "");
			    else
			      func (stream, dis_style_text, "]");
			  }
			else
			  {
			    func (stream, dis_style_text, "]");

			    if (WRITEBACK_BIT_SET)
			      {
				if (offset)
				  func (stream, dis_style_text,
					", #%d", (int) value_in_comment);
			      }
			    else
			      {
				func (stream, dis_style_text,
				      ", {%d}", (int) offset);
				value_in_comment = offset;
			      }
			  }
		      }
		      break;

		    case 'B':
		      /* Print ARM V5 BLX(1) address: pc+25 bits.  */
		      {
			bfd_vma address;
			bfd_vma offset = 0;

			if (! NEGATIVE_BIT_SET)
			  /* Is signed, hi bits should be ones.  */
			  offset = (-1) ^ 0x00ffffff;

			/* Offset is (SignExtend(offset field)<<2).  */
			offset += given & 0x00ffffff;
			offset <<= 2;
			address = offset + pc + 8;

			if (given & 0x01000000)
			  /* H bit allows addressing to 2-byte boundaries.  */
			  address += 2;

		        info->print_address_func (address, info);

			/* Fill in instruction information.  */
			info->insn_info_valid = 1;
			info->insn_type = dis_branch;
			info->target = address;
		      }
		      break;

		    case 'C':
		      if ((given & 0x02000200) == 0x200)
			{
			  const char * name;
			  unsigned sysm = (given & 0x004f0000) >> 16;

			  sysm |= (given & 0x300) >> 4;
			  name = banked_regname (sysm);

			  if (name != NULL)
			    func (stream, dis_style_register, "%s", name);
			  else
			    func (stream, dis_style_text,
				  "(UNDEF: %lu)", (unsigned long) sysm);
			}
		      else
			{
			  func (stream, dis_style_register, "%cPSR_",
				(given & 0x00400000) ? 'S' : 'C');

			  if (given & 0x80000)
			    func (stream, dis_style_register, "f");
			  if (given & 0x40000)
			    func (stream, dis_style_register, "s");
			  if (given & 0x20000)
			    func (stream, dis_style_register, "x");
			  if (given & 0x10000)
			    func (stream, dis_style_register, "c");
			}
		      break;

		    case 'U':
		      if ((given & 0xf0) == 0x60)
			{
			  switch (given & 0xf)
			    {
			    case 0xf:
			      func (stream, dis_style_sub_mnemonic, "sy");
			      break;
			    default:
			      func (stream, dis_style_immediate, "#%d",
				    (int) given & 0xf);
			      break;
			    }
			}
		      else
			{
			  const char * opt = data_barrier_option (given & 0xf);
			  if (opt != NULL)
			    func (stream, dis_style_sub_mnemonic, "%s", opt);
			  else
			    func (stream, dis_style_immediate,
				  "#%d", (int) given & 0xf);
			}
		      break;

		    case '0': case '1': case '2': case '3': case '4':
		    case '5': case '6': case '7': case '8': case '9':
		      {
			int width;
			unsigned long value;

			c = arm_decode_bitfield (c, given, &value, &width);

			switch (*c)
			  {
			  case 'R':
			    if (value == 15)
			      is_unpredictable = true;
			    /* Fall through.  */
			  case 'r':
			  case 'T':
			    /* We want register + 1 when decoding T.  */
			    if (*c == 'T')
			      value = (value + 1) & 0xf;

			    if (c[1] == 'u')
			      {
				/* Eat the 'u' character.  */
				++ c;

				if (u_reg == value)
				  is_unpredictable = true;
				u_reg = value;
			      }
			    if (c[1] == 'U')
			      {
				/* Eat the 'U' character.  */
				++ c;

				if (U_reg == value)
				  is_unpredictable = true;
				U_reg = value;
			      }
			    func (stream, dis_style_register, "%s",
				  arm_regnames[value]);
			    break;
			  case 'd':
			    func (stream, base_style, "%ld", value);
			    value_in_comment = value;
			    break;
			  case 'b':
			    func (stream, dis_style_immediate,
				  "%ld", value * 8);
			    value_in_comment = value * 8;
			    break;
			  case 'W':
			    func (stream, dis_style_immediate,
				  "%ld", value + 1);
			    value_in_comment = value + 1;
			    break;
			  case 'x':
			    func (stream, dis_style_immediate,
				  "0x%08lx", value);

			    /* Some SWI instructions have special
			       meanings.  */
			    if ((given & 0x0fffffff) == 0x0FF00000)
			      func (stream, dis_style_comment_start,
				    "\t@ IMB");
			    else if ((given & 0x0fffffff) == 0x0FF00001)
			      func (stream, dis_style_comment_start,
				    "\t@ IMBRange");
			    break;
			  case 'X':
			    func (stream, dis_style_immediate,
				  "%01lx", value & 0xf);
			    value_in_comment = value;
			    break;
			  case '`':
			    c++;
			    if (value == 0)
			      func (stream, dis_style_text, "%c", *c);
			    break;
			  case '\'':
			    c++;
			    if (value == ((1ul << width) - 1))
			      func (stream, base_style, "%c", *c);
			    break;
			  case '?':
			    func (stream, base_style, "%c",
				  c[(1 << width) - (int) value]);
			    c += 1 << width;
			    break;
			  default:
			    abort ();
			  }
		      }
		      break;

		    case 'e':
		      {
			int imm;

			imm = (given & 0xf) | ((given & 0xfff00) >> 4);
			func (stream, dis_style_immediate, "%d", imm);
			value_in_comment = imm;
		      }
		      break;

		    case 'E':
		      /* LSB and WIDTH fields of BFI or BFC.  The machine-
			 language instruction encodes LSB and MSB.  */
		      {
			long msb = (given & 0x001f0000) >> 16;
			long lsb = (given & 0x00000f80) >> 7;
			long w = msb - lsb + 1;

			if (w > 0)
			  {
			    func (stream, dis_style_immediate, "#%lu", lsb);
			    func (stream, dis_style_text, ", ");
			    func (stream, dis_style_immediate, "#%lu", w);
			  }
			else
			  func (stream, dis_style_text,
				"(invalid: %lu:%lu)", lsb, msb);
		      }
		      break;

		    case 'R':
		      /* Get the PSR/banked register name.  */
		      {
			const char * name;
			unsigned sysm = (given & 0x004f0000) >> 16;

			sysm |= (given & 0x300) >> 4;
			name = banked_regname (sysm);

			if (name != NULL)
			  func (stream, dis_style_register, "%s", name);
			else
			  func (stream, dis_style_text,
				"(UNDEF: %lu)", (unsigned long) sysm);
		      }
		      break;

		    case 'V':
		      /* 16-bit unsigned immediate from a MOVT or MOVW
			 instruction, encoded in bits 0:11 and 15:19.  */
		      {
			long hi = (given & 0x000f0000) >> 4;
			long lo = (given & 0x00000fff);
			long imm16 = hi | lo;

			func (stream, dis_style_immediate, "#%lu", imm16);
			value_in_comment = imm16;
		      }
		      break;

		    default:
		      abort ();
		    }
		}
	      else
		{

		  if (*c == '@')
		    base_style = dis_style_comment_start;

		  if (*c == '\t')
		    base_style = dis_style_text;

		  func (stream, base_style, "%c", *c);
		}
	    }

	  if (value_in_comment > 32 || value_in_comment < -16)
	    func (stream, dis_style_comment_start, "\t@ 0x%lx",
		  (value_in_comment & 0xffffffffUL));

	  if (is_unpredictable)
	    func (stream, dis_style_comment_start, UNPREDICTABLE_INSTRUCTION);

	  return;
	}
    }
  func (stream, dis_style_comment_start, UNKNOWN_INSTRUCTION_32BIT,
	(unsigned) given);
  return;
}

/* Print one 16-bit Thumb instruction from PC on INFO->STREAM.  */

static void
print_insn_thumb16 (bfd_vma pc, struct disassemble_info *info, long given)
{
  const struct opcode16 *insn;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  enum disassembler_style base_style = dis_style_mnemonic;
  enum disassembler_style old_base_style = base_style;

  for (insn = thumb_opcodes; insn->assembler; insn++)
    if ((given & insn->mask) == insn->value)
      {
	signed long value_in_comment = 0;
	const char *c = insn->assembler;

	for (; *c; c++)
	  {
	    int domaskpc = 0;
	    int domasklr = 0;

	    if (*c != '%')
	      {
		if (*c == '@')
		  base_style = dis_style_comment_start;

		if (*c == '\t')
		  base_style = dis_style_text;

		func (stream, base_style, "%c", *c);

		continue;
	      }

	    switch (*++c)
	      {
		case '{':
		  ++c;
		  if (*c == '\0')
		    abort ();
		  old_base_style = base_style;
		  base_style = decode_base_style (*c);
		  ++c;
		  if (*c != ':')
		    abort ();
		  break;

		case '}':
		  base_style = old_base_style;
		  break;

	      case '%':
		func (stream, base_style, "%%");
		break;

	      case 'c':
		if (ifthen_state)
		  func (stream, dis_style_mnemonic, "%s",
			arm_conditional[IFTHEN_COND]);
		break;

	      case 'C':
		if (ifthen_state)
		  func (stream, dis_style_mnemonic, "%s",
			arm_conditional[IFTHEN_COND]);
		else
		  func (stream, dis_style_mnemonic, "s");
		break;

	      case 'I':
		{
		  unsigned int tmp;

		  ifthen_next_state = given & 0xff;
		  for (tmp = given << 1; tmp & 0xf; tmp <<= 1)
		    func (stream, dis_style_mnemonic,
			  ((given ^ tmp) & 0x10) ? "e" : "t");
		  func (stream, dis_style_text, "\t");
		  func (stream, dis_style_sub_mnemonic, "%s",
			arm_conditional[(given >> 4) & 0xf]);
		}
		break;

	      case 'x':
		if (ifthen_next_state)
		  func (stream, dis_style_comment_start,
			"\t@ unpredictable branch in IT block\n");
		break;

	      case 'X':
		if (ifthen_state)
		  func (stream, dis_style_comment_start,
			"\t@ unpredictable <IT:%s>",
			arm_conditional[IFTHEN_COND]);
		break;

	      case 'S':
		{
		  long reg;

		  reg = (given >> 3) & 0x7;
		  if (given & (1 << 6))
		    reg += 8;

		  func (stream, dis_style_register, "%s", arm_regnames[reg]);
		}
		break;

	      case 'D':
		{
		  long reg;

		  reg = given & 0x7;
		  if (given & (1 << 7))
		    reg += 8;

		  func (stream, dis_style_register, "%s", arm_regnames[reg]);
		}
		break;

	      case 'N':
		if (given & (1 << 8))
		  domasklr = 1;
		/* Fall through.  */
	      case 'O':
		if (*c == 'O' && (given & (1 << 8)))
		  domaskpc = 1;
		/* Fall through.  */
	      case 'M':
		{
		  int started = 0;
		  int reg;

		  func (stream, dis_style_text, "{");

		  /* It would be nice if we could spot
		     ranges, and generate the rS-rE format: */
		  for (reg = 0; (reg < 8); reg++)
		    if ((given & (1 << reg)) != 0)
		      {
			if (started)
			  func (stream, dis_style_text, ", ");
			started = 1;
			func (stream, dis_style_register, "%s",
			      arm_regnames[reg]);
		      }

		  if (domasklr)
		    {
		      if (started)
			func (stream, dis_style_text, ", ");
		      started = 1;
		      func (stream, dis_style_register, "%s",
			    arm_regnames[14] /* "lr" */);
		    }

		  if (domaskpc)
		    {
		      if (started)
			func (stream, dis_style_text, ", ");
		      func (stream, dis_style_register, "%s",
			    arm_regnames[15] /* "pc" */);
		    }

		  func (stream, dis_style_text, "}");
		}
		break;

	      case 'W':
		/* Print writeback indicator for a LDMIA.  We are doing a
		   writeback if the base register is not in the register
		   mask.  */
		if ((given & (1 << ((given & 0x0700) >> 8))) == 0)
		  func (stream, dis_style_text, "!");
		break;

	      case 'b':
		/* Print ARM V6T2 CZB address: pc+4+6 bits.  */
		{
		  bfd_vma address = (pc + 4
				     + ((given & 0x00f8) >> 2)
				     + ((given & 0x0200) >> 3));
		  info->print_address_func (address, info);

		  /* Fill in instruction information.  */
		  info->insn_info_valid = 1;
		  info->insn_type = dis_branch;
		  info->target = address;
		}
		break;

	      case 's':
		/* Right shift immediate -- bits 6..10; 1-31 print
		   as themselves, 0 prints as 32.  */
		{
		  long imm = (given & 0x07c0) >> 6;
		  if (imm == 0)
		    imm = 32;
		  func (stream, dis_style_immediate, "#%ld", imm);
		}
		break;

	      case '0': case '1': case '2': case '3': case '4':
	      case '5': case '6': case '7': case '8': case '9':
		{
		  int bitstart = *c++ - '0';
		  int bitend = 0;

		  while (*c >= '0' && *c <= '9')
		    bitstart = (bitstart * 10) + *c++ - '0';

		  switch (*c)
		    {
		    case '-':
		      {
			bfd_vma reg;

			c++;
			while (*c >= '0' && *c <= '9')
			  bitend = (bitend * 10) + *c++ - '0';
			if (!bitend)
			  abort ();
			reg = given >> bitstart;
			reg &= ((bfd_vma) 2 << (bitend - bitstart)) - 1;

			switch (*c)
			  {
			  case 'r':
			    func (stream, dis_style_register, "%s",
				  arm_regnames[reg]);
			    break;

			  case 'd':
			    func (stream, dis_style_immediate, "%ld",
				  (long) reg);
			    value_in_comment = reg;
			    break;

			  case 'H':
			    func (stream, dis_style_immediate, "%ld",
				  (long) (reg << 1));
			    value_in_comment = reg << 1;
			    break;

			  case 'W':
			    func (stream, dis_style_immediate, "%ld",
				  (long) (reg << 2));
			    value_in_comment = reg << 2;
			    break;

			  case 'a':
			    /* PC-relative address -- the bottom two
			       bits of the address are dropped
			       before the calculation.  */
			    info->print_address_func
			      (((pc + 4) & ~3) + (reg << 2), info);
			    value_in_comment = 0;
			    break;

			  case 'x':
			    func (stream, dis_style_immediate, "0x%04lx",
				  (long) reg);
			    break;

			  case 'B':
			    reg = ((reg ^ (1 << bitend)) - (1 << bitend));
			    bfd_vma target = reg * 2 + pc + 4;
			    info->print_address_func (target, info);
			    value_in_comment = 0;

			    /* Fill in instruction information.  */
			    info->insn_info_valid = 1;
			    info->insn_type = dis_branch;
			    info->target = target;
			    break;

			  case 'c':
			    func (stream, dis_style_mnemonic, "%s",
				  arm_conditional [reg]);
			    break;

			  default:
			    abort ();
			  }
		      }
		      break;

		    case '\'':
		      c++;
		      if ((given & (1 << bitstart)) != 0)
			func (stream, base_style, "%c", *c);
		      break;

		    case '?':
		      ++c;
		      if ((given & (1 << bitstart)) != 0)
			func (stream, base_style, "%c", *c++);
		      else
			func (stream, base_style, "%c", *++c);
		      break;

		    default:
		      abort ();
		    }
		}
		break;

	      default:
		abort ();
	      }
	  }

	if (value_in_comment > 32 || value_in_comment < -16)
	  func (stream, dis_style_comment_start,
		"\t@ 0x%lx", value_in_comment);
	return;
      }

  /* No match.  */
  func (stream, dis_style_comment_start, UNKNOWN_INSTRUCTION_16BIT,
	(unsigned) given);
  return;
}

/* Return the name of an V7M special register.  */

static const char *
psr_name (int regno)
{
  switch (regno)
    {
    case 0x0: return "APSR";
    case 0x1: return "IAPSR";
    case 0x2: return "EAPSR";
    case 0x3: return "PSR";
    case 0x5: return "IPSR";
    case 0x6: return "EPSR";
    case 0x7: return "IEPSR";
    case 0x8: return "MSP";
    case 0x9: return "PSP";
    case 0xa: return "MSPLIM";
    case 0xb: return "PSPLIM";
    case 0x10: return "PRIMASK";
    case 0x11: return "BASEPRI";
    case 0x12: return "BASEPRI_MAX";
    case 0x13: return "FAULTMASK";
    case 0x14: return "CONTROL";
    case 0x88: return "MSP_NS";
    case 0x89: return "PSP_NS";
    case 0x8a: return "MSPLIM_NS";
    case 0x8b: return "PSPLIM_NS";
    case 0x90: return "PRIMASK_NS";
    case 0x91: return "BASEPRI_NS";
    case 0x93: return "FAULTMASK_NS";
    case 0x94: return "CONTROL_NS";
    case 0x98: return "SP_NS";
    default: return "<unknown>";
    }
}

/* Print one 32-bit Thumb instruction from PC on INFO->STREAM.  */

static void
print_insn_thumb32 (bfd_vma pc, struct disassemble_info *info, long given)
{
  const struct opcode32 *insn;
  void *stream = info->stream;
  fprintf_styled_ftype func = info->fprintf_styled_func;
  bool is_mve = is_mve_architecture (info);
  enum disassembler_style base_style = dis_style_mnemonic;
  enum disassembler_style old_base_style = base_style;

  if (print_insn_coprocessor (pc, info, given, true))
    return;

  if (!is_mve && print_insn_neon (info, given, true))
    return;

  if (is_mve && print_insn_mve (info, given))
    return;

  if (print_insn_cde (info, given, true))
    return;

  if (print_insn_generic_coprocessor (pc, info, given, true))
    return;

  for (insn = thumb32_opcodes; insn->assembler; insn++)
    if ((given & insn->mask) == insn->value)
      {
	bool is_clrm = false;
	bool is_unpredictable = false;
	signed long value_in_comment = 0;
	const char *c = insn->assembler;

	for (; *c; c++)
	  {
	    if (*c != '%')
	      {
		if (*c == '@')
		  base_style = dis_style_comment_start;
		if (*c == '\t')
		  base_style = dis_style_text;
		func (stream, base_style, "%c", *c);
		continue;
	      }

	    switch (*++c)
	      {
	      case '{':
		++c;
		if (*c == '\0')
		  abort ();
		old_base_style = base_style;
		base_style = decode_base_style (*c);
		++c;
		if (*c != ':')
		  abort ();
		break;

	      case '}':
		base_style = old_base_style;
		break;

	      case '%':
		func (stream, base_style, "%%");
		break;

	      case 'c':
		if (ifthen_state)
		  func (stream, dis_style_mnemonic, "%s",
			arm_conditional[IFTHEN_COND]);
		break;

	      case 'x':
		if (ifthen_next_state)
		  func (stream, dis_style_comment_start,
			"\t@ unpredictable branch in IT block\n");
		break;

	      case 'X':
		if (ifthen_state)
		  func (stream, dis_style_comment_start,
			"\t@ unpredictable <IT:%s>",
			arm_conditional[IFTHEN_COND]);
		break;

	      case 'I':
		{
		  unsigned int imm12 = 0;

		  imm12 |= (given & 0x000000ffu);
		  imm12 |= (given & 0x00007000u) >> 4;
		  imm12 |= (given & 0x04000000u) >> 15;
		  func (stream, dis_style_immediate, "#%u", imm12);
		  value_in_comment = imm12;
		}
		break;

	      case 'M':
		{
		  unsigned int bits = 0, imm, imm8, mod;

		  bits |= (given & 0x000000ffu);
		  bits |= (given & 0x00007000u) >> 4;
		  bits |= (given & 0x04000000u) >> 15;
		  imm8 = (bits & 0x0ff);
		  mod = (bits & 0xf00) >> 8;
		  switch (mod)
		    {
		    case 0: imm = imm8; break;
		    case 1: imm = ((imm8 << 16) | imm8); break;
		    case 2: imm = ((imm8 << 24) | (imm8 << 8)); break;
		    case 3: imm = ((imm8 << 24) | (imm8 << 16) | (imm8 << 8) | imm8); break;
		    default:
		      mod  = (bits & 0xf80) >> 7;
		      imm8 = (bits & 0x07f) | 0x80;
		      imm  = (((imm8 << (32 - mod)) | (imm8 >> mod)) & 0xffffffff);
		    }
		  func (stream, dis_style_immediate, "#%u", imm);
		  value_in_comment = imm;
		}
		break;

	      case 'J':
		{
		  unsigned int imm = 0;

		  imm |= (given & 0x000000ffu);
		  imm |= (given & 0x00007000u) >> 4;
		  imm |= (given & 0x04000000u) >> 15;
		  imm |= (given & 0x000f0000u) >> 4;
		  func (stream, dis_style_immediate, "#%u", imm);
		  value_in_comment = imm;
		}
		break;

	      case 'K':
		{
		  unsigned int imm = 0;

		  imm |= (given & 0x000f0000u) >> 16;
		  imm |= (given & 0x00000ff0u) >> 0;
		  imm |= (given & 0x0000000fu) << 12;
		  func (stream, dis_style_immediate, "#%u", imm);
		  value_in_comment = imm;
		}
		break;

	      case 'H':
		{
		  unsigned int imm = 0;

		  imm |= (given & 0x000f0000u) >> 4;
		  imm |= (given & 0x00000fffu) >> 0;
		  func (stream, dis_style_immediate, "#%u", imm);
		  value_in_comment = imm;
		}
		break;

	      case 'V':
		{
		  unsigned int imm = 0;

		  imm |= (given & 0x00000fffu);
		  imm |= (given & 0x000f0000u) >> 4;
		  func (stream, dis_style_immediate, "#%u", imm);
		  value_in_comment = imm;
		}
		break;

	      case 'S':
		{
		  unsigned int reg = (given & 0x0000000fu);
		  unsigned int stp = (given & 0x00000030u) >> 4;
		  unsigned int imm = 0;
		  imm |= (given & 0x000000c0u) >> 6;
		  imm |= (given & 0x00007000u) >> 10;

		  func (stream, dis_style_register, "%s", arm_regnames[reg]);
		  switch (stp)
		    {
		    case 0:
		      if (imm > 0)
			{
			  func (stream, dis_style_text, ", ");
			  func (stream, dis_style_sub_mnemonic, "lsl ");
			  func (stream, dis_style_immediate, "#%u", imm);
			}
		      break;

		    case 1:
		      if (imm == 0)
			imm = 32;
		      func (stream, dis_style_text, ", ");
		      func (stream, dis_style_sub_mnemonic, "lsr ");
		      func (stream, dis_style_immediate, "#%u", imm);
		      break;

		    case 2:
		      if (imm == 0)
			imm = 32;
		      func (stream, dis_style_text, ", ");
		      func (stream, dis_style_sub_mnemonic, "asr ");
		      func (stream, dis_style_immediate, "#%u", imm);
		      break;

		    case 3:
		      if (imm == 0)
			{
			  func (stream, dis_style_text, ", ");
			  func (stream, dis_style_sub_mnemonic, "rrx");
			}
		      else
			{
			  func (stream, dis_style_text, ", ");
			  func (stream, dis_style_sub_mnemonic, "ror ");
			  func (stream, dis_style_immediate, "#%u", imm);
			}
		    }
		}
		break;

	      case 'a':
		{
		  unsigned int Rn  = (given & 0x000f0000) >> 16;
		  unsigned int U   = ! NEGATIVE_BIT_SET;
		  unsigned int op  = (given & 0x00000f00) >> 8;
		  unsigned int i12 = (given & 0x00000fff);
		  unsigned int i8  = (given & 0x000000ff);
		  bool writeback = false, postind = false;
		  bfd_vma offset = 0;

		  func (stream, dis_style_text, "[");
		  func (stream, dis_style_register, "%s", arm_regnames[Rn]);
		  if (U) /* 12-bit positive immediate offset.  */
		    {
		      offset = i12;
		      if (Rn != 15)
			value_in_comment = offset;
		    }
		  else if (Rn == 15) /* 12-bit negative immediate offset.  */
		    offset = - (int) i12;
		  else if (op == 0x0) /* Shifted register offset.  */
		    {
		      unsigned int Rm = (i8 & 0x0f);
		      unsigned int sh = (i8 & 0x30) >> 4;

		      func (stream, dis_style_text, ", ");
		      func (stream, dis_style_register, "%s",
			    arm_regnames[Rm]);
		      if (sh)
			{
			  func (stream, dis_style_text, ", ");
			  func (stream, dis_style_sub_mnemonic, "lsl ");
			  func (stream, dis_style_immediate, "#%u", sh);
			}
		      func (stream, dis_style_text, "]");
		      break;
		    }
		  else switch (op)
		    {
		    case 0xE:  /* 8-bit positive immediate offset.  */
		      offset = i8;
		      break;

		    case 0xC:  /* 8-bit negative immediate offset.  */
		      offset = -i8;
		      break;

		    case 0xF:  /* 8-bit + preindex with wb.  */
		      offset = i8;
		      writeback = true;
		      break;

		    case 0xD:  /* 8-bit - preindex with wb.  */
		      offset = -i8;
		      writeback = true;
		      break;

		    case 0xB:  /* 8-bit + postindex.  */
		      offset = i8;
		      postind = true;
		      break;

		    case 0x9:  /* 8-bit - postindex.  */
		      offset = -i8;
		      postind = true;
		      break;

		    default:
		      func (stream, dis_style_text, ", <undefined>]");
		      goto skip;
		    }

		  if (postind)
		    {
		      func (stream, dis_style_text, "], ");
		      func (stream, dis_style_immediate, "#%d", (int) offset);
		    }
		  else
		    {
		      if (offset)
			{
			  func (stream, dis_style_text, ", ");
			  func (stream, dis_style_immediate, "#%d",
				(int) offset);
			}
		      func (stream, dis_style_text, writeback ? "]!" : "]");
		    }

		  if (Rn == 15)
		    {
		      func (stream, dis_style_comment_start, "\t@ ");
		      info->print_address_func (((pc + 4) & ~3) + offset, info);
		    }
		}
	      skip:
		break;

	      case 'A':
		{
		  unsigned int U   = ! NEGATIVE_BIT_SET;
		  unsigned int W   = WRITEBACK_BIT_SET;
		  unsigned int Rn  = (given & 0x000f0000) >> 16;
		  unsigned int off = (given & 0x000000ff);

		  func (stream, dis_style_text, "[");
		  func (stream, dis_style_register, "%s", arm_regnames[Rn]);

		  if (PRE_BIT_SET)
		    {
		      if (off || !U)
			{
			  func (stream, dis_style_text, ", ");
			  func (stream, dis_style_immediate, "#%c%u",
				U ? '+' : '-', off * 4);
			  value_in_comment = off * 4 * (U ? 1 : -1);
			}
		      func (stream, dis_style_text, "]");
		      if (W)
			func (stream, dis_style_text, "!");
		    }
		  else
		    {
		      func (stream, dis_style_text, "], ");
		      if (W)
			{
			  func (stream, dis_style_immediate, "#%c%u",
				U ? '+' : '-', off * 4);
			  value_in_comment = off * 4 * (U ? 1 : -1);
			}
		      else
			{
			  func (stream, dis_style_text, "{");
			  func (stream, dis_style_immediate, "%u", off);
			  func (stream, dis_style_text, "}");
			  value_in_comment = off;
			}
		    }
		}
		break;

	      case 'w':
		{
		  unsigned int Sbit = (given & 0x01000000) >> 24;
		  unsigned int type = (given & 0x00600000) >> 21;

		  switch (type)
		    {
		    case 0:
		      func (stream, dis_style_mnemonic, Sbit ? "sb" : "b");
		      break;
		    case 1:
		      func (stream, dis_style_mnemonic, Sbit ? "sh" : "h");
		      break;
		    case 2:
		      if (Sbit)
			func (stream, dis_style_text, "??");
		      break;
		    case 3:
		      func (stream, dis_style_text, "??");
		      break;
		    }
		}
		break;

	      case 'n':
		is_clrm = true;
		/* Fall through.  */
	      case 'm':
		{
		  int started = 0;
		  int reg;

		  func (stream, dis_style_text, "{");
		  for (reg = 0; reg < 16; reg++)
		    if ((given & (1 << reg)) != 0)
		      {
			if (started)
			  func (stream, dis_style_text, ", ");
			started = 1;
			if (is_clrm && reg == 13)
			  func (stream, dis_style_text, "(invalid: %s)",
				arm_regnames[reg]);
			else if (is_clrm && reg == 15)
			  func (stream, dis_style_register, "%s", "APSR");
			else
			  func (stream, dis_style_register, "%s",
				arm_regnames[reg]);
		      }
		  func (stream, dis_style_text, "}");
		}
		break;

	      case 'E':
		{
		  unsigned int msb = (given & 0x0000001f);
		  unsigned int lsb = 0;

		  lsb |= (given & 0x000000c0u) >> 6;
		  lsb |= (given & 0x00007000u) >> 10;
		  func (stream, dis_style_immediate, "#%u", lsb);
		  func (stream, dis_style_text, ", ");
		  func (stream, dis_style_immediate, "#%u", msb - lsb + 1);
		}
		break;

	      case 'F':
		{
		  unsigned int width = (given & 0x0000001f) + 1;
		  unsigned int lsb = 0;

		  lsb |= (given & 0x000000c0u) >> 6;
		  lsb |= (given & 0x00007000u) >> 10;
		  func (stream, dis_style_immediate, "#%u", lsb);
		  func (stream, dis_style_text, ", ");
		  func (stream, dis_style_immediate, "#%u", width);
		}
		break;

	      case 'G':
		{
		  unsigned int boff = (((given & 0x07800000) >> 23) << 1);
		  func (stream, dis_style_immediate, "%x", boff);
		}
		break;

	      case 'W':
		{
		  unsigned int immA = (given & 0x001f0000u) >> 16;
		  unsigned int immB = (given & 0x000007feu) >> 1;
		  unsigned int immC = (given & 0x00000800u) >> 11;
		  bfd_vma offset = 0;

		  offset |= immA << 12;
		  offset |= immB << 2;
		  offset |= immC << 1;
		  /* Sign extend.  */
		  offset = (offset & 0x10000) ? offset - (1 << 17) : offset;

		  info->print_address_func (pc + 4 + offset, info);
		}
		break;

	      case 'Y':
		{
		  unsigned int immA = (given & 0x007f0000u) >> 16;
		  unsigned int immB = (given & 0x000007feu) >> 1;
		  unsigned int immC = (given & 0x00000800u) >> 11;
		  bfd_vma offset = 0;

		  offset |= immA << 12;
		  offset |= immB << 2;
		  offset |= immC << 1;
		  /* Sign extend.  */
		  offset = (offset & 0x40000) ? offset - (1 << 19) : offset;

		  info->print_address_func (pc + 4 + offset, info);
		}
		break;

	      case 'Z':
		{
		  unsigned int immA = (given & 0x00010000u) >> 16;
		  unsigned int immB = (given & 0x000007feu) >> 1;
		  unsigned int immC = (given & 0x00000800u) >> 11;
		  bfd_vma offset = 0;

		  offset |= immA << 12;
		  offset |= immB << 2;
		  offset |= immC << 1;
		  /* Sign extend.  */
		  offset = (offset & 0x1000) ? offset - (1 << 13) : offset;

		  info->print_address_func (pc + 4 + offset, info);

		  unsigned int T    = (given & 0x00020000u) >> 17;
		  unsigned int endoffset = (((given & 0x07800000) >> 23) << 1);
		  unsigned int boffset   = (T == 1) ? 4 : 2;
		  func (stream, dis_style_text, ", ");
		  func (stream, dis_style_immediate, "%x",
			endoffset + boffset);
		}
		break;

	      case 'Q':
		{
		  unsigned int immh = (given & 0x000007feu) >> 1;
		  unsigned int imml = (given & 0x00000800u) >> 11;
		  bfd_vma imm32 = 0;

		  imm32 |= immh << 2;
		  imm32 |= imml << 1;

		  info->print_address_func (pc + 4 + imm32, info);
		}
		break;

	      case 'P':
		{
		  unsigned int immh = (given & 0x000007feu) >> 1;
		  unsigned int imml = (given & 0x00000800u) >> 11;
		  bfd_vma imm32 = 0;

		  imm32 |= immh << 2;
		  imm32 |= imml << 1;

		  info->print_address_func (pc + 4 - imm32, info);
		}
		break;

	      case 'b':
		{
		  unsigned int S = (given & 0x04000000u) >> 26;
		  unsigned int J1 = (given & 0x00002000u) >> 13;
		  unsigned int J2 = (given & 0x00000800u) >> 11;
		  bfd_vma offset = 0;

		  offset |= !S << 20;
		  offset |= J2 << 19;
		  offset |= J1 << 18;
		  offset |= (given & 0x003f0000) >> 4;
		  offset |= (given & 0x000007ff) << 1;
		  offset -= (1 << 20);

		  bfd_vma target = pc + 4 + offset;
		  info->print_address_func (target, info);

		  /* Fill in instruction information.  */
		  info->insn_info_valid = 1;
		  info->insn_type = dis_branch;
		  info->target = target;
		}
		break;

	      case 'B':
		{
		  unsigned int S = (given & 0x04000000u) >> 26;
		  unsigned int I1 = (given & 0x00002000u) >> 13;
		  unsigned int I2 = (given & 0x00000800u) >> 11;
		  bfd_vma offset = 0;

		  offset |= !S << 24;
		  offset |= !(I1 ^ S) << 23;
		  offset |= !(I2 ^ S) << 22;
		  offset |= (given & 0x03ff0000u) >> 4;
		  offset |= (given & 0x000007ffu) << 1;
		  offset -= (1 << 24);
		  offset += pc + 4;

		  /* BLX target addresses are always word aligned.  */
		  if ((given & 0x00001000u) == 0)
		      offset &= ~2u;

		  info->print_address_func (offset, info);

		  /* Fill in instruction information.  */
		  info->insn_info_valid = 1;
		  info->insn_type = dis_branch;
		  info->target = offset;
		}
		break;

	      case 's':
		{
		  unsigned int shift = 0;

		  shift |= (given & 0x000000c0u) >> 6;
		  shift |= (given & 0x00007000u) >> 10;
		  if (WRITEBACK_BIT_SET)
		    {
		      func (stream, dis_style_text, ", ");
		      func (stream, dis_style_sub_mnemonic, "asr ");
		      func (stream, dis_style_immediate, "#%u", shift);
		    }
		  else if (shift)
		    {
		      func (stream, dis_style_text, ", ");
		      func (stream, dis_style_sub_mnemonic, "lsl ");
		      func (stream, dis_style_immediate, "#%u", shift);
		    }
		  /* else print nothing - lsl #0 */
		}
		break;

	      case 'R':
		{
		  unsigned int rot = (given & 0x00000030) >> 4;

		  if (rot)
		    {
		      func (stream, dis_style_text, ", ");
		      func (stream, dis_style_sub_mnemonic, "ror ");
		      func (stream, dis_style_immediate, "#%u", rot * 8);
		    }
		}
		break;

	      case 'U':
		if ((given & 0xf0) == 0x60)
		  {
		    switch (given & 0xf)
		      {
		      case 0xf:
			func (stream, dis_style_sub_mnemonic, "sy");
			break;
		      default:
			func (stream, dis_style_immediate, "#%d",
			      (int) given & 0xf);
			break;
		      }
		  }
		else
		  {
		    const char * opt = data_barrier_option (given & 0xf);
		    if (opt != NULL)
		      func (stream, dis_style_sub_mnemonic, "%s", opt);
		    else
		      func (stream, dis_style_immediate, "#%d",
			    (int) given & 0xf);
		   }
		break;

	      case 'C':
		if ((given & 0xff) == 0)
		  {
		    func (stream, dis_style_register, "%cPSR_",
			  (given & 0x100000) ? 'S' : 'C');

		    if (given & 0x800)
		      func (stream, dis_style_register, "f");
		    if (given & 0x400)
		      func (stream, dis_style_register, "s");
		    if (given & 0x200)
		      func (stream, dis_style_register, "x");
		    if (given & 0x100)
		      func (stream, dis_style_register, "c");
		  }
		else if ((given & 0x20) == 0x20)
		  {
		    char const* name;
		    unsigned sysm = (given & 0xf00) >> 8;

		    sysm |= (given & 0x30);
		    sysm |= (given & 0x00100000) >> 14;
		    name = banked_regname (sysm);

		    if (name != NULL)
		      func (stream, dis_style_register, "%s", name);
		    else
		      func (stream, dis_style_text,
			    "(UNDEF: %lu)", (unsigned long) sysm);
		  }
		else
		  {
		    func (stream, dis_style_register, "%s",
			  psr_name (given & 0xff));
		  }
		break;

	      case 'D':
		if (((given & 0xff) == 0)
		    || ((given & 0x20) == 0x20))
		  {
		    char const* name;
		    unsigned sm = (given & 0xf0000) >> 16;

		    sm |= (given & 0x30);
		    sm |= (given & 0x00100000) >> 14;
		    name = banked_regname (sm);

		    if (name != NULL)
		      func (stream, dis_style_register, "%s", name);
		    else
		      func (stream, dis_style_text,
			    "(UNDEF: %lu)", (unsigned long) sm);
		  }
		else
		  func (stream, dis_style_register, "%s",
			psr_name (given & 0xff));
		break;

	      case '0': case '1': case '2': case '3': case '4':
	      case '5': case '6': case '7': case '8': case '9':
		{
		  int width;
		  unsigned long val;

		  c = arm_decode_bitfield (c, given, &val, &width);

		  switch (*c)
		    {
		    case 's':
		      if (val <= 3)
			func (stream, dis_style_mnemonic, "%s",
			      mve_vec_sizename[val]);
		      else
			func (stream, dis_style_text, "<undef size>");
		      break;

		    case 'd':
		      func (stream, base_style, "%lu", val);
		      value_in_comment = val;
		      break;

		    case 'D':
		      func (stream, dis_style_immediate, "%lu", val + 1);
		      value_in_comment = val + 1;
		      break;

		    case 'W':
		      func (stream, dis_style_immediate, "%lu", val * 4);
		      value_in_comment = val * 4;
		      break;

		    case 'S':
		      if (val == 13)
			is_unpredictable = true;
		      /* Fall through.  */
		    case 'R':
		      if (val == 15)
			is_unpredictable = true;
		      /* Fall through.  */
		    case 'r':
		      func (stream, dis_style_register, "%s",
			    arm_regnames[val]);
		      break;

		    case 'c':
		      func (stream, base_style, "%s", arm_conditional[val]);
		      break;

		    case '\'':
		      c++;
		      if (val == ((1ul << width) - 1))
			func (stream, base_style, "%c", *c);
		      break;

		    case '`':
		      c++;
		      if (val == 0)
			func (stream, dis_style_immediate, "%c", *c);
		      break;

		    case '?':
		      func (stream, dis_style_mnemonic, "%c",
			    c[(1 << width) - (int) val]);
		      c += 1 << width;
		      break;

		    case 'x':
		      func (stream, dis_style_immediate, "0x%lx",
			    val & 0xffffffffUL);
		      break;

		    default:
		      abort ();
		    }
		}
		break;

	      case 'L':
		/* PR binutils/12534
		   If we have a PC relative offset in an LDRD or STRD
		   instructions then display the decoded address.  */
		if (((given >> 16) & 0xf) == 0xf)
		  {
		    bfd_vma offset = (given & 0xff) * 4;

		    if ((given & (1 << 23)) == 0)
		      offset = - offset;
		    func (stream, dis_style_comment_start, "\t@ ");
		    info->print_address_func ((pc & ~3) + 4 + offset, info);
		  }
		break;

	      default:
		abort ();
	      }
	  }

	if (value_in_comment > 32 || value_in_comment < -16)
	  func (stream, dis_style_comment_start, "\t@ 0x%lx",
		value_in_comment);

	if (is_unpredictable)
	  func (stream, dis_style_comment_start, UNPREDICTABLE_INSTRUCTION);

	return;
      }

  /* No match.  */
  func (stream, dis_style_comment_start, UNKNOWN_INSTRUCTION_32BIT,
	(unsigned) given);
  return;
}

/* Print data bytes on INFO->STREAM.  */

static void
print_insn_data (bfd_vma pc ATTRIBUTE_UNUSED,
		 struct disassemble_info *info,
		 long given)
{
  fprintf_styled_ftype func = info->fprintf_styled_func;

  switch (info->bytes_per_chunk)
    {
    case 1:
      func (info->stream, dis_style_assembler_directive, ".byte");
      func (info->stream, dis_style_text, "\t");
      func (info->stream, dis_style_immediate, "0x%02lx", given);
      break;
    case 2:
      func (info->stream, dis_style_assembler_directive, ".short");
      func (info->stream, dis_style_text, "\t");
      func (info->stream, dis_style_immediate, "0x%04lx", given);
      break;
    case 4:
      func (info->stream, dis_style_assembler_directive, ".word");
      func (info->stream, dis_style_text, "\t");
      func (info->stream, dis_style_immediate, "0x%08lx", given);
      break;
    default:
      abort ();
    }
}

/* Disallow mapping symbols ($a, $b, $d, $t etc) from
   being displayed in symbol relative addresses.

   Also disallow private symbol, with __tagsym$$ prefix,
   from ARM RVCT toolchain being displayed.  */

bool
arm_symbol_is_valid (asymbol * sym,
		     struct disassemble_info * info ATTRIBUTE_UNUSED)
{
  const char * name;

  if (sym == NULL)
    return false;

  name = bfd_asymbol_name (sym);

  return (name && *name != '$' && strncmp (name, "__tagsym$$", 10));
}

/* Parse the string of disassembler options.  */

static void
parse_arm_disassembler_options (const char *options)
{
  const char *opt;

  force_thumb = false;
  FOR_EACH_DISASSEMBLER_OPTION (opt, options)
    {
      if (startswith (opt, "reg-names-"))
	{
	  unsigned int i;
	  for (i = 0; i < NUM_ARM_OPTIONS; i++)
	    if (disassembler_options_cmp (opt, regnames[i].name) == 0)
	      {
		regname_selected = i;
		break;
	      }

	  if (i >= NUM_ARM_OPTIONS)
	    /* xgettext: c-format */
	    opcodes_error_handler (_("unrecognised register name set: %s"),
				   opt);
	}
      else if (startswith (opt, "force-thumb"))
	force_thumb = 1;
      else if (startswith (opt, "no-force-thumb"))
	force_thumb = 0;
      else if (startswith (opt, "coproc"))
	{
	  const char *procptr = opt + sizeof ("coproc") - 1;
	  char *endptr;
	  uint8_t coproc_number = strtol (procptr, &endptr, 10);
	  if (endptr != procptr + 1 || coproc_number > 7)
	    {
	      opcodes_error_handler (_("cde coprocessor not between 0-7: %s"),
				     opt);
	      continue;
	    }
	  if (*endptr != '=')
	    {
	      opcodes_error_handler (_("coproc must have an argument: %s"),
				     opt);
	      continue;
	    }
	  endptr += 1;
	  if (startswith (endptr, "generic"))
	    cde_coprocs &= ~(1 << coproc_number);
	  else if (startswith (endptr, "cde")
		   || startswith (endptr, "CDE"))
	    cde_coprocs |= (1 << coproc_number);
	  else
	    {
	      opcodes_error_handler (
		  _("coprocN argument takes options \"generic\","
		    " \"cde\", or \"CDE\": %s"), opt);
	    }
	}
      else
	/* xgettext: c-format */
	opcodes_error_handler (_("unrecognised disassembler option: %s"), opt);
    }

  return;
}

static bool
mapping_symbol_for_insn (bfd_vma pc, struct disassemble_info *info,
			 enum map_type *map_symbol);

/* Search back through the insn stream to determine if this instruction is
   conditionally executed.  */

static void
find_ifthen_state (bfd_vma pc,
		   struct disassemble_info *info,
		   bool little)
{
  unsigned char b[2];
  unsigned int insn;
  int status;
  /* COUNT is twice the number of instructions seen.  It will be odd if we
     just crossed an instruction boundary.  */
  int count;
  int it_count;
  unsigned int seen_it;
  bfd_vma addr;

  ifthen_address = pc;
  ifthen_state = 0;

  addr = pc;
  count = 1;
  it_count = 0;
  seen_it = 0;
  /* Scan backwards looking for IT instructions, keeping track of where
     instruction boundaries are.  We don't know if something is actually an
     IT instruction until we find a definite instruction boundary.  */
  for (;;)
    {
      if (addr == 0 || info->symbol_at_address_func (addr, info))
	{
	  /* A symbol must be on an instruction boundary, and will not
	     be within an IT block.  */
	  if (seen_it && (count & 1))
	    break;

	  return;
	}
      addr -= 2;
      status = info->read_memory_func (addr, (bfd_byte *) b, 2, info);
      if (status)
	return;

      if (little)
	insn = (b[0]) | (b[1] << 8);
      else
	insn = (b[1]) | (b[0] << 8);
      if (seen_it)
	{
	  if ((insn & 0xf800) < 0xe800)
	    {
	      /* Addr + 2 is an instruction boundary.  See if this matches
	         the expected boundary based on the position of the last
		 IT candidate.  */
	      if (count & 1)
		break;
	      seen_it = 0;
	    }
	}
      if ((insn & 0xff00) == 0xbf00 && (insn & 0xf) != 0)
	{
	  enum map_type type = MAP_ARM;
	  bool found = mapping_symbol_for_insn (addr, info, &type);

	  if (!found || (found && type == MAP_THUMB))
	    {
	      /* This could be an IT instruction.  */
	      seen_it = insn;
	      it_count = count >> 1;
	    }
	}
      if ((insn & 0xf800) >= 0xe800)
	count++;
      else
	count = (count + 2) | 1;
      /* IT blocks contain at most 4 instructions.  */
      if (count >= 8 && !seen_it)
	return;
    }
  /* We found an IT instruction.  */
  ifthen_state = (seen_it & 0xe0) | ((seen_it << it_count) & 0x1f);
  if ((ifthen_state & 0xf) == 0)
    ifthen_state = 0;
}

/* Returns nonzero and sets *MAP_TYPE if the N'th symbol is a
   mapping symbol.  */

static int
is_mapping_symbol (struct disassemble_info *info,
		   int n,
		   enum map_type *map_type)
{
  const char *name = bfd_asymbol_name (info->symtab[n]);

  if (name[0] == '$'
      && (name[1] == 'a' || name[1] == 't' || name[1] == 'd')
      && (name[2] == 0 || name[2] == '.'))
    {
      *map_type = ((name[1] == 'a') ? MAP_ARM
		   : (name[1] == 't') ? MAP_THUMB
		   : MAP_DATA);
      return true;
    }

  return false;
}

/* Try to infer the code type (ARM or Thumb) from a mapping symbol.
   Returns nonzero if *MAP_TYPE was set.  */

static int
get_map_sym_type (struct disassemble_info *info,
		  int n,
		  enum map_type *map_type)
{
  /* If the symbol is in a different section, ignore it.  */
  if (info->section != NULL && info->section != info->symtab[n]->section)
    return false;

  return is_mapping_symbol (info, n, map_type);
}

/* Try to infer the code type (ARM or Thumb) from a non-mapping symbol.
   Returns nonzero if *MAP_TYPE was set.  */

static int
get_sym_code_type (struct disassemble_info *info,
		   int n,
		   enum map_type *map_type)
{
  elf_symbol_type *es;
  unsigned int type;
  asymbol * sym;

  /* If the symbol is in a different section, ignore it.  */
  if (info->section != NULL && info->section != info->symtab[n]->section)
    return false;

  /* PR 30230: Reject non-ELF symbols, eg synthetic ones.  */
  sym = info->symtab[n];
  if (bfd_asymbol_flavour (sym) != bfd_target_elf_flavour)
    return false;

  es = (elf_symbol_type *) sym;
  type = ELF_ST_TYPE (es->internal_elf_sym.st_info);

  /* If the symbol has function type then use that.  */
  if (type == STT_FUNC || type == STT_GNU_IFUNC)
    {
      if (ARM_GET_SYM_BRANCH_TYPE (es->internal_elf_sym.st_target_internal)
	  == ST_BRANCH_TO_THUMB)
	*map_type = MAP_THUMB;
      else
	*map_type = MAP_ARM;
      return true;
    }

  return false;
}

/* Search the mapping symbol state for instruction at pc.  This is only
   applicable for elf target.

   There is an assumption Here, info->private_data contains the correct AND
   up-to-date information about current scan process.  The information will be
   used to speed this search process.

   Return TRUE if the mapping state can be determined, and map_symbol
   will be updated accordingly.  Otherwise, return FALSE.  */

static bool
mapping_symbol_for_insn (bfd_vma pc, struct disassemble_info *info,
			 enum map_type *map_symbol)
{
  bfd_vma addr, section_vma = 0;
  int n, last_sym = -1;
  bool found = false;
  bool can_use_search_opt_p = false;

  /* Sanity check.  */
  if (info == NULL)
    return false;

  /* Default to DATA.  A text section is required by the ABI to contain an
     INSN mapping symbol at the start.  A data section has no such
     requirement, hence if no mapping symbol is found the section must
     contain only data.  This however isn't very useful if the user has
     fully stripped the binaries.  If this is the case use the section
     attributes to determine the default.  If we have no section default to
     INSN as well, as we may be disassembling some raw bytes on a baremetal
     HEX file or similar.  */
  enum map_type type = MAP_DATA;
  if ((info->section && info->section->flags & SEC_CODE) || !info->section)
    type = MAP_ARM;
  struct arm_private_data *private_data;

  if (info->private_data == NULL || info->symtab == NULL
      || info->symtab_size == 0
      || bfd_asymbol_flavour (*info->symtab) != bfd_target_elf_flavour)
    return false;

  private_data = info->private_data;

  /* First, look for mapping symbols.  */
  if (pc <= private_data->last_mapping_addr)
    private_data->last_mapping_sym = -1;

  /* Start scanning at the start of the function, or wherever
     we finished last time.  */
  n = info->symtab_pos + 1;

  /* If the last stop offset is different from the current one it means we
     are disassembling a different glob of bytes.  As such the optimization
     would not be safe and we should start over.  */
  can_use_search_opt_p
    = (private_data->last_mapping_sym >= 0
       && info->stop_offset == private_data->last_stop_offset);

  if (n >= private_data->last_mapping_sym && can_use_search_opt_p)
    n = private_data->last_mapping_sym;

  /* Look down while we haven't passed the location being disassembled.
     The reason for this is that there's no defined order between a symbol
     and an mapping symbol that may be at the same address.  We may have to
     look at least one position ahead.  */
  for (; n < info->symtab_size; n++)
    {
      addr = bfd_asymbol_value (info->symtab[n]);
      if (addr > pc)
	break;
      if (get_map_sym_type (info, n, &type))
	{
	  last_sym = n;
	  found = true;
	}
    }

  if (!found)
    {
      n = info->symtab_pos;
      if (n >= private_data->last_mapping_sym && can_use_search_opt_p)
	n = private_data->last_mapping_sym;

      /* No mapping symbol found at this address.  Look backwards
	 for a preceeding one, but don't go pass the section start
	 otherwise a data section with no mapping symbol can pick up
	 a text mapping symbol of a preceeding section.  The documentation
	 says section can be NULL, in which case we will seek up all the
	 way to the top.  */
      if (info->section)
	section_vma = info->section->vma;

      for (; n >= 0; n--)
	{
	  addr = bfd_asymbol_value (info->symtab[n]);
	  if (addr < section_vma)
	    break;

	  if (get_map_sym_type (info, n, &type))
	    {
	      last_sym = n;
	      found = true;
	      break;
	    }
	}
    }

  /* If no mapping symbol was found, try looking up without a mapping
     symbol.  This is done by walking up from the current PC to the nearest
     symbol.  We don't actually have to loop here since symtab_pos will
     contain the nearest symbol already.  */
  if (!found)
    {
      n = info->symtab_pos;
      if (n >= 0 && get_sym_code_type (info, n, &type))
	{
	  last_sym = n;
	  found = true;
	}
    }

  private_data->last_mapping_sym = last_sym;
  private_data->last_type = type;
  private_data->last_stop_offset = info->stop_offset;

  *map_symbol = type;
  return found;
}

/* Given a bfd_mach_arm_XXX value, this function fills in the fields
   of the supplied arm_feature_set structure with bitmasks indicating
   the supported base architectures and coprocessor extensions.

   FIXME: This could more efficiently implemented as a constant array,
   although it would also be less robust.  */

static void
select_arm_features (unsigned long mach,
		     arm_feature_set * features)
{
  arm_feature_set arch_fset;
  const arm_feature_set fpu_any = FPU_ANY;

#undef ARM_SET_FEATURES
#define ARM_SET_FEATURES(FSET) \
  {							\
    const arm_feature_set fset = FSET;			\
    arch_fset = fset;					\
  }

  /* When several architecture versions share the same bfd_mach_arm_XXX value
     the most featureful is chosen.  */
  switch (mach)
    {
    case bfd_mach_arm_2:	 ARM_SET_FEATURES (ARM_ARCH_V2); break;
    case bfd_mach_arm_2a:	 ARM_SET_FEATURES (ARM_ARCH_V2S); break;
    case bfd_mach_arm_3:	 ARM_SET_FEATURES (ARM_ARCH_V3); break;
    case bfd_mach_arm_3M:	 ARM_SET_FEATURES (ARM_ARCH_V3M); break;
    case bfd_mach_arm_4:	 ARM_SET_FEATURES (ARM_ARCH_V4); break;
    case bfd_mach_arm_4T:	 ARM_SET_FEATURES (ARM_ARCH_V4T); break;
    case bfd_mach_arm_5:	 ARM_SET_FEATURES (ARM_ARCH_V5); break;
    case bfd_mach_arm_5T:	 ARM_SET_FEATURES (ARM_ARCH_V5T); break;
    case bfd_mach_arm_5TE:	 ARM_SET_FEATURES (ARM_ARCH_V5TE); break;
    case bfd_mach_arm_XScale:	 ARM_SET_FEATURES (ARM_ARCH_XSCALE); break;
    case bfd_mach_arm_ep9312:
	ARM_SET_FEATURES (ARM_FEATURE_LOW (ARM_AEXT_V4T,
					   ARM_CEXT_MAVERICK | FPU_MAVERICK));
       break;
    case bfd_mach_arm_iWMMXt:	 ARM_SET_FEATURES (ARM_ARCH_IWMMXT); break;
    case bfd_mach_arm_iWMMXt2:	 ARM_SET_FEATURES (ARM_ARCH_IWMMXT2); break;
    case bfd_mach_arm_5TEJ:	 ARM_SET_FEATURES (ARM_ARCH_V5TEJ); break;
    case bfd_mach_arm_6:	 ARM_SET_FEATURES (ARM_ARCH_V6); break;
    case bfd_mach_arm_6KZ:	 ARM_SET_FEATURES (ARM_ARCH_V6KZ); break;
    case bfd_mach_arm_6T2:	 ARM_SET_FEATURES (ARM_ARCH_V6KZT2); break;
    case bfd_mach_arm_6K:	 ARM_SET_FEATURES (ARM_ARCH_V6K); break;
    case bfd_mach_arm_7:	 ARM_SET_FEATURES (ARM_ARCH_V7VE); break;
    case bfd_mach_arm_6M:	 ARM_SET_FEATURES (ARM_ARCH_V6M); break;
    case bfd_mach_arm_6SM:	 ARM_SET_FEATURES (ARM_ARCH_V6SM); break;
    case bfd_mach_arm_7EM:	 ARM_SET_FEATURES (ARM_ARCH_V7EM); break;
    case bfd_mach_arm_8:
	{
	  /* Add bits for extensions that Armv8.6-A recognizes.  */
	  arm_feature_set armv8_6_ext_fset
	    = ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST);
	  ARM_SET_FEATURES (ARM_ARCH_V8_6A);
	  ARM_MERGE_FEATURE_SETS (arch_fset, arch_fset, armv8_6_ext_fset);
	  break;
	}
    case bfd_mach_arm_8R:	 ARM_SET_FEATURES (ARM_ARCH_V8R); break;
    case bfd_mach_arm_8M_BASE:	 ARM_SET_FEATURES (ARM_ARCH_V8M_BASE); break;
    case bfd_mach_arm_8M_MAIN:	 ARM_SET_FEATURES (ARM_ARCH_V8M_MAIN); break;
    case bfd_mach_arm_8_1M_MAIN:
      ARM_SET_FEATURES (ARM_ARCH_V8_1M_MAIN);
      arm_feature_set mve_all
	= ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE | ARM_EXT2_MVE_FP);
      ARM_MERGE_FEATURE_SETS (arch_fset, arch_fset, mve_all);
      force_thumb = 1;
      break;
    case bfd_mach_arm_9:         ARM_SET_FEATURES (ARM_ARCH_V9A); break;
      /* If the machine type is unknown allow all architecture types and all
	 extensions, with the exception of MVE as that clashes with NEON.  */
    case bfd_mach_arm_unknown:
      ARM_SET_FEATURES (ARM_ARCH_UNKNOWN);
      break;
    default:
      abort ();
    }
#undef ARM_SET_FEATURES

  /* None of the feature bits related to -mfpu have an impact on Tag_CPU_arch
     and thus on bfd_mach_arm_XXX value.  Therefore for a given
     bfd_mach_arm_XXX value all coprocessor feature bits should be allowed.  */
  ARM_MERGE_FEATURE_SETS (*features, arch_fset, fpu_any);
}


/* NOTE: There are no checks in these routines that
   the relevant number of data bytes exist.  */

static int
print_insn (bfd_vma pc, struct disassemble_info *info, bool little)
{
  unsigned char b[4];
  unsigned long given;
  int status;
  int is_thumb = false;
  int is_data = false;
  int little_code;
  unsigned int	size = 4;
  void (*printer) (bfd_vma, struct disassemble_info *, long);
  bool found = false;
  struct arm_private_data *private_data;

  /* Clear instruction information field.  */
  info->insn_info_valid = 0;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->insn_type = dis_noninsn;
  info->target = 0;
  info->target2 = 0;

  if (info->disassembler_options)
    {
      parse_arm_disassembler_options (info->disassembler_options);

      /* To avoid repeated parsing of these options, we remove them here.  */
      info->disassembler_options = NULL;
    }

  /* PR 10288: Control which instructions will be disassembled.  */
  if (info->private_data == NULL)
    {
      static struct arm_private_data private;

      if ((info->flags & USER_SPECIFIED_MACHINE_TYPE) == 0)
	/* If the user did not use the -m command line switch then default to
	   disassembling all types of ARM instruction.

	   The info->mach value has to be ignored as this will be based on
	   the default archictecture for the target and/or hints in the notes
	   section, but it will never be greater than the current largest arm
	   machine value (iWMMXt2), which is only equivalent to the V5TE
	   architecture.  ARM architectures have advanced beyond the machine
	   value encoding, and these newer architectures would be ignored if
	   the machine value was used.

	   Ie the -m switch is used to restrict which instructions will be
	   disassembled.  If it is necessary to use the -m switch to tell
	   objdump that an ARM binary is being disassembled, eg because the
	   input is a raw binary file, but it is also desired to disassemble
	   all ARM instructions then use "-marm".  This will select the
	   "unknown" arm architecture which is compatible with any ARM
	   instruction.  */
	  info->mach = bfd_mach_arm_unknown;

      /* Compute the architecture bitmask from the machine number.
	 Note: This assumes that the machine number will not change
	 during disassembly....  */
      select_arm_features (info->mach, & private.features);

      private.last_mapping_sym = -1;
      private.last_mapping_addr = 0;
      private.last_stop_offset = 0;

      info->private_data = & private;
    }

  private_data = info->private_data;

  /* Decide if our code is going to be little-endian, despite what the
     function argument might say.  */
  little_code = ((info->endian_code == BFD_ENDIAN_LITTLE) || little);

  /* For ELF, consult the symbol table to determine what kind of code
     or data we have.  */
  if (info->symtab_size != 0
      && bfd_asymbol_flavour (*info->symtab) == bfd_target_elf_flavour)
    {
      bfd_vma addr;
      int n;
      int last_sym = -1;
      enum map_type type = MAP_ARM;

      found = mapping_symbol_for_insn (pc, info, &type);
      last_sym = private_data->last_mapping_sym;

      is_thumb = (private_data->last_type == MAP_THUMB);
      is_data = (private_data->last_type == MAP_DATA);

      /* Look a little bit ahead to see if we should print out
	 two or four bytes of data.  If there's a symbol,
	 mapping or otherwise, after two bytes then don't
	 print more.  */
      if (is_data)
	{
	  size = 4 - (pc & 3);
	  for (n = last_sym + 1; n < info->symtab_size; n++)
	    {
	      addr = bfd_asymbol_value (info->symtab[n]);
	      if (addr > pc
		  && (info->section == NULL
		      || info->section == info->symtab[n]->section))
		{
		  if (addr - pc < size)
		    size = addr - pc;
		  break;
		}
	    }
	  /* If the next symbol is after three bytes, we need to
	     print only part of the data, so that we can use either
	     .byte or .short.  */
	  if (size == 3)
	    size = (pc & 1) ? 1 : 2;
	}
    }

  if (info->symbols != NULL)
    {
      if (bfd_asymbol_flavour (*info->symbols) == bfd_target_coff_flavour)
	{
	  coff_symbol_type * cs;

	  cs = coffsymbol (*info->symbols);
	  is_thumb = (   cs->native->u.syment.n_sclass == C_THUMBEXT
		      || cs->native->u.syment.n_sclass == C_THUMBSTAT
		      || cs->native->u.syment.n_sclass == C_THUMBLABEL
		      || cs->native->u.syment.n_sclass == C_THUMBEXTFUNC
		      || cs->native->u.syment.n_sclass == C_THUMBSTATFUNC);
	}
      else if (bfd_asymbol_flavour (*info->symbols) == bfd_target_elf_flavour
	       && !found)
	{
	  /* If no mapping symbol has been found then fall back to the type
	     of the function symbol.  */
	  elf_symbol_type *  es;
	  unsigned int       type;

	  es = *(elf_symbol_type **)(info->symbols);
	  type = ELF_ST_TYPE (es->internal_elf_sym.st_info);

	  is_thumb =
	    ((ARM_GET_SYM_BRANCH_TYPE (es->internal_elf_sym.st_target_internal)
	      == ST_BRANCH_TO_THUMB) || type == STT_ARM_16BIT);
	}
      else if (bfd_asymbol_flavour (*info->symbols)
	       == bfd_target_mach_o_flavour)
	{
	  bfd_mach_o_asymbol *asym = (bfd_mach_o_asymbol *)*info->symbols;

	  is_thumb = (asym->n_desc & BFD_MACH_O_N_ARM_THUMB_DEF);
	}
    }

  if (force_thumb)
    is_thumb = true;

  if (is_data)
    info->display_endian = little ? BFD_ENDIAN_LITTLE : BFD_ENDIAN_BIG;
  else
    info->display_endian = little_code ? BFD_ENDIAN_LITTLE : BFD_ENDIAN_BIG;

  info->bytes_per_line = 4;

  /* PR 10263: Disassemble data if requested to do so by the user.  */
  if (is_data && ((info->flags & DISASSEMBLE_DATA) == 0))
    {
      int i;

      /* Size was already set above.  */
      info->bytes_per_chunk = size;
      printer = print_insn_data;

      status = info->read_memory_func (pc, (bfd_byte *) b, size, info);
      given = 0;
      if (little)
	for (i = size - 1; i >= 0; i--)
	  given = b[i] | (given << 8);
      else
	for (i = 0; i < (int) size; i++)
	  given = b[i] | (given << 8);
    }
  else if (!is_thumb)
    {
      /* In ARM mode endianness is a straightforward issue: the instruction
	 is four bytes long and is either ordered 0123 or 3210.  */
      printer = print_insn_arm;
      info->bytes_per_chunk = 4;
      size = 4;

      status = info->read_memory_func (pc, (bfd_byte *) b, 4, info);
      if (little_code)
	given = (b[0]) | (b[1] << 8) | (b[2] << 16) | ((unsigned) b[3] << 24);
      else
	given = (b[3]) | (b[2] << 8) | (b[1] << 16) | ((unsigned) b[0] << 24);
    }
  else
    {
      /* In Thumb mode we have the additional wrinkle of two
	 instruction lengths.  Fortunately, the bits that determine
	 the length of the current instruction are always to be found
	 in the first two bytes.  */
      printer = print_insn_thumb16;
      info->bytes_per_chunk = 2;
      size = 2;

      status = info->read_memory_func (pc, (bfd_byte *) b, 2, info);
      if (little_code)
	given = (b[0]) | (b[1] << 8);
      else
	given = (b[1]) | (b[0] << 8);

      if (!status)
	{
	  /* These bit patterns signal a four-byte Thumb
	     instruction.  */
	  if ((given & 0xF800) == 0xF800
	      || (given & 0xF800) == 0xF000
	      || (given & 0xF800) == 0xE800)
	    {
	      status = info->read_memory_func (pc + 2, (bfd_byte *) b, 2, info);
	      if (little_code)
		given = (b[0]) | (b[1] << 8) | (given << 16);
	      else
		given = (b[1]) | (b[0] << 8) | (given << 16);

	      printer = print_insn_thumb32;
	      size = 4;
	    }
	}

      if (ifthen_address != pc)
	find_ifthen_state (pc, info, little_code);

      if (ifthen_state)
	{
	  if ((ifthen_state & 0xf) == 0x8)
	    ifthen_next_state = 0;
	  else
	    ifthen_next_state = (ifthen_state & 0xe0)
				| ((ifthen_state & 0xf) << 1);
	}
    }

  if (status)
    {
      info->memory_error_func (status, pc, info);
      return -1;
    }
  if (info->flags & INSN_HAS_RELOC)
    /* If the instruction has a reloc associated with it, then
       the offset field in the instruction will actually be the
       addend for the reloc.  (We are using REL type relocs).
       In such cases, we can ignore the pc when computing
       addresses, since the addend is not currently pc-relative.  */
    pc = 0;

  printer (pc, info, given);

  if (is_thumb)
    {
      ifthen_state = ifthen_next_state;
      ifthen_address += size;
    }
  return size;
}

int
print_insn_big_arm (bfd_vma pc, struct disassemble_info *info)
{
  /* Detect BE8-ness and record it in the disassembler info.  */
  if (info->flavour == bfd_target_elf_flavour
      && info->section != NULL
      && (elf_elfheader (info->section->owner)->e_flags & EF_ARM_BE8))
    info->endian_code = BFD_ENDIAN_LITTLE;

  return print_insn (pc, info, false);
}

int
print_insn_little_arm (bfd_vma pc, struct disassemble_info *info)
{
  return print_insn (pc, info, true);
}

const disasm_options_and_args_t *
disassembler_options_arm (void)
{
  static disasm_options_and_args_t *opts_and_args;

  if (opts_and_args == NULL)
    {
      disasm_options_t *opts;
      unsigned int i;

      opts_and_args = XNEW (disasm_options_and_args_t);
      opts_and_args->args = NULL;

      opts = &opts_and_args->options;
      opts->name = XNEWVEC (const char *, NUM_ARM_OPTIONS + 1);
      opts->description = XNEWVEC (const char *, NUM_ARM_OPTIONS + 1);
      opts->arg = NULL;
      for (i = 0; i < NUM_ARM_OPTIONS; i++)
	{
	  opts->name[i] = regnames[i].name;
	  if (regnames[i].description != NULL)
	    opts->description[i] = _(regnames[i].description);
	  else
	    opts->description[i] = NULL;
	}
      /* The array we return must be NULL terminated.  */
      opts->name[i] = NULL;
      opts->description[i] = NULL;
    }

  return opts_and_args;
}

void
print_arm_disassembler_options (FILE *stream)
{
  unsigned int i, max_len = 0;
  fprintf (stream, _("\n\
The following ARM specific disassembler options are supported for use with\n\
the -M switch:\n"));

  for (i = 0; i < NUM_ARM_OPTIONS; i++)
    {
      unsigned int len = strlen (regnames[i].name);
      if (max_len < len)
	max_len = len;
    }

  for (i = 0, max_len++; i < NUM_ARM_OPTIONS; i++)
    fprintf (stream, "  %s%*c %s\n",
	     regnames[i].name,
	     (int)(max_len - strlen (regnames[i].name)), ' ',
	     _(regnames[i].description));
}
