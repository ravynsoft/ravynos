/* Disassemble moxie instructions.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

#include "opcode/moxie.h"
#include "disassemble.h"

static fprintf_ftype fpr;
static void *stream;

/* Macros to extract operands from the instruction word.  */
#define OP_A(i) ((i >> 4) & 0xf)
#define OP_B(i) (i & 0xf)
#define INST2OFFSET(o) (((((o) & 0x3ff) ^ 0x200) - 0x200) * 2)

static const char * reg_names[16] =
  { "$fp", "$sp", "$r0", "$r1", "$r2", "$r3", "$r4", "$r5",
    "$r6", "$r7", "$r8", "$r9", "$r10", "$r11", "$r12", "$r13" };

int
print_insn_moxie (bfd_vma addr, struct disassemble_info * info)
{
  int length = 2;
  int status;
  stream = info->stream;
  const moxie_opc_info_t * opcode;
  bfd_byte buffer[4];
  unsigned short iword;
  fpr = info->fprintf_func;

  if ((status = info->read_memory_func (addr, buffer, 2, info)))
    goto fail;

  if (info->endian == BFD_ENDIAN_BIG)
    iword = bfd_getb16 (buffer);
  else
    iword = bfd_getl16 (buffer);

  /* Form 1 instructions have the high bit set to 0.  */
  if ((iword & (1<<15)) == 0)
    {
      /* Extract the Form 1 opcode.  */
      opcode = &moxie_form1_opc_info[iword >> 8];
      switch (opcode->itype)
	{
	case MOXIE_F1_NARG:
	  fpr (stream, "%s", opcode->name);
	  break;
	case MOXIE_F1_A:
	  fpr (stream, "%s\t%s", opcode->name,
	       reg_names[OP_A(iword)]);
	  break;
	case MOXIE_F1_AB:
	  fpr (stream, "%s\t%s, %s", opcode->name,
	       reg_names[OP_A(iword)],
	       reg_names[OP_B(iword)]);
	  break;
	case MOXIE_F1_A4:
	  {
	    unsigned imm;
	    if ((status = info->read_memory_func (addr + 2, buffer, 4, info)))
	      goto fail;
	    if (info->endian == BFD_ENDIAN_BIG)
	      imm = bfd_getb32 (buffer);
	    else
	      imm = bfd_getl32 (buffer);
	    fpr (stream, "%s\t%s, 0x%x", opcode->name,
		 reg_names[OP_A(iword)], imm);
	    length = 6;
	  }
	  break;
	case MOXIE_F1_4:
	  {
	    unsigned imm;
	    if ((status = info->read_memory_func (addr + 2, buffer, 4, info)))
	      goto fail;
	    if (info->endian == BFD_ENDIAN_BIG)
	      imm = bfd_getb32 (buffer);
	    else
	      imm = bfd_getl32 (buffer);
	    fpr (stream, "%s\t0x%x", opcode->name, imm);
	    length = 6;
	  }
	  break;
	case MOXIE_F1_M:
	  {
	    unsigned imm;
	    if ((status = info->read_memory_func (addr + 2, buffer, 4, info)))
	      goto fail;
	    if (info->endian == BFD_ENDIAN_BIG)
	      imm = bfd_getb32 (buffer);
	    else
	      imm = bfd_getl32 (buffer);
	    fpr (stream, "%s\t", opcode->name);
	    info->print_address_func ((bfd_vma) imm, info);
	    length = 6;
	  }
	  break;
	case MOXIE_F1_AiB:
	  fpr (stream, "%s\t(%s), %s", opcode->name,
	       reg_names[OP_A(iword)], reg_names[OP_B(iword)]);
	  break;
	case MOXIE_F1_ABi:
	  fpr (stream, "%s\t%s, (%s)", opcode->name,
	       reg_names[OP_A(iword)], reg_names[OP_B(iword)]);
	  break;
	case MOXIE_F1_4A:
	  {
	    unsigned imm;
	    if ((status = info->read_memory_func (addr + 2, buffer, 4, info)))
	      goto fail;
	    if (info->endian == BFD_ENDIAN_BIG)
	      imm = bfd_getb32 (buffer);
	    else
	      imm = bfd_getl32 (buffer);
	    fpr (stream, "%s\t0x%x, %s",
		 opcode->name, imm, reg_names[OP_A(iword)]);
	    length = 6;
	  }
	  break;
	case MOXIE_F1_AiB2:
	  {
	    unsigned imm;
	    if ((status = info->read_memory_func (addr+2, buffer, 2, info)))
	      goto fail;
	    if (info->endian == BFD_ENDIAN_BIG)
	      imm = bfd_getb16 (buffer);
	    else
	      imm = bfd_getl16 (buffer);
	    fpr (stream, "%s\t0x%x(%s), %s", opcode->name,
		 imm,
		 reg_names[OP_A(iword)],
		 reg_names[OP_B(iword)]);
	    length = 4;
	  }
	  break;
	case MOXIE_F1_ABi2:
	  {
	    unsigned imm;
	    if ((status = info->read_memory_func (addr+2, buffer, 2, info)))
	      goto fail;
	    if (info->endian == BFD_ENDIAN_BIG)
	      imm = bfd_getb16 (buffer);
	    else
	      imm = bfd_getl16 (buffer);
	    fpr (stream, "%s\t%s, 0x%x(%s)",
		 opcode->name,
		 reg_names[OP_A(iword)],
		 imm,
		 reg_names[OP_B(iword)]);
	    length = 4;
	  }
	  break;
        case MOXIE_BAD:
	  fpr (stream, "bad");
	  break;
	default:
	  abort();
	}
    }
  else if ((iword & (1<<14)) == 0)
    {
      /* Extract the Form 2 opcode.  */
      opcode = &moxie_form2_opc_info[(iword >> 12) & 3];
      switch (opcode->itype)
	{
	case MOXIE_F2_A8V:
	  fpr (stream, "%s\t%s, 0x%x",
	       opcode->name,
	       reg_names[(iword >> 8) & 0xf],
	       iword & ((1 << 8) - 1));
	  break;
	case MOXIE_F2_NARG:
	  fpr (stream, "%s", opcode->name);
	  break;
        case MOXIE_BAD:
	  fpr (stream, "bad");
	  break;
	default:
	  abort();
	}
    }
  else
    {
      /* Extract the Form 3 opcode.  */
      opcode = &moxie_form3_opc_info[(iword >> 10) & 15];
      switch (opcode->itype)
	{
	case MOXIE_F3_PCREL:
	  fpr (stream, "%s\t", opcode->name);
	  info->print_address_func (addr + INST2OFFSET (iword) + 2, info);
	  break;
        case MOXIE_BAD:
	  fpr (stream, "bad");
	  break;
	default:
	  abort();
	}
    }

  return length;

 fail:
  info->memory_error_func (status, addr, info);
  return -1;
}
