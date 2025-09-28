/* TI PRU disassemble routines
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   Contributed by Dimitar Dimitrov <dimitar@dinux.eu>

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
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "disassemble.h"
#include "opcode/pru.h"
#include "libiberty.h"
#include <string.h>
#include <assert.h>

/* No symbol table is available when this code runs out in an embedded
   system as when it is used for disassembler support in a monitor.  */
#if !defined (EMBEDDED_ENV)
#define SYMTAB_AVAILABLE 1
#include "elf-bfd.h"
#include "elf/pru.h"
#endif

/* Length of PRU instruction in bytes.  */
#define INSNLEN 4

/* Return a pointer to an pru_opcode struct for a given instruction
   opcode, or NULL if there is an error.  */
const struct pru_opcode *
pru_find_opcode (unsigned long opcode)
{
  const struct pru_opcode *p;
  const struct pru_opcode *op = NULL;
  const struct pru_opcode *pseudo_op = NULL;

  for (p = pru_opcodes; p < &pru_opcodes[NUMOPCODES]; p++)
    {
      if ((p->mask & opcode) == p->match)
	{
	  if ((p->pinfo & PRU_INSN_MACRO) == PRU_INSN_MACRO)
	    pseudo_op = p;
	  else if ((p->pinfo & PRU_INSN_LDI32) == PRU_INSN_LDI32)
	    /* ignore - should be caught with regular patterns */;
	  else
	    op = p;
	}
    }

  return pseudo_op ? pseudo_op : op;
}

/* There are 32 regular registers, each with 8 possible subfield selectors.  */
#define NUMREGNAMES (32 * 8)

static void
pru_print_insn_arg_reg (unsigned int r, unsigned int sel,
			disassemble_info *info)
{
  unsigned int i = r * RSEL_NUM_ITEMS + sel;
  assert (i < (unsigned int)pru_num_regs);
  assert (i < NUMREGNAMES);
  (*info->fprintf_func) (info->stream, "%s", pru_regs[i].name);
}

/* The function pru_print_insn_arg uses the character pointed
   to by ARGPTR to determine how it print the next token or separator
   character in the arguments to an instruction.  */
