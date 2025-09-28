/* Disassemble D30V instructions.
   Copyright (C) 1997-2023 Free Software Foundation, Inc.

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
#include "opcode/d30v.h"
#include "disassemble.h"
#include "opintl.h"
#include "libiberty.h"

#define PC_MASK 0xFFFFFFFF

/* Return 0 if lookup fails,
   1 if found and only one form,
   2 if found and there are short and long forms.  */

static int
lookup_opcode (struct d30v_insn *insn, long num, int is_long)
{
  int i = 0, op_index;
  struct d30v_format *f;
  struct d30v_opcode *op = (struct d30v_opcode *) d30v_opcode_table;
  int op1 = (num >> 25) & 0x7;
  int op2 = (num >> 20) & 0x1f;
  int mod = (num >> 18) & 0x3;

  /* Find the opcode.  */
  do
    {
      if ((op->op1 == op1) && (op->op2 == op2))
	break;
      op++;
    }
  while (op->name);

  if (!op || !op->name)
    return 0;

  while (op->op1 == op1 && op->op2 == op2)
    {
      /* Scan through all the formats for the opcode.  */
      op_index = op->format[i++];
      do
	{
	  f = (struct d30v_format *) &d30v_format_table[op_index];
	  while (f->form == op_index)
	    {
	      if ((!is_long || f->form >= LONG) && (f->modifier == mod))
		{
		  insn->form = f;
		  break;
		}
	      f++;
	    }
	  if (insn->form)
	    break;
	}
      while ((op_index = op->format[i++]) != 0);
      if (insn->form)
	break;
      op++;
      i = 0;
    }
  if (insn->form == NULL)
    return 0;

  insn->op = op;
  insn->ecc = (num >> 28) & 0x7;
  if (op->format[1])
    return 2;
  else
    return 1;
}

static int
extract_value (uint64_t num, const struct d30v_operand *oper, int is_long)
{
  unsigned int val;
  int shift = 12 - oper->position;
  unsigned int mask = (0xFFFFFFFF >> (32 - oper->bits));

  if (is_long)
    {
      if (oper->bits == 32)
	/* Piece together 32-bit constant.  */
	val = ((num & 0x3FFFF)
	       | ((num & 0xFF00000) >> 2)
	       | ((num & 0x3F00000000LL) >> 6));
      else
	val = (num >> (32 + shift)) & mask;
    }
  else
    val = (num >> shift) & mask;

  if (oper->flags & OPERAND_SHIFT)
    val <<= 3;

  return val;
}

