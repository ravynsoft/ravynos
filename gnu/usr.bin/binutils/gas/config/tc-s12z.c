/* tc-s12z.c -- Assembler code for the Freescale S12Z
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

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
#include "safe-ctype.h"
#include "subsegs.h"
#include "dwarf2dbg.h"
#include "opcode/s12z.h"
#include <limits.h>

const char comment_chars[] = ";";

const char line_comment_chars[] = "#*";
const char line_separator_chars[] = "";

static char * register_prefix = NULL;

const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "dD";

static char *fail_line_pointer;

/* A wrapper around the standard library's strtol.
   It converts STR into an integral value.
   This wrapper deals with literal_prefix_dollar_hex.  */
static long
s12z_strtol (const char *str, char ** endptr)
{
  int base = 0;
  bool negative = false;

  long result = 0;

  char *start = (char *) str;

  /* In the case where literal_prefix_dollar_hex is TRUE the sign has
  to be handled explicitly.  Otherwise the string will not be
  recognised as an integer.  */
  if (str[0] == '-')
    {
      negative = true;
      ++str;
    }
  else if (str[0] == '+')
    {
      ++str;
    }

  if (literal_prefix_dollar_hex && (str[0] == '$'))
    {
      base = 16;
      str++;
    }

  result = strtol (str, endptr, base);
  if (*endptr == str)
    {
      *endptr = start;
    }
  if (negative)
    result = -result;

  return result;
}



/* Options and initialization.  */

const char *md_shortopts = "";

struct option md_longopts[] =
  {
#define OPTION_REG_PREFIX (OPTION_MD_BASE)
   {"mreg-prefix", required_argument, NULL, OPTION_REG_PREFIX},
#define OPTION_DOLLAR_HEX (OPTION_MD_BASE + 1)
   {"mdollar-hex", no_argument, NULL, OPTION_DOLLAR_HEX},
   {NULL, no_argument, NULL, 0}
  };

size_t md_longopts_size = sizeof (md_longopts);


relax_typeS md_relax_table[] =
  {

  };

/* This table describes all the machine specific pseudo-ops the assembler
   has to support.  The fields are:
   pseudo-op name without dot
   function to call to execute this pseudo-op
   Integer arg to pass to the function.  */
const pseudo_typeS md_pseudo_table[] =
  {
    {0, 0, 0}
  };


/* Get the target cpu for the assembler.  */
const char *
s12z_arch_format (void)
{
  return "elf32-s12z";
}

enum bfd_architecture
s12z_arch (void)
{
  return bfd_arch_s12z;
}

int
s12z_mach (void)
{
  return 0;
}

/* Listing header selected according to cpu.  */
const char *
s12z_listing_header (void)
{
  return "S12Z GAS ";
}

void
md_show_usage (FILE *stream)
{
  fputs (_("\ns12z options:\n"), stream);
  fputs (_("  -mreg-prefix=PREFIX     set a prefix used to indicate register names (default none)\n"), stream);
  fputs (_("  -mdollar-hex            the prefix '$' instead of '0x' is used to indicate literal hexadecimal constants\n"), stream);
}

void
s12z_print_statistics (FILE *file ATTRIBUTE_UNUSED)
{
}

int
md_parse_option (int c, const char *arg)
{
  switch (c)
    {
    case OPTION_REG_PREFIX:
      register_prefix = xstrdup (arg);
      break;
    case OPTION_DOLLAR_HEX:
      literal_prefix_dollar_hex = true;
      break;
    default:
      return 0;
    }
  return 1;
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, true);
}

valueT
md_section_align (asection *seg, valueT addr)
{
  int align = bfd_section_alignment (seg);
  return ((addr + (1 << align) - 1) & -(1 << align));
}

void
md_begin (void)
{
}

void
s12z_init_after_args (void)
{
  if (flag_traditional_format)
    literal_prefix_dollar_hex = true;
}

/* Builtin help.  */


static char *
skip_whites (char *p)
{
  while (*p == ' ' || *p == '\t')
    p++;

  return p;
}



/* Start a new insn that contains at least 'size' bytes.  Record the
   line information of that insn in the dwarf2 debug sections.  */
static char *
s12z_new_insn (int size)
{
  char *f = frag_more (size);

  dwarf2_emit_insn (size);

  return f;
}



static bool lex_reg_name (uint16_t which, int *reg);

static bool
lex_constant (long *v)
{
  char *end = NULL;
  char *p = input_line_pointer;

  /* A constant may not have the same value as a register
     eg: "d6" */
  int dummy;
  if (lex_reg_name (~0, &dummy))
    {
      input_line_pointer = p;
      return false;
    }

  errno = 0;
  *v = s12z_strtol (p, &end);
  if (errno == 0 && end != p)
    {
      input_line_pointer = end;
      return true;
    }

  return false;
}

static bool
lex_match (char x)
{
  char *p = input_line_pointer;
  if (*p != x)
    return false;

  input_line_pointer++;
  return true;
}


