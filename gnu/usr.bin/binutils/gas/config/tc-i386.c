/* tc-i386.c -- Assemble code for the Intel 80386
   Copyright (C) 1989-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

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

/* Intel 80386 machine specific gas.
   Written by Eliot Dresselhaus (eliot@mgm.mit.edu).
   x86_64 support by Jan Hubicka (jh@suse.cz)
   VIA PadLock support by Michal Ludvig (mludvig@suse.cz)
   Bugs & suggestions are completely welcome.  This is free software.
   Please help us make it better.  */

#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "dwarf2dbg.h"
#include "dw2gencfi.h"
#include "gen-sframe.h"
#include "sframe.h"
#include "elf/x86-64.h"
#include "opcodes/i386-init.h"
#include "opcodes/i386-mnem.h"
#include <limits.h>

#ifndef INFER_ADDR_PREFIX
#define INFER_ADDR_PREFIX 1
#endif

#ifndef DEFAULT_ARCH
#define DEFAULT_ARCH "i386"
#endif

#ifndef INLINE
#if __GNUC__ >= 2
#define INLINE __inline__
#else
#define INLINE
#endif
#endif

/* Prefixes will be emitted in the order defined below.
   WAIT_PREFIX must be the first prefix since FWAIT is really is an
   instruction, and so must come before any prefixes.
   The preferred prefix order is SEG_PREFIX, ADDR_PREFIX, DATA_PREFIX,
   REP_PREFIX/HLE_PREFIX, LOCK_PREFIX.  */
#define WAIT_PREFIX	0
#define SEG_PREFIX	1
#define ADDR_PREFIX	2
#define DATA_PREFIX	3
#define REP_PREFIX	4
#define HLE_PREFIX	REP_PREFIX
#define BND_PREFIX	REP_PREFIX
#define LOCK_PREFIX	5
#define REX_PREFIX	6       /* must come last.  */
#define MAX_PREFIXES	7	/* max prefixes per opcode */

/* we define the syntax here (modulo base,index,scale syntax) */
#define REGISTER_PREFIX '%'
#define IMMEDIATE_PREFIX '$'
#define ABSOLUTE_PREFIX '*'

/* these are the instruction mnemonic suffixes in AT&T syntax or
   memory operand size in Intel syntax.  */
#define WORD_MNEM_SUFFIX  'w'
#define BYTE_MNEM_SUFFIX  'b'
#define SHORT_MNEM_SUFFIX 's'
#define LONG_MNEM_SUFFIX  'l'
#define QWORD_MNEM_SUFFIX  'q'

#define END_OF_INSN '\0'

#define OPERAND_TYPE_NONE { .bitfield = { .class = ClassNone } }

/* This matches the C -> StaticRounding alias in the opcode table.  */
#define commutative staticrounding

/*
  'templates' is for grouping together 'template' structures for opcodes
  of the same name.  This is only used for storing the insns in the grand
  ole hash table of insns.
  The templates themselves start at START and range up to (but not including)
  END.
  */
typedef struct
{
  const insn_template *start;
  const insn_template *end;
}
templates;

/* 386 operand encoding bytes:  see 386 book for details of this.  */
typedef struct
{
  unsigned int regmem;	/* codes register or memory operand */
  unsigned int reg;	/* codes register operand (or extended opcode) */
  unsigned int mode;	/* how to interpret regmem & reg */
}
modrm_byte;

/* x86-64 extension prefix.  */
typedef int rex_byte;

/* 386 opcode byte to code indirect addressing.  */
typedef struct
{
  unsigned base;
  unsigned index;
  unsigned scale;
}
sib_byte;

/* x86 arch names, types and features */
typedef struct
{
  const char *name;		/* arch name */
  unsigned int len:8;		/* arch string length */
  bool skip:1;			/* show_arch should skip this. */
  enum processor_type type;	/* arch type */
  i386_cpu_flags enable;		/* cpu feature enable flags */
  i386_cpu_flags disable;	/* cpu feature disable flags */
}
arch_entry;

static void update_code_flag (int, int);
static void s_insn (int);
static void set_code_flag (int);
static void set_16bit_gcc_code_flag (int);
static void set_intel_syntax (int);
static void set_intel_mnemonic (int);
static void set_allow_index_reg (int);
static void set_check (int);
static void set_cpu_arch (int);
#ifdef TE_PE
static void pe_directive_secrel (int);
static void pe_directive_secidx (int);
#endif
static void signed_cons (int);
static char *output_invalid (int c);
static int i386_finalize_immediate (segT, expressionS *, i386_operand_type,
				    const char *);
static int i386_finalize_displacement (segT, expressionS *, i386_operand_type,
				       const char *);
static int i386_att_operand (char *);
static int i386_intel_operand (char *, int);
static int i386_intel_simplify (expressionS *);
static int i386_intel_parse_name (const char *, expressionS *);
static const reg_entry *parse_register (const char *, char **);
static const char *parse_insn (const char *, char *, bool);
static char *parse_operands (char *, const char *);
static void swap_operands (void);
static void swap_2_operands (unsigned int, unsigned int);
static enum flag_code i386_addressing_mode (void);
static void optimize_imm (void);
static bool optimize_disp (const insn_template *t);
static const insn_template *match_template (char);
static int check_string (void);
static int process_suffix (void);
static int check_byte_reg (void);
static int check_long_reg (void);
static int check_qword_reg (void);
static int check_word_reg (void);
static int finalize_imm (void);
static int process_operands (void);
static const reg_entry *build_modrm_byte (void);
static void output_insn (void);
static void output_imm (fragS *, offsetT);
static void output_disp (fragS *, offsetT);
#ifndef I386COFF
static void s_bss (int);
#endif
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
static void handle_large_common (int small ATTRIBUTE_UNUSED);

/* GNU_PROPERTY_X86_ISA_1_USED.  */
static unsigned int x86_isa_1_used;
/* GNU_PROPERTY_X86_FEATURE_2_USED.  */
static unsigned int x86_feature_2_used;
/* Generate x86 used ISA and feature properties.  */
static unsigned int x86_used_note = DEFAULT_X86_USED_NOTE;
#endif

static const char *default_arch = DEFAULT_ARCH;

/* parse_register() returns this when a register alias cannot be used.  */
static const reg_entry bad_reg = { "<bad>", OPERAND_TYPE_NONE, 0, 0,
				   { Dw2Inval, Dw2Inval } };

static const reg_entry *reg_eax;
static const reg_entry *reg_ds;
static const reg_entry *reg_es;
static const reg_entry *reg_ss;
static const reg_entry *reg_st0;
static const reg_entry *reg_k0;

/* VEX prefix.  */
typedef struct
{
  /* VEX prefix is either 2 byte or 3 byte.  EVEX is 4 byte.  */
  unsigned char bytes[4];
  unsigned int length;
  /* Destination or source register specifier.  */
  const reg_entry *register_specifier;
} vex_prefix;

/* 'md_assemble ()' gathers together information and puts it into a
   i386_insn.  */

union i386_op
  {
    expressionS *disps;
    expressionS *imms;
    const reg_entry *regs;
  };

enum i386_error
  {
    no_error, /* Must be first.  */
    operand_size_mismatch,
    operand_type_mismatch,
    register_type_mismatch,
    number_of_operands_mismatch,
    invalid_instruction_suffix,
    bad_imm4,
    unsupported_with_intel_mnemonic,
    unsupported_syntax,
    unsupported,
    unsupported_on_arch,
    unsupported_64bit,
    invalid_sib_address,
    invalid_vsib_address,
    invalid_vector_register_set,
    invalid_tmm_register_set,
    invalid_dest_and_src_register_set,
    unsupported_vector_index_register,
    unsupported_broadcast,
    broadcast_needed,
    unsupported_masking,
    mask_not_on_destination,
    no_default_mask,
    unsupported_rc_sae,
    invalid_register_operand,
  };

struct _i386_insn
  {
    /* TM holds the template for the insn were currently assembling.  */
    insn_template tm;

    /* SUFFIX holds the instruction size suffix for byte, word, dword
       or qword, if given.  */
    char suffix;

    /* OPCODE_LENGTH holds the number of base opcode bytes.  */
    unsigned char opcode_length;

    /* OPERANDS gives the number of given operands.  */
    unsigned int operands;

    /* REG_OPERANDS, DISP_OPERANDS, MEM_OPERANDS, IMM_OPERANDS give the number
       of given register, displacement, memory operands and immediate
       operands.  */
    unsigned int reg_operands, disp_operands, mem_operands, imm_operands;

    /* TYPES [i] is the type (see above #defines) which tells us how to
       use OP[i] for the corresponding operand.  */
    i386_operand_type types[MAX_OPERANDS];

    /* Displacement expression, immediate expression, or register for each
       operand.  */
    union i386_op op[MAX_OPERANDS];

    /* Flags for operands.  */
    unsigned int flags[MAX_OPERANDS];
#define Operand_PCrel 1
#define Operand_Mem   2
#define Operand_Signed 4 /* .insn only */

    /* Relocation type for operand */
    enum bfd_reloc_code_real reloc[MAX_OPERANDS];

    /* BASE_REG, INDEX_REG, and LOG2_SCALE_FACTOR are used to encode
       the base index byte below.  */
    const reg_entry *base_reg;
    const reg_entry *index_reg;
    unsigned int log2_scale_factor;

    /* SEG gives the seg_entries of this insn.  They are zero unless
       explicit segment overrides are given.  */
    const reg_entry *seg[2];

    /* PREFIX holds all the given prefix opcodes (usually null).
       PREFIXES is the number of prefix opcodes.  */
    unsigned int prefixes;
    unsigned char prefix[MAX_PREFIXES];

    /* .insn allows for reserved opcode spaces.  */
    unsigned char insn_opcode_space;

    /* .insn also allows (requires) specifying immediate size.  */
    unsigned char imm_bits[MAX_OPERANDS];

    /* Register is in low 3 bits of opcode.  */
    bool short_form;

    /* The operand to a branch insn indicates an absolute branch.  */
    bool jumpabsolute;

    /* The operand to a branch insn indicates a far branch.  */
    bool far_branch;

    /* There is a memory operand of (%dx) which should be only used
       with input/output instructions.  */
    bool input_output_operand;

    /* Extended states.  */
    enum
      {
	/* Use MMX state.  */
	xstate_mmx = 1 << 0,
	/* Use XMM state.  */
	xstate_xmm = 1 << 1,
	/* Use YMM state.  */
	xstate_ymm = 1 << 2 | xstate_xmm,
	/* Use ZMM state.  */
	xstate_zmm = 1 << 3 | xstate_ymm,
	/* Use TMM state.  */
	xstate_tmm = 1 << 4,
	/* Use MASK state.  */
	xstate_mask = 1 << 5
      } xstate;

    /* Has GOTPC or TLS relocation.  */
    bool has_gotpc_tls_reloc;

    /* RM and SIB are the modrm byte and the sib byte where the
       addressing modes of this insn are encoded.  */
    modrm_byte rm;
    rex_byte rex;
    rex_byte vrex;
    sib_byte sib;
    vex_prefix vex;

    /* Masking attributes.

       The struct describes masking, applied to OPERAND in the instruction.
       REG is a pointer to the corresponding mask register.  ZEROING tells
       whether merging or zeroing mask is used.  */
    struct Mask_Operation
    {
      const reg_entry *reg;
      unsigned int zeroing;
      /* The operand where this operation is associated.  */
      unsigned int operand;
    } mask;

    /* Rounding control and SAE attributes.  */
    struct RC_Operation
    {
      enum rc_type
	{
	  rc_none = -1,
	  rne,
	  rd,
	  ru,
	  rz,
	  saeonly
	} type;
      /* In Intel syntax the operand modifier form is supposed to be used, but
	 we continue to accept the immediate forms as well.  */
      bool modifier;
    } rounding;

    /* Broadcasting attributes.

       The struct describes broadcasting, applied to OPERAND.  TYPE is
       expresses the broadcast factor.  */
    struct Broadcast_Operation
    {
      /* Type of broadcast: {1to2}, {1to4}, {1to8}, {1to16} or {1to32}.  */
      unsigned int type;

      /* Index of broadcasted operand.  */
      unsigned int operand;

      /* Number of bytes to broadcast.  */
      unsigned int bytes;
    } broadcast;

    /* Compressed disp8*N attribute.  */
    unsigned int memshift;

    /* Prefer load or store in encoding.  */
    enum
      {
	dir_encoding_default = 0,
	dir_encoding_load,
	dir_encoding_store,
	dir_encoding_swap
      } dir_encoding;

    /* Prefer 8bit, 16bit, 32bit displacement in encoding.  */
    enum
      {
	disp_encoding_default = 0,
	disp_encoding_8bit,
	disp_encoding_16bit,
	disp_encoding_32bit
      } disp_encoding;

    /* Prefer the REX byte in encoding.  */
    bool rex_encoding;

    /* Disable instruction size optimization.  */
    bool no_optimize;

    /* How to encode vector instructions.  */
    enum
      {
	vex_encoding_default = 0,
	vex_encoding_vex,
	vex_encoding_vex3,
	vex_encoding_evex,
	vex_encoding_error
      } vec_encoding;

    /* REP prefix.  */
    const char *rep_prefix;

    /* HLE prefix.  */
    const char *hle_prefix;

    /* Have BND prefix.  */
    const char *bnd_prefix;

    /* Have NOTRACK prefix.  */
    const char *notrack_prefix;

    /* Error message.  */
    enum i386_error error;
  };

typedef struct _i386_insn i386_insn;

/* Link RC type with corresponding string, that'll be looked for in
   asm.  */
struct RC_name
{
  enum rc_type type;
  const char *name;
  unsigned int len;
};

static const struct RC_name RC_NamesTable[] =
{
  {  rne, STRING_COMMA_LEN ("rn-sae") },
  {  rd,  STRING_COMMA_LEN ("rd-sae") },
  {  ru,  STRING_COMMA_LEN ("ru-sae") },
  {  rz,  STRING_COMMA_LEN ("rz-sae") },
  {  saeonly,  STRING_COMMA_LEN ("sae") },
};

/* To be indexed by segment register number.  */
static const unsigned char i386_seg_prefixes[] = {
  ES_PREFIX_OPCODE,
  CS_PREFIX_OPCODE,
  SS_PREFIX_OPCODE,
  DS_PREFIX_OPCODE,
  FS_PREFIX_OPCODE,
  GS_PREFIX_OPCODE
};

/* List of chars besides those in app.c:symbol_chars that can start an
   operand.  Used to prevent the scrubber eating vital white-space.  */
const char extra_symbol_chars[] = "*%-([{}"
#ifdef LEX_AT
	"@"
#endif
#ifdef LEX_QM
	"?"
#endif
	;

#if ((defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF))	\
     && !defined (TE_GNU)				\
     && !defined (TE_LINUX)				\
     && !defined (TE_Haiku)				\
     && !defined (TE_FreeBSD)				\
     && !defined (TE_DragonFly)				\
     && !defined (TE_NetBSD))
/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  The option
   --divide will remove '/' from this list.  */
const char *i386_comment_chars = "#/";
#define SVR4_COMMENT_CHARS 1
#define PREFIX_SEPARATOR '\\'

#else
const char *i386_comment_chars = "#";
#define PREFIX_SEPARATOR '/'
#endif

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.
   Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.
   Also note that comments started like this one will always work if
   '/' isn't otherwise defined.  */
const char line_comment_chars[] = "#/";

const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant from exp in floating point
   nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant
   As in 0f12.456
   or    0d1.2345e12.  */
const char FLT_CHARS[] = "fFdDxXhHbB";

/* Tables for lexical analysis.  */
static char mnemonic_chars[256];
static char register_chars[256];
static char operand_chars[256];

/* Lexical macros.  */
#define is_operand_char(x) (operand_chars[(unsigned char) x])
#define is_register_char(x) (register_chars[(unsigned char) x])
#define is_space_char(x) ((x) == ' ')

/* All non-digit non-letter characters that may occur in an operand and
   which aren't already in extra_symbol_chars[].  */
static const char operand_special_chars[] = "$+,)._~/<>|&^!=:@]";

/* md_assemble() always leaves the strings it's passed unaltered.  To
   effect this we maintain a stack of saved characters that we've smashed
   with '\0's (indicating end of strings for various sub-fields of the
   assembler instruction).  */
static char save_stack[32];
static char *save_stack_p;
#define END_STRING_AND_SAVE(s) \
	do { *save_stack_p++ = *(s); *(s) = '\0'; } while (0)
#define RESTORE_END_STRING(s) \
	do { *(s) = *--save_stack_p; } while (0)

/* The instruction we're assembling.  */
static i386_insn i;

/* Possible templates for current insn.  */
static const templates *current_templates;

/* Per instruction expressionS buffers: max displacements & immediates.  */
static expressionS disp_expressions[MAX_MEMORY_OPERANDS];
static expressionS im_expressions[MAX_IMMEDIATE_OPERANDS];

/* Current operand we are working on.  */
static int this_operand = -1;

/* Are we processing a .insn directive?  */
#define dot_insn() (i.tm.mnem_off == MN__insn)

/* We support four different modes.  FLAG_CODE variable is used to distinguish
   these.  */

enum flag_code {
	CODE_32BIT,
	CODE_16BIT,
	CODE_64BIT };

static enum flag_code flag_code;
static unsigned int object_64bit;
static unsigned int disallow_64bit_reloc;
static int use_rela_relocations = 0;
/* __tls_get_addr/___tls_get_addr symbol for TLS.  */
static const char *tls_get_addr;

#if ((defined (OBJ_MAYBE_COFF) && defined (OBJ_MAYBE_AOUT)) \
     || defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF) \
     || defined (TE_PE) || defined (TE_PEP) || defined (OBJ_MACH_O))

/* The ELF ABI to use.  */
enum x86_elf_abi
{
  I386_ABI,
  X86_64_ABI,
  X86_64_X32_ABI
};

static enum x86_elf_abi x86_elf_abi = I386_ABI;
#endif

#if defined (TE_PE) || defined (TE_PEP)
/* Use big object file format.  */
static int use_big_obj = 0;
#endif

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
/* 1 if generating code for a shared library.  */
static int shared = 0;

unsigned int x86_sframe_cfa_sp_reg;
/* The other CFA base register for SFrame stack trace info.  */
unsigned int x86_sframe_cfa_fp_reg;
unsigned int x86_sframe_cfa_ra_reg;

#endif

/* 1 for intel syntax,
   0 if att syntax.  */
static int intel_syntax = 0;

static enum x86_64_isa
{
  amd64 = 1,	/* AMD64 ISA.  */
  intel64	/* Intel64 ISA.  */
} isa64;

/* 1 for intel mnemonic,
   0 if att mnemonic.  */
static int intel_mnemonic = !SYSV386_COMPAT;

/* 1 if pseudo registers are permitted.  */
static int allow_pseudo_reg = 0;

/* 1 if register prefix % not required.  */
static int allow_naked_reg = 0;

/* 1 if the assembler should add BND prefix for all control-transferring
   instructions supporting it, even if this prefix wasn't specified
   explicitly.  */
static int add_bnd_prefix = 0;

/* 1 if pseudo index register, eiz/riz, is allowed .  */
static int allow_index_reg = 0;

/* 1 if the assembler should ignore LOCK prefix, even if it was
   specified explicitly.  */
static int omit_lock_prefix = 0;

/* 1 if the assembler should encode lfence, mfence, and sfence as
   "lock addl $0, (%{re}sp)".  */
static int avoid_fence = 0;

/* 1 if lfence should be inserted after every load.  */
static int lfence_after_load = 0;

/* Non-zero if lfence should be inserted before indirect branch.  */
static enum lfence_before_indirect_branch_kind
  {
    lfence_branch_none = 0,
    lfence_branch_register,
    lfence_branch_memory,
    lfence_branch_all
  }
lfence_before_indirect_branch;

/* Non-zero if lfence should be inserted before ret.  */
static enum lfence_before_ret_kind
  {
    lfence_before_ret_none = 0,
    lfence_before_ret_not,
    lfence_before_ret_or,
    lfence_before_ret_shl
  }
lfence_before_ret;

/* Types of previous instruction is .byte or prefix.  */
static struct
  {
    segT seg;
    const char *file;
    const char *name;
    unsigned int line;
    enum last_insn_kind
      {
	last_insn_other = 0,
	last_insn_directive,
	last_insn_prefix
      } kind;
  } last_insn;

/* 1 if the assembler should generate relax relocations.  */

static int generate_relax_relocations
  = DEFAULT_GENERATE_X86_RELAX_RELOCATIONS;

static enum check_kind
  {
    check_none = 0,
    check_warning,
    check_error
  }
sse_check, operand_check = check_warning;

/* Non-zero if branches should be aligned within power of 2 boundary.  */
static int align_branch_power = 0;

/* Types of branches to align.  */
enum align_branch_kind
  {
    align_branch_none = 0,
    align_branch_jcc = 1,
    align_branch_fused = 2,
    align_branch_jmp = 3,
    align_branch_call = 4,
    align_branch_indirect = 5,
    align_branch_ret = 6
  };

/* Type bits of branches to align.  */
enum align_branch_bit
  {
    align_branch_jcc_bit = 1 << align_branch_jcc,
    align_branch_fused_bit = 1 << align_branch_fused,
    align_branch_jmp_bit = 1 << align_branch_jmp,
    align_branch_call_bit = 1 << align_branch_call,
    align_branch_indirect_bit = 1 << align_branch_indirect,
    align_branch_ret_bit = 1 << align_branch_ret
  };

static unsigned int align_branch = (align_branch_jcc_bit
				    | align_branch_fused_bit
				    | align_branch_jmp_bit);

/* Types of condition jump used by macro-fusion.  */
enum mf_jcc_kind
  {
    mf_jcc_jo = 0,  /* base opcode 0x70  */
    mf_jcc_jc,      /* base opcode 0x72  */
    mf_jcc_je,      /* base opcode 0x74  */
    mf_jcc_jna,     /* base opcode 0x76  */
    mf_jcc_js,      /* base opcode 0x78  */
    mf_jcc_jp,      /* base opcode 0x7a  */
    mf_jcc_jl,      /* base opcode 0x7c  */
    mf_jcc_jle,     /* base opcode 0x7e  */
  };

/* Types of compare flag-modifying insntructions used by macro-fusion.  */
enum mf_cmp_kind
  {
    mf_cmp_test_and,  /* test/cmp */
    mf_cmp_alu_cmp,  /* add/sub/cmp */
    mf_cmp_incdec  /* inc/dec */
  };

/* The maximum padding size for fused jcc.  CMP like instruction can
   be 9 bytes and jcc can be 6 bytes.  Leave room just in case for
   prefixes.   */
#define MAX_FUSED_JCC_PADDING_SIZE 20

/* The maximum number of prefixes added for an instruction.  */
static unsigned int align_branch_prefix_size = 5;

/* Optimization:
   1. Clear the REX_W bit with register operand if possible.
   2. Above plus use 128bit vector instruction to clear the full vector
      register.
 */
static int optimize = 0;

/* Optimization:
   1. Clear the REX_W bit with register operand if possible.
   2. Above plus use 128bit vector instruction to clear the full vector
      register.
   3. Above plus optimize "test{q,l,w} $imm8,%r{64,32,16}" to
      "testb $imm7,%r8".
 */
static int optimize_for_space = 0;

/* Register prefix used for error message.  */
static const char *register_prefix = "%";

/* Used in 16 bit gcc mode to add an l suffix to call, ret, enter,
   leave, push, and pop instructions so that gcc has the same stack
   frame as in 32 bit mode.  */
static char stackop_size = '\0';

/* Non-zero to optimize code alignment.  */
int optimize_align_code = 1;

/* Non-zero to quieten some warnings.  */
static int quiet_warnings = 0;

/* Guard to avoid repeated warnings about non-16-bit code on 16-bit CPUs.  */
static bool pre_386_16bit_warned;

/* CPU name.  */
static const char *cpu_arch_name = NULL;
static char *cpu_sub_arch_name = NULL;

/* CPU feature flags.  */
static i386_cpu_flags cpu_arch_flags = CPU_UNKNOWN_FLAGS;

/* If we have selected a cpu we are generating instructions for.  */
static int cpu_arch_tune_set = 0;

/* Cpu we are generating instructions for.  */
enum processor_type cpu_arch_tune = PROCESSOR_UNKNOWN;

/* CPU feature flags of cpu we are generating instructions for.  */
static i386_cpu_flags cpu_arch_tune_flags;

/* CPU instruction set architecture used.  */
enum processor_type cpu_arch_isa = PROCESSOR_UNKNOWN;

/* CPU feature flags of instruction set architecture used.  */
i386_cpu_flags cpu_arch_isa_flags;

/* If set, conditional jumps are not automatically promoted to handle
   larger than a byte offset.  */
static bool no_cond_jump_promotion = false;

/* This will be set from an expression parser hook if there's any
   applicable operator involved in an expression.  */
static enum {
  expr_operator_none,
  expr_operator_present,
  expr_large_value,
} expr_mode;

/* Encode SSE instructions with VEX prefix.  */
static unsigned int sse2avx;

/* Encode aligned vector move as unaligned vector move.  */
static unsigned int use_unaligned_vector_move;

/* Encode scalar AVX instructions with specific vector length.  */
static enum
  {
    vex128 = 0,
    vex256
  } avxscalar;

/* Encode VEX WIG instructions with specific vex.w.  */
static enum
  {
    vexw0 = 0,
    vexw1
  } vexwig;

/* Encode scalar EVEX LIG instructions with specific vector length.  */
static enum
  {
    evexl128 = 0,
    evexl256,
    evexl512
  } evexlig;

/* Encode EVEX WIG instructions with specific evex.w.  */
static enum
  {
    evexw0 = 0,
    evexw1
  } evexwig;

/* Value to encode in EVEX RC bits, for SAE-only instructions.  */
static enum rc_type evexrcig = rne;

/* Pre-defined "_GLOBAL_OFFSET_TABLE_".  */
static symbolS *GOT_symbol;

/* The dwarf2 return column, adjusted for 32 or 64 bit.  */
unsigned int x86_dwarf2_return_column;

/* The dwarf2 data alignment, adjusted for 32 or 64 bit.  */
int x86_cie_data_alignment;

/* Interface to relax_segment.
   There are 3 major relax states for 386 jump insns because the
   different types of jumps add different sizes to frags when we're
   figuring out what sort of jump to choose to reach a given label.

   BRANCH_PADDING, BRANCH_PREFIX and FUSED_JCC_PADDING are used to align
   branches which are handled by md_estimate_size_before_relax() and
   i386_generic_table_relax_frag().  */

/* Types.  */
#define UNCOND_JUMP 0
#define COND_JUMP 1
#define COND_JUMP86 2
#define BRANCH_PADDING 3
#define BRANCH_PREFIX 4
#define FUSED_JCC_PADDING 5

/* Sizes.  */
#define CODE16	1
#define SMALL	0
#define SMALL16 (SMALL | CODE16)
#define BIG	2
#define BIG16	(BIG | CODE16)

#ifndef INLINE
#ifdef __GNUC__
#define INLINE __inline__
#else
#define INLINE
#endif
#endif

#define ENCODE_RELAX_STATE(type, size) \
  ((relax_substateT) (((type) << 2) | (size)))
#define TYPE_FROM_RELAX_STATE(s) \
  ((s) >> 2)
#define DISP_SIZE_FROM_RELAX_STATE(s) \
    ((((s) & 3) == BIG ? 4 : (((s) & 3) == BIG16 ? 2 : 1)))

/* This table is used by relax_frag to promote short jumps to long
   ones where necessary.  SMALL (short) jumps may be promoted to BIG
   (32 bit long) ones, and SMALL16 jumps to BIG16 (16 bit long).  We
   don't allow a short jump in a 32 bit code segment to be promoted to
   a 16 bit offset jump because it's slower (requires data size
   prefix), and doesn't work, unless the destination is in the bottom
   64k of the code segment (The top 16 bits of eip are zeroed).  */

const relax_typeS md_relax_table[] =
{
  /* The fields are:
     1) most positive reach of this state,
     2) most negative reach of this state,
     3) how many bytes this mode will have in the variable part of the frag
     4) which index into the table to try if we can't fit into this one.  */

  /* UNCOND_JUMP states.  */
  {127 + 1, -128 + 1, 1, ENCODE_RELAX_STATE (UNCOND_JUMP, BIG)},
  {127 + 1, -128 + 1, 1, ENCODE_RELAX_STATE (UNCOND_JUMP, BIG16)},
  /* dword jmp adds 4 bytes to frag:
     0 extra opcode bytes, 4 displacement bytes.  */
  {0, 0, 4, 0},
  /* word jmp adds 2 byte2 to frag:
     0 extra opcode bytes, 2 displacement bytes.  */
  {0, 0, 2, 0},

  /* COND_JUMP states.  */
  {127 + 1, -128 + 1, 1, ENCODE_RELAX_STATE (COND_JUMP, BIG)},
  {127 + 1, -128 + 1, 1, ENCODE_RELAX_STATE (COND_JUMP, BIG16)},
  /* dword conditionals adds 5 bytes to frag:
     1 extra opcode byte, 4 displacement bytes.  */
  {0, 0, 5, 0},
  /* word conditionals add 3 bytes to frag:
     1 extra opcode byte, 2 displacement bytes.  */
  {0, 0, 3, 0},

  /* COND_JUMP86 states.  */
  {127 + 1, -128 + 1, 1, ENCODE_RELAX_STATE (COND_JUMP86, BIG)},
  {127 + 1, -128 + 1, 1, ENCODE_RELAX_STATE (COND_JUMP86, BIG16)},
  /* dword conditionals adds 5 bytes to frag:
     1 extra opcode byte, 4 displacement bytes.  */
  {0, 0, 5, 0},
  /* word conditionals add 4 bytes to frag:
     1 displacement byte and a 3 byte long branch insn.  */
  {0, 0, 4, 0}
};

#define ARCH(n, t, f, s) \
  { STRING_COMMA_LEN (#n), s, PROCESSOR_ ## t, CPU_ ## f ## _FLAGS, \
    CPU_NONE_FLAGS }
#define SUBARCH(n, e, d, s) \
  { STRING_COMMA_LEN (#n), s, PROCESSOR_NONE, CPU_ ## e ## _FLAGS, \
    CPU_ ## d ## _FLAGS }

static const arch_entry cpu_arch[] =
{
  /* Do not replace the first two entries - i386_target_format() and
     set_cpu_arch() rely on them being there in this order.  */
  ARCH (generic32, GENERIC32, GENERIC32, false),
  ARCH (generic64, GENERIC64, GENERIC64, false),
  ARCH (i8086, UNKNOWN, NONE, false),
  ARCH (i186, UNKNOWN, 186, false),
  ARCH (i286, UNKNOWN, 286, false),
  ARCH (i386, I386, 386, false),
  ARCH (i486, I486, 486, false),
  ARCH (i586, PENTIUM, 586, false),
  ARCH (i686, PENTIUMPRO, 686, false),
  ARCH (pentium, PENTIUM, 586, false),
  ARCH (pentiumpro, PENTIUMPRO, PENTIUMPRO, false),
  ARCH (pentiumii, PENTIUMPRO, P2, false),
  ARCH (pentiumiii, PENTIUMPRO, P3, false),
  ARCH (pentium4, PENTIUM4, P4, false),
  ARCH (prescott, NOCONA, CORE, false),
  ARCH (nocona, NOCONA, NOCONA, false),
  ARCH (yonah, CORE, CORE, true),
  ARCH (core, CORE, CORE, false),
  ARCH (merom, CORE2, CORE2, true),
  ARCH (core2, CORE2, CORE2, false),
  ARCH (corei7, COREI7, COREI7, false),
  ARCH (iamcu, IAMCU, IAMCU, false),
  ARCH (k6, K6, K6, false),
  ARCH (k6_2, K6, K6_2, false),
  ARCH (athlon, ATHLON, ATHLON, false),
  ARCH (sledgehammer, K8, K8, true),
  ARCH (opteron, K8, K8, false),
  ARCH (k8, K8, K8, false),
  ARCH (amdfam10, AMDFAM10, AMDFAM10, false),
  ARCH (bdver1, BD, BDVER1, false),
  ARCH (bdver2, BD, BDVER2, false),
  ARCH (bdver3, BD, BDVER3, false),
  ARCH (bdver4, BD, BDVER4, false),
  ARCH (znver1, ZNVER, ZNVER1, false),
  ARCH (znver2, ZNVER, ZNVER2, false),
  ARCH (znver3, ZNVER, ZNVER3, false),
  ARCH (znver4, ZNVER, ZNVER4, false),
  ARCH (btver1, BT, BTVER1, false),
  ARCH (btver2, BT, BTVER2, false),

  SUBARCH (8087, 8087, ANY_8087, false),
  SUBARCH (87, NONE, ANY_8087, false), /* Disable only!  */
  SUBARCH (287, 287, ANY_287, false),
  SUBARCH (387, 387, ANY_387, false),
  SUBARCH (687, 687, ANY_687, false),
  SUBARCH (cmov, CMOV, CMOV, false),
  SUBARCH (fxsr, FXSR, ANY_FXSR, false),
  SUBARCH (mmx, MMX, ANY_MMX, false),
  SUBARCH (sse, SSE, ANY_SSE, false),
  SUBARCH (sse2, SSE2, ANY_SSE2, false),
  SUBARCH (sse3, SSE3, ANY_SSE3, false),
  SUBARCH (sse4a, SSE4A, ANY_SSE4A, false),
  SUBARCH (ssse3, SSSE3, ANY_SSSE3, false),
  SUBARCH (sse4.1, SSE4_1, ANY_SSE4_1, false),
  SUBARCH (sse4.2, SSE4_2, ANY_SSE4_2, false),
  SUBARCH (sse4, SSE4_2, ANY_SSE4_1, false),
  SUBARCH (avx, AVX, ANY_AVX, false),
  SUBARCH (avx2, AVX2, ANY_AVX2, false),
  SUBARCH (avx512f, AVX512F, ANY_AVX512F, false),
  SUBARCH (avx512cd, AVX512CD, ANY_AVX512CD, false),
  SUBARCH (avx512er, AVX512ER, ANY_AVX512ER, false),
  SUBARCH (avx512pf, AVX512PF, ANY_AVX512PF, false),
  SUBARCH (avx512dq, AVX512DQ, ANY_AVX512DQ, false),
  SUBARCH (avx512bw, AVX512BW, ANY_AVX512BW, false),
  SUBARCH (avx512vl, AVX512VL, ANY_AVX512VL, false),
  SUBARCH (monitor, MONITOR, MONITOR, false),
  SUBARCH (vmx, VMX, ANY_VMX, false),
  SUBARCH (vmfunc, VMFUNC, ANY_VMFUNC, false),
  SUBARCH (smx, SMX, SMX, false),
  SUBARCH (xsave, XSAVE, ANY_XSAVE, false),
  SUBARCH (xsaveopt, XSAVEOPT, ANY_XSAVEOPT, false),
  SUBARCH (xsavec, XSAVEC, ANY_XSAVEC, false),
  SUBARCH (xsaves, XSAVES, ANY_XSAVES, false),
  SUBARCH (aes, AES, ANY_AES, false),
  SUBARCH (pclmul, PCLMUL, ANY_PCLMUL, false),
  SUBARCH (clmul, PCLMUL, ANY_PCLMUL, true),
  SUBARCH (fsgsbase, FSGSBASE, FSGSBASE, false),
  SUBARCH (rdrnd, RDRND, RDRND, false),
  SUBARCH (f16c, F16C, ANY_F16C, false),
  SUBARCH (bmi2, BMI2, BMI2, false),
  SUBARCH (fma, FMA, ANY_FMA, false),
  SUBARCH (fma4, FMA4, ANY_FMA4, false),
  SUBARCH (xop, XOP, ANY_XOP, false),
  SUBARCH (lwp, LWP, ANY_LWP, false),
  SUBARCH (movbe, MOVBE, MOVBE, false),
  SUBARCH (cx16, CX16, CX16, false),
  SUBARCH (lahf_sahf, LAHF_SAHF, LAHF_SAHF, false),
  SUBARCH (ept, EPT, ANY_EPT, false),
  SUBARCH (lzcnt, LZCNT, LZCNT, false),
  SUBARCH (popcnt, POPCNT, POPCNT, false),
  SUBARCH (hle, HLE, HLE, false),
  SUBARCH (rtm, RTM, ANY_RTM, false),
  SUBARCH (tsx, TSX, TSX, false),
  SUBARCH (invpcid, INVPCID, INVPCID, false),
  SUBARCH (clflush, CLFLUSH, CLFLUSH, false),
  SUBARCH (nop, NOP, NOP, false),
  SUBARCH (syscall, SYSCALL, SYSCALL, false),
  SUBARCH (rdtscp, RDTSCP, RDTSCP, false),
  SUBARCH (3dnow, 3DNOW, ANY_3DNOW, false),
  SUBARCH (3dnowa, 3DNOWA, ANY_3DNOWA, false),
  SUBARCH (padlock, PADLOCK, PADLOCK, false),
  SUBARCH (pacifica, SVME, ANY_SVME, true),
  SUBARCH (svme, SVME, ANY_SVME, false),
  SUBARCH (abm, ABM, ABM, false),
  SUBARCH (bmi, BMI, BMI, false),
  SUBARCH (tbm, TBM, TBM, false),
  SUBARCH (adx, ADX, ADX, false),
  SUBARCH (rdseed, RDSEED, RDSEED, false),
  SUBARCH (prfchw, PRFCHW, PRFCHW, false),
  SUBARCH (smap, SMAP, SMAP, false),
  SUBARCH (mpx, MPX, ANY_MPX, false),
  SUBARCH (sha, SHA, ANY_SHA, false),
  SUBARCH (clflushopt, CLFLUSHOPT, CLFLUSHOPT, false),
  SUBARCH (prefetchwt1, PREFETCHWT1, PREFETCHWT1, false),
  SUBARCH (se1, SE1, SE1, false),
  SUBARCH (clwb, CLWB, CLWB, false),
  SUBARCH (avx512ifma, AVX512IFMA, ANY_AVX512IFMA, false),
  SUBARCH (avx512vbmi, AVX512VBMI, ANY_AVX512VBMI, false),
  SUBARCH (avx512_4fmaps, AVX512_4FMAPS, ANY_AVX512_4FMAPS, false),
  SUBARCH (avx512_4vnniw, AVX512_4VNNIW, ANY_AVX512_4VNNIW, false),
  SUBARCH (avx512_vpopcntdq, AVX512_VPOPCNTDQ, ANY_AVX512_VPOPCNTDQ, false),
  SUBARCH (avx512_vbmi2, AVX512_VBMI2, ANY_AVX512_VBMI2, false),
  SUBARCH (avx512_vnni, AVX512_VNNI, ANY_AVX512_VNNI, false),
  SUBARCH (avx512_bitalg, AVX512_BITALG, ANY_AVX512_BITALG, false),
  SUBARCH (avx_vnni, AVX_VNNI, ANY_AVX_VNNI, false),
  SUBARCH (clzero, CLZERO, CLZERO, false),
  SUBARCH (mwaitx, MWAITX, MWAITX, false),
  SUBARCH (ospke, OSPKE, ANY_OSPKE, false),
  SUBARCH (rdpid, RDPID, RDPID, false),
  SUBARCH (ptwrite, PTWRITE, PTWRITE, false),
  SUBARCH (ibt, IBT, IBT, false),
  SUBARCH (shstk, SHSTK, SHSTK, false),
  SUBARCH (gfni, GFNI, ANY_GFNI, false),
  SUBARCH (vaes, VAES, ANY_VAES, false),
  SUBARCH (vpclmulqdq, VPCLMULQDQ, ANY_VPCLMULQDQ, false),
  SUBARCH (wbnoinvd, WBNOINVD, WBNOINVD, false),
  SUBARCH (pconfig, PCONFIG, PCONFIG, false),
  SUBARCH (waitpkg, WAITPKG, WAITPKG, false),
  SUBARCH (cldemote, CLDEMOTE, CLDEMOTE, false),
  SUBARCH (amx_int8, AMX_INT8, ANY_AMX_INT8, false),
  SUBARCH (amx_bf16, AMX_BF16, ANY_AMX_BF16, false),
  SUBARCH (amx_fp16, AMX_FP16, ANY_AMX_FP16, false),
  SUBARCH (amx_complex, AMX_COMPLEX, ANY_AMX_COMPLEX, false),
  SUBARCH (amx_tile, AMX_TILE, ANY_AMX_TILE, false),
  SUBARCH (movdiri, MOVDIRI, MOVDIRI, false),
  SUBARCH (movdir64b, MOVDIR64B, MOVDIR64B, false),
  SUBARCH (avx512_bf16, AVX512_BF16, ANY_AVX512_BF16, false),
  SUBARCH (avx512_vp2intersect, AVX512_VP2INTERSECT,
	   ANY_AVX512_VP2INTERSECT, false),
  SUBARCH (tdx, TDX, TDX, false),
  SUBARCH (enqcmd, ENQCMD, ENQCMD, false),
  SUBARCH (serialize, SERIALIZE, SERIALIZE, false),
  SUBARCH (rdpru, RDPRU, RDPRU, false),
  SUBARCH (mcommit, MCOMMIT, MCOMMIT, false),
  SUBARCH (sev_es, SEV_ES, ANY_SEV_ES, false),
  SUBARCH (tsxldtrk, TSXLDTRK, ANY_TSXLDTRK, false),
  SUBARCH (kl, KL, ANY_KL, false),
  SUBARCH (widekl, WIDEKL, ANY_WIDEKL, false),
  SUBARCH (uintr, UINTR, UINTR, false),
  SUBARCH (hreset, HRESET, HRESET, false),
  SUBARCH (avx512_fp16, AVX512_FP16, ANY_AVX512_FP16, false),
  SUBARCH (prefetchi, PREFETCHI, PREFETCHI, false),
  SUBARCH (avx_ifma, AVX_IFMA, ANY_AVX_IFMA, false),
  SUBARCH (avx_vnni_int8, AVX_VNNI_INT8, ANY_AVX_VNNI_INT8, false),
  SUBARCH (cmpccxadd, CMPCCXADD, CMPCCXADD, false),
  SUBARCH (wrmsrns, WRMSRNS, WRMSRNS, false),
  SUBARCH (msrlist, MSRLIST, MSRLIST, false),
  SUBARCH (avx_ne_convert, AVX_NE_CONVERT, ANY_AVX_NE_CONVERT, false),
  SUBARCH (rao_int, RAO_INT, RAO_INT, false),
  SUBARCH (rmpquery, RMPQUERY, ANY_RMPQUERY, false),
  SUBARCH (fred, FRED, ANY_FRED, false),
  SUBARCH (lkgs, LKGS, ANY_LKGS, false),
};

#undef SUBARCH
#undef ARCH

#ifdef I386COFF
/* Like s_lcomm_internal in gas/read.c but the alignment string
   is allowed to be optional.  */

static symbolS *
pe_lcomm_internal (int needs_align, symbolS *symbolP, addressT size)
{
  addressT align = 0;

  SKIP_WHITESPACE ();

  if (needs_align
      && *input_line_pointer == ',')
    {
      align = parse_align (needs_align - 1);

      if (align == (addressT) -1)
	return NULL;
    }
  else
    {
      if (size >= 8)
	align = 3;
      else if (size >= 4)
	align = 2;
      else if (size >= 2)
	align = 1;
      else
	align = 0;
    }

  bss_alloc (symbolP, size, align);
  return symbolP;
}

static void
pe_lcomm (int needs_align)
{
  s_comm_internal (needs_align * 2, pe_lcomm_internal);
}
#endif

const pseudo_typeS md_pseudo_table[] =
{
#if !defined(OBJ_AOUT) && !defined(USE_ALIGN_PTWO)
  {"align", s_align_bytes, 0},
#else
  {"align", s_align_ptwo, 0},
#endif
  {"arch", set_cpu_arch, 0},
#ifndef I386COFF
  {"bss", s_bss, 0},
#else
  {"lcomm", pe_lcomm, 1},
#endif
  {"ffloat", float_cons, 'f'},
  {"dfloat", float_cons, 'd'},
  {"tfloat", float_cons, 'x'},
  {"hfloat", float_cons, 'h'},
  {"bfloat16", float_cons, 'b'},
  {"value", cons, 2},
  {"slong", signed_cons, 4},
  {"insn", s_insn, 0},
  {"noopt", s_ignore, 0},
  {"optim", s_ignore, 0},
  {"code16gcc", set_16bit_gcc_code_flag, CODE_16BIT},
  {"code16", set_code_flag, CODE_16BIT},
  {"code32", set_code_flag, CODE_32BIT},
#ifdef BFD64
  {"code64", set_code_flag, CODE_64BIT},
#endif
  {"intel_syntax", set_intel_syntax, 1},
  {"att_syntax", set_intel_syntax, 0},
  {"intel_mnemonic", set_intel_mnemonic, 1},
  {"att_mnemonic", set_intel_mnemonic, 0},
  {"allow_index_reg", set_allow_index_reg, 1},
  {"disallow_index_reg", set_allow_index_reg, 0},
  {"sse_check", set_check, 0},
  {"operand_check", set_check, 1},
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  {"largecomm", handle_large_common, 0},
#else
  {"file", dwarf2_directive_file, 0},
  {"loc", dwarf2_directive_loc, 0},
  {"loc_mark_labels", dwarf2_directive_loc_mark_labels, 0},
#endif
#ifdef TE_PE
  {"secrel32", pe_directive_secrel, 0},
  {"secidx", pe_directive_secidx, 0},
#endif
  {0, 0, 0}
};

/* For interface with expression ().  */
extern char *input_line_pointer;

/* Hash table for instruction mnemonic lookup.  */
static htab_t op_hash;

/* Hash table for register lookup.  */
static htab_t reg_hash;

  /* Various efficient no-op patterns for aligning code labels.
     Note: Don't try to assemble the instructions in the comments.
     0L and 0w are not legal.  */
static const unsigned char f32_1[] =
  {0x90};				/* nop			*/
static const unsigned char f32_2[] =
  {0x66,0x90};				/* xchg %ax,%ax		*/
static const unsigned char f32_3[] =
  {0x8d,0x76,0x00};			/* leal 0(%esi),%esi	*/
static const unsigned char f32_4[] =
  {0x8d,0x74,0x26,0x00};		/* leal 0(%esi,1),%esi	*/
static const unsigned char f32_6[] =
  {0x8d,0xb6,0x00,0x00,0x00,0x00};	/* leal 0L(%esi),%esi	*/
static const unsigned char f32_7[] =
  {0x8d,0xb4,0x26,0x00,0x00,0x00,0x00};	/* leal 0L(%esi,1),%esi */
static const unsigned char f16_3[] =
  {0x8d,0x74,0x00};			/* lea 0(%si),%si	*/
static const unsigned char f16_4[] =
  {0x8d,0xb4,0x00,0x00};		/* lea 0W(%si),%si	*/
static const unsigned char jump_disp8[] =
  {0xeb};				/* jmp disp8	       */
static const unsigned char jump32_disp32[] =
  {0xe9};				/* jmp disp32	       */
static const unsigned char jump16_disp32[] =
  {0x66,0xe9};				/* jmp disp32	       */
/* 32-bit NOPs patterns.  */
static const unsigned char *const f32_patt[] = {
  f32_1, f32_2, f32_3, f32_4, NULL, f32_6, f32_7
};
/* 16-bit NOPs patterns.  */
static const unsigned char *const f16_patt[] = {
  f32_1, f32_2, f16_3, f16_4
};
/* nopl (%[re]ax) */
static const unsigned char alt_3[] =
  {0x0f,0x1f,0x00};
/* nopl 0(%[re]ax) */
static const unsigned char alt_4[] =
  {0x0f,0x1f,0x40,0x00};
/* nopl 0(%[re]ax,%[re]ax,1) */
static const unsigned char alt_5[] =
  {0x0f,0x1f,0x44,0x00,0x00};
/* nopw 0(%[re]ax,%[re]ax,1) */
static const unsigned char alt_6[] =
  {0x66,0x0f,0x1f,0x44,0x00,0x00};
/* nopl 0L(%[re]ax) */
static const unsigned char alt_7[] =
  {0x0f,0x1f,0x80,0x00,0x00,0x00,0x00};
/* nopl 0L(%[re]ax,%[re]ax,1) */
static const unsigned char alt_8[] =
  {0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
/* nopw 0L(%[re]ax,%[re]ax,1) */
static const unsigned char alt_9[] =
  {0x66,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
/* nopw %cs:0L(%[re]ax,%[re]ax,1) */
static const unsigned char alt_10[] =
  {0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
/* data16 nopw %cs:0L(%eax,%eax,1) */
static const unsigned char alt_11[] =
  {0x66,0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
/* 32-bit and 64-bit NOPs patterns.  */
static const unsigned char *const alt_patt[] = {
  f32_1, f32_2, alt_3, alt_4, alt_5, alt_6, alt_7, alt_8,
  alt_9, alt_10, alt_11
};

/* Genenerate COUNT bytes of NOPs to WHERE from PATT with the maximum
   size of a single NOP instruction MAX_SINGLE_NOP_SIZE.  */

static void
i386_output_nops (char *where, const unsigned char *const *patt,
		  int count, int max_single_nop_size)

{
  /* Place the longer NOP first.  */
  int last;
  int offset;
  const unsigned char *nops;

  if (max_single_nop_size < 1)
    {
      as_fatal (_("i386_output_nops called to generate nops of at most %d bytes!"),
		max_single_nop_size);
      return;
    }

  nops = patt[max_single_nop_size - 1];

  /* Use the smaller one if the requsted one isn't available.  */
  if (nops == NULL)
    {
      max_single_nop_size--;
      nops = patt[max_single_nop_size - 1];
    }

  last = count % max_single_nop_size;

  count -= last;
  for (offset = 0; offset < count; offset += max_single_nop_size)
    memcpy (where + offset, nops, max_single_nop_size);

  if (last)
    {
      nops = patt[last - 1];
      if (nops == NULL)
	{
	  /* Use the smaller one plus one-byte NOP if the needed one
	     isn't available.  */
	  last--;
	  nops = patt[last - 1];
	  memcpy (where + offset, nops, last);
	  where[offset + last] = *patt[0];
	}
      else
	memcpy (where + offset, nops, last);
    }
}

static INLINE int
fits_in_imm7 (offsetT num)
{
  return (num & 0x7f) == num;
}

static INLINE int
fits_in_imm31 (offsetT num)
{
  return (num & 0x7fffffff) == num;
}

/* Genenerate COUNT bytes of NOPs to WHERE with the maximum size of a
   single NOP instruction LIMIT.  */

void
i386_generate_nops (fragS *fragP, char *where, offsetT count, int limit)
{
  const unsigned char *const *patt = NULL;
  int max_single_nop_size;
  /* Maximum number of NOPs before switching to jump over NOPs.  */
  int max_number_of_nops;

  switch (fragP->fr_type)
    {
    case rs_fill_nop:
    case rs_align_code:
      break;
    case rs_machine_dependent:
      /* Allow NOP padding for jumps and calls.  */
      if (TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == BRANCH_PADDING
	  || TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == FUSED_JCC_PADDING)
	break;
      /* Fall through.  */
    default:
      return;
    }

  /* We need to decide which NOP sequence to use for 32bit and
     64bit. When -mtune= is used:

     1. For PROCESSOR_I386, PROCESSOR_I486, PROCESSOR_PENTIUM and
     PROCESSOR_GENERIC32, f32_patt will be used.
     2. For the rest, alt_patt will be used.

     When -mtune= isn't used, alt_patt will be used if
     cpu_arch_isa_flags has CpuNop.  Otherwise, f32_patt will
     be used.

     When -march= or .arch is used, we can't use anything beyond
     cpu_arch_isa_flags.   */

  if (flag_code == CODE_16BIT)
    {
      patt = f16_patt;
      max_single_nop_size = sizeof (f16_patt) / sizeof (f16_patt[0]);
      /* Limit number of NOPs to 2 in 16-bit mode.  */
      max_number_of_nops = 2;
    }
  else
    {
      if (fragP->tc_frag_data.isa == PROCESSOR_UNKNOWN)
	{
	  /* PROCESSOR_UNKNOWN means that all ISAs may be used.  */
	  switch (cpu_arch_tune)
	    {
	    case PROCESSOR_UNKNOWN:
	      /* We use cpu_arch_isa_flags to check if we SHOULD
		 optimize with nops.  */
	      if (fragP->tc_frag_data.isa_flags.bitfield.cpunop)
		patt = alt_patt;
	      else
		patt = f32_patt;
	      break;
	    case PROCESSOR_PENTIUM4:
	    case PROCESSOR_NOCONA:
	    case PROCESSOR_CORE:
	    case PROCESSOR_CORE2:
	    case PROCESSOR_COREI7:
	    case PROCESSOR_GENERIC64:
	    case PROCESSOR_K6:
	    case PROCESSOR_ATHLON:
	    case PROCESSOR_K8:
	    case PROCESSOR_AMDFAM10:
	    case PROCESSOR_BD:
	    case PROCESSOR_ZNVER:
	    case PROCESSOR_BT:
	      patt = alt_patt;
	      break;
	    case PROCESSOR_I386:
	    case PROCESSOR_I486:
	    case PROCESSOR_PENTIUM:
	    case PROCESSOR_PENTIUMPRO:
	    case PROCESSOR_IAMCU:
	    case PROCESSOR_GENERIC32:
	      patt = f32_patt;
	      break;
	    case PROCESSOR_NONE:
	      abort ();
	    }
	}
      else
	{
	  switch (fragP->tc_frag_data.tune)
	    {
	    case PROCESSOR_UNKNOWN:
	      /* When cpu_arch_isa is set, cpu_arch_tune shouldn't be
		 PROCESSOR_UNKNOWN.  */
	      abort ();
	      break;

	    case PROCESSOR_I386:
	    case PROCESSOR_I486:
	    case PROCESSOR_PENTIUM:
	    case PROCESSOR_IAMCU:
	    case PROCESSOR_K6:
	    case PROCESSOR_ATHLON:
	    case PROCESSOR_K8:
	    case PROCESSOR_AMDFAM10:
	    case PROCESSOR_BD:
	    case PROCESSOR_ZNVER:
	    case PROCESSOR_BT:
	    case PROCESSOR_GENERIC32:
	      /* We use cpu_arch_isa_flags to check if we CAN optimize
		 with nops.  */
	      if (fragP->tc_frag_data.isa_flags.bitfield.cpunop)
		patt = alt_patt;
	      else
		patt = f32_patt;
	      break;
	    case PROCESSOR_PENTIUMPRO:
	    case PROCESSOR_PENTIUM4:
	    case PROCESSOR_NOCONA:
	    case PROCESSOR_CORE:
	    case PROCESSOR_CORE2:
	    case PROCESSOR_COREI7:
	      if (fragP->tc_frag_data.isa_flags.bitfield.cpunop)
		patt = alt_patt;
	      else
		patt = f32_patt;
	      break;
	    case PROCESSOR_GENERIC64:
	      patt = alt_patt;
	      break;
	    case PROCESSOR_NONE:
	      abort ();
	    }
	}

      if (patt == f32_patt)
	{
	  max_single_nop_size = sizeof (f32_patt) / sizeof (f32_patt[0]);
	  /* Limit number of NOPs to 2 for older processors.  */
	  max_number_of_nops = 2;
	}
      else
	{
	  max_single_nop_size = sizeof (alt_patt) / sizeof (alt_patt[0]);
	  /* Limit number of NOPs to 7 for newer processors.  */
	  max_number_of_nops = 7;
	}
    }

  if (limit == 0)
    limit = max_single_nop_size;

  if (fragP->fr_type == rs_fill_nop)
    {
      /* Output NOPs for .nop directive.  */
      if (limit > max_single_nop_size)
	{
	  as_bad_where (fragP->fr_file, fragP->fr_line,
			_("invalid single nop size: %d "
			  "(expect within [0, %d])"),
			limit, max_single_nop_size);
	  return;
	}
    }
  else if (fragP->fr_type != rs_machine_dependent)
    fragP->fr_var = count;

  if ((count / max_single_nop_size) > max_number_of_nops)
    {
      /* Generate jump over NOPs.  */
      offsetT disp = count - 2;
      if (fits_in_imm7 (disp))
	{
	  /* Use "jmp disp8" if possible.  */
	  count = disp;
	  where[0] = jump_disp8[0];
	  where[1] = count;
	  where += 2;
	}
      else
	{
	  unsigned int size_of_jump;

	  if (flag_code == CODE_16BIT)
	    {
	      where[0] = jump16_disp32[0];
	      where[1] = jump16_disp32[1];
	      size_of_jump = 2;
	    }
	  else
	    {
	      where[0] = jump32_disp32[0];
	      size_of_jump = 1;
	    }

	  count -= size_of_jump + 4;
	  if (!fits_in_imm31 (count))
	    {
	      as_bad_where (fragP->fr_file, fragP->fr_line,
			    _("jump over nop padding out of range"));
	      return;
	    }

	  md_number_to_chars (where + size_of_jump, count, 4);
	  where += size_of_jump + 4;
	}
    }

  /* Generate multiple NOPs.  */
  i386_output_nops (where, patt, count, limit);
}

static INLINE int
operand_type_all_zero (const union i386_operand_type *x)
{
  switch (ARRAY_SIZE(x->array))
    {
    case 3:
      if (x->array[2])
	return 0;
      /* Fall through.  */
    case 2:
      if (x->array[1])
	return 0;
      /* Fall through.  */
    case 1:
      return !x->array[0];
    default:
      abort ();
    }
}

static INLINE void
operand_type_set (union i386_operand_type *x, unsigned int v)
{
  switch (ARRAY_SIZE(x->array))
    {
    case 3:
      x->array[2] = v;
      /* Fall through.  */
    case 2:
      x->array[1] = v;
      /* Fall through.  */
    case 1:
      x->array[0] = v;
      /* Fall through.  */
      break;
    default:
      abort ();
    }

  x->bitfield.class = ClassNone;
  x->bitfield.instance = InstanceNone;
}

static INLINE int
operand_type_equal (const union i386_operand_type *x,
		    const union i386_operand_type *y)
{
  switch (ARRAY_SIZE(x->array))
    {
    case 3:
      if (x->array[2] != y->array[2])
	return 0;
      /* Fall through.  */
    case 2:
      if (x->array[1] != y->array[1])
	return 0;
      /* Fall through.  */
    case 1:
      return x->array[0] == y->array[0];
      break;
    default:
      abort ();
    }
}

static INLINE int
cpu_flags_all_zero (const union i386_cpu_flags *x)
{
  switch (ARRAY_SIZE(x->array))
    {
    case 5:
      if (x->array[4])
	return 0;
      /* Fall through.  */
    case 4:
      if (x->array[3])
	return 0;
      /* Fall through.  */
    case 3:
      if (x->array[2])
	return 0;
      /* Fall through.  */
    case 2:
      if (x->array[1])
	return 0;
      /* Fall through.  */
    case 1:
      return !x->array[0];
    default:
      abort ();
    }
}

static INLINE int
cpu_flags_equal (const union i386_cpu_flags *x,
		 const union i386_cpu_flags *y)
{
  switch (ARRAY_SIZE(x->array))
    {
    case 5:
      if (x->array[4] != y->array[4])
	return 0;
      /* Fall through.  */
    case 4:
      if (x->array[3] != y->array[3])
	return 0;
      /* Fall through.  */
    case 3:
      if (x->array[2] != y->array[2])
	return 0;
      /* Fall through.  */
    case 2:
      if (x->array[1] != y->array[1])
	return 0;
      /* Fall through.  */
    case 1:
      return x->array[0] == y->array[0];
      break;
    default:
      abort ();
    }
}

static INLINE int
cpu_flags_check_cpu64 (i386_cpu_flags f)
{
  return !((flag_code == CODE_64BIT && f.bitfield.cpuno64)
	   || (flag_code != CODE_64BIT && f.bitfield.cpu64));
}

static INLINE i386_cpu_flags
cpu_flags_and (i386_cpu_flags x, i386_cpu_flags y)
{
  switch (ARRAY_SIZE (x.array))
    {
    case 5:
      x.array [4] &= y.array [4];
      /* Fall through.  */
    case 4:
      x.array [3] &= y.array [3];
      /* Fall through.  */
    case 3:
      x.array [2] &= y.array [2];
      /* Fall through.  */
    case 2:
      x.array [1] &= y.array [1];
      /* Fall through.  */
    case 1:
      x.array [0] &= y.array [0];
      break;
    default:
      abort ();
    }
  return x;
}

static INLINE i386_cpu_flags
cpu_flags_or (i386_cpu_flags x, i386_cpu_flags y)
{
  switch (ARRAY_SIZE (x.array))
    {
    case 5:
      x.array [4] |= y.array [4];
      /* Fall through.  */
    case 4:
      x.array [3] |= y.array [3];
      /* Fall through.  */
    case 3:
      x.array [2] |= y.array [2];
      /* Fall through.  */
    case 2:
      x.array [1] |= y.array [1];
      /* Fall through.  */
    case 1:
      x.array [0] |= y.array [0];
      break;
    default:
      abort ();
    }
  return x;
}

static INLINE i386_cpu_flags
cpu_flags_and_not (i386_cpu_flags x, i386_cpu_flags y)
{
  switch (ARRAY_SIZE (x.array))
    {
    case 5:
      x.array [4] &= ~y.array [4];
      /* Fall through.  */
    case 4:
      x.array [3] &= ~y.array [3];
      /* Fall through.  */
    case 3:
      x.array [2] &= ~y.array [2];
      /* Fall through.  */
    case 2:
      x.array [1] &= ~y.array [1];
      /* Fall through.  */
    case 1:
      x.array [0] &= ~y.array [0];
      break;
    default:
      abort ();
    }
  return x;
}

static const i386_cpu_flags avx512 = CPU_ANY_AVX512F_FLAGS;

#define CPU_FLAGS_ARCH_MATCH		0x1
#define CPU_FLAGS_64BIT_MATCH		0x2

#define CPU_FLAGS_PERFECT_MATCH \
  (CPU_FLAGS_ARCH_MATCH | CPU_FLAGS_64BIT_MATCH)

/* Return CPU flags match bits. */

static int
cpu_flags_match (const insn_template *t)
{
  i386_cpu_flags x = t->cpu_flags;
  int match = cpu_flags_check_cpu64 (x) ? CPU_FLAGS_64BIT_MATCH : 0;

  x.bitfield.cpu64 = 0;
  x.bitfield.cpuno64 = 0;

  if (cpu_flags_all_zero (&x))
    {
      /* This instruction is available on all archs.  */
      match |= CPU_FLAGS_ARCH_MATCH;
    }
  else
    {
      /* This instruction is available only on some archs.  */
      i386_cpu_flags cpu = cpu_arch_flags;

      /* AVX512VL is no standalone feature - match it and then strip it.  */
      if (x.bitfield.cpuavx512vl && !cpu.bitfield.cpuavx512vl)
	return match;
      x.bitfield.cpuavx512vl = 0;

      /* AVX and AVX2 present at the same time express an operand size
	 dependency - strip AVX2 for the purposes here.  The operand size
	 dependent check occurs in check_vecOperands().  */
      if (x.bitfield.cpuavx && x.bitfield.cpuavx2)
	x.bitfield.cpuavx2 = 0;

      cpu = cpu_flags_and (x, cpu);
      if (!cpu_flags_all_zero (&cpu))
	{
	  if (x.bitfield.cpuavx)
	    {
	      /* We need to check a few extra flags with AVX.  */
	      if (cpu.bitfield.cpuavx
		  && (!t->opcode_modifier.sse2avx
		      || (sse2avx && !i.prefix[DATA_PREFIX]))
		  && (!x.bitfield.cpuaes || cpu.bitfield.cpuaes)
		  && (!x.bitfield.cpugfni || cpu.bitfield.cpugfni)
		  && (!x.bitfield.cpupclmul || cpu.bitfield.cpupclmul))
		match |= CPU_FLAGS_ARCH_MATCH;
	    }
	  else if (x.bitfield.cpuavx512f)
	    {
	      /* We need to check a few extra flags with AVX512F.  */
	      if (cpu.bitfield.cpuavx512f
		  && (!x.bitfield.cpugfni || cpu.bitfield.cpugfni)
		  && (!x.bitfield.cpuvaes || cpu.bitfield.cpuvaes)
		  && (!x.bitfield.cpuvpclmulqdq || cpu.bitfield.cpuvpclmulqdq))
		match |= CPU_FLAGS_ARCH_MATCH;
	    }
	  else
	    match |= CPU_FLAGS_ARCH_MATCH;
	}
    }
  return match;
}

static INLINE i386_operand_type
operand_type_and (i386_operand_type x, i386_operand_type y)
{
  if (x.bitfield.class != y.bitfield.class)
    x.bitfield.class = ClassNone;
  if (x.bitfield.instance != y.bitfield.instance)
    x.bitfield.instance = InstanceNone;

  switch (ARRAY_SIZE (x.array))
    {
    case 3:
      x.array [2] &= y.array [2];
      /* Fall through.  */
    case 2:
      x.array [1] &= y.array [1];
      /* Fall through.  */
    case 1:
      x.array [0] &= y.array [0];
      break;
    default:
      abort ();
    }
  return x;
}

static INLINE i386_operand_type
operand_type_and_not (i386_operand_type x, i386_operand_type y)
{
  gas_assert (y.bitfield.class == ClassNone);
  gas_assert (y.bitfield.instance == InstanceNone);

  switch (ARRAY_SIZE (x.array))
    {
    case 3:
      x.array [2] &= ~y.array [2];
      /* Fall through.  */
    case 2:
      x.array [1] &= ~y.array [1];
      /* Fall through.  */
    case 1:
      x.array [0] &= ~y.array [0];
      break;
    default:
      abort ();
    }
  return x;
}

static INLINE i386_operand_type
operand_type_or (i386_operand_type x, i386_operand_type y)
{
  gas_assert (x.bitfield.class == ClassNone ||
              y.bitfield.class == ClassNone ||
              x.bitfield.class == y.bitfield.class);
  gas_assert (x.bitfield.instance == InstanceNone ||
              y.bitfield.instance == InstanceNone ||
              x.bitfield.instance == y.bitfield.instance);

  switch (ARRAY_SIZE (x.array))
    {
    case 3:
      x.array [2] |= y.array [2];
      /* Fall through.  */
    case 2:
      x.array [1] |= y.array [1];
      /* Fall through.  */
    case 1:
      x.array [0] |= y.array [0];
      break;
    default:
      abort ();
    }
  return x;
}

static INLINE i386_operand_type
operand_type_xor (i386_operand_type x, i386_operand_type y)
{
  gas_assert (y.bitfield.class == ClassNone);
  gas_assert (y.bitfield.instance == InstanceNone);

  switch (ARRAY_SIZE (x.array))
    {
    case 3:
      x.array [2] ^= y.array [2];
      /* Fall through.  */
    case 2:
      x.array [1] ^= y.array [1];
      /* Fall through.  */
    case 1:
      x.array [0] ^= y.array [0];
      break;
    default:
      abort ();
    }
  return x;
}

static const i386_operand_type anydisp = {
  .bitfield = { .disp8 = 1, .disp16 = 1, .disp32 = 1, .disp64 = 1 }
};

enum operand_type
{
  reg,
  imm,
  disp,
  anymem
};

static INLINE int
operand_type_check (i386_operand_type t, enum operand_type c)
{
  switch (c)
    {
    case reg:
      return t.bitfield.class == Reg;

    case imm:
      return (t.bitfield.imm8
	      || t.bitfield.imm8s
	      || t.bitfield.imm16
	      || t.bitfield.imm32
	      || t.bitfield.imm32s
	      || t.bitfield.imm64);

    case disp:
      return (t.bitfield.disp8
	      || t.bitfield.disp16
	      || t.bitfield.disp32
	      || t.bitfield.disp64);

    case anymem:
      return (t.bitfield.disp8
	      || t.bitfield.disp16
	      || t.bitfield.disp32
	      || t.bitfield.disp64
	      || t.bitfield.baseindex);

    default:
      abort ();
    }

  return 0;
}

/* Return 1 if there is no conflict in 8bit/16bit/32bit/64bit/80bit size
   between operand GIVEN and opeand WANTED for instruction template T.  */

static INLINE int
match_operand_size (const insn_template *t, unsigned int wanted,
		    unsigned int given)
{
  return !((i.types[given].bitfield.byte
	    && !t->operand_types[wanted].bitfield.byte)
	   || (i.types[given].bitfield.word
	       && !t->operand_types[wanted].bitfield.word)
	   || (i.types[given].bitfield.dword
	       && !t->operand_types[wanted].bitfield.dword)
	   || (i.types[given].bitfield.qword
	       && (!t->operand_types[wanted].bitfield.qword
		   /* Don't allow 64-bit (memory) operands outside of 64-bit
		      mode, when they're used where a 64-bit GPR could also
		      be used.  Checking is needed for Intel Syntax only.  */
		   || (intel_syntax
		       && flag_code != CODE_64BIT
		       && (t->operand_types[wanted].bitfield.class == Reg
			   || t->operand_types[wanted].bitfield.class == Accum
			   || t->opcode_modifier.isstring))))
	   || (i.types[given].bitfield.tbyte
	       && !t->operand_types[wanted].bitfield.tbyte));
}

/* Return 1 if there is no conflict in SIMD register between operand
   GIVEN and opeand WANTED for instruction template T.  */

static INLINE int
match_simd_size (const insn_template *t, unsigned int wanted,
		 unsigned int given)
{
  return !((i.types[given].bitfield.xmmword
	    && !t->operand_types[wanted].bitfield.xmmword)
	   || (i.types[given].bitfield.ymmword
	       && !t->operand_types[wanted].bitfield.ymmword)
	   || (i.types[given].bitfield.zmmword
	       && !t->operand_types[wanted].bitfield.zmmword)
	   || (i.types[given].bitfield.tmmword
	       && !t->operand_types[wanted].bitfield.tmmword));
}

/* Return 1 if there is no conflict in any size between operand GIVEN
   and opeand WANTED for instruction template T.  */

static INLINE int
match_mem_size (const insn_template *t, unsigned int wanted,
		unsigned int given)
{
  return (match_operand_size (t, wanted, given)
	  && !((i.types[given].bitfield.unspecified
		&& !i.broadcast.type
		&& !i.broadcast.bytes
		&& !t->operand_types[wanted].bitfield.unspecified)
	       || (i.types[given].bitfield.fword
		   && !t->operand_types[wanted].bitfield.fword)
	       /* For scalar opcode templates to allow register and memory
		  operands at the same time, some special casing is needed
		  here.  Also for v{,p}broadcast*, {,v}pmov{s,z}*, and
		  down-conversion vpmov*.  */
	       || ((t->operand_types[wanted].bitfield.class == RegSIMD
		    && t->operand_types[wanted].bitfield.byte
		       + t->operand_types[wanted].bitfield.word
		       + t->operand_types[wanted].bitfield.dword
		       + t->operand_types[wanted].bitfield.qword
		       > !!t->opcode_modifier.broadcast)
		   ? (i.types[given].bitfield.xmmword
		      || i.types[given].bitfield.ymmword
		      || i.types[given].bitfield.zmmword)
		   : !match_simd_size(t, wanted, given))));
}

/* Return value has MATCH_STRAIGHT set if there is no size conflict on any
   operands for instruction template T, and it has MATCH_REVERSE set if there
   is no size conflict on any operands for the template with operands reversed
   (and the template allows for reversing in the first place).  */

#define MATCH_STRAIGHT 1
#define MATCH_REVERSE  2

static INLINE unsigned int
operand_size_match (const insn_template *t)
{
  unsigned int j, match = MATCH_STRAIGHT;

  /* Don't check non-absolute jump instructions.  */
  if (t->opcode_modifier.jump
      && t->opcode_modifier.jump != JUMP_ABSOLUTE)
    return match;

  /* Check memory and accumulator operand size.  */
  for (j = 0; j < i.operands; j++)
    {
      if (i.types[j].bitfield.class != Reg
	  && i.types[j].bitfield.class != RegSIMD
	  && t->opcode_modifier.operandconstraint == ANY_SIZE)
	continue;

      if (t->operand_types[j].bitfield.class == Reg
	  && !match_operand_size (t, j, j))
	{
	  match = 0;
	  break;
	}

      if (t->operand_types[j].bitfield.class == RegSIMD
	  && !match_simd_size (t, j, j))
	{
	  match = 0;
	  break;
	}

      if (t->operand_types[j].bitfield.instance == Accum
	  && (!match_operand_size (t, j, j) || !match_simd_size (t, j, j)))
	{
	  match = 0;
	  break;
	}

      if ((i.flags[j] & Operand_Mem) && !match_mem_size (t, j, j))
	{
	  match = 0;
	  break;
	}
    }

  if (!t->opcode_modifier.d)
    return match;

  /* Check reverse.  */
  gas_assert (i.operands >= 2);

  for (j = 0; j < i.operands; j++)
    {
      unsigned int given = i.operands - j - 1;

      /* For FMA4 and XOP insns VEX.W controls just the first two
	 register operands.  */
      if (t->cpu_flags.bitfield.cpufma4 || t->cpu_flags.bitfield.cpuxop)
	given = j < 2 ? 1 - j : j;

      if (t->operand_types[j].bitfield.class == Reg
	  && !match_operand_size (t, j, given))
	return match;

      if (t->operand_types[j].bitfield.class == RegSIMD
	  && !match_simd_size (t, j, given))
	return match;

      if (t->operand_types[j].bitfield.instance == Accum
	  && (!match_operand_size (t, j, given)
	      || !match_simd_size (t, j, given)))
	return match;

      if ((i.flags[given] & Operand_Mem) && !match_mem_size (t, j, given))
	return match;
    }

  return match | MATCH_REVERSE;
}

static INLINE int
operand_type_match (i386_operand_type overlap,
		    i386_operand_type given)
{
  i386_operand_type temp = overlap;

  temp.bitfield.unspecified = 0;
  temp.bitfield.byte = 0;
  temp.bitfield.word = 0;
  temp.bitfield.dword = 0;
  temp.bitfield.fword = 0;
  temp.bitfield.qword = 0;
  temp.bitfield.tbyte = 0;
  temp.bitfield.xmmword = 0;
  temp.bitfield.ymmword = 0;
  temp.bitfield.zmmword = 0;
  temp.bitfield.tmmword = 0;
  if (operand_type_all_zero (&temp))
    goto mismatch;

  if (given.bitfield.baseindex == overlap.bitfield.baseindex)
    return 1;

 mismatch:
  i.error = operand_type_mismatch;
  return 0;
}

/* If given types g0 and g1 are registers they must be of the same type
   unless the expected operand type register overlap is null.
   Intel syntax sized memory operands are also checked here.  */

static INLINE int
operand_type_register_match (i386_operand_type g0,
			     i386_operand_type t0,
			     i386_operand_type g1,
			     i386_operand_type t1)
{
  if (g0.bitfield.class != Reg
      && g0.bitfield.class != RegSIMD
      && (g0.bitfield.unspecified
	  || !operand_type_check (g0, anymem)))
    return 1;

  if (g1.bitfield.class != Reg
      && g1.bitfield.class != RegSIMD
      && (g1.bitfield.unspecified
	  || !operand_type_check (g1, anymem)))
    return 1;

  if (g0.bitfield.byte == g1.bitfield.byte
      && g0.bitfield.word == g1.bitfield.word
      && g0.bitfield.dword == g1.bitfield.dword
      && g0.bitfield.qword == g1.bitfield.qword
      && g0.bitfield.xmmword == g1.bitfield.xmmword
      && g0.bitfield.ymmword == g1.bitfield.ymmword
      && g0.bitfield.zmmword == g1.bitfield.zmmword)
    return 1;

  /* If expectations overlap in no more than a single size, all is fine. */
  g0 = operand_type_and (t0, t1);
  if (g0.bitfield.byte
      + g0.bitfield.word
      + g0.bitfield.dword
      + g0.bitfield.qword
      + g0.bitfield.xmmword
      + g0.bitfield.ymmword
      + g0.bitfield.zmmword <= 1)
    return 1;

  i.error = register_type_mismatch;

  return 0;
}

static INLINE unsigned int
register_number (const reg_entry *r)
{
  unsigned int nr = r->reg_num;

  if (r->reg_flags & RegRex)
    nr += 8;

  if (r->reg_flags & RegVRex)
    nr += 16;

  return nr;
}

static INLINE unsigned int
mode_from_disp_size (i386_operand_type t)
{
  if (t.bitfield.disp8)
    return 1;
  else if (t.bitfield.disp16
	   || t.bitfield.disp32)
    return 2;
  else
    return 0;
}

static INLINE int
fits_in_signed_byte (addressT num)
{
  return num + 0x80 <= 0xff;
}

static INLINE int
fits_in_unsigned_byte (addressT num)
{
  return num <= 0xff;
}

static INLINE int
fits_in_unsigned_word (addressT num)
{
  return num <= 0xffff;
}

static INLINE int
fits_in_signed_word (addressT num)
{
  return num + 0x8000 <= 0xffff;
}

static INLINE int
fits_in_signed_long (addressT num ATTRIBUTE_UNUSED)
{
#ifndef BFD64
  return 1;
#else
  return num + 0x80000000 <= 0xffffffff;
#endif
}				/* fits_in_signed_long() */

static INLINE int
fits_in_unsigned_long (addressT num ATTRIBUTE_UNUSED)
{
#ifndef BFD64
  return 1;
#else
  return num <= 0xffffffff;
#endif
}				/* fits_in_unsigned_long() */

static INLINE valueT extend_to_32bit_address (addressT num)
{
#ifdef BFD64
  if (fits_in_unsigned_long(num))
    return (num ^ ((addressT) 1 << 31)) - ((addressT) 1 << 31);

  if (!fits_in_signed_long (num))
    return num & 0xffffffff;
#endif

  return num;
}

static INLINE int
fits_in_disp8 (offsetT num)
{
  int shift = i.memshift;
  unsigned int mask;

  if (shift == -1)
    abort ();

  mask = (1 << shift) - 1;

  /* Return 0 if NUM isn't properly aligned.  */
  if ((num & mask))
    return 0;

  /* Check if NUM will fit in 8bit after shift.  */
  return fits_in_signed_byte (num >> shift);
}

static INLINE int
fits_in_imm4 (offsetT num)
{
  /* Despite the name, check for imm3 if we're dealing with EVEX.  */
  return (num & (i.vec_encoding != vex_encoding_evex ? 0xf : 7)) == num;
}

static i386_operand_type
smallest_imm_type (offsetT num)
{
  i386_operand_type t;

  operand_type_set (&t, 0);
  t.bitfield.imm64 = 1;

  if (cpu_arch_tune != PROCESSOR_I486 && num == 1)
    {
      /* This code is disabled on the 486 because all the Imm1 forms
	 in the opcode table are slower on the i486.  They're the
	 versions with the implicitly specified single-position
	 displacement, which has another syntax if you really want to
	 use that form.  */
      t.bitfield.imm1 = 1;
      t.bitfield.imm8 = 1;
      t.bitfield.imm8s = 1;
      t.bitfield.imm16 = 1;
      t.bitfield.imm32 = 1;
      t.bitfield.imm32s = 1;
    }
  else if (fits_in_signed_byte (num))
    {
      if (fits_in_unsigned_byte (num))
	t.bitfield.imm8 = 1;
      t.bitfield.imm8s = 1;
      t.bitfield.imm16 = 1;
      t.bitfield.imm32 = 1;
      t.bitfield.imm32s = 1;
    }
  else if (fits_in_unsigned_byte (num))
    {
      t.bitfield.imm8 = 1;
      t.bitfield.imm16 = 1;
      t.bitfield.imm32 = 1;
      t.bitfield.imm32s = 1;
    }
  else if (fits_in_signed_word (num) || fits_in_unsigned_word (num))
    {
      t.bitfield.imm16 = 1;
      t.bitfield.imm32 = 1;
      t.bitfield.imm32s = 1;
    }
  else if (fits_in_signed_long (num))
    {
      t.bitfield.imm32 = 1;
      t.bitfield.imm32s = 1;
    }
  else if (fits_in_unsigned_long (num))
    t.bitfield.imm32 = 1;

  return t;
}

static offsetT
offset_in_range (offsetT val, int size)
{
  addressT mask;

  switch (size)
    {
    case 1: mask = ((addressT) 1 <<  8) - 1; break;
    case 2: mask = ((addressT) 1 << 16) - 1; break;
#ifdef BFD64
    case 4: mask = ((addressT) 1 << 32) - 1; break;
#endif
    case sizeof (val): return val;
    default: abort ();
    }

  if ((val & ~mask) != 0 && (-val & ~mask) != 0)
    as_warn (_("0x%" PRIx64 " shortened to 0x%" PRIx64),
	     (uint64_t) val, (uint64_t) (val & mask));

  return val & mask;
}

static INLINE const char *insn_name (const insn_template *t)
{
  return &i386_mnemonics[t->mnem_off];
}

enum PREFIX_GROUP
{
  PREFIX_EXIST = 0,
  PREFIX_LOCK,
  PREFIX_REP,
  PREFIX_DS,
  PREFIX_OTHER
};

/* Returns
   a. PREFIX_EXIST if attempting to add a prefix where one from the
   same class already exists.
   b. PREFIX_LOCK if lock prefix is added.
   c. PREFIX_REP if rep/repne prefix is added.
   d. PREFIX_DS if ds prefix is added.
   e. PREFIX_OTHER if other prefix is added.
 */

static enum PREFIX_GROUP
add_prefix (unsigned int prefix)
{
  enum PREFIX_GROUP ret = PREFIX_OTHER;
  unsigned int q;

  if (prefix >= REX_OPCODE && prefix < REX_OPCODE + 16
      && flag_code == CODE_64BIT)
    {
      if ((i.prefix[REX_PREFIX] & prefix & REX_W)
	  || (i.prefix[REX_PREFIX] & prefix & REX_R)
	  || (i.prefix[REX_PREFIX] & prefix & REX_X)
	  || (i.prefix[REX_PREFIX] & prefix & REX_B))
	ret = PREFIX_EXIST;
      q = REX_PREFIX;
    }
  else
    {
      switch (prefix)
	{
	default:
	  abort ();

	case DS_PREFIX_OPCODE:
	  ret = PREFIX_DS;
	  /* Fall through.  */
	case CS_PREFIX_OPCODE:
	case ES_PREFIX_OPCODE:
	case FS_PREFIX_OPCODE:
	case GS_PREFIX_OPCODE:
	case SS_PREFIX_OPCODE:
	  q = SEG_PREFIX;
	  break;

	case REPNE_PREFIX_OPCODE:
	case REPE_PREFIX_OPCODE:
	  q = REP_PREFIX;
	  ret = PREFIX_REP;
	  break;

	case LOCK_PREFIX_OPCODE:
	  q = LOCK_PREFIX;
	  ret = PREFIX_LOCK;
	  break;

	case FWAIT_OPCODE:
	  q = WAIT_PREFIX;
	  break;

	case ADDR_PREFIX_OPCODE:
	  q = ADDR_PREFIX;
	  break;

	case DATA_PREFIX_OPCODE:
	  q = DATA_PREFIX;
	  break;
	}
      if (i.prefix[q] != 0)
	ret = PREFIX_EXIST;
    }

  if (ret)
    {
      if (!i.prefix[q])
	++i.prefixes;
      i.prefix[q] |= prefix;
    }
  else
    as_bad (_("same type of prefix used twice"));

  return ret;
}

static void
update_code_flag (int value, int check)
{
  PRINTF_LIKE ((*as_error));

  flag_code = (enum flag_code) value;
  if (flag_code == CODE_64BIT)
    {
      cpu_arch_flags.bitfield.cpu64 = 1;
      cpu_arch_flags.bitfield.cpuno64 = 0;
    }
  else
    {
      cpu_arch_flags.bitfield.cpu64 = 0;
      cpu_arch_flags.bitfield.cpuno64 = 1;
    }
  if (value == CODE_64BIT && !cpu_arch_flags.bitfield.cpulm )
    {
      if (check)
	as_error = as_fatal;
      else
	as_error = as_bad;
      (*as_error) (_("64bit mode not supported on `%s'."),
		   cpu_arch_name ? cpu_arch_name : default_arch);
    }
  if (value == CODE_32BIT && !cpu_arch_flags.bitfield.cpui386)
    {
      if (check)
	as_error = as_fatal;
      else
	as_error = as_bad;
      (*as_error) (_("32bit mode not supported on `%s'."),
		   cpu_arch_name ? cpu_arch_name : default_arch);
    }
  stackop_size = '\0';
}

static void
set_code_flag (int value)
{
  update_code_flag (value, 0);
}

static void
set_16bit_gcc_code_flag (int new_code_flag)
{
  flag_code = (enum flag_code) new_code_flag;
  if (flag_code != CODE_16BIT)
    abort ();
  cpu_arch_flags.bitfield.cpu64 = 0;
  cpu_arch_flags.bitfield.cpuno64 = 1;
  stackop_size = LONG_MNEM_SUFFIX;
}

static void
set_intel_syntax (int syntax_flag)
{
  /* Find out if register prefixing is specified.  */
  int ask_naked_reg = 0;

  SKIP_WHITESPACE ();
  if (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      char *string;
      int e = get_symbol_name (&string);

      if (strcmp (string, "prefix") == 0)
	ask_naked_reg = 1;
      else if (strcmp (string, "noprefix") == 0)
	ask_naked_reg = -1;
      else
	as_bad (_("bad argument to syntax directive."));
      (void) restore_line_pointer (e);
    }
  demand_empty_rest_of_line ();

  intel_syntax = syntax_flag;

  if (ask_naked_reg == 0)
    allow_naked_reg = (intel_syntax
		       && (bfd_get_symbol_leading_char (stdoutput) != '\0'));
  else
    allow_naked_reg = (ask_naked_reg < 0);

  expr_set_rank (O_full_ptr, syntax_flag ? 10 : 0);

  register_prefix = allow_naked_reg ? "" : "%";
}

static void
set_intel_mnemonic (int mnemonic_flag)
{
  intel_mnemonic = mnemonic_flag;
}

static void
set_allow_index_reg (int flag)
{
  allow_index_reg = flag;
}

static void
set_check (int what)
{
  enum check_kind *kind;
  const char *str;

  if (what)
    {
      kind = &operand_check;
      str = "operand";
    }
  else
    {
      kind = &sse_check;
      str = "sse";
    }

  SKIP_WHITESPACE ();

  if (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      char *string;
      int e = get_symbol_name (&string);

      if (strcmp (string, "none") == 0)
	*kind = check_none;
      else if (strcmp (string, "warning") == 0)
	*kind = check_warning;
      else if (strcmp (string, "error") == 0)
	*kind = check_error;
      else
	as_bad (_("bad argument to %s_check directive."), str);
      (void) restore_line_pointer (e);
    }
  else
    as_bad (_("missing argument for %s_check directive"), str);

  demand_empty_rest_of_line ();
}

static void
check_cpu_arch_compatible (const char *name ATTRIBUTE_UNUSED,
			   i386_cpu_flags new_flag ATTRIBUTE_UNUSED)
{
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  static const char *arch;

  /* Intel MCU is only supported on ELF.  */
  if (!IS_ELF)
    return;

  if (!arch)
    {
      /* Use cpu_arch_name if it is set in md_parse_option.  Otherwise
	 use default_arch.  */
      arch = cpu_arch_name;
      if (!arch)
	arch = default_arch;
    }

  /* If we are targeting Intel MCU, we must enable it.  */
  if ((get_elf_backend_data (stdoutput)->elf_machine_code == EM_IAMCU)
      == new_flag.bitfield.cpuiamcu)
    return;

  as_bad (_("`%s' is not supported on `%s'"), name, arch);
#endif
}

static void
extend_cpu_sub_arch_name (const char *name)
{
  if (cpu_sub_arch_name)
    cpu_sub_arch_name = reconcat (cpu_sub_arch_name, cpu_sub_arch_name,
				  ".", name, (const char *) NULL);
  else
    cpu_sub_arch_name = concat (".", name, (const char *) NULL);
}

static void
set_cpu_arch (int dummy ATTRIBUTE_UNUSED)
{
  typedef struct arch_stack_entry
  {
    const struct arch_stack_entry *prev;
    const char *name;
    char *sub_name;
    i386_cpu_flags flags;
    i386_cpu_flags isa_flags;
    enum processor_type isa;
    enum flag_code flag_code;
    char stackop_size;
    bool no_cond_jump_promotion;
  } arch_stack_entry;
  static const arch_stack_entry *arch_stack_top;

  SKIP_WHITESPACE ();

  if (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      char *s;
      int e = get_symbol_name (&s);
      const char *string = s;
      unsigned int j = 0;
      i386_cpu_flags flags;

      if (strcmp (string, "default") == 0)
	{
	  if (strcmp (default_arch, "iamcu") == 0)
	    string = default_arch;
	  else
	    {
	      static const i386_cpu_flags cpu_unknown_flags = CPU_UNKNOWN_FLAGS;

	      cpu_arch_name = NULL;
	      free (cpu_sub_arch_name);
	      cpu_sub_arch_name = NULL;
	      cpu_arch_flags = cpu_unknown_flags;
	      if (flag_code == CODE_64BIT)
		{
		  cpu_arch_flags.bitfield.cpu64 = 1;
		  cpu_arch_flags.bitfield.cpuno64 = 0;
		}
	      else
		{
		  cpu_arch_flags.bitfield.cpu64 = 0;
		  cpu_arch_flags.bitfield.cpuno64 = 1;
		}
	      cpu_arch_isa = PROCESSOR_UNKNOWN;
	      cpu_arch_isa_flags = cpu_arch[flag_code == CODE_64BIT].enable;
	      if (!cpu_arch_tune_set)
		{
		  cpu_arch_tune = cpu_arch_isa;
		  cpu_arch_tune_flags = cpu_arch_isa_flags;
		}

	      j = ARRAY_SIZE (cpu_arch) + 1;
	    }
	}
      else if (strcmp (string, "push") == 0)
	{
	  arch_stack_entry *top = XNEW (arch_stack_entry);

	  top->name = cpu_arch_name;
	  if (cpu_sub_arch_name)
	    top->sub_name = xstrdup (cpu_sub_arch_name);
	  else
	    top->sub_name = NULL;
	  top->flags = cpu_arch_flags;
	  top->isa = cpu_arch_isa;
	  top->isa_flags = cpu_arch_isa_flags;
	  top->flag_code = flag_code;
	  top->stackop_size = stackop_size;
	  top->no_cond_jump_promotion = no_cond_jump_promotion;

	  top->prev = arch_stack_top;
	  arch_stack_top = top;

	  (void) restore_line_pointer (e);
	  demand_empty_rest_of_line ();
	  return;
	}
      else if (strcmp (string, "pop") == 0)
	{
	  const arch_stack_entry *top = arch_stack_top;

	  if (!top)
	    as_bad (_(".arch stack is empty"));
	  else if (top->flag_code != flag_code
		   || top->stackop_size != stackop_size)
	    {
	      static const unsigned int bits[] = {
	        [CODE_16BIT] = 16,
	        [CODE_32BIT] = 32,
	        [CODE_64BIT] = 64,
	      };

	      as_bad (_("this `.arch pop' requires `.code%u%s' to be in effect"),
		      bits[top->flag_code],
		      top->stackop_size == LONG_MNEM_SUFFIX ? "gcc" : "");
	    }
	  else
	    {
	      arch_stack_top = top->prev;

	      cpu_arch_name = top->name;
	      free (cpu_sub_arch_name);
	      cpu_sub_arch_name = top->sub_name;
	      cpu_arch_flags = top->flags;
	      cpu_arch_isa = top->isa;
	      cpu_arch_isa_flags = top->isa_flags;
	      no_cond_jump_promotion = top->no_cond_jump_promotion;

	      XDELETE (top);
	    }

	  (void) restore_line_pointer (e);
	  demand_empty_rest_of_line ();
	  return;
	}

      for (; j < ARRAY_SIZE (cpu_arch); j++)
	{
	  if (strcmp (string + (*string == '.'), cpu_arch[j].name) == 0
	     && (*string == '.') == (cpu_arch[j].type == PROCESSOR_NONE))
	    {
	      if (*string != '.')
		{
		  check_cpu_arch_compatible (string, cpu_arch[j].enable);

		  cpu_arch_name = cpu_arch[j].name;
		  free (cpu_sub_arch_name);
		  cpu_sub_arch_name = NULL;
		  cpu_arch_flags = cpu_arch[j].enable;
		  if (flag_code == CODE_64BIT)
		    {
		      cpu_arch_flags.bitfield.cpu64 = 1;
		      cpu_arch_flags.bitfield.cpuno64 = 0;
		    }
		  else
		    {
		      cpu_arch_flags.bitfield.cpu64 = 0;
		      cpu_arch_flags.bitfield.cpuno64 = 1;
		    }
		  cpu_arch_isa = cpu_arch[j].type;
		  cpu_arch_isa_flags = cpu_arch[j].enable;
		  if (!cpu_arch_tune_set)
		    {
		      cpu_arch_tune = cpu_arch_isa;
		      cpu_arch_tune_flags = cpu_arch_isa_flags;
		    }
		  pre_386_16bit_warned = false;
		  break;
		}

	      if (cpu_flags_all_zero (&cpu_arch[j].enable))
	        continue;

	      flags = cpu_flags_or (cpu_arch_flags,
				    cpu_arch[j].enable);

	      if (!cpu_flags_equal (&flags, &cpu_arch_flags))
		{
		  extend_cpu_sub_arch_name (string + 1);
		  cpu_arch_flags = flags;
		  cpu_arch_isa_flags = flags;
		}
	      else
		cpu_arch_isa_flags
		  = cpu_flags_or (cpu_arch_isa_flags,
				  cpu_arch[j].enable);
	      (void) restore_line_pointer (e);
	      demand_empty_rest_of_line ();
	      return;
	    }
	}

      if (startswith (string, ".no") && j >= ARRAY_SIZE (cpu_arch))
	{
	  /* Disable an ISA extension.  */
	  for (j = 0; j < ARRAY_SIZE (cpu_arch); j++)
	    if (cpu_arch[j].type == PROCESSOR_NONE
	        && strcmp (string + 3, cpu_arch[j].name) == 0)
	      {
		flags = cpu_flags_and_not (cpu_arch_flags,
					   cpu_arch[j].disable);
		if (!cpu_flags_equal (&flags, &cpu_arch_flags))
		  {
		    extend_cpu_sub_arch_name (string + 1);
		    cpu_arch_flags = flags;
		    cpu_arch_isa_flags = flags;
		  }
		(void) restore_line_pointer (e);
		demand_empty_rest_of_line ();
		return;
	      }
	}

      if (j == ARRAY_SIZE (cpu_arch))
	as_bad (_("no such architecture: `%s'"), string);

      *input_line_pointer = e;
    }
  else
    as_bad (_("missing cpu architecture"));

  no_cond_jump_promotion = 0;
  if (*input_line_pointer == ','
      && !is_end_of_line[(unsigned char) input_line_pointer[1]])
    {
      char *string;
      char e;

      ++input_line_pointer;
      e = get_symbol_name (&string);

      if (strcmp (string, "nojumps") == 0)
	no_cond_jump_promotion = 1;
      else if (strcmp (string, "jumps") == 0)
	;
      else
	as_bad (_("no such architecture modifier: `%s'"), string);

      (void) restore_line_pointer (e);
    }

  demand_empty_rest_of_line ();
}

enum bfd_architecture
i386_arch (void)
{
  if (cpu_arch_isa == PROCESSOR_IAMCU)
    {
      if (OUTPUT_FLAVOR != bfd_target_elf_flavour
	  || flag_code == CODE_64BIT)
	as_fatal (_("Intel MCU is 32bit ELF only"));
      return bfd_arch_iamcu;
    }
  else
    return bfd_arch_i386;
}

unsigned long
i386_mach (void)
{
  if (startswith (default_arch, "x86_64"))
    {
      if (default_arch[6] == '\0')
	return bfd_mach_x86_64;
      else
	return bfd_mach_x64_32;
    }
  else if (!strcmp (default_arch, "i386")
	   || !strcmp (default_arch, "iamcu"))
    {
      if (cpu_arch_isa == PROCESSOR_IAMCU)
	{
	  if (OUTPUT_FLAVOR != bfd_target_elf_flavour)
	    as_fatal (_("Intel MCU is 32bit ELF only"));
	  return bfd_mach_i386_iamcu;
	}
      else
	return bfd_mach_i386_i386;
    }
  else
    as_fatal (_("unknown architecture"));
}

#include "opcodes/i386-tbl.h"

void
md_begin (void)
{
  /* Support pseudo prefixes like {disp32}.  */
  lex_type ['{'] = LEX_BEGIN_NAME;

  /* Initialize op_hash hash table.  */
  op_hash = str_htab_create ();

  {
    const insn_template *const *sets = i386_op_sets;
    const insn_template *const *end = sets + ARRAY_SIZE (i386_op_sets) - 1;

    /* Type checks to compensate for the conversion through void * which
       occurs during hash table insertion / lookup.  */
    (void) sizeof (sets == &current_templates->start);
    (void) sizeof (end == &current_templates->end);
    for (; sets < end; ++sets)
      if (str_hash_insert (op_hash, insn_name (*sets), sets, 0))
	as_fatal (_("duplicate %s"), insn_name (*sets));
  }

  /* Initialize reg_hash hash table.  */
  reg_hash = str_htab_create ();
  {
    const reg_entry *regtab;
    unsigned int regtab_size = i386_regtab_size;

    for (regtab = i386_regtab; regtab_size--; regtab++)
      {
	switch (regtab->reg_type.bitfield.class)
	  {
	  case Reg:
	    if (regtab->reg_type.bitfield.dword)
	      {
		if (regtab->reg_type.bitfield.instance == Accum)
		  reg_eax = regtab;
	      }
	    else if (regtab->reg_type.bitfield.tbyte)
	      {
		/* There's no point inserting st(<N>) in the hash table, as
		   parentheses aren't included in register_chars[] anyway.  */
		if (regtab->reg_type.bitfield.instance != Accum)
		  continue;
		reg_st0 = regtab;
	      }
	    break;

	  case SReg:
	    switch (regtab->reg_num)
	      {
	      case 0: reg_es = regtab; break;
	      case 2: reg_ss = regtab; break;
	      case 3: reg_ds = regtab; break;
	      }
	    break;

	  case RegMask:
	    if (!regtab->reg_num)
	      reg_k0 = regtab;
	    break;
	  }

	if (str_hash_insert (reg_hash, regtab->reg_name, regtab, 0) != NULL)
	  as_fatal (_("duplicate %s"), regtab->reg_name);
      }
  }

  /* Fill in lexical tables:  mnemonic_chars, operand_chars.  */
  {
    int c;
    const char *p;

    for (c = 0; c < 256; c++)
      {
	if (ISDIGIT (c) || ISLOWER (c))
	  {
	    mnemonic_chars[c] = c;
	    register_chars[c] = c;
	    operand_chars[c] = c;
	  }
	else if (ISUPPER (c))
	  {
	    mnemonic_chars[c] = TOLOWER (c);
	    register_chars[c] = mnemonic_chars[c];
	    operand_chars[c] = c;
	  }
#ifdef SVR4_COMMENT_CHARS
	else if (c == '\\' && strchr (i386_comment_chars, '/'))
	  operand_chars[c] = c;
#endif

	if (c >= 128)
	  operand_chars[c] = c;
      }

    mnemonic_chars['_'] = '_';
    mnemonic_chars['-'] = '-';
    mnemonic_chars['.'] = '.';

    for (p = extra_symbol_chars; *p != '\0'; p++)
      operand_chars[(unsigned char) *p] = *p;
    for (p = operand_special_chars; *p != '\0'; p++)
      operand_chars[(unsigned char) *p] = *p;
  }

  if (flag_code == CODE_64BIT)
    {
#if defined (OBJ_COFF) && defined (TE_PE)
      x86_dwarf2_return_column = (OUTPUT_FLAVOR == bfd_target_coff_flavour
				  ? 32 : 16);
#else
      x86_dwarf2_return_column = 16;
#endif
      x86_cie_data_alignment = -8;
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
      x86_sframe_cfa_sp_reg = 7;
      x86_sframe_cfa_fp_reg = 6;
#endif
    }
  else
    {
      x86_dwarf2_return_column = 8;
      x86_cie_data_alignment = -4;
    }

  /* NB: FUSED_JCC_PADDING frag must have sufficient room so that it
     can be turned into BRANCH_PREFIX frag.  */
  if (align_branch_prefix_size > MAX_FUSED_JCC_PADDING_SIZE)
    abort ();
}

void
i386_print_statistics (FILE *file)
{
  htab_print_statistics (file, "i386 opcode", op_hash);
  htab_print_statistics (file, "i386 register", reg_hash);
}

void
i386_md_end (void)
{
  htab_delete (op_hash);
  htab_delete (reg_hash);
}

#ifdef DEBUG386

/* Debugging routines for md_assemble.  */
static void pte (insn_template *);
static void pt (i386_operand_type);
static void pe (expressionS *);
static void ps (symbolS *);

static void
pi (const char *line, i386_insn *x)
{
  unsigned int j;

  fprintf (stdout, "%s: template ", line);
  pte (&x->tm);
  fprintf (stdout, "  address: base %s  index %s  scale %x\n",
	   x->base_reg ? x->base_reg->reg_name : "none",
	   x->index_reg ? x->index_reg->reg_name : "none",
	   x->log2_scale_factor);
  fprintf (stdout, "  modrm:  mode %x  reg %x  reg/mem %x\n",
	   x->rm.mode, x->rm.reg, x->rm.regmem);
  fprintf (stdout, "  sib:  base %x  index %x  scale %x\n",
	   x->sib.base, x->sib.index, x->sib.scale);
  fprintf (stdout, "  rex: 64bit %x  extX %x  extY %x  extZ %x\n",
	   (x->rex & REX_W) != 0,
	   (x->rex & REX_R) != 0,
	   (x->rex & REX_X) != 0,
	   (x->rex & REX_B) != 0);
  for (j = 0; j < x->operands; j++)
    {
      fprintf (stdout, "    #%d:  ", j + 1);
      pt (x->types[j]);
      fprintf (stdout, "\n");
      if (x->types[j].bitfield.class == Reg
	  || x->types[j].bitfield.class == RegMMX
	  || x->types[j].bitfield.class == RegSIMD
	  || x->types[j].bitfield.class == RegMask
	  || x->types[j].bitfield.class == SReg
	  || x->types[j].bitfield.class == RegCR
	  || x->types[j].bitfield.class == RegDR
	  || x->types[j].bitfield.class == RegTR
	  || x->types[j].bitfield.class == RegBND)
	fprintf (stdout, "%s\n", x->op[j].regs->reg_name);
      if (operand_type_check (x->types[j], imm))
	pe (x->op[j].imms);
      if (operand_type_check (x->types[j], disp))
	pe (x->op[j].disps);
    }
}

static void
pte (insn_template *t)
{
  static const unsigned char opc_pfx[] = { 0, 0x66, 0xf3, 0xf2 };
  static const char *const opc_spc[] = {
    NULL, "0f", "0f38", "0f3a", NULL, "evexmap5", "evexmap6", NULL,
    "XOP08", "XOP09", "XOP0A",
  };
  unsigned int j;

  fprintf (stdout, " %d operands ", t->operands);
  if (opc_pfx[t->opcode_modifier.opcodeprefix])
    fprintf (stdout, "pfx %x ", opc_pfx[t->opcode_modifier.opcodeprefix]);
  if (opc_spc[t->opcode_space])
    fprintf (stdout, "space %s ", opc_spc[t->opcode_space]);
  fprintf (stdout, "opcode %x ", t->base_opcode);
  if (t->extension_opcode != None)
    fprintf (stdout, "ext %x ", t->extension_opcode);
  if (t->opcode_modifier.d)
    fprintf (stdout, "D");
  if (t->opcode_modifier.w)
    fprintf (stdout, "W");
  fprintf (stdout, "\n");
  for (j = 0; j < t->operands; j++)
    {
      fprintf (stdout, "    #%d type ", j + 1);
      pt (t->operand_types[j]);
      fprintf (stdout, "\n");
    }
}

static void
pe (expressionS *e)
{
  fprintf (stdout, "    operation     %d\n", e->X_op);
  fprintf (stdout, "    add_number    %" PRId64 " (%" PRIx64 ")\n",
	   (int64_t) e->X_add_number, (uint64_t) (valueT) e->X_add_number);
  if (e->X_add_symbol)
    {
      fprintf (stdout, "    add_symbol    ");
      ps (e->X_add_symbol);
      fprintf (stdout, "\n");
    }
  if (e->X_op_symbol)
    {
      fprintf (stdout, "    op_symbol    ");
      ps (e->X_op_symbol);
      fprintf (stdout, "\n");
    }
}

static void
ps (symbolS *s)
{
  fprintf (stdout, "%s type %s%s",
	   S_GET_NAME (s),
	   S_IS_EXTERNAL (s) ? "EXTERNAL " : "",
	   segment_name (S_GET_SEGMENT (s)));
}

static struct type_name
  {
    i386_operand_type mask;
    const char *name;
  }
const type_names[] =
{
  { { .bitfield = { .class = Reg, .byte = 1 } }, "r8" },
  { { .bitfield = { .class = Reg, .word = 1 } }, "r16" },
  { { .bitfield = { .class = Reg, .dword = 1 } }, "r32" },
  { { .bitfield = { .class = Reg, .qword = 1 } }, "r64" },
  { { .bitfield = { .instance = Accum, .byte = 1 } }, "acc8" },
  { { .bitfield = { .instance = Accum, .word = 1 } }, "acc16" },
  { { .bitfield = { .instance = Accum, .dword = 1 } }, "acc32" },
  { { .bitfield = { .instance = Accum, .qword = 1 } }, "acc64" },
  { { .bitfield = { .imm8 = 1 } }, "i8" },
  { { .bitfield = { .imm8s = 1 } }, "i8s" },
  { { .bitfield = { .imm16 = 1 } }, "i16" },
  { { .bitfield = { .imm32 = 1 } }, "i32" },
  { { .bitfield = { .imm32s = 1 } }, "i32s" },
  { { .bitfield = { .imm64 = 1 } }, "i64" },
  { { .bitfield = { .imm1 = 1 } }, "i1" },
  { { .bitfield = { .baseindex = 1 } }, "BaseIndex" },
  { { .bitfield = { .disp8 = 1 } }, "d8" },
  { { .bitfield = { .disp16 = 1 } }, "d16" },
  { { .bitfield = { .disp32 = 1 } }, "d32" },
  { { .bitfield = { .disp64 = 1 } }, "d64" },
  { { .bitfield = { .instance = RegD, .word = 1 } }, "InOutPortReg" },
  { { .bitfield = { .instance = RegC, .byte = 1 } }, "ShiftCount" },
  { { .bitfield = { .class = RegCR } }, "control reg" },
  { { .bitfield = { .class = RegTR } }, "test reg" },
  { { .bitfield = { .class = RegDR } }, "debug reg" },
  { { .bitfield = { .class = Reg, .tbyte = 1 } }, "FReg" },
  { { .bitfield = { .instance = Accum, .tbyte = 1 } }, "FAcc" },
  { { .bitfield = { .class = SReg } }, "SReg" },
  { { .bitfield = { .class = RegMMX } }, "rMMX" },
  { { .bitfield = { .class = RegSIMD, .xmmword = 1 } }, "rXMM" },
  { { .bitfield = { .class = RegSIMD, .ymmword = 1 } }, "rYMM" },
  { { .bitfield = { .class = RegSIMD, .zmmword = 1 } }, "rZMM" },
  { { .bitfield = { .class = RegSIMD, .tmmword = 1 } }, "rTMM" },
  { { .bitfield = { .class = RegMask } }, "Mask reg" },
};

static void
pt (i386_operand_type t)
{
  unsigned int j;
  i386_operand_type a;

  for (j = 0; j < ARRAY_SIZE (type_names); j++)
    {
      a = operand_type_and (t, type_names[j].mask);
      if (operand_type_equal (&a, &type_names[j].mask))
	fprintf (stdout, "%s, ",  type_names[j].name);
    }
  fflush (stdout);
}

#endif /* DEBUG386 */

static bfd_reloc_code_real_type
reloc (unsigned int size,
       int pcrel,
       int sign,
       bfd_reloc_code_real_type other)
{
  if (other != NO_RELOC)
    {
      reloc_howto_type *rel;

      if (size == 8)
	switch (other)
	  {
	  case BFD_RELOC_X86_64_GOT32:
	    return BFD_RELOC_X86_64_GOT64;
	    break;
	  case BFD_RELOC_X86_64_GOTPLT64:
	    return BFD_RELOC_X86_64_GOTPLT64;
	    break;
	  case BFD_RELOC_X86_64_PLTOFF64:
	    return BFD_RELOC_X86_64_PLTOFF64;
	    break;
	  case BFD_RELOC_X86_64_GOTPC32:
	    other = BFD_RELOC_X86_64_GOTPC64;
	    break;
	  case BFD_RELOC_X86_64_GOTPCREL:
	    other = BFD_RELOC_X86_64_GOTPCREL64;
	    break;
	  case BFD_RELOC_X86_64_TPOFF32:
	    other = BFD_RELOC_X86_64_TPOFF64;
	    break;
	  case BFD_RELOC_X86_64_DTPOFF32:
	    other = BFD_RELOC_X86_64_DTPOFF64;
	    break;
	  default:
	    break;
	  }

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
      if (other == BFD_RELOC_SIZE32)
	{
	  if (size == 8)
	    other = BFD_RELOC_SIZE64;
	  if (pcrel)
	    {
	      as_bad (_("there are no pc-relative size relocations"));
	      return NO_RELOC;
	    }
	}
#endif

      /* Sign-checking 4-byte relocations in 16-/32-bit code is pointless.  */
      if (size == 4 && (flag_code != CODE_64BIT || disallow_64bit_reloc))
	sign = -1;

      rel = bfd_reloc_type_lookup (stdoutput, other);
      if (!rel)
	as_bad (_("unknown relocation (%u)"), other);
      else if (size != bfd_get_reloc_size (rel))
	as_bad (_("%u-byte relocation cannot be applied to %u-byte field"),
		bfd_get_reloc_size (rel),
		size);
      else if (pcrel && !rel->pc_relative)
	as_bad (_("non-pc-relative relocation for pc-relative field"));
      else if ((rel->complain_on_overflow == complain_overflow_signed
		&& !sign)
	       || (rel->complain_on_overflow == complain_overflow_unsigned
		   && sign > 0))
	as_bad (_("relocated field and relocation type differ in signedness"));
      else
	return other;
      return NO_RELOC;
    }

  if (pcrel)
    {
      if (!sign)
	as_bad (_("there are no unsigned pc-relative relocations"));
      switch (size)
	{
	case 1: return BFD_RELOC_8_PCREL;
	case 2: return BFD_RELOC_16_PCREL;
	case 4: return BFD_RELOC_32_PCREL;
	case 8: return BFD_RELOC_64_PCREL;
	}
      as_bad (_("cannot do %u byte pc-relative relocation"), size);
    }
  else
    {
      if (sign > 0)
	switch (size)
	  {
	  case 4: return BFD_RELOC_X86_64_32S;
	  }
      else
	switch (size)
	  {
	  case 1: return BFD_RELOC_8;
	  case 2: return BFD_RELOC_16;
	  case 4: return BFD_RELOC_32;
	  case 8: return BFD_RELOC_64;
	  }
      as_bad (_("cannot do %s %u byte relocation"),
	      sign > 0 ? "signed" : "unsigned", size);
    }

  return NO_RELOC;
}

/* Here we decide which fixups can be adjusted to make them relative to
   the beginning of the section instead of the symbol.  Basically we need
   to make sure that the dynamic relocations are done correctly, so in
   some cases we force the original symbol to be used.  */

int
tc_i386_fix_adjustable (fixS *fixP ATTRIBUTE_UNUSED)
{
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  if (!IS_ELF)
    return 1;

  /* Don't adjust pc-relative references to merge sections in 64-bit
     mode.  */
  if (use_rela_relocations
      && (S_GET_SEGMENT (fixP->fx_addsy)->flags & SEC_MERGE) != 0
      && fixP->fx_pcrel)
    return 0;

  /* The x86_64 GOTPCREL are represented as 32bit PCrel relocations
     and changed later by validate_fix.  */
  if (GOT_symbol && fixP->fx_subsy == GOT_symbol
      && fixP->fx_r_type == BFD_RELOC_32_PCREL)
    return 0;

  /* Adjust_reloc_syms doesn't know about the GOT.  Need to keep symbol
     for size relocations.  */
  if (fixP->fx_r_type == BFD_RELOC_SIZE32
      || fixP->fx_r_type == BFD_RELOC_SIZE64
      || fixP->fx_r_type == BFD_RELOC_386_GOTOFF
      || fixP->fx_r_type == BFD_RELOC_386_GOT32
      || fixP->fx_r_type == BFD_RELOC_386_GOT32X
      || fixP->fx_r_type == BFD_RELOC_386_TLS_GD
      || fixP->fx_r_type == BFD_RELOC_386_TLS_LDM
      || fixP->fx_r_type == BFD_RELOC_386_TLS_LDO_32
      || fixP->fx_r_type == BFD_RELOC_386_TLS_IE_32
      || fixP->fx_r_type == BFD_RELOC_386_TLS_IE
      || fixP->fx_r_type == BFD_RELOC_386_TLS_GOTIE
      || fixP->fx_r_type == BFD_RELOC_386_TLS_LE_32
      || fixP->fx_r_type == BFD_RELOC_386_TLS_LE
      || fixP->fx_r_type == BFD_RELOC_386_TLS_GOTDESC
      || fixP->fx_r_type == BFD_RELOC_386_TLS_DESC_CALL
      || fixP->fx_r_type == BFD_RELOC_X86_64_GOT32
      || fixP->fx_r_type == BFD_RELOC_X86_64_GOTPCREL
      || fixP->fx_r_type == BFD_RELOC_X86_64_GOTPCRELX
      || fixP->fx_r_type == BFD_RELOC_X86_64_REX_GOTPCRELX
      || fixP->fx_r_type == BFD_RELOC_X86_64_TLSGD
      || fixP->fx_r_type == BFD_RELOC_X86_64_TLSLD
      || fixP->fx_r_type == BFD_RELOC_X86_64_DTPOFF32
      || fixP->fx_r_type == BFD_RELOC_X86_64_DTPOFF64
      || fixP->fx_r_type == BFD_RELOC_X86_64_GOTTPOFF
      || fixP->fx_r_type == BFD_RELOC_X86_64_TPOFF32
      || fixP->fx_r_type == BFD_RELOC_X86_64_TPOFF64
      || fixP->fx_r_type == BFD_RELOC_X86_64_GOTOFF64
      || fixP->fx_r_type == BFD_RELOC_X86_64_GOTPC32_TLSDESC
      || fixP->fx_r_type == BFD_RELOC_X86_64_TLSDESC_CALL
      || fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return 0;
#endif
  return 1;
}

static INLINE bool
want_disp32 (const insn_template *t)
{
  return flag_code != CODE_64BIT
	 || i.prefix[ADDR_PREFIX]
	 || (t->mnem_off == MN_lea
	     && (!i.types[1].bitfield.qword
		|| t->opcode_modifier.size == SIZE32));
}

static int
intel_float_operand (const char *mnemonic)
{
  /* Note that the value returned is meaningful only for opcodes with (memory)
     operands, hence the code here is free to improperly handle opcodes that
     have no operands (for better performance and smaller code). */

  if (mnemonic[0] != 'f')
    return 0; /* non-math */

  switch (mnemonic[1])
    {
    /* fclex, fdecstp, fdisi, femms, feni, fincstp, finit, fsetpm, and
       the fs segment override prefix not currently handled because no
       call path can make opcodes without operands get here */
    case 'i':
      return 2 /* integer op */;
    case 'l':
      if (mnemonic[2] == 'd' && (mnemonic[3] == 'c' || mnemonic[3] == 'e'))
	return 3; /* fldcw/fldenv */
      break;
    case 'n':
      if (mnemonic[2] != 'o' /* fnop */)
	return 3; /* non-waiting control op */
      break;
    case 'r':
      if (mnemonic[2] == 's')
	return 3; /* frstor/frstpm */
      break;
    case 's':
      if (mnemonic[2] == 'a')
	return 3; /* fsave */
      if (mnemonic[2] == 't')
	{
	  switch (mnemonic[3])
	    {
	    case 'c': /* fstcw */
	    case 'd': /* fstdw */
	    case 'e': /* fstenv */
	    case 's': /* fsts[gw] */
	      return 3;
	    }
	}
      break;
    case 'x':
      if (mnemonic[2] == 'r' || mnemonic[2] == 's')
	return 0; /* fxsave/fxrstor are not really math ops */
      break;
    }

  return 1;
}

static INLINE void
install_template (const insn_template *t)
{
  unsigned int l;

  i.tm = *t;

  /* Note that for pseudo prefixes this produces a length of 1. But for them
     the length isn't interesting at all.  */
  for (l = 1; l < 4; ++l)
    if (!(t->base_opcode >> (8 * l)))
      break;

  i.opcode_length = l;
}

/* Build the VEX prefix.  */

static void
build_vex_prefix (const insn_template *t)
{
  unsigned int register_specifier;
  unsigned int vector_length;
  unsigned int w;

  /* Check register specifier.  */
  if (i.vex.register_specifier)
    {
      register_specifier =
	~register_number (i.vex.register_specifier) & 0xf;
      gas_assert ((i.vex.register_specifier->reg_flags & RegVRex) == 0);
    }
  else
    register_specifier = 0xf;

  /* Use 2-byte VEX prefix by swapping destination and source operand
     if there are more than 1 register operand.  */
  if (i.reg_operands > 1
      && i.vec_encoding != vex_encoding_vex3
      && i.dir_encoding == dir_encoding_default
      && i.operands == i.reg_operands
      && operand_type_equal (&i.types[0], &i.types[i.operands - 1])
      && i.tm.opcode_space == SPACE_0F
      && (i.tm.opcode_modifier.load || i.tm.opcode_modifier.d)
      && i.rex == REX_B)
    {
      unsigned int xchg;

      swap_2_operands (0, i.operands - 1);

      gas_assert (i.rm.mode == 3);

      i.rex = REX_R;
      xchg = i.rm.regmem;
      i.rm.regmem = i.rm.reg;
      i.rm.reg = xchg;

      if (i.tm.opcode_modifier.d)
	i.tm.base_opcode ^= (i.tm.base_opcode & 0xee) != 0x6e
			    ? Opcode_ExtD : Opcode_SIMD_IntD;
      else /* Use the next insn.  */
	install_template (&t[1]);
    }

  /* Use 2-byte VEX prefix by swapping commutative source operands if there
     are no memory operands and at least 3 register ones.  */
  if (i.reg_operands >= 3
      && i.vec_encoding != vex_encoding_vex3
      && i.reg_operands == i.operands - i.imm_operands
      && i.tm.opcode_modifier.vex
      && i.tm.opcode_modifier.commutative
      && (i.tm.opcode_modifier.sse2avx
	  || (optimize > 1 && !i.no_optimize))
      && i.rex == REX_B
      && i.vex.register_specifier
      && !(i.vex.register_specifier->reg_flags & RegRex))
    {
      unsigned int xchg = i.operands - i.reg_operands;

      gas_assert (i.tm.opcode_space == SPACE_0F);
      gas_assert (!i.tm.opcode_modifier.sae);
      gas_assert (operand_type_equal (&i.types[i.operands - 2],
                                      &i.types[i.operands - 3]));
      gas_assert (i.rm.mode == 3);

      swap_2_operands (xchg, xchg + 1);

      i.rex = 0;
      xchg = i.rm.regmem | 8;
      i.rm.regmem = ~register_specifier & 0xf;
      gas_assert (!(i.rm.regmem & 8));
      i.vex.register_specifier += xchg - i.rm.regmem;
      register_specifier = ~xchg & 0xf;
    }

  if (i.tm.opcode_modifier.vex == VEXScalar)
    vector_length = avxscalar;
  else if (i.tm.opcode_modifier.vex == VEX256)
    vector_length = 1;
  else if (dot_insn () && i.tm.opcode_modifier.vex == VEX128)
    vector_length = 0;
  else
    {
      unsigned int op;

      /* Determine vector length from the last multi-length vector
	 operand.  */
      vector_length = 0;
      for (op = t->operands; op--;)
	if (t->operand_types[op].bitfield.xmmword
	    && t->operand_types[op].bitfield.ymmword
	    && i.types[op].bitfield.ymmword)
	  {
	    vector_length = 1;
	    break;
	  }
    }

  /* Check the REX.W bit and VEXW.  */
  if (i.tm.opcode_modifier.vexw == VEXWIG)
    w = (vexwig == vexw1 || (i.rex & REX_W)) ? 1 : 0;
  else if (i.tm.opcode_modifier.vexw)
    w = i.tm.opcode_modifier.vexw == VEXW1 ? 1 : 0;
  else
    w = (flag_code == CODE_64BIT ? i.rex & REX_W : vexwig == vexw1) ? 1 : 0;

  /* Use 2-byte VEX prefix if possible.  */
  if (w == 0
      && i.vec_encoding != vex_encoding_vex3
      && i.tm.opcode_space == SPACE_0F
      && (i.rex & (REX_W | REX_X | REX_B)) == 0)
    {
      /* 2-byte VEX prefix.  */
      unsigned int r;

      i.vex.length = 2;
      i.vex.bytes[0] = 0xc5;

      /* Check the REX.R bit.  */
      r = (i.rex & REX_R) ? 0 : 1;
      i.vex.bytes[1] = (r << 7
			| register_specifier << 3
			| vector_length << 2
			| i.tm.opcode_modifier.opcodeprefix);
    }
  else
    {
      /* 3-byte VEX prefix.  */
      i.vex.length = 3;

      switch (i.tm.opcode_space)
	{
	case SPACE_0F:
	case SPACE_0F38:
	case SPACE_0F3A:
	  i.vex.bytes[0] = 0xc4;
	  break;
	case SPACE_XOP08:
	case SPACE_XOP09:
	case SPACE_XOP0A:
	  i.vex.bytes[0] = 0x8f;
	  break;
	default:
	  abort ();
	}

      /* The high 3 bits of the second VEX byte are 1's compliment
	 of RXB bits from REX.  */
      i.vex.bytes[1] = ((~i.rex & 7) << 5)
		       | (!dot_insn () ? i.tm.opcode_space
				       : i.insn_opcode_space);

      i.vex.bytes[2] = (w << 7
			| register_specifier << 3
			| vector_length << 2
			| i.tm.opcode_modifier.opcodeprefix);
    }
}

static INLINE bool
is_evex_encoding (const insn_template *t)
{
  return t->opcode_modifier.evex || t->opcode_modifier.disp8memshift
	 || t->opcode_modifier.broadcast || t->opcode_modifier.masking
	 || t->opcode_modifier.sae;
}

static INLINE bool
is_any_vex_encoding (const insn_template *t)
{
  return t->opcode_modifier.vex || is_evex_encoding (t);
}

static unsigned int
get_broadcast_bytes (const insn_template *t, bool diag)
{
  unsigned int op, bytes;
  const i386_operand_type *types;

  if (i.broadcast.type)
    return (1 << (t->opcode_modifier.broadcast - 1)) * i.broadcast.type;

  gas_assert (intel_syntax);

  for (op = 0; op < t->operands; ++op)
    if (t->operand_types[op].bitfield.baseindex)
      break;

  gas_assert (op < t->operands);

  if (t->opcode_modifier.evex
      && t->opcode_modifier.evex != EVEXDYN)
    switch (i.broadcast.bytes)
      {
      case 1:
	if (t->operand_types[op].bitfield.word)
	  return 2;
      /* Fall through.  */
      case 2:
	if (t->operand_types[op].bitfield.dword)
	  return 4;
      /* Fall through.  */
      case 4:
	if (t->operand_types[op].bitfield.qword)
	  return 8;
      /* Fall through.  */
      case 8:
	if (t->operand_types[op].bitfield.xmmword)
	  return 16;
	if (t->operand_types[op].bitfield.ymmword)
	  return 32;
	if (t->operand_types[op].bitfield.zmmword)
	  return 64;
      /* Fall through.  */
      default:
        abort ();
      }

  gas_assert (op + 1 < t->operands);

  if (t->operand_types[op + 1].bitfield.xmmword
      + t->operand_types[op + 1].bitfield.ymmword
      + t->operand_types[op + 1].bitfield.zmmword > 1)
    {
      types = &i.types[op + 1];
      diag = false;
    }
  else /* Ambiguous - guess with a preference to non-AVX512VL forms.  */
    types = &t->operand_types[op];

  if (types->bitfield.zmmword)
    bytes = 64;
  else if (types->bitfield.ymmword)
    bytes = 32;
  else
    bytes = 16;

  if (diag)
    as_warn (_("ambiguous broadcast for `%s', using %u-bit form"),
	     insn_name (t), bytes * 8);

  return bytes;
}

/* Build the EVEX prefix.  */

static void
build_evex_prefix (void)
{
  unsigned int register_specifier, w;
  rex_byte vrex_used = 0;

  /* Check register specifier.  */
  if (i.vex.register_specifier)
    {
      gas_assert ((i.vrex & REX_X) == 0);

      register_specifier = i.vex.register_specifier->reg_num;
      if ((i.vex.register_specifier->reg_flags & RegRex))
	register_specifier += 8;
      /* The upper 16 registers are encoded in the fourth byte of the
	 EVEX prefix.  */
      if (!(i.vex.register_specifier->reg_flags & RegVRex))
	i.vex.bytes[3] = 0x8;
      register_specifier = ~register_specifier & 0xf;
    }
  else
    {
      register_specifier = 0xf;

      /* Encode upper 16 vector index register in the fourth byte of
	 the EVEX prefix.  */
      if (!(i.vrex & REX_X))
	i.vex.bytes[3] = 0x8;
      else
	vrex_used |= REX_X;
    }

  /* 4 byte EVEX prefix.  */
  i.vex.length = 4;
  i.vex.bytes[0] = 0x62;

  /* The high 3 bits of the second EVEX byte are 1's compliment of RXB
     bits from REX.  */
  gas_assert (i.tm.opcode_space >= SPACE_0F);
  gas_assert (i.tm.opcode_space <= SPACE_EVEXMAP6);
  i.vex.bytes[1] = ((~i.rex & 7) << 5)
		   | (!dot_insn () ? i.tm.opcode_space
				   : i.insn_opcode_space);

  /* The fifth bit of the second EVEX byte is 1's compliment of the
     REX_R bit in VREX.  */
  if (!(i.vrex & REX_R))
    i.vex.bytes[1] |= 0x10;
  else
    vrex_used |= REX_R;

  if ((i.reg_operands + i.imm_operands) == i.operands)
    {
      /* When all operands are registers, the REX_X bit in REX is not
	 used.  We reuse it to encode the upper 16 registers, which is
	 indicated by the REX_B bit in VREX.  The REX_X bit is encoded
	 as 1's compliment.  */
      if ((i.vrex & REX_B))
	{
	  vrex_used |= REX_B;
	  i.vex.bytes[1] &= ~0x40;
	}
    }

  /* EVEX instructions shouldn't need the REX prefix.  */
  i.vrex &= ~vrex_used;
  gas_assert (i.vrex == 0);

  /* Check the REX.W bit and VEXW.  */
  if (i.tm.opcode_modifier.vexw == VEXWIG)
    w = (evexwig == evexw1 || (i.rex & REX_W)) ? 1 : 0;
  else if (i.tm.opcode_modifier.vexw)
    w = i.tm.opcode_modifier.vexw == VEXW1 ? 1 : 0;
  else
    w = (flag_code == CODE_64BIT ? i.rex & REX_W : evexwig == evexw1) ? 1 : 0;

  /* The third byte of the EVEX prefix.  */
  i.vex.bytes[2] = ((w << 7)
		    | (register_specifier << 3)
		    | 4 /* Encode the U bit.  */
		    | i.tm.opcode_modifier.opcodeprefix);

  /* The fourth byte of the EVEX prefix.  */
  /* The zeroing-masking bit.  */
  if (i.mask.reg && i.mask.zeroing)
    i.vex.bytes[3] |= 0x80;

  /* Don't always set the broadcast bit if there is no RC.  */
  if (i.rounding.type == rc_none)
    {
      /* Encode the vector length.  */
      unsigned int vec_length;

      if (!i.tm.opcode_modifier.evex
	  || i.tm.opcode_modifier.evex == EVEXDYN)
	{
	  unsigned int op;

	  /* Determine vector length from the last multi-length vector
	     operand.  */
	  for (op = i.operands; op--;)
	    if (i.tm.operand_types[op].bitfield.xmmword
		+ i.tm.operand_types[op].bitfield.ymmword
		+ i.tm.operand_types[op].bitfield.zmmword > 1)
	      {
		if (i.types[op].bitfield.zmmword)
		  {
		    i.tm.opcode_modifier.evex = EVEX512;
		    break;
		  }
		else if (i.types[op].bitfield.ymmword)
		  {
		    i.tm.opcode_modifier.evex = EVEX256;
		    break;
		  }
		else if (i.types[op].bitfield.xmmword)
		  {
		    i.tm.opcode_modifier.evex = EVEX128;
		    break;
		  }
		else if ((i.broadcast.type || i.broadcast.bytes)
			 && op == i.broadcast.operand)
		  {
		    switch (get_broadcast_bytes (&i.tm, true))
		      {
			case 64:
			  i.tm.opcode_modifier.evex = EVEX512;
			  break;
			case 32:
			  i.tm.opcode_modifier.evex = EVEX256;
			  break;
			case 16:
			  i.tm.opcode_modifier.evex = EVEX128;
			  break;
			default:
			  abort ();
		      }
		    break;
		  }
	      }

	  if (op >= MAX_OPERANDS)
	    abort ();
	}

      switch (i.tm.opcode_modifier.evex)
	{
	case EVEXLIG: /* LL' is ignored */
	  vec_length = evexlig << 5;
	  break;
	case EVEX128:
	  vec_length = 0 << 5;
	  break;
	case EVEX256:
	  vec_length = 1 << 5;
	  break;
	case EVEX512:
	  vec_length = 2 << 5;
	  break;
	case EVEX_L3:
	  if (dot_insn ())
	    {
	      vec_length = 3 << 5;
	      break;
	    }
	  /* Fall through.  */
	default:
	  abort ();
	  break;
	}
      i.vex.bytes[3] |= vec_length;
      /* Encode the broadcast bit.  */
      if (i.broadcast.type || i.broadcast.bytes)
	i.vex.bytes[3] |= 0x10;
    }
  else if (i.rounding.type != saeonly)
    i.vex.bytes[3] |= 0x10 | (i.rounding.type << 5);
  else
    i.vex.bytes[3] |= 0x10 | (evexrcig << 5);

  if (i.mask.reg)
    i.vex.bytes[3] |= i.mask.reg->reg_num;
}

static void
process_immext (void)
{
  expressionS *exp;

  /* These AMD 3DNow! and SSE2 instructions have an opcode suffix
     which is coded in the same place as an 8-bit immediate field
     would be.  Here we fake an 8-bit immediate operand from the
     opcode suffix stored in tm.extension_opcode.

     AVX instructions also use this encoding, for some of
     3 argument instructions.  */

  gas_assert (i.imm_operands <= 1
	      && (i.operands <= 2
		  || (is_any_vex_encoding (&i.tm)
		      && i.operands <= 4)));

  exp = &im_expressions[i.imm_operands++];
  i.op[i.operands].imms = exp;
  i.types[i.operands].bitfield.imm8 = 1;
  i.operands++;
  exp->X_op = O_constant;
  exp->X_add_number = i.tm.extension_opcode;
  i.tm.extension_opcode = None;
}


static int
check_hle (void)
{
  switch (i.tm.opcode_modifier.prefixok)
    {
    default:
      abort ();
    case PrefixLock:
    case PrefixNone:
    case PrefixNoTrack:
    case PrefixRep:
      as_bad (_("invalid instruction `%s' after `%s'"),
	      insn_name (&i.tm), i.hle_prefix);
      return 0;
    case PrefixHLELock:
      if (i.prefix[LOCK_PREFIX])
	return 1;
      as_bad (_("missing `lock' with `%s'"), i.hle_prefix);
      return 0;
    case PrefixHLEAny:
      return 1;
    case PrefixHLERelease:
      if (i.prefix[HLE_PREFIX] != XRELEASE_PREFIX_OPCODE)
	{
	  as_bad (_("instruction `%s' after `xacquire' not allowed"),
		  insn_name (&i.tm));
	  return 0;
	}
      if (i.mem_operands == 0 || !(i.flags[i.operands - 1] & Operand_Mem))
	{
	  as_bad (_("memory destination needed for instruction `%s'"
		    " after `xrelease'"), insn_name (&i.tm));
	  return 0;
	}
      return 1;
    }
}

/* Encode aligned vector move as unaligned vector move.  */

static void
encode_with_unaligned_vector_move (void)
{
  switch (i.tm.base_opcode)
    {
    case 0x28:	/* Load instructions.  */
    case 0x29:	/* Store instructions.  */
      /* movaps/movapd/vmovaps/vmovapd.  */
      if (i.tm.opcode_space == SPACE_0F
	  && i.tm.opcode_modifier.opcodeprefix <= PREFIX_0X66)
	i.tm.base_opcode = 0x10 | (i.tm.base_opcode & 1);
      break;
    case 0x6f:	/* Load instructions.  */
    case 0x7f:	/* Store instructions.  */
      /* movdqa/vmovdqa/vmovdqa64/vmovdqa32. */
      if (i.tm.opcode_space == SPACE_0F
	  && i.tm.opcode_modifier.opcodeprefix == PREFIX_0X66)
	i.tm.opcode_modifier.opcodeprefix = PREFIX_0XF3;
      break;
    default:
      break;
    }
}

/* Try the shortest encoding by shortening operand size.  */

static void
optimize_encoding (void)
{
  unsigned int j;

  if (i.tm.mnem_off == MN_lea)
    {
      /* Optimize: -O:
	   lea symbol, %rN    -> mov $symbol, %rN
	   lea (%rM), %rN     -> mov %rM, %rN
	   lea (,%rM,1), %rN  -> mov %rM, %rN

	   and in 32-bit mode for 16-bit addressing

	   lea (%rM), %rN     -> movzx %rM, %rN

	   and in 64-bit mode zap 32-bit addressing in favor of using a
	   32-bit (or less) destination.
       */
      if (flag_code == CODE_64BIT && i.prefix[ADDR_PREFIX])
	{
	  if (!i.op[1].regs->reg_type.bitfield.word)
	    i.tm.opcode_modifier.size = SIZE32;
	  i.prefix[ADDR_PREFIX] = 0;
	}

      if (!i.index_reg && !i.base_reg)
	{
	  /* Handle:
	       lea symbol, %rN    -> mov $symbol, %rN
	   */
	  if (flag_code == CODE_64BIT)
	    {
	      /* Don't transform a relocation to a 16-bit one.  */
	      if (i.op[0].disps
		  && i.op[0].disps->X_op != O_constant
		  && i.op[1].regs->reg_type.bitfield.word)
		return;

	      if (!i.op[1].regs->reg_type.bitfield.qword
		  || i.tm.opcode_modifier.size == SIZE32)
		{
		  i.tm.base_opcode = 0xb8;
		  i.tm.opcode_modifier.modrm = 0;
		  if (!i.op[1].regs->reg_type.bitfield.word)
		    i.types[0].bitfield.imm32 = 1;
		  else
		    {
		      i.tm.opcode_modifier.size = SIZE16;
		      i.types[0].bitfield.imm16 = 1;
		    }
		}
	      else
		{
		  /* Subject to further optimization below.  */
		  i.tm.base_opcode = 0xc7;
		  i.tm.extension_opcode = 0;
		  i.types[0].bitfield.imm32s = 1;
		  i.types[0].bitfield.baseindex = 0;
		}
	    }
	  /* Outside of 64-bit mode address and operand sizes have to match if
	     a relocation is involved, as otherwise we wouldn't (currently) or
	     even couldn't express the relocation correctly.  */
	  else if (i.op[0].disps
		   && i.op[0].disps->X_op != O_constant
		   && ((!i.prefix[ADDR_PREFIX])
		       != (flag_code == CODE_32BIT
			   ? i.op[1].regs->reg_type.bitfield.dword
			   : i.op[1].regs->reg_type.bitfield.word)))
	    return;
	  /* In 16-bit mode converting LEA with 16-bit addressing and a 32-bit
	     destination is going to grow encoding size.  */
	  else if (flag_code == CODE_16BIT
		   && (optimize <= 1 || optimize_for_space)
		   && !i.prefix[ADDR_PREFIX]
		   && i.op[1].regs->reg_type.bitfield.dword)
	    return;
	  else
	    {
	      i.tm.base_opcode = 0xb8;
	      i.tm.opcode_modifier.modrm = 0;
	      if (i.op[1].regs->reg_type.bitfield.dword)
		i.types[0].bitfield.imm32 = 1;
	      else
		i.types[0].bitfield.imm16 = 1;

	      if (i.op[0].disps
		  && i.op[0].disps->X_op == O_constant
		  && i.op[1].regs->reg_type.bitfield.dword
		  /* NB: Add () to !i.prefix[ADDR_PREFIX] to silence
		     GCC 5. */
		  && (!i.prefix[ADDR_PREFIX]) != (flag_code == CODE_32BIT))
		i.op[0].disps->X_add_number &= 0xffff;
	    }

	  i.tm.operand_types[0] = i.types[0];
	  i.imm_operands = 1;
	  if (!i.op[0].imms)
	    {
	      i.op[0].imms = &im_expressions[0];
	      i.op[0].imms->X_op = O_absent;
	    }
	}
      else if (i.op[0].disps
		  && (i.op[0].disps->X_op != O_constant
		      || i.op[0].disps->X_add_number))
	return;
      else
	{
	  /* Handle:
	       lea (%rM), %rN     -> mov %rM, %rN
	       lea (,%rM,1), %rN  -> mov %rM, %rN
	       lea (%rM), %rN     -> movzx %rM, %rN
	   */
	  const reg_entry *addr_reg;

	  if (!i.index_reg && i.base_reg->reg_num != RegIP)
	    addr_reg = i.base_reg;
	  else if (!i.base_reg
		   && i.index_reg->reg_num != RegIZ
		   && !i.log2_scale_factor)
	    addr_reg = i.index_reg;
	  else
	    return;

	  if (addr_reg->reg_type.bitfield.word
	      && i.op[1].regs->reg_type.bitfield.dword)
	    {
	      if (flag_code != CODE_32BIT)
		return;
	      i.tm.opcode_space = SPACE_0F;
	      i.tm.base_opcode = 0xb7;
	    }
	  else
	    i.tm.base_opcode = 0x8b;

	  if (addr_reg->reg_type.bitfield.dword
	      && i.op[1].regs->reg_type.bitfield.qword)
	    i.tm.opcode_modifier.size = SIZE32;

	  i.op[0].regs = addr_reg;
	  i.reg_operands = 2;
	}

      i.mem_operands = 0;
      i.disp_operands = 0;
      i.prefix[ADDR_PREFIX] = 0;
      i.prefix[SEG_PREFIX] = 0;
      i.seg[0] = NULL;
    }

  if (optimize_for_space
      && i.tm.mnem_off == MN_test
      && i.reg_operands == 1
      && i.imm_operands == 1
      && !i.types[1].bitfield.byte
      && i.op[0].imms->X_op == O_constant
      && fits_in_imm7 (i.op[0].imms->X_add_number))
    {
      /* Optimize: -Os:
	   test $imm7, %r64/%r32/%r16  -> test $imm7, %r8
       */
      unsigned int base_regnum = i.op[1].regs->reg_num;
      if (flag_code == CODE_64BIT || base_regnum < 4)
	{
	  i.types[1].bitfield.byte = 1;
	  /* Ignore the suffix.  */
	  i.suffix = 0;
	  /* Convert to byte registers.  */
	  if (i.types[1].bitfield.word)
	    j = 16;
	  else if (i.types[1].bitfield.dword)
	    j = 32;
	  else
	    j = 48;
	  if (!(i.op[1].regs->reg_flags & RegRex) && base_regnum < 4)
	    j += 8;
	  i.op[1].regs -= j;
	}
    }
  else if (flag_code == CODE_64BIT
	   && i.tm.opcode_space == SPACE_BASE
	   && ((i.types[1].bitfield.qword
		&& i.reg_operands == 1
		&& i.imm_operands == 1
		&& i.op[0].imms->X_op == O_constant
		&& ((i.tm.base_opcode == 0xb8
		     && i.tm.extension_opcode == None
		     && fits_in_unsigned_long (i.op[0].imms->X_add_number))
		    || (fits_in_imm31 (i.op[0].imms->X_add_number)
			&& (i.tm.base_opcode == 0x24
			    || (i.tm.base_opcode == 0x80
				&& i.tm.extension_opcode == 0x4)
			    || i.tm.mnem_off == MN_test
			    || ((i.tm.base_opcode | 1) == 0xc7
				&& i.tm.extension_opcode == 0x0)))
		    || (fits_in_imm7 (i.op[0].imms->X_add_number)
			&& i.tm.base_opcode == 0x83
			&& i.tm.extension_opcode == 0x4)))
	       || (i.types[0].bitfield.qword
		   && ((i.reg_operands == 2
			&& i.op[0].regs == i.op[1].regs
			&& (i.tm.mnem_off == MN_xor
			    || i.tm.mnem_off == MN_sub))
		       || i.tm.mnem_off == MN_clr))))
    {
      /* Optimize: -O:
	   andq $imm31, %r64   -> andl $imm31, %r32
	   andq $imm7, %r64    -> andl $imm7, %r32
	   testq $imm31, %r64  -> testl $imm31, %r32
	   xorq %r64, %r64     -> xorl %r32, %r32
	   subq %r64, %r64     -> subl %r32, %r32
	   movq $imm31, %r64   -> movl $imm31, %r32
	   movq $imm32, %r64   -> movl $imm32, %r32
        */
      i.tm.opcode_modifier.size = SIZE32;
      if (i.imm_operands)
	{
	  i.types[0].bitfield.imm32 = 1;
	  i.types[0].bitfield.imm32s = 0;
	  i.types[0].bitfield.imm64 = 0;
	}
      else
	{
	  i.types[0].bitfield.dword = 1;
	  i.types[0].bitfield.qword = 0;
	}
      i.types[1].bitfield.dword = 1;
      i.types[1].bitfield.qword = 0;
      if (i.tm.mnem_off == MN_mov || i.tm.mnem_off == MN_lea)
	{
	  /* Handle
	       movq $imm31, %r64   -> movl $imm31, %r32
	       movq $imm32, %r64   -> movl $imm32, %r32
	   */
	  i.tm.operand_types[0].bitfield.imm32 = 1;
	  i.tm.operand_types[0].bitfield.imm32s = 0;
	  i.tm.operand_types[0].bitfield.imm64 = 0;
	  if ((i.tm.base_opcode | 1) == 0xc7)
	    {
	      /* Handle
		   movq $imm31, %r64   -> movl $imm31, %r32
	       */
	      i.tm.base_opcode = 0xb8;
	      i.tm.extension_opcode = None;
	      i.tm.opcode_modifier.w = 0;
	      i.tm.opcode_modifier.modrm = 0;
	    }
	}
    }
  else if (optimize > 1
	   && !optimize_for_space
	   && i.reg_operands == 2
	   && i.op[0].regs == i.op[1].regs
	   && (i.tm.mnem_off == MN_and || i.tm.mnem_off == MN_or)
	   && (flag_code != CODE_64BIT || !i.types[0].bitfield.dword))
    {
      /* Optimize: -O2:
	   andb %rN, %rN  -> testb %rN, %rN
	   andw %rN, %rN  -> testw %rN, %rN
	   andq %rN, %rN  -> testq %rN, %rN
	   orb %rN, %rN   -> testb %rN, %rN
	   orw %rN, %rN   -> testw %rN, %rN
	   orq %rN, %rN   -> testq %rN, %rN

	   and outside of 64-bit mode

	   andl %rN, %rN  -> testl %rN, %rN
	   orl %rN, %rN   -> testl %rN, %rN
       */
      i.tm.base_opcode = 0x84 | (i.tm.base_opcode & 1);
    }
  else if (i.tm.base_opcode == 0xba
	   && i.tm.opcode_space == SPACE_0F
	   && i.reg_operands == 1
	   && i.op[0].imms->X_op == O_constant
	   && i.op[0].imms->X_add_number >= 0)
    {
      /* Optimize: -O:
	   btw $n, %rN -> btl $n, %rN (outside of 16-bit mode, n < 16)
	   btq $n, %rN -> btl $n, %rN (in 64-bit mode, n < 32, N < 8)
	   btl $n, %rN -> btw $n, %rN (in 16-bit mode, n < 16)

	   With <BT> one of bts, btr, and bts also:
	   <BT>w $n, %rN -> btl $n, %rN (in 32-bit mode, n < 16)
	   <BT>l $n, %rN -> btw $n, %rN (in 16-bit mode, n < 16)
       */
      switch (flag_code)
	{
	case CODE_64BIT:
	  if (i.tm.extension_opcode != 4)
	    break;
	  if (i.types[1].bitfield.qword
	      && i.op[0].imms->X_add_number < 32
	      && !(i.op[1].regs->reg_flags & RegRex))
	    i.tm.opcode_modifier.size = SIZE32;
	  /* Fall through.  */
	case CODE_32BIT:
	  if (i.types[1].bitfield.word
	      && i.op[0].imms->X_add_number < 16)
	    i.tm.opcode_modifier.size = SIZE32;
	  break;
	case CODE_16BIT:
	  if (i.op[0].imms->X_add_number < 16)
	    i.tm.opcode_modifier.size = SIZE16;
	  break;
	}
    }
  else if (i.reg_operands == 3
	   && i.op[0].regs == i.op[1].regs
	   && !i.types[2].bitfield.xmmword
	   && (i.tm.opcode_modifier.vex
	       || ((!i.mask.reg || i.mask.zeroing)
		   && is_evex_encoding (&i.tm)
		   && (i.vec_encoding != vex_encoding_evex
		       || cpu_arch_isa_flags.bitfield.cpuavx512vl
		       || i.tm.cpu_flags.bitfield.cpuavx512vl
		       || (i.tm.operand_types[2].bitfield.zmmword
			   && i.types[2].bitfield.ymmword))))
	   && i.tm.opcode_space == SPACE_0F
	   && ((i.tm.base_opcode | 2) == 0x57
	       || i.tm.base_opcode == 0xdf
	       || i.tm.base_opcode == 0xef
	       || (i.tm.base_opcode | 3) == 0xfb
	       || i.tm.base_opcode == 0x42
	       || i.tm.base_opcode == 0x47))
    {
      /* Optimize: -O1:
	   VOP, one of vandnps, vandnpd, vxorps, vxorpd, vpsubb, vpsubd,
	   vpsubq and vpsubw:
	     EVEX VOP %zmmM, %zmmM, %zmmN
	       -> VEX VOP %xmmM, %xmmM, %xmmN (M and N < 16)
	       -> EVEX VOP %xmmM, %xmmM, %xmmN (M || N >= 16) (-O2)
	     EVEX VOP %ymmM, %ymmM, %ymmN
	       -> VEX VOP %xmmM, %xmmM, %xmmN (M and N < 16)
	       -> EVEX VOP %xmmM, %xmmM, %xmmN (M || N >= 16) (-O2)
	     VEX VOP %ymmM, %ymmM, %ymmN
	       -> VEX VOP %xmmM, %xmmM, %xmmN
	   VOP, one of vpandn and vpxor:
	     VEX VOP %ymmM, %ymmM, %ymmN
	       -> VEX VOP %xmmM, %xmmM, %xmmN
	   VOP, one of vpandnd and vpandnq:
	     EVEX VOP %zmmM, %zmmM, %zmmN
	       -> VEX vpandn %xmmM, %xmmM, %xmmN (M and N < 16)
	       -> EVEX VOP %xmmM, %xmmM, %xmmN (M || N >= 16) (-O2)
	     EVEX VOP %ymmM, %ymmM, %ymmN
	       -> VEX vpandn %xmmM, %xmmM, %xmmN (M and N < 16)
	       -> EVEX VOP %xmmM, %xmmM, %xmmN (M || N >= 16) (-O2)
	   VOP, one of vpxord and vpxorq:
	     EVEX VOP %zmmM, %zmmM, %zmmN
	       -> VEX vpxor %xmmM, %xmmM, %xmmN (M and N < 16)
	       -> EVEX VOP %xmmM, %xmmM, %xmmN (M || N >= 16) (-O2)
	     EVEX VOP %ymmM, %ymmM, %ymmN
	       -> VEX vpxor %xmmM, %xmmM, %xmmN (M and N < 16)
	       -> EVEX VOP %xmmM, %xmmM, %xmmN (M || N >= 16) (-O2)
	   VOP, one of kxord and kxorq:
	     VEX VOP %kM, %kM, %kN
	       -> VEX kxorw %kM, %kM, %kN
	   VOP, one of kandnd and kandnq:
	     VEX VOP %kM, %kM, %kN
	       -> VEX kandnw %kM, %kM, %kN
       */
      if (is_evex_encoding (&i.tm))
	{
	  if (i.vec_encoding != vex_encoding_evex)
	    {
	      i.tm.opcode_modifier.vex = VEX128;
	      i.tm.opcode_modifier.vexw = VEXW0;
	      i.tm.opcode_modifier.evex = 0;
	    }
	  else if (optimize > 1)
	    i.tm.opcode_modifier.evex = EVEX128;
	  else
	    return;
	}
      else if (i.tm.operand_types[0].bitfield.class == RegMask)
	{
	  i.tm.opcode_modifier.opcodeprefix = PREFIX_NONE;
	  i.tm.opcode_modifier.vexw = VEXW0;
	}
      else
	i.tm.opcode_modifier.vex = VEX128;

      if (i.tm.opcode_modifier.vex)
	for (j = 0; j < 3; j++)
	  {
	    i.types[j].bitfield.xmmword = 1;
	    i.types[j].bitfield.ymmword = 0;
	  }
    }
  else if (i.vec_encoding != vex_encoding_evex
	   && !i.types[0].bitfield.zmmword
	   && !i.types[1].bitfield.zmmword
	   && !i.mask.reg
	   && !i.broadcast.type
	   && !i.broadcast.bytes
	   && is_evex_encoding (&i.tm)
	   && ((i.tm.base_opcode & ~Opcode_SIMD_IntD) == 0x6f
	       || (i.tm.base_opcode & ~4) == 0xdb
	       || (i.tm.base_opcode & ~4) == 0xeb)
	   && i.tm.extension_opcode == None)
    {
      /* Optimize: -O1:
	   VOP, one of vmovdqa32, vmovdqa64, vmovdqu8, vmovdqu16,
	   vmovdqu32 and vmovdqu64:
	     EVEX VOP %xmmM, %xmmN
	       -> VEX vmovdqa|vmovdqu %xmmM, %xmmN (M and N < 16)
	     EVEX VOP %ymmM, %ymmN
	       -> VEX vmovdqa|vmovdqu %ymmM, %ymmN (M and N < 16)
	     EVEX VOP %xmmM, mem
	       -> VEX vmovdqa|vmovdqu %xmmM, mem (M < 16)
	     EVEX VOP %ymmM, mem
	       -> VEX vmovdqa|vmovdqu %ymmM, mem (M < 16)
	     EVEX VOP mem, %xmmN
	       -> VEX mvmovdqa|vmovdquem, %xmmN (N < 16)
	     EVEX VOP mem, %ymmN
	       -> VEX vmovdqa|vmovdqu mem, %ymmN (N < 16)
	   VOP, one of vpand, vpandn, vpor, vpxor:
	     EVEX VOP{d,q} %xmmL, %xmmM, %xmmN
	       -> VEX VOP %xmmL, %xmmM, %xmmN (L, M, and N < 16)
	     EVEX VOP{d,q} %ymmL, %ymmM, %ymmN
	       -> VEX VOP %ymmL, %ymmM, %ymmN (L, M, and N < 16)
	     EVEX VOP{d,q} mem, %xmmM, %xmmN
	       -> VEX VOP mem, %xmmM, %xmmN (M and N < 16)
	     EVEX VOP{d,q} mem, %ymmM, %ymmN
	       -> VEX VOP mem, %ymmM, %ymmN (M and N < 16)
       */
      for (j = 0; j < i.operands; j++)
	if (operand_type_check (i.types[j], disp)
	    && i.op[j].disps->X_op == O_constant)
	  {
	    /* Since the VEX prefix has 2 or 3 bytes, the EVEX prefix
	       has 4 bytes, EVEX Disp8 has 1 byte and VEX Disp32 has 4
	       bytes, we choose EVEX Disp8 over VEX Disp32.  */
	    int evex_disp8, vex_disp8;
	    unsigned int memshift = i.memshift;
	    offsetT n = i.op[j].disps->X_add_number;

	    evex_disp8 = fits_in_disp8 (n);
	    i.memshift = 0;
	    vex_disp8 = fits_in_disp8 (n);
	    if (evex_disp8 != vex_disp8)
	      {
		i.memshift = memshift;
		return;
	      }

	    i.types[j].bitfield.disp8 = vex_disp8;
	    break;
	  }
      if ((i.tm.base_opcode & ~Opcode_SIMD_IntD) == 0x6f
	  && i.tm.opcode_modifier.opcodeprefix == PREFIX_0XF2)
	i.tm.opcode_modifier.opcodeprefix = PREFIX_0XF3;
      i.tm.opcode_modifier.vex
	= i.types[0].bitfield.ymmword ? VEX256 : VEX128;
      i.tm.opcode_modifier.vexw = VEXW0;
      /* VPAND, VPOR, and VPXOR are commutative.  */
      if (i.reg_operands == 3 && i.tm.base_opcode != 0xdf)
	i.tm.opcode_modifier.commutative = 1;
      i.tm.opcode_modifier.evex = 0;
      i.tm.opcode_modifier.masking = 0;
      i.tm.opcode_modifier.broadcast = 0;
      i.tm.opcode_modifier.disp8memshift = 0;
      i.memshift = 0;
      if (j < i.operands)
	i.types[j].bitfield.disp8
	  = fits_in_disp8 (i.op[j].disps->X_add_number);
    }
}

/* Return non-zero for load instruction.  */

static int
load_insn_p (void)
{
  unsigned int dest;
  int any_vex_p = is_any_vex_encoding (&i.tm);
  unsigned int base_opcode = i.tm.base_opcode | 1;

  if (!any_vex_p)
    {
      /* Anysize insns: lea, invlpg, clflush, prefetch*, bndmk, bndcl, bndcu,
	 bndcn, bndstx, bndldx, clflushopt, clwb, cldemote.  */
      if (i.tm.opcode_modifier.operandconstraint == ANY_SIZE)
	return 0;

      /* pop.   */
      if (i.tm.mnem_off == MN_pop)
	return 1;
    }

  if (i.tm.opcode_space == SPACE_BASE)
    {
      /* popf, popa.   */
      if (i.tm.base_opcode == 0x9d
	  || i.tm.base_opcode == 0x61)
	return 1;

      /* movs, cmps, lods, scas.  */
      if ((i.tm.base_opcode | 0xb) == 0xaf)
	return 1;

      /* outs, xlatb.  */
      if (base_opcode == 0x6f
	  || i.tm.base_opcode == 0xd7)
	return 1;
      /* NB: For AMD-specific insns with implicit memory operands,
	 they're intentionally not covered.  */
    }

  /* No memory operand.  */
  if (!i.mem_operands)
    return 0;

  if (any_vex_p)
    {
      if (i.tm.mnem_off == MN_vldmxcsr)
	return 1;
    }
  else if (i.tm.opcode_space == SPACE_BASE)
    {
      /* test, not, neg, mul, imul, div, idiv.  */
      if (base_opcode == 0xf7 && i.tm.extension_opcode != 1)
	return 1;

      /* inc, dec.  */
      if (base_opcode == 0xff && i.tm.extension_opcode <= 1)
	return 1;

      /* add, or, adc, sbb, and, sub, xor, cmp.  */
      if (i.tm.base_opcode >= 0x80 && i.tm.base_opcode <= 0x83)
	return 1;

      /* rol, ror, rcl, rcr, shl/sal, shr, sar. */
      if ((base_opcode == 0xc1 || (base_opcode | 2) == 0xd3)
	  && i.tm.extension_opcode != 6)
	return 1;

      /* Check for x87 instructions.  */
      if ((base_opcode | 6) == 0xdf)
	{
	  /* Skip fst, fstp, fstenv, fstcw.  */
	  if (i.tm.base_opcode == 0xd9
	      && (i.tm.extension_opcode == 2
		  || i.tm.extension_opcode == 3
		  || i.tm.extension_opcode == 6
		  || i.tm.extension_opcode == 7))
	    return 0;

	  /* Skip fisttp, fist, fistp, fstp.  */
	  if (i.tm.base_opcode == 0xdb
	      && (i.tm.extension_opcode == 1
		  || i.tm.extension_opcode == 2
		  || i.tm.extension_opcode == 3
		  || i.tm.extension_opcode == 7))
	    return 0;

	  /* Skip fisttp, fst, fstp, fsave, fstsw.  */
	  if (i.tm.base_opcode == 0xdd
	      && (i.tm.extension_opcode == 1
		  || i.tm.extension_opcode == 2
		  || i.tm.extension_opcode == 3
		  || i.tm.extension_opcode == 6
		  || i.tm.extension_opcode == 7))
	    return 0;

	  /* Skip fisttp, fist, fistp, fbstp, fistp.  */
	  if (i.tm.base_opcode == 0xdf
	      && (i.tm.extension_opcode == 1
		  || i.tm.extension_opcode == 2
		  || i.tm.extension_opcode == 3
		  || i.tm.extension_opcode == 6
		  || i.tm.extension_opcode == 7))
	    return 0;

	  return 1;
	}
    }
  else if (i.tm.opcode_space == SPACE_0F)
    {
      /* bt, bts, btr, btc.  */
      if (i.tm.base_opcode == 0xba
	  && (i.tm.extension_opcode | 3) == 7)
	return 1;

      /* cmpxchg8b, cmpxchg16b, xrstors, vmptrld.  */
      if (i.tm.base_opcode == 0xc7
	  && i.tm.opcode_modifier.opcodeprefix == PREFIX_NONE
	  && (i.tm.extension_opcode == 1 || i.tm.extension_opcode == 3
	      || i.tm.extension_opcode == 6))
	return 1;

      /* fxrstor, ldmxcsr, xrstor.  */
      if (i.tm.base_opcode == 0xae
	  && (i.tm.extension_opcode == 1
	      || i.tm.extension_opcode == 2
	      || i.tm.extension_opcode == 5))
	return 1;

      /* lgdt, lidt, lmsw.  */
      if (i.tm.base_opcode == 0x01
	  && (i.tm.extension_opcode == 2
	      || i.tm.extension_opcode == 3
	      || i.tm.extension_opcode == 6))
	return 1;
    }

  dest = i.operands - 1;

  /* Check fake imm8 operand and 3 source operands.  */
  if ((i.tm.opcode_modifier.immext
       || i.reg_operands + i.mem_operands == 4)
      && i.types[dest].bitfield.imm8)
    dest--;

  /* add, or, adc, sbb, and, sub, xor, cmp, test, xchg.  */
  if (i.tm.opcode_space == SPACE_BASE
      && ((base_opcode | 0x38) == 0x39
	  || (base_opcode | 2) == 0x87))
    return 1;

  if (i.tm.mnem_off == MN_xadd)
    return 1;

  /* Check for load instruction.  */
  return (i.types[dest].bitfield.class != ClassNone
	  || i.types[dest].bitfield.instance == Accum);
}

/* Output lfence, 0xfaee8, after instruction.  */

static void
insert_lfence_after (void)
{
  if (lfence_after_load && load_insn_p ())
    {
      /* There are also two REP string instructions that require
	 special treatment. Specifically, the compare string (CMPS)
	 and scan string (SCAS) instructions set EFLAGS in a manner
	 that depends on the data being compared/scanned. When used
	 with a REP prefix, the number of iterations may therefore
	 vary depending on this data. If the data is a program secret
	 chosen by the adversary using an LVI method,
	 then this data-dependent behavior may leak some aspect
	 of the secret.  */
      if (((i.tm.base_opcode | 0x9) == 0xaf)
	  && i.prefix[REP_PREFIX])
	{
	    as_warn (_("`%s` changes flags which would affect control flow behavior"),
		     insn_name (&i.tm));
	}
      char *p = frag_more (3);
      *p++ = 0xf;
      *p++ = 0xae;
      *p = 0xe8;
    }
}

/* Output lfence, 0xfaee8, before instruction.  */

static void
insert_lfence_before (void)
{
  char *p;

  if (i.tm.opcode_space != SPACE_BASE)
    return;

  if (i.tm.base_opcode == 0xff
      && (i.tm.extension_opcode == 2 || i.tm.extension_opcode == 4))
    {
      /* Insert lfence before indirect branch if needed.  */

      if (lfence_before_indirect_branch == lfence_branch_none)
	return;

      if (i.operands != 1)
	abort ();

      if (i.reg_operands == 1)
	{
	  /* Indirect branch via register.  Don't insert lfence with
	     -mlfence-after-load=yes.  */
	  if (lfence_after_load
	      || lfence_before_indirect_branch == lfence_branch_memory)
	    return;
	}
      else if (i.mem_operands == 1
	       && lfence_before_indirect_branch != lfence_branch_register)
	{
	  as_warn (_("indirect `%s` with memory operand should be avoided"),
		   insn_name (&i.tm));
	  return;
	}
      else
	return;

      if (last_insn.kind != last_insn_other
	  && last_insn.seg == now_seg)
	{
	  as_warn_where (last_insn.file, last_insn.line,
			 _("`%s` skips -mlfence-before-indirect-branch on `%s`"),
			 last_insn.name, insn_name (&i.tm));
	  return;
	}

      p = frag_more (3);
      *p++ = 0xf;
      *p++ = 0xae;
      *p = 0xe8;
      return;
    }

  /* Output or/not/shl and lfence before near ret.  */
  if (lfence_before_ret != lfence_before_ret_none
      && (i.tm.base_opcode | 1) == 0xc3)
    {
      if (last_insn.kind != last_insn_other
	  && last_insn.seg == now_seg)
	{
	  as_warn_where (last_insn.file, last_insn.line,
			 _("`%s` skips -mlfence-before-ret on `%s`"),
			 last_insn.name, insn_name (&i.tm));
	  return;
	}

      /* Near ret ingore operand size override under CPU64.  */
      char prefix = flag_code == CODE_64BIT
		    ? 0x48
		    : i.prefix[DATA_PREFIX] ? 0x66 : 0x0;

      if (lfence_before_ret == lfence_before_ret_not)
	{
	  /* not: 0xf71424, may add prefix
	     for operand size override or 64-bit code.  */
	  p = frag_more ((prefix ? 2 : 0) + 6 + 3);
	  if (prefix)
	    *p++ = prefix;
	  *p++ = 0xf7;
	  *p++ = 0x14;
	  *p++ = 0x24;
	  if (prefix)
	    *p++ = prefix;
	  *p++ = 0xf7;
	  *p++ = 0x14;
	  *p++ = 0x24;
	}
      else
	{
	  p = frag_more ((prefix ? 1 : 0) + 4 + 3);
	  if (prefix)
	    *p++ = prefix;
	  if (lfence_before_ret == lfence_before_ret_or)
	    {
	      /* or: 0x830c2400, may add prefix
		 for operand size override or 64-bit code.  */
	      *p++ = 0x83;
	      *p++ = 0x0c;
	    }
	  else
	    {
	      /* shl: 0xc1242400, may add prefix
		 for operand size override or 64-bit code.  */
	      *p++ = 0xc1;
	      *p++ = 0x24;
	    }

	  *p++ = 0x24;
	  *p++ = 0x0;
	}

      *p++ = 0xf;
      *p++ = 0xae;
      *p = 0xe8;
    }
}

/* Shared helper for md_assemble() and s_insn().  */
static void init_globals (void)
{
  unsigned int j;

  memset (&i, '\0', sizeof (i));
  i.rounding.type = rc_none;
  for (j = 0; j < MAX_OPERANDS; j++)
    i.reloc[j] = NO_RELOC;
  memset (disp_expressions, '\0', sizeof (disp_expressions));
  memset (im_expressions, '\0', sizeof (im_expressions));
  save_stack_p = save_stack;
}

/* Helper for md_assemble() to decide whether to prepare for a possible 2nd
   parsing pass. Instead of introducing a rarely use new insn attribute this
   utilizes a common pattern between affected templates. It is deemed
   acceptable that this will lead to unnecessary pass 2 preparations in a
   limited set of cases.  */
static INLINE bool may_need_pass2 (const insn_template *t)
{
  return t->opcode_modifier.sse2avx
	 /* Note that all SSE2AVX templates have at least one operand.  */
	 ? t->operand_types[t->operands - 1].bitfield.class == RegSIMD
	 : (t->opcode_space == SPACE_0F
	    && (t->base_opcode | 1) == 0xbf)
	   || (t->opcode_space == SPACE_BASE
	       && t->base_opcode == 0x63);
}

/* This is the guts of the machine-dependent assembler.  LINE points to a
   machine dependent instruction.  This function is supposed to emit
   the frags/bytes it assembles to.  */

void
md_assemble (char *line)
{
  unsigned int j;
  char mnemonic[MAX_MNEM_SIZE], mnem_suffix = 0, *copy = NULL;
  const char *end, *pass1_mnem = NULL;
  enum i386_error pass1_err = 0;
  const insn_template *t;

  /* Initialize globals.  */
  current_templates = NULL;
 retry:
  init_globals ();

  /* First parse an instruction mnemonic & call i386_operand for the operands.
     We assume that the scrubber has arranged it so that line[0] is the valid
     start of a (possibly prefixed) mnemonic.  */

  end = parse_insn (line, mnemonic, false);
  if (end == NULL)
    {
      if (pass1_mnem != NULL)
	goto match_error;
      if (i.error != no_error)
	{
	  gas_assert (current_templates != NULL);
	  if (may_need_pass2 (current_templates->start) && !i.suffix)
	    goto no_match;
	  /* No point in trying a 2nd pass - it'll only find the same suffix
	     again.  */
	  mnem_suffix = i.suffix;
	  goto match_error;
	}
      return;
    }
  t = current_templates->start;
  if (may_need_pass2 (t))
    {
      /* Make a copy of the full line in case we need to retry.  */
      copy = xstrdup (line);
    }
  line += end - line;
  mnem_suffix = i.suffix;

  line = parse_operands (line, mnemonic);
  this_operand = -1;
  if (line == NULL)
    {
      free (copy);
      return;
    }

  /* Now we've parsed the mnemonic into a set of templates, and have the
     operands at hand.  */

  /* All Intel opcodes have reversed operands except for "bound", "enter",
     "invlpg*", "monitor*", "mwait*", "tpause", "umwait", "pvalidate",
     "rmpadjust", "rmpupdate", and "rmpquery".  We also don't reverse
     intersegment "jmp" and "call" instructions with 2 immediate operands so
     that the immediate segment precedes the offset consistently in Intel and
     AT&T modes.  */
  if (intel_syntax
      && i.operands > 1
      && (t->mnem_off != MN_bound)
      && !startswith (mnemonic, "invlpg")
      && !startswith (mnemonic, "monitor")
      && !startswith (mnemonic, "mwait")
      && (t->mnem_off != MN_pvalidate)
      && !startswith (mnemonic, "rmp")
      && (t->mnem_off != MN_tpause)
      && (t->mnem_off != MN_umwait)
      && !(i.operands == 2
	   && operand_type_check (i.types[0], imm)
	   && operand_type_check (i.types[1], imm)))
    swap_operands ();

  /* The order of the immediates should be reversed
     for 2 immediates extrq and insertq instructions */
  if (i.imm_operands == 2
      && (t->mnem_off == MN_extrq || t->mnem_off == MN_insertq))
      swap_2_operands (0, 1);

  if (i.imm_operands)
    optimize_imm ();

  if (i.disp_operands && !optimize_disp (t))
    return;

  /* Next, we find a template that matches the given insn,
     making sure the overlap of the given operands types is consistent
     with the template operand types.  */

  if (!(t = match_template (mnem_suffix)))
    {
      const char *err_msg;

      if (copy && !mnem_suffix)
	{
	  line = copy;
	  copy = NULL;
  no_match:
	  pass1_err = i.error;
	  pass1_mnem = insn_name (current_templates->start);
	  goto retry;
	}

      /* If a non-/only-64bit template (group) was found in pass 1, and if
	 _some_ template (group) was found in pass 2, squash pass 1's
	 error.  */
      if (pass1_err == unsupported_64bit)
	pass1_mnem = NULL;

  match_error:
      free (copy);

      switch (pass1_mnem ? pass1_err : i.error)
	{
	default:
	  abort ();
	case operand_size_mismatch:
	  err_msg = _("operand size mismatch");
	  break;
	case operand_type_mismatch:
	  err_msg = _("operand type mismatch");
	  break;
	case register_type_mismatch:
	  err_msg = _("register type mismatch");
	  break;
	case number_of_operands_mismatch:
	  err_msg = _("number of operands mismatch");
	  break;
	case invalid_instruction_suffix:
	  err_msg = _("invalid instruction suffix");
	  break;
	case bad_imm4:
	  err_msg = _("constant doesn't fit in 4 bits");
	  break;
	case unsupported_with_intel_mnemonic:
	  err_msg = _("unsupported with Intel mnemonic");
	  break;
	case unsupported_syntax:
	  err_msg = _("unsupported syntax");
	  break;
	case unsupported:
	  as_bad (_("unsupported instruction `%s'"),
		  pass1_mnem ? pass1_mnem : insn_name (current_templates->start));
	  return;
	case unsupported_on_arch:
	  as_bad (_("`%s' is not supported on `%s%s'"),
		  pass1_mnem ? pass1_mnem : insn_name (current_templates->start),
		  cpu_arch_name ? cpu_arch_name : default_arch,
		  cpu_sub_arch_name ? cpu_sub_arch_name : "");
	  return;
	case unsupported_64bit:
	  if (ISLOWER (mnem_suffix))
	    {
	      if (flag_code == CODE_64BIT)
		as_bad (_("`%s%c' is not supported in 64-bit mode"),
			pass1_mnem ? pass1_mnem : insn_name (current_templates->start),
			mnem_suffix);
	      else
		as_bad (_("`%s%c' is only supported in 64-bit mode"),
			pass1_mnem ? pass1_mnem : insn_name (current_templates->start),
			mnem_suffix);
	    }
	  else
	    {
	      if (flag_code == CODE_64BIT)
		as_bad (_("`%s' is not supported in 64-bit mode"),
			pass1_mnem ? pass1_mnem : insn_name (current_templates->start));
	      else
		as_bad (_("`%s' is only supported in 64-bit mode"),
			pass1_mnem ? pass1_mnem : insn_name (current_templates->start));
	    }
	  return;
	case invalid_sib_address:
	  err_msg = _("invalid SIB address");
	  break;
	case invalid_vsib_address:
	  err_msg = _("invalid VSIB address");
	  break;
	case invalid_vector_register_set:
	  err_msg = _("mask, index, and destination registers must be distinct");
	  break;
	case invalid_tmm_register_set:
	  err_msg = _("all tmm registers must be distinct");
	  break;
	case invalid_dest_and_src_register_set:
	  err_msg = _("destination and source registers must be distinct");
	  break;
	case unsupported_vector_index_register:
	  err_msg = _("unsupported vector index register");
	  break;
	case unsupported_broadcast:
	  err_msg = _("unsupported broadcast");
	  break;
	case broadcast_needed:
	  err_msg = _("broadcast is needed for operand of such type");
	  break;
	case unsupported_masking:
	  err_msg = _("unsupported masking");
	  break;
	case mask_not_on_destination:
	  err_msg = _("mask not on destination operand");
	  break;
	case no_default_mask:
	  err_msg = _("default mask isn't allowed");
	  break;
	case unsupported_rc_sae:
	  err_msg = _("unsupported static rounding/sae");
	  break;
	case invalid_register_operand:
	  err_msg = _("invalid register operand");
	  break;
	}
      as_bad (_("%s for `%s'"), err_msg,
	      pass1_mnem ? pass1_mnem : insn_name (current_templates->start));
      return;
    }

  free (copy);

  if (sse_check != check_none
      /* The opcode space check isn't strictly needed; it's there only to
	 bypass the logic below when easily possible.  */
      && t->opcode_space >= SPACE_0F
      && t->opcode_space <= SPACE_0F3A
      && !i.tm.cpu_flags.bitfield.cpusse4a
      && !is_any_vex_encoding (t))
    {
      bool simd = false;

      for (j = 0; j < t->operands; ++j)
	{
	  if (t->operand_types[j].bitfield.class == RegMMX)
	    break;
	  if (t->operand_types[j].bitfield.class == RegSIMD)
	    simd = true;
	}

      if (j >= t->operands && simd)
	(sse_check == check_warning
	 ? as_warn
	 : as_bad) (_("SSE instruction `%s' is used"), insn_name (&i.tm));
    }

  if (i.tm.opcode_modifier.fwait)
    if (!add_prefix (FWAIT_OPCODE))
      return;

  /* Check if REP prefix is OK.  */
  if (i.rep_prefix && i.tm.opcode_modifier.prefixok != PrefixRep)
    {
      as_bad (_("invalid instruction `%s' after `%s'"),
		insn_name (&i.tm), i.rep_prefix);
      return;
    }

  /* Check for lock without a lockable instruction.  Destination operand
     must be memory unless it is xchg (0x86).  */
  if (i.prefix[LOCK_PREFIX])
    {
      if (i.tm.opcode_modifier.prefixok < PrefixLock
	  || i.mem_operands == 0
	  || (i.tm.base_opcode != 0x86
	      && !(i.flags[i.operands - 1] & Operand_Mem)))
	{
	  as_bad (_("expecting lockable instruction after `lock'"));
	  return;
	}

      /* Zap the redundant prefix from XCHG when optimizing.  */
      if (i.tm.base_opcode == 0x86 && optimize && !i.no_optimize)
	i.prefix[LOCK_PREFIX] = 0;
    }

  if (is_any_vex_encoding (&i.tm)
      || i.tm.operand_types[i.imm_operands].bitfield.class >= RegMMX
      || i.tm.operand_types[i.imm_operands + 1].bitfield.class >= RegMMX)
    {
      /* Check for data size prefix on VEX/XOP/EVEX encoded and SIMD insns.  */
      if (i.prefix[DATA_PREFIX])
	{
	  as_bad (_("data size prefix invalid with `%s'"), insn_name (&i.tm));
	  return;
	}

      /* Don't allow e.g. KMOV in TLS code sequences.  */
      for (j = i.imm_operands; j < i.operands; ++j)
	switch (i.reloc[j])
	  {
	  case BFD_RELOC_386_TLS_GOTIE:
	  case BFD_RELOC_386_TLS_LE_32:
	  case BFD_RELOC_X86_64_GOTTPOFF:
	  case BFD_RELOC_X86_64_TLSLD:
	    as_bad (_("TLS relocation cannot be used with `%s'"), insn_name (&i.tm));
	    return;
	  default:
	    break;
	  }
    }

  /* Check if HLE prefix is OK.  */
  if (i.hle_prefix && !check_hle ())
    return;

  /* Check BND prefix.  */
  if (i.bnd_prefix && !i.tm.opcode_modifier.bndprefixok)
    as_bad (_("expecting valid branch instruction after `bnd'"));

  /* Check NOTRACK prefix.  */
  if (i.notrack_prefix && i.tm.opcode_modifier.prefixok != PrefixNoTrack)
    as_bad (_("expecting indirect branch instruction after `notrack'"));

  if (i.tm.cpu_flags.bitfield.cpumpx)
    {
      if (flag_code == CODE_64BIT && i.prefix[ADDR_PREFIX])
	as_bad (_("32-bit address isn't allowed in 64-bit MPX instructions."));
      else if (flag_code != CODE_16BIT
	       ? i.prefix[ADDR_PREFIX]
	       : i.mem_operands && !i.prefix[ADDR_PREFIX])
	as_bad (_("16-bit address isn't allowed in MPX instructions"));
    }

  /* Insert BND prefix.  */
  if (add_bnd_prefix && i.tm.opcode_modifier.bndprefixok)
    {
      if (!i.prefix[BND_PREFIX])
	add_prefix (BND_PREFIX_OPCODE);
      else if (i.prefix[BND_PREFIX] != BND_PREFIX_OPCODE)
	{
	  as_warn (_("replacing `rep'/`repe' prefix by `bnd'"));
	  i.prefix[BND_PREFIX] = BND_PREFIX_OPCODE;
	}
    }

  /* Check string instruction segment overrides.  */
  if (i.tm.opcode_modifier.isstring >= IS_STRING_ES_OP0)
    {
      gas_assert (i.mem_operands);
      if (!check_string ())
	return;
      i.disp_operands = 0;
    }

  /* The memory operand of (%dx) should be only used with input/output
     instructions (base opcodes: 0x6c, 0x6e, 0xec, 0xee).  */
  if (i.input_output_operand
      && ((i.tm.base_opcode | 0x82) != 0xee
	  || i.tm.opcode_space != SPACE_BASE))
    {
      as_bad (_("input/output port address isn't allowed with `%s'"),
	      insn_name (&i.tm));
      return;
    }

  if (optimize && !i.no_optimize && i.tm.opcode_modifier.optimize)
    optimize_encoding ();

  if (use_unaligned_vector_move)
    encode_with_unaligned_vector_move ();

  if (!process_suffix ())
    return;

  /* Check if IP-relative addressing requirements can be satisfied.  */
  if (i.tm.cpu_flags.bitfield.cpuprefetchi
      && !(i.base_reg && i.base_reg->reg_num == RegIP))
    as_warn (_("'%s' only supports RIP-relative address"), insn_name (&i.tm));

  /* Update operand types and check extended states.  */
  for (j = 0; j < i.operands; j++)
    {
      i.types[j] = operand_type_and (i.types[j], i.tm.operand_types[j]);
      switch (i.tm.operand_types[j].bitfield.class)
	{
	default:
	  break;
	case RegMMX:
	  i.xstate |= xstate_mmx;
	  break;
	case RegMask:
	  i.xstate |= xstate_mask;
	  break;
	case RegSIMD:
	  if (i.tm.operand_types[j].bitfield.tmmword)
	    i.xstate |= xstate_tmm;
	  else if (i.tm.operand_types[j].bitfield.zmmword)
	    i.xstate |= xstate_zmm;
	  else if (i.tm.operand_types[j].bitfield.ymmword)
	    i.xstate |= xstate_ymm;
	  else if (i.tm.operand_types[j].bitfield.xmmword)
	    i.xstate |= xstate_xmm;
	  break;
	}
    }

  /* Make still unresolved immediate matches conform to size of immediate
     given in i.suffix.  */
  if (!finalize_imm ())
    return;

  if (i.types[0].bitfield.imm1)
    i.imm_operands = 0;	/* kludge for shift insns.  */

  /* For insns with operands there are more diddles to do to the opcode.  */
  if (i.operands)
    {
      if (!process_operands ())
	return;
    }
  else if (!quiet_warnings && i.tm.opcode_modifier.operandconstraint == UGH)
    {
      /* UnixWare fsub no args is alias for fsubp, fadd -> faddp, etc.  */
      as_warn (_("translating to `%sp'"), insn_name (&i.tm));
    }

  if (is_any_vex_encoding (&i.tm))
    {
      if (!cpu_arch_flags.bitfield.cpui286)
	{
	  as_bad (_("instruction `%s' isn't supported outside of protected mode."),
		  insn_name (&i.tm));
	  return;
	}

      /* Check for explicit REX prefix.  */
      if (i.prefix[REX_PREFIX] || i.rex_encoding)
	{
	  as_bad (_("REX prefix invalid with `%s'"), insn_name (&i.tm));
	  return;
	}

      if (i.tm.opcode_modifier.vex)
	build_vex_prefix (t);
      else
	build_evex_prefix ();

      /* The individual REX.RXBW bits got consumed.  */
      i.rex &= REX_OPCODE;
    }

  /* Handle conversion of 'int $3' --> special int3 insn.  */
  if (i.tm.mnem_off == MN_int
      && i.op[0].imms->X_add_number == 3)
    {
      i.tm.base_opcode = INT3_OPCODE;
      i.imm_operands = 0;
    }

  if ((i.tm.opcode_modifier.jump == JUMP
       || i.tm.opcode_modifier.jump == JUMP_BYTE
       || i.tm.opcode_modifier.jump == JUMP_DWORD)
      && i.op[0].disps->X_op == O_constant)
    {
      /* Convert "jmp constant" (and "call constant") to a jump (call) to
	 the absolute address given by the constant.  Since ix86 jumps and
	 calls are pc relative, we need to generate a reloc.  */
      i.op[0].disps->X_add_symbol = &abs_symbol;
      i.op[0].disps->X_op = O_symbol;
    }

  /* For 8 bit registers we need an empty rex prefix.  Also if the
     instruction already has a prefix, we need to convert old
     registers to new ones.  */

  if ((i.types[0].bitfield.class == Reg && i.types[0].bitfield.byte
       && (i.op[0].regs->reg_flags & RegRex64) != 0)
      || (i.types[1].bitfield.class == Reg && i.types[1].bitfield.byte
	  && (i.op[1].regs->reg_flags & RegRex64) != 0)
      || (((i.types[0].bitfield.class == Reg && i.types[0].bitfield.byte)
	   || (i.types[1].bitfield.class == Reg && i.types[1].bitfield.byte))
	  && i.rex != 0))
    {
      int x;

      i.rex |= REX_OPCODE;
      for (x = 0; x < 2; x++)
	{
	  /* Look for 8 bit operand that uses old registers.  */
	  if (i.types[x].bitfield.class == Reg && i.types[x].bitfield.byte
	      && (i.op[x].regs->reg_flags & RegRex64) == 0)
	    {
	      gas_assert (!(i.op[x].regs->reg_flags & RegRex));
	      /* In case it is "hi" register, give up.  */
	      if (i.op[x].regs->reg_num > 3)
		as_bad (_("can't encode register '%s%s' in an "
			  "instruction requiring REX prefix."),
			register_prefix, i.op[x].regs->reg_name);

	      /* Otherwise it is equivalent to the extended register.
		 Since the encoding doesn't change this is merely
		 cosmetic cleanup for debug output.  */

	      i.op[x].regs = i.op[x].regs + 8;
	    }
	}
    }

  if (i.rex == 0 && i.rex_encoding)
    {
      /* Check if we can add a REX_OPCODE byte.  Look for 8 bit operand
	 that uses legacy register.  If it is "hi" register, don't add
	 the REX_OPCODE byte.  */
      int x;
      for (x = 0; x < 2; x++)
	if (i.types[x].bitfield.class == Reg
	    && i.types[x].bitfield.byte
	    && (i.op[x].regs->reg_flags & RegRex64) == 0
	    && i.op[x].regs->reg_num > 3)
	  {
	    gas_assert (!(i.op[x].regs->reg_flags & RegRex));
	    i.rex_encoding = false;
	    break;
	  }

      if (i.rex_encoding)
	i.rex = REX_OPCODE;
    }

  if (i.rex != 0)
    add_prefix (REX_OPCODE | i.rex);

  insert_lfence_before ();

  /* We are ready to output the insn.  */
  output_insn ();

  insert_lfence_after ();

  last_insn.seg = now_seg;

  if (i.tm.opcode_modifier.isprefix)
    {
      last_insn.kind = last_insn_prefix;
      last_insn.name = insn_name (&i.tm);
      last_insn.file = as_where (&last_insn.line);
    }
  else
    last_insn.kind = last_insn_other;
}

/* The Q suffix is generally valid only in 64-bit mode, with very few
   exceptions: fild, fistp, fisttp, and cmpxchg8b.  Note that for fild
   and fisttp only one of their two templates is matched below: That's
   sufficient since other relevant attributes are the same between both
   respective templates.  */
static INLINE bool q_suffix_allowed(const insn_template *t)
{
  return flag_code == CODE_64BIT
	 || (t->opcode_space == SPACE_BASE
	     && t->base_opcode == 0xdf
	     && (t->extension_opcode & 1)) /* fild / fistp / fisttp */
	 || t->mnem_off == MN_cmpxchg8b;
}

static const char *
parse_insn (const char *line, char *mnemonic, bool prefix_only)
{
  const char *l = line, *token_start = l;
  char *mnem_p;
  bool pass1 = !current_templates;
  int supported;
  const insn_template *t;
  char *dot_p = NULL;

  while (1)
    {
      mnem_p = mnemonic;
      /* Pseudo-prefixes start with an opening figure brace.  */
      if ((*mnem_p = *l) == '{')
	{
	  ++mnem_p;
	  ++l;
	}
      while ((*mnem_p = mnemonic_chars[(unsigned char) *l]) != 0)
	{
	  if (*mnem_p == '.')
	    dot_p = mnem_p;
	  mnem_p++;
	  if (mnem_p >= mnemonic + MAX_MNEM_SIZE)
	    {
	    too_long:
	      as_bad (_("no such instruction: `%s'"), token_start);
	      return NULL;
	    }
	  l++;
	}
      /* Pseudo-prefixes end with a closing figure brace.  */
      if (*mnemonic == '{' && *l == '}')
	{
	  *mnem_p++ = *l++;
	  if (mnem_p >= mnemonic + MAX_MNEM_SIZE)
	    goto too_long;
	  *mnem_p = '\0';

	  /* Point l at the closing brace if there's no other separator.  */
	  if (*l != END_OF_INSN && !is_space_char (*l)
	      && *l != PREFIX_SEPARATOR)
	    --l;
	}
      else if (!is_space_char (*l)
	       && *l != END_OF_INSN
	       && (intel_syntax
		   || (*l != PREFIX_SEPARATOR && *l != ',')))
	{
	  if (prefix_only)
	    break;
	  as_bad (_("invalid character %s in mnemonic"),
		  output_invalid (*l));
	  return NULL;
	}
      if (token_start == l)
	{
	  if (!intel_syntax && *l == PREFIX_SEPARATOR)
	    as_bad (_("expecting prefix; got nothing"));
	  else
	    as_bad (_("expecting mnemonic; got nothing"));
	  return NULL;
	}

      /* Look up instruction (or prefix) via hash table.  */
      current_templates = (const templates *) str_hash_find (op_hash, mnemonic);

      if (*l != END_OF_INSN
	  && (!is_space_char (*l) || l[1] != END_OF_INSN)
	  && current_templates
	  && current_templates->start->opcode_modifier.isprefix)
	{
	  if (!cpu_flags_check_cpu64 (current_templates->start->cpu_flags))
	    {
	      as_bad ((flag_code != CODE_64BIT
		       ? _("`%s' is only supported in 64-bit mode")
		       : _("`%s' is not supported in 64-bit mode")),
		      insn_name (current_templates->start));
	      return NULL;
	    }
	  /* If we are in 16-bit mode, do not allow addr16 or data16.
	     Similarly, in 32-bit mode, do not allow addr32 or data32.  */
	  if ((current_templates->start->opcode_modifier.size == SIZE16
	       || current_templates->start->opcode_modifier.size == SIZE32)
	      && flag_code != CODE_64BIT
	      && ((current_templates->start->opcode_modifier.size == SIZE32)
		  ^ (flag_code == CODE_16BIT)))
	    {
	      as_bad (_("redundant %s prefix"),
		      insn_name (current_templates->start));
	      return NULL;
	    }

	  if (current_templates->start->base_opcode == PSEUDO_PREFIX)
	    {
	      /* Handle pseudo prefixes.  */
	      switch (current_templates->start->extension_opcode)
		{
		case Prefix_Disp8:
		  /* {disp8} */
		  i.disp_encoding = disp_encoding_8bit;
		  break;
		case Prefix_Disp16:
		  /* {disp16} */
		  i.disp_encoding = disp_encoding_16bit;
		  break;
		case Prefix_Disp32:
		  /* {disp32} */
		  i.disp_encoding = disp_encoding_32bit;
		  break;
		case Prefix_Load:
		  /* {load} */
		  i.dir_encoding = dir_encoding_load;
		  break;
		case Prefix_Store:
		  /* {store} */
		  i.dir_encoding = dir_encoding_store;
		  break;
		case Prefix_VEX:
		  /* {vex} */
		  i.vec_encoding = vex_encoding_vex;
		  break;
		case Prefix_VEX3:
		  /* {vex3} */
		  i.vec_encoding = vex_encoding_vex3;
		  break;
		case Prefix_EVEX:
		  /* {evex} */
		  i.vec_encoding = vex_encoding_evex;
		  break;
		case Prefix_REX:
		  /* {rex} */
		  i.rex_encoding = true;
		  break;
		case Prefix_NoOptimize:
		  /* {nooptimize} */
		  i.no_optimize = true;
		  break;
		default:
		  abort ();
		}
	    }
	  else
	    {
	      /* Add prefix, checking for repeated prefixes.  */
	      switch (add_prefix (current_templates->start->base_opcode))
		{
		case PREFIX_EXIST:
		  return NULL;
		case PREFIX_DS:
		  if (current_templates->start->cpu_flags.bitfield.cpuibt)
		    i.notrack_prefix = insn_name (current_templates->start);
		  break;
		case PREFIX_REP:
		  if (current_templates->start->cpu_flags.bitfield.cpuhle)
		    i.hle_prefix = insn_name (current_templates->start);
		  else if (current_templates->start->cpu_flags.bitfield.cpumpx)
		    i.bnd_prefix = insn_name (current_templates->start);
		  else
		    i.rep_prefix = insn_name (current_templates->start);
		  break;
		default:
		  break;
		}
	    }
	  /* Skip past PREFIX_SEPARATOR and reset token_start.  */
	  token_start = ++l;
	}
      else
	break;
    }

  if (prefix_only)
    return token_start;

  if (!current_templates)
    {
      /* Deprecated functionality (new code should use pseudo-prefixes instead):
	 Check if we should swap operand or force 32bit displacement in
	 encoding.  */
      if (mnem_p - 2 == dot_p && dot_p[1] == 's')
	i.dir_encoding = dir_encoding_swap;
      else if (mnem_p - 3 == dot_p
	       && dot_p[1] == 'd'
	       && dot_p[2] == '8')
	i.disp_encoding = disp_encoding_8bit;
      else if (mnem_p - 4 == dot_p
	       && dot_p[1] == 'd'
	       && dot_p[2] == '3'
	       && dot_p[3] == '2')
	i.disp_encoding = disp_encoding_32bit;
      else
	goto check_suffix;
      mnem_p = dot_p;
      *dot_p = '\0';
      current_templates = (const templates *) str_hash_find (op_hash, mnemonic);
    }

  if (!current_templates || !pass1)
    {
      current_templates = NULL;

    check_suffix:
      if (mnem_p > mnemonic)
	{
	  /* See if we can get a match by trimming off a suffix.  */
	  switch (mnem_p[-1])
	    {
	    case WORD_MNEM_SUFFIX:
	      if (intel_syntax && (intel_float_operand (mnemonic) & 2))
		i.suffix = SHORT_MNEM_SUFFIX;
	      else
		/* Fall through.  */
	      case BYTE_MNEM_SUFFIX:
	      case QWORD_MNEM_SUFFIX:
		i.suffix = mnem_p[-1];
	      mnem_p[-1] = '\0';
	      current_templates
		= (const templates *) str_hash_find (op_hash, mnemonic);
	      break;
	    case SHORT_MNEM_SUFFIX:
	    case LONG_MNEM_SUFFIX:
	      if (!intel_syntax)
		{
		  i.suffix = mnem_p[-1];
		  mnem_p[-1] = '\0';
		  current_templates
		    = (const templates *) str_hash_find (op_hash, mnemonic);
		}
	      break;

	      /* Intel Syntax.  */
	    case 'd':
	      if (intel_syntax)
		{
		  if (intel_float_operand (mnemonic) == 1)
		    i.suffix = SHORT_MNEM_SUFFIX;
		  else
		    i.suffix = LONG_MNEM_SUFFIX;
		  mnem_p[-1] = '\0';
		  current_templates
		    = (const templates *) str_hash_find (op_hash, mnemonic);
		}
	      /* For compatibility reasons accept MOVSD and CMPSD without
	         operands even in AT&T mode.  */
	      else if (*l == END_OF_INSN
		       || (is_space_char (*l) && l[1] == END_OF_INSN))
		{
		  mnem_p[-1] = '\0';
		  current_templates
		    = (const templates *) str_hash_find (op_hash, mnemonic);
		  if (current_templates != NULL
		      /* MOVS or CMPS */
		      && (current_templates->start->base_opcode | 2) == 0xa6
		      && current_templates->start->opcode_space
			 == SPACE_BASE
		      && mnem_p[-2] == 's')
		    {
		      as_warn (_("found `%sd'; assuming `%sl' was meant"),
			       mnemonic, mnemonic);
		      i.suffix = LONG_MNEM_SUFFIX;
		    }
		  else
		    {
		      current_templates = NULL;
		      mnem_p[-1] = 'd';
		    }
		}
	      break;
	    }
	}

      if (!current_templates)
	{
	  if (pass1)
	    as_bad (_("no such instruction: `%s'"), token_start);
	  return NULL;
	}
    }

  if (current_templates->start->opcode_modifier.jump == JUMP
      || current_templates->start->opcode_modifier.jump == JUMP_BYTE)
    {
      /* Check for a branch hint.  We allow ",pt" and ",pn" for
	 predict taken and predict not taken respectively.
	 I'm not sure that branch hints actually do anything on loop
	 and jcxz insns (JumpByte) for current Pentium4 chips.  They
	 may work in the future and it doesn't hurt to accept them
	 now.  */
      if (l[0] == ',' && l[1] == 'p')
	{
	  if (l[2] == 't')
	    {
	      if (!add_prefix (DS_PREFIX_OPCODE))
		return NULL;
	      l += 3;
	    }
	  else if (l[2] == 'n')
	    {
	      if (!add_prefix (CS_PREFIX_OPCODE))
		return NULL;
	      l += 3;
	    }
	}
    }
  /* Any other comma loses.  */
  if (*l == ',')
    {
      as_bad (_("invalid character %s in mnemonic"),
	      output_invalid (*l));
      return NULL;
    }

  /* Check if instruction is supported on specified architecture.  */
  supported = 0;
  for (t = current_templates->start; t < current_templates->end; ++t)
    {
      supported |= cpu_flags_match (t);

      if (i.suffix == QWORD_MNEM_SUFFIX && !q_suffix_allowed (t))
	supported &= ~CPU_FLAGS_64BIT_MATCH;

      if (supported == CPU_FLAGS_PERFECT_MATCH)
	return l;
    }

  if (pass1)
    {
      if (supported & CPU_FLAGS_64BIT_MATCH)
        i.error = unsupported_on_arch;
      else
        i.error = unsupported_64bit;
    }

  return NULL;
}

static char *
parse_operands (char *l, const char *mnemonic)
{
  char *token_start;

  /* 1 if operand is pending after ','.  */
  unsigned int expecting_operand = 0;

  while (*l != END_OF_INSN)
    {
      /* Non-zero if operand parens not balanced.  */
      unsigned int paren_not_balanced = 0;
      /* True if inside double quotes.  */
      bool in_quotes = false;

      /* Skip optional white space before operand.  */
      if (is_space_char (*l))
	++l;
      if (!is_operand_char (*l) && *l != END_OF_INSN && *l != '"')
	{
	  as_bad (_("invalid character %s before operand %d"),
		  output_invalid (*l),
		  i.operands + 1);
	  return NULL;
	}
      token_start = l;	/* After white space.  */
      while (in_quotes || paren_not_balanced || *l != ',')
	{
	  if (*l == END_OF_INSN)
	    {
	      if (in_quotes)
		{
		  as_bad (_("unbalanced double quotes in operand %d."),
			  i.operands + 1);
		  return NULL;
		}
	      if (paren_not_balanced)
		{
		  know (!intel_syntax);
		  as_bad (_("unbalanced parenthesis in operand %d."),
			  i.operands + 1);
		  return NULL;
		}
	      else
		break;	/* we are done */
	    }
	  else if (*l == '\\' && l[1] == '"')
	    ++l;
	  else if (*l == '"')
	    in_quotes = !in_quotes;
	  else if (!in_quotes && !is_operand_char (*l) && !is_space_char (*l))
	    {
	      as_bad (_("invalid character %s in operand %d"),
		      output_invalid (*l),
		      i.operands + 1);
	      return NULL;
	    }
	  if (!intel_syntax && !in_quotes)
	    {
	      if (*l == '(')
		++paren_not_balanced;
	      if (*l == ')')
		--paren_not_balanced;
	    }
	  l++;
	}
      if (l != token_start)
	{			/* Yes, we've read in another operand.  */
	  unsigned int operand_ok;
	  this_operand = i.operands++;
	  if (i.operands > MAX_OPERANDS)
	    {
	      as_bad (_("spurious operands; (%d operands/instruction max)"),
		      MAX_OPERANDS);
	      return NULL;
	    }
	  i.types[this_operand].bitfield.unspecified = 1;
	  /* Now parse operand adding info to 'i' as we go along.  */
	  END_STRING_AND_SAVE (l);

	  if (i.mem_operands > 1)
	    {
	      as_bad (_("too many memory references for `%s'"),
		      mnemonic);
	      return 0;
	    }

	  if (intel_syntax)
	    operand_ok =
	      i386_intel_operand (token_start,
				  intel_float_operand (mnemonic));
	  else
	    operand_ok = i386_att_operand (token_start);

	  RESTORE_END_STRING (l);
	  if (!operand_ok)
	    return NULL;
	}
      else
	{
	  if (expecting_operand)
	    {
	    expecting_operand_after_comma:
	      as_bad (_("expecting operand after ','; got nothing"));
	      return NULL;
	    }
	  if (*l == ',')
	    {
	      as_bad (_("expecting operand before ','; got nothing"));
	      return NULL;
	    }
	}

      /* Now *l must be either ',' or END_OF_INSN.  */
      if (*l == ',')
	{
	  if (*++l == END_OF_INSN)
	    {
	      /* Just skip it, if it's \n complain.  */
	      goto expecting_operand_after_comma;
	    }
	  expecting_operand = 1;
	}
    }
  return l;
}

static void
swap_2_operands (unsigned int xchg1, unsigned int xchg2)
{
  union i386_op temp_op;
  i386_operand_type temp_type;
  unsigned int temp_flags;
  enum bfd_reloc_code_real temp_reloc;

  temp_type = i.types[xchg2];
  i.types[xchg2] = i.types[xchg1];
  i.types[xchg1] = temp_type;

  temp_flags = i.flags[xchg2];
  i.flags[xchg2] = i.flags[xchg1];
  i.flags[xchg1] = temp_flags;

  temp_op = i.op[xchg2];
  i.op[xchg2] = i.op[xchg1];
  i.op[xchg1] = temp_op;

  temp_reloc = i.reloc[xchg2];
  i.reloc[xchg2] = i.reloc[xchg1];
  i.reloc[xchg1] = temp_reloc;

  temp_flags = i.imm_bits[xchg2];
  i.imm_bits[xchg2] = i.imm_bits[xchg1];
  i.imm_bits[xchg1] = temp_flags;

  if (i.mask.reg)
    {
      if (i.mask.operand == xchg1)
	i.mask.operand = xchg2;
      else if (i.mask.operand == xchg2)
	i.mask.operand = xchg1;
    }
  if (i.broadcast.type || i.broadcast.bytes)
    {
      if (i.broadcast.operand == xchg1)
	i.broadcast.operand = xchg2;
      else if (i.broadcast.operand == xchg2)
	i.broadcast.operand = xchg1;
    }
}

static void
swap_operands (void)
{
  switch (i.operands)
    {
    case 5:
    case 4:
      swap_2_operands (1, i.operands - 2);
      /* Fall through.  */
    case 3:
    case 2:
      swap_2_operands (0, i.operands - 1);
      break;
    default:
      abort ();
    }

  if (i.mem_operands == 2)
    {
      const reg_entry *temp_seg;
      temp_seg = i.seg[0];
      i.seg[0] = i.seg[1];
      i.seg[1] = temp_seg;
    }
}

/* Try to ensure constant immediates are represented in the smallest
   opcode possible.  */
static void
optimize_imm (void)
{
  char guess_suffix = 0;
  int op;

  if (i.suffix)
    guess_suffix = i.suffix;
  else if (i.reg_operands)
    {
      /* Figure out a suffix from the last register operand specified.
	 We can't do this properly yet, i.e. excluding special register
	 instances, but the following works for instructions with
	 immediates.  In any case, we can't set i.suffix yet.  */
      for (op = i.operands; --op >= 0;)
	if (i.types[op].bitfield.class != Reg)
	  continue;
	else if (i.types[op].bitfield.byte)
	  {
	    guess_suffix = BYTE_MNEM_SUFFIX;
	    break;
	  }
	else if (i.types[op].bitfield.word)
	  {
	    guess_suffix = WORD_MNEM_SUFFIX;
	    break;
	  }
	else if (i.types[op].bitfield.dword)
	  {
	    guess_suffix = LONG_MNEM_SUFFIX;
	    break;
	  }
	else if (i.types[op].bitfield.qword)
	  {
	    guess_suffix = QWORD_MNEM_SUFFIX;
	    break;
	  }
    }
  else if ((flag_code == CODE_16BIT) ^ (i.prefix[DATA_PREFIX] != 0))
    guess_suffix = WORD_MNEM_SUFFIX;
  else if (flag_code != CODE_64BIT || !(i.prefix[REX_PREFIX] & REX_W))
    guess_suffix = LONG_MNEM_SUFFIX;

  for (op = i.operands; --op >= 0;)
    if (operand_type_check (i.types[op], imm))
      {
	switch (i.op[op].imms->X_op)
	  {
	  case O_constant:
	    /* If a suffix is given, this operand may be shortened.  */
	    switch (guess_suffix)
	      {
	      case LONG_MNEM_SUFFIX:
		i.types[op].bitfield.imm32 = 1;
		i.types[op].bitfield.imm64 = 1;
		break;
	      case WORD_MNEM_SUFFIX:
		i.types[op].bitfield.imm16 = 1;
		i.types[op].bitfield.imm32 = 1;
		i.types[op].bitfield.imm32s = 1;
		i.types[op].bitfield.imm64 = 1;
		break;
	      case BYTE_MNEM_SUFFIX:
		i.types[op].bitfield.imm8 = 1;
		i.types[op].bitfield.imm8s = 1;
		i.types[op].bitfield.imm16 = 1;
		i.types[op].bitfield.imm32 = 1;
		i.types[op].bitfield.imm32s = 1;
		i.types[op].bitfield.imm64 = 1;
		break;
	      }

	    /* If this operand is at most 16 bits, convert it
	       to a signed 16 bit number before trying to see
	       whether it will fit in an even smaller size.
	       This allows a 16-bit operand such as $0xffe0 to
	       be recognised as within Imm8S range.  */
	    if ((i.types[op].bitfield.imm16)
		&& fits_in_unsigned_word (i.op[op].imms->X_add_number))
	      {
		i.op[op].imms->X_add_number = ((i.op[op].imms->X_add_number
						^ 0x8000) - 0x8000);
	      }
#ifdef BFD64
	    /* Store 32-bit immediate in 64-bit for 64-bit BFD.  */
	    if ((i.types[op].bitfield.imm32)
		&& fits_in_unsigned_long (i.op[op].imms->X_add_number))
	      {
		i.op[op].imms->X_add_number = ((i.op[op].imms->X_add_number
						^ ((offsetT) 1 << 31))
					       - ((offsetT) 1 << 31));
	      }
#endif
	    i.types[op]
	      = operand_type_or (i.types[op],
				 smallest_imm_type (i.op[op].imms->X_add_number));

	    /* We must avoid matching of Imm32 templates when 64bit
	       only immediate is available.  */
	    if (guess_suffix == QWORD_MNEM_SUFFIX)
	      i.types[op].bitfield.imm32 = 0;
	    break;

	  case O_absent:
	  case O_register:
	    abort ();

	    /* Symbols and expressions.  */
	  default:
	    /* Convert symbolic operand to proper sizes for matching, but don't
	       prevent matching a set of insns that only supports sizes other
	       than those matching the insn suffix.  */
	    {
	      i386_operand_type mask, allowed;
	      const insn_template *t = current_templates->start;

	      operand_type_set (&mask, 0);
	      switch (guess_suffix)
		{
		case QWORD_MNEM_SUFFIX:
		  mask.bitfield.imm64 = 1;
		  mask.bitfield.imm32s = 1;
		  break;
		case LONG_MNEM_SUFFIX:
		  mask.bitfield.imm32 = 1;
		  break;
		case WORD_MNEM_SUFFIX:
		  mask.bitfield.imm16 = 1;
		  break;
		case BYTE_MNEM_SUFFIX:
		  mask.bitfield.imm8 = 1;
		  break;
		default:
		  break;
		}

	      allowed = operand_type_and (t->operand_types[op], mask);
	      while (++t < current_templates->end)
		{
		  allowed = operand_type_or (allowed, t->operand_types[op]);
		  allowed = operand_type_and (allowed, mask);
		}

	      if (!operand_type_all_zero (&allowed))
		i.types[op] = operand_type_and (i.types[op], mask);
	    }
	    break;
	  }
      }
}

/* Try to use the smallest displacement type too.  */
static bool
optimize_disp (const insn_template *t)
{
  unsigned int op;

  if (!want_disp32 (t)
      && (!t->opcode_modifier.jump
	  || i.jumpabsolute || i.types[0].bitfield.baseindex))
    {
      for (op = 0; op < i.operands; ++op)
	{
	  const expressionS *exp = i.op[op].disps;

	  if (!operand_type_check (i.types[op], disp))
	    continue;

	  if (exp->X_op != O_constant)
	    continue;

	  /* Since displacement is signed extended to 64bit, don't allow
	     disp32 if it is out of range.  */
	  if (fits_in_signed_long (exp->X_add_number))
	    continue;

	  i.types[op].bitfield.disp32 = 0;
	  if (i.types[op].bitfield.baseindex)
	    {
	      as_bad (_("0x%" PRIx64 " out of range of signed 32bit displacement"),
		      (uint64_t) exp->X_add_number);
	      return false;
	    }
	}
    }

  /* Don't optimize displacement for movabs since it only takes 64bit
     displacement.  */
  if (i.disp_encoding > disp_encoding_8bit
      || (flag_code == CODE_64BIT && t->mnem_off == MN_movabs))
    return true;

  for (op = i.operands; op-- > 0;)
    if (operand_type_check (i.types[op], disp))
      {
	if (i.op[op].disps->X_op == O_constant)
	  {
	    offsetT op_disp = i.op[op].disps->X_add_number;

	    if (!op_disp && i.types[op].bitfield.baseindex)
	      {
		i.types[op] = operand_type_and_not (i.types[op], anydisp);
		i.op[op].disps = NULL;
		i.disp_operands--;
		continue;
	      }

	    if (i.types[op].bitfield.disp16
		&& fits_in_unsigned_word (op_disp))
	      {
		/* If this operand is at most 16 bits, convert
		   to a signed 16 bit number and don't use 64bit
		   displacement.  */
		op_disp = ((op_disp ^ 0x8000) - 0x8000);
		i.types[op].bitfield.disp64 = 0;
	      }

#ifdef BFD64
	    /* Optimize 64-bit displacement to 32-bit for 64-bit BFD.  */
	    if ((flag_code != CODE_64BIT
		 ? i.types[op].bitfield.disp32
		 : want_disp32 (t)
		   && (!t->opcode_modifier.jump
		       || i.jumpabsolute || i.types[op].bitfield.baseindex))
		&& fits_in_unsigned_long (op_disp))
	      {
		/* If this operand is at most 32 bits, convert
		   to a signed 32 bit number and don't use 64bit
		   displacement.  */
		op_disp = (op_disp ^ ((offsetT) 1 << 31)) - ((addressT) 1 << 31);
		i.types[op].bitfield.disp64 = 0;
		i.types[op].bitfield.disp32 = 1;
	      }

	    if (flag_code == CODE_64BIT && fits_in_signed_long (op_disp))
	      {
		i.types[op].bitfield.disp64 = 0;
		i.types[op].bitfield.disp32 = 1;
	      }
#endif
	    if ((i.types[op].bitfield.disp32
		 || i.types[op].bitfield.disp16)
		&& fits_in_disp8 (op_disp))
	      i.types[op].bitfield.disp8 = 1;

	    i.op[op].disps->X_add_number = op_disp;
	  }
	else if (i.reloc[op] == BFD_RELOC_386_TLS_DESC_CALL
		 || i.reloc[op] == BFD_RELOC_X86_64_TLSDESC_CALL)
	  {
	    fix_new_exp (frag_now, frag_more (0) - frag_now->fr_literal, 0,
			 i.op[op].disps, 0, i.reloc[op]);
	    i.types[op] = operand_type_and_not (i.types[op], anydisp);
	  }
 	else
	  /* We only support 64bit displacement on constants.  */
	  i.types[op].bitfield.disp64 = 0;
      }

  return true;
}

/* Return 1 if there is a match in broadcast bytes between operand
   GIVEN and instruction template T.   */

static INLINE int
match_broadcast_size (const insn_template *t, unsigned int given)
{
  return ((t->opcode_modifier.broadcast == BYTE_BROADCAST
	   && i.types[given].bitfield.byte)
	  || (t->opcode_modifier.broadcast == WORD_BROADCAST
	      && i.types[given].bitfield.word)
	  || (t->opcode_modifier.broadcast == DWORD_BROADCAST
	      && i.types[given].bitfield.dword)
	  || (t->opcode_modifier.broadcast == QWORD_BROADCAST
	      && i.types[given].bitfield.qword));
}

/* Check if operands are valid for the instruction.  */

static int
check_VecOperands (const insn_template *t)
{
  unsigned int op;
  i386_cpu_flags cpu;

  /* Templates allowing for ZMMword as well as YMMword and/or XMMword for
     any one operand are implicity requiring AVX512VL support if the actual
     operand size is YMMword or XMMword.  Since this function runs after
     template matching, there's no need to check for YMMword/XMMword in
     the template.  */
  cpu = cpu_flags_and (t->cpu_flags, avx512);
  if (!cpu_flags_all_zero (&cpu)
      && !t->cpu_flags.bitfield.cpuavx512vl
      && !cpu_arch_flags.bitfield.cpuavx512vl)
    {
      for (op = 0; op < t->operands; ++op)
	{
	  if (t->operand_types[op].bitfield.zmmword
	      && (i.types[op].bitfield.ymmword
		  || i.types[op].bitfield.xmmword))
	    {
	      i.error = unsupported;
	      return 1;
	    }
	}
    }

  /* Somewhat similarly, templates specifying both AVX and AVX2 are
     requiring AVX2 support if the actual operand size is YMMword.  */
  if (t->cpu_flags.bitfield.cpuavx
      && t->cpu_flags.bitfield.cpuavx2
      && !cpu_arch_flags.bitfield.cpuavx2)
    {
      for (op = 0; op < t->operands; ++op)
	{
	  if (t->operand_types[op].bitfield.xmmword
	      && i.types[op].bitfield.ymmword)
	    {
	      i.error = unsupported;
	      return 1;
	    }
	}
    }

  /* Without VSIB byte, we can't have a vector register for index.  */
  if (!t->opcode_modifier.sib
      && i.index_reg
      && (i.index_reg->reg_type.bitfield.xmmword
	  || i.index_reg->reg_type.bitfield.ymmword
	  || i.index_reg->reg_type.bitfield.zmmword))
    {
      i.error = unsupported_vector_index_register;
      return 1;
    }

  /* Check if default mask is allowed.  */
  if (t->opcode_modifier.operandconstraint == NO_DEFAULT_MASK
      && (!i.mask.reg || i.mask.reg->reg_num == 0))
    {
      i.error = no_default_mask;
      return 1;
    }

  /* For VSIB byte, we need a vector register for index, and all vector
     registers must be distinct.  */
  if (t->opcode_modifier.sib && t->opcode_modifier.sib != SIBMEM)
    {
      if (!i.index_reg
	  || !((t->opcode_modifier.sib == VECSIB128
		&& i.index_reg->reg_type.bitfield.xmmword)
	       || (t->opcode_modifier.sib == VECSIB256
		   && i.index_reg->reg_type.bitfield.ymmword)
	       || (t->opcode_modifier.sib == VECSIB512
		   && i.index_reg->reg_type.bitfield.zmmword)))
      {
	i.error = invalid_vsib_address;
	return 1;
      }

      gas_assert (i.reg_operands == 2 || i.mask.reg);
      if (i.reg_operands == 2 && !i.mask.reg)
	{
	  gas_assert (i.types[0].bitfield.class == RegSIMD);
	  gas_assert (i.types[0].bitfield.xmmword
		      || i.types[0].bitfield.ymmword);
	  gas_assert (i.types[2].bitfield.class == RegSIMD);
	  gas_assert (i.types[2].bitfield.xmmword
		      || i.types[2].bitfield.ymmword);
	  if (operand_check == check_none)
	    return 0;
	  if (register_number (i.op[0].regs)
	      != register_number (i.index_reg)
	      && register_number (i.op[2].regs)
		 != register_number (i.index_reg)
	      && register_number (i.op[0].regs)
		 != register_number (i.op[2].regs))
	    return 0;
	  if (operand_check == check_error)
	    {
	      i.error = invalid_vector_register_set;
	      return 1;
	    }
	  as_warn (_("mask, index, and destination registers should be distinct"));
	}
      else if (i.reg_operands == 1 && i.mask.reg)
	{
	  if (i.types[1].bitfield.class == RegSIMD
	      && (i.types[1].bitfield.xmmword
	          || i.types[1].bitfield.ymmword
	          || i.types[1].bitfield.zmmword)
	      && (register_number (i.op[1].regs)
		  == register_number (i.index_reg)))
	    {
	      if (operand_check == check_error)
		{
		  i.error = invalid_vector_register_set;
		  return 1;
		}
	      if (operand_check != check_none)
		as_warn (_("index and destination registers should be distinct"));
	    }
	}
    }

  /* For AMX instructions with 3 TMM register operands, all operands
      must be distinct.  */
  if (i.reg_operands == 3
      && t->operand_types[0].bitfield.tmmword
      && (i.op[0].regs == i.op[1].regs
          || i.op[0].regs == i.op[2].regs
          || i.op[1].regs == i.op[2].regs))
    {
      i.error = invalid_tmm_register_set;
      return 1;
    }

  /* For some special instructions require that destination must be distinct
     from source registers.  */
  if (t->opcode_modifier.operandconstraint == DISTINCT_DEST)
    {
      unsigned int dest_reg = i.operands - 1;

      know (i.operands >= 3);

      /* #UD if dest_reg == src1_reg or dest_reg == src2_reg.  */
      if (i.op[dest_reg - 1].regs == i.op[dest_reg].regs
	  || (i.reg_operands > 2
	      && i.op[dest_reg - 2].regs == i.op[dest_reg].regs))
	{
	  i.error = invalid_dest_and_src_register_set;
	  return 1;
	}
    }

  /* Check if broadcast is supported by the instruction and is applied
     to the memory operand.  */
  if (i.broadcast.type || i.broadcast.bytes)
    {
      i386_operand_type type, overlap;

      /* Check if specified broadcast is supported in this instruction,
	 and its broadcast bytes match the memory operand.  */
      op = i.broadcast.operand;
      if (!t->opcode_modifier.broadcast
	  || !(i.flags[op] & Operand_Mem)
	  || (!i.types[op].bitfield.unspecified
	      && !match_broadcast_size (t, op)))
	{
	bad_broadcast:
	  i.error = unsupported_broadcast;
	  return 1;
	}

      operand_type_set (&type, 0);
      switch (get_broadcast_bytes (t, false))
	{
	case 2:
	  type.bitfield.word = 1;
	  break;
	case 4:
	  type.bitfield.dword = 1;
	  break;
	case 8:
	  type.bitfield.qword = 1;
	  break;
	case 16:
	  type.bitfield.xmmword = 1;
	  break;
	case 32:
	  type.bitfield.ymmword = 1;
	  break;
	case 64:
	  type.bitfield.zmmword = 1;
	  break;
	default:
	  goto bad_broadcast;
	}

      overlap = operand_type_and (type, t->operand_types[op]);
      if (t->operand_types[op].bitfield.class == RegSIMD
	  && t->operand_types[op].bitfield.byte
	     + t->operand_types[op].bitfield.word
	     + t->operand_types[op].bitfield.dword
	     + t->operand_types[op].bitfield.qword > 1)
	{
	  overlap.bitfield.xmmword = 0;
	  overlap.bitfield.ymmword = 0;
	  overlap.bitfield.zmmword = 0;
	}
      if (operand_type_all_zero (&overlap))
	  goto bad_broadcast;

      if (t->opcode_modifier.checkoperandsize)
	{
	  unsigned int j;

	  type.bitfield.baseindex = 1;
	  for (j = 0; j < i.operands; ++j)
	    {
	      if (j != op
		  && !operand_type_register_match(i.types[j],
						  t->operand_types[j],
						  type,
						  t->operand_types[op]))
		goto bad_broadcast;
	    }
	}
    }
  /* If broadcast is supported in this instruction, we need to check if
     operand of one-element size isn't specified without broadcast.  */
  else if (t->opcode_modifier.broadcast && i.mem_operands)
    {
      /* Find memory operand.  */
      for (op = 0; op < i.operands; op++)
	if (i.flags[op] & Operand_Mem)
	  break;
      gas_assert (op < i.operands);
      /* Check size of the memory operand.  */
      if (match_broadcast_size (t, op))
	{
	  i.error = broadcast_needed;
	  return 1;
	}
    }
  else
    op = MAX_OPERANDS - 1; /* Avoid uninitialized variable warning.  */

  /* Check if requested masking is supported.  */
  if (i.mask.reg)
    {
      if (!t->opcode_modifier.masking)
	{
	  i.error = unsupported_masking;
	  return 1;
	}

      /* Common rules for masking:
	 - mask register destinations permit only zeroing-masking, without
	   that actually being expressed by a {z} operand suffix or EVEX.z,
	 - memory destinations allow only merging-masking,
	 - scatter/gather insns (i.e. ones using vSIB) only allow merging-
	   masking.  */
      if (i.mask.zeroing
	  && (t->operand_types[t->operands - 1].bitfield.class == RegMask
	      || (i.flags[t->operands - 1] & Operand_Mem)
	      || t->opcode_modifier.sib))
	{
	  i.error = unsupported_masking;
	  return 1;
	}
    }

  /* Check if masking is applied to dest operand.  */
  if (i.mask.reg && (i.mask.operand != i.operands - 1))
    {
      i.error = mask_not_on_destination;
      return 1;
    }

  /* Check RC/SAE.  */
  if (i.rounding.type != rc_none)
    {
      if (!t->opcode_modifier.sae
	  || ((i.rounding.type != saeonly) != t->opcode_modifier.staticrounding)
	  || i.mem_operands)
	{
	  i.error = unsupported_rc_sae;
	  return 1;
	}

      /* Non-EVEX.LIG forms need to have a ZMM register as at least one
	 operand.  */
      if (t->opcode_modifier.evex != EVEXLIG)
	{
	  for (op = 0; op < t->operands; ++op)
	    if (i.types[op].bitfield.zmmword)
	      break;
	  if (op >= t->operands)
	    {
	      i.error = operand_size_mismatch;
	      return 1;
	    }
	}
    }

  /* Check the special Imm4 cases; must be the first operand.  */
  if (t->cpu_flags.bitfield.cpuxop && t->operands == 5)
    {
      if (i.op[0].imms->X_op != O_constant
	  || !fits_in_imm4 (i.op[0].imms->X_add_number))
	{
	  i.error = bad_imm4;
	  return 1;
	}

      /* Turn off Imm<N> so that update_imm won't complain.  */
      operand_type_set (&i.types[0], 0);
    }

  /* Check vector Disp8 operand.  */
  if (t->opcode_modifier.disp8memshift
      && i.disp_encoding <= disp_encoding_8bit)
    {
      if (i.broadcast.type || i.broadcast.bytes)
	i.memshift = t->opcode_modifier.broadcast - 1;
      else if (t->opcode_modifier.disp8memshift != DISP8_SHIFT_VL)
	i.memshift = t->opcode_modifier.disp8memshift;
      else
	{
	  const i386_operand_type *type = NULL, *fallback = NULL;

	  i.memshift = 0;
	  for (op = 0; op < i.operands; op++)
	    if (i.flags[op] & Operand_Mem)
	      {
		if (t->opcode_modifier.evex == EVEXLIG)
		  i.memshift = 2 + (i.suffix == QWORD_MNEM_SUFFIX);
		else if (t->operand_types[op].bitfield.xmmword
			 + t->operand_types[op].bitfield.ymmword
			 + t->operand_types[op].bitfield.zmmword <= 1)
		  type = &t->operand_types[op];
		else if (!i.types[op].bitfield.unspecified)
		  type = &i.types[op];
		else /* Ambiguities get resolved elsewhere.  */
		  fallback = &t->operand_types[op];
	      }
	    else if (i.types[op].bitfield.class == RegSIMD
		     && t->opcode_modifier.evex != EVEXLIG)
	      {
		if (i.types[op].bitfield.zmmword)
		  i.memshift = 6;
		else if (i.types[op].bitfield.ymmword && i.memshift < 5)
		  i.memshift = 5;
		else if (i.types[op].bitfield.xmmword && i.memshift < 4)
		  i.memshift = 4;
	      }

	  if (!type && !i.memshift)
	    type = fallback;
	  if (type)
	    {
	      if (type->bitfield.zmmword)
		i.memshift = 6;
	      else if (type->bitfield.ymmword)
		i.memshift = 5;
	      else if (type->bitfield.xmmword)
		i.memshift = 4;
	    }

	  /* For the check in fits_in_disp8().  */
	  if (i.memshift == 0)
	    i.memshift = -1;
	}

      for (op = 0; op < i.operands; op++)
	if (operand_type_check (i.types[op], disp)
	    && i.op[op].disps->X_op == O_constant)
	  {
	    if (fits_in_disp8 (i.op[op].disps->X_add_number))
	      {
		i.types[op].bitfield.disp8 = 1;
		return 0;
	      }
	    i.types[op].bitfield.disp8 = 0;
	  }
    }

  i.memshift = 0;

  return 0;
}

/* Check if encoding requirements are met by the instruction.  */

static int
VEX_check_encoding (const insn_template *t)
{
  if (i.vec_encoding == vex_encoding_error)
    {
      i.error = unsupported;
      return 1;
    }

  if (i.vec_encoding == vex_encoding_evex)
    {
      /* This instruction must be encoded with EVEX prefix.  */
      if (!is_evex_encoding (t))
	{
	  i.error = unsupported;
	  return 1;
	}
      return 0;
    }

  if (!t->opcode_modifier.vex)
    {
      /* This instruction template doesn't have VEX prefix.  */
      if (i.vec_encoding != vex_encoding_default)
	{
	  i.error = unsupported;
	  return 1;
	}
      return 0;
    }

  return 0;
}

/* Helper function for the progress() macro in match_template().  */
static INLINE enum i386_error progress (enum i386_error new,
					enum i386_error last,
					unsigned int line, unsigned int *line_p)
{
  if (line <= *line_p)
    return last;
  *line_p = line;
  return new;
}

static const insn_template *
match_template (char mnem_suffix)
{
  /* Points to template once we've found it.  */
  const insn_template *t;
  i386_operand_type overlap0, overlap1, overlap2, overlap3;
  i386_operand_type overlap4;
  unsigned int found_reverse_match;
  i386_operand_type operand_types [MAX_OPERANDS];
  int addr_prefix_disp;
  unsigned int j, size_match, check_register, errline = __LINE__;
  enum i386_error specific_error = number_of_operands_mismatch;
#define progress(err) progress (err, specific_error, __LINE__, &errline)

#if MAX_OPERANDS != 5
# error "MAX_OPERANDS must be 5."
#endif

  found_reverse_match = 0;
  addr_prefix_disp = -1;

  for (t = current_templates->start; t < current_templates->end; t++)
    {
      addr_prefix_disp = -1;
      found_reverse_match = 0;

      /* Must have right number of operands.  */
      if (i.operands != t->operands)
	continue;

      /* Check processor support.  */
      specific_error = progress (unsupported);
      if (cpu_flags_match (t) != CPU_FLAGS_PERFECT_MATCH)
	continue;

      /* Check AT&T mnemonic.   */
      specific_error = progress (unsupported_with_intel_mnemonic);
      if (intel_mnemonic && t->opcode_modifier.attmnemonic)
	continue;

      /* Check AT&T/Intel syntax.  */
      specific_error = progress (unsupported_syntax);
      if ((intel_syntax && t->opcode_modifier.attsyntax)
	  || (!intel_syntax && t->opcode_modifier.intelsyntax))
	continue;

      /* Check Intel64/AMD64 ISA.   */
      switch (isa64)
	{
	default:
	  /* Default: Don't accept Intel64.  */
	  if (t->opcode_modifier.isa64 == INTEL64)
	    continue;
	  break;
	case amd64:
	  /* -mamd64: Don't accept Intel64 and Intel64 only.  */
	  if (t->opcode_modifier.isa64 >= INTEL64)
	    continue;
	  break;
	case intel64:
	  /* -mintel64: Don't accept AMD64.  */
	  if (t->opcode_modifier.isa64 == AMD64 && flag_code == CODE_64BIT)
	    continue;
	  break;
	}

      /* Check the suffix.  */
      specific_error = progress (invalid_instruction_suffix);
      if ((t->opcode_modifier.no_bsuf && mnem_suffix == BYTE_MNEM_SUFFIX)
	  || (t->opcode_modifier.no_wsuf && mnem_suffix == WORD_MNEM_SUFFIX)
	  || (t->opcode_modifier.no_lsuf && mnem_suffix == LONG_MNEM_SUFFIX)
	  || (t->opcode_modifier.no_ssuf && mnem_suffix == SHORT_MNEM_SUFFIX)
	  || (t->opcode_modifier.no_qsuf && mnem_suffix == QWORD_MNEM_SUFFIX))
	continue;

      specific_error = progress (operand_size_mismatch);
      size_match = operand_size_match (t);
      if (!size_match)
	continue;

      /* This is intentionally not

	 if (i.jumpabsolute != (t->opcode_modifier.jump == JUMP_ABSOLUTE))

	 as the case of a missing * on the operand is accepted (perhaps with
	 a warning, issued further down).  */
      specific_error = progress (operand_type_mismatch);
      if (i.jumpabsolute && t->opcode_modifier.jump != JUMP_ABSOLUTE)
	continue;

      /* In Intel syntax, normally we can check for memory operand size when
	 there is no mnemonic suffix.  But jmp and call have 2 different
	 encodings with Dword memory operand size.  Skip the "near" one
	 (permitting a register operand) when "far" was requested.  */
      if (i.far_branch
	  && t->opcode_modifier.jump == JUMP_ABSOLUTE
	  && t->operand_types[0].bitfield.class == Reg)
	continue;

      for (j = 0; j < MAX_OPERANDS; j++)
	operand_types[j] = t->operand_types[j];

      /* In general, don't allow 32-bit operands on pre-386.  */
      specific_error = progress (mnem_suffix ? invalid_instruction_suffix
					     : operand_size_mismatch);
      j = i.imm_operands + (t->operands > i.imm_operands + 1);
      if (i.suffix == LONG_MNEM_SUFFIX
	  && !cpu_arch_flags.bitfield.cpui386
	  && (intel_syntax
	      ? (t->opcode_modifier.mnemonicsize != IGNORESIZE
		 && !intel_float_operand (insn_name (t)))
	      : intel_float_operand (insn_name (t)) != 2)
	  && (t->operands == i.imm_operands
	      || (operand_types[i.imm_operands].bitfield.class != RegMMX
	       && operand_types[i.imm_operands].bitfield.class != RegSIMD
	       && operand_types[i.imm_operands].bitfield.class != RegMask)
	      || (operand_types[j].bitfield.class != RegMMX
		  && operand_types[j].bitfield.class != RegSIMD
		  && operand_types[j].bitfield.class != RegMask))
	  && !t->opcode_modifier.sib)
	continue;

      /* Do not verify operands when there are none.  */
      if (!t->operands)
	{
	  if (VEX_check_encoding (t))
	    {
	      specific_error = progress (i.error);
	      continue;
	    }

	  /* We've found a match; break out of loop.  */
	  break;
	}

      if (!t->opcode_modifier.jump
	  || t->opcode_modifier.jump == JUMP_ABSOLUTE)
	{
	  /* There should be only one Disp operand.  */
	  for (j = 0; j < MAX_OPERANDS; j++)
	    if (operand_type_check (operand_types[j], disp))
	      break;
	  if (j < MAX_OPERANDS)
	    {
	      bool override = (i.prefix[ADDR_PREFIX] != 0);

	      addr_prefix_disp = j;

	      /* Address size prefix will turn Disp64 operand into Disp32 and
		 Disp32/Disp16 one into Disp16/Disp32 respectively.  */
	      switch (flag_code)
		{
		case CODE_16BIT:
		  override = !override;
		  /* Fall through.  */
		case CODE_32BIT:
		  if (operand_types[j].bitfield.disp32
		      && operand_types[j].bitfield.disp16)
		    {
		      operand_types[j].bitfield.disp16 = override;
		      operand_types[j].bitfield.disp32 = !override;
		    }
		  gas_assert (!operand_types[j].bitfield.disp64);
		  break;

		case CODE_64BIT:
		  if (operand_types[j].bitfield.disp64)
		    {
		      gas_assert (!operand_types[j].bitfield.disp32);
		      operand_types[j].bitfield.disp32 = override;
		      operand_types[j].bitfield.disp64 = !override;
		    }
		  operand_types[j].bitfield.disp16 = 0;
		  break;
		}
	    }
	}

      /* We check register size if needed.  */
      if (t->opcode_modifier.checkoperandsize)
	{
	  check_register = (1 << t->operands) - 1;
	  if (i.broadcast.type || i.broadcast.bytes)
	    check_register &= ~(1 << i.broadcast.operand);
	}
      else
	check_register = 0;

      overlap0 = operand_type_and (i.types[0], operand_types[0]);
      switch (t->operands)
	{
	case 1:
	  if (!operand_type_match (overlap0, i.types[0]))
	    continue;

	  /* Allow the ModR/M encoding to be requested by using the {load} or
	     {store} pseudo prefix on an applicable insn.  */
	  if (!t->opcode_modifier.modrm
	      && i.reg_operands == 1
	      && ((i.dir_encoding == dir_encoding_load
		   && t->mnem_off != MN_pop)
		  || (i.dir_encoding == dir_encoding_store
		      && t->mnem_off != MN_push))
	      /* Avoid BSWAP.  */
	      && t->mnem_off != MN_bswap)
	    continue;
	  break;

	case 2:
	  /* xchg %eax, %eax is a special case. It is an alias for nop
	     only in 32bit mode and we can use opcode 0x90.  In 64bit
	     mode, we can't use 0x90 for xchg %eax, %eax since it should
	     zero-extend %eax to %rax.  */
	  if (t->base_opcode == 0x90
	      && t->opcode_space == SPACE_BASE)
	    {
	      if (flag_code == CODE_64BIT
		  && i.types[0].bitfield.instance == Accum
		  && i.types[0].bitfield.dword
		  && i.types[1].bitfield.instance == Accum)
		continue;

	      /* Allow the ModR/M encoding to be requested by using the
		 {load} or {store} pseudo prefix.  */
	      if (i.dir_encoding == dir_encoding_load
		  || i.dir_encoding == dir_encoding_store)
		continue;
	    }

	  if (t->base_opcode == MOV_AX_DISP32
	      && t->opcode_space == SPACE_BASE
	      && t->mnem_off != MN_movabs)
	    {
	      /* Force 0x8b encoding for "mov foo@GOT, %eax".  */
	      if (i.reloc[0] == BFD_RELOC_386_GOT32)
		continue;

	      /* xrelease mov %eax, <disp> is another special case. It must not
		 match the accumulator-only encoding of mov.  */
	      if (i.hle_prefix)
		continue;

	      /* Allow the ModR/M encoding to be requested by using a suitable
		 {load} or {store} pseudo prefix.  */
	      if (i.dir_encoding == (i.types[0].bitfield.instance == Accum
				     ? dir_encoding_store
				     : dir_encoding_load)
		  && !i.types[0].bitfield.disp64
		  && !i.types[1].bitfield.disp64)
		continue;
	    }

	  /* Allow the ModR/M encoding to be requested by using the {load} or
	     {store} pseudo prefix on an applicable insn.  */
	  if (!t->opcode_modifier.modrm
	      && i.reg_operands == 1
	      && i.imm_operands == 1
	      && (i.dir_encoding == dir_encoding_load
		  || i.dir_encoding == dir_encoding_store)
	      && t->opcode_space == SPACE_BASE)
	    {
	      if (t->base_opcode == 0xb0 /* mov $imm, %reg */
		  && i.dir_encoding == dir_encoding_store)
		continue;

	      if ((t->base_opcode | 0x38) == 0x3c /* <alu> $imm, %acc */
		  && (t->base_opcode != 0x3c /* cmp $imm, %acc */
		      || i.dir_encoding == dir_encoding_load))
		continue;

	      if (t->base_opcode == 0xa8 /* test $imm, %acc */
		  && i.dir_encoding == dir_encoding_load)
		continue;
	    }
	  /* Fall through.  */

	case 3:
	  if (!(size_match & MATCH_STRAIGHT))
	    goto check_reverse;
	  /* Reverse direction of operands if swapping is possible in the first
	     place (operands need to be symmetric) and
	     - the load form is requested, and the template is a store form,
	     - the store form is requested, and the template is a load form,
	     - the non-default (swapped) form is requested.  */
	  overlap1 = operand_type_and (operand_types[0], operand_types[1]);
	  if (t->opcode_modifier.d && i.reg_operands == i.operands
	      && !operand_type_all_zero (&overlap1))
	    switch (i.dir_encoding)
	      {
	      case dir_encoding_load:
		if (operand_type_check (operand_types[i.operands - 1], anymem)
		    || t->opcode_modifier.regmem)
		  goto check_reverse;
		break;

	      case dir_encoding_store:
		if (!operand_type_check (operand_types[i.operands - 1], anymem)
		    && !t->opcode_modifier.regmem)
		  goto check_reverse;
		break;

	      case dir_encoding_swap:
		goto check_reverse;

	      case dir_encoding_default:
		break;
	      }
	  /* If we want store form, we skip the current load.  */
	  if ((i.dir_encoding == dir_encoding_store
	       || i.dir_encoding == dir_encoding_swap)
	      && i.mem_operands == 0
	      && t->opcode_modifier.load)
	    continue;
	  /* Fall through.  */
	case 4:
	case 5:
	  overlap1 = operand_type_and (i.types[1], operand_types[1]);
	  if (!operand_type_match (overlap0, i.types[0])
	      || !operand_type_match (overlap1, i.types[1])
	      || ((check_register & 3) == 3
		  && !operand_type_register_match (i.types[0],
						   operand_types[0],
						   i.types[1],
						   operand_types[1])))
	    {
	      specific_error = progress (i.error);

	      /* Check if other direction is valid ...  */
	      if (!t->opcode_modifier.d)
		continue;

	    check_reverse:
	      if (!(size_match & MATCH_REVERSE))
		continue;
	      /* Try reversing direction of operands.  */
	      j = t->cpu_flags.bitfield.cpufma4
		  || t->cpu_flags.bitfield.cpuxop ? 1 : i.operands - 1;
	      overlap0 = operand_type_and (i.types[0], operand_types[j]);
	      overlap1 = operand_type_and (i.types[j], operand_types[0]);
	      overlap2 = operand_type_and (i.types[1], operand_types[1]);
	      gas_assert (t->operands != 3 || !check_register);
	      if (!operand_type_match (overlap0, i.types[0])
		  || !operand_type_match (overlap1, i.types[j])
		  || (t->operands == 3
		      && !operand_type_match (overlap2, i.types[1]))
		  || (check_register
		      && !operand_type_register_match (i.types[0],
						       operand_types[j],
						       i.types[j],
						       operand_types[0])))
		{
		  /* Does not match either direction.  */
		  specific_error = progress (i.error);
		  continue;
		}
	      /* found_reverse_match holds which variant of D
		 we've found.  */
	      if (!t->opcode_modifier.d)
		found_reverse_match = 0;
	      else if (operand_types[0].bitfield.tbyte)
		{
		  if (t->opcode_modifier.operandconstraint != UGH)
		    found_reverse_match = Opcode_FloatD;
		  else
		    found_reverse_match = ~0;
		  /* FSUB{,R} and FDIV{,R} may need a 2nd bit flipped.  */
		  if ((t->extension_opcode & 4)
		      && (intel_syntax || intel_mnemonic))
		    found_reverse_match |= Opcode_FloatR;
		}
	      else if (t->cpu_flags.bitfield.cpufma4
		       || t->cpu_flags.bitfield.cpuxop)
		{
		  found_reverse_match = Opcode_VexW;
		  goto check_operands_345;
		}
	      else if (t->opcode_space != SPACE_BASE
		       && (t->opcode_space != SPACE_0F
			   /* MOV to/from CR/DR/TR, as an exception, follow
			      the base opcode space encoding model.  */
			   || (t->base_opcode | 7) != 0x27))
		found_reverse_match = (t->base_opcode & 0xee) != 0x6e
				      ? Opcode_ExtD : Opcode_SIMD_IntD;
	      else if (!t->opcode_modifier.commutative)
		found_reverse_match = Opcode_D;
	      else
		found_reverse_match = ~0;
	    }
	  else
	    {
	      /* Found a forward 2 operand match here.  */
	    check_operands_345:
	      switch (t->operands)
		{
		case 5:
		  overlap4 = operand_type_and (i.types[4], operand_types[4]);
		  if (!operand_type_match (overlap4, i.types[4])
		      || !operand_type_register_match (i.types[3],
						       operand_types[3],
						       i.types[4],
						       operand_types[4]))
		    {
		      specific_error = progress (i.error);
		      continue;
		    }
		  /* Fall through.  */
		case 4:
		  overlap3 = operand_type_and (i.types[3], operand_types[3]);
		  if (!operand_type_match (overlap3, i.types[3])
		      || ((check_register & 0xa) == 0xa
			  && !operand_type_register_match (i.types[1],
							    operand_types[1],
							    i.types[3],
							    operand_types[3]))
		      || ((check_register & 0xc) == 0xc
			  && !operand_type_register_match (i.types[2],
							    operand_types[2],
							    i.types[3],
							    operand_types[3])))
		    {
		      specific_error = progress (i.error);
		      continue;
		    }
		  /* Fall through.  */
		case 3:
		  overlap2 = operand_type_and (i.types[2], operand_types[2]);
		  if (!operand_type_match (overlap2, i.types[2])
		      || ((check_register & 5) == 5
			  && !operand_type_register_match (i.types[0],
							    operand_types[0],
							    i.types[2],
							    operand_types[2]))
		      || ((check_register & 6) == 6
			  && !operand_type_register_match (i.types[1],
							    operand_types[1],
							    i.types[2],
							    operand_types[2])))
		    {
		      specific_error = progress (i.error);
		      continue;
		    }
		  break;
		}
	    }
	  /* Found either forward/reverse 2, 3 or 4 operand match here:
	     slip through to break.  */
	}

      /* Check if VEX/EVEX encoding requirements can be satisfied.  */
      if (VEX_check_encoding (t))
	{
	  specific_error = progress (i.error);
	  continue;
	}

      /* Check if vector operands are valid.  */
      if (check_VecOperands (t))
	{
	  specific_error = progress (i.error);
	  continue;
	}

      /* We've found a match; break out of loop.  */
      break;
    }

#undef progress

  if (t == current_templates->end)
    {
      /* We found no match.  */
      i.error = specific_error;
      return NULL;
    }

  if (!quiet_warnings)
    {
      if (!intel_syntax
	  && (i.jumpabsolute != (t->opcode_modifier.jump == JUMP_ABSOLUTE)))
	as_warn (_("indirect %s without `*'"), insn_name (t));

      if (t->opcode_modifier.isprefix
	  && t->opcode_modifier.mnemonicsize == IGNORESIZE)
	{
	  /* Warn them that a data or address size prefix doesn't
	     affect assembly of the next line of code.  */
	  as_warn (_("stand-alone `%s' prefix"), insn_name (t));
	}
    }

  /* Copy the template we found.  */
  install_template (t);

  if (addr_prefix_disp != -1)
    i.tm.operand_types[addr_prefix_disp]
      = operand_types[addr_prefix_disp];

  switch (found_reverse_match)
    {
    case 0:
      break;

    case Opcode_FloatR:
    case Opcode_FloatR | Opcode_FloatD:
      i.tm.extension_opcode ^= Opcode_FloatR >> 3;
      found_reverse_match &= Opcode_FloatD;

      /* Fall through.  */
    default:
      /* If we found a reverse match we must alter the opcode direction
	 bit and clear/flip the regmem modifier one.  found_reverse_match
	 holds bits to change (different for int & float insns).  */

      i.tm.base_opcode ^= found_reverse_match;

      /* Certain SIMD insns have their load forms specified in the opcode
	 table, and hence we need to _set_ RegMem instead of clearing it.
	 We need to avoid setting the bit though on insns like KMOVW.  */
      i.tm.opcode_modifier.regmem
	= i.tm.opcode_modifier.modrm && i.tm.opcode_modifier.d
	  && i.tm.operands > 2U - i.tm.opcode_modifier.sse2avx
	  && !i.tm.opcode_modifier.regmem;

      /* Fall through.  */
    case ~0:
      i.tm.operand_types[0] = operand_types[i.operands - 1];
      i.tm.operand_types[i.operands - 1] = operand_types[0];
      break;

    case Opcode_VexW:
      /* Only the first two register operands need reversing, alongside
	 flipping VEX.W.  */
      i.tm.opcode_modifier.vexw ^= VEXW0 ^ VEXW1;

      j = i.tm.operand_types[0].bitfield.imm8;
      i.tm.operand_types[j] = operand_types[j + 1];
      i.tm.operand_types[j + 1] = operand_types[j];
      break;
    }

  return t;
}

static int
check_string (void)
{
  unsigned int es_op = i.tm.opcode_modifier.isstring - IS_STRING_ES_OP0;
  unsigned int op = i.tm.operand_types[0].bitfield.baseindex ? es_op : 0;

  if (i.seg[op] != NULL && i.seg[op] != reg_es)
    {
      as_bad (_("`%s' operand %u must use `%ses' segment"),
	      insn_name (&i.tm),
	      intel_syntax ? i.tm.operands - es_op : es_op + 1,
	      register_prefix);
      return 0;
    }

  /* There's only ever one segment override allowed per instruction.
     This instruction possibly has a legal segment override on the
     second operand, so copy the segment to where non-string
     instructions store it, allowing common code.  */
  i.seg[op] = i.seg[1];

  return 1;
}

static int
process_suffix (void)
{
  bool is_movx = false;

  /* If matched instruction specifies an explicit instruction mnemonic
     suffix, use it.  */
  if (i.tm.opcode_modifier.size == SIZE16)
    i.suffix = WORD_MNEM_SUFFIX;
  else if (i.tm.opcode_modifier.size == SIZE32)
    i.suffix = LONG_MNEM_SUFFIX;
  else if (i.tm.opcode_modifier.size == SIZE64)
    i.suffix = QWORD_MNEM_SUFFIX;
  else if (i.reg_operands
	   && (i.operands > 1 || i.types[0].bitfield.class == Reg)
	   && i.tm.opcode_modifier.operandconstraint != ADDR_PREFIX_OP_REG)
    {
      unsigned int numop = i.operands;

      /* MOVSX/MOVZX */
      is_movx = (i.tm.opcode_space == SPACE_0F
		 && (i.tm.base_opcode | 8) == 0xbe)
		|| (i.tm.opcode_space == SPACE_BASE
		    && i.tm.base_opcode == 0x63
		    && i.tm.cpu_flags.bitfield.cpu64);

      /* movsx/movzx want only their source operand considered here, for the
	 ambiguity checking below.  The suffix will be replaced afterwards
	 to represent the destination (register).  */
      if (is_movx && (i.tm.opcode_modifier.w || i.tm.base_opcode == 0x63))
	--i.operands;

      /* crc32 needs REX.W set regardless of suffix / source operand size.  */
      if (i.tm.mnem_off == MN_crc32 && i.tm.operand_types[1].bitfield.qword)
        i.rex |= REX_W;

      /* If there's no instruction mnemonic suffix we try to invent one
	 based on GPR operands.  */
      if (!i.suffix)
	{
	  /* We take i.suffix from the last register operand specified,
	     Destination register type is more significant than source
	     register type.  crc32 in SSE4.2 prefers source register
	     type. */
	  unsigned int op = i.tm.mnem_off == MN_crc32 ? 1 : i.operands;

	  while (op--)
	    if (i.tm.operand_types[op].bitfield.instance == InstanceNone
		|| i.tm.operand_types[op].bitfield.instance == Accum)
	      {
		if (i.types[op].bitfield.class != Reg)
		  continue;
		if (i.types[op].bitfield.byte)
		  i.suffix = BYTE_MNEM_SUFFIX;
		else if (i.types[op].bitfield.word)
		  i.suffix = WORD_MNEM_SUFFIX;
		else if (i.types[op].bitfield.dword)
		  i.suffix = LONG_MNEM_SUFFIX;
		else if (i.types[op].bitfield.qword)
		  i.suffix = QWORD_MNEM_SUFFIX;
		else
		  continue;
		break;
	      }

	  /* As an exception, movsx/movzx silently default to a byte source
	     in AT&T mode.  */
	  if (is_movx && i.tm.opcode_modifier.w && !i.suffix && !intel_syntax)
	    i.suffix = BYTE_MNEM_SUFFIX;
	}
      else if (i.suffix == BYTE_MNEM_SUFFIX)
	{
	  if (!check_byte_reg ())
	    return 0;
	}
      else if (i.suffix == LONG_MNEM_SUFFIX)
	{
	  if (!check_long_reg ())
	    return 0;
	}
      else if (i.suffix == QWORD_MNEM_SUFFIX)
	{
	  if (!check_qword_reg ())
	    return 0;
	}
      else if (i.suffix == WORD_MNEM_SUFFIX)
	{
	  if (!check_word_reg ())
	    return 0;
	}
      else if (intel_syntax
	       && i.tm.opcode_modifier.mnemonicsize == IGNORESIZE)
	/* Do nothing if the instruction is going to ignore the prefix.  */
	;
      else
	abort ();

      /* Undo the movsx/movzx change done above.  */
      i.operands = numop;
    }
  else if (i.tm.opcode_modifier.mnemonicsize == DEFAULTSIZE
	   && !i.suffix)
    {
      i.suffix = stackop_size;
      if (stackop_size == LONG_MNEM_SUFFIX)
	{
	  /* stackop_size is set to LONG_MNEM_SUFFIX for the
	     .code16gcc directive to support 16-bit mode with
	     32-bit address.  For IRET without a suffix, generate
	     16-bit IRET (opcode 0xcf) to return from an interrupt
	     handler.  */
	  if (i.tm.base_opcode == 0xcf)
	    {
	      i.suffix = WORD_MNEM_SUFFIX;
	      as_warn (_("generating 16-bit `iret' for .code16gcc directive"));
	    }
	  /* Warn about changed behavior for segment register push/pop.  */
	  else if ((i.tm.base_opcode | 1) == 0x07)
	    as_warn (_("generating 32-bit `%s', unlike earlier gas versions"),
		     insn_name (&i.tm));
	}
    }
  else if (!i.suffix
	   && (i.tm.opcode_modifier.jump == JUMP_ABSOLUTE
	       || i.tm.opcode_modifier.jump == JUMP_BYTE
	       || i.tm.opcode_modifier.jump == JUMP_INTERSEGMENT
	       || (i.tm.opcode_space == SPACE_0F
		   && i.tm.base_opcode == 0x01 /* [ls][gi]dt */
		   && i.tm.extension_opcode <= 3)))
    {
      switch (flag_code)
	{
	case CODE_64BIT:
	  if (!i.tm.opcode_modifier.no_qsuf)
	    {
	      if (i.tm.opcode_modifier.jump == JUMP_BYTE
		  || i.tm.opcode_modifier.no_lsuf)
		i.suffix = QWORD_MNEM_SUFFIX;
	      break;
	    }
	  /* Fall through.  */
	case CODE_32BIT:
	  if (!i.tm.opcode_modifier.no_lsuf)
	    i.suffix = LONG_MNEM_SUFFIX;
	  break;
	case CODE_16BIT:
	  if (!i.tm.opcode_modifier.no_wsuf)
	    i.suffix = WORD_MNEM_SUFFIX;
	  break;
	}
    }

  if (!i.suffix
      && (i.tm.opcode_modifier.mnemonicsize != DEFAULTSIZE
	  /* Also cover lret/retf/iret in 64-bit mode.  */
	  || (flag_code == CODE_64BIT
	      && !i.tm.opcode_modifier.no_lsuf
	      && !i.tm.opcode_modifier.no_qsuf))
      && i.tm.opcode_modifier.mnemonicsize != IGNORESIZE
      /* Explicit sizing prefixes are assumed to disambiguate insns.  */
      && !i.prefix[DATA_PREFIX] && !(i.prefix[REX_PREFIX] & REX_W)
      /* Accept FLDENV et al without suffix.  */
      && (i.tm.opcode_modifier.no_ssuf || i.tm.opcode_modifier.floatmf))
    {
      unsigned int suffixes, evex = 0;

      suffixes = !i.tm.opcode_modifier.no_bsuf;
      if (!i.tm.opcode_modifier.no_wsuf)
	suffixes |= 1 << 1;
      if (!i.tm.opcode_modifier.no_lsuf)
	suffixes |= 1 << 2;
      if (!i.tm.opcode_modifier.no_ssuf)
	suffixes |= 1 << 4;
      if (flag_code == CODE_64BIT && !i.tm.opcode_modifier.no_qsuf)
	suffixes |= 1 << 5;

      /* For [XYZ]MMWORD operands inspect operand sizes.  While generally
	 also suitable for AT&T syntax mode, it was requested that this be
	 restricted to just Intel syntax.  */
      if (intel_syntax && is_any_vex_encoding (&i.tm)
	  && !i.broadcast.type && !i.broadcast.bytes)
	{
	  unsigned int op;

	  for (op = 0; op < i.tm.operands; ++op)
	    {
	      if (is_evex_encoding (&i.tm)
		  && !cpu_arch_flags.bitfield.cpuavx512vl)
		{
		  if (i.tm.operand_types[op].bitfield.ymmword)
		    i.tm.operand_types[op].bitfield.xmmword = 0;
		  if (i.tm.operand_types[op].bitfield.zmmword)
		    i.tm.operand_types[op].bitfield.ymmword = 0;
		  if (!i.tm.opcode_modifier.evex
		      || i.tm.opcode_modifier.evex == EVEXDYN)
		    i.tm.opcode_modifier.evex = EVEX512;
		}

	      if (i.tm.operand_types[op].bitfield.xmmword
		  + i.tm.operand_types[op].bitfield.ymmword
		  + i.tm.operand_types[op].bitfield.zmmword < 2)
		continue;

	      /* Any properly sized operand disambiguates the insn.  */
	      if (i.types[op].bitfield.xmmword
		  || i.types[op].bitfield.ymmword
		  || i.types[op].bitfield.zmmword)
		{
		  suffixes &= ~(7 << 6);
		  evex = 0;
		  break;
		}

	      if ((i.flags[op] & Operand_Mem)
		  && i.tm.operand_types[op].bitfield.unspecified)
		{
		  if (i.tm.operand_types[op].bitfield.xmmword)
		    suffixes |= 1 << 6;
		  if (i.tm.operand_types[op].bitfield.ymmword)
		    suffixes |= 1 << 7;
		  if (i.tm.operand_types[op].bitfield.zmmword)
		    suffixes |= 1 << 8;
		  if (is_evex_encoding (&i.tm))
		    evex = EVEX512;
		}
	    }
	}

      /* Are multiple suffixes / operand sizes allowed?  */
      if (suffixes & (suffixes - 1))
	{
	  if (intel_syntax
	      && (i.tm.opcode_modifier.mnemonicsize != DEFAULTSIZE
		  || operand_check == check_error))
	    {
	      as_bad (_("ambiguous operand size for `%s'"), insn_name (&i.tm));
	      return 0;
	    }
	  if (operand_check == check_error)
	    {
	      as_bad (_("no instruction mnemonic suffix given and "
			"no register operands; can't size `%s'"), insn_name (&i.tm));
	      return 0;
	    }
	  if (operand_check == check_warning)
	    as_warn (_("%s; using default for `%s'"),
		       intel_syntax
		       ? _("ambiguous operand size")
		       : _("no instruction mnemonic suffix given and "
			   "no register operands"),
		       insn_name (&i.tm));

	  if (i.tm.opcode_modifier.floatmf)
	    i.suffix = SHORT_MNEM_SUFFIX;
	  else if (is_movx)
	    /* handled below */;
	  else if (evex)
	    i.tm.opcode_modifier.evex = evex;
	  else if (flag_code == CODE_16BIT)
	    i.suffix = WORD_MNEM_SUFFIX;
	  else if (!i.tm.opcode_modifier.no_lsuf)
	    i.suffix = LONG_MNEM_SUFFIX;
	  else
	    i.suffix = QWORD_MNEM_SUFFIX;
	}
    }

  if (is_movx)
    {
      /* In Intel syntax, movsx/movzx must have a "suffix" (checked above).
	 In AT&T syntax, if there is no suffix (warned about above), the default
	 will be byte extension.  */
      if (i.tm.opcode_modifier.w && i.suffix && i.suffix != BYTE_MNEM_SUFFIX)
	i.tm.base_opcode |= 1;

      /* For further processing, the suffix should represent the destination
	 (register).  This is already the case when one was used with
	 mov[sz][bw]*, but we need to replace it for mov[sz]x, or if there was
	 no suffix to begin with.  */
      if (i.tm.opcode_modifier.w || i.tm.base_opcode == 0x63 || !i.suffix)
	{
	  if (i.types[1].bitfield.word)
	    i.suffix = WORD_MNEM_SUFFIX;
	  else if (i.types[1].bitfield.qword)
	    i.suffix = QWORD_MNEM_SUFFIX;
	  else
	    i.suffix = LONG_MNEM_SUFFIX;

	  i.tm.opcode_modifier.w = 0;
	}
    }

  if (!i.tm.opcode_modifier.modrm && i.reg_operands && i.tm.operands < 3)
    i.short_form = (i.tm.operand_types[0].bitfield.class == Reg)
		   != (i.tm.operand_types[1].bitfield.class == Reg);

  /* Change the opcode based on the operand size given by i.suffix.  */
  switch (i.suffix)
    {
    /* Size floating point instruction.  */
    case LONG_MNEM_SUFFIX:
      if (i.tm.opcode_modifier.floatmf)
	{
	  i.tm.base_opcode ^= 4;
	  break;
	}
    /* fall through */
    case WORD_MNEM_SUFFIX:
    case QWORD_MNEM_SUFFIX:
      /* It's not a byte, select word/dword operation.  */
      if (i.tm.opcode_modifier.w)
	{
	  if (i.short_form)
	    i.tm.base_opcode |= 8;
	  else
	    i.tm.base_opcode |= 1;
	}
    /* fall through */
    case SHORT_MNEM_SUFFIX:
      /* Now select between word & dword operations via the operand
	 size prefix, except for instructions that will ignore this
	 prefix anyway.  */
      if (i.suffix != QWORD_MNEM_SUFFIX
	  && i.tm.opcode_modifier.mnemonicsize != IGNORESIZE
	  && !i.tm.opcode_modifier.floatmf
	  && !is_any_vex_encoding (&i.tm)
	  && ((i.suffix == LONG_MNEM_SUFFIX) == (flag_code == CODE_16BIT)
	      || (flag_code == CODE_64BIT
		  && i.tm.opcode_modifier.jump == JUMP_BYTE)))
	{
	  unsigned int prefix = DATA_PREFIX_OPCODE;

	  if (i.tm.opcode_modifier.jump == JUMP_BYTE) /* jcxz, loop */
	    prefix = ADDR_PREFIX_OPCODE;

	  if (!add_prefix (prefix))
	    return 0;
	}

      /* Set mode64 for an operand.  */
      if (i.suffix == QWORD_MNEM_SUFFIX
	  && flag_code == CODE_64BIT
	  && !i.tm.opcode_modifier.norex64
	  && !i.tm.opcode_modifier.vexw
	  /* Special case for xchg %rax,%rax.  It is NOP and doesn't
	     need rex64. */
	  && ! (i.operands == 2
		&& i.tm.base_opcode == 0x90
		&& i.tm.opcode_space == SPACE_BASE
		&& i.types[0].bitfield.instance == Accum
		&& i.types[0].bitfield.qword
		&& i.types[1].bitfield.instance == Accum))
	i.rex |= REX_W;

      break;

    case 0:
      /* Select word/dword/qword operation with explicit data sizing prefix
	 when there are no suitable register operands.  */
      if (i.tm.opcode_modifier.w
	  && (i.prefix[DATA_PREFIX] || (i.prefix[REX_PREFIX] & REX_W))
	  && (!i.reg_operands
	      || (i.reg_operands == 1
		      /* ShiftCount */
		  && (i.tm.operand_types[0].bitfield.instance == RegC
		      /* InOutPortReg */
		      || i.tm.operand_types[0].bitfield.instance == RegD
		      || i.tm.operand_types[1].bitfield.instance == RegD
		      || i.tm.mnem_off == MN_crc32))))
	i.tm.base_opcode |= 1;
      break;
    }

  if (i.tm.opcode_modifier.operandconstraint == ADDR_PREFIX_OP_REG)
    {
      gas_assert (!i.suffix);
      gas_assert (i.reg_operands);

      if (i.tm.operand_types[0].bitfield.instance == Accum
	  || i.operands == 1)
	{
	  /* The address size override prefix changes the size of the
	     first operand.  */
	  if (flag_code == CODE_64BIT
	      && i.op[0].regs->reg_type.bitfield.word)
	    {
	      as_bad (_("16-bit addressing unavailable for `%s'"),
		      insn_name (&i.tm));
	      return 0;
	    }

	  if ((flag_code == CODE_32BIT
	       ? i.op[0].regs->reg_type.bitfield.word
	       : i.op[0].regs->reg_type.bitfield.dword)
	      && !add_prefix (ADDR_PREFIX_OPCODE))
	    return 0;
	}
      else
	{
	  /* Check invalid register operand when the address size override
	     prefix changes the size of register operands.  */
	  unsigned int op;
	  enum { need_word, need_dword, need_qword } need;

	  /* Check the register operand for the address size prefix if
	     the memory operand has no real registers, like symbol, DISP
	     or bogus (x32-only) symbol(%rip) when symbol(%eip) is meant.  */
	  if (i.mem_operands == 1
	      && i.reg_operands == 1
	      && i.operands == 2
	      && i.types[1].bitfield.class == Reg
	      && (flag_code == CODE_32BIT
		  ? i.op[1].regs->reg_type.bitfield.word
		  : i.op[1].regs->reg_type.bitfield.dword)
	      && ((i.base_reg == NULL && i.index_reg == NULL)
#if defined (OBJ_MAYBE_ELF) || defined (OBJ_ELF)
		  || (x86_elf_abi == X86_64_X32_ABI
		      && i.base_reg
		      && i.base_reg->reg_num == RegIP
		      && i.base_reg->reg_type.bitfield.qword))
#else
		  || 0)
#endif
	      && !add_prefix (ADDR_PREFIX_OPCODE))
	    return 0;

	  if (flag_code == CODE_32BIT)
	    need = i.prefix[ADDR_PREFIX] ? need_word : need_dword;
	  else if (i.prefix[ADDR_PREFIX])
	    need = need_dword;
	  else
	    need = flag_code == CODE_64BIT ? need_qword : need_word;

	  for (op = 0; op < i.operands; op++)
	    {
	      if (i.types[op].bitfield.class != Reg)
		continue;

	      switch (need)
		{
		case need_word:
		  if (i.op[op].regs->reg_type.bitfield.word)
		    continue;
		  break;
		case need_dword:
		  if (i.op[op].regs->reg_type.bitfield.dword)
		    continue;
		  break;
		case need_qword:
		  if (i.op[op].regs->reg_type.bitfield.qword)
		    continue;
		  break;
		}

	      as_bad (_("invalid register operand size for `%s'"),
		      insn_name (&i.tm));
	      return 0;
	    }
	}
    }

  return 1;
}

static int
check_byte_reg (void)
{
  int op;

  for (op = i.operands; --op >= 0;)
    {
      /* Skip non-register operands. */
      if (i.types[op].bitfield.class != Reg)
	continue;

      /* If this is an eight bit register, it's OK.  If it's the 16 or
	 32 bit version of an eight bit register, we will just use the
	 low portion, and that's OK too.  */
      if (i.types[op].bitfield.byte)
	continue;

      /* I/O port address operands are OK too.  */
      if (i.tm.operand_types[op].bitfield.instance == RegD
	  && i.tm.operand_types[op].bitfield.word)
	continue;

      /* crc32 only wants its source operand checked here.  */
      if (i.tm.mnem_off == MN_crc32 && op != 0)
	continue;

      /* Any other register is bad.  */
      as_bad (_("`%s%s' not allowed with `%s%c'"),
	      register_prefix, i.op[op].regs->reg_name,
	      insn_name (&i.tm), i.suffix);
      return 0;
    }
  return 1;
}

static int
check_long_reg (void)
{
  int op;

  for (op = i.operands; --op >= 0;)
    /* Skip non-register operands. */
    if (i.types[op].bitfield.class != Reg)
      continue;
    /* Reject eight bit registers, except where the template requires
       them. (eg. movzb)  */
    else if (i.types[op].bitfield.byte
	     && (i.tm.operand_types[op].bitfield.class == Reg
		 || i.tm.operand_types[op].bitfield.instance == Accum)
	     && (i.tm.operand_types[op].bitfield.word
		 || i.tm.operand_types[op].bitfield.dword))
      {
	as_bad (_("`%s%s' not allowed with `%s%c'"),
		register_prefix,
		i.op[op].regs->reg_name,
		insn_name (&i.tm),
		i.suffix);
	return 0;
      }
    /* Error if the e prefix on a general reg is missing.  */
    else if (i.types[op].bitfield.word
	     && (i.tm.operand_types[op].bitfield.class == Reg
		 || i.tm.operand_types[op].bitfield.instance == Accum)
	     && i.tm.operand_types[op].bitfield.dword)
      {
	as_bad (_("incorrect register `%s%s' used with `%c' suffix"),
		register_prefix, i.op[op].regs->reg_name,
		i.suffix);
	return 0;
      }
    /* Warn if the r prefix on a general reg is present.  */
    else if (i.types[op].bitfield.qword
	     && (i.tm.operand_types[op].bitfield.class == Reg
		 || i.tm.operand_types[op].bitfield.instance == Accum)
	     && i.tm.operand_types[op].bitfield.dword)
      {
	as_bad (_("incorrect register `%s%s' used with `%c' suffix"),
		register_prefix, i.op[op].regs->reg_name, i.suffix);
	return 0;
      }
  return 1;
}

static int
check_qword_reg (void)
{
  int op;

  for (op = i.operands; --op >= 0; )
    /* Skip non-register operands. */
    if (i.types[op].bitfield.class != Reg)
      continue;
    /* Reject eight bit registers, except where the template requires
       them. (eg. movzb)  */
    else if (i.types[op].bitfield.byte
	     && (i.tm.operand_types[op].bitfield.class == Reg
		 || i.tm.operand_types[op].bitfield.instance == Accum)
	     && (i.tm.operand_types[op].bitfield.word
		 || i.tm.operand_types[op].bitfield.dword))
      {
	as_bad (_("`%s%s' not allowed with `%s%c'"),
		register_prefix,
		i.op[op].regs->reg_name,
		insn_name (&i.tm),
		i.suffix);
	return 0;
      }
    /* Warn if the r prefix on a general reg is missing.  */
    else if ((i.types[op].bitfield.word
	      || i.types[op].bitfield.dword)
	     && (i.tm.operand_types[op].bitfield.class == Reg
		 || i.tm.operand_types[op].bitfield.instance == Accum)
	     && i.tm.operand_types[op].bitfield.qword)
      {
	/* Prohibit these changes in the 64bit mode, since the
	   lowering is more complicated.  */
	as_bad (_("incorrect register `%s%s' used with `%c' suffix"),
		register_prefix, i.op[op].regs->reg_name, i.suffix);
	return 0;
      }
  return 1;
}

static int
check_word_reg (void)
{
  int op;
  for (op = i.operands; --op >= 0;)
    /* Skip non-register operands. */
    if (i.types[op].bitfield.class != Reg)
      continue;
    /* Reject eight bit registers, except where the template requires
       them. (eg. movzb)  */
    else if (i.types[op].bitfield.byte
	     && (i.tm.operand_types[op].bitfield.class == Reg
		 || i.tm.operand_types[op].bitfield.instance == Accum)
	     && (i.tm.operand_types[op].bitfield.word
		 || i.tm.operand_types[op].bitfield.dword))
      {
	as_bad (_("`%s%s' not allowed with `%s%c'"),
		register_prefix,
		i.op[op].regs->reg_name,
		insn_name (&i.tm),
		i.suffix);
	return 0;
      }
    /* Error if the e or r prefix on a general reg is present.  */
    else if ((i.types[op].bitfield.dword
		 || i.types[op].bitfield.qword)
	     && (i.tm.operand_types[op].bitfield.class == Reg
		 || i.tm.operand_types[op].bitfield.instance == Accum)
	     && i.tm.operand_types[op].bitfield.word)
      {
	as_bad (_("incorrect register `%s%s' used with `%c' suffix"),
		register_prefix, i.op[op].regs->reg_name,
		i.suffix);
	return 0;
      }
  return 1;
}

static int
update_imm (unsigned int j)
{
  i386_operand_type overlap = i.types[j];

  if (i.tm.operand_types[j].bitfield.imm8
      && i.tm.operand_types[j].bitfield.imm8s
      && overlap.bitfield.imm8 && overlap.bitfield.imm8s)
    {
      /* This combination is used on 8-bit immediates where e.g. $~0 is
	 desirable to permit.  We're past operand type matching, so simply
	 put things back in the shape they were before introducing the
	 distinction between Imm8, Imm8S, and Imm8|Imm8S.  */
      overlap.bitfield.imm8s = 0;
    }

  if (overlap.bitfield.imm8
      + overlap.bitfield.imm8s
      + overlap.bitfield.imm16
      + overlap.bitfield.imm32
      + overlap.bitfield.imm32s
      + overlap.bitfield.imm64 > 1)
    {
      static const i386_operand_type imm16 = { .bitfield = { .imm16 = 1 } };
      static const i386_operand_type imm32 = { .bitfield = { .imm32 = 1 } };
      static const i386_operand_type imm32s = { .bitfield = { .imm32s = 1 } };
      static const i386_operand_type imm16_32 = { .bitfield =
	{ .imm16 = 1, .imm32 = 1 }
      };
      static const i386_operand_type imm16_32s =  { .bitfield =
	{ .imm16 = 1, .imm32s = 1 }
      };
      static const i386_operand_type imm16_32_32s = { .bitfield =
	{ .imm16 = 1, .imm32 = 1, .imm32s = 1 }
      };

      if (i.suffix)
	{
	  i386_operand_type temp;

	  operand_type_set (&temp, 0);
	  if (i.suffix == BYTE_MNEM_SUFFIX)
	    {
	      temp.bitfield.imm8 = overlap.bitfield.imm8;
	      temp.bitfield.imm8s = overlap.bitfield.imm8s;
	    }
	  else if (i.suffix == WORD_MNEM_SUFFIX)
	    temp.bitfield.imm16 = overlap.bitfield.imm16;
	  else if (i.suffix == QWORD_MNEM_SUFFIX)
	    {
	      temp.bitfield.imm64 = overlap.bitfield.imm64;
	      temp.bitfield.imm32s = overlap.bitfield.imm32s;
	    }
	  else
	    temp.bitfield.imm32 = overlap.bitfield.imm32;
	  overlap = temp;
	}
      else if (operand_type_equal (&overlap, &imm16_32_32s)
	       || operand_type_equal (&overlap, &imm16_32)
	       || operand_type_equal (&overlap, &imm16_32s))
	{
	  if ((flag_code == CODE_16BIT) ^ (i.prefix[DATA_PREFIX] != 0))
	    overlap = imm16;
	  else
	    overlap = imm32s;
	}
      else if (i.prefix[REX_PREFIX] & REX_W)
	overlap = operand_type_and (overlap, imm32s);
      else if (i.prefix[DATA_PREFIX])
	overlap = operand_type_and (overlap,
				    flag_code != CODE_16BIT ? imm16 : imm32);
      if (overlap.bitfield.imm8
	  + overlap.bitfield.imm8s
	  + overlap.bitfield.imm16
	  + overlap.bitfield.imm32
	  + overlap.bitfield.imm32s
	  + overlap.bitfield.imm64 != 1)
	{
	  as_bad (_("no instruction mnemonic suffix given; "
		    "can't determine immediate size"));
	  return 0;
	}
    }
  i.types[j] = overlap;

  return 1;
}

static int
finalize_imm (void)
{
  unsigned int j, n;

  /* Update the first 2 immediate operands.  */
  n = i.operands > 2 ? 2 : i.operands;
  if (n)
    {
      for (j = 0; j < n; j++)
	if (update_imm (j) == 0)
	  return 0;

      /* The 3rd operand can't be immediate operand.  */
      gas_assert (operand_type_check (i.types[2], imm) == 0);
    }

  return 1;
}

static INLINE void set_rex_vrex (const reg_entry *r, unsigned int rex_bit,
				 bool do_sse2avx)
{
  if (r->reg_flags & RegRex)
    {
      if (i.rex & rex_bit)
	as_bad (_("same type of prefix used twice"));
      i.rex |= rex_bit;
    }
  else if (do_sse2avx && (i.rex & rex_bit) && i.vex.register_specifier)
    {
      gas_assert (i.vex.register_specifier == r);
      i.vex.register_specifier += 8;
    }

  if (r->reg_flags & RegVRex)
    i.vrex |= rex_bit;
}

static int
process_operands (void)
{
  /* Default segment register this instruction will use for memory
     accesses.  0 means unknown.  This is only for optimizing out
     unnecessary segment overrides.  */
  const reg_entry *default_seg = NULL;

  /* We only need to check those implicit registers for instructions
     with 3 operands or less.  */
  if (i.operands <= 3)
    for (unsigned int j = 0; j < i.operands; j++)
      if (i.types[j].bitfield.instance != InstanceNone)
	i.reg_operands--;

  if (i.tm.opcode_modifier.sse2avx)
    {
      /* Legacy encoded insns allow explicit REX prefixes, so these prefixes
	 need converting.  */
      i.rex |= i.prefix[REX_PREFIX] & (REX_W | REX_R | REX_X | REX_B);
      i.prefix[REX_PREFIX] = 0;
      i.rex_encoding = 0;
    }
  /* ImmExt should be processed after SSE2AVX.  */
  else if (i.tm.opcode_modifier.immext)
    process_immext ();

  /* TILEZERO is unusual in that it has a single operand encoded in ModR/M.reg,
     not ModR/M.rm.  To avoid special casing this in build_modrm_byte(), fake a
     new destination operand here, while converting the source one to register
     number 0.  */
  if (i.tm.mnem_off == MN_tilezero)
    {
      i.op[1].regs = i.op[0].regs;
      i.op[0].regs -= i.op[0].regs->reg_num;
      i.types[1] = i.types[0];
      i.tm.operand_types[1] = i.tm.operand_types[0];
      i.flags[1] = i.flags[0];
      i.operands++;
      i.reg_operands++;
      i.tm.operands++;
    }

  if (i.tm.opcode_modifier.sse2avx && i.tm.opcode_modifier.vexvvvv)
    {
      static const i386_operand_type regxmm = {
        .bitfield = { .class = RegSIMD, .xmmword = 1 }
      };
      unsigned int dupl = i.operands;
      unsigned int dest = dupl - 1;
      unsigned int j;

      /* The destination must be an xmm register.  */
      gas_assert (i.reg_operands
		  && MAX_OPERANDS > dupl
		  && operand_type_equal (&i.types[dest], &regxmm));

      if (i.tm.operand_types[0].bitfield.instance == Accum
	  && i.tm.operand_types[0].bitfield.xmmword)
	{
	  /* Keep xmm0 for instructions with VEX prefix and 3
	     sources.  */
	  i.tm.operand_types[0].bitfield.instance = InstanceNone;
	  i.tm.operand_types[0].bitfield.class = RegSIMD;
	  i.reg_operands++;
	  goto duplicate;
	}

      if (i.tm.opcode_modifier.operandconstraint == IMPLICIT_1ST_XMM0)
	{
	  gas_assert ((MAX_OPERANDS - 1) > dupl);

	  /* Add the implicit xmm0 for instructions with VEX prefix
	     and 3 sources.  */
	  for (j = i.operands; j > 0; j--)
	    {
	      i.op[j] = i.op[j - 1];
	      i.types[j] = i.types[j - 1];
	      i.tm.operand_types[j] = i.tm.operand_types[j - 1];
	      i.flags[j] = i.flags[j - 1];
	    }
	  i.op[0].regs
	    = (const reg_entry *) str_hash_find (reg_hash, "xmm0");
	  i.types[0] = regxmm;
	  i.tm.operand_types[0] = regxmm;

	  i.operands += 2;
	  i.reg_operands += 2;
	  i.tm.operands += 2;

	  dupl++;
	  dest++;
	  i.op[dupl] = i.op[dest];
	  i.types[dupl] = i.types[dest];
	  i.tm.operand_types[dupl] = i.tm.operand_types[dest];
	  i.flags[dupl] = i.flags[dest];
	}
      else
	{
	duplicate:
	  i.operands++;
	  i.reg_operands++;
	  i.tm.operands++;

	  i.op[dupl] = i.op[dest];
	  i.types[dupl] = i.types[dest];
	  i.tm.operand_types[dupl] = i.tm.operand_types[dest];
	  i.flags[dupl] = i.flags[dest];
	}

       if (i.tm.opcode_modifier.immext)
	 process_immext ();
    }
  else if (i.tm.operand_types[0].bitfield.instance == Accum
	   && i.tm.opcode_modifier.modrm)
    {
      unsigned int j;

      for (j = 1; j < i.operands; j++)
	{
	  i.op[j - 1] = i.op[j];
	  i.types[j - 1] = i.types[j];

	  /* We need to adjust fields in i.tm since they are used by
	     build_modrm_byte.  */
	  i.tm.operand_types [j - 1] = i.tm.operand_types [j];

	  i.flags[j - 1] = i.flags[j];
	}

      /* No adjustment to i.reg_operands: This was already done at the top
	 of the function.  */
      i.operands--;
      i.tm.operands--;
    }
  else if (i.tm.opcode_modifier.operandconstraint == IMPLICIT_QUAD_GROUP)
    {
      unsigned int regnum, first_reg_in_group, last_reg_in_group;

      /* The second operand must be {x,y,z}mmN, where N is a multiple of 4. */
      gas_assert (i.operands >= 2 && i.types[1].bitfield.class == RegSIMD);
      regnum = register_number (i.op[1].regs);
      first_reg_in_group = regnum & ~3;
      last_reg_in_group = first_reg_in_group + 3;
      if (regnum != first_reg_in_group)
	as_warn (_("source register `%s%s' implicitly denotes"
		   " `%s%.3s%u' to `%s%.3s%u' source group in `%s'"),
		 register_prefix, i.op[1].regs->reg_name,
		 register_prefix, i.op[1].regs->reg_name, first_reg_in_group,
		 register_prefix, i.op[1].regs->reg_name, last_reg_in_group,
		 insn_name (&i.tm));
    }
  else if (i.tm.opcode_modifier.operandconstraint == REG_KLUDGE)
    {
      /* The imul $imm, %reg instruction is converted into
	 imul $imm, %reg, %reg, and the clr %reg instruction
	 is converted into xor %reg, %reg.  */

      unsigned int first_reg_op;

      if (operand_type_check (i.types[0], reg))
	first_reg_op = 0;
      else
	first_reg_op = 1;
      /* Pretend we saw the extra register operand.  */
      gas_assert (i.reg_operands == 1
		  && i.op[first_reg_op + 1].regs == 0);
      i.op[first_reg_op + 1].regs = i.op[first_reg_op].regs;
      i.types[first_reg_op + 1] = i.types[first_reg_op];
      i.operands++;
      i.reg_operands++;
    }

  if (i.tm.opcode_modifier.modrm)
    {
      /* The opcode is completed (modulo i.tm.extension_opcode which
	 must be put into the modrm byte).  Now, we make the modrm and
	 index base bytes based on all the info we've collected.  */

      default_seg = build_modrm_byte ();

      if (!quiet_warnings && i.tm.opcode_modifier.operandconstraint == UGH)
	{
	  /* Warn about some common errors, but press on regardless.  */
	  if (i.operands == 2)
	    {
	      /* Reversed arguments on faddp or fmulp.  */
	      as_warn (_("translating to `%s %s%s,%s%s'"), insn_name (&i.tm),
		       register_prefix, i.op[!intel_syntax].regs->reg_name,
		       register_prefix, i.op[intel_syntax].regs->reg_name);
	    }
	  else if (i.tm.opcode_modifier.mnemonicsize == IGNORESIZE)
	    {
	      /* Extraneous `l' suffix on fp insn.  */
	      as_warn (_("translating to `%s %s%s'"), insn_name (&i.tm),
		       register_prefix, i.op[0].regs->reg_name);
	    }
	}
    }
  else if (i.types[0].bitfield.class == SReg && !dot_insn ())
    {
      if (flag_code != CODE_64BIT
	  ? i.tm.base_opcode == POP_SEG_SHORT
	    && i.op[0].regs->reg_num == 1
	  : (i.tm.base_opcode | 1) == (POP_SEG386_SHORT & 0xff)
	    && i.op[0].regs->reg_num < 4)
	{
	  as_bad (_("you can't `%s %s%s'"),
		  insn_name (&i.tm), register_prefix, i.op[0].regs->reg_name);
	  return 0;
	}
      if (i.op[0].regs->reg_num > 3
	  && i.tm.opcode_space == SPACE_BASE )
	{
	  i.tm.base_opcode ^= (POP_SEG_SHORT ^ POP_SEG386_SHORT) & 0xff;
	  i.tm.opcode_space = SPACE_0F;
	}
      i.tm.base_opcode |= (i.op[0].regs->reg_num << 3);
    }
  else if (i.tm.opcode_space == SPACE_BASE
	   && (i.tm.base_opcode & ~3) == MOV_AX_DISP32)
    {
      default_seg = reg_ds;
    }
  else if (i.tm.opcode_modifier.isstring)
    {
      /* For the string instructions that allow a segment override
	 on one of their operands, the default segment is ds.  */
      default_seg = reg_ds;
    }
  else if (i.short_form)
    {
      /* The register operand is in the 1st or 2nd non-immediate operand.  */
      const reg_entry *r = i.op[i.imm_operands].regs;

      if (!dot_insn ()
	  && r->reg_type.bitfield.instance == Accum
	  && i.op[i.imm_operands + 1].regs)
	r = i.op[i.imm_operands + 1].regs;
      /* Register goes in low 3 bits of opcode.  */
      i.tm.base_opcode |= r->reg_num;
      set_rex_vrex (r, REX_B, false);

      if (dot_insn () && i.reg_operands == 2)
	{
	  gas_assert (is_any_vex_encoding (&i.tm)
		      || i.vec_encoding != vex_encoding_default);
	  i.vex.register_specifier = i.op[i.operands - 1].regs;
	}
    }
  else if (i.reg_operands == 1
	   && !i.flags[i.operands - 1]
	   && i.tm.operand_types[i.operands - 1].bitfield.instance
	      == InstanceNone)
    {
      gas_assert (is_any_vex_encoding (&i.tm)
		  || i.vec_encoding != vex_encoding_default);
      i.vex.register_specifier = i.op[i.operands - 1].regs;
    }

  if ((i.seg[0] || i.prefix[SEG_PREFIX])
      && i.tm.mnem_off == MN_lea)
    {
      if (!quiet_warnings)
	as_warn (_("segment override on `%s' is ineffectual"), insn_name (&i.tm));
      if (optimize && !i.no_optimize)
	{
	  i.seg[0] = NULL;
	  i.prefix[SEG_PREFIX] = 0;
	}
    }

  /* If a segment was explicitly specified, and the specified segment
     is neither the default nor the one already recorded from a prefix,
     use an opcode prefix to select it.  If we never figured out what
     the default segment is, then default_seg will be zero at this
     point, and the specified segment prefix will always be used.  */
  if (i.seg[0]
      && i.seg[0] != default_seg
      && i386_seg_prefixes[i.seg[0]->reg_num] != i.prefix[SEG_PREFIX])
    {
      if (!add_prefix (i386_seg_prefixes[i.seg[0]->reg_num]))
	return 0;
    }
  return 1;
}

static const reg_entry *
build_modrm_byte (void)
{
  const reg_entry *default_seg = NULL;
  unsigned int source = i.imm_operands - i.tm.opcode_modifier.immext
			/* Compensate for kludge in md_assemble().  */
			+ i.tm.operand_types[0].bitfield.imm1;
  unsigned int dest = i.operands - 1 - i.tm.opcode_modifier.immext;
  unsigned int v, op, reg_slot = ~0;

  /* Accumulator (in particular %st), shift count (%cl), and alike need
     to be skipped just like immediate operands do.  */
  if (i.tm.operand_types[source].bitfield.instance)
    ++source;
  while (i.tm.operand_types[dest].bitfield.instance)
    --dest;

  for (op = source; op < i.operands; ++op)
    if (i.tm.operand_types[op].bitfield.baseindex)
      break;

  if (i.reg_operands + i.mem_operands + (i.tm.extension_opcode != None) == 4)
    {
      expressionS *exp;

      /* There are 2 kinds of instructions:
	 1. 5 operands: 4 register operands or 3 register operands
	 plus 1 memory operand plus one Imm4 operand, VexXDS, and
	 VexW0 or VexW1.  The destination must be either XMM, YMM or
	 ZMM register.
	 2. 4 operands: 4 register operands or 3 register operands
	 plus 1 memory operand, with VexXDS.
	 3. Other equivalent combinations when coming from s_insn().  */
      gas_assert (i.tm.opcode_modifier.vexvvvv
		  && i.tm.opcode_modifier.vexw);
      gas_assert (dot_insn ()
		  || i.tm.operand_types[dest].bitfield.class == RegSIMD);

      /* Of the first two non-immediate operands the one with the template
	 not allowing for a memory one is encoded in the immediate operand.  */
      if (source == op)
	reg_slot = source + 1;
      else
	reg_slot = source++;

      if (!dot_insn ())
	{
	  gas_assert (i.tm.operand_types[reg_slot].bitfield.class == RegSIMD);
	  gas_assert (!(i.op[reg_slot].regs->reg_flags & RegVRex));
	}
      else
	gas_assert (i.tm.operand_types[reg_slot].bitfield.class != ClassNone);

      if (i.imm_operands == 0)
	{
	  /* When there is no immediate operand, generate an 8bit
	     immediate operand to encode the first operand.  */
	  exp = &im_expressions[i.imm_operands++];
	  i.op[i.operands].imms = exp;
	  i.types[i.operands].bitfield.imm8 = 1;
	  i.operands++;

	  exp->X_op = O_constant;
	}
      else
	{
	  gas_assert (i.imm_operands == 1);
	  gas_assert (fits_in_imm4 (i.op[0].imms->X_add_number));
	  gas_assert (!i.tm.opcode_modifier.immext);

	  /* Turn on Imm8 again so that output_imm will generate it.  */
	  i.types[0].bitfield.imm8 = 1;

	  exp = i.op[0].imms;
	}
      exp->X_add_number |= register_number (i.op[reg_slot].regs)
			   << (3 + !(is_evex_encoding (&i.tm)
				     || i.vec_encoding == vex_encoding_evex));
    }

  for (v = source + 1; v < dest; ++v)
    if (v != reg_slot)
      break;
  if (v >= dest)
    v = ~0;
  if (i.tm.extension_opcode != None)
    {
      if (dest != source)
	v = dest;
      dest = ~0;
    }
  gas_assert (source < dest);
  if (i.tm.opcode_modifier.operandconstraint == SWAP_SOURCES
      && source != op)
    {
      unsigned int tmp = source;

      source = v;
      v = tmp;
    }

  if (v < MAX_OPERANDS)
    {
      gas_assert (i.tm.opcode_modifier.vexvvvv);
      i.vex.register_specifier = i.op[v].regs;
    }

  if (op < i.operands)
    {
      if (i.mem_operands)
	{
	  unsigned int fake_zero_displacement = 0;

	  gas_assert (i.flags[op] & Operand_Mem);

	  if (i.tm.opcode_modifier.sib)
	    {
	      /* The index register of VSIB shouldn't be RegIZ.  */
	      if (i.tm.opcode_modifier.sib != SIBMEM
		  && i.index_reg->reg_num == RegIZ)
		abort ();

	      i.rm.regmem = ESCAPE_TO_TWO_BYTE_ADDRESSING;
	      if (!i.base_reg)
		{
		  i.sib.base = NO_BASE_REGISTER;
		  i.sib.scale = i.log2_scale_factor;
		  i.types[op] = operand_type_and_not (i.types[op], anydisp);
		  i.types[op].bitfield.disp32 = 1;
		}

	      /* Since the mandatory SIB always has index register, so
		 the code logic remains unchanged. The non-mandatory SIB
		 without index register is allowed and will be handled
		 later.  */
	      if (i.index_reg)
		{
		  if (i.index_reg->reg_num == RegIZ)
		    i.sib.index = NO_INDEX_REGISTER;
		  else
		    i.sib.index = i.index_reg->reg_num;
		  set_rex_vrex (i.index_reg, REX_X, false);
		}
	    }

	  default_seg = reg_ds;

	  if (i.base_reg == 0)
	    {
	      i.rm.mode = 0;
	      if (!i.disp_operands)
		fake_zero_displacement = 1;
	      if (i.index_reg == 0)
		{
		  /* Both check for VSIB and mandatory non-vector SIB. */
		  gas_assert (!i.tm.opcode_modifier.sib
			      || i.tm.opcode_modifier.sib == SIBMEM);
		  /* Operand is just <disp>  */
		  i.types[op] = operand_type_and_not (i.types[op], anydisp);
		  if (flag_code == CODE_64BIT)
		    {
		      /* 64bit mode overwrites the 32bit absolute
			 addressing by RIP relative addressing and
			 absolute addressing is encoded by one of the
			 redundant SIB forms.  */
		      i.rm.regmem = ESCAPE_TO_TWO_BYTE_ADDRESSING;
		      i.sib.base = NO_BASE_REGISTER;
		      i.sib.index = NO_INDEX_REGISTER;
		      i.types[op].bitfield.disp32 = 1;
		    }
		  else if ((flag_code == CODE_16BIT)
			   ^ (i.prefix[ADDR_PREFIX] != 0))
		    {
		      i.rm.regmem = NO_BASE_REGISTER_16;
		      i.types[op].bitfield.disp16 = 1;
		    }
		  else
		    {
		      i.rm.regmem = NO_BASE_REGISTER;
		      i.types[op].bitfield.disp32 = 1;
		    }
		}
	      else if (!i.tm.opcode_modifier.sib)
		{
		  /* !i.base_reg && i.index_reg  */
		  if (i.index_reg->reg_num == RegIZ)
		    i.sib.index = NO_INDEX_REGISTER;
		  else
		    i.sib.index = i.index_reg->reg_num;
		  i.sib.base = NO_BASE_REGISTER;
		  i.sib.scale = i.log2_scale_factor;
		  i.rm.regmem = ESCAPE_TO_TWO_BYTE_ADDRESSING;
		  i.types[op] = operand_type_and_not (i.types[op], anydisp);
		  i.types[op].bitfield.disp32 = 1;
		  if ((i.index_reg->reg_flags & RegRex) != 0)
		    i.rex |= REX_X;
		}
	    }
	  /* RIP addressing for 64bit mode.  */
	  else if (i.base_reg->reg_num == RegIP)
	    {
	      gas_assert (!i.tm.opcode_modifier.sib);
	      i.rm.regmem = NO_BASE_REGISTER;
	      i.types[op].bitfield.disp8 = 0;
	      i.types[op].bitfield.disp16 = 0;
	      i.types[op].bitfield.disp32 = 1;
	      i.types[op].bitfield.disp64 = 0;
	      i.flags[op] |= Operand_PCrel;
	      if (! i.disp_operands)
		fake_zero_displacement = 1;
	    }
	  else if (i.base_reg->reg_type.bitfield.word)
	    {
	      gas_assert (!i.tm.opcode_modifier.sib);
	      switch (i.base_reg->reg_num)
		{
		case 3: /* (%bx)  */
		  if (i.index_reg == 0)
		    i.rm.regmem = 7;
		  else /* (%bx,%si) -> 0, or (%bx,%di) -> 1  */
		    i.rm.regmem = i.index_reg->reg_num - 6;
		  break;
		case 5: /* (%bp)  */
		  default_seg = reg_ss;
		  if (i.index_reg == 0)
		    {
		      i.rm.regmem = 6;
		      if (operand_type_check (i.types[op], disp) == 0)
			{
			  /* fake (%bp) into 0(%bp)  */
			  if (i.disp_encoding == disp_encoding_16bit)
			    i.types[op].bitfield.disp16 = 1;
			  else
			    i.types[op].bitfield.disp8 = 1;
			  fake_zero_displacement = 1;
			}
		    }
		  else /* (%bp,%si) -> 2, or (%bp,%di) -> 3  */
		    i.rm.regmem = i.index_reg->reg_num - 6 + 2;
		  break;
		default: /* (%si) -> 4 or (%di) -> 5  */
		  i.rm.regmem = i.base_reg->reg_num - 6 + 4;
		}
	      if (!fake_zero_displacement
		  && !i.disp_operands
		  && i.disp_encoding)
		{
		  fake_zero_displacement = 1;
		  if (i.disp_encoding == disp_encoding_8bit)
		    i.types[op].bitfield.disp8 = 1;
		  else
		    i.types[op].bitfield.disp16 = 1;
		}
	      i.rm.mode = mode_from_disp_size (i.types[op]);
	    }
	  else /* i.base_reg and 32/64 bit mode  */
	    {
	      if (operand_type_check (i.types[op], disp))
		{
		  i.types[op].bitfield.disp16 = 0;
		  i.types[op].bitfield.disp64 = 0;
		  i.types[op].bitfield.disp32 = 1;
		}

	      if (!i.tm.opcode_modifier.sib)
		i.rm.regmem = i.base_reg->reg_num;
	      if ((i.base_reg->reg_flags & RegRex) != 0)
		i.rex |= REX_B;
	      i.sib.base = i.base_reg->reg_num;
	      /* x86-64 ignores REX prefix bit here to avoid decoder
		 complications.  */
	      if (!(i.base_reg->reg_flags & RegRex)
		  && (i.base_reg->reg_num == EBP_REG_NUM
		   || i.base_reg->reg_num == ESP_REG_NUM))
		  default_seg = reg_ss;
	      if (i.base_reg->reg_num == 5 && i.disp_operands == 0)
		{
		  fake_zero_displacement = 1;
		  if (i.disp_encoding == disp_encoding_32bit)
		    i.types[op].bitfield.disp32 = 1;
		  else
		    i.types[op].bitfield.disp8 = 1;
		}
	      i.sib.scale = i.log2_scale_factor;
	      if (i.index_reg == 0)
		{
		  /* Only check for VSIB. */
		  gas_assert (i.tm.opcode_modifier.sib != VECSIB128
			      && i.tm.opcode_modifier.sib != VECSIB256
			      && i.tm.opcode_modifier.sib != VECSIB512);

		  /* <disp>(%esp) becomes two byte modrm with no index
		     register.  We've already stored the code for esp
		     in i.rm.regmem ie. ESCAPE_TO_TWO_BYTE_ADDRESSING.
		     Any base register besides %esp will not use the
		     extra modrm byte.  */
		  i.sib.index = NO_INDEX_REGISTER;
		}
	      else if (!i.tm.opcode_modifier.sib)
		{
		  if (i.index_reg->reg_num == RegIZ)
		    i.sib.index = NO_INDEX_REGISTER;
		  else
		    i.sib.index = i.index_reg->reg_num;
		  i.rm.regmem = ESCAPE_TO_TWO_BYTE_ADDRESSING;
		  if ((i.index_reg->reg_flags & RegRex) != 0)
		    i.rex |= REX_X;
		}

	      if (i.disp_operands
		  && (i.reloc[op] == BFD_RELOC_386_TLS_DESC_CALL
		      || i.reloc[op] == BFD_RELOC_X86_64_TLSDESC_CALL))
		i.rm.mode = 0;
	      else
		{
		  if (!fake_zero_displacement
		      && !i.disp_operands
		      && i.disp_encoding)
		    {
		      fake_zero_displacement = 1;
		      if (i.disp_encoding == disp_encoding_8bit)
			i.types[op].bitfield.disp8 = 1;
		      else
			i.types[op].bitfield.disp32 = 1;
		    }
		  i.rm.mode = mode_from_disp_size (i.types[op]);
		}
	    }

	  if (fake_zero_displacement)
	    {
	      /* Fakes a zero displacement assuming that i.types[op]
		 holds the correct displacement size.  */
	      expressionS *exp;

	      gas_assert (i.op[op].disps == 0);
	      exp = &disp_expressions[i.disp_operands++];
	      i.op[op].disps = exp;
	      exp->X_op = O_constant;
	      exp->X_add_number = 0;
	      exp->X_add_symbol = (symbolS *) 0;
	      exp->X_op_symbol = (symbolS *) 0;
	    }
	}
    else
	{
      i.rm.mode = 3;
      i.rm.regmem = i.op[op].regs->reg_num;
      set_rex_vrex (i.op[op].regs, REX_B, false);
	}

      if (op == dest)
	dest = ~0;
      if (op == source)
	source = ~0;
    }
  else
    {
      i.rm.mode = 3;
      if (!i.tm.opcode_modifier.regmem)
	{
	  gas_assert (source < MAX_OPERANDS);
	  i.rm.regmem = i.op[source].regs->reg_num;
	  set_rex_vrex (i.op[source].regs, REX_B,
			dest >= MAX_OPERANDS && i.tm.opcode_modifier.sse2avx);
	  source = ~0;
	}
      else
	{
	  gas_assert (dest < MAX_OPERANDS);
	  i.rm.regmem = i.op[dest].regs->reg_num;
	  set_rex_vrex (i.op[dest].regs, REX_B, i.tm.opcode_modifier.sse2avx);
	  dest = ~0;
	}
    }

  /* Fill in i.rm.reg field with extension opcode (if any) or the
     appropriate register.  */
  if (i.tm.extension_opcode != None)
    i.rm.reg = i.tm.extension_opcode;
  else if (!i.tm.opcode_modifier.regmem && dest < MAX_OPERANDS)
    {
      i.rm.reg = i.op[dest].regs->reg_num;
      set_rex_vrex (i.op[dest].regs, REX_R, i.tm.opcode_modifier.sse2avx);
    }
  else
    {
      gas_assert (source < MAX_OPERANDS);
      i.rm.reg = i.op[source].regs->reg_num;
      set_rex_vrex (i.op[source].regs, REX_R, false);
    }

  if (flag_code != CODE_64BIT && (i.rex & REX_R))
    {
      gas_assert (i.types[!i.tm.opcode_modifier.regmem].bitfield.class == RegCR);
      i.rex &= ~REX_R;
      add_prefix (LOCK_PREFIX_OPCODE);
    }

  return default_seg;
}

static INLINE void
frag_opcode_byte (unsigned char byte)
{
  if (now_seg != absolute_section)
    FRAG_APPEND_1_CHAR (byte);
  else
    ++abs_section_offset;
}

static unsigned int
flip_code16 (unsigned int code16)
{
  gas_assert (i.tm.operands == 1);

  return !(i.prefix[REX_PREFIX] & REX_W)
	 && (code16 ? i.tm.operand_types[0].bitfield.disp32
		    : i.tm.operand_types[0].bitfield.disp16)
	 ? CODE16 : 0;
}

static void
output_branch (void)
{
  char *p;
  int size;
  int code16;
  int prefix;
  relax_substateT subtype;
  symbolS *sym;
  offsetT off;

  if (now_seg == absolute_section)
    {
      as_bad (_("relaxable branches not supported in absolute section"));
      return;
    }

  code16 = flag_code == CODE_16BIT ? CODE16 : 0;
  size = i.disp_encoding > disp_encoding_8bit ? BIG : SMALL;

  prefix = 0;
  if (i.prefix[DATA_PREFIX] != 0)
    {
      prefix = 1;
      i.prefixes -= 1;
      code16 ^= flip_code16(code16);
    }
  /* Pentium4 branch hints.  */
  if (i.prefix[SEG_PREFIX] == CS_PREFIX_OPCODE /* not taken */
      || i.prefix[SEG_PREFIX] == DS_PREFIX_OPCODE /* taken */)
    {
      prefix++;
      i.prefixes--;
    }
  if (i.prefix[REX_PREFIX] != 0)
    {
      prefix++;
      i.prefixes--;
    }

  /* BND prefixed jump.  */
  if (i.prefix[BND_PREFIX] != 0)
    {
      prefix++;
      i.prefixes--;
    }

  if (i.prefixes != 0)
    as_warn (_("skipping prefixes on `%s'"), insn_name (&i.tm));

  /* It's always a symbol;  End frag & setup for relax.
     Make sure there is enough room in this frag for the largest
     instruction we may generate in md_convert_frag.  This is 2
     bytes for the opcode and room for the prefix and largest
     displacement.  */
  frag_grow (prefix + 2 + 4);
  /* Prefix and 1 opcode byte go in fr_fix.  */
  p = frag_more (prefix + 1);
  if (i.prefix[DATA_PREFIX] != 0)
    *p++ = DATA_PREFIX_OPCODE;
  if (i.prefix[SEG_PREFIX] == CS_PREFIX_OPCODE
      || i.prefix[SEG_PREFIX] == DS_PREFIX_OPCODE)
    *p++ = i.prefix[SEG_PREFIX];
  if (i.prefix[BND_PREFIX] != 0)
    *p++ = BND_PREFIX_OPCODE;
  if (i.prefix[REX_PREFIX] != 0)
    *p++ = i.prefix[REX_PREFIX];
  *p = i.tm.base_opcode;

  if ((unsigned char) *p == JUMP_PC_RELATIVE)
    subtype = ENCODE_RELAX_STATE (UNCOND_JUMP, size);
  else if (cpu_arch_flags.bitfield.cpui386)
    subtype = ENCODE_RELAX_STATE (COND_JUMP, size);
  else
    subtype = ENCODE_RELAX_STATE (COND_JUMP86, size);
  subtype |= code16;

  sym = i.op[0].disps->X_add_symbol;
  off = i.op[0].disps->X_add_number;

  if (i.op[0].disps->X_op != O_constant
      && i.op[0].disps->X_op != O_symbol)
    {
      /* Handle complex expressions.  */
      sym = make_expr_symbol (i.op[0].disps);
      off = 0;
    }

  frag_now->tc_frag_data.code64 = flag_code == CODE_64BIT;

  /* 1 possible extra opcode + 4 byte displacement go in var part.
     Pass reloc in fr_var.  */
  frag_var (rs_machine_dependent, 5, i.reloc[0], subtype, sym, off, p);
}

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
/* Return TRUE iff PLT32 relocation should be used for branching to
   symbol S.  */

static bool
need_plt32_p (symbolS *s)
{
  /* PLT32 relocation is ELF only.  */
  if (!IS_ELF)
    return false;

#ifdef TE_SOLARIS
  /* Don't emit PLT32 relocation on Solaris: neither native linker nor
     krtld support it.  */
  return false;
#endif

  /* Since there is no need to prepare for PLT branch on x86-64, we
     can generate R_X86_64_PLT32, instead of R_X86_64_PC32, which can
     be used as a marker for 32-bit PC-relative branches.  */
  if (!object_64bit)
    return false;

  if (s == NULL)
    return false;

  /* Weak or undefined symbol need PLT32 relocation.  */
  if (S_IS_WEAK (s) || !S_IS_DEFINED (s))
    return true;

  /* Non-global symbol doesn't need PLT32 relocation.  */
  if (! S_IS_EXTERNAL (s))
    return false;

  /* Other global symbols need PLT32 relocation.  NB: Symbol with
     non-default visibilities are treated as normal global symbol
     so that PLT32 relocation can be used as a marker for 32-bit
     PC-relative branches.  It is useful for linker relaxation.  */
  return true;
}
#endif

static void
output_jump (void)
{
  char *p;
  int size;
  fixS *fixP;
  bfd_reloc_code_real_type jump_reloc = i.reloc[0];

  if (i.tm.opcode_modifier.jump == JUMP_BYTE)
    {
      /* This is a loop or jecxz type instruction.  */
      size = 1;
      if (i.prefix[ADDR_PREFIX] != 0)
	{
	  frag_opcode_byte (ADDR_PREFIX_OPCODE);
	  i.prefixes -= 1;
	}
      /* Pentium4 branch hints.  */
      if (i.prefix[SEG_PREFIX] == CS_PREFIX_OPCODE /* not taken */
	  || i.prefix[SEG_PREFIX] == DS_PREFIX_OPCODE /* taken */)
	{
	  frag_opcode_byte (i.prefix[SEG_PREFIX]);
	  i.prefixes--;
	}
    }
  else
    {
      int code16;

      code16 = 0;
      if (flag_code == CODE_16BIT)
	code16 = CODE16;

      if (i.prefix[DATA_PREFIX] != 0)
	{
	  frag_opcode_byte (DATA_PREFIX_OPCODE);
	  i.prefixes -= 1;
	  code16 ^= flip_code16(code16);
	}

      size = 4;
      if (code16)
	size = 2;
    }

  /* BND prefixed jump.  */
  if (i.prefix[BND_PREFIX] != 0)
    {
      frag_opcode_byte (i.prefix[BND_PREFIX]);
      i.prefixes -= 1;
    }

  if (i.prefix[REX_PREFIX] != 0)
    {
      frag_opcode_byte (i.prefix[REX_PREFIX]);
      i.prefixes -= 1;
    }

  if (i.prefixes != 0)
    as_warn (_("skipping prefixes on `%s'"), insn_name (&i.tm));

  if (now_seg == absolute_section)
    {
      abs_section_offset += i.opcode_length + size;
      return;
    }

  p = frag_more (i.opcode_length + size);
  switch (i.opcode_length)
    {
    case 2:
      *p++ = i.tm.base_opcode >> 8;
      /* Fall through.  */
    case 1:
      *p++ = i.tm.base_opcode;
      break;
    default:
      abort ();
    }

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  if (flag_code == CODE_64BIT && size == 4
      && jump_reloc == NO_RELOC && i.op[0].disps->X_add_number == 0
      && need_plt32_p (i.op[0].disps->X_add_symbol))
    jump_reloc = BFD_RELOC_X86_64_PLT32;
#endif

  jump_reloc = reloc (size, 1, 1, jump_reloc);

  fixP = fix_new_exp (frag_now, p - frag_now->fr_literal, size,
		      i.op[0].disps, 1, jump_reloc);

  /* All jumps handled here are signed, but don't unconditionally use a
     signed limit check for 32 and 16 bit jumps as we want to allow wrap
     around at 4G (outside of 64-bit mode) and 64k (except for XBEGIN)
     respectively.  */
  switch (size)
    {
    case 1:
      fixP->fx_signed = 1;
      break;

    case 2:
      if (i.tm.mnem_off == MN_xbegin)
	fixP->fx_signed = 1;
      break;

    case 4:
      if (flag_code == CODE_64BIT)
	fixP->fx_signed = 1;
      break;
    }
}

static void
output_interseg_jump (void)
{
  char *p;
  int size;
  int prefix;
  int code16;

  code16 = 0;
  if (flag_code == CODE_16BIT)
    code16 = CODE16;

  prefix = 0;
  if (i.prefix[DATA_PREFIX] != 0)
    {
      prefix = 1;
      i.prefixes -= 1;
      code16 ^= CODE16;
    }

  gas_assert (!i.prefix[REX_PREFIX]);

  size = 4;
  if (code16)
    size = 2;

  if (i.prefixes != 0)
    as_warn (_("skipping prefixes on `%s'"), insn_name (&i.tm));

  if (now_seg == absolute_section)
    {
      abs_section_offset += prefix + 1 + 2 + size;
      return;
    }

  /* 1 opcode; 2 segment; offset  */
  p = frag_more (prefix + 1 + 2 + size);

  if (i.prefix[DATA_PREFIX] != 0)
    *p++ = DATA_PREFIX_OPCODE;

  if (i.prefix[REX_PREFIX] != 0)
    *p++ = i.prefix[REX_PREFIX];

  *p++ = i.tm.base_opcode;
  if (i.op[1].imms->X_op == O_constant)
    {
      offsetT n = i.op[1].imms->X_add_number;

      if (size == 2
	  && !fits_in_unsigned_word (n)
	  && !fits_in_signed_word (n))
	{
	  as_bad (_("16-bit jump out of range"));
	  return;
	}
      md_number_to_chars (p, n, size);
    }
  else
    fix_new_exp (frag_now, p - frag_now->fr_literal, size,
		 i.op[1].imms, 0, reloc (size, 0, 0, i.reloc[1]));

  p += size;
  if (i.op[0].imms->X_op == O_constant)
    md_number_to_chars (p, (valueT) i.op[0].imms->X_add_number, 2);
  else
    fix_new_exp (frag_now, p - frag_now->fr_literal, 2,
		 i.op[0].imms, 0, reloc (2, 0, 0, i.reloc[0]));
}

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
void
x86_cleanup (void)
{
  char *p;
  asection *seg = now_seg;
  subsegT subseg = now_subseg;
  asection *sec;
  unsigned int alignment, align_size_1;
  unsigned int isa_1_descsz, feature_2_descsz, descsz;
  unsigned int isa_1_descsz_raw, feature_2_descsz_raw;
  unsigned int padding;

  if (!IS_ELF || !x86_used_note)
    return;

  x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_X86;

  /* The .note.gnu.property section layout:

     Field	Length		Contents
     ----	----		----
     n_namsz	4		4
     n_descsz	4		The note descriptor size
     n_type	4		NT_GNU_PROPERTY_TYPE_0
     n_name	4		"GNU"
     n_desc	n_descsz	The program property array
     ....	....		....
   */

  /* Create the .note.gnu.property section.  */
  sec = subseg_new (NOTE_GNU_PROPERTY_SECTION_NAME, 0);
  bfd_set_section_flags (sec,
			 (SEC_ALLOC
			  | SEC_LOAD
			  | SEC_DATA
			  | SEC_HAS_CONTENTS
			  | SEC_READONLY));

  if (get_elf_backend_data (stdoutput)->s->elfclass == ELFCLASS64)
    {
      align_size_1 = 7;
      alignment = 3;
    }
  else
    {
      align_size_1 = 3;
      alignment = 2;
    }

  bfd_set_section_alignment (sec, alignment);
  elf_section_type (sec) = SHT_NOTE;

  /* GNU_PROPERTY_X86_ISA_1_USED: 4-byte type + 4-byte data size
				  + 4-byte data  */
  isa_1_descsz_raw = 4 + 4 + 4;
  /* Align GNU_PROPERTY_X86_ISA_1_USED.  */
  isa_1_descsz = (isa_1_descsz_raw + align_size_1) & ~align_size_1;

  feature_2_descsz_raw = isa_1_descsz;
  /* GNU_PROPERTY_X86_FEATURE_2_USED: 4-byte type + 4-byte data size
				      + 4-byte data  */
  feature_2_descsz_raw += 4 + 4 + 4;
  /* Align GNU_PROPERTY_X86_FEATURE_2_USED.  */
  feature_2_descsz = ((feature_2_descsz_raw + align_size_1)
		      & ~align_size_1);

  descsz = feature_2_descsz;
  /* Section size: n_namsz + n_descsz + n_type + n_name + n_descsz.  */
  p = frag_more (4 + 4 + 4 + 4 + descsz);

  /* Write n_namsz.  */
  md_number_to_chars (p, (valueT) 4, 4);

  /* Write n_descsz.  */
  md_number_to_chars (p + 4, (valueT) descsz, 4);

  /* Write n_type.  */
  md_number_to_chars (p + 4 * 2, (valueT) NT_GNU_PROPERTY_TYPE_0, 4);

  /* Write n_name.  */
  memcpy (p + 4 * 3, "GNU", 4);

  /* Write 4-byte type.  */
  md_number_to_chars (p + 4 * 4,
		      (valueT) GNU_PROPERTY_X86_ISA_1_USED, 4);

  /* Write 4-byte data size.  */
  md_number_to_chars (p + 4 * 5, (valueT) 4, 4);

  /* Write 4-byte data.  */
  md_number_to_chars (p + 4 * 6, (valueT) x86_isa_1_used, 4);

  /* Zero out paddings.  */
  padding = isa_1_descsz - isa_1_descsz_raw;
  if (padding)
    memset (p + 4 * 7, 0, padding);

  /* Write 4-byte type.  */
  md_number_to_chars (p + isa_1_descsz + 4 * 4,
		      (valueT) GNU_PROPERTY_X86_FEATURE_2_USED, 4);

  /* Write 4-byte data size.  */
  md_number_to_chars (p + isa_1_descsz + 4 * 5, (valueT) 4, 4);

  /* Write 4-byte data.  */
  md_number_to_chars (p + isa_1_descsz + 4 * 6,
		      (valueT) x86_feature_2_used, 4);

  /* Zero out paddings.  */
  padding = feature_2_descsz - feature_2_descsz_raw;
  if (padding)
    memset (p + isa_1_descsz + 4 * 7, 0, padding);

  /* We probably can't restore the current segment, for there likely
     isn't one yet...  */
  if (seg && subseg)
    subseg_set (seg, subseg);
}

bool
x86_support_sframe_p (void)
{
  /* At this time, SFrame stack trace is supported for AMD64 ABI only.  */
  return (x86_elf_abi == X86_64_ABI);
}

bool
x86_sframe_ra_tracking_p (void)
{
  /* In AMD64, return address is always stored on the stack at a fixed offset
     from the CFA (provided via x86_sframe_cfa_ra_offset ()).
     Do not track explicitly via an SFrame Frame Row Entry.  */
  return false;
}

offsetT
x86_sframe_cfa_ra_offset (void)
{
  gas_assert (x86_elf_abi == X86_64_ABI);
  return (offsetT) -8;
}

unsigned char
x86_sframe_get_abi_arch (void)
{
  unsigned char sframe_abi_arch = 0;

  if (x86_support_sframe_p ())
    {
      gas_assert (!target_big_endian);
      sframe_abi_arch = SFRAME_ABI_AMD64_ENDIAN_LITTLE;
    }

  return sframe_abi_arch;
}

#endif

static unsigned int
encoding_length (const fragS *start_frag, offsetT start_off,
		 const char *frag_now_ptr)
{
  unsigned int len = 0;

  if (start_frag != frag_now)
    {
      const fragS *fr = start_frag;

      do {
	len += fr->fr_fix;
	fr = fr->fr_next;
      } while (fr && fr != frag_now);
    }

  return len - start_off + (frag_now_ptr - frag_now->fr_literal);
}

/* Return 1 for test, and, cmp, add, sub, inc and dec which may
   be macro-fused with conditional jumps.
   NB: If TEST/AND/CMP/ADD/SUB/INC/DEC is of RIP relative address,
   or is one of the following format:

    cmp m, imm
    add m, imm
    sub m, imm
   test m, imm
    and m, imm
    inc m
    dec m

   it is unfusible.  */

static int
maybe_fused_with_jcc_p (enum mf_cmp_kind* mf_cmp_p)
{
  /* No RIP address.  */
  if (i.base_reg && i.base_reg->reg_num == RegIP)
    return 0;

  /* No opcodes outside of base encoding space.  */
  if (i.tm.opcode_space != SPACE_BASE)
    return 0;

  /* add, sub without add/sub m, imm.  */
  if (i.tm.base_opcode <= 5
      || (i.tm.base_opcode >= 0x28 && i.tm.base_opcode <= 0x2d)
      || ((i.tm.base_opcode | 3) == 0x83
	  && (i.tm.extension_opcode == 0x5
	      || i.tm.extension_opcode == 0x0)))
    {
      *mf_cmp_p = mf_cmp_alu_cmp;
      return !(i.mem_operands && i.imm_operands);
    }

  /* and without and m, imm.  */
  if ((i.tm.base_opcode >= 0x20 && i.tm.base_opcode <= 0x25)
      || ((i.tm.base_opcode | 3) == 0x83
	  && i.tm.extension_opcode == 0x4))
    {
      *mf_cmp_p = mf_cmp_test_and;
      return !(i.mem_operands && i.imm_operands);
    }

  /* test without test m imm.  */
  if ((i.tm.base_opcode | 1) == 0x85
      || (i.tm.base_opcode | 1) == 0xa9
      || ((i.tm.base_opcode | 1) == 0xf7
	  && i.tm.extension_opcode == 0))
    {
      *mf_cmp_p = mf_cmp_test_and;
      return !(i.mem_operands && i.imm_operands);
    }

  /* cmp without cmp m, imm.  */
  if ((i.tm.base_opcode >= 0x38 && i.tm.base_opcode <= 0x3d)
      || ((i.tm.base_opcode | 3) == 0x83
	  && (i.tm.extension_opcode == 0x7)))
    {
      *mf_cmp_p = mf_cmp_alu_cmp;
      return !(i.mem_operands && i.imm_operands);
    }

  /* inc, dec without inc/dec m.   */
  if ((i.tm.cpu_flags.bitfield.cpuno64
       && (i.tm.base_opcode | 0xf) == 0x4f)
      || ((i.tm.base_opcode | 1) == 0xff
	  && i.tm.extension_opcode <= 0x1))
    {
      *mf_cmp_p = mf_cmp_incdec;
      return !i.mem_operands;
    }

  return 0;
}

/* Return 1 if a FUSED_JCC_PADDING frag should be generated.  */

static int
add_fused_jcc_padding_frag_p (enum mf_cmp_kind* mf_cmp_p)
{
  /* NB: Don't work with COND_JUMP86 without i386.  */
  if (!align_branch_power
      || now_seg == absolute_section
      || !cpu_arch_flags.bitfield.cpui386
      || !(align_branch & align_branch_fused_bit))
    return 0;

  if (maybe_fused_with_jcc_p (mf_cmp_p))
    {
      if (last_insn.kind == last_insn_other
	  || last_insn.seg != now_seg)
	return 1;
      if (flag_debug)
	as_warn_where (last_insn.file, last_insn.line,
		       _("`%s` skips -malign-branch-boundary on `%s`"),
		       last_insn.name, insn_name (&i.tm));
    }

  return 0;
}

/* Return 1 if a BRANCH_PREFIX frag should be generated.  */

static int
add_branch_prefix_frag_p (void)
{
  /* NB: Don't work with COND_JUMP86 without i386.  Don't add prefix
     to PadLock instructions since they include prefixes in opcode.  */
  if (!align_branch_power
      || !align_branch_prefix_size
      || now_seg == absolute_section
      || i.tm.cpu_flags.bitfield.cpupadlock
      || !cpu_arch_flags.bitfield.cpui386)
    return 0;

  /* Don't add prefix if it is a prefix or there is no operand in case
     that segment prefix is special.  */
  if (!i.operands || i.tm.opcode_modifier.isprefix)
    return 0;

  if (last_insn.kind == last_insn_other
      || last_insn.seg != now_seg)
    return 1;

  if (flag_debug)
    as_warn_where (last_insn.file, last_insn.line,
		   _("`%s` skips -malign-branch-boundary on `%s`"),
		   last_insn.name, insn_name (&i.tm));

  return 0;
}

/* Return 1 if a BRANCH_PADDING frag should be generated.  */

static int
add_branch_padding_frag_p (enum align_branch_kind *branch_p,
			   enum mf_jcc_kind *mf_jcc_p)
{
  int add_padding;

  /* NB: Don't work with COND_JUMP86 without i386.  */
  if (!align_branch_power
      || now_seg == absolute_section
      || !cpu_arch_flags.bitfield.cpui386
      || i.tm.opcode_space != SPACE_BASE)
    return 0;

  add_padding = 0;

  /* Check for jcc and direct jmp.  */
  if (i.tm.opcode_modifier.jump == JUMP)
    {
      if (i.tm.base_opcode == JUMP_PC_RELATIVE)
	{
	  *branch_p = align_branch_jmp;
	  add_padding = align_branch & align_branch_jmp_bit;
	}
      else
	{
	  /* Because J<cc> and JN<cc> share same group in macro-fusible table,
	     igore the lowest bit.  */
	  *mf_jcc_p = (i.tm.base_opcode & 0x0e) >> 1;
	  *branch_p = align_branch_jcc;
	  if ((align_branch & align_branch_jcc_bit))
	    add_padding = 1;
	}
    }
  else if ((i.tm.base_opcode | 1) == 0xc3)
    {
      /* Near ret.  */
      *branch_p = align_branch_ret;
      if ((align_branch & align_branch_ret_bit))
	add_padding = 1;
    }
  else
    {
      /* Check for indirect jmp, direct and indirect calls.  */
      if (i.tm.base_opcode == 0xe8)
	{
	  /* Direct call.  */
	  *branch_p = align_branch_call;
	  if ((align_branch & align_branch_call_bit))
	    add_padding = 1;
	}
      else if (i.tm.base_opcode == 0xff
	       && (i.tm.extension_opcode == 2
		   || i.tm.extension_opcode == 4))
	{
	  /* Indirect call and jmp.  */
	  *branch_p = align_branch_indirect;
	  if ((align_branch & align_branch_indirect_bit))
	    add_padding = 1;
	}

      if (add_padding
	  && i.disp_operands
	  && tls_get_addr
	  && (i.op[0].disps->X_op == O_symbol
	      || (i.op[0].disps->X_op == O_subtract
		  && i.op[0].disps->X_op_symbol == GOT_symbol)))
	{
	  symbolS *s = i.op[0].disps->X_add_symbol;
	  /* No padding to call to global or undefined tls_get_addr.  */
	  if ((S_IS_EXTERNAL (s) || !S_IS_DEFINED (s))
	      && strcmp (S_GET_NAME (s), tls_get_addr) == 0)
	    return 0;
	}
    }

  if (add_padding
      && last_insn.kind != last_insn_other
      && last_insn.seg == now_seg)
    {
      if (flag_debug)
	as_warn_where (last_insn.file, last_insn.line,
		       _("`%s` skips -malign-branch-boundary on `%s`"),
		       last_insn.name, insn_name (&i.tm));
      return 0;
    }

  return add_padding;
}

static void
output_insn (void)
{
  fragS *insn_start_frag;
  offsetT insn_start_off;
  fragS *fragP = NULL;
  enum align_branch_kind branch = align_branch_none;
  /* The initializer is arbitrary just to avoid uninitialized error.
     it's actually either assigned in add_branch_padding_frag_p
     or never be used.  */
  enum mf_jcc_kind mf_jcc = mf_jcc_jo;

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  if (IS_ELF && x86_used_note && now_seg != absolute_section)
    {
      if ((i.xstate & xstate_tmm) == xstate_tmm
	  || i.tm.cpu_flags.bitfield.cpuamx_tile)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_TMM;

      if (i.tm.cpu_flags.bitfield.cpu8087
	  || i.tm.cpu_flags.bitfield.cpu287
	  || i.tm.cpu_flags.bitfield.cpu387
	  || i.tm.cpu_flags.bitfield.cpu687
	  || i.tm.cpu_flags.bitfield.cpufisttp)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_X87;

      if ((i.xstate & xstate_mmx)
	  || i.tm.mnem_off == MN_emms
	  || i.tm.mnem_off == MN_femms)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_MMX;

      if (i.index_reg)
	{
	  if (i.index_reg->reg_type.bitfield.zmmword)
	    i.xstate |= xstate_zmm;
	  else if (i.index_reg->reg_type.bitfield.ymmword)
	    i.xstate |= xstate_ymm;
	  else if (i.index_reg->reg_type.bitfield.xmmword)
	    i.xstate |= xstate_xmm;
	}

      /* vzeroall / vzeroupper */
      if (i.tm.base_opcode == 0x77 && i.tm.cpu_flags.bitfield.cpuavx)
	i.xstate |= xstate_ymm;

      if ((i.xstate & xstate_xmm)
	  /* ldmxcsr / stmxcsr / vldmxcsr / vstmxcsr */
	  || (i.tm.base_opcode == 0xae
	      && (i.tm.cpu_flags.bitfield.cpusse
		  || i.tm.cpu_flags.bitfield.cpuavx))
	  || i.tm.cpu_flags.bitfield.cpuwidekl
	  || i.tm.cpu_flags.bitfield.cpukl)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_XMM;

      if ((i.xstate & xstate_ymm) == xstate_ymm)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_YMM;
      if ((i.xstate & xstate_zmm) == xstate_zmm)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_ZMM;
      if (i.mask.reg || (i.xstate & xstate_mask) == xstate_mask)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_MASK;
      if (i.tm.cpu_flags.bitfield.cpufxsr)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_FXSR;
      if (i.tm.cpu_flags.bitfield.cpuxsave)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_XSAVE;
      if (i.tm.cpu_flags.bitfield.cpuxsaveopt)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_XSAVEOPT;
      if (i.tm.cpu_flags.bitfield.cpuxsavec)
	x86_feature_2_used |= GNU_PROPERTY_X86_FEATURE_2_XSAVEC;

      if (x86_feature_2_used
	  || i.tm.cpu_flags.bitfield.cpucmov
	  || i.tm.cpu_flags.bitfield.cpusyscall
	  || i.tm.mnem_off == MN_cmpxchg8b)
	x86_isa_1_used |= GNU_PROPERTY_X86_ISA_1_BASELINE;
      if (i.tm.cpu_flags.bitfield.cpusse3
	  || i.tm.cpu_flags.bitfield.cpussse3
	  || i.tm.cpu_flags.bitfield.cpusse4_1
	  || i.tm.cpu_flags.bitfield.cpusse4_2
	  || i.tm.cpu_flags.bitfield.cpucx16
	  || i.tm.cpu_flags.bitfield.cpupopcnt
	  /* LAHF-SAHF insns in 64-bit mode.  */
	  || (flag_code == CODE_64BIT
	      && (i.tm.base_opcode | 1) == 0x9f
	      && i.tm.opcode_space == SPACE_BASE))
	x86_isa_1_used |= GNU_PROPERTY_X86_ISA_1_V2;
      if (i.tm.cpu_flags.bitfield.cpuavx
	  || i.tm.cpu_flags.bitfield.cpuavx2
	  /* Any VEX encoded insns execpt for AVX512F, AVX512BW, AVX512DQ,
	     XOP, FMA4, LPW, TBM, and AMX.  */
	  || (i.tm.opcode_modifier.vex
	      && !i.tm.cpu_flags.bitfield.cpuavx512f
	      && !i.tm.cpu_flags.bitfield.cpuavx512bw
	      && !i.tm.cpu_flags.bitfield.cpuavx512dq
	      && !i.tm.cpu_flags.bitfield.cpuxop
	      && !i.tm.cpu_flags.bitfield.cpufma4
	      && !i.tm.cpu_flags.bitfield.cpulwp
	      && !i.tm.cpu_flags.bitfield.cputbm
	      && !(x86_feature_2_used & GNU_PROPERTY_X86_FEATURE_2_TMM))
	  || i.tm.cpu_flags.bitfield.cpuf16c
	  || i.tm.cpu_flags.bitfield.cpufma
	  || i.tm.cpu_flags.bitfield.cpulzcnt
	  || i.tm.cpu_flags.bitfield.cpumovbe
	  || i.tm.cpu_flags.bitfield.cpuxsaves
	  || (x86_feature_2_used
	      & (GNU_PROPERTY_X86_FEATURE_2_XSAVE
		 | GNU_PROPERTY_X86_FEATURE_2_XSAVEOPT
		 | GNU_PROPERTY_X86_FEATURE_2_XSAVEC)) != 0)
	x86_isa_1_used |= GNU_PROPERTY_X86_ISA_1_V3;
      if (i.tm.cpu_flags.bitfield.cpuavx512f
	  || i.tm.cpu_flags.bitfield.cpuavx512bw
	  || i.tm.cpu_flags.bitfield.cpuavx512dq
	  || i.tm.cpu_flags.bitfield.cpuavx512vl
	  /* Any EVEX encoded insns except for AVX512ER, AVX512PF,
	     AVX512-4FMAPS, and AVX512-4VNNIW.  */
	  || (i.tm.opcode_modifier.evex
	      && !i.tm.cpu_flags.bitfield.cpuavx512er
	      && !i.tm.cpu_flags.bitfield.cpuavx512pf
	      && !i.tm.cpu_flags.bitfield.cpuavx512_4fmaps
	      && !i.tm.cpu_flags.bitfield.cpuavx512_4vnniw))
	x86_isa_1_used |= GNU_PROPERTY_X86_ISA_1_V4;
    }
#endif

  /* Tie dwarf2 debug info to the address at the start of the insn.
     We can't do this after the insn has been output as the current
     frag may have been closed off.  eg. by frag_var.  */
  dwarf2_emit_insn (0);

  insn_start_frag = frag_now;
  insn_start_off = frag_now_fix ();

  if (add_branch_padding_frag_p (&branch, &mf_jcc))
    {
      char *p;
      /* Branch can be 8 bytes.  Leave some room for prefixes.  */
      unsigned int max_branch_padding_size = 14;

      /* Align section to boundary.  */
      record_alignment (now_seg, align_branch_power);

      /* Make room for padding.  */
      frag_grow (max_branch_padding_size);

      /* Start of the padding.  */
      p = frag_more (0);

      fragP = frag_now;

      frag_var (rs_machine_dependent, max_branch_padding_size, 0,
		ENCODE_RELAX_STATE (BRANCH_PADDING, 0),
		NULL, 0, p);

      fragP->tc_frag_data.mf_type = mf_jcc;
      fragP->tc_frag_data.branch_type = branch;
      fragP->tc_frag_data.max_bytes = max_branch_padding_size;
    }

  if (!cpu_arch_flags.bitfield.cpui386 && (flag_code != CODE_16BIT)
      && !pre_386_16bit_warned)
    {
      as_warn (_("use .code16 to ensure correct addressing mode"));
      pre_386_16bit_warned = true;
    }

  /* Output jumps.  */
  if (i.tm.opcode_modifier.jump == JUMP)
    output_branch ();
  else if (i.tm.opcode_modifier.jump == JUMP_BYTE
	   || i.tm.opcode_modifier.jump == JUMP_DWORD)
    output_jump ();
  else if (i.tm.opcode_modifier.jump == JUMP_INTERSEGMENT)
    output_interseg_jump ();
  else
    {
      /* Output normal instructions here.  */
      char *p;
      unsigned char *q;
      unsigned int j;
      enum mf_cmp_kind mf_cmp;

      if (avoid_fence
	  && (i.tm.base_opcode == 0xaee8
	      || i.tm.base_opcode == 0xaef0
	      || i.tm.base_opcode == 0xaef8))
	{
	  /* Encode lfence, mfence, and sfence as
	     f0 83 04 24 00   lock addl $0x0, (%{re}sp).  */
	  if (flag_code == CODE_16BIT)
	    as_bad (_("Cannot convert `%s' in 16-bit mode"), insn_name (&i.tm));
	  else if (omit_lock_prefix)
	    as_bad (_("Cannot convert `%s' with `-momit-lock-prefix=yes' in effect"),
		    insn_name (&i.tm));
	  else if (now_seg != absolute_section)
	    {
	      offsetT val = 0x240483f0ULL;

	      p = frag_more (5);
	      md_number_to_chars (p, val, 5);
	    }
	  else
	    abs_section_offset += 5;
	  return;
	}

      /* Some processors fail on LOCK prefix. This options makes
	 assembler ignore LOCK prefix and serves as a workaround.  */
      if (omit_lock_prefix)
	{
	  if (i.tm.base_opcode == LOCK_PREFIX_OPCODE
	      && i.tm.opcode_modifier.isprefix)
	    return;
	  i.prefix[LOCK_PREFIX] = 0;
	}

      if (branch)
	/* Skip if this is a branch.  */
	;
      else if (add_fused_jcc_padding_frag_p (&mf_cmp))
	{
	  /* Make room for padding.  */
	  frag_grow (MAX_FUSED_JCC_PADDING_SIZE);
	  p = frag_more (0);

	  fragP = frag_now;

	  frag_var (rs_machine_dependent, MAX_FUSED_JCC_PADDING_SIZE, 0,
		    ENCODE_RELAX_STATE (FUSED_JCC_PADDING, 0),
		    NULL, 0, p);

	  fragP->tc_frag_data.mf_type = mf_cmp;
	  fragP->tc_frag_data.branch_type = align_branch_fused;
	  fragP->tc_frag_data.max_bytes = MAX_FUSED_JCC_PADDING_SIZE;
	}
      else if (add_branch_prefix_frag_p ())
	{
	  unsigned int max_prefix_size = align_branch_prefix_size;

	  /* Make room for padding.  */
	  frag_grow (max_prefix_size);
	  p = frag_more (0);

	  fragP = frag_now;

	  frag_var (rs_machine_dependent, max_prefix_size, 0,
		    ENCODE_RELAX_STATE (BRANCH_PREFIX, 0),
		    NULL, 0, p);

	  fragP->tc_frag_data.max_bytes = max_prefix_size;
	}

      /* Since the VEX/EVEX prefix contains the implicit prefix, we
	 don't need the explicit prefix.  */
      if (!is_any_vex_encoding (&i.tm))
	{
	  switch (i.tm.opcode_modifier.opcodeprefix)
	    {
	    case PREFIX_0X66:
	      add_prefix (0x66);
	      break;
	    case PREFIX_0XF2:
	      add_prefix (0xf2);
	      break;
	    case PREFIX_0XF3:
	      if (!i.tm.cpu_flags.bitfield.cpupadlock
		  || (i.prefix[REP_PREFIX] != 0xf3))
		add_prefix (0xf3);
	      break;
	    case PREFIX_NONE:
	      switch (i.opcode_length)
		{
		case 2:
		  break;
		case 1:
		  /* Check for pseudo prefixes.  */
		  if (!i.tm.opcode_modifier.isprefix || i.tm.base_opcode)
		    break;
		  as_bad_where (insn_start_frag->fr_file,
				insn_start_frag->fr_line,
				_("pseudo prefix without instruction"));
		  return;
		default:
		  abort ();
		}
	      break;
	    default:
	      abort ();
	    }

#if defined (OBJ_MAYBE_ELF) || defined (OBJ_ELF)
	  /* For x32, add a dummy REX_OPCODE prefix for mov/add with
	     R_X86_64_GOTTPOFF relocation so that linker can safely
	     perform IE->LE optimization.  A dummy REX_OPCODE prefix
	     is also needed for lea with R_X86_64_GOTPC32_TLSDESC
	     relocation for GDesc -> IE/LE optimization.  */
	  if (x86_elf_abi == X86_64_X32_ABI
	      && i.operands == 2
	      && (i.reloc[0] == BFD_RELOC_X86_64_GOTTPOFF
		  || i.reloc[0] == BFD_RELOC_X86_64_GOTPC32_TLSDESC)
	      && i.prefix[REX_PREFIX] == 0)
	    add_prefix (REX_OPCODE);
#endif

	  /* The prefix bytes.  */
	  for (j = ARRAY_SIZE (i.prefix), q = i.prefix; j > 0; j--, q++)
	    if (*q)
	      frag_opcode_byte (*q);
	}
      else
	{
	  for (j = 0, q = i.prefix; j < ARRAY_SIZE (i.prefix); j++, q++)
	    if (*q)
	      switch (j)
		{
		case SEG_PREFIX:
		case ADDR_PREFIX:
		  frag_opcode_byte (*q);
		  break;
		default:
		  /* There should be no other prefixes for instructions
		     with VEX prefix.  */
		  abort ();
		}

	  /* For EVEX instructions i.vrex should become 0 after
	     build_evex_prefix.  For VEX instructions upper 16 registers
	     aren't available, so VREX should be 0.  */
	  if (i.vrex)
	    abort ();
	  /* Now the VEX prefix.  */
	  if (now_seg != absolute_section)
	    {
	      p = frag_more (i.vex.length);
	      for (j = 0; j < i.vex.length; j++)
		p[j] = i.vex.bytes[j];
	    }
	  else
	    abs_section_offset += i.vex.length;
	}

      /* Now the opcode; be careful about word order here!  */
      j = i.opcode_length;
      if (!i.vex.length)
	switch (i.tm.opcode_space)
	  {
	  case SPACE_BASE:
	    break;
	  case SPACE_0F:
	    ++j;
	    break;
	  case SPACE_0F38:
	  case SPACE_0F3A:
	    j += 2;
	    break;
	  default:
	    abort ();
	  }

      if (now_seg == absolute_section)
	abs_section_offset += j;
      else if (j == 1)
	{
	  FRAG_APPEND_1_CHAR (i.tm.base_opcode);
	}
      else
	{
	  p = frag_more (j);
	  if (!i.vex.length
	      && i.tm.opcode_space != SPACE_BASE)
	    {
	      *p++ = 0x0f;
	      if (i.tm.opcode_space != SPACE_0F)
		*p++ = i.tm.opcode_space == SPACE_0F38
		       ? 0x38 : 0x3a;
	    }

	  switch (i.opcode_length)
	    {
	    case 2:
	      /* Put out high byte first: can't use md_number_to_chars!  */
	      *p++ = (i.tm.base_opcode >> 8) & 0xff;
	      /* Fall through.  */
	    case 1:
	      *p = i.tm.base_opcode & 0xff;
	      break;
	    default:
	      abort ();
	      break;
	    }

	}

      /* Now the modrm byte and sib byte (if present).  */
      if (i.tm.opcode_modifier.modrm)
	{
	  frag_opcode_byte ((i.rm.regmem << 0)
			     | (i.rm.reg << 3)
			     | (i.rm.mode << 6));
	  /* If i.rm.regmem == ESP (4)
	     && i.rm.mode != (Register mode)
	     && not 16 bit
	     ==> need second modrm byte.  */
	  if (i.rm.regmem == ESCAPE_TO_TWO_BYTE_ADDRESSING
	      && i.rm.mode != 3
	      && !(i.base_reg && i.base_reg->reg_type.bitfield.word))
	    frag_opcode_byte ((i.sib.base << 0)
			      | (i.sib.index << 3)
			      | (i.sib.scale << 6));
	}

      if (i.disp_operands)
	output_disp (insn_start_frag, insn_start_off);

      if (i.imm_operands)
	output_imm (insn_start_frag, insn_start_off);

      /*
       * frag_now_fix () returning plain abs_section_offset when we're in the
       * absolute section, and abs_section_offset not getting updated as data
       * gets added to the frag breaks the logic below.
       */
      if (now_seg != absolute_section)
	{
	  j = encoding_length (insn_start_frag, insn_start_off, frag_more (0));
	  if (j > 15)
	    as_warn (_("instruction length of %u bytes exceeds the limit of 15"),
		     j);
	  else if (fragP)
	    {
	      /* NB: Don't add prefix with GOTPC relocation since
		 output_disp() above depends on the fixed encoding
		 length.  Can't add prefix with TLS relocation since
		 it breaks TLS linker optimization.  */
	      unsigned int max = i.has_gotpc_tls_reloc ? 0 : 15 - j;
	      /* Prefix count on the current instruction.  */
	      unsigned int count = i.vex.length;
	      unsigned int k;
	      for (k = 0; k < ARRAY_SIZE (i.prefix); k++)
		/* REX byte is encoded in VEX/EVEX prefix.  */
		if (i.prefix[k] && (k != REX_PREFIX || !i.vex.length))
		  count++;

	      /* Count prefixes for extended opcode maps.  */
	      if (!i.vex.length)
		switch (i.tm.opcode_space)
		  {
		  case SPACE_BASE:
		    break;
		  case SPACE_0F:
		    count++;
		    break;
		  case SPACE_0F38:
		  case SPACE_0F3A:
		    count += 2;
		    break;
		  default:
		    abort ();
		  }

	      if (TYPE_FROM_RELAX_STATE (fragP->fr_subtype)
		  == BRANCH_PREFIX)
		{
		  /* Set the maximum prefix size in BRANCH_PREFIX
		     frag.  */
		  if (fragP->tc_frag_data.max_bytes > max)
		    fragP->tc_frag_data.max_bytes = max;
		  if (fragP->tc_frag_data.max_bytes > count)
		    fragP->tc_frag_data.max_bytes -= count;
		  else
		    fragP->tc_frag_data.max_bytes = 0;
		}
	      else
		{
		  /* Remember the maximum prefix size in FUSED_JCC_PADDING
		     frag.  */
		  unsigned int max_prefix_size;
		  if (align_branch_prefix_size > max)
		    max_prefix_size = max;
		  else
		    max_prefix_size = align_branch_prefix_size;
		  if (max_prefix_size > count)
		    fragP->tc_frag_data.max_prefix_length
		      = max_prefix_size - count;
		}

	      /* Use existing segment prefix if possible.  Use CS
		 segment prefix in 64-bit mode.  In 32-bit mode, use SS
		 segment prefix with ESP/EBP base register and use DS
		 segment prefix without ESP/EBP base register.  */
	      if (i.prefix[SEG_PREFIX])
		fragP->tc_frag_data.default_prefix = i.prefix[SEG_PREFIX];
	      else if (flag_code == CODE_64BIT)
		fragP->tc_frag_data.default_prefix = CS_PREFIX_OPCODE;
	      else if (i.base_reg
		       && (i.base_reg->reg_num == 4
			   || i.base_reg->reg_num == 5))
		fragP->tc_frag_data.default_prefix = SS_PREFIX_OPCODE;
	      else
		fragP->tc_frag_data.default_prefix = DS_PREFIX_OPCODE;
	    }
	}
    }

  /* NB: Don't work with COND_JUMP86 without i386.  */
  if (align_branch_power
      && now_seg != absolute_section
      && cpu_arch_flags.bitfield.cpui386)
    {
      /* Terminate each frag so that we can add prefix and check for
         fused jcc.  */
      frag_wane (frag_now);
      frag_new (0);
    }

#ifdef DEBUG386
  if (flag_debug)
    {
      pi ("" /*line*/, &i);
    }
#endif /* DEBUG386  */
}

/* Return the size of the displacement operand N.  */

static int
disp_size (unsigned int n)
{
  int size = 4;

  if (i.types[n].bitfield.disp64)
    size = 8;
  else if (i.types[n].bitfield.disp8)
    size = 1;
  else if (i.types[n].bitfield.disp16)
    size = 2;
  return size;
}

/* Return the size of the immediate operand N.  */

static int
imm_size (unsigned int n)
{
  int size = 4;
  if (i.types[n].bitfield.imm64)
    size = 8;
  else if (i.types[n].bitfield.imm8 || i.types[n].bitfield.imm8s)
    size = 1;
  else if (i.types[n].bitfield.imm16)
    size = 2;
  return size;
}

static void
output_disp (fragS *insn_start_frag, offsetT insn_start_off)
{
  char *p;
  unsigned int n;

  for (n = 0; n < i.operands; n++)
    {
      if (operand_type_check (i.types[n], disp))
	{
	  int size = disp_size (n);

	  if (now_seg == absolute_section)
	    abs_section_offset += size;
	  else if (i.op[n].disps->X_op == O_constant)
	    {
	      offsetT val = i.op[n].disps->X_add_number;

	      val = offset_in_range (val >> (size == 1 ? i.memshift : 0),
				     size);
	      p = frag_more (size);
	      md_number_to_chars (p, val, size);
	    }
	  else
	    {
	      enum bfd_reloc_code_real reloc_type;
	      bool pcrel = (i.flags[n] & Operand_PCrel) != 0;
	      bool sign = (flag_code == CODE_64BIT && size == 4
			   && (!want_disp32 (&i.tm)
			       || (i.tm.opcode_modifier.jump && !i.jumpabsolute
				   && !i.types[n].bitfield.baseindex)))
			  || pcrel;
	      fixS *fixP;

	      /* We can't have 8 bit displacement here.  */
	      gas_assert (!i.types[n].bitfield.disp8);

	      /* The PC relative address is computed relative
		 to the instruction boundary, so in case immediate
		 fields follows, we need to adjust the value.  */
	      if (pcrel && i.imm_operands)
		{
		  unsigned int n1;
		  int sz = 0;

		  for (n1 = 0; n1 < i.operands; n1++)
		    if (operand_type_check (i.types[n1], imm))
		      {
			/* Only one immediate is allowed for PC
			   relative address, except with .insn.  */
			gas_assert (sz == 0 || dot_insn ());
			sz += imm_size (n1);
		      }
		  /* We should find at least one immediate.  */
		  gas_assert (sz != 0);
		  i.op[n].disps->X_add_number -= sz;
		}

	      p = frag_more (size);
	      reloc_type = reloc (size, pcrel, sign, i.reloc[n]);
	      if (GOT_symbol
		  && GOT_symbol == i.op[n].disps->X_add_symbol
		  && (((reloc_type == BFD_RELOC_32
			|| reloc_type == BFD_RELOC_X86_64_32S
			|| (reloc_type == BFD_RELOC_64
			    && object_64bit))
		       && (i.op[n].disps->X_op == O_symbol
			   || (i.op[n].disps->X_op == O_add
			       && ((symbol_get_value_expression
				    (i.op[n].disps->X_op_symbol)->X_op)
				   == O_subtract))))
		      || reloc_type == BFD_RELOC_32_PCREL))
		{
		  if (!object_64bit)
		    {
		      reloc_type = BFD_RELOC_386_GOTPC;
		      i.has_gotpc_tls_reloc = true;
		      i.op[n].disps->X_add_number +=
			encoding_length (insn_start_frag, insn_start_off, p);
		    }
		  else if (reloc_type == BFD_RELOC_64)
		    reloc_type = BFD_RELOC_X86_64_GOTPC64;
		  else
		    /* Don't do the adjustment for x86-64, as there
		       the pcrel addressing is relative to the _next_
		       insn, and that is taken care of in other code.  */
		    reloc_type = BFD_RELOC_X86_64_GOTPC32;
		}
	      else if (align_branch_power)
		{
		  switch (reloc_type)
		    {
		    case BFD_RELOC_386_TLS_GD:
		    case BFD_RELOC_386_TLS_LDM:
		    case BFD_RELOC_386_TLS_IE:
		    case BFD_RELOC_386_TLS_IE_32:
		    case BFD_RELOC_386_TLS_GOTIE:
		    case BFD_RELOC_386_TLS_GOTDESC:
		    case BFD_RELOC_386_TLS_DESC_CALL:
		    case BFD_RELOC_X86_64_TLSGD:
		    case BFD_RELOC_X86_64_TLSLD:
		    case BFD_RELOC_X86_64_GOTTPOFF:
		    case BFD_RELOC_X86_64_GOTPC32_TLSDESC:
		    case BFD_RELOC_X86_64_TLSDESC_CALL:
		      i.has_gotpc_tls_reloc = true;
		    default:
		      break;
		    }
		}
	      fixP = fix_new_exp (frag_now, p - frag_now->fr_literal,
				  size, i.op[n].disps, pcrel,
				  reloc_type);

	      if (flag_code == CODE_64BIT && size == 4 && pcrel
		  && !i.prefix[ADDR_PREFIX])
		fixP->fx_signed = 1;

	      /* Check for "call/jmp *mem", "mov mem, %reg",
		 "test %reg, mem" and "binop mem, %reg" where binop
		 is one of adc, add, and, cmp, or, sbb, sub, xor
		 instructions without data prefix.  Always generate
		 R_386_GOT32X for "sym*GOT" operand in 32-bit mode.  */
	      if (i.prefix[DATA_PREFIX] == 0
		  && (generate_relax_relocations
		      || (!object_64bit
			  && i.rm.mode == 0
			  && i.rm.regmem == 5))
		  && (i.rm.mode == 2
		      || (i.rm.mode == 0 && i.rm.regmem == 5))
		  && i.tm.opcode_space == SPACE_BASE
		  && ((i.operands == 1
		       && i.tm.base_opcode == 0xff
		       && (i.rm.reg == 2 || i.rm.reg == 4))
		      || (i.operands == 2
			  && (i.tm.base_opcode == 0x8b
			      || i.tm.base_opcode == 0x85
			      || (i.tm.base_opcode & ~0x38) == 0x03))))
		{
		  if (object_64bit)
		    {
		      fixP->fx_tcbit = i.rex != 0;
		      if (i.base_reg
			  && (i.base_reg->reg_num == RegIP))
		      fixP->fx_tcbit2 = 1;
		    }
		  else
		    fixP->fx_tcbit2 = 1;
		}
	    }
	}
    }
}

static void
output_imm (fragS *insn_start_frag, offsetT insn_start_off)
{
  char *p;
  unsigned int n;

  for (n = 0; n < i.operands; n++)
    {
      if (operand_type_check (i.types[n], imm))
	{
	  int size = imm_size (n);

	  if (now_seg == absolute_section)
	    abs_section_offset += size;
	  else if (i.op[n].imms->X_op == O_constant)
	    {
	      offsetT val;

	      val = offset_in_range (i.op[n].imms->X_add_number,
				     size);
	      p = frag_more (size);
	      md_number_to_chars (p, val, size);
	    }
	  else
	    {
	      /* Not absolute_section.
		 Need a 32-bit fixup (don't support 8bit
		 non-absolute imms).  Try to support other
		 sizes ...  */
	      enum bfd_reloc_code_real reloc_type;
	      int sign;

	      if (i.types[n].bitfield.imm32s
		  && (i.suffix == QWORD_MNEM_SUFFIX
		      || (!i.suffix && i.tm.opcode_modifier.no_lsuf)
		      || dot_insn ()))
		sign = 1;
	      else
		sign = 0;

	      p = frag_more (size);
	      reloc_type = reloc (size, 0, sign, i.reloc[n]);

	      /*   This is tough to explain.  We end up with this one if we
	       * have operands that look like
	       * "_GLOBAL_OFFSET_TABLE_+[.-.L284]".  The goal here is to
	       * obtain the absolute address of the GOT, and it is strongly
	       * preferable from a performance point of view to avoid using
	       * a runtime relocation for this.  The actual sequence of
	       * instructions often look something like:
	       *
	       *	call	.L66
	       * .L66:
	       *	popl	%ebx
	       *	addl	$_GLOBAL_OFFSET_TABLE_+[.-.L66],%ebx
	       *
	       *   The call and pop essentially return the absolute address
	       * of the label .L66 and store it in %ebx.  The linker itself
	       * will ultimately change the first operand of the addl so
	       * that %ebx points to the GOT, but to keep things simple, the
	       * .o file must have this operand set so that it generates not
	       * the absolute address of .L66, but the absolute address of
	       * itself.  This allows the linker itself simply treat a GOTPC
	       * relocation as asking for a pcrel offset to the GOT to be
	       * added in, and the addend of the relocation is stored in the
	       * operand field for the instruction itself.
	       *
	       *   Our job here is to fix the operand so that it would add
	       * the correct offset so that %ebx would point to itself.  The
	       * thing that is tricky is that .-.L66 will point to the
	       * beginning of the instruction, so we need to further modify
	       * the operand so that it will point to itself.  There are
	       * other cases where you have something like:
	       *
	       *	.long	$_GLOBAL_OFFSET_TABLE_+[.-.L66]
	       *
	       * and here no correction would be required.  Internally in
	       * the assembler we treat operands of this form as not being
	       * pcrel since the '.' is explicitly mentioned, and I wonder
	       * whether it would simplify matters to do it this way.  Who
	       * knows.  In earlier versions of the PIC patches, the
	       * pcrel_adjust field was used to store the correction, but
	       * since the expression is not pcrel, I felt it would be
	       * confusing to do it this way.  */

	      if ((reloc_type == BFD_RELOC_32
		   || reloc_type == BFD_RELOC_X86_64_32S
		   || reloc_type == BFD_RELOC_64)
		  && GOT_symbol
		  && GOT_symbol == i.op[n].imms->X_add_symbol
		  && (i.op[n].imms->X_op == O_symbol
		      || (i.op[n].imms->X_op == O_add
			  && ((symbol_get_value_expression
			       (i.op[n].imms->X_op_symbol)->X_op)
			      == O_subtract))))
		{
		  if (!object_64bit)
		    reloc_type = BFD_RELOC_386_GOTPC;
		  else if (size == 4)
		    reloc_type = BFD_RELOC_X86_64_GOTPC32;
		  else if (size == 8)
		    reloc_type = BFD_RELOC_X86_64_GOTPC64;
		  i.has_gotpc_tls_reloc = true;
		  i.op[n].imms->X_add_number +=
		    encoding_length (insn_start_frag, insn_start_off, p);
		}
	      fix_new_exp (frag_now, p - frag_now->fr_literal, size,
			   i.op[n].imms, 0, reloc_type);
	    }
	}
    }
}

/* x86_cons_fix_new is called via the expression parsing code when a
   reloc is needed.  We use this hook to get the correct .got reloc.  */
static int cons_sign = -1;

void
x86_cons_fix_new (fragS *frag, unsigned int off, unsigned int len,
		  expressionS *exp, bfd_reloc_code_real_type r)
{
  r = reloc (len, 0, cons_sign, r);

#ifdef TE_PE
  if (exp->X_op == O_secrel)
    {
      exp->X_op = O_symbol;
      r = BFD_RELOC_32_SECREL;
    }
  else if (exp->X_op == O_secidx)
    r = BFD_RELOC_16_SECIDX;
#endif

  fix_new_exp (frag, off, len, exp, 0, r);
}

/* Export the ABI address size for use by TC_ADDRESS_BYTES for the
   purpose of the `.dc.a' internal pseudo-op.  */

int
x86_address_bytes (void)
{
  if ((stdoutput->arch_info->mach & bfd_mach_x64_32))
    return 4;
  return stdoutput->arch_info->bits_per_address / 8;
}

#if (!(defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF) || defined (OBJ_MACH_O)) \
     || defined (LEX_AT)) && !defined (TE_PE)
# define lex_got(reloc, adjust, types) NULL
#else
/* Parse operands of the form
   <symbol>@GOTOFF+<nnn>
   and similar .plt or .got references.

   If we find one, set up the correct relocation in RELOC and copy the
   input string, minus the `@GOTOFF' into a malloc'd buffer for
   parsing by the calling routine.  Return this buffer, and if ADJUST
   is non-null set it to the length of the string we removed from the
   input line.  Otherwise return NULL.  */
static char *
lex_got (enum bfd_reloc_code_real *rel,
	 int *adjust,
	 i386_operand_type *types)
{
  /* Some of the relocations depend on the size of what field is to
     be relocated.  But in our callers i386_immediate and i386_displacement
     we don't yet know the operand size (this will be set by insn
     matching).  Hence we record the word32 relocation here,
     and adjust the reloc according to the real size in reloc().  */
  static const struct
  {
    const char *str;
    int len;
    const enum bfd_reloc_code_real rel[2];
    const i386_operand_type types64;
    bool need_GOT_symbol;
  }
    gotrel[] =
  {

#define OPERAND_TYPE_IMM32_32S_DISP32 { .bitfield = \
  { .imm32 = 1, .imm32s = 1, .disp32 = 1 } }
#define OPERAND_TYPE_IMM32_32S_64_DISP32 { .bitfield = \
  { .imm32 = 1, .imm32s = 1, .imm64 = 1, .disp32 = 1 } }
#define OPERAND_TYPE_IMM32_32S_64_DISP32_64 { .bitfield = \
  { .imm32 = 1, .imm32s = 1, .imm64 = 1, .disp32 = 1, .disp64 = 1 } }
#define OPERAND_TYPE_IMM64_DISP64 { .bitfield = \
  { .imm64 = 1, .disp64 = 1 } }

#ifndef TE_PE
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
    { STRING_COMMA_LEN ("SIZE"),      { BFD_RELOC_SIZE32,
					BFD_RELOC_SIZE32 },
      { .bitfield = { .imm32 = 1, .imm64 = 1 } }, false },
#endif
    { STRING_COMMA_LEN ("PLTOFF"),   { _dummy_first_bfd_reloc_code_real,
				       BFD_RELOC_X86_64_PLTOFF64 },
      { .bitfield = { .imm64 = 1 } }, true },
    { STRING_COMMA_LEN ("PLT"),      { BFD_RELOC_386_PLT32,
				       BFD_RELOC_X86_64_PLT32    },
      OPERAND_TYPE_IMM32_32S_DISP32, false },
    { STRING_COMMA_LEN ("GOTPLT"),   { _dummy_first_bfd_reloc_code_real,
				       BFD_RELOC_X86_64_GOTPLT64 },
      OPERAND_TYPE_IMM64_DISP64, true },
    { STRING_COMMA_LEN ("GOTOFF"),   { BFD_RELOC_386_GOTOFF,
				       BFD_RELOC_X86_64_GOTOFF64 },
      OPERAND_TYPE_IMM64_DISP64, true },
    { STRING_COMMA_LEN ("GOTPCREL"), { _dummy_first_bfd_reloc_code_real,
				       BFD_RELOC_X86_64_GOTPCREL },
      OPERAND_TYPE_IMM32_32S_DISP32, true },
    { STRING_COMMA_LEN ("TLSGD"),    { BFD_RELOC_386_TLS_GD,
				       BFD_RELOC_X86_64_TLSGD    },
      OPERAND_TYPE_IMM32_32S_DISP32, true },
    { STRING_COMMA_LEN ("TLSLDM"),   { BFD_RELOC_386_TLS_LDM,
				       _dummy_first_bfd_reloc_code_real },
      OPERAND_TYPE_NONE, true },
    { STRING_COMMA_LEN ("TLSLD"),    { _dummy_first_bfd_reloc_code_real,
				       BFD_RELOC_X86_64_TLSLD    },
      OPERAND_TYPE_IMM32_32S_DISP32, true },
    { STRING_COMMA_LEN ("GOTTPOFF"), { BFD_RELOC_386_TLS_IE_32,
				       BFD_RELOC_X86_64_GOTTPOFF },
      OPERAND_TYPE_IMM32_32S_DISP32, true },
    { STRING_COMMA_LEN ("TPOFF"),    { BFD_RELOC_386_TLS_LE_32,
				       BFD_RELOC_X86_64_TPOFF32  },
      OPERAND_TYPE_IMM32_32S_64_DISP32_64, true },
    { STRING_COMMA_LEN ("NTPOFF"),   { BFD_RELOC_386_TLS_LE,
				       _dummy_first_bfd_reloc_code_real },
      OPERAND_TYPE_NONE, true },
    { STRING_COMMA_LEN ("DTPOFF"),   { BFD_RELOC_386_TLS_LDO_32,
				       BFD_RELOC_X86_64_DTPOFF32 },
      OPERAND_TYPE_IMM32_32S_64_DISP32_64, true },
    { STRING_COMMA_LEN ("GOTNTPOFF"),{ BFD_RELOC_386_TLS_GOTIE,
				       _dummy_first_bfd_reloc_code_real },
      OPERAND_TYPE_NONE, true },
    { STRING_COMMA_LEN ("INDNTPOFF"),{ BFD_RELOC_386_TLS_IE,
				       _dummy_first_bfd_reloc_code_real },
      OPERAND_TYPE_NONE, true },
    { STRING_COMMA_LEN ("GOT"),      { BFD_RELOC_386_GOT32,
				       BFD_RELOC_X86_64_GOT32    },
      OPERAND_TYPE_IMM32_32S_64_DISP32, true },
    { STRING_COMMA_LEN ("TLSDESC"),  { BFD_RELOC_386_TLS_GOTDESC,
				       BFD_RELOC_X86_64_GOTPC32_TLSDESC },
      OPERAND_TYPE_IMM32_32S_DISP32, true },
    { STRING_COMMA_LEN ("TLSCALL"),  { BFD_RELOC_386_TLS_DESC_CALL,
				       BFD_RELOC_X86_64_TLSDESC_CALL },
      OPERAND_TYPE_IMM32_32S_DISP32, true },
#else /* TE_PE */
    { STRING_COMMA_LEN ("SECREL32"), { BFD_RELOC_32_SECREL,
				       BFD_RELOC_32_SECREL },
      OPERAND_TYPE_IMM32_32S_64_DISP32_64, false },
#endif

#undef OPERAND_TYPE_IMM32_32S_DISP32
#undef OPERAND_TYPE_IMM32_32S_64_DISP32
#undef OPERAND_TYPE_IMM32_32S_64_DISP32_64
#undef OPERAND_TYPE_IMM64_DISP64

  };
  char *cp;
  unsigned int j;

#if defined (OBJ_MAYBE_ELF) && !defined (TE_PE)
  if (!IS_ELF)
    return NULL;
#endif

  for (cp = input_line_pointer; *cp != '@'; cp++)
    if (is_end_of_line[(unsigned char) *cp] || *cp == ',')
      return NULL;

  for (j = 0; j < ARRAY_SIZE (gotrel); j++)
    {
      int len = gotrel[j].len;
      if (strncasecmp (cp + 1, gotrel[j].str, len) == 0)
	{
	  if (gotrel[j].rel[object_64bit] != 0)
	    {
	      int first, second;
	      char *tmpbuf, *past_reloc;

	      *rel = gotrel[j].rel[object_64bit];

	      if (types)
		{
		  if (flag_code != CODE_64BIT)
		    {
		      types->bitfield.imm32 = 1;
		      types->bitfield.disp32 = 1;
		    }
		  else
		    *types = gotrel[j].types64;
		}

	      if (gotrel[j].need_GOT_symbol && GOT_symbol == NULL)
		GOT_symbol = symbol_find_or_make (GLOBAL_OFFSET_TABLE_NAME);

	      /* The length of the first part of our input line.  */
	      first = cp - input_line_pointer;

	      /* The second part goes from after the reloc token until
		 (and including) an end_of_line char or comma.  */
	      past_reloc = cp + 1 + len;
	      cp = past_reloc;
	      while (!is_end_of_line[(unsigned char) *cp] && *cp != ',')
		++cp;
	      second = cp + 1 - past_reloc;

	      /* Allocate and copy string.  The trailing NUL shouldn't
		 be necessary, but be safe.  */
	      tmpbuf = XNEWVEC (char, first + second + 2);
	      memcpy (tmpbuf, input_line_pointer, first);
	      if (second != 0 && *past_reloc != ' ')
		/* Replace the relocation token with ' ', so that
		   errors like foo@GOTOFF1 will be detected.  */
		tmpbuf[first++] = ' ';
	      else
		/* Increment length by 1 if the relocation token is
		   removed.  */
		len++;
	      if (adjust)
		*adjust = len;
	      memcpy (tmpbuf + first, past_reloc, second);
	      tmpbuf[first + second] = '\0';
	      return tmpbuf;
	    }

	  as_bad (_("@%s reloc is not supported with %d-bit output format"),
		  gotrel[j].str, 1 << (5 + object_64bit));
	  return NULL;
	}
    }

  /* Might be a symbol version string.  Don't as_bad here.  */
  return NULL;
}
#endif

bfd_reloc_code_real_type
x86_cons (expressionS *exp, int size)
{
  bfd_reloc_code_real_type got_reloc = NO_RELOC;

  intel_syntax = -intel_syntax;
  exp->X_md = 0;
  expr_mode = expr_operator_none;

#if ((defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)) \
      && !defined (LEX_AT)) \
    || defined (TE_PE)
  if (size == 4 || (object_64bit && size == 8))
    {
      /* Handle @GOTOFF and the like in an expression.  */
      char *save;
      char *gotfree_input_line;
      int adjust = 0;

      save = input_line_pointer;
      gotfree_input_line = lex_got (&got_reloc, &adjust, NULL);
      if (gotfree_input_line)
	input_line_pointer = gotfree_input_line;

      expression (exp);

      if (gotfree_input_line)
	{
	  /* expression () has merrily parsed up to the end of line,
	     or a comma - in the wrong buffer.  Transfer how far
	     input_line_pointer has moved to the right buffer.  */
	  input_line_pointer = (save
				+ (input_line_pointer - gotfree_input_line)
				+ adjust);
	  free (gotfree_input_line);
	  if (exp->X_op == O_constant
	      || exp->X_op == O_absent
	      || exp->X_op == O_illegal
	      || exp->X_op == O_register
	      || exp->X_op == O_big)
	    {
	      char c = *input_line_pointer;
	      *input_line_pointer = 0;
	      as_bad (_("missing or invalid expression `%s'"), save);
	      *input_line_pointer = c;
	    }
	  else if ((got_reloc == BFD_RELOC_386_PLT32
		    || got_reloc == BFD_RELOC_X86_64_PLT32)
		   && exp->X_op != O_symbol)
	    {
	      char c = *input_line_pointer;
	      *input_line_pointer = 0;
	      as_bad (_("invalid PLT expression `%s'"), save);
	      *input_line_pointer = c;
	    }
	}
    }
  else
#endif
    expression (exp);

  intel_syntax = -intel_syntax;

  if (intel_syntax)
    i386_intel_simplify (exp);

  /* If not 64bit, massage value, to account for wraparound when !BFD64.  */
  if (size <= 4 && expr_mode == expr_operator_present
      && exp->X_op == O_constant && !object_64bit)
    exp->X_add_number = extend_to_32bit_address (exp->X_add_number);

  return got_reloc;
}

static void
signed_cons (int size)
{
  if (object_64bit)
    cons_sign = 1;
  cons (size);
  cons_sign = -1;
}

static void
s_insn (int dummy ATTRIBUTE_UNUSED)
{
  char mnemonic[MAX_MNEM_SIZE], *line = input_line_pointer, *ptr;
  char *saved_ilp = find_end_of_line (line, false), saved_char;
  const char *end;
  unsigned int j;
  valueT val;
  bool vex = false, xop = false, evex = false;
  static const templates tt = { &i.tm, &i.tm + 1 };

  init_globals ();

  saved_char = *saved_ilp;
  *saved_ilp = 0;

  end = parse_insn (line, mnemonic, true);
  if (end == NULL)
    {
  bad:
      *saved_ilp = saved_char;
      ignore_rest_of_line ();
      i.tm.mnem_off = 0;
      return;
    }
  line += end - line;

  current_templates = &tt;
  i.tm.mnem_off = MN__insn;
  i.tm.extension_opcode = None;

  if (startswith (line, "VEX")
      && (line[3] == '.' || is_space_char (line[3])))
    {
      vex = true;
      line += 3;
    }
  else if (startswith (line, "XOP") && ISDIGIT (line[3]))
    {
      char *e;
      unsigned long n = strtoul (line + 3, &e, 16);

      if (e == line + 5 && n >= 0x08 && n <= 0x1f
	  && (*e == '.' || is_space_char (*e)))
	{
	  xop = true;
	  /* Arrange for build_vex_prefix() to emit 0x8f.  */
	  i.tm.opcode_space = SPACE_XOP08;
	  i.insn_opcode_space = n;
	  line = e;
	}
    }
  else if (startswith (line, "EVEX")
	   && (line[4] == '.' || is_space_char (line[4])))
    {
      evex = true;
      line += 4;
    }

  if (vex || xop
      ? i.vec_encoding == vex_encoding_evex
      : evex
	? i.vec_encoding == vex_encoding_vex
	  || i.vec_encoding == vex_encoding_vex3
	: i.vec_encoding != vex_encoding_default)
    {
      as_bad (_("pseudo-prefix conflicts with encoding specifier"));
      goto bad;
    }

  if (line > end && i.vec_encoding == vex_encoding_default)
    i.vec_encoding = evex ? vex_encoding_evex : vex_encoding_vex;

  if (line > end && *line == '.')
    {
      /* Length specifier (VEX.L, XOP.L, EVEX.L'L).  */
      switch (line[1])
	{
	case 'L':
	  switch (line[2])
	    {
	    case '0':
	      if (evex)
		i.tm.opcode_modifier.evex = EVEX128;
	      else
		i.tm.opcode_modifier.vex = VEX128;
	      break;

	    case '1':
	      if (evex)
		i.tm.opcode_modifier.evex = EVEX256;
	      else
		i.tm.opcode_modifier.vex = VEX256;
	      break;

	    case '2':
	      if (evex)
		i.tm.opcode_modifier.evex = EVEX512;
	      break;

	    case '3':
	      if (evex)
		i.tm.opcode_modifier.evex = EVEX_L3;
	      break;

	    case 'I':
	      if (line[3] == 'G')
		{
		  if (evex)
		    i.tm.opcode_modifier.evex = EVEXLIG;
		  else
		    i.tm.opcode_modifier.vex = VEXScalar; /* LIG */
		  ++line;
		}
	      break;
	    }

	  if (i.tm.opcode_modifier.vex || i.tm.opcode_modifier.evex)
	    line += 3;
	  break;

	case '1':
	  if (line[2] == '2' && line[3] == '8')
	    {
	      if (evex)
		i.tm.opcode_modifier.evex = EVEX128;
	      else
		i.tm.opcode_modifier.vex = VEX128;
	      line += 4;
	    }
	  break;

	case '2':
	  if (line[2] == '5' && line[3] == '6')
	    {
	      if (evex)
		i.tm.opcode_modifier.evex = EVEX256;
	      else
		i.tm.opcode_modifier.vex = VEX256;
	      line += 4;
	    }
	  break;

	case '5':
	  if (evex && line[2] == '1' && line[3] == '2')
	    {
	      i.tm.opcode_modifier.evex = EVEX512;
	      line += 4;
	    }
	  break;
	}
    }

  if (line > end && *line == '.')
    {
      /* embedded prefix (VEX.pp, XOP.pp, EVEX.pp).  */
      switch (line[1])
	{
	case 'N':
	  if (line[2] == 'P')
	    line += 3;
	  break;

	case '6':
	  if (line[2] == '6')
	    {
	      i.tm.opcode_modifier.opcodeprefix = PREFIX_0X66;
	      line += 3;
	    }
	  break;

	case 'F': case 'f':
	  if (line[2] == '3')
	    {
	      i.tm.opcode_modifier.opcodeprefix = PREFIX_0XF3;
	      line += 3;
	    }
	  else if (line[2] == '2')
	    {
	      i.tm.opcode_modifier.opcodeprefix = PREFIX_0XF2;
	      line += 3;
	    }
	  break;
	}
    }

  if (line > end && !xop && *line == '.')
    {
      /* Encoding space (VEX.mmmmm, EVEX.mmmm).  */
      switch (line[1])
	{
	case '0':
	  if (TOUPPER (line[2]) != 'F')
	    break;
	  if (line[3] == '.' || is_space_char (line[3]))
	    {
	      i.insn_opcode_space = SPACE_0F;
	      line += 3;
	    }
	  else if (line[3] == '3'
		   && (line[4] == '8' || TOUPPER (line[4]) == 'A')
		   && (line[5] == '.' || is_space_char (line[5])))
	    {
	      i.insn_opcode_space = line[4] == '8' ? SPACE_0F38 : SPACE_0F3A;
	      line += 5;
	    }
	  break;

	case 'M':
	  if (ISDIGIT (line[2]) && line[2] != '0')
	    {
	      char *e;
	      unsigned long n = strtoul (line + 2, &e, 10);

	      if (n <= (evex ? 15 : 31)
		  && (*e == '.' || is_space_char (*e)))
		{
		  i.insn_opcode_space = n;
		  line = e;
		}
	    }
	  break;
	}
    }

  if (line > end && *line == '.' && line[1] == 'W')
    {
      /* VEX.W, XOP.W, EVEX.W  */
      switch (line[2])
	{
	case '0':
	  i.tm.opcode_modifier.vexw = VEXW0;
	  break;

	case '1':
	  i.tm.opcode_modifier.vexw = VEXW1;
	  break;

	case 'I':
	  if (line[3] == 'G')
	    {
	      i.tm.opcode_modifier.vexw = VEXWIG;
	      ++line;
	    }
	  break;
	}

      if (i.tm.opcode_modifier.vexw)
	line += 3;
    }

  if (line > end && *line && !is_space_char (*line))
    {
      /* Improve diagnostic a little.  */
      if (*line == '.' && line[1] && !is_space_char (line[1]))
	++line;
      goto done;
    }

  /* Before processing the opcode expression, find trailing "+r" or
     "/<digit>" specifiers.  */
  for (ptr = line; ; ++ptr)
    {
      unsigned long n;
      char *e;

      ptr = strpbrk (ptr, "+/,");
      if (ptr == NULL || *ptr == ',')
	break;

      if (*ptr == '+' && ptr[1] == 'r'
	  && (ptr[2] == ',' || (is_space_char (ptr[2]) && ptr[3] == ',')))
	{
	  *ptr = ' ';
	  ptr[1] = ' ';
	  i.short_form = true;
	  break;
	}

      if (*ptr == '/' && ISDIGIT (ptr[1])
	  && (n = strtoul (ptr + 1, &e, 8)) < 8
	  && e == ptr + 2
	  && (ptr[2] == ',' || (is_space_char (ptr[2]) && ptr[3] == ',')))
	{
	  *ptr = ' ';
	  ptr[1] = ' ';
	  i.tm.extension_opcode = n;
	  i.tm.opcode_modifier.modrm = 1;
	  break;
	}
    }

  input_line_pointer = line;
  val = get_absolute_expression ();
  line = input_line_pointer;

  if (i.short_form && (val & 7))
    as_warn ("`+r' assumes low three opcode bits to be clear");

  for (j = 1; j < sizeof(val); ++j)
    if (!(val >> (j * 8)))
      break;

  /* Trim off a prefix if present.  */
  if (j > 1 && !vex && !xop && !evex)
    {
      uint8_t byte = val >> ((j - 1) * 8);

      switch (byte)
	{
	case DATA_PREFIX_OPCODE:
	case REPE_PREFIX_OPCODE:
	case REPNE_PREFIX_OPCODE:
	  if (!add_prefix (byte))
	    goto bad;
	  val &= ((uint64_t)1 << (--j * 8)) - 1;
	  break;
	}
    }

  /* Trim off encoding space.  */
  if (j > 1 && !i.insn_opcode_space && (val >> ((j - 1) * 8)) == 0x0f)
    {
      uint8_t byte = val >> ((--j - 1) * 8);

      i.insn_opcode_space = SPACE_0F;
      switch (byte & -(j > 1))
	{
	case 0x38:
	  i.insn_opcode_space = SPACE_0F38;
	  --j;
	  break;
	case 0x3a:
	  i.insn_opcode_space = SPACE_0F3A;
	  --j;
	  break;
	}
      i.tm.opcode_space = i.insn_opcode_space;
      val &= ((uint64_t)1 << (j * 8)) - 1;
    }
  if (!i.tm.opcode_space && (vex || evex))
    /* Arrange for build_vex_prefix() to properly emit 0xC4/0xC5.
       Also avoid hitting abort() there or in build_evex_prefix().  */
    i.tm.opcode_space = i.insn_opcode_space == SPACE_0F ? SPACE_0F
						   : SPACE_0F38;

  if (j > 2)
    {
      as_bad (_("opcode residual (%#"PRIx64") too wide"), (uint64_t) val);
      goto bad;
    }
  i.opcode_length = j;

  /* Handle operands, if any.  */
  if (*line == ',')
    {
      i386_operand_type combined;
      expressionS *disp_exp = NULL;
      bool changed;

      i.memshift = -1;

      ptr = parse_operands (line + 1, &i386_mnemonics[MN__insn]);
      this_operand = -1;
      if (!ptr)
	goto bad;
      line = ptr;

      if (!i.operands)
	{
	  as_bad (_("expecting operand after ','; got nothing"));
	  goto done;
	}

      if (i.mem_operands > 1)
	{
	  as_bad (_("too many memory references for `%s'"),
		  &i386_mnemonics[MN__insn]);
	  goto done;
	}

      /* Are we to emit ModR/M encoding?  */
      if (!i.short_form
	  && (i.mem_operands
	      || i.reg_operands > (i.vec_encoding != vex_encoding_default)
	      || i.tm.extension_opcode != None))
	i.tm.opcode_modifier.modrm = 1;

      if (!i.tm.opcode_modifier.modrm
	  && (i.reg_operands
	      > i.short_form + 0U + (i.vec_encoding != vex_encoding_default)
	      || i.mem_operands))
	{
	  as_bad (_("too many register/memory operands"));
	  goto done;
	}

      /* Enforce certain constraints on operands.  */
      switch (i.reg_operands + i.mem_operands
	      + (i.tm.extension_opcode != None))
	{
	case 0:
	  if (i.short_form)
	    {
	      as_bad (_("too few register/memory operands"));
	      goto done;
	    }
	  /* Fall through.  */
	case 1:
	  if (i.tm.opcode_modifier.modrm)
	    {
	      as_bad (_("too few register/memory operands"));
	      goto done;
	    }
	  break;

	case 2:
	  break;

	case 4:
	  if (i.imm_operands
	      && (i.op[0].imms->X_op != O_constant
		  || !fits_in_imm4 (i.op[0].imms->X_add_number)))
	    {
	      as_bad (_("constant doesn't fit in %d bits"), evex ? 3 : 4);
	      goto done;
	    }
	  /* Fall through.  */
	case 3:
	  if (i.vec_encoding != vex_encoding_default)
	    {
	      i.tm.opcode_modifier.vexvvvv = 1;
	      break;
	    }
	  /* Fall through.  */
	default:
	  as_bad (_("too many register/memory operands"));
	  goto done;
	}

      /* Bring operands into canonical order (imm, mem, reg).  */
      do
	{
	  changed = false;

	  for (j = 1; j < i.operands; ++j)
	    {
	      if ((!operand_type_check (i.types[j - 1], imm)
		   && operand_type_check (i.types[j], imm))
		  || (i.types[j - 1].bitfield.class != ClassNone
		      && i.types[j].bitfield.class == ClassNone))
		{
		  swap_2_operands (j - 1, j);
		  changed = true;
		}
	    }
	}
      while (changed);

      /* For Intel syntax swap the order of register operands.  */
      if (intel_syntax)
	switch (i.reg_operands)
	  {
	  case 0:
	  case 1:
	    break;

	  case 4:
	    swap_2_operands (i.imm_operands + i.mem_operands + 1, i.operands - 2);
	    /* Fall through.  */
	  case 3:
	  case 2:
	    swap_2_operands (i.imm_operands + i.mem_operands, i.operands - 1);
	    break;

	  default:
	    abort ();
	  }

      /* Enforce constraints when using VSIB.  */
      if (i.index_reg
	  && (i.index_reg->reg_type.bitfield.xmmword
	      || i.index_reg->reg_type.bitfield.ymmword
	      || i.index_reg->reg_type.bitfield.zmmword))
	{
	  if (i.vec_encoding == vex_encoding_default)
	    {
	      as_bad (_("VSIB unavailable with legacy encoding"));
	      goto done;
	    }

	  if (i.vec_encoding == vex_encoding_evex
	      && i.reg_operands > 1)
	    {
	      /* We could allow two register operands, encoding the 2nd one in
		 an 8-bit immediate like for 4-register-operand insns, but that
		 would require ugly fiddling with process_operands() and/or
		 build_modrm_byte().  */
	      as_bad (_("too many register operands with VSIB"));
	      goto done;
	    }

	  i.tm.opcode_modifier.sib = 1;
	}

      /* Establish operand size encoding.  */
      operand_type_set (&combined, 0);

      for (j = i.imm_operands; j < i.operands; ++j)
	{
	  i.types[j].bitfield.instance = InstanceNone;

	  if (operand_type_check (i.types[j], disp))
	    {
	      i.types[j].bitfield.baseindex = 1;
	      disp_exp = i.op[j].disps;
	    }

	  if (evex && i.types[j].bitfield.baseindex)
	    {
	      unsigned int n = i.memshift;

	      if (i.types[j].bitfield.byte)
		n = 0;
	      else if (i.types[j].bitfield.word)
		n = 1;
	      else if (i.types[j].bitfield.dword)
		n = 2;
	      else if (i.types[j].bitfield.qword)
		n = 3;
	      else if (i.types[j].bitfield.xmmword)
		n = 4;
	      else if (i.types[j].bitfield.ymmword)
		n = 5;
	      else if (i.types[j].bitfield.zmmword)
		n = 6;

	      if (i.memshift < 32 && n != i.memshift)
		as_warn ("conflicting memory operand size specifiers");
	      i.memshift = n;
	    }

	  if ((i.broadcast.type || i.broadcast.bytes)
	      && j == i.broadcast.operand)
	    continue;

	  combined = operand_type_or (combined, i.types[j]);
	  combined.bitfield.class = ClassNone;
	}

      switch ((i.broadcast.type ? i.broadcast.type : 1)
	      << (i.memshift < 32 ? i.memshift : 0))
	{
	case 64: combined.bitfield.zmmword = 1; break;
	case 32: combined.bitfield.ymmword = 1; break;
	case 16: combined.bitfield.xmmword = 1; break;
	case  8: combined.bitfield.qword = 1; break;
	case  4: combined.bitfield.dword = 1; break;
	}

      if (i.vec_encoding == vex_encoding_default)
	{
	  if (flag_code == CODE_64BIT && combined.bitfield.qword)
	    i.rex |= REX_W;
	  else if ((flag_code == CODE_16BIT ? combined.bitfield.dword
					    : combined.bitfield.word)
	           && !add_prefix (DATA_PREFIX_OPCODE))
	    goto done;
	}
      else if (!i.tm.opcode_modifier.vexw)
	{
	  if (flag_code == CODE_64BIT)
	    {
	      if (combined.bitfield.qword)
	        i.tm.opcode_modifier.vexw = VEXW1;
	      else if (combined.bitfield.dword)
	        i.tm.opcode_modifier.vexw = VEXW0;
	    }

	  if (!i.tm.opcode_modifier.vexw)
	    i.tm.opcode_modifier.vexw = VEXWIG;
	}

      if (vex || xop)
	{
	  if (!i.tm.opcode_modifier.vex)
	    {
	      if (combined.bitfield.ymmword)
	        i.tm.opcode_modifier.vex = VEX256;
	      else if (combined.bitfield.xmmword)
	        i.tm.opcode_modifier.vex = VEX128;
	    }
	}
      else if (evex)
	{
	  if (!i.tm.opcode_modifier.evex)
	    {
	      /* Do _not_ consider AVX512VL here.  */
	      if (i.rounding.type != rc_none || combined.bitfield.zmmword)
	        i.tm.opcode_modifier.evex = EVEX512;
	      else if (combined.bitfield.ymmword)
	        i.tm.opcode_modifier.evex = EVEX256;
	      else if (combined.bitfield.xmmword)
	        i.tm.opcode_modifier.evex = EVEX128;
	    }

	  if (i.memshift >= 32)
	    {
	      unsigned int n = 0;

	      switch (i.tm.opcode_modifier.evex)
		{
		case EVEX512: n = 64; break;
		case EVEX256: n = 32; break;
		case EVEX128: n = 16; break;
		}

	      if (i.broadcast.type)
		n /= i.broadcast.type;

	      if (n > 0)
		for (i.memshift = 0; !(n & 1); n >>= 1)
		  ++i.memshift;
	      else if (disp_exp != NULL && disp_exp->X_op == O_constant
		       && disp_exp->X_add_number != 0
		       && i.disp_encoding != disp_encoding_32bit)
		{
		  if (!quiet_warnings)
		    as_warn ("cannot determine memory operand size");
		  i.disp_encoding = disp_encoding_32bit;
		}
	    }
	}

      if (i.memshift >= 32)
	i.memshift = 0;
      else if (!evex)
	i.vec_encoding = vex_encoding_error;

      if (i.disp_operands && !optimize_disp (&i.tm))
	goto done;

      /* Establish size for immediate operands.  */
      for (j = 0; j < i.imm_operands; ++j)
	{
	  expressionS *expP = i.op[j].imms;

	  gas_assert (operand_type_check (i.types[j], imm));
	  operand_type_set (&i.types[j], 0);

	  if (i.imm_bits[j] > 32)
	    i.types[j].bitfield.imm64 = 1;
	  else if (i.imm_bits[j] > 16)
	    {
	      if (flag_code == CODE_64BIT && (i.flags[j] & Operand_Signed))
		i.types[j].bitfield.imm32s = 1;
	      else
		i.types[j].bitfield.imm32 = 1;
	    }
	  else if (i.imm_bits[j] > 8)
	    i.types[j].bitfield.imm16 = 1;
	  else if (i.imm_bits[j] > 0)
	    {
	      if (i.flags[j] & Operand_Signed)
		i.types[j].bitfield.imm8s = 1;
	      else
		i.types[j].bitfield.imm8 = 1;
	    }
	  else if (expP->X_op == O_constant)
	    {
	      i.types[j] = smallest_imm_type (expP->X_add_number);
	      i.types[j].bitfield.imm1 = 0;
	      /* Oddly enough imm_size() checks imm64 first, so the bit needs
		 zapping since smallest_imm_type() sets it unconditionally.  */
	      if (flag_code != CODE_64BIT)
		{
		  i.types[j].bitfield.imm64 = 0;
		  i.types[j].bitfield.imm32s = 0;
		  i.types[j].bitfield.imm32 = 1;
		}
	      else if (i.types[j].bitfield.imm32 || i.types[j].bitfield.imm32s)
		i.types[j].bitfield.imm64 = 0;
	    }
	  else
	    /* Non-constant expressions are sized heuristically.  */
	    switch (flag_code)
	      {
	      case CODE_64BIT: i.types[j].bitfield.imm32s = 1; break;
	      case CODE_32BIT: i.types[j].bitfield.imm32 = 1; break;
	      case CODE_16BIT: i.types[j].bitfield.imm16 = 1; break;
	      }
	}

      for (j = 0; j < i.operands; ++j)
	i.tm.operand_types[j] = i.types[j];

      process_operands ();
    }

  /* Don't set opcode until after processing operands, to avoid any
     potential special casing there.  */
  i.tm.base_opcode |= val;

  if (i.vec_encoding == vex_encoding_error
      || (i.vec_encoding != vex_encoding_evex
	  ? i.broadcast.type || i.broadcast.bytes
	    || i.rounding.type != rc_none
	    || i.mask.reg
	  : (i.broadcast.type || i.broadcast.bytes)
	    && i.rounding.type != rc_none))
    {
      as_bad (_("conflicting .insn operands"));
      goto done;
    }

  if (vex || xop)
    {
      if (!i.tm.opcode_modifier.vex)
	i.tm.opcode_modifier.vex = VEXScalar; /* LIG */

      build_vex_prefix (NULL);
      i.rex &= REX_OPCODE;
    }
  else if (evex)
    {
      if (!i.tm.opcode_modifier.evex)
	i.tm.opcode_modifier.evex = EVEXLIG;

      build_evex_prefix ();
      i.rex &= REX_OPCODE;
    }
  else if (i.rex != 0)
    add_prefix (REX_OPCODE | i.rex);

  output_insn ();

 done:
  *saved_ilp = saved_char;
  input_line_pointer = line;

  demand_empty_rest_of_line ();

  /* Make sure dot_insn() won't yield "true" anymore.  */
  i.tm.mnem_off = 0;
}

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

static void
pe_directive_secidx (int dummy ATTRIBUTE_UNUSED)
{
  expressionS exp;

  do
    {
      expression (&exp);
      if (exp.X_op == O_symbol)
	exp.X_op = O_secidx;

      emit_expr (&exp, 2);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;
  demand_empty_rest_of_line ();
}
#endif

/* Handle Rounding Control / SAE specifiers.  */

static char *
RC_SAE_specifier (const char *pstr)
{
  unsigned int j;

  for (j = 0; j < ARRAY_SIZE (RC_NamesTable); j++)
    {
      if (!strncmp (pstr, RC_NamesTable[j].name, RC_NamesTable[j].len))
	{
	  if (i.rounding.type != rc_none)
	    {
	      as_bad (_("duplicated `{%s}'"), RC_NamesTable[j].name);
	      return NULL;
	    }

	  i.rounding.type = RC_NamesTable[j].type;

	  return (char *)(pstr + RC_NamesTable[j].len);
	}
    }

  return NULL;
}

/* Handle Vector operations.  */

static char *
check_VecOperations (char *op_string)
{
  const reg_entry *mask;
  const char *saved;
  char *end_op;

  while (*op_string)
    {
      saved = op_string;
      if (*op_string == '{')
	{
	  op_string++;

	  /* Check broadcasts.  */
	  if (startswith (op_string, "1to"))
	    {
	      unsigned int bcst_type;

	      if (i.broadcast.type)
		goto duplicated_vec_op;

	      op_string += 3;
	      if (*op_string == '8')
		bcst_type = 8;
	      else if (*op_string == '4')
		bcst_type = 4;
	      else if (*op_string == '2')
		bcst_type = 2;
	      else if (*op_string == '1'
		       && *(op_string+1) == '6')
		{
		  bcst_type = 16;
		  op_string++;
		}
	      else if (*op_string == '3'
		       && *(op_string+1) == '2')
		{
		  bcst_type = 32;
		  op_string++;
		}
	      else
		{
		  as_bad (_("Unsupported broadcast: `%s'"), saved);
		  return NULL;
		}
	      op_string++;

	      i.broadcast.type = bcst_type;
	      i.broadcast.operand = this_operand;

	      /* For .insn a data size specifier may be appended.  */
	      if (dot_insn () && *op_string == ':')
		goto dot_insn_modifier;
	    }
	  /* Check .insn special cases.  */
	  else if (dot_insn () && *op_string == ':')
	    {
	    dot_insn_modifier:
	      switch (op_string[1])
		{
		  unsigned long n;

		case 'd':
		  if (i.memshift < 32)
		    goto duplicated_vec_op;

		  n = strtoul (op_string + 2, &end_op, 0);
		  if (n)
		    for (i.memshift = 0; !(n & 1); n >>= 1)
		      ++i.memshift;
		  if (i.memshift < 32 && n == 1)
		    op_string = end_op;
		  break;

		case 's': case 'u':
		  /* This isn't really a "vector" operation, but a sign/size
		     specifier for immediate operands of .insn.  Note that AT&T
		     syntax handles the same in i386_immediate().  */
		  if (!intel_syntax)
		    break;

		  if (i.imm_bits[this_operand])
		    goto duplicated_vec_op;

		  n = strtoul (op_string + 2, &end_op, 0);
		  if (n && n <= (flag_code == CODE_64BIT ? 64 : 32))
		    {
		      i.imm_bits[this_operand] = n;
		      if (op_string[1] == 's')
			i.flags[this_operand] |= Operand_Signed;
		      op_string = end_op;
		    }
		  break;
		}
	    }
	  /* Check masking operation.  */
	  else if ((mask = parse_register (op_string, &end_op)) != NULL)
	    {
	      if (mask == &bad_reg)
		return NULL;

	      /* k0 can't be used for write mask.  */
	      if (mask->reg_type.bitfield.class != RegMask || !mask->reg_num)
		{
		  as_bad (_("`%s%s' can't be used for write mask"),
			  register_prefix, mask->reg_name);
		  return NULL;
		}

	      if (!i.mask.reg)
		{
		  i.mask.reg = mask;
		  i.mask.operand = this_operand;
		}
	      else if (i.mask.reg->reg_num)
		goto duplicated_vec_op;
	      else
		{
		  i.mask.reg = mask;

		  /* Only "{z}" is allowed here.  No need to check
		     zeroing mask explicitly.  */
		  if (i.mask.operand != (unsigned int) this_operand)
		    {
		      as_bad (_("invalid write mask `%s'"), saved);
		      return NULL;
		    }
		}

	      op_string = end_op;
	    }
	  /* Check zeroing-flag for masking operation.  */
	  else if (*op_string == 'z')
	    {
	      if (!i.mask.reg)
		{
		  i.mask.reg = reg_k0;
		  i.mask.zeroing = 1;
		  i.mask.operand = this_operand;
		}
	      else
		{
		  if (i.mask.zeroing)
		    {
		    duplicated_vec_op:
		      as_bad (_("duplicated `%s'"), saved);
		      return NULL;
		    }

		  i.mask.zeroing = 1;

		  /* Only "{%k}" is allowed here.  No need to check mask
		     register explicitly.  */
		  if (i.mask.operand != (unsigned int) this_operand)
		    {
		      as_bad (_("invalid zeroing-masking `%s'"),
			      saved);
		      return NULL;
		    }
		}

	      op_string++;
	    }
	  else if (intel_syntax
		   && (op_string = RC_SAE_specifier (op_string)) != NULL)
	    i.rounding.modifier = true;
	  else
	    goto unknown_vec_op;

	  if (*op_string != '}')
	    {
	      as_bad (_("missing `}' in `%s'"), saved);
	      return NULL;
	    }
	  op_string++;

	  /* Strip whitespace since the addition of pseudo prefixes
	     changed how the scrubber treats '{'.  */
	  if (is_space_char (*op_string))
	    ++op_string;

	  continue;
	}
    unknown_vec_op:
      /* We don't know this one.  */
      as_bad (_("unknown vector operation: `%s'"), saved);
      return NULL;
    }

  if (i.mask.reg && i.mask.zeroing && !i.mask.reg->reg_num)
    {
      as_bad (_("zeroing-masking only allowed with write mask"));
      return NULL;
    }

  return op_string;
}

static int
i386_immediate (char *imm_start)
{
  char *save_input_line_pointer;
  char *gotfree_input_line;
  segT exp_seg = 0;
  expressionS *exp;
  i386_operand_type types;

  operand_type_set (&types, ~0);

  if (i.imm_operands == MAX_IMMEDIATE_OPERANDS)
    {
      as_bad (_("at most %d immediate operands are allowed"),
	      MAX_IMMEDIATE_OPERANDS);
      return 0;
    }

  exp = &im_expressions[i.imm_operands++];
  i.op[this_operand].imms = exp;

  if (is_space_char (*imm_start))
    ++imm_start;

  save_input_line_pointer = input_line_pointer;
  input_line_pointer = imm_start;

  gotfree_input_line = lex_got (&i.reloc[this_operand], NULL, &types);
  if (gotfree_input_line)
    input_line_pointer = gotfree_input_line;

  expr_mode = expr_operator_none;
  exp_seg = expression (exp);

  /* For .insn immediates there may be a size specifier.  */
  if (dot_insn () && *input_line_pointer == '{' && input_line_pointer[1] == ':'
      && (input_line_pointer[2] == 's' || input_line_pointer[2] == 'u'))
    {
      char *e;
      unsigned long n = strtoul (input_line_pointer + 3, &e, 0);

      if (*e == '}' && n && n <= (flag_code == CODE_64BIT ? 64 : 32))
	{
	  i.imm_bits[this_operand] = n;
	  if (input_line_pointer[2] == 's')
	    i.flags[this_operand] |= Operand_Signed;
	  input_line_pointer = e + 1;
	}
    }

  SKIP_WHITESPACE ();
  if (*input_line_pointer)
    as_bad (_("junk `%s' after expression"), input_line_pointer);

  input_line_pointer = save_input_line_pointer;
  if (gotfree_input_line)
    {
      free (gotfree_input_line);

      if (exp->X_op == O_constant)
	exp->X_op = O_illegal;
    }

  if (exp_seg == reg_section)
    {
      as_bad (_("illegal immediate register operand %s"), imm_start);
      return 0;
    }

  return i386_finalize_immediate (exp_seg, exp, types, imm_start);
}

static int
i386_finalize_immediate (segT exp_seg ATTRIBUTE_UNUSED, expressionS *exp,
			 i386_operand_type types, const char *imm_start)
{
  if (exp->X_op == O_absent || exp->X_op == O_illegal || exp->X_op == O_big)
    {
      if (imm_start)
	as_bad (_("missing or invalid immediate expression `%s'"),
		imm_start);
      return 0;
    }
  else if (exp->X_op == O_constant)
    {
      /* Size it properly later.  */
      i.types[this_operand].bitfield.imm64 = 1;

      /* If not 64bit, sign/zero extend val, to account for wraparound
	 when !BFD64.  */
      if (expr_mode == expr_operator_present
	  && flag_code != CODE_64BIT && !object_64bit)
	exp->X_add_number = extend_to_32bit_address (exp->X_add_number);
    }
#if (defined (OBJ_AOUT) || defined (OBJ_MAYBE_AOUT))
  else if (OUTPUT_FLAVOR == bfd_target_aout_flavour
	   && exp_seg != absolute_section
	   && exp_seg != text_section
	   && exp_seg != data_section
	   && exp_seg != bss_section
	   && exp_seg != undefined_section
	   && !bfd_is_com_section (exp_seg))
    {
      as_bad (_("unimplemented segment %s in operand"), exp_seg->name);
      return 0;
    }
#endif
  else
    {
      /* This is an address.  The size of the address will be
	 determined later, depending on destination register,
	 suffix, or the default for the section.  */
      i.types[this_operand].bitfield.imm8 = 1;
      i.types[this_operand].bitfield.imm16 = 1;
      i.types[this_operand].bitfield.imm32 = 1;
      i.types[this_operand].bitfield.imm32s = 1;
      i.types[this_operand].bitfield.imm64 = 1;
      i.types[this_operand] = operand_type_and (i.types[this_operand],
						types);
    }

  return 1;
}

static char *
i386_scale (char *scale)
{
  offsetT val;
  char *save = input_line_pointer;

  input_line_pointer = scale;
  val = get_absolute_expression ();

  switch (val)
    {
    case 1:
      i.log2_scale_factor = 0;
      break;
    case 2:
      i.log2_scale_factor = 1;
      break;
    case 4:
      i.log2_scale_factor = 2;
      break;
    case 8:
      i.log2_scale_factor = 3;
      break;
    default:
      {
	char sep = *input_line_pointer;

	*input_line_pointer = '\0';
	as_bad (_("expecting scale factor of 1, 2, 4, or 8: got `%s'"),
		scale);
	*input_line_pointer = sep;
	input_line_pointer = save;
	return NULL;
      }
    }
  if (i.log2_scale_factor != 0 && i.index_reg == 0)
    {
      as_warn (_("scale factor of %d without an index register"),
	       1 << i.log2_scale_factor);
      i.log2_scale_factor = 0;
    }
  scale = input_line_pointer;
  input_line_pointer = save;
  return scale;
}

static int
i386_displacement (char *disp_start, char *disp_end)
{
  expressionS *exp;
  segT exp_seg = 0;
  char *save_input_line_pointer;
  char *gotfree_input_line;
  int override;
  i386_operand_type bigdisp, types = anydisp;
  int ret;

  if (i.disp_operands == MAX_MEMORY_OPERANDS)
    {
      as_bad (_("at most %d displacement operands are allowed"),
	      MAX_MEMORY_OPERANDS);
      return 0;
    }

  operand_type_set (&bigdisp, 0);
  if (i.jumpabsolute
      || i.types[this_operand].bitfield.baseindex
      || (current_templates->start->opcode_modifier.jump != JUMP
	  && current_templates->start->opcode_modifier.jump != JUMP_DWORD))
    {
      i386_addressing_mode ();
      override = (i.prefix[ADDR_PREFIX] != 0);
      if (flag_code == CODE_64BIT)
	{
	  bigdisp.bitfield.disp32 = 1;
	  if (!override)
	    bigdisp.bitfield.disp64 = 1;
	}
      else if ((flag_code == CODE_16BIT) ^ override)
	  bigdisp.bitfield.disp16 = 1;
      else
	  bigdisp.bitfield.disp32 = 1;
    }
  else
    {
      /* For PC-relative branches, the width of the displacement may be
	 dependent upon data size, but is never dependent upon address size.
	 Also make sure to not unintentionally match against a non-PC-relative
	 branch template.  */
      static templates aux_templates;
      const insn_template *t = current_templates->start;
      bool has_intel64 = false;

      aux_templates.start = t;
      while (++t < current_templates->end)
	{
	  if (t->opcode_modifier.jump
	      != current_templates->start->opcode_modifier.jump)
	    break;
	  if ((t->opcode_modifier.isa64 >= INTEL64))
	    has_intel64 = true;
	}
      if (t < current_templates->end)
	{
	  aux_templates.end = t;
	  current_templates = &aux_templates;
	}

      override = (i.prefix[DATA_PREFIX] != 0);
      if (flag_code == CODE_64BIT)
	{
	  if ((override || i.suffix == WORD_MNEM_SUFFIX)
	      && (!intel64 || !has_intel64))
	    bigdisp.bitfield.disp16 = 1;
	  else
	    bigdisp.bitfield.disp32 = 1;
	}
      else
	{
	  if (!override)
	    override = (i.suffix == (flag_code != CODE_16BIT
				     ? WORD_MNEM_SUFFIX
				     : LONG_MNEM_SUFFIX));
	  bigdisp.bitfield.disp32 = 1;
	  if ((flag_code == CODE_16BIT) ^ override)
	    {
	      bigdisp.bitfield.disp32 = 0;
	      bigdisp.bitfield.disp16 = 1;
	    }
	}
    }
  i.types[this_operand] = operand_type_or (i.types[this_operand],
					   bigdisp);

  exp = &disp_expressions[i.disp_operands];
  i.op[this_operand].disps = exp;
  i.disp_operands++;
  save_input_line_pointer = input_line_pointer;
  input_line_pointer = disp_start;
  END_STRING_AND_SAVE (disp_end);

#ifndef GCC_ASM_O_HACK
#define GCC_ASM_O_HACK 0
#endif
#if GCC_ASM_O_HACK
  END_STRING_AND_SAVE (disp_end + 1);
  if (i.types[this_operand].bitfield.baseIndex
      && displacement_string_end[-1] == '+')
    {
      /* This hack is to avoid a warning when using the "o"
	 constraint within gcc asm statements.
	 For instance:

	 #define _set_tssldt_desc(n,addr,limit,type) \
	 __asm__ __volatile__ ( \
	 "movw %w2,%0\n\t" \
	 "movw %w1,2+%0\n\t" \
	 "rorl $16,%1\n\t" \
	 "movb %b1,4+%0\n\t" \
	 "movb %4,5+%0\n\t" \
	 "movb $0,6+%0\n\t" \
	 "movb %h1,7+%0\n\t" \
	 "rorl $16,%1" \
	 : "=o"(*(n)) : "q" (addr), "ri"(limit), "i"(type))

	 This works great except that the output assembler ends
	 up looking a bit weird if it turns out that there is
	 no offset.  You end up producing code that looks like:

	 #APP
	 movw $235,(%eax)
	 movw %dx,2+(%eax)
	 rorl $16,%edx
	 movb %dl,4+(%eax)
	 movb $137,5+(%eax)
	 movb $0,6+(%eax)
	 movb %dh,7+(%eax)
	 rorl $16,%edx
	 #NO_APP

	 So here we provide the missing zero.  */

      *displacement_string_end = '0';
    }
#endif
  gotfree_input_line = lex_got (&i.reloc[this_operand], NULL, &types);
  if (gotfree_input_line)
    input_line_pointer = gotfree_input_line;

  expr_mode = expr_operator_none;
  exp_seg = expression (exp);

  SKIP_WHITESPACE ();
  if (*input_line_pointer)
    as_bad (_("junk `%s' after expression"), input_line_pointer);
#if GCC_ASM_O_HACK
  RESTORE_END_STRING (disp_end + 1);
#endif
  input_line_pointer = save_input_line_pointer;
  if (gotfree_input_line)
    {
      free (gotfree_input_line);

      if (exp->X_op == O_constant || exp->X_op == O_register)
	exp->X_op = O_illegal;
    }

  ret = i386_finalize_displacement (exp_seg, exp, types, disp_start);

  RESTORE_END_STRING (disp_end);

  return ret;
}

static int
i386_finalize_displacement (segT exp_seg ATTRIBUTE_UNUSED, expressionS *exp,
			    i386_operand_type types, const char *disp_start)
{
  int ret = 1;

  /* We do this to make sure that the section symbol is in
     the symbol table.  We will ultimately change the relocation
     to be relative to the beginning of the section.  */
  if (i.reloc[this_operand] == BFD_RELOC_386_GOTOFF
      || i.reloc[this_operand] == BFD_RELOC_X86_64_GOTPCREL
      || i.reloc[this_operand] == BFD_RELOC_X86_64_GOTOFF64)
    {
      if (exp->X_op != O_symbol)
	goto inv_disp;

      if (S_IS_LOCAL (exp->X_add_symbol)
	  && S_GET_SEGMENT (exp->X_add_symbol) != undefined_section
	  && S_GET_SEGMENT (exp->X_add_symbol) != expr_section)
	section_symbol (S_GET_SEGMENT (exp->X_add_symbol));
      exp->X_op = O_subtract;
      exp->X_op_symbol = GOT_symbol;
      if (i.reloc[this_operand] == BFD_RELOC_X86_64_GOTPCREL)
	i.reloc[this_operand] = BFD_RELOC_32_PCREL;
      else if (i.reloc[this_operand] == BFD_RELOC_X86_64_GOTOFF64)
	i.reloc[this_operand] = BFD_RELOC_64;
      else
	i.reloc[this_operand] = BFD_RELOC_32;
    }

  else if (exp->X_op == O_absent
	   || exp->X_op == O_illegal
	   || exp->X_op == O_big)
    {
    inv_disp:
      as_bad (_("missing or invalid displacement expression `%s'"),
	      disp_start);
      ret = 0;
    }

  else if (exp->X_op == O_constant)
    {
      /* Sizing gets taken care of by optimize_disp().

	 If not 64bit, sign/zero extend val, to account for wraparound
	 when !BFD64.  */
      if (expr_mode == expr_operator_present
	  && flag_code != CODE_64BIT && !object_64bit)
	exp->X_add_number = extend_to_32bit_address (exp->X_add_number);
    }

#if (defined (OBJ_AOUT) || defined (OBJ_MAYBE_AOUT))
  else if (OUTPUT_FLAVOR == bfd_target_aout_flavour
	   && exp_seg != absolute_section
	   && exp_seg != text_section
	   && exp_seg != data_section
	   && exp_seg != bss_section
	   && exp_seg != undefined_section
	   && !bfd_is_com_section (exp_seg))
    {
      as_bad (_("unimplemented segment %s in operand"), exp_seg->name);
      ret = 0;
    }
#endif

  else if (current_templates->start->opcode_modifier.jump == JUMP_BYTE)
    i.types[this_operand].bitfield.disp8 = 1;

  /* Check if this is a displacement only operand.  */
  if (!i.types[this_operand].bitfield.baseindex)
    i.types[this_operand] =
      operand_type_or (operand_type_and_not (i.types[this_operand], anydisp),
		       operand_type_and (i.types[this_operand], types));

  return ret;
}

/* Return the active addressing mode, taking address override and
   registers forming the address into consideration.  Update the
   address override prefix if necessary.  */

static enum flag_code
i386_addressing_mode (void)
{
  enum flag_code addr_mode;

  if (i.prefix[ADDR_PREFIX])
    addr_mode = flag_code == CODE_32BIT ? CODE_16BIT : CODE_32BIT;
  else if (flag_code == CODE_16BIT
	   && current_templates->start->cpu_flags.bitfield.cpumpx
	   /* Avoid replacing the "16-bit addressing not allowed" diagnostic
	      from md_assemble() by "is not a valid base/index expression"
	      when there is a base and/or index.  */
	   && !i.types[this_operand].bitfield.baseindex)
    {
      /* MPX insn memory operands with neither base nor index must be forced
	 to use 32-bit addressing in 16-bit mode.  */
      addr_mode = CODE_32BIT;
      i.prefix[ADDR_PREFIX] = ADDR_PREFIX_OPCODE;
      ++i.prefixes;
      gas_assert (!i.types[this_operand].bitfield.disp16);
      gas_assert (!i.types[this_operand].bitfield.disp32);
    }
  else
    {
      addr_mode = flag_code;

#if INFER_ADDR_PREFIX
      if (i.mem_operands == 0)
	{
	  /* Infer address prefix from the first memory operand.  */
	  const reg_entry *addr_reg = i.base_reg;

	  if (addr_reg == NULL)
	    addr_reg = i.index_reg;

	  if (addr_reg)
	    {
	      if (addr_reg->reg_type.bitfield.dword)
		addr_mode = CODE_32BIT;
	      else if (flag_code != CODE_64BIT
		       && addr_reg->reg_type.bitfield.word)
		addr_mode = CODE_16BIT;

	      if (addr_mode != flag_code)
		{
		  i.prefix[ADDR_PREFIX] = ADDR_PREFIX_OPCODE;
		  i.prefixes += 1;
		  /* Change the size of any displacement too.  At most one
		     of Disp16 or Disp32 is set.
		     FIXME.  There doesn't seem to be any real need for
		     separate Disp16 and Disp32 flags.  The same goes for
		     Imm16 and Imm32.  Removing them would probably clean
		     up the code quite a lot.  */
		  if (flag_code != CODE_64BIT
		      && (i.types[this_operand].bitfield.disp16
			  || i.types[this_operand].bitfield.disp32))
		    {
		      static const i386_operand_type disp16_32 = {
			.bitfield = { .disp16 = 1, .disp32 = 1 }
		      };

		      i.types[this_operand]
			= operand_type_xor (i.types[this_operand], disp16_32);
		    }
		}
	    }
	}
#endif
    }

  return addr_mode;
}

/* Make sure the memory operand we've been dealt is valid.
   Return 1 on success, 0 on a failure.  */

static int
i386_index_check (const char *operand_string)
{
  const char *kind = "base/index";
  enum flag_code addr_mode = i386_addressing_mode ();
  const insn_template *t = current_templates->end - 1;

  if (t->opcode_modifier.isstring)
    {
      /* Memory operands of string insns are special in that they only allow
	 a single register (rDI, rSI, or rBX) as their memory address.  */
      const reg_entry *expected_reg;
      static const char *di_si[][2] =
	{
	  { "esi", "edi" },
	  { "si", "di" },
	  { "rsi", "rdi" }
	};
      static const char *bx[] = { "ebx", "bx", "rbx" };

      kind = "string address";

      if (t->opcode_modifier.prefixok == PrefixRep)
	{
	  int es_op = t->opcode_modifier.isstring - IS_STRING_ES_OP0;
	  int op = 0;

	  if (!t->operand_types[0].bitfield.baseindex
	      || ((!i.mem_operands != !intel_syntax)
		  && t->operand_types[1].bitfield.baseindex))
	    op = 1;
	  expected_reg
	    = (const reg_entry *) str_hash_find (reg_hash,
						 di_si[addr_mode][op == es_op]);
	}
      else
	expected_reg
	  = (const reg_entry *)str_hash_find (reg_hash, bx[addr_mode]);

      if (i.base_reg != expected_reg
	  || i.index_reg
	  || operand_type_check (i.types[this_operand], disp))
	{
	  /* The second memory operand must have the same size as
	     the first one.  */
	  if (i.mem_operands
	      && i.base_reg
	      && !((addr_mode == CODE_64BIT
		    && i.base_reg->reg_type.bitfield.qword)
		   || (addr_mode == CODE_32BIT
		       ? i.base_reg->reg_type.bitfield.dword
		       : i.base_reg->reg_type.bitfield.word)))
	    goto bad_address;

	  as_warn (_("`%s' is not valid here (expected `%c%s%s%c')"),
		   operand_string,
		   intel_syntax ? '[' : '(',
		   register_prefix,
		   expected_reg->reg_name,
		   intel_syntax ? ']' : ')');
	  return 1;
	}
      else
	return 1;

    bad_address:
      as_bad (_("`%s' is not a valid %s expression"),
	      operand_string, kind);
      return 0;
    }
  else
    {
      t = current_templates->start;

      if (addr_mode != CODE_16BIT)
	{
	  /* 32-bit/64-bit checks.  */
	  if (i.disp_encoding == disp_encoding_16bit)
	    {
	    bad_disp:
	      as_bad (_("invalid `%s' prefix"),
		      addr_mode == CODE_16BIT ? "{disp32}" : "{disp16}");
	      return 0;
	    }

	  if ((i.base_reg
	       && ((addr_mode == CODE_64BIT
		    ? !i.base_reg->reg_type.bitfield.qword
		    : !i.base_reg->reg_type.bitfield.dword)
		   || (i.index_reg && i.base_reg->reg_num == RegIP)
		   || i.base_reg->reg_num == RegIZ))
	      || (i.index_reg
		  && !i.index_reg->reg_type.bitfield.xmmword
		  && !i.index_reg->reg_type.bitfield.ymmword
		  && !i.index_reg->reg_type.bitfield.zmmword
		  && ((addr_mode == CODE_64BIT
		       ? !i.index_reg->reg_type.bitfield.qword
		       : !i.index_reg->reg_type.bitfield.dword)
		      || !i.index_reg->reg_type.bitfield.baseindex)))
	    goto bad_address;

	  /* bndmk, bndldx, bndstx and mandatory non-vector SIB have special restrictions. */
	  if (t->mnem_off == MN_bndmk
	      || t->mnem_off == MN_bndldx
	      || t->mnem_off == MN_bndstx
	      || t->opcode_modifier.sib == SIBMEM)
	    {
	      /* They cannot use RIP-relative addressing. */
	      if (i.base_reg && i.base_reg->reg_num == RegIP)
		{
		  as_bad (_("`%s' cannot be used here"), operand_string);
		  return 0;
		}

	      /* bndldx and bndstx ignore their scale factor. */
	      if ((t->mnem_off == MN_bndldx || t->mnem_off == MN_bndstx)
		  && i.log2_scale_factor)
		as_warn (_("register scaling is being ignored here"));
	    }
	}
      else
	{
	  /* 16-bit checks.  */
	  if (i.disp_encoding == disp_encoding_32bit)
	    goto bad_disp;

	  if ((i.base_reg
	       && (!i.base_reg->reg_type.bitfield.word
		   || !i.base_reg->reg_type.bitfield.baseindex))
	      || (i.index_reg
		  && (!i.index_reg->reg_type.bitfield.word
		      || !i.index_reg->reg_type.bitfield.baseindex
		      || !(i.base_reg
			   && i.base_reg->reg_num < 6
			   && i.index_reg->reg_num >= 6
			   && i.log2_scale_factor == 0))))
	    goto bad_address;
	}
    }
  return 1;
}

/* Handle vector immediates.  */

static int
RC_SAE_immediate (const char *imm_start)
{
  const char *pstr = imm_start;

  if (*pstr != '{')
    return 0;

  pstr = RC_SAE_specifier (pstr + 1);
  if (pstr == NULL)
    return 0;

  if (*pstr++ != '}')
    {
      as_bad (_("Missing '}': '%s'"), imm_start);
      return 0;
    }
  /* RC/SAE immediate string should contain nothing more.  */;
  if (*pstr != 0)
    {
      as_bad (_("Junk after '}': '%s'"), imm_start);
      return 0;
    }

  /* Internally this doesn't count as an operand.  */
  --i.operands;

  return 1;
}

static INLINE bool starts_memory_operand (char c)
{
  return ISDIGIT (c)
	 || is_name_beginner (c)
	 || strchr ("([\"+-!~", c);
}

/* Parse OPERAND_STRING into the i386_insn structure I.  Returns zero
   on error.  */

static int
i386_att_operand (char *operand_string)
{
  const reg_entry *r;
  char *end_op;
  char *op_string = operand_string;

  if (is_space_char (*op_string))
    ++op_string;

  /* We check for an absolute prefix (differentiating,
     for example, 'jmp pc_relative_label' from 'jmp *absolute_label'.  */
  if (*op_string == ABSOLUTE_PREFIX
      && current_templates->start->opcode_modifier.jump)
    {
      ++op_string;
      if (is_space_char (*op_string))
	++op_string;
      i.jumpabsolute = true;
    }

  /* Check if operand is a register.  */
  if ((r = parse_register (op_string, &end_op)) != NULL)
    {
      i386_operand_type temp;

      if (r == &bad_reg)
	return 0;

      /* Check for a segment override by searching for ':' after a
	 segment register.  */
      op_string = end_op;
      if (is_space_char (*op_string))
	++op_string;
      if (*op_string == ':' && r->reg_type.bitfield.class == SReg)
	{
	  i.seg[i.mem_operands] = r;

	  /* Skip the ':' and whitespace.  */
	  ++op_string;
	  if (is_space_char (*op_string))
	    ++op_string;

	  /* Handle case of %es:*foo.  */
	  if (!i.jumpabsolute && *op_string == ABSOLUTE_PREFIX
	      && current_templates->start->opcode_modifier.jump)
	    {
	      ++op_string;
	      if (is_space_char (*op_string))
		++op_string;
	      i.jumpabsolute = true;
	    }

	  if (!starts_memory_operand (*op_string))
	    {
	      as_bad (_("bad memory operand `%s'"), op_string);
	      return 0;
	    }
	  goto do_memory_reference;
	}

      /* Handle vector operations.  */
      if (*op_string == '{')
	{
	  op_string = check_VecOperations (op_string);
	  if (op_string == NULL)
	    return 0;
	}

      if (*op_string)
	{
	  as_bad (_("junk `%s' after register"), op_string);
	  return 0;
	}

       /* Reject pseudo registers for .insn.  */
      if (dot_insn () && r->reg_type.bitfield.class == ClassNone)
	{
	  as_bad (_("`%s%s' cannot be used here"),
		  register_prefix, r->reg_name);
	  return 0;
	}

      temp = r->reg_type;
      temp.bitfield.baseindex = 0;
      i.types[this_operand] = operand_type_or (i.types[this_operand],
					       temp);
      i.types[this_operand].bitfield.unspecified = 0;
      i.op[this_operand].regs = r;
      i.reg_operands++;

      /* A GPR may follow an RC or SAE immediate only if a (vector) register
         operand was also present earlier on.  */
      if (i.rounding.type != rc_none && temp.bitfield.class == Reg
          && i.reg_operands == 1)
	{
	  unsigned int j;

	  for (j = 0; j < ARRAY_SIZE (RC_NamesTable); ++j)
	    if (i.rounding.type == RC_NamesTable[j].type)
	      break;
	  as_bad (_("`%s': misplaced `{%s}'"),
		  insn_name (current_templates->start), RC_NamesTable[j].name);
	  return 0;
	}
    }
  else if (*op_string == REGISTER_PREFIX)
    {
      as_bad (_("bad register name `%s'"), op_string);
      return 0;
    }
  else if (*op_string == IMMEDIATE_PREFIX)
    {
      ++op_string;
      if (i.jumpabsolute)
	{
	  as_bad (_("immediate operand illegal with absolute jump"));
	  return 0;
	}
      if (!i386_immediate (op_string))
	return 0;
      if (i.rounding.type != rc_none)
	{
	  as_bad (_("`%s': RC/SAE operand must follow immediate operands"),
		  insn_name (current_templates->start));
	  return 0;
	}
    }
  else if (RC_SAE_immediate (operand_string))
    {
      /* If it is a RC or SAE immediate, do the necessary placement check:
	 Only another immediate or a GPR may precede it.  */
      if (i.mem_operands || i.reg_operands + i.imm_operands > 1
	  || (i.reg_operands == 1
	      && i.op[0].regs->reg_type.bitfield.class != Reg))
	{
	  as_bad (_("`%s': misplaced `%s'"),
		  insn_name (current_templates->start), operand_string);
	  return 0;
	}
    }
  else if (starts_memory_operand (*op_string))
    {
      /* This is a memory reference of some sort.  */
      char *base_string;

      /* Start and end of displacement string expression (if found).  */
      char *displacement_string_start;
      char *displacement_string_end;

    do_memory_reference:
      /* Check for base index form.  We detect the base index form by
	 looking for an ')' at the end of the operand, searching
	 for the '(' matching it, and finding a REGISTER_PREFIX or ','
	 after the '('.  */
      base_string = op_string + strlen (op_string);

      /* Handle vector operations.  */
      --base_string;
      if (is_space_char (*base_string))
	--base_string;

      if (*base_string == '}')
	{
	  char *vop_start = NULL;

	  while (base_string-- > op_string)
	    {
	      if (*base_string == '"')
		break;
	      if (*base_string != '{')
		continue;

	      vop_start = base_string;

	      --base_string;
	      if (is_space_char (*base_string))
		--base_string;

	      if (*base_string != '}')
		break;

	      vop_start = NULL;
	    }

	  if (!vop_start)
	    {
	      as_bad (_("unbalanced figure braces"));
	      return 0;
	    }

	  if (check_VecOperations (vop_start) == NULL)
	    return 0;
	}

      /* If we only have a displacement, set-up for it to be parsed later.  */
      displacement_string_start = op_string;
      displacement_string_end = base_string + 1;

      if (*base_string == ')')
	{
	  char *temp_string;
	  unsigned int parens_not_balanced = 0;
	  bool in_quotes = false;

	  /* We've already checked that the number of left & right ()'s are
	     equal, and that there's a matching set of double quotes.  */
	  end_op = base_string;
	  for (temp_string = op_string; temp_string < end_op; temp_string++)
	    {
	      if (*temp_string == '\\' && temp_string[1] == '"')
		++temp_string;
	      else if (*temp_string == '"')
		in_quotes = !in_quotes;
	      else if (!in_quotes)
		{
		  if (*temp_string == '(' && !parens_not_balanced++)
		    base_string = temp_string;
		  if (*temp_string == ')')
		    --parens_not_balanced;
		}
	    }

	  temp_string = base_string;

	  /* Skip past '(' and whitespace.  */
	  gas_assert (*base_string == '(');
	  ++base_string;
	  if (is_space_char (*base_string))
	    ++base_string;

	  if (*base_string == ','
	      || ((i.base_reg = parse_register (base_string, &end_op))
		  != NULL))
	    {
	      displacement_string_end = temp_string;

	      i.types[this_operand].bitfield.baseindex = 1;

	      if (i.base_reg)
		{
		  if (i.base_reg == &bad_reg)
		    return 0;
		  base_string = end_op;
		  if (is_space_char (*base_string))
		    ++base_string;
		}

	      /* There may be an index reg or scale factor here.  */
	      if (*base_string == ',')
		{
		  ++base_string;
		  if (is_space_char (*base_string))
		    ++base_string;

		  if ((i.index_reg = parse_register (base_string, &end_op))
		      != NULL)
		    {
		      if (i.index_reg == &bad_reg)
			return 0;
		      base_string = end_op;
		      if (is_space_char (*base_string))
			++base_string;
		      if (*base_string == ',')
			{
			  ++base_string;
			  if (is_space_char (*base_string))
			    ++base_string;
			}
		      else if (*base_string != ')')
			{
			  as_bad (_("expecting `,' or `)' "
				    "after index register in `%s'"),
				  operand_string);
			  return 0;
			}
		    }
		  else if (*base_string == REGISTER_PREFIX)
		    {
		      end_op = strchr (base_string, ',');
		      if (end_op)
			*end_op = '\0';
		      as_bad (_("bad register name `%s'"), base_string);
		      return 0;
		    }

		  /* Check for scale factor.  */
		  if (*base_string != ')')
		    {
		      char *end_scale = i386_scale (base_string);

		      if (!end_scale)
			return 0;

		      base_string = end_scale;
		      if (is_space_char (*base_string))
			++base_string;
		      if (*base_string != ')')
			{
			  as_bad (_("expecting `)' "
				    "after scale factor in `%s'"),
				  operand_string);
			  return 0;
			}
		    }
		  else if (!i.index_reg)
		    {
		      as_bad (_("expecting index register or scale factor "
				"after `,'; got '%c'"),
			      *base_string);
		      return 0;
		    }
		}
	      else if (*base_string != ')')
		{
		  as_bad (_("expecting `,' or `)' "
			    "after base register in `%s'"),
			  operand_string);
		  return 0;
		}
	    }
	  else if (*base_string == REGISTER_PREFIX)
	    {
	      end_op = strchr (base_string, ',');
	      if (end_op)
		*end_op = '\0';
	      as_bad (_("bad register name `%s'"), base_string);
	      return 0;
	    }
	}

      /* If there's an expression beginning the operand, parse it,
	 assuming displacement_string_start and
	 displacement_string_end are meaningful.  */
      if (displacement_string_start != displacement_string_end)
	{
	  if (!i386_displacement (displacement_string_start,
				  displacement_string_end))
	    return 0;
	}

      /* Special case for (%dx) while doing input/output op.  */
      if (i.base_reg
	  && i.base_reg->reg_type.bitfield.instance == RegD
	  && i.base_reg->reg_type.bitfield.word
	  && i.index_reg == 0
	  && i.log2_scale_factor == 0
	  && i.seg[i.mem_operands] == 0
	  && !operand_type_check (i.types[this_operand], disp))
	{
	  i.types[this_operand] = i.base_reg->reg_type;
	  i.input_output_operand = true;
	  return 1;
	}

      if (i386_index_check (operand_string) == 0)
	return 0;
      i.flags[this_operand] |= Operand_Mem;
      i.mem_operands++;
    }
  else
    {
      /* It's not a memory operand; argh!  */
      as_bad (_("invalid char %s beginning operand %d `%s'"),
	      output_invalid (*op_string),
	      this_operand + 1,
	      op_string);
      return 0;
    }
  return 1;			/* Normal return.  */
}

/* Calculate the maximum variable size (i.e., excluding fr_fix)
   that an rs_machine_dependent frag may reach.  */

unsigned int
i386_frag_max_var (fragS *frag)
{
  /* The only relaxable frags are for jumps.
     Unconditional jumps can grow by 4 bytes and others by 5 bytes.  */
  gas_assert (frag->fr_type == rs_machine_dependent);
  return TYPE_FROM_RELAX_STATE (frag->fr_subtype) == UNCOND_JUMP ? 4 : 5;
}

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
static int
elf_symbol_resolved_in_segment_p (symbolS *fr_symbol, offsetT fr_var)
{
  /* STT_GNU_IFUNC symbol must go through PLT.  */
  if ((symbol_get_bfdsym (fr_symbol)->flags
       & BSF_GNU_INDIRECT_FUNCTION) != 0)
    return 0;

  if (!S_IS_EXTERNAL (fr_symbol))
    /* Symbol may be weak or local.  */
    return !S_IS_WEAK (fr_symbol);

  /* Global symbols with non-default visibility can't be preempted. */
  if (ELF_ST_VISIBILITY (S_GET_OTHER (fr_symbol)) != STV_DEFAULT)
    return 1;

  if (fr_var != NO_RELOC)
    switch ((enum bfd_reloc_code_real) fr_var)
      {
      case BFD_RELOC_386_PLT32:
      case BFD_RELOC_X86_64_PLT32:
	/* Symbol with PLT relocation may be preempted. */
	return 0;
      default:
	abort ();
      }

  /* Global symbols with default visibility in a shared library may be
     preempted by another definition.  */
  return !shared;
}
#endif

/* Table 3-2. Macro-Fusible Instructions in Haswell Microarchitecture
   Note also work for Skylake and Cascadelake.
---------------------------------------------------------------------
|   JCC   | ADD/SUB/CMP | INC/DEC | TEST/AND |
| ------  | ----------- | ------- | -------- |
|   Jo    |      N      |    N    |     Y    |
|   Jno   |      N      |    N    |     Y    |
|  Jc/Jb  |      Y      |    N    |     Y    |
| Jae/Jnb |      Y      |    N    |     Y    |
|  Je/Jz  |      Y      |    Y    |     Y    |
| Jne/Jnz |      Y      |    Y    |     Y    |
| Jna/Jbe |      Y      |    N    |     Y    |
| Ja/Jnbe |      Y      |    N    |     Y    |
|   Js    |      N      |    N    |     Y    |
|   Jns   |      N      |    N    |     Y    |
|  Jp/Jpe |      N      |    N    |     Y    |
| Jnp/Jpo |      N      |    N    |     Y    |
| Jl/Jnge |      Y      |    Y    |     Y    |
| Jge/Jnl |      Y      |    Y    |     Y    |
| Jle/Jng |      Y      |    Y    |     Y    |
| Jg/Jnle |      Y      |    Y    |     Y    |
---------------------------------------------------------------------  */
static int
i386_macro_fusible_p (enum mf_cmp_kind mf_cmp, enum mf_jcc_kind mf_jcc)
{
  if (mf_cmp == mf_cmp_alu_cmp)
    return ((mf_jcc >= mf_jcc_jc && mf_jcc <= mf_jcc_jna)
	    || mf_jcc == mf_jcc_jl || mf_jcc == mf_jcc_jle);
  if (mf_cmp == mf_cmp_incdec)
    return (mf_jcc == mf_jcc_je || mf_jcc == mf_jcc_jl
	    || mf_jcc == mf_jcc_jle);
  if (mf_cmp == mf_cmp_test_and)
    return 1;
  return 0;
}

/* Return the next non-empty frag.  */

static fragS *
i386_next_non_empty_frag (fragS *fragP)
{
  /* There may be a frag with a ".fill 0" when there is no room in
     the current frag for frag_grow in output_insn.  */
  for (fragP = fragP->fr_next;
       (fragP != NULL
	&& fragP->fr_type == rs_fill
	&& fragP->fr_fix == 0);
       fragP = fragP->fr_next)
    ;
  return fragP;
}

/* Return the next jcc frag after BRANCH_PADDING.  */

static fragS *
i386_next_fusible_jcc_frag (fragS *maybe_cmp_fragP, fragS *pad_fragP)
{
  fragS *branch_fragP;
  if (!pad_fragP)
    return NULL;

  if (pad_fragP->fr_type == rs_machine_dependent
      && (TYPE_FROM_RELAX_STATE (pad_fragP->fr_subtype)
	  == BRANCH_PADDING))
    {
      branch_fragP = i386_next_non_empty_frag (pad_fragP);
      if (branch_fragP->fr_type != rs_machine_dependent)
	return NULL;
      if (TYPE_FROM_RELAX_STATE (branch_fragP->fr_subtype) == COND_JUMP
	  && i386_macro_fusible_p (maybe_cmp_fragP->tc_frag_data.mf_type,
				   pad_fragP->tc_frag_data.mf_type))
	return branch_fragP;
    }

  return NULL;
}

/* Classify BRANCH_PADDING, BRANCH_PREFIX and FUSED_JCC_PADDING frags.  */

static void
i386_classify_machine_dependent_frag (fragS *fragP)
{
  fragS *cmp_fragP;
  fragS *pad_fragP;
  fragS *branch_fragP;
  fragS *next_fragP;
  unsigned int max_prefix_length;

  if (fragP->tc_frag_data.classified)
    return;

  /* First scan for BRANCH_PADDING and FUSED_JCC_PADDING.  Convert
     FUSED_JCC_PADDING and merge BRANCH_PADDING.  */
  for (next_fragP = fragP;
       next_fragP != NULL;
       next_fragP = next_fragP->fr_next)
    {
      next_fragP->tc_frag_data.classified = 1;
      if (next_fragP->fr_type == rs_machine_dependent)
	switch (TYPE_FROM_RELAX_STATE (next_fragP->fr_subtype))
	  {
	  case BRANCH_PADDING:
	    /* The BRANCH_PADDING frag must be followed by a branch
	       frag.  */
	    branch_fragP = i386_next_non_empty_frag (next_fragP);
	    next_fragP->tc_frag_data.u.branch_fragP = branch_fragP;
	    break;
	  case FUSED_JCC_PADDING:
	    /* Check if this is a fused jcc:
	       FUSED_JCC_PADDING
	       CMP like instruction
	       BRANCH_PADDING
	       COND_JUMP
	       */
	    cmp_fragP = i386_next_non_empty_frag (next_fragP);
	    pad_fragP = i386_next_non_empty_frag (cmp_fragP);
	    branch_fragP = i386_next_fusible_jcc_frag (next_fragP, pad_fragP);
	    if (branch_fragP)
	      {
		/* The BRANCH_PADDING frag is merged with the
		   FUSED_JCC_PADDING frag.  */
		next_fragP->tc_frag_data.u.branch_fragP = branch_fragP;
		/* CMP like instruction size.  */
		next_fragP->tc_frag_data.cmp_size = cmp_fragP->fr_fix;
		frag_wane (pad_fragP);
		/* Skip to branch_fragP.  */
		next_fragP = branch_fragP;
	      }
	    else if (next_fragP->tc_frag_data.max_prefix_length)
	      {
		/* Turn FUSED_JCC_PADDING into BRANCH_PREFIX if it isn't
		   a fused jcc.  */
		next_fragP->fr_subtype
		  = ENCODE_RELAX_STATE (BRANCH_PREFIX, 0);
		next_fragP->tc_frag_data.max_bytes
		  = next_fragP->tc_frag_data.max_prefix_length;
		/* This will be updated in the BRANCH_PREFIX scan.  */
		next_fragP->tc_frag_data.max_prefix_length = 0;
	      }
	    else
	      frag_wane (next_fragP);
	    break;
	  }
    }

  /* Stop if there is no BRANCH_PREFIX.  */
  if (!align_branch_prefix_size)
    return;

  /* Scan for BRANCH_PREFIX.  */
  for (; fragP != NULL; fragP = fragP->fr_next)
    {
      if (fragP->fr_type != rs_machine_dependent
	  || (TYPE_FROM_RELAX_STATE (fragP->fr_subtype)
	      != BRANCH_PREFIX))
	continue;

      /* Count all BRANCH_PREFIX frags before BRANCH_PADDING and
	 COND_JUMP_PREFIX.  */
      max_prefix_length = 0;
      for (next_fragP = fragP;
	   next_fragP != NULL;
	   next_fragP = next_fragP->fr_next)
	{
	  if (next_fragP->fr_type == rs_fill)
	    /* Skip rs_fill frags.  */
	    continue;
	  else if (next_fragP->fr_type != rs_machine_dependent)
	    /* Stop for all other frags.  */
	    break;

	  /* rs_machine_dependent frags.  */
	  if (TYPE_FROM_RELAX_STATE (next_fragP->fr_subtype)
	      == BRANCH_PREFIX)
	    {
	      /* Count BRANCH_PREFIX frags.  */
	      if (max_prefix_length >= MAX_FUSED_JCC_PADDING_SIZE)
		{
		  max_prefix_length = MAX_FUSED_JCC_PADDING_SIZE;
		  frag_wane (next_fragP);
		}
	      else
		max_prefix_length
		  += next_fragP->tc_frag_data.max_bytes;
	    }
	  else if ((TYPE_FROM_RELAX_STATE (next_fragP->fr_subtype)
		    == BRANCH_PADDING)
		   || (TYPE_FROM_RELAX_STATE (next_fragP->fr_subtype)
		       == FUSED_JCC_PADDING))
	    {
	      /* Stop at BRANCH_PADDING and FUSED_JCC_PADDING.  */
	      fragP->tc_frag_data.u.padding_fragP = next_fragP;
	      break;
	    }
	  else
	    /* Stop for other rs_machine_dependent frags.  */
	    break;
	}

      fragP->tc_frag_data.max_prefix_length = max_prefix_length;

      /* Skip to the next frag.  */
      fragP = next_fragP;
    }
}

/* Compute padding size for

	FUSED_JCC_PADDING
	CMP like instruction
	BRANCH_PADDING
	COND_JUMP/UNCOND_JUMP

   or

	BRANCH_PADDING
	COND_JUMP/UNCOND_JUMP
 */

static int
i386_branch_padding_size (fragS *fragP, offsetT address)
{
  unsigned int offset, size, padding_size;
  fragS *branch_fragP = fragP->tc_frag_data.u.branch_fragP;

  /* The start address of the BRANCH_PADDING or FUSED_JCC_PADDING frag.  */
  if (!address)
    address = fragP->fr_address;
  address += fragP->fr_fix;

  /* CMP like instrunction size.  */
  size = fragP->tc_frag_data.cmp_size;

  /* The base size of the branch frag.  */
  size += branch_fragP->fr_fix;

  /* Add opcode and displacement bytes for the rs_machine_dependent
     branch frag.  */
  if (branch_fragP->fr_type == rs_machine_dependent)
    size += md_relax_table[branch_fragP->fr_subtype].rlx_length;

  /* Check if branch is within boundary and doesn't end at the last
     byte.  */
  offset = address & ((1U << align_branch_power) - 1);
  if ((offset + size) >= (1U << align_branch_power))
    /* Padding needed to avoid crossing boundary.  */
    padding_size = (1U << align_branch_power) - offset;
  else
    /* No padding needed.  */
    padding_size = 0;

  /* The return value may be saved in tc_frag_data.length which is
     unsigned byte.  */
  if (!fits_in_unsigned_byte (padding_size))
    abort ();

  return padding_size;
}

/* i386_generic_table_relax_frag()

   Handle BRANCH_PADDING, BRANCH_PREFIX and FUSED_JCC_PADDING frags to
   grow/shrink padding to align branch frags.  Hand others to
   relax_frag().  */

long
i386_generic_table_relax_frag (segT segment, fragS *fragP, long stretch)
{
  if (TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == BRANCH_PADDING
      || TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == FUSED_JCC_PADDING)
    {
      long padding_size = i386_branch_padding_size (fragP, 0);
      long grow = padding_size - fragP->tc_frag_data.length;

      /* When the BRANCH_PREFIX frag is used, the computed address
         must match the actual address and there should be no padding.  */
      if (fragP->tc_frag_data.padding_address
	  && (fragP->tc_frag_data.padding_address != fragP->fr_address
	      || padding_size))
	abort ();

      /* Update the padding size.  */
      if (grow)
	fragP->tc_frag_data.length = padding_size;

      return grow;
    }
  else if (TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == BRANCH_PREFIX)
    {
      fragS *padding_fragP, *next_fragP;
      long padding_size, left_size, last_size;

      padding_fragP = fragP->tc_frag_data.u.padding_fragP;
      if (!padding_fragP)
	/* Use the padding set by the leading BRANCH_PREFIX frag.  */
	return (fragP->tc_frag_data.length
		- fragP->tc_frag_data.last_length);

      /* Compute the relative address of the padding frag in the very
        first time where the BRANCH_PREFIX frag sizes are zero.  */
      if (!fragP->tc_frag_data.padding_address)
	fragP->tc_frag_data.padding_address
	  = padding_fragP->fr_address - (fragP->fr_address - stretch);

      /* First update the last length from the previous interation.  */
      left_size = fragP->tc_frag_data.prefix_length;
      for (next_fragP = fragP;
	   next_fragP != padding_fragP;
	   next_fragP = next_fragP->fr_next)
	if (next_fragP->fr_type == rs_machine_dependent
	    && (TYPE_FROM_RELAX_STATE (next_fragP->fr_subtype)
		== BRANCH_PREFIX))
	  {
	    if (left_size)
	      {
		int max = next_fragP->tc_frag_data.max_bytes;
		if (max)
		  {
		    int size;
		    if (max > left_size)
		      size = left_size;
		    else
		      size = max;
		    left_size -= size;
		    next_fragP->tc_frag_data.last_length = size;
		  }
	      }
	    else
	      next_fragP->tc_frag_data.last_length = 0;
	  }

      /* Check the padding size for the padding frag.  */
      padding_size = i386_branch_padding_size
	(padding_fragP, (fragP->fr_address
			 + fragP->tc_frag_data.padding_address));

      last_size = fragP->tc_frag_data.prefix_length;
      /* Check if there is change from the last interation.  */
      if (padding_size == last_size)
	{
	  /* Update the expected address of the padding frag.  */
	  padding_fragP->tc_frag_data.padding_address
	    = (fragP->fr_address + padding_size
	       + fragP->tc_frag_data.padding_address);
	  return 0;
	}

      if (padding_size > fragP->tc_frag_data.max_prefix_length)
	{
	  /* No padding if there is no sufficient room.  Clear the
	     expected address of the padding frag.  */
	  padding_fragP->tc_frag_data.padding_address = 0;
	  padding_size = 0;
	}
      else
	/* Store the expected address of the padding frag.  */
	padding_fragP->tc_frag_data.padding_address
	  = (fragP->fr_address + padding_size
	     + fragP->tc_frag_data.padding_address);

      fragP->tc_frag_data.prefix_length = padding_size;

      /* Update the length for the current interation.  */
      left_size = padding_size;
      for (next_fragP = fragP;
	   next_fragP != padding_fragP;
	   next_fragP = next_fragP->fr_next)
	if (next_fragP->fr_type == rs_machine_dependent
	    && (TYPE_FROM_RELAX_STATE (next_fragP->fr_subtype)
		== BRANCH_PREFIX))
	  {
	    if (left_size)
	      {
		int max = next_fragP->tc_frag_data.max_bytes;
		if (max)
		  {
		    int size;
		    if (max > left_size)
		      size = left_size;
		    else
		      size = max;
		    left_size -= size;
		    next_fragP->tc_frag_data.length = size;
		  }
	      }
	    else
	      next_fragP->tc_frag_data.length = 0;
	  }

      return (fragP->tc_frag_data.length
	      - fragP->tc_frag_data.last_length);
    }
  return relax_frag (segment, fragP, stretch);
}

/* md_estimate_size_before_relax()

   Called just before relax() for rs_machine_dependent frags.  The x86
   assembler uses these frags to handle variable size jump
   instructions.

   Any symbol that is now undefined will not become defined.
   Return the correct fr_subtype in the frag.
   Return the initial "guess for variable size of frag" to caller.
   The guess is actually the growth beyond the fixed part.  Whatever
   we do to grow the fixed or variable part contributes to our
   returned value.  */

int
md_estimate_size_before_relax (fragS *fragP, segT segment)
{
  if (TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == BRANCH_PADDING
      || TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == BRANCH_PREFIX
      || TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == FUSED_JCC_PADDING)
    {
      i386_classify_machine_dependent_frag (fragP);
      return fragP->tc_frag_data.length;
    }

  /* We've already got fragP->fr_subtype right;  all we have to do is
     check for un-relaxable symbols.  On an ELF system, we can't relax
     an externally visible symbol, because it may be overridden by a
     shared library.  */
  if (S_GET_SEGMENT (fragP->fr_symbol) != segment
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
      || (IS_ELF
	  && !elf_symbol_resolved_in_segment_p (fragP->fr_symbol,
						fragP->fr_var))
#endif
#if defined (OBJ_COFF) && defined (TE_PE)
      || (OUTPUT_FLAVOR == bfd_target_coff_flavour
	  && S_IS_WEAK (fragP->fr_symbol))
#endif
      )
    {
      /* Symbol is undefined in this segment, or we need to keep a
	 reloc so that weak symbols can be overridden.  */
      int size = (fragP->fr_subtype & CODE16) ? 2 : 4;
      enum bfd_reloc_code_real reloc_type;
      unsigned char *opcode;
      int old_fr_fix;
      fixS *fixP = NULL;

      if (fragP->fr_var != NO_RELOC)
	reloc_type = (enum bfd_reloc_code_real) fragP->fr_var;
      else if (size == 2)
	reloc_type = BFD_RELOC_16_PCREL;
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
      else if (fragP->tc_frag_data.code64 && fragP->fr_offset == 0
	       && need_plt32_p (fragP->fr_symbol))
	reloc_type = BFD_RELOC_X86_64_PLT32;
#endif
      else
	reloc_type = BFD_RELOC_32_PCREL;

      old_fr_fix = fragP->fr_fix;
      opcode = (unsigned char *) fragP->fr_opcode;

      switch (TYPE_FROM_RELAX_STATE (fragP->fr_subtype))
	{
	case UNCOND_JUMP:
	  /* Make jmp (0xeb) a (d)word displacement jump.  */
	  opcode[0] = 0xe9;
	  fragP->fr_fix += size;
	  fixP = fix_new (fragP, old_fr_fix, size,
			  fragP->fr_symbol,
			  fragP->fr_offset, 1,
			  reloc_type);
	  break;

	case COND_JUMP86:
	  if (size == 2
	      && (!no_cond_jump_promotion || fragP->fr_var != NO_RELOC))
	    {
	      /* Negate the condition, and branch past an
		 unconditional jump.  */
	      opcode[0] ^= 1;
	      opcode[1] = 3;
	      /* Insert an unconditional jump.  */
	      opcode[2] = 0xe9;
	      /* We added two extra opcode bytes, and have a two byte
		 offset.  */
	      fragP->fr_fix += 2 + 2;
	      fix_new (fragP, old_fr_fix + 2, 2,
		       fragP->fr_symbol,
		       fragP->fr_offset, 1,
		       reloc_type);
	      break;
	    }
	  /* Fall through.  */

	case COND_JUMP:
	  if (no_cond_jump_promotion && fragP->fr_var == NO_RELOC)
	    {
	      fragP->fr_fix += 1;
	      fixP = fix_new (fragP, old_fr_fix, 1,
			      fragP->fr_symbol,
			      fragP->fr_offset, 1,
			      BFD_RELOC_8_PCREL);
	      fixP->fx_signed = 1;
	      break;
	    }

	  /* This changes the byte-displacement jump 0x7N
	     to the (d)word-displacement jump 0x0f,0x8N.  */
	  opcode[1] = opcode[0] + 0x10;
	  opcode[0] = TWO_BYTE_OPCODE_ESCAPE;
	  /* We've added an opcode byte.  */
	  fragP->fr_fix += 1 + size;
	  fixP = fix_new (fragP, old_fr_fix + 1, size,
			  fragP->fr_symbol,
			  fragP->fr_offset, 1,
			  reloc_type);
	  break;

	default:
	  BAD_CASE (fragP->fr_subtype);
	  break;
	}

      /* All jumps handled here are signed, but don't unconditionally use a
	 signed limit check for 32 and 16 bit jumps as we want to allow wrap
	 around at 4G (outside of 64-bit mode) and 64k.  */
      if (size == 4 && flag_code == CODE_64BIT)
	fixP->fx_signed = 1;

      frag_wane (fragP);
      return fragP->fr_fix - old_fr_fix;
    }

  /* Guess size depending on current relax state.  Initially the relax
     state will correspond to a short jump and we return 1, because
     the variable part of the frag (the branch offset) is one byte
     long.  However, we can relax a section more than once and in that
     case we must either set fr_subtype back to the unrelaxed state,
     or return the value for the appropriate branch.  */
  return md_relax_table[fragP->fr_subtype].rlx_length;
}

/* Called after relax() is finished.

   In:	Address of frag.
	fr_type == rs_machine_dependent.
	fr_subtype is what the address relaxed to.

   Out:	Any fixSs and constants are set up.
	Caller will turn frag into a ".space 0".  */

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED,
                 fragS *fragP)
{
  unsigned char *opcode;
  unsigned char *where_to_put_displacement = NULL;
  offsetT target_address;
  offsetT opcode_address;
  unsigned int extension = 0;
  offsetT displacement_from_opcode_start;

  if (TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == BRANCH_PADDING
      || TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == FUSED_JCC_PADDING
      || TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == BRANCH_PREFIX)
    {
      /* Generate nop padding.  */
      unsigned int size = fragP->tc_frag_data.length;
      if (size)
	{
	  if (size > fragP->tc_frag_data.max_bytes)
	    abort ();

	  if (flag_debug)
	    {
	      const char *msg;
	      const char *branch = "branch";
	      const char *prefix = "";
	      fragS *padding_fragP;
	      if (TYPE_FROM_RELAX_STATE (fragP->fr_subtype)
		  == BRANCH_PREFIX)
		{
		  padding_fragP = fragP->tc_frag_data.u.padding_fragP;
		  switch (fragP->tc_frag_data.default_prefix)
		    {
		    default:
		      abort ();
		      break;
		    case CS_PREFIX_OPCODE:
		      prefix = " cs";
		      break;
		    case DS_PREFIX_OPCODE:
		      prefix = " ds";
		      break;
		    case ES_PREFIX_OPCODE:
		      prefix = " es";
		      break;
		    case FS_PREFIX_OPCODE:
		      prefix = " fs";
		      break;
		    case GS_PREFIX_OPCODE:
		      prefix = " gs";
		      break;
		    case SS_PREFIX_OPCODE:
		      prefix = " ss";
		      break;
		    }
		  if (padding_fragP)
		    msg = _("%s:%u: add %d%s at 0x%llx to align "
			    "%s within %d-byte boundary\n");
		  else
		    msg = _("%s:%u: add additional %d%s at 0x%llx to "
			    "align %s within %d-byte boundary\n");
		}
	      else
		{
		  padding_fragP = fragP;
		  msg = _("%s:%u: add %d%s-byte nop at 0x%llx to align "
			  "%s within %d-byte boundary\n");
		}

	      if (padding_fragP)
		switch (padding_fragP->tc_frag_data.branch_type)
		  {
		  case align_branch_jcc:
		    branch = "jcc";
		    break;
		  case align_branch_fused:
		    branch = "fused jcc";
		    break;
		  case align_branch_jmp:
		    branch = "jmp";
		    break;
		  case align_branch_call:
		    branch = "call";
		    break;
		  case align_branch_indirect:
		    branch = "indiret branch";
		    break;
		  case align_branch_ret:
		    branch = "ret";
		    break;
		  default:
		    break;
		  }

	      fprintf (stdout, msg,
		       fragP->fr_file, fragP->fr_line, size, prefix,
		       (long long) fragP->fr_address, branch,
		       1 << align_branch_power);
	    }
	  if (TYPE_FROM_RELAX_STATE (fragP->fr_subtype) == BRANCH_PREFIX)
	    memset (fragP->fr_opcode,
		    fragP->tc_frag_data.default_prefix, size);
	  else
	    i386_generate_nops (fragP, (char *) fragP->fr_opcode,
				size, 0);
	  fragP->fr_fix += size;
	}
      return;
    }

  opcode = (unsigned char *) fragP->fr_opcode;

  /* Address we want to reach in file space.  */
  target_address = S_GET_VALUE (fragP->fr_symbol) + fragP->fr_offset;

  /* Address opcode resides at in file space.  */
  opcode_address = fragP->fr_address + fragP->fr_fix;

  /* Displacement from opcode start to fill into instruction.  */
  displacement_from_opcode_start = target_address - opcode_address;

  if ((fragP->fr_subtype & BIG) == 0)
    {
      /* Don't have to change opcode.  */
      extension = 1;		/* 1 opcode + 1 displacement  */
      where_to_put_displacement = &opcode[1];
    }
  else
    {
      if (no_cond_jump_promotion
	  && TYPE_FROM_RELAX_STATE (fragP->fr_subtype) != UNCOND_JUMP)
	as_warn_where (fragP->fr_file, fragP->fr_line,
		       _("long jump required"));

      switch (fragP->fr_subtype)
	{
	case ENCODE_RELAX_STATE (UNCOND_JUMP, BIG):
	  extension = 4;		/* 1 opcode + 4 displacement  */
	  opcode[0] = 0xe9;
	  where_to_put_displacement = &opcode[1];
	  break;

	case ENCODE_RELAX_STATE (UNCOND_JUMP, BIG16):
	  extension = 2;		/* 1 opcode + 2 displacement  */
	  opcode[0] = 0xe9;
	  where_to_put_displacement = &opcode[1];
	  break;

	case ENCODE_RELAX_STATE (COND_JUMP, BIG):
	case ENCODE_RELAX_STATE (COND_JUMP86, BIG):
	  extension = 5;		/* 2 opcode + 4 displacement  */
	  opcode[1] = opcode[0] + 0x10;
	  opcode[0] = TWO_BYTE_OPCODE_ESCAPE;
	  where_to_put_displacement = &opcode[2];
	  break;

	case ENCODE_RELAX_STATE (COND_JUMP, BIG16):
	  extension = 3;		/* 2 opcode + 2 displacement  */
	  opcode[1] = opcode[0] + 0x10;
	  opcode[0] = TWO_BYTE_OPCODE_ESCAPE;
	  where_to_put_displacement = &opcode[2];
	  break;

	case ENCODE_RELAX_STATE (COND_JUMP86, BIG16):
	  extension = 4;
	  opcode[0] ^= 1;
	  opcode[1] = 3;
	  opcode[2] = 0xe9;
	  where_to_put_displacement = &opcode[3];
	  break;

	default:
	  BAD_CASE (fragP->fr_subtype);
	  break;
	}
    }

  /* If size if less then four we are sure that the operand fits,
     but if it's 4, then it could be that the displacement is larger
     then -/+ 2GB.  */
  if (DISP_SIZE_FROM_RELAX_STATE (fragP->fr_subtype) == 4
      && object_64bit
      && ((addressT) (displacement_from_opcode_start - extension
		      + ((addressT) 1 << 31))
	  > (((addressT) 2 << 31) - 1)))
    {
      as_bad_where (fragP->fr_file, fragP->fr_line,
		    _("jump target out of range"));
      /* Make us emit 0.  */
      displacement_from_opcode_start = extension;
    }
  /* Now put displacement after opcode.  */
  md_number_to_chars ((char *) where_to_put_displacement,
		      (valueT) (displacement_from_opcode_start - extension),
		      DISP_SIZE_FROM_RELAX_STATE (fragP->fr_subtype));
  fragP->fr_fix += extension;
}

/* Apply a fixup (fixP) to segment data, once it has been determined
   by our caller that we have all the info we need to fix it up.

   Parameter valP is the pointer to the value of the bits.

   On the 386, immediates, displacements, and data pointers are all in
   the same (little-endian) format, so we don't need to care about which
   we are handling.  */

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  char *p = fixP->fx_where + fixP->fx_frag->fr_literal;
  valueT value = *valP;

#if !defined (TE_Mach)
  if (fixP->fx_pcrel)
    {
      switch (fixP->fx_r_type)
	{
	default:
	  break;

	case BFD_RELOC_64:
	  fixP->fx_r_type = BFD_RELOC_64_PCREL;
	  break;
	case BFD_RELOC_32:
	case BFD_RELOC_X86_64_32S:
	  fixP->fx_r_type = BFD_RELOC_32_PCREL;
	  break;
	case BFD_RELOC_16:
	  fixP->fx_r_type = BFD_RELOC_16_PCREL;
	  break;
	case BFD_RELOC_8:
	  fixP->fx_r_type = BFD_RELOC_8_PCREL;
	  break;
	}
    }

  if (fixP->fx_addsy != NULL
      && (fixP->fx_r_type == BFD_RELOC_32_PCREL
	  || fixP->fx_r_type == BFD_RELOC_64_PCREL
	  || fixP->fx_r_type == BFD_RELOC_16_PCREL
	  || fixP->fx_r_type == BFD_RELOC_8_PCREL)
      && !use_rela_relocations)
    {
      /* This is a hack.  There should be a better way to handle this.
	 This covers for the fact that bfd_install_relocation will
	 subtract the current location (for partial_inplace, PC relative
	 relocations); see more below.  */
#ifndef OBJ_AOUT
      if (IS_ELF
#ifdef TE_PE
	  || OUTPUT_FLAVOR == bfd_target_coff_flavour
#endif
	  )
	value += fixP->fx_where + fixP->fx_frag->fr_address;
#endif
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
      if (IS_ELF)
	{
	  segT sym_seg = S_GET_SEGMENT (fixP->fx_addsy);

	  if ((sym_seg == seg
	       || (symbol_section_p (fixP->fx_addsy)
		   && sym_seg != absolute_section))
	      && !generic_force_reloc (fixP))
	    {
	      /* Yes, we add the values in twice.  This is because
		 bfd_install_relocation subtracts them out again.  I think
		 bfd_install_relocation is broken, but I don't dare change
		 it.  FIXME.  */
	      value += fixP->fx_where + fixP->fx_frag->fr_address;
	    }
	}
#endif
#if defined (OBJ_COFF) && defined (TE_PE)
      /* For some reason, the PE format does not store a
	 section address offset for a PC relative symbol.  */
      if (S_GET_SEGMENT (fixP->fx_addsy) != seg
	  || S_IS_WEAK (fixP->fx_addsy))
	value += md_pcrel_from (fixP);
#endif
    }
#if defined (OBJ_COFF) && defined (TE_PE)
  if (fixP->fx_addsy != NULL
      && S_IS_WEAK (fixP->fx_addsy)
      /* PR 16858: Do not modify weak function references.  */
      && ! fixP->fx_pcrel)
    {
#if !defined (TE_PEP)
      /* For x86 PE weak function symbols are neither PC-relative
	 nor do they set S_IS_FUNCTION.  So the only reliable way
	 to detect them is to check the flags of their containing
	 section.  */
      if (S_GET_SEGMENT (fixP->fx_addsy) != NULL
	  && S_GET_SEGMENT (fixP->fx_addsy)->flags & SEC_CODE)
	;
      else
#endif
      value -= S_GET_VALUE (fixP->fx_addsy);
    }
#endif

  /* Fix a few things - the dynamic linker expects certain values here,
     and we must not disappoint it.  */
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  if (IS_ELF && fixP->fx_addsy)
    switch (fixP->fx_r_type)
      {
      case BFD_RELOC_386_PLT32:
      case BFD_RELOC_X86_64_PLT32:
	/* Make the jump instruction point to the address of the operand.
	   At runtime we merely add the offset to the actual PLT entry.
	   NB: Subtract the offset size only for jump instructions.  */
	if (fixP->fx_pcrel)
	  value = -4;
	break;

      case BFD_RELOC_386_TLS_GD:
      case BFD_RELOC_386_TLS_LDM:
      case BFD_RELOC_386_TLS_IE_32:
      case BFD_RELOC_386_TLS_IE:
      case BFD_RELOC_386_TLS_GOTIE:
      case BFD_RELOC_386_TLS_GOTDESC:
      case BFD_RELOC_X86_64_TLSGD:
      case BFD_RELOC_X86_64_TLSLD:
      case BFD_RELOC_X86_64_GOTTPOFF:
      case BFD_RELOC_X86_64_GOTPC32_TLSDESC:
	value = 0; /* Fully resolved at runtime.  No addend.  */
	/* Fallthrough */
      case BFD_RELOC_386_TLS_LE:
      case BFD_RELOC_386_TLS_LDO_32:
      case BFD_RELOC_386_TLS_LE_32:
      case BFD_RELOC_X86_64_DTPOFF32:
      case BFD_RELOC_X86_64_DTPOFF64:
      case BFD_RELOC_X86_64_TPOFF32:
      case BFD_RELOC_X86_64_TPOFF64:
	S_SET_THREAD_LOCAL (fixP->fx_addsy);
	break;

      case BFD_RELOC_386_TLS_DESC_CALL:
      case BFD_RELOC_X86_64_TLSDESC_CALL:
	value = 0; /* Fully resolved at runtime.  No addend.  */
	S_SET_THREAD_LOCAL (fixP->fx_addsy);
	fixP->fx_done = 0;
	return;

      case BFD_RELOC_VTABLE_INHERIT:
      case BFD_RELOC_VTABLE_ENTRY:
	fixP->fx_done = 0;
	return;

      default:
	break;
      }
#endif /* defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)  */

  /* If not 64bit, massage value, to account for wraparound when !BFD64.  */
  if (!object_64bit)
    value = extend_to_32bit_address (value);

  *valP = value;
#endif /* !defined (TE_Mach)  */

  /* Are we finished with this relocation now?  */
  if (fixP->fx_addsy == NULL)
    {
      fixP->fx_done = 1;
      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_X86_64_32S:
	  fixP->fx_signed = 1;
	  break;

	default:
	  break;
	}
    }
#if defined (OBJ_COFF) && defined (TE_PE)
  else if (fixP->fx_addsy != NULL && S_IS_WEAK (fixP->fx_addsy))
    {
      fixP->fx_done = 0;
      /* Remember value for tc_gen_reloc.  */
      fixP->fx_addnumber = value;
      /* Clear out the frag for now.  */
      value = 0;
    }
#endif
  else if (use_rela_relocations)
    {
      if (!disallow_64bit_reloc || fixP->fx_r_type == NO_RELOC)
	fixP->fx_no_overflow = 1;
      /* Remember value for tc_gen_reloc.  */
      fixP->fx_addnumber = value;
      value = 0;
    }

  md_number_to_chars (p, value, fixP->fx_size);
}

const char *
md_atof (int type, char *litP, int *sizeP)
{
  /* This outputs the LITTLENUMs in REVERSE order;
     in accord with the bigendian 386.  */
  return ieee_md_atof (type, litP, sizeP, false);
}

static char output_invalid_buf[sizeof (unsigned char) * 2 + 6];

static char *
output_invalid (int c)
{
  if (ISPRINT (c))
    snprintf (output_invalid_buf, sizeof (output_invalid_buf),
	      "'%c'", c);
  else
    snprintf (output_invalid_buf, sizeof (output_invalid_buf),
	      "(0x%x)", (unsigned char) c);
  return output_invalid_buf;
}

/* Verify that @r can be used in the current context.  */

static bool check_register (const reg_entry *r)
{
  if (allow_pseudo_reg)
    return true;

  if (operand_type_all_zero (&r->reg_type))
    return false;

  if ((r->reg_type.bitfield.dword
       || (r->reg_type.bitfield.class == SReg && r->reg_num > 3)
       || r->reg_type.bitfield.class == RegCR
       || r->reg_type.bitfield.class == RegDR)
      && !cpu_arch_flags.bitfield.cpui386)
    return false;

  if (r->reg_type.bitfield.class == RegTR
      && (flag_code == CODE_64BIT
	  || !cpu_arch_flags.bitfield.cpui386
	  || cpu_arch_isa_flags.bitfield.cpui586
	  || cpu_arch_isa_flags.bitfield.cpui686))
    return false;

  if (r->reg_type.bitfield.class == RegMMX && !cpu_arch_flags.bitfield.cpummx)
    return false;

  if (!cpu_arch_flags.bitfield.cpuavx512f)
    {
      if (r->reg_type.bitfield.zmmword
	  || r->reg_type.bitfield.class == RegMask)
	return false;

      if (!cpu_arch_flags.bitfield.cpuavx)
	{
	  if (r->reg_type.bitfield.ymmword)
	    return false;

	  if (!cpu_arch_flags.bitfield.cpusse && r->reg_type.bitfield.xmmword)
	    return false;
	}
    }

  if (r->reg_type.bitfield.tmmword
      && (!cpu_arch_flags.bitfield.cpuamx_tile
          || flag_code != CODE_64BIT))
    return false;

  if (r->reg_type.bitfield.class == RegBND && !cpu_arch_flags.bitfield.cpumpx)
    return false;

  /* Don't allow fake index register unless allow_index_reg isn't 0. */
  if (!allow_index_reg && r->reg_num == RegIZ)
    return false;

  /* Upper 16 vector registers are only available with VREX in 64bit
     mode, and require EVEX encoding.  */
  if (r->reg_flags & RegVRex)
    {
      if (!cpu_arch_flags.bitfield.cpuavx512f
	  || flag_code != CODE_64BIT)
	return false;

      if (i.vec_encoding == vex_encoding_default)
	i.vec_encoding = vex_encoding_evex;
      else if (i.vec_encoding != vex_encoding_evex)
	i.vec_encoding = vex_encoding_error;
    }

  if (((r->reg_flags & (RegRex64 | RegRex)) || r->reg_type.bitfield.qword)
      && (!cpu_arch_flags.bitfield.cpulm
	  || r->reg_type.bitfield.class != RegCR
	  || dot_insn ())
      && flag_code != CODE_64BIT)
    return false;

  if (r->reg_type.bitfield.class == SReg && r->reg_num == RegFlat
      && !intel_syntax)
    return false;

  return true;
}

/* REG_STRING starts *before* REGISTER_PREFIX.  */

static const reg_entry *
parse_real_register (const char *reg_string, char **end_op)
{
  const char *s = reg_string;
  char *p;
  char reg_name_given[MAX_REG_NAME_SIZE + 1];
  const reg_entry *r;

  /* Skip possible REGISTER_PREFIX and possible whitespace.  */
  if (*s == REGISTER_PREFIX)
    ++s;

  if (is_space_char (*s))
    ++s;

  p = reg_name_given;
  while ((*p++ = register_chars[(unsigned char) *s]) != '\0')
    {
      if (p >= reg_name_given + MAX_REG_NAME_SIZE)
	return (const reg_entry *) NULL;
      s++;
    }

  if (is_part_of_name (*s))
    return (const reg_entry *) NULL;

  *end_op = (char *) s;

  r = (const reg_entry *) str_hash_find (reg_hash, reg_name_given);

  /* Handle floating point regs, allowing spaces in the (i) part.  */
  if (r == reg_st0)
    {
      if (!cpu_arch_flags.bitfield.cpu8087
	  && !cpu_arch_flags.bitfield.cpu287
	  && !cpu_arch_flags.bitfield.cpu387
	  && !allow_pseudo_reg)
	return (const reg_entry *) NULL;

      if (is_space_char (*s))
	++s;
      if (*s == '(')
	{
	  ++s;
	  if (is_space_char (*s))
	    ++s;
	  if (*s >= '0' && *s <= '7')
	    {
	      int fpr = *s - '0';
	      ++s;
	      if (is_space_char (*s))
		++s;
	      if (*s == ')')
		{
		  *end_op = (char *) s + 1;
		  know (r[fpr].reg_num == fpr);
		  return r + fpr;
		}
	    }
	  /* We have "%st(" then garbage.  */
	  return (const reg_entry *) NULL;
	}
    }

  return r && check_register (r) ? r : NULL;
}

/* REG_STRING starts *before* REGISTER_PREFIX.  */

static const reg_entry *
parse_register (const char *reg_string, char **end_op)
{
  const reg_entry *r;

  if (*reg_string == REGISTER_PREFIX || allow_naked_reg)
    r = parse_real_register (reg_string, end_op);
  else
    r = NULL;
  if (!r)
    {
      char *save = input_line_pointer;
      char *buf = xstrdup (reg_string), *name;
      symbolS *symbolP;

      input_line_pointer = buf;
      get_symbol_name (&name);
      symbolP = symbol_find (name);
      while (symbolP && symbol_equated_p (symbolP))
	{
	  const expressionS *e = symbol_get_value_expression(symbolP);

	  if (e->X_add_number)
	    break;
	  symbolP = e->X_add_symbol;
	}
      if (symbolP && S_GET_SEGMENT (symbolP) == reg_section)
	{
	  const expressionS *e = symbol_get_value_expression (symbolP);

	  if (e->X_op == O_register)
	    {
	      know (e->X_add_number >= 0
		    && (valueT) e->X_add_number < i386_regtab_size);
	      r = i386_regtab + e->X_add_number;
	      *end_op = (char *) reg_string + (input_line_pointer - buf);
	    }
	  if (r && !check_register (r))
	    {
	      as_bad (_("register '%s%s' cannot be used here"),
		      register_prefix, r->reg_name);
	      r = &bad_reg;
	    }
	}
      input_line_pointer = save;
      free (buf);
    }
  return r;
}

int
i386_parse_name (char *name, expressionS *e, char *nextcharP)
{
  const reg_entry *r = NULL;
  char *end = input_line_pointer;

  /* We only know the terminating character here.  It being double quote could
     be the closing one of a quoted symbol name, or an opening one from a
     following string (or another quoted symbol name).  Since the latter can't
     be valid syntax for anything, bailing in either case is good enough.  */
  if (*nextcharP == '"')
    return 0;

  *end = *nextcharP;
  if (*name == REGISTER_PREFIX || allow_naked_reg)
    r = parse_real_register (name, &input_line_pointer);
  if (r && end <= input_line_pointer)
    {
      *nextcharP = *input_line_pointer;
      *input_line_pointer = 0;
      e->X_op = O_register;
      e->X_add_number = r - i386_regtab;
      return 1;
    }
  input_line_pointer = end;
  *end = 0;
  return intel_syntax ? i386_intel_parse_name (name, e) : 0;
}

void
md_operand (expressionS *e)
{
  char *end;
  const reg_entry *r;

  switch (*input_line_pointer)
    {
    case REGISTER_PREFIX:
      r = parse_real_register (input_line_pointer, &end);
      if (r)
	{
	  e->X_op = O_register;
	  e->X_add_number = r - i386_regtab;
	  input_line_pointer = end;
	}
      break;

    case '[':
      gas_assert (intel_syntax);
      end = input_line_pointer++;
      expression (e);
      if (*input_line_pointer == ']')
	{
	  ++input_line_pointer;
	  e->X_op_symbol = make_expr_symbol (e);
	  e->X_add_symbol = NULL;
	  e->X_add_number = 0;
	  e->X_op = O_index;
	}
      else
	{
	  e->X_op = O_absent;
	  input_line_pointer = end;
	}
      break;
    }
}

#ifdef BFD64
/* To maintain consistency with !BFD64 builds of gas record, whether any
   (binary) operator was involved in an expression.  As expressions are
   evaluated in only 32 bits when !BFD64, we use this to decide whether to
   truncate results.  */
bool i386_record_operator (operatorT op,
			   const expressionS *left,
			   const expressionS *right)
{
  if (op == O_absent)
    return false;

  if (!left)
    {
      /* Since the expression parser applies unary operators fine to bignum
	 operands, we don't need to be concerned of respective operands not
	 fitting in 32 bits.  */
      if (right->X_op == O_constant && right->X_unsigned
	  && !fits_in_unsigned_long (right->X_add_number))
	return false;
    }
  /* This isn't entirely right: The pattern can also result when constant
     expressions are folded (e.g. 0xffffffff + 1).  */
  else if ((left->X_op == O_constant && left->X_unsigned
	    && !fits_in_unsigned_long (left->X_add_number))
	   || (right->X_op == O_constant && right->X_unsigned
	       && !fits_in_unsigned_long (right->X_add_number)))
    expr_mode = expr_large_value;

  if (expr_mode != expr_large_value)
    expr_mode = expr_operator_present;

  return false;
}
#endif

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
const char *md_shortopts = "kVQ:sqnO::";
#else
const char *md_shortopts = "qnO::";
#endif

#define OPTION_32 (OPTION_MD_BASE + 0)
#define OPTION_64 (OPTION_MD_BASE + 1)
#define OPTION_DIVIDE (OPTION_MD_BASE + 2)
#define OPTION_MARCH (OPTION_MD_BASE + 3)
#define OPTION_MTUNE (OPTION_MD_BASE + 4)
#define OPTION_MMNEMONIC (OPTION_MD_BASE + 5)
#define OPTION_MSYNTAX (OPTION_MD_BASE + 6)
#define OPTION_MINDEX_REG (OPTION_MD_BASE + 7)
#define OPTION_MNAKED_REG (OPTION_MD_BASE + 8)
#define OPTION_MRELAX_RELOCATIONS (OPTION_MD_BASE + 9)
#define OPTION_MSSE2AVX (OPTION_MD_BASE + 10)
#define OPTION_MSSE_CHECK (OPTION_MD_BASE + 11)
#define OPTION_MOPERAND_CHECK (OPTION_MD_BASE + 12)
#define OPTION_MAVXSCALAR (OPTION_MD_BASE + 13)
#define OPTION_X32 (OPTION_MD_BASE + 14)
#define OPTION_MADD_BND_PREFIX (OPTION_MD_BASE + 15)
#define OPTION_MEVEXLIG (OPTION_MD_BASE + 16)
#define OPTION_MEVEXWIG (OPTION_MD_BASE + 17)
#define OPTION_MBIG_OBJ (OPTION_MD_BASE + 18)
#define OPTION_MOMIT_LOCK_PREFIX (OPTION_MD_BASE + 19)
#define OPTION_MEVEXRCIG (OPTION_MD_BASE + 20)
#define OPTION_MSHARED (OPTION_MD_BASE + 21)
#define OPTION_MAMD64 (OPTION_MD_BASE + 22)
#define OPTION_MINTEL64 (OPTION_MD_BASE + 23)
#define OPTION_MFENCE_AS_LOCK_ADD (OPTION_MD_BASE + 24)
#define OPTION_X86_USED_NOTE (OPTION_MD_BASE + 25)
#define OPTION_MVEXWIG (OPTION_MD_BASE + 26)
#define OPTION_MALIGN_BRANCH_BOUNDARY (OPTION_MD_BASE + 27)
#define OPTION_MALIGN_BRANCH_PREFIX_SIZE (OPTION_MD_BASE + 28)
#define OPTION_MALIGN_BRANCH (OPTION_MD_BASE + 29)
#define OPTION_MBRANCHES_WITH_32B_BOUNDARIES (OPTION_MD_BASE + 30)
#define OPTION_MLFENCE_AFTER_LOAD (OPTION_MD_BASE + 31)
#define OPTION_MLFENCE_BEFORE_INDIRECT_BRANCH (OPTION_MD_BASE + 32)
#define OPTION_MLFENCE_BEFORE_RET (OPTION_MD_BASE + 33)
#define OPTION_MUSE_UNALIGNED_VECTOR_MOVE (OPTION_MD_BASE + 34)

struct option md_longopts[] =
{
  {"32", no_argument, NULL, OPTION_32},
#if (defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF) \
     || defined (TE_PE) || defined (TE_PEP) || defined (OBJ_MACH_O))
  {"64", no_argument, NULL, OPTION_64},
#endif
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  {"x32", no_argument, NULL, OPTION_X32},
  {"mshared", no_argument, NULL, OPTION_MSHARED},
  {"mx86-used-note", required_argument, NULL, OPTION_X86_USED_NOTE},
#endif
  {"divide", no_argument, NULL, OPTION_DIVIDE},
  {"march", required_argument, NULL, OPTION_MARCH},
  {"mtune", required_argument, NULL, OPTION_MTUNE},
  {"mmnemonic", required_argument, NULL, OPTION_MMNEMONIC},
  {"msyntax", required_argument, NULL, OPTION_MSYNTAX},
  {"mindex-reg", no_argument, NULL, OPTION_MINDEX_REG},
  {"mnaked-reg", no_argument, NULL, OPTION_MNAKED_REG},
  {"msse2avx", no_argument, NULL, OPTION_MSSE2AVX},
  {"muse-unaligned-vector-move", no_argument, NULL, OPTION_MUSE_UNALIGNED_VECTOR_MOVE},
  {"msse-check", required_argument, NULL, OPTION_MSSE_CHECK},
  {"moperand-check", required_argument, NULL, OPTION_MOPERAND_CHECK},
  {"mavxscalar", required_argument, NULL, OPTION_MAVXSCALAR},
  {"mvexwig", required_argument, NULL, OPTION_MVEXWIG},
  {"madd-bnd-prefix", no_argument, NULL, OPTION_MADD_BND_PREFIX},
  {"mevexlig", required_argument, NULL, OPTION_MEVEXLIG},
  {"mevexwig", required_argument, NULL, OPTION_MEVEXWIG},
# if defined (TE_PE) || defined (TE_PEP)
  {"mbig-obj", no_argument, NULL, OPTION_MBIG_OBJ},
#endif
  {"momit-lock-prefix", required_argument, NULL, OPTION_MOMIT_LOCK_PREFIX},
  {"mfence-as-lock-add", required_argument, NULL, OPTION_MFENCE_AS_LOCK_ADD},
  {"mrelax-relocations", required_argument, NULL, OPTION_MRELAX_RELOCATIONS},
  {"mevexrcig", required_argument, NULL, OPTION_MEVEXRCIG},
  {"malign-branch-boundary", required_argument, NULL, OPTION_MALIGN_BRANCH_BOUNDARY},
  {"malign-branch-prefix-size", required_argument, NULL, OPTION_MALIGN_BRANCH_PREFIX_SIZE},
  {"malign-branch", required_argument, NULL, OPTION_MALIGN_BRANCH},
  {"mbranches-within-32B-boundaries", no_argument, NULL, OPTION_MBRANCHES_WITH_32B_BOUNDARIES},
  {"mlfence-after-load", required_argument, NULL, OPTION_MLFENCE_AFTER_LOAD},
  {"mlfence-before-indirect-branch", required_argument, NULL,
   OPTION_MLFENCE_BEFORE_INDIRECT_BRANCH},
  {"mlfence-before-ret", required_argument, NULL, OPTION_MLFENCE_BEFORE_RET},
  {"mamd64", no_argument, NULL, OPTION_MAMD64},
  {"mintel64", no_argument, NULL, OPTION_MINTEL64},
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

int
md_parse_option (int c, const char *arg)
{
  unsigned int j;
  char *arch, *next, *saved, *type;

  switch (c)
    {
    case 'n':
      optimize_align_code = 0;
      break;

    case 'q':
      quiet_warnings = 1;
      break;

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
      /* -Qy, -Qn: SVR4 arguments controlling whether a .comment section
	 should be emitted or not.  FIXME: Not implemented.  */
    case 'Q':
      if ((arg[0] != 'y' && arg[0] != 'n') || arg[1])
	return 0;
      break;

      /* -V: SVR4 argument to print version ID.  */
    case 'V':
      print_version_id ();
      break;

      /* -k: Ignore for FreeBSD compatibility.  */
    case 'k':
      break;

    case 's':
      /* -s: On i386 Solaris, this tells the native assembler to use
	 .stab instead of .stab.excl.  We always use .stab anyhow.  */
      break;

    case OPTION_MSHARED:
      shared = 1;
      break;

    case OPTION_X86_USED_NOTE:
      if (strcasecmp (arg, "yes") == 0)
        x86_used_note = 1;
      else if (strcasecmp (arg, "no") == 0)
        x86_used_note = 0;
      else
        as_fatal (_("invalid -mx86-used-note= option: `%s'"), arg);
      break;


#endif
#if (defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF) \
     || defined (TE_PE) || defined (TE_PEP) || defined (OBJ_MACH_O))
    case OPTION_64:
      {
	const char **list, **l;

	list = bfd_target_list ();
	for (l = list; *l != NULL; l++)
	  if (startswith (*l, "elf64-x86-64")
	      || strcmp (*l, "coff-x86-64") == 0
	      || strcmp (*l, "pe-x86-64") == 0
	      || strcmp (*l, "pei-x86-64") == 0
	      || strcmp (*l, "mach-o-x86-64") == 0)
	    {
	      default_arch = "x86_64";
	      break;
	    }
	if (*l == NULL)
	  as_fatal (_("no compiled in support for x86_64"));
	free (list);
      }
      break;
#endif

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
    case OPTION_X32:
      if (IS_ELF)
	{
	  const char **list, **l;

	  list = bfd_target_list ();
	  for (l = list; *l != NULL; l++)
	    if (startswith (*l, "elf32-x86-64"))
	      {
		default_arch = "x86_64:32";
		break;
	      }
	  if (*l == NULL)
	    as_fatal (_("no compiled in support for 32bit x86_64"));
	  free (list);
	}
      else
	as_fatal (_("32bit x86_64 is only supported for ELF"));
      break;
#endif

    case OPTION_32:
      {
	const char **list, **l;

	list = bfd_target_list ();
	for (l = list; *l != NULL; l++)
	  if (strstr (*l, "-i386")
	      || strstr (*l, "-go32"))
	    {
	      default_arch = "i386";
	      break;
	    }
	if (*l == NULL)
	  as_fatal (_("no compiled in support for ix86"));
	free (list);
      }
      break;

    case OPTION_DIVIDE:
#ifdef SVR4_COMMENT_CHARS
      {
	char *n, *t;
	const char *s;

	n = XNEWVEC (char, strlen (i386_comment_chars) + 1);
	t = n;
	for (s = i386_comment_chars; *s != '\0'; s++)
	  if (*s != '/')
	    *t++ = *s;
	*t = '\0';
	i386_comment_chars = n;
      }
#endif
      break;

    case OPTION_MARCH:
      saved = xstrdup (arg);
      arch = saved;
      /* Allow -march=+nosse.  */
      if (*arch == '+')
	arch++;
      do
	{
	  if (*arch == '.')
	    as_fatal (_("invalid -march= option: `%s'"), arg);
	  next = strchr (arch, '+');
	  if (next)
	    *next++ = '\0';
	  for (j = 0; j < ARRAY_SIZE (cpu_arch); j++)
	    {
	      if (arch == saved && cpu_arch[j].type != PROCESSOR_NONE
	          && strcmp (arch, cpu_arch[j].name) == 0)
		{
		  /* Processor.  */
		  if (! cpu_arch[j].enable.bitfield.cpui386)
		    continue;

		  cpu_arch_name = cpu_arch[j].name;
		  free (cpu_sub_arch_name);
		  cpu_sub_arch_name = NULL;
		  cpu_arch_flags = cpu_arch[j].enable;
		  cpu_arch_isa = cpu_arch[j].type;
		  cpu_arch_isa_flags = cpu_arch[j].enable;
		  if (!cpu_arch_tune_set)
		    {
		      cpu_arch_tune = cpu_arch_isa;
		      cpu_arch_tune_flags = cpu_arch_isa_flags;
		    }
		  break;
		}
	      else if (cpu_arch[j].type == PROCESSOR_NONE
		       && strcmp (arch, cpu_arch[j].name) == 0
		       && !cpu_flags_all_zero (&cpu_arch[j].enable))
		{
		  /* ISA extension.  */
		  i386_cpu_flags flags;

		  flags = cpu_flags_or (cpu_arch_flags,
					cpu_arch[j].enable);

		  if (!cpu_flags_equal (&flags, &cpu_arch_flags))
		    {
		      extend_cpu_sub_arch_name (arch);
		      cpu_arch_flags = flags;
		      cpu_arch_isa_flags = flags;
		    }
		  else
		    cpu_arch_isa_flags
		      = cpu_flags_or (cpu_arch_isa_flags,
				      cpu_arch[j].enable);
		  break;
		}
	    }

	  if (j >= ARRAY_SIZE (cpu_arch) && startswith (arch, "no"))
	    {
	      /* Disable an ISA extension.  */
	      for (j = 0; j < ARRAY_SIZE (cpu_arch); j++)
		if (cpu_arch[j].type == PROCESSOR_NONE
		    && strcmp (arch + 2, cpu_arch[j].name) == 0)
		  {
		    i386_cpu_flags flags;

		    flags = cpu_flags_and_not (cpu_arch_flags,
					       cpu_arch[j].disable);
		    if (!cpu_flags_equal (&flags, &cpu_arch_flags))
		      {
			extend_cpu_sub_arch_name (arch);
			cpu_arch_flags = flags;
			cpu_arch_isa_flags = flags;
		      }
		    break;
		  }
	    }

	  if (j >= ARRAY_SIZE (cpu_arch))
	    as_fatal (_("invalid -march= option: `%s'"), arg);

	  arch = next;
	}
      while (next != NULL);
      free (saved);
      break;

    case OPTION_MTUNE:
      if (*arg == '.')
	as_fatal (_("invalid -mtune= option: `%s'"), arg);
      for (j = 0; j < ARRAY_SIZE (cpu_arch); j++)
	{
	  if (cpu_arch[j].type != PROCESSOR_NONE
	      && strcmp (arg, cpu_arch[j].name) == 0)
	    {
	      cpu_arch_tune_set = 1;
	      cpu_arch_tune = cpu_arch [j].type;
	      cpu_arch_tune_flags = cpu_arch[j].enable;
	      break;
	    }
	}
      if (j >= ARRAY_SIZE (cpu_arch))
	as_fatal (_("invalid -mtune= option: `%s'"), arg);
      break;

    case OPTION_MMNEMONIC:
      if (strcasecmp (arg, "att") == 0)
	intel_mnemonic = 0;
      else if (strcasecmp (arg, "intel") == 0)
	intel_mnemonic = 1;
      else
	as_fatal (_("invalid -mmnemonic= option: `%s'"), arg);
      break;

    case OPTION_MSYNTAX:
      if (strcasecmp (arg, "att") == 0)
	intel_syntax = 0;
      else if (strcasecmp (arg, "intel") == 0)
	intel_syntax = 1;
      else
	as_fatal (_("invalid -msyntax= option: `%s'"), arg);
      break;

    case OPTION_MINDEX_REG:
      allow_index_reg = 1;
      break;

    case OPTION_MNAKED_REG:
      allow_naked_reg = 1;
      break;

    case OPTION_MSSE2AVX:
      sse2avx = 1;
      break;

    case OPTION_MUSE_UNALIGNED_VECTOR_MOVE:
      use_unaligned_vector_move = 1;
      break;

    case OPTION_MSSE_CHECK:
      if (strcasecmp (arg, "error") == 0)
	sse_check = check_error;
      else if (strcasecmp (arg, "warning") == 0)
	sse_check = check_warning;
      else if (strcasecmp (arg, "none") == 0)
	sse_check = check_none;
      else
	as_fatal (_("invalid -msse-check= option: `%s'"), arg);
      break;

    case OPTION_MOPERAND_CHECK:
      if (strcasecmp (arg, "error") == 0)
	operand_check = check_error;
      else if (strcasecmp (arg, "warning") == 0)
	operand_check = check_warning;
      else if (strcasecmp (arg, "none") == 0)
	operand_check = check_none;
      else
	as_fatal (_("invalid -moperand-check= option: `%s'"), arg);
      break;

    case OPTION_MAVXSCALAR:
      if (strcasecmp (arg, "128") == 0)
	avxscalar = vex128;
      else if (strcasecmp (arg, "256") == 0)
	avxscalar = vex256;
      else
	as_fatal (_("invalid -mavxscalar= option: `%s'"), arg);
      break;

    case OPTION_MVEXWIG:
      if (strcmp (arg, "0") == 0)
	vexwig = vexw0;
      else if (strcmp (arg, "1") == 0)
	vexwig = vexw1;
      else
	as_fatal (_("invalid -mvexwig= option: `%s'"), arg);
      break;

    case OPTION_MADD_BND_PREFIX:
      add_bnd_prefix = 1;
      break;

    case OPTION_MEVEXLIG:
      if (strcmp (arg, "128") == 0)
	evexlig = evexl128;
      else if (strcmp (arg, "256") == 0)
	evexlig = evexl256;
      else  if (strcmp (arg, "512") == 0)
	evexlig = evexl512;
      else
	as_fatal (_("invalid -mevexlig= option: `%s'"), arg);
      break;

    case OPTION_MEVEXRCIG:
      if (strcmp (arg, "rne") == 0)
	evexrcig = rne;
      else if (strcmp (arg, "rd") == 0)
	evexrcig = rd;
      else if (strcmp (arg, "ru") == 0)
	evexrcig = ru;
      else if (strcmp (arg, "rz") == 0)
	evexrcig = rz;
      else
	as_fatal (_("invalid -mevexrcig= option: `%s'"), arg);
      break;

    case OPTION_MEVEXWIG:
      if (strcmp (arg, "0") == 0)
	evexwig = evexw0;
      else if (strcmp (arg, "1") == 0)
	evexwig = evexw1;
      else
	as_fatal (_("invalid -mevexwig= option: `%s'"), arg);
      break;

# if defined (TE_PE) || defined (TE_PEP)
    case OPTION_MBIG_OBJ:
      use_big_obj = 1;
      break;
#endif

    case OPTION_MOMIT_LOCK_PREFIX:
      if (strcasecmp (arg, "yes") == 0)
        omit_lock_prefix = 1;
      else if (strcasecmp (arg, "no") == 0)
        omit_lock_prefix = 0;
      else
        as_fatal (_("invalid -momit-lock-prefix= option: `%s'"), arg);
      break;

    case OPTION_MFENCE_AS_LOCK_ADD:
      if (strcasecmp (arg, "yes") == 0)
        avoid_fence = 1;
      else if (strcasecmp (arg, "no") == 0)
        avoid_fence = 0;
      else
        as_fatal (_("invalid -mfence-as-lock-add= option: `%s'"), arg);
      break;

    case OPTION_MLFENCE_AFTER_LOAD:
      if (strcasecmp (arg, "yes") == 0)
	lfence_after_load = 1;
      else if (strcasecmp (arg, "no") == 0)
	lfence_after_load = 0;
      else
        as_fatal (_("invalid -mlfence-after-load= option: `%s'"), arg);
      break;

    case OPTION_MLFENCE_BEFORE_INDIRECT_BRANCH:
      if (strcasecmp (arg, "all") == 0)
	{
	  lfence_before_indirect_branch = lfence_branch_all;
	  if (lfence_before_ret == lfence_before_ret_none)
	    lfence_before_ret = lfence_before_ret_shl;
	}
      else if (strcasecmp (arg, "memory") == 0)
	lfence_before_indirect_branch = lfence_branch_memory;
      else if (strcasecmp (arg, "register") == 0)
	lfence_before_indirect_branch = lfence_branch_register;
      else if (strcasecmp (arg, "none") == 0)
	lfence_before_indirect_branch = lfence_branch_none;
      else
        as_fatal (_("invalid -mlfence-before-indirect-branch= option: `%s'"),
		  arg);
      break;

    case OPTION_MLFENCE_BEFORE_RET:
      if (strcasecmp (arg, "or") == 0)
	lfence_before_ret = lfence_before_ret_or;
      else if (strcasecmp (arg, "not") == 0)
	lfence_before_ret = lfence_before_ret_not;
      else if (strcasecmp (arg, "shl") == 0 || strcasecmp (arg, "yes") == 0)
	lfence_before_ret = lfence_before_ret_shl;
      else if (strcasecmp (arg, "none") == 0)
	lfence_before_ret = lfence_before_ret_none;
      else
        as_fatal (_("invalid -mlfence-before-ret= option: `%s'"),
		  arg);
      break;

    case OPTION_MRELAX_RELOCATIONS:
      if (strcasecmp (arg, "yes") == 0)
        generate_relax_relocations = 1;
      else if (strcasecmp (arg, "no") == 0)
        generate_relax_relocations = 0;
      else
        as_fatal (_("invalid -mrelax-relocations= option: `%s'"), arg);
      break;

    case OPTION_MALIGN_BRANCH_BOUNDARY:
      {
	char *end;
	long int align = strtoul (arg, &end, 0);
	if (*end == '\0')
	  {
	    if (align == 0)
	      {
		align_branch_power = 0;
		break;
	      }
	    else if (align >= 16)
	      {
		int align_power;
		for (align_power = 0;
		     (align & 1) == 0;
		     align >>= 1, align_power++)
		  continue;
		/* Limit alignment power to 31.  */
		if (align == 1 && align_power < 32)
		  {
		    align_branch_power = align_power;
		    break;
		  }
	      }
	  }
	as_fatal (_("invalid -malign-branch-boundary= value: %s"), arg);
      }
      break;

    case OPTION_MALIGN_BRANCH_PREFIX_SIZE:
      {
	char *end;
	int align = strtoul (arg, &end, 0);
	/* Some processors only support 5 prefixes.  */
	if (*end == '\0' && align >= 0 && align < 6)
	  {
	    align_branch_prefix_size = align;
	    break;
	  }
	as_fatal (_("invalid -malign-branch-prefix-size= value: %s"),
		  arg);
      }
      break;

    case OPTION_MALIGN_BRANCH:
      align_branch = 0;
      saved = xstrdup (arg);
      type = saved;
      do
	{
	  next = strchr (type, '+');
	  if (next)
	    *next++ = '\0';
	  if (strcasecmp (type, "jcc") == 0)
	    align_branch |= align_branch_jcc_bit;
	  else if (strcasecmp (type, "fused") == 0)
	    align_branch |= align_branch_fused_bit;
	  else if (strcasecmp (type, "jmp") == 0)
	    align_branch |= align_branch_jmp_bit;
	  else if (strcasecmp (type, "call") == 0)
	    align_branch |= align_branch_call_bit;
	  else if (strcasecmp (type, "ret") == 0)
	    align_branch |= align_branch_ret_bit;
	  else if (strcasecmp (type, "indirect") == 0)
	    align_branch |= align_branch_indirect_bit;
	  else
	    as_fatal (_("invalid -malign-branch= option: `%s'"), arg);
	  type = next;
	}
      while (next != NULL);
      free (saved);
      break;

    case OPTION_MBRANCHES_WITH_32B_BOUNDARIES:
      align_branch_power = 5;
      align_branch_prefix_size = 5;
      align_branch = (align_branch_jcc_bit
		      | align_branch_fused_bit
		      | align_branch_jmp_bit);
      break;

    case OPTION_MAMD64:
      isa64 = amd64;
      break;

    case OPTION_MINTEL64:
      isa64 = intel64;
      break;

    case 'O':
      if (arg == NULL)
	{
	  optimize = 1;
	  /* Turn off -Os.  */
	  optimize_for_space = 0;
	}
      else if (*arg == 's')
	{
	  optimize_for_space = 1;
	  /* Turn on all encoding optimizations.  */
	  optimize = INT_MAX;
	}
      else
	{
	  optimize = atoi (arg);
	  /* Turn off -Os.  */
	  optimize_for_space = 0;
	}
      break;

    default:
      return 0;
    }
  return 1;
}

#define MESSAGE_TEMPLATE \
"                                                                                "

static char *
output_message (FILE *stream, char *p, char *message, char *start,
		int *left_p, const char *name, int len)
{
  int size = sizeof (MESSAGE_TEMPLATE);
  int left = *left_p;

  /* Reserve 2 spaces for ", " or ",\0" */
  left -= len + 2;

  /* Check if there is any room.  */
  if (left >= 0)
    {
      if (p != start)
	{
	  *p++ = ',';
	  *p++ = ' ';
	}
      p = mempcpy (p, name, len);
    }
  else
    {
      /* Output the current message now and start a new one.  */
      *p++ = ',';
      *p = '\0';
      fprintf (stream, "%s\n", message);
      p = start;
      left = size - (start - message) - len - 2;

      gas_assert (left >= 0);

      p = mempcpy (p, name, len);
    }

  *left_p = left;
  return p;
}

static void
show_arch (FILE *stream, int ext, int check)
{
  static char message[] = MESSAGE_TEMPLATE;
  char *start = message + 27;
  char *p;
  int size = sizeof (MESSAGE_TEMPLATE);
  int left;
  const char *name;
  int len;
  unsigned int j;

  p = start;
  left = size - (start - message);

  if (!ext && check)
    {
      p = output_message (stream, p, message, start, &left,
			  STRING_COMMA_LEN ("default"));
      p = output_message (stream, p, message, start, &left,
			  STRING_COMMA_LEN ("push"));
      p = output_message (stream, p, message, start, &left,
			  STRING_COMMA_LEN ("pop"));
    }

  for (j = 0; j < ARRAY_SIZE (cpu_arch); j++)
    {
      /* Should it be skipped?  */
      if (cpu_arch [j].skip)
	continue;

      name = cpu_arch [j].name;
      len = cpu_arch [j].len;
      if (cpu_arch[j].type == PROCESSOR_NONE)
	{
	  /* It is an extension.  Skip if we aren't asked to show it.  */
	  if (!ext || cpu_flags_all_zero (&cpu_arch[j].enable))
	    continue;
	}
      else if (ext)
	{
	  /* It is an processor.  Skip if we show only extension.  */
	  continue;
	}
      else if (check && ! cpu_arch[j].enable.bitfield.cpui386)
	{
	  /* It is an impossible processor - skip.  */
	  continue;
	}

      p = output_message (stream, p, message, start, &left, name, len);
    }

  /* Display disabled extensions.  */
  if (ext)
    for (j = 0; j < ARRAY_SIZE (cpu_arch); j++)
      {
	char *str;

	if (cpu_arch[j].type != PROCESSOR_NONE
	    || !cpu_flags_all_zero (&cpu_arch[j].enable))
	  continue;
	str = xasprintf ("no%s", cpu_arch[j].name);
	p = output_message (stream, p, message, start, &left, str,
			    strlen (str));
	free (str);
      }

  *p = '\0';
  fprintf (stream, "%s\n", message);
}

void
md_show_usage (FILE *stream)
{
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  fprintf (stream, _("\
  -Qy, -Qn                ignored\n\
  -V                      print assembler version number\n\
  -k                      ignored\n"));
#endif
  fprintf (stream, _("\
  -n                      do not optimize code alignment\n\
  -O{012s}                attempt some code optimizations\n\
  -q                      quieten some warnings\n"));
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  fprintf (stream, _("\
  -s                      ignored\n"));
#endif
#ifdef BFD64
# if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  fprintf (stream, _("\
  --32/--64/--x32         generate 32bit/64bit/x32 object\n"));
# elif defined (TE_PE) || defined (TE_PEP) || defined (OBJ_MACH_O)
  fprintf (stream, _("\
  --32/--64               generate 32bit/64bit object\n"));
# endif
#endif
#ifdef SVR4_COMMENT_CHARS
  fprintf (stream, _("\
  --divide                do not treat `/' as a comment character\n"));
#else
  fprintf (stream, _("\
  --divide                ignored\n"));
#endif
  fprintf (stream, _("\
  -march=CPU[,+EXTENSION...]\n\
                          generate code for CPU and EXTENSION, CPU is one of:\n"));
  show_arch (stream, 0, 1);
  fprintf (stream, _("\
                          EXTENSION is combination of (possibly \"no\"-prefixed):\n"));
  show_arch (stream, 1, 0);
  fprintf (stream, _("\
  -mtune=CPU              optimize for CPU, CPU is one of:\n"));
  show_arch (stream, 0, 0);
  fprintf (stream, _("\
  -msse2avx               encode SSE instructions with VEX prefix\n"));
  fprintf (stream, _("\
  -muse-unaligned-vector-move\n\
                          encode aligned vector move as unaligned vector move\n"));
  fprintf (stream, _("\
  -msse-check=[none|error|warning] (default: warning)\n\
                          check SSE instructions\n"));
  fprintf (stream, _("\
  -moperand-check=[none|error|warning] (default: warning)\n\
                          check operand combinations for validity\n"));
  fprintf (stream, _("\
  -mavxscalar=[128|256] (default: 128)\n\
                          encode scalar AVX instructions with specific vector\n\
                           length\n"));
  fprintf (stream, _("\
  -mvexwig=[0|1] (default: 0)\n\
                          encode VEX instructions with specific VEX.W value\n\
                           for VEX.W bit ignored instructions\n"));
  fprintf (stream, _("\
  -mevexlig=[128|256|512] (default: 128)\n\
                          encode scalar EVEX instructions with specific vector\n\
                           length\n"));
  fprintf (stream, _("\
  -mevexwig=[0|1] (default: 0)\n\
                          encode EVEX instructions with specific EVEX.W value\n\
                           for EVEX.W bit ignored instructions\n"));
  fprintf (stream, _("\
  -mevexrcig=[rne|rd|ru|rz] (default: rne)\n\
                          encode EVEX instructions with specific EVEX.RC value\n\
                           for SAE-only ignored instructions\n"));
  fprintf (stream, _("\
  -mmnemonic=[att|intel] "));
  if (SYSV386_COMPAT)
    fprintf (stream, _("(default: att)\n"));
  else
    fprintf (stream, _("(default: intel)\n"));
  fprintf (stream, _("\
                          use AT&T/Intel mnemonic\n"));
  fprintf (stream, _("\
  -msyntax=[att|intel] (default: att)\n\
                          use AT&T/Intel syntax\n"));
  fprintf (stream, _("\
  -mindex-reg             support pseudo index registers\n"));
  fprintf (stream, _("\
  -mnaked-reg             don't require `%%' prefix for registers\n"));
  fprintf (stream, _("\
  -madd-bnd-prefix        add BND prefix for all valid branches\n"));
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  fprintf (stream, _("\
  -mshared                disable branch optimization for shared code\n"));
  fprintf (stream, _("\
  -mx86-used-note=[no|yes] "));
  if (DEFAULT_X86_USED_NOTE)
    fprintf (stream, _("(default: yes)\n"));
  else
    fprintf (stream, _("(default: no)\n"));
  fprintf (stream, _("\
                          generate x86 used ISA and feature properties\n"));
#endif
#if defined (TE_PE) || defined (TE_PEP)
  fprintf (stream, _("\
  -mbig-obj               generate big object files\n"));
#endif
  fprintf (stream, _("\
  -momit-lock-prefix=[no|yes] (default: no)\n\
                          strip all lock prefixes\n"));
  fprintf (stream, _("\
  -mfence-as-lock-add=[no|yes] (default: no)\n\
                          encode lfence, mfence and sfence as\n\
                           lock addl $0x0, (%%{re}sp)\n"));
  fprintf (stream, _("\
  -mrelax-relocations=[no|yes] "));
  if (DEFAULT_GENERATE_X86_RELAX_RELOCATIONS)
    fprintf (stream, _("(default: yes)\n"));
  else
    fprintf (stream, _("(default: no)\n"));
  fprintf (stream, _("\
                          generate relax relocations\n"));
  fprintf (stream, _("\
  -malign-branch-boundary=NUM (default: 0)\n\
                          align branches within NUM byte boundary\n"));
  fprintf (stream, _("\
  -malign-branch=TYPE[+TYPE...] (default: jcc+fused+jmp)\n\
                          TYPE is combination of jcc, fused, jmp, call, ret,\n\
                           indirect\n\
                          specify types of branches to align\n"));
  fprintf (stream, _("\
  -malign-branch-prefix-size=NUM (default: 5)\n\
                          align branches with NUM prefixes per instruction\n"));
  fprintf (stream, _("\
  -mbranches-within-32B-boundaries\n\
                          align branches within 32 byte boundary\n"));
  fprintf (stream, _("\
  -mlfence-after-load=[no|yes] (default: no)\n\
                          generate lfence after load\n"));
  fprintf (stream, _("\
  -mlfence-before-indirect-branch=[none|all|register|memory] (default: none)\n\
                          generate lfence before indirect near branch\n"));
  fprintf (stream, _("\
  -mlfence-before-ret=[none|or|not|shl|yes] (default: none)\n\
                          generate lfence before ret\n"));
  fprintf (stream, _("\
  -mamd64                 accept only AMD64 ISA [default]\n"));
  fprintf (stream, _("\
  -mintel64               accept only Intel64 ISA\n"));
}

#if ((defined (OBJ_MAYBE_COFF) && defined (OBJ_MAYBE_AOUT)) \
     || defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF) \
     || defined (TE_PE) || defined (TE_PEP) || defined (OBJ_MACH_O))

/* Pick the target format to use.  */

const char *
i386_target_format (void)
{
  if (startswith (default_arch, "x86_64"))
    {
      update_code_flag (CODE_64BIT, 1);
      if (default_arch[6] == '\0')
	x86_elf_abi = X86_64_ABI;
      else
	x86_elf_abi = X86_64_X32_ABI;
    }
  else if (!strcmp (default_arch, "i386"))
    update_code_flag (CODE_32BIT, 1);
  else if (!strcmp (default_arch, "iamcu"))
    {
      update_code_flag (CODE_32BIT, 1);
      if (cpu_arch_isa == PROCESSOR_UNKNOWN)
	{
	  static const i386_cpu_flags iamcu_flags = CPU_IAMCU_FLAGS;
	  cpu_arch_name = "iamcu";
	  free (cpu_sub_arch_name);
	  cpu_sub_arch_name = NULL;
	  cpu_arch_flags = iamcu_flags;
	  cpu_arch_isa = PROCESSOR_IAMCU;
	  cpu_arch_isa_flags = iamcu_flags;
	  if (!cpu_arch_tune_set)
	    {
	      cpu_arch_tune = cpu_arch_isa;
	      cpu_arch_tune_flags = cpu_arch_isa_flags;
	    }
	}
      else if (cpu_arch_isa != PROCESSOR_IAMCU)
	as_fatal (_("Intel MCU doesn't support `%s' architecture"),
		  cpu_arch_name);
    }
  else
    as_fatal (_("unknown architecture"));

  if (cpu_flags_all_zero (&cpu_arch_isa_flags))
    cpu_arch_isa_flags = cpu_arch[flag_code == CODE_64BIT].enable;
  if (cpu_flags_all_zero (&cpu_arch_tune_flags))
    cpu_arch_tune_flags = cpu_arch[flag_code == CODE_64BIT].enable;

  switch (OUTPUT_FLAVOR)
    {
#if defined (OBJ_MAYBE_AOUT) || defined (OBJ_AOUT)
    case bfd_target_aout_flavour:
      return AOUT_TARGET_FORMAT;
#endif
#if defined (OBJ_MAYBE_COFF) || defined (OBJ_COFF)
# if defined (TE_PE) || defined (TE_PEP)
    case bfd_target_coff_flavour:
      if (flag_code == CODE_64BIT)
	{
	  object_64bit = 1;
	  return use_big_obj ? "pe-bigobj-x86-64" : "pe-x86-64";
	}
      return use_big_obj ? "pe-bigobj-i386" : "pe-i386";
# elif defined (TE_GO32)
    case bfd_target_coff_flavour:
      return "coff-go32";
# else
    case bfd_target_coff_flavour:
      return "coff-i386";
# endif
#endif
#if defined (OBJ_MAYBE_ELF) || defined (OBJ_ELF)
    case bfd_target_elf_flavour:
      {
	const char *format;

	switch (x86_elf_abi)
	  {
	  default:
	    format = ELF_TARGET_FORMAT;
#ifndef TE_SOLARIS
	    tls_get_addr = "___tls_get_addr";
#endif
	    break;
	  case X86_64_ABI:
	    use_rela_relocations = 1;
	    object_64bit = 1;
#ifndef TE_SOLARIS
	    tls_get_addr = "__tls_get_addr";
#endif
	    format = ELF_TARGET_FORMAT64;
	    break;
	  case X86_64_X32_ABI:
	    use_rela_relocations = 1;
	    object_64bit = 1;
#ifndef TE_SOLARIS
	    tls_get_addr = "__tls_get_addr";
#endif
	    disallow_64bit_reloc = 1;
	    format = ELF_TARGET_FORMAT32;
	    break;
	  }
	if (cpu_arch_isa == PROCESSOR_IAMCU)
	  {
	    if (x86_elf_abi != I386_ABI)
	      as_fatal (_("Intel MCU is 32bit only"));
	    return ELF_TARGET_IAMCU_FORMAT;
	  }
	else
	  return format;
      }
#endif
#if defined (OBJ_MACH_O)
    case bfd_target_mach_o_flavour:
      if (flag_code == CODE_64BIT)
	{
	  use_rela_relocations = 1;
	  object_64bit = 1;
	  return "mach-o-x86-64";
	}
      else
	return "mach-o-i386";
#endif
    default:
      abort ();
      return NULL;
    }
}

#endif /* OBJ_MAYBE_ more than one  */

symbolS *
md_undefined_symbol (char *name)
{
  if (name[0] == GLOBAL_OFFSET_TABLE_NAME[0]
      && name[1] == GLOBAL_OFFSET_TABLE_NAME[1]
      && name[2] == GLOBAL_OFFSET_TABLE_NAME[2]
      && strcmp (name, GLOBAL_OFFSET_TABLE_NAME) == 0)
    {
      if (!GOT_symbol)
	{
	  if (symbol_find (name))
	    as_bad (_("GOT already in symbol table"));
	  GOT_symbol = symbol_new (name, undefined_section,
				   &zero_address_frag, 0);
	};
      return GOT_symbol;
    }
  return 0;
}

/* Round up a section size to the appropriate boundary.  */

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
#if (defined (OBJ_AOUT) || defined (OBJ_MAYBE_AOUT))
  if (OUTPUT_FLAVOR == bfd_target_aout_flavour)
    {
      /* For a.out, force the section size to be aligned.  If we don't do
	 this, BFD will align it for us, but it will not write out the
	 final bytes of the section.  This may be a bug in BFD, but it is
	 easier to fix it here since that is how the other a.out targets
	 work.  */
      int align;

      align = bfd_section_alignment (segment);
      size = ((size + (1 << align) - 1) & (-((valueT) 1 << align)));
    }
#endif

  return size;
}

/* On the i386, PC-relative offsets are relative to the start of the
   next instruction.  That is, the address of the offset, plus its
   size, since the offset is always the last part of the insn.  */

long
md_pcrel_from (fixS *fixP)
{
  return fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address;
}

#ifndef I386COFF

static void
s_bss (int ignore ATTRIBUTE_UNUSED)
{
  int temp;

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  if (IS_ELF)
    obj_elf_section_change_hook ();
#endif
  temp = get_absolute_expression ();
  subseg_set (bss_section, (subsegT) temp);
  demand_empty_rest_of_line ();
}

#endif

/* Remember constant directive.  */

void
i386_cons_align (int ignore ATTRIBUTE_UNUSED)
{
  if (last_insn.kind != last_insn_directive
      && (bfd_section_flags (now_seg) & SEC_CODE))
    {
      last_insn.seg = now_seg;
      last_insn.kind = last_insn_directive;
      last_insn.name = "constant directive";
      last_insn.file = as_where (&last_insn.line);
      if (lfence_before_ret != lfence_before_ret_none)
	{
	  if (lfence_before_indirect_branch != lfence_branch_none)
	    as_warn (_("constant directive skips -mlfence-before-ret "
		       "and -mlfence-before-indirect-branch"));
	  else
	    as_warn (_("constant directive skips -mlfence-before-ret"));
	}
      else if (lfence_before_indirect_branch != lfence_branch_none)
	as_warn (_("constant directive skips -mlfence-before-indirect-branch"));
    }
}

int
i386_validate_fix (fixS *fixp)
{
  if (fixp->fx_addsy && S_GET_SEGMENT(fixp->fx_addsy) == reg_section)
    {
      reloc_howto_type *howto;

      howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("invalid %s relocation against register"),
		    howto ? howto->name : "<unknown>");
      return 0;
    }

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  if (fixp->fx_r_type == BFD_RELOC_SIZE32
      || fixp->fx_r_type == BFD_RELOC_SIZE64)
    return IS_ELF && fixp->fx_addsy
	   && (!S_IS_DEFINED (fixp->fx_addsy)
	       || S_IS_EXTERNAL (fixp->fx_addsy));
#endif

  if (fixp->fx_subsy)
    {
      if (fixp->fx_subsy == GOT_symbol)
	{
	  if (fixp->fx_r_type == BFD_RELOC_32_PCREL)
	    {
	      if (!object_64bit)
		abort ();
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
	      if (fixp->fx_tcbit2)
		fixp->fx_r_type = (fixp->fx_tcbit
				   ? BFD_RELOC_X86_64_REX_GOTPCRELX
				   : BFD_RELOC_X86_64_GOTPCRELX);
	      else
#endif
		fixp->fx_r_type = BFD_RELOC_X86_64_GOTPCREL;
	    }
	  else
	    {
	      if (!object_64bit)
		fixp->fx_r_type = BFD_RELOC_386_GOTOFF;
	      else
		fixp->fx_r_type = BFD_RELOC_X86_64_GOTOFF64;
	    }
	  fixp->fx_subsy = 0;
	}
    }
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  else
    {
      /* NB: Commit 292676c1 resolved PLT32 reloc aganst local symbol
	 to section.  Since PLT32 relocation must be against symbols,
	 turn such PLT32 relocation into PC32 relocation.  */
      if (fixp->fx_addsy
	  && (fixp->fx_r_type == BFD_RELOC_386_PLT32
	      || fixp->fx_r_type == BFD_RELOC_X86_64_PLT32)
	  && symbol_section_p (fixp->fx_addsy))
	fixp->fx_r_type = BFD_RELOC_32_PCREL;
      if (!object_64bit)
	{
	  if (fixp->fx_r_type == BFD_RELOC_386_GOT32
	      && fixp->fx_tcbit2)
	    fixp->fx_r_type = BFD_RELOC_386_GOT32X;
	}
    }
#endif

  return 1;
}

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *rel;
  bfd_reloc_code_real_type code;

  switch (fixp->fx_r_type)
    {
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
      symbolS *sym;

    case BFD_RELOC_SIZE32:
    case BFD_RELOC_SIZE64:
      if (fixp->fx_addsy
	  && !bfd_is_abs_section (S_GET_SEGMENT (fixp->fx_addsy))
	  && (!fixp->fx_subsy
	      || bfd_is_abs_section (S_GET_SEGMENT (fixp->fx_subsy))))
	sym = fixp->fx_addsy;
      else if (fixp->fx_subsy
	       && !bfd_is_abs_section (S_GET_SEGMENT (fixp->fx_subsy))
	       && (!fixp->fx_addsy
		   || bfd_is_abs_section (S_GET_SEGMENT (fixp->fx_addsy))))
	sym = fixp->fx_subsy;
      else
	sym = NULL;
      if (IS_ELF && sym && S_IS_DEFINED (sym) && !S_IS_EXTERNAL (sym))
	{
	  /* Resolve size relocation against local symbol to size of
	     the symbol plus addend.  */
	  valueT value = S_GET_SIZE (sym);

	  if (symbol_get_bfdsym (sym)->flags & BSF_SECTION_SYM)
	    value = bfd_section_size (S_GET_SEGMENT (sym));
	  if (sym == fixp->fx_subsy)
	    {
	      value = -value;
	      if (fixp->fx_addsy)
	        value += S_GET_VALUE (fixp->fx_addsy);
	    }
	  else if (fixp->fx_subsy)
	    value -= S_GET_VALUE (fixp->fx_subsy);
	  value += fixp->fx_offset;
	  if (fixp->fx_r_type == BFD_RELOC_SIZE32
	      && object_64bit
	      && !fits_in_unsigned_long (value))
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("symbol size computation overflow"));
	  fixp->fx_addsy = NULL;
	  fixp->fx_subsy = NULL;
	  md_apply_fix (fixp, (valueT *) &value, NULL);
	  return NULL;
	}
      if (!fixp->fx_addsy || fixp->fx_subsy)
	{
	  as_bad_where (fixp->fx_file, fixp->fx_line,
			"unsupported expression involving @size");
	  return NULL;
	}
#endif
      /* Fall through.  */

    case BFD_RELOC_X86_64_PLT32:
    case BFD_RELOC_X86_64_GOT32:
    case BFD_RELOC_X86_64_GOTPCREL:
    case BFD_RELOC_X86_64_GOTPCRELX:
    case BFD_RELOC_X86_64_REX_GOTPCRELX:
    case BFD_RELOC_386_PLT32:
    case BFD_RELOC_386_GOT32:
    case BFD_RELOC_386_GOT32X:
    case BFD_RELOC_386_GOTOFF:
    case BFD_RELOC_386_GOTPC:
    case BFD_RELOC_386_TLS_GD:
    case BFD_RELOC_386_TLS_LDM:
    case BFD_RELOC_386_TLS_LDO_32:
    case BFD_RELOC_386_TLS_IE_32:
    case BFD_RELOC_386_TLS_IE:
    case BFD_RELOC_386_TLS_GOTIE:
    case BFD_RELOC_386_TLS_LE_32:
    case BFD_RELOC_386_TLS_LE:
    case BFD_RELOC_386_TLS_GOTDESC:
    case BFD_RELOC_386_TLS_DESC_CALL:
    case BFD_RELOC_X86_64_TLSGD:
    case BFD_RELOC_X86_64_TLSLD:
    case BFD_RELOC_X86_64_DTPOFF32:
    case BFD_RELOC_X86_64_DTPOFF64:
    case BFD_RELOC_X86_64_GOTTPOFF:
    case BFD_RELOC_X86_64_TPOFF32:
    case BFD_RELOC_X86_64_TPOFF64:
    case BFD_RELOC_X86_64_GOTOFF64:
    case BFD_RELOC_X86_64_GOTPC32:
    case BFD_RELOC_X86_64_GOT64:
    case BFD_RELOC_X86_64_GOTPCREL64:
    case BFD_RELOC_X86_64_GOTPC64:
    case BFD_RELOC_X86_64_GOTPLT64:
    case BFD_RELOC_X86_64_PLTOFF64:
    case BFD_RELOC_X86_64_GOTPC32_TLSDESC:
    case BFD_RELOC_X86_64_TLSDESC_CALL:
    case BFD_RELOC_RVA:
    case BFD_RELOC_VTABLE_ENTRY:
    case BFD_RELOC_VTABLE_INHERIT:
#ifdef TE_PE
    case BFD_RELOC_32_SECREL:
    case BFD_RELOC_16_SECIDX:
#endif
      code = fixp->fx_r_type;
      break;
    case BFD_RELOC_X86_64_32S:
      if (!fixp->fx_pcrel)
	{
	  /* Don't turn BFD_RELOC_X86_64_32S into BFD_RELOC_32.  */
	  code = fixp->fx_r_type;
	  break;
	}
      /* Fall through.  */
    default:
      if (fixp->fx_pcrel)
	{
	  switch (fixp->fx_size)
	    {
	    default:
	      as_bad_where (fixp->fx_file, fixp->fx_line,
			    _("can not do %d byte pc-relative relocation"),
			    fixp->fx_size);
	      code = BFD_RELOC_32_PCREL;
	      break;
	    case 1: code = BFD_RELOC_8_PCREL;  break;
	    case 2: code = BFD_RELOC_16_PCREL; break;
	    case 4: code = BFD_RELOC_32_PCREL; break;
#ifdef BFD64
	    case 8: code = BFD_RELOC_64_PCREL; break;
#endif
	    }
	}
      else
	{
	  switch (fixp->fx_size)
	    {
	    default:
	      as_bad_where (fixp->fx_file, fixp->fx_line,
			    _("can not do %d byte relocation"),
			    fixp->fx_size);
	      code = BFD_RELOC_32;
	      break;
	    case 1: code = BFD_RELOC_8;  break;
	    case 2: code = BFD_RELOC_16; break;
	    case 4: code = BFD_RELOC_32; break;
#ifdef BFD64
	    case 8: code = BFD_RELOC_64; break;
#endif
	    }
	}
      break;
    }

  if ((code == BFD_RELOC_32
       || code == BFD_RELOC_32_PCREL
       || code == BFD_RELOC_X86_64_32S)
      && GOT_symbol
      && fixp->fx_addsy == GOT_symbol)
    {
      if (!object_64bit)
	code = BFD_RELOC_386_GOTPC;
      else
	code = BFD_RELOC_X86_64_GOTPC32;
    }
  if ((code == BFD_RELOC_64 || code == BFD_RELOC_64_PCREL)
      && GOT_symbol
      && fixp->fx_addsy == GOT_symbol)
    {
      code = BFD_RELOC_X86_64_GOTPC64;
    }

  rel = XNEW (arelent);
  rel->sym_ptr_ptr = XNEW (asymbol *);
  *rel->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);

  rel->address = fixp->fx_frag->fr_address + fixp->fx_where;

  if (!use_rela_relocations)
    {
      /* HACK: Since i386 ELF uses Rel instead of Rela, encode the
	 vtable entry to be used in the relocation's section offset.  */
      if (fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
	rel->address = fixp->fx_offset;
#if defined (OBJ_COFF) && defined (TE_PE)
      else if (fixp->fx_addsy && S_IS_WEAK (fixp->fx_addsy))
	rel->addend = fixp->fx_addnumber - (S_GET_VALUE (fixp->fx_addsy) * 2);
      else
#endif
      rel->addend = 0;
    }
  /* Use the rela in 64bit mode.  */
  else
    {
      if (disallow_64bit_reloc)
	switch (code)
	  {
	  case BFD_RELOC_X86_64_DTPOFF64:
	  case BFD_RELOC_X86_64_TPOFF64:
	  case BFD_RELOC_64_PCREL:
	  case BFD_RELOC_X86_64_GOTOFF64:
	  case BFD_RELOC_X86_64_GOT64:
	  case BFD_RELOC_X86_64_GOTPCREL64:
	  case BFD_RELOC_X86_64_GOTPC64:
	  case BFD_RELOC_X86_64_GOTPLT64:
	  case BFD_RELOC_X86_64_PLTOFF64:
	    as_bad_where (fixp->fx_file, fixp->fx_line,
			  _("cannot represent relocation type %s in x32 mode"),
			  bfd_get_reloc_code_name (code));
	    break;
	  default:
	    break;
	  }

      if (!fixp->fx_pcrel)
	rel->addend = fixp->fx_offset;
      else
	switch (code)
	  {
	  case BFD_RELOC_X86_64_PLT32:
	  case BFD_RELOC_X86_64_GOT32:
	  case BFD_RELOC_X86_64_GOTPCREL:
	  case BFD_RELOC_X86_64_GOTPCRELX:
	  case BFD_RELOC_X86_64_REX_GOTPCRELX:
	  case BFD_RELOC_X86_64_TLSGD:
	  case BFD_RELOC_X86_64_TLSLD:
	  case BFD_RELOC_X86_64_GOTTPOFF:
	  case BFD_RELOC_X86_64_GOTPC32_TLSDESC:
	  case BFD_RELOC_X86_64_TLSDESC_CALL:
	    rel->addend = fixp->fx_offset - fixp->fx_size;
	    break;
	  default:
	    rel->addend = (section->vma
			   - fixp->fx_size
			   + fixp->fx_addnumber
			   + md_pcrel_from (fixp));
	    break;
	  }
    }

  rel->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (rel->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("cannot represent relocation type %s"),
		    bfd_get_reloc_code_name (code));
      /* Set howto to a garbage value so that we can keep going.  */
      rel->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_32);
      gas_assert (rel->howto != NULL);
    }

  return rel;
}

#include "tc-i386-intel.c"

void
tc_x86_parse_to_dw2regnum (expressionS *exp)
{
  int saved_naked_reg;
  char saved_register_dot;

  saved_naked_reg = allow_naked_reg;
  allow_naked_reg = 1;
  saved_register_dot = register_chars['.'];
  register_chars['.'] = '.';
  allow_pseudo_reg = 1;
  expression_and_evaluate (exp);
  allow_pseudo_reg = 0;
  register_chars['.'] = saved_register_dot;
  allow_naked_reg = saved_naked_reg;

  if (exp->X_op == O_register && exp->X_add_number >= 0)
    {
      if ((addressT) exp->X_add_number < i386_regtab_size)
	{
	  exp->X_op = O_constant;
	  exp->X_add_number = i386_regtab[exp->X_add_number]
			      .dw2_regnum[flag_code >> 1];
	}
      else
	exp->X_op = O_illegal;
    }
}

void
tc_x86_frame_initial_instructions (void)
{
  static unsigned int sp_regno[2];

  if (!sp_regno[flag_code >> 1])
    {
      char *saved_input = input_line_pointer;
      char sp[][4] = {"esp", "rsp"};
      expressionS exp;

      input_line_pointer = sp[flag_code >> 1];
      tc_x86_parse_to_dw2regnum (&exp);
      gas_assert (exp.X_op == O_constant);
      sp_regno[flag_code >> 1] = exp.X_add_number;
      input_line_pointer = saved_input;
    }

  cfi_add_CFA_def_cfa (sp_regno[flag_code >> 1], -x86_cie_data_alignment);
  cfi_add_CFA_offset (x86_dwarf2_return_column, x86_cie_data_alignment);
}

int
x86_dwarf2_addr_size (void)
{
#if defined (OBJ_MAYBE_ELF) || defined (OBJ_ELF)
  if (x86_elf_abi == X86_64_X32_ABI)
    return 4;
#endif
  return bfd_arch_bits_per_address (stdoutput) / 8;
}

int
i386_elf_section_type (const char *str, size_t len)
{
  if (flag_code == CODE_64BIT
      && len == sizeof ("unwind") - 1
      && startswith (str, "unwind"))
    return SHT_X86_64_UNWIND;

  return -1;
}

#ifdef TE_SOLARIS
void
i386_solaris_fix_up_eh_frame (segT sec)
{
  if (flag_code == CODE_64BIT)
    elf_section_type (sec) = SHT_X86_64_UNWIND;
}
#endif

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

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
/* For ELF on x86-64, add support for SHF_X86_64_LARGE.  */

bfd_vma
x86_64_section_letter (int letter, const char **ptr_msg)
{
  if (flag_code == CODE_64BIT)
    {
      if (letter == 'l')
	return SHF_X86_64_LARGE;

      *ptr_msg = _("bad .section directive: want a,l,w,x,M,S,G,T in string");
    }
  else
    *ptr_msg = _("bad .section directive: want a,w,x,M,S,G,T in string");
  return -1;
}

bfd_vma
x86_64_section_word (char *str, size_t len)
{
  if (len == 5 && flag_code == CODE_64BIT && startswith (str, "large"))
    return SHF_X86_64_LARGE;

  return -1;
}

static void
handle_large_common (int small ATTRIBUTE_UNUSED)
{
  if (flag_code != CODE_64BIT)
    {
      s_comm_internal (0, elf_common_parse);
      as_warn (_(".largecomm supported only in 64bit mode, producing .comm"));
    }
  else
    {
      static segT lbss_section;
      asection *saved_com_section_ptr = elf_com_section_ptr;
      asection *saved_bss_section = bss_section;

      if (lbss_section == NULL)
	{
	  flagword applicable;
	  segT seg = now_seg;
	  subsegT subseg = now_subseg;

	  /* The .lbss section is for local .largecomm symbols.  */
	  lbss_section = subseg_new (".lbss", 0);
	  applicable = bfd_applicable_section_flags (stdoutput);
	  bfd_set_section_flags (lbss_section, applicable & SEC_ALLOC);
	  seg_info (lbss_section)->bss = 1;

	  subseg_set (seg, subseg);
	}

      elf_com_section_ptr = &_bfd_elf_large_com_section;
      bss_section = lbss_section;

      s_comm_internal (0, elf_common_parse);

      elf_com_section_ptr = saved_com_section_ptr;
      bss_section = saved_bss_section;
    }
}
#endif /* OBJ_ELF || OBJ_MAYBE_ELF */
