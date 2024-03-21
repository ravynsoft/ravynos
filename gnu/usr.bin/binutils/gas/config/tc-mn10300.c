/* tc-mn10300.c -- Assembler code for the Matsushita 10300
   Copyright (C) 1996-2023 Free Software Foundation, Inc.

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
#include "opcode/mn10300.h"
#include "dwarf2dbg.h"
#include "libiberty.h"

/* Structure to hold information about predefined registers.  */
struct reg_name
{
  const char *name;
  int value;
};

/* Generic assembler global variables which must be defined by all
   targets.  */

/* Characters which always start a comment.  */
const char comment_chars[] = "#";

/* Characters which start a comment at the beginning of a line.  */
const char line_comment_chars[] = ";#";

/* Characters which may be used to separate multiple commands on a
   single line.  */
const char line_separator_chars[] = ";";

/* Characters which are used to indicate an exponent in a floating
   point number.  */
const char EXP_CHARS[] = "eE";

/* Characters which mean that a number is a floating point constant,
   as in 0d1.0.  */
const char FLT_CHARS[] = "dD";

const relax_typeS md_relax_table[] =
{
  /* The plus values for the bCC and fBCC instructions in the table below
     are because the branch instruction is translated into a jump
     instruction that is now +2 or +3 bytes further on in memory, and the
     correct size of jump instruction must be selected.  */
  /* bCC relaxing.  */
  {0x7f, -0x80, 2, 1},
  {0x7fff + 2, -0x8000 + 2, 5, 2},
  {0x7fffffff, -0x80000000, 7, 0},

  /* bCC relaxing (uncommon cases for 3byte length instructions)  */
  {0x7f, -0x80, 3, 4},
  {0x7fff + 3, -0x8000 + 3, 6, 5},
  {0x7fffffff, -0x80000000, 8, 0},

  /* call relaxing.  */
  {0x7fff, -0x8000, 5, 7},
  {0x7fffffff, -0x80000000, 7, 0},

  /* calls relaxing.  */
  {0x7fff, -0x8000, 4, 9},
  {0x7fffffff, -0x80000000, 6, 0},

  /* jmp relaxing.  */
  {0x7f, -0x80, 2, 11},
  {0x7fff, -0x8000, 3, 12},
  {0x7fffffff, -0x80000000, 5, 0},

  /* fbCC relaxing.  */
  {0x7f, -0x80, 3, 14},
  {0x7fff + 3, -0x8000 + 3, 6, 15},
  {0x7fffffff, -0x80000000, 8, 0},

};

static int current_machine;

/* Fixups.  */
#define MAX_INSN_FIXUPS 5

struct mn10300_fixup
{
  expressionS exp;
  int opindex;
  bfd_reloc_code_real_type reloc;
};
struct mn10300_fixup fixups[MAX_INSN_FIXUPS];
static int fc;

/* We must store the value of each register operand so that we can
   verify that certain registers do not match.  */
int mn10300_reg_operands[MN10300_MAX_OPERANDS];

const char *md_shortopts = "";

struct option md_longopts[] =
{
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

#define HAVE_AM33_2 (current_machine == AM33_2)
#define HAVE_AM33   (current_machine == AM33 || HAVE_AM33_2)
#define HAVE_AM30   (current_machine == AM30)

/* Opcode hash table.  */
static htab_t mn10300_hash;

/* This table is sorted. Suitable for searching by a binary search.  */
static const struct reg_name data_registers[] =
{
  { "d0", 0 },
  { "d1", 1 },
  { "d2", 2 },
  { "d3", 3 },
};

static const struct reg_name address_registers[] =
{
  { "a0", 0 },
  { "a1", 1 },
  { "a2", 2 },
  { "a3", 3 },
};

static const struct reg_name r_registers[] =
{
  { "a0", 8 },
  { "a1", 9 },
  { "a2", 10 },
  { "a3", 11 },
  { "d0", 12 },
  { "d1", 13 },
  { "d2", 14 },
  { "d3", 15 },
  { "e0", 0 },
  { "e1", 1 },
  { "e10", 10 },
  { "e11", 11 },
  { "e12", 12 },
  { "e13", 13 },
  { "e14", 14 },
  { "e15", 15 },
  { "e2", 2 },
  { "e3", 3 },
  { "e4", 4 },
  { "e5", 5 },
  { "e6", 6 },
  { "e7", 7 },
  { "e8", 8 },
  { "e9", 9 },
  { "r0", 0 },
  { "r1", 1 },
  { "r10", 10 },
  { "r11", 11 },
  { "r12", 12 },
  { "r13", 13 },
  { "r14", 14 },
  { "r15", 15 },
  { "r2", 2 },
  { "r3", 3 },
  { "r4", 4 },
  { "r5", 5 },
  { "r6", 6 },
  { "r7", 7 },
  { "r8", 8 },
  { "r9", 9 },
};

static const struct reg_name xr_registers[] =
{
  { "mcrh", 2 },
  { "mcrl", 3 },
  { "mcvf", 4 },
  { "mdrq", 1 },
  { "sp", 0 },
  { "xr0", 0 },
  { "xr1", 1 },
  { "xr10", 10 },
  { "xr11", 11 },
  { "xr12", 12 },
  { "xr13", 13 },
  { "xr14", 14 },
  { "xr15", 15 },
  { "xr2", 2 },
  { "xr3", 3 },
  { "xr4", 4 },
  { "xr5", 5 },
  { "xr6", 6 },
  { "xr7", 7 },
  { "xr8", 8 },
  { "xr9", 9 },
};

static const struct reg_name float_registers[] =
{
  { "fs0", 0 },
  { "fs1", 1 },
  { "fs10", 10 },
  { "fs11", 11 },
  { "fs12", 12 },
  { "fs13", 13 },
  { "fs14", 14 },
  { "fs15", 15 },
  { "fs16", 16 },
  { "fs17", 17 },
  { "fs18", 18 },
  { "fs19", 19 },
  { "fs2",   2 },
  { "fs20", 20 },
  { "fs21", 21 },
  { "fs22", 22 },
  { "fs23", 23 },
  { "fs24", 24 },
  { "fs25", 25 },
  { "fs26", 26 },
  { "fs27", 27 },
  { "fs28", 28 },
  { "fs29", 29 },
  { "fs3",   3 },
  { "fs30", 30 },
  { "fs31", 31 },
  { "fs4",   4 },
  { "fs5",   5 },
  { "fs6",   6 },
  { "fs7",   7 },
  { "fs8",   8 },
  { "fs9",   9 },
};

static const struct reg_name double_registers[] =
{
  { "fd0",   0 },
  { "fd10", 10 },
  { "fd12", 12 },
  { "fd14", 14 },
  { "fd16", 16 },
  { "fd18", 18 },
  { "fd2",   2 },
  { "fd20", 20 },
  { "fd22", 22 },
  { "fd24", 24 },
  { "fd26", 26 },
  { "fd28", 28 },
  { "fd30", 30 },
  { "fd4",   4 },
  { "fd6",   6 },
  { "fd8",   8 },
};

/* We abuse the `value' field, that would be otherwise unused, to
   encode the architecture on which (access to) the register was
   introduced.  FIXME: we should probably warn when we encounter a
   register name when assembling for an architecture that doesn't
   support it, before parsing it as a symbol name.  */
static const struct reg_name other_registers[] =
{
  { "epsw", AM33 },
  { "mdr", 0 },
  { "pc", AM33 },
  { "psw", 0 },
  { "sp", 0 },
  { "ssp", 0 },
  { "usp", 0 },
};

#define OTHER_REG_NAME_CNT	ARRAY_SIZE (other_registers)

/* Perform a binary search of the given register table REGS to see
   if NAME is a valid register name.  Returns the register number from
   the array on success, or -1 on failure.  */

static int
reg_name_search (const struct reg_name *regs,
		 int regcount,
		 const char *name)
{
  int low, high;

  low = 0;
  high = regcount - 1;

  do
    {
      int cmp, middle;

      middle = (low + high) / 2;
      cmp = strcasecmp (name, regs[middle].name);
      if (cmp < 0)
	high = middle - 1;
      else if (cmp > 0)
	low = middle + 1;
      else
	return regs[middle].value;
    }
  while (low <= high);

  return -1;
}

/* Looks at the current position in the input line to see if it is
   the name of a register in TABLE.  If it is, then the name is
   converted into an expression returned in EXPRESSIONP (with X_op
   set to O_register and X_add_number set to the register number), the
   input pointer is left pointing at the first non-blank character after
   the name and the function returns TRUE.  Otherwise the input pointer
   is left alone and the function returns FALSE.  */

static bool
get_register_name (expressionS *           expressionP,
		   const struct reg_name * table,
		   size_t                  table_length)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;

