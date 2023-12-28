/* tc-bpf.c -- Assembler for the Linux eBPF.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.
   Contributed by Oracle, Inc.

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
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "as.h"
#include "subsegs.h"
#include "symcat.h"
#include "opcodes/bpf-desc.h"
#include "opcodes/bpf-opc.h"
#include "cgen.h"
#include "elf/common.h"
#include "elf/bpf.h"
#include "dwarf2dbg.h"
#include <ctype.h>

const char comment_chars[]        = ";";
const char line_comment_chars[]   = "#";
const char line_separator_chars[] = "`";
const char EXP_CHARS[]            = "eE";
const char FLT_CHARS[]            = "fFdD";

static const char *invalid_expression;
static char pseudoc_lex[256];
static const char symbol_chars[] =
"_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

static const char arithm_op[] = "+-/<>%&|^";

static void init_pseudoc_lex (void);

#define LEX_IS_SYMBOL_COMPONENT  1
#define LEX_IS_WHITESPACE        2
#define LEX_IS_NEWLINE           3
#define LEX_IS_ARITHM_OP         4
#define LEX_IS_STAR              6
#define LEX_IS_CLSE_BR           7
#define LEX_IS_OPEN_BR           8
#define LEX_IS_EQUAL             9
#define LEX_IS_EXCLA             10

#define ST_EOI        100
#define MAX_TOKEN_SZ  100

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

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
    { "half",      cons,              2 },
    { "word",      cons,              4 },
    { "dword",     cons,              8 },
    { "lcomm",	   pe_lcomm,	      1 },
    { NULL,        NULL,              0 }
};



/* ISA handling.  */
static CGEN_BITSET *bpf_isa;



/* Command-line options processing.  */

enum options
{
  OPTION_LITTLE_ENDIAN = OPTION_MD_BASE,
  OPTION_BIG_ENDIAN,
  OPTION_XBPF
};

struct option md_longopts[] =
{
  { "EL", no_argument, NULL, OPTION_LITTLE_ENDIAN },
  { "EB", no_argument, NULL, OPTION_BIG_ENDIAN },
  { "mxbpf", no_argument, NULL, OPTION_XBPF },
  { NULL,          no_argument, NULL, 0 },
};

size_t md_longopts_size = sizeof (md_longopts);

const char * md_shortopts = "";

extern int target_big_endian;

/* Whether target_big_endian has been set while parsing command-line
   arguments.  */
static int set_target_endian = 0;

static int target_xbpf = 0;

static int set_xbpf = 0;

int
md_parse_option (int c, const char * arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    case OPTION_BIG_ENDIAN:
      set_target_endian = 1;
      target_big_endian = 1;
      break;
    case OPTION_LITTLE_ENDIAN:
      set_target_endian = 1;
      target_big_endian = 0;
      break;
    case OPTION_XBPF:
      set_xbpf = 1;
      target_xbpf = 1;
      break;
    default:
      return 0;
    }

  return 1;
}

void
md_show_usage (FILE * stream)
{
  fprintf (stream, _("\nBPF options:\n"));
  fprintf (stream, _("\
  --EL			generate code for a little endian machine\n\
  --EB			generate code for a big endian machine\n\
  -mxbpf                generate xBPF instructions\n"));
}



static void
init_pseudoc_lex (void)
{
  const char *p;

  for (p = symbol_chars; *p; ++p)
    pseudoc_lex[(unsigned char) *p] = LEX_IS_SYMBOL_COMPONENT;

  pseudoc_lex[' '] = LEX_IS_WHITESPACE;
  pseudoc_lex['\t'] = LEX_IS_WHITESPACE;
  pseudoc_lex['\r'] = LEX_IS_WHITESPACE;
  pseudoc_lex['\n'] = LEX_IS_NEWLINE;
  pseudoc_lex['*'] = LEX_IS_STAR;
  pseudoc_lex[')'] = LEX_IS_CLSE_BR;
  pseudoc_lex['('] = LEX_IS_OPEN_BR;
  pseudoc_lex[']'] = LEX_IS_CLSE_BR;
  pseudoc_lex['['] = LEX_IS_OPEN_BR;

  for (p = arithm_op; *p; ++p)
    pseudoc_lex[(unsigned char) *p] = LEX_IS_ARITHM_OP;

  pseudoc_lex['='] = LEX_IS_EQUAL;
  pseudoc_lex['!'] = LEX_IS_EXCLA;
}

void
md_begin (void)
{
  /* Initialize the `cgen' interface.  */

  /* If not specified in the command line, use the host
     endianness.  */
  if (!set_target_endian)
    {
#ifdef WORDS_BIGENDIAN
      target_big_endian = 1;
#else
      target_big_endian = 0;
#endif
    }

  /* If not specified in the command line, use eBPF rather
     than xBPF.  */
  if (!set_xbpf)
      target_xbpf = 0;

  /* Set the ISA, which depends on the target endianness. */
  bpf_isa = cgen_bitset_create (ISA_MAX);
  if (target_big_endian)
    {
      if (target_xbpf)
	cgen_bitset_set (bpf_isa, ISA_XBPFBE);
      else
	cgen_bitset_set (bpf_isa, ISA_EBPFBE);
    }
  else
    {
      if (target_xbpf)
	cgen_bitset_set (bpf_isa, ISA_XBPFLE);
      else
	cgen_bitset_set (bpf_isa, ISA_EBPFLE);
    }

  /* Ensure that lines can begin with '*' in BPF store pseudoc instruction.  */
  lex_type['*'] |= LEX_BEGIN_NAME;

  /* Set the machine number and endian.  */
  gas_cgen_cpu_desc = bpf_cgen_cpu_open (CGEN_CPU_OPEN_ENDIAN,
                                         target_big_endian ?
                                         CGEN_ENDIAN_BIG : CGEN_ENDIAN_LITTLE,
                                         CGEN_CPU_OPEN_INSN_ENDIAN,
                                         CGEN_ENDIAN_LITTLE,
                                         CGEN_CPU_OPEN_ISAS,
                                         bpf_isa,
                                         CGEN_CPU_OPEN_END);
  bpf_cgen_init_asm (gas_cgen_cpu_desc);

  /* This is a callback from cgen to gas to parse operands.  */
  cgen_set_parse_operand_fn (gas_cgen_cpu_desc, gas_cgen_parse_operand);

  /* Set the machine type. */
  bfd_default_set_arch_mach (stdoutput, bfd_arch_bpf, bfd_mach_bpf);
  init_pseudoc_lex();
}

