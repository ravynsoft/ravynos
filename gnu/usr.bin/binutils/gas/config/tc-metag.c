/* tc-metag.c -- Assembler for the Imagination Technologies Meta.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by Imagination Technologies Ltd.

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
#include "subsegs.h"
#include "symcat.h"
#include "safe-ctype.h"
#include "hashtab.h"

#include <stdio.h>

#include "opcode/metag.h"

const char comment_chars[]        = "!";
const char line_comment_chars[]   = "!#";
const char line_separator_chars[] = ";";
const char FLT_CHARS[]            = "rRsSfFdDxXpP";
const char EXP_CHARS[]            = "eE";
const char metag_symbol_chars[]   = "[";

static char register_chars[256];
static char mnemonic_chars[256];

#define is_register_char(x) (register_chars[(unsigned char) x])
#define is_mnemonic_char(x) (mnemonic_chars[(unsigned char) x])
#define is_whitespace_char(x) (((x) == ' ') || ((x) == '\t'))
#define is_space_char(x) ((x) == ' ')

#define FPU_PREFIX_CHAR 'f'
#define DSP_PREFIX_CHAR 'd'

/* Instruction mnemonics that need disambiguating with respect to prefixes.  */
#define FFB_INSN        "ffb"
#define DCACHE_INSN     "dcache"
#define DEFR_INSN       "defr"

#define FPU_DOUBLE_CHAR 'd'
#define FPU_PAIR_CHAR   'l'

#define DSP_DUAL_CHAR	'l'

#define END_OF_INSN     '\0'

/* Maximum length of a mnemonic including all suffixes.  */
#define MAX_MNEMONIC_LEN 16
/* Maximum length of a register name.  */
#define MAX_REG_LEN      17

/* Addressing modes must be enclosed with square brackets.  */
#define ADDR_BEGIN_CHAR '['
#define ADDR_END_CHAR   ']'
/* Immediates must be prefixed with a hash.  */
#define IMM_CHAR        '#'

#define COMMA           ','
#define PLUS            '+'
#define MINUS           '-'

/* Short units are those that can be encoded with 2 bits.  */
#define SHORT_UNITS     "D0, D1, A0 or A1"

static unsigned int mcpu_opt = CoreMeta12;
static unsigned int mfpu_opt = 0;
static unsigned int mdsp_opt = 0;

const char * md_shortopts = "m:";