  c = get_symbol_name (&name);
  reg_number = reg_name_search (table, table_length, name);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  /* Look to see if it's in the register table.  */
  if (reg_number >= 0)
    {
      expressionP->X_op = O_register;
      expressionP->X_add_number = reg_number;

      /* Make the rest nice.  */
      expressionP->X_add_symbol = NULL;
      expressionP->X_op_symbol = NULL;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;
  return false;
}

static bool
r_register_name (expressionS *expressionP)
{
  return get_register_name (expressionP, r_registers, ARRAY_SIZE (r_registers));
}


static bool
xr_register_name (expressionS *expressionP)
{
  return get_register_name (expressionP, xr_registers, ARRAY_SIZE (xr_registers));
}

static bool
data_register_name (expressionS *expressionP)
{
  return get_register_name (expressionP, data_registers, ARRAY_SIZE (data_registers));
}

static bool
address_register_name (expressionS *expressionP)
{
  return get_register_name (expressionP, address_registers, ARRAY_SIZE (address_registers));
}

static bool
float_register_name (expressionS *expressionP)
{
  return get_register_name (expressionP, float_registers, ARRAY_SIZE (float_registers));
}

static bool
double_register_name (expressionS *expressionP)
{
  return get_register_name (expressionP, double_registers, ARRAY_SIZE (double_registers));
}

static bool
other_register_name (expressionS *expressionP)
{
  int reg_number;
  char *name;
  char *start;
  char c;

  /* Find the spelling of the operand.  */
  start = input_line_pointer;

  c = get_symbol_name (&name);
  reg_number = reg_name_search (other_registers, ARRAY_SIZE (other_registers), name);

  /* Put back the delimiting char.  */
  (void) restore_line_pointer (c);

  /* Look to see if it's in the register table.  */
  if (reg_number == 0
      || (reg_number == AM33 && HAVE_AM33))
    {
      expressionP->X_op = O_register;
      expressionP->X_add_number = 0;

      /* Make the rest nice.  */
      expressionP->X_add_symbol = NULL;
      expressionP->X_op_symbol = NULL;

      return true;
    }

  /* Reset the line as if we had not done anything.  */
  input_line_pointer = start;
  return false;
}

void
md_show_usage (FILE *stream)
{
  fprintf (stream, _("MN10300 assembler options:\n\
none yet\n"));
}

int
md_parse_option (int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  return 0;
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

const char *
md_atof (int type, char *litp, int *sizep)
{
  return ieee_md_atof (type, litp, sizep, false);
}

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED,
		 asection *sec,
		 fragS *fragP)
{
  static unsigned long label_count = 0;
  char buf[40];

  subseg_change (sec, 0);
  if (fragP->fr_subtype == 0)
    {
      fix_new (fragP, fragP->fr_fix + 1, 1, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_8_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 2;
    }
  else if (fragP->fr_subtype == 1)
    {
      /* Reverse the condition of the first branch.  */
      int offset = fragP->fr_fix;
      int opcode = fragP->fr_literal[offset] & 0xff;

      switch (opcode)
	{
	case 0xc8:
	  opcode = 0xc9;
	  break;
	case 0xc9:
	  opcode = 0xc8;
	  break;
	case 0xc0:
	  opcode = 0xc2;
	  break;
	case 0xc2:
	  opcode = 0xc0;
	  break;
	case 0xc3:
	  opcode = 0xc1;
	  break;
	case 0xc1:
	  opcode = 0xc3;
	  break;
	case 0xc4:
	  opcode = 0xc6;
	  break;
	case 0xc6:
	  opcode = 0xc4;
	  break;
	case 0xc7:
	  opcode = 0xc5;
	  break;
	case 0xc5:
	  opcode = 0xc7;
	  break;
	default:
	  abort ();
	}
      fragP->fr_literal[offset] = opcode;

      /* Create a fixup for the reversed conditional branch.  */
      sprintf (buf, ".%s_%ld", FAKE_LABEL_NAME, label_count++);
      fix_new (fragP, fragP->fr_fix + 1, 1,
	       symbol_new (buf, sec, fragP->fr_next, 0),
	       fragP->fr_offset + 1, 1, BFD_RELOC_8_PCREL);

      /* Now create the unconditional branch + fixup to the
	 final target.  */
      fragP->fr_literal[offset + 2] = 0xcc;
      fix_new (fragP, fragP->fr_fix + 3, 2, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_16_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 5;
    }
  else if (fragP->fr_subtype == 2)
    {
      /* Reverse the condition of the first branch.  */
      int offset = fragP->fr_fix;
      int opcode = fragP->fr_literal[offset] & 0xff;

      switch (opcode)
	{
	case 0xc8:
	  opcode = 0xc9;
	  break;
	case 0xc9:
	  opcode = 0xc8;
	  break;
	case 0xc0:
	  opcode = 0xc2;
	  break;
	case 0xc2:
	  opcode = 0xc0;
	  break;
	case 0xc3:
	  opcode = 0xc1;
	  break;
	case 0xc1:
	  opcode = 0xc3;
	  break;
	case 0xc4:
	  opcode = 0xc6;
	  break;
	case 0xc6:
	  opcode = 0xc4;
	  break;
	case 0xc7:
	  opcode = 0xc5;
	  break;
	case 0xc5:
	  opcode = 0xc7;
	  break;
	default:
	  abort ();
	}
      fragP->fr_literal[offset] = opcode;

      /* Create a fixup for the reversed conditional branch.  */
      sprintf (buf, ".%s_%ld", FAKE_LABEL_NAME, label_count++);
      fix_new (fragP, fragP->fr_fix + 1, 1,
	       symbol_new (buf, sec, fragP->fr_next, 0),
	       fragP->fr_offset + 1, 1, BFD_RELOC_8_PCREL);

      /* Now create the unconditional branch + fixup to the
	 final target.  */
      fragP->fr_literal[offset + 2] = 0xdc;
      fix_new (fragP, fragP->fr_fix + 3, 4, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_32_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 7;
    }
  else if (fragP->fr_subtype == 3)
    {
      fix_new (fragP, fragP->fr_fix + 2, 1, fragP->fr_symbol,
	       fragP->fr_offset + 2, 1, BFD_RELOC_8_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 3;
    }
  else if (fragP->fr_subtype == 4)
    {
      /* Reverse the condition of the first branch.  */
      int offset = fragP->fr_fix;
      int opcode = fragP->fr_literal[offset + 1] & 0xff;

      switch (opcode)
	{
	case 0xe8:
	  opcode = 0xe9;
	  break;
	case 0xe9:
	  opcode = 0xe8;
	  break;
	case 0xea:
	  opcode = 0xeb;
	  break;
	case 0xeb:
	  opcode = 0xea;
	  break;
	default:
	  abort ();
	}
      fragP->fr_literal[offset + 1] = opcode;

      /* Create a fixup for the reversed conditional branch.  */
      sprintf (buf, ".%s_%ld", FAKE_LABEL_NAME, label_count++);
      fix_new (fragP, fragP->fr_fix + 2, 1,
	       symbol_new (buf, sec, fragP->fr_next, 0),
	       fragP->fr_offset + 2, 1, BFD_RELOC_8_PCREL);

      /* Now create the unconditional branch + fixup to the
	 final target.  */
      fragP->fr_literal[offset + 3] = 0xcc;
      fix_new (fragP, fragP->fr_fix + 4, 2, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_16_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 6;
    }
  else if (fragP->fr_subtype == 5)
    {
      /* Reverse the condition of the first branch.  */
      int offset = fragP->fr_fix;
      int opcode = fragP->fr_literal[offset + 1] & 0xff;

      switch (opcode)
	{
	case 0xe8:
	  opcode = 0xe9;
	  break;
	case 0xea:
	  opcode = 0xeb;
	  break;
	case 0xeb:
	  opcode = 0xea;
	  break;
	default:
	  abort ();
	}
      fragP->fr_literal[offset + 1] = opcode;

      /* Create a fixup for the reversed conditional branch.  */
      sprintf (buf, ".%s_%ld", FAKE_LABEL_NAME, label_count++);
      fix_new (fragP, fragP->fr_fix + 2, 1,
	       symbol_new (buf, sec, fragP->fr_next, 0),
	       fragP->fr_offset + 2, 1, BFD_RELOC_8_PCREL);

      /* Now create the unconditional branch + fixup to the
	 final target.  */
      fragP->fr_literal[offset + 3] = 0xdc;
      fix_new (fragP, fragP->fr_fix + 4, 4, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_32_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 8;
    }
  else if (fragP->fr_subtype == 6)
    {
      int offset = fragP->fr_fix;

      fragP->fr_literal[offset] = 0xcd;
      fix_new (fragP, fragP->fr_fix + 1, 2, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_16_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 5;
    }
  else if (fragP->fr_subtype == 7)
    {
      int offset = fragP->fr_fix;

      fragP->fr_literal[offset] = 0xdd;
      fragP->fr_literal[offset + 5] = fragP->fr_literal[offset + 3];
      fragP->fr_literal[offset + 6] = fragP->fr_literal[offset + 4];
      fragP->fr_literal[offset + 3] = 0;
      fragP->fr_literal[offset + 4] = 0;

      fix_new (fragP, fragP->fr_fix + 1, 4, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_32_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 7;
    }
  else if (fragP->fr_subtype == 8)
    {
      int offset = fragP->fr_fix;

      fragP->fr_literal[offset] = 0xfa;
      fragP->fr_literal[offset + 1] = 0xff;
      fix_new (fragP, fragP->fr_fix + 2, 2, fragP->fr_symbol,
	       fragP->fr_offset + 2, 1, BFD_RELOC_16_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 4;
    }
  else if (fragP->fr_subtype == 9)
    {
      int offset = fragP->fr_fix;

      fragP->fr_literal[offset] = 0xfc;
      fragP->fr_literal[offset + 1] = 0xff;

      fix_new (fragP, fragP->fr_fix + 2, 4, fragP->fr_symbol,
	       fragP->fr_offset + 2, 1, BFD_RELOC_32_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 6;
    }
  else if (fragP->fr_subtype == 10)
    {
      fragP->fr_literal[fragP->fr_fix] = 0xca;
      fix_new (fragP, fragP->fr_fix + 1, 1, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_8_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 2;
    }
  else if (fragP->fr_subtype == 11)
    {
      int offset = fragP->fr_fix;

      fragP->fr_literal[offset] = 0xcc;

      fix_new (fragP, fragP->fr_fix + 1, 2, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_16_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 3;
    }
  else if (fragP->fr_subtype == 12)
    {
      int offset = fragP->fr_fix;

      fragP->fr_literal[offset] = 0xdc;

      fix_new (fragP, fragP->fr_fix + 1, 4, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_32_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 5;
    }
  else if (fragP->fr_subtype == 13)
    {
      fix_new (fragP, fragP->fr_fix + 2, 1, fragP->fr_symbol,
	       fragP->fr_offset + 2, 1, BFD_RELOC_8_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 3;
    }
  else if (fragP->fr_subtype == 14)
    {
      /* Reverse the condition of the first branch.  */
      int offset = fragP->fr_fix;
      int opcode = fragP->fr_literal[offset + 1] & 0xff;

      switch (opcode)
	{
	case 0xd0:
	  opcode = 0xd1;
	  break;
	case 0xd1:
	  opcode = 0xd0;
	  break;
	case 0xd2:
	  opcode = 0xdc;
	  break;
	case 0xd3:
	  opcode = 0xdb;
	  break;
	case 0xd4:
	  opcode = 0xda;
	  break;
	case 0xd5:
	  opcode = 0xd9;
	  break;
	case 0xd6:
	  opcode = 0xd8;
	  break;
	case 0xd7:
	  opcode = 0xdd;
	  break;
	case 0xd8:
	  opcode = 0xd6;
	  break;
	case 0xd9:
	  opcode = 0xd5;
	  break;
	case 0xda:
	  opcode = 0xd4;
	  break;
	case 0xdb:
	  opcode = 0xd3;
	  break;
	case 0xdc:
	  opcode = 0xd2;
	  break;
	case 0xdd:
	  opcode = 0xd7;
	  break;
	default:
	  abort ();
	}
      fragP->fr_literal[offset + 1] = opcode;

      /* Create a fixup for the reversed conditional branch.  */
      sprintf (buf, ".%s_%ld", FAKE_LABEL_NAME, label_count++);
      fix_new (fragP, fragP->fr_fix + 2, 1,
	       symbol_new (buf, sec, fragP->fr_next, 0),
	       fragP->fr_offset + 2, 1, BFD_RELOC_8_PCREL);

      /* Now create the unconditional branch + fixup to the
	 final target.  */
      fragP->fr_literal[offset + 3] = 0xcc;
      fix_new (fragP, fragP->fr_fix + 4, 2, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_16_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 6;
    }
  else if (fragP->fr_subtype == 15)
    {
      /* Reverse the condition of the first branch.  */
      int offset = fragP->fr_fix;
      int opcode = fragP->fr_literal[offset + 1] & 0xff;

      switch (opcode)
	{
	case 0xd0:
	  opcode = 0xd1;
	  break;
	case 0xd1:
	  opcode = 0xd0;
	  break;
	case 0xd2:
	  opcode = 0xdc;
	  break;
	case 0xd3:
	  opcode = 0xdb;
	  break;
	case 0xd4:
	  opcode = 0xda;
	  break;
	case 0xd5:
	  opcode = 0xd9;
	  break;
	case 0xd6:
	  opcode = 0xd8;
	  break;
	case 0xd7:
	  opcode = 0xdd;
	  break;
	case 0xd8:
	  opcode = 0xd6;
	  break;
	case 0xd9:
	  opcode = 0xd5;
	  break;
	case 0xda:
	  opcode = 0xd4;
	  break;
	case 0xdb:
	  opcode = 0xd3;
	  break;
	case 0xdc:
	  opcode = 0xd2;
	  break;
	case 0xdd:
	  opcode = 0xd7;
	  break;
	default:
	  abort ();
	}
      fragP->fr_literal[offset + 1] = opcode;

      /* Create a fixup for the reversed conditional branch.  */
      sprintf (buf, ".%s_%ld", FAKE_LABEL_NAME, label_count++);
      fix_new (fragP, fragP->fr_fix + 2, 1,
	       symbol_new (buf, sec, fragP->fr_next, 0),
	       fragP->fr_offset + 2, 1, BFD_RELOC_8_PCREL);

      /* Now create the unconditional branch + fixup to the
	 final target.  */
      fragP->fr_literal[offset + 3] = 0xdc;
      fix_new (fragP, fragP->fr_fix + 4, 4, fragP->fr_symbol,
	       fragP->fr_offset + 1, 1, BFD_RELOC_32_PCREL);
      fragP->fr_var = 0;
      fragP->fr_fix += 8;
    }
  else
    abort ();
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
  const char *prev_name = "";
  const struct mn10300_opcode *op;

  mn10300_hash = str_htab_create ();

  /* Insert unique names into hash table.  The MN10300 instruction set
     has many identical opcode names that have different opcodes based
     on the operands.  This hash table then provides a quick index to
     the first opcode with a particular name in the opcode table.  */

  op = mn10300_opcodes;
  while (op->name)
    {
      if (strcmp (prev_name, op->name))
	{
	  prev_name = (char *) op->name;
	  str_hash_insert (mn10300_hash, op->name, op, 0);
	}
      op++;
    }

  /* Set the default machine type.  */
#ifdef TE_LINUX
  if (!bfd_set_arch_mach (stdoutput, bfd_arch_mn10300, AM33_2))
    as_warn (_("could not set architecture and machine"));

  current_machine = AM33_2;
#else
  if (!bfd_set_arch_mach (stdoutput, bfd_arch_mn10300, MN103))
    as_warn (_("could not set architecture and machine"));

  current_machine = MN103;
#endif

  /*  Set linkrelax here to avoid fixups in most sections.  */
  linkrelax = 1;
}

static symbolS *GOT_symbol;

static inline int
mn10300_PIC_related_p (symbolS *sym)
{
  expressionS *exp;

  if (! sym)
    return 0;

  if (sym == GOT_symbol)
    return 1;

  exp = symbol_get_value_expression (sym);

  return (exp->X_op == O_PIC_reloc
	  || mn10300_PIC_related_p (exp->X_add_symbol)
	  || mn10300_PIC_related_p (exp->X_op_symbol));
}

static inline int
mn10300_check_fixup (struct mn10300_fixup *fixup)
{
  expressionS *exp = &fixup->exp;

 repeat:
  switch (exp->X_op)
    {
    case O_add:
    case O_subtract: /* If we're sufficiently unlucky that the label
			and the expression that references it happen
			to end up in different frags, the subtract
			won't be simplified within expression().  */
      /* The PIC-related operand must be the first operand of a sum.  */
      if (exp != &fixup->exp || mn10300_PIC_related_p (exp->X_op_symbol))
	return 1;

      if (exp->X_add_symbol && exp->X_add_symbol == GOT_symbol)
	fixup->reloc = BFD_RELOC_32_GOT_PCREL;

      exp = symbol_get_value_expression (exp->X_add_symbol);
      goto repeat;

    case O_symbol:
      if (exp->X_add_symbol && exp->X_add_symbol == GOT_symbol)
	fixup->reloc = BFD_RELOC_32_GOT_PCREL;
      break;

    case O_PIC_reloc:
      fixup->reloc = exp->X_md;
      exp->X_op = O_symbol;
      if (fixup->reloc == BFD_RELOC_32_PLT_PCREL
	  && fixup->opindex >= 0
	  && (mn10300_operands[fixup->opindex].flags
	      & MN10300_OPERAND_RELAX))
	return 1;
      break;

    default:
      return (mn10300_PIC_related_p (exp->X_add_symbol)
	      || mn10300_PIC_related_p (exp->X_op_symbol));
    }

  return 0;
}

void
mn10300_cons_fix_new (fragS *frag, int off, int size, expressionS *exp,
		      bfd_reloc_code_real_type r ATTRIBUTE_UNUSED)
{
  struct mn10300_fixup fixup;

  fixup.opindex = -1;
  fixup.exp = *exp;
  fixup.reloc = BFD_RELOC_UNUSED;

  mn10300_check_fixup (&fixup);

  if (fixup.reloc == BFD_RELOC_MN10300_GOT32)
    switch (size)
      {
      case 2:
	fixup.reloc = BFD_RELOC_MN10300_GOT16;
	break;

      case 3:
	fixup.reloc = BFD_RELOC_MN10300_GOT24;
	break;

      case 4:
	break;

      default:
	goto error;
      }
  else if (fixup.reloc == BFD_RELOC_UNUSED)
    switch (size)
      {
      case 1:
	fixup.reloc = BFD_RELOC_8;
	break;

      case 2:
	fixup.reloc = BFD_RELOC_16;
	break;

      case 3:
	fixup.reloc = BFD_RELOC_24;
	break;

      case 4:
	fixup.reloc = BFD_RELOC_32;
	break;

      default:
	goto error;
      }
  else if (size != 4)
    {
    error:
      as_bad (_("unsupported BFD relocation size %u"), size);
      fixup.reloc = BFD_RELOC_UNUSED;
    }

  fix_new_exp (frag, off, size, &fixup.exp, 0, fixup.reloc);
}

static bool
check_operand (const struct mn10300_operand *operand,
	       offsetT val)
{
  /* No need to check 32bit operands for a bit.  Note that
     MN10300_OPERAND_SPLIT is an implicit 32bit operand.  */
  if (operand->bits != 32
      && (operand->flags & MN10300_OPERAND_SPLIT) == 0)
    {
      long min, max;
      offsetT test;
      int bits;

      bits = operand->bits;
      if (operand->flags & MN10300_OPERAND_24BIT)
	bits = 24;

      if ((operand->flags & MN10300_OPERAND_SIGNED) != 0)
	{
	  max = (1 << (bits - 1)) - 1;
	  min = - (1 << (bits - 1));
	}
      else
	{
	  max = (1 << bits) - 1;
	  min = 0;
	}

      test = val;

      if (test < (offsetT) min || test > (offsetT) max)
	return false;
    }
  return true;
}

/* Insert an operand value into an instruction.  */

static void
mn10300_insert_operand (unsigned long *insnp,
			unsigned long *extensionp,
			const struct mn10300_operand *operand,
			offsetT val,
			char *file,
			unsigned int line,
			unsigned int shift)
{
  /* No need to check 32bit operands for a bit.  Note that
     MN10300_OPERAND_SPLIT is an implicit 32bit operand.  */
  if (operand->bits != 32
      && (operand->flags & MN10300_OPERAND_SPLIT) == 0)
    {
      long min, max;
      offsetT test;
      int bits;

      bits = operand->bits;
      if (operand->flags & MN10300_OPERAND_24BIT)
	bits = 24;

      if ((operand->flags & MN10300_OPERAND_SIGNED) != 0)
	{
	  max = (1 << (bits - 1)) - 1;
	  min = - (1 << (bits - 1));
	}
      else
	{
	  max = (1 << bits) - 1;
	  min = 0;
	}

      test = val;

      if (test < (offsetT) min || test > (offsetT) max)
	as_warn_value_out_of_range (_("operand"), test, (offsetT) min, (offsetT) max, file, line);
    }

  if ((operand->flags & MN10300_OPERAND_SPLIT) != 0)
    {
      *insnp |= (val >> (32 - operand->bits)) & ((1 << operand->bits) - 1);
      *extensionp |= ((val & ((1 << (32 - operand->bits)) - 1))
		      << operand->shift);
    }
  else if ((operand->flags & MN10300_OPERAND_24BIT) != 0)
    {
      *insnp |= (val >> (24 - operand->bits)) & ((1 << operand->bits) - 1);
      *extensionp |= ((val & ((1 << (24 - operand->bits)) - 1))
		      << operand->shift);
    }
  else if ((operand->flags & (MN10300_OPERAND_FSREG | MN10300_OPERAND_FDREG)))
    {
      /* See devo/opcodes/m10300-opc.c just before #define FSM0 for an
         explanation of these variables.  Note that FMT-implied shifts
        are not taken into account for FP registers.  */
      unsigned long mask_low, mask_high;
      int shl_low, shr_high, shl_high;

      switch (operand->bits)
	{
	case 5:
	  /* Handle regular FP registers.  */
	  if (operand->shift >= 0)
	    {
	      /* This is an `m' register.  */
	      shl_low = operand->shift;
	      shl_high = 8 + (8 & shl_low) + (shl_low & 4) / 4;
	    }
	  else
	    {
	      /* This is an `n' register.  */
	      shl_low = -operand->shift;
	      shl_high = shl_low / 4;
	    }

	  mask_low = 0x0f;
	  mask_high = 0x10;
	  shr_high = 4;
	  break;

	case 3:
	  /* Handle accumulators.  */
	  shl_low = -operand->shift;
	  shl_high = 0;
	  mask_low = 0x03;
	  mask_high = 0x04;
	  shr_high = 2;
	  break;

	default:
	  abort ();
	}
      *insnp |= ((((val & mask_high) >> shr_high) << shl_high)
		 | ((val & mask_low) << shl_low));
    }
  else if ((operand->flags & MN10300_OPERAND_EXTENDED) == 0)
    {
      *insnp |= (((long) val & ((1 << operand->bits) - 1))
		 << (operand->shift + shift));

      if ((operand->flags & MN10300_OPERAND_REPEATED) != 0)
	*insnp |= (((long) val & ((1 << operand->bits) - 1))
		   << (operand->shift + shift + operand->bits));
    }
  else
    {
      *extensionp |= (((long) val & ((1 << operand->bits) - 1))
		      << (operand->shift + shift));

      if ((operand->flags & MN10300_OPERAND_REPEATED) != 0)
	*extensionp |= (((long) val & ((1 << operand->bits) - 1))
			<< (operand->shift + shift + operand->bits));
    }
}

void
md_assemble (char *str)
{
  char *s;
  struct mn10300_opcode *opcode;
  struct mn10300_opcode *next_opcode;
  const unsigned char *opindex_ptr;
  int next_opindex, relaxable;
  unsigned long insn, extension, size = 0;
  char *f;
  int i;
  int match;

  /* Get the opcode.  */
  for (s = str; *s != '\0' && !ISSPACE (*s); s++)
    ;
  if (*s != '\0')
    *s++ = '\0';

  /* Find the first opcode with the proper name.  */
  opcode = (struct mn10300_opcode *) str_hash_find (mn10300_hash, str);
  if (opcode == NULL)
    {
      as_bad (_("Unrecognized opcode: `%s'"), str);
      return;
    }

  str = s;
  while (ISSPACE (*str))
    ++str;

  input_line_pointer = str;

  for (;;)
    {
      const char *errmsg;
      int op_idx;
      char *hold;
      int extra_shift = 0;

      errmsg = _("Invalid opcode/operands");

      /* Reset the array of register operands.  */
      memset (mn10300_reg_operands, -1, sizeof (mn10300_reg_operands));

      relaxable = 0;
      fc = 0;
      match = 0;
      next_opindex = 0;
      insn = opcode->opcode;
      extension = 0;

      /* If the instruction is not available on the current machine
	 then it can not possibly match.  */
      if (opcode->machine
	  && !(opcode->machine == AM33_2 && HAVE_AM33_2)
	  && !(opcode->machine == AM33 && HAVE_AM33)
	  && !(opcode->machine == AM30 && HAVE_AM30))
	goto error;

      for (op_idx = 1, opindex_ptr = opcode->operands;
	   *opindex_ptr != 0;
	   opindex_ptr++, op_idx++)
	{
	  const struct mn10300_operand *operand;
	  expressionS ex;

	  if (next_opindex == 0)
	    {
	      operand = &mn10300_operands[*opindex_ptr];
	    }
	  else
	    {
	      operand = &mn10300_operands[next_opindex];
	      next_opindex = 0;
	    }

	  while (*str == ' ' || *str == ',')
	    ++str;

	  if (operand->flags & MN10300_OPERAND_RELAX)
	    relaxable = 1;

	  /* Gather the operand.  */
	  hold = input_line_pointer;
	  input_line_pointer = str;

	  if (operand->flags & MN10300_OPERAND_PAREN)
	    {
	      if (*input_line_pointer != ')' && *input_line_pointer != '(')
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      input_line_pointer++;
	      goto keep_going;
	    }
	  /* See if we can match the operands.  */
	  else if (operand->flags & MN10300_OPERAND_DREG)
	    {
	      if (!data_register_name (&ex))
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	    }
	  else if (operand->flags & MN10300_OPERAND_AREG)
	    {
	      if (!address_register_name (&ex))
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	    }
	  else if (operand->flags & MN10300_OPERAND_SP)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "sp") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_RREG)
	    {
	      if (!r_register_name (&ex))
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	    }
	  else if (operand->flags & MN10300_OPERAND_XRREG)
	    {
	      if (!xr_register_name (&ex))
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	    }
	  else if (operand->flags & MN10300_OPERAND_FSREG)
	    {
	      if (!float_register_name (&ex))
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	    }
	  else if (operand->flags & MN10300_OPERAND_FDREG)
	    {
	      if (!double_register_name (&ex))
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	    }
	  else if (operand->flags & MN10300_OPERAND_FPCR)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "fpcr") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_USP)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "usp") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_SSP)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "ssp") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_MSP)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "msp") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_PC)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "pc") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_EPSW)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "epsw") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_PLUS)
	    {
	      if (*input_line_pointer != '+')
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      input_line_pointer++;
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_PSW)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "psw") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_MDR)
	    {
	      char *start;
	      char c = get_symbol_name (&start);

	      if (strcasecmp (start, "mdr") != 0)
		{
		  (void) restore_line_pointer (c);
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}
	      (void) restore_line_pointer (c);
	      goto keep_going;
	    }
	  else if (operand->flags & MN10300_OPERAND_REG_LIST)
	    {
	      unsigned int value = 0;
	      if (*input_line_pointer != '[')
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}

	      /* Eat the '['.  */
	      input_line_pointer++;

	      /* We used to reject a null register list here; however,
		 we accept it now so the compiler can emit "call"
		 instructions for all calls to named functions.

		 The linker can then fill in the appropriate bits for the
		 register list and stack size or change the instruction
		 into a "calls" if using "call" is not profitable.  */
	      while (*input_line_pointer != ']')
		{
		  char *start;
		  char c;

		  if (*input_line_pointer == ',')
		    input_line_pointer++;

		  c = get_symbol_name (&start);

		  if (strcasecmp (start, "d2") == 0)
		    {
		      value |= 0x80;
		      (void) restore_line_pointer (c);
		    }
		  else if (strcasecmp (start, "d3") == 0)
		    {
		      value |= 0x40;
		      (void) restore_line_pointer (c);
		    }
		  else if (strcasecmp (start, "a2") == 0)
		    {
		      value |= 0x20;
		      (void) restore_line_pointer (c);
		    }
		  else if (strcasecmp (start, "a3") == 0)
		    {
		      value |= 0x10;
		      (void) restore_line_pointer (c);
		    }
		  else if (strcasecmp (start, "other") == 0)
		    {
		      value |= 0x08;
		      (void) restore_line_pointer (c);
		    }
		  else if (HAVE_AM33
			   && strcasecmp (start, "exreg0") == 0)
		    {
		      value |= 0x04;
		      (void) restore_line_pointer (c);
		    }
		  else if (HAVE_AM33
			   && strcasecmp (start, "exreg1") == 0)
		    {
		      value |= 0x02;
		      (void) restore_line_pointer (c);
		    }
		  else if (HAVE_AM33
			   && strcasecmp (start, "exother") == 0)
		    {
		      value |= 0x01;
		      (void) restore_line_pointer (c);
		    }
		  else if (HAVE_AM33
			   && strcasecmp (start, "all") == 0)
		    {
		      value |= 0xff;
		      (void) restore_line_pointer (c);
		    }
		  else
		    {
		      input_line_pointer = hold;
		      str = hold;
		      goto error;
		    }
		}
	      input_line_pointer++;
              mn10300_insert_operand (& insn, & extension, operand,
                                      value, NULL, 0, 0);
	      goto keep_going;

	    }
	  else if (data_register_name (&ex))
	    {
	      input_line_pointer = hold;
	      str = hold;
	      goto error;
	    }
	  else if (address_register_name (&ex))
	    {
	      input_line_pointer = hold;
	      str = hold;
	      goto error;
	    }
	  else if (other_register_name (&ex))
	    {
	      input_line_pointer = hold;
	      str = hold;
	      goto error;
	    }
	  else if (HAVE_AM33 && r_register_name (&ex))
	    {
	      input_line_pointer = hold;
	      str = hold;
	      goto error;
	    }
	  else if (HAVE_AM33 && xr_register_name (&ex))
	    {
	      input_line_pointer = hold;
	      str = hold;
	      goto error;
	    }
	  else if (HAVE_AM33_2 && float_register_name (&ex))
	    {
	      input_line_pointer = hold;
	      str = hold;
	      goto error;
	    }
	  else if (HAVE_AM33_2 && double_register_name (&ex))
	    {
	      input_line_pointer = hold;
	      str = hold;
	      goto error;
	    }
	  else if (*str == ')' || *str == '(')
	    {
	      input_line_pointer = hold;
	      str = hold;
	      goto error;
	    }
	  else
	    {
	      expression (&ex);
	      resolve_register (&ex);
	    }

	  switch (ex.X_op)
	    {
	    case O_illegal:
	      errmsg = _("illegal operand");
	      goto error;
	    case O_absent:
	      errmsg = _("missing operand");
	      goto error;
	    case O_register:
	      {
		int mask;

		mask = MN10300_OPERAND_DREG | MN10300_OPERAND_AREG;
		if (HAVE_AM33)
		  mask |= MN10300_OPERAND_RREG | MN10300_OPERAND_XRREG;
		if (HAVE_AM33_2)
		  mask |= MN10300_OPERAND_FSREG | MN10300_OPERAND_FDREG;
		if ((operand->flags & mask) == 0)
		  {
		    input_line_pointer = hold;
		    str = hold;
		    goto error;
		  }

		if (opcode->format == FMT_D1 || opcode->format == FMT_S1)
		  extra_shift = 8;
		else if (opcode->format == FMT_D2
			 || opcode->format == FMT_D4
			 || opcode->format == FMT_S2
			 || opcode->format == FMT_S4
			 || opcode->format == FMT_S6
			 || opcode->format == FMT_D5)
		  extra_shift = 16;
		else if (opcode->format == FMT_D7)
		  extra_shift = 8;
		else if (opcode->format == FMT_D8 || opcode->format == FMT_D9)
		  extra_shift = 8;
		else
		  extra_shift = 0;

		mn10300_insert_operand (& insn, & extension, operand,
					ex.X_add_number, NULL,
					0, extra_shift);

		/* And note the register number in the register array.  */
		mn10300_reg_operands[op_idx - 1] = ex.X_add_number;
		break;
	      }

	    case O_constant:
	      /* If this operand can be promoted, and it doesn't
		 fit into the allocated bitfield for this insn,
		 then promote it (ie this opcode does not match).  */
	      if (operand->flags
		  & (MN10300_OPERAND_PROMOTE | MN10300_OPERAND_RELAX)
		  && !check_operand (operand, ex.X_add_number))
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}

	      mn10300_insert_operand (& insn, & extension, operand,
				      ex.X_add_number, NULL, 0, 0);
	      break;

	    default:
	      /* If this operand can be promoted, then this opcode didn't
		 match since we can't know if it needed promotion!  */
	      if (operand->flags & MN10300_OPERAND_PROMOTE)
		{
		  input_line_pointer = hold;
		  str = hold;
		  goto error;
		}

	      /* We need to generate a fixup for this expression.  */
	      if (fc >= MAX_INSN_FIXUPS)
		as_fatal (_("too many fixups"));
	      fixups[fc].exp = ex;
	      fixups[fc].opindex = *opindex_ptr;
	      fixups[fc].reloc = BFD_RELOC_UNUSED;
	      if (mn10300_check_fixup (& fixups[fc]))
		goto error;
	      ++fc;
	      break;
	    }