static bool
lex_expression (expressionS *exp)
{
  char *ilp = input_line_pointer;
  int dummy;
  exp->X_op = O_absent;

  if (lex_match ('#'))
    goto fail;

  if (lex_reg_name (~0, &dummy))
    goto fail;

  expression (exp);
  if (exp->X_op != O_absent)
    return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

/* Immediate operand.
   If EXP_O is non-null, then a symbolic expression is permitted,
   in which case, EXP_O will be populated with the parsed expression.
 */
static bool
lex_imm (long *v, expressionS *exp_o)
{
  char *ilp = input_line_pointer;

  if (*input_line_pointer != '#')
    goto fail;

  input_line_pointer++;
  expressionS exp;
  if (!lex_expression (&exp))
    goto fail;

  if (exp.X_op != O_constant)
    {
      if (!exp_o)
        as_bad (_("A non-constant expression is not permitted here"));
      else
        *exp_o = exp;
    }

  *v = exp.X_add_number;
  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

/* Short mmediate operand */
static bool
lex_imm_e4 (long *val)
{
  char *ilp = input_line_pointer;
  if ((lex_imm (val, NULL)))
    {
      if ((*val == -1) || (*val > 0 && *val <= 15))
	{
	  return true;
	}
    }
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
lex_match_string (const char *s)
{
  char *p = input_line_pointer;
  while (p != 0 && *p != '\t' && *p != ' ' && *p != '\0')
    {
      p++;
    }

  size_t len = p - input_line_pointer;
  if (len != strlen (s))
    return false;

  if (0 == strncasecmp (s, input_line_pointer, len))
    {
      input_line_pointer = p;
      return true;
    }

  return false;
}

/* Parse a register name.
   WHICH is a ORwise combination of the registers which are accepted.
   ~0 accepts all.
   On success, REG will be filled with the index of the register which
   was successfully scanned.
*/
static bool
lex_reg_name (uint16_t which, int *reg)
{
  char *p = input_line_pointer;

  if (p == 0)
    return false;

  /* Scan (and ignore) the register prefix.  */
  if (register_prefix)
    {
      int len = strlen (register_prefix);
      if (0 == strncmp (register_prefix, p, len))
        p += len;
      else
        return false;
    }

  char *start_of_reg_name = p;

  while ((*p >= 'a' && *p <='z')
         || (*p >= '0' && *p <= '9')
         || (*p >= 'A' && *p <='Z'))
    {
      p++;
    }

  size_t len = p - start_of_reg_name;

  if (len <= 0)
    return false;

  int i;
  for (i = 0; i < S12Z_N_REGISTERS; ++i)
    {
      gas_assert (registers[i].name);

      if (len == strlen (registers[i].name)
	  && 0 == strncasecmp (registers[i].name, start_of_reg_name, len))
	{
	  if ((0x1U << i) & which)
	    {
	      input_line_pointer = p;
	      *reg = i;
	      return true;
	    }
	}
    }

  return false;
}

static bool
lex_force_match (char x)
{
  char *p = input_line_pointer;
  if (*p != x)
    {
      as_bad (_("Expecting '%c'"), x);
      return false;
    }

  input_line_pointer++;
  return true;
}

static bool
lex_opr (uint8_t *buffer, int *n_bytes, expressionS *exp,
	 bool immediate_ok)
{
  char *ilp = input_line_pointer;
  uint8_t *xb = buffer;
  int reg;
  long imm;
  exp->X_op = O_absent;
  *n_bytes = 0;
  *xb = 0;
  if (lex_imm_e4 (&imm))
    {
      if (!immediate_ok)
	{
	  as_bad (_("An immediate value in a source operand is inappropriate"));
	  return false;
	}
      if (imm > 0)
	*xb = imm;
      else
	*xb = 0;
      *xb |= 0x70;
      *n_bytes = 1;
      return true;
    }
  else if (lex_reg_name (REG_BIT_Dn, &reg))
    {
      *xb = reg;
      *xb |= 0xb8;
      *n_bytes = 1;
      return true;
    }
  else if (lex_match ('['))
    {
      if (lex_expression (exp))
	{
	  long c = exp->X_add_number;
	  if (lex_match (','))
	    {
	      if (lex_reg_name (REG_BIT_XYSP, &reg))
		{
		  int i;
		  if (c <= 255 && c >= -256)
		    {
		      *n_bytes = 2;
		      *xb |= 0xc4;
		    }
		  else
		    {
		      *n_bytes = 4;
		      *xb |= 0xc6;
		    }
		  *xb |= (reg - REG_X) << 4;

		  if (c < 0)
		    *xb |= 0x01;
		  for (i = 1; i < *n_bytes ; ++i)
		    {
		      buffer[i] = c >> (8 * (*n_bytes - i - 1));
		    }
		}
	      else
		{
		  as_bad (_("Bad operand for constant offset"));
		  goto fail;
		}
	    }
	  else
	    {
	      *xb = 0xfe;
	      *n_bytes = 4;
	      buffer[1] = c >> 16;
	      buffer[2] = c >> 8;
	      buffer[3] = c;
	    }
	}
      else if (lex_reg_name (REG_BIT_Dn, &reg))
	{
	  if (!lex_force_match (','))
	    goto fail;

	  int reg2;
	  if (lex_reg_name (REG_BIT_XY, &reg2))
	    {
	      *n_bytes = 1;
	      *xb = reg;
	      *xb |= (reg2 - REG_X) << 4;
	      *xb |= 0xc8;
	    }
	  else
	    {
	      as_bad (_("Invalid operand for register offset"));
	      goto fail;
	    }
	}
      else
	{
	  goto fail;
	}
      if (!lex_force_match (']'))
	goto fail;
      return true;
    }
  else if (lex_match ('('))
    {
      long c;
      if (lex_constant (&c))
	{
	  if (!lex_force_match (','))
	    goto fail;
	  int reg2;
	  if (lex_reg_name (REG_BIT_XYSP, &reg2))
	    {
	      if (reg2 != REG_P && c >= 0 && c <= 15)
		{
		  *n_bytes = 1;
		  *xb = 0x40;
		  *xb |= (reg2 - REG_X) << 4;
		  *xb |= c;
		}
	      else if (c >= -256 && c <= 255)
		{
		  *n_bytes = 2;
		  *xb = 0xc0;
		  *xb |= (reg2 - REG_X) << 4;
		  if (c < 0)
		    *xb |= 0x01;
		  buffer[1] = c;
		}
	      else
		{
		  *n_bytes = 4;
		  *xb = 0xc2;
		  *xb |= (reg2 - REG_X) << 4;
		  buffer[1] = c >> 16;
		  buffer[2] = c >> 8;
		  buffer[3] = c;
		}
	    }
	  else if (lex_reg_name (REG_BIT_Dn, &reg2))
	    {
	      if (c >= -1 * (long) (0x1u << 17)
		  &&
		  c < (long) (0x1u << 17) - 1)
		{
		  *n_bytes = 3;
		  *xb = 0x80;
		  *xb |= reg2;
		  *xb |= ((c >> 16) & 0x03) << 4;
		  buffer[1] = c >> 8;
		  buffer[2] = c;
		}
	      else
		{
		  *n_bytes = 4;
		  *xb = 0xe8;
		  *xb |= reg2;
		  buffer[1] = c >> 16;
		  buffer[2] = c >> 8;
		  buffer[3] = c;
		}
	    }
	  else
	    {
	      as_bad (_("Bad operand for constant offset"));
	      goto fail;
	    }
	}
      else if (lex_reg_name (REG_BIT_Dn, &reg))
	{
	  if (lex_match (','))
	    {
	      int reg2;
	      if (lex_reg_name (REG_BIT_XYS, &reg2))
		{
		  *n_bytes = 1;
		  *xb = 0x88;
		  *xb |= (reg2 - REG_X) << 4;
		  *xb |= reg;
		}
	      else
		{
		  as_bad (_("Invalid operand for register offset"));
		  goto fail;
		}
	    }
	  else
	    {
	      goto fail;
	    }
	}
      else if (lex_reg_name (REG_BIT_XYS, &reg))
	{
	  if (lex_match ('-'))
	    {
	      if (reg == REG_S)
		{
		  as_bad (_("Invalid register for postdecrement operation"));
		  goto fail;
		}
	      *n_bytes = 1;
	      if (reg == REG_X)
		*xb = 0xc7;
	      else if (reg == REG_Y)
		*xb = 0xd7;
	    }
	  else if (lex_match ('+'))
	    {
	      *n_bytes = 1;
	      if (reg == REG_X)
		*xb = 0xe7;
	      else if (reg == REG_Y)
		*xb = 0xf7;
	      else if (reg == REG_S)
		*xb = 0xff;
	    }
	  else
	    {
	      goto fail;
	    }
	}
      else if (lex_match ('+'))
	{
	  if (lex_reg_name (REG_BIT_XY, &reg))
	    {
	      *n_bytes = 1;
	      if (reg == REG_X)
		*xb = 0xe3;
	      else if (reg == REG_Y)
		*xb = 0xf3;
	    }
	  else
	    {
	      as_bad (_("Invalid register for preincrement operation"));
	      goto fail;
	    }
	}
      else if (lex_match ('-'))
	{
	  if (lex_reg_name (REG_BIT_XYS, &reg))
	    {
	      *n_bytes = 1;
	      if (reg == REG_X)
		*xb = 0xc3;
	      else if (reg == REG_Y)
		*xb = 0xd3;
	      else if (reg == REG_S)
		*xb = 0xfb;
	    }
	  else
	    {
	      as_bad (_("Invalid register for predecrement operation"));
	      goto fail;
	    }
	}
      else
	{
	  goto fail;
	}

      if (! lex_match (')'))
	goto fail;
      return true;
    }
  else if (lex_expression (exp))
    {
      *xb = 0xfa;
      *n_bytes = 4;
      buffer[1] = 0;
      buffer[2] = 0;
      buffer[3] = 0;
      if (exp->X_op == O_constant)
	{
	  valueT value = exp->X_add_number;

	  if (value < (0x1U << 14))
	    {
	      *xb = 0x00;
	      *n_bytes = 2;
	      *xb |= value >> 8;
	      buffer[1] = value;
	    }
	  else if (value < (0x1U << 19))
	    {
	      *xb = 0xf8;
	      if (value & (0x1U << 17))
		*xb |= 0x04;
	      if (value & (0x1U << 16))
		*xb |= 0x01;
	      *n_bytes = 3;
	      buffer[1] = value >> 8;
	      buffer[2] = value;
	    }
	  else
	    {
	      *xb = 0xfa;
	      *n_bytes = 4;
	      buffer[1] = value >> 16;
	      buffer[2] = value >> 8;
	      buffer[3] = value;
	    }
	}
      return true;
    }

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
lex_offset (long *val)
{
  char *end = NULL;
  char *p = input_line_pointer;

  if (*p++ != '*')
    return false;

  if (*p != '+' && *p != '-')
    return false;

  bool negative =  (*p == '-');
  p++;

  errno = 0;
  *val = s12z_strtol (p, &end);
  if (errno == 0)
    {
      if (negative)
	*val *= -1;
      input_line_pointer = end;
      return true;
    }

  return false;
}



struct instruction;

typedef bool (*parse_operand_func) (const struct instruction *);

struct instruction
{
  const char *name;

  /* The "page" to which the instruction belongs.
     This is also only a hint.  Some instructions might have modes in both
     pages... */
  char page;

  /* This is a hint - and only a hint - about the opcode of the instruction.
     The parse_operand_func is free to ignore it.
  */
  uint8_t opc;

  parse_operand_func parse_operands;

  /* Some instructions can be encoded with a different opcode */
  uint8_t alt_opc;
};

static bool
no_operands (const struct instruction *insn)
{
  if (*input_line_pointer != '\0')
    {
      as_bad (_("Garbage at end of instruction"));
      return false;
    }

  char *f = s12z_new_insn (insn->page);
  if (insn->page == 2)
    number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

  number_to_chars_bigendian (f++, insn->opc, 1);

  return true;
}


static void
emit_reloc (expressionS *exp, char *f, int size, enum bfd_reloc_code_real reloc)
{
  if (exp->X_op != O_absent && exp->X_op != O_constant)
    {
      fixS *fix = fix_new_exp (frag_now,
			       f - frag_now->fr_literal,
			       size,
			       exp,
			       false,
                               reloc);
      /* Some third party tools seem to use the lower bits
         of this addend for flags.   They don't get added
         to the final location.   The purpose of these flags
         is not known.  We simply set it to zero.  */
      fix->fx_addnumber = 0x00;
    }
}

/* Emit the code for an OPR address mode operand */
static char *
emit_opr (char *f, const uint8_t *buffer, int n_bytes, expressionS *exp)
{
  int i;
  number_to_chars_bigendian (f++, buffer[0], 1);

  emit_reloc (exp, f, 3, BFD_RELOC_S12Z_OPR);

  for (i = 1; i < n_bytes; ++i)
    number_to_chars_bigendian (f++,  buffer[i], 1);

  return f;
}

/* Emit the code for a 24 bit direct address operand */
static char *
emit_ext24 (char *f, long v)
{
  number_to_chars_bigendian (f, v, 3);

  return f + 3;
}

static bool
opr (const struct instruction *insn)
{
  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (lex_opr (buffer, &n_bytes, &exp, false))
    {
      /* Large constant direct values are more efficiently encoded as ext24 mode.
	 Otherwise a decision has to be deferred to a relax. */
      if (exp.X_op == O_constant
	  && buffer[0] == 0xFA
	  && insn->alt_opc != 0)
	{
	  char *f = s12z_new_insn (4);

	  /* I don't think there are any instances of page 2 opcodes in this case */
	  gas_assert (insn->page == 1);

	  number_to_chars_bigendian (f++, insn->alt_opc, 1);

	  emit_ext24 (f, exp.X_add_number);
	}
      else
	{
	  char *f = s12z_new_insn (n_bytes + 1);
	  number_to_chars_bigendian (f++, insn->opc, 1);

	  emit_opr (f, buffer, n_bytes, &exp);
	}
      return true;
    }

  return false;
}

/* Parse a 15 bit offset, as an expression.
   LONG_DISPLACEMENT will be set to true if the offset is wider than 7 bits.
   */
static bool
lex_15_bit_offset (bool *long_displacement, expressionS *exp)
{
  char *ilp = input_line_pointer;

  long val;
  if (lex_offset (&val))
    {
      exp->X_op = O_absent;
      exp->X_add_number = val;
    }
  else if (lex_expression (exp))
    {
      if (exp->X_op == O_constant)
	{
	  val = exp->X_add_number;
	}
      else
	{
	  /* If a symbol was parsed we don't know the displacement.
	     We have to assume it is long, and relax it later if possible. */
	  *long_displacement = true;
	  return true;
	}
    }
  else
    {
      exp->X_op = O_absent;
      goto fail;
    }

  if (val > 0x3FFF || val < -0x4000)
    {
      as_fatal (_("Offset is outside of 15 bit range"));
      return false;
    }

  *long_displacement = (val > 63 || val < -64);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static void
emit_15_bit_offset (char *f, int where, expressionS *exp)
{
  gas_assert (exp);
  if (exp->X_op != O_absent && exp->X_op != O_constant)
    {
      exp->X_add_number += where;
      fixS *fix = fix_new_exp (frag_now,
		   f - frag_now->fr_literal,
		   2,
		   exp,
		   true,
		   BFD_RELOC_16_PCREL);
      fix->fx_addnumber = where - 2;
    }
  else
    {
      long val = exp->X_add_number;
      bool long_displacement = (val > 63 || val < -64);
      if (long_displacement)
	val |= 0x8000;
      else
	val &= 0x7F;

      number_to_chars_bigendian (f++, val, long_displacement ? 2 : 1);
    }
}

static bool
rel (const struct instruction *insn)
{
  bool long_displacement;

  expressionS exp;
  if (! lex_15_bit_offset (&long_displacement, &exp))
    return false;

  char *f = s12z_new_insn (long_displacement ? 3 : 2);
  number_to_chars_bigendian (f++, insn->opc, 1);
  emit_15_bit_offset (f, 3, &exp);
  return true;
}

static bool
reg_inh (const struct instruction *insn)
{
  int reg;
  if (lex_reg_name (REG_BIT_Dn, &reg))
    {
      char *f = s12z_new_insn (insn->page);
      if (insn->page == 2)
	number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

      number_to_chars_bigendian (f++, insn->opc + reg, 1);
      return true;
    }

  return false;
}


/* Special case for CLR X and CLR Y */
static bool
clr_xy (const struct instruction *insn ATTRIBUTE_UNUSED)
{
  int reg;
  if (lex_reg_name (REG_BIT_XY, &reg))
    {
      char *f = s12z_new_insn (1);
      number_to_chars_bigendian (f, 0x9a + reg - REG_X, 1);
      return true;
    }

  return false;
}

/* Some instructions have a suffix like ".l", ".b", ".w" etc
   which indicates the size of the operands. */
static int
size_from_suffix  (const struct instruction *insn, int idx)
{
  const char *dot = strchr (insn->name, '.');

  if (dot == NULL)
    return -3;

  int size = -2;
  switch (dot[1 + idx])
    {
    case 'b':
      size = 1;
      break;
    case 'w':
      size = 2;
      break;
    case 'p':
      size = 3;
      break;
    case 'l':
      size = 4;
      break;
    default:
      as_fatal (_("Bad size"));
    };

  return size;
}

static bool
mul_reg_reg_reg (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  int Dd;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dj;
  if (!lex_reg_name (REG_BIT_Dn, &Dj))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dk;
  if (!lex_reg_name (REG_BIT_Dn, &Dk))
    goto fail;

  char *f = s12z_new_insn (insn->page + 1);
  if (insn->page == 2)
    number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

  number_to_chars_bigendian (f++, insn->opc + Dd, 1);
  const char *dot = strchrnul (insn->name, '.');
  uint8_t mb ;
  switch (dot[-1])
    {
    case 's':
      mb = 0x80;
      break;
    case 'u':
      mb = 0x00;
      break;
    default:
      as_fatal (_("BAD MUL"));
      break;
    }

  mb |= Dj << 3;
  mb |= Dk;

  number_to_chars_bigendian (f++, mb, 1);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
mul_reg_reg_imm (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  int Dd;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dj;
  if (!lex_reg_name (REG_BIT_Dn, &Dj))
    goto fail;

  if (!lex_match (','))
    goto fail;

  long imm;
  if (!lex_imm (&imm, NULL))
    goto fail;


  int size = size_from_suffix (insn, 0);

  char *f = s12z_new_insn (insn->page + 1 + size);
  if (insn->page == 2)
    number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

  number_to_chars_bigendian (f++, insn->opc + Dd, 1);
  uint8_t mb = 0x44;
  const char *dot = strchrnul (insn->name, '.');
  switch (dot[-1])
    {
    case 's':
      mb |= 0x80;
      break;
    case 'u':
      mb |= 0x00;
      break;
    default:
      as_fatal (_("BAD MUL"));
      break;
    }

  mb |= Dj << 3;
  mb |= size  - 1;

  number_to_chars_bigendian (f++, mb, 1);
  number_to_chars_bigendian (f++, imm, size);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
mul_reg_reg_opr (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  int Dd;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dj;
  if (!lex_reg_name (REG_BIT_Dn, &Dj))
    goto fail;

  if (!lex_match (','))
    goto fail;

  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, true))
    goto fail;

  int size = size_from_suffix (insn, 0);

  char *f = s12z_new_insn (insn->page + 1 + n_bytes);
  if (insn->page == 2)
    number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

  number_to_chars_bigendian (f++, insn->opc + Dd, 1);
  uint8_t mb = 0x40;
  const char *dot = strchrnul (insn->name, '.');
  switch (dot[-1])
    {
    case 's':
      mb |= 0x80;
      break;
    case 'u':
      mb |= 0x00;
      break;
    default:
      as_fatal (_("BAD MUL"));
      break;
    }

  mb |= Dj << 3;
  mb |= size  - 1;

  number_to_chars_bigendian (f++, mb, 1);

  emit_opr (f, buffer, n_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
mul_reg_opr_opr (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  int Dd;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;

  uint8_t buffer1[4];
  int n_bytes1;
  expressionS exp1;
  if (!lex_opr (buffer1, &n_bytes1, &exp1, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  uint8_t buffer2[4];
  int n_bytes2;
  expressionS exp2;
  if (!lex_opr (buffer2, &n_bytes2, &exp2, false))
    goto fail;

  int size1 = size_from_suffix (insn, 0);
  int size2 = size_from_suffix (insn, 1);

  char *f = s12z_new_insn (insn->page + 1 + n_bytes1 + n_bytes2);
  if (insn->page == 2)
    number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

  number_to_chars_bigendian (f++, insn->opc + Dd, 1);
  uint8_t mb = 0x42;
  const char *dot = strchrnul (insn->name, '.');
  switch (dot[-1])
    {
    case 's':
      mb |= 0x80;
      break;
    case 'u':
      mb |= 0x00;
      break;
    default:
      as_fatal (_("BAD MUL"));
      break;
    }

  mb |= (size1  - 1) << 4;
  mb |= (size2  - 1) << 2;
  number_to_chars_bigendian (f++, mb, 1);

  f = emit_opr (f, buffer1, n_bytes1, &exp1);
  f = emit_opr (f, buffer2, n_bytes2, &exp2);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


#define REG_BIT_GRP0				\
  ((0x1U << REG_D2) |				\
   (0x1U << REG_D3) |				\
   (0x1U << REG_CCH) |				\
   (0x1U << REG_CCL) |				\
   (0x1U << REG_D0) |				\
   (0x1U << REG_D1))

#define REG_BIT_GRP1				\
  ((0x1U << REG_D4) |				\
   (0x1U << REG_D5) |				\
   (0x1U << REG_D6) |				\
   (0x1U << REG_D7) |				\
   (0x1U << REG_X) |				\
   (0x1U << REG_Y))

static const uint8_t reg_map [] =
  {
    0x02,  /* D2 */
    0x01,  /* D3 */
    0x20,
    0x10,  /* D5 */
    0x08,  /* D0 */
    0x04,  /* D1 */
    0x08,  /* D6 */
    0x04,  /* D7 */
    0x02,
    0x01,  /* Y */
    0x00,
    0x00,
    0x20,  /* CCH */
    0x10,  /* CCL */
    0x00
  };

static bool
lex_reg_list (uint16_t grp, uint16_t *reg_bits)
{
  if (lex_match (','))
    {
      int reg;
      if (!lex_reg_name (grp, &reg))
	return false;
      *reg_bits |= 0x1u << reg;
      lex_reg_list (grp, reg_bits);
    }

  /* Empty list */
  return true;
}

static bool
psh_pull (const struct instruction *insn)
{
  uint8_t pb =
    (0 == strcmp ("pul", insn->name)) ? 0x80: 0x00;

  if (lex_match_string ("all16b"))
    {
      pb |= 0x40;
    }
  else if (lex_match_string ("all"))
    {
      /* Nothing to do */
    }
  else
    {
      int reg1;
      if (!lex_reg_name (REG_BIT_GRP1 | REG_BIT_GRP0, &reg1))
	goto fail;
      uint16_t admitted_group = 0;

      if ((0x1U << reg1) & REG_BIT_GRP1)
	admitted_group = REG_BIT_GRP1;
      else if ((0x1U << reg1) & REG_BIT_GRP0)
	admitted_group = REG_BIT_GRP0;

      uint16_t reg_bits = 0x1 << reg1;
      if (!lex_reg_list (admitted_group, &reg_bits))
	goto fail;

      if (reg_bits & REG_BIT_GRP1)
	pb |= 0x40;

      int i;
      for (i = 0; i < 16; ++i)
	{
	  if (reg_bits & (0x1u << i))
	    pb |= reg_map[i];
	}
    }

  char *f = s12z_new_insn (2);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, pb, 1);
  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  return false;
}


static bool
tfr (const struct instruction *insn)
{
  int reg1;
  if (!lex_reg_name (~0, &reg1))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int reg2;
  if (!lex_reg_name (~0, &reg2))
    goto fail;

  if ( ((0 == strcasecmp ("sex", insn->name))
        || (0 == strcasecmp ("zex", insn->name)))
       && (registers[reg2].bytes <= registers[reg1].bytes))
      as_warn (_("Source register for %s is no larger than the destination register"),
               insn->name);
  else if (reg1 == reg2)
    as_warn (_("The destination and source registers are identical"));

  char *f = s12z_new_insn (1 + insn->page);
  if (insn->page == 2)
    number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, reg1 << 4 | reg2, 1);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  return false;
}

static bool
imm8 (const struct instruction *insn)
{
  long imm;
  if (! lex_imm (&imm, NULL))
    return false;
  if (imm > 127 || imm < -128)
    {
      as_bad (_("Immediate value %ld is out of range for instruction %s"),
	      imm, insn->name);
    }

  char *f = s12z_new_insn (2);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, imm, 1);

  return true;
}

static bool
reg_imm (const struct instruction *insn, int allowed_reg)
{
  char *ilp = input_line_pointer;
  int reg;
  if (lex_reg_name (allowed_reg, &reg))
    {
      if (!lex_force_match (','))
	goto fail;
      long imm;
      if (! lex_imm (&imm, NULL))
	goto fail;

      short size = registers[reg].bytes;
      char *f = s12z_new_insn (insn->page + size);
      if (insn->page == 2)
	number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

      number_to_chars_bigendian (f++, insn->opc + reg, 1);
      number_to_chars_bigendian (f++, imm, size);
      return true;
    }

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
regd_imm (const struct instruction *insn)
{
  return reg_imm (insn, REG_BIT_Dn);
}

static bool
regdxy_imm (const struct instruction *insn)
{
  return reg_imm (insn, REG_BIT_Dn | REG_BIT_XY);
}


static bool
regs_imm (const struct instruction *insn)
{
  return reg_imm (insn, 0x1U << REG_S);
}

static bool
trap_imm (const struct instruction *insn ATTRIBUTE_UNUSED)
{
  long imm = -1;
  if (! lex_imm (&imm, NULL))
    goto fail;

  if (imm < 0x92 || imm > 0xFF ||
      (imm >= 0xA0 && imm <= 0xA7) ||
      (imm >= 0xB0 && imm <= 0xB7))
    {
      as_bad (_("trap value %ld is not valid"), imm);
      return false;
    }
  else
    {
      char *f = s12z_new_insn (2);
      number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);
      number_to_chars_bigendian (f++, imm & 0xFF, 1);
      return true;
    }

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  return false;
}



/* Special one byte instruction CMP X, Y */
static bool
regx_regy (const struct instruction *insn)
{
  int reg;
  if (lex_reg_name (0x1U << REG_X, &reg))
    {
      if (lex_force_match (','))
	{
	  if (lex_reg_name (0x1U << REG_Y, &reg))
	    {
	      char *f = s12z_new_insn (1);
	      number_to_chars_bigendian (f, insn->opc, 1);
	      return true;
	    }
	}
    }
  return false;
}

/* Special one byte instruction SUB D6, X, Y */
static bool
regd6_regx_regy (const struct instruction *insn)
{
  char *ilp = input_line_pointer;
  int reg;
  if (!lex_reg_name (0x1U << REG_D6, &reg))
    goto fail;

  if (!lex_match (','))
    goto fail;

  if (!lex_reg_name (0x1U << REG_X, &reg))
    goto fail;

  if (!lex_match (','))
    goto fail;

  if (!lex_reg_name (0x1U << REG_Y, &reg))
    goto fail;

  char *f = s12z_new_insn (1);
  number_to_chars_bigendian (f, insn->opc, 1);
  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

/* Special one byte instruction SUB D6, Y, X */
static bool
regd6_regy_regx (const struct instruction *insn)
{
  char *ilp = input_line_pointer;
  int reg;
  if (!lex_reg_name (0x1U << REG_D6, &reg))
    goto fail;

  if (!lex_match (','))
    goto fail;

  if (!lex_reg_name (0x1U << REG_Y, &reg))
    goto fail;

  if (!lex_match (','))
    goto fail;

  if (!lex_reg_name (0x1U << REG_X, &reg))
    goto fail;

  char *f = s12z_new_insn (1);
  number_to_chars_bigendian (f, insn->opc, 1);
  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
reg_opr (const struct instruction *insn, int allowed_regs,
	 bool immediate_ok)
{
  char *ilp = input_line_pointer;
  int reg;
  if (lex_reg_name (allowed_regs, &reg))
    {
      if (!lex_force_match (','))
	goto fail;

      uint8_t buffer[4];
      int n_bytes;
      expressionS exp;
      if (lex_opr (buffer, &n_bytes, &exp, immediate_ok))
	{
	  /* Large constant direct values are more efficiently encoded as ext24 mode.
	     Otherwise a decision has to be deferred to a relax. */
	  if (exp.X_op == O_constant
	      && buffer[0] == 0xFA
	      && insn->alt_opc != 0)
	    {
	      char *f = s12z_new_insn (4);

	      /* I don't think there are any instances of page 2 opcodes in this case */
	      gas_assert (insn->page == 1);

	      number_to_chars_bigendian (f++, insn->alt_opc + reg, 1);

	      emit_ext24 (f, exp.X_add_number);
	    }
	  else
	    {
	      char *f = s12z_new_insn (n_bytes + insn->page);

	      if (insn->page == 2)
		number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);

	      number_to_chars_bigendian (f++, insn->opc + reg, 1);

	      emit_opr (f, buffer, n_bytes, &exp);
	    }

	  return true;
	}
    }

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
regdxy_opr_dest (const struct instruction *insn)
{
  return reg_opr (insn, REG_BIT_Dn | REG_BIT_XY, false);
}

static bool
regdxy_opr_src (const struct instruction *insn)
{
  return reg_opr (insn, REG_BIT_Dn | REG_BIT_XY, true);
}


static bool
regd_opr (const struct instruction *insn)
{
  return reg_opr (insn, REG_BIT_Dn, true);
}


/* OP0: S; OP1: destination OPR */
static bool
regs_opr_dest (const struct instruction *insn)
{
  return reg_opr (insn, 0x1U << REG_S, false);
}

/* OP0: S; OP1: source OPR */
static bool
regs_opr_src (const struct instruction *insn)
{
  return reg_opr (insn, 0x1U << REG_S, true);
}

static bool
imm_opr  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;
  long imm;
  expressionS exp0;
  int size = size_from_suffix (insn, 0);
  exp0.X_op = O_absent;

  /* Note:  The ternary expression below means that "MOV.x #symbol,
     mem-expr"  is accepted when x is a member of {'w', 'p', 'l'} but
     not when it is 'b'.
     The Freescale assembler accepts "MOV.b #symbol, mem-expr" but
     produces obviously incorrect code.    Since such an instruction
     would require an 8-bit reloc (which we don't have) and some
     non-optimal kludges in the OPR encoding, it seems sensible that
     such instructions should be rejected.  */
  if (!lex_imm (&imm, size > 1 ? &exp0 : NULL))
    goto fail;

  if (!lex_match (','))
    goto fail;

  uint8_t buffer[4];
  int n_bytes;
  expressionS exp1;
  if (!lex_opr (buffer, &n_bytes, &exp1, false))
    goto fail;

  char *f = s12z_new_insn (1 + n_bytes + size);
  number_to_chars_bigendian (f++, insn->opc, 1);

  emit_reloc (&exp0, f, size, size == 4 ? BFD_RELOC_32 : BFD_RELOC_S12Z_OPR);

  int i;
  for (i = 0; i < size; ++i)
    number_to_chars_bigendian (f++, imm >> (CHAR_BIT * (size - i - 1)), 1);

  emit_opr (f, buffer, n_bytes, &exp1);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
opr_opr  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  uint8_t buffer1[4];
  int n_bytes1;
  expressionS exp1;
  if (!lex_opr (buffer1, &n_bytes1, &exp1, false))
    goto fail;


  if (!lex_match (','))
    goto fail;

  uint8_t buffer2[4];
  int n_bytes2;
  expressionS exp2;
  if (!lex_opr (buffer2, &n_bytes2, &exp2, false))
    goto fail;

  char *f = s12z_new_insn (1 + n_bytes1 + n_bytes2);
  number_to_chars_bigendian (f++, insn->opc, 1);

  f = emit_opr (f, buffer1, n_bytes1, &exp1);
  f = emit_opr (f, buffer2, n_bytes2, &exp2);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
reg67sxy_opr  (const struct instruction *insn)
{
  int reg;
  if (!lex_reg_name (REG_BIT_XYS | (0x1U << REG_D6) | (0x1U << REG_D7), &reg))
    return false;

  if (!lex_match (','))
    return false;

  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, false))
    return false;

  char *f = s12z_new_insn (1 + n_bytes);
  number_to_chars_bigendian (f++, insn->opc + reg - REG_D6, 1);
  emit_opr (f, buffer, n_bytes, &exp);

  return true;
}

static bool
rotate  (const struct instruction *insn, short dir)
{
  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (lex_opr (buffer, &n_bytes, &exp, false))
    {
      char *f = s12z_new_insn (n_bytes + 2);
      number_to_chars_bigendian (f++, insn->opc, 1);
      int size = size_from_suffix (insn, 0);
      if (size < 0)
	size = 1;
      uint8_t sb = 0x24;
      sb |= size - 1;
      if (dir)
	sb |= 0x40;
      number_to_chars_bigendian (f++, sb, 1);
      emit_opr (f, buffer, n_bytes, &exp);

      return true;
    }

  return false;
}

static bool
rol  (const struct instruction *insn)
{
  return rotate (insn, 1);
}

static bool
ror  (const struct instruction *insn)
{
  return rotate (insn, 0);
}


/* Shift instruction with a register operand and an immediate #1 or #2
   left = 1; right = 0;
   logical = 0; arithmetic = 1;
*/
static bool
lex_shift_reg_imm1  (const struct instruction *insn, short type, short dir)
{
  /*
    This function is highly unusual and a bit wierd!
    It first matches the input against a register {d0, d1, ... d7} followed by an immediate
    {#1, #2}.
    Then, it rewinds the input and parses it again as a OPR.
  */
  char *ilp = input_line_pointer;

  int Dd;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    {
      goto fail;
    }

  if (!lex_match (','))
    goto fail;

  long imm = -1;
  if (!lex_imm (&imm, NULL))
    goto fail;

  if (imm != 1 && imm != 2)
    goto fail;
  input_line_pointer = ilp;

  /* Now parse the first operand again */

  uint8_t buffer[4];
  int n_bytes;

  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, false))
    goto fail;

  gas_assert (n_bytes == 1);

  uint8_t sb = 0x34;
  sb |= dir << 6;
  sb |= type << 7;
  if (imm == 2)
    sb |= 0x08;

  char *f = s12z_new_insn (3);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, sb, 1);
  emit_opr (f, buffer, n_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

/* Shift instruction with a register operand.
   left = 1; right = 0;
   logical = 0; arithmetic = 1; */
static bool
lex_shift_reg  (const struct instruction *insn, short type, short dir)
{
  int Dd, Ds, Dn;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    {
      goto fail;
    }

  if (!lex_match (','))
    goto fail;

  if (!lex_reg_name (REG_BIT_Dn, &Ds))
    {
      goto fail;
    }

  if (!lex_match (','))
    goto fail;

  uint8_t sb = 0x10;
  sb |= Ds;
  sb |= dir << 6;
  sb |= type << 7;
  long imm;
  if (lex_reg_name (REG_BIT_Dn, &Dn))
    {
      char *f = s12z_new_insn (3);
      number_to_chars_bigendian (f++, insn->opc | Dd, 1);
      number_to_chars_bigendian (f++, sb, 1);
      uint8_t xb = 0xb8;
      xb |= Dn;
      number_to_chars_bigendian (f++, xb, 1);

      return true;
    }
  else if (lex_imm (&imm, NULL))
    {
      if (imm < 0 || imm > 31)
	{
	  as_bad (_("Shift value should be in the range [0,31]"));
	  goto fail;
	}

      int n_bytes = 3;
      if (imm == 1 || imm == 2)
	{
	  n_bytes = 2;
	  sb &= ~0x10;
	}
      else
	{
	  sb |= (imm & 0x01) << 3;
	}

      char *f = s12z_new_insn (n_bytes);
      number_to_chars_bigendian (f++, insn->opc | Dd, 1);
      number_to_chars_bigendian (f++, sb, 1);
      if (n_bytes > 2)
	{
	  uint8_t xb = 0x70;
	  xb |= imm >> 1;
	  number_to_chars_bigendian (f++, xb, 1);
	}

      return true;
    }

 fail:
  fail_line_pointer = input_line_pointer;
  return false;
}

static void
impute_shift_dir_and_type (const struct instruction *insn, short *type, short *dir)
{
  *dir = -1;
  *type = -1;
  switch (insn->name[0])
    {
    case 'l':
      *type = 0;
      break;
    case 'a':
      *type = 1;
      break;
    default:
      as_fatal (_("Bad shift mode"));
      break;
    }

  switch (insn->name[2])
    {
    case 'l':
      *dir = 1;
      break;
    case 'r':
      *dir = 0;
      break;
    default:
      as_fatal (_("Bad shift *direction"));
      break;
    }
}

/* Shift instruction with a OPR operand */
static bool
shift_two_operand  (const struct instruction *insn)
{
  uint8_t sb = 0x34;
  char *ilp = input_line_pointer;

  short dir = -1;
  short type = -1;
  impute_shift_dir_and_type (insn, &type, &dir);
  sb |= dir << 6;
  sb |= type << 7;

  int size = size_from_suffix (insn, 0);
  sb |= size - 1;

  uint8_t buffer[4];
  int n_opr_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_opr_bytes, &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  long imm = -1;
  if (!lex_imm (&imm, NULL))
    goto fail;

  if (imm != 1 && imm != 2)
    goto fail;

  if (imm == 2)
    sb |= 0x08;

  char *f = s12z_new_insn (2 + n_opr_bytes);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, sb, 1);
  emit_opr (f, buffer, n_opr_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

/* Shift instruction with a OPR operand */
static bool
shift_opr_imm  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  short dir = -1;
  short type = -1;
  impute_shift_dir_and_type (insn, &type, &dir);

  int Dd = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int n_bytes = 2;

  uint8_t buffer1[4];
  int n_opr_bytes1;

  expressionS exp1;
  if (!lex_opr (buffer1, &n_opr_bytes1, &exp1, false))
    goto fail;

  n_bytes += n_opr_bytes1;
  if (!lex_match (','))
    goto fail;

  uint8_t buffer2[4];
  int n_opr_bytes2 = 0;
  expressionS exp2;
  long imm;
  bool immediate = false;
  if (lex_imm (&imm, NULL))
    {
      immediate = true;
    }
  else if (!lex_opr (buffer2, &n_opr_bytes2, &exp2, false))
    goto fail;

  uint8_t sb = 0x20;

  int size = size_from_suffix (insn, 0);

  if (size != -1)
    sb |= size - 1;

  sb |= dir << 6;
  sb |= type << 7;

  if (immediate)
    {
      if (imm == 2 || imm == 1)
	{
	  if (imm == 2)
	    sb |= 0x08;
	}
      else
	{
	  n_bytes++;
	  sb |= 0x10;
	  if (imm % 2)
	    sb |= 0x08;
	}
    }
  else
    {
      n_bytes += n_opr_bytes2;
      sb |= 0x10;
    }

  char *f = s12z_new_insn (n_bytes);
  number_to_chars_bigendian (f++, insn->opc | Dd, 1);
  number_to_chars_bigendian (f++, sb, 1);
  f = emit_opr (f, buffer1, n_opr_bytes1, &exp1);
  if (immediate)
    {
      if (imm != 1 && imm != 2)
	{
	  number_to_chars_bigendian (f++, 0x70 | (imm >> 1), 1);
	}
    }
  else
    {
      f = emit_opr (f, buffer2, n_opr_bytes2, &exp2);
    }

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

/* Shift instruction with a register operand */
static bool
shift_reg  (const struct instruction *insn)
{
  short dir = -1;
  short type = -1;
  impute_shift_dir_and_type (insn, &type, &dir);

  if (lex_shift_reg_imm1 (insn, type, dir))
    return true;

  return lex_shift_reg (insn, type, dir);
}

static bool
bm_regd_imm  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;
  int Di = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Di))
    goto fail;

  if (!lex_match (','))
    goto fail;

  long imm;
  if (!lex_imm (&imm, NULL))
    goto fail;


  uint8_t bm = imm << 3;
  bm |= Di;

  char *f = s12z_new_insn (2);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, bm, 1);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
bm_opr_reg  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  uint8_t buffer[4];
  int n_opr_bytes;

  expressionS exp;
  if (!lex_opr (buffer, &n_opr_bytes,  &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dn = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dn))
    goto fail;

  uint8_t bm = Dn << 4;
  int size = size_from_suffix (insn, 0);
  bm |= (size - 1) << 2;
  bm |= 0x81;

  char *f = s12z_new_insn (2 + n_opr_bytes);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, bm, 1);

  emit_opr (f, buffer, n_opr_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
bm_opr_imm  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  uint8_t buffer[4];
  int n_opr_bytes;

  expressionS exp;
  if (!lex_opr (buffer, &n_opr_bytes, &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;


  long imm;
  if (!lex_imm (&imm, NULL))
    goto fail;

  int size = size_from_suffix (insn, 0);

  if (imm < 0 || imm >= size * 8)
    {
      as_bad (_("Immediate operand %ld is inappropriate for size of instruction"), imm);
      goto fail;
    }

  uint8_t bm = 0x80;
  if (size == 2)
    bm |= 0x02;
  else if (size == 4)
    bm |= 0x08;
  bm |= (imm & 0x07) << 4;
  bm |= (imm >> 3);


  char *f = s12z_new_insn (2 + n_opr_bytes);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, bm, 1);
  emit_opr (f, buffer, n_opr_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
bm_regd_reg  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;
  int Di = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Di))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dn = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dn))
    goto fail;

  uint8_t bm = Dn << 4;
  bm |= 0x81;

  uint8_t xb = Di | 0xb8;

  char *f = s12z_new_insn (3);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, bm, 1);
  number_to_chars_bigendian (f++, xb, 1);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}