struct option md_longopts[] =
{
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

/* Parser hash tables.  */
static htab_t mnemonic_htab;
static htab_t reg_htab;
static htab_t dsp_reg_htab;
static htab_t dsp_tmpl_reg_htab[2];
static htab_t scond_htab;

#define GOT_NAME "__GLOBAL_OFFSET_TABLE__"
symbolS * GOT_symbol;

enum fpu_insn_width {
  FPU_WIDTH_SINGLE,
  FPU_WIDTH_DOUBLE,
  FPU_WIDTH_PAIR,
};

#define FPU_ACTION_ABS_CHAR   'a'
#define FPU_ACTION_INV_CHAR   'i'
#define FPU_ACTION_QUIET_CHAR 'q'
#define FPU_ACTION_ZERO_CHAR  'z'

#define FPU_ACTION_ABS        0x1
#define FPU_ACTION_INV        0x2
#define FPU_ACTION_QUIET      0x4
#define FPU_ACTION_ZERO       0x8

enum dsp_insn_width {
  DSP_WIDTH_SINGLE,
  DSP_WIDTH_DUAL,
};

#define DSP_ACTION_QR64_CHAR     'q'
#define DSP_ACTION_UMUL_CHAR     'u'
#define DSP_ACTION_ROUND_CHAR    'r'
#define DSP_ACTION_CLAMP9_CHAR   'g'
#define DSP_ACTION_CLAMP8_CHAR   'b'
#define DSP_ACTION_MOD_CHAR      'm'
#define DSP_ACTION_ACC_ZERO_CHAR 'z'
#define DSP_ACTION_ACC_ADD_CHAR  'p'
#define DSP_ACTION_ACC_SUB_CHAR  'n'
#define DSP_ACTION_OV_CHAR       'o'

#define DSP_ACTION_QR64          0x001
#define DSP_ACTION_UMUL          0x002
#define DSP_ACTION_ROUND         0x004
#define DSP_ACTION_CLAMP9        0x008
#define DSP_ACTION_CLAMP8        0x010
#define DSP_ACTION_MOD           0x020
#define DSP_ACTION_ACC_ZERO      0x040
#define DSP_ACTION_ACC_ADD       0x080
#define DSP_ACTION_ACC_SUB       0x100
#define DSP_ACTION_OV            0x200

#define DSP_DAOPPAME_8_CHAR    'b'
#define DSP_DAOPPAME_16_CHAR   'w'
#define DSP_DAOPPAME_TEMP_CHAR 't'
#define DSP_DAOPPAME_HIGH_CHAR 'h'

#define DSP_DAOPPAME_8         0x1
#define DSP_DAOPPAME_16        0x2
#define DSP_DAOPPAME_TEMP      0x4
#define DSP_DAOPPAME_HIGH      0x8

/* Structure holding information about a parsed instruction.  */
typedef struct {
  /* Instruction type.  */
  enum insn_type type;
  /* Split condition code. */
  enum scond_code scond;

  /* Instruction bits.  */
  unsigned int bits;
  /* Size of the instruction in bytes.  */
  size_t len;

  /* FPU instruction encoding.  */
  enum fpu_insn_width fpu_width;
  unsigned int fpu_action_flags;

  /* DSP instruction encoding. */
  enum dsp_insn_width dsp_width;
  unsigned int dsp_action_flags;
  unsigned int dsp_daoppame_flags;

  /* Reloc encoding information, maximum of one reloc per insn.  */
  enum bfd_reloc_code_real reloc_type;
  int reloc_pcrel;
  expressionS reloc_exp;
  unsigned int reloc_size;
} metag_insn;

/* Structure holding information about a parsed addressing mode.  */
typedef struct {
  const metag_reg *base_reg;
  const metag_reg *offset_reg;

  expressionS exp;

  enum bfd_reloc_code_real reloc_type;

  /* Whether we have an immediate or not.  */
  unsigned short immediate:1;
  /* Whether or not the base register is updated.  */
  unsigned short update:1;
  /* Whether the operation uses the address pre or post increment.  */
  unsigned short post_increment:1;
  /* Whether the immediate should be negated.  */
  unsigned short negate:1;
} metag_addr;

/* Linked list of possible parsers for this instruction.  */
typedef struct _insn_templates {
  const insn_template *template;
  struct _insn_templates *next;
} insn_templates;

/* Parse an instruction that takes no operands.  */
static const char *
parse_none (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  insn->bits = template->meta_opcode;
  insn->len = 4;
  return line;
}

/* Return the next non-whitespace character in LINE or NULL.  */
static const char *
skip_whitespace (const char *line)
{
  const char *l = line;

  if (is_whitespace_char (*l))
    {
      l++;
    }

  return l;
}

/* Return the next non-space character in LINE or NULL.  */
static const char *
skip_space (const char *line)
{
  const char *l = line;

  if (is_space_char (*l))
    {
      l++;
    }

  return l;
}

/* Return the character after the current one in LINE if the current
   character is a comma, otherwise NULL.  */
static const char *
skip_comma (const char *line)
{
  const char *l = line;

  if (l == NULL || *l != COMMA)
    return NULL;

  l++;

  return l;
}

/* Return the metag_reg struct corresponding to NAME or NULL if no such
   register exists.  */
static const metag_reg *
parse_gp_reg (const char *name)
{
  const metag_reg *reg;
  metag_reg entry;

  entry.name = name;

  reg = (const metag_reg *) htab_find (reg_htab, &entry);

  return reg;
}

/* Parse a list of up to COUNT GP registers from LINE, returning the
   registers parsed in REGS and the number parsed in REGS_READ. Return
   a pointer to the next character or NULL.  */
static const char *
parse_gp_regs_list (const char *line, const metag_reg **regs, size_t count,
		    size_t *regs_read)
{
  const char *l = line;
  char reg_buf[MAX_REG_LEN];
  int seen_regs = 0;
  size_t i;

  for (i = 0; i < count; i++)
    {
      size_t len = 0;
      const char *next;

      next = l;

      if (i > 0)
	{
	  l = skip_comma (l);
	  if (l == NULL)
	    {
	      *regs_read = seen_regs;
	      return next;
	    }
	}

      while (is_register_char (*l))
	{
	  reg_buf[len] = *l;
	  l++;
	  len++;
	  if (!(len < MAX_REG_LEN))
	    return NULL;
	}

      reg_buf[len] = '\0';

      if (len)
	{
	  const metag_reg *reg = parse_gp_reg (reg_buf);

	  if (!reg)
	    {
	      *regs_read = seen_regs;
	      return next;
	    }
	  else
	    {
	      regs[i] = reg;
	      seen_regs++;
	    }
	}
      else
	{
	  *regs_read = seen_regs;
	  return next;
	}
    }

  *regs_read = seen_regs;
  return l;
}

/* Parse a list of exactly COUNT GP registers from LINE, returning the
   registers parsed in REGS. Return a pointer to the next character or NULL.  */
static const char *
parse_gp_regs (const char *line, const metag_reg **regs, size_t count)
{
  const char *l = line;
  size_t regs_read = 0;

  l = parse_gp_regs_list (l, regs, count, &regs_read);

  if (regs_read != count)
    return NULL;
  else
    return l;
}

/* Parse a list of exactly COUNT FPU registers from LINE, returning the
   registers parsed in REGS. Return a pointer to the next character or NULL.  */
static const char *
parse_fpu_regs (const char *line, const metag_reg **regs, size_t count)
{
  const char *l = line;
  size_t regs_read = 0;

  l = parse_gp_regs_list (l, regs, count, &regs_read);

  if (regs_read != count)
    return NULL;
  else
    {
      size_t i;
      for (i = 0; i < count; i++)
	{
	  if (regs[i]->unit != UNIT_FX)
	    return NULL;
	}
      return l;
    }
}

/* Return TRUE if REG1 and REG2 are in paired units.  */
static bool
is_unit_pair (const metag_reg *reg1, const metag_reg *reg2)
{
  if ((reg1->unit == UNIT_A0 &&
       (reg2->unit == UNIT_A1)) ||
      (reg1->unit == UNIT_A1 &&
       (reg2->unit == UNIT_A0)) ||
      (reg1->unit == UNIT_D0 &&
       (reg2->unit == UNIT_D1)) ||
      (reg1->unit == UNIT_D1 &&
       (reg2->unit == UNIT_D0)))
    return true;

  return false;
}

/* Return TRUE if REG1 and REG2 form a register pair.  */
static bool
is_reg_pair (const metag_reg *reg1, const metag_reg *reg2)
{
  if (reg1->unit == UNIT_FX &&
      reg2->unit == UNIT_FX &&
      reg2->no == reg1->no + 1)
    return true;

  if (reg1->no != reg2->no)
    return false;

  return is_unit_pair (reg1, reg2);
}

/* Parse a pair of GP registers from LINE, returning the registers parsed
   in REGS. Return a pointer to the next character or NULL.  */
static const char *
parse_pair_gp_regs (const char *line, const metag_reg **regs)
{
  const char *l = line;

  l = parse_gp_regs (line, regs, 2);

  if (l == NULL)
    {
      l = parse_gp_regs (line, regs, 1);

      if (l == NULL)
	return NULL;

      if (regs[0]->unit == UNIT_RD)
	return l;
      else
	return NULL;
    }

  if (is_reg_pair (regs[0], regs[1]))
    return l;

  return NULL;
}

/* Parse a unit-to-unit MOV instruction.  */
static const char *
parse_mov_u2u (const char *line, metag_insn *insn,
	       const insn_template *template)
{
  const metag_reg *regs[2];

  line = parse_gp_regs (line, regs, 2);

  if (line == NULL)
    return NULL;

  if (!mfpu_opt && (regs[0]->unit == UNIT_FX || regs[1]->unit == UNIT_FX))
    {
      as_bad (_("no floating point unit specified"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(regs[1]->no << 19) |
		(regs[0]->no << 14) |
		(regs[1]->unit << 10) |
		(regs[0]->unit << 5));
  insn->len = 4;
  return line;
}

/* Parse a MOV to port instruction.  */
static const char *
parse_mov_port (const char *line, metag_insn *insn,
		const insn_template *template)
{
  const char *l = line;
  bool is_movl = MINOR_OPCODE (template->meta_opcode) == MOVL_MINOR;
  const metag_reg *dest_regs[2];
  const metag_reg *port_regs[1];

  if (is_movl)
    l = parse_gp_regs (l, dest_regs, 2);
  else
    l = parse_gp_regs (l, dest_regs, 1);

  if (l == NULL)
    return NULL;

  if (template->insn_type == INSN_FPU && dest_regs[0]->unit != UNIT_FX)
    return NULL;

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_gp_regs (l, port_regs, 1);

  if (l == NULL)
    return NULL;

  if (port_regs[0]->unit != UNIT_RD ||
      port_regs[0]->no != 0)
    return NULL;

  if (is_movl)
    {
      if (!is_unit_pair (dest_regs[0], dest_regs[1]))
	return NULL;

      insn->bits = (template->meta_opcode |
		    (dest_regs[0]->no << 14) |
		    (dest_regs[1]->no << 9) |
		    ((dest_regs[0]->unit & SHORT_UNIT_MASK) << 5));
    }
  else
    insn->bits = (template->meta_opcode |
		  (dest_regs[0]->no << 14) |
		  (dest_regs[0]->unit << 5));

  insn->len = 4;
  return l;
}

/* Parse a MOVL to TTREC instruction.  */
static const char *
parse_movl_ttrec (const char *line, metag_insn *insn,
		  const insn_template *template)
{
  const char *l = line;
  const metag_reg *src_regs[2];
  const metag_reg *dest_regs[1];

  l = parse_gp_regs (l, dest_regs, 1);

  if (l == NULL)
    return NULL;

  if (dest_regs[0]->unit != UNIT_TT ||
      dest_regs[0]->no != 3)
    return NULL;

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_gp_regs (l, src_regs, 2);

  if (l == NULL)
    return NULL;

  if (!is_unit_pair (src_regs[0], src_regs[1]))
    return NULL;

  insn->bits = (template->meta_opcode |
		(src_regs[0]->no << 19) |
		(src_regs[1]->no << 14) |
		((src_regs[0]->unit & SHORT_UNIT_MASK) << 7));

  insn->len = 4;
  return l;
}

/* Parse an incrementing or decrementing addressing mode.  */
static const char *
parse_addr_incr_op (const char *line, metag_addr *addr)
{
  const char *l = line;
  const char *ll;

  ll = l + 1;

  if (*l == PLUS &&
      *ll == PLUS)
    {
      addr->update = 1;
      ll++;
      return ll;
    }
  else if (*l == MINUS &&
	   *ll == MINUS)
    {
      addr->update = 1;
      addr->negate = 1;
      ll++;
      return ll;
    }
  return NULL;
}

/* Parse an pre-incrementing or pre-decrementing addressing mode.  */
static const char *
parse_addr_pre_incr_op (const char *line, metag_addr *addr)
{
  return parse_addr_incr_op (line, addr);
}

/* Parse an post-incrementing or post-decrementing addressing mode.  */
static const char *
parse_addr_post_incr_op (const char *line, metag_addr *addr)
{
  const char *l;

  l = parse_addr_incr_op (line, addr);

  if (l == NULL)
    return NULL;

  addr->post_increment = 1;

  return l;
}

/* Parse an infix addressing mode.  */
static const char *
parse_addr_op (const char *line, metag_addr *addr)
{
  const char *l = line;
  const char *ll;

  ll = l + 1;

  if (*l == PLUS)
    {
      if (*ll == PLUS)
	{
	  addr->update = 1;
	  ll++;
	  return ll;
	}
      l++;
      return l;
    }
  return NULL;
}

/* Parse the immediate portion of an addressing mode.  */
static const char *
parse_imm_addr (const char *line, metag_addr *addr)
{
  const char *l = line;
  char *save_input_line_pointer;
  expressionS *exp = &addr->exp;

  /* Skip #.  */
  if (*l == '#')
    l++;
  else
    return NULL;

  save_input_line_pointer = input_line_pointer;
  input_line_pointer = (char *) l;

  expression (exp);

  l = input_line_pointer;
  input_line_pointer = save_input_line_pointer;

  if (exp->X_op == O_absent || exp->X_op == O_big)
    {
      return NULL;
    }
  else if (exp->X_op == O_constant)
    {
      return l;
    }
  else
    {
      if (exp->X_op == O_PIC_reloc &&
	  exp->X_md == BFD_RELOC_METAG_GETSET_GOT)
	{
	  exp->X_op = O_symbol;
	  addr->reloc_type = BFD_RELOC_METAG_GETSET_GOT;
	}
      else if (exp->X_op == O_PIC_reloc &&
	       exp->X_md == BFD_RELOC_METAG_TLS_IE)
	{
	  exp->X_op = O_symbol;
	  addr->reloc_type = BFD_RELOC_METAG_TLS_IE;
	}
      else if (exp->X_op == O_PIC_reloc &&
	  exp->X_md == BFD_RELOC_METAG_GOTOFF)
	{
	  exp->X_op = O_symbol;
	  addr->reloc_type = BFD_RELOC_METAG_GETSET_GOTOFF;
	}
      else
	addr->reloc_type = BFD_RELOC_METAG_GETSETOFF;
      return l;
    }
}

/* Parse the offset portion of an addressing mode (register or immediate).  */
static const char *
parse_addr_offset (const char *line, metag_addr *addr, int size)
{
  const char *l = line;
  const metag_reg *regs[1];

  if (*l == IMM_CHAR)
    {
      /* ++ is a valid operator in our addressing but not in an expr. Make
	 sure that the expression parser never sees it.  */
      char *ppp = strstr(l, "++");
      char ppch = '+';

      if (ppp)
	*ppp = '\0';

      l = parse_imm_addr (l, addr);

      if (ppp)
	*ppp = ppch;

      if (l == NULL)
	return NULL;

      if (addr->exp.X_add_number % size)
	{
	  as_bad (_("offset must be a multiple of %d"), size);
	  return NULL;
	}

      addr->immediate = 1;
      return l;
    }
  else
    {
      l = parse_gp_regs (l, regs, 1);

      if (l == NULL)
	return NULL;

      if (regs[0]->unit != addr->base_reg->unit)
	{
	  as_bad (_("offset and base must be from the same unit"));
	  return NULL;
	}

      addr->offset_reg = regs[0];
      return l;
    }
}

/* Parse an addressing mode.  */
static const char *
parse_addr (const char *line, metag_addr *addr, unsigned int size)
{
  const char *l = line;
  const char *ll;
  const metag_reg *regs[1];

  /* Skip opening square bracket.  */
  l++;

  ll = parse_addr_pre_incr_op (l, addr);

  if (ll != NULL)
    l = ll;

  l = parse_gp_regs (l, regs, 1);

  if (l == NULL)
    return NULL;

  addr->base_reg = regs[0];

  if (*l == ADDR_END_CHAR)
    {
      addr->exp.X_op = O_constant;
      addr->exp.X_add_symbol = NULL;
      addr->exp.X_op_symbol = NULL;
      if (addr->update == 1)
	{
	  /* We have a pre increment/decrement.  */
	  addr->exp.X_add_number = size;
	}
      else
	{
	  /* Simple register with no offset (0 immediate).  */
	  addr->exp.X_add_number = 0;
	}
      addr->immediate = 1;
      l++;
      return l;
    }

  /* We already had a pre increment/decrement.  */
  if (addr->update == 1)
    return NULL;

  ll = parse_addr_post_incr_op (l, addr);

  if (ll && *ll == ADDR_END_CHAR)
    {
      if (addr->update == 1)
	{
	  /* We have a post increment/decrement.  */
	  addr->exp.X_op = O_constant;
	  addr->exp.X_add_number = size;
	  addr->exp.X_add_symbol = NULL;
	  addr->exp.X_op_symbol = NULL;
	  addr->post_increment = 1;
	}
      addr->immediate = 1;
      ll++;
      return ll;
    }

  addr->post_increment = 0;

  l = parse_addr_op (l, addr);

  if (l == NULL)
    return NULL;

  l = parse_addr_offset (l, addr, size);

  if (l == NULL)
    return NULL;

  if (*l == ADDR_END_CHAR)
    {
      l++;
      return l;
    }

  /* We already had a pre increment/decrement. */
  if (addr->update == 1)
    return NULL;

  l = parse_addr_post_incr_op (l, addr);

  if (l == NULL)
    return NULL;

  if (*l == ADDR_END_CHAR)
    {
      l++;
      return l;
    }

  return NULL;
}

/* Parse a GET or pipeline MOV instruction.  */
static const char *
parse_get (const char *line, const metag_reg **regs, metag_addr *addr,
	   unsigned int size, bool is_mov)
{
  const char *l = line;

  if (size == 8)
    {
      l = parse_pair_gp_regs (l, regs);

      if (l == NULL)
	  return NULL;
    }
  else
    {
      l = parse_gp_regs (l, regs, 1);

      if (l == NULL)
	{
	  if (!is_mov)
	    as_bad (_("invalid destination register"));
	  return NULL;
	}
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_addr (l, addr, size);

  if (l == NULL)
    {
      if (!is_mov)
	as_bad (_("invalid memory operand"));
      return NULL;
    }

  return l;
}

/* Parse a SET instruction.  */
static const char *
parse_set (const char *line, const metag_reg **regs, metag_addr *addr,
	   unsigned int size)
{
  const char *l = line;

  l = parse_addr (l, addr, size);

  if (l == NULL)
    {
	  as_bad (_("invalid memory operand"));
	  return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  if (size == 8)
    {
      const char *ll = l;

      ll = parse_pair_gp_regs (l, regs);

      if (ll == NULL)
	{
	  /* Maybe this is an RD register, which is 64 bits wide so needs no
	     pair.  */
	  l = parse_gp_regs (l, regs, 1);

	  if (l == NULL ||
	      regs[0]->unit != UNIT_RD)
	    {
	      return NULL;
	    }
	}
      else
	l = ll;
    }
  else
    {
      l = parse_gp_regs (l, regs, 1);

      if (l == NULL)
	{
	  as_bad (_("invalid source register"));
	  return NULL;
	}
    }

  return l;
}

/* Check a signed integer value can be represented in the given number
   of bits.  */
static bool
within_signed_range (int value, unsigned int bits)
{
  int min_val = -(1 << (bits - 1));
  int max_val = (1 << (bits - 1)) - 1;
  return (value <= max_val) && (value >= min_val);
}

/* Check an unsigned integer value can be represented in the given number
   of bits.  */
static bool
within_unsigned_range (unsigned int value, unsigned int bits)
{
  return value < (unsigned int)(1 << bits);
}

/* Return TRUE if UNIT can be expressed using a short code.  */
static bool
is_short_unit (enum metag_unit unit)
{
  switch (unit)
    {
    case UNIT_A0:
    case UNIT_A1:
    case UNIT_D0:
    case UNIT_D1:
      return true;
    default:
      return false;
    }
}

/* Copy reloc data from ADDR to INSN.  */
static void
copy_addr_reloc (metag_insn *insn, metag_addr *addr)
{
  memcpy (&insn->reloc_exp, &addr->exp, sizeof(insn->reloc_exp));
  insn->reloc_type = addr->reloc_type;
}

/* Parse a GET, SET or pipeline MOV instruction.  */
static const char *
parse_get_set (const char *line, metag_insn *insn,
	       const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];
  metag_addr addr;
  unsigned int size = metag_get_set_size_bytes (template->meta_opcode);
  bool is_get = MAJOR_OPCODE (template->meta_opcode) == OPC_GET;
  unsigned int reg_no;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  if (is_get)
    {
      bool is_mov = startswith (template->name, "MOV");

      l = parse_get (l, regs, &addr, size, is_mov);

      if (l == NULL)
	return NULL;

      if (!(regs[0]->unit == UNIT_D0 ||
	    regs[0]->unit == UNIT_D1 ||
	    regs[0]->unit == UNIT_A0 ||
	    regs[0]->unit == UNIT_A1 ||
	    (regs[0]->unit == UNIT_RD && is_mov) ||
	    (regs[0]->unit == UNIT_CT && size == 4) ||
	    (regs[0]->unit == UNIT_PC && size == 4) ||
	    (regs[0]->unit == UNIT_TR && size == 4) ||
	    (regs[0]->unit == UNIT_TT && (size == 4 || size == 8)) ||
	    regs[0]->unit == UNIT_FX))
	{
	  as_bad (_("invalid destination unit"));
	  return NULL;
	}

      if (regs[0]->unit == UNIT_RD)
	{
	  if (regs[0]->no == 0)
	    {
	      as_bad (_("mov cannot use RD port as destination"));
	      return NULL;
	    }
	}

      reg_no = regs[0]->no;
    }
  else
    {
      l = parse_set (l, regs, &addr, size);

      if (l == NULL)
	return NULL;

      if (!(regs[0]->unit == UNIT_D0 ||
	    regs[0]->unit == UNIT_D1 ||
	    regs[0]->unit == UNIT_A0 ||
	    regs[0]->unit == UNIT_A1 ||
	    regs[0]->unit == UNIT_RD ||
	    (regs[0]->unit == UNIT_CT && size == 4) ||
	    (regs[0]->unit == UNIT_PC && size == 4) ||
	    (regs[0]->unit == UNIT_TR && size == 4) ||
	    (regs[0]->unit == UNIT_TT && (size == 4 || size == 8)) ||
	    regs[0]->unit == UNIT_FX))
	{
	  as_bad (_("invalid source unit"));
	  return NULL;
	}

      if (addr.immediate == 0 &&
	  (regs[0]->unit == addr.base_reg->unit ||
	   (size == 8 && is_unit_pair (regs[0], addr.base_reg))))
	{
	  as_bad (_("source and address units must not be shared for this addressing mode"));
	  return NULL;
	}

      if (regs[0]->unit == UNIT_RD)
	{
	  if (regs[0]->no != 0)
	    {
	      as_bad (_("set can only use RD port as source"));
	      return NULL;
	    }
	  reg_no = 16;
	}
      else
	reg_no = regs[0]->no;
    }

  insn->bits = (template->meta_opcode |
		(reg_no << 19) |
		(regs[0]->unit << 1));

  if (!is_short_unit (addr.base_reg->unit))
    {
      as_bad (_("base unit must be one of %s"), SHORT_UNITS);
      return NULL;
    }

  insn->bits |= ((addr.base_reg->no << 14) |
		 ((addr.base_reg->unit & SHORT_UNIT_MASK) << 5));

  if (addr.immediate)
    {
      int offset = addr.exp.X_add_number;

      copy_addr_reloc (insn, &addr);

      if (addr.negate)
	offset = -offset;

      offset = offset / (int)size;

      if (!within_signed_range (offset, GET_SET_IMM_BITS))
	{
	  /* We already tried to encode as an extended GET/SET.  */
	  as_bad (_("offset value out of range"));
	  return NULL;
	}

      offset = offset & GET_SET_IMM_MASK;

      insn->bits |= (0x1 << 25);
      insn->bits |= (offset << 8);
    }
  else
    {
      insn->bits |= (addr.offset_reg->no << 9);
    }

  if (addr.update)
    insn->bits |= (0x1 << 7);

  if (addr.post_increment)
    insn->bits |= 0x1;

  insn->len = 4;
  return l;
}

/* Parse an extended GET or SET instruction.  */
static const char *
parse_get_set_ext (const char *line, metag_insn *insn,
		   const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];
  metag_addr addr;
  unsigned int size = metag_get_set_ext_size_bytes (template->meta_opcode);
  bool is_get = MINOR_OPCODE (template->meta_opcode) == GET_EXT_MINOR;
  bool is_mov = MINOR_OPCODE (template->meta_opcode) == MOV_EXT_MINOR;
  unsigned int reg_unit;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  if (is_get || is_mov)
    {
      l = parse_get (l, regs, &addr, size, is_mov);
    }
  else
    {
      l = parse_set (l, regs, &addr, size);
    }

  if (l == NULL)
    return NULL;

  /* Extended GET/SET does not support incrementing addressing.  */
  if (addr.update)
    return NULL;

  if (is_mov)
    {
      if (regs[0]->unit != UNIT_RD)
	{
	  as_bad (_("destination unit must be RD"));
	  return NULL;
	}
      reg_unit = 0;
    }
  else
    {
      if (!is_short_unit (regs[0]->unit))
	{
	  return NULL;
	}
      reg_unit = regs[0]->unit;
    }

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		((reg_unit & SHORT_UNIT_MASK) << 3));

  if (!is_short_unit (addr.base_reg->unit))
    {
      as_bad (_("base unit must be one of %s"), SHORT_UNITS);
      return NULL;
    }

  if (addr.base_reg->no > 1)
    {
      return NULL;
    }

  insn->bits |= ((addr.base_reg->no & EXT_BASE_REG_MASK) |
		 ((addr.base_reg->unit & SHORT_UNIT_MASK) << 5));

  if (addr.immediate)
    {
      int offset = addr.exp.X_add_number;

      copy_addr_reloc (insn, &addr);

      if (addr.negate)
	offset = -offset;

      offset = offset / (int)size;

      if (!within_signed_range (offset, GET_SET_EXT_IMM_BITS))
	{
	  /* Parsing as a standard GET/SET provides a smaller offset.  */
	  as_bad (_("offset value out of range"));
	  return NULL;
	}

      offset = offset & GET_SET_EXT_IMM_MASK;

      insn->bits |= (offset << 7);
    }
  else
    {
      return NULL;
    }

  insn->len = 4;
  return l;
}

/* Parse an MGET or MSET instruction addressing mode.  */
static const char *
parse_mget_mset_addr (const char *line, metag_addr *addr)
{
  const char *l = line;
  const char *ll;
  const metag_reg *regs[1];

  /* Skip opening square bracket.  */
  l++;

  l = parse_gp_regs (l, regs, 1);

  if (l == NULL)
    return NULL;

  addr->base_reg = regs[0];

  ll = parse_addr_post_incr_op (l, addr);

  if (ll != NULL)
    l = ll;

  if (addr->negate == 1)
    return NULL;

  if (*l == ADDR_END_CHAR)
    {
      l++;
      return l;
    }

  return NULL;
}

/* Parse an MGET instruction.  */
static const char *
parse_mget (const char *line, const metag_reg **regs, metag_addr *addr,
	    size_t *regs_read)
{
  const char *l = line;

  l = parse_gp_regs_list (l, regs, MGET_MSET_MAX_REGS, regs_read);

  if (l == NULL ||
      *regs_read == 0)
    {
      as_bad (_("invalid destination register list"));
      return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_mget_mset_addr (l, addr);

  if (l == NULL)
    {
	  as_bad (_("invalid memory operand"));
	  return NULL;
    }

  return l;
}

/* Parse an MSET instruction.  */
static const char *
parse_mset (const char *line, const metag_reg **regs, metag_addr *addr,
	    size_t *regs_read)
{
  const char *l = line;

  l = parse_mget_mset_addr (l, addr);

  if (l == NULL)
    {
	  as_bad (_("invalid memory operand"));
	  return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_gp_regs_list (l, regs, MGET_MSET_MAX_REGS, regs_read);

  if (l == NULL ||
      *regs_read == 0)
    {
      as_bad (_("invalid source register list"));
      return NULL;
    }

  return l;
}

/* Take a register list REGS of size REGS_READ and convert it into an
   rmask value if possible. Return the rmask value in RMASK and the
   lowest numbered register in LOWEST_REG. Return TRUE if the conversion
   was successful.  */
static bool
check_rmask (const metag_reg **regs, size_t regs_read, bool is_fpu,
	     bool is_64bit, unsigned int *lowest_reg,
	     unsigned int *rmask)
{
  unsigned int reg_unit = regs[0]->unit;
  size_t i;

  for (i = 0; i < regs_read; i++)
    {
      if (is_fpu)
	{
	  if (is_64bit && regs[i]->no % 2)
	    {
	      as_bad (_("register list must be even numbered"));
	      return false;
	    }
	}
      else if (regs[i]->unit != reg_unit)
	{
	  as_bad (_("register list must be from the same unit"));
	  return false;
	}

      if (regs[i]->no < *lowest_reg)
	*lowest_reg = regs[i]->no;
    }

  for (i = 0; i < regs_read; i++)
    {
      unsigned int next_bit, next_reg;
      if (regs[i]->no == *lowest_reg)
	continue;

      if (is_fpu && is_64bit)
	next_reg = ((regs[i]->no / 2) - ((*lowest_reg / 2) + 1));
      else
	next_reg = (regs[i]->no - (*lowest_reg + 1));

      next_bit = (1 << next_reg);

      if (*rmask & next_bit)
	{
	  as_bad (_("register list must not contain duplicates"));
	  return false;
	}

      *rmask |= next_bit;
    }

  return true;
}

/* Parse an MGET or MSET instruction.  */
static const char *
parse_mget_mset (const char *line, metag_insn *insn,
		 const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[MGET_MSET_MAX_REGS];
  metag_addr addr;
  bool is_get = MAJOR_OPCODE (template->meta_opcode) == OPC_GET;
  bool is_fpu = (MINOR_OPCODE (template->meta_opcode) & 0x6) == 0x6;
  bool is_64bit = (MINOR_OPCODE (template->meta_opcode) & 0x1) == 0x1;
  size_t regs_read = 0;
  unsigned int rmask = 0, reg_unit = 0, lowest_reg = 0xffffffff;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  if (is_get)
    {
      l = parse_mget (l, regs, &addr, &regs_read);
    }
  else
    {
      l = parse_mset (l, regs, &addr, &regs_read);
    }

  if (l == NULL)
    return NULL;

  if (!check_rmask (regs, regs_read, is_fpu, is_64bit, &lowest_reg, &rmask))
    return NULL;

  reg_unit = regs[0]->unit;

  if (is_fpu)
    {
      if (reg_unit != UNIT_FX)
	return NULL;

      reg_unit = 0;
    }
  else if (reg_unit == UNIT_FX)
    return NULL;

  insn->bits = (template->meta_opcode |
		(lowest_reg << 19) |
		((reg_unit & SHORT_UNIT_MASK) << 3));

  if (!is_short_unit (addr.base_reg->unit))
    {
      as_bad (_("base unit must be one of %s"), SHORT_UNITS);
      return NULL;
    }

  insn->bits |= ((addr.base_reg->no << 14) |
		 ((addr.base_reg->unit & SHORT_UNIT_MASK) << 5));

  insn->bits |= (rmask & RMASK_MASK) << 7;

  insn->len = 4;
  return l;
}

/* Parse a list of registers for MMOV pipeline prime.  */
static const char *
parse_mmov_prime_list (const char *line, const metag_reg **regs,
		       unsigned int *rmask)
{
  const char *l = line;
  const metag_reg *ra_regs[MMOV_MAX_REGS];
  size_t regs_read = 0, i;
  unsigned int mask = 0;

  l = parse_gp_regs_list (l, regs, 1, &regs_read);

  /* First register must be a port. */
  if (l == NULL || regs[0]->unit != UNIT_RD)
    return NULL;

  l = skip_comma (l);

  if (l == NULL)
    return NULL;

  l = parse_gp_regs_list (l, ra_regs, MMOV_MAX_REGS, &regs_read);

  if (l == NULL)
    return NULL;

  /* Check remaining registers match the first.

     Note that we also accept RA (0x10) as input for the remaining registers.
     Whilst this doesn't represent the instruction in any way we're stuck
     with it because the embedded assembler accepts it.  */
  for (i = 0; i < regs_read; i++)
    {
      if (ra_regs[i]->unit != UNIT_RD ||
	  (ra_regs[i]->no != 0x10 && ra_regs[i]->no != regs[0]->no))
	return NULL;

      mask = (mask << 1) | 0x1;
    }

  *rmask = mask;

  return l;
}

/* Parse a MMOV instruction.  */
static const char *
parse_mmov (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  bool is_fpu = template->insn_type == INSN_FPU;
  bool is_prime = (MINOR_OPCODE (template->meta_opcode) & 0x2) != 0 && !is_fpu;
  bool is_64bit = (MINOR_OPCODE (template->meta_opcode) & 0x1) != 0;
  unsigned int rmask = 0;

  if (is_prime)
    {
      const metag_reg *reg;
      metag_addr addr;

      memset (&addr, 0, sizeof(addr));

      l = parse_mmov_prime_list (l, &reg, &rmask);

      if (l == NULL)
	return NULL;

      l = skip_comma (l);

      if (l == NULL)
	return NULL;

      l = parse_mget_mset_addr (l, &addr);

      if (l == NULL)
	{
	  as_bad (_("invalid memory operand"));
	  return NULL;
	}

      insn->bits = (template->meta_opcode |
		    (reg->no << 19) |
		    (addr.base_reg->no << 14) |
		    ((rmask & RMASK_MASK) << 7) |
		    ((addr.base_reg->unit & SHORT_UNIT_MASK) << 5));
    }
  else
    {
      const metag_reg *regs[MMOV_MAX_REGS + 1];
      unsigned int lowest_reg = 0xffffffff;
      size_t regs_read = 0;

      l = parse_gp_regs_list (l, regs, MMOV_MAX_REGS + 1, &regs_read);

      if (l == NULL || regs_read == 0)
	return NULL;

      if (!is_short_unit (regs[0]->unit) &&
	  !(is_fpu && regs[0]->unit == UNIT_FX))
	{
	  return NULL;
	}

      if (!(regs[regs_read-1]->unit == UNIT_RD &&
	    regs[regs_read-1]->no == 0))
	{
	  return NULL;
	}

      if (!check_rmask (regs, regs_read - 1, is_fpu, is_64bit, &lowest_reg,
			&rmask))
	return NULL;

      if (is_fpu)
	{
	  insn->bits = (template->meta_opcode |
			(regs[0]->no << 14) |
			((rmask & RMASK_MASK) << 7));
	}
      else
	{
	  insn->bits = (template->meta_opcode |
			(regs[0]->no << 19) |
			((rmask & RMASK_MASK) << 7) |
			((regs[0]->unit & SHORT_UNIT_MASK) << 3));
	}
    }

  insn->len = 4;
  return l;
}

/* Parse an immediate constant.  */
static const char *
parse_imm_constant (const char *line, metag_insn *insn, int *value)
{
  const char *l = line;
  char *save_input_line_pointer;
  expressionS *exp = &insn->reloc_exp;

  /* Skip #. */
  if (*l == '#')
    l++;
  else
    return NULL;

  save_input_line_pointer = input_line_pointer;
  input_line_pointer = (char *) l;

  expression (exp);

  l = input_line_pointer;
  input_line_pointer = save_input_line_pointer;

  if (exp->X_op == O_constant)
    {
      *value = exp->X_add_number;

      return l;
    }
  else
    {
      return NULL;
    }
}

/* Parse an MDRD instruction.  */
static const char *
parse_mdrd (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  unsigned int rmask = 0;
  int value = 0, i;

  l = parse_imm_constant (l, insn, &value);

  if (l == NULL)
    return NULL;

  if (value < 1 || value > 8)
    {
      as_bad (_("MDRD value must be between 1 and 8"));
      return NULL;
    }

  for (i = 1; i < value; i++)
    {
      rmask <<= 1;
      rmask |= 1;
    }

  insn->bits = (template->meta_opcode |
		(rmask << 7));

  insn->len = 4;
  return l;
}

/* Parse a conditional SET instruction.  */
static const char *
parse_cond_set (const char *line, metag_insn *insn,
		const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];
  metag_addr addr;
  unsigned int size = metag_cond_set_size_bytes (template->meta_opcode);
  unsigned int reg_no;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  l = parse_set (l, regs, &addr, size);

  if (l == NULL)
    return NULL;

  if (regs[0]->unit == UNIT_RD)
    {
      if (regs[0]->no != 0)
	{
	  as_bad (_("set can only use RD port as source"));
	  return NULL;
	}
      reg_no = 16;
    }
  else
    reg_no = regs[0]->no;

  if (addr.update)
    return NULL;

  if (!(addr.immediate &&
	addr.exp.X_add_number == 0))
    return NULL;

  insn->bits = (template->meta_opcode |
		(reg_no << 19) |
		(regs[0]->unit << 10));

  if (!is_short_unit (addr.base_reg->unit))
    {
      as_bad (_("base unit must be one of %s"), SHORT_UNITS);
      return NULL;
    }

  insn->bits |= ((addr.base_reg->no << 14) |
		 ((addr.base_reg->unit & SHORT_UNIT_MASK) << 5));

  insn->len = 4;
  return l;
}

/* Parse an XFR instruction.  */
static const char *
parse_xfr (const char *line, metag_insn *insn,
	   const insn_template *template)
{
  const char *l = line;
  metag_addr dest_addr, src_addr;
  unsigned int size = 4;

  memset(&dest_addr, 0, sizeof(dest_addr));
  memset(&src_addr, 0, sizeof(src_addr));
  dest_addr.reloc_type = BFD_RELOC_UNUSED;
  src_addr.reloc_type = BFD_RELOC_UNUSED;

  l = parse_addr (l, &dest_addr, size);

  if (l == NULL ||
      dest_addr.immediate == 1)
    {
	  as_bad (_("invalid destination memory operand"));
	  return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_addr (l, &src_addr, size);

  if (l == NULL ||
      src_addr.immediate == 1)
    {
	  as_bad (_("invalid source memory operand"));
	  return NULL;
    }

  if (!is_short_unit (dest_addr.base_reg->unit) ||
      !is_short_unit (src_addr.base_reg->unit))
    {
      as_bad (_("address units must be one of %s"), SHORT_UNITS);
      return NULL;
    }

  if ((dest_addr.base_reg->unit != dest_addr.offset_reg->unit) ||
      (src_addr.base_reg->unit != src_addr.offset_reg->unit))
    {
      as_bad (_("base and offset must be from the same unit"));
      return NULL;
    }

  if (dest_addr.update == 1 &&
      src_addr.update == 1 &&
      dest_addr.post_increment != src_addr.post_increment)
    {
      as_bad (_("source and destination increment mode must agree"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(src_addr.base_reg->no << 19) |
		(src_addr.offset_reg->no << 14) |
		((src_addr.base_reg->unit & SHORT_UNIT_MASK) << 2));

  insn->bits |= ((dest_addr.base_reg->no << 9) |
		 (dest_addr.offset_reg->no << 4) |
		 ((dest_addr.base_reg->unit & SHORT_UNIT_MASK)));

  if (dest_addr.update == 1)
    insn->bits |= (1 << 26);

  if (src_addr.update == 1)
    insn->bits |= (1 << 27);

  if (dest_addr.post_increment == 1 ||
      src_addr.post_increment == 1)
    insn->bits |= (1 << 24);

  insn->len = 4;
  return l;
}

/* Parse an 8bit immediate value.  */
static const char *
parse_imm8 (const char *line, metag_insn *insn, int *value)
{
  const char *l = line;
  char *save_input_line_pointer;
  expressionS *exp = &insn->reloc_exp;

  /* Skip #. */
  if (*l == '#')
    l++;
  else
    return NULL;

  save_input_line_pointer = input_line_pointer;
  input_line_pointer = (char *) l;

  expression (exp);

  l = input_line_pointer;
  input_line_pointer = save_input_line_pointer;

  if (exp->X_op == O_absent || exp->X_op == O_big)
    {
      return NULL;
    }
  else if (exp->X_op == O_constant)
    {
      *value = exp->X_add_number;
    }
  else
    {
      insn->reloc_type = BFD_RELOC_METAG_REL8;
      insn->reloc_pcrel = 0;
    }

  return l;
}

/* Parse a 16bit immediate value.  */
static const char *
parse_imm16 (const char *line, metag_insn *insn, int *value)
{
  const char *l = line;
  char *save_input_line_pointer;
  expressionS *exp = &insn->reloc_exp;
  bool is_hi = false;
  bool is_lo = false;

  /* Skip #. */
  if (*l == '#')
    l++;
  else
    return NULL;

  if (strncasecmp (l, "HI", 2) == 0)
    {
      is_hi = true;
      l += 2;
    }
  else if (strncasecmp (l, "LO", 2) == 0)
    {
      is_lo = true;
      l += 2;
    }

  save_input_line_pointer = input_line_pointer;
  input_line_pointer = (char *) l;

  expression (exp);

  l = input_line_pointer;
  input_line_pointer = save_input_line_pointer;

  if (exp->X_op == O_absent || exp->X_op == O_big)
    {
      return NULL;
    }
  else if (exp->X_op == O_constant)
    {
      if (is_hi)
	*value = (exp->X_add_number >> 16) & IMM16_MASK;
      else if (is_lo)
	*value = exp->X_add_number & IMM16_MASK;
      else
	*value = exp->X_add_number;
    }
  else
    {
      if (exp->X_op == O_PIC_reloc)
	{
	  exp->X_op = O_symbol;

	  if (exp->X_md == BFD_RELOC_METAG_GOTOFF)
	    {
	      if (is_hi)
		insn->reloc_type = BFD_RELOC_METAG_HI16_GOTOFF;
	      else if (is_lo)
		insn->reloc_type = BFD_RELOC_METAG_LO16_GOTOFF;
	      else
		return NULL;
	    }
	  else if (exp->X_md == BFD_RELOC_METAG_PLT)
	    {
	      if (is_hi)
		insn->reloc_type = BFD_RELOC_METAG_HI16_PLT;
	      else if (is_lo)
		insn->reloc_type = BFD_RELOC_METAG_LO16_PLT;
	      else
		return NULL;
	    }
	  else if (exp->X_md == BFD_RELOC_METAG_TLS_LDO)
	    {
	      if (is_hi)
		insn->reloc_type = BFD_RELOC_METAG_TLS_LDO_HI16;
	      else if (is_lo)
		insn->reloc_type = BFD_RELOC_METAG_TLS_LDO_LO16;
	      else
		return NULL;
	    }
	  else if (exp->X_md == BFD_RELOC_METAG_TLS_IENONPIC)
	    {
	      if (is_hi)
		insn->reloc_type = BFD_RELOC_METAG_TLS_IENONPIC_HI16;
	      else if (is_lo)
		insn->reloc_type = BFD_RELOC_METAG_TLS_IENONPIC_LO16;
	      else
		return NULL;
	    }
	  else if (exp->X_md == BFD_RELOC_METAG_TLS_LE)
	    {
	      if (is_hi)
		insn->reloc_type = BFD_RELOC_METAG_TLS_LE_HI16;
	      else if (is_lo)
		insn->reloc_type = BFD_RELOC_METAG_TLS_LE_LO16;
	      else
		return NULL;
	    }
	  else if (exp->X_md == BFD_RELOC_METAG_TLS_GD ||
		   exp->X_md == BFD_RELOC_METAG_TLS_LDM)
	    insn->reloc_type = exp->X_md;
	}
      else
	{
	  if (exp->X_op == O_symbol && exp->X_add_symbol == GOT_symbol)
	    {
	      if (is_hi)
		insn->reloc_type = BFD_RELOC_METAG_HI16_GOTPC;
	      else if (is_lo)
		insn->reloc_type = BFD_RELOC_METAG_LO16_GOTPC;
	      else
		return NULL;
	    }
	  else
	    {
	      if (is_hi)
		insn->reloc_type = BFD_RELOC_METAG_HIADDR16;
	      else if (is_lo)
		insn->reloc_type = BFD_RELOC_METAG_LOADDR16;
	      else
		insn->reloc_type = BFD_RELOC_METAG_REL16;
	    }
	}

      insn->reloc_pcrel = 0;
    }

  return l;
}

/* Parse a MOV to control unit instruction.  */
static const char *
parse_mov_ct (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[1];
  bool top = (template->meta_opcode & 0x1) != 0;
  bool is_trace = ((template->meta_opcode >> 2) & 0x1) != 0;
  bool sign_extend = 0;
  int value = 0;

  l = parse_gp_regs (l, regs, 1);

  if (l == NULL)
    return NULL;

  if (is_trace)
    {
      if (regs[0]->unit != UNIT_TT)
	return NULL;
    }
  else
    {
      if (regs[0]->unit != UNIT_CT)
	return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_imm16 (l, insn, &value);

  if (l == NULL)
    return NULL;

  if (value < 0)
    sign_extend = 1;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		((value & IMM16_MASK) << 3));

  if (sign_extend && !top)
    insn->bits |= (1 << 1);

  insn->len = 4;
  return l;
}

/* Parse a SWAP instruction.  */
static const char *
parse_swap (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];

  l = parse_gp_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  /* PC.r | CT.r | TR.r | TT.r are treated as if they are a single unit.  */
  switch (regs[0]->unit)
    {
    case UNIT_PC:
    case UNIT_CT:
    case UNIT_TR:
    case UNIT_TT:
      if (regs[1]->unit == UNIT_PC
	  || regs[1]->unit == UNIT_CT
	  || regs[1]->unit == UNIT_TR
	  || regs[1]->unit == UNIT_TT)
	{
	  as_bad (_("PC, CT, TR and TT are treated as if they are a single unit but operands must be in different units"));
	  return NULL;
	}
      break;

    default:
      /* Registers must be in different units.  */
      if (regs[0]->unit == regs[1]->unit)
	{
	  as_bad (_("source and destination register must be in different units"));
	  return NULL;
	}
      break;
    }

  insn->bits = (template->meta_opcode
		| (regs[1]->no << 19)
		| (regs[0]->no << 14)
		| (regs[1]->unit << 10)
		| (regs[0]->unit << 5));

  insn->len = 4;
  return l;
}

/* Parse a JUMP instruction.  */
static const char *
parse_jump (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[1];
  int value = 0;

  l = parse_gp_regs (l, regs, 1);

  if (l == NULL)
    return NULL;

  if (!is_short_unit (regs[0]->unit))
    {
      as_bad (_("register unit must be one of %s"), SHORT_UNITS);
      return false;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_imm16 (l, insn, &value);

  if (l == NULL)
    return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[0]->unit & SHORT_UNIT_MASK) |
		((value & IMM16_MASK) << 3));

  insn->len = 4;
  return l;
}

/* Parse a 19bit immediate value.  */
static const char *
parse_imm19 (const char *line, metag_insn *insn, int *value)
{
  const char *l = line;
  char *save_input_line_pointer;
  expressionS *exp = &insn->reloc_exp;

  /* Skip #.  */
  if (*l == '#')
    l++;

  save_input_line_pointer = input_line_pointer;
  input_line_pointer = (char *) l;

  expression (exp);

  l = input_line_pointer;
  input_line_pointer = save_input_line_pointer;

  if (exp->X_op == O_absent || exp->X_op == O_big)
    {
      return NULL;
    }
  else if (exp->X_op == O_constant)
    {
      *value = exp->X_add_number;
    }
  else
    {
      if (exp->X_op == O_PIC_reloc)
	{
	  exp->X_op = O_symbol;

	  if (exp->X_md == BFD_RELOC_METAG_PLT)
	    insn->reloc_type = BFD_RELOC_METAG_RELBRANCH_PLT;
	  else
	    return NULL;
	}
      else
	insn->reloc_type = BFD_RELOC_METAG_RELBRANCH;
      insn->reloc_pcrel = 1;
    }

  return l;
}

/* Parse a CALLR instruction.  */
static const char *
parse_callr (const char *line, metag_insn *insn,
	     const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[1];
  int value = 0;

  l = parse_gp_regs (l, regs, 1);

  if (l == NULL)
    return NULL;

  if (!is_short_unit (regs[0]->unit))
    {
      as_bad (_("link register unit must be one of %s"), SHORT_UNITS);
      return NULL;
    }

  if (regs[0]->no & ~CALLR_REG_MASK)
    {
      as_bad (_("link register must be in a low numbered register"));
      return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_imm19 (l, insn, &value);

  if (l == NULL)
    return NULL;

  if (!within_signed_range (value / 4, IMM19_BITS))
    {
      as_bad (_("target out of range"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(regs[0]->no & CALLR_REG_MASK) |
		((regs[0]->unit & SHORT_UNIT_MASK) << 3) |
		((value & IMM19_MASK) << 5));

  insn->len = 4;
  return l;
}

/* Return the value for the register field if we apply the O2R modifier
   to operand 2 REG, combined with UNIT_BIT derived from the destination
   register or source1. Uses address unit O2R if IS_ADDR is set.  */
static int
lookup_o2r (unsigned int is_addr, unsigned int unit_bit, const metag_reg *reg)
{
  if (reg->no & ~O2R_REG_MASK)
    return -1;

  if (is_addr)
    {
      if (unit_bit)
	{
	  switch (reg->unit)
	    {
	    case UNIT_D1:
	      return reg->no;
	    case UNIT_D0:
	      return (1 << 3) | reg->no;
	    case UNIT_RD:
	      return (2 << 3) | reg->no;
	    case UNIT_A0:
	      return (3 << 3) | reg->no;
	    default:
	      return -1;
	    }
	}
      else
	{
	  switch (reg->unit)
	    {
	    case UNIT_A1:
	      return reg->no;
	    case UNIT_D0:
	      return (1 << 3) | reg->no;
	    case UNIT_RD:
	      return (2 << 3) | reg->no;
	    case UNIT_D1:
	      return (3 << 3) | reg->no;
	    default:
	      return -1;
	    }
	}
    }
  else
    {
      if (unit_bit)
	{
	  switch (reg->unit)
	    {
	    case UNIT_A1:
	      return reg->no;
	    case UNIT_D0:
	      return (1 << 3) | reg->no;
	    case UNIT_RD:
	      return (2 << 3) | reg->no;
	    case UNIT_A0:
	      return (3 << 3) | reg->no;
	    default:
	      return -1;
	    }
	}
      else
	{
	  switch (reg->unit)
	    {
	    case UNIT_A1:
	      return reg->no;
	    case UNIT_D1:
	      return (1 << 3) | reg->no;
	    case UNIT_RD:
	      return (2 << 3) | reg->no;
	    case UNIT_A0:
	      return (3 << 3) | reg->no;
	    default:
	      return -1;
	    }
	}
    }
}

/* Parse GP ALU instruction.  */
static const char *
parse_alu (const char *line, metag_insn *insn,
	   const insn_template *template)
{
  const char *l = line;
  const metag_reg *dest_regs[1];
  const metag_reg *src_regs[2];
  int value = 0;
  bool o1z = 0;
  bool imm = ((template->meta_opcode >> 25) & 0x1) != 0;
  bool cond = ((template->meta_opcode >> 26) & 0x1) != 0;
  bool ca = ((template->meta_opcode >> 5) & 0x1) != 0;
  bool top = (template->meta_opcode & 0x1) != 0;
  bool sign_extend = 0;
  bool is_addr_op = MAJOR_OPCODE (template->meta_opcode) == OPC_ADDR;
  bool is_mul = MAJOR_OPCODE (template->meta_opcode) == OPC_MUL;
  unsigned int unit_bit = 0;
  bool is_quickrot = (template->arg_type & GP_ARGS_QR) != 0;

  l = parse_gp_regs (l, dest_regs, 1);

  if (l == NULL)
    return NULL;

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  if (is_addr_op)
    {
      if (dest_regs[0]->unit == UNIT_A0)
	unit_bit = 0;
      else if (dest_regs[0]->unit == UNIT_A1)
	unit_bit = 1;
    }
  else
    {
      if (dest_regs[0]->unit == UNIT_D0)
	unit_bit = 0;
      else if (dest_regs[0]->unit == UNIT_D1)
	unit_bit = 1;
    }

  if ((MAJOR_OPCODE (template->meta_opcode) == OPC_ADDR ||
      MAJOR_OPCODE (template->meta_opcode) == OPC_ADD ||
       MAJOR_OPCODE (template->meta_opcode) == OPC_SUB) &&
      ((template->meta_opcode >> 2) & 0x1))
    o1z = 1;

  if (imm)
    {
      if (!cond)
	{
	  if (is_addr_op)
	    {
	      if (dest_regs[0]->unit == UNIT_A0)
		unit_bit = 0;
	      else if (dest_regs[0]->unit == UNIT_A1)
		unit_bit = 1;
	      else
		return NULL;
	    }
	  else
	    {
	      if (dest_regs[0]->unit == UNIT_D0)
		unit_bit = 0;
	      else if (dest_regs[0]->unit == UNIT_D1)
		unit_bit = 1;
	      else
		return NULL;
	    }
	}

      if (cond)
	{
	  l = parse_gp_regs (l, src_regs, 1);

	  if (l == NULL)
	    return NULL;

	  l = skip_comma (l);

	  if (l == NULL ||
	      *l == END_OF_INSN)
	    return NULL;

	  if (is_addr_op)
	    {
	      if (src_regs[0]->unit == UNIT_A0)
		unit_bit = 0;
	      else if (src_regs[0]->unit == UNIT_A1)
		unit_bit = 1;
	      else
		return NULL;
	    }
	  else
	    {
	      if (src_regs[0]->unit == UNIT_D0)
		unit_bit = 0;
	      else if (src_regs[0]->unit == UNIT_D1)
		unit_bit = 1;
	      else
		return NULL;
	    }

	  if (src_regs[0]->unit != dest_regs[0]->unit && !ca)
	    return NULL;

	  l = parse_imm8 (l, insn, &value);

	  if (l == NULL)
	    return NULL;

	  if (!within_unsigned_range (value, IMM8_BITS))
	    return NULL;

	  insn->bits = (template->meta_opcode |
			(dest_regs[0]->no << 19) |
			(src_regs[0]->no << 14) |
			((value & IMM8_MASK) << 6));

	  if (ca)
	    {
	      if (is_addr_op)
		{
		  if (src_regs[0]->unit == UNIT_A0)
		    unit_bit = 0;
		  else if (src_regs[0]->unit == UNIT_A1)
		    unit_bit = 1;
		  else
		    return NULL;
		}
	      else
		{
		  if (src_regs[0]->unit == UNIT_D0)
		    unit_bit = 0;
		  else if (src_regs[0]->unit == UNIT_D1)
		    unit_bit = 1;
		  else
		    return NULL;
		}

	      insn->bits |= dest_regs[0]->unit << 1;
	    }
	}
      else if (o1z)
	{
	  l = parse_imm16 (l, insn, &value);

	  if (l == NULL)
	    return NULL;

	  if (value < 0)
	    {
	      if (!within_signed_range (value, IMM16_BITS))
		{
		  as_bad (_("immediate out of range"));
		  return NULL;
		}
	      sign_extend = 1;
	    }
	  else
	    {
	      if (!within_unsigned_range (value, IMM16_BITS))
		{
		  as_bad (_("immediate out of range"));
		  return NULL;
		}
	    }

	  insn->bits = (template->meta_opcode |
			(dest_regs[0]->no << 19) |
			((value & IMM16_MASK) << 3));
	}
      else
	{
	  l = parse_gp_regs (l, src_regs, 1);

	  if (l == NULL)
	    return NULL;

	  if (!(src_regs[0]->unit == dest_regs[0]->unit))
	    return NULL;

	  /* CPC is valid for address ops. */
	  if (src_regs[0]->no != dest_regs[0]->no &&
	      !(is_addr_op && src_regs[0]->no == 0x10))
	    return NULL;

	  l = skip_comma (l);

	  if (l == NULL ||
	      *l == END_OF_INSN)
	    return NULL;

	  l = parse_imm16 (l, insn, &value);

	  if (l == NULL)
	    return NULL;

	  if (value < 0)
	    {
	      if (!within_signed_range (value, IMM16_BITS))
		{
		  as_bad (_("immediate out of range"));
		  return NULL;
		}
	      sign_extend = 1;
	    }
	  else
	    {
	      if (!within_unsigned_range (value, IMM16_BITS))
		{
		  as_bad (_("immediate out of range"));
		  return NULL;
		}
	    }

	  insn->bits = (template->meta_opcode |
			(dest_regs[0]->no << 19) |
			(src_regs[0]->no << 19) |
			((value & IMM16_MASK) << 3));
	}
    }
  else
    {
      bool o2r = 0;
      int rs2;

      if (cond || !o1z)
	l = parse_gp_regs (l, src_regs, 2);
      else
	l = parse_gp_regs (l, src_regs, 1);

      if (l == NULL)
	return NULL;

      if (cond || !o1z)
	{
	  if (is_addr_op)
	    {
	      if (src_regs[0]->unit == UNIT_A0)
		unit_bit = 0;
	      else if (src_regs[0]->unit == UNIT_A1)
		unit_bit = 1;
	      else
		return NULL;
	    }
	  else
	    {
	      if (src_regs[0]->unit == UNIT_D0)
		unit_bit = 0;
	      else if (src_regs[0]->unit == UNIT_D1)
		unit_bit = 1;
	      else
		return NULL;
	    }
	}
      else
	{
	  if (is_addr_op)
	    {
	      if (dest_regs[0]->unit == UNIT_A0)
		unit_bit = 0;
	      else if (dest_regs[0]->unit == UNIT_A1)
		unit_bit = 1;
	      else
		return NULL;
	    }
	  else
	    {
	      if (dest_regs[0]->unit == UNIT_D0)
		unit_bit = 0;
	      else if (dest_regs[0]->unit == UNIT_D1)
		unit_bit = 1;
	      else
		return NULL;
	    }
	}

      if (cond)
	{
	  if (src_regs[0]->unit != src_regs[1]->unit)
	    {
	      rs2 = lookup_o2r (is_addr_op, unit_bit, src_regs[1]);

	      if (rs2 < 0)
		return NULL;

	      o2r = 1;
	    }
	  else
	    {
	      rs2 = src_regs[1]->no;
	    }

	  insn->bits = (template->meta_opcode |
			(dest_regs[0]->no << 19) |
			(src_regs[0]->no << 14) |
			(rs2 << 9));

	  if (is_mul)
	    {
	      if (dest_regs[0]->unit != src_regs[0]->unit && is_mul)
		{
		  if (ca)
		    {
		      insn->bits |= dest_regs[0]->unit << 1;
		    }
		  else
		    return NULL;
		}
	    }
	  else
	    insn->bits |= dest_regs[0]->unit << 5;
	}
      else if (o1z)
	{
	  if (dest_regs[0]->unit != src_regs[0]->unit)
	    {
	      rs2 = lookup_o2r (is_addr_op, unit_bit, src_regs[0]);

	      if (rs2 < 0)
		return NULL;

	      o2r = 1;
	    }
	  else
	    {
	      rs2 = src_regs[0]->no;
	    }

	  insn->bits = (template->meta_opcode |
			(dest_regs[0]->no << 19) |
			(rs2 << 9));
	}
      else
	{
	  if (dest_regs[0]->unit != src_regs[0]->unit)
	    return NULL;

	  if (dest_regs[0]->unit != src_regs[1]->unit)
	    {
	      rs2 = lookup_o2r (is_addr_op, unit_bit, src_regs[1]);

	      if (rs2 < 0)
		return NULL;

	      o2r = 1;
	    }
	  else
	    {
	      rs2 = src_regs[1]->no;
	    }

	  insn->bits = (template->meta_opcode |
			(dest_regs[0]->no << 19) |
			(src_regs[0]->no << 14) |
			(rs2 << 9));
	}

      if (o2r)
	insn->bits |= 1;
    }

  if (is_quickrot)
    {
      const metag_reg *qr_regs[1];
      bool limit_regs = imm && cond;

      l = skip_comma (l);

      if (l == NULL ||
	  *l == END_OF_INSN)
	return NULL;

      l = parse_gp_regs (l, qr_regs, 1);

      if (l == NULL)
	return NULL;

      if (!((unit_bit == 0 && qr_regs[0]->unit != UNIT_A0) ||
	    !(unit_bit == 1 && qr_regs[0]->unit != UNIT_A1)))
	{
	  as_bad (_("invalid quickrot unit specified"));
	  return NULL;
	}

      switch (qr_regs[0]->no)
	{
	case 2:
	  break;
	case 3:
	  if (!limit_regs)
	    {
	      insn->bits |= (1 << 7);
	      break;
	    }
	  /* Fall through.  */
	default:
	  as_bad (_("invalid quickrot register specified"));
	  return NULL;
	}
    }

  if (sign_extend && !top)
    insn->bits |= (1 << 1);

  insn->bits |= unit_bit << 24;
  insn->len = 4;
  return l;
}

/* Parse a B instruction.  */
static const char *
parse_branch (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  int value = 0;

  l = parse_imm19 (l, insn, &value);

  if (l == NULL)
    return NULL;

  if (!within_signed_range (value / 4, IMM19_BITS))
    {
      as_bad (_("target out of range"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		((value & IMM19_MASK) << 5));

  insn->len = 4;
  return l;
}

/* Parse a KICK instruction.  */
static const char *
parse_kick (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];

  l = parse_gp_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  if (regs[1]->unit != UNIT_TR)
    {
      as_bad (_("source register must be in the trigger unit"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(regs[1]->no << 19) |
		(regs[0]->no << 14) |
		(regs[0]->unit << 5));

  insn->len = 4;
  return l;
}

/* Parse a SWITCH instruction.  */
static const char *
parse_switch (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  int value = 0;

  l = parse_imm_constant (l, insn, &value);

  if (l == NULL)
    return NULL;

  if (!within_unsigned_range (value, IMM24_BITS))
    {
      as_bad (_("target out of range"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(value & IMM24_MASK));

  insn->len = 4;
  return l;
}

/* Parse a shift instruction.  */
static const char *
parse_shift (const char *line, metag_insn *insn,
	     const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];
  const metag_reg *src2_regs[1];
  int value = 0;
  bool cond = ((template->meta_opcode >> 26) & 0x1) != 0;
  bool ca = ((template->meta_opcode >> 5) & 0x1) != 0;
  unsigned int unit_bit = 0;

  l = parse_gp_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  if (regs[1]->unit == UNIT_D0)
    unit_bit = 0;
  else if (regs[1]->unit == UNIT_D1)
    unit_bit = 1;
  else
    return NULL;

  if (regs[0]->unit != regs[1]->unit && !(cond && ca))
    return NULL;

  if (*l == '#')
    {
      l = parse_imm_constant (l, insn, &value);

      if (l == NULL)
	return NULL;

      if (!within_unsigned_range (value, IMM5_BITS))
	return NULL;

      insn->bits = (template->meta_opcode |
		    (1 << 25) |
		    (regs[0]->no << 19) |
		    (regs[1]->no << 14) |
		    ((value & IMM5_MASK) << 9));
    }
  else
    {
      l = parse_gp_regs (l, src2_regs, 1);

      if (l == NULL)
	return NULL;

      insn->bits = (template->meta_opcode |
		    (regs[0]->no << 19) |
		    (regs[1]->no << 14) |
		    (src2_regs[0]->no << 9));

      if (src2_regs[0]->unit != regs[1]->unit)
	{
	  as_bad(_("Source registers must be in the same unit"));
	  return NULL;
	}
    }

  if (regs[0]->unit != regs[1]->unit)
    {
      if (cond && ca)
	{
	  if (regs[1]->unit == UNIT_D0)
	    unit_bit = 0;
	  else if (regs[1]->unit == UNIT_D1)
	    unit_bit = 1;
	  else
	    return NULL;

	  insn->bits |= ((1 << 5) |
			 (regs[0]->unit << 1));
	}
      else
	return NULL;
    }

  insn->bits |= unit_bit << 24;
  insn->len = 4;
  return l;
}

/* Parse a MIN or MAX instruction.  */
static const char *
parse_min_max (const char *line, metag_insn *insn,
	       const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[3];

  l = parse_gp_regs (l, regs, 3);

  if (l == NULL)
    return NULL;

  if (!(regs[0]->unit == UNIT_D0 ||
	regs[0]->unit == UNIT_D1))
      return NULL;

  if (!(regs[0]->unit == regs[1]->unit &&
	regs[1]->unit == regs[2]->unit))
      return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14) |
		(regs[2]->no << 9));

  if (regs[0]->unit == UNIT_D1)
    insn->bits |= (1 << 24);

  insn->len = 4;
  return l;
}

/* Parse a bit operation instruction.  */
static const char *
parse_bitop (const char *line, metag_insn *insn,
	     const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];
  bool swap_inst = MAJOR_OPCODE (template->meta_opcode) == OPC_MISC;
  bool is_bexl = 0;

  if (swap_inst && ((template->meta_opcode >> 1) & 0xb) == 0xa)
    is_bexl = 1;

  l = parse_gp_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  if (!(regs[0]->unit == UNIT_D0 ||
	regs[0]->unit == UNIT_D1))
      return NULL;

  if (is_bexl)
    {
      if (regs[0]->unit == UNIT_D0 &&
	  regs[1]->unit != UNIT_D1)
	return NULL;
      else if (regs[0]->unit == UNIT_D1 &&
	       regs[1]->unit != UNIT_D0)
	return NULL;
    }
  else if (!(regs[0]->unit == regs[1]->unit))
      return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14));

  if (swap_inst)
    {
      if (regs[1]->unit == UNIT_D1)
	insn->bits |= 1;
    }
  else
    {
      if (regs[1]->unit == UNIT_D1)
	insn->bits |= (1 << 24);
    }

  insn->len = 4;
  return l;
}

/* Parse a CMP or TST instruction.  */
static const char *
parse_cmp (const char *line, metag_insn *insn,
	   const insn_template *template)
{
  const char *l = line;
  const metag_reg *dest_regs[1];
  const metag_reg *src_regs[1];
  int value = 0;
  bool imm = ((template->meta_opcode >> 25) & 0x1) != 0;
  bool cond = ((template->meta_opcode >> 26) & 0x1) != 0;
  bool top = (template->meta_opcode & 0x1) != 0;
  bool sign_extend = 0;
  unsigned int unit_bit = 0;

  l = parse_gp_regs (l, dest_regs, 1);

  if (l == NULL)
    return NULL;

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  if (dest_regs[0]->unit == UNIT_D0)
    unit_bit = 0;
  else if (dest_regs[0]->unit == UNIT_D1)
    unit_bit = 1;
  else
    return NULL;

  if (imm)
    {
      if (cond)
	{
	  l = parse_imm_constant (l, insn, &value);

	  if (l == NULL)
	    return NULL;

	  if (!within_unsigned_range (value, IMM8_BITS))
	    return NULL;

	  insn->bits = (template->meta_opcode |
			(dest_regs[0]->no << 14) |
			((value & IMM8_MASK) << 6));

	}
      else
	{
	  l = parse_imm16 (l, insn, &value);

	  if (l == NULL)
	    return NULL;

	  if (value < 0)
	    {
	      if (!within_signed_range (value, IMM16_BITS))
		{
		  as_bad (_("immediate out of range"));
		  return NULL;
		}
	      sign_extend = 1;
	    }
	  else
	    {
	      if (!within_unsigned_range (value, IMM16_BITS))
		{
		  as_bad (_("immediate out of range"));
		  return NULL;
		}
	    }

	  insn->bits = (template->meta_opcode |
			(dest_regs[0]->no << 19) |
			((value & IMM16_MASK) << 3));
	}
    }
  else
    {
      bool o2r = 0;
      int rs2;

      l = parse_gp_regs (l, src_regs, 1);

      if (l == NULL)
	return NULL;

      if (dest_regs[0]->unit != src_regs[0]->unit)
	{
	  rs2 = lookup_o2r (0, unit_bit, src_regs[0]);

	  if (rs2 < 0)
	    return NULL;

	  o2r = 1;
	}
      else
	{
	  rs2 = src_regs[0]->no;
	}

      insn->bits = (template->meta_opcode |
		    (dest_regs[0]->no << 14) |
		    (rs2 << 9));

      if (o2r)
	insn->bits |= 1;
    }

  if (sign_extend && !top)
    insn->bits |= (1 << 1);

  insn->bits |= unit_bit << 24;
  insn->len = 4;
  return l;
}

/* Parse a CACHEW instruction.  */
static const char *
parse_cachew (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  const metag_reg *src_regs[2];
  unsigned int size = ((template->meta_opcode >> 1) & 0x1) ? 8 : 4;
  metag_addr addr;
  int offset;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  l = parse_addr (l, &addr, size);

  if (l == NULL ||
      !is_short_unit (addr.base_reg->unit) ||
      addr.update ||
      !addr.immediate)
    {
	  as_bad (_("invalid memory operand"));
	  return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  if (size == 4)
    l = parse_gp_regs (l, src_regs, 1);
  else
    l = parse_pair_gp_regs (l, src_regs);

  if (l == NULL ||
      !is_short_unit (src_regs[0]->unit))
    {
      as_bad (_("invalid source register"));
      return NULL;
    }

  offset = addr.exp.X_add_number;

  if (addr.negate)
    offset = -offset;

  offset = offset / 64;

  if (!within_signed_range (offset, GET_SET_IMM_BITS))
    {
      as_bad (_("offset value out of range"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(src_regs[0]->no << 19) |
		(addr.base_reg->no << 14) |
		((offset & GET_SET_IMM_MASK) << 8) |
		((addr.base_reg->unit & SHORT_UNIT_MASK) << 5) |
		((src_regs[0]->unit & SHORT_UNIT_MASK) << 3));

  insn->len = 4;
  return l;
}

/* Parse a CACHEW instruction.  */
static const char *
parse_cacher (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  const metag_reg *dest_regs[2];
  unsigned int size = ((template->meta_opcode >> 1) & 0x1) ? 8 : 4;
  metag_addr addr;
  int offset;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  if (size == 4)
    l = parse_gp_regs (l, dest_regs, 1);
  else
    l = parse_pair_gp_regs (l, dest_regs);

  if (l == NULL ||
      !is_short_unit (dest_regs[0]->unit))
    {
      as_bad (_("invalid destination register"));
      return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_addr (l, &addr, size);

  if (l == NULL ||
      !is_short_unit (addr.base_reg->unit) ||
      addr.update ||
      !addr.immediate)
    {
	  as_bad (_("invalid memory operand"));
	  return NULL;
    }

  offset = addr.exp.X_add_number;

  if (addr.negate)
    offset = -offset;

  offset = offset / (int)size;

  if (!within_signed_range (offset, GET_SET_IMM_BITS))
    {
      as_bad (_("offset value out of range"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(dest_regs[0]->no << 19) |
		(addr.base_reg->no << 14) |
		((offset & GET_SET_IMM_MASK) << 8) |
		((addr.base_reg->unit & SHORT_UNIT_MASK) << 5) |
		((dest_regs[0]->unit & SHORT_UNIT_MASK) << 3));

  insn->len = 4;
  return l;
}

/* Parse an ICACHE instruction.  */
static const char *
parse_icache (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  int offset;
  int pfcount;

  l = parse_imm_constant (l, insn, &offset);

  if (l == NULL)
    return NULL;

  if (!within_signed_range (offset, IMM15_BITS))
    return NULL;

  l = skip_comma (l);

  l = parse_imm_constant (l, insn, &pfcount);

  if (l == NULL)
    return NULL;

  if (!within_unsigned_range (pfcount, IMM4_BITS))
    return NULL;

  insn->bits = (template->meta_opcode |
		((offset & IMM15_MASK) << 9) |
		((pfcount & IMM4_MASK) << 1));

  insn->len = 4;
  return l;
}

/* Parse a LNKGET instruction.  */
static const char *
parse_lnkget (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  const metag_reg *dest_regs[2];
  unsigned int size = metag_get_set_ext_size_bytes (template->meta_opcode);
  metag_addr addr;
  int offset;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  if (size == 8)
    l = parse_pair_gp_regs (l, dest_regs);
  else
    l = parse_gp_regs (l, dest_regs, 1);

  if (l == NULL ||
      !is_short_unit (dest_regs[0]->unit))
    {
      as_bad (_("invalid destination register"));
      return NULL;
    }

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_addr (l, &addr, size);

  if (l == NULL ||
      !is_short_unit (addr.base_reg->unit) ||
      addr.update ||
      !addr.immediate)
    {
	  as_bad (_("invalid memory operand"));
	  return NULL;
    }

  offset = addr.exp.X_add_number;

  if (addr.negate)
    offset = -offset;

  offset = offset / size;

  if (!within_signed_range (offset, GET_SET_IMM_BITS))
    {
      as_bad (_("offset value out of range"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(dest_regs[0]->no << 19) |
		(addr.base_reg->no << 14) |
		((offset & GET_SET_IMM_MASK) << 8) |
		((addr.base_reg->unit & SHORT_UNIT_MASK) << 5) |
		((dest_regs[0]->unit & SHORT_UNIT_MASK) << 3));

  insn->len = 4;
  return l;
}

/* Parse an FPU MOV instruction.  */
static const char *
parse_fmov (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];

  l = parse_fpu_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14));

  if (insn->fpu_width == FPU_WIDTH_DOUBLE)
    insn->bits |= (1 << 5);
  else if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 6);

  insn->len = 4;
  return l;
}

/* Parse an FPU MMOV instruction.  */
static const char *
parse_fmmov (const char *line, metag_insn *insn,
	     const insn_template *template)
{
  const char *l = line;
  bool to_fpu = MAJOR_OPCODE (template->meta_opcode) == OPC_GET;
  bool is_mmovl = (MINOR_OPCODE (template->meta_opcode) & 0x1) != 0;
  size_t regs_read = 0;
  const metag_reg *regs[16];
  unsigned int lowest_data_reg = 0xffffffff;
  unsigned int lowest_fpu_reg = 0xffffffff;
  unsigned int rmask = 0, data_unit;
  size_t i;
  int last_reg = -1;

  if (insn->fpu_width != FPU_WIDTH_SINGLE)
    return NULL;

  l = parse_gp_regs_list (l, regs, 16, &regs_read);

  if (l == NULL)
    return NULL;

  if (regs_read % 2)
    return NULL;

  if (to_fpu)
    {
      for (i = 0; i < regs_read / 2; i++)
	{
	  if (regs[i]->unit != UNIT_FX)
	    return NULL;

	  if (last_reg == -1)
	    {
	      last_reg = regs[i]->no;
	      lowest_fpu_reg = last_reg;
	    }
	  else
	    {
	      if (is_mmovl)
		{
		  if (regs[i]->no != (unsigned int)(last_reg + 2))
		    return NULL;
		}
	      else if (regs[i]->no != (unsigned int)(last_reg + 1))
		return NULL;

	      last_reg = regs[i]->no;
	    }
	}

      if (regs[i]->unit == UNIT_D0)
	data_unit = 0;
      else if (regs[i]->unit == UNIT_D1)
	data_unit = 1;
      else
	return NULL;

      if (!check_rmask (&regs[i], regs_read / 2, true, false, &lowest_data_reg,
			&rmask))
	return NULL;
    }
  else
    {
      if (regs[0]->unit == UNIT_D0)
	data_unit = 0;
      else if (regs[0]->unit == UNIT_D1)
	data_unit = 1;
      else
	return NULL;

      if (!check_rmask (regs, regs_read / 2, true, false, &lowest_data_reg,
			&rmask))
	return NULL;

      for (i = regs_read / 2; i < regs_read; i++)
	{
	  if (regs[i]->unit != UNIT_FX)
	    return NULL;

	  if (last_reg == -1)
	    {
	      last_reg = regs[i]->no;
	      lowest_fpu_reg = last_reg;
	    }
	  else
	    {
	      if (is_mmovl)
		{
		  if (regs[i]->no != (unsigned int)(last_reg + 2))
		    return NULL;
		}
	      else if (regs[i]->no != (unsigned int)(last_reg + 1))
		return NULL;

	      last_reg = regs[i]->no;
	    }
	}
    }

  insn->bits = (template->meta_opcode |
		((lowest_data_reg & REG_MASK) << 19) |
		((lowest_fpu_reg & REG_MASK) << 14) |
		((rmask & RMASK_MASK) << 7) |
		data_unit);

  insn->len = 4;
  return l;
}

/* Parse an FPU data unit MOV instruction.  */
static const char *
parse_fmov_data (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  bool to_fpu = ((template->meta_opcode >> 7) & 0x1) != 0;
  const metag_reg *regs[2];
  unsigned int base_unit;

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    return NULL;

  l = parse_gp_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  if (to_fpu)
    {
      if (regs[0]->unit != UNIT_FX)
	return NULL;

      if (regs[1]->unit == UNIT_D0)
	base_unit = 0;
      else if (regs[1]->unit == UNIT_D1)
	base_unit = 1;
      else
	return NULL;
    }
  else
    {
      if (regs[0]->unit == UNIT_D0)
	base_unit = 0;
      else if (regs[0]->unit == UNIT_D1)
	base_unit = 1;
      else
	return NULL;

      if (regs[1]->unit != UNIT_FX)
	return NULL;
    }

  insn->bits = (template->meta_opcode |
		(base_unit << 24) |
		(regs[0]->no << 19) |
		(regs[1]->no << 9));

  insn->len = 4;
  return l;
}

/* Parse an FPU immediate MOV instruction.  */
static const char *
parse_fmov_i (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[1];
  int value = 0;

  l = parse_fpu_regs (l, regs, 1);

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_imm16 (l, insn, &value);

  if (l == NULL)
    return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		((value & IMM16_MASK) << 3));

  if (insn->fpu_width == FPU_WIDTH_DOUBLE)
    insn->bits |= (1 << 1);
  else if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 2);

  insn->len = 4;
  return l;
}

/* Parse an FPU PACK instruction.  */
static const char *
parse_fpack (const char *line, metag_insn *insn,
	     const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[3];

  l = parse_fpu_regs (l, regs, 3);

  if (l == NULL)
    return NULL;

  if (regs[0]->no % 2)
    {
      as_bad (_("destination register should be even numbered"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14) |
		(regs[2]->no << 9));

  insn->len = 4;
  return l;
}

/* Parse an FPU SWAP instruction.  */
static const char *
parse_fswap (const char *line, metag_insn *insn,
	     const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];

  if (insn->fpu_width != FPU_WIDTH_PAIR)
    return NULL;

  l = parse_fpu_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  if (regs[0]->no % 2)
    return NULL;

  if (regs[1]->no % 2)
    return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14));

  insn->len = 4;
  return l;
}

/* Parse an FPU CMP instruction.  */
static const char *
parse_fcmp (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line, *l2;
  const metag_reg *regs1[1];
  const metag_reg *regs2[1];

  l = parse_fpu_regs (l, regs1, 1);

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l2 = parse_fpu_regs (l, regs2, 1);

  if (l2 != NULL)
    {
      insn->bits = (regs2[0]->no << 9);
    }
  else
    {
      int constant = 0;
      l2 = parse_imm_constant (l, insn, &constant);
      if (!l2 || constant != 0)
	{
	  as_bad (_("comparison must be with register or #0"));
	  return NULL;
	}
      insn->bits = (1 << 8);
    }

  insn->bits |= (template->meta_opcode |
		 (regs1[0]->no << 14));

  if (insn->fpu_action_flags & FPU_ACTION_ABS)
    insn->bits |= (1 << 19);

  if (insn->fpu_action_flags & FPU_ACTION_QUIET)
    insn->bits |= (1 << 7);

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 6);
  else if (insn->fpu_width == FPU_WIDTH_DOUBLE)
    insn->bits |= (1 << 5);

  insn->len = 4;
  return l2;
}

/* Parse an FPU MIN or MAX instruction.  */
static const char *
parse_fminmax (const char *line, metag_insn *insn,
	       const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[3];

  l = parse_fpu_regs (l, regs, 3);

  if (l == NULL)
    return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14) |
		(regs[2]->no << 9));

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 6);
  else if (insn->fpu_width == FPU_WIDTH_DOUBLE)
    insn->bits |= (1 << 5);

  insn->len = 4;
  return l;
}

/* Parse an FPU data conversion instruction.  */
static const char *
parse_fconv (const char *line, metag_insn *insn,
	     const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    {
      if (strncasecmp (template->name, "FTOH", 4) &&
	  strncasecmp (template->name, "HTOF", 4) &&
	  strncasecmp (template->name, "FTOI", 4) &&
	  strncasecmp (template->name, "ITOF", 4))
	{
	  as_bad (_("instruction cannot operate on pair values"));
	  return NULL;
	}
    }

  if (insn->fpu_action_flags & FPU_ACTION_ZERO)
    {
      if (strncasecmp (template->name, "FTOI", 4) &&
	  strncasecmp (template->name, "DTOI", 4) &&
	  strncasecmp (template->name, "DTOL", 4))
	{
	  as_bad (_("zero flag is not valid for this instruction"));
	  return NULL;
	}
    }

  l = parse_fpu_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  if (!strncasecmp (template->name, "DTOL", 4) ||
      !strncasecmp (template->name, "LTOD", 4))
    {
      if (regs[0]->no % 2)
	{
	  as_bad (_("destination register should be even numbered"));
	  return NULL;
	}

      if (regs[1]->no % 2)
	{
	  as_bad (_("source register should be even numbered"));
	  return NULL;
	}
    }

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14));

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 6);

  if (insn->fpu_action_flags & FPU_ACTION_ZERO)
    insn->bits |= (1 << 12);

  insn->len = 4;
  return l;
}

