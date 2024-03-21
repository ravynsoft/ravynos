/* tc-rl78.c -- Assembler for the Renesas RL78
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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
#include "safe-ctype.h"
#include "dwarf2dbg.h"
#include "elf/common.h"
#include "elf/rl78.h"
#include "rl78-defs.h"
#include "filenames.h"
#include "listing.h"
#include "sb.h"
#include "macro.h"

const char comment_chars[]        = ";";
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
const char line_comment_chars[]   = "#";
/* Use something that isn't going to be needed by any expressions or
   other syntax.  */
const char line_separator_chars[] = "@";

const char EXP_CHARS[]            = "eE";
const char FLT_CHARS[]            = "dD";

/* ELF flags to set in the output file header.  */
static int elf_flags = 0;

/*------------------------------------------------------------------*/

char * rl78_lex_start;
char * rl78_lex_end;

typedef struct rl78_bytesT
{
  char prefix[1];
  int n_prefix;
  char base[4];
  int n_base;
  char ops[8];
  int n_ops;
  struct
  {
    expressionS  exp;
    char         offset;
    char         nbits;
    char         type; /* RL78REL_*.  */
    int          reloc;
    fixS *       fixP;
  } fixups[2];
  int n_fixups;
  struct
  {
    char type;
    char field_pos;
    char val_ofs;
  } relax[2];
  int n_relax;
  int link_relax;
  fixS *link_relax_fixP;
  char times_grown;
  char times_shrank;
} rl78_bytesT;

static rl78_bytesT rl78_bytes;

void
rl78_relax (int type, int pos)
{
  rl78_bytes.relax[rl78_bytes.n_relax].type = type;
  rl78_bytes.relax[rl78_bytes.n_relax].field_pos = pos;
  rl78_bytes.relax[rl78_bytes.n_relax].val_ofs = rl78_bytes.n_base + rl78_bytes.n_ops;
  rl78_bytes.n_relax ++;
}

void
rl78_linkrelax_addr16 (void)
{
  rl78_bytes.link_relax |= RL78_RELAXA_ADDR16;
}

void
rl78_linkrelax_branch (void)
{
  rl78_relax (RL78_RELAX_BRANCH, 0);
  rl78_bytes.link_relax |= RL78_RELAXA_BRA;
}

static void
rl78_fixup (expressionS exp, int offsetbits, int nbits, int type)
{
  rl78_bytes.fixups[rl78_bytes.n_fixups].exp = exp;
  rl78_bytes.fixups[rl78_bytes.n_fixups].offset = offsetbits;
  rl78_bytes.fixups[rl78_bytes.n_fixups].nbits = nbits;
  rl78_bytes.fixups[rl78_bytes.n_fixups].type = type;
  rl78_bytes.fixups[rl78_bytes.n_fixups].reloc = exp.X_md;
  rl78_bytes.n_fixups ++;
}

#define rl78_field_fixup(exp, offset, nbits, type)	\
  rl78_fixup (exp, offset + 8 * rl78_bytes.n_prefix), nbits, type)

#define rl78_op_fixup(exp, offset, nbits, type)		\
  rl78_fixup (exp, offset + 8 * (rl78_bytes.n_prefix + rl78_bytes.n_base), nbits, type)

void
rl78_prefix (int p)
{
  rl78_bytes.prefix[0] = p;
  rl78_bytes.n_prefix = 1;
}

int
rl78_has_prefix (void)
{
  return rl78_bytes.n_prefix;
}

void
rl78_base1 (int b1)
{
  rl78_bytes.base[0] = b1;
  rl78_bytes.n_base = 1;
}

void
rl78_base2 (int b1, int b2)
{
  rl78_bytes.base[0] = b1;
  rl78_bytes.base[1] = b2;
  rl78_bytes.n_base = 2;
}

void
rl78_base3 (int b1, int b2, int b3)
{
  rl78_bytes.base[0] = b1;
  rl78_bytes.base[1] = b2;
  rl78_bytes.base[2] = b3;
  rl78_bytes.n_base = 3;
}

void
rl78_base4 (int b1, int b2, int b3, int b4)
{
  rl78_bytes.base[0] = b1;
  rl78_bytes.base[1] = b2;
  rl78_bytes.base[2] = b3;
  rl78_bytes.base[3] = b4;
  rl78_bytes.n_base = 4;
}

#define F_PRECISION 2

void
rl78_op (expressionS exp, int nbytes, int type)
{
  int v = 0;

  if ((exp.X_op == O_constant || exp.X_op == O_big)
      && type != RL78REL_PCREL)
    {
      if (exp.X_op == O_big && exp.X_add_number <= 0)
	{
	  LITTLENUM_TYPE w[2];
	  char * ip = rl78_bytes.ops + rl78_bytes.n_ops;

	  gen_to_words (w, F_PRECISION, 8);
	  ip[3] = w[0] >> 8;
	  ip[2] = w[0];
	  ip[1] = w[1] >> 8;
	  ip[0] = w[1];
	  rl78_bytes.n_ops += 4;
	}
      else
	{
	  v = exp.X_add_number;
	  while (nbytes)
	    {
	      rl78_bytes.ops[rl78_bytes.n_ops++] =v & 0xff;
	      v >>= 8;
	      nbytes --;
	    }
	}
    }
  else
    {
      if (nbytes > 2
	  && exp.X_md == BFD_RELOC_RL78_CODE)
	exp.X_md = 0;

      if (nbytes == 1
	  && (exp.X_md == BFD_RELOC_RL78_LO16
	      || exp.X_md == BFD_RELOC_RL78_HI16))
	as_bad (_("16-bit relocation used in 8-bit operand"));

      if (nbytes == 2
	  && exp.X_md == BFD_RELOC_RL78_HI8)
	as_bad (_("8-bit relocation used in 16-bit operand"));

      rl78_op_fixup (exp, rl78_bytes.n_ops * 8, nbytes * 8, type);
      memset (rl78_bytes.ops + rl78_bytes.n_ops, 0, nbytes);
      rl78_bytes.n_ops += nbytes;
    }
}