static bool
bf_reg_opr_imm  (const struct instruction *insn, short ie)
{
  char *ilp = input_line_pointer;
  int Dd = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;

  uint8_t buffer[4];
  int n_bytes;

  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  long width;
  if (!lex_imm (&width, NULL))
    goto fail;

  if (width < 0 || width > 31)
    {
      as_bad (_("Invalid width value for %s"), insn->name);
      goto fail;
    }

  if (!lex_match (':'))
    goto fail;

  long offset;
  if (!lex_constant (&offset))
    goto fail;

  if (offset < 0 || offset > 31)
    {
      as_bad (_("Invalid offset value for %s"), insn->name);
      goto fail;
    }

  uint8_t i1 = width << 5;
  i1 |= offset;

  int size = size_from_suffix (insn, 0);
  uint8_t bb = ie ? 0x80 : 0x00;
  bb |= 0x60;
  bb |= (size - 1) << 2;
  bb |= width >> 3;

  char *f = s12z_new_insn (4 + n_bytes);
  number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);
  number_to_chars_bigendian (f++, 0x08 | Dd, 1);
  number_to_chars_bigendian (f++, bb, 1);
  number_to_chars_bigendian (f++, i1, 1);

  emit_opr (f, buffer, n_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
bf_opr_reg_imm  (const struct instruction *insn, short ie)
{
  char *ilp = input_line_pointer;
  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Ds = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Ds))
    goto fail;

  if (!lex_match (','))
    goto fail;

  long width;
  if (!lex_imm (&width, NULL))
    goto fail;

  if (width < 0 || width > 31)
    {
      as_bad (_("Invalid width value for %s"), insn->name);
      goto fail;
    }

  if (!lex_match (':'))
    goto fail;

  long offset;
  if (!lex_constant (&offset))
    goto fail;

  if (offset < 0 || offset > 31)
    {
      as_bad (_("Invalid offset value for %s"), insn->name);
      goto fail;
    }

  uint8_t i1 = width << 5;
  i1 |= offset;

  int size = size_from_suffix (insn, 0);
  uint8_t bb = ie ? 0x80 : 0x00;
  bb |= 0x70;
  bb |= (size - 1) << 2;
  bb |= width >> 3;

  char *f = s12z_new_insn (4 + n_bytes);
  number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);
  number_to_chars_bigendian (f++, 0x08 | Ds, 1);
  number_to_chars_bigendian (f++, bb, 1);
  number_to_chars_bigendian (f++, i1, 1);

  emit_opr (f, buffer, n_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}



