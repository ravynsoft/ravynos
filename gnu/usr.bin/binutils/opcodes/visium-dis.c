/* Single instruction disassembler for the Visium.

   Copyright (C) 2002-2023 Free Software Foundation, Inc.

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
#include "disassemble.h"
#include "opcode/visium.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>

/* Maximum length of an instruction.  */
#define MAXLEN 4

struct private
{
  /* Points to first byte not fetched.  */
  bfd_byte *max_fetched;
  bfd_byte the_buffer[MAXLEN];
  bfd_vma insn_start;
  jmp_buf bailout;
};

/* Make sure that bytes from INFO->PRIVATE_DATA->BUFFER (inclusive)
   to ADDR (exclusive) are valid.  Returns 1 for success, longjmps
   on error.  */
#define FETCH_DATA(info, addr) \
  ((addr) <= ((struct private *)(info->private_data))->max_fetched \
   ? 1 : fetch_data ((info), (addr)))

static int fetch_data (struct disassemble_info *info, bfd_byte * addr);

static int
fetch_data (struct disassemble_info *info, bfd_byte *addr)
{
  int status;
  struct private *priv = (struct private *) info->private_data;
  bfd_vma start = priv->insn_start + (priv->max_fetched - priv->the_buffer);

  status = (*info->read_memory_func) (start,
				      priv->max_fetched,
				      addr - priv->max_fetched, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, start, info);
      longjmp (priv->bailout, 1);
    }
  else
    priv->max_fetched = addr;
  return 1;
}

static char *size_names[] = { "?", "b", "w", "?", "l", "?", "?", "?" };

static char *cc_names[] =
{
  "fa", "eq", "cs", "os", "ns", "ne", "cc", "oc",
  "nc", "ge", "gt", "hi", "le", "ls", "lt", "tr"
};

/* Disassemble non-storage relative instructions.  */

static int
disassem_class0 (disassemble_info *info, unsigned int ins)
{
  int opcode = (ins >> 21) & 0x000f;

  if (ins & CLASS0_UNUSED_MASK)
    goto illegal_opcode;

  switch (opcode)
    {
    case 0:
      /* BRR instruction.  */
      {
	unsigned cbf = (ins >> 27) & 0x000f;
	int displacement = ((ins & 0xffff) ^ 0x8000) - 0x8000;

	if (ins == 0)
	  (*info->fprintf_func) (info->stream, "nop");
	else
	  (*info->fprintf_func) (info->stream, "brr     %s,%+d",
				 cc_names[cbf], displacement);
      }
      break;
    case 1:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 2:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 3:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 4:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 5:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 6:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 7:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 8:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 9:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 10:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 11:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 12:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 13:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 14:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 15:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    }
  return 0;

 illegal_opcode:
  return -1;
}

/* Disassemble non-storage register class instructions.   */