/* This gets complicated when the field spans bytes, because fields
   are numbered from the MSB of the first byte as zero, and bits are
   stored LSB towards the LSB of the byte.  Thus, a simple four-bit
   insertion of 12 at position 4 of 0x00 yields: 0x0b.  A three-bit
   insertion of b'MXL at position 7 is like this:

     - - - -  - - - -   - - - -  - - - -
                    M   X L               */

void
rl78_field (int val, int pos, int sz)
{
  int valm;
  int bytep, bitp;

  if (sz > 0)
    {
      if (val < 0 || val >= (1 << sz))
	as_bad (_("Value %d doesn't fit in unsigned %d-bit field"), val, sz);
    }
  else
    {
      sz = - sz;
      if (val < -(1 << (sz - 1)) || val >= (1 << (sz - 1)))
	as_bad (_("Value %d doesn't fit in signed %d-bit field"), val, sz);
    }

  /* This code points at 'M' in the above example.  */
  bytep = pos / 8;
  bitp = pos % 8;

  while (bitp + sz > 8)
    {
      int ssz = 8 - bitp;
      int svalm;

      svalm = val >> (sz - ssz);
      svalm = svalm & ((1 << ssz) - 1);
      svalm = svalm << (8 - bitp - ssz);
      gas_assert (bytep < rl78_bytes.n_base);
      rl78_bytes.base[bytep] |= svalm;

      bitp = 0;
      sz -= ssz;
      bytep ++;
    }
  valm = val & ((1 << sz) - 1);
  valm = valm << (8 - bitp - sz);
  gas_assert (bytep < rl78_bytes.n_base);
  rl78_bytes.base[bytep] |= valm;
}

/*------------------------------------------------------------------*/

enum options
{
  OPTION_RELAX = OPTION_MD_BASE,
  OPTION_NORELAX,
  OPTION_G10,
  OPTION_G13,
  OPTION_G14,
  OPTION_32BIT_DOUBLES,
  OPTION_64BIT_DOUBLES,
};

#define RL78_SHORTOPTS ""
const char * md_shortopts = RL78_SHORTOPTS;

/* Assembler options.  */
struct option md_longopts[] =
{
  {"relax", no_argument, NULL, OPTION_RELAX},
  {"norelax", no_argument, NULL, OPTION_NORELAX},
  {"mg10", no_argument, NULL, OPTION_G10},
  {"mg13", no_argument, NULL, OPTION_G13},
  {"mg14", no_argument, NULL, OPTION_G14},
  {"mrl78", no_argument, NULL, OPTION_G14},
  {"m32bit-doubles", no_argument, NULL, OPTION_32BIT_DOUBLES},
  {"m64bit-doubles", no_argument, NULL, OPTION_64BIT_DOUBLES},
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

int
md_parse_option (int c, const char * arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    case OPTION_RELAX:
      linkrelax = 1;
      return 1;
    case OPTION_NORELAX:
      linkrelax = 0;
      return 1;

    case OPTION_G10:
      elf_flags &= ~ E_FLAG_RL78_CPU_MASK;
      elf_flags |= E_FLAG_RL78_G10;
      return 1;

    case OPTION_G13:
      elf_flags &= ~ E_FLAG_RL78_CPU_MASK;
      elf_flags |= E_FLAG_RL78_G13;
      return 1;

    case OPTION_G14:
      elf_flags &= ~ E_FLAG_RL78_CPU_MASK;
      elf_flags |= E_FLAG_RL78_G14;
      return 1;

    case OPTION_32BIT_DOUBLES:
      elf_flags &= ~ E_FLAG_RL78_64BIT_DOUBLES;
      return 1;

    case OPTION_64BIT_DOUBLES:
      elf_flags |= E_FLAG_RL78_64BIT_DOUBLES;
      return 1;
    }
  return 0;
}

int
rl78_isa_g10 (void)
{
  return (elf_flags & E_FLAG_RL78_CPU_MASK) == E_FLAG_RL78_G10;
}

int
rl78_isa_g13 (void)
{
  return (elf_flags & E_FLAG_RL78_CPU_MASK) == E_FLAG_RL78_G13;
}

int
rl78_isa_g14 (void)
{
  return (elf_flags & E_FLAG_RL78_CPU_MASK) == E_FLAG_RL78_G14;
}

void
md_show_usage (FILE * stream)
{
  fprintf (stream, _(" RL78 specific command line options:\n"));
  fprintf (stream, _("  --mrelax          Enable link time relaxation\n"));
  fprintf (stream, _("  --mg10            Enable support for G10 variant\n"));
  fprintf (stream, _("  --mg13            Selects the G13 core.\n"));
  fprintf (stream, _("  --mg14            Selects the G14 core [default]\n"));
  fprintf (stream, _("  --mrl78           Alias for --mg14\n"));
  fprintf (stream, _("  --m32bit-doubles  [default]\n"));
  fprintf (stream, _("  --m64bit-doubles  Source code uses 64-bit doubles\n"));
}

static void
s_bss (int ignore ATTRIBUTE_UNUSED)
{
  int temp;

  temp = get_absolute_expression ();
  subseg_set (bss_section, (subsegT) temp);
  demand_empty_rest_of_line ();
}

static void
rl78_float_cons (int ignore ATTRIBUTE_UNUSED)
{
  if (elf_flags & E_FLAG_RL78_64BIT_DOUBLES)
    return float_cons ('d');
  return float_cons ('f');
}

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  /* Our "standard" pseudos.  */
  { "double", rl78_float_cons,	'd' },
  { "bss",    s_bss, 		0 },
  { "3byte",  cons,		3 },
  { "int",    cons,		4 },
  { "word",   cons,		4 },

  /* End of list marker.  */
  { NULL, 	NULL, 		0 }
};