static bool
bf_reg_reg_imm  (const struct instruction *insn, short ie)
{
  char *ilp = input_line_pointer;
  int Dd = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Ds = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Ds))
    goto fail;

  if (!lex_match (','))
    goto fail;

  long width;
  if (!lex_imm (&width, NULL))
    goto fail;

  if (width < 0 || width > 31)
    {
      as_bad (_("Invalid width value for %s"), insn->name);
      goto fail;
    }

  if (!lex_match (':'))
    goto fail;

  long offset;
  if (!lex_constant (&offset))
    goto fail;

  if (offset < 0 || offset > 31)
    {
      as_bad (_("Invalid offset value for %s"), insn->name);
      goto fail;
    }

  uint8_t bb = ie ? 0x80 : 0x00;
  bb |= 0x20;
  bb |= Ds << 2;
  bb |= width >> 3;

  uint8_t i1 = width << 5;
  i1 |= offset;

  char *f = s12z_new_insn (4);
  number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);
  number_to_chars_bigendian (f++, 0x08 | Dd, 1);
  number_to_chars_bigendian (f++, bb, 1);
  number_to_chars_bigendian (f++, i1, 1);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
bf_reg_reg_reg  (const struct instruction *insn ATTRIBUTE_UNUSED, short ie)
{
  char *ilp = input_line_pointer;
  int Dd = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Ds = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Ds))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dp = 0;
  if (!lex_reg_name  ((0x01u << REG_D2) |
		      (0x01u << REG_D3) |
		      (0x01u << REG_D4) |
		      (0x01u << REG_D5),
		      &Dp))
    goto fail;

  uint8_t bb = ie ? 0x80 : 0x00;
  bb |= Ds << 2;
  bb |= Dp;

  char *f = s12z_new_insn (3);
  number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);
  number_to_chars_bigendian (f++, 0x08 | Dd, 1);
  number_to_chars_bigendian (f++, bb , 1);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