valueT
md_section_align (segT segment, valueT size)
{
  int align = bfd_section_alignment (segment);

  return ((size + (1 << align) - 1) & -(1 << align));
}


/* Functions concerning relocs.  */

/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from_section (fixS *fixP, segT sec)
{
  if (fixP->fx_addsy != (symbolS *) NULL
      && (! S_IS_DEFINED (fixP->fx_addsy)
          || (S_GET_SEGMENT (fixP->fx_addsy) != sec)
          || S_IS_EXTERNAL (fixP->fx_addsy)
          || S_IS_WEAK (fixP->fx_addsy)))
    {
        /* The symbol is undefined (or is defined but not in this section).
         Let the linker figure it out.  */
      return 0;
    }

  return fixP->fx_where + fixP->fx_frag->fr_address;
}

/* Write a value out to the object file, using the appropriate endianness.  */

void
md_number_to_chars (char * buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

arelent *
tc_gen_reloc (asection *sec, fixS *fix)
{
  return gas_cgen_tc_gen_reloc (sec, fix);
}

/* Return the bfd reloc type for OPERAND of INSN at fixup FIXP.  This
   is called when the operand is an expression that couldn't be fully
   resolved.  Returns BFD_RELOC_NONE if no reloc type can be found.
   *FIXP may be modified if desired.  */

bfd_reloc_code_real_type
md_cgen_lookup_reloc (const CGEN_INSN *insn ATTRIBUTE_UNUSED,
		      const CGEN_OPERAND *operand,
		      fixS *fixP)
{
  switch (operand->type)
    {
    case BPF_OPERAND_IMM64:
      return BFD_RELOC_BPF_64;
    case BPF_OPERAND_DISP32:
      fixP->fx_pcrel = 1;
      return BFD_RELOC_BPF_DISP32;
    default:
      break;
    }
  return BFD_RELOC_NONE;
}

/* *FRAGP has been relaxed to its final size, and now needs to have
   the bytes inside it modified to conform to the new size.

   Called after relaxation is finished.
   fragP->fr_type == rs_machine_dependent.
   fragP->fr_subtype is the subtype of what the address relaxed to.  */

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED,
		 segT sec ATTRIBUTE_UNUSED,
		 fragS *fragP ATTRIBUTE_UNUSED)
{
  as_fatal (_("convert_frag called"));
}

int
md_estimate_size_before_relax (fragS *fragP ATTRIBUTE_UNUSED,
                               segT segment ATTRIBUTE_UNUSED)
{
  as_fatal (_("estimate_size_before_relax called"));
  return 0;
}


void
md_apply_fix (fixS *fixP, valueT *valP, segT seg)
{
  /* Some fixups for instructions require special attention.  This is
     handled in the code block below.  */
  if ((int) fixP->fx_r_type >= (int) BFD_RELOC_UNUSED)
    {
      int opindex = (int) fixP->fx_r_type - (int) BFD_RELOC_UNUSED;
      const CGEN_OPERAND *operand = cgen_operand_lookup_by_num (gas_cgen_cpu_desc,
                                                                opindex);
      char *where;

      switch (operand->type)
        {
        case BPF_OPERAND_DISP32:
          /* eBPF supports two kind of CALL instructions: the so
             called pseudo calls ("bpf to bpf") and external calls
             ("bpf to kernel").

             Both kind of calls use the same instruction (CALL).
             However, external calls are constructed by passing a
             constant argument to the instruction, whereas pseudo
             calls result from expressions involving symbols.  In
             practice, instructions requiring a fixup are interpreted
             as pseudo-calls.  If we are executing this code, this is
             a pseudo call.

             The kernel expects for pseudo-calls to be annotated by
             having BPF_PSEUDO_CALL in the SRC field of the
             instruction.  But beware the infamous nibble-swapping of
             eBPF and take endianness into account here.

             Note that the CALL instruction has only one operand, so
             this code is executed only once per instruction.  */
          where = fixP->fx_frag->fr_literal + fixP->fx_where + 1;
          where[0] = target_big_endian ? 0x01 : 0x10;
          /* Fallthrough.  */
        case BPF_OPERAND_DISP16:
          /* The PC-relative displacement fields in jump instructions
             shouldn't be in bytes.  Instead, they hold the number of
             64-bit words to the target, _minus one_.  */ 
          *valP = (((long) (*valP)) - 8) / 8;
          break;
        default:
          break;
        }
    }

  /* And now invoke CGEN's handler, which will eventually install
     *valP into the corresponding operand.  */
  gas_cgen_md_apply_fix (fixP, valP, seg);
}

