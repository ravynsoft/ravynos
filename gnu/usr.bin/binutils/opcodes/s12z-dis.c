/* s12z-dis.c -- Freescale S12Z disassembly
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "opcode/s12z.h"
#include "bfd.h"
#include "dis-asm.h"
#include "disassemble.h"
#include "s12z-opc.h"
#include "opintl.h"

struct mem_read_abstraction
{
  struct mem_read_abstraction_base base;
  bfd_vma memaddr;
  struct disassemble_info* info;
};

static void
advance (struct mem_read_abstraction_base *b)
{
  struct mem_read_abstraction *mra = (struct mem_read_abstraction *) b;
  mra->memaddr ++;
}

static bfd_vma
posn (struct mem_read_abstraction_base *b)
{
  struct mem_read_abstraction *mra = (struct mem_read_abstraction *) b;
  return mra->memaddr;
}

static int
abstract_read_memory (struct mem_read_abstraction_base *b,
		      int offset,
		      size_t n, bfd_byte *bytes)
{
  struct mem_read_abstraction *mra = (struct mem_read_abstraction *) b;

  int status = (*mra->info->read_memory_func) (mra->memaddr + offset,
					       bytes, n, mra->info);
  if (status != 0)
    (*mra->info->memory_error_func) (status, mra->memaddr + offset,
                                     mra->info);
  return status != 0 ? -1 : 0;
}

/* Start of disassembly file.  */
const struct reg registers[S12Z_N_REGISTERS] =
  {
    {"d2", 2},
    {"d3", 2},
    {"d4", 2},
    {"d5", 2},

    {"d0", 1},
    {"d1", 1},

    {"d6", 4},
    {"d7", 4},

    {"x", 3},
    {"y", 3},
    {"s", 3},
    {"p", 3},
    {"cch", 1},
    {"ccl", 1},
    {"ccw", 2}
  };

static const char *mnemonics[] =
  {
    "!!invalid!!",
    "psh",
    "pul",
    "tbne", "tbeq", "tbpl", "tbmi", "tbgt", "tble",
    "dbne", "dbeq", "dbpl", "dbmi", "dbgt", "dble",
    "sex",
    "exg",
    "lsl", "lsr",
    "asl", "asr",
    "rol", "ror",
    "bfins", "bfext",

    "trap",

    "ld",
    "st",
    "cmp",

    "stop",
    "wai",
    "sys",

    "minu",
    "mins",
    "maxu",
    "maxs",

    "abs",
    "adc",
    "bit",
    "sbc",
    "rti",
    "clb",
    "eor",

    "sat",

    "nop",
    "bgnd",
    "brclr",
    "brset",
    "rts",
    "lea",
    "mov",

    "bra",
    "bsr",
    "bhi",
    "bls",
    "bcc",
    "bcs",
    "bne",
    "beq",
    "bvc",
    "bvs",
    "bpl",
    "bmi",
    "bge",
    "blt",
    "bgt",
    "ble",
    "inc",
    "clr",
    "dec",

    "add",
    "sub",
    "and",
    "or",

    "tfr",
    "jmp",
    "jsr",
    "com",
    "andcc",
    "neg",
    "orcc",
    "bclr",
    "bset",
    "btgl",
    "swi",

    "mulu",
    "divu",
    "modu",
    "macu",
    "qmulu",

    "muls",
    "divs",
    "mods",
    "macs",
    "qmuls",

    NULL
  };


static void
operand_separator (struct disassemble_info *info)
{
  if ((info->flags & 0x2))
    (*info->fprintf_func) (info->stream, ",");

  (*info->fprintf_func) (info->stream, " ");

  info->flags |= 0x2;
}

/* Render the symbol name whose value is ADDR + BASE or the adddress itself if
   there is no symbol.  If BASE is non zero, then the a PC relative adddress is
   assumend (ie BASE is the value in the PC.  */
static void
decode_possible_symbol (bfd_signed_vma addr, bfd_vma base,
                        struct disassemble_info *info, bool relative)
{
  const char *fmt = relative ? "*%+" PRId64 : "%" PRId64;
  asymbol *sym = info->symbol_at_address_func (addr + base, info);

  if (!sym)
    (*info->fprintf_func) (info->stream, fmt, (int64_t) addr);
  else
    (*info->fprintf_func) (info->stream, "%s", bfd_asymbol_name (sym));
}


/* Emit the disassembled text for OPR */
static void
opr_emit_disassembly (const struct operand *opr,
		      struct disassemble_info *info)
{
  operand_separator (info);