bf_opr_reg_reg  (const struct instruction *insn, short ie)
{
  char *ilp = input_line_pointer;

  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;


  int Ds = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Ds))
    goto fail;

  if (!lex_match (','))
    goto fail;


  int Dp = 0;
  if (!lex_reg_name  ((0x01u << REG_D2) |
		      (0x01u << REG_D3) |
		      (0x01u << REG_D4) |
		      (0x01u << REG_D5),
		      &Dp))
    goto fail;

  int size = size_from_suffix (insn, 0);
  uint8_t bb = ie ? 0x80 : 0x00;
  bb |= 0x50;
  bb |= Dp;
  bb |= (size - 1) << 2;

  char *f = s12z_new_insn (3 + n_bytes);
  number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);
  number_to_chars_bigendian (f++, 0x08 | Ds, 1);
  number_to_chars_bigendian (f++, bb , 1);

  emit_opr (f, buffer, n_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
bf_reg_opr_reg  (const struct instruction *insn, short ie)
{
  char *ilp = input_line_pointer;
  int Dd = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dd))
    goto fail;

  if (!lex_match (','))
    goto fail;


  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dp = 0;
  if (!lex_reg_name  ((0x01u << REG_D2) |
		      (0x01u << REG_D3) |
		      (0x01u << REG_D4) |
		      (0x01u << REG_D5),
		      &Dp))
    goto fail;

  int size = size_from_suffix (insn, 0);
  uint8_t bb = ie ? 0x80 : 0x00;
  bb |= 0x40;
  bb |= Dp;
  bb |= (size - 1) << 2;

  char *f = s12z_new_insn (3 + n_bytes);
  number_to_chars_bigendian (f++, PAGE2_PREBYTE, 1);
  number_to_chars_bigendian (f++, 0x08 | Dd, 1);
  number_to_chars_bigendian (f++, bb , 1);

  emit_opr (f, buffer, n_bytes, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}



static bool
bfe_reg_reg_reg  (const struct instruction *insn)
{
  return bf_reg_reg_reg (insn, 0);
}

static bool
bfi_reg_reg_reg  (const struct instruction *insn)
{
  return bf_reg_reg_reg (insn, 1);
}

static bool
bfe_reg_reg_imm  (const struct instruction *insn)
{
  return bf_reg_reg_imm (insn, 0);
}

static bool
bfi_reg_reg_imm  (const struct instruction *insn)
{
  return bf_reg_reg_imm (insn, 1);
}


static bool
bfe_reg_opr_reg  (const struct instruction *insn)
{
  return bf_reg_opr_reg (insn, 0);
}

static bool
bfi_reg_opr_reg  (const struct instruction *insn)
{
  return bf_reg_opr_reg (insn, 1);
}


static bool
bfe_opr_reg_reg  (const struct instruction *insn)
{
  return bf_opr_reg_reg (insn, 0);
}

static bool
bfi_opr_reg_reg  (const struct instruction *insn)
{
  return bf_opr_reg_reg (insn, 1);
}

static bool
bfe_reg_opr_imm  (const struct instruction *insn)
{
  return bf_reg_opr_imm (insn, 0);
}

static bool
bfi_reg_opr_imm  (const struct instruction *insn)
{
  return bf_reg_opr_imm (insn, 1);
}

static bool
bfe_opr_reg_imm  (const struct instruction *insn)
{
  return bf_opr_reg_imm (insn, 0);
}

static bool
bfi_opr_reg_imm  (const struct instruction *insn)
{
  return bf_opr_reg_imm (insn, 1);
}




static bool
tb_reg_rel  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  int reg;
  if (!lex_reg_name (REG_BIT_Dn | REG_BIT_XY, &reg))
    goto fail;

  if (!lex_match (','))
    goto fail;

  bool long_displacement;
  expressionS exp;
  if (! lex_15_bit_offset (&long_displacement, &exp))
    goto fail;

  uint8_t lb = 0x00;
  if (reg == REG_X || reg == REG_Y)
    {
      lb |= 0x08;
    }
  else
    {
      lb |= reg;
    }
  if (reg == REG_Y)
    lb |= 0x01;

  if (startswith (insn->name + 2, "ne"))
    lb |= 0x00 << 4;
  else if (startswith (insn->name + 2, "eq"))
    lb |= 0x01 << 4;
  else if (startswith (insn->name + 2, "pl"))
    lb |= 0x02 << 4;
  else if (startswith (insn->name + 2, "mi"))
    lb |= 0x03 << 4;
  else if (startswith (insn->name + 2, "gt"))
    lb |= 0x04 << 4;
  else if (startswith (insn->name + 2, "le"))
    lb |= 0x05 << 4;

  switch (insn->name[0])
    {
    case 'd':
      lb |= 0x80;
      break;
    case 't':
      break;
    default:
      gas_assert (0);
      break;
    };

  char *f = s12z_new_insn (long_displacement ? 4 : 3);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, lb, 1);

  emit_15_bit_offset (f, 4, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
tb_opr_rel  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  bool long_displacement;
  expressionS exp2;
  if (! lex_15_bit_offset (&long_displacement, &exp2))
    goto fail;

  uint8_t lb = 0x0C;

  if (startswith (insn->name + 2, "ne"))
    lb |= 0x00 << 4;
  else if (startswith (insn->name + 2, "eq"))
    lb |= 0x01 << 4;
  else if (startswith (insn->name + 2, "pl"))
    lb |= 0x02 << 4;
  else if (startswith (insn->name + 2, "mi"))
    lb |= 0x03 << 4;
  else if (startswith (insn->name + 2, "gt"))
    lb |= 0x04 << 4;
  else if (startswith (insn->name + 2, "le"))
    lb |= 0x05 << 4;

  switch (insn->name[0])
    {
    case 'd':
      lb |= 0x80;
      break;
    case 't':
      break;
    default:
      gas_assert (0);
      break;
    };

  int size = size_from_suffix (insn, 0);

  lb |= size -1;

  char *f = s12z_new_insn (n_bytes + (long_displacement ? 4 : 3));
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, lb, 1);
  f = emit_opr (f, buffer, n_bytes, &exp);

  emit_15_bit_offset (f, n_bytes + 4, &exp2);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}




static bool
test_br_reg_reg_rel  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  int Di = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Di))
    goto fail;

  if (!lex_match (','))
    goto fail;


  int Dn = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dn))
    goto fail;

  if (!lex_match (','))
    goto fail;


  bool long_displacement;
  expressionS exp;
  if (! lex_15_bit_offset (&long_displacement, &exp))
    goto fail;

  uint8_t bm = 0x81;
  uint8_t xb = 0xb8;

  bm |= Dn << 4;
  xb |= Di;

  char *f = s12z_new_insn (long_displacement ? 5 : 4);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, bm, 1);
  number_to_chars_bigendian (f++, xb, 1);

  emit_15_bit_offset (f, 5, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}

static bool
test_br_opr_reg_rel  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_bytes,  &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  int Dn = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Dn))
    goto fail;

  if (!lex_match (','))
    goto fail;

  uint8_t bm = 0x81;
  bm |= Dn << 4;
  int size = size_from_suffix (insn, 0);
  bm |= (size -1) << 2;

  bool long_displacement;

  expressionS exp2;
  if (! lex_15_bit_offset (&long_displacement, &exp2))
    goto fail;

  int n = n_bytes + (long_displacement ? 4 : 3);
  char *f = s12z_new_insn (n);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, bm, 1);
  f = emit_opr (f, buffer, n_bytes, &exp);

  emit_15_bit_offset (f, n, &exp2);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
test_br_opr_imm_rel  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  uint8_t buffer[4];
  int n_bytes;
  expressionS exp;
  if (!lex_opr (buffer, &n_bytes, &exp, false))
    goto fail;

  if (!lex_match (','))
    goto fail;

  long imm;
  if (!lex_imm (&imm, NULL))
    goto fail;

  if (imm < 0 || imm > 31)
    goto fail;

  if (!lex_match (','))
    goto fail;

  bool long_displacement;
  expressionS exp2;
  if (! lex_15_bit_offset (&long_displacement, &exp2))
    goto fail;

  int size = size_from_suffix (insn, 0);

  uint8_t bm = 0x80;
  bm |= (imm & 0x07) << 4;
  bm |= (imm >> 3) & 0x03;
  if (size == 4)
    bm |=  0x08;
  else if  (size == 2)
    bm |= 0x02;

  char *f = s12z_new_insn (n_bytes + (long_displacement ? 4 : 3));
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, bm, 1);
  f = emit_opr (f, buffer, n_bytes, &exp);

  emit_15_bit_offset (f, n_bytes + 4,  &exp2);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}


static bool
test_br_reg_imm_rel  (const struct instruction *insn)
{
  char *ilp = input_line_pointer;

  int Di = 0;
  if (!lex_reg_name (REG_BIT_Dn, &Di))
    goto fail;

  if (!lex_match (','))
    goto fail;

  long imm;
  if (!lex_imm (&imm, NULL))
    goto fail;

  if (imm < 0 || imm > 31)
    goto fail;


  if (!lex_match (','))
    goto fail;

  bool long_displacement;
  expressionS exp;
  if (! lex_15_bit_offset (&long_displacement, &exp))
    goto fail;

  uint8_t bm = Di;
  bm |= imm << 3;

  char *f = s12z_new_insn (long_displacement ? 4 : 3);
  number_to_chars_bigendian (f++, insn->opc, 1);
  number_to_chars_bigendian (f++, bm, 1);

  emit_15_bit_offset (f, 4, &exp);

  return true;

 fail:
  fail_line_pointer = input_line_pointer;
  input_line_pointer = ilp;
  return false;
}