static int
disassem_class1 (disassemble_info *info, unsigned int ins)
{
  int opcode = (ins >> 21) & 0xf;
  int source_a = (ins >> 16) & 0x1f;
  int source_b = (ins >> 4) & 0x1f;
  int indx = (ins >> 10) & 0x1f;

  int size = ins & 0x7;

  if (ins & CLASS1_UNUSED_MASK)
    goto illegal_opcode;

  switch (opcode)
    {
    case 0:
      /* Stop.  */
      (*info->fprintf_func) (info->stream, "stop    %d,r%d", indx, source_a);
      break;
    case 1:
      /* BMI - Block Move Indirect.  */
      if (ins != BMI)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "bmi     r1,r2,r3");
      break;
    case 2:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 3:
      /* BMD - Block Move Direct.  */
      if (ins != BMD)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "bmd     r1,r2,r3");
      break;
    case 4:
      /* DSI - Disable Interrupts.  */
      if (ins != DSI)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "dsi");
      break;

    case 5:
      /* ENI - Enable Interrupts.  */
      if (ins != ENI)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "eni");
      break;

    case 6:
      /* Illegal opcode (was EUT).  */
      goto illegal_opcode;
      break;
    case 7:
      /* RFI - Return from Interrupt.  */
      if (ins != RFI)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "rfi");
      break;
    case 8:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 9:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 10:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 11:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 12:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 13:
      goto illegal_opcode;
      break;
    case 14:
      goto illegal_opcode;
      break;
    case 15:
      if (ins & EAM_SELECT_MASK)
	{
	  /* Extension arithmetic module write */
	  int fp_ins = (ins >> 27) & 0xf;

	  if (size != 4)
	    goto illegal_opcode;

	  if (ins & FP_SELECT_MASK)
	    {
	      /* Which floating point instructions don't need a fsrcB
	         register.  */
	      const int no_fsrcb[16] = { 1, 0, 0, 0, 0, 1, 1, 1,
		1, 1, 0, 0, 1, 0, 0, 0
	      };
	      if (no_fsrcb[fp_ins] && source_b)
		goto illegal_opcode;

	      /* Check that none of the floating register register numbers
	         is higher than 15. (If this is fload, then srcA is a
	         general register.  */
	      if (ins & ((1 << 14) | (1 << 8)) || (fp_ins && ins & (1 << 20)))
		goto illegal_opcode;

	      switch (fp_ins)
		{
		case 0:
		  (*info->fprintf_func) (info->stream, "fload   f%d,r%d",
					 indx, source_a);
		  break;
		case 1:
		  (*info->fprintf_func) (info->stream, "fadd    f%d,f%d,f%d",
					 indx, source_a, source_b);
		  break;
		case 2:
		  (*info->fprintf_func) (info->stream, "fsub    f%d,f%d,f%d",
					 indx, source_a, source_b);
		  break;
		case 3:
		  (*info->fprintf_func) (info->stream, "fmult   f%d,f%d,f%d",
					 indx, source_a, source_b);
		  break;
		case 4:
		  (*info->fprintf_func) (info->stream, "fdiv    f%d,f%d,f%d",
					 indx, source_a, source_b);
		  break;
		case 5:
		  (*info->fprintf_func) (info->stream, "fsqrt   f%d,f%d",
					 indx, source_a);
		  break;
		case 6:
		  (*info->fprintf_func) (info->stream, "fneg    f%d,f%d",
					 indx, source_a);
		  break;
		case 7:
		  (*info->fprintf_func) (info->stream, "fabs    f%d,f%d",
					 indx, source_a);
		  break;
		case 8:
		  (*info->fprintf_func) (info->stream, "ftoi    f%d,f%d",
					 indx, source_a);
		  break;
		case 9:
		  (*info->fprintf_func) (info->stream, "itof    f%d,f%d",
					 indx, source_a);
		  break;
		case 12:
		  (*info->fprintf_func) (info->stream, "fmove   f%d,f%d",
					 indx, source_a);
		  break;
		default:
		  (*info->fprintf_func) (info->stream,
					 "fpinst  %d,f%d,f%d,f%d", fp_ins,
					 indx, source_a, source_b);
		  break;
		}
	    }
	  else
	    {
	      /* Which EAM operations do not need a srcB register.  */
	      const int no_srcb[32] =
	      { 0, 0, 1, 1, 0, 1, 1, 1,
		0, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	      };

	      if (no_srcb[indx] && source_b)
		goto illegal_opcode;

	      if (fp_ins)
		goto illegal_opcode;

	      switch (indx)
		{
		case 0:
		  (*info->fprintf_func) (info->stream, "mults   r%d,r%d",
					 source_a, source_b);
		  break;
		case 1:
		  (*info->fprintf_func) (info->stream, "multu   r%d,r%d",
					 source_a, source_b);
		  break;
		case 2:
		  (*info->fprintf_func) (info->stream, "divs    r%d",
					 source_a);
		  break;
		case 3:
		  (*info->fprintf_func) (info->stream, "divu    r%d",
					 source_a);
		  break;
		case 4:
		  (*info->fprintf_func) (info->stream, "writemd r%d,r%d",
					 source_a, source_b);
		  break;
		case 5:
		  (*info->fprintf_func) (info->stream, "writemdc r%d",
					 source_a);
		  break;
		case 6:
		  (*info->fprintf_func) (info->stream, "divds   r%d",
					 source_a);
		  break;
		case 7:
		  (*info->fprintf_func) (info->stream, "divdu   r%d",
					 source_a);
		  break;
		case 9:
		  (*info->fprintf_func) (info->stream, "asrd    r%d",
					 source_a);
		  break;
		case 10:
		  (*info->fprintf_func) (info->stream, "lsrd    r%d",
					 source_a);
		  break;
		case 11:
		  (*info->fprintf_func) (info->stream, "asld    r%d",
					 source_a);
		  break;
		default:
		  (*info->fprintf_func) (info->stream,
					 "eamwrite %d,r%d,r%d", indx,
					 source_a, source_b);
		  break;
		}
	    }
	}
      else
	{
	  /* WRITE - write to memory.  */
	  (*info->fprintf_func) (info->stream, "write.%s %d(r%d),r%d",
				 size_names[size], indx, source_a, source_b);
	}
      break;
    }

  return 0;

 illegal_opcode:
  return -1;
}

