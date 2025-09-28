/* Disassemble MSP430 instructions.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.

   Contributed by Dmitry Diky <diwil@mail.ru>

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
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>

#include "disassemble.h"
#include "opintl.h"
#include "libiberty.h"

#define DASM_SECTION
#include "opcode/msp430.h"
#undef DASM_SECTION


#define PS(x)   (0xffff & (x))

static bool
msp430dis_read_two_bytes (bfd_vma            addr,
			  disassemble_info * info,
			  bfd_byte *         buffer,
			  char *             comm)
{
  int status;

  status = info->read_memory_func (addr, buffer, 2, info);
  if (status == 0)
    return true;

  /* PR 20150: A status of EIO means that there were no more bytes left
     to read in the current section.  This can happen when disassembling
     interrupt vectors for example.  Avoid cluttering the output with
     unhelpful error messages in this case.  */
  if (status == EIO)
    {
      if (comm)
	sprintf (comm, _("Warning: disassembly unreliable - not enough bytes available"));
    }
  else
    {
      info->memory_error_func (status, addr, info);
      if (comm)
	sprintf (comm, _("Error: read from memory failed"));
    }

  return false;
}

static bool
msp430dis_opcode_unsigned (bfd_vma            addr,
			   disassemble_info * info,
			   unsigned short *   return_val,
			   char *             comm)
{
  bfd_byte buffer[2];

  if (msp430dis_read_two_bytes (addr, info, buffer, comm))
    {
      * return_val = bfd_getl16 (buffer);
      return true;
    }
  else
    {
      * return_val = 0;
      return false;
    }
}

static bool
msp430dis_opcode_signed (bfd_vma            addr,
			 disassemble_info * info,
			 signed int *       return_val,
			 char *             comm)
{
  bfd_byte buffer[2];

  if (msp430dis_read_two_bytes (addr, info, buffer, comm))
    {
      int status;

      status = bfd_getl_signed_16 (buffer);
      if (status & 0x8000)
	status |= -1U << 16;
      * return_val = status;
      return true;
    }
  else
    {
      * return_val = 0;
      return false;
    }
}

static int
msp430_nooperands (struct msp430_opcode_s *opcode,
		   bfd_vma addr ATTRIBUTE_UNUSED,
		   unsigned short insn ATTRIBUTE_UNUSED,
		   char *comm,
		   int *cycles)
{
  /* Pop with constant.  */
  if (insn == 0x43b2)
    return 0;
  if (insn == opcode->bin_opcode)
    return 2;

  if (opcode->fmt == 0)
    {
      if ((insn & 0x0f00) != 0x0300 || (insn & 0x0f00) != 0x0200)
	return 0;

      strcpy (comm, "emulated...");
      *cycles = 1;
    }
  else
    {
      strcpy (comm, "return from interupt");
      *cycles = 5;
    }

  return 2;
}

static int
print_as2_reg_name (int regno, char * op1, char * comm1,
		    int c2, int c3, int cd)
{
  switch (regno)
    {
    case 2:
      sprintf (op1, "#4");
      sprintf (comm1, "r2 As==10");
      return c2;

    case 3:
      sprintf (op1, "#2");
      sprintf (comm1, "r3 As==10");
      return c3;

    default:
      /* Indexed register mode @Rn.  */
      sprintf (op1, "@r%d", regno);
      return cd;
    }
}

static int
print_as3_reg_name (int regno, char * op1, char * comm1,
		    int c2, int c3, int cd)
{
  switch (regno)
    {
    case 2:
      sprintf (op1, "#8");
      sprintf (comm1, "r2 As==11");
      return c2;

    case 3:
      sprintf (op1, "#-1");
      sprintf (comm1, "r3 As==11");
      return c3;

    default:
      /* Post incremented @Rn+.  */
      sprintf (op1, "@r%d+", regno);
      return cd;
    }
}