	keep_going:
	  str = input_line_pointer;
	  input_line_pointer = hold;

	  while (*str == ' ' || *str == ',')
	    ++str;
	}

      /* Make sure we used all the operands!  */
      if (*str != ',')
	match = 1;

      /* If this instruction has registers that must not match, verify
	 that they do indeed not match.  */
      if (opcode->no_match_operands)
	{
	  /* Look at each operand to see if it's marked.  */
	  for (i = 0; i < MN10300_MAX_OPERANDS; i++)
	    {
	      if ((1 << i) & opcode->no_match_operands)
		{
		  int j;

		  /* operand I is marked.  Check that it does not match any
		     operands > I which are marked.  */
		  for (j = i + 1; j < MN10300_MAX_OPERANDS; j++)
		    {
		      if (((1 << j) & opcode->no_match_operands)
			  && mn10300_reg_operands[i] == mn10300_reg_operands[j])
			{
			  errmsg = _("Invalid register specification.");
			  match = 0;
			  goto error;
			}
		    }
		}
	    }
	}

    error:
      if (match == 0)
	{
	  next_opcode = opcode + 1;
	  if (!strcmp (next_opcode->name, opcode->name))
	    {
	      opcode = next_opcode;
	      continue;
	    }

	  as_bad ("%s", errmsg);
	  return;
	}
      break;
    }

  while (ISSPACE (*str))
    ++str;

  if (*str != '\0')
    as_bad (_("junk at end of line: `%s'"), str);

  input_line_pointer = str;

  /* Determine the size of the instruction.  */
  if (opcode->format == FMT_S0)
    size = 1;

  if (opcode->format == FMT_S1 || opcode->format == FMT_D0)
    size = 2;

  if (opcode->format == FMT_S2 || opcode->format == FMT_D1)
    size = 3;

  if (opcode->format == FMT_D6)
    size = 3;

  if (opcode->format == FMT_D7 || opcode->format == FMT_D10)
    size = 4;

  if (opcode->format == FMT_D8)
    size = 6;

  if (opcode->format == FMT_D9)
    size = 7;

  if (opcode->format == FMT_S4)
    size = 5;

  if (opcode->format == FMT_S6 || opcode->format == FMT_D5)
    size = 7;

  if (opcode->format == FMT_D2)
    size = 4;

  if (opcode->format == FMT_D3)
    size = 5;

  if (opcode->format == FMT_D4)
    size = 6;

  if (relaxable && fc > 0)
    {
      /* On a 64-bit host the size of an 'int' is not the same
	 as the size of a pointer, so we need a union to convert
	 the opindex field of the fr_cgen structure into a char *
	 so that it can be stored in the frag.  We do not have
	 to worry about losing accuracy as we are not going to
	 be even close to the 32bit limit of the int.  */
      union
      {
	int opindex;
	char * ptr;
      }
      opindex_converter;
      int type;

      /* We want to anchor the line info to the previous frag (if
	 there isn't one, create it), so that, when the insn is
	 resized, we still get the right address for the beginning of
	 the region.  */
      f = frag_more (0);
      dwarf2_emit_insn (0);

      /* bCC  */
      if (size == 2)
	{
	  /* Handle bra specially.  Basically treat it like jmp so
	     that we automatically handle 8, 16 and 32 bit offsets
	     correctly as well as jumps to an undefined address.

	     It is also important to not treat it like other bCC
	     instructions since the long forms of bra is different
	     from other bCC instructions.  */
	  if (opcode->opcode == 0xca00)
	    type = 10;
	  else
	    type = 0;
	}
      /* call  */
      else if (size == 5)
	type = 6;
      /* calls  */
      else if (size == 4)
	type = 8;
      /* jmp  */
      else if (size == 3 && opcode->opcode == 0xcc0000)
	type = 10;
      else if (size == 3 && (opcode->opcode & 0xfff000) == 0xf8d000)
	type = 13;
      /* bCC (uncommon cases)  */
      else
	type = 3;

      opindex_converter.opindex = fixups[0].opindex;
      f = frag_var (rs_machine_dependent, 8, 8 - size, type,
		    fixups[0].exp.X_add_symbol,
		    fixups[0].exp.X_add_number,
		    opindex_converter.ptr);

      /* This is pretty hokey.  We basically just care about the
	 opcode, so we have to write out the first word big endian.

	 The exception is "call", which has two operands that we
	 care about.

	 The first operand (the register list) happens to be in the
	 first instruction word, and will be in the right place if
	 we output the first word in big endian mode.

	 The second operand (stack size) is in the extension word,
	 and we want it to appear as the first character in the extension
	 word (as it appears in memory).  Luckily, writing the extension
	 word in big endian format will do what we want.  */
      number_to_chars_bigendian (f, insn, size > 4 ? 4 : size);
      if (size > 8)
	{
	  number_to_chars_bigendian (f + 4, extension, 4);
	  number_to_chars_bigendian (f + 8, 0, size - 8);
	}
      else if (size > 4)
	number_to_chars_bigendian (f + 4, extension, size - 4);
    }
  else
    {
      /* Allocate space for the instruction.  */
      f = frag_more (size);

      /* Fill in bytes for the instruction.  Note that opcode fields
	 are written big-endian, 16 & 32bit immediates are written
	 little endian.  Egad.  */
      if (opcode->format == FMT_S0
	  || opcode->format == FMT_S1
	  || opcode->format == FMT_D0
	  || opcode->format == FMT_D6
	  || opcode->format == FMT_D7
	  || opcode->format == FMT_D10
	  || opcode->format == FMT_D1)
	{
	  number_to_chars_bigendian (f, insn, size);
	}
      else if (opcode->format == FMT_S2
	       && opcode->opcode != 0xdf0000
	       && opcode->opcode != 0xde0000)
	{
	  /* A format S2 instruction that is _not_ "ret" and "retf".  */
	  number_to_chars_bigendian (f, (insn >> 16) & 0xff, 1);
	  number_to_chars_littleendian (f + 1, insn & 0xffff, 2);
	}
      else if (opcode->format == FMT_S2)
	{
	  /* This must be a ret or retf, which is written entirely in
	     big-endian format.  */
	  number_to_chars_bigendian (f, insn, 3);
	}
      else if (opcode->format == FMT_S4
	       && opcode->opcode != 0xdc000000)
	{
	  /* This must be a format S4 "call" instruction.  What a pain.  */
	  unsigned long temp = (insn >> 8) & 0xffff;
	  number_to_chars_bigendian (f, (insn >> 24) & 0xff, 1);
	  number_to_chars_littleendian (f + 1, temp, 2);
	  number_to_chars_bigendian (f + 3, insn & 0xff, 1);
	  number_to_chars_bigendian (f + 4, extension & 0xff, 1);
	}
      else if (opcode->format == FMT_S4)
	{
	  /* This must be a format S4 "jmp" instruction.  */
	  unsigned long temp = ((insn & 0xffffff) << 8) | (extension & 0xff);
	  number_to_chars_bigendian (f, (insn >> 24) & 0xff, 1);
	  number_to_chars_littleendian (f + 1, temp, 4);
	}
      else if (opcode->format == FMT_S6)
	{
	  unsigned long temp = ((insn & 0xffffff) << 8)
	    | ((extension >> 16) & 0xff);
	  number_to_chars_bigendian (f, (insn >> 24) & 0xff, 1);
	  number_to_chars_littleendian (f + 1, temp, 4);
	  number_to_chars_bigendian (f + 5, (extension >> 8) & 0xff, 1);
	  number_to_chars_bigendian (f + 6, extension & 0xff, 1);
	}
      else if (opcode->format == FMT_D2
	       && opcode->opcode != 0xfaf80000
	       && opcode->opcode != 0xfaf00000
	       && opcode->opcode != 0xfaf40000)
	{
	  /* A format D2 instruction where the 16bit immediate is
	     really a single 16bit value, not two 8bit values.  */
	  number_to_chars_bigendian (f, (insn >> 16) & 0xffff, 2);
	  number_to_chars_littleendian (f + 2, insn & 0xffff, 2);
	}
      else if (opcode->format == FMT_D2)
	{
	  /* A format D2 instruction where the 16bit immediate
	     is really two 8bit immediates.  */
	  number_to_chars_bigendian (f, insn, 4);
	}
      else if (opcode->format == FMT_D3)
	{
	  number_to_chars_bigendian (f, (insn >> 16) & 0xffff, 2);
	  number_to_chars_littleendian (f + 2, insn & 0xffff, 2);
	  number_to_chars_bigendian (f + 4, extension & 0xff, 1);
	}
      else if (opcode->format == FMT_D4)
	{
	  unsigned long temp = ((insn & 0xffff) << 16) | (extension & 0xffff);

	  number_to_chars_bigendian (f, (insn >> 16) & 0xffff, 2);
	  number_to_chars_littleendian (f + 2, temp, 4);
	}
      else if (opcode->format == FMT_D5)
	{
	  unsigned long temp = (((insn & 0xffff) << 16)
				| ((extension >> 8) & 0xffff));

	  number_to_chars_bigendian (f, (insn >> 16) & 0xffff, 2);
	  number_to_chars_littleendian (f + 2, temp, 4);
	  number_to_chars_bigendian (f + 6, extension & 0xff, 1);
	}
      else if (opcode->format == FMT_D8)
	{
	  unsigned long temp = ((insn & 0xff) << 16) | (extension & 0xffff);

	  number_to_chars_bigendian (f, (insn >> 8) & 0xffffff, 3);
	  number_to_chars_bigendian (f + 3, (temp & 0xff), 1);
	  number_to_chars_littleendian (f + 4, temp >> 8, 2);
	}
      else if (opcode->format == FMT_D9)
	{
	  unsigned long temp = ((insn & 0xff) << 24) | (extension & 0xffffff);

	  number_to_chars_bigendian (f, (insn >> 8) & 0xffffff, 3);
	  number_to_chars_littleendian (f + 3, temp, 4);
	}

      /* Create any fixups.  */
      for (i = 0; i < fc; i++)
	{
	  const struct mn10300_operand *operand;
	  int reloc_size;

	  operand = &mn10300_operands[fixups[i].opindex];
	  if (fixups[i].reloc != BFD_RELOC_UNUSED
	      && fixups[i].reloc != BFD_RELOC_32_GOT_PCREL
	      && fixups[i].reloc != BFD_RELOC_32_GOTOFF
	      && fixups[i].reloc != BFD_RELOC_32_PLT_PCREL
	      && fixups[i].reloc != BFD_RELOC_MN10300_TLS_GD
	      && fixups[i].reloc != BFD_RELOC_MN10300_TLS_LD
	      && fixups[i].reloc != BFD_RELOC_MN10300_TLS_LDO
	      && fixups[i].reloc != BFD_RELOC_MN10300_TLS_GOTIE
	      && fixups[i].reloc != BFD_RELOC_MN10300_TLS_IE
	      && fixups[i].reloc != BFD_RELOC_MN10300_TLS_LE
	      && fixups[i].reloc != BFD_RELOC_MN10300_GOT32)
	    {
	      reloc_howto_type *reloc_howto;
	      int offset;

	      reloc_howto = bfd_reloc_type_lookup (stdoutput,
						   fixups[i].reloc);

	      if (!reloc_howto)
		abort ();

	      reloc_size = bfd_get_reloc_size (reloc_howto);

	      if (reloc_size < 1 || reloc_size > 4)
		abort ();

	      offset = 4 - size;
	      fix_new_exp (frag_now, f - frag_now->fr_literal + offset,
			   reloc_size, &fixups[i].exp,
			   reloc_howto->pc_relative,
			   fixups[i].reloc);
	    }
	  else
	    {
	      int reloc, pcrel, offset;
	      fixS *fixP;

	      reloc = BFD_RELOC_NONE;
	      if (fixups[i].reloc != BFD_RELOC_UNUSED)
		reloc = fixups[i].reloc;
	      /* How big is the reloc?  Remember SPLIT relocs are
		 implicitly 32bits.  */
	      if ((operand->flags & MN10300_OPERAND_SPLIT) != 0)
		reloc_size = 32;
	      else if ((operand->flags & MN10300_OPERAND_24BIT) != 0)
		reloc_size = 24;
	      else
		reloc_size = operand->bits;

	      /* Is the reloc pc-relative?  */
	      pcrel = (operand->flags & MN10300_OPERAND_PCREL) != 0;
	      if (reloc != BFD_RELOC_NONE)
		pcrel = bfd_reloc_type_lookup (stdoutput, reloc)->pc_relative;

	      offset = size - (reloc_size + operand->shift) / 8;

	      /* Choose a proper BFD relocation type.  */
	      if (reloc != BFD_RELOC_NONE)
		;
	      else if (pcrel)
		{
		  if (reloc_size == 32)
		    reloc = BFD_RELOC_32_PCREL;
		  else if (reloc_size == 16)
		    reloc = BFD_RELOC_16_PCREL;
		  else if (reloc_size == 8)
		    reloc = BFD_RELOC_8_PCREL;
		  else
		    abort ();
		}
	      else
		{
		  if (reloc_size == 32)
		    reloc = BFD_RELOC_32;
		  else if (reloc_size == 16)
		    reloc = BFD_RELOC_16;
		  else if (reloc_size == 8)
		    reloc = BFD_RELOC_8;
		  else
		    abort ();
		}

	      fixP = fix_new_exp (frag_now, f - frag_now->fr_literal + offset,
				  reloc_size / 8, &fixups[i].exp, pcrel,
				  ((bfd_reloc_code_real_type) reloc));

	      if (pcrel)
		fixP->fx_offset += offset;
	    }
	}

      dwarf2_emit_insn (size);
    }

  /* Label this frag as one that contains instructions.  */
  frag_now->tc_frag_data = true;
}

