/* tc-pdp11.c - pdp11-specific -
   Copyright (C) 2001-2023 Free Software Foundation, Inc.

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
#include "opcode/pdp11.h"

extern int flonum_gen2vax (int, FLONUM_TYPE * f, LITTLENUM_TYPE *);

/* A representation for PDP-11 machine code.  */
struct pdp11_code
{
  const char *error;
  int code;
  int additional;	/* Is there an additional word?  */
  int word;		/* Additional word, if any.  */
  struct
  {
    bfd_reloc_code_real_type type;
    expressionS exp;
    int pc_rel;
  } reloc;
};

/* Instruction set extensions.

   If you change this from an array to something else, please update
   the "PDP-11 instruction set extensions" comment in pdp11.h.  */
int pdp11_extension[PDP11_EXT_NUM];

/* Assembly options.  */

#define ASM_OPT_PIC 1
#define ASM_OPT_NUM 2

int asm_option[ASM_OPT_NUM];

/* These chars start a comment anywhere in a source file (except inside
   another comment.  */
const char comment_chars[] = "#/";

/* These chars only start a comment at the beginning of a line.  */
const char line_comment_chars[] = "#/";

const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.  */
/* as in 0f123.456.  */
/* or    0H1.234E-12 (see exp chars above).  */
const char FLT_CHARS[] = "dDfF";

void pseudo_even (int);
void pseudo_bss (int);

const pseudo_typeS md_pseudo_table[] =
{
  { "bss", pseudo_bss, 0 },
  { "even", pseudo_even, 0 },
  { 0, 0, 0 },
};

static htab_t insn_hash = NULL;

static int
set_option (const char *arg)
{
  int yes = 1;

  if (strcmp (arg, "all-extensions") == 0
      || strcmp (arg, "all") == 0)
    {
      memset (pdp11_extension, ~0, sizeof pdp11_extension);
      pdp11_extension[PDP11_NONE] = 0;
      return 1;
    }
  else if (strcmp (arg, "no-extensions") == 0)
    {
      memset (pdp11_extension, 0, sizeof pdp11_extension);
      pdp11_extension[PDP11_BASIC] = 1;
      return 1;
    }

  if (startswith (arg, "no-"))
    {
      yes = 0;
      arg += 3;
    }

  /* Commercial instructions.  */
  if (strcmp (arg, "cis") == 0)
    pdp11_extension[PDP11_CIS] = yes;
  /* Call supervisor mode.  */
  else if (strcmp (arg, "csm") == 0)
    pdp11_extension[PDP11_CSM] = yes;
  /* Extended instruction set.  */
  else if (strcmp (arg, "eis") == 0)
    pdp11_extension[PDP11_EIS] = pdp11_extension[PDP11_LEIS] = yes;
  /* KEV11 floating-point.  */
  else if (strcmp (arg, "fis") == 0
	   || strcmp (arg, "kev11") == 0
	   || strcmp (arg, "kev-11") == 0)
    pdp11_extension[PDP11_FIS] = yes;
  /* FP-11 floating-point.  */
  else if (strcmp (arg, "fpp") == 0
	   || strcmp (arg, "fpu") == 0
	   || strcmp (arg, "fp11") == 0
	   || strcmp (arg, "fp-11") == 0
	   || strcmp (arg, "fpj11") == 0
	   || strcmp (arg, "fp-j11") == 0
	   || strcmp (arg, "fpj-11") == 0)
    pdp11_extension[PDP11_FPP] = yes;
  /* Limited extended insns.  */
  else if (strcmp (arg, "limited-eis") == 0)
    {
      pdp11_extension[PDP11_LEIS] = yes;
      if (!pdp11_extension[PDP11_LEIS])
	pdp11_extension[PDP11_EIS] = 0;
    }
  /* Move from processor type.  */
  else if (strcmp (arg, "mfpt") == 0)
    pdp11_extension[PDP11_MFPT] = yes;
  /* Multiprocessor insns:  */
  else if (startswith (arg, "mproc")
	   /* TSTSET, WRTLCK */
	   || startswith (arg, "multiproc"))
    pdp11_extension[PDP11_MPROC] = yes;
  /* Move from/to proc status.  */
  else if (strcmp (arg, "mxps") == 0)
    pdp11_extension[PDP11_MXPS] = yes;
  /* Position-independent code.  */
  else if (strcmp (arg, "pic") == 0)
    asm_option[ASM_OPT_PIC] = yes;
  /* Set priority level.  */
  else if (strcmp (arg, "spl") == 0)
    pdp11_extension[PDP11_SPL] = yes;
  /* Microcode instructions:  */
  else if (strcmp (arg, "ucode") == 0
	   /* LDUB, MED, XFC */
	   || strcmp (arg, "microcode") == 0)
    pdp11_extension[PDP11_UCODE] = yes;
  else
    return 0;

  return 1;
}


