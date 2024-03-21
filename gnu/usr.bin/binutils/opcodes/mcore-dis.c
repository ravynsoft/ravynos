/* Disassemble Motorola M*Core instructions.
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

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
#include "libiberty.h"
#define STATIC_TABLE
#define DEFINE_TABLE

#include "mcore-opc.h"
#include "disassemble.h"

/* Mask for each mcore_opclass: */
static const unsigned short imsk[] = {
    /* O0  */ 0xFFFF,
    /* OT  */ 0xFFFC,
    /* O1  */ 0xFFF0,
    /* OC  */ 0xFE00,
    /* O2  */ 0xFF00,
    /* X1  */ 0xFFF0,
    /* OI  */ 0xFE00,
    /* OB  */ 0xFE00,

    /* OMa */ 0xFFF0,
    /* SI  */ 0xFE00,
    /* I7  */ 0xF800,
    /* LS  */ 0xF000,
    /* BR  */ 0xF800,
    /* BL  */ 0xFF00,
    /* LR  */ 0xF000,
    /* LJ  */ 0xFF00,

    /* RM  */ 0xFFF0,
    /* RQ  */ 0xFFF0,
    /* JSR */ 0xFFF0,
    /* JMP */ 0xFFF0,
    /* OBRa*/ 0xFFF0,
    /* OBRb*/ 0xFF80,
    /* OBRc*/ 0xFF00,
    /* OBR2*/ 0xFE00,

    /* O1R1*/ 0xFFF0,
    /* OMb */ 0xFF80,
    /* OMc */ 0xFF00,
    /* SIa */ 0xFE00,

  /* MULSH */ 0xFF00,
  /* OPSR  */ 0xFFF8,   /* psrset/psrclr */

    /* JC  */ 0,		/* JC,JU,JL don't appear in object */
    /* JU  */ 0,
    /* JL  */ 0,
    /* RSI */ 0,
    /* DO21*/ 0,
    /* OB2 */ 0 		/* OB2 won't appear in object.  */
};

