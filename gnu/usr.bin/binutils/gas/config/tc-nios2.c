/* Altera Nios II assembler.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Nigel Gray (ngray@altera.com).
   Contributed by Mentor Graphics, Inc.

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

#include "as.h"
#include "opcode/nios2.h"
#include "elf/nios2.h"
#include "tc-nios2.h"
#include "bfd.h"
#include "dwarf2dbg.h"
#include "subsegs.h"
#include "safe-ctype.h"
#include "dw2gencfi.h"

#ifndef OBJ_ELF
/* We are not supporting any other target so we throw a compile time error.  */
OBJ_ELF not defined
#endif

/* We can choose our endianness at run-time, regardless of configuration.  */
extern int target_big_endian;

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

/* Chars that mean this number is a floating point constant.  */
/* As in 0f12.456 */
/* or	 0d1.2345e12 */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

/* Also be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
   changed in read.c.  Ideally it shouldn't have to know about it at all,
   but nothing is ideal around here.  */

/* Machine-dependent command-line options.  */

const char *md_shortopts = "r";

struct option md_longopts[] = {
#define OPTION_RELAX_ALL (OPTION_MD_BASE + 0)
  {"relax-all", no_argument, NULL, OPTION_RELAX_ALL},
#define OPTION_NORELAX (OPTION_MD_BASE + 1)
  {"no-relax", no_argument, NULL, OPTION_NORELAX},
#define OPTION_RELAX_SECTION (OPTION_MD_BASE + 2)
  {"relax-section", no_argument, NULL, OPTION_RELAX_SECTION},
#define OPTION_EB (OPTION_MD_BASE + 3)
  {"EB", no_argument, NULL, OPTION_EB},
#define OPTION_EL (OPTION_MD_BASE + 4)
  {"EL", no_argument, NULL, OPTION_EL},
#define OPTION_MARCH (OPTION_MD_BASE + 5)
  {"march", required_argument, NULL, OPTION_MARCH}
};

size_t md_longopts_size = sizeof (md_longopts);

/* The assembler supports three different relaxation modes, controlled by
   command-line options.  */
typedef enum
{
  relax_section = 0,
  relax_none,
  relax_all
} relax_optionT;

/* Struct contains all assembler options set with .set.  */
static struct
{
  /* .set noat -> noat = 1 allows assembly code to use at without warning
     and macro expansions generate a warning.
     .set at -> noat = 0, assembly code using at warn but macro expansions
     do not generate warnings.  */
  bool noat;

  /* .set nobreak -> nobreak = 1 allows assembly code to use ba,bt without
				 warning.
     .set break -> nobreak = 0, assembly code using ba,bt warns.  */
  bool nobreak;

  /* .cmd line option -relax-all allows all branches and calls to be replaced
     with longer versions.
     -no-relax inhibits branch/call conversion.
     The default value is relax_section, which relaxes branches within
     a section.  */
  relax_optionT relax;

} nios2_as_options = {false, false, relax_section};


typedef struct nios2_insn_reloc
{
  /* Any expression in the instruction is parsed into this field,
     which is passed to fix_new_exp() to generate a fixup.  */
  expressionS reloc_expression;

  /* The type of the relocation to be applied.  */
  bfd_reloc_code_real_type reloc_type;

  /* PC-relative.  */
  unsigned int reloc_pcrel;

  /* The next relocation to be applied to the instruction.  */
  struct nios2_insn_reloc *reloc_next;
} nios2_insn_relocS;

/* This struct is used to hold state when assembling instructions.  */
typedef struct nios2_insn_info
{
  /* Assembled instruction.  */
  unsigned long insn_code;

  /* Constant bits masked into insn_code for self-check mode.  */
  unsigned long constant_bits;

  /* Pointer to the relevant bit of the opcode table.  */
  const struct nios2_opcode *insn_nios2_opcode;
  /* After parsing ptrs to the tokens in the instruction fill this array
     it is terminated with a null pointer (hence the first +1).
     The second +1 is because in some parts of the code the opcode
     is not counted as a token, but still placed in this array.  */
  const char *insn_tokens[NIOS2_MAX_INSN_TOKENS + 1 + 1];

  /* This holds information used to generate fixups
     and eventually relocations if it is not null.  */
  nios2_insn_relocS *insn_reloc;
} nios2_insn_infoS;


/* This struct is used to convert Nios II pseudo-ops into the
   corresponding real op.  */
typedef struct nios2_ps_insn_info
{
  /* Map this pseudo_op... */
  const char *pseudo_insn;

  /* ...to this real instruction.  */
  const char *insn;

  /* Call this function to modify the operands....  */
  void (*arg_modifer_func) (char ** parsed_args, const char *arg, int num,
			    int start);

  /* ...with these arguments.  */
  const char *arg_modifier;
  int num;
  int index;

  /* If arg_modifier_func allocates new memory, provide this function
     to free it afterwards.  */
  void (*arg_cleanup_func) (char **parsed_args, int num, int start);
} nios2_ps_insn_infoS;

/* Opcode hash table.  */
static htab_t nios2_opcode_hash = NULL;
#define nios2_opcode_lookup(NAME) \
  ((struct nios2_opcode *) str_hash_find (nios2_opcode_hash, (NAME)))

/* Register hash table.  */
static htab_t nios2_reg_hash = NULL;
#define nios2_reg_lookup(NAME) \
  ((struct nios2_reg *) str_hash_find (nios2_reg_hash, (NAME)))


/* Pseudo-op hash table.  */
static htab_t nios2_ps_hash = NULL;
#define nios2_ps_lookup(NAME) \
  ((nios2_ps_insn_infoS *) str_hash_find (nios2_ps_hash, (NAME)))

/* The known current alignment of the current section.  */
static int nios2_current_align;
static segT nios2_current_align_seg;

static int nios2_auto_align_on = 1;

/* The last seen label in the current section.  This is used to auto-align
   labels preceding instructions.  */
static symbolS *nios2_last_label;

/* If we saw a 16-bit CDX instruction, we can align on 2-byte boundaries
   instead of 4-bytes.  Use this to keep track of the minimum power-of-2
   alignment.  */
static int nios2_min_align = 2;

#ifdef OBJ_ELF
/* Pre-defined "_GLOBAL_OFFSET_TABLE_"	*/
symbolS *GOT_symbol;
#endif

/* The processor architecture value, EF_NIOS2_ARCH_R1 by default.  */
static int nios2_architecture = EF_NIOS2_ARCH_R1;


/** Utility routines.  */
/* Function md_chars_to_number takes the sequence of
   bytes in buf and returns the corresponding value
   in an int. n must be 1, 2 or 4.  */
static valueT
md_chars_to_number (char *buf, int n)
{
  int i;
  valueT val;

  gas_assert (n == 1 || n == 2 || n == 4);

  val = 0;
  if (target_big_endian)
    for (i = 0; i < n; ++i)
      val = val | ((valueT) (buf[i] & 0xff) << 8 * (n - (i + 1)));
  else
    for (i = 0; i < n; ++i)
      val = val | ((valueT) (buf[i] & 0xff) << 8 * i);
  return val;
}


/* This function turns a C long int, short int or char
   into the series of bytes that represent the number
   on the target machine.  */