static symbolS * rl78_abs_sym = NULL;

void
md_begin (void)
{
  rl78_abs_sym = symbol_make ("__rl78_abs__");
}

void
rl78_md_end (void)
{
}

/* Set the ELF specific flags.  */
void
rl78_elf_final_processing (void)
{
  elf_elfheader (stdoutput)->e_flags |= elf_flags;
}

/* Write a value out to the object file, using the appropriate endianness.  */
void
md_number_to_chars (char * buf, valueT val, int n)
{
  number_to_chars_littleendian (buf, val, n);
}

static void
require_end_of_expr (const char *fname)
{
  while (* input_line_pointer == ' '
	 || * input_line_pointer == '\t')
    input_line_pointer ++;

  if (! * input_line_pointer
      || strchr ("\n\r,", * input_line_pointer)
      || strchr (comment_chars, * input_line_pointer)
      || strchr (line_comment_chars, * input_line_pointer)
      || strchr (line_separator_chars, * input_line_pointer))
    return;

  as_bad (_("%%%s() must be outermost term in expression"), fname);
}

static struct
{
  const char * fname;
  int    reloc;
}
reloc_functions[] =
{
  { "code", BFD_RELOC_RL78_CODE },
  { "lo16", BFD_RELOC_RL78_LO16 },
  { "hi16", BFD_RELOC_RL78_HI16 },
  { "hi8",  BFD_RELOC_RL78_HI8 },
  { 0, 0 }
};

void
md_operand (expressionS * exp ATTRIBUTE_UNUSED)
{
  int reloc = 0;
  int i;

  for (i = 0; reloc_functions[i].fname; i++)
    {
      int flen = strlen (reloc_functions[i].fname);

      if (input_line_pointer[0] == '%'
	  && strncasecmp (input_line_pointer + 1, reloc_functions[i].fname, flen) == 0
	  && input_line_pointer[flen + 1] == '(')
	{
	  reloc = reloc_functions[i].reloc;
	  input_line_pointer += flen + 2;
	  break;
	}
    }
  if (reloc == 0)
    return;

  expression (exp);
  if (* input_line_pointer == ')')
    input_line_pointer ++;

  exp->X_md = reloc;

  require_end_of_expr (reloc_functions[i].fname);
}

void
rl78_frag_init (fragS * fragP)
{
  if (rl78_bytes.n_relax || rl78_bytes.link_relax)
    {
      fragP->tc_frag_data = XNEW (rl78_bytesT);
      memcpy (fragP->tc_frag_data, & rl78_bytes, sizeof (rl78_bytesT));
    }
  else
    fragP->tc_frag_data = 0;
}

/* When relaxing, we need to output a reloc for any .align directive
   so that we can retain this alignment as we adjust opcode sizes.  */
void
rl78_handle_align (fragS * frag)
{
  if (linkrelax
      && (frag->fr_type == rs_align
	  || frag->fr_type == rs_align_code)
      && frag->fr_address + frag->fr_fix > 0
      && frag->fr_offset > 0
      && now_seg != bss_section)
    {
      fix_new (frag, frag->fr_fix, 0,
	       &abs_symbol, RL78_RELAXA_ALIGN + frag->fr_offset,
	       0, BFD_RELOC_RL78_RELAX);
      /* For the purposes of relaxation, this relocation is attached
	 to the byte *after* the alignment - i.e. the byte that must
	 remain aligned.  */
      fix_new (frag->fr_next, 0, 0,
	       &abs_symbol, RL78_RELAXA_ELIGN + frag->fr_offset,
	       0, BFD_RELOC_RL78_RELAX);
    }
}

const char *
md_atof (int type, char * litP, int * sizeP)
{
  return ieee_md_atof (type, litP, sizeP, target_big_endian);
}

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED)
{
  return NULL;
}

#define APPEND(B, N_B)				       \
  if (rl78_bytes.N_B)				       \
    {						       \
      memcpy (bytes + idx, rl78_bytes.B, rl78_bytes.N_B);  \
      idx += rl78_bytes.N_B;			       \
    }