static int
msp430_singleoperand (disassemble_info *info,
		      struct msp430_opcode_s *opcode,
		      bfd_vma addr,
		      unsigned short insn,
		      char *op,
		      char *comm,
		      unsigned short extension_word,
		      int *cycles)
{
  int regs = 0, regd = 0;
  int ad = 0, as = 0;
  int where = 0;
  int cmd_len = 2;
  int dst = 0;
  int fmt;
  int extended_dst = extension_word & 0xf;

  regd = insn & 0x0f;
  regs = (insn & 0x0f00) >> 8;
  as = (insn & 0x0030) >> 4;
  ad = (insn & 0x0080) >> 7;

  if (opcode->fmt < 0)
    fmt = (- opcode->fmt) - 1;
  else
    fmt = opcode->fmt;

  switch (fmt)
    {
    case 0:			/* Emulated work with dst register.  */
      if (regs != 2 && regs != 3 && regs != 1)
	return 0;

      /* Check if not clr insn.  */
      if (opcode->bin_opcode == 0x4300 && (ad || as))
	return 0;

      /* Check if really inc, incd insns.  */
      if ((opcode->bin_opcode & 0xff00) == 0x5300 && as == 3)
	return 0;

      if (ad == 0)
	{
	  *cycles = 1;

	  /* Register.  */
	  if (regd == 0)
	    {
	      *cycles += 1;
	      sprintf (op, "r0");
	    }
	  else if (regd == 1)
	    sprintf (op, "r1");

	  else if (regd == 2)
	    sprintf (op, "r2");

	  else
	    sprintf (op, "r%d", regd);
	}
      else	/* ad == 1 msp430dis_opcode.  */
	{
	  if (regd == 0)
	    {
	      /* PC relative.  */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm))
		{
		  cmd_len += 2;
		  *cycles = 4;
		  sprintf (op, "0x%04x", dst);
		  sprintf (comm, "PC rel. abs addr 0x%04x",
			   PS ((short) (addr + 2) + dst));
		  if (extended_dst)
		    {
		      dst |= extended_dst << 16;
		      sprintf (op, "0x%05x", dst);
		      sprintf (comm, "PC rel. abs addr 0x%05lx",
			       (long)((addr + 2 + dst) & 0xfffff));
		    }
		}
	      else
		return -1;
	    }
	  else if (regd == 2)
	    {
	      /* Absolute.  */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm))
		{
		  cmd_len += 2;
		  *cycles = 4;
		  sprintf (op, "&0x%04x", PS (dst));
		  if (extended_dst)
		    {
		      dst |= extended_dst << 16;
		      sprintf (op, "&0x%05x", dst & 0xfffff);
		    }
		}
	      else
		return -1;
	    }
	  else
	    {
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm))
		{
		  cmd_len += 2;
		  *cycles = 4;
		  if (extended_dst)
		    {
		      dst |= extended_dst << 16;
		      if (dst & 0x80000)
			dst |= -1U << 20;
		    }
		  sprintf (op, "%d(r%d)", dst, regd);
		}
	      else
		return -1;
	    }
	}
      break;

    case 2:	/* rrc, push, call, swpb, rra, sxt, push, call, reti etc...  */
      if (as == 0)
	{
	  if (regd == 3)
	    {
	      /* Constsnts.  */
	      sprintf (op, "#0");
	      sprintf (comm, "r3 As==00");
	    }
	  else
	    {
	      /* Register.  */
	      sprintf (op, "r%d", regd);
	    }
	  *cycles = 1;
	}
      else if (as == 2)
	{
	  * cycles = print_as2_reg_name (regd, op, comm, 1, 1, 3);
	}
      else if (as == 3)
	{
	  if (regd == 0)
	    {
	      *cycles = 3;
	      /* absolute. @pc+ */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm))
		{
		  cmd_len += 2;
		  sprintf (op, "#%d", dst);
		  if (dst > 9 || dst < 0)
		    sprintf (comm, "#0x%04x", PS (dst));
		  if (extended_dst)
		    {
		      dst |= extended_dst << 16;
		      if (dst & 0x80000)
			dst |= -1U << 20;
		      sprintf (op, "#%d", dst);
		      if (dst > 9 || dst < 0)
			sprintf (comm, "#0x%05x", dst);
		    }
		}
	      else
		return -1;
	    }
	  else
	    * cycles = print_as3_reg_name (regd, op, comm, 1, 1, 3);
	}
      else if (as == 1)
	{
	  *cycles = 4;
	  if (regd == 0)
	    {
	      /* PC relative.  */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm))
		{
		  cmd_len += 2;
		  sprintf (op, "0x%04x", PS (dst));
		  sprintf (comm, "PC rel. 0x%04x",
			   PS ((short) addr + 2 + dst));
		  if (extended_dst)
		    {
		      dst |= extended_dst << 16;
		      sprintf (op, "0x%05x", dst & 0xffff);
		      sprintf (comm, "PC rel. 0x%05lx",
			       (long)((addr + 2 + dst) & 0xfffff));
		    }
		}
	      else
		return -1;
	    }
	  else if (regd == 2)
	    {
	      /* Absolute.  */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm))
		{
		  cmd_len += 2;
		  sprintf (op, "&0x%04x", PS (dst));
		  if (extended_dst)
		    {
		      dst |= extended_dst << 16;
		      sprintf (op, "&0x%05x", dst & 0xfffff);
		    }
		}
	      else
		return -1;
	    }
	  else if (regd == 3)
	    {
	      *cycles = 1;
	      sprintf (op, "#1");
	      sprintf (comm, "r3 As==01");
	    }
	  else
	    {
	      /* Indexed.  */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm))
		{
		  cmd_len += 2;
		  if (extended_dst)
		    {
		      dst |= extended_dst << 16;
		      if (dst & 0x80000)
			dst |= -1U << 20;
		    }
		  sprintf (op, "%d(r%d)", dst, regd);
		  if (dst > 9 || dst < 0)
		    sprintf (comm, "%05x", dst);
		}
	      else
		return -1;
	    }
	}
      break;

    case 3:			/* Jumps.  */
      where = insn & 0x03ff;
      if (where & 0x200)
	where |= ~0x03ff;
      if (where > 512 || where < -511)
	return 0;

      where *= 2;
      sprintf (op, "$%+-8d", where + 2);
      sprintf (comm, "abs 0x%lx", (long) (addr + 2 + where));
      *cycles = 2;
      return 2;
      break;

    default:
      cmd_len = 0;
    }

  return cmd_len;
}