/* Disassemble storage immediate class instructions.   */

static int
disassem_class2 (disassemble_info *info, unsigned int ins)
{
  int opcode = (ins >> 21) & 0xf;
  int source_a = (ins >> 16) & 0x1f;
  unsigned immediate = ins & 0x0000ffff;

  if (ins & CC_MASK)
    goto illegal_opcode;

  switch (opcode)
    {
    case 0:
      /* ADDI instruction.  */
      (*info->fprintf_func) (info->stream, "addi    r%d,%d", source_a,
			     immediate);
      break;
    case 1:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 2:
      /* SUBI instruction.  */
      (*info->fprintf_func) (info->stream, "subi    r%d,%d", source_a,
			     immediate);
      break;
    case 3:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 4:
      /* MOVIL instruction.  */
      (*info->fprintf_func) (info->stream, "movil   r%d,0x%04X", source_a,
			     immediate);
      break;
    case 5:
      /* MOVIU instruction.  */
      (*info->fprintf_func) (info->stream, "moviu   r%d,0x%04X", source_a,
			     immediate);
      break;
    case 6:
      /* MOVIQ instruction.  */
      (*info->fprintf_func) (info->stream, "moviq   r%d,%u", source_a,
			     immediate);
      break;
    case 7:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 8:
      /* WRTL instruction.  */
      if (source_a != 0)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "wrtl    0x%04X", immediate);
      break;
    case 9:
      /* WRTU instruction.  */
      if (source_a != 0)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "wrtu    0x%04X", immediate);
      break;
    case 10:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 11:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 12:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 13:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 14:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    case 15:
      /* Illegal opcode.  */
      goto illegal_opcode;
      break;
    }

  return 0;

 illegal_opcode:
  return -1;
}

/* Disassemble storage register class instructions.  */

