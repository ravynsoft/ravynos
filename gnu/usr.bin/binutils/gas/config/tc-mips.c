/* tc-mips.c -- assemble code for a MIPS chip.
   Copyright (C) 1993-2023 Free Software Foundation, Inc.
   Contributed by the OSF and Ralph Campbell.
   Written by Keith Knowles and Ralph Campbell, working independently.
   Modified for ECOFF and R4000 support by Ian Lance Taylor of Cygnus
   Support.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "config.h"
#include "subsegs.h"
#include "safe-ctype.h"

#include "opcode/mips.h"
#include "itbl-ops.h"
#include "dwarf2dbg.h"
#include "dw2gencfi.h"

/* Check assumptions made in this file.  */
typedef char static_assert1[sizeof (offsetT) < 8 ? -1 : 1];
typedef char static_assert2[sizeof (valueT) < 8 ? -1 : 1];

#ifdef DEBUG
#define DBG(x) printf x
#else
#define DBG(x)
#endif

#define streq(a, b)           (strcmp (a, b) == 0)

#define SKIP_SPACE_TABS(S) \
  do { while (*(S) == ' ' || *(S) == '\t') ++(S); } while (0)

/* Clean up namespace so we can include obj-elf.h too.  */
static int mips_output_flavor (void);
static int mips_output_flavor (void) { return OUTPUT_FLAVOR; }
#undef OBJ_PROCESS_STAB
#undef OUTPUT_FLAVOR
#undef S_GET_ALIGN
#undef S_GET_SIZE
#undef S_SET_ALIGN
#undef S_SET_SIZE
#undef obj_frob_file
#undef obj_frob_file_after_relocs
#undef obj_frob_symbol
#undef obj_pop_insert
#undef obj_sec_sym_ok_for_reloc
#undef OBJ_COPY_SYMBOL_ATTRIBUTES

#include "obj-elf.h"
/* Fix any of them that we actually care about.  */
#undef OUTPUT_FLAVOR
#define OUTPUT_FLAVOR mips_output_flavor()

#include "elf/mips.h"

#ifndef ECOFF_DEBUGGING
#define NO_ECOFF_DEBUGGING
#define ECOFF_DEBUGGING 0
#endif

int mips_flag_mdebug = -1;

/* Control generation of .pdr sections.  Off by default on IRIX: the native
   linker doesn't know about and discards them, but relocations against them
   remain, leading to rld crashes.  */
#ifdef TE_IRIX
int mips_flag_pdr = false;
#else
int mips_flag_pdr = true;
#endif

#include "ecoff.h"

static char *mips_regmask_frag;
static char *mips_flags_frag;

#define ZERO 0
#define ATREG 1
#define S0  16
#define S7  23
#define TREG 24
#define PIC_CALL_REG 25
#define KT0 26
#define KT1 27
#define GP  28
#define SP  29
#define FP  30
#define RA  31

#define FCSR 31

#define ILLEGAL_REG (32)

#define AT  mips_opts.at

extern int target_big_endian;

/* The name of the readonly data section.  */
#define RDATA_SECTION_NAME ".rodata"

/* Ways in which an instruction can be "appended" to the output.  */
enum append_method {
  /* Just add it normally.  */
  APPEND_ADD,

  /* Add it normally and then add a nop.  */
  APPEND_ADD_WITH_NOP,

  /* Turn an instruction with a delay slot into a "compact" version.  */
  APPEND_ADD_COMPACT,

  /* Insert the instruction before the last one.  */
  APPEND_SWAP
};

/* Information about an instruction, including its format, operands
   and fixups.  */
struct mips_cl_insn
{
  /* The opcode's entry in mips_opcodes or mips16_opcodes.  */
  const struct mips_opcode *insn_mo;

  /* The 16-bit or 32-bit bitstring of the instruction itself.  This is
     a copy of INSN_MO->match with the operands filled in.  If we have
     decided to use an extended MIPS16 instruction, this includes the
     extension.  */
  unsigned long insn_opcode;

  /* The name if this is an label.  */
  char label[16];

  /* The target label name if this is an branch.  */
  char target[16];

  /* The frag that contains the instruction.  */
  struct frag *frag;

  /* The offset into FRAG of the first instruction byte.  */
  long where;

  /* The relocs associated with the instruction, if any.  */
  fixS *fixp[3];

  /* True if this entry cannot be moved from its current position.  */
  unsigned int fixed_p : 1;

  /* True if this instruction occurred in a .set noreorder block.  */
  unsigned int noreorder_p : 1;

  /* True for mips16 instructions that jump to an absolute address.  */
  unsigned int mips16_absolute_jump_p : 1;

  /* True if this instruction is complete.  */
  unsigned int complete_p : 1;

  /* True if this instruction is cleared from history by unconditional
     branch.  */
  unsigned int cleared_p : 1;
};

/* The ABI to use.  */
enum mips_abi_level
{
  NO_ABI = 0,
  O32_ABI,
  O64_ABI,
  N32_ABI,
  N64_ABI,
  EABI_ABI
};

/* MIPS ABI we are using for this output file.  */
static enum mips_abi_level mips_abi = NO_ABI;

/* Whether or not we have code that can call pic code.  */
int mips_abicalls = false;

/* Whether or not we have code which can be put into a shared
   library.  */
static bool mips_in_shared = true;

/* This is the set of options which may be modified by the .set
   pseudo-op.  We use a struct so that .set push and .set pop are more
   reliable.  */

struct mips_set_options
{
  /* MIPS ISA (Instruction Set Architecture) level.  This is set to -1
     if it has not been initialized.  Changed by `.set mipsN', and the
     -mipsN command line option, and the default CPU.  */
  int isa;
  /* Enabled Application Specific Extensions (ASEs).  Changed by `.set
     <asename>', by command line options, and based on the default
     architecture.  */
  int ase;
  /* Whether we are assembling for the mips16 processor.  0 if we are
     not, 1 if we are, and -1 if the value has not been initialized.
     Changed by `.set mips16' and `.set nomips16', and the -mips16 and
     -nomips16 command line options, and the default CPU.  */
  int mips16;
  /* Whether we are assembling for the mipsMIPS ASE.  0 if we are not,
     1 if we are, and -1 if the value has not been initialized.  Changed
     by `.set micromips' and `.set nomicromips', and the -mmicromips
     and -mno-micromips command line options, and the default CPU.  */
  int micromips;
  /* Non-zero if we should not reorder instructions.  Changed by `.set
     reorder' and `.set noreorder'.  */
  int noreorder;
  /* Non-zero if we should not permit the register designated "assembler
     temporary" to be used in instructions.  The value is the register
     number, normally $at ($1).  Changed by `.set at=REG', `.set noat'
     (same as `.set at=$0') and `.set at' (same as `.set at=$1').  */
  unsigned int at;
  /* Non-zero if we should warn when a macro instruction expands into
     more than one machine instruction.  Changed by `.set nomacro' and
     `.set macro'.  */
  int warn_about_macros;
  /* Non-zero if we should not move instructions.  Changed by `.set
     move', `.set volatile', `.set nomove', and `.set novolatile'.  */
  int nomove;
  /* Non-zero if we should not optimize branches by moving the target
     of the branch into the delay slot.  Actually, we don't perform
     this optimization anyhow.  Changed by `.set bopt' and `.set
     nobopt'.  */
  int nobopt;
  /* Non-zero if we should not autoextend mips16 instructions.
     Changed by `.set autoextend' and `.set noautoextend'.  */
  int noautoextend;
  /* True if we should only emit 32-bit microMIPS instructions.
     Changed by `.set insn32' and `.set noinsn32', and the -minsn32
     and -mno-insn32 command line options.  */
  bool insn32;
  /* Restrict general purpose registers and floating point registers
     to 32 bit.  This is initially determined when -mgp32 or -mfp32
     is passed but can changed if the assembler code uses .set mipsN.  */
  int gp;
  int fp;
  /* MIPS architecture (CPU) type.  Changed by .set arch=FOO, the -march
     command line option, and the default CPU.  */
  int arch;
  /* True if ".set sym32" is in effect.  */
  bool sym32;
  /* True if floating-point operations are not allowed.  Changed by .set
     softfloat or .set hardfloat, by command line options -msoft-float or
     -mhard-float.  The default is false.  */
  bool soft_float;

  /* True if only single-precision floating-point operations are allowed.
     Changed by .set singlefloat or .set doublefloat, command-line options
     -msingle-float or -mdouble-float.  The default is false.  */
  bool single_float;

  /* 1 if single-precision operations on odd-numbered registers are
     allowed.  */
  int oddspreg;

  /* The set of ASEs that should be enabled for the user specified
     architecture.  This cannot be inferred from 'arch' for all cores
     as processors only have a unique 'arch' if they add architecture
     specific instructions (UDI).  */
  int init_ase;
};

/* Specifies whether module level options have been checked yet.  */
static bool file_mips_opts_checked = false;

/* Do we support nan2008?  0 if we don't, 1 if we do, and -1 if the
   value has not been initialized.  Changed by `.nan legacy' and
   `.nan 2008', and the -mnan=legacy and -mnan=2008 command line
   options, and the default CPU.  */
static int mips_nan2008 = -1;

/* This is the struct we use to hold the module level set of options.
   Note that we must set the isa field to ISA_UNKNOWN and the ASE, gp and
   fp fields to -1 to indicate that they have not been initialized.  */

static struct mips_set_options file_mips_opts =
{
  /* isa */ ISA_UNKNOWN, /* ase */ 0, /* mips16 */ -1, /* micromips */ -1,
  /* noreorder */ 0,  /* at */ ATREG, /* warn_about_macros */ 0,
  /* nomove */ 0, /* nobopt */ 0, /* noautoextend */ 0, /* insn32 */ false,
  /* gp */ -1, /* fp */ -1, /* arch */ CPU_UNKNOWN, /* sym32 */ false,
  /* soft_float */ false, /* single_float */ false, /* oddspreg */ -1,
  /* init_ase */ 0
};

/* This is similar to file_mips_opts, but for the current set of options.  */

static struct mips_set_options mips_opts =
{
  /* isa */ ISA_UNKNOWN, /* ase */ 0, /* mips16 */ -1, /* micromips */ -1,
  /* noreorder */ 0,  /* at */ ATREG, /* warn_about_macros */ 0,
  /* nomove */ 0, /* nobopt */ 0, /* noautoextend */ 0, /* insn32 */ false,
  /* gp */ -1, /* fp */ -1, /* arch */ CPU_UNKNOWN, /* sym32 */ false,
  /* soft_float */ false, /* single_float */ false, /* oddspreg */ -1,
  /* init_ase */ 0
};

/* Which bits of file_ase were explicitly set or cleared by ASE options.  */
static unsigned int file_ase_explicit;

/* These variables are filled in with the masks of registers used.
   The object format code reads them and puts them in the appropriate
   place.  */
unsigned long mips_gprmask;
unsigned long mips_cprmask[4];

/* True if any MIPS16 code was produced.  */
static int file_ase_mips16;

#define ISA_SUPPORTS_MIPS16E (mips_opts.isa == ISA_MIPS32		\
			      || mips_opts.isa == ISA_MIPS32R2		\
			      || mips_opts.isa == ISA_MIPS32R3		\
			      || mips_opts.isa == ISA_MIPS32R5		\
			      || mips_opts.isa == ISA_MIPS64		\
			      || mips_opts.isa == ISA_MIPS64R2		\
			      || mips_opts.isa == ISA_MIPS64R3		\
			      || mips_opts.isa == ISA_MIPS64R5)

/* True if any microMIPS code was produced.  */
static int file_ase_micromips;

/* True if we want to create R_MIPS_JALR for jalr $25.  */
#ifdef TE_IRIX
#define MIPS_JALR_HINT_P(EXPR) HAVE_NEWABI
#else
/* As a GNU extension, we use R_MIPS_JALR for o32 too.  However,
   because there's no place for any addend, the only acceptable
   expression is a bare symbol.  */
#define MIPS_JALR_HINT_P(EXPR) \
  (!HAVE_IN_PLACE_ADDENDS \
   || ((EXPR)->X_op == O_symbol && (EXPR)->X_add_number == 0))
#endif

/* The argument of the -march= flag.  The architecture we are assembling.  */
static const char *mips_arch_string;

/* The argument of the -mtune= flag.  The architecture for which we
   are optimizing.  */
static int mips_tune = CPU_UNKNOWN;
static const char *mips_tune_string;

/* True when generating 32-bit code for a 64-bit processor.  */
static int mips_32bitmode = 0;

/* True if the given ABI requires 32-bit registers.  */
#define ABI_NEEDS_32BIT_REGS(ABI) ((ABI) == O32_ABI)

/* Likewise 64-bit registers.  */
#define ABI_NEEDS_64BIT_REGS(ABI)	\
  ((ABI) == N32_ABI			\
   || (ABI) == N64_ABI			\
   || (ABI) == O64_ABI)

#define ISA_IS_R6(ISA)			\
  ((ISA) == ISA_MIPS32R6		\
   || (ISA) == ISA_MIPS64R6)

/*  Return true if ISA supports 64 bit wide gp registers.  */
#define ISA_HAS_64BIT_REGS(ISA)		\
  ((ISA) == ISA_MIPS3			\
   || (ISA) == ISA_MIPS4		\
   || (ISA) == ISA_MIPS5		\
   || (ISA) == ISA_MIPS64		\
   || (ISA) == ISA_MIPS64R2		\
   || (ISA) == ISA_MIPS64R3		\
   || (ISA) == ISA_MIPS64R5		\
   || (ISA) == ISA_MIPS64R6)

/*  Return true if ISA supports 64 bit wide float registers.  */
#define ISA_HAS_64BIT_FPRS(ISA)		\
  ((ISA) == ISA_MIPS3			\
   || (ISA) == ISA_MIPS4		\
   || (ISA) == ISA_MIPS5		\
   || (ISA) == ISA_MIPS32R2		\
   || (ISA) == ISA_MIPS32R3		\
   || (ISA) == ISA_MIPS32R5		\
   || (ISA) == ISA_MIPS32R6		\
   || (ISA) == ISA_MIPS64		\
   || (ISA) == ISA_MIPS64R2		\
   || (ISA) == ISA_MIPS64R3		\
   || (ISA) == ISA_MIPS64R5		\
   || (ISA) == ISA_MIPS64R6)

/* Return true if ISA supports 64-bit right rotate (dror et al.)
   instructions.  */
#define ISA_HAS_DROR(ISA)		\
  ((ISA) == ISA_MIPS64R2		\
   || (ISA) == ISA_MIPS64R3		\
   || (ISA) == ISA_MIPS64R5		\
   || (ISA) == ISA_MIPS64R6		\
   || (mips_opts.micromips		\
       && ISA_HAS_64BIT_REGS (ISA))	\
   )

/* Return true if ISA supports 32-bit right rotate (ror et al.)
   instructions.  */
#define ISA_HAS_ROR(ISA)		\
  ((ISA) == ISA_MIPS32R2		\
   || (ISA) == ISA_MIPS32R3		\
   || (ISA) == ISA_MIPS32R5		\
   || (ISA) == ISA_MIPS32R6		\
   || (ISA) == ISA_MIPS64R2		\
   || (ISA) == ISA_MIPS64R3		\
   || (ISA) == ISA_MIPS64R5		\
   || (ISA) == ISA_MIPS64R6		\
   || (mips_opts.ase & ASE_SMARTMIPS)	\
   || mips_opts.micromips		\
   )

/* Return true if ISA supports single-precision floats in odd registers.  */
#define ISA_HAS_ODD_SINGLE_FPR(ISA, CPU)\
  (((ISA) == ISA_MIPS32			\
    || (ISA) == ISA_MIPS32R2		\
    || (ISA) == ISA_MIPS32R3		\
    || (ISA) == ISA_MIPS32R5		\
    || (ISA) == ISA_MIPS32R6		\
    || (ISA) == ISA_MIPS64		\
    || (ISA) == ISA_MIPS64R2		\
    || (ISA) == ISA_MIPS64R3		\
    || (ISA) == ISA_MIPS64R5		\
    || (ISA) == ISA_MIPS64R6		\
    || (CPU) == CPU_ALLEGREX		\
    || (CPU) == CPU_R5900)		\
   && ((CPU) != CPU_GS464		\
    || (CPU) != CPU_GS464E		\
    || (CPU) != CPU_GS264E))

/* Return true if ISA supports move to/from high part of a 64-bit
   floating-point register. */
#define ISA_HAS_MXHC1(ISA)		\
  ((ISA) == ISA_MIPS32R2		\
   || (ISA) == ISA_MIPS32R3		\
   || (ISA) == ISA_MIPS32R5		\
   || (ISA) == ISA_MIPS32R6		\
   || (ISA) == ISA_MIPS64R2		\
   || (ISA) == ISA_MIPS64R3		\
   || (ISA) == ISA_MIPS64R5		\
   || (ISA) == ISA_MIPS64R6)

/*  Return true if ISA supports legacy NAN.  */
#define ISA_HAS_LEGACY_NAN(ISA)		\
  ((ISA) == ISA_MIPS1			\
   || (ISA) == ISA_MIPS2		\
   || (ISA) == ISA_MIPS3		\
   || (ISA) == ISA_MIPS4		\
   || (ISA) == ISA_MIPS5		\
   || (ISA) == ISA_MIPS32		\
   || (ISA) == ISA_MIPS32R2		\
   || (ISA) == ISA_MIPS32R3		\
   || (ISA) == ISA_MIPS32R5		\
   || (ISA) == ISA_MIPS64		\
   || (ISA) == ISA_MIPS64R2		\
   || (ISA) == ISA_MIPS64R3		\
   || (ISA) == ISA_MIPS64R5)

#define GPR_SIZE \
    (mips_opts.gp == 64 && !ISA_HAS_64BIT_REGS (mips_opts.isa) \
     ? 32 \
     : mips_opts.gp)

#define FPR_SIZE \
    (mips_opts.fp == 64 && !ISA_HAS_64BIT_FPRS (mips_opts.isa) \
     ? 32 \
     : mips_opts.fp)

#define HAVE_NEWABI (mips_abi == N32_ABI || mips_abi == N64_ABI)

#define HAVE_64BIT_OBJECTS (mips_abi == N64_ABI)

/* True if relocations are stored in-place.  */
#define HAVE_IN_PLACE_ADDENDS (!HAVE_NEWABI)

/* The ABI-derived address size.  */
#define HAVE_64BIT_ADDRESSES \
  (GPR_SIZE == 64 && (mips_abi == EABI_ABI || mips_abi == N64_ABI))
#define HAVE_32BIT_ADDRESSES (!HAVE_64BIT_ADDRESSES)

/* The size of symbolic constants (i.e., expressions of the form
   "SYMBOL" or "SYMBOL + OFFSET").  */
#define HAVE_32BIT_SYMBOLS \
  (HAVE_32BIT_ADDRESSES || !HAVE_64BIT_OBJECTS || mips_opts.sym32)
#define HAVE_64BIT_SYMBOLS (!HAVE_32BIT_SYMBOLS)

/* Addresses are loaded in different ways, depending on the address size
   in use.  The n32 ABI Documentation also mandates the use of additions
   with overflow checking, but existing implementations don't follow it.  */
#define ADDRESS_ADD_INSN						\
   (HAVE_32BIT_ADDRESSES ? "addu" : "daddu")

#define ADDRESS_ADDI_INSN						\
   (HAVE_32BIT_ADDRESSES ? "addiu" : "daddiu")

#define ADDRESS_LOAD_INSN						\
   (HAVE_32BIT_ADDRESSES ? "lw" : "ld")

#define ADDRESS_STORE_INSN						\
   (HAVE_32BIT_ADDRESSES ? "sw" : "sd")

/* Return true if the given CPU supports the MIPS16 ASE.  */
#define CPU_HAS_MIPS16(cpu)						\
   (startswith (TARGET_CPU, "mips16")					\
    || startswith (TARGET_CANONICAL, "mips-lsi-elf"))

/* Return true if the given CPU supports the microMIPS ASE.  */
#define CPU_HAS_MICROMIPS(cpu)	0

/* True if CPU has a dror instruction.  */
#define CPU_HAS_DROR(CPU)	((CPU) == CPU_VR5400 || (CPU) == CPU_VR5500)

/* True if CPU has a ror instruction.  */
#define CPU_HAS_ROR(CPU)	(CPU_HAS_DROR (CPU) || (CPU) == CPU_ALLEGREX)

/* True if CPU is in the Octeon family.  */
#define CPU_IS_OCTEON(CPU) ((CPU) == CPU_OCTEON || (CPU) == CPU_OCTEONP \
			    || (CPU) == CPU_OCTEON2 || (CPU) == CPU_OCTEON3)

/* True if CPU has seq/sne and seqi/snei instructions.  */
#define CPU_HAS_SEQ(CPU)	(CPU_IS_OCTEON (CPU))

/* True, if CPU has support for ldc1 and sdc1. */
#define CPU_HAS_LDC1_SDC1(CPU)	(mips_opts.isa != ISA_MIPS1		\
				 && (CPU) != CPU_ALLEGREX		\
				 && (CPU) != CPU_R5900)

/* True if mflo and mfhi can be immediately followed by instructions
   which write to the HI and LO registers.

   According to MIPS specifications, MIPS ISAs I, II, and III need
   (at least) two instructions between the reads of HI/LO and
   instructions which write them, and later ISAs do not.  Contradicting
   the MIPS specifications, some MIPS IV processor user manuals (e.g.
   the UM for the NEC Vr5000) document needing the instructions between
   HI/LO reads and writes, as well.  Therefore, we declare only MIPS32,
   MIPS64 and later ISAs to have the interlocks, plus any specific
   earlier-ISA CPUs for which CPU documentation declares that the
   instructions are really interlocked.  */
#define hilo_interlocks \
  (mips_opts.isa == ISA_MIPS32                        \
   || mips_opts.isa == ISA_MIPS32R2                   \
   || mips_opts.isa == ISA_MIPS32R3                   \
   || mips_opts.isa == ISA_MIPS32R5                   \
   || mips_opts.isa == ISA_MIPS32R6                   \
   || mips_opts.isa == ISA_MIPS64                     \
   || mips_opts.isa == ISA_MIPS64R2                   \
   || mips_opts.isa == ISA_MIPS64R3                   \
   || mips_opts.isa == ISA_MIPS64R5                   \
   || mips_opts.isa == ISA_MIPS64R6                   \
   || mips_opts.arch == CPU_ALLEGREX                  \
   || mips_opts.arch == CPU_R4010                     \
   || mips_opts.arch == CPU_R5900                     \
   || mips_opts.arch == CPU_R10000                    \
   || mips_opts.arch == CPU_R12000                    \
   || mips_opts.arch == CPU_R14000                    \
   || mips_opts.arch == CPU_R16000                    \
   || mips_opts.arch == CPU_RM7000                    \
   || mips_opts.arch == CPU_VR5500                    \
   || mips_opts.micromips                             \
   )

/* Whether the processor uses hardware interlocks to protect reads
   from the GPRs after they are loaded from memory, and thus does not
   require nops to be inserted.  This applies to instructions marked
   INSN_LOAD_MEMORY.  These nops are only required at MIPS ISA
   level I and microMIPS mode instructions are always interlocked.  */
#define gpr_interlocks                                \
  (mips_opts.isa != ISA_MIPS1                         \
   || mips_opts.arch == CPU_R3900                     \
   || mips_opts.arch == CPU_R5900                     \
   || mips_opts.micromips                             \
   )

/* Whether the processor uses hardware interlocks to avoid delays
   required by coprocessor instructions, and thus does not require
   nops to be inserted.  This applies to instructions marked
   INSN_LOAD_COPROC, INSN_COPROC_MOVE, and to delays between
   instructions marked INSN_WRITE_COND_CODE and ones marked
   INSN_READ_COND_CODE.  These nops are only required at MIPS ISA
   levels I, II, and III and microMIPS mode instructions are always
   interlocked.  */
/* Itbl support may require additional care here.  */
#define cop_interlocks                                \
  ((mips_opts.isa != ISA_MIPS1                        \
    && mips_opts.isa != ISA_MIPS2                     \
    && mips_opts.isa != ISA_MIPS3)                    \
   || mips_opts.arch == CPU_R4300                     \
   || mips_opts.micromips                             \
   )

/* Whether the processor uses hardware interlocks to protect reads
   from coprocessor registers after they are loaded from memory, and
   thus does not require nops to be inserted.  This applies to
   instructions marked INSN_COPROC_MEMORY_DELAY.  These nops are only
   requires at MIPS ISA level I and microMIPS mode instructions are
   always interlocked.  */
#define cop_mem_interlocks                            \
  (mips_opts.isa != ISA_MIPS1                         \
   || mips_opts.micromips                             \
   )

/* Is this a mfhi or mflo instruction?  */
#define MF_HILO_INSN(PINFO) \
  ((PINFO & INSN_READ_HI) || (PINFO & INSN_READ_LO))

/* Whether code compression (either of the MIPS16 or the microMIPS ASEs)
   has been selected.  This implies, in particular, that addresses of text
   labels have their LSB set.  */
#define HAVE_CODE_COMPRESSION						\
  ((mips_opts.mips16 | mips_opts.micromips) != 0)

/* The minimum and maximum signed values that can be stored in a GPR.  */
#define GPR_SMAX ((offsetT) (((valueT) 1 << (GPR_SIZE - 1)) - 1))
#define GPR_SMIN (-GPR_SMAX - 1)

/* MIPS PIC level.  */

enum mips_pic_level mips_pic;

/* 1 if we should generate 32 bit offsets from the $gp register in
   SVR4_PIC mode.  Currently has no meaning in other modes.  */
static int mips_big_got = 0;

/* 1 if trap instructions should used for overflow rather than break
   instructions.  */
static int mips_trap = 0;

/* 1 if double width floating point constants should not be constructed
   by assembling two single width halves into two single width floating
   point registers which just happen to alias the double width destination
   register.  On some architectures this aliasing can be disabled by a bit
   in the status register, and the setting of this bit cannot be determined
   automatically at assemble time.  */
static int mips_disable_float_construction;

/* Non-zero if any .set noreorder directives were used.  */

static int mips_any_noreorder;

/* Non-zero if nops should be inserted when the register referenced in
   an mfhi/mflo instruction is read in the next two instructions.  */
static int mips_7000_hilo_fix;

/* The size of objects in the small data section.  */
static unsigned int g_switch_value = 8;
/* Whether the -G option was used.  */
static int g_switch_seen = 0;

#define N_RMASK 0xc4
#define N_VFP   0xd4

/* If we can determine in advance that GP optimization won't be
   possible, we can skip the relaxation stuff that tries to produce
   GP-relative references.  This makes delay slot optimization work
   better.

   This function can only provide a guess, but it seems to work for
   gcc output.  It needs to guess right for gcc, otherwise gcc
   will put what it thinks is a GP-relative instruction in a branch
   delay slot.

   I don't know if a fix is needed for the SVR4_PIC mode.  I've only
   fixed it for the non-PIC mode.  KR 95/04/07  */
static int nopic_need_relax (symbolS *, int);

/* Handle of the OPCODE hash table.  */
static htab_t op_hash = NULL;

/* The opcode hash table we use for the mips16.  */
static htab_t mips16_op_hash = NULL;

/* The opcode hash table we use for the microMIPS ASE.  */
static htab_t micromips_op_hash = NULL;

/* This array holds the chars that always start a comment.  If the
    pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = "#";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.  */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
/* Also note that C style comments are always supported.  */
const char line_comment_chars[] = "#";

/* This array holds machine specific line separator characters.  */
const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.
   As in 0f12.456
   or    0d1.2345e12.  */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

/* Also be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
   changed in read.c .  Ideally it shouldn't have to know about it at all,
   but nothing is ideal around here.  */

/* Types of printf format used for instruction-related error messages.
   "I" means int ("%d") and "S" means string ("%s").  */
enum mips_insn_error_format
{
  ERR_FMT_PLAIN,
  ERR_FMT_I,
  ERR_FMT_SS,
};

/* Information about an error that was found while assembling the current
   instruction.  */
struct mips_insn_error
{
  /* We sometimes need to match an instruction against more than one
     opcode table entry.  Errors found during this matching are reported
     against a particular syntactic argument rather than against the
     instruction as a whole.  We grade these messages so that errors
     against argument N have a greater priority than an error against
     any argument < N, since the former implies that arguments up to N
     were acceptable and that the opcode entry was therefore a closer match.
     If several matches report an error against the same argument,
     we only use that error if it is the same in all cases.

     min_argnum is the minimum argument number for which an error message
     should be accepted.  It is 0 if MSG is against the instruction as
     a whole.  */
  int min_argnum;

  /* The printf()-style message, including its format and arguments.  */
  enum mips_insn_error_format format;
  const char *msg;
  union
  {
    int i;
    const char *ss[2];
  } u;
};

/* The error that should be reported for the current instruction.  */
static struct mips_insn_error insn_error;

static int auto_align = 1;

/* When outputting SVR4 PIC code, the assembler needs to know the
   offset in the stack frame from which to restore the $gp register.
   This is set by the .cprestore pseudo-op, and saved in this
   variable.  */
static offsetT mips_cprestore_offset = -1;

/* Similar for NewABI PIC code, where $gp is callee-saved.  NewABI has some
   more optimizations, it can use a register value instead of a memory-saved
   offset and even an other register than $gp as global pointer.  */
static offsetT mips_cpreturn_offset = -1;
static int mips_cpreturn_register = -1;
static int mips_gp_register = GP;
static int mips_gprel_offset = 0;

/* Whether mips_cprestore_offset has been set in the current function
   (or whether it has already been warned about, if not).  */
static int mips_cprestore_valid = 0;

/* This is the register which holds the stack frame, as set by the
   .frame pseudo-op.  This is needed to implement .cprestore.  */
static int mips_frame_reg = SP;

/* Whether mips_frame_reg has been set in the current function
   (or whether it has already been warned about, if not).  */
static int mips_frame_reg_valid = 0;

/* To output NOP instructions correctly, we need to keep information
   about the previous two instructions.  */

/* Whether we are optimizing.  The default value of 2 means to remove
   unneeded NOPs and swap branch instructions when possible.  A value
   of 1 means to not swap branches.  A value of 0 means to always
   insert NOPs.  */
static int mips_optimize = 2;

/* Debugging level.  -g sets this to 2.  -gN sets this to N.  -g0 is
   equivalent to seeing no -g option at all.  */
static int mips_debug = 0;

/* The maximum number of NOPs needed to avoid the VR4130 mflo/mfhi errata.  */
#define MAX_VR4130_NOPS 4

/* The maximum number of NOPs needed to fill delay slots.  */
#define MAX_DELAY_NOPS 2

/* The maximum number of NOPs needed for any purpose.  */
#define MAX_NOPS 4

/* The maximum range of context length of ll/sc.  */
#define MAX_LLSC_RANGE 20

/* A list of previous instructions, with index 0 being the most recent.
   We need to look back MAX_NOPS instructions when filling delay slots
   or working around processor errata.  We need to look back one
   instruction further if we're thinking about using history[0] to
   fill a branch delay slot.  */
static struct mips_cl_insn history[1 + MAX_NOPS + MAX_LLSC_RANGE];

/* The maximum number of LABELS detect for the same address.  */
#define MAX_LABELS_SAME 10

/* Arrays of operands for each instruction.  */
#define MAX_OPERANDS 6
struct mips_operand_array
{
  const struct mips_operand *operand[MAX_OPERANDS];
};
static struct mips_operand_array *mips_operands;
static struct mips_operand_array *mips16_operands;
static struct mips_operand_array *micromips_operands;

/* Nop instructions used by emit_nop.  */
static struct mips_cl_insn nop_insn;
static struct mips_cl_insn mips16_nop_insn;
static struct mips_cl_insn micromips_nop16_insn;
static struct mips_cl_insn micromips_nop32_insn;

/* Sync instructions used by insert sync.  */
static struct mips_cl_insn sync_insn;

/* The appropriate nop for the current mode.  */
#define NOP_INSN (mips_opts.mips16					\
		  ? &mips16_nop_insn					\
		  : (mips_opts.micromips				\
		     ? (mips_opts.insn32				\
			? &micromips_nop32_insn				\
			: &micromips_nop16_insn)			\
		     : &nop_insn))

/* The size of NOP_INSN in bytes.  */
#define NOP_INSN_SIZE ((mips_opts.mips16				\
			|| (mips_opts.micromips && !mips_opts.insn32))	\
		       ? 2 : 4)

/* If this is set, it points to a frag holding nop instructions which
   were inserted before the start of a noreorder section.  If those
   nops turn out to be unnecessary, the size of the frag can be
   decreased.  */
static fragS *prev_nop_frag;

/* The number of nop instructions we created in prev_nop_frag.  */
static int prev_nop_frag_holds;

/* The number of nop instructions that we know we need in
   prev_nop_frag.  */
static int prev_nop_frag_required;

/* The number of instructions we've seen since prev_nop_frag.  */
static int prev_nop_frag_since;

/* Relocations against symbols are sometimes done in two parts, with a HI
   relocation and a LO relocation.  Each relocation has only 16 bits of
   space to store an addend.  This means that in order for the linker to
   handle carries correctly, it must be able to locate both the HI and
   the LO relocation.  This means that the relocations must appear in
   order in the relocation table.

   In order to implement this, we keep track of each unmatched HI
   relocation.  We then sort them so that they immediately precede the
   corresponding LO relocation.  */

struct mips_hi_fixup
{
  /* Next HI fixup.  */
  struct mips_hi_fixup *next;
  /* This fixup.  */
  fixS *fixp;
  /* The section this fixup is in.  */
  segT seg;
};

/* The list of unmatched HI relocs.  */

static struct mips_hi_fixup *mips_hi_fixup_list;

/* Map mips16 register numbers to normal MIPS register numbers.  */

static const unsigned int mips16_to_32_reg_map[] =
{
  16, 17, 2, 3, 4, 5, 6, 7
};

/* Map microMIPS register numbers to normal MIPS register numbers.  */

#define micromips_to_32_reg_d_map	mips16_to_32_reg_map

/* The microMIPS registers with type h.  */
static const unsigned int micromips_to_32_reg_h_map1[] =
{
  5, 5, 6, 4, 4, 4, 4, 4
};
static const unsigned int micromips_to_32_reg_h_map2[] =
{
  6, 7, 7, 21, 22, 5, 6, 7
};

/* The microMIPS registers with type m.  */
static const unsigned int micromips_to_32_reg_m_map[] =
{
  0, 17, 2, 3, 16, 18, 19, 20
};

#define micromips_to_32_reg_n_map      micromips_to_32_reg_m_map

/* Classifies the kind of instructions we're interested in when
   implementing -mfix-vr4120.  */
enum fix_vr4120_class
{
  FIX_VR4120_MACC,
  FIX_VR4120_DMACC,
  FIX_VR4120_MULT,
  FIX_VR4120_DMULT,
  FIX_VR4120_DIV,
  FIX_VR4120_MTHILO,
  NUM_FIX_VR4120_CLASSES
};

/* ...likewise -mfix-loongson2f-jump.  */
static bool mips_fix_loongson2f_jump;

/* ...likewise -mfix-loongson2f-nop.  */
static bool mips_fix_loongson2f_nop;

/* True if -mfix-loongson2f-nop or -mfix-loongson2f-jump passed.  */
static bool mips_fix_loongson2f;

/* Given two FIX_VR4120_* values X and Y, bit Y of element X is set if
   there must be at least one other instruction between an instruction
   of type X and an instruction of type Y.  */
static unsigned int vr4120_conflicts[NUM_FIX_VR4120_CLASSES];

/* True if -mfix-vr4120 is in force.  */
static int mips_fix_vr4120;

/* ...likewise -mfix-vr4130.  */
static int mips_fix_vr4130;

/* ...likewise -mfix-24k.  */
static int mips_fix_24k;

/* ...likewise -mfix-rm7000  */
static int mips_fix_rm7000;

/* ...likewise -mfix-cn63xxp1 */
static bool mips_fix_cn63xxp1;

/* ...likewise -mfix-r5900 */
static bool mips_fix_r5900;
static bool mips_fix_r5900_explicit;

/* ...likewise -mfix-loongson3-llsc.  */
static bool mips_fix_loongson3_llsc = DEFAULT_MIPS_FIX_LOONGSON3_LLSC;

/* We don't relax branches by default, since this causes us to expand
   `la .l2 - .l1' if there's a branch between .l1 and .l2, because we
   fail to compute the offset before expanding the macro to the most
   efficient expansion.  */

static int mips_relax_branch;

/* TRUE if checks are suppressed for invalid branches between ISA modes.
   Needed for broken assembly produced by some GCC versions and some
   sloppy code out there, where branches to data labels are present.  */
static bool mips_ignore_branch_isa;

/* The expansion of many macros depends on the type of symbol that
   they refer to.  For example, when generating position-dependent code,
   a macro that refers to a symbol may have two different expansions,
   one which uses GP-relative addresses and one which uses absolute
   addresses.  When generating SVR4-style PIC, a macro may have
   different expansions for local and global symbols.

   We handle these situations by generating both sequences and putting
   them in variant frags.  In position-dependent code, the first sequence
   will be the GP-relative one and the second sequence will be the
   absolute one.  In SVR4 PIC, the first sequence will be for global
   symbols and the second will be for local symbols.

   The frag's "subtype" is RELAX_ENCODE (FIRST, SECOND), where FIRST and
   SECOND are the lengths of the two sequences in bytes.  These fields
   can be extracted using RELAX_FIRST() and RELAX_SECOND().  In addition,
   the subtype has the following flags:

   RELAX_PIC
	Set if generating PIC code.

   RELAX_USE_SECOND
	Set if it has been decided that we should use the second
	sequence instead of the first.

   RELAX_SECOND_LONGER
	Set in the first variant frag if the macro's second implementation
	is longer than its first.  This refers to the macro as a whole,
	not an individual relaxation.

   RELAX_NOMACRO
	Set in the first variant frag if the macro appeared in a .set nomacro
	block and if one alternative requires a warning but the other does not.

   RELAX_DELAY_SLOT
	Like RELAX_NOMACRO, but indicates that the macro appears in a branch
	delay slot.

   RELAX_DELAY_SLOT_16BIT
	Like RELAX_DELAY_SLOT, but indicates that the delay slot requires a
	16-bit instruction.

   RELAX_DELAY_SLOT_SIZE_FIRST
	Like RELAX_DELAY_SLOT, but indicates that the first implementation of
	the macro is of the wrong size for the branch delay slot.

   RELAX_DELAY_SLOT_SIZE_SECOND
	Like RELAX_DELAY_SLOT, but indicates that the second implementation of
	the macro is of the wrong size for the branch delay slot.

   The frag's "opcode" points to the first fixup for relaxable code.

   Relaxable macros are generated using a sequence such as:

      relax_start (SYMBOL);
      ... generate first expansion ...
      relax_switch ();
      ... generate second expansion ...
      relax_end ();

   The code and fixups for the unwanted alternative are discarded
   by md_convert_frag.  */
#define RELAX_ENCODE(FIRST, SECOND, PIC)			\
  (((FIRST) << 8) | (SECOND) | ((PIC) ? 0x10000 : 0))

#define RELAX_FIRST(X) (((X) >> 8) & 0xff)
#define RELAX_SECOND(X) ((X) & 0xff)
#define RELAX_PIC(X) (((X) & 0x10000) != 0)
#define RELAX_USE_SECOND 0x20000
#define RELAX_SECOND_LONGER 0x40000
#define RELAX_NOMACRO 0x80000
#define RELAX_DELAY_SLOT 0x100000
#define RELAX_DELAY_SLOT_16BIT 0x200000
#define RELAX_DELAY_SLOT_SIZE_FIRST 0x400000
#define RELAX_DELAY_SLOT_SIZE_SECOND 0x800000

/* Branch without likely bit.  If label is out of range, we turn:

	beq reg1, reg2, label
	delay slot

   into

        bne reg1, reg2, 0f
        nop
        j label
     0: delay slot

   with the following opcode replacements:

	beq <-> bne
	blez <-> bgtz
	bltz <-> bgez
	bc1f <-> bc1t

	bltzal <-> bgezal  (with jal label instead of j label)

   Even though keeping the delay slot instruction in the delay slot of
   the branch would be more efficient, it would be very tricky to do
   correctly, because we'd have to introduce a variable frag *after*
   the delay slot instruction, and expand that instead.  Let's do it
   the easy way for now, even if the branch-not-taken case now costs
   one additional instruction.  Out-of-range branches are not supposed
   to be common, anyway.

   Branch likely.  If label is out of range, we turn:

	beql reg1, reg2, label
	delay slot (annulled if branch not taken)

   into

        beql reg1, reg2, 1f
        nop
        beql $0, $0, 2f
        nop
     1: j[al] label
        delay slot (executed only if branch taken)
     2:

   It would be possible to generate a shorter sequence by losing the
   likely bit, generating something like:

	bne reg1, reg2, 0f
	nop
	j[al] label
	delay slot (executed only if branch taken)
     0:

	beql -> bne
	bnel -> beq
	blezl -> bgtz
	bgtzl -> blez
	bltzl -> bgez
	bgezl -> bltz
	bc1fl -> bc1t
	bc1tl -> bc1f

	bltzall -> bgezal  (with jal label instead of j label)
	bgezall -> bltzal  (ditto)


   but it's not clear that it would actually improve performance.  */
#define RELAX_BRANCH_ENCODE(at, pic,				\
			    uncond, likely, link, toofar)	\
  ((relax_substateT)						\
   (0xc0000000							\
    | ((at) & 0x1f)						\
    | ((pic) ? 0x20 : 0)					\
    | ((toofar) ? 0x40 : 0)					\
    | ((link) ? 0x80 : 0)					\
    | ((likely) ? 0x100 : 0)					\
    | ((uncond) ? 0x200 : 0)))
#define RELAX_BRANCH_P(i) (((i) & 0xf0000000) == 0xc0000000)
#define RELAX_BRANCH_UNCOND(i) (((i) & 0x200) != 0)
#define RELAX_BRANCH_LIKELY(i) (((i) & 0x100) != 0)
#define RELAX_BRANCH_LINK(i) (((i) & 0x80) != 0)
#define RELAX_BRANCH_TOOFAR(i) (((i) & 0x40) != 0)
#define RELAX_BRANCH_PIC(i) (((i) & 0x20) != 0)
#define RELAX_BRANCH_AT(i) ((i) & 0x1f)

/* For mips16 code, we use an entirely different form of relaxation.
   mips16 supports two versions of most instructions which take
   immediate values: a small one which takes some small value, and a
   larger one which takes a 16 bit value.  Since branches also follow
   this pattern, relaxing these values is required.

   We can assemble both mips16 and normal MIPS code in a single
   object.  Therefore, we need to support this type of relaxation at
   the same time that we support the relaxation described above.  We
   use the high bit of the subtype field to distinguish these cases.

   The information we store for this type of relaxation is the
   argument code found in the opcode file for this relocation, whether
   the user explicitly requested a small or extended form, and whether
   the relocation is in a jump or jal delay slot.  That tells us the
   size of the value, and how it should be stored.  We also store
   whether the fragment is considered to be extended or not.  We also
   store whether this is known to be a branch to a different section,
   whether we have tried to relax this frag yet, and whether we have
   ever extended a PC relative fragment because of a shift count.  */
#define RELAX_MIPS16_ENCODE(type, e2, pic, sym32, nomacro,	\
			    small, ext,				\
			    dslot, jal_dslot)			\
  (0x80000000							\
   | ((type) & 0xff)						\
   | ((e2) ? 0x100 : 0)						\
   | ((pic) ? 0x200 : 0)					\
   | ((sym32) ? 0x400 : 0)					\
   | ((nomacro) ? 0x800 : 0)					\
   | ((small) ? 0x1000 : 0)					\
   | ((ext) ? 0x2000 : 0)					\
   | ((dslot) ? 0x4000 : 0)					\
   | ((jal_dslot) ? 0x8000 : 0))

#define RELAX_MIPS16_P(i) (((i) & 0xc0000000) == 0x80000000)
#define RELAX_MIPS16_TYPE(i) ((i) & 0xff)
#define RELAX_MIPS16_E2(i) (((i) & 0x100) != 0)
#define RELAX_MIPS16_PIC(i) (((i) & 0x200) != 0)
#define RELAX_MIPS16_SYM32(i) (((i) & 0x400) != 0)
#define RELAX_MIPS16_NOMACRO(i) (((i) & 0x800) != 0)
#define RELAX_MIPS16_USER_SMALL(i) (((i) & 0x1000) != 0)
#define RELAX_MIPS16_USER_EXT(i) (((i) & 0x2000) != 0)
#define RELAX_MIPS16_DSLOT(i) (((i) & 0x4000) != 0)
#define RELAX_MIPS16_JAL_DSLOT(i) (((i) & 0x8000) != 0)

#define RELAX_MIPS16_EXTENDED(i) (((i) & 0x10000) != 0)
#define RELAX_MIPS16_MARK_EXTENDED(i) ((i) | 0x10000)
#define RELAX_MIPS16_CLEAR_EXTENDED(i) ((i) & ~0x10000)
#define RELAX_MIPS16_ALWAYS_EXTENDED(i) (((i) & 0x20000) != 0)
#define RELAX_MIPS16_MARK_ALWAYS_EXTENDED(i) ((i) | 0x20000)
#define RELAX_MIPS16_CLEAR_ALWAYS_EXTENDED(i) ((i) & ~0x20000)
#define RELAX_MIPS16_MACRO(i) (((i) & 0x40000) != 0)
#define RELAX_MIPS16_MARK_MACRO(i) ((i) | 0x40000)
#define RELAX_MIPS16_CLEAR_MACRO(i) ((i) & ~0x40000)

/* For microMIPS code, we use relaxation similar to one we use for
   MIPS16 code.  Some instructions that take immediate values support
   two encodings: a small one which takes some small value, and a
   larger one which takes a 16 bit value.  As some branches also follow
   this pattern, relaxing these values is required.

   We can assemble both microMIPS and normal MIPS code in a single
   object.  Therefore, we need to support this type of relaxation at
   the same time that we support the relaxation described above.  We
   use one of the high bits of the subtype field to distinguish these
   cases.

   The information we store for this type of relaxation is the argument
   code found in the opcode file for this relocation, the register
   selected as the assembler temporary, whether in the 32-bit
   instruction mode, whether the branch is unconditional, whether it is
   compact, whether there is no delay-slot instruction available to fill
   in, whether it stores the link address implicitly in $ra, whether
   relaxation of out-of-range 32-bit branches to a sequence of
   instructions is enabled, and whether the displacement of a branch is
   too large to fit as an immediate argument of a 16-bit and a 32-bit
   branch, respectively.  */
#define RELAX_MICROMIPS_ENCODE(type, at, insn32, pic,		\
			       uncond, compact, link, nods,	\
			       relax32, toofar16, toofar32)	\
  (0x40000000							\
   | ((type) & 0xff)						\
   | (((at) & 0x1f) << 8)					\
   | ((insn32) ? 0x2000 : 0)					\
   | ((pic) ? 0x4000 : 0)					\
   | ((uncond) ? 0x8000 : 0)					\
   | ((compact) ? 0x10000 : 0)					\
   | ((link) ? 0x20000 : 0)					\
   | ((nods) ? 0x40000 : 0)					\
   | ((relax32) ? 0x80000 : 0)					\
   | ((toofar16) ? 0x100000 : 0)				\
   | ((toofar32) ? 0x200000 : 0))
#define RELAX_MICROMIPS_P(i) (((i) & 0xc0000000) == 0x40000000)
#define RELAX_MICROMIPS_TYPE(i) ((i) & 0xff)
#define RELAX_MICROMIPS_AT(i) (((i) >> 8) & 0x1f)
#define RELAX_MICROMIPS_INSN32(i) (((i) & 0x2000) != 0)
#define RELAX_MICROMIPS_PIC(i) (((i) & 0x4000) != 0)
#define RELAX_MICROMIPS_UNCOND(i) (((i) & 0x8000) != 0)
#define RELAX_MICROMIPS_COMPACT(i) (((i) & 0x10000) != 0)
#define RELAX_MICROMIPS_LINK(i) (((i) & 0x20000) != 0)
#define RELAX_MICROMIPS_NODS(i) (((i) & 0x40000) != 0)
#define RELAX_MICROMIPS_RELAX32(i) (((i) & 0x80000) != 0)

#define RELAX_MICROMIPS_TOOFAR16(i) (((i) & 0x100000) != 0)
#define RELAX_MICROMIPS_MARK_TOOFAR16(i) ((i) | 0x100000)
#define RELAX_MICROMIPS_CLEAR_TOOFAR16(i) ((i) & ~0x100000)
#define RELAX_MICROMIPS_TOOFAR32(i) (((i) & 0x200000) != 0)
#define RELAX_MICROMIPS_MARK_TOOFAR32(i) ((i) | 0x200000)
#define RELAX_MICROMIPS_CLEAR_TOOFAR32(i) ((i) & ~0x200000)

/* Sign-extend 16-bit value X.  */
#define SEXT_16BIT(X) ((((X) + 0x8000) & 0xffff) - 0x8000)

/* Is the given value a sign-extended 32-bit value?  */
#define IS_SEXT_32BIT_NUM(x)						\
  (((x) &~ (offsetT) 0x7fffffff) == 0					\
   || (((x) &~ (offsetT) 0x7fffffff) == ~ (offsetT) 0x7fffffff))

/* Is the given value a sign-extended 16-bit value?  */
#define IS_SEXT_16BIT_NUM(x)						\
  (((x) &~ (offsetT) 0x7fff) == 0					\
   || (((x) &~ (offsetT) 0x7fff) == ~ (offsetT) 0x7fff))

/* Is the given value a sign-extended 12-bit value?  */
#define IS_SEXT_12BIT_NUM(x)						\
  (((((x) & 0xfff) ^ 0x800LL) - 0x800LL) == (x))

/* Is the given value a sign-extended 9-bit value?  */
#define IS_SEXT_9BIT_NUM(x)						\
  (((((x) & 0x1ff) ^ 0x100LL) - 0x100LL) == (x))

/* Is the given value a zero-extended 32-bit value?  Or a negated one?  */
#define IS_ZEXT_32BIT_NUM(x)						\
  (((x) &~ (offsetT) 0xffffffff) == 0					\
   || (((x) &~ (offsetT) 0xffffffff) == ~ (offsetT) 0xffffffff))

/* Extract bits MASK << SHIFT from STRUCT and shift them right
   SHIFT places.  */
#define EXTRACT_BITS(STRUCT, MASK, SHIFT) \
  (((STRUCT) >> (SHIFT)) & (MASK))

/* Extract the operand given by FIELD from mips_cl_insn INSN.  */
#define EXTRACT_OPERAND(MICROMIPS, FIELD, INSN) \
  (!(MICROMIPS) \
   ? EXTRACT_BITS ((INSN).insn_opcode, OP_MASK_##FIELD, OP_SH_##FIELD) \
   : EXTRACT_BITS ((INSN).insn_opcode, \
		   MICROMIPSOP_MASK_##FIELD, MICROMIPSOP_SH_##FIELD))
#define MIPS16_EXTRACT_OPERAND(FIELD, INSN) \
  EXTRACT_BITS ((INSN).insn_opcode, \
		MIPS16OP_MASK_##FIELD, \
		MIPS16OP_SH_##FIELD)

/* The MIPS16 EXTEND opcode, shifted left 16 places.  */
#define MIPS16_EXTEND (0xf000U << 16)

/* Whether or not we are emitting a branch-likely macro.  */
static bool emit_branch_likely_macro = false;

/* Global variables used when generating relaxable macros.  See the
   comment above RELAX_ENCODE for more details about how relaxation
   is used.  */
static struct {
  /* 0 if we're not emitting a relaxable macro.
     1 if we're emitting the first of the two relaxation alternatives.
     2 if we're emitting the second alternative.  */
  int sequence;

  /* The first relaxable fixup in the current frag.  (In other words,
     the first fixup that refers to relaxable code.)  */
  fixS *first_fixup;

  /* sizes[0] says how many bytes of the first alternative are stored in
     the current frag.  Likewise sizes[1] for the second alternative.  */
  unsigned int sizes[2];

  /* The symbol on which the choice of sequence depends.  */
  symbolS *symbol;
} mips_relax;

/* Global variables used to decide whether a macro needs a warning.  */
static struct {
  /* True if the macro is in a branch delay slot.  */
  bool delay_slot_p;

  /* Set to the length in bytes required if the macro is in a delay slot
     that requires a specific length of instruction, otherwise zero.  */
  unsigned int delay_slot_length;

  /* For relaxable macros, sizes[0] is the length of the first alternative
     in bytes and sizes[1] is the length of the second alternative.
     For non-relaxable macros, both elements give the length of the
     macro in bytes.  */
  unsigned int sizes[2];

  /* For relaxable macros, first_insn_sizes[0] is the length of the first
     instruction of the first alternative in bytes and first_insn_sizes[1]
     is the length of the first instruction of the second alternative.
     For non-relaxable macros, both elements give the length of the first
     instruction in bytes.

     Set to zero if we haven't yet seen the first instruction.  */
  unsigned int first_insn_sizes[2];

  /* For relaxable macros, insns[0] is the number of instructions for the
     first alternative and insns[1] is the number of instructions for the
     second alternative.

     For non-relaxable macros, both elements give the number of
     instructions for the macro.  */
  unsigned int insns[2];

  /* The first variant frag for this macro.  */
  fragS *first_frag;
} mips_macro_warning;

/* Prototypes for static functions.  */

enum mips_regclass { MIPS_GR_REG, MIPS_FP_REG, MIPS16_REG };

static void append_insn
  (struct mips_cl_insn *, expressionS *, bfd_reloc_code_real_type *,
   bool expansionp);
static void mips_no_prev_insn (void);
static void macro_build (expressionS *, const char *, const char *, ...);
static void mips16_macro_build
  (expressionS *, const char *, const char *, va_list *);
static void load_register (int, expressionS *, int);
static void macro_start (void);
static void macro_end (void);
static void macro (struct mips_cl_insn *ip, char *str);
static void mips16_macro (struct mips_cl_insn * ip);
static void mips_ip (char *str, struct mips_cl_insn * ip);
static void mips16_ip (char *str, struct mips_cl_insn * ip);
static unsigned long mips16_immed_extend (offsetT, unsigned int);
static void mips16_immed
  (const char *, unsigned int, int, bfd_reloc_code_real_type, offsetT,
   unsigned int, unsigned long *);
static size_t my_getSmallExpression
  (expressionS *, bfd_reloc_code_real_type *, char *);
static void my_getExpression (expressionS *, char *);
static void s_align (int);
static void s_change_sec (int);
static void s_change_section (int);
static void s_cons (int);
static void s_float_cons (int);
static void s_mips_globl (int);
static void s_option (int);
static void s_mipsset (int);
static void s_abicalls (int);
static void s_cpload (int);
static void s_cpsetup (int);
static void s_cplocal (int);
static void s_cprestore (int);
static void s_cpreturn (int);
static void s_dtprelword (int);
static void s_dtpreldword (int);
static void s_tprelword (int);
static void s_tpreldword (int);
static void s_gpvalue (int);
static void s_gpword (int);
static void s_gpdword (int);
static void s_ehword (int);
static void s_cpadd (int);
static void s_insn (int);
static void s_nan (int);
static void s_module (int);
static void s_mips_ent (int);
static void s_mips_end (int);
static void s_mips_frame (int);
static void s_mips_mask (int reg_type);
static void s_mips_stab (int);
static void s_mips_weakext (int);
static void s_mips_file (int);
static void s_mips_loc (int);
static bool pic_need_relax (symbolS *);
static int relaxed_branch_length (fragS *, asection *, int);
static int relaxed_micromips_16bit_branch_length (fragS *, asection *, int);
static int relaxed_micromips_32bit_branch_length (fragS *, asection *, int);
static void file_mips_check_options (void);

/* Table and functions used to map between CPU/ISA names, and
   ISA levels, and CPU numbers.  */

struct mips_cpu_info
{
  const char *name;           /* CPU or ISA name.  */
  int flags;                  /* MIPS_CPU_* flags.  */
  int ase;                    /* Set of ASEs implemented by the CPU.  */
  int isa;                    /* ISA level.  */
  int cpu;                    /* CPU number (default CPU if ISA).  */
};

#define MIPS_CPU_IS_ISA		0x0001	/* Is this an ISA?  (If 0, a CPU.) */

static const struct mips_cpu_info *mips_parse_cpu (const char *, const char *);
static const struct mips_cpu_info *mips_cpu_info_from_isa (int);
static const struct mips_cpu_info *mips_cpu_info_from_arch (int);

/* Command-line options.  */
const char *md_shortopts = "O::g::G:";

enum options
  {
    OPTION_MARCH = OPTION_MD_BASE,
    OPTION_MTUNE,
    OPTION_MIPS1,
    OPTION_MIPS2,
    OPTION_MIPS3,
    OPTION_MIPS4,
    OPTION_MIPS5,
    OPTION_MIPS32,
    OPTION_MIPS64,
    OPTION_MIPS32R2,
    OPTION_MIPS32R3,
    OPTION_MIPS32R5,
    OPTION_MIPS32R6,
    OPTION_MIPS64R2,
    OPTION_MIPS64R3,
    OPTION_MIPS64R5,
    OPTION_MIPS64R6,
    OPTION_MIPS16,
    OPTION_NO_MIPS16,
    OPTION_MIPS3D,
    OPTION_NO_MIPS3D,
    OPTION_MDMX,
    OPTION_NO_MDMX,
    OPTION_DSP,
    OPTION_NO_DSP,
    OPTION_MT,
    OPTION_NO_MT,
    OPTION_VIRT,
    OPTION_NO_VIRT,
    OPTION_MSA,
    OPTION_NO_MSA,
    OPTION_SMARTMIPS,
    OPTION_NO_SMARTMIPS,
    OPTION_DSPR2,
    OPTION_NO_DSPR2,
    OPTION_DSPR3,
    OPTION_NO_DSPR3,
    OPTION_EVA,
    OPTION_NO_EVA,
    OPTION_XPA,
    OPTION_NO_XPA,
    OPTION_MICROMIPS,
    OPTION_NO_MICROMIPS,
    OPTION_MCU,
    OPTION_NO_MCU,
    OPTION_MIPS16E2,
    OPTION_NO_MIPS16E2,
    OPTION_CRC,
    OPTION_NO_CRC,
    OPTION_M4650,
    OPTION_NO_M4650,
    OPTION_M4010,
    OPTION_NO_M4010,
    OPTION_M4100,
    OPTION_NO_M4100,
    OPTION_M3900,
    OPTION_NO_M3900,
    OPTION_M7000_HILO_FIX,
    OPTION_MNO_7000_HILO_FIX,
    OPTION_FIX_24K,
    OPTION_NO_FIX_24K,
    OPTION_FIX_RM7000,
    OPTION_NO_FIX_RM7000,
    OPTION_FIX_LOONGSON3_LLSC,
    OPTION_NO_FIX_LOONGSON3_LLSC,
    OPTION_FIX_LOONGSON2F_JUMP,
    OPTION_NO_FIX_LOONGSON2F_JUMP,
    OPTION_FIX_LOONGSON2F_NOP,
    OPTION_NO_FIX_LOONGSON2F_NOP,
    OPTION_FIX_VR4120,
    OPTION_NO_FIX_VR4120,
    OPTION_FIX_VR4130,
    OPTION_NO_FIX_VR4130,
    OPTION_FIX_CN63XXP1,
    OPTION_NO_FIX_CN63XXP1,
    OPTION_FIX_R5900,
    OPTION_NO_FIX_R5900,
    OPTION_TRAP,
    OPTION_BREAK,
    OPTION_EB,
    OPTION_EL,
    OPTION_FP32,
    OPTION_GP32,
    OPTION_CONSTRUCT_FLOATS,
    OPTION_NO_CONSTRUCT_FLOATS,
    OPTION_FP64,
    OPTION_FPXX,
    OPTION_GP64,
    OPTION_RELAX_BRANCH,
    OPTION_NO_RELAX_BRANCH,
    OPTION_IGNORE_BRANCH_ISA,
    OPTION_NO_IGNORE_BRANCH_ISA,
    OPTION_INSN32,
    OPTION_NO_INSN32,
    OPTION_MSHARED,
    OPTION_MNO_SHARED,
    OPTION_MSYM32,
    OPTION_MNO_SYM32,
    OPTION_SOFT_FLOAT,
    OPTION_HARD_FLOAT,
    OPTION_SINGLE_FLOAT,
    OPTION_DOUBLE_FLOAT,
    OPTION_32,
    OPTION_CALL_SHARED,
    OPTION_CALL_NONPIC,
    OPTION_NON_SHARED,
    OPTION_XGOT,
    OPTION_MABI,
    OPTION_N32,
    OPTION_64,
    OPTION_MDEBUG,
    OPTION_NO_MDEBUG,
    OPTION_PDR,
    OPTION_NO_PDR,
    OPTION_MVXWORKS_PIC,
    OPTION_NAN,
    OPTION_ODD_SPREG,
    OPTION_NO_ODD_SPREG,
    OPTION_GINV,
    OPTION_NO_GINV,
    OPTION_LOONGSON_MMI,
    OPTION_NO_LOONGSON_MMI,
    OPTION_LOONGSON_CAM,
    OPTION_NO_LOONGSON_CAM,
    OPTION_LOONGSON_EXT,
    OPTION_NO_LOONGSON_EXT,
    OPTION_LOONGSON_EXT2,
    OPTION_NO_LOONGSON_EXT2,
    OPTION_END_OF_ENUM
  };

struct option md_longopts[] =
{
  /* Options which specify architecture.  */
  {"march", required_argument, NULL, OPTION_MARCH},
  {"mtune", required_argument, NULL, OPTION_MTUNE},
  {"mips0", no_argument, NULL, OPTION_MIPS1},
  {"mips1", no_argument, NULL, OPTION_MIPS1},
  {"mips2", no_argument, NULL, OPTION_MIPS2},
  {"mips3", no_argument, NULL, OPTION_MIPS3},
  {"mips4", no_argument, NULL, OPTION_MIPS4},
  {"mips5", no_argument, NULL, OPTION_MIPS5},
  {"mips32", no_argument, NULL, OPTION_MIPS32},
  {"mips64", no_argument, NULL, OPTION_MIPS64},
  {"mips32r2", no_argument, NULL, OPTION_MIPS32R2},
  {"mips32r3", no_argument, NULL, OPTION_MIPS32R3},
  {"mips32r5", no_argument, NULL, OPTION_MIPS32R5},
  {"mips32r6", no_argument, NULL, OPTION_MIPS32R6},
  {"mips64r2", no_argument, NULL, OPTION_MIPS64R2},
  {"mips64r3", no_argument, NULL, OPTION_MIPS64R3},
  {"mips64r5", no_argument, NULL, OPTION_MIPS64R5},
  {"mips64r6", no_argument, NULL, OPTION_MIPS64R6},

  /* Options which specify Application Specific Extensions (ASEs).  */
  {"mips16", no_argument, NULL, OPTION_MIPS16},
  {"no-mips16", no_argument, NULL, OPTION_NO_MIPS16},
  {"mips3d", no_argument, NULL, OPTION_MIPS3D},
  {"no-mips3d", no_argument, NULL, OPTION_NO_MIPS3D},
  {"mdmx", no_argument, NULL, OPTION_MDMX},
  {"no-mdmx", no_argument, NULL, OPTION_NO_MDMX},
  {"mdsp", no_argument, NULL, OPTION_DSP},
  {"mno-dsp", no_argument, NULL, OPTION_NO_DSP},
  {"mmt", no_argument, NULL, OPTION_MT},
  {"mno-mt", no_argument, NULL, OPTION_NO_MT},
  {"msmartmips", no_argument, NULL, OPTION_SMARTMIPS},
  {"mno-smartmips", no_argument, NULL, OPTION_NO_SMARTMIPS},
  {"mdspr2", no_argument, NULL, OPTION_DSPR2},
  {"mno-dspr2", no_argument, NULL, OPTION_NO_DSPR2},
  {"mdspr3", no_argument, NULL, OPTION_DSPR3},
  {"mno-dspr3", no_argument, NULL, OPTION_NO_DSPR3},
  {"meva", no_argument, NULL, OPTION_EVA},
  {"mno-eva", no_argument, NULL, OPTION_NO_EVA},
  {"mmicromips", no_argument, NULL, OPTION_MICROMIPS},
  {"mno-micromips", no_argument, NULL, OPTION_NO_MICROMIPS},
  {"mmcu", no_argument, NULL, OPTION_MCU},
  {"mno-mcu", no_argument, NULL, OPTION_NO_MCU},
  {"mvirt", no_argument, NULL, OPTION_VIRT},
  {"mno-virt", no_argument, NULL, OPTION_NO_VIRT},
  {"mmsa", no_argument, NULL, OPTION_MSA},
  {"mno-msa", no_argument, NULL, OPTION_NO_MSA},
  {"mxpa", no_argument, NULL, OPTION_XPA},
  {"mno-xpa", no_argument, NULL, OPTION_NO_XPA},
  {"mmips16e2", no_argument, NULL, OPTION_MIPS16E2},
  {"mno-mips16e2", no_argument, NULL, OPTION_NO_MIPS16E2},
  {"mcrc", no_argument, NULL, OPTION_CRC},
  {"mno-crc", no_argument, NULL, OPTION_NO_CRC},
  {"mginv", no_argument, NULL, OPTION_GINV},
  {"mno-ginv", no_argument, NULL, OPTION_NO_GINV},
  {"mloongson-mmi", no_argument, NULL, OPTION_LOONGSON_MMI},
  {"mno-loongson-mmi", no_argument, NULL, OPTION_NO_LOONGSON_MMI},
  {"mloongson-cam", no_argument, NULL, OPTION_LOONGSON_CAM},
  {"mno-loongson-cam", no_argument, NULL, OPTION_NO_LOONGSON_CAM},
  {"mloongson-ext", no_argument, NULL, OPTION_LOONGSON_EXT},
  {"mno-loongson-ext", no_argument, NULL, OPTION_NO_LOONGSON_EXT},
  {"mloongson-ext2", no_argument, NULL, OPTION_LOONGSON_EXT2},
  {"mno-loongson-ext2", no_argument, NULL, OPTION_NO_LOONGSON_EXT2},

  /* Old-style architecture options.  Don't add more of these.  */
  {"m4650", no_argument, NULL, OPTION_M4650},
  {"no-m4650", no_argument, NULL, OPTION_NO_M4650},
  {"m4010", no_argument, NULL, OPTION_M4010},
  {"no-m4010", no_argument, NULL, OPTION_NO_M4010},
  {"m4100", no_argument, NULL, OPTION_M4100},
  {"no-m4100", no_argument, NULL, OPTION_NO_M4100},
  {"m3900", no_argument, NULL, OPTION_M3900},
  {"no-m3900", no_argument, NULL, OPTION_NO_M3900},

  /* Options which enable bug fixes.  */
  {"mfix7000", no_argument, NULL, OPTION_M7000_HILO_FIX},
  {"no-fix-7000", no_argument, NULL, OPTION_MNO_7000_HILO_FIX},
  {"mno-fix7000", no_argument, NULL, OPTION_MNO_7000_HILO_FIX},
  {"mfix-loongson3-llsc",   no_argument, NULL, OPTION_FIX_LOONGSON3_LLSC},
  {"mno-fix-loongson3-llsc", no_argument, NULL, OPTION_NO_FIX_LOONGSON3_LLSC},
  {"mfix-loongson2f-jump", no_argument, NULL, OPTION_FIX_LOONGSON2F_JUMP},
  {"mno-fix-loongson2f-jump", no_argument, NULL, OPTION_NO_FIX_LOONGSON2F_JUMP},
  {"mfix-loongson2f-nop", no_argument, NULL, OPTION_FIX_LOONGSON2F_NOP},
  {"mno-fix-loongson2f-nop", no_argument, NULL, OPTION_NO_FIX_LOONGSON2F_NOP},
  {"mfix-vr4120",    no_argument, NULL, OPTION_FIX_VR4120},
  {"mno-fix-vr4120", no_argument, NULL, OPTION_NO_FIX_VR4120},
  {"mfix-vr4130",    no_argument, NULL, OPTION_FIX_VR4130},
  {"mno-fix-vr4130", no_argument, NULL, OPTION_NO_FIX_VR4130},
  {"mfix-24k",    no_argument, NULL, OPTION_FIX_24K},
  {"mno-fix-24k", no_argument, NULL, OPTION_NO_FIX_24K},
  {"mfix-rm7000",    no_argument, NULL, OPTION_FIX_RM7000},
  {"mno-fix-rm7000", no_argument, NULL, OPTION_NO_FIX_RM7000},
  {"mfix-cn63xxp1", no_argument, NULL, OPTION_FIX_CN63XXP1},
  {"mno-fix-cn63xxp1", no_argument, NULL, OPTION_NO_FIX_CN63XXP1},
  {"mfix-r5900", no_argument, NULL, OPTION_FIX_R5900},
  {"mno-fix-r5900", no_argument, NULL, OPTION_NO_FIX_R5900},

  /* Miscellaneous options.  */
  {"trap", no_argument, NULL, OPTION_TRAP},
  {"no-break", no_argument, NULL, OPTION_TRAP},
  {"break", no_argument, NULL, OPTION_BREAK},
  {"no-trap", no_argument, NULL, OPTION_BREAK},
  {"EB", no_argument, NULL, OPTION_EB},
  {"EL", no_argument, NULL, OPTION_EL},
  {"mfp32", no_argument, NULL, OPTION_FP32},
  {"mgp32", no_argument, NULL, OPTION_GP32},
  {"construct-floats", no_argument, NULL, OPTION_CONSTRUCT_FLOATS},
  {"no-construct-floats", no_argument, NULL, OPTION_NO_CONSTRUCT_FLOATS},
  {"mfp64", no_argument, NULL, OPTION_FP64},
  {"mfpxx", no_argument, NULL, OPTION_FPXX},
  {"mgp64", no_argument, NULL, OPTION_GP64},
  {"relax-branch", no_argument, NULL, OPTION_RELAX_BRANCH},
  {"no-relax-branch", no_argument, NULL, OPTION_NO_RELAX_BRANCH},
  {"mignore-branch-isa", no_argument, NULL, OPTION_IGNORE_BRANCH_ISA},
  {"mno-ignore-branch-isa", no_argument, NULL, OPTION_NO_IGNORE_BRANCH_ISA},
  {"minsn32", no_argument, NULL, OPTION_INSN32},
  {"mno-insn32", no_argument, NULL, OPTION_NO_INSN32},
  {"mshared", no_argument, NULL, OPTION_MSHARED},
  {"mno-shared", no_argument, NULL, OPTION_MNO_SHARED},
  {"msym32", no_argument, NULL, OPTION_MSYM32},
  {"mno-sym32", no_argument, NULL, OPTION_MNO_SYM32},
  {"msoft-float", no_argument, NULL, OPTION_SOFT_FLOAT},
  {"mhard-float", no_argument, NULL, OPTION_HARD_FLOAT},
  {"msingle-float", no_argument, NULL, OPTION_SINGLE_FLOAT},
  {"mdouble-float", no_argument, NULL, OPTION_DOUBLE_FLOAT},
  {"modd-spreg", no_argument, NULL, OPTION_ODD_SPREG},
  {"mno-odd-spreg", no_argument, NULL, OPTION_NO_ODD_SPREG},

  /* Strictly speaking this next option is ELF specific,
     but we allow it for other ports as well in order to
     make testing easier.  */
  {"32", no_argument, NULL, OPTION_32},

  /* ELF-specific options.  */
  {"KPIC", no_argument, NULL, OPTION_CALL_SHARED},
  {"call_shared", no_argument, NULL, OPTION_CALL_SHARED},
  {"call_nonpic", no_argument, NULL, OPTION_CALL_NONPIC},
  {"non_shared",  no_argument, NULL, OPTION_NON_SHARED},
  {"xgot", no_argument, NULL, OPTION_XGOT},
  {"mabi", required_argument, NULL, OPTION_MABI},
  {"n32", no_argument, NULL, OPTION_N32},
  {"64", no_argument, NULL, OPTION_64},
  {"mdebug", no_argument, NULL, OPTION_MDEBUG},
  {"no-mdebug", no_argument, NULL, OPTION_NO_MDEBUG},
  {"mpdr", no_argument, NULL, OPTION_PDR},
  {"mno-pdr", no_argument, NULL, OPTION_NO_PDR},
  {"mvxworks-pic", no_argument, NULL, OPTION_MVXWORKS_PIC},
  {"mnan", required_argument, NULL, OPTION_NAN},

  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

/* Information about either an Application Specific Extension or an
   optional architecture feature that, for simplicity, we treat in the
   same way as an ASE.  */
struct mips_ase
{
  /* The name of the ASE, used in both the command-line and .set options.  */
  const char *name;

  /* The associated ASE_* flags.  If the ASE is available on both 32-bit
     and 64-bit architectures, the flags here refer to the subset that
     is available on both.  */
  unsigned int flags;

  /* The ASE_* flag used for instructions that are available on 64-bit
     architectures but that are not included in FLAGS.  */
  unsigned int flags64;

  /* The command-line options that turn the ASE on and off.  */
  int option_on;
  int option_off;

  /* The minimum required architecture revisions for MIPS32, MIPS64,
     microMIPS32 and microMIPS64, or -1 if the extension isn't supported.  */
  int mips32_rev;
  int mips64_rev;
  int micromips32_rev;
  int micromips64_rev;

  /* The architecture where the ASE was removed or -1 if the extension has not
     been removed.  */
  int rem_rev;
};

/* A table of all supported ASEs.  */
static const struct mips_ase mips_ases[] = {
  { "dsp", ASE_DSP, ASE_DSP64,
    OPTION_DSP, OPTION_NO_DSP,
    2, 2, 2, 2,
    -1 },

  { "dspr2", ASE_DSP | ASE_DSPR2, 0,
    OPTION_DSPR2, OPTION_NO_DSPR2,
    2, 2, 2, 2,
    -1 },

  { "dspr3", ASE_DSP | ASE_DSPR2 | ASE_DSPR3, 0,
    OPTION_DSPR3, OPTION_NO_DSPR3,
    6, 6, -1, -1,
    -1 },

  { "eva", ASE_EVA, 0,
    OPTION_EVA, OPTION_NO_EVA,
     2,  2,  2,  2,
    -1 },

  { "mcu", ASE_MCU, 0,
    OPTION_MCU, OPTION_NO_MCU,
     2,  2,  2,  2,
    -1 },

  /* Deprecated in MIPS64r5, but we don't implement that yet.  */
  { "mdmx", ASE_MDMX, 0,
    OPTION_MDMX, OPTION_NO_MDMX,
    -1, 1, -1, -1,
     6 },

  /* Requires 64-bit FPRs, so the minimum MIPS32 revision is 2.  */
  { "mips3d", ASE_MIPS3D, 0,
    OPTION_MIPS3D, OPTION_NO_MIPS3D,
    2, 1, -1, -1,
    6 },

  { "mt", ASE_MT, 0,
    OPTION_MT, OPTION_NO_MT,
     2,  2, -1, -1,
    -1 },

  { "smartmips", ASE_SMARTMIPS, 0,
    OPTION_SMARTMIPS, OPTION_NO_SMARTMIPS,
    1, -1, -1, -1,
    6 },

  { "virt", ASE_VIRT, ASE_VIRT64,
    OPTION_VIRT, OPTION_NO_VIRT,
     2,  2,  2,  2,
    -1 },

  { "msa", ASE_MSA, ASE_MSA64,
    OPTION_MSA, OPTION_NO_MSA,
     2,  2,  2,  2,
    -1 },

  { "xpa", ASE_XPA, 0,
    OPTION_XPA, OPTION_NO_XPA,
    2, 2, 2, 2,
    -1 },

  { "mips16e2", ASE_MIPS16E2, 0,
    OPTION_MIPS16E2, OPTION_NO_MIPS16E2,
    2,  2, -1, -1,
    6 },

  { "crc", ASE_CRC, ASE_CRC64,
    OPTION_CRC, OPTION_NO_CRC,
    6,  6, -1, -1,
    -1 },

  { "ginv", ASE_GINV, 0,
    OPTION_GINV, OPTION_NO_GINV,
    6,  6, 6, 6,
    -1 },

  { "loongson-mmi", ASE_LOONGSON_MMI, 0,
    OPTION_LOONGSON_MMI, OPTION_NO_LOONGSON_MMI,
    0, 0, -1, -1,
    -1 },

  { "loongson-cam", ASE_LOONGSON_CAM, 0,
    OPTION_LOONGSON_CAM, OPTION_NO_LOONGSON_CAM,
    0, 0, -1, -1,
    -1 },

  { "loongson-ext", ASE_LOONGSON_EXT, 0,
    OPTION_LOONGSON_EXT, OPTION_NO_LOONGSON_EXT,
    0, 0, -1, -1,
    -1 },

  { "loongson-ext2", ASE_LOONGSON_EXT | ASE_LOONGSON_EXT2, 0,
    OPTION_LOONGSON_EXT2, OPTION_NO_LOONGSON_EXT2,
    0, 0, -1, -1,
    -1 },
};

/* The set of ASEs that require -mfp64.  */
#define FP64_ASES (ASE_MIPS3D | ASE_MDMX | ASE_MSA)

/* Groups of ASE_* flags that represent different revisions of an ASE.  */
static const unsigned int mips_ase_groups[] = {
  ASE_DSP | ASE_DSPR2 | ASE_DSPR3, 
  ASE_LOONGSON_EXT | ASE_LOONGSON_EXT2 
};

/* Pseudo-op table.

   The following pseudo-ops from the Kane and Heinrich MIPS book
   should be defined here, but are currently unsupported: .alias,
   .galive, .gjaldef, .gjrlive, .livereg, .noalias.

   The following pseudo-ops from the Kane and Heinrich MIPS book are
   specific to the type of debugging information being generated, and
   should be defined by the object format: .aent, .begin, .bend,
   .bgnb, .end, .endb, .ent, .fmask, .frame, .loc, .mask, .verstamp,
   .vreg.

   The following pseudo-ops from the Kane and Heinrich MIPS book are
   not MIPS CPU specific, but are also not specific to the object file
   format.  This file is probably the best place to define them, but
   they are not currently supported: .asm0, .endr, .lab, .struct.  */

static const pseudo_typeS mips_pseudo_table[] =
{
  /* MIPS specific pseudo-ops.  */
  {"option", s_option, 0},
  {"set", s_mipsset, 0},
  {"rdata", s_change_sec, 'r'},
  {"sdata", s_change_sec, 's'},
  {"livereg", s_ignore, 0},
  {"abicalls", s_abicalls, 0},
  {"cpload", s_cpload, 0},
  {"cpsetup", s_cpsetup, 0},
  {"cplocal", s_cplocal, 0},
  {"cprestore", s_cprestore, 0},
  {"cpreturn", s_cpreturn, 0},
  {"dtprelword", s_dtprelword, 0},
  {"dtpreldword", s_dtpreldword, 0},
  {"tprelword", s_tprelword, 0},
  {"tpreldword", s_tpreldword, 0},
  {"gpvalue", s_gpvalue, 0},
  {"gpword", s_gpword, 0},
  {"gpdword", s_gpdword, 0},
  {"ehword", s_ehword, 0},
  {"cpadd", s_cpadd, 0},
  {"insn", s_insn, 0},
  {"nan", s_nan, 0},
  {"module", s_module, 0},

  /* Relatively generic pseudo-ops that happen to be used on MIPS
     chips.  */
  {"asciiz", stringer, 8 + 1},
  {"bss", s_change_sec, 'b'},
  {"err", s_err, 0},
  {"half", s_cons, 1},
  {"dword", s_cons, 3},
  {"weakext", s_mips_weakext, 0},
  {"origin", s_org, 0},
  {"repeat", s_rept, 0},

  /* For MIPS this is non-standard, but we define it for consistency.  */
  {"sbss", s_change_sec, 'B'},

  /* These pseudo-ops are defined in read.c, but must be overridden
     here for one reason or another.  */
  {"align", s_align, 0},
  {"byte", s_cons, 0},
  {"data", s_change_sec, 'd'},
  {"double", s_float_cons, 'd'},
  {"float", s_float_cons, 'f'},
  {"globl", s_mips_globl, 0},
  {"global", s_mips_globl, 0},
  {"hword", s_cons, 1},
  {"int", s_cons, 2},
  {"long", s_cons, 2},
  {"octa", s_cons, 4},
  {"quad", s_cons, 3},
  {"section", s_change_section, 0},
  {"short", s_cons, 1},
  {"single", s_float_cons, 'f'},
  {"stabd", s_mips_stab, 'd'},
  {"stabn", s_mips_stab, 'n'},
  {"stabs", s_mips_stab, 's'},
  {"text", s_change_sec, 't'},
  {"word", s_cons, 2},

  { "extern", ecoff_directive_extern, 0},

  { NULL, NULL, 0 },
};

static const pseudo_typeS mips_nonecoff_pseudo_table[] =
{
  /* These pseudo-ops should be defined by the object file format.
     However, a.out doesn't support them, so we have versions here.  */
  {"aent", s_mips_ent, 1},
  {"bgnb", s_ignore, 0},
  {"end", s_mips_end, 0},
  {"endb", s_ignore, 0},
  {"ent", s_mips_ent, 0},
  {"file", s_mips_file, 0},
  {"fmask", s_mips_mask, 'F'},
  {"frame", s_mips_frame, 0},
  {"loc", s_mips_loc, 0},
  {"mask", s_mips_mask, 'R'},
  {"verstamp", s_ignore, 0},
  { NULL, NULL, 0 },
};

/* Export the ABI address size for use by TC_ADDRESS_BYTES for the
   purpose of the `.dc.a' internal pseudo-op.  */

int
mips_address_bytes (void)
{
  file_mips_check_options ();
  return HAVE_64BIT_ADDRESSES ? 8 : 4;
}

extern void pop_insert (const pseudo_typeS *);

void
mips_pop_insert (void)
{
  pop_insert (mips_pseudo_table);
  if (! ECOFF_DEBUGGING)
    pop_insert (mips_nonecoff_pseudo_table);
}

/* Symbols labelling the current insn.  */

struct insn_label_list
{
  struct insn_label_list *next;
  symbolS *label;
};

static struct insn_label_list *free_insn_labels;
#define label_list tc_segment_info_data.labels

static void mips_clear_insn_labels (void);
static void mips_mark_labels (void);
static void mips_compressed_mark_labels (void);

static inline void
mips_clear_insn_labels (void)
{
  struct insn_label_list **pl;
  segment_info_type *si;

  if (now_seg)
    {
      for (pl = &free_insn_labels; *pl != NULL; pl = &(*pl)->next)
	;

      si = seg_info (now_seg);
      *pl = si->label_list;
      si->label_list = NULL;
    }
}

/* Mark instruction labels in MIPS16/microMIPS mode.  */

static inline void
mips_mark_labels (void)
{
  if (HAVE_CODE_COMPRESSION)
    mips_compressed_mark_labels ();
}

static char *expr_parse_end;

/* An expression in a macro instruction.  This is set by mips_ip and
   mips16_ip and when populated is always an O_constant.  */

static expressionS imm_expr;

/* The relocatable field in an instruction and the relocs associated
   with it.  These variables are used for instructions like LUI and
   JAL as well as true offsets.  They are also used for address
   operands in macros.  */

static expressionS offset_expr;
static bfd_reloc_code_real_type offset_reloc[3]
  = {BFD_RELOC_UNUSED, BFD_RELOC_UNUSED, BFD_RELOC_UNUSED};

/* This is set to the resulting size of the instruction to be produced
   by mips16_ip if an explicit extension is used or by mips_ip if an
   explicit size is supplied.  */

static unsigned int forced_insn_length;

/* True if we are assembling an instruction.  All dot symbols defined during
   this time should be treated as code labels.  */

static bool mips_assembling_insn;

/* The pdr segment for per procedure frame/regmask info.  Not used for
   ECOFF debugging.  */

static segT pdr_seg;

/* The default target format to use.  */

#if defined (TE_FreeBSD)
#define ELF_TARGET(PREFIX, ENDIAN) PREFIX "trad" ENDIAN "mips-freebsd"
#elif defined (TE_TMIPS)
#define ELF_TARGET(PREFIX, ENDIAN) PREFIX "trad" ENDIAN "mips"
#else
#define ELF_TARGET(PREFIX, ENDIAN) PREFIX ENDIAN "mips"
#endif

const char *
mips_target_format (void)
{
  switch (OUTPUT_FLAVOR)
    {
    case bfd_target_elf_flavour:
#ifdef TE_VXWORKS
      if (!HAVE_64BIT_OBJECTS && !HAVE_NEWABI)
	return (target_big_endian
		? "elf32-bigmips-vxworks"
		: "elf32-littlemips-vxworks");
#endif
      return (target_big_endian
	      ? (HAVE_64BIT_OBJECTS
		 ? ELF_TARGET ("elf64-", "big")
		 : (HAVE_NEWABI
		    ? ELF_TARGET ("elf32-n", "big")
		    : ELF_TARGET ("elf32-", "big")))
	      : (HAVE_64BIT_OBJECTS
		 ? ELF_TARGET ("elf64-", "little")
		 : (HAVE_NEWABI
		    ? ELF_TARGET ("elf32-n", "little")
		    : ELF_TARGET ("elf32-", "little"))));
    default:
      abort ();
      return NULL;
    }
}

/* Return the ISA revision that is currently in use, or 0 if we are
   generating code for MIPS V or below.  */

static int
mips_isa_rev (void)
{
  if (mips_opts.isa == ISA_MIPS32R2 || mips_opts.isa == ISA_MIPS64R2)
    return 2;

  if (mips_opts.isa == ISA_MIPS32R3 || mips_opts.isa == ISA_MIPS64R3)
    return 3;

  if (mips_opts.isa == ISA_MIPS32R5 || mips_opts.isa == ISA_MIPS64R5)
    return 5;

  if (mips_opts.isa == ISA_MIPS32R6 || mips_opts.isa == ISA_MIPS64R6)
    return 6;

  /* microMIPS implies revision 2 or above.  */
  if (mips_opts.micromips)
    return 2;

  if (mips_opts.isa == ISA_MIPS32 || mips_opts.isa == ISA_MIPS64)
    return 1;

  return 0;
}

/* Return the mask of all ASEs that are revisions of those in FLAGS.  */

static unsigned int
mips_ase_mask (unsigned int flags)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (mips_ase_groups); i++)
    if (flags & mips_ase_groups[i])
      flags |= mips_ase_groups[i];
  return flags;
}

/* Check whether the current ISA supports ASE.  Issue a warning if
   appropriate.  */

static void
mips_check_isa_supports_ase (const struct mips_ase *ase)
{
  const char *base;
  int min_rev, size;
  static unsigned int warned_isa;
  static unsigned int warned_fp32;

  if (ISA_HAS_64BIT_REGS (mips_opts.isa))
    min_rev = mips_opts.micromips ? ase->micromips64_rev : ase->mips64_rev;
  else
    min_rev = mips_opts.micromips ? ase->micromips32_rev : ase->mips32_rev;
  if ((min_rev < 0 || mips_isa_rev () < min_rev)
      && (warned_isa & ase->flags) != ase->flags)
    {
      warned_isa |= ase->flags;
      base = mips_opts.micromips ? "microMIPS" : "MIPS";
      size = ISA_HAS_64BIT_REGS (mips_opts.isa) ? 64 : 32;
      if (min_rev < 0)
	as_warn (_("the %d-bit %s architecture does not support the"
		   " `%s' extension"), size, base, ase->name);
      else
	as_warn (_("the `%s' extension requires %s%d revision %d or greater"),
		 ase->name, base, size, min_rev);
    }
  else if ((ase->rem_rev > 0 && mips_isa_rev () >= ase->rem_rev)
	   && (warned_isa & ase->flags) != ase->flags)
    {
      warned_isa |= ase->flags;
      base = mips_opts.micromips ? "microMIPS" : "MIPS";
      size = ISA_HAS_64BIT_REGS (mips_opts.isa) ? 64 : 32;
      as_warn (_("the `%s' extension was removed in %s%d revision %d"),
	       ase->name, base, size, ase->rem_rev);
    }

  if ((ase->flags & FP64_ASES)
      && mips_opts.fp != 64
      && (warned_fp32 & ase->flags) != ase->flags)
    {
      warned_fp32 |= ase->flags;
      as_warn (_("the `%s' extension requires 64-bit FPRs"), ase->name);
    }
}

/* Check all enabled ASEs to see whether they are supported by the
   chosen architecture.  */

static void
mips_check_isa_supports_ases (void)
{
  unsigned int i, mask;

  for (i = 0; i < ARRAY_SIZE (mips_ases); i++)
    {
      mask = mips_ase_mask (mips_ases[i].flags);
      if ((mips_opts.ase & mask) == mips_ases[i].flags)
	mips_check_isa_supports_ase (&mips_ases[i]);
    }
}

/* Set the state of ASE to ENABLED_P.  Return the mask of ASE_* flags
   that were affected.  */

static unsigned int
mips_set_ase (const struct mips_ase *ase, struct mips_set_options *opts,
	      bool enabled_p)
{
  unsigned int mask;

  mask = mips_ase_mask (ase->flags);
  opts->ase &= ~mask;

  /* Clear combination ASE flags, which need to be recalculated based on
     updated regular ASE settings.  */
  opts->ase &= ~(ASE_MIPS16E2_MT | ASE_XPA_VIRT | ASE_EVA_R6);

  if (enabled_p)
    opts->ase |= ase->flags;

  /* The Virtualization ASE has eXtended Physical Addressing (XPA)
     instructions which are only valid when both ASEs are enabled.
     This sets the ASE_XPA_VIRT flag when both ASEs are present.  */
  if ((opts->ase & (ASE_XPA | ASE_VIRT)) == (ASE_XPA | ASE_VIRT))
    {
      opts->ase |= ASE_XPA_VIRT;
      mask |= ASE_XPA_VIRT;
    }
  if ((opts->ase & (ASE_MIPS16E2 | ASE_MT)) == (ASE_MIPS16E2 | ASE_MT))
    {
      opts->ase |= ASE_MIPS16E2_MT;
      mask |= ASE_MIPS16E2_MT;
    }

  /* The EVA Extension has instructions which are only valid when the R6 ISA
     is enabled.  This sets the ASE_EVA_R6 flag when both EVA and R6 ISA are
     present.  */
  if (((opts->ase & ASE_EVA) != 0) && ISA_IS_R6 (opts->isa))
    {
      opts->ase |= ASE_EVA_R6;
      mask |= ASE_EVA_R6;
    }

  return mask;
}

/* Return the ASE called NAME, or null if none.  */

static const struct mips_ase *
mips_lookup_ase (const char *name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (mips_ases); i++)
    if (strcmp (name, mips_ases[i].name) == 0)
      return &mips_ases[i];
  return NULL;
}

/* Return the length of a microMIPS instruction in bytes.  If bits of
   the mask beyond the low 16 are 0, then it is a 16-bit instruction,
   otherwise it is a 32-bit instruction.  */

static inline unsigned int
micromips_insn_length (const struct mips_opcode *mo)
{
  return mips_opcode_32bit_p (mo) ? 4 : 2;
}

/* Return the length of MIPS16 instruction OPCODE.  */

static inline unsigned int
mips16_opcode_length (unsigned long opcode)
{
  return (opcode >> 16) == 0 ? 2 : 4;
}

/* Return the length of instruction INSN.  */

static inline unsigned int
insn_length (const struct mips_cl_insn *insn)
{
  if (mips_opts.micromips)
    return micromips_insn_length (insn->insn_mo);
  else if (mips_opts.mips16)
    return mips16_opcode_length (insn->insn_opcode);
  else
    return 4;
}

/* Initialise INSN from opcode entry MO.  Leave its position unspecified.  */

static void
create_insn (struct mips_cl_insn *insn, const struct mips_opcode *mo)
{
  size_t i;

  insn->insn_mo = mo;
  insn->insn_opcode = mo->match;
  insn->frag = NULL;
  insn->where = 0;
  for (i = 0; i < ARRAY_SIZE (insn->fixp); i++)
    insn->fixp[i] = NULL;
  insn->fixed_p = (mips_opts.noreorder > 0);
  insn->noreorder_p = (mips_opts.noreorder > 0);
  insn->mips16_absolute_jump_p = 0;
  insn->complete_p = 0;
  insn->cleared_p = 0;
}

/* Get a list of all the operands in INSN.  */

static const struct mips_operand_array *
insn_operands (const struct mips_cl_insn *insn)
{
  if (insn->insn_mo >= &mips_opcodes[0]
      && insn->insn_mo < &mips_opcodes[NUMOPCODES])
    return &mips_operands[insn->insn_mo - &mips_opcodes[0]];

  if (insn->insn_mo >= &mips16_opcodes[0]
      && insn->insn_mo < &mips16_opcodes[bfd_mips16_num_opcodes])
    return &mips16_operands[insn->insn_mo - &mips16_opcodes[0]];

  if (insn->insn_mo >= &micromips_opcodes[0]
      && insn->insn_mo < &micromips_opcodes[bfd_micromips_num_opcodes])
    return &micromips_operands[insn->insn_mo - &micromips_opcodes[0]];

  abort ();
}

/* Get a description of operand OPNO of INSN.  */

static const struct mips_operand *
insn_opno (const struct mips_cl_insn *insn, unsigned opno)
{
  const struct mips_operand_array *operands;

  operands = insn_operands (insn);
  if (opno >= MAX_OPERANDS || !operands->operand[opno])
    abort ();
  return operands->operand[opno];
}

/* Install UVAL as the value of OPERAND in INSN.  */

static inline void
insn_insert_operand (struct mips_cl_insn *insn,
		     const struct mips_operand *operand, unsigned int uval)
{
  if (mips_opts.mips16
      && operand->type == OP_INT && operand->lsb == 0
      && mips_opcode_32bit_p (insn->insn_mo))
    insn->insn_opcode |= mips16_immed_extend (uval, operand->size);
  else
    insn->insn_opcode = mips_insert_operand (operand, insn->insn_opcode, uval);
}

/* Extract the value of OPERAND from INSN.  */

static inline unsigned
insn_extract_operand (const struct mips_cl_insn *insn,
		      const struct mips_operand *operand)
{
  return mips_extract_operand (operand, insn->insn_opcode);
}

/* Record the current MIPS16/microMIPS mode in now_seg.  */

static void
mips_record_compressed_mode (void)
{
  segment_info_type *si;

  si = seg_info (now_seg);
  if (si->tc_segment_info_data.mips16 != mips_opts.mips16)
    si->tc_segment_info_data.mips16 = mips_opts.mips16;
  if (si->tc_segment_info_data.micromips != mips_opts.micromips)
    si->tc_segment_info_data.micromips = mips_opts.micromips;
}

/* Read a standard MIPS instruction from BUF.  */

static unsigned long
read_insn (char *buf)
{
  if (target_big_endian)
    return bfd_getb32 ((bfd_byte *) buf);
  else
    return bfd_getl32 ((bfd_byte *) buf);
}

/* Write standard MIPS instruction INSN to BUF.  Return a pointer to
   the next byte.  */

static char *
write_insn (char *buf, unsigned int insn)
{
  md_number_to_chars (buf, insn, 4);
  return buf + 4;
}

/* Read a microMIPS or MIPS16 opcode from BUF, given that it
   has length LENGTH.  */

static unsigned long
read_compressed_insn (char *buf, unsigned int length)
{
  unsigned long insn;
  unsigned int i;

  insn = 0;
  for (i = 0; i < length; i += 2)
    {
      insn <<= 16;
      if (target_big_endian)
	insn |= bfd_getb16 ((char *) buf);
      else
	insn |= bfd_getl16 ((char *) buf);
      buf += 2;
    }
  return insn;
}

/* Write microMIPS or MIPS16 instruction INSN to BUF, given that the
   instruction is LENGTH bytes long.  Return a pointer to the next byte.  */

static char *
write_compressed_insn (char *buf, unsigned int insn, unsigned int length)
{
  unsigned int i;

  for (i = 0; i < length; i += 2)
    md_number_to_chars (buf + i, insn >> ((length - i - 2) * 8), 2);
  return buf + length;
}

/* Install INSN at the location specified by its "frag" and "where" fields.  */

static void
install_insn (const struct mips_cl_insn *insn)
{
  char *f = insn->frag->fr_literal + insn->where;
  if (HAVE_CODE_COMPRESSION)
    write_compressed_insn (f, insn->insn_opcode, insn_length (insn));
  else
    write_insn (f, insn->insn_opcode);
  mips_record_compressed_mode ();
}

/* Move INSN to offset WHERE in FRAG.  Adjust the fixups accordingly
   and install the opcode in the new location.  */

static void
move_insn (struct mips_cl_insn *insn, fragS *frag, long where)
{
  size_t i;

  insn->frag = frag;
  insn->where = where;
  for (i = 0; i < ARRAY_SIZE (insn->fixp); i++)
    if (insn->fixp[i] != NULL)
      {
	insn->fixp[i]->fx_frag = frag;
	insn->fixp[i]->fx_where = where;
      }
  install_insn (insn);
}

/* Add INSN to the end of the output.  */

static void
add_fixed_insn (struct mips_cl_insn *insn)
{
  char *f = frag_more (insn_length (insn));
  move_insn (insn, frag_now, f - frag_now->fr_literal);
}

/* Start a variant frag and move INSN to the start of the variant part,
   marking it as fixed.  The other arguments are as for frag_var.  */

static void
add_relaxed_insn (struct mips_cl_insn *insn, int max_chars, int var,
		  relax_substateT subtype, symbolS *symbol, offsetT offset)
{
  frag_grow (max_chars);
  move_insn (insn, frag_now, frag_more (0) - frag_now->fr_literal);
  insn->fixed_p = 1;
  frag_var (rs_machine_dependent, max_chars, var,
	    subtype, symbol, offset, NULL);
}

/* Insert N copies of INSN into the history buffer, starting at
   position FIRST.  Neither FIRST nor N need to be clipped.  */

static void
insert_into_history (unsigned int first, unsigned int n,
		     const struct mips_cl_insn *insn)
{
  if (mips_relax.sequence != 2)
    {
      unsigned int i;

      for (i = ARRAY_SIZE (history); i-- > first;)
	if (i >= first + n)
	  history[i] = history[i - n];
	else
	  history[i] = *insn;
    }
}

/* Clear the error in insn_error.  */

static void
clear_insn_error (void)
{
  memset (&insn_error, 0, sizeof (insn_error));
}

/* Possibly record error message MSG for the current instruction.
   If the error is about a particular argument, ARGNUM is the 1-based
   number of that argument, otherwise it is 0.  FORMAT is the format
   of MSG.  Return true if MSG was used, false if the current message
   was kept.  */

static bool
set_insn_error_format (int argnum, enum mips_insn_error_format format,
		       const char *msg)
{
  if (argnum == 0)
    {
      /* Give priority to errors against specific arguments, and to
	 the first whole-instruction message.  */
      if (insn_error.msg)
	return false;
    }
  else
    {
      /* Keep insn_error if it is against a later argument.  */
      if (argnum < insn_error.min_argnum)
	return false;

      /* If both errors are against the same argument but are different,
	 give up on reporting a specific error for this argument.
	 See the comment about mips_insn_error for details.  */
      if (argnum == insn_error.min_argnum
	  && insn_error.msg
	  && strcmp (insn_error.msg, msg) != 0)
	{
	  insn_error.msg = 0;
	  insn_error.min_argnum += 1;
	  return false;
	}
    }
  insn_error.min_argnum = argnum;
  insn_error.format = format;
  insn_error.msg = msg;
  return true;
}

/* Record an instruction error with no % format fields.  ARGNUM and MSG are
   as for set_insn_error_format.  */

static void
set_insn_error (int argnum, const char *msg)
{
  set_insn_error_format (argnum, ERR_FMT_PLAIN, msg);
}

/* Record an instruction error with one %d field I.  ARGNUM and MSG are
   as for set_insn_error_format.  */

static void
set_insn_error_i (int argnum, const char *msg, int i)
{
  if (set_insn_error_format (argnum, ERR_FMT_I, msg))
    insn_error.u.i = i;
}

/* Record an instruction error with two %s fields S1 and S2.  ARGNUM and MSG
   are as for set_insn_error_format.  */

static void
set_insn_error_ss (int argnum, const char *msg, const char *s1, const char *s2)
{
  if (set_insn_error_format (argnum, ERR_FMT_SS, msg))
    {
      insn_error.u.ss[0] = s1;
      insn_error.u.ss[1] = s2;
    }
}

/* Report the error in insn_error, which is against assembly code STR.  */

static void
report_insn_error (const char *str)
{
  const char *msg = concat (insn_error.msg, " `%s'", NULL);

  switch (insn_error.format)
    {
    case ERR_FMT_PLAIN:
      as_bad (msg, str);
      break;

    case ERR_FMT_I:
      as_bad (msg, insn_error.u.i, str);
      break;

    case ERR_FMT_SS:
      as_bad (msg, insn_error.u.ss[0], insn_error.u.ss[1], str);
      break;
    }

  free ((char *) msg);
}

/* Initialize vr4120_conflicts.  There is a bit of duplication here:
   the idea is to make it obvious at a glance that each errata is
   included.  */

static void
init_vr4120_conflicts (void)
{
#define CONFLICT(FIRST, SECOND) \
    vr4120_conflicts[FIX_VR4120_##FIRST] |= 1 << FIX_VR4120_##SECOND

  /* Errata 21 - [D]DIV[U] after [D]MACC */
  CONFLICT (MACC, DIV);
  CONFLICT (DMACC, DIV);

  /* Errata 23 - Continuous DMULT[U]/DMACC instructions.  */
  CONFLICT (DMULT, DMULT);
  CONFLICT (DMULT, DMACC);
  CONFLICT (DMACC, DMULT);
  CONFLICT (DMACC, DMACC);

  /* Errata 24 - MT{LO,HI} after [D]MACC */
  CONFLICT (MACC, MTHILO);
  CONFLICT (DMACC, MTHILO);

  /* VR4181A errata MD(1): "If a MULT, MULTU, DMULT or DMULTU
     instruction is executed immediately after a MACC or DMACC
     instruction, the result of [either instruction] is incorrect."  */
  CONFLICT (MACC, MULT);
  CONFLICT (MACC, DMULT);
  CONFLICT (DMACC, MULT);
  CONFLICT (DMACC, DMULT);

  /* VR4181A errata MD(4): "If a MACC or DMACC instruction is
     executed immediately after a DMULT, DMULTU, DIV, DIVU,
     DDIV or DDIVU instruction, the result of the MACC or
     DMACC instruction is incorrect.".  */
  CONFLICT (DMULT, MACC);
  CONFLICT (DMULT, DMACC);
  CONFLICT (DIV, MACC);
  CONFLICT (DIV, DMACC);

#undef CONFLICT
}

struct regname {
  const char *name;
  unsigned int num;
};

#define RNUM_MASK	0x00000ff
#define RTYPE_MASK	0x0ffff00
#define RTYPE_NUM	0x0000100
#define RTYPE_FPU	0x0000200
#define RTYPE_FCC	0x0000400
#define RTYPE_VEC	0x0000800
#define RTYPE_GP	0x0001000
#define RTYPE_CP0	0x0002000
#define RTYPE_PC	0x0004000
#define RTYPE_ACC	0x0008000
#define RTYPE_CCC	0x0010000
#define RTYPE_VI	0x0020000
#define RTYPE_VF	0x0040000
#define RTYPE_R5900_I	0x0080000
#define RTYPE_R5900_Q	0x0100000
#define RTYPE_R5900_R	0x0200000
#define RTYPE_R5900_ACC	0x0400000
#define RTYPE_MSA	0x0800000
#define RWARN		0x8000000

#define GENERIC_REGISTER_NUMBERS \
    {"$0",	RTYPE_NUM | 0},  \
    {"$1",	RTYPE_NUM | 1},  \
    {"$2",	RTYPE_NUM | 2},  \
    {"$3",	RTYPE_NUM | 3},  \
    {"$4",	RTYPE_NUM | 4},  \
    {"$5",	RTYPE_NUM | 5},  \
    {"$6",	RTYPE_NUM | 6},  \
    {"$7",	RTYPE_NUM | 7},  \
    {"$8",	RTYPE_NUM | 8},  \
    {"$9",	RTYPE_NUM | 9},  \
    {"$10",	RTYPE_NUM | 10}, \
    {"$11",	RTYPE_NUM | 11}, \
    {"$12",	RTYPE_NUM | 12}, \
    {"$13",	RTYPE_NUM | 13}, \
    {"$14",	RTYPE_NUM | 14}, \
    {"$15",	RTYPE_NUM | 15}, \
    {"$16",	RTYPE_NUM | 16}, \
    {"$17",	RTYPE_NUM | 17}, \
    {"$18",	RTYPE_NUM | 18}, \
    {"$19",	RTYPE_NUM | 19}, \
    {"$20",	RTYPE_NUM | 20}, \
    {"$21",	RTYPE_NUM | 21}, \
    {"$22",	RTYPE_NUM | 22}, \
    {"$23",	RTYPE_NUM | 23}, \
    {"$24",	RTYPE_NUM | 24}, \
    {"$25",	RTYPE_NUM | 25}, \
    {"$26",	RTYPE_NUM | 26}, \
    {"$27",	RTYPE_NUM | 27}, \
    {"$28",	RTYPE_NUM | 28}, \
    {"$29",	RTYPE_NUM | 29}, \
    {"$30",	RTYPE_NUM | 30}, \
    {"$31",	RTYPE_NUM | 31}

#define FPU_REGISTER_NAMES       \
    {"$f0",	RTYPE_FPU | 0},  \
    {"$f1",	RTYPE_FPU | 1},  \
    {"$f2",	RTYPE_FPU | 2},  \
    {"$f3",	RTYPE_FPU | 3},  \
    {"$f4",	RTYPE_FPU | 4},  \
    {"$f5",	RTYPE_FPU | 5},  \
    {"$f6",	RTYPE_FPU | 6},  \
    {"$f7",	RTYPE_FPU | 7},  \
    {"$f8",	RTYPE_FPU | 8},  \
    {"$f9",	RTYPE_FPU | 9},  \
    {"$f10",	RTYPE_FPU | 10}, \
    {"$f11",	RTYPE_FPU | 11}, \
    {"$f12",	RTYPE_FPU | 12}, \
    {"$f13",	RTYPE_FPU | 13}, \
    {"$f14",	RTYPE_FPU | 14}, \
    {"$f15",	RTYPE_FPU | 15}, \
    {"$f16",	RTYPE_FPU | 16}, \
    {"$f17",	RTYPE_FPU | 17}, \
    {"$f18",	RTYPE_FPU | 18}, \
    {"$f19",	RTYPE_FPU | 19}, \
    {"$f20",	RTYPE_FPU | 20}, \
    {"$f21",	RTYPE_FPU | 21}, \
    {"$f22",	RTYPE_FPU | 22}, \
    {"$f23",	RTYPE_FPU | 23}, \
    {"$f24",	RTYPE_FPU | 24}, \
    {"$f25",	RTYPE_FPU | 25}, \
    {"$f26",	RTYPE_FPU | 26}, \
    {"$f27",	RTYPE_FPU | 27}, \
    {"$f28",	RTYPE_FPU | 28}, \
    {"$f29",	RTYPE_FPU | 29}, \
    {"$f30",	RTYPE_FPU | 30}, \
    {"$f31",	RTYPE_FPU | 31}

#define FPU_CONDITION_CODE_NAMES \
    {"$fcc0",	RTYPE_FCC | 0},  \
    {"$fcc1",	RTYPE_FCC | 1},  \
    {"$fcc2",	RTYPE_FCC | 2},  \
    {"$fcc3",	RTYPE_FCC | 3},  \
    {"$fcc4",	RTYPE_FCC | 4},  \
    {"$fcc5",	RTYPE_FCC | 5},  \
    {"$fcc6",	RTYPE_FCC | 6},  \
    {"$fcc7",	RTYPE_FCC | 7}

#define COPROC_CONDITION_CODE_NAMES         \
    {"$cc0",	RTYPE_FCC | RTYPE_CCC | 0}, \
    {"$cc1",	RTYPE_FCC | RTYPE_CCC | 1}, \
    {"$cc2",	RTYPE_FCC | RTYPE_CCC | 2}, \
    {"$cc3",	RTYPE_FCC | RTYPE_CCC | 3}, \
    {"$cc4",	RTYPE_FCC | RTYPE_CCC | 4}, \
    {"$cc5",	RTYPE_FCC | RTYPE_CCC | 5}, \
    {"$cc6",	RTYPE_FCC | RTYPE_CCC | 6}, \
    {"$cc7",	RTYPE_FCC | RTYPE_CCC | 7}

#define N32N64_SYMBOLIC_REGISTER_NAMES \
    {"$a4",	RTYPE_GP | 8},  \
    {"$a5",	RTYPE_GP | 9},  \
    {"$a6",	RTYPE_GP | 10}, \
    {"$a7",	RTYPE_GP | 11}, \
    {"$ta0",	RTYPE_GP | 8},  /* alias for $a4 */ \
    {"$ta1",	RTYPE_GP | 9},  /* alias for $a5 */ \
    {"$ta2",	RTYPE_GP | 10}, /* alias for $a6 */ \
    {"$ta3",	RTYPE_GP | 11}, /* alias for $a7 */ \
    {"$t0",	RTYPE_GP | 12}, \
    {"$t1",	RTYPE_GP | 13}, \
    {"$t2",	RTYPE_GP | 14}, \
    {"$t3",	RTYPE_GP | 15}

#define O32_SYMBOLIC_REGISTER_NAMES \
    {"$t0",	RTYPE_GP | 8},  \
    {"$t1",	RTYPE_GP | 9},  \
    {"$t2",	RTYPE_GP | 10}, \
    {"$t3",	RTYPE_GP | 11}, \
    {"$t4",	RTYPE_GP | 12}, \
    {"$t5",	RTYPE_GP | 13}, \
    {"$t6",	RTYPE_GP | 14}, \
    {"$t7",	RTYPE_GP | 15}, \
    {"$ta0",	RTYPE_GP | 12}, /* alias for $t4 */ \
    {"$ta1",	RTYPE_GP | 13}, /* alias for $t5 */ \
    {"$ta2",	RTYPE_GP | 14}, /* alias for $t6 */ \
    {"$ta3",	RTYPE_GP | 15}  /* alias for $t7 */

/* Remaining symbolic register names.  */
#define SYMBOLIC_REGISTER_NAMES \
    {"$zero",	RTYPE_GP | 0},  \
    {"$at",	RTYPE_GP | 1},  \
    {"$AT",	RTYPE_GP | 1},  \
    {"$v0",	RTYPE_GP | 2},  \
    {"$v1",	RTYPE_GP | 3},  \
    {"$a0",	RTYPE_GP | 4},  \
    {"$a1",	RTYPE_GP | 5},  \
    {"$a2",	RTYPE_GP | 6},  \
    {"$a3",	RTYPE_GP | 7},  \
    {"$s0",	RTYPE_GP | 16}, \
    {"$s1",	RTYPE_GP | 17}, \
    {"$s2",	RTYPE_GP | 18}, \
    {"$s3",	RTYPE_GP | 19}, \
    {"$s4",	RTYPE_GP | 20}, \
    {"$s5",	RTYPE_GP | 21}, \
    {"$s6",	RTYPE_GP | 22}, \
    {"$s7",	RTYPE_GP | 23}, \
    {"$t8",	RTYPE_GP | 24}, \
    {"$t9",	RTYPE_GP | 25}, \
    {"$k0",	RTYPE_GP | 26}, \
    {"$kt0",	RTYPE_GP | 26}, \
    {"$k1",	RTYPE_GP | 27}, \
    {"$kt1",	RTYPE_GP | 27}, \
    {"$gp",	RTYPE_GP | 28}, \
    {"$sp",	RTYPE_GP | 29}, \
    {"$s8",	RTYPE_GP | 30}, \
    {"$fp",	RTYPE_GP | 30}, \
    {"$ra",	RTYPE_GP | 31}

#define MIPS16_SPECIAL_REGISTER_NAMES \
    {"$pc",	RTYPE_PC | 0}

#define MDMX_VECTOR_REGISTER_NAMES \
    /* {"$v0",	RTYPE_VEC | 0},  Clash with REG 2 above.  */ \
    /* {"$v1",	RTYPE_VEC | 1},  Clash with REG 3 above.  */ \
    {"$v2",	RTYPE_VEC | 2},  \
    {"$v3",	RTYPE_VEC | 3},  \
    {"$v4",	RTYPE_VEC | 4},  \
    {"$v5",	RTYPE_VEC | 5},  \
    {"$v6",	RTYPE_VEC | 6},  \
    {"$v7",	RTYPE_VEC | 7},  \
    {"$v8",	RTYPE_VEC | 8},  \
    {"$v9",	RTYPE_VEC | 9},  \
    {"$v10",	RTYPE_VEC | 10}, \
    {"$v11",	RTYPE_VEC | 11}, \
    {"$v12",	RTYPE_VEC | 12}, \
    {"$v13",	RTYPE_VEC | 13}, \
    {"$v14",	RTYPE_VEC | 14}, \
    {"$v15",	RTYPE_VEC | 15}, \
    {"$v16",	RTYPE_VEC | 16}, \
    {"$v17",	RTYPE_VEC | 17}, \
    {"$v18",	RTYPE_VEC | 18}, \
    {"$v19",	RTYPE_VEC | 19}, \
    {"$v20",	RTYPE_VEC | 20}, \
    {"$v21",	RTYPE_VEC | 21}, \
    {"$v22",	RTYPE_VEC | 22}, \
    {"$v23",	RTYPE_VEC | 23}, \
    {"$v24",	RTYPE_VEC | 24}, \
    {"$v25",	RTYPE_VEC | 25}, \
    {"$v26",	RTYPE_VEC | 26}, \
    {"$v27",	RTYPE_VEC | 27}, \
    {"$v28",	RTYPE_VEC | 28}, \
    {"$v29",	RTYPE_VEC | 29}, \
    {"$v30",	RTYPE_VEC | 30}, \
    {"$v31",	RTYPE_VEC | 31}

#define R5900_I_NAMES \
    {"$I",	RTYPE_R5900_I | 0}

#define R5900_Q_NAMES \
    {"$Q",	RTYPE_R5900_Q | 0}

#define R5900_R_NAMES \
    {"$R",	RTYPE_R5900_R | 0}

#define R5900_ACC_NAMES \
    {"$ACC",	RTYPE_R5900_ACC | 0 }

#define MIPS_DSP_ACCUMULATOR_NAMES \
    {"$ac0",	RTYPE_ACC | 0}, \
    {"$ac1",	RTYPE_ACC | 1}, \
    {"$ac2",	RTYPE_ACC | 2}, \
    {"$ac3",	RTYPE_ACC | 3}

static const struct regname reg_names[] = {
  GENERIC_REGISTER_NUMBERS,
  FPU_REGISTER_NAMES,
  FPU_CONDITION_CODE_NAMES,
  COPROC_CONDITION_CODE_NAMES,

  /* The $txx registers depends on the abi,
     these will be added later into the symbol table from
     one of the tables below once mips_abi is set after
     parsing of arguments from the command line. */
  SYMBOLIC_REGISTER_NAMES,

  MIPS16_SPECIAL_REGISTER_NAMES,
  MDMX_VECTOR_REGISTER_NAMES,
  R5900_I_NAMES,
  R5900_Q_NAMES,
  R5900_R_NAMES,
  R5900_ACC_NAMES,
  MIPS_DSP_ACCUMULATOR_NAMES,
  {0, 0}
};

static const struct regname reg_names_o32[] = {
  O32_SYMBOLIC_REGISTER_NAMES,
  {0, 0}
};

static const struct regname reg_names_n32n64[] = {
  N32N64_SYMBOLIC_REGISTER_NAMES,
  {0, 0}
};

/* Register symbols $v0 and $v1 map to GPRs 2 and 3, but they can also be
   interpreted as vector registers 0 and 1.  If SYMVAL is the value of one
   of these register symbols, return the associated vector register,
   otherwise return SYMVAL itself.  */

static unsigned int
mips_prefer_vec_regno (unsigned int symval)
{
  if ((symval & -2) == (RTYPE_GP | 2))
    return RTYPE_VEC | (symval & 1);
  return symval;
}

/* Return true if string [S, E) is a valid register name, storing its
   symbol value in *SYMVAL_PTR if so.  */

static bool
mips_parse_register_1 (char *s, char *e, unsigned int *symval_ptr)
{
  char save_c;
  symbolS *symbol;

  /* Terminate name.  */
  save_c = *e;
  *e = '\0';

  /* Look up the name.  */
  symbol = symbol_find (s);
  *e = save_c;

  if (!symbol || S_GET_SEGMENT (symbol) != reg_section)
    return false;

  *symval_ptr = S_GET_VALUE (symbol);
  return true;
}

/* Return true if the string at *SPTR is a valid register name.  Allow it
   to have a VU0-style channel suffix of the form x?y?z?w? if CHANNELS_PTR
   is nonnull.

   When returning true, move *SPTR past the register, store the
   register's symbol value in *SYMVAL_PTR and the channel mask in
   *CHANNELS_PTR (if nonnull).  The symbol value includes the register
   number (RNUM_MASK) and register type (RTYPE_MASK).  The channel mask
   is a 4-bit value of the form XYZW and is 0 if no suffix was given.  */

static bool
mips_parse_register (char **sptr, unsigned int *symval_ptr,
		     unsigned int *channels_ptr)
{
  char *s, *e, *m;
  const char *q;
  unsigned int channels, symval, bit;

  /* Find end of name.  */
  s = e = *sptr;
  if (is_name_beginner (*e))
    ++e;
  while (is_part_of_name (*e))
    ++e;

  channels = 0;
  if (!mips_parse_register_1 (s, e, &symval))
    {
      if (!channels_ptr)
	return false;

      /* Eat characters from the end of the string that are valid
	 channel suffixes.  The preceding register must be $ACC or
	 end with a digit, so there is no ambiguity.  */
      bit = 1;
      m = e;
      for (q = "wzyx"; *q; q++, bit <<= 1)
	if (m > s && m[-1] == *q)
	  {
	    --m;
	    channels |= bit;
	  }

      if (channels == 0
	  || !mips_parse_register_1 (s, m, &symval)
	  || (symval & (RTYPE_VI | RTYPE_VF | RTYPE_R5900_ACC)) == 0)
	return false;
    }

  *sptr = e;
  *symval_ptr = symval;
  if (channels_ptr)
    *channels_ptr = channels;
  return true;
}

/* Check if SPTR points at a valid register specifier according to TYPES.
   If so, then return 1, advance S to consume the specifier and store
   the register's number in REGNOP, otherwise return 0.  */

static int
reg_lookup (char **s, unsigned int types, unsigned int *regnop)
{
  unsigned int regno;

  if (mips_parse_register (s, &regno, NULL))
    {
      if (types & RTYPE_VEC)
	regno = mips_prefer_vec_regno (regno);
      if (regno & types)
	regno &= RNUM_MASK;
      else
	regno = ~0;
    }
  else
    {
      if (types & RWARN)
	as_warn (_("unrecognized register name `%s'"), *s);
      regno = ~0;
    }
  if (regnop)
    *regnop = regno;
  return regno <= RNUM_MASK;
}

/* Parse a VU0 "x?y?z?w?" channel mask at S and store the associated
   mask in *CHANNELS.  Return a pointer to the first unconsumed character.  */

static char *
mips_parse_vu0_channels (char *s, unsigned int *channels)
{
  unsigned int i;

  *channels = 0;
  for (i = 0; i < 4; i++)
    if (*s == "xyzw"[i])
      {
	*channels |= 1 << (3 - i);
	++s;
      }
  return s;
}

/* Token types for parsed operand lists.  */
enum mips_operand_token_type {
  /* A plain register, e.g. $f2.  */
  OT_REG,

  /* A 4-bit XYZW channel mask.  */
  OT_CHANNELS,

  /* A constant vector index, e.g. [1].  */
  OT_INTEGER_INDEX,

  /* A register vector index, e.g. [$2].  */
  OT_REG_INDEX,

  /* A continuous range of registers, e.g. $s0-$s4.  */
  OT_REG_RANGE,

  /* A (possibly relocated) expression.  */
  OT_INTEGER,

  /* A floating-point value.  */
  OT_FLOAT,

  /* A single character.  This can be '(', ')' or ',', but '(' only appears
     before OT_REGs.  */
  OT_CHAR,

  /* A doubled character, either "--" or "++".  */
  OT_DOUBLE_CHAR,

  /* The end of the operand list.  */
  OT_END
};

/* A parsed operand token.  */
struct mips_operand_token
{
  /* The type of token.  */
  enum mips_operand_token_type type;
  union
  {
    /* The register symbol value for an OT_REG or OT_REG_INDEX.  */
    unsigned int regno;

    /* The 4-bit channel mask for an OT_CHANNEL_SUFFIX.  */
    unsigned int channels;

    /* The integer value of an OT_INTEGER_INDEX.  */
    addressT index;

    /* The two register symbol values involved in an OT_REG_RANGE.  */
    struct {
      unsigned int regno1;
      unsigned int regno2;
    } reg_range;

    /* The value of an OT_INTEGER.  The value is represented as an
       expression and the relocation operators that were applied to
       that expression.  The reloc entries are BFD_RELOC_UNUSED if no
       relocation operators were used.  */
    struct {
      expressionS value;
      bfd_reloc_code_real_type relocs[3];
    } integer;

    /* The binary data for an OT_FLOAT constant, and the number of bytes
       in the constant.  */
    struct {
      unsigned char data[8];
      int length;
    } flt;

    /* The character represented by an OT_CHAR or OT_DOUBLE_CHAR.  */
    char ch;
  } u;
};

/* An obstack used to construct lists of mips_operand_tokens.  */
static struct obstack mips_operand_tokens;

/* Give TOKEN type TYPE and add it to mips_operand_tokens.  */

static void
mips_add_token (struct mips_operand_token *token,
		enum mips_operand_token_type type)
{
  token->type = type;
  obstack_grow (&mips_operand_tokens, token, sizeof (*token));
}

/* Check whether S is '(' followed by a register name.  Add OT_CHAR
   and OT_REG tokens for them if so, and return a pointer to the first
   unconsumed character.  Return null otherwise.  */

static char *
mips_parse_base_start (char *s)
{
  struct mips_operand_token token;
  unsigned int regno, channels;
  bool decrement_p;

  if (*s != '(')
    return 0;

  ++s;
  SKIP_SPACE_TABS (s);

  /* Only match "--" as part of a base expression.  In other contexts "--X"
     is a double negative.  */
  decrement_p = (s[0] == '-' && s[1] == '-');
  if (decrement_p)
    {
      s += 2;
      SKIP_SPACE_TABS (s);
    }

  /* Allow a channel specifier because that leads to better error messages
     than treating something like "$vf0x++" as an expression.  */
  if (!mips_parse_register (&s, &regno, &channels))
    return 0;

  token.u.ch = '(';
  mips_add_token (&token, OT_CHAR);

  if (decrement_p)
    {
      token.u.ch = '-';
      mips_add_token (&token, OT_DOUBLE_CHAR);
    }

  token.u.regno = regno;
  mips_add_token (&token, OT_REG);

  if (channels)
    {
      token.u.channels = channels;
      mips_add_token (&token, OT_CHANNELS);
    }

  /* For consistency, only match "++" as part of base expressions too.  */
  SKIP_SPACE_TABS (s);
  if (s[0] == '+' && s[1] == '+')
    {
      s += 2;
      token.u.ch = '+';
      mips_add_token (&token, OT_DOUBLE_CHAR);
    }

  return s;
}

/* Parse one or more tokens from S.  Return a pointer to the first
   unconsumed character on success.  Return null if an error was found
   and store the error text in insn_error.  FLOAT_FORMAT is as for
   mips_parse_arguments.  */

static char *
mips_parse_argument_token (char *s, char float_format)
{
  char *end, *save_in;
  const char *err;
  unsigned int regno1, regno2, channels;
  struct mips_operand_token token;

  /* First look for "($reg", since we want to treat that as an
     OT_CHAR and OT_REG rather than an expression.  */
  end = mips_parse_base_start (s);
  if (end)
    return end;

  /* Handle other characters that end up as OT_CHARs.  */
  if (*s == ')' || *s == ',')
    {
      token.u.ch = *s;
      mips_add_token (&token, OT_CHAR);
      ++s;
      return s;
    }

  /* Handle tokens that start with a register.  */
  if (mips_parse_register (&s, &regno1, &channels))
    {
      if (channels)
	{
	  /* A register and a VU0 channel suffix.  */
	  token.u.regno = regno1;
	  mips_add_token (&token, OT_REG);

	  token.u.channels = channels;
	  mips_add_token (&token, OT_CHANNELS);
	  return s;
	}

      SKIP_SPACE_TABS (s);
      if (*s == '-')
	{
	  /* A register range.  */
	  ++s;
	  SKIP_SPACE_TABS (s);
	  if (!mips_parse_register (&s, &regno2, NULL))
	    {
	      set_insn_error (0, _("invalid register range"));
	      return 0;
	    }

	  token.u.reg_range.regno1 = regno1;
	  token.u.reg_range.regno2 = regno2;
	  mips_add_token (&token, OT_REG_RANGE);
	  return s;
	}

      /* Add the register itself.  */
      token.u.regno = regno1;
      mips_add_token (&token, OT_REG);

      /* Check for a vector index.  */
      if (*s == '[')
	{
	  ++s;
	  SKIP_SPACE_TABS (s);
	  if (mips_parse_register (&s, &token.u.regno, NULL))
	    mips_add_token (&token, OT_REG_INDEX);
	  else
	    {
	      expressionS element;

	      my_getExpression (&element, s);
	      if (element.X_op != O_constant)
		{
		  set_insn_error (0, _("vector element must be constant"));
		  return 0;
		}
	      s = expr_parse_end;
	      token.u.index = element.X_add_number;
	      mips_add_token (&token, OT_INTEGER_INDEX);
	    }
	  SKIP_SPACE_TABS (s);
	  if (*s != ']')
	    {
	      set_insn_error (0, _("missing `]'"));
	      return 0;
	    }
	  ++s;
	}
      return s;
    }

  if (float_format)
    {
      /* First try to treat expressions as floats.  */
      save_in = input_line_pointer;
      input_line_pointer = s;
      err = md_atof (float_format, (char *) token.u.flt.data,
		     &token.u.flt.length);
      end = input_line_pointer;
      input_line_pointer = save_in;
      if (err && *err)
	{
	  set_insn_error (0, err);
	  return 0;
	}
      if (s != end)
	{
	  mips_add_token (&token, OT_FLOAT);
	  return end;
	}
    }

  /* Treat everything else as an integer expression.  */
  token.u.integer.relocs[0] = BFD_RELOC_UNUSED;
  token.u.integer.relocs[1] = BFD_RELOC_UNUSED;
  token.u.integer.relocs[2] = BFD_RELOC_UNUSED;
  my_getSmallExpression (&token.u.integer.value, token.u.integer.relocs, s);
  s = expr_parse_end;
  mips_add_token (&token, OT_INTEGER);
  return s;
}

/* S points to the operand list for an instruction.  FLOAT_FORMAT is 'f'
   if expressions should be treated as 32-bit floating-point constants,
   'd' if they should be treated as 64-bit floating-point constants,
   or 0 if they should be treated as integer expressions (the usual case).

   Return a list of tokens on success, otherwise return 0.  The caller
   must obstack_free the list after use.  */

static struct mips_operand_token *
mips_parse_arguments (char *s, char float_format)
{
  struct mips_operand_token token;

  SKIP_SPACE_TABS (s);
  while (*s)
    {
      s = mips_parse_argument_token (s, float_format);
      if (!s)
	{
	  obstack_free (&mips_operand_tokens,
			obstack_finish (&mips_operand_tokens));
	  return 0;
	}
      SKIP_SPACE_TABS (s);
    }
  mips_add_token (&token, OT_END);
  return (struct mips_operand_token *) obstack_finish (&mips_operand_tokens);
}

/* Return TRUE if opcode MO is valid on the currently selected ISA, ASE
   and architecture.  Use is_opcode_valid_16 for MIPS16 opcodes.  */

static bool
is_opcode_valid (const struct mips_opcode *mo)
{
  int isa = mips_opts.isa;
  int ase = mips_opts.ase;
  int fp_s, fp_d;
  unsigned int i;

  if (ISA_HAS_64BIT_REGS (isa))
    for (i = 0; i < ARRAY_SIZE (mips_ases); i++)
      if ((ase & mips_ases[i].flags) == mips_ases[i].flags)
	ase |= mips_ases[i].flags64;

  if (!opcode_is_member (mo, isa, ase, mips_opts.arch))
    return false;

  /* Check whether the instruction or macro requires single-precision or
     double-precision floating-point support.  Note that this information is
     stored differently in the opcode table for insns and macros.  */
  if (mo->pinfo == INSN_MACRO)
    {
      fp_s = mo->pinfo2 & INSN2_M_FP_S;
      fp_d = mo->pinfo2 & INSN2_M_FP_D;
    }
  else
    {
      fp_s = mo->pinfo & FP_S;
      fp_d = mo->pinfo & FP_D;
    }

  if (fp_d && (mips_opts.soft_float || mips_opts.single_float))
    return false;

  if (fp_s && mips_opts.soft_float)
    return false;

  return true;
}

/* Return TRUE if the MIPS16 opcode MO is valid on the currently
   selected ISA and architecture.  */

static bool
is_opcode_valid_16 (const struct mips_opcode *mo)
{
  int isa = mips_opts.isa;
  int ase = mips_opts.ase;
  unsigned int i;

  if (ISA_HAS_64BIT_REGS (isa))
    for (i = 0; i < ARRAY_SIZE (mips_ases); i++)
      if ((ase & mips_ases[i].flags) == mips_ases[i].flags)
	ase |= mips_ases[i].flags64;

  return opcode_is_member (mo, isa, ase, mips_opts.arch);
}

/* Return TRUE if the size of the microMIPS opcode MO matches one
   explicitly requested.  Always TRUE in the standard MIPS mode.
   Use is_size_valid_16 for MIPS16 opcodes.  */

static bool
is_size_valid (const struct mips_opcode *mo)
{
  if (!mips_opts.micromips)
    return true;

  if (mips_opts.insn32)
    {
      if (mo->pinfo != INSN_MACRO && micromips_insn_length (mo) != 4)
	return false;
      if ((mo->pinfo2 & INSN2_BRANCH_DELAY_16BIT) != 0)
	return false;
    }
  if (!forced_insn_length)
    return true;
  if (mo->pinfo == INSN_MACRO)
    return false;
  return forced_insn_length == micromips_insn_length (mo);
}

/* Return TRUE if the size of the MIPS16 opcode MO matches one
   explicitly requested.  */

static bool
is_size_valid_16 (const struct mips_opcode *mo)
{
  if (!forced_insn_length)
    return true;
  if (mo->pinfo == INSN_MACRO)
    return false;
  if (forced_insn_length == 2 && mips_opcode_32bit_p (mo))
    return false;
  if (forced_insn_length == 4 && (mo->pinfo2 & INSN2_SHORT_ONLY))
    return false;
  return true;
}

/* Return TRUE if the microMIPS opcode MO is valid for the delay slot
   of the preceding instruction.  Always TRUE in the standard MIPS mode.

   We don't accept macros in 16-bit delay slots to avoid a case where
   a macro expansion fails because it relies on a preceding 32-bit real
   instruction to have matched and does not handle the operands correctly.
   The only macros that may expand to 16-bit instructions are JAL that
   cannot be placed in a delay slot anyway, and corner cases of BALIGN
   and BGT (that likewise cannot be placed in a delay slot) that decay to
   a NOP.  In all these cases the macros precede any corresponding real
   instruction definitions in the opcode table, so they will match in the
   second pass where the size of the delay slot is ignored and therefore
   produce correct code.  */

static bool
is_delay_slot_valid (const struct mips_opcode *mo)
{
  if (!mips_opts.micromips)
    return true;

  if (mo->pinfo == INSN_MACRO)
    return (history[0].insn_mo->pinfo2 & INSN2_BRANCH_DELAY_16BIT) == 0;
  if ((history[0].insn_mo->pinfo2 & INSN2_BRANCH_DELAY_32BIT) != 0
      && micromips_insn_length (mo) != 4)
    return false;
  if ((history[0].insn_mo->pinfo2 & INSN2_BRANCH_DELAY_16BIT) != 0
      && micromips_insn_length (mo) != 2)
    return false;

  return true;
}

/* For consistency checking, verify that all bits of OPCODE are specified
   either by the match/mask part of the instruction definition, or by the
   operand list.  Also build up a list of operands in OPERANDS.

   INSN_BITS says which bits of the instruction are significant.
   If OPCODE is a standard or microMIPS instruction, DECODE_OPERAND
   provides the mips_operand description of each operand.  DECODE_OPERAND
   is null for MIPS16 instructions.  */

static int
validate_mips_insn (const struct mips_opcode *opcode,
		    unsigned long insn_bits,
		    const struct mips_operand *(*decode_operand) (const char *),
		    struct mips_operand_array *operands)
{
  const char *s;
  unsigned long used_bits, doubled, undefined, opno, mask;
  const struct mips_operand *operand;

  mask = (opcode->pinfo == INSN_MACRO ? 0 : opcode->mask);
  if ((mask & opcode->match) != opcode->match)
    {
      as_bad (_("internal: bad mips opcode (mask error): %s %s"),
	      opcode->name, opcode->args);
      return 0;
    }
  used_bits = 0;
  opno = 0;
  if (opcode->pinfo2 & INSN2_VU0_CHANNEL_SUFFIX)
    used_bits = mips_insert_operand (&mips_vu0_channel_mask, used_bits, -1);
  for (s = opcode->args; *s; ++s)
    switch (*s)
      {
      case ',':
      case '(':
      case ')':
	break;

      case '#':
	s++;
	break;

      default:
	if (!decode_operand)
	  operand = decode_mips16_operand (*s, mips_opcode_32bit_p (opcode));
	else
	  operand = decode_operand (s);
	if (!operand && opcode->pinfo != INSN_MACRO)
	  {
	    as_bad (_("internal: unknown operand type: %s %s"),
		    opcode->name, opcode->args);
	    return 0;
	  }
	gas_assert (opno < MAX_OPERANDS);
	operands->operand[opno] = operand;
	if (!decode_operand && operand
	    && operand->type == OP_INT && operand->lsb == 0
	    && mips_opcode_32bit_p (opcode))
	  used_bits |= mips16_immed_extend (-1, operand->size);
	else if (operand && operand->type != OP_VU0_MATCH_SUFFIX)
	  {
	    used_bits = mips_insert_operand (operand, used_bits, -1);
	    if (operand->type == OP_MDMX_IMM_REG)
	      /* Bit 5 is the format selector (OB vs QH).  The opcode table
		 has separate entries for each format.  */
	      used_bits &= ~(1 << (operand->lsb + 5));
	    if (operand->type == OP_ENTRY_EXIT_LIST)
	      used_bits &= ~(mask & 0x700);
	    /* interAptiv MR2 SAVE/RESTORE instructions have a discontiguous
	       operand field that cannot be fully described with LSB/SIZE.  */
	    if (operand->type == OP_SAVE_RESTORE_LIST && operand->lsb == 6)
	      used_bits &= ~0x6000;
	  }
	/* Skip prefix characters.  */
	if (decode_operand && (*s == '+' || *s == 'm' || *s == '-'))
	  ++s;
	opno += 1;
	break;
      }
  doubled = used_bits & mask & insn_bits;
  if (doubled)
    {
      as_bad (_("internal: bad mips opcode (bits 0x%08lx doubly defined):"
		" %s %s"), doubled, opcode->name, opcode->args);
      return 0;
    }
  used_bits |= mask;
  undefined = ~used_bits & insn_bits;
  if (opcode->pinfo != INSN_MACRO && undefined)
    {
      as_bad (_("internal: bad mips opcode (bits 0x%08lx undefined): %s %s"),
	      undefined, opcode->name, opcode->args);
      return 0;
    }
  used_bits &= ~insn_bits;
  if (used_bits)
    {
      as_bad (_("internal: bad mips opcode (bits 0x%08lx defined): %s %s"),
	      used_bits, opcode->name, opcode->args);
      return 0;
    }
  return 1;
}

/* The MIPS16 version of validate_mips_insn.  */

static int
validate_mips16_insn (const struct mips_opcode *opcode,
		      struct mips_operand_array *operands)
{
  unsigned long insn_bits = mips_opcode_32bit_p (opcode) ? 0xffffffff : 0xffff;

  return validate_mips_insn (opcode, insn_bits, 0, operands);
}

/* The microMIPS version of validate_mips_insn.  */

static int
validate_micromips_insn (const struct mips_opcode *opc,
			 struct mips_operand_array *operands)
{
  unsigned long insn_bits;
  unsigned long major;
  unsigned int length;

  if (opc->pinfo == INSN_MACRO)
    return validate_mips_insn (opc, 0xffffffff, decode_micromips_operand,
			       operands);

  length = micromips_insn_length (opc);
  if (length != 2 && length != 4)
    {
      as_bad (_("internal error: bad microMIPS opcode (incorrect length: %u): "
		"%s %s"), length, opc->name, opc->args);
      return 0;
    }
  major = opc->match >> (10 + 8 * (length - 2));
  if ((length == 2 && (major & 7) != 1 && (major & 6) != 2)
      || (length == 4 && (major & 7) != 0 && (major & 4) != 4))
    {
      as_bad (_("internal error: bad microMIPS opcode "
		"(opcode/length mismatch): %s %s"), opc->name, opc->args);
      return 0;
    }

  /* Shift piecewise to avoid an overflow where unsigned long is 32-bit.  */
  insn_bits = 1 << 4 * length;
  insn_bits <<= 4 * length;
  insn_bits -= 1;
  return validate_mips_insn (opc, insn_bits, decode_micromips_operand,
			     operands);
}

/* This function is called once, at assembler startup time.  It should set up
   all the tables, etc. that the MD part of the assembler will need.  */

void
md_begin (void)
{
  int i = 0;
  int broken = 0;

  if (mips_pic != NO_PIC)
    {
      if (g_switch_seen && g_switch_value != 0)
	as_bad (_("-G may not be used in position-independent code"));
      g_switch_value = 0;
    }
  else if (mips_abicalls)
    {
      if (g_switch_seen && g_switch_value != 0)
	as_bad (_("-G may not be used with abicalls"));
      g_switch_value = 0;
    }

  if (! bfd_set_arch_mach (stdoutput, bfd_arch_mips, file_mips_opts.arch))
    as_warn (_("could not set architecture and machine"));

  op_hash = str_htab_create ();

  mips_operands = XCNEWVEC (struct mips_operand_array, NUMOPCODES);
  for (i = 0; i < NUMOPCODES;)
    {
      const char *name = mips_opcodes[i].name;

      if (str_hash_insert (op_hash, name, &mips_opcodes[i], 0) != NULL)
	as_fatal (_("duplicate %s"), name);
      do
	{
	  if (!validate_mips_insn (&mips_opcodes[i], 0xffffffff,
				   decode_mips_operand, &mips_operands[i]))
	    broken = 1;

	  if (nop_insn.insn_mo == NULL && strcmp (name, "nop") == 0)
	    {
	      create_insn (&nop_insn, mips_opcodes + i);
	      if (mips_fix_loongson2f_nop)
		nop_insn.insn_opcode = LOONGSON2F_NOP_INSN;
	      nop_insn.fixed_p = 1;
	    }

          if (sync_insn.insn_mo == NULL && strcmp (name, "sync") == 0)
	    create_insn (&sync_insn, mips_opcodes + i);

	  ++i;
	}
      while ((i < NUMOPCODES) && !strcmp (mips_opcodes[i].name, name));
    }

  mips16_op_hash = str_htab_create ();
  mips16_operands = XCNEWVEC (struct mips_operand_array,
			      bfd_mips16_num_opcodes);

  i = 0;
  while (i < bfd_mips16_num_opcodes)
    {
      const char *name = mips16_opcodes[i].name;

      if (str_hash_insert (mips16_op_hash, name, &mips16_opcodes[i], 0))
	as_fatal (_("duplicate %s"), name);
      do
	{
	  if (!validate_mips16_insn (&mips16_opcodes[i], &mips16_operands[i]))
	    broken = 1;
	  if (mips16_nop_insn.insn_mo == NULL && strcmp (name, "nop") == 0)
	    {
	      create_insn (&mips16_nop_insn, mips16_opcodes + i);
	      mips16_nop_insn.fixed_p = 1;
	    }
	  ++i;
	}
      while (i < bfd_mips16_num_opcodes
	     && strcmp (mips16_opcodes[i].name, name) == 0);
    }

  micromips_op_hash = str_htab_create ();
  micromips_operands = XCNEWVEC (struct mips_operand_array,
				 bfd_micromips_num_opcodes);

  i = 0;
  while (i < bfd_micromips_num_opcodes)
    {
      const char *name = micromips_opcodes[i].name;

      if (str_hash_insert (micromips_op_hash, name, &micromips_opcodes[i], 0))
	as_fatal (_("duplicate %s"), name);
      do
	{
	  struct mips_cl_insn *micromips_nop_insn;

	  if (!validate_micromips_insn (&micromips_opcodes[i],
					&micromips_operands[i]))
	    broken = 1;

	  if (micromips_opcodes[i].pinfo != INSN_MACRO)
	    {
	      if (micromips_insn_length (micromips_opcodes + i) == 2)
		micromips_nop_insn = &micromips_nop16_insn;
	      else if (micromips_insn_length (micromips_opcodes + i) == 4)
		micromips_nop_insn = &micromips_nop32_insn;
	      else
		continue;

	      if (micromips_nop_insn->insn_mo == NULL
		  && strcmp (name, "nop") == 0)
		{
		  create_insn (micromips_nop_insn, micromips_opcodes + i);
		  micromips_nop_insn->fixed_p = 1;
		}
	    }
	}
      while (++i < bfd_micromips_num_opcodes
	     && strcmp (micromips_opcodes[i].name, name) == 0);
    }

  if (broken)
    as_fatal (_("broken assembler, no assembly attempted"));

  /* We add all the general register names to the symbol table.  This
     helps us detect invalid uses of them.  */
  for (i = 0; reg_names[i].name; i++)
    symbol_table_insert (symbol_new (reg_names[i].name, reg_section,
				     &zero_address_frag,
				     reg_names[i].num));
  if (HAVE_NEWABI)
    for (i = 0; reg_names_n32n64[i].name; i++)
      symbol_table_insert (symbol_new (reg_names_n32n64[i].name, reg_section,
				       &zero_address_frag,
				       reg_names_n32n64[i].num));
  else
    for (i = 0; reg_names_o32[i].name; i++)
      symbol_table_insert (symbol_new (reg_names_o32[i].name, reg_section,
				       &zero_address_frag,
				       reg_names_o32[i].num));

  for (i = 0; i < 32; i++)
    {
      char regname[16];

      /* R5900 VU0 floating-point register.  */
      sprintf (regname, "$vf%d", i);
      symbol_table_insert (symbol_new (regname, reg_section,
				       &zero_address_frag, RTYPE_VF | i));

      /* R5900 VU0 integer register.  */
      sprintf (regname, "$vi%d", i);
      symbol_table_insert (symbol_new (regname, reg_section,
				       &zero_address_frag, RTYPE_VI | i));

      /* MSA register.  */
      sprintf (regname, "$w%d", i);
      symbol_table_insert (symbol_new (regname, reg_section,
				       &zero_address_frag, RTYPE_MSA | i));
    }

  obstack_init (&mips_operand_tokens);

  mips_no_prev_insn ();

  mips_gprmask = 0;
  mips_cprmask[0] = 0;
  mips_cprmask[1] = 0;
  mips_cprmask[2] = 0;
  mips_cprmask[3] = 0;

  /* set the default alignment for the text section (2**2) */
  record_alignment (text_section, 2);

  bfd_set_gp_size (stdoutput, g_switch_value);

  /* On a native system other than VxWorks, sections must be aligned
     to 16 byte boundaries.  When configured for an embedded ELF
     target, we don't bother.  */
  if (!startswith (TARGET_OS, "elf")
      && !startswith (TARGET_OS, "vxworks"))
    {
      bfd_set_section_alignment (text_section, 4);
      bfd_set_section_alignment (data_section, 4);
      bfd_set_section_alignment (bss_section, 4);
    }

  /* Create a .reginfo section for register masks and a .mdebug
     section for debugging information.  */
  {
    segT seg;
    subsegT subseg;
    flagword flags;
    segT sec;

    seg = now_seg;
    subseg = now_subseg;

    /* The ABI says this section should be loaded so that the
       running program can access it.  However, we don't load it
       if we are configured for an embedded target.  */
    flags = SEC_READONLY | SEC_DATA;
    if (!startswith (TARGET_OS, "elf"))
      flags |= SEC_ALLOC | SEC_LOAD;

    if (mips_abi != N64_ABI)
      {
	sec = subseg_new (".reginfo", (subsegT) 0);

	bfd_set_section_flags (sec, flags);
	bfd_set_section_alignment (sec, HAVE_NEWABI ? 3 : 2);

	mips_regmask_frag = frag_more (sizeof (Elf32_External_RegInfo));
      }
    else
      {
	/* The 64-bit ABI uses a .MIPS.options section rather than
	   .reginfo section.  */
	sec = subseg_new (".MIPS.options", (subsegT) 0);
	bfd_set_section_flags (sec, flags);
	bfd_set_section_alignment (sec, 3);

	/* Set up the option header.  */
	{
	  Elf_Internal_Options opthdr;
	  char *f;

	  opthdr.kind = ODK_REGINFO;
	  opthdr.size = (sizeof (Elf_External_Options)
			 + sizeof (Elf64_External_RegInfo));
	  opthdr.section = 0;
	  opthdr.info = 0;
	  f = frag_more (sizeof (Elf_External_Options));
	  bfd_mips_elf_swap_options_out (stdoutput, &opthdr,
					 (Elf_External_Options *) f);

	  mips_regmask_frag = frag_more (sizeof (Elf64_External_RegInfo));
	}
      }

    sec = subseg_new (".MIPS.abiflags", (subsegT) 0);
    bfd_set_section_flags (sec,
			   SEC_READONLY | SEC_DATA | SEC_ALLOC | SEC_LOAD);
    bfd_set_section_alignment (sec, 3);
    mips_flags_frag = frag_more (sizeof (Elf_External_ABIFlags_v0));

    if (ECOFF_DEBUGGING)
      {
	sec = subseg_new (".mdebug", (subsegT) 0);
	bfd_set_section_flags (sec, SEC_HAS_CONTENTS | SEC_READONLY);
	bfd_set_section_alignment (sec, 2);
      }
    else if (mips_flag_pdr)
      {
	pdr_seg = subseg_new (".pdr", (subsegT) 0);
	bfd_set_section_flags (pdr_seg,
			       SEC_READONLY | SEC_RELOC | SEC_DEBUGGING);
	bfd_set_section_alignment (pdr_seg, 2);
      }

    subseg_set (seg, subseg);
  }

  if (mips_fix_vr4120)
    init_vr4120_conflicts ();
}

static inline void
fpabi_incompatible_with (int fpabi, const char *what)
{
  as_warn (_(".gnu_attribute %d,%d is incompatible with `%s'"),
	   Tag_GNU_MIPS_ABI_FP, fpabi, what);
}

static inline void
fpabi_requires (int fpabi, const char *what)
{
  as_warn (_(".gnu_attribute %d,%d requires `%s'"),
	   Tag_GNU_MIPS_ABI_FP, fpabi, what);
}

/* Check -mabi and register sizes against the specified FP ABI.  */
static void
check_fpabi (int fpabi)
{
  switch (fpabi)
    {
    case Val_GNU_MIPS_ABI_FP_DOUBLE:
      if (file_mips_opts.soft_float)
	fpabi_incompatible_with (fpabi, "softfloat");
      else if (file_mips_opts.single_float)
	fpabi_incompatible_with (fpabi, "singlefloat");
      if (file_mips_opts.gp == 64 && file_mips_opts.fp == 32)
	fpabi_incompatible_with (fpabi, "gp=64 fp=32");
      else if (file_mips_opts.gp == 32 && file_mips_opts.fp == 64)
	fpabi_incompatible_with (fpabi, "gp=32 fp=64");
      break;

    case Val_GNU_MIPS_ABI_FP_XX:
      if (mips_abi != O32_ABI)
	fpabi_requires (fpabi, "-mabi=32");
      else if (file_mips_opts.soft_float)
	fpabi_incompatible_with (fpabi, "softfloat");
      else if (file_mips_opts.single_float)
	fpabi_incompatible_with (fpabi, "singlefloat");
      else if (file_mips_opts.fp != 0)
	fpabi_requires (fpabi, "fp=xx");
      break;

    case Val_GNU_MIPS_ABI_FP_64A:
    case Val_GNU_MIPS_ABI_FP_64:
      if (mips_abi != O32_ABI)
	fpabi_requires (fpabi, "-mabi=32");
      else if (file_mips_opts.soft_float)
	fpabi_incompatible_with (fpabi, "softfloat");
      else if (file_mips_opts.single_float)
	fpabi_incompatible_with (fpabi, "singlefloat");
      else if (file_mips_opts.fp != 64)
	fpabi_requires (fpabi, "fp=64");
      else if (fpabi == Val_GNU_MIPS_ABI_FP_64 && !file_mips_opts.oddspreg)
	fpabi_incompatible_with (fpabi, "nooddspreg");
      else if (fpabi == Val_GNU_MIPS_ABI_FP_64A && file_mips_opts.oddspreg)
	fpabi_requires (fpabi, "nooddspreg");
      break;

    case Val_GNU_MIPS_ABI_FP_SINGLE:
      if (file_mips_opts.soft_float)
	fpabi_incompatible_with (fpabi, "softfloat");
      else if (!file_mips_opts.single_float)
	fpabi_requires (fpabi, "singlefloat");
      break;

    case Val_GNU_MIPS_ABI_FP_SOFT:
      if (!file_mips_opts.soft_float)
	fpabi_requires (fpabi, "softfloat");
      break;

    case Val_GNU_MIPS_ABI_FP_OLD_64:
      as_warn (_(".gnu_attribute %d,%d is no longer supported"),
	       Tag_GNU_MIPS_ABI_FP, fpabi);
      break;

    case Val_GNU_MIPS_ABI_FP_NAN2008:
      /* Silently ignore compatibility value.  */
      break;

    default:
      as_warn (_(".gnu_attribute %d,%d is not a recognized"
	         " floating-point ABI"), Tag_GNU_MIPS_ABI_FP, fpabi);
      break;
    }
}

/* Perform consistency checks on the current options.  */

static void
mips_check_options (struct mips_set_options *opts, bool abi_checks)
{
  /* Check the size of integer registers agrees with the ABI and ISA.  */
  if (opts->gp == 64 && !ISA_HAS_64BIT_REGS (opts->isa))
    as_bad (_("`gp=64' used with a 32-bit processor"));
  else if (abi_checks
	   && opts->gp == 32 && ABI_NEEDS_64BIT_REGS (mips_abi))
    as_bad (_("`gp=32' used with a 64-bit ABI"));
  else if (abi_checks
	   && opts->gp == 64 && ABI_NEEDS_32BIT_REGS (mips_abi))
    as_bad (_("`gp=64' used with a 32-bit ABI"));

  /* Check the size of the float registers agrees with the ABI and ISA.  */
  switch (opts->fp)
    {
    case 0:
      if (!CPU_HAS_LDC1_SDC1 (opts->arch))
	as_bad (_("`fp=xx' used with a cpu lacking ldc1/sdc1 instructions"));
      else if (opts->single_float == 1)
	as_bad (_("`fp=xx' cannot be used with `singlefloat'"));
      break;
    case 64:
      if (!ISA_HAS_64BIT_FPRS (opts->isa))
	as_bad (_("`fp=64' used with a 32-bit fpu"));
      else if (abi_checks
	       && ABI_NEEDS_32BIT_REGS (mips_abi)
	       && !ISA_HAS_MXHC1 (opts->isa))
	as_warn (_("`fp=64' used with a 32-bit ABI"));
      break;
    case 32:
      if (abi_checks
	  && ABI_NEEDS_64BIT_REGS (mips_abi))
	as_warn (_("`fp=32' used with a 64-bit ABI"));
      if (ISA_IS_R6 (opts->isa) && opts->single_float == 0)
	as_bad (_("`fp=32' used with a MIPS R6 cpu"));
      break;
    default:
      as_bad (_("Unknown size of floating point registers"));
      break;
    }

  if (ABI_NEEDS_64BIT_REGS (mips_abi) && !opts->oddspreg)
    as_bad (_("`nooddspreg` cannot be used with a 64-bit ABI"));

  if (opts->micromips == 1 && opts->mips16 == 1)
    as_bad (_("`%s' cannot be used with `%s'"), "mips16", "micromips");
  else if (ISA_IS_R6 (opts->isa)
	   && (opts->micromips == 1
	       || opts->mips16 == 1))
    as_fatal (_("`%s' cannot be used with `%s'"),
	      opts->micromips ? "micromips" : "mips16",
	      mips_cpu_info_from_isa (opts->isa)->name);

  if (ISA_IS_R6 (opts->isa) && mips_relax_branch)
    as_fatal (_("branch relaxation is not supported in `%s'"),
	      mips_cpu_info_from_isa (opts->isa)->name);
}

/* Perform consistency checks on the module level options exactly once.
   This is a deferred check that happens:
     at the first .set directive
     or, at the first pseudo op that generates code (inc .dc.a)
     or, at the first instruction
     or, at the end.  */

static void
file_mips_check_options (void)
{
  if (file_mips_opts_checked)
    return;

  /* The following code determines the register size.
     Similar code was added to GCC 3.3 (see override_options() in
     config/mips/mips.c).  The GAS and GCC code should be kept in sync
     as much as possible.  */

  if (file_mips_opts.gp < 0)
    {
      /* Infer the integer register size from the ABI and processor.
	 Restrict ourselves to 32-bit registers if that's all the
	 processor has, or if the ABI cannot handle 64-bit registers.  */
      file_mips_opts.gp = (ABI_NEEDS_32BIT_REGS (mips_abi)
			   || !ISA_HAS_64BIT_REGS (file_mips_opts.isa))
			  ? 32 : 64;
    }

  if (file_mips_opts.fp < 0)
    {
      /* No user specified float register size.
	 ??? GAS treats single-float processors as though they had 64-bit
	 float registers (although it complains when double-precision
	 instructions are used).  As things stand, saying they have 32-bit
	 registers would lead to spurious "register must be even" messages.
	 So here we assume float registers are never smaller than the
	 integer ones.  */
      if (file_mips_opts.gp == 64)
	/* 64-bit integer registers implies 64-bit float registers.  */
	file_mips_opts.fp = 64;
      else if ((file_mips_opts.ase & FP64_ASES)
	       && ISA_HAS_64BIT_FPRS (file_mips_opts.isa))
	/* Handle ASEs that require 64-bit float registers, if possible.  */
	file_mips_opts.fp = 64;
      else if (ISA_IS_R6 (mips_opts.isa))
	/* R6 implies 64-bit float registers.  */
	file_mips_opts.fp = 64;
      else
	/* 32-bit float registers.  */
	file_mips_opts.fp = 32;
    }

  /* Disable operations on odd-numbered floating-point registers by default
     when using the FPXX ABI.  */
  if (file_mips_opts.oddspreg < 0)
    {
      if (file_mips_opts.fp == 0)
	file_mips_opts.oddspreg = 0;
      else
	file_mips_opts.oddspreg = 1;
    }

  /* End of GCC-shared inference code.  */

  /* This flag is set when we have a 64-bit capable CPU but use only
     32-bit wide registers.  Note that EABI does not use it.  */
  if (ISA_HAS_64BIT_REGS (file_mips_opts.isa)
      && ((mips_abi == NO_ABI && file_mips_opts.gp == 32)
	  || mips_abi == O32_ABI))
    mips_32bitmode = 1;

  if (file_mips_opts.isa == ISA_MIPS1 && mips_trap)
    as_bad (_("trap exception not supported at ISA 1"));

  /* If the selected architecture includes support for ASEs, enable
     generation of code for them.  */
  if (file_mips_opts.mips16 == -1)
    file_mips_opts.mips16 = (CPU_HAS_MIPS16 (file_mips_opts.arch)) ? 1 : 0;
  if (file_mips_opts.micromips == -1)
    file_mips_opts.micromips = (CPU_HAS_MICROMIPS (file_mips_opts.arch))
				? 1 : 0;

  if (mips_nan2008 == -1)
    mips_nan2008 = (ISA_HAS_LEGACY_NAN (file_mips_opts.isa)) ? 0 : 1;
  else if (!ISA_HAS_LEGACY_NAN (file_mips_opts.isa) && mips_nan2008 == 0)
    as_fatal (_("`%s' does not support legacy NaN"),
	      mips_cpu_info_from_arch (file_mips_opts.arch)->name);

  /* Some ASEs require 64-bit FPRs, so -mfp32 should stop those ASEs from
     being selected implicitly.  */
  if (file_mips_opts.fp != 64)
    file_ase_explicit |= ASE_MIPS3D | ASE_MDMX | ASE_MSA;

  /* If the user didn't explicitly select or deselect a particular ASE,
     use the default setting for the CPU.  */
  file_mips_opts.ase |= (file_mips_opts.init_ase & ~file_ase_explicit);

  /* Set up the current options.  These may change throughout assembly.  */
  mips_opts = file_mips_opts;

  mips_check_isa_supports_ases ();
  mips_check_options (&file_mips_opts, true);
  file_mips_opts_checked = true;

  if (!bfd_set_arch_mach (stdoutput, bfd_arch_mips, file_mips_opts.arch))
    as_warn (_("could not set architecture and machine"));
}

void
md_assemble (char *str)
{
  struct mips_cl_insn insn;
  bfd_reloc_code_real_type unused_reloc[3]
    = {BFD_RELOC_UNUSED, BFD_RELOC_UNUSED, BFD_RELOC_UNUSED};

  file_mips_check_options ();

  imm_expr.X_op = O_absent;
  offset_expr.X_op = O_absent;
  offset_reloc[0] = BFD_RELOC_UNUSED;
  offset_reloc[1] = BFD_RELOC_UNUSED;
  offset_reloc[2] = BFD_RELOC_UNUSED;

  mips_mark_labels ();
  mips_assembling_insn = true;
  clear_insn_error ();

  if (mips_opts.mips16)
    mips16_ip (str, &insn);
  else
    {
      mips_ip (str, &insn);
      DBG ((_("returned from mips_ip(%s) insn_opcode = 0x%x\n"),
	    str, insn.insn_opcode));
    }

  if (insn_error.msg)
    report_insn_error (str);
  else if (insn.insn_mo->pinfo == INSN_MACRO)
    {
      macro_start ();
      if (mips_opts.mips16)
	mips16_macro (&insn);
      else
	macro (&insn, str);
      macro_end ();
    }
  else
    {
      if (offset_expr.X_op != O_absent)
	append_insn (&insn, &offset_expr, offset_reloc, false);
      else
	append_insn (&insn, NULL, unused_reloc, false);
    }

  mips_assembling_insn = false;
}

/* Convenience functions for abstracting away the differences between
   MIPS16 and non-MIPS16 relocations.  */

static inline bool
mips16_reloc_p (bfd_reloc_code_real_type reloc)
{
  switch (reloc)
    {
    case BFD_RELOC_MIPS16_JMP:
    case BFD_RELOC_MIPS16_GPREL:
    case BFD_RELOC_MIPS16_GOT16:
    case BFD_RELOC_MIPS16_CALL16:
    case BFD_RELOC_MIPS16_HI16_S:
    case BFD_RELOC_MIPS16_HI16:
    case BFD_RELOC_MIPS16_LO16:
    case BFD_RELOC_MIPS16_16_PCREL_S1:
      return true;

    default:
      return false;
    }
}

static inline bool
micromips_reloc_p (bfd_reloc_code_real_type reloc)
{
  switch (reloc)
    {
    case BFD_RELOC_MICROMIPS_7_PCREL_S1:
    case BFD_RELOC_MICROMIPS_10_PCREL_S1:
    case BFD_RELOC_MICROMIPS_16_PCREL_S1:
    case BFD_RELOC_MICROMIPS_GPREL16:
    case BFD_RELOC_MICROMIPS_JMP:
    case BFD_RELOC_MICROMIPS_HI16:
    case BFD_RELOC_MICROMIPS_HI16_S:
    case BFD_RELOC_MICROMIPS_LO16:
    case BFD_RELOC_MICROMIPS_LITERAL:
    case BFD_RELOC_MICROMIPS_GOT16:
    case BFD_RELOC_MICROMIPS_CALL16:
    case BFD_RELOC_MICROMIPS_GOT_HI16:
    case BFD_RELOC_MICROMIPS_GOT_LO16:
    case BFD_RELOC_MICROMIPS_CALL_HI16:
    case BFD_RELOC_MICROMIPS_CALL_LO16:
    case BFD_RELOC_MICROMIPS_SUB:
    case BFD_RELOC_MICROMIPS_GOT_PAGE:
    case BFD_RELOC_MICROMIPS_GOT_OFST:
    case BFD_RELOC_MICROMIPS_GOT_DISP:
    case BFD_RELOC_MICROMIPS_HIGHEST:
    case BFD_RELOC_MICROMIPS_HIGHER:
    case BFD_RELOC_MICROMIPS_SCN_DISP:
    case BFD_RELOC_MICROMIPS_JALR:
      return true;

    default:
      return false;
    }
}

static inline bool
jmp_reloc_p (bfd_reloc_code_real_type reloc)
{
  return reloc == BFD_RELOC_MIPS_JMP || reloc == BFD_RELOC_MICROMIPS_JMP;
}

static inline bool
b_reloc_p (bfd_reloc_code_real_type reloc)
{
  return (reloc == BFD_RELOC_MIPS_26_PCREL_S2
	  || reloc == BFD_RELOC_MIPS_21_PCREL_S2
	  || reloc == BFD_RELOC_16_PCREL_S2
	  || reloc == BFD_RELOC_MIPS16_16_PCREL_S1
	  || reloc == BFD_RELOC_MICROMIPS_16_PCREL_S1
	  || reloc == BFD_RELOC_MICROMIPS_10_PCREL_S1
	  || reloc == BFD_RELOC_MICROMIPS_7_PCREL_S1);
}

static inline bool
got16_reloc_p (bfd_reloc_code_real_type reloc)
{
  return (reloc == BFD_RELOC_MIPS_GOT16 || reloc == BFD_RELOC_MIPS16_GOT16
	  || reloc == BFD_RELOC_MICROMIPS_GOT16);
}

static inline bool
hi16_reloc_p (bfd_reloc_code_real_type reloc)
{
  return (reloc == BFD_RELOC_HI16_S || reloc == BFD_RELOC_MIPS16_HI16_S
	  || reloc == BFD_RELOC_MICROMIPS_HI16_S);
}

static inline bool
lo16_reloc_p (bfd_reloc_code_real_type reloc)
{
  return (reloc == BFD_RELOC_LO16 || reloc == BFD_RELOC_MIPS16_LO16
	  || reloc == BFD_RELOC_MICROMIPS_LO16);
}

static inline bool
jalr_reloc_p (bfd_reloc_code_real_type reloc)
{
  return reloc == BFD_RELOC_MIPS_JALR || reloc == BFD_RELOC_MICROMIPS_JALR;
}

static inline bool
gprel16_reloc_p (bfd_reloc_code_real_type reloc)
{
  return (reloc == BFD_RELOC_GPREL16 || reloc == BFD_RELOC_MIPS16_GPREL
	  || reloc == BFD_RELOC_MICROMIPS_GPREL16);
}

/* Return true if RELOC is a PC-relative relocation that does not have
   full address range.  */

static inline bool
limited_pcrel_reloc_p (bfd_reloc_code_real_type reloc)
{
  switch (reloc)
    {
    case BFD_RELOC_16_PCREL_S2:
    case BFD_RELOC_MIPS16_16_PCREL_S1:
    case BFD_RELOC_MICROMIPS_7_PCREL_S1:
    case BFD_RELOC_MICROMIPS_10_PCREL_S1:
    case BFD_RELOC_MICROMIPS_16_PCREL_S1:
    case BFD_RELOC_MIPS_21_PCREL_S2:
    case BFD_RELOC_MIPS_26_PCREL_S2:
    case BFD_RELOC_MIPS_18_PCREL_S3:
    case BFD_RELOC_MIPS_19_PCREL_S2:
      return true;

    case BFD_RELOC_32_PCREL:
    case BFD_RELOC_HI16_S_PCREL:
    case BFD_RELOC_LO16_PCREL:
      return HAVE_64BIT_ADDRESSES;

    default:
      return false;
    }
}

/* Return true if the given relocation might need a matching %lo().
   This is only "might" because SVR4 R_MIPS_GOT16 relocations only
   need a matching %lo() when applied to local symbols.  */

static inline bool
reloc_needs_lo_p (bfd_reloc_code_real_type reloc)
{
  return (HAVE_IN_PLACE_ADDENDS
	  && (hi16_reloc_p (reloc)
	      /* VxWorks R_MIPS_GOT16 relocs never need a matching %lo();
		 all GOT16 relocations evaluate to "G".  */
	      || (got16_reloc_p (reloc) && mips_pic != VXWORKS_PIC)));
}

/* Return the type of %lo() reloc needed by RELOC, given that
   reloc_needs_lo_p.  */

static inline bfd_reloc_code_real_type
matching_lo_reloc (bfd_reloc_code_real_type reloc)
{
  return (mips16_reloc_p (reloc) ? BFD_RELOC_MIPS16_LO16
	  : (micromips_reloc_p (reloc) ? BFD_RELOC_MICROMIPS_LO16
	     : BFD_RELOC_LO16));
}

/* Return true if the given fixup is followed by a matching R_MIPS_LO16
   relocation.  */

static inline bool
fixup_has_matching_lo_p (fixS *fixp)
{
  return (fixp->fx_next != NULL
	  && fixp->fx_next->fx_r_type == matching_lo_reloc (fixp->fx_r_type)
	  && fixp->fx_addsy == fixp->fx_next->fx_addsy
	  && fixp->fx_offset == fixp->fx_next->fx_offset);
}

/* Move all labels in LABELS to the current insertion point.  TEXT_P
   says whether the labels refer to text or data.  */

static void
mips_move_labels (struct insn_label_list *labels, bool text_p)
{
  struct insn_label_list *l;
  valueT val;

  for (l = labels; l != NULL; l = l->next)
    {
      gas_assert (S_GET_SEGMENT (l->label) == now_seg);
      symbol_set_frag (l->label, frag_now);
      val = (valueT) frag_now_fix ();
      /* MIPS16/microMIPS text labels are stored as odd.
	 We just carry the ISA mode bit forward.  */
      if (text_p && HAVE_CODE_COMPRESSION)
	val |= (S_GET_VALUE (l->label) & 0x1);
      S_SET_VALUE (l->label, val);
    }
}

/* Move all labels in insn_labels to the current insertion point
   and treat them as text labels.  */

static void
mips_move_text_labels (void)
{
  mips_move_labels (seg_info (now_seg)->label_list, true);
}

/* Duplicate the test for LINK_ONCE sections as in `adjust_reloc_syms'.  */

static bool
s_is_linkonce (symbolS *sym, segT from_seg)
{
  bool linkonce = false;
  segT symseg = S_GET_SEGMENT (sym);

  if (symseg != from_seg && !S_IS_LOCAL (sym))
    {
      if ((bfd_section_flags (symseg) & SEC_LINK_ONCE))
	linkonce = true;
      /* The GNU toolchain uses an extension for ELF: a section
	 beginning with the magic string .gnu.linkonce is a
	 linkonce section.  */
      if (startswith (segment_name (symseg), ".gnu.linkonce"))
	linkonce = true;
    }
  return linkonce;
}

/* Mark MIPS16 or microMIPS instruction label LABEL.  This permits the
   linker to handle them specially, such as generating jalx instructions
   when needed.  We also make them odd for the duration of the assembly,
   in order to generate the right sort of code.  We will make them even
   in the adjust_symtab routine, while leaving them marked.  This is
   convenient for the debugger and the disassembler.  The linker knows
   to make them odd again.  */

static void
mips_compressed_mark_label (symbolS *label)
{
  gas_assert (HAVE_CODE_COMPRESSION);

  if (mips_opts.mips16)
    S_SET_OTHER (label, ELF_ST_SET_MIPS16 (S_GET_OTHER (label)));
  else
    S_SET_OTHER (label, ELF_ST_SET_MICROMIPS (S_GET_OTHER (label)));
  if ((S_GET_VALUE (label) & 1) == 0
      /* Don't adjust the address if the label is global or weak, or
	 in a link-once section, since we'll be emitting symbol reloc
	 references to it which will be patched up by the linker, and
	 the final value of the symbol may or may not be MIPS16/microMIPS.  */
      && !S_IS_WEAK (label)
      && !S_IS_EXTERNAL (label)
      && !s_is_linkonce (label, now_seg))
    S_SET_VALUE (label, S_GET_VALUE (label) | 1);
}

/* Mark preceding MIPS16 or microMIPS instruction labels.  */

static void
mips_compressed_mark_labels (void)
{
  struct insn_label_list *l;

  for (l = seg_info (now_seg)->label_list; l != NULL; l = l->next)
    mips_compressed_mark_label (l->label);
}

/* End the current frag.  Make it a variant frag and record the
   relaxation info.  */

static void
relax_close_frag (void)
{
  mips_macro_warning.first_frag = frag_now;
  frag_var (rs_machine_dependent, 0, 0,
	    RELAX_ENCODE (mips_relax.sizes[0], mips_relax.sizes[1],
			  mips_pic != NO_PIC),
	    mips_relax.symbol, 0, (char *) mips_relax.first_fixup);

  memset (&mips_relax.sizes, 0, sizeof (mips_relax.sizes));
  mips_relax.first_fixup = 0;
}

/* Start a new relaxation sequence whose expansion depends on SYMBOL.
   See the comment above RELAX_ENCODE for more details.  */

static void
relax_start (symbolS *symbol)
{
  gas_assert (mips_relax.sequence == 0);
  mips_relax.sequence = 1;
  mips_relax.symbol = symbol;
}

/* Start generating the second version of a relaxable sequence.
   See the comment above RELAX_ENCODE for more details.  */

static void
relax_switch (void)
{
  gas_assert (mips_relax.sequence == 1);
  mips_relax.sequence = 2;
}

/* End the current relaxable sequence.  */

static void
relax_end (void)
{
  gas_assert (mips_relax.sequence == 2);
  relax_close_frag ();
  mips_relax.sequence = 0;
}

/* Return true if IP is a delayed branch or jump.  */

static inline bool
delayed_branch_p (const struct mips_cl_insn *ip)
{
  return (ip->insn_mo->pinfo & (INSN_UNCOND_BRANCH_DELAY
				| INSN_COND_BRANCH_DELAY
				| INSN_COND_BRANCH_LIKELY)) != 0;
}

/* Return true if IP is a compact branch or jump.  */

static inline bool
compact_branch_p (const struct mips_cl_insn *ip)
{
  return (ip->insn_mo->pinfo2 & (INSN2_UNCOND_BRANCH
				 | INSN2_COND_BRANCH)) != 0;
}

/* Return true if IP is an unconditional branch or jump.  */

static inline bool
uncond_branch_p (const struct mips_cl_insn *ip)
{
  return ((ip->insn_mo->pinfo & INSN_UNCOND_BRANCH_DELAY) != 0
	  || (ip->insn_mo->pinfo2 & INSN2_UNCOND_BRANCH) != 0);
}

/* Return true if IP is a branch-likely instruction.  */

static inline bool
branch_likely_p (const struct mips_cl_insn *ip)
{
  return (ip->insn_mo->pinfo & INSN_COND_BRANCH_LIKELY) != 0;
}

/* Return the type of nop that should be used to fill the delay slot
   of delayed branch IP.  */

static struct mips_cl_insn *
get_delay_slot_nop (const struct mips_cl_insn *ip)
{
  if (mips_opts.micromips
      && (ip->insn_mo->pinfo2 & INSN2_BRANCH_DELAY_32BIT))
    return &micromips_nop32_insn;
  return NOP_INSN;
}

/* Return a mask that has bit N set if OPCODE reads the register(s)
   in operand N.  */

static unsigned int
insn_read_mask (const struct mips_opcode *opcode)
{
  return (opcode->pinfo & INSN_READ_ALL) >> INSN_READ_SHIFT;
}

/* Return a mask that has bit N set if OPCODE writes to the register(s)
   in operand N.  */

static unsigned int
insn_write_mask (const struct mips_opcode *opcode)
{
  return (opcode->pinfo & INSN_WRITE_ALL) >> INSN_WRITE_SHIFT;
}

/* Return a mask of the registers specified by operand OPERAND of INSN.
   Ignore registers of type OP_REG_<t> unless bit OP_REG_<t> of TYPE_MASK
   is set.  */

static unsigned int
operand_reg_mask (const struct mips_cl_insn *insn,
		  const struct mips_operand *operand,
		  unsigned int type_mask)
{
  unsigned int uval, vsel;

  switch (operand->type)
    {
    case OP_INT:
    case OP_MAPPED_INT:
    case OP_MSB:
    case OP_PCREL:
    case OP_PERF_REG:
    case OP_ADDIUSP_INT:
    case OP_ENTRY_EXIT_LIST:
    case OP_REPEAT_DEST_REG:
    case OP_REPEAT_PREV_REG:
    case OP_PC:
    case OP_VU0_SUFFIX:
    case OP_VU0_MATCH_SUFFIX:
    case OP_IMM_INDEX:
      abort ();

    case OP_REG28:
      return 1 << 28;

    case OP_REG:
    case OP_OPTIONAL_REG:
      {
	const struct mips_reg_operand *reg_op;

	reg_op = (const struct mips_reg_operand *) operand;
	if (!(type_mask & (1 << reg_op->reg_type)))
	  return 0;
	uval = insn_extract_operand (insn, operand);
	return 1u << mips_decode_reg_operand (reg_op, uval);
      }

    case OP_REG_PAIR:
      {
	const struct mips_reg_pair_operand *pair_op;

	pair_op = (const struct mips_reg_pair_operand *) operand;
	if (!(type_mask & (1 << pair_op->reg_type)))
	  return 0;
	uval = insn_extract_operand (insn, operand);
	return (1u << pair_op->reg1_map[uval]) | (1u << pair_op->reg2_map[uval]);
      }

    case OP_CLO_CLZ_DEST:
      if (!(type_mask & (1 << OP_REG_GP)))
	return 0;
      uval = insn_extract_operand (insn, operand);
      return (1u << (uval & 31)) | (1u << (uval >> 5));

    case OP_SAME_RS_RT:
      if (!(type_mask & (1 << OP_REG_GP)))
	return 0;
      uval = insn_extract_operand (insn, operand);
      gas_assert ((uval & 31) == (uval >> 5));
      return 1u << (uval & 31);

    case OP_CHECK_PREV:
    case OP_NON_ZERO_REG:
      if (!(type_mask & (1 << OP_REG_GP)))
	return 0;
      uval = insn_extract_operand (insn, operand);
      return 1u << (uval & 31);

    case OP_LWM_SWM_LIST:
      abort ();

    case OP_SAVE_RESTORE_LIST:
      abort ();

    case OP_MDMX_IMM_REG:
      if (!(type_mask & (1 << OP_REG_VEC)))
	return 0;
      uval = insn_extract_operand (insn, operand);
      vsel = uval >> 5;
      if ((vsel & 0x18) == 0x18)
	return 0;
      return 1u << (uval & 31);

    case OP_REG_INDEX:
      if (!(type_mask & (1 << OP_REG_GP)))
	return 0;
      return 1u << insn_extract_operand (insn, operand);
    }
  abort ();
}

/* Return a mask of the registers specified by operands OPNO_MASK of INSN,
   where bit N of OPNO_MASK is set if operand N should be included.
   Ignore registers of type OP_REG_<t> unless bit OP_REG_<t> of TYPE_MASK
   is set.  */

static unsigned int
insn_reg_mask (const struct mips_cl_insn *insn,
	       unsigned int type_mask, unsigned int opno_mask)
{
  unsigned int opno, reg_mask;

  opno = 0;
  reg_mask = 0;
  while (opno_mask != 0)
    {
      if (opno_mask & 1)
	reg_mask |= operand_reg_mask (insn, insn_opno (insn, opno), type_mask);
      opno_mask >>= 1;
      opno += 1;
    }
  return reg_mask;
}

/* Return the mask of core registers that IP reads.  */

static unsigned int
gpr_read_mask (const struct mips_cl_insn *ip)
{
  unsigned long pinfo, pinfo2;
  unsigned int mask;

  mask = insn_reg_mask (ip, 1 << OP_REG_GP, insn_read_mask (ip->insn_mo));
  pinfo = ip->insn_mo->pinfo;
  pinfo2 = ip->insn_mo->pinfo2;
  if (pinfo & INSN_UDI)
    {
      /* UDI instructions have traditionally been assumed to read RS
	 and RT.  */
      mask |= 1 << EXTRACT_OPERAND (mips_opts.micromips, RT, *ip);
      mask |= 1 << EXTRACT_OPERAND (mips_opts.micromips, RS, *ip);
    }
  if (pinfo & INSN_READ_GPR_24)
    mask |= 1 << 24;
  if (pinfo2 & INSN2_READ_GPR_16)
    mask |= 1 << 16;
  if (pinfo2 & INSN2_READ_SP)
    mask |= 1 << SP;
  if (pinfo2 & INSN2_READ_GPR_31)
    mask |= 1u << 31;
  /* Don't include register 0.  */
  return mask & ~1;
}

/* Return the mask of core registers that IP writes.  */

static unsigned int
gpr_write_mask (const struct mips_cl_insn *ip)
{
  unsigned long pinfo, pinfo2;
  unsigned int mask;

  mask = insn_reg_mask (ip, 1 << OP_REG_GP, insn_write_mask (ip->insn_mo));
  pinfo = ip->insn_mo->pinfo;
  pinfo2 = ip->insn_mo->pinfo2;
  if (pinfo & INSN_WRITE_GPR_24)
    mask |= 1 << 24;
  if (pinfo & INSN_WRITE_GPR_31)
    mask |= 1u << 31;
  if (pinfo & INSN_UDI)
    /* UDI instructions have traditionally been assumed to write to RD.  */
    mask |= 1 << EXTRACT_OPERAND (mips_opts.micromips, RD, *ip);
  if (pinfo2 & INSN2_WRITE_SP)
    mask |= 1 << SP;
  /* Don't include register 0.  */
  return mask & ~1;
}

/* Return the mask of floating-point registers that IP reads.  */

static unsigned int
fpr_read_mask (const struct mips_cl_insn *ip)
{
  unsigned long pinfo;
  unsigned int mask;

  mask = insn_reg_mask (ip, ((1 << OP_REG_FP) | (1 << OP_REG_VEC)
			     | (1 << OP_REG_MSA)),
			insn_read_mask (ip->insn_mo));
  pinfo = ip->insn_mo->pinfo;
  /* Conservatively treat all operands to an FP_D instruction are doubles.
     (This is overly pessimistic for things like cvt.d.s.)  */
  if (FPR_SIZE != 64 && (pinfo & FP_D))
    mask |= mask << 1;
  return mask;
}

/* Return the mask of floating-point registers that IP writes.  */

static unsigned int
fpr_write_mask (const struct mips_cl_insn *ip)
{
  unsigned long pinfo;
  unsigned int mask;

  mask = insn_reg_mask (ip, ((1 << OP_REG_FP) | (1 << OP_REG_VEC)
			     | (1 << OP_REG_MSA)),
			insn_write_mask (ip->insn_mo));
  pinfo = ip->insn_mo->pinfo;
  /* Conservatively treat all operands to an FP_D instruction are doubles.
     (This is overly pessimistic for things like cvt.s.d.)  */
  if (FPR_SIZE != 64 && (pinfo & FP_D))
    mask |= mask << 1;
  return mask;
}

/* Operand OPNUM of INSN is an odd-numbered floating-point register.
   Check whether that is allowed.  */

static bool
mips_oddfpreg_ok (const struct mips_opcode *insn, int opnum)
{
  const char *s = insn->name;
  bool oddspreg = (ISA_HAS_ODD_SINGLE_FPR (mips_opts.isa, mips_opts.arch)
		   || FPR_SIZE == 64) && mips_opts.oddspreg;

  if (insn->pinfo == INSN_MACRO)
    /* Let a macro pass, we'll catch it later when it is expanded.  */
    return true;

  /* Single-precision coprocessor loads and moves are OK for 32-bit registers,
     otherwise it depends on oddspreg.  */
  if ((insn->pinfo & FP_S)
      && (insn->pinfo & (INSN_LOAD_MEMORY | INSN_STORE_MEMORY
			 | INSN_LOAD_COPROC | INSN_COPROC_MOVE)))
    return FPR_SIZE == 32 || oddspreg;

  /* Allow odd registers for single-precision ops and double-precision if the
     floating-point registers are 64-bit wide.  */
  switch (insn->pinfo & (FP_S | FP_D))
    {
    case FP_S:
    case 0:
      return oddspreg;
    case FP_D:
      return FPR_SIZE == 64;
    default:
      break;
    }

  /* Cvt.w.x and cvt.x.w allow an odd register for a 'w' or 's' operand.  */
  s = strchr (insn->name, '.');
  if (s != NULL && opnum == 2)
    s = strchr (s + 1, '.');
  if (s != NULL && (s[1] == 'w' || s[1] == 's'))
    return oddspreg;

  return FPR_SIZE == 64;
}

/* Information about an instruction argument that we're trying to match.  */
struct mips_arg_info
{
  /* The instruction so far.  */
  struct mips_cl_insn *insn;

  /* The first unconsumed operand token.  */
  struct mips_operand_token *token;

  /* The 1-based operand number, in terms of insn->insn_mo->args.  */
  int opnum;

  /* The 1-based argument number, for error reporting.  This does not
     count elided optional registers, etc..  */
  int argnum;

  /* The last OP_REG operand seen, or ILLEGAL_REG if none.  */
  unsigned int last_regno;

  /* If the first operand was an OP_REG, this is the register that it
     specified, otherwise it is ILLEGAL_REG.  */
  unsigned int dest_regno;

  /* The value of the last OP_INT operand.  Only used for OP_MSB,
     where it gives the lsb position.  */
  unsigned int last_op_int;

  /* If true, match routines should assume that no later instruction
     alternative matches and should therefore be as accommodating as
     possible.  Match routines should not report errors if something
     is only invalid for !LAX_MATCH.  */
  bool lax_match;

  /* True if a reference to the current AT register was seen.  */
  bool seen_at;
};

/* Record that the argument is out of range.  */

static void
match_out_of_range (struct mips_arg_info *arg)
{
  set_insn_error_i (arg->argnum, _("operand %d out of range"), arg->argnum);
}

/* Record that the argument isn't constant but needs to be.  */

static void
match_not_constant (struct mips_arg_info *arg)
{
  set_insn_error_i (arg->argnum, _("operand %d must be constant"),
		    arg->argnum);
}

/* Try to match an OT_CHAR token for character CH.  Consume the token
   and return true on success, otherwise return false.  */

static bool
match_char (struct mips_arg_info *arg, char ch)
{
  if (arg->token->type == OT_CHAR && arg->token->u.ch == ch)
    {
      ++arg->token;
      if (ch == ',')
	arg->argnum += 1;
      return true;
    }
  return false;
}

/* Try to get an expression from the next tokens in ARG.  Consume the
   tokens and return true on success, storing the expression value in
   VALUE and relocation types in R.  */

static bool
match_expression (struct mips_arg_info *arg, expressionS *value,
		  bfd_reloc_code_real_type *r)
{
  /* If the next token is a '(' that was parsed as being part of a base
     expression, assume we have an elided offset.  The later match will fail
     if this turns out to be wrong.  */
  if (arg->token->type == OT_CHAR && arg->token->u.ch == '(')
    {
      value->X_op = O_constant;
      value->X_add_number = 0;
      r[0] = r[1] = r[2] = BFD_RELOC_UNUSED;
      return true;
    }

  /* Reject register-based expressions such as "0+$2" and "(($2))".
     For plain registers the default error seems more appropriate.  */
  if (arg->token->type == OT_INTEGER
      && arg->token->u.integer.value.X_op == O_register)
    {
      set_insn_error (arg->argnum, _("register value used as expression"));
      return false;
    }

  if (arg->token->type == OT_INTEGER)
    {
      *value = arg->token->u.integer.value;
      memcpy (r, arg->token->u.integer.relocs, 3 * sizeof (*r));
      ++arg->token;
      return true;
    }

  set_insn_error_i
    (arg->argnum, _("operand %d must be an immediate expression"),
     arg->argnum);
  return false;
}

/* Try to get a constant expression from the next tokens in ARG.  Consume
   the tokens and return true on success, storing the constant value
   in *VALUE.  */

static bool
match_const_int (struct mips_arg_info *arg, offsetT *value)
{
  expressionS ex;
  bfd_reloc_code_real_type r[3];

  if (!match_expression (arg, &ex, r))
    return false;

  if (r[0] == BFD_RELOC_UNUSED && ex.X_op == O_constant)
    *value = ex.X_add_number;
  else
    {
      if (r[0] == BFD_RELOC_UNUSED && ex.X_op == O_big)
	match_out_of_range (arg);
      else
	match_not_constant (arg);
      return false;
    }
  return true;
}

/* Return the RTYPE_* flags for a register operand of type TYPE that
   appears in instruction OPCODE.  */

static unsigned int
convert_reg_type (const struct mips_opcode *opcode,
		  enum mips_reg_operand_type type)
{
  switch (type)
    {
    case OP_REG_GP:
      return RTYPE_NUM | RTYPE_GP;

    case OP_REG_FP:
      /* Allow vector register names for MDMX if the instruction is a 64-bit
	 FPR load, store or move (including moves to and from GPRs).  */
      if ((mips_opts.ase & ASE_MDMX)
	  && (opcode->pinfo & FP_D)
	  && (opcode->pinfo & (INSN_COPROC_MOVE
			       | INSN_COPROC_MEMORY_DELAY
			       | INSN_LOAD_COPROC
			       | INSN_LOAD_MEMORY
			       | INSN_STORE_MEMORY)))
	return RTYPE_FPU | RTYPE_VEC;
      return RTYPE_FPU;

    case OP_REG_CCC:
      if (opcode->pinfo & (FP_D | FP_S))
	return RTYPE_CCC | RTYPE_FCC;
      return RTYPE_CCC;

    case OP_REG_VEC:
      if (opcode->membership & INSN_5400)
	return RTYPE_FPU;
      return RTYPE_FPU | RTYPE_VEC;

    case OP_REG_ACC:
      return RTYPE_ACC;

    case OP_REG_COPRO:
    case OP_REG_CONTROL:
      if (opcode->name[strlen (opcode->name) - 1] == '0')
	return RTYPE_NUM | RTYPE_CP0;
      return RTYPE_NUM;

    case OP_REG_HW:
      return RTYPE_NUM;

    case OP_REG_VI:
      return RTYPE_NUM | RTYPE_VI;

    case OP_REG_VF:
      return RTYPE_NUM | RTYPE_VF;

    case OP_REG_R5900_I:
      return RTYPE_R5900_I;

    case OP_REG_R5900_Q:
      return RTYPE_R5900_Q;

    case OP_REG_R5900_R:
      return RTYPE_R5900_R;

    case OP_REG_R5900_ACC:
      return RTYPE_R5900_ACC;

    case OP_REG_MSA:
      return RTYPE_MSA;

    case OP_REG_MSA_CTRL:
      return RTYPE_NUM;
    }
  abort ();
}

/* ARG is register REGNO, of type TYPE.  Warn about any dubious registers.  */

static void
check_regno (struct mips_arg_info *arg,
	     enum mips_reg_operand_type type, unsigned int regno)
{
  if (AT && type == OP_REG_GP && regno == AT)
    arg->seen_at = true;

  if (type == OP_REG_FP
      && (regno & 1) != 0
      && !mips_oddfpreg_ok (arg->insn->insn_mo, arg->opnum))
    {
      /* This was a warning prior to introducing O32 FPXX and FP64 support
	 so maintain a warning for FP32 but raise an error for the new
	 cases.  */
      if (FPR_SIZE == 32)
	as_warn (_("float register should be even, was %d"), regno);
      else
	as_bad (_("float register should be even, was %d"), regno);
    }

  if (type == OP_REG_CCC)
    {
      const char *name;
      size_t length;

      name = arg->insn->insn_mo->name;
      length = strlen (name);
      if ((regno & 1) != 0
	  && ((length >= 3 && strcmp (name + length - 3, ".ps") == 0)
	      || (length >= 5 && startswith (name + length - 5, "any2"))))
	as_warn (_("condition code register should be even for %s, was %d"),
		 name, regno);

      if ((regno & 3) != 0
	  && (length >= 5 && startswith (name + length - 5, "any4")))
	as_warn (_("condition code register should be 0 or 4 for %s, was %d"),
		 name, regno);
    }
}

/* ARG is a register with symbol value SYMVAL.  Try to interpret it as
   a register of type TYPE.  Return true on success, storing the register
   number in *REGNO and warning about any dubious uses.  */

static bool
match_regno (struct mips_arg_info *arg, enum mips_reg_operand_type type,
	     unsigned int symval, unsigned int *regno)
{
  if (type == OP_REG_VEC)
    symval = mips_prefer_vec_regno (symval);
  if (!(symval & convert_reg_type (arg->insn->insn_mo, type)))
    return false;

  *regno = symval & RNUM_MASK;
  check_regno (arg, type, *regno);
  return true;
}

/* Try to interpret the next token in ARG as a register of type TYPE.
   Consume the token and return true on success, storing the register
   number in *REGNO.  Return false on failure.  */

static bool
match_reg (struct mips_arg_info *arg, enum mips_reg_operand_type type,
	   unsigned int *regno)
{
  if (arg->token->type == OT_REG
      && match_regno (arg, type, arg->token->u.regno, regno))
    {
      ++arg->token;
      return true;
    }
  return false;
}

/* Try to interpret the next token in ARG as a range of registers of type TYPE.
   Consume the token and return true on success, storing the register numbers
   in *REGNO1 and *REGNO2.  Return false on failure.  */

static bool
match_reg_range (struct mips_arg_info *arg, enum mips_reg_operand_type type,
		 unsigned int *regno1, unsigned int *regno2)
{
  if (match_reg (arg, type, regno1))
    {
      *regno2 = *regno1;
      return true;
    }
  if (arg->token->type == OT_REG_RANGE
      && match_regno (arg, type, arg->token->u.reg_range.regno1, regno1)
      && match_regno (arg, type, arg->token->u.reg_range.regno2, regno2)
      && *regno1 <= *regno2)
    {
      ++arg->token;
      return true;
    }
  return false;
}

/* OP_INT matcher.  */

static bool
match_int_operand (struct mips_arg_info *arg,
		   const struct mips_operand *operand_base)
{
  const struct mips_int_operand *operand;
  unsigned int uval;
  int min_val, max_val, factor;
  offsetT sval;

  operand = (const struct mips_int_operand *) operand_base;
  factor = 1 << operand->shift;
  min_val = mips_int_operand_min (operand);
  max_val = mips_int_operand_max (operand);

  if (operand_base->lsb == 0
      && operand_base->size == 16
      && operand->shift == 0
      && operand->bias == 0
      && (operand->max_val == 32767 || operand->max_val == 65535))
    {
      /* The operand can be relocated.  */
      if (!match_expression (arg, &offset_expr, offset_reloc))
	return false;

      if (offset_expr.X_op == O_big)
	{
	  match_out_of_range (arg);
	  return false;
	}

      if (offset_reloc[0] != BFD_RELOC_UNUSED)
	/* Relocation operators were used.  Accept the argument and
	   leave the relocation value in offset_expr and offset_relocs
	   for the caller to process.  */
	return true;

      if (offset_expr.X_op != O_constant)
	{
	  /* Accept non-constant operands if no later alternative matches,
	     leaving it for the caller to process.  */
	  if (!arg->lax_match)
	    {
	      match_not_constant (arg);
	      return false;
	    }
	  offset_reloc[0] = BFD_RELOC_LO16;
	  return true;
	}

      /* Clear the global state; we're going to install the operand
	 ourselves.  */
      sval = offset_expr.X_add_number;
      offset_expr.X_op = O_absent;

      /* For compatibility with older assemblers, we accept
	 0x8000-0xffff as signed 16-bit numbers when only
	 signed numbers are allowed.  */
      if (sval > max_val)
	{
	  max_val = ((1 << operand_base->size) - 1) << operand->shift;
	  if (!arg->lax_match && sval <= max_val)
	    {
	      match_out_of_range (arg);
	      return false;
	    }
	}
    }
  else
    {
      if (!match_const_int (arg, &sval))
	return false;
    }

  arg->last_op_int = sval;

  if (sval < min_val || sval > max_val || sval % factor)
    {
      match_out_of_range (arg);
      return false;
    }

  uval = (unsigned int) sval >> operand->shift;
  uval -= operand->bias;

  /* Handle -mfix-cn63xxp1.  */
  if (arg->opnum == 1
      && mips_fix_cn63xxp1
      && !mips_opts.micromips
      && strcmp ("pref", arg->insn->insn_mo->name) == 0)
    switch (uval)
      {
      case 5:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
      case 30:
      case 31:
	/* These are ok.  */
	break;

      default:
	/* The rest must be changed to 28.  */
	uval = 28;
	break;
      }

  insn_insert_operand (arg->insn, operand_base, uval);
  return true;
}

/* OP_MAPPED_INT matcher.  */

static bool
match_mapped_int_operand (struct mips_arg_info *arg,
			  const struct mips_operand *operand_base)
{
  const struct mips_mapped_int_operand *operand;
  unsigned int uval, num_vals;
  offsetT sval;

  operand = (const struct mips_mapped_int_operand *) operand_base;
  if (!match_const_int (arg, &sval))
    return false;

  num_vals = 1 << operand_base->size;
  for (uval = 0; uval < num_vals; uval++)
    if (operand->int_map[uval] == sval)
      break;
  if (uval == num_vals)
    {
      match_out_of_range (arg);
      return false;
    }

  insn_insert_operand (arg->insn, operand_base, uval);
  return true;
}

/* OP_MSB matcher.  */

static bool
match_msb_operand (struct mips_arg_info *arg,
		   const struct mips_operand *operand_base)
{
  const struct mips_msb_operand *operand;
  int min_val, max_val, max_high;
  offsetT size, sval, high;

  operand = (const struct mips_msb_operand *) operand_base;
  min_val = operand->bias;
  max_val = min_val + (1 << operand_base->size) - 1;
  max_high = operand->opsize;

  if (!match_const_int (arg, &size))
    return false;

  high = size + arg->last_op_int;
  sval = operand->add_lsb ? high : size;

  if (size < 0 || high > max_high || sval < min_val || sval > max_val)
    {
      match_out_of_range (arg);
      return false;
    }
  insn_insert_operand (arg->insn, operand_base, sval - min_val);
  return true;
}

/* OP_REG matcher.  */

static bool
match_reg_operand (struct mips_arg_info *arg,
		   const struct mips_operand *operand_base)
{
  const struct mips_reg_operand *operand;
  unsigned int regno, uval, num_vals;

  operand = (const struct mips_reg_operand *) operand_base;
  if (!match_reg (arg, operand->reg_type, &regno))
    return false;

  if (operand->reg_map)
    {
      num_vals = 1 << operand->root.size;
      for (uval = 0; uval < num_vals; uval++)
	if (operand->reg_map[uval] == regno)
	  break;
      if (num_vals == uval)
	return false;
    }
  else
    uval = regno;

  arg->last_regno = regno;
  if (arg->opnum == 1)
    arg->dest_regno = regno;
  insn_insert_operand (arg->insn, operand_base, uval);
  return true;
}

/* OP_REG_PAIR matcher.  */

static bool
match_reg_pair_operand (struct mips_arg_info *arg,
			const struct mips_operand *operand_base)
{
  const struct mips_reg_pair_operand *operand;
  unsigned int regno1, regno2, uval, num_vals;

  operand = (const struct mips_reg_pair_operand *) operand_base;
  if (!match_reg (arg, operand->reg_type, &regno1)
      || !match_char (arg, ',')
      || !match_reg (arg, operand->reg_type, &regno2))
    return false;

  num_vals = 1 << operand_base->size;
  for (uval = 0; uval < num_vals; uval++)
    if (operand->reg1_map[uval] == regno1 && operand->reg2_map[uval] == regno2)
      break;
  if (uval == num_vals)
    return false;

  insn_insert_operand (arg->insn, operand_base, uval);
  return true;
}

/* OP_PCREL matcher.  The caller chooses the relocation type.  */

static bool
match_pcrel_operand (struct mips_arg_info *arg)
{
  bfd_reloc_code_real_type r[3];

  return match_expression (arg, &offset_expr, r) && r[0] == BFD_RELOC_UNUSED;
}

/* OP_PERF_REG matcher.  */

static bool
match_perf_reg_operand (struct mips_arg_info *arg,
			const struct mips_operand *operand)
{
  offsetT sval;

  if (!match_const_int (arg, &sval))
    return false;

  if (sval != 0
      && (sval != 1
	  || (mips_opts.arch == CPU_R5900
	      && (strcmp (arg->insn->insn_mo->name, "mfps") == 0
		  || strcmp (arg->insn->insn_mo->name, "mtps") == 0))))
    {
      set_insn_error (arg->argnum, _("invalid performance register"));
      return false;
    }

  insn_insert_operand (arg->insn, operand, sval);
  return true;
}

/* OP_ADDIUSP matcher.  */

static bool
match_addiusp_operand (struct mips_arg_info *arg,
		       const struct mips_operand *operand)
{
  offsetT sval;
  unsigned int uval;

  if (!match_const_int (arg, &sval))
    return false;

  if (sval % 4)
    {
      match_out_of_range (arg);
      return false;
    }

  sval /= 4;
  if (!(sval >= -258 && sval <= 257) || (sval >= -2 && sval <= 1))
    {
      match_out_of_range (arg);
      return false;
    }

  uval = (unsigned int) sval;
  uval = ((uval >> 1) & ~0xff) | (uval & 0xff);
  insn_insert_operand (arg->insn, operand, uval);
  return true;
}

/* OP_CLO_CLZ_DEST matcher.  */

static bool
match_clo_clz_dest_operand (struct mips_arg_info *arg,
			    const struct mips_operand *operand)
{
  unsigned int regno;

  if (!match_reg (arg, OP_REG_GP, &regno))
    return false;

  insn_insert_operand (arg->insn, operand, regno | (regno << 5));
  return true;
}

/* OP_CHECK_PREV matcher.  */

static bool
match_check_prev_operand (struct mips_arg_info *arg,
			  const struct mips_operand *operand_base)
{
  const struct mips_check_prev_operand *operand;
  unsigned int regno;

  operand = (const struct mips_check_prev_operand *) operand_base;

  if (!match_reg (arg, OP_REG_GP, &regno))
    return false;

  if (!operand->zero_ok && regno == 0)
    return false;

  if ((operand->less_than_ok && regno < arg->last_regno)
      || (operand->greater_than_ok && regno > arg->last_regno)
      || (operand->equal_ok && regno == arg->last_regno))
    {
      arg->last_regno = regno;
      insn_insert_operand (arg->insn, operand_base, regno);
      return true;
    }

  return false;
}

/* OP_SAME_RS_RT matcher.  */

static bool
match_same_rs_rt_operand (struct mips_arg_info *arg,
			  const struct mips_operand *operand)
{
  unsigned int regno;

  if (!match_reg (arg, OP_REG_GP, &regno))
    return false;

  if (regno == 0)
    {
      set_insn_error (arg->argnum, _("the source register must not be $0"));
      return false;
    }

  arg->last_regno = regno;

  insn_insert_operand (arg->insn, operand, regno | (regno << 5));
  return true;
}

/* OP_LWM_SWM_LIST matcher.  */

static bool
match_lwm_swm_list_operand (struct mips_arg_info *arg,
			    const struct mips_operand *operand)
{
  unsigned int reglist, sregs, ra, regno1, regno2;
  struct mips_arg_info reset;

  reglist = 0;
  if (!match_reg_range (arg, OP_REG_GP, &regno1, &regno2))
    return false;
  do
    {
      if (regno2 == FP && regno1 >= S0 && regno1 <= S7)
	{
	  reglist |= 1 << FP;
	  regno2 = S7;
	}
      reglist |= ((1U << regno2 << 1) - 1) & -(1U << regno1);
      reset = *arg;
    }
  while (match_char (arg, ',')
	 && match_reg_range (arg, OP_REG_GP, &regno1, &regno2));
  *arg = reset;

  if (operand->size == 2)
    {
      /* The list must include both ra and s0-sN, for 0 <= N <= 3.  E.g.:

	 s0, ra
	 s0, s1, ra, s2, s3
	 s0-s2, ra

	 and any permutations of these.  */
      if ((reglist & 0xfff1ffff) != 0x80010000)
	return false;

      sregs = (reglist >> 17) & 7;
      ra = 0;
    }
  else
    {
      /* The list must include at least one of ra and s0-sN,
	 for 0 <= N <= 8.  (Note that there is a gap between s7 and s8,
	 which are $23 and $30 respectively.)  E.g.:

	 ra
	 s0
	 ra, s0, s1, s2
	 s0-s8
	 s0-s5, ra

	 and any permutations of these.  */
      if ((reglist & 0x3f00ffff) != 0)
	return false;

      ra = (reglist >> 27) & 0x10;
      sregs = ((reglist >> 22) & 0x100) | ((reglist >> 16) & 0xff);
    }
  sregs += 1;
  if ((sregs & -sregs) != sregs)
    return false;

  insn_insert_operand (arg->insn, operand, (ffs (sregs) - 1) | ra);
  return true;
}

/* OP_ENTRY_EXIT_LIST matcher.  */

static unsigned int
match_entry_exit_operand (struct mips_arg_info *arg,
			  const struct mips_operand *operand)
{
  unsigned int mask;
  bool is_exit;

  /* The format is the same for both ENTRY and EXIT, but the constraints
     are different.  */
  is_exit = strcmp (arg->insn->insn_mo->name, "exit") == 0;
  mask = (is_exit ? 7 << 3 : 0);
  do
    {
      unsigned int regno1, regno2;
      bool is_freg;

      if (match_reg_range (arg, OP_REG_GP, &regno1, &regno2))
	is_freg = false;
      else if (match_reg_range (arg, OP_REG_FP, &regno1, &regno2))
	is_freg = true;
      else
	return false;

      if (is_exit && is_freg && regno1 == 0 && regno2 < 2)
	{
	  mask &= ~(7 << 3);
	  mask |= (5 + regno2) << 3;
	}
      else if (!is_exit && regno1 == 4 && regno2 >= 4 && regno2 <= 7)
	mask |= (regno2 - 3) << 3;
      else if (regno1 == 16 && regno2 >= 16 && regno2 <= 17)
	mask |= (regno2 - 15) << 1;
      else if (regno1 == RA && regno2 == RA)
	mask |= 1;
      else
	return false;
    }
  while (match_char (arg, ','));

  insn_insert_operand (arg->insn, operand, mask);
  return true;
}

/* Encode regular MIPS SAVE/RESTORE instruction operands according to
   the argument register mask AMASK, the number of static registers
   saved NSREG, the $ra, $s0 and $s1 register specifiers RA, S0 and S1
   respectively, and the frame size FRAME_SIZE.  */

static unsigned int
mips_encode_save_restore (unsigned int amask, unsigned int nsreg,
			  unsigned int ra, unsigned int s0, unsigned int s1,
			  unsigned int frame_size)
{
  return ((nsreg << 23) | ((frame_size & 0xf0) << 15) | (amask << 15)
	  | (ra << 12) | (s0 << 11) | (s1 << 10) | ((frame_size & 0xf) << 6));
}

/* Encode MIPS16 SAVE/RESTORE instruction operands according to the
   argument register mask AMASK, the number of static registers saved
   NSREG, the $ra, $s0 and $s1 register specifiers RA, S0 and S1
   respectively, and the frame size FRAME_SIZE.  */

static unsigned int
mips16_encode_save_restore (unsigned int amask, unsigned int nsreg,
			    unsigned int ra, unsigned int s0, unsigned int s1,
			    unsigned int frame_size)
{
  unsigned int args;

  args = (ra << 6) | (s0 << 5) | (s1 << 4) | (frame_size & 0xf);
  if (nsreg || amask || frame_size == 0 || frame_size > 16)
    args |= (MIPS16_EXTEND | (nsreg << 24) | (amask << 16)
	     | ((frame_size & 0xf0) << 16));
  return args;
}

/* OP_SAVE_RESTORE_LIST matcher.  */

static bool
match_save_restore_list_operand (struct mips_arg_info *arg)
{
  unsigned int opcode, args, statics, sregs;
  unsigned int num_frame_sizes, num_args, num_statics, num_sregs;
  unsigned int arg_mask, ra, s0, s1;
  offsetT frame_size;

  opcode = arg->insn->insn_opcode;
  frame_size = 0;
  num_frame_sizes = 0;
  args = 0;
  statics = 0;
  sregs = 0;
  ra = 0;
  s0 = 0;
  s1 = 0;
  do
    {
      unsigned int regno1, regno2;

      if (arg->token->type == OT_INTEGER)
	{
	  /* Handle the frame size.  */
	  if (!match_const_int (arg, &frame_size))
	    return false;
	  num_frame_sizes += 1;
	}
      else
	{
	  if (!match_reg_range (arg, OP_REG_GP, &regno1, &regno2))
	    return false;

	  while (regno1 <= regno2)
	    {
	      if (regno1 >= 4 && regno1 <= 7)
		{
		  if (num_frame_sizes == 0)
		    /* args $a0-$a3 */
		    args |= 1 << (regno1 - 4);
		  else
		    /* statics $a0-$a3 */
		    statics |= 1 << (regno1 - 4);
		}
	      else if (regno1 >= 16 && regno1 <= 23)
		/* $s0-$s7 */
		sregs |= 1 << (regno1 - 16);
	      else if (regno1 == 30)
		/* $s8 */
		sregs |= 1 << 8;
	      else if (regno1 == 31)
		/* Add $ra to insn.  */
		ra = 1;
	      else
		return false;
	      regno1 += 1;
	      if (regno1 == 24)
		regno1 = 30;
	    }
	}
    }
  while (match_char (arg, ','));

  /* Encode args/statics combination.  */
  if (args & statics)
    return false;
  else if (args == 0xf)
    /* All $a0-$a3 are args.  */
    arg_mask = MIPS_SVRS_ALL_ARGS;
  else if (statics == 0xf)
    /* All $a0-$a3 are statics.  */
    arg_mask = MIPS_SVRS_ALL_STATICS;
  else
    {
      /* Count arg registers.  */
      num_args = 0;
      while (args & 0x1)
	{
	  args >>= 1;
	  num_args += 1;
	}
      if (args != 0)
	return false;

      /* Count static registers.  */
      num_statics = 0;
      while (statics & 0x8)
	{
	  statics = (statics << 1) & 0xf;
	  num_statics += 1;
	}
      if (statics != 0)
	return false;

      /* Encode args/statics.  */
      arg_mask = (num_args << 2) | num_statics;
    }

  /* Encode $s0/$s1.  */
  if (sregs & (1 << 0))		/* $s0 */
    s0 = 1;
  if (sregs & (1 << 1))		/* $s1 */
    s1 = 1;
  sregs >>= 2;

  /* Encode $s2-$s8. */
  num_sregs = 0;
  while (sregs & 1)
    {
      sregs >>= 1;
      num_sregs += 1;
    }
  if (sregs != 0)
    return false;

  /* Encode frame size.  */
  if (num_frame_sizes == 0)
    {
      set_insn_error (arg->argnum, _("missing frame size"));
      return false;
    }
  if (num_frame_sizes > 1)
    {
      set_insn_error (arg->argnum, _("frame size specified twice"));
      return false;
    }
  if ((frame_size & 7) != 0 || frame_size < 0 || frame_size > 0xff * 8)
    {
      set_insn_error (arg->argnum, _("invalid frame size"));
      return false;
    }
  frame_size /= 8;

  /* Finally build the instruction.  */
  if (mips_opts.mips16)
    opcode |= mips16_encode_save_restore (arg_mask, num_sregs, ra, s0, s1,
					  frame_size);
  else if (!mips_opts.micromips)
    opcode |= mips_encode_save_restore (arg_mask, num_sregs, ra, s0, s1,
					frame_size);
  else
    abort ();

  arg->insn->insn_opcode = opcode;
  return true;
}

/* OP_MDMX_IMM_REG matcher.  */

static bool
match_mdmx_imm_reg_operand (struct mips_arg_info *arg,
			    const struct mips_operand *operand)
{
  unsigned int regno, uval;
  bool is_qh;
  const struct mips_opcode *opcode;

  /* The mips_opcode records whether this is an octobyte or quadhalf
     instruction.  Start out with that bit in place.  */
  opcode = arg->insn->insn_mo;
  uval = mips_extract_operand (operand, opcode->match);
  is_qh = (uval != 0);

  if (arg->token->type == OT_REG)
    {
      if ((opcode->membership & INSN_5400)
	  && strcmp (opcode->name, "rzu.ob") == 0)
	{
	  set_insn_error_i (arg->argnum, _("operand %d must be an immediate"),
			    arg->argnum);
	  return false;
	}

      if (!match_regno (arg, OP_REG_VEC, arg->token->u.regno, &regno))
	return false;
      ++arg->token;

      /* Check whether this is a vector register or a broadcast of
	 a single element.  */
      if (arg->token->type == OT_INTEGER_INDEX)
	{
	  if (arg->token->u.index > (is_qh ? 3 : 7))
	    {
	      set_insn_error (arg->argnum, _("invalid element selector"));
	      return false;
	    }
	  uval |= arg->token->u.index << (is_qh ? 2 : 1) << 5;
	  ++arg->token;
	}
      else
	{
	  /* A full vector.  */
	  if ((opcode->membership & INSN_5400)
	      && (strcmp (opcode->name, "sll.ob") == 0
		  || strcmp (opcode->name, "srl.ob") == 0))
	    {
	      set_insn_error_i (arg->argnum, _("operand %d must be scalar"),
				arg->argnum);
	      return false;
	    }

	  if (is_qh)
	    uval |= MDMX_FMTSEL_VEC_QH << 5;
	  else
	    uval |= MDMX_FMTSEL_VEC_OB << 5;
	}
      uval |= regno;
    }
  else
    {
      offsetT sval;

      if (!match_const_int (arg, &sval))
	return false;
      if (sval < 0 || sval > 31)
	{
	  match_out_of_range (arg);
	  return false;
	}
      uval |= (sval & 31);
      if (is_qh)
	uval |= MDMX_FMTSEL_IMM_QH << 5;
      else
	uval |= MDMX_FMTSEL_IMM_OB << 5;
    }
  insn_insert_operand (arg->insn, operand, uval);
  return true;
}

/* OP_IMM_INDEX matcher.  */

static bool
match_imm_index_operand (struct mips_arg_info *arg,
			 const struct mips_operand *operand)
{
  unsigned int max_val;

  if (arg->token->type != OT_INTEGER_INDEX)
    return false;

  max_val = (1 << operand->size) - 1;
  if (arg->token->u.index > max_val)
    {
      match_out_of_range (arg);
      return false;
    }
  insn_insert_operand (arg->insn, operand, arg->token->u.index);
  ++arg->token;
  return true;
}

/* OP_REG_INDEX matcher.  */

static bool
match_reg_index_operand (struct mips_arg_info *arg,
			 const struct mips_operand *operand)
{
  unsigned int regno;

  if (arg->token->type != OT_REG_INDEX)
    return false;

  if (!match_regno (arg, OP_REG_GP, arg->token->u.regno, &regno))
    return false;

  insn_insert_operand (arg->insn, operand, regno);
  ++arg->token;
  return true;
}

/* OP_PC matcher.  */

static bool
match_pc_operand (struct mips_arg_info *arg)
{
  if (arg->token->type == OT_REG && (arg->token->u.regno & RTYPE_PC))
    {
      ++arg->token;
      return true;
    }
  return false;
}

/* OP_REG28 matcher.  */

static bool
match_reg28_operand (struct mips_arg_info *arg)
{
  unsigned int regno;

  if (arg->token->type == OT_REG
      && match_regno (arg, OP_REG_GP, arg->token->u.regno, &regno)
      && regno == GP)
    {
      ++arg->token;
      return true;
    }
  return false;
}

/* OP_NON_ZERO_REG matcher.  */

static bool
match_non_zero_reg_operand (struct mips_arg_info *arg,
			    const struct mips_operand *operand)
{
  unsigned int regno;

  if (!match_reg (arg, OP_REG_GP, &regno))
    return false;

  if (regno == 0)
    {
      set_insn_error (arg->argnum, _("the source register must not be $0"));
      return false;
    }

  arg->last_regno = regno;
  insn_insert_operand (arg->insn, operand, regno);
  return true;
}

/* OP_REPEAT_DEST_REG and OP_REPEAT_PREV_REG matcher.  OTHER_REGNO is the
   register that we need to match.  */

static bool
match_tied_reg_operand (struct mips_arg_info *arg, unsigned int other_regno)
{
  unsigned int regno;

  return match_reg (arg, OP_REG_GP, &regno) && regno == other_regno;
}

/* Try to match a floating-point constant from ARG for LI.S or LI.D.
   LENGTH is the length of the value in bytes (4 for float, 8 for double)
   and USING_GPRS says whether the destination is a GPR rather than an FPR.

   Return the constant in IMM and OFFSET as follows:

   - If the constant should be loaded via memory, set IMM to O_absent and
     OFFSET to the memory address.

   - Otherwise, if the constant should be loaded into two 32-bit registers,
     set IMM to the O_constant to load into the high register and OFFSET
     to the corresponding value for the low register.

   - Otherwise, set IMM to the full O_constant and set OFFSET to O_absent.

   These constants only appear as the last operand in an instruction,
   and every instruction that accepts them in any variant accepts them
   in all variants.  This means we don't have to worry about backing out
   any changes if the instruction does not match.  We just match
   unconditionally and report an error if the constant is invalid.  */

static bool
match_float_constant (struct mips_arg_info *arg, expressionS *imm,
		      expressionS *offset, int length, bool using_gprs)
{
  char *p;
  segT seg, new_seg;
  subsegT subseg;
  const char *newname;
  unsigned char *data;

  /* Where the constant is placed is based on how the MIPS assembler
     does things:

     length == 4 && using_gprs  -- immediate value only
     length == 8 && using_gprs  -- .rdata or immediate value
     length == 4 && !using_gprs -- .lit4 or immediate value
     length == 8 && !using_gprs -- .lit8 or immediate value

     The .lit4 and .lit8 sections are only used if permitted by the
     -G argument.  */
  if (arg->token->type != OT_FLOAT)
    {
      set_insn_error (arg->argnum, _("floating-point expression required"));
      return false;
    }

  gas_assert (arg->token->u.flt.length == length);
  data = arg->token->u.flt.data;
  ++arg->token;

  /* Handle 32-bit constants for which an immediate value is best.  */
  if (length == 4
      && (using_gprs
	  || g_switch_value < 4
	  || (data[0] == 0 && data[1] == 0)
	  || (data[2] == 0 && data[3] == 0)))
    {
      imm->X_op = O_constant;
      if (!target_big_endian)
	imm->X_add_number = bfd_getl32 (data);
      else
	imm->X_add_number = bfd_getb32 (data);
      offset->X_op = O_absent;
      return true;
    }

  /* Handle 64-bit constants for which an immediate value is best.  */
  if (length == 8
      && !mips_disable_float_construction
      /* Constants can only be constructed in GPRs and copied to FPRs if the
	 GPRs are at least as wide as the FPRs or MTHC1 is available.
	 Unlike most tests for 32-bit floating-point registers this check
	 specifically looks for GPR_SIZE == 32 as the FPXX ABI does not
	 permit 64-bit moves without MXHC1.
	 Force the constant into memory otherwise.  */
      && (using_gprs
	  || GPR_SIZE == 64
	  || ISA_HAS_MXHC1 (mips_opts.isa)
	  || FPR_SIZE == 32)
      && ((data[0] == 0 && data[1] == 0)
	  || (data[2] == 0 && data[3] == 0))
      && ((data[4] == 0 && data[5] == 0)
	  || (data[6] == 0 && data[7] == 0)))
    {
      /* The value is simple enough to load with a couple of instructions.
	 If using 32-bit registers, set IMM to the high order 32 bits and
	 OFFSET to the low order 32 bits.  Otherwise, set IMM to the entire
	 64 bit constant.  */
      if (GPR_SIZE == 32 || (!using_gprs && FPR_SIZE != 64))
	{
	  imm->X_op = O_constant;
	  offset->X_op = O_constant;
	  if (!target_big_endian)
	    {
	      imm->X_add_number = bfd_getl32 (data + 4);
	      offset->X_add_number = bfd_getl32 (data);
	    }
	  else
	    {
	      imm->X_add_number = bfd_getb32 (data);
	      offset->X_add_number = bfd_getb32 (data + 4);
	    }
	  if (offset->X_add_number == 0)
	    offset->X_op = O_absent;
	}
      else
	{
	  imm->X_op = O_constant;
	  if (!target_big_endian)
	    imm->X_add_number = bfd_getl64 (data);
	  else
	    imm->X_add_number = bfd_getb64 (data);
	  offset->X_op = O_absent;
	}
      return true;
    }

  /* Switch to the right section.  */
  seg = now_seg;
  subseg = now_subseg;
  if (length == 4)
    {
      gas_assert (!using_gprs && g_switch_value >= 4);
      newname = ".lit4";
    }
  else
    {
      if (using_gprs || g_switch_value < 8)
	newname = RDATA_SECTION_NAME;
      else
	newname = ".lit8";
    }

  new_seg = subseg_new (newname, (subsegT) 0);
  bfd_set_section_flags (new_seg,
			 SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_DATA);
  frag_align (length == 4 ? 2 : 3, 0, 0);
  if (!startswith (TARGET_OS, "elf"))
    record_alignment (new_seg, 4);
  else
    record_alignment (new_seg, length == 4 ? 2 : 3);
  if (seg == now_seg)
    as_bad (_("cannot use `%s' in this section"), arg->insn->insn_mo->name);

  /* Set the argument to the current address in the section.  */
  imm->X_op = O_absent;
  offset->X_op = O_symbol;
  offset->X_add_symbol = symbol_temp_new_now ();
  offset->X_add_number = 0;

  /* Put the floating point number into the section.  */
  p = frag_more (length);
  memcpy (p, data, length);

  /* Switch back to the original section.  */
  subseg_set (seg, subseg);
  return true;
}

/* OP_VU0_SUFFIX and OP_VU0_MATCH_SUFFIX matcher; MATCH_P selects between
   them.  */

static bool
match_vu0_suffix_operand (struct mips_arg_info *arg,
			  const struct mips_operand *operand,
			  bool match_p)
{
  unsigned int uval;

  /* The operand can be an XYZW mask or a single 2-bit channel index
     (with X being 0).  */
  gas_assert (operand->size == 2 || operand->size == 4);

  /* The suffix can be omitted when it is already part of the opcode.  */
  if (arg->token->type != OT_CHANNELS)
    return match_p;

  uval = arg->token->u.channels;
  if (operand->size == 2)
    {
      /* Check that a single bit is set and convert it into a 2-bit index.  */
      if ((uval & -uval) != uval)
	return false;
      uval = 4 - ffs (uval);
    }

  if (match_p && insn_extract_operand (arg->insn, operand) != uval)
    return false;

  ++arg->token;
  if (!match_p)
    insn_insert_operand (arg->insn, operand, uval);
  return true;
}

/* Try to match a token from ARG against OPERAND.  Consume the token
   and return true on success, otherwise return false.  */

static bool
match_operand (struct mips_arg_info *arg,
	       const struct mips_operand *operand)
{
  switch (operand->type)
    {
    case OP_INT:
      return match_int_operand (arg, operand);

    case OP_MAPPED_INT:
      return match_mapped_int_operand (arg, operand);

    case OP_MSB:
      return match_msb_operand (arg, operand);

    case OP_REG:
    case OP_OPTIONAL_REG:
      return match_reg_operand (arg, operand);

    case OP_REG_PAIR:
      return match_reg_pair_operand (arg, operand);

    case OP_PCREL:
      return match_pcrel_operand (arg);

    case OP_PERF_REG:
      return match_perf_reg_operand (arg, operand);

    case OP_ADDIUSP_INT:
      return match_addiusp_operand (arg, operand);

    case OP_CLO_CLZ_DEST:
      return match_clo_clz_dest_operand (arg, operand);

    case OP_LWM_SWM_LIST:
      return match_lwm_swm_list_operand (arg, operand);

    case OP_ENTRY_EXIT_LIST:
      return match_entry_exit_operand (arg, operand);

    case OP_SAVE_RESTORE_LIST:
      return match_save_restore_list_operand (arg);

    case OP_MDMX_IMM_REG:
      return match_mdmx_imm_reg_operand (arg, operand);

    case OP_REPEAT_DEST_REG:
      return match_tied_reg_operand (arg, arg->dest_regno);

    case OP_REPEAT_PREV_REG:
      return match_tied_reg_operand (arg, arg->last_regno);

    case OP_PC:
      return match_pc_operand (arg);

    case OP_REG28:
      return match_reg28_operand (arg);

    case OP_VU0_SUFFIX:
      return match_vu0_suffix_operand (arg, operand, false);

    case OP_VU0_MATCH_SUFFIX:
      return match_vu0_suffix_operand (arg, operand, true);

    case OP_IMM_INDEX:
      return match_imm_index_operand (arg, operand);

    case OP_REG_INDEX:
      return match_reg_index_operand (arg, operand);

    case OP_SAME_RS_RT:
      return match_same_rs_rt_operand (arg, operand);

    case OP_CHECK_PREV:
      return match_check_prev_operand (arg, operand);

    case OP_NON_ZERO_REG:
      return match_non_zero_reg_operand (arg, operand);
    }
  abort ();
}

/* ARG is the state after successfully matching an instruction.
   Issue any queued-up warnings.  */

static void
check_completed_insn (struct mips_arg_info *arg)
{
  if (arg->seen_at)
    {
      if (AT == ATREG)
	as_warn (_("used $at without \".set noat\""));
      else
	as_warn (_("used $%u with \".set at=$%u\""), AT, AT);
    }
}

/* Return true if modifying general-purpose register REG needs a delay.  */

static bool
reg_needs_delay (unsigned int reg)
{
  unsigned long prev_pinfo;

  prev_pinfo = history[0].insn_mo->pinfo;
  if (!mips_opts.noreorder
      && (((prev_pinfo & INSN_LOAD_MEMORY) && !gpr_interlocks)
	  || ((prev_pinfo & INSN_LOAD_COPROC) && !cop_interlocks))
      && (gpr_write_mask (&history[0]) & (1 << reg)))
    return true;

  return false;
}

/* Classify an instruction according to the FIX_VR4120_* enumeration.
   Return NUM_FIX_VR4120_CLASSES if the instruction isn't affected
   by VR4120 errata.  */

static unsigned int
classify_vr4120_insn (const char *name)
{
  if (startswith (name, "macc"))
    return FIX_VR4120_MACC;
  if (startswith (name, "dmacc"))
    return FIX_VR4120_DMACC;
  if (startswith (name, "mult"))
    return FIX_VR4120_MULT;
  if (startswith (name, "dmult"))
    return FIX_VR4120_DMULT;
  if (strstr (name, "div"))
    return FIX_VR4120_DIV;
  if (strcmp (name, "mtlo") == 0 || strcmp (name, "mthi") == 0)
    return FIX_VR4120_MTHILO;
  return NUM_FIX_VR4120_CLASSES;
}

#define INSN_ERET	0x42000018
#define INSN_DERET	0x4200001f
#define INSN_DMULT	0x1c
#define INSN_DMULTU	0x1d

/* Return the number of instructions that must separate INSN1 and INSN2,
   where INSN1 is the earlier instruction.  Return the worst-case value
   for any INSN2 if INSN2 is null.  */

static unsigned int
insns_between (const struct mips_cl_insn *insn1,
	       const struct mips_cl_insn *insn2)
{
  unsigned long pinfo1, pinfo2;
  unsigned int mask;

  /* If INFO2 is null, pessimistically assume that all flags are set for
     the second instruction.  */
  pinfo1 = insn1->insn_mo->pinfo;
  pinfo2 = insn2 ? insn2->insn_mo->pinfo : ~0U;

  /* For most targets, write-after-read dependencies on the HI and LO
     registers must be separated by at least two instructions.  */
  if (!hilo_interlocks)
    {
      if ((pinfo1 & INSN_READ_LO) && (pinfo2 & INSN_WRITE_LO))
	return 2;
      if ((pinfo1 & INSN_READ_HI) && (pinfo2 & INSN_WRITE_HI))
	return 2;
    }

  /* If we're working around r7000 errata, there must be two instructions
     between an mfhi or mflo and any instruction that uses the result.  */
  if (mips_7000_hilo_fix
      && !mips_opts.micromips
      && MF_HILO_INSN (pinfo1)
      && (insn2 == NULL || (gpr_read_mask (insn2) & gpr_write_mask (insn1))))
    return 2;

  /* If we're working around 24K errata, one instruction is required
     if an ERET or DERET is followed by a branch instruction.  */
  if (mips_fix_24k && !mips_opts.micromips)
    {
      if (insn1->insn_opcode == INSN_ERET
	  || insn1->insn_opcode == INSN_DERET)
	{
	  if (insn2 == NULL
	      || insn2->insn_opcode == INSN_ERET
	      || insn2->insn_opcode == INSN_DERET
	      || delayed_branch_p (insn2))
	    return 1;
	}
    }

  /* If we're working around PMC RM7000 errata, there must be three
     nops between a dmult and a load instruction.  */
  if (mips_fix_rm7000 && !mips_opts.micromips)
    {
      if ((insn1->insn_opcode & insn1->insn_mo->mask) == INSN_DMULT
	  || (insn1->insn_opcode & insn1->insn_mo->mask) == INSN_DMULTU)
	{
	  if (pinfo2 & INSN_LOAD_MEMORY)
	   return 3;
	}
    }

  /* If working around VR4120 errata, check for combinations that need
     a single intervening instruction.  */
  if (mips_fix_vr4120 && !mips_opts.micromips)
    {
      unsigned int class1, class2;

      class1 = classify_vr4120_insn (insn1->insn_mo->name);
      if (class1 != NUM_FIX_VR4120_CLASSES && vr4120_conflicts[class1] != 0)
	{
	  if (insn2 == NULL)
	    return 1;
	  class2 = classify_vr4120_insn (insn2->insn_mo->name);
	  if (vr4120_conflicts[class1] & (1 << class2))
	    return 1;
	}
    }

  if (!HAVE_CODE_COMPRESSION)
    {
      /* Check for GPR or coprocessor load delays.  All such delays
	 are on the RT register.  */
      /* Itbl support may require additional care here.  */
      if ((!gpr_interlocks && (pinfo1 & INSN_LOAD_MEMORY))
	  || (!cop_interlocks && (pinfo1 & INSN_LOAD_COPROC)))
	{
	  if (insn2 == NULL || (gpr_read_mask (insn2) & gpr_write_mask (insn1)))
	    return 1;
	}

      /* Check for generic coprocessor hazards.

	 This case is not handled very well.  There is no special
	 knowledge of CP0 handling, and the coprocessors other than
	 the floating point unit are not distinguished at all.  */
      /* Itbl support may require additional care here. FIXME!
	 Need to modify this to include knowledge about
	 user specified delays!  */
      else if ((!cop_interlocks && (pinfo1 & INSN_COPROC_MOVE))
	       || (!cop_mem_interlocks && (pinfo1 & INSN_COPROC_MEMORY_DELAY)))
	{
	  /* Handle cases where INSN1 writes to a known general coprocessor
	     register.  There must be a one instruction delay before INSN2
	     if INSN2 reads that register, otherwise no delay is needed.  */
	  mask = fpr_write_mask (insn1);
	  if (mask != 0)
	    {
	      if (!insn2 || (mask & fpr_read_mask (insn2)) != 0)
		return 1;
	    }
	  else
	    {
	      /* Read-after-write dependencies on the control registers
		 require a two-instruction gap.  */
	      if ((pinfo1 & INSN_WRITE_COND_CODE)
		  && (pinfo2 & INSN_READ_COND_CODE))
		return 2;

	      /* We don't know exactly what INSN1 does.  If INSN2 is
		 also a coprocessor instruction, assume there must be
		 a one instruction gap.  */
	      if (pinfo2 & INSN_COP)
		return 1;
	    }
	}

      /* Check for read-after-write dependencies on the coprocessor
	 control registers in cases where INSN1 does not need a general
	 coprocessor delay.  This means that INSN1 is a floating point
	 comparison instruction.  */
      /* Itbl support may require additional care here.  */
      else if (!cop_interlocks
	       && (pinfo1 & INSN_WRITE_COND_CODE)
	       && (pinfo2 & INSN_READ_COND_CODE))
	return 1;
    }

  /* Forbidden slots can not contain Control Transfer Instructions (CTIs)
     CTIs include all branches and jumps, nal, eret, eretnc, deret, wait
     and pause.  */
  if ((insn1->insn_mo->pinfo2 & INSN2_FORBIDDEN_SLOT)
      && ((pinfo2 & INSN_NO_DELAY_SLOT)
	  || (insn2 && delayed_branch_p (insn2))))
    return 1;

  return 0;
}

/* Return the number of nops that would be needed to work around the
   VR4130 mflo/mfhi errata if instruction INSN immediately followed
   the MAX_VR4130_NOPS instructions described by HIST.  Ignore hazards
   that are contained within the first IGNORE instructions of HIST.  */

static int
nops_for_vr4130 (int ignore, const struct mips_cl_insn *hist,
		 const struct mips_cl_insn *insn)
{
  int i, j;
  unsigned int mask;

  /* Check if the instruction writes to HI or LO.  MTHI and MTLO
     are not affected by the errata.  */
  if (insn != 0
      && ((insn->insn_mo->pinfo & (INSN_WRITE_HI | INSN_WRITE_LO)) == 0
	  || strcmp (insn->insn_mo->name, "mtlo") == 0
	  || strcmp (insn->insn_mo->name, "mthi") == 0))
    return 0;

  /* Search for the first MFLO or MFHI.  */
  for (i = 0; i < MAX_VR4130_NOPS; i++)
    if (MF_HILO_INSN (hist[i].insn_mo->pinfo))
      {
	/* Extract the destination register.  */
	mask = gpr_write_mask (&hist[i]);

	/* No nops are needed if INSN reads that register.  */
	if (insn != NULL && (gpr_read_mask (insn) & mask) != 0)
	  return 0;

	/* ...or if any of the intervening instructions do.  */
	for (j = 0; j < i; j++)
	  if (gpr_read_mask (&hist[j]) & mask)
	    return 0;

	if (i >= ignore)
	  return MAX_VR4130_NOPS - i;
      }
  return 0;
}

#define BASE_REG_EQ(INSN1, INSN2)	\
  ((((INSN1) >> OP_SH_RS) & OP_MASK_RS)	\
      == (((INSN2) >> OP_SH_RS) & OP_MASK_RS))

/* Return the minimum alignment for this store instruction.  */

static int
fix_24k_align_to (const struct mips_opcode *mo)
{
  if (strcmp (mo->name, "sh") == 0)
    return 2;

  if (strcmp (mo->name, "swc1") == 0
      || strcmp (mo->name, "swc2") == 0
      || strcmp (mo->name, "sw") == 0
      || strcmp (mo->name, "sc") == 0
      || strcmp (mo->name, "s.s") == 0)
    return 4;

  if (strcmp (mo->name, "sdc1") == 0
      || strcmp (mo->name, "sdc2") == 0
      || strcmp (mo->name, "s.d") == 0)
    return 8;

  /* sb, swl, swr */
  return 1;
}

struct fix_24k_store_info
  {
    /* Immediate offset, if any, for this store instruction.  */
    short off;
    /* Alignment required by this store instruction.  */
    int align_to;
    /* True for register offsets.  */
    int register_offset;
  };

/* Comparison function used by qsort.  */

static int
fix_24k_sort (const void *a, const void *b)
{
  const struct fix_24k_store_info *pos1 = a;
  const struct fix_24k_store_info *pos2 = b;

  return (pos1->off - pos2->off);
}

/* INSN is a store instruction.  Try to record the store information
   in STINFO.  Return false if the information isn't known.  */

static bool
fix_24k_record_store_info (struct fix_24k_store_info *stinfo,
			   const struct mips_cl_insn *insn)
{
  /* The instruction must have a known offset.  */
  if (!insn->complete_p || !strstr (insn->insn_mo->args, "o("))
    return false;

  stinfo->off = (insn->insn_opcode >> OP_SH_IMMEDIATE) & OP_MASK_IMMEDIATE;
  stinfo->align_to = fix_24k_align_to (insn->insn_mo);
  return true;
}

/* Return the number of nops that would be needed to work around the 24k
   "lost data on stores during refill" errata if instruction INSN
   immediately followed the 2 instructions described by HIST.
   Ignore hazards that are contained within the first IGNORE
   instructions of HIST.

   Problem: The FSB (fetch store buffer) acts as an intermediate buffer
   for the data cache refills and store data. The following describes
   the scenario where the store data could be lost.

   * A data cache miss, due to either a load or a store, causing fill
     data to be supplied by the memory subsystem
   * The first three doublewords of fill data are returned and written
     into the cache
   * A sequence of four stores occurs in consecutive cycles around the
     final doubleword of the fill:
   * Store A
   * Store B
   * Store C
   * Zero, One or more instructions
   * Store D

   The four stores A-D must be to different doublewords of the line that
   is being filled. The fourth instruction in the sequence above permits
   the fill of the final doubleword to be transferred from the FSB into
   the cache. In the sequence above, the stores may be either integer
   (sb, sh, sw, swr, swl, sc) or coprocessor (swc1/swc2, sdc1/sdc2,
   swxc1, sdxc1, suxc1) stores, as long as the four stores are to
   different doublewords on the line. If the floating point unit is
   running in 1:2 mode, it is not possible to create the sequence above
   using only floating point store instructions.

   In this case, the cache line being filled is incorrectly marked
   invalid, thereby losing the data from any store to the line that
   occurs between the original miss and the completion of the five
   cycle sequence shown above.

   The workarounds are:

   * Run the data cache in write-through mode.
   * Insert a non-store instruction between
     Store A and Store B or Store B and Store C.  */

static int
nops_for_24k (int ignore, const struct mips_cl_insn *hist,
	      const struct mips_cl_insn *insn)
{
  struct fix_24k_store_info pos[3];
  int align, i, base_offset;

  if (ignore >= 2)
    return 0;

  /* If the previous instruction wasn't a store, there's nothing to
     worry about.  */
  if ((hist[0].insn_mo->pinfo & INSN_STORE_MEMORY) == 0)
    return 0;

  /* If the instructions after the previous one are unknown, we have
     to assume the worst.  */
  if (!insn)
    return 1;

  /* Check whether we are dealing with three consecutive stores.  */
  if ((insn->insn_mo->pinfo & INSN_STORE_MEMORY) == 0
      || (hist[1].insn_mo->pinfo & INSN_STORE_MEMORY) == 0)
    return 0;

  /* If we don't know the relationship between the store addresses,
     assume the worst.  */
  if (!BASE_REG_EQ (insn->insn_opcode, hist[0].insn_opcode)
      || !BASE_REG_EQ (insn->insn_opcode, hist[1].insn_opcode))
    return 1;

  if (!fix_24k_record_store_info (&pos[0], insn)
      || !fix_24k_record_store_info (&pos[1], &hist[0])
      || !fix_24k_record_store_info (&pos[2], &hist[1]))
    return 1;

  qsort (&pos, 3, sizeof (struct fix_24k_store_info), fix_24k_sort);

  /* Pick a value of ALIGN and X such that all offsets are adjusted by
     X bytes and such that the base register + X is known to be aligned
     to align bytes.  */

  if (((insn->insn_opcode >> OP_SH_RS) & OP_MASK_RS) == SP)
    align = 8;
  else
    {
      align = pos[0].align_to;
      base_offset = pos[0].off;
      for (i = 1; i < 3; i++)
	if (align < pos[i].align_to)
	  {
	    align = pos[i].align_to;
	    base_offset = pos[i].off;
	  }
      for (i = 0; i < 3; i++)
	pos[i].off -= base_offset;
    }

  pos[0].off &= ~align + 1;
  pos[1].off &= ~align + 1;
  pos[2].off &= ~align + 1;

  /* If any two stores write to the same chunk, they also write to the
     same doubleword.  The offsets are still sorted at this point.  */
  if (pos[0].off == pos[1].off || pos[1].off == pos[2].off)
    return 0;

  /* A range of at least 9 bytes is needed for the stores to be in
     non-overlapping doublewords.  */
  if (pos[2].off - pos[0].off <= 8)
    return 0;

  if (pos[2].off - pos[1].off >= 24
      || pos[1].off - pos[0].off >= 24
      || pos[2].off - pos[0].off >= 32)
    return 0;

  return 1;
}

/* Return the number of nops that would be needed if instruction INSN
   immediately followed the MAX_NOPS instructions given by HIST,
   where HIST[0] is the most recent instruction.  Ignore hazards
   between INSN and the first IGNORE instructions in HIST.

   If INSN is null, return the worse-case number of nops for any
   instruction.  */

static int
nops_for_insn (int ignore, const struct mips_cl_insn *hist,
	       const struct mips_cl_insn *insn)
{
  int i, nops, tmp_nops;

  nops = 0;
  for (i = ignore; i < MAX_DELAY_NOPS; i++)
    {
      tmp_nops = insns_between (hist + i, insn) - i;
      if (tmp_nops > nops)
	nops = tmp_nops;
    }

  if (mips_fix_vr4130 && !mips_opts.micromips)
    {
      tmp_nops = nops_for_vr4130 (ignore, hist, insn);
      if (tmp_nops > nops)
	nops = tmp_nops;
    }

  if (mips_fix_24k && !mips_opts.micromips)
    {
      tmp_nops = nops_for_24k (ignore, hist, insn);
      if (tmp_nops > nops)
	nops = tmp_nops;
    }

  return nops;
}

/* The variable arguments provide NUM_INSNS extra instructions that
   might be added to HIST.  Return the largest number of nops that
   would be needed after the extended sequence, ignoring hazards
   in the first IGNORE instructions.  */

static int
nops_for_sequence (int num_insns, int ignore,
		   const struct mips_cl_insn *hist, ...)
{
  va_list args;
  struct mips_cl_insn buffer[MAX_NOPS];
  struct mips_cl_insn *cursor;
  int nops;

  va_start (args, hist);
  cursor = buffer + num_insns;
  memcpy (cursor, hist, (MAX_NOPS - num_insns) * sizeof (*cursor));
  while (cursor > buffer)
    *--cursor = *va_arg (args, const struct mips_cl_insn *);

  nops = nops_for_insn (ignore, buffer, NULL);
  va_end (args);
  return nops;
}

/* Like nops_for_insn, but if INSN is a branch, take into account the
   worst-case delay for the branch target.  */

static int
nops_for_insn_or_target (int ignore, const struct mips_cl_insn *hist,
			 const struct mips_cl_insn *insn)
{
  int nops, tmp_nops;

  nops = nops_for_insn (ignore, hist, insn);
  if (delayed_branch_p (insn))
    {
      tmp_nops = nops_for_sequence (2, ignore ? ignore + 2 : 0,
				    hist, insn, get_delay_slot_nop (insn));
      if (tmp_nops > nops)
	nops = tmp_nops;
    }
  else if (compact_branch_p (insn))
    {
      tmp_nops = nops_for_sequence (1, ignore ? ignore + 1 : 0, hist, insn);
      if (tmp_nops > nops)
	nops = tmp_nops;
    }
  return nops;
}

/* Fix NOP issue: Replace nops by "or at,at,zero".  */

static void
fix_loongson2f_nop (struct mips_cl_insn * ip)
{
  gas_assert (!HAVE_CODE_COMPRESSION);
  if (strcmp (ip->insn_mo->name, "nop") == 0)
    ip->insn_opcode = LOONGSON2F_NOP_INSN;
}

/* Fix Jump Issue: Eliminate instruction fetch from outside 256M region
                   jr target pc &= 'hffff_ffff_cfff_ffff.  */

static void
fix_loongson2f_jump (struct mips_cl_insn * ip)
{
  gas_assert (!HAVE_CODE_COMPRESSION);
  if (strcmp (ip->insn_mo->name, "j") == 0
      || strcmp (ip->insn_mo->name, "jr") == 0
      || strcmp (ip->insn_mo->name, "jalr") == 0)
    {
      int sreg;
      expressionS ep;

      if (! mips_opts.at)
        return;

      sreg = EXTRACT_OPERAND (0, RS, *ip);
      if (sreg == ZERO || sreg == KT0 || sreg == KT1 || sreg == ATREG)
        return;

      ep.X_op = O_constant;
      ep.X_add_number = 0xcfff0000;
      macro_build (&ep, "lui", "t,u", ATREG, BFD_RELOC_HI16);
      ep.X_add_number = 0xffff;
      macro_build (&ep, "ori", "t,r,i", ATREG, ATREG, BFD_RELOC_LO16);
      macro_build (NULL, "and", "d,v,t", sreg, sreg, ATREG);
    }
}

static void
fix_loongson2f (struct mips_cl_insn * ip)
{
  if (mips_fix_loongson2f_nop)
    fix_loongson2f_nop (ip);

  if (mips_fix_loongson2f_jump)
    fix_loongson2f_jump (ip);
}

static bool
has_label_name (const char *arr[], size_t len ,const char *s)
{
  unsigned long i;
  for (i = 0; i < len; i++)
    {
      if (!arr[i])
	return false;
      if (streq (arr[i], s))
	return true;
    }
  return false;
}

/* Fix loongson3 llsc errata: Insert sync before ll/lld.  */

static void
fix_loongson3_llsc (struct mips_cl_insn * ip)
{
  gas_assert (!HAVE_CODE_COMPRESSION);

  /* If is an local label and the insn is not sync,
     look forward that whether an branch between ll/sc jump to here
     if so, insert a sync.  */
  if (seg_info (now_seg)->label_list
      && S_IS_LOCAL (seg_info (now_seg)->label_list->label)
      && (strcmp (ip->insn_mo->name, "sync") != 0))
    {
      unsigned long i;
      valueT label_value;
      const char *label_names[MAX_LABELS_SAME];
      const char *label_name;

      label_name = S_GET_NAME (seg_info (now_seg)->label_list->label);
      label_names[0] = label_name;
      struct insn_label_list *llist = seg_info (now_seg)->label_list;
      label_value = S_GET_VALUE (llist->label);

      for (i = 1; i < MAX_LABELS_SAME; i++)
	{
	  llist = llist->next;
	  if (!llist)
	    break;
	  if (S_GET_VALUE (llist->label) == label_value)
	    label_names[i] = S_GET_NAME (llist->label);
	  else
	    break;
	}
      for (; i < MAX_LABELS_SAME; i++)
	label_names[i] = NULL;

      unsigned long lookback = ARRAY_SIZE (history);
      for (i = 0; i < lookback; i++)
	{
	  if (streq (history[i].insn_mo->name, "sc")
	      || streq (history[i].insn_mo->name, "scd"))
	    {
	      unsigned long j;

	      for (j = i + 1; j < lookback; j++)
		{
		  if (streq (history[j].insn_mo->name, "ll")
		      || streq (history[j].insn_mo->name, "lld"))
		    break;

		  if (delayed_branch_p (&history[j]))
		    {
		      if (has_label_name (label_names,
					  MAX_LABELS_SAME,
					  history[j].target))
			{
			  add_fixed_insn (&sync_insn);
			  insert_into_history (0, 1, &sync_insn);
			  i = lookback;
			  break;
			}
		    }
		}
	    }
	}
    }
  /* If we find a sc, we look forward to look for an branch insn,
     and see whether it jump back and out of ll/sc.  */
  else if (streq (ip->insn_mo->name, "sc") || streq (ip->insn_mo->name, "scd"))
    {
      unsigned long lookback = ARRAY_SIZE (history) - 1;
      unsigned long i;

      for (i = 0; i < lookback; i++)
	{
	  if (streq (history[i].insn_mo->name, "ll")
	      || streq (history[i].insn_mo->name, "lld"))
	    break;

	  if (delayed_branch_p (&history[i]))
	    {
	      unsigned long j;

	      for (j = i + 1; j < lookback; j++)
		{
		  if (streq (history[j].insn_mo->name, "ll")
		      || streq (history[j].insn_mo->name, "lld"))
		    break;
		}

	      for (; j < lookback; j++)
		{
		  if (history[j].label[0] != '\0'
		      && streq (history[j].label, history[i].target)
		      && strcmp (history[j+1].insn_mo->name, "sync") != 0)
		    {
		      add_fixed_insn (&sync_insn);
		      insert_into_history (++j, 1, &sync_insn);
		    }
		}
	    }
	}
    }

  /* Skip if there is a sync before ll/lld.  */
  if ((strcmp (ip->insn_mo->name, "ll") == 0
       || strcmp (ip->insn_mo->name, "lld") == 0)
      && (strcmp (history[0].insn_mo->name, "sync") != 0))
    {
      add_fixed_insn (&sync_insn);
      insert_into_history (0, 1, &sync_insn);
    }
}

/* IP is a branch that has a delay slot, and we need to fill it
   automatically.   Return true if we can do that by swapping IP
   with the previous instruction.
   ADDRESS_EXPR is an operand of the instruction to be used with
   RELOC_TYPE.  */

static bool
can_swap_branch_p (struct mips_cl_insn *ip, expressionS *address_expr,
		   bfd_reloc_code_real_type *reloc_type)
{
  unsigned long pinfo, pinfo2, prev_pinfo, prev_pinfo2;
  unsigned int gpr_read, gpr_write, prev_gpr_read, prev_gpr_write;
  unsigned int fpr_read, prev_fpr_write;

  /* -O2 and above is required for this optimization.  */
  if (mips_optimize < 2)
    return false;

  /* If we have seen .set volatile or .set nomove, don't optimize.  */
  if (mips_opts.nomove)
    return false;

  /* We can't swap if the previous instruction's position is fixed.  */
  if (history[0].fixed_p)
    return false;

  /* If the previous previous insn was in a .set noreorder, we can't
     swap.  Actually, the MIPS assembler will swap in this situation.
     However, gcc configured -with-gnu-as will generate code like

	.set	noreorder
	lw	$4,XXX
	.set	reorder
	INSN
	bne	$4,$0,foo

     in which we can not swap the bne and INSN.  If gcc is not configured
     -with-gnu-as, it does not output the .set pseudo-ops.  */
  if (history[1].noreorder_p)
    return false;

  /* If the previous instruction had a fixup in mips16 mode, we can not swap.
     This means that the previous instruction was a 4-byte one anyhow.  */
  if (mips_opts.mips16 && history[0].fixp[0])
    return false;

  /* If the branch is itself the target of a branch, we can not swap.
     We cheat on this; all we check for is whether there is a label on
     this instruction.  If there are any branches to anything other than
     a label, users must use .set noreorder.  */
  if (seg_info (now_seg)->label_list)
    return false;

  /* If the previous instruction is in a variant frag other than this
     branch's one, we cannot do the swap.  This does not apply to
     MIPS16 code, which uses variant frags for different purposes.  */
  if (!mips_opts.mips16
      && history[0].frag
      && history[0].frag->fr_type == rs_machine_dependent)
    return false;

  /* We do not swap with instructions that cannot architecturally
     be placed in a branch delay slot, such as SYNC or ERET.  We
     also refrain from swapping with a trap instruction, since it
     complicates trap handlers to have the trap instruction be in
     a delay slot.  */
  prev_pinfo = history[0].insn_mo->pinfo;
  if (prev_pinfo & INSN_NO_DELAY_SLOT)
    return false;

  /* Check for conflicts between the branch and the instructions
     before the candidate delay slot.  */
  if (nops_for_insn (0, history + 1, ip) > 0)
    return false;

  /* Check for conflicts between the swapped sequence and the
     target of the branch.  */
  if (nops_for_sequence (2, 0, history + 1, ip, history) > 0)
    return false;

  /* If the branch reads a register that the previous
     instruction sets, we can not swap.  */
  gpr_read = gpr_read_mask (ip);
  prev_gpr_write = gpr_write_mask (&history[0]);
  if (gpr_read & prev_gpr_write)
    return false;

  fpr_read = fpr_read_mask (ip);
  prev_fpr_write = fpr_write_mask (&history[0]);
  if (fpr_read & prev_fpr_write)
    return false;

  /* If the branch writes a register that the previous
     instruction sets, we can not swap.  */
  gpr_write = gpr_write_mask (ip);
  if (gpr_write & prev_gpr_write)
    return false;

  /* If the branch writes a register that the previous
     instruction reads, we can not swap.  */
  prev_gpr_read = gpr_read_mask (&history[0]);
  if (gpr_write & prev_gpr_read)
    return false;

  /* If one instruction sets a condition code and the
     other one uses a condition code, we can not swap.  */
  pinfo = ip->insn_mo->pinfo;
  if ((pinfo & INSN_READ_COND_CODE)
      && (prev_pinfo & INSN_WRITE_COND_CODE))
    return false;
  if ((pinfo & INSN_WRITE_COND_CODE)
      && (prev_pinfo & INSN_READ_COND_CODE))
    return false;

  /* If the previous instruction uses the PC, we can not swap.  */
  prev_pinfo2 = history[0].insn_mo->pinfo2;
  if (prev_pinfo2 & INSN2_READ_PC)
    return false;

  /* If the previous instruction has an incorrect size for a fixed
     branch delay slot in microMIPS mode, we cannot swap.  */
  pinfo2 = ip->insn_mo->pinfo2;
  if (mips_opts.micromips
      && (pinfo2 & INSN2_BRANCH_DELAY_16BIT)
      && insn_length (history) != 2)
    return false;
  if (mips_opts.micromips
      && (pinfo2 & INSN2_BRANCH_DELAY_32BIT)
      && insn_length (history) != 4)
    return false;

  /* On the R5900 short loops need to be fixed by inserting a NOP in the
     branch delay slot.

     The short loop bug under certain conditions causes loops to execute
     only once or twice.  We must ensure that the assembler never
     generates loops that satisfy all of the following conditions:

     - a loop consists of less than or equal to six instructions
       (including the branch delay slot);
     - a loop contains only one conditional branch instruction at the end
       of the loop;
     - a loop does not contain any other branch or jump instructions;
     - a branch delay slot of the loop is not NOP (EE 2.9 or later).

     We need to do this because of a hardware bug in the R5900 chip.  */
  if (mips_fix_r5900
      /* Check if instruction has a parameter, ignore "j $31". */
      && (address_expr != NULL)
      /* Parameter must be 16 bit. */
      && (*reloc_type == BFD_RELOC_16_PCREL_S2)
      /* Branch to same segment. */
      && (S_GET_SEGMENT (address_expr->X_add_symbol) == now_seg)
      /* Branch to same code fragment. */
      && (symbol_get_frag (address_expr->X_add_symbol) == frag_now)
      /* Can only calculate branch offset if value is known. */
      && symbol_constant_p (address_expr->X_add_symbol)
      /* Check if branch is really conditional. */
      && !((ip->insn_opcode & 0xffff0000) == 0x10000000   /* beq $0,$0 */
	|| (ip->insn_opcode & 0xffff0000) == 0x04010000   /* bgez $0 */
	|| (ip->insn_opcode & 0xffff0000) == 0x04110000)) /* bgezal $0 */
    {
      int distance;
      /* Check if loop is shorter than or equal to 6 instructions
         including branch and delay slot.  */
      distance = frag_now_fix () - S_GET_VALUE (address_expr->X_add_symbol);
      if (distance <= 20)
        {
          int i;
          int rv;

          rv = false;
          /* When the loop includes branches or jumps,
             it is not a short loop. */
          for (i = 0; i < (distance / 4); i++)
            {
              if ((history[i].cleared_p)
                  || delayed_branch_p (&history[i]))
                {
                  rv = true;
                  break;
                }
            }
          if (!rv)
            {
              /* Insert nop after branch to fix short loop. */
              return false;
            }
        }
    }

  return true;
}

/* Decide how we should add IP to the instruction stream.
   ADDRESS_EXPR is an operand of the instruction to be used with
   RELOC_TYPE.  */

static enum append_method
get_append_method (struct mips_cl_insn *ip, expressionS *address_expr,
		   bfd_reloc_code_real_type *reloc_type)
{
  /* The relaxed version of a macro sequence must be inherently
     hazard-free.  */
  if (mips_relax.sequence == 2)
    return APPEND_ADD;

  /* We must not dabble with instructions in a ".set noreorder" block.  */
  if (mips_opts.noreorder)
    return APPEND_ADD;

  /* Otherwise, it's our responsibility to fill branch delay slots.  */
  if (delayed_branch_p (ip))
    {
      if (!branch_likely_p (ip)
	  && can_swap_branch_p (ip, address_expr, reloc_type))
	return APPEND_SWAP;

      if (mips_opts.mips16
	  && ISA_SUPPORTS_MIPS16E
	  && gpr_read_mask (ip) != 0)
	return APPEND_ADD_COMPACT;

      if (mips_opts.micromips
	  && ((ip->insn_opcode & 0xffe0) == 0x4580
	      || (!forced_insn_length
		  && ((ip->insn_opcode & 0xfc00) == 0xcc00
		      || (ip->insn_opcode & 0xdc00) == 0x8c00))
	      || (ip->insn_opcode & 0xdfe00000) == 0x94000000
	      || (ip->insn_opcode & 0xdc1f0000) == 0x94000000))
	return APPEND_ADD_COMPACT;

      return APPEND_ADD_WITH_NOP;
    }

  return APPEND_ADD;
}

/* IP is an instruction whose opcode we have just changed, END points
   to the end of the opcode table processed.  Point IP->insn_mo to the
   new opcode's definition.  */

static void
find_altered_opcode (struct mips_cl_insn *ip, const struct mips_opcode *end)
{
  const struct mips_opcode *mo;

  for (mo = ip->insn_mo; mo < end; mo++)
    if (mo->pinfo != INSN_MACRO
	&& (ip->insn_opcode & mo->mask) == mo->match)
      {
	ip->insn_mo = mo;
	return;
      }
  abort ();
}

/* IP is a MIPS16 instruction whose opcode we have just changed.
   Point IP->insn_mo to the new opcode's definition.  */

static void
find_altered_mips16_opcode (struct mips_cl_insn *ip)
{
  find_altered_opcode (ip, &mips16_opcodes[bfd_mips16_num_opcodes]);
}

/* IP is a microMIPS instruction whose opcode we have just changed.
   Point IP->insn_mo to the new opcode's definition.  */

static void
find_altered_micromips_opcode (struct mips_cl_insn *ip)
{
  find_altered_opcode (ip, &micromips_opcodes[bfd_micromips_num_opcodes]);
}

/* For microMIPS macros, we need to generate a local number label
   as the target of branches.  */
#define MICROMIPS_LABEL_CHAR		'\037'
static unsigned long micromips_target_label;
static char micromips_target_name[32];

static char *
micromips_label_name (void)
{
  char *p = micromips_target_name;
  char symbol_name_temporary[24];
  unsigned long l;
  int i;

  if (*p)
    return p;

  i = 0;
  l = micromips_target_label;
#ifdef LOCAL_LABEL_PREFIX
  *p++ = LOCAL_LABEL_PREFIX;
#endif
  *p++ = 'L';
  *p++ = MICROMIPS_LABEL_CHAR;
  do
    {
      symbol_name_temporary[i++] = l % 10 + '0';
      l /= 10;
    }
  while (l != 0);
  while (i > 0)
    *p++ = symbol_name_temporary[--i];
  *p = '\0';

  return micromips_target_name;
}

static void
micromips_label_expr (expressionS *label_expr)
{
  label_expr->X_op = O_symbol;
  label_expr->X_add_symbol = symbol_find_or_make (micromips_label_name ());
  label_expr->X_add_number = 0;
}

static void
micromips_label_inc (void)
{
  micromips_target_label++;
  *micromips_target_name = '\0';
}

static void
micromips_add_label (void)
{
  symbolS *s;

  s = colon (micromips_label_name ());
  micromips_label_inc ();
  S_SET_OTHER (s, ELF_ST_SET_MICROMIPS (S_GET_OTHER (s)));
}

/* If assembling microMIPS code, then return the microMIPS reloc
   corresponding to the requested one if any.  Otherwise return
   the reloc unchanged.  */

static bfd_reloc_code_real_type
micromips_map_reloc (bfd_reloc_code_real_type reloc)
{
  static const bfd_reloc_code_real_type relocs[][2] =
    {
      /* Keep sorted incrementally by the left-hand key.  */
      { BFD_RELOC_16_PCREL_S2, BFD_RELOC_MICROMIPS_16_PCREL_S1 },
      { BFD_RELOC_GPREL16, BFD_RELOC_MICROMIPS_GPREL16 },
      { BFD_RELOC_MIPS_JMP, BFD_RELOC_MICROMIPS_JMP },
      { BFD_RELOC_HI16, BFD_RELOC_MICROMIPS_HI16 },
      { BFD_RELOC_HI16_S, BFD_RELOC_MICROMIPS_HI16_S },
      { BFD_RELOC_LO16, BFD_RELOC_MICROMIPS_LO16 },
      { BFD_RELOC_MIPS_LITERAL, BFD_RELOC_MICROMIPS_LITERAL },
      { BFD_RELOC_MIPS_GOT16, BFD_RELOC_MICROMIPS_GOT16 },
      { BFD_RELOC_MIPS_CALL16, BFD_RELOC_MICROMIPS_CALL16 },
      { BFD_RELOC_MIPS_GOT_HI16, BFD_RELOC_MICROMIPS_GOT_HI16 },
      { BFD_RELOC_MIPS_GOT_LO16, BFD_RELOC_MICROMIPS_GOT_LO16 },
      { BFD_RELOC_MIPS_CALL_HI16, BFD_RELOC_MICROMIPS_CALL_HI16 },
      { BFD_RELOC_MIPS_CALL_LO16, BFD_RELOC_MICROMIPS_CALL_LO16 },
      { BFD_RELOC_MIPS_SUB, BFD_RELOC_MICROMIPS_SUB },
      { BFD_RELOC_MIPS_GOT_PAGE, BFD_RELOC_MICROMIPS_GOT_PAGE },
      { BFD_RELOC_MIPS_GOT_OFST, BFD_RELOC_MICROMIPS_GOT_OFST },
      { BFD_RELOC_MIPS_GOT_DISP, BFD_RELOC_MICROMIPS_GOT_DISP },
      { BFD_RELOC_MIPS_HIGHEST, BFD_RELOC_MICROMIPS_HIGHEST },
      { BFD_RELOC_MIPS_HIGHER, BFD_RELOC_MICROMIPS_HIGHER },
      { BFD_RELOC_MIPS_SCN_DISP, BFD_RELOC_MICROMIPS_SCN_DISP },
      { BFD_RELOC_MIPS_TLS_GD, BFD_RELOC_MICROMIPS_TLS_GD },
      { BFD_RELOC_MIPS_TLS_LDM, BFD_RELOC_MICROMIPS_TLS_LDM },
      { BFD_RELOC_MIPS_TLS_DTPREL_HI16, BFD_RELOC_MICROMIPS_TLS_DTPREL_HI16 },
      { BFD_RELOC_MIPS_TLS_DTPREL_LO16, BFD_RELOC_MICROMIPS_TLS_DTPREL_LO16 },
      { BFD_RELOC_MIPS_TLS_GOTTPREL, BFD_RELOC_MICROMIPS_TLS_GOTTPREL },
      { BFD_RELOC_MIPS_TLS_TPREL_HI16, BFD_RELOC_MICROMIPS_TLS_TPREL_HI16 },
      { BFD_RELOC_MIPS_TLS_TPREL_LO16, BFD_RELOC_MICROMIPS_TLS_TPREL_LO16 }
    };
  bfd_reloc_code_real_type r;
  size_t i;

  if (!mips_opts.micromips)
    return reloc;
  for (i = 0; i < ARRAY_SIZE (relocs); i++)
    {
      r = relocs[i][0];
      if (r > reloc)
	return reloc;
      if (r == reloc)
	return relocs[i][1];
    }
  return reloc;
}

/* Try to resolve relocation RELOC against constant OPERAND at assembly time.
   Return true on success, storing the resolved value in RESULT.  */

static bool
calculate_reloc (bfd_reloc_code_real_type reloc, offsetT operand,
		 offsetT *result)
{
  switch (reloc)
    {
    case BFD_RELOC_MIPS_HIGHEST:
    case BFD_RELOC_MICROMIPS_HIGHEST:
      *result = ((operand + 0x800080008000ull) >> 48) & 0xffff;
      return true;

    case BFD_RELOC_MIPS_HIGHER:
    case BFD_RELOC_MICROMIPS_HIGHER:
      *result = ((operand + 0x80008000ull) >> 32) & 0xffff;
      return true;

    case BFD_RELOC_HI16_S:
    case BFD_RELOC_HI16_S_PCREL:
    case BFD_RELOC_MICROMIPS_HI16_S:
    case BFD_RELOC_MIPS16_HI16_S:
      *result = ((operand + 0x8000) >> 16) & 0xffff;
      return true;

    case BFD_RELOC_HI16:
    case BFD_RELOC_MICROMIPS_HI16:
    case BFD_RELOC_MIPS16_HI16:
      *result = (operand >> 16) & 0xffff;
      return true;

    case BFD_RELOC_LO16:
    case BFD_RELOC_LO16_PCREL:
    case BFD_RELOC_MICROMIPS_LO16:
    case BFD_RELOC_MIPS16_LO16:
      *result = operand & 0xffff;
      return true;

    case BFD_RELOC_UNUSED:
      *result = operand;
      return true;

    default:
      return false;
    }
}

/* Output an instruction.  IP is the instruction information.
   ADDRESS_EXPR is an operand of the instruction to be used with
   RELOC_TYPE.  EXPANSIONP is true if the instruction is part of
   a macro expansion.  */

static void
append_insn (struct mips_cl_insn *ip, expressionS *address_expr,
	     bfd_reloc_code_real_type *reloc_type, bool expansionp)
{
  unsigned long prev_pinfo2, pinfo;
  bool relaxed_branch = false;
  enum append_method method;
  bool relax32;
  int branch_disp;

  if (mips_fix_loongson2f && !HAVE_CODE_COMPRESSION)
    fix_loongson2f (ip);

  ip->target[0] = '\0';
  if (offset_expr.X_op == O_symbol)
    strncpy (ip->target, S_GET_NAME (offset_expr.X_add_symbol), 15);
  ip->label[0] = '\0';
  if (seg_info (now_seg)->label_list)
    strncpy (ip->label, S_GET_NAME (seg_info (now_seg)->label_list->label), 15);
  if (mips_fix_loongson3_llsc && !HAVE_CODE_COMPRESSION)
    fix_loongson3_llsc (ip);

  file_ase_mips16 |= mips_opts.mips16;
  file_ase_micromips |= mips_opts.micromips;

  prev_pinfo2 = history[0].insn_mo->pinfo2;
  pinfo = ip->insn_mo->pinfo;

  /* Don't raise alarm about `nods' frags as they'll fill in the right
     kind of nop in relaxation if required.  */
  if (mips_opts.micromips
      && !expansionp
      && !(history[0].frag
	   && history[0].frag->fr_type == rs_machine_dependent
	   && RELAX_MICROMIPS_P (history[0].frag->fr_subtype)
	   && RELAX_MICROMIPS_NODS (history[0].frag->fr_subtype))
      && (((prev_pinfo2 & INSN2_BRANCH_DELAY_16BIT) != 0
	   && micromips_insn_length (ip->insn_mo) != 2)
	  || ((prev_pinfo2 & INSN2_BRANCH_DELAY_32BIT) != 0
	      && micromips_insn_length (ip->insn_mo) != 4)))
    as_warn (_("wrong size instruction in a %u-bit branch delay slot"),
	     (prev_pinfo2 & INSN2_BRANCH_DELAY_16BIT) != 0 ? 16 : 32);

  if (address_expr == NULL)
    ip->complete_p = 1;
  else if (reloc_type[0] <= BFD_RELOC_UNUSED
	   && reloc_type[1] == BFD_RELOC_UNUSED
	   && reloc_type[2] == BFD_RELOC_UNUSED
	   && address_expr->X_op == O_constant)
    {
      switch (*reloc_type)
	{
	case BFD_RELOC_MIPS_JMP:
	  {
	    int shift;

	    /* Shift is 2, unusually, for microMIPS JALX.  */
	    shift = (mips_opts.micromips
		     && strcmp (ip->insn_mo->name, "jalx") != 0) ? 1 : 2;
	    if ((address_expr->X_add_number & ((1 << shift) - 1)) != 0)
	      as_bad (_("jump to misaligned address (0x%lx)"),
		      (unsigned long) address_expr->X_add_number);
	    ip->insn_opcode |= ((address_expr->X_add_number >> shift)
				& 0x3ffffff);
	    ip->complete_p = 1;
	  }
	  break;

	case BFD_RELOC_MIPS16_JMP:
	  if ((address_expr->X_add_number & 3) != 0)
	    as_bad (_("jump to misaligned address (0x%lx)"),
	            (unsigned long) address_expr->X_add_number);
	  ip->insn_opcode |=
	    (((address_expr->X_add_number & 0x7c0000) << 3)
	       | ((address_expr->X_add_number & 0xf800000) >> 7)
	       | ((address_expr->X_add_number & 0x3fffc) >> 2));
	  ip->complete_p = 1;
	  break;

	case BFD_RELOC_16_PCREL_S2:
	  {
	    int shift;

	    shift = mips_opts.micromips ? 1 : 2;
	    if ((address_expr->X_add_number & ((1 << shift) - 1)) != 0)
	      as_bad (_("branch to misaligned address (0x%lx)"),
		      (unsigned long) address_expr->X_add_number);
	    if (!mips_relax_branch)
	      {
		if ((address_expr->X_add_number + (1 << (shift + 15)))
		    & ~((1 << (shift + 16)) - 1))
		  as_bad (_("branch address range overflow (0x%lx)"),
			  (unsigned long) address_expr->X_add_number);
		ip->insn_opcode |= ((address_expr->X_add_number >> shift)
				    & 0xffff);
	      }
	  }
	  break;

	case BFD_RELOC_MIPS_21_PCREL_S2:
	  {
	    int shift;

	    shift = 2;
	    if ((address_expr->X_add_number & ((1 << shift) - 1)) != 0)
	      as_bad (_("branch to misaligned address (0x%lx)"),
		      (unsigned long) address_expr->X_add_number);
	    if ((address_expr->X_add_number + (1 << (shift + 20)))
		& ~((1 << (shift + 21)) - 1))
	      as_bad (_("branch address range overflow (0x%lx)"),
		      (unsigned long) address_expr->X_add_number);
	    ip->insn_opcode |= ((address_expr->X_add_number >> shift)
				& 0x1fffff);
	  }
	  break;

	case BFD_RELOC_MIPS_26_PCREL_S2:
	  {
	    int shift;

	    shift = 2;
	    if ((address_expr->X_add_number & ((1 << shift) - 1)) != 0)
	      as_bad (_("branch to misaligned address (0x%lx)"),
		      (unsigned long) address_expr->X_add_number);
	    if ((address_expr->X_add_number + (1 << (shift + 25)))
		& ~((1 << (shift + 26)) - 1))
	      as_bad (_("branch address range overflow (0x%lx)"),
		      (unsigned long) address_expr->X_add_number);
	    ip->insn_opcode |= ((address_expr->X_add_number >> shift)
				& 0x3ffffff);
	  }
	  break;

	default:
	  {
	    offsetT value;

	    if (calculate_reloc (*reloc_type, address_expr->X_add_number,
				 &value))
	      {
		ip->insn_opcode |= value & 0xffff;
		ip->complete_p = 1;
	      }
	  }
	  break;
	}
    }

  if (mips_relax.sequence != 2 && !mips_opts.noreorder)
    {
      /* There are a lot of optimizations we could do that we don't.
	 In particular, we do not, in general, reorder instructions.
	 If you use gcc with optimization, it will reorder
	 instructions and generally do much more optimization then we
	 do here; repeating all that work in the assembler would only
	 benefit hand written assembly code, and does not seem worth
	 it.  */
      int nops = (mips_optimize == 0
		  ? nops_for_insn (0, history, NULL)
		  : nops_for_insn_or_target (0, history, ip));
      if (nops > 0)
	{
	  fragS *old_frag;
	  unsigned long old_frag_offset;
	  int i;

	  old_frag = frag_now;
	  old_frag_offset = frag_now_fix ();

	  for (i = 0; i < nops; i++)
	    add_fixed_insn (NOP_INSN);
	  insert_into_history (0, nops, NOP_INSN);

	  if (listing)
	    {
	      listing_prev_line ();
	      /* We may be at the start of a variant frag.  In case we
                 are, make sure there is enough space for the frag
                 after the frags created by listing_prev_line.  The
                 argument to frag_grow here must be at least as large
                 as the argument to all other calls to frag_grow in
                 this file.  We don't have to worry about being in the
                 middle of a variant frag, because the variants insert
                 all needed nop instructions themselves.  */
	      frag_grow (40);
	    }

	  mips_move_text_labels ();

#ifndef NO_ECOFF_DEBUGGING
	  if (ECOFF_DEBUGGING)
	    ecoff_fix_loc (old_frag, old_frag_offset);
#endif
	}
    }
  else if (mips_relax.sequence != 2 && prev_nop_frag != NULL)
    {
      int nops;

      /* Work out how many nops in prev_nop_frag are needed by IP,
	 ignoring hazards generated by the first prev_nop_frag_since
	 instructions.  */
      nops = nops_for_insn_or_target (prev_nop_frag_since, history, ip);
      gas_assert (nops <= prev_nop_frag_holds);

      /* Enforce NOPS as a minimum.  */
      if (nops > prev_nop_frag_required)
	prev_nop_frag_required = nops;

      if (prev_nop_frag_holds == prev_nop_frag_required)
	{
	  /* Settle for the current number of nops.  Update the history
	     accordingly (for the benefit of any future .set reorder code).  */
	  prev_nop_frag = NULL;
	  insert_into_history (prev_nop_frag_since,
			       prev_nop_frag_holds, NOP_INSN);
	}
      else
	{
	  /* Allow this instruction to replace one of the nops that was
	     tentatively added to prev_nop_frag.  */
	  prev_nop_frag->fr_fix -= NOP_INSN_SIZE;
	  prev_nop_frag_holds--;
	  prev_nop_frag_since++;
	}
    }

  method = get_append_method (ip, address_expr, reloc_type);
  branch_disp = method == APPEND_SWAP ? insn_length (history) : 0;

  dwarf2_emit_insn (0);
  /* We want MIPS16 and microMIPS debug info to use ISA-encoded addresses,
     so "move" the instruction address accordingly.

     Also, it doesn't seem appropriate for the assembler to reorder .loc
     entries.  If this instruction is a branch that we are going to swap
     with the previous instruction, the two instructions should be
     treated as a unit, and the debug information for both instructions
     should refer to the start of the branch sequence.  Using the
     current position is certainly wrong when swapping a 32-bit branch
     and a 16-bit delay slot, since the current position would then be
     in the middle of a branch.  */
  dwarf2_move_insn ((HAVE_CODE_COMPRESSION ? 1 : 0) - branch_disp);

  relax32 = (mips_relax_branch
	     /* Don't try branch relaxation within .set nomacro, or within
	        .set noat if we use $at for PIC computations.  If it turns
	        out that the branch was out-of-range, we'll get an error.  */
	     && !mips_opts.warn_about_macros
	     && (mips_opts.at || mips_pic == NO_PIC)
	     /* Don't relax BPOSGE32/64 or BC1ANY2T/F and BC1ANY4T/F
	        as they have no complementing branches.  */
	     && !(ip->insn_mo->ase & (ASE_MIPS3D | ASE_DSP64 | ASE_DSP)));

  if (!HAVE_CODE_COMPRESSION
      && address_expr
      && relax32
      && *reloc_type == BFD_RELOC_16_PCREL_S2
      && delayed_branch_p (ip))
    {
      relaxed_branch = true;
      add_relaxed_insn (ip, (relaxed_branch_length
			     (NULL, NULL,
			      uncond_branch_p (ip) ? -1
			      : branch_likely_p (ip) ? 1
			      : 0)), 4,
			RELAX_BRANCH_ENCODE
			(AT, mips_pic != NO_PIC,
			 uncond_branch_p (ip),
			 branch_likely_p (ip),
			 pinfo & INSN_WRITE_GPR_31,
			 0),
			address_expr->X_add_symbol,
			address_expr->X_add_number);
      *reloc_type = BFD_RELOC_UNUSED;
    }
  else if (mips_opts.micromips
	   && address_expr
	   && ((relax32 && *reloc_type == BFD_RELOC_16_PCREL_S2)
	       || *reloc_type > BFD_RELOC_UNUSED)
	   && (delayed_branch_p (ip) || compact_branch_p (ip))
	   /* Don't try branch relaxation when users specify
	      16-bit/32-bit instructions.  */
	   && !forced_insn_length)
    {
      bool relax16 = (method != APPEND_ADD_COMPACT
		      && *reloc_type > BFD_RELOC_UNUSED);
      int type = relax16 ? *reloc_type - BFD_RELOC_UNUSED : 0;
      int uncond = uncond_branch_p (ip) ? -1 : 0;
      int compact = compact_branch_p (ip) || method == APPEND_ADD_COMPACT;
      int nods = method == APPEND_ADD_WITH_NOP;
      int al = pinfo & INSN_WRITE_GPR_31;
      int length32 = nods ? 8 : 4;

      gas_assert (address_expr != NULL);
      gas_assert (!mips_relax.sequence);

      relaxed_branch = true;
      if (nods)
	method = APPEND_ADD;
      if (relax32)
	length32 = relaxed_micromips_32bit_branch_length (NULL, NULL, uncond);
      add_relaxed_insn (ip, length32, relax16 ? 2 : 4,
			RELAX_MICROMIPS_ENCODE (type, AT, mips_opts.insn32,
						mips_pic != NO_PIC,
						uncond, compact, al, nods,
						relax32, 0, 0),
			address_expr->X_add_symbol,
			address_expr->X_add_number);
      *reloc_type = BFD_RELOC_UNUSED;
    }
  else if (mips_opts.mips16 && *reloc_type > BFD_RELOC_UNUSED)
    {
      bool require_unextended;
      bool require_extended;
      symbolS *symbol;
      offsetT offset;

      if (forced_insn_length != 0)
	{
	  require_unextended = forced_insn_length == 2;
	  require_extended = forced_insn_length == 4;
	}
      else
	{
	  require_unextended = (mips_opts.noautoextend
				&& !mips_opcode_32bit_p (ip->insn_mo));
	  require_extended = 0;
	}

      /* We need to set up a variant frag.  */
      gas_assert (address_expr != NULL);
      /* Pass any `O_symbol' expression unchanged as an `expr_section'
         symbol created by `make_expr_symbol' may not get a necessary
         external relocation produced.  */
      if (address_expr->X_op == O_symbol)
	{
	  symbol = address_expr->X_add_symbol;
	  offset = address_expr->X_add_number;
	}
      else
	{
	  symbol = make_expr_symbol (address_expr);
	  symbol_append (symbol, symbol_lastP, &symbol_rootP, &symbol_lastP);
	  offset = 0;
	}
      add_relaxed_insn (ip, 12, 0,
			RELAX_MIPS16_ENCODE
			(*reloc_type - BFD_RELOC_UNUSED,
			 mips_opts.ase & ASE_MIPS16E2,
			 mips_pic != NO_PIC,
			 HAVE_32BIT_SYMBOLS,
			 mips_opts.warn_about_macros,
			 require_unextended, require_extended,
			 delayed_branch_p (&history[0]),
			 history[0].mips16_absolute_jump_p),
			symbol, offset);
    }
  else if (mips_opts.mips16 && insn_length (ip) == 2)
    {
      if (!delayed_branch_p (ip))
	/* Make sure there is enough room to swap this instruction with
	   a following jump instruction.  */
	frag_grow (6);
      add_fixed_insn (ip);
    }
  else
    {
      if (mips_opts.mips16
	  && mips_opts.noreorder
	  && delayed_branch_p (&history[0]))
	as_warn (_("extended instruction in delay slot"));

      if (mips_relax.sequence)
	{
	  /* If we've reached the end of this frag, turn it into a variant
	     frag and record the information for the instructions we've
	     written so far.  */
	  if (frag_room () < 4)
	    relax_close_frag ();
	  mips_relax.sizes[mips_relax.sequence - 1] += insn_length (ip);
	}

      if (mips_relax.sequence != 2)
	{
	  if (mips_macro_warning.first_insn_sizes[0] == 0)
	    mips_macro_warning.first_insn_sizes[0] = insn_length (ip);
	  mips_macro_warning.sizes[0] += insn_length (ip);
	  mips_macro_warning.insns[0]++;
	}
      if (mips_relax.sequence != 1)
	{
	  if (mips_macro_warning.first_insn_sizes[1] == 0)
	    mips_macro_warning.first_insn_sizes[1] = insn_length (ip);
	  mips_macro_warning.sizes[1] += insn_length (ip);
	  mips_macro_warning.insns[1]++;
	}

      if (mips_opts.mips16)
	{
	  ip->fixed_p = 1;
	  ip->mips16_absolute_jump_p = (*reloc_type == BFD_RELOC_MIPS16_JMP);
	}
      add_fixed_insn (ip);
    }

  if (!ip->complete_p && *reloc_type < BFD_RELOC_UNUSED)
    {
      bfd_reloc_code_real_type final_type[3];
      reloc_howto_type *howto0;
      reloc_howto_type *howto;
      int i;

      /* Perform any necessary conversion to microMIPS relocations
	 and find out how many relocations there actually are.  */
      for (i = 0; i < 3 && reloc_type[i] != BFD_RELOC_UNUSED; i++)
	final_type[i] = micromips_map_reloc (reloc_type[i]);

      /* In a compound relocation, it is the final (outermost)
	 operator that determines the relocated field.  */
      howto = howto0 = bfd_reloc_type_lookup (stdoutput, final_type[i - 1]);
      if (!howto)
	abort ();

      if (i > 1)
	howto0 = bfd_reloc_type_lookup (stdoutput, final_type[0]);
      ip->fixp[0] = fix_new_exp (ip->frag, ip->where,
				 bfd_get_reloc_size (howto),
				 address_expr,
				 howto0 && howto0->pc_relative,
				 final_type[0]);
      /* Record non-PIC mode in `fx_tcbit2' for `md_apply_fix'.  */
      ip->fixp[0]->fx_tcbit2 = mips_pic == NO_PIC;

      /* Tag symbols that have a R_MIPS16_26 relocation against them.  */
      if (final_type[0] == BFD_RELOC_MIPS16_JMP && ip->fixp[0]->fx_addsy)
	*symbol_get_tc (ip->fixp[0]->fx_addsy) = 1;

      /* These relocations can have an addend that won't fit in
	 4 octets for 64bit assembly.  */
      if (GPR_SIZE == 64
	  && ! howto->partial_inplace
	  && (reloc_type[0] == BFD_RELOC_16
	      || reloc_type[0] == BFD_RELOC_32
	      || reloc_type[0] == BFD_RELOC_MIPS_JMP
	      || reloc_type[0] == BFD_RELOC_GPREL16
	      || reloc_type[0] == BFD_RELOC_MIPS_LITERAL
	      || reloc_type[0] == BFD_RELOC_GPREL32
	      || reloc_type[0] == BFD_RELOC_64
	      || reloc_type[0] == BFD_RELOC_CTOR
	      || reloc_type[0] == BFD_RELOC_MIPS_SUB
	      || reloc_type[0] == BFD_RELOC_MIPS_HIGHEST
	      || reloc_type[0] == BFD_RELOC_MIPS_HIGHER
	      || reloc_type[0] == BFD_RELOC_MIPS_SCN_DISP
	      || reloc_type[0] == BFD_RELOC_MIPS_16
	      || reloc_type[0] == BFD_RELOC_MIPS_RELGOT
	      || reloc_type[0] == BFD_RELOC_MIPS16_GPREL
	      || hi16_reloc_p (reloc_type[0])
	      || lo16_reloc_p (reloc_type[0])))
	ip->fixp[0]->fx_no_overflow = 1;

      /* These relocations can have an addend that won't fit in 2 octets.  */
      if (reloc_type[0] == BFD_RELOC_MICROMIPS_7_PCREL_S1
	  || reloc_type[0] == BFD_RELOC_MICROMIPS_10_PCREL_S1)
	ip->fixp[0]->fx_no_overflow = 1;

      if (mips_relax.sequence)
	{
	  if (mips_relax.first_fixup == 0)
	    mips_relax.first_fixup = ip->fixp[0];
	}
      else if (reloc_needs_lo_p (*reloc_type))
	{
	  struct mips_hi_fixup *hi_fixup;

	  /* Reuse the last entry if it already has a matching %lo.  */
	  hi_fixup = mips_hi_fixup_list;
	  if (hi_fixup == 0
	      || !fixup_has_matching_lo_p (hi_fixup->fixp))
	    {
	      hi_fixup = XNEW (struct mips_hi_fixup);
	      hi_fixup->next = mips_hi_fixup_list;
	      mips_hi_fixup_list = hi_fixup;
	    }
	  hi_fixup->fixp = ip->fixp[0];
	  hi_fixup->seg = now_seg;
	}

      /* Add fixups for the second and third relocations, if given.
	 Note that the ABI allows the second relocation to be
	 against RSS_UNDEF, RSS_GP, RSS_GP0 or RSS_LOC.  At the
	 moment we only use RSS_UNDEF, but we could add support
	 for the others if it ever becomes necessary.  */
      for (i = 1; i < 3; i++)
	if (reloc_type[i] != BFD_RELOC_UNUSED)
	  {
	    ip->fixp[i] = fix_new (ip->frag, ip->where,
				   ip->fixp[0]->fx_size, NULL, 0,
				   false, final_type[i]);

	    /* Use fx_tcbit to mark compound relocs.  */
	    ip->fixp[0]->fx_tcbit = 1;
	    ip->fixp[i]->fx_tcbit = 1;
	  }
    }

  /* Update the register mask information.  */
  mips_gprmask |= gpr_read_mask (ip) | gpr_write_mask (ip);
  mips_cprmask[1] |= fpr_read_mask (ip) | fpr_write_mask (ip);

  switch (method)
    {
    case APPEND_ADD:
      insert_into_history (0, 1, ip);
      break;

    case APPEND_ADD_WITH_NOP:
      {
	struct mips_cl_insn *nop;

	insert_into_history (0, 1, ip);
	nop = get_delay_slot_nop (ip);
	add_fixed_insn (nop);
	insert_into_history (0, 1, nop);
	if (mips_relax.sequence)
	  mips_relax.sizes[mips_relax.sequence - 1] += insn_length (nop);
      }
      break;

    case APPEND_ADD_COMPACT:
      /* Convert MIPS16 jr/jalr into a "compact" jump.  */
      if (mips_opts.mips16)
	{
	  ip->insn_opcode |= 0x0080;
	  find_altered_mips16_opcode (ip);
	}
      /* Convert microMIPS instructions.  */
      else if (mips_opts.micromips)
	{
	  /* jr16->jrc */
	  if ((ip->insn_opcode & 0xffe0) == 0x4580)
	    ip->insn_opcode |= 0x0020;
	  /* b16->bc */
	  else if ((ip->insn_opcode & 0xfc00) == 0xcc00)
	    ip->insn_opcode = 0x40e00000;
	  /* beqz16->beqzc, bnez16->bnezc */
	  else if ((ip->insn_opcode & 0xdc00) == 0x8c00)
	    {
	      unsigned long regno;

	      regno = ip->insn_opcode >> MICROMIPSOP_SH_MD;
	      regno &= MICROMIPSOP_MASK_MD;
	      regno = micromips_to_32_reg_d_map[regno];
	      ip->insn_opcode = (((ip->insn_opcode << 9) & 0x00400000)
				 | (regno << MICROMIPSOP_SH_RS)
				 | 0x40a00000) ^ 0x00400000;
	    }
	  /* beqz->beqzc, bnez->bnezc */
	  else if ((ip->insn_opcode & 0xdfe00000) == 0x94000000)
	    ip->insn_opcode = ((ip->insn_opcode & 0x001f0000)
			       | ((ip->insn_opcode >> 7) & 0x00400000)
			       | 0x40a00000) ^ 0x00400000;
	  /* beq $0->beqzc, bne $0->bnezc */
	  else if ((ip->insn_opcode & 0xdc1f0000) == 0x94000000)
	    ip->insn_opcode = (((ip->insn_opcode >>
				 (MICROMIPSOP_SH_RT - MICROMIPSOP_SH_RS))
				& (MICROMIPSOP_MASK_RS << MICROMIPSOP_SH_RS))
			       | ((ip->insn_opcode >> 7) & 0x00400000)
			       | 0x40a00000) ^ 0x00400000;
	  else
	    abort ();
	  find_altered_micromips_opcode (ip);
	}
      else
	abort ();
      install_insn (ip);
      insert_into_history (0, 1, ip);
      break;

    case APPEND_SWAP:
      {
	struct mips_cl_insn delay = history[0];

	if (relaxed_branch || delay.frag != ip->frag)
	  {
	    /* Add the delay slot instruction to the end of the
	       current frag and shrink the fixed part of the
	       original frag.  If the branch occupies the tail of
	       the latter, move it backwards to cover the gap.  */
	    delay.frag->fr_fix -= branch_disp;
	    if (delay.frag == ip->frag)
	      move_insn (ip, ip->frag, ip->where - branch_disp);
	    add_fixed_insn (&delay);
	  }
	else
	  {
	    /* If this is not a relaxed branch and we are in the
	       same frag, then just swap the instructions.  */
	    move_insn (ip, delay.frag, delay.where);
	    move_insn (&delay, ip->frag, ip->where + insn_length (ip));
	  }
	history[0] = *ip;
	delay.fixed_p = 1;
	insert_into_history (0, 1, &delay);
      }
      break;
    }

  /* If we have just completed an unconditional branch, clear the history.  */
  if ((delayed_branch_p (&history[1]) && uncond_branch_p (&history[1]))
      || (compact_branch_p (&history[0]) && uncond_branch_p (&history[0])))
    {
      unsigned int i;

      mips_no_prev_insn ();

      for (i = 0; i < ARRAY_SIZE (history); i++)
	history[i].cleared_p = 1;
    }

  /* We need to emit a label at the end of branch-likely macros.  */
  if (emit_branch_likely_macro)
    {
      emit_branch_likely_macro = false;
      micromips_add_label ();
    }

  /* We just output an insn, so the next one doesn't have a label.  */
  mips_clear_insn_labels ();
}

/* Forget that there was any previous instruction or label.
   When BRANCH is true, the branch history is also flushed.  */

static void
mips_no_prev_insn (void)
{
  prev_nop_frag = NULL;
  insert_into_history (0, ARRAY_SIZE (history), NOP_INSN);
  mips_clear_insn_labels ();
}

/* This function must be called before we emit something other than
   instructions.  It is like mips_no_prev_insn except that it inserts
   any NOPS that might be needed by previous instructions.  */

void
mips_emit_delays (void)
{
  if (! mips_opts.noreorder)
    {
      int nops = nops_for_insn (0, history, NULL);
      if (nops > 0)
	{
	  while (nops-- > 0)
	    add_fixed_insn (NOP_INSN);
	  mips_move_text_labels ();
	}
    }
  mips_no_prev_insn ();
}

/* Start a (possibly nested) noreorder block.  */

static void
start_noreorder (void)
{
  if (mips_opts.noreorder == 0)
    {
      unsigned int i;
      int nops;

      /* None of the instructions before the .set noreorder can be moved.  */
      for (i = 0; i < ARRAY_SIZE (history); i++)
	history[i].fixed_p = 1;

      /* Insert any nops that might be needed between the .set noreorder
	 block and the previous instructions.  We will later remove any
	 nops that turn out not to be needed.  */
      nops = nops_for_insn (0, history, NULL);
      if (nops > 0)
	{
	  if (mips_optimize != 0)
	    {
	      /* Record the frag which holds the nop instructions, so
                 that we can remove them if we don't need them.  */
	      frag_grow (nops * NOP_INSN_SIZE);
	      prev_nop_frag = frag_now;
	      prev_nop_frag_holds = nops;
	      prev_nop_frag_required = 0;
	      prev_nop_frag_since = 0;
	    }

	  for (; nops > 0; --nops)
	    add_fixed_insn (NOP_INSN);

	  /* Move on to a new frag, so that it is safe to simply
	     decrease the size of prev_nop_frag.  */
	  frag_wane (frag_now);
	  frag_new (0);
	  mips_move_text_labels ();
	}
      mips_mark_labels ();
      mips_clear_insn_labels ();
    }
  mips_opts.noreorder++;
  mips_any_noreorder = 1;
}

/* End a nested noreorder block.  */

static void
end_noreorder (void)
{
  mips_opts.noreorder--;
  if (mips_opts.noreorder == 0 && prev_nop_frag != NULL)
    {
      /* Commit to inserting prev_nop_frag_required nops and go back to
	 handling nop insertion the .set reorder way.  */
      prev_nop_frag->fr_fix -= ((prev_nop_frag_holds - prev_nop_frag_required)
				* NOP_INSN_SIZE);
      insert_into_history (prev_nop_frag_since,
			   prev_nop_frag_required, NOP_INSN);
      prev_nop_frag = NULL;
    }
}

/* Sign-extend 32-bit mode constants that have bit 31 set and all
   higher bits unset.  */

static void
normalize_constant_expr (expressionS *ex)
{
  if (ex->X_op == O_constant
      && IS_ZEXT_32BIT_NUM (ex->X_add_number))
    ex->X_add_number = (((ex->X_add_number & 0xffffffff) ^ 0x80000000)
			- 0x80000000);
}

/* Sign-extend 32-bit mode address offsets that have bit 31 set and
   all higher bits unset.  */

static void
normalize_address_expr (expressionS *ex)
{
  if (((ex->X_op == O_constant && HAVE_32BIT_ADDRESSES)
	|| (ex->X_op == O_symbol && HAVE_32BIT_SYMBOLS))
      && IS_ZEXT_32BIT_NUM (ex->X_add_number))
    ex->X_add_number = (((ex->X_add_number & 0xffffffff) ^ 0x80000000)
			- 0x80000000);
}

/* Try to match TOKENS against OPCODE, storing the result in INSN.
   Return true if the match was successful.

   OPCODE_EXTRA is a value that should be ORed into the opcode
   (used for VU0 channel suffixes, etc.).  MORE_ALTS is true if
   there are more alternatives after OPCODE and SOFT_MATCH is
   as for mips_arg_info.  */

static bool
match_insn (struct mips_cl_insn *insn, const struct mips_opcode *opcode,
	    struct mips_operand_token *tokens, unsigned int opcode_extra,
	    bool lax_match, bool complete_p)
{
  const char *args;
  struct mips_arg_info arg;
  const struct mips_operand *operand;
  char c;

  imm_expr.X_op = O_absent;
  offset_expr.X_op = O_absent;
  offset_reloc[0] = BFD_RELOC_UNUSED;
  offset_reloc[1] = BFD_RELOC_UNUSED;
  offset_reloc[2] = BFD_RELOC_UNUSED;

  create_insn (insn, opcode);
  /* When no opcode suffix is specified, assume ".xyzw". */
  if ((opcode->pinfo2 & INSN2_VU0_CHANNEL_SUFFIX) != 0 && opcode_extra == 0)
    insn->insn_opcode |= 0xf << mips_vu0_channel_mask.lsb;
  else
    insn->insn_opcode |= opcode_extra;
  memset (&arg, 0, sizeof (arg));
  arg.insn = insn;
  arg.token = tokens;
  arg.argnum = 1;
  arg.last_regno = ILLEGAL_REG;
  arg.dest_regno = ILLEGAL_REG;
  arg.lax_match = lax_match;
  for (args = opcode->args;; ++args)
    {
      if (arg.token->type == OT_END)
	{
	  /* Handle unary instructions in which only one operand is given.
	     The source is then the same as the destination.  */
	  if (arg.opnum == 1 && *args == ',')
	    {
	      operand = (mips_opts.micromips
			 ? decode_micromips_operand (args + 1)
			 : decode_mips_operand (args + 1));
	      if (operand && mips_optional_operand_p (operand))
		{
		  arg.token = tokens;
		  arg.argnum = 1;
		  continue;
		}
	    }

	  /* Treat elided base registers as $0.  */
	  if (strcmp (args, "(b)") == 0)
	    args += 3;

	  if (args[0] == '+')
	    switch (args[1])
	      {
	      case 'K':
	      case 'N':
		/* The register suffix is optional. */
		args += 2;
		break;
	      }

	  /* Fail the match if there were too few operands.  */
	  if (*args)
	    return false;

	  /* Successful match.  */
	  if (!complete_p)
	    return true;
	  clear_insn_error ();
	  if (arg.dest_regno == arg.last_regno
	      && startswith (insn->insn_mo->name, "jalr"))
	    {
	      if (arg.opnum == 2)
		set_insn_error
		  (0, _("source and destination must be different"));
	      else if (arg.last_regno == 31)
		set_insn_error
		  (0, _("a destination register must be supplied"));
	    }
	  else if (arg.last_regno == 31
		   && (startswith (insn->insn_mo->name, "bltzal")
		       || startswith (insn->insn_mo->name, "bgezal")))
	    set_insn_error (0, _("the source register must not be $31"));
	  check_completed_insn (&arg);
	  return true;
	}

      /* Fail the match if the line has too many operands.   */
      if (*args == 0)
	return false;

      /* Handle characters that need to match exactly.  */
      if (*args == '(' || *args == ')' || *args == ',')
	{
	  if (match_char (&arg, *args))
	    continue;
	  return false;
	}
      if (*args == '#')
	{
	  ++args;
	  if (arg.token->type == OT_DOUBLE_CHAR
	      && arg.token->u.ch == *args)
	    {
	      ++arg.token;
	      continue;
	    }
	  return false;
	}

      /* Handle special macro operands.  Work out the properties of
	 other operands.  */
      arg.opnum += 1;
      switch (*args)
	{
	case '-':
	  switch (args[1])
	    {
	    case 'A':
	      *offset_reloc = BFD_RELOC_MIPS_19_PCREL_S2;
	      break;

	    case 'B':
	      *offset_reloc = BFD_RELOC_MIPS_18_PCREL_S3;
	      break;
	    }
	  break;

	case '+':
	  switch (args[1])
	    {
	    case 'i':
	      *offset_reloc = BFD_RELOC_MIPS_JMP;
	      break;

	    case '\'':
	      *offset_reloc = BFD_RELOC_MIPS_26_PCREL_S2;
	      break;

	    case '\"':
	      *offset_reloc = BFD_RELOC_MIPS_21_PCREL_S2;
	      break;
	    }
	  break;

	case 'I':
	  if (!match_const_int (&arg, &imm_expr.X_add_number))
	    return false;
	  imm_expr.X_op = O_constant;
	  if (GPR_SIZE == 32)
	    normalize_constant_expr (&imm_expr);
	  continue;

	case 'A':
	  if (arg.token->type == OT_CHAR && arg.token->u.ch == '(')
	    {
	      /* Assume that the offset has been elided and that what
		 we saw was a base register.  The match will fail later
		 if that assumption turns out to be wrong.  */
	      offset_expr.X_op = O_constant;
	      offset_expr.X_add_number = 0;
	    }
	  else
	    {
	      if (!match_expression (&arg, &offset_expr, offset_reloc))
		return false;
	      normalize_address_expr (&offset_expr);
	    }
	  continue;

	case 'F':
	  if (!match_float_constant (&arg, &imm_expr, &offset_expr,
				     8, true))
	    return false;
	  continue;

	case 'L':
	  if (!match_float_constant (&arg, &imm_expr, &offset_expr,
				     8, false))
	    return false;
	  continue;

	case 'f':
	  if (!match_float_constant (&arg, &imm_expr, &offset_expr,
				     4, true))
	    return false;
	  continue;

	case 'l':
	  if (!match_float_constant (&arg, &imm_expr, &offset_expr,
				     4, false))
	    return false;
	  continue;

	case 'p':
	  *offset_reloc = BFD_RELOC_16_PCREL_S2;
	  break;

	case 'a':
	  *offset_reloc = BFD_RELOC_MIPS_JMP;
	  break;

	case 'm':
	  gas_assert (mips_opts.micromips);
	  c = args[1];
	  switch (c)
	    {
	    case 'D':
	    case 'E':
	      if (!forced_insn_length)
		*offset_reloc = (int) BFD_RELOC_UNUSED + c;
	      else if (c == 'D')
		*offset_reloc = BFD_RELOC_MICROMIPS_10_PCREL_S1;
	      else
		*offset_reloc = BFD_RELOC_MICROMIPS_7_PCREL_S1;
	      break;
	    }
	  break;
	}

      operand = (mips_opts.micromips
		 ? decode_micromips_operand (args)
		 : decode_mips_operand (args));
      if (!operand)
	abort ();

      /* Skip prefixes.  */
      if (*args == '+' || *args == 'm' || *args == '-')
	args++;

      if (mips_optional_operand_p (operand)
	  && args[1] == ','
	  && (arg.token[0].type != OT_REG
	      || arg.token[1].type == OT_END))
	{
	  /* Assume that the register has been elided and is the
	     same as the first operand.  */
	  arg.token = tokens;
	  arg.argnum = 1;
	}

      if (!match_operand (&arg, operand))
	return false;
    }
}

/* Like match_insn, but for MIPS16.  */

static bool
match_mips16_insn (struct mips_cl_insn *insn, const struct mips_opcode *opcode,
		   struct mips_operand_token *tokens)
{
  const char *args;
  const struct mips_operand *operand;
  const struct mips_operand *ext_operand;
  bool pcrel = false;
  int required_insn_length;
  struct mips_arg_info arg;
  int relax_char;

  if (forced_insn_length)
    required_insn_length = forced_insn_length;
  else if (mips_opts.noautoextend && !mips_opcode_32bit_p (opcode))
    required_insn_length = 2;
  else
    required_insn_length = 0;

  create_insn (insn, opcode);
  imm_expr.X_op = O_absent;
  offset_expr.X_op = O_absent;
  offset_reloc[0] = BFD_RELOC_UNUSED;
  offset_reloc[1] = BFD_RELOC_UNUSED;
  offset_reloc[2] = BFD_RELOC_UNUSED;
  relax_char = 0;

  memset (&arg, 0, sizeof (arg));
  arg.insn = insn;
  arg.token = tokens;
  arg.argnum = 1;
  arg.last_regno = ILLEGAL_REG;
  arg.dest_regno = ILLEGAL_REG;
  relax_char = 0;
  for (args = opcode->args;; ++args)
    {
      int c;

      if (arg.token->type == OT_END)
	{
	  offsetT value;

	  /* Handle unary instructions in which only one operand is given.
	     The source is then the same as the destination.  */
	  if (arg.opnum == 1 && *args == ',')
	    {
	      operand = decode_mips16_operand (args[1], false);
	      if (operand && mips_optional_operand_p (operand))
		{
		  arg.token = tokens;
		  arg.argnum = 1;
		  continue;
		}
	    }

	  /* Fail the match if there were too few operands.  */
	  if (*args)
	    return false;

	  /* Successful match.  Stuff the immediate value in now, if
	     we can.  */
	  clear_insn_error ();
	  if (opcode->pinfo == INSN_MACRO)
	    {
	      gas_assert (relax_char == 0 || relax_char == 'p');
	      gas_assert (*offset_reloc == BFD_RELOC_UNUSED);
	    }
	  else if (relax_char
		   && offset_expr.X_op == O_constant
		   && !pcrel
		   && calculate_reloc (*offset_reloc,
				       offset_expr.X_add_number,
				       &value))
	    {
	      mips16_immed (NULL, 0, relax_char, *offset_reloc, value,
			    required_insn_length, &insn->insn_opcode);
	      offset_expr.X_op = O_absent;
	      *offset_reloc = BFD_RELOC_UNUSED;
	    }
	  else if (relax_char && *offset_reloc != BFD_RELOC_UNUSED)
	    {
	      if (required_insn_length == 2)
		set_insn_error (0, _("invalid unextended operand value"));
	      else if (!mips_opcode_32bit_p (opcode))
		{
		  forced_insn_length = 4;
		  insn->insn_opcode |= MIPS16_EXTEND;
		}
	    }
	  else if (relax_char)
	    *offset_reloc = (int) BFD_RELOC_UNUSED + relax_char;

	  check_completed_insn (&arg);
	  return true;
	}

      /* Fail the match if the line has too many operands.   */
      if (*args == 0)
	return false;

      /* Handle characters that need to match exactly.  */
      if (*args == '(' || *args == ')' || *args == ',')
	{
	  if (match_char (&arg, *args))
	    continue;
	  return false;
	}

      arg.opnum += 1;
      c = *args;
      switch (c)
	{
	case 'p':
	case 'q':
	case 'A':
	case 'B':
	case 'E':
	case 'V':
	case 'u':
	  relax_char = c;
	  break;

	case 'I':
	  if (!match_const_int (&arg, &imm_expr.X_add_number))
	    return false;
	  imm_expr.X_op = O_constant;
	  if (GPR_SIZE == 32)
	    normalize_constant_expr (&imm_expr);
	  continue;

	case 'a':
	case 'i':
	  *offset_reloc = BFD_RELOC_MIPS16_JMP;
	  break;
	}

      operand = decode_mips16_operand (c, mips_opcode_32bit_p (opcode));
      if (!operand)
	abort ();

      if (operand->type == OP_PCREL)
	pcrel = true;
      else
	{
	  ext_operand = decode_mips16_operand (c, true);
	  if (operand != ext_operand)
	    {
	      if (arg.token->type == OT_CHAR && arg.token->u.ch == '(')
		{
		  offset_expr.X_op = O_constant;
		  offset_expr.X_add_number = 0;
		  relax_char = c;
		  continue;
		}

	      if (!match_expression (&arg, &offset_expr, offset_reloc))
		return false;

	      /* '8' is used for SLTI(U) and has traditionally not
		 been allowed to take relocation operators.  */
	      if (offset_reloc[0] != BFD_RELOC_UNUSED
		  && (ext_operand->size != 16 || c == '8'))
		{
		  match_not_constant (&arg);
		  return false;
		}

	      if (offset_expr.X_op == O_big)
		{
		  match_out_of_range (&arg);
		  return false;
		}

	      relax_char = c;
	      continue;
	    }
	}

      if (mips_optional_operand_p (operand)
	  && args[1] == ','
	  && (arg.token[0].type != OT_REG
	      || arg.token[1].type == OT_END))
	{
	  /* Assume that the register has been elided and is the
	     same as the first operand.  */
	  arg.token = tokens;
	  arg.argnum = 1;
	}

      if (!match_operand (&arg, operand))
	return false;
    }
}

/* Record that the current instruction is invalid for the current ISA.  */

static void
match_invalid_for_isa (void)
{
  set_insn_error_ss
    (0, _("opcode not supported on this processor: %s (%s)"),
     mips_cpu_info_from_arch (mips_opts.arch)->name,
     mips_cpu_info_from_isa (mips_opts.isa)->name);
}

/* Try to match TOKENS against a series of opcode entries, starting at FIRST.
   Return true if a definite match or failure was found, storing any match
   in INSN.  OPCODE_EXTRA is a value that should be ORed into the opcode
   (to handle things like VU0 suffixes).  LAX_MATCH is true if we have already
   tried and failed to match under normal conditions and now want to try a
   more relaxed match.  */

static bool
match_insns (struct mips_cl_insn *insn, const struct mips_opcode *first,
	     const struct mips_opcode *past, struct mips_operand_token *tokens,
	     int opcode_extra, bool lax_match)
{
  const struct mips_opcode *opcode;
  const struct mips_opcode *invalid_delay_slot;
  bool seen_valid_for_isa, seen_valid_for_size;

  /* Search for a match, ignoring alternatives that don't satisfy the
     current ISA or forced_length.  */
  invalid_delay_slot = 0;
  seen_valid_for_isa = false;
  seen_valid_for_size = false;
  opcode = first;
  do
    {
      gas_assert (strcmp (opcode->name, first->name) == 0);
      if (is_opcode_valid (opcode))
	{
	  seen_valid_for_isa = true;
	  if (is_size_valid (opcode))
	    {
	      bool delay_slot_ok;

	      seen_valid_for_size = true;
	      delay_slot_ok = is_delay_slot_valid (opcode);
	      if (match_insn (insn, opcode, tokens, opcode_extra,
			      lax_match, delay_slot_ok))
		{
		  if (!delay_slot_ok)
		    {
		      if (!invalid_delay_slot)
			invalid_delay_slot = opcode;
		    }
		  else
		    return true;
		}
	    }
	}
      ++opcode;
    }
  while (opcode < past && strcmp (opcode->name, first->name) == 0);

  /* If the only matches we found had the wrong length for the delay slot,
     pick the first such match.  We'll issue an appropriate warning later.  */
  if (invalid_delay_slot)
    {
      if (match_insn (insn, invalid_delay_slot, tokens, opcode_extra,
		      lax_match, true))
	return true;
      abort ();
    }

  /* Handle the case where we didn't try to match an instruction because
     all the alternatives were incompatible with the current ISA.  */
  if (!seen_valid_for_isa)
    {
      match_invalid_for_isa ();
      return true;
    }

  /* Handle the case where we didn't try to match an instruction because
     all the alternatives were of the wrong size.  */
  if (!seen_valid_for_size)
    {
      if (mips_opts.insn32)
	set_insn_error (0, _("opcode not supported in the `insn32' mode"));
      else
	set_insn_error_i
	  (0, _("unrecognized %d-bit version of microMIPS opcode"),
	   8 * forced_insn_length);
      return true;
    }

  return false;
}

/* Like match_insns, but for MIPS16.  */

static bool
match_mips16_insns (struct mips_cl_insn *insn, const struct mips_opcode *first,
		    struct mips_operand_token *tokens)
{
  const struct mips_opcode *opcode;
  bool seen_valid_for_isa;
  bool seen_valid_for_size;

  /* Search for a match, ignoring alternatives that don't satisfy the
     current ISA.  There are no separate entries for extended forms so
     we deal with forced_length later.  */
  seen_valid_for_isa = false;
  seen_valid_for_size = false;
  opcode = first;
  do
    {
      gas_assert (strcmp (opcode->name, first->name) == 0);
      if (is_opcode_valid_16 (opcode))
	{
	  seen_valid_for_isa = true;
	  if (is_size_valid_16 (opcode))
	    {
	      seen_valid_for_size = true;
	      if (match_mips16_insn (insn, opcode, tokens))
		return true;
	    }
	}
      ++opcode;
    }
  while (opcode < &mips16_opcodes[bfd_mips16_num_opcodes]
	 && strcmp (opcode->name, first->name) == 0);

  /* Handle the case where we didn't try to match an instruction because
     all the alternatives were incompatible with the current ISA.  */
  if (!seen_valid_for_isa)
    {
      match_invalid_for_isa ();
      return true;
    }

  /* Handle the case where we didn't try to match an instruction because
     all the alternatives were of the wrong size.  */
  if (!seen_valid_for_size)
    {
      if (forced_insn_length == 2)
	set_insn_error
	  (0, _("unrecognized unextended version of MIPS16 opcode"));
      else
	set_insn_error
	  (0, _("unrecognized extended version of MIPS16 opcode"));
      return true;
    }

  return false;
}

/* Set up global variables for the start of a new macro.  */

static void
macro_start (void)
{
  memset (&mips_macro_warning.sizes, 0, sizeof (mips_macro_warning.sizes));
  memset (&mips_macro_warning.first_insn_sizes, 0,
	  sizeof (mips_macro_warning.first_insn_sizes));
  memset (&mips_macro_warning.insns, 0, sizeof (mips_macro_warning.insns));
  mips_macro_warning.delay_slot_p = (mips_opts.noreorder
				     && delayed_branch_p (&history[0]));
  if (history[0].frag
      && history[0].frag->fr_type == rs_machine_dependent
      && RELAX_MICROMIPS_P (history[0].frag->fr_subtype)
      && RELAX_MICROMIPS_NODS (history[0].frag->fr_subtype))
    mips_macro_warning.delay_slot_length = 0;
  else
    switch (history[0].insn_mo->pinfo2
	    & (INSN2_BRANCH_DELAY_32BIT | INSN2_BRANCH_DELAY_16BIT))
      {
      case INSN2_BRANCH_DELAY_32BIT:
	mips_macro_warning.delay_slot_length = 4;
	break;
      case INSN2_BRANCH_DELAY_16BIT:
	mips_macro_warning.delay_slot_length = 2;
	break;
      default:
	mips_macro_warning.delay_slot_length = 0;
	break;
      }
  mips_macro_warning.first_frag = NULL;
}

/* Given that a macro is longer than one instruction or of the wrong size,
   return the appropriate warning for it.  Return null if no warning is
   needed.  SUBTYPE is a bitmask of RELAX_DELAY_SLOT, RELAX_DELAY_SLOT_16BIT,
   RELAX_DELAY_SLOT_SIZE_FIRST, RELAX_DELAY_SLOT_SIZE_SECOND,
   and RELAX_NOMACRO.  */

static const char *
macro_warning (relax_substateT subtype)
{
  if (subtype & RELAX_DELAY_SLOT)
    return _("macro instruction expanded into multiple instructions"
	     " in a branch delay slot");
  else if (subtype & RELAX_NOMACRO)
    return _("macro instruction expanded into multiple instructions");
  else if (subtype & (RELAX_DELAY_SLOT_SIZE_FIRST
		      | RELAX_DELAY_SLOT_SIZE_SECOND))
    return ((subtype & RELAX_DELAY_SLOT_16BIT)
	    ? _("macro instruction expanded into a wrong size instruction"
		" in a 16-bit branch delay slot")
	    : _("macro instruction expanded into a wrong size instruction"
		" in a 32-bit branch delay slot"));
  else
    return 0;
}

/* Finish up a macro.  Emit warnings as appropriate.  */

static void
macro_end (void)
{
  /* Relaxation warning flags.  */
  relax_substateT subtype = 0;

  /* Check delay slot size requirements.  */
  if (mips_macro_warning.delay_slot_length == 2)
    subtype |= RELAX_DELAY_SLOT_16BIT;
  if (mips_macro_warning.delay_slot_length != 0)
    {
      if (mips_macro_warning.delay_slot_length
	  != mips_macro_warning.first_insn_sizes[0])
	subtype |= RELAX_DELAY_SLOT_SIZE_FIRST;
      if (mips_macro_warning.delay_slot_length
	  != mips_macro_warning.first_insn_sizes[1])
	subtype |= RELAX_DELAY_SLOT_SIZE_SECOND;
    }

  /* Check instruction count requirements.  */
  if (mips_macro_warning.insns[0] > 1 || mips_macro_warning.insns[1] > 1)
    {
      if (mips_macro_warning.insns[1] > mips_macro_warning.insns[0])
	subtype |= RELAX_SECOND_LONGER;
      if (mips_opts.warn_about_macros)
	subtype |= RELAX_NOMACRO;
      if (mips_macro_warning.delay_slot_p)
	subtype |= RELAX_DELAY_SLOT;
    }

  /* If both alternatives fail to fill a delay slot correctly,
     emit the warning now.  */
  if ((subtype & RELAX_DELAY_SLOT_SIZE_FIRST) != 0
      && (subtype & RELAX_DELAY_SLOT_SIZE_SECOND) != 0)
    {
      relax_substateT s;
      const char *msg;

      s = subtype & (RELAX_DELAY_SLOT_16BIT
		     | RELAX_DELAY_SLOT_SIZE_FIRST
		     | RELAX_DELAY_SLOT_SIZE_SECOND);
      msg = macro_warning (s);
      if (msg != NULL)
	as_warn ("%s", msg);
      subtype &= ~s;
    }

  /* If both implementations are longer than 1 instruction, then emit the
     warning now.  */
  if (mips_macro_warning.insns[0] > 1 && mips_macro_warning.insns[1] > 1)
    {
      relax_substateT s;
      const char *msg;

      s = subtype & (RELAX_SECOND_LONGER | RELAX_NOMACRO | RELAX_DELAY_SLOT);
      msg = macro_warning (s);
      if (msg != NULL)
	as_warn ("%s", msg);
      subtype &= ~s;
    }

  /* If any flags still set, then one implementation might need a warning
     and the other either will need one of a different kind or none at all.
     Pass any remaining flags over to relaxation.  */
  if (mips_macro_warning.first_frag != NULL)
    mips_macro_warning.first_frag->fr_subtype |= subtype;
}

/* Instruction operand formats used in macros that vary between
   standard MIPS and microMIPS code.  */

static const char * const brk_fmt[2][2] = { { "c", "c" }, { "mF", "c" } };
static const char * const cop12_fmt[2] = { "E,o(b)", "E,~(b)" };
static const char * const jalr_fmt[2] = { "d,s", "t,s" };
static const char * const lui_fmt[2] = { "t,u", "s,u" };
static const char * const mem12_fmt[2] = { "t,o(b)", "t,~(b)" };
static const char * const mfhl_fmt[2][2] = { { "d", "d" }, { "mj", "s" } };
static const char * const shft_fmt[2] = { "d,w,<", "t,r,<" };
static const char * const trap_fmt[2] = { "s,t,q", "s,t,|" };

#define BRK_FMT (brk_fmt[mips_opts.micromips][mips_opts.insn32])
#define COP12_FMT (ISA_IS_R6 (mips_opts.isa) ? "E,+:(d)" \
					     : cop12_fmt[mips_opts.micromips])
#define JALR_FMT (jalr_fmt[mips_opts.micromips])
#define LUI_FMT (lui_fmt[mips_opts.micromips])
#define MEM12_FMT (mem12_fmt[mips_opts.micromips])
#define LL_SC_FMT (ISA_IS_R6 (mips_opts.isa) ? "t,+j(b)" \
					     : mem12_fmt[mips_opts.micromips])
#define MFHL_FMT (mfhl_fmt[mips_opts.micromips][mips_opts.insn32])
#define SHFT_FMT (shft_fmt[mips_opts.micromips])
#define TRAP_FMT (trap_fmt[mips_opts.micromips])

/* Read a macro's relocation codes from *ARGS and store them in *R.
   The first argument in *ARGS will be either the code for a single
   relocation or -1 followed by the three codes that make up a
   composite relocation.  */

static void
macro_read_relocs (va_list *args, bfd_reloc_code_real_type *r)
{
  int i, next;

  next = va_arg (*args, int);
  if (next >= 0)
    r[0] = (bfd_reloc_code_real_type) next;
  else
    {
      for (i = 0; i < 3; i++)
	r[i] = (bfd_reloc_code_real_type) va_arg (*args, int);
      /* This function is only used for 16-bit relocation fields.
	 To make the macro code simpler, treat an unrelocated value
	 in the same way as BFD_RELOC_LO16.  */
      if (r[0] == BFD_RELOC_UNUSED)
	r[0] = BFD_RELOC_LO16;
    }
}

/* Build an instruction created by a macro expansion.  This is passed
   a pointer to the count of instructions created so far, an
   expression, the name of the instruction to build, an operand format
   string, and corresponding arguments.  */

static void
macro_build (expressionS *ep, const char *name, const char *fmt, ...)
{
  const struct mips_opcode *mo = NULL;
  bfd_reloc_code_real_type r[3];
  const struct mips_opcode *amo;
  const struct mips_operand *operand;
  htab_t hash;
  struct mips_cl_insn insn;
  va_list args;
  unsigned int uval;

  va_start (args, fmt);

  if (mips_opts.mips16)
    {
      mips16_macro_build (ep, name, fmt, &args);
      va_end (args);
      return;
    }

  r[0] = BFD_RELOC_UNUSED;
  r[1] = BFD_RELOC_UNUSED;
  r[2] = BFD_RELOC_UNUSED;
  hash = mips_opts.micromips ? micromips_op_hash : op_hash;
  amo = (struct mips_opcode *) str_hash_find (hash, name);
  gas_assert (amo);
  gas_assert (strcmp (name, amo->name) == 0);

  do
    {
      /* Search until we get a match for NAME.  It is assumed here that
	 macros will never generate MDMX, MIPS-3D, or MT instructions.
	 We try to match an instruction that fulfills the branch delay
	 slot instruction length requirement (if any) of the previous
	 instruction.  While doing this we record the first instruction
	 seen that matches all the other conditions and use it anyway
	 if the requirement cannot be met; we will issue an appropriate
	 warning later on.  */
      if (strcmp (fmt, amo->args) == 0
	  && amo->pinfo != INSN_MACRO
	  && is_opcode_valid (amo)
	  && is_size_valid (amo))
	{
	  if (is_delay_slot_valid (amo))
	    {
	      mo = amo;
	      break;
	    }
	  else if (!mo)
	    mo = amo;
	}

      ++amo;
      gas_assert (amo->name);
    }
  while (strcmp (name, amo->name) == 0);

  gas_assert (mo);
  create_insn (&insn, mo);
  for (; *fmt; ++fmt)
    {
      switch (*fmt)
	{
	case ',':
	case '(':
	case ')':
	case 'z':
	  break;

	case 'i':
	case 'j':
	  macro_read_relocs (&args, r);
	  gas_assert (*r == BFD_RELOC_GPREL16
		      || *r == BFD_RELOC_MIPS_HIGHER
		      || *r == BFD_RELOC_HI16_S
		      || *r == BFD_RELOC_LO16
		      || *r == BFD_RELOC_MIPS_GOT_OFST
		      || (mips_opts.micromips
			  && (*r == BFD_RELOC_MIPS_16
			      || *r == BFD_RELOC_MIPS_GOT16
			      || *r == BFD_RELOC_MIPS_CALL16
			      || *r == BFD_RELOC_MIPS_GOT_HI16
			      || *r == BFD_RELOC_MIPS_GOT_LO16
			      || *r == BFD_RELOC_MIPS_CALL_HI16
			      || *r == BFD_RELOC_MIPS_CALL_LO16
			      || *r == BFD_RELOC_MIPS_SUB
			      || *r == BFD_RELOC_MIPS_GOT_PAGE
			      || *r == BFD_RELOC_MIPS_HIGHEST
			      || *r == BFD_RELOC_MIPS_GOT_DISP
			      || *r == BFD_RELOC_MIPS_TLS_GD
			      || *r == BFD_RELOC_MIPS_TLS_LDM
			      || *r == BFD_RELOC_MIPS_TLS_DTPREL_HI16
			      || *r == BFD_RELOC_MIPS_TLS_DTPREL_LO16
			      || *r == BFD_RELOC_MIPS_TLS_GOTTPREL
			      || *r == BFD_RELOC_MIPS_TLS_TPREL_HI16
			      || *r == BFD_RELOC_MIPS_TLS_TPREL_LO16)));
	  break;

	case 'o':
	  macro_read_relocs (&args, r);
	  break;

	case 'u':
	  macro_read_relocs (&args, r);
	  gas_assert (ep != NULL
		      && (ep->X_op == O_constant
			  || (ep->X_op == O_symbol
			      && (*r == BFD_RELOC_MIPS_HIGHEST
				  || *r == BFD_RELOC_HI16_S
				  || *r == BFD_RELOC_HI16
				  || *r == BFD_RELOC_GPREL16
				  || *r == BFD_RELOC_MIPS_GOT_HI16
				  || *r == BFD_RELOC_MIPS_CALL_HI16))));
	  break;

	case 'p':
	  gas_assert (ep != NULL);

	  /*
	   * This allows macro() to pass an immediate expression for
	   * creating short branches without creating a symbol.
	   *
	   * We don't allow branch relaxation for these branches, as
	   * they should only appear in ".set nomacro" anyway.
	   */
	  if (ep->X_op == O_constant)
	    {
	      /* For microMIPS we always use relocations for branches.
	         So we should not resolve immediate values.  */
	      gas_assert (!mips_opts.micromips);

	      if ((ep->X_add_number & 3) != 0)
		as_bad (_("branch to misaligned address (0x%lx)"),
			(unsigned long) ep->X_add_number);
	      if ((ep->X_add_number + 0x20000) & ~0x3ffff)
		as_bad (_("branch address range overflow (0x%lx)"),
			(unsigned long) ep->X_add_number);
	      insn.insn_opcode |= (ep->X_add_number >> 2) & 0xffff;
	      ep = NULL;
	    }
	  else
	    *r = BFD_RELOC_16_PCREL_S2;
	  break;

	case 'a':
	  gas_assert (ep != NULL);
	  *r = BFD_RELOC_MIPS_JMP;
	  break;

	default:
	  operand = (mips_opts.micromips
		     ? decode_micromips_operand (fmt)
		     : decode_mips_operand (fmt));
	  if (!operand)
	    abort ();

	  uval = va_arg (args, int);
	  if (operand->type == OP_CLO_CLZ_DEST)
	    uval |= (uval << 5);
	  insn_insert_operand (&insn, operand, uval);

	  if (*fmt == '+' || *fmt == 'm' || *fmt == '-')
	    ++fmt;
	  break;
	}
    }
  va_end (args);
  gas_assert (*r == BFD_RELOC_UNUSED ? ep == NULL : ep != NULL);

  append_insn (&insn, ep, r, true);
}

static void
mips16_macro_build (expressionS *ep, const char *name, const char *fmt,
		    va_list *args)
{
  struct mips_opcode *mo;
  struct mips_cl_insn insn;
  const struct mips_operand *operand;
  bfd_reloc_code_real_type r[3]
    = {BFD_RELOC_UNUSED, BFD_RELOC_UNUSED, BFD_RELOC_UNUSED};

  mo = (struct mips_opcode *) str_hash_find (mips16_op_hash, name);
  gas_assert (mo);
  gas_assert (strcmp (name, mo->name) == 0);

  while (strcmp (fmt, mo->args) != 0 || mo->pinfo == INSN_MACRO)
    {
      ++mo;
      gas_assert (mo->name);
      gas_assert (strcmp (name, mo->name) == 0);
    }

  create_insn (&insn, mo);
  for (; *fmt; ++fmt)
    {
      int c;

      c = *fmt;
      switch (c)
	{
	case ',':
	case '(':
	case ')':
	  break;

	case '.':
	case 'S':
	case 'P':
	case 'R':
	  break;

	case '<':
	case '5':
	case 'F':
	case 'H':
	case 'W':
	case 'D':
	case 'j':
	case '8':
	case 'V':
	case 'C':
	case 'U':
	case 'k':
	case 'K':
	case 'p':
	case 'q':
	  {
	    offsetT value;

	    gas_assert (ep != NULL);

	    if (ep->X_op != O_constant)
	      *r = (int) BFD_RELOC_UNUSED + c;
	    else if (calculate_reloc (*r, ep->X_add_number, &value))
	      {
		mips16_immed (NULL, 0, c, *r, value, 0, &insn.insn_opcode);
		ep = NULL;
		*r = BFD_RELOC_UNUSED;
	      }
	  }
	  break;

	default:
	  operand = decode_mips16_operand (c, false);
	  if (!operand)
	    abort ();

	  insn_insert_operand (&insn, operand, va_arg (*args, int));
	  break;
	}
    }

  gas_assert (*r == BFD_RELOC_UNUSED ? ep == NULL : ep != NULL);

  append_insn (&insn, ep, r, true);
}

/*
 * Generate a "jalr" instruction with a relocation hint to the called
 * function.  This occurs in NewABI PIC code.
 */
static void
macro_build_jalr (expressionS *ep, int cprestore)
{
  static const bfd_reloc_code_real_type jalr_relocs[2]
    = { BFD_RELOC_MIPS_JALR, BFD_RELOC_MICROMIPS_JALR };
  bfd_reloc_code_real_type jalr_reloc = jalr_relocs[mips_opts.micromips];
  const char *jalr;
  char *f = NULL;

  if (MIPS_JALR_HINT_P (ep))
    {
      frag_grow (8);
      f = frag_more (0);
    }
  if (mips_opts.micromips)
    {
      jalr = ((mips_opts.noreorder && !cprestore) || mips_opts.insn32
	      ? "jalr" : "jalrs");
      if (MIPS_JALR_HINT_P (ep)
	  || mips_opts.insn32
	  || (history[0].insn_mo->pinfo2 & INSN2_BRANCH_DELAY_32BIT))
	macro_build (NULL, jalr, "t,s", RA, PIC_CALL_REG);
      else
	macro_build (NULL, jalr, "mj", PIC_CALL_REG);
    }
  else
    macro_build (NULL, "jalr", "d,s", RA, PIC_CALL_REG);
  if (MIPS_JALR_HINT_P (ep))
    fix_new_exp (frag_now, f - frag_now->fr_literal, 4, ep, false, jalr_reloc);
}

/*
 * Generate a "lui" instruction.
 */
static void
macro_build_lui (expressionS *ep, int regnum)
{
  gas_assert (! mips_opts.mips16);

  if (ep->X_op != O_constant)
    {
      gas_assert (ep->X_op == O_symbol);
      /* _gp_disp is a special case, used from s_cpload.
	 __gnu_local_gp is used if mips_no_shared.  */
      gas_assert (mips_pic == NO_PIC
	      || (! HAVE_NEWABI
		  && strcmp (S_GET_NAME (ep->X_add_symbol), "_gp_disp") == 0)
	      || (! mips_in_shared
		  && strcmp (S_GET_NAME (ep->X_add_symbol),
                             "__gnu_local_gp") == 0));
    }

  macro_build (ep, "lui", LUI_FMT, regnum, BFD_RELOC_HI16_S);
}

/* Generate a sequence of instructions to do a load or store from a constant
   offset off of a base register (breg) into/from a target register (treg),
   using AT if necessary.  */
static void
macro_build_ldst_constoffset (expressionS *ep, const char *op,
			      int treg, int breg, int dbl)
{
  gas_assert (ep->X_op == O_constant);

  /* Sign-extending 32-bit constants makes their handling easier.  */
  if (!dbl)
    normalize_constant_expr (ep);

  /* Right now, this routine can only handle signed 32-bit constants.  */
  if (! IS_SEXT_32BIT_NUM(ep->X_add_number + 0x8000))
    as_warn (_("operand overflow"));

  if (IS_SEXT_16BIT_NUM(ep->X_add_number))
    {
      /* Signed 16-bit offset will fit in the op.  Easy!  */
      macro_build (ep, op, "t,o(b)", treg, BFD_RELOC_LO16, breg);
    }
  else
    {
      /* 32-bit offset, need multiple instructions and AT, like:
	   lui      $tempreg,const_hi       (BFD_RELOC_HI16_S)
	   addu     $tempreg,$tempreg,$breg
           <op>     $treg,const_lo($tempreg)   (BFD_RELOC_LO16)
         to handle the complete offset.  */
      macro_build_lui (ep, AT);
      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", AT, AT, breg);
      macro_build (ep, op, "t,o(b)", treg, BFD_RELOC_LO16, AT);

      if (!mips_opts.at)
	as_bad (_("macro used $at after \".set noat\""));
    }
}

/*			set_at()
 * Generates code to set the $at register to true (one)
 * if reg is less than the immediate expression.
 */
static void
set_at (int reg, int unsignedp)
{
  if (imm_expr.X_add_number >= -0x8000
      && imm_expr.X_add_number < 0x8000)
    macro_build (&imm_expr, unsignedp ? "sltiu" : "slti", "t,r,j",
		 AT, reg, BFD_RELOC_LO16);
  else
    {
      load_register (AT, &imm_expr, GPR_SIZE == 64);
      macro_build (NULL, unsignedp ? "sltu" : "slt", "d,v,t", AT, reg, AT);
    }
}

/* Count the leading zeroes by performing a binary chop. This is a
   bulky bit of source, but performance is a LOT better for the
   majority of values than a simple loop to count the bits:
       for (lcnt = 0; (lcnt < 32); lcnt++)
         if ((v) & (1 << (31 - lcnt)))
           break;
  However it is not code size friendly, and the gain will drop a bit
  on certain cached systems.
*/
#define COUNT_TOP_ZEROES(v)             \
  (((v) & ~0xffff) == 0                 \
   ? ((v) & ~0xff) == 0                 \
     ? ((v) & ~0xf) == 0                \
       ? ((v) & ~0x3) == 0              \
         ? ((v) & ~0x1) == 0            \
           ? !(v)                       \
             ? 32                       \
             : 31                       \
           : 30                         \
         : ((v) & ~0x7) == 0            \
           ? 29                         \
           : 28                         \
       : ((v) & ~0x3f) == 0             \
         ? ((v) & ~0x1f) == 0           \
           ? 27                         \
           : 26                         \
         : ((v) & ~0x7f) == 0           \
           ? 25                         \
           : 24                         \
     : ((v) & ~0xfff) == 0              \
       ? ((v) & ~0x3ff) == 0            \
         ? ((v) & ~0x1ff) == 0          \
           ? 23                         \
           : 22                         \
         : ((v) & ~0x7ff) == 0          \
           ? 21                         \
           : 20                         \
       : ((v) & ~0x3fff) == 0           \
         ? ((v) & ~0x1fff) == 0         \
           ? 19                         \
           : 18                         \
         : ((v) & ~0x7fff) == 0         \
           ? 17                         \
           : 16                         \
   : ((v) & ~0xffffff) == 0             \
     ? ((v) & ~0xfffff) == 0            \
       ? ((v) & ~0x3ffff) == 0          \
         ? ((v) & ~0x1ffff) == 0        \
           ? 15                         \
           : 14                         \
         : ((v) & ~0x7ffff) == 0        \
           ? 13                         \
           : 12                         \
       : ((v) & ~0x3fffff) == 0         \
         ? ((v) & ~0x1fffff) == 0       \
           ? 11                         \
           : 10                         \
         : ((v) & ~0x7fffff) == 0       \
           ? 9                          \
           : 8                          \
     : ((v) & ~0xfffffff) == 0          \
       ? ((v) & ~0x3ffffff) == 0        \
         ? ((v) & ~0x1ffffff) == 0      \
           ? 7                          \
           : 6                          \
         : ((v) & ~0x7ffffff) == 0      \
           ? 5                          \
           : 4                          \
       : ((v) & ~0x3fffffff) == 0       \
         ? ((v) & ~0x1fffffff) == 0     \
           ? 3                          \
           : 2                          \
         : ((v) & ~0x7fffffff) == 0     \
           ? 1                          \
           : 0)

/*			load_register()
 *  This routine generates the least number of instructions necessary to load
 *  an absolute expression value into a register.
 */
static void
load_register (int reg, expressionS *ep, int dbl)
{
  int freg;
  expressionS hi32, lo32;

  if (ep->X_op != O_big)
    {
      gas_assert (ep->X_op == O_constant);

      /* Sign-extending 32-bit constants makes their handling easier.  */
      if (!dbl)
	normalize_constant_expr (ep);

      if (IS_SEXT_16BIT_NUM (ep->X_add_number))
	{
	  /* We can handle 16 bit signed values with an addiu to
	     $zero.  No need to ever use daddiu here, since $zero and
	     the result are always correct in 32 bit mode.  */
	  macro_build (ep, "addiu", "t,r,j", reg, 0, BFD_RELOC_LO16);
	  return;
	}
      else if (ep->X_add_number >= 0 && ep->X_add_number < 0x10000)
	{
	  /* We can handle 16 bit unsigned values with an ori to
             $zero.  */
	  macro_build (ep, "ori", "t,r,i", reg, 0, BFD_RELOC_LO16);
	  return;
	}
      else if ((IS_SEXT_32BIT_NUM (ep->X_add_number)))
	{
	  /* 32 bit values require an lui.  */
	  macro_build (ep, "lui", LUI_FMT, reg, BFD_RELOC_HI16);
	  if ((ep->X_add_number & 0xffff) != 0)
	    macro_build (ep, "ori", "t,r,i", reg, reg, BFD_RELOC_LO16);
	  return;
	}
    }

  /* The value is larger than 32 bits.  */

  if (!dbl || GPR_SIZE == 32)
    {
      as_bad (_("number (0x%" PRIx64 ") larger than 32 bits"),
	      ep->X_add_number);
      macro_build (ep, "addiu", "t,r,j", reg, 0, BFD_RELOC_LO16);
      return;
    }

  if (ep->X_op != O_big)
    {
      hi32 = *ep;
      hi32.X_add_number = (valueT) hi32.X_add_number >> 16;
      hi32.X_add_number = (valueT) hi32.X_add_number >> 16;
      hi32.X_add_number &= 0xffffffff;
      lo32 = *ep;
      lo32.X_add_number &= 0xffffffff;
    }
  else
    {
      gas_assert (ep->X_add_number > 2);
      if (ep->X_add_number == 3)
	generic_bignum[3] = 0;
      else if (ep->X_add_number > 4)
	as_bad (_("number larger than 64 bits"));
      lo32.X_op = O_constant;
      lo32.X_add_number = generic_bignum[0] + (generic_bignum[1] << 16);
      hi32.X_op = O_constant;
      hi32.X_add_number = generic_bignum[2] + (generic_bignum[3] << 16);
    }

  if (hi32.X_add_number == 0)
    freg = 0;
  else
    {
      int shift, bit;
      unsigned long hi, lo;

      if (hi32.X_add_number == (offsetT) 0xffffffff)
	{
	  if ((lo32.X_add_number & 0xffff8000) == 0xffff8000)
	    {
	      macro_build (&lo32, "addiu", "t,r,j", reg, 0, BFD_RELOC_LO16);
	      return;
	    }
	  if (lo32.X_add_number & 0x80000000)
	    {
	      macro_build (&lo32, "lui", LUI_FMT, reg, BFD_RELOC_HI16);
	      if (lo32.X_add_number & 0xffff)
		macro_build (&lo32, "ori", "t,r,i", reg, reg, BFD_RELOC_LO16);
	      return;
	    }
	}

      /* Check for 16bit shifted constant.  We know that hi32 is
         non-zero, so start the mask on the first bit of the hi32
         value.  */
      shift = 17;
      do
	{
	  unsigned long himask, lomask;

	  if (shift < 32)
	    {
	      himask = 0xffff >> (32 - shift);
	      lomask = (0xffffU << shift) & 0xffffffff;
	    }
	  else
	    {
	      himask = 0xffffU << (shift - 32);
	      lomask = 0;
	    }
	  if ((hi32.X_add_number & ~(offsetT) himask) == 0
	      && (lo32.X_add_number & ~(offsetT) lomask) == 0)
	    {
	      expressionS tmp;

	      tmp.X_op = O_constant;
	      if (shift < 32)
		tmp.X_add_number = ((hi32.X_add_number << (32 - shift))
				    | (lo32.X_add_number >> shift));
	      else
		tmp.X_add_number = hi32.X_add_number >> (shift - 32);
	      macro_build (&tmp, "ori", "t,r,i", reg, 0, BFD_RELOC_LO16);
	      macro_build (NULL, (shift >= 32) ? "dsll32" : "dsll", SHFT_FMT,
			   reg, reg, (shift >= 32) ? shift - 32 : shift);
	      return;
	    }
	  ++shift;
	}
      while (shift <= (64 - 16));

      /* Find the bit number of the lowest one bit, and store the
         shifted value in hi/lo.  */
      hi = (unsigned long) (hi32.X_add_number & 0xffffffff);
      lo = (unsigned long) (lo32.X_add_number & 0xffffffff);
      if (lo != 0)
	{
	  bit = 0;
	  while ((lo & 1) == 0)
	    {
	      lo >>= 1;
	      ++bit;
	    }
	  if (bit != 0)
	    {
	      lo |= (hi & ((2UL << (bit - 1)) - 1)) << (32 - bit);
	      hi >>= bit;
	    }
	}
      else
	{
	  bit = 32;
	  while ((hi & 1) == 0)
	    {
	      hi >>= 1;
	      ++bit;
	    }
	  lo = hi;
	  hi = 0;
	}

      /* Optimize if the shifted value is a (power of 2) - 1.  */
      if ((hi == 0 && ((lo + 1) & lo) == 0)
	  || (lo == 0xffffffff && ((hi + 1) & hi) == 0))
	{
	  shift = COUNT_TOP_ZEROES ((unsigned int) hi32.X_add_number);
	  if (shift != 0)
	    {
	      expressionS tmp;

	      /* This instruction will set the register to be all
                 ones.  */
	      tmp.X_op = O_constant;
	      tmp.X_add_number = (offsetT) -1;
	      macro_build (&tmp, "addiu", "t,r,j", reg, 0, BFD_RELOC_LO16);
	      if (bit != 0)
		{
		  bit += shift;
		  macro_build (NULL, (bit >= 32) ? "dsll32" : "dsll", SHFT_FMT,
			       reg, reg, (bit >= 32) ? bit - 32 : bit);
		}
	      macro_build (NULL, (shift >= 32) ? "dsrl32" : "dsrl", SHFT_FMT,
			   reg, reg, (shift >= 32) ? shift - 32 : shift);
	      return;
	    }
	}

      /* Sign extend hi32 before calling load_register, because we can
         generally get better code when we load a sign extended value.  */
      if ((hi32.X_add_number & 0x80000000) != 0)
	hi32.X_add_number |= ~(offsetT) 0xffffffff;
      load_register (reg, &hi32, 0);
      freg = reg;
    }
  if ((lo32.X_add_number & 0xffff0000) == 0)
    {
      if (freg != 0)
	{
	  macro_build (NULL, "dsll32", SHFT_FMT, reg, freg, 0);
	  freg = reg;
	}
    }
  else
    {
      expressionS mid16;

      if ((freg == 0) && (lo32.X_add_number == (offsetT) 0xffffffff))
	{
	  macro_build (&lo32, "lui", LUI_FMT, reg, BFD_RELOC_HI16);
	  macro_build (NULL, "dsrl32", SHFT_FMT, reg, reg, 0);
	  return;
	}

      if (freg != 0)
	{
	  macro_build (NULL, "dsll", SHFT_FMT, reg, freg, 16);
	  freg = reg;
	}
      mid16 = lo32;
      mid16.X_add_number >>= 16;
      macro_build (&mid16, "ori", "t,r,i", reg, freg, BFD_RELOC_LO16);
      macro_build (NULL, "dsll", SHFT_FMT, reg, reg, 16);
      freg = reg;
    }
  if ((lo32.X_add_number & 0xffff) != 0)
    macro_build (&lo32, "ori", "t,r,i", reg, freg, BFD_RELOC_LO16);
}

static inline void
load_delay_nop (void)
{
  if (!gpr_interlocks)
    macro_build (NULL, "nop", "");
}

/* Load an address into a register.  */

static void
load_address (int reg, expressionS *ep, int *used_at)
{
  if (ep->X_op != O_constant
      && ep->X_op != O_symbol)
    {
      as_bad (_("expression too complex"));
      ep->X_op = O_constant;
    }

  if (ep->X_op == O_constant)
    {
      load_register (reg, ep, HAVE_64BIT_ADDRESSES);
      return;
    }

  if (mips_pic == NO_PIC)
    {
      /* If this is a reference to a GP relative symbol, we want
	   addiu	$reg,$gp,<sym>		(BFD_RELOC_GPREL16)
	 Otherwise we want
	   lui		$reg,<sym>		(BFD_RELOC_HI16_S)
	   addiu	$reg,$reg,<sym>		(BFD_RELOC_LO16)
	 If we have an addend, we always use the latter form.

	 With 64bit address space and a usable $at we want
	   lui		$reg,<sym>		(BFD_RELOC_MIPS_HIGHEST)
	   lui		$at,<sym>		(BFD_RELOC_HI16_S)
	   daddiu	$reg,<sym>		(BFD_RELOC_MIPS_HIGHER)
	   daddiu	$at,<sym>		(BFD_RELOC_LO16)
	   dsll32	$reg,0
	   daddu	$reg,$reg,$at

	 If $at is already in use, we use a path which is suboptimal
	 on superscalar processors.
	   lui		$reg,<sym>		(BFD_RELOC_MIPS_HIGHEST)
	   daddiu	$reg,<sym>		(BFD_RELOC_MIPS_HIGHER)
	   dsll		$reg,16
	   daddiu	$reg,<sym>		(BFD_RELOC_HI16_S)
	   dsll		$reg,16
	   daddiu	$reg,<sym>		(BFD_RELOC_LO16)

	 For GP relative symbols in 64bit address space we can use
	 the same sequence as in 32bit address space.  */
      if (HAVE_64BIT_SYMBOLS)
	{
	  if ((valueT) ep->X_add_number <= MAX_GPREL_OFFSET
	      && !nopic_need_relax (ep->X_add_symbol, 1))
	    {
	      relax_start (ep->X_add_symbol);
	      macro_build (ep, ADDRESS_ADDI_INSN, "t,r,j", reg,
			   mips_gp_register, BFD_RELOC_GPREL16);
	      relax_switch ();
	    }

	  if (*used_at == 0 && mips_opts.at)
	    {
	      macro_build (ep, "lui", LUI_FMT, reg, BFD_RELOC_MIPS_HIGHEST);
	      macro_build (ep, "lui", LUI_FMT, AT, BFD_RELOC_HI16_S);
	      macro_build (ep, "daddiu", "t,r,j", reg, reg,
			   BFD_RELOC_MIPS_HIGHER);
	      macro_build (ep, "daddiu", "t,r,j", AT, AT, BFD_RELOC_LO16);
	      macro_build (NULL, "dsll32", SHFT_FMT, reg, reg, 0);
	      macro_build (NULL, "daddu", "d,v,t", reg, reg, AT);
	      *used_at = 1;
	    }
	  else
	    {
	      macro_build (ep, "lui", LUI_FMT, reg, BFD_RELOC_MIPS_HIGHEST);
	      macro_build (ep, "daddiu", "t,r,j", reg, reg,
			   BFD_RELOC_MIPS_HIGHER);
	      macro_build (NULL, "dsll", SHFT_FMT, reg, reg, 16);
	      macro_build (ep, "daddiu", "t,r,j", reg, reg, BFD_RELOC_HI16_S);
	      macro_build (NULL, "dsll", SHFT_FMT, reg, reg, 16);
	      macro_build (ep, "daddiu", "t,r,j", reg, reg, BFD_RELOC_LO16);
	    }

	  if (mips_relax.sequence)
	    relax_end ();
	}
      else
	{
	  if ((valueT) ep->X_add_number <= MAX_GPREL_OFFSET
	      && !nopic_need_relax (ep->X_add_symbol, 1))
	    {
	      relax_start (ep->X_add_symbol);
	      macro_build (ep, ADDRESS_ADDI_INSN, "t,r,j", reg,
			   mips_gp_register, BFD_RELOC_GPREL16);
	      relax_switch ();
	    }
	  macro_build_lui (ep, reg);
	  macro_build (ep, ADDRESS_ADDI_INSN, "t,r,j",
		       reg, reg, BFD_RELOC_LO16);
	  if (mips_relax.sequence)
	    relax_end ();
	}
    }
  else if (!mips_big_got)
    {
      expressionS ex;

      /* If this is a reference to an external symbol, we want
	   lw		$reg,<sym>($gp)		(BFD_RELOC_MIPS_GOT16)
	 Otherwise we want
	   lw		$reg,<sym>($gp)		(BFD_RELOC_MIPS_GOT16)
	   nop
	   addiu	$reg,$reg,<sym>		(BFD_RELOC_LO16)
	 If there is a constant, it must be added in after.

	 If we have NewABI, we want
	   lw		$reg,<sym+cst>($gp)	(BFD_RELOC_MIPS_GOT_DISP)
         unless we're referencing a global symbol with a non-zero
         offset, in which case cst must be added separately.  */
      if (HAVE_NEWABI)
	{
	  if (ep->X_add_number)
	    {
	      ex.X_add_number = ep->X_add_number;
	      ep->X_add_number = 0;
	      relax_start (ep->X_add_symbol);
	      macro_build (ep, ADDRESS_LOAD_INSN, "t,o(b)", reg,
			   BFD_RELOC_MIPS_GOT_DISP, mips_gp_register);
	      if (ex.X_add_number < -0x8000 || ex.X_add_number >= 0x8000)
		as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	      ex.X_op = O_constant;
	      macro_build (&ex, ADDRESS_ADDI_INSN, "t,r,j",
			   reg, reg, BFD_RELOC_LO16);
	      ep->X_add_number = ex.X_add_number;
	      relax_switch ();
	    }
	  macro_build (ep, ADDRESS_LOAD_INSN, "t,o(b)", reg,
		       BFD_RELOC_MIPS_GOT_DISP, mips_gp_register);
	  if (mips_relax.sequence)
	    relax_end ();
	}
      else
	{
	  ex.X_add_number = ep->X_add_number;
	  ep->X_add_number = 0;
	  macro_build (ep, ADDRESS_LOAD_INSN, "t,o(b)", reg,
		       BFD_RELOC_MIPS_GOT16, mips_gp_register);
	  load_delay_nop ();
	  relax_start (ep->X_add_symbol);
	  relax_switch ();
	  macro_build (ep, ADDRESS_ADDI_INSN, "t,r,j", reg, reg,
		       BFD_RELOC_LO16);
	  relax_end ();

	  if (ex.X_add_number != 0)
	    {
	      if (ex.X_add_number < -0x8000 || ex.X_add_number >= 0x8000)
		as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	      ex.X_op = O_constant;
	      macro_build (&ex, ADDRESS_ADDI_INSN, "t,r,j",
			   reg, reg, BFD_RELOC_LO16);
	    }
	}
    }
  else if (mips_big_got)
    {
      expressionS ex;

      /* This is the large GOT case.  If this is a reference to an
	 external symbol, we want
	   lui		$reg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	   addu		$reg,$reg,$gp
	   lw		$reg,<sym>($reg)	(BFD_RELOC_MIPS_GOT_LO16)

	 Otherwise, for a reference to a local symbol in old ABI, we want
	   lw		$reg,<sym>($gp)		(BFD_RELOC_MIPS_GOT16)
	   nop
	   addiu	$reg,$reg,<sym>		(BFD_RELOC_LO16)
	 If there is a constant, it must be added in after.

	 In the NewABI, for local symbols, with or without offsets, we want:
	   lw		$reg,<sym>($gp)		(BFD_RELOC_MIPS_GOT_PAGE)
	   addiu	$reg,$reg,<sym>		(BFD_RELOC_MIPS_GOT_OFST)
      */
      if (HAVE_NEWABI)
	{
	  ex.X_add_number = ep->X_add_number;
	  ep->X_add_number = 0;
	  relax_start (ep->X_add_symbol);
	  macro_build (ep, "lui", LUI_FMT, reg, BFD_RELOC_MIPS_GOT_HI16);
	  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
		       reg, reg, mips_gp_register);
	  macro_build (ep, ADDRESS_LOAD_INSN, "t,o(b)",
		       reg, BFD_RELOC_MIPS_GOT_LO16, reg);
	  if (ex.X_add_number < -0x8000 || ex.X_add_number >= 0x8000)
	    as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	  else if (ex.X_add_number)
	    {
	      ex.X_op = O_constant;
	      macro_build (&ex, ADDRESS_ADDI_INSN, "t,r,j", reg, reg,
			   BFD_RELOC_LO16);
	    }

	  ep->X_add_number = ex.X_add_number;
	  relax_switch ();
	  macro_build (ep, ADDRESS_LOAD_INSN, "t,o(b)", reg,
		       BFD_RELOC_MIPS_GOT_PAGE, mips_gp_register);
	  macro_build (ep, ADDRESS_ADDI_INSN, "t,r,j", reg, reg,
		       BFD_RELOC_MIPS_GOT_OFST);
	  relax_end ();
	}
      else
	{
	  ex.X_add_number = ep->X_add_number;
	  ep->X_add_number = 0;
	  relax_start (ep->X_add_symbol);
	  macro_build (ep, "lui", LUI_FMT, reg, BFD_RELOC_MIPS_GOT_HI16);
	  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
		       reg, reg, mips_gp_register);
	  macro_build (ep, ADDRESS_LOAD_INSN, "t,o(b)",
		       reg, BFD_RELOC_MIPS_GOT_LO16, reg);
	  relax_switch ();
	  if (reg_needs_delay (mips_gp_register))
	    {
	      /* We need a nop before loading from $gp.  This special
		 check is required because the lui which starts the main
		 instruction stream does not refer to $gp, and so will not
		 insert the nop which may be required.  */
	      macro_build (NULL, "nop", "");
	    }
	  macro_build (ep, ADDRESS_LOAD_INSN, "t,o(b)", reg,
		       BFD_RELOC_MIPS_GOT16, mips_gp_register);
	  load_delay_nop ();
	  macro_build (ep, ADDRESS_ADDI_INSN, "t,r,j", reg, reg,
		       BFD_RELOC_LO16);
	  relax_end ();

	  if (ex.X_add_number != 0)
	    {
	      if (ex.X_add_number < -0x8000 || ex.X_add_number >= 0x8000)
		as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	      ex.X_op = O_constant;
	      macro_build (&ex, ADDRESS_ADDI_INSN, "t,r,j", reg, reg,
			   BFD_RELOC_LO16);
	    }
	}
    }
  else
    abort ();

  if (!mips_opts.at && *used_at == 1)
    as_bad (_("macro used $at after \".set noat\""));
}

/* Move the contents of register SOURCE into register DEST.  */

static void
move_register (int dest, int source)
{
  /* Prefer to use a 16-bit microMIPS instruction unless the previous
     instruction specifically requires a 32-bit one.  */
  if (mips_opts.micromips
      && !mips_opts.insn32
      && !(history[0].insn_mo->pinfo2 & INSN2_BRANCH_DELAY_32BIT))
    macro_build (NULL, "move", "mp,mj", dest, source);
  else
    macro_build (NULL, "or", "d,v,t", dest, source, 0);
}

/* Emit an SVR4 PIC sequence to load address LOCAL into DEST, where
   LOCAL is the sum of a symbol and a 16-bit or 32-bit displacement.
   The two alternatives are:

   Global symbol		Local symbol
   -------------		------------
   lw DEST,%got(SYMBOL)		lw DEST,%got(SYMBOL + OFFSET)
   ...				...
   addiu DEST,DEST,OFFSET	addiu DEST,DEST,%lo(SYMBOL + OFFSET)

   load_got_offset emits the first instruction and add_got_offset
   emits the second for a 16-bit offset or add_got_offset_hilo emits
   a sequence to add a 32-bit offset using a scratch register.  */

static void
load_got_offset (int dest, expressionS *local)
{
  expressionS global;

  global = *local;
  global.X_add_number = 0;

  relax_start (local->X_add_symbol);
  macro_build (&global, ADDRESS_LOAD_INSN, "t,o(b)", dest,
	       BFD_RELOC_MIPS_GOT16, mips_gp_register);
  relax_switch ();
  macro_build (local, ADDRESS_LOAD_INSN, "t,o(b)", dest,
	       BFD_RELOC_MIPS_GOT16, mips_gp_register);
  relax_end ();
}

static void
add_got_offset (int dest, expressionS *local)
{
  expressionS global;

  global.X_op = O_constant;
  global.X_op_symbol = NULL;
  global.X_add_symbol = NULL;
  global.X_add_number = local->X_add_number;

  relax_start (local->X_add_symbol);
  macro_build (&global, ADDRESS_ADDI_INSN, "t,r,j",
	       dest, dest, BFD_RELOC_LO16);
  relax_switch ();
  macro_build (local, ADDRESS_ADDI_INSN, "t,r,j", dest, dest, BFD_RELOC_LO16);
  relax_end ();
}

static void
add_got_offset_hilo (int dest, expressionS *local, int tmp)
{
  expressionS global;
  int hold_mips_optimize;

  global.X_op = O_constant;
  global.X_op_symbol = NULL;
  global.X_add_symbol = NULL;
  global.X_add_number = local->X_add_number;

  relax_start (local->X_add_symbol);
  load_register (tmp, &global, HAVE_64BIT_ADDRESSES);
  relax_switch ();
  /* Set mips_optimize around the lui instruction to avoid
     inserting an unnecessary nop after the lw.  */
  hold_mips_optimize = mips_optimize;
  mips_optimize = 2;
  macro_build_lui (&global, tmp);
  mips_optimize = hold_mips_optimize;
  macro_build (local, ADDRESS_ADDI_INSN, "t,r,j", tmp, tmp, BFD_RELOC_LO16);
  relax_end ();

  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", dest, dest, tmp);
}

/* Emit a sequence of instructions to emulate a branch likely operation.
   BR is an ordinary branch corresponding to one to be emulated.  BRNEG
   is its complementing branch with the original condition negated.
   CALL is set if the original branch specified the link operation.
   EP, FMT, SREG and TREG specify the usual macro_build() parameters.

   Code like this is produced in the noreorder mode:

	BRNEG	<args>, 1f
	 nop
	b	<sym>
	 delay slot (executed only if branch taken)
    1:

   or, if CALL is set:

	BRNEG	<args>, 1f
	 nop
	bal	<sym>
	 delay slot (executed only if branch taken)
    1:

   In the reorder mode the delay slot would be filled with a nop anyway,
   so code produced is simply:

	BR	<args>, <sym>
	 nop

   This function is used when producing code for the microMIPS ASE that
   does not implement branch likely instructions in hardware.  */

static void
macro_build_branch_likely (const char *br, const char *brneg,
			   int call, expressionS *ep, const char *fmt,
			   unsigned int sreg, unsigned int treg)
{
  int noreorder = mips_opts.noreorder;
  expressionS expr1;

  gas_assert (mips_opts.micromips);
  start_noreorder ();
  if (noreorder)
    {
      micromips_label_expr (&expr1);
      macro_build (&expr1, brneg, fmt, sreg, treg);
      macro_build (NULL, "nop", "");
      macro_build (ep, call ? "bal" : "b", "p");

      /* Set to true so that append_insn adds a label.  */
      emit_branch_likely_macro = true;
    }
  else
    {
      macro_build (ep, br, fmt, sreg, treg);
      macro_build (NULL, "nop", "");
    }
  end_noreorder ();
}

/* Emit a coprocessor branch-likely macro specified by TYPE, using CC as
   the condition code tested.  EP specifies the branch target.  */

static void
macro_build_branch_ccl (int type, expressionS *ep, unsigned int cc)
{
  const int call = 0;
  const char *brneg;
  const char *br;

  switch (type)
    {
    case M_BC1FL:
      br = "bc1f";
      brneg = "bc1t";
      break;
    case M_BC1TL:
      br = "bc1t";
      brneg = "bc1f";
      break;
    case M_BC2FL:
      br = "bc2f";
      brneg = "bc2t";
      break;
    case M_BC2TL:
      br = "bc2t";
      brneg = "bc2f";
      break;
    default:
      abort ();
    }
  macro_build_branch_likely (br, brneg, call, ep, "N,p", cc, ZERO);
}

/* Emit a two-argument branch macro specified by TYPE, using SREG as
   the register tested.  EP specifies the branch target.  */

static void
macro_build_branch_rs (int type, expressionS *ep, unsigned int sreg)
{
  const char *brneg = NULL;
  const char *br;
  int call = 0;

  switch (type)
    {
    case M_BGEZ:
      br = "bgez";
      break;
    case M_BGEZL:
      br = mips_opts.micromips ? "bgez" : "bgezl";
      brneg = "bltz";
      break;
    case M_BGEZALL:
      gas_assert (mips_opts.micromips);
      br = mips_opts.insn32 ? "bgezal" : "bgezals";
      brneg = "bltz";
      call = 1;
      break;
    case M_BGTZ:
      br = "bgtz";
      break;
    case M_BGTZL:
      br = mips_opts.micromips ? "bgtz" : "bgtzl";
      brneg = "blez";
      break;
    case M_BLEZ:
      br = "blez";
      break;
    case M_BLEZL:
      br = mips_opts.micromips ? "blez" : "blezl";
      brneg = "bgtz";
      break;
    case M_BLTZ:
      br = "bltz";
      break;
    case M_BLTZL:
      br = mips_opts.micromips ? "bltz" : "bltzl";
      brneg = "bgez";
      break;
    case M_BLTZALL:
      gas_assert (mips_opts.micromips);
      br = mips_opts.insn32 ? "bltzal" : "bltzals";
      brneg = "bgez";
      call = 1;
      break;
    default:
      abort ();
    }
  if (mips_opts.micromips && brneg)
    macro_build_branch_likely (br, brneg, call, ep, "s,p", sreg, ZERO);
  else
    macro_build (ep, br, "s,p", sreg);
}

/* Emit a three-argument branch macro specified by TYPE, using SREG and
   TREG as the registers tested.  EP specifies the branch target.  */

static void
macro_build_branch_rsrt (int type, expressionS *ep,
			 unsigned int sreg, unsigned int treg)
{
  const char *brneg = NULL;
  const int call = 0;
  const char *br;

  switch (type)
    {
    case M_BEQ:
    case M_BEQ_I:
      br = "beq";
      break;
    case M_BEQL:
    case M_BEQL_I:
      br = mips_opts.micromips ? "beq" : "beql";
      brneg = "bne";
      break;
    case M_BNE:
    case M_BNE_I:
      br = "bne";
      break;
    case M_BNEL:
    case M_BNEL_I:
      br = mips_opts.micromips ? "bne" : "bnel";
      brneg = "beq";
      break;
    default:
      abort ();
    }
  if (mips_opts.micromips && brneg)
    macro_build_branch_likely (br, brneg, call, ep, "s,t,p", sreg, treg);
  else
    macro_build (ep, br, "s,t,p", sreg, treg);
}

/* Return the high part that should be loaded in order to make the low
   part of VALUE accessible using an offset of OFFBITS bits.  */

static offsetT
offset_high_part (offsetT value, unsigned int offbits)
{
  offsetT bias;
  addressT low_mask;

  if (offbits == 0)
    return value;
  bias = 1 << (offbits - 1);
  low_mask = bias * 2 - 1;
  return (value + bias) & ~low_mask;
}

/* Return true if the value stored in offset_expr and offset_reloc
   fits into a signed offset of OFFBITS bits.  RANGE is the maximum
   amount that the caller wants to add without inducing overflow
   and ALIGN is the known alignment of the value in bytes.  */

static bool
small_offset_p (unsigned int range, unsigned int align, unsigned int offbits)
{
  if (offbits == 16)
    {
      /* Accept any relocation operator if overflow isn't a concern.  */
      if (range < align && *offset_reloc != BFD_RELOC_UNUSED)
	return true;

      /* These relocations are guaranteed not to overflow in correct links.  */
      if (*offset_reloc == BFD_RELOC_MIPS_LITERAL
	  || gprel16_reloc_p (*offset_reloc))
	return true;
    }
  if (offset_expr.X_op == O_constant
      && offset_high_part (offset_expr.X_add_number, offbits) == 0
      && offset_high_part (offset_expr.X_add_number + range, offbits) == 0)
    return true;
  return false;
}

/*
 *			Build macros
 *   This routine implements the seemingly endless macro or synthesized
 * instructions and addressing modes in the mips assembly language. Many
 * of these macros are simple and are similar to each other. These could
 * probably be handled by some kind of table or grammar approach instead of
 * this verbose method. Others are not simple macros but are more like
 * optimizing code generation.
 *   One interesting optimization is when several store macros appear
 * consecutively that would load AT with the upper half of the same address.
 * The ensuing load upper instructions are omitted. This implies some kind
 * of global optimization. We currently only optimize within a single macro.
 *   For many of the load and store macros if the address is specified as a
 * constant expression in the first 64k of memory (ie ld $2,0x4000c) we
 * first load register 'at' with zero and use it as the base register. The
 * mips assembler simply uses register $zero. Just one tiny optimization
 * we're missing.
 */
static void
macro (struct mips_cl_insn *ip, char *str)
{
  const struct mips_operand_array *operands;
  unsigned int breg, i;
  unsigned int tempreg;
  int mask;
  int used_at = 0;
  expressionS label_expr;
  expressionS expr1;
  expressionS *ep;
  const char *s;
  const char *s2;
  const char *fmt;
  int likely = 0;
  int coproc = 0;
  int offbits = 16;
  int call = 0;
  int jals = 0;
  int dbl = 0;
  int imm = 0;
  int ust = 0;
  int lp = 0;
  int ll_sc_paired = 0;
  bool large_offset;
  int off;
  int hold_mips_optimize;
  unsigned int align;
  unsigned int op[MAX_OPERANDS];

  gas_assert (! mips_opts.mips16);

  operands = insn_operands (ip);
  for (i = 0; i < MAX_OPERANDS; i++)
    if (operands->operand[i])
      op[i] = insn_extract_operand (ip, operands->operand[i]);
    else
      op[i] = -1;

  mask = ip->insn_mo->mask;

  label_expr.X_op = O_constant;
  label_expr.X_op_symbol = NULL;
  label_expr.X_add_symbol = NULL;
  label_expr.X_add_number = 0;

  expr1.X_op = O_constant;
  expr1.X_op_symbol = NULL;
  expr1.X_add_symbol = NULL;
  expr1.X_add_number = 1;
  align = 1;

  switch (mask)
    {
    case M_DABS:
      dbl = 1;
      /* Fall through.  */
    case M_ABS:
      /*    bgez    $a0,1f
	    move    v0,$a0
	    sub     v0,$zero,$a0
	 1:
       */

      start_noreorder ();

      if (mips_opts.micromips)
	micromips_label_expr (&label_expr);
      else
	label_expr.X_add_number = 8;
      macro_build (&label_expr, "bgez", "s,p", op[1]);
      if (op[0] == op[1])
	macro_build (NULL, "nop", "");
      else
	move_register (op[0], op[1]);
      macro_build (NULL, dbl ? "dsub" : "sub", "d,v,t", op[0], 0, op[1]);
      if (mips_opts.micromips)
	micromips_add_label ();

      end_noreorder ();
      break;

    case M_ADD_I:
      s = "addi";
      s2 = "add";
      if (ISA_IS_R6 (mips_opts.isa))
	goto do_addi_i;
      else
	goto do_addi;
    case M_ADDU_I:
      s = "addiu";
      s2 = "addu";
      goto do_addi;
    case M_DADD_I:
      dbl = 1;
      s = "daddi";
      s2 = "dadd";
      if (!mips_opts.micromips && !ISA_IS_R6 (mips_opts.isa))
	goto do_addi;
      if (imm_expr.X_add_number >= -0x200
	  && imm_expr.X_add_number < 0x200
	  && !ISA_IS_R6 (mips_opts.isa))
	{
	  macro_build (NULL, s, "t,r,.", op[0], op[1],
		       (int) imm_expr.X_add_number);
	  break;
	}
      goto do_addi_i;
    case M_DADDU_I:
      dbl = 1;
      s = "daddiu";
      s2 = "daddu";
    do_addi:
      if (imm_expr.X_add_number >= -0x8000
	  && imm_expr.X_add_number < 0x8000)
	{
	  macro_build (&imm_expr, s, "t,r,j", op[0], op[1], BFD_RELOC_LO16);
	  break;
	}
    do_addi_i:
      used_at = 1;
      load_register (AT, &imm_expr, dbl);
      macro_build (NULL, s2, "d,v,t", op[0], op[1], AT);
      break;

    case M_AND_I:
      s = "andi";
      s2 = "and";
      goto do_bit;
    case M_OR_I:
      s = "ori";
      s2 = "or";
      goto do_bit;
    case M_NOR_I:
      s = "";
      s2 = "nor";
      goto do_bit;
    case M_XOR_I:
      s = "xori";
      s2 = "xor";
    do_bit:
      if (imm_expr.X_add_number >= 0
	  && imm_expr.X_add_number < 0x10000)
	{
	  if (mask != M_NOR_I)
	    macro_build (&imm_expr, s, "t,r,i", op[0], op[1], BFD_RELOC_LO16);
	  else
	    {
	      macro_build (&imm_expr, "ori", "t,r,i",
			   op[0], op[1], BFD_RELOC_LO16);
	      macro_build (NULL, "nor", "d,v,t", op[0], op[0], 0);
	    }
	  break;
	}

      used_at = 1;
      load_register (AT, &imm_expr, GPR_SIZE == 64);
      macro_build (NULL, s2, "d,v,t", op[0], op[1], AT);
      break;

    case M_BALIGN:
      switch (imm_expr.X_add_number)
	{
	case 0:
	  macro_build (NULL, "nop", "");
	  break;
	case 2:
	  macro_build (NULL, "packrl.ph", "d,s,t", op[0], op[0], op[1]);
	  break;
	case 1:
	case 3:
	  macro_build (NULL, "balign", "t,s,2", op[0], op[1],
		       (int) imm_expr.X_add_number);
	  break;
	default:
	  as_bad (_("BALIGN immediate not 0, 1, 2 or 3 (%lu)"),
		  (unsigned long) imm_expr.X_add_number);
	  break;
	}
      break;

    case M_BC1FL:
    case M_BC1TL:
    case M_BC2FL:
    case M_BC2TL:
      gas_assert (mips_opts.micromips);
      macro_build_branch_ccl (mask, &offset_expr,
			      EXTRACT_OPERAND (1, BCC, *ip));
      break;

    case M_BEQ_I:
    case M_BEQL_I:
    case M_BNE_I:
    case M_BNEL_I:
      if (imm_expr.X_add_number == 0)
	op[1] = 0;
      else
	{
	  op[1] = AT;
	  used_at = 1;
	  load_register (op[1], &imm_expr, GPR_SIZE == 64);
	}
      /* Fall through.  */
    case M_BEQL:
    case M_BNEL:
      macro_build_branch_rsrt (mask, &offset_expr, op[0], op[1]);
      break;

    case M_BGEL:
      likely = 1;
      /* Fall through.  */
    case M_BGE:
      if (op[1] == 0)
	macro_build_branch_rs (likely ? M_BGEZL : M_BGEZ, &offset_expr, op[0]);
      else if (op[0] == 0)
	macro_build_branch_rs (likely ? M_BLEZL : M_BLEZ, &offset_expr, op[1]);
      else
	{
	  used_at = 1;
	  macro_build (NULL, "slt", "d,v,t", AT, op[0], op[1]);
	  macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BGEZL:
    case M_BGEZALL:
    case M_BGTZL:
    case M_BLEZL:
    case M_BLTZL:
    case M_BLTZALL:
      macro_build_branch_rs (mask, &offset_expr, op[0]);
      break;

    case M_BGTL_I:
      likely = 1;
      /* Fall through.  */
    case M_BGT_I:
      /* Check for > max integer.  */
      if (imm_expr.X_add_number >= GPR_SMAX)
	{
	do_false:
	  /* Result is always false.  */
	  if (! likely)
	    macro_build (NULL, "nop", "");
	  else
	    macro_build_branch_rsrt (M_BNEL, &offset_expr, ZERO, ZERO);
	  break;
	}
      ++imm_expr.X_add_number;
      /* Fall through.  */
    case M_BGE_I:
    case M_BGEL_I:
      if (mask == M_BGEL_I)
	likely = 1;
      if (imm_expr.X_add_number == 0)
	{
	  macro_build_branch_rs (likely ? M_BGEZL : M_BGEZ,
				 &offset_expr, op[0]);
	  break;
	}
      if (imm_expr.X_add_number == 1)
	{
	  macro_build_branch_rs (likely ? M_BGTZL : M_BGTZ,
				 &offset_expr, op[0]);
	  break;
	}
      if (imm_expr.X_add_number <= GPR_SMIN)
	{
	do_true:
	  /* Result is always true.  */
	  as_warn (_("branch %s is always true"), ip->insn_mo->name);
	  macro_build (&offset_expr, "b", "p");
	  break;
	}
      used_at = 1;
      set_at (op[0], 0);
      macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
			       &offset_expr, AT, ZERO);
      break;

    case M_BGEUL:
      likely = 1;
      /* Fall through.  */
    case M_BGEU:
      if (op[1] == 0)
	goto do_true;
      else if (op[0] == 0)
	macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
				 &offset_expr, ZERO, op[1]);
      else
	{
	  used_at = 1;
	  macro_build (NULL, "sltu", "d,v,t", AT, op[0], op[1]);
	  macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BGTUL_I:
      likely = 1;
      /* Fall through.  */
    case M_BGTU_I:
      if (op[0] == 0
	  || (GPR_SIZE == 32
	      && imm_expr.X_add_number == -1))
	goto do_false;
      ++imm_expr.X_add_number;
      /* Fall through.  */
    case M_BGEU_I:
    case M_BGEUL_I:
      if (mask == M_BGEUL_I)
	likely = 1;
      if (imm_expr.X_add_number == 0)
	goto do_true;
      else if (imm_expr.X_add_number == 1)
	macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				 &offset_expr, op[0], ZERO);
      else
	{
	  used_at = 1;
	  set_at (op[0], 1);
	  macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BGTL:
      likely = 1;
      /* Fall through.  */
    case M_BGT:
      if (op[1] == 0)
	macro_build_branch_rs (likely ? M_BGTZL : M_BGTZ, &offset_expr, op[0]);
      else if (op[0] == 0)
	macro_build_branch_rs (likely ? M_BLTZL : M_BLTZ, &offset_expr, op[1]);
      else
	{
	  used_at = 1;
	  macro_build (NULL, "slt", "d,v,t", AT, op[1], op[0]);
	  macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BGTUL:
      likely = 1;
      /* Fall through.  */
    case M_BGTU:
      if (op[1] == 0)
	macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				 &offset_expr, op[0], ZERO);
      else if (op[0] == 0)
	goto do_false;
      else
	{
	  used_at = 1;
	  macro_build (NULL, "sltu", "d,v,t", AT, op[1], op[0]);
	  macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BLEL:
      likely = 1;
      /* Fall through.  */
    case M_BLE:
      if (op[1] == 0)
	macro_build_branch_rs (likely ? M_BLEZL : M_BLEZ, &offset_expr, op[0]);
      else if (op[0] == 0)
	macro_build_branch_rs (likely ? M_BGEZL : M_BGEZ, &offset_expr, op[1]);
      else
	{
	  used_at = 1;
	  macro_build (NULL, "slt", "d,v,t", AT, op[1], op[0]);
	  macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BLEL_I:
      likely = 1;
      /* Fall through.  */
    case M_BLE_I:
      if (imm_expr.X_add_number >= GPR_SMAX)
	goto do_true;
      ++imm_expr.X_add_number;
      /* Fall through.  */
    case M_BLT_I:
    case M_BLTL_I:
      if (mask == M_BLTL_I)
	likely = 1;
      if (imm_expr.X_add_number == 0)
	macro_build_branch_rs (likely ? M_BLTZL : M_BLTZ, &offset_expr, op[0]);
      else if (imm_expr.X_add_number == 1)
	macro_build_branch_rs (likely ? M_BLEZL : M_BLEZ, &offset_expr, op[0]);
      else
	{
	  used_at = 1;
	  set_at (op[0], 0);
	  macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BLEUL:
      likely = 1;
      /* Fall through.  */
    case M_BLEU:
      if (op[1] == 0)
	macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
				 &offset_expr, op[0], ZERO);
      else if (op[0] == 0)
	goto do_true;
      else
	{
	  used_at = 1;
	  macro_build (NULL, "sltu", "d,v,t", AT, op[1], op[0]);
	  macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BLEUL_I:
      likely = 1;
      /* Fall through.  */
    case M_BLEU_I:
      if (op[0] == 0
	  || (GPR_SIZE == 32
	      && imm_expr.X_add_number == -1))
	goto do_true;
      ++imm_expr.X_add_number;
      /* Fall through.  */
    case M_BLTU_I:
    case M_BLTUL_I:
      if (mask == M_BLTUL_I)
	likely = 1;
      if (imm_expr.X_add_number == 0)
	goto do_false;
      else if (imm_expr.X_add_number == 1)
	macro_build_branch_rsrt (likely ? M_BEQL : M_BEQ,
				 &offset_expr, op[0], ZERO);
      else
	{
	  used_at = 1;
	  set_at (op[0], 1);
	  macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BLTL:
      likely = 1;
      /* Fall through.  */
    case M_BLT:
      if (op[1] == 0)
	macro_build_branch_rs (likely ? M_BLTZL : M_BLTZ, &offset_expr, op[0]);
      else if (op[0] == 0)
	macro_build_branch_rs (likely ? M_BGTZL : M_BGTZ, &offset_expr, op[1]);
      else
	{
	  used_at = 1;
	  macro_build (NULL, "slt", "d,v,t", AT, op[0], op[1]);
	  macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_BLTUL:
      likely = 1;
      /* Fall through.  */
    case M_BLTU:
      if (op[1] == 0)
	goto do_false;
      else if (op[0] == 0)
	macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				 &offset_expr, ZERO, op[1]);
      else
	{
	  used_at = 1;
	  macro_build (NULL, "sltu", "d,v,t", AT, op[0], op[1]);
	  macro_build_branch_rsrt (likely ? M_BNEL : M_BNE,
				   &offset_expr, AT, ZERO);
	}
      break;

    case M_DDIV_3:
      dbl = 1;
      /* Fall through.  */
    case M_DIV_3:
      s = "mflo";
      goto do_div3;
    case M_DREM_3:
      dbl = 1;
      /* Fall through.  */
    case M_REM_3:
      s = "mfhi";
    do_div3:
      if (op[2] == 0)
	{
	  as_warn (_("divide by zero"));
	  if (mips_trap)
	    macro_build (NULL, "teq", TRAP_FMT, ZERO, ZERO, 7);
	  else
	    macro_build (NULL, "break", BRK_FMT, 7);
	  break;
	}

      start_noreorder ();
      if (mips_trap)
	{
	  macro_build (NULL, "teq", TRAP_FMT, op[2], ZERO, 7);
	  macro_build (NULL, dbl ? "ddiv" : "div", "z,s,t", op[1], op[2]);
	}
      else
	{
	  if (mips_opts.micromips)
	    micromips_label_expr (&label_expr);
	  else
	    label_expr.X_add_number = 8;
	  macro_build (&label_expr, "bne", "s,t,p", op[2], ZERO);
	  macro_build (NULL, dbl ? "ddiv" : "div", "z,s,t", op[1], op[2]);
	  macro_build (NULL, "break", BRK_FMT, 7);
	  if (mips_opts.micromips)
	    micromips_add_label ();
	}
      expr1.X_add_number = -1;
      used_at = 1;
      load_register (AT, &expr1, dbl);
      if (mips_opts.micromips)
	micromips_label_expr (&label_expr);
      else
	label_expr.X_add_number = mips_trap ? (dbl ? 12 : 8) : (dbl ? 20 : 16);
      macro_build (&label_expr, "bne", "s,t,p", op[2], AT);
      if (dbl)
	{
	  expr1.X_add_number = 1;
	  load_register (AT, &expr1, dbl);
	  macro_build (NULL, "dsll32", SHFT_FMT, AT, AT, 31);
	}
      else
	{
	  expr1.X_add_number = 0x80000000;
	  macro_build (&expr1, "lui", LUI_FMT, AT, BFD_RELOC_HI16);
	}
      if (mips_trap)
	{
	  macro_build (NULL, "teq", TRAP_FMT, op[1], AT, 6);
	  /* We want to close the noreorder block as soon as possible, so
	     that later insns are available for delay slot filling.  */
	  end_noreorder ();
	}
      else
	{
	  if (mips_opts.micromips)
	    micromips_label_expr (&label_expr);
	  else
	    label_expr.X_add_number = 8;
	  macro_build (&label_expr, "bne", "s,t,p", op[1], AT);
	  macro_build (NULL, "nop", "");

	  /* We want to close the noreorder block as soon as possible, so
	     that later insns are available for delay slot filling.  */
	  end_noreorder ();

	  macro_build (NULL, "break", BRK_FMT, 6);
	}
      if (mips_opts.micromips)
	micromips_add_label ();
      macro_build (NULL, s, MFHL_FMT, op[0]);
      break;

    case M_DIV_3I:
      s = "div";
      s2 = "mflo";
      goto do_divi;
    case M_DIVU_3I:
      s = "divu";
      s2 = "mflo";
      goto do_divi;
    case M_REM_3I:
      s = "div";
      s2 = "mfhi";
      goto do_divi;
    case M_REMU_3I:
      s = "divu";
      s2 = "mfhi";
      goto do_divi;
    case M_DDIV_3I:
      dbl = 1;
      s = "ddiv";
      s2 = "mflo";
      goto do_divi;
    case M_DDIVU_3I:
      dbl = 1;
      s = "ddivu";
      s2 = "mflo";
      goto do_divi;
    case M_DREM_3I:
      dbl = 1;
      s = "ddiv";
      s2 = "mfhi";
      goto do_divi;
    case M_DREMU_3I:
      dbl = 1;
      s = "ddivu";
      s2 = "mfhi";
    do_divi:
      if (imm_expr.X_add_number == 0)
	{
	  as_warn (_("divide by zero"));
	  if (mips_trap)
	    macro_build (NULL, "teq", TRAP_FMT, ZERO, ZERO, 7);
	  else
	    macro_build (NULL, "break", BRK_FMT, 7);
	  break;
	}
      if (imm_expr.X_add_number == 1)
	{
	  if (strcmp (s2, "mflo") == 0)
	    move_register (op[0], op[1]);
	  else
	    move_register (op[0], ZERO);
	  break;
	}
      if (imm_expr.X_add_number == -1 && s[strlen (s) - 1] != 'u')
	{
	  if (strcmp (s2, "mflo") == 0)
	    macro_build (NULL, dbl ? "dneg" : "neg", "d,w", op[0], op[1]);
	  else
	    move_register (op[0], ZERO);
	  break;
	}

      used_at = 1;
      load_register (AT, &imm_expr, dbl);
      macro_build (NULL, s, "z,s,t", op[1], AT);
      macro_build (NULL, s2, MFHL_FMT, op[0]);
      break;

    case M_DIVU_3:
      s = "divu";
      s2 = "mflo";
      goto do_divu3;
    case M_REMU_3:
      s = "divu";
      s2 = "mfhi";
      goto do_divu3;
    case M_DDIVU_3:
      s = "ddivu";
      s2 = "mflo";
      goto do_divu3;
    case M_DREMU_3:
      s = "ddivu";
      s2 = "mfhi";
    do_divu3:
      start_noreorder ();
      if (mips_trap)
	{
	  macro_build (NULL, "teq", TRAP_FMT, op[2], ZERO, 7);
	  macro_build (NULL, s, "z,s,t", op[1], op[2]);
	  /* We want to close the noreorder block as soon as possible, so
	     that later insns are available for delay slot filling.  */
	  end_noreorder ();
	}
      else
	{
	  if (mips_opts.micromips)
	    micromips_label_expr (&label_expr);
	  else
	    label_expr.X_add_number = 8;
	  macro_build (&label_expr, "bne", "s,t,p", op[2], ZERO);
	  macro_build (NULL, s, "z,s,t", op[1], op[2]);

	  /* We want to close the noreorder block as soon as possible, so
	     that later insns are available for delay slot filling.  */
	  end_noreorder ();
	  macro_build (NULL, "break", BRK_FMT, 7);
	  if (mips_opts.micromips)
	    micromips_add_label ();
	}
      macro_build (NULL, s2, MFHL_FMT, op[0]);
      break;

    case M_DLCA_AB:
      dbl = 1;
      /* Fall through.  */
    case M_LCA_AB:
      call = 1;
      goto do_la;
    case M_DLA_AB:
      dbl = 1;
      /* Fall through.  */
    case M_LA_AB:
    do_la:
      /* Load the address of a symbol into a register.  If breg is not
	 zero, we then add a base register to it.  */

      breg = op[2];
      if (dbl && GPR_SIZE == 32)
	as_warn (_("dla used to load 32-bit register; recommend using la "
		   "instead"));

      if (!dbl && HAVE_64BIT_OBJECTS)
	as_warn (_("la used to load 64-bit address; recommend using dla "
		   "instead"));

      if (small_offset_p (0, align, 16))
	{
	  macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j", op[0], breg,
		       -1, offset_reloc[0], offset_reloc[1], offset_reloc[2]);
	  break;
	}

      if (mips_opts.at && (op[0] == breg))
	{
	  tempreg = AT;
	  used_at = 1;
	}
      else
	tempreg = op[0];

      if (offset_expr.X_op != O_symbol
	  && offset_expr.X_op != O_constant)
	{
	  as_bad (_("expression too complex"));
	  offset_expr.X_op = O_constant;
	}

      if (offset_expr.X_op == O_constant)
	load_register (tempreg, &offset_expr, HAVE_64BIT_ADDRESSES);
      else if (mips_pic == NO_PIC)
	{
	  /* If this is a reference to a GP relative symbol, we want
	       addiu	$tempreg,$gp,<sym>	(BFD_RELOC_GPREL16)
	     Otherwise we want
	       lui	$tempreg,<sym>		(BFD_RELOC_HI16_S)
	       addiu	$tempreg,$tempreg,<sym>	(BFD_RELOC_LO16)
	     If we have a constant, we need two instructions anyhow,
	     so we may as well always use the latter form.

	     With 64bit address space and a usable $at we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHEST)
	       lui	$at,<sym>		(BFD_RELOC_HI16_S)
	       daddiu	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHER)
	       daddiu	$at,<sym>		(BFD_RELOC_LO16)
	       dsll32	$tempreg,0
	       daddu	$tempreg,$tempreg,$at

	     If $at is already in use, we use a path which is suboptimal
	     on superscalar processors.
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHEST)
	       daddiu	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHER)
	       dsll	$tempreg,16
	       daddiu	$tempreg,<sym>		(BFD_RELOC_HI16_S)
	       dsll	$tempreg,16
	       daddiu	$tempreg,<sym>		(BFD_RELOC_LO16)

	     For GP relative symbols in 64bit address space we can use
	     the same sequence as in 32bit address space.  */
	  if (HAVE_64BIT_SYMBOLS)
	    {
	      if ((valueT) offset_expr.X_add_number <= MAX_GPREL_OFFSET
		  && !nopic_need_relax (offset_expr.X_add_symbol, 1))
		{
		  relax_start (offset_expr.X_add_symbol);
		  macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			       tempreg, mips_gp_register, BFD_RELOC_GPREL16);
		  relax_switch ();
		}

	      if (used_at == 0 && mips_opts.at)
		{
		  macro_build (&offset_expr, "lui", LUI_FMT,
			       tempreg, BFD_RELOC_MIPS_HIGHEST);
		  macro_build (&offset_expr, "lui", LUI_FMT,
			       AT, BFD_RELOC_HI16_S);
		  macro_build (&offset_expr, "daddiu", "t,r,j",
			       tempreg, tempreg, BFD_RELOC_MIPS_HIGHER);
		  macro_build (&offset_expr, "daddiu", "t,r,j",
			       AT, AT, BFD_RELOC_LO16);
		  macro_build (NULL, "dsll32", SHFT_FMT, tempreg, tempreg, 0);
		  macro_build (NULL, "daddu", "d,v,t", tempreg, tempreg, AT);
		  used_at = 1;
		}
	      else
		{
		  macro_build (&offset_expr, "lui", LUI_FMT,
			       tempreg, BFD_RELOC_MIPS_HIGHEST);
		  macro_build (&offset_expr, "daddiu", "t,r,j",
			       tempreg, tempreg, BFD_RELOC_MIPS_HIGHER);
		  macro_build (NULL, "dsll", SHFT_FMT, tempreg, tempreg, 16);
		  macro_build (&offset_expr, "daddiu", "t,r,j",
			       tempreg, tempreg, BFD_RELOC_HI16_S);
		  macro_build (NULL, "dsll", SHFT_FMT, tempreg, tempreg, 16);
		  macro_build (&offset_expr, "daddiu", "t,r,j",
			       tempreg, tempreg, BFD_RELOC_LO16);
		}

	      if (mips_relax.sequence)
		relax_end ();
	    }
	  else
	    {
	      if ((valueT) offset_expr.X_add_number <= MAX_GPREL_OFFSET
		  && !nopic_need_relax (offset_expr.X_add_symbol, 1))
		{
		  relax_start (offset_expr.X_add_symbol);
		  macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			       tempreg, mips_gp_register, BFD_RELOC_GPREL16);
		  relax_switch ();
		}
	      if (!IS_SEXT_32BIT_NUM (offset_expr.X_add_number))
		as_bad (_("offset too large"));
	      macro_build_lui (&offset_expr, tempreg);
	      macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			   tempreg, tempreg, BFD_RELOC_LO16);
	      if (mips_relax.sequence)
		relax_end ();
	    }
	}
      else if (!mips_big_got && !HAVE_NEWABI)
	{
	  int lw_reloc_type = (int) BFD_RELOC_MIPS_GOT16;

	  /* If this is a reference to an external symbol, and there
	     is no constant, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	     or for lca or if tempreg is PIC_CALL_REG
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_CALL16)
	     For a local symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       nop
	       addiu	$tempreg,$tempreg,<sym>	(BFD_RELOC_LO16)

	     If we have a small constant, and this is a reference to
	     an external symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       nop
	       addiu	$tempreg,$tempreg,<constant>
	     For a local symbol, we want the same instruction
	     sequence, but we output a BFD_RELOC_LO16 reloc on the
	     addiu instruction.

	     If we have a large constant, and this is a reference to
	     an external symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       lui	$at,<hiconstant>
	       addiu	$at,$at,<loconstant>
	       addu	$tempreg,$tempreg,$at
	     For a local symbol, we want the same instruction
	     sequence, but we output a BFD_RELOC_LO16 reloc on the
	     addiu instruction.
	   */

	  if (offset_expr.X_add_number == 0)
	    {
	      if (mips_pic == SVR4_PIC
		  && breg == 0
		  && (call || tempreg == PIC_CALL_REG))
		lw_reloc_type = (int) BFD_RELOC_MIPS_CALL16;

	      relax_start (offset_expr.X_add_symbol);
	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
			   lw_reloc_type, mips_gp_register);
	      if (breg != 0)
		{
		  /* We're going to put in an addu instruction using
		     tempreg, so we may as well insert the nop right
		     now.  */
		  load_delay_nop ();
		}
	      relax_switch ();
	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
			   tempreg, BFD_RELOC_MIPS_GOT16, mips_gp_register);
	      load_delay_nop ();
	      macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			   tempreg, tempreg, BFD_RELOC_LO16);
	      relax_end ();
	      /* FIXME: If breg == 0, and the next instruction uses
		 $tempreg, then if this variant case is used an extra
		 nop will be generated.  */
	    }
	  else if (offset_expr.X_add_number >= -0x8000
		   && offset_expr.X_add_number < 0x8000)
	    {
	      load_got_offset (tempreg, &offset_expr);
	      load_delay_nop ();
	      add_got_offset (tempreg, &offset_expr);
	    }
	  else
	    {
	      expr1.X_add_number = offset_expr.X_add_number;
	      offset_expr.X_add_number =
		SEXT_16BIT (offset_expr.X_add_number);
	      load_got_offset (tempreg, &offset_expr);
	      offset_expr.X_add_number = expr1.X_add_number;
	      /* If we are going to add in a base register, and the
		 target register and the base register are the same,
		 then we are using AT as a temporary register.  Since
		 we want to load the constant into AT, we add our
		 current AT (from the global offset table) and the
		 register into the register now, and pretend we were
		 not using a base register.  */
	      if (breg == op[0])
		{
		  load_delay_nop ();
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			       op[0], AT, breg);
		  breg = 0;
		  tempreg = op[0];
		}
	      add_got_offset_hilo (tempreg, &offset_expr, AT);
	      used_at = 1;
	    }
	}
      else if (!mips_big_got && HAVE_NEWABI)
	{
	  int add_breg_early = 0;

	  /* If this is a reference to an external, and there is no
	     constant, or local symbol (*), with or without a
	     constant, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT_DISP)
	     or for lca or if tempreg is PIC_CALL_REG
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_CALL16)

	     If we have a small constant, and this is a reference to
	     an external symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT_DISP)
	       addiu	$tempreg,$tempreg,<constant>

	     If we have a large constant, and this is a reference to
	     an external symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT_DISP)
	       lui	$at,<hiconstant>
	       addiu	$at,$at,<loconstant>
	       addu	$tempreg,$tempreg,$at

	     (*) Other assemblers seem to prefer GOT_PAGE/GOT_OFST for
	     local symbols, even though it introduces an additional
	     instruction.  */

	  if (offset_expr.X_add_number)
	    {
	      expr1.X_add_number = offset_expr.X_add_number;
	      offset_expr.X_add_number = 0;

	      relax_start (offset_expr.X_add_symbol);
	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
			   BFD_RELOC_MIPS_GOT_DISP, mips_gp_register);

	      if (expr1.X_add_number >= -0x8000
		  && expr1.X_add_number < 0x8000)
		{
		  macro_build (&expr1, ADDRESS_ADDI_INSN, "t,r,j",
			       tempreg, tempreg, BFD_RELOC_LO16);
		}
	      else if (IS_SEXT_32BIT_NUM (expr1.X_add_number + 0x8000))
		{
		  unsigned int dreg;

		  /* If we are going to add in a base register, and the
		     target register and the base register are the same,
		     then we are using AT as a temporary register.  Since
		     we want to load the constant into AT, we add our
		     current AT (from the global offset table) and the
		     register into the register now, and pretend we were
		     not using a base register.  */
		  if (breg != op[0])
		    dreg = tempreg;
		  else
		    {
		      gas_assert (tempreg == AT);
		      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
				   op[0], AT, breg);
		      dreg = op[0];
		      add_breg_early = 1;
		    }

		  load_register (AT, &expr1, HAVE_64BIT_ADDRESSES);
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			       dreg, dreg, AT);

		  used_at = 1;
		}
	      else
		as_bad (_("PIC code offset overflow (max 32 signed bits)"));

	      relax_switch ();
	      offset_expr.X_add_number = expr1.X_add_number;

	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
			   BFD_RELOC_MIPS_GOT_DISP, mips_gp_register);
	      if (add_breg_early)
		{
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			       op[0], tempreg, breg);
		  breg = 0;
		  tempreg = op[0];
		}
	      relax_end ();
	    }
	  else if (breg == 0 && (call || tempreg == PIC_CALL_REG))
	    {
	      relax_start (offset_expr.X_add_symbol);
	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
			   BFD_RELOC_MIPS_CALL16, mips_gp_register);
	      relax_switch ();
	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
			   BFD_RELOC_MIPS_GOT_DISP, mips_gp_register);
	      relax_end ();
	    }
	  else
	    {
	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
			   BFD_RELOC_MIPS_GOT_DISP, mips_gp_register);
	    }
	}
      else if (mips_big_got && !HAVE_NEWABI)
	{
	  int gpdelay;
	  int lui_reloc_type = (int) BFD_RELOC_MIPS_GOT_HI16;
	  int lw_reloc_type = (int) BFD_RELOC_MIPS_GOT_LO16;
	  int local_reloc_type = (int) BFD_RELOC_MIPS_GOT16;

	  /* This is the large GOT case.  If this is a reference to an
	     external symbol, and there is no constant, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       addu	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_GOT_LO16)
	     or for lca or if tempreg is PIC_CALL_REG
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_CALL_HI16)
	       addu	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_CALL_LO16)
	     For a local symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       nop
	       addiu	$tempreg,$tempreg,<sym>	(BFD_RELOC_LO16)

	     If we have a small constant, and this is a reference to
	     an external symbol, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       addu	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_GOT_LO16)
	       nop
	       addiu	$tempreg,$tempreg,<constant>
	     For a local symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       nop
	       addiu	$tempreg,$tempreg,<constant> (BFD_RELOC_LO16)

	     If we have a large constant, and this is a reference to
	     an external symbol, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       addu	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_GOT_LO16)
	       lui	$at,<hiconstant>
	       addiu	$at,$at,<loconstant>
	       addu	$tempreg,$tempreg,$at
	     For a local symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       lui	$at,<hiconstant>
	       addiu	$at,$at,<loconstant>	(BFD_RELOC_LO16)
	       addu	$tempreg,$tempreg,$at
	  */

	  expr1.X_add_number = offset_expr.X_add_number;
	  offset_expr.X_add_number = 0;
	  relax_start (offset_expr.X_add_symbol);
	  gpdelay = reg_needs_delay (mips_gp_register);
	  if (expr1.X_add_number == 0 && breg == 0
	      && (call || tempreg == PIC_CALL_REG))
	    {
	      lui_reloc_type = (int) BFD_RELOC_MIPS_CALL_HI16;
	      lw_reloc_type = (int) BFD_RELOC_MIPS_CALL_LO16;
	    }
	  macro_build (&offset_expr, "lui", LUI_FMT, tempreg, lui_reloc_type);
	  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
		       tempreg, tempreg, mips_gp_register);
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
		       tempreg, lw_reloc_type, tempreg);
	  if (expr1.X_add_number == 0)
	    {
	      if (breg != 0)
		{
		  /* We're going to put in an addu instruction using
		     tempreg, so we may as well insert the nop right
		     now.  */
		  load_delay_nop ();
		}
	    }
	  else if (expr1.X_add_number >= -0x8000
		   && expr1.X_add_number < 0x8000)
	    {
	      load_delay_nop ();
	      macro_build (&expr1, ADDRESS_ADDI_INSN, "t,r,j",
			   tempreg, tempreg, BFD_RELOC_LO16);
	    }
	  else
	    {
	      unsigned int dreg;

	      /* If we are going to add in a base register, and the
		 target register and the base register are the same,
		 then we are using AT as a temporary register.  Since
		 we want to load the constant into AT, we add our
		 current AT (from the global offset table) and the
		 register into the register now, and pretend we were
		 not using a base register.  */
	      if (breg != op[0])
		dreg = tempreg;
	      else
		{
		  gas_assert (tempreg == AT);
		  load_delay_nop ();
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			       op[0], AT, breg);
		  dreg = op[0];
		}

	      load_register (AT, &expr1, HAVE_64BIT_ADDRESSES);
	      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", dreg, dreg, AT);

	      used_at = 1;
	    }
	  offset_expr.X_add_number = SEXT_16BIT (expr1.X_add_number);
	  relax_switch ();

	  if (gpdelay)
	    {
	      /* This is needed because this instruction uses $gp, but
		 the first instruction on the main stream does not.  */
	      macro_build (NULL, "nop", "");
	    }

	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
		       local_reloc_type, mips_gp_register);
	  if (expr1.X_add_number >= -0x8000
	      && expr1.X_add_number < 0x8000)
	    {
	      load_delay_nop ();
	      macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			   tempreg, tempreg, BFD_RELOC_LO16);
	      /* FIXME: If add_number is 0, and there was no base
		 register, the external symbol case ended with a load,
		 so if the symbol turns out to not be external, and
		 the next instruction uses tempreg, an unnecessary nop
		 will be inserted.  */
	    }
	  else
	    {
	      if (breg == op[0])
		{
		  /* We must add in the base register now, as in the
		     external symbol case.  */
		  gas_assert (tempreg == AT);
		  load_delay_nop ();
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			       op[0], AT, breg);
		  tempreg = op[0];
		  /* We set breg to 0 because we have arranged to add
		     it in in both cases.  */
		  breg = 0;
		}

	      macro_build_lui (&expr1, AT);
	      macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			   AT, AT, BFD_RELOC_LO16);
	      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			   tempreg, tempreg, AT);
	      used_at = 1;
	    }
	  relax_end ();
	}
      else if (mips_big_got && HAVE_NEWABI)
	{
	  int lui_reloc_type = (int) BFD_RELOC_MIPS_GOT_HI16;
	  int lw_reloc_type = (int) BFD_RELOC_MIPS_GOT_LO16;
	  int add_breg_early = 0;

	  /* This is the large GOT case.  If this is a reference to an
	     external symbol, and there is no constant, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       add	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_GOT_LO16)
	     or for lca or if tempreg is PIC_CALL_REG
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_CALL_HI16)
	       add	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_CALL_LO16)

	     If we have a small constant, and this is a reference to
	     an external symbol, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       add	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_GOT_LO16)
	       addi	$tempreg,$tempreg,<constant>

	     If we have a large constant, and this is a reference to
	     an external symbol, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       addu	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_GOT_LO16)
	       lui	$at,<hiconstant>
	       addi	$at,$at,<loconstant>
	       add	$tempreg,$tempreg,$at

	     If we have NewABI, and we know it's a local symbol, we want
	       lw	$reg,<sym>($gp)		(BFD_RELOC_MIPS_GOT_PAGE)
	       addiu	$reg,$reg,<sym>		(BFD_RELOC_MIPS_GOT_OFST)
	     otherwise we have to resort to GOT_HI16/GOT_LO16.  */

	  relax_start (offset_expr.X_add_symbol);

	  expr1.X_add_number = offset_expr.X_add_number;
	  offset_expr.X_add_number = 0;

	  if (expr1.X_add_number == 0 && breg == 0
	      && (call || tempreg == PIC_CALL_REG))
	    {
	      lui_reloc_type = (int) BFD_RELOC_MIPS_CALL_HI16;
	      lw_reloc_type = (int) BFD_RELOC_MIPS_CALL_LO16;
	    }
	  macro_build (&offset_expr, "lui", LUI_FMT, tempreg, lui_reloc_type);
	  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
		       tempreg, tempreg, mips_gp_register);
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
		       tempreg, lw_reloc_type, tempreg);

	  if (expr1.X_add_number == 0)
	    ;
	  else if (expr1.X_add_number >= -0x8000
		   && expr1.X_add_number < 0x8000)
	    {
	      macro_build (&expr1, ADDRESS_ADDI_INSN, "t,r,j",
			   tempreg, tempreg, BFD_RELOC_LO16);
	    }
	  else if (IS_SEXT_32BIT_NUM (expr1.X_add_number + 0x8000))
	    {
	      unsigned int dreg;

	      /* If we are going to add in a base register, and the
		 target register and the base register are the same,
		 then we are using AT as a temporary register.  Since
		 we want to load the constant into AT, we add our
		 current AT (from the global offset table) and the
		 register into the register now, and pretend we were
		 not using a base register.  */
	      if (breg != op[0])
		dreg = tempreg;
	      else
		{
		  gas_assert (tempreg == AT);
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			       op[0], AT, breg);
		  dreg = op[0];
		  add_breg_early = 1;
		}

	      load_register (AT, &expr1, HAVE_64BIT_ADDRESSES);
	      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", dreg, dreg, AT);

	      used_at = 1;
	    }
	  else
	    as_bad (_("PIC code offset overflow (max 32 signed bits)"));

	  relax_switch ();
	  offset_expr.X_add_number = expr1.X_add_number;
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
		       BFD_RELOC_MIPS_GOT_PAGE, mips_gp_register);
	  macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j", tempreg,
		       tempreg, BFD_RELOC_MIPS_GOT_OFST);
	  if (add_breg_early)
	    {
	      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			   op[0], tempreg, breg);
	      breg = 0;
	      tempreg = op[0];
	    }
	  relax_end ();
	}
      else
	abort ();

      if (breg != 0)
	macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", op[0], tempreg, breg);
      break;

    case M_MSGSND:
      gas_assert (!mips_opts.micromips);
      macro_build (NULL, "c2", "C", (op[0] << 16) | 0x01);
      break;

    case M_MSGLD:
      gas_assert (!mips_opts.micromips);
      macro_build (NULL, "c2", "C", 0x02);
      break;

    case M_MSGLD_T:
      gas_assert (!mips_opts.micromips);
      macro_build (NULL, "c2", "C", (op[0] << 16) | 0x02);
      break;

    case M_MSGWAIT:
      gas_assert (!mips_opts.micromips);
      macro_build (NULL, "c2", "C", 3);
      break;

    case M_MSGWAIT_T:
      gas_assert (!mips_opts.micromips);
      macro_build (NULL, "c2", "C", (op[0] << 16) | 0x03);
      break;

    case M_J_A:
      /* The j instruction may not be used in PIC code, since it
	 requires an absolute address.  We convert it to a b
	 instruction.  */
      if (mips_pic == NO_PIC)
	macro_build (&offset_expr, "j", "a");
      else
	macro_build (&offset_expr, "b", "p");
      break;

      /* The jal instructions must be handled as macros because when
	 generating PIC code they expand to multi-instruction
	 sequences.  Normally they are simple instructions.  */
    case M_JALS_1:
      op[1] = op[0];
      op[0] = RA;
      /* Fall through.  */
    case M_JALS_2:
      gas_assert (mips_opts.micromips);
      if (mips_opts.insn32)
	{
	  as_bad (_("opcode not supported in the `insn32' mode `%s'"), str);
	  break;
	}
      jals = 1;
      goto jal;
    case M_JAL_1:
      op[1] = op[0];
      op[0] = RA;
      /* Fall through.  */
    case M_JAL_2:
    jal:
      if (mips_pic == NO_PIC)
	{
	  s = jals ? "jalrs" : "jalr";
	  if (mips_opts.micromips
	      && !mips_opts.insn32
	      && op[0] == RA
	      && !(history[0].insn_mo->pinfo2 & INSN2_BRANCH_DELAY_32BIT))
	    macro_build (NULL, s, "mj", op[1]);
	  else
	    macro_build (NULL, s, JALR_FMT, op[0], op[1]);
	}
      else
	{
	  int cprestore = (mips_pic == SVR4_PIC && !HAVE_NEWABI
			   && mips_cprestore_offset >= 0);

	  if (op[1] != PIC_CALL_REG)
	    as_warn (_("MIPS PIC call to register other than $25"));

	  s = ((mips_opts.micromips
		&& !mips_opts.insn32
		&& (!mips_opts.noreorder || cprestore))
	       ? "jalrs" : "jalr");
	  if (mips_opts.micromips
	      && !mips_opts.insn32
	      && op[0] == RA
	      && !(history[0].insn_mo->pinfo2 & INSN2_BRANCH_DELAY_32BIT))
	    macro_build (NULL, s, "mj", op[1]);
	  else
	    macro_build (NULL, s, JALR_FMT, op[0], op[1]);
	  if (mips_pic == SVR4_PIC && !HAVE_NEWABI)
	    {
	      if (mips_cprestore_offset < 0)
		as_warn (_("no .cprestore pseudo-op used in PIC code"));
	      else
		{
		  if (!mips_frame_reg_valid)
		    {
		      as_warn (_("no .frame pseudo-op used in PIC code"));
		      /* Quiet this warning.  */
		      mips_frame_reg_valid = 1;
		    }
		  if (!mips_cprestore_valid)
		    {
		      as_warn (_("no .cprestore pseudo-op used in PIC code"));
		      /* Quiet this warning.  */
		      mips_cprestore_valid = 1;
		    }
		  if (mips_opts.noreorder)
		    macro_build (NULL, "nop", "");
		  expr1.X_add_number = mips_cprestore_offset;
		  macro_build_ldst_constoffset (&expr1, ADDRESS_LOAD_INSN,
						mips_gp_register,
						mips_frame_reg,
						HAVE_64BIT_ADDRESSES);
		}
	    }
	}

      break;

    case M_JALS_A:
      gas_assert (mips_opts.micromips);
      if (mips_opts.insn32)
	{
	  as_bad (_("opcode not supported in the `insn32' mode `%s'"), str);
	  break;
	}
      jals = 1;
      /* Fall through.  */
    case M_JAL_A:
      if (mips_pic == NO_PIC)
	macro_build (&offset_expr, jals ? "jals" : "jal", "a");
      else if (mips_pic == SVR4_PIC)
	{
	  /* If this is a reference to an external symbol, and we are
	     using a small GOT, we want
	       lw	$25,<sym>($gp)		(BFD_RELOC_MIPS_CALL16)
	       nop
	       jalr	$ra,$25
	       nop
	       lw	$gp,cprestore($sp)
	     The cprestore value is set using the .cprestore
	     pseudo-op.  If we are using a big GOT, we want
	       lui	$25,<sym>		(BFD_RELOC_MIPS_CALL_HI16)
	       addu	$25,$25,$gp
	       lw	$25,<sym>($25)		(BFD_RELOC_MIPS_CALL_LO16)
	       nop
	       jalr	$ra,$25
	       nop
	       lw	$gp,cprestore($sp)
	     If the symbol is not external, we want
	       lw	$25,<sym>($gp)		(BFD_RELOC_MIPS_GOT16)
	       nop
	       addiu	$25,$25,<sym>		(BFD_RELOC_LO16)
	       jalr	$ra,$25
	       nop
	       lw $gp,cprestore($sp)

	     For NewABI, we use the same CALL16 or CALL_HI16/CALL_LO16
	     sequences above, minus nops, unless the symbol is local,
	     which enables us to use GOT_PAGE/GOT_OFST (big got) or
	     GOT_DISP.  */
	  if (HAVE_NEWABI)
	    {
	      if (!mips_big_got)
		{
		  relax_start (offset_expr.X_add_symbol);
		  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
			       PIC_CALL_REG, BFD_RELOC_MIPS_CALL16,
			       mips_gp_register);
		  relax_switch ();
		  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
			       PIC_CALL_REG, BFD_RELOC_MIPS_GOT_DISP,
			       mips_gp_register);
		  relax_end ();
		}
	      else
		{
		  relax_start (offset_expr.X_add_symbol);
		  macro_build (&offset_expr, "lui", LUI_FMT, PIC_CALL_REG,
			       BFD_RELOC_MIPS_CALL_HI16);
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", PIC_CALL_REG,
			       PIC_CALL_REG, mips_gp_register);
		  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
			       PIC_CALL_REG, BFD_RELOC_MIPS_CALL_LO16,
			       PIC_CALL_REG);
		  relax_switch ();
		  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
			       PIC_CALL_REG, BFD_RELOC_MIPS_GOT_PAGE,
			       mips_gp_register);
		  macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			       PIC_CALL_REG, PIC_CALL_REG,
			       BFD_RELOC_MIPS_GOT_OFST);
		  relax_end ();
		}

	      macro_build_jalr (&offset_expr, 0);
	    }
	  else
	    {
	      relax_start (offset_expr.X_add_symbol);
	      if (!mips_big_got)
		{
		  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
			       PIC_CALL_REG, BFD_RELOC_MIPS_CALL16,
			       mips_gp_register);
		  load_delay_nop ();
		  relax_switch ();
		}
	      else
		{
		  int gpdelay;

		  gpdelay = reg_needs_delay (mips_gp_register);
		  macro_build (&offset_expr, "lui", LUI_FMT, PIC_CALL_REG,
			       BFD_RELOC_MIPS_CALL_HI16);
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", PIC_CALL_REG,
			       PIC_CALL_REG, mips_gp_register);
		  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
			       PIC_CALL_REG, BFD_RELOC_MIPS_CALL_LO16,
			       PIC_CALL_REG);
		  load_delay_nop ();
		  relax_switch ();
		  if (gpdelay)
		    macro_build (NULL, "nop", "");
		}
	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
			   PIC_CALL_REG, BFD_RELOC_MIPS_GOT16,
			   mips_gp_register);
	      load_delay_nop ();
	      macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			   PIC_CALL_REG, PIC_CALL_REG, BFD_RELOC_LO16);
	      relax_end ();
	      macro_build_jalr (&offset_expr, mips_cprestore_offset >= 0);

	      if (mips_cprestore_offset < 0)
		as_warn (_("no .cprestore pseudo-op used in PIC code"));
	      else
		{
		  if (!mips_frame_reg_valid)
		    {
		      as_warn (_("no .frame pseudo-op used in PIC code"));
		      /* Quiet this warning.  */
		      mips_frame_reg_valid = 1;
		    }
		  if (!mips_cprestore_valid)
		    {
		      as_warn (_("no .cprestore pseudo-op used in PIC code"));
		      /* Quiet this warning.  */
		      mips_cprestore_valid = 1;
		    }
		  if (mips_opts.noreorder)
		    macro_build (NULL, "nop", "");
		  expr1.X_add_number = mips_cprestore_offset;
		  macro_build_ldst_constoffset (&expr1, ADDRESS_LOAD_INSN,
						mips_gp_register,
						mips_frame_reg,
						HAVE_64BIT_ADDRESSES);
		}
	    }
	}
      else if (mips_pic == VXWORKS_PIC)
	as_bad (_("non-PIC jump used in PIC library"));
      else
	abort ();

      break;

    case M_LBUE_AB:
      s = "lbue";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_LHUE_AB:
      s = "lhue";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_LBE_AB:
      s = "lbe";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_LHE_AB:
      s = "lhe";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_LLE_AB:
      s = "lle";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_LWE_AB:
      s = "lwe";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_LWLE_AB:
      s = "lwle";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_LWRE_AB:
      s = "lwre";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_SBE_AB:
      s = "sbe";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_SCE_AB:
      s = "sce";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_SHE_AB:
      s = "she";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_SWE_AB:
      s = "swe";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_SWLE_AB:
      s = "swle";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_SWRE_AB:
      s = "swre";
      fmt = "t,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_ACLR_AB:
      s = "aclr";
      fmt = "\\,~(b)";
      offbits = 12;
      goto ld_st;
    case M_ASET_AB:
      s = "aset";
      fmt = "\\,~(b)";
      offbits = 12;
      goto ld_st;
    case M_LB_AB:
      s = "lb";
      fmt = "t,o(b)";
      goto ld;
    case M_LBU_AB:
      s = "lbu";
      fmt = "t,o(b)";
      goto ld;
    case M_LH_AB:
      s = "lh";
      fmt = "t,o(b)";
      goto ld;
    case M_LHU_AB:
      s = "lhu";
      fmt = "t,o(b)";
      goto ld;
    case M_LW_AB:
      s = "lw";
      fmt = "t,o(b)";
      goto ld;
    case M_LWC0_AB:
      gas_assert (!mips_opts.micromips);
      s = "lwc0";
      fmt = "E,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_LWC1_AB:
      s = "lwc1";
      fmt = "T,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_LWC2_AB:
      s = "lwc2";
      fmt = COP12_FMT;
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 11
		 : 16);
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_LWC3_AB:
      gas_assert (!mips_opts.micromips);
      s = "lwc3";
      fmt = "E,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_LWL_AB:
      s = "lwl";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld_st;
    case M_LWR_AB:
      s = "lwr";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld_st;
    case M_LDC1_AB:
      s = "ldc1";
      fmt = "T,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_LDC2_AB:
      s = "ldc2";
      fmt = COP12_FMT;
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 11
		 : 16);
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_LQC2_AB:
      s = "lqc2";
      fmt = "+7,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_LDC3_AB:
      s = "ldc3";
      fmt = "E,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_LDL_AB:
      s = "ldl";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld_st;
    case M_LDR_AB:
      s = "ldr";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld_st;
    case M_LL_AB:
      s = "ll";
      fmt = LL_SC_FMT;
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 9
		 : 16);
      goto ld;
    case M_LLD_AB:
      s = "lld";
      fmt = LL_SC_FMT;
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 9
		 : 16);
      goto ld;
    case M_LWU_AB:
      s = "lwu";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld;
    case M_LWP_AB:
      gas_assert (mips_opts.micromips);
      s = "lwp";
      fmt = "t,~(b)";
      offbits = 12;
      lp = 1;
      goto ld;
    case M_LDP_AB:
      gas_assert (mips_opts.micromips);
      s = "ldp";
      fmt = "t,~(b)";
      offbits = 12;
      lp = 1;
      goto ld;
    case M_LLDP_AB:
    case M_LLWP_AB:
    case M_LLWPE_AB:
      s = ip->insn_mo->name;
      fmt = "t,d,s";
      ll_sc_paired = 1;
      offbits = 0;
      goto ld;
    case M_LWM_AB:
      gas_assert (mips_opts.micromips);
      s = "lwm";
      fmt = "n,~(b)";
      offbits = 12;
      goto ld_st;
    case M_LDM_AB:
      gas_assert (mips_opts.micromips);
      s = "ldm";
      fmt = "n,~(b)";
      offbits = 12;
      goto ld_st;

    ld:
      /* Try to use one the the load registers to compute the base address.
	 We don't want to use $0 as tempreg.  */
      if (ll_sc_paired)
	{
	  if ((op[0] == ZERO && op[3] == op[1])
	      || (op[1] == ZERO && op[3] == op[0])
	      || (op[0] == ZERO && op[1] == ZERO))
	    goto ld_st;
	  else if (op[0] != op[3] && op[0] != ZERO)
	    tempreg = op[0];
	  else
	    tempreg = op[1];
	}
      else
        {
	  if (op[2] == op[0] + lp || op[0] + lp == ZERO)
	    goto ld_st;
	  else
	    tempreg = op[0] + lp;
	}
      goto ld_noat;

    case M_SB_AB:
      s = "sb";
      fmt = "t,o(b)";
      goto ld_st;
    case M_SH_AB:
      s = "sh";
      fmt = "t,o(b)";
      goto ld_st;
    case M_SW_AB:
      s = "sw";
      fmt = "t,o(b)";
      goto ld_st;
    case M_SWC0_AB:
      gas_assert (!mips_opts.micromips);
      s = "swc0";
      fmt = "E,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_SWC1_AB:
      s = "swc1";
      fmt = "T,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_SWC2_AB:
      s = "swc2";
      fmt = COP12_FMT;
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 11
		 : 16);
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_SWC3_AB:
      gas_assert (!mips_opts.micromips);
      s = "swc3";
      fmt = "E,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_SWL_AB:
      s = "swl";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld_st;
    case M_SWR_AB:
      s = "swr";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld_st;
    case M_SC_AB:
      s = "sc";
      fmt = LL_SC_FMT;
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 9
		 : 16);
      goto ld_st;
    case M_SCD_AB:
      s = "scd";
      fmt = LL_SC_FMT;
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 9
		 : 16);
      goto ld_st;
    case M_SCDP_AB:
    case M_SCWP_AB:
    case M_SCWPE_AB:
      s = ip->insn_mo->name;
      fmt = "t,d,s";
      ll_sc_paired = 1;
      offbits = 0;
      goto ld_st;
    case M_CACHE_AB:
      s = "cache";
      fmt = (mips_opts.micromips ? "k,~(b)"
	     : ISA_IS_R6 (mips_opts.isa) ? "k,+j(b)"
	     : "k,o(b)");
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 9
		 : 16);
      goto ld_st;
    case M_CACHEE_AB:
      s = "cachee";
      fmt = "k,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_PREF_AB:
      s = "pref";
      fmt = (mips_opts.micromips ? "k,~(b)"
	     : ISA_IS_R6 (mips_opts.isa) ? "k,+j(b)"
	     : "k,o(b)");
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 9
		 : 16);
      goto ld_st;
    case M_PREFE_AB:
      s = "prefe";
      fmt = "k,+j(b)";
      offbits = 9;
      goto ld_st;
    case M_SDC1_AB:
      s = "sdc1";
      fmt = "T,o(b)";
      coproc = 1;
      /* Itbl support may require additional care here.  */
      goto ld_st;
    case M_SDC2_AB:
      s = "sdc2";
      fmt = COP12_FMT;
      offbits = (mips_opts.micromips ? 12
		 : ISA_IS_R6 (mips_opts.isa) ? 11
		 : 16);
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_SQC2_AB:
      s = "sqc2";
      fmt = "+7,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_SDC3_AB:
      gas_assert (!mips_opts.micromips);
      s = "sdc3";
      fmt = "E,o(b)";
      /* Itbl support may require additional care here.  */
      coproc = 1;
      goto ld_st;
    case M_SDL_AB:
      s = "sdl";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld_st;
    case M_SDR_AB:
      s = "sdr";
      fmt = MEM12_FMT;
      offbits = (mips_opts.micromips ? 12 : 16);
      goto ld_st;
    case M_SWP_AB:
      gas_assert (mips_opts.micromips);
      s = "swp";
      fmt = "t,~(b)";
      offbits = 12;
      goto ld_st;
    case M_SDP_AB:
      gas_assert (mips_opts.micromips);
      s = "sdp";
      fmt = "t,~(b)";
      offbits = 12;
      goto ld_st;
    case M_SWM_AB:
      gas_assert (mips_opts.micromips);
      s = "swm";
      fmt = "n,~(b)";
      offbits = 12;
      goto ld_st;
    case M_SDM_AB:
      gas_assert (mips_opts.micromips);
      s = "sdm";
      fmt = "n,~(b)";
      offbits = 12;

    ld_st:
      tempreg = AT;
    ld_noat:
      breg = ll_sc_paired ? op[3] : op[2];
      if (small_offset_p (0, align, 16))
	{
	  /* The first case exists for M_LD_AB and M_SD_AB, which are
	     macros for o32 but which should act like normal instructions
	     otherwise.  */
	  if (offbits == 16)
	    macro_build (&offset_expr, s, fmt, op[0], -1, offset_reloc[0],
			 offset_reloc[1], offset_reloc[2], breg);
	  else if (small_offset_p (0, align, offbits))
	    {
	      if (offbits == 0)
		{
		  if (ll_sc_paired)
		   macro_build (NULL, s, fmt, op[0], op[1], breg);
		  else
		   macro_build (NULL, s, fmt, op[0], breg);
		}
	      else
		macro_build (NULL, s, fmt, op[0],
			     (int) offset_expr.X_add_number, breg);
	    }
	  else
	    {
	      if (tempreg == AT)
		used_at = 1;
	      macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j",
			   tempreg, breg, -1, offset_reloc[0],
			   offset_reloc[1], offset_reloc[2]);
	      if (offbits == 0)
		{
		  if (ll_sc_paired)
		    macro_build (NULL, s, fmt, op[0], op[1], tempreg);
		  else
		    macro_build (NULL, s, fmt, op[0], tempreg);
		}
	      else
		macro_build (NULL, s, fmt, op[0], 0, tempreg);
	    }
	  break;
	}

      if (tempreg == AT)
	used_at = 1;

      if (offset_expr.X_op != O_constant
	  && offset_expr.X_op != O_symbol)
	{
	  as_bad (_("expression too complex"));
	  offset_expr.X_op = O_constant;
	}

      if (HAVE_32BIT_ADDRESSES
	  && !IS_SEXT_32BIT_NUM (offset_expr.X_add_number))
	{
	  as_bad (_("number (0x%" PRIx64 ") larger than 32 bits"),
		  offset_expr.X_add_number);
	}

      /* A constant expression in PIC code can be handled just as it
	 is in non PIC code.  */
      if (offset_expr.X_op == O_constant)
	{
	  expr1.X_add_number = offset_high_part (offset_expr.X_add_number,
						 offbits == 0 ? 16 : offbits);
	  offset_expr.X_add_number -= expr1.X_add_number;

	  load_register (tempreg, &expr1, HAVE_64BIT_ADDRESSES);
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			 tempreg, tempreg, breg);
	  if (offbits == 0)
	    {
	      if (offset_expr.X_add_number != 0)
		macro_build (&offset_expr, ADDRESS_ADDI_INSN,
			     "t,r,j", tempreg, tempreg, BFD_RELOC_LO16);
	      if (ll_sc_paired)
		macro_build (NULL, s, fmt, op[0], op[1], tempreg);
	      else
		macro_build (NULL, s, fmt, op[0], tempreg);
	    }
	  else if (offbits == 16)
	    macro_build (&offset_expr, s, fmt, op[0], BFD_RELOC_LO16, tempreg);
	  else
	    macro_build (NULL, s, fmt, op[0],
			 (int) offset_expr.X_add_number, tempreg);
	}
      else if (offbits != 16)
	{
	  /* The offset field is too narrow to be used for a low-part
	     relocation, so load the whole address into the auxiliary
	     register.  */
	  load_address (tempreg, &offset_expr, &used_at);
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			 tempreg, tempreg, breg);
	  if (offbits == 0)
	    {
	      if (ll_sc_paired)
		macro_build (NULL, s, fmt, op[0], op[1], tempreg);
	      else
		macro_build (NULL, s, fmt, op[0], tempreg);
	    }
	  else
	    macro_build (NULL, s, fmt, op[0], 0, tempreg);
	}
      else if (mips_pic == NO_PIC)
	{
	  /* If this is a reference to a GP relative symbol, and there
	     is no base register, we want
	       <op>	op[0],<sym>($gp)	(BFD_RELOC_GPREL16)
	     Otherwise, if there is no base register, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_HI16_S)
	       <op>	op[0],<sym>($tempreg)	(BFD_RELOC_LO16)
	     If we have a constant, we need two instructions anyhow,
	     so we always use the latter form.

	     If we have a base register, and this is a reference to a
	     GP relative symbol, we want
	       addu	$tempreg,$breg,$gp
	       <op>	op[0],<sym>($tempreg)	(BFD_RELOC_GPREL16)
	     Otherwise we want
	       lui	$tempreg,<sym>		(BFD_RELOC_HI16_S)
	       addu	$tempreg,$tempreg,$breg
	       <op>	op[0],<sym>($tempreg)	(BFD_RELOC_LO16)
	     With a constant we always use the latter case.

	     With 64bit address space and no base register and $at usable,
	     we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHEST)
	       lui	$at,<sym>		(BFD_RELOC_HI16_S)
	       daddiu	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHER)
	       dsll32	$tempreg,0
	       daddu	$tempreg,$at
	       <op>	op[0],<sym>($tempreg)	(BFD_RELOC_LO16)
	     If we have a base register, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHEST)
	       lui	$at,<sym>		(BFD_RELOC_HI16_S)
	       daddiu	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHER)
	       daddu	$at,$breg
	       dsll32	$tempreg,0
	       daddu	$tempreg,$at
	       <op>	op[0],<sym>($tempreg)	(BFD_RELOC_LO16)

	     Without $at we can't generate the optimal path for superscalar
	     processors here since this would require two temporary registers.
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHEST)
	       daddiu	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHER)
	       dsll	$tempreg,16
	       daddiu	$tempreg,<sym>		(BFD_RELOC_HI16_S)
	       dsll	$tempreg,16
	       <op>	op[0],<sym>($tempreg)	(BFD_RELOC_LO16)
	     If we have a base register, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHEST)
	       daddiu	$tempreg,<sym>		(BFD_RELOC_MIPS_HIGHER)
	       dsll	$tempreg,16
	       daddiu	$tempreg,<sym>		(BFD_RELOC_HI16_S)
	       dsll	$tempreg,16
	       daddu	$tempreg,$tempreg,$breg
	       <op>	op[0],<sym>($tempreg)	(BFD_RELOC_LO16)

	     For GP relative symbols in 64bit address space we can use
	     the same sequence as in 32bit address space.  */
	  if (HAVE_64BIT_SYMBOLS)
	    {
	      if ((valueT) offset_expr.X_add_number <= MAX_GPREL_OFFSET
		  && !nopic_need_relax (offset_expr.X_add_symbol, 1))
		{
		  relax_start (offset_expr.X_add_symbol);
		  if (breg == 0)
		    {
		      macro_build (&offset_expr, s, fmt, op[0],
				   BFD_RELOC_GPREL16, mips_gp_register);
		    }
		  else
		    {
		      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
				   tempreg, breg, mips_gp_register);
		      macro_build (&offset_expr, s, fmt, op[0],
				   BFD_RELOC_GPREL16, tempreg);
		    }
		  relax_switch ();
		}

	      if (used_at == 0 && mips_opts.at)
		{
		  macro_build (&offset_expr, "lui", LUI_FMT, tempreg,
			       BFD_RELOC_MIPS_HIGHEST);
		  macro_build (&offset_expr, "lui", LUI_FMT, AT,
			       BFD_RELOC_HI16_S);
		  macro_build (&offset_expr, "daddiu", "t,r,j", tempreg,
			       tempreg, BFD_RELOC_MIPS_HIGHER);
		  if (breg != 0)
		    macro_build (NULL, "daddu", "d,v,t", AT, AT, breg);
		  macro_build (NULL, "dsll32", SHFT_FMT, tempreg, tempreg, 0);
		  macro_build (NULL, "daddu", "d,v,t", tempreg, tempreg, AT);
		  macro_build (&offset_expr, s, fmt, op[0], BFD_RELOC_LO16,
			       tempreg);
		  used_at = 1;
		}
	      else
		{
		  macro_build (&offset_expr, "lui", LUI_FMT, tempreg,
			       BFD_RELOC_MIPS_HIGHEST);
		  macro_build (&offset_expr, "daddiu", "t,r,j", tempreg,
			       tempreg, BFD_RELOC_MIPS_HIGHER);
		  macro_build (NULL, "dsll", SHFT_FMT, tempreg, tempreg, 16);
		  macro_build (&offset_expr, "daddiu", "t,r,j", tempreg,
			       tempreg, BFD_RELOC_HI16_S);
		  macro_build (NULL, "dsll", SHFT_FMT, tempreg, tempreg, 16);
		  if (breg != 0)
		    macro_build (NULL, "daddu", "d,v,t",
				 tempreg, tempreg, breg);
		  macro_build (&offset_expr, s, fmt, op[0],
			       BFD_RELOC_LO16, tempreg);
		}

	      if (mips_relax.sequence)
		relax_end ();
	      break;
	    }

	  if (breg == 0)
	    {
	      if ((valueT) offset_expr.X_add_number <= MAX_GPREL_OFFSET
		  && !nopic_need_relax (offset_expr.X_add_symbol, 1))
		{
		  relax_start (offset_expr.X_add_symbol);
		  macro_build (&offset_expr, s, fmt, op[0], BFD_RELOC_GPREL16,
			       mips_gp_register);
		  relax_switch ();
		}
	      macro_build_lui (&offset_expr, tempreg);
	      macro_build (&offset_expr, s, fmt, op[0],
			   BFD_RELOC_LO16, tempreg);
	      if (mips_relax.sequence)
		relax_end ();
	    }
	  else
	    {
	      if ((valueT) offset_expr.X_add_number <= MAX_GPREL_OFFSET
		  && !nopic_need_relax (offset_expr.X_add_symbol, 1))
		{
		  relax_start (offset_expr.X_add_symbol);
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			       tempreg, breg, mips_gp_register);
		  macro_build (&offset_expr, s, fmt, op[0],
			       BFD_RELOC_GPREL16, tempreg);
		  relax_switch ();
		}
	      macro_build_lui (&offset_expr, tempreg);
	      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			   tempreg, tempreg, breg);
	      macro_build (&offset_expr, s, fmt, op[0],
			   BFD_RELOC_LO16, tempreg);
	      if (mips_relax.sequence)
		relax_end ();
	    }
	}
      else if (!mips_big_got)
	{
	  int lw_reloc_type = (int) BFD_RELOC_MIPS_GOT16;

	  /* If this is a reference to an external symbol, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       nop
	       <op>	op[0],0($tempreg)
	     Otherwise we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       nop
	       addiu	$tempreg,$tempreg,<sym>	(BFD_RELOC_LO16)
	       <op>	op[0],0($tempreg)

	     For NewABI, we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT_PAGE)
	       <op>	op[0],<sym>($tempreg)   (BFD_RELOC_MIPS_GOT_OFST)

	     If there is a base register, we add it to $tempreg before
	     the <op>.  If there is a constant, we stick it in the
	     <op> instruction.  We don't handle constants larger than
	     16 bits, because we have no way to load the upper 16 bits
	     (actually, we could handle them for the subset of cases
	     in which we are not using $at).  */
	  gas_assert (offset_expr.X_op == O_symbol);
	  if (HAVE_NEWABI)
	    {
	      macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
			   BFD_RELOC_MIPS_GOT_PAGE, mips_gp_register);
	      if (breg != 0)
		macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			     tempreg, tempreg, breg);
	      macro_build (&offset_expr, s, fmt, op[0],
			   BFD_RELOC_MIPS_GOT_OFST, tempreg);
	      break;
	    }
	  expr1.X_add_number = offset_expr.X_add_number;
	  offset_expr.X_add_number = 0;
	  if (expr1.X_add_number < -0x8000
	      || expr1.X_add_number >= 0x8000)
	    as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
		       lw_reloc_type, mips_gp_register);
	  load_delay_nop ();
	  relax_start (offset_expr.X_add_symbol);
	  relax_switch ();
	  macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j", tempreg,
		       tempreg, BFD_RELOC_LO16);
	  relax_end ();
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			 tempreg, tempreg, breg);
	  macro_build (&expr1, s, fmt, op[0], BFD_RELOC_LO16, tempreg);
	}
      else if (mips_big_got && !HAVE_NEWABI)
	{
	  int gpdelay;

	  /* If this is a reference to an external symbol, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       addu	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_GOT_LO16)
	       <op>	op[0],0($tempreg)
	     Otherwise we want
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT16)
	       nop
	       addiu	$tempreg,$tempreg,<sym>	(BFD_RELOC_LO16)
	       <op>	op[0],0($tempreg)
	     If there is a base register, we add it to $tempreg before
	     the <op>.  If there is a constant, we stick it in the
	     <op> instruction.  We don't handle constants larger than
	     16 bits, because we have no way to load the upper 16 bits
	     (actually, we could handle them for the subset of cases
	     in which we are not using $at).  */
	  gas_assert (offset_expr.X_op == O_symbol);
	  expr1.X_add_number = offset_expr.X_add_number;
	  offset_expr.X_add_number = 0;
	  if (expr1.X_add_number < -0x8000
	      || expr1.X_add_number >= 0x8000)
	    as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	  gpdelay = reg_needs_delay (mips_gp_register);
	  relax_start (offset_expr.X_add_symbol);
	  macro_build (&offset_expr, "lui", LUI_FMT, tempreg,
		       BFD_RELOC_MIPS_GOT_HI16);
	  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", tempreg, tempreg,
		       mips_gp_register);
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
		       BFD_RELOC_MIPS_GOT_LO16, tempreg);
	  relax_switch ();
	  if (gpdelay)
	    macro_build (NULL, "nop", "");
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
		       BFD_RELOC_MIPS_GOT16, mips_gp_register);
	  load_delay_nop ();
	  macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j", tempreg,
		       tempreg, BFD_RELOC_LO16);
	  relax_end ();

	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			 tempreg, tempreg, breg);
	  macro_build (&expr1, s, fmt, op[0], BFD_RELOC_LO16, tempreg);
	}
      else if (mips_big_got && HAVE_NEWABI)
	{
	  /* If this is a reference to an external symbol, we want
	       lui	$tempreg,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       add	$tempreg,$tempreg,$gp
	       lw	$tempreg,<sym>($tempreg) (BFD_RELOC_MIPS_GOT_LO16)
	       <op>	op[0],<ofst>($tempreg)
	     Otherwise, for local symbols, we want:
	       lw	$tempreg,<sym>($gp)	(BFD_RELOC_MIPS_GOT_PAGE)
	       <op>	op[0],<sym>($tempreg)   (BFD_RELOC_MIPS_GOT_OFST)  */
	  gas_assert (offset_expr.X_op == O_symbol);
	  expr1.X_add_number = offset_expr.X_add_number;
	  offset_expr.X_add_number = 0;
	  if (expr1.X_add_number < -0x8000
	      || expr1.X_add_number >= 0x8000)
	    as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	  relax_start (offset_expr.X_add_symbol);
	  macro_build (&offset_expr, "lui", LUI_FMT, tempreg,
		       BFD_RELOC_MIPS_GOT_HI16);
	  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", tempreg, tempreg,
		       mips_gp_register);
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
		       BFD_RELOC_MIPS_GOT_LO16, tempreg);
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			 tempreg, tempreg, breg);
	  macro_build (&expr1, s, fmt, op[0], BFD_RELOC_LO16, tempreg);

	  relax_switch ();
	  offset_expr.X_add_number = expr1.X_add_number;
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", tempreg,
		       BFD_RELOC_MIPS_GOT_PAGE, mips_gp_register);
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			 tempreg, tempreg, breg);
	  macro_build (&offset_expr, s, fmt, op[0],
		       BFD_RELOC_MIPS_GOT_OFST, tempreg);
	  relax_end ();
	}
      else
	abort ();

      break;

    case M_JRADDIUSP:
      gas_assert (mips_opts.micromips);
      gas_assert (mips_opts.insn32);
      start_noreorder ();
      macro_build (NULL, "jr", "s", RA);
      expr1.X_add_number = op[0] << 2;
      macro_build (&expr1, "addiu", "t,r,j", SP, SP, BFD_RELOC_LO16);
      end_noreorder ();
      break;

    case M_JRC:
      gas_assert (mips_opts.micromips);
      gas_assert (mips_opts.insn32);
      macro_build (NULL, "jr", "s", op[0]);
      if (mips_opts.noreorder)
	macro_build (NULL, "nop", "");
      break;

    case M_LI:
    case M_LI_S:
      load_register (op[0], &imm_expr, 0);
      break;

    case M_DLI:
      load_register (op[0], &imm_expr, 1);
      break;

    case M_LI_SS:
      if (imm_expr.X_op == O_constant)
	{
	  used_at = 1;
	  load_register (AT, &imm_expr, 0);
	  macro_build (NULL, "mtc1", "t,G", AT, op[0]);
	  break;
	}
      else
	{
	  gas_assert (imm_expr.X_op == O_absent
		      && offset_expr.X_op == O_symbol
		      && strcmp (segment_name (S_GET_SEGMENT
					       (offset_expr.X_add_symbol)),
				 ".lit4") == 0
		      && offset_expr.X_add_number == 0);
	  macro_build (&offset_expr, "lwc1", "T,o(b)", op[0],
		       BFD_RELOC_MIPS_LITERAL, mips_gp_register);
	  break;
	}

    case M_LI_D:
      /* Check if we have a constant in IMM_EXPR.  If the GPRs are 64 bits
         wide, IMM_EXPR is the entire value.  Otherwise IMM_EXPR is the high
         order 32 bits of the value and the low order 32 bits are either
         zero or in OFFSET_EXPR.  */
      if (imm_expr.X_op == O_constant)
	{
	  if (GPR_SIZE == 64)
	    load_register (op[0], &imm_expr, 1);
	  else
	    {
	      int hreg, lreg;

	      if (target_big_endian)
		{
		  hreg = op[0];
		  lreg = op[0] + 1;
		}
	      else
		{
		  hreg = op[0] + 1;
		  lreg = op[0];
		}

	      if (hreg <= 31)
		load_register (hreg, &imm_expr, 0);
	      if (lreg <= 31)
		{
		  if (offset_expr.X_op == O_absent)
		    move_register (lreg, 0);
		  else
		    {
		      gas_assert (offset_expr.X_op == O_constant);
		      load_register (lreg, &offset_expr, 0);
		    }
		}
	    }
	  break;
	}
      gas_assert (imm_expr.X_op == O_absent);

      /* We know that sym is in the .rdata section.  First we get the
	 upper 16 bits of the address.  */
      if (mips_pic == NO_PIC)
	{
	  macro_build_lui (&offset_expr, AT);
	  used_at = 1;
	}
      else
	{
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", AT,
		       BFD_RELOC_MIPS_GOT16, mips_gp_register);
	  used_at = 1;
	}

      /* Now we load the register(s).  */
      if (GPR_SIZE == 64)
	{
	  used_at = 1;
	  macro_build (&offset_expr, "ld", "t,o(b)", op[0],
		       BFD_RELOC_LO16, AT);
	}
      else
	{
	  used_at = 1;
	  macro_build (&offset_expr, "lw", "t,o(b)", op[0],
		       BFD_RELOC_LO16, AT);
	  if (op[0] != RA)
	    {
	      /* FIXME: How in the world do we deal with the possible
		 overflow here?  */
	      offset_expr.X_add_number += 4;
	      macro_build (&offset_expr, "lw", "t,o(b)",
			   op[0] + 1, BFD_RELOC_LO16, AT);
	    }
	}
      break;

    case M_LI_DD:
      /* Check if we have a constant in IMM_EXPR.  If the FPRs are 64 bits
         wide, IMM_EXPR is the entire value and the GPRs are known to be 64
         bits wide as well.  Otherwise IMM_EXPR is the high order 32 bits of
         the value and the low order 32 bits are either zero or in
         OFFSET_EXPR.  */
      if (imm_expr.X_op == O_constant)
	{
	  tempreg = ZERO;
	  if (((FPR_SIZE == 64 && GPR_SIZE == 64)
	       || !ISA_HAS_MXHC1 (mips_opts.isa))
	      && imm_expr.X_add_number != 0)
	    {
	      used_at = 1;
	      tempreg = AT;
	      load_register (AT, &imm_expr, FPR_SIZE == 64);
	    }
	  if (FPR_SIZE == 64 && GPR_SIZE == 64)
	    macro_build (NULL, "dmtc1", "t,S", tempreg, op[0]);
	  else
	    {
	      if (!ISA_HAS_MXHC1 (mips_opts.isa))
		{
		  if (FPR_SIZE != 32)
		    as_bad (_("Unable to generate `%s' compliant code "
			      "without mthc1"),
			    (FPR_SIZE == 64) ? "fp64" : "fpxx");
		  else
		    macro_build (NULL, "mtc1", "t,G", tempreg, op[0] + 1);
		}
	      if (offset_expr.X_op == O_absent)
		macro_build (NULL, "mtc1", "t,G", 0, op[0]);
	      else
		{
		  gas_assert (offset_expr.X_op == O_constant);
		  load_register (AT, &offset_expr, 0);
		  macro_build (NULL, "mtc1", "t,G", AT, op[0]);
		}
	      if (ISA_HAS_MXHC1 (mips_opts.isa))
		{
		  if (imm_expr.X_add_number != 0)
		    {
		      used_at = 1;
		      tempreg = AT;
		      load_register (AT, &imm_expr, 0);
		    }
		  macro_build (NULL, "mthc1", "t,G", tempreg, op[0]);
		}
	    }
	  break;
	}

      gas_assert (imm_expr.X_op == O_absent
		  && offset_expr.X_op == O_symbol
		  && offset_expr.X_add_number == 0);
      s = segment_name (S_GET_SEGMENT (offset_expr.X_add_symbol));
      if (strcmp (s, ".lit8") == 0)
	{
	  op[2] = mips_gp_register;
	  offset_reloc[0] = BFD_RELOC_MIPS_LITERAL;
	  offset_reloc[1] = BFD_RELOC_UNUSED;
	  offset_reloc[2] = BFD_RELOC_UNUSED;
	}
      else
	{
	  gas_assert (strcmp (s, RDATA_SECTION_NAME) == 0);
	  used_at = 1;
	  if (mips_pic != NO_PIC)
	    macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", AT,
			 BFD_RELOC_MIPS_GOT16, mips_gp_register);
	  else
	    {
	      /* FIXME: This won't work for a 64 bit address.  */
	      macro_build_lui (&offset_expr, AT);
	    }

	  op[2] = AT;
	  offset_reloc[0] = BFD_RELOC_LO16;
	  offset_reloc[1] = BFD_RELOC_UNUSED;
	  offset_reloc[2] = BFD_RELOC_UNUSED;
	}
      align = 8;
      /* Fall through.  */

    case M_L_DAB:
      /* The MIPS assembler seems to check for X_add_number not
         being double aligned and generating:
        	lui	at,%hi(foo+1)
        	addu	at,at,v1
        	addiu	at,at,%lo(foo+1)
        	lwc1	f2,0(at)
        	lwc1	f3,4(at)
         But, the resulting address is the same after relocation so why
         generate the extra instruction?  */
      /* Itbl support may require additional care here.  */
      coproc = 1;
      fmt = "T,o(b)";
      if (CPU_HAS_LDC1_SDC1 (mips_opts.arch))
	{
	  s = "ldc1";
	  goto ld_st;
	}
      s = "lwc1";
      goto ldd_std;

    case M_S_DAB:
      gas_assert (!mips_opts.micromips);
      /* Itbl support may require additional care here.  */
      coproc = 1;
      fmt = "T,o(b)";
      if (CPU_HAS_LDC1_SDC1 (mips_opts.arch))
	{
	  s = "sdc1";
	  goto ld_st;
	}
      s = "swc1";
      goto ldd_std;

    case M_LQ_AB:
      fmt = "t,o(b)";
      s = "lq";
      goto ld;

    case M_SQ_AB:
      fmt = "t,o(b)";
      s = "sq";
      goto ld_st;

    case M_LD_AB:
      fmt = "t,o(b)";
      if (GPR_SIZE == 64)
	{
	  s = "ld";
	  goto ld;
	}
      s = "lw";
      goto ldd_std;

    case M_SD_AB:
      fmt = "t,o(b)";
      if (GPR_SIZE == 64)
	{
	  s = "sd";
	  goto ld_st;
	}
      s = "sw";

    ldd_std:
      /* Even on a big endian machine $fn comes before $fn+1.  We have
	 to adjust when loading from memory.  We set coproc if we must
	 load $fn+1 first.  */
      /* Itbl support may require additional care here.  */
      if (!target_big_endian)
	coproc = 0;

      breg = op[2];
      if (small_offset_p (0, align, 16))
	{
	  ep = &offset_expr;
	  if (!small_offset_p (4, align, 16))
	    {
	      macro_build (&offset_expr, ADDRESS_ADDI_INSN, "t,r,j", AT, breg,
			   -1, offset_reloc[0], offset_reloc[1],
			   offset_reloc[2]);
	      expr1.X_add_number = 0;
	      ep = &expr1;
	      breg = AT;
	      used_at = 1;
	      offset_reloc[0] = BFD_RELOC_LO16;
	      offset_reloc[1] = BFD_RELOC_UNUSED;
	      offset_reloc[2] = BFD_RELOC_UNUSED;
	    }
	  if (strcmp (s, "lw") == 0 && op[0] == breg)
	    {
	      ep->X_add_number += 4;
	      macro_build (ep, s, fmt, op[0] + 1, -1, offset_reloc[0],
			   offset_reloc[1], offset_reloc[2], breg);
	      ep->X_add_number -= 4;
	      macro_build (ep, s, fmt, op[0], -1, offset_reloc[0],
			   offset_reloc[1], offset_reloc[2], breg);
	    }
	  else
	    {
	      macro_build (ep, s, fmt, coproc ? op[0] + 1 : op[0], -1,
			   offset_reloc[0], offset_reloc[1], offset_reloc[2],
			   breg);
	      ep->X_add_number += 4;
	      macro_build (ep, s, fmt, coproc ? op[0] : op[0] + 1, -1,
			   offset_reloc[0], offset_reloc[1], offset_reloc[2],
			   breg);
	    }
	  break;
	}

      if (offset_expr.X_op != O_symbol
	  && offset_expr.X_op != O_constant)
	{
	  as_bad (_("expression too complex"));
	  offset_expr.X_op = O_constant;
	}

      if (HAVE_32BIT_ADDRESSES
	  && !IS_SEXT_32BIT_NUM (offset_expr.X_add_number))
	{
	  as_bad (_("number (0x%" PRIx64 ") larger than 32 bits"),
		  offset_expr.X_add_number);
	}

      if (mips_pic == NO_PIC || offset_expr.X_op == O_constant)
	{
	  /* If this is a reference to a GP relative symbol, we want
	       <op>	op[0],<sym>($gp)	(BFD_RELOC_GPREL16)
	       <op>	op[0]+1,<sym>+4($gp)	(BFD_RELOC_GPREL16)
	     If we have a base register, we use this
	       addu	$at,$breg,$gp
	       <op>	op[0],<sym>($at)	(BFD_RELOC_GPREL16)
	       <op>	op[0]+1,<sym>+4($at)	(BFD_RELOC_GPREL16)
	     If this is not a GP relative symbol, we want
	       lui	$at,<sym>		(BFD_RELOC_HI16_S)
	       <op>	op[0],<sym>($at)	(BFD_RELOC_LO16)
	       <op>	op[0]+1,<sym>+4($at)	(BFD_RELOC_LO16)
	     If there is a base register, we add it to $at after the
	     lui instruction.  If there is a constant, we always use
	     the last case.  */
	  if (offset_expr.X_op == O_symbol
	      && (valueT) offset_expr.X_add_number <= MAX_GPREL_OFFSET
	      && !nopic_need_relax (offset_expr.X_add_symbol, 1))
	    {
	      relax_start (offset_expr.X_add_symbol);
	      if (breg == 0)
		{
		  tempreg = mips_gp_register;
		}
	      else
		{
		  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			       AT, breg, mips_gp_register);
		  tempreg = AT;
		  used_at = 1;
		}

	      /* Itbl support may require additional care here.  */
	      macro_build (&offset_expr, s, fmt, coproc ? op[0] + 1 : op[0],
			   BFD_RELOC_GPREL16, tempreg);
	      offset_expr.X_add_number += 4;

	      /* Set mips_optimize to 2 to avoid inserting an
                 undesired nop.  */
	      hold_mips_optimize = mips_optimize;
	      mips_optimize = 2;
	      /* Itbl support may require additional care here.  */
	      macro_build (&offset_expr, s, fmt, coproc ? op[0] : op[0] + 1,
			   BFD_RELOC_GPREL16, tempreg);
	      mips_optimize = hold_mips_optimize;

	      relax_switch ();

	      offset_expr.X_add_number -= 4;
	    }
	  used_at = 1;
	  if (offset_high_part (offset_expr.X_add_number, 16)
	      != offset_high_part (offset_expr.X_add_number + 4, 16))
	    {
	      load_address (AT, &offset_expr, &used_at);
	      offset_expr.X_op = O_constant;
	      offset_expr.X_add_number = 0;
	    }
	  else
	    macro_build_lui (&offset_expr, AT);
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", AT, breg, AT);
	  /* Itbl support may require additional care here.  */
	  macro_build (&offset_expr, s, fmt, coproc ? op[0] + 1 : op[0],
		       BFD_RELOC_LO16, AT);
	  /* FIXME: How do we handle overflow here?  */
	  offset_expr.X_add_number += 4;
	  /* Itbl support may require additional care here.  */
	  macro_build (&offset_expr, s, fmt, coproc ? op[0] : op[0] + 1,
		       BFD_RELOC_LO16, AT);
	  if (mips_relax.sequence)
	    relax_end ();
	}
      else if (!mips_big_got)
	{
	  /* If this is a reference to an external symbol, we want
	       lw	$at,<sym>($gp)		(BFD_RELOC_MIPS_GOT16)
	       nop
	       <op>	op[0],0($at)
	       <op>	op[0]+1,4($at)
	     Otherwise we want
	       lw	$at,<sym>($gp)		(BFD_RELOC_MIPS_GOT16)
	       nop
	       <op>	op[0],<sym>($at)	(BFD_RELOC_LO16)
	       <op>	op[0]+1,<sym>+4($at)	(BFD_RELOC_LO16)
	     If there is a base register we add it to $at before the
	     lwc1 instructions.  If there is a constant we include it
	     in the lwc1 instructions.  */
	  used_at = 1;
	  expr1.X_add_number = offset_expr.X_add_number;
	  if (expr1.X_add_number < -0x8000
	      || expr1.X_add_number >= 0x8000 - 4)
	    as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	  load_got_offset (AT, &offset_expr);
	  load_delay_nop ();
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", AT, breg, AT);

	  /* Set mips_optimize to 2 to avoid inserting an undesired
             nop.  */
	  hold_mips_optimize = mips_optimize;
	  mips_optimize = 2;

	  /* Itbl support may require additional care here.  */
	  relax_start (offset_expr.X_add_symbol);
	  macro_build (&expr1, s, fmt, coproc ? op[0] + 1 : op[0],
		       BFD_RELOC_LO16, AT);
	  expr1.X_add_number += 4;
	  macro_build (&expr1, s, fmt, coproc ? op[0] : op[0] + 1,
		       BFD_RELOC_LO16, AT);
	  relax_switch ();
	  macro_build (&offset_expr, s, fmt, coproc ? op[0] + 1 : op[0],
		       BFD_RELOC_LO16, AT);
	  offset_expr.X_add_number += 4;
	  macro_build (&offset_expr, s, fmt, coproc ? op[0] : op[0] + 1,
		       BFD_RELOC_LO16, AT);
	  relax_end ();

	  mips_optimize = hold_mips_optimize;
	}
      else if (mips_big_got)
	{
	  int gpdelay;

	  /* If this is a reference to an external symbol, we want
	       lui	$at,<sym>		(BFD_RELOC_MIPS_GOT_HI16)
	       addu	$at,$at,$gp
	       lw	$at,<sym>($at)		(BFD_RELOC_MIPS_GOT_LO16)
	       nop
	       <op>	op[0],0($at)
	       <op>	op[0]+1,4($at)
	     Otherwise we want
	       lw	$at,<sym>($gp)		(BFD_RELOC_MIPS_GOT16)
	       nop
	       <op>	op[0],<sym>($at)	(BFD_RELOC_LO16)
	       <op>	op[0]+1,<sym>+4($at)	(BFD_RELOC_LO16)
	     If there is a base register we add it to $at before the
	     lwc1 instructions.  If there is a constant we include it
	     in the lwc1 instructions.  */
	  used_at = 1;
	  expr1.X_add_number = offset_expr.X_add_number;
	  offset_expr.X_add_number = 0;
	  if (expr1.X_add_number < -0x8000
	      || expr1.X_add_number >= 0x8000 - 4)
	    as_bad (_("PIC code offset overflow (max 16 signed bits)"));
	  gpdelay = reg_needs_delay (mips_gp_register);
	  relax_start (offset_expr.X_add_symbol);
	  macro_build (&offset_expr, "lui", LUI_FMT,
		       AT, BFD_RELOC_MIPS_GOT_HI16);
	  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
		       AT, AT, mips_gp_register);
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)",
		       AT, BFD_RELOC_MIPS_GOT_LO16, AT);
	  load_delay_nop ();
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", AT, breg, AT);
	  /* Itbl support may require additional care here.  */
	  macro_build (&expr1, s, fmt, coproc ? op[0] + 1 : op[0],
		       BFD_RELOC_LO16, AT);
	  expr1.X_add_number += 4;

	  /* Set mips_optimize to 2 to avoid inserting an undesired
             nop.  */
	  hold_mips_optimize = mips_optimize;
	  mips_optimize = 2;
	  /* Itbl support may require additional care here.  */
	  macro_build (&expr1, s, fmt, coproc ? op[0] : op[0] + 1,
		       BFD_RELOC_LO16, AT);
	  mips_optimize = hold_mips_optimize;
	  expr1.X_add_number -= 4;

	  relax_switch ();
	  offset_expr.X_add_number = expr1.X_add_number;
	  if (gpdelay)
	    macro_build (NULL, "nop", "");
	  macro_build (&offset_expr, ADDRESS_LOAD_INSN, "t,o(b)", AT,
		       BFD_RELOC_MIPS_GOT16, mips_gp_register);
	  load_delay_nop ();
	  if (breg != 0)
	    macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", AT, breg, AT);
	  /* Itbl support may require additional care here.  */
	  macro_build (&offset_expr, s, fmt, coproc ? op[0] + 1 : op[0],
		       BFD_RELOC_LO16, AT);
	  offset_expr.X_add_number += 4;

	  /* Set mips_optimize to 2 to avoid inserting an undesired
             nop.  */
	  hold_mips_optimize = mips_optimize;
	  mips_optimize = 2;
	  /* Itbl support may require additional care here.  */
	  macro_build (&offset_expr, s, fmt, coproc ? op[0] : op[0] + 1,
		       BFD_RELOC_LO16, AT);
	  mips_optimize = hold_mips_optimize;
	  relax_end ();
	}
      else
	abort ();

      break;

    case M_SAA_AB:
      s = "saa";
      goto saa_saad;
    case M_SAAD_AB:
      s = "saad";
    saa_saad:
      gas_assert (!mips_opts.micromips);
      offbits = 0;
      fmt = "t,(b)";
      goto ld_st;

   /* New code added to support COPZ instructions.
      This code builds table entries out of the macros in mip_opcodes.
      R4000 uses interlocks to handle coproc delays.
      Other chips (like the R3000) require nops to be inserted for delays.

      FIXME: Currently, we require that the user handle delays.
      In order to fill delay slots for non-interlocked chips,
      we must have a way to specify delays based on the coprocessor.
      Eg. 4 cycles if load coproc reg from memory, 1 if in cache, etc.
      What are the side-effects of the cop instruction?
      What cache support might we have and what are its effects?
      Both coprocessor & memory require delays. how long???
      What registers are read/set/modified?

      If an itbl is provided to interpret cop instructions,
      this knowledge can be encoded in the itbl spec.  */

    case M_COP0:
      s = "c0";
      goto copz;
    case M_COP1:
      s = "c1";
      goto copz;
    case M_COP2:
      s = "c2";
      goto copz;
    case M_COP3:
      s = "c3";
    copz:
      gas_assert (!mips_opts.micromips);
      /* For now we just do C (same as Cz).  The parameter will be
         stored in insn_opcode by mips_ip.  */
      macro_build (NULL, s, "C", (int) ip->insn_opcode);
      break;

    case M_MOVE:
      move_register (op[0], op[1]);
      break;

    case M_MOVEP:
      gas_assert (mips_opts.micromips);
      gas_assert (mips_opts.insn32);
      move_register (micromips_to_32_reg_h_map1[op[0]],
		     micromips_to_32_reg_m_map[op[1]]);
      move_register (micromips_to_32_reg_h_map2[op[0]],
		     micromips_to_32_reg_n_map[op[2]]);
      break;

    case M_DMUL:
      dbl = 1;
      /* Fall through.  */
    case M_MUL:
      if (mips_opts.arch == CPU_R5900)
	macro_build (NULL, dbl ? "dmultu" : "multu", "d,s,t", op[0], op[1],
		     op[2]);
      else
        {
	  macro_build (NULL, dbl ? "dmultu" : "multu", "s,t", op[1], op[2]);
	  macro_build (NULL, "mflo", MFHL_FMT, op[0]);
        }
      break;

    case M_DMUL_I:
      dbl = 1;
      /* Fall through.  */
    case M_MUL_I:
      /* The MIPS assembler some times generates shifts and adds.  I'm
	 not trying to be that fancy. GCC should do this for us
	 anyway.  */
      used_at = 1;
      load_register (AT, &imm_expr, dbl);
      macro_build (NULL, dbl ? "dmult" : "mult", "s,t", op[1], AT);
      macro_build (NULL, "mflo", MFHL_FMT, op[0]);
      break;

    case M_DMULO_I:
      dbl = 1;
      /* Fall through.  */
    case M_MULO_I:
      imm = 1;
      goto do_mulo;

    case M_DMULO:
      dbl = 1;
      /* Fall through.  */
    case M_MULO:
    do_mulo:
      start_noreorder ();
      used_at = 1;
      if (imm)
	load_register (AT, &imm_expr, dbl);
      macro_build (NULL, dbl ? "dmult" : "mult", "s,t",
		   op[1], imm ? AT : op[2]);
      macro_build (NULL, "mflo", MFHL_FMT, op[0]);
      macro_build (NULL, dbl ? "dsra32" : "sra", SHFT_FMT, op[0], op[0], 31);
      macro_build (NULL, "mfhi", MFHL_FMT, AT);
      if (mips_trap)
	macro_build (NULL, "tne", TRAP_FMT, op[0], AT, 6);
      else
	{
	  if (mips_opts.micromips)
	    micromips_label_expr (&label_expr);
	  else
	    label_expr.X_add_number = 8;
	  macro_build (&label_expr, "beq", "s,t,p", op[0], AT);
	  macro_build (NULL, "nop", "");
	  macro_build (NULL, "break", BRK_FMT, 6);
	  if (mips_opts.micromips)
	    micromips_add_label ();
	}
      end_noreorder ();
      macro_build (NULL, "mflo", MFHL_FMT, op[0]);
      break;

    case M_DMULOU_I:
      dbl = 1;
      /* Fall through.  */
    case M_MULOU_I:
      imm = 1;
      goto do_mulou;

    case M_DMULOU:
      dbl = 1;
      /* Fall through.  */
    case M_MULOU:
    do_mulou:
      start_noreorder ();
      used_at = 1;
      if (imm)
	load_register (AT, &imm_expr, dbl);
      macro_build (NULL, dbl ? "dmultu" : "multu", "s,t",
		   op[1], imm ? AT : op[2]);
      macro_build (NULL, "mfhi", MFHL_FMT, AT);
      macro_build (NULL, "mflo", MFHL_FMT, op[0]);
      if (mips_trap)
	macro_build (NULL, "tne", TRAP_FMT, AT, ZERO, 6);
      else
	{
	  if (mips_opts.micromips)
	    micromips_label_expr (&label_expr);
	  else
	    label_expr.X_add_number = 8;
	  macro_build (&label_expr, "beq", "s,t,p", AT, ZERO);
	  macro_build (NULL, "nop", "");
	  macro_build (NULL, "break", BRK_FMT, 6);
	  if (mips_opts.micromips)
	    micromips_add_label ();
	}
      end_noreorder ();
      break;

    case M_DROL:
      if (ISA_HAS_DROR (mips_opts.isa) || CPU_HAS_DROR (mips_opts.arch))
	{
	  if (op[0] == op[1])
	    {
	      tempreg = AT;
	      used_at = 1;
	    }
	  else
	    tempreg = op[0];
	  macro_build (NULL, "dnegu", "d,w", tempreg, op[2]);
	  macro_build (NULL, "drorv", "d,t,s", op[0], op[1], tempreg);
	  break;
	}
      used_at = 1;
      macro_build (NULL, "dsubu", "d,v,t", AT, ZERO, op[2]);
      macro_build (NULL, "dsrlv", "d,t,s", AT, op[1], AT);
      macro_build (NULL, "dsllv", "d,t,s", op[0], op[1], op[2]);
      macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
      break;

    case M_ROL:
      if (ISA_HAS_ROR (mips_opts.isa) || CPU_HAS_ROR (mips_opts.arch))
	{
	  if (op[0] == op[1])
	    {
	      tempreg = AT;
	      used_at = 1;
	    }
	  else
	    tempreg = op[0];
	  macro_build (NULL, "negu", "d,w", tempreg, op[2]);
	  macro_build (NULL, "rorv", "d,t,s", op[0], op[1], tempreg);
	  break;
	}
      used_at = 1;
      macro_build (NULL, "subu", "d,v,t", AT, ZERO, op[2]);
      macro_build (NULL, "srlv", "d,t,s", AT, op[1], AT);
      macro_build (NULL, "sllv", "d,t,s", op[0], op[1], op[2]);
      macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
      break;

    case M_DROL_I:
      {
	unsigned int rot;
	const char *l;
	const char *rr;

	rot = imm_expr.X_add_number & 0x3f;
	if (ISA_HAS_DROR (mips_opts.isa) || CPU_HAS_DROR (mips_opts.arch))
	  {
	    rot = (64 - rot) & 0x3f;
	    if (rot >= 32)
	      macro_build (NULL, "dror32", SHFT_FMT, op[0], op[1], rot - 32);
	    else
	      macro_build (NULL, "dror", SHFT_FMT, op[0], op[1], rot);
	    break;
	  }
	if (rot == 0)
	  {
	    macro_build (NULL, "dsrl", SHFT_FMT, op[0], op[1], 0);
	    break;
	  }
	l = (rot < 0x20) ? "dsll" : "dsll32";
	rr = ((0x40 - rot) < 0x20) ? "dsrl" : "dsrl32";
	rot &= 0x1f;
	used_at = 1;
	macro_build (NULL, l, SHFT_FMT, AT, op[1], rot);
	macro_build (NULL, rr, SHFT_FMT, op[0], op[1], (0x20 - rot) & 0x1f);
	macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
      }
      break;

    case M_ROL_I:
      {
	unsigned int rot;

	rot = imm_expr.X_add_number & 0x1f;
	if (ISA_HAS_ROR (mips_opts.isa) || CPU_HAS_ROR (mips_opts.arch))
	  {
	    macro_build (NULL, "ror", SHFT_FMT, op[0], op[1],
			 (32 - rot) & 0x1f);
	    break;
	  }
	if (rot == 0)
	  {
	    macro_build (NULL, "srl", SHFT_FMT, op[0], op[1], 0);
	    break;
	  }
	used_at = 1;
	macro_build (NULL, "sll", SHFT_FMT, AT, op[1], rot);
	macro_build (NULL, "srl", SHFT_FMT, op[0], op[1], (0x20 - rot) & 0x1f);
	macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
      }
      break;

    case M_DROR:
      if (ISA_HAS_DROR (mips_opts.isa) || CPU_HAS_DROR (mips_opts.arch))
	{
	  macro_build (NULL, "drorv", "d,t,s", op[0], op[1], op[2]);
	  break;
	}
      used_at = 1;
      macro_build (NULL, "dsubu", "d,v,t", AT, ZERO, op[2]);
      macro_build (NULL, "dsllv", "d,t,s", AT, op[1], AT);
      macro_build (NULL, "dsrlv", "d,t,s", op[0], op[1], op[2]);
      macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
      break;

    case M_ROR:
      if (ISA_HAS_ROR (mips_opts.isa) || CPU_HAS_ROR (mips_opts.arch))
	{
	  macro_build (NULL, "rorv", "d,t,s", op[0], op[1], op[2]);
	  break;
	}
      used_at = 1;
      macro_build (NULL, "subu", "d,v,t", AT, ZERO, op[2]);
      macro_build (NULL, "sllv", "d,t,s", AT, op[1], AT);
      macro_build (NULL, "srlv", "d,t,s", op[0], op[1], op[2]);
      macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
      break;

    case M_DROR_I:
      {
	unsigned int rot;
	const char *l;
	const char *rr;

	rot = imm_expr.X_add_number & 0x3f;
	if (ISA_HAS_DROR (mips_opts.isa) || CPU_HAS_DROR (mips_opts.arch))
	  {
	    if (rot >= 32)
	      macro_build (NULL, "dror32", SHFT_FMT, op[0], op[1], rot - 32);
	    else
	      macro_build (NULL, "dror", SHFT_FMT, op[0], op[1], rot);
	    break;
	  }
	if (rot == 0)
	  {
	    macro_build (NULL, "dsrl", SHFT_FMT, op[0], op[1], 0);
	    break;
	  }
	rr = (rot < 0x20) ? "dsrl" : "dsrl32";
	l = ((0x40 - rot) < 0x20) ? "dsll" : "dsll32";
	rot &= 0x1f;
	used_at = 1;
	macro_build (NULL, rr, SHFT_FMT, AT, op[1], rot);
	macro_build (NULL, l, SHFT_FMT, op[0], op[1], (0x20 - rot) & 0x1f);
	macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
      }
      break;

    case M_ROR_I:
      {
	unsigned int rot;

	rot = imm_expr.X_add_number & 0x1f;
	if (ISA_HAS_ROR (mips_opts.isa) || CPU_HAS_ROR (mips_opts.arch))
	  {
	    macro_build (NULL, "ror", SHFT_FMT, op[0], op[1], rot);
	    break;
	  }
	if (rot == 0)
	  {
	    macro_build (NULL, "srl", SHFT_FMT, op[0], op[1], 0);
	    break;
	  }
	used_at = 1;
	macro_build (NULL, "srl", SHFT_FMT, AT, op[1], rot);
	macro_build (NULL, "sll", SHFT_FMT, op[0], op[1], (0x20 - rot) & 0x1f);
	macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
      }
      break;

    case M_SEQ:
      if (op[1] == 0)
	macro_build (&expr1, "sltiu", "t,r,j", op[0], op[2], BFD_RELOC_LO16);
      else if (op[2] == 0)
	macro_build (&expr1, "sltiu", "t,r,j", op[0], op[1], BFD_RELOC_LO16);
      else
	{
	  macro_build (NULL, "xor", "d,v,t", op[0], op[1], op[2]);
	  macro_build (&expr1, "sltiu", "t,r,j", op[0], op[0], BFD_RELOC_LO16);
	}
      break;

    case M_SEQ_I:
      if (imm_expr.X_add_number == 0)
	{
	  macro_build (&expr1, "sltiu", "t,r,j", op[0], op[1], BFD_RELOC_LO16);
	  break;
	}
      if (op[1] == 0)
	{
	  as_warn (_("instruction %s: result is always false"),
		   ip->insn_mo->name);
	  move_register (op[0], 0);
	  break;
	}
      if (CPU_HAS_SEQ (mips_opts.arch)
	  && -512 <= imm_expr.X_add_number
	  && imm_expr.X_add_number < 512)
	{
	  macro_build (NULL, "seqi", "t,r,+Q", op[0], op[1],
		       (int) imm_expr.X_add_number);
	  break;
	}
      if (imm_expr.X_add_number >= 0
	  && imm_expr.X_add_number < 0x10000)
	macro_build (&imm_expr, "xori", "t,r,i", op[0], op[1], BFD_RELOC_LO16);
      else if (imm_expr.X_add_number > -0x8000
	       && imm_expr.X_add_number < 0)
	{
	  imm_expr.X_add_number = -imm_expr.X_add_number;
	  macro_build (&imm_expr, GPR_SIZE == 32 ? "addiu" : "daddiu",
		       "t,r,j", op[0], op[1], BFD_RELOC_LO16);
	}
      else if (CPU_HAS_SEQ (mips_opts.arch))
	{
	  used_at = 1;
	  load_register (AT, &imm_expr, GPR_SIZE == 64);
	  macro_build (NULL, "seq", "d,v,t", op[0], op[1], AT);
	  break;
	}
      else
	{
	  load_register (AT, &imm_expr, GPR_SIZE == 64);
	  macro_build (NULL, "xor", "d,v,t", op[0], op[1], AT);
	  used_at = 1;
	}
      macro_build (&expr1, "sltiu", "t,r,j", op[0], op[0], BFD_RELOC_LO16);
      break;

    case M_SGE:		/* X >= Y  <==>  not (X < Y) */
      s = "slt";
      goto sge;
    case M_SGEU:
      s = "sltu";
    sge:
      macro_build (NULL, s, "d,v,t", op[0], op[1], op[2]);
      macro_build (&expr1, "xori", "t,r,i", op[0], op[0], BFD_RELOC_LO16);
      break;

    case M_SGE_I:	/* X >= I  <==>  not (X < I).  */
    case M_SGEU_I:
      if (imm_expr.X_add_number >= -0x8000
	  && imm_expr.X_add_number < 0x8000)
	macro_build (&imm_expr, mask == M_SGE_I ? "slti" : "sltiu", "t,r,j",
		     op[0], op[1], BFD_RELOC_LO16);
      else
	{
	  load_register (AT, &imm_expr, GPR_SIZE == 64);
	  macro_build (NULL, mask == M_SGE_I ? "slt" : "sltu", "d,v,t",
		       op[0], op[1], AT);
	  used_at = 1;
	}
      macro_build (&expr1, "xori", "t,r,i", op[0], op[0], BFD_RELOC_LO16);
      break;

    case M_SGT:		/* X > Y  <==>  Y < X.  */
      s = "slt";
      goto sgt;
    case M_SGTU:
      s = "sltu";
    sgt:
      macro_build (NULL, s, "d,v,t", op[0], op[2], op[1]);
      break;

    case M_SGT_I:	/* X > I  <==>  I < X.  */
      s = "slt";
      goto sgti;
    case M_SGTU_I:
      s = "sltu";
    sgti:
      used_at = 1;
      load_register (AT, &imm_expr, GPR_SIZE == 64);
      macro_build (NULL, s, "d,v,t", op[0], AT, op[1]);
      break;

    case M_SLE:		/* X <= Y  <==>  Y >= X  <==>  not (Y < X).  */
      s = "slt";
      goto sle;
    case M_SLEU:
      s = "sltu";
    sle:
      macro_build (NULL, s, "d,v,t", op[0], op[2], op[1]);
      macro_build (&expr1, "xori", "t,r,i", op[0], op[0], BFD_RELOC_LO16);
      break;

    case M_SLE_I:	/* X <= I  <==>  I >= X  <==>  not (I < X) */
      s = "slt";
      goto slei;
    case M_SLEU_I:
      s = "sltu";
    slei:
      used_at = 1;
      load_register (AT, &imm_expr, GPR_SIZE == 64);
      macro_build (NULL, s, "d,v,t", op[0], AT, op[1]);
      macro_build (&expr1, "xori", "t,r,i", op[0], op[0], BFD_RELOC_LO16);
      break;

    case M_SLT_I:
      if (imm_expr.X_add_number >= -0x8000
	  && imm_expr.X_add_number < 0x8000)
	{
	  macro_build (&imm_expr, "slti", "t,r,j", op[0], op[1],
		       BFD_RELOC_LO16);
	  break;
	}
      used_at = 1;
      load_register (AT, &imm_expr, GPR_SIZE == 64);
      macro_build (NULL, "slt", "d,v,t", op[0], op[1], AT);
      break;

    case M_SLTU_I:
      if (imm_expr.X_add_number >= -0x8000
	  && imm_expr.X_add_number < 0x8000)
	{
	  macro_build (&imm_expr, "sltiu", "t,r,j", op[0], op[1],
		       BFD_RELOC_LO16);
	  break;
	}
      used_at = 1;
      load_register (AT, &imm_expr, GPR_SIZE == 64);
      macro_build (NULL, "sltu", "d,v,t", op[0], op[1], AT);
      break;

    case M_SNE:
      if (op[1] == 0)
	macro_build (NULL, "sltu", "d,v,t", op[0], 0, op[2]);
      else if (op[2] == 0)
	macro_build (NULL, "sltu", "d,v,t", op[0], 0, op[1]);
      else
	{
	  macro_build (NULL, "xor", "d,v,t", op[0], op[1], op[2]);
	  macro_build (NULL, "sltu", "d,v,t", op[0], 0, op[0]);
	}
      break;

    case M_SNE_I:
      if (imm_expr.X_add_number == 0)
	{
	  macro_build (NULL, "sltu", "d,v,t", op[0], 0, op[1]);
	  break;
	}
      if (op[1] == 0)
	{
	  as_warn (_("instruction %s: result is always true"),
		   ip->insn_mo->name);
	  macro_build (&expr1, GPR_SIZE == 32 ? "addiu" : "daddiu", "t,r,j",
		       op[0], 0, BFD_RELOC_LO16);
	  break;
	}
      if (CPU_HAS_SEQ (mips_opts.arch)
	  && -512 <= imm_expr.X_add_number
	  && imm_expr.X_add_number < 512)
	{
	  macro_build (NULL, "snei", "t,r,+Q", op[0], op[1],
		       (int) imm_expr.X_add_number);
	  break;
	}
      if (imm_expr.X_add_number >= 0
	  && imm_expr.X_add_number < 0x10000)
	{
	  macro_build (&imm_expr, "xori", "t,r,i", op[0], op[1],
		       BFD_RELOC_LO16);
	}
      else if (imm_expr.X_add_number > -0x8000
	       && imm_expr.X_add_number < 0)
	{
	  imm_expr.X_add_number = -imm_expr.X_add_number;
	  macro_build (&imm_expr, GPR_SIZE == 32 ? "addiu" : "daddiu",
		       "t,r,j", op[0], op[1], BFD_RELOC_LO16);
	}
      else if (CPU_HAS_SEQ (mips_opts.arch))
	{
	  used_at = 1;
	  load_register (AT, &imm_expr, GPR_SIZE == 64);
	  macro_build (NULL, "sne", "d,v,t", op[0], op[1], AT);
	  break;
	}
      else
	{
	  load_register (AT, &imm_expr, GPR_SIZE == 64);
	  macro_build (NULL, "xor", "d,v,t", op[0], op[1], AT);
	  used_at = 1;
	}
      macro_build (NULL, "sltu", "d,v,t", op[0], 0, op[0]);
      break;

    case M_SUB_I:
      s = "addi";
      s2 = "sub";
      if (ISA_IS_R6 (mips_opts.isa))
	goto do_subi_i;
      else
	goto do_subi;
    case M_SUBU_I:
      s = "addiu";
      s2 = "subu";
      goto do_subi;
    case M_DSUB_I:
      dbl = 1;
      s = "daddi";
      s2 = "dsub";
      if (!mips_opts.micromips && !ISA_IS_R6 (mips_opts.isa))
	goto do_subi;
      if (imm_expr.X_add_number > -0x200
	  && imm_expr.X_add_number <= 0x200
	  && !ISA_IS_R6 (mips_opts.isa))
	{
	  macro_build (NULL, s, "t,r,.", op[0], op[1],
		       (int) -imm_expr.X_add_number);
	  break;
	}
      goto do_subi_i;
    case M_DSUBU_I:
      dbl = 1;
      s = "daddiu";
      s2 = "dsubu";
    do_subi:
      if (imm_expr.X_add_number > -0x8000
	  && imm_expr.X_add_number <= 0x8000)
	{
	  imm_expr.X_add_number = -imm_expr.X_add_number;
	  macro_build (&imm_expr, s, "t,r,j", op[0], op[1], BFD_RELOC_LO16);
	  break;
	}
    do_subi_i:
      used_at = 1;
      load_register (AT, &imm_expr, dbl);
      macro_build (NULL, s2, "d,v,t", op[0], op[1], AT);
      break;

    case M_TEQ_I:
      s = "teq";
      goto trap;
    case M_TGE_I:
      s = "tge";
      goto trap;
    case M_TGEU_I:
      s = "tgeu";
      goto trap;
    case M_TLT_I:
      s = "tlt";
      goto trap;
    case M_TLTU_I:
      s = "tltu";
      goto trap;
    case M_TNE_I:
      s = "tne";
    trap:
      used_at = 1;
      load_register (AT, &imm_expr, GPR_SIZE == 64);
      macro_build (NULL, s, "s,t", op[0], AT);
      break;

    case M_TRUNCWS:
    case M_TRUNCWD:
      gas_assert (!mips_opts.micromips);
      gas_assert (mips_opts.isa == ISA_MIPS1);
      used_at = 1;

      /*
       * Is the double cfc1 instruction a bug in the mips assembler;
       * or is there a reason for it?
       */
      start_noreorder ();
      macro_build (NULL, "cfc1", "t,g", op[2], FCSR);
      macro_build (NULL, "cfc1", "t,g", op[2], FCSR);
      macro_build (NULL, "nop", "");
      expr1.X_add_number = 3;
      macro_build (&expr1, "ori", "t,r,i", AT, op[2], BFD_RELOC_LO16);
      expr1.X_add_number = 2;
      macro_build (&expr1, "xori", "t,r,i", AT, AT, BFD_RELOC_LO16);
      macro_build (NULL, "ctc1", "t,g", AT, FCSR);
      macro_build (NULL, "nop", "");
      macro_build (NULL, mask == M_TRUNCWD ? "cvt.w.d" : "cvt.w.s", "D,S",
		   op[0], op[1]);
      macro_build (NULL, "ctc1", "t,g", op[2], FCSR);
      macro_build (NULL, "nop", "");
      end_noreorder ();
      break;

    case M_ULH_AB:
      s = "lb";
      s2 = "lbu";
      off = 1;
      goto uld_st;
    case M_ULHU_AB:
      s = "lbu";
      s2 = "lbu";
      off = 1;
      goto uld_st;
    case M_ULW_AB:
      s = "lwl";
      s2 = "lwr";
      offbits = (mips_opts.micromips ? 12 : 16);
      off = 3;
      goto uld_st;
    case M_ULD_AB:
      s = "ldl";
      s2 = "ldr";
      offbits = (mips_opts.micromips ? 12 : 16);
      off = 7;
      goto uld_st;
    case M_USH_AB:
      s = "sb";
      s2 = "sb";
      off = 1;
      ust = 1;
      goto uld_st;
    case M_USW_AB:
      s = "swl";
      s2 = "swr";
      offbits = (mips_opts.micromips ? 12 : 16);
      off = 3;
      ust = 1;
      goto uld_st;
    case M_USD_AB:
      s = "sdl";
      s2 = "sdr";
      offbits = (mips_opts.micromips ? 12 : 16);
      off = 7;
      ust = 1;

    uld_st:
      breg = op[2];
      large_offset = !small_offset_p (off, align, offbits);
      ep = &offset_expr;
      expr1.X_add_number = 0;
      if (large_offset)
	{
	  used_at = 1;
	  tempreg = AT;
	  if (small_offset_p (0, align, 16))
	    macro_build (ep, ADDRESS_ADDI_INSN, "t,r,j", tempreg, breg, -1,
			 offset_reloc[0], offset_reloc[1], offset_reloc[2]);
	  else
	    {
	      load_address (tempreg, ep, &used_at);
	      if (breg != 0)
		macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t",
			     tempreg, tempreg, breg);
	    }
	  offset_reloc[0] = BFD_RELOC_LO16;
	  offset_reloc[1] = BFD_RELOC_UNUSED;
	  offset_reloc[2] = BFD_RELOC_UNUSED;
	  breg = tempreg;
	  tempreg = op[0];
	  ep = &expr1;
	}
      else if (!ust && op[0] == breg)
	{
	  used_at = 1;
	  tempreg = AT;
	}
      else
	tempreg = op[0];

      if (off == 1)
	goto ulh_sh;

      if (!target_big_endian)
	ep->X_add_number += off;
      if (offbits == 12)
	macro_build (NULL, s, "t,~(b)", tempreg, (int) ep->X_add_number, breg);
      else
	macro_build (ep, s, "t,o(b)", tempreg, -1,
		     offset_reloc[0], offset_reloc[1], offset_reloc[2], breg);

      if (!target_big_endian)
	ep->X_add_number -= off;
      else
	ep->X_add_number += off;
      if (offbits == 12)
	macro_build (NULL, s2, "t,~(b)",
		     tempreg, (int) ep->X_add_number, breg);
      else
	macro_build (ep, s2, "t,o(b)", tempreg, -1,
		     offset_reloc[0], offset_reloc[1], offset_reloc[2], breg);

      /* If necessary, move the result in tempreg to the final destination.  */
      if (!ust && op[0] != tempreg)
        {
	  /* Protect second load's delay slot.  */
	  load_delay_nop ();
	  move_register (op[0], tempreg);
	}
      break;

    ulh_sh:
      used_at = 1;
      if (target_big_endian == ust)
	ep->X_add_number += off;
      tempreg = ust || large_offset ? op[0] : AT;
      macro_build (ep, s, "t,o(b)", tempreg, -1,
		   offset_reloc[0], offset_reloc[1], offset_reloc[2], breg);

      /* For halfword transfers we need a temporary register to shuffle
         bytes.  Unfortunately for M_USH_A we have none available before
         the next store as AT holds the base address.  We deal with this
         case by clobbering TREG and then restoring it as with ULH.  */
      tempreg = ust == large_offset ? op[0] : AT;
      if (ust)
	macro_build (NULL, "srl", SHFT_FMT, tempreg, op[0], 8);

      if (target_big_endian == ust)
	ep->X_add_number -= off;
      else
	ep->X_add_number += off;
      macro_build (ep, s2, "t,o(b)", tempreg, -1,
		   offset_reloc[0], offset_reloc[1], offset_reloc[2], breg);

      /* For M_USH_A re-retrieve the LSB.  */
      if (ust && large_offset)
	{
	  if (target_big_endian)
	    ep->X_add_number += off;
	  else
	    ep->X_add_number -= off;
	  macro_build (&expr1, "lbu", "t,o(b)", AT, -1,
		       offset_reloc[0], offset_reloc[1], offset_reloc[2], AT);
	}
      /* For ULH and M_USH_A OR the LSB in.  */
      if (!ust || large_offset)
	{
	  tempreg = !large_offset ? AT : op[0];
	  macro_build (NULL, "sll", SHFT_FMT, tempreg, tempreg, 8);
	  macro_build (NULL, "or", "d,v,t", op[0], op[0], AT);
	}
      break;

    default:
      /* FIXME: Check if this is one of the itbl macros, since they
	 are added dynamically.  */
      as_bad (_("macro %s not implemented yet"), ip->insn_mo->name);
      break;
    }
  if (!mips_opts.at && used_at)
    as_bad (_("macro used $at after \".set noat\""));
}

/* Implement macros in mips16 mode.  */

static void
mips16_macro (struct mips_cl_insn *ip)
{
  const struct mips_operand_array *operands;
  int mask;
  int tmp;
  expressionS expr1;
  int dbl;
  const char *s, *s2, *s3;
  unsigned int op[MAX_OPERANDS];
  unsigned int i;

  mask = ip->insn_mo->mask;

  operands = insn_operands (ip);
  for (i = 0; i < MAX_OPERANDS; i++)
    if (operands->operand[i])
      op[i] = insn_extract_operand (ip, operands->operand[i]);
    else
      op[i] = -1;

  expr1.X_op = O_constant;
  expr1.X_op_symbol = NULL;
  expr1.X_add_symbol = NULL;
  expr1.X_add_number = 1;

  dbl = 0;

  switch (mask)
    {
    default:
      abort ();

    case M_DDIV_3:
      dbl = 1;
      /* Fall through.  */
    case M_DIV_3:
      s = "mflo";
      goto do_div3;
    case M_DREM_3:
      dbl = 1;
      /* Fall through.  */
    case M_REM_3:
      s = "mfhi";
    do_div3:
      start_noreorder ();
      macro_build (NULL, dbl ? "ddiv" : "div", ".,x,y", op[1], op[2]);
      expr1.X_add_number = 2;
      macro_build (&expr1, "bnez", "x,p", op[2]);
      macro_build (NULL, "break", "6", 7);

      /* FIXME: The normal code checks for of -1 / -0x80000000 here,
         since that causes an overflow.  We should do that as well,
         but I don't see how to do the comparisons without a temporary
         register.  */
      end_noreorder ();
      macro_build (NULL, s, "x", op[0]);
      break;

    case M_DIVU_3:
      s = "divu";
      s2 = "mflo";
      goto do_divu3;
    case M_REMU_3:
      s = "divu";
      s2 = "mfhi";
      goto do_divu3;
    case M_DDIVU_3:
      s = "ddivu";
      s2 = "mflo";
      goto do_divu3;
    case M_DREMU_3:
      s = "ddivu";
      s2 = "mfhi";
    do_divu3:
      start_noreorder ();
      macro_build (NULL, s, ".,x,y", op[1], op[2]);
      expr1.X_add_number = 2;
      macro_build (&expr1, "bnez", "x,p", op[2]);
      macro_build (NULL, "break", "6", 7);
      end_noreorder ();
      macro_build (NULL, s2, "x", op[0]);
      break;

    case M_DMUL:
      dbl = 1;
      /* Fall through.  */
    case M_MUL:
      macro_build (NULL, dbl ? "dmultu" : "multu", "x,y", op[1], op[2]);
      macro_build (NULL, "mflo", "x", op[0]);
      break;

    case M_DSUBU_I:
      dbl = 1;
      goto do_subu;
    case M_SUBU_I:
    do_subu:
      imm_expr.X_add_number = -imm_expr.X_add_number;
      macro_build (&imm_expr, dbl ? "daddiu" : "addiu", "y,x,F", op[0], op[1]);
      break;

    case M_SUBU_I_2:
      imm_expr.X_add_number = -imm_expr.X_add_number;
      macro_build (&imm_expr, "addiu", "x,k", op[0]);
      break;

    case M_DSUBU_I_2:
      imm_expr.X_add_number = -imm_expr.X_add_number;
      macro_build (&imm_expr, "daddiu", "y,j", op[0]);
      break;

    case M_BEQ:
      s = "cmp";
      s2 = "bteqz";
      goto do_branch;
    case M_BNE:
      s = "cmp";
      s2 = "btnez";
      goto do_branch;
    case M_BLT:
      s = "slt";
      s2 = "btnez";
      goto do_branch;
    case M_BLTU:
      s = "sltu";
      s2 = "btnez";
      goto do_branch;
    case M_BLE:
      s = "slt";
      s2 = "bteqz";
      goto do_reverse_branch;
    case M_BLEU:
      s = "sltu";
      s2 = "bteqz";
      goto do_reverse_branch;
    case M_BGE:
      s = "slt";
      s2 = "bteqz";
      goto do_branch;
    case M_BGEU:
      s = "sltu";
      s2 = "bteqz";
      goto do_branch;
    case M_BGT:
      s = "slt";
      s2 = "btnez";
      goto do_reverse_branch;
    case M_BGTU:
      s = "sltu";
      s2 = "btnez";

    do_reverse_branch:
      tmp = op[1];
      op[1] = op[0];
      op[0] = tmp;

    do_branch:
      macro_build (NULL, s, "x,y", op[0], op[1]);
      macro_build (&offset_expr, s2, "p");
      break;

    case M_BEQ_I:
      s = "cmpi";
      s2 = "bteqz";
      s3 = "x,U";
      goto do_branch_i;
    case M_BNE_I:
      s = "cmpi";
      s2 = "btnez";
      s3 = "x,U";
      goto do_branch_i;
    case M_BLT_I:
      s = "slti";
      s2 = "btnez";
      s3 = "x,8";
      goto do_branch_i;
    case M_BLTU_I:
      s = "sltiu";
      s2 = "btnez";
      s3 = "x,8";
      goto do_branch_i;
    case M_BLE_I:
      s = "slti";
      s2 = "btnez";
      s3 = "x,8";
      goto do_addone_branch_i;
    case M_BLEU_I:
      s = "sltiu";
      s2 = "btnez";
      s3 = "x,8";
      goto do_addone_branch_i;
    case M_BGE_I:
      s = "slti";
      s2 = "bteqz";
      s3 = "x,8";
      goto do_branch_i;
    case M_BGEU_I:
      s = "sltiu";
      s2 = "bteqz";
      s3 = "x,8";
      goto do_branch_i;
    case M_BGT_I:
      s = "slti";
      s2 = "bteqz";
      s3 = "x,8";
      goto do_addone_branch_i;
    case M_BGTU_I:
      s = "sltiu";
      s2 = "bteqz";
      s3 = "x,8";

    do_addone_branch_i:
      ++imm_expr.X_add_number;

    do_branch_i:
      macro_build (&imm_expr, s, s3, op[0]);
      macro_build (&offset_expr, s2, "p");
      break;

    case M_ABS:
      expr1.X_add_number = 0;
      macro_build (&expr1, "slti", "x,8", op[1]);
      if (op[0] != op[1])
	macro_build (NULL, "move", "y,X", op[0], mips16_to_32_reg_map[op[1]]);
      expr1.X_add_number = 2;
      macro_build (&expr1, "bteqz", "p");
      macro_build (NULL, "neg", "x,w", op[0], op[0]);
      break;
    }
}

/* Look up instruction [START, START + LENGTH) in HASH.  Record any extra
   opcode bits in *OPCODE_EXTRA.  */

static struct mips_opcode *
mips_lookup_insn (htab_t hash, const char *start,
		  ssize_t length, unsigned int *opcode_extra)
{
  char *name, *dot, *p;
  unsigned int mask, suffix;
  ssize_t opend;
  struct mips_opcode *insn;

  /* Make a copy of the instruction so that we can fiddle with it.  */
  name = xstrndup (start, length);

  /* Look up the instruction as-is.  */
  insn = (struct mips_opcode *) str_hash_find (hash, name);
  if (insn)
    goto end;

  dot = strchr (name, '.');
  if (dot && dot[1])
    {
      /* Try to interpret the text after the dot as a VU0 channel suffix.  */
      p = mips_parse_vu0_channels (dot + 1, &mask);
      if (*p == 0 && mask != 0)
	{
	  *dot = 0;
	  insn = (struct mips_opcode *) str_hash_find (hash, name);
	  *dot = '.';
	  if (insn && (insn->pinfo2 & INSN2_VU0_CHANNEL_SUFFIX) != 0)
	    {
	      *opcode_extra |= mask << mips_vu0_channel_mask.lsb;
	      goto end;
	    }
	}
    }

  if (mips_opts.micromips)
    {
      /* See if there's an instruction size override suffix,
	 either `16' or `32', at the end of the mnemonic proper,
	 that defines the operation, i.e. before the first `.'
	 character if any.  Strip it and retry.  */
      opend = dot != NULL ? dot - name : length;
      if (opend >= 3 && name[opend - 2] == '1' && name[opend - 1] == '6')
	suffix = 2;
      else if (opend >= 2 && name[opend - 2] == '3' && name[opend - 1] == '2')
	suffix = 4;
      else
	suffix = 0;
      if (suffix)
	{
	  memmove (name + opend - 2, name + opend, length - opend + 1);
	  insn = (struct mips_opcode *) str_hash_find (hash, name);
	  if (insn)
	    {
	      forced_insn_length = suffix;
	      goto end;
	    }
	}
    }

  insn = NULL;
 end:
  free (name);
  return insn;
}

/* Assemble an instruction into its binary format.  If the instruction
   is a macro, set imm_expr and offset_expr to the values associated
   with "I" and "A" operands respectively.  Otherwise store the value
   of the relocatable field (if any) in offset_expr.  In both cases
   set offset_reloc to the relocation operators applied to offset_expr.  */

static void
mips_ip (char *str, struct mips_cl_insn *insn)
{
  const struct mips_opcode *first, *past;
  htab_t hash;
  char format;
  size_t end;
  struct mips_operand_token *tokens;
  unsigned int opcode_extra;

  if (mips_opts.micromips)
    {
      hash = micromips_op_hash;
      past = &micromips_opcodes[bfd_micromips_num_opcodes];
    }
  else
    {
      hash = op_hash;
      past = &mips_opcodes[NUMOPCODES];
    }
  forced_insn_length = 0;
  opcode_extra = 0;

  /* We first try to match an instruction up to a space or to the end.  */
  for (end = 0; str[end] != '\0' && !ISSPACE (str[end]); end++)
    continue;

  first = mips_lookup_insn (hash, str, end, &opcode_extra);
  if (first == NULL)
    {
      set_insn_error (0, _("unrecognized opcode"));
      return;
    }

  if (strcmp (first->name, "li.s") == 0)
    format = 'f';
  else if (strcmp (first->name, "li.d") == 0)
    format = 'd';
  else
    format = 0;
  tokens = mips_parse_arguments (str + end, format);
  if (!tokens)
    return;

  if (!match_insns (insn, first, past, tokens, opcode_extra, false)
      && !match_insns (insn, first, past, tokens, opcode_extra, true))
    set_insn_error (0, _("invalid operands"));

  obstack_free (&mips_operand_tokens, tokens);
}

/* As for mips_ip, but used when assembling MIPS16 code.
   Also set forced_insn_length to the resulting instruction size in
   bytes if the user explicitly requested a small or extended instruction.  */

static void
mips16_ip (char *str, struct mips_cl_insn *insn)
{
  char *end, *s, c;
  struct mips_opcode *first;
  struct mips_operand_token *tokens;
  unsigned int l;

  for (s = str; *s != '\0' && *s != '.' && *s != ' '; ++s)
    ;
  end = s;
  c = *end;

  l = 0;
  switch (c)
    {
    case '\0':
      break;

    case ' ':
      s++;
      break;

    case '.':
      s++;
      if (*s == 't')
	{
	  l = 2;
	  s++;
	}
      else if (*s == 'e')
	{
	  l = 4;
	  s++;
	}
      if (*s == '\0')
	break;
      else if (*s++ == ' ')
	break;
      set_insn_error (0, _("unrecognized opcode"));
      return;
    }
  forced_insn_length = l;

  *end = 0;
  first = (struct mips_opcode *) str_hash_find (mips16_op_hash, str);
  *end = c;

  if (!first)
    {
      set_insn_error (0, _("unrecognized opcode"));
      return;
    }

  tokens = mips_parse_arguments (s, 0);
  if (!tokens)
    return;

  if (!match_mips16_insns (insn, first, tokens))
    set_insn_error (0, _("invalid operands"));

  obstack_free (&mips_operand_tokens, tokens);
}

/* Marshal immediate value VAL for an extended MIPS16 instruction.
   NBITS is the number of significant bits in VAL.  */

static unsigned long
mips16_immed_extend (offsetT val, unsigned int nbits)
{
  int extval;

  extval = 0;
  val &= (1U << nbits) - 1;
  if (nbits == 16 || nbits == 9)
    {
      extval = ((val >> 11) & 0x1f) | (val & 0x7e0);
      val &= 0x1f;
    }
  else if (nbits == 15)
    {
      extval = ((val >> 11) & 0xf) | (val & 0x7f0);
      val &= 0xf;
    }
  else if (nbits == 6)
    {
      extval = ((val & 0x1f) << 6) | (val & 0x20);
      val = 0;
    }
  return (extval << 16) | val;
}

/* Like decode_mips16_operand, but require the operand to be defined and
   require it to be an integer.  */

static const struct mips_int_operand *
mips16_immed_operand (int type, bool extended_p)
{
  const struct mips_operand *operand;

  operand = decode_mips16_operand (type, extended_p);
  if (!operand || (operand->type != OP_INT && operand->type != OP_PCREL))
    abort ();
  return (const struct mips_int_operand *) operand;
}

/* Return true if SVAL fits OPERAND.  RELOC is as for mips16_immed.  */

static bool
mips16_immed_in_range_p (const struct mips_int_operand *operand,
			 bfd_reloc_code_real_type reloc, offsetT sval)
{
  int min_val, max_val;

  min_val = mips_int_operand_min (operand);
  max_val = mips_int_operand_max (operand);
  if (reloc != BFD_RELOC_UNUSED)
    {
      if (min_val < 0)
	sval = SEXT_16BIT (sval);
      else
	sval &= 0xffff;
    }

  return (sval >= min_val
	  && sval <= max_val
	  && (sval & ((1 << operand->shift) - 1)) == 0);
}

/* Install immediate value VAL into MIPS16 instruction *INSN,
   extending it if necessary.  The instruction in *INSN may
   already be extended.

   RELOC is the relocation that produced VAL, or BFD_RELOC_UNUSED
   if none.  In the former case, VAL is a 16-bit number with no
   defined signedness.

   TYPE is the type of the immediate field.  USER_INSN_LENGTH
   is the length that the user requested, or 0 if none.  */

static void
mips16_immed (const char *file, unsigned int line, int type,
	      bfd_reloc_code_real_type reloc, offsetT val,
	      unsigned int user_insn_length, unsigned long *insn)
{
  const struct mips_int_operand *operand;
  unsigned int uval, length;

  operand = mips16_immed_operand (type, false);
  if (!mips16_immed_in_range_p (operand, reloc, val))
    {
      /* We need an extended instruction.  */
      if (user_insn_length == 2)
	as_bad_where (file, line, _("invalid unextended operand value"));
      else
	*insn |= MIPS16_EXTEND;
    }
  else if (user_insn_length == 4)
    {
      /* The operand doesn't force an unextended instruction to be extended.
	 Warn if the user wanted an extended instruction anyway.  */
      *insn |= MIPS16_EXTEND;
      as_warn_where (file, line,
		     _("extended operand requested but not required"));
    }

  length = mips16_opcode_length (*insn);
  if (length == 4)
    {
      operand = mips16_immed_operand (type, true);
      if (!mips16_immed_in_range_p (operand, reloc, val))
	as_bad_where (file, line,
		      _("operand value out of range for instruction"));
    }
  uval = ((unsigned int) val >> operand->shift) - operand->bias;
  if (length == 2 || operand->root.lsb != 0)
    *insn = mips_insert_operand (&operand->root, *insn, uval);
  else
    *insn |= mips16_immed_extend (uval, operand->root.size);
}

struct percent_op_match
{
  const char *str;
  bfd_reloc_code_real_type reloc;
};

static const struct percent_op_match mips_percent_op[] =
{
  {"%lo", BFD_RELOC_LO16},
  {"%call_hi", BFD_RELOC_MIPS_CALL_HI16},
  {"%call_lo", BFD_RELOC_MIPS_CALL_LO16},
  {"%call16", BFD_RELOC_MIPS_CALL16},
  {"%got_disp", BFD_RELOC_MIPS_GOT_DISP},
  {"%got_page", BFD_RELOC_MIPS_GOT_PAGE},
  {"%got_ofst", BFD_RELOC_MIPS_GOT_OFST},
  {"%got_hi", BFD_RELOC_MIPS_GOT_HI16},
  {"%got_lo", BFD_RELOC_MIPS_GOT_LO16},
  {"%got", BFD_RELOC_MIPS_GOT16},
  {"%gp_rel", BFD_RELOC_GPREL16},
  {"%gprel", BFD_RELOC_GPREL16},
  {"%half", BFD_RELOC_MIPS_16},
  {"%highest", BFD_RELOC_MIPS_HIGHEST},
  {"%higher", BFD_RELOC_MIPS_HIGHER},
  {"%neg", BFD_RELOC_MIPS_SUB},
  {"%tlsgd", BFD_RELOC_MIPS_TLS_GD},
  {"%tlsldm", BFD_RELOC_MIPS_TLS_LDM},
  {"%dtprel_hi", BFD_RELOC_MIPS_TLS_DTPREL_HI16},
  {"%dtprel_lo", BFD_RELOC_MIPS_TLS_DTPREL_LO16},
  {"%tprel_hi", BFD_RELOC_MIPS_TLS_TPREL_HI16},
  {"%tprel_lo", BFD_RELOC_MIPS_TLS_TPREL_LO16},
  {"%gottprel", BFD_RELOC_MIPS_TLS_GOTTPREL},
  {"%hi", BFD_RELOC_HI16_S},
  {"%pcrel_hi", BFD_RELOC_HI16_S_PCREL},
  {"%pcrel_lo", BFD_RELOC_LO16_PCREL}
};

static const struct percent_op_match mips16_percent_op[] =
{
  {"%lo", BFD_RELOC_MIPS16_LO16},
  {"%gp_rel", BFD_RELOC_MIPS16_GPREL},
  {"%gprel", BFD_RELOC_MIPS16_GPREL},
  {"%got", BFD_RELOC_MIPS16_GOT16},
  {"%call16", BFD_RELOC_MIPS16_CALL16},
  {"%hi", BFD_RELOC_MIPS16_HI16_S},
  {"%tlsgd", BFD_RELOC_MIPS16_TLS_GD},
  {"%tlsldm", BFD_RELOC_MIPS16_TLS_LDM},
  {"%dtprel_hi", BFD_RELOC_MIPS16_TLS_DTPREL_HI16},
  {"%dtprel_lo", BFD_RELOC_MIPS16_TLS_DTPREL_LO16},
  {"%tprel_hi", BFD_RELOC_MIPS16_TLS_TPREL_HI16},
  {"%tprel_lo", BFD_RELOC_MIPS16_TLS_TPREL_LO16},
  {"%gottprel", BFD_RELOC_MIPS16_TLS_GOTTPREL}
};


/* Return true if *STR points to a relocation operator.  When returning true,
   move *STR over the operator and store its relocation code in *RELOC.
   Leave both *STR and *RELOC alone when returning false.  */

static bool
parse_relocation (char **str, bfd_reloc_code_real_type *reloc)
{
  const struct percent_op_match *percent_op;
  size_t limit, i;

  if (mips_opts.mips16)
    {
      percent_op = mips16_percent_op;
      limit = ARRAY_SIZE (mips16_percent_op);
    }
  else
    {
      percent_op = mips_percent_op;
      limit = ARRAY_SIZE (mips_percent_op);
    }

  for (i = 0; i < limit; i++)
    if (strncasecmp (*str, percent_op[i].str, strlen (percent_op[i].str)) == 0)
      {
	int len = strlen (percent_op[i].str);

	if (!ISSPACE ((*str)[len]) && (*str)[len] != '(')
	  continue;

	*str += strlen (percent_op[i].str);
	*reloc = percent_op[i].reloc;

	/* Check whether the output BFD supports this relocation.
	   If not, issue an error and fall back on something safe.  */
	if (!bfd_reloc_type_lookup (stdoutput, percent_op[i].reloc))
	  {
	    as_bad (_("relocation %s isn't supported by the current ABI"),
		    percent_op[i].str);
	    *reloc = BFD_RELOC_UNUSED;
	  }
	return true;
      }
  return false;
}


/* Parse string STR as a 16-bit relocatable operand.  Store the
   expression in *EP and the relocations in the array starting
   at RELOC.  Return the number of relocation operators used.

   On exit, EXPR_PARSE_END points to the first character after the
   expression.  */

static size_t
my_getSmallExpression (expressionS *ep, bfd_reloc_code_real_type *reloc,
		       char *str)
{
  bfd_reloc_code_real_type reversed_reloc[3];
  size_t reloc_index, i;
  int crux_depth, str_depth;
  char *crux;

  /* Search for the start of the main expression, recoding relocations
     in REVERSED_RELOC.  End the loop with CRUX pointing to the start
     of the main expression and with CRUX_DEPTH containing the number
     of open brackets at that point.  */
  reloc_index = -1;
  str_depth = 0;
  do
    {
      reloc_index++;
      crux = str;
      crux_depth = str_depth;

      /* Skip over whitespace and brackets, keeping count of the number
	 of brackets.  */
      while (*str == ' ' || *str == '\t' || *str == '(')
	if (*str++ == '(')
	  str_depth++;
    }
  while (*str == '%'
	 && reloc_index < (HAVE_NEWABI ? 3 : 1)
	 && parse_relocation (&str, &reversed_reloc[reloc_index]));

  my_getExpression (ep, crux);
  str = expr_parse_end;

  /* Match every open bracket.  */
  while (crux_depth > 0 && (*str == ')' || *str == ' ' || *str == '\t'))
    if (*str++ == ')')
      crux_depth--;

  if (crux_depth > 0)
    as_bad (_("unclosed '('"));

  expr_parse_end = str;

  for (i = 0; i < reloc_index; i++)
    reloc[i] = reversed_reloc[reloc_index - 1 - i];

  return reloc_index;
}

static void
my_getExpression (expressionS *ep, char *str)
{
  char *save_in;

  save_in = input_line_pointer;
  input_line_pointer = str;
  expression (ep);
  expr_parse_end = input_line_pointer;
  input_line_pointer = save_in;
}

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, target_big_endian);
}

void
md_number_to_chars (char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

static int support_64bit_objects(void)
{
  const char **list, **l;
  int yes;

  list = bfd_target_list ();
  for (l = list; *l != NULL; l++)
    if (strcmp (*l, ELF_TARGET ("elf64-", "big")) == 0
	|| strcmp (*l, ELF_TARGET ("elf64-", "little")) == 0)
      break;
  yes = (*l != NULL);
  free (list);
  return yes;
}

/* Set STRING_PTR (either &mips_arch_string or &mips_tune_string) to
   NEW_VALUE.  Warn if another value was already specified.  Note:
   we have to defer parsing the -march and -mtune arguments in order
   to handle 'from-abi' correctly, since the ABI might be specified
   in a later argument.  */

static void
mips_set_option_string (const char **string_ptr, const char *new_value)
{
  if (*string_ptr != 0 && strcasecmp (*string_ptr, new_value) != 0)
    as_warn (_("a different %s was already specified, is now %s"),
	     string_ptr == &mips_arch_string ? "-march" : "-mtune",
	     new_value);

  *string_ptr = new_value;
}

int
md_parse_option (int c, const char *arg)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (mips_ases); i++)
    if (c == mips_ases[i].option_on || c == mips_ases[i].option_off)
      {
	file_ase_explicit |= mips_set_ase (&mips_ases[i], &file_mips_opts,
					   c == mips_ases[i].option_on);
	return 1;
      }

  switch (c)
    {
    case OPTION_CONSTRUCT_FLOATS:
      mips_disable_float_construction = 0;
      break;

    case OPTION_NO_CONSTRUCT_FLOATS:
      mips_disable_float_construction = 1;
      break;

    case OPTION_TRAP:
      mips_trap = 1;
      break;

    case OPTION_BREAK:
      mips_trap = 0;
      break;

    case OPTION_EB:
      target_big_endian = 1;
      break;

    case OPTION_EL:
      target_big_endian = 0;
      break;

    case 'O':
      if (arg == NULL)
	mips_optimize = 1;
      else if (arg[0] == '0')
	mips_optimize = 0;
      else if (arg[0] == '1')
	mips_optimize = 1;
      else
	mips_optimize = 2;
      break;

    case 'g':
      if (arg == NULL)
	mips_debug = 2;
      else
	mips_debug = atoi (arg);
      break;

    case OPTION_MIPS1:
      file_mips_opts.isa = ISA_MIPS1;
      break;

    case OPTION_MIPS2:
      file_mips_opts.isa = ISA_MIPS2;
      break;

    case OPTION_MIPS3:
      file_mips_opts.isa = ISA_MIPS3;
      break;

    case OPTION_MIPS4:
      file_mips_opts.isa = ISA_MIPS4;
      break;

    case OPTION_MIPS5:
      file_mips_opts.isa = ISA_MIPS5;
      break;

    case OPTION_MIPS32:
      file_mips_opts.isa = ISA_MIPS32;
      break;

    case OPTION_MIPS32R2:
      file_mips_opts.isa = ISA_MIPS32R2;
      break;

    case OPTION_MIPS32R3:
      file_mips_opts.isa = ISA_MIPS32R3;
      break;

    case OPTION_MIPS32R5:
      file_mips_opts.isa = ISA_MIPS32R5;
      break;

    case OPTION_MIPS32R6:
      file_mips_opts.isa = ISA_MIPS32R6;
      break;

    case OPTION_MIPS64R2:
      file_mips_opts.isa = ISA_MIPS64R2;
      break;

    case OPTION_MIPS64R3:
      file_mips_opts.isa = ISA_MIPS64R3;
      break;

    case OPTION_MIPS64R5:
      file_mips_opts.isa = ISA_MIPS64R5;
      break;

    case OPTION_MIPS64R6:
      file_mips_opts.isa = ISA_MIPS64R6;
      break;

    case OPTION_MIPS64:
      file_mips_opts.isa = ISA_MIPS64;
      break;

    case OPTION_MTUNE:
      mips_set_option_string (&mips_tune_string, arg);
      break;

    case OPTION_MARCH:
      mips_set_option_string (&mips_arch_string, arg);
      break;

    case OPTION_M4650:
      mips_set_option_string (&mips_arch_string, "4650");
      mips_set_option_string (&mips_tune_string, "4650");
      break;

    case OPTION_NO_M4650:
      break;

    case OPTION_M4010:
      mips_set_option_string (&mips_arch_string, "4010");
      mips_set_option_string (&mips_tune_string, "4010");
      break;

    case OPTION_NO_M4010:
      break;

    case OPTION_M4100:
      mips_set_option_string (&mips_arch_string, "4100");
      mips_set_option_string (&mips_tune_string, "4100");
      break;

    case OPTION_NO_M4100:
      break;

    case OPTION_M3900:
      mips_set_option_string (&mips_arch_string, "3900");
      mips_set_option_string (&mips_tune_string, "3900");
      break;

    case OPTION_NO_M3900:
      break;

    case OPTION_MICROMIPS:
      if (file_mips_opts.mips16 == 1)
	{
	  as_bad (_("-mmicromips cannot be used with -mips16"));
	  return 0;
	}
      file_mips_opts.micromips = 1;
      mips_no_prev_insn ();
      break;

    case OPTION_NO_MICROMIPS:
      file_mips_opts.micromips = 0;
      mips_no_prev_insn ();
      break;

    case OPTION_MIPS16:
      if (file_mips_opts.micromips == 1)
	{
	  as_bad (_("-mips16 cannot be used with -micromips"));
	  return 0;
	}
      file_mips_opts.mips16 = 1;
      mips_no_prev_insn ();
      break;

    case OPTION_NO_MIPS16:
      file_mips_opts.mips16 = 0;
      mips_no_prev_insn ();
      break;

    case OPTION_FIX_24K:
      mips_fix_24k = 1;
      break;

    case OPTION_NO_FIX_24K:
      mips_fix_24k = 0;
      break;

    case OPTION_FIX_RM7000:
      mips_fix_rm7000 = 1;
      break;

    case OPTION_NO_FIX_RM7000:
      mips_fix_rm7000 = 0;
      break;

    case OPTION_FIX_LOONGSON3_LLSC:
      mips_fix_loongson3_llsc = true;
      break;

    case OPTION_NO_FIX_LOONGSON3_LLSC:
      mips_fix_loongson3_llsc = false;
      break;

    case OPTION_FIX_LOONGSON2F_JUMP:
      mips_fix_loongson2f_jump = true;
      break;

    case OPTION_NO_FIX_LOONGSON2F_JUMP:
      mips_fix_loongson2f_jump = false;
      break;

    case OPTION_FIX_LOONGSON2F_NOP:
      mips_fix_loongson2f_nop = true;
      break;

    case OPTION_NO_FIX_LOONGSON2F_NOP:
      mips_fix_loongson2f_nop = false;
      break;

    case OPTION_FIX_VR4120:
      mips_fix_vr4120 = 1;
      break;

    case OPTION_NO_FIX_VR4120:
      mips_fix_vr4120 = 0;
      break;

    case OPTION_FIX_VR4130:
      mips_fix_vr4130 = 1;
      break;

    case OPTION_NO_FIX_VR4130:
      mips_fix_vr4130 = 0;
      break;

    case OPTION_FIX_CN63XXP1:
      mips_fix_cn63xxp1 = true;
      break;

    case OPTION_NO_FIX_CN63XXP1:
      mips_fix_cn63xxp1 = false;
      break;

    case OPTION_FIX_R5900:
      mips_fix_r5900 = true;
      mips_fix_r5900_explicit = true;
      break;

    case OPTION_NO_FIX_R5900:
      mips_fix_r5900 = false;
      mips_fix_r5900_explicit = true;
      break;

    case OPTION_RELAX_BRANCH:
      mips_relax_branch = 1;
      break;

    case OPTION_NO_RELAX_BRANCH:
      mips_relax_branch = 0;
      break;

    case OPTION_IGNORE_BRANCH_ISA:
      mips_ignore_branch_isa = true;
      break;

    case OPTION_NO_IGNORE_BRANCH_ISA:
      mips_ignore_branch_isa = false;
      break;

    case OPTION_INSN32:
      file_mips_opts.insn32 = true;
      break;

    case OPTION_NO_INSN32:
      file_mips_opts.insn32 = false;
      break;

    case OPTION_MSHARED:
      mips_in_shared = true;
      break;

    case OPTION_MNO_SHARED:
      mips_in_shared = false;
      break;

    case OPTION_MSYM32:
      file_mips_opts.sym32 = true;
      break;

    case OPTION_MNO_SYM32:
      file_mips_opts.sym32 = false;
      break;

      /* When generating ELF code, we permit -KPIC and -call_shared to
	 select SVR4_PIC, and -non_shared to select no PIC.  This is
	 intended to be compatible with Irix 5.  */
    case OPTION_CALL_SHARED:
      mips_pic = SVR4_PIC;
      mips_abicalls = true;
      break;

    case OPTION_CALL_NONPIC:
      mips_pic = NO_PIC;
      mips_abicalls = true;
      break;

    case OPTION_NON_SHARED:
      mips_pic = NO_PIC;
      mips_abicalls = false;
      break;

      /* The -xgot option tells the assembler to use 32 bit offsets
         when accessing the got in SVR4_PIC mode.  It is for Irix
         compatibility.  */
    case OPTION_XGOT:
      mips_big_got = 1;
      break;

    case 'G':
      g_switch_value = atoi (arg);
      g_switch_seen = 1;
      break;

      /* The -32, -n32 and -64 options are shortcuts for -mabi=32, -mabi=n32
	 and -mabi=64.  */
    case OPTION_32:
      mips_abi = O32_ABI;
      break;

    case OPTION_N32:
      mips_abi = N32_ABI;
      break;

    case OPTION_64:
      mips_abi = N64_ABI;
      if (!support_64bit_objects())
	as_fatal (_("no compiled in support for 64 bit object file format"));
      break;

    case OPTION_GP32:
      file_mips_opts.gp = 32;
      break;

    case OPTION_GP64:
      file_mips_opts.gp = 64;
      break;

    case OPTION_FP32:
      file_mips_opts.fp = 32;
      break;

    case OPTION_FPXX:
      file_mips_opts.fp = 0;
      break;

    case OPTION_FP64:
      file_mips_opts.fp = 64;
      break;

    case OPTION_ODD_SPREG:
      file_mips_opts.oddspreg = 1;
      break;

    case OPTION_NO_ODD_SPREG:
      file_mips_opts.oddspreg = 0;
      break;

    case OPTION_SINGLE_FLOAT:
      file_mips_opts.single_float = 1;
      break;

    case OPTION_DOUBLE_FLOAT:
      file_mips_opts.single_float = 0;
      break;

    case OPTION_SOFT_FLOAT:
      file_mips_opts.soft_float = 1;
      break;

    case OPTION_HARD_FLOAT:
      file_mips_opts.soft_float = 0;
      break;

    case OPTION_MABI:
      if (strcmp (arg, "32") == 0)
	mips_abi = O32_ABI;
      else if (strcmp (arg, "o64") == 0)
	mips_abi = O64_ABI;
      else if (strcmp (arg, "n32") == 0)
	mips_abi = N32_ABI;
      else if (strcmp (arg, "64") == 0)
	{
	  mips_abi = N64_ABI;
	  if (! support_64bit_objects())
	    as_fatal (_("no compiled in support for 64 bit object file "
			"format"));
	}
      else if (strcmp (arg, "eabi") == 0)
	mips_abi = EABI_ABI;
      else
	{
	  as_fatal (_("invalid abi -mabi=%s"), arg);
	  return 0;
	}
      break;

    case OPTION_M7000_HILO_FIX:
      mips_7000_hilo_fix = true;
      break;

    case OPTION_MNO_7000_HILO_FIX:
      mips_7000_hilo_fix = false;
      break;

    case OPTION_MDEBUG:
      mips_flag_mdebug = true;
      break;

    case OPTION_NO_MDEBUG:
      mips_flag_mdebug = false;
      break;

    case OPTION_PDR:
      mips_flag_pdr = true;
      break;

    case OPTION_NO_PDR:
      mips_flag_pdr = false;
      break;

    case OPTION_MVXWORKS_PIC:
      mips_pic = VXWORKS_PIC;
      break;

    case OPTION_NAN:
      if (strcmp (arg, "2008") == 0)
	mips_nan2008 = 1;
      else if (strcmp (arg, "legacy") == 0)
	mips_nan2008 = 0;
      else
	{
	  as_fatal (_("invalid NaN setting -mnan=%s"), arg);
	  return 0;
	}
      break;

    default:
      return 0;
    }

    mips_fix_loongson2f = mips_fix_loongson2f_nop || mips_fix_loongson2f_jump;

  return 1;
}

/* Set up globals to tune for the ISA or processor described by INFO.  */

static void
mips_set_tune (const struct mips_cpu_info *info)
{
  if (info != 0)
    mips_tune = info->cpu;
}


void
mips_after_parse_args (void)
{
  const struct mips_cpu_info *arch_info = 0;
  const struct mips_cpu_info *tune_info = 0;

  /* GP relative stuff not working for PE.  */
  if (startswith (TARGET_OS, "pe"))
    {
      if (g_switch_seen && g_switch_value != 0)
	as_bad (_("-G not supported in this configuration"));
      g_switch_value = 0;
    }

  if (mips_abi == NO_ABI)
    mips_abi = MIPS_DEFAULT_ABI;

  /* The following code determines the architecture.
     Similar code was added to GCC 3.3 (see override_options() in
     config/mips/mips.c).  The GAS and GCC code should be kept in sync
     as much as possible.  */

  if (mips_arch_string != 0)
    arch_info = mips_parse_cpu ("-march", mips_arch_string);

  if (file_mips_opts.isa != ISA_UNKNOWN)
    {
      /* Handle -mipsN.  At this point, file_mips_opts.isa contains the
	 ISA level specified by -mipsN, while arch_info->isa contains
	 the -march selection (if any).  */
      if (arch_info != 0)
	{
	  /* -march takes precedence over -mipsN, since it is more descriptive.
	     There's no harm in specifying both as long as the ISA levels
	     are the same.  */
	  if (file_mips_opts.isa != arch_info->isa)
	    as_bad (_("-%s conflicts with the other architecture options,"
		      " which imply -%s"),
		    mips_cpu_info_from_isa (file_mips_opts.isa)->name,
		    mips_cpu_info_from_isa (arch_info->isa)->name);
	}
      else
	arch_info = mips_cpu_info_from_isa (file_mips_opts.isa);
    }

  if (arch_info == 0)
    {
      arch_info = mips_parse_cpu ("default CPU", MIPS_CPU_STRING_DEFAULT);
      gas_assert (arch_info);
    }

  if (ABI_NEEDS_64BIT_REGS (mips_abi) && !ISA_HAS_64BIT_REGS (arch_info->isa))
    as_bad (_("-march=%s is not compatible with the selected ABI"),
	    arch_info->name);

  file_mips_opts.arch = arch_info->cpu;
  file_mips_opts.isa = arch_info->isa;
  file_mips_opts.init_ase = arch_info->ase;

  /* The EVA Extension has instructions which are only valid when the R6 ISA
     is enabled.  This sets the ASE_EVA_R6 flag when both EVA and R6 ISA are
     present.  */
  if (((file_mips_opts.ase & ASE_EVA) != 0) && ISA_IS_R6 (file_mips_opts.isa))
    file_mips_opts.ase |= ASE_EVA_R6;

  /* Set up initial mips_opts state.  */
  mips_opts = file_mips_opts;

  /* For the R5900 default to `-mfix-r5900' unless the user told otherwise.  */
  if (!mips_fix_r5900_explicit)
    mips_fix_r5900 = file_mips_opts.arch == CPU_R5900;

  /* The register size inference code is now placed in
     file_mips_check_options.  */

  /* Optimize for file_mips_opts.arch, unless -mtune selects a different
     processor.  */
  if (mips_tune_string != 0)
    tune_info = mips_parse_cpu ("-mtune", mips_tune_string);

  if (tune_info == 0)
    mips_set_tune (arch_info);
  else
    mips_set_tune (tune_info);

  if (mips_flag_mdebug < 0)
    mips_flag_mdebug = 0;
}

void
mips_init_after_args (void)
{
  /* Initialize opcodes.  */
  bfd_mips_num_opcodes = bfd_mips_num_builtin_opcodes;
  mips_opcodes = (struct mips_opcode *) mips_builtin_opcodes;
}

long
md_pcrel_from (fixS *fixP)
{
  valueT addr = fixP->fx_where + fixP->fx_frag->fr_address;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_MICROMIPS_7_PCREL_S1:
    case BFD_RELOC_MICROMIPS_10_PCREL_S1:
      /* Return the address of the delay slot.  */
      return addr + 2;

    case BFD_RELOC_MICROMIPS_16_PCREL_S1:
    case BFD_RELOC_MICROMIPS_JMP:
    case BFD_RELOC_MIPS16_16_PCREL_S1:
    case BFD_RELOC_16_PCREL_S2:
    case BFD_RELOC_MIPS_21_PCREL_S2:
    case BFD_RELOC_MIPS_26_PCREL_S2:
    case BFD_RELOC_MIPS_JMP:
      /* Return the address of the delay slot.  */
      return addr + 4;

    case BFD_RELOC_MIPS_18_PCREL_S3:
      /* Return the aligned address of the doubleword containing
         the instruction.  */
      return addr & ~7;

    default:
      return addr;
    }
}

/* This is called before the symbol table is processed.  In order to
   work with gcc when using mips-tfile, we must keep all local labels.
   However, in other cases, we want to discard them.  If we were
   called with -g, but we didn't see any debugging information, it may
   mean that gcc is smuggling debugging information through to
   mips-tfile, in which case we must generate all local labels.  */

void
mips_frob_file_before_adjust (void)
{
#ifndef NO_ECOFF_DEBUGGING
  if (ECOFF_DEBUGGING
      && mips_debug != 0
      && ! ecoff_debugging_seen)
    flag_keep_locals = 1;
#endif
}

/* Sort any unmatched HI16 and GOT16 relocs so that they immediately precede
   the corresponding LO16 reloc.  This is called before md_apply_fix and
   tc_gen_reloc.  Unmatched relocs can only be generated by use of explicit
   relocation operators.

   For our purposes, a %lo() expression matches a %got() or %hi()
   expression if:

      (a) it refers to the same symbol; and
      (b) the offset applied in the %lo() expression is no lower than
	  the offset applied in the %got() or %hi().

   (b) allows us to cope with code like:

	lui	$4,%hi(foo)
	lh	$4,%lo(foo+2)($4)

   ...which is legal on RELA targets, and has a well-defined behaviour
   if the user knows that adding 2 to "foo" will not induce a carry to
   the high 16 bits.

   When several %lo()s match a particular %got() or %hi(), we use the
   following rules to distinguish them:

     (1) %lo()s with smaller offsets are a better match than %lo()s with
         higher offsets.

     (2) %lo()s with no matching %got() or %hi() are better than those
         that already have a matching %got() or %hi().

     (3) later %lo()s are better than earlier %lo()s.

   These rules are applied in order.

   (1) means, among other things, that %lo()s with identical offsets are
   chosen if they exist.

   (2) means that we won't associate several high-part relocations with
   the same low-part relocation unless there's no alternative.  Having
   several high parts for the same low part is a GNU extension; this rule
   allows careful users to avoid it.

   (3) is purely cosmetic.  mips_hi_fixup_list is is in reverse order,
   with the last high-part relocation being at the front of the list.
   It therefore makes sense to choose the last matching low-part
   relocation, all other things being equal.  It's also easier
   to code that way.  */

void
mips_frob_file (void)
{
  struct mips_hi_fixup *l;
  bfd_reloc_code_real_type looking_for_rtype = BFD_RELOC_UNUSED;

  for (l = mips_hi_fixup_list; l != NULL; l = l->next)
    {
      segment_info_type *seginfo;
      bool matched_lo_p;
      fixS **hi_pos, **lo_pos, **pos;

      gas_assert (reloc_needs_lo_p (l->fixp->fx_r_type));

      /* If a GOT16 relocation turns out to be against a global symbol,
	 there isn't supposed to be a matching LO.  Ignore %gots against
	 constants; we'll report an error for those later.  */
      if (got16_reloc_p (l->fixp->fx_r_type)
	  && !pic_need_relax (l->fixp->fx_addsy))
	continue;

      /* Check quickly whether the next fixup happens to be a matching %lo.  */
      if (fixup_has_matching_lo_p (l->fixp))
	continue;

      seginfo = seg_info (l->seg);

      /* Set HI_POS to the position of this relocation in the chain.
	 Set LO_POS to the position of the chosen low-part relocation.
	 MATCHED_LO_P is true on entry to the loop if *POS is a low-part
	 relocation that matches an immediately-preceding high-part
	 relocation.  */
      hi_pos = NULL;
      lo_pos = NULL;
      matched_lo_p = false;
      looking_for_rtype = matching_lo_reloc (l->fixp->fx_r_type);

      for (pos = &seginfo->fix_root; *pos != NULL; pos = &(*pos)->fx_next)
	{
	  if (*pos == l->fixp)
	    hi_pos = pos;

	  if ((*pos)->fx_r_type == looking_for_rtype
	      && symbol_same_p ((*pos)->fx_addsy, l->fixp->fx_addsy)
	      && (*pos)->fx_offset >= l->fixp->fx_offset
	      && (lo_pos == NULL
		  || (*pos)->fx_offset < (*lo_pos)->fx_offset
		  || (!matched_lo_p
		      && (*pos)->fx_offset == (*lo_pos)->fx_offset)))
	    lo_pos = pos;

	  matched_lo_p = (reloc_needs_lo_p ((*pos)->fx_r_type)
			  && fixup_has_matching_lo_p (*pos));
	}

      /* If we found a match, remove the high-part relocation from its
	 current position and insert it before the low-part relocation.
	 Make the offsets match so that fixup_has_matching_lo_p()
	 will return true.

	 We don't warn about unmatched high-part relocations since some
	 versions of gcc have been known to emit dead "lui ...%hi(...)"
	 instructions.  */
      if (lo_pos != NULL)
	{
	  l->fixp->fx_offset = (*lo_pos)->fx_offset;
	  if (l->fixp->fx_next != *lo_pos)
	    {
	      *hi_pos = l->fixp->fx_next;
	      l->fixp->fx_next = *lo_pos;
	      *lo_pos = l->fixp;
	    }
	}
    }
}

int
mips_force_relocation (fixS *fixp)
{
  if (generic_force_reloc (fixp))
    return 1;

  /* We want to keep BFD_RELOC_MICROMIPS_*_PCREL_S1 relocation,
     so that the linker relaxation can update targets.  */
  if (fixp->fx_r_type == BFD_RELOC_MICROMIPS_7_PCREL_S1
      || fixp->fx_r_type == BFD_RELOC_MICROMIPS_10_PCREL_S1
      || fixp->fx_r_type == BFD_RELOC_MICROMIPS_16_PCREL_S1)
    return 1;

  /* We want to keep BFD_RELOC_16_PCREL_S2 BFD_RELOC_MIPS_21_PCREL_S2
     and BFD_RELOC_MIPS_26_PCREL_S2 relocations against MIPS16 and
     microMIPS symbols so that we can do cross-mode branch diagnostics
     and BAL to JALX conversion by the linker.  */
  if ((fixp->fx_r_type == BFD_RELOC_16_PCREL_S2
       || fixp->fx_r_type == BFD_RELOC_MIPS_21_PCREL_S2
       || fixp->fx_r_type == BFD_RELOC_MIPS_26_PCREL_S2)
      && fixp->fx_addsy
      && ELF_ST_IS_COMPRESSED (S_GET_OTHER (fixp->fx_addsy)))
    return 1;

  /* We want all PC-relative relocations to be kept for R6 relaxation.  */
  if (ISA_IS_R6 (file_mips_opts.isa)
      && (fixp->fx_r_type == BFD_RELOC_16_PCREL_S2
	  || fixp->fx_r_type == BFD_RELOC_MIPS_21_PCREL_S2
	  || fixp->fx_r_type == BFD_RELOC_MIPS_26_PCREL_S2
	  || fixp->fx_r_type == BFD_RELOC_MIPS_18_PCREL_S3
	  || fixp->fx_r_type == BFD_RELOC_MIPS_19_PCREL_S2
	  || fixp->fx_r_type == BFD_RELOC_HI16_S_PCREL
	  || fixp->fx_r_type == BFD_RELOC_LO16_PCREL))
    return 1;

  return 0;
}

/* Implement TC_FORCE_RELOCATION_ABS.  */

bool
mips_force_relocation_abs (fixS *fixp)
{
  if (generic_force_reloc (fixp))
    return true;

  /* These relocations do not have enough bits in the in-place addend
     to hold an arbitrary absolute section's offset.  */
  if (HAVE_IN_PLACE_ADDENDS && limited_pcrel_reloc_p (fixp->fx_r_type))
    return true;

  return false;
}

/* Read the instruction associated with RELOC from BUF.  */

static unsigned int
read_reloc_insn (char *buf, bfd_reloc_code_real_type reloc)
{
  if (mips16_reloc_p (reloc) || micromips_reloc_p (reloc))
    return read_compressed_insn (buf, 4);
  else
    return read_insn (buf);
}

/* Write instruction INSN to BUF, given that it has been relocated
   by RELOC.  */

static void
write_reloc_insn (char *buf, bfd_reloc_code_real_type reloc,
		  unsigned long insn)
{
  if (mips16_reloc_p (reloc) || micromips_reloc_p (reloc))
    write_compressed_insn (buf, insn, 4);
  else
    write_insn (buf, insn);
}

/* Return TRUE if the instruction pointed to by FIXP is an invalid jump
   to a symbol in another ISA mode, which cannot be converted to JALX.  */

static bool
fix_bad_cross_mode_jump_p (fixS *fixP)
{
  unsigned long opcode;
  int other;
  char *buf;

  if (!fixP->fx_addsy || S_FORCE_RELOC (fixP->fx_addsy, true))
    return false;

  other = S_GET_OTHER (fixP->fx_addsy);
  buf = fixP->fx_frag->fr_literal + fixP->fx_where;
  opcode = read_reloc_insn (buf, fixP->fx_r_type) >> 26;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_MIPS_JMP:
      return opcode != 0x1d && opcode != 0x03 && ELF_ST_IS_COMPRESSED (other);
    case BFD_RELOC_MICROMIPS_JMP:
      return opcode != 0x3c && opcode != 0x3d && !ELF_ST_IS_MICROMIPS (other);
    default:
      return false;
    }
}

/* Return TRUE if the instruction pointed to by FIXP is an invalid JALX
   jump to a symbol in the same ISA mode.  */

static bool
fix_bad_same_mode_jalx_p (fixS *fixP)
{
  unsigned long opcode;
  int other;
  char *buf;

  if (!fixP->fx_addsy || S_FORCE_RELOC (fixP->fx_addsy, true))
    return false;

  other = S_GET_OTHER (fixP->fx_addsy);
  buf = fixP->fx_frag->fr_literal + fixP->fx_where;
  opcode = read_reloc_insn (buf, fixP->fx_r_type) >> 26;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_MIPS_JMP:
      return opcode == 0x1d && !ELF_ST_IS_COMPRESSED (other);
    case BFD_RELOC_MIPS16_JMP:
      return opcode == 0x07 && ELF_ST_IS_COMPRESSED (other);
    case BFD_RELOC_MICROMIPS_JMP:
      return opcode == 0x3c && ELF_ST_IS_COMPRESSED (other);
    default:
      return false;
    }
}

/* Return TRUE if the instruction pointed to by FIXP is an invalid jump
   to a symbol whose value plus addend is not aligned according to the
   ultimate (after linker relaxation) jump instruction's immediate field
   requirement, either to (1 << SHIFT), or, for jumps from microMIPS to
   regular MIPS code, to (1 << 2).  */

static bool
fix_bad_misaligned_jump_p (fixS *fixP, int shift)
{
  bool micro_to_mips_p;
  valueT val;
  int other;

  if (!fixP->fx_addsy || S_FORCE_RELOC (fixP->fx_addsy, true))
    return false;

  other = S_GET_OTHER (fixP->fx_addsy);
  val = S_GET_VALUE (fixP->fx_addsy) | ELF_ST_IS_COMPRESSED (other);
  val += fixP->fx_offset;
  micro_to_mips_p = (fixP->fx_r_type == BFD_RELOC_MICROMIPS_JMP
		     && !ELF_ST_IS_MICROMIPS (other));
  return ((val & ((1 << (micro_to_mips_p ? 2 : shift)) - 1))
	  != ELF_ST_IS_COMPRESSED (other));
}

/* Return TRUE if the instruction pointed to by FIXP is an invalid branch
   to a symbol whose annotation indicates another ISA mode.  For absolute
   symbols check the ISA bit instead.

   We accept BFD_RELOC_16_PCREL_S2 relocations against MIPS16 and microMIPS
   symbols or BFD_RELOC_MICROMIPS_16_PCREL_S1 relocations against regular
   MIPS symbols and associated with BAL instructions as these instructions
   may be converted to JALX by the linker.  */

static bool
fix_bad_cross_mode_branch_p (fixS *fixP)
{
  bool absolute_p;
  unsigned long opcode;
  asection *symsec;
  valueT val;
  int other;
  char *buf;

  if (mips_ignore_branch_isa)
    return false;

  if (!fixP->fx_addsy || S_FORCE_RELOC (fixP->fx_addsy, true))
    return false;

  symsec = S_GET_SEGMENT (fixP->fx_addsy);
  absolute_p = bfd_is_abs_section (symsec);

  val = S_GET_VALUE (fixP->fx_addsy) + fixP->fx_offset;
  other = S_GET_OTHER (fixP->fx_addsy);

  buf = fixP->fx_frag->fr_literal + fixP->fx_where;
  opcode = read_reloc_insn (buf, fixP->fx_r_type) >> 16;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_16_PCREL_S2:
      return ((absolute_p ? val & 1 : ELF_ST_IS_COMPRESSED (other))
	      && opcode != 0x0411);
    case BFD_RELOC_MICROMIPS_16_PCREL_S1:
      return ((absolute_p ? !(val & 1) : !ELF_ST_IS_MICROMIPS (other))
	      && opcode != 0x4060);
    case BFD_RELOC_MIPS_21_PCREL_S2:
    case BFD_RELOC_MIPS_26_PCREL_S2:
      return absolute_p ? val & 1 : ELF_ST_IS_COMPRESSED (other);
    case BFD_RELOC_MIPS16_16_PCREL_S1:
      return absolute_p ? !(val & 1) : !ELF_ST_IS_MIPS16 (other);
    case BFD_RELOC_MICROMIPS_7_PCREL_S1:
    case BFD_RELOC_MICROMIPS_10_PCREL_S1:
      return absolute_p ? !(val & 1) : !ELF_ST_IS_MICROMIPS (other);
    default:
      abort ();
    }
}

/* Return TRUE if the symbol plus addend associated with a regular MIPS
   branch instruction pointed to by FIXP is not aligned according to the
   branch instruction's immediate field requirement.  We need the addend
   to preserve the ISA bit and also the sum must not have bit 2 set.  We
   must explicitly OR in the ISA bit from symbol annotation as the bit
   won't be set in the symbol's value then.  */

static bool
fix_bad_misaligned_branch_p (fixS *fixP)
{
  bool absolute_p;
  asection *symsec;
  valueT isa_bit;
  valueT val;
  valueT off;
  int other;

  if (!fixP->fx_addsy || S_FORCE_RELOC (fixP->fx_addsy, true))
    return false;

  symsec = S_GET_SEGMENT (fixP->fx_addsy);
  absolute_p = bfd_is_abs_section (symsec);

  val = S_GET_VALUE (fixP->fx_addsy);
  other = S_GET_OTHER (fixP->fx_addsy);
  off = fixP->fx_offset;

  isa_bit = absolute_p ? (val + off) & 1 : ELF_ST_IS_COMPRESSED (other);
  val |= ELF_ST_IS_COMPRESSED (other);
  val += off;
  return (val & 0x3) != isa_bit;
}

/* Calculate the relocation target by masking off ISA mode bit before
   combining symbol and addend.  */

static valueT
fix_bad_misaligned_address (fixS *fixP)
{
  valueT val;
  valueT off;
  unsigned isa_mode;
  gas_assert (fixP != NULL && fixP->fx_addsy != NULL);
  val = S_GET_VALUE (fixP->fx_addsy);
  off = fixP->fx_offset;
  isa_mode = (ELF_ST_IS_COMPRESSED (S_GET_OTHER (fixP->fx_addsy))
	      ? 1 : 0);

  return ((val & ~isa_mode) + off);
}

/* Make the necessary checks on a regular MIPS branch pointed to by FIXP
   and its calculated value VAL.  */

static void
fix_validate_branch (fixS *fixP, valueT val)
{
  if (fixP->fx_done && (val & 0x3) != 0)
    as_bad_where (fixP->fx_file, fixP->fx_line,
		  _("branch to misaligned address (0x%lx)"),
		  (long) (val + md_pcrel_from (fixP)));
  else if (fix_bad_cross_mode_branch_p (fixP))
    as_bad_where (fixP->fx_file, fixP->fx_line,
		  _("branch to a symbol in another ISA mode"));
  else if (fix_bad_misaligned_branch_p (fixP))
    as_bad_where (fixP->fx_file, fixP->fx_line,
		  _("branch to misaligned address (0x%lx)"),
		  (long) fix_bad_misaligned_address (fixP));
  else if (HAVE_IN_PLACE_ADDENDS && (fixP->fx_offset & 0x3) != 0)
    as_bad_where (fixP->fx_file, fixP->fx_line,
		  _("cannot encode misaligned addend "
		    "in the relocatable field (0x%lx)"),
		  (long) fixP->fx_offset);
}

/* Apply a fixup to the object file.  */

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  char *buf;
  unsigned long insn;
  reloc_howto_type *howto;

  if (fixP->fx_pcrel)
    switch (fixP->fx_r_type)
      {
      case BFD_RELOC_16_PCREL_S2:
      case BFD_RELOC_MIPS16_16_PCREL_S1:
      case BFD_RELOC_MICROMIPS_7_PCREL_S1:
      case BFD_RELOC_MICROMIPS_10_PCREL_S1:
      case BFD_RELOC_MICROMIPS_16_PCREL_S1:
      case BFD_RELOC_32_PCREL:
      case BFD_RELOC_MIPS_21_PCREL_S2:
      case BFD_RELOC_MIPS_26_PCREL_S2:
      case BFD_RELOC_MIPS_18_PCREL_S3:
      case BFD_RELOC_MIPS_19_PCREL_S2:
      case BFD_RELOC_HI16_S_PCREL:
      case BFD_RELOC_LO16_PCREL:
	break;

      case BFD_RELOC_32:
	fixP->fx_r_type = BFD_RELOC_32_PCREL;
	break;

      default:
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("PC-relative reference to a different section"));
	break;
      }

  /* Handle BFD_RELOC_8 and BFD_RELOC_16.  Punt on other bfd
     relocations that have no MIPS ELF equivalent.  */
  if (fixP->fx_r_type != BFD_RELOC_8
      && fixP->fx_r_type != BFD_RELOC_16)
    {
      howto = bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);
      if (!howto)
	return;
    }

  gas_assert (fixP->fx_size == 2
	      || fixP->fx_size == 4
	      || fixP->fx_r_type == BFD_RELOC_8
	      || fixP->fx_r_type == BFD_RELOC_64
	      || fixP->fx_r_type == BFD_RELOC_CTOR
	      || fixP->fx_r_type == BFD_RELOC_MIPS_SUB
	      || fixP->fx_r_type == BFD_RELOC_MICROMIPS_SUB
	      || fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
	      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY
	      || fixP->fx_r_type == BFD_RELOC_MIPS_TLS_DTPREL64
	      || fixP->fx_r_type == BFD_RELOC_NONE);

  buf = fixP->fx_frag->fr_literal + fixP->fx_where;

  /* Don't treat parts of a composite relocation as done.  There are two
     reasons for this:

     (1) The second and third parts will be against 0 (RSS_UNDEF) but
	 should nevertheless be emitted if the first part is.

     (2) In normal usage, composite relocations are never assembly-time
	 constants.  The easiest way of dealing with the pathological
	 exceptions is to generate a relocation against STN_UNDEF and
	 leave everything up to the linker.  */
  if (fixP->fx_addsy == NULL && !fixP->fx_pcrel && fixP->fx_tcbit == 0)
    fixP->fx_done = 1;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_MIPS_TLS_GD:
    case BFD_RELOC_MIPS_TLS_LDM:
    case BFD_RELOC_MIPS_TLS_DTPREL32:
    case BFD_RELOC_MIPS_TLS_DTPREL64:
    case BFD_RELOC_MIPS_TLS_DTPREL_HI16:
    case BFD_RELOC_MIPS_TLS_DTPREL_LO16:
    case BFD_RELOC_MIPS_TLS_GOTTPREL:
    case BFD_RELOC_MIPS_TLS_TPREL32:
    case BFD_RELOC_MIPS_TLS_TPREL64:
    case BFD_RELOC_MIPS_TLS_TPREL_HI16:
    case BFD_RELOC_MIPS_TLS_TPREL_LO16:
    case BFD_RELOC_MICROMIPS_TLS_GD:
    case BFD_RELOC_MICROMIPS_TLS_LDM:
    case BFD_RELOC_MICROMIPS_TLS_DTPREL_HI16:
    case BFD_RELOC_MICROMIPS_TLS_DTPREL_LO16:
    case BFD_RELOC_MICROMIPS_TLS_GOTTPREL:
    case BFD_RELOC_MICROMIPS_TLS_TPREL_HI16:
    case BFD_RELOC_MICROMIPS_TLS_TPREL_LO16:
    case BFD_RELOC_MIPS16_TLS_GD:
    case BFD_RELOC_MIPS16_TLS_LDM:
    case BFD_RELOC_MIPS16_TLS_DTPREL_HI16:
    case BFD_RELOC_MIPS16_TLS_DTPREL_LO16:
    case BFD_RELOC_MIPS16_TLS_GOTTPREL:
    case BFD_RELOC_MIPS16_TLS_TPREL_HI16:
    case BFD_RELOC_MIPS16_TLS_TPREL_LO16:
      if (fixP->fx_addsy)
	S_SET_THREAD_LOCAL (fixP->fx_addsy);
      else
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("TLS relocation against a constant"));
      break;

    case BFD_RELOC_MIPS_JMP:
    case BFD_RELOC_MIPS16_JMP:
    case BFD_RELOC_MICROMIPS_JMP:
      {
	int shift;

	gas_assert (!fixP->fx_done);

	/* Shift is 2, unusually, for microMIPS JALX.  */
	if (fixP->fx_r_type == BFD_RELOC_MICROMIPS_JMP
	    && (read_compressed_insn (buf, 4) >> 26) != 0x3c)
	  shift = 1;
	else
	  shift = 2;

	if (fix_bad_cross_mode_jump_p (fixP))
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("jump to a symbol in another ISA mode"));
	else if (fix_bad_same_mode_jalx_p (fixP))
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("JALX to a symbol in the same ISA mode"));
	else if (fix_bad_misaligned_jump_p (fixP, shift))
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("jump to misaligned address (0x%lx)"),
			(long) fix_bad_misaligned_address (fixP));
	else if (HAVE_IN_PLACE_ADDENDS
		 && (fixP->fx_offset & ((1 << shift) - 1)) != 0)
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("cannot encode misaligned addend "
			  "in the relocatable field (0x%lx)"),
			(long) fixP->fx_offset);
      }
      /* Fall through.  */

    case BFD_RELOC_MIPS_SHIFT5:
    case BFD_RELOC_MIPS_SHIFT6:
    case BFD_RELOC_MIPS_GOT_DISP:
    case BFD_RELOC_MIPS_GOT_PAGE:
    case BFD_RELOC_MIPS_GOT_OFST:
    case BFD_RELOC_MIPS_SUB:
    case BFD_RELOC_MIPS_INSERT_A:
    case BFD_RELOC_MIPS_INSERT_B:
    case BFD_RELOC_MIPS_DELETE:
    case BFD_RELOC_MIPS_HIGHEST:
    case BFD_RELOC_MIPS_HIGHER:
    case BFD_RELOC_MIPS_SCN_DISP:
    case BFD_RELOC_MIPS_RELGOT:
    case BFD_RELOC_MIPS_JALR:
    case BFD_RELOC_HI16:
    case BFD_RELOC_HI16_S:
    case BFD_RELOC_LO16:
    case BFD_RELOC_GPREL16:
    case BFD_RELOC_MIPS_LITERAL:
    case BFD_RELOC_MIPS_CALL16:
    case BFD_RELOC_MIPS_GOT16:
    case BFD_RELOC_GPREL32:
    case BFD_RELOC_MIPS_GOT_HI16:
    case BFD_RELOC_MIPS_GOT_LO16:
    case BFD_RELOC_MIPS_CALL_HI16:
    case BFD_RELOC_MIPS_CALL_LO16:
    case BFD_RELOC_HI16_S_PCREL:
    case BFD_RELOC_LO16_PCREL:
    case BFD_RELOC_MIPS16_GPREL:
    case BFD_RELOC_MIPS16_GOT16:
    case BFD_RELOC_MIPS16_CALL16:
    case BFD_RELOC_MIPS16_HI16:
    case BFD_RELOC_MIPS16_HI16_S:
    case BFD_RELOC_MIPS16_LO16:
    case BFD_RELOC_MICROMIPS_GOT_DISP:
    case BFD_RELOC_MICROMIPS_GOT_PAGE:
    case BFD_RELOC_MICROMIPS_GOT_OFST:
    case BFD_RELOC_MICROMIPS_SUB:
    case BFD_RELOC_MICROMIPS_HIGHEST:
    case BFD_RELOC_MICROMIPS_HIGHER:
    case BFD_RELOC_MICROMIPS_SCN_DISP:
    case BFD_RELOC_MICROMIPS_JALR:
    case BFD_RELOC_MICROMIPS_HI16:
    case BFD_RELOC_MICROMIPS_HI16_S:
    case BFD_RELOC_MICROMIPS_LO16:
    case BFD_RELOC_MICROMIPS_GPREL16:
    case BFD_RELOC_MICROMIPS_LITERAL:
    case BFD_RELOC_MICROMIPS_CALL16:
    case BFD_RELOC_MICROMIPS_GOT16:
    case BFD_RELOC_MICROMIPS_GOT_HI16:
    case BFD_RELOC_MICROMIPS_GOT_LO16:
    case BFD_RELOC_MICROMIPS_CALL_HI16:
    case BFD_RELOC_MICROMIPS_CALL_LO16:
    case BFD_RELOC_MIPS_EH:
      if (fixP->fx_done)
	{
	  offsetT value;

	  if (calculate_reloc (fixP->fx_r_type, *valP, &value))
	    {
	      insn = read_reloc_insn (buf, fixP->fx_r_type);
	      if (mips16_reloc_p (fixP->fx_r_type))
		insn |= mips16_immed_extend (value, 16);
	      else
		insn |= (value & 0xffff);
	      write_reloc_insn (buf, fixP->fx_r_type, insn);
	    }
	  else
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("unsupported constant in relocation"));
	}
      break;

    case BFD_RELOC_64:
      /* This is handled like BFD_RELOC_32, but we output a sign
         extended value if we are only 32 bits.  */
      if (fixP->fx_done)
	{
	  if (8 <= sizeof (valueT))
	    md_number_to_chars (buf, *valP, 8);
	  else
	    {
	      valueT hiv;

	      if ((*valP & 0x80000000) != 0)
		hiv = 0xffffffff;
	      else
		hiv = 0;
	      md_number_to_chars (buf + (target_big_endian ? 4 : 0), *valP, 4);
	      md_number_to_chars (buf + (target_big_endian ? 0 : 4), hiv, 4);
	    }
	}
      break;

    case BFD_RELOC_RVA:
    case BFD_RELOC_32:
    case BFD_RELOC_32_PCREL:
    case BFD_RELOC_MIPS_16:
    case BFD_RELOC_16:
    case BFD_RELOC_8:
      /* If we are deleting this reloc entry, we must fill in the
	 value now.  This can happen if we have a .word which is not
	 resolved when it appears but is later defined.  */
      if (fixP->fx_done)
	md_number_to_chars (buf, *valP, fixP->fx_size);
      break;

    case BFD_RELOC_MIPS_21_PCREL_S2:
      fix_validate_branch (fixP, *valP);
      if (!fixP->fx_done)
	break;

      if (*valP + 0x400000 <= 0x7fffff)
	{
	  insn = read_insn (buf);
	  insn |= (*valP >> 2) & 0x1fffff;
	  write_insn (buf, insn);
	}
      else
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("branch out of range"));
      break;

    case BFD_RELOC_MIPS_26_PCREL_S2:
      fix_validate_branch (fixP, *valP);
      if (!fixP->fx_done)
	break;

      if (*valP + 0x8000000 <= 0xfffffff)
	{
	  insn = read_insn (buf);
	  insn |= (*valP >> 2) & 0x3ffffff;
	  write_insn (buf, insn);
	}
      else
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("branch out of range"));
      break;

    case BFD_RELOC_MIPS_18_PCREL_S3:
      if (fixP->fx_addsy && (S_GET_VALUE (fixP->fx_addsy) & 0x7) != 0)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("PC-relative access using misaligned symbol (%lx)"),
		      (long) S_GET_VALUE (fixP->fx_addsy));
      if ((fixP->fx_offset & 0x7) != 0)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("PC-relative access using misaligned offset (%lx)"),
		      (long) fixP->fx_offset);
      if (!fixP->fx_done)
	break;

      if (*valP + 0x100000 <= 0x1fffff)
	{
	  insn = read_insn (buf);
	  insn |= (*valP >> 3) & 0x3ffff;
	  write_insn (buf, insn);
	}
      else
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("PC-relative access out of range"));
      break;

    case BFD_RELOC_MIPS_19_PCREL_S2:
      if ((*valP & 0x3) != 0)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("PC-relative access to misaligned address (%lx)"),
		      (long) *valP);
      if (!fixP->fx_done)
	break;

      if (*valP + 0x100000 <= 0x1fffff)
	{
	  insn = read_insn (buf);
	  insn |= (*valP >> 2) & 0x7ffff;
	  write_insn (buf, insn);
	}
      else
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("PC-relative access out of range"));
      break;

    case BFD_RELOC_16_PCREL_S2:
      fix_validate_branch (fixP, *valP);

      /* We need to save the bits in the instruction since fixup_segment()
	 might be deleting the relocation entry (i.e., a branch within
	 the current segment).  */
      if (! fixP->fx_done)
	break;

      /* Update old instruction data.  */
      insn = read_insn (buf);

      if (*valP + 0x20000 <= 0x3ffff)
	{
	  insn |= (*valP >> 2) & 0xffff;
	  write_insn (buf, insn);
	}
      else if (fixP->fx_tcbit2
	       && fixP->fx_done
	       && fixP->fx_frag->fr_address >= text_section->vma
	       && (fixP->fx_frag->fr_address
		   < text_section->vma + bfd_section_size (text_section))
	       && ((insn & 0xffff0000) == 0x10000000	 /* beq $0,$0 */
		   || (insn & 0xffff0000) == 0x04010000	 /* bgez $0 */
		   || (insn & 0xffff0000) == 0x04110000)) /* bgezal $0 */
	{
	  /* The branch offset is too large.  If this is an
             unconditional branch, and we are not generating PIC code,
             we can convert it to an absolute jump instruction.  */
	  if ((insn & 0xffff0000) == 0x04110000)	 /* bgezal $0 */
	    insn = 0x0c000000;	/* jal */
	  else
	    insn = 0x08000000;	/* j */
	  fixP->fx_r_type = BFD_RELOC_MIPS_JMP;
	  fixP->fx_done = 0;
	  fixP->fx_addsy = section_symbol (text_section);
	  *valP += md_pcrel_from (fixP);
	  write_insn (buf, insn);
	}
      else
	{
	  /* If we got here, we have branch-relaxation disabled,
	     and there's nothing we can do to fix this instruction
	     without turning it into a longer sequence.  */
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("branch out of range"));
	}
      break;

    case BFD_RELOC_MIPS16_16_PCREL_S1:
    case BFD_RELOC_MICROMIPS_7_PCREL_S1:
    case BFD_RELOC_MICROMIPS_10_PCREL_S1:
    case BFD_RELOC_MICROMIPS_16_PCREL_S1:
      gas_assert (!fixP->fx_done);
      if (fix_bad_cross_mode_branch_p (fixP))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("branch to a symbol in another ISA mode"));
      else if (fixP->fx_addsy
	       && !S_FORCE_RELOC (fixP->fx_addsy, true)
	       && !bfd_is_abs_section (S_GET_SEGMENT (fixP->fx_addsy))
	       && (fixP->fx_offset & 0x1) != 0)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("branch to misaligned address (0x%lx)"),
		      (long) fix_bad_misaligned_address (fixP));
      else if (HAVE_IN_PLACE_ADDENDS && (fixP->fx_offset & 0x1) != 0)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("cannot encode misaligned addend "
			"in the relocatable field (0x%lx)"),
		      (long) fixP->fx_offset);
      break;

    case BFD_RELOC_VTABLE_INHERIT:
      fixP->fx_done = 0;
      if (fixP->fx_addsy
          && !S_IS_DEFINED (fixP->fx_addsy)
          && !S_IS_WEAK (fixP->fx_addsy))
        S_SET_WEAK (fixP->fx_addsy);
      break;

    case BFD_RELOC_NONE:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = 0;
      break;

    default:
      abort ();
    }

  /* Remember value for tc_gen_reloc.  */
  fixP->fx_addnumber = *valP;
}

static symbolS *
get_symbol (void)
{
  int c;
  char *name;
  symbolS *p;

  c = get_symbol_name (&name);
  p = (symbolS *) symbol_find_or_make (name);
  (void) restore_line_pointer (c);
  return p;
}

/* Align the current frag to a given power of two.  If a particular
   fill byte should be used, FILL points to an integer that contains
   that byte, otherwise FILL is null.

   This function used to have the comment:

      The MIPS assembler also automatically adjusts any preceding label.

   The implementation therefore applied the adjustment to a maximum of
   one label.  However, other label adjustments are applied to batches
   of labels, and adjusting just one caused problems when new labels
   were added for the sake of debugging or unwind information.
   We therefore adjust all preceding labels (given as LABELS) instead.  */

static void
mips_align (int to, int *fill, struct insn_label_list *labels)
{
  mips_emit_delays ();
  mips_record_compressed_mode ();
  if (fill == NULL && subseg_text_p (now_seg))
    frag_align_code (to, 0);
  else
    frag_align (to, fill ? *fill : 0, 0);
  record_alignment (now_seg, to);
  mips_move_labels (labels, subseg_text_p (now_seg));
}

/* Align to a given power of two.  .align 0 turns off the automatic
   alignment used by the data creating pseudo-ops.  */

static void
s_align (int x ATTRIBUTE_UNUSED)
{
  int temp, fill_value, *fill_ptr;
  long max_alignment = 28;

  file_mips_check_options ();

  /* o Note that the assembler pulls down any immediately preceding label
       to the aligned address.
     o It's not documented but auto alignment is reinstated by
       a .align pseudo instruction.
     o Note also that after auto alignment is turned off the mips assembler
       issues an error on attempt to assemble an improperly aligned data item.
       We don't.  */

  temp = get_absolute_expression ();
  if (temp > max_alignment)
    as_bad (_("alignment too large, %d assumed"), temp = max_alignment);
  else if (temp < 0)
    {
      as_warn (_("alignment negative, 0 assumed"));
      temp = 0;
    }
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      fill_value = get_absolute_expression ();
      fill_ptr = &fill_value;
    }
  else
    fill_ptr = 0;

  mips_mark_labels ();

  if (temp)
    {
      segment_info_type *si = seg_info (now_seg);
      struct insn_label_list *l = si->label_list;
      /* Auto alignment should be switched on by next section change.  */
      auto_align = 1;
      mips_align (temp, fill_ptr, l);
    }
  else
    {
      auto_align = 0;
    }

  demand_empty_rest_of_line ();
}

static void
s_change_sec (int sec)
{
  segT seg;

  /* The ELF backend needs to know that we are changing sections, so
     that .previous works correctly.  We could do something like check
     for an obj_section_change_hook macro, but that might be confusing
     as it would not be appropriate to use it in the section changing
     functions in read.c, since obj-elf.c intercepts those.  FIXME:
     This should be cleaner, somehow.  */
  obj_elf_section_change_hook ();

  mips_emit_delays ();

  switch (sec)
    {
    case 't':
      s_text (0);
      break;
    case 'd':
      s_data (0);
      break;
    case 'b':
      subseg_set (bss_section, (subsegT) get_absolute_expression ());
      demand_empty_rest_of_line ();
      break;

    case 'r':
      seg = subseg_new (RDATA_SECTION_NAME,
			(subsegT) get_absolute_expression ());
      bfd_set_section_flags (seg, (SEC_ALLOC | SEC_LOAD | SEC_READONLY
				   | SEC_RELOC | SEC_DATA));
      if (!startswith (TARGET_OS, "elf"))
	record_alignment (seg, 4);
      demand_empty_rest_of_line ();
      break;

    case 's':
      seg = subseg_new (".sdata", (subsegT) get_absolute_expression ());
      bfd_set_section_flags (seg, (SEC_ALLOC | SEC_LOAD | SEC_RELOC
				   | SEC_DATA | SEC_SMALL_DATA));
      if (!startswith (TARGET_OS, "elf"))
	record_alignment (seg, 4);
      demand_empty_rest_of_line ();
      break;

    case 'B':
      seg = subseg_new (".sbss", (subsegT) get_absolute_expression ());
      bfd_set_section_flags (seg, SEC_ALLOC | SEC_SMALL_DATA);
      if (!startswith (TARGET_OS, "elf"))
	record_alignment (seg, 4);
      demand_empty_rest_of_line ();
      break;
    }

  auto_align = 1;
}

void
s_change_section (int ignore ATTRIBUTE_UNUSED)
{
  char *saved_ilp;
  const char *section_name;
  char c, next_c = 0;
  int section_type;
  int section_flag;
  int section_entry_size;
  int section_alignment;

  saved_ilp = input_line_pointer;
  section_name = obj_elf_section_name ();
  if (section_name == NULL)
    return;
  c = input_line_pointer[0];
  if (c)
    next_c = input_line_pointer[1];

  /* Do we have .section Name<,"flags">?  */
  if (c != ',' || (c == ',' && next_c == '"'))
    {
      input_line_pointer = saved_ilp;
      obj_elf_section (ignore);
      return;
    }

  input_line_pointer++;

  /* Do we have .section Name<,type><,flag><,entry_size><,alignment>  */
  if (c == ',')
    section_type = get_absolute_expression ();
  else
    section_type = 0;

  if (*input_line_pointer++ == ',')
    section_flag = get_absolute_expression ();
  else
    section_flag = 0;

  if (*input_line_pointer++ == ',')
    section_entry_size = get_absolute_expression ();
  else
    section_entry_size = 0;

  if (*input_line_pointer++ == ',')
    section_alignment = get_absolute_expression ();
  else
    section_alignment = 0;

  /* FIXME: really ignore?  */
  (void) section_alignment;

  /* When using the generic form of .section (as implemented by obj-elf.c),
     there's no way to set the section type to SHT_MIPS_DWARF.  Users have
     traditionally had to fall back on the more common @progbits instead.

     There's nothing really harmful in this, since bfd will correct
     SHT_PROGBITS to SHT_MIPS_DWARF before writing out the file.  But it
     means that, for backwards compatibility, the special_section entries
     for dwarf sections must use SHT_PROGBITS rather than SHT_MIPS_DWARF.

     Even so, we shouldn't force users of the MIPS .section syntax to
     incorrectly label the sections as SHT_PROGBITS.  The best compromise
     seems to be to map SHT_MIPS_DWARF to SHT_PROGBITS before calling the
     generic type-checking code.  */
  if (section_type == SHT_MIPS_DWARF)
    section_type = SHT_PROGBITS;

  obj_elf_change_section (section_name, section_type, section_flag,
			  section_entry_size, 0, 0, 0);
}

void
mips_enable_auto_align (void)
{
  auto_align = 1;
}

static void
s_cons (int log_size)
{
  segment_info_type *si = seg_info (now_seg);
  struct insn_label_list *l = si->label_list;

  mips_emit_delays ();
  if (log_size > 0 && auto_align)
    mips_align (log_size, 0, l);
  cons (1 << log_size);
  mips_clear_insn_labels ();
}

static void
s_float_cons (int type)
{
  segment_info_type *si = seg_info (now_seg);
  struct insn_label_list *l = si->label_list;

  mips_emit_delays ();

  if (auto_align)
    {
      if (type == 'd')
	mips_align (3, 0, l);
      else
	mips_align (2, 0, l);
    }

  float_cons (type);
  mips_clear_insn_labels ();
}

/* Handle .globl.  We need to override it because on Irix 5 you are
   permitted to say
       .globl foo .text
   where foo is an undefined symbol, to mean that foo should be
   considered to be the address of a function.  */

static void
s_mips_globl (int x ATTRIBUTE_UNUSED)
{
  char *name;
  int c;
  symbolS *symbolP;

  do
    {
      c = get_symbol_name (&name);
      symbolP = symbol_find_or_make (name);
      S_SET_EXTERNAL (symbolP);

      *input_line_pointer = c;
      SKIP_WHITESPACE_AFTER_NAME ();

      if (!is_end_of_line[(unsigned char) *input_line_pointer]
	  && (*input_line_pointer != ','))
	{
	  char *secname;
	  asection *sec;

	  c = get_symbol_name (&secname);
	  sec = bfd_get_section_by_name (stdoutput, secname);
	  if (sec == NULL)
	    as_bad (_("%s: no such section"), secname);
	  (void) restore_line_pointer (c);

	  if (sec != NULL && (sec->flags & SEC_CODE) != 0)
	    symbol_get_bfdsym (symbolP)->flags |= BSF_FUNCTION;
	}

      c = *input_line_pointer;
      if (c == ',')
	{
	  input_line_pointer++;
	  SKIP_WHITESPACE ();
	  if (is_end_of_line[(unsigned char) *input_line_pointer])
	    c = '\n';
	}
    }
  while (c == ',');

  demand_empty_rest_of_line ();
}

#ifdef TE_IRIX
/* The Irix 5 and 6 assemblers set the type of any common symbol and
   any undefined non-function symbol to STT_OBJECT.  We try to be
   compatible, since newer Irix 5 and 6 linkers care.  */

void
mips_frob_symbol (symbolS *symp ATTRIBUTE_UNUSED)
{
  /* This late in assembly we can set BSF_OBJECT indiscriminately
     and let elf.c:swap_out_syms sort out the symbol type.  */
  flagword *flags = &symbol_get_bfdsym (symp)->flags;
  if ((*flags & (BSF_GLOBAL | BSF_WEAK)) != 0
      || !S_IS_DEFINED (symp))
    *flags |= BSF_OBJECT;
}
#endif

static void
s_option (int x ATTRIBUTE_UNUSED)
{
  char *opt;
  char c;

  c = get_symbol_name (&opt);

  if (*opt == 'O')
    {
      /* FIXME: What does this mean?  */
    }
  else if (startswith (opt, "pic") && ISDIGIT (opt[3]) && opt[4] == '\0')
    {
      int i;

      i = atoi (opt + 3);
      if (i != 0 && i != 2)
	as_bad (_(".option pic%d not supported"), i);
      else if (mips_pic == VXWORKS_PIC)
	as_bad (_(".option pic%d not supported in VxWorks PIC mode"), i);
      else if (i == 0)
	mips_pic = NO_PIC;
      else if (i == 2)
	{
	  mips_pic = SVR4_PIC;
	  mips_abicalls = true;
	}

      if (mips_pic == SVR4_PIC)
	{
	  if (g_switch_seen && g_switch_value != 0)
	    as_warn (_("-G may not be used with SVR4 PIC code"));
	  g_switch_value = 0;
	  bfd_set_gp_size (stdoutput, 0);
	}
    }
  else
    as_warn (_("unrecognized option \"%s\""), opt);

  (void) restore_line_pointer (c);
  demand_empty_rest_of_line ();
}

/* This structure is used to hold a stack of .set values.  */

struct mips_option_stack
{
  struct mips_option_stack *next;
  struct mips_set_options options;
};

static struct mips_option_stack *mips_opts_stack;

/* Return status for .set/.module option handling.  */

enum code_option_type
{
  /* Unrecognized option.  */
  OPTION_TYPE_BAD = -1,

  /* Ordinary option.  */
  OPTION_TYPE_NORMAL,

  /* ISA changing option.  */
  OPTION_TYPE_ISA
};

/* Handle common .set/.module options.  Return status indicating option
   type.  */

static enum code_option_type
parse_code_option (char * name)
{
  bool isa_set = false;
  const struct mips_ase *ase;

  if (startswith (name, "at="))
    {
      char *s = name + 3;

      if (!reg_lookup (&s, RTYPE_NUM | RTYPE_GP, &mips_opts.at))
	as_bad (_("unrecognized register name `%s'"), s);
    }
  else if (strcmp (name, "at") == 0)
    mips_opts.at = ATREG;
  else if (strcmp (name, "noat") == 0)
    mips_opts.at = ZERO;
  else if (strcmp (name, "move") == 0 || strcmp (name, "novolatile") == 0)
    mips_opts.nomove = 0;
  else if (strcmp (name, "nomove") == 0 || strcmp (name, "volatile") == 0)
    mips_opts.nomove = 1;
  else if (strcmp (name, "bopt") == 0)
    mips_opts.nobopt = 0;
  else if (strcmp (name, "nobopt") == 0)
    mips_opts.nobopt = 1;
  else if (strcmp (name, "gp=32") == 0)
    mips_opts.gp = 32;
  else if (strcmp (name, "gp=64") == 0)
    mips_opts.gp = 64;
  else if (strcmp (name, "fp=32") == 0)
    mips_opts.fp = 32;
  else if (strcmp (name, "fp=xx") == 0)
    mips_opts.fp = 0;
  else if (strcmp (name, "fp=64") == 0)
    mips_opts.fp = 64;
  else if (strcmp (name, "softfloat") == 0)
    mips_opts.soft_float = 1;
  else if (strcmp (name, "hardfloat") == 0)
    mips_opts.soft_float = 0;
  else if (strcmp (name, "singlefloat") == 0)
    mips_opts.single_float = 1;
  else if (strcmp (name, "doublefloat") == 0)
    mips_opts.single_float = 0;
  else if (strcmp (name, "nooddspreg") == 0)
    mips_opts.oddspreg = 0;
  else if (strcmp (name, "oddspreg") == 0)
    mips_opts.oddspreg = 1;
  else if (strcmp (name, "mips16") == 0
	   || strcmp (name, "MIPS-16") == 0)
    mips_opts.mips16 = 1;
  else if (strcmp (name, "nomips16") == 0
	   || strcmp (name, "noMIPS-16") == 0)
    mips_opts.mips16 = 0;
  else if (strcmp (name, "micromips") == 0)
    mips_opts.micromips = 1;
  else if (strcmp (name, "nomicromips") == 0)
    mips_opts.micromips = 0;
  else if (name[0] == 'n'
	   && name[1] == 'o'
	   && (ase = mips_lookup_ase (name + 2)))
    mips_set_ase (ase, &mips_opts, false);
  else if ((ase = mips_lookup_ase (name)))
    mips_set_ase (ase, &mips_opts, true);
  else if (startswith (name, "mips") || startswith (name, "arch="))
    {
      /* Permit the user to change the ISA and architecture on the fly.
	 Needless to say, misuse can cause serious problems.  */
      if (startswith (name, "arch="))
	{
	  const struct mips_cpu_info *p;

	  p = mips_parse_cpu ("internal use", name + 5);
	  if (!p)
	    as_bad (_("unknown architecture %s"), name + 5);
	  else
	    {
	      mips_opts.arch = p->cpu;
	      mips_opts.isa = p->isa;
	      isa_set = true;
	      mips_opts.init_ase = p->ase;
	    }
	}
      else if (startswith (name, "mips"))
	{
	  const struct mips_cpu_info *p;

	  p = mips_parse_cpu ("internal use", name);
	  if (!p)
	    as_bad (_("unknown ISA level %s"), name + 4);
	  else
	    {
	      mips_opts.arch = p->cpu;
	      mips_opts.isa = p->isa;
	      isa_set = true;
	      mips_opts.init_ase = p->ase;
	    }
	}
      else
	as_bad (_("unknown ISA or architecture %s"), name);
    }
  else if (strcmp (name, "autoextend") == 0)
    mips_opts.noautoextend = 0;
  else if (strcmp (name, "noautoextend") == 0)
    mips_opts.noautoextend = 1;
  else if (strcmp (name, "insn32") == 0)
    mips_opts.insn32 = true;
  else if (strcmp (name, "noinsn32") == 0)
    mips_opts.insn32 = false;
  else if (strcmp (name, "sym32") == 0)
    mips_opts.sym32 = true;
  else if (strcmp (name, "nosym32") == 0)
    mips_opts.sym32 = false;
  else
    return OPTION_TYPE_BAD;

  return isa_set ? OPTION_TYPE_ISA : OPTION_TYPE_NORMAL;
}

/* Handle the .set pseudo-op.  */

static void
s_mipsset (int x ATTRIBUTE_UNUSED)
{
  enum code_option_type type = OPTION_TYPE_NORMAL;
  char *name = input_line_pointer, ch;

  file_mips_check_options ();

  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    ++input_line_pointer;
  ch = *input_line_pointer;
  *input_line_pointer = '\0';

  if (strchr (name, ','))
    {
      /* Generic ".set" directive; use the generic handler.  */
      *input_line_pointer = ch;
      input_line_pointer = name;
      s_set (0);
      return;
    }

  if (strcmp (name, "reorder") == 0)
    {
      if (mips_opts.noreorder)
	end_noreorder ();
    }
  else if (strcmp (name, "noreorder") == 0)
    {
      if (!mips_opts.noreorder)
	start_noreorder ();
    }
  else if (strcmp (name, "macro") == 0)
    mips_opts.warn_about_macros = 0;
  else if (strcmp (name, "nomacro") == 0)
    {
      if (mips_opts.noreorder == 0)
	as_bad (_("`noreorder' must be set before `nomacro'"));
      mips_opts.warn_about_macros = 1;
    }
  else if (strcmp (name, "gp=default") == 0)
    mips_opts.gp = file_mips_opts.gp;
  else if (strcmp (name, "fp=default") == 0)
    mips_opts.fp = file_mips_opts.fp;
  else if (strcmp (name, "mips0") == 0 || strcmp (name, "arch=default") == 0)
    {
      mips_opts.isa = file_mips_opts.isa;
      mips_opts.arch = file_mips_opts.arch;
      mips_opts.init_ase = file_mips_opts.init_ase;
      mips_opts.gp = file_mips_opts.gp;
      mips_opts.fp = file_mips_opts.fp;
    }
  else if (strcmp (name, "push") == 0)
    {
      struct mips_option_stack *s;

      s = XNEW (struct mips_option_stack);
      s->next = mips_opts_stack;
      s->options = mips_opts;
      mips_opts_stack = s;
    }
  else if (strcmp (name, "pop") == 0)
    {
      struct mips_option_stack *s;

      s = mips_opts_stack;
      if (s == NULL)
	as_bad (_(".set pop with no .set push"));
      else
	{
	  /* If we're changing the reorder mode we need to handle
             delay slots correctly.  */
	  if (s->options.noreorder && ! mips_opts.noreorder)
	    start_noreorder ();
	  else if (! s->options.noreorder && mips_opts.noreorder)
	    end_noreorder ();

	  mips_opts = s->options;
	  mips_opts_stack = s->next;
	  free (s);
	}
    }
  else
    {
      type = parse_code_option (name);
      if (type == OPTION_TYPE_BAD)
	as_warn (_("tried to set unrecognized symbol: %s\n"), name);
    }

  /* The use of .set [arch|cpu]= historically 'fixes' the width of gp and fp
     registers based on what is supported by the arch/cpu.  */
  if (type == OPTION_TYPE_ISA)
    {
      switch (mips_opts.isa)
	{
	case 0:
	  break;
	case ISA_MIPS1:
	  /* MIPS I cannot support FPXX.  */
	  mips_opts.fp = 32;
	  /* fall-through.  */
	case ISA_MIPS2:
	case ISA_MIPS32:
	case ISA_MIPS32R2:
	case ISA_MIPS32R3:
	case ISA_MIPS32R5:
	  mips_opts.gp = 32;
	  if (mips_opts.fp != 0)
	    mips_opts.fp = 32;
	  break;
	case ISA_MIPS32R6:
	  mips_opts.gp = 32;
	  mips_opts.fp = 64;
	  break;
	case ISA_MIPS3:
	case ISA_MIPS4:
	case ISA_MIPS5:
	case ISA_MIPS64:
	case ISA_MIPS64R2:
	case ISA_MIPS64R3:
	case ISA_MIPS64R5:
	case ISA_MIPS64R6:
	  mips_opts.gp = 64;
	  if (mips_opts.fp != 0)
	    {
	      if (mips_opts.arch == CPU_R5900)
		mips_opts.fp = 32;
	      else
		mips_opts.fp = 64;
	    }
	  break;
	default:
	  as_bad (_("unknown ISA level %s"), name + 4);
	  break;
	}
    }

  mips_check_options (&mips_opts, false);

  mips_check_isa_supports_ases ();
  *input_line_pointer = ch;
  demand_empty_rest_of_line ();
}

/* Handle the .module pseudo-op.  */

static void
s_module (int ignore ATTRIBUTE_UNUSED)
{
  char *name = input_line_pointer, ch;

  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    ++input_line_pointer;
  ch = *input_line_pointer;
  *input_line_pointer = '\0';

  if (!file_mips_opts_checked)
    {
      if (parse_code_option (name) == OPTION_TYPE_BAD)
	as_bad (_(".module used with unrecognized symbol: %s\n"), name);

      /* Update module level settings from mips_opts.  */
      file_mips_opts = mips_opts;
    }
  else
    as_bad (_(".module is not permitted after generating code"));

  *input_line_pointer = ch;
  demand_empty_rest_of_line ();
}

/* Handle the .abicalls pseudo-op.  I believe this is equivalent to
   .option pic2.  It means to generate SVR4 PIC calls.  */

static void
s_abicalls (int ignore ATTRIBUTE_UNUSED)
{
  mips_pic = SVR4_PIC;
  mips_abicalls = true;

  if (g_switch_seen && g_switch_value != 0)
    as_warn (_("-G may not be used with SVR4 PIC code"));
  g_switch_value = 0;

  bfd_set_gp_size (stdoutput, 0);
  demand_empty_rest_of_line ();
}

/* Handle the .cpload pseudo-op.  This is used when generating SVR4
   PIC code.  It sets the $gp register for the function based on the
   function address, which is in the register named in the argument.
   This uses a relocation against _gp_disp, which is handled specially
   by the linker.  The result is:
	lui	$gp,%hi(_gp_disp)
	addiu	$gp,$gp,%lo(_gp_disp)
	addu	$gp,$gp,.cpload argument
   The .cpload argument is normally $25 == $t9.

   The -mno-shared option changes this to:
	lui	$gp,%hi(__gnu_local_gp)
	addiu	$gp,$gp,%lo(__gnu_local_gp)
   and the argument is ignored.  This saves an instruction, but the
   resulting code is not position independent; it uses an absolute
   address for __gnu_local_gp.  Thus code assembled with -mno-shared
   can go into an ordinary executable, but not into a shared library.  */

static void
s_cpload (int ignore ATTRIBUTE_UNUSED)
{
  expressionS ex;
  int reg;
  int in_shared;

  file_mips_check_options ();

  /* If we are not generating SVR4 PIC code, or if this is NewABI code,
     .cpload is ignored.  */
  if (mips_pic != SVR4_PIC || HAVE_NEWABI)
    {
      s_ignore (0);
      return;
    }

  if (mips_opts.mips16)
    {
      as_bad (_("%s not supported in MIPS16 mode"), ".cpload");
      ignore_rest_of_line ();
      return;
    }

  /* .cpload should be in a .set noreorder section.  */
  if (mips_opts.noreorder == 0)
    as_warn (_(".cpload not in noreorder section"));

  reg = tc_get_register (0);

  /* If we need to produce a 64-bit address, we are better off using
     the default instruction sequence.  */
  in_shared = mips_in_shared || HAVE_64BIT_SYMBOLS;

  ex.X_op = O_symbol;
  ex.X_add_symbol = symbol_find_or_make (in_shared ? "_gp_disp" :
                                         "__gnu_local_gp");
  ex.X_op_symbol = NULL;
  ex.X_add_number = 0;

  /* In ELF, this symbol is implicitly an STT_OBJECT symbol.  */
  symbol_get_bfdsym (ex.X_add_symbol)->flags |= BSF_OBJECT;

  mips_mark_labels ();
  mips_assembling_insn = true;

  macro_start ();
  macro_build_lui (&ex, mips_gp_register);
  macro_build (&ex, "addiu", "t,r,j", mips_gp_register,
	       mips_gp_register, BFD_RELOC_LO16);
  if (in_shared)
    macro_build (NULL, "addu", "d,v,t", mips_gp_register,
		 mips_gp_register, reg);
  macro_end ();

  mips_assembling_insn = false;
  demand_empty_rest_of_line ();
}

/* Handle the .cpsetup pseudo-op defined for NewABI PIC code.  The syntax is:
     .cpsetup $reg1, offset|$reg2, label

   If offset is given, this results in:
     sd		$gp, offset($sp)
     lui	$gp, %hi(%neg(%gp_rel(label)))
     addiu	$gp, $gp, %lo(%neg(%gp_rel(label)))
     daddu	$gp, $gp, $reg1

   If $reg2 is given, this results in:
     or		$reg2, $gp, $0
     lui	$gp, %hi(%neg(%gp_rel(label)))
     addiu	$gp, $gp, %lo(%neg(%gp_rel(label)))
     daddu	$gp, $gp, $reg1
   $reg1 is normally $25 == $t9.

   The -mno-shared option replaces the last three instructions with
	lui	$gp,%hi(_gp)
	addiu	$gp,$gp,%lo(_gp)  */

static void
s_cpsetup (int ignore ATTRIBUTE_UNUSED)
{
  expressionS ex_off;
  expressionS ex_sym;
  int reg1;

  file_mips_check_options ();

  /* If we are not generating SVR4 PIC code, .cpsetup is ignored.
     We also need NewABI support.  */
  if (mips_pic != SVR4_PIC || ! HAVE_NEWABI)
    {
      s_ignore (0);
      return;
    }

  if (mips_opts.mips16)
    {
      as_bad (_("%s not supported in MIPS16 mode"), ".cpsetup");
      ignore_rest_of_line ();
      return;
    }

  reg1 = tc_get_register (0);
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad (_("missing argument separator ',' for .cpsetup"));
      return;
    }
  else
    ++input_line_pointer;
  SKIP_WHITESPACE ();
  if (*input_line_pointer == '$')
    {
      mips_cpreturn_register = tc_get_register (0);
      mips_cpreturn_offset = -1;
    }
  else
    {
      mips_cpreturn_offset = get_absolute_expression ();
      mips_cpreturn_register = -1;
    }
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad (_("missing argument separator ',' for .cpsetup"));
      return;
    }
  else
    ++input_line_pointer;
  SKIP_WHITESPACE ();
  expression (&ex_sym);

  mips_mark_labels ();
  mips_assembling_insn = true;

  macro_start ();
  if (mips_cpreturn_register == -1)
    {
      ex_off.X_op = O_constant;
      ex_off.X_add_symbol = NULL;
      ex_off.X_op_symbol = NULL;
      ex_off.X_add_number = mips_cpreturn_offset;

      macro_build (&ex_off, "sd", "t,o(b)", mips_gp_register,
		   BFD_RELOC_LO16, SP);
    }
  else
    move_register (mips_cpreturn_register, mips_gp_register);

  if (mips_in_shared || HAVE_64BIT_SYMBOLS)
    {
      macro_build (&ex_sym, "lui", LUI_FMT, mips_gp_register,
		   -1, BFD_RELOC_GPREL16, BFD_RELOC_MIPS_SUB,
		   BFD_RELOC_HI16_S);

      macro_build (&ex_sym, "addiu", "t,r,j", mips_gp_register,
		   mips_gp_register, -1, BFD_RELOC_GPREL16,
		   BFD_RELOC_MIPS_SUB, BFD_RELOC_LO16);

      macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", mips_gp_register,
		   mips_gp_register, reg1);
    }
  else
    {
      expressionS ex;

      ex.X_op = O_symbol;
      ex.X_add_symbol = symbol_find_or_make ("__gnu_local_gp");
      ex.X_op_symbol = NULL;
      ex.X_add_number = 0;

      /* In ELF, this symbol is implicitly an STT_OBJECT symbol.  */
      symbol_get_bfdsym (ex.X_add_symbol)->flags |= BSF_OBJECT;

      macro_build_lui (&ex, mips_gp_register);
      macro_build (&ex, "addiu", "t,r,j", mips_gp_register,
		   mips_gp_register, BFD_RELOC_LO16);
    }

  macro_end ();

  mips_assembling_insn = false;
  demand_empty_rest_of_line ();
}

static void
s_cplocal (int ignore ATTRIBUTE_UNUSED)
{
  file_mips_check_options ();

  /* If we are not generating SVR4 PIC code, or if this is not NewABI code,
     .cplocal is ignored.  */
  if (mips_pic != SVR4_PIC || ! HAVE_NEWABI)
    {
      s_ignore (0);
      return;
    }

  if (mips_opts.mips16)
    {
      as_bad (_("%s not supported in MIPS16 mode"), ".cplocal");
      ignore_rest_of_line ();
      return;
    }

  mips_gp_register = tc_get_register (0);
  demand_empty_rest_of_line ();
}

/* Handle the .cprestore pseudo-op.  This stores $gp into a given
   offset from $sp.  The offset is remembered, and after making a PIC
   call $gp is restored from that location.  */

static void
s_cprestore (int ignore ATTRIBUTE_UNUSED)
{
  expressionS ex;

  file_mips_check_options ();

  /* If we are not generating SVR4 PIC code, or if this is NewABI code,
     .cprestore is ignored.  */
  if (mips_pic != SVR4_PIC || HAVE_NEWABI)
    {
      s_ignore (0);
      return;
    }

  if (mips_opts.mips16)
    {
      as_bad (_("%s not supported in MIPS16 mode"), ".cprestore");
      ignore_rest_of_line ();
      return;
    }

  mips_cprestore_offset = get_absolute_expression ();
  mips_cprestore_valid = 1;

  ex.X_op = O_constant;
  ex.X_add_symbol = NULL;
  ex.X_op_symbol = NULL;
  ex.X_add_number = mips_cprestore_offset;

  mips_mark_labels ();
  mips_assembling_insn = true;

  macro_start ();
  macro_build_ldst_constoffset (&ex, ADDRESS_STORE_INSN, mips_gp_register,
				SP, HAVE_64BIT_ADDRESSES);
  macro_end ();

  mips_assembling_insn = false;
  demand_empty_rest_of_line ();
}

/* Handle the .cpreturn pseudo-op defined for NewABI PIC code. If an offset
   was given in the preceding .cpsetup, it results in:
     ld		$gp, offset($sp)

   If a register $reg2 was given there, it results in:
     or		$gp, $reg2, $0  */

static void
s_cpreturn (int ignore ATTRIBUTE_UNUSED)
{
  expressionS ex;

  file_mips_check_options ();

  /* If we are not generating SVR4 PIC code, .cpreturn is ignored.
     We also need NewABI support.  */
  if (mips_pic != SVR4_PIC || ! HAVE_NEWABI)
    {
      s_ignore (0);
      return;
    }

  if (mips_opts.mips16)
    {
      as_bad (_("%s not supported in MIPS16 mode"), ".cpreturn");
      ignore_rest_of_line ();
      return;
    }

  mips_mark_labels ();
  mips_assembling_insn = true;

  macro_start ();
  if (mips_cpreturn_register == -1)
    {
      ex.X_op = O_constant;
      ex.X_add_symbol = NULL;
      ex.X_op_symbol = NULL;
      ex.X_add_number = mips_cpreturn_offset;

      macro_build (&ex, "ld", "t,o(b)", mips_gp_register, BFD_RELOC_LO16, SP);
    }
  else
    move_register (mips_gp_register, mips_cpreturn_register);

  macro_end ();

  mips_assembling_insn = false;
  demand_empty_rest_of_line ();
}

/* Handle a .dtprelword, .dtpreldword, .tprelword, or .tpreldword
   pseudo-op; DIRSTR says which. The pseudo-op generates a BYTES-size
   DTP- or TP-relative relocation of type RTYPE, for use in either DWARF
   debug information or MIPS16 TLS.  */

static void
s_tls_rel_directive (const size_t bytes, const char *dirstr,
		     bfd_reloc_code_real_type rtype)
{
  expressionS ex;
  char *p;

  expression (&ex);

  if (ex.X_op != O_symbol)
    {
      as_bad (_("unsupported use of %s"), dirstr);
      ignore_rest_of_line ();
    }

  p = frag_more (bytes);
  md_number_to_chars (p, 0, bytes);
  fix_new_exp (frag_now, p - frag_now->fr_literal, bytes, &ex, false, rtype);
  demand_empty_rest_of_line ();
  mips_clear_insn_labels ();
}

/* Handle .dtprelword.  */

static void
s_dtprelword (int ignore ATTRIBUTE_UNUSED)
{
  s_tls_rel_directive (4, ".dtprelword", BFD_RELOC_MIPS_TLS_DTPREL32);
}

/* Handle .dtpreldword.  */

static void
s_dtpreldword (int ignore ATTRIBUTE_UNUSED)
{
  s_tls_rel_directive (8, ".dtpreldword", BFD_RELOC_MIPS_TLS_DTPREL64);
}

/* Handle .tprelword.  */

static void
s_tprelword (int ignore ATTRIBUTE_UNUSED)
{
  s_tls_rel_directive (4, ".tprelword", BFD_RELOC_MIPS_TLS_TPREL32);
}

/* Handle .tpreldword.  */

static void
s_tpreldword (int ignore ATTRIBUTE_UNUSED)
{
  s_tls_rel_directive (8, ".tpreldword", BFD_RELOC_MIPS_TLS_TPREL64);
}

/* Handle the .gpvalue pseudo-op.  This is used when generating NewABI PIC
   code.  It sets the offset to use in gp_rel relocations.  */

static void
s_gpvalue (int ignore ATTRIBUTE_UNUSED)
{
  /* If we are not generating SVR4 PIC code, .gpvalue is ignored.
     We also need NewABI support.  */
  if (mips_pic != SVR4_PIC || ! HAVE_NEWABI)
    {
      s_ignore (0);
      return;
    }

  mips_gprel_offset = get_absolute_expression ();

  demand_empty_rest_of_line ();
}

/* Handle the .gpword pseudo-op.  This is used when generating PIC
   code.  It generates a 32 bit GP relative reloc.  */

static void
s_gpword (int ignore ATTRIBUTE_UNUSED)
{
  segment_info_type *si;
  struct insn_label_list *l;
  expressionS ex;
  char *p;

  /* When not generating PIC code, this is treated as .word.  */
  if (mips_pic != SVR4_PIC)
    {
      s_cons (2);
      return;
    }

  si = seg_info (now_seg);
  l = si->label_list;
  mips_emit_delays ();
  if (auto_align)
    mips_align (2, 0, l);

  expression (&ex);
  mips_clear_insn_labels ();

  if (ex.X_op != O_symbol || ex.X_add_number != 0)
    {
      as_bad (_("unsupported use of .gpword"));
      ignore_rest_of_line ();
    }

  p = frag_more (4);
  md_number_to_chars (p, 0, 4);
  fix_new_exp (frag_now, p - frag_now->fr_literal, 4, &ex, false,
	       BFD_RELOC_GPREL32);

  demand_empty_rest_of_line ();
}

static void
s_gpdword (int ignore ATTRIBUTE_UNUSED)
{
  segment_info_type *si;
  struct insn_label_list *l;
  expressionS ex;
  char *p;

  /* When not generating PIC code, this is treated as .dword.  */
  if (mips_pic != SVR4_PIC)
    {
      s_cons (3);
      return;
    }

  si = seg_info (now_seg);
  l = si->label_list;
  mips_emit_delays ();
  if (auto_align)
    mips_align (3, 0, l);

  expression (&ex);
  mips_clear_insn_labels ();

  if (ex.X_op != O_symbol || ex.X_add_number != 0)
    {
      as_bad (_("unsupported use of .gpdword"));
      ignore_rest_of_line ();
    }

  p = frag_more (8);
  md_number_to_chars (p, 0, 8);
  fix_new_exp (frag_now, p - frag_now->fr_literal, 4, &ex, false,
	       BFD_RELOC_GPREL32)->fx_tcbit = 1;

  /* GPREL32 composed with 64 gives a 64-bit GP offset.  */
  fix_new (frag_now, p - frag_now->fr_literal, 8, NULL, 0,
	   false, BFD_RELOC_64)->fx_tcbit = 1;

  demand_empty_rest_of_line ();
}

/* Handle the .ehword pseudo-op.  This is used when generating unwinding
   tables.  It generates a R_MIPS_EH reloc.  */

static void
s_ehword (int ignore ATTRIBUTE_UNUSED)
{
  expressionS ex;
  char *p;

  mips_emit_delays ();

  expression (&ex);
  mips_clear_insn_labels ();

  if (ex.X_op != O_symbol || ex.X_add_number != 0)
    {
      as_bad (_("unsupported use of .ehword"));
      ignore_rest_of_line ();
    }

  p = frag_more (4);
  md_number_to_chars (p, 0, 4);
  fix_new_exp (frag_now, p - frag_now->fr_literal, 4, &ex, false,
	       BFD_RELOC_32_PCREL);

  demand_empty_rest_of_line ();
}

/* Handle the .cpadd pseudo-op.  This is used when dealing with switch
   tables in SVR4 PIC code.  */

static void
s_cpadd (int ignore ATTRIBUTE_UNUSED)
{
  int reg;

  file_mips_check_options ();

  /* This is ignored when not generating SVR4 PIC code.  */
  if (mips_pic != SVR4_PIC)
    {
      s_ignore (0);
      return;
    }

  mips_mark_labels ();
  mips_assembling_insn = true;

  /* Add $gp to the register named as an argument.  */
  macro_start ();
  reg = tc_get_register (0);
  macro_build (NULL, ADDRESS_ADD_INSN, "d,v,t", reg, reg, mips_gp_register);
  macro_end ();

  mips_assembling_insn = false;
  demand_empty_rest_of_line ();
}

/* Handle the .insn pseudo-op.  This marks instruction labels in
   mips16/micromips mode.  This permits the linker to handle them specially,
   such as generating jalx instructions when needed.  We also make
   them odd for the duration of the assembly, in order to generate the
   right sort of code.  We will make them even in the adjust_symtab
   routine, while leaving them marked.  This is convenient for the
   debugger and the disassembler.  The linker knows to make them odd
   again.  */

static void
s_insn (int ignore ATTRIBUTE_UNUSED)
{
  file_mips_check_options ();
  file_ase_mips16 |= mips_opts.mips16;
  file_ase_micromips |= mips_opts.micromips;

  mips_mark_labels ();

  demand_empty_rest_of_line ();
}

/* Handle the .nan pseudo-op.  */

static void
s_nan (int ignore ATTRIBUTE_UNUSED)
{
  static const char str_legacy[] = "legacy";
  static const char str_2008[] = "2008";
  size_t i;

  for (i = 0; !is_end_of_line[(unsigned char) input_line_pointer[i]]; i++);

  if (i == sizeof (str_2008) - 1
      && memcmp (input_line_pointer, str_2008, i) == 0)
    mips_nan2008 = 1;
  else if (i == sizeof (str_legacy) - 1
	   && memcmp (input_line_pointer, str_legacy, i) == 0)
    {
      if (ISA_HAS_LEGACY_NAN (file_mips_opts.isa))
	mips_nan2008 = 0;
      else
	as_bad (_("`%s' does not support legacy NaN"),
	          mips_cpu_info_from_isa (file_mips_opts.isa)->name);
    }
  else
    as_bad (_("bad .nan directive"));

  input_line_pointer += i;
  demand_empty_rest_of_line ();
}

/* Handle a .stab[snd] directive.  Ideally these directives would be
   implemented in a transparent way, so that removing them would not
   have any effect on the generated instructions.  However, s_stab
   internally changes the section, so in practice we need to decide
   now whether the preceding label marks compressed code.  We do not
   support changing the compression mode of a label after a .stab*
   directive, such as in:

   foo:
	.stabs ...
	.set mips16

   so the current mode wins.  */

static void
s_mips_stab (int type)
{
  file_mips_check_options ();
  mips_mark_labels ();
  s_stab (type);
}

/* Handle the .weakext pseudo-op as defined in Kane and Heinrich.  */

static void
s_mips_weakext (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  int c;
  symbolS *symbolP;
  expressionS exp;

  c = get_symbol_name (&name);
  symbolP = symbol_find_or_make (name);
  S_SET_WEAK (symbolP);
  *input_line_pointer = c;

  SKIP_WHITESPACE_AFTER_NAME ();

  if (! is_end_of_line[(unsigned char) *input_line_pointer])
    {
      if (S_IS_DEFINED (symbolP))
	{
	  as_bad (_("ignoring attempt to redefine symbol %s"),
		  S_GET_NAME (symbolP));
	  ignore_rest_of_line ();
	  return;
	}

      if (*input_line_pointer == ',')
	{
	  ++input_line_pointer;
	  SKIP_WHITESPACE ();
	}

      expression (&exp);
      if (exp.X_op != O_symbol)
	{
	  as_bad (_("bad .weakext directive"));
	  ignore_rest_of_line ();
	  return;
	}
      symbol_set_value_expression (symbolP, &exp);
    }

  demand_empty_rest_of_line ();
}

/* Parse a register string into a number.  Called from the ECOFF code
   to parse .frame.  The argument is non-zero if this is the frame
   register, so that we can record it in mips_frame_reg.  */

int
tc_get_register (int frame)
{
  unsigned int reg;

  SKIP_WHITESPACE ();
  if (! reg_lookup (&input_line_pointer, RWARN | RTYPE_NUM | RTYPE_GP, &reg))
    reg = 0;
  if (frame)
    {
      mips_frame_reg = reg != 0 ? reg : SP;
      mips_frame_reg_valid = 1;
      mips_cprestore_valid = 0;
    }
  return reg;
}

valueT
md_section_align (asection *seg, valueT addr)
{
  int align = bfd_section_alignment (seg);

  /* We don't need to align ELF sections to the full alignment.
     However, Irix 5 may prefer that we align them at least to a 16
     byte boundary.  We don't bother to align the sections if we
     are targeted for an embedded system.  */
  if (startswith (TARGET_OS, "elf"))
    return addr;
  if (align > 4)
    align = 4;

  return ((addr + (1 << align) - 1) & -(1 << align));
}

/* Utility routine, called from above as well.  If called while the
   input file is still being read, it's only an approximation.  (For
   example, a symbol may later become defined which appeared to be
   undefined earlier.)  */

static int
nopic_need_relax (symbolS *sym, int before_relaxing)
{
  if (sym == 0)
    return 0;

  if (g_switch_value > 0)
    {
      const char *symname;
      int change;

      /* Find out whether this symbol can be referenced off the $gp
	 register.  It can be if it is smaller than the -G size or if
	 it is in the .sdata or .sbss section.  Certain symbols can
	 not be referenced off the $gp, although it appears as though
	 they can.  */
      symname = S_GET_NAME (sym);
      if (symname != (const char *) NULL
	  && (strcmp (symname, "eprol") == 0
	      || strcmp (symname, "etext") == 0
	      || strcmp (symname, "_gp") == 0
	      || strcmp (symname, "edata") == 0
	      || strcmp (symname, "_fbss") == 0
	      || strcmp (symname, "_fdata") == 0
	      || strcmp (symname, "_ftext") == 0
	      || strcmp (symname, "end") == 0
	      || strcmp (symname, "_gp_disp") == 0))
	change = 1;
      else if ((! S_IS_DEFINED (sym) || S_IS_COMMON (sym))
	       && (0
#ifndef NO_ECOFF_DEBUGGING
		   || (symbol_get_obj (sym)->ecoff_extern_size != 0
		       && (symbol_get_obj (sym)->ecoff_extern_size
			   <= g_switch_value))
#endif
		   /* We must defer this decision until after the whole
		      file has been read, since there might be a .extern
		      after the first use of this symbol.  */
		   || (before_relaxing
#ifndef NO_ECOFF_DEBUGGING
		       && symbol_get_obj (sym)->ecoff_extern_size == 0
#endif
		       && S_GET_VALUE (sym) == 0)
		   || (S_GET_VALUE (sym) != 0
		       && S_GET_VALUE (sym) <= g_switch_value)))
	change = 0;
      else
	{
	  const char *segname;

	  segname = segment_name (S_GET_SEGMENT (sym));
	  gas_assert (strcmp (segname, ".lit8") != 0
		  && strcmp (segname, ".lit4") != 0);
	  change = (strcmp (segname, ".sdata") != 0
		    && strcmp (segname, ".sbss") != 0
		    && !startswith (segname, ".sdata.")
		    && !startswith (segname, ".sbss.")
		    && !startswith (segname, ".gnu.linkonce.sb.")
		    && !startswith (segname, ".gnu.linkonce.s."));
	}
      return change;
    }
  else
    /* We are not optimizing for the $gp register.  */
    return 1;
}


/* Return true if the given symbol should be considered local for SVR4 PIC.  */

static bool
pic_need_relax (symbolS *sym)
{
  asection *symsec;

  if (!sym)
    return false;

  /* Handle the case of a symbol equated to another symbol.  */
  while (symbol_equated_reloc_p (sym))
    {
      symbolS *n;

      /* It's possible to get a loop here in a badly written program.  */
      n = symbol_get_value_expression (sym)->X_add_symbol;
      if (n == sym)
	break;
      sym = n;
    }

  if (symbol_section_p (sym))
    return true;

  symsec = S_GET_SEGMENT (sym);

  /* This must duplicate the test in adjust_reloc_syms.  */
  return (!bfd_is_und_section (symsec)
	  && !bfd_is_abs_section (symsec)
	  && !bfd_is_com_section (symsec)
	  /* A global or weak symbol is treated as external.  */
	  && (!S_IS_WEAK (sym) && !S_IS_EXTERNAL (sym)));
}

/* Given a MIPS16 variant frag FRAGP and PC-relative operand PCREL_OP
   convert a section-relative value VAL to the equivalent PC-relative
   value.  */

static offsetT
mips16_pcrel_val (fragS *fragp, const struct mips_pcrel_operand *pcrel_op,
		  offsetT val, long stretch)
{
  fragS *sym_frag;
  addressT addr;

  gas_assert (pcrel_op->root.root.type == OP_PCREL);

  sym_frag = symbol_get_frag (fragp->fr_symbol);

  /* If the relax_marker of the symbol fragment differs from the
     relax_marker of this fragment, we have not yet adjusted the
     symbol fragment fr_address.  We want to add in STRETCH in
     order to get a better estimate of the address.  This
     particularly matters because of the shift bits.  */
  if (stretch != 0 && sym_frag->relax_marker != fragp->relax_marker)
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
		stretch = -(-stretch & ~((1 << (int) f->fr_offset) - 1));
	      else
		stretch &= ~((1 << (int) f->fr_offset) - 1);
	      if (stretch == 0)
		break;
	    }
	}
      if (f != NULL)
	val += stretch;
    }

  addr = fragp->fr_address + fragp->fr_fix;

  /* The base address rules are complicated.  The base address of
     a branch is the following instruction.  The base address of a
     PC relative load or add is the instruction itself, but if it
     is in a delay slot (in which case it can not be extended) use
     the address of the instruction whose delay slot it is in.  */
  if (pcrel_op->include_isa_bit)
    {
      addr += 2;

      /* If we are currently assuming that this frag should be
	 extended, then the current address is two bytes higher.  */
      if (RELAX_MIPS16_EXTENDED (fragp->fr_subtype))
	addr += 2;

      /* Ignore the low bit in the target, since it will be set
	 for a text label.  */
      val &= -2;
    }
  else if (RELAX_MIPS16_JAL_DSLOT (fragp->fr_subtype))
    addr -= 4;
  else if (RELAX_MIPS16_DSLOT (fragp->fr_subtype))
    addr -= 2;

  val -= addr & -(1 << pcrel_op->align_log2);

  return val;
}

/* Given a mips16 variant frag FRAGP, return non-zero if it needs an
   extended opcode.  SEC is the section the frag is in.  */

static int
mips16_extended_frag (fragS *fragp, asection *sec, long stretch)
{
  const struct mips_int_operand *operand;
  offsetT val;
  segT symsec;
  int type;

  if (RELAX_MIPS16_USER_SMALL (fragp->fr_subtype))
    return 0;
  if (RELAX_MIPS16_USER_EXT (fragp->fr_subtype))
    return 1;

  symsec = S_GET_SEGMENT (fragp->fr_symbol);
  type = RELAX_MIPS16_TYPE (fragp->fr_subtype);
  operand = mips16_immed_operand (type, false);
  if (S_FORCE_RELOC (fragp->fr_symbol, true)
      || (operand->root.type == OP_PCREL
	  ? sec != symsec
	  : !bfd_is_abs_section (symsec)))
    return 1;

  val = S_GET_VALUE (fragp->fr_symbol) + fragp->fr_offset;

  if (operand->root.type == OP_PCREL)
    {
      const struct mips_pcrel_operand *pcrel_op;
      offsetT maxtiny;

      if (RELAX_MIPS16_ALWAYS_EXTENDED (fragp->fr_subtype))
	return 1;

      pcrel_op = (const struct mips_pcrel_operand *) operand;
      val = mips16_pcrel_val (fragp, pcrel_op, val, stretch);

      /* If any of the shifted bits are set, we must use an extended
         opcode.  If the address depends on the size of this
         instruction, this can lead to a loop, so we arrange to always
         use an extended opcode.  */
      if ((val & ((1 << operand->shift) - 1)) != 0)
	{
	  fragp->fr_subtype =
	    RELAX_MIPS16_MARK_ALWAYS_EXTENDED (fragp->fr_subtype);
	  return 1;
	}

      /* If we are about to mark a frag as extended because the value
         is precisely the next value above maxtiny, then there is a
         chance of an infinite loop as in the following code:
	     la	$4,foo
	     .skip	1020
	     .align	2
	   foo:
	 In this case when the la is extended, foo is 0x3fc bytes
	 away, so the la can be shrunk, but then foo is 0x400 away, so
	 the la must be extended.  To avoid this loop, we mark the
	 frag as extended if it was small, and is about to become
	 extended with the next value above maxtiny.  */
      maxtiny = mips_int_operand_max (operand);
      if (val == maxtiny + (1 << operand->shift)
	  && ! RELAX_MIPS16_EXTENDED (fragp->fr_subtype))
	{
	  fragp->fr_subtype =
	    RELAX_MIPS16_MARK_ALWAYS_EXTENDED (fragp->fr_subtype);
	  return 1;
	}
    }

  return !mips16_immed_in_range_p (operand, BFD_RELOC_UNUSED, val);
}

/* Given a MIPS16 variant frag FRAGP, return non-zero if it needs
   macro expansion.  SEC is the section the frag is in.  We only
   support PC-relative instructions (LA, DLA, LW, LD) here, in
   non-PIC code using 32-bit addressing.  */

static int
mips16_macro_frag (fragS *fragp, asection *sec, long stretch)
{
  const struct mips_pcrel_operand *pcrel_op;
  const struct mips_int_operand *operand;
  offsetT val;
  segT symsec;
  int type;

  gas_assert (!RELAX_MIPS16_USER_SMALL (fragp->fr_subtype));

  if (RELAX_MIPS16_USER_EXT (fragp->fr_subtype))
    return 0;
  if (!RELAX_MIPS16_SYM32 (fragp->fr_subtype))
    return 0;

  type = RELAX_MIPS16_TYPE (fragp->fr_subtype);
  switch (type)
    {
    case 'A':
    case 'B':
    case 'E':
      symsec = S_GET_SEGMENT (fragp->fr_symbol);
      if (bfd_is_abs_section (symsec))
	return 1;
      if (RELAX_MIPS16_PIC (fragp->fr_subtype))
	return 0;
      if (S_FORCE_RELOC (fragp->fr_symbol, true) || sec != symsec)
	return 1;

      operand = mips16_immed_operand (type, true);
      val = S_GET_VALUE (fragp->fr_symbol) + fragp->fr_offset;
      pcrel_op = (const struct mips_pcrel_operand *) operand;
      val = mips16_pcrel_val (fragp, pcrel_op, val, stretch);

      return !mips16_immed_in_range_p (operand, BFD_RELOC_UNUSED, val);

    default:
      return 0;
    }
}

/* Compute the length of a branch sequence, and adjust the
   RELAX_BRANCH_TOOFAR bit accordingly.  If FRAGP is NULL, the
   worst-case length is computed, with UPDATE being used to indicate
   whether an unconditional (-1), branch-likely (+1) or regular (0)
   branch is to be computed.  */
static int
relaxed_branch_length (fragS *fragp, asection *sec, int update)
{
  bool toofar;
  int length;

  if (fragp
      && S_IS_DEFINED (fragp->fr_symbol)
      && !S_IS_WEAK (fragp->fr_symbol)
      && sec == S_GET_SEGMENT (fragp->fr_symbol))
    {
      addressT addr;
      offsetT val;

      val = S_GET_VALUE (fragp->fr_symbol) + fragp->fr_offset;

      addr = fragp->fr_address + fragp->fr_fix + 4;

      val -= addr;

      toofar = val < - (0x8000 << 2) || val >= (0x8000 << 2);
    }
  else
    /* If the symbol is not defined or it's in a different segment,
       we emit the long sequence.  */
    toofar = true;

  if (fragp && update && toofar != RELAX_BRANCH_TOOFAR (fragp->fr_subtype))
    fragp->fr_subtype
      = RELAX_BRANCH_ENCODE (RELAX_BRANCH_AT (fragp->fr_subtype),
			     RELAX_BRANCH_PIC (fragp->fr_subtype),
			     RELAX_BRANCH_UNCOND (fragp->fr_subtype),
			     RELAX_BRANCH_LIKELY (fragp->fr_subtype),
			     RELAX_BRANCH_LINK (fragp->fr_subtype),
			     toofar);

  length = 4;
  if (toofar)
    {
      if (fragp ? RELAX_BRANCH_LIKELY (fragp->fr_subtype) : (update > 0))
	length += 8;

      if (!fragp || RELAX_BRANCH_PIC (fragp->fr_subtype))
	{
	  /* Additional space for PIC loading of target address.  */
	  length += 8;
	  if (mips_opts.isa == ISA_MIPS1)
	    /* Additional space for $at-stabilizing nop.  */
	    length += 4;
	}

      /* If branch is conditional.  */
      if (fragp ? !RELAX_BRANCH_UNCOND (fragp->fr_subtype) : (update >= 0))
	length += 8;
    }

  return length;
}

/* Get a FRAG's branch instruction delay slot size, either from the
   short-delay-slot bit of a branch-and-link instruction if AL is TRUE,
   or SHORT_INSN_SIZE otherwise.  */

static int
frag_branch_delay_slot_size (fragS *fragp, bool al, int short_insn_size)
{
  char *buf = fragp->fr_literal + fragp->fr_fix;

  if (al)
    return (read_compressed_insn (buf, 4) & 0x02000000) ? 2 : 4;
  else
    return short_insn_size;
}

/* Compute the length of a branch sequence, and adjust the
   RELAX_MICROMIPS_TOOFAR32 bit accordingly.  If FRAGP is NULL, the
   worst-case length is computed, with UPDATE being used to indicate
   whether an unconditional (-1), or regular (0) branch is to be
   computed.  */

static int
relaxed_micromips_32bit_branch_length (fragS *fragp, asection *sec, int update)
{
  bool insn32 = true;
  bool nods = true;
  bool pic = true;
  bool al = true;
  int short_insn_size;
  bool toofar;
  int length;

  if (fragp)
    {
      insn32 = RELAX_MICROMIPS_INSN32 (fragp->fr_subtype);
      nods = RELAX_MICROMIPS_NODS (fragp->fr_subtype);
      pic = RELAX_MICROMIPS_PIC (fragp->fr_subtype);
      al = RELAX_MICROMIPS_LINK (fragp->fr_subtype);
    }
  short_insn_size = insn32 ? 4 : 2;

  if (fragp
      && S_IS_DEFINED (fragp->fr_symbol)
      && !S_IS_WEAK (fragp->fr_symbol)
      && sec == S_GET_SEGMENT (fragp->fr_symbol))
    {
      addressT addr;
      offsetT val;

      val = S_GET_VALUE (fragp->fr_symbol) + fragp->fr_offset;
      /* Ignore the low bit in the target, since it will be set
	 for a text label.  */
      if ((val & 1) != 0)
	--val;

      addr = fragp->fr_address + fragp->fr_fix + 4;

      val -= addr;

      toofar = val < - (0x8000 << 1) || val >= (0x8000 << 1);
    }
  else
    /* If the symbol is not defined or it's in a different segment,
       we emit the long sequence.  */
    toofar = true;

  if (fragp && update
      && toofar != RELAX_MICROMIPS_TOOFAR32 (fragp->fr_subtype))
    fragp->fr_subtype = (toofar
			 ? RELAX_MICROMIPS_MARK_TOOFAR32 (fragp->fr_subtype)
			 : RELAX_MICROMIPS_CLEAR_TOOFAR32 (fragp->fr_subtype));

  length = 4;
  if (toofar)
    {
      bool compact_known = fragp != NULL;
      bool compact = false;
      bool uncond;

      if (fragp)
	{
	  compact = RELAX_MICROMIPS_COMPACT (fragp->fr_subtype);
	  uncond = RELAX_MICROMIPS_UNCOND (fragp->fr_subtype);
	}
      else
	uncond = update < 0;

      /* If label is out of range, we turn branch <br>:

		<br>	label			# 4 bytes
	    0:

         into:

		j	label			# 4 bytes
		nop				# 2/4 bytes if
						#  compact && (!PIC || insn32)
	    0:
       */
      if ((!pic || insn32) && (!compact_known || compact))
	length += short_insn_size;

      /* If assembling PIC code, we further turn:

			j	label			# 4 bytes

         into:

			lw/ld	at, %got(label)(gp)	# 4 bytes
			d/addiu	at, %lo(label)		# 4 bytes
			jr/c	at			# 2/4 bytes
       */
      if (pic)
	length += 4 + short_insn_size;

      /* Add an extra nop if the jump has no compact form and we need
         to fill the delay slot.  */
      if ((!pic || al) && nods)
	length += (fragp
		   ? frag_branch_delay_slot_size (fragp, al, short_insn_size)
		   : short_insn_size);

      /* If branch <br> is conditional, we prepend negated branch <brneg>:

			<brneg>	0f			# 4 bytes
			nop				# 2/4 bytes if !compact
       */
      if (!uncond)
	length += (compact_known && compact) ? 4 : 4 + short_insn_size;
    }
  else if (nods)
    {
      /* Add an extra nop to fill the delay slot.  */
      gas_assert (fragp);
      length += frag_branch_delay_slot_size (fragp, al, short_insn_size);
    }

  return length;
}

/* Compute the length of a branch, and adjust the RELAX_MICROMIPS_TOOFAR16
   bit accordingly.  */

static int
relaxed_micromips_16bit_branch_length (fragS *fragp, asection *sec, int update)
{
  bool toofar;

  if (fragp
      && S_IS_DEFINED (fragp->fr_symbol)
      && !S_IS_WEAK (fragp->fr_symbol)
      && sec == S_GET_SEGMENT (fragp->fr_symbol))
    {
      addressT addr;
      offsetT val;
      int type;

      val = S_GET_VALUE (fragp->fr_symbol) + fragp->fr_offset;
      /* Ignore the low bit in the target, since it will be set
	 for a text label.  */
      if ((val & 1) != 0)
	--val;

      /* Assume this is a 2-byte branch.  */
      addr = fragp->fr_address + fragp->fr_fix + 2;

      /* We try to avoid the infinite loop by not adding 2 more bytes for
	 long branches.  */

      val -= addr;

      type = RELAX_MICROMIPS_TYPE (fragp->fr_subtype);
      if (type == 'D')
	toofar = val < - (0x200 << 1) || val >= (0x200 << 1);
      else if (type == 'E')
	toofar = val < - (0x40 << 1) || val >= (0x40 << 1);
      else
	abort ();
    }
  else
    /* If the symbol is not defined or it's in a different segment,
       we emit a normal 32-bit branch.  */
    toofar = true;

  if (fragp && update
      && toofar != RELAX_MICROMIPS_TOOFAR16 (fragp->fr_subtype))
    fragp->fr_subtype
      = toofar ? RELAX_MICROMIPS_MARK_TOOFAR16 (fragp->fr_subtype)
	       : RELAX_MICROMIPS_CLEAR_TOOFAR16 (fragp->fr_subtype);

  if (toofar)
    return 4;

  return 2;
}

/* Estimate the size of a frag before relaxing.  Unless this is the
   mips16, we are not really relaxing here, and the final size is
   encoded in the subtype information.  For the mips16, we have to
   decide whether we are using an extended opcode or not.  */

int
md_estimate_size_before_relax (fragS *fragp, asection *segtype)
{
  int change;

  if (RELAX_BRANCH_P (fragp->fr_subtype))
    {

      fragp->fr_var = relaxed_branch_length (fragp, segtype, false);

      return fragp->fr_var;
    }

  if (RELAX_MIPS16_P (fragp->fr_subtype))
    {
      /* We don't want to modify the EXTENDED bit here; it might get us
	 into infinite loops.  We change it only in mips_relax_frag().  */
      if (RELAX_MIPS16_MACRO (fragp->fr_subtype))
	return RELAX_MIPS16_E2 (fragp->fr_subtype) ? 8 : 12;
      else
	return RELAX_MIPS16_EXTENDED (fragp->fr_subtype) ? 4 : 2;
    }

  if (RELAX_MICROMIPS_P (fragp->fr_subtype))
    {
      int length = 4;

      if (RELAX_MICROMIPS_TYPE (fragp->fr_subtype) != 0)
	length = relaxed_micromips_16bit_branch_length (fragp, segtype, false);
      if (length == 4 && RELAX_MICROMIPS_RELAX32 (fragp->fr_subtype))
	length = relaxed_micromips_32bit_branch_length (fragp, segtype, false);
      fragp->fr_var = length;

      return length;
    }

  if (mips_pic == VXWORKS_PIC)
    /* For vxworks, GOT16 relocations never have a corresponding LO16.  */
    change = 0;
  else if (RELAX_PIC (fragp->fr_subtype))
    change = pic_need_relax (fragp->fr_symbol);
  else
    change = nopic_need_relax (fragp->fr_symbol, 0);

  if (change)
    {
      fragp->fr_subtype |= RELAX_USE_SECOND;
      return -RELAX_FIRST (fragp->fr_subtype);
    }
  else
    return -RELAX_SECOND (fragp->fr_subtype);
}

/* This is called to see whether a reloc against a defined symbol
   should be converted into a reloc against a section.  */

int
mips_fix_adjustable (fixS *fixp)
{
  if (fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return 0;

  if (fixp->fx_addsy == NULL)
    return 1;

  /* Allow relocs used for EH tables.  */
  if (fixp->fx_r_type == BFD_RELOC_32_PCREL)
    return 1;

  /* If symbol SYM is in a mergeable section, relocations of the form
     SYM + 0 can usually be made section-relative.  The mergeable data
     is then identified by the section offset rather than by the symbol.

     However, if we're generating REL LO16 relocations, the offset is split
     between the LO16 and partnering high part relocation.  The linker will
     need to recalculate the complete offset in order to correctly identify
     the merge data.

     The linker has traditionally not looked for the partnering high part
     relocation, and has thus allowed orphaned R_MIPS_LO16 relocations to be
     placed anywhere.  Rather than break backwards compatibility by changing
     this, it seems better not to force the issue, and instead keep the
     original symbol.  This will work with either linker behavior.  */
  if ((lo16_reloc_p (fixp->fx_r_type)
       || reloc_needs_lo_p (fixp->fx_r_type))
      && HAVE_IN_PLACE_ADDENDS
      && (S_GET_SEGMENT (fixp->fx_addsy)->flags & SEC_MERGE) != 0)
    return 0;

  /* There is no place to store an in-place offset for JALR relocations.  */
  if (jalr_reloc_p (fixp->fx_r_type) && HAVE_IN_PLACE_ADDENDS)
    return 0;

  /* Likewise an in-range offset of limited PC-relative relocations may
     overflow the in-place relocatable field if recalculated against the
     start address of the symbol's containing section.

     Also, PC relative relocations for MIPS R6 need to be symbol rather than
     section relative to allow linker relaxations to be performed later on.  */
  if (limited_pcrel_reloc_p (fixp->fx_r_type)
      && (HAVE_IN_PLACE_ADDENDS || ISA_IS_R6 (file_mips_opts.isa)))
    return 0;

  /* R_MIPS16_26 relocations against non-MIPS16 functions might resolve
     to a floating-point stub.  The same is true for non-R_MIPS16_26
     relocations against MIPS16 functions; in this case, the stub becomes
     the function's canonical address.

     Floating-point stubs are stored in unique .mips16.call.* or
     .mips16.fn.* sections.  If a stub T for function F is in section S,
     the first relocation in section S must be against F; this is how the
     linker determines the target function.  All relocations that might
     resolve to T must also be against F.  We therefore have the following
     restrictions, which are given in an intentionally-redundant way:

       1. We cannot reduce R_MIPS16_26 relocations against non-MIPS16
	  symbols.

       2. We cannot reduce a stub's relocations against non-MIPS16 symbols
	  if that stub might be used.

       3. We cannot reduce non-R_MIPS16_26 relocations against MIPS16
	  symbols.

       4. We cannot reduce a stub's relocations against MIPS16 symbols if
	  that stub might be used.

     There is a further restriction:

       5. We cannot reduce jump relocations (R_MIPS_26, R_MIPS16_26 or
	  R_MICROMIPS_26_S1) or branch relocations (R_MIPS_PC26_S2,
	  R_MIPS_PC21_S2, R_MIPS_PC16, R_MIPS16_PC16_S1,
	  R_MICROMIPS_PC16_S1, R_MICROMIPS_PC10_S1 or R_MICROMIPS_PC7_S1)
	  against MIPS16 or microMIPS symbols because we need to keep the
	  MIPS16 or microMIPS symbol for the purpose of mode mismatch
	  detection and JAL or BAL to JALX instruction conversion in the
	  linker.

     For simplicity, we deal with (3)-(4) by not reducing _any_ relocation
     against a MIPS16 symbol.  We deal with (5) by additionally leaving
     alone any jump and branch relocations against a microMIPS symbol.

     We deal with (1)-(2) by saying that, if there's a R_MIPS16_26
     relocation against some symbol R, no relocation against R may be
     reduced.  (Note that this deals with (2) as well as (1) because
     relocations against global symbols will never be reduced on ELF
     targets.)  This approach is a little simpler than trying to detect
     stub sections, and gives the "all or nothing" per-symbol consistency
     that we have for MIPS16 symbols.  */
  if (fixp->fx_subsy == NULL
      && (ELF_ST_IS_MIPS16 (S_GET_OTHER (fixp->fx_addsy))
	  || (ELF_ST_IS_MICROMIPS (S_GET_OTHER (fixp->fx_addsy))
	      && (jmp_reloc_p (fixp->fx_r_type)
		  || b_reloc_p (fixp->fx_r_type)))
	  || *symbol_get_tc (fixp->fx_addsy)))
    return 0;

  return 1;
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent **
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  static arelent *retval[4];
  arelent *reloc;
  bfd_reloc_code_real_type code;

  memset (retval, 0, sizeof(retval));
  reloc = retval[0] = XCNEW (arelent);
  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  if (fixp->fx_pcrel)
    {
      gas_assert (fixp->fx_r_type == BFD_RELOC_16_PCREL_S2
		  || fixp->fx_r_type == BFD_RELOC_MIPS16_16_PCREL_S1
		  || fixp->fx_r_type == BFD_RELOC_MICROMIPS_7_PCREL_S1
		  || fixp->fx_r_type == BFD_RELOC_MICROMIPS_10_PCREL_S1
		  || fixp->fx_r_type == BFD_RELOC_MICROMIPS_16_PCREL_S1
		  || fixp->fx_r_type == BFD_RELOC_32_PCREL
		  || fixp->fx_r_type == BFD_RELOC_MIPS_21_PCREL_S2
		  || fixp->fx_r_type == BFD_RELOC_MIPS_26_PCREL_S2
		  || fixp->fx_r_type == BFD_RELOC_MIPS_18_PCREL_S3
		  || fixp->fx_r_type == BFD_RELOC_MIPS_19_PCREL_S2
		  || fixp->fx_r_type == BFD_RELOC_HI16_S_PCREL
		  || fixp->fx_r_type == BFD_RELOC_LO16_PCREL);

      /* At this point, fx_addnumber is "symbol offset - pcrel address".
	 Relocations want only the symbol offset.  */
      switch (fixp->fx_r_type)
	{
	case BFD_RELOC_MIPS_18_PCREL_S3:
	  reloc->addend = fixp->fx_addnumber + (reloc->address & ~7);
	  break;
	default:
	  reloc->addend = fixp->fx_addnumber + reloc->address;
	  break;
	}
    }
  else if (HAVE_IN_PLACE_ADDENDS
	   && fixp->fx_r_type == BFD_RELOC_MICROMIPS_JMP
	   && (read_compressed_insn (fixp->fx_frag->fr_literal
				     + fixp->fx_where, 4) >> 26) == 0x3c)
    {
      /* Shift is 2, unusually, for microMIPS JALX.  Adjust the in-place
         addend accordingly.  */
      reloc->addend = fixp->fx_addnumber >> 1;
    }
  else
    reloc->addend = fixp->fx_addnumber;

  /* Since the old MIPS ELF ABI uses Rel instead of Rela, encode the vtable
     entry to be used in the relocation's section offset.  */
  if (! HAVE_NEWABI && fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    {
      reloc->address = reloc->addend;
      reloc->addend = 0;
    }

  code = fixp->fx_r_type;

  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("cannot represent %s relocation in this object file"
		      " format"),
		    bfd_get_reloc_code_name (code));
      retval[0] = NULL;
    }

  return retval;
}

/* Relax a machine dependent frag.  This returns the amount by which
   the current size of the frag should change.  */

int
mips_relax_frag (asection *sec, fragS *fragp, long stretch)
{
  if (RELAX_BRANCH_P (fragp->fr_subtype))
    {
      offsetT old_var = fragp->fr_var;

      fragp->fr_var = relaxed_branch_length (fragp, sec, true);

      return fragp->fr_var - old_var;
    }

  if (RELAX_MICROMIPS_P (fragp->fr_subtype))
    {
      offsetT old_var = fragp->fr_var;
      offsetT new_var = 4;

      if (RELAX_MICROMIPS_TYPE (fragp->fr_subtype) != 0)
	new_var = relaxed_micromips_16bit_branch_length (fragp, sec, true);
      if (new_var == 4 && RELAX_MICROMIPS_RELAX32 (fragp->fr_subtype))
	new_var = relaxed_micromips_32bit_branch_length (fragp, sec, true);
      fragp->fr_var = new_var;

      return new_var - old_var;
    }

  if (! RELAX_MIPS16_P (fragp->fr_subtype))
    return 0;

  if (!mips16_extended_frag (fragp, sec, stretch))
    {
      if (RELAX_MIPS16_MACRO (fragp->fr_subtype))
	{
	  fragp->fr_subtype = RELAX_MIPS16_CLEAR_MACRO (fragp->fr_subtype);
	  return RELAX_MIPS16_E2 (fragp->fr_subtype) ? -6 : -10;
	}
      else if (RELAX_MIPS16_EXTENDED (fragp->fr_subtype))
	{
	  fragp->fr_subtype = RELAX_MIPS16_CLEAR_EXTENDED (fragp->fr_subtype);
	  return -2;
	}
      else
	return 0;
    }
  else if (!mips16_macro_frag (fragp, sec, stretch))
    {
      if (RELAX_MIPS16_MACRO (fragp->fr_subtype))
	{
	  fragp->fr_subtype = RELAX_MIPS16_CLEAR_MACRO (fragp->fr_subtype);
	  fragp->fr_subtype = RELAX_MIPS16_MARK_EXTENDED (fragp->fr_subtype);
	  return RELAX_MIPS16_E2 (fragp->fr_subtype) ? -4 : -8;
	}
      else if (!RELAX_MIPS16_EXTENDED (fragp->fr_subtype))
	{
	  fragp->fr_subtype = RELAX_MIPS16_MARK_EXTENDED (fragp->fr_subtype);
	  return 2;
	}
      else
	return 0;
    }
  else
    {
      if (RELAX_MIPS16_MACRO (fragp->fr_subtype))
	return 0;
      else if (RELAX_MIPS16_EXTENDED (fragp->fr_subtype))
	{
	  fragp->fr_subtype = RELAX_MIPS16_CLEAR_EXTENDED (fragp->fr_subtype);
	  fragp->fr_subtype = RELAX_MIPS16_MARK_MACRO (fragp->fr_subtype);
	  return RELAX_MIPS16_E2 (fragp->fr_subtype) ? 4 : 8;
	}
      else
	{
	  fragp->fr_subtype = RELAX_MIPS16_MARK_MACRO (fragp->fr_subtype);
	  return RELAX_MIPS16_E2 (fragp->fr_subtype) ? 6 : 10;
	}
    }

  return 0;
}

/* Convert a machine dependent frag.  */

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, segT asec, fragS *fragp)
{
  if (RELAX_BRANCH_P (fragp->fr_subtype))
    {
      char *buf;
      unsigned long insn;
      fixS *fixp;

      buf = fragp->fr_literal + fragp->fr_fix;
      insn = read_insn (buf);

      if (!RELAX_BRANCH_TOOFAR (fragp->fr_subtype))
	{
	  /* We generate a fixup instead of applying it right now
	     because, if there are linker relaxations, we're going to
	     need the relocations.  */
	  fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			  fragp->fr_symbol, fragp->fr_offset,
			  true, BFD_RELOC_16_PCREL_S2);
	  fixp->fx_file = fragp->fr_file;
	  fixp->fx_line = fragp->fr_line;

	  buf = write_insn (buf, insn);
	}
      else
	{
	  int i;

	  as_warn_where (fragp->fr_file, fragp->fr_line,
			 _("relaxed out-of-range branch into a jump"));

	  if (RELAX_BRANCH_UNCOND (fragp->fr_subtype))
	    goto uncond;

	  if (!RELAX_BRANCH_LIKELY (fragp->fr_subtype))
	    {
	      /* Reverse the branch.  */
	      switch ((insn >> 28) & 0xf)
		{
		case 4:
		  if ((insn & 0xff000000) == 0x47000000
		      || (insn & 0xff600000) == 0x45600000)
		    {
		      /* BZ.df/BNZ.df, BZ.V/BNZ.V can have the condition
			 reversed by tweaking bit 23.  */
		      insn ^= 0x00800000;
		    }
		  else
		    {
		      /* bc[0-3][tf]l? instructions can have the condition
			 reversed by tweaking a single TF bit, and their
			 opcodes all have 0x4???????.  */
		      gas_assert ((insn & 0xf3e00000) == 0x41000000);
		      insn ^= 0x00010000;
		    }
		  break;

		case 0:
		  /* bltz	0x04000000	bgez	0x04010000
		     bltzal	0x04100000	bgezal	0x04110000  */
		  gas_assert ((insn & 0xfc0e0000) == 0x04000000);
		  insn ^= 0x00010000;
		  break;

		case 1:
		  /* beq	0x10000000	bne	0x14000000
		     blez	0x18000000	bgtz	0x1c000000  */
		  insn ^= 0x04000000;
		  break;

		default:
		  abort ();
		}
	    }

	  if (RELAX_BRANCH_LINK (fragp->fr_subtype))
	    {
	      /* Clear the and-link bit.  */
	      gas_assert ((insn & 0xfc1c0000) == 0x04100000);

	      /* bltzal		0x04100000	bgezal	0x04110000
		 bltzall	0x04120000	bgezall	0x04130000  */
	      insn &= ~0x00100000;
	    }

	  /* Branch over the branch (if the branch was likely) or the
	     full jump (not likely case).  Compute the offset from the
	     current instruction to branch to.  */
	  if (RELAX_BRANCH_LIKELY (fragp->fr_subtype))
	    i = 16;
	  else
	    {
	      /* How many bytes in instructions we've already emitted?  */
	      i = buf - fragp->fr_literal - fragp->fr_fix;
	      /* How many bytes in instructions from here to the end?  */
	      i = fragp->fr_var - i;
	    }
	  /* Convert to instruction count.  */
	  i >>= 2;
	  /* Branch counts from the next instruction.  */
	  i--;
	  insn |= i;
	  /* Branch over the jump.  */
	  buf = write_insn (buf, insn);

	  /* nop */
	  buf = write_insn (buf, 0);

	  if (RELAX_BRANCH_LIKELY (fragp->fr_subtype))
	    {
	      /* beql $0, $0, 2f */
	      insn = 0x50000000;
	      /* Compute the PC offset from the current instruction to
		 the end of the variable frag.  */
	      /* How many bytes in instructions we've already emitted?  */
	      i = buf - fragp->fr_literal - fragp->fr_fix;
	      /* How many bytes in instructions from here to the end?  */
	      i = fragp->fr_var - i;
	      /* Convert to instruction count.  */
	      i >>= 2;
	      /* Don't decrement i, because we want to branch over the
		 delay slot.  */
	      insn |= i;

	      buf = write_insn (buf, insn);
	      buf = write_insn (buf, 0);
	    }

	uncond:
	  if (!RELAX_BRANCH_PIC (fragp->fr_subtype))
	    {
	      /* j or jal.  */
	      insn = (RELAX_BRANCH_LINK (fragp->fr_subtype)
		      ? 0x0c000000 : 0x08000000);

	      fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			      fragp->fr_symbol, fragp->fr_offset,
			      false, BFD_RELOC_MIPS_JMP);
	      fixp->fx_file = fragp->fr_file;
	      fixp->fx_line = fragp->fr_line;

	      buf = write_insn (buf, insn);
	    }
	  else
	    {
	      unsigned long at = RELAX_BRANCH_AT (fragp->fr_subtype);

	      /* lw/ld $at, <sym>($gp)  R_MIPS_GOT16 */
	      insn = HAVE_64BIT_ADDRESSES ? 0xdf800000 : 0x8f800000;
	      insn |= at << OP_SH_RT;

	      fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			      fragp->fr_symbol, fragp->fr_offset,
			      false, BFD_RELOC_MIPS_GOT16);
	      fixp->fx_file = fragp->fr_file;
	      fixp->fx_line = fragp->fr_line;

	      buf = write_insn (buf, insn);

	      if (mips_opts.isa == ISA_MIPS1)
		/* nop */
		buf = write_insn (buf, 0);

	      /* d/addiu $at, $at, <sym>  R_MIPS_LO16 */
	      insn = HAVE_64BIT_ADDRESSES ? 0x64000000 : 0x24000000;
	      insn |= at << OP_SH_RS | at << OP_SH_RT;

	      fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			      fragp->fr_symbol, fragp->fr_offset,
			      false, BFD_RELOC_LO16);
	      fixp->fx_file = fragp->fr_file;
	      fixp->fx_line = fragp->fr_line;

	      buf = write_insn (buf, insn);

	      /* j(al)r $at.  */
	      if (RELAX_BRANCH_LINK (fragp->fr_subtype))
		insn = 0x0000f809;
	      else
		insn = 0x00000008;
	      insn |= at << OP_SH_RS;

	      buf = write_insn (buf, insn);
	    }
	}

      fragp->fr_fix += fragp->fr_var;
      gas_assert (buf == fragp->fr_literal + fragp->fr_fix);
      return;
    }

  /* Relax microMIPS branches.  */
  if (RELAX_MICROMIPS_P (fragp->fr_subtype))
    {
      char *buf = fragp->fr_literal + fragp->fr_fix;
      bool compact = RELAX_MICROMIPS_COMPACT (fragp->fr_subtype);
      bool insn32 = RELAX_MICROMIPS_INSN32 (fragp->fr_subtype);
      bool nods = RELAX_MICROMIPS_NODS (fragp->fr_subtype);
      bool pic = RELAX_MICROMIPS_PIC (fragp->fr_subtype);
      bool al = RELAX_MICROMIPS_LINK (fragp->fr_subtype);
      int type = RELAX_MICROMIPS_TYPE (fragp->fr_subtype);
      bool short_ds;
      unsigned long insn;
      fixS *fixp;

      fragp->fr_fix += fragp->fr_var;

      /* Handle 16-bit branches that fit or are forced to fit.  */
      if (type != 0 && !RELAX_MICROMIPS_TOOFAR16 (fragp->fr_subtype))
	{
	  /* We generate a fixup instead of applying it right now,
	     because if there is linker relaxation, we're going to
	     need the relocations.  */
	  switch (type)
	    {
	    case 'D':
	      fixp = fix_new (fragp, buf - fragp->fr_literal, 2,
			      fragp->fr_symbol, fragp->fr_offset,
			      true, BFD_RELOC_MICROMIPS_10_PCREL_S1);
	      break;
	    case 'E':
	      fixp = fix_new (fragp, buf - fragp->fr_literal, 2,
			      fragp->fr_symbol, fragp->fr_offset,
			      true, BFD_RELOC_MICROMIPS_7_PCREL_S1);
	      break;
	    default:
	      abort ();
	    }

	  fixp->fx_file = fragp->fr_file;
	  fixp->fx_line = fragp->fr_line;

	  /* These relocations can have an addend that won't fit in
	     2 octets.  */
	  fixp->fx_no_overflow = 1;

	  return;
	}

      /* Handle 32-bit branches that fit or are forced to fit.  */
      if (!RELAX_MICROMIPS_RELAX32 (fragp->fr_subtype)
	  || !RELAX_MICROMIPS_TOOFAR32 (fragp->fr_subtype))
	{
	  /* We generate a fixup instead of applying it right now,
	     because if there is linker relaxation, we're going to
	     need the relocations.  */
	  fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			  fragp->fr_symbol, fragp->fr_offset,
			  true, BFD_RELOC_MICROMIPS_16_PCREL_S1);
	  fixp->fx_file = fragp->fr_file;
	  fixp->fx_line = fragp->fr_line;

	  if (type == 0)
	    {
	      insn = read_compressed_insn (buf, 4);
	      buf += 4;

	      if (nods)
		{
		  /* Check the short-delay-slot bit.  */
		  if (!al || (insn & 0x02000000) != 0)
		    buf = write_compressed_insn (buf, 0x0c00, 2);
		  else
		    buf = write_compressed_insn (buf, 0x00000000, 4);
		}

	      gas_assert (buf == fragp->fr_literal + fragp->fr_fix);
	      return;
	    }
	}

      /* Relax 16-bit branches to 32-bit branches.  */
      if (type != 0)
	{
	  insn = read_compressed_insn (buf, 2);

	  if ((insn & 0xfc00) == 0xcc00)		/* b16  */
	    insn = 0x94000000;				/* beq  */
	  else if ((insn & 0xdc00) == 0x8c00)		/* beqz16/bnez16  */
	    {
	      unsigned long regno;

	      regno = (insn >> MICROMIPSOP_SH_MD) & MICROMIPSOP_MASK_MD;
	      regno = micromips_to_32_reg_d_map [regno];
	      insn = ((insn & 0x2000) << 16) | 0x94000000;	/* beq/bne  */
	      insn |= regno << MICROMIPSOP_SH_RS;
	    }
	  else
	    abort ();

	  /* Nothing else to do, just write it out.  */
	  if (!RELAX_MICROMIPS_RELAX32 (fragp->fr_subtype)
	      || !RELAX_MICROMIPS_TOOFAR32 (fragp->fr_subtype))
	    {
	      buf = write_compressed_insn (buf, insn, 4);
	      if (nods)
		buf = write_compressed_insn (buf, 0x0c00, 2);
	      gas_assert (buf == fragp->fr_literal + fragp->fr_fix);
	      return;
	    }
	}
      else
	insn = read_compressed_insn (buf, 4);

      /* Relax 32-bit branches to a sequence of instructions.  */
      as_warn_where (fragp->fr_file, fragp->fr_line,
		     _("relaxed out-of-range branch into a jump"));

      /* Set the short-delay-slot bit.  */
      short_ds = !al || (insn & 0x02000000) != 0;

      if (!RELAX_MICROMIPS_UNCOND (fragp->fr_subtype))
	{
	  symbolS *l;

	  /* Reverse the branch.  */
	  if ((insn & 0xfc000000) == 0x94000000			/* beq  */
	      || (insn & 0xfc000000) == 0xb4000000)		/* bne  */
	    insn ^= 0x20000000;
	  else if ((insn & 0xffe00000) == 0x40000000		/* bltz  */
		   || (insn & 0xffe00000) == 0x40400000		/* bgez  */
		   || (insn & 0xffe00000) == 0x40800000		/* blez  */
		   || (insn & 0xffe00000) == 0x40c00000		/* bgtz  */
		   || (insn & 0xffe00000) == 0x40a00000		/* bnezc  */
		   || (insn & 0xffe00000) == 0x40e00000		/* beqzc  */
		   || (insn & 0xffe00000) == 0x40200000		/* bltzal  */
		   || (insn & 0xffe00000) == 0x40600000		/* bgezal  */
		   || (insn & 0xffe00000) == 0x42200000		/* bltzals  */
		   || (insn & 0xffe00000) == 0x42600000)	/* bgezals  */
	    insn ^= 0x00400000;
	  else if ((insn & 0xffe30000) == 0x43800000		/* bc1f  */
		   || (insn & 0xffe30000) == 0x43a00000		/* bc1t  */
		   || (insn & 0xffe30000) == 0x42800000		/* bc2f  */
		   || (insn & 0xffe30000) == 0x42a00000)	/* bc2t  */
	    insn ^= 0x00200000;
	  else if ((insn & 0xff000000) == 0x83000000		/* BZ.df
								   BNZ.df  */
		    || (insn & 0xff600000) == 0x81600000)	/* BZ.V
								   BNZ.V */
	    insn ^= 0x00800000;
	  else
	    abort ();

	  if (al)
	    {
	      /* Clear the and-link and short-delay-slot bits.  */
	      gas_assert ((insn & 0xfda00000) == 0x40200000);

	      /* bltzal  0x40200000	bgezal  0x40600000  */
	      /* bltzals 0x42200000	bgezals 0x42600000  */
	      insn &= ~0x02200000;
	    }

	  /* Make a label at the end for use with the branch.  */
	  l = symbol_new (micromips_label_name (), asec, fragp, fragp->fr_fix);
	  micromips_label_inc ();
	  S_SET_OTHER (l, ELF_ST_SET_MICROMIPS (S_GET_OTHER (l)));

	  /* Refer to it.  */
	  fixp = fix_new (fragp, buf - fragp->fr_literal, 4, l, 0, true,
			  BFD_RELOC_MICROMIPS_16_PCREL_S1);
	  fixp->fx_file = fragp->fr_file;
	  fixp->fx_line = fragp->fr_line;

	  /* Branch over the jump.  */
	  buf = write_compressed_insn (buf, insn, 4);

	  if (!compact)
	    {
	      /* nop  */
	      if (insn32)
		buf = write_compressed_insn (buf, 0x00000000, 4);
	      else
		buf = write_compressed_insn (buf, 0x0c00, 2);
	    }
	}

      if (!pic)
	{
	  unsigned long jal = (short_ds || nods
			       ? 0x74000000 : 0xf4000000);	/* jal/s  */

	  /* j/jal/jals <sym>  R_MICROMIPS_26_S1  */
	  insn = al ? jal : 0xd4000000;

	  fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			  fragp->fr_symbol, fragp->fr_offset,
			  false, BFD_RELOC_MICROMIPS_JMP);
	  fixp->fx_file = fragp->fr_file;
	  fixp->fx_line = fragp->fr_line;

	  buf = write_compressed_insn (buf, insn, 4);

	  if (compact || nods)
	    {
	      /* nop  */
	      if (insn32)
		buf = write_compressed_insn (buf, 0x00000000, 4);
	      else
		buf = write_compressed_insn (buf, 0x0c00, 2);
	    }
	}
      else
	{
	  unsigned long at = RELAX_MICROMIPS_AT (fragp->fr_subtype);

	  /* lw/ld $at, <sym>($gp)  R_MICROMIPS_GOT16  */
	  insn = HAVE_64BIT_ADDRESSES ? 0xdc1c0000 : 0xfc1c0000;
	  insn |= at << MICROMIPSOP_SH_RT;

	  fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			  fragp->fr_symbol, fragp->fr_offset,
			  false, BFD_RELOC_MICROMIPS_GOT16);
	  fixp->fx_file = fragp->fr_file;
	  fixp->fx_line = fragp->fr_line;

	  buf = write_compressed_insn (buf, insn, 4);

	  /* d/addiu $at, $at, <sym>  R_MICROMIPS_LO16  */
	  insn = HAVE_64BIT_ADDRESSES ? 0x5c000000 : 0x30000000;
	  insn |= at << MICROMIPSOP_SH_RT | at << MICROMIPSOP_SH_RS;

	  fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			  fragp->fr_symbol, fragp->fr_offset,
			  false, BFD_RELOC_MICROMIPS_LO16);
	  fixp->fx_file = fragp->fr_file;
	  fixp->fx_line = fragp->fr_line;

	  buf = write_compressed_insn (buf, insn, 4);

	  if (insn32)
	    {
	      /* jr/jalr $at  */
	      insn = 0x00000f3c | (al ? RA : ZERO) << MICROMIPSOP_SH_RT;
	      insn |= at << MICROMIPSOP_SH_RS;

	      buf = write_compressed_insn (buf, insn, 4);

	      if (compact || nods)
		/* nop  */
		buf = write_compressed_insn (buf, 0x00000000, 4);
	    }
	  else
	    {
	      /* jr/jrc/jalr/jalrs $at  */
	      unsigned long jalr = short_ds ? 0x45e0 : 0x45c0;	/* jalr/s  */
	      unsigned long jr = compact || nods ? 0x45a0 : 0x4580; /* jr/c  */

	      insn = al ? jalr : jr;
	      insn |= at << MICROMIPSOP_SH_MJ;

	      buf = write_compressed_insn (buf, insn, 2);
	      if (al && nods)
		{
		  /* nop  */
		  if (short_ds)
		    buf = write_compressed_insn (buf, 0x0c00, 2);
		  else
		    buf = write_compressed_insn (buf, 0x00000000, 4);
		}
	    }
	}

      gas_assert (buf == fragp->fr_literal + fragp->fr_fix);
      return;
    }

  if (RELAX_MIPS16_P (fragp->fr_subtype))
    {
      int type;
      const struct mips_int_operand *operand;
      offsetT val;
      char *buf;
      unsigned int user_length;
      bool need_reloc;
      unsigned long insn;
      bool mac;
      bool ext;
      segT symsec;

      type = RELAX_MIPS16_TYPE (fragp->fr_subtype);
      operand = mips16_immed_operand (type, false);

      mac = RELAX_MIPS16_MACRO (fragp->fr_subtype);
      ext = RELAX_MIPS16_EXTENDED (fragp->fr_subtype);
      val = resolve_symbol_value (fragp->fr_symbol) + fragp->fr_offset;

      symsec = S_GET_SEGMENT (fragp->fr_symbol);
      need_reloc = (S_FORCE_RELOC (fragp->fr_symbol, true)
		    || (operand->root.type == OP_PCREL && !mac
			? asec != symsec
			: !bfd_is_abs_section (symsec)));

      if (operand->root.type == OP_PCREL && !mac)
	{
	  const struct mips_pcrel_operand *pcrel_op;

	  pcrel_op = (const struct mips_pcrel_operand *) operand;

	  if (pcrel_op->include_isa_bit && !need_reloc)
	    {
	      if (!mips_ignore_branch_isa
		  && !ELF_ST_IS_MIPS16 (S_GET_OTHER (fragp->fr_symbol)))
		as_bad_where (fragp->fr_file, fragp->fr_line,
			      _("branch to a symbol in another ISA mode"));
	      else if ((fragp->fr_offset & 0x1) != 0)
		as_bad_where (fragp->fr_file, fragp->fr_line,
			      _("branch to misaligned address (0x%lx)"),
			      (long) (resolve_symbol_value (fragp->fr_symbol)
				      + (fragp->fr_offset & ~1)));
	    }

	  val = mips16_pcrel_val (fragp, pcrel_op, val, 0);

	  /* Make sure the section winds up with the alignment we have
             assumed.  */
	  if (operand->shift > 0)
	    record_alignment (asec, operand->shift);
	}

      if (RELAX_MIPS16_JAL_DSLOT (fragp->fr_subtype)
	  || RELAX_MIPS16_DSLOT (fragp->fr_subtype))
	{
	  if (mac)
	    as_warn_where (fragp->fr_file, fragp->fr_line,
			   _("macro instruction expanded into multiple "
			     "instructions in a branch delay slot"));
	  else if (ext)
	    as_warn_where (fragp->fr_file, fragp->fr_line,
			   _("extended instruction in a branch delay slot"));
	}
      else if (RELAX_MIPS16_NOMACRO (fragp->fr_subtype) && mac)
	as_warn_where (fragp->fr_file, fragp->fr_line,
		       _("macro instruction expanded into multiple "
			 "instructions"));

      buf = fragp->fr_literal + fragp->fr_fix;

      insn = read_compressed_insn (buf, 2);
      if (ext)
	insn |= MIPS16_EXTEND;

      if (RELAX_MIPS16_USER_EXT (fragp->fr_subtype))
	user_length = 4;
      else if (RELAX_MIPS16_USER_SMALL (fragp->fr_subtype))
	user_length = 2;
      else
	user_length = 0;

      if (mac)
	{
	  unsigned long reg;
	  unsigned long new;
	  unsigned long op;
	  bool e2;

	  gas_assert (type == 'A' || type == 'B' || type == 'E');
	  gas_assert (RELAX_MIPS16_SYM32 (fragp->fr_subtype));

	  e2 = RELAX_MIPS16_E2 (fragp->fr_subtype);

	  if (need_reloc)
	    {
	      fixS *fixp;

	      gas_assert (!RELAX_MIPS16_PIC (fragp->fr_subtype));

	      fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
			      fragp->fr_symbol, fragp->fr_offset,
			      false, BFD_RELOC_MIPS16_HI16_S);
	      fixp->fx_file = fragp->fr_file;
	      fixp->fx_line = fragp->fr_line;

	      fixp = fix_new (fragp, buf - fragp->fr_literal + (e2 ? 4 : 8), 4,
			      fragp->fr_symbol, fragp->fr_offset,
			      false, BFD_RELOC_MIPS16_LO16);
	      fixp->fx_file = fragp->fr_file;
	      fixp->fx_line = fragp->fr_line;

	      val = 0;
	    }

	  switch (insn & 0xf800)
	    {
	    case 0x0800:					/* ADDIU */
	      reg = (insn >> 8) & 0x7;
	      op = 0xf0004800 | (reg << 8);
	      break;
	    case 0xb000:					/* LW */
	      reg = (insn >> 8) & 0x7;
	      op = 0xf0009800 | (reg << 8) | (reg << 5);
	      break;
	    case 0xf800:					/* I64 */
	      reg = (insn >> 5) & 0x7;
	      switch (insn & 0x0700)
		{
		case 0x0400:					/* LD */
		  op = 0xf0003800 | (reg << 8) | (reg << 5);
		  break;
		case 0x0600:					/* DADDIU */
		  op = 0xf000fd00 | (reg << 5);
		  break;
		default:
		  abort ();
		}
	      break;
	    default:
	      abort ();
	    }

	  new = (e2 ? 0xf0006820 : 0xf0006800) | (reg << 8);	/* LUI/LI */
	  new |= mips16_immed_extend ((val + 0x8000) >> 16, 16);
	  buf = write_compressed_insn (buf, new, 4);
	  if (!e2)
	    {
	      new = 0xf4003000 | (reg << 8) | (reg << 5);	/* SLL */
	      buf = write_compressed_insn (buf, new, 4);
	    }
	  op |= mips16_immed_extend (val, 16);
	  buf = write_compressed_insn (buf, op, 4);

	  fragp->fr_fix += e2 ? 8 : 12;
	}
      else
	{
	  unsigned int length = ext ? 4 : 2;

	  if (need_reloc)
	    {
	      bfd_reloc_code_real_type reloc = BFD_RELOC_NONE;
	      fixS *fixp;

	      switch (type)
		{
		case 'p':
		case 'q':
		  reloc = BFD_RELOC_MIPS16_16_PCREL_S1;
		  break;
		default:
		  break;
		}
	      if (mac || reloc == BFD_RELOC_NONE)
		as_bad_where (fragp->fr_file, fragp->fr_line,
			      _("unsupported relocation"));
	      else if (ext)
		{
		  fixp = fix_new (fragp, buf - fragp->fr_literal, 4,
				  fragp->fr_symbol, fragp->fr_offset,
				  true, reloc);
		  fixp->fx_file = fragp->fr_file;
		  fixp->fx_line = fragp->fr_line;
		}
	      else
		as_bad_where (fragp->fr_file, fragp->fr_line,
			      _("invalid unextended operand value"));
	    }
	  else
	    mips16_immed (fragp->fr_file, fragp->fr_line, type,
			  BFD_RELOC_UNUSED, val, user_length, &insn);

	  gas_assert (mips16_opcode_length (insn) == length);
	  write_compressed_insn (buf, insn, length);
	  fragp->fr_fix += length;
	}
    }
  else
    {
      relax_substateT subtype = fragp->fr_subtype;
      bool second_longer = (subtype & RELAX_SECOND_LONGER) != 0;
      bool use_second = (subtype & RELAX_USE_SECOND) != 0;
      unsigned int first, second;
      fixS *fixp;

      first = RELAX_FIRST (subtype);
      second = RELAX_SECOND (subtype);
      fixp = (fixS *) fragp->fr_opcode;

      /* If the delay slot chosen does not match the size of the instruction,
         then emit a warning.  */
      if ((!use_second && (subtype & RELAX_DELAY_SLOT_SIZE_FIRST) != 0)
	   || (use_second && (subtype & RELAX_DELAY_SLOT_SIZE_SECOND) != 0))
	{
	  relax_substateT s;
	  const char *msg;

	  s = subtype & (RELAX_DELAY_SLOT_16BIT
			 | RELAX_DELAY_SLOT_SIZE_FIRST
			 | RELAX_DELAY_SLOT_SIZE_SECOND);
	  msg = macro_warning (s);
	  if (msg != NULL)
	    as_warn_where (fragp->fr_file, fragp->fr_line, "%s", msg);
	  subtype &= ~s;
	}

      /* Possibly emit a warning if we've chosen the longer option.  */
      if (use_second == second_longer)
	{
	  relax_substateT s;
	  const char *msg;

	  s = (subtype
	       & (RELAX_SECOND_LONGER | RELAX_NOMACRO | RELAX_DELAY_SLOT));
	  msg = macro_warning (s);
	  if (msg != NULL)
	    as_warn_where (fragp->fr_file, fragp->fr_line, "%s", msg);
	  subtype &= ~s;
	}

      /* Go through all the fixups for the first sequence.  Disable them
	 (by marking them as done) if we're going to use the second
	 sequence instead.  */
      while (fixp
	     && fixp->fx_frag == fragp
	     && fixp->fx_where + second < fragp->fr_fix)
	{
	  if (subtype & RELAX_USE_SECOND)
	    fixp->fx_done = 1;
	  fixp = fixp->fx_next;
	}

      /* Go through the fixups for the second sequence.  Disable them if
	 we're going to use the first sequence, otherwise adjust their
	 addresses to account for the relaxation.  */
      while (fixp && fixp->fx_frag == fragp)
	{
	  if (subtype & RELAX_USE_SECOND)
	    fixp->fx_where -= first;
	  else
	    fixp->fx_done = 1;
	  fixp = fixp->fx_next;
	}

      /* Now modify the frag contents.  */
      if (subtype & RELAX_USE_SECOND)
	{
	  char *start;

	  start = fragp->fr_literal + fragp->fr_fix - first - second;
	  memmove (start, start + first, second);
	  fragp->fr_fix -= first;
	}
      else
	fragp->fr_fix -= second;
    }
}

/* This function is called after the relocs have been generated.
   We've been storing mips16 text labels as odd.  Here we convert them
   back to even for the convenience of the debugger.  */

void
mips_frob_file_after_relocs (void)
{
  asymbol **syms;
  unsigned int count, i;

  syms = bfd_get_outsymbols (stdoutput);
  count = bfd_get_symcount (stdoutput);
  for (i = 0; i < count; i++, syms++)
    if (ELF_ST_IS_COMPRESSED (elf_symbol (*syms)->internal_elf_sym.st_other)
	&& ((*syms)->value & 1) != 0)
      {
	(*syms)->value &= ~1;
	/* If the symbol has an odd size, it was probably computed
	   incorrectly, so adjust that as well.  */
	if ((elf_symbol (*syms)->internal_elf_sym.st_size & 1) != 0)
	  ++elf_symbol (*syms)->internal_elf_sym.st_size;
      }
}

/* This function is called whenever a label is defined, including fake
   labels instantiated off the dot special symbol.  It is used when
   handling branch delays; if a branch has a label, we assume we cannot
   move it.  This also bumps the value of the symbol by 1 in compressed
   code.  */

static void
mips_record_label (symbolS *sym)
{
  segment_info_type *si = seg_info (now_seg);
  struct insn_label_list *l;

  if (free_insn_labels == NULL)
    l = XNEW (struct insn_label_list);
  else
    {
      l = free_insn_labels;
      free_insn_labels = l->next;
    }

  l->label = sym;
  l->next = si->label_list;
  si->label_list = l;
}

/* This function is called as tc_frob_label() whenever a label is defined
   and adds a DWARF-2 record we only want for true labels.  */

void
mips_define_label (symbolS *sym)
{
  mips_record_label (sym);
  dwarf2_emit_label (sym);
}

/* This function is called by tc_new_dot_label whenever a new dot symbol
   is defined.  */

void
mips_add_dot_label (symbolS *sym)
{
  mips_record_label (sym);
  if (mips_assembling_insn && HAVE_CODE_COMPRESSION)
    mips_compressed_mark_label (sym);
}

/* Converting ASE flags from internal to .MIPS.abiflags values.  */
static unsigned int
mips_convert_ase_flags (int ase)
{
  unsigned int ext_ases = 0;

  if (ase & ASE_DSP)
    ext_ases |= AFL_ASE_DSP;
  if (ase & ASE_DSPR2)
    ext_ases |= AFL_ASE_DSPR2;
  if (ase & ASE_DSPR3)
    ext_ases |= AFL_ASE_DSPR3;
  if (ase & ASE_EVA)
    ext_ases |= AFL_ASE_EVA;
  if (ase & ASE_MCU)
    ext_ases |= AFL_ASE_MCU;
  if (ase & ASE_MDMX)
    ext_ases |= AFL_ASE_MDMX;
  if (ase & ASE_MIPS3D)
    ext_ases |= AFL_ASE_MIPS3D;
  if (ase & ASE_MT)
    ext_ases |= AFL_ASE_MT;
  if (ase & ASE_SMARTMIPS)
    ext_ases |= AFL_ASE_SMARTMIPS;
  if (ase & ASE_VIRT)
    ext_ases |= AFL_ASE_VIRT;
  if (ase & ASE_MSA)
    ext_ases |= AFL_ASE_MSA;
  if (ase & ASE_XPA)
    ext_ases |= AFL_ASE_XPA;
  if (ase & ASE_MIPS16E2)
    ext_ases |= file_ase_mips16 ? AFL_ASE_MIPS16E2 : 0;
  if (ase & ASE_CRC)
    ext_ases |= AFL_ASE_CRC;
  if (ase & ASE_GINV)
    ext_ases |= AFL_ASE_GINV;
  if (ase & ASE_LOONGSON_MMI)
    ext_ases |= AFL_ASE_LOONGSON_MMI;
  if (ase & ASE_LOONGSON_CAM)
    ext_ases |= AFL_ASE_LOONGSON_CAM;
  if (ase & ASE_LOONGSON_EXT)
    ext_ases |= AFL_ASE_LOONGSON_EXT;
  if (ase & ASE_LOONGSON_EXT2)
    ext_ases |= AFL_ASE_LOONGSON_EXT2;

  return ext_ases;
}
/* Some special processing for a MIPS ELF file.  */

void
mips_elf_final_processing (void)
{
  int fpabi;
  Elf_Internal_ABIFlags_v0 flags;

  flags.version = 0;
  flags.isa_rev = 0;
  switch (file_mips_opts.isa)
    {
    case INSN_ISA1:
      flags.isa_level = 1;
      break;
    case INSN_ISA2:
      flags.isa_level = 2;
      break;
    case INSN_ISA3:
      flags.isa_level = 3;
      break;
    case INSN_ISA4:
      flags.isa_level = 4;
      break;
    case INSN_ISA5:
      flags.isa_level = 5;
      break;
    case INSN_ISA32:
      flags.isa_level = 32;
      flags.isa_rev = 1;
      break;
    case INSN_ISA32R2:
      flags.isa_level = 32;
      flags.isa_rev = 2;
      break;
    case INSN_ISA32R3:
      flags.isa_level = 32;
      flags.isa_rev = 3;
      break;
    case INSN_ISA32R5:
      flags.isa_level = 32;
      flags.isa_rev = 5;
      break;
    case INSN_ISA32R6:
      flags.isa_level = 32;
      flags.isa_rev = 6;
      break;
    case INSN_ISA64:
      flags.isa_level = 64;
      flags.isa_rev = 1;
      break;
    case INSN_ISA64R2:
      flags.isa_level = 64;
      flags.isa_rev = 2;
      break;
    case INSN_ISA64R3:
      flags.isa_level = 64;
      flags.isa_rev = 3;
      break;
    case INSN_ISA64R5:
      flags.isa_level = 64;
      flags.isa_rev = 5;
      break;
    case INSN_ISA64R6:
      flags.isa_level = 64;
      flags.isa_rev = 6;
      break;
    }

  flags.gpr_size = file_mips_opts.gp == 32 ? AFL_REG_32 : AFL_REG_64;
  flags.cpr1_size = file_mips_opts.soft_float ? AFL_REG_NONE
		    : (file_mips_opts.ase & ASE_MSA) ? AFL_REG_128
		    : (file_mips_opts.fp == 64) ? AFL_REG_64
		    : AFL_REG_32;
  flags.cpr2_size = AFL_REG_NONE;
  flags.fp_abi = bfd_elf_get_obj_attr_int (stdoutput, OBJ_ATTR_GNU,
                                           Tag_GNU_MIPS_ABI_FP);
  flags.isa_ext = bfd_mips_isa_ext (stdoutput);
  flags.ases = mips_convert_ase_flags (file_mips_opts.ase);
  if (file_ase_mips16)
    flags.ases |= AFL_ASE_MIPS16;
  if (file_ase_micromips)
    flags.ases |= AFL_ASE_MICROMIPS;
  flags.flags1 = 0;
  if ((ISA_HAS_ODD_SINGLE_FPR (file_mips_opts.isa, file_mips_opts.arch)
       || file_mips_opts.fp == 64)
      && file_mips_opts.oddspreg)
    flags.flags1 |= AFL_FLAGS1_ODDSPREG;
  flags.flags2 = 0;

  bfd_mips_elf_swap_abiflags_v0_out (stdoutput, &flags,
				     ((Elf_External_ABIFlags_v0 *)
				     mips_flags_frag));

  /* Write out the register information.  */
  if (mips_abi != N64_ABI)
    {
      Elf32_RegInfo s;

      s.ri_gprmask = mips_gprmask;
      s.ri_cprmask[0] = mips_cprmask[0];
      s.ri_cprmask[1] = mips_cprmask[1];
      s.ri_cprmask[2] = mips_cprmask[2];
      s.ri_cprmask[3] = mips_cprmask[3];
      /* The gp_value field is set by the MIPS ELF backend.  */

      bfd_mips_elf32_swap_reginfo_out (stdoutput, &s,
				       ((Elf32_External_RegInfo *)
					mips_regmask_frag));
    }
  else
    {
      Elf64_Internal_RegInfo s;

      s.ri_gprmask = mips_gprmask;
      s.ri_pad = 0;
      s.ri_cprmask[0] = mips_cprmask[0];
      s.ri_cprmask[1] = mips_cprmask[1];
      s.ri_cprmask[2] = mips_cprmask[2];
      s.ri_cprmask[3] = mips_cprmask[3];
      /* The gp_value field is set by the MIPS ELF backend.  */

      bfd_mips_elf64_swap_reginfo_out (stdoutput, &s,
				       ((Elf64_External_RegInfo *)
					mips_regmask_frag));
    }

  /* Set the MIPS ELF flag bits.  FIXME: There should probably be some
     sort of BFD interface for this.  */
  if (mips_any_noreorder)
    elf_elfheader (stdoutput)->e_flags |= EF_MIPS_NOREORDER;
  if (mips_pic != NO_PIC)
    {
      elf_elfheader (stdoutput)->e_flags |= EF_MIPS_PIC;
      elf_elfheader (stdoutput)->e_flags |= EF_MIPS_CPIC;
    }
  if (mips_abicalls)
    elf_elfheader (stdoutput)->e_flags |= EF_MIPS_CPIC;

  /* Set MIPS ELF flags for ASEs.  Note that not all ASEs have flags
     defined at present; this might need to change in future.  */
  if (file_ase_mips16)
    elf_elfheader (stdoutput)->e_flags |= EF_MIPS_ARCH_ASE_M16;
  if (file_ase_micromips)
    elf_elfheader (stdoutput)->e_flags |= EF_MIPS_ARCH_ASE_MICROMIPS;
  if (file_mips_opts.ase & ASE_MDMX)
    elf_elfheader (stdoutput)->e_flags |= EF_MIPS_ARCH_ASE_MDMX;

  /* Set the MIPS ELF ABI flags.  */
  if (mips_abi == O32_ABI && USE_E_MIPS_ABI_O32)
    elf_elfheader (stdoutput)->e_flags |= E_MIPS_ABI_O32;
  else if (mips_abi == O64_ABI)
    elf_elfheader (stdoutput)->e_flags |= E_MIPS_ABI_O64;
  else if (mips_abi == EABI_ABI)
    {
      if (file_mips_opts.gp == 64)
	elf_elfheader (stdoutput)->e_flags |= E_MIPS_ABI_EABI64;
      else
	elf_elfheader (stdoutput)->e_flags |= E_MIPS_ABI_EABI32;
    }

  /* Nothing to do for N32_ABI or N64_ABI.  */

  if (mips_32bitmode)
    elf_elfheader (stdoutput)->e_flags |= EF_MIPS_32BITMODE;

  if (mips_nan2008 == 1)
    elf_elfheader (stdoutput)->e_flags |= EF_MIPS_NAN2008;

  /* 32 bit code with 64 bit FP registers.  */
  fpabi = bfd_elf_get_obj_attr_int (stdoutput, OBJ_ATTR_GNU,
				    Tag_GNU_MIPS_ABI_FP);
  if (fpabi == Val_GNU_MIPS_ABI_FP_OLD_64)
    elf_elfheader (stdoutput)->e_flags |= EF_MIPS_FP64;
}

typedef struct proc {
  symbolS *func_sym;
  symbolS *func_end_sym;
  unsigned long reg_mask;
  unsigned long reg_offset;
  unsigned long fpreg_mask;
  unsigned long fpreg_offset;
  unsigned long frame_offset;
  unsigned long frame_reg;
  unsigned long pc_reg;
} procS;

static procS cur_proc;
static procS *cur_proc_ptr;
static int numprocs;

/* Implement NOP_OPCODE.  We encode a MIPS16 nop as "1", a microMIPS nop
   as "2", and a normal nop as "0".  */

#define NOP_OPCODE_MIPS		0
#define NOP_OPCODE_MIPS16	1
#define NOP_OPCODE_MICROMIPS	2

char
mips_nop_opcode (void)
{
  if (seg_info (now_seg)->tc_segment_info_data.micromips)
    return NOP_OPCODE_MICROMIPS;
  else if (seg_info (now_seg)->tc_segment_info_data.mips16)
    return NOP_OPCODE_MIPS16;
  else
    return NOP_OPCODE_MIPS;
}

/* Fill in an rs_align_code fragment.  Unlike elsewhere we want to use
   32-bit microMIPS NOPs here (if applicable).  */

void
mips_handle_align (fragS *fragp)
{
  char nop_opcode;
  char *p;
  int bytes, size, excess;
  valueT opcode;

  if (fragp->fr_type != rs_align_code)
    return;

  p = fragp->fr_literal + fragp->fr_fix;
  nop_opcode = *p;
  switch (nop_opcode)
    {
    case NOP_OPCODE_MICROMIPS:
      opcode = micromips_nop32_insn.insn_opcode;
      size = 4;
      break;
    case NOP_OPCODE_MIPS16:
      opcode = mips16_nop_insn.insn_opcode;
      size = 2;
      break;
    case NOP_OPCODE_MIPS:
    default:
      opcode = nop_insn.insn_opcode;
      size = 4;
      break;
    }

  bytes = fragp->fr_next->fr_address - fragp->fr_address - fragp->fr_fix;
  excess = bytes % size;

  /* Handle the leading part if we're not inserting a whole number of
     instructions, and make it the end of the fixed part of the frag.
     Try to fit in a short microMIPS NOP if applicable and possible,
     and use zeroes otherwise.  */
  gas_assert (excess < 4);
  fragp->fr_fix += excess;
  switch (excess)
    {
    case 3:
      *p++ = '\0';
      /* Fall through.  */
    case 2:
      if (nop_opcode == NOP_OPCODE_MICROMIPS && !mips_opts.insn32)
	{
	  p = write_compressed_insn (p, micromips_nop16_insn.insn_opcode, 2);
	  break;
	}
      *p++ = '\0';
      /* Fall through.  */
    case 1:
      *p++ = '\0';
      /* Fall through.  */
    case 0:
      break;
    }

  md_number_to_chars (p, opcode, size);
  fragp->fr_var = size;
}

static long
get_number (void)
{
  int negative = 0;
  long val = 0;

  if (*input_line_pointer == '-')
    {
      ++input_line_pointer;
      negative = 1;
    }
  if (!ISDIGIT (*input_line_pointer))
    as_bad (_("expected simple number"));
  if (input_line_pointer[0] == '0')
    {
      if (input_line_pointer[1] == 'x')
	{
	  input_line_pointer += 2;
	  while (ISXDIGIT (*input_line_pointer))
	    {
	      val <<= 4;
	      val |= hex_value (*input_line_pointer++);
	    }
	  return negative ? -val : val;
	}
      else
	{
	  ++input_line_pointer;
	  while (ISDIGIT (*input_line_pointer))
	    {
	      val <<= 3;
	      val |= *input_line_pointer++ - '0';
	    }
	  return negative ? -val : val;
	}
    }
  if (!ISDIGIT (*input_line_pointer))
    {
      printf (_(" *input_line_pointer == '%c' 0x%02x\n"),
	      *input_line_pointer, *input_line_pointer);
      as_warn (_("invalid number"));
      return -1;
    }
  while (ISDIGIT (*input_line_pointer))
    {
      val *= 10;
      val += *input_line_pointer++ - '0';
    }
  return negative ? -val : val;
}

/* The .file directive; just like the usual .file directive, but there
   is an initial number which is the ECOFF file index.  In the non-ECOFF
   case .file implies DWARF-2.  */

static void
s_mips_file (int x ATTRIBUTE_UNUSED)
{
  static int first_file_directive = 0;

  if (ECOFF_DEBUGGING)
    {
      get_number ();
      s_file (0);
    }
  else
    {
      char *filename;

      filename = dwarf2_directive_filename ();

      /* Versions of GCC up to 3.1 start files with a ".file"
	 directive even for stabs output.  Make sure that this
	 ".file" is handled.  Note that you need a version of GCC
         after 3.1 in order to support DWARF-2 on MIPS.  */
      if (filename != NULL && ! first_file_directive)
	{
	  new_logical_line (filename, -1);
	  s_file_string (filename);
	}
      first_file_directive = 1;
    }
}

/* The .loc directive, implying DWARF-2.  */

static void
s_mips_loc (int x ATTRIBUTE_UNUSED)
{
  if (!ECOFF_DEBUGGING)
    dwarf2_directive_loc (0);
}

/* The .end directive.  */

static void
s_mips_end (int x ATTRIBUTE_UNUSED)
{
  symbolS *p;

  /* Following functions need their own .frame and .cprestore directives.  */
  mips_frame_reg_valid = 0;
  mips_cprestore_valid = 0;

  if (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      p = get_symbol ();
      demand_empty_rest_of_line ();
    }
  else
    p = NULL;

  if ((bfd_section_flags (now_seg) & SEC_CODE) == 0)
    as_warn (_(".end not in text section"));

  if (!cur_proc_ptr)
    {
      as_warn (_(".end directive without a preceding .ent directive"));
      demand_empty_rest_of_line ();
      return;
    }

  if (p != NULL)
    {
      gas_assert (S_GET_NAME (p));
      if (strcmp (S_GET_NAME (p), S_GET_NAME (cur_proc_ptr->func_sym)))
	as_warn (_(".end symbol does not match .ent symbol"));

      if (debug_type == DEBUG_STABS)
	stabs_generate_asm_endfunc (S_GET_NAME (p),
				    S_GET_NAME (p));
    }
  else
    as_warn (_(".end directive missing or unknown symbol"));

  /* Create an expression to calculate the size of the function.  */
  if (p && cur_proc_ptr)
    {
      OBJ_SYMFIELD_TYPE *obj = symbol_get_obj (p);
      expressionS *exp = XNEW (expressionS);

      obj->size = exp;
      exp->X_op = O_subtract;
      exp->X_add_symbol = symbol_temp_new_now ();
      exp->X_op_symbol = p;
      exp->X_add_number = 0;

      cur_proc_ptr->func_end_sym = exp->X_add_symbol;
    }

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  /* Generate a .pdr section.  */
  if (!ECOFF_DEBUGGING && mips_flag_pdr)
    {
      segT saved_seg = now_seg;
      subsegT saved_subseg = now_subseg;
      expressionS exp;
      char *fragp;

      gas_assert (pdr_seg);
      subseg_set (pdr_seg, 0);

      /* Write the symbol.  */
      exp.X_op = O_symbol;
      exp.X_add_symbol = p;
      exp.X_add_number = 0;
      emit_expr (&exp, 4);

      fragp = frag_more (7 * 4);

      md_number_to_chars (fragp, cur_proc_ptr->reg_mask, 4);
      md_number_to_chars (fragp + 4, cur_proc_ptr->reg_offset, 4);
      md_number_to_chars (fragp + 8, cur_proc_ptr->fpreg_mask, 4);
      md_number_to_chars (fragp + 12, cur_proc_ptr->fpreg_offset, 4);
      md_number_to_chars (fragp + 16, cur_proc_ptr->frame_offset, 4);
      md_number_to_chars (fragp + 20, cur_proc_ptr->frame_reg, 4);
      md_number_to_chars (fragp + 24, cur_proc_ptr->pc_reg, 4);

      subseg_set (saved_seg, saved_subseg);
    }

  cur_proc_ptr = NULL;
}

/* The .aent and .ent directives.  */

static void
s_mips_ent (int aent)
{
  symbolS *symbolP;

  symbolP = get_symbol ();
  if (*input_line_pointer == ',')
    ++input_line_pointer;
  SKIP_WHITESPACE ();
  if (ISDIGIT (*input_line_pointer)
      || *input_line_pointer == '-')
    get_number ();

  if ((bfd_section_flags (now_seg) & SEC_CODE) == 0)
    as_warn (_(".ent or .aent not in text section"));

  if (!aent && cur_proc_ptr)
    as_warn (_("missing .end"));

  if (!aent)
    {
      /* This function needs its own .frame and .cprestore directives.  */
      mips_frame_reg_valid = 0;
      mips_cprestore_valid = 0;

      cur_proc_ptr = &cur_proc;
      memset (cur_proc_ptr, '\0', sizeof (procS));

      cur_proc_ptr->func_sym = symbolP;

      ++numprocs;

      if (debug_type == DEBUG_STABS)
        stabs_generate_asm_func (S_GET_NAME (symbolP),
				 S_GET_NAME (symbolP));
    }

  symbol_get_bfdsym (symbolP)->flags |= BSF_FUNCTION;

  demand_empty_rest_of_line ();
}

/* The .frame directive. If the mdebug section is present (IRIX 5 native)
   then ecoff.c (ecoff_directive_frame) is used. For embedded targets,
   s_mips_frame is used so that we can set the PDR information correctly.
   We can't use the ecoff routines because they make reference to the ecoff
   symbol table (in the mdebug section).  */

static void
s_mips_frame (int ignore ATTRIBUTE_UNUSED)
{
  if (ECOFF_DEBUGGING)
    s_ignore (ignore);
  else
    {
      long val;

      if (cur_proc_ptr == (procS *) NULL)
	{
	  as_warn (_(".frame outside of .ent"));
	  demand_empty_rest_of_line ();
	  return;
	}

      cur_proc_ptr->frame_reg = tc_get_register (1);

      SKIP_WHITESPACE ();
      if (*input_line_pointer++ != ','
	  || get_absolute_expression_and_terminator (&val) != ',')
	{
	  as_warn (_("bad .frame directive"));
	  --input_line_pointer;
	  demand_empty_rest_of_line ();
	  return;
	}

      cur_proc_ptr->frame_offset = val;
      cur_proc_ptr->pc_reg = tc_get_register (0);

      demand_empty_rest_of_line ();
    }
}

/* The .fmask and .mask directives. If the mdebug section is present
   (IRIX 5 native) then ecoff.c (ecoff_directive_mask) is used. For
   embedded targets, s_mips_mask is used so that we can set the PDR
   information correctly. We can't use the ecoff routines because they
   make reference to the ecoff symbol table (in the mdebug section).  */

static void
s_mips_mask (int reg_type)
{
  if (ECOFF_DEBUGGING)
    s_ignore (reg_type);
  else
    {
      long mask, off;

      if (cur_proc_ptr == (procS *) NULL)
	{
	  as_warn (_(".mask/.fmask outside of .ent"));
	  demand_empty_rest_of_line ();
	  return;
	}

      if (get_absolute_expression_and_terminator (&mask) != ',')
	{
	  as_warn (_("bad .mask/.fmask directive"));
	  --input_line_pointer;
	  demand_empty_rest_of_line ();
	  return;
	}

      off = get_absolute_expression ();

      if (reg_type == 'F')
	{
	  cur_proc_ptr->fpreg_mask = mask;
	  cur_proc_ptr->fpreg_offset = off;
	}
      else
	{
	  cur_proc_ptr->reg_mask = mask;
	  cur_proc_ptr->reg_offset = off;
	}

      demand_empty_rest_of_line ();
    }
}

/* A table describing all the processors gas knows about.  Names are
   matched in the order listed.

   To ease comparison, please keep this table in the same order as
   gcc's mips_cpu_info_table[].  */
static const struct mips_cpu_info mips_cpu_info_table[] =
{
  /* Entries for generic ISAs.  */
  { "mips1",          MIPS_CPU_IS_ISA, 0,	ISA_MIPS1,    CPU_R3000 },
  { "mips2",          MIPS_CPU_IS_ISA, 0,	ISA_MIPS2,    CPU_R6000 },
  { "mips3",          MIPS_CPU_IS_ISA, 0,	ISA_MIPS3,    CPU_R4000 },
  { "mips4",          MIPS_CPU_IS_ISA, 0,	ISA_MIPS4,    CPU_R8000 },
  { "mips5",          MIPS_CPU_IS_ISA, 0,	ISA_MIPS5,    CPU_MIPS5 },
  { "mips32",         MIPS_CPU_IS_ISA, 0,	ISA_MIPS32,   CPU_MIPS32 },
  { "mips32r2",       MIPS_CPU_IS_ISA, 0,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "mips32r3",       MIPS_CPU_IS_ISA, 0,	ISA_MIPS32R3, CPU_MIPS32R3 },
  { "mips32r5",       MIPS_CPU_IS_ISA, 0,	ISA_MIPS32R5, CPU_MIPS32R5 },
  { "mips32r6",       MIPS_CPU_IS_ISA, 0,	ISA_MIPS32R6, CPU_MIPS32R6 },
  { "mips64",         MIPS_CPU_IS_ISA, 0,	ISA_MIPS64,   CPU_MIPS64 },
  { "mips64r2",       MIPS_CPU_IS_ISA, 0,	ISA_MIPS64R2, CPU_MIPS64R2 },
  { "mips64r3",       MIPS_CPU_IS_ISA, 0,	ISA_MIPS64R3, CPU_MIPS64R3 },
  { "mips64r5",       MIPS_CPU_IS_ISA, 0,	ISA_MIPS64R5, CPU_MIPS64R5 },
  { "mips64r6",       MIPS_CPU_IS_ISA, 0,	ISA_MIPS64R6, CPU_MIPS64R6 },

  /* MIPS I */
  { "r3000",          0, 0,			ISA_MIPS1,    CPU_R3000 },
  { "r2000",          0, 0,			ISA_MIPS1,    CPU_R3000 },
  { "r3900",          0, 0,			ISA_MIPS1,    CPU_R3900 },

  /* MIPS II */
  { "r6000",          0, 0,			ISA_MIPS2,    CPU_R6000 },
  { "allegrex",       0, 0,			ISA_MIPS2,    CPU_ALLEGREX },

  /* MIPS III */
  { "r4000",          0, 0,			ISA_MIPS3,    CPU_R4000 },
  { "r4010",          0, 0,			ISA_MIPS2,    CPU_R4010 },
  { "vr4100",         0, 0,			ISA_MIPS3,    CPU_VR4100 },
  { "vr4111",         0, 0,			ISA_MIPS3,    CPU_R4111 },
  { "vr4120",         0, 0,			ISA_MIPS3,    CPU_VR4120 },
  { "vr4130",         0, 0,			ISA_MIPS3,    CPU_VR4120 },
  { "vr4181",         0, 0,			ISA_MIPS3,    CPU_R4111 },
  { "vr4300",         0, 0,			ISA_MIPS3,    CPU_R4300 },
  { "r4400",          0, 0,			ISA_MIPS3,    CPU_R4400 },
  { "r4600",          0, 0,			ISA_MIPS3,    CPU_R4600 },
  { "orion",          0, 0,			ISA_MIPS3,    CPU_R4600 },
  { "r4650",          0, 0,			ISA_MIPS3,    CPU_R4650 },
  { "r5900",          0, 0,			ISA_MIPS3,    CPU_R5900 },
  /* ST Microelectronics Loongson 2E and 2F cores.  */
  { "loongson2e",     0, 0,			ISA_MIPS3,    CPU_LOONGSON_2E },
  { "loongson2f",     0, ASE_LOONGSON_MMI,	ISA_MIPS3,    CPU_LOONGSON_2F },

  /* MIPS IV */
  { "r8000",          0, 0,			ISA_MIPS4,    CPU_R8000 },
  { "r10000",         0, 0,			ISA_MIPS4,    CPU_R10000 },
  { "r12000",         0, 0,			ISA_MIPS4,    CPU_R12000 },
  { "r14000",         0, 0,			ISA_MIPS4,    CPU_R14000 },
  { "r16000",         0, 0,			ISA_MIPS4,    CPU_R16000 },
  { "vr5000",         0, 0,			ISA_MIPS4,    CPU_R5000 },
  { "vr5400",         0, 0,			ISA_MIPS4,    CPU_VR5400 },
  { "vr5500",         0, 0,			ISA_MIPS4,    CPU_VR5500 },
  { "rm5200",         0, 0,			ISA_MIPS4,    CPU_R5000 },
  { "rm5230",         0, 0,			ISA_MIPS4,    CPU_R5000 },
  { "rm5231",         0, 0,			ISA_MIPS4,    CPU_R5000 },
  { "rm5261",         0, 0,			ISA_MIPS4,    CPU_R5000 },
  { "rm5721",         0, 0,			ISA_MIPS4,    CPU_R5000 },
  { "rm7000",         0, 0,			ISA_MIPS4,    CPU_RM7000 },
  { "rm9000",         0, 0,			ISA_MIPS4,    CPU_RM9000 },

  /* MIPS 32 */
  { "4kc",            0, 0,			ISA_MIPS32,   CPU_MIPS32 },
  { "4km",            0, 0,			ISA_MIPS32,   CPU_MIPS32 },
  { "4kp",            0, 0,			ISA_MIPS32,   CPU_MIPS32 },
  { "4ksc",           0, ASE_SMARTMIPS,		ISA_MIPS32,   CPU_MIPS32 },

  /* MIPS 32 Release 2 */
  { "4kec",           0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "4kem",           0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "4kep",           0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "4ksd",           0, ASE_SMARTMIPS,		ISA_MIPS32R2, CPU_MIPS32R2 },
  { "m4k",            0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "m4kp",           0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "m14k",           0, ASE_MCU,		ISA_MIPS32R2, CPU_MIPS32R2 },
  { "m14kc",          0, ASE_MCU,		ISA_MIPS32R2, CPU_MIPS32R2 },
  { "m14ke",          0, ASE_DSP | ASE_DSPR2 | ASE_MCU,
						ISA_MIPS32R2, CPU_MIPS32R2 },
  { "m14kec",         0, ASE_DSP | ASE_DSPR2 | ASE_MCU,
						ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kc",           0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kf2_1",        0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kf",           0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kf1_1",        0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  /* Deprecated forms of the above.  */
  { "24kfx",          0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kx",           0, 0,			ISA_MIPS32R2, CPU_MIPS32R2 },
  /* 24KE is a 24K with DSP ASE, other ASEs are optional.  */
  { "24kec",          0, ASE_DSP,		ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kef2_1",       0, ASE_DSP,		ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kef",          0, ASE_DSP,		ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kef1_1",       0, ASE_DSP,		ISA_MIPS32R2, CPU_MIPS32R2 },
  /* Deprecated forms of the above.  */
  { "24kefx",         0, ASE_DSP,		ISA_MIPS32R2, CPU_MIPS32R2 },
  { "24kex",          0, ASE_DSP,		ISA_MIPS32R2, CPU_MIPS32R2 },
  /* 34K is a 24K with DSP and MT ASE, other ASEs are optional.  */
  { "34kc",           0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "34kf2_1",        0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "34kf",           0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "34kf1_1",        0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  /* Deprecated forms of the above.  */
  { "34kfx",          0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "34kx",           0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  /* 34Kn is a 34kc without DSP.  */
  { "34kn",           0, ASE_MT,		ISA_MIPS32R2, CPU_MIPS32R2 },
  /* 74K with DSP and DSPR2 ASE, other ASEs are optional.  */
  { "74kc",           0, ASE_DSP | ASE_DSPR2,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "74kf2_1",        0, ASE_DSP | ASE_DSPR2,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "74kf",           0, ASE_DSP | ASE_DSPR2,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "74kf1_1",        0, ASE_DSP | ASE_DSPR2,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "74kf3_2",        0, ASE_DSP | ASE_DSPR2,	ISA_MIPS32R2, CPU_MIPS32R2 },
  /* Deprecated forms of the above.  */
  { "74kfx",          0, ASE_DSP | ASE_DSPR2,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "74kx",           0, ASE_DSP | ASE_DSPR2,	ISA_MIPS32R2, CPU_MIPS32R2 },
  /* 1004K cores are multiprocessor versions of the 34K.  */
  { "1004kc",         0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "1004kf2_1",      0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "1004kf",         0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "1004kf1_1",      0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  /* interaptiv is the new name for 1004kf.  */
  { "interaptiv",     0, ASE_DSP | ASE_MT,	ISA_MIPS32R2, CPU_MIPS32R2 },
  { "interaptiv-mr2", 0,
    ASE_DSP | ASE_EVA | ASE_MT | ASE_MIPS16E2 | ASE_MIPS16E2_MT,
    ISA_MIPS32R3, CPU_INTERAPTIV_MR2 },
  /* M5100 family.  */
  { "m5100",          0, ASE_MCU,		ISA_MIPS32R5, CPU_MIPS32R5 },
  { "m5101",          0, ASE_MCU,		ISA_MIPS32R5, CPU_MIPS32R5 },
  /* P5600 with EVA and Virtualization ASEs, other ASEs are optional.  */
  { "p5600",          0, ASE_VIRT | ASE_EVA | ASE_XPA,	ISA_MIPS32R5, CPU_MIPS32R5 },

  /* MIPS 64 */
  { "5kc",            0, 0,			ISA_MIPS64,   CPU_MIPS64 },
  { "5kf",            0, 0,			ISA_MIPS64,   CPU_MIPS64 },
  { "20kc",           0, ASE_MIPS3D,		ISA_MIPS64,   CPU_MIPS64 },
  { "25kf",           0, ASE_MIPS3D,		ISA_MIPS64,   CPU_MIPS64 },

  /* Broadcom SB-1 CPU core.  */
  { "sb1",            0, ASE_MIPS3D | ASE_MDMX,	ISA_MIPS64,   CPU_SB1 },
  /* Broadcom SB-1A CPU core.  */
  { "sb1a",           0, ASE_MIPS3D | ASE_MDMX,	ISA_MIPS64,   CPU_SB1 },

  /* MIPS 64 Release 2.  */
  /* Loongson CPU core.  */
  /* -march=loongson3a is an alias of -march=gs464 for compatibility.  */
  { "loongson3a",     0, ASE_LOONGSON_MMI | ASE_LOONGSON_CAM | ASE_LOONGSON_EXT,
     ISA_MIPS64R2,	CPU_GS464 },
  { "gs464",          0, ASE_LOONGSON_MMI | ASE_LOONGSON_CAM | ASE_LOONGSON_EXT,
     ISA_MIPS64R2,	CPU_GS464 },
  { "gs464e",         0, ASE_LOONGSON_MMI | ASE_LOONGSON_CAM | ASE_LOONGSON_EXT
     | ASE_LOONGSON_EXT2,	ISA_MIPS64R2,	CPU_GS464E },
  { "gs264e",         0, ASE_LOONGSON_MMI | ASE_LOONGSON_CAM | ASE_LOONGSON_EXT
     | ASE_LOONGSON_EXT2 | ASE_MSA | ASE_MSA64,	ISA_MIPS64R2,	CPU_GS264E },

  /* Cavium Networks Octeon CPU core.  */
  { "octeon",	      0, 0,			ISA_MIPS64R2, CPU_OCTEON },
  { "octeon+",	      0, 0,			ISA_MIPS64R2, CPU_OCTEONP },
  { "octeon2",	      0, 0,			ISA_MIPS64R2, CPU_OCTEON2 },
  { "octeon3",	      0, ASE_VIRT | ASE_VIRT64,	ISA_MIPS64R5, CPU_OCTEON3 },

  /* RMI Xlr */
  { "xlr",	      0, 0,			ISA_MIPS64,   CPU_XLR },

  /* Broadcom XLP.
     XLP is mostly like XLR, with the prominent exception that it is
     MIPS64R2 rather than MIPS64.  */
  { "xlp",	      0, 0,			ISA_MIPS64R2, CPU_XLR },

  /* MIPS 64 Release 6.  */
  { "i6400",	      0, ASE_VIRT | ASE_MSA,	ISA_MIPS64R6, CPU_MIPS64R6},
  { "i6500",	      0, ASE_VIRT | ASE_MSA | ASE_CRC | ASE_GINV,
						ISA_MIPS64R6, CPU_MIPS64R6},
  { "p6600",	      0, ASE_VIRT | ASE_MSA,	ISA_MIPS64R6, CPU_MIPS64R6},

  /* End marker.  */
  { NULL, 0, 0, 0, 0 }
};


/* Return true if GIVEN is the same as CANONICAL, or if it is CANONICAL
   with a final "000" replaced by "k".  Ignore case.

   Note: this function is shared between GCC and GAS.  */

static bool
mips_strict_matching_cpu_name_p (const char *canonical, const char *given)
{
  while (*given != 0 && TOLOWER (*given) == TOLOWER (*canonical))
    given++, canonical++;

  return ((*given == 0 && *canonical == 0)
	  || (strcmp (canonical, "000") == 0 && strcasecmp (given, "k") == 0));
}


/* Return true if GIVEN matches CANONICAL, where GIVEN is a user-supplied
   CPU name.  We've traditionally allowed a lot of variation here.

   Note: this function is shared between GCC and GAS.  */

static bool
mips_matching_cpu_name_p (const char *canonical, const char *given)
{
  /* First see if the name matches exactly, or with a final "000"
     turned into "k".  */
  if (mips_strict_matching_cpu_name_p (canonical, given))
    return true;

  /* If not, try comparing based on numerical designation alone.
     See if GIVEN is an unadorned number, or 'r' followed by a number.  */
  if (TOLOWER (*given) == 'r')
    given++;
  if (!ISDIGIT (*given))
    return false;

  /* Skip over some well-known prefixes in the canonical name,
     hoping to find a number there too.  */
  if (TOLOWER (canonical[0]) == 'v' && TOLOWER (canonical[1]) == 'r')
    canonical += 2;
  else if (TOLOWER (canonical[0]) == 'r' && TOLOWER (canonical[1]) == 'm')
    canonical += 2;
  else if (TOLOWER (canonical[0]) == 'r')
    canonical += 1;

  return mips_strict_matching_cpu_name_p (canonical, given);
}


/* Parse an option that takes the name of a processor as its argument.
   OPTION is the name of the option and CPU_STRING is the argument.
   Return the corresponding processor enumeration if the CPU_STRING is
   recognized, otherwise report an error and return null.

   A similar function exists in GCC.  */

static const struct mips_cpu_info *
mips_parse_cpu (const char *option, const char *cpu_string)
{
  const struct mips_cpu_info *p;

  /* 'from-abi' selects the most compatible architecture for the given
     ABI: MIPS I for 32-bit ABIs and MIPS III for 64-bit ABIs.  For the
     EABIs, we have to decide whether we're using the 32-bit or 64-bit
     version.  Look first at the -mgp options, if given, otherwise base
     the choice on MIPS_DEFAULT_64BIT.

     Treat NO_ABI like the EABIs.  One reason to do this is that the
     plain 'mips' and 'mips64' configs have 'from-abi' as their default
     architecture.  This code picks MIPS I for 'mips' and MIPS III for
     'mips64', just as we did in the days before 'from-abi'.  */
  if (strcasecmp (cpu_string, "from-abi") == 0)
    {
      if (ABI_NEEDS_32BIT_REGS (mips_abi))
	return mips_cpu_info_from_isa (ISA_MIPS1);

      if (ABI_NEEDS_64BIT_REGS (mips_abi))
	return mips_cpu_info_from_isa (ISA_MIPS3);

      if (file_mips_opts.gp >= 0)
	return mips_cpu_info_from_isa (file_mips_opts.gp == 32
				       ? ISA_MIPS1 : ISA_MIPS3);

      return mips_cpu_info_from_isa (MIPS_DEFAULT_64BIT
				     ? ISA_MIPS3
				     : ISA_MIPS1);
    }

  /* 'default' has traditionally been a no-op.  Probably not very useful.  */
  if (strcasecmp (cpu_string, "default") == 0)
    return 0;

  for (p = mips_cpu_info_table; p->name != 0; p++)
    if (mips_matching_cpu_name_p (p->name, cpu_string))
      return p;

  as_bad (_("bad value (%s) for %s"), cpu_string, option);
  return 0;
}

/* Return the canonical processor information for ISA (a member of the
   ISA_MIPS* enumeration).  */

static const struct mips_cpu_info *
mips_cpu_info_from_isa (int isa)
{
  int i;

  for (i = 0; mips_cpu_info_table[i].name != NULL; i++)
    if ((mips_cpu_info_table[i].flags & MIPS_CPU_IS_ISA)
	&& isa == mips_cpu_info_table[i].isa)
      return (&mips_cpu_info_table[i]);

  return NULL;
}

static const struct mips_cpu_info *
mips_cpu_info_from_arch (int arch)
{
  int i;

  for (i = 0; mips_cpu_info_table[i].name != NULL; i++)
    if (arch == mips_cpu_info_table[i].cpu)
      return (&mips_cpu_info_table[i]);

  return NULL;
}

static void
show (FILE *stream, const char *string, int *col_p, int *first_p)
{
  if (*first_p)
    {
      fprintf (stream, "%24s", "");
      *col_p = 24;
    }
  else
    {
      fprintf (stream, ", ");
      *col_p += 2;
    }

  if (*col_p + strlen (string) > 72)
    {
      fprintf (stream, "\n%24s", "");
      *col_p = 24;
    }

  fprintf (stream, "%s", string);
  *col_p += strlen (string);

  *first_p = 0;
}

void
md_show_usage (FILE *stream)
{
  int column, first;
  size_t i;

  fprintf (stream, _("\
MIPS options:\n\
-EB			generate big endian output\n\
-EL			generate little endian output\n\
-g, -g2			do not remove unneeded NOPs or swap branches\n\
-G NUM			allow referencing objects up to NUM bytes\n\
			implicitly with the gp register [default 8]\n"));
  fprintf (stream, _("\
-mips1			generate MIPS ISA I instructions\n\
-mips2			generate MIPS ISA II instructions\n\
-mips3			generate MIPS ISA III instructions\n\
-mips4			generate MIPS ISA IV instructions\n\
-mips5                  generate MIPS ISA V instructions\n\
-mips32                 generate MIPS32 ISA instructions\n\
-mips32r2               generate MIPS32 release 2 ISA instructions\n\
-mips32r3               generate MIPS32 release 3 ISA instructions\n\
-mips32r5               generate MIPS32 release 5 ISA instructions\n\
-mips32r6               generate MIPS32 release 6 ISA instructions\n\
-mips64                 generate MIPS64 ISA instructions\n\
-mips64r2               generate MIPS64 release 2 ISA instructions\n\
-mips64r3               generate MIPS64 release 3 ISA instructions\n\
-mips64r5               generate MIPS64 release 5 ISA instructions\n\
-mips64r6               generate MIPS64 release 6 ISA instructions\n\
-march=CPU/-mtune=CPU	generate code/schedule for CPU, where CPU is one of:\n"));

  first = 1;

  for (i = 0; mips_cpu_info_table[i].name != NULL; i++)
    show (stream, mips_cpu_info_table[i].name, &column, &first);
  show (stream, "from-abi", &column, &first);
  fputc ('\n', stream);

  fprintf (stream, _("\
-mCPU			equivalent to -march=CPU -mtune=CPU. Deprecated.\n\
-no-mCPU		don't generate code specific to CPU.\n\
			For -mCPU and -no-mCPU, CPU must be one of:\n"));

  first = 1;

  show (stream, "3900", &column, &first);
  show (stream, "4010", &column, &first);
  show (stream, "4100", &column, &first);
  show (stream, "4650", &column, &first);
  fputc ('\n', stream);

  fprintf (stream, _("\
-mips16			generate mips16 instructions\n\
-no-mips16		do not generate mips16 instructions\n"));
  fprintf (stream, _("\
-mmips16e2		generate MIPS16e2 instructions\n\
-mno-mips16e2		do not generate MIPS16e2 instructions\n"));
  fprintf (stream, _("\
-mmicromips		generate microMIPS instructions\n\
-mno-micromips		do not generate microMIPS instructions\n"));
  fprintf (stream, _("\
-msmartmips		generate smartmips instructions\n\
-mno-smartmips		do not generate smartmips instructions\n"));
  fprintf (stream, _("\
-mdsp			generate DSP instructions\n\
-mno-dsp		do not generate DSP instructions\n"));
  fprintf (stream, _("\
-mdspr2			generate DSP R2 instructions\n\
-mno-dspr2		do not generate DSP R2 instructions\n"));
  fprintf (stream, _("\
-mdspr3			generate DSP R3 instructions\n\
-mno-dspr3		do not generate DSP R3 instructions\n"));
  fprintf (stream, _("\
-mmt			generate MT instructions\n\
-mno-mt			do not generate MT instructions\n"));
  fprintf (stream, _("\
-mmcu			generate MCU instructions\n\
-mno-mcu		do not generate MCU instructions\n"));
  fprintf (stream, _("\
-mmsa			generate MSA instructions\n\
-mno-msa		do not generate MSA instructions\n"));
  fprintf (stream, _("\
-mxpa			generate eXtended Physical Address (XPA) instructions\n\
-mno-xpa		do not generate eXtended Physical Address (XPA) instructions\n"));
  fprintf (stream, _("\
-mvirt			generate Virtualization instructions\n\
-mno-virt		do not generate Virtualization instructions\n"));
  fprintf (stream, _("\
-mcrc			generate CRC instructions\n\
-mno-crc		do not generate CRC instructions\n"));
  fprintf (stream, _("\
-mginv			generate Global INValidate (GINV) instructions\n\
-mno-ginv		do not generate Global INValidate instructions\n"));
  fprintf (stream, _("\
-mloongson-mmi		generate Loongson MultiMedia extensions Instructions (MMI) instructions\n\
-mno-loongson-mmi	do not generate Loongson MultiMedia extensions Instructions\n"));
  fprintf (stream, _("\
-mloongson-cam		generate Loongson Content Address Memory (CAM) instructions\n\
-mno-loongson-cam	do not generate Loongson Content Address Memory Instructions\n"));
  fprintf (stream, _("\
-mloongson-ext		generate Loongson EXTensions (EXT) instructions\n\
-mno-loongson-ext	do not generate Loongson EXTensions Instructions\n"));
  fprintf (stream, _("\
-mloongson-ext2		generate Loongson EXTensions R2 (EXT2) instructions\n\
-mno-loongson-ext2	do not generate Loongson EXTensions R2 Instructions\n"));
  fprintf (stream, _("\
-minsn32		only generate 32-bit microMIPS instructions\n\
-mno-insn32		generate all microMIPS instructions\n"));
#if DEFAULT_MIPS_FIX_LOONGSON3_LLSC
  fprintf (stream, _("\
-mfix-loongson3-llsc	work around Loongson3 LL/SC errata, default\n\
-mno-fix-loongson3-llsc	disable work around Loongson3 LL/SC errata\n"));
#else
  fprintf (stream, _("\
-mfix-loongson3-llsc	work around Loongson3 LL/SC errata\n\
-mno-fix-loongson3-llsc	disable work around Loongson3 LL/SC errata, default\n"));
#endif
  fprintf (stream, _("\
-mfix-loongson2f-jump	work around Loongson2F JUMP instructions\n\
-mfix-loongson2f-nop	work around Loongson2F NOP errata\n\
-mfix-loongson3-llsc	work around Loongson3 LL/SC errata\n\
-mno-fix-loongson3-llsc	disable work around Loongson3 LL/SC errata\n\
-mfix-vr4120		work around certain VR4120 errata\n\
-mfix-vr4130		work around VR4130 mflo/mfhi errata\n\
-mfix-24k		insert a nop after ERET and DERET instructions\n\
-mfix-cn63xxp1		work around CN63XXP1 PREF errata\n\
-mfix-r5900		work around R5900 short loop errata\n\
-mgp32			use 32-bit GPRs, regardless of the chosen ISA\n\
-mfp32			use 32-bit FPRs, regardless of the chosen ISA\n\
-msym32			assume all symbols have 32-bit values\n\
-O0			do not remove unneeded NOPs, do not swap branches\n\
-O, -O1			remove unneeded NOPs, do not swap branches\n\
-O2			remove unneeded NOPs and swap branches\n\
--trap, --no-break	trap exception on div by 0 and mult overflow\n\
--break, --no-trap	break exception on div by 0 and mult overflow\n"));
  fprintf (stream, _("\
-mhard-float		allow floating-point instructions\n\
-msoft-float		do not allow floating-point instructions\n\
-msingle-float		only allow 32-bit floating-point operations\n\
-mdouble-float		allow 32-bit and 64-bit floating-point operations\n\
--[no-]construct-floats	[dis]allow floating point values to be constructed\n\
--[no-]relax-branch	[dis]allow out-of-range branches to be relaxed\n\
-mignore-branch-isa	accept invalid branches requiring an ISA mode switch\n\
-mno-ignore-branch-isa	reject invalid branches requiring an ISA mode switch\n\
-mnan=ENCODING		select an IEEE 754 NaN encoding convention, either of:\n"));

  first = 1;

  show (stream, "legacy", &column, &first);
  show (stream, "2008", &column, &first);

  fputc ('\n', stream);

  fprintf (stream, _("\
-KPIC, -call_shared	generate SVR4 position independent code\n\
-call_nonpic		generate non-PIC code that can operate with DSOs\n\
-mvxworks-pic		generate VxWorks position independent code\n\
-non_shared		do not generate code that can operate with DSOs\n\
-xgot			assume a 32 bit GOT\n\
-mpdr, -mno-pdr		enable/disable creation of .pdr sections\n\
-mshared, -mno-shared   disable/enable .cpload optimization for\n\
                        position dependent (non shared) code\n\
-mabi=ABI		create ABI conformant object file for:\n"));

  first = 1;

  show (stream, "32", &column, &first);
  show (stream, "o64", &column, &first);
  show (stream, "n32", &column, &first);
  show (stream, "64", &column, &first);
  show (stream, "eabi", &column, &first);

  fputc ('\n', stream);

  fprintf (stream, _("\
-32			create o32 ABI object file%s\n"),
	   MIPS_DEFAULT_ABI == O32_ABI ? _(" (default)") : "");
  fprintf (stream, _("\
-n32			create n32 ABI object file%s\n"),
	   MIPS_DEFAULT_ABI == N32_ABI ? _(" (default)") : "");
  fprintf (stream, _("\
-64			create 64 ABI object file%s\n"),
	   MIPS_DEFAULT_ABI == N64_ABI ? _(" (default)") : "");
}

#ifdef TE_IRIX
enum dwarf2_format
mips_dwarf2_format (asection *sec ATTRIBUTE_UNUSED)
{
  if (HAVE_64BIT_SYMBOLS)
    return dwarf2_format_64bit_irix;
  else
    return dwarf2_format_32bit;
}
#endif

int
mips_dwarf2_addr_size (void)
{
  if (HAVE_64BIT_OBJECTS)
    return 8;
  else
    return 4;
}

/* Standard calling conventions leave the CFA at SP on entry.  */
void
mips_cfi_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa_register (SP);
}

int
tc_mips_regname_to_dw2regnum (char *regname)
{
  unsigned int regnum = -1;
  unsigned int reg;

  if (reg_lookup (&regname, RTYPE_GP | RTYPE_NUM, &reg))
    regnum = reg;

  return regnum;
}

/* Implement CONVERT_SYMBOLIC_ATTRIBUTE.
   Given a symbolic attribute NAME, return the proper integer value.
   Returns -1 if the attribute is not known.  */

int
mips_convert_symbolic_attribute (const char *name)
{
  static const struct
  {
    const char * name;
    const int    tag;
  }
  attribute_table[] =
    {
#define T(tag) {#tag, tag}
      T (Tag_GNU_MIPS_ABI_FP),
      T (Tag_GNU_MIPS_ABI_MSA),
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

void
mips_md_finish (void)
{
  int fpabi = Val_GNU_MIPS_ABI_FP_ANY;

  mips_emit_delays ();
  if (cur_proc_ptr)
    as_warn (_("missing .end at end of assembly"));

  /* Just in case no code was emitted, do the consistency check.  */
  file_mips_check_options ();

  /* Set a floating-point ABI if the user did not.  */
  if (obj_elf_seen_attribute (OBJ_ATTR_GNU, Tag_GNU_MIPS_ABI_FP))
    {
      /* Perform consistency checks on the floating-point ABI.  */
      fpabi = bfd_elf_get_obj_attr_int (stdoutput, OBJ_ATTR_GNU,
					Tag_GNU_MIPS_ABI_FP);
      if (fpabi != Val_GNU_MIPS_ABI_FP_ANY)
	check_fpabi (fpabi);
    }
  else
    {
      /* Soft-float gets precedence over single-float, the two options should
         not be used together so this should not matter.  */
      if (file_mips_opts.soft_float == 1)
	fpabi = Val_GNU_MIPS_ABI_FP_SOFT;
      /* Single-float gets precedence over all double_float cases.  */
      else if (file_mips_opts.single_float == 1)
	fpabi = Val_GNU_MIPS_ABI_FP_SINGLE;
      else
	{
	  switch (file_mips_opts.fp)
	    {
	    case 32:
	      if (file_mips_opts.gp == 32)
		fpabi = Val_GNU_MIPS_ABI_FP_DOUBLE;
	      break;
	    case 0:
	      fpabi = Val_GNU_MIPS_ABI_FP_XX;
	      break;
	    case 64:
	      if (file_mips_opts.gp == 32 && !file_mips_opts.oddspreg)
		fpabi = Val_GNU_MIPS_ABI_FP_64A;
	      else if (file_mips_opts.gp == 32)
		fpabi = Val_GNU_MIPS_ABI_FP_64;
	      else
		fpabi = Val_GNU_MIPS_ABI_FP_DOUBLE;
	      break;
	    }
	}

      bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_GNU,
				Tag_GNU_MIPS_ABI_FP, fpabi);
    }
}

/*  Returns the relocation type required for a particular CFI encoding.  */

bfd_reloc_code_real_type
mips_cfi_reloc_for_encoding (int encoding)
{
  if (encoding == (DW_EH_PE_sdata4 | DW_EH_PE_pcrel))
    return BFD_RELOC_32_PCREL;
  else return BFD_RELOC_NONE;
}