static int
msp430_doubleoperand (disassemble_info *info,
		      struct msp430_opcode_s *opcode,
		      bfd_vma addr,
		      unsigned short insn,
		      char *op1,
		      char *op2,
		      char *comm1,
		      char *comm2,
		      unsigned short extension_word,
		      int *cycles)
{
  int regs = 0, regd = 0;
  int ad = 0, as = 0;
  int cmd_len = 2;
  int dst = 0;
  int fmt;
  int extended_dst = extension_word & 0xf;
  int extended_src = (extension_word >> 7) & 0xf;

  regd = insn & 0x0f;
  regs = (insn & 0x0f00) >> 8;
  as = (insn & 0x0030) >> 4;
  ad = (insn & 0x0080) >> 7;

  if (opcode->fmt < 0)
    fmt = (- opcode->fmt) - 1;
  else
    fmt = opcode->fmt;

  if (fmt == 0)
    {
      /* Special case: rla and rlc are the only 2 emulated instructions that
	 fall into two operand instructions.  */
      /* With dst, there are only:
	 Rm       	Register,
         x(Rm)     	Indexed,
         0xXXXX    	Relative,
         &0xXXXX    	Absolute
         emulated_ins   dst
         basic_ins      dst, dst.  */

      if (regd != regs || as != ad)
	return 0;		/* May be 'data' section.  */

      if (ad == 0)
	{
	  /* Register mode.  */
	  if (regd == 3)
	    {
	      strcpy (comm1, _("Warning: illegal as emulation instr"));
	      return -1;
	    }

	  sprintf (op1, "r%d", regd);
	  *cycles = 1;
	}
      else			/* ad == 1 */
	{
	  if (regd == 0)
	    {
	      /* PC relative, Symbolic.  */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
		{
		  cmd_len += 4;
		  *cycles = 6;
		  sprintf (op1, "0x%04x", PS (dst));
		  sprintf (comm1, "PC rel. 0x%04x",
			   PS ((short) addr + 2 + dst));
		  if (extension_word)
		    {
		      dst |= extended_dst << 16;
		      if (dst & 0x80000)
			dst |= -1U << 20;
		      sprintf (op1, "0x%05x", dst & 0xfffff);
		      sprintf (comm1, "PC rel. 0x%05lx",
			       (long)((addr + 2 + dst) & 0xfffff));
		    }
		}
	      else
		return -1;
	    }
	  else if (regd == 2)
	    {
	      /* Absolute.  */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
		{
		  int src;

		  /* If the 'src' field is not the same as the dst
		     then this is not an rla instruction.  */
		  if (msp430dis_opcode_signed (addr + 4, info, &src, comm2))
		    {
		      if (src != dst)
			return 0;
		    }
		  else
		    return -1;
		  cmd_len += 4;
		  *cycles = 6;
		  sprintf (op1, "&0x%04x", PS (dst));
		  if (extension_word)
		    {
		      dst |= extended_dst << 16;
		      sprintf (op1, "&0x%05x", dst & 0xfffff);
		    }
		}
	      else
		return -1;
	    }
	  else
	    {
	      /* Indexed.  */
	      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
		{
		  if (extension_word)
		    {
		      dst |= extended_dst << 16;
		      if (dst & 0x80000)
			dst |= -1U << 20;
		    }
		  cmd_len += 4;
		  *cycles = 6;
		  sprintf (op1, "%d(r%d)", dst, regd);
		  if (dst > 9 || dst < -9)
		    sprintf (comm1, "#0x%05x", dst);
		}
	      else
		return -1;
	    }
	}

      *op2 = 0;
      *comm2 = 0;

      return cmd_len;
    }

  /* Two operands exactly.  */
  if (ad == 0 && regd == 3)
    {
      /* R2/R3 are illegal as dest: may be data section.  */
      strcpy (comm1, _("Warning: illegal as 2-op instr"));
      return -1;
    }

  /* Source.  */
  if (as == 0)
    {
      *cycles = 1;
      if (regs == 3)
	{
	  /* Constants.  */
	  sprintf (op1, "#0");
	  sprintf (comm1, "r3 As==00");
	}
      else
	{
	  /* Register.  */
	  sprintf (op1, "r%d", regs);
	}
    }
  else if (as == 2)
    {
      * cycles = print_as2_reg_name (regs, op1, comm1, 1, 1, regs == 0 ? 3 : 2);
    }
  else if (as == 3)
    {
      if (regs == 0)
	{
	  *cycles = 3;
	  /* Absolute. @pc+.  */
	  if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
	    {
	      cmd_len += 2;
	      sprintf (op1, "#%d", dst);
	      if (dst > 9 || dst < 0)
		sprintf (comm1, "#0x%04x", PS (dst));
	      if (extension_word)
		{
		  dst &= 0xffff;
		  dst |= extended_src << 16;
		  if (dst & 0x80000)
		    dst |= -1U << 20;
		  sprintf (op1, "#%d", dst);
		  if (dst > 9 || dst < 0)
		    sprintf (comm1, "0x%05x", dst & 0xfffff);
		}
	    }
	  else
	    return -1;
	}
      else
	* cycles = print_as3_reg_name (regs, op1, comm1, 1, 1, 2);
    }
  else if (as == 1)
    {
      if (regs == 0)
	{
	  *cycles = 4;
	  /* PC relative.  */
	  if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
	    {
	      cmd_len += 2;
	      sprintf (op1, "0x%04x", PS (dst));
	      sprintf (comm1, "PC rel. 0x%04x",
		       PS ((short) addr + 2 + dst));
	      if (extension_word)
		{
		  dst &= 0xffff;
		  dst |= extended_src << 16;
		  if (dst & 0x80000)
		    dst |= -1U << 20;
		  sprintf (op1, "0x%05x", dst & 0xfffff);
		  sprintf (comm1, "PC rel. 0x%05lx",
			   (long) ((addr + 2 + dst) & 0xfffff));
		}
	    }
	  else
	    return -1;
	}
      else if (regs == 2)
	{
	  *cycles = 2;
	  /* Absolute.  */
	  if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
	    {
	      cmd_len += 2;
	      sprintf (op1, "&0x%04x", PS (dst));
	      sprintf (comm1, "0x%04x", PS (dst));
	      if (extension_word)
		{
		  dst &= 0xffff;
		  dst |= extended_src << 16;
		  sprintf (op1, "&0x%05x", dst & 0xfffff);
		  * comm1 = 0;
		}
	    }
	  else
	    return -1;
	}
      else if (regs == 3)
	{
	  *cycles = 1;
	  sprintf (op1, "#1");
	  sprintf (comm1, "r3 As==01");
	}
      else
	{
	  *cycles = 3;
	  /* Indexed.  */
	  if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
	    {
	      cmd_len += 2;
	      if (extension_word)
		{
		  dst &= 0xffff;
		  dst |= extended_src << 16;
		  if (dst & 0x80000)
		    dst |= -1U << 20;
		}
	      sprintf (op1, "%d(r%d)", dst, regs);
	      if (dst > 9 || dst < -9)
		sprintf (comm1, "0x%05x", dst);
	    }
	  else
	    return -1;
	}
    }

  /* Destination. Special care needed on addr + XXXX.  */

  if (ad == 0)
    {
      /* Register.  */
      if (regd == 0)
	{
	  *cycles += 1;
	  sprintf (op2, "r0");
	}
      else if (regd == 1)
	sprintf (op2, "r1");

      else if (regd == 2)
	sprintf (op2, "r2");

      else
	sprintf (op2, "r%d", regd);
    }
  else	/* ad == 1.  */
    {
      * cycles += 3;

      if (regd == 0)
	{
	  /* PC relative.  */
	  *cycles += 1;
	  if (msp430dis_opcode_signed (addr + cmd_len, info, &dst, comm2))
	    {
	      sprintf (op2, "0x%04x", PS (dst));
	      sprintf (comm2, "PC rel. 0x%04x",
		       PS ((short) addr + cmd_len + dst));
	      if (extension_word)
		{
		  dst |= extended_dst << 16;
		  if (dst & 0x80000)
		    dst |= -1U << 20;
		  sprintf (op2, "0x%05x", dst & 0xfffff);
		  sprintf (comm2, "PC rel. 0x%05lx",
			   (long)((addr + cmd_len + dst) & 0xfffff));
		}
	    }
	  else
	    return -1;
	  cmd_len += 2;
	}
      else if (regd == 2)
	{
	  /* Absolute.  */
	  if (msp430dis_opcode_signed (addr + cmd_len, info, &dst, comm2))
	    {
	      cmd_len += 2;
	      sprintf (op2, "&0x%04x", PS (dst));
	      if (extension_word)
		{
		  dst |= extended_dst << 16;
		  sprintf (op2, "&0x%05x", dst & 0xfffff);
		}
	    }
	  else
	    return -1;
	}
      else
	{
	  if (msp430dis_opcode_signed (addr + cmd_len, info, &dst, comm2))
	    {
	      cmd_len += 2;
	      if (dst > 9 || dst < 0)
		sprintf (comm2, "0x%04x", PS (dst));
	      if (extension_word)
		{
		  dst |= extended_dst << 16;
		  if (dst & 0x80000)
		    dst |= -1U << 20;
		  if (dst > 9 || dst < 0)
		    sprintf (comm2, "0x%05x", dst & 0xfffff);
		}
	      sprintf (op2, "%d(r%d)", dst, regd);
	    }
	  else
	    return -1;
	}
    }

  return cmd_len;
}