/* Parse an FPU extended data conversion instruction.  */
static const char *
parse_fconvx (const char *line, metag_insn *insn,
	      const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];
  int fraction_bits = 0;

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    {
      if (strncasecmp (template->name, "FTOX", 4) &&
	  strncasecmp (template->name, "XTOF", 4))
	{
	  as_bad (_("instruction cannot operate on pair values"));
	  return NULL;
	}
    }

  l = parse_fpu_regs (l, regs, 2);

  l = skip_comma (l);

  if (l == NULL ||
      *l == END_OF_INSN)
    return NULL;

  l = parse_imm_constant (l, insn, &fraction_bits);

  if (l == NULL)
    return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14));

  if (strncasecmp (template->name, "DTOXL", 5) &&
      strncasecmp (template->name, "XLTOD", 5))
    {
      if (!within_unsigned_range (fraction_bits, IMM5_BITS))
	{
	  as_bad (_("fraction bits value out of range"));
	  return NULL;
	}
      insn->bits |= ((fraction_bits & IMM5_MASK) << 9);
    }
  else
    {
      if (!within_unsigned_range (fraction_bits, IMM6_BITS))
	{
	  as_bad (_("fraction bits value out of range"));
	  return NULL;
	}
      insn->bits |= ((fraction_bits & IMM6_MASK) << 8);
    }

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 6);

  insn->len = 4;
  return l;
}