/* If while processing a fixup, a reloc really needs to be created
   then it is done here.  */

arelent **
tc_gen_reloc (asection *seg ATTRIBUTE_UNUSED, fixS *fixp)
{
  static arelent * no_relocs = NULL;
  static arelent * relocs[MAX_RELOC_EXPANSION + 1];
  arelent *reloc;

  reloc = XNEW (arelent);

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("reloc %d not supported by object file format"),
		    (int) fixp->fx_r_type);
      free (reloc);
      return & no_relocs;
    }

  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  relocs[0] = reloc;
  relocs[1] = NULL;

  if (fixp->fx_subsy
      && S_GET_SEGMENT (fixp->fx_subsy) == absolute_section)
    {
      fixp->fx_offset -= S_GET_VALUE (fixp->fx_subsy);
      fixp->fx_subsy = NULL;
    }

  if (fixp->fx_addsy && fixp->fx_subsy)
    {
      asection *asec, *ssec;

      asec = S_GET_SEGMENT (fixp->fx_addsy);
      ssec = S_GET_SEGMENT (fixp->fx_subsy);

      /* If we have a difference between two (non-absolute) symbols we must
	 generate two relocs (one for each symbol) and allow the linker to
	 resolve them - relaxation may change the distances between symbols,
	 even local symbols defined in the same section.  */
      if (ssec != absolute_section || asec != absolute_section)
	{
	  arelent * reloc2 = XNEW (arelent);

	  relocs[0] = reloc2;
	  relocs[1] = reloc;

	  reloc2->address = reloc->address;
	  reloc2->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_MN10300_SYM_DIFF);
	  reloc2->addend = - S_GET_VALUE (fixp->fx_subsy);
	  reloc2->sym_ptr_ptr = XNEW (asymbol *);
	  *reloc2->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_subsy);

	  reloc->addend = fixp->fx_offset;
	  if (asec == absolute_section)
	    {
	      reloc->addend += S_GET_VALUE (fixp->fx_addsy);
	      reloc->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	    }
	  else
	    {
	      reloc->sym_ptr_ptr = XNEW (asymbol *);
	      *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
	    }

	  fixp->fx_pcrel = 0;
	  fixp->fx_done = 1;
	  return relocs;
	}
      else
	{
	  char *fixpos = fixp->fx_where + fixp->fx_frag->fr_literal;

	  reloc->addend = (S_GET_VALUE (fixp->fx_addsy)
			   - S_GET_VALUE (fixp->fx_subsy) + fixp->fx_offset);

	  switch (fixp->fx_r_type)
	    {
	    case BFD_RELOC_8:
	      md_number_to_chars (fixpos, reloc->addend, 1);
	      break;

	    case BFD_RELOC_16:
	      md_number_to_chars (fixpos, reloc->addend, 2);
	      break;

	    case BFD_RELOC_24:
	      md_number_to_chars (fixpos, reloc->addend, 3);
	      break;

	    case BFD_RELOC_32:
	      md_number_to_chars (fixpos, reloc->addend, 4);
	      break;

	    default:
	      reloc->sym_ptr_ptr
		= (asymbol **) bfd_abs_section_ptr->symbol_ptr_ptr;
	      return relocs;
	    }

	  free (reloc);
	  return & no_relocs;
	}
    }
  else
    {
      reloc->sym_ptr_ptr = XNEW (asymbol *);
      *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
      reloc->addend = fixp->fx_offset;
    }
  return relocs;
}