static int
msp430_branchinstr (disassemble_info *info,
		    struct msp430_opcode_s *opcode ATTRIBUTE_UNUSED,
		    bfd_vma addr ATTRIBUTE_UNUSED,
		    unsigned short insn,
		    char *op1,
		    char *comm1,
		    int *cycles)
{
  int regs = 0, regd = 0;
  int as = 0;
  int cmd_len = 2;
  int dst = 0;
  unsigned short udst = 0;

  regd = insn & 0x0f;
  regs = (insn & 0x0f00) >> 8;
  as = (insn & 0x0030) >> 4;

  if (regd != 0)	/* Destination register is not a PC.  */
    return 0;

  /* dst is a source register.  */
  if (as == 0)
    {
      /* Constants.  */
      if (regs == 3)
	{
	  *cycles = 1;
	  sprintf (op1, "#0");
	  sprintf (comm1, "r3 As==00");
	}
      else
	{
	  /* Register.  */
	  *cycles = 1;
	  sprintf (op1, "r%d", regs);
	}
    }
  else if (as == 2)
    {
      * cycles = print_as2_reg_name (regs, op1, comm1, 2, 1, 2);
    }
  else if (as == 3)
    {
      if (regs == 0)
	{
	  /* Absolute. @pc+  */
	  *cycles = 3;
	  if (msp430dis_opcode_unsigned (addr + 2, info, &udst, comm1))
	    {
	      cmd_len += 2;
	      sprintf (op1, "#0x%04x", PS (udst));
	    }
	  else
	    return -1;
	}
      else
	* cycles = print_as3_reg_name (regs, op1, comm1, 1, 1, 2);
    }
  else if (as == 1)
    {
      * cycles = 3;

      if (regs == 0)
	{
	  /* PC relative.  */
	  if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
	    {
	      cmd_len += 2;
	      (*cycles)++;
	      sprintf (op1, "0x%04x", PS (dst));
	      sprintf (comm1, "PC rel. 0x%04x",
		       PS ((short) addr + 2 + dst));
	    }
	  else
	    return -1;
	}
      else if (regs == 2)
	{
	  /* Absolute.  */
	  if (msp430dis_opcode_unsigned (addr + 2, info, &udst, comm1))
	    {
	      cmd_len += 2;
	      sprintf (op1, "&0x%04x", PS (udst));
	    }
	  else
	    return -1;
	}
      else if (regs == 3)
	{
	  (*cycles)--;
	  sprintf (op1, "#1");
	  sprintf (comm1, "r3 As==01");
	}
      else
	{
	  /* Indexed.  */
	  if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
	    {
	      cmd_len += 2;
	      sprintf (op1, "%d(r%d)", dst, regs);
	    }
	  else
	    return -1;
	}
    }

  return cmd_len;
}