void
md_number_to_chars (char *buf, valueT val, int n)
{
  gas_assert (n == 1 || n == 2 || n == 4 || n == 8);
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */
const char *
md_atof (int type, char *litP, int *sizeP)
{
  int prec;
  LITTLENUM_TYPE words[4];
  char *t;
  int i;

  switch (type)
    {
    case 'f':
      prec = 2;
      break;
    case 'd':
      prec = 4;
      break;
    default:
      *sizeP = 0;
      return _("bad call to md_atof");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizeP = prec * 2;

  if (! target_big_endian)
    for (i = prec - 1; i >= 0; i--, litP += 2)
      md_number_to_chars (litP, (valueT) words[i], 2);
  else
    for (i = 0; i < prec; i++, litP += 2)
      md_number_to_chars (litP, (valueT) words[i], 2);

  return NULL;
}

/* Return true if STR is prefixed with a special relocation operator.  */
static int
nios2_special_relocation_p (const char *str)
{
  return (startswith (str, "%lo")
	  || startswith (str, "%hi")
	  || startswith (str, "%hiadj")
	  || startswith (str, "%gprel")
	  || startswith (str, "%got")
	  || startswith (str, "%call")
	  || startswith (str, "%gotoff_lo")
	  || startswith (str, "%gotoff_hiadj")
	  || startswith (str, "%tls_gd")
	  || startswith (str, "%tls_ldm")
	  || startswith (str, "%tls_ldo")
	  || startswith (str, "%tls_ie")
	  || startswith (str, "%tls_le")
	  || startswith (str, "%gotoff"));
}


/* nop fill patterns for text section.  */
static char const nop_r1[4] = { 0x3a, 0x88, 0x01, 0x00 };
static char const nop_r2[4] = { 0x20, 0x00, 0x00, 0xc4 };
static char const nop_r2_cdx[2] = { 0x3b, 0x00 };
static char const *nop32 = nop_r1;
static char const *nop16 = NULL;

/* Handles all machine-dependent alignment needs.  */
static void
nios2_align (int log_size, const char *pfill, symbolS *label)
{
  int align;
  long max_alignment = 15;

  /* The front end is prone to changing segments out from under us
     temporarily when -g is in effect.  */
  int switched_seg_p = (nios2_current_align_seg != now_seg);

  align = log_size;
  if (align > max_alignment)
    {
      align = max_alignment;
      as_bad (_("Alignment too large: %d. assumed"), align);
    }
  else if (align < 0)
    {
      as_warn (_("Alignment negative: 0 assumed"));
      align = 0;
    }

  if (align != 0)
    {
      if (subseg_text_p (now_seg) && align >= nios2_min_align)
	{
	  /* First, make sure we're on the minimum boundary, in case
	     someone has been putting .byte values the text section.  */
	  if (nios2_current_align < nios2_min_align || switched_seg_p)
	    frag_align (nios2_min_align, 0, 0);

	  /* If we might be on a 2-byte boundary, first align to a
	     4-byte boundary using the 2-byte nop as fill.  */
	  if (nios2_min_align == 1
	      && align > nios2_min_align
	      && pfill == nop32 )
	    {
	      gas_assert (nop16);
	      frag_align_pattern (2, nop16, 2, 0);
	    }

	  /* Now fill in the alignment pattern.  */
	  if (pfill != NULL)
	    frag_align_pattern (align, pfill, 4, 0);
	  else
	    frag_align (align, 0, 0);
	}
      else
	frag_align (align, 0, 0);

      if (!switched_seg_p)
	nios2_current_align = align;

      /* If the last label was in a different section we can't align it.  */
      if (label != NULL && !switched_seg_p)
	{
	  symbolS *sym;
	  int label_seen = false;
	  struct frag *old_frag;
	  valueT old_value;
	  valueT new_value;

	  gas_assert (S_GET_SEGMENT (label) == now_seg);

	  old_frag = symbol_get_frag (label);
	  old_value = S_GET_VALUE (label);
	  new_value = (valueT) frag_now_fix ();

	  /* It is possible to have more than one label at a particular
	     address, especially if debugging is enabled, so we must
	     take care to adjust all the labels at this address in this
	     fragment.  To save time we search from the end of the symbol
	     list, backwards, since the symbols we are interested in are
	     almost certainly the ones that were most recently added.
	     Also to save time we stop searching once we have seen at least
	     one matching label, and we encounter a label that is no longer
	     in the target fragment.  Note, this search is guaranteed to
	     find at least one match when sym == label, so no special case
	     code is necessary.  */
	  for (sym = symbol_lastP; sym != NULL; sym = symbol_previous (sym))
	    if (symbol_get_frag (sym) == old_frag
		&& S_GET_VALUE (sym) == old_value)
	      {
		label_seen = true;
		symbol_set_frag (sym, frag_now);
		S_SET_VALUE (sym, new_value);
	      }
	    else if (label_seen && symbol_get_frag (sym) != old_frag)
	      break;
	}
      record_alignment (now_seg, align);
    }
}


/** Support for self-check mode.  */

/* Mode of the assembler.  */
typedef enum
{
  NIOS2_MODE_ASSEMBLE,		/* Ordinary operation.  */
  NIOS2_MODE_TEST		/* Hidden mode used for self testing.  */
} NIOS2_MODE;

static NIOS2_MODE nios2_mode = NIOS2_MODE_ASSEMBLE;

/* This function is used to in self-checking mode
   to check the assembled instruction
   opcode should be the assembled opcode, and exp_opcode
   the parsed string representing the expected opcode.  */
static void
nios2_check_assembly (unsigned int opcode, const char *exp_opcode)
{
  if (nios2_mode == NIOS2_MODE_TEST)
    {
      if (exp_opcode == NULL)
	as_bad (_("expecting opcode string in self test mode"));
      else if (opcode != strtoul (exp_opcode, NULL, 16))
	as_bad (_("assembly 0x%08x, expected %s"), opcode, exp_opcode);
    }
}


/** Support for machine-dependent assembler directives.  */
/* Handle the .align pseudo-op.  This aligns to a power of two.  It
   also adjusts any current instruction label.  We treat this the same
   way the MIPS port does: .align 0 turns off auto alignment.  */
static void
s_nios2_align (int ignore ATTRIBUTE_UNUSED)
{
  int align;
  char fill;
  const char *pfill = NULL;
  long max_alignment = 15;

  align = get_absolute_expression ();
  if (align > max_alignment)
    {
      align = max_alignment;
      as_bad (_("Alignment too large: %d. assumed"), align);
    }
  else if (align < 0)
    {
      as_warn (_("Alignment negative: 0 assumed"));
      align = 0;
    }

  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      fill = get_absolute_expression ();
      pfill = (const char *) &fill;
    }
  else if (subseg_text_p (now_seg))
    pfill = (const char *) nop32;
  else
    {
      pfill = NULL;
      nios2_last_label = NULL;
    }

  if (align != 0)
    {
      nios2_auto_align_on = 1;
      nios2_align (align, pfill, nios2_last_label);
      nios2_last_label = NULL;
    }
  else
    nios2_auto_align_on = 0;

  demand_empty_rest_of_line ();
}

/* Handle the .text pseudo-op.  This is like the usual one, but it
   clears the saved last label and resets known alignment.  */
static void
s_nios2_text (int i)
{
  s_text (i);
  nios2_last_label = NULL;
  nios2_current_align = 0;
  nios2_current_align_seg = now_seg;
}

/* Handle the .data pseudo-op.  This is like the usual one, but it
   clears the saved last label and resets known alignment.  */
static void
s_nios2_data (int i)
{
  s_data (i);
  nios2_last_label = NULL;
  nios2_current_align = 0;
  nios2_current_align_seg = now_seg;
}

/* Handle the .section pseudo-op.  This is like the usual one, but it
   clears the saved last label and resets known alignment.  */
static void
s_nios2_section (int ignore)
{
  obj_elf_section (ignore);
  nios2_last_label = NULL;
  nios2_current_align = 0;
  nios2_current_align_seg = now_seg;
}

/* Explicitly unaligned cons.  */
static void
s_nios2_ucons (int nbytes)
{
  int hold;
  hold = nios2_auto_align_on;
  nios2_auto_align_on = 0;
  cons (nbytes);
  nios2_auto_align_on = hold;
}

/* Handle the .sdata directive.  */
static void
s_nios2_sdata (int ignore ATTRIBUTE_UNUSED)
{
  get_absolute_expression ();  /* Ignored.  */
  subseg_new (".sdata", 0);
  demand_empty_rest_of_line ();
}

/* .set sets assembler options eg noat/at and is also used
   to set symbol values (.equ, .equiv ).  */
static void
s_nios2_set (int equiv)
{
  char *save = input_line_pointer;
  char *directive;
  char delim = get_symbol_name (&directive);
  char *endline = input_line_pointer;

  (void) restore_line_pointer (delim);

  /* We only want to handle ".set XXX" if the
     user has tried ".set XXX, YYY" they are not
     trying a directive.  This prevents
     us from polluting the name space.  */
  SKIP_WHITESPACE ();
  if (is_end_of_line[(unsigned char) *input_line_pointer])
    {
      bool done = true;
      *endline = 0;

      if (!strcmp (directive, "noat"))
	  nios2_as_options.noat = true;
      else if (!strcmp (directive, "at"))
	  nios2_as_options.noat = false;
      else if (!strcmp (directive, "nobreak"))
	  nios2_as_options.nobreak = true;
      else if (!strcmp (directive, "break"))
	  nios2_as_options.nobreak = false;
      else if (!strcmp (directive, "norelax"))
	  nios2_as_options.relax = relax_none;
      else if (!strcmp (directive, "relaxsection"))
	  nios2_as_options.relax = relax_section;
      else if (!strcmp (directive, "relaxall"))
	  nios2_as_options.relax = relax_all;
      else
	done = false;

      if (done)
	{
	  *endline = delim;
	  demand_empty_rest_of_line ();
	  return;
	}
    }

  /* If we fall through to here, either we have ".set XXX, YYY"
     or we have ".set XXX" where XXX is unknown or we have
     a syntax error.  */
  input_line_pointer = save;
  s_set (equiv);
}

/* Machine-dependent assembler directives.
   Format of each entry is:
   { "directive", handler_func, param }	 */
const pseudo_typeS md_pseudo_table[] = {
  {"align", s_nios2_align, 0},
  {"text", s_nios2_text, 0},
  {"data", s_nios2_data, 0},
  {"section", s_nios2_section, 0},
  {"section.s", s_nios2_section, 0},
  {"sect", s_nios2_section, 0},
  {"sect.s", s_nios2_section, 0},
  /* .dword and .half are included for compatibility with MIPS.  */
  {"dword", cons, 8},
  {"half", cons, 2},
  /* NIOS2 native word size is 4 bytes, so we override
     the GAS default of 2.  */
  {"word", cons, 4},
  /* Explicitly unaligned directives.  */
  {"2byte", s_nios2_ucons, 2},
  {"4byte", s_nios2_ucons, 4},
  {"8byte", s_nios2_ucons, 8},
  {"16byte", s_nios2_ucons, 16},
#ifdef OBJ_ELF
  {"sdata", s_nios2_sdata, 0},
#endif
  {"set", s_nios2_set, 0},
  {NULL, NULL, 0}
};


/** Relaxation support. */

/* We support two relaxation modes:  a limited PC-relative mode with
   -relax-section (the default), and an absolute jump mode with -relax-all.

   Nios II PC-relative branch instructions only support 16-bit offsets.
   And, there's no good way to add a 32-bit constant to the PC without
   using two registers.

   To deal with this, for the pc-relative relaxation mode we convert
     br label
   into a series of 16-bit adds, like:
     nextpc at
     addi at, at, 32767
     ...
     addi at, at, remainder
     jmp at

   Similarly, conditional branches are converted from
     b(condition) r, s, label
   into a series like:
     b(opposite condition) r, s, skip
     nextpc at
     addi at, at, 32767
     ...
     addi at, at, remainder
     jmp at
     skip:

   The compiler can do a better job, either by converting the branch
   directly into a JMP (going through the GOT for PIC) or by allocating
   a second register for the 32-bit displacement.

   For the -relax-all relaxation mode, the conversions are
     movhi at, %hi(symbol+offset)
     ori at, %lo(symbol+offset)
     jmp at
   and
     b(opposite condition), r, s, skip
     movhi at, %hi(symbol+offset)
     ori at, %lo(symbol+offset)
     jmp at
     skip:
   respectively.

   16-bit CDX branch instructions are relaxed first into equivalent
   32-bit branches and then the above transformations are applied
   if necessary.

*/

/* Arbitrarily limit the number of addis we can insert; we need to be able
   to specify the maximum growth size for each frag that contains a
   relaxable branch.  There's no point in specifying a huge number here
   since that means the assembler needs to allocate that much extra
   memory for every branch, and almost no real code will ever need it.
   Plus, as already noted a better solution is to just use a jmp, or
   allocate a second register to hold a 32-bit displacement.
   FIXME:  Rather than making this a constant, it could be controlled by
   a command-line argument.  */
#define RELAX_MAX_ADDI 32

/* The fr_subtype field represents the target-specific relocation state.
   It has type relax_substateT (unsigned int).  We use it to track the
   number of addis necessary, plus a bit to track whether this is a
   conditional branch and a bit for 16-bit CDX instructions.
   Regardless of the smaller RELAX_MAX_ADDI limit, we reserve 16 bits
   in the fr_subtype to encode the number of addis so that the whole
   theoretically-valid range is representable.
   For the -relax-all mode, N = 0 represents an in-range branch and N = 1
   represents a branch that needs to be relaxed.  */
#define UBRANCH (0 << 16)
#define CBRANCH (1 << 16)
#define CDXBRANCH (1 << 17)
#define IS_CBRANCH(SUBTYPE) ((SUBTYPE) & CBRANCH)
#define IS_UBRANCH(SUBTYPE) (!IS_CBRANCH (SUBTYPE))
#define IS_CDXBRANCH(SUBTYPE) ((SUBTYPE) & CDXBRANCH)
#define UBRANCH_SUBTYPE(N) (UBRANCH | (N))
#define CBRANCH_SUBTYPE(N) (CBRANCH | (N))
#define CDX_UBRANCH_SUBTYPE(N) (CDXBRANCH | UBRANCH | (N))
#define CDX_CBRANCH_SUBTYPE(N) (CDXBRANCH | CBRANCH | (N))
#define SUBTYPE_ADDIS(SUBTYPE) ((SUBTYPE) & 0xffff)

/* For the -relax-section mode, unconditional branches require 2 extra
   instructions besides the addis, conditional branches require 3.  */
#define UBRANCH_ADDIS_TO_SIZE(N) (((N) + 2) * 4)
#define CBRANCH_ADDIS_TO_SIZE(N) (((N) + 3) * 4)

/* For the -relax-all mode, unconditional branches require 3 instructions
   and conditional branches require 4.  */
#define UBRANCH_JUMP_SIZE 12
#define CBRANCH_JUMP_SIZE 16

/* Maximum sizes of relaxation sequences.  */
#define UBRANCH_MAX_SIZE \
  (nios2_as_options.relax == relax_all		\
   ? UBRANCH_JUMP_SIZE				\
   : UBRANCH_ADDIS_TO_SIZE (RELAX_MAX_ADDI))
#define CBRANCH_MAX_SIZE \
  (nios2_as_options.relax == relax_all		\
   ? CBRANCH_JUMP_SIZE				\
   : CBRANCH_ADDIS_TO_SIZE (RELAX_MAX_ADDI))

/* Register number of AT, the assembler temporary.  */
#define AT_REGNUM 1

/* Determine how many bytes are required to represent the sequence
   indicated by SUBTYPE.  */
static int
nios2_relax_subtype_size (relax_substateT subtype)
{
  int n = SUBTYPE_ADDIS (subtype);
  if (n == 0)
    /* Regular conditional/unconditional branch instruction.  */
    return (IS_CDXBRANCH (subtype) ? 2 : 4);
  else if (nios2_as_options.relax == relax_all)
    return (IS_CBRANCH (subtype) ? CBRANCH_JUMP_SIZE : UBRANCH_JUMP_SIZE);
  else if (IS_CBRANCH (subtype))
    return CBRANCH_ADDIS_TO_SIZE (n);
  else
    return UBRANCH_ADDIS_TO_SIZE (n);
}

/* Estimate size of fragp before relaxation.
   This could also examine the offset in fragp and adjust
   fragp->fr_subtype, but we will do that in nios2_relax_frag anyway.  */
int
md_estimate_size_before_relax (fragS *fragp, segT segment ATTRIBUTE_UNUSED)
{
  return nios2_relax_subtype_size (fragp->fr_subtype);
}

/* Implement md_relax_frag, returning the change in size of the frag.  */
long
nios2_relax_frag (segT segment, fragS *fragp, long stretch)
{
  addressT target = fragp->fr_offset;
  relax_substateT subtype = fragp->fr_subtype;
  symbolS *symbolp = fragp->fr_symbol;

  if (symbolp)
    {
      fragS *sym_frag = symbol_get_frag (symbolp);
      offsetT offset;
      int n;
      bool is_cdx = false;

      target += S_GET_VALUE (symbolp);

      /* See comments in write.c:relax_frag about handling of stretch.  */
      if (stretch != 0
	  && sym_frag->relax_marker != fragp->relax_marker)
	{
	  if (stretch < 0 || sym_frag->region == fragp->region)
	    target += stretch;
	  else if (target < fragp->fr_address)
	    target = fragp->fr_next->fr_address + stretch;
	}

      /* We subtract fr_var (4 for 32-bit insns) because all pc relative
	 branches are from the next instruction.  */
      offset = target - fragp->fr_address - fragp->fr_fix - fragp->fr_var;
      if (IS_CDXBRANCH (subtype) && IS_UBRANCH (subtype)
	  && offset >= -1024 && offset < 1024)
	/* PC-relative CDX branch with 11-bit offset.  */
	is_cdx = true;
      else if (IS_CDXBRANCH (subtype) && IS_CBRANCH (subtype)
	       && offset >= -128 && offset < 128)
	/* PC-relative CDX branch with 8-bit offset.  */
	is_cdx = true;
      else if (offset >= -32768 && offset < 32768)
	/* Fits in PC-relative branch.  */
	n = 0;
      else if (nios2_as_options.relax == relax_all)
	/* Convert to jump.  */
	n = 1;
      else if (nios2_as_options.relax == relax_section
	       && S_GET_SEGMENT (symbolp) == segment
	       && S_IS_DEFINED (symbolp))
	/* Attempt a PC-relative relaxation on a branch to a defined
	   symbol in the same segment.  */
	{
	  /* The relaxation for conditional branches is offset by 4
	     bytes because we insert the inverted branch around the
	     sequence.  */
	  if (IS_CBRANCH (subtype))
	    offset = offset - 4;
	  if (offset > 0)
	    n = offset / 32767 + 1;
	  else
	    n = offset / -32768 + 1;

	  /* Bail out immediately if relaxation has failed.  If we try to
	     defer the diagnostic to md_convert_frag, some pathological test
	     cases (e.g. gcc/testsuite/gcc.c-torture/compile/20001226-1.c)
	     apparently never converge.  By returning 0 here we could pretend
	     to the caller that nothing has changed, but that leaves things
	     in an inconsistent state when we get to md_convert_frag.  */
	  if (n > RELAX_MAX_ADDI)
	    {
	      as_bad_where (fragp->fr_file, fragp->fr_line,
			    _("branch offset out of range\n"));
	      as_fatal (_("branch relaxation failed\n"));
	    }
	}
      else
	/* We cannot handle this case, diagnose overflow later.  */
	return 0;

      if (is_cdx)
	fragp->fr_subtype = subtype;
      else if (IS_CBRANCH (subtype))
	fragp->fr_subtype = CBRANCH_SUBTYPE (n);
      else
	fragp->fr_subtype = UBRANCH_SUBTYPE (n);

      return (nios2_relax_subtype_size (fragp->fr_subtype)
	      - nios2_relax_subtype_size (subtype));
    }

  /* If we got here, it's probably an error.  */
  return 0;
}


/* Complete fragp using the data from the relaxation pass. */
void
md_convert_frag (bfd *headers ATTRIBUTE_UNUSED, segT segment ATTRIBUTE_UNUSED,
		 fragS *fragp)
{
  char *buffer = fragp->fr_literal + fragp->fr_fix;
  relax_substateT subtype = fragp->fr_subtype;
  int n = SUBTYPE_ADDIS (subtype);
  addressT target = fragp->fr_offset;
  symbolS *symbolp = fragp->fr_symbol;
  offsetT offset;
  unsigned int addend_mask, addi_mask, op;
  offsetT addend, remainder;
  int i;
  bool is_r2 = (bfd_get_mach (stdoutput) == bfd_mach_nios2r2);

  /* If this is a CDX branch we're not relaxing, just generate the fixup.  */
  if (IS_CDXBRANCH (subtype))
    {
      gas_assert (is_r2);
      fix_new (fragp, fragp->fr_fix, 2, fragp->fr_symbol,
	       fragp->fr_offset, 1,
	       (IS_UBRANCH (subtype)
		? BFD_RELOC_NIOS2_R2_I10_1_PCREL
		: BFD_RELOC_NIOS2_R2_T1I7_1_PCREL));
      fragp->fr_fix += 2;
      return;
    }

  /* If this is a CDX branch we are relaxing, turn it into an equivalent
     32-bit branch and then fall through to the normal non-CDX cases.  */
  if (fragp->fr_var == 2)
    {
      unsigned int opcode = md_chars_to_number (buffer, 2);
      gas_assert (is_r2);
      if (IS_CBRANCH (subtype))
	{
	  unsigned int reg = nios2_r2_reg3_mappings[GET_IW_T1I7_A3 (opcode)];
	  if (GET_IW_R2_OP (opcode) == R2_OP_BNEZ_N)
	    opcode = MATCH_R2_BNE | SET_IW_F2I16_A (reg);
	  else
	    opcode = MATCH_R2_BEQ | SET_IW_F2I16_A (reg);
	}
      else
	opcode = MATCH_R2_BR;
      md_number_to_chars (buffer, opcode, 4);
      fragp->fr_var = 4;
    }

  /* If we didn't or can't relax, this is a regular branch instruction.
     We just need to generate the fixup for the symbol and offset.  */
  if (n == 0)
    {
      fix_new (fragp, fragp->fr_fix, 4, fragp->fr_symbol,
	       fragp->fr_offset, 1, BFD_RELOC_16_PCREL);
      fragp->fr_fix += 4;
      return;
    }

  /* Replace the cbranch at fr_fix with one that has the opposite condition
     in order to jump around the block of instructions we'll be adding.  */
  if (IS_CBRANCH (subtype))
    {
      unsigned int br_opcode;
      unsigned int old_op, new_op;
      int nbytes;

      /* Account for the nextpc and jmp in the pc-relative case, or the two
	 load instructions and jump in the absolute case.  */
      if (nios2_as_options.relax == relax_section)
	nbytes = (n + 2) * 4;
      else
	nbytes = 12;

      br_opcode = md_chars_to_number (buffer, 4);
      if (is_r2)
	{
	  old_op = GET_IW_R2_OP (br_opcode);
	  switch (old_op)
	    {
	    case R2_OP_BEQ:
	      new_op = R2_OP_BNE;
	      break;
	    case R2_OP_BNE:
	      new_op = R2_OP_BEQ;
	      break;
	    case R2_OP_BGE:
	      new_op = R2_OP_BLT;
	      break;
	    case R2_OP_BGEU:
	      new_op = R2_OP_BLTU;
	      break;
	    case R2_OP_BLT:
	      new_op = R2_OP_BGE;
	      break;
	    case R2_OP_BLTU:
	      new_op = R2_OP_BGEU;
	      break;
	    default:
	      abort ();
	    }
	  br_opcode = ((br_opcode & ~IW_R2_OP_SHIFTED_MASK)
		       | SET_IW_R2_OP (new_op));
	  br_opcode = br_opcode | SET_IW_F2I16_IMM16 (nbytes);
	}
      else
	{
	  old_op = GET_IW_R1_OP (br_opcode);
	  switch (old_op)
	    {
	    case R1_OP_BEQ:
	      new_op = R1_OP_BNE;
	      break;
	    case R1_OP_BNE:
	      new_op = R1_OP_BEQ;
	      break;
	    case R1_OP_BGE:
	      new_op = R1_OP_BLT;
	      break;
	    case R1_OP_BGEU:
	      new_op = R1_OP_BLTU;
	      break;
	    case R1_OP_BLT:
	      new_op = R1_OP_BGE;
	      break;
	    case R1_OP_BLTU:
	      new_op = R1_OP_BGEU;
	      break;
	    default:
	      abort ();
	    }
	  br_opcode = ((br_opcode & ~IW_R1_OP_SHIFTED_MASK)
		       | SET_IW_R1_OP (new_op));
	  br_opcode = br_opcode | SET_IW_I_IMM16 (nbytes);
	}
      md_number_to_chars (buffer, br_opcode, 4);
      fragp->fr_fix += 4;
      buffer += 4;
    }

  /* Load at for the PC-relative case.  */
  if (nios2_as_options.relax == relax_section)
    {
      /* Insert the nextpc instruction.  */
      if (is_r2)
	op = MATCH_R2_NEXTPC | SET_IW_F3X6L5_C (AT_REGNUM);
      else
	op = MATCH_R1_NEXTPC | SET_IW_R_C (AT_REGNUM);
      md_number_to_chars (buffer, op, 4);
      fragp->fr_fix += 4;
      buffer += 4;

      /* We need to know whether the offset is positive or negative.  */
      target += S_GET_VALUE (symbolp);
      offset = target - fragp->fr_address - fragp->fr_fix;
      if (offset > 0)
	addend = 32767;
      else
	addend = -32768;
      if (is_r2)
	addend_mask = SET_IW_F2I16_IMM16 ((unsigned int)addend);
      else
	addend_mask = SET_IW_I_IMM16 ((unsigned int)addend);

      /* Insert n-1 addi instructions.  */
      if (is_r2)
	addi_mask = (MATCH_R2_ADDI
		     | SET_IW_F2I16_B (AT_REGNUM)
		     | SET_IW_F2I16_A (AT_REGNUM));
      else
	addi_mask = (MATCH_R1_ADDI
		     | SET_IW_I_B (AT_REGNUM)
		     | SET_IW_I_A (AT_REGNUM));
      for (i = 0; i < n - 1; i ++)
	{
	  md_number_to_chars (buffer, addi_mask | addend_mask, 4);
	  fragp->fr_fix += 4;
	  buffer += 4;
	}

      /* Insert the last addi instruction to hold the remainder.  */
      remainder = offset - addend * (n - 1);
      gas_assert (remainder >= -32768 && remainder <= 32767);
      if (is_r2)
	addend_mask = SET_IW_F2I16_IMM16 ((unsigned int)remainder);
      else
	addend_mask = SET_IW_I_IMM16 ((unsigned int)remainder);
      md_number_to_chars (buffer, addi_mask | addend_mask, 4);
      fragp->fr_fix += 4;
      buffer += 4;
    }

  /* Load at for the absolute case.  */
  else
    {
      if (is_r2)
	op = MATCH_R2_ORHI | SET_IW_F2I16_B (AT_REGNUM) | SET_IW_F2I16_A (0);
      else
	op = MATCH_R1_ORHI | SET_IW_I_B (AT_REGNUM) | SET_IW_I_A (0);
      md_number_to_chars (buffer, op, 4);
      fix_new (fragp, fragp->fr_fix, 4, fragp->fr_symbol, fragp->fr_offset,
	       0, BFD_RELOC_NIOS2_HI16);
      fragp->fr_fix += 4;
      buffer += 4;
      if (is_r2)
	op = (MATCH_R2_ORI | SET_IW_F2I16_B (AT_REGNUM)
	      | SET_IW_F2I16_A (AT_REGNUM));
      else
	op = (MATCH_R1_ORI | SET_IW_I_B (AT_REGNUM)
	      | SET_IW_I_A (AT_REGNUM));
      md_number_to_chars (buffer, op, 4);
      fix_new (fragp, fragp->fr_fix, 4, fragp->fr_symbol, fragp->fr_offset,
	       0, BFD_RELOC_NIOS2_LO16);
      fragp->fr_fix += 4;
      buffer += 4;
    }

  /* Insert the jmp instruction.  */
  if (is_r2)
    op = MATCH_R2_JMP | SET_IW_F3X6L5_A (AT_REGNUM);
  else
    op = MATCH_R1_JMP | SET_IW_R_A (AT_REGNUM);
  md_number_to_chars (buffer, op, 4);
  fragp->fr_fix += 4;
  buffer += 4;
}


/** Fixups and overflow checking.  */

/* Check a fixup for overflow. */
static bool
nios2_check_overflow (valueT fixup, reloc_howto_type *howto)
{
  /* If there is a rightshift, check that the low-order bits are
     zero before applying it.  */
  if (howto->rightshift)
    {
      if ((~(~((valueT) 0) << howto->rightshift) & fixup)
	  && howto->complain_on_overflow != complain_overflow_dont)
	return true;
      fixup = ((signed)fixup) >> howto->rightshift;
    }

  /* Check for overflow - return TRUE if overflow, FALSE if not.  */
  switch (howto->complain_on_overflow)
    {
    case complain_overflow_dont:
      break;
    case complain_overflow_bitfield:
      if ((fixup >> howto->bitsize) != 0
	  && ((signed) fixup >> howto->bitsize) != -1)
	return true;
      break;
    case complain_overflow_signed:
      if ((fixup & 0x80000000) > 0)
	{
	  /* Check for negative overflow.  */
	  if ((signed) fixup < (signed) (~0U << (howto->bitsize - 1)))
	    return true;
	}
      else
	{
	  /* Check for positive overflow.  */
	  if (fixup >= ((unsigned) 1 << (howto->bitsize - 1)))
	    return true;
	}
      break;
    case complain_overflow_unsigned:
      if ((fixup >> howto->bitsize) != 0)
	return true;
      break;
    default:
      as_bad (_("error checking for overflow - broken assembler"));
      break;
    }
  return false;
}

/* Emit diagnostic for fixup overflow.  */
static void
nios2_diagnose_overflow (valueT fixup, reloc_howto_type *howto,
			 fixS *fixP, valueT value)
{
  if (fixP->fx_r_type == BFD_RELOC_8
      || fixP->fx_r_type == BFD_RELOC_16
      || fixP->fx_r_type == BFD_RELOC_32)
    /* These relocs are against data, not instructions.  */
    as_bad_where (fixP->fx_file, fixP->fx_line,
		  _("immediate value 0x%x truncated to 0x%x"),
		  (unsigned int) fixup,
		  (unsigned int) (~(~(valueT) 0 << howto->bitsize) & fixup));
  else
    {
      /* What opcode is the instruction?  This will determine
	 whether we check for overflow in immediate values
	 and what error message we get.  */
      const struct nios2_opcode *opcode;
      enum overflow_type overflow_msg_type;
      unsigned int range_min;
      unsigned int range_max;
      unsigned int address;

      opcode = nios2_find_opcode_hash (value, bfd_get_mach (stdoutput));
      gas_assert (opcode);
      gas_assert (fixP->fx_size == opcode->size);
      overflow_msg_type = opcode->overflow_msg;
      switch (overflow_msg_type)
	{
	case call_target_overflow:
	  range_min
	    = ((fixP->fx_frag->fr_address + fixP->fx_where) & 0xf0000000);
	  range_max = range_min + 0x0fffffff;
	  address = fixup | range_min;

	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("call target address 0x%08x out of range 0x%08x to 0x%08x"),
			address, range_min, range_max);
	  break;
	case branch_target_overflow:
	  if (opcode->format == iw_i_type || opcode->format == iw_F2I16_type)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("branch offset %d out of range %d to %d"),
			  (int)fixup, -32768, 32767);
	  else
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("branch offset %d out of range"),
			  (int)fixup);
	  break;
	case address_offset_overflow:
	  if (opcode->format == iw_i_type || opcode->format == iw_F2I16_type)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("%s offset %d out of range %d to %d"),
			  opcode->name, (int)fixup, -32768, 32767);
	  else
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("%s offset %d out of range"),
			  opcode->name, (int)fixup);
	  break;
	case signed_immed16_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("immediate value %d out of range %d to %d"),
			(int)fixup, -32768, 32767);
	  break;
	case unsigned_immed16_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("immediate value %u out of range %u to %u"),
			(unsigned int)fixup, 0, 65535);
	  break;
	case unsigned_immed5_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("immediate value %u out of range %u to %u"),
			(unsigned int)fixup, 0, 31);
	  break;
	case signed_immed12_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("immediate value %d out of range %d to %d"),
			(int)fixup, -2048, 2047);
	  break;
	case custom_opcode_overflow:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("custom instruction opcode %u out of range %u to %u"),
			(unsigned int)fixup, 0, 255);
	  break;
	default:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("overflow in immediate argument"));
	  break;
	}
    }
}