/* Returns true iff the symbol attached to the frag is at a known location
   in the given section, (and hence the relocation to it can be relaxed by
   the assembler).  */
static inline bool
has_known_symbol_location (fragS * fragp, asection * sec)
{
  symbolS * sym = fragp->fr_symbol;

  return sym != NULL
    && S_IS_DEFINED (sym)
    && ! S_IS_WEAK (sym)
    && S_GET_SEGMENT (sym) == sec;
}

int
md_estimate_size_before_relax (fragS *fragp, asection *seg)
{
  if (fragp->fr_subtype == 6
      && ! has_known_symbol_location (fragp, seg))
    fragp->fr_subtype = 7;
  else if (fragp->fr_subtype == 8
	   && ! has_known_symbol_location (fragp, seg))
    fragp->fr_subtype = 9;
  else if (fragp->fr_subtype == 10
	   && ! has_known_symbol_location (fragp, seg))
    fragp->fr_subtype = 12;

  if (fragp->fr_subtype == 13)
    return 3;

  if (fragp->fr_subtype >= sizeof (md_relax_table) / sizeof (md_relax_table[0]))
    abort ();

  return md_relax_table[fragp->fr_subtype].rlx_length;
}

long
md_pcrel_from (fixS *fixp)
{
  if (fixp->fx_addsy != (symbolS *) NULL
      && (!S_IS_DEFINED (fixp->fx_addsy) || S_IS_WEAK (fixp->fx_addsy)))
    /* The symbol is undefined or weak.  Let the linker figure it out.  */
    return 0;

  return fixp->fx_frag->fr_address + fixp->fx_where;
}