static int
msp430x_calla_instr (disassemble_info * info,
		     bfd_vma            addr,
		     unsigned short     insn,
		     char *             op1,
		     char *             comm1,
		     int *              cycles)
{
  unsigned int   ureg = insn & 0xf;
  int            reg = insn & 0xf;
  int            am = (insn & 0xf0) >> 4;
  int            cmd_len = 2;
  unsigned short udst = 0;
  int            dst = 0;

  switch (am)
    {
    case 4: /* CALLA Rdst */
      *cycles = 1;
      sprintf (op1, "r%d", reg);
      break;

    case 5: /* CALLA x(Rdst) */
      *cycles = 3;
      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
	{
	  cmd_len += 2;
	  sprintf (op1, "%d(r%d)", dst, reg);
	  if (reg == 0)
	    sprintf (comm1, "PC rel. 0x%05lx", (long) (addr + 2 + dst));
	  else
	    sprintf (comm1, "0x%05x", dst);
	}
      else
	return -1;
      break;

    case 6: /* CALLA @Rdst */
      *cycles = 2;
      sprintf (op1, "@r%d", reg);
      break;

    case 7: /* CALLA @Rdst+ */
      *cycles = 2;
      sprintf (op1, "@r%d+", reg);
      break;

    case 8: /* CALLA &abs20 */
      if (msp430dis_opcode_unsigned (addr + 2, info, &udst, comm1))
	{
	  cmd_len += 2;
	  *cycles = 4;
	  sprintf (op1, "&%d", (ureg << 16) + udst);
	  sprintf (comm1, "0x%05x", (ureg << 16) + udst);
	}
      else
	return -1;
      break;

    case 9: /* CALLA pcrel-sym */
      if (msp430dis_opcode_signed (addr + 2, info, &dst, comm1))
	{
	  cmd_len += 2;
	  *cycles = 4;
	  sprintf (op1, "%d(PC)", (reg << 16) + dst);
	  sprintf (comm1, "PC rel. 0x%05lx",
		   (long) (addr + 2 + dst + (reg << 16)));
	}
      else
	return -1;
      break;

    case 11: /* CALLA #imm20 */
      if (msp430dis_opcode_unsigned (addr + 2, info, &udst, comm1))
	{
	  cmd_len += 2;
	  *cycles = 4;
	  sprintf (op1, "#%d", (ureg << 16) + udst);
	  sprintf (comm1, "0x%05x", (ureg << 16) + udst);
	}
      else
	return -1;
      break;

    default:
      strcpy (comm1, _("Warning: unrecognised CALLA addressing mode"));
      return -1;
    }

  return cmd_len;
}