/* Apply a fixup to the object file.  */
void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  /* Assert that the fixup is one we can handle.  */
  gas_assert (fixP != NULL && valP != NULL
	      && (fixP->fx_r_type == BFD_RELOC_8
		  || fixP->fx_r_type == BFD_RELOC_16
		  || fixP->fx_r_type == BFD_RELOC_32
		  || fixP->fx_r_type == BFD_RELOC_64
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_S16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_U16
		  || fixP->fx_r_type == BFD_RELOC_16_PCREL
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_CALL26
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_IMM5
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_CACHE_OPX
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_IMM6
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_IMM8
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_HI16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_LO16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_HIADJ16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_GPREL
		  || fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
		  || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_UJMP
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_CJMP
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_CALLR
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_ALIGN
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_GOT16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_CALL16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_GOTOFF_LO
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_GOTOFF_HA
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_TLS_GD16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_TLS_LDM16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_TLS_LDO16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_TLS_IE16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_TLS_LE16
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_GOTOFF
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_TLS_DTPREL
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_CALL26_NOAT
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_GOT_LO
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_GOT_HA
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_CALL_LO
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_CALL_HA
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_S12
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_I10_1_PCREL
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_T1I7_1_PCREL
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_T1I7_2
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_T2I4
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_T2I4_1
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_T2I4_2
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_X1I7_2
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_X2L5
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_F1I5_2
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_L5I4X1
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_T1X1I6
		  || fixP->fx_r_type == BFD_RELOC_NIOS2_R2_T1X1I6_2
		  /* Add other relocs here as we generate them.  */
		  ));

  if (fixP->fx_r_type == BFD_RELOC_64)
    {
      /* We may reach here due to .8byte directives, but we never output
	 BFD_RELOC_64; it must be resolved.  */
      if (fixP->fx_addsy != NULL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("cannot create 64-bit relocation"));
      else
	{
	  md_number_to_chars (fixP->fx_frag->fr_literal + fixP->fx_where,
			      *valP, 8);
	  fixP->fx_done = 1;
	}
      return;
    }

  /* The value passed in valP can be the value of a fully
     resolved expression, or it can be the value of a partially
     resolved expression.  In the former case, both fixP->fx_addsy
     and fixP->fx_subsy are NULL, and fixP->fx_offset == *valP, and
     we can fix up the instruction that fixP relates to.
     In the latter case, one or both of fixP->fx_addsy and
     fixP->fx_subsy are not NULL, and fixP->fx_offset may or may not
     equal *valP.  We don't need to check for fixP->fx_subsy being null
     because the generic part of the assembler generates an error if
     it is not an absolute symbol.  */
  if (fixP->fx_addsy != NULL)
    /* Partially resolved expression.  */
    {
      fixP->fx_addnumber = fixP->fx_offset;
      fixP->fx_done = 0;

      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_NIOS2_TLS_GD16:
	case BFD_RELOC_NIOS2_TLS_LDM16:
	case BFD_RELOC_NIOS2_TLS_LDO16:
	case BFD_RELOC_NIOS2_TLS_IE16:
	case BFD_RELOC_NIOS2_TLS_LE16:
	case BFD_RELOC_NIOS2_TLS_DTPMOD:
	case BFD_RELOC_NIOS2_TLS_DTPREL:
	case BFD_RELOC_NIOS2_TLS_TPREL:
	  S_SET_THREAD_LOCAL (fixP->fx_addsy);
	  break;
	default:
	  break;
	}
    }
  else
    /* Fully resolved fixup.  */
    {
      reloc_howto_type *howto
	= bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);

      if (howto == NULL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("relocation is not supported"));
      else
	{
	  valueT fixup = *valP;
	  valueT value;
	  char *buf;

	  /* If this is a pc-relative relocation, we need to
	     subtract the current offset within the object file
	     FIXME : for some reason fixP->fx_pcrel isn't 1 when it should be
	     so I'm using the howto structure instead to determine this.  */
	  if (howto->pc_relative == 1)
	    {
	      fixup = (fixup - (fixP->fx_frag->fr_address + fixP->fx_where
				+ fixP->fx_size));
	      *valP = fixup;
	    }

	  /* Get the instruction or data to be fixed up.  */
	  buf = fixP->fx_frag->fr_literal + fixP->fx_where;
	  value = md_chars_to_number (buf, fixP->fx_size);

	  /* Check for overflow, emitting a diagnostic if necessary.  */
	  if (nios2_check_overflow (fixup, howto))
	    nios2_diagnose_overflow (fixup, howto, fixP, value);

	  /* Apply the right shift.  */
	  fixup = (offsetT) fixup >> howto->rightshift;

	  /* Truncate the fixup to right size.  */
	  switch (fixP->fx_r_type)
	    {
	    case BFD_RELOC_NIOS2_HI16:
	      fixup = (fixup >> 16) & 0xFFFF;
	      break;
	    case BFD_RELOC_NIOS2_LO16:
	      fixup = fixup & 0xFFFF;
	      break;
	    case BFD_RELOC_NIOS2_HIADJ16:
	      fixup = ((fixup + 0x8000) >> 16) & 0xFFFF;
	      break;
	    default:
	      {
		fixup &= ((valueT) 2 << (howto->bitsize - 1)) - 1;
		break;
	      }
	    }

	  /* Fix up the instruction.  */
	  value = (value & ~howto->dst_mask) | (fixup << howto->bitpos);
	  md_number_to_chars (buf, value, fixP->fx_size);
	}

      fixP->fx_done = 1;
    }

  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT)
    {
      fixP->fx_done = 0;
      if (fixP->fx_addsy
	  && !S_IS_DEFINED (fixP->fx_addsy) && !S_IS_WEAK (fixP->fx_addsy))
	S_SET_WEAK (fixP->fx_addsy);
    }
  else if (fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    fixP->fx_done = 0;
}



/** Instruction parsing support. */

/* General internal error routine.  */

static void
bad_opcode (const struct nios2_opcode *op)
{
  fprintf (stderr, _("internal error: broken opcode descriptor for `%s %s'\n"),
	   op->name, op->args);
  as_fatal (_("Broken assembler.  No assembly attempted."));
}

/* Special relocation directive strings.  */

struct nios2_special_relocS
{
  const char *string;
  bfd_reloc_code_real_type reloc_type;
};

/* This table is sorted so that prefix strings are listed after the longer
   strings that include them -- e.g., %got after %got_hiadj, etc.  */