/* Parse an FPU basic arithmetic instruction.  */
static const char *
parse_fbarith (const char *line, metag_insn *insn,
	       const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[3];

  l = parse_fpu_regs (l, regs, 3);

  if (l == NULL)
    return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14) |
		(regs[2]->no << 9));

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 6);
  else if (insn->fpu_width == FPU_WIDTH_DOUBLE)
    insn->bits |= (1 << 5);

  if (insn->fpu_action_flags & FPU_ACTION_INV)
    insn->bits |= (1 << 7);

  insn->len = 4;
  return l;
}

/* Parse a floating point accumulator name.  */
static const char *
parse_acf (const char *line, int *part)
{
  const char *l = line;
  size_t i;

  for (i = 0; i < sizeof(metag_acftab)/sizeof(metag_acftab[0]); i++)
    {
      const metag_acf *acf = &metag_acftab[i];
      size_t name_len = strlen (acf->name);

      if (strncasecmp (l, acf->name, name_len) == 0)
	{
	  l += name_len;
	  *part = acf->part;
	  return l;
	}
    }
  return NULL;
}

/* Parse an FPU extended arithmetic instruction.  */
static const char *
parse_fearith (const char *line, metag_insn *insn,
	       const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[3];
  bool is_muz = (MINOR_OPCODE (template->meta_opcode) == 0x6
		 && ((template->meta_opcode >> 4) & 0x1) != 0);
  bool is_o3o = (template->meta_opcode & 0x1) != 0;
  bool is_mac = 0;
  bool is_maw = 0;

  if (!strncasecmp (template->name, "MAW", 3))
    is_maw = 1;

  if (!strncasecmp (template->name, "MAC", 3))
    {
      int part;
      l = parse_acf (l, &part);

      if (l == NULL || part != 0)
	return NULL;

      l = skip_comma (l);

      l = parse_fpu_regs (l, &regs[1], 2);

      is_mac = 1;
    }
  else
    {
      if (is_o3o && is_maw)
	l = parse_fpu_regs (l, regs, 2);
      else
	l = parse_fpu_regs (l, regs, 3);
    }

  if (l == NULL)
    return NULL;

  if (is_o3o && is_maw)
    insn->bits = (template->meta_opcode |
		  (regs[1]->no << 9));
  else
    insn->bits = (template->meta_opcode |
		  (regs[1]->no << 14));

  if (!(is_o3o && is_maw))
    insn->bits |= (regs[2]->no << 9);

  if (is_o3o && is_maw)
    insn->bits |= (regs[0]->no << 14);
  else if (!is_mac)
    insn->bits |= (regs[0]->no << 19);

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 6);
  else if (insn->fpu_width == FPU_WIDTH_DOUBLE)
    insn->bits |= (1 << 5);

  if (!is_mac && !is_maw)
    if (insn->fpu_action_flags & FPU_ACTION_INV)
      insn->bits |= (1 << 7);

  if (is_muz)
    if (insn->fpu_action_flags & FPU_ACTION_QUIET)
      insn->bits |= (1 << 1);

  insn->len = 4;
  return l;
}