static void
init_defaults (void)
{
  static int first = 1;

  if (first)
    {
      set_option ("all-extensions");
      set_option ("pic");
      first = 0;
    }
}

void
md_begin (void)
{
  int i;

  init_defaults ();

  insn_hash = str_htab_create ();

  for (i = 0; i < pdp11_num_opcodes; i++)
    str_hash_insert (insn_hash, pdp11_opcodes[i].name, pdp11_opcodes + i, 0);
  for (i = 0; i < pdp11_num_aliases; i++)
    str_hash_insert (insn_hash, pdp11_aliases[i].name, pdp11_aliases + i, 0);
}

void
md_number_to_chars (char con[], valueT value, int nbytes)
{
  /* On a PDP-11, 0x1234 is stored as "\x12\x34", and
     0x12345678 is stored as "\x56\x78\x12\x34". It's
     anyone's guess what 0x123456 would be stored like.  */

  switch (nbytes)
    {
    case 0:
      break;
    case 1:
      con[0] =  value       & 0xff;
      break;
    case 2:
      con[0] =  value        & 0xff;
      con[1] = (value >>  8) & 0xff;
      break;
    case 4:
      con[0] = (value >> 16) & 0xff;
      con[1] = (value >> 24) & 0xff;
      con[2] =  value        & 0xff;
      con[3] = (value >>  8) & 0xff;
      break;
#ifdef BFD64
    case 8:
      con[0] = (value >> 48) & 0xff;
      con[1] = (value >> 56) & 0xff;
      con[2] = (value >> 32) & 0xff;
      con[3] = (value >> 40) & 0xff;
      con[4] = (value >> 16) & 0xff;
      con[5] = (value >> 24) & 0xff;
      con[6] =  value        & 0xff;
      con[7] = (value >>  8) & 0xff;
      break;
#endif
    default:
      BAD_CASE (nbytes);
    }
}

/* Fix up some data or instructions after we find out the value of a symbol
   that they reference.  Knows about order of bytes in address.  */

void
md_apply_fix (fixS *fixP,
	       valueT * valP,
	       segT seg ATTRIBUTE_UNUSED)
{
  valueT code;
  valueT mask;
  valueT val = * valP;
  char *buf;
  int shift;
  int size;

  buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  size = fixP->fx_size;
  code = md_chars_to_number ((unsigned char *) buf, size);

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_8:
      mask = 0xff;
      shift = 0;
      break;
    case BFD_RELOC_16:
    case BFD_RELOC_16_PCREL:
      mask = 0xffff;
      shift = 0;
      break;
    case BFD_RELOC_32:
      mask = 0xffffffff;
      shift = 0;
      break;
    case BFD_RELOC_PDP11_DISP_8_PCREL:
      mask = 0x00ff;
      shift = 1;
      break;
    case BFD_RELOC_PDP11_DISP_6_PCREL:
      mask = 0x003f;
      shift = 1;
      val = -val;
      break;
    default:
      BAD_CASE (fixP->fx_r_type);
    }

  if (fixP->fx_addsy != NULL)
    val += symbol_get_bfdsym (fixP->fx_addsy)->section->vma;
    /* *value += fixP->fx_addsy->bsym->section->vma; */

  code &= ~mask;
  code |= (val >> shift) & mask;
  number_to_chars_littleendian (buf, code, size);

  if (fixP->fx_addsy == NULL && fixP->fx_pcrel == 0)
    fixP->fx_done = 1;
}

long
md_chars_to_number (unsigned char *con, int nbytes)
{
  /* On a PDP-11, 0x1234 is stored as "\x12\x34", and
     0x12345678 is stored as "\x56\x78\x12\x34". It's
     anyone's guess what 0x123456 would be stored like.  */
  switch (nbytes)
    {
    case 0:
      return 0;
    case 1:
      return con[0];
    case 2:
      return (con[1] << BITS_PER_CHAR) | con[0];
    case 4:
      return
	(((con[1] << BITS_PER_CHAR) | con[0]) << (2 * BITS_PER_CHAR))
	|((con[3] << BITS_PER_CHAR) | con[2]);
    default:
      BAD_CASE (nbytes);
      return 0;
    }
}

static char *
skip_whitespace (char *str)
{
  while (*str == ' ' || *str == '\t')
    str++;
  return str;
}

static char *
find_whitespace (char *str)
{
  while (*str != ' ' && *str != '\t' && *str != 0)
    str++;
  return str;
}