void
md_assemble (char * str)
{
  char * bytes;
  fragS * frag_then = frag_now;
  int idx = 0;
  int i;
  int rel;
  expressionS  *exp;

  /*printf("\033[32mASM: %s\033[0m\n", str);*/

  dwarf2_emit_insn (0);

  memset (& rl78_bytes, 0, sizeof (rl78_bytes));

  rl78_lex_init (str, str + strlen (str));

  rl78_parse ();

  /* This simplifies the relaxation code.  */
  if (rl78_bytes.n_relax || rl78_bytes.link_relax)
    {
      int olen = rl78_bytes.n_prefix + rl78_bytes.n_base + rl78_bytes.n_ops;
      /* We do it this way because we want the frag to have the
	 rl78_bytes in it, which we initialize above.  The extra bytes
	 are for relaxing.  */
      bytes = frag_more (olen + 3);
      frag_then = frag_now;
      frag_variant (rs_machine_dependent,
		    olen /* max_chars */,
		    0 /* var */,
		    olen /* subtype */,
		    0 /* symbol */,
		    0 /* offset */,
		    0 /* opcode */);
      frag_then->fr_opcode = bytes;
      frag_then->fr_fix = olen + (bytes - frag_then->fr_literal);
      frag_then->fr_subtype = olen;
      frag_then->fr_var = 0;
    }
  else
    {
      bytes = frag_more (rl78_bytes.n_prefix + rl78_bytes.n_base + rl78_bytes.n_ops);
      frag_then = frag_now;
    }

  APPEND (prefix, n_prefix);
  APPEND (base, n_base);
  APPEND (ops, n_ops);

  if (rl78_bytes.link_relax)
    {
      fixS * f;

      f = fix_new (frag_then,
		   (char *) bytes - frag_then->fr_literal,
		   0,
		   abs_section_sym,
		   rl78_bytes.link_relax | rl78_bytes.n_fixups,
		   0,
		   BFD_RELOC_RL78_RELAX);
      frag_then->tc_frag_data->link_relax_fixP = f;
    }

  for (i = 0; i < rl78_bytes.n_fixups; i ++)
    {
      /* index: [nbytes][type] */
      static int reloc_map[5][4] =
	{
	  { 0,            0 },
	  { BFD_RELOC_8,  BFD_RELOC_8_PCREL },
	  { BFD_RELOC_16, BFD_RELOC_16_PCREL },
	  { BFD_RELOC_24, BFD_RELOC_24_PCREL },
	  { BFD_RELOC_32, BFD_RELOC_32_PCREL },
	};
      fixS * f;

      idx = rl78_bytes.fixups[i].offset / 8;
      rel = reloc_map [rl78_bytes.fixups[i].nbits / 8][(int) rl78_bytes.fixups[i].type];

      if (rl78_bytes.fixups[i].reloc)
	rel = rl78_bytes.fixups[i].reloc;

      if (frag_then->tc_frag_data)
	exp = & frag_then->tc_frag_data->fixups[i].exp;
      else
	exp = & rl78_bytes.fixups[i].exp;

      f = fix_new_exp (frag_then,
		       (char *) bytes + idx - frag_then->fr_literal,
		       rl78_bytes.fixups[i].nbits / 8,
		       exp,
		       rl78_bytes.fixups[i].type == RL78REL_PCREL ? 1 : 0,
		       rel);
      if (frag_then->tc_frag_data)
	frag_then->tc_frag_data->fixups[i].fixP = f;
    }
}

void
rl78_cons_fix_new (fragS *	frag,
		 int		where,
		 int		size,
		 expressionS *  exp)
{
  bfd_reloc_code_real_type type;
  fixS *fixP;

  switch (size)
    {
    case 1:
      type = BFD_RELOC_8;
      break;
    case 2:
      type = BFD_RELOC_16;
      break;
    case 3:
      type = BFD_RELOC_24;
      break;
    case 4:
      type = BFD_RELOC_32;
      break;
    default:
      as_bad (_("unsupported constant size %d\n"), size);
      return;
    }

  switch (exp->X_md)
    {
    case BFD_RELOC_RL78_CODE:
      if (size == 2)
	type = exp->X_md;
      break;
    case BFD_RELOC_RL78_LO16:
    case BFD_RELOC_RL78_HI16:
      if (size != 2)
	{
	  /* Fixups to assembler generated expressions do not use %hi or %lo.  */
	  if (frag->fr_file)
	    as_bad (_("%%hi16/%%lo16 only applies to .short or .hword"));
	}
      else
	type = exp->X_md;
      break;
    case BFD_RELOC_RL78_HI8:
      if (size != 1)
	{
	  /* Fixups to assembler generated expressions do not use %hi or %lo.  */
	  if (frag->fr_file)
	    as_bad (_("%%hi8 only applies to .byte"));
	}
      else
	type = exp->X_md;
      break;
    default:
      break;
    }

  if (exp->X_op == O_subtract && exp->X_op_symbol)
    {
      if (size != 4 && size != 2 && size != 1)
	as_bad (_("difference of two symbols only supported with .long, .short, or .byte"));
      else
	type = BFD_RELOC_RL78_DIFF;
    }

  fixP = fix_new_exp (frag, where, (int) size, exp, 0, type);
  switch (exp->X_md)
    {
      /* These are intended to have values larger than the container,
	 since the backend puts only the portion we need in it.
	 However, we don't have a backend-specific reloc for them as
	 they're handled with complex relocations.  */
    case BFD_RELOC_RL78_LO16:
    case BFD_RELOC_RL78_HI16:
    case BFD_RELOC_RL78_HI8:
      fixP->fx_no_overflow = 1;
      break;
    default:
      break;
    }
}


/*----------------------------------------------------------------------*/
/* To recap: we estimate everything based on md_estimate_size, then
   adjust based on rl78_relax_frag.  When it all settles, we call
   md_convert frag to update the bytes.  The relaxation types and
   relocations are in fragP->tc_frag_data, which is a copy of that
   rl78_bytes.

   Our scheme is as follows: fr_fix has the size of the smallest
   opcode (like BRA.S).  We store the number of total bytes we need in
   fr_subtype.  When we're done relaxing, we use fr_subtype and the
   existing opcode bytes to figure out what actual opcode we need to
   put in there.  If the fixup isn't resolvable now, we use the
   maximal size.  */

#define TRACE_RELAX 0
#define tprintf if (TRACE_RELAX) printf


typedef enum
{
  OT_other,
  OT_bt,
  OT_bt_sfr,
  OT_bt_es,
  OT_bc,
  OT_bh,
  OT_sk,
  OT_call,
  OT_br,
} op_type_T;

/* We're looking for these types of relaxations:

   BT		00110001 sbit0cc1 addr----	(cc is 10 (BF) or 01 (BT))
   B~T		00110001 sbit0cc1 00000011 11101110 pcrel16- -------- (BR $!pcrel20)

   BT sfr	00110001 sbit0cc0 sfr----- addr----
   BT ES:	00010001 00101110 sbit0cc1 addr----

   BC		110111cc addr----
   B~C		110111cc 00000011 11101110 pcrel16- -------- (BR $!pcrel20)

   BH		01100001 110c0011 00000011 11101110 pcrel16- -------- (BR $!pcrel20)
   B~H		01100001 110c0011 00000011 11101110 pcrel16- -------- (BR $!pcrel20)
*/