static void
print_insn (struct disassemble_info *info,
	    bfd_vma memaddr,
	    uint64_t num,
	    struct d30v_insn *insn,
	    int is_long,
	    int show_ext)
{
  unsigned int val, opnum;
  const struct d30v_operand *oper;
  int i, match, need_comma = 0, need_paren = 0, found_control = 0;
  unsigned int opind = 0;

  (*info->fprintf_func) (info->stream, "%s", insn->op->name);

  /* Check for CMP or CMPU.  */
  if (d30v_operand_table[insn->form->operands[0]].flags & OPERAND_NAME)
    {
      opind++;
      val =
	extract_value (num,
		       &d30v_operand_table[insn->form->operands[0]],
		       is_long);
      (*info->fprintf_func) (info->stream, "%s", d30v_cc_names[val]);
    }

  /* Add in ".s" or ".l".  */
  if (show_ext == 2)
    {
      if (is_long)
	(*info->fprintf_func) (info->stream, ".l");
      else
	(*info->fprintf_func) (info->stream, ".s");
    }

  if (insn->ecc)
    (*info->fprintf_func) (info->stream, "/%s", d30v_ecc_names[insn->ecc]);

  (*info->fprintf_func) (info->stream, "\t");

  while (opind < ARRAY_SIZE (insn->form->operands)
	 && (opnum = insn->form->operands[opind++]) != 0)
    {
      int bits;

      oper = &d30v_operand_table[opnum];
      bits = oper->bits;
      if (oper->flags & OPERAND_SHIFT)
	bits += 3;

      if (need_comma
	  && oper->flags != OPERAND_PLUS
	  && oper->flags != OPERAND_MINUS)
	{
	  need_comma = 0;
	  (*info->fprintf_func) (info->stream, ", ");
	}

      if (oper->flags == OPERAND_ATMINUS)
	{
	  (*info->fprintf_func) (info->stream, "@-");
	  continue;
	}
      if (oper->flags == OPERAND_MINUS)
	{
	  (*info->fprintf_func) (info->stream, "-");
	  continue;
	}
      if (oper->flags == OPERAND_PLUS)
	{
	  (*info->fprintf_func) (info->stream, "+");
	  continue;
	}
      if (oper->flags == OPERAND_ATSIGN)
	{
	  (*info->fprintf_func) (info->stream, "@");
	  continue;
	}
      if (oper->flags == OPERAND_ATPAR)
	{
	  (*info->fprintf_func) (info->stream, "@(");
	  need_paren = 1;
	  continue;
	}

      if (oper->flags == OPERAND_SPECIAL)
	continue;

      val = extract_value (num, oper, is_long);

      if (oper->flags & OPERAND_REG)
	{
	  match = 0;
	  if (oper->flags & OPERAND_CONTROL)
	    {
	      const struct d30v_operand *oper3
		= &d30v_operand_table[insn->form->operands[2]];
	      int id = extract_value (num, oper3, is_long);

	      found_control = 1;
	      switch (id)
		{
		case 0:
		  val |= OPERAND_CONTROL;
		  break;
		case 1:
		case 2:
		  val = OPERAND_CONTROL + MAX_CONTROL_REG + id;
		  break;
		case 3:
		  val |= OPERAND_FLAG;
		  break;
		default:
		  /* xgettext: c-format */
		  opcodes_error_handler (_("illegal id (%d)"), id);
		  abort ();
		}
	    }
	  else if (oper->flags & OPERAND_ACC)
	    val |= OPERAND_ACC;
	  else if (oper->flags & OPERAND_FLAG)
	    val |= OPERAND_FLAG;
	  for (i = 0; i < reg_name_cnt (); i++)
	    {
	      if (val == pre_defined_registers[i].value)
		{
		  if (pre_defined_registers[i].pname)
		    (*info->fprintf_func)
		      (info->stream, "%s", pre_defined_registers[i].pname);
		  else
		    (*info->fprintf_func)
		      (info->stream, "%s", pre_defined_registers[i].name);
		  match = 1;
		  break;
		}
	    }
	  if (match == 0)
	    {
	      /* This would only get executed if a register was not in
		 the register table.  */
	      (*info->fprintf_func)
		(info->stream, _("<unknown register %d>"), val & 0x3F);
	    }
	}
      /* repeati has a relocation, but its first argument is a plain
	 immediate.  OTOH instructions like djsri have a pc-relative
	 delay target, but an absolute jump target.  Therefore, a test
	 of insn->op->reloc_flag is not specific enough; we must test
	 if the actual operand we are handling now is pc-relative.  */
      else if (oper->flags & OPERAND_PCREL)
	{
	  int neg = 0;

	  /* IMM6S3 is unsigned.  */
	  if (oper->flags & OPERAND_SIGNED || bits == 32)
	    {
	      unsigned int sign = 1u << (bits - 1);
	      if (val & sign)
		{
		  val = -val & (sign + sign - 1);
		  neg = 1;
		}
	    }
	  if (neg)
	    {
	      (*info->fprintf_func) (info->stream, "-%x\t(", val);
	      (*info->print_address_func) ((memaddr - val) & PC_MASK, info);
	      (*info->fprintf_func) (info->stream, ")");
	    }
	  else
	    {
	      (*info->fprintf_func) (info->stream, "%x\t(", val);
	      (*info->print_address_func) ((memaddr + val) & PC_MASK, info);
	      (*info->fprintf_func) (info->stream, ")");
	    }
	}
      else if (insn->op->reloc_flag == RELOC_ABS)
	{
	  (*info->print_address_func) (val, info);
	}
      else
	{
	  if (oper->flags & OPERAND_SIGNED)
	    {
	      unsigned int sign = 1u << (bits - 1);

	      if (val & sign)
		{
		  val = -val & (sign + sign - 1);
		  (*info->fprintf_func) (info->stream, "-");
		}
	    }
	  (*info->fprintf_func) (info->stream, "0x%x", val);
	}
      /* If there is another operand, then write a comma and space.  */
      if (opind < ARRAY_SIZE (insn->form->operands)
	  && insn->form->operands[opind]
	  && !(found_control && opind == 2))
	need_comma = 1;
    }
  if (need_paren)
    (*info->fprintf_func) (info->stream, ")");
}

int
print_insn_d30v (bfd_vma memaddr, struct disassemble_info *info)
{
  int status, result;
  bfd_byte buffer[12];
  uint32_t in1, in2;
  struct d30v_insn insn;
  uint64_t num;

  insn.form = NULL;

  info->bytes_per_line = 8;
  info->bytes_per_chunk = 4;
  info->display_endian = BFD_ENDIAN_BIG;

  status = (*info->read_memory_func) (memaddr, buffer, 4, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }
  in1 = bfd_getb32 (buffer);

  status = (*info->read_memory_func) (memaddr + 4, buffer, 4, info);
  if (status != 0)
    {
      info->bytes_per_line = 8;
      if (!(result = lookup_opcode (&insn, in1, 0)))
	(*info->fprintf_func) (info->stream, ".long\t0x%x", in1);
      else
	print_insn (info, memaddr, (uint64_t) in1, &insn, 0, result);
      return 4;
    }
  in2 = bfd_getb32 (buffer);

  if (in1 & in2 & FM01)
    {
      /* LONG instruction.  */
      if (!(result = lookup_opcode (&insn, in1, 1)))
	{
	  (*info->fprintf_func) (info->stream, ".long\t0x%x,0x%x", in1, in2);
	  return 8;
	}
      num = (uint64_t) in1 << 32 | in2;
      print_insn (info, memaddr, num, &insn, 1, result);
    }
  else
    {
      num = in1;
      if (!(result = lookup_opcode (&insn, in1, 0)))
	(*info->fprintf_func) (info->stream, ".long\t0x%x", in1);
      else
	print_insn (info, memaddr, num, &insn, 0, result);

      switch (((in1 >> 31) << 1) | (in2 >> 31))
	{
	case 0:
	  (*info->fprintf_func) (info->stream, "\t||\t");
	  break;
	case 1:
	  (*info->fprintf_func) (info->stream, "\t->\t");
	  break;
	case 2:
	  (*info->fprintf_func) (info->stream, "\t<-\t");
	default:
	  break;
	}

      insn.form = NULL;
      num = in2;
      if (!(result = lookup_opcode (&insn, in2, 0)))
	(*info->fprintf_func) (info->stream, ".long\t0x%x", in2);
      else
	print_insn (info, memaddr, num, &insn, 0, result);
    }
  return 8;
}