static const struct instruction opcodes[] = {
  {"bgnd", 1,  0x00,  no_operands, 0},
  {"nop", 1,   0x01,  no_operands, 0},

  {"brclr", 1, 0x02,  test_br_reg_reg_rel, 0},
  {"brset", 1, 0x03,  test_br_reg_reg_rel, 0},

  {"brclr", 1, 0x02,  test_br_reg_imm_rel, 0},
  {"brset", 1, 0x03,  test_br_reg_imm_rel, 0},

  {"brclr.b", 1, 0x02, test_br_opr_reg_rel, 0},
  {"brclr.w", 1, 0x02, test_br_opr_reg_rel, 0},
  {"brclr.l", 1, 0x02, test_br_opr_reg_rel, 0},

  {"brset.b", 1, 0x03, test_br_opr_reg_rel, 0},
  {"brset.w", 1, 0x03, test_br_opr_reg_rel, 0},
  {"brset.l", 1, 0x03, test_br_opr_reg_rel, 0},

  {"brclr.b", 1, 0x02, test_br_opr_imm_rel, 0},
  {"brclr.w", 1, 0x02, test_br_opr_imm_rel, 0},
  {"brclr.l", 1, 0x02, test_br_opr_imm_rel, 0},

  {"brset.b", 1, 0x03, test_br_opr_imm_rel, 0},
  {"brset.w", 1, 0x03, test_br_opr_imm_rel, 0},
  {"brset.l", 1, 0x03, test_br_opr_imm_rel, 0},

  {"psh", 1,   0x04,  psh_pull, 0},
  {"pul", 1,   0x04,  psh_pull, 0},

  {"rts", 1,   0x05,  no_operands, 0},
  {"lea", 1,   0x06,  reg67sxy_opr, 0},

  {"dbne", 1,  0x0b,  tb_reg_rel, 0},
  {"dbeq", 1,  0x0b,  tb_reg_rel, 0},
  {"dbpl", 1,  0x0b,  tb_reg_rel, 0},
  {"dbmi", 1,  0x0b,  tb_reg_rel, 0},
  {"dbgt", 1,  0x0b,  tb_reg_rel, 0},
  {"dble", 1,  0x0b,  tb_reg_rel, 0},

  {"dbne.b", 1,  0x0b,  tb_opr_rel, 0},
  {"dbeq.b", 1,  0x0b,  tb_opr_rel, 0},
  {"dbpl.b", 1,  0x0b,  tb_opr_rel, 0},
  {"dbmi.b", 1,  0x0b,  tb_opr_rel, 0},
  {"dbgt.b", 1,  0x0b,  tb_opr_rel, 0},
  {"dble.b", 1,  0x0b,  tb_opr_rel, 0},

  {"dbne.w", 1,  0x0b,  tb_opr_rel, 0},
  {"dbeq.w", 1,  0x0b,  tb_opr_rel, 0},
  {"dbpl.w", 1,  0x0b,  tb_opr_rel, 0},
  {"dbmi.w", 1,  0x0b,  tb_opr_rel, 0},
  {"dbgt.w", 1,  0x0b,  tb_opr_rel, 0},
  {"dble.w", 1,  0x0b,  tb_opr_rel, 0},

  {"dbne.p", 1,  0x0b,  tb_opr_rel, 0},
  {"dbeq.p", 1,  0x0b,  tb_opr_rel, 0},
  {"dbpl.p", 1,  0x0b,  tb_opr_rel, 0},
  {"dbmi.p", 1,  0x0b,  tb_opr_rel, 0},
  {"dbgt.p", 1,  0x0b,  tb_opr_rel, 0},
  {"dble.p", 1,  0x0b,  tb_opr_rel, 0},

  {"dbne.l", 1,  0x0b,  tb_opr_rel, 0},
  {"dbeq.l", 1,  0x0b,  tb_opr_rel, 0},
  {"dbpl.l", 1,  0x0b,  tb_opr_rel, 0},
  {"dbmi.l", 1,  0x0b,  tb_opr_rel, 0},
  {"dbgt.l", 1,  0x0b,  tb_opr_rel, 0},
  {"dble.l", 1,  0x0b,  tb_opr_rel, 0},

  {"tbne", 1,  0x0b,  tb_reg_rel, 0},
  {"tbeq", 1,  0x0b,  tb_reg_rel, 0},
  {"tbpl", 1,  0x0b,  tb_reg_rel, 0},
  {"tbmi", 1,  0x0b,  tb_reg_rel, 0},
  {"tbgt", 1,  0x0b,  tb_reg_rel, 0},
  {"tble", 1,  0x0b,  tb_reg_rel, 0},

  {"tbne.b", 1,  0x0b,  tb_opr_rel, 0},
  {"tbeq.b", 1,  0x0b,  tb_opr_rel, 0},
  {"tbpl.b", 1,  0x0b,  tb_opr_rel, 0},
  {"tbmi.b", 1,  0x0b,  tb_opr_rel, 0},
  {"tbgt.b", 1,  0x0b,  tb_opr_rel, 0},
  {"tble.b", 1,  0x0b,  tb_opr_rel, 0},

  {"tbne.w", 1,  0x0b,  tb_opr_rel, 0},
  {"tbeq.w", 1,  0x0b,  tb_opr_rel, 0},
  {"tbpl.w", 1,  0x0b,  tb_opr_rel, 0},
  {"tbmi.w", 1,  0x0b,  tb_opr_rel, 0},
  {"tbgt.w", 1,  0x0b,  tb_opr_rel, 0},
  {"tble.w", 1,  0x0b,  tb_opr_rel, 0},

  {"tbne.p", 1,  0x0b,  tb_opr_rel, 0},
  {"tbeq.p", 1,  0x0b,  tb_opr_rel, 0},
  {"tbpl.p", 1,  0x0b,  tb_opr_rel, 0},
  {"tbmi.p", 1,  0x0b,  tb_opr_rel, 0},
  {"tbgt.p", 1,  0x0b,  tb_opr_rel, 0},
  {"tble.p", 1,  0x0b,  tb_opr_rel, 0},

  {"tbne.l", 1,  0x0b,  tb_opr_rel, 0},
  {"tbeq.l", 1,  0x0b,  tb_opr_rel, 0},
  {"tbpl.l", 1,  0x0b,  tb_opr_rel, 0},
  {"tbmi.l", 1,  0x0b,  tb_opr_rel, 0},
  {"tbgt.l", 1,  0x0b,  tb_opr_rel, 0},
  {"tble.l", 1,  0x0b,  tb_opr_rel, 0},

  {"mov.b", 1, 0x0c,  imm_opr, 0},
  {"mov.w", 1, 0x0d,  imm_opr, 0},
  {"mov.p", 1, 0x0e,  imm_opr, 0},
  {"mov.l", 1, 0x0f,  imm_opr, 0},

  {"rol",   1, 0x10,  rol, 0},
  {"rol.b", 1, 0x10,  rol, 0},
  {"rol.w", 1, 0x10,  rol, 0},
  {"rol.p", 1, 0x10,  rol, 0},
  {"rol.l", 1, 0x10,  rol, 0},

  {"ror",   1, 0x10,  ror, 0},
  {"ror.b", 1, 0x10,  ror, 0},
  {"ror.w", 1, 0x10,  ror, 0},
  {"ror.p", 1, 0x10,  ror, 0},
  {"ror.l", 1, 0x10,  ror, 0},

  {"lsl", 1,   0x10,  shift_reg, 0},
  {"lsr", 1,   0x10,  shift_reg, 0},
  {"asl", 1,   0x10,  shift_reg, 0},
  {"asr", 1,   0x10,  shift_reg, 0},

  {"lsl.b", 1, 0x10,  shift_two_operand, 0},
  {"lsl.w", 1, 0x10,  shift_two_operand, 0},
  {"lsl.p", 1, 0x10,  shift_two_operand, 0},
  {"lsl.l", 1, 0x10,  shift_two_operand, 0},
  {"asl.b", 1, 0x10,  shift_two_operand, 0},
  {"asl.w", 1, 0x10,  shift_two_operand, 0},
  {"asl.p", 1, 0x10,  shift_two_operand, 0},
  {"asl.l", 1, 0x10,  shift_two_operand, 0},

  {"lsr.b", 1, 0x10,  shift_two_operand, 0},
  {"lsr.w", 1, 0x10,  shift_two_operand, 0},
  {"lsr.p", 1, 0x10,  shift_two_operand, 0},
  {"lsr.l", 1, 0x10,  shift_two_operand, 0},
  {"asr.b", 1, 0x10,  shift_two_operand, 0},
  {"asr.w", 1, 0x10,  shift_two_operand, 0},
  {"asr.p", 1, 0x10,  shift_two_operand, 0},
  {"asr.l", 1, 0x10,  shift_two_operand, 0},

  {"lsl.b", 1, 0x10,  shift_opr_imm, 0},
  {"lsl.w", 1, 0x10,  shift_opr_imm, 0},
  {"lsl.p", 1, 0x10,  shift_opr_imm, 0},
  {"lsl.l", 1, 0x10,  shift_opr_imm, 0},
  {"asl.b", 1, 0x10,  shift_opr_imm, 0},
  {"asl.w", 1, 0x10,  shift_opr_imm, 0},
  {"asl.p", 1, 0x10,  shift_opr_imm, 0},
  {"asl.l", 1, 0x10,  shift_opr_imm, 0},

  {"lsr.b", 1, 0x10,  shift_opr_imm, 0},
  {"lsr.w", 1, 0x10,  shift_opr_imm, 0},
  {"lsr.p", 1, 0x10,  shift_opr_imm, 0},
  {"lsr.l", 1, 0x10,  shift_opr_imm, 0},
  {"asr.b", 1, 0x10,  shift_opr_imm, 0},
  {"asr.w", 1, 0x10,  shift_opr_imm, 0},
  {"asr.p", 1, 0x10,  shift_opr_imm, 0},
  {"asr.l", 1, 0x10,  shift_opr_imm, 0},

  {"mov.b", 1, 0x1c,  opr_opr, 0},
  {"mov.w", 1, 0x1d,  opr_opr, 0},
  {"mov.p", 1, 0x1e,  opr_opr, 0},
  {"mov.l", 1, 0x1f,  opr_opr, 0},

  {"bra", 1,   0x20,  rel, 0},
  {"bsr", 1,   0x21,  rel, 0},
  {"bhi", 1,   0x22,  rel, 0},
  {"bls", 1,   0x23,  rel, 0},
  {"bcc", 1,   0x24,  rel, 0},
  {"bhs", 1,   0x24,  rel, 0}, /* Alias for bcc */
  {"bcs", 1,   0x25,  rel, 0},
  {"blo", 1,   0x25,  rel, 0}, /* Alias for bcs */
  {"bne", 1,   0x26,  rel, 0},
  {"beq", 1,   0x27,  rel, 0},
  {"bvc", 1,   0x28,  rel, 0},
  {"bvs", 1,   0x29,  rel, 0},
  {"bpl", 1,   0x2a,  rel, 0},
  {"bmi", 1,   0x2b,  rel, 0},
  {"bge", 1,   0x2c,  rel, 0},
  {"blt", 1,   0x2d,  rel, 0},
  {"bgt", 1,   0x2e,  rel, 0},
  {"ble", 1,   0x2f,  rel, 0},

  {"inc", 1,   0x30,  reg_inh, 0},
  {"clr", 1,   0x38,  reg_inh, 0},
  {"dec", 1,   0x40,  reg_inh, 0},

  {"muls", 1,  0x48,  mul_reg_reg_reg, 0},
  {"mulu", 1,  0x48,  mul_reg_reg_reg, 0},

  {"muls.b", 1,  0x48,  mul_reg_reg_opr, 0},
  {"muls.w", 1,  0x48,  mul_reg_reg_opr, 0},
  {"muls.l", 1,  0x48,  mul_reg_reg_opr, 0},

  {"mulu.b", 1,  0x48,  mul_reg_reg_opr, 0},
  {"mulu.w", 1,  0x48,  mul_reg_reg_opr, 0},
  {"mulu.l", 1,  0x48,  mul_reg_reg_opr, 0},

  {"muls.b", 1,  0x48,  mul_reg_reg_imm, 0},
  {"muls.w", 1,  0x48,  mul_reg_reg_imm, 0},
  {"muls.l", 1,  0x48,  mul_reg_reg_imm, 0},

  {"mulu.b", 1,  0x48,  mul_reg_reg_imm, 0},
  {"mulu.w", 1,  0x48,  mul_reg_reg_imm, 0},
  {"mulu.l", 1,  0x48,  mul_reg_reg_imm, 0},

  {"muls.bb", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.bw", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.bp", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.bl", 1,  0x48,  mul_reg_opr_opr, 0},

  {"muls.wb", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.ww", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.wp", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.wl", 1,  0x48,  mul_reg_opr_opr, 0},

  {"muls.pb", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.pw", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.pp", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.pl", 1,  0x48,  mul_reg_opr_opr, 0},

  {"muls.lb", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.lw", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.lp", 1,  0x48,  mul_reg_opr_opr, 0},
  {"muls.ll", 1,  0x48,  mul_reg_opr_opr, 0},

  {"mulu.bb", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.bw", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.bp", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.bl", 1,  0x48,  mul_reg_opr_opr, 0},

  {"mulu.wb", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.ww", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.wp", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.wl", 1,  0x48,  mul_reg_opr_opr, 0},

  {"mulu.pb", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.pw", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.pp", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.pl", 1,  0x48,  mul_reg_opr_opr, 0},

  {"mulu.lb", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.lw", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.lp", 1,  0x48,  mul_reg_opr_opr, 0},
  {"mulu.ll", 1,  0x48,  mul_reg_opr_opr, 0},

  {"add", 1,   0x50,  regd_imm, 0},
  {"and", 1,   0x58,  regd_imm, 0},

  {"add", 1,   0x60,  regd_opr, 0},
  {"and", 1,   0x68,  regd_opr, 0},

  {"sub", 1,   0x70,  regd_imm, 0},
  {"or", 1,    0x78,  regd_imm, 0},

  {"sub", 1,   0x80,  regd_opr, 0},
  {"or",  1,    0x88,  regd_opr, 0},

  {"ld",  1,    0x90,  regdxy_imm, 0},

  {"clr", 1,   0x9a,  clr_xy, 0},
  {"tfr", 1,   0x9e,  tfr, 0},
  {"zex", 1,   0x9e,  tfr, 0},

  {"ld",  1,   0xa0,  regdxy_opr_src, 0xb0},

  {"jmp", 1,   0xaa,  opr, 0xba},
  {"jsr", 1,   0xab,  opr, 0xbb},

  {"exg", 1,   0xae,  tfr, 0},
  {"sex", 1,   0xae,  tfr, 0},

  {"st", 1,    0xc0,  regdxy_opr_dest, 0xd0},

  {"andcc", 1, 0xce,  imm8, 0},
  {"orcc", 1,  0xde,  imm8, 0},

  {"inc.b", 1, 0x9c,  opr, 0},
  {"inc.w", 1, 0x9d,  opr, 0},
  {"inc.l", 1, 0x9f,  opr, 0},

  {"dec.b", 1, 0xac,  opr, 0},
  {"dec.w", 1, 0xad,  opr, 0},
  {"dec.l", 1, 0xaf,  opr, 0},

  {"clr.b", 1, 0xbc,  opr, 0},
  {"clr.w", 1, 0xbd,  opr, 0},
  {"clr.p", 1, 0xbe,  opr, 0},
  {"clr.l", 1, 0xbf,  opr, 0},

  {"com.b", 1, 0xcc,  opr, 0},
  {"com.w", 1, 0xcd,  opr, 0},
  {"com.l", 1, 0xcf,  opr, 0},

  {"neg.b", 1, 0xdc,  opr, 0},
  {"neg.w", 1, 0xdd,  opr, 0},
  {"neg.l", 1, 0xdf,  opr, 0},

  {"bclr",  1, 0xec, bm_regd_imm, 0},
  {"bset",  1, 0xed, bm_regd_imm, 0},
  {"btgl",  1, 0xee, bm_regd_imm, 0},

  {"bclr",  1, 0xec, bm_regd_reg, 0},
  {"bset",  1, 0xed, bm_regd_reg, 0},
  {"btgl",  1, 0xee, bm_regd_reg, 0},

  {"bclr.b",  1, 0xec, bm_opr_imm, 0},
  {"bclr.w",  1, 0xec, bm_opr_imm, 0},
  {"bclr.l",  1, 0xec, bm_opr_imm, 0},

  {"bset.b",  1, 0xed, bm_opr_imm, 0},
  {"bset.w",  1, 0xed, bm_opr_imm, 0},
  {"bset.l",  1, 0xed, bm_opr_imm, 0},

  {"btgl.b",  1, 0xee, bm_opr_imm, 0},
  {"btgl.w",  1, 0xee, bm_opr_imm, 0},
  {"btgl.l",  1, 0xee, bm_opr_imm, 0},

  {"bclr.b",  1, 0xec, bm_opr_reg, 0},
  {"bclr.w",  1, 0xec, bm_opr_reg, 0},
  {"bclr.l",  1, 0xec, bm_opr_reg, 0},

  {"bset.b",  1, 0xed, bm_opr_reg, 0},
  {"bset.w",  1, 0xed, bm_opr_reg, 0},
  {"bset.l",  1, 0xed, bm_opr_reg, 0},

  {"btgl.b",  1, 0xee, bm_opr_reg, 0},
  {"btgl.w",  1, 0xee, bm_opr_reg, 0},
  {"btgl.l",  1, 0xee, bm_opr_reg, 0},

  {"cmp", 1,   0xe0,  regdxy_imm, 0},
  {"cmp", 1,   0xf0,  regdxy_opr_src, 0},

  {"cmp", 1,   0xfc,  regx_regy, 0},
  {"sub", 1,   0xfd,  regd6_regx_regy, 0},
  {"sub", 1,   0xfe,  regd6_regy_regx, 0},

  {"swi", 1,   0xff,  no_operands, 0},

  /* Page 2 */

  /* The -10 below is a kludge.  The opcode is in fact 0x00 */
  {"ld",    2,  -10,  regs_opr_src, 0},

  /* The -9 below is a kludge.  The opcode is in fact 0x01 */
  {"st",    2,  -9,  regs_opr_dest, 0},

  /* The -8 below is a kludge.  The opcode is in fact 0x02 */
  {"cmp",    2,  -8,  regs_opr_src, 0},

  /* The -7 below is a kludge.  The opcode is in fact 0x03 */
  {"ld",    2,  -7,  regs_imm, 0},

  /* The -6 below is a kludge.  The opcode is in fact 0x04 */
  {"cmp",    2,  -6,  regs_imm, 0},

  {"bfext",   2,  0x08,  bfe_reg_reg_reg, 0},
  {"bfext",   2,  0x08,  bfe_reg_reg_imm, 0},
  {"bfext.b", 2,  0x08,  bfe_reg_opr_reg, 0},
  {"bfext.w", 2,  0x08,  bfe_reg_opr_reg, 0},
  {"bfext.p", 2,  0x08,  bfe_reg_opr_reg, 0},
  {"bfext.l", 2,  0x08,  bfe_reg_opr_reg, 0},
  {"bfext.b", 2,  0x08,  bfe_opr_reg_reg, 0},
  {"bfext.w", 2,  0x08,  bfe_opr_reg_reg, 0},
  {"bfext.p", 2,  0x08,  bfe_opr_reg_reg, 0},
  {"bfext.l", 2,  0x08,  bfe_opr_reg_reg, 0},
  {"bfext.b", 2,  0x08,  bfe_reg_opr_imm, 0},
  {"bfext.w", 2,  0x08,  bfe_reg_opr_imm, 0},
  {"bfext.p", 2,  0x08,  bfe_reg_opr_imm, 0},
  {"bfext.l", 2,  0x08,  bfe_reg_opr_imm, 0},
  {"bfext.b", 2,  0x08,  bfe_opr_reg_imm, 0},
  {"bfext.w", 2,  0x08,  bfe_opr_reg_imm, 0},
  {"bfext.p", 2,  0x08,  bfe_opr_reg_imm, 0},
  {"bfext.l", 2,  0x08,  bfe_opr_reg_imm, 0},


  {"bfins",   2,  0x08,  bfi_reg_reg_reg, 0},
  {"bfins",   2,  0x08,  bfi_reg_reg_imm, 0},
  {"bfins.b", 2,  0x08,  bfi_reg_opr_reg, 0},
  {"bfins.w", 2,  0x08,  bfi_reg_opr_reg, 0},
  {"bfins.p", 2,  0x08,  bfi_reg_opr_reg, 0},
  {"bfins.l", 2,  0x08,  bfi_reg_opr_reg, 0},
  {"bfins.b", 2,  0x08,  bfi_opr_reg_reg, 0},
  {"bfins.w", 2,  0x08,  bfi_opr_reg_reg, 0},
  {"bfins.p", 2,  0x08,  bfi_opr_reg_reg, 0},
  {"bfins.l", 2,  0x08,  bfi_opr_reg_reg, 0},
  {"bfins.b", 2,  0x08,  bfi_reg_opr_imm, 0},
  {"bfins.w", 2,  0x08,  bfi_reg_opr_imm, 0},
  {"bfins.p", 2,  0x08,  bfi_reg_opr_imm, 0},
  {"bfins.l", 2,  0x08,  bfi_reg_opr_imm, 0},
  {"bfins.b", 2,  0x08,  bfi_opr_reg_imm, 0},
  {"bfins.w", 2,  0x08,  bfi_opr_reg_imm, 0},
  {"bfins.p", 2,  0x08,  bfi_opr_reg_imm, 0},
  {"bfins.l", 2,  0x08,  bfi_opr_reg_imm, 0},


  {"minu",  2,  0x10,  regd_opr, 0},
  {"maxu",  2,  0x18,  regd_opr, 0},
  {"mins",  2,  0x20,  regd_opr, 0},
  {"maxs",  2,  0x28,  regd_opr, 0},

  {"clb",   2,  0x91,  tfr, 0},

  {"trap",  2,  0x00, trap_imm, 0},
  {"abs",   2,  0x40, reg_inh, 0},
  {"sat",   2,  0xa0, reg_inh, 0},

  {"rti",   2,  0x90, no_operands, 0},
  {"stop",  2,  0x05, no_operands, 0},
  {"wai",   2,  0x06, no_operands, 0},
  {"sys",   2,  0x07, no_operands, 0},

  {"bit",   2,   0x58,  regd_imm, 0},
  {"bit",   2,   0x68,  regd_opr, 0},

  {"adc",   2,   0x50,  regd_imm, 0},
  {"adc",   2,   0x60,  regd_opr, 0},

  {"sbc",   2,   0x70,  regd_imm, 0},
  {"eor",   2,   0x78,  regd_imm, 0},

  {"sbc",   2,   0x80,  regd_opr, 0},
  {"eor",   2,   0x88,  regd_opr, 0},

  {"divs",   2,  0x30,  mul_reg_reg_reg, 0},
  {"divu",   2,  0x30,  mul_reg_reg_reg, 0},

  {"divs.b", 2,  0x30,  mul_reg_reg_opr, 0},
  {"divs.w", 2,  0x30,  mul_reg_reg_opr, 0},
  {"divs.l", 2,  0x30,  mul_reg_reg_opr, 0},

  {"divu.b", 2,  0x30,  mul_reg_reg_opr, 0},
  {"divu.w", 2,  0x30,  mul_reg_reg_opr, 0},
  {"divu.l", 2,  0x30,  mul_reg_reg_opr, 0},

  {"divs.b", 2,  0x30,  mul_reg_reg_imm, 0},
  {"divs.w", 2,  0x30,  mul_reg_reg_imm, 0},
  {"divs.l", 2,  0x30,  mul_reg_reg_imm, 0},

  {"divu.b", 2,  0x30,  mul_reg_reg_imm, 0},
  {"divu.w", 2,  0x30,  mul_reg_reg_imm, 0},
  {"divu.l", 2,  0x30,  mul_reg_reg_imm, 0},

  {"divs.bb", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.bw", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.bp", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.bl", 2,  0x30,  mul_reg_opr_opr, 0},

  {"divs.wb", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.ww", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.wp", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.wl", 2,  0x30,  mul_reg_opr_opr, 0},

  {"divs.pb", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.pw", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.pp", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.pl", 2,  0x30,  mul_reg_opr_opr, 0},

  {"divs.lb", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.lw", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.lp", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divs.ll", 2,  0x30,  mul_reg_opr_opr, 0},

  {"divu.bb", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.bw", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.bp", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.bl", 2,  0x30,  mul_reg_opr_opr, 0},

  {"divu.wb", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.ww", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.wp", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.wl", 2,  0x30,  mul_reg_opr_opr, 0},

  {"divu.pb", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.pw", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.pp", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.pl", 2,  0x30,  mul_reg_opr_opr, 0},

  {"divu.lb", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.lw", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.lp", 2,  0x30,  mul_reg_opr_opr, 0},
  {"divu.ll", 2,  0x30,  mul_reg_opr_opr, 0},

  {"qmuls",   2,  0xb0,  mul_reg_reg_reg, 0},
  {"qmulu",   2,  0xb0,  mul_reg_reg_reg, 0},

  {"qmuls.b", 2,  0xb0,  mul_reg_reg_opr, 0},
  {"qmuls.w", 2,  0xb0,  mul_reg_reg_opr, 0},
  {"qmuls.l", 2,  0xb0,  mul_reg_reg_opr, 0},

  {"qmulu.b", 2,  0xb0,  mul_reg_reg_opr, 0},
  {"qmulu.w", 2,  0xb0,  mul_reg_reg_opr, 0},
  {"qmulu.l", 2,  0xb0,  mul_reg_reg_opr, 0},

  {"qmuls.b", 2,  0xb0,  mul_reg_reg_imm, 0},
  {"qmuls.w", 2,  0xb0,  mul_reg_reg_imm, 0},
  {"qmuls.l", 2,  0xb0,  mul_reg_reg_imm, 0},

  {"qmulu.b", 2,  0xb0,  mul_reg_reg_imm, 0},
  {"qmulu.w", 2,  0xb0,  mul_reg_reg_imm, 0},
  {"qmulu.l", 2,  0xb0,  mul_reg_reg_imm, 0},

  {"qmuls.bb", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.bw", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.bp", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.bl", 2,  0xb0,  mul_reg_opr_opr, 0},

  {"qmuls.wb", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.ww", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.wp", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.wl", 2,  0xb0,  mul_reg_opr_opr, 0},

  {"qmuls.pb", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.pw", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.pp", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.pl", 2,  0xb0,  mul_reg_opr_opr, 0},

  {"qmuls.lb", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.lw", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.lp", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmuls.ll", 2,  0xb0,  mul_reg_opr_opr, 0},

  {"qmulu.bb", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.bw", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.bp", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.bl", 2,  0xb0,  mul_reg_opr_opr, 0},

  {"qmulu.wb", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.ww", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.wp", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.wl", 2,  0xb0,  mul_reg_opr_opr, 0},

  {"qmulu.pb", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.pw", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.pp", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.pl", 2,  0xb0,  mul_reg_opr_opr, 0},

  {"qmulu.lb", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.lw", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.lp", 2,  0xb0,  mul_reg_opr_opr, 0},
  {"qmulu.ll", 2,  0xb0,  mul_reg_opr_opr, 0},

  {"macs",   2,  0x48,  mul_reg_reg_reg, 0},
  {"macu",   2,  0x48,  mul_reg_reg_reg, 0},

  {"macs.b", 2,  0x48,  mul_reg_reg_opr, 0},
  {"macs.w", 2,  0x48,  mul_reg_reg_opr, 0},
  {"macs.l", 2,  0x48,  mul_reg_reg_opr, 0},

  {"macu.b", 2,  0x48,  mul_reg_reg_opr, 0},
  {"macu.w", 2,  0x48,  mul_reg_reg_opr, 0},
  {"macu.l", 2,  0x48,  mul_reg_reg_opr, 0},

  {"macs.b", 2,  0x48,  mul_reg_reg_imm, 0},
  {"macs.w", 2,  0x48,  mul_reg_reg_imm, 0},
  {"macs.l", 2,  0x48,  mul_reg_reg_imm, 0},

  {"macu.b", 2,  0x48,  mul_reg_reg_imm, 0},
  {"macu.w", 2,  0x48,  mul_reg_reg_imm, 0},
  {"macu.l", 2,  0x48,  mul_reg_reg_imm, 0},

  {"macs.bb", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.bw", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.bp", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.bl", 2,  0x48,  mul_reg_opr_opr, 0},

  {"macs.wb", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.ww", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.wp", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.wl", 2,  0x48,  mul_reg_opr_opr, 0},

  {"macs.pb", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.pw", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.pp", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.pl", 2,  0x48,  mul_reg_opr_opr, 0},

  {"macs.lb", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.lw", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.lp", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macs.ll", 2,  0x48,  mul_reg_opr_opr, 0},

  {"macu.bb", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.bw", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.bp", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.bl", 2,  0x48,  mul_reg_opr_opr, 0},

  {"macu.wb", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.ww", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.wp", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.wl", 2,  0x48,  mul_reg_opr_opr, 0},

  {"macu.pb", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.pw", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.pp", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.pl", 2,  0x48,  mul_reg_opr_opr, 0},

  {"macu.lb", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.lw", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.lp", 2,  0x48,  mul_reg_opr_opr, 0},
  {"macu.ll", 2,  0x48,  mul_reg_opr_opr, 0},

  {"mods",   2,  0x38,  mul_reg_reg_reg, 0},
  {"modu",   2,  0x38,  mul_reg_reg_reg, 0},

  {"mods.b", 2,  0x38,  mul_reg_reg_opr, 0},
  {"mods.w", 2,  0x38,  mul_reg_reg_opr, 0},
  {"mods.l", 2,  0x38,  mul_reg_reg_opr, 0},

  {"modu.b", 2,  0x38,  mul_reg_reg_opr, 0},
  {"modu.w", 2,  0x38,  mul_reg_reg_opr, 0},
  {"modu.l", 2,  0x38,  mul_reg_reg_opr, 0},

  {"mods.b", 2,  0x38,  mul_reg_reg_imm, 0},
  {"mods.w", 2,  0x38,  mul_reg_reg_imm, 0},
  {"mods.l", 2,  0x38,  mul_reg_reg_imm, 0},

  {"modu.b", 2,  0x38,  mul_reg_reg_imm, 0},
  {"modu.w", 2,  0x38,  mul_reg_reg_imm, 0},
  {"modu.l", 2,  0x38,  mul_reg_reg_imm, 0},

  {"mods.bb", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.bw", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.bp", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.bl", 2,  0x38,  mul_reg_opr_opr, 0},

  {"mods.wb", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.ww", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.wp", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.wl", 2,  0x38,  mul_reg_opr_opr, 0},

  {"mods.pb", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.pw", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.pp", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.pl", 2,  0x38,  mul_reg_opr_opr, 0},

  {"mods.lb", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.lw", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.lp", 2,  0x38,  mul_reg_opr_opr, 0},
  {"mods.ll", 2,  0x38,  mul_reg_opr_opr, 0},

  {"modu.bb", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.bw", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.bp", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.bl", 2,  0x38,  mul_reg_opr_opr, 0},

  {"modu.wb", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.ww", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.wp", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.wl", 2,  0x38,  mul_reg_opr_opr, 0},

  {"modu.pb", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.pw", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.pp", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.pl", 2,  0x38,  mul_reg_opr_opr, 0},

  {"modu.lb", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.lw", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.lp", 2,  0x38,  mul_reg_opr_opr, 0},
  {"modu.ll", 2,  0x38,  mul_reg_opr_opr, 0}
};


/* Gas line assembler entry point.  */

/* This is the main entry point for the machine-dependent assembler.  str
   points to a machine-dependent instruction.  This function is supposed to
   emit the frags/bytes it assembles to.  */
void
md_assemble (char *str)
{
  char *op_start;
  char *op_end;
  char name[20];
  size_t nlen = 0;

  fail_line_pointer = NULL;

  /* Find the opcode end and get the opcode in 'name'.  The opcode is forced
     lower case (the opcode table only has lower case op-codes).  */
  for (op_start = op_end = str;
       *op_end && !is_end_of_line[(int)*op_end] && *op_end != ' ';
       op_end++)
    {
      name[nlen] = TOLOWER (op_start[nlen]);
      nlen++;
      gas_assert (nlen < sizeof (name) - 1);
    }
  name[nlen] = 0;

  if (nlen == 0)
    {
      as_bad (_("No instruction or missing opcode."));
      return;
    }

  input_line_pointer = skip_whites (op_end);

  size_t i;
  for (i = 0; i < sizeof (opcodes) / sizeof (opcodes[0]); ++i)
    {
      const struct instruction *opc = opcodes + i;
      if (0 == strcmp (name, opc->name))
	{
	  if (opc->parse_operands (opc))
	    return;
	  continue;
	}
    }

  as_bad (_("Invalid instruction: \"%s\""), str);
  as_bad (_("First invalid token: \"%s\""), fail_line_pointer);
  while (*input_line_pointer++)
    ;
}





/* Relocation, relaxation and frag conversions.  */

/* PC-relative offsets are relative to the start of the
   next instruction.  That is, the address of the offset, plus its
   size, since the offset is always the last part of the insn.  */
long
md_pcrel_from (fixS *fixP)
{
  long ret = fixP->fx_size + fixP->fx_frag->fr_address;
  if (fixP->fx_addsy && S_IS_DEFINED (fixP->fx_addsy))
    ret += fixP->fx_where;

  return ret;
}


/* We need a port-specific relaxation function to cope with sym2 - sym1
   relative expressions with both symbols in the same segment (but not
   necessarily in the same frag as this insn), for example:
   ldab sym2-(sym1-2),pc
   sym1:
   The offset can be 5, 9 or 16 bits long.  */

long
s12z_relax_frag (segT seg ATTRIBUTE_UNUSED, fragS *fragP ATTRIBUTE_UNUSED,
		   long stretch ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, asection *sec ATTRIBUTE_UNUSED,
                 fragS *fragP ATTRIBUTE_UNUSED)
{
}

/* On an ELF system, we can't relax a weak symbol.  The weak symbol
   can be overridden at final link time by a non weak symbol.  We can
   relax externally visible symbol because there is no shared library
   and such symbol can't be overridden (unless they are weak).  */

/* Force truly undefined symbols to their maximum size, and generally set up
   the frag list to be relaxed.  */
int
md_estimate_size_before_relax (fragS *fragP ATTRIBUTE_UNUSED, asection *segment ATTRIBUTE_UNUSED)
{
  return 0;
}


/* If while processing a fixup, a reloc really needs to be created
   then it is done here.  */
arelent *
tc_gen_reloc (asection *section, fixS *fixp)
{
  arelent *reloc = XNEW (arelent);
  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("Relocation %d is not supported by object file format."),
		    (int) fixp->fx_r_type);
      return NULL;
    }

  if (0 == (section->flags & SEC_CODE))
    reloc->addend = fixp->fx_offset;
  else
    reloc->addend = fixp->fx_addnumber;

  return reloc;
}

/* See whether we need to force a relocation into the output file.  */
int
tc_s12z_force_relocation (fixS *fixP)
{
  return generic_force_reloc (fixP);
}

/* Here we decide which fixups can be adjusted to make them relative
   to the beginning of the section instead of the symbol.  Basically
   we need to make sure that the linker relaxation is done
   correctly, so in some cases we force the original symbol to be
   used.  */
bool
tc_s12z_fix_adjustable (fixS *fixP ATTRIBUTE_UNUSED)
{
  return true;
}

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  long value = *valP;

  if (fixP->fx_addsy == (symbolS *) NULL)
    fixP->fx_done = 1;

  /* We don't actually support subtracting a symbol.  */
  if (fixP->fx_subsy != (symbolS *) NULL)
    as_bad_subtract (fixP);

  /*
    Patch the instruction with the resolved operand.  Elf relocation
    info will also be generated to take care of linker/loader fixups.
  */
  char *where = fixP->fx_frag->fr_literal + fixP->fx_where;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_8:
      ((bfd_byte *) where)[0] = (bfd_byte) value;
      break;
    case BFD_RELOC_16:
      bfd_putb16 ((bfd_vma) value, (unsigned char *) where);
      break;
    case BFD_RELOC_24:
      bfd_putb24 ((bfd_vma) value, (unsigned char *) where);
      break;
    case BFD_RELOC_S12Z_OPR:
      {
        switch (fixP->fx_size)
          {
          case 3:
            bfd_putb24 ((bfd_vma) value, (unsigned char *) where);
            break;
          case 2:
            bfd_putb16 ((bfd_vma) value, (unsigned char *) where);
            break;
          default:
            abort ();
          }
      }
      break;
    case BFD_RELOC_32:
      bfd_putb32 ((bfd_vma) value, (unsigned char *) where);
      break;
    case BFD_RELOC_16_PCREL:
      if (value < -0x4000 || value > 0x3FFF)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("Value out of 16-bit range."));

      bfd_putb16 ((bfd_vma) value | 0x8000, (unsigned char *) where);
      break;

    default:
      as_fatal (_("Line %d: unknown relocation type: 0x%x."),
		fixP->fx_line, fixP->fx_r_type);
    }
}

/* Set the ELF specific flags.  */
void
s12z_elf_final_processing (void)
{
}