static char *
parse_reg (char *str, struct pdp11_code *operand)
{
  str = skip_whitespace (str);
  if (TOLOWER (*str) == 'r')
    {
      str++;
      switch (*str)
	{
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	  operand->code = *str - '0';
	  str++;
	  break;
	default:
	  operand->error = _("Bad register name");
	  return str - 1;
	}
    }
  else if (startswith (str, "sp")
	   || startswith (str, "SP"))
    {
      operand->code = 6;
      str += 2;
    }
  else if (startswith (str, "pc")
	   || startswith (str, "PC"))
    {
      operand->code = 7;
      str += 2;
    }
  else
    {
      operand->error = _("Bad register name");
      return str;
    }

  if (ISALNUM (*str) || *str == '_' || *str == '.')
    {
      operand->error = _("Bad register name");
      str -= 2;
    }
  
  return str;
}

static char *
parse_ac5 (char *str, struct pdp11_code *operand)
{
  str = skip_whitespace (str);
  if (startswith (str, "fr")
      || startswith (str, "FR")
      || startswith (str, "ac")
      || startswith (str, "AC"))
    {
      str += 2;
      switch (*str)
	{
	case '0': case '1': case '2': case '3':
        case '4': case '5':
	  operand->code = *str - '0';
	  str++;
	  break;
	default:
	  operand->error = _("Bad register name");
	  return str - 2;
	}
    }
  else
    {
      operand->error = _("Bad register name");
      return str;
    }

  return str;
}

static char *
parse_ac (char *str, struct pdp11_code *operand)
{
  str = parse_ac5 (str, operand);
  if (!operand->error && operand->code > 3)
    {
      operand->error = _("Bad register name");
	  return str - 3;
    }

  return str;
}

static char *
parse_expression (char *str, struct pdp11_code *operand)
{
  char *save_input_line_pointer;
  segT seg;

  save_input_line_pointer = input_line_pointer;
  input_line_pointer = str;
  seg = expression (&operand->reloc.exp);
  if (seg == NULL)
    {
      input_line_pointer = save_input_line_pointer;
      operand->error = _("Error in expression");
      return str;
    }

  str = input_line_pointer;
  input_line_pointer = save_input_line_pointer;

  operand->reloc.pc_rel = 0;

  return str;
}

static char *
parse_op_no_deferred (char *str, struct pdp11_code *operand)
{
  LITTLENUM_TYPE literal_float[2];

  str = skip_whitespace (str);

  switch (*str)
    {
    case '(':				/* (rn) and (rn)+ */
      str = parse_reg (str + 1, operand);
      if (operand->error)
	return str;
      str = skip_whitespace (str);
      if (*str != ')')
	{
	  operand->error = _("Missing ')'");
	  return str;
	}
      str++;
      if (*str == '+')
	{
	  operand->code |= 020;
	  str++;
	}
      else
	{
	  operand->code |= 010;
	}
      break;

      /* Immediate.  */
    case '#':
    case '$':
      str = parse_expression (str + 1, operand);
      if (operand->error)
	return str;
      operand->additional = true;
      operand->word = operand->reloc.exp.X_add_number;
      switch (operand->reloc.exp.X_op)
	{
	case O_constant:
	  break;
	case O_symbol:
	case O_add:
	case O_subtract:
	  operand->reloc.type = BFD_RELOC_16;
	  operand->reloc.pc_rel = 0;
	  break;
        case O_big:
          if (operand->reloc.exp.X_add_number > 0)
            {
              operand->error = _("Error in expression");
              break;
            }
          /* It's a floating literal...  */
          know (operand->reloc.exp.X_add_number < 0);
          flonum_gen2vax ('f', &generic_floating_point_number, literal_float);
          operand->word = literal_float[0];
          if (literal_float[1] != 0)
            as_warn (_("Low order bits truncated in immediate float operand"));
          break;
	default:
	  operand->error = _("Error in expression");
	  break;
	}
      operand->code = 027;
      break;

      /* label, d(rn), -(rn)  */
    default:
      {
	if (startswith (str, "-("))	/* -(rn) */
	  {
	    str = parse_reg (str + 2, operand);
	    if (operand->error)
	      return str;
	    str = skip_whitespace (str);
	    if (*str != ')')
	      {
		operand->error = _("Missing ')'");
		return str;
	      }
	    operand->code |= 040;
	    str++;
	    break;
	  }

	str = parse_expression (str, operand);
	if (operand->error)
	  return str;

	str = skip_whitespace (str);

	if (*str != '(')
	  {
	    operand->code = 067;
	    operand->additional = 1;
	    operand->word = 0;
	    operand->reloc.type = BFD_RELOC_16_PCREL;
	    operand->reloc.pc_rel = 1;
	    break;
	  }

	/* d(rn) */
	str++;
	str = parse_reg (str, operand);
	if (operand->error)
	  return str;

	str = skip_whitespace (str);

	if (*str != ')')
	  {
	    operand->error = _("Missing ')'");
	    return str;
	  }

	str++;
	operand->additional = true;
	operand->code |= 060;
	switch (operand->reloc.exp.X_op)
	  {
	  case O_symbol:
	    operand->reloc.type = BFD_RELOC_16;
	    operand->reloc.pc_rel = 0;
	    break;
	  case O_constant:
	    if ((operand->code & 7) == 7)
	      {
		operand->reloc.pc_rel = 1;
		operand->word = operand->reloc.exp.X_add_number;
	      }
	    else
	      operand->word = operand->reloc.exp.X_add_number;

	    break;
	  default:
	    BAD_CASE (operand->reloc.exp.X_op);
	  }
	break;
      }
    }

  return str;
}