int
print_insn_msp430 (bfd_vma addr, disassemble_info *info)
{
  void *stream = info->stream;
  fprintf_ftype prin = info->fprintf_func;
  struct msp430_opcode_s *opcode;
  char op1[32], op2[32], comm1[64], comm2[64];
  int cmd_len = 0;
  unsigned short insn;
  int cycles = 0;
  char *bc = "";
  unsigned short extension_word = 0;
  unsigned short bits;

  if (! msp430dis_opcode_unsigned (addr, info, &insn, NULL))
    return -1;

  if (((int) addr & 0xffff) > 0xffdf)
    {
      (*prin) (stream, "interrupt service routine at 0x%04x", 0xffff & insn);
      return 2;
    }

  *comm1 = 0;
  *comm2 = 0;

  /* Check for an extension word.  */
  if ((insn & 0xf800) == 0x1800)
    {
      extension_word = insn;
      addr += 2;
      if (! msp430dis_opcode_unsigned (addr, info, &insn, NULL))
	return -1;
   }

  for (opcode = msp430_opcodes; opcode->name; opcode++)
    {
      if ((insn & opcode->bin_mask) == opcode->bin_opcode
	  && opcode->bin_opcode != 0x9300)
	{
	  *op1 = 0;
	  *op2 = 0;
	  *comm1 = 0;
	  *comm2 = 0;

	  /* r0 as destination. Ad should be zero.  */
	  if (opcode->insn_opnumb == 3
	      && (insn & 0x000f) == 0
	      && (insn & 0x0080) == 0)
	    {
	      int ret =
		msp430_branchinstr (info, opcode, addr, insn, op1, comm1,
				    &cycles);

	      if (ret == -1)
		return -1;
	      cmd_len += ret;
	      if (cmd_len)
		break;
	    }

	  switch (opcode->insn_opnumb)
	    {
	      int n;
	      int reg;
	      int ret;

	    case 4:
	      ret = msp430x_calla_instr (info, addr, insn,
					 op1, comm1, & cycles);
	      if (ret == -1)
		return -1;
	      cmd_len += ret;
	      break;

	    case 5: /* PUSHM/POPM */
	      n = (insn & 0xf0) >> 4;
	      reg = (insn & 0xf);

	      sprintf (op1, "#%d", n + 1);
	      if (opcode->bin_opcode == 0x1400)
		/* PUSHM */
		sprintf (op2, "r%d", reg);
	      else
		/* POPM */
		sprintf (op2, "r%d", reg + n);
	      if (insn & 0x100)
		sprintf (comm1, "16-bit words");
	      else
		{
		  sprintf (comm1, "20-bit words");
		  bc =".a";
		}

	      cycles = 2; /*FIXME*/
	      cmd_len = 2;
	      break;

	    case 6: /* RRAM, RRCM, RRUM, RLAM.  */
	      n = ((insn >> 10) & 0x3) + 1;
	      reg = (insn & 0xf);
	      if ((insn & 0x10) == 0)
		bc =".a";
	      sprintf (op1, "#%d", n);
	      sprintf (op2, "r%d", reg);
	      cycles = 2; /*FIXME*/
	      cmd_len = 2;
	      break;

	    case 8: /* ADDA, CMPA, SUBA.  */
	      reg = (insn & 0xf);
	      n = (insn >> 8) & 0xf;
	      if (insn & 0x40)
		{
		  sprintf (op1, "r%d", n);
		  cmd_len = 2;
		}
	      else
		{
		  n <<= 16;
		  if (msp430dis_opcode_unsigned (addr + 2, info, &bits, comm1))
		    {
		      n |= bits;
		      sprintf (op1, "#%d", n);
		      if (n > 9 || n < 0)
			sprintf (comm1, "0x%05x", n);
		    }
		  else
		    return -1;
		  cmd_len = 4;
		}
	      sprintf (op2, "r%d", reg);
	      cycles = 2; /*FIXME*/
	      break;

	    case 9: /* MOVA */
	      reg = (insn & 0xf);
	      n = (insn >> 8) & 0xf;
	      switch ((insn >> 4) & 0xf)
		{
		case 0: /* MOVA @Rsrc, Rdst */
		  cmd_len = 2;
		  sprintf (op1, "@r%d", n);
		  if (strcmp (opcode->name, "bra") != 0)
		    sprintf (op2, "r%d", reg);
		  break;

		case 1: /* MOVA @Rsrc+, Rdst */
		  cmd_len = 2;
		  if (strcmp (opcode->name, "reta") != 0)
		    {
		      sprintf (op1, "@r%d+", n);
		      if (strcmp (opcode->name, "bra") != 0)
			sprintf (op2, "r%d", reg);
		    }
		  break;

		case 2: /* MOVA &abs20, Rdst */
		  cmd_len = 4;
		  n <<= 16;
		  if (msp430dis_opcode_unsigned (addr + 2, info, &bits, comm1))
		    {
		      n |= bits;
		      sprintf (op1, "&%d", n);
		      if (n > 9 || n < 0)
			sprintf (comm1, "0x%05x", n);
		      if (strcmp (opcode->name, "bra") != 0)
			sprintf (op2, "r%d", reg);
		    }
		  else
		    return -1;
		  break;

		case 3: /* MOVA x(Rsrc), Rdst */
		  cmd_len = 4;
		  if (strcmp (opcode->name, "bra") != 0)
		    sprintf (op2, "r%d", reg);
		  reg = n;
		  if (msp430dis_opcode_signed (addr + 2, info, &n, comm1))
		    {
		      sprintf (op1, "%d(r%d)", n, reg);
		      if (n > 9 || n < 0)
			{
			  if (reg == 0)
			    sprintf (comm1, "PC rel. 0x%05lx",
				     (long) (addr + 2 + n));
			  else
			    sprintf (comm1, "0x%05x", n);
			}
		    }
		  else
		    return -1;
		  break;

		case 6: /* MOVA Rsrc, &abs20 */
		  cmd_len = 4;
		  reg <<= 16;
		  if (msp430dis_opcode_unsigned (addr + 2, info, &bits, comm2))
		    {
		      reg |= bits;
		      sprintf (op1, "r%d", n);
		      sprintf (op2, "&%d", reg);
		      if (reg > 9 || reg < 0)
			sprintf (comm2, "0x%05x", reg);
		    }
		  else
		    return -1;
		  break;

		case 7: /* MOVA Rsrc, x(Rdst) */
		  cmd_len = 4;
		  sprintf (op1, "r%d", n);
		  if (msp430dis_opcode_signed (addr + 2, info, &n, comm2))
		    {
		      sprintf (op2, "%d(r%d)", n, reg);
		      if (n > 9 || n < 0)
			{
			  if (reg == 0)
			    sprintf (comm2, "PC rel. 0x%05lx",
				     (long) (addr + 2 + n));
			  else
			    sprintf (comm2, "0x%05x", n);
			}
		    }
		  else
		    return -1;
		  break;

		case 8: /* MOVA #imm20, Rdst */
		  cmd_len = 4;
		  n <<= 16;
		  if (msp430dis_opcode_unsigned (addr + 2, info, &bits, comm1))
		    {
		      n |= bits;
		      if (n & 0x80000)
			n |= -1U << 20;
		      sprintf (op1, "#%d", n);
		      if (n > 9 || n < 0)
			sprintf (comm1, "0x%05x", n);
		      if (strcmp (opcode->name, "bra") != 0)
			sprintf (op2, "r%d", reg);
		    }
		  else
		    return -1;
		  break;

		case 12: /* MOVA Rsrc, Rdst */
		  cmd_len = 2;
		  sprintf (op1, "r%d", n);
		  if (strcmp (opcode->name, "bra") != 0)
		    sprintf (op2, "r%d", reg);
		  break;

		default:
		  break;
		}
	      cycles = 2; /* FIXME */
	      break;
	    }

	  if (cmd_len)
	    break;

	  switch (opcode->insn_opnumb)
	    {
	      int ret;

	    case 0:
	      cmd_len += msp430_nooperands (opcode, addr, insn, comm1, &cycles);
	      break;
	    case 2:
	      ret =
		msp430_doubleoperand (info, opcode, addr, insn, op1, op2,
				      comm1, comm2,
				      extension_word,
				      &cycles);

	      if (ret == -1)
		return -1;
	      cmd_len += ret;
	      if (insn & BYTE_OPERATION)
		{
		  if (extension_word != 0 && ((extension_word & BYTE_OPERATION) == 0))
		    bc = ".a";
		  else
		    bc = ".b";
		}
	      else if (extension_word)
		{
		  if (extension_word & BYTE_OPERATION)
		    bc = ".w";
		  else
		    {
		      bc = ".?";
		      sprintf (comm2, _("Warning: reserved use of A/L and B/W bits detected"));
		    }
		}

	      break;
	    case 1:
	      ret =
		msp430_singleoperand (info, opcode, addr, insn, op1, comm1,
				      extension_word,
				      &cycles);

	      if (ret == -1)
		return -1;
	      cmd_len += ret;
	      if (extension_word
		  && (strcmp (opcode->name, "swpb") == 0
		      || strcmp (opcode->name, "sxt") == 0))
		{
		  if (insn & BYTE_OPERATION)
		    {
		      bc = ".?";
		      sprintf (comm2, _("Warning: reserved use of A/L and B/W bits detected"));
		    }
		  else if (extension_word & BYTE_OPERATION)
		    bc = ".w";
		  else
		    bc = ".a";
		}
	      else if (insn & BYTE_OPERATION && opcode->fmt != 3)
		{
		  if (extension_word != 0 && ((extension_word & BYTE_OPERATION) == 0))
		    bc = ".a";
		  else
		    bc = ".b";
		}
	      else if (extension_word)
		{
		  if (extension_word & (1 << 6))
		    bc = ".w";
		  else
		    {
		      bc = ".?";
		      sprintf (comm2, _("Warning: reserved use of A/L and B/W bits detected"));
		    }
		}
	      break;
	    default:
	      break;
	    }
	}

      if (cmd_len)
	break;
    }

  if (cmd_len < 1)
    {
      /* Unknown opcode, or invalid combination of operands.  */
      if (extension_word)
	{
	  prin (stream, ".word	0x%04x, 0x%04x;	????", extension_word, PS (insn));
	  if (*comm1)
	    prin (stream, "\t %s", comm1);
	  return 4;
	}
      (*prin) (stream, ".word	0x%04x;	????", PS (insn));
      return 2;
    }

  /* Display the repeat count (if set) for extended register mode.  */
  if (cmd_len == 2 && ((extension_word & 0xf) != 0))
    {
      if (extension_word & (1 << 7))
	prin (stream, "rpt r%d { ", extension_word & 0xf);
      else
	prin (stream, "rpt #%d { ", (extension_word & 0xf) + 1);
    }

  /* Special case:  RRC with an extension word and the ZC bit set is actually RRU.  */
  if (extension_word
      && (extension_word & IGNORE_CARRY_BIT)
      && strcmp (opcode->name, "rrc") == 0)
    (*prin) (stream, "rrux%s", bc);
  else if (extension_word && opcode->name[strlen (opcode->name) - 1] != 'x')
    (*prin) (stream, "%sx%s", opcode->name, bc);
  else
    (*prin) (stream, "%s%s", opcode->name, bc);

  if (*op1)
    (*prin) (stream, "\t%s", op1);
  if (*op2)
    (*prin) (stream, ",");

  if (strlen (op1) < 7)
    (*prin) (stream, "\t");
  if (!strlen (op1))
    (*prin) (stream, "\t");

  if (*op2)
    (*prin) (stream, "%s", op2);
  if (strlen (op2) < 8)
    (*prin) (stream, "\t");

  if (*comm1 || *comm2)
    (*prin) (stream, ";");
  else if (cycles)
    {
      if (*op2)
	(*prin) (stream, ";");
      else
	{
	  if (strlen (op1) < 7)
	    (*prin) (stream, ";");
	  else
	    (*prin) (stream, "\t;");
	}
    }
  if (*comm1)
    (*prin) (stream, "%s", comm1);
  if (*comm1 && *comm2)
    (*prin) (stream, ",");
  if (*comm2)
    (*prin) (stream, " %s", comm2);

  if (extension_word)
    cmd_len += 2;

  return cmd_len;
}