void
md_apply_fix (fixS * fixP, valueT * valP, segT seg)
{
  char * fixpos = fixP->fx_where + fixP->fx_frag->fr_literal;
  int size = 0;
  int value = (int) * valP;

  gas_assert (fixP->fx_r_type < BFD_RELOC_UNUSED);

  /* This should never happen.  */
  if (seg->flags & SEC_ALLOC)
    abort ();

  /* The value we are passed in *valuep includes the symbol values.
     If we are doing this relocation the code in write.c is going to
     call bfd_install_relocation, which is also going to use the symbol
     value.  That means that if the reloc is fully resolved we want to
     use *valuep since bfd_install_relocation is not being used.

     However, if the reloc is not fully resolved we do not want to use
     *valuep, and must use fx_offset instead.  However, if the reloc
     is PC relative, we do want to use *valuep since it includes the
     result of md_pcrel_from.  */
  if (fixP->fx_addsy != NULL && ! fixP->fx_pcrel)
    value = fixP->fx_offset;

  /* If the fix is relative to a symbol which is not defined, or not
     in the same segment as the fix, we cannot resolve it here.  */
  if (fixP->fx_addsy != NULL
      && (! S_IS_DEFINED (fixP->fx_addsy)
	  || (S_GET_SEGMENT (fixP->fx_addsy) != seg)))
    {
      fixP->fx_done = 0;
      return;
    }

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_8:
    case BFD_RELOC_8_PCREL:
      size = 1;
      break;

    case BFD_RELOC_16:
    case BFD_RELOC_16_PCREL:
      size = 2;
      break;

    case BFD_RELOC_32:
    case BFD_RELOC_32_PCREL:
      size = 4;
      break;

    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = 0;
      return;

    case BFD_RELOC_MN10300_ALIGN:
      fixP->fx_done = 1;
      return;

    case BFD_RELOC_NONE:
    default:
      as_bad_where (fixP->fx_file, fixP->fx_line,
                   _("Bad relocation fixup type (%d)"), fixP->fx_r_type);
    }

  md_number_to_chars (fixpos, value, size);

  /* If a symbol remains, pass the fixup, as a reloc, onto the linker.  */
  if (fixP->fx_addsy == NULL)
    fixP->fx_done = 1;
}