struct nios2_special_relocS nios2_special_reloc[] = {
  {"%hiadj", BFD_RELOC_NIOS2_HIADJ16},
  {"%hi", BFD_RELOC_NIOS2_HI16},
  {"%lo", BFD_RELOC_NIOS2_LO16},
  {"%gprel", BFD_RELOC_NIOS2_GPREL},
  {"%call_lo", BFD_RELOC_NIOS2_CALL_LO},
  {"%call_hiadj", BFD_RELOC_NIOS2_CALL_HA},
  {"%call", BFD_RELOC_NIOS2_CALL16},
  {"%gotoff_lo", BFD_RELOC_NIOS2_GOTOFF_LO},
  {"%gotoff_hiadj", BFD_RELOC_NIOS2_GOTOFF_HA},
  {"%gotoff", BFD_RELOC_NIOS2_GOTOFF},
  {"%got_hiadj", BFD_RELOC_NIOS2_GOT_HA},
  {"%got_lo", BFD_RELOC_NIOS2_GOT_LO},
  {"%got", BFD_RELOC_NIOS2_GOT16},
  {"%tls_gd", BFD_RELOC_NIOS2_TLS_GD16},
  {"%tls_ldm", BFD_RELOC_NIOS2_TLS_LDM16},
  {"%tls_ldo", BFD_RELOC_NIOS2_TLS_LDO16},
  {"%tls_ie", BFD_RELOC_NIOS2_TLS_IE16},
  {"%tls_le", BFD_RELOC_NIOS2_TLS_LE16},
};

#define NIOS2_NUM_SPECIAL_RELOCS \
	(sizeof(nios2_special_reloc)/sizeof(nios2_special_reloc[0]))
const int nios2_num_special_relocs = NIOS2_NUM_SPECIAL_RELOCS;

/* Creates a new nios2_insn_relocS and returns a pointer to it.  */
static nios2_insn_relocS *
nios2_insn_reloc_new (bfd_reloc_code_real_type reloc_type, unsigned int pcrel)
{
  nios2_insn_relocS *retval;
  retval = XNEW (nios2_insn_relocS);
  if (retval == NULL)
    {
      as_bad (_("can't create relocation"));
      abort ();
    }

  /* Fill out the fields with default values.  */
  retval->reloc_next = NULL;
  retval->reloc_type = reloc_type;
  retval->reloc_pcrel = pcrel;
  return retval;
}

/* Frees up memory previously allocated by nios2_insn_reloc_new().  */
/* FIXME:  this is never called; memory leak?  */
#if 0
static void
nios2_insn_reloc_destroy (nios2_insn_relocS *reloc)
{
  gas_assert (reloc != NULL);
  free (reloc);
}
#endif

/* Look up a register name and validate it for the given regtype.
   Return the register mapping or NULL on failure.  */
static struct nios2_reg *
nios2_parse_reg (const char *token, unsigned long regtype)
{
  struct nios2_reg *reg = nios2_reg_lookup (token);

  if (reg == NULL)
    {
      as_bad (_("unknown register %s"), token);
      return NULL;
    }

  /* Matched a register, but is it the wrong type?  */
  if (!(regtype & reg->regtype))
    {
      if (regtype & REG_CONTROL)
	as_bad (_("expecting control register"));
      else if (reg->regtype & REG_CONTROL)
	as_bad (_("illegal use of control register"));
      else if (reg->regtype & REG_COPROCESSOR)
	as_bad (_("illegal use of coprocessor register"));
      else
	as_bad (_("invalid register %s"), token);
      return NULL;
    }

  /* Warn for explicit use of special registers.  */
  if (reg->regtype & REG_NORMAL)
    {
      if (!nios2_as_options.noat && reg->index == 1)
	as_warn (_("Register at (r1) can sometimes be corrupted by "
		   "assembler optimizations.\n"
		   "Use .set noat to turn off those optimizations "
		   "(and this warning)."));
      if (!nios2_as_options.nobreak && reg->index == 25)
	as_warn (_("The debugger will corrupt bt (r25).\n"
		   "If you don't need to debug this "
		   "code use .set nobreak to turn off this warning."));
      if (!nios2_as_options.nobreak && reg->index == 30)
	as_warn (_("The debugger will corrupt sstatus/ba (r30).\n"
		   "If you don't need to debug this "
		   "code use .set nobreak to turn off this warning."));
    }

  return reg;
}

/* This function parses a reglist for ldwm/stwm and push.n/pop.n
   instructions, given as a brace-enclosed register list.  The tokenizer
   has replaced commas in the token with spaces.
   The return value is a bitmask of registers in the set.  It also
   sets nios2_reglist_mask and nios2_reglist_dir to allow error checking
   when parsing the base register.  */

static unsigned long nios2_reglist_mask;
static int nios2_reglist_dir;

static unsigned long
nios2_parse_reglist (char *token, const struct nios2_opcode *op)
{
  unsigned long mask = 0;
  int dir = 0;
  unsigned long regtype = 0;
  int last = -1;
  const char *regname;

  nios2_reglist_mask = 0;
  nios2_reglist_dir = 0;

  if (op->match == MATCH_R2_LDWM || op->match == MATCH_R2_STWM)
    {
      regtype = REG_LDWM;
      dir = 0;
    }
  else if (op->match == MATCH_R2_PUSH_N)
    {
      regtype = REG_POP;
      dir = -1;
    }
  else if (op->match == MATCH_R2_POP_N)
    {
      regtype = REG_POP;
      dir = 1;
    }
  else
    bad_opcode (op);

  for (regname = strtok (token, "{ }");
       regname;
       regname = strtok (NULL, "{ }"))
    {
      int regno;
      struct nios2_reg *reg = nios2_parse_reg (regname, regtype);

      if (!reg)
	break;
      regno = reg->index;

      /* Make sure registers are listed in proper sequence.  */
      if (last >= 0)
	{
	  if (regno == last)
	    {
	      as_bad ("duplicate register %s\n", reg->name);
	      return 0;
	    }
	  else if (dir == 0)
	    dir = (regno < last ? -1 : 1);
	  else if ((dir > 0 && regno < last)
		   || (dir < 0 && regno > last)
		   || (op->match == MATCH_R2_PUSH_N
		       && ! ((last == 31 && regno == 28)
			     || (last == 31 && regno <= 23)
			     || (last == 28 && regno <= 23)
			     || (regno < 23 && regno == last - 1)))
		   || (op->match == MATCH_R2_POP_N
		       && ! ((regno == 31 && last == 28)
			     || (regno == 31 && last <= 23)
			     || (regno == 28 && last <= 23)
			     || (last < 23 && last == regno - 1))))
	    {
	      as_bad ("invalid register order");
	      return 0;
	    }
	}

      mask |= 1UL << regno;
      last = regno;
    }

  /* Check that all ldwm/stwm regs belong to the same set.  */
  if ((op->match == MATCH_R2_LDWM || op->match == MATCH_R2_STWM)
      && (mask & 0x00003ffc) && (mask & 0x90ffc000))
    {
      as_bad ("invalid register set in reglist");
      return 0;
    }

  /* Check that push.n/pop.n regs include RA.  */
  if ((op->match == MATCH_R2_PUSH_N || op->match == MATCH_R2_POP_N)
      && ((mask & 0x80000000) == 0))
    {
      as_bad ("reglist must include ra (r31)");
      return 0;
    }

  /* Check that there is at least one register in the set.  */
  if (!mask)
    {
      as_bad ("reglist must include at least one register");
      return 0;
    }

  /* OK, reglist passed validation.  */
  nios2_reglist_mask = mask;
  nios2_reglist_dir = dir;
  return mask;
}

/* This function parses the base register and options used by the ldwm/stwm
   instructions.  Returns the base register and sets the option arguments
   accordingly.  On failure, returns NULL.  */
static struct nios2_reg *
nios2_parse_base_register (char *str, int *direction, int *writeback, int *ret)
{
  char *regname;
  struct nios2_reg *reg;

  *direction = 0;
  *writeback = 0;
  *ret = 0;

  /* Check for --.  */
  if (startswith (str, "--"))
    {
      str += 2;
      *direction -= 1;
    }

  /* Extract the base register.  */
  if (*str != '(')
    {
      as_bad ("expected '(' before base register");
      return NULL;
    }
  str++;
  regname = str;
  str = strchr (str, ')');
  if (!str)
    {
      as_bad ("expected ')' after base register");
      return NULL;
    }
  *str = '\0';
  str++;
  reg = nios2_parse_reg (regname, REG_NORMAL);
  if (reg == NULL)
    return NULL;

  /* Check for ++.  */
  if (startswith (str, "++"))
    {
      str += 2;
      *direction += 1;
    }

  /* Ensure that either -- or ++ is specified, but not both.  */
  if (*direction == 0)
    {
      as_bad ("invalid base register syntax");
      return NULL;;
    }

  /* Check for options.  The tokenizer has replaced commas with spaces.  */
  while (*str)
    {
      while (*str == ' ')
	str++;
      if (startswith (str, "writeback"))
	{
	  *writeback = 1;
	  str += 9;
	}
      else if (startswith (str, "ret"))
	{
	  *ret = 1;
	  str += 3;
	}
      else if (*str)
	{
	  as_bad ("invalid option syntax");
	  return NULL;
	}
    }

  return reg;
}


/* The various nios2_assemble_* functions call this
   function to generate an expression from a string representing an expression.
   It then tries to evaluate the expression, and if it can, returns its value.
   If not, it creates a new nios2_insn_relocS and stores the expression and
   reloc_type for future use.  */
static unsigned long
nios2_assemble_expression (const char *exprstr,
			   nios2_insn_infoS *insn,
			   bfd_reloc_code_real_type orig_reloc_type,
			   unsigned int pcrel)
{
  nios2_insn_relocS *reloc;
  char *saved_line_ptr;
  unsigned long value = 0;
  int i;
  bfd_reloc_code_real_type reloc_type = orig_reloc_type;

  gas_assert (exprstr != NULL);
  gas_assert (insn != NULL);

  /* Check for relocation operators.
     Change the relocation type and advance the ptr to the start of
     the expression proper. */
  for (i = 0; i < nios2_num_special_relocs; i++)
    if (strstr (exprstr, nios2_special_reloc[i].string) != NULL)
      {
	reloc_type = nios2_special_reloc[i].reloc_type;
	exprstr += strlen (nios2_special_reloc[i].string) + 1;

	/* %lo and %hiadj have different meanings for PC-relative
	   expressions.  */
	if (pcrel)
	  {
	    if (reloc_type == BFD_RELOC_NIOS2_LO16)
	      reloc_type = BFD_RELOC_NIOS2_PCREL_LO;
	    if (reloc_type == BFD_RELOC_NIOS2_HIADJ16)
	      reloc_type = BFD_RELOC_NIOS2_PCREL_HA;
	  }

	break;
      }

  /* No relocation allowed; we must have a constant expression.  */
  if (orig_reloc_type == BFD_RELOC_NONE)
    {
      expressionS exp;

      /* Parse the expression string.  */
      saved_line_ptr = input_line_pointer;
      input_line_pointer = (char *) exprstr;
      expression (&exp);
      input_line_pointer = saved_line_ptr;

      /* If we don't have a constant, give an error.  */
      if (reloc_type != orig_reloc_type || exp.X_op != O_constant)
	as_bad (_("expression must be constant"));
      else
	value = exp.X_add_number;
      return (unsigned long) value;
    }

  /* We potentially have a relocation.  */
  reloc = nios2_insn_reloc_new (reloc_type, pcrel);
  reloc->reloc_next = insn->insn_reloc;
  insn->insn_reloc = reloc;

  /* Parse the expression string.  */
  saved_line_ptr = input_line_pointer;
  input_line_pointer = (char *) exprstr;
  expression (&reloc->reloc_expression);
  input_line_pointer = saved_line_ptr;

  /* This is redundant as the fixup will put this into
     the instruction, but it is included here so that
     self-test mode (-r) works.  */
  if (nios2_mode == NIOS2_MODE_TEST
      && reloc->reloc_expression.X_op == O_constant)
    value = reloc->reloc_expression.X_add_number;

  return (unsigned long) value;
}

/* Encode a 3-bit register number, giving an error if this is not possible.  */
static unsigned int
nios2_assemble_reg3 (const char *token)
{
  struct nios2_reg *reg = nios2_parse_reg (token, REG_3BIT);
  int j;

  if (reg == NULL)
    return 0;

  for (j = 0; j < nios2_num_r2_reg3_mappings; j++)
    if (nios2_r2_reg3_mappings[j] == reg->index)
      return j;

  /* Should never get here if we passed validation.  */
  as_bad (_("invalid register %s"), token);
  return 0;
}

/* Argument assemble functions.  */


/* Control register index.  */
static void
nios2_assemble_arg_c (const char *token, nios2_insn_infoS *insn)
{
  struct nios2_reg *reg = nios2_parse_reg (token, REG_CONTROL);
  const struct nios2_opcode *op = insn->insn_nios2_opcode;

  if (reg == NULL)
    return;

  switch (op->format)
    {
    case iw_r_type:
      insn->insn_code |= SET_IW_R_IMM5 (reg->index);
      break;
    case iw_F3X6L5_type:
      insn->insn_code |= SET_IW_F3X6L5_IMM5 (reg->index);
      break;
    default:
      bad_opcode (op);
    }
}

/* Destination register.  */
static void
nios2_assemble_arg_d (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned long regtype = REG_NORMAL;
  struct nios2_reg *reg;

  if (op->format == iw_custom_type || op->format == iw_F3X8_type)
    regtype |= REG_COPROCESSOR;
  reg = nios2_parse_reg (token, regtype);
  if (reg == NULL)
    return;

  switch (op->format)
    {
    case iw_r_type:
      insn->insn_code |= SET_IW_R_C (reg->index);
      break;
    case iw_custom_type:
      insn->insn_code |= SET_IW_CUSTOM_C (reg->index);
      if (reg->regtype & REG_COPROCESSOR)
	insn->insn_code |= SET_IW_CUSTOM_READC (0);
      else
	insn->insn_code |= SET_IW_CUSTOM_READC (1);
      break;
    case iw_F3X6L5_type:
    case iw_F3X6_type:
      insn->insn_code |= SET_IW_F3X6L5_C (reg->index);
      break;
    case iw_F3X8_type:
      insn->insn_code |= SET_IW_F3X8_C (reg->index);
      if (reg->regtype & REG_COPROCESSOR)
	insn->insn_code |= SET_IW_F3X8_READC (0);
      else
	insn->insn_code |= SET_IW_F3X8_READC (1);
      break;
    case iw_F2_type:
      insn->insn_code |= SET_IW_F2_B (reg->index);
      break;
    default:
      bad_opcode (op);
    }
}

/* Source register 1.  */
static void
nios2_assemble_arg_s (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned long regtype = REG_NORMAL;
  struct nios2_reg *reg;

  if (op->format == iw_custom_type || op->format == iw_F3X8_type)
    regtype |= REG_COPROCESSOR;
  reg = nios2_parse_reg (token, regtype);
  if (reg == NULL)
    return;

  switch (op->format)
    {
    case iw_r_type:
      if (op->match == MATCH_R1_JMP && reg->index == 31)
	as_bad (_("r31 cannot be used with jmp; use ret instead"));
      insn->insn_code |= SET_IW_R_A (reg->index);
      break;
    case iw_i_type:
      insn->insn_code |= SET_IW_I_A (reg->index);
      break;
    case iw_custom_type:
      insn->insn_code |= SET_IW_CUSTOM_A (reg->index);
      if (reg->regtype & REG_COPROCESSOR)
	insn->insn_code |= SET_IW_CUSTOM_READA (0);
      else
	insn->insn_code |= SET_IW_CUSTOM_READA (1);
      break;
    case iw_F2I16_type:
      insn->insn_code |= SET_IW_F2I16_A (reg->index);
      break;
    case iw_F2X4I12_type:
      insn->insn_code |= SET_IW_F2X4I12_A (reg->index);
      break;
    case iw_F1X4I12_type:
      insn->insn_code |= SET_IW_F1X4I12_A (reg->index);
      break;
    case iw_F1X4L17_type:
      insn->insn_code |= SET_IW_F1X4L17_A (reg->index);
      break;
    case iw_F3X6L5_type:
    case iw_F3X6_type:
      if (op->match == MATCH_R2_JMP && reg->index == 31)
	as_bad (_("r31 cannot be used with jmp; use ret instead"));
      insn->insn_code |= SET_IW_F3X6L5_A (reg->index);
      break;
    case iw_F2X6L10_type:
      insn->insn_code |= SET_IW_F2X6L10_A (reg->index);
      break;
    case iw_F3X8_type:
      insn->insn_code |= SET_IW_F3X8_A (reg->index);
      if (reg->regtype & REG_COPROCESSOR)
	insn->insn_code |= SET_IW_F3X8_READA (0);
      else
	insn->insn_code |= SET_IW_F3X8_READA (1);
      break;
    case iw_F1X1_type:
      if (op->match == MATCH_R2_JMPR_N && reg->index == 31)
	as_bad (_("r31 cannot be used with jmpr.n; use ret.n instead"));
      insn->insn_code |= SET_IW_F1X1_A (reg->index);
      break;
    case iw_F1I5_type:
      /* Implicit stack pointer reference.  */
      if (reg->index != 27)
	as_bad (_("invalid register %s"), token);
      break;
    case iw_F2_type:
      insn->insn_code |= SET_IW_F2_A (reg->index);
      break;
    default:
      bad_opcode (op);
    }
}