/*
  The BPF pseudo grammar:

	instruction  : bpf_alu_insn
		     | bpf_alu32_insn
		     | bpf_jump_insn
		     | bpf_load_store_insn
		     | bpf_load_store32_insn
		     | bpf_non_generic_load
		     | bpf_endianness_conv_insn
		     | bpf_64_imm_load_insn
		     | bpf_atomic_insn
		     ;

	bpf_alu_insn : BPF_REG bpf_alu_operator register_or_imm32
		     ;

	bpf_alu32_insn : BPF_REG32 bpf_alu_operator register32_or_imm32
		       ;

	bpf_jump_insn  : BPF_JA offset
		       | IF BPF_REG bpf_jump_operator register_or_imm32 BPF_JA offset
		       | IF BPF_REG32 bpf_jump_operator register_or_imm32 BPF_JA offset
		       | BPF_CALL offset
		       | BPF_EXIT
		       ;

	bpf_load_store_insn  : BPF_REG CHR_EQUAL bpf_size_cast BPF_CHR_OPEN_BR \
			       register_and_offset BPF_CHR_CLSE_BR
			     | bpf_size_cast register_and_offset CHR_EQUAL BPF_REG
			     ;

	bpf_load_store32_insn  : BPF_REG CHR_EQUAL bpf_size_cast BPF_CHR_OPEN_BR \
				 register32_and_offset BPF_CHR_CLSE_BR
			       | bpf_size_cast register_and_offset CHR_EQUAL BPF_REG32
			     ;

	bpf_non_generic_load : BPF_REG_R0 CHR_EQUAL bpf_size_cast BPF_LD BPF_CHR_OPEN_BR \
			       imm32 BPF_CHR_CLSE_BR
			     ;

	bpf_endianness_conv_insn : BPF_REG_N bpf_endianness_mnem BPF_REG_N
				 ;

	bpf_64_imm_load_insn : BPF_REG imm64 BPF_LL
			     ;

	bpf_atomic_insn : BPF_LOCK bpf_size_cast_32_64 register_and_offset BPF_ADD BPF_REG

	register_and_offset : BPF_CHR_OPEN_BR BPF_REG offset BPF_CHR_CLSE_BR
			    ;

	register32_and_offset : BPF_CHR_OPEN_BR BPF_REG32 offset BPF_CHR_CLSE_BR
			      ;

	bpf_size_cast : CHR_START BPF_CHR_OPEN_BR bpf_size CHR_START BPF_CHR_CLSE_BR
		      ;

	bpf_size_cast_32_64 : CHR_START BPF_CHR_OPEN_BR bpf_size_cast_32_64 CHR_STAR BPF_CHR_CLSE_BR
			    ;

	bpf_size_32_64 : BPF_CAST_U32
		       | BPF_CAST_U64
		       ;

	bpf_size  : BPF_CAST_U8
		  | BPF_CAST_U16
		  | BPF_CAST_U32
		  | BPF_CAST_U64
		  ;

	bpf_jump_operator : BPF_JEQ
			  | BPF_JGT
			  | BPF_JGE
			  | BPF_JNE
			  | BPF_JSGT
			  | BPF_JSGE
			  | BPF_JLT
			  | BPF_JLE
			  | BPF_JSLT
			  | BPF_JSLE
			  ;

	bpf_alu_operator : BPF_ADD
			 | BPF_SUB
			 | BPF_MUL
			 | BPF_DIV
			 | BPF_OR
			 | BPF_AND
			 | BPF_LSH
			 | BPF_RSH
			 | BPF_NEG
			 | BPF_MOD
			 | BPF_XOR
			 | BPF_ARSH
			 | CHR_EQUAL
			 ;

	bpf_endianness_mnem : BPF_LE16
			    | BPF_LE32
			    | BPF_LE64
			    | BPF_BE16
			    | BPF_BE32
			    | BPF_BE64
			    ;

	offset : BPF_EXPR
	       | BPF_SYMBOL
	       ;

	register_or_imm32 : BPF_REG
			  | expression
			  ;

	register32_or_imm32 : BPF_REG32
			    | expression
			    ;

	imm32 : BPF_EXPR
	      | BPF_SYMBOL
	      ;

	imm64 : BPF_EXPR
	      | BPF_SYMBOL
	      ;

	register_or_expression : BPF_EXPR
			       | BPF_REG
			       ;

	BPF_EXPR : GAS_EXPR

*/

enum bpf_token_type
  {
    /* Keep grouped to quickly access. */
    BPF_ADD,
    BPF_SUB,
    BPF_MUL,
    BPF_DIV,
    BPF_OR,
    BPF_AND,
    BPF_LSH,
    BPF_RSH,
    BPF_MOD,
    BPF_XOR,
    BPF_MOV,
    BPF_ARSH,
    BPF_NEG,

    BPF_REG,

    BPF_IF,
    BPF_GOTO,

    /* Keep grouped to quickly access.  */
    BPF_JEQ,
    BPF_JGT,
    BPF_JGE,
    BPF_JLT,
    BPF_JLE,
    BPF_JSET,
    BPF_JNE,
    BPF_JSGT,
    BPF_JSGE,
    BPF_JSLT,
    BPF_JSLE,

    BPF_SYMBOL,
    BPF_CHR_CLSE_BR,
    BPF_CHR_OPEN_BR,

    /* Keep grouped to quickly access.  */
    BPF_CAST_U8,
    BPF_CAST_U16,
    BPF_CAST_U32,
    BPF_CAST_U64,

    /* Keep grouped to quickly access.  */
    BPF_LE16,
    BPF_LE32,
    BPF_LE64,
    BPF_BE16,
    BPF_BE32,
    BPF_BE64,

    BPF_LOCK,

    BPF_IND_CALL,
    BPF_LD,
    BPF_LL,
    BPF_EXPR,
    BPF_UNKNOWN,
  };

static int
valid_expr (const char *e, const char **end_expr)
{
  invalid_expression = NULL;
  char *hold = input_line_pointer;
  expressionS exp;

  input_line_pointer = (char *) e;
  deferred_expression  (&exp);
  *end_expr = input_line_pointer;
  input_line_pointer = hold;

  return invalid_expression == NULL;
}

static char *
build_bpf_non_generic_load (char *src, enum bpf_token_type cast,
			    const char *imm32)
{
  char *bpf_insn;
  static const char *cast_rw[] = {"b", "h", "w", "dw"};

  bpf_insn = xasprintf ("%s%s%s %s%s%s%s",
			"ld",
			src ? "ind" : "abs",
			cast_rw[cast - BPF_CAST_U8],
			src ? "%" : "",
			src ? src : "",
			src ? "," : "",
			imm32);
  return bpf_insn;
}

static char *
build_bpf_atomic_insn (char *dst, char *src,
		       enum bpf_token_type atomic_insn,
		       enum bpf_token_type cast,
		       const char *offset)
{
  char *bpf_insn;
  static const char *cast_rw[] = {"w", "dw"};
  static const char *mnem[] = {"xadd"};

  bpf_insn = xasprintf ("%s%s [%%%s%s%s],%%%s", mnem[atomic_insn - BPF_ADD],
			cast_rw[cast - BPF_CAST_U32], dst,
			*offset != '+' ? "+" : "",
			offset, src);
  return bpf_insn;
}

static char *
build_bpf_jmp_insn (char *dst, char *src,
		    char *imm32, enum bpf_token_type op,
		    const char *sym, const char *offset)
{
  char *bpf_insn;
  static const char *mnem[] =
    {
      "jeq", "jgt", "jge", "jlt",
      "jle", "jset", "jne", "jsgt",
      "jsge", "jslt", "jsle"
    };

  const char *in32 = (*dst == 'w' ? "32" : "");

  *dst = 'r';
  if (src)
    *src = 'r';

  bpf_insn = xasprintf ("%s%s %%%s,%s%s,%s",
			mnem[op - BPF_JEQ], in32, dst,
			src ? "%" : "",
			src ? src : imm32,
			offset ? offset : sym);
  return bpf_insn;
}

static char *
build_bpf_arithm_insn (char *dst, char *src,
		       int load64, const char *imm32,
		       enum bpf_token_type type)
{
  char *bpf_insn;
  static const char *mnem[] =
    {
      "add", "sub", "mul", "div",
      "or", "and", "lsh", "rsh",
      "mod", "xor", "mov", "arsh",
      "neg",
    };
  const char *in32 = (*dst == 'w' ? "32" : "");

