/* Disassembler code for Renesas RL78.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Contributed by Red Hat.
   Written by DJ Delorie.

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

#include "bfd.h"
#include "elf-bfd.h"
#include "disassemble.h"
#include "opcode/rl78.h"
#include "elf/rl78.h"

#include <setjmp.h>

#define DEBUG_SEMANTICS 0

typedef struct
{
  bfd_vma pc;
  disassemble_info * dis;
} RL78_Data;

struct private
{
  OPCODES_SIGJMP_BUF bailout;
};

static int
rl78_get_byte (void * vdata)
{
  bfd_byte buf[1];
  RL78_Data *rl78_data = (RL78_Data *) vdata;
  int status;

  status = rl78_data->dis->read_memory_func (rl78_data->pc,
					     buf,
					     1,
					     rl78_data->dis);
  if (status != 0)
    {
      struct private *priv = (struct private *) rl78_data->dis->private_data;

      rl78_data->dis->memory_error_func (status, rl78_data->pc,
					 rl78_data->dis);
      OPCODES_SIGLONGJMP (priv->bailout, 1);
    }

  rl78_data->pc ++;
  return buf[0];
}

static char const *
register_names[] =
{
  "",
  "x", "a", "c", "b", "e", "d", "l", "h",
  "ax", "bc", "de", "hl",
  "sp", "psw", "cs", "es", "pmc", "mem"
};

static char const *
condition_names[] =
{
  "t", "f", "c", "nc", "h", "nh", "z", "nz"
};

static int
indirect_type (int t)
{
  switch (t)
    {
    case RL78_Operand_Indirect:
    case RL78_Operand_BitIndirect:
    case RL78_Operand_PostInc:
    case RL78_Operand_PreDec:
      return 1;
    default:
      return 0;
    }
}

static int
print_insn_rl78_common (bfd_vma addr, disassemble_info * dis, RL78_Dis_Isa isa)
{
  int rv;
  RL78_Data rl78_data;
  RL78_Opcode_Decoded opcode;
  const char * s;
#if DEBUG_SEMANTICS
  static char buf[200];
#endif
  struct private priv;

  dis->private_data = &priv;
  rl78_data.pc = addr;
  rl78_data.dis = dis;

  if (OPCODES_SIGSETJMP (priv.bailout) != 0)
    {
      /* Error return.  */
      return -1;
    }

  rv = rl78_decode_opcode (addr, &opcode, rl78_get_byte, &rl78_data, isa);

  dis->bytes_per_line = 10;

#define PR (dis->fprintf_func)
#define PS (dis->stream)
#define PC(c) PR (PS, "%c", c)

  s = opcode.syntax;

#if DEBUG_SEMANTICS

  switch (opcode.id)
    {
    case RLO_unknown: s = "uknown"; break;
    case RLO_add: s = "add: %e0%0 += %e1%1"; break;
    case RLO_addc: s = "addc: %e0%0 += %e1%1 + CY"; break;
    case RLO_and: s = "and: %e0%0 &= %e1%1"; break;
    case RLO_branch: s = "branch: pc = %e0%0"; break;
    case RLO_branch_cond: s = "branch_cond: pc = %e0%0 if %c1 / %e1%1"; break;
    case RLO_branch_cond_clear: s = "branch_cond_clear: pc = %e0%0 if %c1 / %e1%1, %e1%1 = 0"; break;
    case RLO_call: s = "call: pc = %e1%0"; break;
    case RLO_cmp: s = "cmp: %e0%0 - %e1%1"; break;
    case RLO_mov: s = "mov: %e0%0 = %e1%1"; break;
    case RLO_or: s = "or: %e0%0 |= %e1%1"; break;
    case RLO_rol: s = "rol: %e0%0 <<= %e1%1"; break;
    case RLO_rolc: s = "rol: %e0%0 <<= %e1%1,CY"; break;
    case RLO_ror: s = "ror: %e0%0 >>= %e1%1"; break;
    case RLO_rorc: s = "ror: %e0%0 >>= %e1%1,CY"; break;
    case RLO_sar: s = "sar: %e0%0 >>= %e1%1 signed"; break;
    case RLO_sel: s = "sel: rb = %1"; break;
    case RLO_shr: s = "shr: %e0%0 >>= %e1%1 unsigned"; break;
    case RLO_shl: s = "shl: %e0%0 <<= %e1%1"; break;
    case RLO_skip: s = "skip: if %c1"; break;
    case RLO_sub: s = "sub: %e0%0 -= %e1%1"; break;
    case RLO_subc: s = "subc: %e0%0 -= %e1%1 - CY"; break;
    case RLO_xch: s = "xch: %e0%0 <-> %e1%1"; break;
    case RLO_xor: s = "xor: %e0%0 ^= %e1%1"; break;
    }

  sprintf(buf, "%s%%W%%f\t\033[32m%s\033[0m", s, opcode.syntax);
  s = buf;