static int
disassem_class3 (disassemble_info *info, unsigned int ins)
{
  int opcode = (ins >> 21) & 0xf;
  int source_b = (ins >> 4) & 0x1f;
  int source_a = (ins >> 16) & 0x1f;
  int size = ins & 0x7;
  int dest = (ins >> 10) & 0x1f;

  /* Those instructions that don't have a srcB register.  */
  const int no_srcb[16] =
  { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0 };

  /* These are instructions which can take an immediate srcB value.  */
  const int srcb_immed[16] =
  { 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 };

  /* User opcodes should not provide a non-zero srcB register
     when none is required. Only a BRA or floating point
     instruction should have a non-zero condition code field.
     Only a WRITE or EAMWRITE (opcode 15) should select an EAM
     or floating point operation.  Note that FP_SELECT_MASK is
     the same bit (bit 3) as the interrupt bit which
     distinguishes SYS1 from BRA and SYS2 from RFLAG.  */
  if ((no_srcb[opcode] && source_b)
      || (!srcb_immed[opcode] && ins & CLASS3_SOURCEB_IMMED)
      || (opcode != 12 && opcode != 15 && ins & CC_MASK)
      || (opcode != 15 && ins & (EAM_SELECT_MASK | FP_SELECT_MASK)))
    goto illegal_opcode;


  switch (opcode)
    {
    case 0:
      /* ADD instruction.  */
      (*info->fprintf_func) (info->stream, "add.%s   r%d,r%d,r%d",
			     size_names[size], dest, source_a, source_b);
      break;
    case 1:
      /* ADC instruction.  */
      (*info->fprintf_func) (info->stream, "adc.%s   r%d,r%d,r%d",
			     size_names[size], dest, source_a, source_b);
      break;
    case 2:
      /* SUB instruction.  */
      if (dest == 0)
	(*info->fprintf_func) (info->stream, "cmp.%s   r%d,r%d",
			       size_names[size], source_a, source_b);
      else
	(*info->fprintf_func) (info->stream, "sub.%s   r%d,r%d,r%d",
			       size_names[size], dest, source_a, source_b);
      break;
    case 3:
      /* SUBC instruction.  */
      if (dest == 0)
	(*info->fprintf_func) (info->stream, "cmpc.%s  r%d,r%d",
			       size_names[size], source_a, source_b);
      else
	(*info->fprintf_func) (info->stream, "subc.%s  r%d,r%d,r%d",
			       size_names[size], dest, source_a, source_b);
      break;
    case 4:
      /* EXTW instruction.  */
      if (size == 1)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "extw.%s  r%d,r%d",
			     size_names[size], dest, source_a);
      break;
    case 5:
      /* ASR instruction.  */
      if (ins & CLASS3_SOURCEB_IMMED)
	(*info->fprintf_func) (info->stream, "asr.%s   r%d,r%d,%d",
			       size_names[size], dest, source_a, source_b);
      else
	(*info->fprintf_func) (info->stream, "asr.%s   r%d,r%d,r%d",
			       size_names[size], dest, source_a, source_b);
      break;
    case 6:
      /* LSR instruction.  */
      if (ins & CLASS3_SOURCEB_IMMED)
	(*info->fprintf_func) (info->stream, "lsr.%s   r%d,r%d,%d",
			       size_names[size], dest, source_a, source_b);
      else
	(*info->fprintf_func) (info->stream, "lsr.%s   r%d,r%d,r%d",
			       size_names[size], dest, source_a, source_b);
      break;
    case 7:
      /* ASL instruction.  */
      if (ins & CLASS3_SOURCEB_IMMED)
	(*info->fprintf_func) (info->stream, "asl.%s   r%d,r%d,%d",
			       size_names[size], dest, source_a, source_b);
      else
	(*info->fprintf_func) (info->stream, "asl.%s   r%d,r%d,r%d",
			       size_names[size], dest, source_a, source_b);
      break;
    case 8:
      /* XOR instruction.  */
      (*info->fprintf_func) (info->stream, "xor.%s   r%d,r%d,r%d",
			     size_names[size], dest, source_a, source_b);
      break;
    case 9:
      /* OR instruction.  */
      if (source_b == 0)
	(*info->fprintf_func) (info->stream, "move.%s  r%d,r%d",
			       size_names[size], dest, source_a);
      else
	(*info->fprintf_func) (info->stream, "or.%s    r%d,r%d,r%d",
			       size_names[size], dest, source_a, source_b);
      break;
    case 10:
      /* AND instruction.  */
      (*info->fprintf_func) (info->stream, "and.%s   r%d,r%d,r%d",
			     size_names[size], dest, source_a, source_b);
      break;
    case 11:
      /* NOT instruction.  */
      (*info->fprintf_func) (info->stream, "not.%s   r%d,r%d",
			     size_names[size], dest, source_a);
      break;
    case 12:
      /* BRA instruction.  */
      {
	unsigned cbf = (ins >> 27) & 0x000f;

	if (size != 4)
	  goto illegal_opcode;

	(*info->fprintf_func) (info->stream, "bra     %s,r%d,r%d",
			       cc_names[cbf], source_a, dest);
      }
      break;
    case 13:
      /* RFLAG instruction.  */
      if (source_a || size != 4)
	goto illegal_opcode;

      (*info->fprintf_func) (info->stream, "rflag   r%d", dest);
      break;
    case 14:
      /* EXTB instruction.  */
      (*info->fprintf_func) (info->stream, "extb.%s  r%d,r%d",
			     size_names[size], dest, source_a);
      break;
    case 15:
      if (!(ins & CLASS3_SOURCEB_IMMED))
	goto illegal_opcode;

      if (ins & EAM_SELECT_MASK)
	{
	  /* Extension arithmetic module read.  */
	  int fp_ins = (ins >> 27) & 0xf;

	  if (size != 4)
	    goto illegal_opcode;

	  if (ins & FP_SELECT_MASK)
	    {
	      /* Check fsrcA <= 15 and fsrcB <= 15.  */
	      if (ins & ((1 << 20) | (1 << 8)))
		goto illegal_opcode;

	      switch (fp_ins)
		{
		case 0:
		  if (source_b)
		    goto illegal_opcode;

		  (*info->fprintf_func) (info->stream, "fstore  r%d,f%d",
					 dest, source_a);
		  break;
		case 10:
		  (*info->fprintf_func) (info->stream, "fcmp    r%d,f%d,f%d",
					 dest, source_a, source_b);
		  break;
		case 11:
		  (*info->fprintf_func) (info->stream, "fcmpe   r%d,f%d,f%d",
					 dest, source_a, source_b);
		  break;
		default:
		  (*info->fprintf_func) (info->stream,
					 "fpuread %d,r%d,f%d,f%d", fp_ins,
					 dest, source_a, source_b);
		  break;
		}
	    }
	  else
	    {
	      if (fp_ins || source_a)
		goto illegal_opcode;

	      switch (source_b)
		{
		case 0:
		  (*info->fprintf_func) (info->stream, "readmda r%d", dest);
		  break;
		case 1:
		  (*info->fprintf_func) (info->stream, "readmdb r%d", dest);
		  break;
		case 2:
		  (*info->fprintf_func) (info->stream, "readmdc r%d", dest);
		  break;
		default:
		  (*info->fprintf_func) (info->stream, "eamread r%d,%d",
					 dest, source_b);
		  break;
		}
	    }
	}
      else
	{
	  if (ins & FP_SELECT_MASK)
	    goto illegal_opcode;

	  /* READ instruction.  */
	  (*info->fprintf_func) (info->stream, "read.%s  r%d,%d(r%d)",
				 size_names[size], dest, source_b, source_a);
	}
      break;
    }

  return 0;

 illegal_opcode:
  return -1;

}