  *dst = 'r';
  if (src)
    *src = 'r';

  if (type == BPF_NEG)
    bpf_insn = xasprintf ("%s%s %%%s", mnem[type - BPF_ADD], in32, dst);
  else if (load64)
    bpf_insn = xasprintf ("%s %%%s,%s", "lddw", dst, imm32);
  else
    bpf_insn = xasprintf ("%s%s %%%s,%s%s", mnem[type - BPF_ADD],
			  in32, dst,
			  src ? "%" : "",
			  src ? src: imm32);
  return bpf_insn;
}

static char *
build_bpf_endianness (char *dst, enum bpf_token_type endianness)
{
  char *bpf_insn;
  static const char *size[] = {"16", "32", "64"};
  int be = 1;

  if (endianness == BPF_LE16
      || endianness == BPF_LE32
      || endianness == BPF_LE64)
    be = 0;
  else
    gas_assert (endianness == BPF_BE16 || endianness == BPF_BE32 || endianness == BPF_BE64);

  bpf_insn = xasprintf ("%s %%%s,%s", be ? "endbe" : "endle",
			dst, be ? size[endianness - BPF_BE16] : size[endianness - BPF_LE16]);
  return bpf_insn;
}

static char *
build_bpf_load_store_insn (char *dst, char *src,
			   enum bpf_token_type cast,
			   const char *offset, int isload)
{
  char *bpf_insn;
  static const char *cast_rw[] = {"b", "h", "w", "dw"};

  *dst = *src = 'r';
  if (isload)
    bpf_insn = xasprintf ("%s%s %%%s,[%%%s%s%s]", "ldx",
			  cast_rw[cast - BPF_CAST_U8], dst, src,
			  *offset != '+' ? "+" : "",
			  offset);
  else
    bpf_insn = xasprintf ("%s%s [%%%s%s%s],%%%s", "stx",
			  cast_rw[cast - BPF_CAST_U8], dst,
			  *offset != '+' ? "+" : "",
			  offset, src);
  return bpf_insn;
}

static int
look_for_reserved_word (const char *token, enum bpf_token_type *type)
{
  int i;
  static struct
  {
    const char *name;
    enum bpf_token_type type;
  } reserved_words[] =
    {
      {
	.name = "if",
	.type = BPF_IF
      },
      {
	.name = "goto",
	.type = BPF_GOTO
      },
      {
	.name = "le16",
	.type = BPF_LE16
      },
      {
	.name = "le32",
	.type = BPF_LE32
      },
      {
	.name = "le64",
	.type = BPF_LE64
      },
      {
	.name = "be16",
	.type = BPF_BE16
      },
      {
	.name = "be32",
	.type = BPF_BE32
      },
      {
	.name = "be64",
	.type = BPF_BE64
	},
      {
	.name = "lock",
	.type = BPF_LOCK
      },
      {
	.name = "callx",
	.type = BPF_IND_CALL
      },
      {
	.name = "skb",
	.type = BPF_LD
      },
      {
	.name = "ll",
	.type = BPF_LL
      },
      {
	.name = NULL,
      }
    };

  for (i = 0; reserved_words[i].name; ++i)
    if (*reserved_words[i].name == *token
	&& !strcmp (reserved_words[i].name, token))
      {
	*type = reserved_words[i].type;
	return 1;
      }

  return 0;
}

static int
is_register (const char *token, int len)
{
  if (token[0] == 'r' || token[0] == 'w')
    if ((len == 2 && isdigit (token[1]))
	|| (len == 3 && token[1] == '1' && token[2] == '0'))
      return 1;

  return 0;
}

static enum bpf_token_type
is_cast (const char *token)
{
  static const char *cast_rw[] = {"u8", "u16", "u32", "u64"};
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (cast_rw); ++i)
    if (!strcmp (token, cast_rw[i]))
      return BPF_CAST_U8 + i;

  return BPF_UNKNOWN;
}