/* Source register 2.  */
static void
nios2_assemble_arg_t (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned long regtype = REG_NORMAL;
  struct nios2_reg *reg;

  if (op->format == iw_custom_type || op->format == iw_F3X8_type)
    regtype |= REG_COPROCESSOR;
  reg = nios2_parse_reg (token, regtype);
  if (reg == NULL)
    return;

  switch (op->format)
    {
    case iw_r_type:
      insn->insn_code |= SET_IW_R_B (reg->index);
      break;
    case iw_i_type:
      insn->insn_code |= SET_IW_I_B (reg->index);
      break;
    case iw_custom_type:
      insn->insn_code |= SET_IW_CUSTOM_B (reg->index);
      if (reg->regtype & REG_COPROCESSOR)
	insn->insn_code |= SET_IW_CUSTOM_READB (0);
      else
	insn->insn_code |= SET_IW_CUSTOM_READB (1);
      break;
    case iw_F2I16_type:
      insn->insn_code |= SET_IW_F2I16_B (reg->index);
      break;
    case iw_F2X4I12_type:
      insn->insn_code |= SET_IW_F2X4I12_B (reg->index);
      break;
    case iw_F3X6L5_type:
    case iw_F3X6_type:
      insn->insn_code |= SET_IW_F3X6L5_B (reg->index);
      break;
    case iw_F2X6L10_type:
      insn->insn_code |= SET_IW_F2X6L10_B (reg->index);
      break;
    case iw_F3X8_type:
      insn->insn_code |= SET_IW_F3X8_B (reg->index);
      if (reg->regtype & REG_COPROCESSOR)
	insn->insn_code |= SET_IW_F3X8_READB (0);
      else
	insn->insn_code |= SET_IW_F3X8_READB (1);
      break;
    case iw_F1I5_type:
      insn->insn_code |= SET_IW_F1I5_B (reg->index);
      break;
    case iw_F2_type:
      insn->insn_code |= SET_IW_F2_B (reg->index);
      break;
    case iw_T1X1I6_type:
      /* Implicit zero register reference.  */
      if (reg->index != 0)
	as_bad (_("invalid register %s"), token);
      break;

    default:
      bad_opcode (op);
    }
}

/* Destination register w/3-bit encoding.  */
static void
nios2_assemble_arg_D (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  int reg = nios2_assemble_reg3 (token);

  switch (op->format)
    {
    case iw_T1I7_type:
      insn->insn_code |= SET_IW_T1I7_A3 (reg);
      break;
    case iw_T2X1L3_type:
      insn->insn_code |= SET_IW_T2X1L3_B3 (reg);
      break;
    case iw_T2X1I3_type:
      insn->insn_code |= SET_IW_T2X1I3_B3 (reg);
      break;
    case iw_T3X1_type:
      insn->insn_code |= SET_IW_T3X1_C3 (reg);
      break;
    case iw_T2X3_type:
      /* Some instructions using this encoding take 3 register arguments,
	 requiring the destination register to be the same as the first
	 source register.  */
      if (op->num_args == 3)
	insn->insn_code |= SET_IW_T2X3_A3 (reg);
      else
	insn->insn_code |= SET_IW_T2X3_B3 (reg);
      break;
    default:
      bad_opcode (op);
    }
}

/* Source register w/3-bit encoding.  */
static void
nios2_assemble_arg_S (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  int reg = nios2_assemble_reg3 (token);

  switch (op->format)
    {
    case iw_T1I7_type:
      insn->insn_code |= SET_IW_T1I7_A3 (reg);
      break;
    case iw_T2I4_type:
      insn->insn_code |= SET_IW_T2I4_A3 (reg);
      break;
    case iw_T2X1L3_type:
      insn->insn_code |= SET_IW_T2X1L3_A3 (reg);
      break;
    case iw_T2X1I3_type:
      insn->insn_code |= SET_IW_T2X1I3_A3 (reg);
      break;
    case iw_T3X1_type:
      insn->insn_code |= SET_IW_T3X1_A3 (reg);
      break;
    case iw_T2X3_type:
      /* Some instructions using this encoding take 3 register arguments,
	 requiring the destination register to be the same as the first
	 source register.  */
      if (op->num_args == 3)
	{
	  int dreg = GET_IW_T2X3_A3 (insn->insn_code);
	  if (dreg != reg)
	    as_bad ("source and destination registers must be the same");
	}
      else
	insn->insn_code |= SET_IW_T2X3_A3 (reg);
      break;
    case iw_T1X1I6_type:
      insn->insn_code |= SET_IW_T1X1I6_A3 (reg);
      break;
    default:
      bad_opcode (op);
    }
}

/* Source register 2 w/3-bit encoding.  */
static void
nios2_assemble_arg_T (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  int reg = nios2_assemble_reg3 (token);

  switch (op->format)
    {
    case iw_T2I4_type:
      insn->insn_code |= SET_IW_T2I4_B3 (reg);
      break;
    case iw_T3X1_type:
      insn->insn_code |= SET_IW_T3X1_B3 (reg);
      break;
    case iw_T2X3_type:
      insn->insn_code |= SET_IW_T2X3_B3 (reg);
      break;
    default:
      bad_opcode (op);
    }
}

/* 16-bit signed immediate.  */
static void
nios2_assemble_arg_i (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_i_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_S16, 0);
      insn->constant_bits |= SET_IW_I_IMM16 (val);
      break;
    case iw_F2I16_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_S16, 0);
      insn->constant_bits |= SET_IW_F2I16_IMM16 (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* 12-bit signed immediate.  */
static void
nios2_assemble_arg_I (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_F2X4I12_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_S12, 0);
      insn->constant_bits |= SET_IW_F2X4I12_IMM12 (val);
      break;
    case iw_F1X4I12_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_S12, 0);
      insn->constant_bits |= SET_IW_F2X4I12_IMM12 (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* 16-bit unsigned immediate.  */
static void
nios2_assemble_arg_u (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_i_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_U16, 0);
      insn->constant_bits |= SET_IW_I_IMM16 (val);
      break;
    case iw_F2I16_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_U16, 0);
      insn->constant_bits |= SET_IW_F2I16_IMM16 (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* 7-bit unsigned immediate with 2-bit shift.  */
static void
nios2_assemble_arg_U (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_T1I7_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_T1I7_2, 0);
      insn->constant_bits |= SET_IW_T1I7_IMM7 (val >> 2);
      break;
    case iw_X1I7_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_X1I7_2, 0);
      insn->constant_bits |= SET_IW_X1I7_IMM7 (val >> 2);
      break;
    default:
      bad_opcode (op);
    }
}

/* 5-bit unsigned immediate with 2-bit shift.  */
static void
nios2_assemble_arg_V (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_F1I5_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_F1I5_2, 0);
      insn->constant_bits |= SET_IW_F1I5_IMM5 (val >> 2);
      break;
    default:
      bad_opcode (op);
    }
}

/* 4-bit unsigned immediate with 2-bit shift.  */
static void
nios2_assemble_arg_W (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_T2I4_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_T2I4_2, 0);
      insn->constant_bits |= SET_IW_T2I4_IMM4 (val >> 2);
      break;
    case iw_L5I4X1_type:
      /* This argument is optional for push.n/pop.n, and defaults to
	 zero if unspecified.  */
      if (token == NULL)
	return;

      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_L5I4X1, 0);
      insn->constant_bits |= SET_IW_L5I4X1_IMM4 (val >> 2);
      break;
    default:
      bad_opcode (op);
    }
}

/* 4-bit unsigned immediate with 1-bit shift.  */
static void
nios2_assemble_arg_X (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_T2I4_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_T2I4_1, 0);
      insn->constant_bits |= SET_IW_T2I4_IMM4 (val >> 1);
      break;
    default:
      bad_opcode (op);
    }
}

/* 4-bit unsigned immediate without shift.  */
static void
nios2_assemble_arg_Y (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_T2I4_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_T2I4, 0);
      insn->constant_bits |= SET_IW_T2I4_IMM4 (val);
      break;
    default:
      bad_opcode (op);
    }
}


/* 16-bit signed immediate address offset.  */
static void
nios2_assemble_arg_o (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_i_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_16_PCREL, 1);
      insn->constant_bits |= SET_IW_I_IMM16 (val);
      break;
    case iw_F2I16_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_16_PCREL, 1);
      insn->constant_bits |= SET_IW_F2I16_IMM16 (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* 10-bit signed address offset with 1-bit shift.  */
static void
nios2_assemble_arg_O (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_I10_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_I10_1_PCREL, 1);
      insn->constant_bits |= SET_IW_I10_IMM10 (val >> 1);
      break;
    default:
      bad_opcode (op);
    }
}

/* 7-bit signed address offset with 1-bit shift.  */
static void
nios2_assemble_arg_P (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_T1I7_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_T1I7_1_PCREL, 1);
      insn->constant_bits |= SET_IW_T1I7_IMM7 (val >> 1);
      break;
    default:
      bad_opcode (op);
    }
}