/* Parse an FPU RCP or RSQ instruction.  */
static const char *
parse_frec (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[2];

  l = parse_fpu_regs (l, regs, 2);

  if (l == NULL)
    return NULL;

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14));

  if (insn->fpu_width == FPU_WIDTH_PAIR)
    insn->bits |= (1 << 6);
  else if (insn->fpu_width == FPU_WIDTH_DOUBLE)
    insn->bits |= (1 << 5);

  if (insn->fpu_action_flags & FPU_ACTION_ZERO)
    insn->bits |= (1 << 10);
  else if (insn->fpu_action_flags & FPU_ACTION_QUIET)
    insn->bits |= (1 << 9);

  if (insn->fpu_action_flags & FPU_ACTION_INV)
    insn->bits |= (1 << 7);

  insn->len = 4;
  return l;
}

/* Parse an FPU vector arithmetic instruction.  */
static const char *
parse_fsimd (const char *line, metag_insn *insn,
	     const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[3];

  if (insn->fpu_width != FPU_WIDTH_PAIR)
    {
      as_bad (_("simd instructions operate on pair values (L prefix)"));
      return NULL;
    }

  l = parse_fpu_regs (l, regs, 3);

  if (l == NULL)
    return NULL;

  if (regs[0]->no % 2)
    {
      as_bad (_("destination register should be even numbered"));
      return NULL;
    }

  if ((regs[1]->no % 2) ||
      (regs[2]->no % 2))
    {
      as_bad (_("source registers should be even numbered"));
      return NULL;
    }

  insn->bits = (template->meta_opcode |
		(regs[0]->no << 19) |
		(regs[1]->no << 14) |
		(regs[2]->no << 9));

  if (insn->fpu_action_flags & FPU_ACTION_INV)
    insn->bits |= (1 << 7);

  insn->len = 4;
  return l;
}

/* Parse an FPU accumulator GET or SET instruction. */
static const char *
parse_fget_set_acf (const char *line, metag_insn *insn,
		    const insn_template *template)
{
  const char *l = line;
  int part;
  metag_addr addr;
  bool is_get = MAJOR_OPCODE (template->meta_opcode) == OPC_GET;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  if (is_get)
    {
      l = parse_acf (l, &part);

      l = skip_comma (l);

      if (l == NULL)
	return NULL;

      l = parse_mget_mset_addr (l, &addr);
    }
  else
    {
      l = parse_mget_mset_addr (l, &addr);

      l = skip_comma (l);

      if (l == NULL)
	return NULL;

      l = parse_acf (l, &part);
    }

  if (l == NULL)
    return NULL;

  insn->bits = (template->meta_opcode |
		(part << 19));

  if (!is_short_unit (addr.base_reg->unit))
    {
      as_bad (_("base unit must be one of %s"), SHORT_UNITS);
      return NULL;
    }

  insn->bits |= ((addr.base_reg->no << 14) |
		 ((addr.base_reg->unit & SHORT_UNIT_MASK) << 5));

  insn->len = 4;
  return l;
}

/* Copy the name of the next register in LINE to REG_BUF.  */
static size_t
strip_reg_name(const char *line, char *reg_buf)
{
  const char *l = line;
  size_t len = 0;

  while (is_register_char (*l))
    {
      reg_buf[len] = *l;
      l++;
      len++;
      if (!(len < MAX_REG_LEN))
	return 0;
    }

  if (len)
    reg_buf[len] = '\0';

  return len;
}

/* Parse a DSP register from LINE into REG using only the registers
   from DSP_REGTAB. Return the next character or NULL.  */
static const char *
__parse_dsp_reg (const char *line, const metag_reg **reg, htab_t dsp_regtab)
{
  const char *l = line;
  char name[MAX_REG_LEN];
  size_t len = 0;
  metag_reg entry;
  const metag_reg *_reg;

  /* We don't entirely strip the register name because we might
     actually want to match whole string in the register table,
     e.g. "D0AW.1++" not just "D0AW.1". The string length of the table
     entry limits our comparison to a reasonable bound anyway.  */
  while (is_register_char (*l) || *l == PLUS)
    {
      name[len] = *l;
      l++;
      len++;
      if (!(len < MAX_REG_LEN))
	return NULL;
    }

  if (!len)
    return NULL;

  name[len] = '\0';
  entry.name = name;

  _reg = (const metag_reg *) htab_find (dsp_regtab, &entry);
  if (!_reg)
    return NULL;

  *reg = _reg;

  return l;
}

/* Parse a DSP register and setup "reg" with a metag_reg whose "no"
   member is suitable for encoding into a DSP insn register field.  */
static const char *
parse_dsp_insn_reg (const char *line, const metag_reg **reg)
{
  return __parse_dsp_reg (line, reg, dsp_reg_htab);
}

/* Parse a DSP register and setup "reg" with a metag_reg whose "no"
   member is suitable for encoding into a DSP template definition insn
   register field.

   There is a separate table for whether we're doing a load or a store
   definition. "load" specifies which table to look at.  */
static const char *
parse_dsp_template_reg (const char *line, const metag_reg **reg,
			bool load)
{
  return __parse_dsp_reg (line, reg, dsp_tmpl_reg_htab[load]);
}

/* Parse a single DSP register from LINE.  */
static const char *
parse_dsp_reg (const char *line, const metag_reg **reg,
	       bool tmpl, bool load)
{
  if (tmpl)
    return parse_dsp_template_reg (line, reg, load);
  else
    return parse_dsp_insn_reg (line, reg);
}

/* Return TRUE if UNIT is an address unit.  */
static bool
is_addr_unit (enum metag_unit unit)
{
  switch (unit)
    {
    case UNIT_A0:
    case UNIT_A1:
      return true;
    default:
      return false;
    }
}

/* Return TRUE if UNIT1 and UNIT2 are equivalent units.  */
static bool
is_same_data_unit (enum metag_unit unit1, enum metag_unit unit2)
{
  if (unit1 == unit2)
    return true;

  switch (unit1)
    {
    case UNIT_D0:
      if (unit2 == UNIT_ACC_D0 || unit2 == UNIT_RAM_D0)
	return true;
      break;
    case UNIT_D1:
      if (unit2 == UNIT_ACC_D1 || unit2 == UNIT_RAM_D1)
	return true;
      break;
    case UNIT_ACC_D0:
      if (unit2 == UNIT_D0 || unit2 == UNIT_RAM_D0)
	return true;
      break;
    case UNIT_ACC_D1:
      if (unit2 == UNIT_D1 || unit2 == UNIT_RAM_D1)
	return true;
      break;
    case UNIT_RAM_D0:
      if (unit2 == UNIT_ACC_D0 || unit2 == UNIT_D0)
	return true;
      break;
    case UNIT_RAM_D1:
      if (unit2 == UNIT_ACC_D1 || unit2 == UNIT_D1)
	return true;
      break;
    default:
      return false;
    }

  return false;
}

/* Return TRUE if the register NUM is a quickrot control register.  */
static bool
is_quickrot_reg (unsigned int num)
{
  switch (num)
    {
    case 2:
    case 3:
      return true;
    }

  return false;
}

/* Return TRUE if REG is an accumulator register.  */
static bool
is_accumulator_reg (const metag_reg *reg)
{
  if (reg->unit == UNIT_ACC_D0 || reg->unit == UNIT_ACC_D1)
    return true;

  return false;
}

/* Return TRUE if REG is a DSP RAM register.  */
static bool
is_dspram_reg (const metag_reg *reg)
{
  if (reg->unit == UNIT_RAM_D0 || reg->unit == UNIT_RAM_D1)
      return true;

  return false;
}

static const char *
__parse_gp_reg (const char *line, const metag_reg **reg, bool load)
{
  const char *l = line;
  char reg_buf[MAX_REG_LEN];
  size_t len = 0;

  if (l == NULL)
    return NULL;

  /* Parse [DSPRAM.x].  */
  if (*l == ADDR_BEGIN_CHAR)
    {
      l++;

      if (l == NULL)
	return NULL;

      l = parse_dsp_reg (l, reg, true, load);
      if (l == NULL)
	return NULL;

      if (*l == ADDR_END_CHAR)
	l++;
      else
	{
	  as_bad (_("expected ']', not %c in %s"), *l, l);
	  return NULL;
	}

      return l;
    }
  else
    {

      len = strip_reg_name (l, reg_buf);
      if (!len)
	return NULL;

      l += len;
      *reg = parse_gp_reg (reg_buf);
      if (*reg == NULL)
	return NULL;
    }

  return l;
}

/* Parse a list of DSP/GP registers. TRY_GP indicates whether we
   should try to parse the register as a general-purpose register if
   we fail to parse it as a DSP one. TMPL indicates whether the
   registers are part of a template definition instruction. If this is
   a template definition instruction LOAD says whether it's a load
   template insn. FIRST_DST indicates whether the first register is
   a destination operand.  */
static const char *
parse_dsp_regs_list (const char *line, const metag_reg **regs, size_t count,
		     size_t *regs_read, bool try_gp, bool tmpl,
		     bool load, bool first_dst)
{
  const char *l = line;
  int seen_regs = 0;
  size_t i;
  const metag_reg *reg;

  for (i = 0; i < count; i++)
    {
      const char *next, *ll;

      next = l;

      if (i > 0)
	{
	  l = skip_comma (l);
	  if (l == NULL)
	    {
	      *regs_read = seen_regs;
	      return next;
	    }
	}

      ll = parse_dsp_reg (l, &reg, tmpl, load);

      if (!ll)
	{
	  if (try_gp)
	    {
	      l = __parse_gp_reg (l, &reg, !(first_dst && i == 0));
	      if (l == NULL)
		{
		  *regs_read = seen_regs;
		  return next;
		}
	      regs[i] = reg;
	      seen_regs++;
	    }
	  else
	    {
	      *regs_read = seen_regs;
	      return l;
	    }
	}
      else
	{
	  regs[i] = reg;
	  seen_regs++;
	  l = ll;
	}
    }

  *regs_read = seen_regs;
  return l;
}

/* Parse the following memory references:

     - [Ax.r]
     - [Ax.r++]
     - [Ax.r--]
     - [Ax.r+Ax.r++]
     - [Ax.r-Ax.r--]

     - [DSPRam]
     - [DSPRam++]
     - [DSPRam+DSPRam++]
     - [DSPRam-DSPRam--]  */
static const char *
parse_dsp_addr (const char *line, metag_addr *addr, unsigned int size,
		bool load)
{
  const char *l = line, *ll;
  const metag_reg *regs[1];
  size_t regs_read;

  /* Skip opening square bracket.  */
  l++;

  l = parse_dsp_regs_list (l, regs, 1, &regs_read, true, true, load, false);

  if (l == NULL)
    return NULL;

  if (!is_addr_unit (regs[0]->unit) &&
      !is_dspram_reg (regs[0]))
    {
      as_bad (_("invalid register for memory access"));
      return NULL;
    }

  addr->base_reg = regs[0];

  if (*l == ADDR_END_CHAR)
    {
      addr->exp.X_op = O_constant;
      addr->exp.X_add_symbol = NULL;
      addr->exp.X_op_symbol = NULL;

      /* Simple register with no offset (0 immediate).  */
      addr->exp.X_add_number = 0;

      addr->immediate = 1;
      l++;

      return l;
    }

  ll = parse_addr_post_incr_op (l, addr);

  if (ll && *ll == ADDR_END_CHAR)
    {
      if (addr->update == 1)
	{
	  /* We have a post increment/decrement.  */
	  addr->exp.X_op = O_constant;
	  addr->exp.X_add_number = size;
	  addr->exp.X_add_symbol = NULL;
	  addr->exp.X_op_symbol = NULL;
	  addr->post_increment = 1;
	}
      addr->immediate = 1;
      ll++;
      return ll;
    }

  addr->post_increment = 0;

  l = parse_addr_op (l, addr);

  if (l == NULL)
    return NULL;

  l = parse_dsp_regs_list (l, regs, 1, &regs_read, true, true, load, false);

  if (l == NULL)
    return NULL;

  if (regs[0]->unit != addr->base_reg->unit)
    {
      as_bad (_("offset and base must be from the same unit"));
      return NULL;
    }

  addr->offset_reg = regs[0];

  if (*l == ADDR_END_CHAR)
    {
      l++;
      return l;
    }

  l = parse_addr_post_incr_op (l, addr);

  if (l == NULL)
    return NULL;

  if (*l == ADDR_END_CHAR)
    {
      l++;
      return l;
    }

  return NULL;
}

/* Parse a DSP GET or SET instruction.  */
static const char *
parse_dget_set (const char *line, metag_insn *insn,
		const insn_template *template)
{
  const char *l = line;
  metag_addr addr;
  int unit = 0;
  int rd_reg = 0;
  bool is_get = (template->meta_opcode & 0x100) != 0;
  bool is_dual = (template->meta_opcode & 0x4) != 0;
  bool is_template = false;
  const metag_reg *regs[2];
  unsigned int size;
  size_t count, regs_read;

  memset(&addr, 0, sizeof(addr));
  addr.reloc_type = BFD_RELOC_UNUSED;

  size = is_dual ? 8 : 4;
  count = is_dual ? 2 : 1;

  if (is_get)
    {
      /* GETL can be used on one template table entry.  */
      if (*l == 'T')
	count = 1;

      l = parse_dsp_regs_list (l, regs, count, &regs_read, false,
			       false, false, false);
      l = skip_comma (l);

      if (l == NULL)
	{
	  as_bad (_("unexpected end of line"));
	  return NULL;
	}

      l = parse_addr (l, &addr, size);
    }
  else
    {
      l = parse_addr (l, &addr, size);

      l = skip_comma (l);

      if (l == NULL)
	return NULL;

      /* GETL can be used on one template table entry.  */
      if (*l == 'T')
	count = 1;

      l = parse_dsp_regs_list (l, regs, count, &regs_read, false, false,
			       false, false);
    }

  if (l == NULL)
    return NULL;

  /* The first register dictates the unit.  */
  if (regs[0]->unit == UNIT_DT)
      is_template = true;
  else
    {
      if (regs[0]->unit == UNIT_D0 || regs[0]->unit == UNIT_RAM_D0 ||
	  regs[0]->unit == UNIT_ACC_D0)
	unit = 0;
      else
	unit = 1;
    }

  rd_reg = regs[0]->no;

  /* The 'H' modifier allows a DSP GET/SET instruction to target the
     upper 8-bits of an accumulator. It is _only_ valid for the
     accumulators.  */
  if (insn->dsp_daoppame_flags & DSP_DAOPPAME_HIGH)
    {
      if (is_template || !(rd_reg >= 16 && rd_reg < 20))
	{
	  as_bad (_("'H' modifier only valid for accumulator registers"));
	  return NULL;
	}

      /* Top 8-bits of the accumulator.  */
      rd_reg |= 8;
    }

  if (is_template)
    {
      insn->bits = (template->meta_opcode | (1 << 1));
    }
  else
    {
      insn->bits = (template->meta_opcode | unit);
    }

  insn->bits |= (rd_reg << 19);

  if (addr.immediate)
    {
      int offset = addr.exp.X_add_number;

      if (addr.negate)
	offset = -offset;

      offset = offset / (int)size;

      if (!within_signed_range (offset, DGET_SET_IMM_BITS))
	{
	  as_bad (_("offset value out of range"));
	  return NULL;
	}

      offset = offset & DGET_SET_IMM_MASK;

      insn->bits |= (1 << 13);
      insn->bits |= (offset << 9);
    }
  else
    {
      int au = (addr.base_reg->unit == UNIT_A1);

      insn->bits |= (au << 18);
      insn->bits |= ((addr.base_reg->no & REG_MASK) << 14);
      insn->bits |= ((addr.offset_reg->no & REG_MASK) << 9);
    }

  if (is_dual)
      insn->bits |= (1 << 2);

  if (!is_addr_unit (addr.base_reg->unit))
    {
      as_bad (_("base unit must be either A0 or A1"));
      return NULL;
    }

  unit = (addr.base_reg->unit == UNIT_A0) ? 0 : 1;
  insn->bits |= ((addr.base_reg->no << 14) | (unit << 18));

  insn->len = 4;

  return l;
}

/* Parse a DSP template instruction.  */
static const char *
parse_dtemplate (const char *line, metag_insn *insn,
		 const insn_template *template)
{
  const char *l = line;
  const metag_reg *regs[TEMPLATE_NUM_REGS];
  bool daop_only = false;
  int regs_val[4];
  int regs_which[4] = { -1, -1, -1, -1};	/* Register or immediate?  */
  int i;

  for (i = 0; i < TEMPLATE_NUM_REGS; i++)
    {
      if (l == NULL)
	{
	  as_bad (_("unexpected end of line"));
	  return NULL;
	}

      /* We may only have 3 register operands.  */
      if (*l == END_OF_INSN && i == 3)
	{
	  daop_only = true;
	  break;
	}

      if (i != 0)
	{
	  l = skip_comma (l);
	  if (l == NULL)
	    return NULL;
	}

      if (*l == IMM_CHAR)
	{
	  l = parse_imm_constant (l, insn, &regs_val[i]);
	  if (l == NULL)
	    {
	      as_bad (_("invalid immediate"));
	      return NULL;
	    }
	  regs_which[i] = 0;
	}
      else
	{
	  /* We can't tell from the template instantiation whether
	     this is a load or store. So we have to try looking up the
	     register name in both the load and store tables.  */
	  const char *l2 = l;
	  l = __parse_gp_reg (l, &regs[i], true);
	  if (l == NULL)
	    {
	      /* Try the store table too.  */
	      l = __parse_gp_reg (l2, &regs[i], false);
	      if (l == NULL)
		{
		  /* Then try a DSP register.  */
		  l = parse_dsp_insn_reg (l2, &regs[i]);
		  if (l == NULL || regs[i]->unit == UNIT_DT)
		    {
		      as_bad (_("invalid register"));
		      return NULL;
		    }
		}
	    }
	  regs_which[i] = 1;
	}
    }

  insn->bits = template->meta_opcode;

  if (regs_which[0] == 0)
    insn->bits |= (regs_val[0] << 19);
  else if (regs_which[0] == 1)
    insn->bits |= (regs[0]->no << 19);

  if (regs_which[1] == 0)
    insn->bits |= (regs_val[1] << 14);
  else if (regs_which[1] == 1)
    insn->bits |= (regs[1]->no << 14);

  if (regs_which[2] == 0)
    insn->bits |= (regs_val[2] << 9);
  else if (regs_which[2] == 1)
    insn->bits |= (regs[2]->no << 9);

  if (regs_which[3] == 0)
    insn->bits |= (regs_val[3] << 4);
  else if (regs_which[3] == 1)
    insn->bits |= (regs[3]->no << 4);

  /* DaOp only.  */
  if (daop_only)
    insn->bits |= (0x3 << 24); /* Set the minor opcode.  */
  else if (insn->dsp_daoppame_flags & DSP_DAOPPAME_HIGH) /* Half Load/Store.  */
    insn->bits |= (0x5 << 24); /* Set the minor opcode.  */

  insn->len = 4;

  return l;
}

/* Parse a DSP Template definition memory reference, e.g
   [A0.7+A0.5++]. DSPRAM is set to true by this function if this
   template definition is a DSP RAM template definition.  */
static const char *
template_mem_ref(const char *line, metag_addr *addr,
		 bool *dspram, int size, bool load)
{
  const char *l = line;

  l = parse_dsp_addr (l, addr, size, load);

  if (l != NULL)
    {
      if (is_addr_unit(addr->base_reg->unit))
	*dspram = false;
      else
	*dspram = true;
    }

  return l;
}

/* Sets LOAD to TRUE if this is a Template load definition (otherwise
   it's a store). Fills out ADDR, TEMPLATE_REG and ADDR_UNIT.  */