#endif

  for (; *s; s++)
    {
      if (*s != '%')
	{
	  PC (*s);
	}
      else
	{
	  RL78_Opcode_Operand * oper;
	  int do_hex = 0;
	  int do_addr = 0;
	  int do_es = 0;
	  int do_sfr = 0;
	  int do_cond = 0;
	  int do_bang = 0;

	  while (1)
	    {
	      s ++;
	      switch (*s)
		{
		case 'x':
		  do_hex = 1;
		  break;
		case '!':
		  do_bang = 1;
		  break;
		case 'e':
		  do_es = 1;
		  break;
		case 'a':
		  do_addr = 1;
		  break;
		case 's':
		  do_sfr = 1;
		  break;
		case 'c':
		  do_cond = 1;
		  break;
		default:
		  goto no_more_modifiers;
		}
	    }
	no_more_modifiers:;

	  switch (*s)
	    {
	    case '%':
	      PC ('%');
	      break;

#if DEBUG_SEMANTICS

	    case 'W':
	      if (opcode.size == RL78_Word)
		PR (PS, " \033[33mW\033[0m");
	      break;

	    case 'f':
	      if (opcode.flags)
		{
		  char *comma = "";
		  PR (PS, "  \033[35m");

		  if (opcode.flags & RL78_PSW_Z)
		    { PR (PS, "Z"); comma = ","; }
		  if (opcode.flags & RL78_PSW_AC)
		    { PR (PS, "%sAC", comma); comma = ","; }
		  if (opcode.flags & RL78_PSW_CY)
		    { PR (PS, "%sCY", comma); comma = ","; }
		  PR (PS, "\033[0m");
		}
	      break;

#endif

	    case '0':
	    case '1':
	      oper = *s == '0' ? &opcode.op[0] : &opcode.op[1];
	    if (do_es)
	      {
		if (oper->use_es && indirect_type (oper->type))
		  PR (PS, "es:");
	      }

	    if (do_bang)
	      {
		/* If we are going to display SP by name, we must omit the bang.  */
		if ((oper->type == RL78_Operand_Indirect
		     || oper->type == RL78_Operand_BitIndirect)
		    && oper->reg == RL78_Reg_None
		    && do_sfr
		    && ((oper->addend == 0xffff8 && opcode.size == RL78_Word)
			|| (oper->addend == 0x0fff8 && do_es && opcode.size == RL78_Word)))
		  ;
		else
		  PC ('!');
	      }

	    if (do_cond)
	      {
		PR (PS, "%s", condition_names[oper->condition]);
		break;
	      }

	    switch (oper->type)
	      {
	      case RL78_Operand_Immediate:
		if (do_addr)
		  dis->print_address_func (oper->addend, dis);
		else if (do_hex
			 || oper->addend > 999
			 || oper->addend < -999)
		  PR (PS, "%#x", oper->addend);
		else
		  PR (PS, "%d", oper->addend);
		break;

	      case RL78_Operand_Register:
		PR (PS, "%s", register_names[oper->reg]);
		break;

	      case RL78_Operand_Bit:
		PR (PS, "%s.%d", register_names[oper->reg], oper->bit_number);
		break;

	      case RL78_Operand_Indirect:
	      case RL78_Operand_BitIndirect:
		switch (oper->reg)
		  {
		  case RL78_Reg_None:
		    if (oper->addend == 0xffffa && do_sfr && opcode.size == RL78_Byte)
		      PR (PS, "psw");
		    else if (oper->addend == 0xffff8 && do_sfr && opcode.size == RL78_Word)
		      PR (PS, "sp");
		    else if (oper->addend == 0x0fff8 && do_sfr && do_es && opcode.size == RL78_Word)
		      PR (PS, "sp");
                    else if (oper->addend == 0xffff8 && do_sfr && opcode.size == RL78_Byte)
                      PR (PS, "spl");
                    else if (oper->addend == 0xffff9 && do_sfr && opcode.size == RL78_Byte)
                      PR (PS, "sph");
                    else if (oper->addend == 0xffffc && do_sfr && opcode.size == RL78_Byte)
                      PR (PS, "cs");
                    else if (oper->addend == 0xffffd && do_sfr && opcode.size == RL78_Byte)
                      PR (PS, "es");
                    else if (oper->addend == 0xffffe && do_sfr && opcode.size == RL78_Byte)
                      PR (PS, "pmc");
                    else if (oper->addend == 0xfffff && do_sfr && opcode.size == RL78_Byte)
                      PR (PS, "mem");
		    else if (oper->addend >= 0xffe20)
		      PR (PS, "%#x", oper->addend);
		    else
		      {
			int faddr = oper->addend;
			if (do_es && ! oper->use_es)
			  faddr += 0xf0000;
			dis->print_address_func (faddr, dis);
		      }
		    break;

		  case RL78_Reg_B:
		  case RL78_Reg_C:
		  case RL78_Reg_BC:
		    PR (PS, "%d[%s]", oper->addend, register_names[oper->reg]);
		    break;

		  default:
		    PR (PS, "[%s", register_names[oper->reg]);
		    if (oper->reg2 != RL78_Reg_None)
		      PR (PS, "+%s", register_names[oper->reg2]);
		    if (oper->addend || do_addr)
		      PR (PS, "+%d", oper->addend);
		    PC (']');
		    break;

		  }
		if (oper->type == RL78_Operand_BitIndirect)
		  PR (PS, ".%d", oper->bit_number);
		break;

#if DEBUG_SEMANTICS
		/* Shouldn't happen - push and pop don't print
		   [SP] directly.  But we *do* use them for
		   semantic debugging.  */
	      case RL78_Operand_PostInc:
		PR (PS, "[%s++]", register_names[oper->reg]);
		break;
	      case RL78_Operand_PreDec:
		PR (PS, "[--%s]", register_names[oper->reg]);
		break;
#endif

	      default:
		/* If we ever print this, that means the
		   programmer tried to print an operand with a
		   type we don't expect.  Print the line and
		   operand number from rl78-decode.opc for
		   them.  */
		PR (PS, "???%d.%d", opcode.lineno, *s - '0');
		break;
	      }
	    }
	}
    }