/* 5-bit unsigned immediate.  */
static void
nios2_assemble_arg_j (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_r_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_IMM5, 0);
      insn->constant_bits |= SET_IW_R_IMM5 (val);
      break;
    case iw_F3X6L5_type:
      if (op->match == MATCH_R2_ENI)
	/* Value must be constant 0 or 1.  */
	{
	  val = nios2_assemble_expression (token, insn, BFD_RELOC_NONE, 0);
	  if (val != 0 && val != 1)
	    as_bad ("invalid eni argument %u", val);
	  insn->insn_code |= SET_IW_F3X6L5_IMM5 (val);
	}
      else
	{
	  val = nios2_assemble_expression (token, insn,
					   BFD_RELOC_NIOS2_IMM5, 0);
	  insn->constant_bits |= SET_IW_F3X6L5_IMM5 (val);
	}
      break;
    case iw_F2X6L10_type:
      /* Only constant expression without relocation permitted for
	 bit position.  */
      val = nios2_assemble_expression (token, insn, BFD_RELOC_NONE, 0);
      if (val > 31)
	as_bad ("invalid bit position %u", val);
      insn->insn_code |= SET_IW_F2X6L10_MSB (val);
      break;
    case iw_X2L5_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_X2L5, 0);
      insn->constant_bits |= SET_IW_X2L5_IMM5 (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* Second 5-bit unsigned immediate field.  */
static void
nios2_assemble_arg_k (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_F2X6L10_type:
      /* Only constant expression without relocation permitted for
	 bit position.  */
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NONE, 0);
      if (val > 31)
	as_bad ("invalid bit position %u", val);
      else if (GET_IW_F2X6L10_MSB (insn->insn_code) < val)
	as_bad ("MSB must be greater than or equal to LSB");
      insn->insn_code |= SET_IW_F2X6L10_LSB (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* 8-bit unsigned immediate.  */
static void
nios2_assemble_arg_l (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_custom_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_IMM8, 0);
      insn->constant_bits |= SET_IW_CUSTOM_N (val);
      break;
    case iw_F3X8_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_IMM8, 0);
      insn->constant_bits |= SET_IW_F3X8_N (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* 26-bit unsigned immediate.  */
static void
nios2_assemble_arg_m (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_j_type:
      val = nios2_assemble_expression (token, insn,
				       (nios2_as_options.noat
					? BFD_RELOC_NIOS2_CALL26_NOAT
					: BFD_RELOC_NIOS2_CALL26),
				       0);
      insn->constant_bits |= SET_IW_J_IMM26 (val);
      break;
    case iw_L26_type:
      val = nios2_assemble_expression (token, insn,
				       (nios2_as_options.noat
					? BFD_RELOC_NIOS2_CALL26_NOAT
					: BFD_RELOC_NIOS2_CALL26),
				       0);
      insn->constant_bits |= SET_IW_L26_IMM26 (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* 6-bit unsigned immediate with no shifting.  */
static void
nios2_assemble_arg_M (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_T1X1I6_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_T1X1I6, 0);
      insn->constant_bits |= SET_IW_T1X1I6_IMM6 (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* 6-bit unsigned immediate with 2-bit shift.  */
static void
nios2_assemble_arg_N (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;

  switch (op->format)
    {
    case iw_T1X1I6_type:
      val = nios2_assemble_expression (token, insn,
				       BFD_RELOC_NIOS2_R2_T1X1I6_2, 0);
      insn->constant_bits |= SET_IW_T1X1I6_IMM6 (val >> 2);
      break;
    default:
      bad_opcode (op);
    }
}


/* Encoded enumeration for addi.n/subi.n.  */
static void
nios2_assemble_arg_e (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;
  int i;

  switch (op->format)
    {
    case iw_T2X1I3_type:
      val = nios2_assemble_expression (token, insn, BFD_RELOC_NONE, 0);
      for (i = 0; i < nios2_num_r2_asi_n_mappings; i++)
	if (val == nios2_r2_asi_n_mappings[i])
	  break;
      if (i == nios2_num_r2_asi_n_mappings)
	{
	  as_bad (_("Invalid constant operand %s"), token);
	  return;
	}
      insn->insn_code |= SET_IW_T2X1I3_IMM3 ((unsigned)i);
      break;
    default:
      bad_opcode (op);
    }
}

/* Encoded enumeration for slli.n/srli.n.  */
static void
nios2_assemble_arg_f (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;
  int i;

  switch (op->format)
    {
    case iw_T2X1L3_type:
      val = nios2_assemble_expression (token, insn, BFD_RELOC_NONE, 0);
      for (i = 0; i < nios2_num_r2_shi_n_mappings; i++)
	if (val == nios2_r2_shi_n_mappings[i])
	  break;
      if (i == nios2_num_r2_shi_n_mappings)
	{
	  as_bad (_("Invalid constant operand %s"), token);
	  return;
	}
      insn->insn_code |= SET_IW_T2X1L3_SHAMT ((unsigned)i);
      break;
    default:
      bad_opcode (op);
    }
}

/* Encoded enumeration for andi.n.  */
static void
nios2_assemble_arg_g (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;
  int i;

  switch (op->format)
    {
    case iw_T2I4_type:
      val = nios2_assemble_expression (token, insn, BFD_RELOC_NONE, 0);
      for (i = 0; i < nios2_num_r2_andi_n_mappings; i++)
	if (val == nios2_r2_andi_n_mappings[i])
	  break;
      if (i == nios2_num_r2_andi_n_mappings)
	{
	  as_bad (_("Invalid constant operand %s"), token);
	  return;
	}
      insn->insn_code |= SET_IW_T2I4_IMM4 ((unsigned)i);
      break;
    default:
      bad_opcode (op);
    }
}

/* Encoded enumeration for movi.n.  */
static void
nios2_assemble_arg_h (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned int val;
  int i;

  switch (op->format)
    {
    case iw_T1I7_type:
      val = nios2_assemble_expression (token, insn, BFD_RELOC_NONE, 0);
      i = (signed) val;
      if ((signed) i == -1)
	val = 127;
      else if (i == -2)
	val = 126;
      else if (i == 0xff)
	val = 125;
      else if (i < 0 || i > 125)
	{
	  as_bad (_("Invalid constant operand %s"), token);
	  return;
	}
      insn->insn_code |= SET_IW_T1I7_IMM7 (val);
      break;
    default:
      bad_opcode (op);
    }
}

/* Encoded REGMASK for ldwm/stwm or push.n/pop.n.  */
static void
nios2_assemble_arg_R (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  unsigned long mask;
  char *buf = strdup (token);
  unsigned long reglist = nios2_parse_reglist (buf, op);
  free (buf);

  if (reglist == 0)
    return;

  switch (op->format)
    {
    case iw_F1X4L17_type:
      /* Encoding for ldwm/stwm.  */
      if (reglist & 0x00003ffc)
	mask = reglist >> 2;
      else
	{
	  insn->insn_code |= SET_IW_F1X4L17_RS (1);
	  mask = (reglist & 0x00ffc000) >> 14;
	  if (reglist & (1 << 28))
	    mask |= 1 << 10;
	  if (reglist & (1u << 31))
	    mask |= 1 << 11;
	}
      insn->insn_code |= SET_IW_F1X4L17_REGMASK (mask);
      break;

    case iw_L5I4X1_type:
      /* Encoding for push.n/pop.n.  */
      if (reglist & (1 << 28))
	insn->insn_code |= SET_IW_L5I4X1_FP (1);
      mask = reglist & 0x00ff0000;
      if (mask)
	{
	  int i;

	  for (i = 0; i < nios2_num_r2_reg_range_mappings; i++)
	    if (nios2_r2_reg_range_mappings[i] == mask)
	      break;
	  if (i == nios2_num_r2_reg_range_mappings)
	    {
	      as_bad ("invalid reglist");
	      return;
	    }
	  insn->insn_code |= SET_IW_L5I4X1_REGRANGE (i);
	  insn->insn_code |= SET_IW_L5I4X1_CS (1);
	}
      break;

    default:
      bad_opcode (op);
    }
}

/* Base register for ldwm/stwm.  */
static void
nios2_assemble_arg_B (const char *token, nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  int direction, writeback, ret;
  char *str = strdup (token);
  struct nios2_reg *reg
    = nios2_parse_base_register (str, &direction, &writeback, &ret);

  free (str);
  if (!reg)
    return;

  switch (op->format)
    {
    case iw_F1X4L17_type:
      /* For ldwm, check to see if the base register is already inside the
	 register list.  */
      if (op->match == MATCH_R2_LDWM
	  && (nios2_reglist_mask & (1 << reg->index)))
	{
	  as_bad ("invalid base register; %s is inside the reglist", reg->name);
	  return;
	}

      /* For stwm, ret option is not allowed.  */
      if (op->match == MATCH_R2_STWM && ret)
	{
	  as_bad ("invalid option syntax");
	  return;
	}

      /* Check that the direction matches the ordering of the reglist.  */
      if (nios2_reglist_dir && direction != nios2_reglist_dir)
	{
	  as_bad ("reglist order does not match increment/decrement mode");
	  return;
	}

      insn->insn_code |= SET_IW_F1X4L17_A (reg->index);
      if (direction > 0)
	insn->insn_code |= SET_IW_F1X4L17_ID (1);
      if (writeback)
	insn->insn_code |= SET_IW_F1X4L17_WB (1);
      if (ret)
	insn->insn_code |= SET_IW_F1X4L17_PC (1);
      break;

    default:
      bad_opcode (op);
    }
}

static void
nios2_assemble_args (nios2_insn_infoS *insn)
{
  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  const char *argptr;
  unsigned int tokidx, ntok;

  /* Make sure there are enough arguments.  */
  ntok = (op->pinfo & NIOS2_INSN_OPTARG) ? op->num_args - 1 : op->num_args;
  for (tokidx = 1; tokidx <= ntok; tokidx++)
    if (insn->insn_tokens[tokidx] == NULL)
      {
	as_bad ("missing argument");
	return;
      }

  for (argptr = op->args, tokidx = 1;
       *argptr && insn->insn_tokens[tokidx];
       argptr++)
    switch (*argptr)
      {
      case ',':
      case '(':
      case ')':
	break;

      case 'c':
	nios2_assemble_arg_c (insn->insn_tokens[tokidx++], insn);
	break;

      case 'd':
	nios2_assemble_arg_d (insn->insn_tokens[tokidx++], insn);
	break;

      case 's':
	nios2_assemble_arg_s (insn->insn_tokens[tokidx++], insn);
	break;

      case 't':
	nios2_assemble_arg_t (insn->insn_tokens[tokidx++], insn);
	break;

      case 'D':
	nios2_assemble_arg_D (insn->insn_tokens[tokidx++], insn);
	break;

      case 'S':
	nios2_assemble_arg_S (insn->insn_tokens[tokidx++], insn);
	break;

      case 'T':
	nios2_assemble_arg_T (insn->insn_tokens[tokidx++], insn);
	break;

      case 'i':
	nios2_assemble_arg_i (insn->insn_tokens[tokidx++], insn);
	break;

      case 'I':
	nios2_assemble_arg_I (insn->insn_tokens[tokidx++], insn);
	break;

      case 'u':
	nios2_assemble_arg_u (insn->insn_tokens[tokidx++], insn);
	break;

      case 'U':
	nios2_assemble_arg_U (insn->insn_tokens[tokidx++], insn);
	break;

      case 'V':
	nios2_assemble_arg_V (insn->insn_tokens[tokidx++], insn);
	break;

      case 'W':
	nios2_assemble_arg_W (insn->insn_tokens[tokidx++], insn);
	break;

      case 'X':
	nios2_assemble_arg_X (insn->insn_tokens[tokidx++], insn);
	break;

      case 'Y':
	nios2_assemble_arg_Y (insn->insn_tokens[tokidx++], insn);
	break;

      case 'o':
	nios2_assemble_arg_o (insn->insn_tokens[tokidx++], insn);
	break;

      case 'O':
	nios2_assemble_arg_O (insn->insn_tokens[tokidx++], insn);
	break;

      case 'P':
	nios2_assemble_arg_P (insn->insn_tokens[tokidx++], insn);
	break;

      case 'j':
	nios2_assemble_arg_j (insn->insn_tokens[tokidx++], insn);
	break;

      case 'k':
	nios2_assemble_arg_k (insn->insn_tokens[tokidx++], insn);
	break;

      case 'l':
	nios2_assemble_arg_l (insn->insn_tokens[tokidx++], insn);
	break;

      case 'm':
	nios2_assemble_arg_m (insn->insn_tokens[tokidx++], insn);
	break;

      case 'M':
	nios2_assemble_arg_M (insn->insn_tokens[tokidx++], insn);
	break;

      case 'N':
	nios2_assemble_arg_N (insn->insn_tokens[tokidx++], insn);
	break;

      case 'e':
	nios2_assemble_arg_e (insn->insn_tokens[tokidx++], insn);
	break;

      case 'f':
	nios2_assemble_arg_f (insn->insn_tokens[tokidx++], insn);
	break;

      case 'g':
	nios2_assemble_arg_g (insn->insn_tokens[tokidx++], insn);
	break;

      case 'h':
	nios2_assemble_arg_h (insn->insn_tokens[tokidx++], insn);
	break;

      case 'R':
	nios2_assemble_arg_R (insn->insn_tokens[tokidx++], insn);
	break;

      case 'B':
	nios2_assemble_arg_B (insn->insn_tokens[tokidx++], insn);
	break;

      default:
	bad_opcode (op);
	break;
      }

  /* Perform argument checking.  */
  nios2_check_assembly (insn->insn_code | insn->constant_bits,
			insn->insn_tokens[tokidx]);
}


/* The function consume_arg takes a pointer into a string
   of instruction tokens (args) and a pointer into a string
   representing the expected sequence of tokens and separators.
   It checks whether the first argument in argstr is of the
   expected type, throwing an error if it is not, and returns
   the pointer argstr.  */
static char *
nios2_consume_arg (char *argstr, const char *parsestr)
{
  char *temp;

  switch (*parsestr)
    {
    case 'c':
    case 'd':
    case 's':
    case 't':
    case 'D':
    case 'S':
    case 'T':
      break;

    case 'i':
    case 'u':
      if (*argstr == '%')
	{
	  if (nios2_special_relocation_p (argstr))
	    {
	      /* We zap the parentheses because we don't want them confused
		 with separators.  */
	      temp = strchr (argstr, '(');
	      if (temp != NULL)
		*temp = ' ';
	      temp = strchr (argstr, ')');
	      if (temp != NULL)
		*temp = ' ';
	    }
	  else
	    as_bad (_("badly formed expression near %s"), argstr);
	}
      break;
    case 'm':
    case 'j':
    case 'k':
    case 'l':
    case 'I':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'O':
    case 'P':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'M':
    case 'N':

      /* We can't have %hi, %lo or %hiadj here.  */
      if (*argstr == '%')
	as_bad (_("badly formed expression near %s"), argstr);
      break;

    case 'R':
      /* Register list for ldwm/stwm or push.n/pop.n.  Replace the commas
	 in the list with spaces so we don't confuse them with separators.  */
      if (*argstr != '{')
	{
	  as_bad ("missing '{' in register list");
	  break;
	}
      for (temp = argstr + 1; *temp; temp++)
	{
	  if (*temp == '}')
	    break;
	  else if (*temp == ',')
	    *temp = ' ';
	}
      if (!*temp)
	{
	  as_bad ("missing '}' in register list");
	  break;
	}
      break;

    case 'B':
      /* Base register and options for ldwm/stwm.  This is the final argument
	 and consumes the rest of the argument string; replace commas
	 with spaces so that the token splitter doesn't think they are
	 separate arguments.  */
      for (temp = argstr; *temp; temp++)
	if (*temp == ',')
	  *temp = ' ';
      break;

    case 'o':
    case 'E':
      break;
    default:
      BAD_CASE (*parsestr);
      break;
    }

  return argstr;
}

/* The function consume_separator takes a pointer into a string
   of instruction tokens (args) and a pointer into a string representing
   the expected sequence of tokens and separators.  It finds the first
   instance of the character pointed to by separator in argstr, and
   returns a pointer to the next element of argstr, which is the
   following token in the sequence.  */
static char *
nios2_consume_separator (char *argstr, const char *separator)
{
  char *p;

  /* If we have a opcode reg, expr(reg) type instruction, and
   * we are separating the expr from the (reg), we find the last
   * (, just in case the expression has parentheses.  */

  if (*separator == '(')
    p = strrchr (argstr, *separator);
  else
    p = strchr (argstr, *separator);

  if (p != NULL)
    *p++ = 0;
  return p;
}

/* The principal argument parsing function which takes a string argstr
   representing the instruction arguments for insn, and extracts the argument
   tokens matching parsestr into parsed_args.  */
static void
nios2_parse_args (nios2_insn_infoS *insn, char *argstr,
		  const char *parsestr, char **parsed_args)
{
  char *p;
  char *end = NULL;
  int i;
  p = argstr;
  i = 0;
  bool terminate = false;

  /* This rest of this function is it too fragile and it mostly works,
     therefore special case this one.  */
  if (*parsestr == 0 && argstr != 0)
    {
      as_bad (_("too many arguments"));
      parsed_args[0] = NULL;
      return;
    }

  while (p != NULL && !terminate && i < NIOS2_MAX_INSN_TOKENS)
    {
      parsed_args[i] = nios2_consume_arg (p, parsestr);
      ++parsestr;
      while (*parsestr == '(' || *parsestr == ')' || *parsestr == ',')
	{
	  char *context = p;
	  p = nios2_consume_separator (p, parsestr);
	  /* Check for missing separators.  */
	  if (!p && !(insn->insn_nios2_opcode->pinfo & NIOS2_INSN_OPTARG))
	    {
	      as_bad (_("expecting %c near %s"), *parsestr, context);
	      break;
	    }
	  ++parsestr;
	}

      if (*parsestr == '\0')
	{
	  /* Check that the argument string has no trailing arguments.  */
	  end = strpbrk (p, ",");
	  if (end != NULL)
	    as_bad (_("too many arguments"));
	}

      if (*parsestr == '\0' || (p != NULL && *p == '\0'))
	terminate = true;
      ++i;
    }

  parsed_args[i] = NULL;
}



/** Support for pseudo-op parsing.  These are macro-like opcodes that
    expand into real insns by suitable fiddling with the operands.  */

/* Append the string modifier to the string contained in the argument at
   parsed_args[ndx].  */
static void
nios2_modify_arg (char **parsed_args, const char *modifier,
		  int unused ATTRIBUTE_UNUSED, int ndx)
{
  char *tmp = parsed_args[ndx];

  parsed_args[ndx] = concat (tmp, modifier, (char *) NULL);
}

/* Modify parsed_args[ndx] by negating that argument.  */
static void
nios2_negate_arg (char **parsed_args, const char *modifier ATTRIBUTE_UNUSED,
		  int unused ATTRIBUTE_UNUSED, int ndx)
{
  char *tmp = parsed_args[ndx];

  parsed_args[ndx] = concat ("~(", tmp, ")+1", (char *) NULL);
}

/* The function nios2_swap_args swaps the pointers at indices index_1 and
   index_2 in the array parsed_args[] - this is used for operand swapping
   for comparison operations.  */
static void
nios2_swap_args (char **parsed_args, const char *unused ATTRIBUTE_UNUSED,
		 int index_1, int index_2)
{
  char *tmp;
  gas_assert (index_1 < NIOS2_MAX_INSN_TOKENS
	      && index_2 < NIOS2_MAX_INSN_TOKENS);
  tmp = parsed_args[index_1];
  parsed_args[index_1] = parsed_args[index_2];
  parsed_args[index_2] = tmp;
}

/* This function appends the string appnd to the array of strings in
   parsed_args num times starting at index start in the array.  */
static void
nios2_append_arg (char **parsed_args, const char *appnd, int num,
		  int start)
{
  int i, count;
  char *tmp;

  gas_assert ((start + num) < NIOS2_MAX_INSN_TOKENS);

  if (nios2_mode == NIOS2_MODE_TEST)
    tmp = parsed_args[start];
  else
    tmp = NULL;

  for (i = start, count = num; count > 0; ++i, --count)
    parsed_args[i] = (char *) appnd;

  gas_assert (i == (start + num));
  parsed_args[i] = tmp;
  parsed_args[i + 1] = NULL;
}

/* This function inserts the string insert num times in the array
   parsed_args, starting at the index start.  */
static void
nios2_insert_arg (char **parsed_args, const char *insert, int num,
		  int start)
{
  int i, count;

  gas_assert ((start + num) < NIOS2_MAX_INSN_TOKENS);

  /* Move the existing arguments up to create space.  */
  for (i = NIOS2_MAX_INSN_TOKENS; i - num >= start; --i)
    parsed_args[i] = parsed_args[i - num];

  for (i = start, count = num; count > 0; ++i, --count)
    parsed_args[i] = (char *) insert;
}

/* Cleanup function to free malloc'ed arg strings.  */
static void
nios2_free_arg (char **parsed_args, int num ATTRIBUTE_UNUSED, int start)
{
  free (parsed_args[start]);
  parsed_args[start] = NULL;
}

/* This function swaps the pseudo-op for a real op.  */
static nios2_ps_insn_infoS*
nios2_translate_pseudo_insn (nios2_insn_infoS *insn)
{

  const struct nios2_opcode *op = insn->insn_nios2_opcode;
  nios2_ps_insn_infoS *ps_insn;
  unsigned int tokidx, ntok;

  /* Find which real insn the pseudo-op translates to and
     switch the insn_info ptr to point to it.  */
  ps_insn = nios2_ps_lookup (op->name);

  if (ps_insn != NULL)
    {
      insn->insn_nios2_opcode = nios2_opcode_lookup (ps_insn->insn);
      insn->insn_tokens[0] = insn->insn_nios2_opcode->name;
      
      /* Make sure there are enough arguments.  */
      ntok = ((op->pinfo & NIOS2_INSN_OPTARG)
	      ? op->num_args - 1 : op->num_args);
      for (tokidx = 1; tokidx <= ntok; tokidx++)
	if (insn->insn_tokens[tokidx] == NULL)
	  {
	    as_bad ("missing argument");
	    return NULL;
	  }

      /* Modify the args so they work with the real insn.  */
      ps_insn->arg_modifer_func ((char **) insn->insn_tokens,
				 ps_insn->arg_modifier, ps_insn->num,
				 ps_insn->index);
    }
  else
    /* we cannot recover from this.  */
    as_fatal (_("unrecognized pseudo-instruction %s"),
	      insn->insn_nios2_opcode->name);
  return ps_insn;
}

/* Invoke the cleanup handler for pseudo-insn ps_insn on insn.  */
static void
nios2_cleanup_pseudo_insn (nios2_insn_infoS *insn,
			   nios2_ps_insn_infoS *ps_insn)
{
  if (ps_insn->arg_cleanup_func)
    (ps_insn->arg_cleanup_func) ((char **) insn->insn_tokens,
				 ps_insn->num, ps_insn->index);
}

const nios2_ps_insn_infoS nios2_ps_insn_info_structs[] = {
  /* pseudo-op, real-op, arg, arg_modifier_func, num, index, arg_cleanup_func */
  {"mov", "add", nios2_append_arg, "zero", 1, 3, NULL},
  {"movi", "addi", nios2_insert_arg, "zero", 1, 2, NULL},
  {"movhi", "orhi", nios2_insert_arg, "zero", 1, 2, NULL},
  {"movui", "ori", nios2_insert_arg, "zero", 1, 2, NULL},
  {"movia", "orhi", nios2_insert_arg, "zero", 1, 2, NULL},
  {"nop", "add", nios2_append_arg, "zero", 3, 1, NULL},
  {"bgt", "blt", nios2_swap_args, "", 1, 2, NULL},
  {"bgtu", "bltu", nios2_swap_args, "", 1, 2, NULL},
  {"ble", "bge", nios2_swap_args, "", 1, 2, NULL},
  {"bleu", "bgeu", nios2_swap_args, "", 1, 2, NULL},
  {"cmpgt", "cmplt", nios2_swap_args, "", 2, 3, NULL},
  {"cmpgtu", "cmpltu", nios2_swap_args, "", 2, 3, NULL},
  {"cmple", "cmpge", nios2_swap_args, "", 2, 3, NULL},
  {"cmpleu", "cmpgeu", nios2_swap_args, "", 2, 3, NULL},
  {"cmpgti", "cmpgei", nios2_modify_arg, "+1", 0, 3, nios2_free_arg},
  {"cmpgtui", "cmpgeui", nios2_modify_arg, "+1", 0, 3, nios2_free_arg},
  {"cmplei", "cmplti", nios2_modify_arg, "+1", 0, 3, nios2_free_arg},
  {"cmpleui", "cmpltui", nios2_modify_arg, "+1", 0, 3, nios2_free_arg},
  {"subi", "addi", nios2_negate_arg, "", 0, 3, nios2_free_arg},
  {"nop.n", "mov.n", nios2_append_arg, "zero", 2, 1, NULL}
  /* Add further pseudo-ops here.  */
};

#define NIOS2_NUM_PSEUDO_INSNS \
	((sizeof(nios2_ps_insn_info_structs)/ \
	  sizeof(nios2_ps_insn_info_structs[0])))
const int nios2_num_ps_insn_info_structs = NIOS2_NUM_PSEUDO_INSNS;


/** Assembler output support.  */

/* Output a normal instruction.  */
static void
output_insn (nios2_insn_infoS *insn)
{
  char *f;
  nios2_insn_relocS *reloc;
  f = frag_more (insn->insn_nios2_opcode->size);
  /* This allocates enough space for the instruction
     and puts it in the current frag.  */
  md_number_to_chars (f, insn->insn_code, insn->insn_nios2_opcode->size);
  /* Emit debug info.  */
  dwarf2_emit_insn (insn->insn_nios2_opcode->size);
  /* Create any fixups to be acted on later.  */

  for (reloc = insn->insn_reloc; reloc != NULL; reloc = reloc->reloc_next)
    fix_new_exp (frag_now, f - frag_now->fr_literal,
		 insn->insn_nios2_opcode->size,
		 &reloc->reloc_expression, reloc->reloc_pcrel,
		 reloc->reloc_type);
}

/* Output an unconditional branch.  */
static void
output_ubranch (nios2_insn_infoS *insn)
{
  nios2_insn_relocS *reloc = insn->insn_reloc;

  /* If the reloc is NULL, there was an error assembling the branch.  */
  if (reloc != NULL)
    {
      symbolS *symp = reloc->reloc_expression.X_add_symbol;
      offsetT offset = reloc->reloc_expression.X_add_number;
      char *f;
      bool is_cdx = (insn->insn_nios2_opcode->size == 2);

      /* Tag dwarf2 debug info to the address at the start of the insn.
	 We must do it before frag_var() below closes off the frag.  */
      dwarf2_emit_insn (0);

      /* We create a machine dependent frag which can grow
	 to accommodate the largest possible instruction sequence
	 this may generate.  */
      f = frag_var (rs_machine_dependent,
		    UBRANCH_MAX_SIZE, insn->insn_nios2_opcode->size,
		    (is_cdx ? CDX_UBRANCH_SUBTYPE (0) : UBRANCH_SUBTYPE (0)),
		    symp, offset, NULL);

      md_number_to_chars (f, insn->insn_code, insn->insn_nios2_opcode->size);

      /* We leave fixup generation to md_convert_frag.  */
    }
}

/* Output a conditional branch.  */
static void
output_cbranch (nios2_insn_infoS *insn)
{
  nios2_insn_relocS *reloc = insn->insn_reloc;

  /* If the reloc is NULL, there was an error assembling the branch.  */
  if (reloc != NULL)
    {
      symbolS *symp = reloc->reloc_expression.X_add_symbol;
      offsetT offset = reloc->reloc_expression.X_add_number;
      char *f;
      bool is_cdx = (insn->insn_nios2_opcode->size == 2);

      /* Tag dwarf2 debug info to the address at the start of the insn.
	 We must do it before frag_var() below closes off the frag.  */
      dwarf2_emit_insn (0);

      /* We create a machine dependent frag which can grow
	 to accommodate the largest possible instruction sequence
	 this may generate.  */
      f = frag_var (rs_machine_dependent,
		    CBRANCH_MAX_SIZE, insn->insn_nios2_opcode->size,
		    (is_cdx ? CDX_CBRANCH_SUBTYPE (0) : CBRANCH_SUBTYPE (0)),
		    symp, offset, NULL);

      md_number_to_chars (f, insn->insn_code, insn->insn_nios2_opcode->size);

      /* We leave fixup generation to md_convert_frag.  */
    }
}

/* Output a call sequence.  Since calls are not pc-relative for NIOS2,
   but are page-relative, we cannot tell at any stage in assembly
   whether a call will be out of range since a section may be linked
   at any address.  So if we are relaxing, we convert all call instructions
   to long call sequences, and rely on the linker to relax them back to
   short calls.  */
static void
output_call (nios2_insn_infoS *insn)
{
  /* This allocates enough space for the instruction
     and puts it in the current frag.  */
  char *f = frag_more (12);
  nios2_insn_relocS *reloc = insn->insn_reloc;
  const struct nios2_opcode *op = insn->insn_nios2_opcode;

  switch (op->format)
    {
    case iw_j_type:
      md_number_to_chars (f,
			  (MATCH_R1_ORHI | SET_IW_I_B (AT_REGNUM)
			   | SET_IW_I_A (0)),
			  4);
      dwarf2_emit_insn (4);
      fix_new_exp (frag_now, f - frag_now->fr_literal, 4,
		   &reloc->reloc_expression, 0, BFD_RELOC_NIOS2_HI16);
      md_number_to_chars (f + 4,
			  (MATCH_R1_ORI | SET_IW_I_B (AT_REGNUM)
			   | SET_IW_I_A (AT_REGNUM)),
			  4);
      dwarf2_emit_insn (4);
      fix_new_exp (frag_now, f - frag_now->fr_literal + 4, 4,
		   &reloc->reloc_expression, 0, BFD_RELOC_NIOS2_LO16);
      md_number_to_chars (f + 8, MATCH_R1_CALLR | SET_IW_R_A (AT_REGNUM), 4);
      dwarf2_emit_insn (4);
      break;
    case iw_L26_type:
      md_number_to_chars (f,
			  (MATCH_R2_ORHI | SET_IW_F2I16_B (AT_REGNUM)
			   | SET_IW_F2I16_A (0)),
			  4);
      dwarf2_emit_insn (4);
      fix_new_exp (frag_now, f - frag_now->fr_literal, 4,
		   &reloc->reloc_expression, 0, BFD_RELOC_NIOS2_HI16);
      md_number_to_chars (f + 4,
			  (MATCH_R2_ORI | SET_IW_F2I16_B (AT_REGNUM)
			   | SET_IW_F2I16_A (AT_REGNUM)),
			  4);
      dwarf2_emit_insn (4);
      fix_new_exp (frag_now, f - frag_now->fr_literal + 4, 4,
		   &reloc->reloc_expression, 0, BFD_RELOC_NIOS2_LO16);
      md_number_to_chars (f + 8, MATCH_R2_CALLR | SET_IW_F3X6L5_A (AT_REGNUM),
			  4);
      dwarf2_emit_insn (4);
      break;
    default:
      bad_opcode (op);
    }
}

/* Output a movhi/addi pair for the movia pseudo-op.  */
static void
output_movia (nios2_insn_infoS *insn)
{
  /* This allocates enough space for the instruction
     and puts it in the current frag.  */
  char *f = frag_more (8);
  nios2_insn_relocS *reloc = insn->insn_reloc;
  unsigned long reg, code = 0;
  const struct nios2_opcode *op = insn->insn_nios2_opcode;

  /* If the reloc is NULL, there was an error assembling the movia.  */
  if (reloc != NULL)
    {
      switch (op->format)
	{
	case iw_i_type:
	  reg = GET_IW_I_B (insn->insn_code);
	  code = MATCH_R1_ADDI | SET_IW_I_A (reg) | SET_IW_I_B (reg);
	  break;
	case iw_F2I16_type:
	  reg = GET_IW_F2I16_B (insn->insn_code);
	  code = MATCH_R2_ADDI | SET_IW_F2I16_A (reg) | SET_IW_F2I16_B (reg);
	  break;
	default:
	  bad_opcode (op);
	}

      md_number_to_chars (f, insn->insn_code, 4);
      dwarf2_emit_insn (4);
      fix_new (frag_now, f - frag_now->fr_literal, 4,
	       reloc->reloc_expression.X_add_symbol,
	       reloc->reloc_expression.X_add_number, 0,
	       BFD_RELOC_NIOS2_HIADJ16);
      md_number_to_chars (f + 4, code, 4);
      dwarf2_emit_insn (4);
      fix_new (frag_now, f + 4 - frag_now->fr_literal, 4,
	       reloc->reloc_expression.X_add_symbol,
	       reloc->reloc_expression.X_add_number, 0, BFD_RELOC_NIOS2_LO16);
    }
}



/** External interfaces.  */

/* Update the selected architecture based on ARCH, giving an error if
   ARCH is an invalid value.  */

static void
nios2_use_arch (const char *arch)
{
  if (strcmp (arch, "nios2") == 0 || strcmp (arch, "r1") == 0)
    {
      nios2_architecture |= EF_NIOS2_ARCH_R1;
      nios2_opcodes = (struct nios2_opcode *) nios2_r1_opcodes;
      nios2_num_opcodes = nios2_num_r1_opcodes;
      nop32 = nop_r1;
      nop16 = NULL;
      return;
    }
  else if (strcmp (arch, "r2") == 0)
    {
      nios2_architecture |= EF_NIOS2_ARCH_R2;
      nios2_opcodes = (struct nios2_opcode *) nios2_r2_opcodes;
      nios2_num_opcodes = nios2_num_r2_opcodes;
      nop32 = nop_r2;
      nop16 = nop_r2_cdx;
      return;
    }

  as_bad (_("unknown architecture '%s'"), arch);
}

/* The following functions are called by machine-independent parts of
   the assembler. */
int
md_parse_option (int c, const char *arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    case 'r':
      /* Hidden option for self-test mode.  */
      nios2_mode = NIOS2_MODE_TEST;
      break;
    case OPTION_RELAX_ALL:
      nios2_as_options.relax = relax_all;
      break;
    case OPTION_NORELAX:
      nios2_as_options.relax = relax_none;
      break;
    case OPTION_RELAX_SECTION:
      nios2_as_options.relax = relax_section;
      break;
    case OPTION_EB:
      target_big_endian = 1;
      break;
    case OPTION_EL:
      target_big_endian = 0;
      break;
    case OPTION_MARCH:
      nios2_use_arch (arg);
      break;
    default:
      return 0;
      break;
    }

  return 1;
}

/* Implement TARGET_FORMAT.  We can choose to be big-endian or
   little-endian at runtime based on a switch.  */
const char *
nios2_target_format (void)
{
  return target_big_endian ? "elf32-bignios2" : "elf32-littlenios2";
}

/* Machine-dependent usage message. */
void
md_show_usage (FILE *stream)
{
  fprintf (stream, "	    NIOS2 options:\n"
	   "  -relax-all	    replace all branch and call "
	   "instructions with jmp and callr sequences\n"
	   "  -relax-section	    replace identified out of range "
	   "branches with jmp sequences (default)\n"
	   "  -no-relax		    do not replace any branches or calls\n"
	   "  -EB		    force big-endian byte ordering\n"
	   "  -EL		    force little-endian byte ordering\n"
	   "  -march=ARCH	    enable instructions from architecture ARCH\n");
}


/* This function is called once, at assembler startup time.
   It should set up all the tables, etc. that the MD part of the
   assembler will need. */
void
md_begin (void)
{
  int i;

  switch (nios2_architecture)
    {
    default:
    case EF_NIOS2_ARCH_R1:
      bfd_default_set_arch_mach (stdoutput, bfd_arch_nios2, bfd_mach_nios2r1);
      break;
    case EF_NIOS2_ARCH_R2:
      if (target_big_endian)
	as_fatal (_("Big-endian R2 is not supported."));
      bfd_default_set_arch_mach (stdoutput, bfd_arch_nios2, bfd_mach_nios2r2);
      break;
    }

  /* Create and fill a hashtable for the Nios II opcodes, registers and
     arguments.  */
  nios2_opcode_hash = str_htab_create ();
  nios2_reg_hash = str_htab_create ();
  nios2_ps_hash = str_htab_create ();

  for (i = 0; i < nios2_num_opcodes; ++i)
    if (str_hash_insert (nios2_opcode_hash, nios2_opcodes[i].name,
			 &nios2_opcodes[i], 0) != NULL)
      as_fatal (_("duplicate %s"), nios2_opcodes[i].name);

  for (i = 0; i < nios2_num_regs; ++i)
    if (str_hash_insert (nios2_reg_hash, nios2_regs[i].name,
			 &nios2_regs[i], 0) != NULL)
      as_fatal (_("duplicate %s"), nios2_regs[i].name);

  for (i = 0; i < nios2_num_ps_insn_info_structs; ++i)
    if (str_hash_insert (nios2_ps_hash,
			 nios2_ps_insn_info_structs[i].pseudo_insn,
			 &nios2_ps_insn_info_structs[i], 0) != NULL)
      as_fatal (_("duplicate %s"), nios2_ps_insn_info_structs[i].pseudo_insn);

  /* Assembler option defaults.  */
  nios2_as_options.noat = false;
  nios2_as_options.nobreak = false;

  /* Initialize the alignment data.  */
  nios2_current_align_seg = now_seg;
  nios2_last_label = NULL;
  nios2_current_align = 0;
  nios2_min_align = 2;
}


/* Assembles a single line of Nios II assembly language.  */
void
md_assemble (char *op_str)
{
  char *argstr;
  char *op_strdup = NULL;
  unsigned long saved_pinfo = 0;
  nios2_insn_infoS thisinsn;
  nios2_insn_infoS *insn = &thisinsn;
  bool ps_error = false;

  /* Make sure we are aligned on an appropriate boundary.  */
  if (nios2_current_align < nios2_min_align)
    nios2_align (nios2_min_align, NULL, nios2_last_label);
  else if (nios2_current_align > nios2_min_align)
    nios2_current_align = nios2_min_align;
  nios2_last_label = NULL;

  /* We don't want to clobber to op_str
     because we want to be able to use it in messages.  */
  op_strdup = strdup (op_str);
  insn->insn_tokens[0] = strtok (op_strdup, " ");
  argstr = strtok (NULL, "");

  /* Assemble the opcode.  */
  insn->insn_nios2_opcode = nios2_opcode_lookup (insn->insn_tokens[0]);
  insn->insn_reloc = NULL;

  if (insn->insn_nios2_opcode != NULL)
    {
      nios2_ps_insn_infoS *ps_insn = NULL;

      /* Note if we've seen a 16-bit instruction.  */
      if (insn->insn_nios2_opcode->size == 2)
	nios2_min_align = 1;

      /* Set the opcode for the instruction.  */
      insn->insn_code = insn->insn_nios2_opcode->match;
      insn->constant_bits = 0;

      /* Parse the arguments pointed to by argstr.  */
      if (nios2_mode == NIOS2_MODE_ASSEMBLE)
	nios2_parse_args (insn, argstr, insn->insn_nios2_opcode->args,
			  (char **) &insn->insn_tokens[1]);
      else
	nios2_parse_args (insn, argstr, insn->insn_nios2_opcode->args_test,
			  (char **) &insn->insn_tokens[1]);

      /* We need to preserve the MOVIA macro as this is clobbered by
	 translate_pseudo_insn.  */
      if (insn->insn_nios2_opcode->pinfo == NIOS2_INSN_MACRO_MOVIA)
	saved_pinfo = NIOS2_INSN_MACRO_MOVIA;
      /* If the instruction is an pseudo-instruction, we want to replace it
	 with its real equivalent, and then continue.  */
      if ((insn->insn_nios2_opcode->pinfo & NIOS2_INSN_MACRO)
	  == NIOS2_INSN_MACRO)
	{
	  ps_insn = nios2_translate_pseudo_insn (insn);
	  if (!ps_insn)
	    ps_error = true;
	}

      /* If we found invalid pseudo-instruction syntax, the error's already
	 been diagnosed in nios2_translate_pseudo_insn, so skip
	 remaining processing.  */
      if (!ps_error)
	{
	  /* Assemble the parsed arguments into the instruction word.  */
	  nios2_assemble_args (insn);

	  /* Handle relaxation and other transformations.  */
	  if (nios2_as_options.relax != relax_none
	      && !nios2_as_options.noat
	      && insn->insn_nios2_opcode->pinfo & NIOS2_INSN_UBRANCH)
	    output_ubranch (insn);
	  else if (nios2_as_options.relax != relax_none
		   && !nios2_as_options.noat
		   && insn->insn_nios2_opcode->pinfo & NIOS2_INSN_CBRANCH)
	    output_cbranch (insn);
	  else if (nios2_as_options.relax == relax_all
		   && !nios2_as_options.noat
		   && insn->insn_nios2_opcode->pinfo & NIOS2_INSN_CALL
		   && insn->insn_reloc
		   && ((insn->insn_reloc->reloc_type
			== BFD_RELOC_NIOS2_CALL26)
		       || (insn->insn_reloc->reloc_type
			   == BFD_RELOC_NIOS2_CALL26_NOAT)))
	    output_call (insn);
	  else if (saved_pinfo == NIOS2_INSN_MACRO_MOVIA)
	    output_movia (insn);
	  else
	    output_insn (insn);
	  if (ps_insn)
	    nios2_cleanup_pseudo_insn (insn, ps_insn);
	}
    }
  else
    /* Unrecognised instruction - error.  */
    as_bad (_("unrecognised instruction %s"), insn->insn_tokens[0]);

  /* Don't leak memory.  */
  free (op_strdup);
}

/* Round up section size.  */
valueT
md_section_align (asection *seg ATTRIBUTE_UNUSED, valueT size)
{
  /* I think byte alignment is fine here.  */
  return size;
}

/* Implement TC_FORCE_RELOCATION.  */
int
nios2_force_relocation (fixS *fixp)
{
  if (fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY
      || fixp->fx_r_type == BFD_RELOC_NIOS2_ALIGN)
    return 1;

  return generic_force_reloc (fixp);
}

/* Implement tc_fix_adjustable.  */
int
nios2_fix_adjustable (fixS *fixp)
{
  if (fixp->fx_addsy == NULL)
    return 1;

#ifdef OBJ_ELF
  /* Prevent all adjustments to global symbols.  */
  if (OUTPUT_FLAVOR == bfd_target_elf_flavour
      && (S_IS_EXTERNAL (fixp->fx_addsy) || S_IS_WEAK (fixp->fx_addsy)))
    return 0;
#endif
  if (fixp->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return 0;

  /* Preserve relocations against symbols with function type.  */
  if (symbol_get_bfdsym (fixp->fx_addsy)->flags & BSF_FUNCTION)
    return 0;

  /* Don't allow symbols to be discarded on GOT related relocs.  */
  if (fixp->fx_r_type == BFD_RELOC_NIOS2_GOT16
      || fixp->fx_r_type == BFD_RELOC_NIOS2_CALL16
      || fixp->fx_r_type == BFD_RELOC_NIOS2_GOTOFF_LO
      || fixp->fx_r_type == BFD_RELOC_NIOS2_GOTOFF_HA
      || fixp->fx_r_type == BFD_RELOC_NIOS2_TLS_GD16
      || fixp->fx_r_type == BFD_RELOC_NIOS2_TLS_LDM16
      || fixp->fx_r_type == BFD_RELOC_NIOS2_TLS_LDO16
      || fixp->fx_r_type == BFD_RELOC_NIOS2_TLS_IE16
      || fixp->fx_r_type == BFD_RELOC_NIOS2_TLS_LE16
      || fixp->fx_r_type == BFD_RELOC_NIOS2_TLS_DTPMOD
      || fixp->fx_r_type == BFD_RELOC_NIOS2_TLS_DTPREL
      || fixp->fx_r_type == BFD_RELOC_NIOS2_TLS_TPREL
      || fixp->fx_r_type == BFD_RELOC_NIOS2_GOTOFF
      || fixp->fx_r_type == BFD_RELOC_NIOS2_GOT_LO
      || fixp->fx_r_type == BFD_RELOC_NIOS2_GOT_HA
      || fixp->fx_r_type == BFD_RELOC_NIOS2_CALL_LO
      || fixp->fx_r_type == BFD_RELOC_NIOS2_CALL_HA
      )
    return 0;

  return 1;
}

/* Implement tc_frob_symbol.  This is called in adjust_reloc_syms;
   it is used to remove *ABS* references from the symbol table.  */
int
nios2_frob_symbol (symbolS *symp)
{
  if ((OUTPUT_FLAVOR == bfd_target_elf_flavour
       && symp == section_symbol (absolute_section))
      || !S_IS_DEFINED (symp))
    return 1;
  else
    return 0;
}

/* The function tc_gen_reloc creates a relocation structure for the
   fixup fixp, and returns a pointer to it.  This structure is passed
   to bfd_install_relocation so that it can be written to the object
   file for linking.  */
arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc = XNEW (arelent);
  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);

  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->addend = fixp->fx_offset;  /* fixp->fx_addnumber; */

  if (fixp->fx_pcrel)
    {
      switch (fixp->fx_r_type)
	{
	case BFD_RELOC_16:
	  fixp->fx_r_type = BFD_RELOC_16_PCREL;
	  break;
	case BFD_RELOC_NIOS2_LO16:
	  fixp->fx_r_type = BFD_RELOC_NIOS2_PCREL_LO;
	  break;
	case BFD_RELOC_NIOS2_HIADJ16:
	  fixp->fx_r_type = BFD_RELOC_NIOS2_PCREL_HA;
	  break;
	default:
	  break;
	}
    }

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("can't represent relocation type %s"),
		    bfd_get_reloc_code_name (fixp->fx_r_type));

      /* Set howto to a garbage value so that we can keep going.  */
      reloc->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_32);
      gas_assert (reloc->howto != NULL);
    }
  return reloc;
}