/* Given the opcode bytes at OP, figure out which opcode it is and
   return the type of opcode.  We use this to re-encode the opcode as
   a different size later.  */

static op_type_T
rl78_opcode_type (char * ops)
{
  unsigned char *op = (unsigned char *)ops;

  if (op[0] == 0x31
      && ((op[1] & 0x0f) == 0x05
	  || (op[1] & 0x0f) == 0x03))
    return OT_bt;

  if (op[0] == 0x31
      && ((op[1] & 0x0f) == 0x04
	  || (op[1] & 0x0f) == 0x02))
    return OT_bt_sfr;

  if (op[0] == 0x11
      && op[1] == 0x31
      && ((op[2] & 0x0f) == 0x05
	  || (op[2] & 0x0f) == 0x03))
    return OT_bt_es;

  if ((op[0] & 0xfc) == 0xdc)
    return OT_bc;

  if (op[0] == 0x61
      && (op[1] & 0xef) == 0xc3)
    return OT_bh;

  if (op[0] == 0x61
      && (op[1] & 0xcf) == 0xc8)
    return OT_sk;

  if (op[0] == 0x61
      && (op[1] & 0xef) == 0xe3)
    return OT_sk;

  if (op[0] == 0xfc)
    return OT_call;

  if ((op[0] & 0xec) == 0xec)
    return OT_br;

  return OT_other;
}

/* Returns zero if *addrP has the target address.  Else returns nonzero
   if we cannot compute the target address yet.  */

static int
rl78_frag_fix_value (fragS *    fragP,
		     segT       segment,
		     int        which,
		     addressT * addrP,
		     int        need_diff,
		     addressT * sym_addr)
{
  addressT addr = 0;
  rl78_bytesT * b = fragP->tc_frag_data;
  expressionS * exp = & b->fixups[which].exp;

  if (need_diff && exp->X_op != O_subtract)
    return 1;

  if (exp->X_add_symbol)
    {
      if (S_FORCE_RELOC (exp->X_add_symbol, 1))
	return 1;
      if (S_GET_SEGMENT (exp->X_add_symbol) != segment)
	return 1;
      addr += S_GET_VALUE (exp->X_add_symbol);
    }

  if (exp->X_op_symbol)
    {
      if (exp->X_op != O_subtract)
	return 1;
      if (S_FORCE_RELOC (exp->X_op_symbol, 1))
	return 1;
      if (S_GET_SEGMENT (exp->X_op_symbol) != segment)
	return 1;
      addr -= S_GET_VALUE (exp->X_op_symbol);
    }
  if (sym_addr)
    * sym_addr = addr;
  addr += exp->X_add_number;
  * addrP = addr;
  return 0;
}

/* Estimate how big the opcode is after this relax pass.  The return
   value is the difference between fr_fix and the actual size.  We
   compute the total size in rl78_relax_frag and store it in fr_subtype,
   so we only need to subtract fx_fix and return it.  */

int
md_estimate_size_before_relax (fragS * fragP ATTRIBUTE_UNUSED, segT segment ATTRIBUTE_UNUSED)
{
  int opfixsize;
  int delta;

  /* This is the size of the opcode that's accounted for in fr_fix.  */
  opfixsize = fragP->fr_fix - (fragP->fr_opcode - fragP->fr_literal);
  /* This is the size of the opcode that isn't.  */
  delta = (fragP->fr_subtype - opfixsize);

  tprintf (" -> opfixsize %d delta %d\n", opfixsize, delta);
  return delta;
}

/* Given the new addresses for this relax pass, figure out how big
   each opcode must be.  We store the total number of bytes needed in
   fr_subtype.  The return value is the difference between the size
   after the last pass and the size after this pass, so we use the old
   fr_subtype to calculate the difference.  */

int
rl78_relax_frag (segT segment ATTRIBUTE_UNUSED, fragS * fragP, long stretch)
{
  addressT addr0, sym_addr;
  addressT mypc;
  int disp;
  int oldsize = fragP->fr_subtype;
  int newsize = oldsize;
  op_type_T optype;
  int ri;

  mypc = fragP->fr_address + (fragP->fr_opcode - fragP->fr_literal);

  /* If we ever get more than one reloc per opcode, this is the one
     we're relaxing.  */
  ri = 0;

  optype = rl78_opcode_type (fragP->fr_opcode);
  /* Try to get the target address.  */
  if (rl78_frag_fix_value (fragP, segment, ri, & addr0,
			   fragP->tc_frag_data->relax[ri].type != RL78_RELAX_BRANCH,
			   & sym_addr))
    {
      /* If we don't expect the linker to do relaxing, don't emit
	 expanded opcodes that only the linker will relax.  */
      if (!linkrelax)
	return newsize - oldsize;

      /* If we don't, we must use the maximum size for the linker.  */
      switch (fragP->tc_frag_data->relax[ri].type)
	{
	case RL78_RELAX_BRANCH:
	  switch (optype)
	    {
	    case OT_bt:
	      newsize = 6;
	      break;
	    case OT_bt_sfr:
	    case OT_bt_es:
	      newsize = 7;
	      break;
	    case OT_bc:
	      newsize = 5;
	      break;
	    case OT_bh:
	      newsize = 6;
	      break;
	    case OT_sk:
	      newsize = 2;
	      break;
	    default:
	      newsize = oldsize;
	      break;
	    }
	  break;

	}
      fragP->fr_subtype = newsize;
      tprintf (" -> new %d old %d delta %d (external)\n", newsize, oldsize, newsize-oldsize);
      return newsize - oldsize;
    }

  if (sym_addr > mypc)
    addr0 += stretch;

  switch (fragP->tc_frag_data->relax[ri].type)
    {
    case  RL78_RELAX_BRANCH:
      disp = (int) addr0 - (int) mypc;

      switch (optype)
	{
	case OT_bt:
	  if (disp >= -128 && (disp - (oldsize-2)) <= 127)
	    newsize = 3;
	  else
	    newsize = 6;
	  break;
	case OT_bt_sfr:
	case OT_bt_es:
	  if (disp >= -128 && (disp - (oldsize-3)) <= 127)
	    newsize = 4;
	  else
	    newsize = 7;
	  break;
	case OT_bc:
	  if (disp >= -128 && (disp - (oldsize-1)) <= 127)
	    newsize = 2;
	  else
	    newsize = 5;
	  break;
	case OT_bh:
	  if (disp >= -128 && (disp - (oldsize-2)) <= 127)
	    newsize = 3;
	  else
	    newsize = 6;
	  break;
	case OT_sk:
	  newsize = 2;
	  break;
	default:
	  newsize = oldsize;
	  break;
	}
      break;
    }

  /* This prevents infinite loops in align-heavy sources.  */
  if (newsize < oldsize)
    {
      if (fragP->tc_frag_data->times_shrank > 10
         && fragP->tc_frag_data->times_grown > 10)
       newsize = oldsize;
      if (fragP->tc_frag_data->times_shrank < 20)
       fragP->tc_frag_data->times_shrank ++;
    }
  else if (newsize > oldsize)
    {
      if (fragP->tc_frag_data->times_grown < 20)
       fragP->tc_frag_data->times_grown ++;
    }

  fragP->fr_subtype = newsize;
  tprintf (" -> new %d old %d delta %d\n", newsize, oldsize, newsize-oldsize);
  return newsize - oldsize;
}