static char *
parse_op_noreg (char *str, struct pdp11_code *operand)
{
  str = skip_whitespace (str);
  operand->error = NULL;

  if (*str == '@' || *str == '*')
    {
      /* @(Rn) == @0(Rn): Mode 7, Indexed deferred.
	 Check for auto-increment deferred.  */
      if (str[1] == '('
	  && str[2] != 0
	  && str[3] != 0
	  && str[4] != 0
	  && str[5] != '+')
        {
	  /* Change implied to explicit index deferred.  */
          *str = '0';
          str = parse_op_no_deferred (str, operand);
        }
      else
        {
          /* @Rn == (Rn): Register deferred.  */
          str = parse_reg (str + 1, operand);
	  
          /* Not @Rn */
          if (operand->error)
	    {
	      operand->error = NULL;
	      str = parse_op_no_deferred (str, operand);
	    }
        }

      if (operand->error)
	return str;

      operand->code |= 010;
    }
  else
    str = parse_op_no_deferred (str, operand);

  return str;
}

static char *
parse_op (char *str, struct pdp11_code *operand)
{
  str = skip_whitespace (str);

  str = parse_reg (str, operand);
  if (!operand->error)
    return str;

  operand->error = NULL;
  parse_ac5 (str, operand);
  if (!operand->error)
    {
      operand->error = _("Float AC not legal as integer operand");
      return str;
    }

  return parse_op_noreg (str, operand);
}

static char *
parse_fop (char *str, struct pdp11_code *operand)
{
  str = skip_whitespace (str);

  str = parse_ac5 (str, operand);
  if (!operand->error)
    return str;

  operand->error = NULL;
  parse_reg (str, operand);
  if (!operand->error)
    {
      operand->error = _("General register not legal as float operand");
      return str;
    }

  return parse_op_noreg (str, operand);
}

static char *
parse_separator (char *str, int *error)
{
  str = skip_whitespace (str);
  *error = (*str != ',');
  if (!*error)
    str++;
  return str;
}