static int
pru_print_insn_arg (const char *argptr,
		      unsigned long opcode, bfd_vma address,
		      disassemble_info *info)
{
  long offs = 0;
  unsigned long i = 0;
  unsigned long io = 0;

  switch (*argptr)
    {
    case ',':
      (*info->fprintf_func) (info->stream, "%c ", *argptr);
      break;
    case 'd':
      pru_print_insn_arg_reg (GET_INSN_FIELD (RD, opcode),
			      GET_INSN_FIELD (RDSEL, opcode),
			      info);
      break;
    case 'D':
      /* The first 4 values for RDB and RSEL are the same, so we
	 can reuse some code.  */
      pru_print_insn_arg_reg (GET_INSN_FIELD (RD, opcode),
			      GET_INSN_FIELD (RDB, opcode),
			      info);
      break;
    case 's':
      pru_print_insn_arg_reg (GET_INSN_FIELD (RS1, opcode),
			      GET_INSN_FIELD (RS1SEL, opcode),
			      info);
      break;
    case 'S':
      pru_print_insn_arg_reg (GET_INSN_FIELD (RS1, opcode),
			      RSEL_31_0,
			      info);
      break;
    case 'b':
      io = GET_INSN_FIELD (IO, opcode);

      if (io)
	{
	  i = GET_INSN_FIELD (IMM8, opcode);
	  (*info->fprintf_func) (info->stream, "%ld", i);
	}
      else
	{
	pru_print_insn_arg_reg (GET_INSN_FIELD (RS2, opcode),
				GET_INSN_FIELD (RS2SEL, opcode),
				info);
	}
      break;
    case 'B':
      io = GET_INSN_FIELD (IO, opcode);

      if (io)
	{
	  i = GET_INSN_FIELD (IMM8, opcode) + 1;
	  (*info->fprintf_func) (info->stream, "%ld", i);
	}
      else
	{
	pru_print_insn_arg_reg (GET_INSN_FIELD (RS2, opcode),
				GET_INSN_FIELD (RS2SEL, opcode),
				info);
	}
      break;
    case 'j':
      io = GET_INSN_FIELD (IO, opcode);

      if (io)
	{
	  /* For the sake of pretty-printing, dump text addresses with
	     their "virtual" offset that we use for distinguishing
	     PMEM vs DMEM. This is needed for printing the correct text
	     labels.  */
	  bfd_vma text_offset = address & ~0x3fffff;
	  i = GET_INSN_FIELD (IMM16, opcode) * 4;
	  (*info->print_address_func) (i + text_offset, info);
	}
      else
	{
	  pru_print_insn_arg_reg (GET_INSN_FIELD (RS2, opcode),
				GET_INSN_FIELD (RS2SEL, opcode),
				info);
	}
      break;
    case 'W':
      i = GET_INSN_FIELD (IMM16, opcode);
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;
    case 'o':
      offs = GET_BROFF_SIGNED (opcode) * 4;
      (*info->print_address_func) (address + offs, info);
      break;
    case 'O':
      offs = GET_INSN_FIELD (LOOP_JMPOFFS, opcode) * 4;
      (*info->print_address_func) (address + offs, info);
      break;
    case 'l':
      i = GET_BURSTLEN (opcode);
      if (i < LSSBBO_BYTECOUNT_R0_BITS7_0)
	(*info->fprintf_func) (info->stream, "%ld", i + 1);
      else
	{
	  i -= LSSBBO_BYTECOUNT_R0_BITS7_0;
	  (*info->fprintf_func) (info->stream, "r0.b%ld", i);
	}
      break;
    case 'n':
      i = GET_INSN_FIELD (XFR_LENGTH, opcode);
      if (i < LSSBBO_BYTECOUNT_R0_BITS7_0)
	(*info->fprintf_func) (info->stream, "%ld", i + 1);
      else
	{
	  i -= LSSBBO_BYTECOUNT_R0_BITS7_0;
	  (*info->fprintf_func) (info->stream, "r0.b%ld", i);
	}
      break;
    case 'c':
      i = GET_INSN_FIELD (CB, opcode);
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;
    case 'w':
      i = GET_INSN_FIELD (WAKEONSTATUS, opcode);
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;
    case 'x':
      i = GET_INSN_FIELD (XFR_WBA, opcode);
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;
    default:
      (*info->fprintf_func) (info->stream, "unknown");
      break;
    }
  return 0;
}

/* pru_disassemble does all the work of disassembling a PRU
   instruction opcode.  */
static int
pru_disassemble (bfd_vma address, unsigned long opcode,
		   disassemble_info *info)
{
  const struct pru_opcode *op;

  info->bytes_per_line = INSNLEN;
  info->bytes_per_chunk = INSNLEN;
  info->display_endian = info->endian;
  info->insn_info_valid = 1;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->insn_type = dis_nonbranch;
  info->target = 0;
  info->target2 = 0;

  /* Find the major opcode and use this to disassemble
     the instruction and its arguments.  */
  op = pru_find_opcode (opcode);

  if (op != NULL)
    {
      (*info->fprintf_func) (info->stream, "%s", op->name);

      const char *argstr = op->args;
      if (argstr != NULL && *argstr != '\0')
	{
	  (*info->fprintf_func) (info->stream, "\t");
	  while (*argstr != '\0')
	    {
	      pru_print_insn_arg (argstr, opcode, address, info);
	      ++argstr;
	    }
	}
    }
  else
    {
      /* Handle undefined instructions.  */
      info->insn_type = dis_noninsn;
      (*info->fprintf_func) (info->stream, "0x%lx", opcode);
    }
  /* Tell the caller how far to advance the program counter.  */
  return INSNLEN;
}


/* print_insn_pru is the main disassemble function for PRU.  */
int
print_insn_pru (bfd_vma address, disassemble_info *info)
{
  bfd_byte buffer[INSNLEN];
  int status;

  status = (*info->read_memory_func) (address, buffer, INSNLEN, info);
  if (status == 0)
    {
      unsigned long insn;
      insn = (unsigned long) bfd_getl32 (buffer);
      status = pru_disassemble (address, insn, info);
    }
  else
    {
      (*info->memory_error_func) (status, address, info);
      status = -1;
    }
  return status;
}