static const char *
parse_template_regs (const char *line, bool *load,
		     unsigned int *addr_unit,
		     const metag_reg **template_reg, metag_addr *addr,
		     bool *dspram, int size)
{
  const char *l = line;

  if (l == NULL)
    return NULL;

  /* DSP Template load definition (Tx, [Ax]) */
  if (*l == 'T')
    {
      *load = true;
      l = parse_dsp_reg (l, &template_reg[0], false, false);
      if (l == NULL)
	return NULL;

      l = skip_comma (l);

      l = template_mem_ref (l, addr, dspram, size, *load);

      if (addr->base_reg->unit == UNIT_A1)
	*addr_unit = 1;

    }
  else if (*l == ADDR_BEGIN_CHAR) /* DSP Template store ([Ax], Tx) */
    {
      *load = false;
      l = template_mem_ref (l, addr, dspram, size, *load);
      l = skip_comma(l);

      if (l == NULL)
	return NULL;

      l = parse_dsp_reg (l, &template_reg[0], false, false);
      if (l == NULL)
	return NULL;

      if (addr->base_reg->unit == UNIT_A1)
	*addr_unit = 1;
    }
  else
    {
      as_bad (_("invalid register operand"));
      return NULL;
    }

  return l;
}

#define INVALID_SHIFT (-1)

static metag_reg _reg;

/* Parse a template instruction definition.  */
static const char *
interpret_template_regs(const char *line, metag_insn *insn,
			const metag_reg **regs,
			int *regs_shift, bool *load, bool *dspram,
			int size, int *ls_shift, int *au_shift,
			unsigned int *au, int *imm, int *imm_shift,
			unsigned int *imm_mask)
{
  const char *l = line;
  metag_addr addr;
  const metag_reg *template_reg[1];

  memset (&addr, 0, sizeof(addr));

  regs_shift[0] = 19;
  regs_shift[1] = INVALID_SHIFT;

  insn->bits |= (1 << 1);

  l = skip_whitespace (l);

  l = parse_template_regs (l, load, au, template_reg,
			   &addr, dspram, size);
  if (l == NULL)
    {
      as_bad (_("could not parse template definition"));
      return NULL;
    }

  regs[2] = template_reg[0];
  regs_shift[2] = 9;

  /* DSPRAM definition.  */
  if (*dspram)
    {

      _reg = *addr.base_reg;

      if (addr.immediate)
	{
	  /* Set the post-increment bit in the register field.  */
	  if (addr.update)
	    _reg.no |= 0x1;
	}
      else
	{
	  /* The bottom bit of the increment register tells us
	     whether it's increment register 0 or 1.  */
	  if (addr.offset_reg->no & 0x1)
	    _reg.no |= 0x3;
	  else
	    _reg.no |= 0x2;
	}

      regs[0] = &_reg;

      insn->bits |= (0x3 << 17); /* This signifies a DSPRAM definition.  */
    }
  else /* DaOpPaMe definition.  */
    {
      regs[0] = addr.base_reg;
      if (addr.immediate)
	{
	  /* Set the I bit.  */
	  insn->bits |= (1 << 18);

	  if (addr.update == 1)
	    {
	      if (addr.negate == 1)
		*imm = 0x3;
	      else
		*imm = 0x1;
	    }

	  *imm_shift = 14;
	  *imm_mask = 0x3;
	}
      else
	{
	  /* Setup the offset register.  */
	  regs[1] = addr.offset_reg;
	  regs_shift[1] = 14;
	}
      *au_shift = 23;
    }

  *ls_shift = 13;

  return l;
}

/* Does this combination of units need the O2R bit and can it be encoded?  */
static bool
units_need_o2r (enum metag_unit unit1, enum metag_unit unit2)
{
  if (unit1 == unit2)
    return false;

  if (unit1 == UNIT_D0 || unit1 == UNIT_ACC_D0 || unit1 == UNIT_RAM_D0)
    {
      if (unit2 == UNIT_ACC_D0 || unit2 == UNIT_RAM_D0 || unit2 == UNIT_D0)
	return false;

      switch (unit2)
	{
	case UNIT_A1:
	case UNIT_D1:
	case UNIT_RD:
	case UNIT_A0:
	  return true;
	default:
	  return false;
	}
    }

  if (unit1 == UNIT_D1 || unit1 == UNIT_ACC_D1 || unit1 == UNIT_RAM_D1)
    {
      if (unit2 == UNIT_ACC_D1 || unit2 == UNIT_RAM_D1 || unit2 == UNIT_D1)
	return false;

      switch (unit2)
	{
	case UNIT_A1:
	case UNIT_D0:
	case UNIT_RD:
	case UNIT_A0:
	  return true;
	default:
	  return false;
	}
    }

  return false;
}

/* Return TRUE if this is a DSP data unit.  */
static bool
is_dsp_data_unit (const metag_reg *reg)
{
  switch (reg->unit)
    {
    case UNIT_D0:
    case UNIT_D1:
    case UNIT_ACC_D0:
    case UNIT_ACC_D1:
    case UNIT_RAM_D0:
    case UNIT_RAM_D1:
      return true;
    default:
      return false;
    }
}

static metag_reg o2r_reg;

/* Parse a DaOpPaMe load template definition.  */
static const char *
parse_dalu (const char *line, metag_insn *insn,
	    const insn_template *template)
{
  const char *l = line;
  const char *ll;
  const metag_reg *regs[4];
  metag_addr addr;
  size_t regs_read;
  bool is_mov = MAJOR_OPCODE (template->meta_opcode) == OPC_ADD;
  bool is_cmp = (MAJOR_OPCODE (template->meta_opcode) == OPC_CMP
		 && (template->meta_opcode & 0xee) == 0);
  bool is_dual = insn->dsp_width == DSP_WIDTH_DUAL;
  bool is_quickrot64 = (insn->dsp_action_flags & DSP_ACTION_QR64) != 0;
  int l1_shift = INVALID_SHIFT;
  bool load = false;
  int ls_shift = INVALID_SHIFT;
  bool ar = false;
  int ar_shift = INVALID_SHIFT;
  int regs_shift[3] = { INVALID_SHIFT, INVALID_SHIFT, INVALID_SHIFT };
  int imm = 0;
  int imm_shift = INVALID_SHIFT;
  unsigned int imm_mask = 0;
  unsigned int au = 0;
  int au_shift = INVALID_SHIFT;
  unsigned int du = 0;
  int du_shift = INVALID_SHIFT;
  unsigned int sc = ((insn->dsp_action_flags & DSP_ACTION_OV) != 0);
  int sc_shift = INVALID_SHIFT;
  unsigned int om = ((insn->dsp_action_flags & DSP_ACTION_MOD) != 0);
  int om_shift = INVALID_SHIFT;
  unsigned int o2r = 0;
  int o2r_shift = INVALID_SHIFT;
  unsigned int qr = 0;
  int qr_shift = INVALID_SHIFT;
  int qd_shift = INVALID_SHIFT;
  unsigned int qn = 0;
  int qn_shift = INVALID_SHIFT;
  unsigned int a1 = ((insn->dsp_action_flags & (DSP_ACTION_ACC_SUB|DSP_ACTION_ACC_ZERO)) != 0);
  int a1_shift = INVALID_SHIFT;
  unsigned int a2 = ((insn->dsp_action_flags & (DSP_ACTION_ACC_SUB|DSP_ACTION_ACC_ADD)) != 0);
  int a2_shift = INVALID_SHIFT;
  unsigned su = ((insn->dsp_action_flags & DSP_ACTION_UMUL) != 0);
  int su_shift = INVALID_SHIFT;
  unsigned int ac;
  int ac_shift = INVALID_SHIFT;
  unsigned int mx = (((insn->dsp_daoppame_flags & DSP_DAOPPAME_8) != 0) ||
		     (insn->dsp_daoppame_flags & DSP_DAOPPAME_16) != 0);
  int mx_shift = INVALID_SHIFT;
  int size = is_dual ? 8 : 4;
  bool dspram;
  bool conditional = (MINOR_OPCODE (template->meta_opcode) & 0x4) != 0;

  /* XFIXME: check the flags are valid with the instruction.  */
  if (is_quickrot64 && !(template->arg_type & DSP_ARGS_QR))
    {
      as_bad (_("QUICKRoT 64-bit extension not applicable to this instruction"));
      return NULL;
    }

  insn->bits = template->meta_opcode;

  memset (regs, 0, sizeof (regs));
  memset (&addr, 0, sizeof (addr));

  /* There are the following forms of DSP ALU instructions,

   * Group 1:
      19. D[T]  Op    De.r,Dx.r,De.r
      1.  D[T]  Op    De.r,Dx.r,De.r|ACe.r	[Accumulator in src 2]
      3.  D[T]  Op    De.r,Dx.r,De.r[,Ae.r]	[QUICKRoT]
      2.  D[T]  Op    ACe.e,ACx.r,ACo.e		[cross-unit accumulator op]
      5.  D[T]  Op    De.r|ACe.r,Dx.r,De.r
      20. D[T]  Op    De.r,Dx.r|ACx.r,De.r
      8.  D     Opcc  De.r,Dx.r,Rx.r
      6.  D     Op    De.r,Dx.r,Rx.r|RD
      17. D     Op    De.r|ACe.r,Dx.r,Rx.r|RD
      7.  D     Op    De.e,Dx.r,#I16

   * Group 2:
      4.  D[T]  Op    Dx.r,De.r
      10. D     Op    Dx.r,Rx.r|RD
      13. D     Op    Dx.r,Rx.r
      11. D     Op    Dx.r,#I16
      12. D[T]  Op    De.r,Dx.r
      14. D     Op    DSPe.r,Dx.r
      15. D     Op    DSPx.r,#I16
      16. D     Op    De.r,DSPx.r
      18. D     Op    De.r,Dx.r|ACx.r

   * Group 3:
      22. D     Op    De.r,Dx.r|ACx.r,De.r|#I5
      23. D     Op    Ux.r,Dx.r|ACx.r,De.r|#I5
      21. D     Op    De.r,Dx.r|ACx.r,#I5  */

  /* Group 1.  */
  if (template->arg_type & DSP_ARGS_1)
    {
      du_shift = 24;

      /* Could this be a cross-unit accumulator op,
	 e.g. ACe.e,ACx.r,ACo.e */
      if (template->arg_type & DSP_ARGS_XACC)
	{
	  ll = parse_dsp_regs_list (l, regs, 3, &regs_read, false, false,
				    false, false);
	  if (ll != NULL && regs_read == 3
	      && is_accumulator_reg (regs[0]))
	    {
	      if (regs[0]->unit != regs[1]->unit ||
		  regs[2]->unit == regs[1]->unit)
		{
		  as_bad (_("invalid operands for cross-unit op"));
		  return NULL;
		}

	      du = (regs[1]->unit == UNIT_ACC_D1);
	      regs_shift[1] = 19;
	      l = ll;

	      /* All cross-unit accumulator ops have bits 8 and 6 set.  */
	      insn->bits |= (5 << 6);

	      goto check_for_template;
	    }

	  /* If we reach here, this instruction is not a
	     cross-unit accumulator op.  */
	}

      if (template->arg_type & DSP_ARGS_SPLIT8)
	om_shift = 7;

      sc_shift = 5;
      l1_shift = 4;
      o2r_shift = 0;

      /* De.r|ACe.r,Dx.r,De.r */
      if (template->arg_type & DSP_ARGS_DACC)
	{
	  /* XFIXME: these need moving?  */
	  a2_shift = 7;
	  su_shift = 6;
	  a1_shift = 2;
	  om_shift = 3;

	  ll = parse_dsp_reg (l, &regs[0], false, false);
	  if (ll != NULL)
	    {
	      /* Using ACe.r as the dst requires one of the P,N or Z
		 flags to be used.  */
	      if (!(insn->dsp_action_flags &
		    (DSP_ACTION_ACC_SUB|DSP_ACTION_ACC_ADD|DSP_ACTION_ACC_ZERO)))
		{
		  as_bad (_("missing flags: one of 'P', 'N' or 'Z' required"));
		  return NULL;
		}

	      l = ll;
	      l = skip_comma (l);
	      l = parse_dsp_regs_list (l, &regs[1], 2, &regs_read,
				       true, false, false, false);
	      if (l == NULL || regs_read != 2)
		{
		  as_bad (_("invalid register"));
		  return NULL;
		}

	      if (regs[1]->unit == UNIT_D1 || regs[1]->unit == UNIT_RAM_D1)
		du = 1;

	      regs_shift[0] = 19;
	      regs_shift[1] = 14;
	      regs_shift[2] = 9;
	      goto check_for_template;
	    }

	  /* If we reach here, this instruction does not use the
	     accumulator as the destination register.  */
	  if ((insn->dsp_action_flags &
	       (DSP_ACTION_ACC_SUB|DSP_ACTION_ACC_ADD|DSP_ACTION_ACC_ZERO)))
	    {
	      as_bad (_("'P', 'N' or 'Z' flags may only be specified when accumulating"));
	      return NULL;
	    }
	}

      regs_shift[0] = 19;


      l = parse_dsp_regs_list (l, regs, 2, &regs_read, true, false, false, true);
      if (l == NULL || regs_read != 2)
	return NULL;

      l = skip_comma (l);
      if (l == NULL)
	return NULL;

      if (regs[1]->unit == UNIT_D1 || regs[1]->unit == UNIT_RAM_D1)
	du = 1;

      if (is_accumulator_reg(regs[0]) && !(template->arg_type & DSP_ARGS_DACC))
       {
	 as_bad (_("accumulator not a valid destination"));
	 return NULL;
       }

      /* Check for immediate, e.g. De.r,Dx.r,#I16 */
      if (*l == IMM_CHAR)
	{
	  l = parse_imm16 (l, insn, &imm);
	  if (l == NULL)
	    {
	      as_bad (_("invalid immediate value"));
	      return NULL;
	    }

	  if (!within_signed_range (imm, IMM16_BITS))
	    {
	      as_bad (_("immediate value out of range"));
	      return NULL;
	    }

	  if (regs[0]->unit != regs[1]->unit || regs[0]->no != regs[1]->no)
	    {
	      as_bad (_("immediate value not allowed when source & dest differ"));
	      return NULL;
	    }

	  imm_mask = 0xffff;
	  imm_shift = 3;

	  /* Set the I-bit */
	  insn->bits |= (1 << 25);

	  insn->bits |= (0x3 << 0);

	  l1_shift = 2;

	  /* Remove any bits that have been set in the immediate
	     field.  */
	  insn->bits &= ~(imm_mask << imm_shift);
	}
      else
	{

	  regs_shift[1] = 14;
	  regs_shift[2] = 9;

	  /* Is Rs2 an accumulator reg, e.g. De.r,Dx.r,De.r|ACe.r */
	  ll = parse_dsp_reg (l, &regs[2], false, false);
	  if (ll != NULL)
	    {
	      l = ll;

	      if (!(template->arg_type & DSP_ARGS_ACC2))
		{
		  as_bad (_("invalid register operand: %s"), regs[2]->name);
		  return NULL;
		}

	      om_shift = 3;
	      ar_shift = 7;
	      ar = true;
	    }
	  else
	    {
	      /* De.r,Dx.r,De.r */
	      l = __parse_gp_reg (l, &regs[2], true);
	      if (l == NULL)
		return NULL;
	    }

	  if (template->arg_type & DSP_ARGS_ACC2)
	    om_shift = 3;

	  /* Is this a QUICKRoT instruction? De.r,Dx.r,De.r[,Ae.r] */
	  if (template->arg_type & DSP_ARGS_QR)
	    {
	      if (conditional)
		qn_shift = 5;
	      else
		{
		  qn_shift = 7;
		  qr_shift = 6;
		  qd_shift = 5;
		}

	      l = skip_comma (l);
	      if (l == NULL)
		{
		  as_bad (_("QUICKRoT extension requires 4 registers"));
		  return NULL;
		}

	      l = __parse_gp_reg (l, &regs[3], true);
	      if (l == NULL)
		{
		  as_bad (_("invalid fourth register"));
		  return NULL;
		}

	      if (!is_addr_unit (regs[3]->unit) ||
		  !is_quickrot_reg (regs[3]->no))
		{
		  as_bad (_("A0.2,A0.3,A1.2,A1.3 required for QUICKRoT register"));
		  return NULL;
		}

	      qn = (regs[3]->no == 3);
	    }
	}

    check_for_template:
      /* This is the common exit path. Check for o2r.  */
      if (regs[2] != NULL)
	{
	  o2r = units_need_o2r (regs[1]->unit, regs[2]->unit);
	  if (o2r)
	    {
	      o2r_reg.no = lookup_o2r (0, du, regs[2]);
	      o2r_reg.unit = regs[2]->unit;
	      regs[2] = &o2r_reg;
	    }
	}

      /* Check any DSP RAM pointers are valid for this unit.  */
      if ((du && (regs[0]->unit == UNIT_RAM_D0)) ||
	  (!du && (regs[0]->unit == UNIT_RAM_D1)) ||
	  (du && (regs[1]->unit == UNIT_RAM_D0)) ||
	  (!du && (regs[1]->unit == UNIT_RAM_D1)) ||
	  (du && regs[2] && (regs[2]->unit == UNIT_RAM_D0)) ||
	  (!du && regs[2] && (regs[2]->unit == UNIT_RAM_D1))) {
	as_bad (_("DSP RAM pointer in incorrect unit"));
	return NULL;
      }

      /* Is this a template definition?  */
      if (IS_TEMPLATE_DEF (insn))
	{
	  l = interpret_template_regs(l, insn, regs, regs_shift, &load,
				      &dspram, size, &ls_shift, &au_shift,
				      &au, &imm, &imm_shift, &imm_mask);

	  if (l == NULL)
	    return NULL;

	  if (!dspram)
	    mx_shift = 0;
	}

      goto matched;
    }

  /* Group 2.  */
  if (template->arg_type & DSP_ARGS_2)
    {
      bool is_xsd = (MAJOR_OPCODE (template->meta_opcode) == OPC_MISC
		     && MINOR_OPCODE (template->meta_opcode) == 0xa);
      bool is_fpu_mov = template->insn_type == INSN_DSP_FPU;
      bool to_fpu = ((template->meta_opcode >> 7) & 0x1) != 0;

      if (is_xsd)
	du_shift = 0;
      else
	du_shift = 24;

      l1_shift = 4;

      /* CMPs and TSTs don't store to their destination operand.  */
      ll = __parse_gp_reg (l, regs, is_cmp);
      if (ll == NULL)
	{
	  /* DSPe.r,Dx.r or DSPx.r,#I16 */
	  if (template->arg_type & DSP_ARGS_DSP_SRC1)
	    {
	      l = parse_dsp_reg (l, regs, false, false);
	      if (l == NULL)
		{
		  as_bad (_("invalid register operand #1"));
		  return NULL;
		}

	      /* Only MOV instructions have a DSP register as a
		 destination. Set the MOV DSPe.r opcode. The simple
		 OR'ing is OK because the usual MOV opcode is 0x00.  */
	      insn->bits = 0x91u << 24;
	      du_shift = 0;
	      l1_shift = 2;
	      regs_shift[0] = 19;
	    }
	  else
	    {
	      as_bad (_("invalid register operand #2"));
	      return NULL;
	    }
	}
      else
	{
	  l = ll;

	  /* Everything but CMP and TST.  */
	  if (MAJOR_OPCODE (template->meta_opcode) == OPC_ADD ||
	      MAJOR_OPCODE (template->meta_opcode) == OPC_SUB ||
	      MAJOR_OPCODE (insn->bits) == OPC_9 ||
	      MAJOR_OPCODE (template->meta_opcode) == OPC_MISC ||
	      ((template->meta_opcode & 0x0000002c) != 0))
	    regs_shift[0] = 19;
	  else
	    regs_shift[0] = 14;
	}

      if (!is_dsp_data_unit (regs[0]) && !(regs[0]->unit == UNIT_FX &&
					   is_fpu_mov && to_fpu))
	return NULL;

      du = (regs[0]->unit == UNIT_D1 || regs[0]->unit == UNIT_RAM_D1 ||
	    regs[0]->unit == UNIT_ACC_D1);

      l = skip_comma (l);

      if (*l == IMM_CHAR)
	{
	  if (template->arg_type & DSP_ARGS_IMM &&
	      !(is_mov && (MAJOR_OPCODE (insn->bits) != OPC_9)))
	    {
	      l = parse_imm16 (l, insn, &imm);
	      if (l == NULL)
		{
		  as_bad (_("invalid immediate value"));
		  return NULL;
		}

	      if (!within_signed_range (imm, IMM16_BITS))
		return NULL;

	      l1_shift = 2;
	      regs_shift[0] = 19;

	      imm_mask = 0xffff;
	      imm_shift = 3;

	      /* Set the I-bit unless it's a MOV because they're
		 different.  */
	      if (!(is_mov && MAJOR_OPCODE (insn->bits) == OPC_9))
		insn->bits |= (1 << 25);

	      /* All instructions that takes immediates also have bit 1 set.  */
	      insn->bits |= (1 << 1);

	      if (MAJOR_OPCODE (insn->bits) != OPC_9)
		insn->bits |= (1 << 0);

	      insn->bits &= ~(1 << 8);
	    }
	  else
	    {
	      as_bad (_("this instruction does not accept an immediate"));
	      return NULL;
	    }
	}
      else
	{
	  if (MAJOR_OPCODE (insn->bits) != OPC_9)
	    {
	      insn->bits |= (1 << 8);
	      l1_shift = 4;
	    }

	  ll = __parse_gp_reg (l, &regs[1], true);
	  if (ll == NULL)
	    {
	      if (template->arg_type & DSP_ARGS_DSP_SRC2)
		{
		  l = parse_dsp_reg (l, &regs[1], false, false);
		  if (l == NULL)
		    {
		      as_bad (_("invalid register operand #3"));
		      return NULL;
		    }

		  /* MOV and NEG.  */
		  if ((is_mov && (MAJOR_OPCODE (insn->bits) != OPC_9)) ||
		      MAJOR_OPCODE (template->meta_opcode) == OPC_SUB)
		    {
		      if (is_accumulator_reg (regs[1]))
			{
			  if (is_fpu_mov)
			    {
			      as_bad (_("this instruction does not accept an accumulator"));
			      return NULL;
			    }
			  ar_shift = 7;
			  ar = 1;
			  regs_shift[1] = 9;
			}
		      else
			{
			  du_shift = 0;
			  l1_shift = 2;
			  regs_shift[1] = 14;
			  insn->bits = 0x92u << 24; /* Set opcode.  */
			}
		    }
		}
	      else
		{
		  as_bad (_("invalid register operand #4"));
		  return NULL;
		}
	    }
	  else
	    {
	      /* Set the o2r bit if required.  */
	      if (!is_fpu_mov && units_need_o2r (regs[0]->unit, regs[1]->unit))
		{
		  o2r_reg = *regs[1];
		  o2r_reg.no = lookup_o2r (0, du, regs[1]);
		  regs[1] = &o2r_reg;
		  o2r_shift = 0;
		  o2r = 1;
		}
	      else if (!is_dsp_data_unit (regs[1]) &&
		       !(is_fpu_mov && !to_fpu && regs[1]->unit == UNIT_FX))
		return NULL;

	      if (is_fpu_mov && to_fpu)
		du = (regs[1]->unit == UNIT_D1 ||
		      regs[1]->unit == UNIT_RAM_D1 ||
		      regs[1]->unit == UNIT_ACC_D1);

	      l = ll;

	      if (MAJOR_OPCODE (insn->bits) == OPC_ADD ||
		  MAJOR_OPCODE (template->meta_opcode) == OPC_SUB ||
		  (((template->meta_opcode & 0x0000002c) == 0) &&
		   MAJOR_OPCODE (template->meta_opcode) != OPC_MISC))
		regs_shift[1] = 9;
	      else
		regs_shift[1] = 14;
	    }
	}

      /* If it's an 0x0 MOV or NEG set some lower bits.  */
      if ((MAJOR_OPCODE (insn->bits) == OPC_ADD ||
	   MAJOR_OPCODE (template->meta_opcode) == OPC_SUB) && !is_fpu_mov)
	{
	  om_shift = 3;
	  sc_shift = 5;
	  insn->bits |= (1 << 2);
	}

      /* Check for template definitions.  */
      if (IS_TEMPLATE_DEF (insn))
	{
	  l = interpret_template_regs(l, insn, regs, regs_shift, &load,
				      &dspram, size, &ls_shift, &au_shift,
				      &au, &imm, &imm_shift, &imm_mask);
	  mx_shift = 0;

	  if (l == NULL)
	    return NULL;
	}
      goto matched;
    }

  /* Group 3.  */
  du_shift = 24;
  l1_shift = 4;

  l = __parse_gp_reg (l, regs, false);
  if (l == NULL)
    {
      as_bad (_("invalid register operand"));
      return NULL;
    }

  l = skip_comma (l);

  if (*l == 'A')
    {
      l = parse_dsp_reg (l, &regs[1], false, false);
      if (l == NULL)
	{
	  as_bad (_("invalid accumulator register"));
	  return NULL;
	}
      ac = 1;
      ac_shift = 0;
    }
  else
    {
      l = __parse_gp_reg (l, &regs[1], true);
      if (l == NULL)
	{
	  as_bad (_("invalid register operand"));
	  return NULL;
	}
    }

  regs_shift[0] = 19;
  regs_shift[1] = 14;

  du = (regs[1]->unit == UNIT_D1 || regs[1]->unit == UNIT_ACC_D1
	|| regs[1]->unit == UNIT_RAM_D1);

  l = skip_comma (l);

  if (*l == IMM_CHAR)
    {
      l = parse_imm_constant (l, insn, &imm);
      if (l == NULL)
	{
	  as_bad (_("invalid immediate value"));
	  return NULL;
	}

      if (!within_unsigned_range (imm, IMM5_BITS))
	return NULL;

      imm_mask = 0x1f;
      imm_shift = 9;

      /* Set the I-bit */
      insn->bits |= (1 << 25);
    }
  else
    {
      regs_shift[2] = 9;
      l = __parse_gp_reg (l, &regs[2], true);
      if (l == NULL)
	return NULL;
    }

  /* Check for post-processing R,G,B flags. Conditional instructions
     do not have these bits.  */
  if (insn->dsp_action_flags & DSP_ACTION_CLAMP9)
    {
      if ((template->meta_opcode >> 26) & 0x1)
	{
	  as_bad (_("conditional instruction cannot use G flag"));
	  return NULL;
	}

      insn->bits |= (1 << 3);
    }

  if (insn->dsp_action_flags & DSP_ACTION_CLAMP8)
    {
      if ((template->meta_opcode >> 26) & 0x1)
	{
	  as_bad (_("conditional instruction cannot use B flag"));
	  return NULL;
	}

      insn->bits |= (0x3 << 2);
    }

  if (insn->dsp_action_flags & DSP_ACTION_ROUND)
    {
      if ((template->meta_opcode >> 26) & 0x1)
	{
	  as_bad (_("conditional instruction cannot use R flag"));
	  return NULL;
	}
      insn->bits |= (1 << 2);
    }

  /* Conditional Data Unit Shift instructions cannot be dual unit.  */
  if ((template->meta_opcode >> 26) & 0x1)
    ls_shift = INVALID_SHIFT;

  /* The Condition Is Always (CA) bit must be set if we're targeting a
     Ux.r register as the destination. This means that we can't have
     any other condition bits set.  */
  if (!is_same_data_unit (regs[1]->unit, regs[0]->unit))
    {
      /* Set both the Conditional bit and the Condition is Always bit.  */
      insn->bits |= (1 << 26);
      insn->bits |= (1 << 5);

      /* Fill out the Ud field.  */
      insn->bits |= (regs[0]->unit << 1);
    }

  if (IS_TEMPLATE_DEF (insn))
    {
      l = interpret_template_regs(l, insn, regs, regs_shift, &load,
				  &dspram, size, &ls_shift, &au_shift,
				  &au, &imm, &imm_shift, &imm_mask);

      if (l == NULL)
	return NULL;

      if (!dspram)
	mx_shift = 5;
    }

  /* Fall through.  */
 matched:

  /* Set the registers and immediate values.  */
  if (regs_shift[0] != INVALID_SHIFT)
    insn->bits |= (regs[0]->no << regs_shift[0]);

  if (regs_shift[1] != INVALID_SHIFT)
    insn->bits |= (regs[1]->no << regs_shift[1]);

  if (regs_shift[2] != INVALID_SHIFT)
    insn->bits |= (regs[2]->no << regs_shift[2]);

  /* Does this insn have an 'IMM' bit? The immediate value should
     already have been masked.  */
  if (imm_shift != INVALID_SHIFT)
    insn->bits |= ((imm & imm_mask) << imm_shift);

  /* Does this insn have an 'AU' bit? */
  if (au_shift != INVALID_SHIFT)
    insn->bits |= (au << au_shift);

  /* Does this instruction have an 'LS' bit?  */
  if (ls_shift != INVALID_SHIFT)
    insn->bits |= (load << ls_shift);

  /* Does this instruction have an 'AR' bit?  */
  if (ar)
      insn->bits |= (1 << ar_shift);

  if (du_shift != INVALID_SHIFT)
    insn->bits |= (du << du_shift);

  if (sc_shift != INVALID_SHIFT)
    insn->bits |= (sc << sc_shift);

  if (om_shift != INVALID_SHIFT)
    insn->bits |= (om << om_shift);

  if (o2r_shift != INVALID_SHIFT)
    insn->bits |= (o2r << o2r_shift);

  if (qn_shift != INVALID_SHIFT)
    insn->bits |= (qn << qn_shift);

  if (qr_shift != INVALID_SHIFT)
    insn->bits |= (qr << qr_shift);

  if (qd_shift != INVALID_SHIFT)
    insn->bits |= (is_quickrot64 << qd_shift);

  if (a1_shift != INVALID_SHIFT)
    insn->bits |= (a1 << a1_shift);

  if (a2_shift != INVALID_SHIFT)
    insn->bits |= (a2 << a2_shift);

  if (su_shift != INVALID_SHIFT)
    insn->bits |= (su << su_shift);

  if (imm_shift != INVALID_SHIFT)
    insn->bits |= ((imm & imm_mask) << imm_shift);

  if (ac_shift != INVALID_SHIFT)
    insn->bits |= (ac << ac_shift);

  if (mx_shift != INVALID_SHIFT)
    insn->bits |= (mx << mx_shift);

  if (is_dual)
    {
      if (l1_shift == INVALID_SHIFT)
	{
	  as_bad (_("'L' modifier not valid for this instruction"));
	  return NULL;
	}

      insn->bits |= (1 << l1_shift);
    }

  insn->len = 4;

  return l;
}