long
md_pcrel_from (fixS *fixP ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Called just before the assembler exits.  */
void
md_end (void)
{
  /* FIXME - not yet implemented */
}

/* Under ELF we need to default _GLOBAL_OFFSET_TABLE.
   Otherwise we have no need to default values of symbols.  */
symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
#ifdef OBJ_ELF
  if (name[0] == '_' && name[1] == 'G'
      && strcmp (name, GLOBAL_OFFSET_TABLE_NAME) == 0)
    {
      if (!GOT_symbol)
	{
	  if (symbol_find (name))
	    as_bad ("GOT already in the symbol table");

	  GOT_symbol = symbol_new (name, undefined_section,
				   &zero_address_frag, 0);
	}

      return GOT_symbol;
    }
#endif

  return 0;
}

/* Implement tc_frob_label.  */
void
nios2_frob_label (symbolS *lab)
{
  /* Emit dwarf information.  */
  dwarf2_emit_label (lab);

  /* Update the label's address with the current output pointer.  */
  symbol_set_frag (lab, frag_now);
  S_SET_VALUE (lab, (valueT) frag_now_fix ());

  /* Record this label for future adjustment after we find out what
     kind of data it references, and the required alignment therewith.  */
  nios2_last_label = lab;
}

/* Implement md_cons_align.  */
void
nios2_cons_align (int size)
{
  int log_size = 0;
  const char *pfill = NULL;

  while ((size >>= 1) != 0)
    ++log_size;

  if (subseg_text_p (now_seg))
    pfill = (const char *) nop32;
  else
    pfill = NULL;

  if (nios2_auto_align_on)
    nios2_align (log_size, pfill, NULL);

  nios2_last_label = NULL;
}