/* This lets us test for the opcode type and the desired size in a
   switch statement.  */
#define OPCODE(type,size) ((type) * 16 + (size))

/* Given the opcode stored in fr_opcode and the number of bytes we
   think we need, encode a new opcode.  We stored a pointer to the
   fixup for this opcode in the tc_frag_data structure.  If we can do
   the fixup here, we change the relocation type to "none" (we test
   for that in tc_gen_reloc) else we change it to the right type for
   the new (biggest) opcode.  */

void
md_convert_frag (bfd *   abfd ATTRIBUTE_UNUSED,
		 segT    segment ATTRIBUTE_UNUSED,
		 fragS * fragP ATTRIBUTE_UNUSED)
{
  rl78_bytesT * rl78b = fragP->tc_frag_data;
  addressT addr0, mypc;
  int disp;
  int reloc_type, reloc_adjust;
  char * op = fragP->fr_opcode;
  int keep_reloc = 0;
  int ri;
  int fi = (rl78b->n_fixups > 1) ? 1 : 0;
  fixS * fix = rl78b->fixups[fi].fixP;

  /* If we ever get more than one reloc per opcode, this is the one
     we're relaxing.  */
  ri = 0;

  /* We used a new frag for this opcode, so the opcode address should
     be the frag address.  */
  mypc = fragP->fr_address + (fragP->fr_opcode - fragP->fr_literal);
  tprintf ("\033[32mmypc: 0x%x\033[0m\n", (int)mypc);

  /* Try to get the target address.  If we fail here, we just use the
     largest format.  */
  if (rl78_frag_fix_value (fragP, segment, 0, & addr0,
			   fragP->tc_frag_data->relax[ri].type != RL78_RELAX_BRANCH, 0))
    {
      /* We don't know the target address.  */
      keep_reloc = 1;
      addr0 = 0;
      disp = 0;
      tprintf ("unknown addr ? - %x = ?\n", (int)mypc);
    }
  else
    {
      /* We know the target address, and it's in addr0.  */
      disp = (int) addr0 - (int) mypc;
      tprintf ("known addr %x - %x = %d\n", (int)addr0, (int)mypc, disp);
    }

  if (linkrelax)
    keep_reloc = 1;

  reloc_type = BFD_RELOC_NONE;
  reloc_adjust = 0;

  switch (fragP->tc_frag_data->relax[ri].type)
    {
    case RL78_RELAX_BRANCH:
      switch (OPCODE (rl78_opcode_type (fragP->fr_opcode), fragP->fr_subtype))
	{

	case OPCODE (OT_bt, 3): /* BT A,$ - no change.  */
	  disp -= 3;
	  op[2] = disp;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  break;

	case OPCODE (OT_bt, 6): /* BT A,$ - long version.  */
	  disp -= 3;
	  op[1] ^= 0x06; /* toggle conditional.  */
	  op[2] = 3; /* displacement over long branch.  */
	  disp -= 3;
	  op[3] = 0xEE; /* BR $!addr20 */
	  op[4] = disp & 0xff;
	  op[5] = disp >> 8;
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;

	case OPCODE (OT_bt_sfr, 4): /* BT PSW,$ - no change.  */
	  disp -= 4;
	  op[3] = disp;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  break;

	case OPCODE (OT_bt_sfr, 7): /* BT PSW,$ - long version.  */
	  disp -= 4;
	  op[1] ^= 0x06; /* toggle conditional.  */
	  op[3] = 3; /* displacement over long branch.  */
	  disp -= 3;
	  op[4] = 0xEE; /* BR $!addr20 */
	  op[5] = disp & 0xff;
	  op[6] = disp >> 8;
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;

	case OPCODE (OT_bt_es, 4): /* BT ES:[HL],$ - no change.  */
	  disp -= 4;
	  op[3] = disp;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  break;

	case OPCODE (OT_bt_es, 7): /* BT PSW,$ - long version.  */
	  disp -= 4;
	  op[2] ^= 0x06; /* toggle conditional.  */
	  op[3] = 3; /* displacement over long branch.  */
	  disp -= 3;
	  op[4] = 0xEE; /* BR $!addr20 */
	  op[5] = disp & 0xff;
	  op[6] = disp >> 8;
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;

	case OPCODE (OT_bc, 2): /* BC $ - no change.  */
	  disp -= 2;
	  op[1] = disp;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  break;

	case OPCODE (OT_bc, 5): /* BC $ - long version.  */
	  disp -= 2;
	  op[0] ^= 0x02; /* toggle conditional.  */
	  op[1] = 3;
	  disp -= 3;
	  op[2] = 0xEE; /* BR $!addr20 */
	  op[3] = disp & 0xff;
	  op[4] = disp >> 8;
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;

	case OPCODE (OT_bh, 3): /* BH $ - no change.  */
	  disp -= 3;
	  op[2] = disp;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  break;

	case OPCODE (OT_bh, 6): /* BC $ - long version.  */
	  disp -= 3;
	  op[1] ^= 0x10; /* toggle conditional.  */
	  op[2] = 3;
	  disp -= 3;
	  op[3] = 0xEE; /* BR $!addr20 */
	  op[4] = disp & 0xff;
	  op[5] = disp >> 8;
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;

	case OPCODE (OT_sk, 2): /* SK<cond> - no change */
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  break;

	default:
	  reloc_type = fix ? fix->fx_r_type : BFD_RELOC_NONE;
	  break;
	}
      break;

    default:
      if (rl78b->n_fixups)
	{
	  reloc_type = fix->fx_r_type;
	  reloc_adjust = 0;
	}
      break;
    }

  if (rl78b->n_fixups)
    {

      fix->fx_r_type = reloc_type;
      fix->fx_where += reloc_adjust;
      switch (reloc_type)
	{
	case BFD_RELOC_NONE:
	  fix->fx_size = 0;
	  break;
	case BFD_RELOC_8:
	  fix->fx_size = 1;
	  break;
	case BFD_RELOC_16_PCREL:
	  fix->fx_size = 2;
	  break;
	}
    }

  fragP->fr_fix = fragP->fr_subtype + (fragP->fr_opcode - fragP->fr_literal);
  tprintf ("fragP->fr_fix now %ld (%d + (%p - %p)\n", (long) fragP->fr_fix,
	  fragP->fr_subtype, fragP->fr_opcode, fragP->fr_literal);
  fragP->fr_var = 0;

  tprintf ("compare 0x%lx vs 0x%lx - 0x%lx = 0x%lx (%p)\n",
	   (long)fragP->fr_fix,
	   (long)fragP->fr_next->fr_address, (long)fragP->fr_address,
	   (long)(fragP->fr_next->fr_address - fragP->fr_address),
	   fragP->fr_next);

  if (fragP->fr_next != NULL
      && fragP->fr_next->fr_address - fragP->fr_address != fragP->fr_fix)
    as_bad (_("bad frag at %p : fix %ld addr %ld %ld \n"), fragP,
	    (long) fragP->fr_fix,
	    (long) fragP->fr_address, (long) fragP->fr_next->fr_address);
}