/* Return zero if the fixup in fixp should be left alone and not
   adjusted.  */

bool
mn10300_fix_adjustable (struct fix *fixp)
{
  if (fixp->fx_pcrel)
    {
      if (TC_FORCE_RELOCATION_LOCAL (fixp))
	return false;
    }
  /* Non-relative relocs can (and must) be adjusted if they do
     not meet the criteria below, or the generic criteria.  */
  else if (TC_FORCE_RELOCATION (fixp))
    return false;

  /* Do not adjust relocations involving symbols in code sections,
     because it breaks linker relaxations.  This could be fixed in the
     linker, but this fix is simpler, and it pretty much only affects
     object size a little bit.  */
  if (S_GET_SEGMENT (fixp->fx_addsy)->flags & SEC_CODE)
    return false;

  /* Likewise, do not adjust symbols that won't be merged, or debug
     symbols, because they too break relaxation.  We do want to adjust
     other mergeable symbols, like .rodata, because code relaxations
     need section-relative symbols to properly relax them.  */
  if (! (S_GET_SEGMENT (fixp->fx_addsy)->flags & SEC_MERGE))
    return false;

  if (startswith (S_GET_SEGMENT (fixp->fx_addsy)->name, ".debug"))
    return false;

  return true;
}

static void
set_arch_mach (int mach)
{
  if (!bfd_set_arch_mach (stdoutput, bfd_arch_mn10300, mach))
    as_warn (_("could not set architecture and machine"));

  current_machine = mach;
}

