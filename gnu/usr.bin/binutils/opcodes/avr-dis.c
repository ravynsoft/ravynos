/* Disassemble AVR instructions.
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

   Contributed by Denis Chertykov <denisc@overta.ru>

   This file is part of libopcodes.

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
#include <assert.h>
#include "disassemble.h"
#include "opintl.h"
#include "libiberty.h"
#include <stdint.h>

struct avr_opcodes_s
{
  char *name;
  char *constraints;
  char *opcode;
  int insn_size;		/* In words.  */
  int isa;
  unsigned int bin_opcode;
};

#define AVR_INSN(NAME, CONSTR, OPCODE, SIZE, ISA, BIN) \
{#NAME, CONSTR, OPCODE, SIZE, ISA, BIN},

const struct avr_opcodes_s avr_opcodes[] =
{
  #include "opcode/avr.h"
  {NULL, NULL, NULL, 0, 0, 0}
};

static const char * comment_start = "0x";

static int
avr_operand (unsigned int        insn,
	     unsigned int        insn2,
	     unsigned int        pc,
	     int                 constraint,
             char *              opcode_str,
	     char *              buf,
	     char *              comment,
	     enum disassembler_style *  style,
	     int                 regs,
	     int *               sym,
	     bfd_vma *           sym_addr,
	     disassemble_info *  info)
{
  int ok = 1;
  *sym = 0;

  switch (constraint)
    {
      /* Any register operand.  */
    case 'r':
      if (regs)
	insn = (insn & 0xf) | ((insn & 0x0200) >> 5); /* Source register.  */
      else
	insn = (insn & 0x01f0) >> 4; /* Destination register.  */

      sprintf (buf, "r%d", insn);
      *style = dis_style_register;
      break;

    case 'd':
      if (regs)
	sprintf (buf, "r%d", 16 + (insn & 0xf));
      else
	sprintf (buf, "r%d", 16 + ((insn & 0xf0) >> 4));
      *style = dis_style_register;
      break;

    case 'w':
      sprintf (buf, "r%d", 24 + ((insn & 0x30) >> 3));
      *style = dis_style_register;
      break;

    case 'a':
      if (regs)
	sprintf (buf, "r%d", 16 + (insn & 7));
      else
	sprintf (buf, "r%d", 16 + ((insn >> 4) & 7));
      *style = dis_style_register;
      break;

    case 'v':
      if (regs)
	sprintf (buf, "r%d", (insn & 0xf) * 2);
      else
	sprintf (buf, "r%d", ((insn & 0xf0) >> 3));
      *style = dis_style_register;
      break;

    case 'e':
      {
	char *xyz;

	switch (insn & 0x100f)
	  {
	    case 0x0000: xyz = "Z";  break;
	    case 0x1001: xyz = "Z+"; break;
	    case 0x1002: xyz = "-Z"; break;
	    case 0x0008: xyz = "Y";  break;
	    case 0x1009: xyz = "Y+"; break;
	    case 0x100a: xyz = "-Y"; break;
	    case 0x100c: xyz = "X";  break;
	    case 0x100d: xyz = "X+"; break;
	    case 0x100e: xyz = "-X"; break;
	    default: xyz = "??"; ok = 0;
	  }
	strcpy (buf, xyz);

	if (AVR_UNDEF_P (insn))
	  sprintf (comment, _("undefined"));
      }
      *style = dis_style_register;
      break;

    case 'z':
      *buf++ = 'Z';

      /* Check for post-increment. */
      char *s;
      for (s = opcode_str; *s; ++s)
        {
          if (*s == '+')
            {
	      if (insn & (1 << (15 - (s - opcode_str))))
		*buf++ = '+';
              break;
            }
        }

      *buf = '\0';
      if (AVR_UNDEF_P (insn))
	sprintf (comment, _("undefined"));
      *style = dis_style_register;
      break;

    case 'b':
      {
	unsigned int x;

	x = (insn & 7);
	x |= (insn >> 7) & (3 << 3);
	x |= (insn >> 8) & (1 << 5);

	if (insn & 0x8)
	  *buf++ = 'Y';
	else
	  *buf++ = 'Z';
	sprintf (buf, "+%d", x);
	sprintf (comment, "0x%02x", x);
	*style = dis_style_register;
      }
      break;

    case 'h':
      *sym = 1;
      *sym_addr = ((((insn & 1) | ((insn & 0x1f0) >> 3)) << 16) | insn2) * 2;
      /* See PR binutils/2454.  Ideally we would like to display the hex
	 value of the address only once, but this would mean recoding
	 objdump_print_address() which would affect many targets.  */
      sprintf (buf, "%#lx", (unsigned long) *sym_addr);
      strcpy (comment, comment_start);
      info->insn_info_valid = 1;
      info->insn_type = dis_jsr;
      info->target = *sym_addr;
      *style = dis_style_address;
      break;

    case 'L':
      {
	int rel_addr = (((insn & 0xfff) ^ 0x800) - 0x800) * 2;
	sprintf (buf, ".%+-8d", rel_addr);
        *sym = 1;
        *sym_addr = pc + 2 + rel_addr;
	strcpy (comment, comment_start);
        info->insn_info_valid = 1;
        info->insn_type = dis_branch;
        info->target = *sym_addr;
	*style = dis_style_address_offset;
      }
      break;

    case 'l':
      {
	int rel_addr = ((((insn >> 3) & 0x7f) ^ 0x40) - 0x40) * 2;

	sprintf (buf, ".%+-8d", rel_addr);
        *sym = 1;
        *sym_addr = pc + 2 + rel_addr;
	strcpy (comment, comment_start);
        info->insn_info_valid = 1;
        info->insn_type = dis_condbranch;
        info->target = *sym_addr;
	*style = dis_style_address_offset;
      }
      break;

    case 'i':
      {
        unsigned int val = insn2 | 0x800000;
        *sym = 1;
        *sym_addr = val;
        sprintf (buf, "0x%04X", insn2);
        strcpy (comment, comment_start);
	*style = dis_style_immediate;
      }
      break;

    case 'j':
      {
        unsigned int val = ((insn & 0xf) | ((insn & 0x600) >> 5)
                                         | ((insn & 0x100) >> 2));
	if ((insn & 0x100) == 0)
	  val |= 0x80;
        *sym = 1;
        *sym_addr = val | 0x800000;
        sprintf (buf, "0x%02x", val);
        strcpy (comment, comment_start);
	*style = dis_style_immediate;
      }
      break;

    case 'M':
      sprintf (buf, "0x%02X", ((insn & 0xf00) >> 4) | (insn & 0xf));
      sprintf (comment, "%d", ((insn & 0xf00) >> 4) | (insn & 0xf));
      *style = dis_style_immediate;
      break;

    case 'n':
      sprintf (buf, "??");
      /* xgettext:c-format */
      opcodes_error_handler (_("internal disassembler error"));
      ok = 0;
      *style = dis_style_immediate;
      break;

    case 'K':
      {
	unsigned int x;

	x = (insn & 0xf) | ((insn >> 2) & 0x30);
	sprintf (buf, "0x%02x", x);
	sprintf (comment, "%d", x);
	*style = dis_style_immediate;
      }
      break;

    case 's':
      sprintf (buf, "%d", insn & 7);
      *style = dis_style_immediate;
      break;

    case 'S':
      sprintf (buf, "%d", (insn >> 4) & 7);
      *style = dis_style_immediate;
      break;

    case 'P':
      {
	unsigned int x;

	x = (insn & 0xf);
	x |= (insn >> 5) & 0x30;
	sprintf (buf, "0x%02x", x);
	sprintf (comment, "%d", x);
	*style = dis_style_address;
      }
      break;

    case 'p':
      {
	unsigned int x;

	x = (insn >> 3) & 0x1f;
	sprintf (buf, "0x%02x", x);
	sprintf (comment, "%d", x);
	*style = dis_style_address;
      }
      break;

    case 'E':
      sprintf (buf, "%d", (insn >> 4) & 15);
      *style = dis_style_immediate;
      break;

    case '?':
      *buf = '\0';
      break;

    default:
      sprintf (buf, "??");
      /* xgettext:c-format */
      opcodes_error_handler (_("unknown constraint `%c'"), constraint);
      ok = 0;
    }

    return ok;
}

/* Read the opcode from ADDR.  Return 0 in success and save opcode
   in *INSN, otherwise, return -1.  */

static int
avrdis_opcode (bfd_vma addr, disassemble_info *info, uint16_t *insn)
{
  bfd_byte buffer[2];
  int status;

  status = info->read_memory_func (addr, buffer, 2, info);

  if (status == 0)
    {
      *insn = bfd_getl16 (buffer);
      return 0;
    }

  info->memory_error_func (status, addr, info);
  return -1;
}


int
print_insn_avr (bfd_vma addr, disassemble_info *info)
{
  uint16_t insn, insn2;
  const struct avr_opcodes_s *opcode;
  static unsigned int *maskptr;
  void *stream = info->stream;
  fprintf_styled_ftype prin = info->fprintf_styled_func;
  static unsigned int *avr_bin_masks;
  static int initialized;
  int cmd_len = 2;
  int ok = 0;
  char op1[20], op2[20], comment1[40], comment2[40];
  enum disassembler_style style_op1, style_op2;
  int sym_op1 = 0, sym_op2 = 0;
  bfd_vma sym_addr1, sym_addr2;

  /* Clear instruction information field.  */
  info->insn_info_valid = 0;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->insn_type = dis_noninsn;
  info->target = 0;
  info->target2 = 0;

  if (!initialized)
    {
      unsigned int nopcodes;

      /* PR 4045: Try to avoid duplicating the 0x prefix that
	 objdump_print_addr() will put on addresses when there
	 is no symbol table available.  */
      if (info->symtab_size == 0)
	comment_start = " ";

      nopcodes = sizeof (avr_opcodes) / sizeof (struct avr_opcodes_s);

      avr_bin_masks = xmalloc (nopcodes * sizeof (unsigned int));

      for (opcode = avr_opcodes, maskptr = avr_bin_masks;
	   opcode->name;
	   opcode++, maskptr++)
	{
	  char * s;
	  unsigned int bin = 0;
	  unsigned int mask = 0;

	  for (s = opcode->opcode; *s; ++s)
	    {
	      bin <<= 1;
	      mask <<= 1;
	      bin |= (*s == '1');
	      mask |= (*s == '1' || *s == '0');
	    }
	  assert (s - opcode->opcode == 16);
	  assert (opcode->bin_opcode == bin);
	  *maskptr = mask;
	}

      initialized = 1;
    }

  if (avrdis_opcode (addr, info, &insn)  != 0)
    return -1;

  for (opcode = avr_opcodes, maskptr = avr_bin_masks;
       opcode->name;
       opcode++, maskptr++)
    {
      if ((opcode->isa == AVR_ISA_TINY) && (info->mach != bfd_mach_avrtiny))
        continue;
      if ((insn & *maskptr) == opcode->bin_opcode)
        break;
    }

  /* Special case: disassemble `ldd r,b+0' as `ld r,b', and
     `std b+0,r' as `st b,r' (next entry in the table).  */

  if (AVR_DISP0_P (insn))
    opcode++;

  op1[0] = 0;
  op2[0] = 0;
  comment1[0] = 0;
  comment2[0] = 0;
  style_op1 = dis_style_text;
  style_op2 = dis_style_text;

  if (opcode->name)
    {
      char *constraints = opcode->constraints;
      char *opcode_str = opcode->opcode;

      insn2 = 0;
      ok = 1;

      if (opcode->insn_size > 1)
	{
	  if (avrdis_opcode (addr + 2, info, &insn2) != 0)
	    return -1;
	  cmd_len = 4;
	}

      if (*constraints && *constraints != '?')
	{
	  int regs = REGISTER_P (*constraints);

	  ok = avr_operand (insn, insn2, addr, *constraints, opcode_str, op1,
			    comment1, &style_op1, 0, &sym_op1, &sym_addr1,
			    info);

	  if (ok && *(++constraints) == ',')
	    ok = avr_operand (insn, insn2, addr, *(++constraints), opcode_str,
			      op2, *comment1 ? comment2 : comment1,
			      &style_op2, regs, &sym_op2, &sym_addr2,
			      info);
	}
    }

  if (!ok)
    {
      /* Unknown opcode, or invalid combination of operands.  */
      sprintf (op1, "0x%04x", insn);
      op2[0] = 0;
      sprintf (comment1, "????");
      comment2[0] = 0;
    }

  (*prin) (stream, ok ? dis_style_mnemonic : dis_style_assembler_directive,
	   "%s", ok ? opcode->name : ".word");
  
  if (*op1)
    (*prin) (stream, style_op1, "\t%s", op1);

  if (*op2)
    {
      (*prin) (stream, dis_style_text, ", ");
      (*prin) (stream, style_op2, "%s", op2);
    }

  if (*comment1)
    (*prin) (stream, dis_style_comment_start, "\t; %s", comment1);

  if (sym_op1)
    info->print_address_func (sym_addr1, info);

  if (*comment2)
    (*prin) (stream, dis_style_comment_start, " %s", comment2);

  if (sym_op2)
    info->print_address_func (sym_addr2, info);

  return cmd_len;
}