/* Map 's' to SHF_NIOS2_GPREL.  */
/* This is from the Alpha code tc-alpha.c.  */
int
nios2_elf_section_letter (int letter, const char **ptr_msg)
{
  if (letter == 's')
    return SHF_NIOS2_GPREL;

  *ptr_msg = _("Bad .section directive: want a,s,w,x,M,S,G,T in string");
  return -1;
}

/* Map SHF_ALPHA_GPREL to SEC_SMALL_DATA.  */
/* This is from the Alpha code tc-alpha.c.  */
flagword
nios2_elf_section_flags (flagword flags, int attr, int type ATTRIBUTE_UNUSED)
{
  if (attr & SHF_NIOS2_GPREL)
    flags |= SEC_SMALL_DATA;
  return flags;
}

/* Implement TC_PARSE_CONS_EXPRESSION to handle %tls_ldo(...) and
   %gotoff(...).  */
bfd_reloc_code_real_type
nios2_cons (expressionS *exp, int size)
{
  bfd_reloc_code_real_type explicit_reloc = BFD_RELOC_NONE;
  const char *reloc_name = NULL;

  SKIP_WHITESPACE ();
  if (input_line_pointer[0] == '%')
    {
      if (startswith (input_line_pointer + 1, "tls_ldo"))
	{
	  reloc_name = "%tls_ldo";
	  if (size != 4)
	    as_bad (_("Illegal operands: %%tls_ldo in %d-byte data field"),
		    size);
	  else
	    {
	      input_line_pointer += 8;
	      explicit_reloc = BFD_RELOC_NIOS2_TLS_DTPREL;
	    }
	}
      else if (startswith (input_line_pointer + 1, "gotoff"))
	{
	  reloc_name = "%gotoff";
	  if (size != 4)
	    as_bad (_("Illegal operands: %%gotoff in %d-byte data field"),
		    size);
	  else
	    {
	      input_line_pointer += 7;
	      explicit_reloc = BFD_RELOC_NIOS2_GOTOFF;
	    }
	}

      if (explicit_reloc != BFD_RELOC_NONE)
	{
	  SKIP_WHITESPACE ();
	  if (input_line_pointer[0] != '(')
	    as_bad (_("Illegal operands: %s requires arguments in ()"),
		    reloc_name);
	  else
	    {
	      int c;
	      char *end = ++input_line_pointer;
	      int npar = 0;

	      for (c = *end; !is_end_of_line[c]; end++, c = *end)
		if (c == '(')
		  npar++;
		else if (c == ')')
		  {
		    if (!npar)
		      break;
		    npar--;
		  }

	      if (c != ')')
		as_bad (_("Illegal operands: %s requires arguments in ()"),
			reloc_name);
	      else
		{
		  *end = '\0';
		  expression (exp);
		  *end = c;
		  if (input_line_pointer != end)
		    as_bad (_("Illegal operands: %s requires arguments in ()"),
			    reloc_name);
		  else
		    {
		      input_line_pointer++;
		      SKIP_WHITESPACE ();
		      c = *input_line_pointer;
		      if (! is_end_of_line[c] && c != ',')
			as_bad (_("Illegal operands: garbage after %s()"),
				reloc_name);
		    }
		}
	    }
	}
    }
  if (explicit_reloc == BFD_RELOC_NONE)
    expression (exp);
  return explicit_reloc;
}

/* Implement HANDLE_ALIGN.  */
void
nios2_handle_align (fragS *fragp)
{
  /* If we are expecting to relax in the linker, then we must output a
     relocation to tell the linker we are aligning code.  */
  if (nios2_as_options.relax == relax_all
      && (fragp->fr_type == rs_align || fragp->fr_type == rs_align_code)
      && fragp->fr_address + fragp->fr_fix > 0
      && fragp->fr_offset > 1
      && now_seg != bss_section)
    fix_new (fragp, fragp->fr_fix, 0, &abs_symbol, fragp->fr_offset, 0,
	     BFD_RELOC_NIOS2_ALIGN);
}

/* Implement tc_regname_to_dw2regnum, to convert REGNAME to a DWARF-2
   register number.  */
int
nios2_regname_to_dw2regnum (char *regname)
{
  struct nios2_reg *r = nios2_reg_lookup (regname);
  if (r == NULL)
    return -1;
  return r->index;
}

/* Implement tc_cfi_frame_initial_instructions, to initialize the DWARF-2
   unwind information for this procedure.  */
void
nios2_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa (27, 0);
}

#ifdef OBJ_ELF
/* Some special processing for a Nios II ELF file.  */

void
nios2_elf_final_processing (void)
{
  elf_elfheader (stdoutput)->e_flags = nios2_architecture;
}
#endif