static enum bpf_token_type
get_token (const char **insn, char *token, size_t *tlen)
{
#define GET()					\
  (*str == '\0'					\
   ? EOF					\
   : *(unsigned char *)(str++))

#define UNGET() (--str)

#define START_EXPR()			       \
  do					       \
    {					       \
      if (expr == NULL)			       \
	expr = str - 1;			       \
    } while (0)

#define SCANNER_SKIP_WHITESPACE()		\
  do						\
    {						\
      do					\
	ch = GET ();				\
      while (ch != EOF				\
	     && ((ch) == ' ' || (ch) == '\t'));	\
      if (ch != EOF)				\
	UNGET ();				\
    } while (0)

  const char *str = *insn;
  int ch, ch2 = 0;
  enum bpf_token_type ttype = BPF_UNKNOWN;
  size_t len = 0;
  const char *expr = NULL;
  const char *end_expr = NULL;
  int state = 0;
  int return_token = 0;

  while (1)
    {
      ch = GET ();

      if (ch == EOF || len > MAX_TOKEN_SZ)
	break;

      switch (pseudoc_lex[(unsigned char) ch])
	{
	case LEX_IS_WHITESPACE:
	  SCANNER_SKIP_WHITESPACE ();
	  return_token = 1;

	  switch (state)
	    {
	    case 12: /* >' ' */
	      ttype = BPF_JGT;
	      break;

	    case 17: /* ==' ' */
	      ttype = BPF_JEQ;
	      break;

	    case 18: /* <' ' */
	      ttype = BPF_JLT;
	      break;

	    case 20: /* &' ' */
	      ttype = BPF_JSET;
	      break;

	    case 22:  /* s<' '*/
	      ttype = BPF_JSLT;
	      break;

	    case 14: /* s> ' ' */
	      ttype = BPF_JSGT;
	      break;

	    case 16: /* =' ' */
	      ttype = BPF_MOV;
	      break;

	    default:
	      return_token = 0;
	    }
	  break;

	case LEX_IS_EXCLA:
	  token[len++] = ch;
	  state = 21;
	  break;

	case LEX_IS_ARITHM_OP:
	  if (state == 16)
	    {
	      /* ='-' is handle as '=' */
	      UNGET ();
	      ttype = BPF_MOV;
	      return_token = 1;
	      break;
	    }

	  START_EXPR();
	  token[len++] = ch;
	  switch (ch)
	    {
#define BPF_ARITHM_OP(op, type)			\
	      case (op):			\
		state = 6;			\
		ttype = (type);			\
		break;

	      BPF_ARITHM_OP('+', BPF_ADD);
	      BPF_ARITHM_OP('-', BPF_SUB);
	      BPF_ARITHM_OP('*', BPF_MUL);
	      BPF_ARITHM_OP('/', BPF_DIV);
	      BPF_ARITHM_OP('|', BPF_OR);
	      BPF_ARITHM_OP('%', BPF_MOD);
	      BPF_ARITHM_OP('^', BPF_XOR);

	    case '&':
	      state = 20; /* '&' */
	      break;

	    case '<':
	      switch (state)
		{
		case 0:
		  state = 18; /* '<' */
		  break;

		case 18:
		  state = 19; /* <'<' */
		  break;

		case 8:
		  state = 22; /* s'<' */
		  break;
		}
	      break;

	    case '>':
	      switch (state)
		{
		case 0:
		  state = 12; /* '>' */
		  break;

		case 12:
		  state = 13; /* >'>' */
		  break;

		case 8:
		  state = 14; /* s'>' */
		  break;

		case 14:
		  state = 15; /* s>'>' */
		  break;
		}
	      break;
	    }
	  break;

	case LEX_IS_STAR:
	  switch (state)
	    {
	    case 0:
	      token[len++] = ch;
	      START_EXPR ();
	      state = 2; /* '*', It could be the fist cast char.  */
	      break;

	    case 16: /* ='*' Not valid token.  */
	      ttype = BPF_MOV;
	      return_token = 1;
	      UNGET ();
	      break;

	    case 4: /* *(uXX'*' */
	      token[len++] = ch;
	      state = 5;
	      break;
	    }
	  break;

	case LEX_IS_OPEN_BR:
	  START_EXPR ();
	  token[len++] = ch;
	  return_token = 1;

	  switch (state)
	    {
	    case 2:
	      state = 3; /* *'(' second char of a cast or expr.  */
	      return_token = 0;
	      break;

	    case 6:
	      if (valid_expr (expr, &end_expr))
		{
		  len = end_expr - expr;
		  memcpy (token, expr, len);
		  ttype = BPF_EXPR;
		  str = end_expr;
		}
	      else
		{
		  len = 0;
		  while (*invalid_expression)
		    token[len++] = *invalid_expression++;

		  token[len] = 0;
		  ttype = BPF_UNKNOWN;
		}
	      break;

	    default:
	      ttype = BPF_CHR_OPEN_BR;
	      SCANNER_SKIP_WHITESPACE ();
	      ch2 = GET ();

	      if ((isdigit (ch2) || ch2 == '(')
		  && valid_expr (expr, &end_expr))
		{
		  len = end_expr - expr;
		  memcpy (token, expr, len);
		  ttype = BPF_EXPR;
		  str = end_expr;
		}
	      else
		UNGET ();
	    }
	  break;

	case LEX_IS_CLSE_BR:
	  token[len++] = ch;

	  if (state == 0)
	    {
	      ttype = BPF_CHR_CLSE_BR;
	      return_token = 1;
	    }
	  else if (state == 5) /* *(uXX*')'  */
	    return_token = 1;
	  break;

	case LEX_IS_EQUAL:
	  token[len++] = ch;
	  return_token = 1;

	  switch (state)
	    {
	    case 0:
	      state = 16; /* '=' */
	      return_token = 0;
	      break;

	    case 16:
	      state = 17; /* ='=' */
	      return_token = 0;
	      break;

	    case 2: /* *'=' */
	      ttype = BPF_MUL;
	      break;

	    case 10: /* s>>'=' */
	      ttype = BPF_ARSH;
	      break;

	    case 12: /* >'=' */
	      ttype = BPF_JGE;
	      break;

	    case 13: /* >>'=' */
	      ttype = BPF_RSH;
	      break;

	    case 14: /* s>'=' */
	      ttype = BPF_JSGE;
	      break;

	    case 15: /* s>>'=' */
	      ttype = BPF_ARSH;
	      break;

	    case 18: /* <'=' */
	      ttype = BPF_JLE;
	      break;

	    case 19: /* <<'=' */
	      ttype = BPF_LSH;
	      break;

	    case 20: /* &'=' */
	      ttype = BPF_AND;
	      break;

	    case 21: /* !'=' */
	      ttype = BPF_JNE;
	      break;

	    case 22: /* s<'=' */
	      ttype = BPF_JSLE;
	      break;
	    }
	  break;

	case LEX_IS_SYMBOL_COMPONENT:
	  return_token = 1;

	  switch (state)
	    {
	    case 17: /* =='sym' */
	      ttype = BPF_JEQ;
	      break;

	    case 12: /* >'sym' */
	      ttype = BPF_JGT;
	      break;

	    case 18: /* <'sym' */
	      ttype = BPF_JLT;
	      break;

	    case 20: /* &'sym' */
	      ttype = BPF_JSET;
	      break;

	    case 14: /*s>'sym' */
	      ttype = BPF_JSGT;
	      break;

	    case 22:  /* s<'sym' */
	      ttype = BPF_JSLT;
	      break;

	    case 16: /* ='sym' */
	      ttype = BPF_MOV;
	      break;

	    default:
	      return_token = 0;
	    }

	  if (return_token)
	    {
	      UNGET ();
	      break;
	    }

	  START_EXPR ();
	  token[len++] = ch;

	  while ((ch2 = GET ()) != EOF)
	    {
	      int type;

	      type = pseudoc_lex[(unsigned char) ch2];
	      if (type != LEX_IS_SYMBOL_COMPONENT)
		break;
	      token[len++] = ch2;
	    }

	  if (ch2 != EOF)
	    UNGET ();

	  if (state == 0)
	    {
	      if (len == 1 && ch == 's')
		state = 8; /* signed instructions: 's' */
	      else
		{
		  ttype = BPF_SYMBOL;
		  if (is_register (token, len))
		    ttype = BPF_REG;
		  else if (look_for_reserved_word (token, &ttype))
		    ;
		  else if ((pseudoc_lex[(unsigned char) *token] == LEX_IS_ARITHM_OP
			    || *token == '(' || isdigit(*token))
			   && valid_expr (expr, &end_expr))
		    {
		      len = end_expr - expr;
		      token[len] = '\0';
		      ttype = BPF_EXPR;
		      str = end_expr;
		    }

		  return_token = 1;
		}
	    }
	  else if (state == 3) /* *('sym' */
	    {
	      if ((ttype = is_cast (&token[2])) != BPF_UNKNOWN)
		state = 4; /* *('uXX' */
	      else
		{
		  ttype = BPF_EXPR;
		  return_token = 1;
		}
	    }
	  else if (state == 6)
	    {
	      if (ttype == BPF_SUB) /* neg */
		{
		  if (is_register (&token[1], len - 1))
		    ttype =  BPF_NEG;
		  else if (valid_expr(expr, &end_expr))
		    {
		      len = end_expr - expr;
		      memcpy(token, expr, len);
		      ttype = BPF_EXPR;
		      str = end_expr;
		    }
		  else
		    {
		      len = 0;
		      while (*invalid_expression)
			token[len++] = *invalid_expression++;
		      token[len] = 0;
		      ttype = BPF_UNKNOWN;
		    }
		}
	      else if (valid_expr (expr, &end_expr))
		{
		  len = end_expr - expr;
		  memcpy(token, expr, len);
		  ttype = BPF_EXPR;
		  str = end_expr;
		}
	      else
		ttype = BPF_UNKNOWN;

	      return_token = 1;
	    }
	  break;
	}

      if (return_token)
	{
	  *tlen = len;
	  *insn = str;
	  break;
	}
    }

  return ttype;

#undef GET
#undef UNGET
#undef START_EXPR
#undef SCANNER_SKIP_WHITESPACE
#undef BPF_ARITHM_OP
}