/* End of relaxation code.
  ----------------------------------------------------------------------*/


arelent **
tc_gen_reloc (asection * seg ATTRIBUTE_UNUSED, fixS * fixp)
{
  static arelent * reloc[8];
  int rp;

  if (fixp->fx_r_type == BFD_RELOC_NONE)
    {
      reloc[0] = NULL;
      return reloc;
    }

  if (fixp->fx_r_type == BFD_RELOC_RL78_RELAX && !linkrelax)
    {
      reloc[0] = NULL;
      return reloc;
    }

  if (fixp->fx_subsy
      && S_GET_SEGMENT (fixp->fx_subsy) == absolute_section)
    {
      fixp->fx_offset -= S_GET_VALUE (fixp->fx_subsy);
      fixp->fx_subsy = NULL;
    }

  reloc[0]		  = XNEW (arelent);
  reloc[0]->sym_ptr_ptr   = XNEW (asymbol *);
  * reloc[0]->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc[0]->address       = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc[0]->addend        = fixp->fx_offset;

  if (fixp->fx_r_type == BFD_RELOC_RL78_32_OP
      && fixp->fx_subsy)
    {
      fixp->fx_r_type = BFD_RELOC_RL78_DIFF;
    }

#define OPX(REL,SYM,ADD)							\
  reloc[rp]		   = XNEW (arelent);		\
  reloc[rp]->sym_ptr_ptr   = XNEW (asymbol *);		\
  reloc[rp]->howto         = bfd_reloc_type_lookup (stdoutput, REL);		\
  reloc[rp]->addend        = ADD;						\
  * reloc[rp]->sym_ptr_ptr = SYM;						\
  reloc[rp]->address       = fixp->fx_frag->fr_address + fixp->fx_where;	\
  reloc[++rp] = NULL
#define OPSYM(SYM) OPX(BFD_RELOC_RL78_SYM, SYM, 0)

  /* FIXME: We cannot do the normal thing for an immediate value reloc,
     ie creating a RL78_SYM reloc in the *ABS* section with an offset
     equal to the immediate value we want to store.  This fails because
     the reloc processing in bfd_perform_relocation and bfd_install_relocation
     will short circuit such relocs and never pass them on to the special
     reloc processing code.  So instead we create a RL78_SYM reloc against
     the __rl78_abs__ symbol and arrange for the linker scripts to place
     this symbol at address 0.  */
#define OPIMM(IMM) OPX (BFD_RELOC_RL78_SYM, symbol_get_bfdsym (rl78_abs_sym), IMM)

#define OP(OP) OPX(BFD_RELOC_RL78_##OP, *reloc[0]->sym_ptr_ptr, 0)
#define SYM0() reloc[0]->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RL78_SYM)

  rp = 1;

  /* Certain BFD relocations cannot be translated directly into
     a single (non-Red Hat) RL78 relocation, but instead need
     multiple RL78 relocations - handle them here.  */
  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_RL78_DIFF:
      SYM0 ();
      OPSYM (symbol_get_bfdsym (fixp->fx_subsy));
      OP(OP_SUBTRACT);

      switch (fixp->fx_size)
	{
	case 1:
	  OP(ABS8);
	  break;
	case 2:
	  OP (ABS16);
	  break;
	case 4:
	  OP (ABS32);
	  break;
	}
      break;

    case BFD_RELOC_RL78_NEG32:
      SYM0 ();
      OP (OP_NEG);
      OP (ABS32);
      break;

    case BFD_RELOC_RL78_CODE:
      reloc[0]->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RL78_16U);
      reloc[1] = NULL;
      break;

    case BFD_RELOC_RL78_LO16:
      SYM0 ();
      OPIMM (0xffff);
      OP (OP_AND);
      OP (ABS16);
      break;

    case BFD_RELOC_RL78_HI16:
      SYM0 ();
      OPIMM (16);
      OP (OP_SHRA);
      OP (ABS16);
      break;

    case BFD_RELOC_RL78_HI8:
      SYM0 ();
      OPIMM (16);
      OP (OP_SHRA);
      OPIMM (0xff);
      OP (OP_AND);
      OP (ABS8);
      break;

    default:
      reloc[0]->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
      reloc[1] = NULL;
      break;
    }

  return reloc;
}