#if DEBUG_SEMANTICS

  PR (PS, "\t\033[34m(line %d)\033[0m", opcode.lineno);

#endif

  return rv;
}

int
print_insn_rl78 (bfd_vma addr, disassemble_info * dis)
{
  return print_insn_rl78_common (addr, dis, RL78_ISA_DEFAULT);
}

int
print_insn_rl78_g10 (bfd_vma addr, disassemble_info * dis)
{
  return print_insn_rl78_common (addr, dis, RL78_ISA_G10);
}

int
print_insn_rl78_g13 (bfd_vma addr, disassemble_info * dis)
{
  return print_insn_rl78_common (addr, dis, RL78_ISA_G13);
}

int
print_insn_rl78_g14 (bfd_vma addr, disassemble_info * dis)
{
  return print_insn_rl78_common (addr, dis, RL78_ISA_G14);
}

disassembler_ftype
rl78_get_disassembler (bfd *abfd)
{
  int cpu = E_FLAG_RL78_ANY_CPU;

  if (abfd != NULL && bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    cpu = abfd->tdata.elf_obj_data->elf_header->e_flags & E_FLAG_RL78_CPU_MASK;

  switch (cpu)
    {
    case E_FLAG_RL78_G10:
      return print_insn_rl78_g10;
    case E_FLAG_RL78_G13:
      return print_insn_rl78_g13;
    case E_FLAG_RL78_G14:
      return print_insn_rl78_g14;
    default:
      return print_insn_rl78;
    }
}