/*
  The parser represent a FSM for the grammar described above. So for example
  the following rule:

     ` bpf_alu_insn : BPF_REG bpf_alu_operator register_or_imm32'

  Is parser as follows:

      1. It starts in state 0.

      2. Consumes next token, e.g: `BPF_REG' and set `state' variable to a
      particular state to helps to identify, in this case, that a register
      token has been read, a comment surrounded by a single quote in the
      pseudo-c token is added along with the new `state' value to indicate
      what the scanner has read, e.g.:

          state = 6; // dst_reg = str_cast ( 'src_reg'

      So, in `state 6' the scanner has consumed: a destination register
      (BPF_REG), an equal character (BPF_MOV), a cast token (BPF_CAST), an
      open parenthesis (BPF_CHR_OPEN_BR) and the source register (BPF_REG).

      3. If the accumulated tokens represent a complete BPF pseudo-c syntax
      instruction then, a validation of the terms is made, for example: if
      the registers have the same sizes (32/64 bits), if a specific
      destination register must be used, etc., after that, a builder:
      build_bfp_{non_generic_load,atomic_insn,jmp_insn,arithm_insn,endianness,load_store_insn}
      is invoked, internally, it translates the BPF pseudo-c instruction to
      a BPF GAS instruction using the previous terms recollected by the
      scanner.

      4. If a successful build of BPF GAS instruction was done, a final
      state is set to `ST_EOI' (End Of Instruction) meaning that is not
      expecting for more tokens in such instruction.  Otherwise if the
      conditions to calling builder are not satisfied an error is emitted
      and `parse_err' is set.
*/