void
md_assemble (char *instruction_string)
{
  const struct pdp11_opcode *op;
  struct pdp11_code insn, op1, op2;
  int error;
  int size;
  const char *err = NULL;
  char *str;
  char *p;
  char c;

  str = skip_whitespace (instruction_string);
  p = find_whitespace (str);
  if (p - str == 0)
    {
      as_bad (_("No instruction found"));
      return;
    }

  c = *p;
  *p = '\0';
  op = (struct pdp11_opcode *)str_hash_find (insn_hash, str);
  *p = c;
  if (op == 0)
    {
      as_bad (_("Unknown instruction '%s'"), str);
      return;
    }

  if (!pdp11_extension[op->extension])
    {
      as_warn (_("Unsupported instruction set extension: %s"), op->name);
      return;
    }

  insn.error = NULL;
  insn.code = op->opcode;
  insn.reloc.type = BFD_RELOC_NONE;
  op1.error = NULL;
  op1.additional = false;
  op1.reloc.type = BFD_RELOC_NONE;
  op2.error = NULL;
  op2.additional = false;
  op2.reloc.type = BFD_RELOC_NONE;

  str = p;
  size = 2;

  switch (op->type)
    {
    case PDP11_OPCODE_NO_OPS:
      str = skip_whitespace (str);
      break;

    case PDP11_OPCODE_IMM3:
    case PDP11_OPCODE_IMM6:
    case PDP11_OPCODE_IMM8:
      str = skip_whitespace (str);
      if (*str == '#' || *str == '$')
	str++;
      str = parse_expression (str, &op1);
      if (op1.error)
	break;
      if (op1.reloc.exp.X_op != O_constant || op1.reloc.type != BFD_RELOC_NONE)
	{
	  op1.error = _("operand is not an absolute constant");
	  break;
	}
      switch (op->type)
	{
	case PDP11_OPCODE_IMM3:
	  if (op1.reloc.exp.X_add_number & ~7)
	    {
	      op1.error = _("3-bit immediate out of range");
	      break;
	    }
	  break;
	case PDP11_OPCODE_IMM6:
	  if (op1.reloc.exp.X_add_number & ~0x3f)
	    {
	      op1.error = _("6-bit immediate out of range");
	      break;
	    }
	  break;
	case PDP11_OPCODE_IMM8:
	  if (op1.reloc.exp.X_add_number & ~0xff)
	    {
	      op1.error = _("8-bit immediate out of range");
	      break;
	    }
	  break;
	}
      insn.code |= op1.reloc.exp.X_add_number;
      break;

    case PDP11_OPCODE_DISPL:
      {
	char *new_pointer;
	new_pointer = parse_expression (str, &op1);
	op1.code = 0;
	op1.reloc.pc_rel = 1;
	op1.reloc.type = BFD_RELOC_PDP11_DISP_8_PCREL;
	if (op1.reloc.exp.X_op != O_symbol)
	  {
	    op1.error = _("Symbol expected");
	    break;
	  }
	if (op1.code & ~0xff)
	  {
	    err = _("8-bit displacement out of range");
	    break;
	  }
	str = new_pointer;
	insn.code |= op1.code;
	insn.reloc = op1.reloc;
      }
      break;

    case PDP11_OPCODE_REG:
      str = parse_reg (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      break;

    case PDP11_OPCODE_OP:
      str = parse_op (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      if (op1.additional)
	size += 2;
      break;

    case PDP11_OPCODE_FOP:
      str = parse_fop (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      if (op1.additional)
	size += 2;
      break;

    case PDP11_OPCODE_REG_OP:
      str = parse_reg (str, &op2);
      if (op2.error)
	break;
      insn.code |= op2.code << 6;
      str = parse_separator (str, &error);
      if (error)
	{
	  op2.error = _("Missing ','");
	  break;
	}
      str = parse_op (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      if (op1.additional)
	size += 2;
      break;

    case PDP11_OPCODE_REG_OP_REV:
      str = parse_op (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      if (op1.additional)
	size += 2;
      str = parse_separator (str, &error);
      if (error)
	{
	  op2.error = _("Missing ','");
	  break;
	}
      str = parse_reg (str, &op2);
      if (op2.error)
	break;
      insn.code |= op2.code << 6;
      break;

    case PDP11_OPCODE_AC_FOP:
      str = parse_ac (str, &op2);
      if (op2.error)
	break;
      insn.code |= op2.code << 6;
      str = parse_separator (str, &error);
      if (error)
	{
	  op1.error = _("Missing ','");
	  break;
	}
      str = parse_fop (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      if (op1.additional)
	size += 2;
      break;

    case PDP11_OPCODE_FOP_AC:
      str = parse_fop (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      if (op1.additional)
	size += 2;
      str = parse_separator (str, &error);
      if (error)
	{
	  op1.error = _("Missing ','");
	  break;
	}
      str = parse_ac (str, &op2);
      if (op2.error)
	break;
      insn.code |= op2.code << 6;
      break;

    case PDP11_OPCODE_AC_OP:
      str = parse_ac (str, &op2);
      if (op2.error)
	break;
      insn.code |= op2.code << 6;
      str = parse_separator (str, &error);
      if (error)
	{
	  op1.error = _("Missing ','");
	  break;
	}
      str = parse_op (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      if (op1.additional)
	size += 2;
      break;

    case PDP11_OPCODE_OP_AC:
      str = parse_op (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code;
      if (op1.additional)
	size += 2;
      str = parse_separator (str, &error);
      if (error)
	{
	  op1.error = _("Missing ','");
	  break;
	}
      str = parse_ac (str, &op2);
      if (op2.error)
	break;
      insn.code |= op2.code << 6;
      break;

    case PDP11_OPCODE_OP_OP:
      str = parse_op (str, &op1);
      if (op1.error)
	break;
      insn.code |= op1.code << 6;
      if (op1.additional)
	size += 2;
      str = parse_separator (str, &error);
      if (error)
	{
	  op2.error = _("Missing ','");
	  break;
	}
      str = parse_op (str, &op2);
      if (op2.error)
	break;
      insn.code |= op2.code;
      if (op2.additional)
	size += 2;
      break;

    case PDP11_OPCODE_REG_DISPL:
      {
	char *new_pointer;
	str = parse_reg (str, &op2);
	if (op2.error)
	  break;
	insn.code |= op2.code << 6;
	str = parse_separator (str, &error);
	if (error)
	  {
	    op1.error = _("Missing ','");
	    break;
	  }
	new_pointer = parse_expression (str, &op1);
	op1.code = 0;
	op1.reloc.pc_rel = 1;
	op1.reloc.type = BFD_RELOC_PDP11_DISP_6_PCREL;
	if (op1.reloc.exp.X_op != O_symbol)
	  {
	    op1.error = _("Symbol expected");
	    break;
	  }
	if (op1.code & ~0x3f)
	  {
	    err = _("6-bit displacement out of range");
	    break;
	  }
	str = new_pointer;
	insn.code |= op1.code;
	insn.reloc = op1.reloc;
      }
      break;

    default:
      BAD_CASE (op->type);
    }

  if (op1.error)
    err = op1.error;
  else if (op2.error)
    err = op2.error;
  else
    {
      str = skip_whitespace (str);
      if (*str)
	err = _("Too many operands");
    }

  {
    char *to = NULL;

    if (err)
      {
	as_bad ("%s", err);
	return;
      }

    to = frag_more (size);

    md_number_to_chars (to, insn.code, 2);
    if (insn.reloc.type != BFD_RELOC_NONE)
      fix_new_exp (frag_now, to - frag_now->fr_literal, 2,
		   &insn.reloc.exp, insn.reloc.pc_rel, insn.reloc.type);
    to += 2;

    if (op1.additional)
      {
	md_number_to_chars (to, op1.word, 2);
	if (op1.reloc.type != BFD_RELOC_NONE)
	  fix_new_exp (frag_now, to - frag_now->fr_literal, 2,
		       &op1.reloc.exp, op1.reloc.pc_rel, op1.reloc.type);
	to += 2;
      }

    if (op2.additional)
      {
	md_number_to_chars (to, op2.word, 2);
	if (op2.reloc.type != BFD_RELOC_NONE)
	  fix_new_exp (frag_now, to - frag_now->fr_literal, 2,
		       &op2.reloc.exp, op2.reloc.pc_rel, op2.reloc.type);
      }
  }
}

int
md_estimate_size_before_relax (fragS *fragP ATTRIBUTE_UNUSED,
			       segT segment ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_convert_frag (bfd *headers ATTRIBUTE_UNUSED,
		 segT seg ATTRIBUTE_UNUSED,
		 fragS *fragP ATTRIBUTE_UNUSED)
{
}

int md_short_jump_size = 2;
int md_long_jump_size = 4;

void
md_create_short_jump (char *ptr ATTRIBUTE_UNUSED,
		      addressT from_addr ATTRIBUTE_UNUSED,
		      addressT to_addr ATTRIBUTE_UNUSED,
		      fragS *frag ATTRIBUTE_UNUSED,
		      symbolS *to_symbol ATTRIBUTE_UNUSED)
{
}

void
md_create_long_jump (char *ptr ATTRIBUTE_UNUSED,
		     addressT from_addr ATTRIBUTE_UNUSED,
		     addressT to_addr ATTRIBUTE_UNUSED,
		     fragS *frag ATTRIBUTE_UNUSED,
		     symbolS *to_symbol ATTRIBUTE_UNUSED)
{
}

static int
set_cpu_model (const char *arg)
{
  char buf[4];
  char *model = buf;

  if (arg[0] == 'k')
    arg++;

  *model++ = *arg++;

  if (strchr ("abdx", model[-1]) == NULL)
    return 0;

  if (model[-1] == 'd')
    {
      if (arg[0] == 'f' || arg[0] == 'j')
	model[-1] = *arg++;
    }
  else if (model[-1] == 'x')
    {
      if (arg[0] == 't')
	model[-1] = *arg++;
    }

  if (arg[0] == '-')
    arg++;

  if (!startswith (arg, "11"))
    return 0;
  arg += 2;

  if (arg[0] == '-')
    {
      if (*++arg == 0)
	return 0;
    }

  /* Allow up to two revision letters.  */
  if (arg[0] != 0)
    *model++ = *arg++;
  if (arg[0] != 0)
    *model++ = *arg++;

  *model++ = 0;

  set_option ("no-extensions");

  /* KA11 (11/15/20).  */
  if (startswith (buf, "a"))
    return 1; /* No extensions.  */

  /* KB11 (11/45/50/55/70).  */
  else if (startswith (buf, "b"))
    return set_option ("eis") && set_option ("spl");

  /* KD11-A (11/35/40).  */
  else if (startswith (buf, "da"))
    return set_option ("limited-eis");

  /* KD11-B (11/05/10).  */
  else if (startswith (buf, "db")
	   /* KD11-D (11/04).  */
	   || startswith (buf, "dd"))
    return 1; /* no extensions */

  /* KD11-E (11/34).  */
  else if (startswith (buf, "de"))
    return set_option ("eis") && set_option ("mxps");

  /* KD11-F (11/03).  */
  else if (startswith (buf, "df")
	   /* KD11-H (11/03).  */
	   || startswith (buf, "dh")
	   /* KD11-Q (11/03).  */
	   || startswith (buf, "dq"))
    return set_option ("limited-eis") && set_option ("mxps");

  /* KD11-K (11/60).  */
  else if (startswith (buf, "dk"))
    return set_option ("eis")
      && set_option ("mxps")
      && set_option ("ucode");

  /* KD11-Z (11/44).  */
  else if (startswith (buf, "dz"))
    return set_option ("csm")
      && set_option ("eis")
      && set_option ("mfpt")
      && set_option ("mxps")
      && set_option ("spl");

  /* F11 (11/23/24).  */
  else if (startswith (buf, "f"))
    return set_option ("eis")
      && set_option ("mfpt")
      && set_option ("mxps");

  /* J11 (11/53/73/83/84/93/94).  */
  else if (startswith (buf, "j"))
    return set_option ("csm")
      && set_option ("eis")
      && set_option ("mfpt")
      && set_option ("multiproc")
      && set_option ("mxps")
      && set_option ("spl");

  /* T11 (11/21).  */
  else if (startswith (buf, "t"))
    return set_option ("limited-eis")
      && set_option ("mxps");

  else
    return 0;
}

static int
set_machine_model (const char *arg)
{
  if (!startswith (arg, "pdp-11/")
      && !startswith (arg, "pdp11/")
      && !startswith (arg, "11/"))
    return 0;

  if (startswith (arg, "pdp"))
    arg += 3;
  if (arg[0] == '-')
    arg++;
  if (startswith (arg, "11/"))
    arg += 3;

  if (strcmp (arg, "03") == 0)
    return set_cpu_model ("kd11f");

  else if (strcmp (arg, "04") == 0)
    return set_cpu_model ("kd11d");

  else if (strcmp (arg, "05") == 0
	   || strcmp (arg, "10") == 0)
    return set_cpu_model ("kd11b");

  else if (strcmp (arg, "15") == 0
	   || strcmp (arg, "20") == 0)
    return set_cpu_model ("ka11");

  else if (strcmp (arg, "21") == 0)
    return set_cpu_model ("t11");

  else if (strcmp (arg, "23") == 0
	   || strcmp (arg, "24") == 0)
    return set_cpu_model ("f11");

  else if (strcmp (arg, "34") == 0
	   || strcmp (arg, "34a") == 0)
    return set_cpu_model ("kd11e");

  else if (strcmp (arg, "35") == 0
	   || strcmp (arg, "40") == 0)
    return set_cpu_model ("kd11da");

  else if (strcmp (arg, "44") == 0)
    return set_cpu_model ("kd11dz");

  else if (strcmp (arg, "45") == 0
	   || strcmp (arg, "50") == 0
	   || strcmp (arg, "55") == 0
	   || strcmp (arg, "70") == 0)
    return set_cpu_model ("kb11");

  else if (strcmp (arg, "60") == 0)
    return set_cpu_model ("kd11k");

  else if (strcmp (arg, "53") == 0
	   || strcmp (arg, "73") == 0
	   || strcmp (arg, "83") == 0
	   || strcmp (arg, "84") == 0
	   || strcmp (arg, "93") == 0
	   || strcmp (arg, "94") == 0)
    return set_cpu_model ("j11")
      && set_option ("fpp");

  else
    return 0;
}

const char *md_shortopts = "m:";

struct option md_longopts[] =
{
#define OPTION_CPU 257
  { "cpu", required_argument, NULL, OPTION_CPU },
#define OPTION_MACHINE 258
  { "machine", required_argument, NULL, OPTION_MACHINE },
#define OPTION_PIC 259
  { "pic", no_argument, NULL, OPTION_PIC },
  { NULL, no_argument, NULL, 0 }
};

size_t md_longopts_size = sizeof (md_longopts);

/* Invocation line includes a switch not recognized by the base assembler.
   See if it's a processor-specific option.  */

int
md_parse_option (int c, const char *arg)
{
  init_defaults ();

  switch (c)
    {
    case 'm':
      if (set_option (arg))
	return 1;
      if (set_cpu_model (arg))
	return 1;
      if (set_machine_model (arg))
	return 1;
      break;

    case OPTION_CPU:
      if (set_cpu_model (arg))
	return 1;
      break;

    case OPTION_MACHINE:
      if (set_machine_model (arg))
	return 1;
      break;

    case OPTION_PIC:
      if (set_option ("pic"))
	return 1;
      break;

    default:
      break;
    }

  return 0;
}

void
md_show_usage (FILE *stream)
{
  fprintf (stream, "\
\n\
PDP-11 instruction set extensions:\n\
\n\
-m(no-)cis		allow (disallow) commercial instruction set\n\
-m(no-)csm		allow (disallow) CSM instruction\n\
-m(no-)eis		allow (disallow) full extended instruction set\n\
-m(no-)fis		allow (disallow) KEV11 floating-point instructions\n\
-m(no-)fpp		allow (disallow) FP-11 floating-point instructions\n\
-m(no-)fpu		allow (disallow) FP-11 floating-point instructions\n\
-m(no-)limited-eis	allow (disallow) limited extended instruction set\n\
-m(no-)mfpt		allow (disallow) processor type instruction\n\
-m(no-)multiproc	allow (disallow) multiprocessor instructions\n\
-m(no-)mxps		allow (disallow) processor status instructions\n\
-m(no-)spl		allow (disallow) SPL instruction\n\
-m(no-)ucode		allow (disallow) microcode instructions\n\
-mall-extensions	allow all instruction set extensions\n\
			(this is the default)\n\
-mno-extensions		disallow all instruction set extensions\n\
-pic			generate position-independent code\n\
\n\
PDP-11 CPU model options:\n\
\n\
-mka11*			KA11 CPU.  base line instruction set only\n\
-mkb11*			KB11 CPU.  enable full EIS and SPL\n\
-mkd11a*		KD11-A CPU.  enable limited EIS\n\
-mkd11b*		KD11-B CPU.  base line instruction set only\n\
-mkd11d*		KD11-D CPU.  base line instruction set only\n\
-mkd11e*		KD11-E CPU.  enable full EIS, MTPS, and MFPS\n\
-mkd11f*		KD11-F CPU.  enable limited EIS, MTPS, and MFPS\n\
-mkd11h*		KD11-H CPU.  enable limited EIS, MTPS, and MFPS\n\
-mkd11q*		KD11-Q CPU.  enable limited EIS, MTPS, and MFPS\n\
-mkd11k*		KD11-K CPU.  enable full EIS, MTPS, MFPS, LDUB, MED,\n\
			XFC, and MFPT\n\
-mkd11z*		KD11-Z CPU.  enable full EIS, MTPS, MFPS, MFPT, SPL,\n\
			and CSM\n\
-mf11*			F11 CPU.  enable full EIS, MFPS, MTPS, and MFPT\n\
-mj11*			J11 CPU.  enable full EIS, MTPS, MFPS, MFPT, SPL,\n\
			CSM, TSTSET, and WRTLCK\n\
-mt11*			T11 CPU.  enable limited EIS, MTPS, and MFPS\n\
\n\
PDP-11 machine model options:\n\
\n\
-m11/03			same as -mkd11f\n\
-m11/04			same as -mkd11d\n\
-m11/05			same as -mkd11b\n\
-m11/10			same as -mkd11b\n\
-m11/15			same as -mka11\n\
-m11/20			same as -mka11\n\
-m11/21			same as -mt11\n\
-m11/23			same as -mf11\n\
-m11/24			same as -mf11\n\
-m11/34			same as -mkd11e\n\
-m11/34a		same as -mkd11e -mfpp\n\
-m11/35			same as -mkd11a\n\
-m11/40			same as -mkd11a\n\
-m11/44			same as -mkd11z\n\
-m11/45			same as -mkb11\n\
-m11/50			same as -mkb11\n\
-m11/53			same as -mj11\n\
-m11/55			same as -mkb11\n\
-m11/60			same as -mkd11k\n\
-m11/70			same as -mkb11\n\
-m11/73			same as -mj11\n\
-m11/83			same as -mj11\n\
-m11/84			same as -mj11\n\
-m11/93			same as -mj11\n\
-m11/94			same as -mj11\n\
");
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED,
		  valueT size)
{
  return (size + 1) & ~1;
}

long
md_pcrel_from (fixS *fixP)
{
  return fixP->fx_frag->fr_address + fixP->fx_where + fixP->fx_size;
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED,
	      fixS *fixp)
{
  arelent *reloc;
  bfd_reloc_code_real_type code;

  reloc = XNEW (arelent);

  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  /* This is taken account for in md_apply_fix().  */
  reloc->addend = -symbol_get_bfdsym (fixp->fx_addsy)->section->vma;

  code = fixp->fx_r_type;
  if (fixp->fx_pcrel)
    {
      switch (code)
	{
	case BFD_RELOC_16:
	  code = BFD_RELOC_16_PCREL;
	  break;

	case BFD_RELOC_16_PCREL:
	  break;

	default:
	  BAD_CASE (code);
	  return NULL;
	}
    }

  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);

  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("Can not represent %s relocation in this object file format"),
		    bfd_get_reloc_code_name (code));
      return NULL;
    }

  return reloc;
}

void
pseudo_bss (int c ATTRIBUTE_UNUSED)
{
  int temp;

  temp = get_absolute_expression ();
  subseg_set (bss_section, temp);
  demand_empty_rest_of_line ();
}

void
pseudo_even (int c ATTRIBUTE_UNUSED)
{
  int alignment = 1; /* 2^1 */
  frag_align (alignment, 0, 1);
  record_alignment (now_seg, alignment);
}

const char *
md_atof (int type, char * litP, int * sizeP)
{
  return vax_md_atof (type, litP, sizeP);
}