int
rl78_validate_fix_sub (struct fix * f)
{
  /* We permit the subtraction of two symbols in a few cases.  */
  /* mov #sym1-sym2, R3 */
  if (f->fx_r_type == BFD_RELOC_RL78_32_OP)
    return 1;
  /* .long sym1-sym2 */
  if (f->fx_r_type == BFD_RELOC_RL78_DIFF
      && ! f->fx_pcrel
      && (f->fx_size == 4 || f->fx_size == 2 || f->fx_size == 1))
    return 1;
  return 0;
}

long
md_pcrel_from_section (fixS * fixP, segT sec)
{
  long rv;

  if (fixP->fx_addsy != NULL
      && (! S_IS_DEFINED (fixP->fx_addsy)
	  || S_GET_SEGMENT (fixP->fx_addsy) != sec))
    /* The symbol is undefined (or is defined but not in this section).
       Let the linker figure it out.  */
    return 0;

  rv = fixP->fx_frag->fr_address + fixP->fx_where;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_8_PCREL:
      rv += 1;
      break;
    case BFD_RELOC_16_PCREL:
      rv += 2;
      break;
    default:
      break;
    }
  return rv;
}

void
md_apply_fix (struct fix * f ATTRIBUTE_UNUSED,
	      valueT *     t ATTRIBUTE_UNUSED,
	      segT         s ATTRIBUTE_UNUSED)
{
  char * op;
  unsigned long val;

  /* We always defer overflow checks for these to the linker, as it
     needs to do PLT stuff.  */
  if (f->fx_r_type == BFD_RELOC_RL78_CODE)
    f->fx_no_overflow = 1;

  if (f->fx_addsy && S_FORCE_RELOC (f->fx_addsy, 1))
    return;
  if (f->fx_subsy && S_FORCE_RELOC (f->fx_subsy, 1))
    return;

  op = f->fx_frag->fr_literal + f->fx_where;
  val = (unsigned long) * t;

  if (f->fx_addsy == NULL)
    f->fx_done = 1;

  switch (f->fx_r_type)
    {
    case BFD_RELOC_NONE:
      break;

    case BFD_RELOC_RL78_RELAX:
      f->fx_done = 0;
      break;

    case BFD_RELOC_8_PCREL:
      if ((long)val < -128 || (long)val > 127)
	as_bad_where (f->fx_file, f->fx_line,
		      _("value of %ld too large for 8-bit branch"),
		      val);
      /* Fall through.  */
    case BFD_RELOC_8:
    case BFD_RELOC_RL78_SADDR: /* We need to store the 8 LSB, but this works.  */
      op[0] = val;
      break;

    case BFD_RELOC_16_PCREL:
      if ((long)val < -32768 || (long)val > 32767)
	as_bad_where (f->fx_file, f->fx_line,
		      _("value of %ld too large for 16-bit branch"),
		      val);
      /* Fall through.  */
    case BFD_RELOC_16:
    case BFD_RELOC_RL78_CODE:
      op[0] = val;
      op[1] = val >> 8;
      break;

    case BFD_RELOC_24:
      op[0] = val;
      op[1] = val >> 8;
      op[2] = val >> 16;
      break;

    case BFD_RELOC_32:
      op[0] = val;
      op[1] = val >> 8;
      op[2] = val >> 16;
      op[3] = val >> 24;
      break;

    case BFD_RELOC_RL78_DIFF:
      op[0] = val;
      if (f->fx_size > 1)
	op[1] = val >> 8;
      if (f->fx_size > 2)
	op[2] = val >> 16;
      if (f->fx_size > 3)
	op[3] = val >> 24;
      break;

    case BFD_RELOC_RL78_HI8:
      val = val >> 16;
      op[0] = val;
      break;

    case BFD_RELOC_RL78_HI16:
      val = val >> 16;
      op[0] = val;
      op[1] = val >> 8;
      break;

    case BFD_RELOC_RL78_LO16:
      op[0] = val;
      op[1] = val >> 8;
      break;

    default:
      as_bad (_("Unknown reloc in md_apply_fix: %s"),
	      bfd_get_reloc_code_name (f->fx_r_type));
      break;
    }

}

valueT
md_section_align (segT segment, valueT size)
{
  int align = bfd_section_alignment (segment);
  return ((size + (1 << align) - 1) & -(1 << align));
}
