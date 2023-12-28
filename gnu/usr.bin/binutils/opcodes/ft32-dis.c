/* Disassemble ft32 instructions.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by FTDI (support@ftdichip.com)

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
#define STATIC_TABLE
#define DEFINE_TABLE

#include "opcode/ft32.h"
#include "disassemble.h"

extern const ft32_opc_info_t ft32_opc_info[128];

static fprintf_ftype fpr;
static void *stream;

static int
sign_extend (int bit, int value)
{
  int onebit = (1 << bit);
  return (value & (onebit - 1)) - (value & onebit);
}

static void
ft32_opcode1 (unsigned int iword,
	      struct disassemble_info *info)
{
  const ft32_opc_info_t *oo;

  for (oo = ft32_opc_info; oo->name; oo++)
    if ((iword & oo->mask) == oo->bits)
      break;

  if (oo->name)
    {
      int f = oo->fields;
      int imm;

      fpr (stream, "%s", oo->name);
      if (oo->dw)
	fpr (stream, ".%c ", "bsl"[(iword >> FT32_FLD_DW_BIT) & 3]);
      else
	fpr (stream, " ");

      while (f)
	{
	  int lobit = f & -f;
	  if (f & lobit)
	    {
	      switch (lobit)
		{
		case  FT32_FLD_CBCRCV:
		  /* imm is {CB, CV}  */
		  imm = ((iword >> FT32_FLD_CB_BIT) & ((1 << FT32_FLD_CB_SIZ) - 1)) << 4;
		  imm |= ((iword >> FT32_FLD_CV_BIT) & ((1 << FT32_FLD_CV_SIZ) - 1));
		  switch (imm)
		    {
		    case 0x00: fpr (stream, "nz");  break;
		    case 0x01: fpr (stream, "z");   break;
		    case 0x10: fpr (stream, "ae");  break;
		    case 0x11: fpr (stream, "b");   break;
		    case 0x20: fpr (stream, "no");  break;
		    case 0x21: fpr (stream, "o");   break;
		    case 0x30: fpr (stream, "ns");  break;
		    case 0x31: fpr (stream, "s");   break;
		    case 0x40: fpr (stream, "lt");  break;
		    case 0x41: fpr (stream, "gte"); break;
		    case 0x50: fpr (stream, "lte"); break;
		    case 0x51: fpr (stream, "gt");  break;
		    case 0x60: fpr (stream, "be");  break;
		    case 0x61: fpr (stream, "a");   break;
		    default:
		      fpr (stream, "%d,$r30,%d", (imm >> 4), (imm & 1));
		      break;
		    }
		  break;
		case  FT32_FLD_CB:
		  imm = (iword >> FT32_FLD_CB_BIT) & ((1 << FT32_FLD_CB_SIZ) - 1);
		  fpr (stream, "%d", imm);
		  break;
		case  FT32_FLD_R_D:
		  fpr (stream, "$r%d", (iword >> FT32_FLD_R_D_BIT) & 0x1f);
		  break;
		case  FT32_FLD_CR:
		  imm = (iword >> FT32_FLD_CR_BIT) & ((1 << FT32_FLD_CR_SIZ) - 1);
		  fpr (stream, "$r%d", 28 + imm);
		  break;
		case  FT32_FLD_CV:
		  imm = (iword >> FT32_FLD_CV_BIT) & ((1 << FT32_FLD_CV_SIZ) - 1);
		  fpr (stream, "%d", imm);
		  break;
		case  FT32_FLD_R_1:
		  fpr (stream, "$r%d", (iword >> FT32_FLD_R_1_BIT) & 0x1f);
		  break;
		case  FT32_FLD_RIMM:
		  imm = (iword >> FT32_FLD_RIMM_BIT) & ((1 << FT32_FLD_RIMM_SIZ) - 1);
		  if (imm & 0x400)
		    fpr (stream, "%d", sign_extend (9, imm));
		  else
		    fpr (stream, "$r%d", imm & 0x1f);
		  break;
		case  FT32_FLD_R_2:
		  fpr (stream, "$r%d", (iword >> FT32_FLD_R_2_BIT) & 0x1f);
		  break;
		case  FT32_FLD_K20:
		  imm = iword & ((1 << FT32_FLD_K20_SIZ) - 1);
		  fpr (stream, "%d", sign_extend (19, imm));
		  break;
		case  FT32_FLD_PA:
		  imm = (iword & ((1 << FT32_FLD_PA_SIZ) - 1)) << 2;
		  info->print_address_func ((bfd_vma) imm, info);
		  break;
		case  FT32_FLD_AA:
		  imm = iword & ((1 << FT32_FLD_AA_SIZ) - 1);
		  info->print_address_func ((1 << 23) | (bfd_vma) imm, info);
		  break;
		case  FT32_FLD_K16:
		  imm = iword & ((1 << FT32_FLD_K16_SIZ) - 1);
		  fpr (stream, "%d", imm);
		  break;
		case  FT32_FLD_K15:
		  imm = iword & ((1 << FT32_FLD_K15_SIZ) - 1);
		  fpr (stream, "%d", sign_extend (14, imm));
		  break;
		case  FT32_FLD_R_D_POST:
		  fpr (stream, "$r%d", (iword >> FT32_FLD_R_D_BIT) & 0x1f);
		  break;
		case  FT32_FLD_R_1_POST:
		  fpr (stream, "$r%d", (iword >> FT32_FLD_R_1_BIT) & 0x1f);
		  break;
		default:
		  break;
		}
	      f &= ~lobit;
	      if (f)
		fpr (stream, ",");
	    }
	}
    }
  else
    fpr (stream, "!");
}

static void
ft32_opcode (bfd_vma addr ATTRIBUTE_UNUSED,
	     unsigned int iword,
	     struct disassemble_info *info)
{
  unsigned int sc[2];
  if (ft32_decode_shortcode ((unsigned int) addr, iword, sc))
    {
      ft32_opcode1 (sc[0], info);
      fpr (stream, " ; ");
      ft32_opcode1 (sc[1], info);
    }
  else
    ft32_opcode1 (iword, info);
}

int
print_insn_ft32 (bfd_vma addr, struct disassemble_info *info)
{
  int status;
  stream = info->stream;
  bfd_byte buffer[4];
  unsigned int iword;

  fpr = info->fprintf_func;

  if ((status = info->read_memory_func (addr, buffer, 4, info)))
    goto fail;

  iword = bfd_getl32 (buffer);

  fpr (stream, "%08x ", iword);

  ft32_opcode (addr, iword, info);

  return 4;

 fail:
  info->memory_error_func (status, addr, info);
  return -1;
}