static inline char *
mn10300_end_of_match (char *cont, const char *what)
{
  int len = strlen (what);

  if (startswith (cont, what)
      && ! is_part_of_name (cont[len]))
    return cont + len;

  return NULL;
}

int
mn10300_parse_name (char const *name,
		    expressionS *exprP,
		    enum expr_mode mode,
		    char *nextcharP)
{
  char *next = input_line_pointer;
  char *next_end;
  int reloc_type;
  segT segment;

  exprP->X_op_symbol = NULL;

  if (strcmp (name, GLOBAL_OFFSET_TABLE_NAME) == 0)
    {
      if (! GOT_symbol)
	GOT_symbol = symbol_find_or_make (name);

      exprP->X_add_symbol = GOT_symbol;
    no_suffix:
      /* If we have an absolute symbol or a reg,
	 then we know its value now.  */
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
  else if ((next_end = mn10300_end_of_match (next + 1, "GOTOFF")))
    reloc_type = BFD_RELOC_32_GOTOFF;
  else if ((next_end = mn10300_end_of_match (next + 1, "GOT")))
    reloc_type = BFD_RELOC_MN10300_GOT32;
  else if ((next_end = mn10300_end_of_match (next + 1, "PLT")))
    reloc_type = BFD_RELOC_32_PLT_PCREL;
  else if ((next_end = mn10300_end_of_match (next + 1, "tlsgd")))
    reloc_type = BFD_RELOC_MN10300_TLS_GD;
  else if ((next_end = mn10300_end_of_match (next + 1, "tlsldm")))
    reloc_type = BFD_RELOC_MN10300_TLS_LD;
  else if ((next_end = mn10300_end_of_match (next + 1, "dtpoff")))
    reloc_type = BFD_RELOC_MN10300_TLS_LDO;
  else if ((next_end = mn10300_end_of_match (next + 1, "gotntpoff")))
    reloc_type = BFD_RELOC_MN10300_TLS_GOTIE;
  else if ((next_end = mn10300_end_of_match (next + 1, "indntpoff")))
    reloc_type = BFD_RELOC_MN10300_TLS_IE;
  else if ((next_end = mn10300_end_of_match (next + 1, "tpoff")))
    reloc_type = BFD_RELOC_MN10300_TLS_LE;
  else
    goto no_suffix;

  *input_line_pointer = *nextcharP;
  input_line_pointer = next_end;
  *nextcharP = *input_line_pointer;
  *input_line_pointer = '\0';

  exprP->X_op = O_PIC_reloc;
  exprP->X_add_number = 0;
  exprP->X_md = reloc_type;

  return 1;
}

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  { "am30",	set_arch_mach,	AM30 },
  { "am33",	set_arch_mach,	AM33 },
  { "am33_2",	set_arch_mach,	AM33_2 },
  { "mn10300",	set_arch_mach,	MN103 },
  {NULL, 0, 0}
};

/* Returns FALSE if there is some mn10300 specific reason why the
   subtraction of two same-section symbols cannot be computed by
   the assembler.  */

bool
mn10300_allow_local_subtract (expressionS * left, expressionS * right, segT section)
{
  bool result;
  fragS * left_frag;
  fragS * right_frag;
  fragS * frag;

  /* If we are not performing linker relaxation then we have nothing
     to worry about.  */
  if (linkrelax == 0)
    return true;

  /* If the symbols are not in a code section then they are OK.  */
  if ((section->flags & SEC_CODE) == 0)
    return true;

  /* Otherwise we have to scan the fragments between the two symbols.
     If any instructions are found then we have to assume that linker
     relaxation may change their size and so we must delay resolving
     the subtraction until the final link.  */
  left_frag = symbol_get_frag (left->X_add_symbol);
  right_frag = symbol_get_frag (right->X_add_symbol);

  if (left_frag == right_frag)
    return ! left_frag->tc_frag_data;

  result = true;
  for (frag = left_frag; frag != NULL; frag = frag->fr_next)
    {
      if (frag->tc_frag_data)
	result = false;
      if (frag == right_frag)
	break;
    }

  if (frag == NULL)
    for (frag = right_frag; frag != NULL; frag = frag->fr_next)
      {
	if (frag->tc_frag_data)
	  result = false;
	if (frag == left_frag)
	  break;
      }

  if (frag == NULL)
    /* The two symbols are on disjoint fragment chains
       - we cannot possibly compute their difference.  */
    return false;

  return result;
}

/* When relaxing, we need to output a reloc for any .align directive
   that requests alignment to a two byte boundary or larger.  */

void
mn10300_handle_align (fragS *frag)
{
  if (linkrelax
      && (frag->fr_type == rs_align
	  || frag->fr_type == rs_align_code)
      && frag->fr_address + frag->fr_fix > 0
      && frag->fr_offset > 1
      && now_seg != bss_section
      /* Do not create relocs for the merging sections - such
	 relocs will prevent the contents from being merged.  */
      && (bfd_section_flags (now_seg) & SEC_MERGE) == 0)
    /* Create a new fixup to record the alignment request.  The symbol is
       irrelevant but must be present so we use the absolute section symbol.
       The offset from the symbol is used to record the power-of-two alignment
       value.  The size is set to 0 because the frag may already be aligned,
       thus causing cvt_frag_to_fill to reduce the size of the frag to zero.  */
    fix_new (frag, frag->fr_fix, 0, & abs_symbol, frag->fr_offset, false,
	     BFD_RELOC_MN10300_ALIGN);
}

bool
mn10300_force_relocation (struct fix * fixp)
{
  if (linkrelax
      && (fixp->fx_pcrel
	  || fixp->fx_r_type == BFD_RELOC_MN10300_ALIGN))
    return true;

  return generic_force_reloc (fixp);
}