typedef const char *(*insn_parser)(const char *, metag_insn *,
				   const insn_template *);

/* Parser table.  */
static const insn_parser insn_parsers[ENC_MAX] =
  {
    [ENC_NONE] = parse_none,
    [ENC_MOV_U2U] = parse_mov_u2u,
    [ENC_MOV_PORT] = parse_mov_port,
    [ENC_MMOV] = parse_mmov,
    [ENC_MDRD] = parse_mdrd,
    [ENC_MOVL_TTREC] = parse_movl_ttrec,
    [ENC_GET_SET] = parse_get_set,
    [ENC_GET_SET_EXT] = parse_get_set_ext,
    [ENC_MGET_MSET] = parse_mget_mset,
    [ENC_COND_SET] = parse_cond_set,
    [ENC_XFR] = parse_xfr,
    [ENC_MOV_CT] = parse_mov_ct,
    [ENC_SWAP] = parse_swap,
    [ENC_JUMP] = parse_jump,
    [ENC_CALLR] = parse_callr,
    [ENC_ALU] = parse_alu,
    [ENC_SHIFT] = parse_shift,
    [ENC_MIN_MAX] = parse_min_max,
    [ENC_BITOP] = parse_bitop,
    [ENC_CMP] = parse_cmp,
    [ENC_BRANCH] = parse_branch,
    [ENC_KICK] = parse_kick,
    [ENC_SWITCH] = parse_switch,
    [ENC_CACHER] = parse_cacher,
    [ENC_CACHEW] = parse_cachew,
    [ENC_ICACHE] = parse_icache,
    [ENC_LNKGET] = parse_lnkget,
    [ENC_FMOV] = parse_fmov,
    [ENC_FMMOV] = parse_fmmov,
    [ENC_FMOV_DATA] = parse_fmov_data,
    [ENC_FMOV_I] = parse_fmov_i,
    [ENC_FPACK] = parse_fpack,
    [ENC_FSWAP] = parse_fswap,
    [ENC_FCMP] = parse_fcmp,
    [ENC_FMINMAX] = parse_fminmax,
    [ENC_FCONV] = parse_fconv,
    [ENC_FCONVX] = parse_fconvx,
    [ENC_FBARITH] = parse_fbarith,
    [ENC_FEARITH] = parse_fearith,
    [ENC_FREC] = parse_frec,
    [ENC_FSIMD] = parse_fsimd,
    [ENC_FGET_SET_ACF] = parse_fget_set_acf,
    [ENC_DGET_SET] = parse_dget_set,
    [ENC_DTEMPLATE] = parse_dtemplate,
    [ENC_DALU] = parse_dalu,
  };

struct metag_core_option
{
  const char *name;
  unsigned int value;
};

/* CPU type options.  */
static const struct metag_core_option metag_cpus[] =
  {
    {"all",               CoreMeta11|CoreMeta12|CoreMeta21},
    {"metac11",           CoreMeta11},
    {"metac12",           CoreMeta12},
    {"metac21",           CoreMeta21},
    {NULL,                0},
  };

/* FPU type options.  */
static const struct metag_core_option metag_fpus[] =
  {
    {"metac21",           FpuMeta21},
    {NULL,                0},
  };

/* DSP type options.  */
static const struct metag_core_option metag_dsps[] =
  {
    {"metac21",           DspMeta21},
    {NULL,                0},
  };

/* Parse a CPU command line option.  */
static bool
metag_parse_cpu (const char * str)
{
  const struct metag_core_option * opt;
  int optlen;

  optlen = strlen (str);

  if (optlen == 0)
    {
      as_bad (_("missing cpu name `%s'"), str);
      return 0;
    }

  for (opt = metag_cpus; opt->name != NULL; opt++)
    if (strncmp (opt->name, str, optlen) == 0)
      {
	mcpu_opt = opt->value;
	return 1;
      }

  as_bad (_("unknown cpu `%s'"), str);
  return 0;
}

/* Parse an FPU command line option.  */
static bool
metag_parse_fpu (const char * str)
{
  const struct metag_core_option * opt;
  int optlen;

  optlen = strlen (str);

  if (optlen == 0)
    {
      as_bad (_("missing fpu name `%s'"), str);
      return 0;
    }

  for (opt = metag_fpus; opt->name != NULL; opt++)
    if (strncmp (opt->name, str, optlen) == 0)
      {
	mfpu_opt = opt->value;
	return 1;
      }

  as_bad (_("unknown fpu `%s'"), str);
  return 0;
}

/* Parse a DSP command line option.  */
static bool
metag_parse_dsp (const char * str)
{
  const struct metag_core_option * opt;
  int optlen;

  optlen = strlen (str);

  if (optlen == 0)
    {
      as_bad (_("missing DSP name `%s'"), str);
      return 0;
    }

  for (opt = metag_dsps; opt->name != NULL; opt++)
    if (strncmp (opt->name, str, optlen) == 0)
      {
	mdsp_opt = opt->value;
	return 1;
      }

  as_bad (_("unknown DSP `%s'"), str);
  return 0;
}

struct metag_long_option
{
  const char *option;			/* Substring to match.  */
  const char *help;			/* Help information.  */
  bool (*func) (const char *subopt);	/* Function to decode sub-option.  */
  const char *deprecated;		/* If non-null, print this message.  */
};

struct metag_long_option metag_long_opts[] =
  {
    {"mcpu=", N_("<cpu name>\t  assemble for CPU <cpu name>"),
     metag_parse_cpu, NULL},
    {"mfpu=", N_("<fpu name>\t  assemble for FPU architecture <fpu name>"),
     metag_parse_fpu, NULL},
    {"mdsp=", N_("<dsp name>\t  assemble for DSP architecture <dsp name>"),
     metag_parse_dsp, NULL},
    {NULL, NULL, 0, NULL}
  };

int
md_parse_option (int c, const char * arg)
{
  struct metag_long_option *lopt;

  for (lopt = metag_long_opts; lopt->option != NULL; lopt++)
    {
      /* These options are expected to have an argument.  */
      if (c == lopt->option[0]
	  && arg != NULL
	  && startswith (arg, lopt->option + 1))
	{
#if WARN_DEPRECATED
	      /* If the option is deprecated, tell the user.  */
	      if (lopt->deprecated != NULL)
		as_tsktsk (_("option `-%c%s' is deprecated: %s"), c, arg,
			   _(lopt->deprecated));
#endif

	      /* Call the sup-option parser.  */
	      return lopt->func (arg + strlen (lopt->option) - 1);
	}
    }

  return 0;
}

void
md_show_usage (FILE * stream)
{
  struct metag_long_option *lopt;

  fprintf (stream, _(" Meta specific command line options:\n"));

  for (lopt = metag_long_opts; lopt->option != NULL; lopt++)
    if (lopt->help != NULL)
      fprintf (stream, "  -%s%s\n", lopt->option, _(lopt->help));
}

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  { "word",	cons,		2 },
  { NULL, 	NULL, 		0 }
};

void
md_begin (void)
{
  int c;

  for (c = 0; c < 256; c++)
    {
      if (ISDIGIT (c))
	{
	  register_chars[c] = c;
	  /* LOCK0, LOCK1, LOCK2.  */
	  mnemonic_chars[c] = c;
	}
      else if (ISLOWER (c))
	{
	  register_chars[c] = c;
	  mnemonic_chars[c] = c;
	}
      else if (ISUPPER (c))
	{
	  register_chars[c] = c;
	  mnemonic_chars[c] = c;
	}
      else if (c == '.')
	{
	  register_chars[c] = c;
	}
    }
}

/* Parse a split condition code prefix.  */
static const char *
parse_split_condition (const char *line, metag_insn *insn)
{
  const char *l = line;
  const split_condition *scond;
  split_condition entry;
  char buf[4];

  memcpy (buf, l, 4);
  buf[3] = '\0';

  entry.name = buf;

  scond = (const split_condition *) htab_find (scond_htab, &entry);

  if (!scond)
    return NULL;

  insn->scond = scond->code;

  return l + strlen (scond->name);
}

/* Parse an instruction prefix - F for float, D for DSP - and associated
   flags and condition codes.  */
static const char *
parse_prefix (const char *line, metag_insn *insn)
{
  const char *l = line;

  l = skip_whitespace (l);

  insn->type = INSN_GP;

  if (TOLOWER (*l) == FPU_PREFIX_CHAR)
    {
      if (strncasecmp (l, FFB_INSN, strlen(FFB_INSN)))
	{
	  insn->type = INSN_FPU;

	  l++;

	  if (*l == END_OF_INSN)
	    {
	      as_bad (_("premature end of floating point prefix"));
	      return NULL;
	    }

	  if (TOLOWER (*l) == FPU_DOUBLE_CHAR)
	    {
	      insn->fpu_width = FPU_WIDTH_DOUBLE;
	      l++;
	    }
	  else if (TOLOWER (*l) == FPU_PAIR_CHAR)
	    {
	      const char *l2 = l;

	      /* Check this isn't a split condition beginning with L.  */
	      l2 = parse_split_condition (l2, insn);

	      if (l2 && is_whitespace_char (*l2))
		{
		  l = l2;
		}
	      else
		{
		  insn->fpu_width = FPU_WIDTH_PAIR;
		  l++;
		}
	    }
	  else
	    {
	      insn->fpu_width = FPU_WIDTH_SINGLE;
	    }

	  if (TOLOWER (*l) == FPU_ACTION_ABS_CHAR)
	    {
	      insn->fpu_action_flags |= FPU_ACTION_ABS;
	      l++;
	    }
	  else if (TOLOWER (*l) == FPU_ACTION_INV_CHAR)
	    {
	      insn->fpu_action_flags |= FPU_ACTION_INV;
	      l++;
	    }

	  if (TOLOWER (*l) == FPU_ACTION_QUIET_CHAR)
	    {
	      insn->fpu_action_flags |= FPU_ACTION_QUIET;
	      l++;
	    }

	  if (TOLOWER (*l) == FPU_ACTION_ZERO_CHAR)
	    {
	      insn->fpu_action_flags |= FPU_ACTION_ZERO;
	      l++;
	    }

	  if (! is_whitespace_char (*l))
	    {
	      l = parse_split_condition (l, insn);

	      if (!l)
		{
		  as_bad (_("unknown floating point prefix character"));
		  return NULL;
		}
	    }

	  l = skip_space (l);
	}
    }
  else if (TOLOWER (*l) == DSP_PREFIX_CHAR)
    {
      if (strncasecmp (l, DCACHE_INSN, strlen (DCACHE_INSN)) &&
	  strncasecmp (l, DEFR_INSN, strlen (DEFR_INSN)))
	{
	  const char *ll = l;
	  insn->type = INSN_DSP;

	  l++;

	  insn->dsp_width = DSP_WIDTH_SINGLE;

	  while (!is_whitespace_char (*l))
	    {
	      /* We have to check for split condition codes first
		 because they are the longest strings to match,
		 e.g. if the string contains "LLS" we want it to match
		 the split condition code "LLS", not the dual unit
		 character "L".  */
	      ll = l;
	      l = parse_split_condition (l, insn);

	      if (l == NULL)
		l = ll;
	      else
		continue;

	      /* Accept an FPU prefix char which may be used when doing
		 template MOV with FPU registers. */
	      if (TOLOWER(*l) == FPU_PREFIX_CHAR)
		{
		  insn->type = INSN_DSP_FPU;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_DUAL_CHAR)
		{
		  insn->dsp_width = DSP_WIDTH_DUAL;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_QR64_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_QR64;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_UMUL_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_UMUL;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_ROUND_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_ROUND;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_CLAMP9_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_CLAMP9;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_CLAMP8_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_CLAMP8;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_MOD_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_MOD;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_ACC_ZERO_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_ACC_ZERO;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_ACC_ADD_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_ACC_ADD;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_ACC_SUB_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_ACC_SUB;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_ACTION_OV_CHAR)
		{
		  insn->dsp_action_flags |= DSP_ACTION_OV;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_DAOPPAME_8_CHAR)
		{
		  insn->dsp_daoppame_flags |= DSP_DAOPPAME_8;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_DAOPPAME_16_CHAR)
		{
		  insn->dsp_daoppame_flags |= DSP_DAOPPAME_16;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_DAOPPAME_TEMP_CHAR)
		{
		  insn->dsp_daoppame_flags |= DSP_DAOPPAME_TEMP;
		  l++;
		  continue;
		}

	      if (TOLOWER(*l) == DSP_DAOPPAME_HIGH_CHAR)
		{
		  insn->dsp_daoppame_flags |= DSP_DAOPPAME_HIGH;
		  l++;
		  continue;
		}

	      as_bad (_("unknown DSP prefix character %c %s"), *l, l);
	      return NULL;
	    }

	  l = skip_space (l);
	}
    }

  return l;
}

/* Return a list of appropriate instruction parsers for MNEMONIC.  */
static insn_templates *
find_insn_templates (const char *mnemonic)
{
  insn_template template;
  insn_templates entry;
  insn_templates *slot;

  entry.template = &template;

  memcpy ((void *)&entry.template->name, &mnemonic, sizeof (char *));

  slot = (insn_templates *) htab_find (mnemonic_htab, &entry);

  if (slot)
    return slot;

  return NULL;
}

/* Make an uppercase copy of SRC into DST and return DST.  */
static char *
strupper (char * dst, const char *src)
{
  size_t i = 0;

  while (src[i])
    {
      dst[i] = TOUPPER (src[i]);
      i++;
    }

  dst[i] = 0;

  return dst;
}

/* Calculate a hash value for a template. */
static hashval_t
hash_templates (const void *p)
{
  insn_templates *tp = (insn_templates *)p;
  char buf[MAX_MNEMONIC_LEN];

  strupper (buf, tp->template->name);

  return htab_hash_string (buf);
}