static char *
bpf_pseudoc_to_normal_syntax (const char *str, char **errmsg)
{
#define syntax_err(format, ...)						\
  do									\
    {									\
      if (! parse_err)							\
	{								\
	  parse_err = 1;						\
	  errbuf = xasprintf (format, ##__VA_ARGS__);			\
	}								\
    } while (0)

  enum bpf_token_type ttype;
  enum bpf_token_type bpf_endianness = BPF_UNKNOWN,
		      bpf_atomic_insn;
  enum bpf_token_type bpf_jmp_op = BPF_JEQ; /* Arbitrary.  */
  enum bpf_token_type bpf_cast = BPF_CAST_U8; /* Arbitrary.  */
  enum bpf_token_type bpf_arithm_op = BPF_ADD; /* Arbitrary.  */
  char *bpf_insn = NULL;
  char *errbuf = NULL;
  char src_reg[3] = {0};
  char dst_reg[3] = {0};
  char str_imm32[40] = {0};
  char str_offset[40] = {0};
  char str_symbol[MAX_TOKEN_SZ] = {0};
  char token[MAX_TOKEN_SZ] = {0};
  int state = 0;
  int parse_err = 0;
  size_t tlen;

  while (*str)
    {
      ttype = get_token (&str, token, &tlen);
      if (ttype == BPF_UNKNOWN || state == ST_EOI)
	{
	  syntax_err ("unexpected token: '%s'", token);
	  break;
	}

      switch (ttype)
	{
	case BPF_UNKNOWN:
	case BPF_LL:
	  break;

	case BPF_REG:
	  switch (state)
	    {
	    case 0:
	      memcpy (dst_reg, token, tlen);
	      state = 1; /* 'dst_reg' */
	      break;

	    case 3:
	      /* dst_reg bpf_op 'src_reg' */
	      memcpy (src_reg, token, tlen);
	      if (*dst_reg == *src_reg)
		bpf_insn = build_bpf_arithm_insn (dst_reg, src_reg, 0,
						  NULL, bpf_arithm_op);
	      else
		{
		  syntax_err ("different register sizes: '%s', '%s'",
			      dst_reg, src_reg);
		  break;
		}
	      state = ST_EOI;
	      break;

	    case 5:
	      memcpy (src_reg, token, tlen);
	      state = 6; /* dst_reg = str_cast ( 'src_reg' */
	      break;

	    case 9:
	      memcpy (dst_reg, token, tlen);
	      state = 10; /* str_cast ( 'dst_reg' */
	      break;

	    case 11:
	      /* str_cast ( dst_reg offset ) = 'src_reg' */
	      memcpy (src_reg, token, tlen);
	      bpf_insn = build_bpf_load_store_insn (dst_reg, src_reg,
						    bpf_cast, str_offset, 0);
	      state = ST_EOI;
	      break;

	    case 14:
	      memcpy (dst_reg, token, tlen);
	      state = 15; /* if 'dst_reg' */
	      break;

	    case 16:
	      memcpy (src_reg, token, tlen);
	      state = 17; /* if dst_reg jmp_op 'src_reg' */
	      break;

	    case 24:
	      /* dst_reg = endianness src_reg */
	      memcpy (src_reg, token, tlen);
	      if (*dst_reg == 'r' && !strcmp (dst_reg, src_reg))
		bpf_insn = build_bpf_endianness (dst_reg, bpf_endianness);
	      else
		syntax_err ("invalid operand for instruction: '%s'", token);

	      state = ST_EOI;
	      break;

	    case 28:
	      memcpy (dst_reg, token, tlen);
	      state = 29; /* lock str_cast ( 'dst_reg'  */
	      break;

	    case 32:
	      {
		/* lock str_cast ( dst_reg offset ) atomic_insn 'src_reg' */
		int with_offset = *str_offset != '\0';

		memcpy (src_reg, token, tlen);
		if ((bpf_cast != BPF_CAST_U32
		     && bpf_cast != BPF_CAST_U64)
		    || *dst_reg != 'r'
		    || *src_reg != 'r')
		  syntax_err ("invalid wide atomic instruction");
		else
		  bpf_insn = build_bpf_atomic_insn (dst_reg, src_reg, bpf_atomic_insn,
						    bpf_cast, with_offset ? str_offset : str_symbol);
	      }

	      state = ST_EOI;
	      break;

	    case 33:
	      /* callx 'dst_reg' */
	      bpf_insn = xasprintf ("%s %%%s", "call", token);
	      state = ST_EOI;
	      break;

	    case 35:
	      memcpy (src_reg, token, tlen);
	      state = 36; /* dst_reg = str_cast skb [ 'src_reg' */
	      break;
	    }
	  break;

	case BPF_MOV:
	case BPF_ADD:
	case BPF_SUB:
	case BPF_MUL:
	case BPF_DIV:
	case BPF_OR:
	case BPF_AND:
	case BPF_LSH:
	case BPF_RSH:
	case BPF_MOD:
	case BPF_XOR:
	case BPF_ARSH:
	case BPF_NEG:
	  switch (state)
	    {
	    case 1:
	      state = 3;  /* dst_reg 'arith_op' */
	      bpf_arithm_op = ttype;
	      break;

	    case 3:
	      if (ttype == BPF_NEG)
		{
		  /* reg = -reg */
		  bpf_arithm_op = ttype;
		  memcpy (src_reg, token + 1, tlen - 1);
		  if (strcmp (dst_reg, src_reg))
		    {
		      syntax_err ("found: '%s', expected: -%s", token, dst_reg);
		      break;
		    }

		  bpf_insn = build_bpf_arithm_insn (dst_reg, src_reg, 0,
						    NULL, bpf_arithm_op);
		  state = ST_EOI;
		}
	      break;

	    case 23:
	      memcpy (src_reg, token, tlen);
	      state = 11; /* str_cast ( dst_reg offset ) '=' */
	      break;

	    case 12:
	      if (ttype == BPF_MOV)
		state = 13; /* str_cast ( dst_reg offset ) '=' */
	      break;

	    case 31:
	      bpf_atomic_insn = ttype;
	      state = 32; /* lock str_cast ( dst_reg offset ) 'atomic_insn' */
	      break;

	    default:
	      syntax_err ("unexpected '%s'", token);
	      state = ST_EOI;
	    }
	  break;

	case BPF_CAST_U8:
	case BPF_CAST_U16:
	case BPF_CAST_U32:
	case BPF_CAST_U64:
	  bpf_cast = ttype;
	  switch (state)
	    {
	    case 3:
	      state = 4; /* dst_reg = 'str_cast' */
	      break;

	    case 0:
	      state = 8;  /* 'str_cast' */
	      break;

	    case 26:
	      state = 27; /* lock 'str_cast' */
	      break;
	    }
	  break;

	case BPF_CHR_OPEN_BR:
	  switch (state)
	    {
	    case 4:
	      state = 5; /* dst_reg = str_cast '(' */
	      break;

	    case 8:
	      state = 9; /* str_cast '(' */
	      break;

	    case 27:
	      state = 28; /* lock str_cast '(' */
	      break;

	    case 34:
	      state = 35; /* dst_reg = str_cast skb '[' */
	      break;
	    }
	  break;

	case BPF_CHR_CLSE_BR:
	  switch (state)
	    {
	    case 7:
	      /* dst_reg = str_cast ( imm32 ')' */
	      bpf_insn = build_bpf_load_store_insn (dst_reg, src_reg,
						    bpf_cast, str_imm32, 1);
	      state = ST_EOI;
	      break;

	    case 11:
	      state = 12; /* str_cast ( dst_reg imm32 ')' */
	      break;

	    case 21:
	      /* dst_reg = str_cast ( src_reg  offset ')' */
	      bpf_insn = build_bpf_load_store_insn (dst_reg, src_reg,
						    bpf_cast, str_offset, 1);
	      state = ST_EOI;
	      break;

	    case 22:
	      state = 23; /* str_cast ( dst_reg offset ')' */
	      break;

	    case 30:
	      state = 31; /* lock str_cast ( dst_reg offset ')' */
	      break;

	    case 37:
	      /* dst_reg = str_cast skb [ src_reg imm32 ']' */
	      if (*dst_reg != 'w' && !strcmp ("r0", dst_reg))
		bpf_insn = build_bpf_non_generic_load (*src_reg != '\0' ? src_reg : NULL,
						       bpf_cast, str_imm32);
	      else
		syntax_err ("invalid register operand: '%s'", dst_reg);

	      state = ST_EOI;
	      break;
	    }
	  break;

	case BPF_EXPR:
	  switch (state)
	    {
	    case 3:
	      {
		/* dst_reg bpf_arithm_op 'imm32' */
		int load64 = 0;

		memcpy (str_imm32, token, tlen);
		memset (token, 0, tlen);

		if ((ttype = get_token (&str, token, &tlen)) == BPF_LL
		    && bpf_arithm_op == BPF_MOV)
		  load64 = 1;
		else if (ttype != BPF_UNKNOWN)
		  syntax_err ("unexpected token: '%s'", token);

		if (load64 && *dst_reg == 'w')
		  syntax_err ("unexpected register size: '%s'", dst_reg);

		if (! parse_err)
		  bpf_insn = build_bpf_arithm_insn (dst_reg, NULL, load64,
						    str_imm32, bpf_arithm_op);
		state = ST_EOI;
	      }
	      break;

	    case 18:
	      {
		/* if dst_reg jmp_op src_reg goto 'offset' */
		int with_src = *src_reg != '\0';

		memcpy (str_offset, token, tlen);
		if (with_src && *dst_reg != *src_reg)
		  syntax_err ("different register size: '%s', '%s'",
			      dst_reg, src_reg);
		else
		  bpf_insn = build_bpf_jmp_insn (dst_reg, with_src ? src_reg : NULL,
						 with_src ? NULL: str_imm32,
						 bpf_jmp_op, NULL, str_offset);
		state = ST_EOI;
	      }
	      break;

	    case 19:
	      /* goto 'offset' */
	      memcpy (str_offset, token, tlen);
	      bpf_insn = xasprintf ("%s %s", "ja", str_offset);
	      state = ST_EOI;
	      break;

	    case 6:
	      memcpy (str_offset, token, tlen);
	      state = 21; /* dst_reg = str_cast ( src_reg  'offset' */
	      break;

	    case 10:
	      memcpy (str_offset, token, tlen);
	      state = 22; /* str_cast ( dst_reg 'offset' */
	      break;

	    case 16:
	      memcpy (str_imm32, token, tlen);
	      state = 25; /* if dst_reg jmp_op 'imm32' */
	      break;

	    case 29:
	      memcpy (str_offset, token, tlen);
	      state = 30; /* lock str_cast ( dst_reg 'offset' */
	      break;

	    case 34:
	      /* dst_reg = str_cast skb 'imm32' */
	      if (*dst_reg != 'w' && !strcmp ("r0", dst_reg))
		{
		  memcpy (str_imm32, token, tlen);
		  bpf_insn = build_bpf_non_generic_load (*src_reg != '\0' ? src_reg : NULL,
							 bpf_cast, str_imm32);
		}
	      else
		syntax_err ("invalid register operand: '%s'", dst_reg);

	      state = ST_EOI;
	      break;

	    case 36:
	      memcpy (str_imm32, token, tlen);
	      state = 37; /* dst_reg = str_cast skb [ src_reg 'imm32' */
	      break;
	    }
	  break;

	case BPF_IF:
	  if (state == 0)
	    state = 14;
	  break;

	case BPF_JSGT:
	case BPF_JSLT:
	case BPF_JSLE:
	case BPF_JSGE:
	case BPF_JGT:
	case BPF_JGE:
	case BPF_JLE:
	case BPF_JSET:
	case BPF_JNE:
	case BPF_JLT:
	case BPF_JEQ:
	  if (state == 15)
	    {
	      bpf_jmp_op = ttype;
	      state = 16; /* if dst_reg 'jmp_op' */
	    }
	  break;

	case BPF_GOTO:
	  switch (state)
	    {
	    case 17:
	    case 25:
	      state = 18; /* if dst_reg jmp_op src_reg|imm32 'goto' */
	      break;

	    case 0:
	      state = 19;
	      break;
	    }
	  break;

	case BPF_SYMBOL:
	  switch (state)
	    {
	    case 18:
	      {
		/* if dst_reg jmp_op src_reg goto 'sym' */
		int with_src = *src_reg != '\0';

		memcpy (str_symbol, token, tlen);
		if (with_src && *dst_reg != *src_reg)
		  syntax_err ("different register size: '%s', '%s'",
			      dst_reg, src_reg);
		else
		  bpf_insn = build_bpf_jmp_insn (dst_reg, with_src ? src_reg : NULL,
						 with_src ? NULL: str_imm32,
						 bpf_jmp_op, str_symbol, NULL);
		state = ST_EOI;
	      }
	      break;

	    case 19:
	      /* goto 'sym' */
	      memcpy (str_symbol, token, tlen);
	      bpf_insn = xasprintf ("%s %s", "ja", str_symbol);
	      state = ST_EOI;
	      break;

	    case 0:
	      state = ST_EOI;
	      break;

	    case 3:
	      {
		/* dst_reg arithm_op 'sym' */
		int load64 = 0;
		
		memcpy (str_symbol, token, tlen);
		memset (token, 0, tlen);

		if ((ttype = get_token (&str, token, &tlen)) == BPF_LL
		    && bpf_arithm_op == BPF_MOV)
		  load64 = 1;
		else if (ttype != BPF_UNKNOWN)
		  syntax_err ("unexpected token: '%s'", token);

		if (load64 && *dst_reg == 'w')
		  syntax_err ("unexpected register size: '%s'", dst_reg);

		if (! parse_err)
		  bpf_insn = build_bpf_arithm_insn (dst_reg, NULL, load64,
						    str_symbol, bpf_arithm_op);
		state = ST_EOI;
	      }
	      break;
	    }
	  break;

	case BPF_LE16:
	case BPF_LE32:
	case BPF_LE64:
	case BPF_BE16:
	case BPF_BE32:
	case BPF_BE64:
	  bpf_endianness = ttype;
	  state = 24; /* dst_reg = 'endianness' */
	  break;

	case BPF_LOCK:
	  state = 26;
	  break;

	case BPF_IND_CALL:
	  state = 33;
	  break;

	case BPF_LD:
	  state = 34; /* dst_reg = str_cast 'skb' */
	  break;
	}

      memset (token, 0, tlen);
    }

  if (state != ST_EOI)
    syntax_err ("incomplete instruction");

  *errmsg = errbuf;
  return bpf_insn;

#undef syntax_err
}

void
md_assemble (char *str)
{
  const CGEN_INSN *insn;
  char *errmsg;
  char *a_errmsg;
  CGEN_FIELDS fields;
  char *normal;

#if CGEN_INT_INSN_P
  CGEN_INSN_INT buffer[CGEN_MAX_INSN_SIZE / sizeof (CGEN_INT_INSN_P)];
#else
  unsigned char buffer[CGEN_MAX_INSN_SIZE];
#endif

  gas_cgen_init_parse ();
  insn = bpf_cgen_assemble_insn (gas_cgen_cpu_desc, str, &fields,
                                  buffer, &errmsg);
  if (insn == NULL)
    {
      normal = bpf_pseudoc_to_normal_syntax (str, &a_errmsg);
      if (normal)
	{
	  insn = bpf_cgen_assemble_insn (gas_cgen_cpu_desc, normal, &fields,
					 buffer, &a_errmsg);
	  xfree (normal);
	}

      if (insn == NULL)
	{
	  as_bad ("%s", errmsg);
	  if (a_errmsg)
	    {
	      as_bad ("%s", a_errmsg);
	      xfree (a_errmsg);
	    }
	  return;
	}
    }

  gas_cgen_finish_insn (insn, buffer, CGEN_FIELDS_BITSIZE (&fields),
                        0, /* zero to ban relaxable insns.  */
                        NULL); /* NULL so results not returned here.  */
}

void
md_operand (expressionS *expressionP)
{
  invalid_expression = input_line_pointer - 1;
  gas_cgen_md_operand (expressionP);
}


symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}


/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, false);
}