/* Print the visium instruction at address addr in debugged memory,
   on info->stream. Return length of the instruction, in bytes.  */

int
print_insn_visium (bfd_vma addr, disassemble_info *info)
{
  unsigned ins;
  unsigned p1, p2;
  int ans;
  int i;

  /* Stuff copied from m68k-dis.c.  */
  struct private priv;
  bfd_byte *buffer = priv.the_buffer;
  info->private_data = &priv;
  priv.max_fetched = priv.the_buffer;
  priv.insn_start = addr;
  if (setjmp (priv.bailout) != 0)
    {
      /* Error return.  */
      return -1;
    }

  /* We do return this info.  */
  info->insn_info_valid = 1;

  /* Assume non branch insn.  */
  info->insn_type = dis_nonbranch;

  /* Assume no delay.  */
  info->branch_delay_insns = 0;

  /* Assume no target known.  */
  info->target = 0;

  /* Get 32-bit instruction word.  */
  FETCH_DATA (info, buffer + 4);
  ins = (unsigned) buffer[0] << 24;
  ins |= buffer[1] << 16;
  ins |= buffer[2] << 8;
  ins |= buffer[3];

  ans = 0;

  p1 = buffer[0] ^ buffer[1] ^ buffer[2] ^ buffer[3];
  p2 = 0;
  for (i = 0; i < 8; i++)
    {
      p2 += p1 & 1;
      p1 >>= 1;
    }

  /* Decode the instruction.  */
  if (p2 & 1)
    ans = -1;
  else
    {
      switch ((ins >> 25) & 0x3)
	{
	case 0:
	  ans = disassem_class0 (info, ins);
	  break;
	case 1:
	  ans = disassem_class1 (info, ins);
	  break;
	case 2:
	  ans = disassem_class2 (info, ins);
	  break;
	case 3:
	  ans = disassem_class3 (info, ins);
	  break;
	}
    }

  if (ans != 0)
    (*info->fprintf_func) (info->stream, "err");

  /* Return number of bytes consumed (always 4 for the Visium).  */
  return 4;
}