/* Check if two templates are equal.  */
static int
eq_templates (const void *a, const void *b)
{
  insn_templates *ta = (insn_templates *)a;
  insn_templates *tb = (insn_templates *)b;
  return strcasecmp (ta->template->name, tb->template->name) == 0;
}

/* Create the hash table required for parsing instructions.  */
static void
create_mnemonic_htab (void)
{
  size_t i, num_templates = sizeof(metag_optab)/sizeof(metag_optab[0]);

  mnemonic_htab = htab_create_alloc (num_templates, hash_templates,
				     eq_templates, NULL, xcalloc, free);

  for (i = 0; i < num_templates; i++)
    {
      const insn_template *template = &metag_optab[i];
      insn_templates **slot = NULL;
      insn_templates *new_entry;

      new_entry = XNEW (insn_templates);

      new_entry->template = template;
      new_entry->next = NULL;

      slot = (insn_templates **) htab_find_slot (mnemonic_htab, new_entry,
						 INSERT);

      if (*slot)
	{
	  insn_templates *last_entry = *slot;

	  while (last_entry->next)
	    last_entry = last_entry->next;

	  last_entry->next = new_entry;
	}
      else
	{
	  *slot = new_entry;
	}
    }
}

/* Calculate a hash value for a register. */
static hashval_t
hash_regs (const void *p)
{
  metag_reg *rp = (metag_reg *)p;
  char buf[MAX_REG_LEN];

  strupper (buf, rp->name);

  return htab_hash_string (buf);
}

/* Check if two registers are equal.  */
static int
eq_regs (const void *a, const void *b)
{
  metag_reg *ra = (metag_reg *)a;
  metag_reg *rb = (metag_reg *)b;
  return strcasecmp (ra->name, rb->name) == 0;
}

/* Create the hash table required for parsing registers.  */
static void
create_reg_htab (void)
{
  size_t i, num_regs = sizeof(metag_regtab)/sizeof(metag_regtab[0]);

  reg_htab = htab_create_alloc (num_regs, hash_regs,
				eq_regs, NULL, xcalloc, free);

  for (i = 0; i < num_regs; i++)
    {
      const metag_reg *reg = &metag_regtab[i];
      const metag_reg **slot;

      slot = (const metag_reg **) htab_find_slot (reg_htab, reg, INSERT);

      if (!*slot)
	*slot = reg;
    }
}

/* Create the hash table required for parsing DSP registers.  */
static void
create_dspreg_htabs (void)
{
  size_t i, num_regs = sizeof(metag_dsp_regtab)/sizeof(metag_dsp_regtab[0]);
  size_t h;

  dsp_reg_htab = htab_create_alloc (num_regs, hash_regs,
				    eq_regs, NULL, xcalloc, free);

  for (i = 0; i < num_regs; i++)
    {
      const metag_reg *reg = &metag_dsp_regtab[i];
      const metag_reg **slot;

      slot = (const metag_reg **) htab_find_slot (dsp_reg_htab, reg, INSERT);

      /* Make sure there are no hash table collisions, which would
	 require chaining entries.  */
      gas_assert (*slot == NULL);
      *slot = reg;
    }

  num_regs = sizeof(metag_dsp_tmpl_regtab[0])/sizeof(metag_dsp_tmpl_regtab[0][0]);

  for (h = 0; h < 2; h++)
    {
      dsp_tmpl_reg_htab[h] = htab_create_alloc (num_regs, hash_regs,
						eq_regs, NULL, xcalloc, free);
    }

  for (h = 0; h < 2; h++)
    {
      for (i = 0; i < num_regs; i++)
	{
	  const metag_reg *reg = &metag_dsp_tmpl_regtab[h][i];
	  const metag_reg **slot;
	  slot = (const metag_reg **) htab_find_slot (dsp_tmpl_reg_htab[h],
						      reg, INSERT);

	  /* Make sure there are no hash table collisions, which would
	     require chaining entries.  */
	  gas_assert (*slot == NULL);
	  *slot = reg;
	}
    }
}

/* Calculate a hash value for a split condition code. */
static hashval_t
hash_scond (const void *p)
{
  split_condition *cp = (split_condition *)p;
  char buf[4];

  strupper (buf, cp->name);

  return htab_hash_string (buf);
}

/* Check if two split condition codes are equal.  */
static int
eq_scond (const void *a, const void *b)
{
  split_condition *ra = (split_condition *)a;
  split_condition *rb = (split_condition *)b;

  return strcasecmp (ra->name, rb->name) == 0;
}

/* Create the hash table required for parsing split condition codes.  */
static void
create_scond_htab (void)
{
  size_t i, nentries;

  nentries = sizeof (metag_scondtab) / sizeof (metag_scondtab[0]);

  scond_htab = htab_create_alloc (nentries, hash_scond, eq_scond,
				  NULL, xcalloc, free);
  for (i = 0; i < nentries; i++)
    {
      const split_condition *scond = &metag_scondtab[i];
      const split_condition **slot;

      slot = (const split_condition **) htab_find_slot (scond_htab,
							scond, INSERT);
      /* Make sure there are no hash table collisions, which would
	 require chaining entries.  */
      gas_assert (*slot == NULL);
      *slot = scond;
    }
}

/* Entry point for instruction parsing.  */
static bool
parse_insn (const char *line, metag_insn *insn)
{
  char mnemonic[MAX_MNEMONIC_LEN];
  const char *l = line;
  size_t mnemonic_len = 0;
  insn_templates *templates;

  l = skip_space (l);

  while (is_mnemonic_char(*l))
    {
      l++;
      mnemonic_len++;
    }

  if (mnemonic_len >= MAX_MNEMONIC_LEN)
    {
      as_bad (_("instruction mnemonic too long: %s"), line);
      return false;
    }

  strncpy(mnemonic, line, mnemonic_len);

  mnemonic[mnemonic_len] = '\0';

  templates = find_insn_templates (mnemonic);

  if (templates)
    {
      insn_templates *current_template = templates;

      l = skip_space (l);

      while (current_template)
	{
	  const insn_template *template = current_template->template;
	  enum insn_encoding encoding = template->encoding;
	  insn_parser parser = insn_parsers[encoding];

	  current_template = current_template->next;

	  if (template->insn_type == INSN_GP &&
	      !(template->core_flags & mcpu_opt))
	    continue;

	  if (template->insn_type == INSN_FPU &&
	      !(template->core_flags & mfpu_opt))
	    continue;

	  if (template->insn_type == INSN_DSP &&
	      !(template->core_flags & mdsp_opt))
	    continue;

	  if (template->insn_type == INSN_DSP_FPU &&
	      !((template->core_flags & mdsp_opt) &&
		(template->core_flags & mfpu_opt)))
	    continue;

	  /* DSP instructions always require special decoding */
	  if ((insn->type == INSN_DSP && (template->insn_type != INSN_DSP)) ||
	      ((template->insn_type == INSN_DSP) && insn->type != INSN_DSP) ||
	      (insn->type == INSN_DSP_FPU && (template->insn_type != INSN_DSP_FPU)) ||
	      ((template->insn_type == INSN_DSP_FPU) && insn->type != INSN_DSP_FPU))
	    continue;

	  if (parser)
	    {
	      const char *end = parser(l, insn, template);

	      if (end != NULL)
		{
		  if (*end != END_OF_INSN)
		    as_bad (_("junk at end of line: \"%s\""), line);
		  else
		    return true;
		}
	    }
	}

      as_bad (_("failed to assemble instruction: \"%s\""), line);
    }
  else
    {
      if (insn->type == INSN_FPU)
	as_bad (_("unknown floating point mnemonic: \"%s\""), mnemonic);
      else
	as_bad (_("unknown mnemonic: \"%s\""), mnemonic);
    }
  return false;
}

static void
output_insn (metag_insn *insn)
{
  char *output;

  output = frag_more (insn->len);
  dwarf2_emit_insn (insn->len);

  if (insn->reloc_type != BFD_RELOC_UNUSED)
    {
      fix_new_exp (frag_now, output - frag_now->fr_literal,
		   insn->reloc_size, &insn->reloc_exp,
		   insn->reloc_pcrel, insn->reloc_type);
    }

  md_number_to_chars (output, insn->bits, insn->len);
}

void
md_assemble (char *line)
{
  const char *l = line;
  metag_insn insn;

  memset (&insn, 0, sizeof(insn));

  insn.reloc_type = BFD_RELOC_UNUSED;
  insn.reloc_pcrel = 0;
  insn.reloc_size = 4;

  if (!mnemonic_htab)
    {
      create_mnemonic_htab ();
      create_reg_htab ();
      create_dspreg_htabs ();
      create_scond_htab ();
    }

  l = parse_prefix (l, &insn);

  if (l == NULL)
    return;

  if (insn.type == INSN_DSP &&
      !mdsp_opt)
    {
      as_bad (_("cannot assemble DSP instruction, DSP option not set: %s"),
	      line);
      return;
    }
  else if (insn.type == INSN_FPU &&
	   !mfpu_opt)
    {
      as_bad (_("cannot assemble FPU instruction, FPU option not set: %s"),
	      line);
      return;
    }

  if (!parse_insn (l, &insn))
    return;

  output_insn (&insn);
}

void
md_operand (expressionS * expressionP)
{
  if (* input_line_pointer == IMM_CHAR)
    {
      input_line_pointer ++;
      expression (expressionP);
    }
}

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
  return size;
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Functions concerning relocs.  */

/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from_section (fixS * fixP, segT sec)
{
  if ((fixP->fx_addsy != (symbolS *) NULL
       && (! S_IS_DEFINED (fixP->fx_addsy)
	   || S_GET_SEGMENT (fixP->fx_addsy) != sec))
      || metag_force_relocation (fixP))
    {
      /* The symbol is undefined (or is defined but not in this section).
	 Let the linker figure it out.  */
      return 0;
    }

  return fixP->fx_frag->fr_address + fixP->fx_where;
}

/* Write a value out to the object file, using the appropriate endianness.  */

void
md_number_to_chars (char * buf, valueT val, int n)
{
  number_to_chars_littleendian (buf, val, n);
}

/* Turn a string in input_line_pointer into a floating point constant of type
   type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
   emitted is stored in *sizeP .  An error message is returned, or NULL on OK.
*/

const char *
md_atof (int type, char * litP, int * sizeP)
{
  int              i;
  int              prec;
  LITTLENUM_TYPE   words [MAX_LITTLENUMS];
  char *           t;

  switch (type)
    {
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

   /* FIXME: Some targets allow other format chars for bigger sizes here.  */

    default:
      * sizeP = 0;
      return _("Bad call to md_atof()");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  * sizeP = prec * sizeof (LITTLENUM_TYPE);

  for (i = 0; i < prec; i++)
    {
      md_number_to_chars (litP, (valueT) words[i],
			  sizeof (LITTLENUM_TYPE));
      litP += sizeof (LITTLENUM_TYPE);
    }

  return 0;
}

/* If this function returns non-zero, it prevents the relocation
   against symbol(s) in the FIXP from being replaced with relocations
   against section symbols, and guarantees that a relocation will be
   emitted even when the value can be resolved locally.  */

int
metag_force_relocation (fixS * fix)
{
  switch (fix->fx_r_type)
    {
    case BFD_RELOC_METAG_RELBRANCH_PLT:
    case BFD_RELOC_METAG_TLS_LE:
    case BFD_RELOC_METAG_TLS_IE:
    case BFD_RELOC_METAG_TLS_LDO:
    case BFD_RELOC_METAG_TLS_LDM:
    case BFD_RELOC_METAG_TLS_GD:
      return 1;
    default:
      ;
    }

  return generic_force_reloc (fix);
}

bool
metag_fix_adjustable (fixS * fixP)
{
  if (fixP->fx_addsy == NULL)
    return 1;

  /* Prevent all adjustments to global symbols.  */
  if (S_IS_EXTERNAL (fixP->fx_addsy))
    return 0;
  if (S_IS_WEAK (fixP->fx_addsy))
    return 0;

  if (fixP->fx_r_type == BFD_RELOC_METAG_HI16_GOTOFF ||
      fixP->fx_r_type == BFD_RELOC_METAG_LO16_GOTOFF ||
      fixP->fx_r_type == BFD_RELOC_METAG_GETSET_GOTOFF ||
      fixP->fx_r_type == BFD_RELOC_METAG_GETSET_GOT ||
      fixP->fx_r_type == BFD_RELOC_METAG_HI16_GOTPC ||
      fixP->fx_r_type == BFD_RELOC_METAG_LO16_GOTPC ||
      fixP->fx_r_type == BFD_RELOC_METAG_HI16_PLT ||
      fixP->fx_r_type == BFD_RELOC_METAG_LO16_PLT ||
      fixP->fx_r_type == BFD_RELOC_METAG_RELBRANCH_PLT)
    return 0;

  /* We need the symbol name for the VTABLE entries.  */
  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return 0;

  return 1;
}

/* Return an initial guess of the length by which a fragment must grow to
   hold a branch to reach its destination.
   Also updates fr_type/fr_subtype as necessary.

   Called just before doing relaxation.
   Any symbol that is now undefined will not become defined.
   The guess for fr_var is ACTUALLY the growth beyond fr_fix.
   Whatever we do to grow fr_fix or fr_var contributes to our returned value.
   Although it may not be explicit in the frag, pretend fr_var starts with a
   0 value.  */

int
md_estimate_size_before_relax (fragS * fragP ATTRIBUTE_UNUSED,
			       segT    segment ATTRIBUTE_UNUSED)
{
  /* No assembler relaxation is defined (or necessary) for this port.  */
  abort ();
}

/* *fragP has been relaxed to its final size, and now needs to have
   the bytes inside it modified to conform to the new size.

   Called after relaxation is finished.
   fragP->fr_type == rs_machine_dependent.
   fragP->fr_subtype is the subtype of what the address relaxed to.  */

void
md_convert_frag (bfd * abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED,
		 fragS * fragP ATTRIBUTE_UNUSED)
{
  /* No assembler relaxation is defined (or necessary) for this port.  */
  abort ();
}

/* This is called from HANDLE_ALIGN in tc-metag.h.  */

void
metag_handle_align (fragS * fragP)
{
  static unsigned char const noop[4] = { 0xfe, 0xff, 0xff, 0xa0 };
  int bytes, fix;
  char *p;

  if (fragP->fr_type != rs_align_code)
    return;

  bytes = fragP->fr_next->fr_address - fragP->fr_address - fragP->fr_fix;
  p = fragP->fr_literal + fragP->fr_fix;
  fix = 0;

  if (bytes & 3)
    {
      fix = bytes & 3;
      memset (p, 0, fix);
      p += fix;
      bytes -= fix;
    }

  while (bytes >= 4)
    {
      memcpy (p, noop, 4);
      p += 4;
      bytes -= 4;
      fix += 4;
    }

  fragP->fr_fix += fix;
  fragP->fr_var = 4;
}

static char *
metag_end_of_match (char * cont, const char * what)
{
  int len = strlen (what);

  if (strncasecmp (cont, what, strlen (what)) == 0
      && ! is_part_of_name (cont[len]))
    return cont + len;

  return NULL;
}

int
metag_parse_name (char const * name, expressionS * exprP, enum expr_mode mode,
		  char * nextcharP)
{
  char *next = input_line_pointer;
  char *next_end;
  int reloc_type;
  operatorT op_type;
  segT segment;

  exprP->X_op_symbol = NULL;
  exprP->X_md = BFD_RELOC_UNUSED;

  if (strcmp (name, GOT_NAME) == 0)
    {
      if (! GOT_symbol)
	GOT_symbol = symbol_find_or_make (name);

      exprP->X_add_symbol = GOT_symbol;
    no_suffix:
      /* If we have an absolute symbol or a
	 reg, then we know its value now.  */
      segment = S_GET_SEGMENT (exprP->X_add_symbol);
      if (mode != expr_defer && segment == absolute_section)
	{
	  exprP->X_op = O_constant;
	  exprP->X_add_number = S_GET_VALUE (exprP->X_add_symbol);
	  exprP->X_add_symbol = NULL;
	}
      else if (mode != expr_defer && segment == reg_section)
	{
	  exprP->X_op = O_register;
	  exprP->X_add_number = S_GET_VALUE (exprP->X_add_symbol);
	  exprP->X_add_symbol = NULL;
	}
      else
	{
	  exprP->X_op = O_symbol;
	  exprP->X_add_number = 0;
	}

      return 1;
    }

  exprP->X_add_symbol = symbol_find_or_make (name);

  if (*nextcharP != '@')
    goto no_suffix;
  else if ((next_end = metag_end_of_match (next + 1, "GOTOFF")))
    {
      reloc_type = BFD_RELOC_METAG_GOTOFF;
      op_type = O_PIC_reloc;
    }
  else if ((next_end = metag_end_of_match (next + 1, "GOT")))
    {
      reloc_type = BFD_RELOC_METAG_GETSET_GOT;
      op_type = O_PIC_reloc;
    }
  else if ((next_end = metag_end_of_match (next + 1, "PLT")))
    {
      reloc_type = BFD_RELOC_METAG_PLT;
      op_type = O_PIC_reloc;
    }
  else if ((next_end = metag_end_of_match (next + 1, "TLSGD")))
    {
      reloc_type = BFD_RELOC_METAG_TLS_GD;
      op_type = O_PIC_reloc;
    }
  else if ((next_end = metag_end_of_match (next + 1, "TLSLDM")))
    {
      reloc_type = BFD_RELOC_METAG_TLS_LDM;
      op_type = O_PIC_reloc;
    }
  else if ((next_end = metag_end_of_match (next + 1, "TLSLDO")))
    {
      reloc_type = BFD_RELOC_METAG_TLS_LDO;
      op_type = O_PIC_reloc;
    }
  else if ((next_end = metag_end_of_match (next + 1, "TLSIE")))
    {
      reloc_type = BFD_RELOC_METAG_TLS_IE;
      op_type = O_PIC_reloc;
    }
  else if ((next_end = metag_end_of_match (next + 1, "TLSIENONPIC")))
    {
      reloc_type = BFD_RELOC_METAG_TLS_IENONPIC;
      op_type = O_PIC_reloc;	/* FIXME: is this correct? */
    }
  else if ((next_end = metag_end_of_match (next + 1, "TLSLE")))
    {
      reloc_type = BFD_RELOC_METAG_TLS_LE;
      op_type = O_PIC_reloc;
    }
  else
    goto no_suffix;

  *input_line_pointer = *nextcharP;
  input_line_pointer = next_end;
  *nextcharP = *input_line_pointer;
  *input_line_pointer = '\0';

  exprP->X_op = op_type;
  exprP->X_add_number = 0;
  exprP->X_md = reloc_type;

  return 1;
}

/* If while processing a fixup, a reloc really needs to be created
   then it is done here.  */

arelent *
tc_gen_reloc (asection *seg ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc;

  reloc		      = XNEW (arelent);
  reloc->sym_ptr_ptr  = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address      = fixp->fx_frag->fr_address + fixp->fx_where;

  reloc->addend = fixp->fx_offset;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);

  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    /* xgettext:c-format.  */
		    _("reloc %d not supported by object file format"),
		    (int) fixp->fx_r_type);

      xfree (reloc);

      return NULL;
    }

  return reloc;
}

static unsigned int
md_chars_to_number (char *val, int n)
{
  unsigned int retval;
  unsigned char * where = (unsigned char *) val;

  for (retval = 0; n--;)
    {
      retval <<= 8;
      retval |= where[n];
    }
  return retval;
}

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  int value = (int)*valP;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_METAG_TLS_GD:
    case BFD_RELOC_METAG_TLS_LE_HI16:
    case BFD_RELOC_METAG_TLS_LE_LO16:
    case BFD_RELOC_METAG_TLS_IE:
    case BFD_RELOC_METAG_TLS_IENONPIC_HI16:
    case BFD_RELOC_METAG_TLS_IENONPIC_LO16:
    case BFD_RELOC_METAG_TLS_LDM:
    case BFD_RELOC_METAG_TLS_LDO_HI16:
    case BFD_RELOC_METAG_TLS_LDO_LO16:
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      /* Fall through */

    case BFD_RELOC_METAG_HIADDR16:
    case BFD_RELOC_METAG_LOADDR16:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = false;
      break;

    case BFD_RELOC_METAG_REL8:
      if (!within_unsigned_range (value, IMM8_BITS))
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			"rel8 out of range %d", value);
	}
      else
	{
	  unsigned int newval;
	  newval = md_chars_to_number (buf, 4);
	  newval = (newval & 0xffffc03f) | ((value & IMM8_MASK) << 6);
	  md_number_to_chars (buf, newval, 4);
	}
      break;
    case BFD_RELOC_METAG_REL16:
      if (!within_unsigned_range (value, IMM16_BITS))
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			"rel16 out of range %d", value);
	}
      else
	{
	  unsigned int newval;
	  newval = md_chars_to_number (buf, 4);
	  newval = (newval & 0xfff80007) | ((value & IMM16_MASK) << 3);
	  md_number_to_chars (buf, newval, 4);
	}
      break;

    case BFD_RELOC_8:
      md_number_to_chars (buf, value, 1);
      break;
    case BFD_RELOC_16:
      md_number_to_chars (buf, value, 2);
      break;
    case BFD_RELOC_32:
      md_number_to_chars (buf, value, 4);
      break;
    case BFD_RELOC_64:
      md_number_to_chars (buf, value, 8);
      break;

    case BFD_RELOC_METAG_RELBRANCH:
      if (!value)
	break;

      value = value / 4;

      if (!within_signed_range (value, IMM19_BITS))
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			"relbranch out of range %d", value);
	}
      else
	{
	  unsigned int newval;
	  newval = md_chars_to_number (buf, 4);
	  newval = (newval & 0xff00001f) | ((value & IMM19_MASK) << 5);
	  md_number_to_chars (buf, newval, 4);
	}
	break;
    default:
      break;
    }

  if (fixP->fx_addsy == NULL)
    fixP->fx_done = true;
}