  switch (opr->cl)
    {
    case OPND_CL_IMMEDIATE:
      (*info->fprintf_func) (info->stream, "#%d",
			     ((struct immediate_operand *) opr)->value);
      break;
    case OPND_CL_REGISTER:
      {
        int r = ((struct register_operand*) opr)->reg;

	if (r < 0 || r >= S12Z_N_REGISTERS)
	  (*info->fprintf_func) (info->stream, _("<illegal reg num>"));
	else
	  (*info->fprintf_func) (info->stream, "%s", registers[r].name);
      }
      break;
    case OPND_CL_REGISTER_ALL16:
      (*info->fprintf_func) (info->stream, "%s", "ALL16b");
      break;
    case OPND_CL_REGISTER_ALL:
      (*info->fprintf_func) (info->stream, "%s", "ALL");
      break;
    case OPND_CL_BIT_FIELD:
      (*info->fprintf_func) (info->stream, "#%d:%d",
                             ((struct bitfield_operand*)opr)->width,
                             ((struct bitfield_operand*)opr)->offset);
      break;
    case OPND_CL_SIMPLE_MEMORY:
      {
        struct simple_memory_operand *mo =
	  (struct simple_memory_operand *) opr;
	decode_possible_symbol (mo->addr, mo->base, info, mo->relative);
      }
      break;
    case OPND_CL_MEMORY:
      {
        int used_reg = 0;
        struct memory_operand *mo = (struct memory_operand *) opr;
	(*info->fprintf_func) (info->stream, "%c", mo->indirect ? '[' : '(');

	const char *fmt;
	assert (mo->mutation == OPND_RM_NONE || mo->n_regs == 1);
	switch (mo->mutation)
	  {
	  case OPND_RM_PRE_DEC:
	    fmt = "-%s";
	    break;
	  case OPND_RM_PRE_INC:
	    fmt = "+%s";
	    break;
	  case OPND_RM_POST_DEC:
	    fmt = "%s-";
	    break;
	  case OPND_RM_POST_INC:
	    fmt = "%s+";
	    break;
	  case OPND_RM_NONE:
	  default:
	    if (mo->n_regs < 2)
	      (*info->fprintf_func) (info->stream, (mo->n_regs == 0) ? "%d" : "%d,", mo->base_offset);
	    fmt = "%s";
	    break;
	  }
	if (mo->n_regs > 0)
	  {
	    int r = mo->regs[0];

	    if (r < 0 || r >= S12Z_N_REGISTERS)
	      (*info->fprintf_func) (info->stream, fmt, _("<illegal reg num>"));
	    else
	      (*info->fprintf_func) (info->stream, fmt, registers[r].name);
	  }
	used_reg = 1;

        if (mo->n_regs > used_reg)
          {
	    int r = mo->regs[used_reg];

	    if (r < 0 || r >= S12Z_N_REGISTERS)
	      (*info->fprintf_func) (info->stream, _("<illegal reg num>"));
	    else
	      (*info->fprintf_func) (info->stream, ",%s",
				     registers[r].name);
          }

	(*info->fprintf_func) (info->stream, "%c",
			       mo->indirect ? ']' : ')');
      }
      break;
    };
}

#define S12Z_N_SIZES 4
static const char shift_size_table[S12Z_N_SIZES] =
{
  'b', 'w', 'p', 'l'
};

int
print_insn_s12z (bfd_vma memaddr, struct disassemble_info* info)
{
  int o;
  enum optr operator = OP_INVALID;
  int n_operands = 0;

  /* The longest instruction in S12Z can have 6 operands.
     (Most have 3 or less.  Only PSH and PUL have so many.  */
  struct operand *operands[6];

  struct mem_read_abstraction mra;
  mra.base.read = (void *) abstract_read_memory ;
  mra.base.advance = advance ;
  mra.base.posn = posn;
  mra.memaddr = memaddr;
  mra.info = info;

  short osize = -1;
  int n_bytes =
    decode_s12z (&operator, &osize, &n_operands, operands,
		 (struct mem_read_abstraction_base *) &mra);

  (info->fprintf_func) (info->stream, "%s", mnemonics[(long)operator]);

  /* Ship out size sufficies for those instructions which
     need them.  */
  if (osize == -1)
    {
      bool suffix = false;

      for (o = 0; o < n_operands; ++o)
	{
	  if (operands[o] && operands[o]->osize != -1)
	    {
	      if (!suffix)
		{
		  (*mra.info->fprintf_func) (mra.info->stream, "%c", '.');
		  suffix = true;
		}

	      osize = operands[o]->osize;

	      if (osize < 0 || osize >= S12Z_N_SIZES)
		(*mra.info->fprintf_func) (mra.info->stream, _("<bad>"));
	      else
		(*mra.info->fprintf_func) (mra.info->stream, "%c",
					   shift_size_table[osize]);
	    }
	}
    }
  else
    {
      if (osize < 0 || osize >= S12Z_N_SIZES)
	(*mra.info->fprintf_func) (mra.info->stream, _(".<bad>"));
      else
	(*mra.info->fprintf_func) (mra.info->stream, ".%c",
				   shift_size_table[osize]);
    }

  /* Ship out the operands.  */
  for (o = 0; o < n_operands; ++o)
    {
      if (operands[o])
	opr_emit_disassembly (operands[o], mra.info);
      free (operands[o]);
    }

  return n_bytes;
}