static const char *grname[] = {
 "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
 "r8",  "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

static const char X[] = "??";

static const char *crname[] = {
  "psr",  "vbr", "epsr", "fpsr", "epc",  "fpc",  "ss0",  "ss1",
  "ss2",  "ss3", "ss4",  "gcr",  "gsr",     X,      X,      X,
     X,      X,      X,      X,      X,     X,      X,      X,
     X,      X,      X,      X,      X,     X,      X,      X
};

static const unsigned isiz[] = { 2, 0, 1, 0 };

int
print_insn_mcore (bfd_vma memaddr,
		  struct disassemble_info *info)
{
  unsigned char ibytes[4];
  fprintf_ftype print_func = info->fprintf_func;
  void *stream = info->stream;
  unsigned int inst;
  unsigned int i;
  int status;

  info->bytes_per_chunk = 2;

  status = info->read_memory_func (memaddr, ibytes, 2, info);

  if (status != 0)
    {
      info->memory_error_func (status, memaddr, info);
      return -1;
    }

  if (info->endian == BFD_ENDIAN_BIG)
    inst = (ibytes[0] << 8) | ibytes[1];
  else if (info->endian == BFD_ENDIAN_LITTLE)
    inst = (ibytes[1] << 8) | ibytes[0];
  else
    abort ();

  /* Just a linear search of the table.  */
  for (i = 0; i < ARRAY_SIZE (mcore_table); i++)
    if (mcore_table[i].inst == (inst & imsk[mcore_table[i].opclass]))
      break;

  if (i == ARRAY_SIZE (mcore_table))
    (*print_func) (stream, ".short 0x%04x", inst);
  else
    {
      const char *name = grname[inst & 0x0F];

      (*print_func) (stream, "%s", mcore_table[i].name);

      switch (mcore_table[i].opclass)
	{
	case O0:
	  break;

	case OT:
	  (*print_func) (stream, "\t%d", inst & 0x3);
	  break;

	case O1:
	case JMP:
	case JSR:
	  (*print_func) (stream, "\t%s", name);
	  break;

	case OC:
	  (*print_func) (stream, "\t%s, %s", name, crname[(inst >> 4) & 0x1F]);
	  break;

	case O1R1:
	  (*print_func) (stream, "\t%s, r1", name);
	  break;

	case MULSH:
	case O2:
	  (*print_func) (stream, "\t%s, %s", name, grname[(inst >> 4) & 0xF]);
	  break;

	case X1:
	  (*print_func) (stream, "\tr1, %s", name);
	  break;

	case OI:
	  (*print_func) (stream, "\t%s, %d", name, ((inst >> 4) & 0x1F) + 1);
	  break;

	case RM:
	  (*print_func) (stream, "\t%s-r15, (r0)", name);
	  break;

	case RQ:
	  (*print_func) (stream, "\tr4-r7, (%s)", name);
	  break;

	case OB:
	case OBRa:
	case OBRb:
	case OBRc:
	case SI:
	case SIa:
	case OMa:
	case OMb:
	case OMc:
	  (*print_func) (stream, "\t%s, %d", name, (inst >> 4) & 0x1F);
	  break;

	case I7:
	  (*print_func) (stream, "\t%s, %d", name, (inst >> 4) & 0x7F);
	  break;

	case LS:
	  (*print_func) (stream, "\t%s, (%s, %d)", grname[(inst >> 8) & 0xF],
			 name, ((inst >> 4) & 0xF) << isiz[(inst >> 13) & 3]);
	  break;

	case BR:
	  {
	    uint32_t val = ((inst & 0x3FF) ^ 0x400) - 0x400;

	    val = memaddr + 2 + (val << 1);
	    (*print_func) (stream, "\t0x%x", val);

	    if (strcmp (mcore_table[i].name, "bsr") == 0)
	      {
		/* For bsr, we'll try to get a symbol for the target.  */
		if (info->print_address_func && val != 0)
		  {
		    (*print_func) (stream, "\t// ");
		    info->print_address_func (val, info);
		  }
	      }
	  }
	  break;

	case BL:
	  {
	    uint32_t val = memaddr + 2 + ((inst | ~0xF) << 1);

	    (*print_func) (stream, "\t%s, 0x%x",
			   grname[(inst >> 4) & 0xF], val);
	  }
	  break;

	case LR:
	  {
	    uint32_t val;

	    val = (memaddr + 2 + ((inst & 0xFF) << 2)) & ~3;

	    /* We are not reading an instruction, so allow
	       reads to extend beyond the next symbol.  */
	    info->stop_vma = 0;
	    status = info->read_memory_func (val, ibytes, 4, info);
	    if (status != 0)
	      {
		info->memory_error_func (status, memaddr, info);
		break;
	      }

	    if (info->endian == BFD_ENDIAN_LITTLE)
	      val = (((unsigned) ibytes[3] << 24) | (ibytes[2] << 16)
		     | (ibytes[1] << 8) | (ibytes[0]));
	    else
	      val = (((unsigned) ibytes[0] << 24) | (ibytes[1] << 16)
		     | (ibytes[2] << 8) | (ibytes[3]));

	    /* Removed [] around literal value to match ABI syntax 12/95.  */
	    (*print_func) (stream, "\t%s, 0x%X", grname[(inst >> 8) & 0xF], val);

	    if (val == 0)
	      (*print_func) (stream, "\t// from address pool at 0x%x",
			     (uint32_t) (memaddr + 2
					 + ((inst & 0xFF) << 2)) & ~3);
	  }
	  break;

	case LJ:
	  {
	    uint32_t val;

	    val = (memaddr + 2 + ((inst & 0xFF) << 2)) & ~3;

	    /* We are not reading an instruction, so allow
	       reads to extend beyond the next symbol.  */
	    info->stop_vma = 0;
	    status = info->read_memory_func (val, ibytes, 4, info);
	    if (status != 0)
	      {
		info->memory_error_func (status, memaddr, info);
		break;
	      }

	    if (info->endian == BFD_ENDIAN_LITTLE)
	      val = (((unsigned) ibytes[3] << 24) | (ibytes[2] << 16)
		     | (ibytes[1] << 8) | (ibytes[0]));
	    else
	      val = (((unsigned) ibytes[0] << 24) | (ibytes[1] << 16)
		     | (ibytes[2] << 8) | (ibytes[3]));

	    /* Removed [] around literal value to match ABI syntax 12/95.  */
	    (*print_func) (stream, "\t0x%X", val);
	    /* For jmpi/jsri, we'll try to get a symbol for the target.  */
	    if (info->print_address_func && val != 0)
	      {
		(*print_func) (stream, "\t// ");
		info->print_address_func (val, info);
	      }
	    else
	      {
		(*print_func) (stream, "\t// from address pool at 0x%x",
			       (uint32_t) (memaddr + 2
					   + ((inst & 0xFF) << 2)) & ~3);
	      }
	  }
	  break;

	case OPSR:
	  {
	    static char *fields[] = {
	      "af", "ie",    "fe",    "fe,ie",
	      "ee", "ee,ie", "ee,fe", "ee,fe,ie"
	    };

	    (*print_func) (stream, "\t%s", fields[inst & 0x7]);
	  }
	  break;

	default:
	  /* If the disassembler lags the instruction set.  */
	  (*print_func) (stream, "\tundecoded operands, inst is 0x%04x", inst);
	  break;
	}
    }

  /* Say how many bytes we consumed.  */
  return 2;
}
