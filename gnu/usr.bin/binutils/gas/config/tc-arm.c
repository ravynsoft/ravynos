/* tc-arm.c -- Assemble for the ARM
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Contributed by Richard Earnshaw (rwe@pegasus.esprit.ec.org)
	Modified by David Taylor (dtaylor@armltd.co.uk)
	Cirrus coprocessor mods by Aldy Hernandez (aldyh@redhat.com)
	Cirrus coprocessor fixes by Petko Manolov (petkan@nucleusys.com)
	Cirrus coprocessor fixes by Vladimir Ivanov (vladitx@nucleusys.com)

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include <limits.h>
#include <stdarg.h>
#define	 NO_RELOC 0
#include "safe-ctype.h"
#include "subsegs.h"
#include "obstack.h"
#include "libiberty.h"
#include "opcode/arm.h"
#include "cpu-arm.h"

#ifdef OBJ_ELF
#include "elf/arm.h"
#include "dw2gencfi.h"
#endif

#include "dwarf2dbg.h"

#ifdef OBJ_ELF
/* Must be at least the size of the largest unwind opcode (currently two).  */
#define ARM_OPCODE_CHUNK_SIZE 8

/* This structure holds the unwinding state.  */

static struct
{
  symbolS *	  proc_start;
  symbolS *	  table_entry;
  symbolS *	  personality_routine;
  int		  personality_index;
  /* The segment containing the function.  */
  segT		  saved_seg;
  subsegT	  saved_subseg;
  /* Opcodes generated from this function.  */
  unsigned char * opcodes;
  int		  opcode_count;
  int		  opcode_alloc;
  /* The number of bytes pushed to the stack.  */
  offsetT	  frame_size;
  /* We don't add stack adjustment opcodes immediately so that we can merge
     multiple adjustments.  We can also omit the final adjustment
     when using a frame pointer.  */
  offsetT	  pending_offset;
  /* These two fields are set by both unwind_movsp and unwind_setfp.  They
     hold the reg+offset to use when restoring sp from a frame pointer.	 */
  offsetT	  fp_offset;
  int		  fp_reg;
  /* Nonzero if an unwind_setfp directive has been seen.  */
  unsigned	  fp_used:1;
  /* Nonzero if the last opcode restores sp from fp_reg.  */
  unsigned	  sp_restored:1;
} unwind;

/* Whether --fdpic was given.  */
static int arm_fdpic;

#endif /* OBJ_ELF */

/* Results from operand parsing worker functions.  */

typedef enum
{
  PARSE_OPERAND_SUCCESS,
  PARSE_OPERAND_FAIL,
  PARSE_OPERAND_FAIL_NO_BACKTRACK
} parse_operand_result;

enum arm_float_abi
{
  ARM_FLOAT_ABI_HARD,
  ARM_FLOAT_ABI_SOFTFP,
  ARM_FLOAT_ABI_SOFT
};

/* Types of processor to assemble for.	*/
#ifndef CPU_DEFAULT
/* The code that was here used to select a default CPU depending on compiler
   pre-defines which were only present when doing native builds, thus
   changing gas' default behaviour depending upon the build host.

   If you have a target that requires a default CPU option then the you
   should define CPU_DEFAULT here.  */
#endif

/* Perform range checks on positive and negative overflows by checking if the
   VALUE given fits within the range of an BITS sized immediate.  */
static bool out_of_range_p (offsetT value, offsetT bits)
 {
  gas_assert (bits < (offsetT)(sizeof (value) * 8));
  return (value & ~((1 << bits)-1))
	  && ((value & ~((1 << bits)-1)) != ~((1 << bits)-1));
}

#ifndef FPU_DEFAULT
# ifdef TE_LINUX
#  define FPU_DEFAULT FPU_ARCH_FPA
# elif defined (TE_NetBSD)
#  ifdef OBJ_ELF
#   define FPU_DEFAULT FPU_ARCH_VFP	/* Soft-float, but VFP order.  */
#  else
    /* Legacy a.out format.  */
#   define FPU_DEFAULT FPU_ARCH_FPA	/* Soft-float, but FPA order.  */
#  endif
# elif defined (TE_VXWORKS)
#  define FPU_DEFAULT FPU_ARCH_VFP	/* Soft-float, VFP order.  */
# else
   /* For backwards compatibility, default to FPA.  */
#  define FPU_DEFAULT FPU_ARCH_FPA
# endif
#endif /* ifndef FPU_DEFAULT */

#define streq(a, b)	      (strcmp (a, b) == 0)

/* Current set of feature bits available (CPU+FPU).  Different from
   selected_cpu + selected_fpu in case of autodetection since the CPU
   feature bits are then all set.  */
static arm_feature_set cpu_variant;
/* Feature bits used in each execution state.  Used to set build attribute
   (in particular Tag_*_ISA_use) in CPU autodetection mode.  */
static arm_feature_set arm_arch_used;
static arm_feature_set thumb_arch_used;

/* Flags stored in private area of BFD structure.  */
static int uses_apcs_26	     = false;
static int atpcs	     = false;
static int support_interwork = false;
static int uses_apcs_float   = false;
static int pic_code	     = false;
static int fix_v4bx	     = false;
/* Warn on using deprecated features.  */
static int warn_on_deprecated = true;
static int warn_on_restrict_it = false;

/* Understand CodeComposer Studio assembly syntax.  */
bool codecomposer_syntax = false;

/* Variables that we set while parsing command-line options.  Once all
   options have been read we re-process these values to set the real
   assembly flags.  */

/* CPU and FPU feature bits set for legacy CPU and FPU options (eg. -marm1
   instead of -mcpu=arm1).  */
static const arm_feature_set *legacy_cpu = NULL;
static const arm_feature_set *legacy_fpu = NULL;

/* CPU, extension and FPU feature bits selected by -mcpu.  */
static const arm_feature_set *mcpu_cpu_opt = NULL;
static arm_feature_set *mcpu_ext_opt = NULL;
static const arm_feature_set *mcpu_fpu_opt = NULL;

/* CPU, extension and FPU feature bits selected by -march.  */
static const arm_feature_set *march_cpu_opt = NULL;
static arm_feature_set *march_ext_opt = NULL;
static const arm_feature_set *march_fpu_opt = NULL;

/* Feature bits selected by -mfpu.  */
static const arm_feature_set *mfpu_opt = NULL;

/* Constants for known architecture features.  */
static const arm_feature_set fpu_default = FPU_DEFAULT;
static const arm_feature_set fpu_arch_vfp_v1 ATTRIBUTE_UNUSED = FPU_ARCH_VFP_V1;
static const arm_feature_set fpu_arch_vfp_v2 = FPU_ARCH_VFP_V2;
static const arm_feature_set fpu_arch_vfp_v3 ATTRIBUTE_UNUSED = FPU_ARCH_VFP_V3;
static const arm_feature_set fpu_arch_neon_v1 ATTRIBUTE_UNUSED = FPU_ARCH_NEON_V1;
static const arm_feature_set fpu_arch_fpa = FPU_ARCH_FPA;
static const arm_feature_set fpu_any_hard = FPU_ANY_HARD;
#ifdef OBJ_ELF
static const arm_feature_set fpu_arch_maverick = FPU_ARCH_MAVERICK;
#endif
static const arm_feature_set fpu_endian_pure = FPU_ARCH_ENDIAN_PURE;

#ifdef CPU_DEFAULT
static const arm_feature_set cpu_default = CPU_DEFAULT;
#endif

static const arm_feature_set arm_ext_v1 = ARM_FEATURE_CORE_LOW (ARM_EXT_V1);
static const arm_feature_set arm_ext_v2 = ARM_FEATURE_CORE_LOW (ARM_EXT_V2);
static const arm_feature_set arm_ext_v2s = ARM_FEATURE_CORE_LOW (ARM_EXT_V2S);
static const arm_feature_set arm_ext_v3 = ARM_FEATURE_CORE_LOW (ARM_EXT_V3);
static const arm_feature_set arm_ext_v3m = ARM_FEATURE_CORE_LOW (ARM_EXT_V3M);
static const arm_feature_set arm_ext_v4 = ARM_FEATURE_CORE_LOW (ARM_EXT_V4);
static const arm_feature_set arm_ext_v4t = ARM_FEATURE_CORE_LOW (ARM_EXT_V4T);
static const arm_feature_set arm_ext_v5 = ARM_FEATURE_CORE_LOW (ARM_EXT_V5);
static const arm_feature_set arm_ext_v4t_5 =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V4T | ARM_EXT_V5);
static const arm_feature_set arm_ext_v5t = ARM_FEATURE_CORE_LOW (ARM_EXT_V5T);
static const arm_feature_set arm_ext_v5e = ARM_FEATURE_CORE_LOW (ARM_EXT_V5E);
static const arm_feature_set arm_ext_v5exp = ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP);
static const arm_feature_set arm_ext_v5j = ARM_FEATURE_CORE_LOW (ARM_EXT_V5J);
static const arm_feature_set arm_ext_v6 = ARM_FEATURE_CORE_LOW (ARM_EXT_V6);
static const arm_feature_set arm_ext_v6k = ARM_FEATURE_CORE_LOW (ARM_EXT_V6K);
static const arm_feature_set arm_ext_v6t2 = ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2);
/* Only for compatability of hint instructions.  */
static const arm_feature_set arm_ext_v6k_v6t2 =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V6K | ARM_EXT_V6T2);
static const arm_feature_set arm_ext_v6_notm =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V6_NOTM);
static const arm_feature_set arm_ext_v6_dsp =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V6_DSP);
static const arm_feature_set arm_ext_barrier =
  ARM_FEATURE_CORE_LOW (ARM_EXT_BARRIER);
static const arm_feature_set arm_ext_msr =
  ARM_FEATURE_CORE_LOW (ARM_EXT_THUMB_MSR);
static const arm_feature_set arm_ext_div = ARM_FEATURE_CORE_LOW (ARM_EXT_DIV);
static const arm_feature_set arm_ext_v7 = ARM_FEATURE_CORE_LOW (ARM_EXT_V7);
static const arm_feature_set arm_ext_v7a = ARM_FEATURE_CORE_LOW (ARM_EXT_V7A);
static const arm_feature_set arm_ext_v7r = ARM_FEATURE_CORE_LOW (ARM_EXT_V7R);
static const arm_feature_set arm_ext_v8r = ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8R);
#ifdef OBJ_ELF
static const arm_feature_set ATTRIBUTE_UNUSED arm_ext_v7m = ARM_FEATURE_CORE_LOW (ARM_EXT_V7M);
#endif
static const arm_feature_set arm_ext_v8 = ARM_FEATURE_CORE_LOW (ARM_EXT_V8);
static const arm_feature_set arm_ext_m =
  ARM_FEATURE_CORE (ARM_EXT_V6M | ARM_EXT_V7M,
		    ARM_EXT2_V8M | ARM_EXT2_V8M_MAIN);
static const arm_feature_set arm_ext_mp = ARM_FEATURE_CORE_LOW (ARM_EXT_MP);
static const arm_feature_set arm_ext_sec = ARM_FEATURE_CORE_LOW (ARM_EXT_SEC);
static const arm_feature_set arm_ext_os = ARM_FEATURE_CORE_LOW (ARM_EXT_OS);
static const arm_feature_set arm_ext_adiv = ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV);
static const arm_feature_set arm_ext_virt = ARM_FEATURE_CORE_LOW (ARM_EXT_VIRT);
static const arm_feature_set arm_ext_pan = ARM_FEATURE_CORE_HIGH (ARM_EXT2_PAN);
static const arm_feature_set arm_ext_v8m = ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M);
static const arm_feature_set arm_ext_v8m_main =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M_MAIN);
static const arm_feature_set arm_ext_v8_1m_main =
ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN);
/* Instructions in ARMv8-M only found in M profile architectures.  */
static const arm_feature_set arm_ext_v8m_m_only =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M | ARM_EXT2_V8M_MAIN);
static const arm_feature_set arm_ext_v6t2_v8m =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M);
/* Instructions shared between ARMv8-A and ARMv8-M.  */
static const arm_feature_set arm_ext_atomics =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_ATOMICS);
#ifdef OBJ_ELF
/* DSP instructions Tag_DSP_extension refers to.  */
static const arm_feature_set arm_ext_dsp =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V5E | ARM_EXT_V5ExP | ARM_EXT_V6_DSP);
#endif
static const arm_feature_set arm_ext_ras =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_RAS);
/* FP16 instructions.  */
static const arm_feature_set arm_ext_fp16 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST);
static const arm_feature_set arm_ext_fp16_fml =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_FML);
static const arm_feature_set arm_ext_v8_2 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_2A);
static const arm_feature_set arm_ext_v8_3 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A);
static const arm_feature_set arm_ext_sb =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB);
static const arm_feature_set arm_ext_predres =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_PREDRES);
static const arm_feature_set arm_ext_bf16 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16);
static const arm_feature_set arm_ext_i8mm =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM);
static const arm_feature_set arm_ext_crc =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC);
static const arm_feature_set arm_ext_cde =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE);
static const arm_feature_set arm_ext_cde0 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE0);
static const arm_feature_set arm_ext_cde1 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE1);
static const arm_feature_set arm_ext_cde2 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE2);
static const arm_feature_set arm_ext_cde3 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE3);
static const arm_feature_set arm_ext_cde4 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE4);
static const arm_feature_set arm_ext_cde5 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE5);
static const arm_feature_set arm_ext_cde6 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE6);
static const arm_feature_set arm_ext_cde7 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE7);

static const arm_feature_set arm_arch_any = ARM_ANY;
static const arm_feature_set fpu_any = FPU_ANY;
static const arm_feature_set arm_arch_full ATTRIBUTE_UNUSED = ARM_FEATURE (-1, -1, -1);
static const arm_feature_set arm_arch_t2 = ARM_ARCH_THUMB2;
static const arm_feature_set arm_arch_none = ARM_ARCH_NONE;

static const arm_feature_set arm_cext_iwmmxt2 =
  ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT2);
static const arm_feature_set arm_cext_iwmmxt =
  ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT);
static const arm_feature_set arm_cext_xscale =
  ARM_FEATURE_COPROC (ARM_CEXT_XSCALE);
static const arm_feature_set arm_cext_maverick =
  ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK);
static const arm_feature_set fpu_fpa_ext_v1 =
  ARM_FEATURE_COPROC (FPU_FPA_EXT_V1);
static const arm_feature_set fpu_fpa_ext_v2 =
  ARM_FEATURE_COPROC (FPU_FPA_EXT_V2);
static const arm_feature_set fpu_vfp_ext_v1xd =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD);
static const arm_feature_set fpu_vfp_ext_v1 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V1);
static const arm_feature_set fpu_vfp_ext_v2 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V2);
static const arm_feature_set fpu_vfp_ext_v3xd =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V3xD);
static const arm_feature_set fpu_vfp_ext_v3 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V3);
static const arm_feature_set fpu_vfp_ext_d32 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_D32);
static const arm_feature_set fpu_neon_ext_v1 =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_V1);
static const arm_feature_set fpu_vfp_v3_or_neon_ext =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_V1 | FPU_VFP_EXT_V3);
static const arm_feature_set mve_ext =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE);
static const arm_feature_set mve_fp_ext =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP);
/* Note: This has more than one bit set, which means using it with
   mark_feature_used (which returns if *any* of the bits are set in the current
   cpu variant) can give surprising results.  */
static const arm_feature_set armv8m_fp =
  ARM_FEATURE_COPROC (FPU_VFP_V5_SP_D16);
#ifdef OBJ_ELF
static const arm_feature_set fpu_vfp_fp16 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_FP16);
static const arm_feature_set fpu_neon_ext_fma =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_FMA);
#endif
static const arm_feature_set fpu_vfp_ext_fma =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA);
static const arm_feature_set fpu_vfp_ext_armv8 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8);
static const arm_feature_set fpu_vfp_ext_armv8xd =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8xD);
static const arm_feature_set fpu_neon_ext_armv8 =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_ARMV8);
static const arm_feature_set fpu_crypto_ext_armv8 =
  ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8);
static const arm_feature_set fpu_neon_ext_v8_1 =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_RDMA);
static const arm_feature_set fpu_neon_ext_dotprod =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_DOTPROD);
static const arm_feature_set pacbti_ext =
  ARM_FEATURE_CORE_HIGH_HIGH (ARM_EXT3_PACBTI);

static int mfloat_abi_opt = -1;
/* Architecture feature bits selected by the last -mcpu/-march or .cpu/.arch
   directive.  */
static arm_feature_set selected_arch = ARM_ARCH_NONE;
/* Extension feature bits selected by the last -mcpu/-march or .arch_extension
   directive.  */
static arm_feature_set selected_ext = ARM_ARCH_NONE;
/* Feature bits selected by the last -mcpu/-march or by the combination of the
   last .cpu/.arch directive .arch_extension directives since that
   directive.  */
static arm_feature_set selected_cpu = ARM_ARCH_NONE;
/* FPU feature bits selected by the last -mfpu or .fpu directive.  */
static arm_feature_set selected_fpu = FPU_NONE;
/* Feature bits selected by the last .object_arch directive.  */
static arm_feature_set selected_object_arch = ARM_ARCH_NONE;
/* Must be long enough to hold any of the names in arm_cpus.  */
static const struct arm_ext_table * selected_ctx_ext_table = NULL;
static char selected_cpu_name[20];

extern FLONUM_TYPE generic_floating_point_number;

/* Return if no cpu was selected on command-line.  */
static bool
no_cpu_selected (void)
{
  return ARM_FEATURE_EQUAL (selected_cpu, arm_arch_none);
}

#ifdef OBJ_ELF
# ifdef EABI_DEFAULT
static int meabi_flags = EABI_DEFAULT;
# else
static int meabi_flags = EF_ARM_EABI_UNKNOWN;
# endif

static int attributes_set_explicitly[NUM_KNOWN_OBJ_ATTRIBUTES];

bool
arm_is_eabi (void)
{
  return (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4);
}
#endif

#ifdef OBJ_ELF
/* Pre-defined "_GLOBAL_OFFSET_TABLE_"	*/
symbolS * GOT_symbol;
#endif

/* 0: assemble for ARM,
   1: assemble for Thumb,
   2: assemble for Thumb even though target CPU does not support thumb
      instructions.  */
static int thumb_mode = 0;
/* A value distinct from the possible values for thumb_mode that we
   can use to record whether thumb_mode has been copied into the
   tc_frag_data field of a frag.  */
#define MODE_RECORDED (1 << 4)

/* Specifies the intrinsic IT insn behavior mode.  */
enum implicit_it_mode
{
  IMPLICIT_IT_MODE_NEVER  = 0x00,
  IMPLICIT_IT_MODE_ARM    = 0x01,
  IMPLICIT_IT_MODE_THUMB  = 0x02,
  IMPLICIT_IT_MODE_ALWAYS = (IMPLICIT_IT_MODE_ARM | IMPLICIT_IT_MODE_THUMB)
};
static int implicit_it_mode = IMPLICIT_IT_MODE_ARM;

/* If unified_syntax is true, we are processing the new unified
   ARM/Thumb syntax.  Important differences from the old ARM mode:

     - Immediate operands do not require a # prefix.
     - Conditional affixes always appear at the end of the
       instruction.  (For backward compatibility, those instructions
       that formerly had them in the middle, continue to accept them
       there.)
     - The IT instruction may appear, and if it does is validated
       against subsequent conditional affixes.  It does not generate
       machine code.

   Important differences from the old Thumb mode:

     - Immediate operands do not require a # prefix.
     - Most of the V6T2 instructions are only available in unified mode.
     - The .N and .W suffixes are recognized and honored (it is an error
       if they cannot be honored).
     - All instructions set the flags if and only if they have an 's' affix.
     - Conditional affixes may be used.  They are validated against
       preceding IT instructions.  Unlike ARM mode, you cannot use a
       conditional affix except in the scope of an IT instruction.  */

static bool unified_syntax = false;

/* An immediate operand can start with #, and ld*, st*, pld operands
   can contain [ and ].  We need to tell APP not to elide whitespace
   before a [, which can appear as the first operand for pld.
   Likewise, a { can appear as the first operand for push, pop, vld*, etc.  */
const char arm_symbol_chars[] = "#[]{}";

enum neon_el_type
{
  NT_invtype,
  NT_untyped,
  NT_integer,
  NT_float,
  NT_poly,
  NT_signed,
  NT_bfloat,
  NT_unsigned
};

struct neon_type_el
{
  enum neon_el_type type;
  unsigned size;
};

#define NEON_MAX_TYPE_ELS 5

struct neon_type
{
  struct neon_type_el el[NEON_MAX_TYPE_ELS];
  unsigned elems;
};

enum pred_instruction_type
{
   OUTSIDE_PRED_INSN,
   INSIDE_VPT_INSN,
   INSIDE_IT_INSN,
   INSIDE_IT_LAST_INSN,
   IF_INSIDE_IT_LAST_INSN, /* Either outside or inside;
			      if inside, should be the last one.  */
   NEUTRAL_IT_INSN,        /* This could be either inside or outside,
			      i.e. BKPT and NOP.  */
   IT_INSN,		   /* The IT insn has been parsed.  */
   VPT_INSN,		   /* The VPT/VPST insn has been parsed.  */
   MVE_OUTSIDE_PRED_INSN , /* Instruction to indicate a MVE instruction without
			      a predication code.  */
   MVE_UNPREDICABLE_INSN,  /* MVE instruction that is non-predicable.  */
};

/* The maximum number of operands we need.  */
#define ARM_IT_MAX_OPERANDS 6
#define ARM_IT_MAX_RELOCS 3

struct arm_it
{
  const char *	error;
  unsigned long instruction;
  unsigned int	size;
  unsigned int	size_req;
  unsigned int	cond;
  /* "uncond_value" is set to the value in place of the conditional field in
     unconditional versions of the instruction, or -1u if nothing is
     appropriate.  */
  unsigned int	uncond_value;
  struct neon_type vectype;
  /* This does not indicate an actual NEON instruction, only that
     the mnemonic accepts neon-style type suffixes.  */
  int		is_neon;
  /* Set to the opcode if the instruction needs relaxation.
     Zero if the instruction is not relaxed.  */
  unsigned long	relax;
  struct
  {
    bfd_reloc_code_real_type type;
    expressionS		     exp;
    int			     pc_rel;
  } relocs[ARM_IT_MAX_RELOCS];

  enum pred_instruction_type pred_insn_type;

  struct
  {
    unsigned reg;
    signed int imm;
    struct neon_type_el vectype;
    unsigned present	: 1;  /* Operand present.  */
    unsigned isreg	: 1;  /* Operand was a register.  */
    unsigned immisreg	: 2;  /* .imm field is a second register.
				 0: imm, 1: gpr, 2: MVE Q-register.  */
    unsigned isscalar   : 2;  /* Operand is a (SIMD) scalar:
				 0) not scalar,
				 1) Neon scalar,
				 2) MVE scalar.  */
    unsigned immisalign : 1;  /* Immediate is an alignment specifier.  */
    unsigned immisfloat : 1;  /* Immediate was parsed as a float.  */
    /* Note: we abuse "regisimm" to mean "is Neon register" in VMOV
       instructions. This allows us to disambiguate ARM <-> vector insns.  */
    unsigned regisimm   : 1;  /* 64-bit immediate, reg forms high 32 bits.  */
    unsigned isvec      : 1;  /* Is a single, double or quad VFP/Neon reg.  */
    unsigned isquad     : 1;  /* Operand is SIMD quad register.  */
    unsigned issingle   : 1;  /* Operand is VFP single-precision register.  */
    unsigned iszr	: 1;  /* Operand is ZR register.  */
    unsigned hasreloc	: 1;  /* Operand has relocation suffix.  */
    unsigned writeback	: 1;  /* Operand has trailing !  */
    unsigned preind	: 1;  /* Preindexed address.  */
    unsigned postind	: 1;  /* Postindexed address.  */
    unsigned negative	: 1;  /* Index register was negated.  */
    unsigned shifted	: 1;  /* Shift applied to operation.  */
    unsigned shift_kind : 3;  /* Shift operation (enum shift_kind).  */
  } operands[ARM_IT_MAX_OPERANDS];
};

static struct arm_it inst;

#define NUM_FLOAT_VALS 8

const char * fp_const[] =
{
  "0.0", "1.0", "2.0", "3.0", "4.0", "5.0", "0.5", "10.0", 0
};

LITTLENUM_TYPE fp_values[NUM_FLOAT_VALS][MAX_LITTLENUMS];

#define FAIL	(-1)
#define SUCCESS (0)

#define SUFF_S 1
#define SUFF_D 2
#define SUFF_E 3
#define SUFF_P 4

#define CP_T_X	 0x00008000
#define CP_T_Y	 0x00400000

#define CONDS_BIT	 0x00100000
#define LOAD_BIT	 0x00100000

#define DOUBLE_LOAD_FLAG 0x00000001

struct asm_cond
{
  const char *	 template_name;
  unsigned long  value;
};

#define COND_ALWAYS 0xE

struct asm_psr
{
  const char *   template_name;
  unsigned long  field;
};

struct asm_barrier_opt
{
  const char *    template_name;
  unsigned long   value;
  const arm_feature_set arch;
};

/* The bit that distinguishes CPSR and SPSR.  */
#define SPSR_BIT   (1 << 22)

/* The individual PSR flag bits.  */
#define PSR_c	(1 << 16)
#define PSR_x	(1 << 17)
#define PSR_s	(1 << 18)
#define PSR_f	(1 << 19)

struct reloc_entry
{
  const char *              name;
  bfd_reloc_code_real_type  reloc;
};

enum vfp_reg_pos
{
  VFP_REG_Sd, VFP_REG_Sm, VFP_REG_Sn,
  VFP_REG_Dd, VFP_REG_Dm, VFP_REG_Dn
};

enum vfp_ldstm_type
{
  VFP_LDSTMIA, VFP_LDSTMDB, VFP_LDSTMIAX, VFP_LDSTMDBX
};

/* Bits for DEFINED field in neon_typed_alias.  */
#define NTA_HASTYPE  1
#define NTA_HASINDEX 2

struct neon_typed_alias
{
  unsigned char        defined;
  unsigned char        index;
  struct neon_type_el  eltype;
};

/* ARM register categories.  This includes coprocessor numbers and various
   architecture extensions' registers.  Each entry should have an error message
   in reg_expected_msgs below.  */
enum arm_reg_type
{
  REG_TYPE_RN,
  REG_TYPE_CP,
  REG_TYPE_CN,
  REG_TYPE_FN,
  REG_TYPE_VFS,
  REG_TYPE_VFD,
  REG_TYPE_NQ,
  REG_TYPE_VFSD,
  REG_TYPE_NDQ,
  REG_TYPE_NSD,
  REG_TYPE_NSDQ,
  REG_TYPE_VFC,
  REG_TYPE_MVF,
  REG_TYPE_MVD,
  REG_TYPE_MVFX,
  REG_TYPE_MVDX,
  REG_TYPE_MVAX,
  REG_TYPE_MQ,
  REG_TYPE_DSPSC,
  REG_TYPE_MMXWR,
  REG_TYPE_MMXWC,
  REG_TYPE_MMXWCG,
  REG_TYPE_XSCALE,
  REG_TYPE_RNB,
  REG_TYPE_ZR,
  REG_TYPE_PSEUDO
};

/* Structure for a hash table entry for a register.
   If TYPE is REG_TYPE_VFD or REG_TYPE_NQ, the NEON field can point to extra
   information which states whether a vector type or index is specified (for a
   register alias created with .dn or .qn). Otherwise NEON should be NULL.  */
struct reg_entry
{
  const char *               name;
  unsigned int               number;
  unsigned char              type;
  unsigned char              builtin;
  struct neon_typed_alias *  neon;
};

/* Diagnostics used when we don't get a register of the expected type.	*/
const char * const reg_expected_msgs[] =
{
  [REG_TYPE_RN]	    = N_("ARM register expected"),
  [REG_TYPE_CP]	    = N_("bad or missing co-processor number"),
  [REG_TYPE_CN]	    = N_("co-processor register expected"),
  [REG_TYPE_FN]	    = N_("FPA register expected"),
  [REG_TYPE_VFS]    = N_("VFP single precision register expected"),
  [REG_TYPE_VFD]    = N_("VFP/Neon double precision register expected"),
  [REG_TYPE_NQ]	    = N_("Neon quad precision register expected"),
  [REG_TYPE_VFSD]   = N_("VFP single or double precision register expected"),
  [REG_TYPE_NDQ]    = N_("Neon double or quad precision register expected"),
  [REG_TYPE_NSD]    = N_("Neon single or double precision register expected"),
  [REG_TYPE_NSDQ]   = N_("VFP single, double or Neon quad precision register"
			 " expected"),
  [REG_TYPE_VFC]    = N_("VFP system register expected"),
  [REG_TYPE_MVF]    = N_("Maverick MVF register expected"),
  [REG_TYPE_MVD]    = N_("Maverick MVD register expected"),
  [REG_TYPE_MVFX]   = N_("Maverick MVFX register expected"),
  [REG_TYPE_MVDX]   = N_("Maverick MVDX register expected"),
  [REG_TYPE_MVAX]   = N_("Maverick MVAX register expected"),
  [REG_TYPE_DSPSC]  = N_("Maverick DSPSC register expected"),
  [REG_TYPE_MMXWR]  = N_("iWMMXt data register expected"),
  [REG_TYPE_MMXWC]  = N_("iWMMXt control register expected"),
  [REG_TYPE_MMXWCG] = N_("iWMMXt scalar register expected"),
  [REG_TYPE_XSCALE] = N_("XScale accumulator register expected"),
  [REG_TYPE_MQ]	    = N_("MVE vector register expected"),
  [REG_TYPE_RNB]    = "",
  [REG_TYPE_ZR]     = N_("ZR register expected"),
  [REG_TYPE_PSEUDO] = N_("Pseudo register expected"),
};

/* Some well known registers that we refer to directly elsewhere.  */
#define REG_R12	12
#define REG_SP	13
#define REG_LR	14
#define REG_PC	15
#define REG_RA_AUTH_CODE 143

/* ARM instructions take 4bytes in the object file, Thumb instructions
   take 2:  */
#define INSN_SIZE	4

struct asm_opcode
{
  /* Basic string to match.  */
  const char * template_name;

  /* Parameters to instruction.	 */
  unsigned int operands[8];

  /* Conditional tag - see opcode_lookup.  */
  unsigned int tag : 4;

  /* Basic instruction code.  */
  unsigned int avalue;

  /* Thumb-format instruction code.  */
  unsigned int tvalue;

  /* Which architecture variant provides this instruction.  */
  const arm_feature_set * avariant;
  const arm_feature_set * tvariant;

  /* Function to call to encode instruction in ARM format.  */
  void (* aencode) (void);

  /* Function to call to encode instruction in Thumb format.  */
  void (* tencode) (void);

  /* Indicates whether this instruction may be vector predicated.  */
  unsigned int mayBeVecPred : 1;
};

/* Defines for various bits that we will want to toggle.  */
#define INST_IMMEDIATE	0x02000000
#define OFFSET_REG	0x02000000
#define HWOFFSET_IMM	0x00400000
#define SHIFT_BY_REG	0x00000010
#define PRE_INDEX	0x01000000
#define INDEX_UP	0x00800000
#define WRITE_BACK	0x00200000
#define LDM_TYPE_2_OR_3	0x00400000
#define CPSI_MMOD	0x00020000

#define LITERAL_MASK	0xf000f000
#define OPCODE_MASK	0xfe1fffff
#define V4_STR_BIT	0x00000020
#define VLDR_VMOV_SAME	0x0040f000

#define T2_SUBS_PC_LR	0xf3de8f00

#define DATA_OP_SHIFT	21
#define SBIT_SHIFT	20

#define T2_OPCODE_MASK	0xfe1fffff
#define T2_DATA_OP_SHIFT 21
#define T2_SBIT_SHIFT	 20

#define A_COND_MASK         0xf0000000
#define A_PUSH_POP_OP_MASK  0x0fff0000

/* Opcodes for pushing/poping registers to/from the stack.  */
#define A1_OPCODE_PUSH    0x092d0000
#define A2_OPCODE_PUSH    0x052d0004
#define A2_OPCODE_POP     0x049d0004

/* Codes to distinguish the arithmetic instructions.  */
#define OPCODE_AND	0
#define OPCODE_EOR	1
#define OPCODE_SUB	2
#define OPCODE_RSB	3
#define OPCODE_ADD	4
#define OPCODE_ADC	5
#define OPCODE_SBC	6
#define OPCODE_RSC	7
#define OPCODE_TST	8
#define OPCODE_TEQ	9
#define OPCODE_CMP	10
#define OPCODE_CMN	11
#define OPCODE_ORR	12
#define OPCODE_MOV	13
#define OPCODE_BIC	14
#define OPCODE_MVN	15

#define T2_OPCODE_AND	0
#define T2_OPCODE_BIC	1
#define T2_OPCODE_ORR	2
#define T2_OPCODE_ORN	3
#define T2_OPCODE_EOR	4
#define T2_OPCODE_ADD	8
#define T2_OPCODE_ADC	10
#define T2_OPCODE_SBC	11
#define T2_OPCODE_SUB	13
#define T2_OPCODE_RSB	14

#define T_OPCODE_MUL 0x4340
#define T_OPCODE_TST 0x4200
#define T_OPCODE_CMN 0x42c0
#define T_OPCODE_NEG 0x4240
#define T_OPCODE_MVN 0x43c0

#define T_OPCODE_ADD_R3	0x1800
#define T_OPCODE_SUB_R3 0x1a00
#define T_OPCODE_ADD_HI 0x4400
#define T_OPCODE_ADD_ST 0xb000
#define T_OPCODE_SUB_ST 0xb080
#define T_OPCODE_ADD_SP 0xa800
#define T_OPCODE_ADD_PC 0xa000
#define T_OPCODE_ADD_I8 0x3000
#define T_OPCODE_SUB_I8 0x3800
#define T_OPCODE_ADD_I3 0x1c00
#define T_OPCODE_SUB_I3 0x1e00

#define T_OPCODE_ASR_R	0x4100
#define T_OPCODE_LSL_R	0x4080
#define T_OPCODE_LSR_R	0x40c0
#define T_OPCODE_ROR_R	0x41c0
#define T_OPCODE_ASR_I	0x1000
#define T_OPCODE_LSL_I	0x0000
#define T_OPCODE_LSR_I	0x0800

#define T_OPCODE_MOV_I8	0x2000
#define T_OPCODE_CMP_I8 0x2800
#define T_OPCODE_CMP_LR 0x4280
#define T_OPCODE_MOV_HR 0x4600
#define T_OPCODE_CMP_HR 0x4500

#define T_OPCODE_LDR_PC 0x4800
#define T_OPCODE_LDR_SP 0x9800
#define T_OPCODE_STR_SP 0x9000
#define T_OPCODE_LDR_IW 0x6800
#define T_OPCODE_STR_IW 0x6000
#define T_OPCODE_LDR_IH 0x8800
#define T_OPCODE_STR_IH 0x8000
#define T_OPCODE_LDR_IB 0x7800
#define T_OPCODE_STR_IB 0x7000
#define T_OPCODE_LDR_RW 0x5800
#define T_OPCODE_STR_RW 0x5000
#define T_OPCODE_LDR_RH 0x5a00
#define T_OPCODE_STR_RH 0x5200
#define T_OPCODE_LDR_RB 0x5c00
#define T_OPCODE_STR_RB 0x5400

#define T_OPCODE_PUSH	0xb400
#define T_OPCODE_POP	0xbc00

#define T_OPCODE_BRANCH 0xe000

#define THUMB_SIZE	2	/* Size of thumb instruction.  */
#define THUMB_PP_PC_LR 0x0100
#define THUMB_LOAD_BIT 0x0800
#define THUMB2_LOAD_BIT 0x00100000

#define BAD_SYNTAX	_("syntax error")
#define BAD_ARGS	_("bad arguments to instruction")
#define BAD_SP          _("r13 not allowed here")
#define BAD_PC		_("r15 not allowed here")
#define BAD_ODD		_("Odd register not allowed here")
#define BAD_EVEN	_("Even register not allowed here")
#define BAD_COND	_("instruction cannot be conditional")
#define BAD_OVERLAP	_("registers may not be the same")
#define BAD_HIREG	_("lo register required")
#define BAD_THUMB32	_("instruction not supported in Thumb16 mode")
#define BAD_ADDR_MODE   _("instruction does not accept this addressing mode")
#define BAD_BRANCH	_("branch must be last instruction in IT block")
#define BAD_BRANCH_OFF	_("branch out of range or not a multiple of 2")
#define BAD_NO_VPT	_("instruction not allowed in VPT block")
#define BAD_NOT_IT	_("instruction not allowed in IT block")
#define BAD_NOT_VPT	_("instruction missing MVE vector predication code")
#define BAD_FPU		_("selected FPU does not support instruction")
#define BAD_OUT_IT 	_("thumb conditional instruction should be in IT block")
#define BAD_OUT_VPT	\
	_("vector predicated instruction should be in VPT/VPST block")
#define BAD_IT_COND	_("incorrect condition in IT block")
#define BAD_VPT_COND	_("incorrect condition in VPT/VPST block")
#define BAD_IT_IT 	_("IT falling in the range of a previous IT block")
#define MISSING_FNSTART	_("missing .fnstart before unwinding directive")
#define BAD_PC_ADDRESSING \
	_("cannot use register index with PC-relative addressing")
#define BAD_PC_WRITEBACK \
	_("cannot use writeback with PC-relative addressing")
#define BAD_RANGE	_("branch out of range")
#define BAD_FP16	_("selected processor does not support fp16 instruction")
#define BAD_BF16	_("selected processor does not support bf16 instruction")
#define BAD_CDE	_("selected processor does not support cde instruction")
#define BAD_CDE_COPROC	_("coprocessor for insn is not enabled for cde")
#define UNPRED_REG(R)	_("using " R " results in unpredictable behaviour")
#define THUMB1_RELOC_ONLY  _("relocation valid in thumb1 code only")
#define MVE_NOT_IT	_("Warning: instruction is UNPREDICTABLE in an IT " \
			  "block")
#define MVE_NOT_VPT	_("Warning: instruction is UNPREDICTABLE in a VPT " \
			  "block")
#define MVE_BAD_PC	_("Warning: instruction is UNPREDICTABLE with PC" \
			  " operand")
#define MVE_BAD_SP	_("Warning: instruction is UNPREDICTABLE with SP" \
			  " operand")
#define BAD_SIMD_TYPE	_("bad type in SIMD instruction")
#define BAD_MVE_AUTO	\
  _("GAS auto-detection mode and -march=all is deprecated for MVE, please" \
    " use a valid -march or -mcpu option.")
#define BAD_MVE_SRCDEST	_("Warning: 32-bit element size and same destination "\
			  "and source operands makes instruction UNPREDICTABLE")
#define BAD_EL_TYPE	_("bad element type for instruction")
#define MVE_BAD_QREG	_("MVE vector register Q[0..7] expected")
#define BAD_PACBTI	_("selected processor does not support PACBTI extention")

static htab_t  arm_ops_hsh;
static htab_t  arm_cond_hsh;
static htab_t  arm_vcond_hsh;
static htab_t  arm_shift_hsh;
static htab_t  arm_psr_hsh;
static htab_t  arm_v7m_psr_hsh;
static htab_t  arm_reg_hsh;
static htab_t  arm_reloc_hsh;
static htab_t  arm_barrier_opt_hsh;

/* Stuff needed to resolve the label ambiguity
   As:
     ...
     label:   <insn>
   may differ from:
     ...
     label:
	      <insn>  */

symbolS *  last_label_seen;
static int label_is_thumb_function_name = false;

/* Literal pool structure.  Held on a per-section
   and per-sub-section basis.  */

#define MAX_LITERAL_POOL_SIZE 1024
typedef struct literal_pool
{
  expressionS	         literals [MAX_LITERAL_POOL_SIZE];
  unsigned int	         next_free_entry;
  unsigned int	         id;
  symbolS *	         symbol;
  segT		         section;
  subsegT	         sub_section;
#ifdef OBJ_ELF
  struct dwarf2_line_info locs [MAX_LITERAL_POOL_SIZE];
#endif
  struct literal_pool *  next;
  unsigned int		 alignment;
} literal_pool;

/* Pointer to a linked list of literal pools.  */
literal_pool * list_of_pools = NULL;

typedef enum asmfunc_states
{
  OUTSIDE_ASMFUNC,
  WAITING_ASMFUNC_NAME,
  WAITING_ENDASMFUNC
} asmfunc_states;

static asmfunc_states asmfunc_state = OUTSIDE_ASMFUNC;

#ifdef OBJ_ELF
#  define now_pred seg_info (now_seg)->tc_segment_info_data.current_pred
#else
static struct current_pred now_pred;
#endif

static inline int
now_pred_compatible (int cond)
{
  return (cond & ~1) == (now_pred.cc & ~1);
}

static inline int
conditional_insn (void)
{
  return inst.cond != COND_ALWAYS;
}

static int in_pred_block (void);

static int handle_pred_state (void);

static void force_automatic_it_block_close (void);

static void it_fsm_post_encode (void);

#define set_pred_insn_type(type)			\
  do						\
    {						\
      inst.pred_insn_type = type;			\
      if (handle_pred_state () == FAIL)		\
	return;					\
    }						\
  while (0)

#define set_pred_insn_type_nonvoid(type, failret) \
  do						\
    {                                           \
      inst.pred_insn_type = type;			\
      if (handle_pred_state () == FAIL)		\
	return failret;				\
    }						\
  while(0)

#define set_pred_insn_type_last()				\
  do							\
    {							\
      if (inst.cond == COND_ALWAYS)			\
	set_pred_insn_type (IF_INSIDE_IT_LAST_INSN);	\
      else						\
	set_pred_insn_type (INSIDE_IT_LAST_INSN);		\
    }							\
  while (0)

/* Toggle value[pos].  */
#define TOGGLE_BIT(value, pos) (value ^ (1 << pos))

/* Pure syntax.	 */

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.	 */
char arm_comment_chars[] = "@";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.	*/
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
/* Also note that comments like this one will always work.  */
const char line_comment_chars[] = "#";

char arm_line_separator_chars[] = ";";

/* Chars that can be used to separate mant
   from exp in floating point numbers.	*/
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.  */
/* As in 0f12.456  */
/* or	 0d1.2345e12  */

const char FLT_CHARS[] = "rRsSfFdDxXeEpPHh";

/* Prefix characters that indicate the start of an immediate
   value.  */
#define is_immediate_prefix(C) ((C) == '#' || (C) == '$')

/* Separator character handling.  */

#define skip_whitespace(str)  do { if (*(str) == ' ') ++(str); } while (0)

enum fp_16bit_format
{
  ARM_FP16_FORMAT_IEEE		= 0x1,
  ARM_FP16_FORMAT_ALTERNATIVE	= 0x2,
  ARM_FP16_FORMAT_DEFAULT	= 0x3
};

static enum fp_16bit_format fp16_format = ARM_FP16_FORMAT_DEFAULT;


static inline int
skip_past_char (char ** str, char c)
{
  /* PR gas/14987: Allow for whitespace before the expected character.  */
  skip_whitespace (*str);

  if (**str == c)
    {
      (*str)++;
      return SUCCESS;
    }
  else
    return FAIL;
}

#define skip_past_comma(str) skip_past_char (str, ',')

/* Arithmetic expressions (possibly involving symbols).	 */

/* Return TRUE if anything in the expression is a bignum.  */

static bool
walk_no_bignums (symbolS * sp)
{
  if (symbol_get_value_expression (sp)->X_op == O_big)
    return true;

  if (symbol_get_value_expression (sp)->X_add_symbol)
    {
      return (walk_no_bignums (symbol_get_value_expression (sp)->X_add_symbol)
	      || (symbol_get_value_expression (sp)->X_op_symbol
		  && walk_no_bignums (symbol_get_value_expression (sp)->X_op_symbol)));
    }

  return false;
}

static bool in_my_get_expression = false;

/* Third argument to my_get_expression.	 */
#define GE_NO_PREFIX 0
#define GE_IMM_PREFIX 1
#define GE_OPT_PREFIX 2
/* This is a bit of a hack. Use an optional prefix, and also allow big (64-bit)
   immediates, as can be used in Neon VMVN and VMOV immediate instructions.  */
#define GE_OPT_PREFIX_BIG 3

static int
my_get_expression (expressionS * ep, char ** str, int prefix_mode)
{
  char * save_in;

  /* In unified syntax, all prefixes are optional.  */
  if (unified_syntax)
    prefix_mode = (prefix_mode == GE_OPT_PREFIX_BIG) ? prefix_mode
		  : GE_OPT_PREFIX;

  switch (prefix_mode)
    {
    case GE_NO_PREFIX: break;
    case GE_IMM_PREFIX:
      if (!is_immediate_prefix (**str))
	{
	  inst.error = _("immediate expression requires a # prefix");
	  return FAIL;
	}
      (*str)++;
      break;
    case GE_OPT_PREFIX:
    case GE_OPT_PREFIX_BIG:
      if (is_immediate_prefix (**str))
	(*str)++;
      break;
    default:
      abort ();
    }

  memset (ep, 0, sizeof (expressionS));

  save_in = input_line_pointer;
  input_line_pointer = *str;
  in_my_get_expression = true;
  expression (ep);
  in_my_get_expression = false;

  if (ep->X_op == O_illegal || ep->X_op == O_absent)
    {
      /* We found a bad or missing expression in md_operand().  */
      *str = input_line_pointer;
      input_line_pointer = save_in;
      if (inst.error == NULL)
	inst.error = (ep->X_op == O_absent
		      ? _("missing expression") :_("bad expression"));
      return 1;
    }

  /* Get rid of any bignums now, so that we don't generate an error for which
     we can't establish a line number later on.	 Big numbers are never valid
     in instructions, which is where this routine is always called.  */
  if (prefix_mode != GE_OPT_PREFIX_BIG
      && (ep->X_op == O_big
	  || (ep->X_add_symbol
	      && (walk_no_bignums (ep->X_add_symbol)
		  || (ep->X_op_symbol
		      && walk_no_bignums (ep->X_op_symbol))))))
    {
      inst.error = _("invalid constant");
      *str = input_line_pointer;
      input_line_pointer = save_in;
      return 1;
    }

  *str = input_line_pointer;
  input_line_pointer = save_in;
  return SUCCESS;
}

/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.

   Note that fp constants aren't represent in the normal way on the ARM.
   In big endian mode, things are as expected.	However, in little endian
   mode fp constants are big-endian word-wise, and little-endian byte-wise
   within the words.  For example, (double) 1.1 in big endian mode is
   the byte sequence 3f f1 99 99 99 99 99 9a, and in little endian mode is
   the byte sequence 99 99 f1 3f 9a 99 99 99.

   ??? The format of 12 byte floats is uncertain according to gcc's arm.h.  */

const char *
md_atof (int type, char * litP, int * sizeP)
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  char *t;
  int i;

  switch (type)
    {
    case 'H':
    case 'h':
    /* bfloat16, despite not being part of the IEEE specification, can also
       be handled by atof_ieee().  */
    case 'b':
      prec = 1;
      break;

    case 'f':
    case 'F':
    case 's':
    case 'S':
      prec = 2;
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
      prec = 4;
      break;

    case 'x':
    case 'X':
      prec = 5;
      break;

    case 'p':
    case 'P':
      prec = 5;
      break;

    default:
      *sizeP = 0;
      return _("Unrecognized or unsupported floating point constant");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  *sizeP = prec * sizeof (LITTLENUM_TYPE);

  if (target_big_endian || prec == 1)
    for (i = 0; i < prec; i++)
      {
	md_number_to_chars (litP, (valueT) words[i], sizeof (LITTLENUM_TYPE));
	litP += sizeof (LITTLENUM_TYPE);
      }
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_endian_pure))
    for (i = prec - 1; i >= 0; i--)
      {
	md_number_to_chars (litP, (valueT) words[i], sizeof (LITTLENUM_TYPE));
	litP += sizeof (LITTLENUM_TYPE);
      }
  else
    /* For a 4 byte float the order of elements in `words' is 1 0.
       For an 8 byte float the order is 1 0 3 2.  */
    for (i = 0; i < prec; i += 2)
      {
	md_number_to_chars (litP, (valueT) words[i + 1],
			    sizeof (LITTLENUM_TYPE));
	md_number_to_chars (litP + sizeof (LITTLENUM_TYPE),
			    (valueT) words[i], sizeof (LITTLENUM_TYPE));
	litP += 2 * sizeof (LITTLENUM_TYPE);
      }

  return NULL;
}

/* We handle all bad expressions here, so that we can report the faulty
   instruction in the error message.  */

void
md_operand (expressionS * exp)
{
  if (in_my_get_expression)
    exp->X_op = O_illegal;
}

/* Immediate values.  */

#ifdef OBJ_ELF
/* Generic immediate-value read function for use in directives.
   Accepts anything that 'expression' can fold to a constant.
   *val receives the number.  */

static int
immediate_for_directive (int *val)
{
  expressionS exp;
  exp.X_op = O_illegal;

  if (is_immediate_prefix (*input_line_pointer))
    {
      input_line_pointer++;
      expression (&exp);
    }

  if (exp.X_op != O_constant)
    {
      as_bad (_("expected #constant"));
      ignore_rest_of_line ();
      return FAIL;
    }
  *val = exp.X_add_number;
  return SUCCESS;
}
#endif

/* Register parsing.  */

/* Generic register parser.  CCP points to what should be the
   beginning of a register name.  If it is indeed a valid register
   name, advance CCP over it and return the reg_entry structure;
   otherwise return NULL.  Does not issue diagnostics.	*/

static struct reg_entry *
arm_reg_parse_multi (char **ccp)
{
  char *start = *ccp;
  char *p;
  struct reg_entry *reg;

  skip_whitespace (start);

#ifdef REGISTER_PREFIX
  if (*start != REGISTER_PREFIX)
    return NULL;
  start++;
#endif
#ifdef OPTIONAL_REGISTER_PREFIX
  if (*start == OPTIONAL_REGISTER_PREFIX)
    start++;
#endif

  p = start;
  if (!ISALPHA (*p) || !is_name_beginner (*p))
    return NULL;

  do
    p++;
  while (ISALPHA (*p) || ISDIGIT (*p) || *p == '_');

  reg = (struct reg_entry *) str_hash_find_n (arm_reg_hsh, start, p - start);

  if (!reg)
    return NULL;

  *ccp = p;
  return reg;
}

static int
arm_reg_alt_syntax (char **ccp, char *start, struct reg_entry *reg,
		    enum arm_reg_type type)
{
  /* Alternative syntaxes are accepted for a few register classes.  */
  switch (type)
    {
    case REG_TYPE_MVF:
    case REG_TYPE_MVD:
    case REG_TYPE_MVFX:
    case REG_TYPE_MVDX:
      /* Generic coprocessor register names are allowed for these.  */
      if (reg && reg->type == REG_TYPE_CN)
	return reg->number;
      break;

    case REG_TYPE_CP:
      /* For backward compatibility, a bare number is valid here.  */
      {
	unsigned long processor = strtoul (start, ccp, 10);
	if (*ccp != start && processor <= 15)
	  return processor;
      }
      /* Fall through.  */

    case REG_TYPE_MMXWC:
      /* WC includes WCG.  ??? I'm not sure this is true for all
	 instructions that take WC registers.  */
      if (reg && reg->type == REG_TYPE_MMXWCG)
	return reg->number;
      break;

    default:
      break;
    }

  return FAIL;
}

/* As arm_reg_parse_multi, but the register must be of type TYPE, and the
   return value is the register number or FAIL.  */

static int
arm_reg_parse (char **ccp, enum arm_reg_type type)
{
  char *start = *ccp;
  struct reg_entry *reg = arm_reg_parse_multi (ccp);
  int ret;

  /* Do not allow a scalar (reg+index) to parse as a register.  */
  if (reg && reg->neon && (reg->neon->defined & NTA_HASINDEX))
    return FAIL;

  if (reg && reg->type == type)
    return reg->number;

  if ((ret = arm_reg_alt_syntax (ccp, start, reg, type)) != FAIL)
    return ret;

  *ccp = start;
  return FAIL;
}

/* Parse a Neon type specifier. *STR should point at the leading '.'
   character. Does no verification at this stage that the type fits the opcode
   properly. E.g.,

     .i32.i32.s16
     .s32.f32
     .u16

   Can all be legally parsed by this function.

   Fills in neon_type struct pointer with parsed information, and updates STR
   to point after the parsed type specifier. Returns SUCCESS if this was a legal
   type, FAIL if not.  */

static int
parse_neon_type (struct neon_type *type, char **str)
{
  char *ptr = *str;

  if (type)
    type->elems = 0;

  while (type->elems < NEON_MAX_TYPE_ELS)
    {
      enum neon_el_type thistype = NT_untyped;
      unsigned thissize = -1u;

      if (*ptr != '.')
	break;

      ptr++;

      /* Just a size without an explicit type.  */
      if (ISDIGIT (*ptr))
	goto parsesize;

      switch (TOLOWER (*ptr))
	{
	case 'i': thistype = NT_integer; break;
	case 'f': thistype = NT_float; break;
	case 'p': thistype = NT_poly; break;
	case 's': thistype = NT_signed; break;
	case 'u': thistype = NT_unsigned; break;
	case 'd':
	  thistype = NT_float;
	  thissize = 64;
	  ptr++;
	  goto done;
	case 'b':
	  thistype = NT_bfloat;
	  switch (TOLOWER (*(++ptr)))
	    {
	    case 'f':
	      ptr += 1;
	      thissize = strtoul (ptr, &ptr, 10);
	      if (thissize != 16)
		{
		  as_bad (_("bad size %d in type specifier"), thissize);
		  return FAIL;
		}
	      goto done;
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	    case ' ': case '.':
	      as_bad (_("unexpected type character `b' -- did you mean `bf'?"));
	      return FAIL;
	    default:
	      break;
	    }
	  break;
	default:
	  as_bad (_("unexpected character `%c' in type specifier"), *ptr);
	  return FAIL;
	}

      ptr++;

      /* .f is an abbreviation for .f32.  */
      if (thistype == NT_float && !ISDIGIT (*ptr))
	thissize = 32;
      else
	{
	parsesize:
	  thissize = strtoul (ptr, &ptr, 10);

	  if (thissize != 8 && thissize != 16 && thissize != 32
	      && thissize != 64)
	    {
	      as_bad (_("bad size %d in type specifier"), thissize);
	      return FAIL;
	    }
	}

      done:
      if (type)
	{
	  type->el[type->elems].type = thistype;
	  type->el[type->elems].size = thissize;
	  type->elems++;
	}
    }

  /* Empty/missing type is not a successful parse.  */
  if (type->elems == 0)
    return FAIL;

  *str = ptr;

  return SUCCESS;
}

/* Errors may be set multiple times during parsing or bit encoding
   (particularly in the Neon bits), but usually the earliest error which is set
   will be the most meaningful. Avoid overwriting it with later (cascading)
   errors by calling this function.  */

static void
first_error (const char *err)
{
  if (!inst.error)
    inst.error = err;
}

/* Parse a single type, e.g. ".s32", leading period included.  */
static int
parse_neon_operand_type (struct neon_type_el *vectype, char **ccp)
{
  char *str = *ccp;
  struct neon_type optype;

  if (*str == '.')
    {
      if (parse_neon_type (&optype, &str) == SUCCESS)
	{
	  if (optype.elems == 1)
	    *vectype = optype.el[0];
	  else
	    {
	      first_error (_("only one type should be specified for operand"));
	      return FAIL;
	    }
	}
      else
	{
	  first_error (_("vector type expected"));
	  return FAIL;
	}
    }
  else
    return FAIL;

  *ccp = str;

  return SUCCESS;
}

/* Special meanings for indices (which have a range of 0-7), which will fit into
   a 4-bit integer.  */

#define NEON_ALL_LANES		15
#define NEON_INTERLEAVE_LANES	14

/* Record a use of the given feature.  */
static void
record_feature_use (const arm_feature_set *feature)
{
  if (thumb_mode)
    ARM_MERGE_FEATURE_SETS (thumb_arch_used, thumb_arch_used, *feature);
  else
    ARM_MERGE_FEATURE_SETS (arm_arch_used, arm_arch_used, *feature);
}

/* If the given feature available in the selected CPU, mark it as used.
   Returns TRUE iff feature is available.  */
static bool
mark_feature_used (const arm_feature_set *feature)
{

  /* Do not support the use of MVE only instructions when in auto-detection or
     -march=all.  */
  if (((feature == &mve_ext) || (feature == &mve_fp_ext))
      && ARM_CPU_IS_ANY (cpu_variant))
    {
      first_error (BAD_MVE_AUTO);
      return false;
    }
  /* Ensure the option is valid on the current architecture.  */
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, *feature))
    return false;

  /* Add the appropriate architecture feature for the barrier option used.
     */
  record_feature_use (feature);

  return true;
}

/* Parse either a register or a scalar, with an optional type. Return the
   register number, and optionally fill in the actual type of the register
   when multiple alternatives were given (NEON_TYPE_NDQ) in *RTYPE, and
   type/index information in *TYPEINFO.  */

static int
parse_typed_reg_or_scalar (char **ccp, enum arm_reg_type type,
			   enum arm_reg_type *rtype,
			   struct neon_typed_alias *typeinfo)
{
  char *str = *ccp;
  struct reg_entry *reg = arm_reg_parse_multi (&str);
  struct neon_typed_alias atype;
  struct neon_type_el parsetype;

  atype.defined = 0;
  atype.index = -1;
  atype.eltype.type = NT_invtype;
  atype.eltype.size = -1;

  /* Try alternate syntax for some types of register. Note these are mutually
     exclusive with the Neon syntax extensions.  */
  if (reg == NULL)
    {
      int altreg = arm_reg_alt_syntax (&str, *ccp, reg, type);
      if (altreg != FAIL)
	*ccp = str;
      if (typeinfo)
	*typeinfo = atype;
      return altreg;
    }

  /* Undo polymorphism when a set of register types may be accepted.  */
  if ((type == REG_TYPE_NDQ
       && (reg->type == REG_TYPE_NQ || reg->type == REG_TYPE_VFD))
      || (type == REG_TYPE_VFSD
	  && (reg->type == REG_TYPE_VFS || reg->type == REG_TYPE_VFD))
      || (type == REG_TYPE_NSDQ
	  && (reg->type == REG_TYPE_VFS || reg->type == REG_TYPE_VFD
	      || reg->type == REG_TYPE_NQ))
      || (type == REG_TYPE_NSD
	  && (reg->type == REG_TYPE_VFS || reg->type == REG_TYPE_VFD))
      || (type == REG_TYPE_MMXWC
	  && (reg->type == REG_TYPE_MMXWCG)))
    type = (enum arm_reg_type) reg->type;

  if (type == REG_TYPE_MQ)
    {
      if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	return FAIL;

      if (!reg || reg->type != REG_TYPE_NQ)
	return FAIL;

      if (reg->number > 14 && !mark_feature_used (&fpu_vfp_ext_d32))
	{
	  first_error (_("expected MVE register [q0..q7]"));
	  return FAIL;
	}
      type = REG_TYPE_NQ;
    }
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
	   && (type == REG_TYPE_NQ))
    return FAIL;


  if (type != reg->type)
    return FAIL;

  if (reg->neon)
    atype = *reg->neon;

  if (parse_neon_operand_type (&parsetype, &str) == SUCCESS)
    {
      if ((atype.defined & NTA_HASTYPE) != 0)
	{
	  first_error (_("can't redefine type for operand"));
	  return FAIL;
	}
      atype.defined |= NTA_HASTYPE;
      atype.eltype = parsetype;
    }

  if (skip_past_char (&str, '[') == SUCCESS)
    {
      if (type != REG_TYPE_VFD
	  && !(type == REG_TYPE_VFS
	       && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8_2))
	  && !(type == REG_TYPE_NQ
	       && ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)))
	{
	  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	    first_error (_("only D and Q registers may be indexed"));
	  else
	    first_error (_("only D registers may be indexed"));
	  return FAIL;
	}

      if ((atype.defined & NTA_HASINDEX) != 0)
	{
	  first_error (_("can't change index for operand"));
	  return FAIL;
	}

      atype.defined |= NTA_HASINDEX;

      if (skip_past_char (&str, ']') == SUCCESS)
	atype.index = NEON_ALL_LANES;
      else
	{
	  expressionS exp;

	  my_get_expression (&exp, &str, GE_NO_PREFIX);

	  if (exp.X_op != O_constant)
	    {
	      first_error (_("constant expression required"));
	      return FAIL;
	    }

	  if (skip_past_char (&str, ']') == FAIL)
	    return FAIL;

	  atype.index = exp.X_add_number;
	}
    }

  if (typeinfo)
    *typeinfo = atype;

  if (rtype)
    *rtype = type;

  *ccp = str;

  return reg->number;
}

/* Like arm_reg_parse, but also allow the following extra features:
    - If RTYPE is non-zero, return the (possibly restricted) type of the
      register (e.g. Neon double or quad reg when either has been requested).
    - If this is a Neon vector type with additional type information, fill
      in the struct pointed to by VECTYPE (if non-NULL).
   This function will fault on encountering a scalar.  */

static int
arm_typed_reg_parse (char **ccp, enum arm_reg_type type,
		     enum arm_reg_type *rtype, struct neon_type_el *vectype)
{
  struct neon_typed_alias atype;
  char *str = *ccp;
  int reg = parse_typed_reg_or_scalar (&str, type, rtype, &atype);

  if (reg == FAIL)
    return FAIL;

  /* Do not allow regname(... to parse as a register.  */
  if (*str == '(')
    return FAIL;

  /* Do not allow a scalar (reg+index) to parse as a register.  */
  if ((atype.defined & NTA_HASINDEX) != 0)
    {
      first_error (_("register operand expected, but got scalar"));
      return FAIL;
    }

  if (vectype)
    *vectype = atype.eltype;

  *ccp = str;

  return reg;
}

#define NEON_SCALAR_REG(X)	((X) >> 4)
#define NEON_SCALAR_INDEX(X)	((X) & 15)

/* Parse a Neon scalar. Most of the time when we're parsing a scalar, we don't
   have enough information to be able to do a good job bounds-checking. So, we
   just do easy checks here, and do further checks later.  */

static int
parse_scalar (char **ccp, int elsize, struct neon_type_el *type, enum
	      arm_reg_type reg_type)
{
  int reg;
  char *str = *ccp;
  struct neon_typed_alias atype;
  unsigned reg_size;

  reg = parse_typed_reg_or_scalar (&str, reg_type, NULL, &atype);

  switch (reg_type)
    {
    case REG_TYPE_VFS:
      reg_size = 32;
      break;
    case REG_TYPE_VFD:
      reg_size = 64;
      break;
    case REG_TYPE_MQ:
      reg_size = 128;
      break;
    default:
      gas_assert (0);
      return FAIL;
    }

  if (reg == FAIL || (atype.defined & NTA_HASINDEX) == 0)
    return FAIL;

  if (reg_type != REG_TYPE_MQ && atype.index == NEON_ALL_LANES)
    {
      first_error (_("scalar must have an index"));
      return FAIL;
    }
  else if (atype.index >= reg_size / elsize)
    {
      first_error (_("scalar index out of range"));
      return FAIL;
    }

  if (type)
    *type = atype.eltype;

  *ccp = str;

  return reg * 16 + atype.index;
}

/* Types of registers in a list.  */

enum reg_list_els
{
  REGLIST_RN,
  REGLIST_PSEUDO,
  REGLIST_CLRM,
  REGLIST_VFP_S,
  REGLIST_VFP_S_VPR,
  REGLIST_VFP_D,
  REGLIST_VFP_D_VPR,
  REGLIST_NEON_D
};

/* Parse an ARM register list.  Returns the bitmask, or FAIL.  */

static long
parse_reg_list (char ** strp, enum reg_list_els etype)
{
  char *str = *strp;
  long range = 0;
  int another_range;

  gas_assert (etype == REGLIST_RN || etype == REGLIST_CLRM
	      || etype == REGLIST_PSEUDO);

  /* We come back here if we get ranges concatenated by '+' or '|'.  */
  do
    {
      skip_whitespace (str);

      another_range = 0;

      if (*str == '{')
	{
	  int in_range = 0;
	  int cur_reg = -1;

	  str++;
	  do
	    {
	      int reg;
	      const char apsr_str[] = "apsr";
	      int apsr_str_len = strlen (apsr_str);
	      enum arm_reg_type rt;

	      if (etype == REGLIST_RN || etype == REGLIST_CLRM)
		rt = REG_TYPE_RN;
	      else
		rt = REG_TYPE_PSEUDO;

	      reg = arm_reg_parse (&str, rt);

	      if (etype == REGLIST_CLRM)
		{
		  if (reg == REG_SP || reg == REG_PC)
		    reg = FAIL;
		  else if (reg == FAIL
			   && !strncasecmp (str, apsr_str, apsr_str_len)
			   && !ISALPHA (*(str + apsr_str_len)))
		    {
		      reg = 15;
		      str += apsr_str_len;
		    }

		  if (reg == FAIL)
		    {
		      first_error (_("r0-r12, lr or APSR expected"));
		      return FAIL;
		    }
		}
	      else if (etype == REGLIST_PSEUDO)
		{
		  if (reg == FAIL)
		    {
		      first_error (_(reg_expected_msgs[REG_TYPE_PSEUDO]));
		      return FAIL;
		    }
		}
	      else /* etype == REGLIST_RN.  */
		{
		  if (reg == FAIL)
		    {
		      first_error (_(reg_expected_msgs[REGLIST_RN]));
		      return FAIL;
		    }
		}

	      if (in_range)
		{
		  int i;

		  if (reg <= cur_reg)
		    {
		      first_error (_("bad range in register list"));
		      return FAIL;
		    }

		  for (i = cur_reg + 1; i < reg; i++)
		    {
		      if (range & (1 << i))
			as_tsktsk
			  (_("Warning: duplicated register (r%d) in register list"),
			   i);
		      else
			range |= 1 << i;
		    }
		  in_range = 0;
		}

	      if (range & (1 << reg))
		as_tsktsk (_("Warning: duplicated register (r%d) in register list"),
			   reg);
	      else if (reg <= cur_reg)
		as_tsktsk (_("Warning: register range not in ascending order"));

	      range |= 1 << reg;
	      cur_reg = reg;
	    }
	  while (skip_past_comma (&str) != FAIL
		 || (in_range = 1, *str++ == '-'));
	  str--;

	  if (skip_past_char (&str, '}') == FAIL)
	    {
	      first_error (_("missing `}'"));
	      return FAIL;
	    }
	}
      else if (etype == REGLIST_RN)
	{
	  expressionS exp;

	  if (my_get_expression (&exp, &str, GE_NO_PREFIX))
	    return FAIL;

	  if (exp.X_op == O_constant)
	    {
	      if (exp.X_add_number
		  != (exp.X_add_number & 0x0000ffff))
		{
		  inst.error = _("invalid register mask");
		  return FAIL;
		}

	      if ((range & exp.X_add_number) != 0)
		{
		  int regno = range & exp.X_add_number;

		  regno &= -regno;
		  regno = (1 << regno) - 1;
		  as_tsktsk
		    (_("Warning: duplicated register (r%d) in register list"),
		     regno);
		}

	      range |= exp.X_add_number;
	    }
	  else
	    {
	      if (inst.relocs[0].type != 0)
		{
		  inst.error = _("expression too complex");
		  return FAIL;
		}

	      memcpy (&inst.relocs[0].exp, &exp, sizeof (expressionS));
	      inst.relocs[0].type = BFD_RELOC_ARM_MULTI;
	      inst.relocs[0].pc_rel = 0;
	    }
	}

      if (*str == '|' || *str == '+')
	{
	  str++;
	  another_range = 1;
	}
    }
  while (another_range);

  *strp = str;
  return range;
}

/* Parse a VFP register list.  If the string is invalid return FAIL.
   Otherwise return the number of registers, and set PBASE to the first
   register.  Parses registers of type ETYPE.
   If REGLIST_NEON_D is used, several syntax enhancements are enabled:
     - Q registers can be used to specify pairs of D registers
     - { } can be omitted from around a singleton register list
	 FIXME: This is not implemented, as it would require backtracking in
	 some cases, e.g.:
	   vtbl.8 d3,d4,d5
	 This could be done (the meaning isn't really ambiguous), but doesn't
	 fit in well with the current parsing framework.
     - 32 D registers may be used (also true for VFPv3).
   FIXME: Types are ignored in these register lists, which is probably a
   bug.  */

static int
parse_vfp_reg_list (char **ccp, unsigned int *pbase, enum reg_list_els etype,
		    bool *partial_match)
{
  char *str = *ccp;
  int base_reg;
  int new_base;
  enum arm_reg_type regtype = (enum arm_reg_type) 0;
  int max_regs = 0;
  int count = 0;
  int warned = 0;
  unsigned long mask = 0;
  int i;
  bool vpr_seen = false;
  bool expect_vpr =
    (etype == REGLIST_VFP_S_VPR) || (etype == REGLIST_VFP_D_VPR);

  if (skip_past_char (&str, '{') == FAIL)
    {
      inst.error = _("expecting {");
      return FAIL;
    }

  switch (etype)
    {
    case REGLIST_VFP_S:
    case REGLIST_VFP_S_VPR:
      regtype = REG_TYPE_VFS;
      max_regs = 32;
      break;

    case REGLIST_VFP_D:
    case REGLIST_VFP_D_VPR:
      regtype = REG_TYPE_VFD;
      break;

    case REGLIST_NEON_D:
      regtype = REG_TYPE_NDQ;
      break;

    default:
      gas_assert (0);
    }

  if (etype != REGLIST_VFP_S && etype != REGLIST_VFP_S_VPR)
    {
      /* VFPv3 allows 32 D registers, except for the VFPv3-D16 variant.  */
      if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_d32))
	{
	  max_regs = 32;
	  if (thumb_mode)
	    ARM_MERGE_FEATURE_SETS (thumb_arch_used, thumb_arch_used,
				    fpu_vfp_ext_d32);
	  else
	    ARM_MERGE_FEATURE_SETS (arm_arch_used, arm_arch_used,
				    fpu_vfp_ext_d32);
	}
      else
	max_regs = 16;
    }

  base_reg = max_regs;
  *partial_match = false;

  do
    {
      unsigned int setmask = 1, addregs = 1;
      const char vpr_str[] = "vpr";
      size_t vpr_str_len = strlen (vpr_str);

      new_base = arm_typed_reg_parse (&str, regtype, &regtype, NULL);

      if (expect_vpr)
	{
	  if (new_base == FAIL
	      && !strncasecmp (str, vpr_str, vpr_str_len)
	      && !ISALPHA (*(str + vpr_str_len))
	      && !vpr_seen)
	    {
	      vpr_seen = true;
	      str += vpr_str_len;
	      if (count == 0)
		base_reg = 0; /* Canonicalize VPR only on d0 with 0 regs.  */
	    }
	  else if (vpr_seen)
	    {
	      first_error (_("VPR expected last"));
	      return FAIL;
	    }
	  else if (new_base == FAIL)
	    {
	      if (regtype == REG_TYPE_VFS)
		first_error (_("VFP single precision register or VPR "
			       "expected"));
	      else /* regtype == REG_TYPE_VFD.  */
		first_error (_("VFP/Neon double precision register or VPR "
			       "expected"));
	      return FAIL;
	    }
	}
      else if (new_base == FAIL)
	{
	  first_error (_(reg_expected_msgs[regtype]));
	  return FAIL;
	}

      *partial_match = true;
      if (vpr_seen)
	continue;

      if (new_base >= max_regs)
	{
	  first_error (_("register out of range in list"));
	  return FAIL;
	}

      /* Note: a value of 2 * n is returned for the register Q<n>.  */
      if (regtype == REG_TYPE_NQ)
	{
	  setmask = 3;
	  addregs = 2;
	}

      if (new_base < base_reg)
	base_reg = new_base;

      if (mask & (setmask << new_base))
	{
	  first_error (_("invalid register list"));
	  return FAIL;
	}

      if ((mask >> new_base) != 0 && ! warned && !vpr_seen)
	{
	  as_tsktsk (_("register list not in ascending order"));
	  warned = 1;
	}

      mask |= setmask << new_base;
      count += addregs;

      if (*str == '-') /* We have the start of a range expression */
	{
	  int high_range;

	  str++;

	  if ((high_range = arm_typed_reg_parse (&str, regtype, NULL, NULL))
	      == FAIL)
	    {
	      inst.error = gettext (reg_expected_msgs[regtype]);
	      return FAIL;
	    }

	  if (high_range >= max_regs)
	    {
	      first_error (_("register out of range in list"));
	      return FAIL;
	    }

	  if (regtype == REG_TYPE_NQ)
	    high_range = high_range + 1;

	  if (high_range <= new_base)
	    {
	      inst.error = _("register range not in ascending order");
	      return FAIL;
	    }

	  for (new_base += addregs; new_base <= high_range; new_base += addregs)
	    {
	      if (mask & (setmask << new_base))
		{
		  inst.error = _("invalid register list");
		  return FAIL;
		}

	      mask |= setmask << new_base;
	      count += addregs;
	    }
	}
    }
  while (skip_past_comma (&str) != FAIL);

  str++;

  /* Sanity check -- should have raised a parse error above.  */
  if ((!vpr_seen && count == 0) || count > max_regs)
    abort ();

  *pbase = base_reg;

  if (expect_vpr && !vpr_seen)
    {
      first_error (_("VPR expected last"));
      return FAIL;
    }

  /* Final test -- the registers must be consecutive.  */
  mask >>= base_reg;
  for (i = 0; i < count; i++)
    {
      if ((mask & (1u << i)) == 0)
	{
	  inst.error = _("non-contiguous register range");
	  return FAIL;
	}
    }

  *ccp = str;

  return count;
}

/* True if two alias types are the same.  */

static bool
neon_alias_types_same (struct neon_typed_alias *a, struct neon_typed_alias *b)
{
  if (!a && !b)
    return true;

  if (!a || !b)
    return false;

  if (a->defined != b->defined)
    return false;

  if ((a->defined & NTA_HASTYPE) != 0
      && (a->eltype.type != b->eltype.type
	  || a->eltype.size != b->eltype.size))
    return false;

  if ((a->defined & NTA_HASINDEX) != 0
      && (a->index != b->index))
    return false;

  return true;
}

/* Parse element/structure lists for Neon VLD<n> and VST<n> instructions.
   The base register is put in *PBASE.
   The lane (or one of the NEON_*_LANES constants) is placed in bits [3:0] of
   the return value.
   The register stride (minus one) is put in bit 4 of the return value.
   Bits [6:5] encode the list length (minus one).
   The type of the list elements is put in *ELTYPE, if non-NULL.  */

#define NEON_LANE(X)		((X) & 0xf)
#define NEON_REG_STRIDE(X)	((((X) >> 4) & 1) + 1)
#define NEON_REGLIST_LENGTH(X)	((((X) >> 5) & 3) + 1)

static int
parse_neon_el_struct_list (char **str, unsigned *pbase,
			   int mve,
			   struct neon_type_el *eltype)
{
  char *ptr = *str;
  int base_reg = -1;
  int reg_incr = -1;
  int count = 0;
  int lane = -1;
  int leading_brace = 0;
  enum arm_reg_type rtype = REG_TYPE_NDQ;
  const char *const incr_error = mve ? _("register stride must be 1") :
    _("register stride must be 1 or 2");
  const char *const type_error = _("mismatched element/structure types in list");
  struct neon_typed_alias firsttype;
  firsttype.defined = 0;
  firsttype.eltype.type = NT_invtype;
  firsttype.eltype.size = -1;
  firsttype.index = -1;

  if (skip_past_char (&ptr, '{') == SUCCESS)
    leading_brace = 1;

  do
    {
      struct neon_typed_alias atype;
      if (mve)
	rtype = REG_TYPE_MQ;
      int getreg = parse_typed_reg_or_scalar (&ptr, rtype, &rtype, &atype);

      if (getreg == FAIL)
	{
	  first_error (_(reg_expected_msgs[rtype]));
	  return FAIL;
	}

      if (base_reg == -1)
	{
	  base_reg = getreg;
	  if (rtype == REG_TYPE_NQ)
	    {
	      reg_incr = 1;
	    }
	  firsttype = atype;
	}
      else if (reg_incr == -1)
	{
	  reg_incr = getreg - base_reg;
	  if (reg_incr < 1 || reg_incr > 2)
	    {
	      first_error (_(incr_error));
	      return FAIL;
	    }
	}
      else if (getreg != base_reg + reg_incr * count)
	{
	  first_error (_(incr_error));
	  return FAIL;
	}

      if (! neon_alias_types_same (&atype, &firsttype))
	{
	  first_error (_(type_error));
	  return FAIL;
	}

      /* Handle Dn-Dm or Qn-Qm syntax. Can only be used with non-indexed list
	 modes.  */
      if (ptr[0] == '-')
	{
	  struct neon_typed_alias htype;
	  int hireg, dregs = (rtype == REG_TYPE_NQ) ? 2 : 1;
	  if (lane == -1)
	    lane = NEON_INTERLEAVE_LANES;
	  else if (lane != NEON_INTERLEAVE_LANES)
	    {
	      first_error (_(type_error));
	      return FAIL;
	    }
	  if (reg_incr == -1)
	    reg_incr = 1;
	  else if (reg_incr != 1)
	    {
	      first_error (_("don't use Rn-Rm syntax with non-unit stride"));
	      return FAIL;
	    }
	  ptr++;
	  hireg = parse_typed_reg_or_scalar (&ptr, rtype, NULL, &htype);
	  if (hireg == FAIL)
	    {
	      first_error (_(reg_expected_msgs[rtype]));
	      return FAIL;
	    }
	  if (! neon_alias_types_same (&htype, &firsttype))
	    {
	      first_error (_(type_error));
	      return FAIL;
	    }
	  count += hireg + dregs - getreg;
	  continue;
	}

      /* If we're using Q registers, we can't use [] or [n] syntax.  */
      if (rtype == REG_TYPE_NQ)
	{
	  count += 2;
	  continue;
	}

      if ((atype.defined & NTA_HASINDEX) != 0)
	{
	  if (lane == -1)
	    lane = atype.index;
	  else if (lane != atype.index)
	    {
	      first_error (_(type_error));
	      return FAIL;
	    }
	}
      else if (lane == -1)
	lane = NEON_INTERLEAVE_LANES;
      else if (lane != NEON_INTERLEAVE_LANES)
	{
	  first_error (_(type_error));
	  return FAIL;
	}
      count++;
    }
  while ((count != 1 || leading_brace) && skip_past_comma (&ptr) != FAIL);

  /* No lane set by [x]. We must be interleaving structures.  */
  if (lane == -1)
    lane = NEON_INTERLEAVE_LANES;

  /* Sanity check.  */
  if (lane == -1 || base_reg == -1 || count < 1 || (!mve && count > 4)
      || (count > 1 && reg_incr == -1))
    {
      first_error (_("error parsing element/structure list"));
      return FAIL;
    }

  if ((count > 1 || leading_brace) && skip_past_char (&ptr, '}') == FAIL)
    {
      first_error (_("expected }"));
      return FAIL;
    }

  if (reg_incr == -1)
    reg_incr = 1;

  if (eltype)
    *eltype = firsttype.eltype;

  *pbase = base_reg;
  *str = ptr;

  return lane | ((reg_incr - 1) << 4) | ((count - 1) << 5);
}

/* Parse an explicit relocation suffix on an expression.  This is
   either nothing, or a word in parentheses.  Note that if !OBJ_ELF,
   arm_reloc_hsh contains no entries, so this function can only
   succeed if there is no () after the word.  Returns -1 on error,
   BFD_RELOC_UNUSED if there wasn't any suffix.	 */

static int
parse_reloc (char **str)
{
  struct reloc_entry *r;
  char *p, *q;

  if (**str != '(')
    return BFD_RELOC_UNUSED;

  p = *str + 1;
  q = p;

  while (*q && *q != ')' && *q != ',')
    q++;
  if (*q != ')')
    return -1;

  if ((r = (struct reloc_entry *)
       str_hash_find_n (arm_reloc_hsh, p, q - p)) == NULL)
    return -1;

  *str = q + 1;
  return r->reloc;
}

/* Directives: register aliases.  */

static struct reg_entry *
insert_reg_alias (char *str, unsigned number, int type)
{
  struct reg_entry *new_reg;
  const char *name;

  if ((new_reg = (struct reg_entry *) str_hash_find (arm_reg_hsh, str)) != 0)
    {
      if (new_reg->builtin)
	as_warn (_("ignoring attempt to redefine built-in register '%s'"), str);

      /* Only warn about a redefinition if it's not defined as the
	 same register.	 */
      else if (new_reg->number != number || new_reg->type != type)
	as_warn (_("ignoring redefinition of register alias '%s'"), str);

      return NULL;
    }

  name = xstrdup (str);
  new_reg = XNEW (struct reg_entry);

  new_reg->name = name;
  new_reg->number = number;
  new_reg->type = type;
  new_reg->builtin = false;
  new_reg->neon = NULL;

  str_hash_insert (arm_reg_hsh, name, new_reg, 0);

  return new_reg;
}

static void
insert_neon_reg_alias (char *str, int number, int type,
		       struct neon_typed_alias *atype)
{
  struct reg_entry *reg = insert_reg_alias (str, number, type);

  if (!reg)
    {
      first_error (_("attempt to redefine typed alias"));
      return;
    }

  if (atype)
    {
      reg->neon = XNEW (struct neon_typed_alias);
      *reg->neon = *atype;
    }
}

/* Look for the .req directive.	 This is of the form:

	new_register_name .req existing_register_name

   If we find one, or if it looks sufficiently like one that we want to
   handle any error here, return TRUE.  Otherwise return FALSE.  */

static bool
create_register_alias (char * newname, char *p)
{
  struct reg_entry *old;
  char *oldname, *nbuf;
  size_t nlen;

  /* The input scrubber ensures that whitespace after the mnemonic is
     collapsed to single spaces.  */
  oldname = p;
  if (!startswith (oldname, " .req "))
    return false;

  oldname += 6;
  if (*oldname == '\0')
    return false;

  old = (struct reg_entry *) str_hash_find (arm_reg_hsh, oldname);
  if (!old)
    {
      as_warn (_("unknown register '%s' -- .req ignored"), oldname);
      return true;
    }

  /* If TC_CASE_SENSITIVE is defined, then newname already points to
     the desired alias name, and p points to its end.  If not, then
     the desired alias name is in the global original_case_string.  */
#ifdef TC_CASE_SENSITIVE
  nlen = p - newname;
#else
  newname = original_case_string;
  nlen = strlen (newname);
#endif

  nbuf = xmemdup0 (newname, nlen);

  /* Create aliases under the new name as stated; an all-lowercase
     version of the new name; and an all-uppercase version of the new
     name.  */
  if (insert_reg_alias (nbuf, old->number, old->type) != NULL)
    {
      for (p = nbuf; *p; p++)
	*p = TOUPPER (*p);

      if (strncmp (nbuf, newname, nlen))
	{
	  /* If this attempt to create an additional alias fails, do not bother
	     trying to create the all-lower case alias.  We will fail and issue
	     a second, duplicate error message.  This situation arises when the
	     programmer does something like:
	       foo .req r0
	       Foo .req r1
	     The second .req creates the "Foo" alias but then fails to create
	     the artificial FOO alias because it has already been created by the
	     first .req.  */
	  if (insert_reg_alias (nbuf, old->number, old->type) == NULL)
	    {
	      free (nbuf);
	      return true;
	    }
	}

      for (p = nbuf; *p; p++)
	*p = TOLOWER (*p);

      if (strncmp (nbuf, newname, nlen))
	insert_reg_alias (nbuf, old->number, old->type);
    }

  free (nbuf);
  return true;
}

/* Create a Neon typed/indexed register alias using directives, e.g.:
     X .dn d5.s32[1]
     Y .qn 6.s16
     Z .dn d7
     T .dn Z[0]
   These typed registers can be used instead of the types specified after the
   Neon mnemonic, so long as all operands given have types. Types can also be
   specified directly, e.g.:
     vadd d0.s32, d1.s32, d2.s32  */

static bool
create_neon_reg_alias (char *newname, char *p)
{
  enum arm_reg_type basetype;
  struct reg_entry *basereg;
  struct reg_entry mybasereg;
  struct neon_type ntype;
  struct neon_typed_alias typeinfo;
  char *namebuf, *nameend ATTRIBUTE_UNUSED;
  int namelen;

  typeinfo.defined = 0;
  typeinfo.eltype.type = NT_invtype;
  typeinfo.eltype.size = -1;
  typeinfo.index = -1;

  nameend = p;

  if (startswith (p, " .dn "))
    basetype = REG_TYPE_VFD;
  else if (startswith (p, " .qn "))
    basetype = REG_TYPE_NQ;
  else
    return false;

  p += 5;

  if (*p == '\0')
    return false;

  basereg = arm_reg_parse_multi (&p);

  if (basereg && basereg->type != basetype)
    {
      as_bad (_("bad type for register"));
      return false;
    }

  if (basereg == NULL)
    {
      expressionS exp;
      /* Try parsing as an integer.  */
      my_get_expression (&exp, &p, GE_NO_PREFIX);
      if (exp.X_op != O_constant)
	{
	  as_bad (_("expression must be constant"));
	  return false;
	}
      basereg = &mybasereg;
      basereg->number = (basetype == REG_TYPE_NQ) ? exp.X_add_number * 2
						  : exp.X_add_number;
      basereg->neon = 0;
    }

  if (basereg->neon)
    typeinfo = *basereg->neon;

  if (parse_neon_type (&ntype, &p) == SUCCESS)
    {
      /* We got a type.  */
      if (typeinfo.defined & NTA_HASTYPE)
	{
	  as_bad (_("can't redefine the type of a register alias"));
	  return false;
	}

      typeinfo.defined |= NTA_HASTYPE;
      if (ntype.elems != 1)
	{
	  as_bad (_("you must specify a single type only"));
	  return false;
	}
      typeinfo.eltype = ntype.el[0];
    }

  if (skip_past_char (&p, '[') == SUCCESS)
    {
      expressionS exp;
      /* We got a scalar index.  */

      if (typeinfo.defined & NTA_HASINDEX)
	{
	  as_bad (_("can't redefine the index of a scalar alias"));
	  return false;
	}

      my_get_expression (&exp, &p, GE_NO_PREFIX);

      if (exp.X_op != O_constant)
	{
	  as_bad (_("scalar index must be constant"));
	  return false;
	}

      typeinfo.defined |= NTA_HASINDEX;
      typeinfo.index = exp.X_add_number;

      if (skip_past_char (&p, ']') == FAIL)
	{
	  as_bad (_("expecting ]"));
	  return false;
	}
    }

  /* If TC_CASE_SENSITIVE is defined, then newname already points to
     the desired alias name, and p points to its end.  If not, then
     the desired alias name is in the global original_case_string.  */
#ifdef TC_CASE_SENSITIVE
  namelen = nameend - newname;
#else
  newname = original_case_string;
  namelen = strlen (newname);
#endif

  namebuf = xmemdup0 (newname, namelen);

  insert_neon_reg_alias (namebuf, basereg->number, basetype,
			 typeinfo.defined != 0 ? &typeinfo : NULL);

  /* Insert name in all uppercase.  */
  for (p = namebuf; *p; p++)
    *p = TOUPPER (*p);

  if (strncmp (namebuf, newname, namelen))
    insert_neon_reg_alias (namebuf, basereg->number, basetype,
			   typeinfo.defined != 0 ? &typeinfo : NULL);

  /* Insert name in all lowercase.  */
  for (p = namebuf; *p; p++)
    *p = TOLOWER (*p);

  if (strncmp (namebuf, newname, namelen))
    insert_neon_reg_alias (namebuf, basereg->number, basetype,
			   typeinfo.defined != 0 ? &typeinfo : NULL);

  free (namebuf);
  return true;
}

/* Should never be called, as .req goes between the alias and the
   register name, not at the beginning of the line.  */

static void
s_req (int a ATTRIBUTE_UNUSED)
{
  as_bad (_("invalid syntax for .req directive"));
}

static void
s_dn (int a ATTRIBUTE_UNUSED)
{
  as_bad (_("invalid syntax for .dn directive"));
}

static void
s_qn (int a ATTRIBUTE_UNUSED)
{
  as_bad (_("invalid syntax for .qn directive"));
}

/* The .unreq directive deletes an alias which was previously defined
   by .req.  For example:

       my_alias .req r11
       .unreq my_alias	  */

static void
s_unreq (int a ATTRIBUTE_UNUSED)
{
  char * name;
  char saved_char;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (!*name)
    as_bad (_("invalid syntax for .unreq directive"));
  else
    {
      struct reg_entry *reg
	= (struct reg_entry *) str_hash_find (arm_reg_hsh, name);

      if (!reg)
	as_bad (_("unknown register alias '%s'"), name);
      else if (reg->builtin)
	as_warn (_("ignoring attempt to use .unreq on fixed register name: '%s'"),
		 name);
      else
	{
	  char * p;
	  char * nbuf;

	  str_hash_delete (arm_reg_hsh, name);
	  free ((char *) reg->name);
	  free (reg->neon);
	  free (reg);

	  /* Also locate the all upper case and all lower case versions.
	     Do not complain if we cannot find one or the other as it
	     was probably deleted above.  */

	  nbuf = strdup (name);
	  for (p = nbuf; *p; p++)
	    *p = TOUPPER (*p);
	  reg = (struct reg_entry *) str_hash_find (arm_reg_hsh, nbuf);
	  if (reg)
	    {
	      str_hash_delete (arm_reg_hsh, nbuf);
	      free ((char *) reg->name);
	      free (reg->neon);
	      free (reg);
	    }

	  for (p = nbuf; *p; p++)
	    *p = TOLOWER (*p);
	  reg = (struct reg_entry *) str_hash_find (arm_reg_hsh, nbuf);
	  if (reg)
	    {
	      str_hash_delete (arm_reg_hsh, nbuf);
	      free ((char *) reg->name);
	      free (reg->neon);
	      free (reg);
	    }

	  free (nbuf);
	}
    }

  *input_line_pointer = saved_char;
  demand_empty_rest_of_line ();
}

/* Directives: Instruction set selection.  */

#ifdef OBJ_ELF
/* This code is to handle mapping symbols as defined in the ARM ELF spec.
   (See "Mapping symbols", section 4.5.5, ARM AAELF version 1.0).
   Note that previously, $a and $t has type STT_FUNC (BSF_OBJECT flag),
   and $d has type STT_OBJECT (BSF_OBJECT flag). Now all three are untyped.  */

/* Create a new mapping symbol for the transition to STATE.  */

static void
make_mapping_symbol (enum mstate state, valueT value, fragS *frag)
{
  symbolS * symbolP;
  const char * symname;
  int type;

  switch (state)
    {
    case MAP_DATA:
      symname = "$d";
      type = BSF_NO_FLAGS;
      break;
    case MAP_ARM:
      symname = "$a";
      type = BSF_NO_FLAGS;
      break;
    case MAP_THUMB:
      symname = "$t";
      type = BSF_NO_FLAGS;
      break;
    default:
      abort ();
    }

  symbolP = symbol_new (symname, now_seg, frag, value);
  symbol_get_bfdsym (symbolP)->flags |= type | BSF_LOCAL;

  switch (state)
    {
    case MAP_ARM:
      THUMB_SET_FUNC (symbolP, 0);
      ARM_SET_THUMB (symbolP, 0);
      ARM_SET_INTERWORK (symbolP, support_interwork);
      break;

    case MAP_THUMB:
      THUMB_SET_FUNC (symbolP, 1);
      ARM_SET_THUMB (symbolP, 1);
      ARM_SET_INTERWORK (symbolP, support_interwork);
      break;

    case MAP_DATA:
    default:
      break;
    }

  /* Save the mapping symbols for future reference.  Also check that
     we do not place two mapping symbols at the same offset within a
     frag.  We'll handle overlap between frags in
     check_mapping_symbols.

     If .fill or other data filling directive generates zero sized data,
     the mapping symbol for the following code will have the same value
     as the one generated for the data filling directive.  In this case,
     we replace the old symbol with the new one at the same address.  */
  if (value == 0)
    {
      if (frag->tc_frag_data.first_map != NULL)
	{
	  know (S_GET_VALUE (frag->tc_frag_data.first_map) == 0);
	  symbol_remove (frag->tc_frag_data.first_map, &symbol_rootP, &symbol_lastP);
	}
      frag->tc_frag_data.first_map = symbolP;
    }
  if (frag->tc_frag_data.last_map != NULL)
    {
      know (S_GET_VALUE (frag->tc_frag_data.last_map) <= S_GET_VALUE (symbolP));
      if (S_GET_VALUE (frag->tc_frag_data.last_map) == S_GET_VALUE (symbolP))
	symbol_remove (frag->tc_frag_data.last_map, &symbol_rootP, &symbol_lastP);
    }
  frag->tc_frag_data.last_map = symbolP;
}

/* We must sometimes convert a region marked as code to data during
   code alignment, if an odd number of bytes have to be padded.  The
   code mapping symbol is pushed to an aligned address.  */

static void
insert_data_mapping_symbol (enum mstate state,
			    valueT value, fragS *frag, offsetT bytes)
{
  /* If there was already a mapping symbol, remove it.  */
  if (frag->tc_frag_data.last_map != NULL
      && S_GET_VALUE (frag->tc_frag_data.last_map) == frag->fr_address + value)
    {
      symbolS *symp = frag->tc_frag_data.last_map;

      if (value == 0)
	{
	  know (frag->tc_frag_data.first_map == symp);
	  frag->tc_frag_data.first_map = NULL;
	}
      frag->tc_frag_data.last_map = NULL;
      symbol_remove (symp, &symbol_rootP, &symbol_lastP);
    }

  make_mapping_symbol (MAP_DATA, value, frag);
  make_mapping_symbol (state, value + bytes, frag);
}

static void mapping_state_2 (enum mstate state, int max_chars);

/* Set the mapping state to STATE.  Only call this when about to
   emit some STATE bytes to the file.  */

#define TRANSITION(from, to) (mapstate == (from) && state == (to))
void
mapping_state (enum mstate state)
{
  enum mstate mapstate = seg_info (now_seg)->tc_segment_info_data.mapstate;

  if (mapstate == state)
    /* The mapping symbol has already been emitted.
       There is nothing else to do.  */
    return;

  if (state == MAP_ARM || state == MAP_THUMB)
    /*  PR gas/12931
	All ARM instructions require 4-byte alignment.
	(Almost) all Thumb instructions require 2-byte alignment.

	When emitting instructions into any section, mark the section
	appropriately.

	Some Thumb instructions are alignment-sensitive modulo 4 bytes,
	but themselves require 2-byte alignment; this applies to some
	PC- relative forms.  However, these cases will involve implicit
	literal pool generation or an explicit .align >=2, both of
	which will cause the section to me marked with sufficient
	alignment.  Thus, we don't handle those cases here.  */
    record_alignment (now_seg, state == MAP_ARM ? 2 : 1);

  if (TRANSITION (MAP_UNDEFINED, MAP_DATA))
    /* This case will be evaluated later.  */
    return;

  mapping_state_2 (state, 0);
}

/* Same as mapping_state, but MAX_CHARS bytes have already been
   allocated.  Put the mapping symbol that far back.  */

static void
mapping_state_2 (enum mstate state, int max_chars)
{
  enum mstate mapstate = seg_info (now_seg)->tc_segment_info_data.mapstate;

  if (!SEG_NORMAL (now_seg))
    return;

  if (mapstate == state)
    /* The mapping symbol has already been emitted.
       There is nothing else to do.  */
    return;

  if (TRANSITION (MAP_UNDEFINED, MAP_ARM)
	  || TRANSITION (MAP_UNDEFINED, MAP_THUMB))
    {
      struct frag * const frag_first = seg_info (now_seg)->frchainP->frch_root;
      const int add_symbol = (frag_now != frag_first) || (frag_now_fix () > 0);

      if (add_symbol)
	make_mapping_symbol (MAP_DATA, (valueT) 0, frag_first);
    }

  seg_info (now_seg)->tc_segment_info_data.mapstate = state;
  make_mapping_symbol (state, (valueT) frag_now_fix () - max_chars, frag_now);
}
#undef TRANSITION
#else
#define mapping_state(x) ((void)0)
#define mapping_state_2(x, y) ((void)0)
#endif

/* Find the real, Thumb encoded start of a Thumb function.  */

#ifdef OBJ_COFF
static symbolS *
find_real_start (symbolS * symbolP)
{
  char *       real_start;
  const char * name = S_GET_NAME (symbolP);
  symbolS *    new_target;

  /* This definition must agree with the one in gcc/config/arm/thumb.c.	 */
#define STUB_NAME ".real_start_of"

  if (name == NULL)
    abort ();

  /* The compiler may generate BL instructions to local labels because
     it needs to perform a branch to a far away location. These labels
     do not have a corresponding ".real_start_of" label.  We check
     both for S_IS_LOCAL and for a leading dot, to give a way to bypass
     the ".real_start_of" convention for nonlocal branches.  */
  if (S_IS_LOCAL (symbolP) || name[0] == '.')
    return symbolP;

  real_start = concat (STUB_NAME, name, NULL);
  new_target = symbol_find (real_start);
  free (real_start);

  if (new_target == NULL)
    {
      as_warn (_("Failed to find real start of function: %s\n"), name);
      new_target = symbolP;
    }

  return new_target;
}
#endif

static void
opcode_select (int width)
{
  switch (width)
    {
    case 16:
      if (! thumb_mode)
	{
	  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v4t))
	    as_bad (_("selected processor does not support THUMB opcodes"));

	  thumb_mode = 1;
	  /* No need to force the alignment, since we will have been
	     coming from ARM mode, which is word-aligned.  */
	  record_alignment (now_seg, 1);
	}
      break;

    case 32:
      if (thumb_mode)
	{
	  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v1))
	    as_bad (_("selected processor does not support ARM opcodes"));

	  thumb_mode = 0;

	  if (!need_pass_2)
	    frag_align (2, 0, 0);

	  record_alignment (now_seg, 1);
	}
      break;

    default:
      as_bad (_("invalid instruction size selected (%d)"), width);
    }
}

static void
s_arm (int ignore ATTRIBUTE_UNUSED)
{
  opcode_select (32);
  demand_empty_rest_of_line ();
}

static void
s_thumb (int ignore ATTRIBUTE_UNUSED)
{
  opcode_select (16);
  demand_empty_rest_of_line ();
}

static void
s_code (int unused ATTRIBUTE_UNUSED)
{
  int temp;

  temp = get_absolute_expression ();
  switch (temp)
    {
    case 16:
    case 32:
      opcode_select (temp);
      break;

    default:
      as_bad (_("invalid operand to .code directive (%d) (expecting 16 or 32)"), temp);
    }
  demand_empty_rest_of_line ();
}

static void
s_force_thumb (int ignore ATTRIBUTE_UNUSED)
{
  /* If we are not already in thumb mode go into it, EVEN if
     the target processor does not support thumb instructions.
     This is used by gcc/config/arm/lib1funcs.asm for example
     to compile interworking support functions even if the
     target processor should not support interworking.	*/
  if (! thumb_mode)
    {
      thumb_mode = 2;
      record_alignment (now_seg, 1);
    }

  demand_empty_rest_of_line ();
}

static void
s_thumb_func (int ignore ATTRIBUTE_UNUSED)
{
  s_thumb (0);  /* Will check for end-of-line.  */

  /* The following label is the name/address of the start of a Thumb function.
     We need to know this for the interworking support.	 */
  label_is_thumb_function_name = true;
}

/* Perform a .set directive, but also mark the alias as
   being a thumb function.  */

static void
s_thumb_set (int equiv)
{
  /* XXX the following is a duplicate of the code for s_set() in read.c
     We cannot just call that code as we need to get at the symbol that
     is created.  */
  char *    name;
  char	    delim;
  char *    end_name;
  symbolS * symbolP;

  /* Especial apologies for the random logic:
     This just grew, and could be parsed much more simply!
     Dean - in haste.  */
  delim	    = get_symbol_name (& name);
  end_name  = input_line_pointer;
  (void) restore_line_pointer (delim);

  if (*input_line_pointer != ',')
    {
      *end_name = 0;
      as_bad (_("expected comma after name \"%s\""), name);
      *end_name = delim;
      ignore_rest_of_line ();
      return;
    }

  input_line_pointer++;
  *end_name = 0;

  if (name[0] == '.' && name[1] == '\0')
    {
      /* XXX - this should not happen to .thumb_set.  */
      abort ();
    }

  if ((symbolP = symbol_find (name)) == NULL
      && (symbolP = md_undefined_symbol (name)) == NULL)
    {
#ifndef NO_LISTING
      /* When doing symbol listings, play games with dummy fragments living
	 outside the normal fragment chain to record the file and line info
	 for this symbol.  */
      if (listing & LISTING_SYMBOLS)
	{
	  extern struct list_info_struct * listing_tail;
	  fragS * dummy_frag = (fragS * ) xmalloc (sizeof (fragS));

	  memset (dummy_frag, 0, sizeof (fragS));
	  dummy_frag->fr_type = rs_fill;
	  dummy_frag->line = listing_tail;
	  symbolP = symbol_new (name, undefined_section, dummy_frag, 0);
	  dummy_frag->fr_symbol = symbolP;
	}
      else
#endif
	symbolP = symbol_new (name, undefined_section, &zero_address_frag, 0);

#ifdef OBJ_COFF
      /* "set" symbols are local unless otherwise specified.  */
      SF_SET_LOCAL (symbolP);
#endif /* OBJ_COFF  */
    }				/* Make a new symbol.  */

  symbol_table_insert (symbolP);

  * end_name = delim;

  if (equiv
      && S_IS_DEFINED (symbolP)
      && S_GET_SEGMENT (symbolP) != reg_section)
    as_bad (_("symbol `%s' already defined"), S_GET_NAME (symbolP));

  pseudo_set (symbolP);

  demand_empty_rest_of_line ();

  /* XXX Now we come to the Thumb specific bit of code.	 */

  THUMB_SET_FUNC (symbolP, 1);
  ARM_SET_THUMB (symbolP, 1);
#if defined OBJ_ELF || defined OBJ_COFF
  ARM_SET_INTERWORK (symbolP, support_interwork);
#endif
}

/* Directives: Mode selection.  */

/* .syntax [unified|divided] - choose the new unified syntax
   (same for Arm and Thumb encoding, modulo slight differences in what
   can be represented) or the old divergent syntax for each mode.  */
static void
s_syntax (int unused ATTRIBUTE_UNUSED)
{
  char *name, delim;

  delim = get_symbol_name (& name);

  if (!strcasecmp (name, "unified"))
    unified_syntax = true;
  else if (!strcasecmp (name, "divided"))
    unified_syntax = false;
  else
    {
      as_bad (_("unrecognized syntax mode \"%s\""), name);
      return;
    }
  (void) restore_line_pointer (delim);
  demand_empty_rest_of_line ();
}

/* Directives: sectioning and alignment.  */

static void
s_bss (int ignore ATTRIBUTE_UNUSED)
{
  /* We don't support putting frags in the BSS segment, we fake it by
     marking in_bss, then looking at s_skip for clues.	*/
  subseg_set (bss_section, 0);
  demand_empty_rest_of_line ();

#ifdef md_elf_section_change_hook
  md_elf_section_change_hook ();
#endif
}

static void
s_even (int ignore ATTRIBUTE_UNUSED)
{
  /* Never make frag if expect extra pass.  */
  if (!need_pass_2)
    frag_align (1, 0, 0);

  record_alignment (now_seg, 1);

  demand_empty_rest_of_line ();
}

/* Directives: CodeComposer Studio.  */

/*  .ref  (for CodeComposer Studio syntax only).  */
static void
s_ccs_ref (int unused ATTRIBUTE_UNUSED)
{
  if (codecomposer_syntax)
    ignore_rest_of_line ();
  else
    as_bad (_(".ref pseudo-op only available with -mccs flag."));
}

/*  If name is not NULL, then it is used for marking the beginning of a
    function, whereas if it is NULL then it means the function end.  */
static void
asmfunc_debug (const char * name)
{
  static const char * last_name = NULL;

  if (name != NULL)
    {
      gas_assert (last_name == NULL);
      last_name = name;

      if (debug_type == DEBUG_STABS)
         stabs_generate_asm_func (name, name);
    }
  else
    {
      gas_assert (last_name != NULL);

      if (debug_type == DEBUG_STABS)
        stabs_generate_asm_endfunc (last_name, last_name);

      last_name = NULL;
    }
}

static void
s_ccs_asmfunc (int unused ATTRIBUTE_UNUSED)
{
  if (codecomposer_syntax)
    {
      switch (asmfunc_state)
	{
	case OUTSIDE_ASMFUNC:
	  asmfunc_state = WAITING_ASMFUNC_NAME;
	  break;

	case WAITING_ASMFUNC_NAME:
	  as_bad (_(".asmfunc repeated."));
	  break;

	case WAITING_ENDASMFUNC:
	  as_bad (_(".asmfunc without function."));
	  break;
	}
      demand_empty_rest_of_line ();
    }
  else
    as_bad (_(".asmfunc pseudo-op only available with -mccs flag."));
}

static void
s_ccs_endasmfunc (int unused ATTRIBUTE_UNUSED)
{
  if (codecomposer_syntax)
    {
      switch (asmfunc_state)
	{
	case OUTSIDE_ASMFUNC:
	  as_bad (_(".endasmfunc without a .asmfunc."));
	  break;

	case WAITING_ASMFUNC_NAME:
	  as_bad (_(".endasmfunc without function."));
	  break;

	case WAITING_ENDASMFUNC:
	  asmfunc_state = OUTSIDE_ASMFUNC;
	  asmfunc_debug (NULL);
	  break;
	}
      demand_empty_rest_of_line ();
    }
  else
    as_bad (_(".endasmfunc pseudo-op only available with -mccs flag."));
}

static void
s_ccs_def (int name)
{
  if (codecomposer_syntax)
    s_globl (name);
  else
    as_bad (_(".def pseudo-op only available with -mccs flag."));
}

/* Directives: Literal pools.  */

static literal_pool *
find_literal_pool (void)
{
  literal_pool * pool;

  for (pool = list_of_pools; pool != NULL; pool = pool->next)
    {
      if (pool->section == now_seg
	  && pool->sub_section == now_subseg)
	break;
    }

  return pool;
}

static literal_pool *
find_or_make_literal_pool (void)
{
  /* Next literal pool ID number.  */
  static unsigned int latest_pool_num = 1;
  literal_pool *      pool;

  pool = find_literal_pool ();

  if (pool == NULL)
    {
      /* Create a new pool.  */
      pool = XNEW (literal_pool);
      if (! pool)
	return NULL;

      pool->next_free_entry = 0;
      pool->section	    = now_seg;
      pool->sub_section	    = now_subseg;
      pool->next	    = list_of_pools;
      pool->symbol	    = NULL;
      pool->alignment	    = 2;

      /* Add it to the list.  */
      list_of_pools = pool;
    }

  /* New pools, and emptied pools, will have a NULL symbol.  */
  if (pool->symbol == NULL)
    {
      pool->symbol = symbol_create (FAKE_LABEL_NAME, undefined_section,
				    &zero_address_frag, 0);
      pool->id = latest_pool_num ++;
    }

  /* Done.  */
  return pool;
}

/* Add the literal in the global 'inst'
   structure to the relevant literal pool.  */

static int
add_to_lit_pool (unsigned int nbytes)
{
#define PADDING_SLOT 0x1
#define LIT_ENTRY_SIZE_MASK 0xFF
  literal_pool * pool;
  unsigned int entry, pool_size = 0;
  bool padding_slot_p = false;
  unsigned imm1 = 0;
  unsigned imm2 = 0;

  if (nbytes == 8)
    {
      imm1 = inst.operands[1].imm;
      imm2 = (inst.operands[1].regisimm ? inst.operands[1].reg
	       : inst.relocs[0].exp.X_unsigned ? 0
	       : (int64_t) inst.operands[1].imm >> 32);
      if (target_big_endian)
	{
	  imm1 = imm2;
	  imm2 = inst.operands[1].imm;
	}
    }

  pool = find_or_make_literal_pool ();

  /* Check if this literal value is already in the pool.  */
  for (entry = 0; entry < pool->next_free_entry; entry ++)
    {
      if (nbytes == 4)
	{
	  if ((pool->literals[entry].X_op == inst.relocs[0].exp.X_op)
	      && (inst.relocs[0].exp.X_op == O_constant)
	      && (pool->literals[entry].X_add_number
		  == inst.relocs[0].exp.X_add_number)
	      && (pool->literals[entry].X_md == nbytes)
	      && (pool->literals[entry].X_unsigned
		  == inst.relocs[0].exp.X_unsigned))
	    break;

	  if ((pool->literals[entry].X_op == inst.relocs[0].exp.X_op)
	      && (inst.relocs[0].exp.X_op == O_symbol)
	      && (pool->literals[entry].X_add_number
		  == inst.relocs[0].exp.X_add_number)
	      && (pool->literals[entry].X_add_symbol
		  == inst.relocs[0].exp.X_add_symbol)
	      && (pool->literals[entry].X_op_symbol
		  == inst.relocs[0].exp.X_op_symbol)
	      && (pool->literals[entry].X_md == nbytes))
	    break;
	}
      else if ((nbytes == 8)
	       && !(pool_size & 0x7)
	       && ((entry + 1) != pool->next_free_entry)
	       && (pool->literals[entry].X_op == O_constant)
	       && (pool->literals[entry].X_add_number == (offsetT) imm1)
	       && (pool->literals[entry].X_unsigned
		   == inst.relocs[0].exp.X_unsigned)
	       && (pool->literals[entry + 1].X_op == O_constant)
	       && (pool->literals[entry + 1].X_add_number == (offsetT) imm2)
	       && (pool->literals[entry + 1].X_unsigned
		   == inst.relocs[0].exp.X_unsigned))
	break;

      padding_slot_p = ((pool->literals[entry].X_md >> 8) == PADDING_SLOT);
      if (padding_slot_p && (nbytes == 4))
	break;

      pool_size += 4;
    }

  /* Do we need to create a new entry?	*/
  if (entry == pool->next_free_entry)
    {
      if (entry >= MAX_LITERAL_POOL_SIZE)
	{
	  inst.error = _("literal pool overflow");
	  return FAIL;
	}

      if (nbytes == 8)
	{
	  /* For 8-byte entries, we align to an 8-byte boundary,
	     and split it into two 4-byte entries, because on 32-bit
	     host, 8-byte constants are treated as big num, thus
	     saved in "generic_bignum" which will be overwritten
	     by later assignments.

	     We also need to make sure there is enough space for
	     the split.

	     We also check to make sure the literal operand is a
	     constant number.  */
	  if (!(inst.relocs[0].exp.X_op == O_constant
		|| inst.relocs[0].exp.X_op == O_big))
	    {
	      inst.error = _("invalid type for literal pool");
	      return FAIL;
	    }
	  else if (pool_size & 0x7)
	    {
	      if ((entry + 2) >= MAX_LITERAL_POOL_SIZE)
		{
		  inst.error = _("literal pool overflow");
		  return FAIL;
		}

	      pool->literals[entry] = inst.relocs[0].exp;
	      pool->literals[entry].X_op = O_constant;
	      pool->literals[entry].X_add_number = 0;
	      pool->literals[entry++].X_md = (PADDING_SLOT << 8) | 4;
	      pool->next_free_entry += 1;
	      pool_size += 4;
	    }
	  else if ((entry + 1) >= MAX_LITERAL_POOL_SIZE)
	    {
	      inst.error = _("literal pool overflow");
	      return FAIL;
	    }

	  pool->literals[entry] = inst.relocs[0].exp;
	  pool->literals[entry].X_op = O_constant;
	  pool->literals[entry].X_add_number = imm1;
	  pool->literals[entry].X_unsigned = inst.relocs[0].exp.X_unsigned;
	  pool->literals[entry++].X_md = 4;
	  pool->literals[entry] = inst.relocs[0].exp;
	  pool->literals[entry].X_op = O_constant;
	  pool->literals[entry].X_add_number = imm2;
	  pool->literals[entry].X_unsigned = inst.relocs[0].exp.X_unsigned;
	  pool->literals[entry].X_md = 4;
	  pool->alignment = 3;
	  pool->next_free_entry += 1;
	}
      else
	{
	  pool->literals[entry] = inst.relocs[0].exp;
	  pool->literals[entry].X_md = 4;
	}

#ifdef OBJ_ELF
      /* PR ld/12974: Record the location of the first source line to reference
	 this entry in the literal pool.  If it turns out during linking that the
	 symbol does not exist we will be able to give an accurate line number for
	 the (first use of the) missing reference.  */
      if (debug_type == DEBUG_DWARF2)
	dwarf2_where (pool->locs + entry);
#endif
      pool->next_free_entry += 1;
    }
  else if (padding_slot_p)
    {
      pool->literals[entry] = inst.relocs[0].exp;
      pool->literals[entry].X_md = nbytes;
    }

  inst.relocs[0].exp.X_op	      = O_symbol;
  inst.relocs[0].exp.X_add_number = pool_size;
  inst.relocs[0].exp.X_add_symbol = pool->symbol;

  return SUCCESS;
}

bool
tc_start_label_without_colon (void)
{
  bool ret = true;

  if (codecomposer_syntax && asmfunc_state == WAITING_ASMFUNC_NAME)
    {
      const char *label = input_line_pointer;

      while (!is_end_of_line[(int) label[-1]])
	--label;

      if (*label == '.')
	{
	  as_bad (_("Invalid label '%s'"), label);
	  ret = false;
	}

      asmfunc_debug (label);

      asmfunc_state = WAITING_ENDASMFUNC;
    }

  return ret;
}

/* Can't use symbol_new here, so have to create a symbol and then at
   a later date assign it a value. That's what these functions do.  */

static void
symbol_locate (symbolS *    symbolP,
	       const char * name,	/* It is copied, the caller can modify.	 */
	       segT	    segment,	/* Segment identifier (SEG_<something>).  */
	       valueT	    valu,	/* Symbol value.  */
	       fragS *	    frag)	/* Associated fragment.	 */
{
  size_t name_length;
  char * preserved_copy_of_name;

  name_length = strlen (name) + 1;   /* +1 for \0.  */
  obstack_grow (&notes, name, name_length);
  preserved_copy_of_name = (char *) obstack_finish (&notes);

#ifdef tc_canonicalize_symbol_name
  preserved_copy_of_name =
    tc_canonicalize_symbol_name (preserved_copy_of_name);
#endif

  S_SET_NAME (symbolP, preserved_copy_of_name);

  S_SET_SEGMENT (symbolP, segment);
  S_SET_VALUE (symbolP, valu);
  symbol_clear_list_pointers (symbolP);

  symbol_set_frag (symbolP, frag);

  /* Link to end of symbol chain.  */
  {
    extern int symbol_table_frozen;

    if (symbol_table_frozen)
      abort ();
  }

  symbol_append (symbolP, symbol_lastP, & symbol_rootP, & symbol_lastP);

  obj_symbol_new_hook (symbolP);

#ifdef tc_symbol_new_hook
  tc_symbol_new_hook (symbolP);
#endif

#ifdef DEBUG_SYMS
  verify_symbol_chain (symbol_rootP, symbol_lastP);
#endif /* DEBUG_SYMS  */
}

static void
s_ltorg (int ignored ATTRIBUTE_UNUSED)
{
  unsigned int entry;
  literal_pool * pool;
  char sym_name[20];

  demand_empty_rest_of_line ();
  pool = find_literal_pool ();
  if (pool == NULL
      || pool->symbol == NULL
      || pool->next_free_entry == 0)
    return;

  /* Align pool as you have word accesses.
     Only make a frag if we have to.  */
  if (!need_pass_2)
    frag_align (pool->alignment, 0, 0);

  record_alignment (now_seg, 2);

#ifdef OBJ_ELF
  seg_info (now_seg)->tc_segment_info_data.mapstate = MAP_DATA;
  make_mapping_symbol (MAP_DATA, (valueT) frag_now_fix (), frag_now);
#endif
  sprintf (sym_name, "$$lit_\002%x", pool->id);

  symbol_locate (pool->symbol, sym_name, now_seg,
		 (valueT) frag_now_fix (), frag_now);
  symbol_table_insert (pool->symbol);

  ARM_SET_THUMB (pool->symbol, thumb_mode);

#if defined OBJ_COFF || defined OBJ_ELF
  ARM_SET_INTERWORK (pool->symbol, support_interwork);
#endif

  for (entry = 0; entry < pool->next_free_entry; entry ++)
    {
#ifdef OBJ_ELF
      if (debug_type == DEBUG_DWARF2)
	dwarf2_gen_line_info (frag_now_fix (), pool->locs + entry);
#endif
      /* First output the expression in the instruction to the pool.  */
      emit_expr (&(pool->literals[entry]),
		 pool->literals[entry].X_md & LIT_ENTRY_SIZE_MASK);
    }

  /* Mark the pool as empty.  */
  pool->next_free_entry = 0;
  pool->symbol = NULL;
}

#ifdef OBJ_ELF
/* Forward declarations for functions below, in the MD interface
   section.  */
static void fix_new_arm (fragS *, int, short, expressionS *, int, int);
static valueT create_unwind_entry (int);
static void start_unwind_section (const segT, int);
static void add_unwind_opcode (valueT, int);
static void flush_pending_unwind (void);

/* Directives: Data.  */

static void
s_arm_elf_cons (int nbytes)
{
  expressionS exp;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

#ifdef md_cons_align
  md_cons_align (nbytes);
#endif

  mapping_state (MAP_DATA);
  do
    {
      int reloc;
      char *base = input_line_pointer;

      expression (& exp);

      if (exp.X_op != O_symbol)
	emit_expr (&exp, (unsigned int) nbytes);
      else
	{
	  char *before_reloc = input_line_pointer;
	  reloc = parse_reloc (&input_line_pointer);
	  if (reloc == -1)
	    {
	      as_bad (_("unrecognized relocation suffix"));
	      ignore_rest_of_line ();
	      return;
	    }
	  else if (reloc == BFD_RELOC_UNUSED)
	    emit_expr (&exp, (unsigned int) nbytes);
	  else
	    {
	      reloc_howto_type *howto = (reloc_howto_type *)
		  bfd_reloc_type_lookup (stdoutput,
					 (bfd_reloc_code_real_type) reloc);
	      int size = bfd_get_reloc_size (howto);

	      if (reloc == BFD_RELOC_ARM_PLT32)
		{
		  as_bad (_("(plt) is only valid on branch targets"));
		  reloc = BFD_RELOC_UNUSED;
		  size = 0;
		}

	      if (size > nbytes)
		as_bad (ngettext ("%s relocations do not fit in %d byte",
				  "%s relocations do not fit in %d bytes",
				  nbytes),
			howto->name, nbytes);
	      else
		{
		  /* We've parsed an expression stopping at O_symbol.
		     But there may be more expression left now that we
		     have parsed the relocation marker.  Parse it again.
		     XXX Surely there is a cleaner way to do this.  */
		  char *p = input_line_pointer;
		  int offset;
		  char *save_buf = XNEWVEC (char, input_line_pointer - base);

		  memcpy (save_buf, base, input_line_pointer - base);
		  memmove (base + (input_line_pointer - before_reloc),
			   base, before_reloc - base);

		  input_line_pointer = base + (input_line_pointer-before_reloc);
		  expression (&exp);
		  memcpy (base, save_buf, p - base);

		  offset = nbytes - size;
		  p = frag_more (nbytes);
		  memset (p, 0, nbytes);
		  fix_new_exp (frag_now, p - frag_now->fr_literal + offset,
			       size, &exp, 0, (enum bfd_reloc_code_real) reloc);
		  free (save_buf);
		}
	    }
	}
    }
  while (*input_line_pointer++ == ',');

  /* Put terminator back into stream.  */
  input_line_pointer --;
  demand_empty_rest_of_line ();
}

/* Emit an expression containing a 32-bit thumb instruction.
   Implementation based on put_thumb32_insn.  */

static void
emit_thumb32_expr (expressionS * exp)
{
  expressionS exp_high = *exp;

  exp_high.X_add_number = (unsigned long)exp_high.X_add_number >> 16;
  emit_expr (& exp_high, (unsigned int) THUMB_SIZE);
  exp->X_add_number &= 0xffff;
  emit_expr (exp, (unsigned int) THUMB_SIZE);
}

/*  Guess the instruction size based on the opcode.  */

static int
thumb_insn_size (int opcode)
{
  if ((unsigned int) opcode < 0xe800u)
    return 2;
  else if ((unsigned int) opcode >= 0xe8000000u)
    return 4;
  else
    return 0;
}

static bool
emit_insn (expressionS *exp, int nbytes)
{
  int size = 0;

  if (exp->X_op == O_constant)
    {
      size = nbytes;

      if (size == 0)
	size = thumb_insn_size (exp->X_add_number);

      if (size != 0)
	{
	  if (size == 2 && (unsigned int)exp->X_add_number > 0xffffu)
	    {
	      as_bad (_(".inst.n operand too big. "\
			"Use .inst.w instead"));
	      size = 0;
	    }
	  else
	    {
	      if (now_pred.state == AUTOMATIC_PRED_BLOCK)
		set_pred_insn_type_nonvoid (OUTSIDE_PRED_INSN, 0);
	      else
		set_pred_insn_type_nonvoid (NEUTRAL_IT_INSN, 0);

	      if (thumb_mode && (size > THUMB_SIZE) && !target_big_endian)
		emit_thumb32_expr (exp);
	      else
		emit_expr (exp, (unsigned int) size);

	      it_fsm_post_encode ();
	    }
	}
      else
	as_bad (_("cannot determine Thumb instruction size. "	\
		  "Use .inst.n/.inst.w instead"));
    }
  else
    as_bad (_("constant expression required"));

  return (size != 0);
}

/* Like s_arm_elf_cons but do not use md_cons_align and
   set the mapping state to MAP_ARM/MAP_THUMB.  */

static void
s_arm_elf_inst (int nbytes)
{
  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  /* Calling mapping_state () here will not change ARM/THUMB,
     but will ensure not to be in DATA state.  */

  if (thumb_mode)
    mapping_state (MAP_THUMB);
  else
    {
      if (nbytes != 0)
	{
	  as_bad (_("width suffixes are invalid in ARM mode"));
	  ignore_rest_of_line ();
	  return;
	}

      nbytes = 4;

      mapping_state (MAP_ARM);
    }

  dwarf2_emit_insn (0);

  do
    {
      expressionS exp;

      expression (& exp);

      if (! emit_insn (& exp, nbytes))
	{
	  ignore_rest_of_line ();
	  return;
	}
    }
  while (*input_line_pointer++ == ',');

  /* Put terminator back into stream.  */
  input_line_pointer --;
  demand_empty_rest_of_line ();
}

/* Parse a .rel31 directive.  */

static void
s_arm_rel31 (int ignored ATTRIBUTE_UNUSED)
{
  expressionS exp;
  char *p;
  valueT highbit;

  highbit = 0;
  if (*input_line_pointer == '1')
    highbit = 0x80000000;
  else if (*input_line_pointer != '0')
    as_bad (_("expected 0 or 1"));

  input_line_pointer++;
  if (*input_line_pointer != ',')
    as_bad (_("missing comma"));
  input_line_pointer++;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_cons_align
  md_cons_align (4);
#endif

  mapping_state (MAP_DATA);

  expression (&exp);

  p = frag_more (4);
  md_number_to_chars (p, highbit, 4);
  fix_new_arm (frag_now, p - frag_now->fr_literal, 4, &exp, 1,
	       BFD_RELOC_ARM_PREL31);

  demand_empty_rest_of_line ();
}

/* Directives: AEABI stack-unwind tables.  */

/* Parse an unwind_fnstart directive.  Simply records the current location.  */

static void
s_arm_unwind_fnstart (int ignored ATTRIBUTE_UNUSED)
{
  demand_empty_rest_of_line ();
  if (unwind.proc_start)
    {
      as_bad (_("duplicate .fnstart directive"));
      return;
    }

  /* Mark the start of the function.  */
  unwind.proc_start = expr_build_dot ();

  /* Reset the rest of the unwind info.	 */
  unwind.opcode_count = 0;
  unwind.table_entry = NULL;
  unwind.personality_routine = NULL;
  unwind.personality_index = -1;
  unwind.frame_size = 0;
  unwind.fp_offset = 0;
  unwind.fp_reg = REG_SP;
  unwind.fp_used = 0;
  unwind.sp_restored = 0;
}

/* Parse a handlerdata directive.  Creates the exception handling table entry
   for the function.  */

static void
s_arm_unwind_handlerdata (int ignored ATTRIBUTE_UNUSED)
{
  demand_empty_rest_of_line ();
  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  if (unwind.table_entry)
    as_bad (_("duplicate .handlerdata directive"));

  create_unwind_entry (1);
}

/* Parse an unwind_fnend directive.  Generates the index table entry.  */

static void
s_arm_unwind_fnend (int ignored ATTRIBUTE_UNUSED)
{
  long where;
  char *ptr;
  valueT val;
  unsigned int marked_pr_dependency;

  demand_empty_rest_of_line ();

  if (!unwind.proc_start)
    {
      as_bad (_(".fnend directive without .fnstart"));
      return;
    }

  /* Add eh table entry.  */
  if (unwind.table_entry == NULL)
    val = create_unwind_entry (0);
  else
    val = 0;

  /* Add index table entry.  This is two words.	 */
  start_unwind_section (unwind.saved_seg, 1);
  frag_align (2, 0, 0);
  record_alignment (now_seg, 2);

  ptr = frag_more (8);
  memset (ptr, 0, 8);
  where = frag_now_fix () - 8;

  /* Self relative offset of the function start.  */
  fix_new (frag_now, where, 4, unwind.proc_start, 0, 1,
	   BFD_RELOC_ARM_PREL31);

  /* Indicate dependency on EHABI-defined personality routines to the
     linker, if it hasn't been done already.  */
  marked_pr_dependency
    = seg_info (now_seg)->tc_segment_info_data.marked_pr_dependency;
  if (unwind.personality_index >= 0 && unwind.personality_index < 3
      && !(marked_pr_dependency & (1 << unwind.personality_index)))
    {
      static const char *const name[] =
	{
	  "__aeabi_unwind_cpp_pr0",
	  "__aeabi_unwind_cpp_pr1",
	  "__aeabi_unwind_cpp_pr2"
	};
      symbolS *pr = symbol_find_or_make (name[unwind.personality_index]);
      fix_new (frag_now, where, 0, pr, 0, 1, BFD_RELOC_NONE);
      seg_info (now_seg)->tc_segment_info_data.marked_pr_dependency
	|= 1 << unwind.personality_index;
    }

  if (val)
    /* Inline exception table entry.  */
    md_number_to_chars (ptr + 4, val, 4);
  else
    /* Self relative offset of the table entry.	 */
    fix_new (frag_now, where + 4, 4, unwind.table_entry, 0, 1,
	     BFD_RELOC_ARM_PREL31);

  /* Restore the original section.  */
  subseg_set (unwind.saved_seg, unwind.saved_subseg);

  unwind.proc_start = NULL;
}


/* Parse an unwind_cantunwind directive.  */

static void
s_arm_unwind_cantunwind (int ignored ATTRIBUTE_UNUSED)
{
  demand_empty_rest_of_line ();
  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  if (unwind.personality_routine || unwind.personality_index != -1)
    as_bad (_("personality routine specified for cantunwind frame"));

  unwind.personality_index = -2;
}


/* Parse a personalityindex directive.	*/

static void
s_arm_unwind_personalityindex (int ignored ATTRIBUTE_UNUSED)
{
  expressionS exp;

  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  if (unwind.personality_routine || unwind.personality_index != -1)
    as_bad (_("duplicate .personalityindex directive"));

  expression (&exp);

  if (exp.X_op != O_constant
      || exp.X_add_number < 0 || exp.X_add_number > 15)
    {
      as_bad (_("bad personality routine number"));
      ignore_rest_of_line ();
      return;
    }

  unwind.personality_index = exp.X_add_number;

  demand_empty_rest_of_line ();
}


/* Parse a personality directive.  */

static void
s_arm_unwind_personality (int ignored ATTRIBUTE_UNUSED)
{
  char *name, *p, c;

  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  if (unwind.personality_routine || unwind.personality_index != -1)
    as_bad (_("duplicate .personality directive"));

  c = get_symbol_name (& name);
  p = input_line_pointer;
  if (c == '"')
    ++ input_line_pointer;
  unwind.personality_routine = symbol_find_or_make (name);
  *p = c;
  demand_empty_rest_of_line ();
}

/* Parse a directive saving pseudo registers.  */

static void
s_arm_unwind_save_pseudo (int regno)
{
  valueT op;

  switch (regno)
    {
    case REG_RA_AUTH_CODE:
      /* Opcode for restoring RA_AUTH_CODE.  */
      op = 0xb4;
      add_unwind_opcode (op, 1);
      break;
    default:
      as_bad (_("Unknown register no. encountered: %d\n"), regno);
    }
}


/* Parse a directive saving core registers.  */

static void
s_arm_unwind_save_core (long range)
{
  valueT op;
  int n;

  /* Turn .unwind_movsp ip followed by .unwind_save {..., ip, ...}
     into .unwind_save {..., sp...}.  We aren't bothered about the value of
     ip because it is clobbered by calls.  */
  if (unwind.sp_restored && unwind.fp_reg == 12
      && (range & 0x3000) == 0x1000)
    {
      unwind.opcode_count--;
      unwind.sp_restored = 0;
      range = (range | 0x2000) & ~0x1000;
      unwind.pending_offset = 0;
    }

  /* Pop r4-r15.  */
  if (range & 0xfff0)
    {
      /* See if we can use the short opcodes.  These pop a block of up to 8
	 registers starting with r4, plus maybe r14.  */
      for (n = 0; n < 8; n++)
	{
	  /* Break at the first non-saved register.	 */
	  if ((range & (1 << (n + 4))) == 0)
	    break;
	}
      /* See if there are any other bits set.  */
      if (n == 0 || (range & (0xfff0 << n) & 0xbff0) != 0)
	{
	  /* Use the long form.  */
	  op = 0x8000 | ((range >> 4) & 0xfff);
	  add_unwind_opcode (op, 2);
	}
      else
	{
	  /* Use the short form.  */
	  if (range & 0x4000)
	    op = 0xa8; /* Pop r14.	*/
	  else
	    op = 0xa0; /* Do not pop r14.  */
	  op |= (n - 1);
	  add_unwind_opcode (op, 1);
	}
    }

  /* Pop r0-r3.	 */
  if (range & 0xf)
    {
      op = 0xb100 | (range & 0xf);
      add_unwind_opcode (op, 2);
    }

  /* Record the number of bytes pushed.	 */
  for (n = 0; n < 16; n++)
    {
      if (range & (1 << n))
	unwind.frame_size += 4;
    }
}

/* Implement correct handling of .save lists enabling the split into
sublists where necessary, while preserving correct sublist ordering.  */

static void
parse_dot_save (char **str_p, int prev_reg)
{
  long core_regs = 0;
  int reg;
  int in_range = 0;

  if (**str_p == ',')
    *str_p += 1;
  if (**str_p == '}')
    {
      *str_p += 1;
      return;
    }

  while ((reg = arm_reg_parse (str_p, REG_TYPE_RN)) != FAIL)
    {
      if (!in_range)
	{
	  if (core_regs & (1 << reg))
	    as_tsktsk (_("Warning: duplicated register (r%d) in register list"),
		       reg);
	  else if (reg <= prev_reg)
	    as_tsktsk (_("Warning: register list not in ascending order"));

	  core_regs |= (1 << reg);
	  prev_reg = reg;
	  if (skip_past_char(str_p, '-') != FAIL)
	    in_range = 1;
	  else if (skip_past_comma(str_p) == FAIL)
	    first_error (_("bad register list"));
	}
      else
	{
	  int i;
	  if (reg <= prev_reg)
	    first_error (_("bad range in register list"));
	  for (i = prev_reg + 1; i <= reg; i++)
	    {
	      if (core_regs & (1 << i))
		as_tsktsk (_("Warning: duplicated register (r%d) in register list"),
			   i);
	      else
		core_regs |= 1 << i;
	    }
	  in_range = 0;
	}
    }
  if (core_regs)
    {
      /* Higher register numbers go in higher memory addresses. When splitting a list,
	 right-most sublist should therefore be .saved first. Use recursion for this.  */
      parse_dot_save (str_p, reg);
      /* We're back from recursion, so emit .save insn for sublist.  */
      s_arm_unwind_save_core (core_regs);
      return;
    }
  /* Handle pseudo-regs, under assumption these are emitted singly.  */
  else if ((reg = arm_reg_parse (str_p, REG_TYPE_PSEUDO)) != FAIL)
    {
      /* recurse for remainder of input. Note: No assumption is made regarding which
	 register in core register set holds pseudo-register. It's not considered in
	 ordering check beyond ensuring it's not sandwiched between 2 consecutive
	 registers.  */
      parse_dot_save (str_p, prev_reg + 1);
      s_arm_unwind_save_pseudo (reg);
      return;
    }
  else
    as_bad (BAD_SYNTAX);
}

/* Parse a directive saving FPA registers.  */

static void
s_arm_unwind_save_fpa (int reg)
{
  expressionS exp;
  int num_regs;
  valueT op;

  /* Get Number of registers to transfer.  */
  if (skip_past_comma (&input_line_pointer) != FAIL)
    expression (&exp);
  else
    exp.X_op = O_illegal;

  if (exp.X_op != O_constant)
    {
      as_bad (_("expected , <constant>"));
      ignore_rest_of_line ();
      return;
    }

  num_regs = exp.X_add_number;

  if (num_regs < 1 || num_regs > 4)
    {
      as_bad (_("number of registers must be in the range [1:4]"));
      ignore_rest_of_line ();
      return;
    }

  demand_empty_rest_of_line ();

  if (reg == 4)
    {
      /* Short form.  */
      op = 0xb4 | (num_regs - 1);
      add_unwind_opcode (op, 1);
    }
  else
    {
      /* Long form.  */
      op = 0xc800 | (reg << 4) | (num_regs - 1);
      add_unwind_opcode (op, 2);
    }
  unwind.frame_size += num_regs * 12;
}


/* Parse a directive saving VFP registers for ARMv6 and above.  */

static void
s_arm_unwind_save_vfp_armv6 (void)
{
  int count;
  unsigned int start;
  valueT op;
  int num_vfpv3_regs = 0;
  int num_regs_below_16;
  bool partial_match;

  count = parse_vfp_reg_list (&input_line_pointer, &start, REGLIST_VFP_D,
			      &partial_match);
  if (count == FAIL)
    {
      as_bad (_("expected register list"));
      ignore_rest_of_line ();
      return;
    }

  demand_empty_rest_of_line ();

  /* We always generate FSTMD/FLDMD-style unwinding opcodes (rather
     than FSTMX/FLDMX-style ones).  */

  /* Generate opcode for (VFPv3) registers numbered in the range 16 .. 31.  */
  if (start >= 16)
    num_vfpv3_regs = count;
  else if (start + count > 16)
    num_vfpv3_regs = start + count - 16;

  if (num_vfpv3_regs > 0)
    {
      int start_offset = start > 16 ? start - 16 : 0;
      op = 0xc800 | (start_offset << 4) | (num_vfpv3_regs - 1);
      add_unwind_opcode (op, 2);
    }

  /* Generate opcode for registers numbered in the range 0 .. 15.  */
  num_regs_below_16 = num_vfpv3_regs > 0 ? 16 - (int) start : count;
  gas_assert (num_regs_below_16 + num_vfpv3_regs == count);
  if (num_regs_below_16 > 0)
    {
      op = 0xc900 | (start << 4) | (num_regs_below_16 - 1);
      add_unwind_opcode (op, 2);
    }

  unwind.frame_size += count * 8;
}


/* Parse a directive saving VFP registers for pre-ARMv6.  */

static void
s_arm_unwind_save_vfp (void)
{
  int count;
  unsigned int reg;
  valueT op;
  bool partial_match;

  count = parse_vfp_reg_list (&input_line_pointer, &reg, REGLIST_VFP_D,
			      &partial_match);
  if (count == FAIL)
    {
      as_bad (_("expected register list"));
      ignore_rest_of_line ();
      return;
    }

  demand_empty_rest_of_line ();

  if (reg == 8)
    {
      /* Short form.  */
      op = 0xb8 | (count - 1);
      add_unwind_opcode (op, 1);
    }
  else
    {
      /* Long form.  */
      op = 0xb300 | (reg << 4) | (count - 1);
      add_unwind_opcode (op, 2);
    }
  unwind.frame_size += count * 8 + 4;
}


/* Parse a directive saving iWMMXt data registers.  */

static void
s_arm_unwind_save_mmxwr (void)
{
  int reg;
  int hi_reg;
  int i;
  unsigned mask = 0;
  valueT op;

  if (*input_line_pointer == '{')
    input_line_pointer++;

  do
    {
      reg = arm_reg_parse (&input_line_pointer, REG_TYPE_MMXWR);

      if (reg == FAIL)
	{
	  as_bad ("%s", _(reg_expected_msgs[REG_TYPE_MMXWR]));
	  goto error;
	}

      if (mask >> reg)
	as_tsktsk (_("register list not in ascending order"));
      mask |= 1 << reg;

      if (*input_line_pointer == '-')
	{
	  input_line_pointer++;
	  hi_reg = arm_reg_parse (&input_line_pointer, REG_TYPE_MMXWR);
	  if (hi_reg == FAIL)
	    {
	      as_bad ("%s", _(reg_expected_msgs[REG_TYPE_MMXWR]));
	      goto error;
	    }
	  else if (reg >= hi_reg)
	    {
	      as_bad (_("bad register range"));
	      goto error;
	    }
	  for (; reg < hi_reg; reg++)
	    mask |= 1 << reg;
	}
    }
  while (skip_past_comma (&input_line_pointer) != FAIL);

  skip_past_char (&input_line_pointer, '}');

  demand_empty_rest_of_line ();

  /* Generate any deferred opcodes because we're going to be looking at
     the list.	*/
  flush_pending_unwind ();

  for (i = 0; i < 16; i++)
    {
      if (mask & (1 << i))
	unwind.frame_size += 8;
    }

  /* Attempt to combine with a previous opcode.	 We do this because gcc
     likes to output separate unwind directives for a single block of
     registers.	 */
  if (unwind.opcode_count > 0)
    {
      i = unwind.opcodes[unwind.opcode_count - 1];
      if ((i & 0xf8) == 0xc0)
	{
	  i &= 7;
	  /* Only merge if the blocks are contiguous.  */
	  if (i < 6)
	    {
	      if ((mask & 0xfe00) == (1 << 9))
		{
		  mask |= ((1 << (i + 11)) - 1) & 0xfc00;
		  unwind.opcode_count--;
		}
	    }
	  else if (i == 6 && unwind.opcode_count >= 2)
	    {
	      i = unwind.opcodes[unwind.opcode_count - 2];
	      reg = i >> 4;
	      i &= 0xf;

	      op = 0xffff << (reg - 1);
	      if (reg > 0
		  && ((mask & op) == (1u << (reg - 1))))
		{
		  op = (1 << (reg + i + 1)) - 1;
		  op &= ~((1 << reg) - 1);
		  mask |= op;
		  unwind.opcode_count -= 2;
		}
	    }
	}
    }

  hi_reg = 15;
  /* We want to generate opcodes in the order the registers have been
     saved, ie. descending order.  */
  for (reg = 15; reg >= -1; reg--)
    {
      /* Save registers in blocks.  */
      if (reg < 0
	  || !(mask & (1 << reg)))
	{
	  /* We found an unsaved reg.  Generate opcodes to save the
	     preceding block.	*/
	  if (reg != hi_reg)
	    {
	      if (reg == 9)
		{
		  /* Short form.  */
		  op = 0xc0 | (hi_reg - 10);
		  add_unwind_opcode (op, 1);
		}
	      else
		{
		  /* Long form.	 */
		  op = 0xc600 | ((reg + 1) << 4) | ((hi_reg - reg) - 1);
		  add_unwind_opcode (op, 2);
		}
	    }
	  hi_reg = reg - 1;
	}
    }

  return;
 error:
  ignore_rest_of_line ();
}

static void
s_arm_unwind_save_mmxwcg (void)
{
  int reg;
  int hi_reg;
  unsigned mask = 0;
  valueT op;

  if (*input_line_pointer == '{')
    input_line_pointer++;

  skip_whitespace (input_line_pointer);

  do
    {
      reg = arm_reg_parse (&input_line_pointer, REG_TYPE_MMXWCG);

      if (reg == FAIL)
	{
	  as_bad ("%s", _(reg_expected_msgs[REG_TYPE_MMXWCG]));
	  goto error;
	}

      reg -= 8;
      if (mask >> reg)
	as_tsktsk (_("register list not in ascending order"));
      mask |= 1 << reg;

      if (*input_line_pointer == '-')
	{
	  input_line_pointer++;
	  hi_reg = arm_reg_parse (&input_line_pointer, REG_TYPE_MMXWCG);
	  if (hi_reg == FAIL)
	    {
	      as_bad ("%s", _(reg_expected_msgs[REG_TYPE_MMXWCG]));
	      goto error;
	    }
	  else if (reg >= hi_reg)
	    {
	      as_bad (_("bad register range"));
	      goto error;
	    }
	  for (; reg < hi_reg; reg++)
	    mask |= 1 << reg;
	}
    }
  while (skip_past_comma (&input_line_pointer) != FAIL);

  skip_past_char (&input_line_pointer, '}');

  demand_empty_rest_of_line ();

  /* Generate any deferred opcodes because we're going to be looking at
     the list.	*/
  flush_pending_unwind ();

  for (reg = 0; reg < 16; reg++)
    {
      if (mask & (1 << reg))
	unwind.frame_size += 4;
    }
  op = 0xc700 | mask;
  add_unwind_opcode (op, 2);
  return;
 error:
  ignore_rest_of_line ();
}

/* Parse an unwind_save directive.
   If the argument is non-zero, this is a .vsave directive.  */

static void
s_arm_unwind_save (int arch_v6)
{
  char *peek;
  struct reg_entry *reg;
  bool had_brace = false;

  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  /* Figure out what sort of save we have.  */
  peek = input_line_pointer;

  if (*peek == '{')
    {
      had_brace = true;
      peek++;
    }

  reg = arm_reg_parse_multi (&peek);

  if (!reg)
    {
      as_bad (_("register expected"));
      ignore_rest_of_line ();
      return;
    }

  switch (reg->type)
    {
    case REG_TYPE_FN:
      if (had_brace)
	{
	  as_bad (_("FPA .unwind_save does not take a register list"));
	  ignore_rest_of_line ();
	  return;
	}
      input_line_pointer = peek;
      s_arm_unwind_save_fpa (reg->number);
      return;

    case REG_TYPE_PSEUDO:
    case REG_TYPE_RN:
      {
	if (had_brace)
	  input_line_pointer++;
	parse_dot_save (&input_line_pointer, -1);
	demand_empty_rest_of_line ();
	return;
      }

    case REG_TYPE_VFD:
      if (arch_v6)
	s_arm_unwind_save_vfp_armv6 ();
      else
	s_arm_unwind_save_vfp ();
      return;

    case REG_TYPE_MMXWR:
      s_arm_unwind_save_mmxwr ();
      return;

    case REG_TYPE_MMXWCG:
      s_arm_unwind_save_mmxwcg ();
      return;

    default:
      as_bad (_(".unwind_save does not support this kind of register"));
      ignore_rest_of_line ();
    }
}


/* Parse an unwind_movsp directive.  */

static void
s_arm_unwind_movsp (int ignored ATTRIBUTE_UNUSED)
{
  int reg;
  valueT op;
  int offset;

  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  reg = arm_reg_parse (&input_line_pointer, REG_TYPE_RN);
  if (reg == FAIL)
    {
      as_bad ("%s", _(reg_expected_msgs[REG_TYPE_RN]));
      ignore_rest_of_line ();
      return;
    }

  /* Optional constant.	 */
  if (skip_past_comma (&input_line_pointer) != FAIL)
    {
      if (immediate_for_directive (&offset) == FAIL)
	return;
    }
  else
    offset = 0;

  demand_empty_rest_of_line ();

  if (reg == REG_SP || reg == REG_PC)
    {
      as_bad (_("SP and PC not permitted in .unwind_movsp directive"));
      return;
    }

  if (unwind.fp_reg != REG_SP)
    as_bad (_("unexpected .unwind_movsp directive"));

  /* Generate opcode to restore the value.  */
  op = 0x90 | reg;
  add_unwind_opcode (op, 1);

  /* Record the information for later.	*/
  unwind.fp_reg = reg;
  unwind.fp_offset = unwind.frame_size - offset;
  unwind.sp_restored = 1;
}

/* Parse an unwind_pad directive.  */

static void
s_arm_unwind_pad (int ignored ATTRIBUTE_UNUSED)
{
  int offset;

  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  if (immediate_for_directive (&offset) == FAIL)
    return;

  if (offset & 3)
    {
      as_bad (_("stack increment must be multiple of 4"));
      ignore_rest_of_line ();
      return;
    }

  /* Don't generate any opcodes, just record the details for later.  */
  unwind.frame_size += offset;
  unwind.pending_offset += offset;

  demand_empty_rest_of_line ();
}

/* Parse an unwind_pacspval directive.  */

static void
s_arm_unwind_pacspval (int ignored ATTRIBUTE_UNUSED)
{
  valueT op;

  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  demand_empty_rest_of_line ();

  op = 0xb5;
  add_unwind_opcode (op, 1);
}

/* Parse an unwind_setfp directive.  */

static void
s_arm_unwind_setfp (int ignored ATTRIBUTE_UNUSED)
{
  int sp_reg;
  int fp_reg;
  int offset;

  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  fp_reg = arm_reg_parse (&input_line_pointer, REG_TYPE_RN);
  if (skip_past_comma (&input_line_pointer) == FAIL)
    sp_reg = FAIL;
  else
    sp_reg = arm_reg_parse (&input_line_pointer, REG_TYPE_RN);

  if (fp_reg == FAIL || sp_reg == FAIL)
    {
      as_bad (_("expected <reg>, <reg>"));
      ignore_rest_of_line ();
      return;
    }

  /* Optional constant.	 */
  if (skip_past_comma (&input_line_pointer) != FAIL)
    {
      if (immediate_for_directive (&offset) == FAIL)
	return;
    }
  else
    offset = 0;

  demand_empty_rest_of_line ();

  if (sp_reg != REG_SP && sp_reg != unwind.fp_reg)
    {
      as_bad (_("register must be either sp or set by a previous"
		"unwind_movsp directive"));
      return;
    }

  /* Don't generate any opcodes, just record the information for later.	 */
  unwind.fp_reg = fp_reg;
  unwind.fp_used = 1;
  if (sp_reg == REG_SP)
    unwind.fp_offset = unwind.frame_size - offset;
  else
    unwind.fp_offset -= offset;
}

/* Parse an unwind_raw directive.  */

static void
s_arm_unwind_raw (int ignored ATTRIBUTE_UNUSED)
{
  expressionS exp;
  /* This is an arbitrary limit.	 */
  unsigned char op[16];
  int count;

  if (!unwind.proc_start)
    as_bad (MISSING_FNSTART);

  expression (&exp);
  if (exp.X_op == O_constant
      && skip_past_comma (&input_line_pointer) != FAIL)
    {
      unwind.frame_size += exp.X_add_number;
      expression (&exp);
    }
  else
    exp.X_op = O_illegal;

  if (exp.X_op != O_constant)
    {
      as_bad (_("expected <offset>, <opcode>"));
      ignore_rest_of_line ();
      return;
    }

  count = 0;

  /* Parse the opcode.	*/
  for (;;)
    {
      if (count >= 16)
	{
	  as_bad (_("unwind opcode too long"));
	  ignore_rest_of_line ();
	}
      if (exp.X_op != O_constant || exp.X_add_number & ~0xff)
	{
	  as_bad (_("invalid unwind opcode"));
	  ignore_rest_of_line ();
	  return;
	}
      op[count++] = exp.X_add_number;

      /* Parse the next byte.  */
      if (skip_past_comma (&input_line_pointer) == FAIL)
	break;

      expression (&exp);
    }

  /* Add the opcode bytes in reverse order.  */
  while (count--)
    add_unwind_opcode (op[count], 1);

  demand_empty_rest_of_line ();
}


/* Parse a .eabi_attribute directive.  */

static void
s_arm_eabi_attribute (int ignored ATTRIBUTE_UNUSED)
{
  int tag = obj_elf_vendor_attribute (OBJ_ATTR_PROC);

  if (tag >= 0 && tag < NUM_KNOWN_OBJ_ATTRIBUTES)
    attributes_set_explicitly[tag] = 1;
}

/* Emit a tls fix for the symbol.  */

static void
s_arm_tls_descseq (int ignored ATTRIBUTE_UNUSED)
{
  char *p;
  expressionS exp;
#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_cons_align
  md_cons_align (4);
#endif

  /* Since we're just labelling the code, there's no need to define a
     mapping symbol.  */
  expression (&exp);
  p = obstack_next_free (&frchain_now->frch_obstack);
  fix_new_arm (frag_now, p - frag_now->fr_literal, 4, &exp, 0,
	       thumb_mode ? BFD_RELOC_ARM_THM_TLS_DESCSEQ
	       : BFD_RELOC_ARM_TLS_DESCSEQ);
}
#endif /* OBJ_ELF */

static void s_arm_arch (int);
static void s_arm_object_arch (int);
static void s_arm_cpu (int);
static void s_arm_fpu (int);
static void s_arm_arch_extension (int);

#ifdef TE_PE

static void
pe_directive_secrel (int dummy ATTRIBUTE_UNUSED)
{
  expressionS exp;

  do
    {
      expression (&exp);
      if (exp.X_op == O_symbol)
	exp.X_op = O_secrel;

      emit_expr (&exp, 4);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;
  demand_empty_rest_of_line ();
}
#endif /* TE_PE */

int
arm_is_largest_exponent_ok (int precision)
{
  /* precision == 1 ensures that this will only return
     true for 16 bit floats.  */
  return (precision == 1) && (fp16_format == ARM_FP16_FORMAT_ALTERNATIVE);
}

static void
set_fp16_format (int dummy ATTRIBUTE_UNUSED)
{
  char saved_char;
  char* name;
  enum fp_16bit_format new_format;

  new_format = ARM_FP16_FORMAT_DEFAULT;

  name = input_line_pointer;
  while (*input_line_pointer && !ISSPACE (*input_line_pointer))
    input_line_pointer++;

  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (strcasecmp (name, "ieee") == 0)
    new_format = ARM_FP16_FORMAT_IEEE;
  else if (strcasecmp (name, "alternative") == 0)
    new_format = ARM_FP16_FORMAT_ALTERNATIVE;
  else
    {
      as_bad (_("unrecognised float16 format \"%s\""), name);
      goto cleanup;
    }

  /* Only set fp16_format if it is still the default (aka not already
     been set yet).  */
  if (fp16_format == ARM_FP16_FORMAT_DEFAULT)
    fp16_format = new_format;
  else
    {
      if (new_format != fp16_format)
	as_warn (_("float16 format cannot be set more than once, ignoring."));
    }

 cleanup:
  *input_line_pointer = saved_char;
  ignore_rest_of_line ();
}

/* This table describes all the machine specific pseudo-ops the assembler
   has to support.  The fields are:
     pseudo-op name without dot
     function to call to execute this pseudo-op
     Integer arg to pass to the function.  */

const pseudo_typeS md_pseudo_table[] =
{
  /* Never called because '.req' does not start a line.	 */
  { "req",	   s_req,	  0 },
  /* Following two are likewise never called.  */
  { "dn",	   s_dn,          0 },
  { "qn",          s_qn,          0 },
  { "unreq",	   s_unreq,	  0 },
  { "bss",	   s_bss,	  0 },
  { "align",	   s_align_ptwo,  2 },
  { "arm",	   s_arm,	  0 },
  { "thumb",	   s_thumb,	  0 },
  { "code",	   s_code,	  0 },
  { "force_thumb", s_force_thumb, 0 },
  { "thumb_func",  s_thumb_func,  0 },
  { "thumb_set",   s_thumb_set,	  0 },
  { "even",	   s_even,	  0 },
  { "ltorg",	   s_ltorg,	  0 },
  { "pool",	   s_ltorg,	  0 },
  { "syntax",	   s_syntax,	  0 },
  { "cpu",	   s_arm_cpu,	  0 },
  { "arch",	   s_arm_arch,	  0 },
  { "object_arch", s_arm_object_arch,	0 },
  { "fpu",	   s_arm_fpu,	  0 },
  { "arch_extension", s_arm_arch_extension, 0 },
#ifdef OBJ_ELF
  { "word",	        s_arm_elf_cons, 4 },
  { "long",	        s_arm_elf_cons, 4 },
  { "inst.n",           s_arm_elf_inst, 2 },
  { "inst.w",           s_arm_elf_inst, 4 },
  { "inst",             s_arm_elf_inst, 0 },
  { "rel31",	        s_arm_rel31,	  0 },
  { "fnstart",		s_arm_unwind_fnstart,	0 },
  { "fnend",		s_arm_unwind_fnend,	0 },
  { "cantunwind",	s_arm_unwind_cantunwind, 0 },
  { "personality",	s_arm_unwind_personality, 0 },
  { "personalityindex",	s_arm_unwind_personalityindex, 0 },
  { "handlerdata",	s_arm_unwind_handlerdata, 0 },
  { "save",		s_arm_unwind_save,	0 },
  { "vsave",		s_arm_unwind_save,	1 },
  { "movsp",		s_arm_unwind_movsp,	0 },
  { "pad",		s_arm_unwind_pad,	0 },
  { "pacspval",		s_arm_unwind_pacspval,	0 },
  { "setfp",		s_arm_unwind_setfp,	0 },
  { "unwind_raw",	s_arm_unwind_raw,	0 },
  { "eabi_attribute",	s_arm_eabi_attribute,	0 },
  { "tlsdescseq",	s_arm_tls_descseq,      0 },
#else
  { "word",	   cons, 4},

  /* These are used for dwarf.  */
  {"2byte", cons, 2},
  {"4byte", cons, 4},
  {"8byte", cons, 8},
  /* These are used for dwarf2.  */
  { "file", dwarf2_directive_file, 0 },
  { "loc",  dwarf2_directive_loc,  0 },
  { "loc_mark_labels", dwarf2_directive_loc_mark_labels, 0 },
#endif
  { "extend",	   float_cons, 'x' },
  { "ldouble",	   float_cons, 'x' },
  { "packed",	   float_cons, 'p' },
  { "bfloat16",	   float_cons, 'b' },
#ifdef TE_PE
  {"secrel32", pe_directive_secrel, 0},
#endif

  /* These are for compatibility with CodeComposer Studio.  */
  {"ref",          s_ccs_ref,        0},
  {"def",          s_ccs_def,        0},
  {"asmfunc",      s_ccs_asmfunc,    0},
  {"endasmfunc",   s_ccs_endasmfunc, 0},

  {"float16", float_cons, 'h' },
  {"float16_format", set_fp16_format, 0 },

  { 0, 0, 0 }
};

/* Parser functions used exclusively in instruction operands.  */

/* Generic immediate-value read function for use in insn parsing.
   STR points to the beginning of the immediate (the leading #);
   VAL receives the value; if the value is outside [MIN, MAX]
   issue an error.  PREFIX_OPT is true if the immediate prefix is
   optional.  */

static int
parse_immediate (char **str, int *val, int min, int max,
		 bool prefix_opt)
{
  expressionS exp;

  my_get_expression (&exp, str, prefix_opt ? GE_OPT_PREFIX : GE_IMM_PREFIX);
  if (exp.X_op != O_constant)
    {
      inst.error = _("constant expression required");
      return FAIL;
    }

  if (exp.X_add_number < min || exp.X_add_number > max)
    {
      inst.error = _("immediate value out of range");
      return FAIL;
    }

  *val = exp.X_add_number;
  return SUCCESS;
}

/* Less-generic immediate-value read function with the possibility of loading a
   big (64-bit) immediate, as required by Neon VMOV, VMVN and logic immediate
   instructions. Puts the result directly in inst.operands[i].  */

static int
parse_big_immediate (char **str, int i, expressionS *in_exp,
		     bool allow_symbol_p)
{
  expressionS exp;
  expressionS *exp_p = in_exp ? in_exp : &exp;
  char *ptr = *str;

  my_get_expression (exp_p, &ptr, GE_OPT_PREFIX_BIG);

  if (exp_p->X_op == O_constant)
    {
      inst.operands[i].imm = exp_p->X_add_number & 0xffffffff;
      /* If we're on a 64-bit host, then a 64-bit number can be returned using
	 O_constant.  We have to be careful not to break compilation for
	 32-bit X_add_number, though.  */
      if ((exp_p->X_add_number & ~(offsetT)(0xffffffffU)) != 0)
	{
	  /* X >> 32 is illegal if sizeof (exp_p->X_add_number) == 4.  */
	  inst.operands[i].reg = (((exp_p->X_add_number >> 16) >> 16)
				  & 0xffffffff);
	  inst.operands[i].regisimm = 1;
	}
    }
  else if (exp_p->X_op == O_big
	   && LITTLENUM_NUMBER_OF_BITS * exp_p->X_add_number > 32)
    {
      unsigned parts = 32 / LITTLENUM_NUMBER_OF_BITS, j, idx = 0;

      /* Bignums have their least significant bits in
	 generic_bignum[0]. Make sure we put 32 bits in imm and
	 32 bits in reg,  in a (hopefully) portable way.  */
      gas_assert (parts != 0);

      /* Make sure that the number is not too big.
	 PR 11972: Bignums can now be sign-extended to the
	 size of a .octa so check that the out of range bits
	 are all zero or all one.  */
      if (LITTLENUM_NUMBER_OF_BITS * exp_p->X_add_number > 64)
	{
	  LITTLENUM_TYPE m = -1;

	  if (generic_bignum[parts * 2] != 0
	      && generic_bignum[parts * 2] != m)
	    return FAIL;

	  for (j = parts * 2 + 1; j < (unsigned) exp_p->X_add_number; j++)
	    if (generic_bignum[j] != generic_bignum[j-1])
	      return FAIL;
	}

      inst.operands[i].imm = 0;
      for (j = 0; j < parts; j++, idx++)
	inst.operands[i].imm |= ((unsigned) generic_bignum[idx]
				 << (LITTLENUM_NUMBER_OF_BITS * j));
      inst.operands[i].reg = 0;
      for (j = 0; j < parts; j++, idx++)
	inst.operands[i].reg |= ((unsigned) generic_bignum[idx]
				 << (LITTLENUM_NUMBER_OF_BITS * j));
      inst.operands[i].regisimm = 1;
    }
  else if (!(exp_p->X_op == O_symbol && allow_symbol_p))
    return FAIL;

  *str = ptr;

  return SUCCESS;
}

/* Returns the pseudo-register number of an FPA immediate constant,
   or FAIL if there isn't a valid constant here.  */

static int
parse_fpa_immediate (char ** str)
{
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  char *	 save_in;
  expressionS	 exp;
  int		 i;
  int		 j;

  /* First try and match exact strings, this is to guarantee
     that some formats will work even for cross assembly.  */

  for (i = 0; fp_const[i]; i++)
    {
      if (strncmp (*str, fp_const[i], strlen (fp_const[i])) == 0)
	{
	  char *start = *str;

	  *str += strlen (fp_const[i]);
	  if (is_end_of_line[(unsigned char) **str])
	    return i + 8;
	  *str = start;
	}
    }

  /* Just because we didn't get a match doesn't mean that the constant
     isn't valid, just that it is in a format that we don't
     automatically recognize.  Try parsing it with the standard
     expression routines.  */

  memset (words, 0, MAX_LITTLENUMS * sizeof (LITTLENUM_TYPE));

  /* Look for a raw floating point number.  */
  if ((save_in = atof_ieee (*str, 'x', words)) != NULL
      && is_end_of_line[(unsigned char) *save_in])
    {
      for (i = 0; i < NUM_FLOAT_VALS; i++)
	{
	  for (j = 0; j < MAX_LITTLENUMS; j++)
	    {
	      if (words[j] != fp_values[i][j])
		break;
	    }

	  if (j == MAX_LITTLENUMS)
	    {
	      *str = save_in;
	      return i + 8;
	    }
	}
    }

  /* Try and parse a more complex expression, this will probably fail
     unless the code uses a floating point prefix (eg "0f").  */
  save_in = input_line_pointer;
  input_line_pointer = *str;
  if (expression (&exp) == absolute_section
      && exp.X_op == O_big
      && exp.X_add_number < 0)
    {
      /* FIXME: 5 = X_PRECISION, should be #define'd where we can use it.
	 Ditto for 15.	*/
#define X_PRECISION 5
#define E_PRECISION 15L
      if (gen_to_words (words, X_PRECISION, E_PRECISION) == 0)
	{
	  for (i = 0; i < NUM_FLOAT_VALS; i++)
	    {
	      for (j = 0; j < MAX_LITTLENUMS; j++)
		{
		  if (words[j] != fp_values[i][j])
		    break;
		}

	      if (j == MAX_LITTLENUMS)
		{
		  *str = input_line_pointer;
		  input_line_pointer = save_in;
		  return i + 8;
		}
	    }
	}
    }

  *str = input_line_pointer;
  input_line_pointer = save_in;
  inst.error = _("invalid FPA immediate expression");
  return FAIL;
}

/* Returns 1 if a number has "quarter-precision" float format
   0baBbbbbbc defgh000 00000000 00000000.  */

static int
is_quarter_float (unsigned imm)
{
  int bs = (imm & 0x20000000) ? 0x3e000000 : 0x40000000;
  return (imm & 0x7ffff) == 0 && ((imm & 0x7e000000) ^ bs) == 0;
}


/* Detect the presence of a floating point or integer zero constant,
   i.e. #0.0 or #0.  */

static bool
parse_ifimm_zero (char **in)
{
  int error_code;

  if (!is_immediate_prefix (**in))
    {
      /* In unified syntax, all prefixes are optional.  */
      if (!unified_syntax)
	return false;
    }
  else
    ++*in;

  /* Accept #0x0 as a synonym for #0.  */
  if (startswith (*in, "0x"))
    {
      int val;
      if (parse_immediate (in, &val, 0, 0, true) == FAIL)
        return false;
      return true;
    }

  error_code = atof_generic (in, ".", EXP_CHARS,
                             &generic_floating_point_number);

  if (!error_code
      && generic_floating_point_number.sign == '+'
      && (generic_floating_point_number.low
          > generic_floating_point_number.leader))
    return true;

  return false;
}

/* Parse an 8-bit "quarter-precision" floating point number of the form:
   0baBbbbbbc defgh000 00000000 00000000.
   The zero and minus-zero cases need special handling, since they can't be
   encoded in the "quarter-precision" float format, but can nonetheless be
   loaded as integer constants.  */

static unsigned
parse_qfloat_immediate (char **ccp, int *immed)
{
  char *str = *ccp;
  char *fpnum;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  int found_fpchar = 0;

  skip_past_char (&str, '#');

  /* We must not accidentally parse an integer as a floating-point number. Make
     sure that the value we parse is not an integer by checking for special
     characters '.' or 'e'.
     FIXME: This is a horrible hack, but doing better is tricky because type
     information isn't in a very usable state at parse time.  */
  fpnum = str;
  skip_whitespace (fpnum);

  if (startswith (fpnum, "0x"))
    return FAIL;
  else
    {
      for (; *fpnum != '\0' && *fpnum != ' ' && *fpnum != '\n'; fpnum++)
	if (*fpnum == '.' || *fpnum == 'e' || *fpnum == 'E')
	  {
	    found_fpchar = 1;
	    break;
	  }

      if (!found_fpchar)
	return FAIL;
    }

  if ((str = atof_ieee (str, 's', words)) != NULL)
    {
      unsigned fpword = 0;
      int i;

      /* Our FP word must be 32 bits (single-precision FP).  */
      for (i = 0; i < 32 / LITTLENUM_NUMBER_OF_BITS; i++)
	{
	  fpword <<= LITTLENUM_NUMBER_OF_BITS;
	  fpword |= words[i];
	}

      if (is_quarter_float (fpword) || (fpword & 0x7fffffff) == 0)
	*immed = fpword;
      else
	return FAIL;

      *ccp = str;

      return SUCCESS;
    }

  return FAIL;
}

/* Shift operands.  */
enum shift_kind
{
  SHIFT_LSL, SHIFT_LSR, SHIFT_ASR, SHIFT_ROR, SHIFT_RRX, SHIFT_UXTW
};

struct asm_shift_name
{
  const char	  *name;
  enum shift_kind  kind;
};

/* Third argument to parse_shift.  */
enum parse_shift_mode
{
  NO_SHIFT_RESTRICT,		/* Any kind of shift is accepted.  */
  SHIFT_IMMEDIATE,		/* Shift operand must be an immediate.	*/
  SHIFT_LSL_OR_ASR_IMMEDIATE,	/* Shift must be LSL or ASR immediate.	*/
  SHIFT_ASR_IMMEDIATE,		/* Shift must be ASR immediate.	 */
  SHIFT_LSL_IMMEDIATE,		/* Shift must be LSL immediate.	 */
  SHIFT_UXTW_IMMEDIATE		/* Shift must be UXTW immediate.  */
};

/* Parse a <shift> specifier on an ARM data processing instruction.
   This has three forms:

     (LSL|LSR|ASL|ASR|ROR) Rs
     (LSL|LSR|ASL|ASR|ROR) #imm
     RRX

   Note that ASL is assimilated to LSL in the instruction encoding, and
   RRX to ROR #0 (which cannot be written as such).  */

static int
parse_shift (char **str, int i, enum parse_shift_mode mode)
{
  const struct asm_shift_name *shift_name;
  enum shift_kind shift;
  char *s = *str;
  char *p = s;
  int reg;

  for (p = *str; ISALPHA (*p); p++)
    ;

  if (p == *str)
    {
      inst.error = _("shift expression expected");
      return FAIL;
    }

  shift_name
    = (const struct asm_shift_name *) str_hash_find_n (arm_shift_hsh, *str,
						       p - *str);

  if (shift_name == NULL)
    {
      inst.error = _("shift expression expected");
      return FAIL;
    }

  shift = shift_name->kind;

  switch (mode)
    {
    case NO_SHIFT_RESTRICT:
    case SHIFT_IMMEDIATE:
      if (shift == SHIFT_UXTW)
	{
	  inst.error = _("'UXTW' not allowed here");
	  return FAIL;
	}
      break;

    case SHIFT_LSL_OR_ASR_IMMEDIATE:
      if (shift != SHIFT_LSL && shift != SHIFT_ASR)
	{
	  inst.error = _("'LSL' or 'ASR' required");
	  return FAIL;
	}
      break;

    case SHIFT_LSL_IMMEDIATE:
      if (shift != SHIFT_LSL)
	{
	  inst.error = _("'LSL' required");
	  return FAIL;
	}
      break;

    case SHIFT_ASR_IMMEDIATE:
      if (shift != SHIFT_ASR)
	{
	  inst.error = _("'ASR' required");
	  return FAIL;
	}
      break;
    case SHIFT_UXTW_IMMEDIATE:
      if (shift != SHIFT_UXTW)
	{
	  inst.error = _("'UXTW' required");
	  return FAIL;
	}
      break;

    default: abort ();
    }

  if (shift != SHIFT_RRX)
    {
      /* Whitespace can appear here if the next thing is a bare digit.	*/
      skip_whitespace (p);

      if (mode == NO_SHIFT_RESTRICT
	  && (reg = arm_reg_parse (&p, REG_TYPE_RN)) != FAIL)
	{
	  inst.operands[i].imm = reg;
	  inst.operands[i].immisreg = 1;
	}
      else if (my_get_expression (&inst.relocs[0].exp, &p, GE_IMM_PREFIX))
	return FAIL;
    }
  inst.operands[i].shift_kind = shift;
  inst.operands[i].shifted = 1;
  *str = p;
  return SUCCESS;
}

/* Parse a <shifter_operand> for an ARM data processing instruction:

      #<immediate>
      #<immediate>, <rotate>
      <Rm>
      <Rm>, <shift>

   where <shift> is defined by parse_shift above, and <rotate> is a
   multiple of 2 between 0 and 30.  Validation of immediate operands
   is deferred to md_apply_fix.  */

static int
parse_shifter_operand (char **str, int i)
{
  int value;
  expressionS exp;

  if ((value = arm_reg_parse (str, REG_TYPE_RN)) != FAIL)
    {
      inst.operands[i].reg = value;
      inst.operands[i].isreg = 1;

      /* parse_shift will override this if appropriate */
      inst.relocs[0].exp.X_op = O_constant;
      inst.relocs[0].exp.X_add_number = 0;

      if (skip_past_comma (str) == FAIL)
	return SUCCESS;

      /* Shift operation on register.  */
      return parse_shift (str, i, NO_SHIFT_RESTRICT);
    }

  if (my_get_expression (&inst.relocs[0].exp, str, GE_IMM_PREFIX))
    return FAIL;

  if (skip_past_comma (str) == SUCCESS)
    {
      /* #x, y -- ie explicit rotation by Y.  */
      if (my_get_expression (&exp, str, GE_NO_PREFIX))
	return FAIL;

      if (exp.X_op != O_constant || inst.relocs[0].exp.X_op != O_constant)
	{
	  inst.error = _("constant expression expected");
	  return FAIL;
	}

      value = exp.X_add_number;
      if (value < 0 || value > 30 || value % 2 != 0)
	{
	  inst.error = _("invalid rotation");
	  return FAIL;
	}
      if (inst.relocs[0].exp.X_add_number < 0
	  || inst.relocs[0].exp.X_add_number > 255)
	{
	  inst.error = _("invalid constant");
	  return FAIL;
	}

      /* Encode as specified.  */
      inst.operands[i].imm = inst.relocs[0].exp.X_add_number | value << 7;
      return SUCCESS;
    }

  inst.relocs[0].type = BFD_RELOC_ARM_IMMEDIATE;
  inst.relocs[0].pc_rel = 0;
  return SUCCESS;
}

/* Group relocation information.  Each entry in the table contains the
   textual name of the relocation as may appear in assembler source
   and must end with a colon.
   Along with this textual name are the relocation codes to be used if
   the corresponding instruction is an ALU instruction (ADD or SUB only),
   an LDR, an LDRS, or an LDC.  */

struct group_reloc_table_entry
{
  const char *name;
  int alu_code;
  int ldr_code;
  int ldrs_code;
  int ldc_code;
};

typedef enum
{
  /* Varieties of non-ALU group relocation.  */

  GROUP_LDR,
  GROUP_LDRS,
  GROUP_LDC,
  GROUP_MVE
} group_reloc_type;

static struct group_reloc_table_entry group_reloc_table[] =
  { /* Program counter relative: */
    { "pc_g0_nc",
      BFD_RELOC_ARM_ALU_PC_G0_NC,	/* ALU */
      0,				/* LDR */
      0,				/* LDRS */
      0 },				/* LDC */
    { "pc_g0",
      BFD_RELOC_ARM_ALU_PC_G0,		/* ALU */
      BFD_RELOC_ARM_LDR_PC_G0,		/* LDR */
      BFD_RELOC_ARM_LDRS_PC_G0,		/* LDRS */
      BFD_RELOC_ARM_LDC_PC_G0 },	/* LDC */
    { "pc_g1_nc",
      BFD_RELOC_ARM_ALU_PC_G1_NC,	/* ALU */
      0,				/* LDR */
      0,				/* LDRS */
      0 },				/* LDC */
    { "pc_g1",
      BFD_RELOC_ARM_ALU_PC_G1,		/* ALU */
      BFD_RELOC_ARM_LDR_PC_G1, 		/* LDR */
      BFD_RELOC_ARM_LDRS_PC_G1,		/* LDRS */
      BFD_RELOC_ARM_LDC_PC_G1 },	/* LDC */
    { "pc_g2",
      BFD_RELOC_ARM_ALU_PC_G2,		/* ALU */
      BFD_RELOC_ARM_LDR_PC_G2,		/* LDR */
      BFD_RELOC_ARM_LDRS_PC_G2,		/* LDRS */
      BFD_RELOC_ARM_LDC_PC_G2 },	/* LDC */
    /* Section base relative */
    { "sb_g0_nc",
      BFD_RELOC_ARM_ALU_SB_G0_NC,	/* ALU */
      0,				/* LDR */
      0,				/* LDRS */
      0 },				/* LDC */
    { "sb_g0",
      BFD_RELOC_ARM_ALU_SB_G0,		/* ALU */
      BFD_RELOC_ARM_LDR_SB_G0,		/* LDR */
      BFD_RELOC_ARM_LDRS_SB_G0,		/* LDRS */
      BFD_RELOC_ARM_LDC_SB_G0 },	/* LDC */
    { "sb_g1_nc",
      BFD_RELOC_ARM_ALU_SB_G1_NC,	/* ALU */
      0,				/* LDR */
      0,				/* LDRS */
      0 },				/* LDC */
    { "sb_g1",
      BFD_RELOC_ARM_ALU_SB_G1,		/* ALU */
      BFD_RELOC_ARM_LDR_SB_G1, 		/* LDR */
      BFD_RELOC_ARM_LDRS_SB_G1,		/* LDRS */
      BFD_RELOC_ARM_LDC_SB_G1 },	/* LDC */
    { "sb_g2",
      BFD_RELOC_ARM_ALU_SB_G2,		/* ALU */
      BFD_RELOC_ARM_LDR_SB_G2,		/* LDR */
      BFD_RELOC_ARM_LDRS_SB_G2,		/* LDRS */
      BFD_RELOC_ARM_LDC_SB_G2 },	/* LDC */
    /* Absolute thumb alu relocations.  */
    { "lower0_7",
      BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC,/* ALU.  */
      0,				/* LDR.  */
      0,				/* LDRS.  */
      0 },				/* LDC.  */
    { "lower8_15",
      BFD_RELOC_ARM_THUMB_ALU_ABS_G1_NC,/* ALU.  */
      0,				/* LDR.  */
      0,				/* LDRS.  */
      0 },				/* LDC.  */
    { "upper0_7",
      BFD_RELOC_ARM_THUMB_ALU_ABS_G2_NC,/* ALU.  */
      0,				/* LDR.  */
      0,				/* LDRS.  */
      0 },				/* LDC.  */
    { "upper8_15",
      BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC,/* ALU.  */
      0,				/* LDR.  */
      0,				/* LDRS.  */
      0 } };				/* LDC.  */

/* Given the address of a pointer pointing to the textual name of a group
   relocation as may appear in assembler source, attempt to find its details
   in group_reloc_table.  The pointer will be updated to the character after
   the trailing colon.  On failure, FAIL will be returned; SUCCESS
   otherwise.  On success, *entry will be updated to point at the relevant
   group_reloc_table entry. */

static int
find_group_reloc_table_entry (char **str, struct group_reloc_table_entry **out)
{
  unsigned int i;
  for (i = 0; i < ARRAY_SIZE (group_reloc_table); i++)
    {
      int length = strlen (group_reloc_table[i].name);

      if (strncasecmp (group_reloc_table[i].name, *str, length) == 0
	  && (*str)[length] == ':')
	{
	  *out = &group_reloc_table[i];
	  *str += (length + 1);
	  return SUCCESS;
	}
    }

  return FAIL;
}

/* Parse a <shifter_operand> for an ARM data processing instruction
   (as for parse_shifter_operand) where group relocations are allowed:

      #<immediate>
      #<immediate>, <rotate>
      #:<group_reloc>:<expression>
      <Rm>
      <Rm>, <shift>

   where <group_reloc> is one of the strings defined in group_reloc_table.
   The hashes are optional.

   Everything else is as for parse_shifter_operand.  */

static parse_operand_result
parse_shifter_operand_group_reloc (char **str, int i)
{
  /* Determine if we have the sequence of characters #: or just :
     coming next.  If we do, then we check for a group relocation.
     If we don't, punt the whole lot to parse_shifter_operand.  */

  if (((*str)[0] == '#' && (*str)[1] == ':')
      || (*str)[0] == ':')
    {
      struct group_reloc_table_entry *entry;

      if ((*str)[0] == '#')
	(*str) += 2;
      else
	(*str)++;

      /* Try to parse a group relocation.  Anything else is an error.  */
      if (find_group_reloc_table_entry (str, &entry) == FAIL)
	{
	  inst.error = _("unknown group relocation");
	  return PARSE_OPERAND_FAIL_NO_BACKTRACK;
	}

      /* We now have the group relocation table entry corresponding to
	 the name in the assembler source.  Next, we parse the expression.  */
      if (my_get_expression (&inst.relocs[0].exp, str, GE_NO_PREFIX))
	return PARSE_OPERAND_FAIL_NO_BACKTRACK;

      /* Record the relocation type (always the ALU variant here).  */
      inst.relocs[0].type = (bfd_reloc_code_real_type) entry->alu_code;
      gas_assert (inst.relocs[0].type != 0);

      return PARSE_OPERAND_SUCCESS;
    }
  else
    return parse_shifter_operand (str, i) == SUCCESS
	   ? PARSE_OPERAND_SUCCESS : PARSE_OPERAND_FAIL;

  /* Never reached.  */
}

/* Parse a Neon alignment expression.  Information is written to
   inst.operands[i].  We assume the initial ':' has been skipped.

   align	.imm = align << 8, .immisalign=1, .preind=0  */
static parse_operand_result
parse_neon_alignment (char **str, int i)
{
  char *p = *str;
  expressionS exp;

  my_get_expression (&exp, &p, GE_NO_PREFIX);

  if (exp.X_op != O_constant)
    {
      inst.error = _("alignment must be constant");
      return PARSE_OPERAND_FAIL;
    }

  inst.operands[i].imm = exp.X_add_number << 8;
  inst.operands[i].immisalign = 1;
  /* Alignments are not pre-indexes.  */
  inst.operands[i].preind = 0;

  *str = p;
  return PARSE_OPERAND_SUCCESS;
}

/* Parse all forms of an ARM address expression.  Information is written
   to inst.operands[i] and/or inst.relocs[0].

   Preindexed addressing (.preind=1):

   [Rn, #offset]       .reg=Rn .relocs[0].exp=offset
   [Rn, +/-Rm]	       .reg=Rn .imm=Rm .immisreg=1 .negative=0/1
   [Rn, +/-Rm, shift]  .reg=Rn .imm=Rm .immisreg=1 .negative=0/1
		       .shift_kind=shift .relocs[0].exp=shift_imm

   These three may have a trailing ! which causes .writeback to be set also.

   Postindexed addressing (.postind=1, .writeback=1):

   [Rn], #offset       .reg=Rn .relocs[0].exp=offset
   [Rn], +/-Rm	       .reg=Rn .imm=Rm .immisreg=1 .negative=0/1
   [Rn], +/-Rm, shift  .reg=Rn .imm=Rm .immisreg=1 .negative=0/1
		       .shift_kind=shift .relocs[0].exp=shift_imm

   Unindexed addressing (.preind=0, .postind=0):

   [Rn], {option}      .reg=Rn .imm=option .immisreg=0

   Other:

   [Rn]{!}	       shorthand for [Rn,#0]{!}
   =immediate	       .isreg=0 .relocs[0].exp=immediate
   label	       .reg=PC .relocs[0].pc_rel=1 .relocs[0].exp=label

  It is the caller's responsibility to check for addressing modes not
  supported by the instruction, and to set inst.relocs[0].type.  */

static parse_operand_result
parse_address_main (char **str, int i, int group_relocations,
		    group_reloc_type group_type)
{
  char *p = *str;
  int reg;

  if (skip_past_char (&p, '[') == FAIL)
    {
      if (group_type == GROUP_MVE
	  && (reg = arm_reg_parse (&p, REG_TYPE_RN)) != FAIL)
	{
	  /* [r0-r15] expected as argument but receiving r0-r15 without
	     [] brackets.  */
	  inst.error = BAD_SYNTAX;
	  return PARSE_OPERAND_FAIL;
	}
      else if (skip_past_char (&p, '=') == FAIL)
	{
	  /* Bare address - translate to PC-relative offset.  */
	  inst.relocs[0].pc_rel = 1;
	  inst.operands[i].reg = REG_PC;
	  inst.operands[i].isreg = 1;
	  inst.operands[i].preind = 1;

	  if (my_get_expression (&inst.relocs[0].exp, &p, GE_OPT_PREFIX_BIG))
	    return PARSE_OPERAND_FAIL;
	}
      else if (parse_big_immediate (&p, i, &inst.relocs[0].exp,
				    /*allow_symbol_p=*/true))
	return PARSE_OPERAND_FAIL;

      *str = p;
      return PARSE_OPERAND_SUCCESS;
    }

  /* PR gas/14887: Allow for whitespace after the opening bracket.  */
  skip_whitespace (p);

  if (group_type == GROUP_MVE)
    {
      enum arm_reg_type rtype = REG_TYPE_MQ;
      struct neon_type_el et;
      if ((reg = arm_typed_reg_parse (&p, rtype, &rtype, &et)) != FAIL)
	{
	  inst.operands[i].isquad = 1;
	}
      else if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) == FAIL)
	{
	  inst.error = BAD_ADDR_MODE;
	  return PARSE_OPERAND_FAIL;
	}
    }
  else if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) == FAIL)
    {
      if (group_type == GROUP_MVE)
	inst.error = BAD_ADDR_MODE;
      else
	inst.error = _(reg_expected_msgs[REG_TYPE_RN]);
      return PARSE_OPERAND_FAIL;
    }
  inst.operands[i].reg = reg;
  inst.operands[i].isreg = 1;

  if (skip_past_comma (&p) == SUCCESS)
    {
      inst.operands[i].preind = 1;

      if (*p == '+') p++;
      else if (*p == '-') p++, inst.operands[i].negative = 1;

      enum arm_reg_type rtype = REG_TYPE_MQ;
      struct neon_type_el et;
      if (group_type == GROUP_MVE
	  && (reg = arm_typed_reg_parse (&p, rtype, &rtype, &et)) != FAIL)
	{
	  inst.operands[i].immisreg = 2;
	  inst.operands[i].imm = reg;

	  if (skip_past_comma (&p) == SUCCESS)
	    {
	      if (parse_shift (&p, i, SHIFT_UXTW_IMMEDIATE) == SUCCESS)
		{
		  inst.operands[i].imm |= inst.relocs[0].exp.X_add_number << 5;
		  inst.relocs[0].exp.X_add_number = 0;
		}
	      else
		return PARSE_OPERAND_FAIL;
	    }
	}
      else if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) != FAIL)
	{
	  inst.operands[i].imm = reg;
	  inst.operands[i].immisreg = 1;

	  if (skip_past_comma (&p) == SUCCESS)
	    if (parse_shift (&p, i, SHIFT_IMMEDIATE) == FAIL)
	      return PARSE_OPERAND_FAIL;
	}
      else if (skip_past_char (&p, ':') == SUCCESS)
	{
	  /* FIXME: '@' should be used here, but it's filtered out by generic
	     code before we get to see it here. This may be subject to
	     change.  */
	  parse_operand_result result = parse_neon_alignment (&p, i);

	  if (result != PARSE_OPERAND_SUCCESS)
	    return result;
	}
      else
	{
	  if (inst.operands[i].negative)
	    {
	      inst.operands[i].negative = 0;
	      p--;
	    }

	  if (group_relocations
	      && ((*p == '#' && *(p + 1) == ':') || *p == ':'))
	    {
	      struct group_reloc_table_entry *entry;

	      /* Skip over the #: or : sequence.  */
	      if (*p == '#')
		p += 2;
	      else
		p++;

	      /* Try to parse a group relocation.  Anything else is an
		 error.  */
	      if (find_group_reloc_table_entry (&p, &entry) == FAIL)
		{
		  inst.error = _("unknown group relocation");
		  return PARSE_OPERAND_FAIL_NO_BACKTRACK;
		}

	      /* We now have the group relocation table entry corresponding to
		 the name in the assembler source.  Next, we parse the
		 expression.  */
	      if (my_get_expression (&inst.relocs[0].exp, &p, GE_NO_PREFIX))
		return PARSE_OPERAND_FAIL_NO_BACKTRACK;

	      /* Record the relocation type.  */
	      switch (group_type)
		{
		  case GROUP_LDR:
		    inst.relocs[0].type
			= (bfd_reloc_code_real_type) entry->ldr_code;
		    break;

		  case GROUP_LDRS:
		    inst.relocs[0].type
			= (bfd_reloc_code_real_type) entry->ldrs_code;
		    break;

		  case GROUP_LDC:
		    inst.relocs[0].type
			= (bfd_reloc_code_real_type) entry->ldc_code;
		    break;

		  default:
		    gas_assert (0);
		}

	      if (inst.relocs[0].type == 0)
		{
		  inst.error = _("this group relocation is not allowed on this instruction");
		  return PARSE_OPERAND_FAIL_NO_BACKTRACK;
		}
	    }
	  else
	    {
	      char *q = p;

	      if (my_get_expression (&inst.relocs[0].exp, &p, GE_IMM_PREFIX))
		return PARSE_OPERAND_FAIL;
	      /* If the offset is 0, find out if it's a +0 or -0.  */
	      if (inst.relocs[0].exp.X_op == O_constant
		  && inst.relocs[0].exp.X_add_number == 0)
		{
		  skip_whitespace (q);
		  if (*q == '#')
		    {
		      q++;
		      skip_whitespace (q);
		    }
		  if (*q == '-')
		    inst.operands[i].negative = 1;
		}
	    }
	}
    }
  else if (skip_past_char (&p, ':') == SUCCESS)
    {
      /* FIXME: '@' should be used here, but it's filtered out by generic code
	 before we get to see it here. This may be subject to change.  */
      parse_operand_result result = parse_neon_alignment (&p, i);

      if (result != PARSE_OPERAND_SUCCESS)
	return result;
    }

  if (skip_past_char (&p, ']') == FAIL)
    {
      inst.error = _("']' expected");
      return PARSE_OPERAND_FAIL;
    }

  if (skip_past_char (&p, '!') == SUCCESS)
    inst.operands[i].writeback = 1;

  else if (skip_past_comma (&p) == SUCCESS)
    {
      if (skip_past_char (&p, '{') == SUCCESS)
	{
	  /* [Rn], {expr} - unindexed, with option */
	  if (parse_immediate (&p, &inst.operands[i].imm,
			       0, 255, true) == FAIL)
	    return PARSE_OPERAND_FAIL;

	  if (skip_past_char (&p, '}') == FAIL)
	    {
	      inst.error = _("'}' expected at end of 'option' field");
	      return PARSE_OPERAND_FAIL;
	    }
	  if (inst.operands[i].preind)
	    {
	      inst.error = _("cannot combine index with option");
	      return PARSE_OPERAND_FAIL;
	    }
	  *str = p;
	  return PARSE_OPERAND_SUCCESS;
	}
      else
	{
	  inst.operands[i].postind = 1;
	  inst.operands[i].writeback = 1;

	  if (inst.operands[i].preind)
	    {
	      inst.error = _("cannot combine pre- and post-indexing");
	      return PARSE_OPERAND_FAIL;
	    }

	  if (*p == '+') p++;
	  else if (*p == '-') p++, inst.operands[i].negative = 1;

	  enum arm_reg_type rtype = REG_TYPE_MQ;
	  struct neon_type_el et;
	  if (group_type == GROUP_MVE
	      && (reg = arm_typed_reg_parse (&p, rtype, &rtype, &et)) != FAIL)
	    {
	      inst.operands[i].immisreg = 2;
	      inst.operands[i].imm = reg;
	    }
	  else if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) != FAIL)
	    {
	      /* We might be using the immediate for alignment already. If we
		 are, OR the register number into the low-order bits.  */
	      if (inst.operands[i].immisalign)
		inst.operands[i].imm |= reg;
	      else
		inst.operands[i].imm = reg;
	      inst.operands[i].immisreg = 1;

	      if (skip_past_comma (&p) == SUCCESS)
		if (parse_shift (&p, i, SHIFT_IMMEDIATE) == FAIL)
		  return PARSE_OPERAND_FAIL;
	    }
	  else
	    {
	      char *q = p;

	      if (inst.operands[i].negative)
		{
		  inst.operands[i].negative = 0;
		  p--;
		}
	      if (my_get_expression (&inst.relocs[0].exp, &p, GE_IMM_PREFIX))
		return PARSE_OPERAND_FAIL;
	      /* If the offset is 0, find out if it's a +0 or -0.  */
	      if (inst.relocs[0].exp.X_op == O_constant
		  && inst.relocs[0].exp.X_add_number == 0)
		{
		  skip_whitespace (q);
		  if (*q == '#')
		    {
		      q++;
		      skip_whitespace (q);
		    }
		  if (*q == '-')
		    inst.operands[i].negative = 1;
		}
	    }
	}
    }

  /* If at this point neither .preind nor .postind is set, we have a
     bare [Rn]{!}, which is shorthand for [Rn,#0]{!}.  */
  if (inst.operands[i].preind == 0 && inst.operands[i].postind == 0)
    {
      inst.operands[i].preind = 1;
      inst.relocs[0].exp.X_op = O_constant;
      inst.relocs[0].exp.X_add_number = 0;
    }
  *str = p;
  return PARSE_OPERAND_SUCCESS;
}

static int
parse_address (char **str, int i)
{
  return parse_address_main (str, i, 0, GROUP_LDR) == PARSE_OPERAND_SUCCESS
	 ? SUCCESS : FAIL;
}

static parse_operand_result
parse_address_group_reloc (char **str, int i, group_reloc_type type)
{
  return parse_address_main (str, i, 1, type);
}

/* Parse an operand for a MOVW or MOVT instruction.  */
static int
parse_half (char **str)
{
  char * p;

  p = *str;
  skip_past_char (&p, '#');
  if (strncasecmp (p, ":lower16:", 9) == 0)
    inst.relocs[0].type = BFD_RELOC_ARM_MOVW;
  else if (strncasecmp (p, ":upper16:", 9) == 0)
    inst.relocs[0].type = BFD_RELOC_ARM_MOVT;

  if (inst.relocs[0].type != BFD_RELOC_UNUSED)
    {
      p += 9;
      skip_whitespace (p);
    }

  if (my_get_expression (&inst.relocs[0].exp, &p, GE_NO_PREFIX))
    return FAIL;

  if (inst.relocs[0].type == BFD_RELOC_UNUSED)
    {
      if (inst.relocs[0].exp.X_op != O_constant)
	{
	  inst.error = _("constant expression expected");
	  return FAIL;
	}
      if (inst.relocs[0].exp.X_add_number < 0
	  || inst.relocs[0].exp.X_add_number > 0xffff)
	{
	  inst.error = _("immediate value out of range");
	  return FAIL;
	}
    }
  *str = p;
  return SUCCESS;
}

/* Miscellaneous. */

/* Parse a PSR flag operand.  The value returned is FAIL on syntax error,
   or a bitmask suitable to be or-ed into the ARM msr instruction.  */
static int
parse_psr (char **str, bool lhs)
{
  char *p;
  unsigned long psr_field;
  const struct asm_psr *psr;
  char *start;
  bool is_apsr = false;
  bool m_profile = ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_m);

  /* PR gas/12698:  If the user has specified -march=all then m_profile will
     be TRUE, but we want to ignore it in this case as we are building for any
     CPU type, including non-m variants.  */
  if (ARM_FEATURE_CORE_EQUAL (selected_cpu, arm_arch_any))
    m_profile = false;

  /* CPSR's and SPSR's can now be lowercase.  This is just a convenience
     feature for ease of use and backwards compatibility.  */
  p = *str;
  if (strncasecmp (p, "SPSR", 4) == 0)
    {
      if (m_profile)
	goto unsupported_psr;

      psr_field = SPSR_BIT;
    }
  else if (strncasecmp (p, "CPSR", 4) == 0)
    {
      if (m_profile)
	goto unsupported_psr;

      psr_field = 0;
    }
  else if (strncasecmp (p, "APSR", 4) == 0)
    {
      /* APSR[_<bits>] can be used as a synonym for CPSR[_<flags>] on ARMv7-A
	 and ARMv7-R architecture CPUs.  */
      is_apsr = true;
      psr_field = 0;
    }
  else if (m_profile)
    {
      start = p;
      do
	p++;
      while (ISALNUM (*p) || *p == '_');

      if (strncasecmp (start, "iapsr", 5) == 0
	  || strncasecmp (start, "eapsr", 5) == 0
	  || strncasecmp (start, "xpsr", 4) == 0
	  || strncasecmp (start, "psr", 3) == 0)
	p = start + strcspn (start, "rR") + 1;

      psr = (const struct asm_psr *) str_hash_find_n (arm_v7m_psr_hsh, start,
						      p - start);

      if (!psr)
	return FAIL;

      /* If APSR is being written, a bitfield may be specified.  Note that
	 APSR itself is handled above.  */
      if (psr->field <= 3)
	{
	  psr_field = psr->field;
	  is_apsr = true;
	  goto check_suffix;
	}

      *str = p;
      /* M-profile MSR instructions have the mask field set to "10", except
	 *PSR variants which modify APSR, which may use a different mask (and
	 have been handled already).  Do that by setting the PSR_f field
	 here.  */
      return psr->field | (lhs ? PSR_f : 0);
    }
  else
    goto unsupported_psr;

  p += 4;
 check_suffix:
  if (*p == '_')
    {
      /* A suffix follows.  */
      p++;
      start = p;

      do
	p++;
      while (ISALNUM (*p) || *p == '_');

      if (is_apsr)
	{
	  /* APSR uses a notation for bits, rather than fields.  */
	  unsigned int nzcvq_bits = 0;
	  unsigned int g_bit = 0;
	  char *bit;

	  for (bit = start; bit != p; bit++)
	    {
	      switch (TOLOWER (*bit))
		{
		case 'n':
		  nzcvq_bits |= (nzcvq_bits & 0x01) ? 0x20 : 0x01;
		  break;

		case 'z':
		  nzcvq_bits |= (nzcvq_bits & 0x02) ? 0x20 : 0x02;
		  break;

		case 'c':
		  nzcvq_bits |= (nzcvq_bits & 0x04) ? 0x20 : 0x04;
		  break;

		case 'v':
		  nzcvq_bits |= (nzcvq_bits & 0x08) ? 0x20 : 0x08;
		  break;

		case 'q':
		  nzcvq_bits |= (nzcvq_bits & 0x10) ? 0x20 : 0x10;
		  break;

		case 'g':
		  g_bit |= (g_bit & 0x1) ? 0x2 : 0x1;
		  break;

		default:
		  inst.error = _("unexpected bit specified after APSR");
		  return FAIL;
		}
	    }

	  if (nzcvq_bits == 0x1f)
	    psr_field |= PSR_f;

	  if (g_bit == 0x1)
	    {
	      if (!ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6_dsp))
		{
		  inst.error = _("selected processor does not "
				 "support DSP extension");
		  return FAIL;
		}

	      psr_field |= PSR_s;
	    }

	  if ((nzcvq_bits & 0x20) != 0
	      || (nzcvq_bits != 0x1f && nzcvq_bits != 0)
	      || (g_bit & 0x2) != 0)
	    {
	      inst.error = _("bad bitmask specified after APSR");
	      return FAIL;
	    }
	}
      else
	{
	  psr = (const struct asm_psr *) str_hash_find_n (arm_psr_hsh, start,
							  p - start);
	  if (!psr)
	    goto error;

	  psr_field |= psr->field;
	}
    }
  else
    {
      if (ISALNUM (*p))
	goto error;    /* Garbage after "[CS]PSR".  */

      /* Unadorned APSR is equivalent to APSR_nzcvq/CPSR_f (for writes).  This
	 is deprecated, but allow it anyway.  */
      if (is_apsr && lhs)
	{
	  psr_field |= PSR_f;
	  as_tsktsk (_("writing to APSR without specifying a bitmask is "
		       "deprecated"));
	}
      else if (!m_profile)
	/* These bits are never right for M-profile devices: don't set them
	   (only code paths which read/write APSR reach here).  */
	psr_field |= (PSR_c | PSR_f);
    }
  *str = p;
  return psr_field;

 unsupported_psr:
  inst.error = _("selected processor does not support requested special "
		 "purpose register");
  return FAIL;

 error:
  inst.error = _("flag for {c}psr instruction expected");
  return FAIL;
}

static int
parse_sys_vldr_vstr (char **str)
{
  unsigned i;
  int val = FAIL;
  struct {
    const char *name;
    int regl;
    int regh;
  } sysregs[] = {
    {"FPSCR",		0x1, 0x0},
    {"FPSCR_nzcvqc",	0x2, 0x0},
    {"VPR",		0x4, 0x1},
    {"P0",		0x5, 0x1},
    {"FPCXTNS",		0x6, 0x1},
    {"FPCXT_NS",	0x6, 0x1},
    {"fpcxtns",		0x6, 0x1},
    {"fpcxt_ns",	0x6, 0x1},
    {"FPCXTS",		0x7, 0x1},
    {"FPCXT_S",		0x7, 0x1},
    {"fpcxts",		0x7, 0x1},
    {"fpcxt_s",		0x7, 0x1}
  };
  char *op_end = strchr (*str, ',');
  size_t op_strlen = op_end - *str;

  for (i = 0; i < sizeof (sysregs) / sizeof (sysregs[0]); i++)
    {
      if (!strncmp (*str, sysregs[i].name, op_strlen))
	{
	  val = sysregs[i].regl | (sysregs[i].regh << 3);
	  *str = op_end;
	  break;
	}
    }

  return val;
}

/* Parse the flags argument to CPSI[ED].  Returns FAIL on error, or a
   value suitable for splatting into the AIF field of the instruction.	*/

static int
parse_cps_flags (char **str)
{
  int val = 0;
  int saw_a_flag = 0;
  char *s = *str;

  for (;;)
    switch (*s++)
      {
      case '\0': case ',':
	goto done;

      case 'a': case 'A': saw_a_flag = 1; val |= 0x4; break;
      case 'i': case 'I': saw_a_flag = 1; val |= 0x2; break;
      case 'f': case 'F': saw_a_flag = 1; val |= 0x1; break;

      default:
	inst.error = _("unrecognized CPS flag");
	return FAIL;
      }

 done:
  if (saw_a_flag == 0)
    {
      inst.error = _("missing CPS flags");
      return FAIL;
    }

  *str = s - 1;
  return val;
}

/* Parse an endian specifier ("BE" or "LE", case insensitive);
   returns 0 for big-endian, 1 for little-endian, FAIL for an error.  */

static int
parse_endian_specifier (char **str)
{
  int little_endian;
  char *s = *str;

  if (strncasecmp (s, "BE", 2))
    little_endian = 0;
  else if (strncasecmp (s, "LE", 2))
    little_endian = 1;
  else
    {
      inst.error = _("valid endian specifiers are be or le");
      return FAIL;
    }

  if (ISALNUM (s[2]) || s[2] == '_')
    {
      inst.error = _("valid endian specifiers are be or le");
      return FAIL;
    }

  *str = s + 2;
  return little_endian;
}

/* Parse a rotation specifier: ROR #0, #8, #16, #24.  *val receives a
   value suitable for poking into the rotate field of an sxt or sxta
   instruction, or FAIL on error.  */

static int
parse_ror (char **str)
{
  int rot;
  char *s = *str;

  if (strncasecmp (s, "ROR", 3) == 0)
    s += 3;
  else
    {
      inst.error = _("missing rotation field after comma");
      return FAIL;
    }

  if (parse_immediate (&s, &rot, 0, 24, false) == FAIL)
    return FAIL;

  switch (rot)
    {
    case  0: *str = s; return 0x0;
    case  8: *str = s; return 0x1;
    case 16: *str = s; return 0x2;
    case 24: *str = s; return 0x3;

    default:
      inst.error = _("rotation can only be 0, 8, 16, or 24");
      return FAIL;
    }
}

/* Parse a conditional code (from conds[] below).  The value returned is in the
   range 0 .. 14, or FAIL.  */
static int
parse_cond (char **str)
{
  char *q;
  const struct asm_cond *c;
  int n;
  /* Condition codes are always 2 characters, so matching up to
     3 characters is sufficient.  */
  char cond[3];

  q = *str;
  n = 0;
  while (ISALPHA (*q) && n < 3)
    {
      cond[n] = TOLOWER (*q);
      q++;
      n++;
    }

  c = (const struct asm_cond *) str_hash_find_n (arm_cond_hsh, cond, n);
  if (!c)
    {
      inst.error = _("condition required");
      return FAIL;
    }

  *str = q;
  return c->value;
}

/* Parse an option for a barrier instruction.  Returns the encoding for the
   option, or FAIL.  */
static int
parse_barrier (char **str)
{
  char *p, *q;
  const struct asm_barrier_opt *o;

  p = q = *str;
  while (ISALPHA (*q))
    q++;

  o = (const struct asm_barrier_opt *) str_hash_find_n (arm_barrier_opt_hsh, p,
							q - p);
  if (!o)
    return FAIL;

  if (!mark_feature_used (&o->arch))
    return FAIL;

  *str = q;
  return o->value;
}

/* Parse the operands of a table branch instruction.  Similar to a memory
   operand.  */
static int
parse_tb (char **str)
{
  char * p = *str;
  int reg;

  if (skip_past_char (&p, '[') == FAIL)
    {
      inst.error = _("'[' expected");
      return FAIL;
    }

  if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) == FAIL)
    {
      inst.error = _(reg_expected_msgs[REG_TYPE_RN]);
      return FAIL;
    }
  inst.operands[0].reg = reg;

  if (skip_past_comma (&p) == FAIL)
    {
      inst.error = _("',' expected");
      return FAIL;
    }

  if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) == FAIL)
    {
      inst.error = _(reg_expected_msgs[REG_TYPE_RN]);
      return FAIL;
    }
  inst.operands[0].imm = reg;

  if (skip_past_comma (&p) == SUCCESS)
    {
      if (parse_shift (&p, 0, SHIFT_LSL_IMMEDIATE) == FAIL)
	return FAIL;
      if (inst.relocs[0].exp.X_add_number != 1)
	{
	  inst.error = _("invalid shift");
	  return FAIL;
	}
      inst.operands[0].shifted = 1;
    }

  if (skip_past_char (&p, ']') == FAIL)
    {
      inst.error = _("']' expected");
      return FAIL;
    }
  *str = p;
  return SUCCESS;
}

/* Parse the operands of a Neon VMOV instruction. See do_neon_mov for more
   information on the types the operands can take and how they are encoded.
   Up to four operands may be read; this function handles setting the
   ".present" field for each read operand itself.
   Updates STR and WHICH_OPERAND if parsing is successful and returns SUCCESS,
   else returns FAIL.  */

static int
parse_neon_mov (char **str, int *which_operand)
{
  int i = *which_operand, val;
  enum arm_reg_type rtype;
  char *ptr = *str;
  struct neon_type_el optype;

   if ((val = parse_scalar (&ptr, 8, &optype, REG_TYPE_MQ)) != FAIL)
    {
      /* Cases 17 or 19.  */
      inst.operands[i].reg = val;
      inst.operands[i].isvec = 1;
      inst.operands[i].isscalar = 2;
      inst.operands[i].vectype = optype;
      inst.operands[i++].present = 1;

      if (skip_past_comma (&ptr) == FAIL)
	goto wanted_comma;

      if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) != FAIL)
	{
	  /* Case 17: VMOV<c>.<dt> <Qd[idx]>, <Rt>  */
	  inst.operands[i].reg = val;
	  inst.operands[i].isreg = 1;
	  inst.operands[i].present = 1;
	}
      else if ((val = parse_scalar (&ptr, 8, &optype, REG_TYPE_MQ)) != FAIL)
	{
	  /* Case 19: VMOV<c> <Qd[idx]>, <Qd[idx2]>, <Rt>, <Rt2>  */
	  inst.operands[i].reg = val;
	  inst.operands[i].isvec = 1;
	  inst.operands[i].isscalar = 2;
	  inst.operands[i].vectype = optype;
	  inst.operands[i++].present = 1;

	  if (skip_past_comma (&ptr) == FAIL)
	    goto wanted_comma;

	  if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) == FAIL)
	    goto wanted_arm;

	  inst.operands[i].reg = val;
	  inst.operands[i].isreg = 1;
	  inst.operands[i++].present = 1;

	  if (skip_past_comma (&ptr) == FAIL)
	    goto wanted_comma;

	  if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) == FAIL)
	    goto wanted_arm;

	  inst.operands[i].reg = val;
	  inst.operands[i].isreg = 1;
	  inst.operands[i].present = 1;
	}
      else
	{
	  first_error (_("expected ARM or MVE vector register"));
	  return FAIL;
	}
    }
   else if ((val = parse_scalar (&ptr, 8, &optype, REG_TYPE_VFD)) != FAIL)
    {
      /* Case 4: VMOV<c><q>.<size> <Dn[x]>, <Rd>.  */
      inst.operands[i].reg = val;
      inst.operands[i].isscalar = 1;
      inst.operands[i].vectype = optype;
      inst.operands[i++].present = 1;

      if (skip_past_comma (&ptr) == FAIL)
	goto wanted_comma;

      if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) == FAIL)
	goto wanted_arm;

      inst.operands[i].reg = val;
      inst.operands[i].isreg = 1;
      inst.operands[i].present = 1;
    }
  else if (((val = arm_typed_reg_parse (&ptr, REG_TYPE_NSDQ, &rtype, &optype))
	    != FAIL)
	   || ((val = arm_typed_reg_parse (&ptr, REG_TYPE_MQ, &rtype, &optype))
	       != FAIL))
    {
      /* Cases 0, 1, 2, 3, 5 (D only).  */
      if (skip_past_comma (&ptr) == FAIL)
	goto wanted_comma;

      inst.operands[i].reg = val;
      inst.operands[i].isreg = 1;
      inst.operands[i].isquad = (rtype == REG_TYPE_NQ);
      inst.operands[i].issingle = (rtype == REG_TYPE_VFS);
      inst.operands[i].isvec = 1;
      inst.operands[i].vectype = optype;
      inst.operands[i++].present = 1;

      if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) != FAIL)
	{
	  /* Case 5: VMOV<c><q> <Dm>, <Rd>, <Rn>.
	     Case 13: VMOV <Sd>, <Rm>  */
	  inst.operands[i].reg = val;
	  inst.operands[i].isreg = 1;
	  inst.operands[i].present = 1;

	  if (rtype == REG_TYPE_NQ)
	    {
	      first_error (_("can't use Neon quad register here"));
	      return FAIL;
	    }
	  else if (rtype != REG_TYPE_VFS)
	    {
	      i++;
	      if (skip_past_comma (&ptr) == FAIL)
		goto wanted_comma;
	      if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) == FAIL)
		goto wanted_arm;
	      inst.operands[i].reg = val;
	      inst.operands[i].isreg = 1;
	      inst.operands[i].present = 1;
	    }
	}
      else if (((val = arm_typed_reg_parse (&ptr, REG_TYPE_NSDQ, &rtype,
		&optype)) != FAIL)
	       || ((val = arm_typed_reg_parse (&ptr, REG_TYPE_MQ, &rtype,
		   &optype)) != FAIL))
	{
	  /* Case 0: VMOV<c><q> <Qd>, <Qm>
	     Case 1: VMOV<c><q> <Dd>, <Dm>
	     Case 8: VMOV.F32 <Sd>, <Sm>
	     Case 15: VMOV <Sd>, <Se>, <Rn>, <Rm>  */

	  inst.operands[i].reg = val;
	  inst.operands[i].isreg = 1;
	  inst.operands[i].isquad = (rtype == REG_TYPE_NQ);
	  inst.operands[i].issingle = (rtype == REG_TYPE_VFS);
	  inst.operands[i].isvec = 1;
	  inst.operands[i].vectype = optype;
	  inst.operands[i].present = 1;

	  if (skip_past_comma (&ptr) == SUCCESS)
	    {
	      /* Case 15.  */
	      i++;

	      if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) == FAIL)
		goto wanted_arm;

	      inst.operands[i].reg = val;
	      inst.operands[i].isreg = 1;
	      inst.operands[i++].present = 1;

	      if (skip_past_comma (&ptr) == FAIL)
		goto wanted_comma;

	      if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) == FAIL)
		goto wanted_arm;

	      inst.operands[i].reg = val;
	      inst.operands[i].isreg = 1;
	      inst.operands[i].present = 1;
	    }
	}
      else if (parse_qfloat_immediate (&ptr, &inst.operands[i].imm) == SUCCESS)
	  /* Case 2: VMOV<c><q>.<dt> <Qd>, #<float-imm>
	     Case 3: VMOV<c><q>.<dt> <Dd>, #<float-imm>
	     Case 10: VMOV.F32 <Sd>, #<imm>
	     Case 11: VMOV.F64 <Dd>, #<imm>  */
	inst.operands[i].immisfloat = 1;
      else if (parse_big_immediate (&ptr, i, NULL, /*allow_symbol_p=*/false)
	       == SUCCESS)
	  /* Case 2: VMOV<c><q>.<dt> <Qd>, #<imm>
	     Case 3: VMOV<c><q>.<dt> <Dd>, #<imm>  */
	;
      else
	{
	  first_error (_("expected <Rm> or <Dm> or <Qm> operand"));
	  return FAIL;
	}
    }
  else if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) != FAIL)
    {
      /* Cases 6, 7, 16, 18.  */
      inst.operands[i].reg = val;
      inst.operands[i].isreg = 1;
      inst.operands[i++].present = 1;

      if (skip_past_comma (&ptr) == FAIL)
	goto wanted_comma;

      if ((val = parse_scalar (&ptr, 8, &optype, REG_TYPE_MQ)) != FAIL)
	{
	  /* Case 18: VMOV<c>.<dt> <Rt>, <Qn[idx]>  */
	  inst.operands[i].reg = val;
	  inst.operands[i].isscalar = 2;
	  inst.operands[i].present = 1;
	  inst.operands[i].vectype = optype;
	}
      else if ((val = parse_scalar (&ptr, 8, &optype, REG_TYPE_VFD)) != FAIL)
	{
	  /* Case 6: VMOV<c><q>.<dt> <Rd>, <Dn[x]>  */
	  inst.operands[i].reg = val;
	  inst.operands[i].isscalar = 1;
	  inst.operands[i].present = 1;
	  inst.operands[i].vectype = optype;
	}
      else if ((val = arm_reg_parse (&ptr, REG_TYPE_RN)) != FAIL)
	{
	  inst.operands[i].reg = val;
	  inst.operands[i].isreg = 1;
	  inst.operands[i++].present = 1;

	  if (skip_past_comma (&ptr) == FAIL)
	    goto wanted_comma;

	  if ((val = arm_typed_reg_parse (&ptr, REG_TYPE_VFSD, &rtype, &optype))
	      != FAIL)
	    {
	      /* Case 7: VMOV<c><q> <Rd>, <Rn>, <Dm>  */

	      inst.operands[i].reg = val;
	      inst.operands[i].isreg = 1;
	      inst.operands[i].isvec = 1;
	      inst.operands[i].issingle = (rtype == REG_TYPE_VFS);
	      inst.operands[i].vectype = optype;
	      inst.operands[i].present = 1;

	      if (rtype == REG_TYPE_VFS)
		{
		  /* Case 14.  */
		  i++;
		  if (skip_past_comma (&ptr) == FAIL)
		    goto wanted_comma;
		  if ((val = arm_typed_reg_parse (&ptr, REG_TYPE_VFS, NULL,
						  &optype)) == FAIL)
		    {
		      first_error (_(reg_expected_msgs[REG_TYPE_VFS]));
		      return FAIL;
		    }
		  inst.operands[i].reg = val;
		  inst.operands[i].isreg = 1;
		  inst.operands[i].isvec = 1;
		  inst.operands[i].issingle = 1;
		  inst.operands[i].vectype = optype;
		  inst.operands[i].present = 1;
		}
	    }
	  else
	    {
	      if ((val = parse_scalar (&ptr, 8, &optype, REG_TYPE_MQ))
		       != FAIL)
		{
		  /* Case 16: VMOV<c> <Rt>, <Rt2>, <Qd[idx]>, <Qd[idx2]>  */
		  inst.operands[i].reg = val;
		  inst.operands[i].isvec = 1;
		  inst.operands[i].isscalar = 2;
		  inst.operands[i].vectype = optype;
		  inst.operands[i++].present = 1;

		  if (skip_past_comma (&ptr) == FAIL)
		    goto wanted_comma;

		  if ((val = parse_scalar (&ptr, 8, &optype, REG_TYPE_MQ))
		      == FAIL)
		    {
		      first_error (_(reg_expected_msgs[REG_TYPE_MQ]));
		      return FAIL;
		    }
		  inst.operands[i].reg = val;
		  inst.operands[i].isvec = 1;
		  inst.operands[i].isscalar = 2;
		  inst.operands[i].vectype = optype;
		  inst.operands[i].present = 1;
		}
	      else
		{
		  first_error (_("VFP single, double or MVE vector register"
			       " expected"));
		  return FAIL;
		}
	    }
	}
      else if ((val = arm_typed_reg_parse (&ptr, REG_TYPE_VFS, NULL, &optype))
	       != FAIL)
	{
	  /* Case 13.  */
	  inst.operands[i].reg = val;
	  inst.operands[i].isreg = 1;
	  inst.operands[i].isvec = 1;
	  inst.operands[i].issingle = 1;
	  inst.operands[i].vectype = optype;
	  inst.operands[i].present = 1;
	}
    }
  else
    {
      first_error (_("parse error"));
      return FAIL;
    }

  /* Successfully parsed the operands. Update args.  */
  *which_operand = i;
  *str = ptr;
  return SUCCESS;

 wanted_comma:
  first_error (_("expected comma"));
  return FAIL;

 wanted_arm:
  first_error (_(reg_expected_msgs[REG_TYPE_RN]));
  return FAIL;
}

/* Use this macro when the operand constraints are different
   for ARM and THUMB (e.g. ldrd).  */
#define MIX_ARM_THUMB_OPERANDS(arm_operand, thumb_operand) \
	((arm_operand) | ((thumb_operand) << 16))

/* Matcher codes for parse_operands.  */
enum operand_parse_code
{
  OP_stop,	/* end of line */

  OP_RR,	/* ARM register */
  OP_RRnpc,	/* ARM register, not r15 */
  OP_RRnpcsp,	/* ARM register, neither r15 nor r13 (a.k.a. 'BadReg') */
  OP_RRnpcb,	/* ARM register, not r15, in square brackets */
  OP_RRnpctw,	/* ARM register, not r15 in Thumb-state or with writeback,
		   optional trailing ! */
  OP_RRw,	/* ARM register, not r15, optional trailing ! */
  OP_RCP,	/* Coprocessor number */
  OP_RCN,	/* Coprocessor register */
  OP_RF,	/* FPA register */
  OP_RVS,	/* VFP single precision register */
  OP_RVD,	/* VFP double precision register (0..15) */
  OP_RND,       /* Neon double precision register (0..31) */
  OP_RNDMQ,     /* Neon double precision (0..31) or MVE vector register.  */
  OP_RNDMQR,    /* Neon double precision (0..31), MVE vector or ARM register.
		 */
  OP_RNSDMQR,    /* Neon single or double precision, MVE vector or ARM register.
		 */
  OP_RNQ,	/* Neon quad precision register */
  OP_RNQMQ,	/* Neon quad or MVE vector register.  */
  OP_RVSD,	/* VFP single or double precision register */
  OP_RVSD_COND,	/* VFP single, double precision register or condition code.  */
  OP_RVSDMQ,	/* VFP single, double precision or MVE vector register.  */
  OP_RNSD,      /* Neon single or double precision register */
  OP_RNDQ,      /* Neon double or quad precision register */
  OP_RNDQMQ,     /* Neon double, quad or MVE vector register.  */
  OP_RNDQMQR,   /* Neon double, quad, MVE vector or ARM register.  */
  OP_RNSDQ,	/* Neon single, double or quad precision register */
  OP_RNSC,      /* Neon scalar D[X] */
  OP_RVC,	/* VFP control register */
  OP_RMF,	/* Maverick F register */
  OP_RMD,	/* Maverick D register */
  OP_RMFX,	/* Maverick FX register */
  OP_RMDX,	/* Maverick DX register */
  OP_RMAX,	/* Maverick AX register */
  OP_RMDS,	/* Maverick DSPSC register */
  OP_RIWR,	/* iWMMXt wR register */
  OP_RIWC,	/* iWMMXt wC register */
  OP_RIWG,	/* iWMMXt wCG register */
  OP_RXA,	/* XScale accumulator register */

  OP_RNSDMQ,	/* Neon single, double or MVE vector register */
  OP_RNSDQMQ,	/* Neon single, double or quad register or MVE vector register
		 */
  OP_RNSDQMQR,	/* Neon single, double or quad register, MVE vector register or
		   GPR (no SP/SP)  */
  OP_RMQ,	/* MVE vector register.  */
  OP_RMQRZ,	/* MVE vector or ARM register including ZR.  */
  OP_RMQRR,     /* MVE vector or ARM register.  */

  /* New operands for Armv8.1-M Mainline.  */
  OP_LR,	/* ARM LR register */
  OP_SP,	/* ARM SP register */
  OP_R12,
  OP_RRe,	/* ARM register, only even numbered.  */
  OP_RRo,	/* ARM register, only odd numbered, not r13 or r15.  */
  OP_RRnpcsp_I32, /* ARM register (no BadReg) or literal 1 .. 32 */
  OP_RR_ZR,	/* ARM register or ZR but no PC */

  OP_REGLST,	/* ARM register list */
  OP_CLRMLST,	/* CLRM register list */
  OP_VRSLST,	/* VFP single-precision register list */
  OP_VRDLST,	/* VFP double-precision register list */
  OP_VRSDLST,   /* VFP single or double-precision register list (& quad) */
  OP_NRDLST,    /* Neon double-precision register list (d0-d31, qN aliases) */
  OP_NSTRLST,   /* Neon element/structure list */
  OP_VRSDVLST,  /* VFP single or double-precision register list and VPR */
  OP_MSTRLST2,	/* MVE vector list with two elements.  */
  OP_MSTRLST4,	/* MVE vector list with four elements.  */

  OP_RNDQ_I0,   /* Neon D or Q reg, or immediate zero.  */
  OP_RVSD_I0,	/* VFP S or D reg, or immediate zero.  */
  OP_RSVD_FI0, /* VFP S or D reg, or floating point immediate zero.  */
  OP_RSVDMQ_FI0, /* VFP S, D, MVE vector register or floating point immediate
		    zero.  */
  OP_RR_RNSC,   /* ARM reg or Neon scalar.  */
  OP_RNSD_RNSC, /* Neon S or D reg, or Neon scalar.  */
  OP_RNSDQ_RNSC, /* Vector S, D or Q reg, or Neon scalar.  */
  OP_RNSDQ_RNSC_MQ, /* Vector S, D or Q reg, Neon scalar or MVE vector register.
		     */
  OP_RNSDQ_RNSC_MQ_RR, /* Vector S, D or Q reg, or MVE vector reg , or Neon
			  scalar, or ARM register.  */
  OP_RNDQ_RNSC, /* Neon D or Q reg, or Neon scalar.  */
  OP_RNDQ_RNSC_RR, /* Neon D or Q reg, Neon scalar, or ARM register.  */
  OP_RNDQMQ_RNSC_RR, /* Neon D or Q reg, Neon scalar, MVE vector or ARM
			register.  */
  OP_RNDQMQ_RNSC, /* Neon D, Q or MVE vector reg, or Neon scalar.  */
  OP_RND_RNSC,  /* Neon D reg, or Neon scalar.  */
  OP_VMOV,      /* Neon VMOV operands.  */
  OP_RNDQ_Ibig,	/* Neon D or Q reg, or big immediate for logic and VMVN.  */
  /* Neon D, Q or MVE vector register, or big immediate for logic and VMVN.  */
  OP_RNDQMQ_Ibig,
  OP_RNDQ_I63b, /* Neon D or Q reg, or immediate for shift.  */
  OP_RNDQMQ_I63b_RR, /* Neon D or Q reg, immediate for shift, MVE vector or
			ARM register.  */
  OP_RIWR_I32z, /* iWMMXt wR register, or immediate 0 .. 32 for iWMMXt2.  */
  OP_VLDR,	/* VLDR operand.  */

  OP_I0,        /* immediate zero */
  OP_I7,	/* immediate value 0 .. 7 */
  OP_I15,	/*		   0 .. 15 */
  OP_I16,	/*		   1 .. 16 */
  OP_I16z,      /*                 0 .. 16 */
  OP_I31,	/*		   0 .. 31 */
  OP_I31w,	/*		   0 .. 31, optional trailing ! */
  OP_I32,	/*		   1 .. 32 */
  OP_I32z,	/*		   0 .. 32 */
  OP_I48_I64,	/*		   48 or 64 */
  OP_I63,	/*		   0 .. 63 */
  OP_I63s,	/*		 -64 .. 63 */
  OP_I64,	/*		   1 .. 64 */
  OP_I64z,	/*		   0 .. 64 */
  OP_I127,	/*		   0 .. 127 */
  OP_I255,	/*		   0 .. 255 */
  OP_I511,	/*		   0 .. 511 */
  OP_I4095,	/*		   0 .. 4095 */
  OP_I8191,	/*		   0 .. 8191 */
  OP_I4b,	/* immediate, prefix optional, 1 .. 4 */
  OP_I7b,	/*			       0 .. 7 */
  OP_I15b,	/*			       0 .. 15 */
  OP_I31b,	/*			       0 .. 31 */

  OP_SH,	/* shifter operand */
  OP_SHG,	/* shifter operand with possible group relocation */
  OP_ADDR,	/* Memory address expression (any mode) */
  OP_ADDRMVE,	/* Memory address expression for MVE's VSTR/VLDR.  */
  OP_ADDRGLDR,	/* Mem addr expr (any mode) with possible LDR group reloc */
  OP_ADDRGLDRS, /* Mem addr expr (any mode) with possible LDRS group reloc */
  OP_ADDRGLDC,  /* Mem addr expr (any mode) with possible LDC group reloc */
  OP_EXP,	/* arbitrary expression */
  OP_EXPi,	/* same, with optional immediate prefix */
  OP_EXPr,	/* same, with optional relocation suffix */
  OP_EXPs,	/* same, with optional non-first operand relocation suffix */
  OP_HALF,	/* 0 .. 65535 or low/high reloc.  */
  OP_IROT1,	/* VCADD rotate immediate: 90, 270.  */
  OP_IROT2,	/* VCMLA rotate immediate: 0, 90, 180, 270.  */

  OP_CPSF,	/* CPS flags */
  OP_ENDI,	/* Endianness specifier */
  OP_wPSR,	/* CPSR/SPSR/APSR mask for msr (writing).  */
  OP_rPSR,	/* CPSR/SPSR/APSR mask for msr (reading).  */
  OP_COND,	/* conditional code */
  OP_TB,	/* Table branch.  */

  OP_APSR_RR,   /* ARM register or "APSR_nzcv".  */

  OP_RRnpc_I0,	/* ARM register or literal 0 */
  OP_RR_EXr,	/* ARM register or expression with opt. reloc stuff. */
  OP_RR_EXi,	/* ARM register or expression with imm prefix */
  OP_RF_IF,	/* FPA register or immediate */
  OP_RIWR_RIWC, /* iWMMXt R or C reg */
  OP_RIWC_RIWG, /* iWMMXt wC or wCG reg */

  /* Optional operands.	 */
  OP_oI7b,	 /* immediate, prefix optional, 0 .. 7 */
  OP_oI31b,	 /*				0 .. 31 */
  OP_oI32b,      /*                             1 .. 32 */
  OP_oI32z,      /*                             0 .. 32 */
  OP_oIffffb,	 /*				0 .. 65535 */
  OP_oI255c,	 /*	  curly-brace enclosed, 0 .. 255 */

  OP_oRR,	 /* ARM register */
  OP_oLR,	 /* ARM LR register */
  OP_oRRnpc,	 /* ARM register, not the PC */
  OP_oRRnpcsp,	 /* ARM register, neither the PC nor the SP (a.k.a. BadReg) */
  OP_oRRw,	 /* ARM register, not r15, optional trailing ! */
  OP_oRND,       /* Optional Neon double precision register */
  OP_oRNQ,       /* Optional Neon quad precision register */
  OP_oRNDQMQ,     /* Optional Neon double, quad or MVE vector register.  */
  OP_oRNDQ,      /* Optional Neon double or quad precision register */
  OP_oRNSDQ,	 /* Optional single, double or quad precision vector register */
  OP_oRNSDQMQ,	 /* Optional single, double or quad register or MVE vector
		    register.  */
  OP_oRNSDMQ,	 /* Optional single, double register or MVE vector
		    register.  */
  OP_oSHll,	 /* LSL immediate */
  OP_oSHar,	 /* ASR immediate */
  OP_oSHllar,	 /* LSL or ASR immediate */
  OP_oROR,	 /* ROR 0/8/16/24 */
  OP_oBARRIER_I15, /* Option argument for a barrier instruction.  */

  OP_oRMQRZ,	/* optional MVE vector or ARM register including ZR.  */

  /* Some pre-defined mixed (ARM/THUMB) operands.  */
  OP_RR_npcsp		= MIX_ARM_THUMB_OPERANDS (OP_RR, OP_RRnpcsp),
  OP_RRnpc_npcsp	= MIX_ARM_THUMB_OPERANDS (OP_RRnpc, OP_RRnpcsp),
  OP_oRRnpc_npcsp	= MIX_ARM_THUMB_OPERANDS (OP_oRRnpc, OP_oRRnpcsp),

  OP_FIRST_OPTIONAL = OP_oI7b
};

/* Generic instruction operand parser.	This does no encoding and no
   semantic validation; it merely squirrels values away in the inst
   structure.  Returns SUCCESS or FAIL depending on whether the
   specified grammar matched.  */
static int
parse_operands (char *str, const unsigned int *pattern, bool thumb)
{
  unsigned const int *upat = pattern;
  char *backtrack_pos = 0;
  const char *backtrack_error = 0;
  int i, val = 0, backtrack_index = 0;
  enum arm_reg_type rtype;
  parse_operand_result result;
  unsigned int op_parse_code;
  bool partial_match;

#define po_char_or_fail(chr)			\
  do						\
    {						\
      if (skip_past_char (&str, chr) == FAIL)	\
	goto bad_args;				\
    }						\
  while (0)

#define po_reg_or_fail(regtype)					\
  do								\
    {								\
      val = arm_typed_reg_parse (& str, regtype, & rtype,	\
				 & inst.operands[i].vectype);	\
      if (val == FAIL)						\
	{							\
	  first_error (_(reg_expected_msgs[regtype]));		\
	  goto failure;						\
	}							\
      inst.operands[i].reg = val;				\
      inst.operands[i].isreg = 1;				\
      inst.operands[i].isquad = (rtype == REG_TYPE_NQ);		\
      inst.operands[i].issingle = (rtype == REG_TYPE_VFS);	\
      inst.operands[i].isvec = (rtype == REG_TYPE_VFS		\
			     || rtype == REG_TYPE_VFD		\
			     || rtype == REG_TYPE_NQ);		\
      inst.operands[i].iszr = (rtype == REG_TYPE_ZR);		\
    }								\
  while (0)

#define po_reg_or_goto(regtype, label)				\
  do								\
    {								\
      val = arm_typed_reg_parse (& str, regtype, & rtype,	\
				 & inst.operands[i].vectype);	\
      if (val == FAIL)						\
	goto label;						\
								\
      inst.operands[i].reg = val;				\
      inst.operands[i].isreg = 1;				\
      inst.operands[i].isquad = (rtype == REG_TYPE_NQ);		\
      inst.operands[i].issingle = (rtype == REG_TYPE_VFS);	\
      inst.operands[i].isvec = (rtype == REG_TYPE_VFS		\
			     || rtype == REG_TYPE_VFD		\
			     || rtype == REG_TYPE_NQ);		\
      inst.operands[i].iszr = (rtype == REG_TYPE_ZR);		\
    }								\
  while (0)

#define po_imm_or_fail(min, max, popt)				\
  do								\
    {								\
      if (parse_immediate (&str, &val, min, max, popt) == FAIL)	\
	goto failure;						\
      inst.operands[i].imm = val;				\
    }								\
  while (0)

#define po_imm1_or_imm2_or_fail(imm1, imm2, popt)		\
  do								\
    {								\
      expressionS exp;						\
      my_get_expression (&exp, &str, popt);			\
      if (exp.X_op != O_constant)				\
	{							\
	  inst.error = _("constant expression required");	\
	  goto failure;						\
	}							\
      if (exp.X_add_number != imm1 && exp.X_add_number != imm2) \
	{							\
	  inst.error = _("immediate value 48 or 64 expected");	\
	  goto failure;						\
	}							\
      inst.operands[i].imm = exp.X_add_number;			\
    }								\
  while (0)

#define po_scalar_or_goto(elsz, label, reg_type)			\
  do									\
    {									\
      val = parse_scalar (& str, elsz, & inst.operands[i].vectype,	\
			  reg_type);					\
      if (val == FAIL)							\
	goto label;							\
      inst.operands[i].reg = val;					\
      inst.operands[i].isscalar = 1;					\
    }									\
  while (0)

#define po_misc_or_fail(expr)			\
  do						\
    {						\
      if (expr)					\
	goto failure;				\
    }						\
  while (0)

#define po_misc_or_fail_no_backtrack(expr)		\
  do							\
    {							\
      result = expr;					\
      if (result == PARSE_OPERAND_FAIL_NO_BACKTRACK)	\
	backtrack_pos = 0;				\
      if (result != PARSE_OPERAND_SUCCESS)		\
	goto failure;					\
    }							\
  while (0)

#define po_barrier_or_imm(str)				   \
  do							   \
    {						 	   \
      val = parse_barrier (&str);			   \
      if (val == FAIL && ! ISALPHA (*str))		   \
	goto immediate;					   \
      if (val == FAIL					   \
	  /* ISB can only take SY as an option.  */	   \
	  || ((inst.instruction & 0xf0) == 0x60		   \
	       && val != 0xf))				   \
	{						   \
	   inst.error = _("invalid barrier type");	   \
	   backtrack_pos = 0;				   \
	   goto failure;				   \
	}						   \
    }							   \
  while (0)

  skip_whitespace (str);

  for (i = 0; upat[i] != OP_stop; i++)
    {
      op_parse_code = upat[i];
      if (op_parse_code >= 1<<16)
	op_parse_code = thumb ? (op_parse_code >> 16)
				: (op_parse_code & ((1<<16)-1));

      if (op_parse_code >= OP_FIRST_OPTIONAL)
	{
	  /* Remember where we are in case we need to backtrack.  */
	  backtrack_pos = str;
	  backtrack_error = inst.error;
	  backtrack_index = i;
	}

      if (i > 0 && (i > 1 || inst.operands[0].present))
	po_char_or_fail (',');

      switch (op_parse_code)
	{
	  /* Registers */
	case OP_oRRnpc:
	case OP_oRRnpcsp:
	case OP_RRnpc:
	case OP_RRnpcsp:
	case OP_oRR:
	case OP_RRe:
	case OP_RRo:
	case OP_LR:
	case OP_oLR:
	case OP_SP:
	case OP_R12:
	case OP_RR:    po_reg_or_fail (REG_TYPE_RN);	  break;
	case OP_RCP:   po_reg_or_fail (REG_TYPE_CP);	  break;
	case OP_RCN:   po_reg_or_fail (REG_TYPE_CN);	  break;
	case OP_RF:    po_reg_or_fail (REG_TYPE_FN);	  break;
	case OP_RVS:   po_reg_or_fail (REG_TYPE_VFS);	  break;
	case OP_RVD:   po_reg_or_fail (REG_TYPE_VFD);	  break;
	case OP_oRND:
	case OP_RNSDMQR:
	  po_reg_or_goto (REG_TYPE_VFS, try_rndmqr);
	  break;
	try_rndmqr:
	case OP_RNDMQR:
	  po_reg_or_goto (REG_TYPE_RN, try_rndmq);
	  break;
	try_rndmq:
	case OP_RNDMQ:
	  po_reg_or_goto (REG_TYPE_MQ, try_rnd);
	  break;
	try_rnd:
	case OP_RND:   po_reg_or_fail (REG_TYPE_VFD);	  break;
	case OP_RVC:
	  po_reg_or_goto (REG_TYPE_VFC, coproc_reg);
	  break;
	  /* Also accept generic coprocessor regs for unknown registers.  */
	  coproc_reg:
	  po_reg_or_goto (REG_TYPE_CN, vpr_po);
	  break;
	  /* Also accept P0 or p0 for VPR.P0.  Since P0 is already an
	     existing register with a value of 0, this seems like the
	     best way to parse P0.  */
	  vpr_po:
	  if (strncasecmp (str, "P0", 2) == 0)
	    {
	      str += 2;
	      inst.operands[i].isreg = 1;
	      inst.operands[i].reg = 13;
	    }
	  else
	    goto failure;
	  break;
	case OP_RMF:   po_reg_or_fail (REG_TYPE_MVF);	  break;
	case OP_RMD:   po_reg_or_fail (REG_TYPE_MVD);	  break;
	case OP_RMFX:  po_reg_or_fail (REG_TYPE_MVFX);	  break;
	case OP_RMDX:  po_reg_or_fail (REG_TYPE_MVDX);	  break;
	case OP_RMAX:  po_reg_or_fail (REG_TYPE_MVAX);	  break;
	case OP_RMDS:  po_reg_or_fail (REG_TYPE_DSPSC);	  break;
	case OP_RIWR:  po_reg_or_fail (REG_TYPE_MMXWR);	  break;
	case OP_RIWC:  po_reg_or_fail (REG_TYPE_MMXWC);	  break;
	case OP_RIWG:  po_reg_or_fail (REG_TYPE_MMXWCG);  break;
	case OP_RXA:   po_reg_or_fail (REG_TYPE_XSCALE);  break;
	case OP_oRNQ:
	case OP_RNQMQ:
	  po_reg_or_goto (REG_TYPE_MQ, try_nq);
	  break;
	try_nq:
	case OP_RNQ:   po_reg_or_fail (REG_TYPE_NQ);      break;
	case OP_RNSD:  po_reg_or_fail (REG_TYPE_NSD);     break;
	case OP_RNDQMQR:
	  po_reg_or_goto (REG_TYPE_RN, try_rndqmq);
	  break;
	try_rndqmq:
	case OP_oRNDQMQ:
	case OP_RNDQMQ:
	  po_reg_or_goto (REG_TYPE_MQ, try_rndq);
	  break;
	try_rndq:
	case OP_oRNDQ:
	case OP_RNDQ:  po_reg_or_fail (REG_TYPE_NDQ);     break;
	case OP_RVSDMQ:
	  po_reg_or_goto (REG_TYPE_MQ, try_rvsd);
	  break;
	try_rvsd:
	case OP_RVSD:  po_reg_or_fail (REG_TYPE_VFSD);    break;
	case OP_RVSD_COND:
	  po_reg_or_goto (REG_TYPE_VFSD, try_cond);
	  break;
	case OP_oRNSDMQ:
	case OP_RNSDMQ:
	  po_reg_or_goto (REG_TYPE_NSD, try_mq2);
	  break;
	  try_mq2:
	  po_reg_or_fail (REG_TYPE_MQ);
	  break;
	case OP_oRNSDQ:
	case OP_RNSDQ: po_reg_or_fail (REG_TYPE_NSDQ);    break;
	case OP_RNSDQMQR:
	  po_reg_or_goto (REG_TYPE_RN, try_mq);
	  break;
	  try_mq:
	case OP_oRNSDQMQ:
	case OP_RNSDQMQ:
	  po_reg_or_goto (REG_TYPE_MQ, try_nsdq2);
	  break;
	  try_nsdq2:
	  po_reg_or_fail (REG_TYPE_NSDQ);
	  inst.error = 0;
	  break;
	case OP_RMQRR:
	  po_reg_or_goto (REG_TYPE_RN, try_rmq);
	  break;
	try_rmq:
	case OP_RMQ:
	  po_reg_or_fail (REG_TYPE_MQ);
	  break;
	/* Neon scalar. Using an element size of 8 means that some invalid
	   scalars are accepted here, so deal with those in later code.  */
	case OP_RNSC:  po_scalar_or_goto (8, failure, REG_TYPE_VFD);    break;

	case OP_RNDQ_I0:
	  {
	    po_reg_or_goto (REG_TYPE_NDQ, try_imm0);
	    break;
	    try_imm0:
	    po_imm_or_fail (0, 0, true);
	  }
	  break;

	case OP_RVSD_I0:
	  po_reg_or_goto (REG_TYPE_VFSD, try_imm0);
	  break;

	case OP_RSVDMQ_FI0:
	  po_reg_or_goto (REG_TYPE_MQ, try_rsvd_fi0);
	  break;
	try_rsvd_fi0:
	case OP_RSVD_FI0:
	  {
	    po_reg_or_goto (REG_TYPE_VFSD, try_ifimm0);
	    break;
	    try_ifimm0:
	    if (parse_ifimm_zero (&str))
	      inst.operands[i].imm = 0;
	    else
	    {
	      inst.error
	        = _("only floating point zero is allowed as immediate value");
	      goto failure;
	    }
	  }
	  break;

	case OP_RR_RNSC:
	  {
	    po_scalar_or_goto (8, try_rr, REG_TYPE_VFD);
	    break;
	    try_rr:
	    po_reg_or_fail (REG_TYPE_RN);
	  }
	  break;

	case OP_RNSDQ_RNSC_MQ_RR:
	  po_reg_or_goto (REG_TYPE_RN, try_rnsdq_rnsc_mq);
	  break;
	try_rnsdq_rnsc_mq:
	case OP_RNSDQ_RNSC_MQ:
	  po_reg_or_goto (REG_TYPE_MQ, try_rnsdq_rnsc);
	  break;
	try_rnsdq_rnsc:
	case OP_RNSDQ_RNSC:
	  {
	    po_scalar_or_goto (8, try_nsdq, REG_TYPE_VFD);
	    inst.error = 0;
	    break;
	    try_nsdq:
	    po_reg_or_fail (REG_TYPE_NSDQ);
	    inst.error = 0;
	  }
	  break;

	case OP_RNSD_RNSC:
	  {
	    po_scalar_or_goto (8, try_s_scalar, REG_TYPE_VFD);
	    break;
	    try_s_scalar:
	    po_scalar_or_goto (4, try_nsd, REG_TYPE_VFS);
	    break;
	    try_nsd:
	    po_reg_or_fail (REG_TYPE_NSD);
	  }
	  break;

	case OP_RNDQMQ_RNSC_RR:
	  po_reg_or_goto (REG_TYPE_MQ, try_rndq_rnsc_rr);
	  break;
	try_rndq_rnsc_rr:
	case OP_RNDQ_RNSC_RR:
	  po_reg_or_goto (REG_TYPE_RN, try_rndq_rnsc);
	  break;
	case OP_RNDQMQ_RNSC:
	  po_reg_or_goto (REG_TYPE_MQ, try_rndq_rnsc);
	  break;
	try_rndq_rnsc:
	case OP_RNDQ_RNSC:
	  {
	    po_scalar_or_goto (8, try_ndq, REG_TYPE_VFD);
	    break;
	    try_ndq:
	    po_reg_or_fail (REG_TYPE_NDQ);
	  }
	  break;

	case OP_RND_RNSC:
	  {
	    po_scalar_or_goto (8, try_vfd, REG_TYPE_VFD);
	    break;
	    try_vfd:
	    po_reg_or_fail (REG_TYPE_VFD);
	  }
	  break;

	case OP_VMOV:
	  /* WARNING: parse_neon_mov can move the operand counter, i. If we're
	     not careful then bad things might happen.  */
	  po_misc_or_fail (parse_neon_mov (&str, &i) == FAIL);
	  break;

	case OP_RNDQMQ_Ibig:
	  po_reg_or_goto (REG_TYPE_MQ, try_rndq_ibig);
	  break;
	try_rndq_ibig:
	case OP_RNDQ_Ibig:
	  {
	    po_reg_or_goto (REG_TYPE_NDQ, try_immbig);
	    break;
	    try_immbig:
	    /* There's a possibility of getting a 64-bit immediate here, so
	       we need special handling.  */
	    if (parse_big_immediate (&str, i, NULL, /*allow_symbol_p=*/false)
		== FAIL)
	      {
		inst.error = _("immediate value is out of range");
		goto failure;
	      }
	  }
	  break;

	case OP_RNDQMQ_I63b_RR:
	  po_reg_or_goto (REG_TYPE_MQ, try_rndq_i63b_rr);
	  break;
	try_rndq_i63b_rr:
	  po_reg_or_goto (REG_TYPE_RN, try_rndq_i63b);
	  break;
	try_rndq_i63b:
	case OP_RNDQ_I63b:
	  {
	    po_reg_or_goto (REG_TYPE_NDQ, try_shimm);
	    break;
	    try_shimm:
	    po_imm_or_fail (0, 63, true);
	  }
	  break;

	case OP_RRnpcb:
	  po_char_or_fail ('[');
	  po_reg_or_fail  (REG_TYPE_RN);
	  po_char_or_fail (']');
	  break;

	case OP_RRnpctw:
	case OP_RRw:
	case OP_oRRw:
	  po_reg_or_fail (REG_TYPE_RN);
	  if (skip_past_char (&str, '!') == SUCCESS)
	    inst.operands[i].writeback = 1;
	  break;

	  /* Immediates */
	case OP_I7:	 po_imm_or_fail (  0,	   7, false);	break;
	case OP_I15:	 po_imm_or_fail (  0,	  15, false);	break;
	case OP_I16:	 po_imm_or_fail (  1,	  16, false);	break;
	case OP_I16z:	 po_imm_or_fail (  0,     16, false);   break;
	case OP_I31:	 po_imm_or_fail (  0,	  31, false);	break;
	case OP_I32:	 po_imm_or_fail (  1,	  32, false);	break;
	case OP_I32z:	 po_imm_or_fail (  0,     32, false);   break;
	case OP_I48_I64: po_imm1_or_imm2_or_fail (48, 64, false); break;
	case OP_I63s:	 po_imm_or_fail (-64,	  63, false);	break;
	case OP_I63:	 po_imm_or_fail (  0,     63, false);   break;
	case OP_I64:	 po_imm_or_fail (  1,     64, false);   break;
	case OP_I64z:	 po_imm_or_fail (  0,     64, false);   break;
	case OP_I127:	 po_imm_or_fail (  0,	 127, false);	break;
	case OP_I255:	 po_imm_or_fail (  0,	 255, false);	break;
	case OP_I511:	 po_imm_or_fail (  0,	 511, false);	break;
	case OP_I4095:	 po_imm_or_fail (  0,	 4095, false);	break;
	case OP_I8191:   po_imm_or_fail (  0,	 8191, false);	break;
	case OP_I4b:	 po_imm_or_fail (  1,	   4, true);	break;
	case OP_oI7b:
	case OP_I7b:	 po_imm_or_fail (  0,	   7, true);	break;
	case OP_I15b:	 po_imm_or_fail (  0,	  15, true);	break;
	case OP_oI31b:
	case OP_I31b:	 po_imm_or_fail (  0,	  31, true);	break;
	case OP_oI32b:   po_imm_or_fail (  1,     32, true);    break;
	case OP_oI32z:   po_imm_or_fail (  0,     32, true);    break;
	case OP_oIffffb: po_imm_or_fail (  0, 0xffff, true);	break;

	  /* Immediate variants */
	case OP_oI255c:
	  po_char_or_fail ('{');
	  po_imm_or_fail (0, 255, true);
	  po_char_or_fail ('}');
	  break;

	case OP_I31w:
	  /* The expression parser chokes on a trailing !, so we have
	     to find it first and zap it.  */
	  {
	    char *s = str;
	    while (*s && *s != ',')
	      s++;
	    if (s[-1] == '!')
	      {
		s[-1] = '\0';
		inst.operands[i].writeback = 1;
	      }
	    po_imm_or_fail (0, 31, true);
	    if (str == s - 1)
	      str = s;
	  }
	  break;

	  /* Expressions */
	case OP_EXPi:	EXPi:
	  po_misc_or_fail (my_get_expression (&inst.relocs[0].exp, &str,
					      GE_OPT_PREFIX));
	  break;

	case OP_EXP:
	  po_misc_or_fail (my_get_expression (&inst.relocs[0].exp, &str,
					      GE_NO_PREFIX));
	  break;

	case OP_EXPr:	EXPr:
	  po_misc_or_fail (my_get_expression (&inst.relocs[0].exp, &str,
					      GE_NO_PREFIX));
	  if (inst.relocs[0].exp.X_op == O_symbol)
	    {
	      val = parse_reloc (&str);
	      if (val == -1)
		{
		  inst.error = _("unrecognized relocation suffix");
		  goto failure;
		}
	      else if (val != BFD_RELOC_UNUSED)
		{
		  inst.operands[i].imm = val;
		  inst.operands[i].hasreloc = 1;
		}
	    }
	  break;

	case OP_EXPs:
	  po_misc_or_fail (my_get_expression (&inst.relocs[i].exp, &str,
					      GE_NO_PREFIX));
	  if (inst.relocs[i].exp.X_op == O_symbol)
	    {
	      inst.operands[i].hasreloc = 1;
	    }
	  else if (inst.relocs[i].exp.X_op == O_constant)
	    {
	      inst.operands[i].imm = inst.relocs[i].exp.X_add_number;
	      inst.operands[i].hasreloc = 0;
	    }
	  break;

	  /* Operand for MOVW or MOVT.  */
	case OP_HALF:
	  po_misc_or_fail (parse_half (&str));
	  break;

	  /* Register or expression.  */
	case OP_RR_EXr:	  po_reg_or_goto (REG_TYPE_RN, EXPr); break;
	case OP_RR_EXi:	  po_reg_or_goto (REG_TYPE_RN, EXPi); break;

	  /* Register or immediate.  */
	case OP_RRnpc_I0: po_reg_or_goto (REG_TYPE_RN, I0);   break;
	I0:		  po_imm_or_fail (0, 0, false);	      break;

	case OP_RRnpcsp_I32: po_reg_or_goto (REG_TYPE_RN, I32);	break;
	I32:		     po_imm_or_fail (1, 32, false);	break;

	case OP_RF_IF:    po_reg_or_goto (REG_TYPE_FN, IF);   break;
	IF:
	  if (!is_immediate_prefix (*str))
	    goto bad_args;
	  str++;
	  val = parse_fpa_immediate (&str);
	  if (val == FAIL)
	    goto failure;
	  /* FPA immediates are encoded as registers 8-15.
	     parse_fpa_immediate has already applied the offset.  */
	  inst.operands[i].reg = val;
	  inst.operands[i].isreg = 1;
	  break;

	case OP_RIWR_I32z: po_reg_or_goto (REG_TYPE_MMXWR, I32z); break;
	I32z:		  po_imm_or_fail (0, 32, false);	  break;

	  /* Two kinds of register.  */
	case OP_RIWR_RIWC:
	  {
	    struct reg_entry *rege = arm_reg_parse_multi (&str);
	    if (!rege
		|| (rege->type != REG_TYPE_MMXWR
		    && rege->type != REG_TYPE_MMXWC
		    && rege->type != REG_TYPE_MMXWCG))
	      {
		inst.error = _("iWMMXt data or control register expected");
		goto failure;
	      }
	    inst.operands[i].reg = rege->number;
	    inst.operands[i].isreg = (rege->type == REG_TYPE_MMXWR);
	  }
	  break;

	case OP_RIWC_RIWG:
	  {
	    struct reg_entry *rege = arm_reg_parse_multi (&str);
	    if (!rege
		|| (rege->type != REG_TYPE_MMXWC
		    && rege->type != REG_TYPE_MMXWCG))
	      {
		inst.error = _("iWMMXt control register expected");
		goto failure;
	      }
	    inst.operands[i].reg = rege->number;
	    inst.operands[i].isreg = 1;
	  }
	  break;

	  /* Misc */
	case OP_CPSF:	 val = parse_cps_flags (&str);		break;
	case OP_ENDI:	 val = parse_endian_specifier (&str);	break;
	case OP_oROR:	 val = parse_ror (&str);		break;
	try_cond:
	case OP_COND:	 val = parse_cond (&str);		break;
	case OP_oBARRIER_I15:
	  po_barrier_or_imm (str); break;
	  immediate:
	  if (parse_immediate (&str, &val, 0, 15, true) == FAIL)
	    goto failure;
	  break;

	case OP_wPSR:
	case OP_rPSR:
	  po_reg_or_goto (REG_TYPE_RNB, try_psr);
	  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_virt))
	    {
	      inst.error = _("Banked registers are not available with this "
			     "architecture.");
	      goto failure;
	    }
	  break;
	  try_psr:
	  val = parse_psr (&str, op_parse_code == OP_wPSR);
	  break;

	case OP_VLDR:
	  po_reg_or_goto (REG_TYPE_VFSD, try_sysreg);
	  break;
	try_sysreg:
	  val = parse_sys_vldr_vstr (&str);
	  break;

	case OP_APSR_RR:
	  po_reg_or_goto (REG_TYPE_RN, try_apsr);
	  break;
	  try_apsr:
	  /* Parse "APSR_nvzc" operand (for FMSTAT-equivalent MRS
	     instruction).  */
	  if (strncasecmp (str, "APSR_", 5) == 0)
	    {
	      unsigned found = 0;
	      str += 5;
	      while (found < 15)
		switch (*str++)
		  {
		  case 'c': found = (found & 1) ? 16 : found | 1; break;
		  case 'n': found = (found & 2) ? 16 : found | 2; break;
		  case 'z': found = (found & 4) ? 16 : found | 4; break;
		  case 'v': found = (found & 8) ? 16 : found | 8; break;
		  default: found = 16;
		  }
	      if (found != 15)
		goto failure;
	      inst.operands[i].isvec = 1;
	      /* APSR_nzcv is encoded in instructions as if it were the REG_PC.  */
	      inst.operands[i].reg = REG_PC;
	    }
	  else
	    goto failure;
	  break;

	case OP_TB:
	  po_misc_or_fail (parse_tb (&str));
	  break;

	  /* Register lists.  */
	case OP_REGLST:
	  val = parse_reg_list (&str, REGLIST_RN);
	  if (*str == '^')
	    {
	      inst.operands[i].writeback = 1;
	      str++;
	    }
	  break;

	case OP_CLRMLST:
	  val = parse_reg_list (&str, REGLIST_CLRM);
	  break;

	case OP_VRSLST:
	  val = parse_vfp_reg_list (&str, &inst.operands[i].reg, REGLIST_VFP_S,
				    &partial_match);
	  break;

	case OP_VRDLST:
	  val = parse_vfp_reg_list (&str, &inst.operands[i].reg, REGLIST_VFP_D,
				    &partial_match);
	  break;

	case OP_VRSDLST:
	  /* Allow Q registers too.  */
	  val = parse_vfp_reg_list (&str, &inst.operands[i].reg,
				    REGLIST_NEON_D, &partial_match);
	  if (val == FAIL)
	    {
	      inst.error = NULL;
	      val = parse_vfp_reg_list (&str, &inst.operands[i].reg,
					REGLIST_VFP_S, &partial_match);
	      inst.operands[i].issingle = 1;
	    }
	  break;

	case OP_VRSDVLST:
	  val = parse_vfp_reg_list (&str, &inst.operands[i].reg,
				    REGLIST_VFP_D_VPR, &partial_match);
	  if (val == FAIL && !partial_match)
	    {
	      inst.error = NULL;
	      val = parse_vfp_reg_list (&str, &inst.operands[i].reg,
					REGLIST_VFP_S_VPR, &partial_match);
	      inst.operands[i].issingle = 1;
	    }
	  break;

	case OP_NRDLST:
	  val = parse_vfp_reg_list (&str, &inst.operands[i].reg,
				    REGLIST_NEON_D, &partial_match);
	  break;

	case OP_MSTRLST4:
	case OP_MSTRLST2:
	  val = parse_neon_el_struct_list (&str, &inst.operands[i].reg,
					   1, &inst.operands[i].vectype);
	  if (val != (((op_parse_code == OP_MSTRLST2) ? 3 : 7) << 5 | 0xe))
	    goto failure;
	  break;
	case OP_NSTRLST:
	  val = parse_neon_el_struct_list (&str, &inst.operands[i].reg,
					   0, &inst.operands[i].vectype);
	  break;

	  /* Addressing modes */
	case OP_ADDRMVE:
	  po_misc_or_fail (parse_address_group_reloc (&str, i, GROUP_MVE));
	  break;

	case OP_ADDR:
	  po_misc_or_fail (parse_address (&str, i));
	  break;

	case OP_ADDRGLDR:
	  po_misc_or_fail_no_backtrack (
	    parse_address_group_reloc (&str, i, GROUP_LDR));
	  break;

	case OP_ADDRGLDRS:
	  po_misc_or_fail_no_backtrack (
	    parse_address_group_reloc (&str, i, GROUP_LDRS));
	  break;

	case OP_ADDRGLDC:
	  po_misc_or_fail_no_backtrack (
	    parse_address_group_reloc (&str, i, GROUP_LDC));
	  break;

	case OP_SH:
	  po_misc_or_fail (parse_shifter_operand (&str, i));
	  break;

	case OP_SHG:
	  po_misc_or_fail_no_backtrack (
	    parse_shifter_operand_group_reloc (&str, i));
	  break;

	case OP_oSHll:
	  po_misc_or_fail (parse_shift (&str, i, SHIFT_LSL_IMMEDIATE));
	  break;

	case OP_oSHar:
	  po_misc_or_fail (parse_shift (&str, i, SHIFT_ASR_IMMEDIATE));
	  break;

	case OP_oSHllar:
	  po_misc_or_fail (parse_shift (&str, i, SHIFT_LSL_OR_ASR_IMMEDIATE));
	  break;

	case OP_RMQRZ:
	case OP_oRMQRZ:
	  po_reg_or_goto (REG_TYPE_MQ, try_rr_zr);
	  break;

	case OP_RR_ZR:
	try_rr_zr:
	  po_reg_or_goto (REG_TYPE_RN, ZR);
	  break;
	ZR:
	  po_reg_or_fail (REG_TYPE_ZR);
	  break;

	default:
	  as_fatal (_("unhandled operand code %d"), op_parse_code);
	}

      /* Various value-based sanity checks and shared operations.  We
	 do not signal immediate failures for the register constraints;
	 this allows a syntax error to take precedence.	 */
      switch (op_parse_code)
	{
	case OP_oRRnpc:
	case OP_RRnpc:
	case OP_RRnpcb:
	case OP_RRw:
	case OP_oRRw:
	case OP_RRnpc_I0:
	  if (inst.operands[i].isreg && inst.operands[i].reg == REG_PC)
	    inst.error = BAD_PC;
	  break;

	case OP_oRRnpcsp:
	case OP_RRnpcsp:
	case OP_RRnpcsp_I32:
	  if (inst.operands[i].isreg)
	    {
	      if (inst.operands[i].reg == REG_PC)
		inst.error = BAD_PC;
	      else if (inst.operands[i].reg == REG_SP
		       /* The restriction on Rd/Rt/Rt2 on Thumb mode has been
			  relaxed since ARMv8-A.  */
		       && !ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))
		{
		  gas_assert (thumb);
		  inst.error = BAD_SP;
		}
	    }
	  break;

	case OP_RRnpctw:
	  if (inst.operands[i].isreg
	      && inst.operands[i].reg == REG_PC
	      && (inst.operands[i].writeback || thumb))
	    inst.error = BAD_PC;
	  break;

	case OP_RVSD_COND:
	case OP_VLDR:
	  if (inst.operands[i].isreg)
	    break;
	/* fall through.  */

	case OP_CPSF:
	case OP_ENDI:
	case OP_oROR:
	case OP_wPSR:
	case OP_rPSR:
	case OP_COND:
	case OP_oBARRIER_I15:
	case OP_REGLST:
	case OP_CLRMLST:
	case OP_VRSLST:
	case OP_VRDLST:
	case OP_VRSDLST:
	case OP_VRSDVLST:
	case OP_NRDLST:
	case OP_NSTRLST:
	case OP_MSTRLST2:
	case OP_MSTRLST4:
	  if (val == FAIL)
	    goto failure;
	  inst.operands[i].imm = val;
	  break;

	case OP_LR:
	case OP_oLR:
	  if (inst.operands[i].reg != REG_LR)
	    inst.error = _("operand must be LR register");
	  break;

	case OP_SP:
	  if (inst.operands[i].reg != REG_SP)
	    inst.error = _("operand must be SP register");
	  break;

	case OP_R12:
	  if (inst.operands[i].reg != REG_R12)
	    inst.error = _("operand must be r12");
	  break;

	case OP_RMQRZ:
	case OP_oRMQRZ:
	case OP_RR_ZR:
	  if (!inst.operands[i].iszr && inst.operands[i].reg == REG_PC)
	    inst.error = BAD_PC;
	  break;

	case OP_RRe:
	  if (inst.operands[i].isreg
	      && (inst.operands[i].reg & 0x00000001) != 0)
	    inst.error = BAD_ODD;
	  break;

	case OP_RRo:
	  if (inst.operands[i].isreg)
	    {
	      if ((inst.operands[i].reg & 0x00000001) != 1)
		inst.error = BAD_EVEN;
	      else if (inst.operands[i].reg == REG_SP)
		as_tsktsk (MVE_BAD_SP);
	      else if (inst.operands[i].reg == REG_PC)
		inst.error = BAD_PC;
	    }
	  break;

	default:
	  break;
	}

      /* If we get here, this operand was successfully parsed.	*/
      inst.operands[i].present = 1;
      continue;

    bad_args:
      inst.error = BAD_ARGS;

    failure:
      if (!backtrack_pos)
	{
	  /* The parse routine should already have set inst.error, but set a
	     default here just in case.  */
	  if (!inst.error)
	    inst.error = BAD_SYNTAX;
	  return FAIL;
	}

      /* Do not backtrack over a trailing optional argument that
	 absorbed some text.  We will only fail again, with the
	 'garbage following instruction' error message, which is
	 probably less helpful than the current one.  */
      if (backtrack_index == i && backtrack_pos != str
	  && upat[i+1] == OP_stop)
	{
	  if (!inst.error)
	    inst.error = BAD_SYNTAX;
	  return FAIL;
	}

      /* Try again, skipping the optional argument at backtrack_pos.  */
      str = backtrack_pos;
      inst.error = backtrack_error;
      inst.operands[backtrack_index].present = 0;
      i = backtrack_index;
      backtrack_pos = 0;
    }

  /* Check that we have parsed all the arguments.  */
  if (*str != '\0' && !inst.error)
    inst.error = _("garbage following instruction");

  return inst.error ? FAIL : SUCCESS;
}

#undef po_char_or_fail
#undef po_reg_or_fail
#undef po_reg_or_goto
#undef po_imm_or_fail
#undef po_scalar_or_fail
#undef po_barrier_or_imm

/* Shorthand macro for instruction encoding functions issuing errors.  */
#define constraint(expr, err)			\
  do						\
    {						\
      if (expr)					\
	{					\
	  inst.error = err;			\
	  return;				\
	}					\
    }						\
  while (0)

/* Reject "bad registers" for Thumb-2 instructions.  Many Thumb-2
   instructions are unpredictable if these registers are used.  This
   is the BadReg predicate in ARM's Thumb-2 documentation.

   Before ARMv8-A, REG_PC and REG_SP were not allowed in quite a few
   places, while the restriction on REG_SP was relaxed since ARMv8-A.  */
#define reject_bad_reg(reg)					\
  do								\
   if (reg == REG_PC)						\
     {								\
       inst.error = BAD_PC;					\
       return;							\
     }								\
   else if (reg == REG_SP					\
	    && !ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))	\
     {								\
       inst.error = BAD_SP;					\
       return;							\
     }								\
  while (0)

/* If REG is R13 (the stack pointer), warn that its use is
   deprecated.  */
#define warn_deprecated_sp(reg)			\
  do						\
    if (warn_on_deprecated && reg == REG_SP)	\
       as_tsktsk (_("use of r13 is deprecated"));	\
  while (0)

/* Functions for operand encoding.  ARM, then Thumb.  */

#define rotate_left(v, n) (v << (n & 31) | v >> ((32 - n) & 31))

/* If the current inst is scalar ARMv8.2 fp16 instruction, do special encoding.

   The only binary encoding difference is the Coprocessor number.  Coprocessor
   9 is used for half-precision calculations or conversions.  The format of the
   instruction is the same as the equivalent Coprocessor 10 instruction that
   exists for Single-Precision operation.  */

static void
do_scalar_fp16_v82_encode (void)
{
  if (inst.cond < COND_ALWAYS)
    as_warn (_("scalar fp16 instruction cannot be conditional,"
	       " the behaviour is UNPREDICTABLE"));
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_fp16),
	      _(BAD_FP16));

  inst.instruction = (inst.instruction & 0xfffff0ff) | 0x900;
  mark_feature_used (&arm_ext_fp16);
}

/* If VAL can be encoded in the immediate field of an ARM instruction,
   return the encoded form.  Otherwise, return FAIL.  */

static unsigned int
encode_arm_immediate (unsigned int val)
{
  unsigned int a, i;

  if (val <= 0xff)
    return val;

  for (i = 2; i < 32; i += 2)
    if ((a = rotate_left (val, i)) <= 0xff)
      return a | (i << 7); /* 12-bit pack: [shift-cnt,const].  */

  return FAIL;
}

/* If VAL can be encoded in the immediate field of a Thumb32 instruction,
   return the encoded form.  Otherwise, return FAIL.  */
static unsigned int
encode_thumb32_immediate (unsigned int val)
{
  unsigned int a, i;

  if (val <= 0xff)
    return val;

  for (i = 1; i <= 24; i++)
    {
      a = val >> i;
      if ((val & ~(0xffU << i)) == 0)
	return ((val >> i) & 0x7f) | ((32 - i) << 7);
    }

  a = val & 0xff;
  if (val == ((a << 16) | a))
    return 0x100 | a;
  if (val == ((a << 24) | (a << 16) | (a << 8) | a))
    return 0x300 | a;

  a = val & 0xff00;
  if (val == ((a << 16) | a))
    return 0x200 | (a >> 8);

  return FAIL;
}
/* Encode a VFP SP or DP register number into inst.instruction.  */

static void
encode_arm_vfp_reg (int reg, enum vfp_reg_pos pos)
{
  if ((pos == VFP_REG_Dd || pos == VFP_REG_Dn || pos == VFP_REG_Dm)
      && reg > 15)
    {
      if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_d32))
	{
	  if (thumb_mode)
	    ARM_MERGE_FEATURE_SETS (thumb_arch_used, thumb_arch_used,
				    fpu_vfp_ext_d32);
	  else
	    ARM_MERGE_FEATURE_SETS (arm_arch_used, arm_arch_used,
				    fpu_vfp_ext_d32);
	}
      else
	{
	  first_error (_("D register out of range for selected VFP version"));
	  return;
	}
    }

  switch (pos)
    {
    case VFP_REG_Sd:
      inst.instruction |= ((reg >> 1) << 12) | ((reg & 1) << 22);
      break;

    case VFP_REG_Sn:
      inst.instruction |= ((reg >> 1) << 16) | ((reg & 1) << 7);
      break;

    case VFP_REG_Sm:
      inst.instruction |= ((reg >> 1) << 0) | ((reg & 1) << 5);
      break;

    case VFP_REG_Dd:
      inst.instruction |= ((reg & 15) << 12) | ((reg >> 4) << 22);
      break;

    case VFP_REG_Dn:
      inst.instruction |= ((reg & 15) << 16) | ((reg >> 4) << 7);
      break;

    case VFP_REG_Dm:
      inst.instruction |= (reg & 15) | ((reg >> 4) << 5);
      break;

    default:
      abort ();
    }
}

/* Encode a <shift> in an ARM-format instruction.  The immediate,
   if any, is handled by md_apply_fix.	 */
static void
encode_arm_shift (int i)
{
  /* register-shifted register.  */
  if (inst.operands[i].immisreg)
    {
      int op_index;
      for (op_index = 0; op_index <= i; ++op_index)
	{
	  /* Check the operand only when it's presented.  In pre-UAL syntax,
	     if the destination register is the same as the first operand, two
	     register form of the instruction can be used.  */
	  if (inst.operands[op_index].present && inst.operands[op_index].isreg
	      && inst.operands[op_index].reg == REG_PC)
	    as_warn (UNPRED_REG ("r15"));
	}

      if (inst.operands[i].imm == REG_PC)
	as_warn (UNPRED_REG ("r15"));
    }

  if (inst.operands[i].shift_kind == SHIFT_RRX)
    inst.instruction |= SHIFT_ROR << 5;
  else
    {
      inst.instruction |= inst.operands[i].shift_kind << 5;
      if (inst.operands[i].immisreg)
	{
	  inst.instruction |= SHIFT_BY_REG;
	  inst.instruction |= inst.operands[i].imm << 8;
	}
      else
	inst.relocs[0].type = BFD_RELOC_ARM_SHIFT_IMM;
    }
}

static void
encode_arm_shifter_operand (int i)
{
  if (inst.operands[i].isreg)
    {
      inst.instruction |= inst.operands[i].reg;
      encode_arm_shift (i);
    }
  else
    {
      inst.instruction |= INST_IMMEDIATE;
      if (inst.relocs[0].type != BFD_RELOC_ARM_IMMEDIATE)
	inst.instruction |= inst.operands[i].imm;
    }
}

/* Subroutine of encode_arm_addr_mode_2 and encode_arm_addr_mode_3.  */
static void
encode_arm_addr_mode_common (int i, bool is_t)
{
  /* PR 14260:
     Generate an error if the operand is not a register.  */
  constraint (!inst.operands[i].isreg,
	      _("Instruction does not support =N addresses"));

  inst.instruction |= inst.operands[i].reg << 16;

  if (inst.operands[i].preind)
    {
      if (is_t)
	{
	  inst.error = _("instruction does not accept preindexed addressing");
	  return;
	}
      inst.instruction |= PRE_INDEX;
      if (inst.operands[i].writeback)
	inst.instruction |= WRITE_BACK;

    }
  else if (inst.operands[i].postind)
    {
      gas_assert (inst.operands[i].writeback);
      if (is_t)
	inst.instruction |= WRITE_BACK;
    }
  else /* unindexed - only for coprocessor */
    {
      inst.error = _("instruction does not accept unindexed addressing");
      return;
    }

  if (((inst.instruction & WRITE_BACK) || !(inst.instruction & PRE_INDEX))
      && (((inst.instruction & 0x000f0000) >> 16)
	  == ((inst.instruction & 0x0000f000) >> 12)))
    as_warn ((inst.instruction & LOAD_BIT)
	     ? _("destination register same as write-back base")
	     : _("source register same as write-back base"));
}

/* inst.operands[i] was set up by parse_address.  Encode it into an
   ARM-format mode 2 load or store instruction.	 If is_t is true,
   reject forms that cannot be used with a T instruction (i.e. not
   post-indexed).  */
static void
encode_arm_addr_mode_2 (int i, bool is_t)
{
  const bool is_pc = (inst.operands[i].reg == REG_PC);

  encode_arm_addr_mode_common (i, is_t);

  if (inst.operands[i].immisreg)
    {
      constraint ((inst.operands[i].imm == REG_PC
		   || (is_pc && inst.operands[i].writeback)),
		  BAD_PC_ADDRESSING);
      inst.instruction |= INST_IMMEDIATE;  /* yes, this is backwards */
      inst.instruction |= inst.operands[i].imm;
      if (!inst.operands[i].negative)
	inst.instruction |= INDEX_UP;
      if (inst.operands[i].shifted)
	{
	  if (inst.operands[i].shift_kind == SHIFT_RRX)
	    inst.instruction |= SHIFT_ROR << 5;
	  else
	    {
	      inst.instruction |= inst.operands[i].shift_kind << 5;
	      inst.relocs[0].type = BFD_RELOC_ARM_SHIFT_IMM;
	    }
	}
    }
  else /* immediate offset in inst.relocs[0] */
    {
      if (is_pc && !inst.relocs[0].pc_rel)
	{
	  const bool is_load = ((inst.instruction & LOAD_BIT) != 0);

	  /* If is_t is TRUE, it's called from do_ldstt.  ldrt/strt
	     cannot use PC in addressing.
	     PC cannot be used in writeback addressing, either.  */
	  constraint ((is_t || inst.operands[i].writeback),
		      BAD_PC_ADDRESSING);

	  /* Use of PC in str is deprecated for ARMv7.  */
	  if (warn_on_deprecated
	      && !is_load
	      && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v7))
	    as_tsktsk (_("use of PC in this instruction is deprecated"));
	}

      if (inst.relocs[0].type == BFD_RELOC_UNUSED)
	{
	  /* Prefer + for zero encoded value.  */
	  if (!inst.operands[i].negative)
	    inst.instruction |= INDEX_UP;
	  inst.relocs[0].type = BFD_RELOC_ARM_OFFSET_IMM;
	}
    }
}

/* inst.operands[i] was set up by parse_address.  Encode it into an
   ARM-format mode 3 load or store instruction.	 Reject forms that
   cannot be used with such instructions.  If is_t is true, reject
   forms that cannot be used with a T instruction (i.e. not
   post-indexed).  */
static void
encode_arm_addr_mode_3 (int i, bool is_t)
{
  if (inst.operands[i].immisreg && inst.operands[i].shifted)
    {
      inst.error = _("instruction does not accept scaled register index");
      return;
    }

  encode_arm_addr_mode_common (i, is_t);

  if (inst.operands[i].immisreg)
    {
      constraint ((inst.operands[i].imm == REG_PC
		   || (is_t && inst.operands[i].reg == REG_PC)),
		  BAD_PC_ADDRESSING);
      constraint (inst.operands[i].reg == REG_PC && inst.operands[i].writeback,
		  BAD_PC_WRITEBACK);
      inst.instruction |= inst.operands[i].imm;
      if (!inst.operands[i].negative)
	inst.instruction |= INDEX_UP;
    }
  else /* immediate offset in inst.relocs[0] */
    {
      constraint ((inst.operands[i].reg == REG_PC && !inst.relocs[0].pc_rel
		   && inst.operands[i].writeback),
		  BAD_PC_WRITEBACK);
      inst.instruction |= HWOFFSET_IMM;
      if (inst.relocs[0].type == BFD_RELOC_UNUSED)
	{
	  /* Prefer + for zero encoded value.  */
	  if (!inst.operands[i].negative)
	    inst.instruction |= INDEX_UP;

	  inst.relocs[0].type = BFD_RELOC_ARM_OFFSET_IMM8;
	}
    }
}

/* Write immediate bits [7:0] to the following locations:

  |28/24|23     19|18 16|15                    4|3     0|
  |  a  |x x x x x|b c d|x x x x x x x x x x x x|e f g h|

  This function is used by VMOV/VMVN/VORR/VBIC.  */

static void
neon_write_immbits (unsigned immbits)
{
  inst.instruction |= immbits & 0xf;
  inst.instruction |= ((immbits >> 4) & 0x7) << 16;
  inst.instruction |= ((immbits >> 7) & 0x1) << (thumb_mode ? 28 : 24);
}

/* Invert low-order SIZE bits of XHI:XLO.  */

static void
neon_invert_size (unsigned *xlo, unsigned *xhi, int size)
{
  unsigned immlo = xlo ? *xlo : 0;
  unsigned immhi = xhi ? *xhi : 0;

  switch (size)
    {
    case 8:
      immlo = (~immlo) & 0xff;
      break;

    case 16:
      immlo = (~immlo) & 0xffff;
      break;

    case 64:
      immhi = (~immhi) & 0xffffffff;
      /* fall through.  */

    case 32:
      immlo = (~immlo) & 0xffffffff;
      break;

    default:
      abort ();
    }

  if (xlo)
    *xlo = immlo;

  if (xhi)
    *xhi = immhi;
}

/* True if IMM has form 0bAAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD for bits
   A, B, C, D.  */

static int
neon_bits_same_in_bytes (unsigned imm)
{
  return ((imm & 0x000000ff) == 0 || (imm & 0x000000ff) == 0x000000ff)
	 && ((imm & 0x0000ff00) == 0 || (imm & 0x0000ff00) == 0x0000ff00)
	 && ((imm & 0x00ff0000) == 0 || (imm & 0x00ff0000) == 0x00ff0000)
	 && ((imm & 0xff000000) == 0 || (imm & 0xff000000) == 0xff000000);
}

/* For immediate of above form, return 0bABCD.  */

static unsigned
neon_squash_bits (unsigned imm)
{
  return (imm & 0x01) | ((imm & 0x0100) >> 7) | ((imm & 0x010000) >> 14)
	 | ((imm & 0x01000000) >> 21);
}

/* Compress quarter-float representation to 0b...000 abcdefgh.  */

static unsigned
neon_qfloat_bits (unsigned imm)
{
  return ((imm >> 19) & 0x7f) | ((imm >> 24) & 0x80);
}

/* Returns CMODE. IMMBITS [7:0] is set to bits suitable for inserting into
   the instruction. *OP is passed as the initial value of the op field, and
   may be set to a different value depending on the constant (i.e.
   "MOV I64, 0bAAAAAAAABBBB..." which uses OP = 1 despite being MOV not
   MVN).  If the immediate looks like a repeated pattern then also
   try smaller element sizes.  */

static int
neon_cmode_for_move_imm (unsigned immlo, unsigned immhi, int float_p,
			 unsigned *immbits, int *op, int size,
			 enum neon_el_type type)
{
  /* Only permit float immediates (including 0.0/-0.0) if the operand type is
     float.  */
  if (type == NT_float && !float_p)
    return FAIL;

  if (type == NT_float && is_quarter_float (immlo) && immhi == 0)
    {
      if (size != 32 || *op == 1)
	return FAIL;
      *immbits = neon_qfloat_bits (immlo);
      return 0xf;
    }

  if (size == 64)
    {
      if (neon_bits_same_in_bytes (immhi)
	  && neon_bits_same_in_bytes (immlo))
	{
	  if (*op == 1)
	    return FAIL;
	  *immbits = (neon_squash_bits (immhi) << 4)
		     | neon_squash_bits (immlo);
	  *op = 1;
	  return 0xe;
	}

      if (immhi != immlo)
	return FAIL;
    }

  if (size >= 32)
    {
      if (immlo == (immlo & 0x000000ff))
	{
	  *immbits = immlo;
	  return 0x0;
	}
      else if (immlo == (immlo & 0x0000ff00))
	{
	  *immbits = immlo >> 8;
	  return 0x2;
	}
      else if (immlo == (immlo & 0x00ff0000))
	{
	  *immbits = immlo >> 16;
	  return 0x4;
	}
      else if (immlo == (immlo & 0xff000000))
	{
	  *immbits = immlo >> 24;
	  return 0x6;
	}
      else if (immlo == ((immlo & 0x0000ff00) | 0x000000ff))
	{
	  *immbits = (immlo >> 8) & 0xff;
	  return 0xc;
	}
      else if (immlo == ((immlo & 0x00ff0000) | 0x0000ffff))
	{
	  *immbits = (immlo >> 16) & 0xff;
	  return 0xd;
	}

      if ((immlo & 0xffff) != (immlo >> 16))
	return FAIL;
      immlo &= 0xffff;
    }

  if (size >= 16)
    {
      if (immlo == (immlo & 0x000000ff))
	{
	  *immbits = immlo;
	  return 0x8;
	}
      else if (immlo == (immlo & 0x0000ff00))
	{
	  *immbits = immlo >> 8;
	  return 0xa;
	}

      if ((immlo & 0xff) != (immlo >> 8))
	return FAIL;
      immlo &= 0xff;
    }

  if (immlo == (immlo & 0x000000ff))
    {
      /* Don't allow MVN with 8-bit immediate.  */
      if (*op == 1)
	return FAIL;
      *immbits = immlo;
      return 0xe;
    }

  return FAIL;
}

/* Returns TRUE if double precision value V may be cast
   to single precision without loss of accuracy.  */

static bool
is_double_a_single (uint64_t v)
{
  int exp = (v >> 52) & 0x7FF;
  uint64_t mantissa = v & 0xFFFFFFFFFFFFFULL;

  return ((exp == 0 || exp == 0x7FF
	   || (exp >= 1023 - 126 && exp <= 1023 + 127))
	  && (mantissa & 0x1FFFFFFFL) == 0);
}

/* Returns a double precision value casted to single precision
   (ignoring the least significant bits in exponent and mantissa).  */

static int
double_to_single (uint64_t v)
{
  unsigned int sign = (v >> 63) & 1;
  int exp = (v >> 52) & 0x7FF;
  uint64_t mantissa = v & 0xFFFFFFFFFFFFFULL;

  if (exp == 0x7FF)
    exp = 0xFF;
  else
    {
      exp = exp - 1023 + 127;
      if (exp >= 0xFF)
	{
	  /* Infinity.  */
	  exp = 0x7F;
	  mantissa = 0;
	}
      else if (exp < 0)
	{
	  /* No denormalized numbers.  */
	  exp = 0;
	  mantissa = 0;
	}
    }
  mantissa >>= 29;
  return (sign << 31) | (exp << 23) | mantissa;
}

enum lit_type
{
  CONST_THUMB,
  CONST_ARM,
  CONST_VEC
};

static void do_vfp_nsyn_opcode (const char *);

/* inst.relocs[0].exp describes an "=expr" load pseudo-operation.
   Determine whether it can be performed with a move instruction; if
   it can, convert inst.instruction to that move instruction and
   return true; if it can't, convert inst.instruction to a literal-pool
   load and return FALSE.  If this is not a valid thing to do in the
   current context, set inst.error and return TRUE.

   inst.operands[i] describes the destination register.	 */

static bool
move_or_literal_pool (int i, enum lit_type t, bool mode_3)
{
  unsigned long tbit;
  bool thumb_p = (t == CONST_THUMB);
  bool arm_p   = (t == CONST_ARM);

  if (thumb_p)
    tbit = (inst.instruction > 0xffff) ? THUMB2_LOAD_BIT : THUMB_LOAD_BIT;
  else
    tbit = LOAD_BIT;

  if ((inst.instruction & tbit) == 0)
    {
      inst.error = _("invalid pseudo operation");
      return true;
    }

  if (inst.relocs[0].exp.X_op != O_constant
      && inst.relocs[0].exp.X_op != O_symbol
      && inst.relocs[0].exp.X_op != O_big)
    {
      inst.error = _("constant expression expected");
      return true;
    }

  if (inst.relocs[0].exp.X_op == O_constant
      || inst.relocs[0].exp.X_op == O_big)
    {
      uint64_t v;
      if (inst.relocs[0].exp.X_op == O_big)
	{
	  LITTLENUM_TYPE w[X_PRECISION];
	  LITTLENUM_TYPE * l;

	  if (inst.relocs[0].exp.X_add_number == -1)
	    {
	      gen_to_words (w, X_PRECISION, E_PRECISION);
	      l = w;
	      /* FIXME: Should we check words w[2..5] ?  */
	    }
	  else
	    l = generic_bignum;

	  v = l[3] & LITTLENUM_MASK;
	  v <<= LITTLENUM_NUMBER_OF_BITS;
	  v |= l[2] & LITTLENUM_MASK;
	  v <<= LITTLENUM_NUMBER_OF_BITS;
	  v |= l[1] & LITTLENUM_MASK;
	  v <<= LITTLENUM_NUMBER_OF_BITS;
	  v |= l[0] & LITTLENUM_MASK;
	}
      else
	v = inst.relocs[0].exp.X_add_number;

      if (!inst.operands[i].issingle)
	{
	  if (thumb_p)
	    {
	      /* LDR should not use lead in a flag-setting instruction being
		 chosen so we do not check whether movs can be used.  */

	      if ((ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2)
		  || ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2_v8m))
		  && inst.operands[i].reg != 13
		  && inst.operands[i].reg != 15)
		{
		  /* Check if on thumb2 it can be done with a mov.w, mvn or
		     movw instruction.  */
		  unsigned int newimm;
		  bool isNegated = false;

		  newimm = encode_thumb32_immediate (v);
		  if (newimm == (unsigned int) FAIL)
		    {
		      newimm = encode_thumb32_immediate (~v);
		      isNegated = true;
		    }

		  /* The number can be loaded with a mov.w or mvn
		     instruction.  */
		  if (newimm != (unsigned int) FAIL
		      && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2))
		    {
		      inst.instruction = (0xf04f0000  /*  MOV.W.  */
					  | (inst.operands[i].reg << 8));
		      /* Change to MOVN.  */
		      inst.instruction |= (isNegated ? 0x200000 : 0);
		      inst.instruction |= (newimm & 0x800) << 15;
		      inst.instruction |= (newimm & 0x700) << 4;
		      inst.instruction |= (newimm & 0x0ff);
		      return true;
		    }
		  /* The number can be loaded with a movw instruction.  */
		  else if ((v & ~0xFFFF) == 0
			   && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2_v8m))
		    {
		      int imm = v & 0xFFFF;

		      inst.instruction = 0xf2400000;  /* MOVW.  */
		      inst.instruction |= (inst.operands[i].reg << 8);
		      inst.instruction |= (imm & 0xf000) << 4;
		      inst.instruction |= (imm & 0x0800) << 15;
		      inst.instruction |= (imm & 0x0700) << 4;
		      inst.instruction |= (imm & 0x00ff);
		      /*  In case this replacement is being done on Armv8-M
			  Baseline we need to make sure to disable the
			  instruction size check, as otherwise GAS will reject
			  the use of this T32 instruction.  */
		      inst.size_req = 0;
		      return true;
		    }
		}
	    }
	  else if (arm_p)
	    {
	      int value = encode_arm_immediate (v);

	      if (value != FAIL)
		{
		  /* This can be done with a mov instruction.  */
		  inst.instruction &= LITERAL_MASK;
		  inst.instruction |= INST_IMMEDIATE | (OPCODE_MOV << DATA_OP_SHIFT);
		  inst.instruction |= value & 0xfff;
		  return true;
		}

	      value = encode_arm_immediate (~ v);
	      if (value != FAIL)
		{
		  /* This can be done with a mvn instruction.  */
		  inst.instruction &= LITERAL_MASK;
		  inst.instruction |= INST_IMMEDIATE | (OPCODE_MVN << DATA_OP_SHIFT);
		  inst.instruction |= value & 0xfff;
		  return true;
		}
	    }
	  else if (t == CONST_VEC && ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_v1))
	    {
	      int op = 0;
	      unsigned immbits = 0;
	      unsigned immlo = inst.operands[1].imm;
	      unsigned immhi = inst.operands[1].regisimm
		? inst.operands[1].reg
		: inst.relocs[0].exp.X_unsigned
		? 0
		: (int64_t) (int) immlo >> 32;
	      int cmode = neon_cmode_for_move_imm (immlo, immhi, false, &immbits,
						   &op, 64, NT_invtype);

	      if (cmode == FAIL)
		{
		  neon_invert_size (&immlo, &immhi, 64);
		  op = !op;
		  cmode = neon_cmode_for_move_imm (immlo, immhi, false, &immbits,
						   &op, 64, NT_invtype);
		}

	      if (cmode != FAIL)
		{
		  inst.instruction = (inst.instruction & VLDR_VMOV_SAME)
		    | (1 << 23)
		    | (cmode << 8)
		    | (op << 5)
		    | (1 << 4);

		  /* Fill other bits in vmov encoding for both thumb and arm.  */
		  if (thumb_mode)
		    inst.instruction |= (0x7U << 29) | (0xF << 24);
		  else
		    inst.instruction |= (0xFU << 28) | (0x1 << 25);
		  neon_write_immbits (immbits);
		  return true;
		}
	    }
	}

      if (t == CONST_VEC)
	{
	  /* Check if vldr Rx, =constant could be optimized to vmov Rx, #constant.  */
	  if (inst.operands[i].issingle
	      && is_quarter_float (inst.operands[1].imm)
	      && ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v3xd))
	    {
	      inst.operands[1].imm =
		neon_qfloat_bits (v);
	      do_vfp_nsyn_opcode ("fconsts");
	      return true;
	    }

	  /* If our host does not support a 64-bit type then we cannot perform
	     the following optimization.  This mean that there will be a
	     discrepancy between the output produced by an assembler built for
	     a 32-bit-only host and the output produced from a 64-bit host, but
	     this cannot be helped.  */
	  else if (!inst.operands[1].issingle
		   && ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v3))
	    {
	      if (is_double_a_single (v)
		  && is_quarter_float (double_to_single (v)))
		{
		  inst.operands[1].imm =
		    neon_qfloat_bits (double_to_single (v));
		  do_vfp_nsyn_opcode ("fconstd");
		  return true;
		}
	    }
	}
    }

  if (add_to_lit_pool ((!inst.operands[i].isvec
			|| inst.operands[i].issingle) ? 4 : 8) == FAIL)
    return true;

  inst.operands[1].reg = REG_PC;
  inst.operands[1].isreg = 1;
  inst.operands[1].preind = 1;
  inst.relocs[0].pc_rel = 1;
  inst.relocs[0].type = (thumb_p
		     ? BFD_RELOC_ARM_THUMB_OFFSET
		     : (mode_3
			? BFD_RELOC_ARM_HWLITERAL
			: BFD_RELOC_ARM_LITERAL));
  return false;
}

/* inst.operands[i] was set up by parse_address.  Encode it into an
   ARM-format instruction.  Reject all forms which cannot be encoded
   into a coprocessor load/store instruction.  If wb_ok is false,
   reject use of writeback; if unind_ok is false, reject use of
   unindexed addressing.  If reloc_override is not 0, use it instead
   of BFD_ARM_CP_OFF_IMM, unless the initial relocation is a group one
   (in which case it is preserved).  */

static int
encode_arm_cp_address (int i, int wb_ok, int unind_ok, int reloc_override)
{
  if (!inst.operands[i].isreg)
    {
      /* PR 18256 */
      if (! inst.operands[0].isvec)
	{
	  inst.error = _("invalid co-processor operand");
	  return FAIL;
	}
      if (move_or_literal_pool (0, CONST_VEC, /*mode_3=*/false))
	return SUCCESS;
    }

  inst.instruction |= inst.operands[i].reg << 16;

  gas_assert (!(inst.operands[i].preind && inst.operands[i].postind));

  if (!inst.operands[i].preind && !inst.operands[i].postind) /* unindexed */
    {
      gas_assert (!inst.operands[i].writeback);
      if (!unind_ok)
	{
	  inst.error = _("instruction does not support unindexed addressing");
	  return FAIL;
	}
      inst.instruction |= inst.operands[i].imm;
      inst.instruction |= INDEX_UP;
      return SUCCESS;
    }

  if (inst.operands[i].preind)
    inst.instruction |= PRE_INDEX;

  if (inst.operands[i].writeback)
    {
      if (inst.operands[i].reg == REG_PC)
	{
	  inst.error = _("pc may not be used with write-back");
	  return FAIL;
	}
      if (!wb_ok)
	{
	  inst.error = _("instruction does not support writeback");
	  return FAIL;
	}
      inst.instruction |= WRITE_BACK;
    }

  if (reloc_override)
    inst.relocs[0].type = (bfd_reloc_code_real_type) reloc_override;
  else if ((inst.relocs[0].type < BFD_RELOC_ARM_ALU_PC_G0_NC
	    || inst.relocs[0].type > BFD_RELOC_ARM_LDC_SB_G2)
	   && inst.relocs[0].type != BFD_RELOC_ARM_LDR_PC_G0)
    {
      if (thumb_mode)
	inst.relocs[0].type = BFD_RELOC_ARM_T32_CP_OFF_IMM;
      else
	inst.relocs[0].type = BFD_RELOC_ARM_CP_OFF_IMM;
    }

  /* Prefer + for zero encoded value.  */
  if (!inst.operands[i].negative)
    inst.instruction |= INDEX_UP;

  return SUCCESS;
}

/* Functions for instruction encoding, sorted by sub-architecture.
   First some generics; their names are taken from the conventional
   bit positions for register arguments in ARM format instructions.  */

static void
do_noargs (void)
{
}

static void
do_rd (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
}

static void
do_rn (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
}

static void
do_rd_rm (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg;
}

static void
do_rm_rn (void)
{
  inst.instruction |= inst.operands[0].reg;
  inst.instruction |= inst.operands[1].reg << 16;
}

static void
do_rd_rn (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
}

static void
do_rn_rd (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg << 12;
}

static void
do_tt (void)
{
  inst.instruction |= inst.operands[0].reg << 8;
  inst.instruction |= inst.operands[1].reg << 16;
}

static bool
check_obsolete (const arm_feature_set *feature, const char *msg)
{
  if (ARM_CPU_IS_ANY (cpu_variant))
    {
      as_tsktsk ("%s", msg);
      return true;
    }
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, *feature))
    {
      as_bad ("%s", msg);
      return true;
    }

  return false;
}

static void
do_rd_rm_rn (void)
{
  unsigned Rn = inst.operands[2].reg;
  /* Enforce restrictions on SWP instruction.  */
  if ((inst.instruction & 0x0fbfffff) == 0x01000090)
    {
      constraint (Rn == inst.operands[0].reg || Rn == inst.operands[1].reg,
		  _("Rn must not overlap other operands"));

      /* SWP{b} is obsolete for ARMv8-A, and deprecated for ARMv6* and ARMv7.
       */
      if (!check_obsolete (&arm_ext_v8,
			   _("swp{b} use is obsoleted for ARMv8 and later"))
	  && warn_on_deprecated
	  && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6))
	as_tsktsk (_("swp{b} use is deprecated for ARMv6 and ARMv7"));
    }

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= Rn << 16;
}

static void
do_rd_rn_rm (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
}

static void
do_rm_rd_rn (void)
{
  constraint ((inst.operands[2].reg == REG_PC), BAD_PC);
  constraint (((inst.relocs[0].exp.X_op != O_constant
		&& inst.relocs[0].exp.X_op != O_illegal)
	       || inst.relocs[0].exp.X_add_number != 0),
	      BAD_ADDR_MODE);
  inst.instruction |= inst.operands[0].reg;
  inst.instruction |= inst.operands[1].reg << 12;
  inst.instruction |= inst.operands[2].reg << 16;
}

static void
do_imm0 (void)
{
  inst.instruction |= inst.operands[0].imm;
}

static void
do_rd_cpaddr (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  encode_arm_cp_address (1, true, true, 0);
}

/* ARM instructions, in alphabetical order by function name (except
   that wrapper functions appear immediately after the function they
   wrap).  */

/* This is a pseudo-op of the form "adr rd, label" to be converted
   into a relative address of the form "add rd, pc, #label-.-8".  */

static void
do_adr (void)
{
  inst.instruction |= (inst.operands[0].reg << 12);  /* Rd */

  /* Frag hacking will turn this into a sub instruction if the offset turns
     out to be negative.  */
  inst.relocs[0].type = BFD_RELOC_ARM_IMMEDIATE;
  inst.relocs[0].pc_rel = 1;
  inst.relocs[0].exp.X_add_number -= 8;

  if (support_interwork
      && inst.relocs[0].exp.X_op == O_symbol
      && inst.relocs[0].exp.X_add_symbol != NULL
      && S_IS_DEFINED (inst.relocs[0].exp.X_add_symbol)
      && THUMB_IS_FUNC (inst.relocs[0].exp.X_add_symbol))
    inst.relocs[0].exp.X_add_number |= 1;
}

/* This is a pseudo-op of the form "adrl rd, label" to be converted
   into a relative address of the form:
   add rd, pc, #low(label-.-8)"
   add rd, rd, #high(label-.-8)"  */

static void
do_adrl (void)
{
  inst.instruction |= (inst.operands[0].reg << 12);  /* Rd */

  /* Frag hacking will turn this into a sub instruction if the offset turns
     out to be negative.  */
  inst.relocs[0].type	       = BFD_RELOC_ARM_ADRL_IMMEDIATE;
  inst.relocs[0].pc_rel	       = 1;
  inst.size		       = INSN_SIZE * 2;
  inst.relocs[0].exp.X_add_number -= 8;

  if (support_interwork
      && inst.relocs[0].exp.X_op == O_symbol
      && inst.relocs[0].exp.X_add_symbol != NULL
      && S_IS_DEFINED (inst.relocs[0].exp.X_add_symbol)
      && THUMB_IS_FUNC (inst.relocs[0].exp.X_add_symbol))
    inst.relocs[0].exp.X_add_number |= 1;
}

static void
do_arit (void)
{
  constraint (inst.relocs[0].type >= BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC
	      && inst.relocs[0].type <= BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC ,
	      THUMB1_RELOC_ONLY);
  if (!inst.operands[1].present)
    inst.operands[1].reg = inst.operands[0].reg;
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  encode_arm_shifter_operand (2);
}

static void
do_barrier (void)
{
  if (inst.operands[0].present)
    inst.instruction |= inst.operands[0].imm;
  else
    inst.instruction |= 0xf;
}

static void
do_bfc (void)
{
  unsigned int msb = inst.operands[1].imm + inst.operands[2].imm;
  constraint (msb > 32, _("bit-field extends past end of register"));
  /* The instruction encoding stores the LSB and MSB,
     not the LSB and width.  */
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].imm << 7;
  inst.instruction |= (msb - 1) << 16;
}

static void
do_bfi (void)
{
  unsigned int msb;

  /* #0 in second position is alternative syntax for bfc, which is
     the same instruction but with REG_PC in the Rm field.  */
  if (!inst.operands[1].isreg)
    inst.operands[1].reg = REG_PC;

  msb = inst.operands[2].imm + inst.operands[3].imm;
  constraint (msb > 32, _("bit-field extends past end of register"));
  /* The instruction encoding stores the LSB and MSB,
     not the LSB and width.  */
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].imm << 7;
  inst.instruction |= (msb - 1) << 16;
}

static void
do_bfx (void)
{
  constraint (inst.operands[2].imm + inst.operands[3].imm > 32,
	      _("bit-field extends past end of register"));
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].imm << 7;
  inst.instruction |= (inst.operands[3].imm - 1) << 16;
}

/* ARM V5 breakpoint instruction (argument parse)
     BKPT <16 bit unsigned immediate>
     Instruction is not conditional.
	The bit pattern given in insns[] has the COND_ALWAYS condition,
	and it is an error if the caller tried to override that.  */

static void
do_bkpt (void)
{
  /* Top 12 of 16 bits to bits 19:8.  */
  inst.instruction |= (inst.operands[0].imm & 0xfff0) << 4;

  /* Bottom 4 of 16 bits to bits 3:0.  */
  inst.instruction |= inst.operands[0].imm & 0xf;
}

static void
encode_branch (int default_reloc)
{
  if (inst.operands[0].hasreloc)
    {
      constraint (inst.operands[0].imm != BFD_RELOC_ARM_PLT32
		  && inst.operands[0].imm != BFD_RELOC_ARM_TLS_CALL,
		  _("the only valid suffixes here are '(plt)' and '(tlscall)'"));
      inst.relocs[0].type = inst.operands[0].imm == BFD_RELOC_ARM_PLT32
	? BFD_RELOC_ARM_PLT32
	: thumb_mode ? BFD_RELOC_ARM_THM_TLS_CALL : BFD_RELOC_ARM_TLS_CALL;
    }
  else
    inst.relocs[0].type = (bfd_reloc_code_real_type) default_reloc;
  inst.relocs[0].pc_rel = 1;
}

static void
do_branch (void)
{
#ifdef OBJ_ELF
  if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4)
    encode_branch (BFD_RELOC_ARM_PCREL_JUMP);
  else
#endif
    encode_branch (BFD_RELOC_ARM_PCREL_BRANCH);
}

static void
do_bl (void)
{
#ifdef OBJ_ELF
  if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4)
    {
      if (inst.cond == COND_ALWAYS)
	encode_branch (BFD_RELOC_ARM_PCREL_CALL);
      else
	encode_branch (BFD_RELOC_ARM_PCREL_JUMP);
    }
  else
#endif
    encode_branch (BFD_RELOC_ARM_PCREL_BRANCH);
}

/* ARM V5 branch-link-exchange instruction (argument parse)
     BLX <target_addr>		ie BLX(1)
     BLX{<condition>} <Rm>	ie BLX(2)
   Unfortunately, there are two different opcodes for this mnemonic.
   So, the insns[].value is not used, and the code here zaps values
	into inst.instruction.
   Also, the <target_addr> can be 25 bits, hence has its own reloc.  */

static void
do_blx (void)
{
  if (inst.operands[0].isreg)
    {
      /* Arg is a register; the opcode provided by insns[] is correct.
	 It is not illegal to do "blx pc", just useless.  */
      if (inst.operands[0].reg == REG_PC)
	as_tsktsk (_("use of r15 in blx in ARM mode is not really useful"));

      inst.instruction |= inst.operands[0].reg;
    }
  else
    {
      /* Arg is an address; this instruction cannot be executed
	 conditionally, and the opcode must be adjusted.
	 We retain the BFD_RELOC_ARM_PCREL_BLX till the very end
	 where we generate out a BFD_RELOC_ARM_PCREL_CALL instead.  */
      constraint (inst.cond != COND_ALWAYS, BAD_COND);
      inst.instruction = 0xfa000000;
      encode_branch (BFD_RELOC_ARM_PCREL_BLX);
    }
}

static void
do_bx (void)
{
  bool want_reloc;

  if (inst.operands[0].reg == REG_PC)
    as_tsktsk (_("use of r15 in bx in ARM mode is not really useful"));

  inst.instruction |= inst.operands[0].reg;
  /* Output R_ARM_V4BX relocations if is an EABI object that looks like
     it is for ARMv4t or earlier.  */
  want_reloc = !ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5);
  if (!ARM_FEATURE_ZERO (selected_object_arch)
      && !ARM_CPU_HAS_FEATURE (selected_object_arch, arm_ext_v5))
      want_reloc = true;

#ifdef OBJ_ELF
  if (EF_ARM_EABI_VERSION (meabi_flags) < EF_ARM_EABI_VER4)
#endif
    want_reloc = false;

  if (want_reloc)
    inst.relocs[0].type = BFD_RELOC_ARM_V4BX;
}


/* ARM v5TEJ.  Jump to Jazelle code.  */

static void
do_bxj (void)
{
  if (inst.operands[0].reg == REG_PC)
    as_tsktsk (_("use of r15 in bxj is not really useful"));

  inst.instruction |= inst.operands[0].reg;
}

/* Co-processor data operation:
      CDP{cond} <coproc>, <opcode_1>, <CRd>, <CRn>, <CRm>{, <opcode_2>}
      CDP2	<coproc>, <opcode_1>, <CRd>, <CRn>, <CRm>{, <opcode_2>}	 */
static void
do_cdp (void)
{
  inst.instruction |= inst.operands[0].reg << 8;
  inst.instruction |= inst.operands[1].imm << 20;
  inst.instruction |= inst.operands[2].reg << 12;
  inst.instruction |= inst.operands[3].reg << 16;
  inst.instruction |= inst.operands[4].reg;
  inst.instruction |= inst.operands[5].imm << 5;
}

static void
do_cmp (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  encode_arm_shifter_operand (1);
}

/* Transfer between coprocessor and ARM registers.
   MRC{cond} <coproc>, <opcode_1>, <Rd>, <CRn>, <CRm>{, <opcode_2>}
   MRC2
   MCR{cond}
   MCR2

   No special properties.  */

struct deprecated_coproc_regs_s
{
  unsigned cp;
  int opc1;
  unsigned crn;
  unsigned crm;
  int opc2;
  arm_feature_set deprecated;
  arm_feature_set obsoleted;
  const char *dep_msg;
  const char *obs_msg;
};

#define DEPR_ACCESS_V8 \
  N_("This coprocessor register access is deprecated in ARMv8")

/* Table of all deprecated coprocessor registers.  */
static struct deprecated_coproc_regs_s deprecated_coproc_regs[] =
{
    {15, 0, 7, 10, 5,					/* CP15DMB.  */
     ARM_FEATURE_CORE_LOW (ARM_EXT_V8), ARM_ARCH_NONE,
     DEPR_ACCESS_V8, NULL},
    {15, 0, 7, 10, 4,					/* CP15DSB.  */
     ARM_FEATURE_CORE_LOW (ARM_EXT_V8), ARM_ARCH_NONE,
     DEPR_ACCESS_V8, NULL},
    {15, 0, 7,  5, 4,					/* CP15ISB.  */
     ARM_FEATURE_CORE_LOW (ARM_EXT_V8), ARM_ARCH_NONE,
     DEPR_ACCESS_V8, NULL},
    {14, 6, 1,  0, 0,					/* TEEHBR.  */
     ARM_FEATURE_CORE_LOW (ARM_EXT_V8), ARM_ARCH_NONE,
     DEPR_ACCESS_V8, NULL},
    {14, 6, 0,  0, 0,					/* TEECR.  */
     ARM_FEATURE_CORE_LOW (ARM_EXT_V8), ARM_ARCH_NONE,
     DEPR_ACCESS_V8, NULL},
};

#undef DEPR_ACCESS_V8

static const size_t deprecated_coproc_reg_count =
  sizeof (deprecated_coproc_regs) / sizeof (deprecated_coproc_regs[0]);

static void
do_co_reg (void)
{
  unsigned Rd;
  size_t i;

  Rd = inst.operands[2].reg;
  if (thumb_mode)
    {
      if (inst.instruction == 0xee000010
	  || inst.instruction == 0xfe000010)
	/* MCR, MCR2  */
	reject_bad_reg (Rd);
      else if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))
	/* MRC, MRC2  */
	constraint (Rd == REG_SP, BAD_SP);
    }
  else
    {
      /* MCR */
      if (inst.instruction == 0xe000010)
	constraint (Rd == REG_PC, BAD_PC);
    }

    for (i = 0; i < deprecated_coproc_reg_count; ++i)
      {
	const struct deprecated_coproc_regs_s *r =
	  deprecated_coproc_regs + i;

	if (inst.operands[0].reg == r->cp
	    && inst.operands[1].imm == r->opc1
	    && inst.operands[3].reg == r->crn
	    && inst.operands[4].reg == r->crm
	    && inst.operands[5].imm == r->opc2)
	  {
	    if (! ARM_CPU_IS_ANY (cpu_variant)
		&& warn_on_deprecated
		&& ARM_CPU_HAS_FEATURE (cpu_variant, r->deprecated))
	      as_tsktsk ("%s", r->dep_msg);
	  }
      }

  inst.instruction |= inst.operands[0].reg << 8;
  inst.instruction |= inst.operands[1].imm << 21;
  inst.instruction |= Rd << 12;
  inst.instruction |= inst.operands[3].reg << 16;
  inst.instruction |= inst.operands[4].reg;
  inst.instruction |= inst.operands[5].imm << 5;
}

/* Transfer between coprocessor register and pair of ARM registers.
   MCRR{cond} <coproc>, <opcode>, <Rd>, <Rn>, <CRm>.
   MCRR2
   MRRC{cond}
   MRRC2

   Two XScale instructions are special cases of these:

     MAR{cond} acc0, <RdLo>, <RdHi> == MCRR{cond} p0, #0, <RdLo>, <RdHi>, c0
     MRA{cond} acc0, <RdLo>, <RdHi> == MRRC{cond} p0, #0, <RdLo>, <RdHi>, c0

   Result unpredictable if Rd or Rn is R15.  */

static void
do_co_reg2c (void)
{
  unsigned Rd, Rn;

  Rd = inst.operands[2].reg;
  Rn = inst.operands[3].reg;

  if (thumb_mode)
    {
      reject_bad_reg (Rd);
      reject_bad_reg (Rn);
    }
  else
    {
      constraint (Rd == REG_PC, BAD_PC);
      constraint (Rn == REG_PC, BAD_PC);
    }

  /* Only check the MRRC{2} variants.  */
  if ((inst.instruction & 0x0FF00000) == 0x0C500000)
    {
       /* If Rd == Rn, error that the operation is
	  unpredictable (example MRRC p3,#1,r1,r1,c4).  */
       constraint (Rd == Rn, BAD_OVERLAP);
    }

  inst.instruction |= inst.operands[0].reg << 8;
  inst.instruction |= inst.operands[1].imm << 4;
  inst.instruction |= Rd << 12;
  inst.instruction |= Rn << 16;
  inst.instruction |= inst.operands[4].reg;
}

static void
do_cpsi (void)
{
  inst.instruction |= inst.operands[0].imm << 6;
  if (inst.operands[1].present)
    {
      inst.instruction |= CPSI_MMOD;
      inst.instruction |= inst.operands[1].imm;
    }
}

static void
do_dbg (void)
{
  inst.instruction |= inst.operands[0].imm;
}

static void
do_div (void)
{
  unsigned Rd, Rn, Rm;

  Rd = inst.operands[0].reg;
  Rn = (inst.operands[1].present
	? inst.operands[1].reg : Rd);
  Rm = inst.operands[2].reg;

  constraint ((Rd == REG_PC), BAD_PC);
  constraint ((Rn == REG_PC), BAD_PC);
  constraint ((Rm == REG_PC), BAD_PC);

  inst.instruction |= Rd << 16;
  inst.instruction |= Rn << 0;
  inst.instruction |= Rm << 8;
}

static void
do_it (void)
{
  /* There is no IT instruction in ARM mode.  We
     process it to do the validation as if in
     thumb mode, just in case the code gets
     assembled for thumb using the unified syntax.  */

  inst.size = 0;
  if (unified_syntax)
    {
      set_pred_insn_type (IT_INSN);
      now_pred.mask = (inst.instruction & 0xf) | 0x10;
      now_pred.cc = inst.operands[0].imm;
    }
}

/* If there is only one register in the register list,
   then return its register number.  Otherwise return -1.  */
static int
only_one_reg_in_list (int range)
{
  int i = ffs (range) - 1;
  return (i > 15 || range != (1 << i)) ? -1 : i;
}

static void
encode_ldmstm(int from_push_pop_mnem)
{
  int base_reg = inst.operands[0].reg;
  int range = inst.operands[1].imm;
  int one_reg;

  inst.instruction |= base_reg << 16;
  inst.instruction |= range;

  if (inst.operands[1].writeback)
    inst.instruction |= LDM_TYPE_2_OR_3;

  if (inst.operands[0].writeback)
    {
      inst.instruction |= WRITE_BACK;
      /* Check for unpredictable uses of writeback.  */
      if (inst.instruction & LOAD_BIT)
	{
	  /* Not allowed in LDM type 2.	 */
	  if ((inst.instruction & LDM_TYPE_2_OR_3)
	      && ((range & (1 << REG_PC)) == 0))
	    as_warn (_("writeback of base register is UNPREDICTABLE"));
	  /* Only allowed if base reg not in list for other types.  */
	  else if (range & (1 << base_reg))
	    as_warn (_("writeback of base register when in register list is UNPREDICTABLE"));
	}
      else /* STM.  */
	{
	  /* Not allowed for type 2.  */
	  if (inst.instruction & LDM_TYPE_2_OR_3)
	    as_warn (_("writeback of base register is UNPREDICTABLE"));
	  /* Only allowed if base reg not in list, or first in list.  */
	  else if ((range & (1 << base_reg))
		   && (range & ((1 << base_reg) - 1)))
	    as_warn (_("if writeback register is in list, it must be the lowest reg in the list"));
	}
    }

  /* If PUSH/POP has only one register, then use the A2 encoding.  */
  one_reg = only_one_reg_in_list (range);
  if (from_push_pop_mnem && one_reg >= 0)
    {
      int is_push = (inst.instruction & A_PUSH_POP_OP_MASK) == A1_OPCODE_PUSH;

      if (is_push && one_reg == 13 /* SP */)
	/* PR 22483: The A2 encoding cannot be used when
	   pushing the stack pointer as this is UNPREDICTABLE.  */
	return;

      inst.instruction &= A_COND_MASK;
      inst.instruction |= is_push ? A2_OPCODE_PUSH : A2_OPCODE_POP;
      inst.instruction |= one_reg << 12;
    }
}

static void
do_ldmstm (void)
{
  encode_ldmstm (/*from_push_pop_mnem=*/false);
}

/* ARMv5TE load-consecutive (argument parse)
   Mode is like LDRH.

     LDRccD R, mode
     STRccD R, mode.  */

static void
do_ldrd (void)
{
  constraint (inst.operands[0].reg % 2 != 0,
	      _("first transfer register must be even"));
  constraint (inst.operands[1].present
	      && inst.operands[1].reg != inst.operands[0].reg + 1,
	      _("can only transfer two consecutive registers"));
  constraint (inst.operands[0].reg == REG_LR, _("r14 not allowed here"));
  constraint (!inst.operands[2].isreg, _("'[' expected"));

  if (!inst.operands[1].present)
    inst.operands[1].reg = inst.operands[0].reg + 1;

  /* encode_arm_addr_mode_3 will diagnose overlap between the base
     register and the first register written; we have to diagnose
     overlap between the base and the second register written here.  */

  if (inst.operands[2].reg == inst.operands[1].reg
      && (inst.operands[2].writeback || inst.operands[2].postind))
    as_warn (_("base register written back, and overlaps "
	       "second transfer register"));

  if (!(inst.instruction & V4_STR_BIT))
    {
      /* For an index-register load, the index register must not overlap the
	destination (even if not write-back).  */
      if (inst.operands[2].immisreg
	      && ((unsigned) inst.operands[2].imm == inst.operands[0].reg
	      || (unsigned) inst.operands[2].imm == inst.operands[1].reg))
	as_warn (_("index register overlaps transfer register"));
    }
  inst.instruction |= inst.operands[0].reg << 12;
  encode_arm_addr_mode_3 (2, /*is_t=*/false);
}

static void
do_ldrex (void)
{
  constraint (!inst.operands[1].isreg || !inst.operands[1].preind
	      || inst.operands[1].postind || inst.operands[1].writeback
	      || inst.operands[1].immisreg || inst.operands[1].shifted
	      || inst.operands[1].negative
	      /* This can arise if the programmer has written
		   strex rN, rM, foo
		 or if they have mistakenly used a register name as the last
		 operand,  eg:
		   strex rN, rM, rX
		 It is very difficult to distinguish between these two cases
		 because "rX" might actually be a label. ie the register
		 name has been occluded by a symbol of the same name. So we
		 just generate a general 'bad addressing mode' type error
		 message and leave it up to the programmer to discover the
		 true cause and fix their mistake.  */
	      || (inst.operands[1].reg == REG_PC),
	      BAD_ADDR_MODE);

  constraint (inst.relocs[0].exp.X_op != O_constant
	      || inst.relocs[0].exp.X_add_number != 0,
	      _("offset must be zero in ARM encoding"));

  constraint ((inst.operands[1].reg == REG_PC), BAD_PC);

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.relocs[0].type = BFD_RELOC_UNUSED;
}

static void
do_ldrexd (void)
{
  constraint (inst.operands[0].reg % 2 != 0,
	      _("even register required"));
  constraint (inst.operands[1].present
	      && inst.operands[1].reg != inst.operands[0].reg + 1,
	      _("can only load two consecutive registers"));
  /* If op 1 were present and equal to PC, this function wouldn't
     have been called in the first place.  */
  constraint (inst.operands[0].reg == REG_LR, _("r14 not allowed here"));

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[2].reg << 16;
}

/* In both ARM and thumb state 'ldr pc, #imm'  with an immediate
   which is not a multiple of four is UNPREDICTABLE.  */
static void
check_ldr_r15_aligned (void)
{
  constraint (!(inst.operands[1].immisreg)
	      && (inst.operands[0].reg == REG_PC
	      && inst.operands[1].reg == REG_PC
	      && (inst.relocs[0].exp.X_add_number & 0x3)),
	      _("ldr to register 15 must be 4-byte aligned"));
}

static void
do_ldst (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  if (!inst.operands[1].isreg)
    if (move_or_literal_pool (0, CONST_ARM, /*mode_3=*/false))
      return;
  encode_arm_addr_mode_2 (1, /*is_t=*/false);
  check_ldr_r15_aligned ();
}

static void
do_ldstt (void)
{
  /* ldrt/strt always use post-indexed addressing.  Turn [Rn] into [Rn]! and
     reject [Rn,...].  */
  if (inst.operands[1].preind)
    {
      constraint (inst.relocs[0].exp.X_op != O_constant
		  || inst.relocs[0].exp.X_add_number != 0,
		  _("this instruction requires a post-indexed address"));

      inst.operands[1].preind = 0;
      inst.operands[1].postind = 1;
      inst.operands[1].writeback = 1;
    }
  inst.instruction |= inst.operands[0].reg << 12;
  encode_arm_addr_mode_2 (1, /*is_t=*/true);
}

/* Halfword and signed-byte load/store operations.  */

static void
do_ldstv4 (void)
{
  constraint (inst.operands[0].reg == REG_PC, BAD_PC);
  inst.instruction |= inst.operands[0].reg << 12;
  if (!inst.operands[1].isreg)
    if (move_or_literal_pool (0, CONST_ARM, /*mode_3=*/true))
      return;
  encode_arm_addr_mode_3 (1, /*is_t=*/false);
}

static void
do_ldsttv4 (void)
{
  /* ldrt/strt always use post-indexed addressing.  Turn [Rn] into [Rn]! and
     reject [Rn,...].  */
  if (inst.operands[1].preind)
    {
      constraint (inst.relocs[0].exp.X_op != O_constant
		  || inst.relocs[0].exp.X_add_number != 0,
		  _("this instruction requires a post-indexed address"));

      inst.operands[1].preind = 0;
      inst.operands[1].postind = 1;
      inst.operands[1].writeback = 1;
    }
  inst.instruction |= inst.operands[0].reg << 12;
  encode_arm_addr_mode_3 (1, /*is_t=*/true);
}

/* Co-processor register load/store.
   Format: <LDC|STC>{cond}[L] CP#,CRd,<address>	 */
static void
do_lstc (void)
{
  inst.instruction |= inst.operands[0].reg << 8;
  inst.instruction |= inst.operands[1].reg << 12;
  encode_arm_cp_address (2, true, true, 0);
}

static void
do_mlas (void)
{
  /* This restriction does not apply to mls (nor to mla in v6 or later).  */
  if (inst.operands[0].reg == inst.operands[1].reg
      && !ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6)
      && !(inst.instruction & 0x00400000))
    as_tsktsk (_("Rd and Rm should be different in mla"));

  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].reg << 8;
  inst.instruction |= inst.operands[3].reg << 12;
}

static void
do_mov (void)
{
  constraint (inst.relocs[0].type >= BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC
	      && inst.relocs[0].type <= BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC ,
	      THUMB1_RELOC_ONLY);
  inst.instruction |= inst.operands[0].reg << 12;
  encode_arm_shifter_operand (1);
}

/* ARM V6T2 16-bit immediate register load: MOV[WT]{cond} Rd, #<imm16>.	 */
static void
do_mov16 (void)
{
  bfd_vma imm;
  bool top;

  top = (inst.instruction & 0x00400000) != 0;
  constraint (top && inst.relocs[0].type == BFD_RELOC_ARM_MOVW,
	      _(":lower16: not allowed in this instruction"));
  constraint (!top && inst.relocs[0].type == BFD_RELOC_ARM_MOVT,
	      _(":upper16: not allowed in this instruction"));
  inst.instruction |= inst.operands[0].reg << 12;
  if (inst.relocs[0].type == BFD_RELOC_UNUSED)
    {
      imm = inst.relocs[0].exp.X_add_number;
      /* The value is in two pieces: 0:11, 16:19.  */
      inst.instruction |= (imm & 0x00000fff);
      inst.instruction |= (imm & 0x0000f000) << 4;
    }
}

static int
do_vfp_nsyn_mrs (void)
{
  if (inst.operands[0].isvec)
    {
      if (inst.operands[1].reg != 1)
	first_error (_("operand 1 must be FPSCR"));
      memset (&inst.operands[0], '\0', sizeof (inst.operands[0]));
      memset (&inst.operands[1], '\0', sizeof (inst.operands[1]));
      do_vfp_nsyn_opcode ("fmstat");
    }
  else if (inst.operands[1].isvec)
    do_vfp_nsyn_opcode ("fmrx");
  else
    return FAIL;

  return SUCCESS;
}

static int
do_vfp_nsyn_msr (void)
{
  if (inst.operands[0].isvec)
    do_vfp_nsyn_opcode ("fmxr");
  else
    return FAIL;

  return SUCCESS;
}

static void
do_vmrs (void)
{
  unsigned Rt = inst.operands[0].reg;

  if (thumb_mode && Rt == REG_SP)
    {
      inst.error = BAD_SP;
      return;
    }

  switch (inst.operands[1].reg)
    {
    /* MVFR2 is only valid for Armv8-A.  */
    case 5:
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_armv8),
		  _(BAD_FPU));
      break;

    /* Check for new Armv8.1-M Mainline changes to <spec_reg>.  */
    case 1: /* fpscr.  */
      constraint (!(ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
		    || ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)),
		  _(BAD_FPU));
      break;

    case 14: /* fpcxt_ns, fpcxtns, FPCXT_NS, FPCXTNS.  */
    case 15: /* fpcxt_s, fpcxts, FPCXT_S, FPCXTS.  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8_1m_main),
		  _("selected processor does not support instruction"));
      break;

    case  2: /* fpscr_nzcvqc.  */
    case 12: /* vpr.  */
    case 13: /* p0.  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8_1m_main)
		  || (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
		      && !ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)),
		  _("selected processor does not support instruction"));
      if (inst.operands[0].reg != 2
	  && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	as_warn (_("accessing MVE system register without MVE is UNPREDICTABLE"));
      break;

    default:
      break;
    }

  /* APSR_ sets isvec. All other refs to PC are illegal.  */
  if (!inst.operands[0].isvec && Rt == REG_PC)
    {
      inst.error = BAD_PC;
      return;
    }

  /* If we get through parsing the register name, we just insert the number
     generated into the instruction without further validation.  */
  inst.instruction |= (inst.operands[1].reg << 16);
  inst.instruction |= (Rt << 12);
}

static void
do_vmsr (void)
{
  unsigned Rt = inst.operands[1].reg;

  if (thumb_mode)
    reject_bad_reg (Rt);
  else if (Rt == REG_PC)
    {
      inst.error = BAD_PC;
      return;
    }

  switch (inst.operands[0].reg)
    {
    /* MVFR2 is only valid for Armv8-A.  */
    case 5:
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_armv8),
		  _(BAD_FPU));
      break;

    /* Check for new Armv8.1-M Mainline changes to <spec_reg>.  */
    case  1: /* fpcr.  */
      constraint (!(ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
		    || ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)),
		  _(BAD_FPU));
      break;

    case 14: /* fpcxt_ns.  */
    case 15: /* fpcxt_s.  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8_1m_main),
		  _("selected processor does not support instruction"));
      break;

    case  2: /* fpscr_nzcvqc.  */
    case 12: /* vpr.  */
    case 13: /* p0.  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8_1m_main)
		  || (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
		      && !ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)),
		  _("selected processor does not support instruction"));
      if (inst.operands[0].reg != 2
	  && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	as_warn (_("accessing MVE system register without MVE is UNPREDICTABLE"));
      break;

    default:
      break;
    }

  /* If we get through parsing the register name, we just insert the number
     generated into the instruction without further validation.  */
  inst.instruction |= (inst.operands[0].reg << 16);
  inst.instruction |= (Rt << 12);
}

static void
do_mrs (void)
{
  unsigned br;

  if (do_vfp_nsyn_mrs () == SUCCESS)
    return;

  constraint (inst.operands[0].reg == REG_PC, BAD_PC);
  inst.instruction |= inst.operands[0].reg << 12;

  if (inst.operands[1].isreg)
    {
      br = inst.operands[1].reg;
      if (((br & 0x200) == 0) && ((br & 0xf0000) != 0xf0000))
	as_bad (_("bad register for mrs"));
    }
  else
    {
      /* mrs only accepts CPSR/SPSR/CPSR_all/SPSR_all.  */
      constraint ((inst.operands[1].imm & (PSR_c|PSR_x|PSR_s|PSR_f))
		  != (PSR_c|PSR_f),
		  _("'APSR', 'CPSR' or 'SPSR' expected"));
      br = (15<<16) | (inst.operands[1].imm & SPSR_BIT);
    }

  inst.instruction |= br;
}

/* Two possible forms:
      "{C|S}PSR_<field>, Rm",
      "{C|S}PSR_f, #expression".  */

static void
do_msr (void)
{
  if (do_vfp_nsyn_msr () == SUCCESS)
    return;

  inst.instruction |= inst.operands[0].imm;
  if (inst.operands[1].isreg)
    inst.instruction |= inst.operands[1].reg;
  else
    {
      inst.instruction |= INST_IMMEDIATE;
      inst.relocs[0].type = BFD_RELOC_ARM_IMMEDIATE;
      inst.relocs[0].pc_rel = 0;
    }
}

static void
do_mul (void)
{
  constraint (inst.operands[2].reg == REG_PC, BAD_PC);

  if (!inst.operands[2].present)
    inst.operands[2].reg = inst.operands[0].reg;
  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].reg << 8;

  if (inst.operands[0].reg == inst.operands[1].reg
      && !ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6))
    as_tsktsk (_("Rd and Rm should be different in mul"));
}

/* Long Multiply Parser
   UMULL RdLo, RdHi, Rm, Rs
   SMULL RdLo, RdHi, Rm, Rs
   UMLAL RdLo, RdHi, Rm, Rs
   SMLAL RdLo, RdHi, Rm, Rs.  */

static void
do_mull (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
  inst.instruction |= inst.operands[3].reg << 8;

  /* rdhi and rdlo must be different.  */
  if (inst.operands[0].reg == inst.operands[1].reg)
    as_tsktsk (_("rdhi and rdlo must be different"));

  /* rdhi, rdlo and rm must all be different before armv6.  */
  if ((inst.operands[0].reg == inst.operands[2].reg
      || inst.operands[1].reg == inst.operands[2].reg)
      && !ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6))
    as_tsktsk (_("rdhi, rdlo and rm must all be different"));
}

static void
do_nop (void)
{
  if (inst.operands[0].present
      || ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6k))
    {
      /* Architectural NOP hints are CPSR sets with no bits selected.  */
      inst.instruction &= 0xf0000000;
      inst.instruction |= 0x0320f000;
      if (inst.operands[0].present)
	inst.instruction |= inst.operands[0].imm;
    }
}

/* ARM V6 Pack Halfword Bottom Top instruction (argument parse).
   PKHBT {<cond>} <Rd>, <Rn>, <Rm> {, LSL #<shift_imm>}
   Condition defaults to COND_ALWAYS.
   Error if Rd, Rn or Rm are R15.  */

static void
do_pkhbt (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
  if (inst.operands[3].present)
    encode_arm_shift (3);
}

/* ARM V6 PKHTB (Argument Parse).  */

static void
do_pkhtb (void)
{
  if (!inst.operands[3].present)
    {
      /* If the shift specifier is omitted, turn the instruction
	 into pkhbt rd, rm, rn. */
      inst.instruction &= 0xfff00010;
      inst.instruction |= inst.operands[0].reg << 12;
      inst.instruction |= inst.operands[1].reg;
      inst.instruction |= inst.operands[2].reg << 16;
    }
  else
    {
      inst.instruction |= inst.operands[0].reg << 12;
      inst.instruction |= inst.operands[1].reg << 16;
      inst.instruction |= inst.operands[2].reg;
      encode_arm_shift (3);
    }
}

/* ARMv5TE: Preload-Cache
   MP Extensions: Preload for write

    PLD(W) <addr_mode>

  Syntactically, like LDR with B=1, W=0, L=1.  */

static void
do_pld (void)
{
  constraint (!inst.operands[0].isreg,
	      _("'[' expected after PLD mnemonic"));
  constraint (inst.operands[0].postind,
	      _("post-indexed expression used in preload instruction"));
  constraint (inst.operands[0].writeback,
	      _("writeback used in preload instruction"));
  constraint (!inst.operands[0].preind,
	      _("unindexed addressing used in preload instruction"));
  encode_arm_addr_mode_2 (0, /*is_t=*/false);
}

/* ARMv7: PLI <addr_mode>  */
static void
do_pli (void)
{
  constraint (!inst.operands[0].isreg,
	      _("'[' expected after PLI mnemonic"));
  constraint (inst.operands[0].postind,
	      _("post-indexed expression used in preload instruction"));
  constraint (inst.operands[0].writeback,
	      _("writeback used in preload instruction"));
  constraint (!inst.operands[0].preind,
	      _("unindexed addressing used in preload instruction"));
  encode_arm_addr_mode_2 (0, /*is_t=*/false);
  inst.instruction &= ~PRE_INDEX;
}

static void
do_push_pop (void)
{
  constraint (inst.operands[0].writeback,
	      _("push/pop do not support {reglist}^"));
  inst.operands[1] = inst.operands[0];
  memset (&inst.operands[0], 0, sizeof inst.operands[0]);
  inst.operands[0].isreg = 1;
  inst.operands[0].writeback = 1;
  inst.operands[0].reg = REG_SP;
  encode_ldmstm (/*from_push_pop_mnem=*/true);
}

/* ARM V6 RFE (Return from Exception) loads the PC and CPSR from the
   word at the specified address and the following word
   respectively.
   Unconditionally executed.
   Error if Rn is R15.	*/

static void
do_rfe (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  if (inst.operands[0].writeback)
    inst.instruction |= WRITE_BACK;
}

/* ARM V6 ssat (argument parse).  */

static void
do_ssat (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= (inst.operands[1].imm - 1) << 16;
  inst.instruction |= inst.operands[2].reg;

  if (inst.operands[3].present)
    encode_arm_shift (3);
}

/* ARM V6 usat (argument parse).  */

static void
do_usat (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].imm << 16;
  inst.instruction |= inst.operands[2].reg;

  if (inst.operands[3].present)
    encode_arm_shift (3);
}

/* ARM V6 ssat16 (argument parse).  */

static void
do_ssat16 (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= ((inst.operands[1].imm - 1) << 16);
  inst.instruction |= inst.operands[2].reg;
}

static void
do_usat16 (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].imm << 16;
  inst.instruction |= inst.operands[2].reg;
}

/* ARM V6 SETEND (argument parse).  Sets the E bit in the CPSR while
   preserving the other bits.

   setend <endian_specifier>, where <endian_specifier> is either
   BE or LE.  */

static void
do_setend (void)
{
  if (warn_on_deprecated
      && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))
      as_tsktsk (_("setend use is deprecated for ARMv8"));

  if (inst.operands[0].imm)
    inst.instruction |= 0x200;
}

static void
do_shift (void)
{
  unsigned int Rm = (inst.operands[1].present
		     ? inst.operands[1].reg
		     : inst.operands[0].reg);

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= Rm;
  if (inst.operands[2].isreg)  /* Rd, {Rm,} Rs */
    {
      inst.instruction |= inst.operands[2].reg << 8;
      inst.instruction |= SHIFT_BY_REG;
      /* PR 12854: Error on extraneous shifts.  */
      constraint (inst.operands[2].shifted,
		  _("extraneous shift as part of operand to shift insn"));
    }
  else
    inst.relocs[0].type = BFD_RELOC_ARM_SHIFT_IMM;
}

static void
do_smc (void)
{
  unsigned int value = inst.relocs[0].exp.X_add_number;
  constraint (value > 0xf, _("immediate too large (bigger than 0xF)"));

  inst.relocs[0].type = BFD_RELOC_ARM_SMC;
  inst.relocs[0].pc_rel = 0;
}

static void
do_hvc (void)
{
  inst.relocs[0].type = BFD_RELOC_ARM_HVC;
  inst.relocs[0].pc_rel = 0;
}

static void
do_swi (void)
{
  inst.relocs[0].type = BFD_RELOC_ARM_SWI;
  inst.relocs[0].pc_rel = 0;
}

static void
do_setpan (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_pan),
	      _("selected processor does not support SETPAN instruction"));

  inst.instruction |= ((inst.operands[0].imm & 1) << 9);
}

static void
do_t_setpan (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_pan),
	      _("selected processor does not support SETPAN instruction"));

  inst.instruction |= (inst.operands[0].imm << 3);
}

/* ARM V5E (El Segundo) signed-multiply-accumulate (argument parse)
   SMLAxy{cond} Rd,Rm,Rs,Rn
   SMLAWy{cond} Rd,Rm,Rs,Rn
   Error if any register is R15.  */

static void
do_smla (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].reg << 8;
  inst.instruction |= inst.operands[3].reg << 12;
}

/* ARM V5E (El Segundo) signed-multiply-accumulate-long (argument parse)
   SMLALxy{cond} Rdlo,Rdhi,Rm,Rs
   Error if any register is R15.
   Warning if Rdlo == Rdhi.  */

static void
do_smlal (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
  inst.instruction |= inst.operands[3].reg << 8;

  if (inst.operands[0].reg == inst.operands[1].reg)
    as_tsktsk (_("rdhi and rdlo must be different"));
}

/* ARM V5E (El Segundo) signed-multiply (argument parse)
   SMULxy{cond} Rd,Rm,Rs
   Error if any register is R15.  */

static void
do_smul (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].reg << 8;
}

/* ARM V6 srs (argument parse).  The variable fields in the encoding are
   the same for both ARM and Thumb-2.  */

static void
do_srs (void)
{
  int reg;

  if (inst.operands[0].present)
    {
      reg = inst.operands[0].reg;
      constraint (reg != REG_SP, _("SRS base register must be r13"));
    }
  else
    reg = REG_SP;

  inst.instruction |= reg << 16;
  inst.instruction |= inst.operands[1].imm;
  if (inst.operands[0].writeback || inst.operands[1].writeback)
    inst.instruction |= WRITE_BACK;
}

/* ARM V6 strex (argument parse).  */

static void
do_strex (void)
{
  constraint (!inst.operands[2].isreg || !inst.operands[2].preind
	      || inst.operands[2].postind || inst.operands[2].writeback
	      || inst.operands[2].immisreg || inst.operands[2].shifted
	      || inst.operands[2].negative
	      /* See comment in do_ldrex().  */
	      || (inst.operands[2].reg == REG_PC),
	      BAD_ADDR_MODE);

  constraint (inst.operands[0].reg == inst.operands[1].reg
	      || inst.operands[0].reg == inst.operands[2].reg, BAD_OVERLAP);

  constraint (inst.relocs[0].exp.X_op != O_constant
	      || inst.relocs[0].exp.X_add_number != 0,
	      _("offset must be zero in ARM encoding"));

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].reg << 16;
  inst.relocs[0].type = BFD_RELOC_UNUSED;
}

static void
do_t_strexbh (void)
{
  constraint (!inst.operands[2].isreg || !inst.operands[2].preind
	      || inst.operands[2].postind || inst.operands[2].writeback
	      || inst.operands[2].immisreg || inst.operands[2].shifted
	      || inst.operands[2].negative,
	      BAD_ADDR_MODE);

  constraint (inst.operands[0].reg == inst.operands[1].reg
	      || inst.operands[0].reg == inst.operands[2].reg, BAD_OVERLAP);

  do_rm_rd_rn ();
}

static void
do_strexd (void)
{
  constraint (inst.operands[1].reg % 2 != 0,
	      _("even register required"));
  constraint (inst.operands[2].present
	      && inst.operands[2].reg != inst.operands[1].reg + 1,
	      _("can only store two consecutive registers"));
  /* If op 2 were present and equal to PC, this function wouldn't
     have been called in the first place.  */
  constraint (inst.operands[1].reg == REG_LR, _("r14 not allowed here"));

  constraint (inst.operands[0].reg == inst.operands[1].reg
	      || inst.operands[0].reg == inst.operands[1].reg + 1
	      || inst.operands[0].reg == inst.operands[3].reg,
	      BAD_OVERLAP);

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[3].reg << 16;
}

/* ARM V8 STRL.  */
static void
do_stlex (void)
{
  constraint (inst.operands[0].reg == inst.operands[1].reg
	      || inst.operands[0].reg == inst.operands[2].reg, BAD_OVERLAP);

  do_rd_rm_rn ();
}

static void
do_t_stlex (void)
{
  constraint (inst.operands[0].reg == inst.operands[1].reg
	      || inst.operands[0].reg == inst.operands[2].reg, BAD_OVERLAP);

  do_rm_rd_rn ();
}

/* ARM V6 SXTAH extracts a 16-bit value from a register, sign
   extends it to 32-bits, and adds the result to a value in another
   register.  You can specify a rotation by 0, 8, 16, or 24 bits
   before extracting the 16-bit value.
   SXTAH{<cond>} <Rd>, <Rn>, <Rm>{, <rotation>}
   Condition defaults to COND_ALWAYS.
   Error if any register uses R15.  */

static void
do_sxtah (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
  inst.instruction |= inst.operands[3].imm << 10;
}

/* ARM V6 SXTH.

   SXTH {<cond>} <Rd>, <Rm>{, <rotation>}
   Condition defaults to COND_ALWAYS.
   Error if any register uses R15.  */

static void
do_sxth (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].imm << 10;
}

/* VFP instructions.  In a logical order: SP variant first, monad
   before dyad, arithmetic then move then load/store.  */

static void
do_vfp_sp_monadic (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)
	      && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
	      _(BAD_FPU));

  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Sm);
}

static void
do_vfp_sp_dyadic (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Sn);
  encode_arm_vfp_reg (inst.operands[2].reg, VFP_REG_Sm);
}

static void
do_vfp_sp_compare_z (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
}

static void
do_vfp_dp_sp_cvt (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Sm);
}

static void
do_vfp_sp_dp_cvt (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Dm);
}

static void
do_vfp_reg_from_sp (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)
	     && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
	     _(BAD_FPU));

  inst.instruction |= inst.operands[0].reg << 12;
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Sn);
}

static void
do_vfp_reg2_from_sp2 (void)
{
  constraint (inst.operands[2].imm != 2,
	      _("only two consecutive VFP SP registers allowed here"));
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  encode_arm_vfp_reg (inst.operands[2].reg, VFP_REG_Sm);
}

static void
do_vfp_sp_from_reg (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)
	     && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
	     _(BAD_FPU));

  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sn);
  inst.instruction |= inst.operands[1].reg << 12;
}

static void
do_vfp_sp2_from_reg2 (void)
{
  constraint (inst.operands[0].imm != 2,
	      _("only two consecutive VFP SP registers allowed here"));
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sm);
  inst.instruction |= inst.operands[1].reg << 12;
  inst.instruction |= inst.operands[2].reg << 16;
}

static void
do_vfp_sp_ldst (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
  encode_arm_cp_address (1, false, true, 0);
}

static void
do_vfp_dp_ldst (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
  encode_arm_cp_address (1, false, true, 0);
}


static void
vfp_sp_ldstm (enum vfp_ldstm_type ldstm_type)
{
  if (inst.operands[0].writeback)
    inst.instruction |= WRITE_BACK;
  else
    constraint (ldstm_type != VFP_LDSTMIA,
		_("this addressing mode requires base-register writeback"));
  inst.instruction |= inst.operands[0].reg << 16;
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Sd);
  inst.instruction |= inst.operands[1].imm;
}

static void
vfp_dp_ldstm (enum vfp_ldstm_type ldstm_type)
{
  int count;

  if (inst.operands[0].writeback)
    inst.instruction |= WRITE_BACK;
  else
    constraint (ldstm_type != VFP_LDSTMIA && ldstm_type != VFP_LDSTMIAX,
		_("this addressing mode requires base-register writeback"));

  inst.instruction |= inst.operands[0].reg << 16;
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Dd);

  count = inst.operands[1].imm << 1;
  if (ldstm_type == VFP_LDSTMIAX || ldstm_type == VFP_LDSTMDBX)
    count += 1;

  inst.instruction |= count;
}

static void
do_vfp_sp_ldstmia (void)
{
  vfp_sp_ldstm (VFP_LDSTMIA);
}

static void
do_vfp_sp_ldstmdb (void)
{
  vfp_sp_ldstm (VFP_LDSTMDB);
}

static void
do_vfp_dp_ldstmia (void)
{
  vfp_dp_ldstm (VFP_LDSTMIA);
}

static void
do_vfp_dp_ldstmdb (void)
{
  vfp_dp_ldstm (VFP_LDSTMDB);
}

static void
do_vfp_xp_ldstmia (void)
{
  vfp_dp_ldstm (VFP_LDSTMIAX);
}

static void
do_vfp_xp_ldstmdb (void)
{
  vfp_dp_ldstm (VFP_LDSTMDBX);
}

static void
do_vfp_dp_rd_rm (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1)
	      && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
	      _(BAD_FPU));

  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Dm);
}

static void
do_vfp_dp_rn_rd (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dn);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Dd);
}

static void
do_vfp_dp_rd_rn (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Dn);
}

static void
do_vfp_dp_rd_rn_rm (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v2)
	      && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
	      _(BAD_FPU));

  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Dn);
  encode_arm_vfp_reg (inst.operands[2].reg, VFP_REG_Dm);
}

static void
do_vfp_dp_rd (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
}

static void
do_vfp_dp_rm_rd_rn (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v2)
	      && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
	      _(BAD_FPU));

  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dm);
  encode_arm_vfp_reg (inst.operands[1].reg, VFP_REG_Dd);
  encode_arm_vfp_reg (inst.operands[2].reg, VFP_REG_Dn);
}

/* VFPv3 instructions.  */
static void
do_vfp_sp_const (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
  inst.instruction |= (inst.operands[1].imm & 0xf0) << 12;
  inst.instruction |= (inst.operands[1].imm & 0x0f);
}

static void
do_vfp_dp_const (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
  inst.instruction |= (inst.operands[1].imm & 0xf0) << 12;
  inst.instruction |= (inst.operands[1].imm & 0x0f);
}

static void
vfp_conv (int srcsize)
{
  int immbits = srcsize - inst.operands[1].imm;

  if (srcsize == 16 && !(immbits >= 0 && immbits <= srcsize))
    {
      /* If srcsize is 16, inst.operands[1].imm must be in the range 0-16.
	 i.e. immbits must be in range 0 - 16.  */
      inst.error = _("immediate value out of range, expected range [0, 16]");
      return;
    }
  else if (srcsize == 32 && !(immbits >= 0 && immbits < srcsize))
    {
      /* If srcsize is 32, inst.operands[1].imm must be in the range 1-32.
	 i.e. immbits must be in range 0 - 31.  */
      inst.error = _("immediate value out of range, expected range [1, 32]");
      return;
    }

  inst.instruction |= (immbits & 1) << 5;
  inst.instruction |= (immbits >> 1);
}

static void
do_vfp_sp_conv_16 (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
  vfp_conv (16);
}

static void
do_vfp_dp_conv_16 (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
  vfp_conv (16);
}

static void
do_vfp_sp_conv_32 (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
  vfp_conv (32);
}

static void
do_vfp_dp_conv_32 (void)
{
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Dd);
  vfp_conv (32);
}

/* FPA instructions.  Also in a logical order.	*/

static void
do_fpa_cmp (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg;
}

static void
do_fpa_ldmstm (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  switch (inst.operands[1].imm)
    {
    case 1: inst.instruction |= CP_T_X;		 break;
    case 2: inst.instruction |= CP_T_Y;		 break;
    case 3: inst.instruction |= CP_T_Y | CP_T_X; break;
    case 4:					 break;
    default: abort ();
    }

  if (inst.instruction & (PRE_INDEX | INDEX_UP))
    {
      /* The instruction specified "ea" or "fd", so we can only accept
	 [Rn]{!}.  The instruction does not really support stacking or
	 unstacking, so we have to emulate these by setting appropriate
	 bits and offsets.  */
      constraint (inst.relocs[0].exp.X_op != O_constant
		  || inst.relocs[0].exp.X_add_number != 0,
		  _("this instruction does not support indexing"));

      if ((inst.instruction & PRE_INDEX) || inst.operands[2].writeback)
	inst.relocs[0].exp.X_add_number = 12 * inst.operands[1].imm;

      if (!(inst.instruction & INDEX_UP))
	inst.relocs[0].exp.X_add_number = -inst.relocs[0].exp.X_add_number;

      if (!(inst.instruction & PRE_INDEX) && inst.operands[2].writeback)
	{
	  inst.operands[2].preind = 0;
	  inst.operands[2].postind = 1;
	}
    }

  encode_arm_cp_address (2, true, true, 0);
}

/* iWMMXt instructions: strictly in alphabetical order.	 */

static void
do_iwmmxt_tandorc (void)
{
  constraint (inst.operands[0].reg != REG_PC, _("only r15 allowed here"));
}

static void
do_iwmmxt_textrc (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].imm;
}

static void
do_iwmmxt_textrm (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].imm;
}

static void
do_iwmmxt_tinsr (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg << 12;
  inst.instruction |= inst.operands[2].imm;
}

static void
do_iwmmxt_tmia (void)
{
  inst.instruction |= inst.operands[0].reg << 5;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].reg << 12;
}

static void
do_iwmmxt_waligni (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
  inst.instruction |= inst.operands[3].imm << 20;
}

static void
do_iwmmxt_wmerge (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
  inst.instruction |= inst.operands[3].imm << 21;
}

static void
do_iwmmxt_wmov (void)
{
  /* WMOV rD, rN is an alias for WOR rD, rN, rN.  */
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[1].reg;
}

static void
do_iwmmxt_wldstbh (void)
{
  int reloc;
  inst.instruction |= inst.operands[0].reg << 12;
  if (thumb_mode)
    reloc = BFD_RELOC_ARM_T32_CP_OFF_IMM_S2;
  else
    reloc = BFD_RELOC_ARM_CP_OFF_IMM_S2;
  encode_arm_cp_address (1, true, false, reloc);
}

static void
do_iwmmxt_wldstw (void)
{
  /* RIWR_RIWC clears .isreg for a control register.  */
  if (!inst.operands[0].isreg)
    {
      constraint (inst.cond != COND_ALWAYS, BAD_COND);
      inst.instruction |= 0xf0000000;
    }

  inst.instruction |= inst.operands[0].reg << 12;
  encode_arm_cp_address (1, true, true, 0);
}

static void
do_iwmmxt_wldstd (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_iwmmxt2)
      && inst.operands[1].immisreg)
    {
      inst.instruction &= ~0x1a000ff;
      inst.instruction |= (0xfU << 28);
      if (inst.operands[1].preind)
	inst.instruction |= PRE_INDEX;
      if (!inst.operands[1].negative)
	inst.instruction |= INDEX_UP;
      if (inst.operands[1].writeback)
	inst.instruction |= WRITE_BACK;
      inst.instruction |= inst.operands[1].reg << 16;
      inst.instruction |= inst.relocs[0].exp.X_add_number << 4;
      inst.instruction |= inst.operands[1].imm;
    }
  else
    encode_arm_cp_address (1, true, false, 0);
}

static void
do_iwmmxt_wshufh (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= ((inst.operands[2].imm & 0xf0) << 16);
  inst.instruction |= (inst.operands[2].imm & 0x0f);
}

static void
do_iwmmxt_wzero (void)
{
  /* WZERO reg is an alias for WANDN reg, reg, reg.  */
  inst.instruction |= inst.operands[0].reg;
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[0].reg << 16;
}

static void
do_iwmmxt_wrwrwr_or_imm5 (void)
{
  if (inst.operands[2].isreg)
    do_rd_rn_rm ();
  else {
    constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_iwmmxt2),
		_("immediate operand requires iWMMXt2"));
    do_rd_rn ();
    if (inst.operands[2].imm == 0)
      {
	switch ((inst.instruction >> 20) & 0xf)
	  {
	  case 4:
	  case 5:
	  case 6:
	  case 7:
	    /* w...h wrd, wrn, #0 -> wrorh wrd, wrn, #16.  */
	    inst.operands[2].imm = 16;
	    inst.instruction = (inst.instruction & 0xff0fffff) | (0x7 << 20);
	    break;
	  case 8:
	  case 9:
	  case 10:
	  case 11:
	    /* w...w wrd, wrn, #0 -> wrorw wrd, wrn, #32.  */
	    inst.operands[2].imm = 32;
	    inst.instruction = (inst.instruction & 0xff0fffff) | (0xb << 20);
	    break;
	  case 12:
	  case 13:
	  case 14:
	  case 15:
	    {
	      /* w...d wrd, wrn, #0 -> wor wrd, wrn, wrn.  */
	      unsigned long wrn;
	      wrn = (inst.instruction >> 16) & 0xf;
	      inst.instruction &= 0xff0fff0f;
	      inst.instruction |= wrn;
	      /* Bail out here; the instruction is now assembled.  */
	      return;
	    }
	  }
      }
    /* Map 32 -> 0, etc.  */
    inst.operands[2].imm &= 0x1f;
    inst.instruction |= (0xfU << 28) | ((inst.operands[2].imm & 0x10) << 4) | (inst.operands[2].imm & 0xf);
  }
}

/* Cirrus Maverick instructions.  Simple 2-, 3-, and 4-register
   operations first, then control, shift, and load/store.  */

/* Insns like "foo X,Y,Z".  */

static void
do_mav_triple (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].reg << 12;
}

/* Insns like "foo W,X,Y,Z".
    where W=MVAX[0:3] and X,Y,Z=MVFX[0:15].  */

static void
do_mav_quad (void)
{
  inst.instruction |= inst.operands[0].reg << 5;
  inst.instruction |= inst.operands[1].reg << 12;
  inst.instruction |= inst.operands[2].reg << 16;
  inst.instruction |= inst.operands[3].reg;
}

/* cfmvsc32<cond> DSPSC,MVDX[15:0].  */
static void
do_mav_dspsc (void)
{
  inst.instruction |= inst.operands[1].reg << 12;
}

/* Maverick shift immediate instructions.
   cfsh32<cond> MVFX[15:0],MVFX[15:0],Shift[6:0].
   cfsh64<cond> MVDX[15:0],MVDX[15:0],Shift[6:0].  */

static void
do_mav_shift (void)
{
  int imm = inst.operands[2].imm;

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;

  /* Bits 0-3 of the insn should have bits 0-3 of the immediate.
     Bits 5-7 of the insn should have bits 4-6 of the immediate.
     Bit 4 should be 0.	 */
  imm = (imm & 0xf) | ((imm & 0x70) << 1);

  inst.instruction |= imm;
}

/* XScale instructions.	 Also sorted arithmetic before move.  */

/* Xscale multiply-accumulate (argument parse)
     MIAcc   acc0,Rm,Rs
     MIAPHcc acc0,Rm,Rs
     MIAxycc acc0,Rm,Rs.  */

static void
do_xsc_mia (void)
{
  inst.instruction |= inst.operands[1].reg;
  inst.instruction |= inst.operands[2].reg << 12;
}

/* Xscale move-accumulator-register (argument parse)

     MARcc   acc0,RdLo,RdHi.  */

static void
do_xsc_mar (void)
{
  inst.instruction |= inst.operands[1].reg << 12;
  inst.instruction |= inst.operands[2].reg << 16;
}

/* Xscale move-register-accumulator (argument parse)

     MRAcc   RdLo,RdHi,acc0.  */

static void
do_xsc_mra (void)
{
  constraint (inst.operands[0].reg == inst.operands[1].reg, BAD_OVERLAP);
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
}

/* Encoding functions relevant only to Thumb.  */

/* inst.operands[i] is a shifted-register operand; encode
   it into inst.instruction in the format used by Thumb32.  */

static void
encode_thumb32_shifted_operand (int i)
{
  unsigned int value = inst.relocs[0].exp.X_add_number;
  unsigned int shift = inst.operands[i].shift_kind;

  constraint (inst.operands[i].immisreg,
	      _("shift by register not allowed in thumb mode"));
  inst.instruction |= inst.operands[i].reg;
  if (shift == SHIFT_RRX)
    inst.instruction |= SHIFT_ROR << 4;
  else
    {
      constraint (inst.relocs[0].exp.X_op != O_constant,
		  _("expression too complex"));

      constraint (value > 32
		  || (value == 32 && (shift == SHIFT_LSL
				      || shift == SHIFT_ROR)),
		  _("shift expression is too large"));

      if (value == 0)
	shift = SHIFT_LSL;
      else if (value == 32)
	value = 0;

      inst.instruction |= shift << 4;
      inst.instruction |= (value & 0x1c) << 10;
      inst.instruction |= (value & 0x03) << 6;
    }
}


/* inst.operands[i] was set up by parse_address.  Encode it into a
   Thumb32 format load or store instruction.  Reject forms that cannot
   be used with such instructions.  If is_t is true, reject forms that
   cannot be used with a T instruction; if is_d is true, reject forms
   that cannot be used with a D instruction.  If it is a store insn,
   reject PC in Rn.  */

static void
encode_thumb32_addr_mode (int i, bool is_t, bool is_d)
{
  const bool is_pc = (inst.operands[i].reg == REG_PC);

  constraint (!inst.operands[i].isreg,
	      _("Instruction does not support =N addresses"));

  inst.instruction |= inst.operands[i].reg << 16;
  if (inst.operands[i].immisreg)
    {
      constraint (is_pc, BAD_PC_ADDRESSING);
      constraint (is_t || is_d, _("cannot use register index with this instruction"));
      constraint (inst.operands[i].negative,
		  _("Thumb does not support negative register indexing"));
      constraint (inst.operands[i].postind,
		  _("Thumb does not support register post-indexing"));
      constraint (inst.operands[i].writeback,
		  _("Thumb does not support register indexing with writeback"));
      constraint (inst.operands[i].shifted && inst.operands[i].shift_kind != SHIFT_LSL,
		  _("Thumb supports only LSL in shifted register indexing"));

      inst.instruction |= inst.operands[i].imm;
      if (inst.operands[i].shifted)
	{
	  constraint (inst.relocs[0].exp.X_op != O_constant,
		      _("expression too complex"));
	  constraint (inst.relocs[0].exp.X_add_number < 0
		      || inst.relocs[0].exp.X_add_number > 3,
		      _("shift out of range"));
	  inst.instruction |= inst.relocs[0].exp.X_add_number << 4;
	}
      inst.relocs[0].type = BFD_RELOC_UNUSED;
    }
  else if (inst.operands[i].preind)
    {
      constraint (is_pc && inst.operands[i].writeback, BAD_PC_WRITEBACK);
      constraint (is_t && inst.operands[i].writeback,
		  _("cannot use writeback with this instruction"));
      constraint (is_pc && ((inst.instruction & THUMB2_LOAD_BIT) == 0),
		  BAD_PC_ADDRESSING);

      if (is_d)
	{
	  inst.instruction |= 0x01000000;
	  if (inst.operands[i].writeback)
	    inst.instruction |= 0x00200000;
	}
      else
	{
	  inst.instruction |= 0x00000c00;
	  if (inst.operands[i].writeback)
	    inst.instruction |= 0x00000100;
	}
      inst.relocs[0].type = BFD_RELOC_ARM_T32_OFFSET_IMM;
    }
  else if (inst.operands[i].postind)
    {
      gas_assert (inst.operands[i].writeback);
      constraint (is_pc, _("cannot use post-indexing with PC-relative addressing"));
      constraint (is_t, _("cannot use post-indexing with this instruction"));

      if (is_d)
	inst.instruction |= 0x00200000;
      else
	inst.instruction |= 0x00000900;
      inst.relocs[0].type = BFD_RELOC_ARM_T32_OFFSET_IMM;
    }
  else /* unindexed - only for coprocessor */
    inst.error = _("instruction does not accept unindexed addressing");
}

/* Table of Thumb instructions which exist in 16- and/or 32-bit
   encodings (the latter only in post-V6T2 cores).  The index is the
   value used in the insns table below.  When there is more than one
   possible 16-bit encoding for the instruction, this table always
   holds variant (1).
   Also contains several pseudo-instructions used during relaxation.  */
#define T16_32_TAB				\
  X(_adc,   4140, eb400000),			\
  X(_adcs,  4140, eb500000),			\
  X(_add,   1c00, eb000000),			\
  X(_adds,  1c00, eb100000),			\
  X(_addi,  0000, f1000000),			\
  X(_addis, 0000, f1100000),			\
  X(_add_pc,000f, f20f0000),			\
  X(_add_sp,000d, f10d0000),			\
  X(_adr,   000f, f20f0000),			\
  X(_and,   4000, ea000000),			\
  X(_ands,  4000, ea100000),			\
  X(_asr,   1000, fa40f000),			\
  X(_asrs,  1000, fa50f000),			\
  X(_aut,   0000, f3af802d),			\
  X(_autg,   0000, fb500f00),			\
  X(_b,     e000, f000b000),			\
  X(_bcond, d000, f0008000),			\
  X(_bf,    0000, f040e001),			\
  X(_bfcsel,0000, f000e001),			\
  X(_bfx,   0000, f060e001),			\
  X(_bfl,   0000, f000c001),			\
  X(_bflx,  0000, f070e001),			\
  X(_bic,   4380, ea200000),			\
  X(_bics,  4380, ea300000),			\
  X(_bxaut, 0000, fb500f10),			\
  X(_cinc,  0000, ea509000),			\
  X(_cinv,  0000, ea50a000),			\
  X(_cmn,   42c0, eb100f00),			\
  X(_cmp,   2800, ebb00f00),			\
  X(_cneg,  0000, ea50b000),			\
  X(_cpsie, b660, f3af8400),			\
  X(_cpsid, b670, f3af8600),			\
  X(_cpy,   4600, ea4f0000),			\
  X(_csel,  0000, ea508000),			\
  X(_cset,  0000, ea5f900f),			\
  X(_csetm, 0000, ea5fa00f),			\
  X(_csinc, 0000, ea509000),			\
  X(_csinv, 0000, ea50a000),			\
  X(_csneg, 0000, ea50b000),			\
  X(_dec_sp,80dd, f1ad0d00),			\
  X(_dls,   0000, f040e001),			\
  X(_dlstp, 0000, f000e001),			\
  X(_eor,   4040, ea800000),			\
  X(_eors,  4040, ea900000),			\
  X(_inc_sp,00dd, f10d0d00),			\
  X(_lctp,  0000, f00fe001),			\
  X(_ldmia, c800, e8900000),			\
  X(_ldr,   6800, f8500000),			\
  X(_ldrb,  7800, f8100000),			\
  X(_ldrh,  8800, f8300000),			\
  X(_ldrsb, 5600, f9100000),			\
  X(_ldrsh, 5e00, f9300000),			\
  X(_ldr_pc,4800, f85f0000),			\
  X(_ldr_pc2,4800, f85f0000),			\
  X(_ldr_sp,9800, f85d0000),			\
  X(_le,    0000, f00fc001),			\
  X(_letp,  0000, f01fc001),			\
  X(_lsl,   0000, fa00f000),			\
  X(_lsls,  0000, fa10f000),			\
  X(_lsr,   0800, fa20f000),			\
  X(_lsrs,  0800, fa30f000),			\
  X(_mov,   2000, ea4f0000),			\
  X(_movs,  2000, ea5f0000),			\
  X(_mul,   4340, fb00f000),                     \
  X(_muls,  4340, ffffffff), /* no 32b muls */	\
  X(_mvn,   43c0, ea6f0000),			\
  X(_mvns,  43c0, ea7f0000),			\
  X(_neg,   4240, f1c00000), /* rsb #0 */	\
  X(_negs,  4240, f1d00000), /* rsbs #0 */	\
  X(_orr,   4300, ea400000),			\
  X(_orrs,  4300, ea500000),			\
  X(_pac,   0000, f3af801d),			\
  X(_pacbti, 0000, f3af800d),			\
  X(_pacg,  0000, fb60f000),			\
  X(_pop,   bc00, e8bd0000), /* ldmia sp!,... */	\
  X(_push,  b400, e92d0000), /* stmdb sp!,... */	\
  X(_rev,   ba00, fa90f080),			\
  X(_rev16, ba40, fa90f090),			\
  X(_revsh, bac0, fa90f0b0),			\
  X(_ror,   41c0, fa60f000),			\
  X(_rors,  41c0, fa70f000),			\
  X(_sbc,   4180, eb600000),			\
  X(_sbcs,  4180, eb700000),			\
  X(_stmia, c000, e8800000),			\
  X(_str,   6000, f8400000),			\
  X(_strb,  7000, f8000000),			\
  X(_strh,  8000, f8200000),			\
  X(_str_sp,9000, f84d0000),			\
  X(_sub,   1e00, eba00000),			\
  X(_subs,  1e00, ebb00000),			\
  X(_subi,  8000, f1a00000),			\
  X(_subis, 8000, f1b00000),			\
  X(_sxtb,  b240, fa4ff080),			\
  X(_sxth,  b200, fa0ff080),			\
  X(_tst,   4200, ea100f00),			\
  X(_uxtb,  b2c0, fa5ff080),			\
  X(_uxth,  b280, fa1ff080),			\
  X(_nop,   bf00, f3af8000),			\
  X(_yield, bf10, f3af8001),			\
  X(_wfe,   bf20, f3af8002),			\
  X(_wfi,   bf30, f3af8003),			\
  X(_wls,   0000, f040c001),			\
  X(_wlstp, 0000, f000c001),			\
  X(_sev,   bf40, f3af8004),                    \
  X(_sevl,  bf50, f3af8005),			\
  X(_udf,   de00, f7f0a000)

/* To catch errors in encoding functions, the codes are all offset by
   0xF800, putting them in one of the 32-bit prefix ranges, ergo undefined
   as 16-bit instructions.  */
#define X(a,b,c) T_MNEM##a
enum t16_32_codes { T16_32_OFFSET = 0xF7FF, T16_32_TAB };
#undef X

#define X(a,b,c) 0x##b
static const unsigned short thumb_op16[] = { T16_32_TAB };
#define THUMB_OP16(n) (thumb_op16[(n) - (T16_32_OFFSET + 1)])
#undef X

#define X(a,b,c) 0x##c
static const unsigned int thumb_op32[] = { T16_32_TAB };
#define THUMB_OP32(n)        (thumb_op32[(n) - (T16_32_OFFSET + 1)])
#define THUMB_SETS_FLAGS(n)  (THUMB_OP32 (n) & 0x00100000)
#undef X
#undef T16_32_TAB

/* Thumb instruction encoders, in alphabetical order.  */

/* ADDW or SUBW.  */

static void
do_t_add_sub_w (void)
{
  int Rd, Rn;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[1].reg;

  /* If Rn is REG_PC, this is ADR; if Rn is REG_SP, then this
     is the SP-{plus,minus}-immediate form of the instruction.  */
  if (Rn == REG_SP)
    constraint (Rd == REG_PC, BAD_PC);
  else
    reject_bad_reg (Rd);

  inst.instruction |= (Rn << 16) | (Rd << 8);
  inst.relocs[0].type = BFD_RELOC_ARM_T32_IMM12;
}

/* Parse an add or subtract instruction.  We get here with inst.instruction
   equaling any of THUMB_OPCODE_add, adds, sub, or subs.  */

static void
do_t_add_sub (void)
{
  int Rd, Rs, Rn;

  Rd = inst.operands[0].reg;
  Rs = (inst.operands[1].present
	? inst.operands[1].reg    /* Rd, Rs, foo */
	: inst.operands[0].reg);  /* Rd, foo -> Rd, Rd, foo */

  if (Rd == REG_PC)
    set_pred_insn_type_last ();

  if (unified_syntax)
    {
      bool flags;
      bool narrow;
      int opcode;

      flags = (inst.instruction == T_MNEM_adds
	       || inst.instruction == T_MNEM_subs);
      if (flags)
	narrow = !in_pred_block ();
      else
	narrow = in_pred_block ();
      if (!inst.operands[2].isreg)
	{
	  int add;

	  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))
	    constraint (Rd == REG_SP && Rs != REG_SP, BAD_SP);

	  add = (inst.instruction == T_MNEM_add
		 || inst.instruction == T_MNEM_adds);
	  opcode = 0;
	  if (inst.size_req != 4)
	    {
	      /* Attempt to use a narrow opcode, with relaxation if
		 appropriate.  */
	      if (Rd == REG_SP && Rs == REG_SP && !flags)
		opcode = add ? T_MNEM_inc_sp : T_MNEM_dec_sp;
	      else if (Rd <= 7 && Rs == REG_SP && add && !flags)
		opcode = T_MNEM_add_sp;
	      else if (Rd <= 7 && Rs == REG_PC && add && !flags)
		opcode = T_MNEM_add_pc;
	      else if (Rd <= 7 && Rs <= 7 && narrow)
		{
		  if (flags)
		    opcode = add ? T_MNEM_addis : T_MNEM_subis;
		  else
		    opcode = add ? T_MNEM_addi : T_MNEM_subi;
		}
	      if (opcode)
		{
		  inst.instruction = THUMB_OP16(opcode);
		  inst.instruction |= (Rd << 4) | Rs;
		  if (inst.relocs[0].type < BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC
		      || (inst.relocs[0].type
			  > BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC))
		  {
		    if (inst.size_req == 2)
		      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_ADD;
		    else
		      inst.relax = opcode;
		  }
		}
	      else
		constraint (inst.size_req == 2, _("cannot honor width suffix"));
	    }
	  if (inst.size_req == 4
	      || (inst.size_req != 2 && !opcode))
	    {
	      constraint ((inst.relocs[0].type
			   >= BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC)
			  && (inst.relocs[0].type
			      <= BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC) ,
			  THUMB1_RELOC_ONLY);
	      if (Rd == REG_PC)
		{
		  constraint (add, BAD_PC);
		  constraint (Rs != REG_LR || inst.instruction != T_MNEM_subs,
			     _("only SUBS PC, LR, #const allowed"));
		  constraint (inst.relocs[0].exp.X_op != O_constant,
			      _("expression too complex"));
		  constraint (inst.relocs[0].exp.X_add_number < 0
			      || inst.relocs[0].exp.X_add_number > 0xff,
			     _("immediate value out of range"));
		  inst.instruction = T2_SUBS_PC_LR
				     | inst.relocs[0].exp.X_add_number;
		  inst.relocs[0].type = BFD_RELOC_UNUSED;
		  return;
		}
	      else if (Rs == REG_PC)
		{
		  /* Always use addw/subw.  */
		  inst.instruction = add ? 0xf20f0000 : 0xf2af0000;
		  inst.relocs[0].type = BFD_RELOC_ARM_T32_IMM12;
		}
	      else
		{
		  inst.instruction = THUMB_OP32 (inst.instruction);
		  inst.instruction = (inst.instruction & 0xe1ffffff)
				     | 0x10000000;
		  if (flags)
		    inst.relocs[0].type = BFD_RELOC_ARM_T32_IMMEDIATE;
		  else
		    inst.relocs[0].type = BFD_RELOC_ARM_T32_ADD_IMM;
		}
	      inst.instruction |= Rd << 8;
	      inst.instruction |= Rs << 16;
	    }
	}
      else
	{
	  unsigned int value = inst.relocs[0].exp.X_add_number;
	  unsigned int shift = inst.operands[2].shift_kind;

	  Rn = inst.operands[2].reg;
	  /* See if we can do this with a 16-bit instruction.  */
	  if (!inst.operands[2].shifted && inst.size_req != 4)
	    {
	      if (Rd > 7 || Rs > 7 || Rn > 7)
		narrow = false;

	      if (narrow)
		{
		  inst.instruction = ((inst.instruction == T_MNEM_adds
				       || inst.instruction == T_MNEM_add)
				      ? T_OPCODE_ADD_R3
				      : T_OPCODE_SUB_R3);
		  inst.instruction |= Rd | (Rs << 3) | (Rn << 6);
		  return;
		}

	      if (inst.instruction == T_MNEM_add && (Rd == Rs || Rd == Rn))
		{
		  /* Thumb-1 cores (except v6-M) require at least one high
		     register in a narrow non flag setting add.  */
		  if (Rd > 7 || Rn > 7
		      || ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6t2)
		      || ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_msr))
		    {
		      if (Rd == Rn)
			{
			  Rn = Rs;
			  Rs = Rd;
			}
		      inst.instruction = T_OPCODE_ADD_HI;
		      inst.instruction |= (Rd & 8) << 4;
		      inst.instruction |= (Rd & 7);
		      inst.instruction |= Rn << 3;
		      return;
		    }
		}
	    }

	  constraint (Rd == REG_PC, BAD_PC);
	  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))
	    constraint (Rd == REG_SP && Rs != REG_SP, BAD_SP);
	  constraint (Rs == REG_PC, BAD_PC);
	  reject_bad_reg (Rn);

	  /* If we get here, it can't be done in 16 bits.  */
	  constraint (inst.operands[2].shifted && inst.operands[2].immisreg,
		      _("shift must be constant"));
	  inst.instruction = THUMB_OP32 (inst.instruction);
	  inst.instruction |= Rd << 8;
	  inst.instruction |= Rs << 16;
	  constraint (Rd == REG_SP && Rs == REG_SP && value > 3,
		      _("shift value over 3 not allowed in thumb mode"));
	  constraint (Rd == REG_SP && Rs == REG_SP && shift != SHIFT_LSL,
		      _("only LSL shift allowed in thumb mode"));
	  encode_thumb32_shifted_operand (2);
	}
    }
  else
    {
      constraint (inst.instruction == T_MNEM_adds
		  || inst.instruction == T_MNEM_subs,
		  BAD_THUMB32);

      if (!inst.operands[2].isreg) /* Rd, Rs, #imm */
	{
	  constraint ((Rd > 7 && (Rd != REG_SP || Rs != REG_SP))
		      || (Rs > 7 && Rs != REG_SP && Rs != REG_PC),
		      BAD_HIREG);

	  inst.instruction = (inst.instruction == T_MNEM_add
			      ? 0x0000 : 0x8000);
	  inst.instruction |= (Rd << 4) | Rs;
	  inst.relocs[0].type = BFD_RELOC_ARM_THUMB_ADD;
	  return;
	}

      Rn = inst.operands[2].reg;
      constraint (inst.operands[2].shifted, _("unshifted register required"));

      /* We now have Rd, Rs, and Rn set to registers.  */
      if (Rd > 7 || Rs > 7 || Rn > 7)
	{
	  /* Can't do this for SUB.	 */
	  constraint (inst.instruction == T_MNEM_sub, BAD_HIREG);
	  inst.instruction = T_OPCODE_ADD_HI;
	  inst.instruction |= (Rd & 8) << 4;
	  inst.instruction |= (Rd & 7);
	  if (Rs == Rd)
	    inst.instruction |= Rn << 3;
	  else if (Rn == Rd)
	    inst.instruction |= Rs << 3;
	  else
	    constraint (1, _("dest must overlap one source register"));
	}
      else
	{
	  inst.instruction = (inst.instruction == T_MNEM_add
			      ? T_OPCODE_ADD_R3 : T_OPCODE_SUB_R3);
	  inst.instruction |= Rd | (Rs << 3) | (Rn << 6);
	}
    }
}

static void
do_t_adr (void)
{
  unsigned Rd;

  Rd = inst.operands[0].reg;
  reject_bad_reg (Rd);

  if (unified_syntax && inst.size_req == 0 && Rd <= 7)
    {
      /* Defer to section relaxation.  */
      inst.relax = inst.instruction;
      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= Rd << 4;
    }
  else if (unified_syntax && inst.size_req != 2)
    {
      /* Generate a 32-bit opcode.  */
      inst.instruction = THUMB_OP32 (inst.instruction);
      inst.instruction |= Rd << 8;
      inst.relocs[0].type = BFD_RELOC_ARM_T32_ADD_PC12;
      inst.relocs[0].pc_rel = 1;
    }
  else
    {
      /* Generate a 16-bit opcode.  */
      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_ADD;
      inst.relocs[0].exp.X_add_number -= 4; /* PC relative adjust.  */
      inst.relocs[0].pc_rel = 1;
      inst.instruction |= Rd << 4;
    }

  if (inst.relocs[0].exp.X_op == O_symbol
      && inst.relocs[0].exp.X_add_symbol != NULL
      && S_IS_DEFINED (inst.relocs[0].exp.X_add_symbol)
      && THUMB_IS_FUNC (inst.relocs[0].exp.X_add_symbol))
    inst.relocs[0].exp.X_add_number += 1;
}

/* Arithmetic instructions for which there is just one 16-bit
   instruction encoding, and it allows only two low registers.
   For maximal compatibility with ARM syntax, we allow three register
   operands even when Thumb-32 instructions are not available, as long
   as the first two are identical.  For instance, both "sbc r0,r1" and
   "sbc r0,r0,r1" are allowed.  */
static void
do_t_arit3 (void)
{
  int Rd, Rs, Rn;

  Rd = inst.operands[0].reg;
  Rs = (inst.operands[1].present
	? inst.operands[1].reg    /* Rd, Rs, foo */
	: inst.operands[0].reg);  /* Rd, foo -> Rd, Rd, foo */
  Rn = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rs);
  if (inst.operands[2].isreg)
    reject_bad_reg (Rn);

  if (unified_syntax)
    {
      if (!inst.operands[2].isreg)
	{
	  /* For an immediate, we always generate a 32-bit opcode;
	     section relaxation will shrink it later if possible.  */
	  inst.instruction = THUMB_OP32 (inst.instruction);
	  inst.instruction = (inst.instruction & 0xe1ffffff) | 0x10000000;
	  inst.instruction |= Rd << 8;
	  inst.instruction |= Rs << 16;
	  inst.relocs[0].type = BFD_RELOC_ARM_T32_IMMEDIATE;
	}
      else
	{
	  bool narrow;

	  /* See if we can do this with a 16-bit instruction.  */
	  if (THUMB_SETS_FLAGS (inst.instruction))
	    narrow = !in_pred_block ();
	  else
	    narrow = in_pred_block ();

	  if (Rd > 7 || Rn > 7 || Rs > 7)
	    narrow = false;
	  if (inst.operands[2].shifted)
	    narrow = false;
	  if (inst.size_req == 4)
	    narrow = false;

	  if (narrow
	      && Rd == Rs)
	    {
	      inst.instruction = THUMB_OP16 (inst.instruction);
	      inst.instruction |= Rd;
	      inst.instruction |= Rn << 3;
	      return;
	    }

	  /* If we get here, it can't be done in 16 bits.  */
	  constraint (inst.operands[2].shifted
		      && inst.operands[2].immisreg,
		      _("shift must be constant"));
	  inst.instruction = THUMB_OP32 (inst.instruction);
	  inst.instruction |= Rd << 8;
	  inst.instruction |= Rs << 16;
	  encode_thumb32_shifted_operand (2);
	}
    }
  else
    {
      /* On its face this is a lie - the instruction does set the
	 flags.  However, the only supported mnemonic in this mode
	 says it doesn't.  */
      constraint (THUMB_SETS_FLAGS (inst.instruction), BAD_THUMB32);

      constraint (!inst.operands[2].isreg || inst.operands[2].shifted,
		  _("unshifted register required"));
      constraint (Rd > 7 || Rs > 7 || Rn > 7, BAD_HIREG);
      constraint (Rd != Rs,
		  _("dest and source1 must be the same register"));

      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= Rd;
      inst.instruction |= Rn << 3;
    }
}

/* Similarly, but for instructions where the arithmetic operation is
   commutative, so we can allow either of them to be different from
   the destination operand in a 16-bit instruction.  For instance, all
   three of "adc r0,r1", "adc r0,r0,r1", and "adc r0,r1,r0" are
   accepted.  */
static void
do_t_arit3c (void)
{
  int Rd, Rs, Rn;

  Rd = inst.operands[0].reg;
  Rs = (inst.operands[1].present
	? inst.operands[1].reg    /* Rd, Rs, foo */
	: inst.operands[0].reg);  /* Rd, foo -> Rd, Rd, foo */
  Rn = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rs);
  if (inst.operands[2].isreg)
    reject_bad_reg (Rn);

  if (unified_syntax)
    {
      if (!inst.operands[2].isreg)
	{
	  /* For an immediate, we always generate a 32-bit opcode;
	     section relaxation will shrink it later if possible.  */
	  inst.instruction = THUMB_OP32 (inst.instruction);
	  inst.instruction = (inst.instruction & 0xe1ffffff) | 0x10000000;
	  inst.instruction |= Rd << 8;
	  inst.instruction |= Rs << 16;
	  inst.relocs[0].type = BFD_RELOC_ARM_T32_IMMEDIATE;
	}
      else
	{
	  bool narrow;

	  /* See if we can do this with a 16-bit instruction.  */
	  if (THUMB_SETS_FLAGS (inst.instruction))
	    narrow = !in_pred_block ();
	  else
	    narrow = in_pred_block ();

	  if (Rd > 7 || Rn > 7 || Rs > 7)
	    narrow = false;
	  if (inst.operands[2].shifted)
	    narrow = false;
	  if (inst.size_req == 4)
	    narrow = false;

	  if (narrow)
	    {
	      if (Rd == Rs)
		{
		  inst.instruction = THUMB_OP16 (inst.instruction);
		  inst.instruction |= Rd;
		  inst.instruction |= Rn << 3;
		  return;
		}
	      if (Rd == Rn)
		{
		  inst.instruction = THUMB_OP16 (inst.instruction);
		  inst.instruction |= Rd;
		  inst.instruction |= Rs << 3;
		  return;
		}
	    }

	  /* If we get here, it can't be done in 16 bits.  */
	  constraint (inst.operands[2].shifted
		      && inst.operands[2].immisreg,
		      _("shift must be constant"));
	  inst.instruction = THUMB_OP32 (inst.instruction);
	  inst.instruction |= Rd << 8;
	  inst.instruction |= Rs << 16;
	  encode_thumb32_shifted_operand (2);
	}
    }
  else
    {
      /* On its face this is a lie - the instruction does set the
	 flags.  However, the only supported mnemonic in this mode
	 says it doesn't.  */
      constraint (THUMB_SETS_FLAGS (inst.instruction), BAD_THUMB32);

      constraint (!inst.operands[2].isreg || inst.operands[2].shifted,
		  _("unshifted register required"));
      constraint (Rd > 7 || Rs > 7 || Rn > 7, BAD_HIREG);

      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= Rd;

      if (Rd == Rs)
	inst.instruction |= Rn << 3;
      else if (Rd == Rn)
	inst.instruction |= Rs << 3;
      else
	constraint (1, _("dest must overlap one source register"));
    }
}

static void
do_t_bfc (void)
{
  unsigned Rd;
  unsigned int msb = inst.operands[1].imm + inst.operands[2].imm;
  constraint (msb > 32, _("bit-field extends past end of register"));
  /* The instruction encoding stores the LSB and MSB,
     not the LSB and width.  */
  Rd = inst.operands[0].reg;
  reject_bad_reg (Rd);
  inst.instruction |= Rd << 8;
  inst.instruction |= (inst.operands[1].imm & 0x1c) << 10;
  inst.instruction |= (inst.operands[1].imm & 0x03) << 6;
  inst.instruction |= msb - 1;
}

static void
do_t_bfi (void)
{
  int Rd, Rn;
  unsigned int msb;

  Rd = inst.operands[0].reg;
  reject_bad_reg (Rd);

  /* #0 in second position is alternative syntax for bfc, which is
     the same instruction but with REG_PC in the Rm field.  */
  if (!inst.operands[1].isreg)
    Rn = REG_PC;
  else
    {
      Rn = inst.operands[1].reg;
      reject_bad_reg (Rn);
    }

  msb = inst.operands[2].imm + inst.operands[3].imm;
  constraint (msb > 32, _("bit-field extends past end of register"));
  /* The instruction encoding stores the LSB and MSB,
     not the LSB and width.  */
  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= (inst.operands[2].imm & 0x1c) << 10;
  inst.instruction |= (inst.operands[2].imm & 0x03) << 6;
  inst.instruction |= msb - 1;
}

static void
do_t_bfx (void)
{
  unsigned Rd, Rn;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[1].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);

  constraint (inst.operands[2].imm + inst.operands[3].imm > 32,
	      _("bit-field extends past end of register"));
  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= (inst.operands[2].imm & 0x1c) << 10;
  inst.instruction |= (inst.operands[2].imm & 0x03) << 6;
  inst.instruction |= inst.operands[3].imm - 1;
}

/* ARM V5 Thumb BLX (argument parse)
	BLX <target_addr>	which is BLX(1)
	BLX <Rm>		which is BLX(2)
   Unfortunately, there are two different opcodes for this mnemonic.
   So, the insns[].value is not used, and the code here zaps values
	into inst.instruction.

   ??? How to take advantage of the additional two bits of displacement
   available in Thumb32 mode?  Need new relocation?  */

static void
do_t_blx (void)
{
  set_pred_insn_type_last ();

  if (inst.operands[0].isreg)
    {
      constraint (inst.operands[0].reg == REG_PC, BAD_PC);
      /* We have a register, so this is BLX(2).  */
      inst.instruction |= inst.operands[0].reg << 3;
    }
  else
    {
      /* No register.  This must be BLX(1).  */
      inst.instruction = 0xf000e800;
      encode_branch (BFD_RELOC_THUMB_PCREL_BLX);
    }
}

static void
do_t_branch (void)
{
  int opcode;
  int cond;
  bfd_reloc_code_real_type reloc;

  cond = inst.cond;
  set_pred_insn_type (IF_INSIDE_IT_LAST_INSN);

  if (in_pred_block ())
    {
      /* Conditional branches inside IT blocks are encoded as unconditional
	 branches.  */
      cond = COND_ALWAYS;
    }
  else
    cond = inst.cond;

  if (cond != COND_ALWAYS)
    opcode = T_MNEM_bcond;
  else
    opcode = inst.instruction;

  if (unified_syntax
      && (inst.size_req == 4
	  || (inst.size_req != 2
	      && (inst.operands[0].hasreloc
		  || inst.relocs[0].exp.X_op == O_constant))))
    {
      inst.instruction = THUMB_OP32(opcode);
      if (cond == COND_ALWAYS)
	reloc = BFD_RELOC_THUMB_PCREL_BRANCH25;
      else
	{
	  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2),
		      _("selected architecture does not support "
			"wide conditional branch instruction"));

	  gas_assert (cond != 0xF);
	  inst.instruction |= cond << 22;
	  reloc = BFD_RELOC_THUMB_PCREL_BRANCH20;
	}
    }
  else
    {
      inst.instruction = THUMB_OP16(opcode);
      if (cond == COND_ALWAYS)
	reloc = BFD_RELOC_THUMB_PCREL_BRANCH12;
      else
	{
	  inst.instruction |= cond << 8;
	  reloc = BFD_RELOC_THUMB_PCREL_BRANCH9;
	}
      /* Allow section relaxation.  */
      if (unified_syntax && inst.size_req != 2)
	inst.relax = opcode;
    }
  inst.relocs[0].type = reloc;
  inst.relocs[0].pc_rel = 1;
}

/* Actually do the work for Thumb state bkpt and hlt.  The only difference
   between the two is the maximum immediate allowed - which is passed in
   RANGE.  */
static void
do_t_bkpt_hlt1 (int range)
{
  constraint (inst.cond != COND_ALWAYS,
	      _("instruction is always unconditional"));
  if (inst.operands[0].present)
    {
      constraint (inst.operands[0].imm > range,
		  _("immediate value out of range"));
      inst.instruction |= inst.operands[0].imm;
    }

  set_pred_insn_type (NEUTRAL_IT_INSN);
}

static void
do_t_hlt (void)
{
  do_t_bkpt_hlt1 (63);
}

static void
do_t_bkpt (void)
{
  do_t_bkpt_hlt1 (255);
}

static void
do_t_branch23 (void)
{
  set_pred_insn_type_last ();
  encode_branch (BFD_RELOC_THUMB_PCREL_BRANCH23);

  /* md_apply_fix blows up with 'bl foo(PLT)' where foo is defined in
     this file.  We used to simply ignore the PLT reloc type here --
     the branch encoding is now needed to deal with TLSCALL relocs.
     So if we see a PLT reloc now, put it back to how it used to be to
     keep the preexisting behaviour.  */
  if (inst.relocs[0].type == BFD_RELOC_ARM_PLT32)
    inst.relocs[0].type = BFD_RELOC_THUMB_PCREL_BRANCH23;

#if defined(OBJ_COFF)
  /* If the destination of the branch is a defined symbol which does not have
     the THUMB_FUNC attribute, then we must be calling a function which has
     the (interfacearm) attribute.  We look for the Thumb entry point to that
     function and change the branch to refer to that function instead.	*/
  if (	 inst.relocs[0].exp.X_op == O_symbol
      && inst.relocs[0].exp.X_add_symbol != NULL
      && S_IS_DEFINED (inst.relocs[0].exp.X_add_symbol)
      && ! THUMB_IS_FUNC (inst.relocs[0].exp.X_add_symbol))
    inst.relocs[0].exp.X_add_symbol
      = find_real_start (inst.relocs[0].exp.X_add_symbol);
#endif
}

static void
do_t_bx (void)
{
  set_pred_insn_type_last ();
  inst.instruction |= inst.operands[0].reg << 3;
  /* ??? FIXME: Should add a hacky reloc here if reg is REG_PC.	 The reloc
     should cause the alignment to be checked once it is known.	 This is
     because BX PC only works if the instruction is word aligned.  */
}

static void
do_t_bxj (void)
{
  int Rm;

  set_pred_insn_type_last ();
  Rm = inst.operands[0].reg;
  reject_bad_reg (Rm);
  inst.instruction |= Rm << 16;
}

static void
do_t_clz (void)
{
  unsigned Rd;
  unsigned Rm;

  Rd = inst.operands[0].reg;
  Rm = inst.operands[1].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rm);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rm << 16;
  inst.instruction |= Rm;
}

/* For the Armv8.1-M conditional instructions.  */
static void
do_t_cond (void)
{
  unsigned Rd, Rn, Rm;
  signed int cond;

  constraint (inst.cond != COND_ALWAYS, BAD_COND);

  Rd = inst.operands[0].reg;
  switch (inst.instruction)
    {
      case T_MNEM_csinc:
      case T_MNEM_csinv:
      case T_MNEM_csneg:
      case T_MNEM_csel:
	Rn = inst.operands[1].reg;
	Rm = inst.operands[2].reg;
	cond = inst.operands[3].imm;
	constraint (Rn == REG_SP, BAD_SP);
	constraint (Rm == REG_SP, BAD_SP);
	break;

      case T_MNEM_cinc:
      case T_MNEM_cinv:
      case T_MNEM_cneg:
	Rn = inst.operands[1].reg;
	cond = inst.operands[2].imm;
	/* Invert the last bit to invert the cond.  */
	cond = TOGGLE_BIT (cond, 0);
	constraint (Rn == REG_SP, BAD_SP);
	Rm = Rn;
	break;

      case T_MNEM_csetm:
      case T_MNEM_cset:
	cond = inst.operands[1].imm;
	/* Invert the last bit to invert the cond.  */
	cond = TOGGLE_BIT (cond, 0);
	Rn = REG_PC;
	Rm = REG_PC;
	break;

      default: abort ();
    }

  set_pred_insn_type (OUTSIDE_PRED_INSN);
  inst.instruction = THUMB_OP32 (inst.instruction);
  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;
  inst.instruction |= cond << 4;
}

static void
do_t_csdb (void)
{
  set_pred_insn_type (OUTSIDE_PRED_INSN);
}

static void
do_t_cps (void)
{
  set_pred_insn_type (OUTSIDE_PRED_INSN);
  inst.instruction |= inst.operands[0].imm;
}

static void
do_t_cpsi (void)
{
  set_pred_insn_type (OUTSIDE_PRED_INSN);
  if (unified_syntax
      && (inst.operands[1].present || inst.size_req == 4)
      && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6_notm))
    {
      unsigned int imod = (inst.instruction & 0x0030) >> 4;
      inst.instruction = 0xf3af8000;
      inst.instruction |= imod << 9;
      inst.instruction |= inst.operands[0].imm << 5;
      if (inst.operands[1].present)
	inst.instruction |= 0x100 | inst.operands[1].imm;
    }
  else
    {
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v1)
		  && (inst.operands[0].imm & 4),
		  _("selected processor does not support 'A' form "
		    "of this instruction"));
      constraint (inst.operands[1].present || inst.size_req == 4,
		  _("Thumb does not support the 2-argument "
		    "form of this instruction"));
      inst.instruction |= inst.operands[0].imm;
    }
}

/* THUMB CPY instruction (argument parse).  */

static void
do_t_cpy (void)
{
  if (inst.size_req == 4)
    {
      inst.instruction = THUMB_OP32 (T_MNEM_mov);
      inst.instruction |= inst.operands[0].reg << 8;
      inst.instruction |= inst.operands[1].reg;
    }
  else
    {
      inst.instruction |= (inst.operands[0].reg & 0x8) << 4;
      inst.instruction |= (inst.operands[0].reg & 0x7);
      inst.instruction |= inst.operands[1].reg << 3;
    }
}

static void
do_t_cbz (void)
{
  set_pred_insn_type (OUTSIDE_PRED_INSN);
  constraint (inst.operands[0].reg > 7, BAD_HIREG);
  inst.instruction |= inst.operands[0].reg;
  inst.relocs[0].pc_rel = 1;
  inst.relocs[0].type = BFD_RELOC_THUMB_PCREL_BRANCH7;
}

static void
do_t_dbg (void)
{
  inst.instruction |= inst.operands[0].imm;
}

static void
do_t_div (void)
{
  unsigned Rd, Rn, Rm;

  Rd = inst.operands[0].reg;
  Rn = (inst.operands[1].present
	? inst.operands[1].reg : Rd);
  Rm = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);
  reject_bad_reg (Rm);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;
}

static void
do_t_hint (void)
{
  if (unified_syntax && inst.size_req == 4)
    inst.instruction = THUMB_OP32 (inst.instruction);
  else
    inst.instruction = THUMB_OP16 (inst.instruction);
}

static void
do_t_it (void)
{
  unsigned int cond = inst.operands[0].imm;

  set_pred_insn_type (IT_INSN);
  now_pred.mask = (inst.instruction & 0xf) | 0x10;
  now_pred.cc = cond;
  now_pred.warn_deprecated = false;
  now_pred.type = SCALAR_PRED;

  /* If the condition is a negative condition, invert the mask.  */
  if ((cond & 0x1) == 0x0)
    {
      unsigned int mask = inst.instruction & 0x000f;

      if ((mask & 0x7) == 0)
	{
	  /* No conversion needed.  */
	  now_pred.block_length = 1;
	}
      else if ((mask & 0x3) == 0)
	{
	  mask ^= 0x8;
	  now_pred.block_length = 2;
	}
      else if ((mask & 0x1) == 0)
	{
	  mask ^= 0xC;
	  now_pred.block_length = 3;
	}
      else
	{
	  mask ^= 0xE;
	  now_pred.block_length = 4;
	}

      inst.instruction &= 0xfff0;
      inst.instruction |= mask;
    }

  inst.instruction |= cond << 4;
}

/* Helper function used for both push/pop and ldm/stm.  */
static void
encode_thumb2_multi (bool do_io, int base, unsigned mask,
		     bool writeback)
{
  bool load, store;

  gas_assert (base != -1 || !do_io);
  load = do_io && ((inst.instruction & (1 << 20)) != 0);
  store = do_io && !load;

  if (mask & (1 << 13))
    inst.error =  _("SP not allowed in register list");

  if (do_io && (mask & (1 << base)) != 0
      && writeback)
    inst.error = _("having the base register in the register list when "
		   "using write back is UNPREDICTABLE");

  if (load)
    {
      if (mask & (1 << 15))
	{
	  if (mask & (1 << 14))
	    inst.error = _("LR and PC should not both be in register list");
	  else
	    set_pred_insn_type_last ();
	}
    }
  else if (store)
    {
      if (mask & (1 << 15))
	inst.error = _("PC not allowed in register list");
    }

  if (do_io && ((mask & (mask - 1)) == 0))
    {
      /* Single register transfers implemented as str/ldr.  */
      if (writeback)
	{
	  if (inst.instruction & (1 << 23))
	    inst.instruction = 0x00000b04; /* ia! -> [base], #4 */
	  else
	    inst.instruction = 0x00000d04; /* db! -> [base, #-4]! */
	}
      else
	{
	  if (inst.instruction & (1 << 23))
	    inst.instruction = 0x00800000; /* ia -> [base] */
	  else
	    inst.instruction = 0x00000c04; /* db -> [base, #-4] */
	}

      inst.instruction |= 0xf8400000;
      if (load)
	inst.instruction |= 0x00100000;

      mask = ffs (mask) - 1;
      mask <<= 12;
    }
  else if (writeback)
    inst.instruction |= WRITE_BACK;

  inst.instruction |= mask;
  if (do_io)
    inst.instruction |= base << 16;
}

static void
do_t_ldmstm (void)
{
  /* This really doesn't seem worth it.  */
  constraint (inst.relocs[0].type != BFD_RELOC_UNUSED,
	      _("expression too complex"));
  constraint (inst.operands[1].writeback,
	      _("Thumb load/store multiple does not support {reglist}^"));

  if (unified_syntax)
    {
      bool narrow;
      unsigned mask;

      narrow = false;
      /* See if we can use a 16-bit instruction.  */
      if (inst.instruction < 0xffff /* not ldmdb/stmdb */
	  && inst.size_req != 4
	  && !(inst.operands[1].imm & ~0xff))
	{
	  mask = 1 << inst.operands[0].reg;

	  if (inst.operands[0].reg <= 7)
	    {
	      if (inst.instruction == T_MNEM_stmia
		  ? inst.operands[0].writeback
		  : (inst.operands[0].writeback
		     == !(inst.operands[1].imm & mask)))
		{
		  if (inst.instruction == T_MNEM_stmia
		      && (inst.operands[1].imm & mask)
		      && (inst.operands[1].imm & (mask - 1)))
		    as_warn (_("value stored for r%d is UNKNOWN"),
			     inst.operands[0].reg);

		  inst.instruction = THUMB_OP16 (inst.instruction);
		  inst.instruction |= inst.operands[0].reg << 8;
		  inst.instruction |= inst.operands[1].imm;
		  narrow = true;
		}
	      else if ((inst.operands[1].imm & (inst.operands[1].imm-1)) == 0)
		{
		  /* This means 1 register in reg list one of 3 situations:
		     1. Instruction is stmia, but without writeback.
		     2. lmdia without writeback, but with Rn not in
			reglist.
		     3. ldmia with writeback, but with Rn in reglist.
		     Case 3 is UNPREDICTABLE behaviour, so we handle
		     case 1 and 2 which can be converted into a 16-bit
		     str or ldr. The SP cases are handled below.  */
		  unsigned long opcode;
		  /* First, record an error for Case 3.  */
		  if (inst.operands[1].imm & mask
		      && inst.operands[0].writeback)
		    inst.error =
			_("having the base register in the register list when "
			  "using write back is UNPREDICTABLE");

		  opcode = (inst.instruction == T_MNEM_stmia ? T_MNEM_str
							     : T_MNEM_ldr);
		  inst.instruction = THUMB_OP16 (opcode);
		  inst.instruction |= inst.operands[0].reg << 3;
		  inst.instruction |= (ffs (inst.operands[1].imm)-1);
		  narrow = true;
		}
	    }
	  else if (inst.operands[0] .reg == REG_SP)
	    {
	      if (inst.operands[0].writeback)
		{
		  inst.instruction =
			THUMB_OP16 (inst.instruction == T_MNEM_stmia
				    ? T_MNEM_push : T_MNEM_pop);
		  inst.instruction |= inst.operands[1].imm;
		  narrow = true;
		}
	      else if ((inst.operands[1].imm & (inst.operands[1].imm-1)) == 0)
		{
		  inst.instruction =
			THUMB_OP16 (inst.instruction == T_MNEM_stmia
				    ? T_MNEM_str_sp : T_MNEM_ldr_sp);
		  inst.instruction |= ((ffs (inst.operands[1].imm)-1) << 8);
		  narrow = true;
		}
	    }
	}

      if (!narrow)
	{
	  if (inst.instruction < 0xffff)
	    inst.instruction = THUMB_OP32 (inst.instruction);

	  encode_thumb2_multi (true /* do_io */, inst.operands[0].reg,
			       inst.operands[1].imm,
			       inst.operands[0].writeback);
	}
    }
  else
    {
      constraint (inst.operands[0].reg > 7
		  || (inst.operands[1].imm & ~0xff), BAD_HIREG);
      constraint (inst.instruction != T_MNEM_ldmia
		  && inst.instruction != T_MNEM_stmia,
		  _("Thumb-2 instruction only valid in unified syntax"));
      if (inst.instruction == T_MNEM_stmia)
	{
	  if (!inst.operands[0].writeback)
	    as_warn (_("this instruction will write back the base register"));
	  if ((inst.operands[1].imm & (1 << inst.operands[0].reg))
	      && (inst.operands[1].imm & ((1 << inst.operands[0].reg) - 1)))
	    as_warn (_("value stored for r%d is UNKNOWN"),
		     inst.operands[0].reg);
	}
      else
	{
	  if (!inst.operands[0].writeback
	      && !(inst.operands[1].imm & (1 << inst.operands[0].reg)))
	    as_warn (_("this instruction will write back the base register"));
	  else if (inst.operands[0].writeback
		   && (inst.operands[1].imm & (1 << inst.operands[0].reg)))
	    as_warn (_("this instruction will not write back the base register"));
	}

      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= inst.operands[0].reg << 8;
      inst.instruction |= inst.operands[1].imm;
    }
}

static void
do_t_ldrex (void)
{
  constraint (!inst.operands[1].isreg || !inst.operands[1].preind
	      || inst.operands[1].postind || inst.operands[1].writeback
	      || inst.operands[1].immisreg || inst.operands[1].shifted
	      || inst.operands[1].negative,
	      BAD_ADDR_MODE);

  constraint ((inst.operands[1].reg == REG_PC), BAD_PC);

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.relocs[0].type = BFD_RELOC_ARM_T32_OFFSET_U8;
}

static void
do_t_ldrexd (void)
{
  if (!inst.operands[1].present)
    {
      constraint (inst.operands[0].reg == REG_LR,
		  _("r14 not allowed as first register "
		    "when second register is omitted"));
      inst.operands[1].reg = inst.operands[0].reg + 1;
    }
  constraint (inst.operands[0].reg == inst.operands[1].reg,
	      BAD_OVERLAP);

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 8;
  inst.instruction |= inst.operands[2].reg << 16;
}

static void
do_t_ldst (void)
{
  unsigned long opcode;
  int Rn;

  if (inst.operands[0].isreg
      && !inst.operands[0].preind
      && inst.operands[0].reg == REG_PC)
    set_pred_insn_type_last ();

  opcode = inst.instruction;
  if (unified_syntax)
    {
      if (!inst.operands[1].isreg)
	{
	  if (opcode <= 0xffff)
	    inst.instruction = THUMB_OP32 (opcode);
	  if (move_or_literal_pool (0, CONST_THUMB, /*mode_3=*/false))
	    return;
	}
      if (inst.operands[1].isreg
	  && !inst.operands[1].writeback
	  && !inst.operands[1].shifted && !inst.operands[1].postind
	  && !inst.operands[1].negative && inst.operands[0].reg <= 7
	  && opcode <= 0xffff
	  && inst.size_req != 4)
	{
	  /* Insn may have a 16-bit form.  */
	  Rn = inst.operands[1].reg;
	  if (inst.operands[1].immisreg)
	    {
	      inst.instruction = THUMB_OP16 (opcode);
	      /* [Rn, Rik] */
	      if (Rn <= 7 && inst.operands[1].imm <= 7)
		goto op16;
	      else if (opcode != T_MNEM_ldr && opcode != T_MNEM_str)
		reject_bad_reg (inst.operands[1].imm);
	    }
	  else if ((Rn <= 7 && opcode != T_MNEM_ldrsh
		    && opcode != T_MNEM_ldrsb)
		   || ((Rn == REG_PC || Rn == REG_SP) && opcode == T_MNEM_ldr)
		   || (Rn == REG_SP && opcode == T_MNEM_str))
	    {
	      /* [Rn, #const] */
	      if (Rn > 7)
		{
		  if (Rn == REG_PC)
		    {
		      if (inst.relocs[0].pc_rel)
			opcode = T_MNEM_ldr_pc2;
		      else
			opcode = T_MNEM_ldr_pc;
		    }
		  else
		    {
		      if (opcode == T_MNEM_ldr)
			opcode = T_MNEM_ldr_sp;
		      else
			opcode = T_MNEM_str_sp;
		    }
		  inst.instruction = inst.operands[0].reg << 8;
		}
	      else
		{
		  inst.instruction = inst.operands[0].reg;
		  inst.instruction |= inst.operands[1].reg << 3;
		}
	      inst.instruction |= THUMB_OP16 (opcode);
	      if (inst.size_req == 2)
		inst.relocs[0].type = BFD_RELOC_ARM_THUMB_OFFSET;
	      else
		inst.relax = opcode;
	      return;
	    }
	}
      /* Definitely a 32-bit variant.  */

      /* Warning for Erratum 752419.  */
      if (opcode == T_MNEM_ldr
	  && inst.operands[0].reg == REG_SP
	  && inst.operands[1].writeback == 1
	  && !inst.operands[1].immisreg)
	{
	  if (no_cpu_selected ()
	      || (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v7)
		  && !ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v7a)
		  && !ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v7r)))
	    as_warn (_("This instruction may be unpredictable "
		       "if executed on M-profile cores "
		       "with interrupts enabled."));
	}

      /* Do some validations regarding addressing modes.  */
      if (inst.operands[1].immisreg)
	reject_bad_reg (inst.operands[1].imm);

      constraint (inst.operands[1].writeback == 1
		  && inst.operands[0].reg == inst.operands[1].reg,
		  BAD_OVERLAP);

      inst.instruction = THUMB_OP32 (opcode);
      inst.instruction |= inst.operands[0].reg << 12;
      encode_thumb32_addr_mode (1, /*is_t=*/false, /*is_d=*/false);
      check_ldr_r15_aligned ();
      return;
    }

  constraint (inst.operands[0].reg > 7, BAD_HIREG);

  if (inst.instruction == T_MNEM_ldrsh || inst.instruction == T_MNEM_ldrsb)
    {
      /* Only [Rn,Rm] is acceptable.  */
      constraint (inst.operands[1].reg > 7 || inst.operands[1].imm > 7, BAD_HIREG);
      constraint (!inst.operands[1].isreg || !inst.operands[1].immisreg
		  || inst.operands[1].postind || inst.operands[1].shifted
		  || inst.operands[1].negative,
		  _("Thumb does not support this addressing mode"));
      inst.instruction = THUMB_OP16 (inst.instruction);
      goto op16;
    }

  inst.instruction = THUMB_OP16 (inst.instruction);
  if (!inst.operands[1].isreg)
    if (move_or_literal_pool (0, CONST_THUMB, /*mode_3=*/false))
      return;

  constraint (!inst.operands[1].preind
	      || inst.operands[1].shifted
	      || inst.operands[1].writeback,
	      _("Thumb does not support this addressing mode"));
  if (inst.operands[1].reg == REG_PC || inst.operands[1].reg == REG_SP)
    {
      constraint (inst.instruction & 0x0600,
		  _("byte or halfword not valid for base register"));
      constraint (inst.operands[1].reg == REG_PC
		  && !(inst.instruction & THUMB_LOAD_BIT),
		  _("r15 based store not allowed"));
      constraint (inst.operands[1].immisreg,
		  _("invalid base register for register offset"));

      if (inst.operands[1].reg == REG_PC)
	inst.instruction = T_OPCODE_LDR_PC;
      else if (inst.instruction & THUMB_LOAD_BIT)
	inst.instruction = T_OPCODE_LDR_SP;
      else
	inst.instruction = T_OPCODE_STR_SP;

      inst.instruction |= inst.operands[0].reg << 8;
      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_OFFSET;
      return;
    }

  constraint (inst.operands[1].reg > 7, BAD_HIREG);
  if (!inst.operands[1].immisreg)
    {
      /* Immediate offset.  */
      inst.instruction |= inst.operands[0].reg;
      inst.instruction |= inst.operands[1].reg << 3;
      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_OFFSET;
      return;
    }

  /* Register offset.  */
  constraint (inst.operands[1].imm > 7, BAD_HIREG);
  constraint (inst.operands[1].negative,
	      _("Thumb does not support this addressing mode"));

 op16:
  switch (inst.instruction)
    {
    case T_OPCODE_STR_IW: inst.instruction = T_OPCODE_STR_RW; break;
    case T_OPCODE_STR_IH: inst.instruction = T_OPCODE_STR_RH; break;
    case T_OPCODE_STR_IB: inst.instruction = T_OPCODE_STR_RB; break;
    case T_OPCODE_LDR_IW: inst.instruction = T_OPCODE_LDR_RW; break;
    case T_OPCODE_LDR_IH: inst.instruction = T_OPCODE_LDR_RH; break;
    case T_OPCODE_LDR_IB: inst.instruction = T_OPCODE_LDR_RB; break;
    case 0x5600 /* ldrsb */:
    case 0x5e00 /* ldrsh */: break;
    default: abort ();
    }

  inst.instruction |= inst.operands[0].reg;
  inst.instruction |= inst.operands[1].reg << 3;
  inst.instruction |= inst.operands[1].imm << 6;
}

static void
do_t_ldstd (void)
{
  if (!inst.operands[1].present)
    {
      inst.operands[1].reg = inst.operands[0].reg + 1;
      constraint (inst.operands[0].reg == REG_LR,
		  _("r14 not allowed here"));
      constraint (inst.operands[0].reg == REG_R12,
		  _("r12 not allowed here"));
    }

  if (inst.operands[2].writeback
      && (inst.operands[0].reg == inst.operands[2].reg
      || inst.operands[1].reg == inst.operands[2].reg))
    as_warn (_("base register written back, and overlaps "
	       "one of transfer registers"));

  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 8;
  encode_thumb32_addr_mode (2, /*is_t=*/false, /*is_d=*/true);
}

static void
do_t_ldstt (void)
{
  inst.instruction |= inst.operands[0].reg << 12;
  encode_thumb32_addr_mode (1, /*is_t=*/true, /*is_d=*/false);
}

static void
do_t_mla (void)
{
  unsigned Rd, Rn, Rm, Ra;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[1].reg;
  Rm = inst.operands[2].reg;
  Ra = inst.operands[3].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);
  reject_bad_reg (Rm);
  reject_bad_reg (Ra);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;
  inst.instruction |= Ra << 12;
}

static void
do_t_mlal (void)
{
  unsigned RdLo, RdHi, Rn, Rm;

  RdLo = inst.operands[0].reg;
  RdHi = inst.operands[1].reg;
  Rn = inst.operands[2].reg;
  Rm = inst.operands[3].reg;

  reject_bad_reg (RdLo);
  reject_bad_reg (RdHi);
  reject_bad_reg (Rn);
  reject_bad_reg (Rm);

  inst.instruction |= RdLo << 12;
  inst.instruction |= RdHi << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;
}

static void
do_t_mov_cmp (void)
{
  unsigned Rn, Rm;

  Rn = inst.operands[0].reg;
  Rm = inst.operands[1].reg;

  if (Rn == REG_PC)
    set_pred_insn_type_last ();

  if (unified_syntax)
    {
      int r0off = (inst.instruction == T_MNEM_mov
		   || inst.instruction == T_MNEM_movs) ? 8 : 16;
      unsigned long opcode;
      bool narrow;
      bool low_regs;

      low_regs = (Rn <= 7 && Rm <= 7);
      opcode = inst.instruction;
      if (in_pred_block ())
	narrow = opcode != T_MNEM_movs;
      else
	narrow = opcode != T_MNEM_movs || low_regs;
      if (inst.size_req == 4
	  || inst.operands[1].shifted)
	narrow = false;

      /* MOVS PC, LR is encoded as SUBS PC, LR, #0.  */
      if (opcode == T_MNEM_movs && inst.operands[1].isreg
	  && !inst.operands[1].shifted
	  && Rn == REG_PC
	  && Rm == REG_LR)
	{
	  inst.instruction = T2_SUBS_PC_LR;
	  return;
	}

      if (opcode == T_MNEM_cmp)
	{
	  constraint (Rn == REG_PC, BAD_PC);
	  if (narrow)
	    {
	      /* In the Thumb-2 ISA, use of R13 as Rm is deprecated,
		 but valid.  */
	      warn_deprecated_sp (Rm);
	      /* R15 was documented as a valid choice for Rm in ARMv6,
		 but as UNPREDICTABLE in ARMv7.  ARM's proprietary
		 tools reject R15, so we do too.  */
	      constraint (Rm == REG_PC, BAD_PC);
	    }
	  else
	    reject_bad_reg (Rm);
	}
      else if (opcode == T_MNEM_mov
	       || opcode == T_MNEM_movs)
	{
	  if (inst.operands[1].isreg)
	    {
	      if (opcode == T_MNEM_movs)
		{
		  reject_bad_reg (Rn);
		  reject_bad_reg (Rm);
		}
	      else if (narrow)
		{
		  /* This is mov.n.  */
		  if ((Rn == REG_SP || Rn == REG_PC)
		      && (Rm == REG_SP || Rm == REG_PC))
		    {
		      as_tsktsk (_("Use of r%u as a source register is "
				 "deprecated when r%u is the destination "
				 "register."), Rm, Rn);
		    }
		}
	      else
		{
		  /* This is mov.w.  */
		  constraint (Rn == REG_PC, BAD_PC);
		  constraint (Rm == REG_PC, BAD_PC);
		  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))
		    constraint (Rn == REG_SP && Rm == REG_SP, BAD_SP);
		}
	    }
	  else
	    reject_bad_reg (Rn);
	}

      if (!inst.operands[1].isreg)
	{
	  /* Immediate operand.  */
	  if (!in_pred_block () && opcode == T_MNEM_mov)
	    narrow = 0;
	  if (low_regs && narrow)
	    {
	      inst.instruction = THUMB_OP16 (opcode);
	      inst.instruction |= Rn << 8;
	      if (inst.relocs[0].type < BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC
		  || inst.relocs[0].type > BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC)
		{
		  if (inst.size_req == 2)
		    inst.relocs[0].type = BFD_RELOC_ARM_THUMB_IMM;
		  else
		    inst.relax = opcode;
		}
	    }
	  else
	    {
	      constraint ((inst.relocs[0].type
			   >= BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC)
			  && (inst.relocs[0].type
			      <= BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC) ,
			  THUMB1_RELOC_ONLY);

	      inst.instruction = THUMB_OP32 (inst.instruction);
	      inst.instruction = (inst.instruction & 0xe1ffffff) | 0x10000000;
	      inst.instruction |= Rn << r0off;
	      inst.relocs[0].type = BFD_RELOC_ARM_T32_IMMEDIATE;
	    }
	}
      else if (inst.operands[1].shifted && inst.operands[1].immisreg
	       && (inst.instruction == T_MNEM_mov
		   || inst.instruction == T_MNEM_movs))
	{
	  /* Register shifts are encoded as separate shift instructions.  */
	  bool flags = (inst.instruction == T_MNEM_movs);

	  if (in_pred_block ())
	    narrow = !flags;
	  else
	    narrow = flags;

	  if (inst.size_req == 4)
	    narrow = false;

	  if (!low_regs || inst.operands[1].imm > 7)
	    narrow = false;

	  if (Rn != Rm)
	    narrow = false;

	  switch (inst.operands[1].shift_kind)
	    {
	    case SHIFT_LSL:
	      opcode = narrow ? T_OPCODE_LSL_R : THUMB_OP32 (T_MNEM_lsl);
	      break;
	    case SHIFT_ASR:
	      opcode = narrow ? T_OPCODE_ASR_R : THUMB_OP32 (T_MNEM_asr);
	      break;
	    case SHIFT_LSR:
	      opcode = narrow ? T_OPCODE_LSR_R : THUMB_OP32 (T_MNEM_lsr);
	      break;
	    case SHIFT_ROR:
	      opcode = narrow ? T_OPCODE_ROR_R : THUMB_OP32 (T_MNEM_ror);
	      break;
	    default:
	      abort ();
	    }

	  inst.instruction = opcode;
	  if (narrow)
	    {
	      inst.instruction |= Rn;
	      inst.instruction |= inst.operands[1].imm << 3;
	    }
	  else
	    {
	      if (flags)
		inst.instruction |= CONDS_BIT;

	      inst.instruction |= Rn << 8;
	      inst.instruction |= Rm << 16;
	      inst.instruction |= inst.operands[1].imm;
	    }
	}
      else if (!narrow)
	{
	  /* Some mov with immediate shift have narrow variants.
	     Register shifts are handled above.  */
	  if (low_regs && inst.operands[1].shifted
	      && (inst.instruction == T_MNEM_mov
		  || inst.instruction == T_MNEM_movs))
	    {
	      if (in_pred_block ())
		narrow = (inst.instruction == T_MNEM_mov);
	      else
		narrow = (inst.instruction == T_MNEM_movs);
	    }

	  if (narrow)
	    {
	      switch (inst.operands[1].shift_kind)
		{
		case SHIFT_LSL: inst.instruction = T_OPCODE_LSL_I; break;
		case SHIFT_LSR: inst.instruction = T_OPCODE_LSR_I; break;
		case SHIFT_ASR: inst.instruction = T_OPCODE_ASR_I; break;
		default: narrow = false; break;
		}
	    }

	  if (narrow)
	    {
	      inst.instruction |= Rn;
	      inst.instruction |= Rm << 3;
	      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_SHIFT;
	    }
	  else
	    {
	      inst.instruction = THUMB_OP32 (inst.instruction);
	      inst.instruction |= Rn << r0off;
	      encode_thumb32_shifted_operand (1);
	    }
	}
      else
	switch (inst.instruction)
	  {
	  case T_MNEM_mov:
	    /* In v4t or v5t a move of two lowregs produces unpredictable
	       results. Don't allow this.  */
	    if (low_regs)
	      {
		constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6),
			    "MOV Rd, Rs with two low registers is not "
			    "permitted on this architecture");
		ARM_MERGE_FEATURE_SETS (thumb_arch_used, thumb_arch_used,
					arm_ext_v6);
	      }

	    inst.instruction = T_OPCODE_MOV_HR;
	    inst.instruction |= (Rn & 0x8) << 4;
	    inst.instruction |= (Rn & 0x7);
	    inst.instruction |= Rm << 3;
	    break;

	  case T_MNEM_movs:
	    /* We know we have low registers at this point.
	       Generate LSLS Rd, Rs, #0.  */
	    inst.instruction = T_OPCODE_LSL_I;
	    inst.instruction |= Rn;
	    inst.instruction |= Rm << 3;
	    break;

	  case T_MNEM_cmp:
	    if (low_regs)
	      {
		inst.instruction = T_OPCODE_CMP_LR;
		inst.instruction |= Rn;
		inst.instruction |= Rm << 3;
	      }
	    else
	      {
		inst.instruction = T_OPCODE_CMP_HR;
		inst.instruction |= (Rn & 0x8) << 4;
		inst.instruction |= (Rn & 0x7);
		inst.instruction |= Rm << 3;
	      }
	    break;
	  }
      return;
    }

  inst.instruction = THUMB_OP16 (inst.instruction);

  /* PR 10443: Do not silently ignore shifted operands.  */
  constraint (inst.operands[1].shifted,
	      _("shifts in CMP/MOV instructions are only supported in unified syntax"));

  if (inst.operands[1].isreg)
    {
      if (Rn < 8 && Rm < 8)
	{
	  /* A move of two lowregs is encoded as ADD Rd, Rs, #0
	     since a MOV instruction produces unpredictable results.  */
	  if (inst.instruction == T_OPCODE_MOV_I8)
	    inst.instruction = T_OPCODE_ADD_I3;
	  else
	    inst.instruction = T_OPCODE_CMP_LR;

	  inst.instruction |= Rn;
	  inst.instruction |= Rm << 3;
	}
      else
	{
	  if (inst.instruction == T_OPCODE_MOV_I8)
	    inst.instruction = T_OPCODE_MOV_HR;
	  else
	    inst.instruction = T_OPCODE_CMP_HR;
	  do_t_cpy ();
	}
    }
  else
    {
      constraint (Rn > 7,
		  _("only lo regs allowed with immediate"));
      inst.instruction |= Rn << 8;
      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_IMM;
    }
}

static void
do_t_mov16 (void)
{
  unsigned Rd;
  bfd_vma imm;
  bool top;

  top = (inst.instruction & 0x00800000) != 0;
  if (inst.relocs[0].type == BFD_RELOC_ARM_MOVW)
    {
      constraint (top, _(":lower16: not allowed in this instruction"));
      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_MOVW;
    }
  else if (inst.relocs[0].type == BFD_RELOC_ARM_MOVT)
    {
      constraint (!top, _(":upper16: not allowed in this instruction"));
      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_MOVT;
    }

  Rd = inst.operands[0].reg;
  reject_bad_reg (Rd);

  inst.instruction |= Rd << 8;
  if (inst.relocs[0].type == BFD_RELOC_UNUSED)
    {
      imm = inst.relocs[0].exp.X_add_number;
      inst.instruction |= (imm & 0xf000) << 4;
      inst.instruction |= (imm & 0x0800) << 15;
      inst.instruction |= (imm & 0x0700) << 4;
      inst.instruction |= (imm & 0x00ff);
    }
}

static void
do_t_mvn_tst (void)
{
  unsigned Rn, Rm;

  Rn = inst.operands[0].reg;
  Rm = inst.operands[1].reg;

  if (inst.instruction == T_MNEM_cmp
      || inst.instruction == T_MNEM_cmn)
    constraint (Rn == REG_PC, BAD_PC);
  else
    reject_bad_reg (Rn);
  reject_bad_reg (Rm);

  if (unified_syntax)
    {
      int r0off = (inst.instruction == T_MNEM_mvn
		   || inst.instruction == T_MNEM_mvns) ? 8 : 16;
      bool narrow;

      if (inst.size_req == 4
	  || inst.instruction > 0xffff
	  || inst.operands[1].shifted
	  || Rn > 7 || Rm > 7)
	narrow = false;
      else if (inst.instruction == T_MNEM_cmn
	       || inst.instruction == T_MNEM_tst)
	narrow = true;
      else if (THUMB_SETS_FLAGS (inst.instruction))
	narrow = !in_pred_block ();
      else
	narrow = in_pred_block ();

      if (!inst.operands[1].isreg)
	{
	  /* For an immediate, we always generate a 32-bit opcode;
	     section relaxation will shrink it later if possible.  */
	  if (inst.instruction < 0xffff)
	    inst.instruction = THUMB_OP32 (inst.instruction);
	  inst.instruction = (inst.instruction & 0xe1ffffff) | 0x10000000;
	  inst.instruction |= Rn << r0off;
	  inst.relocs[0].type = BFD_RELOC_ARM_T32_IMMEDIATE;
	}
      else
	{
	  /* See if we can do this with a 16-bit instruction.  */
	  if (narrow)
	    {
	      inst.instruction = THUMB_OP16 (inst.instruction);
	      inst.instruction |= Rn;
	      inst.instruction |= Rm << 3;
	    }
	  else
	    {
	      constraint (inst.operands[1].shifted
			  && inst.operands[1].immisreg,
			  _("shift must be constant"));
	      if (inst.instruction < 0xffff)
		inst.instruction = THUMB_OP32 (inst.instruction);
	      inst.instruction |= Rn << r0off;
	      encode_thumb32_shifted_operand (1);
	    }
	}
    }
  else
    {
      constraint (inst.instruction > 0xffff
		  || inst.instruction == T_MNEM_mvns, BAD_THUMB32);
      constraint (!inst.operands[1].isreg || inst.operands[1].shifted,
		  _("unshifted register required"));
      constraint (Rn > 7 || Rm > 7,
		  BAD_HIREG);

      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= Rn;
      inst.instruction |= Rm << 3;
    }
}

static void
do_t_mrs (void)
{
  unsigned Rd;

  if (do_vfp_nsyn_mrs () == SUCCESS)
    return;

  Rd = inst.operands[0].reg;
  reject_bad_reg (Rd);
  inst.instruction |= Rd << 8;

  if (inst.operands[1].isreg)
    {
      unsigned br = inst.operands[1].reg;
      if (((br & 0x200) == 0) && ((br & 0xf000) != 0xf000))
	as_bad (_("bad register for mrs"));

      inst.instruction |= br & (0xf << 16);
      inst.instruction |= (br & 0x300) >> 4;
      inst.instruction |= (br & SPSR_BIT) >> 2;
    }
  else
    {
      int flags = inst.operands[1].imm & (PSR_c|PSR_x|PSR_s|PSR_f|SPSR_BIT);

      if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_m))
	{
	  /* PR gas/12698:  The constraint is only applied for m_profile.
	     If the user has specified -march=all, we want to ignore it as
	     we are building for any CPU type, including non-m variants.  */
	  bool m_profile =
	    !ARM_FEATURE_CORE_EQUAL (selected_cpu, arm_arch_any);
	  constraint ((flags != 0) && m_profile, _("selected processor does "
						   "not support requested special purpose register"));
	}
      else
	/* mrs only accepts APSR/CPSR/SPSR/CPSR_all/SPSR_all (for non-M profile
	   devices).  */
	constraint ((flags & ~SPSR_BIT) != (PSR_c|PSR_f),
		    _("'APSR', 'CPSR' or 'SPSR' expected"));

      inst.instruction |= (flags & SPSR_BIT) >> 2;
      inst.instruction |= inst.operands[1].imm & 0xff;
      inst.instruction |= 0xf0000;
    }
}

static void
do_t_msr (void)
{
  int flags;
  unsigned Rn;

  if (do_vfp_nsyn_msr () == SUCCESS)
    return;

  constraint (!inst.operands[1].isreg,
	      _("Thumb encoding does not support an immediate here"));

  if (inst.operands[0].isreg)
    flags = (int)(inst.operands[0].reg);
  else
    flags = inst.operands[0].imm;

  if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_m))
    {
      int bits = inst.operands[0].imm & (PSR_c|PSR_x|PSR_s|PSR_f|SPSR_BIT);

      /* PR gas/12698:  The constraint is only applied for m_profile.
	 If the user has specified -march=all, we want to ignore it as
	 we are building for any CPU type, including non-m variants.  */
      bool m_profile =
	!ARM_FEATURE_CORE_EQUAL (selected_cpu, arm_arch_any);
      constraint (((ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6_dsp)
	   && (bits & ~(PSR_s | PSR_f)) != 0)
	  || (!ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6_dsp)
	      && bits != PSR_f)) && m_profile,
	  _("selected processor does not support requested special "
	    "purpose register"));
    }
  else
     constraint ((flags & 0xff) != 0, _("selected processor does not support "
		 "requested special purpose register"));

  Rn = inst.operands[1].reg;
  reject_bad_reg (Rn);

  inst.instruction |= (flags & SPSR_BIT) >> 2;
  inst.instruction |= (flags & 0xf0000) >> 8;
  inst.instruction |= (flags & 0x300) >> 4;
  inst.instruction |= (flags & 0xff);
  inst.instruction |= Rn << 16;
}

static void
do_t_mul (void)
{
  bool narrow;
  unsigned Rd, Rn, Rm;

  if (!inst.operands[2].present)
    inst.operands[2].reg = inst.operands[0].reg;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[1].reg;
  Rm = inst.operands[2].reg;

  if (unified_syntax)
    {
      if (inst.size_req == 4
	  || (Rd != Rn
	      && Rd != Rm)
	  || Rn > 7
	  || Rm > 7)
	narrow = false;
      else if (inst.instruction == T_MNEM_muls)
	narrow = !in_pred_block ();
      else
	narrow = in_pred_block ();
    }
  else
    {
      constraint (inst.instruction == T_MNEM_muls, BAD_THUMB32);
      constraint (Rn > 7 || Rm > 7,
		  BAD_HIREG);
      narrow = true;
    }

  if (narrow)
    {
      /* 16-bit MULS/Conditional MUL.  */
      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= Rd;

      if (Rd == Rn)
	inst.instruction |= Rm << 3;
      else if (Rd == Rm)
	inst.instruction |= Rn << 3;
      else
	constraint (1, _("dest must overlap one source register"));
    }
  else
    {
      constraint (inst.instruction != T_MNEM_mul,
		  _("Thumb-2 MUL must not set flags"));
      /* 32-bit MUL.  */
      inst.instruction = THUMB_OP32 (inst.instruction);
      inst.instruction |= Rd << 8;
      inst.instruction |= Rn << 16;
      inst.instruction |= Rm << 0;

      reject_bad_reg (Rd);
      reject_bad_reg (Rn);
      reject_bad_reg (Rm);
    }
}

static void
do_t_mull (void)
{
  unsigned RdLo, RdHi, Rn, Rm;

  RdLo = inst.operands[0].reg;
  RdHi = inst.operands[1].reg;
  Rn = inst.operands[2].reg;
  Rm = inst.operands[3].reg;

  reject_bad_reg (RdLo);
  reject_bad_reg (RdHi);
  reject_bad_reg (Rn);
  reject_bad_reg (Rm);

  inst.instruction |= RdLo << 12;
  inst.instruction |= RdHi << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;

 if (RdLo == RdHi)
    as_tsktsk (_("rdhi and rdlo must be different"));
}

static void
do_t_nop (void)
{
  set_pred_insn_type (NEUTRAL_IT_INSN);

  if (unified_syntax)
    {
      if (inst.size_req == 4 || inst.operands[0].imm > 15)
	{
	  inst.instruction = THUMB_OP32 (inst.instruction);
	  inst.instruction |= inst.operands[0].imm;
	}
      else
	{
	  /* PR9722: Check for Thumb2 availability before
	     generating a thumb2 nop instruction.  */
	  if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6t2))
	    {
	      inst.instruction = THUMB_OP16 (inst.instruction);
	      inst.instruction |= inst.operands[0].imm << 4;
	    }
	  else
	    inst.instruction = 0x46c0;
	}
    }
  else
    {
      constraint (inst.operands[0].present,
		  _("Thumb does not support NOP with hints"));
      inst.instruction = 0x46c0;
    }
}

static void
do_t_neg (void)
{
  if (unified_syntax)
    {
      bool narrow;

      if (THUMB_SETS_FLAGS (inst.instruction))
	narrow = !in_pred_block ();
      else
	narrow = in_pred_block ();
      if (inst.operands[0].reg > 7 || inst.operands[1].reg > 7)
	narrow = false;
      if (inst.size_req == 4)
	narrow = false;

      if (!narrow)
	{
	  inst.instruction = THUMB_OP32 (inst.instruction);
	  inst.instruction |= inst.operands[0].reg << 8;
	  inst.instruction |= inst.operands[1].reg << 16;
	}
      else
	{
	  inst.instruction = THUMB_OP16 (inst.instruction);
	  inst.instruction |= inst.operands[0].reg;
	  inst.instruction |= inst.operands[1].reg << 3;
	}
    }
  else
    {
      constraint (inst.operands[0].reg > 7 || inst.operands[1].reg > 7,
		  BAD_HIREG);
      constraint (THUMB_SETS_FLAGS (inst.instruction), BAD_THUMB32);

      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= inst.operands[0].reg;
      inst.instruction |= inst.operands[1].reg << 3;
    }
}

static void
do_t_orn (void)
{
  unsigned Rd, Rn;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[1].present ? inst.operands[1].reg : Rd;

  reject_bad_reg (Rd);
  /* Rn == REG_SP is unpredictable; Rn == REG_PC is MVN.  */
  reject_bad_reg (Rn);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;

  if (!inst.operands[2].isreg)
    {
      inst.instruction = (inst.instruction & 0xe1ffffff) | 0x10000000;
      inst.relocs[0].type = BFD_RELOC_ARM_T32_IMMEDIATE;
    }
  else
    {
      unsigned Rm;

      Rm = inst.operands[2].reg;
      reject_bad_reg (Rm);

      constraint (inst.operands[2].shifted
		  && inst.operands[2].immisreg,
		  _("shift must be constant"));
      encode_thumb32_shifted_operand (2);
    }
}

static void
do_t_pkhbt (void)
{
  unsigned Rd, Rn, Rm;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[1].reg;
  Rm = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);
  reject_bad_reg (Rm);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;
  if (inst.operands[3].present)
    {
      unsigned int val = inst.relocs[0].exp.X_add_number;
      constraint (inst.relocs[0].exp.X_op != O_constant,
		  _("expression too complex"));
      inst.instruction |= (val & 0x1c) << 10;
      inst.instruction |= (val & 0x03) << 6;
    }
}

static void
do_t_pkhtb (void)
{
  if (!inst.operands[3].present)
    {
      unsigned Rtmp;

      inst.instruction &= ~0x00000020;

      /* PR 10168.  Swap the Rm and Rn registers.  */
      Rtmp = inst.operands[1].reg;
      inst.operands[1].reg = inst.operands[2].reg;
      inst.operands[2].reg = Rtmp;
    }
  do_t_pkhbt ();
}

static void
do_t_pld (void)
{
  if (inst.operands[0].immisreg)
    reject_bad_reg (inst.operands[0].imm);

  encode_thumb32_addr_mode (0, /*is_t=*/false, /*is_d=*/false);
}

static void
do_t_push_pop (void)
{
  unsigned mask;

  constraint (inst.operands[0].writeback,
	      _("push/pop do not support {reglist}^"));
  constraint (inst.relocs[0].type != BFD_RELOC_UNUSED,
	      _("expression too complex"));

  mask = inst.operands[0].imm;
  if (inst.size_req != 4 && (mask & ~0xff) == 0)
    inst.instruction = THUMB_OP16 (inst.instruction) | mask;
  else if (inst.size_req != 4
	   && (mask & ~0xff) == (1U << (inst.instruction == T_MNEM_push
				       ? REG_LR : REG_PC)))
    {
      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= THUMB_PP_PC_LR;
      inst.instruction |= mask & 0xff;
    }
  else if (unified_syntax)
    {
      inst.instruction = THUMB_OP32 (inst.instruction);
      encode_thumb2_multi (true /* do_io */, 13, mask, true);
    }
  else
    {
      inst.error = _("invalid register list to push/pop instruction");
      return;
    }
}

static void
do_t_clrm (void)
{
  if (unified_syntax)
    encode_thumb2_multi (false /* do_io */, -1, inst.operands[0].imm, false);
  else
    {
      inst.error = _("invalid register list to push/pop instruction");
      return;
    }
}

static void
do_t_vscclrm (void)
{
  if (inst.operands[0].issingle)
    {
      inst.instruction |= (inst.operands[0].reg & 0x1) << 22;
      inst.instruction |= (inst.operands[0].reg & 0x1e) << 11;
      inst.instruction |= inst.operands[0].imm;
    }
  else
    {
      inst.instruction |= (inst.operands[0].reg & 0x10) << 18;
      inst.instruction |= (inst.operands[0].reg & 0xf) << 12;
      inst.instruction |= 1 << 8;
      inst.instruction |= inst.operands[0].imm << 1;
    }
}

static void
do_t_rbit (void)
{
  unsigned Rd, Rm;

  Rd = inst.operands[0].reg;
  Rm = inst.operands[1].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rm);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rm << 16;
  inst.instruction |= Rm;
}

static void
do_t_rev (void)
{
  unsigned Rd, Rm;

  Rd = inst.operands[0].reg;
  Rm = inst.operands[1].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rm);

  if (Rd <= 7 && Rm <= 7
      && inst.size_req != 4)
    {
      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= Rd;
      inst.instruction |= Rm << 3;
    }
  else if (unified_syntax)
    {
      inst.instruction = THUMB_OP32 (inst.instruction);
      inst.instruction |= Rd << 8;
      inst.instruction |= Rm << 16;
      inst.instruction |= Rm;
    }
  else
    inst.error = BAD_HIREG;
}

static void
do_t_rrx (void)
{
  unsigned Rd, Rm;

  Rd = inst.operands[0].reg;
  Rm = inst.operands[1].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rm);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rm;
}

static void
do_t_rsb (void)
{
  unsigned Rd, Rs;

  Rd = inst.operands[0].reg;
  Rs = (inst.operands[1].present
	? inst.operands[1].reg    /* Rd, Rs, foo */
	: inst.operands[0].reg);  /* Rd, foo -> Rd, Rd, foo */

  reject_bad_reg (Rd);
  reject_bad_reg (Rs);
  if (inst.operands[2].isreg)
    reject_bad_reg (inst.operands[2].reg);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rs << 16;
  if (!inst.operands[2].isreg)
    {
      bool narrow;

      if ((inst.instruction & 0x00100000) != 0)
	narrow = !in_pred_block ();
      else
	narrow = in_pred_block ();

      if (Rd > 7 || Rs > 7)
	narrow = false;

      if (inst.size_req == 4 || !unified_syntax)
	narrow = false;

      if (inst.relocs[0].exp.X_op != O_constant
	  || inst.relocs[0].exp.X_add_number != 0)
	narrow = false;

      /* Turn rsb #0 into 16-bit neg.  We should probably do this via
	 relaxation, but it doesn't seem worth the hassle.  */
      if (narrow)
	{
	  inst.relocs[0].type = BFD_RELOC_UNUSED;
	  inst.instruction = THUMB_OP16 (T_MNEM_negs);
	  inst.instruction |= Rs << 3;
	  inst.instruction |= Rd;
	}
      else
	{
	  inst.instruction = (inst.instruction & 0xe1ffffff) | 0x10000000;
	  inst.relocs[0].type = BFD_RELOC_ARM_T32_IMMEDIATE;
	}
    }
  else
    encode_thumb32_shifted_operand (2);
}

static void
do_t_setend (void)
{
  if (warn_on_deprecated
      && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))
      as_tsktsk (_("setend use is deprecated for ARMv8"));

  set_pred_insn_type (OUTSIDE_PRED_INSN);
  if (inst.operands[0].imm)
    inst.instruction |= 0x8;
}

static void
do_t_shift (void)
{
  if (!inst.operands[1].present)
    inst.operands[1].reg = inst.operands[0].reg;

  if (unified_syntax)
    {
      bool narrow;
      int shift_kind;

      switch (inst.instruction)
	{
	case T_MNEM_asr:
	case T_MNEM_asrs: shift_kind = SHIFT_ASR; break;
	case T_MNEM_lsl:
	case T_MNEM_lsls: shift_kind = SHIFT_LSL; break;
	case T_MNEM_lsr:
	case T_MNEM_lsrs: shift_kind = SHIFT_LSR; break;
	case T_MNEM_ror:
	case T_MNEM_rors: shift_kind = SHIFT_ROR; break;
	default: abort ();
	}

      if (THUMB_SETS_FLAGS (inst.instruction))
	narrow = !in_pred_block ();
      else
	narrow = in_pred_block ();
      if (inst.operands[0].reg > 7 || inst.operands[1].reg > 7)
	narrow = false;
      if (!inst.operands[2].isreg && shift_kind == SHIFT_ROR)
	narrow = false;
      if (inst.operands[2].isreg
	  && (inst.operands[1].reg != inst.operands[0].reg
	      || inst.operands[2].reg > 7))
	narrow = false;
      if (inst.size_req == 4)
	narrow = false;

      reject_bad_reg (inst.operands[0].reg);
      reject_bad_reg (inst.operands[1].reg);

      if (!narrow)
	{
	  if (inst.operands[2].isreg)
	    {
	      reject_bad_reg (inst.operands[2].reg);
	      inst.instruction = THUMB_OP32 (inst.instruction);
	      inst.instruction |= inst.operands[0].reg << 8;
	      inst.instruction |= inst.operands[1].reg << 16;
	      inst.instruction |= inst.operands[2].reg;

	      /* PR 12854: Error on extraneous shifts.  */
	      constraint (inst.operands[2].shifted,
			  _("extraneous shift as part of operand to shift insn"));
	    }
	  else
	    {
	      inst.operands[1].shifted = 1;
	      inst.operands[1].shift_kind = shift_kind;
	      inst.instruction = THUMB_OP32 (THUMB_SETS_FLAGS (inst.instruction)
					     ? T_MNEM_movs : T_MNEM_mov);
	      inst.instruction |= inst.operands[0].reg << 8;
	      encode_thumb32_shifted_operand (1);
	      /* Prevent the incorrect generation of an ARM_IMMEDIATE fixup.  */
	      inst.relocs[0].type = BFD_RELOC_UNUSED;
	    }
	}
      else
	{
	  if (inst.operands[2].isreg)
	    {
	      switch (shift_kind)
		{
		case SHIFT_ASR: inst.instruction = T_OPCODE_ASR_R; break;
		case SHIFT_LSL: inst.instruction = T_OPCODE_LSL_R; break;
		case SHIFT_LSR: inst.instruction = T_OPCODE_LSR_R; break;
		case SHIFT_ROR: inst.instruction = T_OPCODE_ROR_R; break;
		default: abort ();
		}

	      inst.instruction |= inst.operands[0].reg;
	      inst.instruction |= inst.operands[2].reg << 3;

	      /* PR 12854: Error on extraneous shifts.  */
	      constraint (inst.operands[2].shifted,
			  _("extraneous shift as part of operand to shift insn"));
	    }
	  else
	    {
	      switch (shift_kind)
		{
		case SHIFT_ASR: inst.instruction = T_OPCODE_ASR_I; break;
		case SHIFT_LSL: inst.instruction = T_OPCODE_LSL_I; break;
		case SHIFT_LSR: inst.instruction = T_OPCODE_LSR_I; break;
		default: abort ();
		}
	      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_SHIFT;
	      inst.instruction |= inst.operands[0].reg;
	      inst.instruction |= inst.operands[1].reg << 3;
	    }
	}
    }
  else
    {
      constraint (inst.operands[0].reg > 7
		  || inst.operands[1].reg > 7, BAD_HIREG);
      constraint (THUMB_SETS_FLAGS (inst.instruction), BAD_THUMB32);

      if (inst.operands[2].isreg)  /* Rd, {Rs,} Rn */
	{
	  constraint (inst.operands[2].reg > 7, BAD_HIREG);
	  constraint (inst.operands[0].reg != inst.operands[1].reg,
		      _("source1 and dest must be same register"));

	  switch (inst.instruction)
	    {
	    case T_MNEM_asr: inst.instruction = T_OPCODE_ASR_R; break;
	    case T_MNEM_lsl: inst.instruction = T_OPCODE_LSL_R; break;
	    case T_MNEM_lsr: inst.instruction = T_OPCODE_LSR_R; break;
	    case T_MNEM_ror: inst.instruction = T_OPCODE_ROR_R; break;
	    default: abort ();
	    }

	  inst.instruction |= inst.operands[0].reg;
	  inst.instruction |= inst.operands[2].reg << 3;

	  /* PR 12854: Error on extraneous shifts.  */
	  constraint (inst.operands[2].shifted,
		      _("extraneous shift as part of operand to shift insn"));
	}
      else
	{
	  switch (inst.instruction)
	    {
	    case T_MNEM_asr: inst.instruction = T_OPCODE_ASR_I; break;
	    case T_MNEM_lsl: inst.instruction = T_OPCODE_LSL_I; break;
	    case T_MNEM_lsr: inst.instruction = T_OPCODE_LSR_I; break;
	    case T_MNEM_ror: inst.error = _("ror #imm not supported"); return;
	    default: abort ();
	    }
	  inst.relocs[0].type = BFD_RELOC_ARM_THUMB_SHIFT;
	  inst.instruction |= inst.operands[0].reg;
	  inst.instruction |= inst.operands[1].reg << 3;
	}
    }
}

static void
do_t_simd (void)
{
  unsigned Rd, Rn, Rm;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[1].reg;
  Rm = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);
  reject_bad_reg (Rm);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;
}

static void
do_t_simd2 (void)
{
  unsigned Rd, Rn, Rm;

  Rd = inst.operands[0].reg;
  Rm = inst.operands[1].reg;
  Rn = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);
  reject_bad_reg (Rm);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;
}

static void
do_t_smc (void)
{
  unsigned int value = inst.relocs[0].exp.X_add_number;
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v7a),
	      _("SMC is not permitted on this architecture"));
  constraint (inst.relocs[0].exp.X_op != O_constant,
	      _("expression too complex"));
  constraint (value > 0xf, _("immediate too large (bigger than 0xF)"));

  inst.relocs[0].type = BFD_RELOC_UNUSED;
  inst.instruction |= (value & 0x000f) << 16;

  /* PR gas/15623: SMC instructions must be last in an IT block.  */
  set_pred_insn_type_last ();
}

static void
do_t_hvc (void)
{
  unsigned int value = inst.relocs[0].exp.X_add_number;

  inst.relocs[0].type = BFD_RELOC_UNUSED;
  inst.instruction |= (value & 0x0fff);
  inst.instruction |= (value & 0xf000) << 4;
}

static void
do_t_ssat_usat (int bias)
{
  unsigned Rd, Rn;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);

  inst.instruction |= Rd << 8;
  inst.instruction |= inst.operands[1].imm - bias;
  inst.instruction |= Rn << 16;

  if (inst.operands[3].present)
    {
      offsetT shift_amount = inst.relocs[0].exp.X_add_number;

      inst.relocs[0].type = BFD_RELOC_UNUSED;

      constraint (inst.relocs[0].exp.X_op != O_constant,
		  _("expression too complex"));

      if (shift_amount != 0)
	{
	  constraint (shift_amount > 31,
		      _("shift expression is too large"));

	  if (inst.operands[3].shift_kind == SHIFT_ASR)
	    inst.instruction |= 0x00200000;  /* sh bit.  */

	  inst.instruction |= (shift_amount & 0x1c) << 10;
	  inst.instruction |= (shift_amount & 0x03) << 6;
	}
    }
}

static void
do_t_ssat (void)
{
  do_t_ssat_usat (1);
}

static void
do_t_ssat16 (void)
{
  unsigned Rd, Rn;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);

  inst.instruction |= Rd << 8;
  inst.instruction |= inst.operands[1].imm - 1;
  inst.instruction |= Rn << 16;
}

static void
do_t_strex (void)
{
  constraint (!inst.operands[2].isreg || !inst.operands[2].preind
	      || inst.operands[2].postind || inst.operands[2].writeback
	      || inst.operands[2].immisreg || inst.operands[2].shifted
	      || inst.operands[2].negative,
	      BAD_ADDR_MODE);

  constraint (inst.operands[2].reg == REG_PC, BAD_PC);

  inst.instruction |= inst.operands[0].reg << 8;
  inst.instruction |= inst.operands[1].reg << 12;
  inst.instruction |= inst.operands[2].reg << 16;
  inst.relocs[0].type = BFD_RELOC_ARM_T32_OFFSET_U8;
}

static void
do_t_strexd (void)
{
  if (!inst.operands[2].present)
    inst.operands[2].reg = inst.operands[1].reg + 1;

  constraint (inst.operands[0].reg == inst.operands[1].reg
	      || inst.operands[0].reg == inst.operands[2].reg
	      || inst.operands[0].reg == inst.operands[3].reg,
	      BAD_OVERLAP);

  inst.instruction |= inst.operands[0].reg;
  inst.instruction |= inst.operands[1].reg << 12;
  inst.instruction |= inst.operands[2].reg << 8;
  inst.instruction |= inst.operands[3].reg << 16;
}

static void
do_t_sxtah (void)
{
  unsigned Rd, Rn, Rm;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[1].reg;
  Rm = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);
  reject_bad_reg (Rm);

  inst.instruction |= Rd << 8;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm;
  inst.instruction |= inst.operands[3].imm << 4;
}

static void
do_t_sxth (void)
{
  unsigned Rd, Rm;

  Rd = inst.operands[0].reg;
  Rm = inst.operands[1].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rm);

  if (inst.instruction <= 0xffff
      && inst.size_req != 4
      && Rd <= 7 && Rm <= 7
      && (!inst.operands[2].present || inst.operands[2].imm == 0))
    {
      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= Rd;
      inst.instruction |= Rm << 3;
    }
  else if (unified_syntax)
    {
      if (inst.instruction <= 0xffff)
	inst.instruction = THUMB_OP32 (inst.instruction);
      inst.instruction |= Rd << 8;
      inst.instruction |= Rm;
      inst.instruction |= inst.operands[2].imm << 4;
    }
  else
    {
      constraint (inst.operands[2].present && inst.operands[2].imm != 0,
		  _("Thumb encoding does not support rotation"));
      constraint (1, BAD_HIREG);
    }
}

static void
do_t_swi (void)
{
  inst.relocs[0].type = BFD_RELOC_ARM_SWI;
}

static void
do_t_tb (void)
{
  unsigned Rn, Rm;
  int half;

  half = (inst.instruction & 0x10) != 0;
  set_pred_insn_type_last ();
  constraint (inst.operands[0].immisreg,
	      _("instruction requires register index"));

  Rn = inst.operands[0].reg;
  Rm = inst.operands[0].imm;

  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))
    constraint (Rn == REG_SP, BAD_SP);
  reject_bad_reg (Rm);

  constraint (!half && inst.operands[0].shifted,
	      _("instruction does not allow shifted index"));
  inst.instruction |= (Rn << 16) | Rm;
}

static void
do_t_udf (void)
{
  if (!inst.operands[0].present)
    inst.operands[0].imm = 0;

  if ((unsigned int) inst.operands[0].imm > 255 || inst.size_req == 4)
    {
      constraint (inst.size_req == 2,
                  _("immediate value out of range"));
      inst.instruction = THUMB_OP32 (inst.instruction);
      inst.instruction |= (inst.operands[0].imm & 0xf000u) << 4;
      inst.instruction |= (inst.operands[0].imm & 0x0fffu) << 0;
    }
  else
    {
      inst.instruction = THUMB_OP16 (inst.instruction);
      inst.instruction |= inst.operands[0].imm;
    }

  set_pred_insn_type (NEUTRAL_IT_INSN);
}


static void
do_t_usat (void)
{
  do_t_ssat_usat (0);
}

static void
do_t_usat16 (void)
{
  unsigned Rd, Rn;

  Rd = inst.operands[0].reg;
  Rn = inst.operands[2].reg;

  reject_bad_reg (Rd);
  reject_bad_reg (Rn);

  inst.instruction |= Rd << 8;
  inst.instruction |= inst.operands[1].imm;
  inst.instruction |= Rn << 16;
}

/* Checking the range of the branch offset (VAL) with NBITS bits
   and IS_SIGNED signedness.  Also checks the LSB to be 0.  */
static int
v8_1_branch_value_check (int val, int nbits, int is_signed)
{
  gas_assert (nbits > 0 && nbits <= 32);
  if (is_signed)
    {
      int cmp = (1 << (nbits - 1));
      if ((val < -cmp) || (val >= cmp) || (val & 0x01))
	return FAIL;
    }
  else
    {
      if ((val <= 0) || (val >= (1 << nbits)) || (val & 0x1))
	return FAIL;
    }
    return SUCCESS;
}

/* For branches in Armv8.1-M Mainline.  */
static void
do_t_branch_future (void)
{
  unsigned long insn = inst.instruction;

  inst.instruction = THUMB_OP32 (inst.instruction);
  if (inst.operands[0].hasreloc == 0)
    {
      if (v8_1_branch_value_check (inst.operands[0].imm, 5, false) == FAIL)
	as_bad (BAD_BRANCH_OFF);

      inst.instruction |= ((inst.operands[0].imm & 0x1f) >> 1) << 23;
    }
  else
    {
      inst.relocs[0].type = BFD_RELOC_THUMB_PCREL_BRANCH5;
      inst.relocs[0].pc_rel = 1;
    }

  switch (insn)
    {
      case T_MNEM_bf:
	if (inst.operands[1].hasreloc == 0)
	  {
	    int val = inst.operands[1].imm;
	    if (v8_1_branch_value_check (inst.operands[1].imm, 17, true) == FAIL)
	      as_bad (BAD_BRANCH_OFF);

	    int immA = (val & 0x0001f000) >> 12;
	    int immB = (val & 0x00000ffc) >> 2;
	    int immC = (val & 0x00000002) >> 1;
	    inst.instruction |= (immA << 16) | (immB << 1) | (immC << 11);
	  }
	else
	  {
	    inst.relocs[1].type = BFD_RELOC_ARM_THUMB_BF17;
	    inst.relocs[1].pc_rel = 1;
	  }
	break;

      case T_MNEM_bfl:
	if (inst.operands[1].hasreloc == 0)
	  {
	    int val = inst.operands[1].imm;
	    if (v8_1_branch_value_check (inst.operands[1].imm, 19, true) == FAIL)
	      as_bad (BAD_BRANCH_OFF);

	    int immA = (val & 0x0007f000) >> 12;
	    int immB = (val & 0x00000ffc) >> 2;
	    int immC = (val & 0x00000002) >> 1;
	    inst.instruction |= (immA << 16) | (immB << 1) | (immC << 11);
	  }
	  else
	  {
	    inst.relocs[1].type = BFD_RELOC_ARM_THUMB_BF19;
	    inst.relocs[1].pc_rel = 1;
	  }
	break;

      case T_MNEM_bfcsel:
	/* Operand 1.  */
	if (inst.operands[1].hasreloc == 0)
	  {
	    int val = inst.operands[1].imm;
	    int immA = (val & 0x00001000) >> 12;
	    int immB = (val & 0x00000ffc) >> 2;
	    int immC = (val & 0x00000002) >> 1;
	    inst.instruction |= (immA << 16) | (immB << 1) | (immC << 11);
	  }
	  else
	  {
	    inst.relocs[1].type = BFD_RELOC_ARM_THUMB_BF13;
	    inst.relocs[1].pc_rel = 1;
	  }

	/* Operand 2.  */
	if (inst.operands[2].hasreloc == 0)
	  {
	      constraint ((inst.operands[0].hasreloc != 0), BAD_ARGS);
	      int val2 = inst.operands[2].imm;
	      int val0 = inst.operands[0].imm & 0x1f;
	      int diff = val2 - val0;
	      if (diff == 4)
		inst.instruction |= 1 << 17; /* T bit.  */
	      else if (diff != 2)
		as_bad (_("out of range label-relative fixup value"));
	  }
	else
	  {
	      constraint ((inst.operands[0].hasreloc == 0), BAD_ARGS);
	      inst.relocs[2].type = BFD_RELOC_THUMB_PCREL_BFCSEL;
	      inst.relocs[2].pc_rel = 1;
	  }

	/* Operand 3.  */
	constraint (inst.cond != COND_ALWAYS, BAD_COND);
	inst.instruction |= (inst.operands[3].imm & 0xf) << 18;
	break;

      case T_MNEM_bfx:
      case T_MNEM_bflx:
	inst.instruction |= inst.operands[1].reg << 16;
	break;

      default: abort ();
    }
}

/* Helper function for do_t_loloop to handle relocations.  */
static void
v8_1_loop_reloc (int is_le)
{
  if (inst.relocs[0].exp.X_op == O_constant)
    {
      int value = inst.relocs[0].exp.X_add_number;
      value = (is_le) ? -value : value;

      if (v8_1_branch_value_check (value, 12, false) == FAIL)
	as_bad (BAD_BRANCH_OFF);

      int imml, immh;

      immh = (value & 0x00000ffc) >> 2;
      imml = (value & 0x00000002) >> 1;

      inst.instruction |= (imml << 11) | (immh << 1);
    }
  else
    {
      inst.relocs[0].type = BFD_RELOC_ARM_THUMB_LOOP12;
      inst.relocs[0].pc_rel = 1;
    }
}

/* For shifts with four operands in MVE.  */
static void
do_mve_scalar_shift1 (void)
{
  unsigned int value = inst.operands[2].imm;

  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg << 8;

  /* Setting the bit for saturation.  */
  inst.instruction |= ((value == 64) ? 0: 1) << 7;

  /* Assuming Rm is already checked not to be 11x1.  */
  constraint (inst.operands[3].reg == inst.operands[0].reg, BAD_OVERLAP);
  constraint (inst.operands[3].reg == inst.operands[1].reg, BAD_OVERLAP);
  inst.instruction |= inst.operands[3].reg << 12;
}

/* For shifts in MVE.  */
static void
do_mve_scalar_shift (void)
{
  if (!inst.operands[2].present)
    {
      inst.operands[2] = inst.operands[1];
      inst.operands[1].reg = 0xf;
    }

  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[1].reg << 8;

  if (inst.operands[2].isreg)
    {
      /* Assuming Rm is already checked not to be 11x1.  */
      constraint (inst.operands[2].reg == inst.operands[0].reg, BAD_OVERLAP);
      constraint (inst.operands[2].reg == inst.operands[1].reg, BAD_OVERLAP);
      inst.instruction |= inst.operands[2].reg << 12;
    }
  else
    {
      /* Assuming imm is already checked as [1,32].  */
      unsigned int value = inst.operands[2].imm;
      inst.instruction |= (value & 0x1c) << 10;
      inst.instruction |= (value & 0x03) << 6;
      /* Change last 4 bits from 0xd to 0xf.  */
      inst.instruction |= 0x2;
    }
}

/* MVE instruction encoder helpers.  */
#define M_MNEM_vabav	0xee800f01
#define M_MNEM_vmladav	  0xeef00e00
#define M_MNEM_vmladava	  0xeef00e20
#define M_MNEM_vmladavx	  0xeef01e00
#define M_MNEM_vmladavax  0xeef01e20
#define M_MNEM_vmlsdav	  0xeef00e01
#define M_MNEM_vmlsdava	  0xeef00e21
#define M_MNEM_vmlsdavx	  0xeef01e01
#define M_MNEM_vmlsdavax  0xeef01e21
#define M_MNEM_vmullt	0xee011e00
#define M_MNEM_vmullb	0xee010e00
#define M_MNEM_vctp	0xf000e801
#define M_MNEM_vst20	0xfc801e00
#define M_MNEM_vst21	0xfc801e20
#define M_MNEM_vst40	0xfc801e01
#define M_MNEM_vst41	0xfc801e21
#define M_MNEM_vst42	0xfc801e41
#define M_MNEM_vst43	0xfc801e61
#define M_MNEM_vld20	0xfc901e00
#define M_MNEM_vld21	0xfc901e20
#define M_MNEM_vld40	0xfc901e01
#define M_MNEM_vld41	0xfc901e21
#define M_MNEM_vld42	0xfc901e41
#define M_MNEM_vld43	0xfc901e61
#define M_MNEM_vstrb	0xec000e00
#define M_MNEM_vstrh	0xec000e10
#define M_MNEM_vstrw	0xec000e40
#define M_MNEM_vstrd	0xec000e50
#define M_MNEM_vldrb	0xec100e00
#define M_MNEM_vldrh	0xec100e10
#define M_MNEM_vldrw	0xec100e40
#define M_MNEM_vldrd	0xec100e50
#define M_MNEM_vmovlt	0xeea01f40
#define M_MNEM_vmovlb	0xeea00f40
#define M_MNEM_vmovnt	0xfe311e81
#define M_MNEM_vmovnb	0xfe310e81
#define M_MNEM_vadc	0xee300f00
#define M_MNEM_vadci	0xee301f00
#define M_MNEM_vbrsr	0xfe011e60
#define M_MNEM_vaddlv	0xee890f00
#define M_MNEM_vaddlva	0xee890f20
#define M_MNEM_vaddv	0xeef10f00
#define M_MNEM_vaddva	0xeef10f20
#define M_MNEM_vddup	0xee011f6e
#define M_MNEM_vdwdup	0xee011f60
#define M_MNEM_vidup	0xee010f6e
#define M_MNEM_viwdup	0xee010f60
#define M_MNEM_vmaxv	0xeee20f00
#define M_MNEM_vmaxav	0xeee00f00
#define M_MNEM_vminv	0xeee20f80
#define M_MNEM_vminav	0xeee00f80
#define M_MNEM_vmlaldav	  0xee800e00
#define M_MNEM_vmlaldava  0xee800e20
#define M_MNEM_vmlaldavx  0xee801e00
#define M_MNEM_vmlaldavax 0xee801e20
#define M_MNEM_vmlsldav	  0xee800e01
#define M_MNEM_vmlsldava  0xee800e21
#define M_MNEM_vmlsldavx  0xee801e01
#define M_MNEM_vmlsldavax 0xee801e21
#define M_MNEM_vrmlaldavhx  0xee801f00
#define M_MNEM_vrmlaldavhax 0xee801f20
#define M_MNEM_vrmlsldavh   0xfe800e01
#define M_MNEM_vrmlsldavha  0xfe800e21
#define M_MNEM_vrmlsldavhx  0xfe801e01
#define M_MNEM_vrmlsldavhax 0xfe801e21
#define M_MNEM_vqmovnt	  0xee331e01
#define M_MNEM_vqmovnb	  0xee330e01
#define M_MNEM_vqmovunt	  0xee311e81
#define M_MNEM_vqmovunb	  0xee310e81
#define M_MNEM_vshrnt	    0xee801fc1
#define M_MNEM_vshrnb	    0xee800fc1
#define M_MNEM_vrshrnt	    0xfe801fc1
#define M_MNEM_vqshrnt	    0xee801f40
#define M_MNEM_vqshrnb	    0xee800f40
#define M_MNEM_vqshrunt	    0xee801fc0
#define M_MNEM_vqshrunb	    0xee800fc0
#define M_MNEM_vrshrnb	    0xfe800fc1
#define M_MNEM_vqrshrnt	    0xee801f41
#define M_MNEM_vqrshrnb	    0xee800f41
#define M_MNEM_vqrshrunt    0xfe801fc0
#define M_MNEM_vqrshrunb    0xfe800fc0

/* Bfloat16 instruction encoder helpers.  */
#define B_MNEM_vfmat 0xfc300850
#define B_MNEM_vfmab 0xfc300810

/* Neon instruction encoder helpers.  */

/* Encodings for the different types for various Neon opcodes.  */

/* An "invalid" code for the following tables.  */
#define N_INV -1u

struct neon_tab_entry
{
  unsigned integer;
  unsigned float_or_poly;
  unsigned scalar_or_imm;
};

/* Map overloaded Neon opcodes to their respective encodings.  */
#define NEON_ENC_TAB					\
  X(vabd,	0x0000700, 0x1200d00, N_INV),		\
  X(vabdl,	0x0800700, N_INV,     N_INV),		\
  X(vmax,	0x0000600, 0x0000f00, N_INV),		\
  X(vmin,	0x0000610, 0x0200f00, N_INV),		\
  X(vpadd,	0x0000b10, 0x1000d00, N_INV),		\
  X(vpmax,	0x0000a00, 0x1000f00, N_INV),		\
  X(vpmin,	0x0000a10, 0x1200f00, N_INV),		\
  X(vadd,	0x0000800, 0x0000d00, N_INV),		\
  X(vaddl,	0x0800000, N_INV,     N_INV),		\
  X(vsub,	0x1000800, 0x0200d00, N_INV),		\
  X(vsubl,	0x0800200, N_INV,     N_INV),		\
  X(vceq,	0x1000810, 0x0000e00, 0x1b10100),	\
  X(vcge,	0x0000310, 0x1000e00, 0x1b10080),	\
  X(vcgt,	0x0000300, 0x1200e00, 0x1b10000),	\
  /* Register variants of the following two instructions are encoded as
     vcge / vcgt with the operands reversed.  */  	\
  X(vclt,	0x0000300, 0x1200e00, 0x1b10200),	\
  X(vcle,	0x0000310, 0x1000e00, 0x1b10180),	\
  X(vfma,	N_INV, 0x0000c10, N_INV),		\
  X(vfms,	N_INV, 0x0200c10, N_INV),		\
  X(vmla,	0x0000900, 0x0000d10, 0x0800040),	\
  X(vmls,	0x1000900, 0x0200d10, 0x0800440),	\
  X(vmul,	0x0000910, 0x1000d10, 0x0800840),	\
  X(vmull,	0x0800c00, 0x0800e00, 0x0800a40), /* polynomial not float.  */ \
  X(vmlal,	0x0800800, N_INV,     0x0800240),	\
  X(vmlsl,	0x0800a00, N_INV,     0x0800640),	\
  X(vqdmlal,	0x0800900, N_INV,     0x0800340),	\
  X(vqdmlsl,	0x0800b00, N_INV,     0x0800740),	\
  X(vqdmull,	0x0800d00, N_INV,     0x0800b40),	\
  X(vqdmulh,    0x0000b00, N_INV,     0x0800c40),	\
  X(vqrdmulh,   0x1000b00, N_INV,     0x0800d40),	\
  X(vqrdmlah,   0x3000b10, N_INV,     0x0800e40),	\
  X(vqrdmlsh,   0x3000c10, N_INV,     0x0800f40),	\
  X(vshl,	0x0000400, N_INV,     0x0800510),	\
  X(vqshl,	0x0000410, N_INV,     0x0800710),	\
  X(vand,	0x0000110, N_INV,     0x0800030),	\
  X(vbic,	0x0100110, N_INV,     0x0800030),	\
  X(veor,	0x1000110, N_INV,     N_INV),		\
  X(vorn,	0x0300110, N_INV,     0x0800010),	\
  X(vorr,	0x0200110, N_INV,     0x0800010),	\
  X(vmvn,	0x1b00580, N_INV,     0x0800030),	\
  X(vshll,	0x1b20300, N_INV,     0x0800a10), /* max shift, immediate.  */ \
  X(vcvt,       0x1b30600, N_INV,     0x0800e10), /* integer, fixed-point.  */ \
  X(vdup,       0xe800b10, N_INV,     0x1b00c00), /* arm, scalar.  */ \
  X(vld1,       0x0200000, 0x0a00000, 0x0a00c00), /* interlv, lane, dup.  */ \
  X(vst1,	0x0000000, 0x0800000, N_INV),		\
  X(vld2,	0x0200100, 0x0a00100, 0x0a00d00),	\
  X(vst2,	0x0000100, 0x0800100, N_INV),		\
  X(vld3,	0x0200200, 0x0a00200, 0x0a00e00),	\
  X(vst3,	0x0000200, 0x0800200, N_INV),		\
  X(vld4,	0x0200300, 0x0a00300, 0x0a00f00),	\
  X(vst4,	0x0000300, 0x0800300, N_INV),		\
  X(vmovn,	0x1b20200, N_INV,     N_INV),		\
  X(vtrn,	0x1b20080, N_INV,     N_INV),		\
  X(vqmovn,	0x1b20200, N_INV,     N_INV),		\
  X(vqmovun,	0x1b20240, N_INV,     N_INV),		\
  X(vnmul,      0xe200a40, 0xe200b40, N_INV),		\
  X(vnmla,      0xe100a40, 0xe100b40, N_INV),		\
  X(vnmls,      0xe100a00, 0xe100b00, N_INV),		\
  X(vfnma,      0xe900a40, 0xe900b40, N_INV),		\
  X(vfnms,      0xe900a00, 0xe900b00, N_INV),		\
  X(vcmp,	0xeb40a40, 0xeb40b40, N_INV),		\
  X(vcmpz,	0xeb50a40, 0xeb50b40, N_INV),		\
  X(vcmpe,	0xeb40ac0, 0xeb40bc0, N_INV),		\
  X(vcmpez,     0xeb50ac0, 0xeb50bc0, N_INV),		\
  X(vseleq,	0xe000a00, N_INV,     N_INV),		\
  X(vselvs,	0xe100a00, N_INV,     N_INV),		\
  X(vselge,	0xe200a00, N_INV,     N_INV),		\
  X(vselgt,	0xe300a00, N_INV,     N_INV),		\
  X(vmaxnm,	0xe800a00, 0x3000f10, N_INV),		\
  X(vminnm,	0xe800a40, 0x3200f10, N_INV),		\
  X(vcvta,	0xebc0a40, 0x3bb0000, N_INV),		\
  X(vrintr,	0xeb60a40, 0x3ba0400, N_INV),		\
  X(vrinta,	0xeb80a40, 0x3ba0400, N_INV),		\
  X(aes,	0x3b00300, N_INV,     N_INV),		\
  X(sha3op,	0x2000c00, N_INV,     N_INV),		\
  X(sha1h,	0x3b902c0, N_INV,     N_INV),           \
  X(sha2op,     0x3ba0380, N_INV,     N_INV)

enum neon_opc
{
#define X(OPC,I,F,S) N_MNEM_##OPC
NEON_ENC_TAB
#undef X
};

static const struct neon_tab_entry neon_enc_tab[] =
{
#define X(OPC,I,F,S) { (I), (F), (S) }
NEON_ENC_TAB
#undef X
};

/* Do not use these macros; instead, use NEON_ENCODE defined below.  */
#define NEON_ENC_INTEGER_(X) (neon_enc_tab[(X) & 0x0fffffff].integer)
#define NEON_ENC_ARMREG_(X)  (neon_enc_tab[(X) & 0x0fffffff].integer)
#define NEON_ENC_POLY_(X)    (neon_enc_tab[(X) & 0x0fffffff].float_or_poly)
#define NEON_ENC_FLOAT_(X)   (neon_enc_tab[(X) & 0x0fffffff].float_or_poly)
#define NEON_ENC_SCALAR_(X)  (neon_enc_tab[(X) & 0x0fffffff].scalar_or_imm)
#define NEON_ENC_IMMED_(X)   (neon_enc_tab[(X) & 0x0fffffff].scalar_or_imm)
#define NEON_ENC_INTERLV_(X) (neon_enc_tab[(X) & 0x0fffffff].integer)
#define NEON_ENC_LANE_(X)    (neon_enc_tab[(X) & 0x0fffffff].float_or_poly)
#define NEON_ENC_DUP_(X)     (neon_enc_tab[(X) & 0x0fffffff].scalar_or_imm)
#define NEON_ENC_SINGLE_(X) \
  ((neon_enc_tab[(X) & 0x0fffffff].integer) | ((X) & 0xf0000000))
#define NEON_ENC_DOUBLE_(X) \
  ((neon_enc_tab[(X) & 0x0fffffff].float_or_poly) | ((X) & 0xf0000000))
#define NEON_ENC_FPV8_(X) \
  ((neon_enc_tab[(X) & 0x0fffffff].integer) | ((X) & 0xf000000))

#define NEON_ENCODE(type, inst)					\
  do								\
    {								\
      inst.instruction = NEON_ENC_##type##_ (inst.instruction);	\
      inst.is_neon = 1;						\
    }								\
  while (0)

#define check_neon_suffixes						\
  do									\
    {									\
      if (!inst.error && inst.vectype.elems > 0 && !inst.is_neon)	\
	{								\
	  as_bad (_("invalid neon suffix for non neon instruction"));	\
	  return;							\
	}								\
    }									\
  while (0)

/* Define shapes for instruction operands. The following mnemonic characters
   are used in this table:

     F - VFP S<n> register
     D - Neon D<n> register
     Q - Neon Q<n> register
     I - Immediate
     S - Scalar
     R - ARM register
     L - D<n> register list

   This table is used to generate various data:
     - enumerations of the form NS_DDR to be used as arguments to
       neon_select_shape.
     - a table classifying shapes into single, double, quad, mixed.
     - a table used to drive neon_select_shape.  */

#define NEON_SHAPE_DEF			\
  X(4, (R, R, Q, Q), QUAD),		\
  X(4, (Q, R, R, I), QUAD),		\
  X(4, (R, R, S, S), QUAD),		\
  X(4, (S, S, R, R), QUAD),		\
  X(3, (Q, R, I), QUAD),		\
  X(3, (I, Q, Q), QUAD),		\
  X(3, (I, Q, R), QUAD),		\
  X(3, (R, Q, Q), QUAD),		\
  X(3, (D, D, D), DOUBLE),		\
  X(3, (Q, Q, Q), QUAD),		\
  X(3, (D, D, I), DOUBLE),		\
  X(3, (Q, Q, I), QUAD),		\
  X(3, (D, D, S), DOUBLE),		\
  X(3, (Q, Q, S), QUAD),		\
  X(3, (Q, Q, R), QUAD),		\
  X(3, (R, R, Q), QUAD),		\
  X(2, (R, Q),	  QUAD),		\
  X(2, (D, D), DOUBLE),			\
  X(2, (Q, Q), QUAD),			\
  X(2, (D, S), DOUBLE),			\
  X(2, (Q, S), QUAD),			\
  X(2, (D, R), DOUBLE),			\
  X(2, (Q, R), QUAD),			\
  X(2, (D, I), DOUBLE),			\
  X(2, (Q, I), QUAD),			\
  X(3, (P, F, I), SINGLE),		\
  X(3, (P, D, I), DOUBLE),		\
  X(3, (P, Q, I), QUAD),		\
  X(4, (P, F, F, I), SINGLE),		\
  X(4, (P, D, D, I), DOUBLE),		\
  X(4, (P, Q, Q, I), QUAD),		\
  X(5, (P, F, F, F, I), SINGLE),	\
  X(5, (P, D, D, D, I), DOUBLE),	\
  X(5, (P, Q, Q, Q, I), QUAD),		\
  X(3, (D, L, D), DOUBLE),		\
  X(2, (D, Q), MIXED),			\
  X(2, (Q, D), MIXED),			\
  X(3, (D, Q, I), MIXED),		\
  X(3, (Q, D, I), MIXED),		\
  X(3, (Q, D, D), MIXED),		\
  X(3, (D, Q, Q), MIXED),		\
  X(3, (Q, Q, D), MIXED),		\
  X(3, (Q, D, S), MIXED),		\
  X(3, (D, Q, S), MIXED),		\
  X(4, (D, D, D, I), DOUBLE),		\
  X(4, (Q, Q, Q, I), QUAD),		\
  X(4, (D, D, S, I), DOUBLE),		\
  X(4, (Q, Q, S, I), QUAD),		\
  X(2, (F, F), SINGLE),			\
  X(3, (F, F, F), SINGLE),		\
  X(2, (F, I), SINGLE),			\
  X(2, (F, D), MIXED),			\
  X(2, (D, F), MIXED),			\
  X(3, (F, F, I), MIXED),		\
  X(4, (R, R, F, F), SINGLE),		\
  X(4, (F, F, R, R), SINGLE),		\
  X(3, (D, R, R), DOUBLE),		\
  X(3, (R, R, D), DOUBLE),		\
  X(2, (S, R), SINGLE),			\
  X(2, (R, S), SINGLE),			\
  X(2, (F, R), SINGLE),			\
  X(2, (R, F), SINGLE),			\
/* Used for MVE tail predicated loop instructions.  */\
  X(2, (R, R), QUAD),			\
/* Half float shape supported so far.  */\
  X (2, (H, D), MIXED),			\
  X (2, (D, H), MIXED),			\
  X (2, (H, F), MIXED),			\
  X (2, (F, H), MIXED),			\
  X (2, (H, H), HALF),			\
  X (2, (H, R), HALF),			\
  X (2, (R, H), HALF),			\
  X (2, (H, I), HALF),			\
  X (3, (H, H, H), HALF),		\
  X (3, (H, F, I), MIXED),		\
  X (3, (F, H, I), MIXED),		\
  X (3, (D, H, H), MIXED),		\
  X (3, (D, H, S), MIXED)

#define S2(A,B)		NS_##A##B
#define S3(A,B,C)	NS_##A##B##C
#define S4(A,B,C,D)	NS_##A##B##C##D
#define S5(A,B,C,D,E)	NS_##A##B##C##D##E

#define X(N, L, C) S##N L

enum neon_shape
{
  NEON_SHAPE_DEF,
  NS_NULL
};

#undef X
#undef S2
#undef S3
#undef S4
#undef S5

enum neon_shape_class
{
  SC_HALF,
  SC_SINGLE,
  SC_DOUBLE,
  SC_QUAD,
  SC_MIXED
};

#define X(N, L, C) SC_##C

static enum neon_shape_class neon_shape_class[] =
{
  NEON_SHAPE_DEF
};

#undef X

enum neon_shape_el
{
  SE_H,
  SE_F,
  SE_D,
  SE_Q,
  SE_I,
  SE_S,
  SE_R,
  SE_L,
  SE_P
};

/* Register widths of above.  */
static unsigned neon_shape_el_size[] =
{
  16,
  32,
  64,
  128,
  0,
  32,
  32,
  0,
  0
};

struct neon_shape_info
{
  unsigned els;
  enum neon_shape_el el[NEON_MAX_TYPE_ELS];
};

#define S2(A,B)		{ SE_##A, SE_##B }
#define S3(A,B,C)	{ SE_##A, SE_##B, SE_##C }
#define S4(A,B,C,D)	{ SE_##A, SE_##B, SE_##C, SE_##D }
#define S5(A,B,C,D,E)	{ SE_##A, SE_##B, SE_##C, SE_##D, SE_##E }

#define X(N, L, C) { N, S##N L }

static struct neon_shape_info neon_shape_tab[] =
{
  NEON_SHAPE_DEF
};

#undef X
#undef S2
#undef S3
#undef S4
#undef S5

/* Bit masks used in type checking given instructions.
  'N_EQK' means the type must be the same as (or based on in some way) the key
   type, which itself is marked with the 'N_KEY' bit. If the 'N_EQK' bit is
   set, various other bits can be set as well in order to modify the meaning of
   the type constraint.  */

enum neon_type_mask
{
  N_S8   = 0x0000001,
  N_S16  = 0x0000002,
  N_S32  = 0x0000004,
  N_S64  = 0x0000008,
  N_U8   = 0x0000010,
  N_U16  = 0x0000020,
  N_U32  = 0x0000040,
  N_U64  = 0x0000080,
  N_I8   = 0x0000100,
  N_I16  = 0x0000200,
  N_I32  = 0x0000400,
  N_I64  = 0x0000800,
  N_8    = 0x0001000,
  N_16   = 0x0002000,
  N_32   = 0x0004000,
  N_64   = 0x0008000,
  N_P8   = 0x0010000,
  N_P16  = 0x0020000,
  N_F16  = 0x0040000,
  N_F32  = 0x0080000,
  N_F64  = 0x0100000,
  N_P64	 = 0x0200000,
  N_BF16 = 0x0400000,
  N_KEY  = 0x1000000, /* Key element (main type specifier).  */
  N_EQK  = 0x2000000, /* Given operand has the same type & size as the key.  */
  N_VFP  = 0x4000000, /* VFP mode: operand size must match register width.  */
  N_UNT  = 0x8000000, /* Must be explicitly untyped.  */
  N_DBL  = 0x0000001, /* If N_EQK, this operand is twice the size.  */
  N_HLF  = 0x0000002, /* If N_EQK, this operand is half the size.  */
  N_SGN  = 0x0000004, /* If N_EQK, this operand is forced to be signed.  */
  N_UNS  = 0x0000008, /* If N_EQK, this operand is forced to be unsigned.  */
  N_INT  = 0x0000010, /* If N_EQK, this operand is forced to be integer.  */
  N_FLT  = 0x0000020, /* If N_EQK, this operand is forced to be float.  */
  N_SIZ  = 0x0000040, /* If N_EQK, this operand is forced to be size-only.  */
  N_UTYP = 0,
  N_MAX_NONSPECIAL = N_P64
};

#define N_ALLMODS  (N_DBL | N_HLF | N_SGN | N_UNS | N_INT | N_FLT | N_SIZ)

#define N_SU_ALL   (N_S8 | N_S16 | N_S32 | N_S64 | N_U8 | N_U16 | N_U32 | N_U64)
#define N_SU_32    (N_S8 | N_S16 | N_S32 | N_U8 | N_U16 | N_U32)
#define N_SU_16_64 (N_S16 | N_S32 | N_S64 | N_U16 | N_U32 | N_U64)
#define N_S_32     (N_S8 | N_S16 | N_S32)
#define N_F_16_32  (N_F16 | N_F32)
#define N_SUF_32   (N_SU_32 | N_F_16_32)
#define N_I_ALL    (N_I8 | N_I16 | N_I32 | N_I64)
#define N_IF_32    (N_I8 | N_I16 | N_I32 | N_F16 | N_F32)
#define N_F_ALL    (N_F16 | N_F32 | N_F64)
#define N_I_MVE	   (N_I8 | N_I16 | N_I32)
#define N_F_MVE	   (N_F16 | N_F32)
#define N_SU_MVE   (N_S8 | N_S16 | N_S32 | N_U8 | N_U16 | N_U32)

/* Pass this as the first type argument to neon_check_type to ignore types
   altogether.  */
#define N_IGNORE_TYPE (N_KEY | N_EQK)

/* Select a "shape" for the current instruction (describing register types or
   sizes) from a list of alternatives. Return NS_NULL if the current instruction
   doesn't fit. For non-polymorphic shapes, checking is usually done as a
   function of operand parsing, so this function doesn't need to be called.
   Shapes should be listed in order of decreasing length.  */

static enum neon_shape
neon_select_shape (enum neon_shape shape, ...)
{
  va_list ap;
  enum neon_shape first_shape = shape;

  /* Fix missing optional operands. FIXME: we don't know at this point how
     many arguments we should have, so this makes the assumption that we have
     > 1. This is true of all current Neon opcodes, I think, but may not be
     true in the future.  */
  if (!inst.operands[1].present)
    inst.operands[1] = inst.operands[0];

  va_start (ap, shape);

  for (; shape != NS_NULL; shape = (enum neon_shape) va_arg (ap, int))
    {
      unsigned j;
      int matches = 1;

      for (j = 0; j < neon_shape_tab[shape].els; j++)
	{
	  if (!inst.operands[j].present)
	    {
	      matches = 0;
	      break;
	    }

	  switch (neon_shape_tab[shape].el[j])
	    {
	      /* If a  .f16,  .16,  .u16,  .s16 type specifier is given over
		 a VFP single precision register operand, it's essentially
		 means only half of the register is used.

		 If the type specifier is given after the mnemonics, the
		 information is stored in inst.vectype.  If the type specifier
		 is given after register operand, the information is stored
		 in inst.operands[].vectype.

		 When there is only one type specifier, and all the register
		 operands are the same type of hardware register, the type
		 specifier applies to all register operands.

		 If no type specifier is given, the shape is inferred from
		 operand information.

		 for example:
		 vadd.f16 s0, s1, s2:		NS_HHH
		 vabs.f16 s0, s1:		NS_HH
		 vmov.f16 s0, r1:		NS_HR
		 vmov.f16 r0, s1:		NS_RH
		 vcvt.f16 r0, s1:		NS_RH
		 vcvt.f16.s32	s2, s2, #29:	NS_HFI
		 vcvt.f16.s32	s2, s2:		NS_HF
	      */
	    case SE_H:
	      if (!(inst.operands[j].isreg
		    && inst.operands[j].isvec
		    && inst.operands[j].issingle
		    && !inst.operands[j].isquad
		    && ((inst.vectype.elems == 1
			 && inst.vectype.el[0].size == 16)
			|| (inst.vectype.elems > 1
			    && inst.vectype.el[j].size == 16)
			|| (inst.vectype.elems == 0
			    && inst.operands[j].vectype.type != NT_invtype
			    && inst.operands[j].vectype.size == 16))))
		matches = 0;
	      break;

	    case SE_F:
	      if (!(inst.operands[j].isreg
		    && inst.operands[j].isvec
		    && inst.operands[j].issingle
		    && !inst.operands[j].isquad
		    && ((inst.vectype.elems == 1 && inst.vectype.el[0].size == 32)
			|| (inst.vectype.elems > 1 && inst.vectype.el[j].size == 32)
			|| (inst.vectype.elems == 0
			    && (inst.operands[j].vectype.size == 32
				|| inst.operands[j].vectype.type == NT_invtype)))))
		matches = 0;
	      break;

	    case SE_D:
	      if (!(inst.operands[j].isreg
		    && inst.operands[j].isvec
		    && !inst.operands[j].isquad
		    && !inst.operands[j].issingle))
		matches = 0;
	      break;

	    case SE_R:
	      if (!(inst.operands[j].isreg
		    && !inst.operands[j].isvec))
		matches = 0;
	      break;

	    case SE_Q:
	      if (!(inst.operands[j].isreg
		    && inst.operands[j].isvec
		    && inst.operands[j].isquad
		    && !inst.operands[j].issingle))
		matches = 0;
	      break;

	    case SE_I:
	      if (!(!inst.operands[j].isreg
		    && !inst.operands[j].isscalar))
		matches = 0;
	      break;

	    case SE_S:
	      if (!(!inst.operands[j].isreg
		    && inst.operands[j].isscalar))
		matches = 0;
	      break;

	    case SE_P:
	    case SE_L:
	      break;
	    }
	  if (!matches)
	    break;
	}
      if (matches && (j >= ARM_IT_MAX_OPERANDS || !inst.operands[j].present))
	/* We've matched all the entries in the shape table, and we don't
	   have any left over operands which have not been matched.  */
	break;
    }

  va_end (ap);

  if (shape == NS_NULL && first_shape != NS_NULL)
    first_error (_("invalid instruction shape"));

  return shape;
}

/* True if SHAPE is predominantly a quadword operation (most of the time, this
   means the Q bit should be set).  */

static int
neon_quad (enum neon_shape shape)
{
  return neon_shape_class[shape] == SC_QUAD;
}

static void
neon_modify_type_size (unsigned typebits, enum neon_el_type *g_type,
		       unsigned *g_size)
{
  /* Allow modification to be made to types which are constrained to be
     based on the key element, based on bits set alongside N_EQK.  */
  if ((typebits & N_EQK) != 0)
    {
      if ((typebits & N_HLF) != 0)
	*g_size /= 2;
      else if ((typebits & N_DBL) != 0)
	*g_size *= 2;
      if ((typebits & N_SGN) != 0)
	*g_type = NT_signed;
      else if ((typebits & N_UNS) != 0)
	*g_type = NT_unsigned;
      else if ((typebits & N_INT) != 0)
	*g_type = NT_integer;
      else if ((typebits & N_FLT) != 0)
	*g_type = NT_float;
      else if ((typebits & N_SIZ) != 0)
	*g_type = NT_untyped;
    }
}

/* Return operand OPNO promoted by bits set in THISARG. KEY should be the "key"
   operand type, i.e. the single type specified in a Neon instruction when it
   is the only one given.  */

static struct neon_type_el
neon_type_promote (struct neon_type_el *key, unsigned thisarg)
{
  struct neon_type_el dest = *key;

  gas_assert ((thisarg & N_EQK) != 0);

  neon_modify_type_size (thisarg, &dest.type, &dest.size);

  return dest;
}

/* Convert Neon type and size into compact bitmask representation.  */

static enum neon_type_mask
type_chk_of_el_type (enum neon_el_type type, unsigned size)
{
  switch (type)
    {
    case NT_untyped:
      switch (size)
	{
	case 8:  return N_8;
	case 16: return N_16;
	case 32: return N_32;
	case 64: return N_64;
	default: ;
	}
      break;

    case NT_integer:
      switch (size)
	{
	case 8:  return N_I8;
	case 16: return N_I16;
	case 32: return N_I32;
	case 64: return N_I64;
	default: ;
	}
      break;

    case NT_float:
      switch (size)
	{
	case 16: return N_F16;
	case 32: return N_F32;
	case 64: return N_F64;
	default: ;
	}
      break;

    case NT_poly:
      switch (size)
	{
	case 8:  return N_P8;
	case 16: return N_P16;
	case 64: return N_P64;
	default: ;
	}
      break;

    case NT_signed:
      switch (size)
	{
	case 8:  return N_S8;
	case 16: return N_S16;
	case 32: return N_S32;
	case 64: return N_S64;
	default: ;
	}
      break;

    case NT_unsigned:
      switch (size)
	{
	case 8:  return N_U8;
	case 16: return N_U16;
	case 32: return N_U32;
	case 64: return N_U64;
	default: ;
	}
      break;

    case NT_bfloat:
      if (size == 16) return N_BF16;
      break;

    default: ;
    }

  return N_UTYP;
}

/* Convert compact Neon bitmask type representation to a type and size. Only
   handles the case where a single bit is set in the mask.  */

static int
el_type_of_type_chk (enum neon_el_type *type, unsigned *size,
		     enum neon_type_mask mask)
{
  if ((mask & N_EQK) != 0)
    return FAIL;

  if ((mask & (N_S8 | N_U8 | N_I8 | N_8 | N_P8)) != 0)
    *size = 8;
  else if ((mask & (N_S16 | N_U16 | N_I16 | N_16 | N_F16 | N_P16 | N_BF16))
	   != 0)
    *size = 16;
  else if ((mask & (N_S32 | N_U32 | N_I32 | N_32 | N_F32)) != 0)
    *size = 32;
  else if ((mask & (N_S64 | N_U64 | N_I64 | N_64 | N_F64 | N_P64)) != 0)
    *size = 64;
  else
    return FAIL;

  if ((mask & (N_S8 | N_S16 | N_S32 | N_S64)) != 0)
    *type = NT_signed;
  else if ((mask & (N_U8 | N_U16 | N_U32 | N_U64)) != 0)
    *type = NT_unsigned;
  else if ((mask & (N_I8 | N_I16 | N_I32 | N_I64)) != 0)
    *type = NT_integer;
  else if ((mask & (N_8 | N_16 | N_32 | N_64)) != 0)
    *type = NT_untyped;
  else if ((mask & (N_P8 | N_P16 | N_P64)) != 0)
    *type = NT_poly;
  else if ((mask & (N_F_ALL)) != 0)
    *type = NT_float;
  else if ((mask & (N_BF16)) != 0)
    *type = NT_bfloat;
  else
    return FAIL;

  return SUCCESS;
}

/* Modify a bitmask of allowed types. This is only needed for type
   relaxation.  */

static unsigned
modify_types_allowed (unsigned allowed, unsigned mods)
{
  unsigned size;
  enum neon_el_type type;
  unsigned destmask;
  int i;

  destmask = 0;

  for (i = 1; i <= N_MAX_NONSPECIAL; i <<= 1)
    {
      if (el_type_of_type_chk (&type, &size,
			       (enum neon_type_mask) (allowed & i)) == SUCCESS)
	{
	  neon_modify_type_size (mods, &type, &size);
	  destmask |= type_chk_of_el_type (type, size);
	}
    }

  return destmask;
}

/* Check type and return type classification.
   The manual states (paraphrase): If one datatype is given, it indicates the
   type given in:
    - the second operand, if there is one
    - the operand, if there is no second operand
    - the result, if there are no operands.
   This isn't quite good enough though, so we use a concept of a "key" datatype
   which is set on a per-instruction basis, which is the one which matters when
   only one data type is written.
   Note: this function has side-effects (e.g. filling in missing operands). All
   Neon instructions should call it before performing bit encoding.  */

static struct neon_type_el
neon_check_type (unsigned els, enum neon_shape ns, ...)
{
  va_list ap;
  unsigned i, pass, key_el = 0;
  unsigned types[NEON_MAX_TYPE_ELS];
  enum neon_el_type k_type = NT_invtype;
  unsigned k_size = -1u;
  struct neon_type_el badtype = {NT_invtype, -1};
  unsigned key_allowed = 0;

  /* Optional registers in Neon instructions are always (not) in operand 1.
     Fill in the missing operand here, if it was omitted.  */
  if (els > 1 && !inst.operands[1].present)
    inst.operands[1] = inst.operands[0];

  /* Suck up all the varargs.  */
  va_start (ap, ns);
  for (i = 0; i < els; i++)
    {
      unsigned thisarg = va_arg (ap, unsigned);
      if (thisarg == N_IGNORE_TYPE)
	{
	  va_end (ap);
	  return badtype;
	}
      types[i] = thisarg;
      if ((thisarg & N_KEY) != 0)
	key_el = i;
    }
  va_end (ap);

  if (inst.vectype.elems > 0)
    for (i = 0; i < els; i++)
      if (inst.operands[i].vectype.type != NT_invtype)
	{
	  first_error (_("types specified in both the mnemonic and operands"));
	  return badtype;
	}

  /* Duplicate inst.vectype elements here as necessary.
     FIXME: No idea if this is exactly the same as the ARM assembler,
     particularly when an insn takes one register and one non-register
     operand. */
  if (inst.vectype.elems == 1 && els > 1)
    {
      unsigned j;
      inst.vectype.elems = els;
      inst.vectype.el[key_el] = inst.vectype.el[0];
      for (j = 0; j < els; j++)
	if (j != key_el)
	  inst.vectype.el[j] = neon_type_promote (&inst.vectype.el[key_el],
						  types[j]);
    }
  else if (inst.vectype.elems == 0 && els > 0)
    {
      unsigned j;
      /* No types were given after the mnemonic, so look for types specified
	 after each operand. We allow some flexibility here; as long as the
	 "key" operand has a type, we can infer the others.  */
      for (j = 0; j < els; j++)
	if (inst.operands[j].vectype.type != NT_invtype)
	  inst.vectype.el[j] = inst.operands[j].vectype;

      if (inst.operands[key_el].vectype.type != NT_invtype)
	{
	  for (j = 0; j < els; j++)
	    if (inst.operands[j].vectype.type == NT_invtype)
	      inst.vectype.el[j] = neon_type_promote (&inst.vectype.el[key_el],
						      types[j]);
	}
      else
	{
	  first_error (_("operand types can't be inferred"));
	  return badtype;
	}
    }
  else if (inst.vectype.elems != els)
    {
      first_error (_("type specifier has the wrong number of parts"));
      return badtype;
    }

  for (pass = 0; pass < 2; pass++)
    {
      for (i = 0; i < els; i++)
	{
	  unsigned thisarg = types[i];
	  unsigned types_allowed = ((thisarg & N_EQK) != 0 && pass != 0)
	    ? modify_types_allowed (key_allowed, thisarg) : thisarg;
	  enum neon_el_type g_type = inst.vectype.el[i].type;
	  unsigned g_size = inst.vectype.el[i].size;

	  /* Decay more-specific signed & unsigned types to sign-insensitive
	     integer types if sign-specific variants are unavailable.  */
	  if ((g_type == NT_signed || g_type == NT_unsigned)
	      && (types_allowed & N_SU_ALL) == 0)
	    g_type = NT_integer;

	  /* If only untyped args are allowed, decay any more specific types to
	     them. Some instructions only care about signs for some element
	     sizes, so handle that properly.  */
	  if (((types_allowed & N_UNT) == 0)
	      && ((g_size == 8 && (types_allowed & N_8) != 0)
		  || (g_size == 16 && (types_allowed & N_16) != 0)
		  || (g_size == 32 && (types_allowed & N_32) != 0)
		  || (g_size == 64 && (types_allowed & N_64) != 0)))
	    g_type = NT_untyped;

	  if (pass == 0)
	    {
	      if ((thisarg & N_KEY) != 0)
		{
		  k_type = g_type;
		  k_size = g_size;
		  key_allowed = thisarg & ~N_KEY;

		  /* Check architecture constraint on FP16 extension.  */
		  if (k_size == 16
		      && k_type == NT_float
		      && ! ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_fp16))
		    {
		      inst.error = _(BAD_FP16);
		      return badtype;
		    }
		}
	    }
	  else
	    {
	      if ((thisarg & N_VFP) != 0)
		{
		  enum neon_shape_el regshape;
		  unsigned regwidth, match;

		  /* PR 11136: Catch the case where we are passed a shape of NS_NULL.  */
		  if (ns == NS_NULL)
		    {
		      first_error (_("invalid instruction shape"));
		      return badtype;
		    }
		  regshape = neon_shape_tab[ns].el[i];
		  regwidth = neon_shape_el_size[regshape];

		  /* In VFP mode, operands must match register widths. If we
		     have a key operand, use its width, else use the width of
		     the current operand.  */
		  if (k_size != -1u)
		    match = k_size;
		  else
		    match = g_size;

		  /* FP16 will use a single precision register.  */
		  if (regwidth == 32 && match == 16)
		    {
		      if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_fp16))
			match = regwidth;
		      else
			{
			  inst.error = _(BAD_FP16);
			  return badtype;
			}
		    }

		  if (regwidth != match)
		    {
		      first_error (_("operand size must match register width"));
		      return badtype;
		    }
		}

	      if ((thisarg & N_EQK) == 0)
		{
		  unsigned given_type = type_chk_of_el_type (g_type, g_size);

		  if ((given_type & types_allowed) == 0)
		    {
		      first_error (BAD_SIMD_TYPE);
		      return badtype;
		    }
		}
	      else
		{
		  enum neon_el_type mod_k_type = k_type;
		  unsigned mod_k_size = k_size;
		  neon_modify_type_size (thisarg, &mod_k_type, &mod_k_size);
		  if (g_type != mod_k_type || g_size != mod_k_size)
		    {
		      first_error (_("inconsistent types in Neon instruction"));
		      return badtype;
		    }
		}
	    }
	}
    }

  return inst.vectype.el[key_el];
}

/* Neon-style VFP instruction forwarding.  */

/* Thumb VFP instructions have 0xE in the condition field.  */

static void
do_vfp_cond_or_thumb (void)
{
  inst.is_neon = 1;

  if (thumb_mode)
    inst.instruction |= 0xe0000000;
  else
    inst.instruction |= inst.cond << 28;
}

/* Look up and encode a simple mnemonic, for use as a helper function for the
   Neon-style VFP syntax.  This avoids duplication of bits of the insns table,
   etc.  It is assumed that operand parsing has already been done, and that the
   operands are in the form expected by the given opcode (this isn't necessarily
   the same as the form in which they were parsed, hence some massaging must
   take place before this function is called).
   Checks current arch version against that in the looked-up opcode.  */

static void
do_vfp_nsyn_opcode (const char *opname)
{
  const struct asm_opcode *opcode;

  opcode = (const struct asm_opcode *) str_hash_find (arm_ops_hsh, opname);

  if (!opcode)
    abort ();

  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant,
		thumb_mode ? *opcode->tvariant : *opcode->avariant),
	      _(BAD_FPU));

  inst.is_neon = 1;

  if (thumb_mode)
    {
      inst.instruction = opcode->tvalue;
      opcode->tencode ();
    }
  else
    {
      inst.instruction = (inst.cond << 28) | opcode->avalue;
      opcode->aencode ();
    }
}

static void
do_vfp_nsyn_add_sub (enum neon_shape rs)
{
  int is_add = (inst.instruction & 0x0fffffff) == N_MNEM_vadd;

  if (rs == NS_FFF || rs == NS_HHH)
    {
      if (is_add)
	do_vfp_nsyn_opcode ("fadds");
      else
	do_vfp_nsyn_opcode ("fsubs");

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HHH)
	do_scalar_fp16_v82_encode ();
    }
  else
    {
      if (is_add)
	do_vfp_nsyn_opcode ("faddd");
      else
	do_vfp_nsyn_opcode ("fsubd");
    }
}

/* Check operand types to see if this is a VFP instruction, and if so call
   PFN ().  */

static int
try_vfp_nsyn (int args, void (*pfn) (enum neon_shape))
{
  enum neon_shape rs;
  struct neon_type_el et;

  switch (args)
    {
    case 2:
      rs = neon_select_shape (NS_HH, NS_FF, NS_DD, NS_NULL);
      et = neon_check_type (2, rs, N_EQK | N_VFP, N_F_ALL | N_KEY | N_VFP);
      break;

    case 3:
      rs = neon_select_shape (NS_HHH, NS_FFF, NS_DDD, NS_NULL);
      et = neon_check_type (3, rs, N_EQK | N_VFP, N_EQK | N_VFP,
			    N_F_ALL | N_KEY | N_VFP);
      break;

    default:
      abort ();
    }

  if (et.type != NT_invtype)
    {
      pfn (rs);
      return SUCCESS;
    }

  inst.error = NULL;
  return FAIL;
}

static void
do_vfp_nsyn_mla_mls (enum neon_shape rs)
{
  int is_mla = (inst.instruction & 0x0fffffff) == N_MNEM_vmla;

  if (rs == NS_FFF || rs == NS_HHH)
    {
      if (is_mla)
	do_vfp_nsyn_opcode ("fmacs");
      else
	do_vfp_nsyn_opcode ("fnmacs");

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HHH)
	do_scalar_fp16_v82_encode ();
    }
  else
    {
      if (is_mla)
	do_vfp_nsyn_opcode ("fmacd");
      else
	do_vfp_nsyn_opcode ("fnmacd");
    }
}

static void
do_vfp_nsyn_fma_fms (enum neon_shape rs)
{
  int is_fma = (inst.instruction & 0x0fffffff) == N_MNEM_vfma;

  if (rs == NS_FFF || rs == NS_HHH)
    {
      if (is_fma)
	do_vfp_nsyn_opcode ("ffmas");
      else
	do_vfp_nsyn_opcode ("ffnmas");

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HHH)
	do_scalar_fp16_v82_encode ();
    }
  else
    {
      if (is_fma)
	do_vfp_nsyn_opcode ("ffmad");
      else
	do_vfp_nsyn_opcode ("ffnmad");
    }
}

static void
do_vfp_nsyn_mul (enum neon_shape rs)
{
  if (rs == NS_FFF || rs == NS_HHH)
    {
      do_vfp_nsyn_opcode ("fmuls");

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HHH)
	do_scalar_fp16_v82_encode ();
    }
  else
    do_vfp_nsyn_opcode ("fmuld");
}

static void
do_vfp_nsyn_abs_neg (enum neon_shape rs)
{
  int is_neg = (inst.instruction & 0x80) != 0;
  neon_check_type (2, rs, N_EQK | N_VFP, N_F_ALL | N_VFP | N_KEY);

  if (rs == NS_FF || rs == NS_HH)
    {
      if (is_neg)
	do_vfp_nsyn_opcode ("fnegs");
      else
	do_vfp_nsyn_opcode ("fabss");

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HH)
	do_scalar_fp16_v82_encode ();
    }
  else
    {
      if (is_neg)
	do_vfp_nsyn_opcode ("fnegd");
      else
	do_vfp_nsyn_opcode ("fabsd");
    }
}

/* Encode single-precision (only!) VFP fldm/fstm instructions. Double precision
   insns belong to Neon, and are handled elsewhere.  */

static void
do_vfp_nsyn_ldm_stm (int is_dbmode)
{
  int is_ldm = (inst.instruction & (1 << 20)) != 0;
  if (is_ldm)
    {
      if (is_dbmode)
	do_vfp_nsyn_opcode ("fldmdbs");
      else
	do_vfp_nsyn_opcode ("fldmias");
    }
  else
    {
      if (is_dbmode)
	do_vfp_nsyn_opcode ("fstmdbs");
      else
	do_vfp_nsyn_opcode ("fstmias");
    }
}

static void
do_vfp_nsyn_sqrt (void)
{
  enum neon_shape rs = neon_select_shape (NS_HH, NS_FF, NS_DD, NS_NULL);
  neon_check_type (2, rs, N_EQK | N_VFP, N_F_ALL | N_KEY | N_VFP);

  if (rs == NS_FF || rs == NS_HH)
    {
      do_vfp_nsyn_opcode ("fsqrts");

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HH)
	do_scalar_fp16_v82_encode ();
    }
  else
    do_vfp_nsyn_opcode ("fsqrtd");
}

static void
do_vfp_nsyn_div (void)
{
  enum neon_shape rs = neon_select_shape (NS_HHH, NS_FFF, NS_DDD, NS_NULL);
  neon_check_type (3, rs, N_EQK | N_VFP, N_EQK | N_VFP,
		   N_F_ALL | N_KEY | N_VFP);

  if (rs == NS_FFF || rs == NS_HHH)
    {
      do_vfp_nsyn_opcode ("fdivs");

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HHH)
	do_scalar_fp16_v82_encode ();
    }
  else
    do_vfp_nsyn_opcode ("fdivd");
}

static void
do_vfp_nsyn_nmul (void)
{
  enum neon_shape rs = neon_select_shape (NS_HHH, NS_FFF, NS_DDD, NS_NULL);
  neon_check_type (3, rs, N_EQK | N_VFP, N_EQK | N_VFP,
		   N_F_ALL | N_KEY | N_VFP);

  if (rs == NS_FFF || rs == NS_HHH)
    {
      NEON_ENCODE (SINGLE, inst);
      do_vfp_sp_dyadic ();

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HHH)
	do_scalar_fp16_v82_encode ();
    }
  else
    {
      NEON_ENCODE (DOUBLE, inst);
      do_vfp_dp_rd_rn_rm ();
    }
  do_vfp_cond_or_thumb ();

}

/* Turn a size (8, 16, 32, 64) into the respective bit number minus 3
   (0, 1, 2, 3).  */

static unsigned
neon_logbits (unsigned x)
{
  return ffs (x) - 4;
}

#define LOW4(R) ((R) & 0xf)
#define HI1(R) (((R) >> 4) & 1)
#define LOW1(R) ((R) & 0x1)
#define HI4(R) (((R) >> 1) & 0xf)

static unsigned
mve_get_vcmp_vpt_cond (struct neon_type_el et)
{
  switch (et.type)
    {
    default:
      first_error (BAD_EL_TYPE);
      return 0;
    case NT_float:
      switch (inst.operands[0].imm)
	{
	default:
	  first_error (_("invalid condition"));
	  return 0;
	case 0x0:
	  /* eq.  */
	  return 0;
	case 0x1:
	  /* ne.  */
	  return 1;
	case 0xa:
	  /* ge/  */
	  return 4;
	case 0xb:
	  /* lt.  */
	  return 5;
	case 0xc:
	  /* gt.  */
	  return 6;
	case 0xd:
	  /* le.  */
	  return 7;
	}
    case NT_integer:
      /* only accept eq and ne.  */
      if (inst.operands[0].imm > 1)
	{
	  first_error (_("invalid condition"));
	  return 0;
	}
      return inst.operands[0].imm;
    case NT_unsigned:
      if (inst.operands[0].imm == 0x2)
	return 2;
      else if (inst.operands[0].imm == 0x8)
	return 3;
      else
	{
	  first_error (_("invalid condition"));
	  return 0;
	}
    case NT_signed:
      switch (inst.operands[0].imm)
	{
	  default:
	    first_error (_("invalid condition"));
	    return 0;
	  case 0xa:
	    /* ge.  */
	    return 4;
	  case 0xb:
	    /* lt.  */
	    return 5;
	  case 0xc:
	    /* gt.  */
	    return 6;
	  case 0xd:
	    /* le.  */
	    return 7;
	}
    }
  /* Should be unreachable.  */
  abort ();
}

/* For VCTP (create vector tail predicate) in MVE.  */
static void
do_mve_vctp (void)
{
  int dt = 0;
  unsigned size = 0x0;

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  /* This is a typical MVE instruction which has no type but have size 8, 16,
     32 and 64.  For instructions with no type, inst.vectype.el[j].type is set
     to NT_untyped and size is updated in inst.vectype.el[j].size.  */
  if ((inst.operands[0].present) && (inst.vectype.el[0].type == NT_untyped))
    dt = inst.vectype.el[0].size;

  /* Setting this does not indicate an actual NEON instruction, but only
     indicates that the mnemonic accepts neon-style type suffixes.  */
  inst.is_neon = 1;

  switch (dt)
    {
      case 8:
	break;
      case 16:
	size = 0x1; break;
      case 32:
	size = 0x2; break;
      case 64:
	size = 0x3; break;
      default:
	first_error (_("Type is not allowed for this instruction"));
    }
  inst.instruction |= size << 20;
  inst.instruction |= inst.operands[0].reg << 16;
}

static void
do_mve_vpt (void)
{
  /* We are dealing with a vector predicated block.  */
  if (inst.operands[0].present)
    {
      enum neon_shape rs = neon_select_shape (NS_IQQ, NS_IQR, NS_NULL);
      struct neon_type_el et
	= neon_check_type (3, rs, N_EQK, N_KEY | N_F_MVE | N_I_MVE | N_SU_32,
			   N_EQK);

      unsigned fcond = mve_get_vcmp_vpt_cond (et);

      constraint (inst.operands[1].reg > 14, MVE_BAD_QREG);

      if (et.type == NT_invtype)
	return;

      if (et.type == NT_float)
	{
	  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext),
		      BAD_FPU);
	  constraint (et.size != 16 && et.size != 32, BAD_EL_TYPE);
	  inst.instruction |= (et.size == 16) << 28;
	  inst.instruction |= 0x3 << 20;
	}
      else
	{
	  constraint (et.size != 8 && et.size != 16 && et.size != 32,
		      BAD_EL_TYPE);
	  inst.instruction |= 1 << 28;
	  inst.instruction |= neon_logbits (et.size) << 20;
	}

      if (inst.operands[2].isquad)
	{
	  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
	  inst.instruction |= LOW4 (inst.operands[2].reg);
	  inst.instruction |= (fcond & 0x2) >> 1;
	}
      else
	{
	  if (inst.operands[2].reg == REG_SP)
	    as_tsktsk (MVE_BAD_SP);
	  inst.instruction |= 1 << 6;
	  inst.instruction |= (fcond & 0x2) << 4;
	  inst.instruction |= inst.operands[2].reg;
	}
      inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
      inst.instruction |= (fcond & 0x4) << 10;
      inst.instruction |= (fcond & 0x1) << 7;

    }
    set_pred_insn_type (VPT_INSN);
    now_pred.cc = 0;
    now_pred.mask = ((inst.instruction & 0x00400000) >> 19)
		    | ((inst.instruction & 0xe000) >> 13);
    now_pred.warn_deprecated = false;
    now_pred.type = VECTOR_PRED;
    inst.is_neon = 1;
}

static void
do_mve_vcmp (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext), BAD_FPU);
  if (!inst.operands[1].isreg || !inst.operands[1].isquad)
    first_error (_(reg_expected_msgs[REG_TYPE_MQ]));
  if (!inst.operands[2].present)
    first_error (_("MVE vector or ARM register expected"));
  constraint (inst.operands[1].reg > 14, MVE_BAD_QREG);

  /* Deal with 'else' conditional MVE's vcmp, it will be parsed as vcmpe.  */
  if ((inst.instruction & 0xffffffff) == N_MNEM_vcmpe
      && inst.operands[1].isquad)
    {
      inst.instruction = N_MNEM_vcmp;
      inst.cond = 0x10;
    }

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  enum neon_shape rs = neon_select_shape (NS_IQQ, NS_IQR, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_KEY | N_F_MVE | N_I_MVE | N_SU_32,
		       N_EQK);

  constraint (rs == NS_IQR && inst.operands[2].reg == REG_PC
	      && !inst.operands[2].iszr, BAD_PC);

  unsigned fcond = mve_get_vcmp_vpt_cond (et);

  inst.instruction = 0xee010f00;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= (fcond & 0x4) << 10;
  inst.instruction |= (fcond & 0x1) << 7;
  if (et.type == NT_float)
    {
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext),
		  BAD_FPU);
      inst.instruction |= (et.size == 16) << 28;
      inst.instruction |= 0x3 << 20;
    }
  else
    {
      inst.instruction |= 1 << 28;
      inst.instruction |= neon_logbits (et.size) << 20;
    }
  if (inst.operands[2].isquad)
    {
      inst.instruction |= HI1 (inst.operands[2].reg) << 5;
      inst.instruction |= (fcond & 0x2) >> 1;
      inst.instruction |= LOW4 (inst.operands[2].reg);
    }
  else
    {
      if (inst.operands[2].reg == REG_SP)
	as_tsktsk (MVE_BAD_SP);
      inst.instruction |= 1 << 6;
      inst.instruction |= (fcond & 0x2) << 4;
      inst.instruction |= inst.operands[2].reg;
    }

  inst.is_neon = 1;
  return;
}

static void
do_mve_vmaxa_vmina (void)
{
  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  enum neon_shape rs = neon_select_shape (NS_QQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (2, rs, N_EQK, N_KEY | N_S8 | N_S16 | N_S32);

  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= neon_logbits (et.size) << 18;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.is_neon = 1;
}

static void
do_mve_vfmas (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQR, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_F_MVE | N_KEY, N_EQK, N_EQK);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  if (inst.operands[2].reg == REG_SP)
    as_tsktsk (MVE_BAD_SP);
  else if (inst.operands[2].reg == REG_PC)
    as_tsktsk (MVE_BAD_PC);

  inst.instruction |= (et.size == 16) << 28;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= inst.operands[2].reg;
  inst.is_neon = 1;
}

static void
do_mve_viddup (void)
{
  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  unsigned imm = inst.relocs[0].exp.X_add_number;
  constraint (imm != 1 && imm != 2 && imm != 4 && imm != 8,
	      _("immediate must be either 1, 2, 4 or 8"));

  enum neon_shape rs;
  struct neon_type_el et;
  unsigned Rm;
  if (inst.instruction == M_MNEM_vddup || inst.instruction == M_MNEM_vidup)
    {
      rs = neon_select_shape (NS_QRI, NS_NULL);
      et = neon_check_type (2, rs, N_KEY | N_U8 | N_U16 | N_U32, N_EQK);
      Rm = 7;
    }
  else
    {
      constraint ((inst.operands[2].reg % 2) != 1, BAD_EVEN);
      if (inst.operands[2].reg == REG_SP)
	as_tsktsk (MVE_BAD_SP);
      else if (inst.operands[2].reg == REG_PC)
	first_error (BAD_PC);

      rs = neon_select_shape (NS_QRRI, NS_NULL);
      et = neon_check_type (3, rs, N_KEY | N_U8 | N_U16 | N_U32, N_EQK, N_EQK);
      Rm = inst.operands[2].reg >> 1;
    }
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= neon_logbits (et.size) << 20;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= (imm > 2) << 7;
  inst.instruction |= Rm << 1;
  inst.instruction |= (imm == 2 || imm == 8);
  inst.is_neon = 1;
}

static void
do_mve_vmlas (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQR, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_SU_MVE | N_KEY);

  if (inst.operands[2].reg == REG_PC)
    as_tsktsk (MVE_BAD_PC);
  else if (inst.operands[2].reg == REG_SP)
    as_tsktsk (MVE_BAD_SP);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  inst.instruction |= (et.type == NT_unsigned) << 28;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= neon_logbits (et.size) << 20;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= inst.operands[2].reg;
  inst.is_neon = 1;
}

static void
do_mve_vshll (void)
{
  struct neon_type_el et
    = neon_check_type (2, NS_QQI, N_EQK, N_S8 | N_U8 | N_S16 | N_U16 | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  int imm = inst.operands[2].imm;
  constraint (imm < 1 || (unsigned)imm > et.size,
	      _("immediate value out of range"));

  if ((unsigned)imm == et.size)
    {
      inst.instruction |= neon_logbits (et.size) << 18;
      inst.instruction |= 0x110001;
    }
  else
    {
      inst.instruction |= (et.size + imm) << 16;
      inst.instruction |= 0x800140;
    }

  inst.instruction |= (et.type == NT_unsigned) << 28;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.is_neon = 1;
}

static void
do_mve_vshlc (void)
{
  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  if (inst.operands[1].reg == REG_PC)
    as_tsktsk (MVE_BAD_PC);
  else if (inst.operands[1].reg == REG_SP)
    as_tsktsk (MVE_BAD_SP);

  int imm = inst.operands[2].imm;
  constraint (imm < 1 || imm > 32, _("immediate value out of range"));

  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= (imm & 0x1f) << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= inst.operands[1].reg;
  inst.is_neon = 1;
}

static void
do_mve_vshrn (void)
{
  unsigned types;
  switch (inst.instruction)
    {
    case M_MNEM_vshrnt:
    case M_MNEM_vshrnb:
    case M_MNEM_vrshrnt:
    case M_MNEM_vrshrnb:
      types = N_I16 | N_I32;
      break;
    case M_MNEM_vqshrnt:
    case M_MNEM_vqshrnb:
    case M_MNEM_vqrshrnt:
    case M_MNEM_vqrshrnb:
      types = N_U16 | N_U32 | N_S16 | N_S32;
      break;
    case M_MNEM_vqshrunt:
    case M_MNEM_vqshrunb:
    case M_MNEM_vqrshrunt:
    case M_MNEM_vqrshrunb:
      types = N_S16 | N_S32;
      break;
    default:
      abort ();
    }

  struct neon_type_el et = neon_check_type (2, NS_QQI, N_EQK, types | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  unsigned Qd = inst.operands[0].reg;
  unsigned Qm = inst.operands[1].reg;
  unsigned imm = inst.operands[2].imm;
  constraint (imm < 1 || ((unsigned) imm) > (et.size / 2),
	      et.size == 16
	      ? _("immediate operand expected in the range [1,8]")
	      : _("immediate operand expected in the range [1,16]"));

  inst.instruction |= (et.type == NT_unsigned) << 28;
  inst.instruction |= HI1 (Qd) << 22;
  inst.instruction |= (et.size - imm) << 16;
  inst.instruction |= LOW4 (Qd) << 12;
  inst.instruction |= HI1 (Qm) << 5;
  inst.instruction |= LOW4 (Qm);
  inst.is_neon = 1;
}

static void
do_mve_vqmovn (void)
{
  struct neon_type_el et;
  if (inst.instruction == M_MNEM_vqmovnt
     || inst.instruction == M_MNEM_vqmovnb)
    et = neon_check_type (2, NS_QQ, N_EQK,
			  N_U16 | N_U32 | N_S16 | N_S32 | N_KEY);
  else
    et = neon_check_type (2, NS_QQ, N_EQK, N_S16 | N_S32 | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  inst.instruction |= (et.type == NT_unsigned) << 28;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= (et.size == 32) << 18;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.is_neon = 1;
}

static void
do_mve_vpsel (void)
{
  neon_select_shape (NS_QQQ, NS_NULL);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[2].reg);
  inst.is_neon = 1;
}

static void
do_mve_vpnot (void)
{
  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;
}

static void
do_mve_vmaxnma_vminnma (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (2, rs, N_EQK, N_F_MVE | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  inst.instruction |= (et.size == 16) << 28;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.is_neon = 1;
}

static void
do_mve_vcmul (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQI, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_F_MVE | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  unsigned rot = inst.relocs[0].exp.X_add_number;
  constraint (rot != 0 && rot != 90 && rot != 180 && rot != 270,
	      _("immediate out of range"));

  if (et.size == 32 && (inst.operands[0].reg == inst.operands[1].reg
			|| inst.operands[0].reg == inst.operands[2].reg))
    as_tsktsk (BAD_MVE_SRCDEST);

  inst.instruction |= (et.size == 32) << 28;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= (rot > 90) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[2].reg);
  inst.instruction |= (rot == 90 || rot == 270);
  inst.is_neon = 1;
}

/* To handle the Low Overhead Loop instructions
   in Armv8.1-M Mainline and MVE.  */
static void
do_t_loloop (void)
{
  unsigned long insn = inst.instruction;

  inst.instruction = THUMB_OP32 (inst.instruction);

  if (insn == T_MNEM_lctp)
    return;

  set_pred_insn_type (MVE_OUTSIDE_PRED_INSN);

  if (insn == T_MNEM_wlstp || insn == T_MNEM_dlstp)
    {
      struct neon_type_el et
       = neon_check_type (2, NS_RR, N_EQK, N_8 | N_16 | N_32 | N_64 | N_KEY);
      inst.instruction |= neon_logbits (et.size) << 20;
      inst.is_neon = 1;
    }

  switch (insn)
    {
    case T_MNEM_letp:
      constraint (!inst.operands[0].present,
		  _("expected LR"));
      /* fall through.  */
    case T_MNEM_le:
      /* le <label>.  */
      if (!inst.operands[0].present)
       inst.instruction |= 1 << 21;

      v8_1_loop_reloc (true);
      break;

    case T_MNEM_wls:
    case T_MNEM_wlstp:
      v8_1_loop_reloc (false);
      /* fall through.  */
    case T_MNEM_dlstp:
    case T_MNEM_dls:
      constraint (inst.operands[1].isreg != 1, BAD_ARGS);

      if (insn == T_MNEM_wlstp || insn == T_MNEM_dlstp)
       constraint (inst.operands[1].reg == REG_PC, BAD_PC);
      else if (inst.operands[1].reg == REG_PC)
       as_tsktsk (MVE_BAD_PC);
      if (inst.operands[1].reg == REG_SP)
       as_tsktsk (MVE_BAD_SP);

      inst.instruction |= (inst.operands[1].reg << 16);
      break;

    default:
      abort ();
    }
}


static void
do_vfp_nsyn_cmp (void)
{
  enum neon_shape rs;
  if (!inst.operands[0].isreg)
    {
      do_mve_vcmp ();
      return;
    }
  else
    {
      constraint (inst.operands[2].present, BAD_SYNTAX);
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd),
		  BAD_FPU);
    }

  if (inst.operands[1].isreg)
    {
      rs = neon_select_shape (NS_HH, NS_FF, NS_DD, NS_NULL);
      neon_check_type (2, rs, N_EQK | N_VFP, N_F_ALL | N_KEY | N_VFP);

      if (rs == NS_FF || rs == NS_HH)
	{
	  NEON_ENCODE (SINGLE, inst);
	  do_vfp_sp_monadic ();
	}
      else
	{
	  NEON_ENCODE (DOUBLE, inst);
	  do_vfp_dp_rd_rm ();
	}
    }
  else
    {
      rs = neon_select_shape (NS_HI, NS_FI, NS_DI, NS_NULL);
      neon_check_type (2, rs, N_F_ALL | N_KEY | N_VFP, N_EQK);

      switch (inst.instruction & 0x0fffffff)
	{
	case N_MNEM_vcmp:
	  inst.instruction += N_MNEM_vcmpz - N_MNEM_vcmp;
	  break;
	case N_MNEM_vcmpe:
	  inst.instruction += N_MNEM_vcmpez - N_MNEM_vcmpe;
	  break;
	default:
	  abort ();
	}

      if (rs == NS_FI || rs == NS_HI)
	{
	  NEON_ENCODE (SINGLE, inst);
	  do_vfp_sp_compare_z ();
	}
      else
	{
	  NEON_ENCODE (DOUBLE, inst);
	  do_vfp_dp_rd ();
	}
    }
  do_vfp_cond_or_thumb ();

  /* ARMv8.2 fp16 instruction.  */
  if (rs == NS_HI || rs == NS_HH)
    do_scalar_fp16_v82_encode ();
}

static void
nsyn_insert_sp (void)
{
  inst.operands[1] = inst.operands[0];
  memset (&inst.operands[0], '\0', sizeof (inst.operands[0]));
  inst.operands[0].reg = REG_SP;
  inst.operands[0].isreg = 1;
  inst.operands[0].writeback = 1;
  inst.operands[0].present = 1;
}

/* Fix up Neon data-processing instructions, ORing in the correct bits for
   ARM mode or Thumb mode and moving the encoded bit 24 to bit 28.  */

static void
neon_dp_fixup (struct arm_it* insn)
{
  unsigned int i = insn->instruction;
  insn->is_neon = 1;

  if (thumb_mode)
    {
      /* The U bit is at bit 24 by default. Move to bit 28 in Thumb mode.  */
      if (i & (1 << 24))
	i |= 1 << 28;

      i &= ~(1 << 24);

      i |= 0xef000000;
    }
  else
    i |= 0xf2000000;

  insn->instruction = i;
}

static void
mve_encode_qqr (int size, int U, int fp)
{
  if (inst.operands[2].reg == REG_SP)
    as_tsktsk (MVE_BAD_SP);
  else if (inst.operands[2].reg == REG_PC)
    as_tsktsk (MVE_BAD_PC);

  if (fp)
    {
      /* vadd.  */
      if (((unsigned)inst.instruction) == 0xd00)
	inst.instruction = 0xee300f40;
      /* vsub.  */
      else if (((unsigned)inst.instruction) == 0x200d00)
	inst.instruction = 0xee301f40;
      /* vmul.  */
      else if (((unsigned)inst.instruction) == 0x1000d10)
	inst.instruction = 0xee310e60;

      /* Setting size which is 1 for F16 and 0 for F32.  */
      inst.instruction |= (size == 16) << 28;
    }
  else
    {
      /* vadd.  */
      if (((unsigned)inst.instruction) == 0x800)
	inst.instruction = 0xee010f40;
      /* vsub.  */
      else if (((unsigned)inst.instruction) == 0x1000800)
	inst.instruction = 0xee011f40;
      /* vhadd.  */
      else if (((unsigned)inst.instruction) == 0)
	inst.instruction = 0xee000f40;
      /* vhsub.  */
      else if (((unsigned)inst.instruction) == 0x200)
	inst.instruction = 0xee001f40;
      /* vmla.  */
      else if (((unsigned)inst.instruction) == 0x900)
	inst.instruction = 0xee010e40;
      /* vmul.  */
      else if (((unsigned)inst.instruction) == 0x910)
	inst.instruction = 0xee011e60;
      /* vqadd.  */
      else if (((unsigned)inst.instruction) == 0x10)
	inst.instruction = 0xee000f60;
      /* vqsub.  */
      else if (((unsigned)inst.instruction) == 0x210)
	inst.instruction = 0xee001f60;
      /* vqrdmlah.  */
      else if (((unsigned)inst.instruction) == 0x3000b10)
	inst.instruction = 0xee000e40;
      /* vqdmulh.  */
      else if (((unsigned)inst.instruction) == 0x0000b00)
	inst.instruction = 0xee010e60;
      /* vqrdmulh.  */
      else if (((unsigned)inst.instruction) == 0x1000b00)
	inst.instruction = 0xfe010e60;

      /* Set U-bit.  */
      inst.instruction |= U << 28;

      /* Setting bits for size.  */
      inst.instruction |= neon_logbits (size) << 20;
    }
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= inst.operands[2].reg;
  inst.is_neon = 1;
}

static void
mve_encode_rqq (unsigned bit28, unsigned size)
{
  inst.instruction |= bit28 << 28;
  inst.instruction |= neon_logbits (size) << 20;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[2].reg);
  inst.is_neon = 1;
}

static void
mve_encode_qqq (int ubit, int size)
{

  inst.instruction |= (ubit != 0) << 28;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= neon_logbits (size) << 20;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[2].reg);

  inst.is_neon = 1;
}

static void
mve_encode_rq (unsigned bit28, unsigned size)
{
  inst.instruction |= bit28 << 28;
  inst.instruction |= neon_logbits (size) << 18;
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.is_neon = 1;
}

static void
mve_encode_rrqq (unsigned U, unsigned size)
{
  constraint (inst.operands[3].reg > 14, MVE_BAD_QREG);

  inst.instruction |= U << 28;
  inst.instruction |= (inst.operands[1].reg >> 1) << 20;
  inst.instruction |= LOW4 (inst.operands[2].reg) << 16;
  inst.instruction |= (size == 32) << 16;
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= HI1 (inst.operands[2].reg) << 7;
  inst.instruction |= inst.operands[3].reg;
  inst.is_neon = 1;
}

/* Helper function for neon_three_same handling the operands.  */
static void
neon_three_args (int isquad)
{
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= LOW4 (inst.operands[2].reg);
  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
  inst.instruction |= (isquad != 0) << 6;
  inst.is_neon = 1;
}

/* Encode insns with bit pattern:

  |28/24|23|22 |21 20|19 16|15 12|11    8|7|6|5|4|3  0|
  |  U  |x |D  |size | Rn  | Rd  |x x x x|N|Q|M|x| Rm |

  SIZE is passed in bits. -1 means size field isn't changed, in case it has a
  different meaning for some instruction.  */

static void
neon_three_same (int isquad, int ubit, int size)
{
  neon_three_args (isquad);
  inst.instruction |= (ubit != 0) << 24;
  if (size != -1)
    inst.instruction |= neon_logbits (size) << 20;

  neon_dp_fixup (&inst);
}

/* Encode instructions of the form:

  |28/24|23|22|21 20|19 18|17 16|15 12|11      7|6|5|4|3  0|
  |  U  |x |D |x  x |size |x  x | Rd  |x x x x x|Q|M|x| Rm |

  Don't write size if SIZE == -1.  */

static void
neon_two_same (int qbit, int ubit, int size)
{
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= (qbit != 0) << 6;
  inst.instruction |= (ubit != 0) << 24;

  if (size != -1)
    inst.instruction |= neon_logbits (size) << 18;

  neon_dp_fixup (&inst);
}

enum vfp_or_neon_is_neon_bits
{
NEON_CHECK_CC = 1,
NEON_CHECK_ARCH = 2,
NEON_CHECK_ARCH8 = 4
};

/* Call this function if an instruction which may have belonged to the VFP or
 Neon instruction sets, but turned out to be a Neon instruction (due to the
 operand types involved, etc.). We have to check and/or fix-up a couple of
 things:

   - Make sure the user hasn't attempted to make a Neon instruction
     conditional.
   - Alter the value in the condition code field if necessary.
   - Make sure that the arch supports Neon instructions.

 Which of these operations take place depends on bits from enum
 vfp_or_neon_is_neon_bits.

 WARNING: This function has side effects! If NEON_CHECK_CC is used and the
 current instruction's condition is COND_ALWAYS, the condition field is
 changed to inst.uncond_value.  This is necessary because instructions shared
 between VFP and Neon may be conditional for the VFP variants only, and the
 unconditional Neon version must have, e.g., 0xF in the condition field.  */

static int
vfp_or_neon_is_neon (unsigned check)
{
/* Conditions are always legal in Thumb mode (IT blocks).  */
if (!thumb_mode && (check & NEON_CHECK_CC))
  {
    if (inst.cond != COND_ALWAYS)
      {
	first_error (_(BAD_COND));
	return FAIL;
      }
    if (inst.uncond_value != -1u)
      inst.instruction |= inst.uncond_value << 28;
  }


  if (((check & NEON_CHECK_ARCH) && !mark_feature_used (&fpu_neon_ext_v1))
      || ((check & NEON_CHECK_ARCH8)
	  && !mark_feature_used (&fpu_neon_ext_armv8)))
    {
      first_error (_(BAD_FPU));
      return FAIL;
    }

return SUCCESS;
}


/* Return TRUE if the SIMD instruction is available for the current
   cpu_variant.  FP is set to TRUE if this is a SIMD floating-point
   instruction.  CHECK contains th.  CHECK contains the set of bits to pass to
   vfp_or_neon_is_neon for the NEON specific checks.  */

static bool
check_simd_pred_availability (int fp, unsigned check)
{
if (inst.cond > COND_ALWAYS)
  {
    if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
      {
	inst.error = BAD_FPU;
	return false;
      }
    inst.pred_insn_type = INSIDE_VPT_INSN;
  }
else if (inst.cond < COND_ALWAYS)
  {
    if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
      inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;
    else if (vfp_or_neon_is_neon (check) == FAIL)
      return false;
  }
else
  {
    if (!ARM_CPU_HAS_FEATURE (cpu_variant, fp ? mve_fp_ext : mve_ext)
	&& vfp_or_neon_is_neon (check) == FAIL)
      return false;

    if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
      inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;
  }
return true;
}

/* Neon instruction encoders, in approximate order of appearance.  */

static void
do_neon_dyadic_i_su (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
   return;

  enum neon_shape rs;
  struct neon_type_el et;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    rs = neon_select_shape (NS_QQQ, NS_QQR, NS_NULL);
  else
    rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);

  et = neon_check_type (3, rs, N_EQK, N_EQK, N_SU_32 | N_KEY);


  if (rs != NS_QQR)
    neon_three_same (neon_quad (rs), et.type == NT_unsigned, et.size);
  else
    mve_encode_qqr (et.size, et.type == NT_unsigned, 0);
}

static void
do_neon_dyadic_i64_su (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_CC | NEON_CHECK_ARCH))
    return;
  enum neon_shape rs;
  struct neon_type_el et;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      rs = neon_select_shape (NS_QQR, NS_QQQ, NS_NULL);
      et = neon_check_type (3, rs, N_EQK, N_EQK, N_SU_MVE | N_KEY);
    }
  else
    {
      rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
      et = neon_check_type (3, rs, N_EQK, N_EQK, N_SU_ALL | N_KEY);
    }
  if (rs == NS_QQR)
    mve_encode_qqr (et.size, et.type == NT_unsigned, 0);
  else
    neon_three_same (neon_quad (rs), et.type == NT_unsigned, et.size);
}

static void
neon_imm_shift (int write_ubit, int uval, int isquad, struct neon_type_el et,
		unsigned immbits)
{
  unsigned size = et.size >> 3;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= (isquad != 0) << 6;
  inst.instruction |= immbits << 16;
  inst.instruction |= (size >> 3) << 7;
  inst.instruction |= (size & 0x7) << 19;
  if (write_ubit)
    inst.instruction |= (uval != 0) << 24;

  neon_dp_fixup (&inst);
}

static void
do_neon_shl (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
   return;

  if (!inst.operands[2].isreg)
    {
      enum neon_shape rs;
      struct neon_type_el et;
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	{
	  rs = neon_select_shape (NS_QQI, NS_NULL);
	  et = neon_check_type (2, rs, N_EQK, N_KEY | N_I_MVE);
	}
      else
	{
	  rs = neon_select_shape (NS_DDI, NS_QQI, NS_NULL);
	  et = neon_check_type (2, rs, N_EQK, N_KEY | N_I_ALL);
	}
      int imm = inst.operands[2].imm;

      constraint (imm < 0 || (unsigned)imm >= et.size,
		  _("immediate out of range for shift"));
      NEON_ENCODE (IMMED, inst);
      neon_imm_shift (false, 0, neon_quad (rs), et, imm);
    }
  else
    {
      enum neon_shape rs;
      struct neon_type_el et;
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	{
	  rs = neon_select_shape (NS_QQQ, NS_QQR, NS_NULL);
	  et = neon_check_type (3, rs, N_EQK, N_SU_MVE | N_KEY, N_EQK | N_EQK);
	}
      else
	{
	  rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
	  et = neon_check_type (3, rs, N_EQK, N_SU_ALL | N_KEY, N_EQK | N_SGN);
	}


      if (rs == NS_QQR)
	{
	  constraint (inst.operands[0].reg != inst.operands[1].reg,
		       _("invalid instruction shape"));
	  if (inst.operands[2].reg == REG_SP)
	    as_tsktsk (MVE_BAD_SP);
	  else if (inst.operands[2].reg == REG_PC)
	    as_tsktsk (MVE_BAD_PC);

	  inst.instruction = 0xee311e60;
	  inst.instruction |= (et.type == NT_unsigned) << 28;
	  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
	  inst.instruction |= neon_logbits (et.size) << 18;
	  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
	  inst.instruction |= inst.operands[2].reg;
	  inst.is_neon = 1;
	}
      else
	{
	  unsigned int tmp;

	  /* VSHL/VQSHL 3-register variants have syntax such as:
	       vshl.xx Dd, Dm, Dn
	     whereas other 3-register operations encoded by neon_three_same have
	     syntax like:
	       vadd.xx Dd, Dn, Dm
	     (i.e. with Dn & Dm reversed). Swap operands[1].reg and
	     operands[2].reg here.  */
	  tmp = inst.operands[2].reg;
	  inst.operands[2].reg = inst.operands[1].reg;
	  inst.operands[1].reg = tmp;
	  NEON_ENCODE (INTEGER, inst);
	  neon_three_same (neon_quad (rs), et.type == NT_unsigned, et.size);
	}
    }
}

static void
do_neon_qshl (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
   return;

  if (!inst.operands[2].isreg)
    {
      enum neon_shape rs;
      struct neon_type_el et;
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	{
	  rs = neon_select_shape (NS_QQI, NS_NULL);
	  et = neon_check_type (2, rs, N_EQK, N_KEY | N_SU_MVE);
	}
      else
	{
	  rs = neon_select_shape (NS_DDI, NS_QQI, NS_NULL);
	  et = neon_check_type (2, rs, N_EQK, N_SU_ALL | N_KEY);
	}
      int imm = inst.operands[2].imm;

      constraint (imm < 0 || (unsigned)imm >= et.size,
		  _("immediate out of range for shift"));
      NEON_ENCODE (IMMED, inst);
      neon_imm_shift (true, et.type == NT_unsigned, neon_quad (rs), et, imm);
    }
  else
    {
      enum neon_shape rs;
      struct neon_type_el et;

      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	{
	  rs = neon_select_shape (NS_QQQ, NS_QQR, NS_NULL);
	  et = neon_check_type (3, rs, N_EQK, N_SU_MVE | N_KEY, N_EQK | N_EQK);
	}
      else
	{
	  rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
	  et = neon_check_type (3, rs, N_EQK, N_SU_ALL | N_KEY, N_EQK | N_SGN);
	}

      if (rs == NS_QQR)
	{
	  constraint (inst.operands[0].reg != inst.operands[1].reg,
		       _("invalid instruction shape"));
	  if (inst.operands[2].reg == REG_SP)
	    as_tsktsk (MVE_BAD_SP);
	  else if (inst.operands[2].reg == REG_PC)
	    as_tsktsk (MVE_BAD_PC);

	  inst.instruction = 0xee311ee0;
	  inst.instruction |= (et.type == NT_unsigned) << 28;
	  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
	  inst.instruction |= neon_logbits (et.size) << 18;
	  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
	  inst.instruction |= inst.operands[2].reg;
	  inst.is_neon = 1;
	}
      else
	{
	  unsigned int tmp;

	  /* See note in do_neon_shl.  */
	  tmp = inst.operands[2].reg;
	  inst.operands[2].reg = inst.operands[1].reg;
	  inst.operands[1].reg = tmp;
	  NEON_ENCODE (INTEGER, inst);
	  neon_three_same (neon_quad (rs), et.type == NT_unsigned, et.size);
	}
    }
}

static void
do_neon_rshl (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
   return;

  enum neon_shape rs;
  struct neon_type_el et;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      rs = neon_select_shape (NS_QQR, NS_QQQ, NS_NULL);
      et = neon_check_type (3, rs, N_EQK, N_EQK, N_SU_MVE | N_KEY);
    }
  else
    {
      rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
      et = neon_check_type (3, rs, N_EQK, N_EQK, N_SU_ALL | N_KEY);
    }

  unsigned int tmp;

  if (rs == NS_QQR)
    {
      if (inst.operands[2].reg == REG_PC)
	as_tsktsk (MVE_BAD_PC);
      else if (inst.operands[2].reg == REG_SP)
	as_tsktsk (MVE_BAD_SP);

      constraint (inst.operands[0].reg != inst.operands[1].reg,
		  _("invalid instruction shape"));

      if (inst.instruction == 0x0000510)
	/* We are dealing with vqrshl.  */
	inst.instruction = 0xee331ee0;
      else
	/* We are dealing with vrshl.  */
	inst.instruction = 0xee331e60;

      inst.instruction |= (et.type == NT_unsigned) << 28;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= neon_logbits (et.size) << 18;
      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= inst.operands[2].reg;
      inst.is_neon = 1;
    }
  else
    {
      tmp = inst.operands[2].reg;
      inst.operands[2].reg = inst.operands[1].reg;
      inst.operands[1].reg = tmp;
      neon_three_same (neon_quad (rs), et.type == NT_unsigned, et.size);
    }
}

static int
neon_cmode_for_logic_imm (unsigned immediate, unsigned *immbits, int size)
{
  /* Handle .I8 pseudo-instructions.  */
  if (size == 8)
    {
      /* Unfortunately, this will make everything apart from zero out-of-range.
	 FIXME is this the intended semantics? There doesn't seem much point in
	 accepting .I8 if so.  */
      immediate |= immediate << 8;
      size = 16;
    }

  if (size >= 32)
    {
      if (immediate == (immediate & 0x000000ff))
	{
	  *immbits = immediate;
	  return 0x1;
	}
      else if (immediate == (immediate & 0x0000ff00))
	{
	  *immbits = immediate >> 8;
	  return 0x3;
	}
      else if (immediate == (immediate & 0x00ff0000))
	{
	  *immbits = immediate >> 16;
	  return 0x5;
	}
      else if (immediate == (immediate & 0xff000000))
	{
	  *immbits = immediate >> 24;
	  return 0x7;
	}
      if ((immediate & 0xffff) != (immediate >> 16))
	goto bad_immediate;
      immediate &= 0xffff;
    }

  if (immediate == (immediate & 0x000000ff))
    {
      *immbits = immediate;
      return 0x9;
    }
  else if (immediate == (immediate & 0x0000ff00))
    {
      *immbits = immediate >> 8;
      return 0xb;
    }

  bad_immediate:
  first_error (_("immediate value out of range"));
  return FAIL;
}

static void
do_neon_logic (void)
{
  if (inst.operands[2].present && inst.operands[2].isreg)
    {
      enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
      if (rs == NS_QQQ
	  && !check_simd_pred_availability (false,
					    NEON_CHECK_ARCH | NEON_CHECK_CC))
	return;
      else if (rs != NS_QQQ
	       && !ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_v1))
	first_error (BAD_FPU);

      neon_check_type (3, rs, N_IGNORE_TYPE);
      /* U bit and size field were set as part of the bitmask.  */
      NEON_ENCODE (INTEGER, inst);
      neon_three_same (neon_quad (rs), 0, -1);
    }
  else
    {
      const int three_ops_form = (inst.operands[2].present
				  && !inst.operands[2].isreg);
      const int immoperand = (three_ops_form ? 2 : 1);
      enum neon_shape rs = (three_ops_form
			    ? neon_select_shape (NS_DDI, NS_QQI, NS_NULL)
			    : neon_select_shape (NS_DI, NS_QI, NS_NULL));
      /* Because neon_select_shape makes the second operand a copy of the first
	 if the second operand is not present.  */
      if (rs == NS_QQI
	  && !check_simd_pred_availability (false,
					    NEON_CHECK_ARCH | NEON_CHECK_CC))
	return;
      else if (rs != NS_QQI
	       && !ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_v1))
	first_error (BAD_FPU);

      struct neon_type_el et;
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	et = neon_check_type (2, rs, N_I32 | N_I16 | N_KEY, N_EQK);
      else
	et = neon_check_type (2, rs, N_I8 | N_I16 | N_I32 | N_I64 | N_F32
			      | N_KEY, N_EQK);

      if (et.type == NT_invtype)
	return;
      enum neon_opc opcode = (enum neon_opc) inst.instruction & 0x0fffffff;
      unsigned immbits;
      int cmode;


      if (three_ops_form)
	constraint (inst.operands[0].reg != inst.operands[1].reg,
		    _("first and second operands shall be the same register"));

      NEON_ENCODE (IMMED, inst);

      immbits = inst.operands[immoperand].imm;
      if (et.size == 64)
	{
	  /* .i64 is a pseudo-op, so the immediate must be a repeating
	     pattern.  */
	  if (immbits != (inst.operands[immoperand].regisimm ?
			  inst.operands[immoperand].reg : 0))
	    {
	      /* Set immbits to an invalid constant.  */
	      immbits = 0xdeadbeef;
	    }
	}

      switch (opcode)
	{
	case N_MNEM_vbic:
	  cmode = neon_cmode_for_logic_imm (immbits, &immbits, et.size);
	  break;

	case N_MNEM_vorr:
	  cmode = neon_cmode_for_logic_imm (immbits, &immbits, et.size);
	  break;

	case N_MNEM_vand:
	  /* Pseudo-instruction for VBIC.  */
	  neon_invert_size (&immbits, 0, et.size);
	  cmode = neon_cmode_for_logic_imm (immbits, &immbits, et.size);
	  break;

	case N_MNEM_vorn:
	  /* Pseudo-instruction for VORR.  */
	  neon_invert_size (&immbits, 0, et.size);
	  cmode = neon_cmode_for_logic_imm (immbits, &immbits, et.size);
	  break;

	default:
	  abort ();
	}

      if (cmode == FAIL)
	return;

      inst.instruction |= neon_quad (rs) << 6;
      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= cmode << 8;
      neon_write_immbits (immbits);

      neon_dp_fixup (&inst);
    }
}

static void
do_neon_bitfield (void)
{
  enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
  neon_check_type (3, rs, N_IGNORE_TYPE);
  neon_three_same (neon_quad (rs), 0, -1);
}

static void
neon_dyadic_misc (enum neon_el_type ubit_meaning, unsigned types,
		  unsigned destbits)
{
  enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_QQR, NS_NULL);
  struct neon_type_el et = neon_check_type (3, rs, N_EQK | destbits, N_EQK,
					    types | N_KEY);
  if (et.type == NT_float)
    {
      NEON_ENCODE (FLOAT, inst);
      if (rs == NS_QQR)
	mve_encode_qqr (et.size, 0, 1);
      else
	neon_three_same (neon_quad (rs), 0, et.size == 16 ? (int) et.size : -1);
    }
  else
    {
      NEON_ENCODE (INTEGER, inst);
      if (rs == NS_QQR)
	mve_encode_qqr (et.size, et.type == ubit_meaning, 0);
      else
	neon_three_same (neon_quad (rs), et.type == ubit_meaning, et.size);
    }
}


static void
do_neon_dyadic_if_su_d (void)
{
  /* This version only allow D registers, but that constraint is enforced during
     operand parsing so we don't need to do anything extra here.  */
  neon_dyadic_misc (NT_unsigned, N_SUF_32, 0);
}

static void
do_neon_dyadic_if_i_d (void)
{
  /* The "untyped" case can't happen. Do this to stop the "U" bit being
     affected if we specify unsigned args.  */
  neon_dyadic_misc (NT_untyped, N_IF_32, 0);
}

static void
do_mve_vstr_vldr_QI (int size, int elsize, int load)
{
  constraint (size < 32, BAD_ADDR_MODE);
  constraint (size != elsize, BAD_EL_TYPE);
  constraint (inst.operands[1].immisreg, BAD_ADDR_MODE);
  constraint (!inst.operands[1].preind, BAD_ADDR_MODE);
  constraint (load && inst.operands[0].reg == inst.operands[1].reg,
	      _("destination register and offset register may not be the"
		" same"));

  int imm = inst.relocs[0].exp.X_add_number;
  int add = 1;
  if (imm < 0)
    {
      add = 0;
      imm = -imm;
    }
  constraint ((imm % (size / 8) != 0)
	      || imm > (0x7f << neon_logbits (size)),
	      (size == 32) ? _("immediate must be a multiple of 4 in the"
			       " range of +/-[0,508]")
			   : _("immediate must be a multiple of 8 in the"
			       " range of +/-[0,1016]"));
  inst.instruction |= 0x11 << 24;
  inst.instruction |= add << 23;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= inst.operands[1].writeback << 21;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= 1 << 12;
  inst.instruction |= (size == 64) << 8;
  inst.instruction &= 0xffffff00;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= imm >> neon_logbits (size);
}

static void
do_mve_vstr_vldr_RQ (int size, int elsize, int load)
{
    unsigned os = inst.operands[1].imm >> 5;
    unsigned type = inst.vectype.el[0].type;
    constraint (os != 0 && size == 8,
		_("can not shift offsets when accessing less than half-word"));
    constraint (os && os != neon_logbits (size),
		_("shift immediate must be 1, 2 or 3 for half-word, word"
		  " or double-word accesses respectively"));
    if (inst.operands[1].reg == REG_PC)
      as_tsktsk (MVE_BAD_PC);

    switch (size)
      {
      case 8:
	constraint (elsize >= 64, BAD_EL_TYPE);
	break;
      case 16:
	constraint (elsize < 16 || elsize >= 64, BAD_EL_TYPE);
	break;
      case 32:
      case 64:
	constraint (elsize != size, BAD_EL_TYPE);
	break;
      default:
	break;
      }
    constraint (inst.operands[1].writeback || !inst.operands[1].preind,
		BAD_ADDR_MODE);
    if (load)
      {
	constraint (inst.operands[0].reg == (inst.operands[1].imm & 0x1f),
		    _("destination register and offset register may not be"
		    " the same"));
	constraint (size == elsize && type == NT_signed, BAD_EL_TYPE);
	constraint (size != elsize && type != NT_unsigned && type != NT_signed,
		    BAD_EL_TYPE);
	inst.instruction |= ((size == elsize) || (type == NT_unsigned)) << 28;
      }
    else
      {
	constraint (type != NT_untyped, BAD_EL_TYPE);
      }

    inst.instruction |= 1 << 23;
    inst.instruction |= HI1 (inst.operands[0].reg) << 22;
    inst.instruction |= inst.operands[1].reg << 16;
    inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
    inst.instruction |= neon_logbits (elsize) << 7;
    inst.instruction |= HI1 (inst.operands[1].imm) << 5;
    inst.instruction |= LOW4 (inst.operands[1].imm);
    inst.instruction |= !!os;
}

static void
do_mve_vstr_vldr_RI (int size, int elsize, int load)
{
  enum neon_el_type type = inst.vectype.el[0].type;

  constraint (size >= 64, BAD_ADDR_MODE);
  switch (size)
    {
    case 16:
      constraint (elsize < 16 || elsize >= 64, BAD_EL_TYPE);
      break;
    case 32:
      constraint (elsize != size, BAD_EL_TYPE);
      break;
    default:
      break;
    }
  if (load)
    {
      constraint (elsize != size && type != NT_unsigned
		  && type != NT_signed, BAD_EL_TYPE);
    }
  else
    {
      constraint (elsize != size && type != NT_untyped, BAD_EL_TYPE);
    }

  int imm = inst.relocs[0].exp.X_add_number;
  int add = 1;
  if (imm < 0)
    {
      add = 0;
      imm = -imm;
    }

  if ((imm % (size / 8) != 0) || imm > (0x7f << neon_logbits (size)))
    {
      switch (size)
	{
	case 8:
	  constraint (1, _("immediate must be in the range of +/-[0,127]"));
	  break;
	case 16:
	  constraint (1, _("immediate must be a multiple of 2 in the"
			   " range of +/-[0,254]"));
	  break;
	case 32:
	  constraint (1, _("immediate must be a multiple of 4 in the"
			   " range of +/-[0,508]"));
	  break;
	}
    }

  if (size != elsize)
    {
      constraint (inst.operands[1].reg > 7, BAD_HIREG);
      constraint (inst.operands[0].reg > 14,
		  _("MVE vector register in the range [Q0..Q7] expected"));
      inst.instruction |= (load && type == NT_unsigned) << 28;
      inst.instruction |= (size == 16) << 19;
      inst.instruction |= neon_logbits (elsize) << 7;
    }
  else
    {
      if (inst.operands[1].reg == REG_PC)
	as_tsktsk (MVE_BAD_PC);
      else if (inst.operands[1].reg == REG_SP && inst.operands[1].writeback)
	as_tsktsk (MVE_BAD_SP);
      inst.instruction |= 1 << 12;
      inst.instruction |= neon_logbits (size) << 7;
    }
  inst.instruction |= inst.operands[1].preind << 24;
  inst.instruction |= add << 23;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= inst.operands[1].writeback << 21;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction &= 0xffffff80;
  inst.instruction |= imm >> neon_logbits (size);

}

static void
do_mve_vstr_vldr (void)
{
  unsigned size;
  int load = 0;

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  switch (inst.instruction)
    {
    default:
      gas_assert (0);
      break;
    case M_MNEM_vldrb:
      load = 1;
      /* fall through.  */
    case M_MNEM_vstrb:
      size = 8;
      break;
    case M_MNEM_vldrh:
      load = 1;
      /* fall through.  */
    case M_MNEM_vstrh:
      size = 16;
      break;
    case M_MNEM_vldrw:
      load = 1;
      /* fall through.  */
    case M_MNEM_vstrw:
      size = 32;
      break;
    case M_MNEM_vldrd:
      load = 1;
      /* fall through.  */
    case M_MNEM_vstrd:
      size = 64;
      break;
    }
  unsigned elsize = inst.vectype.el[0].size;

  if (inst.operands[1].isquad)
    {
      /* We are dealing with [Q, imm]{!} cases.  */
      do_mve_vstr_vldr_QI (size, elsize, load);
    }
  else
    {
      if (inst.operands[1].immisreg == 2)
	{
	  /* We are dealing with [R, Q, {UXTW #os}] cases.  */
	  do_mve_vstr_vldr_RQ (size, elsize, load);
	}
      else if (!inst.operands[1].immisreg)
	{
	  /* We are dealing with [R, Imm]{!}/[R], Imm cases.  */
	  do_mve_vstr_vldr_RI (size, elsize, load);
	}
      else
	constraint (1, BAD_ADDR_MODE);
    }

  inst.is_neon = 1;
}

static void
do_mve_vst_vld (void)
{
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    return;

  constraint (!inst.operands[1].preind || inst.relocs[0].exp.X_add_symbol != 0
	      || inst.relocs[0].exp.X_add_number != 0
	      || inst.operands[1].immisreg != 0,
	      BAD_ADDR_MODE);
  constraint (inst.vectype.el[0].size > 32, BAD_EL_TYPE);
  if (inst.operands[1].reg == REG_PC)
    as_tsktsk (MVE_BAD_PC);
  else if (inst.operands[1].reg == REG_SP && inst.operands[1].writeback)
    as_tsktsk (MVE_BAD_SP);


  /* These instructions are one of the "exceptions" mentioned in
     handle_pred_state.  They are MVE instructions that are not VPT compatible
     and do not accept a VPT code, thus appending such a code is a syntax
     error.  */
  if (inst.cond > COND_ALWAYS)
    first_error (BAD_SYNTAX);
  /* If we append a scalar condition code we can set this to
     MVE_OUTSIDE_PRED_INSN as it will also lead to a syntax error.  */
  else if (inst.cond < COND_ALWAYS)
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;
  else
    inst.pred_insn_type = MVE_UNPREDICABLE_INSN;

  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= inst.operands[1].writeback << 21;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= neon_logbits (inst.vectype.el[0].size) << 7;
  inst.is_neon = 1;
}

static void
do_mve_vaddlv (void)
{
  enum neon_shape rs = neon_select_shape (NS_RRQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_S32 | N_U32 | N_KEY);

  if (et.type == NT_invtype)
    first_error (BAD_EL_TYPE);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  constraint (inst.operands[1].reg > 14, MVE_BAD_QREG);

  inst.instruction |= (et.type == NT_unsigned) << 28;
  inst.instruction |= inst.operands[1].reg << 19;
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[2].reg;
  inst.is_neon = 1;
}

static void
do_neon_dyadic_if_su (void)
{
  enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_QQR, NS_NULL);
  struct neon_type_el et = neon_check_type (3, rs, N_EQK , N_EQK,
					    N_SUF_32 | N_KEY);

  constraint ((inst.instruction == ((unsigned) N_MNEM_vmax)
	       || inst.instruction == ((unsigned) N_MNEM_vmin))
	      && et.type == NT_float
	      && !ARM_CPU_HAS_FEATURE (cpu_variant,fpu_neon_ext_v1), BAD_FPU);

  if (!check_simd_pred_availability (et.type == NT_float,
				     NEON_CHECK_ARCH | NEON_CHECK_CC))
    return;

  neon_dyadic_misc (NT_unsigned, N_SUF_32, 0);
}

static void
do_neon_addsub_if_i (void)
{
  if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)
      && try_vfp_nsyn (3, do_vfp_nsyn_add_sub) == SUCCESS)
    return;

  enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_QQR, NS_NULL);
  struct neon_type_el et = neon_check_type (3, rs, N_EQK,
					    N_EQK, N_IF_32 | N_I64 | N_KEY);

  constraint (rs == NS_QQR && et.size == 64, BAD_FPU);
  /* If we are parsing Q registers and the element types match MVE, which NEON
     also supports, then we must check whether this is an instruction that can
     be used by both MVE/NEON.  This distinction can be made based on whether
     they are predicated or not.  */
  if ((rs == NS_QQQ || rs == NS_QQR) && et.size != 64)
    {
      if (!check_simd_pred_availability (et.type == NT_float,
					 NEON_CHECK_ARCH | NEON_CHECK_CC))
	return;
    }
  else
    {
      /* If they are either in a D register or are using an unsupported.  */
      if (rs != NS_QQR
	  && vfp_or_neon_is_neon (NEON_CHECK_CC | NEON_CHECK_ARCH) == FAIL)
	return;
    }

  /* The "untyped" case can't happen. Do this to stop the "U" bit being
     affected if we specify unsigned args.  */
  neon_dyadic_misc (NT_untyped, N_IF_32 | N_I64, 0);
}

/* Swaps operands 1 and 2. If operand 1 (optional arg) was omitted, we want the
   result to be:
     V<op> A,B     (A is operand 0, B is operand 2)
   to mean:
     V<op> A,B,A
   not:
     V<op> A,B,B
   so handle that case specially.  */

static void
neon_exchange_operands (void)
{
  if (inst.operands[1].present)
    {
      void *scratch = xmalloc (sizeof (inst.operands[0]));

      /* Swap operands[1] and operands[2].  */
      memcpy (scratch, &inst.operands[1], sizeof (inst.operands[0]));
      inst.operands[1] = inst.operands[2];
      memcpy (&inst.operands[2], scratch, sizeof (inst.operands[0]));
      free (scratch);
    }
  else
    {
      inst.operands[1] = inst.operands[2];
      inst.operands[2] = inst.operands[0];
    }
}

static void
neon_compare (unsigned regtypes, unsigned immtypes, int invert)
{
  if (inst.operands[2].isreg)
    {
      if (invert)
	neon_exchange_operands ();
      neon_dyadic_misc (NT_unsigned, regtypes, N_SIZ);
    }
  else
    {
      enum neon_shape rs = neon_select_shape (NS_DDI, NS_QQI, NS_NULL);
      struct neon_type_el et = neon_check_type (2, rs,
	N_EQK | N_SIZ, immtypes | N_KEY);

      NEON_ENCODE (IMMED, inst);
      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= LOW4 (inst.operands[1].reg);
      inst.instruction |= HI1 (inst.operands[1].reg) << 5;
      inst.instruction |= neon_quad (rs) << 6;
      inst.instruction |= (et.type == NT_float) << 10;
      inst.instruction |= neon_logbits (et.size) << 18;

      neon_dp_fixup (&inst);
    }
}

static void
do_neon_cmp (void)
{
  neon_compare (N_SUF_32, N_S_32 | N_F_16_32, false);
}

static void
do_neon_cmp_inv (void)
{
  neon_compare (N_SUF_32, N_S_32 | N_F_16_32, true);
}

static void
do_neon_ceq (void)
{
  neon_compare (N_IF_32, N_IF_32, false);
}

/* For multiply instructions, we have the possibility of 16-bit or 32-bit
   scalars, which are encoded in 5 bits, M : Rm.
   For 16-bit scalars, the register is encoded in Rm[2:0] and the index in
   M:Rm[3], and for 32-bit scalars, the register is encoded in Rm[3:0] and the
   index in M.

   Dot Product instructions are similar to multiply instructions except elsize
   should always be 32.

   This function translates SCALAR, which is GAS's internal encoding of indexed
   scalar register, to raw encoding.  There is also register and index range
   check based on ELSIZE.  */

static unsigned
neon_scalar_for_mul (unsigned scalar, unsigned elsize)
{
  unsigned regno = NEON_SCALAR_REG (scalar);
  unsigned elno = NEON_SCALAR_INDEX (scalar);

  switch (elsize)
    {
    case 16:
      if (regno > 7 || elno > 3)
	goto bad_scalar;
      return regno | (elno << 3);

    case 32:
      if (regno > 15 || elno > 1)
	goto bad_scalar;
      return regno | (elno << 4);

    default:
    bad_scalar:
      first_error (_("scalar out of range for multiply instruction"));
    }

  return 0;
}

/* Encode multiply / multiply-accumulate scalar instructions.  */

static void
neon_mul_mac (struct neon_type_el et, int ubit)
{
  unsigned scalar;

  /* Give a more helpful error message if we have an invalid type.  */
  if (et.type == NT_invtype)
    return;

  scalar = neon_scalar_for_mul (inst.operands[2].reg, et.size);
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= LOW4 (scalar);
  inst.instruction |= HI1 (scalar) << 5;
  inst.instruction |= (et.type == NT_float) << 8;
  inst.instruction |= neon_logbits (et.size) << 20;
  inst.instruction |= (ubit != 0) << 24;

  neon_dp_fixup (&inst);
}

static void
do_neon_mac_maybe_scalar (void)
{
  if (try_vfp_nsyn (3, do_vfp_nsyn_mla_mls) == SUCCESS)
    return;

  if (!check_simd_pred_availability (false, NEON_CHECK_CC | NEON_CHECK_ARCH))
    return;

  if (inst.operands[2].isscalar)
    {
      constraint (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext), BAD_FPU);
      enum neon_shape rs = neon_select_shape (NS_DDS, NS_QQS, NS_NULL);
      struct neon_type_el et = neon_check_type (3, rs,
	N_EQK, N_EQK, N_I16 | N_I32 | N_F_16_32 | N_KEY);
      NEON_ENCODE (SCALAR, inst);
      neon_mul_mac (et, neon_quad (rs));
    }
  else if (!inst.operands[2].isvec)
    {
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext), BAD_FPU);

      enum neon_shape rs = neon_select_shape (NS_QQR, NS_NULL);
      neon_check_type (3, rs, N_EQK, N_EQK, N_SU_MVE | N_KEY);

      neon_dyadic_misc (NT_unsigned, N_SU_MVE, 0);
    }
  else
    {
      constraint (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext), BAD_FPU);
      /* The "untyped" case can't happen.  Do this to stop the "U" bit being
	 affected if we specify unsigned args.  */
      neon_dyadic_misc (NT_untyped, N_IF_32, 0);
    }
}

static void
do_bfloat_vfma (void)
{
  constraint (!mark_feature_used (&fpu_neon_ext_armv8), _(BAD_FPU));
  constraint (!mark_feature_used (&arm_ext_bf16), _(BAD_BF16));
  enum neon_shape rs;
  int t_bit = 0;

  if (inst.instruction != B_MNEM_vfmab)
  {
      t_bit = 1;
      inst.instruction = B_MNEM_vfmat;
  }

  if (inst.operands[2].isscalar)
    {
      rs = neon_select_shape (NS_QQS, NS_NULL);
      neon_check_type (3, rs, N_EQK, N_EQK, N_BF16 | N_KEY);

      inst.instruction |= (1 << 25);
      int idx = inst.operands[2].reg & 0xf;
      constraint (!(idx < 4), _("index must be in the range 0 to 3"));
      inst.operands[2].reg >>= 4;
      constraint (!(inst.operands[2].reg < 8),
		  _("indexed register must be less than 8"));
      neon_three_args (t_bit);
      inst.instruction |= ((idx & 1) << 3);
      inst.instruction |= ((idx & 2) << 4);
    }
  else
    {
      rs = neon_select_shape (NS_QQQ, NS_NULL);
      neon_check_type (3, rs, N_EQK, N_EQK, N_BF16 | N_KEY);
      neon_three_args (t_bit);
    }

}

static void
do_neon_fmac (void)
{
  if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_fma)
      && try_vfp_nsyn (3, do_vfp_nsyn_fma_fms) == SUCCESS)
    return;

  if (!check_simd_pred_availability (true, NEON_CHECK_CC | NEON_CHECK_ARCH))
    return;

  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext))
    {
      enum neon_shape rs = neon_select_shape (NS_QQQ, NS_QQR, NS_NULL);
      struct neon_type_el et = neon_check_type (3, rs, N_F_MVE | N_KEY, N_EQK,
						N_EQK);

      if (rs == NS_QQR)
	{

	  if (inst.operands[2].reg == REG_SP)
	    as_tsktsk (MVE_BAD_SP);
	  else if (inst.operands[2].reg == REG_PC)
	    as_tsktsk (MVE_BAD_PC);

	  inst.instruction = 0xee310e40;
	  inst.instruction |= (et.size == 16) << 28;
	  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
	  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
	  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
	  inst.instruction |= HI1 (inst.operands[1].reg) << 6;
	  inst.instruction |= inst.operands[2].reg;
	  inst.is_neon = 1;
	  return;
	}
    }
  else
    {
      constraint (!inst.operands[2].isvec, BAD_FPU);
    }

  neon_dyadic_misc (NT_untyped, N_IF_32, 0);
}

static void
do_mve_vfma (void)
{
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_bf16) &&
      inst.cond == COND_ALWAYS)
    {
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext), BAD_FPU);
      inst.instruction = N_MNEM_vfma;
      inst.pred_insn_type = INSIDE_VPT_INSN;
      inst.cond = 0xf;
      return do_neon_fmac();
    }
  else
    {
      do_bfloat_vfma();
    }
}

static void
do_neon_tst (void)
{
  enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
  struct neon_type_el et = neon_check_type (3, rs,
    N_EQK, N_EQK, N_8 | N_16 | N_32 | N_KEY);
  neon_three_same (neon_quad (rs), 0, et.size);
}

/* VMUL with 3 registers allows the P8 type. The scalar version supports the
   same types as the MAC equivalents. The polynomial type for this instruction
   is encoded the same as the integer type.  */

static void
do_neon_mul (void)
{
  if (try_vfp_nsyn (3, do_vfp_nsyn_mul) == SUCCESS)
    return;

  if (!check_simd_pred_availability (false, NEON_CHECK_CC | NEON_CHECK_ARCH))
    return;

  if (inst.operands[2].isscalar)
    {
      constraint (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext), BAD_FPU);
      do_neon_mac_maybe_scalar ();
    }
  else
    {
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	{
	  enum neon_shape rs = neon_select_shape (NS_QQR, NS_QQQ, NS_NULL);
	  struct neon_type_el et
	    = neon_check_type (3, rs, N_EQK, N_EQK, N_I_MVE | N_F_MVE | N_KEY);
	  if (et.type == NT_float)
	    constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext),
			BAD_FPU);

	  neon_dyadic_misc (NT_float, N_I_MVE | N_F_MVE, 0);
	}
      else
	{
	  constraint (!inst.operands[2].isvec, BAD_FPU);
	  neon_dyadic_misc (NT_poly,
			    N_I8 | N_I16 | N_I32 | N_F16 | N_F32 | N_P8, 0);
	}
    }
}

static void
do_neon_qdmulh (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
   return;

  if (inst.operands[2].isscalar)
    {
      constraint (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext), BAD_FPU);
      enum neon_shape rs = neon_select_shape (NS_DDS, NS_QQS, NS_NULL);
      struct neon_type_el et = neon_check_type (3, rs,
	N_EQK, N_EQK, N_S16 | N_S32 | N_KEY);
      NEON_ENCODE (SCALAR, inst);
      neon_mul_mac (et, neon_quad (rs));
    }
  else
    {
      enum neon_shape rs;
      struct neon_type_el et;
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	{
	  rs = neon_select_shape (NS_QQR, NS_QQQ, NS_NULL);
	  et = neon_check_type (3, rs,
	    N_EQK, N_EQK, N_S8 | N_S16 | N_S32 | N_KEY);
	}
      else
	{
	  rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
	  et = neon_check_type (3, rs,
	    N_EQK, N_EQK, N_S16 | N_S32 | N_KEY);
	}

      NEON_ENCODE (INTEGER, inst);
      if (rs == NS_QQR)
	mve_encode_qqr (et.size, 0, 0);
      else
	/* The U bit (rounding) comes from bit mask.  */
	neon_three_same (neon_quad (rs), 0, et.size);
    }
}

static void
do_mve_vaddv (void)
{
  enum neon_shape rs = neon_select_shape (NS_RQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (2, rs, N_EQK,  N_SU_32 | N_KEY);

  if (et.type == NT_invtype)
    first_error (BAD_EL_TYPE);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  constraint (inst.operands[1].reg > 14, MVE_BAD_QREG);

  mve_encode_rq (et.type == NT_unsigned, et.size);
}

static void
do_mve_vhcadd (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQI, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_S8 | N_S16 | N_S32 | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  unsigned rot = inst.relocs[0].exp.X_add_number;
  constraint (rot != 90 && rot != 270, _("immediate out of range"));

  if (et.size == 32 && inst.operands[0].reg == inst.operands[2].reg)
    as_tsktsk (_("Warning: 32-bit element size and same first and third "
		 "operand makes instruction UNPREDICTABLE"));

  mve_encode_qqq (0, et.size);
  inst.instruction |= (rot == 270) << 12;
  inst.is_neon = 1;
}

static void
do_mve_vqdmull (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQ, NS_QQR, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_S16 | N_S32 | N_KEY);

  if (et.size == 32
      && (inst.operands[0].reg == inst.operands[1].reg
	  || (rs == NS_QQQ && inst.operands[0].reg == inst.operands[2].reg)))
    as_tsktsk (BAD_MVE_SRCDEST);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  if (rs == NS_QQQ)
    {
      mve_encode_qqq (et.size == 32, 64);
      inst.instruction |= 1;
    }
  else
    {
      mve_encode_qqr (64, et.size == 32, 0);
      inst.instruction |= 0x3 << 5;
    }
}

static void
do_mve_vadc (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_KEY | N_I32, N_EQK, N_EQK);

  if (et.type == NT_invtype)
    first_error (BAD_EL_TYPE);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_qqq (0, 64);
}

static void
do_mve_vbrsr (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQR, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_8 | N_16 | N_32 | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_qqr (et.size, 0, 0);
}

static void
do_mve_vsbc (void)
{
  neon_check_type (3, NS_QQQ, N_EQK, N_EQK, N_I32 | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_qqq (1, 64);
}

static void
do_mve_vmulh (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_SU_MVE | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_qqq (et.type == NT_unsigned, et.size);
}

static void
do_mve_vqdmlah (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQR, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_S_32 | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_qqr (et.size, et.type == NT_unsigned, 0);
}

static void
do_mve_vqdmladh (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (3, rs, N_EQK, N_EQK, N_S8 | N_S16 | N_S32 | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_qqq (0, et.size);
}


static void
do_mve_vmull (void)
{

  enum neon_shape rs = neon_select_shape (NS_HHH, NS_FFF, NS_DDD, NS_DDS,
					  NS_QQS, NS_QQQ, NS_QQR, NS_NULL);
  if (inst.cond == COND_ALWAYS
      && ((unsigned)inst.instruction) == M_MNEM_vmullt)
    {

      if (rs == NS_QQQ)
	{
	  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	    goto neon_vmul;
	}
      else
	goto neon_vmul;
    }

  constraint (rs != NS_QQQ, BAD_FPU);
  struct neon_type_el et = neon_check_type (3, rs, N_EQK , N_EQK,
					    N_SU_32 | N_P8 | N_P16 | N_KEY);

  /* We are dealing with MVE's vmullt.  */
  if (et.size == 32
      && (inst.operands[0].reg == inst.operands[1].reg
	  || inst.operands[0].reg == inst.operands[2].reg))
    as_tsktsk (BAD_MVE_SRCDEST);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  if (et.type == NT_poly)
    mve_encode_qqq (neon_logbits (et.size), 64);
  else
    mve_encode_qqq (et.type == NT_unsigned, et.size);

  return;

 neon_vmul:
  inst.instruction = N_MNEM_vmul;
  inst.cond = 0xb;
  if (thumb_mode)
    inst.pred_insn_type = INSIDE_IT_INSN;
  do_neon_mul ();
}

static void
do_mve_vabav (void)
{
  enum neon_shape rs = neon_select_shape (NS_RQQ, NS_NULL);

  if (rs == NS_NULL)
    return;

  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    return;

  struct neon_type_el et = neon_check_type (2, NS_NULL, N_EQK, N_KEY | N_S8
					    | N_S16 | N_S32 | N_U8 | N_U16
					    | N_U32);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_rqq (et.type == NT_unsigned, et.size);
}

static void
do_mve_vmladav (void)
{
  enum neon_shape rs = neon_select_shape (NS_RQQ, NS_NULL);
  struct neon_type_el et = neon_check_type (3, rs,
					    N_EQK, N_EQK, N_SU_MVE | N_KEY);

  if (et.type == NT_unsigned
      && (inst.instruction == M_MNEM_vmladavx
	  || inst.instruction == M_MNEM_vmladavax
	  || inst.instruction == M_MNEM_vmlsdav
	  || inst.instruction == M_MNEM_vmlsdava
	  || inst.instruction == M_MNEM_vmlsdavx
	  || inst.instruction == M_MNEM_vmlsdavax))
    first_error (BAD_SIMD_TYPE);

  constraint (inst.operands[2].reg > 14,
	      _("MVE vector register in the range [Q0..Q7] expected"));

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  if (inst.instruction == M_MNEM_vmlsdav
      || inst.instruction == M_MNEM_vmlsdava
      || inst.instruction == M_MNEM_vmlsdavx
      || inst.instruction == M_MNEM_vmlsdavax)
    inst.instruction |= (et.size == 8) << 28;
  else
    inst.instruction |= (et.size == 8) << 8;

  mve_encode_rqq (et.type == NT_unsigned, 64);
  inst.instruction |= (et.size == 32) << 16;
}

static void
do_mve_vmlaldav (void)
{
  enum neon_shape rs = neon_select_shape (NS_RRQQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (4, rs, N_EQK, N_EQK, N_EQK,
		       N_S16 | N_S32 | N_U16 | N_U32 | N_KEY);

  if (et.type == NT_unsigned
      && (inst.instruction == M_MNEM_vmlsldav
	  || inst.instruction == M_MNEM_vmlsldava
	  || inst.instruction == M_MNEM_vmlsldavx
	  || inst.instruction == M_MNEM_vmlsldavax))
    first_error (BAD_SIMD_TYPE);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_rrqq (et.type == NT_unsigned, et.size);
}

static void
do_mve_vrmlaldavh (void)
{
  struct neon_type_el et;
  if (inst.instruction == M_MNEM_vrmlsldavh
     || inst.instruction == M_MNEM_vrmlsldavha
     || inst.instruction == M_MNEM_vrmlsldavhx
     || inst.instruction == M_MNEM_vrmlsldavhax)
    {
      et = neon_check_type (4, NS_RRQQ, N_EQK, N_EQK, N_EQK, N_S32 | N_KEY);
      if (inst.operands[1].reg == REG_SP)
	as_tsktsk (MVE_BAD_SP);
    }
  else
    {
      if (inst.instruction == M_MNEM_vrmlaldavhx
	  || inst.instruction == M_MNEM_vrmlaldavhax)
	et = neon_check_type (4, NS_RRQQ, N_EQK, N_EQK, N_EQK, N_S32 | N_KEY);
      else
	et = neon_check_type (4, NS_RRQQ, N_EQK, N_EQK, N_EQK,
			      N_U32 | N_S32 | N_KEY);
      /* vrmlaldavh's encoding with SP as the second, odd, GPR operand may alias
	 with vmax/min instructions, making the use of SP in assembly really
	 nonsensical, so instead of issuing a warning like we do for other uses
	 of SP for the odd register operand we error out.  */
      constraint (inst.operands[1].reg == REG_SP, BAD_SP);
    }

  /* Make sure we still check the second operand is an odd one and that PC is
     disallowed.  This because we are parsing for any GPR operand, to be able
     to distinguish between giving a warning or an error for SP as described
     above.  */
  constraint ((inst.operands[1].reg % 2) != 1, BAD_EVEN);
  constraint (inst.operands[1].reg == REG_PC, BAD_PC);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  mve_encode_rrqq (et.type == NT_unsigned, 0);
}


static void
do_mve_vmaxnmv (void)
{
  enum neon_shape rs = neon_select_shape (NS_RQ, NS_NULL);
  struct neon_type_el et
    = neon_check_type (2, rs, N_EQK, N_F_MVE | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  if (inst.operands[0].reg == REG_SP)
    as_tsktsk (MVE_BAD_SP);
  else if (inst.operands[0].reg == REG_PC)
    as_tsktsk (MVE_BAD_PC);

  mve_encode_rq (et.size == 16, 64);
}

static void
do_mve_vmaxv (void)
{
  enum neon_shape rs = neon_select_shape (NS_RQ, NS_NULL);
  struct neon_type_el et;

  if (inst.instruction == M_MNEM_vmaxv || inst.instruction == M_MNEM_vminv)
    et = neon_check_type (2, rs, N_EQK, N_SU_MVE | N_KEY);
  else
    et = neon_check_type (2, rs, N_EQK, N_S8 | N_S16 | N_S32 | N_KEY);

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  if (inst.operands[0].reg == REG_SP)
    as_tsktsk (MVE_BAD_SP);
  else if (inst.operands[0].reg == REG_PC)
    as_tsktsk (MVE_BAD_PC);

  mve_encode_rq (et.type == NT_unsigned, et.size);
}


static void
do_neon_qrdmlah (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
   return;
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      /* Check we're on the correct architecture.  */
      if (!mark_feature_used (&fpu_neon_ext_armv8))
	inst.error
	  = _("instruction form not available on this architecture.");
      else if (!mark_feature_used (&fpu_neon_ext_v8_1))
	{
	  as_warn (_("this instruction implies use of ARMv8.1 AdvSIMD."));
	  record_feature_use (&fpu_neon_ext_v8_1);
	}
	if (inst.operands[2].isscalar)
	  {
	    enum neon_shape rs = neon_select_shape (NS_DDS, NS_QQS, NS_NULL);
	    struct neon_type_el et = neon_check_type (3, rs,
	      N_EQK, N_EQK, N_S16 | N_S32 | N_KEY);
	    NEON_ENCODE (SCALAR, inst);
	    neon_mul_mac (et, neon_quad (rs));
	  }
	else
	  {
	    enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
	    struct neon_type_el et = neon_check_type (3, rs,
	      N_EQK, N_EQK, N_S16 | N_S32 | N_KEY);
	    NEON_ENCODE (INTEGER, inst);
	    /* The U bit (rounding) comes from bit mask.  */
	    neon_three_same (neon_quad (rs), 0, et.size);
	  }
    }
  else
    {
      enum neon_shape rs = neon_select_shape (NS_QQR, NS_NULL);
      struct neon_type_el et
	= neon_check_type (3, rs, N_EQK, N_EQK, N_S_32 | N_KEY);

      NEON_ENCODE (INTEGER, inst);
      mve_encode_qqr (et.size, et.type == NT_unsigned, 0);
    }
}

static void
do_neon_fcmp_absolute (void)
{
  enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
  struct neon_type_el et = neon_check_type (3, rs, N_EQK, N_EQK,
					    N_F_16_32 | N_KEY);
  /* Size field comes from bit mask.  */
  neon_three_same (neon_quad (rs), 1, et.size == 16 ? (int) et.size : -1);
}

static void
do_neon_fcmp_absolute_inv (void)
{
  neon_exchange_operands ();
  do_neon_fcmp_absolute ();
}

static void
do_neon_step (void)
{
  enum neon_shape rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
  struct neon_type_el et = neon_check_type (3, rs, N_EQK, N_EQK,
					    N_F_16_32 | N_KEY);
  neon_three_same (neon_quad (rs), 0, et.size == 16 ? (int) et.size : -1);
}

static void
do_neon_abs_neg (void)
{
  enum neon_shape rs;
  struct neon_type_el et;

  if (try_vfp_nsyn (2, do_vfp_nsyn_abs_neg) == SUCCESS)
    return;

  rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);
  et = neon_check_type (2, rs, N_EQK, N_S_32 | N_F_16_32 | N_KEY);

  if (!check_simd_pred_availability (et.type == NT_float,
				     NEON_CHECK_ARCH | NEON_CHECK_CC))
    return;

  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= neon_quad (rs) << 6;
  inst.instruction |= (et.type == NT_float) << 10;
  inst.instruction |= neon_logbits (et.size) << 18;

  neon_dp_fixup (&inst);
}

static void
do_neon_sli (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
    return;

  enum neon_shape rs;
  struct neon_type_el et;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      rs = neon_select_shape (NS_QQI, NS_NULL);
      et = neon_check_type (2, rs, N_EQK, N_8 | N_16 | N_32 | N_KEY);
    }
  else
    {
      rs = neon_select_shape (NS_DDI, NS_QQI, NS_NULL);
      et = neon_check_type (2, rs, N_EQK, N_8 | N_16 | N_32 | N_64 | N_KEY);
    }


  int imm = inst.operands[2].imm;
  constraint (imm < 0 || (unsigned)imm >= et.size,
	      _("immediate out of range for insert"));
  neon_imm_shift (false, 0, neon_quad (rs), et, imm);
}

static void
do_neon_sri (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
    return;

  enum neon_shape rs;
  struct neon_type_el et;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      rs = neon_select_shape (NS_QQI, NS_NULL);
      et = neon_check_type (2, rs, N_EQK, N_8 | N_16 | N_32 | N_KEY);
    }
  else
    {
      rs = neon_select_shape (NS_DDI, NS_QQI, NS_NULL);
      et = neon_check_type (2, rs, N_EQK, N_8 | N_16 | N_32 | N_64 | N_KEY);
    }

  int imm = inst.operands[2].imm;
  constraint (imm < 1 || (unsigned)imm > et.size,
	      _("immediate out of range for insert"));
  neon_imm_shift (false, 0, neon_quad (rs), et, et.size - imm);
}

static void
do_neon_qshlu_imm (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
    return;

  enum neon_shape rs;
  struct neon_type_el et;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      rs = neon_select_shape (NS_QQI, NS_NULL);
      et = neon_check_type (2, rs, N_EQK, N_S8 | N_S16 | N_S32 | N_KEY);
    }
  else
    {
      rs = neon_select_shape (NS_DDI, NS_QQI, NS_NULL);
      et = neon_check_type (2, rs, N_EQK | N_UNS,
			    N_S8 | N_S16 | N_S32 | N_S64 | N_KEY);
    }

  int imm = inst.operands[2].imm;
  constraint (imm < 0 || (unsigned)imm >= et.size,
	      _("immediate out of range for shift"));
  /* Only encodes the 'U present' variant of the instruction.
     In this case, signed types have OP (bit 8) set to 0.
     Unsigned types have OP set to 1.  */
  inst.instruction |= (et.type == NT_unsigned) << 8;
  /* The rest of the bits are the same as other immediate shifts.  */
  neon_imm_shift (false, 0, neon_quad (rs), et, imm);
}

static void
do_neon_qmovn (void)
{
  struct neon_type_el et = neon_check_type (2, NS_DQ,
    N_EQK | N_HLF, N_SU_16_64 | N_KEY);
  /* Saturating move where operands can be signed or unsigned, and the
     destination has the same signedness.  */
  NEON_ENCODE (INTEGER, inst);
  if (et.type == NT_unsigned)
    inst.instruction |= 0xc0;
  else
    inst.instruction |= 0x80;
  neon_two_same (0, 1, et.size / 2);
}

static void
do_neon_qmovun (void)
{
  struct neon_type_el et = neon_check_type (2, NS_DQ,
    N_EQK | N_HLF | N_UNS, N_S16 | N_S32 | N_S64 | N_KEY);
  /* Saturating move with unsigned results. Operands must be signed.  */
  NEON_ENCODE (INTEGER, inst);
  neon_two_same (0, 1, et.size / 2);
}

static void
do_neon_rshift_sat_narrow (void)
{
  /* FIXME: Types for narrowing. If operands are signed, results can be signed
     or unsigned. If operands are unsigned, results must also be unsigned.  */
  struct neon_type_el et = neon_check_type (2, NS_DQI,
    N_EQK | N_HLF, N_SU_16_64 | N_KEY);
  int imm = inst.operands[2].imm;
  /* This gets the bounds check, size encoding and immediate bits calculation
     right.  */
  et.size /= 2;

  /* VQ{R}SHRN.I<size> <Dd>, <Qm>, #0 is a synonym for
     VQMOVN.I<size> <Dd>, <Qm>.  */
  if (imm == 0)
    {
      inst.operands[2].present = 0;
      inst.instruction = N_MNEM_vqmovn;
      do_neon_qmovn ();
      return;
    }

  constraint (imm < 1 || (unsigned)imm > et.size,
	      _("immediate out of range"));
  neon_imm_shift (true, et.type == NT_unsigned, 0, et, et.size - imm);
}

static void
do_neon_rshift_sat_narrow_u (void)
{
  /* FIXME: Types for narrowing. If operands are signed, results can be signed
     or unsigned. If operands are unsigned, results must also be unsigned.  */
  struct neon_type_el et = neon_check_type (2, NS_DQI,
    N_EQK | N_HLF | N_UNS, N_S16 | N_S32 | N_S64 | N_KEY);
  int imm = inst.operands[2].imm;
  /* This gets the bounds check, size encoding and immediate bits calculation
     right.  */
  et.size /= 2;

  /* VQSHRUN.I<size> <Dd>, <Qm>, #0 is a synonym for
     VQMOVUN.I<size> <Dd>, <Qm>.  */
  if (imm == 0)
    {
      inst.operands[2].present = 0;
      inst.instruction = N_MNEM_vqmovun;
      do_neon_qmovun ();
      return;
    }

  constraint (imm < 1 || (unsigned)imm > et.size,
	      _("immediate out of range"));
  /* FIXME: The manual is kind of unclear about what value U should have in
     VQ{R}SHRUN instructions, but U=0, op=0 definitely encodes VRSHR, so it
     must be 1.  */
  neon_imm_shift (true, 1, 0, et, et.size - imm);
}

static void
do_neon_movn (void)
{
  struct neon_type_el et = neon_check_type (2, NS_DQ,
    N_EQK | N_HLF, N_I16 | N_I32 | N_I64 | N_KEY);
  NEON_ENCODE (INTEGER, inst);
  neon_two_same (0, 1, et.size / 2);
}

static void
do_neon_rshift_narrow (void)
{
  struct neon_type_el et = neon_check_type (2, NS_DQI,
    N_EQK | N_HLF, N_I16 | N_I32 | N_I64 | N_KEY);
  int imm = inst.operands[2].imm;
  /* This gets the bounds check, size encoding and immediate bits calculation
     right.  */
  et.size /= 2;

  /* If immediate is zero then we are a pseudo-instruction for
     VMOVN.I<size> <Dd>, <Qm>  */
  if (imm == 0)
    {
      inst.operands[2].present = 0;
      inst.instruction = N_MNEM_vmovn;
      do_neon_movn ();
      return;
    }

  constraint (imm < 1 || (unsigned)imm > et.size,
	      _("immediate out of range for narrowing operation"));
  neon_imm_shift (false, 0, 0, et, et.size - imm);
}

static void
do_neon_shll (void)
{
  /* FIXME: Type checking when lengthening.  */
  struct neon_type_el et = neon_check_type (2, NS_QDI,
    N_EQK | N_DBL, N_I8 | N_I16 | N_I32 | N_KEY);
  unsigned imm = inst.operands[2].imm;

  if (imm == et.size)
    {
      /* Maximum shift variant.  */
      NEON_ENCODE (INTEGER, inst);
      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= LOW4 (inst.operands[1].reg);
      inst.instruction |= HI1 (inst.operands[1].reg) << 5;
      inst.instruction |= neon_logbits (et.size) << 18;

      neon_dp_fixup (&inst);
    }
  else
    {
      /* A more-specific type check for non-max versions.  */
      et = neon_check_type (2, NS_QDI,
	N_EQK | N_DBL, N_SU_32 | N_KEY);
      NEON_ENCODE (IMMED, inst);
      neon_imm_shift (true, et.type == NT_unsigned, 0, et, imm);
    }
}

/* Check the various types for the VCVT instruction, and return which version
   the current instruction is.  */

#define CVT_FLAVOUR_VAR							      \
  CVT_VAR (s32_f32, N_S32, N_F32, whole_reg,   "ftosls", "ftosis", "ftosizs") \
  CVT_VAR (u32_f32, N_U32, N_F32, whole_reg,   "ftouls", "ftouis", "ftouizs") \
  CVT_VAR (f32_s32, N_F32, N_S32, whole_reg,   "fsltos", "fsitos", NULL)      \
  CVT_VAR (f32_u32, N_F32, N_U32, whole_reg,   "fultos", "fuitos", NULL)      \
  /* Half-precision conversions.  */					      \
  CVT_VAR (s16_f16, N_S16, N_F16 | N_KEY, whole_reg, NULL, NULL, NULL)	      \
  CVT_VAR (u16_f16, N_U16, N_F16 | N_KEY, whole_reg, NULL, NULL, NULL)	      \
  CVT_VAR (f16_s16, N_F16 | N_KEY, N_S16, whole_reg, NULL, NULL, NULL)	      \
  CVT_VAR (f16_u16, N_F16 | N_KEY, N_U16, whole_reg, NULL, NULL, NULL)	      \
  CVT_VAR (f32_f16, N_F32, N_F16, whole_reg,   NULL,     NULL,     NULL)      \
  CVT_VAR (f16_f32, N_F16, N_F32, whole_reg,   NULL,     NULL,     NULL)      \
  /* New VCVT instructions introduced by ARMv8.2 fp16 extension.	      \
     Compared with single/double precision variants, only the co-processor    \
     field is different, so the encoding flow is reused here.  */	      \
  CVT_VAR (f16_s32, N_F16 | N_KEY, N_S32, N_VFP, "fsltos", "fsitos", NULL)    \
  CVT_VAR (f16_u32, N_F16 | N_KEY, N_U32, N_VFP, "fultos", "fuitos", NULL)    \
  CVT_VAR (u32_f16, N_U32, N_F16 | N_KEY, N_VFP, "ftouls", "ftouis", "ftouizs")\
  CVT_VAR (s32_f16, N_S32, N_F16 | N_KEY, N_VFP, "ftosls", "ftosis", "ftosizs")\
  CVT_VAR (bf16_f32, N_BF16, N_F32, whole_reg,   NULL, NULL, NULL)	      \
  /* VFP instructions.  */						      \
  CVT_VAR (f32_f64, N_F32, N_F64, N_VFP,       NULL,     "fcvtsd", NULL)      \
  CVT_VAR (f64_f32, N_F64, N_F32, N_VFP,       NULL,     "fcvtds", NULL)      \
  CVT_VAR (s32_f64, N_S32, N_F64 | key, N_VFP, "ftosld", "ftosid", "ftosizd") \
  CVT_VAR (u32_f64, N_U32, N_F64 | key, N_VFP, "ftould", "ftouid", "ftouizd") \
  CVT_VAR (f64_s32, N_F64 | key, N_S32, N_VFP, "fsltod", "fsitod", NULL)      \
  CVT_VAR (f64_u32, N_F64 | key, N_U32, N_VFP, "fultod", "fuitod", NULL)      \
  /* VFP instructions with bitshift.  */				      \
  CVT_VAR (f32_s16, N_F32 | key, N_S16, N_VFP, "fshtos", NULL,     NULL)      \
  CVT_VAR (f32_u16, N_F32 | key, N_U16, N_VFP, "fuhtos", NULL,     NULL)      \
  CVT_VAR (f64_s16, N_F64 | key, N_S16, N_VFP, "fshtod", NULL,     NULL)      \
  CVT_VAR (f64_u16, N_F64 | key, N_U16, N_VFP, "fuhtod", NULL,     NULL)      \
  CVT_VAR (s16_f32, N_S16, N_F32 | key, N_VFP, "ftoshs", NULL,     NULL)      \
  CVT_VAR (u16_f32, N_U16, N_F32 | key, N_VFP, "ftouhs", NULL,     NULL)      \
  CVT_VAR (s16_f64, N_S16, N_F64 | key, N_VFP, "ftoshd", NULL,     NULL)      \
  CVT_VAR (u16_f64, N_U16, N_F64 | key, N_VFP, "ftouhd", NULL,     NULL)

#define CVT_VAR(C, X, Y, R, BSN, CN, ZN) \
  neon_cvt_flavour_##C,

/* The different types of conversions we can do.  */
enum neon_cvt_flavour
{
  CVT_FLAVOUR_VAR
  neon_cvt_flavour_invalid,
  neon_cvt_flavour_first_fp = neon_cvt_flavour_f32_f64
};

#undef CVT_VAR

static enum neon_cvt_flavour
get_neon_cvt_flavour (enum neon_shape rs)
{
#define CVT_VAR(C,X,Y,R,BSN,CN,ZN)			\
  et = neon_check_type (2, rs, (R) | (X), (R) | (Y));	\
  if (et.type != NT_invtype)				\
    {							\
      inst.error = NULL;				\
      return (neon_cvt_flavour_##C);			\
    }

  struct neon_type_el et;
  unsigned whole_reg = (rs == NS_FFI || rs == NS_FD || rs == NS_DF
			|| rs == NS_FF) ? N_VFP : 0;
  /* The instruction versions which take an immediate take one register
     argument, which is extended to the width of the full register. Thus the
     "source" and "destination" registers must have the same width.  Hack that
     here by making the size equal to the key (wider, in this case) operand.  */
  unsigned key = (rs == NS_QQI || rs == NS_DDI || rs == NS_FFI) ? N_KEY : 0;

  CVT_FLAVOUR_VAR;

  return neon_cvt_flavour_invalid;
#undef CVT_VAR
}

enum neon_cvt_mode
{
  neon_cvt_mode_a,
  neon_cvt_mode_n,
  neon_cvt_mode_p,
  neon_cvt_mode_m,
  neon_cvt_mode_z,
  neon_cvt_mode_x,
  neon_cvt_mode_r
};

/* Neon-syntax VFP conversions.  */

static void
do_vfp_nsyn_cvt (enum neon_shape rs, enum neon_cvt_flavour flavour)
{
  const char *opname = 0;

  if (rs == NS_DDI || rs == NS_QQI || rs == NS_FFI
      || rs == NS_FHI || rs == NS_HFI)
    {
      /* Conversions with immediate bitshift.  */
      const char *enc[] =
	{
#define CVT_VAR(C,A,B,R,BSN,CN,ZN) BSN,
	  CVT_FLAVOUR_VAR
	  NULL
#undef CVT_VAR
	};

      if (flavour < (int) ARRAY_SIZE (enc))
	{
	  opname = enc[flavour];
	  constraint (inst.operands[0].reg != inst.operands[1].reg,
		      _("operands 0 and 1 must be the same register"));
	  inst.operands[1] = inst.operands[2];
	  memset (&inst.operands[2], '\0', sizeof (inst.operands[2]));
	}
    }
  else
    {
      /* Conversions without bitshift.  */
      const char *enc[] =
	{
#define CVT_VAR(C,A,B,R,BSN,CN,ZN) CN,
	  CVT_FLAVOUR_VAR
	  NULL
#undef CVT_VAR
	};

      if (flavour < (int) ARRAY_SIZE (enc))
	opname = enc[flavour];
    }

  if (opname)
    do_vfp_nsyn_opcode (opname);

  /* ARMv8.2 fp16 VCVT instruction.  */
  if (flavour == neon_cvt_flavour_s32_f16
      || flavour == neon_cvt_flavour_u32_f16
      || flavour == neon_cvt_flavour_f16_u32
      || flavour == neon_cvt_flavour_f16_s32)
    do_scalar_fp16_v82_encode ();
}

static void
do_vfp_nsyn_cvtz (void)
{
  enum neon_shape rs = neon_select_shape (NS_FH, NS_FF, NS_FD, NS_NULL);
  enum neon_cvt_flavour flavour = get_neon_cvt_flavour (rs);
  const char *enc[] =
    {
#define CVT_VAR(C,A,B,R,BSN,CN,ZN) ZN,
      CVT_FLAVOUR_VAR
      NULL
#undef CVT_VAR
    };

  if (flavour < (int) ARRAY_SIZE (enc) && enc[flavour])
    do_vfp_nsyn_opcode (enc[flavour]);
}

static void
do_vfp_nsyn_cvt_fpv8 (enum neon_cvt_flavour flavour,
		      enum neon_cvt_mode mode)
{
  int sz, op;
  int rm;

  /* Targets like FPv5-SP-D16 don't support FP v8 instructions with
     D register operands.  */
  if (flavour == neon_cvt_flavour_s32_f64
      || flavour == neon_cvt_flavour_u32_f64)
    constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_armv8),
		_(BAD_FPU));

  if (flavour == neon_cvt_flavour_s32_f16
      || flavour == neon_cvt_flavour_u32_f16)
    constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_fp16),
		_(BAD_FP16));

  set_pred_insn_type (OUTSIDE_PRED_INSN);

  switch (flavour)
    {
    case neon_cvt_flavour_s32_f64:
      sz = 1;
      op = 1;
      break;
    case neon_cvt_flavour_s32_f32:
      sz = 0;
      op = 1;
      break;
    case neon_cvt_flavour_s32_f16:
      sz = 0;
      op = 1;
      break;
    case neon_cvt_flavour_u32_f64:
      sz = 1;
      op = 0;
      break;
    case neon_cvt_flavour_u32_f32:
      sz = 0;
      op = 0;
      break;
    case neon_cvt_flavour_u32_f16:
      sz = 0;
      op = 0;
      break;
    default:
      first_error (_("invalid instruction shape"));
      return;
    }

  switch (mode)
    {
    case neon_cvt_mode_a: rm = 0; break;
    case neon_cvt_mode_n: rm = 1; break;
    case neon_cvt_mode_p: rm = 2; break;
    case neon_cvt_mode_m: rm = 3; break;
    default: first_error (_("invalid rounding mode")); return;
    }

  NEON_ENCODE (FPV8, inst);
  encode_arm_vfp_reg (inst.operands[0].reg, VFP_REG_Sd);
  encode_arm_vfp_reg (inst.operands[1].reg, sz == 1 ? VFP_REG_Dm : VFP_REG_Sm);
  inst.instruction |= sz << 8;

  /* ARMv8.2 fp16 VCVT instruction.  */
  if (flavour == neon_cvt_flavour_s32_f16
      ||flavour == neon_cvt_flavour_u32_f16)
    do_scalar_fp16_v82_encode ();
  inst.instruction |= op << 7;
  inst.instruction |= rm << 16;
  inst.instruction |= 0xf0000000;
  inst.is_neon = true;
}

static void
do_neon_cvt_1 (enum neon_cvt_mode mode)
{
  enum neon_shape rs = neon_select_shape (NS_DDI, NS_QQI, NS_FFI, NS_DD, NS_QQ,
					  NS_FD, NS_DF, NS_FF, NS_QD, NS_DQ,
					  NS_FH, NS_HF, NS_FHI, NS_HFI,
					  NS_NULL);
  enum neon_cvt_flavour flavour = get_neon_cvt_flavour (rs);

  if (flavour == neon_cvt_flavour_invalid)
    return;

  /* PR11109: Handle round-to-zero for VCVT conversions.  */
  if (mode == neon_cvt_mode_z
      && ARM_CPU_HAS_FEATURE (cpu_variant, fpu_arch_vfp_v2)
      && (flavour == neon_cvt_flavour_s16_f16
	  || flavour == neon_cvt_flavour_u16_f16
	  || flavour == neon_cvt_flavour_s32_f32
	  || flavour == neon_cvt_flavour_u32_f32
	  || flavour == neon_cvt_flavour_s32_f64
	  || flavour == neon_cvt_flavour_u32_f64)
      && (rs == NS_FD || rs == NS_FF))
    {
      do_vfp_nsyn_cvtz ();
      return;
    }

  /* ARMv8.2 fp16 VCVT conversions.  */
  if (mode == neon_cvt_mode_z
      && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_fp16)
      && (flavour == neon_cvt_flavour_s32_f16
	  || flavour == neon_cvt_flavour_u32_f16)
      && (rs == NS_FH))
    {
      do_vfp_nsyn_cvtz ();
      do_scalar_fp16_v82_encode ();
      return;
    }

  /* VFP rather than Neon conversions.  */
  if (flavour >= neon_cvt_flavour_first_fp)
    {
      if (mode == neon_cvt_mode_x || mode == neon_cvt_mode_z)
	do_vfp_nsyn_cvt (rs, flavour);
      else
	do_vfp_nsyn_cvt_fpv8 (flavour, mode);

      return;
    }

  switch (rs)
    {
    case NS_QQI:
      if (mode == neon_cvt_mode_z
	  && (flavour == neon_cvt_flavour_f16_s16
	      || flavour == neon_cvt_flavour_f16_u16
	      || flavour == neon_cvt_flavour_s16_f16
	      || flavour == neon_cvt_flavour_u16_f16
	      || flavour == neon_cvt_flavour_f32_u32
	      || flavour == neon_cvt_flavour_f32_s32
	      || flavour == neon_cvt_flavour_s32_f32
	      || flavour == neon_cvt_flavour_u32_f32))
	{
	  if (!check_simd_pred_availability (true,
					     NEON_CHECK_CC | NEON_CHECK_ARCH))
	    return;
	}
      /* fall through.  */
    case NS_DDI:
      {
	unsigned immbits;
	unsigned enctab[] = {0x0000100, 0x1000100, 0x0, 0x1000000,
			     0x0000100, 0x1000100, 0x0, 0x1000000};

	if ((rs != NS_QQI || !ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext))
	    && vfp_or_neon_is_neon (NEON_CHECK_CC | NEON_CHECK_ARCH) == FAIL)
	    return;

	if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext))
	  {
	    constraint (inst.operands[2].present && inst.operands[2].imm == 0,
			_("immediate value out of range"));
	    switch (flavour)
	      {
		case neon_cvt_flavour_f16_s16:
		case neon_cvt_flavour_f16_u16:
		case neon_cvt_flavour_s16_f16:
		case neon_cvt_flavour_u16_f16:
		  constraint (inst.operands[2].imm > 16,
			      _("immediate value out of range"));
		  break;
		case neon_cvt_flavour_f32_u32:
		case neon_cvt_flavour_f32_s32:
		case neon_cvt_flavour_s32_f32:
		case neon_cvt_flavour_u32_f32:
		  constraint (inst.operands[2].imm > 32,
			      _("immediate value out of range"));
		  break;
		default:
		  inst.error = BAD_FPU;
		  return;
	      }
	  }

	/* Fixed-point conversion with #0 immediate is encoded as an
	   integer conversion.  */
	if (inst.operands[2].present && inst.operands[2].imm == 0)
	  goto int_encode;
	NEON_ENCODE (IMMED, inst);
	if (flavour != neon_cvt_flavour_invalid)
	  inst.instruction |= enctab[flavour];
	inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
	inst.instruction |= HI1 (inst.operands[0].reg) << 22;
	inst.instruction |= LOW4 (inst.operands[1].reg);
	inst.instruction |= HI1 (inst.operands[1].reg) << 5;
	inst.instruction |= neon_quad (rs) << 6;
	inst.instruction |= 1 << 21;
	if (flavour < neon_cvt_flavour_s16_f16)
	  {
	    inst.instruction |= 1 << 21;
	    immbits = 32 - inst.operands[2].imm;
	    inst.instruction |= immbits << 16;
	  }
	else
	  {
	    inst.instruction |= 3 << 20;
	    immbits = 16 - inst.operands[2].imm;
	    inst.instruction |= immbits << 16;
	    inst.instruction &= ~(1 << 9);
	  }

	neon_dp_fixup (&inst);
      }
      break;

    case NS_QQ:
      if ((mode == neon_cvt_mode_a || mode == neon_cvt_mode_n
	   || mode == neon_cvt_mode_m || mode == neon_cvt_mode_p)
	  && (flavour == neon_cvt_flavour_s16_f16
	      || flavour == neon_cvt_flavour_u16_f16
	      || flavour == neon_cvt_flavour_s32_f32
	      || flavour == neon_cvt_flavour_u32_f32))
	{
	  if (!check_simd_pred_availability (true,
					     NEON_CHECK_CC | NEON_CHECK_ARCH8))
	    return;
	}
      else if (mode == neon_cvt_mode_z
	       && (flavour == neon_cvt_flavour_f16_s16
		   || flavour == neon_cvt_flavour_f16_u16
		   || flavour == neon_cvt_flavour_s16_f16
		   || flavour == neon_cvt_flavour_u16_f16
		   || flavour == neon_cvt_flavour_f32_u32
		   || flavour == neon_cvt_flavour_f32_s32
		   || flavour == neon_cvt_flavour_s32_f32
		   || flavour == neon_cvt_flavour_u32_f32))
	{
	  if (!check_simd_pred_availability (true,
					     NEON_CHECK_CC | NEON_CHECK_ARCH))
	    return;
	}
      /* fall through.  */
    case NS_DD:
      if (mode != neon_cvt_mode_x && mode != neon_cvt_mode_z)
	{

	  NEON_ENCODE (FLOAT, inst);
	  if (!check_simd_pred_availability (true,
					     NEON_CHECK_CC | NEON_CHECK_ARCH8))
	    return;

	  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
	  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
	  inst.instruction |= LOW4 (inst.operands[1].reg);
	  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
	  inst.instruction |= neon_quad (rs) << 6;
	  inst.instruction |= (flavour == neon_cvt_flavour_u16_f16
			       || flavour == neon_cvt_flavour_u32_f32) << 7;
	  inst.instruction |= mode << 8;
	  if (flavour == neon_cvt_flavour_u16_f16
	      || flavour == neon_cvt_flavour_s16_f16)
	    /* Mask off the original size bits and reencode them.  */
	    inst.instruction = ((inst.instruction & 0xfff3ffff) | (1 << 18));

	  if (thumb_mode)
	    inst.instruction |= 0xfc000000;
	  else
	    inst.instruction |= 0xf0000000;
	}
      else
	{
    int_encode:
	  {
	    unsigned enctab[] = { 0x100, 0x180, 0x0, 0x080,
				  0x100, 0x180, 0x0, 0x080};

	    NEON_ENCODE (INTEGER, inst);

	  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext))
	    {
	      if (vfp_or_neon_is_neon (NEON_CHECK_CC | NEON_CHECK_ARCH) == FAIL)
		return;
	    }

	    if (flavour != neon_cvt_flavour_invalid)
	      inst.instruction |= enctab[flavour];

	    inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
	    inst.instruction |= HI1 (inst.operands[0].reg) << 22;
	    inst.instruction |= LOW4 (inst.operands[1].reg);
	    inst.instruction |= HI1 (inst.operands[1].reg) << 5;
	    inst.instruction |= neon_quad (rs) << 6;
	    if (flavour >= neon_cvt_flavour_s16_f16
		&& flavour <= neon_cvt_flavour_f16_u16)
	      /* Half precision.  */
	      inst.instruction |= 1 << 18;
	    else
	      inst.instruction |= 2 << 18;

	    neon_dp_fixup (&inst);
	  }
	}
      break;

    /* Half-precision conversions for Advanced SIMD -- neon.  */
    case NS_QD:
    case NS_DQ:
      if (vfp_or_neon_is_neon (NEON_CHECK_CC | NEON_CHECK_ARCH) == FAIL)
	return;

      if ((rs == NS_DQ)
	  && (inst.vectype.el[0].size != 16 || inst.vectype.el[1].size != 32))
	  {
	    as_bad (_("operand size must match register width"));
	    break;
	  }

      if ((rs == NS_QD)
	  && ((inst.vectype.el[0].size != 32 || inst.vectype.el[1].size != 16)))
	  {
	    as_bad (_("operand size must match register width"));
	    break;
	  }

      if (rs == NS_DQ)
	{
	  if (flavour == neon_cvt_flavour_bf16_f32)
	    {
	      if (vfp_or_neon_is_neon (NEON_CHECK_ARCH8) == FAIL)
		return;
	      constraint (!mark_feature_used (&arm_ext_bf16), _(BAD_BF16));
	      /* VCVT.bf16.f32.  */
	      inst.instruction = 0x11b60640;
	    }
	  else
	    /* VCVT.f16.f32.  */
	    inst.instruction = 0x3b60600;
	}
      else
	/* VCVT.f32.f16.  */
	inst.instruction = 0x3b60700;

      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= LOW4 (inst.operands[1].reg);
      inst.instruction |= HI1 (inst.operands[1].reg) << 5;
      neon_dp_fixup (&inst);
      break;

    default:
      /* Some VFP conversions go here (s32 <-> f32, u32 <-> f32).  */
      if (mode == neon_cvt_mode_x || mode == neon_cvt_mode_z)
	do_vfp_nsyn_cvt (rs, flavour);
      else
	do_vfp_nsyn_cvt_fpv8 (flavour, mode);
    }
}

static void
do_neon_cvtr (void)
{
  do_neon_cvt_1 (neon_cvt_mode_x);
}

static void
do_neon_cvt (void)
{
  do_neon_cvt_1 (neon_cvt_mode_z);
}

static void
do_neon_cvta (void)
{
  do_neon_cvt_1 (neon_cvt_mode_a);
}

static void
do_neon_cvtn (void)
{
  do_neon_cvt_1 (neon_cvt_mode_n);
}

static void
do_neon_cvtp (void)
{
  do_neon_cvt_1 (neon_cvt_mode_p);
}

static void
do_neon_cvtm (void)
{
  do_neon_cvt_1 (neon_cvt_mode_m);
}

static void
do_neon_cvttb_2 (bool t, bool to, bool is_double)
{
  if (is_double)
    mark_feature_used (&fpu_vfp_ext_armv8);

  encode_arm_vfp_reg (inst.operands[0].reg,
		      (is_double && !to) ? VFP_REG_Dd : VFP_REG_Sd);
  encode_arm_vfp_reg (inst.operands[1].reg,
		      (is_double && to) ? VFP_REG_Dm : VFP_REG_Sm);
  inst.instruction |= to ? 0x10000 : 0;
  inst.instruction |= t ? 0x80 : 0;
  inst.instruction |= is_double ? 0x100 : 0;
  do_vfp_cond_or_thumb ();
}

static void
do_neon_cvttb_1 (bool t)
{
  enum neon_shape rs = neon_select_shape (NS_HF, NS_HD, NS_FH, NS_FF, NS_FD,
					  NS_DF, NS_DH, NS_QQ, NS_QQI, NS_NULL);

  if (rs == NS_NULL)
    return;
  else if (rs == NS_QQ || rs == NS_QQI)
    {
      int single_to_half = 0;
      if (!check_simd_pred_availability (true, NEON_CHECK_ARCH))
	return;

      enum neon_cvt_flavour flavour = get_neon_cvt_flavour (rs);

      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
	  && (flavour ==  neon_cvt_flavour_u16_f16
	      || flavour ==  neon_cvt_flavour_s16_f16
	      || flavour ==  neon_cvt_flavour_f16_s16
	      || flavour ==  neon_cvt_flavour_f16_u16
	      || flavour ==  neon_cvt_flavour_u32_f32
	      || flavour ==  neon_cvt_flavour_s32_f32
	      || flavour ==  neon_cvt_flavour_f32_s32
	      || flavour ==  neon_cvt_flavour_f32_u32))
	{
	  inst.cond = 0xf;
	  inst.instruction = N_MNEM_vcvt;
	  set_pred_insn_type (INSIDE_VPT_INSN);
	  do_neon_cvt_1 (neon_cvt_mode_z);
	  return;
	}
      else if (rs == NS_QQ && flavour == neon_cvt_flavour_f32_f16)
	single_to_half = 1;
      else if (rs == NS_QQ && flavour != neon_cvt_flavour_f16_f32)
	{
	  first_error (BAD_FPU);
	  return;
	}

      inst.instruction = 0xee3f0e01;
      inst.instruction |= single_to_half << 28;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= LOW4 (inst.operands[0].reg) << 13;
      inst.instruction |= t << 12;
      inst.instruction |= HI1 (inst.operands[1].reg) << 5;
      inst.instruction |= LOW4 (inst.operands[1].reg) << 1;
      inst.is_neon = 1;
    }
  else if (neon_check_type (2, rs, N_F16, N_F32 | N_VFP).type != NT_invtype)
    {
      inst.error = NULL;
      do_neon_cvttb_2 (t, /*to=*/true, /*is_double=*/false);
    }
  else if (neon_check_type (2, rs, N_F32 | N_VFP, N_F16).type != NT_invtype)
    {
      inst.error = NULL;
      do_neon_cvttb_2 (t, /*to=*/false, /*is_double=*/false);
    }
  else if (neon_check_type (2, rs, N_F16, N_F64 | N_VFP).type != NT_invtype)
    {
      /* The VCVTB and VCVTT instructions with D-register operands
         don't work for SP only targets.  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_armv8),
		  _(BAD_FPU));

      inst.error = NULL;
      do_neon_cvttb_2 (t, /*to=*/true, /*is_double=*/true);
    }
  else if (neon_check_type (2, rs, N_F64 | N_VFP, N_F16).type != NT_invtype)
    {
      /* The VCVTB and VCVTT instructions with D-register operands
         don't work for SP only targets.  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_armv8),
		  _(BAD_FPU));

      inst.error = NULL;
      do_neon_cvttb_2 (t, /*to=*/false, /*is_double=*/true);
    }
  else if (neon_check_type (2, rs, N_BF16 | N_VFP, N_F32).type != NT_invtype)
    {
      constraint (!mark_feature_used (&arm_ext_bf16), _(BAD_BF16));
      inst.error = NULL;
      inst.instruction |= (1 << 8);
      inst.instruction &= ~(1 << 9);
      do_neon_cvttb_2 (t, /*to=*/true, /*is_double=*/false);
    }
  else
    return;
}

static void
do_neon_cvtb (void)
{
  do_neon_cvttb_1 (false);
}


static void
do_neon_cvtt (void)
{
  do_neon_cvttb_1 (true);
}

static void
neon_move_immediate (void)
{
  enum neon_shape rs = neon_select_shape (NS_DI, NS_QI, NS_NULL);
  struct neon_type_el et = neon_check_type (2, rs,
    N_I8 | N_I16 | N_I32 | N_I64 | N_F32 | N_KEY, N_EQK);
  unsigned immlo, immhi = 0, immbits;
  int op, cmode, float_p;

  constraint (et.type == NT_invtype,
	      _("operand size must be specified for immediate VMOV"));

  /* We start out as an MVN instruction if OP = 1, MOV otherwise.  */
  op = (inst.instruction & (1 << 5)) != 0;

  immlo = inst.operands[1].imm;
  if (inst.operands[1].regisimm)
    immhi = inst.operands[1].reg;

  constraint (et.size < 32 && (immlo & ~((1 << et.size) - 1)) != 0,
	      _("immediate has bits set outside the operand size"));

  float_p = inst.operands[1].immisfloat;

  if ((cmode = neon_cmode_for_move_imm (immlo, immhi, float_p, &immbits, &op,
					et.size, et.type)) == FAIL)
    {
      /* Invert relevant bits only.  */
      neon_invert_size (&immlo, &immhi, et.size);
      /* Flip from VMOV/VMVN to VMVN/VMOV. Some immediate types are unavailable
	 with one or the other; those cases are caught by
	 neon_cmode_for_move_imm.  */
      op = !op;
      if ((cmode = neon_cmode_for_move_imm (immlo, immhi, float_p, &immbits,
					    &op, et.size, et.type)) == FAIL)
	{
	  first_error (_("immediate out of range"));
	  return;
	}
    }

  inst.instruction &= ~(1 << 5);
  inst.instruction |= op << 5;

  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= neon_quad (rs) << 6;
  inst.instruction |= cmode << 8;

  neon_write_immbits (immbits);
}

static void
do_neon_mvn (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_CC | NEON_CHECK_ARCH))
    return;

  if (inst.operands[1].isreg)
    {
      enum neon_shape rs;
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	rs = neon_select_shape (NS_QQ, NS_NULL);
      else
	rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);

      if (rs == NS_NULL)
	return;

      NEON_ENCODE (INTEGER, inst);
      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= LOW4 (inst.operands[1].reg);
      inst.instruction |= HI1 (inst.operands[1].reg) << 5;
      inst.instruction |= neon_quad (rs) << 6;
    }
  else
    {
      NEON_ENCODE (IMMED, inst);
      neon_move_immediate ();
    }

  neon_dp_fixup (&inst);

  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      constraint (!inst.operands[1].isreg && !inst.operands[0].isquad, BAD_FPU);
    }
}

/* Encode instructions of form:

  |28/24|23|22|21 20|19 16|15 12|11    8|7|6|5|4|3  0|
  |  U  |x |D |size | Rn  | Rd  |x x x x|N|x|M|x| Rm |  */

static void
neon_mixed_length (struct neon_type_el et, unsigned size)
{
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= LOW4 (inst.operands[2].reg);
  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
  inst.instruction |= (et.type == NT_unsigned) << 24;
  inst.instruction |= neon_logbits (size) << 20;

  neon_dp_fixup (&inst);
}

static void
do_neon_dyadic_long (void)
{
  enum neon_shape rs = neon_select_shape (NS_QDD, NS_HHH, NS_FFF, NS_DDD, NS_NULL);
  if (rs == NS_QDD)
    {
      if (vfp_or_neon_is_neon (NEON_CHECK_ARCH | NEON_CHECK_CC) == FAIL)
	return;

      NEON_ENCODE (INTEGER, inst);
      /* FIXME: Type checking for lengthening op.  */
      struct neon_type_el et = neon_check_type (3, NS_QDD,
	N_EQK | N_DBL, N_EQK, N_SU_32 | N_KEY);
      neon_mixed_length (et, et.size);
    }
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
	   && (inst.cond == 0xf || inst.cond == 0x10))
    {
      /* If parsing for MVE, vaddl/vsubl/vabdl{e,t} can only be vadd/vsub/vabd
	 in an IT block with le/lt conditions.  */

      if (inst.cond == 0xf)
	inst.cond = 0xb;
      else if (inst.cond == 0x10)
	inst.cond = 0xd;

      inst.pred_insn_type = INSIDE_IT_INSN;

      if (inst.instruction == N_MNEM_vaddl)
	{
	  inst.instruction = N_MNEM_vadd;
	  do_neon_addsub_if_i ();
	}
      else if (inst.instruction == N_MNEM_vsubl)
	{
	  inst.instruction = N_MNEM_vsub;
	  do_neon_addsub_if_i ();
	}
      else if (inst.instruction == N_MNEM_vabdl)
	{
	  inst.instruction = N_MNEM_vabd;
	  do_neon_dyadic_if_su ();
	}
    }
  else
    first_error (BAD_FPU);
}

static void
do_neon_abal (void)
{
  struct neon_type_el et = neon_check_type (3, NS_QDD,
    N_EQK | N_INT | N_DBL, N_EQK, N_SU_32 | N_KEY);
  neon_mixed_length (et, et.size);
}

static void
neon_mac_reg_scalar_long (unsigned regtypes, unsigned scalartypes)
{
  if (inst.operands[2].isscalar)
    {
      struct neon_type_el et = neon_check_type (3, NS_QDS,
	N_EQK | N_DBL, N_EQK, regtypes | N_KEY);
      NEON_ENCODE (SCALAR, inst);
      neon_mul_mac (et, et.type == NT_unsigned);
    }
  else
    {
      struct neon_type_el et = neon_check_type (3, NS_QDD,
	N_EQK | N_DBL, N_EQK, scalartypes | N_KEY);
      NEON_ENCODE (INTEGER, inst);
      neon_mixed_length (et, et.size);
    }
}

static void
do_neon_mac_maybe_scalar_long (void)
{
  neon_mac_reg_scalar_long (N_S16 | N_S32 | N_U16 | N_U32, N_SU_32);
}

/* Like neon_scalar_for_mul, this function generate Rm encoding from GAS's
   internal SCALAR.  QUAD_P is 1 if it's for Q format, otherwise it's 0.  */

static unsigned
neon_scalar_for_fmac_fp16_long (unsigned scalar, unsigned quad_p)
{
  unsigned regno = NEON_SCALAR_REG (scalar);
  unsigned elno = NEON_SCALAR_INDEX (scalar);

  if (quad_p)
    {
      if (regno > 7 || elno > 3)
	goto bad_scalar;

      return ((regno & 0x7)
	      | ((elno & 0x1) << 3)
	      | (((elno >> 1) & 0x1) << 5));
    }
  else
    {
      if (regno > 15 || elno > 1)
	goto bad_scalar;

      return (((regno & 0x1) << 5)
	      | ((regno >> 1) & 0x7)
	      | ((elno & 0x1) << 3));
    }

 bad_scalar:
  first_error (_("scalar out of range for multiply instruction"));
  return 0;
}

static void
do_neon_fmac_maybe_scalar_long (int subtype)
{
  enum neon_shape rs;
  int high8;
  /* NOTE: vfmal/vfmsl use slightly different NEON three-same encoding.  'size"
     field (bits[21:20]) has different meaning.  For scalar index variant, it's
     used to differentiate add and subtract, otherwise it's with fixed value
     0x2.  */
  int size = -1;

  /* vfmal/vfmsl are in three-same D/Q register format or the third operand can
     be a scalar index register.  */
  if (inst.operands[2].isscalar)
    {
      high8 = 0xfe000000;
      if (subtype)
	size = 16;
      rs = neon_select_shape (NS_DHS, NS_QDS, NS_NULL);
    }
  else
    {
      high8 = 0xfc000000;
      size = 32;
      if (subtype)
	inst.instruction |= (0x1 << 23);
      rs = neon_select_shape (NS_DHH, NS_QDD, NS_NULL);
    }


  if (inst.cond != COND_ALWAYS)
    as_warn (_("vfmal/vfmsl with FP16 type cannot be conditional, the "
	       "behaviour is UNPREDICTABLE"));

  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_fp16_fml),
	      _(BAD_FP16));

  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_armv8),
	      _(BAD_FPU));

  /* "opcode" from template has included "ubit", so simply pass 0 here.  Also,
     the "S" bit in size field has been reused to differentiate vfmal and vfmsl,
     so we simply pass -1 as size.  */
  unsigned quad_p = (rs == NS_QDD || rs == NS_QDS);
  neon_three_same (quad_p, 0, size);

  /* Undo neon_dp_fixup.  Redo the high eight bits.  */
  inst.instruction &= 0x00ffffff;
  inst.instruction |= high8;

  /* Unlike usually NEON three-same, encoding for Vn and Vm will depend on
     whether the instruction is in Q form and whether Vm is a scalar indexed
     operand.  */
  if (inst.operands[2].isscalar)
    {
      unsigned rm
	= neon_scalar_for_fmac_fp16_long (inst.operands[2].reg, quad_p);
      inst.instruction &= 0xffffffd0;
      inst.instruction |= rm;

      if (!quad_p)
	{
	  /* Redo Rn as well.  */
	  inst.instruction &= 0xfff0ff7f;
	  inst.instruction |= HI4 (inst.operands[1].reg) << 16;
	  inst.instruction |= LOW1 (inst.operands[1].reg) << 7;
	}
    }
  else if (!quad_p)
    {
      /* Redo Rn and Rm.  */
      inst.instruction &= 0xfff0ff50;
      inst.instruction |= HI4 (inst.operands[1].reg) << 16;
      inst.instruction |= LOW1 (inst.operands[1].reg) << 7;
      inst.instruction |= HI4 (inst.operands[2].reg);
      inst.instruction |= LOW1 (inst.operands[2].reg) << 5;
    }
}

static void
do_neon_vfmal (void)
{
  return do_neon_fmac_maybe_scalar_long (0);
}

static void
do_neon_vfmsl (void)
{
  return do_neon_fmac_maybe_scalar_long (1);
}

static void
do_neon_dyadic_wide (void)
{
  struct neon_type_el et = neon_check_type (3, NS_QQD,
    N_EQK | N_DBL, N_EQK | N_DBL, N_SU_32 | N_KEY);
  neon_mixed_length (et, et.size);
}

static void
do_neon_dyadic_narrow (void)
{
  struct neon_type_el et = neon_check_type (3, NS_QDD,
    N_EQK | N_DBL, N_EQK, N_I16 | N_I32 | N_I64 | N_KEY);
  /* Operand sign is unimportant, and the U bit is part of the opcode,
     so force the operand type to integer.  */
  et.type = NT_integer;
  neon_mixed_length (et, et.size / 2);
}

static void
do_neon_mul_sat_scalar_long (void)
{
  neon_mac_reg_scalar_long (N_S16 | N_S32, N_S16 | N_S32);
}

static void
do_neon_vmull (void)
{
  if (inst.operands[2].isscalar)
    do_neon_mac_maybe_scalar_long ();
  else
    {
      struct neon_type_el et = neon_check_type (3, NS_QDD,
	N_EQK | N_DBL, N_EQK, N_SU_32 | N_P8 | N_P64 | N_KEY);

      if (et.type == NT_poly)
	NEON_ENCODE (POLY, inst);
      else
	NEON_ENCODE (INTEGER, inst);

      /* For polynomial encoding the U bit must be zero, and the size must
	 be 8 (encoded as 0b00) or, on ARMv8 or later 64 (encoded, non
	 obviously, as 0b10).  */
      if (et.size == 64)
	{
	  /* Check we're on the correct architecture.  */
	  if (!mark_feature_used (&fpu_crypto_ext_armv8))
	    inst.error =
	      _("Instruction form not available on this architecture.");

	  et.size = 32;
	}

      neon_mixed_length (et, et.size);
    }
}

static void
do_neon_ext (void)
{
  enum neon_shape rs = neon_select_shape (NS_DDDI, NS_QQQI, NS_NULL);
  struct neon_type_el et = neon_check_type (3, rs,
    N_EQK, N_EQK, N_8 | N_16 | N_32 | N_64 | N_KEY);
  unsigned imm = (inst.operands[3].imm * et.size) / 8;

  constraint (imm >= (unsigned) (neon_quad (rs) ? 16 : 8),
	      _("shift out of range"));
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= LOW4 (inst.operands[2].reg);
  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
  inst.instruction |= neon_quad (rs) << 6;
  inst.instruction |= imm << 8;

  neon_dp_fixup (&inst);
}

static void
do_neon_rev (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
   return;

  enum neon_shape rs;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    rs = neon_select_shape (NS_QQ, NS_NULL);
  else
    rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);

  struct neon_type_el et = neon_check_type (2, rs,
    N_EQK, N_8 | N_16 | N_32 | N_KEY);

  unsigned op = (inst.instruction >> 7) & 3;
  /* N (width of reversed regions) is encoded as part of the bitmask. We
     extract it here to check the elements to be reversed are smaller.
     Otherwise we'd get a reserved instruction.  */
  unsigned elsize = (op == 2) ? 16 : (op == 1) ? 32 : (op == 0) ? 64 : 0;

  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext) && elsize == 64
      && inst.operands[0].reg == inst.operands[1].reg)
    as_tsktsk (_("Warning: 64-bit element size and same destination and source"
		 " operands makes instruction UNPREDICTABLE"));

  gas_assert (elsize != 0);
  constraint (et.size >= elsize,
	      _("elements must be smaller than reversal region"));
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_dup (void)
{
  if (inst.operands[1].isscalar)
    {
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_v1),
		  BAD_FPU);
      enum neon_shape rs = neon_select_shape (NS_DS, NS_QS, NS_NULL);
      struct neon_type_el et = neon_check_type (2, rs,
	N_EQK, N_8 | N_16 | N_32 | N_KEY);
      unsigned sizebits = et.size >> 3;
      unsigned dm = NEON_SCALAR_REG (inst.operands[1].reg);
      int logsize = neon_logbits (et.size);
      unsigned x = NEON_SCALAR_INDEX (inst.operands[1].reg) << logsize;

      if (vfp_or_neon_is_neon (NEON_CHECK_CC) == FAIL)
	return;

      NEON_ENCODE (SCALAR, inst);
      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= LOW4 (dm);
      inst.instruction |= HI1 (dm) << 5;
      inst.instruction |= neon_quad (rs) << 6;
      inst.instruction |= x << 17;
      inst.instruction |= sizebits << 16;

      neon_dp_fixup (&inst);
    }
  else
    {
      enum neon_shape rs = neon_select_shape (NS_DR, NS_QR, NS_NULL);
      struct neon_type_el et = neon_check_type (2, rs,
	N_8 | N_16 | N_32 | N_KEY, N_EQK);
      if (rs == NS_QR)
	{
	  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH))
	    return;
	}
      else
	constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_v1),
		    BAD_FPU);

      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	{
	  if (inst.operands[1].reg == REG_SP)
	    as_tsktsk (MVE_BAD_SP);
	  else if (inst.operands[1].reg == REG_PC)
	    as_tsktsk (MVE_BAD_PC);
	}

      /* Duplicate ARM register to lanes of vector.  */
      NEON_ENCODE (ARMREG, inst);
      switch (et.size)
	{
	case 8:  inst.instruction |= 0x400000; break;
	case 16: inst.instruction |= 0x000020; break;
	case 32: inst.instruction |= 0x000000; break;
	default: break;
	}
      inst.instruction |= LOW4 (inst.operands[1].reg) << 12;
      inst.instruction |= LOW4 (inst.operands[0].reg) << 16;
      inst.instruction |= HI1 (inst.operands[0].reg) << 7;
      inst.instruction |= neon_quad (rs) << 21;
      /* The encoding for this instruction is identical for the ARM and Thumb
	 variants, except for the condition field.  */
      do_vfp_cond_or_thumb ();
    }
}

static void
do_mve_mov (int toQ)
{
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    return;
  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = MVE_UNPREDICABLE_INSN;

  unsigned Rt = 0, Rt2 = 1, Q0 = 2, Q1 = 3;
  if (toQ)
    {
      Q0 = 0;
      Q1 = 1;
      Rt = 2;
      Rt2 = 3;
    }

  constraint (inst.operands[Q0].reg != inst.operands[Q1].reg + 2,
	      _("Index one must be [2,3] and index two must be two less than"
		" index one."));
  constraint (!toQ && inst.operands[Rt].reg == inst.operands[Rt2].reg,
	      _("Destination registers may not be the same"));
  constraint (inst.operands[Rt].reg == REG_SP
	      || inst.operands[Rt2].reg == REG_SP,
	      BAD_SP);
  constraint (inst.operands[Rt].reg == REG_PC
	      || inst.operands[Rt2].reg == REG_PC,
	      BAD_PC);

  inst.instruction = 0xec000f00;
  inst.instruction |= HI1 (inst.operands[Q1].reg / 32) << 23;
  inst.instruction |= !!toQ << 20;
  inst.instruction |= inst.operands[Rt2].reg << 16;
  inst.instruction |= LOW4 (inst.operands[Q1].reg / 32) << 13;
  inst.instruction |= (inst.operands[Q1].reg % 4) << 4;
  inst.instruction |= inst.operands[Rt].reg;
}

static void
do_mve_movn (void)
{
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    return;

  if (inst.cond > COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;
  else
    inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;

  struct neon_type_el et = neon_check_type (2, NS_QQ, N_EQK, N_I16 | N_I32
					    | N_KEY);

  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= (neon_logbits (et.size) - 1) << 18;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.is_neon = 1;

}

/* VMOV has particularly many variations. It can be one of:
     0. VMOV<c><q> <Qd>, <Qm>
     1. VMOV<c><q> <Dd>, <Dm>
   (Register operations, which are VORR with Rm = Rn.)
     2. VMOV<c><q>.<dt> <Qd>, #<imm>
     3. VMOV<c><q>.<dt> <Dd>, #<imm>
   (Immediate loads.)
     4. VMOV<c><q>.<size> <Dn[x]>, <Rd>
   (ARM register to scalar.)
     5. VMOV<c><q> <Dm>, <Rd>, <Rn>
   (Two ARM registers to vector.)
     6. VMOV<c><q>.<dt> <Rd>, <Dn[x]>
   (Scalar to ARM register.)
     7. VMOV<c><q> <Rd>, <Rn>, <Dm>
   (Vector to two ARM registers.)
     8. VMOV.F32 <Sd>, <Sm>
     9. VMOV.F64 <Dd>, <Dm>
   (VFP register moves.)
    10. VMOV.F32 <Sd>, #imm
    11. VMOV.F64 <Dd>, #imm
   (VFP float immediate load.)
    12. VMOV <Rd>, <Sm>
   (VFP single to ARM reg.)
    13. VMOV <Sd>, <Rm>
   (ARM reg to VFP single.)
    14. VMOV <Rd>, <Re>, <Sn>, <Sm>
   (Two ARM regs to two VFP singles.)
    15. VMOV <Sd>, <Se>, <Rn>, <Rm>
   (Two VFP singles to two ARM regs.)
   16. VMOV<c> <Rt>, <Rt2>, <Qd[idx]>, <Qd[idx2]>
   17. VMOV<c> <Qd[idx]>, <Qd[idx2]>, <Rt>, <Rt2>
   18. VMOV<c>.<dt> <Rt>, <Qn[idx]>
   19. VMOV<c>.<dt> <Qd[idx]>, <Rt>

   These cases can be disambiguated using neon_select_shape, except cases 1/9
   and 3/11 which depend on the operand type too.

   All the encoded bits are hardcoded by this function.

   Cases 4, 6 may be used with VFPv1 and above (only 32-bit transfers!).
   Cases 5, 7 may be used with VFPv2 and above.

   FIXME: Some of the checking may be a bit sloppy (in a couple of cases you
   can specify a type where it doesn't make sense to, and is ignored).  */

static void
do_neon_mov (void)
{
  enum neon_shape rs = neon_select_shape (NS_RRSS, NS_SSRR, NS_RRFF, NS_FFRR,
					  NS_DRR, NS_RRD, NS_QQ, NS_DD, NS_QI,
					  NS_DI, NS_SR, NS_RS, NS_FF, NS_FI,
					  NS_RF, NS_FR, NS_HR, NS_RH, NS_HI,
					  NS_NULL);
  struct neon_type_el et;
  const char *ldconst = 0;

  switch (rs)
    {
    case NS_DD:  /* case 1/9.  */
      et = neon_check_type (2, rs, N_EQK, N_F64 | N_KEY);
      /* It is not an error here if no type is given.  */
      inst.error = NULL;

      /* In MVE we interpret the following instructions as same, so ignoring
	 the following type (float) and size (64) checks.
	 a: VMOV<c><q> <Dd>, <Dm>
	 b: VMOV<c><q>.F64 <Dd>, <Dm>.  */
      if ((et.type == NT_float && et.size == 64)
	  || (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)))
	{
	  do_vfp_nsyn_opcode ("fcpyd");
	  break;
	}
      /* fall through.  */

    case NS_QQ:  /* case 0/1.  */
      {
	if (!check_simd_pred_availability (false,
					   NEON_CHECK_CC | NEON_CHECK_ARCH))
	  return;
	/* The architecture manual I have doesn't explicitly state which
	   value the U bit should have for register->register moves, but
	   the equivalent VORR instruction has U = 0, so do that.  */
	inst.instruction = 0x0200110;
	inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
	inst.instruction |= HI1 (inst.operands[0].reg) << 22;
	inst.instruction |= LOW4 (inst.operands[1].reg);
	inst.instruction |= HI1 (inst.operands[1].reg) << 5;
	inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
	inst.instruction |= HI1 (inst.operands[1].reg) << 7;
	inst.instruction |= neon_quad (rs) << 6;

	neon_dp_fixup (&inst);
      }
      break;

    case NS_DI:  /* case 3/11.  */
      et = neon_check_type (2, rs, N_EQK, N_F64 | N_KEY);
      inst.error = NULL;
      if (et.type == NT_float && et.size == 64)
	{
	  /* case 11 (fconstd).  */
	  ldconst = "fconstd";
	  goto encode_fconstd;
	}
      /* fall through.  */

    case NS_QI:  /* case 2/3.  */
      if (!check_simd_pred_availability (false,
					 NEON_CHECK_CC | NEON_CHECK_ARCH))
	return;
      inst.instruction = 0x0800010;
      neon_move_immediate ();
      neon_dp_fixup (&inst);
      break;

    case NS_SR:  /* case 4.  */
      {
	unsigned bcdebits = 0;
	int logsize;
	unsigned dn = NEON_SCALAR_REG (inst.operands[0].reg);
	unsigned x = NEON_SCALAR_INDEX (inst.operands[0].reg);

	/* .<size> is optional here, defaulting to .32. */
	if (inst.vectype.elems == 0
	    && inst.operands[0].vectype.type == NT_invtype
	    && inst.operands[1].vectype.type == NT_invtype)
	  {
	    inst.vectype.el[0].type = NT_untyped;
	    inst.vectype.el[0].size = 32;
	    inst.vectype.elems = 1;
	  }

	et = neon_check_type (2, NS_NULL, N_8 | N_16 | N_32 | N_KEY, N_EQK);
	logsize = neon_logbits (et.size);

	if (et.size != 32)
	  {
	    if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
		&& vfp_or_neon_is_neon (NEON_CHECK_ARCH) == FAIL)
	      return;
	  }
	else
	  {
	    constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1)
			&& !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
			_(BAD_FPU));
	  }

	if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	  {
	    if (inst.operands[1].reg == REG_SP)
	      as_tsktsk (MVE_BAD_SP);
	    else if (inst.operands[1].reg == REG_PC)
	      as_tsktsk (MVE_BAD_PC);
	  }
	unsigned size = inst.operands[0].isscalar == 1 ? 64 : 128;

	constraint (et.type == NT_invtype, _("bad type for scalar"));
	constraint (x >= size / et.size, _("scalar index out of range"));


	switch (et.size)
	  {
	  case 8:  bcdebits = 0x8; break;
	  case 16: bcdebits = 0x1; break;
	  case 32: bcdebits = 0x0; break;
	  default: ;
	  }

	bcdebits |= (x & ((1 << (3-logsize)) - 1)) << logsize;

	inst.instruction = 0xe000b10;
	do_vfp_cond_or_thumb ();
	inst.instruction |= LOW4 (dn) << 16;
	inst.instruction |= HI1 (dn) << 7;
	inst.instruction |= inst.operands[1].reg << 12;
	inst.instruction |= (bcdebits & 3) << 5;
	inst.instruction |= ((bcdebits >> 2) & 3) << 21;
	inst.instruction |= (x >> (3-logsize)) << 16;
      }
      break;

    case NS_DRR:  /* case 5 (fmdrr).  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v2)
		  && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
		  _(BAD_FPU));

      inst.instruction = 0xc400b10;
      do_vfp_cond_or_thumb ();
      inst.instruction |= LOW4 (inst.operands[0].reg);
      inst.instruction |= HI1 (inst.operands[0].reg) << 5;
      inst.instruction |= inst.operands[1].reg << 12;
      inst.instruction |= inst.operands[2].reg << 16;
      break;

    case NS_RS:  /* case 6.  */
      {
	unsigned logsize;
	unsigned dn = NEON_SCALAR_REG (inst.operands[1].reg);
	unsigned x = NEON_SCALAR_INDEX (inst.operands[1].reg);
	unsigned abcdebits = 0;

	/* .<dt> is optional here, defaulting to .32. */
	if (inst.vectype.elems == 0
	    && inst.operands[0].vectype.type == NT_invtype
	    && inst.operands[1].vectype.type == NT_invtype)
	  {
	    inst.vectype.el[0].type = NT_untyped;
	    inst.vectype.el[0].size = 32;
	    inst.vectype.elems = 1;
	  }

	et = neon_check_type (2, NS_NULL,
			      N_EQK, N_S8 | N_S16 | N_U8 | N_U16 | N_32 | N_KEY);
	logsize = neon_logbits (et.size);

	if (et.size != 32)
	  {
	    if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
		&& vfp_or_neon_is_neon (NEON_CHECK_CC
					| NEON_CHECK_ARCH) == FAIL)
	      return;
	  }
	else
	  {
	    constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1)
			&& !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
			_(BAD_FPU));
	  }

	if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	  {
	    if (inst.operands[0].reg == REG_SP)
	      as_tsktsk (MVE_BAD_SP);
	    else if (inst.operands[0].reg == REG_PC)
	      as_tsktsk (MVE_BAD_PC);
	  }

	unsigned size = inst.operands[1].isscalar == 1 ? 64 : 128;

	constraint (et.type == NT_invtype, _("bad type for scalar"));
	constraint (x >= size / et.size, _("scalar index out of range"));

	switch (et.size)
	  {
	  case 8:  abcdebits = (et.type == NT_signed) ? 0x08 : 0x18; break;
	  case 16: abcdebits = (et.type == NT_signed) ? 0x01 : 0x11; break;
	  case 32: abcdebits = 0x00; break;
	  default: ;
	  }

	abcdebits |= (x & ((1 << (3-logsize)) - 1)) << logsize;
	inst.instruction = 0xe100b10;
	do_vfp_cond_or_thumb ();
	inst.instruction |= LOW4 (dn) << 16;
	inst.instruction |= HI1 (dn) << 7;
	inst.instruction |= inst.operands[0].reg << 12;
	inst.instruction |= (abcdebits & 3) << 5;
	inst.instruction |= (abcdebits >> 2) << 21;
	inst.instruction |= (x >> (3-logsize)) << 16;
      }
      break;

    case NS_RRD:  /* case 7 (fmrrd).  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v2)
		  && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
		  _(BAD_FPU));

      inst.instruction = 0xc500b10;
      do_vfp_cond_or_thumb ();
      inst.instruction |= inst.operands[0].reg << 12;
      inst.instruction |= inst.operands[1].reg << 16;
      inst.instruction |= LOW4 (inst.operands[2].reg);
      inst.instruction |= HI1 (inst.operands[2].reg) << 5;
      break;

    case NS_FF:  /* case 8 (fcpys).  */
      do_vfp_nsyn_opcode ("fcpys");
      break;

    case NS_HI:
    case NS_FI:  /* case 10 (fconsts).  */
      ldconst = "fconsts";
    encode_fconstd:
      if (!inst.operands[1].immisfloat)
	{
	  unsigned new_imm;
	  /* Immediate has to fit in 8 bits so float is enough.  */
	  float imm = (float) inst.operands[1].imm;
	  memcpy (&new_imm, &imm, sizeof (float));
	  /* But the assembly may have been written to provide an integer
	     bit pattern that equates to a float, so check that the
	     conversion has worked.  */
	  if (is_quarter_float (new_imm))
	    {
	      if (is_quarter_float (inst.operands[1].imm))
		as_warn (_("immediate constant is valid both as a bit-pattern and a floating point value (using the fp value)"));

	      inst.operands[1].imm = new_imm;
	      inst.operands[1].immisfloat = 1;
	    }
	}

      if (is_quarter_float (inst.operands[1].imm))
	{
	  inst.operands[1].imm = neon_qfloat_bits (inst.operands[1].imm);
	  do_vfp_nsyn_opcode (ldconst);

	  /* ARMv8.2 fp16 vmov.f16 instruction.  */
	  if (rs == NS_HI)
	    do_scalar_fp16_v82_encode ();
	}
      else
	first_error (_("immediate out of range"));
      break;

    case NS_RH:
    case NS_RF:  /* case 12 (fmrs).  */
      do_vfp_nsyn_opcode ("fmrs");
      /* ARMv8.2 fp16 vmov.f16 instruction.  */
      if (rs == NS_RH)
	do_scalar_fp16_v82_encode ();
      break;

    case NS_HR:
    case NS_FR:  /* case 13 (fmsr).  */
      do_vfp_nsyn_opcode ("fmsr");
      /* ARMv8.2 fp16 vmov.f16 instruction.  */
      if (rs == NS_HR)
	do_scalar_fp16_v82_encode ();
      break;

    case NS_RRSS:
      do_mve_mov (0);
      break;
    case NS_SSRR:
      do_mve_mov (1);
      break;

    /* The encoders for the fmrrs and fmsrr instructions expect three operands
       (one of which is a list), but we have parsed four.  Do some fiddling to
       make the operands what do_vfp_reg2_from_sp2 and do_vfp_sp2_from_reg2
       expect.  */
    case NS_RRFF:  /* case 14 (fmrrs).  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v2)
		  && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
		  _(BAD_FPU));
      constraint (inst.operands[3].reg != inst.operands[2].reg + 1,
		  _("VFP registers must be adjacent"));
      inst.operands[2].imm = 2;
      memset (&inst.operands[3], '\0', sizeof (inst.operands[3]));
      do_vfp_nsyn_opcode ("fmrrs");
      break;

    case NS_FFRR:  /* case 15 (fmsrr).  */
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v2)
		  && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
		  _(BAD_FPU));
      constraint (inst.operands[1].reg != inst.operands[0].reg + 1,
		  _("VFP registers must be adjacent"));
      inst.operands[1] = inst.operands[2];
      inst.operands[2] = inst.operands[3];
      inst.operands[0].imm = 2;
      memset (&inst.operands[3], '\0', sizeof (inst.operands[3]));
      do_vfp_nsyn_opcode ("fmsrr");
      break;

    case NS_NULL:
      /* neon_select_shape has determined that the instruction
	 shape is wrong and has already set the error message.  */
      break;

    default:
      abort ();
    }
}

static void
do_mve_movl (void)
{
  if (!(inst.operands[0].present && inst.operands[0].isquad
      && inst.operands[1].present && inst.operands[1].isquad
      && !inst.operands[2].present))
    {
      inst.instruction = 0;
      inst.cond = 0xb;
      if (thumb_mode)
	set_pred_insn_type (INSIDE_IT_INSN);
      do_neon_mov ();
      return;
    }

  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    return;

  if (inst.cond != COND_ALWAYS)
    inst.pred_insn_type = INSIDE_VPT_INSN;

  struct neon_type_el et = neon_check_type (2, NS_QQ, N_EQK, N_S8 | N_U8
					    | N_S16 | N_U16 | N_KEY);

  inst.instruction |= (et.type == NT_unsigned) << 28;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= (neon_logbits (et.size) + 1) << 19;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.is_neon = 1;
}

static void
do_neon_rshift_round_imm (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
   return;

  enum neon_shape rs;
  struct neon_type_el et;

  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      rs = neon_select_shape (NS_QQI, NS_NULL);
      et = neon_check_type (2, rs, N_EQK, N_SU_MVE | N_KEY);
    }
  else
    {
      rs = neon_select_shape (NS_DDI, NS_QQI, NS_NULL);
      et = neon_check_type (2, rs, N_EQK, N_SU_ALL | N_KEY);
    }
  int imm = inst.operands[2].imm;

  /* imm == 0 case is encoded as VMOV for V{R}SHR.  */
  if (imm == 0)
    {
      inst.operands[2].present = 0;
      do_neon_mov ();
      return;
    }

  constraint (imm < 1 || (unsigned)imm > et.size,
	      _("immediate out of range for shift"));
  neon_imm_shift (true, et.type == NT_unsigned, neon_quad (rs), et,
		  et.size - imm);
}

static void
do_neon_movhf (void)
{
  enum neon_shape rs = neon_select_shape (NS_HH, NS_NULL);
  constraint (rs != NS_HH, _("invalid suffix"));

  if (inst.cond != COND_ALWAYS)
    {
      if (thumb_mode)
	{
	  as_warn (_("scalar fp16 instruction cannot be conditional,"
		     " the behaviour is UNPREDICTABLE"));
	}
      else
	{
	  inst.error = BAD_COND;
	  return;
	}
    }

  do_vfp_sp_monadic ();

  inst.is_neon = 1;
  inst.instruction |= 0xf0000000;
}

static void
do_neon_movl (void)
{
  struct neon_type_el et = neon_check_type (2, NS_QD,
    N_EQK | N_DBL, N_SU_32 | N_KEY);
  unsigned sizebits = et.size >> 3;
  inst.instruction |= sizebits << 19;
  neon_two_same (0, et.type == NT_unsigned, -1);
}

static void
do_neon_trn (void)
{
  enum neon_shape rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);
  struct neon_type_el et = neon_check_type (2, rs,
    N_EQK, N_8 | N_16 | N_32 | N_KEY);
  NEON_ENCODE (INTEGER, inst);
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_zip_uzp (void)
{
  enum neon_shape rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);
  struct neon_type_el et = neon_check_type (2, rs,
    N_EQK, N_8 | N_16 | N_32 | N_KEY);
  if (rs == NS_DD && et.size == 32)
    {
      /* Special case: encode as VTRN.32 <Dd>, <Dm>.  */
      inst.instruction = N_MNEM_vtrn;
      do_neon_trn ();
      return;
    }
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_sat_abs_neg (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_CC | NEON_CHECK_ARCH))
    return;

  enum neon_shape rs;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    rs = neon_select_shape (NS_QQ, NS_NULL);
  else
    rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);
  struct neon_type_el et = neon_check_type (2, rs,
    N_EQK, N_S8 | N_S16 | N_S32 | N_KEY);
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_pair_long (void)
{
  enum neon_shape rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);
  struct neon_type_el et = neon_check_type (2, rs, N_EQK, N_SU_32 | N_KEY);
  /* Unsigned is encoded in OP field (bit 7) for these instruction.  */
  inst.instruction |= (et.type == NT_unsigned) << 7;
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_recip_est (void)
{
  enum neon_shape rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);
  struct neon_type_el et = neon_check_type (2, rs,
    N_EQK | N_FLT, N_F_16_32 | N_U32 | N_KEY);
  inst.instruction |= (et.type == NT_float) << 8;
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_cls (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
    return;

  enum neon_shape rs;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
   rs = neon_select_shape (NS_QQ, NS_NULL);
  else
   rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);

  struct neon_type_el et = neon_check_type (2, rs,
    N_EQK, N_S8 | N_S16 | N_S32 | N_KEY);
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_clz (void)
{
  if (!check_simd_pred_availability (false, NEON_CHECK_ARCH | NEON_CHECK_CC))
    return;

  enum neon_shape rs;
  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
   rs = neon_select_shape (NS_QQ, NS_NULL);
  else
   rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);

  struct neon_type_el et = neon_check_type (2, rs,
    N_EQK, N_I8 | N_I16 | N_I32 | N_KEY);
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_cnt (void)
{
  enum neon_shape rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);
  struct neon_type_el et = neon_check_type (2, rs,
    N_EQK | N_INT, N_8 | N_KEY);
  neon_two_same (neon_quad (rs), 1, et.size);
}

static void
do_neon_swp (void)
{
  enum neon_shape rs = neon_select_shape (NS_DD, NS_QQ, NS_NULL);
  if (rs == NS_NULL)
    return;
  neon_two_same (neon_quad (rs), 1, -1);
}

static void
do_neon_tbl_tbx (void)
{
  unsigned listlenbits;
  neon_check_type (3, NS_DLD, N_EQK, N_EQK, N_8 | N_KEY);

  if (inst.operands[1].imm < 1 || inst.operands[1].imm > 4)
    {
      first_error (_("bad list length for table lookup"));
      return;
    }

  listlenbits = inst.operands[1].imm - 1;
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
  inst.instruction |= HI1 (inst.operands[1].reg) << 7;
  inst.instruction |= LOW4 (inst.operands[2].reg);
  inst.instruction |= HI1 (inst.operands[2].reg) << 5;
  inst.instruction |= listlenbits << 8;

  neon_dp_fixup (&inst);
}

static void
do_neon_ldm_stm (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd)
	      && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext),
	      _(BAD_FPU));
  /* P, U and L bits are part of bitmask.  */
  int is_dbmode = (inst.instruction & (1 << 24)) != 0;
  unsigned offsetbits = inst.operands[1].imm * 2;

  if (inst.operands[1].issingle)
    {
      do_vfp_nsyn_ldm_stm (is_dbmode);
      return;
    }

  constraint (is_dbmode && !inst.operands[0].writeback,
	      _("writeback (!) must be used for VLDMDB and VSTMDB"));

  constraint (inst.operands[1].imm < 1 || inst.operands[1].imm > 16,
	      _("register list must contain at least 1 and at most 16 "
		"registers"));

  inst.instruction |= inst.operands[0].reg << 16;
  inst.instruction |= inst.operands[0].writeback << 21;
  inst.instruction |= LOW4 (inst.operands[1].reg) << 12;
  inst.instruction |= HI1 (inst.operands[1].reg) << 22;

  inst.instruction |= offsetbits;

  do_vfp_cond_or_thumb ();
}

static void
do_vfp_nsyn_push_pop_check (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_v1xd), _(BAD_FPU));

  if (inst.operands[1].issingle)
    {
      constraint (inst.operands[1].imm < 1 || inst.operands[1].imm > 32,
		  _("register list must contain at least 1 and at most 32 registers"));
    }
  else
    {
      constraint (inst.operands[1].imm < 1 || inst.operands[1].imm > 16,
		  _("register list must contain at least 1 and at most 16 registers"));
    }
}

static void
do_vfp_nsyn_pop (void)
{
  nsyn_insert_sp ();

  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    return do_vfp_nsyn_opcode ("vldm");

  do_vfp_nsyn_push_pop_check ();

  if (inst.operands[1].issingle)
    do_vfp_nsyn_opcode ("fldmias");
  else
    do_vfp_nsyn_opcode ("fldmiad");
}

static void
do_vfp_nsyn_push (void)
{
  nsyn_insert_sp ();

  if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    return do_vfp_nsyn_opcode ("vstmdb");

  do_vfp_nsyn_push_pop_check ();

  if (inst.operands[1].issingle)
    do_vfp_nsyn_opcode ("fstmdbs");
  else
    do_vfp_nsyn_opcode ("fstmdbd");
}

static void
do_neon_ldr_str (void)
{
  int is_ldr = (inst.instruction & (1 << 20)) != 0;

  /* Use of PC in vstr in ARM mode is deprecated in ARMv7.
     And is UNPREDICTABLE in thumb mode.  */
  if (!is_ldr
      && inst.operands[1].reg == REG_PC
      && (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v7) || thumb_mode))
    {
      if (thumb_mode)
	inst.error = _("Use of PC here is UNPREDICTABLE");
      else if (warn_on_deprecated)
	as_tsktsk (_("Use of PC here is deprecated"));
    }

  if (inst.operands[0].issingle)
    {
      if (is_ldr)
	do_vfp_nsyn_opcode ("flds");
      else
	do_vfp_nsyn_opcode ("fsts");

      /* ARMv8.2 vldr.16/vstr.16 instruction.  */
      if (inst.vectype.el[0].size == 16)
	do_scalar_fp16_v82_encode ();
    }
  else
    {
      if (is_ldr)
	do_vfp_nsyn_opcode ("fldd");
      else
	do_vfp_nsyn_opcode ("fstd");
    }
}

static void
do_t_vldr_vstr_sysreg (void)
{
  int fp_vldr_bitno = 20, sysreg_vldr_bitno = 20;
  bool is_vldr = ((inst.instruction & (1 << fp_vldr_bitno)) != 0);

  /* Use of PC is UNPREDICTABLE.  */
  if (inst.operands[1].reg == REG_PC)
    inst.error = _("Use of PC here is UNPREDICTABLE");

  if (inst.operands[1].immisreg)
    inst.error = _("instruction does not accept register index");

  if (!inst.operands[1].isreg)
    inst.error = _("instruction does not accept PC-relative addressing");

  if (abs (inst.operands[1].imm) >= (1 << 7))
    inst.error = _("immediate value out of range");

  inst.instruction = 0xec000f80;
  if (is_vldr)
    inst.instruction |= 1 << sysreg_vldr_bitno;
  encode_arm_cp_address (1, true, false, BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM);
  inst.instruction |= (inst.operands[0].imm & 0x7) << 13;
  inst.instruction |= (inst.operands[0].imm & 0x8) << 19;
}

static void
do_vldr_vstr (void)
{
  bool sysreg_op = !inst.operands[0].isreg;

  /* VLDR/VSTR (System Register).  */
  if (sysreg_op)
    {
      if (!mark_feature_used (&arm_ext_v8_1m_main))
	as_bad (_("Instruction not permitted on this architecture"));

      do_t_vldr_vstr_sysreg ();
    }
  /* VLDR/VSTR.  */
  else
    {
      if (!mark_feature_used (&fpu_vfp_ext_v1xd)
	  && !ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
	as_bad (_("Instruction not permitted on this architecture"));
      do_neon_ldr_str ();
    }
}

/* "interleave" version also handles non-interleaving register VLD1/VST1
   instructions.  */

static void
do_neon_ld_st_interleave (void)
{
  struct neon_type_el et = neon_check_type (1, NS_NULL,
					    N_8 | N_16 | N_32 | N_64);
  unsigned alignbits = 0;
  unsigned idx;
  /* The bits in this table go:
     0: register stride of one (0) or two (1)
     1,2: register list length, minus one (1, 2, 3, 4).
     3,4: <n> in instruction type, minus one (VLD<n> / VST<n>).
     We use -1 for invalid entries.  */
  const int typetable[] =
    {
      0x7,  -1, 0xa,  -1, 0x6,  -1, 0x2,  -1, /* VLD1 / VST1.  */
       -1,  -1, 0x8, 0x9,  -1,  -1, 0x3,  -1, /* VLD2 / VST2.  */
       -1,  -1,  -1,  -1, 0x4, 0x5,  -1,  -1, /* VLD3 / VST3.  */
       -1,  -1,  -1,  -1,  -1,  -1, 0x0, 0x1  /* VLD4 / VST4.  */
    };
  int typebits;

  if (et.type == NT_invtype)
    return;

  if (inst.operands[1].immisalign)
    switch (inst.operands[1].imm >> 8)
      {
      case 64: alignbits = 1; break;
      case 128:
	if (NEON_REGLIST_LENGTH (inst.operands[0].imm) != 2
	    && NEON_REGLIST_LENGTH (inst.operands[0].imm) != 4)
	  goto bad_alignment;
	alignbits = 2;
	break;
      case 256:
	if (NEON_REGLIST_LENGTH (inst.operands[0].imm) != 4)
	  goto bad_alignment;
	alignbits = 3;
	break;
      default:
      bad_alignment:
	first_error (_("bad alignment"));
	return;
      }

  inst.instruction |= alignbits << 4;
  inst.instruction |= neon_logbits (et.size) << 6;

  /* Bits [4:6] of the immediate in a list specifier encode register stride
     (minus 1) in bit 4, and list length in bits [5:6]. We put the <n> of
     VLD<n>/VST<n> in bits [9:8] of the initial bitmask. Suck it out here, look
     up the right value for "type" in a table based on this value and the given
     list style, then stick it back.  */
  idx = ((inst.operands[0].imm >> 4) & 7)
	| (((inst.instruction >> 8) & 3) << 3);

  typebits = typetable[idx];

  constraint (typebits == -1, _("bad list type for instruction"));
  constraint (((inst.instruction >> 8) & 3) && et.size == 64,
	      BAD_EL_TYPE);

  inst.instruction &= ~0xf00;
  inst.instruction |= typebits << 8;
}

/* Check alignment is valid for do_neon_ld_st_lane and do_neon_ld_dup.
   *DO_ALIGN is set to 1 if the relevant alignment bit should be set, 0
   otherwise. The variable arguments are a list of pairs of legal (size, align)
   values, terminated with -1.  */

static int
neon_alignment_bit (int size, int align, int *do_alignment, ...)
{
  va_list ap;
  int result = FAIL, thissize, thisalign;

  if (!inst.operands[1].immisalign)
    {
      *do_alignment = 0;
      return SUCCESS;
    }

  va_start (ap, do_alignment);

  do
    {
      thissize = va_arg (ap, int);
      if (thissize == -1)
	break;
      thisalign = va_arg (ap, int);

      if (size == thissize && align == thisalign)
	result = SUCCESS;
    }
  while (result != SUCCESS);

  va_end (ap);

  if (result == SUCCESS)
    *do_alignment = 1;
  else
    first_error (_("unsupported alignment for instruction"));

  return result;
}

static void
do_neon_ld_st_lane (void)
{
  struct neon_type_el et = neon_check_type (1, NS_NULL, N_8 | N_16 | N_32);
  int align_good, do_alignment = 0;
  int logsize = neon_logbits (et.size);
  int align = inst.operands[1].imm >> 8;
  int n = (inst.instruction >> 8) & 3;
  int max_el = 64 / et.size;

  if (et.type == NT_invtype)
    return;

  constraint (NEON_REGLIST_LENGTH (inst.operands[0].imm) != n + 1,
	      _("bad list length"));
  constraint (NEON_LANE (inst.operands[0].imm) >= max_el,
	      _("scalar index out of range"));
  constraint (n != 0 && NEON_REG_STRIDE (inst.operands[0].imm) == 2
	      && et.size == 8,
	      _("stride of 2 unavailable when element size is 8"));

  switch (n)
    {
    case 0:  /* VLD1 / VST1.  */
      align_good = neon_alignment_bit (et.size, align, &do_alignment, 16, 16,
				       32, 32, -1);
      if (align_good == FAIL)
	return;
      if (do_alignment)
	{
	  unsigned alignbits = 0;
	  switch (et.size)
	    {
	    case 16: alignbits = 0x1; break;
	    case 32: alignbits = 0x3; break;
	    default: ;
	    }
	  inst.instruction |= alignbits << 4;
	}
      break;

    case 1:  /* VLD2 / VST2.  */
      align_good = neon_alignment_bit (et.size, align, &do_alignment, 8, 16,
		      16, 32, 32, 64, -1);
      if (align_good == FAIL)
	return;
      if (do_alignment)
	inst.instruction |= 1 << 4;
      break;

    case 2:  /* VLD3 / VST3.  */
      constraint (inst.operands[1].immisalign,
		  _("can't use alignment with this instruction"));
      break;

    case 3:  /* VLD4 / VST4.  */
      align_good = neon_alignment_bit (et.size, align, &do_alignment, 8, 32,
				       16, 64, 32, 64, 32, 128, -1);
      if (align_good == FAIL)
	return;
      if (do_alignment)
	{
	  unsigned alignbits = 0;
	  switch (et.size)
	    {
	    case 8:  alignbits = 0x1; break;
	    case 16: alignbits = 0x1; break;
	    case 32: alignbits = (align == 64) ? 0x1 : 0x2; break;
	    default: ;
	    }
	  inst.instruction |= alignbits << 4;
	}
      break;

    default: ;
    }

  /* Reg stride of 2 is encoded in bit 5 when size==16, bit 6 when size==32.  */
  if (n != 0 && NEON_REG_STRIDE (inst.operands[0].imm) == 2)
    inst.instruction |= 1 << (4 + logsize);

  inst.instruction |= NEON_LANE (inst.operands[0].imm) << (logsize + 5);
  inst.instruction |= logsize << 10;
}

/* Encode single n-element structure to all lanes VLD<n> instructions.  */

static void
do_neon_ld_dup (void)
{
  struct neon_type_el et = neon_check_type (1, NS_NULL, N_8 | N_16 | N_32);
  int align_good, do_alignment = 0;

  if (et.type == NT_invtype)
    return;

  switch ((inst.instruction >> 8) & 3)
    {
    case 0:  /* VLD1.  */
      gas_assert (NEON_REG_STRIDE (inst.operands[0].imm) != 2);
      align_good = neon_alignment_bit (et.size, inst.operands[1].imm >> 8,
				       &do_alignment, 16, 16, 32, 32, -1);
      if (align_good == FAIL)
	return;
      switch (NEON_REGLIST_LENGTH (inst.operands[0].imm))
	{
	case 1: break;
	case 2: inst.instruction |= 1 << 5; break;
	default: first_error (_("bad list length")); return;
	}
      inst.instruction |= neon_logbits (et.size) << 6;
      break;

    case 1:  /* VLD2.  */
      align_good = neon_alignment_bit (et.size, inst.operands[1].imm >> 8,
				       &do_alignment, 8, 16, 16, 32, 32, 64,
				       -1);
      if (align_good == FAIL)
	return;
      constraint (NEON_REGLIST_LENGTH (inst.operands[0].imm) != 2,
		  _("bad list length"));
      if (NEON_REG_STRIDE (inst.operands[0].imm) == 2)
	inst.instruction |= 1 << 5;
      inst.instruction |= neon_logbits (et.size) << 6;
      break;

    case 2:  /* VLD3.  */
      constraint (inst.operands[1].immisalign,
		  _("can't use alignment with this instruction"));
      constraint (NEON_REGLIST_LENGTH (inst.operands[0].imm) != 3,
		  _("bad list length"));
      if (NEON_REG_STRIDE (inst.operands[0].imm) == 2)
	inst.instruction |= 1 << 5;
      inst.instruction |= neon_logbits (et.size) << 6;
      break;

    case 3:  /* VLD4.  */
      {
	int align = inst.operands[1].imm >> 8;
	align_good = neon_alignment_bit (et.size, align, &do_alignment, 8, 32,
					 16, 64, 32, 64, 32, 128, -1);
	if (align_good == FAIL)
	  return;
	constraint (NEON_REGLIST_LENGTH (inst.operands[0].imm) != 4,
		    _("bad list length"));
	if (NEON_REG_STRIDE (inst.operands[0].imm) == 2)
	  inst.instruction |= 1 << 5;
	if (et.size == 32 && align == 128)
	  inst.instruction |= 0x3 << 6;
	else
	  inst.instruction |= neon_logbits (et.size) << 6;
      }
      break;

    default: ;
    }

  inst.instruction |= do_alignment << 4;
}

/* Disambiguate VLD<n> and VST<n> instructions, and fill in common bits (those
   apart from bits [11:4].  */

static void
do_neon_ldx_stx (void)
{
  if (inst.operands[1].isreg)
    constraint (inst.operands[1].reg == REG_PC, BAD_PC);

  switch (NEON_LANE (inst.operands[0].imm))
    {
    case NEON_INTERLEAVE_LANES:
      NEON_ENCODE (INTERLV, inst);
      do_neon_ld_st_interleave ();
      break;

    case NEON_ALL_LANES:
      NEON_ENCODE (DUP, inst);
      if (inst.instruction == N_INV)
	{
	  first_error ("only loads support such operands");
	  break;
	}
      do_neon_ld_dup ();
      break;

    default:
      NEON_ENCODE (LANE, inst);
      do_neon_ld_st_lane ();
    }

  /* L bit comes from bit mask.  */
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= inst.operands[1].reg << 16;

  if (inst.operands[1].postind)
    {
      int postreg = inst.operands[1].imm & 0xf;
      constraint (!inst.operands[1].immisreg,
		  _("post-index must be a register"));
      constraint (postreg == 0xd || postreg == 0xf,
		  _("bad register for post-index"));
      inst.instruction |= postreg;
    }
  else
    {
      constraint (inst.operands[1].immisreg, BAD_ADDR_MODE);
      constraint (inst.relocs[0].exp.X_op != O_constant
		  || inst.relocs[0].exp.X_add_number != 0,
		  BAD_ADDR_MODE);

      if (inst.operands[1].writeback)
	{
	  inst.instruction |= 0xd;
	}
      else
	inst.instruction |= 0xf;
    }

  if (thumb_mode)
    inst.instruction |= 0xf9000000;
  else
    inst.instruction |= 0xf4000000;
}

/* FP v8.  */
static void
do_vfp_nsyn_fpv8 (enum neon_shape rs)
{
  /* Targets like FPv5-SP-D16 don't support FP v8 instructions with
     D register operands.  */
  if (neon_shape_class[rs] == SC_DOUBLE)
    constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_armv8),
		_(BAD_FPU));

  NEON_ENCODE (FPV8, inst);

  if (rs == NS_FFF || rs == NS_HHH)
    {
      do_vfp_sp_dyadic ();

      /* ARMv8.2 fp16 instruction.  */
      if (rs == NS_HHH)
	do_scalar_fp16_v82_encode ();
    }
  else
    do_vfp_dp_rd_rn_rm ();

  if (rs == NS_DDD)
    inst.instruction |= 0x100;

  inst.instruction |= 0xf0000000;
}

static void
do_vsel (void)
{
  set_pred_insn_type (OUTSIDE_PRED_INSN);

  if (try_vfp_nsyn (3, do_vfp_nsyn_fpv8) != SUCCESS)
    first_error (_("invalid instruction shape"));
}

static void
do_vmaxnm (void)
{
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    set_pred_insn_type (OUTSIDE_PRED_INSN);

  if (try_vfp_nsyn (3, do_vfp_nsyn_fpv8) == SUCCESS)
    return;

  if (!check_simd_pred_availability (true, NEON_CHECK_CC | NEON_CHECK_ARCH8))
    return;

  neon_dyadic_misc (NT_untyped, N_F_16_32, 0);
}

static void
do_vrint_1 (enum neon_cvt_mode mode)
{
  enum neon_shape rs = neon_select_shape (NS_HH, NS_FF, NS_DD, NS_QQ, NS_NULL);
  struct neon_type_el et;

  if (rs == NS_NULL)
    return;

  /* Targets like FPv5-SP-D16 don't support FP v8 instructions with
     D register operands.  */
  if (neon_shape_class[rs] == SC_DOUBLE)
    constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_armv8),
		_(BAD_FPU));

  et = neon_check_type (2, rs, N_EQK | N_VFP, N_F_ALL | N_KEY
			| N_VFP);
  if (et.type != NT_invtype)
    {
      /* VFP encodings.  */
      if (mode == neon_cvt_mode_a || mode == neon_cvt_mode_n
	  || mode == neon_cvt_mode_p || mode == neon_cvt_mode_m)
	set_pred_insn_type (OUTSIDE_PRED_INSN);

      NEON_ENCODE (FPV8, inst);
      if (rs == NS_FF || rs == NS_HH)
	do_vfp_sp_monadic ();
      else
	do_vfp_dp_rd_rm ();

      switch (mode)
	{
	case neon_cvt_mode_r: inst.instruction |= 0x00000000; break;
	case neon_cvt_mode_z: inst.instruction |= 0x00000080; break;
	case neon_cvt_mode_x: inst.instruction |= 0x00010000; break;
	case neon_cvt_mode_a: inst.instruction |= 0xf0000000; break;
	case neon_cvt_mode_n: inst.instruction |= 0xf0010000; break;
	case neon_cvt_mode_p: inst.instruction |= 0xf0020000; break;
	case neon_cvt_mode_m: inst.instruction |= 0xf0030000; break;
	default: abort ();
	}

      inst.instruction |= (rs == NS_DD) << 8;
      do_vfp_cond_or_thumb ();

      /* ARMv8.2 fp16 vrint instruction.  */
      if (rs == NS_HH)
      do_scalar_fp16_v82_encode ();
    }
  else
    {
      /* Neon encodings (or something broken...).  */
      inst.error = NULL;
      et = neon_check_type (2, rs, N_EQK, N_F_16_32 | N_KEY);

      if (et.type == NT_invtype)
	return;

      if (!check_simd_pred_availability (true,
					 NEON_CHECK_CC | NEON_CHECK_ARCH8))
	return;

      NEON_ENCODE (FLOAT, inst);

      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= LOW4 (inst.operands[1].reg);
      inst.instruction |= HI1 (inst.operands[1].reg) << 5;
      inst.instruction |= neon_quad (rs) << 6;
      /* Mask off the original size bits and reencode them.  */
      inst.instruction = ((inst.instruction & 0xfff3ffff)
			  | neon_logbits (et.size) << 18);

      switch (mode)
	{
	case neon_cvt_mode_z: inst.instruction |= 3 << 7; break;
	case neon_cvt_mode_x: inst.instruction |= 1 << 7; break;
	case neon_cvt_mode_a: inst.instruction |= 2 << 7; break;
	case neon_cvt_mode_n: inst.instruction |= 0 << 7; break;
	case neon_cvt_mode_p: inst.instruction |= 7 << 7; break;
	case neon_cvt_mode_m: inst.instruction |= 5 << 7; break;
	case neon_cvt_mode_r: inst.error = _("invalid rounding mode"); break;
	default: abort ();
	}

      if (thumb_mode)
	inst.instruction |= 0xfc000000;
      else
	inst.instruction |= 0xf0000000;
    }
}

static void
do_vrintx (void)
{
  do_vrint_1 (neon_cvt_mode_x);
}

static void
do_vrintz (void)
{
  do_vrint_1 (neon_cvt_mode_z);
}

static void
do_vrintr (void)
{
  do_vrint_1 (neon_cvt_mode_r);
}

static void
do_vrinta (void)
{
  do_vrint_1 (neon_cvt_mode_a);
}

static void
do_vrintn (void)
{
  do_vrint_1 (neon_cvt_mode_n);
}

static void
do_vrintp (void)
{
  do_vrint_1 (neon_cvt_mode_p);
}

static void
do_vrintm (void)
{
  do_vrint_1 (neon_cvt_mode_m);
}

static unsigned
neon_scalar_for_vcmla (unsigned opnd, unsigned elsize)
{
  unsigned regno = NEON_SCALAR_REG (opnd);
  unsigned elno = NEON_SCALAR_INDEX (opnd);

  if (elsize == 16 && elno < 2 && regno < 16)
    return regno | (elno << 4);
  else if (elsize == 32 && elno == 0)
    return regno;

  first_error (_("scalar out of range"));
  return 0;
}

static void
do_vcmla (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext)
	      && (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_armv8)
		  || !mark_feature_used (&arm_ext_v8_3)), (BAD_FPU));
  constraint (inst.relocs[0].exp.X_op != O_constant,
	      _("expression too complex"));
  unsigned rot = inst.relocs[0].exp.X_add_number;
  constraint (rot != 0 && rot != 90 && rot != 180 && rot != 270,
	      _("immediate out of range"));
  rot /= 90;

  if (!check_simd_pred_availability (true,
				     NEON_CHECK_ARCH8 | NEON_CHECK_CC))
    return;

  if (inst.operands[2].isscalar)
    {
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext))
	first_error (_("invalid instruction shape"));
      enum neon_shape rs = neon_select_shape (NS_DDSI, NS_QQSI, NS_NULL);
      unsigned size = neon_check_type (3, rs, N_EQK, N_EQK,
				       N_KEY | N_F16 | N_F32).size;
      unsigned m = neon_scalar_for_vcmla (inst.operands[2].reg, size);
      inst.is_neon = 1;
      inst.instruction = 0xfe000800;
      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
      inst.instruction |= HI1 (inst.operands[1].reg) << 7;
      inst.instruction |= LOW4 (m);
      inst.instruction |= HI1 (m) << 5;
      inst.instruction |= neon_quad (rs) << 6;
      inst.instruction |= rot << 20;
      inst.instruction |= (size == 32) << 23;
    }
  else
    {
      enum neon_shape rs;
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext))
	rs = neon_select_shape (NS_QQQI, NS_NULL);
      else
	rs = neon_select_shape (NS_DDDI, NS_QQQI, NS_NULL);

      unsigned size = neon_check_type (3, rs, N_EQK, N_EQK,
				       N_KEY | N_F16 | N_F32).size;
      if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_fp_ext) && size == 32
	  && (inst.operands[0].reg == inst.operands[1].reg
	      || inst.operands[0].reg == inst.operands[2].reg))
	as_tsktsk (BAD_MVE_SRCDEST);

      neon_three_same (neon_quad (rs), 0, -1);
      inst.instruction &= 0x00ffffff; /* Undo neon_dp_fixup.  */
      inst.instruction |= 0xfc200800;
      inst.instruction |= rot << 23;
      inst.instruction |= (size == 32) << 20;
    }
}

static void
do_vcadd (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext)
	      && (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_armv8)
		  || !mark_feature_used (&arm_ext_v8_3)), (BAD_FPU));
  constraint (inst.relocs[0].exp.X_op != O_constant,
	      _("expression too complex"));

  unsigned rot = inst.relocs[0].exp.X_add_number;
  constraint (rot != 90 && rot != 270, _("immediate out of range"));
  enum neon_shape rs;
  struct neon_type_el et;
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
    {
      rs = neon_select_shape (NS_DDDI, NS_QQQI, NS_NULL);
      et = neon_check_type (3, rs, N_EQK, N_EQK, N_KEY | N_F16 | N_F32);
    }
  else
    {
      rs = neon_select_shape (NS_QQQI, NS_NULL);
      et = neon_check_type (3, rs, N_EQK, N_EQK, N_KEY | N_F16 | N_F32 | N_I8
			    | N_I16 | N_I32);
      if (et.size == 32 && inst.operands[0].reg == inst.operands[2].reg)
	as_tsktsk (_("Warning: 32-bit element size and same first and third "
		     "operand makes instruction UNPREDICTABLE"));
    }

  if (et.type == NT_invtype)
    return;

  if (!check_simd_pred_availability (et.type == NT_float,
				     NEON_CHECK_ARCH8 | NEON_CHECK_CC))
    return;

  if (et.type == NT_float)
    {
      neon_three_same (neon_quad (rs), 0, -1);
      inst.instruction &= 0x00ffffff; /* Undo neon_dp_fixup.  */
      inst.instruction |= 0xfc800800;
      inst.instruction |= (rot == 270) << 24;
      inst.instruction |= (et.size == 32) << 20;
    }
  else
    {
      constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext), BAD_FPU);
      inst.instruction = 0xfe000f00;
      inst.instruction |= HI1 (inst.operands[0].reg) << 22;
      inst.instruction |= neon_logbits (et.size) << 20;
      inst.instruction |= LOW4 (inst.operands[1].reg) << 16;
      inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
      inst.instruction |= (rot == 270) << 12;
      inst.instruction |= HI1 (inst.operands[1].reg) << 7;
      inst.instruction |= HI1 (inst.operands[2].reg) << 5;
      inst.instruction |= LOW4 (inst.operands[2].reg);
      inst.is_neon = 1;
    }
}

/* Dot Product instructions encoding support.  */

static void
do_neon_dotproduct (int unsigned_p)
{
  enum neon_shape rs;
  unsigned scalar_oprd2 = 0;
  int high8;

  if (inst.cond != COND_ALWAYS)
    as_warn (_("Dot Product instructions cannot be conditional,  the behaviour "
	       "is UNPREDICTABLE"));

  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_neon_ext_armv8),
	      _(BAD_FPU));

  /* Dot Product instructions are in three-same D/Q register format or the third
     operand can be a scalar index register.  */
  if (inst.operands[2].isscalar)
    {
      scalar_oprd2 = neon_scalar_for_mul (inst.operands[2].reg, 32);
      high8 = 0xfe000000;
      rs = neon_select_shape (NS_DDS, NS_QQS, NS_NULL);
    }
  else
    {
      high8 = 0xfc000000;
      rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
    }

  if (unsigned_p)
    neon_check_type (3, rs, N_EQK, N_EQK, N_KEY | N_U8);
  else
    neon_check_type (3, rs, N_EQK, N_EQK, N_KEY | N_S8);

  /* The "U" bit in traditional Three Same encoding is fixed to 0 for Dot
     Product instruction, so we pass 0 as the "ubit" parameter.  And the
     "Size" field are fixed to 0x2, so we pass 32 as the "size" parameter.  */
  neon_three_same (neon_quad (rs), 0, 32);

  /* Undo neon_dp_fixup.  Dot Product instructions are using a slightly
     different NEON three-same encoding.  */
  inst.instruction &= 0x00ffffff;
  inst.instruction |= high8;
  /* Encode 'U' bit which indicates signedness.  */
  inst.instruction |= (unsigned_p ? 1 : 0) << 4;
  /* Re-encode operand2 if it's indexed scalar operand.  What has been encoded
     from inst.operand[2].reg in neon_three_same is GAS's internal encoding, not
     the instruction encoding.  */
  if (inst.operands[2].isscalar)
    {
      inst.instruction &= 0xffffffd0;
      inst.instruction |= LOW4 (scalar_oprd2);
      inst.instruction |= HI1 (scalar_oprd2) << 5;
    }
}

/* Dot Product instructions for signed integer.  */

static void
do_neon_dotproduct_s (void)
{
  return do_neon_dotproduct (0);
}

/* Dot Product instructions for unsigned integer.  */

static void
do_neon_dotproduct_u (void)
{
  return do_neon_dotproduct (1);
}

static void
do_vusdot (void)
{
  enum neon_shape rs;
  set_pred_insn_type (OUTSIDE_PRED_INSN);
  if (inst.operands[2].isscalar)
    {
      rs = neon_select_shape (NS_DDS, NS_QQS, NS_NULL);
      neon_check_type (3, rs, N_EQK, N_EQK, N_S8 | N_KEY);

      inst.instruction |= (1 << 25);
      int idx = inst.operands[2].reg & 0xf;
      constraint ((idx != 1 && idx != 0), _("index must be 0 or 1"));
      inst.operands[2].reg >>= 4;
      constraint (!(inst.operands[2].reg < 16),
		  _("indexed register must be less than 16"));
      neon_three_args (rs == NS_QQS);
      inst.instruction |= (idx << 5);
    }
  else
    {
      inst.instruction |= (1 << 21);
      rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
      neon_check_type (3, rs, N_EQK, N_EQK, N_S8 | N_KEY);
      neon_three_args (rs == NS_QQQ);
    }
}

static void
do_vsudot (void)
{
  enum neon_shape rs;
  set_pred_insn_type (OUTSIDE_PRED_INSN);
  if (inst.operands[2].isscalar)
    {
      rs = neon_select_shape (NS_DDS, NS_QQS, NS_NULL);
      neon_check_type (3, rs, N_EQK, N_EQK, N_U8 | N_KEY);

      inst.instruction |= (1 << 25);
      int idx = inst.operands[2].reg & 0xf;
      constraint ((idx != 1 && idx != 0), _("index must be 0 or 1"));
      inst.operands[2].reg >>= 4;
      constraint (!(inst.operands[2].reg < 16),
		  _("indexed register must be less than 16"));
      neon_three_args (rs == NS_QQS);
      inst.instruction |= (idx << 5);
    }
}

static void
do_vsmmla (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQ, NS_NULL);
  neon_check_type (3, rs, N_EQK, N_EQK, N_S8 | N_KEY);

  set_pred_insn_type (OUTSIDE_PRED_INSN);

  neon_three_args (1);

}

static void
do_vummla (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQ, NS_NULL);
  neon_check_type (3, rs, N_EQK, N_EQK, N_U8 | N_KEY);

  set_pred_insn_type (OUTSIDE_PRED_INSN);

  neon_three_args (1);

}

static void
check_cde_operand (size_t idx, int is_dual)
{
  unsigned Rx = inst.operands[idx].reg;
  bool isvec = inst.operands[idx].isvec;
  if (is_dual == 0 && thumb_mode)
    constraint (
		!((Rx <= 14 && Rx != 13) || (Rx == REG_PC && isvec)),
		_("Register must be r0-r14 except r13, or APSR_nzcv."));
  else
    constraint ( !((Rx <= 10 && Rx % 2 == 0 )),
      _("Register must be an even register between r0-r10."));
}

static bool
cde_coproc_enabled (unsigned coproc)
{
  switch (coproc)
  {
    case 0: return mark_feature_used (&arm_ext_cde0);
    case 1: return mark_feature_used (&arm_ext_cde1);
    case 2: return mark_feature_used (&arm_ext_cde2);
    case 3: return mark_feature_used (&arm_ext_cde3);
    case 4: return mark_feature_used (&arm_ext_cde4);
    case 5: return mark_feature_used (&arm_ext_cde5);
    case 6: return mark_feature_used (&arm_ext_cde6);
    case 7: return mark_feature_used (&arm_ext_cde7);
    default: return false;
  }
}

#define cde_coproc_pos 8
static void
cde_handle_coproc (void)
{
  unsigned coproc = inst.operands[0].reg;
  constraint (coproc > 7, _("CDE Coprocessor must be in range 0-7"));
  constraint (!(cde_coproc_enabled (coproc)), BAD_CDE_COPROC);
  inst.instruction |= coproc << cde_coproc_pos;
}
#undef cde_coproc_pos

static void
cxn_handle_predication (bool is_accum)
{
  if (is_accum && conditional_insn ())
    set_pred_insn_type (INSIDE_IT_INSN);
  else if (conditional_insn ())
  /* conditional_insn essentially checks for a suffix, not whether the
     instruction is inside an IT block or not.
     The non-accumulator versions should not have suffixes.  */
    inst.error = BAD_SYNTAX;
  else
    set_pred_insn_type (OUTSIDE_PRED_INSN);
}

static void
do_custom_instruction_1 (int is_dual, bool is_accum)
{

  constraint (!mark_feature_used (&arm_ext_cde), _(BAD_CDE));

  unsigned imm, Rd;

  Rd = inst.operands[1].reg;
  check_cde_operand (1, is_dual);

  if (is_dual == 1)
    {
      constraint (inst.operands[2].reg != Rd + 1,
		  _("cx1d requires consecutive destination registers."));
      imm = inst.operands[3].imm;
    }
  else if (is_dual == 0)
    imm = inst.operands[2].imm;
  else
    abort ();

  inst.instruction |= Rd << 12;
  inst.instruction |= (imm & 0x1F80) << 9;
  inst.instruction |= (imm & 0x0040) << 1;
  inst.instruction |= (imm & 0x003f);

  cde_handle_coproc ();
  cxn_handle_predication (is_accum);
}

static void
do_custom_instruction_2 (int is_dual, bool is_accum)
{

  constraint (!mark_feature_used (&arm_ext_cde), _(BAD_CDE));

  unsigned imm, Rd, Rn;

  Rd = inst.operands[1].reg;

  if (is_dual == 1)
    {
      constraint (inst.operands[2].reg != Rd + 1,
		  _("cx2d requires consecutive destination registers."));
      imm = inst.operands[4].imm;
      Rn = inst.operands[3].reg;
    }
  else if (is_dual == 0)
  {
    imm = inst.operands[3].imm;
    Rn = inst.operands[2].reg;
  }
  else
    abort ();

  check_cde_operand (2 + is_dual, /* is_dual = */0);
  check_cde_operand (1, is_dual);

  inst.instruction |= Rd << 12;
  inst.instruction |= Rn << 16;

  inst.instruction |= (imm & 0x0380) << 13;
  inst.instruction |= (imm & 0x0040) << 1;
  inst.instruction |= (imm & 0x003f);

  cde_handle_coproc ();
  cxn_handle_predication (is_accum);
}

static void
do_custom_instruction_3 (int is_dual, bool is_accum)
{

  constraint (!mark_feature_used (&arm_ext_cde), _(BAD_CDE));

  unsigned imm, Rd, Rn, Rm;

  Rd = inst.operands[1].reg;

  if (is_dual == 1)
    {
      constraint (inst.operands[2].reg != Rd + 1,
		  _("cx3d requires consecutive destination registers."));
      imm = inst.operands[5].imm;
      Rn = inst.operands[3].reg;
      Rm = inst.operands[4].reg;
    }
  else if (is_dual == 0)
  {
    imm = inst.operands[4].imm;
    Rn = inst.operands[2].reg;
    Rm = inst.operands[3].reg;
  }
  else
    abort ();

  check_cde_operand (1, is_dual);
  check_cde_operand (2 + is_dual, /* is_dual = */0);
  check_cde_operand (3 + is_dual, /* is_dual = */0);

  inst.instruction |= Rd;
  inst.instruction |= Rn << 16;
  inst.instruction |= Rm << 12;

  inst.instruction |= (imm & 0x0038) << 17;
  inst.instruction |= (imm & 0x0004) << 5;
  inst.instruction |= (imm & 0x0003) << 4;

  cde_handle_coproc ();
  cxn_handle_predication (is_accum);
}

static void
do_cx1 (void)
{
  return do_custom_instruction_1 (0, 0);
}

static void
do_cx1a (void)
{
  return do_custom_instruction_1 (0, 1);
}

static void
do_cx1d (void)
{
  return do_custom_instruction_1 (1, 0);
}

static void
do_cx1da (void)
{
  return do_custom_instruction_1 (1, 1);
}

static void
do_cx2 (void)
{
  return do_custom_instruction_2 (0, 0);
}

static void
do_cx2a (void)
{
  return do_custom_instruction_2 (0, 1);
}

static void
do_cx2d (void)
{
  return do_custom_instruction_2 (1, 0);
}

static void
do_cx2da (void)
{
  return do_custom_instruction_2 (1, 1);
}

static void
do_cx3 (void)
{
  return do_custom_instruction_3 (0, 0);
}

static void
do_cx3a (void)
{
  return do_custom_instruction_3 (0, 1);
}

static void
do_cx3d (void)
{
  return do_custom_instruction_3 (1, 0);
}

static void
do_cx3da (void)
{
  return do_custom_instruction_3 (1, 1);
}

static void
vcx_assign_vec_d (unsigned regnum)
{
  inst.instruction |= HI4 (regnum) << 12;
  inst.instruction |= LOW1 (regnum) << 22;
}

static void
vcx_assign_vec_m (unsigned regnum)
{
  inst.instruction |= HI4 (regnum);
  inst.instruction |= LOW1 (regnum) << 5;
}

static void
vcx_assign_vec_n (unsigned regnum)
{
  inst.instruction |= HI4 (regnum) << 16;
  inst.instruction |= LOW1 (regnum) << 7;
}

enum vcx_reg_type {
    q_reg,
    d_reg,
    s_reg
};

static enum vcx_reg_type
vcx_get_reg_type (enum neon_shape ns)
{
  gas_assert (ns == NS_PQI
	      || ns == NS_PDI
	      || ns == NS_PFI
	      || ns == NS_PQQI
	      || ns == NS_PDDI
	      || ns == NS_PFFI
	      || ns == NS_PQQQI
	      || ns == NS_PDDDI
	      || ns == NS_PFFFI);
  if (ns == NS_PQI || ns == NS_PQQI || ns == NS_PQQQI)
    return q_reg;
  if (ns == NS_PDI || ns == NS_PDDI || ns == NS_PDDDI)
    return d_reg;
  return s_reg;
}

#define vcx_size_pos 24
#define vcx_vec_pos 6
static unsigned
vcx_handle_shape (enum vcx_reg_type reg_type)
{
  unsigned mult = 2;
  if (reg_type == q_reg)
    inst.instruction |= 1 << vcx_vec_pos;
  else if (reg_type == d_reg)
    inst.instruction |= 1 << vcx_size_pos;
  else
    mult = 1;
  /* NOTE:
     The documentation says that the Q registers are encoded as 2*N in the D:Vd
     bits (or equivalent for N and M registers).
     Similarly the D registers are encoded as N in D:Vd bits.
     While the S registers are encoded as N in the Vd:D bits.

     Taking into account the maximum values of these registers we can see a
     nicer pattern for calculation:
       Q -> 7, D -> 15, S -> 31

     If we say that everything is encoded in the Vd:D bits, then we can say
     that Q is encoded as 4*N, and D is encoded as 2*N.
     This way the bits will end up the same, and calculation is simpler.
     (calculation is now:
	1. Multiply by a number determined by the register letter.
	2. Encode resulting number in Vd:D bits.)

      This is made a little more complicated by automatic handling of 'Q'
      registers elsewhere, which means the register number is already 2*N where
      N is the number the user wrote after the register letter.
     */
  return mult;
}
#undef vcx_vec_pos
#undef vcx_size_pos

static void
vcx_ensure_register_in_range (unsigned R, enum vcx_reg_type reg_type)
{
  if (reg_type == q_reg)
    {
      gas_assert (R % 2 == 0);
      constraint (R >= 16, _("'q' register must be in range 0-7"));
    }
  else if (reg_type == d_reg)
    constraint (R >= 16, _("'d' register must be in range 0-15"));
  else
    constraint (R >= 32, _("'s' register must be in range 0-31"));
}

static void (*vcx_assign_vec[3]) (unsigned) = {
    vcx_assign_vec_d,
    vcx_assign_vec_m,
    vcx_assign_vec_n
};

static void
vcx_handle_register_arguments (unsigned num_registers,
			       enum vcx_reg_type reg_type)
{
  unsigned R, i;
  unsigned reg_mult = vcx_handle_shape (reg_type);
  for (i = 0; i < num_registers; i++)
    {
      R = inst.operands[i+1].reg;
      vcx_ensure_register_in_range (R, reg_type);
      if (num_registers == 3 && i > 0)
	{
	  if (i == 2)
	    vcx_assign_vec[1] (R * reg_mult);
	  else
	    vcx_assign_vec[2] (R * reg_mult);
	  continue;
	}
      vcx_assign_vec[i](R * reg_mult);
    }
}

static void
vcx_handle_insn_block (enum vcx_reg_type reg_type)
{
  if (reg_type == q_reg)
    if (inst.cond > COND_ALWAYS)
      inst.pred_insn_type = INSIDE_VPT_INSN;
    else
      inst.pred_insn_type = MVE_OUTSIDE_PRED_INSN;
  else if (inst.cond == COND_ALWAYS)
    inst.pred_insn_type = OUTSIDE_PRED_INSN;
  else
    inst.error = BAD_NOT_IT;
}

static void
vcx_handle_common_checks (unsigned num_args, enum neon_shape rs)
{
  constraint (!mark_feature_used (&arm_ext_cde), _(BAD_CDE));
  cde_handle_coproc ();
  enum vcx_reg_type reg_type = vcx_get_reg_type (rs);
  vcx_handle_register_arguments (num_args, reg_type);
  vcx_handle_insn_block (reg_type);
  if (reg_type == q_reg)
    constraint (!mark_feature_used (&mve_ext),
		_("vcx instructions with Q registers require MVE"));
  else
    constraint (!(ARM_FSET_CPU_SUBSET (armv8m_fp, cpu_variant)
		  && mark_feature_used (&armv8m_fp))
		&& !mark_feature_used (&mve_ext),
		_("vcx instructions with S or D registers require either MVE"
		  " or Armv8-M floating point extension."));
}

static void
do_vcx1 (void)
{
  enum neon_shape rs = neon_select_shape (NS_PQI, NS_PDI, NS_PFI, NS_NULL);
  vcx_handle_common_checks (1, rs);

  unsigned imm = inst.operands[2].imm;
  inst.instruction |= (imm & 0x03f);
  inst.instruction |= (imm & 0x040) << 1;
  inst.instruction |= (imm & 0x780) << 9;
  if (rs != NS_PQI)
    constraint (imm >= 2048,
		_("vcx1 with S or D registers takes immediate within 0-2047"));
  inst.instruction |= (imm & 0x800) << 13;
}

static void
do_vcx2 (void)
{
  enum neon_shape rs = neon_select_shape (NS_PQQI, NS_PDDI, NS_PFFI, NS_NULL);
  vcx_handle_common_checks (2, rs);

  unsigned imm = inst.operands[3].imm;
  inst.instruction |= (imm & 0x01) << 4;
  inst.instruction |= (imm & 0x02) << 6;
  inst.instruction |= (imm & 0x3c) << 14;
  if (rs != NS_PQQI)
    constraint (imm >= 64,
		_("vcx2 with S or D registers takes immediate within 0-63"));
  inst.instruction |= (imm & 0x40) << 18;
}

static void
do_vcx3 (void)
{
  enum neon_shape rs = neon_select_shape (NS_PQQQI, NS_PDDDI, NS_PFFFI, NS_NULL);
  vcx_handle_common_checks (3, rs);

  unsigned imm = inst.operands[4].imm;
  inst.instruction |= (imm & 0x1) << 4;
  inst.instruction |= (imm & 0x6) << 19;
  if (rs != NS_PQQQI)
    constraint (imm >= 8,
		_("vcx2 with S or D registers takes immediate within 0-7"));
  inst.instruction |= (imm & 0x8) << 21;
}

/* Crypto v1 instructions.  */
static void
do_crypto_2op_1 (unsigned elttype, int op)
{
  set_pred_insn_type (OUTSIDE_PRED_INSN);

  if (neon_check_type (2, NS_QQ, N_EQK | N_UNT, elttype | N_UNT | N_KEY).type
      == NT_invtype)
    return;

  inst.error = NULL;

  NEON_ENCODE (INTEGER, inst);
  inst.instruction |= LOW4 (inst.operands[0].reg) << 12;
  inst.instruction |= HI1 (inst.operands[0].reg) << 22;
  inst.instruction |= LOW4 (inst.operands[1].reg);
  inst.instruction |= HI1 (inst.operands[1].reg) << 5;
  if (op != -1)
    inst.instruction |= op << 6;

  if (thumb_mode)
    inst.instruction |= 0xfc000000;
  else
    inst.instruction |= 0xf0000000;
}

static void
do_crypto_3op_1 (int u, int op)
{
  set_pred_insn_type (OUTSIDE_PRED_INSN);

  if (neon_check_type (3, NS_QQQ, N_EQK | N_UNT, N_EQK | N_UNT,
		       N_32 | N_UNT | N_KEY).type == NT_invtype)
    return;

  inst.error = NULL;

  NEON_ENCODE (INTEGER, inst);
  neon_three_same (1, u, 8 << op);
}

static void
do_aese (void)
{
  do_crypto_2op_1 (N_8, 0);
}

static void
do_aesd (void)
{
  do_crypto_2op_1 (N_8, 1);
}

static void
do_aesmc (void)
{
  do_crypto_2op_1 (N_8, 2);
}

static void
do_aesimc (void)
{
  do_crypto_2op_1 (N_8, 3);
}

static void
do_sha1c (void)
{
  do_crypto_3op_1 (0, 0);
}

static void
do_sha1p (void)
{
  do_crypto_3op_1 (0, 1);
}

static void
do_sha1m (void)
{
  do_crypto_3op_1 (0, 2);
}

static void
do_sha1su0 (void)
{
  do_crypto_3op_1 (0, 3);
}

static void
do_sha256h (void)
{
  do_crypto_3op_1 (1, 0);
}

static void
do_sha256h2 (void)
{
  do_crypto_3op_1 (1, 1);
}

static void
do_sha256su1 (void)
{
  do_crypto_3op_1 (1, 2);
}

static void
do_sha1h (void)
{
  do_crypto_2op_1 (N_32, -1);
}

static void
do_sha1su1 (void)
{
  do_crypto_2op_1 (N_32, 0);
}

static void
do_sha256su0 (void)
{
  do_crypto_2op_1 (N_32, 1);
}

static void
do_crc32_1 (unsigned int poly, unsigned int sz)
{
  unsigned int Rd = inst.operands[0].reg;
  unsigned int Rn = inst.operands[1].reg;
  unsigned int Rm = inst.operands[2].reg;

  set_pred_insn_type (OUTSIDE_PRED_INSN);
  inst.instruction |= LOW4 (Rd) << (thumb_mode ? 8 : 12);
  inst.instruction |= LOW4 (Rn) << 16;
  inst.instruction |= LOW4 (Rm);
  inst.instruction |= sz << (thumb_mode ? 4 : 21);
  inst.instruction |= poly << (thumb_mode ? 20 : 9);

  if (Rd == REG_PC || Rn == REG_PC || Rm == REG_PC)
    as_warn (UNPRED_REG ("r15"));
}

static void
do_crc32b (void)
{
  do_crc32_1 (0, 0);
}

static void
do_crc32h (void)
{
  do_crc32_1 (0, 1);
}

static void
do_crc32w (void)
{
  do_crc32_1 (0, 2);
}

static void
do_crc32cb (void)
{
  do_crc32_1 (1, 0);
}

static void
do_crc32ch (void)
{
  do_crc32_1 (1, 1);
}

static void
do_crc32cw (void)
{
  do_crc32_1 (1, 2);
}

static void
do_vjcvt (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_vfp_ext_armv8),
	      _(BAD_FPU));
  neon_check_type (2, NS_FD, N_S32, N_F64);
  do_vfp_sp_dp_cvt ();
  do_vfp_cond_or_thumb ();
}

static void
do_vdot (void)
{
  enum neon_shape rs;
  constraint (!mark_feature_used (&fpu_neon_ext_armv8), _(BAD_FPU));
  set_pred_insn_type (OUTSIDE_PRED_INSN);
  if (inst.operands[2].isscalar)
    {
      rs = neon_select_shape (NS_DDS, NS_QQS, NS_NULL);
      neon_check_type (3, rs, N_EQK, N_EQK, N_BF16 | N_KEY);

      inst.instruction |= (1 << 25);
      int idx = inst.operands[2].reg & 0xf;
      constraint ((idx != 1 && idx != 0), _("index must be 0 or 1"));
      inst.operands[2].reg >>= 4;
      constraint (!(inst.operands[2].reg < 16),
		  _("indexed register must be less than 16"));
      neon_three_args (rs == NS_QQS);
      inst.instruction |= (idx << 5);
    }
  else
    {
      rs = neon_select_shape (NS_DDD, NS_QQQ, NS_NULL);
      neon_check_type (3, rs, N_EQK, N_EQK, N_BF16 | N_KEY);
      neon_three_args (rs == NS_QQQ);
    }
}

static void
do_vmmla (void)
{
  enum neon_shape rs = neon_select_shape (NS_QQQ, NS_NULL);
  neon_check_type (3, rs, N_EQK, N_EQK, N_BF16 | N_KEY);

  constraint (!mark_feature_used (&fpu_neon_ext_armv8), _(BAD_FPU));
  set_pred_insn_type (OUTSIDE_PRED_INSN);

  neon_three_args (1);
}

static void
do_t_pacbti (void)
{
  inst.instruction = THUMB_OP32 (inst.instruction);
}

static void
do_t_pacbti_nonop (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, pacbti_ext),
	      _(BAD_PACBTI));

  inst.instruction = THUMB_OP32 (inst.instruction);
  inst.instruction |= inst.operands[0].reg << 12;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
}

static void
do_t_pacbti_pacg (void)
{
  constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, pacbti_ext),
	      _(BAD_PACBTI));

  inst.instruction = THUMB_OP32 (inst.instruction);
  inst.instruction |= inst.operands[0].reg << 8;
  inst.instruction |= inst.operands[1].reg << 16;
  inst.instruction |= inst.operands[2].reg;
}


/* Overall per-instruction processing.	*/

/* We need to be able to fix up arbitrary expressions in some statements.
   This is so that we can handle symbols that are an arbitrary distance from
   the pc.  The most common cases are of the form ((+/-sym -/+ . - 8) & mask),
   which returns part of an address in a form which will be valid for
   a data instruction.	We do this by pushing the expression into a symbol
   in the expr_section, and creating a fix for that.  */

static void
fix_new_arm (fragS *	   frag,
	     int	   where,
	     short int	   size,
	     expressionS * exp,
	     int	   pc_rel,
	     int	   reloc)
{
  fixS *	   new_fix;

  switch (exp->X_op)
    {
    case O_constant:
      if (pc_rel)
	{
	  /* Create an absolute valued symbol, so we have something to
	     refer to in the object file.  Unfortunately for us, gas's
	     generic expression parsing will already have folded out
	     any use of .set foo/.type foo %function that may have
	     been used to set type information of the target location,
	     that's being specified symbolically.  We have to presume
	     the user knows what they are doing.  */
	  char name[16 + 8];
	  symbolS *symbol;

	  sprintf (name, "*ABS*0x%lx", (unsigned long)exp->X_add_number);

	  symbol = symbol_find_or_make (name);
	  S_SET_SEGMENT (symbol, absolute_section);
	  symbol_set_frag (symbol, &zero_address_frag);
	  S_SET_VALUE (symbol, exp->X_add_number);
	  exp->X_op = O_symbol;
	  exp->X_add_symbol = symbol;
	  exp->X_add_number = 0;
	}
      /* FALLTHROUGH */
    case O_symbol:
    case O_add:
    case O_subtract:
      new_fix = fix_new_exp (frag, where, size, exp, pc_rel,
			     (enum bfd_reloc_code_real) reloc);
      break;

    default:
      new_fix = (fixS *) fix_new (frag, where, size, make_expr_symbol (exp), 0,
				  pc_rel, (enum bfd_reloc_code_real) reloc);
      break;
    }

  /* Mark whether the fix is to a THUMB instruction, or an ARM
     instruction.  */
  new_fix->tc_fix_data = thumb_mode;
}

/* Create a frg for an instruction requiring relaxation.  */
static void
output_relax_insn (void)
{
  char * to;
  symbolS *sym;
  int offset;

  /* The size of the instruction is unknown, so tie the debug info to the
     start of the instruction.  */
  dwarf2_emit_insn (0);

  switch (inst.relocs[0].exp.X_op)
    {
    case O_symbol:
      sym = inst.relocs[0].exp.X_add_symbol;
      offset = inst.relocs[0].exp.X_add_number;
      break;
    case O_constant:
      sym = NULL;
      offset = inst.relocs[0].exp.X_add_number;
      break;
    default:
      sym = make_expr_symbol (&inst.relocs[0].exp);
      offset = 0;
      break;
  }
  to = frag_var (rs_machine_dependent, INSN_SIZE, THUMB_SIZE,
		 inst.relax, sym, offset, NULL/*offset, opcode*/);
  md_number_to_chars (to, inst.instruction, THUMB_SIZE);
}

/* Write a 32-bit thumb instruction to buf.  */
static void
put_thumb32_insn (char * buf, unsigned long insn)
{
  md_number_to_chars (buf, insn >> 16, THUMB_SIZE);
  md_number_to_chars (buf + THUMB_SIZE, insn, THUMB_SIZE);
}

static void
output_inst (const char * str)
{
  char * to = NULL;

  if (inst.error)
    {
      as_bad ("%s -- `%s'", inst.error, str);
      return;
    }
  if (inst.relax)
    {
      output_relax_insn ();
      return;
    }
  if (inst.size == 0)
    return;

  to = frag_more (inst.size);
  /* PR 9814: Record the thumb mode into the current frag so that we know
     what type of NOP padding to use, if necessary.  We override any previous
     setting so that if the mode has changed then the NOPS that we use will
     match the encoding of the last instruction in the frag.  */
  frag_now->tc_frag_data.thumb_mode = thumb_mode | MODE_RECORDED;

  if (thumb_mode && (inst.size > THUMB_SIZE))
    {
      gas_assert (inst.size == (2 * THUMB_SIZE));
      put_thumb32_insn (to, inst.instruction);
    }
  else if (inst.size > INSN_SIZE)
    {
      gas_assert (inst.size == (2 * INSN_SIZE));
      md_number_to_chars (to, inst.instruction, INSN_SIZE);
      md_number_to_chars (to + INSN_SIZE, inst.instruction, INSN_SIZE);
    }
  else
    md_number_to_chars (to, inst.instruction, inst.size);

  int r;
  for (r = 0; r < ARM_IT_MAX_RELOCS; r++)
    {
      if (inst.relocs[r].type != BFD_RELOC_UNUSED)
	fix_new_arm (frag_now, to - frag_now->fr_literal,
		     inst.size, & inst.relocs[r].exp, inst.relocs[r].pc_rel,
		     inst.relocs[r].type);
    }

  dwarf2_emit_insn (inst.size);
}

static char *
output_it_inst (int cond, int mask, char * to)
{
  unsigned long instruction = 0xbf00;

  mask &= 0xf;
  instruction |= mask;
  instruction |= cond << 4;

  if (to == NULL)
    {
      to = frag_more (2);
#ifdef OBJ_ELF
      dwarf2_emit_insn (2);
#endif
    }

  md_number_to_chars (to, instruction, 2);

  return to;
}

/* Tag values used in struct asm_opcode's tag field.  */
enum opcode_tag
{
  OT_unconditional,	/* Instruction cannot be conditionalized.
			   The ARM condition field is still 0xE.  */
  OT_unconditionalF,	/* Instruction cannot be conditionalized
			   and carries 0xF in its ARM condition field.  */
  OT_csuffix,		/* Instruction takes a conditional suffix.  */
  OT_csuffixF,		/* Some forms of the instruction take a scalar
			   conditional suffix, others place 0xF where the
			   condition field would be, others take a vector
			   conditional suffix.  */
  OT_cinfix3,		/* Instruction takes a conditional infix,
			   beginning at character index 3.  (In
			   unified mode, it becomes a suffix.)  */
  OT_cinfix3_deprecated, /* The same as OT_cinfix3.  This is used for
			    tsts, cmps, cmns, and teqs. */
  OT_cinfix3_legacy,	/* Legacy instruction takes a conditional infix at
			   character index 3, even in unified mode.  Used for
			   legacy instructions where suffix and infix forms
			   may be ambiguous.  */
  OT_csuf_or_in3,	/* Instruction takes either a conditional
			   suffix or an infix at character index 3.  */
  OT_odd_infix_unc,	/* This is the unconditional variant of an
			   instruction that takes a conditional infix
			   at an unusual position.  In unified mode,
			   this variant will accept a suffix.  */
  OT_odd_infix_0	/* Values greater than or equal to OT_odd_infix_0
			   are the conditional variants of instructions that
			   take conditional infixes in unusual positions.
			   The infix appears at character index
			   (tag - OT_odd_infix_0).  These are not accepted
			   in unified mode.  */
};

/* Subroutine of md_assemble, responsible for looking up the primary
   opcode from the mnemonic the user wrote.  STR points to the
   beginning of the mnemonic.

   This is not simply a hash table lookup, because of conditional
   variants.  Most instructions have conditional variants, which are
   expressed with a _conditional affix_ to the mnemonic.  If we were
   to encode each conditional variant as a literal string in the opcode
   table, it would have approximately 20,000 entries.

   Most mnemonics take this affix as a suffix, and in unified syntax,
   'most' is upgraded to 'all'.  However, in the divided syntax, some
   instructions take the affix as an infix, notably the s-variants of
   the arithmetic instructions.  Of those instructions, all but six
   have the infix appear after the third character of the mnemonic.

   Accordingly, the algorithm for looking up primary opcodes given
   an identifier is:

   1. Look up the identifier in the opcode table.
      If we find a match, go to step U.

   2. Look up the last two characters of the identifier in the
      conditions table.  If we find a match, look up the first N-2
      characters of the identifier in the opcode table.  If we
      find a match, go to step CE.

   3. Look up the fourth and fifth characters of the identifier in
      the conditions table.  If we find a match, extract those
      characters from the identifier, and look up the remaining
      characters in the opcode table.  If we find a match, go
      to step CM.

   4. Fail.

   U. Examine the tag field of the opcode structure, in case this is
      one of the six instructions with its conditional infix in an
      unusual place.  If it is, the tag tells us where to find the
      infix; look it up in the conditions table and set inst.cond
      accordingly.  Otherwise, this is an unconditional instruction.
      Again set inst.cond accordingly.  Return the opcode structure.

  CE. Examine the tag field to make sure this is an instruction that
      should receive a conditional suffix.  If it is not, fail.
      Otherwise, set inst.cond from the suffix we already looked up,
      and return the opcode structure.

  CM. Examine the tag field to make sure this is an instruction that
      should receive a conditional infix after the third character.
      If it is not, fail.  Otherwise, undo the edits to the current
      line of input and proceed as for case CE.  */

static const struct asm_opcode *
opcode_lookup (char **str)
{
  char *end, *base;
  char *affix;
  const struct asm_opcode *opcode;
  const struct asm_cond *cond;
  char save[2];

  /* Scan up to the end of the mnemonic, which must end in white space,
     '.' (in unified mode, or for Neon/VFP instructions), or end of string.  */
  for (base = end = *str; *end != '\0'; end++)
    if (*end == ' ' || *end == '.')
      break;

  if (end == base)
    return NULL;

  /* Handle a possible width suffix and/or Neon type suffix.  */
  if (end[0] == '.')
    {
      int offset = 2;

      /* The .w and .n suffixes are only valid if the unified syntax is in
	 use.  */
      if (unified_syntax && end[1] == 'w')
	inst.size_req = 4;
      else if (unified_syntax && end[1] == 'n')
	inst.size_req = 2;
      else
	offset = 0;

      inst.vectype.elems = 0;

      *str = end + offset;

      if (end[offset] == '.')
	{
	  /* See if we have a Neon type suffix (possible in either unified or
	     non-unified ARM syntax mode).  */
	  if (parse_neon_type (&inst.vectype, str) == FAIL)
	    return NULL;
	}
      else if (end[offset] != '\0' && end[offset] != ' ')
	return NULL;
    }
  else
    *str = end;

  /* Look for unaffixed or special-case affixed mnemonic.  */
  opcode = (const struct asm_opcode *) str_hash_find_n (arm_ops_hsh, base,
							end - base);
  cond = NULL;
  if (opcode)
    {
      /* step U */
      if (opcode->tag < OT_odd_infix_0)
	{
	  inst.cond = COND_ALWAYS;
	  return opcode;
	}

      if (warn_on_deprecated && unified_syntax)
	as_tsktsk (_("conditional infixes are deprecated in unified syntax"));
      affix = base + (opcode->tag - OT_odd_infix_0);
      cond = (const struct asm_cond *) str_hash_find_n (arm_cond_hsh, affix, 2);
      gas_assert (cond);

      inst.cond = cond->value;
      return opcode;
    }
 if (ARM_CPU_HAS_FEATURE (cpu_variant, mve_ext))
   {
    /* Cannot have a conditional suffix on a mnemonic of less than a character.
     */
    if (end - base < 2)
      return NULL;
     affix = end - 1;
     cond = (const struct asm_cond *) str_hash_find_n (arm_vcond_hsh, affix, 1);
     opcode = (const struct asm_opcode *) str_hash_find_n (arm_ops_hsh, base,
							   affix - base);

     /* A known edge case is a conflict between an 'e' as a suffix for an
	Else of a VPT predication block and an 'ne' suffix for an IT block.
	If we detect that edge case here and we are not in a VPT VECTOR_PRED
	block, reset opcode and cond, so that the 'ne' case can be detected
	in the next section for 2-character conditional suffixes.
	An example where this is a problem is between the MVE 'vcvtn' and the
	non-MVE 'vcvt' instructions. */
     if (cond && opcode
	 && cond->template_name[0] == 'e'
	 && opcode->template_name[affix - base - 1] == 'n'
	 && now_pred.type != VECTOR_PRED)
       {
	 opcode = NULL;
	 cond = NULL;
       }

     /* If this opcode can not be vector predicated then don't accept it with a
	vector predication code.  */
     if (opcode && !opcode->mayBeVecPred)
       opcode = NULL;
   }
  if (!opcode || !cond)
    {
      /* Cannot have a conditional suffix on a mnemonic of less than two
	 characters.  */
      if (end - base < 3)
	return NULL;

      /* Look for suffixed mnemonic.  */
      affix = end - 2;
      cond = (const struct asm_cond *) str_hash_find_n (arm_cond_hsh, affix, 2);
      opcode = (const struct asm_opcode *) str_hash_find_n (arm_ops_hsh, base,
							    affix - base);
    }

  if (opcode && cond)
    {
      /* step CE */
      switch (opcode->tag)
	{
	case OT_cinfix3_legacy:
	  /* Ignore conditional suffixes matched on infix only mnemonics.  */
	  break;

	case OT_cinfix3:
	case OT_cinfix3_deprecated:
	case OT_odd_infix_unc:
	  if (!unified_syntax)
	    return NULL;
	  /* Fall through.  */

	case OT_csuffix:
	case OT_csuffixF:
	case OT_csuf_or_in3:
	  inst.cond = cond->value;
	  return opcode;

	case OT_unconditional:
	case OT_unconditionalF:
	  if (thumb_mode)
	    inst.cond = cond->value;
	  else
	    {
	      /* Delayed diagnostic.  */
	      inst.error = BAD_COND;
	      inst.cond = COND_ALWAYS;
	    }
	  return opcode;

	default:
	  return NULL;
	}
    }

  /* Cannot have a usual-position infix on a mnemonic of less than
     six characters (five would be a suffix).  */
  if (end - base < 6)
    return NULL;

  /* Look for infixed mnemonic in the usual position.  */
  affix = base + 3;
  cond = (const struct asm_cond *) str_hash_find_n (arm_cond_hsh, affix, 2);
  if (!cond)
    return NULL;

  memcpy (save, affix, 2);
  memmove (affix, affix + 2, (end - affix) - 2);
  opcode = (const struct asm_opcode *) str_hash_find_n (arm_ops_hsh, base,
							(end - base) - 2);
  memmove (affix + 2, affix, (end - affix) - 2);
  memcpy (affix, save, 2);

  if (opcode
      && (opcode->tag == OT_cinfix3
	  || opcode->tag == OT_cinfix3_deprecated
	  || opcode->tag == OT_csuf_or_in3
	  || opcode->tag == OT_cinfix3_legacy))
    {
      /* Step CM.  */
      if (warn_on_deprecated && unified_syntax
	  && (opcode->tag == OT_cinfix3
	      || opcode->tag == OT_cinfix3_deprecated))
	as_tsktsk (_("conditional infixes are deprecated in unified syntax"));

      inst.cond = cond->value;
      return opcode;
    }

  return NULL;
}

/* This function generates an initial IT instruction, leaving its block
   virtually open for the new instructions. Eventually,
   the mask will be updated by now_pred_add_mask () each time
   a new instruction needs to be included in the IT block.
   Finally, the block is closed with close_automatic_it_block ().
   The block closure can be requested either from md_assemble (),
   a tencode (), or due to a label hook.  */

static void
new_automatic_it_block (int cond)
{
  now_pred.state = AUTOMATIC_PRED_BLOCK;
  now_pred.mask = 0x18;
  now_pred.cc = cond;
  now_pred.block_length = 1;
  mapping_state (MAP_THUMB);
  now_pred.insn = output_it_inst (cond, now_pred.mask, NULL);
  now_pred.warn_deprecated = false;
  now_pred.insn_cond = true;
}

/* Close an automatic IT block.
   See comments in new_automatic_it_block ().  */

static void
close_automatic_it_block (void)
{
  now_pred.mask = 0x10;
  now_pred.block_length = 0;
}

/* Update the mask of the current automatically-generated IT
   instruction. See comments in new_automatic_it_block ().  */

static void
now_pred_add_mask (int cond)
{
#define CLEAR_BIT(value, nbit)  ((value) & ~(1 << (nbit)))
#define SET_BIT_VALUE(value, bitvalue, nbit)  (CLEAR_BIT (value, nbit) \
					      | ((bitvalue) << (nbit)))
  const int resulting_bit = (cond & 1);

  now_pred.mask &= 0xf;
  now_pred.mask = SET_BIT_VALUE (now_pred.mask,
				   resulting_bit,
				  (5 - now_pred.block_length));
  now_pred.mask = SET_BIT_VALUE (now_pred.mask,
				   1,
				   ((5 - now_pred.block_length) - 1));
  output_it_inst (now_pred.cc, now_pred.mask, now_pred.insn);

#undef CLEAR_BIT
#undef SET_BIT_VALUE
}

/* The IT blocks handling machinery is accessed through the these functions:
     it_fsm_pre_encode ()               from md_assemble ()
     set_pred_insn_type ()		optional, from the tencode functions
     set_pred_insn_type_last ()		ditto
     in_pred_block ()			ditto
     it_fsm_post_encode ()              from md_assemble ()
     force_automatic_it_block_close ()  from label handling functions

   Rationale:
     1) md_assemble () calls it_fsm_pre_encode () before calling tencode (),
	initializing the IT insn type with a generic initial value depending
	on the inst.condition.
     2) During the tencode function, two things may happen:
	a) The tencode function overrides the IT insn type by
	   calling either set_pred_insn_type (type) or
	   set_pred_insn_type_last ().
	b) The tencode function queries the IT block state by
	   calling in_pred_block () (i.e. to determine narrow/not narrow mode).

	Both set_pred_insn_type and in_pred_block run the internal FSM state
	handling function (handle_pred_state), because: a) setting the IT insn
	type may incur in an invalid state (exiting the function),
	and b) querying the state requires the FSM to be updated.
	Specifically we want to avoid creating an IT block for conditional
	branches, so it_fsm_pre_encode is actually a guess and we can't
	determine whether an IT block is required until the tencode () routine
	has decided what type of instruction this actually it.
	Because of this, if set_pred_insn_type and in_pred_block have to be
	used, set_pred_insn_type has to be called first.

	set_pred_insn_type_last () is a wrapper of set_pred_insn_type (type),
	that determines the insn IT type depending on the inst.cond code.
	When a tencode () routine encodes an instruction that can be
	either outside an IT block, or, in the case of being inside, has to be
	the last one, set_pred_insn_type_last () will determine the proper
	IT instruction type based on the inst.cond code. Otherwise,
	set_pred_insn_type can be called for overriding that logic or
	for covering other cases.

	Calling handle_pred_state () may not transition the IT block state to
	OUTSIDE_PRED_BLOCK immediately, since the (current) state could be
	still queried. Instead, if the FSM determines that the state should
	be transitioned to OUTSIDE_PRED_BLOCK, a flag is marked to be closed
	after the tencode () function: that's what it_fsm_post_encode () does.

	Since in_pred_block () calls the state handling function to get an
	updated state, an error may occur (due to invalid insns combination).
	In that case, inst.error is set.
	Therefore, inst.error has to be checked after the execution of
	the tencode () routine.

     3) Back in md_assemble(), it_fsm_post_encode () is called to commit
	any pending state change (if any) that didn't take place in
	handle_pred_state () as explained above.  */

static void
it_fsm_pre_encode (void)
{
  if (inst.cond != COND_ALWAYS)
    inst.pred_insn_type =  INSIDE_IT_INSN;
  else
    inst.pred_insn_type = OUTSIDE_PRED_INSN;

  now_pred.state_handled = 0;
}

/* IT state FSM handling function.  */
/* MVE instructions and non-MVE instructions are handled differently because of
   the introduction of VPT blocks.
   Specifications say that any non-MVE instruction inside a VPT block is
   UNPREDICTABLE, with the exception of the BKPT instruction.  Whereas most MVE
   instructions are deemed to be UNPREDICTABLE if inside an IT block.  For the
   few exceptions we have MVE_UNPREDICABLE_INSN.
   The error messages provided depending on the different combinations possible
   are described in the cases below:
   For 'most' MVE instructions:
   1) In an IT block, with an IT code: syntax error
   2) In an IT block, with a VPT code: error: must be in a VPT block
   3) In an IT block, with no code: warning: UNPREDICTABLE
   4) In a VPT block, with an IT code: syntax error
   5) In a VPT block, with a VPT code: OK!
   6) In a VPT block, with no code: error: missing code
   7) Outside a pred block, with an IT code: error: syntax error
   8) Outside a pred block, with a VPT code: error: should be in a VPT block
   9) Outside a pred block, with no code: OK!
   For non-MVE instructions:
   10) In an IT block, with an IT code: OK!
   11) In an IT block, with a VPT code: syntax error
   12) In an IT block, with no code: error: missing code
   13) In a VPT block, with an IT code: error: should be in an IT block
   14) In a VPT block, with a VPT code: syntax error
   15) In a VPT block, with no code: UNPREDICTABLE
   16) Outside a pred block, with an IT code: error: should be in an IT block
   17) Outside a pred block, with a VPT code: syntax error
   18) Outside a pred block, with no code: OK!
 */


static int
handle_pred_state (void)
{
  now_pred.state_handled = 1;
  now_pred.insn_cond = false;

  switch (now_pred.state)
    {
    case OUTSIDE_PRED_BLOCK:
      switch (inst.pred_insn_type)
	{
	case MVE_UNPREDICABLE_INSN:
	case MVE_OUTSIDE_PRED_INSN:
	  if (inst.cond < COND_ALWAYS)
	    {
	      /* Case 7: Outside a pred block, with an IT code: error: syntax
		 error.  */
	      inst.error = BAD_SYNTAX;
	      return FAIL;
	    }
	  /* Case 9:  Outside a pred block, with no code: OK!  */
	  break;
	case OUTSIDE_PRED_INSN:
	  if (inst.cond > COND_ALWAYS)
	    {
	      /* Case 17:  Outside a pred block, with a VPT code: syntax error.
	       */
	      inst.error = BAD_SYNTAX;
	      return FAIL;
	    }
	  /* Case 18: Outside a pred block, with no code: OK!  */
	  break;

	case INSIDE_VPT_INSN:
	  /* Case 8: Outside a pred block, with a VPT code: error: should be in
	     a VPT block.  */
	  inst.error = BAD_OUT_VPT;
	  return FAIL;

	case INSIDE_IT_INSN:
	case INSIDE_IT_LAST_INSN:
	  if (inst.cond < COND_ALWAYS)
	    {
	      /* Case 16: Outside a pred block, with an IT code: error: should
		 be in an IT block.  */
	      if (thumb_mode == 0)
		{
		  if (unified_syntax
		      && !(implicit_it_mode & IMPLICIT_IT_MODE_ARM))
		    as_tsktsk (_("Warning: conditional outside an IT block"\
				 " for Thumb."));
		}
	      else
		{
		  if ((implicit_it_mode & IMPLICIT_IT_MODE_THUMB)
		      && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2))
		    {
		      /* Automatically generate the IT instruction.  */
		      new_automatic_it_block (inst.cond);
		      if (inst.pred_insn_type == INSIDE_IT_LAST_INSN)
			close_automatic_it_block ();
		    }
		  else
		    {
		      inst.error = BAD_OUT_IT;
		      return FAIL;
		    }
		}
	      break;
	    }
	  else if (inst.cond > COND_ALWAYS)
	    {
	      /* Case 17: Outside a pred block, with a VPT code: syntax error.
	       */
	      inst.error = BAD_SYNTAX;
	      return FAIL;
	    }
	  else
	    gas_assert (0);
	case IF_INSIDE_IT_LAST_INSN:
	case NEUTRAL_IT_INSN:
	  break;

	case VPT_INSN:
	  if (inst.cond != COND_ALWAYS)
	    first_error (BAD_SYNTAX);
	  now_pred.state = MANUAL_PRED_BLOCK;
	  now_pred.block_length = 0;
	  now_pred.type = VECTOR_PRED;
	  now_pred.cc = 0;
	  break;
	case IT_INSN:
	  now_pred.state = MANUAL_PRED_BLOCK;
	  now_pred.block_length = 0;
	  now_pred.type = SCALAR_PRED;
	  break;
	}
      break;

    case AUTOMATIC_PRED_BLOCK:
      /* Three things may happen now:
	 a) We should increment current it block size;
	 b) We should close current it block (closing insn or 4 insns);
	 c) We should close current it block and start a new one (due
	 to incompatible conditions or
	 4 insns-length block reached).  */

      switch (inst.pred_insn_type)
	{
	case INSIDE_VPT_INSN:
	case VPT_INSN:
	case MVE_UNPREDICABLE_INSN:
	case MVE_OUTSIDE_PRED_INSN:
	  gas_assert (0);
	case OUTSIDE_PRED_INSN:
	  /* The closure of the block shall happen immediately,
	     so any in_pred_block () call reports the block as closed.  */
	  force_automatic_it_block_close ();
	  break;

	case INSIDE_IT_INSN:
	case INSIDE_IT_LAST_INSN:
	case IF_INSIDE_IT_LAST_INSN:
	  now_pred.block_length++;

	  if (now_pred.block_length > 4
	      || !now_pred_compatible (inst.cond))
	    {
	      force_automatic_it_block_close ();
	      if (inst.pred_insn_type != IF_INSIDE_IT_LAST_INSN)
		new_automatic_it_block (inst.cond);
	    }
	  else
	    {
	      now_pred.insn_cond = true;
	      now_pred_add_mask (inst.cond);
	    }

	  if (now_pred.state == AUTOMATIC_PRED_BLOCK
	      && (inst.pred_insn_type == INSIDE_IT_LAST_INSN
		  || inst.pred_insn_type == IF_INSIDE_IT_LAST_INSN))
	    close_automatic_it_block ();
	  break;

	  /* Fallthrough.  */
	case NEUTRAL_IT_INSN:
	  now_pred.block_length++;
	  now_pred.insn_cond = true;

	  if (now_pred.block_length > 4)
	    force_automatic_it_block_close ();
	  else
	    now_pred_add_mask (now_pred.cc & 1);
	  break;

	case IT_INSN:
	  close_automatic_it_block ();
	  now_pred.state = MANUAL_PRED_BLOCK;
	  break;
	}
      break;

    case MANUAL_PRED_BLOCK:
      {
	unsigned int cond;
	int is_last;
	if (now_pred.type == SCALAR_PRED)
	  {
	    /* Check conditional suffixes.  */
	    cond = now_pred.cc ^ ((now_pred.mask >> 4) & 1) ^ 1;
	    now_pred.mask <<= 1;
	    now_pred.mask &= 0x1f;
	    is_last = (now_pred.mask == 0x10);
	  }
	else
	  {
	    now_pred.cc ^= (now_pred.mask >> 4);
	    cond = now_pred.cc + 0xf;
	    now_pred.mask <<= 1;
	    now_pred.mask &= 0x1f;
	    is_last = now_pred.mask == 0x10;
	  }
	now_pred.insn_cond = true;

	switch (inst.pred_insn_type)
	  {
	  case OUTSIDE_PRED_INSN:
	    if (now_pred.type == SCALAR_PRED)
	      {
		if (inst.cond == COND_ALWAYS)
		  {
		    /* Case 12: In an IT block, with no code: error: missing
		       code.  */
		    inst.error = BAD_NOT_IT;
		    return FAIL;
		  }
		else if (inst.cond > COND_ALWAYS)
		  {
		    /* Case 11: In an IT block, with a VPT code: syntax error.
		     */
		    inst.error = BAD_SYNTAX;
		    return FAIL;
		  }
		else if (thumb_mode)
		  {
		    /* This is for some special cases where a non-MVE
		       instruction is not allowed in an IT block, such as cbz,
		       but are put into one with a condition code.
		       You could argue this should be a syntax error, but we
		       gave the 'not allowed in IT block' diagnostic in the
		       past so we will keep doing so.  */
		    inst.error = BAD_NOT_IT;
		    return FAIL;
		  }
		break;
	      }
	    else
	      {
		/* Case 15: In a VPT block, with no code: UNPREDICTABLE.  */
		as_tsktsk (MVE_NOT_VPT);
		return SUCCESS;
	      }
	  case MVE_OUTSIDE_PRED_INSN:
	    if (now_pred.type == SCALAR_PRED)
	      {
		if (inst.cond == COND_ALWAYS)
		  {
		    /* Case 3: In an IT block, with no code: warning:
		       UNPREDICTABLE.  */
		    as_tsktsk (MVE_NOT_IT);
		    return SUCCESS;
		  }
		else if (inst.cond < COND_ALWAYS)
		  {
		    /* Case 1: In an IT block, with an IT code: syntax error.
		     */
		    inst.error = BAD_SYNTAX;
		    return FAIL;
		  }
		else
		  gas_assert (0);
	      }
	    else
	      {
		if (inst.cond < COND_ALWAYS)
		  {
		    /* Case 4: In a VPT block, with an IT code: syntax error.
		     */
		    inst.error = BAD_SYNTAX;
		    return FAIL;
		  }
		else if (inst.cond == COND_ALWAYS)
		  {
		    /* Case 6: In a VPT block, with no code: error: missing
		       code.  */
		    inst.error = BAD_NOT_VPT;
		    return FAIL;
		  }
		else
		  {
		    gas_assert (0);
		  }
	      }
	  case MVE_UNPREDICABLE_INSN:
	    as_tsktsk (now_pred.type == SCALAR_PRED ? MVE_NOT_IT : MVE_NOT_VPT);
	    return SUCCESS;
	  case INSIDE_IT_INSN:
	    if (inst.cond > COND_ALWAYS)
	      {
		/* Case 11: In an IT block, with a VPT code: syntax error.  */
		/* Case 14: In a VPT block, with a VPT code: syntax error.  */
		inst.error = BAD_SYNTAX;
		return FAIL;
	      }
	    else if (now_pred.type == SCALAR_PRED)
	      {
		/* Case 10: In an IT block, with an IT code: OK!  */
		if (cond != inst.cond)
		  {
		    inst.error = now_pred.type == SCALAR_PRED ? BAD_IT_COND :
		      BAD_VPT_COND;
		    return FAIL;
		  }
	      }
	    else
	      {
		/* Case 13: In a VPT block, with an IT code: error: should be
		   in an IT block.  */
		inst.error = BAD_OUT_IT;
		return FAIL;
	      }
	    break;

	  case INSIDE_VPT_INSN:
	    if (now_pred.type == SCALAR_PRED)
	      {
		/* Case 2: In an IT block, with a VPT code: error: must be in a
		   VPT block.  */
		inst.error = BAD_OUT_VPT;
		return FAIL;
	      }
	    /* Case 5:  In a VPT block, with a VPT code: OK!  */
	    else if (cond != inst.cond)
	      {
		inst.error = BAD_VPT_COND;
		return FAIL;
	      }
	    break;
	  case INSIDE_IT_LAST_INSN:
	  case IF_INSIDE_IT_LAST_INSN:
	    if (now_pred.type == VECTOR_PRED || inst.cond > COND_ALWAYS)
	      {
		/* Case 4: In a VPT block, with an IT code: syntax error.  */
		/* Case 11: In an IT block, with a VPT code: syntax error.  */
		inst.error = BAD_SYNTAX;
		return FAIL;
	      }
	    else if (cond != inst.cond)
	      {
		inst.error = BAD_IT_COND;
		return FAIL;
	      }
	    if (!is_last)
	      {
		inst.error = BAD_BRANCH;
		return FAIL;
	      }
	    break;

	  case NEUTRAL_IT_INSN:
	    /* The BKPT instruction is unconditional even in a IT or VPT
	       block.  */
	    break;

	  case IT_INSN:
	    if (now_pred.type == SCALAR_PRED)
	      {
		inst.error = BAD_IT_IT;
		return FAIL;
	      }
	    /* fall through.  */
	  case VPT_INSN:
	    if (inst.cond == COND_ALWAYS)
	      {
		/* Executing a VPT/VPST instruction inside an IT block or a
		   VPT/VPST/IT instruction inside a VPT block is UNPREDICTABLE.
		 */
		if (now_pred.type == SCALAR_PRED)
		  as_tsktsk (MVE_NOT_IT);
		else
		  as_tsktsk (MVE_NOT_VPT);
		return SUCCESS;
	      }
	    else
	      {
		/* VPT/VPST do not accept condition codes.  */
		inst.error = BAD_SYNTAX;
		return FAIL;
	      }
	  }
	}
      break;
    }

  return SUCCESS;
}

struct depr_insn_mask
{
  unsigned long pattern;
  unsigned long mask;
  const char* description;
};

/* List of 16-bit instruction patterns deprecated in an IT block in
   ARMv8.  */
static const struct depr_insn_mask depr_it_insns[] = {
  { 0xc000, 0xc000, N_("Short branches, Undefined, SVC, LDM/STM") },
  { 0xb000, 0xb000, N_("Miscellaneous 16-bit instructions") },
  { 0xa000, 0xb800, N_("ADR") },
  { 0x4800, 0xf800, N_("Literal loads") },
  { 0x4478, 0xf478, N_("Hi-register ADD, MOV, CMP, BX, BLX using pc") },
  { 0x4487, 0xfc87, N_("Hi-register ADD, MOV, CMP using pc") },
  /* NOTE: 0x00dd is not the real encoding, instead, it is the 'tvalue'
     field in asm_opcode. 'tvalue' is used at the stage this check happen.  */
  { 0x00dd, 0x7fff, N_("ADD/SUB sp, sp #imm") },
  { 0, 0, NULL }
};

static void
it_fsm_post_encode (void)
{
  int is_last;

  if (!now_pred.state_handled)
    handle_pred_state ();

  if (now_pred.insn_cond
      && warn_on_restrict_it
      && !now_pred.warn_deprecated
      && warn_on_deprecated
      && (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8)
          || ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8r))
      && !ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_m))
    {
      if (inst.instruction >= 0x10000)
	{
	  as_tsktsk (_("IT blocks containing 32-bit Thumb instructions are "
		     "performance deprecated in ARMv8-A and ARMv8-R"));
	  now_pred.warn_deprecated = true;
	}
      else
	{
	  const struct depr_insn_mask *p = depr_it_insns;

	  while (p->mask != 0)
	    {
	      if ((inst.instruction & p->mask) == p->pattern)
		{
		  as_tsktsk (_("IT blocks containing 16-bit Thumb "
			       "instructions of the following class are "
			       "performance deprecated in ARMv8-A and "
			       "ARMv8-R: %s"), p->description);
		  now_pred.warn_deprecated = true;
		  break;
		}

	      ++p;
	    }
	}

      if (now_pred.block_length > 1)
	{
	  as_tsktsk (_("IT blocks containing more than one conditional "
		     "instruction are performance deprecated in ARMv8-A and "
		     "ARMv8-R"));
	  now_pred.warn_deprecated = true;
	}
    }

    is_last = (now_pred.mask == 0x10);
    if (is_last)
      {
	now_pred.state = OUTSIDE_PRED_BLOCK;
	now_pred.mask = 0;
      }
}

static void
force_automatic_it_block_close (void)
{
  if (now_pred.state == AUTOMATIC_PRED_BLOCK)
    {
      close_automatic_it_block ();
      now_pred.state = OUTSIDE_PRED_BLOCK;
      now_pred.mask = 0;
    }
}

static int
in_pred_block (void)
{
  if (!now_pred.state_handled)
    handle_pred_state ();

  return now_pred.state != OUTSIDE_PRED_BLOCK;
}

/* Whether OPCODE only has T32 encoding.  Since this function is only used by
   t32_insn_ok, OPCODE enabled by v6t2 extension bit do not need to be listed
   here, hence the "known" in the function name.  */

static bool
known_t32_only_insn (const struct asm_opcode *opcode)
{
  /* Original Thumb-1 wide instruction.  */
  if (opcode->tencode == do_t_blx
      || opcode->tencode == do_t_branch23
      || ARM_CPU_HAS_FEATURE (*opcode->tvariant, arm_ext_msr)
      || ARM_CPU_HAS_FEATURE (*opcode->tvariant, arm_ext_barrier))
    return true;

  /* Wide-only instruction added to ARMv8-M Baseline.  */
  if (ARM_CPU_HAS_FEATURE (*opcode->tvariant, arm_ext_v8m_m_only)
      || ARM_CPU_HAS_FEATURE (*opcode->tvariant, arm_ext_atomics)
      || ARM_CPU_HAS_FEATURE (*opcode->tvariant, arm_ext_v6t2_v8m)
      || ARM_CPU_HAS_FEATURE (*opcode->tvariant, arm_ext_div))
    return true;

  return false;
}

/* Whether wide instruction variant can be used if available for a valid OPCODE
   in ARCH.  */

static bool
t32_insn_ok (arm_feature_set arch, const struct asm_opcode *opcode)
{
  if (known_t32_only_insn (opcode))
    return true;

  /* Instruction with narrow and wide encoding added to ARMv8-M.  Availability
     of variant T3 of B.W is checked in do_t_branch.  */
  if (ARM_CPU_HAS_FEATURE (arch, arm_ext_v8m)
      && opcode->tencode == do_t_branch)
    return true;

  /* MOV accepts T1/T3 encodings under Baseline, T3 encoding is 32bit.  */
  if (ARM_CPU_HAS_FEATURE (arch, arm_ext_v8m)
      && opcode->tencode == do_t_mov_cmp
      /* Make sure CMP instruction is not affected.  */
      && opcode->aencode == do_mov)
    return true;

  /* Wide instruction variants of all instructions with narrow *and* wide
     variants become available with ARMv6t2.  Other opcodes are either
     narrow-only or wide-only and are thus available if OPCODE is valid.  */
  if (ARM_CPU_HAS_FEATURE (arch, arm_ext_v6t2))
    return true;

  /* OPCODE with narrow only instruction variant or wide variant not
     available.  */
  return false;
}

void
md_assemble (char *str)
{
  char *p = str;
  const struct asm_opcode * opcode;

  /* Align the previous label if needed.  */
  if (last_label_seen != NULL)
    {
      symbol_set_frag (last_label_seen, frag_now);
      S_SET_VALUE (last_label_seen, (valueT) frag_now_fix ());
      S_SET_SEGMENT (last_label_seen, now_seg);
    }

  memset (&inst, '\0', sizeof (inst));
  int r;
  for (r = 0; r < ARM_IT_MAX_RELOCS; r++)
    inst.relocs[r].type = BFD_RELOC_UNUSED;

  opcode = opcode_lookup (&p);
  if (!opcode)
    {
      /* It wasn't an instruction, but it might be a register alias of
	 the form alias .req reg, or a Neon .dn/.qn directive.  */
      if (! create_register_alias (str, p)
	  && ! create_neon_reg_alias (str, p))
	as_bad (_("bad instruction `%s'"), str);

      return;
    }

  if (warn_on_deprecated && opcode->tag == OT_cinfix3_deprecated)
    as_tsktsk (_("s suffix on comparison instruction is deprecated"));

  /* The value which unconditional instructions should have in place of the
     condition field.  */
  inst.uncond_value = (opcode->tag == OT_csuffixF) ? 0xf : -1u;

  if (thumb_mode)
    {
      arm_feature_set variant;

      variant = cpu_variant;
      /* Only allow coprocessor instructions on Thumb-2 capable devices.  */
      if (!ARM_CPU_HAS_FEATURE (variant, arm_arch_t2))
	ARM_CLEAR_FEATURE (variant, variant, fpu_any_hard);
      /* Check that this instruction is supported for this CPU.  */
      if (!opcode->tvariant
	  || (thumb_mode == 1
	      && !ARM_CPU_HAS_FEATURE (variant, *opcode->tvariant)))
	{
	  if (opcode->tencode == do_t_swi)
	    as_bad (_("SVC is not permitted on this architecture"));
	  else
	    as_bad (_("selected processor does not support `%s' in Thumb mode"), str);
	  return;
	}
      if (inst.cond != COND_ALWAYS && !unified_syntax
	  && opcode->tencode != do_t_branch)
	{
	  as_bad (_("Thumb does not support conditional execution"));
	  return;
	}

      /* Two things are addressed here:
	 1) Implicit require narrow instructions on Thumb-1.
	    This avoids relaxation accidentally introducing Thumb-2
	    instructions.
	 2) Reject wide instructions in non Thumb-2 cores.

	 Only instructions with narrow and wide variants need to be handled
	 but selecting all non wide-only instructions is easier.  */
      if (!ARM_CPU_HAS_FEATURE (variant, arm_ext_v6t2)
	  && !t32_insn_ok (variant, opcode))
	{
	  if (inst.size_req == 0)
	    inst.size_req = 2;
	  else if (inst.size_req == 4)
	    {
	      if (ARM_CPU_HAS_FEATURE (variant, arm_ext_v8m))
		as_bad (_("selected processor does not support 32bit wide "
			  "variant of instruction `%s'"), str);
	      else
		as_bad (_("selected processor does not support `%s' in "
			  "Thumb-2 mode"), str);
	      return;
	    }
	}

      inst.instruction = opcode->tvalue;

      if (!parse_operands (p, opcode->operands, /*thumb=*/true))
	{
	  /* Prepare the pred_insn_type for those encodings that don't set
	     it.  */
	  it_fsm_pre_encode ();

	  opcode->tencode ();

	  it_fsm_post_encode ();
	}

      if (!(inst.error || inst.relax))
	{
	  gas_assert (inst.instruction < 0xe800 || inst.instruction > 0xffff);
	  inst.size = (inst.instruction > 0xffff ? 4 : 2);
	  if (inst.size_req && inst.size_req != inst.size)
	    {
	      as_bad (_("cannot honor width suffix -- `%s'"), str);
	      return;
	    }
	}

      /* Something has gone badly wrong if we try to relax a fixed size
	 instruction.  */
      gas_assert (inst.size_req == 0 || !inst.relax);

      ARM_MERGE_FEATURE_SETS (thumb_arch_used, thumb_arch_used,
			      *opcode->tvariant);
      /* Many Thumb-2 instructions also have Thumb-1 variants, so explicitly
	 set those bits when Thumb-2 32-bit instructions are seen.  The impact
	 of relaxable instructions will be considered later after we finish all
	 relaxation.  */
      if (ARM_FEATURE_CORE_EQUAL (cpu_variant, arm_arch_any))
	variant = arm_arch_none;
      else
	variant = cpu_variant;
      if (inst.size == 4 && !t32_insn_ok (variant, opcode))
	ARM_MERGE_FEATURE_SETS (thumb_arch_used, thumb_arch_used,
				arm_ext_v6t2);

      check_neon_suffixes;

      if (!inst.error)
	{
	  mapping_state (MAP_THUMB);
	}
    }
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v1))
    {
      bool is_bx;

      /* bx is allowed on v5 cores, and sometimes on v4 cores.  */
      is_bx = (opcode->aencode == do_bx);

      /* Check that this instruction is supported for this CPU.  */
      if (!(is_bx && fix_v4bx)
	  && !(opcode->avariant &&
	       ARM_CPU_HAS_FEATURE (cpu_variant, *opcode->avariant)))
	{
	  as_bad (_("selected processor does not support `%s' in ARM mode"), str);
	  return;
	}
      if (inst.size_req)
	{
	  as_bad (_("width suffixes are invalid in ARM mode -- `%s'"), str);
	  return;
	}

      inst.instruction = opcode->avalue;
      if (opcode->tag == OT_unconditionalF)
	inst.instruction |= 0xFU << 28;
      else
	inst.instruction |= inst.cond << 28;
      inst.size = INSN_SIZE;
      if (!parse_operands (p, opcode->operands, /*thumb=*/false))
	{
	  it_fsm_pre_encode ();
	  opcode->aencode ();
	  it_fsm_post_encode ();
	}
      /* Arm mode bx is marked as both v4T and v5 because it's still required
	 on a hypothetical non-thumb v5 core.  */
      if (is_bx)
	ARM_MERGE_FEATURE_SETS (arm_arch_used, arm_arch_used, arm_ext_v4t);
      else
	ARM_MERGE_FEATURE_SETS (arm_arch_used, arm_arch_used,
				*opcode->avariant);

      check_neon_suffixes;

      if (!inst.error)
	{
	  mapping_state (MAP_ARM);
	}
    }
  else
    {
      as_bad (_("attempt to use an ARM instruction on a Thumb-only processor "
		"-- `%s'"), str);
      return;
    }
  output_inst (str);
}

static void
check_pred_blocks_finished (void)
{
#ifdef OBJ_ELF
  asection *sect;

  for (sect = stdoutput->sections; sect != NULL; sect = sect->next)
    if (seg_info (sect)->tc_segment_info_data.current_pred.state
	== MANUAL_PRED_BLOCK)
      {
	if (now_pred.type == SCALAR_PRED)
	  as_warn (_("section '%s' finished with an open IT block."),
		   sect->name);
	else
	  as_warn (_("section '%s' finished with an open VPT/VPST block."),
		   sect->name);
      }
#else
  if (now_pred.state == MANUAL_PRED_BLOCK)
    {
      if (now_pred.type == SCALAR_PRED)
       as_warn (_("file finished with an open IT block."));
      else
	as_warn (_("file finished with an open VPT/VPST block."));
    }
#endif
}

/* Various frobbings of labels and their addresses.  */

void
arm_start_line_hook (void)
{
  last_label_seen = NULL;
}

void
arm_frob_label (symbolS * sym)
{
  last_label_seen = sym;

  ARM_SET_THUMB (sym, thumb_mode);

#if defined OBJ_COFF || defined OBJ_ELF
  ARM_SET_INTERWORK (sym, support_interwork);
#endif

  force_automatic_it_block_close ();

  /* Note - do not allow local symbols (.Lxxx) to be labelled
     as Thumb functions.  This is because these labels, whilst
     they exist inside Thumb code, are not the entry points for
     possible ARM->Thumb calls.	 Also, these labels can be used
     as part of a computed goto or switch statement.  eg gcc
     can generate code that looks like this:

		ldr  r2, [pc, .Laaa]
		lsl  r3, r3, #2
		ldr  r2, [r3, r2]
		mov  pc, r2

       .Lbbb:  .word .Lxxx
       .Lccc:  .word .Lyyy
       ..etc...
       .Laaa:	.word Lbbb

     The first instruction loads the address of the jump table.
     The second instruction converts a table index into a byte offset.
     The third instruction gets the jump address out of the table.
     The fourth instruction performs the jump.

     If the address stored at .Laaa is that of a symbol which has the
     Thumb_Func bit set, then the linker will arrange for this address
     to have the bottom bit set, which in turn would mean that the
     address computation performed by the third instruction would end
     up with the bottom bit set.  Since the ARM is capable of unaligned
     word loads, the instruction would then load the incorrect address
     out of the jump table, and chaos would ensue.  */
  if (label_is_thumb_function_name
      && (S_GET_NAME (sym)[0] != '.' || S_GET_NAME (sym)[1] != 'L')
      && (bfd_section_flags (now_seg) & SEC_CODE) != 0)
    {
      /* When the address of a Thumb function is taken the bottom
	 bit of that address should be set.  This will allow
	 interworking between Arm and Thumb functions to work
	 correctly.  */

      THUMB_SET_FUNC (sym, 1);

      label_is_thumb_function_name = false;
    }

  dwarf2_emit_label (sym);
}

bool
arm_data_in_code (void)
{
  if (thumb_mode && startswith (input_line_pointer + 1, "data:"))
    {
      *input_line_pointer = '/';
      input_line_pointer += 5;
      *input_line_pointer = 0;
      return true;
    }

  return false;
}

char *
arm_canonicalize_symbol_name (char * name)
{
  int len;

  if (thumb_mode && (len = strlen (name)) > 5
      && streq (name + len - 5, "/data"))
    *(name + len - 5) = 0;

  return name;
}

/* Table of all register names defined by default.  The user can
   define additional names with .req.  Note that all register names
   should appear in both upper and lowercase variants.	Some registers
   also have mixed-case names.	*/

#define REGDEF(s,n,t) { #s, n, REG_TYPE_##t, true, 0 }
#define REGNUM(p,n,t) REGDEF(p##n, n, t)
#define REGNUM2(p,n,t) REGDEF(p##n, 2 * n, t)
#define REGSET(p,t) \
  REGNUM(p, 0,t), REGNUM(p, 1,t), REGNUM(p, 2,t), REGNUM(p, 3,t), \
  REGNUM(p, 4,t), REGNUM(p, 5,t), REGNUM(p, 6,t), REGNUM(p, 7,t), \
  REGNUM(p, 8,t), REGNUM(p, 9,t), REGNUM(p,10,t), REGNUM(p,11,t), \
  REGNUM(p,12,t), REGNUM(p,13,t), REGNUM(p,14,t), REGNUM(p,15,t)
#define REGSETH(p,t) \
  REGNUM(p,16,t), REGNUM(p,17,t), REGNUM(p,18,t), REGNUM(p,19,t), \
  REGNUM(p,20,t), REGNUM(p,21,t), REGNUM(p,22,t), REGNUM(p,23,t), \
  REGNUM(p,24,t), REGNUM(p,25,t), REGNUM(p,26,t), REGNUM(p,27,t), \
  REGNUM(p,28,t), REGNUM(p,29,t), REGNUM(p,30,t), REGNUM(p,31,t)
#define REGSET2(p,t) \
  REGNUM2(p, 0,t), REGNUM2(p, 1,t), REGNUM2(p, 2,t), REGNUM2(p, 3,t), \
  REGNUM2(p, 4,t), REGNUM2(p, 5,t), REGNUM2(p, 6,t), REGNUM2(p, 7,t), \
  REGNUM2(p, 8,t), REGNUM2(p, 9,t), REGNUM2(p,10,t), REGNUM2(p,11,t), \
  REGNUM2(p,12,t), REGNUM2(p,13,t), REGNUM2(p,14,t), REGNUM2(p,15,t)
#define SPLRBANK(base,bank,t) \
  REGDEF(lr_##bank, 768|((base+0)<<16), t), \
  REGDEF(sp_##bank, 768|((base+1)<<16), t), \
  REGDEF(spsr_##bank, 768|(base<<16)|SPSR_BIT, t), \
  REGDEF(LR_##bank, 768|((base+0)<<16), t), \
  REGDEF(SP_##bank, 768|((base+1)<<16), t), \
  REGDEF(SPSR_##bank, 768|(base<<16)|SPSR_BIT, t)

static const struct reg_entry reg_names[] =
{
  /* ARM integer registers.  */
  REGSET(r, RN), REGSET(R, RN),

  /* ATPCS synonyms.  */
  REGDEF(a1,0,RN), REGDEF(a2,1,RN), REGDEF(a3, 2,RN), REGDEF(a4, 3,RN),
  REGDEF(v1,4,RN), REGDEF(v2,5,RN), REGDEF(v3, 6,RN), REGDEF(v4, 7,RN),
  REGDEF(v5,8,RN), REGDEF(v6,9,RN), REGDEF(v7,10,RN), REGDEF(v8,11,RN),

  REGDEF(A1,0,RN), REGDEF(A2,1,RN), REGDEF(A3, 2,RN), REGDEF(A4, 3,RN),
  REGDEF(V1,4,RN), REGDEF(V2,5,RN), REGDEF(V3, 6,RN), REGDEF(V4, 7,RN),
  REGDEF(V5,8,RN), REGDEF(V6,9,RN), REGDEF(V7,10,RN), REGDEF(V8,11,RN),

  /* Well-known aliases.  */
  REGDEF(wr, 7,RN), REGDEF(sb, 9,RN), REGDEF(sl,10,RN), REGDEF(fp,11,RN),
  REGDEF(ip,12,RN), REGDEF(sp,13,RN), REGDEF(lr,14,RN), REGDEF(pc,15,RN),

  REGDEF(WR, 7,RN), REGDEF(SB, 9,RN), REGDEF(SL,10,RN), REGDEF(FP,11,RN),
  REGDEF(IP,12,RN), REGDEF(SP,13,RN), REGDEF(LR,14,RN), REGDEF(PC,15,RN),

  /* Defining the new Zero register from ARMv8.1-M.  */
  REGDEF(zr,15,ZR),
  REGDEF(ZR,15,ZR),

  /* Coprocessor numbers.  */
  REGSET(p, CP), REGSET(P, CP),

  /* Coprocessor register numbers.  The "cr" variants are for backward
     compatibility.  */
  REGSET(c,  CN), REGSET(C, CN),
  REGSET(cr, CN), REGSET(CR, CN),

  /* ARM banked registers.  */
  REGDEF(R8_usr,512|(0<<16),RNB), REGDEF(r8_usr,512|(0<<16),RNB),
  REGDEF(R9_usr,512|(1<<16),RNB), REGDEF(r9_usr,512|(1<<16),RNB),
  REGDEF(R10_usr,512|(2<<16),RNB), REGDEF(r10_usr,512|(2<<16),RNB),
  REGDEF(R11_usr,512|(3<<16),RNB), REGDEF(r11_usr,512|(3<<16),RNB),
  REGDEF(R12_usr,512|(4<<16),RNB), REGDEF(r12_usr,512|(4<<16),RNB),
  REGDEF(SP_usr,512|(5<<16),RNB), REGDEF(sp_usr,512|(5<<16),RNB),
  REGDEF(LR_usr,512|(6<<16),RNB), REGDEF(lr_usr,512|(6<<16),RNB),

  REGDEF(R8_fiq,512|(8<<16),RNB), REGDEF(r8_fiq,512|(8<<16),RNB),
  REGDEF(R9_fiq,512|(9<<16),RNB), REGDEF(r9_fiq,512|(9<<16),RNB),
  REGDEF(R10_fiq,512|(10<<16),RNB), REGDEF(r10_fiq,512|(10<<16),RNB),
  REGDEF(R11_fiq,512|(11<<16),RNB), REGDEF(r11_fiq,512|(11<<16),RNB),
  REGDEF(R12_fiq,512|(12<<16),RNB), REGDEF(r12_fiq,512|(12<<16),RNB),
  REGDEF(SP_fiq,512|(13<<16),RNB), REGDEF(sp_fiq,512|(13<<16),RNB),
  REGDEF(LR_fiq,512|(14<<16),RNB), REGDEF(lr_fiq,512|(14<<16),RNB),
  REGDEF(SPSR_fiq,512|(14<<16)|SPSR_BIT,RNB), REGDEF(spsr_fiq,512|(14<<16)|SPSR_BIT,RNB),

  SPLRBANK(0,IRQ,RNB), SPLRBANK(0,irq,RNB),
  SPLRBANK(2,SVC,RNB), SPLRBANK(2,svc,RNB),
  SPLRBANK(4,ABT,RNB), SPLRBANK(4,abt,RNB),
  SPLRBANK(6,UND,RNB), SPLRBANK(6,und,RNB),
  SPLRBANK(12,MON,RNB), SPLRBANK(12,mon,RNB),
  REGDEF(elr_hyp,768|(14<<16),RNB), REGDEF(ELR_hyp,768|(14<<16),RNB),
  REGDEF(sp_hyp,768|(15<<16),RNB), REGDEF(SP_hyp,768|(15<<16),RNB),
  REGDEF(spsr_hyp,768|(14<<16)|SPSR_BIT,RNB),
  REGDEF(SPSR_hyp,768|(14<<16)|SPSR_BIT,RNB),

  /* FPA registers.  */
  REGNUM(f,0,FN), REGNUM(f,1,FN), REGNUM(f,2,FN), REGNUM(f,3,FN),
  REGNUM(f,4,FN), REGNUM(f,5,FN), REGNUM(f,6,FN), REGNUM(f,7, FN),

  REGNUM(F,0,FN), REGNUM(F,1,FN), REGNUM(F,2,FN), REGNUM(F,3,FN),
  REGNUM(F,4,FN), REGNUM(F,5,FN), REGNUM(F,6,FN), REGNUM(F,7, FN),

  /* VFP SP registers.	*/
  REGSET(s,VFS),  REGSET(S,VFS),
  REGSETH(s,VFS), REGSETH(S,VFS),

  /* VFP DP Registers.	*/
  REGSET(d,VFD),  REGSET(D,VFD),
  /* Extra Neon DP registers.  */
  REGSETH(d,VFD), REGSETH(D,VFD),

  /* Neon QP registers.  */
  REGSET2(q,NQ),  REGSET2(Q,NQ),

  /* VFP control registers.  */
  REGDEF(fpsid,0,VFC), REGDEF(fpscr,1,VFC), REGDEF(fpexc,8,VFC),
  REGDEF(FPSID,0,VFC), REGDEF(FPSCR,1,VFC), REGDEF(FPEXC,8,VFC),
  REGDEF(fpinst,9,VFC), REGDEF(fpinst2,10,VFC),
  REGDEF(FPINST,9,VFC), REGDEF(FPINST2,10,VFC),
  REGDEF(mvfr0,7,VFC), REGDEF(mvfr1,6,VFC),
  REGDEF(MVFR0,7,VFC), REGDEF(MVFR1,6,VFC),
  REGDEF(mvfr2,5,VFC), REGDEF(MVFR2,5,VFC),
  REGDEF(fpscr_nzcvqc,2,VFC), REGDEF(FPSCR_nzcvqc,2,VFC),
  REGDEF(vpr,12,VFC), REGDEF(VPR,12,VFC),
  REGDEF(fpcxt_ns,14,VFC), REGDEF(FPCXT_NS,14,VFC),
  REGDEF(fpcxt_s,15,VFC), REGDEF(FPCXT_S,15,VFC),
  REGDEF(fpcxtns,14,VFC), REGDEF(FPCXTNS,14,VFC),
  REGDEF(fpcxts,15,VFC), REGDEF(FPCXTS,15,VFC),

  /* Maverick DSP coprocessor registers.  */
  REGSET(mvf,MVF),  REGSET(mvd,MVD),  REGSET(mvfx,MVFX),  REGSET(mvdx,MVDX),
  REGSET(MVF,MVF),  REGSET(MVD,MVD),  REGSET(MVFX,MVFX),  REGSET(MVDX,MVDX),

  REGNUM(mvax,0,MVAX), REGNUM(mvax,1,MVAX),
  REGNUM(mvax,2,MVAX), REGNUM(mvax,3,MVAX),
  REGDEF(dspsc,0,DSPSC),

  REGNUM(MVAX,0,MVAX), REGNUM(MVAX,1,MVAX),
  REGNUM(MVAX,2,MVAX), REGNUM(MVAX,3,MVAX),
  REGDEF(DSPSC,0,DSPSC),

  /* iWMMXt data registers - p0, c0-15.	 */
  REGSET(wr,MMXWR), REGSET(wR,MMXWR), REGSET(WR, MMXWR),

  /* iWMMXt control registers - p1, c0-3.  */
  REGDEF(wcid,	0,MMXWC),  REGDEF(wCID,	 0,MMXWC),  REGDEF(WCID,  0,MMXWC),
  REGDEF(wcon,	1,MMXWC),  REGDEF(wCon,	 1,MMXWC),  REGDEF(WCON,  1,MMXWC),
  REGDEF(wcssf, 2,MMXWC),  REGDEF(wCSSF, 2,MMXWC),  REGDEF(WCSSF, 2,MMXWC),
  REGDEF(wcasf, 3,MMXWC),  REGDEF(wCASF, 3,MMXWC),  REGDEF(WCASF, 3,MMXWC),

  /* iWMMXt scalar (constant/offset) registers - p1, c8-11.  */
  REGDEF(wcgr0, 8,MMXWCG),  REGDEF(wCGR0, 8,MMXWCG),  REGDEF(WCGR0, 8,MMXWCG),
  REGDEF(wcgr1, 9,MMXWCG),  REGDEF(wCGR1, 9,MMXWCG),  REGDEF(WCGR1, 9,MMXWCG),
  REGDEF(wcgr2,10,MMXWCG),  REGDEF(wCGR2,10,MMXWCG),  REGDEF(WCGR2,10,MMXWCG),
  REGDEF(wcgr3,11,MMXWCG),  REGDEF(wCGR3,11,MMXWCG),  REGDEF(WCGR3,11,MMXWCG),

  /* XScale accumulator registers.  */
  REGNUM(acc,0,XSCALE), REGNUM(ACC,0,XSCALE),

  /* DWARF ABI defines RA_AUTH_CODE to 143.  */
  REGDEF(ra_auth_code,143,PSEUDO),
};
#undef REGDEF
#undef REGNUM
#undef REGSET

/* Table of all PSR suffixes.  Bare "CPSR" and "SPSR" are handled
   within psr_required_here.  */
static const struct asm_psr psrs[] =
{
  /* Backward compatibility notation.  Note that "all" is no longer
     truly all possible PSR bits.  */
  {"all",  PSR_c | PSR_f},
  {"flg",  PSR_f},
  {"ctl",  PSR_c},

  /* Individual flags.	*/
  {"f",	   PSR_f},
  {"c",	   PSR_c},
  {"x",	   PSR_x},
  {"s",	   PSR_s},

  /* Combinations of flags.  */
  {"fs",   PSR_f | PSR_s},
  {"fx",   PSR_f | PSR_x},
  {"fc",   PSR_f | PSR_c},
  {"sf",   PSR_s | PSR_f},
  {"sx",   PSR_s | PSR_x},
  {"sc",   PSR_s | PSR_c},
  {"xf",   PSR_x | PSR_f},
  {"xs",   PSR_x | PSR_s},
  {"xc",   PSR_x | PSR_c},
  {"cf",   PSR_c | PSR_f},
  {"cs",   PSR_c | PSR_s},
  {"cx",   PSR_c | PSR_x},
  {"fsx",  PSR_f | PSR_s | PSR_x},
  {"fsc",  PSR_f | PSR_s | PSR_c},
  {"fxs",  PSR_f | PSR_x | PSR_s},
  {"fxc",  PSR_f | PSR_x | PSR_c},
  {"fcs",  PSR_f | PSR_c | PSR_s},
  {"fcx",  PSR_f | PSR_c | PSR_x},
  {"sfx",  PSR_s | PSR_f | PSR_x},
  {"sfc",  PSR_s | PSR_f | PSR_c},
  {"sxf",  PSR_s | PSR_x | PSR_f},
  {"sxc",  PSR_s | PSR_x | PSR_c},
  {"scf",  PSR_s | PSR_c | PSR_f},
  {"scx",  PSR_s | PSR_c | PSR_x},
  {"xfs",  PSR_x | PSR_f | PSR_s},
  {"xfc",  PSR_x | PSR_f | PSR_c},
  {"xsf",  PSR_x | PSR_s | PSR_f},
  {"xsc",  PSR_x | PSR_s | PSR_c},
  {"xcf",  PSR_x | PSR_c | PSR_f},
  {"xcs",  PSR_x | PSR_c | PSR_s},
  {"cfs",  PSR_c | PSR_f | PSR_s},
  {"cfx",  PSR_c | PSR_f | PSR_x},
  {"csf",  PSR_c | PSR_s | PSR_f},
  {"csx",  PSR_c | PSR_s | PSR_x},
  {"cxf",  PSR_c | PSR_x | PSR_f},
  {"cxs",  PSR_c | PSR_x | PSR_s},
  {"fsxc", PSR_f | PSR_s | PSR_x | PSR_c},
  {"fscx", PSR_f | PSR_s | PSR_c | PSR_x},
  {"fxsc", PSR_f | PSR_x | PSR_s | PSR_c},
  {"fxcs", PSR_f | PSR_x | PSR_c | PSR_s},
  {"fcsx", PSR_f | PSR_c | PSR_s | PSR_x},
  {"fcxs", PSR_f | PSR_c | PSR_x | PSR_s},
  {"sfxc", PSR_s | PSR_f | PSR_x | PSR_c},
  {"sfcx", PSR_s | PSR_f | PSR_c | PSR_x},
  {"sxfc", PSR_s | PSR_x | PSR_f | PSR_c},
  {"sxcf", PSR_s | PSR_x | PSR_c | PSR_f},
  {"scfx", PSR_s | PSR_c | PSR_f | PSR_x},
  {"scxf", PSR_s | PSR_c | PSR_x | PSR_f},
  {"xfsc", PSR_x | PSR_f | PSR_s | PSR_c},
  {"xfcs", PSR_x | PSR_f | PSR_c | PSR_s},
  {"xsfc", PSR_x | PSR_s | PSR_f | PSR_c},
  {"xscf", PSR_x | PSR_s | PSR_c | PSR_f},
  {"xcfs", PSR_x | PSR_c | PSR_f | PSR_s},
  {"xcsf", PSR_x | PSR_c | PSR_s | PSR_f},
  {"cfsx", PSR_c | PSR_f | PSR_s | PSR_x},
  {"cfxs", PSR_c | PSR_f | PSR_x | PSR_s},
  {"csfx", PSR_c | PSR_s | PSR_f | PSR_x},
  {"csxf", PSR_c | PSR_s | PSR_x | PSR_f},
  {"cxfs", PSR_c | PSR_x | PSR_f | PSR_s},
  {"cxsf", PSR_c | PSR_x | PSR_s | PSR_f},
};

/* Table of V7M psr names.  */
static const struct asm_psr v7m_psrs[] =
{
  {"apsr",	   0x0 }, {"APSR",	   0x0 },
  {"iapsr",	   0x1 }, {"IAPSR",	   0x1 },
  {"eapsr",	   0x2 }, {"EAPSR",	   0x2 },
  {"psr",	   0x3 }, {"PSR",	   0x3 },
  {"xpsr",	   0x3 }, {"XPSR",	   0x3 }, {"xPSR",	  3 },
  {"ipsr",	   0x5 }, {"IPSR",	   0x5 },
  {"epsr",	   0x6 }, {"EPSR",	   0x6 },
  {"iepsr",	   0x7 }, {"IEPSR",	   0x7 },
  {"msp",	   0x8 }, {"MSP",	   0x8 },
  {"psp",	   0x9 }, {"PSP",	   0x9 },
  {"msplim",	   0xa }, {"MSPLIM",	   0xa },
  {"psplim",	   0xb }, {"PSPLIM",	   0xb },
  {"primask",	   0x10}, {"PRIMASK",	   0x10},
  {"basepri",	   0x11}, {"BASEPRI",	   0x11},
  {"basepri_max",  0x12}, {"BASEPRI_MAX",  0x12},
  {"faultmask",	   0x13}, {"FAULTMASK",	   0x13},
  {"control",	   0x14}, {"CONTROL",	   0x14},
  {"msp_ns",	   0x88}, {"MSP_NS",	   0x88},
  {"psp_ns",	   0x89}, {"PSP_NS",	   0x89},
  {"msplim_ns",	   0x8a}, {"MSPLIM_NS",	   0x8a},
  {"psplim_ns",	   0x8b}, {"PSPLIM_NS",	   0x8b},
  {"primask_ns",   0x90}, {"PRIMASK_NS",   0x90},
  {"basepri_ns",   0x91}, {"BASEPRI_NS",   0x91},
  {"faultmask_ns", 0x93}, {"FAULTMASK_NS", 0x93},
  {"control_ns",   0x94}, {"CONTROL_NS",   0x94},
  {"sp_ns",	   0x98}, {"SP_NS",	   0x98 }
};

/* Table of all shift-in-operand names.	 */
static const struct asm_shift_name shift_names [] =
{
  { "asl", SHIFT_LSL },	 { "ASL", SHIFT_LSL },
  { "lsl", SHIFT_LSL },	 { "LSL", SHIFT_LSL },
  { "lsr", SHIFT_LSR },	 { "LSR", SHIFT_LSR },
  { "asr", SHIFT_ASR },	 { "ASR", SHIFT_ASR },
  { "ror", SHIFT_ROR },	 { "ROR", SHIFT_ROR },
  { "rrx", SHIFT_RRX },	 { "RRX", SHIFT_RRX },
  { "uxtw", SHIFT_UXTW}, { "UXTW", SHIFT_UXTW}
};

/* Table of all explicit relocation names.  */
#ifdef OBJ_ELF
static struct reloc_entry reloc_names[] =
{
  { "got",     BFD_RELOC_ARM_GOT32   },	 { "GOT",     BFD_RELOC_ARM_GOT32   },
  { "gotoff",  BFD_RELOC_ARM_GOTOFF  },	 { "GOTOFF",  BFD_RELOC_ARM_GOTOFF  },
  { "plt",     BFD_RELOC_ARM_PLT32   },	 { "PLT",     BFD_RELOC_ARM_PLT32   },
  { "target1", BFD_RELOC_ARM_TARGET1 },	 { "TARGET1", BFD_RELOC_ARM_TARGET1 },
  { "target2", BFD_RELOC_ARM_TARGET2 },	 { "TARGET2", BFD_RELOC_ARM_TARGET2 },
  { "sbrel",   BFD_RELOC_ARM_SBREL32 },	 { "SBREL",   BFD_RELOC_ARM_SBREL32 },
  { "tlsgd",   BFD_RELOC_ARM_TLS_GD32},  { "TLSGD",   BFD_RELOC_ARM_TLS_GD32},
  { "tlsldm",  BFD_RELOC_ARM_TLS_LDM32}, { "TLSLDM",  BFD_RELOC_ARM_TLS_LDM32},
  { "tlsldo",  BFD_RELOC_ARM_TLS_LDO32}, { "TLSLDO",  BFD_RELOC_ARM_TLS_LDO32},
  { "gottpoff",BFD_RELOC_ARM_TLS_IE32},  { "GOTTPOFF",BFD_RELOC_ARM_TLS_IE32},
  { "tpoff",   BFD_RELOC_ARM_TLS_LE32},  { "TPOFF",   BFD_RELOC_ARM_TLS_LE32},
  { "got_prel", BFD_RELOC_ARM_GOT_PREL}, { "GOT_PREL", BFD_RELOC_ARM_GOT_PREL},
  { "tlsdesc", BFD_RELOC_ARM_TLS_GOTDESC},
	{ "TLSDESC", BFD_RELOC_ARM_TLS_GOTDESC},
  { "tlscall", BFD_RELOC_ARM_TLS_CALL},
	{ "TLSCALL", BFD_RELOC_ARM_TLS_CALL},
  { "tlsdescseq", BFD_RELOC_ARM_TLS_DESCSEQ},
	{ "TLSDESCSEQ", BFD_RELOC_ARM_TLS_DESCSEQ},
  { "gotfuncdesc", BFD_RELOC_ARM_GOTFUNCDESC },
	{ "GOTFUNCDESC", BFD_RELOC_ARM_GOTFUNCDESC },
  { "gotofffuncdesc", BFD_RELOC_ARM_GOTOFFFUNCDESC },
	{ "GOTOFFFUNCDESC", BFD_RELOC_ARM_GOTOFFFUNCDESC },
  { "funcdesc", BFD_RELOC_ARM_FUNCDESC },
	{ "FUNCDESC", BFD_RELOC_ARM_FUNCDESC },
   { "tlsgd_fdpic", BFD_RELOC_ARM_TLS_GD32_FDPIC },      { "TLSGD_FDPIC", BFD_RELOC_ARM_TLS_GD32_FDPIC },
   { "tlsldm_fdpic", BFD_RELOC_ARM_TLS_LDM32_FDPIC },    { "TLSLDM_FDPIC", BFD_RELOC_ARM_TLS_LDM32_FDPIC },
   { "gottpoff_fdpic", BFD_RELOC_ARM_TLS_IE32_FDPIC },   { "GOTTPOFF_FDIC", BFD_RELOC_ARM_TLS_IE32_FDPIC },
};
#endif

/* Table of all conditional affixes.  */
static const struct asm_cond conds[] =
{
  {"eq", 0x0},
  {"ne", 0x1},
  {"cs", 0x2}, {"hs", 0x2},
  {"cc", 0x3}, {"ul", 0x3}, {"lo", 0x3},
  {"mi", 0x4},
  {"pl", 0x5},
  {"vs", 0x6},
  {"vc", 0x7},
  {"hi", 0x8},
  {"ls", 0x9},
  {"ge", 0xa},
  {"lt", 0xb},
  {"gt", 0xc},
  {"le", 0xd},
  {"al", 0xe}
};
static const struct asm_cond vconds[] =
{
    {"t", 0xf},
    {"e", 0x10}
};

#define UL_BARRIER(L,U,CODE,FEAT) \
  { L, CODE, ARM_FEATURE_CORE_LOW (FEAT) }, \
  { U, CODE, ARM_FEATURE_CORE_LOW (FEAT) }

static struct asm_barrier_opt barrier_opt_names[] =
{
  UL_BARRIER ("sy",	"SY",	 0xf, ARM_EXT_BARRIER),
  UL_BARRIER ("st",	"ST",	 0xe, ARM_EXT_BARRIER),
  UL_BARRIER ("ld",	"LD",	 0xd, ARM_EXT_V8),
  UL_BARRIER ("ish",	"ISH",	 0xb, ARM_EXT_BARRIER),
  UL_BARRIER ("sh",	"SH",	 0xb, ARM_EXT_BARRIER),
  UL_BARRIER ("ishst",	"ISHST", 0xa, ARM_EXT_BARRIER),
  UL_BARRIER ("shst",	"SHST",	 0xa, ARM_EXT_BARRIER),
  UL_BARRIER ("ishld",	"ISHLD", 0x9, ARM_EXT_V8),
  UL_BARRIER ("un",	"UN",	 0x7, ARM_EXT_BARRIER),
  UL_BARRIER ("nsh",	"NSH",	 0x7, ARM_EXT_BARRIER),
  UL_BARRIER ("unst",	"UNST",	 0x6, ARM_EXT_BARRIER),
  UL_BARRIER ("nshst",	"NSHST", 0x6, ARM_EXT_BARRIER),
  UL_BARRIER ("nshld",	"NSHLD", 0x5, ARM_EXT_V8),
  UL_BARRIER ("osh",	"OSH",	 0x3, ARM_EXT_BARRIER),
  UL_BARRIER ("oshst",	"OSHST", 0x2, ARM_EXT_BARRIER),
  UL_BARRIER ("oshld",	"OSHLD", 0x1, ARM_EXT_V8)
};

#undef UL_BARRIER

/* Table of ARM-format instructions.	*/

/* Macros for gluing together operand strings.  N.B. In all cases
   other than OPS0, the trailing OP_stop comes from default
   zero-initialization of the unspecified elements of the array.  */
#define OPS0()		  { OP_stop, }
#define OPS1(a)		  { OP_##a, }
#define OPS2(a,b)	  { OP_##a,OP_##b, }
#define OPS3(a,b,c)	  { OP_##a,OP_##b,OP_##c, }
#define OPS4(a,b,c,d)	  { OP_##a,OP_##b,OP_##c,OP_##d, }
#define OPS5(a,b,c,d,e)	  { OP_##a,OP_##b,OP_##c,OP_##d,OP_##e, }
#define OPS6(a,b,c,d,e,f) { OP_##a,OP_##b,OP_##c,OP_##d,OP_##e,OP_##f, }

/* These macros are similar to the OPSn, but do not prepend the OP_ prefix.
   This is useful when mixing operands for ARM and THUMB, i.e. using the
   MIX_ARM_THUMB_OPERANDS macro.
   In order to use these macros, prefix the number of operands with _
   e.g. _3.  */
#define OPS_1(a)	   { a, }
#define OPS_2(a,b)	   { a,b, }
#define OPS_3(a,b,c)	   { a,b,c, }
#define OPS_4(a,b,c,d)	   { a,b,c,d, }
#define OPS_5(a,b,c,d,e)   { a,b,c,d,e, }
#define OPS_6(a,b,c,d,e,f) { a,b,c,d,e,f, }

/* These macros abstract out the exact format of the mnemonic table and
   save some repeated characters.  */

/* The normal sort of mnemonic; has a Thumb variant; takes a conditional suffix.  */
#define TxCE(mnem, op, top, nops, ops, ae, te) \
  { mnem, OPS##nops ops, OT_csuffix, 0x##op, top, ARM_VARIANT, \
    THUMB_VARIANT, do_##ae, do_##te, 0 }

/* Two variants of the above - TCE for a numeric Thumb opcode, tCE for
   a T_MNEM_xyz enumerator.  */
#define TCE(mnem, aop, top, nops, ops, ae, te) \
      TxCE (mnem, aop, 0x##top, nops, ops, ae, te)
#define tCE(mnem, aop, top, nops, ops, ae, te) \
      TxCE (mnem, aop, T_MNEM##top, nops, ops, ae, te)

/* Second most common sort of mnemonic: has a Thumb variant, takes a conditional
   infix after the third character.  */
#define TxC3(mnem, op, top, nops, ops, ae, te) \
  { mnem, OPS##nops ops, OT_cinfix3, 0x##op, top, ARM_VARIANT, \
    THUMB_VARIANT, do_##ae, do_##te, 0 }
#define TxC3w(mnem, op, top, nops, ops, ae, te) \
  { mnem, OPS##nops ops, OT_cinfix3_deprecated, 0x##op, top, ARM_VARIANT, \
    THUMB_VARIANT, do_##ae, do_##te, 0 }
#define TC3(mnem, aop, top, nops, ops, ae, te) \
      TxC3 (mnem, aop, 0x##top, nops, ops, ae, te)
#define TC3w(mnem, aop, top, nops, ops, ae, te) \
      TxC3w (mnem, aop, 0x##top, nops, ops, ae, te)
#define tC3(mnem, aop, top, nops, ops, ae, te) \
      TxC3 (mnem, aop, T_MNEM##top, nops, ops, ae, te)
#define tC3w(mnem, aop, top, nops, ops, ae, te) \
      TxC3w (mnem, aop, T_MNEM##top, nops, ops, ae, te)

/* Mnemonic that cannot be conditionalized.  The ARM condition-code
   field is still 0xE.  Many of the Thumb variants can be executed
   conditionally, so this is checked separately.  */
#define TUE(mnem, op, top, nops, ops, ae, te)				\
  { mnem, OPS##nops ops, OT_unconditional, 0x##op, 0x##top, ARM_VARIANT, \
    THUMB_VARIANT, do_##ae, do_##te, 0 }

/* Same as TUE but the encoding function for ARM and Thumb modes is the same.
   Used by mnemonics that have very minimal differences in the encoding for
   ARM and Thumb variants and can be handled in a common function.  */
#define TUEc(mnem, op, top, nops, ops, en) \
  { mnem, OPS##nops ops, OT_unconditional, 0x##op, 0x##top, ARM_VARIANT, \
    THUMB_VARIANT, do_##en, do_##en, 0 }

/* Mnemonic that cannot be conditionalized, and bears 0xF in its ARM
   condition code field.  */
#define TUF(mnem, op, top, nops, ops, ae, te)				\
  { mnem, OPS##nops ops, OT_unconditionalF, 0x##op, 0x##top, ARM_VARIANT, \
    THUMB_VARIANT, do_##ae, do_##te, 0 }

/* ARM-only variants of all the above.  */
#define CE(mnem,  op, nops, ops, ae)	\
  { mnem, OPS##nops ops, OT_csuffix, 0x##op, 0x0, ARM_VARIANT, 0, do_##ae, NULL, 0 }

#define C3(mnem, op, nops, ops, ae)	\
  { #mnem, OPS##nops ops, OT_cinfix3, 0x##op, 0x0, ARM_VARIANT, 0, do_##ae, NULL, 0 }

/* Thumb-only variants of TCE and TUE.  */
#define ToC(mnem, top, nops, ops, te) \
  { mnem, OPS##nops ops, OT_csuffix, 0x0, 0x##top, 0, THUMB_VARIANT, NULL, \
    do_##te, 0 }

#define ToU(mnem, top, nops, ops, te) \
  { mnem, OPS##nops ops, OT_unconditional, 0x0, 0x##top, 0, THUMB_VARIANT, \
    NULL, do_##te, 0 }

/* T_MNEM_xyz enumerator variants of ToC.  */
#define toC(mnem, top, nops, ops, te) \
  { mnem, OPS##nops ops, OT_csuffix, 0x0, T_MNEM##top, 0, THUMB_VARIANT, NULL, \
    do_##te, 0 }

/* T_MNEM_xyz enumerator variants of ToU.  */
#define toU(mnem, top, nops, ops, te) \
  { mnem, OPS##nops ops, OT_unconditional, 0x0, T_MNEM##top, 0, THUMB_VARIANT, \
    NULL, do_##te, 0 }

/* Legacy mnemonics that always have conditional infix after the third
   character.  */
#define CL(mnem, op, nops, ops, ae)	\
  { mnem, OPS##nops ops, OT_cinfix3_legacy, \
    0x##op, 0x0, ARM_VARIANT, 0, do_##ae, NULL, 0 }

/* Coprocessor instructions.  Isomorphic between Arm and Thumb-2.  */
#define cCE(mnem,  op, nops, ops, ae)	\
  { mnem, OPS##nops ops, OT_csuffix, 0x##op, 0xe##op, ARM_VARIANT, ARM_VARIANT, do_##ae, do_##ae, 0 }

/* mov instructions that are shared between coprocessor and MVE.  */
#define mcCE(mnem,  op, nops, ops, ae)	\
  { #mnem, OPS##nops ops, OT_csuffix, 0x##op, 0xe##op, ARM_VARIANT, THUMB_VARIANT, do_##ae, do_##ae, 0 }

/* Legacy coprocessor instructions where conditional infix and conditional
   suffix are ambiguous.  For consistency this includes all FPA instructions,
   not just the potentially ambiguous ones.  */
#define cCL(mnem, op, nops, ops, ae)	\
  { mnem, OPS##nops ops, OT_cinfix3_legacy, \
    0x##op, 0xe##op, ARM_VARIANT, ARM_VARIANT, do_##ae, do_##ae, 0 }

/* Coprocessor, takes either a suffix or a position-3 infix
   (for an FPA corner case). */
#define C3E(mnem, op, nops, ops, ae) \
  { mnem, OPS##nops ops, OT_csuf_or_in3, \
    0x##op, 0xe##op, ARM_VARIANT, ARM_VARIANT, do_##ae, do_##ae, 0 }

#define xCM_(m1, m2, m3, op, nops, ops, ae)	\
  { m1 #m2 m3, OPS##nops ops, \
    sizeof (#m2) == 1 ? OT_odd_infix_unc : OT_odd_infix_0 + sizeof (m1) - 1, \
    0x##op, 0x0, ARM_VARIANT, 0, do_##ae, NULL, 0 }

#define CM(m1, m2, op, nops, ops, ae)	\
  xCM_ (m1,   , m2, op, nops, ops, ae),	\
  xCM_ (m1, eq, m2, op, nops, ops, ae),	\
  xCM_ (m1, ne, m2, op, nops, ops, ae),	\
  xCM_ (m1, cs, m2, op, nops, ops, ae),	\
  xCM_ (m1, hs, m2, op, nops, ops, ae),	\
  xCM_ (m1, cc, m2, op, nops, ops, ae),	\
  xCM_ (m1, ul, m2, op, nops, ops, ae),	\
  xCM_ (m1, lo, m2, op, nops, ops, ae),	\
  xCM_ (m1, mi, m2, op, nops, ops, ae),	\
  xCM_ (m1, pl, m2, op, nops, ops, ae),	\
  xCM_ (m1, vs, m2, op, nops, ops, ae),	\
  xCM_ (m1, vc, m2, op, nops, ops, ae),	\
  xCM_ (m1, hi, m2, op, nops, ops, ae),	\
  xCM_ (m1, ls, m2, op, nops, ops, ae),	\
  xCM_ (m1, ge, m2, op, nops, ops, ae),	\
  xCM_ (m1, lt, m2, op, nops, ops, ae),	\
  xCM_ (m1, gt, m2, op, nops, ops, ae),	\
  xCM_ (m1, le, m2, op, nops, ops, ae),	\
  xCM_ (m1, al, m2, op, nops, ops, ae)

#define UE(mnem, op, nops, ops, ae)	\
  { #mnem, OPS##nops ops, OT_unconditional, 0x##op, 0, ARM_VARIANT, 0, do_##ae, NULL, 0 }

#define UF(mnem, op, nops, ops, ae)	\
  { #mnem, OPS##nops ops, OT_unconditionalF, 0x##op, 0, ARM_VARIANT, 0, do_##ae, NULL, 0 }

/* Neon data-processing. ARM versions are unconditional with cond=0xf.
   The Thumb and ARM variants are mostly the same (bits 0-23 and 24/28), so we
   use the same encoding function for each.  */
#define NUF(mnem, op, nops, ops, enc)					\
  { #mnem, OPS##nops ops, OT_unconditionalF, 0x##op, 0x##op,		\
    ARM_VARIANT, THUMB_VARIANT, do_##enc, do_##enc, 0 }

/* Neon data processing, version which indirects through neon_enc_tab for
   the various overloaded versions of opcodes.  */
#define nUF(mnem, op, nops, ops, enc)					\
  { #mnem, OPS##nops ops, OT_unconditionalF, N_MNEM##op, N_MNEM##op,	\
    ARM_VARIANT, THUMB_VARIANT, do_##enc, do_##enc, 0 }

/* Neon insn with conditional suffix for the ARM version, non-overloaded
   version.  */
#define NCE_tag(mnem, op, nops, ops, enc, tag, mve_p)				\
  { #mnem, OPS##nops ops, tag, 0x##op, 0x##op, ARM_VARIANT,		\
    THUMB_VARIANT, do_##enc, do_##enc, mve_p }

#define NCE(mnem, op, nops, ops, enc)					\
   NCE_tag (mnem, op, nops, ops, enc, OT_csuffix, 0)

#define NCEF(mnem, op, nops, ops, enc)					\
    NCE_tag (mnem, op, nops, ops, enc, OT_csuffixF, 0)

/* Neon insn with conditional suffix for the ARM version, overloaded types.  */
#define nCE_tag(mnem, op, nops, ops, enc, tag, mve_p)				\
  { #mnem, OPS##nops ops, tag, N_MNEM##op, N_MNEM##op,		\
    ARM_VARIANT, THUMB_VARIANT, do_##enc, do_##enc, mve_p }

#define nCE(mnem, op, nops, ops, enc)					\
   nCE_tag (mnem, op, nops, ops, enc, OT_csuffix, 0)

#define nCEF(mnem, op, nops, ops, enc)					\
    nCE_tag (mnem, op, nops, ops, enc, OT_csuffixF, 0)

/*   */
#define mCEF(mnem, op, nops, ops, enc)				\
  { #mnem, OPS##nops ops, OT_csuffixF, M_MNEM##op, M_MNEM##op,	\
    ARM_VARIANT, THUMB_VARIANT, do_##enc, do_##enc, 1 }


/* nCEF but for MVE predicated instructions.  */
#define mnCEF(mnem, op, nops, ops, enc)					\
    nCE_tag (mnem, op, nops, ops, enc, OT_csuffixF, 1)

/* nCE but for MVE predicated instructions.  */
#define mnCE(mnem, op, nops, ops, enc)					\
   nCE_tag (mnem, op, nops, ops, enc, OT_csuffix, 1)

/* NUF but for potentially MVE predicated instructions.  */
#define MNUF(mnem, op, nops, ops, enc)					\
  { #mnem, OPS##nops ops, OT_unconditionalF, 0x##op, 0x##op,		\
    ARM_VARIANT, THUMB_VARIANT, do_##enc, do_##enc, 1 }

/* nUF but for potentially MVE predicated instructions.  */
#define mnUF(mnem, op, nops, ops, enc)					\
  { #mnem, OPS##nops ops, OT_unconditionalF, N_MNEM##op, N_MNEM##op,	\
    ARM_VARIANT, THUMB_VARIANT, do_##enc, do_##enc, 1 }

/* ToC but for potentially MVE predicated instructions.  */
#define mToC(mnem, top, nops, ops, te) \
  { mnem, OPS##nops ops, OT_csuffix, 0x0, 0x##top, 0, THUMB_VARIANT, NULL, \
    do_##te, 1 }

/* NCE but for MVE predicated instructions.  */
#define MNCE(mnem, op, nops, ops, enc)					\
   NCE_tag (mnem, op, nops, ops, enc, OT_csuffix, 1)

/* NCEF but for MVE predicated instructions.  */
#define MNCEF(mnem, op, nops, ops, enc)					\
    NCE_tag (mnem, op, nops, ops, enc, OT_csuffixF, 1)
#define do_0 0

static const struct asm_opcode insns[] =
{
#define ARM_VARIANT    & arm_ext_v1 /* Core ARM Instructions.  */
#define THUMB_VARIANT  & arm_ext_v4t
 tCE("and",	0000000, _and,     3, (RR, oRR, SH), arit, t_arit3c),
 tC3("ands",	0100000, _ands,	   3, (RR, oRR, SH), arit, t_arit3c),
 tCE("eor",	0200000, _eor,	   3, (RR, oRR, SH), arit, t_arit3c),
 tC3("eors",	0300000, _eors,	   3, (RR, oRR, SH), arit, t_arit3c),
 tCE("sub",	0400000, _sub,	   3, (RR, oRR, SH), arit, t_add_sub),
 tC3("subs",	0500000, _subs,	   3, (RR, oRR, SH), arit, t_add_sub),
 tCE("add",	0800000, _add,	   3, (RR, oRR, SHG), arit, t_add_sub),
 tC3("adds",	0900000, _adds,	   3, (RR, oRR, SHG), arit, t_add_sub),
 tCE("adc",	0a00000, _adc,	   3, (RR, oRR, SH), arit, t_arit3c),
 tC3("adcs",	0b00000, _adcs,	   3, (RR, oRR, SH), arit, t_arit3c),
 tCE("sbc",	0c00000, _sbc,	   3, (RR, oRR, SH), arit, t_arit3),
 tC3("sbcs",	0d00000, _sbcs,	   3, (RR, oRR, SH), arit, t_arit3),
 tCE("orr",	1800000, _orr,	   3, (RR, oRR, SH), arit, t_arit3c),
 tC3("orrs",	1900000, _orrs,	   3, (RR, oRR, SH), arit, t_arit3c),
 tCE("bic",	1c00000, _bic,	   3, (RR, oRR, SH), arit, t_arit3),
 tC3("bics",	1d00000, _bics,	   3, (RR, oRR, SH), arit, t_arit3),

 /* The p-variants of tst/cmp/cmn/teq (below) are the pre-V6 mechanism
    for setting PSR flag bits.  They are obsolete in V6 and do not
    have Thumb equivalents. */
 tCE("tst",	1100000, _tst,	   2, (RR, SH),      cmp,  t_mvn_tst),
 tC3w("tsts",	1100000, _tst,	   2, (RR, SH),      cmp,  t_mvn_tst),
  CL("tstp",	110f000,     	   2, (RR, SH),      cmp),
 tCE("cmp",	1500000, _cmp,	   2, (RR, SH),      cmp,  t_mov_cmp),
 tC3w("cmps",	1500000, _cmp,	   2, (RR, SH),      cmp,  t_mov_cmp),
  CL("cmpp",	150f000,     	   2, (RR, SH),      cmp),
 tCE("cmn",	1700000, _cmn,	   2, (RR, SH),      cmp,  t_mvn_tst),
 tC3w("cmns",	1700000, _cmn,	   2, (RR, SH),      cmp,  t_mvn_tst),
  CL("cmnp",	170f000,     	   2, (RR, SH),      cmp),

 tCE("mov",	1a00000, _mov,	   2, (RR, SH),      mov,  t_mov_cmp),
 tC3("movs",	1b00000, _movs,	   2, (RR, SHG),     mov,  t_mov_cmp),
 tCE("mvn",	1e00000, _mvn,	   2, (RR, SH),      mov,  t_mvn_tst),
 tC3("mvns",	1f00000, _mvns,	   2, (RR, SH),      mov,  t_mvn_tst),

 tCE("ldr",	4100000, _ldr,	   2, (RR, ADDRGLDR),ldst, t_ldst),
 tC3("ldrb",	4500000, _ldrb,	   2, (RRnpc_npcsp, ADDRGLDR),ldst, t_ldst),
 tCE("str",	4000000, _str,	   _2, (MIX_ARM_THUMB_OPERANDS (OP_RR,
								OP_RRnpc),
					OP_ADDRGLDR),ldst, t_ldst),
 tC3("strb",	4400000, _strb,	   2, (RRnpc_npcsp, ADDRGLDR),ldst, t_ldst),

 tCE("stm",	8800000, _stmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
 tC3("stmia",	8800000, _stmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
 tC3("stmea",	8800000, _stmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
 tCE("ldm",	8900000, _ldmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
 tC3("ldmia",	8900000, _ldmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
 tC3("ldmfd",	8900000, _ldmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),

 tCE("b",	a000000, _b,	   1, (EXPr),	     branch, t_branch),
 TCE("bl",	b000000, f000f800, 1, (EXPr),	     bl, t_branch23),

  /* Pseudo ops.  */
 tCE("adr",	28f0000, _adr,	   2, (RR, EXP),     adr,  t_adr),
  C3(adrl,	28f0000,           2, (RR, EXP),     adrl),
 tCE("nop",	1a00000, _nop,	   1, (oI255c),	     nop,  t_nop),
 tCE("udf",	7f000f0, _udf,     1, (oIffffb),     bkpt, t_udf),

  /* Thumb-compatibility pseudo ops.  */
 tCE("lsl",	1a00000, _lsl,	   3, (RR, oRR, SH), shift, t_shift),
 tC3("lsls",	1b00000, _lsls,	   3, (RR, oRR, SH), shift, t_shift),
 tCE("lsr",	1a00020, _lsr,	   3, (RR, oRR, SH), shift, t_shift),
 tC3("lsrs",	1b00020, _lsrs,	   3, (RR, oRR, SH), shift, t_shift),
 tCE("asr",	1a00040, _asr,	   3, (RR, oRR, SH), shift, t_shift),
 tC3("asrs",      1b00040, _asrs,     3, (RR, oRR, SH), shift, t_shift),
 tCE("ror",	1a00060, _ror,	   3, (RR, oRR, SH), shift, t_shift),
 tC3("rors",	1b00060, _rors,	   3, (RR, oRR, SH), shift, t_shift),
 tCE("neg",	2600000, _neg,	   2, (RR, RR),      rd_rn, t_neg),
 tC3("negs",	2700000, _negs,	   2, (RR, RR),      rd_rn, t_neg),
 tCE("push",	92d0000, _push,     1, (REGLST),	     push_pop, t_push_pop),
 tCE("pop",	8bd0000, _pop,	   1, (REGLST),	     push_pop, t_push_pop),

 /* These may simplify to neg.  */
 TCE("rsb",	0600000, ebc00000, 3, (RR, oRR, SH), arit, t_rsb),
 TC3("rsbs",	0700000, ebd00000, 3, (RR, oRR, SH), arit, t_rsb),

#undef THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_os

 TCE("swi",	f000000, df00,     1, (EXPi),        swi, t_swi),
 TCE("svc",	f000000, df00,     1, (EXPi),        swi, t_swi),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6

 TCE("cpy",       1a00000, 4600,     2, (RR, RR),      rd_rm, t_cpy),

 /* V1 instructions with no Thumb analogue prior to V6T2.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TCE("teq",	1300000, ea900f00, 2, (RR, SH),      cmp,  t_mvn_tst),
 TC3w("teqs",	1300000, ea900f00, 2, (RR, SH),      cmp,  t_mvn_tst),
  CL("teqp",	130f000,           2, (RR, SH),      cmp),

 TC3("ldrt",	4300000, f8500e00, 2, (RRnpc_npcsp, ADDR),ldstt, t_ldstt),
 TC3("ldrbt",	4700000, f8100e00, 2, (RRnpc_npcsp, ADDR),ldstt, t_ldstt),
 TC3("strt",	4200000, f8400e00, 2, (RR_npcsp, ADDR),   ldstt, t_ldstt),
 TC3("strbt",	4600000, f8000e00, 2, (RRnpc_npcsp, ADDR),ldstt, t_ldstt),

 TC3("stmdb",	9000000, e9000000, 2, (RRw, REGLST), ldmstm, t_ldmstm),
 TC3("stmfd",     9000000, e9000000, 2, (RRw, REGLST), ldmstm, t_ldmstm),

 TC3("ldmdb",	9100000, e9100000, 2, (RRw, REGLST), ldmstm, t_ldmstm),
 TC3("ldmea",	9100000, e9100000, 2, (RRw, REGLST), ldmstm, t_ldmstm),

 /* V1 instructions with no Thumb analogue at all.  */
  CE("rsc",	0e00000,	   3, (RR, oRR, SH), arit),
  C3(rscs,	0f00000,	   3, (RR, oRR, SH), arit),

  C3(stmib,	9800000,	   2, (RRw, REGLST), ldmstm),
  C3(stmfa,	9800000,	   2, (RRw, REGLST), ldmstm),
  C3(stmda,	8000000,	   2, (RRw, REGLST), ldmstm),
  C3(stmed,	8000000,	   2, (RRw, REGLST), ldmstm),
  C3(ldmib,	9900000,	   2, (RRw, REGLST), ldmstm),
  C3(ldmed,	9900000,	   2, (RRw, REGLST), ldmstm),
  C3(ldmda,	8100000,	   2, (RRw, REGLST), ldmstm),
  C3(ldmfa,	8100000,	   2, (RRw, REGLST), ldmstm),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v2	/* ARM 2 - multiplies.	*/
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v4t

 tCE("mul",	0000090, _mul,	   3, (RRnpc, RRnpc, oRR), mul, t_mul),
 tC3("muls",	0100090, _muls,	   3, (RRnpc, RRnpc, oRR), mul, t_mul),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TCE("mla",	0200090, fb000000, 4, (RRnpc, RRnpc, RRnpc, RRnpc), mlas, t_mla),
  C3(mlas,	0300090,           4, (RRnpc, RRnpc, RRnpc, RRnpc), mlas),

  /* Generic coprocessor instructions.	*/
 TCE("cdp",	e000000, ee000000, 6, (RCP, I15b, RCN, RCN, RCN, oI7b), cdp,    cdp),
 TCE("ldc",	c100000, ec100000, 3, (RCP, RCN, ADDRGLDC),	        lstc,   lstc),
 TC3("ldcl",	c500000, ec500000, 3, (RCP, RCN, ADDRGLDC),	        lstc,   lstc),
 TCE("stc",	c000000, ec000000, 3, (RCP, RCN, ADDRGLDC),	        lstc,   lstc),
 TC3("stcl",	c400000, ec400000, 3, (RCP, RCN, ADDRGLDC),	        lstc,   lstc),
 TCE("mcr",	e000010, ee000010, 6, (RCP, I7b, RR, RCN, RCN, oI7b),   co_reg, co_reg),
 TCE("mrc",	e100010, ee100010, 6, (RCP, I7b, APSR_RR, RCN, RCN, oI7b),   co_reg, co_reg),

#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_ext_v2s /* ARM 3 - swp instructions.  */

  CE("swp",	1000090,           3, (RRnpc, RRnpc, RRnpcb), rd_rm_rn),
  C3(swpb,	1400090,           3, (RRnpc, RRnpc, RRnpcb), rd_rm_rn),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v3	/* ARM 6 Status register instructions.	*/
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_msr

 TCE("mrs",	1000000, f3e08000, 2, (RRnpc, rPSR), mrs, t_mrs),
 TCE("msr",	120f000, f3808000, 2, (wPSR, RR_EXi), msr, t_msr),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v3m	 /* ARM 7M long multiplies.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TCE("smull",	0c00090, fb800000, 4, (RRnpc, RRnpc, RRnpc, RRnpc), mull, t_mull),
  CM("smull","s",	0d00090,           4, (RRnpc, RRnpc, RRnpc, RRnpc), mull),
 TCE("umull",	0800090, fba00000, 4, (RRnpc, RRnpc, RRnpc, RRnpc), mull, t_mull),
  CM("umull","s",	0900090,           4, (RRnpc, RRnpc, RRnpc, RRnpc), mull),
 TCE("smlal",	0e00090, fbc00000, 4, (RRnpc, RRnpc, RRnpc, RRnpc), mull, t_mull),
  CM("smlal","s",	0f00090,           4, (RRnpc, RRnpc, RRnpc, RRnpc), mull),
 TCE("umlal",	0a00090, fbe00000, 4, (RRnpc, RRnpc, RRnpc, RRnpc), mull, t_mull),
  CM("umlal","s",	0b00090,           4, (RRnpc, RRnpc, RRnpc, RRnpc), mull),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v4	/* ARM Architecture 4.	*/
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v4t

 tC3("ldrh",	01000b0, _ldrh,     2, (RRnpc_npcsp, ADDRGLDRS), ldstv4, t_ldst),
 tC3("strh",	00000b0, _strh,     2, (RRnpc_npcsp, ADDRGLDRS), ldstv4, t_ldst),
 tC3("ldrsh",	01000f0, _ldrsh,    2, (RRnpc_npcsp, ADDRGLDRS), ldstv4, t_ldst),
 tC3("ldrsb",	01000d0, _ldrsb,    2, (RRnpc_npcsp, ADDRGLDRS), ldstv4, t_ldst),
 tC3("ldsh",	01000f0, _ldrsh,    2, (RRnpc_npcsp, ADDRGLDRS), ldstv4, t_ldst),
 tC3("ldsb",	01000d0, _ldrsb,    2, (RRnpc_npcsp, ADDRGLDRS), ldstv4, t_ldst),

#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_ext_v4t_5

  /* ARM Architecture 4T.  */
  /* Note: bx (and blx) are required on V5, even if the processor does
     not support Thumb.	 */
 TCE("bx",	12fff10, 4700, 1, (RR),	bx, t_bx),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v5 /*  ARM Architecture 5T.	 */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v5t

  /* Note: blx has 2 variants; the .value coded here is for
     BLX(2).  Only this variant has conditional execution.  */
 TCE("blx",	12fff30, 4780, 1, (RR_EXr),			    blx,  t_blx),
 TUE("bkpt",	1200070, be00, 1, (oIffffb),			    bkpt, t_bkpt),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TCE("clz",	16f0f10, fab0f080, 2, (RRnpc, RRnpc),		        rd_rm,  t_clz),
 TUF("ldc2",	c100000, fc100000, 3, (RCP, RCN, ADDRGLDC),	        lstc,	lstc),
 TUF("ldc2l",	c500000, fc500000, 3, (RCP, RCN, ADDRGLDC),		        lstc,	lstc),
 TUF("stc2",	c000000, fc000000, 3, (RCP, RCN, ADDRGLDC),	        lstc,	lstc),
 TUF("stc2l",	c400000, fc400000, 3, (RCP, RCN, ADDRGLDC),		        lstc,	lstc),
 TUF("cdp2",	e000000, fe000000, 6, (RCP, I15b, RCN, RCN, RCN, oI7b), cdp,    cdp),
 TUF("mcr2",	e000010, fe000010, 6, (RCP, I7b, RR, RCN, RCN, oI7b),   co_reg, co_reg),
 TUF("mrc2",	e100010, fe100010, 6, (RCP, I7b, RR, RCN, RCN, oI7b),   co_reg, co_reg),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v5exp /*  ARM Architecture 5TExP.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v5exp

 TCE("smlabb",	1000080, fb100000, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smla, t_mla),
 TCE("smlatb",	10000a0, fb100020, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smla, t_mla),
 TCE("smlabt",	10000c0, fb100010, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smla, t_mla),
 TCE("smlatt",	10000e0, fb100030, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smla, t_mla),

 TCE("smlawb",	1200080, fb300000, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smla, t_mla),
 TCE("smlawt",	12000c0, fb300010, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smla, t_mla),

 TCE("smlalbb",	1400080, fbc00080, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smlal, t_mlal),
 TCE("smlaltb",	14000a0, fbc000a0, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smlal, t_mlal),
 TCE("smlalbt",	14000c0, fbc00090, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smlal, t_mlal),
 TCE("smlaltt",	14000e0, fbc000b0, 4, (RRnpc, RRnpc, RRnpc, RRnpc),   smlal, t_mlal),

 TCE("smulbb",	1600080, fb10f000, 3, (RRnpc, RRnpc, RRnpc),	    smul, t_simd),
 TCE("smultb",	16000a0, fb10f020, 3, (RRnpc, RRnpc, RRnpc),	    smul, t_simd),
 TCE("smulbt",	16000c0, fb10f010, 3, (RRnpc, RRnpc, RRnpc),	    smul, t_simd),
 TCE("smultt",	16000e0, fb10f030, 3, (RRnpc, RRnpc, RRnpc),	    smul, t_simd),

 TCE("smulwb",	12000a0, fb30f000, 3, (RRnpc, RRnpc, RRnpc),	    smul, t_simd),
 TCE("smulwt",	12000e0, fb30f010, 3, (RRnpc, RRnpc, RRnpc),	    smul, t_simd),

 TCE("qadd",	1000050, fa80f080, 3, (RRnpc, RRnpc, RRnpc),	    rd_rm_rn, t_simd2),
 TCE("qdadd",	1400050, fa80f090, 3, (RRnpc, RRnpc, RRnpc),	    rd_rm_rn, t_simd2),
 TCE("qsub",	1200050, fa80f0a0, 3, (RRnpc, RRnpc, RRnpc),	    rd_rm_rn, t_simd2),
 TCE("qdsub",	1600050, fa80f0b0, 3, (RRnpc, RRnpc, RRnpc),	    rd_rm_rn, t_simd2),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v5e /*  ARM Architecture 5TE.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TUF("pld",	450f000, f810f000, 1, (ADDR),		     pld,  t_pld),
 TC3("ldrd",	00000d0, e8500000, 3, (RRnpc_npcsp, oRRnpc_npcsp, ADDRGLDRS),
     ldrd, t_ldstd),
 TC3("strd",	00000f0, e8400000, 3, (RRnpc_npcsp, oRRnpc_npcsp,
				       ADDRGLDRS), ldrd, t_ldstd),

 TCE("mcrr",	c400000, ec400000, 5, (RCP, I15b, RRnpc, RRnpc, RCN), co_reg2c, co_reg2c),
 TCE("mrrc",	c500000, ec500000, 5, (RCP, I15b, RRnpc, RRnpc, RCN), co_reg2c, co_reg2c),

#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_ext_v5j /*  ARM Architecture 5TEJ.  */

 TCE("bxj",	12fff20, f3c08f00, 1, (RR),			  bxj, t_bxj),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v6 /*  ARM V6.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6

 TUF("cpsie",     1080000, b660,     2, (CPSF, oI31b),              cpsi,   t_cpsi),
 TUF("cpsid",     10c0000, b670,     2, (CPSF, oI31b),              cpsi,   t_cpsi),
 tCE("rev",       6bf0f30, _rev,      2, (RRnpc, RRnpc),             rd_rm,  t_rev),
 tCE("rev16",     6bf0fb0, _rev16,    2, (RRnpc, RRnpc),             rd_rm,  t_rev),
 tCE("revsh",     6ff0fb0, _revsh,    2, (RRnpc, RRnpc),             rd_rm,  t_rev),
 tCE("sxth",      6bf0070, _sxth,     3, (RRnpc, RRnpc, oROR),       sxth,   t_sxth),
 tCE("uxth",      6ff0070, _uxth,     3, (RRnpc, RRnpc, oROR),       sxth,   t_sxth),
 tCE("sxtb",      6af0070, _sxtb,     3, (RRnpc, RRnpc, oROR),       sxth,   t_sxth),
 tCE("uxtb",      6ef0070, _uxtb,     3, (RRnpc, RRnpc, oROR),       sxth,   t_sxth),
 TUF("setend",    1010000, b650,     1, (ENDI),                     setend, t_setend),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2_v8m

 TCE("ldrex",	1900f9f, e8500f00, 2, (RRnpc_npcsp, ADDR),	  ldrex, t_ldrex),
 TCE("strex",	1800f90, e8400000, 3, (RRnpc_npcsp, RRnpc_npcsp, ADDR),
				      strex,  t_strex),
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TUF("mcrr2",	c400000, fc400000, 5, (RCP, I15b, RRnpc, RRnpc, RCN), co_reg2c, co_reg2c),
 TUF("mrrc2",	c500000, fc500000, 5, (RCP, I15b, RRnpc, RRnpc, RCN), co_reg2c, co_reg2c),

 TCE("ssat",	6a00010, f3000000, 4, (RRnpc, I32, RRnpc, oSHllar),ssat,   t_ssat),
 TCE("usat",	6e00010, f3800000, 4, (RRnpc, I31, RRnpc, oSHllar),usat,   t_usat),

/*  ARM V6 not included in V7M.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6_notm
 TUF("rfeia",	8900a00, e990c000, 1, (RRw),			   rfe, rfe),
 TUF("rfe",	8900a00, e990c000, 1, (RRw),			   rfe, rfe),
  UF(rfeib,	9900a00,           1, (RRw),			   rfe),
  UF(rfeda,	8100a00,           1, (RRw),			   rfe),
 TUF("rfedb",	9100a00, e810c000, 1, (RRw),			   rfe, rfe),
 TUF("rfefd",	8900a00, e990c000, 1, (RRw),			   rfe, rfe),
  UF(rfefa,	8100a00,           1, (RRw),			   rfe),
 TUF("rfeea",	9100a00, e810c000, 1, (RRw),			   rfe, rfe),
  UF(rfeed,	9900a00,           1, (RRw),			   rfe),
 TUF("srsia",	8c00500, e980c000, 2, (oRRw, I31w),		   srs,  srs),
 TUF("srs",	8c00500, e980c000, 2, (oRRw, I31w),		   srs,  srs),
 TUF("srsea",	8c00500, e980c000, 2, (oRRw, I31w),		   srs,  srs),
  UF(srsib,	9c00500,           2, (oRRw, I31w),		   srs),
  UF(srsfa,	9c00500,           2, (oRRw, I31w),		   srs),
  UF(srsda,	8400500,	   2, (oRRw, I31w),		   srs),
  UF(srsed,	8400500,	   2, (oRRw, I31w),		   srs),
 TUF("srsdb",	9400500, e800c000, 2, (oRRw, I31w),		   srs,  srs),
 TUF("srsfd",	9400500, e800c000, 2, (oRRw, I31w),		   srs,  srs),
 TUF("cps",	1020000, f3af8100, 1, (I31b),			  imm0, t_cps),

/*  ARM V6 not included in V7M (eg. integer SIMD).  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6_dsp
 TCE("pkhbt",	6800010, eac00000, 4, (RRnpc, RRnpc, RRnpc, oSHll),   pkhbt, t_pkhbt),
 TCE("pkhtb",	6800050, eac00020, 4, (RRnpc, RRnpc, RRnpc, oSHar),   pkhtb, t_pkhtb),
 TCE("qadd16",	6200f10, fa90f010, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("qadd8",	6200f90, fa80f010, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("qasx",	6200f30, faa0f010, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for QASX.  */
 TCE("qaddsubx",6200f30, faa0f010, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("qsax",	6200f50, fae0f010, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for QSAX.  */
 TCE("qsubaddx",6200f50, fae0f010, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("qsub16",	6200f70, fad0f010, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("qsub8",	6200ff0, fac0f010, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("sadd16",	6100f10, fa90f000, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("sadd8",	6100f90, fa80f000, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("sasx",	6100f30, faa0f000, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for SASX.  */
 TCE("saddsubx",6100f30, faa0f000, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("shadd16",	6300f10, fa90f020, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("shadd8",	6300f90, fa80f020, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("shasx",   6300f30, faa0f020, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for SHASX.  */
 TCE("shaddsubx", 6300f30, faa0f020, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("shsax",     6300f50, fae0f020, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for SHSAX.  */
 TCE("shsubaddx", 6300f50, fae0f020, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("shsub16",	6300f70, fad0f020, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("shsub8",	6300ff0, fac0f020, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("ssax",	6100f50, fae0f000, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for SSAX.  */
 TCE("ssubaddx",6100f50, fae0f000, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("ssub16",	6100f70, fad0f000, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("ssub8",	6100ff0, fac0f000, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uadd16",	6500f10, fa90f040, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uadd8",	6500f90, fa80f040, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uasx",	6500f30, faa0f040, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for UASX.  */
 TCE("uaddsubx",6500f30, faa0f040, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uhadd16",	6700f10, fa90f060, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uhadd8",	6700f90, fa80f060, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uhasx",   6700f30, faa0f060, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for UHASX.  */
 TCE("uhaddsubx", 6700f30, faa0f060, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uhsax",     6700f50, fae0f060, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for UHSAX.  */
 TCE("uhsubaddx", 6700f50, fae0f060, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uhsub16",	6700f70, fad0f060, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uhsub8",	6700ff0, fac0f060, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uqadd16",	6600f10, fa90f050, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uqadd8",	6600f90, fa80f050, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uqasx",   6600f30, faa0f050, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for UQASX.  */
 TCE("uqaddsubx", 6600f30, faa0f050, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uqsax",     6600f50, fae0f050, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for UQSAX.  */
 TCE("uqsubaddx", 6600f50, fae0f050, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uqsub16",	6600f70, fad0f050, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("uqsub8",	6600ff0, fac0f050, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("usub16",	6500f70, fad0f040, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("usax",	6500f50, fae0f040, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 /* Old name for USAX.  */
 TCE("usubaddx",6500f50, fae0f040, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("usub8",	6500ff0, fac0f040, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("sxtah",	6b00070, fa00f080, 4, (RRnpc, RRnpc, RRnpc, oROR), sxtah, t_sxtah),
 TCE("sxtab16",	6800070, fa20f080, 4, (RRnpc, RRnpc, RRnpc, oROR), sxtah, t_sxtah),
 TCE("sxtab",	6a00070, fa40f080, 4, (RRnpc, RRnpc, RRnpc, oROR), sxtah, t_sxtah),
 TCE("sxtb16",	68f0070, fa2ff080, 3, (RRnpc, RRnpc, oROR),	   sxth,  t_sxth),
 TCE("uxtah",	6f00070, fa10f080, 4, (RRnpc, RRnpc, RRnpc, oROR), sxtah, t_sxtah),
 TCE("uxtab16",	6c00070, fa30f080, 4, (RRnpc, RRnpc, RRnpc, oROR), sxtah, t_sxtah),
 TCE("uxtab",	6e00070, fa50f080, 4, (RRnpc, RRnpc, RRnpc, oROR), sxtah, t_sxtah),
 TCE("uxtb16",	6cf0070, fa3ff080, 3, (RRnpc, RRnpc, oROR),	   sxth,  t_sxth),
 TCE("sel",	6800fb0, faa0f080, 3, (RRnpc, RRnpc, RRnpc),	   rd_rn_rm, t_simd),
 TCE("smlad",	7000010, fb200000, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla, t_mla),
 TCE("smladx",	7000030, fb200010, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla, t_mla),
 TCE("smlald",	7400010, fbc000c0, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smlal,t_mlal),
 TCE("smlaldx",	7400030, fbc000d0, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smlal,t_mlal),
 TCE("smlsd",	7000050, fb400000, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla, t_mla),
 TCE("smlsdx",	7000070, fb400010, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla, t_mla),
 TCE("smlsld",	7400050, fbd000c0, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smlal,t_mlal),
 TCE("smlsldx",	7400070, fbd000d0, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smlal,t_mlal),
 TCE("smmla",	7500010, fb500000, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla, t_mla),
 TCE("smmlar",	7500030, fb500010, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla, t_mla),
 TCE("smmls",	75000d0, fb600000, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla, t_mla),
 TCE("smmlsr",	75000f0, fb600010, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla, t_mla),
 TCE("smmul",	750f010, fb50f000, 3, (RRnpc, RRnpc, RRnpc),	   smul, t_simd),
 TCE("smmulr",	750f030, fb50f010, 3, (RRnpc, RRnpc, RRnpc),	   smul, t_simd),
 TCE("smuad",	700f010, fb20f000, 3, (RRnpc, RRnpc, RRnpc),	   smul, t_simd),
 TCE("smuadx",	700f030, fb20f010, 3, (RRnpc, RRnpc, RRnpc),	   smul, t_simd),
 TCE("smusd",	700f050, fb40f000, 3, (RRnpc, RRnpc, RRnpc),	   smul, t_simd),
 TCE("smusdx",	700f070, fb40f010, 3, (RRnpc, RRnpc, RRnpc),	   smul, t_simd),
 TCE("ssat16",	6a00f30, f3200000, 3, (RRnpc, I16, RRnpc),	   ssat16, t_ssat16),
 TCE("umaal",	0400090, fbe00060, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smlal,  t_mlal),
 TCE("usad8",	780f010, fb70f000, 3, (RRnpc, RRnpc, RRnpc),	   smul,   t_simd),
 TCE("usada8",	7800010, fb700000, 4, (RRnpc, RRnpc, RRnpc, RRnpc),smla,   t_mla),
 TCE("usat16",	6e00f30, f3a00000, 3, (RRnpc, I15, RRnpc),	   usat16, t_usat16),

#undef  ARM_VARIANT
#define ARM_VARIANT   & arm_ext_v6k_v6t2
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_v6k_v6t2

 tCE("yield",	320f001, _yield,    0, (), noargs, t_hint),
 tCE("wfe",	320f002, _wfe,      0, (), noargs, t_hint),
 tCE("wfi",	320f003, _wfi,      0, (), noargs, t_hint),
 tCE("sev",	320f004, _sev,      0, (), noargs, t_hint),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6_notm
 TCE("ldrexd",	1b00f9f, e8d0007f, 3, (RRnpc_npcsp, oRRnpc_npcsp, RRnpcb),
				      ldrexd, t_ldrexd),
 TCE("strexd",	1a00f90, e8c00070, 4, (RRnpc_npcsp, RRnpc_npcsp, oRRnpc_npcsp,
				       RRnpcb), strexd, t_strexd),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2_v8m
 TCE("ldrexb",	1d00f9f, e8d00f4f, 2, (RRnpc_npcsp,RRnpcb),
     rd_rn,  rd_rn),
 TCE("ldrexh",	1f00f9f, e8d00f5f, 2, (RRnpc_npcsp, RRnpcb),
     rd_rn,  rd_rn),
 TCE("strexb",	1c00f90, e8c00f40, 3, (RRnpc_npcsp, RRnpc_npcsp, ADDR),
     strex, t_strexbh),
 TCE("strexh",	1e00f90, e8c00f50, 3, (RRnpc_npcsp, RRnpc_npcsp, ADDR),
     strex, t_strexbh),
 TUF("clrex",	57ff01f, f3bf8f2f, 0, (),			      noargs, noargs),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_sec
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_sec

 TCE("smc",	1600070, f7f08000, 1, (EXPi), smc, t_smc),

#undef	ARM_VARIANT
#define	ARM_VARIANT    & arm_ext_virt
#undef	THUMB_VARIANT
#define	THUMB_VARIANT    & arm_ext_virt

 TCE("hvc",	1400070, f7e08000, 1, (EXPi), hvc, t_hvc),
 TCE("eret",	160006e, f3de8f00, 0, (), noargs, noargs),

#undef	ARM_VARIANT
#define	ARM_VARIANT    & arm_ext_pan
#undef	THUMB_VARIANT
#define	THUMB_VARIANT  & arm_ext_pan

 TUF("setpan",	1100000, b610, 1, (I7), setpan, t_setpan),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v6t2
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TCE("bfc",	7c0001f, f36f0000, 3, (RRnpc, I31, I32),	   bfc, t_bfc),
 TCE("bfi",	7c00010, f3600000, 4, (RRnpc, RRnpc_I0, I31, I32), bfi, t_bfi),
 TCE("sbfx",	7a00050, f3400000, 4, (RR, RR, I31, I32),	   bfx, t_bfx),
 TCE("ubfx",	7e00050, f3c00000, 4, (RR, RR, I31, I32),	   bfx, t_bfx),

 TCE("mls",	0600090, fb000010, 4, (RRnpc, RRnpc, RRnpc, RRnpc), mlas, t_mla),
 TCE("rbit",	6ff0f30, fa90f0a0, 2, (RR, RR),			    rd_rm, t_rbit),

 TC3("ldrht",	03000b0, f8300e00, 2, (RRnpc_npcsp, ADDR), ldsttv4, t_ldstt),
 TC3("ldrsht",	03000f0, f9300e00, 2, (RRnpc_npcsp, ADDR), ldsttv4, t_ldstt),
 TC3("ldrsbt",	03000d0, f9100e00, 2, (RRnpc_npcsp, ADDR), ldsttv4, t_ldstt),
 TC3("strht",	02000b0, f8200e00, 2, (RRnpc_npcsp, ADDR), ldsttv4, t_ldstt),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v3
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TUE("csdb",	320f014, f3af8014, 0, (), noargs, t_csdb),
 TUF("ssbb",	57ff040, f3bf8f40, 0, (), noargs, t_csdb),
 TUF("pssbb",	57ff044, f3bf8f44, 0, (), noargs, t_csdb),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v6t2
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2_v8m
 TCE("movw",	3000000, f2400000, 2, (RRnpc, HALF),		    mov16, t_mov16),
 TCE("movt",	3400000, f2c00000, 2, (RRnpc, HALF),		    mov16, t_mov16),

 /* Thumb-only instructions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT NULL
  TUE("cbnz",     0,           b900,     2, (RR, EXP), 0, t_cbz),
  TUE("cbz",      0,           b100,     2, (RR, EXP), 0, t_cbz),

 /* ARM does not really have an IT instruction, so always allow it.
    The opcode is copied from Thumb in order to allow warnings in
    -mimplicit-it=[never | arm] modes.  */
#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_ext_v1
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 TUE("it",        bf08,        bf08,     1, (COND),   it,    t_it),
 TUE("itt",       bf0c,        bf0c,     1, (COND),   it,    t_it),
 TUE("ite",       bf04,        bf04,     1, (COND),   it,    t_it),
 TUE("ittt",      bf0e,        bf0e,     1, (COND),   it,    t_it),
 TUE("itet",      bf06,        bf06,     1, (COND),   it,    t_it),
 TUE("itte",      bf0a,        bf0a,     1, (COND),   it,    t_it),
 TUE("itee",      bf02,        bf02,     1, (COND),   it,    t_it),
 TUE("itttt",     bf0f,        bf0f,     1, (COND),   it,    t_it),
 TUE("itett",     bf07,        bf07,     1, (COND),   it,    t_it),
 TUE("ittet",     bf0b,        bf0b,     1, (COND),   it,    t_it),
 TUE("iteet",     bf03,        bf03,     1, (COND),   it,    t_it),
 TUE("ittte",     bf0d,        bf0d,     1, (COND),   it,    t_it),
 TUE("itete",     bf05,        bf05,     1, (COND),   it,    t_it),
 TUE("ittee",     bf09,        bf09,     1, (COND),   it,    t_it),
 TUE("iteee",     bf01,        bf01,     1, (COND),   it,    t_it),
 /* ARM/Thumb-2 instructions with no Thumb-1 equivalent.  */
 TC3("rrx",       01a00060, ea4f0030, 2, (RR, RR), rd_rm, t_rrx),
 TC3("rrxs",      01b00060, ea5f0030, 2, (RR, RR), rd_rm, t_rrx),

 /* Thumb2 only instructions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT  NULL

 TCE("addw",	0, f2000000, 3, (RR, RR, EXPi), 0, t_add_sub_w),
 TCE("subw",	0, f2a00000, 3, (RR, RR, EXPi), 0, t_add_sub_w),
 TCE("orn",       0, ea600000, 3, (RR, oRR, SH),  0, t_orn),
 TCE("orns",      0, ea700000, 3, (RR, oRR, SH),  0, t_orn),
 TCE("tbb",       0, e8d0f000, 1, (TB), 0, t_tb),
 TCE("tbh",       0, e8d0f010, 1, (TB), 0, t_tb),

 /* Hardware division instructions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_adiv
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_div

 TCE("sdiv",	710f010, fb90f0f0, 3, (RR, oRR, RR), div, t_div),
 TCE("udiv",	730f010, fbb0f0f0, 3, (RR, oRR, RR), div, t_div),

 /* ARM V6M/V7 instructions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_barrier
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_barrier

 TUF("dmb",	57ff050, f3bf8f50, 1, (oBARRIER_I15), barrier, barrier),
 TUF("dsb",	57ff040, f3bf8f40, 1, (oBARRIER_I15), barrier, barrier),
 TUF("isb",	57ff060, f3bf8f60, 1, (oBARRIER_I15), barrier, barrier),

 /* ARM V7 instructions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_v7
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v7

 TUF("pli",	450f000, f910f000, 1, (ADDR),	  pli,	    t_pld),
 TCE("dbg",	320f0f0, f3af80f0, 1, (I15),	  dbg,	    t_dbg),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_mp
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_mp

 TUF("pldw",	410f000, f830f000, 1, (ADDR),	pld,	t_pld),

 /* AArchv8 instructions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT   & arm_ext_v8

/* Instructions shared between armv8-a and armv8-m.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_atomics

 TCE("lda",	1900c9f, e8d00faf, 2, (RRnpc, RRnpcb),	rd_rn,	rd_rn),
 TCE("ldab",	1d00c9f, e8d00f8f, 2, (RRnpc, RRnpcb),	rd_rn,  rd_rn),
 TCE("ldah",	1f00c9f, e8d00f9f, 2, (RRnpc, RRnpcb),	rd_rn,  rd_rn),
 TCE("stl",	180fc90, e8c00faf, 2, (RRnpc, RRnpcb),	rm_rn,  rd_rn),
 TCE("stlb",	1c0fc90, e8c00f8f, 2, (RRnpc, RRnpcb),	rm_rn,  rd_rn),
 TCE("stlh",	1e0fc90, e8c00f9f, 2, (RRnpc, RRnpcb),	rm_rn,  rd_rn),
 TCE("ldaex",	1900e9f, e8d00fef, 2, (RRnpc, RRnpcb),	rd_rn,	rd_rn),
 TCE("ldaexb",	1d00e9f, e8d00fcf, 2, (RRnpc,RRnpcb),	rd_rn,  rd_rn),
 TCE("ldaexh",	1f00e9f, e8d00fdf, 2, (RRnpc, RRnpcb),	rd_rn,  rd_rn),
 TCE("stlex",	1800e90, e8c00fe0, 3, (RRnpc, RRnpc, RRnpcb),
							stlex,  t_stlex),
 TCE("stlexb",	1c00e90, e8c00fc0, 3, (RRnpc, RRnpc, RRnpcb),
							stlex, t_stlex),
 TCE("stlexh",	1e00e90, e8c00fd0, 3, (RRnpc, RRnpc, RRnpcb),
							stlex, t_stlex),
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_v8

 tCE("sevl",	320f005, _sevl,    0, (),		noargs,	t_hint),
 TCE("ldaexd",	1b00e9f, e8d000ff, 3, (RRnpc, oRRnpc, RRnpcb),
							ldrexd, t_ldrexd),
 TCE("stlexd",	1a00e90, e8c000f0, 4, (RRnpc, RRnpc, oRRnpc, RRnpcb),
							strexd, t_strexd),
#undef THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_v8r
#undef ARM_VARIANT
#define ARM_VARIANT & arm_ext_v8r

/* ARMv8-R instructions.  */
 TUF("dfb",	57ff04c, f3bf8f4c, 0, (), noargs, noargs),

/* Defined in V8 but is in undefined encoding space for earlier
   architectures.  However earlier architectures are required to treat
   this instuction as a semihosting trap as well.  Hence while not explicitly
   defined as such, it is in fact correct to define the instruction for all
   architectures.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v1
#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_ext_v1
 TUE("hlt",	1000070, ba80,     1, (oIffffb),	bkpt,	t_hlt),

 /* ARMv8 T32 only.  */
#undef  ARM_VARIANT
#define ARM_VARIANT  NULL
 TUF("dcps1",	0,	 f78f8001, 0, (),	noargs, noargs),
 TUF("dcps2",	0,	 f78f8002, 0, (),	noargs, noargs),
 TUF("dcps3",	0,	 f78f8003, 0, (),	noargs, noargs),

  /* FP for ARMv8.  */
#undef  ARM_VARIANT
#define ARM_VARIANT   & fpu_vfp_ext_armv8xd
#undef  THUMB_VARIANT
#define THUMB_VARIANT & fpu_vfp_ext_armv8xd

  nUF(vseleq, _vseleq, 3, (RVSD, RVSD, RVSD),		vsel),
  nUF(vselvs, _vselvs, 3, (RVSD, RVSD, RVSD),		vsel),
  nUF(vselge, _vselge, 3, (RVSD, RVSD, RVSD),		vsel),
  nUF(vselgt, _vselgt, 3, (RVSD, RVSD, RVSD),		vsel),
  nCE(vrintr, _vrintr, 2, (RNSDQ, oRNSDQ),		vrintr),
  mnCE(vrintz, _vrintr, 2, (RNSDQMQ, oRNSDQMQ),		vrintz),
  mnCE(vrintx, _vrintr, 2, (RNSDQMQ, oRNSDQMQ),		vrintx),
  mnUF(vrinta, _vrinta, 2, (RNSDQMQ, oRNSDQMQ),		vrinta),
  mnUF(vrintn, _vrinta, 2, (RNSDQMQ, oRNSDQMQ),		vrintn),
  mnUF(vrintp, _vrinta, 2, (RNSDQMQ, oRNSDQMQ),		vrintp),
  mnUF(vrintm, _vrinta, 2, (RNSDQMQ, oRNSDQMQ),		vrintm),

  /* Crypto v1 extensions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT & fpu_crypto_ext_armv8
#undef  THUMB_VARIANT
#define THUMB_VARIANT & fpu_crypto_ext_armv8

  nUF(aese, _aes, 2, (RNQ, RNQ), aese),
  nUF(aesd, _aes, 2, (RNQ, RNQ), aesd),
  nUF(aesmc, _aes, 2, (RNQ, RNQ), aesmc),
  nUF(aesimc, _aes, 2, (RNQ, RNQ), aesimc),
  nUF(sha1c, _sha3op, 3, (RNQ, RNQ, RNQ), sha1c),
  nUF(sha1p, _sha3op, 3, (RNQ, RNQ, RNQ), sha1p),
  nUF(sha1m, _sha3op, 3, (RNQ, RNQ, RNQ), sha1m),
  nUF(sha1su0, _sha3op, 3, (RNQ, RNQ, RNQ), sha1su0),
  nUF(sha256h, _sha3op, 3, (RNQ, RNQ, RNQ), sha256h),
  nUF(sha256h2, _sha3op, 3, (RNQ, RNQ, RNQ), sha256h2),
  nUF(sha256su1, _sha3op, 3, (RNQ, RNQ, RNQ), sha256su1),
  nUF(sha1h, _sha1h, 2, (RNQ, RNQ), sha1h),
  nUF(sha1su1, _sha2op, 2, (RNQ, RNQ), sha1su1),
  nUF(sha256su0, _sha2op, 2, (RNQ, RNQ), sha256su0),

#undef  ARM_VARIANT
#define ARM_VARIANT   & arm_ext_crc
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_crc
  TUEc("crc32b", 1000040, fac0f080, 3, (RR, oRR, RR), crc32b),
  TUEc("crc32h", 1200040, fac0f090, 3, (RR, oRR, RR), crc32h),
  TUEc("crc32w", 1400040, fac0f0a0, 3, (RR, oRR, RR), crc32w),
  TUEc("crc32cb",1000240, fad0f080, 3, (RR, oRR, RR), crc32cb),
  TUEc("crc32ch",1200240, fad0f090, 3, (RR, oRR, RR), crc32ch),
  TUEc("crc32cw",1400240, fad0f0a0, 3, (RR, oRR, RR), crc32cw),

 /* ARMv8.2 RAS extension.  */
#undef  ARM_VARIANT
#define ARM_VARIANT   & arm_ext_ras
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_ras
 TUE ("esb", 320f010, f3af8010, 0, (), noargs,  noargs),

#undef  ARM_VARIANT
#define ARM_VARIANT   & arm_ext_v8_3
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_v8_3
 NCE (vjcvt, eb90bc0, 2, (RVS, RVD), vjcvt),

#undef  ARM_VARIANT
#define ARM_VARIANT   & fpu_neon_ext_dotprod
#undef  THUMB_VARIANT
#define THUMB_VARIANT & fpu_neon_ext_dotprod
 NUF (vsdot, d00, 3, (RNDQ, RNDQ, RNDQ_RNSC), neon_dotproduct_s),
 NUF (vudot, d00, 3, (RNDQ, RNDQ, RNDQ_RNSC), neon_dotproduct_u),

#undef  ARM_VARIANT
#define ARM_VARIANT  & fpu_fpa_ext_v1  /* Core FPA instruction set (V1).  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT NULL

 cCE("wfs",	e200110, 1, (RR),	     rd),
 cCE("rfs",	e300110, 1, (RR),	     rd),
 cCE("wfc",	e400110, 1, (RR),	     rd),
 cCE("rfc",	e500110, 1, (RR),	     rd),

 cCL("ldfs",	c100100, 2, (RF, ADDRGLDC),  rd_cpaddr),
 cCL("ldfd",	c108100, 2, (RF, ADDRGLDC),  rd_cpaddr),
 cCL("ldfe",	c500100, 2, (RF, ADDRGLDC),  rd_cpaddr),
 cCL("ldfp",	c508100, 2, (RF, ADDRGLDC),  rd_cpaddr),

 cCL("stfs",	c000100, 2, (RF, ADDRGLDC),  rd_cpaddr),
 cCL("stfd",	c008100, 2, (RF, ADDRGLDC),  rd_cpaddr),
 cCL("stfe",	c400100, 2, (RF, ADDRGLDC),  rd_cpaddr),
 cCL("stfp",	c408100, 2, (RF, ADDRGLDC),  rd_cpaddr),

 cCL("mvfs",	e008100, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfsp",	e008120, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfsm",	e008140, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfsz",	e008160, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfd",	e008180, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfdp",	e0081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfdm",	e0081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfdz",	e0081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfe",	e088100, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfep",	e088120, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfem",	e088140, 2, (RF, RF_IF),     rd_rm),
 cCL("mvfez",	e088160, 2, (RF, RF_IF),     rd_rm),

 cCL("mnfs",	e108100, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfsp",	e108120, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfsm",	e108140, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfsz",	e108160, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfd",	e108180, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfdp",	e1081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfdm",	e1081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfdz",	e1081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfe",	e188100, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfep",	e188120, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfem",	e188140, 2, (RF, RF_IF),     rd_rm),
 cCL("mnfez",	e188160, 2, (RF, RF_IF),     rd_rm),

 cCL("abss",	e208100, 2, (RF, RF_IF),     rd_rm),
 cCL("abssp",	e208120, 2, (RF, RF_IF),     rd_rm),
 cCL("abssm",	e208140, 2, (RF, RF_IF),     rd_rm),
 cCL("abssz",	e208160, 2, (RF, RF_IF),     rd_rm),
 cCL("absd",	e208180, 2, (RF, RF_IF),     rd_rm),
 cCL("absdp",	e2081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("absdm",	e2081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("absdz",	e2081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("abse",	e288100, 2, (RF, RF_IF),     rd_rm),
 cCL("absep",	e288120, 2, (RF, RF_IF),     rd_rm),
 cCL("absem",	e288140, 2, (RF, RF_IF),     rd_rm),
 cCL("absez",	e288160, 2, (RF, RF_IF),     rd_rm),

 cCL("rnds",	e308100, 2, (RF, RF_IF),     rd_rm),
 cCL("rndsp",	e308120, 2, (RF, RF_IF),     rd_rm),
 cCL("rndsm",	e308140, 2, (RF, RF_IF),     rd_rm),
 cCL("rndsz",	e308160, 2, (RF, RF_IF),     rd_rm),
 cCL("rndd",	e308180, 2, (RF, RF_IF),     rd_rm),
 cCL("rnddp",	e3081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("rnddm",	e3081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("rnddz",	e3081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("rnde",	e388100, 2, (RF, RF_IF),     rd_rm),
 cCL("rndep",	e388120, 2, (RF, RF_IF),     rd_rm),
 cCL("rndem",	e388140, 2, (RF, RF_IF),     rd_rm),
 cCL("rndez",	e388160, 2, (RF, RF_IF),     rd_rm),

 cCL("sqts",	e408100, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtsp",	e408120, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtsm",	e408140, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtsz",	e408160, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtd",	e408180, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtdp",	e4081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtdm",	e4081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtdz",	e4081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("sqte",	e488100, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtep",	e488120, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtem",	e488140, 2, (RF, RF_IF),     rd_rm),
 cCL("sqtez",	e488160, 2, (RF, RF_IF),     rd_rm),

 cCL("logs",	e508100, 2, (RF, RF_IF),     rd_rm),
 cCL("logsp",	e508120, 2, (RF, RF_IF),     rd_rm),
 cCL("logsm",	e508140, 2, (RF, RF_IF),     rd_rm),
 cCL("logsz",	e508160, 2, (RF, RF_IF),     rd_rm),
 cCL("logd",	e508180, 2, (RF, RF_IF),     rd_rm),
 cCL("logdp",	e5081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("logdm",	e5081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("logdz",	e5081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("loge",	e588100, 2, (RF, RF_IF),     rd_rm),
 cCL("logep",	e588120, 2, (RF, RF_IF),     rd_rm),
 cCL("logem",	e588140, 2, (RF, RF_IF),     rd_rm),
 cCL("logez",	e588160, 2, (RF, RF_IF),     rd_rm),

 cCL("lgns",	e608100, 2, (RF, RF_IF),     rd_rm),
 cCL("lgnsp",	e608120, 2, (RF, RF_IF),     rd_rm),
 cCL("lgnsm",	e608140, 2, (RF, RF_IF),     rd_rm),
 cCL("lgnsz",	e608160, 2, (RF, RF_IF),     rd_rm),
 cCL("lgnd",	e608180, 2, (RF, RF_IF),     rd_rm),
 cCL("lgndp",	e6081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("lgndm",	e6081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("lgndz",	e6081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("lgne",	e688100, 2, (RF, RF_IF),     rd_rm),
 cCL("lgnep",	e688120, 2, (RF, RF_IF),     rd_rm),
 cCL("lgnem",	e688140, 2, (RF, RF_IF),     rd_rm),
 cCL("lgnez",	e688160, 2, (RF, RF_IF),     rd_rm),

 cCL("exps",	e708100, 2, (RF, RF_IF),     rd_rm),
 cCL("expsp",	e708120, 2, (RF, RF_IF),     rd_rm),
 cCL("expsm",	e708140, 2, (RF, RF_IF),     rd_rm),
 cCL("expsz",	e708160, 2, (RF, RF_IF),     rd_rm),
 cCL("expd",	e708180, 2, (RF, RF_IF),     rd_rm),
 cCL("expdp",	e7081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("expdm",	e7081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("expdz",	e7081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("expe",	e788100, 2, (RF, RF_IF),     rd_rm),
 cCL("expep",	e788120, 2, (RF, RF_IF),     rd_rm),
 cCL("expem",	e788140, 2, (RF, RF_IF),     rd_rm),
 cCL("expdz",	e788160, 2, (RF, RF_IF),     rd_rm),

 cCL("sins",	e808100, 2, (RF, RF_IF),     rd_rm),
 cCL("sinsp",	e808120, 2, (RF, RF_IF),     rd_rm),
 cCL("sinsm",	e808140, 2, (RF, RF_IF),     rd_rm),
 cCL("sinsz",	e808160, 2, (RF, RF_IF),     rd_rm),
 cCL("sind",	e808180, 2, (RF, RF_IF),     rd_rm),
 cCL("sindp",	e8081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("sindm",	e8081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("sindz",	e8081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("sine",	e888100, 2, (RF, RF_IF),     rd_rm),
 cCL("sinep",	e888120, 2, (RF, RF_IF),     rd_rm),
 cCL("sinem",	e888140, 2, (RF, RF_IF),     rd_rm),
 cCL("sinez",	e888160, 2, (RF, RF_IF),     rd_rm),

 cCL("coss",	e908100, 2, (RF, RF_IF),     rd_rm),
 cCL("cossp",	e908120, 2, (RF, RF_IF),     rd_rm),
 cCL("cossm",	e908140, 2, (RF, RF_IF),     rd_rm),
 cCL("cossz",	e908160, 2, (RF, RF_IF),     rd_rm),
 cCL("cosd",	e908180, 2, (RF, RF_IF),     rd_rm),
 cCL("cosdp",	e9081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("cosdm",	e9081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("cosdz",	e9081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("cose",	e988100, 2, (RF, RF_IF),     rd_rm),
 cCL("cosep",	e988120, 2, (RF, RF_IF),     rd_rm),
 cCL("cosem",	e988140, 2, (RF, RF_IF),     rd_rm),
 cCL("cosez",	e988160, 2, (RF, RF_IF),     rd_rm),

 cCL("tans",	ea08100, 2, (RF, RF_IF),     rd_rm),
 cCL("tansp",	ea08120, 2, (RF, RF_IF),     rd_rm),
 cCL("tansm",	ea08140, 2, (RF, RF_IF),     rd_rm),
 cCL("tansz",	ea08160, 2, (RF, RF_IF),     rd_rm),
 cCL("tand",	ea08180, 2, (RF, RF_IF),     rd_rm),
 cCL("tandp",	ea081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("tandm",	ea081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("tandz",	ea081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("tane",	ea88100, 2, (RF, RF_IF),     rd_rm),
 cCL("tanep",	ea88120, 2, (RF, RF_IF),     rd_rm),
 cCL("tanem",	ea88140, 2, (RF, RF_IF),     rd_rm),
 cCL("tanez",	ea88160, 2, (RF, RF_IF),     rd_rm),

 cCL("asns",	eb08100, 2, (RF, RF_IF),     rd_rm),
 cCL("asnsp",	eb08120, 2, (RF, RF_IF),     rd_rm),
 cCL("asnsm",	eb08140, 2, (RF, RF_IF),     rd_rm),
 cCL("asnsz",	eb08160, 2, (RF, RF_IF),     rd_rm),
 cCL("asnd",	eb08180, 2, (RF, RF_IF),     rd_rm),
 cCL("asndp",	eb081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("asndm",	eb081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("asndz",	eb081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("asne",	eb88100, 2, (RF, RF_IF),     rd_rm),
 cCL("asnep",	eb88120, 2, (RF, RF_IF),     rd_rm),
 cCL("asnem",	eb88140, 2, (RF, RF_IF),     rd_rm),
 cCL("asnez",	eb88160, 2, (RF, RF_IF),     rd_rm),

 cCL("acss",	ec08100, 2, (RF, RF_IF),     rd_rm),
 cCL("acssp",	ec08120, 2, (RF, RF_IF),     rd_rm),
 cCL("acssm",	ec08140, 2, (RF, RF_IF),     rd_rm),
 cCL("acssz",	ec08160, 2, (RF, RF_IF),     rd_rm),
 cCL("acsd",	ec08180, 2, (RF, RF_IF),     rd_rm),
 cCL("acsdp",	ec081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("acsdm",	ec081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("acsdz",	ec081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("acse",	ec88100, 2, (RF, RF_IF),     rd_rm),
 cCL("acsep",	ec88120, 2, (RF, RF_IF),     rd_rm),
 cCL("acsem",	ec88140, 2, (RF, RF_IF),     rd_rm),
 cCL("acsez",	ec88160, 2, (RF, RF_IF),     rd_rm),

 cCL("atns",	ed08100, 2, (RF, RF_IF),     rd_rm),
 cCL("atnsp",	ed08120, 2, (RF, RF_IF),     rd_rm),
 cCL("atnsm",	ed08140, 2, (RF, RF_IF),     rd_rm),
 cCL("atnsz",	ed08160, 2, (RF, RF_IF),     rd_rm),
 cCL("atnd",	ed08180, 2, (RF, RF_IF),     rd_rm),
 cCL("atndp",	ed081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("atndm",	ed081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("atndz",	ed081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("atne",	ed88100, 2, (RF, RF_IF),     rd_rm),
 cCL("atnep",	ed88120, 2, (RF, RF_IF),     rd_rm),
 cCL("atnem",	ed88140, 2, (RF, RF_IF),     rd_rm),
 cCL("atnez",	ed88160, 2, (RF, RF_IF),     rd_rm),

 cCL("urds",	ee08100, 2, (RF, RF_IF),     rd_rm),
 cCL("urdsp",	ee08120, 2, (RF, RF_IF),     rd_rm),
 cCL("urdsm",	ee08140, 2, (RF, RF_IF),     rd_rm),
 cCL("urdsz",	ee08160, 2, (RF, RF_IF),     rd_rm),
 cCL("urdd",	ee08180, 2, (RF, RF_IF),     rd_rm),
 cCL("urddp",	ee081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("urddm",	ee081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("urddz",	ee081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("urde",	ee88100, 2, (RF, RF_IF),     rd_rm),
 cCL("urdep",	ee88120, 2, (RF, RF_IF),     rd_rm),
 cCL("urdem",	ee88140, 2, (RF, RF_IF),     rd_rm),
 cCL("urdez",	ee88160, 2, (RF, RF_IF),     rd_rm),

 cCL("nrms",	ef08100, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmsp",	ef08120, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmsm",	ef08140, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmsz",	ef08160, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmd",	ef08180, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmdp",	ef081a0, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmdm",	ef081c0, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmdz",	ef081e0, 2, (RF, RF_IF),     rd_rm),
 cCL("nrme",	ef88100, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmep",	ef88120, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmem",	ef88140, 2, (RF, RF_IF),     rd_rm),
 cCL("nrmez",	ef88160, 2, (RF, RF_IF),     rd_rm),

 cCL("adfs",	e000100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfsp",	e000120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfsm",	e000140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfsz",	e000160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfd",	e000180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfdp",	e0001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfdm",	e0001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfdz",	e0001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfe",	e080100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfep",	e080120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfem",	e080140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("adfez",	e080160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("sufs",	e200100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufsp",	e200120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufsm",	e200140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufsz",	e200160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufd",	e200180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufdp",	e2001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufdm",	e2001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufdz",	e2001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufe",	e280100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufep",	e280120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufem",	e280140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("sufez",	e280160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("rsfs",	e300100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfsp",	e300120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfsm",	e300140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfsz",	e300160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfd",	e300180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfdp",	e3001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfdm",	e3001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfdz",	e3001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfe",	e380100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfep",	e380120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfem",	e380140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rsfez",	e380160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("mufs",	e100100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufsp",	e100120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufsm",	e100140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufsz",	e100160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufd",	e100180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufdp",	e1001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufdm",	e1001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufdz",	e1001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufe",	e180100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufep",	e180120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufem",	e180140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("mufez",	e180160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("dvfs",	e400100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfsp",	e400120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfsm",	e400140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfsz",	e400160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfd",	e400180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfdp",	e4001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfdm",	e4001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfdz",	e4001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfe",	e480100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfep",	e480120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfem",	e480140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("dvfez",	e480160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("rdfs",	e500100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfsp",	e500120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfsm",	e500140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfsz",	e500160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfd",	e500180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfdp",	e5001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfdm",	e5001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfdz",	e5001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfe",	e580100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfep",	e580120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfem",	e580140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rdfez",	e580160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("pows",	e600100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powsp",	e600120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powsm",	e600140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powsz",	e600160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powd",	e600180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powdp",	e6001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powdm",	e6001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powdz",	e6001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powe",	e680100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powep",	e680120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powem",	e680140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("powez",	e680160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("rpws",	e700100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwsp",	e700120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwsm",	e700140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwsz",	e700160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwd",	e700180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwdp",	e7001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwdm",	e7001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwdz",	e7001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwe",	e780100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwep",	e780120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwem",	e780140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rpwez",	e780160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("rmfs",	e800100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfsp",	e800120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfsm",	e800140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfsz",	e800160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfd",	e800180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfdp",	e8001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfdm",	e8001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfdz",	e8001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfe",	e880100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfep",	e880120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfem",	e880140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("rmfez",	e880160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("fmls",	e900100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmlsp",	e900120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmlsm",	e900140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmlsz",	e900160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmld",	e900180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmldp",	e9001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmldm",	e9001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmldz",	e9001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmle",	e980100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmlep",	e980120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmlem",	e980140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fmlez",	e980160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("fdvs",	ea00100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvsp",	ea00120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvsm",	ea00140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvsz",	ea00160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvd",	ea00180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvdp",	ea001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvdm",	ea001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvdz",	ea001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdve",	ea80100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvep",	ea80120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvem",	ea80140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("fdvez",	ea80160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("frds",	eb00100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frdsp",	eb00120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frdsm",	eb00140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frdsz",	eb00160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frdd",	eb00180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frddp",	eb001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frddm",	eb001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frddz",	eb001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frde",	eb80100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frdep",	eb80120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frdem",	eb80140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("frdez",	eb80160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCL("pols",	ec00100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("polsp",	ec00120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("polsm",	ec00140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("polsz",	ec00160, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("pold",	ec00180, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("poldp",	ec001a0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("poldm",	ec001c0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("poldz",	ec001e0, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("pole",	ec80100, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("polep",	ec80120, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("polem",	ec80140, 3, (RF, RF, RF_IF), rd_rn_rm),
 cCL("polez",	ec80160, 3, (RF, RF, RF_IF), rd_rn_rm),

 cCE("cmf",	e90f110, 2, (RF, RF_IF),     fpa_cmp),
 C3E("cmfe",	ed0f110, 2, (RF, RF_IF),     fpa_cmp),
 cCE("cnf",	eb0f110, 2, (RF, RF_IF),     fpa_cmp),
 C3E("cnfe",	ef0f110, 2, (RF, RF_IF),     fpa_cmp),

 cCL("flts",	e000110, 2, (RF, RR),	     rn_rd),
 cCL("fltsp",	e000130, 2, (RF, RR),	     rn_rd),
 cCL("fltsm",	e000150, 2, (RF, RR),	     rn_rd),
 cCL("fltsz",	e000170, 2, (RF, RR),	     rn_rd),
 cCL("fltd",	e000190, 2, (RF, RR),	     rn_rd),
 cCL("fltdp",	e0001b0, 2, (RF, RR),	     rn_rd),
 cCL("fltdm",	e0001d0, 2, (RF, RR),	     rn_rd),
 cCL("fltdz",	e0001f0, 2, (RF, RR),	     rn_rd),
 cCL("flte",	e080110, 2, (RF, RR),	     rn_rd),
 cCL("fltep",	e080130, 2, (RF, RR),	     rn_rd),
 cCL("fltem",	e080150, 2, (RF, RR),	     rn_rd),
 cCL("fltez",	e080170, 2, (RF, RR),	     rn_rd),

  /* The implementation of the FIX instruction is broken on some
     assemblers, in that it accepts a precision specifier as well as a
     rounding specifier, despite the fact that this is meaningless.
     To be more compatible, we accept it as well, though of course it
     does not set any bits.  */
 cCE("fix",	e100110, 2, (RR, RF),	     rd_rm),
 cCL("fixp",	e100130, 2, (RR, RF),	     rd_rm),
 cCL("fixm",	e100150, 2, (RR, RF),	     rd_rm),
 cCL("fixz",	e100170, 2, (RR, RF),	     rd_rm),
 cCL("fixsp",	e100130, 2, (RR, RF),	     rd_rm),
 cCL("fixsm",	e100150, 2, (RR, RF),	     rd_rm),
 cCL("fixsz",	e100170, 2, (RR, RF),	     rd_rm),
 cCL("fixdp",	e100130, 2, (RR, RF),	     rd_rm),
 cCL("fixdm",	e100150, 2, (RR, RF),	     rd_rm),
 cCL("fixdz",	e100170, 2, (RR, RF),	     rd_rm),
 cCL("fixep",	e100130, 2, (RR, RF),	     rd_rm),
 cCL("fixem",	e100150, 2, (RR, RF),	     rd_rm),
 cCL("fixez",	e100170, 2, (RR, RF),	     rd_rm),

  /* Instructions that were new with the real FPA, call them V2.  */
#undef  ARM_VARIANT
#define ARM_VARIANT  & fpu_fpa_ext_v2

 cCE("lfm",	c100200, 3, (RF, I4b, ADDR), fpa_ldmstm),
 cCL("lfmfd",	c900200, 3, (RF, I4b, ADDR), fpa_ldmstm),
 cCL("lfmea",	d100200, 3, (RF, I4b, ADDR), fpa_ldmstm),
 cCE("sfm",	c000200, 3, (RF, I4b, ADDR), fpa_ldmstm),
 cCL("sfmfd",	d000200, 3, (RF, I4b, ADDR), fpa_ldmstm),
 cCL("sfmea",	c800200, 3, (RF, I4b, ADDR), fpa_ldmstm),

#undef  ARM_VARIANT
#define ARM_VARIANT  & fpu_vfp_ext_v1xd  /* VFP V1xD (single precision).  */
#undef THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2
 mcCE(vmrs,	ef00a10, 2, (APSR_RR, RVC),   vmrs),
 mcCE(vmsr,	ee00a10, 2, (RVC, RR),        vmsr),
 mcCE(fldd,	d100b00, 2, (RVD, ADDRGLDC),  vfp_dp_ldst),
 mcCE(fstd,	d000b00, 2, (RVD, ADDRGLDC),  vfp_dp_ldst),
 mcCE(flds,	d100a00, 2, (RVS, ADDRGLDC),  vfp_sp_ldst),
 mcCE(fsts,	d000a00, 2, (RVS, ADDRGLDC),  vfp_sp_ldst),

  /* Memory operations.	 */
 mcCE(fldmias,	c900a00, 2, (RRnpctw, VRSLST),    vfp_sp_ldstmia),
 mcCE(fldmdbs,	d300a00, 2, (RRnpctw, VRSLST),    vfp_sp_ldstmdb),
 mcCE(fstmias,	c800a00, 2, (RRnpctw, VRSLST),    vfp_sp_ldstmia),
 mcCE(fstmdbs,	d200a00, 2, (RRnpctw, VRSLST),    vfp_sp_ldstmdb),
#undef THUMB_VARIANT

  /* Moves and type conversions.  */
 cCE("fmstat",	ef1fa10, 0, (),		      noargs),
 cCE("fsitos",	eb80ac0, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("fuitos",	eb80a40, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("ftosis",	ebd0a40, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("ftosizs",	ebd0ac0, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("ftouis",	ebc0a40, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("ftouizs",	ebc0ac0, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("fmrx",	ef00a10, 2, (RR, RVC),	      rd_rn),
 cCE("fmxr",	ee00a10, 2, (RVC, RR),	      rn_rd),

  /* Memory operations.	 */
 cCE("fldmfds",	c900a00, 2, (RRnpctw, VRSLST),    vfp_sp_ldstmia),
 cCE("fldmeas",	d300a00, 2, (RRnpctw, VRSLST),    vfp_sp_ldstmdb),
 cCE("fldmiax",	c900b00, 2, (RRnpctw, VRDLST),    vfp_xp_ldstmia),
 cCE("fldmfdx",	c900b00, 2, (RRnpctw, VRDLST),    vfp_xp_ldstmia),
 cCE("fldmdbx",	d300b00, 2, (RRnpctw, VRDLST),    vfp_xp_ldstmdb),
 cCE("fldmeax",	d300b00, 2, (RRnpctw, VRDLST),    vfp_xp_ldstmdb),
 cCE("fstmeas",	c800a00, 2, (RRnpctw, VRSLST),    vfp_sp_ldstmia),
 cCE("fstmfds",	d200a00, 2, (RRnpctw, VRSLST),    vfp_sp_ldstmdb),
 cCE("fstmiax",	c800b00, 2, (RRnpctw, VRDLST),    vfp_xp_ldstmia),
 cCE("fstmeax",	c800b00, 2, (RRnpctw, VRDLST),    vfp_xp_ldstmia),
 cCE("fstmdbx",	d200b00, 2, (RRnpctw, VRDLST),    vfp_xp_ldstmdb),
 cCE("fstmfdx",	d200b00, 2, (RRnpctw, VRDLST),    vfp_xp_ldstmdb),

  /* Monadic operations.  */
 cCE("fabss",	eb00ac0, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("fnegs",	eb10a40, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("fsqrts",	eb10ac0, 2, (RVS, RVS),	      vfp_sp_monadic),

  /* Dyadic operations.	 */
 cCE("fadds",	e300a00, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("fsubs",	e300a40, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("fmuls",	e200a00, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("fdivs",	e800a00, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("fmacs",	e000a00, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("fmscs",	e100a00, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("fnmuls",	e200a40, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("fnmacs",	e000a40, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("fnmscs",	e100a40, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),

  /* Comparisons.  */
 cCE("fcmps",	eb40a40, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("fcmpzs",	eb50a40, 1, (RVS),	      vfp_sp_compare_z),
 cCE("fcmpes",	eb40ac0, 2, (RVS, RVS),	      vfp_sp_monadic),
 cCE("fcmpezs",	eb50ac0, 1, (RVS),	      vfp_sp_compare_z),

 /* Double precision load/store are still present on single precision
    implementations.  */
 cCE("fldmiad",	c900b00, 2, (RRnpctw, VRDLST),    vfp_dp_ldstmia),
 cCE("fldmfdd",	c900b00, 2, (RRnpctw, VRDLST),    vfp_dp_ldstmia),
 cCE("fldmdbd",	d300b00, 2, (RRnpctw, VRDLST),    vfp_dp_ldstmdb),
 cCE("fldmead",	d300b00, 2, (RRnpctw, VRDLST),    vfp_dp_ldstmdb),
 cCE("fstmiad",	c800b00, 2, (RRnpctw, VRDLST),    vfp_dp_ldstmia),
 cCE("fstmead",	c800b00, 2, (RRnpctw, VRDLST),    vfp_dp_ldstmia),
 cCE("fstmdbd",	d200b00, 2, (RRnpctw, VRDLST),    vfp_dp_ldstmdb),
 cCE("fstmfdd",	d200b00, 2, (RRnpctw, VRDLST),    vfp_dp_ldstmdb),

#undef  ARM_VARIANT
#define ARM_VARIANT  & fpu_vfp_ext_v1 /* VFP V1 (Double precision).  */

  /* Moves and type conversions.  */
 cCE("fcvtds",	eb70ac0, 2, (RVD, RVS),	      vfp_dp_sp_cvt),
 cCE("fcvtsd",	eb70bc0, 2, (RVS, RVD),	      vfp_sp_dp_cvt),
 cCE("fmdhr",	e200b10, 2, (RVD, RR),	      vfp_dp_rn_rd),
 cCE("fmdlr",	e000b10, 2, (RVD, RR),	      vfp_dp_rn_rd),
 cCE("fmrdh",	e300b10, 2, (RR, RVD),	      vfp_dp_rd_rn),
 cCE("fmrdl",	e100b10, 2, (RR, RVD),	      vfp_dp_rd_rn),
 cCE("fsitod",	eb80bc0, 2, (RVD, RVS),	      vfp_dp_sp_cvt),
 cCE("fuitod",	eb80b40, 2, (RVD, RVS),	      vfp_dp_sp_cvt),
 cCE("ftosid",	ebd0b40, 2, (RVS, RVD),	      vfp_sp_dp_cvt),
 cCE("ftosizd",	ebd0bc0, 2, (RVS, RVD),	      vfp_sp_dp_cvt),
 cCE("ftouid",	ebc0b40, 2, (RVS, RVD),	      vfp_sp_dp_cvt),
 cCE("ftouizd",	ebc0bc0, 2, (RVS, RVD),	      vfp_sp_dp_cvt),

  /* Monadic operations.  */
 cCE("fabsd",	eb00bc0, 2, (RVD, RVD),	      vfp_dp_rd_rm),
 cCE("fnegd",	eb10b40, 2, (RVD, RVD),	      vfp_dp_rd_rm),
 cCE("fsqrtd",	eb10bc0, 2, (RVD, RVD),	      vfp_dp_rd_rm),

  /* Dyadic operations.	 */
 cCE("faddd",	e300b00, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("fsubd",	e300b40, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("fmuld",	e200b00, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("fdivd",	e800b00, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("fmacd",	e000b00, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("fmscd",	e100b00, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("fnmuld",	e200b40, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("fnmacd",	e000b40, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("fnmscd",	e100b40, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),

  /* Comparisons.  */
 cCE("fcmpd",	eb40b40, 2, (RVD, RVD),	      vfp_dp_rd_rm),
 cCE("fcmpzd",	eb50b40, 1, (RVD),	      vfp_dp_rd),
 cCE("fcmped",	eb40bc0, 2, (RVD, RVD),	      vfp_dp_rd_rm),
 cCE("fcmpezd",	eb50bc0, 1, (RVD),	      vfp_dp_rd),

/* Instructions which may belong to either the Neon or VFP instruction sets.
   Individual encoder functions perform additional architecture checks.  */
#undef  ARM_VARIANT
#define ARM_VARIANT    & fpu_vfp_ext_v1xd
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 NCE(vldm,      c900b00, 2, (RRnpctw, VRSDLST), neon_ldm_stm),
 NCE(vldmia,    c900b00, 2, (RRnpctw, VRSDLST), neon_ldm_stm),
 NCE(vldmdb,    d100b00, 2, (RRnpctw, VRSDLST), neon_ldm_stm),
 NCE(vstm,      c800b00, 2, (RRnpctw, VRSDLST), neon_ldm_stm),
 NCE(vstmia,    c800b00, 2, (RRnpctw, VRSDLST), neon_ldm_stm),
 NCE(vstmdb,    d000b00, 2, (RRnpctw, VRSDLST), neon_ldm_stm),

 NCE(vpop,      0,       1, (VRSDLST),          vfp_nsyn_pop),
 NCE(vpush,     0,       1, (VRSDLST),          vfp_nsyn_push),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & fpu_vfp_ext_v1xd

  /* These mnemonics are unique to VFP.  */
 NCE(vsqrt,     0,       2, (RVSD, RVSD),       vfp_nsyn_sqrt),
 NCE(vdiv,      0,       3, (RVSD, RVSD, RVSD), vfp_nsyn_div),
 nCE(vnmul,     _vnmul,   3, (RVSD, RVSD, RVSD), vfp_nsyn_nmul),
 nCE(vnmla,     _vnmla,   3, (RVSD, RVSD, RVSD), vfp_nsyn_nmul),
 nCE(vnmls,     _vnmls,   3, (RVSD, RVSD, RVSD), vfp_nsyn_nmul),
 NCE(vcvtz,     0,       2, (RVSD, RVSD),       vfp_nsyn_cvtz),

  /* Mnemonics shared by Neon and VFP.  */
 nCEF(vmls,     _vmls,    3, (RNSDQ, oRNSDQ, RNSDQ_RNSC), neon_mac_maybe_scalar),

 mnCEF(vcvt,     _vcvt,   3, (RNSDQMQ, RNSDQMQ, oI32z), neon_cvt),
 nCEF(vcvtr,    _vcvt,   2, (RNSDQ, RNSDQ), neon_cvtr),
 MNCEF(vcvtb,	eb20a40, 3, (RVSDMQ, RVSDMQ, oI32b), neon_cvtb),
 MNCEF(vcvtt,	eb20a40, 3, (RVSDMQ, RVSDMQ, oI32b), neon_cvtt),


  /* NOTE: All VMOV encoding is special-cased!  */
 NCE(vmovq,     0,       1, (VMOV), neon_mov),

#undef  THUMB_VARIANT
/* Could be either VLDR/VSTR or VLDR/VSTR (system register) which are guarded
   by different feature bits.  Since we are setting the Thumb guard, we can
   require Thumb-1 which makes it a nop guard and set the right feature bit in
   do_vldr_vstr ().  */
#define THUMB_VARIANT  & arm_ext_v4t
 NCE(vldr,      d100b00, 2, (VLDR, ADDRGLDC), vldr_vstr),
 NCE(vstr,      d000b00, 2, (VLDR, ADDRGLDC), vldr_vstr),

#undef  ARM_VARIANT
#define ARM_VARIANT    & arm_ext_fp16
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_fp16
 /* New instructions added from v8.2, allowing the extraction and insertion of
    the upper 16 bits of a 32-bit vector register.  */
 NCE (vmovx,     eb00a40,       2, (RVS, RVS), neon_movhf),
 NCE (vins,      eb00ac0,       2, (RVS, RVS), neon_movhf),

 /* New backported fma/fms instructions optional in v8.2.  */
 NUF (vfmsl, 810, 3, (RNDQ, RNSD, RNSD_RNSC), neon_vfmsl),
 NUF (vfmal, 810, 3, (RNDQ, RNSD, RNSD_RNSC), neon_vfmal),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & fpu_neon_ext_v1
#undef  ARM_VARIANT
#define ARM_VARIANT    & fpu_neon_ext_v1

  /* Data processing with three registers of the same length.  */
  /* integer ops, valid types S8 S16 S32 U8 U16 U32.  */
 NUF(vaba,      0000710, 3, (RNDQ, RNDQ,  RNDQ), neon_dyadic_i_su),
 NUF(vabaq,     0000710, 3, (RNQ,  RNQ,   RNQ),  neon_dyadic_i_su),
 NUF(vhaddq,    0000000, 3, (RNQ,  oRNQ,  RNQ),  neon_dyadic_i_su),
 NUF(vrhaddq,   0000100, 3, (RNQ,  oRNQ,  RNQ),  neon_dyadic_i_su),
 NUF(vhsubq,    0000200, 3, (RNQ,  oRNQ,  RNQ),  neon_dyadic_i_su),
  /* integer ops, valid types S8 S16 S32 S64 U8 U16 U32 U64.  */
 NUF(vqaddq,    0000010, 3, (RNQ,  oRNQ,  RNQ),  neon_dyadic_i64_su),
 NUF(vqsubq,    0000210, 3, (RNQ,  oRNQ,  RNQ),  neon_dyadic_i64_su),
 NUF(vrshlq,    0000500, 3, (RNQ,  oRNQ,  RNQ),  neon_rshl),
 NUF(vqrshlq,   0000510, 3, (RNQ,  oRNQ,  RNQ),  neon_rshl),
  /* If not immediate, fall back to neon_dyadic_i64_su.
     shl should accept I8 I16 I32 I64,
     qshl should accept S8 S16 S32 S64 U8 U16 U32 U64.  */
 nUF(vshlq,     _vshl,    3, (RNQ,  oRNQ,  RNDQ_I63b), neon_shl),
 nUF(vqshlq,    _vqshl,   3, (RNQ,  oRNQ,  RNDQ_I63b), neon_qshl),
  /* Logic ops, types optional & ignored.  */
 nUF(vandq,     _vand,    3, (RNQ,  oRNQ,  RNDQ_Ibig), neon_logic),
 nUF(vbicq,     _vbic,    3, (RNQ,  oRNQ,  RNDQ_Ibig), neon_logic),
 nUF(vorrq,     _vorr,    3, (RNQ,  oRNQ,  RNDQ_Ibig), neon_logic),
 nUF(vornq,     _vorn,    3, (RNQ,  oRNQ,  RNDQ_Ibig), neon_logic),
 nUF(veorq,     _veor,    3, (RNQ,  oRNQ,  RNQ),       neon_logic),
  /* Bitfield ops, untyped.  */
 NUF(vbsl,      1100110, 3, (RNDQ, RNDQ, RNDQ), neon_bitfield),
 NUF(vbslq,     1100110, 3, (RNQ,  RNQ,  RNQ),  neon_bitfield),
 NUF(vbit,      1200110, 3, (RNDQ, RNDQ, RNDQ), neon_bitfield),
 NUF(vbitq,     1200110, 3, (RNQ,  RNQ,  RNQ),  neon_bitfield),
 NUF(vbif,      1300110, 3, (RNDQ, RNDQ, RNDQ), neon_bitfield),
 NUF(vbifq,     1300110, 3, (RNQ,  RNQ,  RNQ),  neon_bitfield),
  /* Int and float variants, types S8 S16 S32 U8 U16 U32 F16 F32.  */
 nUF(vabdq,     _vabd,    3, (RNQ,  oRNQ,  RNQ),  neon_dyadic_if_su),
 nUF(vmaxq,     _vmax,    3, (RNQ,  oRNQ,  RNQ),  neon_dyadic_if_su),
 nUF(vminq,     _vmin,    3, (RNQ,  oRNQ,  RNQ),  neon_dyadic_if_su),
  /* Comparisons. Types S8 S16 S32 U8 U16 U32 F32. Non-immediate versions fall
     back to neon_dyadic_if_su.  */
 nUF(vcge,      _vcge,    3, (RNDQ, oRNDQ, RNDQ_I0), neon_cmp),
 nUF(vcgeq,     _vcge,    3, (RNQ,  oRNQ,  RNDQ_I0), neon_cmp),
 nUF(vcgt,      _vcgt,    3, (RNDQ, oRNDQ, RNDQ_I0), neon_cmp),
 nUF(vcgtq,     _vcgt,    3, (RNQ,  oRNQ,  RNDQ_I0), neon_cmp),
 nUF(vclt,      _vclt,    3, (RNDQ, oRNDQ, RNDQ_I0), neon_cmp_inv),
 nUF(vcltq,     _vclt,    3, (RNQ,  oRNQ,  RNDQ_I0), neon_cmp_inv),
 nUF(vcle,      _vcle,    3, (RNDQ, oRNDQ, RNDQ_I0), neon_cmp_inv),
 nUF(vcleq,     _vcle,    3, (RNQ,  oRNQ,  RNDQ_I0), neon_cmp_inv),
  /* Comparison. Type I8 I16 I32 F32.  */
 nUF(vceq,      _vceq,    3, (RNDQ, oRNDQ, RNDQ_I0), neon_ceq),
 nUF(vceqq,     _vceq,    3, (RNQ,  oRNQ,  RNDQ_I0), neon_ceq),
  /* As above, D registers only.  */
 nUF(vpmax,     _vpmax,   3, (RND, oRND, RND), neon_dyadic_if_su_d),
 nUF(vpmin,     _vpmin,   3, (RND, oRND, RND), neon_dyadic_if_su_d),
  /* Int and float variants, signedness unimportant.  */
 nUF(vmlaq,     _vmla,    3, (RNQ,  oRNQ,  RNDQ_RNSC), neon_mac_maybe_scalar),
 nUF(vmlsq,     _vmls,    3, (RNQ,  oRNQ,  RNDQ_RNSC), neon_mac_maybe_scalar),
 nUF(vpadd,     _vpadd,   3, (RND,  oRND,  RND),       neon_dyadic_if_i_d),
  /* Add/sub take types I8 I16 I32 I64 F32.  */
 nUF(vaddq,     _vadd,    3, (RNQ,  oRNQ,  RNQ),  neon_addsub_if_i),
 nUF(vsubq,     _vsub,    3, (RNQ,  oRNQ,  RNQ),  neon_addsub_if_i),
  /* vtst takes sizes 8, 16, 32.  */
 NUF(vtst,      0000810, 3, (RNDQ, oRNDQ, RNDQ), neon_tst),
 NUF(vtstq,     0000810, 3, (RNQ,  oRNQ,  RNQ),  neon_tst),
  /* VMUL takes I8 I16 I32 F32 P8.  */
 nUF(vmulq,     _vmul,     3, (RNQ,  oRNQ,  RNDQ_RNSC), neon_mul),
  /* VQD{R}MULH takes S16 S32.  */
 nUF(vqdmulhq,  _vqdmulh,  3, (RNQ,  oRNQ,  RNDQ_RNSC), neon_qdmulh),
 nUF(vqrdmulhq, _vqrdmulh, 3, (RNQ,  oRNQ,  RNDQ_RNSC), neon_qdmulh),
 NUF(vacge,     0000e10,  3, (RNDQ, oRNDQ, RNDQ), neon_fcmp_absolute),
 NUF(vacgeq,    0000e10,  3, (RNQ,  oRNQ,  RNQ),  neon_fcmp_absolute),
 NUF(vacgt,     0200e10,  3, (RNDQ, oRNDQ, RNDQ), neon_fcmp_absolute),
 NUF(vacgtq,    0200e10,  3, (RNQ,  oRNQ,  RNQ),  neon_fcmp_absolute),
 NUF(vaclt,     0200e10,  3, (RNDQ, oRNDQ, RNDQ), neon_fcmp_absolute_inv),
 NUF(vacltq,    0200e10,  3, (RNQ,  oRNQ,  RNQ),  neon_fcmp_absolute_inv),
 NUF(vacle,     0000e10,  3, (RNDQ, oRNDQ, RNDQ), neon_fcmp_absolute_inv),
 NUF(vacleq,    0000e10,  3, (RNQ,  oRNQ,  RNQ),  neon_fcmp_absolute_inv),
 NUF(vrecps,    0000f10,  3, (RNDQ, oRNDQ, RNDQ), neon_step),
 NUF(vrecpsq,   0000f10,  3, (RNQ,  oRNQ,  RNQ),  neon_step),
 NUF(vrsqrts,   0200f10,  3, (RNDQ, oRNDQ, RNDQ), neon_step),
 NUF(vrsqrtsq,  0200f10,  3, (RNQ,  oRNQ,  RNQ),  neon_step),
 /* ARM v8.1 extension.  */
 nUF (vqrdmlahq, _vqrdmlah, 3, (RNQ,  oRNQ,  RNDQ_RNSC), neon_qrdmlah),
 nUF (vqrdmlsh,  _vqrdmlsh, 3, (RNDQ, oRNDQ, RNDQ_RNSC), neon_qrdmlah),
 nUF (vqrdmlshq, _vqrdmlsh, 3, (RNQ,  oRNQ,  RNDQ_RNSC), neon_qrdmlah),

  /* Two address, int/float. Types S8 S16 S32 F32.  */
 NUF(vabsq,     1b10300, 2, (RNQ,  RNQ),      neon_abs_neg),
 NUF(vnegq,     1b10380, 2, (RNQ,  RNQ),      neon_abs_neg),

  /* Data processing with two registers and a shift amount.  */
  /* Right shifts, and variants with rounding.
     Types accepted S8 S16 S32 S64 U8 U16 U32 U64.  */
 NUF(vshrq,     0800010, 3, (RNQ,  oRNQ,  I64z), neon_rshift_round_imm),
 NUF(vrshrq,    0800210, 3, (RNQ,  oRNQ,  I64z), neon_rshift_round_imm),
 NUF(vsra,      0800110, 3, (RNDQ, oRNDQ, I64),  neon_rshift_round_imm),
 NUF(vsraq,     0800110, 3, (RNQ,  oRNQ,  I64),  neon_rshift_round_imm),
 NUF(vrsra,     0800310, 3, (RNDQ, oRNDQ, I64),  neon_rshift_round_imm),
 NUF(vrsraq,    0800310, 3, (RNQ,  oRNQ,  I64),  neon_rshift_round_imm),
  /* Shift and insert. Sizes accepted 8 16 32 64.  */
 NUF(vsliq,     1800510, 3, (RNQ,  oRNQ,  I63), neon_sli),
 NUF(vsriq,     1800410, 3, (RNQ,  oRNQ,  I64), neon_sri),
  /* QSHL{U} immediate accepts S8 S16 S32 S64 U8 U16 U32 U64.  */
 NUF(vqshluq,   1800610, 3, (RNQ,  oRNQ,  I63), neon_qshlu_imm),
  /* Right shift immediate, saturating & narrowing, with rounding variants.
     Types accepted S16 S32 S64 U16 U32 U64.  */
 NUF(vqshrn,    0800910, 3, (RND, RNQ, I32z), neon_rshift_sat_narrow),
 NUF(vqrshrn,   0800950, 3, (RND, RNQ, I32z), neon_rshift_sat_narrow),
  /* As above, unsigned. Types accepted S16 S32 S64.  */
 NUF(vqshrun,   0800810, 3, (RND, RNQ, I32z), neon_rshift_sat_narrow_u),
 NUF(vqrshrun,  0800850, 3, (RND, RNQ, I32z), neon_rshift_sat_narrow_u),
  /* Right shift narrowing. Types accepted I16 I32 I64.  */
 NUF(vshrn,     0800810, 3, (RND, RNQ, I32z), neon_rshift_narrow),
 NUF(vrshrn,    0800850, 3, (RND, RNQ, I32z), neon_rshift_narrow),
  /* Special case. Types S8 S16 S32 U8 U16 U32. Handles max shift variant.  */
 nUF(vshll,     _vshll,   3, (RNQ, RND, I32),  neon_shll),
  /* CVT with optional immediate for fixed-point variant.  */
 nUF(vcvtq,     _vcvt,    3, (RNQ, RNQ, oI32b), neon_cvt),

 nUF(vmvnq,     _vmvn,    2, (RNQ,  RNDQ_Ibig), neon_mvn),

  /* Data processing, three registers of different lengths.  */
  /* Dyadic, long insns. Types S8 S16 S32 U8 U16 U32.  */
 NUF(vabal,     0800500, 3, (RNQ, RND, RND),  neon_abal),
  /* If not scalar, fall back to neon_dyadic_long.
     Vector types as above, scalar types S16 S32 U16 U32.  */
 nUF(vmlal,     _vmlal,   3, (RNQ, RND, RND_RNSC), neon_mac_maybe_scalar_long),
 nUF(vmlsl,     _vmlsl,   3, (RNQ, RND, RND_RNSC), neon_mac_maybe_scalar_long),
  /* Dyadic, widening insns. Types S8 S16 S32 U8 U16 U32.  */
 NUF(vaddw,     0800100, 3, (RNQ, oRNQ, RND), neon_dyadic_wide),
 NUF(vsubw,     0800300, 3, (RNQ, oRNQ, RND), neon_dyadic_wide),
  /* Dyadic, narrowing insns. Types I16 I32 I64.  */
 NUF(vaddhn,    0800400, 3, (RND, RNQ, RNQ),  neon_dyadic_narrow),
 NUF(vraddhn,   1800400, 3, (RND, RNQ, RNQ),  neon_dyadic_narrow),
 NUF(vsubhn,    0800600, 3, (RND, RNQ, RNQ),  neon_dyadic_narrow),
 NUF(vrsubhn,   1800600, 3, (RND, RNQ, RNQ),  neon_dyadic_narrow),
  /* Saturating doubling multiplies. Types S16 S32.  */
 nUF(vqdmlal,   _vqdmlal, 3, (RNQ, RND, RND_RNSC), neon_mul_sat_scalar_long),
 nUF(vqdmlsl,   _vqdmlsl, 3, (RNQ, RND, RND_RNSC), neon_mul_sat_scalar_long),
 nUF(vqdmull,   _vqdmull, 3, (RNQ, RND, RND_RNSC), neon_mul_sat_scalar_long),
  /* VMULL. Vector types S8 S16 S32 U8 U16 U32 P8, scalar types
     S16 S32 U16 U32.  */
 nUF(vmull,     _vmull,   3, (RNQ, RND, RND_RNSC), neon_vmull),

  /* Extract. Size 8.  */
 NUF(vext,      0b00000, 4, (RNDQ, oRNDQ, RNDQ, I15), neon_ext),
 NUF(vextq,     0b00000, 4, (RNQ,  oRNQ,  RNQ,  I15), neon_ext),

  /* Two registers, miscellaneous.  */
  /* Reverse. Sizes 8 16 32 (must be < size in opcode).  */
 NUF(vrev64q,   1b00000, 2, (RNQ,  RNQ),      neon_rev),
 NUF(vrev32q,   1b00080, 2, (RNQ,  RNQ),      neon_rev),
 NUF(vrev16q,   1b00100, 2, (RNQ,  RNQ),      neon_rev),
  /* Vector replicate. Sizes 8 16 32.  */
 nCE(vdupq,     _vdup,    2, (RNQ,  RR_RNSC),  neon_dup),
  /* VMOVL. Types S8 S16 S32 U8 U16 U32.  */
 NUF(vmovl,     0800a10, 2, (RNQ, RND),       neon_movl),
  /* VMOVN. Types I16 I32 I64.  */
 nUF(vmovn,     _vmovn,   2, (RND, RNQ),       neon_movn),
  /* VQMOVN. Types S16 S32 S64 U16 U32 U64.  */
 nUF(vqmovn,    _vqmovn,  2, (RND, RNQ),       neon_qmovn),
  /* VQMOVUN. Types S16 S32 S64.  */
 nUF(vqmovun,   _vqmovun, 2, (RND, RNQ),       neon_qmovun),
  /* VZIP / VUZP. Sizes 8 16 32.  */
 NUF(vzip,      1b20180, 2, (RNDQ, RNDQ),     neon_zip_uzp),
 NUF(vzipq,     1b20180, 2, (RNQ,  RNQ),      neon_zip_uzp),
 NUF(vuzp,      1b20100, 2, (RNDQ, RNDQ),     neon_zip_uzp),
 NUF(vuzpq,     1b20100, 2, (RNQ,  RNQ),      neon_zip_uzp),
  /* VQABS / VQNEG. Types S8 S16 S32.  */
 NUF(vqabsq,    1b00700, 2, (RNQ,  RNQ),      neon_sat_abs_neg),
 NUF(vqnegq,    1b00780, 2, (RNQ,  RNQ),      neon_sat_abs_neg),
  /* Pairwise, lengthening. Types S8 S16 S32 U8 U16 U32.  */
 NUF(vpadal,    1b00600, 2, (RNDQ, RNDQ),     neon_pair_long),
 NUF(vpadalq,   1b00600, 2, (RNQ,  RNQ),      neon_pair_long),
 NUF(vpaddl,    1b00200, 2, (RNDQ, RNDQ),     neon_pair_long),
 NUF(vpaddlq,   1b00200, 2, (RNQ,  RNQ),      neon_pair_long),
  /* Reciprocal estimates.  Types U32 F16 F32.  */
 NUF(vrecpe,    1b30400, 2, (RNDQ, RNDQ),     neon_recip_est),
 NUF(vrecpeq,   1b30400, 2, (RNQ,  RNQ),      neon_recip_est),
 NUF(vrsqrte,   1b30480, 2, (RNDQ, RNDQ),     neon_recip_est),
 NUF(vrsqrteq,  1b30480, 2, (RNQ,  RNQ),      neon_recip_est),
  /* VCLS. Types S8 S16 S32.  */
 NUF(vclsq,     1b00400, 2, (RNQ,  RNQ),      neon_cls),
  /* VCLZ. Types I8 I16 I32.  */
 NUF(vclzq,     1b00480, 2, (RNQ,  RNQ),      neon_clz),
  /* VCNT. Size 8.  */
 NUF(vcnt,      1b00500, 2, (RNDQ, RNDQ),     neon_cnt),
 NUF(vcntq,     1b00500, 2, (RNQ,  RNQ),      neon_cnt),
  /* Two address, untyped.  */
 NUF(vswp,      1b20000, 2, (RNDQ, RNDQ),     neon_swp),
 NUF(vswpq,     1b20000, 2, (RNQ,  RNQ),      neon_swp),
  /* VTRN. Sizes 8 16 32.  */
 nUF(vtrn,      _vtrn,    2, (RNDQ, RNDQ),     neon_trn),
 nUF(vtrnq,     _vtrn,    2, (RNQ,  RNQ),      neon_trn),

  /* Table lookup. Size 8.  */
 NUF(vtbl,      1b00800, 3, (RND, NRDLST, RND), neon_tbl_tbx),
 NUF(vtbx,      1b00840, 3, (RND, NRDLST, RND), neon_tbl_tbx),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & fpu_vfp_v3_or_neon_ext
#undef  ARM_VARIANT
#define ARM_VARIANT    & fpu_vfp_v3_or_neon_ext

  /* Neon element/structure load/store.  */
 nUF(vld1,      _vld1,    2, (NSTRLST, ADDR),  neon_ldx_stx),
 nUF(vst1,      _vst1,    2, (NSTRLST, ADDR),  neon_ldx_stx),
 nUF(vld2,      _vld2,    2, (NSTRLST, ADDR),  neon_ldx_stx),
 nUF(vst2,      _vst2,    2, (NSTRLST, ADDR),  neon_ldx_stx),
 nUF(vld3,      _vld3,    2, (NSTRLST, ADDR),  neon_ldx_stx),
 nUF(vst3,      _vst3,    2, (NSTRLST, ADDR),  neon_ldx_stx),
 nUF(vld4,      _vld4,    2, (NSTRLST, ADDR),  neon_ldx_stx),
 nUF(vst4,      _vst4,    2, (NSTRLST, ADDR),  neon_ldx_stx),

#undef  THUMB_VARIANT
#define THUMB_VARIANT & fpu_vfp_ext_v3xd
#undef  ARM_VARIANT
#define ARM_VARIANT   & fpu_vfp_ext_v3xd
 cCE("fconsts",   eb00a00, 2, (RVS, I255),      vfp_sp_const),
 cCE("fshtos",    eba0a40, 2, (RVS, I16z),      vfp_sp_conv_16),
 cCE("fsltos",    eba0ac0, 2, (RVS, I32),       vfp_sp_conv_32),
 cCE("fuhtos",    ebb0a40, 2, (RVS, I16z),      vfp_sp_conv_16),
 cCE("fultos",    ebb0ac0, 2, (RVS, I32),       vfp_sp_conv_32),
 cCE("ftoshs",    ebe0a40, 2, (RVS, I16z),      vfp_sp_conv_16),
 cCE("ftosls",    ebe0ac0, 2, (RVS, I32),       vfp_sp_conv_32),
 cCE("ftouhs",    ebf0a40, 2, (RVS, I16z),      vfp_sp_conv_16),
 cCE("ftouls",    ebf0ac0, 2, (RVS, I32),       vfp_sp_conv_32),

#undef  THUMB_VARIANT
#define THUMB_VARIANT  & fpu_vfp_ext_v3
#undef  ARM_VARIANT
#define ARM_VARIANT    & fpu_vfp_ext_v3

 cCE("fconstd",   eb00b00, 2, (RVD, I255),      vfp_dp_const),
 cCE("fshtod",    eba0b40, 2, (RVD, I16z),      vfp_dp_conv_16),
 cCE("fsltod",    eba0bc0, 2, (RVD, I32),       vfp_dp_conv_32),
 cCE("fuhtod",    ebb0b40, 2, (RVD, I16z),      vfp_dp_conv_16),
 cCE("fultod",    ebb0bc0, 2, (RVD, I32),       vfp_dp_conv_32),
 cCE("ftoshd",    ebe0b40, 2, (RVD, I16z),      vfp_dp_conv_16),
 cCE("ftosld",    ebe0bc0, 2, (RVD, I32),       vfp_dp_conv_32),
 cCE("ftouhd",    ebf0b40, 2, (RVD, I16z),      vfp_dp_conv_16),
 cCE("ftould",    ebf0bc0, 2, (RVD, I32),       vfp_dp_conv_32),

#undef  ARM_VARIANT
#define ARM_VARIANT    & fpu_vfp_ext_fma
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & fpu_vfp_ext_fma
 /* Mnemonics shared by Neon, VFP, MVE and BF16.  These are included in the
    VFP FMA variant; NEON and VFP FMA always includes the NEON
    FMA instructions.  */
 mnCEF(vfma,     _vfma,    3, (RNSDQMQ, oRNSDQMQ, RNSDQMQR), neon_fmac),
 TUF ("vfmat",    c300850,    fc300850,  3, (RNSDQMQ, oRNSDQMQ, RNSDQ_RNSC_MQ_RR), mve_vfma, mve_vfma),
 mnCEF(vfms,     _vfms,    3, (RNSDQMQ, oRNSDQMQ, RNSDQMQ),  neon_fmac),

 /* ffmas/ffmad/ffmss/ffmsd are dummy mnemonics to satisfy gas;
    the v form should always be used.  */
 cCE("ffmas",	ea00a00, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("ffnmas",	ea00a40, 3, (RVS, RVS, RVS),  vfp_sp_dyadic),
 cCE("ffmad",	ea00b00, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 cCE("ffnmad",	ea00b40, 3, (RVD, RVD, RVD),  vfp_dp_rd_rn_rm),
 nCE(vfnma,     _vfnma,   3, (RVSD, RVSD, RVSD), vfp_nsyn_nmul),
 nCE(vfnms,     _vfnms,   3, (RVSD, RVSD, RVSD), vfp_nsyn_nmul),

#undef THUMB_VARIANT
#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_cext_xscale /* Intel XScale extensions.  */

 cCE("mia",	e200010, 3, (RXA, RRnpc, RRnpc), xsc_mia),
 cCE("miaph",	e280010, 3, (RXA, RRnpc, RRnpc), xsc_mia),
 cCE("miabb",	e2c0010, 3, (RXA, RRnpc, RRnpc), xsc_mia),
 cCE("miabt",	e2d0010, 3, (RXA, RRnpc, RRnpc), xsc_mia),
 cCE("miatb",	e2e0010, 3, (RXA, RRnpc, RRnpc), xsc_mia),
 cCE("miatt",	e2f0010, 3, (RXA, RRnpc, RRnpc), xsc_mia),
 cCE("mar",	c400000, 3, (RXA, RRnpc, RRnpc), xsc_mar),
 cCE("mra",	c500000, 3, (RRnpc, RRnpc, RXA), xsc_mra),

#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_cext_iwmmxt /* Intel Wireless MMX technology.  */

 cCE("tandcb",	e13f130, 1, (RR),		    iwmmxt_tandorc),
 cCE("tandch",	e53f130, 1, (RR),		    iwmmxt_tandorc),
 cCE("tandcw",	e93f130, 1, (RR),		    iwmmxt_tandorc),
 cCE("tbcstb",	e400010, 2, (RIWR, RR),		    rn_rd),
 cCE("tbcsth",	e400050, 2, (RIWR, RR),		    rn_rd),
 cCE("tbcstw",	e400090, 2, (RIWR, RR),		    rn_rd),
 cCE("textrcb",	e130170, 2, (RR, I7),		    iwmmxt_textrc),
 cCE("textrch",	e530170, 2, (RR, I7),		    iwmmxt_textrc),
 cCE("textrcw",	e930170, 2, (RR, I7),		    iwmmxt_textrc),
 cCE("textrmub",e100070, 3, (RR, RIWR, I7),	    iwmmxt_textrm),
 cCE("textrmuh",e500070, 3, (RR, RIWR, I7),	    iwmmxt_textrm),
 cCE("textrmuw",e900070, 3, (RR, RIWR, I7),	    iwmmxt_textrm),
 cCE("textrmsb",e100078, 3, (RR, RIWR, I7),	    iwmmxt_textrm),
 cCE("textrmsh",e500078, 3, (RR, RIWR, I7),	    iwmmxt_textrm),
 cCE("textrmsw",e900078, 3, (RR, RIWR, I7),	    iwmmxt_textrm),
 cCE("tinsrb",	e600010, 3, (RIWR, RR, I7),	    iwmmxt_tinsr),
 cCE("tinsrh",	e600050, 3, (RIWR, RR, I7),	    iwmmxt_tinsr),
 cCE("tinsrw",	e600090, 3, (RIWR, RR, I7),	    iwmmxt_tinsr),
 cCE("tmcr",	e000110, 2, (RIWC_RIWG, RR),	    rn_rd),
 cCE("tmcrr",	c400000, 3, (RIWR, RR, RR),	    rm_rd_rn),
 cCE("tmia",	e200010, 3, (RIWR, RR, RR),	    iwmmxt_tmia),
 cCE("tmiaph",	e280010, 3, (RIWR, RR, RR),	    iwmmxt_tmia),
 cCE("tmiabb",	e2c0010, 3, (RIWR, RR, RR),	    iwmmxt_tmia),
 cCE("tmiabt",	e2d0010, 3, (RIWR, RR, RR),	    iwmmxt_tmia),
 cCE("tmiatb",	e2e0010, 3, (RIWR, RR, RR),	    iwmmxt_tmia),
 cCE("tmiatt",	e2f0010, 3, (RIWR, RR, RR),	    iwmmxt_tmia),
 cCE("tmovmskb",e100030, 2, (RR, RIWR),		    rd_rn),
 cCE("tmovmskh",e500030, 2, (RR, RIWR),		    rd_rn),
 cCE("tmovmskw",e900030, 2, (RR, RIWR),		    rd_rn),
 cCE("tmrc",	e100110, 2, (RR, RIWC_RIWG),	    rd_rn),
 cCE("tmrrc",	c500000, 3, (RR, RR, RIWR),	    rd_rn_rm),
 cCE("torcb",	e13f150, 1, (RR),		    iwmmxt_tandorc),
 cCE("torch",	e53f150, 1, (RR),		    iwmmxt_tandorc),
 cCE("torcw",	e93f150, 1, (RR),		    iwmmxt_tandorc),
 cCE("waccb",	e0001c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wacch",	e4001c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("waccw",	e8001c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("waddbss",	e300180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waddb",	e000180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waddbus",	e100180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waddhss",	e700180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waddh",	e400180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waddhus",	e500180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waddwss",	eb00180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waddw",	e800180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waddwus",	e900180, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("waligni",	e000020, 4, (RIWR, RIWR, RIWR, I7), iwmmxt_waligni),
 cCE("walignr0",e800020, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("walignr1",e900020, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("walignr2",ea00020, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("walignr3",eb00020, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wand",	e200000, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wandn",	e300000, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wavg2b",	e800000, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wavg2br",	e900000, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wavg2h",	ec00000, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wavg2hr",	ed00000, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpeqb",	e000060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpeqh",	e400060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpeqw",	e800060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpgtub",e100060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpgtuh",e500060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpgtuw",e900060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpgtsb",e300060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpgtsh",e700060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wcmpgtsw",eb00060, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wldrb",	c100000, 2, (RIWR, ADDR),	    iwmmxt_wldstbh),
 cCE("wldrh",	c500000, 2, (RIWR, ADDR),	    iwmmxt_wldstbh),
 cCE("wldrw",	c100100, 2, (RIWR_RIWC, ADDR),	    iwmmxt_wldstw),
 cCE("wldrd",	c500100, 2, (RIWR, ADDR),	    iwmmxt_wldstd),
 cCE("wmacs",	e600100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmacsz",	e700100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmacu",	e400100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmacuz",	e500100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmadds",	ea00100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmaddu",	e800100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmaxsb",	e200160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmaxsh",	e600160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmaxsw",	ea00160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmaxub",	e000160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmaxuh",	e400160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmaxuw",	e800160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wminsb",	e300160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wminsh",	e700160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wminsw",	eb00160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wminub",	e100160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wminuh",	e500160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wminuw",	e900160, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmov",	e000000, 2, (RIWR, RIWR),	    iwmmxt_wmov),
 cCE("wmulsm",	e300100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmulsl",	e200100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmulum",	e100100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wmulul",	e000100, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wor",	e000000, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wpackhss",e700080, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wpackhus",e500080, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wpackwss",eb00080, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wpackwus",e900080, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wpackdss",ef00080, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wpackdus",ed00080, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wrorh",	e700040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wrorhg",	e700148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wrorw",	eb00040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wrorwg",	eb00148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wrord",	ef00040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wrordg",	ef00148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wsadb",	e000120, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsadbz",	e100120, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsadh",	e400120, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsadhz",	e500120, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wshufh",	e0001e0, 3, (RIWR, RIWR, I255),	    iwmmxt_wshufh),
 cCE("wsllh",	e500040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wsllhg",	e500148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wsllw",	e900040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wsllwg",	e900148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wslld",	ed00040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wslldg",	ed00148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wsrah",	e400040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wsrahg",	e400148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wsraw",	e800040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wsrawg",	e800148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wsrad",	ec00040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wsradg",	ec00148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wsrlh",	e600040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wsrlhg",	e600148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wsrlw",	ea00040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wsrlwg",	ea00148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wsrld",	ee00040, 3, (RIWR, RIWR, RIWR_I32z),iwmmxt_wrwrwr_or_imm5),
 cCE("wsrldg",	ee00148, 3, (RIWR, RIWR, RIWG),	    rd_rn_rm),
 cCE("wstrb",	c000000, 2, (RIWR, ADDR),	    iwmmxt_wldstbh),
 cCE("wstrh",	c400000, 2, (RIWR, ADDR),	    iwmmxt_wldstbh),
 cCE("wstrw",	c000100, 2, (RIWR_RIWC, ADDR),	    iwmmxt_wldstw),
 cCE("wstrd",	c400100, 2, (RIWR, ADDR),	    iwmmxt_wldstd),
 cCE("wsubbss",	e3001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsubb",	e0001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsubbus",	e1001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsubhss",	e7001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsubh",	e4001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsubhus",	e5001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsubwss",	eb001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsubw",	e8001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wsubwus",	e9001a0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wunpckehub",e0000c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckehuh",e4000c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckehuw",e8000c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckehsb",e2000c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckehsh",e6000c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckehsw",ea000c0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckihb", e1000c0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wunpckihh", e5000c0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wunpckihw", e9000c0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wunpckelub",e0000e0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckeluh",e4000e0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckeluw",e8000e0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckelsb",e2000e0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckelsh",e6000e0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckelsw",ea000e0, 2, (RIWR, RIWR),	    rd_rn),
 cCE("wunpckilb", e1000e0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wunpckilh", e5000e0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wunpckilw", e9000e0, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wxor",	e100000, 3, (RIWR, RIWR, RIWR),	    rd_rn_rm),
 cCE("wzero",	e300000, 1, (RIWR),		    iwmmxt_wzero),

#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_cext_iwmmxt2 /* Intel Wireless MMX technology, version 2.  */

 cCE("torvscb",   e12f190, 1, (RR),		    iwmmxt_tandorc),
 cCE("torvsch",   e52f190, 1, (RR),		    iwmmxt_tandorc),
 cCE("torvscw",   e92f190, 1, (RR),		    iwmmxt_tandorc),
 cCE("wabsb",     e2001c0, 2, (RIWR, RIWR),           rd_rn),
 cCE("wabsh",     e6001c0, 2, (RIWR, RIWR),           rd_rn),
 cCE("wabsw",     ea001c0, 2, (RIWR, RIWR),           rd_rn),
 cCE("wabsdiffb", e1001c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wabsdiffh", e5001c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wabsdiffw", e9001c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("waddbhusl", e2001a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("waddbhusm", e6001a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("waddhc",    e600180, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("waddwc",    ea00180, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("waddsubhx", ea001a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wavg4",	e400000, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wavg4r",    e500000, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmaddsn",   ee00100, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmaddsx",   eb00100, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmaddun",   ec00100, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmaddux",   e900100, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmerge",    e000080, 4, (RIWR, RIWR, RIWR, I7), iwmmxt_wmerge),
 cCE("wmiabb",    e0000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiabt",    e1000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiatb",    e2000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiatt",    e3000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiabbn",   e4000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiabtn",   e5000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiatbn",   e6000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiattn",   e7000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiawbb",   e800120, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiawbt",   e900120, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiawtb",   ea00120, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiawtt",   eb00120, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiawbbn",  ec00120, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiawbtn",  ed00120, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiawtbn",  ee00120, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmiawttn",  ef00120, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmulsmr",   ef00100, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmulumr",   ed00100, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmulwumr",  ec000c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmulwsmr",  ee000c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmulwum",   ed000c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmulwsm",   ef000c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wmulwl",    eb000c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmiabb",   e8000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmiabt",   e9000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmiatb",   ea000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmiatt",   eb000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmiabbn",  ec000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmiabtn",  ed000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmiatbn",  ee000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmiattn",  ef000a0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmulm",    e100080, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmulmr",   e300080, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmulwm",   ec000e0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wqmulwmr",  ee000e0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),
 cCE("wsubaddhx", ed001c0, 3, (RIWR, RIWR, RIWR),     rd_rn_rm),

#undef  ARM_VARIANT
#define ARM_VARIANT  & arm_cext_maverick /* Cirrus Maverick instructions.  */

 cCE("cfldrs",	c100400, 2, (RMF, ADDRGLDC),	      rd_cpaddr),
 cCE("cfldrd",	c500400, 2, (RMD, ADDRGLDC),	      rd_cpaddr),
 cCE("cfldr32",	c100500, 2, (RMFX, ADDRGLDC),	      rd_cpaddr),
 cCE("cfldr64",	c500500, 2, (RMDX, ADDRGLDC),	      rd_cpaddr),
 cCE("cfstrs",	c000400, 2, (RMF, ADDRGLDC),	      rd_cpaddr),
 cCE("cfstrd",	c400400, 2, (RMD, ADDRGLDC),	      rd_cpaddr),
 cCE("cfstr32",	c000500, 2, (RMFX, ADDRGLDC),	      rd_cpaddr),
 cCE("cfstr64",	c400500, 2, (RMDX, ADDRGLDC),	      rd_cpaddr),
 cCE("cfmvsr",	e000450, 2, (RMF, RR),		      rn_rd),
 cCE("cfmvrs",	e100450, 2, (RR, RMF),		      rd_rn),
 cCE("cfmvdlr",	e000410, 2, (RMD, RR),		      rn_rd),
 cCE("cfmvrdl",	e100410, 2, (RR, RMD),		      rd_rn),
 cCE("cfmvdhr",	e000430, 2, (RMD, RR),		      rn_rd),
 cCE("cfmvrdh",	e100430, 2, (RR, RMD),		      rd_rn),
 cCE("cfmv64lr",e000510, 2, (RMDX, RR),		      rn_rd),
 cCE("cfmvr64l",e100510, 2, (RR, RMDX),		      rd_rn),
 cCE("cfmv64hr",e000530, 2, (RMDX, RR),		      rn_rd),
 cCE("cfmvr64h",e100530, 2, (RR, RMDX),		      rd_rn),
 cCE("cfmval32",e200440, 2, (RMAX, RMFX),	      rd_rn),
 cCE("cfmv32al",e100440, 2, (RMFX, RMAX),	      rd_rn),
 cCE("cfmvam32",e200460, 2, (RMAX, RMFX),	      rd_rn),
 cCE("cfmv32am",e100460, 2, (RMFX, RMAX),	      rd_rn),
 cCE("cfmvah32",e200480, 2, (RMAX, RMFX),	      rd_rn),
 cCE("cfmv32ah",e100480, 2, (RMFX, RMAX),	      rd_rn),
 cCE("cfmva32",	e2004a0, 2, (RMAX, RMFX),	      rd_rn),
 cCE("cfmv32a",	e1004a0, 2, (RMFX, RMAX),	      rd_rn),
 cCE("cfmva64",	e2004c0, 2, (RMAX, RMDX),	      rd_rn),
 cCE("cfmv64a",	e1004c0, 2, (RMDX, RMAX),	      rd_rn),
 cCE("cfmvsc32",e2004e0, 2, (RMDS, RMDX),	      mav_dspsc),
 cCE("cfmv32sc",e1004e0, 2, (RMDX, RMDS),	      rd),
 cCE("cfcpys",	e000400, 2, (RMF, RMF),		      rd_rn),
 cCE("cfcpyd",	e000420, 2, (RMD, RMD),		      rd_rn),
 cCE("cfcvtsd",	e000460, 2, (RMD, RMF),		      rd_rn),
 cCE("cfcvtds",	e000440, 2, (RMF, RMD),		      rd_rn),
 cCE("cfcvt32s",e000480, 2, (RMF, RMFX),	      rd_rn),
 cCE("cfcvt32d",e0004a0, 2, (RMD, RMFX),	      rd_rn),
 cCE("cfcvt64s",e0004c0, 2, (RMF, RMDX),	      rd_rn),
 cCE("cfcvt64d",e0004e0, 2, (RMD, RMDX),	      rd_rn),
 cCE("cfcvts32",e100580, 2, (RMFX, RMF),	      rd_rn),
 cCE("cfcvtd32",e1005a0, 2, (RMFX, RMD),	      rd_rn),
 cCE("cftruncs32",e1005c0, 2, (RMFX, RMF),	      rd_rn),
 cCE("cftruncd32",e1005e0, 2, (RMFX, RMD),	      rd_rn),
 cCE("cfrshl32",e000550, 3, (RMFX, RMFX, RR),	      mav_triple),
 cCE("cfrshl64",e000570, 3, (RMDX, RMDX, RR),	      mav_triple),
 cCE("cfsh32",	e000500, 3, (RMFX, RMFX, I63s),	      mav_shift),
 cCE("cfsh64",	e200500, 3, (RMDX, RMDX, I63s),	      mav_shift),
 cCE("cfcmps",	e100490, 3, (RR, RMF, RMF),	      rd_rn_rm),
 cCE("cfcmpd",	e1004b0, 3, (RR, RMD, RMD),	      rd_rn_rm),
 cCE("cfcmp32",	e100590, 3, (RR, RMFX, RMFX),	      rd_rn_rm),
 cCE("cfcmp64",	e1005b0, 3, (RR, RMDX, RMDX),	      rd_rn_rm),
 cCE("cfabss",	e300400, 2, (RMF, RMF),		      rd_rn),
 cCE("cfabsd",	e300420, 2, (RMD, RMD),		      rd_rn),
 cCE("cfnegs",	e300440, 2, (RMF, RMF),		      rd_rn),
 cCE("cfnegd",	e300460, 2, (RMD, RMD),		      rd_rn),
 cCE("cfadds",	e300480, 3, (RMF, RMF, RMF),	      rd_rn_rm),
 cCE("cfaddd",	e3004a0, 3, (RMD, RMD, RMD),	      rd_rn_rm),
 cCE("cfsubs",	e3004c0, 3, (RMF, RMF, RMF),	      rd_rn_rm),
 cCE("cfsubd",	e3004e0, 3, (RMD, RMD, RMD),	      rd_rn_rm),
 cCE("cfmuls",	e100400, 3, (RMF, RMF, RMF),	      rd_rn_rm),
 cCE("cfmuld",	e100420, 3, (RMD, RMD, RMD),	      rd_rn_rm),
 cCE("cfabs32",	e300500, 2, (RMFX, RMFX),	      rd_rn),
 cCE("cfabs64",	e300520, 2, (RMDX, RMDX),	      rd_rn),
 cCE("cfneg32",	e300540, 2, (RMFX, RMFX),	      rd_rn),
 cCE("cfneg64",	e300560, 2, (RMDX, RMDX),	      rd_rn),
 cCE("cfadd32",	e300580, 3, (RMFX, RMFX, RMFX),	      rd_rn_rm),
 cCE("cfadd64",	e3005a0, 3, (RMDX, RMDX, RMDX),	      rd_rn_rm),
 cCE("cfsub32",	e3005c0, 3, (RMFX, RMFX, RMFX),	      rd_rn_rm),
 cCE("cfsub64",	e3005e0, 3, (RMDX, RMDX, RMDX),	      rd_rn_rm),
 cCE("cfmul32",	e100500, 3, (RMFX, RMFX, RMFX),	      rd_rn_rm),
 cCE("cfmul64",	e100520, 3, (RMDX, RMDX, RMDX),	      rd_rn_rm),
 cCE("cfmac32",	e100540, 3, (RMFX, RMFX, RMFX),	      rd_rn_rm),
 cCE("cfmsc32",	e100560, 3, (RMFX, RMFX, RMFX),	      rd_rn_rm),
 cCE("cfmadd32",e000600, 4, (RMAX, RMFX, RMFX, RMFX), mav_quad),
 cCE("cfmsub32",e100600, 4, (RMAX, RMFX, RMFX, RMFX), mav_quad),
 cCE("cfmadda32", e200600, 4, (RMAX, RMAX, RMFX, RMFX), mav_quad),
 cCE("cfmsuba32", e300600, 4, (RMAX, RMAX, RMFX, RMFX), mav_quad),

 /* ARMv8.5-A instructions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT   & arm_ext_sb
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_sb
 TUF("sb", 57ff070, f3bf8f70, 0, (), noargs, noargs),

#undef  ARM_VARIANT
#define ARM_VARIANT   & arm_ext_predres
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_predres
 CE("cfprctx", e070f93, 1, (RRnpc), rd),
 CE("dvprctx", e070fb3, 1, (RRnpc), rd),
 CE("cpprctx", e070ff3, 1, (RRnpc), rd),

 /* ARMv8-M instructions.  */
#undef  ARM_VARIANT
#define ARM_VARIANT NULL
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_v8m
 ToU("sg",    e97fe97f,	0, (),		   noargs),
 ToC("blxns", 4784,	1, (RRnpc),	   t_blx),
 ToC("bxns",  4704,	1, (RRnpc),	   t_bx),
 ToC("tt",    e840f000,	2, (RRnpc, RRnpc), tt),
 ToC("ttt",   e840f040,	2, (RRnpc, RRnpc), tt),
 ToC("tta",   e840f080,	2, (RRnpc, RRnpc), tt),
 ToC("ttat",  e840f0c0,	2, (RRnpc, RRnpc), tt),

 /* FP for ARMv8-M Mainline.  Enabled for ARMv8-M Mainline because the
    instructions behave as nop if no VFP is present.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_v8m_main
 ToC("vlldm", ec300a00, 1, (RRnpc), rn),
 ToC("vlstm", ec200a00, 1, (RRnpc), rn),

 /* Armv8.1-M Mainline instructions.  */
#undef  THUMB_VARIANT
#define THUMB_VARIANT & arm_ext_v8_1m_main
 toU("aut",   _aut, 3, (R12, LR, SP), t_pacbti),
 toU("autg",  _autg, 3, (RR, RR, RR), t_pacbti_nonop),
 ToU("bti",   f3af800f, 0, (), noargs),
 toU("bxaut", _bxaut, 3, (RR, RR, RR), t_pacbti_nonop),
 toU("pac",   _pac,   3, (R12, LR, SP), t_pacbti),
 toU("pacbti", _pacbti, 3, (R12, LR, SP), t_pacbti),
 toU("pacg",   _pacg,   3, (RR, RR, RR), t_pacbti_pacg),
 toU("cinc",  _cinc,  3, (RRnpcsp, RR_ZR, COND),	t_cond),
 toU("cinv",  _cinv,  3, (RRnpcsp, RR_ZR, COND),	t_cond),
 toU("cneg",  _cneg,  3, (RRnpcsp, RR_ZR, COND),	t_cond),
 toU("csel",  _csel,  4, (RRnpcsp, RR_ZR, RR_ZR, COND),	t_cond),
 toU("csetm", _csetm, 2, (RRnpcsp, COND),		t_cond),
 toU("cset",  _cset,  2, (RRnpcsp, COND),		t_cond),
 toU("csinc", _csinc, 4, (RRnpcsp, RR_ZR, RR_ZR, COND),	t_cond),
 toU("csinv", _csinv, 4, (RRnpcsp, RR_ZR, RR_ZR, COND),	t_cond),
 toU("csneg", _csneg, 4, (RRnpcsp, RR_ZR, RR_ZR, COND),	t_cond),

 toC("bf",     _bf,	2, (EXPs, EXPs),	     t_branch_future),
 toU("bfcsel", _bfcsel,	4, (EXPs, EXPs, EXPs, COND), t_branch_future),
 toC("bfx",    _bfx,	2, (EXPs, RRnpcsp),	     t_branch_future),
 toC("bfl",    _bfl,	2, (EXPs, EXPs),	     t_branch_future),
 toC("bflx",   _bflx,	2, (EXPs, RRnpcsp),	     t_branch_future),

 toU("dls", _dls, 2, (LR, RRnpcsp),	 t_loloop),
 toU("wls", _wls, 3, (LR, RRnpcsp, EXP), t_loloop),
 toU("le",  _le,  2, (oLR, EXP),	 t_loloop),

 ToC("clrm",	e89f0000, 1, (CLRMLST),  t_clrm),
 ToC("vscclrm",	ec9f0a00, 1, (VRSDVLST), t_vscclrm),

#undef  THUMB_VARIANT
#define THUMB_VARIANT & mve_ext
 ToC("lsll",	ea50010d, 3, (RRe, RRo, RRnpcsp_I32), mve_scalar_shift),
 ToC("lsrl",	ea50011f, 3, (RRe, RRo, I32),	      mve_scalar_shift),
 ToC("asrl",	ea50012d, 3, (RRe, RRo, RRnpcsp_I32), mve_scalar_shift),
 ToC("uqrshll",	ea51010d, 4, (RRe, RRo, I48_I64, RRnpcsp), mve_scalar_shift1),
 ToC("sqrshrl",	ea51012d, 4, (RRe, RRo, I48_I64, RRnpcsp), mve_scalar_shift1),
 ToC("uqshll",	ea51010f, 3, (RRe, RRo, I32),	      mve_scalar_shift),
 ToC("urshrl",	ea51011f, 3, (RRe, RRo, I32),	      mve_scalar_shift),
 ToC("srshrl",	ea51012f, 3, (RRe, RRo, I32),	      mve_scalar_shift),
 ToC("sqshll",	ea51013f, 3, (RRe, RRo, I32),	      mve_scalar_shift),
 ToC("uqrshl",	ea500f0d, 2, (RRnpcsp, RRnpcsp),      mve_scalar_shift),
 ToC("sqrshr",	ea500f2d, 2, (RRnpcsp, RRnpcsp),      mve_scalar_shift),
 ToC("uqshl",	ea500f0f, 2, (RRnpcsp, I32),	      mve_scalar_shift),
 ToC("urshr",	ea500f1f, 2, (RRnpcsp, I32),	      mve_scalar_shift),
 ToC("srshr",	ea500f2f, 2, (RRnpcsp, I32),	      mve_scalar_shift),
 ToC("sqshl",	ea500f3f, 2, (RRnpcsp, I32),	      mve_scalar_shift),

 ToC("vpt",	ee410f00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vptt",	ee018f00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vpte",	ee418f00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vpttt",	ee014f00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vptte",	ee01cf00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vptet",	ee41cf00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vptee",	ee414f00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vptttt",	ee012f00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vpttte",	ee016f00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vpttet",	ee01ef00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vpttee",	ee01af00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vptett",	ee41af00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vptete",	ee41ef00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vpteet",	ee416f00, 3, (COND, RMQ, RMQRZ), mve_vpt),
 ToC("vpteee",	ee412f00, 3, (COND, RMQ, RMQRZ), mve_vpt),

 ToC("vpst",	fe710f4d, 0, (), mve_vpt),
 ToC("vpstt",	fe318f4d, 0, (), mve_vpt),
 ToC("vpste",	fe718f4d, 0, (), mve_vpt),
 ToC("vpsttt",	fe314f4d, 0, (), mve_vpt),
 ToC("vpstte",	fe31cf4d, 0, (), mve_vpt),
 ToC("vpstet",	fe71cf4d, 0, (), mve_vpt),
 ToC("vpstee",	fe714f4d, 0, (), mve_vpt),
 ToC("vpstttt",	fe312f4d, 0, (), mve_vpt),
 ToC("vpsttte", fe316f4d, 0, (), mve_vpt),
 ToC("vpsttet",	fe31ef4d, 0, (), mve_vpt),
 ToC("vpsttee",	fe31af4d, 0, (), mve_vpt),
 ToC("vpstett",	fe71af4d, 0, (), mve_vpt),
 ToC("vpstete",	fe71ef4d, 0, (), mve_vpt),
 ToC("vpsteet",	fe716f4d, 0, (), mve_vpt),
 ToC("vpsteee",	fe712f4d, 0, (), mve_vpt),

 /* MVE and MVE FP only.  */
 mToC("vhcadd",	ee000f00,   4, (RMQ, RMQ, RMQ, EXPi),		  mve_vhcadd),
 mCEF(vctp,	_vctp,      1, (RRnpc),				  mve_vctp),
 mCEF(vadc,	_vadc,      3, (RMQ, RMQ, RMQ),			  mve_vadc),
 mCEF(vadci,	_vadci,     3, (RMQ, RMQ, RMQ),			  mve_vadc),
 mToC("vsbc",	fe300f00,   3, (RMQ, RMQ, RMQ),			  mve_vsbc),
 mToC("vsbci",	fe301f00,   3, (RMQ, RMQ, RMQ),			  mve_vsbc),
 mCEF(vmullb,	_vmullb,    3, (RMQ, RMQ, RMQ),			  mve_vmull),
 mCEF(vabav,	_vabav,	    3, (RRnpcsp, RMQ, RMQ),		  mve_vabav),
 mCEF(vmladav,	  _vmladav,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmladava,	  _vmladava,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmladavx,	  _vmladavx,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmladavax,  _vmladavax,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmlav,	  _vmladav,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmlava,	  _vmladava,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmlsdav,	  _vmlsdav,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmlsdava,	  _vmlsdava,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmlsdavx,	  _vmlsdavx,	3, (RRe, RMQ, RMQ),		mve_vmladav),
 mCEF(vmlsdavax,  _vmlsdavax,	3, (RRe, RMQ, RMQ),		mve_vmladav),

 mCEF(vst20,	_vst20,	    2, (MSTRLST2, ADDRMVE),		mve_vst_vld),
 mCEF(vst21,	_vst21,	    2, (MSTRLST2, ADDRMVE),		mve_vst_vld),
 mCEF(vst40,	_vst40,	    2, (MSTRLST4, ADDRMVE),		mve_vst_vld),
 mCEF(vst41,	_vst41,	    2, (MSTRLST4, ADDRMVE),		mve_vst_vld),
 mCEF(vst42,	_vst42,	    2, (MSTRLST4, ADDRMVE),		mve_vst_vld),
 mCEF(vst43,	_vst43,	    2, (MSTRLST4, ADDRMVE),		mve_vst_vld),
 mCEF(vld20,	_vld20,	    2, (MSTRLST2, ADDRMVE),		mve_vst_vld),
 mCEF(vld21,	_vld21,	    2, (MSTRLST2, ADDRMVE),		mve_vst_vld),
 mCEF(vld40,	_vld40,	    2, (MSTRLST4, ADDRMVE),		mve_vst_vld),
 mCEF(vld41,	_vld41,	    2, (MSTRLST4, ADDRMVE),		mve_vst_vld),
 mCEF(vld42,	_vld42,	    2, (MSTRLST4, ADDRMVE),		mve_vst_vld),
 mCEF(vld43,	_vld43,	    2, (MSTRLST4, ADDRMVE),		mve_vst_vld),
 mCEF(vstrb,	_vstrb,	    2, (RMQ, ADDRMVE),			mve_vstr_vldr),
 mCEF(vstrh,	_vstrh,	    2, (RMQ, ADDRMVE),			mve_vstr_vldr),
 mCEF(vstrw,	_vstrw,	    2, (RMQ, ADDRMVE),			mve_vstr_vldr),
 mCEF(vstrd,	_vstrd,	    2, (RMQ, ADDRMVE),			mve_vstr_vldr),
 mCEF(vldrb,	_vldrb,	    2, (RMQ, ADDRMVE),			mve_vstr_vldr),
 mCEF(vldrh,	_vldrh,	    2, (RMQ, ADDRMVE),			mve_vstr_vldr),
 mCEF(vldrw,	_vldrw,	    2, (RMQ, ADDRMVE),			mve_vstr_vldr),
 mCEF(vldrd,	_vldrd,	    2, (RMQ, ADDRMVE),			mve_vstr_vldr),

 mCEF(vmovnt,	_vmovnt,    2, (RMQ, RMQ),			  mve_movn),
 mCEF(vmovnb,	_vmovnb,    2, (RMQ, RMQ),			  mve_movn),
 mCEF(vbrsr,	_vbrsr,     3, (RMQ, RMQ, RR),			  mve_vbrsr),
 mCEF(vaddlv,	_vaddlv,    3, (RRe, RRo, RMQ),			  mve_vaddlv),
 mCEF(vaddlva,	_vaddlva,   3, (RRe, RRo, RMQ),			  mve_vaddlv),
 mCEF(vaddv,	_vaddv,	    2, (RRe, RMQ),			  mve_vaddv),
 mCEF(vaddva,	_vaddva,    2, (RRe, RMQ),			  mve_vaddv),
 mCEF(vddup,	_vddup,	    3, (RMQ, RRe, EXPi),		  mve_viddup),
 mCEF(vdwdup,	_vdwdup,    4, (RMQ, RRe, RR, EXPi),		  mve_viddup),
 mCEF(vidup,	_vidup,	    3, (RMQ, RRe, EXPi),		  mve_viddup),
 mCEF(viwdup,	_viwdup,    4, (RMQ, RRe, RR, EXPi),		  mve_viddup),
 mToC("vmaxa",	ee330e81,   2, (RMQ, RMQ),			  mve_vmaxa_vmina),
 mToC("vmina",	ee331e81,   2, (RMQ, RMQ),			  mve_vmaxa_vmina),
 mCEF(vmaxv,	_vmaxv,	  2, (RR, RMQ),				  mve_vmaxv),
 mCEF(vmaxav,	_vmaxav,  2, (RR, RMQ),				  mve_vmaxv),
 mCEF(vminv,	_vminv,	  2, (RR, RMQ),				  mve_vmaxv),
 mCEF(vminav,	_vminav,  2, (RR, RMQ),				  mve_vmaxv),

 mCEF(vmlaldav,	  _vmlaldav,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlaldava,  _vmlaldava,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlaldavx,  _vmlaldavx,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlaldavax, _vmlaldavax,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlalv,	  _vmlaldav,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlalva,	  _vmlaldava,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlsldav,	  _vmlsldav,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlsldava,  _vmlsldava,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlsldavx,  _vmlsldavx,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mCEF(vmlsldavax, _vmlsldavax,	4, (RRe, RRo, RMQ, RMQ),	mve_vmlaldav),
 mToC("vrmlaldavh", ee800f00,	   4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mToC("vrmlaldavha",ee800f20,	   4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mCEF(vrmlaldavhx,  _vrmlaldavhx,  4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mCEF(vrmlaldavhax, _vrmlaldavhax, 4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mToC("vrmlalvh",   ee800f00,	   4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mToC("vrmlalvha",  ee800f20,	   4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mCEF(vrmlsldavh,   _vrmlsldavh,   4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mCEF(vrmlsldavha,  _vrmlsldavha,  4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mCEF(vrmlsldavhx,  _vrmlsldavhx,  4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),
 mCEF(vrmlsldavhax, _vrmlsldavhax, 4, (RRe, RR, RMQ, RMQ),  mve_vrmlaldavh),

 mToC("vmlas",	  ee011e40,	3, (RMQ, RMQ, RR),		mve_vmlas),
 mToC("vmulh",	  ee010e01,	3, (RMQ, RMQ, RMQ),		mve_vmulh),
 mToC("vrmulh",	  ee011e01,	3, (RMQ, RMQ, RMQ),		mve_vmulh),
 mToC("vpnot",	  fe310f4d,	0, (),				mve_vpnot),
 mToC("vpsel",	  fe310f01,	3, (RMQ, RMQ, RMQ),		mve_vpsel),

 mToC("vqdmladh",  ee000e00,	3, (RMQ, RMQ, RMQ),		mve_vqdmladh),
 mToC("vqdmladhx", ee001e00,	3, (RMQ, RMQ, RMQ),		mve_vqdmladh),
 mToC("vqrdmladh", ee000e01,	3, (RMQ, RMQ, RMQ),		mve_vqdmladh),
 mToC("vqrdmladhx",ee001e01,	3, (RMQ, RMQ, RMQ),		mve_vqdmladh),
 mToC("vqdmlsdh",  fe000e00,	3, (RMQ, RMQ, RMQ),		mve_vqdmladh),
 mToC("vqdmlsdhx", fe001e00,	3, (RMQ, RMQ, RMQ),		mve_vqdmladh),
 mToC("vqrdmlsdh", fe000e01,	3, (RMQ, RMQ, RMQ),		mve_vqdmladh),
 mToC("vqrdmlsdhx",fe001e01,	3, (RMQ, RMQ, RMQ),		mve_vqdmladh),
 mToC("vqdmlah",   ee000e60,	3, (RMQ, RMQ, RR),		mve_vqdmlah),
 mToC("vqdmlash",  ee001e60,	3, (RMQ, RMQ, RR),		mve_vqdmlah),
 mToC("vqrdmlash", ee001e40,	3, (RMQ, RMQ, RR),		mve_vqdmlah),
 mToC("vqdmullt",  ee301f00,	3, (RMQ, RMQ, RMQRR),		mve_vqdmull),
 mToC("vqdmullb",  ee300f00,	3, (RMQ, RMQ, RMQRR),		mve_vqdmull),
 mCEF(vqmovnt,	  _vqmovnt,	2, (RMQ, RMQ),			mve_vqmovn),
 mCEF(vqmovnb,	  _vqmovnb,	2, (RMQ, RMQ),			mve_vqmovn),
 mCEF(vqmovunt,	  _vqmovunt,	2, (RMQ, RMQ),			mve_vqmovn),
 mCEF(vqmovunb,	  _vqmovunb,	2, (RMQ, RMQ),			mve_vqmovn),

 mCEF(vshrnt,	  _vshrnt,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vshrnb,	  _vshrnb,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vrshrnt,	  _vrshrnt,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vrshrnb,	  _vrshrnb,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vqshrnt,	  _vqrshrnt,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vqshrnb,	  _vqrshrnb,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vqshrunt,	  _vqrshrunt,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vqshrunb,	  _vqrshrunb,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vqrshrnt,	  _vqrshrnt,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vqrshrnb,	  _vqrshrnb,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vqrshrunt,  _vqrshrunt,	3, (RMQ, RMQ, I32z),	mve_vshrn),
 mCEF(vqrshrunb,  _vqrshrunb,	3, (RMQ, RMQ, I32z),	mve_vshrn),

 mToC("vshlc",	    eea00fc0,	   3, (RMQ, RR, I32z),	    mve_vshlc),
 mToC("vshllt",	    ee201e00,	   3, (RMQ, RMQ, I32),	    mve_vshll),
 mToC("vshllb",	    ee200e00,	   3, (RMQ, RMQ, I32),	    mve_vshll),

 toU("dlstp",	_dlstp, 2, (LR, RR),      t_loloop),
 toU("wlstp",	_wlstp, 3, (LR, RR, EXP), t_loloop),
 toU("letp",	_letp,  2, (LR, EXP),	  t_loloop),
 toU("lctp",	_lctp,  0, (),		  t_loloop),

#undef THUMB_VARIANT
#define THUMB_VARIANT & mve_fp_ext
 mToC("vcmul", ee300e00,   4, (RMQ, RMQ, RMQ, EXPi),		  mve_vcmul),
 mToC("vfmas", ee311e40,   3, (RMQ, RMQ, RR),			  mve_vfmas),
 mToC("vmaxnma", ee3f0e81, 2, (RMQ, RMQ),			  mve_vmaxnma_vminnma),
 mToC("vminnma", ee3f1e81, 2, (RMQ, RMQ),			  mve_vmaxnma_vminnma),
 mToC("vmaxnmv", eeee0f00, 2, (RR, RMQ),			  mve_vmaxnmv),
 mToC("vmaxnmav",eeec0f00, 2, (RR, RMQ),			  mve_vmaxnmv),
 mToC("vminnmv", eeee0f80, 2, (RR, RMQ),			  mve_vmaxnmv),
 mToC("vminnmav",eeec0f80, 2, (RR, RMQ),			  mve_vmaxnmv),

#undef  ARM_VARIANT
#define ARM_VARIANT  & fpu_vfp_ext_v1
#undef  THUMB_VARIANT
#define THUMB_VARIANT  & arm_ext_v6t2

 mcCE(fcpyd,	eb00b40, 2, (RVD, RVD),	      vfp_dp_rd_rm),

#undef  ARM_VARIANT
#define ARM_VARIANT  & fpu_vfp_ext_v1xd

 mnCEF(vmla,     _vmla,    3, (RNSDQMQ, oRNSDQMQ, RNSDQ_RNSC_MQ_RR), neon_mac_maybe_scalar),
 mnCEF(vmul,     _vmul,    3, (RNSDQMQ, oRNSDQMQ, RNSDQ_RNSC_MQ_RR), neon_mul),
 MNCE(vmov,   0,	1, (VMOV),	      neon_mov),
 mcCE(fmrs,	e100a10, 2, (RR, RVS),	      vfp_reg_from_sp),
 mcCE(fmsr,	e000a10, 2, (RVS, RR),	      vfp_sp_from_reg),
 mcCE(fcpys,	eb00a40, 2, (RVS, RVS),	      vfp_sp_monadic),

 mCEF(vmullt, _vmullt,	3, (RNSDQMQ, oRNSDQMQ, RNSDQ_RNSC_MQ),	mve_vmull),
 mnCEF(vadd,  _vadd,	3, (RNSDQMQ, oRNSDQMQ, RNSDQMQR),	neon_addsub_if_i),
 mnCEF(vsub,  _vsub,	3, (RNSDQMQ, oRNSDQMQ, RNSDQMQR),	neon_addsub_if_i),

 MNCEF(vabs,  1b10300,	2, (RNSDQMQ, RNSDQMQ),	neon_abs_neg),
 MNCEF(vneg,  1b10380,	2, (RNSDQMQ, RNSDQMQ),	neon_abs_neg),

 mCEF(vmovlt, _vmovlt,	1, (VMOV),		mve_movl),
 mCEF(vmovlb, _vmovlb,	1, (VMOV),		mve_movl),

 mnCE(vcmp,      _vcmp,    3, (RVSD_COND, RSVDMQ_FI0, oRMQRZ),    vfp_nsyn_cmp),
 mnCE(vcmpe,     _vcmpe,   3, (RVSD_COND, RSVDMQ_FI0, oRMQRZ),    vfp_nsyn_cmp),

#undef  ARM_VARIANT
#define ARM_VARIANT  & fpu_vfp_ext_v2

 mcCE(fmsrr,	c400a10, 3, (VRSLST, RR, RR), vfp_sp2_from_reg2),
 mcCE(fmrrs,	c500a10, 3, (RR, RR, VRSLST), vfp_reg2_from_sp2),
 mcCE(fmdrr,	c400b10, 3, (RVD, RR, RR),    vfp_dp_rm_rd_rn),
 mcCE(fmrrd,	c500b10, 3, (RR, RR, RVD),    vfp_dp_rd_rn_rm),

#undef  ARM_VARIANT
#define ARM_VARIANT    & fpu_vfp_ext_armv8xd
 mnUF(vcvta,  _vcvta,  2, (RNSDQMQ, oRNSDQMQ),		neon_cvta),
 mnUF(vcvtp,  _vcvta,  2, (RNSDQMQ, oRNSDQMQ),		neon_cvtp),
 mnUF(vcvtn,  _vcvta,  3, (RNSDQMQ, oRNSDQMQ, oI32z),	neon_cvtn),
 mnUF(vcvtm,  _vcvta,  2, (RNSDQMQ, oRNSDQMQ),		neon_cvtm),
 mnUF(vmaxnm, _vmaxnm, 3, (RNSDQMQ, oRNSDQMQ, RNSDQMQ),	vmaxnm),
 mnUF(vminnm, _vminnm, 3, (RNSDQMQ, oRNSDQMQ, RNSDQMQ),	vmaxnm),

#undef	ARM_VARIANT
#define ARM_VARIANT & fpu_neon_ext_v1
 mnUF(vabd,      _vabd,		  3, (RNDQMQ, oRNDQMQ, RNDQMQ), neon_dyadic_if_su),
 mnUF(vabdl,     _vabdl,	  3, (RNQMQ, RNDMQ, RNDMQ),   neon_dyadic_long),
 mnUF(vaddl,     _vaddl,	  3, (RNSDQMQ, oRNSDMQ, RNSDMQR),  neon_dyadic_long),
 mnUF(vsubl,     _vsubl,	  3, (RNSDQMQ, oRNSDMQ, RNSDMQR),  neon_dyadic_long),
 mnUF(vand,      _vand,		  3, (RNDQMQ, oRNDQMQ, RNDQMQ_Ibig), neon_logic),
 mnUF(vbic,      _vbic,		  3, (RNDQMQ, oRNDQMQ, RNDQMQ_Ibig), neon_logic),
 mnUF(vorr,      _vorr,		  3, (RNDQMQ, oRNDQMQ, RNDQMQ_Ibig), neon_logic),
 mnUF(vorn,      _vorn,		  3, (RNDQMQ, oRNDQMQ, RNDQMQ_Ibig), neon_logic),
 mnUF(veor,      _veor,		  3, (RNDQMQ, oRNDQMQ, RNDQMQ),      neon_logic),
 MNUF(vcls,      1b00400,	  2, (RNDQMQ, RNDQMQ),		     neon_cls),
 MNUF(vclz,      1b00480,	  2, (RNDQMQ, RNDQMQ),		     neon_clz),
 mnCE(vdup,      _vdup,		  2, (RNDQMQ, RR_RNSC),		     neon_dup),
 MNUF(vhadd,     00000000,	  3, (RNDQMQ, oRNDQMQ, RNDQMQR),  neon_dyadic_i_su),
 MNUF(vrhadd,    00000100,	  3, (RNDQMQ, oRNDQMQ, RNDQMQ),	  neon_dyadic_i_su),
 MNUF(vhsub,     00000200,	  3, (RNDQMQ, oRNDQMQ, RNDQMQR),  neon_dyadic_i_su),
 mnUF(vmin,      _vmin,    3, (RNDQMQ, oRNDQMQ, RNDQMQ), neon_dyadic_if_su),
 mnUF(vmax,      _vmax,    3, (RNDQMQ, oRNDQMQ, RNDQMQ), neon_dyadic_if_su),
 MNUF(vqadd,     0000010,  3, (RNDQMQ, oRNDQMQ, RNDQMQR), neon_dyadic_i64_su),
 MNUF(vqsub,     0000210,  3, (RNDQMQ, oRNDQMQ, RNDQMQR), neon_dyadic_i64_su),
 mnUF(vmvn,      _vmvn,    2, (RNDQMQ, RNDQMQ_Ibig), neon_mvn),
 MNUF(vqabs,     1b00700,  2, (RNDQMQ, RNDQMQ),     neon_sat_abs_neg),
 MNUF(vqneg,     1b00780,  2, (RNDQMQ, RNDQMQ),     neon_sat_abs_neg),
 mnUF(vqrdmlah,  _vqrdmlah,3, (RNDQMQ, oRNDQMQ, RNDQ_RNSC_RR), neon_qrdmlah),
 mnUF(vqdmulh,   _vqdmulh, 3, (RNDQMQ, oRNDQMQ, RNDQMQ_RNSC_RR), neon_qdmulh),
 mnUF(vqrdmulh,  _vqrdmulh,3, (RNDQMQ, oRNDQMQ, RNDQMQ_RNSC_RR), neon_qdmulh),
 MNUF(vqrshl,    0000510,  3, (RNDQMQ, oRNDQMQ, RNDQMQR), neon_rshl),
 MNUF(vrshl,     0000500,  3, (RNDQMQ, oRNDQMQ, RNDQMQR), neon_rshl),
 MNUF(vshr,      0800010,  3, (RNDQMQ, oRNDQMQ, I64z), neon_rshift_round_imm),
 MNUF(vrshr,     0800210,  3, (RNDQMQ, oRNDQMQ, I64z), neon_rshift_round_imm),
 MNUF(vsli,      1800510,  3, (RNDQMQ, oRNDQMQ, I63),  neon_sli),
 MNUF(vsri,      1800410,  3, (RNDQMQ, oRNDQMQ, I64z), neon_sri),
 MNUF(vrev64,    1b00000,  2, (RNDQMQ, RNDQMQ),     neon_rev),
 MNUF(vrev32,    1b00080,  2, (RNDQMQ, RNDQMQ),     neon_rev),
 MNUF(vrev16,    1b00100,  2, (RNDQMQ, RNDQMQ),     neon_rev),
 mnUF(vshl,	 _vshl,    3, (RNDQMQ, oRNDQMQ, RNDQMQ_I63b_RR), neon_shl),
 mnUF(vqshl,     _vqshl,   3, (RNDQMQ, oRNDQMQ, RNDQMQ_I63b_RR), neon_qshl),
 MNUF(vqshlu,    1800610,  3, (RNDQMQ, oRNDQMQ, I63),		 neon_qshlu_imm),

#undef	ARM_VARIANT
#define ARM_VARIANT & arm_ext_v8_3
#undef	THUMB_VARIANT
#define	THUMB_VARIANT & arm_ext_v6t2_v8m
 MNUF (vcadd, 0, 4, (RNDQMQ, RNDQMQ, RNDQMQ, EXPi), vcadd),
 MNUF (vcmla, 0, 4, (RNDQMQ, RNDQMQ, RNDQMQ_RNSC, EXPi), vcmla),

#undef	ARM_VARIANT
#define ARM_VARIANT &arm_ext_bf16
#undef	THUMB_VARIANT
#define	THUMB_VARIANT &arm_ext_bf16
 TUF ("vdot", c000d00, fc000d00, 3, (RNDQ, RNDQ, RNDQ_RNSC), vdot, vdot),
 TUF ("vmmla", c000c40, fc000c40, 3, (RNQ, RNQ, RNQ), vmmla, vmmla),
 TUF ("vfmab", c300810, fc300810, 3, (RNDQ, RNDQ, RNDQ_RNSC), bfloat_vfma, bfloat_vfma),

#undef	ARM_VARIANT
#define ARM_VARIANT &arm_ext_i8mm
#undef	THUMB_VARIANT
#define	THUMB_VARIANT &arm_ext_i8mm
 TUF ("vsmmla", c200c40, fc200c40, 3, (RNQ, RNQ, RNQ), vsmmla, vsmmla),
 TUF ("vummla", c200c50, fc200c50, 3, (RNQ, RNQ, RNQ), vummla, vummla),
 TUF ("vusmmla", ca00c40, fca00c40, 3, (RNQ, RNQ, RNQ), vsmmla, vsmmla),
 TUF ("vusdot", c800d00, fc800d00, 3, (RNDQ, RNDQ, RNDQ_RNSC), vusdot, vusdot),
 TUF ("vsudot", c800d10, fc800d10, 3, (RNDQ, RNDQ, RNSC), vsudot, vsudot),

#undef	ARM_VARIANT
#undef	THUMB_VARIANT
#define	THUMB_VARIANT &arm_ext_cde
 ToC ("cx1", ee000000, 3, (RCP, APSR_RR, I8191), cx1),
 ToC ("cx1a", fe000000, 3, (RCP, APSR_RR, I8191), cx1a),
 ToC ("cx1d", ee000040, 4, (RCP, RR, APSR_RR, I8191), cx1d),
 ToC ("cx1da", fe000040, 4, (RCP, RR, APSR_RR, I8191), cx1da),

 ToC ("cx2", ee400000, 4, (RCP, APSR_RR, APSR_RR, I511), cx2),
 ToC ("cx2a", fe400000, 4, (RCP, APSR_RR, APSR_RR, I511), cx2a),
 ToC ("cx2d", ee400040, 5, (RCP, RR, APSR_RR, APSR_RR, I511), cx2d),
 ToC ("cx2da", fe400040, 5, (RCP, RR, APSR_RR, APSR_RR, I511), cx2da),

 ToC ("cx3", ee800000, 5, (RCP, APSR_RR, APSR_RR, APSR_RR, I63), cx3),
 ToC ("cx3a", fe800000, 5, (RCP, APSR_RR, APSR_RR, APSR_RR, I63), cx3a),
 ToC ("cx3d", ee800040, 6, (RCP, RR, APSR_RR, APSR_RR, APSR_RR, I63), cx3d),
 ToC ("cx3da", fe800040, 6, (RCP, RR, APSR_RR, APSR_RR, APSR_RR, I63), cx3da),

 mToC ("vcx1", ec200000, 3, (RCP, RNSDMQ, I4095), vcx1),
 mToC ("vcx1a", fc200000, 3, (RCP, RNSDMQ, I4095), vcx1),

 mToC ("vcx2", ec300000, 4, (RCP, RNSDMQ, RNSDMQ, I127), vcx2),
 mToC ("vcx2a", fc300000, 4, (RCP, RNSDMQ, RNSDMQ, I127), vcx2),

 mToC ("vcx3", ec800000, 5, (RCP, RNSDMQ, RNSDMQ, RNSDMQ, I15), vcx3),
 mToC ("vcx3a", fc800000, 5, (RCP, RNSDMQ, RNSDMQ, RNSDMQ, I15), vcx3),
};

#undef ARM_VARIANT
#undef THUMB_VARIANT
#undef TCE
#undef TUE
#undef TUF
#undef TCC
#undef cCE
#undef cCL
#undef C3E
#undef C3
#undef CE
#undef CM
#undef CL
#undef UE
#undef UF
#undef UT
#undef NUF
#undef nUF
#undef NCE
#undef nCE
#undef OPS0
#undef OPS1
#undef OPS2
#undef OPS3
#undef OPS4
#undef OPS5
#undef OPS6
#undef do_0
#undef ToC
#undef toC
#undef ToU
#undef toU

/* MD interface: bits in the object file.  */

/* Turn an integer of n bytes (in val) into a stream of bytes appropriate
   for use in the a.out file, and stores them in the array pointed to by buf.
   This knows about the endian-ness of the target machine and does
   THE RIGHT THING, whatever it is.  Possible values for n are 1 (byte)
   2 (short) and 4 (long)  Floating numbers are put out as a series of
   LITTLENUMS (shorts, here at least).	*/

void
md_number_to_chars (char * buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

static valueT
md_chars_to_number (char * buf, int n)
{
  valueT result = 0;
  unsigned char * where = (unsigned char *) buf;

  if (target_big_endian)
    {
      while (n--)
	{
	  result <<= 8;
	  result |= (*where++ & 255);
	}
    }
  else
    {
      while (n--)
	{
	  result <<= 8;
	  result |= (where[n] & 255);
	}
    }

  return result;
}

/* MD interface: Sections.  */

/* Calculate the maximum variable size (i.e., excluding fr_fix)
   that an rs_machine_dependent frag may reach.  */

unsigned int
arm_frag_max_var (fragS *fragp)
{
  /* We only use rs_machine_dependent for variable-size Thumb instructions,
     which are either THUMB_SIZE (2) or INSN_SIZE (4).

     Note that we generate relaxable instructions even for cases that don't
     really need it, like an immediate that's a trivial constant.  So we're
     overestimating the instruction size for some of those cases.  Rather
     than putting more intelligence here, it would probably be better to
     avoid generating a relaxation frag in the first place when it can be
     determined up front that a short instruction will suffice.  */

  gas_assert (fragp->fr_type == rs_machine_dependent);
  return INSN_SIZE;
}

/* Estimate the size of a frag before relaxing.  Assume everything fits in
   2 bytes.  */

int
md_estimate_size_before_relax (fragS * fragp,
			       segT    segtype ATTRIBUTE_UNUSED)
{
  fragp->fr_var = 2;
  return 2;
}

/* Convert a machine dependent frag.  */

void
md_convert_frag (bfd *abfd, segT asec ATTRIBUTE_UNUSED, fragS *fragp)
{
  unsigned long insn;
  unsigned long old_op;
  char *buf;
  expressionS exp;
  fixS *fixp;
  int reloc_type;
  int pc_rel;
  int opcode;

  buf = fragp->fr_literal + fragp->fr_fix;

  old_op = bfd_get_16(abfd, buf);
  if (fragp->fr_symbol)
    {
      exp.X_op = O_symbol;
      exp.X_add_symbol = fragp->fr_symbol;
    }
  else
    {
      exp.X_op = O_constant;
    }
  exp.X_add_number = fragp->fr_offset;
  opcode = fragp->fr_subtype;
  switch (opcode)
    {
    case T_MNEM_ldr_pc:
    case T_MNEM_ldr_pc2:
    case T_MNEM_ldr_sp:
    case T_MNEM_str_sp:
    case T_MNEM_ldr:
    case T_MNEM_ldrb:
    case T_MNEM_ldrh:
    case T_MNEM_str:
    case T_MNEM_strb:
    case T_MNEM_strh:
      if (fragp->fr_var == 4)
	{
	  insn = THUMB_OP32 (opcode);
	  if ((old_op >> 12) == 4 || (old_op >> 12) == 9)
	    {
	      insn |= (old_op & 0x700) << 4;
	    }
	  else
	    {
	      insn |= (old_op & 7) << 12;
	      insn |= (old_op & 0x38) << 13;
	    }
	  insn |= 0x00000c00;
	  put_thumb32_insn (buf, insn);
	  reloc_type = BFD_RELOC_ARM_T32_OFFSET_IMM;
	}
      else
	{
	  reloc_type = BFD_RELOC_ARM_THUMB_OFFSET;
	}
      pc_rel = (opcode == T_MNEM_ldr_pc2);
      break;
    case T_MNEM_adr:
      /* Thumb bits should be set in the frag handling so we process them
	 after all symbols have been seen.  PR gas/25235.  */
      if (exp.X_op == O_symbol
	  && exp.X_add_symbol != NULL
	  && S_IS_DEFINED (exp.X_add_symbol)
	  && THUMB_IS_FUNC (exp.X_add_symbol))
	exp.X_add_number |= 1;

      if (fragp->fr_var == 4)
	{
	  insn = THUMB_OP32 (opcode);
	  insn |= (old_op & 0xf0) << 4;
	  put_thumb32_insn (buf, insn);
	  reloc_type = BFD_RELOC_ARM_T32_ADD_PC12;
	}
      else
	{
	  reloc_type = BFD_RELOC_ARM_THUMB_ADD;
	  exp.X_add_number -= 4;
	}
      pc_rel = 1;
      break;
    case T_MNEM_mov:
    case T_MNEM_movs:
    case T_MNEM_cmp:
    case T_MNEM_cmn:
      if (fragp->fr_var == 4)
	{
	  int r0off = (opcode == T_MNEM_mov
		       || opcode == T_MNEM_movs) ? 0 : 8;
	  insn = THUMB_OP32 (opcode);
	  insn = (insn & 0xe1ffffff) | 0x10000000;
	  insn |= (old_op & 0x700) << r0off;
	  put_thumb32_insn (buf, insn);
	  reloc_type = BFD_RELOC_ARM_T32_IMMEDIATE;
	}
      else
	{
	  reloc_type = BFD_RELOC_ARM_THUMB_IMM;
	}
      pc_rel = 0;
      break;
    case T_MNEM_b:
      if (fragp->fr_var == 4)
	{
	  insn = THUMB_OP32(opcode);
	  put_thumb32_insn (buf, insn);
	  reloc_type = BFD_RELOC_THUMB_PCREL_BRANCH25;
	}
      else
	reloc_type = BFD_RELOC_THUMB_PCREL_BRANCH12;
      pc_rel = 1;
      break;
    case T_MNEM_bcond:
      if (fragp->fr_var == 4)
	{
	  insn = THUMB_OP32(opcode);
	  insn |= (old_op & 0xf00) << 14;
	  put_thumb32_insn (buf, insn);
	  reloc_type = BFD_RELOC_THUMB_PCREL_BRANCH20;
	}
      else
	reloc_type = BFD_RELOC_THUMB_PCREL_BRANCH9;
      pc_rel = 1;
      break;
    case T_MNEM_add_sp:
    case T_MNEM_add_pc:
    case T_MNEM_inc_sp:
    case T_MNEM_dec_sp:
      if (fragp->fr_var == 4)
	{
	  /* ??? Choose between add and addw.  */
	  insn = THUMB_OP32 (opcode);
	  insn |= (old_op & 0xf0) << 4;
	  put_thumb32_insn (buf, insn);
	  if (opcode == T_MNEM_add_pc)
	    reloc_type = BFD_RELOC_ARM_T32_IMM12;
	  else
	    reloc_type = BFD_RELOC_ARM_T32_ADD_IMM;
	}
      else
	reloc_type = BFD_RELOC_ARM_THUMB_ADD;
      pc_rel = 0;
      break;

    case T_MNEM_addi:
    case T_MNEM_addis:
    case T_MNEM_subi:
    case T_MNEM_subis:
      if (fragp->fr_var == 4)
	{
	  insn = THUMB_OP32 (opcode);
	  insn |= (old_op & 0xf0) << 4;
	  insn |= (old_op & 0xf) << 16;
	  put_thumb32_insn (buf, insn);
	  if (insn & (1 << 20))
	    reloc_type = BFD_RELOC_ARM_T32_ADD_IMM;
	  else
	    reloc_type = BFD_RELOC_ARM_T32_IMMEDIATE;
	}
      else
	reloc_type = BFD_RELOC_ARM_THUMB_ADD;
      pc_rel = 0;
      break;
    default:
      abort ();
    }
  fixp = fix_new_exp (fragp, fragp->fr_fix, fragp->fr_var, &exp, pc_rel,
		      (enum bfd_reloc_code_real) reloc_type);
  fixp->fx_file = fragp->fr_file;
  fixp->fx_line = fragp->fr_line;
  fragp->fr_fix += fragp->fr_var;

  /* Set whether we use thumb-2 ISA based on final relaxation results.  */
  if (thumb_mode && fragp->fr_var == 4 && no_cpu_selected ()
      && !ARM_CPU_HAS_FEATURE (thumb_arch_used, arm_arch_t2))
    ARM_MERGE_FEATURE_SETS (arm_arch_used, thumb_arch_used, arm_ext_v6t2);
}

/* Return the size of a relaxable immediate operand instruction.
   SHIFT and SIZE specify the form of the allowable immediate.  */
static int
relax_immediate (fragS *fragp, int size, int shift)
{
  offsetT offset;
  offsetT mask;
  offsetT low;

  /* ??? Should be able to do better than this.  */
  if (fragp->fr_symbol)
    return 4;

  low = (1 << shift) - 1;
  mask = (1 << (shift + size)) - (1 << shift);
  offset = fragp->fr_offset;
  /* Force misaligned offsets to 32-bit variant.  */
  if (offset & low)
    return 4;
  if (offset & ~mask)
    return 4;
  return 2;
}

/* Get the address of a symbol during relaxation.  */
static addressT
relaxed_symbol_addr (fragS *fragp, long stretch)
{
  fragS *sym_frag;
  addressT addr;
  symbolS *sym;

  sym = fragp->fr_symbol;
  sym_frag = symbol_get_frag (sym);
  know (S_GET_SEGMENT (sym) != absolute_section
	|| sym_frag == &zero_address_frag);
  addr = S_GET_VALUE (sym) + fragp->fr_offset;

  /* If frag has yet to be reached on this pass, assume it will
     move by STRETCH just as we did.  If this is not so, it will
     be because some frag between grows, and that will force
     another pass.  */

  if (stretch != 0
      && sym_frag->relax_marker != fragp->relax_marker)
    {
      fragS *f;

      /* Adjust stretch for any alignment frag.  Note that if have
	 been expanding the earlier code, the symbol may be
	 defined in what appears to be an earlier frag.  FIXME:
	 This doesn't handle the fr_subtype field, which specifies
	 a maximum number of bytes to skip when doing an
	 alignment.  */
      for (f = fragp; f != NULL && f != sym_frag; f = f->fr_next)
	{
	  if (f->fr_type == rs_align || f->fr_type == rs_align_code)
	    {
	      if (stretch < 0)
		stretch = - ((- stretch)
			     & ~ ((1 << (int) f->fr_offset) - 1));
	      else
		stretch &= ~ ((1 << (int) f->fr_offset) - 1);
	      if (stretch == 0)
		break;
	    }
	}
      if (f != NULL)
	addr += stretch;
    }

  return addr;
}

/* Return the size of a relaxable adr pseudo-instruction or PC-relative
   load.  */
static int
relax_adr (fragS *fragp, asection *sec, long stretch)
{
  addressT addr;
  offsetT val;

  /* Assume worst case for symbols not known to be in the same section.  */
  if (fragp->fr_symbol == NULL
      || !S_IS_DEFINED (fragp->fr_symbol)
      || sec != S_GET_SEGMENT (fragp->fr_symbol)
      || S_IS_WEAK (fragp->fr_symbol)
      || THUMB_IS_FUNC (fragp->fr_symbol))
    return 4;

  val = relaxed_symbol_addr (fragp, stretch);
  addr = fragp->fr_address + fragp->fr_fix;
  addr = (addr + 4) & ~3;
  /* Force misaligned targets to 32-bit variant.  */
  if (val & 3)
    return 4;
  val -= addr;
  if (val < 0 || val > 1020)
    return 4;
  return 2;
}

/* Return the size of a relaxable add/sub immediate instruction.  */
static int
relax_addsub (fragS *fragp, asection *sec)
{
  char *buf;
  int op;

  buf = fragp->fr_literal + fragp->fr_fix;
  op = bfd_get_16(sec->owner, buf);
  if ((op & 0xf) == ((op >> 4) & 0xf))
    return relax_immediate (fragp, 8, 0);
  else
    return relax_immediate (fragp, 3, 0);
}

/* Return TRUE iff the definition of symbol S could be pre-empted
   (overridden) at link or load time.  */
static bool
symbol_preemptible (symbolS *s)
{
  /* Weak symbols can always be pre-empted.  */
  if (S_IS_WEAK (s))
    return true;

  /* Non-global symbols cannot be pre-empted. */
  if (! S_IS_EXTERNAL (s))
    return false;

#ifdef OBJ_ELF
  /* In ELF, a global symbol can be marked protected, or private.  In that
     case it can't be pre-empted (other definitions in the same link unit
     would violate the ODR).  */
  if (ELF_ST_VISIBILITY (S_GET_OTHER (s)) > STV_DEFAULT)
    return false;
#endif

  /* Other global symbols might be pre-empted.  */
  return true;
}

/* Return the size of a relaxable branch instruction.  BITS is the
   size of the offset field in the narrow instruction.  */

static int
relax_branch (fragS *fragp, asection *sec, int bits, long stretch)
{
  addressT addr;
  offsetT val;
  offsetT limit;

  /* Assume worst case for symbols not known to be in the same section.  */
  if (!S_IS_DEFINED (fragp->fr_symbol)
      || sec != S_GET_SEGMENT (fragp->fr_symbol)
      || S_IS_WEAK (fragp->fr_symbol))
    return 4;

#ifdef OBJ_ELF
  /* A branch to a function in ARM state will require interworking.  */
  if (S_IS_DEFINED (fragp->fr_symbol)
      && ARM_IS_FUNC (fragp->fr_symbol))
      return 4;
#endif

  if (symbol_preemptible (fragp->fr_symbol))
    return 4;

  val = relaxed_symbol_addr (fragp, stretch);
  addr = fragp->fr_address + fragp->fr_fix + 4;
  val -= addr;

  /* Offset is a signed value *2 */
  limit = 1 << bits;
  if (val >= limit || val < -limit)
    return 4;
  return 2;
}


/* Relax a machine dependent frag.  This returns the amount by which
   the current size of the frag should change.  */

int
arm_relax_frag (asection *sec, fragS *fragp, long stretch)
{
  int oldsize;
  int newsize;

  oldsize = fragp->fr_var;
  switch (fragp->fr_subtype)
    {
    case T_MNEM_ldr_pc2:
      newsize = relax_adr (fragp, sec, stretch);
      break;
    case T_MNEM_ldr_pc:
    case T_MNEM_ldr_sp:
    case T_MNEM_str_sp:
      newsize = relax_immediate (fragp, 8, 2);
      break;
    case T_MNEM_ldr:
    case T_MNEM_str:
      newsize = relax_immediate (fragp, 5, 2);
      break;
    case T_MNEM_ldrh:
    case T_MNEM_strh:
      newsize = relax_immediate (fragp, 5, 1);
      break;
    case T_MNEM_ldrb:
    case T_MNEM_strb:
      newsize = relax_immediate (fragp, 5, 0);
      break;
    case T_MNEM_adr:
      newsize = relax_adr (fragp, sec, stretch);
      break;
    case T_MNEM_mov:
    case T_MNEM_movs:
    case T_MNEM_cmp:
    case T_MNEM_cmn:
      newsize = relax_immediate (fragp, 8, 0);
      break;
    case T_MNEM_b:
      newsize = relax_branch (fragp, sec, 11, stretch);
      break;
    case T_MNEM_bcond:
      newsize = relax_branch (fragp, sec, 8, stretch);
      break;
    case T_MNEM_add_sp:
    case T_MNEM_add_pc:
      newsize = relax_immediate (fragp, 8, 2);
      break;
    case T_MNEM_inc_sp:
    case T_MNEM_dec_sp:
      newsize = relax_immediate (fragp, 7, 2);
      break;
    case T_MNEM_addi:
    case T_MNEM_addis:
    case T_MNEM_subi:
    case T_MNEM_subis:
      newsize = relax_addsub (fragp, sec);
      break;
    default:
      abort ();
    }

  fragp->fr_var = newsize;
  /* Freeze wide instructions that are at or before the same location as
     in the previous pass.  This avoids infinite loops.
     Don't freeze them unconditionally because targets may be artificially
     misaligned by the expansion of preceding frags.  */
  if (stretch <= 0 && newsize > 2)
    {
      md_convert_frag (sec->owner, sec, fragp);
      frag_wane (fragp);
    }

  return newsize - oldsize;
}

/* Round up a section size to the appropriate boundary.	 */

valueT
md_section_align (segT	 segment ATTRIBUTE_UNUSED,
		  valueT size)
{
  return size;
}

/* This is called from HANDLE_ALIGN in write.c.	 Fill in the contents
   of an rs_align_code fragment.  */

void
arm_handle_align (fragS * fragP)
{
  static unsigned char const arm_noop[2][2][4] =
    {
      {  /* ARMv1 */
	{0x00, 0x00, 0xa0, 0xe1},  /* LE */
	{0xe1, 0xa0, 0x00, 0x00},  /* BE */
      },
      {  /* ARMv6k */
	{0x00, 0xf0, 0x20, 0xe3},  /* LE */
	{0xe3, 0x20, 0xf0, 0x00},  /* BE */
      },
    };
  static unsigned char const thumb_noop[2][2][2] =
    {
      {  /* Thumb-1 */
	{0xc0, 0x46},  /* LE */
	{0x46, 0xc0},  /* BE */
      },
      {  /* Thumb-2 */
	{0x00, 0xbf},  /* LE */
	{0xbf, 0x00}   /* BE */
      }
    };
  static unsigned char const wide_thumb_noop[2][4] =
    {  /* Wide Thumb-2 */
      {0xaf, 0xf3, 0x00, 0x80},  /* LE */
      {0xf3, 0xaf, 0x80, 0x00},  /* BE */
    };

  unsigned bytes, fix, noop_size;
  char * p;
  const unsigned char * noop;
  const unsigned char *narrow_noop = NULL;
#ifdef OBJ_ELF
  enum mstate state;
#endif

  if (fragP->fr_type != rs_align_code)
    return;

  bytes = fragP->fr_next->fr_address - fragP->fr_address - fragP->fr_fix;
  p = fragP->fr_literal + fragP->fr_fix;
  fix = 0;

  if (bytes > MAX_MEM_FOR_RS_ALIGN_CODE)
    bytes &= MAX_MEM_FOR_RS_ALIGN_CODE;

  gas_assert ((fragP->tc_frag_data.thumb_mode & MODE_RECORDED) != 0);

  if (fragP->tc_frag_data.thumb_mode & (~ MODE_RECORDED))
    {
      if (ARM_CPU_HAS_FEATURE (selected_cpu_name[0]
			       ? selected_cpu : arm_arch_none, arm_ext_v6t2))
	{
	  narrow_noop = thumb_noop[1][target_big_endian];
	  noop = wide_thumb_noop[target_big_endian];
	}
      else
	noop = thumb_noop[0][target_big_endian];
      noop_size = 2;
#ifdef OBJ_ELF
      state = MAP_THUMB;
#endif
    }
  else
    {
      noop = arm_noop[ARM_CPU_HAS_FEATURE (selected_cpu_name[0]
					   ? selected_cpu : arm_arch_none,
					   arm_ext_v6k) != 0]
		     [target_big_endian];
      noop_size = 4;
#ifdef OBJ_ELF
      state = MAP_ARM;
#endif
    }

  fragP->fr_var = noop_size;

  if (bytes & (noop_size - 1))
    {
      fix = bytes & (noop_size - 1);
#ifdef OBJ_ELF
      insert_data_mapping_symbol (state, fragP->fr_fix, fragP, fix);
#endif
      memset (p, 0, fix);
      p += fix;
      bytes -= fix;
    }

  if (narrow_noop)
    {
      if (bytes & noop_size)
	{
	  /* Insert a narrow noop.  */
	  memcpy (p, narrow_noop, noop_size);
	  p += noop_size;
	  bytes -= noop_size;
	  fix += noop_size;
	}

      /* Use wide noops for the remainder */
      noop_size = 4;
    }

  while (bytes >= noop_size)
    {
      memcpy (p, noop, noop_size);
      p += noop_size;
      bytes -= noop_size;
      fix += noop_size;
    }

  fragP->fr_fix += fix;
}

/* Called from md_do_align.  Used to create an alignment
   frag in a code section.  */

void
arm_frag_align_code (int n, int max)
{
  char * p;

  /* We assume that there will never be a requirement
     to support alignments greater than MAX_MEM_FOR_RS_ALIGN_CODE bytes.  */
  if (max > MAX_MEM_FOR_RS_ALIGN_CODE)
    {
      char err_msg[128];

      sprintf (err_msg,
	_("alignments greater than %d bytes not supported in .text sections."),
	MAX_MEM_FOR_RS_ALIGN_CODE + 1);
      as_fatal ("%s", err_msg);
    }

  p = frag_var (rs_align_code,
		MAX_MEM_FOR_RS_ALIGN_CODE,
		1,
		(relax_substateT) max,
		(symbolS *) NULL,
		(offsetT) n,
		(char *) NULL);
  *p = 0;
}

/* Perform target specific initialisation of a frag.
   Note - despite the name this initialisation is not done when the frag
   is created, but only when its type is assigned.  A frag can be created
   and used a long time before its type is set, so beware of assuming that
   this initialisation is performed first.  */

#ifndef OBJ_ELF
void
arm_init_frag (fragS * fragP, int max_chars ATTRIBUTE_UNUSED)
{
  /* Record whether this frag is in an ARM or a THUMB area.  */
  fragP->tc_frag_data.thumb_mode = thumb_mode | MODE_RECORDED;
}

#else /* OBJ_ELF is defined.  */
void
arm_init_frag (fragS * fragP, int max_chars)
{
  bool frag_thumb_mode;

  /* If the current ARM vs THUMB mode has not already
     been recorded into this frag then do so now.  */
  if ((fragP->tc_frag_data.thumb_mode & MODE_RECORDED) == 0)
    fragP->tc_frag_data.thumb_mode = thumb_mode | MODE_RECORDED;

  /* PR 21809: Do not set a mapping state for debug sections
     - it just confuses other tools.  */
  if (bfd_section_flags (now_seg) & SEC_DEBUGGING)
    return;

  frag_thumb_mode = fragP->tc_frag_data.thumb_mode ^ MODE_RECORDED;

  /* Record a mapping symbol for alignment frags.  We will delete this
     later if the alignment ends up empty.  */
  switch (fragP->fr_type)
    {
    case rs_align:
    case rs_align_test:
    case rs_fill:
      mapping_state_2 (MAP_DATA, max_chars);
      break;
    case rs_align_code:
      mapping_state_2 (frag_thumb_mode ? MAP_THUMB : MAP_ARM, max_chars);
      break;
    default:
      break;
    }
}

/* When we change sections we need to issue a new mapping symbol.  */

void
arm_elf_change_section (void)
{
  /* Link an unlinked unwind index table section to the .text section.	*/
  if (elf_section_type (now_seg) == SHT_ARM_EXIDX
      && elf_linked_to_section (now_seg) == NULL)
    elf_linked_to_section (now_seg) = text_section;
}

int
arm_elf_section_type (const char * str, size_t len)
{
  if (len == 5 && startswith (str, "exidx"))
    return SHT_ARM_EXIDX;

  return -1;
}

/* Code to deal with unwinding tables.	*/

static void add_unwind_adjustsp (offsetT);

/* Generate any deferred unwind frame offset.  */

static void
flush_pending_unwind (void)
{
  offsetT offset;

  offset = unwind.pending_offset;
  unwind.pending_offset = 0;
  if (offset != 0)
    add_unwind_adjustsp (offset);
}

/* Add an opcode to this list for this function.  Two-byte opcodes should
   be passed as op[0] << 8 | op[1].  The list of opcodes is built in reverse
   order.  */

static void
add_unwind_opcode (valueT op, int length)
{
  /* Add any deferred stack adjustment.	 */
  if (unwind.pending_offset)
    flush_pending_unwind ();

  unwind.sp_restored = 0;

  if (unwind.opcode_count + length > unwind.opcode_alloc)
    {
      unwind.opcode_alloc += ARM_OPCODE_CHUNK_SIZE;
      if (unwind.opcodes)
	unwind.opcodes = XRESIZEVEC (unsigned char, unwind.opcodes,
				     unwind.opcode_alloc);
      else
	unwind.opcodes = XNEWVEC (unsigned char, unwind.opcode_alloc);
    }
  while (length > 0)
    {
      length--;
      unwind.opcodes[unwind.opcode_count] = op & 0xff;
      op >>= 8;
      unwind.opcode_count++;
    }
}

/* Add unwind opcodes to adjust the stack pointer.  */

static void
add_unwind_adjustsp (offsetT offset)
{
  valueT op;

  if (offset > 0x200)
    {
      /* We need at most 5 bytes to hold a 32-bit value in a uleb128.  */
      char bytes[5];
      int n;
      valueT o;

      /* Long form: 0xb2, uleb128.  */
      /* This might not fit in a word so add the individual bytes,
	 remembering the list is built in reverse order.  */
      o = (valueT) ((offset - 0x204) >> 2);
      if (o == 0)
	add_unwind_opcode (0, 1);

      /* Calculate the uleb128 encoding of the offset.	*/
      n = 0;
      while (o)
	{
	  bytes[n] = o & 0x7f;
	  o >>= 7;
	  if (o)
	    bytes[n] |= 0x80;
	  n++;
	}
      /* Add the insn.	*/
      for (; n; n--)
	add_unwind_opcode (bytes[n - 1], 1);
      add_unwind_opcode (0xb2, 1);
    }
  else if (offset > 0x100)
    {
      /* Two short opcodes.  */
      add_unwind_opcode (0x3f, 1);
      op = (offset - 0x104) >> 2;
      add_unwind_opcode (op, 1);
    }
  else if (offset > 0)
    {
      /* Short opcode.	*/
      op = (offset - 4) >> 2;
      add_unwind_opcode (op, 1);
    }
  else if (offset < 0)
    {
      offset = -offset;
      while (offset > 0x100)
	{
	  add_unwind_opcode (0x7f, 1);
	  offset -= 0x100;
	}
      op = ((offset - 4) >> 2) | 0x40;
      add_unwind_opcode (op, 1);
    }
}

/* Finish the list of unwind opcodes for this function.	 */

static void
finish_unwind_opcodes (void)
{
  valueT op;

  if (unwind.fp_used)
    {
      /* Adjust sp as necessary.  */
      unwind.pending_offset += unwind.fp_offset - unwind.frame_size;
      flush_pending_unwind ();

      /* After restoring sp from the frame pointer.  */
      op = 0x90 | unwind.fp_reg;
      add_unwind_opcode (op, 1);
    }
  else
    flush_pending_unwind ();
}


/* Start an exception table entry.  If idx is nonzero this is an index table
   entry.  */

static void
start_unwind_section (const segT text_seg, int idx)
{
  const char * text_name;
  const char * prefix;
  const char * prefix_once;
  struct elf_section_match match;
  char * sec_name;
  int type;
  int flags;
  int linkonce;

  if (idx)
    {
      prefix = ELF_STRING_ARM_unwind;
      prefix_once = ELF_STRING_ARM_unwind_once;
      type = SHT_ARM_EXIDX;
    }
  else
    {
      prefix = ELF_STRING_ARM_unwind_info;
      prefix_once = ELF_STRING_ARM_unwind_info_once;
      type = SHT_PROGBITS;
    }

  text_name = segment_name (text_seg);
  if (streq (text_name, ".text"))
    text_name = "";

  if (startswith (text_name, ".gnu.linkonce.t."))
    {
      prefix = prefix_once;
      text_name += strlen (".gnu.linkonce.t.");
    }

  sec_name = concat (prefix, text_name, (char *) NULL);

  flags = SHF_ALLOC;
  linkonce = 0;
  memset (&match, 0, sizeof (match));

  /* Handle COMDAT group.  */
  if (prefix != prefix_once && (text_seg->flags & SEC_LINK_ONCE) != 0)
    {
      match.group_name = elf_group_name (text_seg);
      if (match.group_name == NULL)
	{
	  as_bad (_("Group section `%s' has no group signature"),
		  segment_name (text_seg));
	  ignore_rest_of_line ();
	  return;
	}
      flags |= SHF_GROUP;
      linkonce = 1;
    }

  obj_elf_change_section (sec_name, type, flags, 0, &match,
			  linkonce, 0);

  /* Set the section link for index tables.  */
  if (idx)
    elf_linked_to_section (now_seg) = text_seg;
}


/* Start an unwind table entry.	 HAVE_DATA is nonzero if we have additional
   personality routine data.  Returns zero, or the index table value for
   an inline entry.  */

static valueT
create_unwind_entry (int have_data)
{
  int size;
  addressT where;
  char *ptr;
  /* The current word of data.	*/
  valueT data;
  /* The number of bytes left in this word.  */
  int n;

  finish_unwind_opcodes ();

  /* Remember the current text section.	 */
  unwind.saved_seg = now_seg;
  unwind.saved_subseg = now_subseg;

  start_unwind_section (now_seg, 0);

  if (unwind.personality_routine == NULL)
    {
      if (unwind.personality_index == -2)
	{
	  if (have_data)
	    as_bad (_("handlerdata in cantunwind frame"));
	  return 1; /* EXIDX_CANTUNWIND.  */
	}

      /* Use a default personality routine if none is specified.  */
      if (unwind.personality_index == -1)
	{
	  if (unwind.opcode_count > 3)
	    unwind.personality_index = 1;
	  else
	    unwind.personality_index = 0;
	}

      /* Space for the personality routine entry.  */
      if (unwind.personality_index == 0)
	{
	  if (unwind.opcode_count > 3)
	    as_bad (_("too many unwind opcodes for personality routine 0"));

	  if (!have_data)
	    {
	      /* All the data is inline in the index table.  */
	      data = 0x80;
	      n = 3;
	      while (unwind.opcode_count > 0)
		{
		  unwind.opcode_count--;
		  data = (data << 8) | unwind.opcodes[unwind.opcode_count];
		  n--;
		}

	      /* Pad with "finish" opcodes.  */
	      while (n--)
		data = (data << 8) | 0xb0;

	      return data;
	    }
	  size = 0;
	}
      else
	/* We get two opcodes "free" in the first word.	 */
	size = unwind.opcode_count - 2;
    }
  else
    {
      /* PR 16765: Missing or misplaced unwind directives can trigger this.  */
      if (unwind.personality_index != -1)
	{
	  as_bad (_("attempt to recreate an unwind entry"));
	  return 1;
	}

      /* An extra byte is required for the opcode count.	*/
      size = unwind.opcode_count + 1;
    }

  size = (size + 3) >> 2;
  if (size > 0xff)
    as_bad (_("too many unwind opcodes"));

  frag_align (2, 0, 0);
  record_alignment (now_seg, 2);
  unwind.table_entry = expr_build_dot ();

  /* Allocate the table entry.	*/
  ptr = frag_more ((size << 2) + 4);
  /* PR 13449: Zero the table entries in case some of them are not used.  */
  memset (ptr, 0, (size << 2) + 4);
  where = frag_now_fix () - ((size << 2) + 4);

  switch (unwind.personality_index)
    {
    case -1:
      /* ??? Should this be a PLT generating relocation?  */
      /* Custom personality routine.  */
      fix_new (frag_now, where, 4, unwind.personality_routine, 0, 1,
	       BFD_RELOC_ARM_PREL31);

      where += 4;
      ptr += 4;

      /* Set the first byte to the number of additional words.	*/
      data = size > 0 ? size - 1 : 0;
      n = 3;
      break;

    /* ABI defined personality routines.  */
    case 0:
      /* Three opcodes bytes are packed into the first word.  */
      data = 0x80;
      n = 3;
      break;

    case 1:
    case 2:
      /* The size and first two opcode bytes go in the first word.  */
      data = ((0x80 + unwind.personality_index) << 8) | size;
      n = 2;
      break;

    default:
      /* Should never happen.  */
      abort ();
    }

  /* Pack the opcodes into words (MSB first), reversing the list at the same
     time.  */
  while (unwind.opcode_count > 0)
    {
      if (n == 0)
	{
	  md_number_to_chars (ptr, data, 4);
	  ptr += 4;
	  n = 4;
	  data = 0;
	}
      unwind.opcode_count--;
      n--;
      data = (data << 8) | unwind.opcodes[unwind.opcode_count];
    }

  /* Finish off the last word.	*/
  if (n < 4)
    {
      /* Pad with "finish" opcodes.  */
      while (n--)
	data = (data << 8) | 0xb0;

      md_number_to_chars (ptr, data, 4);
    }

  if (!have_data)
    {
      /* Add an empty descriptor if there is no user-specified data.   */
      ptr = frag_more (4);
      md_number_to_chars (ptr, 0, 4);
    }

  return 0;
}

/* Initialize the DWARF-2 unwind information for this procedure.  */

void
tc_arm_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa (REG_SP, 0);
}
#endif /* OBJ_ELF */

/* Convert REGNAME to a DWARF-2 register number.  */

int
tc_arm_regname_to_dw2regnum (char *regname)
{
  int reg = arm_reg_parse (&regname, REG_TYPE_RN);
  if (reg != FAIL)
    return reg;

  /* PR 16694: Allow VFP registers as well.  */
  reg = arm_reg_parse (&regname, REG_TYPE_VFS);
  if (reg != FAIL)
    return 64 + reg;

  reg = arm_reg_parse (&regname, REG_TYPE_VFD);
  if (reg != FAIL)
    return reg + 256;

  reg = arm_reg_parse (&regname, REG_TYPE_PSEUDO);
  if (reg != FAIL)
    return reg;

  return FAIL;
}

#ifdef TE_PE
void
tc_pe_dwarf2_emit_offset (symbolS *symbol, unsigned int size)
{
  expressionS exp;

  exp.X_op = O_secrel;
  exp.X_add_symbol = symbol;
  exp.X_add_number = 0;
  emit_expr (&exp, size);
}
#endif

/* MD interface: Symbol and relocation handling.  */

/* Return the address within the segment that a PC-relative fixup is
   relative to.  For ARM, PC-relative fixups applied to instructions
   are generally relative to the location of the fixup plus 8 bytes.
   Thumb branches are offset by 4, and Thumb loads relative to PC
   require special handling.  */

long
md_pcrel_from_section (fixS * fixP, segT seg)
{
  offsetT base = fixP->fx_where + fixP->fx_frag->fr_address;

  /* If this is pc-relative and we are going to emit a relocation
     then we just want to put out any pipeline compensation that the linker
     will need.  Otherwise we want to use the calculated base.
     For WinCE we skip the bias for externals as well, since this
     is how the MS ARM-CE assembler behaves and we want to be compatible.  */
  if (fixP->fx_pcrel
      && ((fixP->fx_addsy && S_GET_SEGMENT (fixP->fx_addsy) != seg)
	  || (arm_force_relocation (fixP)
#ifdef TE_WINCE
	      && !S_IS_EXTERNAL (fixP->fx_addsy)
#endif
	      )))
    base = 0;


  switch (fixP->fx_r_type)
    {
      /* PC relative addressing on the Thumb is slightly odd as the
	 bottom two bits of the PC are forced to zero for the
	 calculation.  This happens *after* application of the
	 pipeline offset.  However, Thumb adrl already adjusts for
	 this, so we need not do it again.  */
    case BFD_RELOC_ARM_THUMB_ADD:
      return base & ~3;

    case BFD_RELOC_ARM_THUMB_OFFSET:
    case BFD_RELOC_ARM_T32_OFFSET_IMM:
    case BFD_RELOC_ARM_T32_ADD_PC12:
    case BFD_RELOC_ARM_T32_CP_OFF_IMM:
      return (base + 4) & ~3;

      /* Thumb branches are simply offset by +4.  */
    case BFD_RELOC_THUMB_PCREL_BRANCH5:
    case BFD_RELOC_THUMB_PCREL_BRANCH7:
    case BFD_RELOC_THUMB_PCREL_BRANCH9:
    case BFD_RELOC_THUMB_PCREL_BRANCH12:
    case BFD_RELOC_THUMB_PCREL_BRANCH20:
    case BFD_RELOC_THUMB_PCREL_BRANCH25:
    case BFD_RELOC_THUMB_PCREL_BFCSEL:
    case BFD_RELOC_ARM_THUMB_BF17:
    case BFD_RELOC_ARM_THUMB_BF19:
    case BFD_RELOC_ARM_THUMB_BF13:
    case BFD_RELOC_ARM_THUMB_LOOP12:
      return base + 4;

    case BFD_RELOC_THUMB_PCREL_BRANCH23:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
	base = fixP->fx_where + fixP->fx_frag->fr_address;
       return base + 4;

      /* BLX is like branches above, but forces the low two bits of PC to
	 zero.  */
    case BFD_RELOC_THUMB_PCREL_BLX:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && THUMB_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
	base = fixP->fx_where + fixP->fx_frag->fr_address;
      return (base + 4) & ~3;

      /* ARM mode branches are offset by +8.  However, the Windows CE
	 loader expects the relocation not to take this into account.  */
    case BFD_RELOC_ARM_PCREL_BLX:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
	base = fixP->fx_where + fixP->fx_frag->fr_address;
      return base + 8;

    case BFD_RELOC_ARM_PCREL_CALL:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && THUMB_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
	base = fixP->fx_where + fixP->fx_frag->fr_address;
      return base + 8;

    case BFD_RELOC_ARM_PCREL_BRANCH:
    case BFD_RELOC_ARM_PCREL_JUMP:
    case BFD_RELOC_ARM_PLT32:
#ifdef TE_WINCE
      /* When handling fixups immediately, because we have already
	 discovered the value of a symbol, or the address of the frag involved
	 we must account for the offset by +8, as the OS loader will never see the reloc.
	 see fixup_segment() in write.c
	 The S_IS_EXTERNAL test handles the case of global symbols.
	 Those need the calculated base, not just the pipe compensation the linker will need.  */
      if (fixP->fx_pcrel
	  && fixP->fx_addsy != NULL
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && (S_IS_EXTERNAL (fixP->fx_addsy) || !arm_force_relocation (fixP)))
	return base + 8;
      return base;
#else
      return base + 8;
#endif


      /* ARM mode loads relative to PC are also offset by +8.  Unlike
	 branches, the Windows CE loader *does* expect the relocation
	 to take this into account.  */
    case BFD_RELOC_ARM_OFFSET_IMM:
    case BFD_RELOC_ARM_OFFSET_IMM8:
    case BFD_RELOC_ARM_HWLITERAL:
    case BFD_RELOC_ARM_LITERAL:
    case BFD_RELOC_ARM_CP_OFF_IMM:
      return base + 8;


      /* Other PC-relative relocations are un-offset.  */
    default:
      return base;
    }
}

static bool flag_warn_syms = true;

bool
arm_tc_equal_in_insn (int c ATTRIBUTE_UNUSED, char * name)
{
  /* PR 18347 - Warn if the user attempts to create a symbol with the same
     name as an ARM instruction.  Whilst strictly speaking it is allowed, it
     does mean that the resulting code might be very confusing to the reader.
     Also this warning can be triggered if the user omits an operand before
     an immediate address, eg:

       LDR =foo

     GAS treats this as an assignment of the value of the symbol foo to a
     symbol LDR, and so (without this code) it will not issue any kind of
     warning or error message.

     Note - ARM instructions are case-insensitive but the strings in the hash
     table are all stored in lower case, so we must first ensure that name is
     lower case too.  */
  if (flag_warn_syms && arm_ops_hsh)
    {
      char * nbuf = strdup (name);
      char * p;

      for (p = nbuf; *p; p++)
	*p = TOLOWER (*p);
      if (str_hash_find (arm_ops_hsh, nbuf) != NULL)
	{
	  static htab_t  already_warned = NULL;

	  if (already_warned == NULL)
	    already_warned = str_htab_create ();
	  /* Only warn about the symbol once.  To keep the code
	     simple we let str_hash_insert do the lookup for us.  */
	  if (str_hash_insert (already_warned, nbuf, NULL, 0) == NULL)
	    as_warn (_("[-mwarn-syms]: Assignment makes a symbol match an ARM instruction: %s"), name);
	}
      else
	free (nbuf);
    }

  return false;
}

/* Under ELF we need to default _GLOBAL_OFFSET_TABLE.
   Otherwise we have no need to default values of symbols.  */

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED)
{
#ifdef OBJ_ELF
  if (name[0] == '_' && name[1] == 'G'
      && streq (name, GLOBAL_OFFSET_TABLE_NAME))
    {
      if (!GOT_symbol)
	{
	  if (symbol_find (name))
	    as_bad (_("GOT already in the symbol table"));

	  GOT_symbol = symbol_new (name, undefined_section,
				   &zero_address_frag, 0);
	}

      return GOT_symbol;
    }
#endif

  return NULL;
}

/* Subroutine of md_apply_fix.	 Check to see if an immediate can be
   computed as two separate immediate values, added together.  We
   already know that this value cannot be computed by just one ARM
   instruction.	 */

static unsigned int
validate_immediate_twopart (unsigned int   val,
			    unsigned int * highpart)
{
  unsigned int a;
  unsigned int i;

  for (i = 0; i < 32; i += 2)
    if (((a = rotate_left (val, i)) & 0xff) != 0)
      {
	if (a & 0xff00)
	  {
	    if (a & ~ 0xffff)
	      continue;
	    * highpart = (a  >> 8) | ((i + 24) << 7);
	  }
	else if (a & 0xff0000)
	  {
	    if (a & 0xff000000)
	      continue;
	    * highpart = (a >> 16) | ((i + 16) << 7);
	  }
	else
	  {
	    gas_assert (a & 0xff000000);
	    * highpart = (a >> 24) | ((i + 8) << 7);
	  }

	return (a & 0xff) | (i << 7);
      }

  return FAIL;
}

static int
validate_offset_imm (unsigned int val, int hwse)
{
  if ((hwse && val > 255) || val > 4095)
    return FAIL;
  return val;
}

/* Subroutine of md_apply_fix.	 Do those data_ops which can take a
   negative immediate constant by altering the instruction.  A bit of
   a hack really.
	MOV <-> MVN
	AND <-> BIC
	ADC <-> SBC
	by inverting the second operand, and
	ADD <-> SUB
	CMP <-> CMN
	by negating the second operand.	 */

static int
negate_data_op (unsigned long * instruction,
		unsigned long	value)
{
  int op, new_inst;
  unsigned long negated, inverted;

  negated = encode_arm_immediate (-value);
  inverted = encode_arm_immediate (~value);

  op = (*instruction >> DATA_OP_SHIFT) & 0xf;
  switch (op)
    {
      /* First negates.	 */
    case OPCODE_SUB:		 /* ADD <-> SUB	 */
      new_inst = OPCODE_ADD;
      value = negated;
      break;

    case OPCODE_ADD:
      new_inst = OPCODE_SUB;
      value = negated;
      break;

    case OPCODE_CMP:		 /* CMP <-> CMN	 */
      new_inst = OPCODE_CMN;
      value = negated;
      break;

    case OPCODE_CMN:
      new_inst = OPCODE_CMP;
      value = negated;
      break;

      /* Now Inverted ops.  */
    case OPCODE_MOV:		 /* MOV <-> MVN	 */
      new_inst = OPCODE_MVN;
      value = inverted;
      break;

    case OPCODE_MVN:
      new_inst = OPCODE_MOV;
      value = inverted;
      break;

    case OPCODE_AND:		 /* AND <-> BIC	 */
      new_inst = OPCODE_BIC;
      value = inverted;
      break;

    case OPCODE_BIC:
      new_inst = OPCODE_AND;
      value = inverted;
      break;

    case OPCODE_ADC:		  /* ADC <-> SBC  */
      new_inst = OPCODE_SBC;
      value = inverted;
      break;

    case OPCODE_SBC:
      new_inst = OPCODE_ADC;
      value = inverted;
      break;

      /* We cannot do anything.	 */
    default:
      return FAIL;
    }

  if (value == (unsigned) FAIL)
    return FAIL;

  *instruction &= OPCODE_MASK;
  *instruction |= new_inst << DATA_OP_SHIFT;
  return value;
}

/* Like negate_data_op, but for Thumb-2.   */

static unsigned int
thumb32_negate_data_op (valueT *instruction, unsigned int value)
{
  unsigned int op, new_inst;
  unsigned int rd;
  unsigned int negated, inverted;

  negated = encode_thumb32_immediate (-value);
  inverted = encode_thumb32_immediate (~value);

  rd = (*instruction >> 8) & 0xf;
  op = (*instruction >> T2_DATA_OP_SHIFT) & 0xf;
  switch (op)
    {
      /* ADD <-> SUB.  Includes CMP <-> CMN.  */
    case T2_OPCODE_SUB:
      new_inst = T2_OPCODE_ADD;
      value = negated;
      break;

    case T2_OPCODE_ADD:
      new_inst = T2_OPCODE_SUB;
      value = negated;
      break;

      /* ORR <-> ORN.  Includes MOV <-> MVN.  */
    case T2_OPCODE_ORR:
      new_inst = T2_OPCODE_ORN;
      value = inverted;
      break;

    case T2_OPCODE_ORN:
      new_inst = T2_OPCODE_ORR;
      value = inverted;
      break;

      /* AND <-> BIC.  TST has no inverted equivalent.  */
    case T2_OPCODE_AND:
      new_inst = T2_OPCODE_BIC;
      if (rd == 15)
	value = FAIL;
      else
	value = inverted;
      break;

    case T2_OPCODE_BIC:
      new_inst = T2_OPCODE_AND;
      value = inverted;
      break;

      /* ADC <-> SBC  */
    case T2_OPCODE_ADC:
      new_inst = T2_OPCODE_SBC;
      value = inverted;
      break;

    case T2_OPCODE_SBC:
      new_inst = T2_OPCODE_ADC;
      value = inverted;
      break;

      /* We cannot do anything.	 */
    default:
      return FAIL;
    }

  if (value == (unsigned int)FAIL)
    return FAIL;

  *instruction &= T2_OPCODE_MASK;
  *instruction |= new_inst << T2_DATA_OP_SHIFT;
  return value;
}

/* Read a 32-bit thumb instruction from buf.  */

static unsigned long
get_thumb32_insn (char * buf)
{
  unsigned long insn;
  insn = md_chars_to_number (buf, THUMB_SIZE) << 16;
  insn |= md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);

  return insn;
}

/* We usually want to set the low bit on the address of thumb function
   symbols.  In particular .word foo - . should have the low bit set.
   Generic code tries to fold the difference of two symbols to
   a constant.  Prevent this and force a relocation when the first symbols
   is a thumb function.  */

bool
arm_optimize_expr (expressionS *l, operatorT op, expressionS *r)
{
  if (op == O_subtract
      && l->X_op == O_symbol
      && r->X_op == O_symbol
      && THUMB_IS_FUNC (l->X_add_symbol))
    {
      l->X_op = O_subtract;
      l->X_op_symbol = r->X_add_symbol;
      l->X_add_number -= r->X_add_number;
      return true;
    }

  /* Process as normal.  */
  return false;
}

/* Encode Thumb2 unconditional branches and calls. The encoding
   for the 2 are identical for the immediate values.  */

static void
encode_thumb2_b_bl_offset (char * buf, offsetT value)
{
#define T2I1I2MASK  ((1 << 13) | (1 << 11))
  offsetT newval;
  offsetT newval2;
  addressT S, I1, I2, lo, hi;

  S = (value >> 24) & 0x01;
  I1 = (value >> 23) & 0x01;
  I2 = (value >> 22) & 0x01;
  hi = (value >> 12) & 0x3ff;
  lo = (value >> 1) & 0x7ff;
  newval   = md_chars_to_number (buf, THUMB_SIZE);
  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
  newval  |= (S << 10) | hi;
  newval2 &=  ~T2I1I2MASK;
  newval2 |= (((I1 ^ S) << 13) | ((I2 ^ S) << 11) | lo) ^ T2I1I2MASK;
  md_number_to_chars (buf, newval, THUMB_SIZE);
  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
}

void
md_apply_fix (fixS *	fixP,
	       valueT * valP,
	       segT	seg)
{
  valueT	 value = * valP;
  valueT	 newval;
  unsigned int	 newimm;
  unsigned long	 temp;
  int		 sign;
  char *	 buf = fixP->fx_where + fixP->fx_frag->fr_literal;

  gas_assert (fixP->fx_r_type <= BFD_RELOC_UNUSED);

  /* Note whether this will delete the relocation.  */

  if (fixP->fx_addsy == 0 && !fixP->fx_pcrel)
    fixP->fx_done = 1;

  /* On a 64-bit host, silently truncate 'value' to 32 bits for
     consistency with the behaviour on 32-bit hosts.  Remember value
     for emit_reloc.  */
  value &= 0xffffffff;
  value ^= 0x80000000;
  value -= 0x80000000;

  *valP = value;
  fixP->fx_addnumber = value;

  /* Same treatment for fixP->fx_offset.  */
  fixP->fx_offset &= 0xffffffff;
  fixP->fx_offset ^= 0x80000000;
  fixP->fx_offset -= 0x80000000;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_NONE:
      /* This will need to go in the object file.  */
      fixP->fx_done = 0;
      break;

    case BFD_RELOC_ARM_IMMEDIATE:
      /* We claim that this fixup has been processed here,
	 even if in fact we generate an error because we do
	 not have a reloc for it, so tc_gen_reloc will reject it.  */
      fixP->fx_done = 1;

      if (fixP->fx_addsy)
	{
	  const char *msg = 0;

	  if (! S_IS_DEFINED (fixP->fx_addsy))
	    msg = _("undefined symbol %s used as an immediate value");
	  else if (S_GET_SEGMENT (fixP->fx_addsy) != seg)
	    msg = _("symbol %s is in a different section");
	  else if (S_IS_WEAK (fixP->fx_addsy))
	    msg = _("symbol %s is weak and may be overridden later");

	  if (msg)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    msg, S_GET_NAME (fixP->fx_addsy));
	      break;
	    }
	}

      temp = md_chars_to_number (buf, INSN_SIZE);

      /* If the offset is negative, we should use encoding A2 for ADR.  */
      if ((temp & 0xfff0000) == 0x28f0000 && (offsetT) value < 0)
	newimm = negate_data_op (&temp, value);
      else
	{
	  newimm = encode_arm_immediate (value);

	  /* If the instruction will fail, see if we can fix things up by
	     changing the opcode.  */
	  if (newimm == (unsigned int) FAIL)
	    newimm = negate_data_op (&temp, value);
	  /* MOV accepts both ARM modified immediate (A1 encoding) and
	     UINT16 (A2 encoding) when possible, MOVW only accepts UINT16.
	     When disassembling, MOV is preferred when there is no encoding
	     overlap.  */
	  if (newimm == (unsigned int) FAIL
	      && ((temp >> DATA_OP_SHIFT) & 0xf) == OPCODE_MOV
	      && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2)
	      && !((temp >> SBIT_SHIFT) & 0x1)
	      && value <= 0xffff)
	    {
	      /* Clear bits[23:20] to change encoding from A1 to A2.  */
	      temp &= 0xff0fffff;
	      /* Encoding high 4bits imm.  Code below will encode the remaining
		 low 12bits.  */
	      temp |= (value & 0x0000f000) << 4;
	      newimm = value & 0x00000fff;
	    }
	}

      if (newimm == (unsigned int) FAIL)
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("invalid constant (%lx) after fixup"),
			(unsigned long) value);
	  break;
	}

      newimm |= (temp & 0xfffff000);
      md_number_to_chars (buf, (valueT) newimm, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_ADRL_IMMEDIATE:
      {
	unsigned int highpart = 0;
	unsigned int newinsn  = 0xe1a00000; /* nop.  */

	if (fixP->fx_addsy)
	  {
	    const char *msg = 0;

	    if (! S_IS_DEFINED (fixP->fx_addsy))
	      msg = _("undefined symbol %s used as an immediate value");
	    else if (S_GET_SEGMENT (fixP->fx_addsy) != seg)
	      msg = _("symbol %s is in a different section");
	    else if (S_IS_WEAK (fixP->fx_addsy))
	      msg = _("symbol %s is weak and may be overridden later");

	    if (msg)
	      {
		as_bad_where (fixP->fx_file, fixP->fx_line,
			      msg, S_GET_NAME (fixP->fx_addsy));
		break;
	      }
	  }

	newimm = encode_arm_immediate (value);
	temp = md_chars_to_number (buf, INSN_SIZE);

	/* If the instruction will fail, see if we can fix things up by
	   changing the opcode.	 */
	if (newimm == (unsigned int) FAIL
	    && (newimm = negate_data_op (& temp, value)) == (unsigned int) FAIL)
	  {
	    /* No ?  OK - try using two ADD instructions to generate
	       the value.  */
	    newimm = validate_immediate_twopart (value, & highpart);

	    /* Yes - then make sure that the second instruction is
	       also an add.  */
	    if (newimm != (unsigned int) FAIL)
	      newinsn = temp;
	    /* Still No ?  Try using a negated value.  */
	    else if ((newimm = validate_immediate_twopart (- value, & highpart)) != (unsigned int) FAIL)
	      temp = newinsn = (temp & OPCODE_MASK) | OPCODE_SUB << DATA_OP_SHIFT;
	    /* Otherwise - give up.  */
	    else
	      {
		as_bad_where (fixP->fx_file, fixP->fx_line,
			      _("unable to compute ADRL instructions for PC offset of 0x%lx"),
			      (long) value);
		break;
	      }

	    /* Replace the first operand in the 2nd instruction (which
	       is the PC) with the destination register.  We have
	       already added in the PC in the first instruction and we
	       do not want to do it again.  */
	    newinsn &= ~ 0xf0000;
	    newinsn |= ((newinsn & 0x0f000) << 4);
	  }

	newimm |= (temp & 0xfffff000);
	md_number_to_chars (buf, (valueT) newimm, INSN_SIZE);

	highpart |= (newinsn & 0xfffff000);
	md_number_to_chars (buf + INSN_SIZE, (valueT) highpart, INSN_SIZE);
      }
      break;

    case BFD_RELOC_ARM_OFFSET_IMM:
      if (!fixP->fx_done && seg->use_rela_p)
	value = 0;
      /* Fall through.  */

    case BFD_RELOC_ARM_LITERAL:
      sign = (offsetT) value > 0;

      if ((offsetT) value < 0)
	value = - value;

      if (validate_offset_imm (value, 0) == FAIL)
	{
	  if (fixP->fx_r_type == BFD_RELOC_ARM_LITERAL)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid literal constant: pool needs to be closer"));
	  else
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad immediate value for offset (%ld)"),
			  (long) value);
	  break;
	}

      newval = md_chars_to_number (buf, INSN_SIZE);
      if (value == 0)
	newval &= 0xfffff000;
      else
	{
	  newval &= 0xff7ff000;
	  newval |= value | (sign ? INDEX_UP : 0);
	}
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_OFFSET_IMM8:
    case BFD_RELOC_ARM_HWLITERAL:
      sign = (offsetT) value > 0;

      if ((offsetT) value < 0)
	value = - value;

      if (validate_offset_imm (value, 1) == FAIL)
	{
	  if (fixP->fx_r_type == BFD_RELOC_ARM_HWLITERAL)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid literal constant: pool needs to be closer"));
	  else
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad immediate value for 8-bit offset (%ld)"),
			  (long) value);
	  break;
	}

      newval = md_chars_to_number (buf, INSN_SIZE);
      if (value == 0)
	newval &= 0xfffff0f0;
      else
	{
	  newval &= 0xff7ff0f0;
	  newval |= ((value >> 4) << 8) | (value & 0xf) | (sign ? INDEX_UP : 0);
	}
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_T32_OFFSET_U8:
      if (value > 1020 || value % 4 != 0)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("bad immediate value for offset (%ld)"), (long) value);
      value /= 4;

      newval = md_chars_to_number (buf+2, THUMB_SIZE);
      newval |= value;
      md_number_to_chars (buf+2, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_T32_OFFSET_IMM:
      /* This is a complicated relocation used for all varieties of Thumb32
	 load/store instruction with immediate offset:

	 1110 100P u1WL NNNN XXXX YYYY iiii iiii - +/-(U) pre/post(P) 8-bit,
						   *4, optional writeback(W)
						   (doubleword load/store)

	 1111 100S uTTL 1111 XXXX iiii iiii iiii - +/-(U) 12-bit PC-rel
	 1111 100S 0TTL NNNN XXXX 1Pu1 iiii iiii - +/-(U) pre/post(P) 8-bit
	 1111 100S 0TTL NNNN XXXX 1110 iiii iiii - positive 8-bit (T instruction)
	 1111 100S 1TTL NNNN XXXX iiii iiii iiii - positive 12-bit
	 1111 100S 0TTL NNNN XXXX 1100 iiii iiii - negative 8-bit

	 Uppercase letters indicate bits that are already encoded at
	 this point.  Lowercase letters are our problem.  For the
	 second block of instructions, the secondary opcode nybble
	 (bits 8..11) is present, and bit 23 is zero, even if this is
	 a PC-relative operation.  */
      newval = md_chars_to_number (buf, THUMB_SIZE);
      newval <<= 16;
      newval |= md_chars_to_number (buf+THUMB_SIZE, THUMB_SIZE);

      if ((newval & 0xf0000000) == 0xe0000000)
	{
	  /* Doubleword load/store: 8-bit offset, scaled by 4.  */
	  if ((offsetT) value >= 0)
	    newval |= (1 << 23);
	  else
	    value = -value;
	  if (value % 4 != 0)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset not a multiple of 4"));
	      break;
	    }
	  value /= 4;
	  if (value > 0xff)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~0xff;
	}
      else if ((newval & 0x000f0000) == 0x000f0000)
	{
	  /* PC-relative, 12-bit offset.  */
	  if ((offsetT) value >= 0)
	    newval |= (1 << 23);
	  else
	    value = -value;
	  if (value > 0xfff)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~0xfff;
	}
      else if ((newval & 0x00000100) == 0x00000100)
	{
	  /* Writeback: 8-bit, +/- offset.  */
	  if ((offsetT) value >= 0)
	    newval |= (1 << 9);
	  else
	    value = -value;
	  if (value > 0xff)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~0xff;
	}
      else if ((newval & 0x00000f00) == 0x00000e00)
	{
	  /* T-instruction: positive 8-bit offset.  */
	  if (value > 0xff)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~0xff;
	  newval |= value;
	}
      else
	{
	  /* Positive 12-bit or negative 8-bit offset.  */
	  unsigned int limit;
	  if ((offsetT) value >= 0)
	    {
	      newval |= (1 << 23);
	      limit = 0xfff;
	    }
	  else
	    {
	      value = -value;
	      limit = 0xff;
	    }
	  if (value > limit)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~limit;
	}

      newval |= value;
      md_number_to_chars (buf, (newval >> 16) & 0xffff, THUMB_SIZE);
      md_number_to_chars (buf + THUMB_SIZE, newval & 0xffff, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_SHIFT_IMM:
      newval = md_chars_to_number (buf, INSN_SIZE);
      if (value > 32
	  || (value == 32
	      && (((newval & 0x60) == 0) || (newval & 0x60) == 0x60)))
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("shift expression is too large"));
	  break;
	}

      if (value == 0)
	/* Shifts of zero must be done as lsl.	*/
	newval &= ~0x60;
      else if (value == 32)
	value = 0;
      newval &= 0xfffff07f;
      newval |= (value & 0x1f) << 7;
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_T32_IMMEDIATE:
    case BFD_RELOC_ARM_T32_ADD_IMM:
    case BFD_RELOC_ARM_T32_IMM12:
    case BFD_RELOC_ARM_T32_ADD_PC12:
      /* We claim that this fixup has been processed here,
	 even if in fact we generate an error because we do
	 not have a reloc for it, so tc_gen_reloc will reject it.  */
      fixP->fx_done = 1;

      if (fixP->fx_addsy
	  && ! S_IS_DEFINED (fixP->fx_addsy))
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("undefined symbol %s used as an immediate value"),
			S_GET_NAME (fixP->fx_addsy));
	  break;
	}

      newval = md_chars_to_number (buf, THUMB_SIZE);
      newval <<= 16;
      newval |= md_chars_to_number (buf+2, THUMB_SIZE);

      newimm = FAIL;
      if ((fixP->fx_r_type == BFD_RELOC_ARM_T32_IMMEDIATE
	   /* ARMv8-M Baseline MOV will reach here, but it doesn't support
	      Thumb2 modified immediate encoding (T2).  */
	   && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2))
	  || fixP->fx_r_type == BFD_RELOC_ARM_T32_ADD_IMM)
	{
	  newimm = encode_thumb32_immediate (value);
	  if (newimm == (unsigned int) FAIL)
	    newimm = thumb32_negate_data_op (&newval, value);
	}
      if (newimm == (unsigned int) FAIL)
	{
	  if (fixP->fx_r_type != BFD_RELOC_ARM_T32_IMMEDIATE)
	    {
	      /* Turn add/sum into addw/subw.  */
	      if (fixP->fx_r_type == BFD_RELOC_ARM_T32_ADD_IMM)
		newval = (newval & 0xfeffffff) | 0x02000000;
	      /* No flat 12-bit imm encoding for addsw/subsw.  */
	      if ((newval & 0x00100000) == 0)
		{
		  /* 12 bit immediate for addw/subw.  */
		  if ((offsetT) value < 0)
		    {
		      value = -value;
		      newval ^= 0x00a00000;
		    }
		  if (value > 0xfff)
		    newimm = (unsigned int) FAIL;
		  else
		    newimm = value;
		}
	    }
	  else
	    {
	      /* MOV accepts both Thumb2 modified immediate (T2 encoding) and
		 UINT16 (T3 encoding), MOVW only accepts UINT16.  When
		 disassembling, MOV is preferred when there is no encoding
		 overlap.  */
	      if (((newval >> T2_DATA_OP_SHIFT) & 0xf) == T2_OPCODE_ORR
		  /* NOTE: MOV uses the ORR opcode in Thumb 2 mode
		     but with the Rn field [19:16] set to 1111.  */
		  && (((newval >> 16) & 0xf) == 0xf)
		  && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2_v8m)
		  && !((newval >> T2_SBIT_SHIFT) & 0x1)
		  && value <= 0xffff)
		{
		  /* Toggle bit[25] to change encoding from T2 to T3.  */
		  newval ^= 1 << 25;
		  /* Clear bits[19:16].  */
		  newval &= 0xfff0ffff;
		  /* Encoding high 4bits imm.  Code below will encode the
		     remaining low 12bits.  */
		  newval |= (value & 0x0000f000) << 4;
		  newimm = value & 0x00000fff;
		}
	    }
	}

      if (newimm == (unsigned int)FAIL)
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("invalid constant (%lx) after fixup"),
			(unsigned long) value);
	  break;
	}

      newval |= (newimm & 0x800) << 15;
      newval |= (newimm & 0x700) << 4;
      newval |= (newimm & 0x0ff);

      md_number_to_chars (buf,   (valueT) ((newval >> 16) & 0xffff), THUMB_SIZE);
      md_number_to_chars (buf+2, (valueT) (newval & 0xffff), THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_SMC:
      if (value > 0xf)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid smc expression"));

      newval = md_chars_to_number (buf, INSN_SIZE);
      newval |= (value & 0xf);
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_HVC:
      if (value > 0xffff)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid hvc expression"));
      newval = md_chars_to_number (buf, INSN_SIZE);
      newval |= (value & 0xf) | ((value & 0xfff0) << 4);
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_SWI:
      if (fixP->tc_fix_data != 0)
	{
	  if (value > 0xff)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid swi expression"));
	  newval = md_chars_to_number (buf, THUMB_SIZE);
	  newval |= value;
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      else
	{
	  if (value > 0x00ffffff)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid swi expression"));
	  newval = md_chars_to_number (buf, INSN_SIZE);
	  newval |= value;
	  md_number_to_chars (buf, newval, INSN_SIZE);
	}
      break;

    case BFD_RELOC_ARM_MULTI:
      if (value > 0xffff)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid expression in load/store multiple"));
      newval = value | md_chars_to_number (buf, INSN_SIZE);
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

#ifdef OBJ_ELF
    case BFD_RELOC_ARM_PCREL_CALL:

      if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t)
	  && fixP->fx_addsy
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && THUMB_IS_FUNC (fixP->fx_addsy))
	/* Flip the bl to blx. This is a simple flip
	   bit here because we generate PCREL_CALL for
	   unconditional bls.  */
	{
	  newval = md_chars_to_number (buf, INSN_SIZE);
	  newval = newval | 0x10000000;
	  md_number_to_chars (buf, newval, INSN_SIZE);
	  temp = 1;
	  fixP->fx_done = 1;
	}
      else
	temp = 3;
      goto arm_branch_common;

    case BFD_RELOC_ARM_PCREL_JUMP:
      if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t)
	  && fixP->fx_addsy
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && THUMB_IS_FUNC (fixP->fx_addsy))
	{
	  /* This would map to a bl<cond>, b<cond>,
	     b<always> to a Thumb function. We
	     need to force a relocation for this particular
	     case.  */
	  newval = md_chars_to_number (buf, INSN_SIZE);
	  fixP->fx_done = 0;
	}
      /* Fall through.  */

    case BFD_RELOC_ARM_PLT32:
#endif
    case BFD_RELOC_ARM_PCREL_BRANCH:
      temp = 3;
      goto arm_branch_common;

    case BFD_RELOC_ARM_PCREL_BLX:

      temp = 1;
      if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t)
	  && fixP->fx_addsy
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && ARM_IS_FUNC (fixP->fx_addsy))
	{
	  /* Flip the blx to a bl and warn.  */
	  const char *name = S_GET_NAME (fixP->fx_addsy);
	  newval = 0xeb000000;
	  as_warn_where (fixP->fx_file, fixP->fx_line,
			 _("blx to '%s' an ARM ISA state function changed to bl"),
			  name);
	  md_number_to_chars (buf, newval, INSN_SIZE);
	  temp = 3;
	  fixP->fx_done = 1;
	}

#ifdef OBJ_ELF
       if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4)
	 fixP->fx_r_type = BFD_RELOC_ARM_PCREL_CALL;
#endif

    arm_branch_common:
      /* We are going to store value (shifted right by two) in the
	 instruction, in a 24 bit, signed field.  Bits 26 through 32 either
	 all clear or all set and bit 0 must be clear.  For B/BL bit 1 must
	 also be clear.  */
      if (value & temp)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("misaligned branch destination"));
      if ((value & 0xfe000000) != 0
	  && (value & 0xfe000000) != 0xfe000000)
	as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval = md_chars_to_number (buf, INSN_SIZE);
	  newval |= (value >> 2) & 0x00ffffff;
	  /* Set the H bit on BLX instructions.  */
	  if (temp == 1)
	    {
	      if (value & 2)
		newval |= 0x01000000;
	      else
		newval &= ~0x01000000;
	    }
	  md_number_to_chars (buf, newval, INSN_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH7: /* CBZ */
      /* CBZ can only branch forward.  */

      /* Attempts to use CBZ to branch to the next instruction
	 (which, strictly speaking, are prohibited) will be turned into
	 no-ops.

	 FIXME: It may be better to remove the instruction completely and
	 perform relaxation.  */
      if ((offsetT) value == -2)
	{
	  newval = md_chars_to_number (buf, THUMB_SIZE);
	  newval = 0xbf00; /* NOP encoding T1 */
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      else
	{
	  if (value & ~0x7e)
	    as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

	  if (fixP->fx_done || !seg->use_rela_p)
	    {
	      newval = md_chars_to_number (buf, THUMB_SIZE);
	      newval |= ((value & 0x3e) << 2) | ((value & 0x40) << 3);
	      md_number_to_chars (buf, newval, THUMB_SIZE);
	    }
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH9: /* Conditional branch.	*/
      if (out_of_range_p (value, 8))
	as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval = md_chars_to_number (buf, THUMB_SIZE);
	  newval |= (value & 0x1ff) >> 1;
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH12: /* Unconditional branch.  */
      if (out_of_range_p (value, 11))
	as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval = md_chars_to_number (buf, THUMB_SIZE);
	  newval |= (value & 0xfff) >> 1;
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      break;

    /* This relocation is misnamed, it should be BRANCH21.  */
    case BFD_RELOC_THUMB_PCREL_BRANCH20:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
	{
	  /* Force a relocation for a branch 20 bits wide.  */
	  fixP->fx_done = 0;
	}
      if (out_of_range_p (value, 20))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("conditional branch out of range"));

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  offsetT newval2;
	  addressT S, J1, J2, lo, hi;

	  S  = (value & 0x00100000) >> 20;
	  J2 = (value & 0x00080000) >> 19;
	  J1 = (value & 0x00040000) >> 18;
	  hi = (value & 0x0003f000) >> 12;
	  lo = (value & 0x00000ffe) >> 1;

	  newval   = md_chars_to_number (buf, THUMB_SIZE);
	  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval  |= (S << 10) | hi;
	  newval2 |= (J1 << 13) | (J2 << 11) | lo;
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BLX:
      /* If there is a blx from a thumb state function to
	 another thumb function flip this to a bl and warn
	 about it.  */

      if (fixP->fx_addsy
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && THUMB_IS_FUNC (fixP->fx_addsy))
	{
	  const char *name = S_GET_NAME (fixP->fx_addsy);
	  as_warn_where (fixP->fx_file, fixP->fx_line,
			 _("blx to Thumb func '%s' from Thumb ISA state changed to bl"),
			 name);
	  newval = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval = newval | 0x1000;
	  md_number_to_chars (buf+THUMB_SIZE, newval, THUMB_SIZE);
	  fixP->fx_r_type = BFD_RELOC_THUMB_PCREL_BRANCH23;
	  fixP->fx_done = 1;
	}


      goto thumb_bl_common;

    case BFD_RELOC_THUMB_PCREL_BRANCH23:
      /* A bl from Thumb state ISA to an internal ARM state function
	 is converted to a blx.  */
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
	{
	  newval = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval = newval & ~0x1000;
	  md_number_to_chars (buf+THUMB_SIZE, newval, THUMB_SIZE);
	  fixP->fx_r_type = BFD_RELOC_THUMB_PCREL_BLX;
	  fixP->fx_done = 1;
	}

    thumb_bl_common:

      if (fixP->fx_r_type == BFD_RELOC_THUMB_PCREL_BLX)
	/* For a BLX instruction, make sure that the relocation is rounded up
	   to a word boundary.  This follows the semantics of the instruction
	   which specifies that bit 1 of the target address will come from bit
	   1 of the base address.  */
	value = (value + 3) & ~ 3;

#ifdef OBJ_ELF
       if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4
	   && fixP->fx_r_type == BFD_RELOC_THUMB_PCREL_BLX)
	 fixP->fx_r_type = BFD_RELOC_THUMB_PCREL_BRANCH23;
#endif

      if (out_of_range_p (value, 22))
	{
	  if (!(ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2)))
	    as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);
	  else if (out_of_range_p (value, 24))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("Thumb2 branch out of range"));
	}

      if (fixP->fx_done || !seg->use_rela_p)
	encode_thumb2_b_bl_offset (buf, value);

      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH25:
      if (out_of_range_p (value, 24))
	as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

      if (fixP->fx_done || !seg->use_rela_p)
	  encode_thumb2_b_bl_offset (buf, value);

      break;

    case BFD_RELOC_8:
      if (fixP->fx_done || !seg->use_rela_p)
	*buf = value;
      break;

    case BFD_RELOC_16:
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, value, 2);
      break;

#ifdef OBJ_ELF
    case BFD_RELOC_ARM_TLS_CALL:
    case BFD_RELOC_ARM_THM_TLS_CALL:
    case BFD_RELOC_ARM_TLS_DESCSEQ:
    case BFD_RELOC_ARM_THM_TLS_DESCSEQ:
    case BFD_RELOC_ARM_TLS_GOTDESC:
    case BFD_RELOC_ARM_TLS_GD32:
    case BFD_RELOC_ARM_TLS_LE32:
    case BFD_RELOC_ARM_TLS_IE32:
    case BFD_RELOC_ARM_TLS_LDM32:
    case BFD_RELOC_ARM_TLS_LDO32:
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      break;

      /* Same handling as above, but with the arm_fdpic guard.  */
    case BFD_RELOC_ARM_TLS_GD32_FDPIC:
    case BFD_RELOC_ARM_TLS_IE32_FDPIC:
    case BFD_RELOC_ARM_TLS_LDM32_FDPIC:
      if (arm_fdpic)
	{
	  S_SET_THREAD_LOCAL (fixP->fx_addsy);
	}
      else
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("Relocation supported only in FDPIC mode"));
	}
      break;

    case BFD_RELOC_ARM_GOT32:
    case BFD_RELOC_ARM_GOTOFF:
      break;

    case BFD_RELOC_ARM_GOT_PREL:
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, value, 4);
      break;

    case BFD_RELOC_ARM_TARGET2:
      /* TARGET2 is not partial-inplace, so we need to write the
	 addend here for REL targets, because it won't be written out
	 during reloc processing later.  */
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, fixP->fx_offset, 4);
      break;

      /* Relocations for FDPIC.  */
    case BFD_RELOC_ARM_GOTFUNCDESC:
    case BFD_RELOC_ARM_GOTOFFFUNCDESC:
    case BFD_RELOC_ARM_FUNCDESC:
      if (arm_fdpic)
	{
	  if (fixP->fx_done || !seg->use_rela_p)
	    md_number_to_chars (buf, 0, 4);
	}
      else
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("Relocation supported only in FDPIC mode"));
      }
      break;
#endif

    case BFD_RELOC_RVA:
    case BFD_RELOC_32:
    case BFD_RELOC_ARM_TARGET1:
    case BFD_RELOC_ARM_ROSEGREL32:
    case BFD_RELOC_ARM_SBREL32:
    case BFD_RELOC_32_PCREL:
#ifdef TE_PE
    case BFD_RELOC_32_SECREL:
#endif
      if (fixP->fx_done || !seg->use_rela_p)
#ifdef TE_WINCE
	/* For WinCE we only do this for pcrel fixups.  */
	if (fixP->fx_done || fixP->fx_pcrel)
#endif
	  md_number_to_chars (buf, value, 4);
      break;

#ifdef OBJ_ELF
    case BFD_RELOC_ARM_PREL31:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval = md_chars_to_number (buf, 4) & 0x80000000;
	  if ((value ^ (value >> 1)) & 0x40000000)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("rel31 relocation overflow"));
	    }
	  newval |= value & 0x7fffffff;
	  md_number_to_chars (buf, newval, 4);
	}
      break;
#endif

    case BFD_RELOC_ARM_CP_OFF_IMM:
    case BFD_RELOC_ARM_T32_CP_OFF_IMM:
    case BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM:
      if (fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM)
	newval = md_chars_to_number (buf, INSN_SIZE);
      else
	newval = get_thumb32_insn (buf);
      if ((newval & 0x0f200f00) == 0x0d000900)
	{
	  /* This is a fp16 vstr/vldr.  The immediate offset in the mnemonic
	     has permitted values that are multiples of 2, in the range -510
	     to 510.  */
	  if (value + 510 > 510 + 510 || (value & 1))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("co-processor offset out of range"));
	}
      else if ((newval & 0xfe001f80) == 0xec000f80)
	{
	  if (value + 511 > 512 + 511 || (value & 3))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("co-processor offset out of range"));
	}
      else if (value + 1023 > 1023 + 1023 || (value & 3))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("co-processor offset out of range"));
    cp_off_common:
      sign = (offsetT) value > 0;
      if ((offsetT) value < 0)
	value = -value;
      if (fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM
	  || fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM_S2)
	newval = md_chars_to_number (buf, INSN_SIZE);
      else
	newval = get_thumb32_insn (buf);
      if (value == 0)
	{
	  if (fixP->fx_r_type == BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM)
	    newval &= 0xffffff80;
	  else
	    newval &= 0xffffff00;
	}
      else
	{
	  if (fixP->fx_r_type == BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM)
	    newval &= 0xff7fff80;
	  else
	    newval &= 0xff7fff00;
	  if ((newval & 0x0f200f00) == 0x0d000900)
	    {
	      /* This is a fp16 vstr/vldr.

		 It requires the immediate offset in the instruction is shifted
		 left by 1 to be a half-word offset.

		 Here, left shift by 1 first, and later right shift by 2
		 should get the right offset.  */
	      value <<= 1;
	    }
	  newval |= (value >> 2) | (sign ? INDEX_UP : 0);
	}
      if (fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM
	  || fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM_S2)
	md_number_to_chars (buf, newval, INSN_SIZE);
      else
	put_thumb32_insn (buf, newval);
      break;

    case BFD_RELOC_ARM_CP_OFF_IMM_S2:
    case BFD_RELOC_ARM_T32_CP_OFF_IMM_S2:
      if (value + 255 > 255 + 255)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("co-processor offset out of range"));
      value *= 4;
      goto cp_off_common;

    case BFD_RELOC_ARM_THUMB_OFFSET:
      newval = md_chars_to_number (buf, THUMB_SIZE);
      /* Exactly what ranges, and where the offset is inserted depends
	 on the type of instruction, we can establish this from the
	 top 4 bits.  */
      switch (newval >> 12)
	{
	case 4: /* PC load.  */
	  /* Thumb PC loads are somewhat odd, bit 1 of the PC is
	     forced to zero for these loads; md_pcrel_from has already
	     compensated for this.  */
	  if (value & 3)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, target not word aligned (0x%08lX)"),
			  (((unsigned long) fixP->fx_frag->fr_address
			    + (unsigned long) fixP->fx_where) & ~3)
			  + (unsigned long) value);
	  else if (get_recorded_alignment (seg) < 2)
	    as_warn_where (fixP->fx_file, fixP->fx_line,
			   _("section does not have enough alignment to ensure safe PC-relative loads"));

	  if (value & ~0x3fc)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);

	  newval |= value >> 2;
	  break;

	case 9: /* SP load/store.  */
	  if (value & ~0x3fc)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);
	  newval |= value >> 2;
	  break;

	case 6: /* Word load/store.  */
	  if (value & ~0x7c)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);
	  newval |= value << 4; /* 6 - 2.  */
	  break;

	case 7: /* Byte load/store.  */
	  if (value & ~0x1f)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);
	  newval |= value << 6;
	  break;

	case 8: /* Halfword load/store.	 */
	  if (value & ~0x3e)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);
	  newval |= value << 5; /* 6 - 1.  */
	  break;

	default:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			"Unable to process relocation for thumb opcode: %lx",
			(unsigned long) newval);
	  break;
	}
      md_number_to_chars (buf, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_THUMB_ADD:
      /* This is a complicated relocation, since we use it for all of
	 the following immediate relocations:

	    3bit ADD/SUB
	    8bit ADD/SUB
	    9bit ADD/SUB SP word-aligned
	   10bit ADD PC/SP word-aligned

	 The type of instruction being processed is encoded in the
	 instruction field:

	   0x8000  SUB
	   0x00F0  Rd
	   0x000F  Rs
      */
      newval = md_chars_to_number (buf, THUMB_SIZE);
      {
	int rd = (newval >> 4) & 0xf;
	int rs = newval & 0xf;
	int subtract = !!(newval & 0x8000);

	/* Check for HI regs, only very restricted cases allowed:
	   Adjusting SP, and using PC or SP to get an address.	*/
	if ((rd > 7 && (rd != REG_SP || rs != REG_SP))
	    || (rs > 7 && rs != REG_SP && rs != REG_PC))
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("invalid Hi register with immediate"));

	/* If value is negative, choose the opposite instruction.  */
	if ((offsetT) value < 0)
	  {
	    value = -value;
	    subtract = !subtract;
	    if ((offsetT) value < 0)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("immediate value out of range"));
	  }

	if (rd == REG_SP)
	  {
 	    if (value & ~0x1fc)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("invalid immediate for stack address calculation"));
	    newval = subtract ? T_OPCODE_SUB_ST : T_OPCODE_ADD_ST;
	    newval |= value >> 2;
	  }
	else if (rs == REG_PC || rs == REG_SP)
	  {
	    /* PR gas/18541.  If the addition is for a defined symbol
	       within range of an ADR instruction then accept it.  */
	    if (subtract
		&& value == 4
		&& fixP->fx_addsy != NULL)
	      {
		subtract = 0;

		if (! S_IS_DEFINED (fixP->fx_addsy)
		    || S_GET_SEGMENT (fixP->fx_addsy) != seg
		    || S_IS_WEAK (fixP->fx_addsy))
		  {
		    as_bad_where (fixP->fx_file, fixP->fx_line,
				  _("address calculation needs a strongly defined nearby symbol"));
		  }
		else
		  {
		    offsetT v = fixP->fx_where + fixP->fx_frag->fr_address;

		    /* Round up to the next 4-byte boundary.  */
		    if (v & 3)
		      v = (v + 3) & ~ 3;
		    else
		      v += 4;
		    v = S_GET_VALUE (fixP->fx_addsy) - v;

		    if (v & ~0x3fc)
		      {
			as_bad_where (fixP->fx_file, fixP->fx_line,
				      _("symbol too far away"));
		      }
		    else
		      {
			fixP->fx_done = 1;
			value = v;
		      }
		  }
	      }

	    if (subtract || value & ~0x3fc)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("invalid immediate for address calculation (value = 0x%08lX)"),
			    (unsigned long) (subtract ? - value : value));
	    newval = (rs == REG_PC ? T_OPCODE_ADD_PC : T_OPCODE_ADD_SP);
	    newval |= rd << 8;
	    newval |= value >> 2;
	  }
	else if (rs == rd)
	  {
	    if (value & ~0xff)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("immediate value out of range"));
	    newval = subtract ? T_OPCODE_SUB_I8 : T_OPCODE_ADD_I8;
	    newval |= (rd << 8) | value;
	  }
	else
	  {
	    if (value & ~0x7)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("immediate value out of range"));
	    newval = subtract ? T_OPCODE_SUB_I3 : T_OPCODE_ADD_I3;
	    newval |= rd | (rs << 3) | (value << 6);
	  }
      }
      md_number_to_chars (buf, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_THUMB_IMM:
      newval = md_chars_to_number (buf, THUMB_SIZE);
      if (value > 255)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid immediate: %ld is out of range"),
		      (long) value);
      newval |= value;
      md_number_to_chars (buf, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_THUMB_SHIFT:
      /* 5bit shift value (0..32).  LSL cannot take 32.	 */
      newval = md_chars_to_number (buf, THUMB_SIZE) & 0xf83f;
      temp = newval & 0xf800;
      if (value > 32 || (value == 32 && temp == T_OPCODE_LSL_I))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid shift value: %ld"), (long) value);
      /* Shifts of zero must be encoded as LSL.	 */
      if (value == 0)
	newval = (newval & 0x003f) | T_OPCODE_LSL_I;
      /* Shifts of 32 are encoded as zero.  */
      else if (value == 32)
	value = 0;
      newval |= value << 6;
      md_number_to_chars (buf, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = 0;
      return;

    case BFD_RELOC_ARM_MOVW:
    case BFD_RELOC_ARM_MOVT:
    case BFD_RELOC_ARM_THUMB_MOVW:
    case BFD_RELOC_ARM_THUMB_MOVT:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  /* REL format relocations are limited to a 16-bit addend.  */
	  if (!fixP->fx_done)
	    {
	      if (value + 0x8000 > 0x7fff + 0x8000)
		  as_bad_where (fixP->fx_file, fixP->fx_line,
				_("offset out of range"));
	    }
	  else if (fixP->fx_r_type == BFD_RELOC_ARM_MOVT
		   || fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVT)
	    {
	      value >>= 16;
	    }

	  if (fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVW
	      || fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVT)
	    {
	      newval = get_thumb32_insn (buf);
	      newval &= 0xfbf08f00;
	      newval |= (value & 0xf000) << 4;
	      newval |= (value & 0x0800) << 15;
	      newval |= (value & 0x0700) << 4;
	      newval |= (value & 0x00ff);
	      put_thumb32_insn (buf, newval);
	    }
	  else
	    {
	      newval = md_chars_to_number (buf, 4);
	      newval &= 0xfff0f000;
	      newval |= value & 0x0fff;
	      newval |= (value & 0xf000) << 4;
	      md_number_to_chars (buf, newval, 4);
	    }
	}
      return;

   case BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC:
   case BFD_RELOC_ARM_THUMB_ALU_ABS_G1_NC:
   case BFD_RELOC_ARM_THUMB_ALU_ABS_G2_NC:
   case BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC:
      gas_assert (!fixP->fx_done);
      {
	bfd_vma insn;
	bool is_mov;
	bfd_vma encoded_addend = value;

	/* Check that addend can be encoded in instruction.  */
	if (!seg->use_rela_p && value > 255)
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("the offset 0x%08lX is not representable"),
			(unsigned long) encoded_addend);

	/* Extract the instruction.  */
	insn = md_chars_to_number (buf, THUMB_SIZE);
	is_mov = (insn & 0xf800) == 0x2000;

	/* Encode insn.  */
	if (is_mov)
	  {
	    if (!seg->use_rela_p)
	      insn |= encoded_addend;
	  }
	else
	  {
	    int rd, rs;

	    /* Extract the instruction.  */
	     /* Encoding is the following
		0x8000  SUB
		0x00F0  Rd
		0x000F  Rs
	     */
	     /* The following conditions must be true :
		- ADD
		- Rd == Rs
		- Rd <= 7
	     */
	    rd = (insn >> 4) & 0xf;
	    rs = insn & 0xf;
	    if ((insn & 0x8000) || (rd != rs) || rd > 7)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			_("Unable to process relocation for thumb opcode: %lx"),
			(unsigned long) insn);

	    /* Encode as ADD immediate8 thumb 1 code.  */
	    insn = 0x3000 | (rd << 8);

	    /* Place the encoded addend into the first 8 bits of the
	       instruction.  */
	    if (!seg->use_rela_p)
	      insn |= encoded_addend;
	  }

	/* Update the instruction.  */
	md_number_to_chars (buf, insn, THUMB_SIZE);
      }
      break;

   case BFD_RELOC_ARM_ALU_PC_G0_NC:
   case BFD_RELOC_ARM_ALU_PC_G0:
   case BFD_RELOC_ARM_ALU_PC_G1_NC:
   case BFD_RELOC_ARM_ALU_PC_G1:
   case BFD_RELOC_ARM_ALU_PC_G2:
   case BFD_RELOC_ARM_ALU_SB_G0_NC:
   case BFD_RELOC_ARM_ALU_SB_G0:
   case BFD_RELOC_ARM_ALU_SB_G1_NC:
   case BFD_RELOC_ARM_ALU_SB_G1:
   case BFD_RELOC_ARM_ALU_SB_G2:
     gas_assert (!fixP->fx_done);
     if (!seg->use_rela_p)
       {
	 bfd_vma insn;
	 bfd_vma encoded_addend;
	 bfd_vma addend_abs = llabs ((offsetT) value);

	 /* Check that the absolute value of the addend can be
	    expressed as an 8-bit constant plus a rotation.  */
	 encoded_addend = encode_arm_immediate (addend_abs);
	 if (encoded_addend == (unsigned int) FAIL)
	   as_bad_where (fixP->fx_file, fixP->fx_line,
			 _("the offset 0x%08lX is not representable"),
			 (unsigned long) addend_abs);

	 /* Extract the instruction.  */
	 insn = md_chars_to_number (buf, INSN_SIZE);

	 /* If the addend is positive, use an ADD instruction.
	    Otherwise use a SUB.  Take care not to destroy the S bit.  */
	 insn &= 0xff1fffff;
	 if ((offsetT) value < 0)
	   insn |= 1 << 22;
	 else
	   insn |= 1 << 23;

	 /* Place the encoded addend into the first 12 bits of the
	    instruction.  */
	 insn &= 0xfffff000;
	 insn |= encoded_addend;

	 /* Update the instruction.  */
	 md_number_to_chars (buf, insn, INSN_SIZE);
       }
     break;

    case BFD_RELOC_ARM_LDR_PC_G0:
    case BFD_RELOC_ARM_LDR_PC_G1:
    case BFD_RELOC_ARM_LDR_PC_G2:
    case BFD_RELOC_ARM_LDR_SB_G0:
    case BFD_RELOC_ARM_LDR_SB_G1:
    case BFD_RELOC_ARM_LDR_SB_G2:
      gas_assert (!fixP->fx_done);
      if (!seg->use_rela_p)
	{
	  bfd_vma insn;
	  bfd_vma addend_abs = llabs ((offsetT) value);

	  /* Check that the absolute value of the addend can be
	     encoded in 12 bits.  */
	  if (addend_abs >= 0x1000)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad offset 0x%08lX (only 12 bits available for the magnitude)"),
			  (unsigned long) addend_abs);

	  /* Extract the instruction.  */
	  insn = md_chars_to_number (buf, INSN_SIZE);

	  /* If the addend is negative, clear bit 23 of the instruction.
	     Otherwise set it.  */
	  if ((offsetT) value < 0)
	    insn &= ~(1 << 23);
	  else
	    insn |= 1 << 23;

	  /* Place the absolute value of the addend into the first 12 bits
	     of the instruction.  */
	  insn &= 0xfffff000;
	  insn |= addend_abs;

	  /* Update the instruction.  */
	  md_number_to_chars (buf, insn, INSN_SIZE);
	}
      break;

    case BFD_RELOC_ARM_LDRS_PC_G0:
    case BFD_RELOC_ARM_LDRS_PC_G1:
    case BFD_RELOC_ARM_LDRS_PC_G2:
    case BFD_RELOC_ARM_LDRS_SB_G0:
    case BFD_RELOC_ARM_LDRS_SB_G1:
    case BFD_RELOC_ARM_LDRS_SB_G2:
      gas_assert (!fixP->fx_done);
      if (!seg->use_rela_p)
	{
	  bfd_vma insn;
	  bfd_vma addend_abs = llabs ((offsetT) value);

	  /* Check that the absolute value of the addend can be
	     encoded in 8 bits.  */
	  if (addend_abs >= 0x100)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad offset 0x%08lX (only 8 bits available for the magnitude)"),
			  (unsigned long) addend_abs);

	  /* Extract the instruction.  */
	  insn = md_chars_to_number (buf, INSN_SIZE);

	  /* If the addend is negative, clear bit 23 of the instruction.
	     Otherwise set it.  */
	  if ((offsetT) value < 0)
	    insn &= ~(1 << 23);
	  else
	    insn |= 1 << 23;

	  /* Place the first four bits of the absolute value of the addend
	     into the first 4 bits of the instruction, and the remaining
	     four into bits 8 .. 11.  */
	  insn &= 0xfffff0f0;
	  insn |= (addend_abs & 0xf) | ((addend_abs & 0xf0) << 4);

	  /* Update the instruction.  */
	  md_number_to_chars (buf, insn, INSN_SIZE);
	}
      break;

    case BFD_RELOC_ARM_LDC_PC_G0:
    case BFD_RELOC_ARM_LDC_PC_G1:
    case BFD_RELOC_ARM_LDC_PC_G2:
    case BFD_RELOC_ARM_LDC_SB_G0:
    case BFD_RELOC_ARM_LDC_SB_G1:
    case BFD_RELOC_ARM_LDC_SB_G2:
      gas_assert (!fixP->fx_done);
      if (!seg->use_rela_p)
	{
	  bfd_vma insn;
	  bfd_vma addend_abs = llabs ((offsetT) value);

	  /* Check that the absolute value of the addend is a multiple of
	     four and, when divided by four, fits in 8 bits.  */
	  if (addend_abs & 0x3)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad offset 0x%08lX (must be word-aligned)"),
			  (unsigned long) addend_abs);

	  if ((addend_abs >> 2) > 0xff)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad offset 0x%08lX (must be an 8-bit number of words)"),
			  (unsigned long) addend_abs);

	  /* Extract the instruction.  */
	  insn = md_chars_to_number (buf, INSN_SIZE);

	  /* If the addend is negative, clear bit 23 of the instruction.
	     Otherwise set it.  */
	  if ((offsetT) value < 0)
	    insn &= ~(1 << 23);
	  else
	    insn |= 1 << 23;

	  /* Place the addend (divided by four) into the first eight
	     bits of the instruction.  */
	  insn &= 0xfffffff0;
	  insn |= addend_abs >> 2;

	  /* Update the instruction.  */
	  md_number_to_chars (buf, insn, INSN_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH5:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 5 bits wide.  */
	  fixP->fx_done = 0;
	}
      if (v8_1_branch_value_check (value, 5, false) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  addressT boff = value >> 1;

	  newval  = md_chars_to_number (buf, THUMB_SIZE);
	  newval |= (boff << 7);
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BFCSEL:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  fixP->fx_done = 0;
	}
      if ((value & ~0x7f) && ((value & ~0x3f) != (valueT) ~0x3f))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("branch out of range"));

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval  = md_chars_to_number (buf, THUMB_SIZE);

	  addressT boff = ((newval & 0x0780) >> 7) << 1;
	  addressT diff = value - boff;

	  if (diff == 4)
	    {
	      newval |= 1 << 1; /* T bit.  */
	    }
	  else if (diff != 2)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("out of range label-relative fixup value"));
	    }
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_THUMB_BF17:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 17 bits wide.  */
	  fixP->fx_done = 0;
	}

      if (v8_1_branch_value_check (value, 17, true) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  offsetT newval2;
	  addressT immA, immB, immC;

	  immA = (value & 0x0001f000) >> 12;
	  immB = (value & 0x00000ffc) >> 2;
	  immC = (value & 0x00000002) >> 1;

	  newval   = md_chars_to_number (buf, THUMB_SIZE);
	  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval  |= immA;
	  newval2 |= (immC << 11) | (immB << 1);
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_THUMB_BF19:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 19 bits wide.  */
	  fixP->fx_done = 0;
	}

      if (v8_1_branch_value_check (value, 19, true) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  offsetT newval2;
	  addressT immA, immB, immC;

	  immA = (value & 0x0007f000) >> 12;
	  immB = (value & 0x00000ffc) >> 2;
	  immC = (value & 0x00000002) >> 1;

	  newval   = md_chars_to_number (buf, THUMB_SIZE);
	  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval  |= immA;
	  newval2 |= (immC << 11) | (immB << 1);
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_THUMB_BF13:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 13 bits wide.  */
	  fixP->fx_done = 0;
	}

      if (v8_1_branch_value_check (value, 13, true) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  offsetT newval2;
	  addressT immA, immB, immC;

	  immA = (value & 0x00001000) >> 12;
	  immB = (value & 0x00000ffc) >> 2;
	  immC = (value & 0x00000002) >> 1;

	  newval   = md_chars_to_number (buf, THUMB_SIZE);
	  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval  |= immA;
	  newval2 |= (immC << 11) | (immB << 1);
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_THUMB_LOOP12:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 12 bits wide.  */
	  fixP->fx_done = 0;
	}

      bfd_vma insn = get_thumb32_insn (buf);
      /* le lr, <label>, le <label> or letp lr, <label> */
      if (((insn & 0xffffffff) == 0xf00fc001)
	  || ((insn & 0xffffffff) == 0xf02fc001)
	  || ((insn & 0xffffffff) == 0xf01fc001))
	value = -value;

      if (v8_1_branch_value_check (value, 12, false) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  addressT imml, immh;

	  immh = (value & 0x00000ffc) >> 2;
	  imml = (value & 0x00000002) >> 1;

	  newval  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval |= (imml << 11) | (immh << 1);
	  md_number_to_chars (buf + THUMB_SIZE, newval, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_V4BX:
      /* This will need to go in the object file.  */
      fixP->fx_done = 0;
      break;

    case BFD_RELOC_UNUSED:
    default:
      as_bad_where (fixP->fx_file, fixP->fx_line,
		    _("bad relocation fixup type (%d)"), fixP->fx_r_type);
    }
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection *section, fixS *fixp)
{
  arelent * reloc;
  bfd_reloc_code_real_type code;

  reloc = XNEW (arelent);

  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  if (fixp->fx_pcrel)
    {
      if (section->use_rela_p)
	fixp->fx_offset -= md_pcrel_from_section (fixp, section);
      else
	fixp->fx_offset = reloc->address;
    }
  reloc->addend = fixp->fx_offset;

  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_8:
      if (fixp->fx_pcrel)
	{
	  code = BFD_RELOC_8_PCREL;
	  break;
	}
      /* Fall through.  */

    case BFD_RELOC_16:
      if (fixp->fx_pcrel)
	{
	  code = BFD_RELOC_16_PCREL;
	  break;
	}
      /* Fall through.  */

    case BFD_RELOC_32:
      if (fixp->fx_pcrel)
	{
	  code = BFD_RELOC_32_PCREL;
	  break;
	}
      /* Fall through.  */

    case BFD_RELOC_ARM_MOVW:
      if (fixp->fx_pcrel)
	{
	  code = BFD_RELOC_ARM_MOVW_PCREL;
	  break;
	}
      /* Fall through.  */

    case BFD_RELOC_ARM_MOVT:
      if (fixp->fx_pcrel)
	{
	  code = BFD_RELOC_ARM_MOVT_PCREL;
	  break;
	}
      /* Fall through.  */

    case BFD_RELOC_ARM_THUMB_MOVW:
      if (fixp->fx_pcrel)
	{
	  code = BFD_RELOC_ARM_THUMB_MOVW_PCREL;
	  break;
	}
      /* Fall through.  */

    case BFD_RELOC_ARM_THUMB_MOVT:
      if (fixp->fx_pcrel)
	{
	  code = BFD_RELOC_ARM_THUMB_MOVT_PCREL;
	  break;
	}
      /* Fall through.  */

    case BFD_RELOC_NONE:
    case BFD_RELOC_ARM_PCREL_BRANCH:
    case BFD_RELOC_ARM_PCREL_BLX:
    case BFD_RELOC_RVA:
    case BFD_RELOC_THUMB_PCREL_BRANCH7:
    case BFD_RELOC_THUMB_PCREL_BRANCH9:
    case BFD_RELOC_THUMB_PCREL_BRANCH12:
    case BFD_RELOC_THUMB_PCREL_BRANCH20:
    case BFD_RELOC_THUMB_PCREL_BRANCH23:
    case BFD_RELOC_THUMB_PCREL_BRANCH25:
    case BFD_RELOC_VTABLE_ENTRY:
    case BFD_RELOC_VTABLE_INHERIT:
#ifdef TE_PE
    case BFD_RELOC_32_SECREL:
#endif
      code = fixp->fx_r_type;
      break;

    case BFD_RELOC_THUMB_PCREL_BLX:
#ifdef OBJ_ELF
      if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4)
	code = BFD_RELOC_THUMB_PCREL_BRANCH23;
      else
#endif
	code = BFD_RELOC_THUMB_PCREL_BLX;
      break;

    case BFD_RELOC_ARM_LITERAL:
    case BFD_RELOC_ARM_HWLITERAL:
      /* If this is called then the a literal has
	 been referenced across a section boundary.  */
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("literal referenced across section boundary"));
      return NULL;

#ifdef OBJ_ELF
    case BFD_RELOC_ARM_TLS_CALL:
    case BFD_RELOC_ARM_THM_TLS_CALL:
    case BFD_RELOC_ARM_TLS_DESCSEQ:
    case BFD_RELOC_ARM_THM_TLS_DESCSEQ:
    case BFD_RELOC_ARM_GOT32:
    case BFD_RELOC_ARM_GOTOFF:
    case BFD_RELOC_ARM_GOT_PREL:
    case BFD_RELOC_ARM_PLT32:
    case BFD_RELOC_ARM_TARGET1:
    case BFD_RELOC_ARM_ROSEGREL32:
    case BFD_RELOC_ARM_SBREL32:
    case BFD_RELOC_ARM_PREL31:
    case BFD_RELOC_ARM_TARGET2:
    case BFD_RELOC_ARM_TLS_LDO32:
    case BFD_RELOC_ARM_PCREL_CALL:
    case BFD_RELOC_ARM_PCREL_JUMP:
    case BFD_RELOC_ARM_ALU_PC_G0_NC:
    case BFD_RELOC_ARM_ALU_PC_G0:
    case BFD_RELOC_ARM_ALU_PC_G1_NC:
    case BFD_RELOC_ARM_ALU_PC_G1:
    case BFD_RELOC_ARM_ALU_PC_G2:
    case BFD_RELOC_ARM_LDR_PC_G0:
    case BFD_RELOC_ARM_LDR_PC_G1:
    case BFD_RELOC_ARM_LDR_PC_G2:
    case BFD_RELOC_ARM_LDRS_PC_G0:
    case BFD_RELOC_ARM_LDRS_PC_G1:
    case BFD_RELOC_ARM_LDRS_PC_G2:
    case BFD_RELOC_ARM_LDC_PC_G0:
    case BFD_RELOC_ARM_LDC_PC_G1:
    case BFD_RELOC_ARM_LDC_PC_G2:
    case BFD_RELOC_ARM_ALU_SB_G0_NC:
    case BFD_RELOC_ARM_ALU_SB_G0:
    case BFD_RELOC_ARM_ALU_SB_G1_NC:
    case BFD_RELOC_ARM_ALU_SB_G1:
    case BFD_RELOC_ARM_ALU_SB_G2:
    case BFD_RELOC_ARM_LDR_SB_G0:
    case BFD_RELOC_ARM_LDR_SB_G1:
    case BFD_RELOC_ARM_LDR_SB_G2:
    case BFD_RELOC_ARM_LDRS_SB_G0:
    case BFD_RELOC_ARM_LDRS_SB_G1:
    case BFD_RELOC_ARM_LDRS_SB_G2:
    case BFD_RELOC_ARM_LDC_SB_G0:
    case BFD_RELOC_ARM_LDC_SB_G1:
    case BFD_RELOC_ARM_LDC_SB_G2:
    case BFD_RELOC_ARM_V4BX:
    case BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC:
    case BFD_RELOC_ARM_THUMB_ALU_ABS_G1_NC:
    case BFD_RELOC_ARM_THUMB_ALU_ABS_G2_NC:
    case BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC:
    case BFD_RELOC_ARM_GOTFUNCDESC:
    case BFD_RELOC_ARM_GOTOFFFUNCDESC:
    case BFD_RELOC_ARM_FUNCDESC:
    case BFD_RELOC_ARM_THUMB_BF17:
    case BFD_RELOC_ARM_THUMB_BF19:
    case BFD_RELOC_ARM_THUMB_BF13:
      code = fixp->fx_r_type;
      break;

    case BFD_RELOC_ARM_TLS_GOTDESC:
    case BFD_RELOC_ARM_TLS_GD32:
    case BFD_RELOC_ARM_TLS_GD32_FDPIC:
    case BFD_RELOC_ARM_TLS_LE32:
    case BFD_RELOC_ARM_TLS_IE32:
    case BFD_RELOC_ARM_TLS_IE32_FDPIC:
    case BFD_RELOC_ARM_TLS_LDM32:
    case BFD_RELOC_ARM_TLS_LDM32_FDPIC:
      /* BFD will include the symbol's address in the addend.
	 But we don't want that, so subtract it out again here.  */
      if (!S_IS_COMMON (fixp->fx_addsy))
	reloc->addend -= (*reloc->sym_ptr_ptr)->value;
      code = fixp->fx_r_type;
      break;
#endif

    case BFD_RELOC_ARM_IMMEDIATE:
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("internal relocation (type: IMMEDIATE) not fixed up"));
      return NULL;

    case BFD_RELOC_ARM_ADRL_IMMEDIATE:
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("ADRL used for a symbol not defined in the same file"));
      return NULL;

    case BFD_RELOC_THUMB_PCREL_BRANCH5:
    case BFD_RELOC_THUMB_PCREL_BFCSEL:
    case BFD_RELOC_ARM_THUMB_LOOP12:
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("%s used for a symbol not defined in the same file"),
		    bfd_get_reloc_code_name (fixp->fx_r_type));
      return NULL;

    case BFD_RELOC_ARM_OFFSET_IMM:
      if (section->use_rela_p)
	{
	  code = fixp->fx_r_type;
	  break;
	}

      if (fixp->fx_addsy != NULL
	  && !S_IS_DEFINED (fixp->fx_addsy)
	  && S_IS_LOCAL (fixp->fx_addsy))
	{
	  as_bad_where (fixp->fx_file, fixp->fx_line,
			_("undefined local label `%s'"),
			S_GET_NAME (fixp->fx_addsy));
	  return NULL;
	}

      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("internal_relocation (type: OFFSET_IMM) not fixed up"));
      return NULL;

    default:
      {
	const char * type;

	switch (fixp->fx_r_type)
	  {
	  case BFD_RELOC_NONE:		   type = "NONE";	  break;
	  case BFD_RELOC_ARM_OFFSET_IMM8:  type = "OFFSET_IMM8";  break;
	  case BFD_RELOC_ARM_SHIFT_IMM:	   type = "SHIFT_IMM";	  break;
	  case BFD_RELOC_ARM_SMC:	   type = "SMC";	  break;
	  case BFD_RELOC_ARM_SWI:	   type = "SWI";	  break;
	  case BFD_RELOC_ARM_MULTI:	   type = "MULTI";	  break;
	  case BFD_RELOC_ARM_CP_OFF_IMM:   type = "CP_OFF_IMM";	  break;
	  case BFD_RELOC_ARM_T32_OFFSET_IMM: type = "T32_OFFSET_IMM"; break;
	  case BFD_RELOC_ARM_T32_CP_OFF_IMM: type = "T32_CP_OFF_IMM"; break;
	  case BFD_RELOC_ARM_THUMB_ADD:	   type = "THUMB_ADD";	  break;
	  case BFD_RELOC_ARM_THUMB_SHIFT:  type = "THUMB_SHIFT";  break;
	  case BFD_RELOC_ARM_THUMB_IMM:	   type = "THUMB_IMM";	  break;
	  case BFD_RELOC_ARM_THUMB_OFFSET: type = "THUMB_OFFSET"; break;
	  default:			   type = _("<unknown>"); break;
	  }
	as_bad_where (fixp->fx_file, fixp->fx_line,
		      _("cannot represent %s relocation in this object file format"),
		      type);
	return NULL;
      }
    }

#ifdef OBJ_ELF
  if ((code == BFD_RELOC_32_PCREL || code == BFD_RELOC_32)
      && GOT_symbol
      && fixp->fx_addsy == GOT_symbol)
    {
      code = BFD_RELOC_ARM_GOTPC;
      reloc->addend = fixp->fx_offset = reloc->address;
    }
#endif

  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);

  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("cannot represent %s relocation in this object file format"),
		    bfd_get_reloc_code_name (code));
      return NULL;
    }

  /* HACK: Since arm ELF uses Rel instead of Rela, encode the
     vtable entry to be used in the relocation's section offset.  */
  if (fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    reloc->address = fixp->fx_offset;

  return reloc;
}

/* This fix_new is called by cons via TC_CONS_FIX_NEW.	*/

void
cons_fix_new_arm (fragS *	frag,
		  int		where,
		  int		size,
		  expressionS * exp,
		  bfd_reloc_code_real_type reloc)
{
  int pcrel = 0;

  /* Pick a reloc.
     FIXME: @@ Should look at CPU word size.  */
  switch (size)
    {
    case 1:
      reloc = BFD_RELOC_8;
      break;
    case 2:
      reloc = BFD_RELOC_16;
      break;
    case 4:
    default:
      reloc = BFD_RELOC_32;
      break;
    case 8:
      reloc = BFD_RELOC_64;
      break;
    }

#ifdef TE_PE
  if (exp->X_op == O_secrel)
  {
    exp->X_op = O_symbol;
    reloc = BFD_RELOC_32_SECREL;
  }
#endif

  fix_new_exp (frag, where, size, exp, pcrel, reloc);
}

#if defined (OBJ_COFF)
void
arm_validate_fix (fixS * fixP)
{
  /* If the destination of the branch is a defined symbol which does not have
     the THUMB_FUNC attribute, then we must be calling a function which has
     the (interfacearm) attribute.  We look for the Thumb entry point to that
     function and change the branch to refer to that function instead.	*/
  if (fixP->fx_r_type == BFD_RELOC_THUMB_PCREL_BRANCH23
      && fixP->fx_addsy != NULL
      && S_IS_DEFINED (fixP->fx_addsy)
      && ! THUMB_IS_FUNC (fixP->fx_addsy))
    {
      fixP->fx_addsy = find_real_start (fixP->fx_addsy);
    }
}
#endif


int
arm_force_relocation (struct fix * fixp)
{
#if defined (OBJ_COFF) && defined (TE_PE)
  if (fixp->fx_r_type == BFD_RELOC_RVA)
    return 1;
#endif

  /* In case we have a call or a branch to a function in ARM ISA mode from
     a thumb function or vice-versa force the relocation. These relocations
     are cleared off for some cores that might have blx and simple transformations
     are possible.  */

#ifdef OBJ_ELF
  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_ARM_PCREL_JUMP:
    case BFD_RELOC_ARM_PCREL_CALL:
    case BFD_RELOC_THUMB_PCREL_BLX:
      if (THUMB_IS_FUNC (fixp->fx_addsy))
	return 1;
      break;

    case BFD_RELOC_ARM_PCREL_BLX:
    case BFD_RELOC_THUMB_PCREL_BRANCH25:
    case BFD_RELOC_THUMB_PCREL_BRANCH20:
    case BFD_RELOC_THUMB_PCREL_BRANCH23:
      if (ARM_IS_FUNC (fixp->fx_addsy))
	return 1;
      break;

    default:
      break;
    }
#endif

  /* Resolve these relocations even if the symbol is extern or weak.
     Technically this is probably wrong due to symbol preemption.
     In practice these relocations do not have enough range to be useful
     at dynamic link time, and some code (e.g. in the Linux kernel)
     expects these references to be resolved.  */
  if (fixp->fx_r_type == BFD_RELOC_ARM_IMMEDIATE
      || fixp->fx_r_type == BFD_RELOC_ARM_OFFSET_IMM
      || fixp->fx_r_type == BFD_RELOC_ARM_OFFSET_IMM8
      || fixp->fx_r_type == BFD_RELOC_ARM_ADRL_IMMEDIATE
      || fixp->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM
      || fixp->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM_S2
      || fixp->fx_r_type == BFD_RELOC_ARM_THUMB_OFFSET
      || fixp->fx_r_type == BFD_RELOC_THUMB_PCREL_BRANCH12
      || fixp->fx_r_type == BFD_RELOC_ARM_T32_ADD_IMM
      || fixp->fx_r_type == BFD_RELOC_ARM_T32_IMMEDIATE
      || fixp->fx_r_type == BFD_RELOC_ARM_T32_IMM12
      || fixp->fx_r_type == BFD_RELOC_ARM_T32_OFFSET_IMM
      || fixp->fx_r_type == BFD_RELOC_ARM_T32_ADD_PC12
      || fixp->fx_r_type == BFD_RELOC_ARM_T32_CP_OFF_IMM
      || fixp->fx_r_type == BFD_RELOC_ARM_T32_CP_OFF_IMM_S2)
    return 0;

  /* Always leave these relocations for the linker.  */
  if ((fixp->fx_r_type >= BFD_RELOC_ARM_ALU_PC_G0_NC
       && fixp->fx_r_type <= BFD_RELOC_ARM_LDC_SB_G2)
      || fixp->fx_r_type == BFD_RELOC_ARM_LDR_PC_G0)
    return 1;

  /* Always generate relocations against function symbols.  */
  if (fixp->fx_r_type == BFD_RELOC_32
      && fixp->fx_addsy
      && (symbol_get_bfdsym (fixp->fx_addsy)->flags & BSF_FUNCTION))
    return 1;

  return generic_force_reloc (fixp);
}

#if defined (OBJ_ELF) || defined (OBJ_COFF)
/* Relocations against function names must be left unadjusted,
   so that the linker can use this information to generate interworking
   stubs.  The MIPS version of this function
   also prevents relocations that are mips-16 specific, but I do not
   know why it does this.

   FIXME:
   There is one other problem that ought to be addressed here, but
   which currently is not:  Taking the address of a label (rather
   than a function) and then later jumping to that address.  Such
   addresses also ought to have their bottom bit set (assuming that
   they reside in Thumb code), but at the moment they will not.	 */

bool
arm_fix_adjustable (fixS * fixP)
{
  if (fixP->fx_addsy == NULL)
    return 1;

  /* Preserve relocations against symbols with function type.  */
  if (symbol_get_bfdsym (fixP->fx_addsy)->flags & BSF_FUNCTION)
    return false;

  if (THUMB_IS_FUNC (fixP->fx_addsy)
      && fixP->fx_subsy == NULL)
    return false;

  /* We need the symbol name for the VTABLE entries.  */
  if (	 fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return false;

  /* Don't allow symbols to be discarded on GOT related relocs.	 */
  if (fixP->fx_r_type == BFD_RELOC_ARM_PLT32
      || fixP->fx_r_type == BFD_RELOC_ARM_GOT32
      || fixP->fx_r_type == BFD_RELOC_ARM_GOTOFF
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_GD32
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_GD32_FDPIC
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_LE32
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_IE32
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_IE32_FDPIC
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_LDM32
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_LDM32_FDPIC
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_LDO32
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_GOTDESC
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_CALL
      || fixP->fx_r_type == BFD_RELOC_ARM_THM_TLS_CALL
      || fixP->fx_r_type == BFD_RELOC_ARM_TLS_DESCSEQ
      || fixP->fx_r_type == BFD_RELOC_ARM_THM_TLS_DESCSEQ
      || fixP->fx_r_type == BFD_RELOC_ARM_TARGET2)
    return false;

  /* Similarly for group relocations.  */
  if ((fixP->fx_r_type >= BFD_RELOC_ARM_ALU_PC_G0_NC
       && fixP->fx_r_type <= BFD_RELOC_ARM_LDC_SB_G2)
      || fixP->fx_r_type == BFD_RELOC_ARM_LDR_PC_G0)
    return false;

  /* MOVW/MOVT REL relocations have limited offsets, so keep the symbols.  */
  if (fixP->fx_r_type == BFD_RELOC_ARM_MOVW
      || fixP->fx_r_type == BFD_RELOC_ARM_MOVT
      || fixP->fx_r_type == BFD_RELOC_ARM_MOVW_PCREL
      || fixP->fx_r_type == BFD_RELOC_ARM_MOVT_PCREL
      || fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVW
      || fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVT
      || fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVW_PCREL
      || fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVT_PCREL)
    return false;

  /* BFD_RELOC_ARM_THUMB_ALU_ABS_Gx_NC relocations have VERY limited
     offsets, so keep these symbols.  */
  if (fixP->fx_r_type >= BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC
      && fixP->fx_r_type <= BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC)
    return false;

  return true;
}
#endif /* defined (OBJ_ELF) || defined (OBJ_COFF) */

#ifdef OBJ_ELF
const char *
elf32_arm_target_format (void)
{
#if defined (TE_VXWORKS)
  return (target_big_endian
	  ? "elf32-bigarm-vxworks"
	  : "elf32-littlearm-vxworks");
#elif defined (TE_NACL)
  return (target_big_endian
	  ? "elf32-bigarm-nacl"
	  : "elf32-littlearm-nacl");
#else
  if (arm_fdpic)
    {
      if (target_big_endian)
	return "elf32-bigarm-fdpic";
      else
	return "elf32-littlearm-fdpic";
    }
  else
    {
      if (target_big_endian)
	return "elf32-bigarm";
      else
	return "elf32-littlearm";
    }
#endif
}

void
armelf_frob_symbol (symbolS * symp,
		    int *     puntp)
{
  elf_frob_symbol (symp, puntp);
}
#endif

/* MD interface: Finalization.	*/

void
arm_cleanup (void)
{
  literal_pool * pool;

  /* Ensure that all the predication blocks are properly closed.  */
  check_pred_blocks_finished ();

  for (pool = list_of_pools; pool; pool = pool->next)
    {
      /* Put it at the end of the relevant section.  */
      subseg_set (pool->section, pool->sub_section);
#ifdef OBJ_ELF
      arm_elf_change_section ();
#endif
      s_ltorg (0);
    }
}

#ifdef OBJ_ELF
/* Remove any excess mapping symbols generated for alignment frags in
   SEC.  We may have created a mapping symbol before a zero byte
   alignment; remove it if there's a mapping symbol after the
   alignment.  */
static void
check_mapping_symbols (bfd *abfd ATTRIBUTE_UNUSED, asection *sec,
		       void *dummy ATTRIBUTE_UNUSED)
{
  segment_info_type *seginfo = seg_info (sec);
  fragS *fragp;

  if (seginfo == NULL || seginfo->frchainP == NULL)
    return;

  for (fragp = seginfo->frchainP->frch_root;
       fragp != NULL;
       fragp = fragp->fr_next)
    {
      symbolS *sym = fragp->tc_frag_data.last_map;
      fragS *next = fragp->fr_next;

      /* Variable-sized frags have been converted to fixed size by
	 this point.  But if this was variable-sized to start with,
	 there will be a fixed-size frag after it.  So don't handle
	 next == NULL.  */
      if (sym == NULL || next == NULL)
	continue;

      if (S_GET_VALUE (sym) < next->fr_address)
	/* Not at the end of this frag.  */
	continue;
      know (S_GET_VALUE (sym) == next->fr_address);

      do
	{
	  if (next->tc_frag_data.first_map != NULL)
	    {
	      /* Next frag starts with a mapping symbol.  Discard this
		 one.  */
	      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
	      break;
	    }

	  if (next->fr_next == NULL)
	    {
	      /* This mapping symbol is at the end of the section.  Discard
		 it.  */
	      know (next->fr_fix == 0 && next->fr_var == 0);
	      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
	      break;
	    }

	  /* As long as we have empty frags without any mapping symbols,
	     keep looking.  */
	  /* If the next frag is non-empty and does not start with a
	     mapping symbol, then this mapping symbol is required.  */
	  if (next->fr_address != next->fr_next->fr_address)
	    break;

	  next = next->fr_next;
	}
      while (next != NULL);
    }
}
#endif

/* Adjust the symbol table.  This marks Thumb symbols as distinct from
   ARM ones.  */

void
arm_adjust_symtab (void)
{
#ifdef OBJ_COFF
  symbolS * sym;

  for (sym = symbol_rootP; sym != NULL; sym = symbol_next (sym))
    {
      if (ARM_IS_THUMB (sym))
	{
	  if (THUMB_IS_FUNC (sym))
	    {
	      /* Mark the symbol as a Thumb function.  */
	      if (   S_GET_STORAGE_CLASS (sym) == C_STAT
		  || S_GET_STORAGE_CLASS (sym) == C_LABEL)  /* This can happen!	 */
		S_SET_STORAGE_CLASS (sym, C_THUMBSTATFUNC);

	      else if (S_GET_STORAGE_CLASS (sym) == C_EXT)
		S_SET_STORAGE_CLASS (sym, C_THUMBEXTFUNC);
	      else
		as_bad (_("%s: unexpected function type: %d"),
			S_GET_NAME (sym), S_GET_STORAGE_CLASS (sym));
	    }
	  else switch (S_GET_STORAGE_CLASS (sym))
	    {
	    case C_EXT:
	      S_SET_STORAGE_CLASS (sym, C_THUMBEXT);
	      break;
	    case C_STAT:
	      S_SET_STORAGE_CLASS (sym, C_THUMBSTAT);
	      break;
	    case C_LABEL:
	      S_SET_STORAGE_CLASS (sym, C_THUMBLABEL);
	      break;
	    default:
	      /* Do nothing.  */
	      break;
	    }
	}

      if (ARM_IS_INTERWORK (sym))
	coffsymbol (symbol_get_bfdsym (sym))->native->u.syment.n_flags = 0xFF;
    }
#endif
#ifdef OBJ_ELF
  symbolS * sym;
  char	    bind;

  for (sym = symbol_rootP; sym != NULL; sym = symbol_next (sym))
    {
      if (ARM_IS_THUMB (sym))
	{
	  elf_symbol_type * elf_sym;

	  elf_sym = elf_symbol (symbol_get_bfdsym (sym));
	  bind = ELF_ST_BIND (elf_sym->internal_elf_sym.st_info);

	  if (! bfd_is_arm_special_symbol_name (elf_sym->symbol.name,
		BFD_ARM_SPECIAL_SYM_TYPE_ANY))
	    {
	      /* If it's a .thumb_func, declare it as so,
		 otherwise tag label as .code 16.  */
	      if (THUMB_IS_FUNC (sym))
		ARM_SET_SYM_BRANCH_TYPE (elf_sym->internal_elf_sym.st_target_internal,
					 ST_BRANCH_TO_THUMB);
	      else if (EF_ARM_EABI_VERSION (meabi_flags) < EF_ARM_EABI_VER4)
		elf_sym->internal_elf_sym.st_info =
		  ELF_ST_INFO (bind, STT_ARM_16BIT);
	    }
	}
    }

  /* Remove any overlapping mapping symbols generated by alignment frags.  */
  bfd_map_over_sections (stdoutput, check_mapping_symbols, (char *) 0);
  /* Now do generic ELF adjustments.  */
  elf_adjust_symtab ();
#endif
}

/* MD interface: Initialization.  */

static void
set_constant_flonums (void)
{
  int i;

  for (i = 0; i < NUM_FLOAT_VALS; i++)
    if (atof_ieee ((char *) fp_const[i], 'x', fp_values[i]) == NULL)
      abort ();
}

/* Auto-select Thumb mode if it's the only available instruction set for the
   given architecture.  */

static void
autoselect_thumb_from_cpu_variant (void)
{
  if (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v1))
    opcode_select (16);
}

void
md_begin (void)
{
  unsigned mach;
  unsigned int i;

  arm_ops_hsh = str_htab_create ();
  arm_cond_hsh = str_htab_create ();
  arm_vcond_hsh = str_htab_create ();
  arm_shift_hsh = str_htab_create ();
  arm_psr_hsh = str_htab_create ();
  arm_v7m_psr_hsh = str_htab_create ();
  arm_reg_hsh = str_htab_create ();
  arm_reloc_hsh = str_htab_create ();
  arm_barrier_opt_hsh = str_htab_create ();

  for (i = 0; i < sizeof (insns) / sizeof (struct asm_opcode); i++)
    if (str_hash_find (arm_ops_hsh, insns[i].template_name) == NULL)
      str_hash_insert (arm_ops_hsh, insns[i].template_name, insns + i, 0);
  for (i = 0; i < sizeof (conds) / sizeof (struct asm_cond); i++)
    str_hash_insert (arm_cond_hsh, conds[i].template_name, conds + i, 0);
  for (i = 0; i < sizeof (vconds) / sizeof (struct asm_cond); i++)
    str_hash_insert (arm_vcond_hsh, vconds[i].template_name, vconds + i, 0);
  for (i = 0; i < sizeof (shift_names) / sizeof (struct asm_shift_name); i++)
    str_hash_insert (arm_shift_hsh, shift_names[i].name, shift_names + i, 0);
  for (i = 0; i < sizeof (psrs) / sizeof (struct asm_psr); i++)
    str_hash_insert (arm_psr_hsh, psrs[i].template_name, psrs + i, 0);
  for (i = 0; i < sizeof (v7m_psrs) / sizeof (struct asm_psr); i++)
    str_hash_insert (arm_v7m_psr_hsh, v7m_psrs[i].template_name,
		     v7m_psrs + i, 0);
  for (i = 0; i < sizeof (reg_names) / sizeof (struct reg_entry); i++)
    str_hash_insert (arm_reg_hsh, reg_names[i].name, reg_names + i, 0);
  for (i = 0;
       i < sizeof (barrier_opt_names) / sizeof (struct asm_barrier_opt);
       i++)
    str_hash_insert (arm_barrier_opt_hsh, barrier_opt_names[i].template_name,
		     barrier_opt_names + i, 0);
#ifdef OBJ_ELF
  for (i = 0; i < ARRAY_SIZE (reloc_names); i++)
    {
      struct reloc_entry * entry = reloc_names + i;

      if (arm_is_eabi() && entry->reloc == BFD_RELOC_ARM_PLT32)
	/* This makes encode_branch() use the EABI versions of this relocation.  */
	entry->reloc = BFD_RELOC_UNUSED;

      str_hash_insert (arm_reloc_hsh, entry->name, entry, 0);
    }
#endif

  set_constant_flonums ();

  /* Set the cpu variant based on the command-line options.  We prefer
     -mcpu= over -march= if both are set (as for GCC); and we prefer
     -mfpu= over any other way of setting the floating point unit.
     Use of legacy options with new options are faulted.  */
  if (legacy_cpu)
    {
      if (mcpu_cpu_opt || march_cpu_opt)
	as_bad (_("use of old and new-style options to set CPU type"));

      selected_arch = *legacy_cpu;
    }
  else if (mcpu_cpu_opt)
    {
      selected_arch = *mcpu_cpu_opt;
      selected_ext = *mcpu_ext_opt;
    }
  else if (march_cpu_opt)
    {
      selected_arch = *march_cpu_opt;
      selected_ext = *march_ext_opt;
    }
  ARM_MERGE_FEATURE_SETS (selected_cpu, selected_arch, selected_ext);

  if (legacy_fpu)
    {
      if (mfpu_opt)
	as_bad (_("use of old and new-style options to set FPU type"));

      selected_fpu = *legacy_fpu;
    }
  else if (mfpu_opt)
    selected_fpu = *mfpu_opt;
  else
    {
#if !(defined (EABI_DEFAULT) || defined (TE_LINUX) \
	|| defined (TE_NetBSD) || defined (TE_VXWORKS))
      /* Some environments specify a default FPU.  If they don't, infer it
	 from the processor.  */
      if (mcpu_fpu_opt)
	selected_fpu = *mcpu_fpu_opt;
      else if (march_fpu_opt)
	selected_fpu = *march_fpu_opt;
#else
      selected_fpu = fpu_default;
#endif
    }

  if (ARM_FEATURE_ZERO (selected_fpu))
    {
      if (!no_cpu_selected ())
	selected_fpu = fpu_default;
      else
	selected_fpu = fpu_arch_fpa;
    }

#ifdef CPU_DEFAULT
  if (ARM_FEATURE_ZERO (selected_arch))
    {
      selected_arch = cpu_default;
      selected_cpu = selected_arch;
    }
  ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
#else
  /*  Autodection of feature mode: allow all features in cpu_variant but leave
      selected_cpu unset.  It will be set in aeabi_set_public_attributes ()
      after all instruction have been processed and we can decide what CPU
      should be selected.  */
  if (ARM_FEATURE_ZERO (selected_arch))
    ARM_MERGE_FEATURE_SETS (cpu_variant, arm_arch_any, selected_fpu);
  else
    ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
#endif

  autoselect_thumb_from_cpu_variant ();

  arm_arch_used = thumb_arch_used = arm_arch_none;

#if defined OBJ_COFF || defined OBJ_ELF
  {
    unsigned int flags = 0;

#if defined OBJ_ELF
    flags = meabi_flags;

    switch (meabi_flags)
      {
      case EF_ARM_EABI_UNKNOWN:
#endif
	/* Set the flags in the private structure.  */
	if (uses_apcs_26)      flags |= F_APCS26;
	if (support_interwork) flags |= F_INTERWORK;
	if (uses_apcs_float)   flags |= F_APCS_FLOAT;
	if (pic_code)	       flags |= F_PIC;
	if (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_any_hard))
	  flags |= F_SOFT_FLOAT;

	switch (mfloat_abi_opt)
	  {
	  case ARM_FLOAT_ABI_SOFT:
	  case ARM_FLOAT_ABI_SOFTFP:
	    flags |= F_SOFT_FLOAT;
	    break;

	  case ARM_FLOAT_ABI_HARD:
	    if (flags & F_SOFT_FLOAT)
	      as_bad (_("hard-float conflicts with specified fpu"));
	    break;
	  }

	/* Using pure-endian doubles (even if soft-float).	*/
	if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_endian_pure))
	  flags |= F_VFP_FLOAT;

#if defined OBJ_ELF
	if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_arch_maverick))
	    flags |= EF_ARM_MAVERICK_FLOAT;
	break;

      case EF_ARM_EABI_VER4:
      case EF_ARM_EABI_VER5:
	/* No additional flags to set.	*/
	break;

      default:
	abort ();
      }
#endif
    bfd_set_private_flags (stdoutput, flags);

    /* We have run out flags in the COFF header to encode the
       status of ATPCS support, so instead we create a dummy,
       empty, debug section called .arm.atpcs.	*/
    if (atpcs)
      {
	asection * sec;

	sec = bfd_make_section (stdoutput, ".arm.atpcs");

	if (sec != NULL)
	  {
	    bfd_set_section_flags (sec, SEC_READONLY | SEC_DEBUGGING);
	    bfd_set_section_size (sec, 0);
	    bfd_set_section_contents (stdoutput, sec, NULL, 0, 0);
	  }
      }
  }
#endif

  /* Record the CPU type as well.  */
  if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_iwmmxt2))
    mach = bfd_mach_arm_iWMMXt2;
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_iwmmxt))
    mach = bfd_mach_arm_iWMMXt;
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_xscale))
    mach = bfd_mach_arm_XScale;
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_maverick))
    mach = bfd_mach_arm_ep9312;
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v5e))
    mach = bfd_mach_arm_5TE;
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v5))
    {
      if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v4t))
	mach = bfd_mach_arm_5T;
      else
	mach = bfd_mach_arm_5;
    }
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v4))
    {
      if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v4t))
	mach = bfd_mach_arm_4T;
      else
	mach = bfd_mach_arm_4;
    }
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v3m))
    mach = bfd_mach_arm_3M;
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v3))
    mach = bfd_mach_arm_3;
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v2s))
    mach = bfd_mach_arm_2a;
  else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v2))
    mach = bfd_mach_arm_2;
  else
    mach = bfd_mach_arm_unknown;

  bfd_set_arch_mach (stdoutput, TARGET_ARCH, mach);
}

/* Command line processing.  */

/* md_parse_option
      Invocation line includes a switch not recognized by the base assembler.
      See if it's a processor-specific option.

      This routine is somewhat complicated by the need for backwards
      compatibility (since older releases of gcc can't be changed).
      The new options try to make the interface as compatible as
      possible with GCC.

      New options (supported) are:

	      -mcpu=<cpu name>		 Assemble for selected processor
	      -march=<architecture name> Assemble for selected architecture
	      -mfpu=<fpu architecture>	 Assemble for selected FPU.
	      -EB/-mbig-endian		 Big-endian
	      -EL/-mlittle-endian	 Little-endian
	      -k			 Generate PIC code
	      -mthumb			 Start in Thumb mode
	      -mthumb-interwork		 Code supports ARM/Thumb interworking

	      -m[no-]warn-deprecated     Warn about deprecated features
	      -m[no-]warn-syms		 Warn when symbols match instructions

      For now we will also provide support for:

	      -mapcs-32			 32-bit Program counter
	      -mapcs-26			 26-bit Program counter
	      -macps-float		 Floats passed in FP registers
	      -mapcs-reentrant		 Reentrant code
	      -matpcs
      (sometime these will probably be replaced with -mapcs=<list of options>
      and -matpcs=<list of options>)

      The remaining options are only supported for back-wards compatibility.
      Cpu variants, the arm part is optional:
	      -m[arm]1		      Currently not supported.
	      -m[arm]2, -m[arm]250    Arm 2 and Arm 250 processor
	      -m[arm]3		      Arm 3 processor
	      -m[arm]6[xx],	      Arm 6 processors
	      -m[arm]7[xx][t][[d]m]   Arm 7 processors
	      -m[arm]8[10]	      Arm 8 processors
	      -m[arm]9[20][tdmi]      Arm 9 processors
	      -mstrongarm[110[0]]     StrongARM processors
	      -mxscale		      XScale processors
	      -m[arm]v[2345[t[e]]]    Arm architectures
	      -mall		      All (except the ARM1)
      FP variants:
	      -mfpa10, -mfpa11	      FPA10 and 11 co-processor instructions
	      -mfpe-old		      (No float load/store multiples)
	      -mvfpxd		      VFP Single precision
	      -mvfp		      All VFP
	      -mno-fpu		      Disable all floating point instructions

      The following CPU names are recognized:
	      arm1, arm2, arm250, arm3, arm6, arm600, arm610, arm620,
	      arm7, arm7m, arm7d, arm7dm, arm7di, arm7dmi, arm70, arm700,
	      arm700i, arm710 arm710t, arm720, arm720t, arm740t, arm710c,
	      arm7100, arm7500, arm7500fe, arm7tdmi, arm8, arm810, arm9,
	      arm920, arm920t, arm940t, arm946, arm966, arm9tdmi, arm9e,
	      arm10t arm10e, arm1020t, arm1020e, arm10200e,
	      strongarm, strongarm110, strongarm1100, strongarm1110, xscale.

      */

const char * md_shortopts = "m:k";

#ifdef ARM_BI_ENDIAN
#define OPTION_EB (OPTION_MD_BASE + 0)
#define OPTION_EL (OPTION_MD_BASE + 1)
#else
#if TARGET_BYTES_BIG_ENDIAN
#define OPTION_EB (OPTION_MD_BASE + 0)
#else
#define OPTION_EL (OPTION_MD_BASE + 1)
#endif
#endif
#define OPTION_FIX_V4BX (OPTION_MD_BASE + 2)
#define OPTION_FDPIC (OPTION_MD_BASE + 3)

struct option md_longopts[] =
{
#ifdef OPTION_EB
  {"EB", no_argument, NULL, OPTION_EB},
#endif
#ifdef OPTION_EL
  {"EL", no_argument, NULL, OPTION_EL},
#endif
  {"fix-v4bx", no_argument, NULL, OPTION_FIX_V4BX},
#ifdef OBJ_ELF
  {"fdpic", no_argument, NULL, OPTION_FDPIC},
#endif
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

struct arm_option_table
{
  const char *  option;		/* Option name to match.  */
  const char *  help;		/* Help information.  */
  int *         var;		/* Variable to change.	*/
  int	        value;		/* What to change it to.  */
  const char *  deprecated;	/* If non-null, print this message.  */
};

struct arm_option_table arm_opts[] =
{
  {"k",	     N_("generate PIC code"),	   &pic_code,	 1, NULL},
  {"mthumb", N_("assemble Thumb code"),	   &thumb_mode,	 1, NULL},
  {"mthumb-interwork", N_("support ARM/Thumb interworking"),
   &support_interwork, 1, NULL},
  {"mapcs-32", N_("code uses 32-bit program counter"), &uses_apcs_26, 0, NULL},
  {"mapcs-26", N_("code uses 26-bit program counter"), &uses_apcs_26, 1, NULL},
  {"mapcs-float", N_("floating point args are in fp regs"), &uses_apcs_float,
   1, NULL},
  {"mapcs-reentrant", N_("re-entrant code"), &pic_code, 1, NULL},
  {"matpcs", N_("code is ATPCS conformant"), &atpcs, 1, NULL},
  {"mbig-endian", N_("assemble for big-endian"), &target_big_endian, 1, NULL},
  {"mlittle-endian", N_("assemble for little-endian"), &target_big_endian, 0,
   NULL},

  /* These are recognized by the assembler, but have no affect on code.	 */
  {"mapcs-frame", N_("use frame pointer"), NULL, 0, NULL},
  {"mapcs-stack-check", N_("use stack size checking"), NULL, 0, NULL},

  {"mwarn-deprecated", NULL, &warn_on_deprecated, 1, NULL},
  {"mno-warn-deprecated", N_("do not warn on use of deprecated feature"),
   &warn_on_deprecated, 0, NULL},

  {"mwarn-restrict-it", N_("warn about performance deprecated IT instructions"
   " in ARMv8-A and ARMv8-R"), &warn_on_restrict_it, 1, NULL},
  {"mno-warn-restrict-it", NULL, &warn_on_restrict_it, 0, NULL},

  {"mwarn-syms", N_("warn about symbols that match instruction names [default]"), (int *) (& flag_warn_syms), true, NULL},
  {"mno-warn-syms", N_("disable warnings about symobls that match instructions"), (int *) (& flag_warn_syms), false, NULL},
  {NULL, NULL, NULL, 0, NULL}
};

struct arm_legacy_option_table
{
  const char *              option;		/* Option name to match.  */
  const arm_feature_set	**  var;		/* Variable to change.	*/
  const arm_feature_set	    value;		/* What to change it to.  */
  const char *              deprecated;		/* If non-null, print this message.  */
};

const struct arm_legacy_option_table arm_legacy_opts[] =
{
  /* DON'T add any new processors to this list -- we want the whole list
     to go away...  Add them to the processors table instead.  */
  {"marm1",	 &legacy_cpu, ARM_ARCH_V1,  N_("use -mcpu=arm1")},
  {"m1",	 &legacy_cpu, ARM_ARCH_V1,  N_("use -mcpu=arm1")},
  {"marm2",	 &legacy_cpu, ARM_ARCH_V2,  N_("use -mcpu=arm2")},
  {"m2",	 &legacy_cpu, ARM_ARCH_V2,  N_("use -mcpu=arm2")},
  {"marm250",	 &legacy_cpu, ARM_ARCH_V2S, N_("use -mcpu=arm250")},
  {"m250",	 &legacy_cpu, ARM_ARCH_V2S, N_("use -mcpu=arm250")},
  {"marm3",	 &legacy_cpu, ARM_ARCH_V2S, N_("use -mcpu=arm3")},
  {"m3",	 &legacy_cpu, ARM_ARCH_V2S, N_("use -mcpu=arm3")},
  {"marm6",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm6")},
  {"m6",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm6")},
  {"marm600",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm600")},
  {"m600",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm600")},
  {"marm610",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm610")},
  {"m610",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm610")},
  {"marm620",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm620")},
  {"m620",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm620")},
  {"marm7",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7")},
  {"m7",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7")},
  {"marm70",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm70")},
  {"m70",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm70")},
  {"marm700",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm700")},
  {"m700",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm700")},
  {"marm700i",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm700i")},
  {"m700i",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm700i")},
  {"marm710",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm710")},
  {"m710",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm710")},
  {"marm710c",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm710c")},
  {"m710c",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm710c")},
  {"marm720",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm720")},
  {"m720",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm720")},
  {"marm7d",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7d")},
  {"m7d",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7d")},
  {"marm7di",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7di")},
  {"m7di",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7di")},
  {"marm7m",	 &legacy_cpu, ARM_ARCH_V3M, N_("use -mcpu=arm7m")},
  {"m7m",	 &legacy_cpu, ARM_ARCH_V3M, N_("use -mcpu=arm7m")},
  {"marm7dm",	 &legacy_cpu, ARM_ARCH_V3M, N_("use -mcpu=arm7dm")},
  {"m7dm",	 &legacy_cpu, ARM_ARCH_V3M, N_("use -mcpu=arm7dm")},
  {"marm7dmi",	 &legacy_cpu, ARM_ARCH_V3M, N_("use -mcpu=arm7dmi")},
  {"m7dmi",	 &legacy_cpu, ARM_ARCH_V3M, N_("use -mcpu=arm7dmi")},
  {"marm7100",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7100")},
  {"m7100",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7100")},
  {"marm7500",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7500")},
  {"m7500",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7500")},
  {"marm7500fe", &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7500fe")},
  {"m7500fe",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -mcpu=arm7500fe")},
  {"marm7t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm7tdmi")},
  {"m7t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm7tdmi")},
  {"marm7tdmi",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm7tdmi")},
  {"m7tdmi",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm7tdmi")},
  {"marm710t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm710t")},
  {"m710t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm710t")},
  {"marm720t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm720t")},
  {"m720t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm720t")},
  {"marm740t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm740t")},
  {"m740t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm740t")},
  {"marm8",	 &legacy_cpu, ARM_ARCH_V4,  N_("use -mcpu=arm8")},
  {"m8",	 &legacy_cpu, ARM_ARCH_V4,  N_("use -mcpu=arm8")},
  {"marm810",	 &legacy_cpu, ARM_ARCH_V4,  N_("use -mcpu=arm810")},
  {"m810",	 &legacy_cpu, ARM_ARCH_V4,  N_("use -mcpu=arm810")},
  {"marm9",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm9")},
  {"m9",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm9")},
  {"marm9tdmi",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm9tdmi")},
  {"m9tdmi",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm9tdmi")},
  {"marm920",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm920")},
  {"m920",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm920")},
  {"marm940",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm940")},
  {"m940",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -mcpu=arm940")},
  {"mstrongarm", &legacy_cpu, ARM_ARCH_V4,  N_("use -mcpu=strongarm")},
  {"mstrongarm110", &legacy_cpu, ARM_ARCH_V4,
   N_("use -mcpu=strongarm110")},
  {"mstrongarm1100", &legacy_cpu, ARM_ARCH_V4,
   N_("use -mcpu=strongarm1100")},
  {"mstrongarm1110", &legacy_cpu, ARM_ARCH_V4,
   N_("use -mcpu=strongarm1110")},
  {"mxscale",	 &legacy_cpu, ARM_ARCH_XSCALE, N_("use -mcpu=xscale")},
  {"miwmmxt",	 &legacy_cpu, ARM_ARCH_IWMMXT, N_("use -mcpu=iwmmxt")},
  {"mall",	 &legacy_cpu, ARM_ANY,	       N_("use -mcpu=all")},

  /* Architecture variants -- don't add any more to this list either.  */
  {"mv2",	 &legacy_cpu, ARM_ARCH_V2,  N_("use -march=armv2")},
  {"marmv2",	 &legacy_cpu, ARM_ARCH_V2,  N_("use -march=armv2")},
  {"mv2a",	 &legacy_cpu, ARM_ARCH_V2S, N_("use -march=armv2a")},
  {"marmv2a",	 &legacy_cpu, ARM_ARCH_V2S, N_("use -march=armv2a")},
  {"mv3",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -march=armv3")},
  {"marmv3",	 &legacy_cpu, ARM_ARCH_V3,  N_("use -march=armv3")},
  {"mv3m",	 &legacy_cpu, ARM_ARCH_V3M, N_("use -march=armv3m")},
  {"marmv3m",	 &legacy_cpu, ARM_ARCH_V3M, N_("use -march=armv3m")},
  {"mv4",	 &legacy_cpu, ARM_ARCH_V4,  N_("use -march=armv4")},
  {"marmv4",	 &legacy_cpu, ARM_ARCH_V4,  N_("use -march=armv4")},
  {"mv4t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -march=armv4t")},
  {"marmv4t",	 &legacy_cpu, ARM_ARCH_V4T, N_("use -march=armv4t")},
  {"mv5",	 &legacy_cpu, ARM_ARCH_V5,  N_("use -march=armv5")},
  {"marmv5",	 &legacy_cpu, ARM_ARCH_V5,  N_("use -march=armv5")},
  {"mv5t",	 &legacy_cpu, ARM_ARCH_V5T, N_("use -march=armv5t")},
  {"marmv5t",	 &legacy_cpu, ARM_ARCH_V5T, N_("use -march=armv5t")},
  {"mv5e",	 &legacy_cpu, ARM_ARCH_V5TE, N_("use -march=armv5te")},
  {"marmv5e",	 &legacy_cpu, ARM_ARCH_V5TE, N_("use -march=armv5te")},

  /* Floating point variants -- don't add any more to this list either.	 */
  {"mfpe-old",   &legacy_fpu, FPU_ARCH_FPE, N_("use -mfpu=fpe")},
  {"mfpa10",     &legacy_fpu, FPU_ARCH_FPA, N_("use -mfpu=fpa10")},
  {"mfpa11",     &legacy_fpu, FPU_ARCH_FPA, N_("use -mfpu=fpa11")},
  {"mno-fpu",    &legacy_fpu, ARM_ARCH_NONE,
   N_("use either -mfpu=softfpa or -mfpu=softvfp")},

  {NULL, NULL, ARM_ARCH_NONE, NULL}
};

struct arm_cpu_option_table
{
  const char *           name;
  size_t                 name_len;
  const arm_feature_set	 value;
  const arm_feature_set	 ext;
  /* For some CPUs we assume an FPU unless the user explicitly sets
     -mfpu=...	*/
  const arm_feature_set	 default_fpu;
  /* The canonical name of the CPU, or NULL to use NAME converted to upper
     case.  */
  const char *           canonical_name;
};

/* This list should, at a minimum, contain all the cpu names
   recognized by GCC.  */
#define ARM_CPU_OPT(N, CN, V, E, DF) { N, sizeof (N) - 1, V, E, DF, CN }

static const struct arm_cpu_option_table arm_cpus[] =
{
  ARM_CPU_OPT ("all",		  NULL,		       ARM_ANY,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm1",		  NULL,		       ARM_ARCH_V1,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm2",		  NULL,		       ARM_ARCH_V2,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm250",	  NULL,		       ARM_ARCH_V2S,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm3",		  NULL,		       ARM_ARCH_V2S,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm6",		  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm60",		  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm600",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm610",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm620",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7",		  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7m",		  NULL,		       ARM_ARCH_V3M,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7d",		  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7dm",	  NULL,		       ARM_ARCH_V3M,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7di",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7dmi",	  NULL,		       ARM_ARCH_V3M,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm70",		  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm700",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm700i",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm710",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm710t",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm720",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm720t",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm740t",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm710c",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7100",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7500",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7500fe",	  NULL,		       ARM_ARCH_V3,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7t",		  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7tdmi",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm7tdmi-s",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm8",		  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm810",	  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("strongarm",	  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("strongarm1",	  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("strongarm110",	  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("strongarm1100",	  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("strongarm1110",	  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm9",		  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm920",	  "ARM920T",	       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm920t",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm922t",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm940t",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("arm9tdmi",	  NULL,		       ARM_ARCH_V4T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("fa526",		  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),
  ARM_CPU_OPT ("fa626",		  NULL,		       ARM_ARCH_V4,
	       ARM_ARCH_NONE,
	       FPU_ARCH_FPA),

  /* For V5 or later processors we default to using VFP; but the user
     should really set the FPU type explicitly.	 */
  ARM_CPU_OPT ("arm9e-r0",	  NULL,		       ARM_ARCH_V5TExP,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm9e",		  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm926ej",	  "ARM926EJ-S",	       ARM_ARCH_V5TEJ,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm926ejs",	  "ARM926EJ-S",	       ARM_ARCH_V5TEJ,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm926ej-s",	  NULL,		       ARM_ARCH_V5TEJ,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm946e-r0",	  NULL,		       ARM_ARCH_V5TExP,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm946e",	  "ARM946E-S",	       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm946e-s",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm966e-r0",	  NULL,		       ARM_ARCH_V5TExP,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm966e",	  "ARM966E-S",	       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm966e-s",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm968e-s",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm10t",	  NULL,		       ARM_ARCH_V5T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V1),
  ARM_CPU_OPT ("arm10tdmi",	  NULL,		       ARM_ARCH_V5T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V1),
  ARM_CPU_OPT ("arm10e",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm1020",	  "ARM1020E",	       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm1020t",	  NULL,		       ARM_ARCH_V5T,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V1),
  ARM_CPU_OPT ("arm1020e",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm1022e",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm1026ejs",	  "ARM1026EJ-S",       ARM_ARCH_V5TEJ,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm1026ej-s",	  NULL,		       ARM_ARCH_V5TEJ,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("fa606te",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("fa616te",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("fa626te",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("fmp626",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("fa726te",	  NULL,		       ARM_ARCH_V5TE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm1136js",	  "ARM1136J-S",	       ARM_ARCH_V6,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("arm1136j-s",	  NULL,		       ARM_ARCH_V6,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("arm1136jfs",	  "ARM1136JF-S",       ARM_ARCH_V6,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm1136jf-s",	  NULL,		       ARM_ARCH_V6,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("mpcore",	  "MPCore",	       ARM_ARCH_V6K,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("mpcorenovfp",	  "MPCore",	       ARM_ARCH_V6K,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("arm1156t2-s",	  NULL,		       ARM_ARCH_V6T2,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("arm1156t2f-s",	  NULL,		       ARM_ARCH_V6T2,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("arm1176jz-s",	  NULL,		       ARM_ARCH_V6KZ,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("arm1176jzf-s",	  NULL,		       ARM_ARCH_V6KZ,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("cortex-a5",	  "Cortex-A5",	       ARM_ARCH_V7A,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_MP | ARM_EXT_SEC),
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-a7",	  "Cortex-A7",	       ARM_ARCH_V7VE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_NEON_VFP_V4),
  ARM_CPU_OPT ("cortex-a8",	  "Cortex-A8",	       ARM_ARCH_V7A,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_SEC),
	       ARM_FEATURE_COPROC (FPU_VFP_V3 | FPU_NEON_EXT_V1)),
  ARM_CPU_OPT ("cortex-a9",	  "Cortex-A9",	       ARM_ARCH_V7A,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_MP | ARM_EXT_SEC),
	       ARM_FEATURE_COPROC (FPU_VFP_V3 | FPU_NEON_EXT_V1)),
  ARM_CPU_OPT ("cortex-a12",	  "Cortex-A12",	       ARM_ARCH_V7VE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_NEON_VFP_V4),
  ARM_CPU_OPT ("cortex-a15",	  "Cortex-A15",	       ARM_ARCH_V7VE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_NEON_VFP_V4),
  ARM_CPU_OPT ("cortex-a17",	  "Cortex-A17",	       ARM_ARCH_V7VE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_NEON_VFP_V4),
  ARM_CPU_OPT ("cortex-a32",	  "Cortex-A32",	       ARM_ARCH_V8A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a35",	  "Cortex-A35",	       ARM_ARCH_V8A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a53",	  "Cortex-A53",	       ARM_ARCH_V8A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a55",    "Cortex-A55",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_DOTPROD),
  ARM_CPU_OPT ("cortex-a57",	  "Cortex-A57",	       ARM_ARCH_V8A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a72",	  "Cortex-A72",	       ARM_ARCH_V8A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	      FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a73",	  "Cortex-A73",	       ARM_ARCH_V8A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	      FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a75",    "Cortex-A75",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_DOTPROD),
  ARM_CPU_OPT ("cortex-a76",    "Cortex-A76",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_DOTPROD),
  ARM_CPU_OPT ("cortex-a76ae",    "Cortex-A76AE",      ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_DOTPROD),
  ARM_CPU_OPT ("cortex-a77",    "Cortex-A77",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_DOTPROD),
  ARM_CPU_OPT ("cortex-a78",   "Cortex-A78",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_SB),
	       FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a78ae",   "Cortex-A78AE",	   ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_SB),
	       FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a78c",   "Cortex-A78C",	   ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_SB),
	       FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-a710",   "Cortex-A710",	   ARM_ARCH_V9A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST
				    | ARM_EXT2_BF16
				    | ARM_EXT2_I8MM),
	       FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("ares",    "Ares",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_DOTPROD),
  ARM_CPU_OPT ("cortex-r4",	  "Cortex-R4",	       ARM_ARCH_V7R,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-r4f",	  "Cortex-R4F",	       ARM_ARCH_V7R,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V3D16),
  ARM_CPU_OPT ("cortex-r5",	  "Cortex-R5",	       ARM_ARCH_V7R,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV),
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-r7",	  "Cortex-R7",	       ARM_ARCH_V7R,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV),
	       FPU_ARCH_VFP_V3D16),
  ARM_CPU_OPT ("cortex-r8",	  "Cortex-R8",	       ARM_ARCH_V7R,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV),
	       FPU_ARCH_VFP_V3D16),
  ARM_CPU_OPT ("cortex-r52",	  "Cortex-R52",	       ARM_ARCH_V8R,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	      FPU_ARCH_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-r52plus",	  "Cortex-R52+",	       ARM_ARCH_V8R,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	      FPU_ARCH_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-m35p",	  "Cortex-M35P",       ARM_ARCH_V8M_MAIN,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP | ARM_EXT_V6_DSP),
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-m33",	  "Cortex-M33",	       ARM_ARCH_V8M_MAIN,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP | ARM_EXT_V6_DSP),
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-m23",	  "Cortex-M23",	       ARM_ARCH_V8M_BASE,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-m7",	  "Cortex-M7",	       ARM_ARCH_V7EM,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-m4",	  "Cortex-M4",	       ARM_ARCH_V7EM,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-m3",	  "Cortex-M3",	       ARM_ARCH_V7M,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-m1",	  "Cortex-M1",	       ARM_ARCH_V6SM,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-m0",	  "Cortex-M0",	       ARM_ARCH_V6SM,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-m0plus",	  "Cortex-M0+",	       ARM_ARCH_V6SM,
	       ARM_ARCH_NONE,
	       FPU_NONE),
  ARM_CPU_OPT ("cortex-x1",   "Cortex-X1",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_SB),
	       FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("cortex-x1c",   "Cortex-X1C",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST | ARM_EXT2_SB),
	       FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("exynos-m1",	  "Samsung Exynos M1", ARM_ARCH_V8A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("neoverse-n1",    "Neoverse N1",	       ARM_ARCH_V8_2A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_DOTPROD),
  ARM_CPU_OPT ("neoverse-n2",	 "Neoverse N2",	       ARM_ARCH_V8_5A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST
				    | ARM_EXT2_BF16
				    | ARM_EXT2_I8MM),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_4),
  ARM_CPU_OPT ("neoverse-v1", "Neoverse V1", ARM_ARCH_V8_4A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST
				    | ARM_EXT2_BF16
				    | ARM_EXT2_I8MM),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_4),
  /* ??? XSCALE is really an architecture.  */
  ARM_CPU_OPT ("xscale",	  NULL,		       ARM_ARCH_XSCALE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),

  /* ??? iwmmxt is not a processor.  */
  ARM_CPU_OPT ("iwmmxt",	  NULL,		       ARM_ARCH_IWMMXT,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("iwmmxt2",	  NULL,		       ARM_ARCH_IWMMXT2,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),
  ARM_CPU_OPT ("i80200",	  NULL,		       ARM_ARCH_XSCALE,
	       ARM_ARCH_NONE,
	       FPU_ARCH_VFP_V2),

  /* Maverick.  */
  ARM_CPU_OPT ("ep9312",	  "ARM920T",
	       ARM_FEATURE_LOW (ARM_AEXT_V4T, ARM_CEXT_MAVERICK),
	       ARM_ARCH_NONE, FPU_ARCH_MAVERICK),

  /* Marvell processors.  */
  ARM_CPU_OPT ("marvell-pj4",	  NULL,		       ARM_ARCH_V7A,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_MP | ARM_EXT_SEC),
	       FPU_ARCH_VFP_V3D16),
  ARM_CPU_OPT ("marvell-whitney", NULL,		       ARM_ARCH_V7A,
	       ARM_FEATURE_CORE_LOW (ARM_EXT_MP | ARM_EXT_SEC),
	       FPU_ARCH_NEON_VFP_V4),

  /* APM X-Gene family.  */
  ARM_CPU_OPT ("xgene1",	  "APM X-Gene 1",      ARM_ARCH_V8A,
	       ARM_ARCH_NONE,
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),
  ARM_CPU_OPT ("xgene2",	  "APM X-Gene 2",      ARM_ARCH_V8A,
	       ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC),
	       FPU_ARCH_CRYPTO_NEON_VFP_ARMV8),

  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE, ARM_ARCH_NONE, NULL }
};
#undef ARM_CPU_OPT

struct arm_ext_table
{
  const char *		  name;
  size_t		  name_len;
  const arm_feature_set	  merge;
  const arm_feature_set	  clear;
};

struct arm_arch_option_table
{
  const char *			name;
  size_t			name_len;
  const arm_feature_set		value;
  const arm_feature_set		default_fpu;
  const struct arm_ext_table *	ext_table;
};

/* Used to add support for +E and +noE extension.  */
#define ARM_EXT(E, M, C) { E, sizeof (E) - 1, M, C }
/* Used to add support for a +E extension.  */
#define ARM_ADD(E, M) { E, sizeof(E) - 1, M, ARM_ARCH_NONE }
/* Used to add support for a +noE extension.  */
#define ARM_REMOVE(E, C) { E, sizeof(E) -1, ARM_ARCH_NONE, C }

#define ALL_FP ARM_FEATURE (0, ARM_EXT2_FP16_INST | ARM_EXT2_FP16_FML, \
			    ~0 & ~FPU_ENDIAN_PURE)

static const struct arm_ext_table armv5te_ext_table[] =
{
  ARM_EXT ("fp", FPU_ARCH_VFP_V2, ALL_FP),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv7_ext_table[] =
{
  ARM_EXT ("fp", FPU_ARCH_VFP_V3D16, ALL_FP),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv7ve_ext_table[] =
{
  ARM_EXT ("fp", FPU_ARCH_VFP_V4D16, ALL_FP),
  ARM_ADD ("vfpv3-d16", FPU_ARCH_VFP_V3D16),
  ARM_ADD ("vfpv3", FPU_ARCH_VFP_V3),
  ARM_ADD ("vfpv3-d16-fp16", FPU_ARCH_VFP_V3D16_FP16),
  ARM_ADD ("vfpv3-fp16", FPU_ARCH_VFP_V3_FP16),
  ARM_ADD ("vfpv4-d16", FPU_ARCH_VFP_V4D16),  /* Alias for +fp.  */
  ARM_ADD ("vfpv4", FPU_ARCH_VFP_V4),

  ARM_EXT ("simd", FPU_ARCH_NEON_VFP_V4,
	   ARM_FEATURE_COPROC (FPU_NEON_EXT_V1 | FPU_NEON_EXT_FMA)),

  /* Aliases for +simd.  */
  ARM_ADD ("neon-vfpv4", FPU_ARCH_NEON_VFP_V4),

  ARM_ADD ("neon", FPU_ARCH_VFP_V3_PLUS_NEON_V1),
  ARM_ADD ("neon-vfpv3", FPU_ARCH_VFP_V3_PLUS_NEON_V1),
  ARM_ADD ("neon-fp16", FPU_ARCH_NEON_FP16),

  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv7a_ext_table[] =
{
  ARM_EXT ("fp", FPU_ARCH_VFP_V3D16, ALL_FP),
  ARM_ADD ("vfpv3-d16", FPU_ARCH_VFP_V3D16), /* Alias for +fp.  */
  ARM_ADD ("vfpv3", FPU_ARCH_VFP_V3),
  ARM_ADD ("vfpv3-d16-fp16", FPU_ARCH_VFP_V3D16_FP16),
  ARM_ADD ("vfpv3-fp16", FPU_ARCH_VFP_V3_FP16),
  ARM_ADD ("vfpv4-d16", FPU_ARCH_VFP_V4D16),
  ARM_ADD ("vfpv4", FPU_ARCH_VFP_V4),

  ARM_EXT ("simd", FPU_ARCH_VFP_V3_PLUS_NEON_V1,
	   ARM_FEATURE_COPROC (FPU_NEON_EXT_V1 | FPU_NEON_EXT_FMA)),

  /* Aliases for +simd.  */
  ARM_ADD ("neon", FPU_ARCH_VFP_V3_PLUS_NEON_V1),
  ARM_ADD ("neon-vfpv3", FPU_ARCH_VFP_V3_PLUS_NEON_V1),

  ARM_ADD ("neon-fp16", FPU_ARCH_NEON_FP16),
  ARM_ADD ("neon-vfpv4", FPU_ARCH_NEON_VFP_V4),

  ARM_ADD ("mp", ARM_FEATURE_CORE_LOW (ARM_EXT_MP)),
  ARM_ADD ("sec", ARM_FEATURE_CORE_LOW (ARM_EXT_SEC)),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv7r_ext_table[] =
{
  ARM_ADD ("fp.sp", FPU_ARCH_VFP_V3xD),
  ARM_ADD ("vfpv3xd", FPU_ARCH_VFP_V3xD), /* Alias for +fp.sp.  */
  ARM_EXT ("fp", FPU_ARCH_VFP_V3D16, ALL_FP),
  ARM_ADD ("vfpv3-d16", FPU_ARCH_VFP_V3D16), /* Alias for +fp.  */
  ARM_ADD ("vfpv3xd-fp16", FPU_ARCH_VFP_V3xD_FP16),
  ARM_ADD ("vfpv3-d16-fp16", FPU_ARCH_VFP_V3D16_FP16),
  ARM_EXT ("idiv", ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV | ARM_EXT_DIV),
	   ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV | ARM_EXT_DIV)),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv7em_ext_table[] =
{
  ARM_EXT ("fp", FPU_ARCH_VFP_V4_SP_D16, ALL_FP),
  /* Alias for +fp, used to be known as fpv4-sp-d16.  */
  ARM_ADD ("vfpv4-sp-d16", FPU_ARCH_VFP_V4_SP_D16),
  ARM_ADD ("fpv5", FPU_ARCH_VFP_V5_SP_D16),
  ARM_ADD ("fp.dp", FPU_ARCH_VFP_V5D16),
  ARM_ADD ("fpv5-d16", FPU_ARCH_VFP_V5D16),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv8a_ext_table[] =
{
  ARM_ADD ("crc", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC)),
  ARM_ADD ("simd", FPU_ARCH_NEON_VFP_ARMV8),
  ARM_EXT ("crypto", FPU_ARCH_CRYPTO_NEON_VFP_ARMV8,
	   ARM_FEATURE_COPROC (FPU_CRYPTO_ARMV8)),

  /* Armv8-a does not allow an FP implementation without SIMD, so the user
     should use the +simd option to turn on FP.  */
  ARM_REMOVE ("fp", ALL_FP),
  ARM_ADD ("sb", ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB)),
  ARM_ADD ("predres", ARM_FEATURE_CORE_HIGH (ARM_EXT2_PREDRES)),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};


static const struct arm_ext_table armv81a_ext_table[] =
{
  ARM_ADD ("simd", FPU_ARCH_NEON_VFP_ARMV8_1),
  ARM_EXT ("crypto", FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_1,
	   ARM_FEATURE_COPROC (FPU_CRYPTO_ARMV8)),

  /* Armv8-a does not allow an FP implementation without SIMD, so the user
     should use the +simd option to turn on FP.  */
  ARM_REMOVE ("fp", ALL_FP),
  ARM_ADD ("sb", ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB)),
  ARM_ADD ("predres", ARM_FEATURE_CORE_HIGH (ARM_EXT2_PREDRES)),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv82a_ext_table[] =
{
  ARM_ADD ("simd", FPU_ARCH_NEON_VFP_ARMV8_1),
  ARM_ADD ("fp16", FPU_ARCH_NEON_VFP_ARMV8_2_FP16),
  ARM_ADD ("fp16fml", FPU_ARCH_NEON_VFP_ARMV8_2_FP16FML),
  ARM_ADD ("bf16", ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16)),
  ARM_ADD ("i8mm", ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM)),
  ARM_EXT ("crypto", FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_1,
	   ARM_FEATURE_COPROC (FPU_CRYPTO_ARMV8)),
  ARM_ADD ("dotprod", FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),

  /* Armv8-a does not allow an FP implementation without SIMD, so the user
     should use the +simd option to turn on FP.  */
  ARM_REMOVE ("fp", ALL_FP),
  ARM_ADD ("sb", ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB)),
  ARM_ADD ("predres", ARM_FEATURE_CORE_HIGH (ARM_EXT2_PREDRES)),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv84a_ext_table[] =
{
  ARM_ADD ("simd", FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_ADD ("fp16", FPU_ARCH_NEON_VFP_ARMV8_4_FP16FML),
  ARM_ADD ("bf16", ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16)),
  ARM_ADD ("i8mm", ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM)),
  ARM_EXT ("crypto", FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_4,
	   ARM_FEATURE_COPROC (FPU_CRYPTO_ARMV8)),

  /* Armv8-a does not allow an FP implementation without SIMD, so the user
     should use the +simd option to turn on FP.  */
  ARM_REMOVE ("fp", ALL_FP),
  ARM_ADD ("sb", ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB)),
  ARM_ADD ("predres", ARM_FEATURE_CORE_HIGH (ARM_EXT2_PREDRES)),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv85a_ext_table[] =
{
  ARM_ADD ("simd", FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_ADD ("fp16", FPU_ARCH_NEON_VFP_ARMV8_4_FP16FML),
  ARM_ADD ("bf16", ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16)),
  ARM_ADD ("i8mm", ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM)),
  ARM_EXT ("crypto", FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_4,
	   ARM_FEATURE_COPROC (FPU_CRYPTO_ARMV8)),

  /* Armv8-a does not allow an FP implementation without SIMD, so the user
     should use the +simd option to turn on FP.  */
  ARM_REMOVE ("fp", ALL_FP),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

static const struct arm_ext_table armv86a_ext_table[] =
{
  ARM_ADD ("i8mm", ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM)),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

#define armv87a_ext_table armv86a_ext_table
#define armv88a_ext_table armv87a_ext_table

static const struct arm_ext_table armv9a_ext_table[] =
{
  ARM_ADD ("simd", FPU_ARCH_DOTPROD_NEON_VFP_ARMV8),
  ARM_ADD ("fp16", FPU_ARCH_NEON_VFP_ARMV8_4_FP16FML),
  ARM_ADD ("bf16", ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16)),
  ARM_ADD ("i8mm", ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM)),
  ARM_EXT ("crypto", FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_4,
	   ARM_FEATURE_COPROC (FPU_CRYPTO_ARMV8)),

  /* Armv9-a does not allow an FP implementation without SIMD, so the user
     should use the +simd option to turn on FP.  */
  ARM_REMOVE ("fp", ALL_FP),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

#define armv91a_ext_table armv86a_ext_table
#define armv92a_ext_table armv91a_ext_table
#define armv93a_ext_table armv92a_ext_table

#define CDE_EXTENSIONS \
  ARM_ADD ("cdecp0", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE | ARM_EXT2_CDE0)), \
  ARM_ADD ("cdecp1", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE | ARM_EXT2_CDE1)), \
  ARM_ADD ("cdecp2", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE | ARM_EXT2_CDE2)), \
  ARM_ADD ("cdecp3", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE | ARM_EXT2_CDE3)), \
  ARM_ADD ("cdecp4", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE | ARM_EXT2_CDE4)), \
  ARM_ADD ("cdecp5", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE | ARM_EXT2_CDE5)), \
  ARM_ADD ("cdecp6", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE | ARM_EXT2_CDE6)), \
  ARM_ADD ("cdecp7", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE | ARM_EXT2_CDE7))

static const struct arm_ext_table armv8m_main_ext_table[] =
{
  ARM_EXT ("dsp", ARM_FEATURE_CORE_LOW (ARM_AEXT_V8M_MAIN_DSP),
		  ARM_FEATURE_CORE_LOW (ARM_AEXT_V8M_MAIN_DSP)),
  ARM_EXT ("fp", FPU_ARCH_VFP_V5_SP_D16, ALL_FP),
  ARM_ADD ("fp.dp", FPU_ARCH_VFP_V5D16),
  CDE_EXTENSIONS,
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};


static const struct arm_ext_table armv8_1m_main_ext_table[] =
{
  ARM_EXT ("dsp", ARM_FEATURE_CORE_LOW (ARM_AEXT_V8M_MAIN_DSP),
		  ARM_FEATURE_CORE_LOW (ARM_AEXT_V8M_MAIN_DSP)),
  ARM_EXT ("fp",
	   ARM_FEATURE (0, ARM_EXT2_FP16_INST,
			FPU_VFP_V5_SP_D16 | FPU_VFP_EXT_FP16 | FPU_VFP_EXT_FMA),
	   ALL_FP),
  ARM_ADD ("fp.dp",
	   ARM_FEATURE (0, ARM_EXT2_FP16_INST,
			FPU_VFP_V5D16 | FPU_VFP_EXT_FP16 | FPU_VFP_EXT_FMA)),
  ARM_EXT ("mve", ARM_FEATURE (ARM_AEXT_V8M_MAIN_DSP, ARM_EXT2_MVE, 0),
	   ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE | ARM_EXT2_MVE_FP)),
  ARM_ADD ("mve.fp",
	   ARM_FEATURE (ARM_AEXT_V8M_MAIN_DSP,
			ARM_EXT2_FP16_INST | ARM_EXT2_MVE | ARM_EXT2_MVE_FP,
			FPU_VFP_V5_SP_D16 | FPU_VFP_EXT_FP16 | FPU_VFP_EXT_FMA)),
  CDE_EXTENSIONS,
  ARM_ADD ("pacbti", ARM_FEATURE_CORE_HIGH_HIGH (ARM_AEXT3_V8_1M_MAIN_PACBTI)),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

#undef CDE_EXTENSIONS

static const struct arm_ext_table armv8r_ext_table[] =
{
  ARM_ADD ("crc", ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC)),
  ARM_ADD ("simd", FPU_ARCH_NEON_VFP_ARMV8),
  ARM_EXT ("crypto", FPU_ARCH_CRYPTO_NEON_VFP_ARMV8,
	   ARM_FEATURE_COPROC (FPU_CRYPTO_ARMV8)),
  ARM_REMOVE ("fp", ALL_FP),
  ARM_ADD ("fp.sp", FPU_ARCH_VFP_V5_SP_D16),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE }
};

/* This list should, at a minimum, contain all the architecture names
   recognized by GCC.  */
#define ARM_ARCH_OPT(N, V, DF) { N, sizeof (N) - 1, V, DF, NULL }
#define ARM_ARCH_OPT2(N, V, DF, ext) \
  { N, sizeof (N) - 1, V, DF, ext##_ext_table }

static const struct arm_arch_option_table arm_archs[] =
{
  ARM_ARCH_OPT ("all",		  ARM_ANY,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv1",	  ARM_ARCH_V1,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv2",	  ARM_ARCH_V2,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv2a",	  ARM_ARCH_V2S,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv2s",	  ARM_ARCH_V2S,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv3",	  ARM_ARCH_V3,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv3m",	  ARM_ARCH_V3M,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv4",	  ARM_ARCH_V4,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv4xm",	  ARM_ARCH_V4xM,	FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv4t",	  ARM_ARCH_V4T,		FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv4txm",	  ARM_ARCH_V4TxM,	FPU_ARCH_FPA),
  ARM_ARCH_OPT ("armv5",	  ARM_ARCH_V5,		FPU_ARCH_VFP),
  ARM_ARCH_OPT ("armv5t",	  ARM_ARCH_V5T,		FPU_ARCH_VFP),
  ARM_ARCH_OPT ("armv5txm",	  ARM_ARCH_V5TxM,	FPU_ARCH_VFP),
  ARM_ARCH_OPT2 ("armv5te",	  ARM_ARCH_V5TE,	FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv5texp",	  ARM_ARCH_V5TExP,	FPU_ARCH_VFP, armv5te),
  ARM_ARCH_OPT2 ("armv5tej",	  ARM_ARCH_V5TEJ,	FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6",	  ARM_ARCH_V6,		FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6j",	  ARM_ARCH_V6,		FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6k",	  ARM_ARCH_V6K,		FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6z",	  ARM_ARCH_V6Z,		FPU_ARCH_VFP,	armv5te),
  /* The official spelling of this variant is ARMv6KZ, the name "armv6zk" is
     kept to preserve existing behaviour.  */
  ARM_ARCH_OPT2 ("armv6kz",	  ARM_ARCH_V6KZ,	FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6zk",	  ARM_ARCH_V6KZ,	FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6t2",	  ARM_ARCH_V6T2,	FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6kt2",	  ARM_ARCH_V6KT2,	FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6zt2",	  ARM_ARCH_V6ZT2,	FPU_ARCH_VFP,	armv5te),
  /* The official spelling of this variant is ARMv6KZ, the name "armv6zkt2" is
     kept to preserve existing behaviour.  */
  ARM_ARCH_OPT2 ("armv6kzt2",	  ARM_ARCH_V6KZT2,	FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT2 ("armv6zkt2",	  ARM_ARCH_V6KZT2,	FPU_ARCH_VFP,	armv5te),
  ARM_ARCH_OPT ("armv6-m",	  ARM_ARCH_V6M,		FPU_ARCH_VFP),
  ARM_ARCH_OPT ("armv6s-m",	  ARM_ARCH_V6SM,	FPU_ARCH_VFP),
  ARM_ARCH_OPT2 ("armv7",	  ARM_ARCH_V7,		FPU_ARCH_VFP, armv7),
  /* The official spelling of the ARMv7 profile variants is the dashed form.
     Accept the non-dashed form for compatibility with old toolchains.  */
  ARM_ARCH_OPT2 ("armv7a",	  ARM_ARCH_V7A,		FPU_ARCH_VFP, armv7a),
  ARM_ARCH_OPT2 ("armv7ve",	  ARM_ARCH_V7VE,	FPU_ARCH_VFP, armv7ve),
  ARM_ARCH_OPT2 ("armv7r",	  ARM_ARCH_V7R,		FPU_ARCH_VFP, armv7r),
  ARM_ARCH_OPT ("armv7m",	  ARM_ARCH_V7M,		FPU_ARCH_VFP),
  ARM_ARCH_OPT2 ("armv7-a",	  ARM_ARCH_V7A,		FPU_ARCH_VFP, armv7a),
  ARM_ARCH_OPT2 ("armv7-r",	  ARM_ARCH_V7R,		FPU_ARCH_VFP, armv7r),
  ARM_ARCH_OPT ("armv7-m",	  ARM_ARCH_V7M,		FPU_ARCH_VFP),
  ARM_ARCH_OPT2 ("armv7e-m",	  ARM_ARCH_V7EM,	FPU_ARCH_VFP, armv7em),
  ARM_ARCH_OPT ("armv8-m.base",	  ARM_ARCH_V8M_BASE,	FPU_ARCH_VFP),
  ARM_ARCH_OPT2 ("armv8-m.main",  ARM_ARCH_V8M_MAIN,	FPU_ARCH_VFP,
		 armv8m_main),
  ARM_ARCH_OPT2 ("armv8.1-m.main", ARM_ARCH_V8_1M_MAIN,	FPU_ARCH_VFP,
		 armv8_1m_main),
  ARM_ARCH_OPT2 ("armv8-a",	  ARM_ARCH_V8A,		FPU_ARCH_VFP, armv8a),
  ARM_ARCH_OPT2 ("armv8.1-a",	  ARM_ARCH_V8_1A,	FPU_ARCH_VFP, armv81a),
  ARM_ARCH_OPT2 ("armv8.2-a",	  ARM_ARCH_V8_2A,	FPU_ARCH_VFP, armv82a),
  ARM_ARCH_OPT2 ("armv8.3-a",	  ARM_ARCH_V8_3A,	FPU_ARCH_VFP, armv82a),
  ARM_ARCH_OPT2 ("armv8-r",	  ARM_ARCH_V8R,		FPU_ARCH_VFP, armv8r),
  ARM_ARCH_OPT2 ("armv8.4-a",	  ARM_ARCH_V8_4A,	FPU_ARCH_VFP, armv84a),
  ARM_ARCH_OPT2 ("armv8.5-a",	  ARM_ARCH_V8_5A,	FPU_ARCH_VFP, armv85a),
  ARM_ARCH_OPT2 ("armv8.6-a",	  ARM_ARCH_V8_6A,	FPU_ARCH_VFP, armv86a),
  ARM_ARCH_OPT2 ("armv8.7-a",	  ARM_ARCH_V8_7A,	FPU_ARCH_VFP, armv87a),
  ARM_ARCH_OPT2 ("armv8.8-a",	  ARM_ARCH_V8_8A,	FPU_ARCH_VFP, armv88a),
  ARM_ARCH_OPT2 ("armv9-a",	  ARM_ARCH_V9A,		FPU_ARCH_VFP, armv9a),
  ARM_ARCH_OPT2 ("armv9.1-a",	  ARM_ARCH_V9_1A,	FPU_ARCH_VFP, armv91a),
  ARM_ARCH_OPT2 ("armv9.2-a",	  ARM_ARCH_V9_2A,	FPU_ARCH_VFP, armv92a),
  ARM_ARCH_OPT2 ("armv9.3-a",	  ARM_ARCH_V9_2A,	FPU_ARCH_VFP, armv93a),
  ARM_ARCH_OPT ("xscale",	  ARM_ARCH_XSCALE,	FPU_ARCH_VFP),
  ARM_ARCH_OPT ("iwmmxt",	  ARM_ARCH_IWMMXT,	FPU_ARCH_VFP),
  ARM_ARCH_OPT ("iwmmxt2",	  ARM_ARCH_IWMMXT2,	FPU_ARCH_VFP),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE, NULL }
};
#undef ARM_ARCH_OPT

/* ISA extensions in the co-processor and main instruction set space.  */

struct arm_option_extension_value_table
{
  const char *           name;
  size_t                 name_len;
  const arm_feature_set  merge_value;
  const arm_feature_set  clear_value;
  /* List of architectures for which an extension is available.  ARM_ARCH_NONE
     indicates that an extension is available for all architectures while
     ARM_ANY marks an empty entry.  */
  const arm_feature_set  allowed_archs[2];
};

/* The following table must be in alphabetical order with a NULL last entry.  */

#define ARM_EXT_OPT(N, M, C, AA) { N, sizeof (N) - 1, M, C, { AA, ARM_ANY } }
#define ARM_EXT_OPT2(N, M, C, AA1, AA2) { N, sizeof (N) - 1, M, C, {AA1, AA2} }

/* DEPRECATED: Refrain from using this table to add any new extensions, instead
   use the context sensitive approach using arm_ext_table's.  */
static const struct arm_option_extension_value_table arm_extensions[] =
{
  ARM_EXT_OPT ("crc",	 ARM_FEATURE_CORE_HIGH(ARM_EXT2_CRC),
			 ARM_FEATURE_CORE_HIGH(ARM_EXT2_CRC),
			 ARM_FEATURE_CORE_LOW (ARM_EXT_V8)),
  ARM_EXT_OPT ("crypto", FPU_ARCH_CRYPTO_NEON_VFP_ARMV8,
			 ARM_FEATURE_COPROC (FPU_CRYPTO_ARMV8),
				   ARM_FEATURE_CORE_LOW (ARM_EXT_V8)),
  ARM_EXT_OPT ("dotprod", FPU_ARCH_DOTPROD_NEON_VFP_ARMV8,
			  ARM_FEATURE_COPROC (FPU_NEON_EXT_DOTPROD),
			  ARM_ARCH_V8_2A),
  ARM_EXT_OPT ("dsp",	ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP | ARM_EXT_V6_DSP),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP | ARM_EXT_V6_DSP),
			ARM_FEATURE_CORE (ARM_EXT_V7M, ARM_EXT2_V8M)),
  ARM_EXT_OPT ("fp",     FPU_ARCH_VFP_ARMV8, ARM_FEATURE_COPROC (FPU_VFP_ARMV8),
				   ARM_FEATURE_CORE_LOW (ARM_EXT_V8)),
  ARM_EXT_OPT ("fp16",  ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
			ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST),
			ARM_ARCH_V8_2A),
  ARM_EXT_OPT ("fp16fml",  ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST
						  | ARM_EXT2_FP16_FML),
			   ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST
						  | ARM_EXT2_FP16_FML),
			   ARM_ARCH_V8_2A),
  ARM_EXT_OPT2 ("idiv",	ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV | ARM_EXT_DIV),
			ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV | ARM_EXT_DIV),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V7A),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V7R)),
  /* Duplicate entry for the purpose of allowing ARMv7 to match in presence of
     Thumb divide instruction.  Due to this having the same name as the
     previous entry, this will be ignored when doing command-line parsing and
     only considered by build attribute selection code.  */
  ARM_EXT_OPT ("idiv",	ARM_FEATURE_CORE_LOW (ARM_EXT_DIV),
			ARM_FEATURE_CORE_LOW (ARM_EXT_DIV),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V7)),
  ARM_EXT_OPT ("iwmmxt",ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT),
			ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT), ARM_ARCH_NONE),
  ARM_EXT_OPT ("iwmmxt2", ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT2),
			ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT2), ARM_ARCH_NONE),
  ARM_EXT_OPT ("maverick", ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK),
			ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK), ARM_ARCH_NONE),
  ARM_EXT_OPT2 ("mp",	ARM_FEATURE_CORE_LOW (ARM_EXT_MP),
			ARM_FEATURE_CORE_LOW (ARM_EXT_MP),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V7A),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V7R)),
  ARM_EXT_OPT ("os",	ARM_FEATURE_CORE_LOW (ARM_EXT_OS),
			ARM_FEATURE_CORE_LOW (ARM_EXT_OS),
				   ARM_FEATURE_CORE_LOW (ARM_EXT_V6M)),
  ARM_EXT_OPT ("pan",	ARM_FEATURE_CORE_HIGH (ARM_EXT2_PAN),
			ARM_FEATURE (ARM_EXT_V8, ARM_EXT2_PAN, 0),
			ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8A)),
  ARM_EXT_OPT ("predres", ARM_FEATURE_CORE_HIGH (ARM_EXT2_PREDRES),
			ARM_FEATURE_CORE_HIGH (ARM_EXT2_PREDRES),
			ARM_ARCH_V8A),
  ARM_EXT_OPT ("ras",	ARM_FEATURE_CORE_HIGH (ARM_EXT2_RAS),
			ARM_FEATURE (ARM_EXT_V8, ARM_EXT2_RAS, 0),
			ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8A)),
  ARM_EXT_OPT ("rdma",  FPU_ARCH_NEON_VFP_ARMV8_1,
			ARM_FEATURE_COPROC (FPU_NEON_ARMV8 | FPU_NEON_EXT_RDMA),
			ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8A)),
  ARM_EXT_OPT ("sb",	ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB),
			ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB),
			ARM_ARCH_V8A),
  ARM_EXT_OPT2 ("sec",	ARM_FEATURE_CORE_LOW (ARM_EXT_SEC),
			ARM_FEATURE_CORE_LOW (ARM_EXT_SEC),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V6K),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V7A)),
  ARM_EXT_OPT ("simd",  FPU_ARCH_NEON_VFP_ARMV8,
			ARM_FEATURE_COPROC (FPU_NEON_ARMV8),
			ARM_FEATURE_CORE_LOW (ARM_EXT_V8)),
  ARM_EXT_OPT ("virt",	ARM_FEATURE_CORE_LOW (ARM_EXT_VIRT | ARM_EXT_ADIV
				     | ARM_EXT_DIV),
			ARM_FEATURE_CORE_LOW (ARM_EXT_VIRT),
				   ARM_FEATURE_CORE_LOW (ARM_EXT_V7A)),
  ARM_EXT_OPT ("xscale",ARM_FEATURE_COPROC (ARM_CEXT_XSCALE),
			ARM_FEATURE_COPROC (ARM_CEXT_XSCALE), ARM_ARCH_NONE),
  { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE, { ARM_ARCH_NONE, ARM_ARCH_NONE } }
};
#undef ARM_EXT_OPT

/* ISA floating-point and Advanced SIMD extensions.  */
struct arm_option_fpu_value_table
{
  const char *           name;
  const arm_feature_set  value;
};

/* This list should, at a minimum, contain all the fpu names
   recognized by GCC.  */
static const struct arm_option_fpu_value_table arm_fpus[] =
{
  {"softfpa",		FPU_NONE},
  {"fpe",		FPU_ARCH_FPE},
  {"fpe2",		FPU_ARCH_FPE},
  {"fpe3",		FPU_ARCH_FPA},	/* Third release supports LFM/SFM.  */
  {"fpa",		FPU_ARCH_FPA},
  {"fpa10",		FPU_ARCH_FPA},
  {"fpa11",		FPU_ARCH_FPA},
  {"arm7500fe",		FPU_ARCH_FPA},
  {"softvfp",		FPU_ARCH_VFP},
  {"softvfp+vfp",	FPU_ARCH_VFP_V2},
  {"vfp",		FPU_ARCH_VFP_V2},
  {"vfp9",		FPU_ARCH_VFP_V2},
  {"vfp3",		FPU_ARCH_VFP_V3}, /* Undocumented, use vfpv3.  */
  {"vfp10",		FPU_ARCH_VFP_V2},
  {"vfp10-r0",		FPU_ARCH_VFP_V1},
  {"vfpxd",		FPU_ARCH_VFP_V1xD},
  {"vfpv2",		FPU_ARCH_VFP_V2},
  {"vfpv3",		FPU_ARCH_VFP_V3},
  {"vfpv3-fp16",	FPU_ARCH_VFP_V3_FP16},
  {"vfpv3-d16",		FPU_ARCH_VFP_V3D16},
  {"vfpv3-d16-fp16",	FPU_ARCH_VFP_V3D16_FP16},
  {"vfpv3xd",		FPU_ARCH_VFP_V3xD},
  {"vfpv3xd-fp16",	FPU_ARCH_VFP_V3xD_FP16},
  {"arm1020t",		FPU_ARCH_VFP_V1},
  {"arm1020e",		FPU_ARCH_VFP_V2},
  {"arm1136jfs",	FPU_ARCH_VFP_V2}, /* Undocumented, use arm1136jf-s.  */
  {"arm1136jf-s",	FPU_ARCH_VFP_V2},
  {"maverick",		FPU_ARCH_MAVERICK},
  {"neon",		FPU_ARCH_VFP_V3_PLUS_NEON_V1},
  {"neon-vfpv3",	FPU_ARCH_VFP_V3_PLUS_NEON_V1},
  {"neon-fp16",		FPU_ARCH_NEON_FP16},
  {"vfpv4",		FPU_ARCH_VFP_V4},
  {"vfpv4-d16",		FPU_ARCH_VFP_V4D16},
  {"fpv4-sp-d16",	FPU_ARCH_VFP_V4_SP_D16},
  {"fpv5-d16",		FPU_ARCH_VFP_V5D16},
  {"fpv5-sp-d16",	FPU_ARCH_VFP_V5_SP_D16},
  {"neon-vfpv4",	FPU_ARCH_NEON_VFP_V4},
  {"fp-armv8",		FPU_ARCH_VFP_ARMV8},
  {"neon-fp-armv8",	FPU_ARCH_NEON_VFP_ARMV8},
  {"crypto-neon-fp-armv8",
			FPU_ARCH_CRYPTO_NEON_VFP_ARMV8},
  {"neon-fp-armv8.1",	FPU_ARCH_NEON_VFP_ARMV8_1},
  {"crypto-neon-fp-armv8.1",
			FPU_ARCH_CRYPTO_NEON_VFP_ARMV8_1},
  {NULL,		ARM_ARCH_NONE}
};

struct arm_option_value_table
{
  const char *name;
  long value;
};

static const struct arm_option_value_table arm_float_abis[] =
{
  {"hard",	ARM_FLOAT_ABI_HARD},
  {"softfp",	ARM_FLOAT_ABI_SOFTFP},
  {"soft",	ARM_FLOAT_ABI_SOFT},
  {NULL,	0}
};

#ifdef OBJ_ELF
/* We only know how to output GNU and ver 4/5 (AAELF) formats.  */
static const struct arm_option_value_table arm_eabis[] =
{
  {"gnu",	EF_ARM_EABI_UNKNOWN},
  {"4",		EF_ARM_EABI_VER4},
  {"5",		EF_ARM_EABI_VER5},
  {NULL,	0}
};
#endif

struct arm_long_option_table
{
  const char *option;			/* Substring to match.	*/
  const char *help;			/* Help information.  */
  bool (*func) (const char *subopt);	/* Function to decode sub-option.  */
  const char *deprecated;		/* If non-null, print this message.  */
};

static bool
arm_parse_extension (const char *str, const arm_feature_set *opt_set,
		     arm_feature_set *ext_set,
		     const struct arm_ext_table *ext_table)
{
  /* We insist on extensions being specified in alphabetical order, and with
     extensions being added before being removed.  We achieve this by having
     the global ARM_EXTENSIONS table in alphabetical order, and using the
     ADDING_VALUE variable to indicate whether we are adding an extension (1)
     or removing it (0) and only allowing it to change in the order
     -1 -> 1 -> 0.  */
  const struct arm_option_extension_value_table * opt = NULL;
  const arm_feature_set arm_any = ARM_ANY;
  int adding_value = -1;

  while (str != NULL && *str != 0)
    {
      const char *ext;
      size_t len;

      if (*str != '+')
	{
	  as_bad (_("invalid architectural extension"));
	  return false;
	}

      str++;
      ext = strchr (str, '+');

      if (ext != NULL)
	len = ext - str;
      else
	len = strlen (str);

      if (len >= 2 && startswith (str, "no"))
	{
	  if (adding_value != 0)
	    {
	      adding_value = 0;
	      opt = arm_extensions;
	    }

	  len -= 2;
	  str += 2;
	}
      else if (len > 0)
	{
	  if (adding_value == -1)
	    {
	      adding_value = 1;
	      opt = arm_extensions;
	    }
	  else if (adding_value != 1)
	    {
	      as_bad (_("must specify extensions to add before specifying "
			"those to remove"));
	      return false;
	    }
	}

      if (len == 0)
	{
	  as_bad (_("missing architectural extension"));
	  return false;
	}

      gas_assert (adding_value != -1);
      gas_assert (opt != NULL);

      if (ext_table != NULL)
	{
	  const struct arm_ext_table * ext_opt = ext_table;
	  bool found = false;
	  for (; ext_opt->name != NULL; ext_opt++)
	    if (ext_opt->name_len == len
		&& strncmp (ext_opt->name, str, len) == 0)
	      {
		if (adding_value)
		  {
		    if (ARM_FEATURE_ZERO (ext_opt->merge))
			/* TODO: Option not supported.  When we remove the
			   legacy table this case should error out.  */
			continue;

		    ARM_MERGE_FEATURE_SETS (*ext_set, *ext_set, ext_opt->merge);
		  }
		else
		  {
		    if (ARM_FEATURE_ZERO (ext_opt->clear))
			/* TODO: Option not supported.  When we remove the
			   legacy table this case should error out.  */
			continue;
		    ARM_CLEAR_FEATURE (*ext_set, *ext_set, ext_opt->clear);
		  }
		found = true;
		break;
	      }
	  if (found)
	    {
	      str = ext;
	      continue;
	    }
	}

      /* Scan over the options table trying to find an exact match. */
      for (; opt->name != NULL; opt++)
	if (opt->name_len == len && strncmp (opt->name, str, len) == 0)
	  {
	    int i, nb_allowed_archs =
	      sizeof (opt->allowed_archs) / sizeof (opt->allowed_archs[0]);
	    /* Check we can apply the extension to this architecture.  */
	    for (i = 0; i < nb_allowed_archs; i++)
	      {
		/* Empty entry.  */
		if (ARM_FEATURE_EQUAL (opt->allowed_archs[i], arm_any))
		  continue;
		if (ARM_FSET_CPU_SUBSET (opt->allowed_archs[i], *opt_set))
		  break;
	      }
	    if (i == nb_allowed_archs)
	      {
		as_bad (_("extension does not apply to the base architecture"));
		return false;
	      }

	    /* Add or remove the extension.  */
	    if (adding_value)
	      ARM_MERGE_FEATURE_SETS (*ext_set, *ext_set, opt->merge_value);
	    else
	      ARM_CLEAR_FEATURE (*ext_set, *ext_set, opt->clear_value);

	    /* Allowing Thumb division instructions for ARMv7 in autodetection
	       rely on this break so that duplicate extensions (extensions
	       with the same name as a previous extension in the list) are not
	       considered for command-line parsing.  */
	    break;
	  }

      if (opt->name == NULL)
	{
	  /* Did we fail to find an extension because it wasn't specified in
	     alphabetical order, or because it does not exist?  */

	  for (opt = arm_extensions; opt->name != NULL; opt++)
	    if (opt->name_len == len && strncmp (opt->name, str, len) == 0)
	      break;

	  if (opt->name == NULL)
	    as_bad (_("unknown architectural extension `%s'"), str);
	  else
	    as_bad (_("architectural extensions must be specified in "
		      "alphabetical order"));

	  return false;
	}
      else
	{
	  /* We should skip the extension we've just matched the next time
	     round.  */
	  opt++;
	}

      str = ext;
    };

  return true;
}

static bool
arm_parse_fp16_opt (const char *str)
{
  if (strcasecmp (str, "ieee") == 0)
    fp16_format = ARM_FP16_FORMAT_IEEE;
  else if (strcasecmp (str, "alternative") == 0)
    fp16_format = ARM_FP16_FORMAT_ALTERNATIVE;
  else
    {
      as_bad (_("unrecognised float16 format \"%s\""), str);
      return false;
    }

  return true;
}

static bool
arm_parse_cpu (const char *str)
{
  const struct arm_cpu_option_table *opt;
  const char *ext = strchr (str, '+');
  size_t len;

  if (ext != NULL)
    len = ext - str;
  else
    len = strlen (str);

  if (len == 0)
    {
      as_bad (_("missing cpu name `%s'"), str);
      return false;
    }

  for (opt = arm_cpus; opt->name != NULL; opt++)
    if (opt->name_len == len && strncmp (opt->name, str, len) == 0)
      {
	mcpu_cpu_opt = &opt->value;
	if (mcpu_ext_opt == NULL)
	  mcpu_ext_opt = XNEW (arm_feature_set);
	*mcpu_ext_opt = opt->ext;
	mcpu_fpu_opt = &opt->default_fpu;
	if (opt->canonical_name)
	  {
	    gas_assert (sizeof selected_cpu_name > strlen (opt->canonical_name));
	    strcpy (selected_cpu_name, opt->canonical_name);
	  }
	else
	  {
	    size_t i;

	    if (len >= sizeof selected_cpu_name)
	      len = (sizeof selected_cpu_name) - 1;

	    for (i = 0; i < len; i++)
	      selected_cpu_name[i] = TOUPPER (opt->name[i]);
	    selected_cpu_name[i] = 0;
	  }

	if (ext != NULL)
	  return arm_parse_extension (ext, mcpu_cpu_opt, mcpu_ext_opt, NULL);

	return true;
      }

  as_bad (_("unknown cpu `%s'"), str);
  return false;
}

static bool
arm_parse_arch (const char *str)
{
  const struct arm_arch_option_table *opt;
  const char *ext = strchr (str, '+');
  size_t len;

  if (ext != NULL)
    len = ext - str;
  else
    len = strlen (str);

  if (len == 0)
    {
      as_bad (_("missing architecture name `%s'"), str);
      return false;
    }

  for (opt = arm_archs; opt->name != NULL; opt++)
    if (opt->name_len == len && strncmp (opt->name, str, len) == 0)
      {
	march_cpu_opt = &opt->value;
	if (march_ext_opt == NULL)
	  march_ext_opt = XNEW (arm_feature_set);
	*march_ext_opt = arm_arch_none;
	march_fpu_opt = &opt->default_fpu;
	selected_ctx_ext_table = opt->ext_table;
	strcpy (selected_cpu_name, opt->name);

	if (ext != NULL)
	  return arm_parse_extension (ext, march_cpu_opt, march_ext_opt,
				      opt->ext_table);

	return true;
      }

  as_bad (_("unknown architecture `%s'\n"), str);
  return false;
}

static bool
arm_parse_fpu (const char * str)
{
  const struct arm_option_fpu_value_table * opt;

  for (opt = arm_fpus; opt->name != NULL; opt++)
    if (streq (opt->name, str))
      {
	mfpu_opt = &opt->value;
	return true;
      }

  as_bad (_("unknown floating point format `%s'\n"), str);
  return false;
}

static bool
arm_parse_float_abi (const char * str)
{
  const struct arm_option_value_table * opt;

  for (opt = arm_float_abis; opt->name != NULL; opt++)
    if (streq (opt->name, str))
      {
	mfloat_abi_opt = opt->value;
	return true;
      }

  as_bad (_("unknown floating point abi `%s'\n"), str);
  return false;
}

#ifdef OBJ_ELF
static bool
arm_parse_eabi (const char * str)
{
  const struct arm_option_value_table *opt;

  for (opt = arm_eabis; opt->name != NULL; opt++)
    if (streq (opt->name, str))
      {
	meabi_flags = opt->value;
	return true;
      }
  as_bad (_("unknown EABI `%s'\n"), str);
  return false;
}
#endif

static bool
arm_parse_it_mode (const char * str)
{
  bool ret = true;

  if (streq ("arm", str))
    implicit_it_mode = IMPLICIT_IT_MODE_ARM;
  else if (streq ("thumb", str))
    implicit_it_mode = IMPLICIT_IT_MODE_THUMB;
  else if (streq ("always", str))
    implicit_it_mode = IMPLICIT_IT_MODE_ALWAYS;
  else if (streq ("never", str))
    implicit_it_mode = IMPLICIT_IT_MODE_NEVER;
  else
    {
      as_bad (_("unknown implicit IT mode `%s', should be "\
		"arm, thumb, always, or never."), str);
      ret = false;
    }

  return ret;
}

static bool
arm_ccs_mode (const char * unused ATTRIBUTE_UNUSED)
{
  codecomposer_syntax = true;
  arm_comment_chars[0] = ';';
  arm_line_separator_chars[0] = 0;
  return true;
}

struct arm_long_option_table arm_long_opts[] =
{
  {"mcpu=", N_("<cpu name>\t  assemble for CPU <cpu name>"),
   arm_parse_cpu, NULL},
  {"march=", N_("<arch name>\t  assemble for architecture <arch name>"),
   arm_parse_arch, NULL},
  {"mfpu=", N_("<fpu name>\t  assemble for FPU architecture <fpu name>"),
   arm_parse_fpu, NULL},
  {"mfloat-abi=", N_("<abi>\t  assemble for floating point ABI <abi>"),
   arm_parse_float_abi, NULL},
#ifdef OBJ_ELF
  {"meabi=", N_("<ver>\t\t  assemble for eabi version <ver>"),
   arm_parse_eabi, NULL},
#endif
  {"mimplicit-it=", N_("<mode>\t  controls implicit insertion of IT instructions"),
   arm_parse_it_mode, NULL},
  {"mccs", N_("\t\t\t  TI CodeComposer Studio syntax compatibility mode"),
   arm_ccs_mode, NULL},
  {"mfp16-format=",
   N_("[ieee|alternative]\n\
                          set the encoding for half precision floating point "
			  "numbers to IEEE\n\
                          or Arm alternative format."),
   arm_parse_fp16_opt, NULL },
  {NULL, NULL, 0, NULL}
};

int
md_parse_option (int c, const char * arg)
{
  struct arm_option_table *opt;
  const struct arm_legacy_option_table *fopt;
  struct arm_long_option_table *lopt;

  switch (c)
    {
#ifdef OPTION_EB
    case OPTION_EB:
      target_big_endian = 1;
      break;
#endif

#ifdef OPTION_EL
    case OPTION_EL:
      target_big_endian = 0;
      break;
#endif

    case OPTION_FIX_V4BX:
      fix_v4bx = true;
      break;

#ifdef OBJ_ELF
    case OPTION_FDPIC:
      arm_fdpic = true;
      break;
#endif /* OBJ_ELF */

    case 'a':
      /* Listing option.  Just ignore these, we don't support additional
	 ones.	*/
      return 0;

    default:
      for (opt = arm_opts; opt->option != NULL; opt++)
	{
	  if (c == opt->option[0]
	      && ((arg == NULL && opt->option[1] == 0)
		  || streq (arg, opt->option + 1)))
	    {
	      /* If the option is deprecated, tell the user.  */
	      if (warn_on_deprecated && opt->deprecated != NULL)
		as_tsktsk (_("option `-%c%s' is deprecated: %s"), c,
			   arg ? arg : "", _(opt->deprecated));

	      if (opt->var != NULL)
		*opt->var = opt->value;

	      return 1;
	    }
	}

      for (fopt = arm_legacy_opts; fopt->option != NULL; fopt++)
	{
	  if (c == fopt->option[0]
	      && ((arg == NULL && fopt->option[1] == 0)
		  || streq (arg, fopt->option + 1)))
	    {
	      /* If the option is deprecated, tell the user.  */
	      if (warn_on_deprecated && fopt->deprecated != NULL)
		as_tsktsk (_("option `-%c%s' is deprecated: %s"), c,
			   arg ? arg : "", _(fopt->deprecated));

	      if (fopt->var != NULL)
		*fopt->var = &fopt->value;

	      return 1;
	    }
	}

      for (lopt = arm_long_opts; lopt->option != NULL; lopt++)
	{
	  /* These options are expected to have an argument.  */
	  if (c == lopt->option[0]
	      && arg != NULL
	      && strncmp (arg, lopt->option + 1,
			  strlen (lopt->option + 1)) == 0)
	    {
	      /* If the option is deprecated, tell the user.  */
	      if (warn_on_deprecated && lopt->deprecated != NULL)
		as_tsktsk (_("option `-%c%s' is deprecated: %s"), c, arg,
			   _(lopt->deprecated));

	      /* Call the sup-option parser.  */
	      return lopt->func (arg + strlen (lopt->option) - 1);
	    }
	}

      return 0;
    }

  return 1;
}

void
md_show_usage (FILE * fp)
{
  struct arm_option_table *opt;
  struct arm_long_option_table *lopt;

  fprintf (fp, _(" ARM-specific assembler options:\n"));

  for (opt = arm_opts; opt->option != NULL; opt++)
    if (opt->help != NULL)
      fprintf (fp, "  -%-23s%s\n", opt->option, _(opt->help));

  for (lopt = arm_long_opts; lopt->option != NULL; lopt++)
    if (lopt->help != NULL)
      fprintf (fp, "  -%s%s\n", lopt->option, _(lopt->help));

#ifdef OPTION_EB
  fprintf (fp, _("\
  -EB                     assemble code for a big-endian cpu\n"));
#endif

#ifdef OPTION_EL
  fprintf (fp, _("\
  -EL                     assemble code for a little-endian cpu\n"));
#endif

  fprintf (fp, _("\
  --fix-v4bx              Allow BX in ARMv4 code\n"));

#ifdef OBJ_ELF
  fprintf (fp, _("\
  --fdpic                 generate an FDPIC object file\n"));
#endif /* OBJ_ELF */
}

#ifdef OBJ_ELF

typedef struct
{
  int val;
  arm_feature_set flags;
} cpu_arch_ver_table;

/* Mapping from CPU features to EABI CPU arch values.  Table must be sorted
   chronologically for architectures, with an exception for ARMv6-M and
   ARMv6S-M due to legacy reasons.  No new architecture should have a
   special case.  This allows for build attribute selection results to be
   stable when new architectures are added.  */
static const cpu_arch_ver_table cpu_arch_ver[] =
{
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V1},
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V2},
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V2S},
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V3},
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V3M},
    {TAG_CPU_ARCH_V4,	      ARM_ARCH_V4xM},
    {TAG_CPU_ARCH_V4,	      ARM_ARCH_V4},
    {TAG_CPU_ARCH_V4T,	      ARM_ARCH_V4TxM},
    {TAG_CPU_ARCH_V4T,	      ARM_ARCH_V4T},
    {TAG_CPU_ARCH_V5T,	      ARM_ARCH_V5xM},
    {TAG_CPU_ARCH_V5T,	      ARM_ARCH_V5},
    {TAG_CPU_ARCH_V5T,	      ARM_ARCH_V5TxM},
    {TAG_CPU_ARCH_V5T,	      ARM_ARCH_V5T},
    {TAG_CPU_ARCH_V5TE,	      ARM_ARCH_V5TExP},
    {TAG_CPU_ARCH_V5TE,	      ARM_ARCH_V5TE},
    {TAG_CPU_ARCH_V5TEJ,      ARM_ARCH_V5TEJ},
    {TAG_CPU_ARCH_V6,	      ARM_ARCH_V6},
    {TAG_CPU_ARCH_V6KZ,	      ARM_ARCH_V6Z},
    {TAG_CPU_ARCH_V6KZ,	      ARM_ARCH_V6KZ},
    {TAG_CPU_ARCH_V6K,	      ARM_ARCH_V6K},
    {TAG_CPU_ARCH_V6T2,	      ARM_ARCH_V6T2},
    {TAG_CPU_ARCH_V6T2,	      ARM_ARCH_V6KT2},
    {TAG_CPU_ARCH_V6T2,	      ARM_ARCH_V6ZT2},
    {TAG_CPU_ARCH_V6T2,	      ARM_ARCH_V6KZT2},

    /* When assembling a file with only ARMv6-M or ARMv6S-M instruction, GNU as
       always selected build attributes to match those of ARMv6-M
       (resp. ARMv6S-M).  However, due to these architectures being a strict
       subset of ARMv7-M in terms of instructions available, ARMv7-M attributes
       would be selected when fully respecting chronology of architectures.
       It is thus necessary to make a special case of ARMv6-M and ARMv6S-M and
       move them before ARMv7 architectures.  */
    {TAG_CPU_ARCH_V6_M,	      ARM_ARCH_V6M},
    {TAG_CPU_ARCH_V6S_M,      ARM_ARCH_V6SM},

    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7},
    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7A},
    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7R},
    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7M},
    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7VE},
    {TAG_CPU_ARCH_V7E_M,      ARM_ARCH_V7EM},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8A},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_1A},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_2A},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_3A},
    {TAG_CPU_ARCH_V8M_BASE,   ARM_ARCH_V8M_BASE},
    {TAG_CPU_ARCH_V8M_MAIN,   ARM_ARCH_V8M_MAIN},
    {TAG_CPU_ARCH_V8R,	      ARM_ARCH_V8R},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_4A},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_5A},
    {TAG_CPU_ARCH_V8_1M_MAIN, ARM_ARCH_V8_1M_MAIN},
    {TAG_CPU_ARCH_V8,	    ARM_ARCH_V8_6A},
    {TAG_CPU_ARCH_V8,	    ARM_ARCH_V8_7A},
    {TAG_CPU_ARCH_V8,	    ARM_ARCH_V8_8A},
    {TAG_CPU_ARCH_V9,	    ARM_ARCH_V9A},
    {TAG_CPU_ARCH_V9,	    ARM_ARCH_V9_1A},
    {TAG_CPU_ARCH_V9,	    ARM_ARCH_V9_2A},
    {TAG_CPU_ARCH_V9,	    ARM_ARCH_V9_3A},
    {-1,		    ARM_ARCH_NONE}
};

/* Set an attribute if it has not already been set by the user.  */

static void
aeabi_set_attribute_int (int tag, int value)
{
  if (tag < 1
      || tag >= NUM_KNOWN_OBJ_ATTRIBUTES
      || !attributes_set_explicitly[tag])
    bfd_elf_add_proc_attr_int (stdoutput, tag, value);
}

static void
aeabi_set_attribute_string (int tag, const char *value)
{
  if (tag < 1
      || tag >= NUM_KNOWN_OBJ_ATTRIBUTES
      || !attributes_set_explicitly[tag])
    bfd_elf_add_proc_attr_string (stdoutput, tag, value);
}

/* Return whether features in the *NEEDED feature set are available via
   extensions for the architecture whose feature set is *ARCH_FSET.  */

static bool
have_ext_for_needed_feat_p (const arm_feature_set *arch_fset,
			    const arm_feature_set *needed)
{
  int i, nb_allowed_archs;
  arm_feature_set ext_fset;
  const struct arm_option_extension_value_table *opt;

  ext_fset = arm_arch_none;
  for (opt = arm_extensions; opt->name != NULL; opt++)
    {
      /* Extension does not provide any feature we need.  */
      if (!ARM_CPU_HAS_FEATURE (*needed, opt->merge_value))
	continue;

      nb_allowed_archs =
	sizeof (opt->allowed_archs) / sizeof (opt->allowed_archs[0]);
      for (i = 0; i < nb_allowed_archs; i++)
	{
	  /* Empty entry.  */
	  if (ARM_FEATURE_EQUAL (opt->allowed_archs[i], arm_arch_any))
	    break;

	  /* Extension is available, add it.  */
	  if (ARM_FSET_CPU_SUBSET (opt->allowed_archs[i], *arch_fset))
	    ARM_MERGE_FEATURE_SETS (ext_fset, ext_fset, opt->merge_value);
	}
    }

  /* Can we enable all features in *needed?  */
  return ARM_FSET_CPU_SUBSET (*needed, ext_fset);
}

/* Select value for Tag_CPU_arch and Tag_CPU_arch_profile build attributes for
   a given architecture feature set *ARCH_EXT_FSET including extension feature
   set *EXT_FSET.  Selection logic used depend on EXACT_MATCH:
   - if true, check for an exact match of the architecture modulo extensions;
   - otherwise, select build attribute value of the first superset
     architecture released so that results remains stable when new architectures
     are added.
   For -march/-mcpu=all the build attribute value of the most featureful
   architecture is returned.  Tag_CPU_arch_profile result is returned in
   PROFILE.  */

static int
get_aeabi_cpu_arch_from_fset (const arm_feature_set *arch_ext_fset,
			      const arm_feature_set *ext_fset,
			      char *profile, int exact_match)
{
  arm_feature_set arch_fset;
  const cpu_arch_ver_table *p_ver, *p_ver_ret = NULL;

  /* Select most featureful architecture with all its extensions if building
     for -march=all as the feature sets used to set build attributes.  */
  if (ARM_FEATURE_EQUAL (*arch_ext_fset, arm_arch_any))
    {
      /* Force revisiting of decision for each new architecture.  */
      gas_assert (MAX_TAG_CPU_ARCH <= TAG_CPU_ARCH_V9);
      *profile = 'A';
      return TAG_CPU_ARCH_V9;
    }

  ARM_CLEAR_FEATURE (arch_fset, *arch_ext_fset, *ext_fset);

  for (p_ver = cpu_arch_ver; p_ver->val != -1; p_ver++)
    {
      arm_feature_set known_arch_fset;

      ARM_CLEAR_FEATURE (known_arch_fset, p_ver->flags, fpu_any);
      if (exact_match)
	{
	  /* Base architecture match user-specified architecture and
	     extensions, eg. ARMv6S-M matching -march=armv6-m+os.  */
	  if (ARM_FEATURE_EQUAL (*arch_ext_fset, known_arch_fset))
	    {
	      p_ver_ret = p_ver;
	      goto found;
	    }
	  /* Base architecture match user-specified architecture only
	     (eg. ARMv6-M in the same case as above).  Record it in case we
	     find a match with above condition.  */
	  else if (p_ver_ret == NULL
		   && ARM_FEATURE_EQUAL (arch_fset, known_arch_fset))
	    p_ver_ret = p_ver;
	}
      else
	{

	  /* Architecture has all features wanted.  */
	  if (ARM_FSET_CPU_SUBSET (arch_fset, known_arch_fset))
	    {
	      arm_feature_set added_fset;

	      /* Compute features added by this architecture over the one
		 recorded in p_ver_ret.  */
	      if (p_ver_ret != NULL)
		ARM_CLEAR_FEATURE (added_fset, known_arch_fset,
				   p_ver_ret->flags);
	      /* First architecture that match incl. with extensions, or the
		 only difference in features over the recorded match is
		 features that were optional and are now mandatory.  */
	      if (p_ver_ret == NULL
		  || ARM_FSET_CPU_SUBSET (added_fset, arch_fset))
		{
		  p_ver_ret = p_ver;
		  goto found;
		}
	    }
	  else if (p_ver_ret == NULL)
	    {
	      arm_feature_set needed_ext_fset;

	      ARM_CLEAR_FEATURE (needed_ext_fset, arch_fset, known_arch_fset);

	      /* Architecture has all features needed when using some
		 extensions.  Record it and continue searching in case there
		 exist an architecture providing all needed features without
		 the need for extensions (eg. ARMv6S-M Vs ARMv6-M with
		 OS extension).  */
	      if (have_ext_for_needed_feat_p (&known_arch_fset,
					      &needed_ext_fset))
		p_ver_ret = p_ver;
	    }
	}
    }

  if (p_ver_ret == NULL)
    return -1;

 found:
  /* Tag_CPU_arch_profile.  */
  if (!ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v8r)
      && (ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v7a)
          || ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v8)
          || (ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_atomics)
              && !ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v8m_m_only))))
    *profile = 'A';
  else if (ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v7r)
      || ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v8r))
    *profile = 'R';
  else if (ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_m))
    *profile = 'M';
  else
    *profile = '\0';
  return p_ver_ret->val;
}

/* Set the public EABI object attributes.  */

static void
aeabi_set_public_attributes (void)
{
  char profile = '\0';
  int arch = -1;
  int virt_sec = 0;
  int fp16_optional = 0;
  int skip_exact_match = 0;
  arm_feature_set flags, flags_arch, flags_ext;

  /* Autodetection mode, choose the architecture based the instructions
     actually used.  */
  if (no_cpu_selected ())
    {
      ARM_MERGE_FEATURE_SETS (flags, arm_arch_used, thumb_arch_used);

      if (ARM_CPU_HAS_FEATURE (arm_arch_used, arm_arch_any))
	ARM_MERGE_FEATURE_SETS (flags, flags, arm_ext_v1);

      if (ARM_CPU_HAS_FEATURE (thumb_arch_used, arm_arch_any))
	ARM_MERGE_FEATURE_SETS (flags, flags, arm_ext_v4t);

      /* Code run during relaxation relies on selected_cpu being set.  */
      ARM_CLEAR_FEATURE (flags_arch, flags, fpu_any);
      flags_ext = arm_arch_none;
      ARM_CLEAR_FEATURE (selected_arch, flags_arch, flags_ext);
      selected_ext = flags_ext;
      selected_cpu = flags;
    }
  /* Otherwise, choose the architecture based on the capabilities of the
     requested cpu.  */
  else
    {
      ARM_MERGE_FEATURE_SETS (flags_arch, selected_arch, selected_ext);
      ARM_CLEAR_FEATURE (flags_arch, flags_arch, fpu_any);
      flags_ext = selected_ext;
      flags = selected_cpu;
    }
  ARM_MERGE_FEATURE_SETS (flags, flags, selected_fpu);

  /* Allow the user to override the reported architecture.  */
  if (!ARM_FEATURE_ZERO (selected_object_arch))
    {
      ARM_CLEAR_FEATURE (flags_arch, selected_object_arch, fpu_any);
      flags_ext = arm_arch_none;
    }
  else
    skip_exact_match = ARM_FEATURE_EQUAL (selected_cpu, arm_arch_any);

  /* When this function is run again after relaxation has happened there is no
     way to determine whether an architecture or CPU was specified by the user:
     - selected_cpu is set above for relaxation to work;
     - march_cpu_opt is not set if only -mcpu or .cpu is used;
     - mcpu_cpu_opt is set to arm_arch_any for autodetection.
     Therefore, if not in -march=all case we first try an exact match and fall
     back to autodetection.  */
  if (!skip_exact_match)
    arch = get_aeabi_cpu_arch_from_fset (&flags_arch, &flags_ext, &profile, 1);
  if (arch == -1)
    arch = get_aeabi_cpu_arch_from_fset (&flags_arch, &flags_ext, &profile, 0);
  if (arch == -1)
    as_bad (_("no architecture contains all the instructions used\n"));

  /* Tag_CPU_name.  */
  if (selected_cpu_name[0])
    {
      char *q;

      q = selected_cpu_name;
      if (startswith (q, "armv"))
	{
	  int i;

	  q += 4;
	  for (i = 0; q[i]; i++)
	    q[i] = TOUPPER (q[i]);
	}
      aeabi_set_attribute_string (Tag_CPU_name, q);
    }

  /* Tag_CPU_arch.  */
  aeabi_set_attribute_int (Tag_CPU_arch, arch);

  /* Tag_CPU_arch_profile.  */
  if (profile != '\0')
    aeabi_set_attribute_int (Tag_CPU_arch_profile, profile);

  /* Tag_DSP_extension.  */
  if (ARM_CPU_HAS_FEATURE (selected_ext, arm_ext_dsp))
    aeabi_set_attribute_int (Tag_DSP_extension, 1);

  ARM_CLEAR_FEATURE (flags_arch, flags, fpu_any);
  /* Tag_ARM_ISA_use.  */
  if (ARM_CPU_HAS_FEATURE (flags, arm_ext_v1)
      || ARM_FEATURE_ZERO (flags_arch))
    aeabi_set_attribute_int (Tag_ARM_ISA_use, 1);

  /* Tag_THUMB_ISA_use.  */
  if (ARM_CPU_HAS_FEATURE (flags, arm_ext_v4t)
      || ARM_FEATURE_ZERO (flags_arch))
    {
      int thumb_isa_use;

      if (!ARM_CPU_HAS_FEATURE (flags, arm_ext_v8)
	  && ARM_CPU_HAS_FEATURE (flags, arm_ext_v8m_m_only))
	thumb_isa_use = 3;
      else if (ARM_CPU_HAS_FEATURE (flags, arm_arch_t2))
	thumb_isa_use = 2;
      else
	thumb_isa_use = 1;
      aeabi_set_attribute_int (Tag_THUMB_ISA_use, thumb_isa_use);
    }

  /* Tag_VFP_arch.  */
  if (ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_armv8xd))
    aeabi_set_attribute_int (Tag_VFP_arch,
			     ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_d32)
			     ? 7 : 8);
  else if (ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_fma))
    aeabi_set_attribute_int (Tag_VFP_arch,
			     ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_d32)
			     ? 5 : 6);
  else if (ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_d32))
    {
      fp16_optional = 1;
      aeabi_set_attribute_int (Tag_VFP_arch, 3);
    }
  else if (ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_v3xd))
    {
      aeabi_set_attribute_int (Tag_VFP_arch, 4);
      fp16_optional = 1;
    }
  else if (ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_v2))
    aeabi_set_attribute_int (Tag_VFP_arch, 2);
  else if (ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_v1)
	   || ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_v1xd))
    aeabi_set_attribute_int (Tag_VFP_arch, 1);

  /* Tag_ABI_HardFP_use.  */
  if (ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_v1xd)
      && !ARM_CPU_HAS_FEATURE (flags, fpu_vfp_ext_v1))
    aeabi_set_attribute_int (Tag_ABI_HardFP_use, 1);

  /* Tag_WMMX_arch.  */
  if (ARM_CPU_HAS_FEATURE (flags, arm_cext_iwmmxt2))
    aeabi_set_attribute_int (Tag_WMMX_arch, 2);
  else if (ARM_CPU_HAS_FEATURE (flags, arm_cext_iwmmxt))
    aeabi_set_attribute_int (Tag_WMMX_arch, 1);

  /* Tag_Advanced_SIMD_arch (formerly Tag_NEON_arch).  */
  if (ARM_CPU_HAS_FEATURE (flags, fpu_neon_ext_v8_1))
    aeabi_set_attribute_int (Tag_Advanced_SIMD_arch, 4);
  else if (ARM_CPU_HAS_FEATURE (flags, fpu_neon_ext_armv8))
    aeabi_set_attribute_int (Tag_Advanced_SIMD_arch, 3);
  else if (ARM_CPU_HAS_FEATURE (flags, fpu_neon_ext_v1))
    {
      if (ARM_CPU_HAS_FEATURE (flags, fpu_neon_ext_fma))
	{
	  aeabi_set_attribute_int (Tag_Advanced_SIMD_arch, 2);
	}
      else
	{
	  aeabi_set_attribute_int (Tag_Advanced_SIMD_arch, 1);
	  fp16_optional = 1;
	}
    }

  if (ARM_CPU_HAS_FEATURE (flags, mve_fp_ext))
    aeabi_set_attribute_int (Tag_MVE_arch, 2);
  else if (ARM_CPU_HAS_FEATURE (flags, mve_ext))
    aeabi_set_attribute_int (Tag_MVE_arch, 1);

  /* Tag_VFP_HP_extension (formerly Tag_NEON_FP16_arch).  */
  if (ARM_CPU_HAS_FEATURE (flags, fpu_vfp_fp16) && fp16_optional)
    aeabi_set_attribute_int (Tag_VFP_HP_extension, 1);

  /* Tag_DIV_use.

     We set Tag_DIV_use to two when integer divide instructions have been used
     in ARM state, or when Thumb integer divide instructions have been used,
     but we have no architecture profile set, nor have we any ARM instructions.

     For ARMv8-A and ARMv8-M we set the tag to 0 as integer divide is implied
     by the base architecture.

     For new architectures we will have to check these tests.  */
  gas_assert (arch <= TAG_CPU_ARCH_V9);
  if (ARM_CPU_HAS_FEATURE (flags, arm_ext_v8)
      || ARM_CPU_HAS_FEATURE (flags, arm_ext_v8m))
    aeabi_set_attribute_int (Tag_DIV_use, 0);
  else if (ARM_CPU_HAS_FEATURE (flags, arm_ext_adiv)
	   || (profile == '\0'
	       && ARM_CPU_HAS_FEATURE (flags, arm_ext_div)
	       && !ARM_CPU_HAS_FEATURE (arm_arch_used, arm_arch_any)))
    aeabi_set_attribute_int (Tag_DIV_use, 2);

  /* Tag_MP_extension_use.  */
  if (ARM_CPU_HAS_FEATURE (flags, arm_ext_mp))
    aeabi_set_attribute_int (Tag_MPextension_use, 1);

  /* Tag Virtualization_use.  */
  if (ARM_CPU_HAS_FEATURE (flags, arm_ext_sec))
    virt_sec |= 1;
  if (ARM_CPU_HAS_FEATURE (flags, arm_ext_virt))
    virt_sec |= 2;
  if (virt_sec != 0)
    aeabi_set_attribute_int (Tag_Virtualization_use, virt_sec);

  if (fp16_format != ARM_FP16_FORMAT_DEFAULT)
    aeabi_set_attribute_int (Tag_ABI_FP_16bit_format, fp16_format);
}

/* Post relaxation hook.  Recompute ARM attributes now that relaxation is
   finished and free extension feature bits which will not be used anymore.  */

void
arm_md_post_relax (void)
{
  aeabi_set_public_attributes ();
  XDELETE (mcpu_ext_opt);
  mcpu_ext_opt = NULL;
  XDELETE (march_ext_opt);
  march_ext_opt = NULL;
}

/* Add the default contents for the .ARM.attributes section.  */

void
arm_md_finish (void)
{
  if (EF_ARM_EABI_VERSION (meabi_flags) < EF_ARM_EABI_VER4)
    return;

  aeabi_set_public_attributes ();
}
#endif /* OBJ_ELF */

/* Parse a .cpu directive.  */

static void
s_arm_cpu (int ignored ATTRIBUTE_UNUSED)
{
  const struct arm_cpu_option_table *opt;
  char *name;
  char saved_char;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (!*name)
    {
      as_bad (_(".cpu: missing cpu name"));
      *input_line_pointer = saved_char;
      return;
    }

  /* Skip the first "all" entry.  */
  for (opt = arm_cpus + 1; opt->name != NULL; opt++)
    if (streq (opt->name, name))
      {
	selected_arch = opt->value;
	selected_ext = opt->ext;
	ARM_MERGE_FEATURE_SETS (selected_cpu, selected_arch, selected_ext);
	if (opt->canonical_name)
	  strcpy (selected_cpu_name, opt->canonical_name);
	else
	  {
	    int i;
	    for (i = 0; opt->name[i]; i++)
	      selected_cpu_name[i] = TOUPPER (opt->name[i]);

	    selected_cpu_name[i] = 0;
	  }
	ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);

	*input_line_pointer = saved_char;
	demand_empty_rest_of_line ();
	return;
      }
  as_bad (_("unknown cpu `%s'"), name);
  *input_line_pointer = saved_char;
}

/* Parse a .arch directive.  */

static void
s_arm_arch (int ignored ATTRIBUTE_UNUSED)
{
  const struct arm_arch_option_table *opt;
  char saved_char;
  char *name;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (!*name)
    {
      as_bad (_(".arch: missing architecture name"));
      *input_line_pointer = saved_char;
      return;
    }

  /* Skip the first "all" entry.  */
  for (opt = arm_archs + 1; opt->name != NULL; opt++)
    if (streq (opt->name, name))
      {
	selected_arch = opt->value;
	selected_ctx_ext_table = opt->ext_table;
	selected_ext = arm_arch_none;
	selected_cpu = selected_arch;
	strcpy (selected_cpu_name, opt->name);
	ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
	*input_line_pointer = saved_char;
	demand_empty_rest_of_line ();
	return;
      }

  as_bad (_("unknown architecture `%s'\n"), name);
  *input_line_pointer = saved_char;
  ignore_rest_of_line ();
}

/* Parse a .object_arch directive.  */

static void
s_arm_object_arch (int ignored ATTRIBUTE_UNUSED)
{
  const struct arm_arch_option_table *opt;
  char saved_char;
  char *name;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (!*name)
    {
      as_bad (_(".object_arch: missing architecture name"));
      *input_line_pointer = saved_char;
      return;
    }

  /* Skip the first "all" entry.  */
  for (opt = arm_archs + 1; opt->name != NULL; opt++)
    if (streq (opt->name, name))
      {
	selected_object_arch = opt->value;
	*input_line_pointer = saved_char;
	demand_empty_rest_of_line ();
	return;
      }

  as_bad (_("unknown architecture `%s'\n"), name);
  *input_line_pointer = saved_char;
  ignore_rest_of_line ();
}

/* Parse a .arch_extension directive.  */

static void
s_arm_arch_extension (int ignored ATTRIBUTE_UNUSED)
{
  const struct arm_option_extension_value_table *opt;
  char saved_char;
  char *name;
  int adding_value = 1;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (!*name)
    {
      as_bad (_(".arch_extension: missing architecture extension"));
      *input_line_pointer = saved_char;
      return;
    }

  if (strlen (name) >= 2
      && startswith (name, "no"))
    {
      adding_value = 0;
      name += 2;
    }

  /* Check the context specific extension table */
  if (selected_ctx_ext_table)
    {
      const struct arm_ext_table * ext_opt;
      for (ext_opt = selected_ctx_ext_table; ext_opt->name != NULL; ext_opt++)
        {
          if (streq (ext_opt->name, name))
	    {
	      if (adding_value)
		{
		  if (ARM_FEATURE_ZERO (ext_opt->merge))
		    /* TODO: Option not supported.  When we remove the
		    legacy table this case should error out.  */
		    continue;
		  ARM_MERGE_FEATURE_SETS (selected_ext, selected_ext,
					  ext_opt->merge);
		}
	      else
		ARM_CLEAR_FEATURE (selected_ext, selected_ext, ext_opt->clear);

	      ARM_MERGE_FEATURE_SETS (selected_cpu, selected_arch, selected_ext);
	      ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
	      *input_line_pointer = saved_char;
	      demand_empty_rest_of_line ();
	      return;
	    }
	}
    }

  for (opt = arm_extensions; opt->name != NULL; opt++)
    if (streq (opt->name, name))
      {
	int i, nb_allowed_archs =
	  sizeof (opt->allowed_archs) / sizeof (opt->allowed_archs[i]);
	for (i = 0; i < nb_allowed_archs; i++)
	  {
	    /* Empty entry.  */
	    if (ARM_CPU_IS_ANY (opt->allowed_archs[i]))
	      continue;
	    if (ARM_FSET_CPU_SUBSET (opt->allowed_archs[i], selected_arch))
	      break;
	  }

	if (i == nb_allowed_archs)
	  {
	    as_bad (_("architectural extension `%s' is not allowed for the "
		      "current base architecture"), name);
	    break;
	  }

	if (adding_value)
	  ARM_MERGE_FEATURE_SETS (selected_ext, selected_ext,
				  opt->merge_value);
	else
	  ARM_CLEAR_FEATURE (selected_ext, selected_ext, opt->clear_value);

	ARM_MERGE_FEATURE_SETS (selected_cpu, selected_arch, selected_ext);
	ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
	*input_line_pointer = saved_char;
	demand_empty_rest_of_line ();
	/* Allowing Thumb division instructions for ARMv7 in autodetection rely
	   on this return so that duplicate extensions (extensions with the
	   same name as a previous extension in the list) are not considered
	   for command-line parsing.  */
	return;
      }

  if (opt->name == NULL)
    as_bad (_("unknown architecture extension `%s'\n"), name);

  *input_line_pointer = saved_char;
}

/* Parse a .fpu directive.  */

static void
s_arm_fpu (int ignored ATTRIBUTE_UNUSED)
{
  const struct arm_option_fpu_value_table *opt;
  char saved_char;
  char *name;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (!*name)
    {
      as_bad (_(".fpu: missing fpu name"));
      *input_line_pointer = saved_char;
      return;
    }

  for (opt = arm_fpus; opt->name != NULL; opt++)
    if (streq (opt->name, name))
      {
	selected_fpu = opt->value;
	ARM_CLEAR_FEATURE (selected_cpu, selected_cpu, fpu_any);
#ifndef CPU_DEFAULT
	if (no_cpu_selected ())
	  ARM_MERGE_FEATURE_SETS (cpu_variant, arm_arch_any, selected_fpu);
	else
#endif
	  ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
	*input_line_pointer = saved_char;
	return;
      }

  as_bad (_("unknown floating point format `%s'\n"), name);
  *input_line_pointer = saved_char;
  ignore_rest_of_line ();
}

/* Copy symbol information.  */

void
arm_copy_symbol_attributes (symbolS *dest, symbolS *src)
{
  ARM_GET_FLAG (dest) = ARM_GET_FLAG (src);
}

#ifdef OBJ_ELF
/* Given a symbolic attribute NAME, return the proper integer value.
   Returns -1 if the attribute is not known.  */

int
arm_convert_symbolic_attribute (const char *name)
{
  static const struct
  {
    const char * name;
    const int    tag;
  }
  attribute_table[] =
    {
      /* When you modify this table you should
	 also modify the list in doc/c-arm.texi.  */
#define T(tag) {#tag, tag}
      T (Tag_CPU_raw_name),
      T (Tag_CPU_name),
      T (Tag_CPU_arch),
      T (Tag_CPU_arch_profile),
      T (Tag_ARM_ISA_use),
      T (Tag_THUMB_ISA_use),
      T (Tag_FP_arch),
      T (Tag_VFP_arch),
      T (Tag_WMMX_arch),
      T (Tag_Advanced_SIMD_arch),
      T (Tag_PCS_config),
      T (Tag_ABI_PCS_R9_use),
      T (Tag_ABI_PCS_RW_data),
      T (Tag_ABI_PCS_RO_data),
      T (Tag_ABI_PCS_GOT_use),
      T (Tag_ABI_PCS_wchar_t),
      T (Tag_ABI_FP_rounding),
      T (Tag_ABI_FP_denormal),
      T (Tag_ABI_FP_exceptions),
      T (Tag_ABI_FP_user_exceptions),
      T (Tag_ABI_FP_number_model),
      T (Tag_ABI_align_needed),
      T (Tag_ABI_align8_needed),
      T (Tag_ABI_align_preserved),
      T (Tag_ABI_align8_preserved),
      T (Tag_ABI_enum_size),
      T (Tag_ABI_HardFP_use),
      T (Tag_ABI_VFP_args),
      T (Tag_ABI_WMMX_args),
      T (Tag_ABI_optimization_goals),
      T (Tag_ABI_FP_optimization_goals),
      T (Tag_compatibility),
      T (Tag_CPU_unaligned_access),
      T (Tag_FP_HP_extension),
      T (Tag_VFP_HP_extension),
      T (Tag_ABI_FP_16bit_format),
      T (Tag_MPextension_use),
      T (Tag_DIV_use),
      T (Tag_nodefaults),
      T (Tag_also_compatible_with),
      T (Tag_conformance),
      T (Tag_T2EE_use),
      T (Tag_Virtualization_use),
      T (Tag_DSP_extension),
      T (Tag_MVE_arch),
      T (Tag_PAC_extension),
      T (Tag_BTI_extension),
      T (Tag_BTI_use),
      T (Tag_PACRET_use),
      /* We deliberately do not include Tag_MPextension_use_legacy.  */
#undef T
    };
  unsigned int i;

  if (name == NULL)
    return -1;

  for (i = 0; i < ARRAY_SIZE (attribute_table); i++)
    if (streq (name, attribute_table[i].name))
      return attribute_table[i].tag;

  return -1;
}

/* Apply sym value for relocations only in the case that they are for
   local symbols in the same segment as the fixup and you have the
   respective architectural feature for blx and simple switches.  */

int
arm_apply_sym_value (struct fix * fixP, segT this_seg)
{
  if (fixP->fx_addsy
      && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t)
      /* PR 17444: If the local symbol is in a different section then a reloc
	 will always be generated for it, so applying the symbol value now
	 will result in a double offset being stored in the relocation.  */
      && (S_GET_SEGMENT (fixP->fx_addsy) == this_seg)
      && !S_FORCE_RELOC (fixP->fx_addsy, true))
    {
      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_ARM_PCREL_BLX:
	case BFD_RELOC_THUMB_PCREL_BRANCH23:
	  if (ARM_IS_FUNC (fixP->fx_addsy))
	    return 1;
	  break;

	case BFD_RELOC_ARM_PCREL_CALL:
	case BFD_RELOC_THUMB_PCREL_BLX:
	  if (THUMB_IS_FUNC (fixP->fx_addsy))
	    return 1;
	  break;

	default:
	  break;
	}

    }
  return 0;
}
#endif /* OBJ_ELF */
