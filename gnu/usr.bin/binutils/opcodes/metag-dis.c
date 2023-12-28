/* Disassemble Imagination Technologies Meta instructions.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by Imagination Technologies Ltd.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

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
#include "opintl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opcode/metag.h"

/* Column widths for printing.  */
#define PREFIX_WIDTH    "10"
#define INSN_NAME_WIDTH "10"

#define OPERAND_WIDTH   92
#define ADDR_WIDTH      20
#define REG_WIDTH       64
#define DSP_PREFIX_WIDTH 17

/* Value to print if we fail to parse a register name.  */
const char unknown_reg[] = "?";

/* Return the size of a GET or SET instruction.  */
unsigned int
metag_get_set_size_bytes (unsigned int opcode)
{
  switch (((opcode) >> 24) & 0x5)
    {
    case 0x5:
      return 8;
    case 0x4:
      return 4;
    case 0x1:
      return 2;
    case 0x0:
      return 1;
    }
  return 1;
}

/* Return the size of an extended GET or SET instruction.  */
unsigned int
metag_get_set_ext_size_bytes (unsigned int opcode)
{
  switch (((opcode) >> 1) & 0x3)
    {
    case 0x3:
      return 8;
    case 0x2:
      return 4;
    case 0x1:
      return 2;
    case 0x0:
      return 1;
    }
  return 1;
}

/* Return the size of a conditional SET instruction.  */
unsigned int
metag_cond_set_size_bytes (unsigned int opcode)
{
  switch (opcode & 0x201)
    {
    case 0x201:
      return 8;
    case 0x200:
      return 4;
    case 0x001:
      return 2;
    case 0x000:
      return 1;
    }
  return 1;
}

/* Return a value sign-extended.  */
static int
sign_extend (int n, unsigned int bits)
{
  int mask = 1 << (bits - 1);
  return -(n & mask) | n;
}

/* Return the short interpretation of UNIT.  */
static unsigned int
short_unit (unsigned int unit)
{
  if (unit == UNIT_CT)
    return UNIT_A1;
  else
    return unit;
}

/* Return the register corresponding to UNIT and NUMBER or NULL.  */
static const metag_reg *
lookup_reg (unsigned int unit, unsigned int number)
{
  size_t i;

  for (i = 0; i < sizeof(metag_regtab)/sizeof(metag_regtab[0]); i++)
    {
      const metag_reg *reg = &metag_regtab[i];

      if (reg->unit == unit && reg->no == number)
	return reg;
    }
  return NULL;
}


/* Return the register name corresponding to UNIT and NUMBER or NULL.  */
static const char *
lookup_reg_name (unsigned int unit, unsigned int number)
{
  const metag_reg *reg;

  reg = lookup_reg (unit, number);

  if (reg)
    return reg->name;
  else
    return unknown_reg;
}

/* Return the unit that is the pair of UNIT.  */
static unsigned int
get_pair_unit (unsigned int unit)
{
  switch (unit)
    {
    case UNIT_D0:
      return UNIT_D1;
    case UNIT_D1:
      return UNIT_D0;
    case UNIT_A0:
      return UNIT_A1;
    case UNIT_A1:
      return UNIT_A0;
    default:
      return unit;
    }
}

/* Return the name of the pair register for UNIT and NUMBER or NULL.  */
static const char *
lookup_pair_reg_name (unsigned int unit, unsigned int number)
{
  if (unit == UNIT_FX)
    return lookup_reg_name (unit, number + 1);
  else
    return lookup_reg_name (get_pair_unit (unit), number);
}

/* Return the name of the accumulator register for PART.  */
static const char *
lookup_acf_name (unsigned int part)
{
  size_t i;

  for (i = 0; i < sizeof(metag_acftab)/sizeof(metag_acftab[0]); i++)
    {
      const metag_acf *acf = &metag_acftab[i];

      if (acf->part == part)
	return acf->name;
    }
  return "ACF.?";
}

/* Return the register name for the O2R register for UNIT and NUMBER.  */
static const char *
lookup_o2r (enum metag_unit unit, unsigned int number)
{
  unsigned int o2r_unit;
  enum metag_unit actual_unit = UNIT_A0;
  const metag_reg *reg;

  o2r_unit = (number & ~O2R_REG_MASK) >> 3;
  number = number & O2R_REG_MASK;

  if (unit == UNIT_A0)
    {
      switch (o2r_unit)
	{
	case 0:
	  actual_unit = UNIT_A1;
	  break;
	case 1:
	  actual_unit = UNIT_D0;
	  break;
	case 2:
	  actual_unit = UNIT_RD;
	  break;
	case 3:
	  actual_unit = UNIT_D1;
	  break;
	}
    }
  else if (unit == UNIT_A1)
    {
      switch (o2r_unit)
	{
	case 0:
	  actual_unit = UNIT_D1;
	  break;
	case 1:
	  actual_unit = UNIT_D0;
	  break;
	case 2:
	  actual_unit = UNIT_RD;
	  break;
	case 3:
	  actual_unit = UNIT_A0;
	  break;
	}
    }
  else if (unit == UNIT_D0)
    {
      switch (o2r_unit)
	{
	case 0:
	  actual_unit = UNIT_A1;
	  break;
	case 1:
	  actual_unit = UNIT_D1;
	  break;
	case 2:
	  actual_unit = UNIT_RD;
	  break;
	case 3:
	  actual_unit = UNIT_A0;
	  break;
	}
    }
  else if (unit == UNIT_D1)
    {
      switch (o2r_unit)
	{
	case 0:
	  actual_unit = UNIT_A1;
	  break;
	case 1:
	  actual_unit = UNIT_D0;
	  break;
	case 2:
	  actual_unit = UNIT_RD;
	  break;
	case 3:
	  actual_unit = UNIT_A0;
	  break;
	}
    }

  reg = lookup_reg (actual_unit, number);

  if (reg)
    return reg->name;
  else
    return unknown_reg;
}

/* Return the string for split condition code CODE. */
static const char *
lookup_scc_flags (unsigned int code)
{
  size_t i;

  for (i = 0; i < sizeof (metag_dsp_scondtab) / sizeof (metag_dsp_scondtab[0]); i++)
    {
      if (metag_dsp_scondtab[i].code == code)
	{
	  return metag_dsp_scondtab[i].name;
	}
    }
  return NULL;
}

/* Return the string for FPU split condition code CODE. */
static const char *
lookup_fpu_scc_flags (unsigned int code)
{
  size_t i;

  for (i = 0; i < sizeof (metag_fpu_scondtab) / sizeof (metag_fpu_scondtab[0]); i++)
    {
      if (metag_fpu_scondtab[i].code == code)
	{
	  return metag_fpu_scondtab[i].name;
	}
    }
  return NULL;
}

/* Print an instruction with PREFIX, NAME and OPERANDS.  */
static void
print_insn (disassemble_info *outf, const char *prefix, const char *name,
	    const char *operands)
{
  outf->fprintf_func (outf->stream, "%-" PREFIX_WIDTH "s%-" INSN_NAME_WIDTH "s%s", prefix, name, operands);
}

/* Print an instruction with no operands.  */
static void
print_none (unsigned int insn_word ATTRIBUTE_UNUSED,
	    bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  outf->fprintf_func (outf->stream, "%-" PREFIX_WIDTH "s%s", "",
		      template->name);
}

/* Print a unit to unit MOV instruction.  */
static void
print_mov_u2u (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	       const insn_template *template,
	       disassemble_info *outf)
{
  unsigned int dest_unit, dest_no, src_unit, src_no;
  unsigned int is_kick = (insn_word & 0x1) && !((insn_word >> 9) & 0x1);
  unsigned int major = MAJOR_OPCODE (insn_word);
  unsigned int minor = MINOR_OPCODE (insn_word);
  char buf[OPERAND_WIDTH];
  const char *dest_reg;
  const char *src_reg;

  dest_unit = (insn_word >> 5) & UNIT_MASK;
  dest_no = (insn_word >> 14) & REG_MASK;

  dest_reg = lookup_reg_name (dest_unit, dest_no);

  if (is_kick)
    src_unit = UNIT_TR;
  else
    src_unit = (insn_word >> 10) & UNIT_MASK;

  /* This is really an RTI/RTH. No, really.  */
  if (major == OPC_MISC &&
      minor == 0x3 &&
      src_unit == 0xf)
    {
      if (insn_word & 0x800000)
	outf->fprintf_func (outf->stream, "%-" PREFIX_WIDTH "s%s", "",
			    "RTI");
      else
	outf->fprintf_func (outf->stream, "%-" PREFIX_WIDTH "s%s", "",
			    "RTH");

      return;
    }

  src_no = (insn_word >> 19) & REG_MASK;

  src_reg = lookup_reg_name (src_unit, src_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  if (dest_unit == UNIT_FX || src_unit == UNIT_FX)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Print a MOV to port instruction.  */
static void
print_mov_port (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		const insn_template *template,
		disassemble_info *outf)
{
  unsigned int dest_unit, dest1_no, dest2_no, src_unit, src_no;
  unsigned int is_movl = MINOR_OPCODE (insn_word) == MOVL_MINOR;
  char buf[OPERAND_WIDTH];
  const char *dest_reg;
  const char *pair_reg;
  const char *src_reg;

  if (is_movl)
    dest_unit = short_unit ((insn_word >> 5) & SHORT_UNIT_MASK);
  else
    dest_unit = (insn_word >> 5) & UNIT_MASK;

  dest1_no = (insn_word >> 14) & REG_MASK;
  dest2_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (dest_unit, dest1_no);
  pair_reg = lookup_pair_reg_name (dest_unit, dest2_no);

  src_unit = UNIT_RD;
  src_no = 0;

  src_reg = lookup_reg_name (src_unit, src_no);

  if (is_movl)
    snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, pair_reg, src_reg);
  else
    snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  if (dest_unit == UNIT_FX)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Return the number of bits set in rmask.  */
static unsigned int hweight (unsigned int rmask)
{
  unsigned int count;

  for (count = 0; rmask; count++)
    {
      rmask &= rmask - 1;
    }

  return count;
}

/* Print a MOVL to TTREC instruction.  */
static void
print_movl_ttrec (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		  const insn_template *template,
		  disassemble_info *outf)
{
  unsigned int dest_unit, dest_no, src1_no, src2_no, src_unit;
  char buf[OPERAND_WIDTH];
  const char *dest_reg;
  const char *src_reg;
  const char *pair_reg;

  dest_unit = UNIT_TT;
  dest_no = 3;

  dest_reg = lookup_reg_name (dest_unit, dest_no);

  src1_no = (insn_word >> 19) & REG_MASK;
  src2_no = (insn_word >> 14) & REG_MASK;

  src_unit = short_unit ((insn_word >> 7) & SHORT_UNIT_MASK);

  src_reg = lookup_reg_name (src_unit, src1_no);
  pair_reg = lookup_pair_reg_name (src_unit, src2_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, src_reg, pair_reg);

  print_insn (outf, "", template->name, buf);
}

/* Format a GET or SET address mode string from INSN_WORD into BUF.  */
static void
get_set_addr_str (char *buf, unsigned int buf_size, unsigned int size,
		  unsigned int insn_word)
{
  const char *base_reg;
  unsigned int base_unit, base_no;
  unsigned int imm = (insn_word >> 25) & 1;
  unsigned int ua = (insn_word >> 7) & 1;
  unsigned int pp = insn_word & 1;

  base_unit = short_unit ((insn_word >> 5) & SHORT_UNIT_MASK);
  base_no = (insn_word >> 14) & REG_MASK;

  base_reg = lookup_reg_name (base_unit, base_no);

  if (imm)
    {
      int offset = (insn_word >> 8) & GET_SET_IMM_MASK;

      offset = sign_extend (offset, GET_SET_IMM_BITS);

      if (offset == 0)
	{
	  snprintf (buf, buf_size, "[%s]", base_reg);
	  return;
	}

      if (offset == 1 && ua)
	{
	  if (pp)
	    snprintf (buf, buf_size, "[%s++]", base_reg);
	  else
	    snprintf (buf, buf_size, "[++%s]", base_reg);

	  return;
	}
      else if (offset == -1 && ua)
	{
	  if (pp)
	    snprintf (buf, buf_size, "[%s--]", base_reg);
	  else
	    snprintf (buf, buf_size, "[--%s]", base_reg);

	  return;
	}

      offset = offset * size;

      if (ua)
	{
	  if (pp)
	    snprintf (buf, buf_size, "[%s+#%d++]", base_reg, offset);
	  else
	    snprintf (buf, buf_size, "[%s++#%d]", base_reg, offset);
	}
      else
	snprintf (buf, buf_size, "[%s+#%d]", base_reg, offset);
    }
  else
    {
      const char *offset_reg;
      unsigned int offset_no;

      offset_no = (insn_word >> 9) & REG_MASK;

      offset_reg = lookup_reg_name (base_unit, offset_no);

      if (ua)
	{
	  if (pp)
	    snprintf (buf, buf_size, "[%s+%s++]", base_reg, offset_reg);
	  else
	    snprintf (buf, buf_size, "[%s++%s]", base_reg, offset_reg);
	}
      else
	snprintf (buf, buf_size, "[%s+%s]", base_reg, offset_reg);
    }
}

/* Format an extended GET or SET address mode string from INSN_WORD into BUF. */
static void
get_set_ext_addr_str (char *buf, unsigned int buf_size, unsigned int size,
		      unsigned int insn_word)
{
  const char *base_reg;
  unsigned int base_unit, base_no;
  int offset;

  base_unit = short_unit ((insn_word >> 5) & SHORT_UNIT_MASK);
  base_no = insn_word & EXT_BASE_REG_MASK;

  base_reg = lookup_reg_name (base_unit, base_no);

  offset = (insn_word >> 7) & GET_SET_EXT_IMM_MASK;

  offset = sign_extend (offset, GET_SET_EXT_IMM_BITS);

  offset = offset * size;

  if (offset == 0)
    {
      snprintf (buf, buf_size, "[%s]", base_reg);
    }
  else
    {
      snprintf (buf, buf_size, "[%s+#%d]", base_reg, offset);
    }
}

/* Format an MGET or MSET address mode string from INSN_WORD into BUF.  */
static void
mget_mset_addr_str (char *buf, unsigned int buf_size,
		    unsigned int insn_word)
{
  const char *base_reg;
  unsigned int base_unit, base_no;

  base_unit = short_unit ((insn_word >> 5) & SHORT_UNIT_MASK);
  base_no = (insn_word >> 14) & REG_MASK;

  base_reg = lookup_reg_name (base_unit, base_no);

  snprintf (buf, buf_size, "[%s++]", base_reg);
}

/* Format a conditional SET address mode string from INSN_WORD into BUF.  */
static void
cond_set_addr_str (char *buf, unsigned int buf_size,
		   unsigned int insn_word)
{
  const char *base_reg;
  unsigned int base_unit, base_no;

  base_unit = short_unit ((insn_word >> 5) & SHORT_UNIT_MASK);
  base_no = (insn_word >> 14) & REG_MASK;

  base_reg = lookup_reg_name (base_unit, base_no);

  snprintf (buf, buf_size, "[%s]", base_reg);
}

/* Format a cache instruction address mode string from INSN_WORD into BUF.  */
static void
cache_addr_str (char *buf, unsigned int buf_size, unsigned int insn_word,
		int width)
{
  const char *base_reg;
  unsigned int base_unit, base_no;
  int offset;

  base_unit = short_unit ((insn_word >> 5) & SHORT_UNIT_MASK);
  base_no = (insn_word >> 14) & REG_MASK;

  base_reg = lookup_reg_name (base_unit, base_no);

  offset = (insn_word >> 8) & GET_SET_IMM_MASK;

  offset = sign_extend (offset, GET_SET_IMM_BITS);

  offset = offset * width;

  if (offset == 0)
    {
      snprintf (buf, buf_size, "[%s]", base_reg);
    }
  else
    {
      snprintf (buf, buf_size, "[%s+#%d]", base_reg, offset);
    }
}

/* Format a list of registers starting at REG_UNIT and REG_NO and conforming
   to RMASK into BUF.  */
static void
lookup_reg_list (char *reg_buf, size_t buf_len, unsigned int reg_unit,
		 unsigned int reg_no, unsigned int rmask,
		 bool is_fpu_64bit)
{
  const char *regs[MGET_MSET_MAX_REGS];
  size_t used_regs = 1, i, remaining;

  regs[0] = lookup_reg_name (reg_unit, reg_no);

  for (i = 1; i < MGET_MSET_MAX_REGS; i++)
    {
      if (rmask & 1)
	{
	  if (is_fpu_64bit)
	    regs[used_regs] = lookup_reg_name (reg_unit, reg_no + (i * 2));
	  else
	    regs[used_regs] = lookup_reg_name (reg_unit, reg_no + i);
	  used_regs++;
	}
      rmask = rmask >> 1;
    }

  remaining = buf_len;

  for (i = 0; i < used_regs; i++)
    {
      size_t len;
      if (i == 0)
	len = snprintf(reg_buf, remaining, "%s", regs[i]);
      else
	len = snprintf(reg_buf, remaining, ",%s", regs[i]);

      reg_buf += len;
      remaining -= len;
    }
}

/* Print a GET instruction.  */
static void
print_get (char *buf, char *addr_buf, unsigned int size,
	   const char *dest_reg, const char *pair_reg, unsigned int reg_unit,
	   const insn_template *template,
	   disassemble_info *outf)
{
  if (size == 8)
    {
      snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, pair_reg,
		addr_buf);
    }
  else
    {
      snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, addr_buf);
    }

  if (reg_unit == UNIT_FX)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Print a SET instruction.  */
static void
print_set (char *buf, char *addr_buf, unsigned int size,
	   const char *src_reg, const char *pair_reg, unsigned int reg_unit,
	   const insn_template *template,
	   disassemble_info *outf)
{
  if (size == 8)
    {
      snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", addr_buf, src_reg, pair_reg);
    }
  else
    {
      snprintf (buf, OPERAND_WIDTH, "%s,%s", addr_buf, src_reg);
    }

  if (reg_unit == UNIT_FX)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Print a GET or SET instruction.  */
static void
print_get_set (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	       const insn_template *template,
	       disassemble_info *outf)
{
  bool is_get = MAJOR_OPCODE (template->meta_opcode) == OPC_GET;
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  unsigned int reg_unit, reg_no;
  unsigned int size = metag_get_set_size_bytes (insn_word);
  const char *reg_name;
  const char *pair_reg;

  reg_unit = (insn_word >> 1) & UNIT_MASK;
  reg_no = (insn_word >> 19) & REG_MASK;

  /* SETs should always print RD. */
  if (!is_get && reg_unit == UNIT_RD)
    reg_no = 0;

  reg_name = lookup_reg_name (reg_unit, reg_no);

  pair_reg = lookup_pair_reg_name (reg_unit, reg_no);

  get_set_addr_str (addr_buf, ADDR_WIDTH, size, insn_word);

  if (is_get)
    {
      /* RD regs are 64 bits wide so don't use the pair syntax.  */
      if (reg_unit == UNIT_RD)
	print_get (buf, addr_buf, 4, reg_name, pair_reg, reg_unit,
		   template, outf);
      else
	print_get (buf, addr_buf, size, reg_name, pair_reg, reg_unit,
		   template, outf);
    }
  else
    {
      /* RD regs are 64 bits wide so don't use the pair syntax.  */
      if (reg_unit == UNIT_RD)
	print_set (buf, addr_buf, 4, reg_name, pair_reg, reg_unit,
		   template, outf);
      else
	print_set (buf, addr_buf, size, reg_name, pair_reg, reg_unit,
		   template, outf);
    }
}

/* Print an extended GET or SET instruction.  */
static void
print_get_set_ext (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		   const insn_template *template,
		   disassemble_info *outf)
{
  bool is_get = MINOR_OPCODE (template->meta_opcode) == GET_EXT_MINOR;
  bool is_mov = MINOR_OPCODE (template->meta_opcode) == MOV_EXT_MINOR;
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  unsigned int reg_unit, reg_no;
  unsigned int size = metag_get_set_ext_size_bytes (insn_word);
  const char *reg_name;
  const char *pair_reg;

  if (is_mov)
    reg_unit = UNIT_RD;
  else
    reg_unit = short_unit ((insn_word >> 3) & SHORT_UNIT_MASK);

  reg_no = (insn_word >> 19) & REG_MASK;

  reg_name = lookup_reg_name (reg_unit, reg_no);

  pair_reg = lookup_pair_reg_name (reg_unit, reg_no);

  get_set_ext_addr_str (addr_buf, ADDR_WIDTH, size, insn_word);

  if (is_get)
    print_get (buf, addr_buf, size, reg_name, pair_reg, reg_unit,
	       template, outf);
  else if (is_mov)
    print_get (buf, addr_buf, 4, reg_name, pair_reg, reg_unit,
	       template, outf);
  else
    print_set (buf, addr_buf, size, reg_name, pair_reg, reg_unit,
	       template, outf);
}

/* Print an MGET or MSET instruction.  */
static void
print_mget_mset (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		 const insn_template *template,
		 disassemble_info *outf)
{
  bool is_get = MAJOR_OPCODE (template->meta_opcode) == OPC_GET;
  bool is_fpu = (MINOR_OPCODE (template->meta_opcode) & 0x6) == 0x6;
  bool is_64bit = (MINOR_OPCODE (template->meta_opcode) & 0x1) == 0x1;
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  char reg_buf[REG_WIDTH];
  unsigned int reg_unit, reg_no, rmask;

  if (is_fpu)
    reg_unit = UNIT_FX;
  else
    reg_unit = short_unit ((insn_word >> 3) & SHORT_UNIT_MASK);

  reg_no = (insn_word >> 19) & REG_MASK;
  rmask = (insn_word >> 7) & RMASK_MASK;

  lookup_reg_list (reg_buf, REG_WIDTH, reg_unit, reg_no, rmask,
		   is_fpu && is_64bit);

  mget_mset_addr_str (addr_buf, ADDR_WIDTH, insn_word);

  if (is_get)
    snprintf (buf, OPERAND_WIDTH, "%s,%s", reg_buf, addr_buf);
  else
    snprintf (buf, OPERAND_WIDTH, "%s,%s", addr_buf, reg_buf);

  if (is_fpu)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Print a conditional SET instruction.  */
static void
print_cond_set (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		const insn_template *template,
		disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  unsigned int src_unit, src_no;
  unsigned int size = metag_cond_set_size_bytes (insn_word);
  const char *src_reg;
  const char *pair_reg;

  src_unit = (insn_word >> 10) & UNIT_MASK;
  src_no = (insn_word >> 19) & REG_MASK;

  if (src_unit == UNIT_RD)
    src_no = 0;

  src_reg = lookup_reg_name (src_unit, src_no);

  pair_reg = lookup_pair_reg_name (src_unit, src_no);

  cond_set_addr_str (addr_buf, ADDR_WIDTH, insn_word);

  if (src_unit == UNIT_RD)
    print_set (buf, addr_buf, 4, src_reg, pair_reg, src_unit,
	       template, outf);
  else
    print_set (buf, addr_buf, size, src_reg, pair_reg, src_unit,
	       template, outf);
}

/* Print a MMOV instruction.  */
static void
print_mmov (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  unsigned int is_fpu = template->insn_type == INSN_FPU;
  unsigned int is_prime = ((MINOR_OPCODE (template->meta_opcode) & 0x2) &&
			   !is_fpu);
  unsigned int is_64bit = MINOR_OPCODE (template->meta_opcode) & 0x1;
  unsigned int is_dsp = template->meta_opcode & 0x1;
  unsigned int dest_unit, dest_no, rmask;
  char buf[OPERAND_WIDTH];
  char reg_buf[REG_WIDTH];
  char addr_buf[ADDR_WIDTH];

  if (is_fpu)
    dest_no = (insn_word >> 14) & REG_MASK;
  else
    dest_no = (insn_word >> 19) & REG_MASK;

  rmask = (insn_word >> 7) & RMASK_MASK;

  if (is_prime)
    {
      const char *dest_reg;
      const char *base_reg;
      unsigned int base_unit, base_no;
      int i, count = hweight (rmask);

      dest_reg = lookup_reg_name (UNIT_RD, dest_no);

      strcpy (reg_buf, dest_reg);

      for (i = 0; i < count; i++)
	{
	  strcat (reg_buf, ",");
	  strcat (reg_buf, dest_reg);
	}

      base_unit = short_unit ((insn_word >> 5) & SHORT_UNIT_MASK);
      base_no = (insn_word >> 14) & REG_MASK;

      base_reg = lookup_reg_name (base_unit, base_no);

      snprintf (addr_buf, ADDR_WIDTH, "[%s++]", base_reg);

      snprintf (buf, OPERAND_WIDTH, "%s,%s", reg_buf, addr_buf);
    }
  else
    {
      if (is_fpu)
	dest_unit = UNIT_FX;
      else
	dest_unit = short_unit ((insn_word >> 3) & SHORT_UNIT_MASK);

      lookup_reg_list (reg_buf, REG_WIDTH, dest_unit, dest_no, rmask,
		       is_fpu && is_64bit);

      snprintf (buf, OPERAND_WIDTH, "%s,RD", reg_buf);
    }

  if (is_dsp)
    {
      char prefix_buf[10] = {0};
      if (is_prime)
	{
	  if (dest_no == 22 || dest_no == 23)
	    strcpy (prefix_buf, "DB");
	  else if (dest_no == 24)
	    strcpy (prefix_buf, "DBH");
	  else if (dest_no == 25)
	    strcpy (prefix_buf, "DWH");
	  else if (dest_no == 31)
	    strcpy (prefix_buf, "DW");
	}
      else
	strcpy (prefix_buf, "DW");
      print_insn (outf, prefix_buf, template->name, buf);
    }
  else if (is_fpu)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Print an MDRD instruction.  */
static void
print_mdrd (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  unsigned int rmask, count;
  char buf[OPERAND_WIDTH];

  rmask = (insn_word >> 7) & RMASK_MASK;

  count = hweight (rmask);

  snprintf (buf, OPERAND_WIDTH, "#%#x", count + 1);

  print_insn (outf, "", template->name, buf);
}

/* Print an XFR instruction.  */
static void
print_xfr (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	   const insn_template *template,
	   disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char dest_buf[ADDR_WIDTH];
  char src_buf[ADDR_WIDTH];
  unsigned int dest_unit, src_unit;
  unsigned int dest_no, src_no;
  unsigned int us, ud, pp;
  const char *dest_base_reg;
  const char *dest_offset_reg;
  const char *src_base_reg;
  const char *src_offset_reg;

  src_unit = short_unit ((insn_word >> 2) & SHORT_UNIT_MASK);
  src_no = (insn_word >> 19) & REG_MASK;

  src_base_reg = lookup_reg_name (src_unit, src_no);

  src_no = (insn_word >> 14) & REG_MASK;

  src_offset_reg = lookup_reg_name (src_unit, src_no);

  dest_unit = short_unit (insn_word & SHORT_UNIT_MASK);
  dest_no = (insn_word >> 9) & REG_MASK;

  dest_base_reg = lookup_reg_name (dest_unit, dest_no);

  dest_no = (insn_word >> 4) & REG_MASK;

  dest_offset_reg = lookup_reg_name (dest_unit, dest_no);

  us = (insn_word >> 27) & 0x1;
  ud = (insn_word >> 26) & 0x1;
  pp = (insn_word >> 24) & 0x1;

  if (us)
    if (pp)
      snprintf (src_buf, ADDR_WIDTH, "[%s+%s++]", src_base_reg,
		src_offset_reg);
    else
      snprintf (src_buf, ADDR_WIDTH, "[%s++%s]", src_base_reg,
		src_offset_reg);
  else
    snprintf (src_buf, ADDR_WIDTH, "[%s+%s]", src_base_reg,
	      src_offset_reg);

  if (ud)
    if (pp)
      snprintf (dest_buf, ADDR_WIDTH, "[%s+%s++]", dest_base_reg,
		dest_offset_reg);
    else
      snprintf (dest_buf, ADDR_WIDTH, "[%s++%s]", dest_base_reg,
		dest_offset_reg);
  else
    snprintf (dest_buf, ADDR_WIDTH, "[%s+%s]", dest_base_reg,
	      dest_offset_reg);

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_buf, src_buf);

  print_insn (outf, "", template->name, buf);
}

/* Print a MOV to control unit instruction.  */
static void
print_mov_ct (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	      const insn_template *template,
	      disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int reg_no;
  unsigned int se = (insn_word >> 1) & 0x1;
  unsigned int is_trace = (insn_word >> 2) & 0x1;
  int value;
  const char *dest_reg;

  reg_no = (insn_word >> 19) & REG_MASK;

  if (is_trace)
    dest_reg = lookup_reg_name (UNIT_TT, reg_no);
  else
    dest_reg = lookup_reg_name (UNIT_CT, reg_no);

  value = (insn_word >> 3) & IMM16_MASK;

  if (se)
    {
      value = sign_extend (value, IMM16_BITS);
      snprintf (buf, OPERAND_WIDTH, "%s,#%d", dest_reg, value);
    }
  else
    {
      snprintf (buf, OPERAND_WIDTH, "%s,#%#x", dest_reg, value);
    }

  print_insn (outf, "", template->name, buf);
}

/* Print a SWAP instruction.  */
static void
print_swap (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int dest_no, src_no;
  unsigned int dest_unit, src_unit;
  const char *dest_reg;
  const char *src_reg;

  src_unit = (insn_word >> 10) & UNIT_MASK;
  src_no = (insn_word >> 19) & REG_MASK;

  src_reg = lookup_reg_name (src_unit, src_no);

  dest_unit = (insn_word >> 5) & UNIT_MASK;
  dest_no = (insn_word >> 14) & REG_MASK;

  dest_reg = lookup_reg_name (dest_unit, dest_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  if (dest_unit == UNIT_FX || src_unit == UNIT_FX)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Print a SWAP instruction.  */
static void
print_jump (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int reg_no, reg_unit;
  const char *reg_name;
  int value;

  reg_unit = short_unit (insn_word & SHORT_UNIT_MASK);
  reg_no = (insn_word >> 19) & REG_MASK;

  reg_name = lookup_reg_name (reg_unit, reg_no);

  value = (insn_word >> 3) & IMM16_MASK;

  snprintf (buf, OPERAND_WIDTH, "%s,#%#x", reg_name, value);

  print_insn (outf, "", template->name, buf);
}

/* Print a CALLR instruction.  */
static void
print_callr (unsigned int insn_word, bfd_vma pc, const insn_template *template,
	     disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int reg_no, reg_unit;
  const char *reg_name;
  int value;

  reg_unit = short_unit ((insn_word >> 3) & SHORT_UNIT_MASK);
  reg_no = insn_word & CALLR_REG_MASK;

  reg_name = lookup_reg_name (reg_unit, reg_no);

  value = (insn_word >> 5) & IMM19_MASK;

  value = sign_extend (value, IMM19_BITS);

  value = value * 4;

  value += pc;

  snprintf (buf, OPERAND_WIDTH, "%s,", reg_name);

  print_insn (outf, "", template->name, buf);

  outf->print_address_func (value, outf);
}

/* Print a GP ALU instruction.  */
static void
print_alu (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	   const insn_template *template,
	   disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int is_addr_op = MAJOR_OPCODE (template->meta_opcode) == OPC_ADDR;
  unsigned int is_mul = MAJOR_OPCODE (template->meta_opcode) == OPC_MUL;
  unsigned int dest_no, src1_no, src2_no;
  unsigned int imm = (insn_word >> 25) & 0x1;
  unsigned int cond = (insn_word >> 26) & 0x1;
  unsigned int o1z = 0;
  unsigned int o2r = insn_word & 0x1;
  unsigned int unit_bit = (insn_word >> 24) & 0x1;
  unsigned int ca = (insn_word >> 5) & 0x1;
  unsigned int se = (insn_word >> 1) & 0x1;
  bool is_quickrot = template->arg_type & GP_ARGS_QR;
  enum metag_unit base_unit;
  enum metag_unit dest_unit;
  const char *dest_reg;
  const char *src1_reg;
  const char *src2_reg;
  int value;

  if ((MAJOR_OPCODE (template->meta_opcode) == OPC_ADDR ||
      MAJOR_OPCODE (template->meta_opcode) == OPC_ADD ||
       MAJOR_OPCODE (template->meta_opcode) == OPC_SUB) &&
      ((insn_word >> 2) & 0x1))
    o1z = 1;

  if (is_addr_op)
    {
      if (unit_bit)
	base_unit = UNIT_A1;
      else
	base_unit = UNIT_A0;
    }
  else
    {
      if (unit_bit)
	base_unit = UNIT_D1;
      else
	base_unit = UNIT_D0;
    }

  dest_no = (insn_word >> 19) & REG_MASK;
  src1_no = (insn_word >> 14) & REG_MASK;
  src2_no = (insn_word >> 9) & REG_MASK;

  dest_unit = base_unit;

  if (imm)
    {
      if (cond)
	{
	  if (ca)
	    {
	      dest_unit = (insn_word >> 1) & UNIT_MASK;
	      dest_reg = lookup_reg_name (dest_unit, dest_no);
	    }
	  else
	      dest_reg = lookup_reg_name (dest_unit, dest_no);

	  src1_reg = lookup_reg_name (base_unit, src1_no);

	  value = (insn_word >> 6) & IMM8_MASK;

	  if (is_quickrot)
	    {
	      unsigned int qr_unit = unit_bit ? UNIT_A1 : UNIT_A0;
	      unsigned int qr_no = 2;
	      const char *qr_reg = lookup_reg_name (qr_unit, qr_no);

	      snprintf (buf, OPERAND_WIDTH, "%s,%s,#%#x,%s", dest_reg,
			src1_reg, value, qr_reg);
	    }
	  else
	    snprintf (buf, OPERAND_WIDTH, "%s,%s,#%#x", dest_reg,
		      src1_reg, value);
	}
      else
	{
	  if (is_addr_op && (dest_no & ~CPC_REG_MASK))
	    {
	      dest_reg = lookup_reg_name (dest_unit, dest_no & CPC_REG_MASK);
	      src1_reg = lookup_reg_name (base_unit, 0x10);
	    }
	  else
	    {
	      dest_reg = lookup_reg_name (dest_unit, dest_no);
	      src1_reg = lookup_reg_name (base_unit, dest_no);
	    }

	  value = (insn_word >> 3) & IMM16_MASK;

	  if (se)
	    {
	      value = sign_extend (value, IMM16_BITS);
	      if (o1z)
		{
		  snprintf (buf, OPERAND_WIDTH, "%s,#%d", dest_reg, value);
		}
	      else
		{
		  snprintf (buf, OPERAND_WIDTH, "%s,%s,#%d", dest_reg,
			    src1_reg, value);
		}
	    }
	  else
	    {
	      if (o1z)
		{
		  snprintf (buf, OPERAND_WIDTH, "%s,#%#x", dest_reg, value);
		}
	      else
		{
		  snprintf (buf, OPERAND_WIDTH, "%s,%s,#%#x", dest_reg,
			    src1_reg, value);
		}
	    }
	}
    }
  else
    {
      src1_reg = lookup_reg_name (base_unit, src1_no);

      if (o2r)
	src2_reg = lookup_o2r (base_unit, src2_no);
      else
	src2_reg = lookup_reg_name (base_unit, src2_no);

      if (cond)
	{
	  dest_unit = (insn_word >> 5) & UNIT_MASK;

	  if (is_mul)
	    {
	      if (ca)
		dest_unit = (insn_word >> 1) & UNIT_MASK;
	      else
		dest_unit = base_unit;
	    }

	  dest_reg = lookup_reg_name (dest_unit, dest_no);

	  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg,
		    src1_reg, src2_reg);
	}
      else
	{
	  dest_reg = lookup_reg_name (dest_unit, dest_no);

	  if (is_quickrot)
	    {
	      unsigned int qr_unit = unit_bit ? UNIT_A1 : UNIT_A0;
	      unsigned int qr_no = 2 + ((insn_word >> 7) & 0x1);
	      const char *qr_reg = lookup_reg_name (qr_unit, qr_no);

	      snprintf (buf, OPERAND_WIDTH, "%s,%s,%s,%s", dest_reg,
			src1_reg, src2_reg, qr_reg);
	    }
	  else if (o1z)
	    {
	      snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src2_reg);
	    }
	  else
	    {
	      snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg,
			src1_reg, src2_reg);
	    }
	}
    }

  if (dest_unit == UNIT_FX)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Print a B instruction.  */
static void
print_branch (unsigned int insn_word, bfd_vma pc,
	      const insn_template *template,
	      disassemble_info *outf)
{
  int value;

  value = (insn_word >> 5) & IMM19_MASK;

  value = sign_extend (value, IMM19_BITS);

  value = value * 4;

  value += pc;

  print_insn (outf, "", template->name, "");

  outf->print_address_func (value, outf);
}

/* Print a SWITCH instruction.  */
static void
print_switch (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	      const insn_template *template,
	      disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int value;

  value = insn_word & IMM24_MASK;

  snprintf (buf, OPERAND_WIDTH, "#%#x", value);

  print_insn (outf, "", template->name, buf);
}

/* Print a shift instruction.  */
static void
print_shift (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	     const insn_template *template,
	     disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int dest_no, src1_no, src2_no;
  unsigned int imm = (insn_word >> 25) & 0x1;
  unsigned int cond = (insn_word >> 26) & 0x1;
  unsigned int unit_bit = (insn_word >> 24) & 0x1;
  unsigned int ca = (insn_word >> 5) & 0x1;
  enum metag_unit base_unit;
  unsigned int dest_unit;
  const char *dest_reg;
  const char *src1_reg;
  const char *src2_reg;
  int value;

  if (unit_bit)
    base_unit = UNIT_D1;
  else
    base_unit = UNIT_D0;

  dest_no = (insn_word >> 19) & REG_MASK;
  src1_no = (insn_word >> 14) & REG_MASK;
  src2_no = (insn_word >> 9) & REG_MASK;

  dest_unit = base_unit;

  if (imm)
    {
      if (cond && ca)
	dest_unit = (insn_word >> 1) & UNIT_MASK;

      dest_reg = lookup_reg_name (dest_unit, dest_no);

      src1_reg = lookup_reg_name (base_unit, src1_no);

      value = (insn_word >> 9) & IMM5_MASK;

      snprintf (buf, OPERAND_WIDTH, "%s,%s,#%#x", dest_reg,
		src1_reg, value);
    }
  else
    {
      if (cond && ca)
	dest_unit = (insn_word >> 1) & UNIT_MASK;

      dest_reg = lookup_reg_name (dest_unit, dest_no);

      src1_reg = lookup_reg_name (base_unit, src1_no);
      src2_reg = lookup_reg_name (base_unit, src2_no);

      snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg,
		src1_reg, src2_reg);
    }

  if (dest_unit == UNIT_FX)
    print_insn (outf, "F", template->name, buf);
  else
    print_insn (outf, "", template->name, buf);
}

/* Print a MIN or MAX instruction.  */
static void
print_min_max (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	       const insn_template *template,
	       disassemble_info *outf)
{
  unsigned int base_unit, dest_no, src1_no, src2_no;
  char buf[OPERAND_WIDTH];
  const char *dest_reg;
  const char *src1_reg;
  const char *src2_reg;

  if ((insn_word >> 24) & UNIT_MASK)
    base_unit = UNIT_D1;
  else
    base_unit = UNIT_D0;

  dest_no = (insn_word >> 19) & REG_MASK;
  src1_no = (insn_word >> 14) & REG_MASK;
  src2_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (base_unit, dest_no);

  src1_reg = lookup_reg_name (base_unit, src1_no);
  src2_reg = lookup_reg_name (base_unit, src2_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, src1_reg, src2_reg);

  print_insn (outf, "", template->name, buf);
}

/* Print a bit operation instruction.  */
static void
print_bitop (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	     const insn_template *template,
	     disassemble_info *outf)
{
  unsigned int swap_inst = MAJOR_OPCODE (template->meta_opcode) == OPC_MISC;
  unsigned int base_unit, src_unit, dest_no, src_no;
  unsigned int is_bexl = 0;
  char buf[OPERAND_WIDTH];
  const char *dest_reg;
  const char *src_reg;

  if (swap_inst &&
      ((insn_word >> 1) & 0xb) == 0xa)
    is_bexl = 1;

  if (swap_inst)
    {
      if (insn_word & 0x1)
	base_unit = UNIT_D1;
      else
	base_unit = UNIT_D0;
    }
  else
    {
      if ((insn_word >> 24) & 0x1)
	base_unit = UNIT_D1;
      else
	base_unit = UNIT_D0;
    }

  src_unit = base_unit;

  if (is_bexl)
    base_unit = get_pair_unit (base_unit);

  dest_no = (insn_word >> 19) & REG_MASK;

  dest_reg = lookup_reg_name (base_unit, dest_no);

  src_no = (insn_word >> 14) & REG_MASK;

  src_reg = lookup_reg_name (src_unit, src_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  print_insn (outf, "", template->name, buf);
}

/* Print a CMP or TST instruction.  */
static void
print_cmp (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	   const insn_template *template,
	   disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int dest_no, src_no;
  unsigned int imm = (insn_word >> 25) & 0x1;
  unsigned int cond = (insn_word >> 26) & 0x1;
  unsigned int o2r = insn_word & 0x1;
  unsigned int unit_bit = (insn_word >> 24) & 0x1;
  unsigned int se = (insn_word >> 1) & 0x1;
  enum metag_unit base_unit;
  const char *dest_reg;
  const char *src_reg;
  int value;

  if (unit_bit)
    base_unit = UNIT_D1;
  else
    base_unit = UNIT_D0;

  dest_no = (insn_word >> 14) & REG_MASK;
  src_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (base_unit, dest_no);

  if (imm)
    {
      if (cond)
	{
	  value = (insn_word >> 6) & IMM8_MASK;

	  snprintf (buf, OPERAND_WIDTH, "%s,#%#x", dest_reg, value);
	}
      else
	{
	  dest_no = (insn_word >> 19) & REG_MASK;

	  dest_reg = lookup_reg_name (base_unit, dest_no);

	  value = (insn_word >> 3) & IMM16_MASK;

	  if (se)
	    {
	      value = sign_extend (value, IMM16_BITS);
	      snprintf (buf, OPERAND_WIDTH, "%s,#%d", dest_reg, value);
	    }
	  else
	    {
	      snprintf (buf, OPERAND_WIDTH, "%s,#%#x", dest_reg, value);
	    }
	}
    }
  else
    {
      if (o2r)
	src_reg = lookup_o2r (base_unit, src_no);
      else
	src_reg = lookup_reg_name (base_unit, src_no);

      snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);
    }

  print_insn (outf, "", template->name, buf);
}

/* Print a CACHER instruction.  */
static void
print_cacher (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	      const insn_template *template,
	      disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  unsigned int reg_unit, reg_no;
  unsigned int size = ((insn_word >> 1) & 0x1) ? 8 : 4;
  const char *reg_name;
  const char *pair_name;

  reg_unit = short_unit ((insn_word >> 3) & SHORT_UNIT_MASK);
  reg_no = (insn_word >> 19) & REG_MASK;

  reg_name = lookup_reg_name (reg_unit, reg_no);
  pair_name = lookup_pair_reg_name (reg_unit, reg_no);

  cache_addr_str (addr_buf, ADDR_WIDTH, insn_word, size);

  if (size == 8)
    snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", reg_name, pair_name, addr_buf);
  else
    snprintf (buf, OPERAND_WIDTH, "%s,%s", reg_name, addr_buf);

  print_insn (outf, "", template->name, buf);
}

/* Print a CACHEW instruction.  */
static void
print_cachew (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	      const insn_template *template,
	      disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  unsigned int reg_unit, reg_no;
  unsigned int size = ((insn_word >> 1) & 0x1) ? 8 : 4;
  const char *reg_name;
  const char *pair_name;

  reg_unit = short_unit ((insn_word >> 3) & SHORT_UNIT_MASK);
  reg_no = (insn_word >> 19) & REG_MASK;

  reg_name = lookup_reg_name (reg_unit, reg_no);
  pair_name = lookup_pair_reg_name (reg_unit, reg_no);

  cache_addr_str (addr_buf, ADDR_WIDTH, insn_word, 64);

  if (size == 8)
    snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", addr_buf, reg_name, pair_name);
  else
    snprintf (buf, OPERAND_WIDTH, "%s,%s", addr_buf, reg_name);

  print_insn (outf, "", template->name, buf);
}

/* Print an ICACHE instruction.  */
static void
print_icache (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	      const insn_template *template,
	      disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  int offset;
  int pfcount;

  offset = ((insn_word >> 9) & IMM15_MASK);
  pfcount = ((insn_word >> 1) & IMM4_MASK);

  offset = sign_extend (offset, IMM15_BITS);

  if (pfcount)
    snprintf (buf, OPERAND_WIDTH, "#%d,#0x%x", offset, pfcount);
  else
    snprintf (buf, OPERAND_WIDTH, "#%d,#0", offset);
  print_insn (outf, "", template->name, buf);
}

/* Print a LNKGET instruction.  */
static void
print_lnkget (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	      const insn_template *template,
	      disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  unsigned int reg_unit, reg_no;
  unsigned int size = metag_get_set_ext_size_bytes (insn_word);
  const char *reg_name;
  const char *pair_name;

  reg_unit = short_unit ((insn_word >> 3) & SHORT_UNIT_MASK);
  reg_no = (insn_word >> 19) & REG_MASK;

  reg_name = lookup_reg_name (reg_unit, reg_no);
  pair_name = lookup_pair_reg_name (reg_unit, reg_no);

  cache_addr_str (addr_buf, ADDR_WIDTH, insn_word, size);

  if (size == 8)
    snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", reg_name, pair_name, addr_buf);
  else
    snprintf (buf, OPERAND_WIDTH, "%s,%s", reg_name, addr_buf);

  print_insn (outf, "", template->name, buf);
}

/* Print an FPU MOV instruction.  */
static void
print_fmov (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix_buf[10];
  unsigned int src_no, dest_no;
  unsigned int p = (insn_word >> 6) & 0x1;
  unsigned int d = (insn_word >> 5) & 0x1;
  unsigned int cc = (insn_word >> 1) & CC_MASK;
  bool show_cond = cc != COND_A && cc != COND_NV;
  const char *dest_reg;
  const char *src_reg;
  const char *cc_flags;

  dest_no = (insn_word >> 19) & REG_MASK;
  src_no = (insn_word >> 14) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src_reg = lookup_reg_name (UNIT_FX, src_no);

  cc_flags = lookup_fpu_scc_flags (cc);

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  snprintf (prefix_buf, 10, "F%s%s%s", p ? "L" : "",
	    d ? "D" : "", show_cond ? cc_flags : "");

  print_insn (outf, prefix_buf, template->name, buf);
}

/* Convert an FPU rmask into a compatible form. */
static unsigned int
convert_fx_rmask (unsigned int rmask)
{
  int num_bits = hweight (rmask), i;
  unsigned int ret = 0;

  for (i = 0; i < num_bits; i++)
    {
      ret <<= 1;
      ret |= 0x1;
    }

  return ret;
}

/* Print an FPU MMOV instruction.  */
static void
print_fmmov (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  /* We used to have buf[OPERAND_WIDTH] here, but gcc v8 complains
     about the snprintf()s below possibly truncating the output.
     (There is no way to tell gcc that this truncation is intentional).
     So now we use an extra wide buffer.  */
  char buf[OPERAND_WIDTH * 2];
  char data_buf[REG_WIDTH];
  char fpu_buf[REG_WIDTH];
  bool to_fpu = MAJOR_OPCODE (insn_word) == OPC_GET;
  bool is_mmovl = MINOR_OPCODE (insn_word) & 0x1;
  unsigned int rmask = (insn_word >> 7) & RMASK_MASK;
  unsigned int fpu_no, data_no, data_unit;

  data_no = (insn_word >> 19) & REG_MASK;
  fpu_no = (insn_word >> 14) & REG_MASK;

  if (insn_word & 0x1)
    data_unit = UNIT_D1;
  else
    data_unit = UNIT_D0;

  lookup_reg_list (data_buf, REG_WIDTH, data_unit, data_no, rmask, false);
  lookup_reg_list (fpu_buf, REG_WIDTH, UNIT_FX, fpu_no,
		   convert_fx_rmask (rmask), is_mmovl);

  if (to_fpu)
    snprintf (buf, sizeof buf, "%s,%s", fpu_buf, data_buf);
  else
    snprintf (buf, sizeof buf, "%s,%s", data_buf, fpu_buf);

  print_insn (outf, "F", template->name, buf);
}

/* Print an FPU data unit MOV instruction.  */
static void
print_fmov_data (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		 const insn_template *template,
		 disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int src_no, dest_no;
  unsigned int to_fpu = ((insn_word >> 7) & 0x1);
  unsigned int unit_bit = (insn_word >> 24) & 0x1;
  enum metag_unit base_unit;
  const char *dest_reg;
  const char *src_reg;

  dest_no = (insn_word >> 19) & REG_MASK;
  src_no = (insn_word >> 9) & REG_MASK;

  if (unit_bit)
    base_unit = UNIT_D1;
  else
    base_unit = UNIT_D0;

  if (to_fpu)
    {
      dest_reg = lookup_reg_name (UNIT_FX, dest_no);
      src_reg = lookup_reg_name (base_unit, src_no);
    }
  else
    {
      dest_reg = lookup_reg_name (base_unit, dest_no);
      src_reg = lookup_reg_name (UNIT_FX, src_no);
    }

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  print_insn (outf, "F", template->name, buf);
}

/* Print an FPU MOV immediate instruction.  */
static void
print_fmov_i (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	      const insn_template *template,
	      disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int dest_no;
  unsigned int p = (insn_word >> 2) & 0x1;
  unsigned int d = (insn_word >> 1) & 0x1;
  const char *dest_reg;
  unsigned int value = (insn_word >> 3) & IMM16_MASK;

  dest_no = (insn_word >> 19) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);

  snprintf (buf, OPERAND_WIDTH, "%s,#%#x", dest_reg, value);

  if (p)
    print_insn (outf, "FL", template->name, buf);
  else if (d)
    print_insn (outf, "FD", template->name, buf);
  else
    print_insn (outf, "F", template->name, buf);
}

/* Print an FPU PACK instruction.  */
static void
print_fpack (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	     const insn_template *template,
	     disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int src1_no, src2_no, dest_no;
  const char *dest_reg;
  const char *src1_reg;
  const char *src2_reg;

  dest_no = (insn_word >> 19) & REG_MASK;
  src1_no = (insn_word >> 14) & REG_MASK;
  src2_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src1_reg = lookup_reg_name (UNIT_FX, src1_no);
  src2_reg = lookup_reg_name (UNIT_FX, src2_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, src1_reg, src2_reg);

  print_insn (outf, "F", template->name, buf);
}

/* Print an FPU SWAP instruction.  */
static void
print_fswap (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	     const insn_template *template,
	     disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int src_no, dest_no;
  const char *dest_reg;
  const char *src_reg;

  dest_no = (insn_word >> 19) & REG_MASK;
  src_no = (insn_word >> 14) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src_reg = lookup_reg_name (UNIT_FX, src_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  print_insn (outf, "FL", template->name, buf);
}

/* Print an FPU CMP instruction.  */
static void
print_fcmp (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix_buf[10];
  unsigned int src_no, dest_no;
  unsigned int a = (insn_word >> 19) & 0x1;
  unsigned int z = (insn_word >> 8) & 0x1;
  unsigned int p = (insn_word >> 6) & 0x1;
  unsigned int d = (insn_word >> 5) & 0x1;
  unsigned int q = (insn_word >> 7) & 0x1;
  unsigned int cc = (insn_word >> 1) & CC_MASK;
  bool show_cond = cc != COND_A && cc != COND_NV;
  const char *dest_reg;
  const char *src_reg;
  const char *cc_flags;

  dest_no = (insn_word >> 14) & REG_MASK;
  src_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src_reg = lookup_reg_name (UNIT_FX, src_no);

  cc_flags = lookup_fpu_scc_flags (cc);

  if (z)
    snprintf (buf, OPERAND_WIDTH, "%s,#0", dest_reg);
  else
    snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  snprintf (prefix_buf, 10, "F%s%s%s%s%s", p ? "L" : "",
	    d ? "D" : "", a ? "A" : "", q ? "Q" : "",
	    show_cond ? cc_flags : "");

  print_insn (outf, prefix_buf, template->name, buf);
}

/* Print an FPU MIN or MAX instruction.  */
static void
print_fminmax (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	       const insn_template *template,
	       disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix_buf[10];
  unsigned int p = (insn_word >> 6) & 0x1;
  unsigned int d = (insn_word >> 5) & 0x1;
  unsigned int src1_no, src2_no, dest_no;
  unsigned int cc = (insn_word >> 1) & CC_MASK;
  bool show_cond = cc != COND_A && cc != COND_NV;
  const char *dest_reg;
  const char *src1_reg;
  const char *src2_reg;
  const char *cc_flags;

  dest_no = (insn_word >> 19) & REG_MASK;
  src1_no = (insn_word >> 14) & REG_MASK;
  src2_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src1_reg = lookup_reg_name (UNIT_FX, src1_no);
  src2_reg = lookup_reg_name (UNIT_FX, src2_no);

  cc_flags = lookup_fpu_scc_flags (cc);

  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, src1_reg, src2_reg);

  snprintf (prefix_buf, 10, "F%s%s%s", p ? "L" : "",
	    d ? "D" : "", show_cond ? cc_flags : "");

  print_insn (outf, prefix_buf, template->name, buf);
}

/* Print an FPU data conversion instruction.  */
static void
print_fconv (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	     const insn_template *template,
	     disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix_buf[10];
  unsigned int p = (insn_word >> 6) & 0x1;
  unsigned int z = (insn_word >> 12) & 0x1;
  unsigned int src_no, dest_no;
  unsigned int cc = (insn_word >> 1) & CC_MASK;
  bool show_cond = cc != COND_A && cc != COND_NV;
  const char *dest_reg;
  const char *src_reg;
  const char *cc_flags;

  dest_no = (insn_word >> 19) & REG_MASK;
  src_no = (insn_word >> 14) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src_reg = lookup_reg_name (UNIT_FX, src_no);

  cc_flags = lookup_fpu_scc_flags (cc);

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  snprintf (prefix_buf, 10, "F%s%s%s", p ? "L" : "",
	    z ? "Z" : "", show_cond ? cc_flags : "");

  print_insn (outf, prefix_buf, template->name, buf);
}

/* Print an FPU extended data conversion instruction.  */
static void
print_fconvx (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	      const insn_template *template,
	      disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix_buf[10];
  unsigned int p = (insn_word >> 6) & 0x1;
  unsigned int xl = (insn_word >> 7) & 0x1;
  unsigned int src_no, dest_no, fraction_bits;
  unsigned int cc = (insn_word >> 1) & CC_MASK;
  bool show_cond = cc != COND_A && cc != COND_NV;
  const char *dest_reg;
  const char *src_reg;
  const char *cc_flags;

  dest_no = (insn_word >> 19) & REG_MASK;
  src_no = (insn_word >> 14) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src_reg = lookup_reg_name (UNIT_FX, src_no);

  cc_flags = lookup_fpu_scc_flags (cc);

  if (xl)
    fraction_bits = (insn_word >> 8) & IMM6_MASK;
  else
    fraction_bits = (insn_word >> 9) & IMM5_MASK;

  snprintf (buf, OPERAND_WIDTH, "%s,%s,#%#x", dest_reg, src_reg,
	    fraction_bits);

  snprintf (prefix_buf, 10, "F%s%s", p ? "L" : "",
	    show_cond ? cc_flags : "");

  print_insn (outf, prefix_buf, template->name, buf);
}

/* Print an FPU basic arithmetic instruction.  */
static void
print_fbarith (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	       const insn_template *template,
	       disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix_buf[10];
  unsigned int n = (insn_word >> 7) & 0x1;
  unsigned int p = (insn_word >> 6) & 0x1;
  unsigned int d = (insn_word >> 5) & 0x1;
  unsigned int src1_no, src2_no, dest_no;
  unsigned int cc = (insn_word >> 1) & CC_MASK;
  bool show_cond = cc != COND_A && cc != COND_NV;
  const char *dest_reg;
  const char *src1_reg;
  const char *src2_reg;
  const char *cc_flags;

  dest_no = (insn_word >> 19) & REG_MASK;
  src1_no = (insn_word >> 14) & REG_MASK;
  src2_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src1_reg = lookup_reg_name (UNIT_FX, src1_no);
  src2_reg = lookup_reg_name (UNIT_FX, src2_no);

  cc_flags = lookup_fpu_scc_flags (cc);

  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, src1_reg, src2_reg);

  snprintf (prefix_buf, 10, "F%s%s%s%s", p ? "L" : "",
	    d ? "D" : "", n ? "I" : "", show_cond ? cc_flags : "");

  print_insn (outf, prefix_buf, template->name, buf);
}

/* Print an FPU extended arithmetic instruction.  */
static void
print_fearith (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	       const insn_template *template,
	       disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix_buf[10];
  bool is_muz = MINOR_OPCODE (insn_word) == 0x6 && ((insn_word >> 4) & 0x1);
  bool is_mac = MINOR_OPCODE (insn_word) == 0x6 && (insn_word & 0x1f) == 0;
  bool is_maw = MINOR_OPCODE (insn_word) == 0x6 && ((insn_word >> 3) & 0x1);
  unsigned int o3o = insn_word & 0x1;
  unsigned int q = is_muz && ((insn_word >> 1) & 0x1);
  unsigned int n = (insn_word >> 7) & 0x1;
  unsigned int p = (insn_word >> 6) & 0x1;
  unsigned int d = (insn_word >> 5) & 0x1;
  unsigned int cc = (insn_word >> 1) & CC_MASK;
  bool show_cond = (MINOR_OPCODE (insn_word) == 0x5 && cc != COND_A
		    && cc != COND_NV);
  unsigned int src1_no, src2_no, dest_no;
  const char *dest_reg;
  const char *src1_reg;
  const char *src2_reg;
  const char *cc_flags;

  dest_no = (insn_word >> 19) & REG_MASK;
  src1_no = (insn_word >> 14) & REG_MASK;
  src2_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src1_reg = lookup_reg_name (UNIT_FX, src1_no);
  src2_reg = lookup_reg_name (UNIT_FX, src2_no);

  cc_flags = lookup_fpu_scc_flags (cc);

  if (is_mac)
    snprintf (buf, OPERAND_WIDTH, "ACF.0,%s,%s", src1_reg, src2_reg);
  else if (o3o && is_maw)
    snprintf (buf, OPERAND_WIDTH, "%s,%s", src1_reg, src2_reg);
  else
    snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, src1_reg, src2_reg);

  snprintf (prefix_buf, 10, "F%s%s%s%s%s", p ? "L" : "",
	    d ? "D" : "", n ? "I" : "", q ? "Q" : "",
	    show_cond ? cc_flags : "");

  print_insn (outf, prefix_buf, template->name, buf);
}

/* Print an FPU RCP or RSQ instruction.  */
static void
print_frec (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix_buf[10];
  unsigned int z = (insn_word >> 10) & 0x1;
  unsigned int q = (insn_word >> 9) & 0x1;
  unsigned int n = (insn_word >> 7) & 0x1;
  unsigned int p = (insn_word >> 6) & 0x1;
  unsigned int d = (insn_word >> 5) & 0x1;
  unsigned int src_no, dest_no;
  const char *dest_reg;
  const char *src_reg;

  dest_no = (insn_word >> 19) & REG_MASK;
  src_no = (insn_word >> 14) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src_reg = lookup_reg_name (UNIT_FX, src_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s", dest_reg, src_reg);

  snprintf (prefix_buf, 10, "F%s%s%s%s%s", p ? "L" : "",
	    d ? "D" : "", n ? "I" : "", q ? "Q" : "", z ? "Z" : "");

  print_insn (outf, prefix_buf, template->name, buf);
}

static void
print_fsimd (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	     const insn_template *template,
	     disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  unsigned int n = (insn_word >> 7) & 0x1;
  unsigned int src1_no, src2_no, dest_no;
  const char *dest_reg;
  const char *src1_reg;
  const char *src2_reg;

  dest_no = (insn_word >> 19) & REG_MASK;
  src1_no = (insn_word >> 14) & REG_MASK;
  src2_no = (insn_word >> 9) & REG_MASK;

  dest_reg = lookup_reg_name (UNIT_FX, dest_no);
  src1_reg = lookup_reg_name (UNIT_FX, src1_no);
  src2_reg = lookup_reg_name (UNIT_FX, src2_no);

  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", dest_reg, src1_reg, src2_reg);

  if (n)
    print_insn (outf, "FLI", template->name, buf);
  else
    print_insn (outf, "FL", template->name, buf);
}

/* Print an FPU accumulator GET or SET instruction.  */
static void
print_fget_set_acf (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		    const insn_template *template,
		    disassemble_info *outf)
{
  bool is_get = MAJOR_OPCODE (template->meta_opcode) == OPC_GET;
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  unsigned int part;
  const char *reg_name;

  part = (insn_word >> 19) & ACF_PART_MASK;

  reg_name = lookup_acf_name (part);

  mget_mset_addr_str (addr_buf, ADDR_WIDTH, insn_word);

  if (is_get)
    {
      snprintf (buf, OPERAND_WIDTH, "%s,%s", reg_name, addr_buf);
    }
  else
    {
      snprintf (buf, OPERAND_WIDTH, "%s,%s", addr_buf, reg_name);
    }
  print_insn (outf, "F", template->name, buf);
}

/* Return the name of the DSP register or accumulator for NUM and UNIT.  */
static const char *
__lookup_dsp_name (unsigned int num, unsigned int unit)
{
  size_t i;

  for (i = 0; i < sizeof(metag_dsp_regtab)/sizeof(metag_dsp_regtab[0]); i++)
    {
      const metag_reg *reg = &metag_dsp_regtab[i];

      if (reg->no == num)
	{
	  if ((reg->unit == UNIT_RAM_D0 || reg->unit == UNIT_ACC_D0) &&
	      unit == UNIT_D0)
	    return reg->name;

	  if ((reg->unit == UNIT_RAM_D1 || reg->unit == UNIT_ACC_D1) &&
	      unit == UNIT_D1)
	    return reg->name;
	}
    }
  return "?.?";
}

/* Return the name of the DSP register for NUM and UNIT.  */
static const char *
lookup_dsp_name (unsigned int num, unsigned int unit)
{
  size_t i;

  for (i = 0; i < sizeof(metag_dsp_regtab)/sizeof(metag_dsp_regtab[0]); i++)
    {
      const metag_reg *reg = &metag_dsp_regtab[i];

      if (reg->no == num && reg->unit == unit)
	return reg->name;
    }
  return "?.?";
}

/* Return the name of the DSP RAM register for NUM and UNIT.  */
static const char *
lookup_dspram_name (unsigned int num, unsigned int unit, bool load)
{
  size_t i, nentries;

  nentries = sizeof(metag_dsp_tmpl_regtab[load])/sizeof(metag_dsp_tmpl_regtab[load][0]);

  for (i = 0; i < nentries; i++)
    {
      const metag_reg *reg = &metag_dsp_tmpl_regtab[load][i];

      if (reg->no == num && reg->unit == unit)
	return reg->name;
    }
  return "?.?";
}

/* This lookup function looks up the corresponding name for a register
   number in a DSP instruction. SOURCE indicates whether this
   register is a source or destination operand.  */
static const char *
lookup_any_reg_name (unsigned int unit, unsigned int num, bool source)
{
  /* A register with the top bit set (5th bit) indicates a DSPRAM
     register.  */
  if (num > 15)
    {
      unsigned int dunit = (unit == UNIT_D0) ? UNIT_RAM_D0 : UNIT_RAM_D1;
      return lookup_dspram_name (num, dunit, source);
    }
  else
    return lookup_reg_name (unit, num);
}

/* Return the DSP data unit for UNIT.  */
static inline enum metag_unit
dsp_data_unit_to_sym (unsigned int unit)
{
  if (unit == 0)
    return UNIT_D0;
  else
    return UNIT_D1;
}

/* Print a DSP GET or SET instruction.  */
static void
print_dget_set (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		const insn_template *template,
		disassemble_info *outf)
{
  bool is_get = (template->meta_opcode & 0x100);
  char buf[OPERAND_WIDTH];
  char addr_buf[ADDR_WIDTH];
  char prefix[DSP_PREFIX_WIDTH];
  unsigned int part;
  const char *reg_name[2];
  bool is_high = false;
  bool is_dual = (insn_word & 0x4);
  bool is_template = (insn_word & 0x2);
  const char *base_reg = "?";
  unsigned int addr_unit, base_no, unit;

  unit = dsp_data_unit_to_sym (insn_word & 0x1);

  /* Is this a load/store to a template table?  */
  if (is_template)
    {
      part = (insn_word >> 19) & 0x1f;
      reg_name[0] = lookup_dsp_name (part, UNIT_DT);
    }
  else
    {
      part = (insn_word >> 19) & REG_MASK;
      is_high = ((part & 0x18) == 0x18);

      /* Strip bit high indicator.  */
      if (is_high)
	part &= 0x17;

      reg_name[0] = __lookup_dsp_name (part, unit);

    }

  /* Is this a dual unit DSP operation?  The modulo operator below
     makes sure that we print the Rd register in the correct order,
     e.g. because there's only one bit in the instruction for the Data
     Unit we have to work out what the other data unit number is.
     (there's only 2).  */
  if (is_dual)
    {
      unsigned int _unit = insn_word & 0x1;

      _unit = ((_unit + 1) % 2);
      reg_name[1] = __lookup_dsp_name(part, dsp_data_unit_to_sym (_unit));
    }
  else
    reg_name[1] = NULL;

  addr_unit = ((insn_word >> 18) & 0x1);
  if (addr_unit == 0)
	  addr_unit = UNIT_A0;
  else
	  addr_unit = UNIT_A1;

  base_no = (insn_word >> 14) & DSP_REG_MASK;

  base_reg = lookup_reg_name (addr_unit, base_no);

  /* Check if it's a post-increment/post-decrement.  */
  if (insn_word & 0x2000)
  {
	  unsigned int imm = (insn_word >> 9) & DGET_SET_IMM_MASK;
	  const char *post_op;

	  switch (imm)
	    {
	    case 0x1:
	      post_op = "++";
	      break;
	    case 0x3:
	      post_op = "--";
	      break;
	    default:
	      post_op = "";
	    }

	  snprintf (addr_buf, ADDR_WIDTH, "[%s%s]", base_reg, post_op);
  }
  else
  {
	  unsigned int offset_part = (insn_word >> 9) & DSP_REG_MASK;
	  const char *offset_reg = lookup_reg_name (addr_unit, offset_part);

	  snprintf (addr_buf, ADDR_WIDTH, "[%s+%s++]", base_reg, offset_reg);
  }

  if (is_get)
    {
      if (is_dual && !is_template)
	snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", reg_name[0],
		  reg_name[1], addr_buf);
      else
	snprintf (buf, OPERAND_WIDTH, "%s,%s", reg_name[0], addr_buf);
    }
  else
    {
      if (is_dual && !is_template)
	snprintf (buf, OPERAND_WIDTH, "%s,%s,%s", addr_buf,
		  reg_name[0], reg_name[1]);
      else
	snprintf (buf, OPERAND_WIDTH, "%s,%s", addr_buf, reg_name[0]);
    }

  snprintf (prefix, DSP_PREFIX_WIDTH, "D%s", is_high ? "H" : "");
  print_insn (outf, prefix, template->name, buf);
}

/* Print a DSP template instruction.  */
static void
print_dtemplate (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
		 const insn_template *template,
		 disassemble_info *outf)
{
  char buf[OPERAND_WIDTH];
  char prefix[DSP_PREFIX_WIDTH];
  unsigned int offset[4];
  bool is_half = (MINOR_OPCODE (insn_word) == 0x5);
  bool daop_only = (MINOR_OPCODE (insn_word) == 0x3);

  offset[0] = ((insn_word >> 19) & REG_MASK);
  offset[1] = ((insn_word >> 14) & REG_MASK);
  offset[2] = ((insn_word >> 9) & REG_MASK);
  offset[3] = ((insn_word >> 4) & REG_MASK);

  if (daop_only)
	  snprintf (buf, OPERAND_WIDTH, "#0x%x,#0x%x,#0x%x", offset[0],
		    offset[1], offset[2]);
  else
    {
      snprintf (buf, OPERAND_WIDTH, "#0x%x,#0x%x,#0x%x,#0x%x", offset[0],
		offset[1], offset[2], offset[3]);
    }

  snprintf (prefix, DSP_PREFIX_WIDTH, "D%s", is_half ? "H" : "");
  print_insn (outf, prefix, template->name, buf);
}

/* Format template definition from INSN_WORD into BUF.  */
static void
decode_template_definition(unsigned int insn_word, char *buf, size_t len)
{
  bool load = ((insn_word >> 13) & 0x1);
  bool dspram = (((insn_word >> 17) & 0x3) == 0x3);
  const char *template[1];
  unsigned int tidx = ((insn_word >> 9) & TEMPLATE_REGS_MASK);
  enum metag_unit au, ram_unit;
  unsigned int addr_reg_nums[2];
  const char *addr_reg_names[2];
  const char *post_op = "";
  const char *join_op = "";
  enum metag_unit data_unit = ((insn_word >> 24) & 0x1) ? UNIT_D1 : UNIT_D0;

  template[0] = lookup_dsp_name (tidx, UNIT_DT);

  addr_reg_names[1] = "";

  if (dspram)
    {
      ram_unit = (data_unit == UNIT_D0) ? UNIT_RAM_D0 : UNIT_RAM_D1;
      addr_reg_nums[0] = ((insn_word >> 19) & REG_MASK);
      addr_reg_names[0] = lookup_dspram_name (addr_reg_nums[0],
					      ram_unit, load);
    }
  else
    {
      bool im = (((insn_word >> 18) & 0x1) != 0);

      au = (((insn_word >> 23) & 0x1) == 0) ? UNIT_A0 : UNIT_A1;
      addr_reg_nums[0] = ((insn_word >> 19) & DSP_REG_MASK);

      addr_reg_names[0] = lookup_reg_name (au, addr_reg_nums[0]);

      if (im)
	{
	  unsigned int im_value = ((insn_word >> 14) & 0x3);

	  switch (im_value)
	    {
	    case 0x1:
	      post_op = "++";
	      break;
	    case 0x3:
	      post_op = "--";
	      break;
	    }
	}
      else
	{
	  addr_reg_nums[1] = ((insn_word >> 14) & DSP_REG_MASK);
	  addr_reg_names[1] = lookup_reg_name (au, addr_reg_nums[1]);
	  join_op = "+";
	  post_op = "++";
	}
    }

  if (load)
    {
      len = snprintf (buf, len, " %s,[%s%s%s%s]", template[0], addr_reg_names[0],
		      join_op, addr_reg_names[1], post_op);
    }
  else
    {
      len = snprintf (buf, len, " [%s%s%s%s],%s", addr_reg_names[0], join_op,
		      addr_reg_names[1], post_op, template[0]);
    }
}

/* Print a DSP ALU instruction.  */
static void
print_dalu (unsigned int insn_word, bfd_vma pc ATTRIBUTE_UNUSED,
	    const insn_template *template,
	    disassemble_info *outf)
{
  bool is_dual = false;
  unsigned int data_unit = (((insn_word >> 24) & 0x1) ? UNIT_D1 : UNIT_D0);
  const char *reg_names[3];
  unsigned int reg_nums[3];
  bool ac = ((insn_word >> 7) & 0x1);
  char buf[OPERAND_WIDTH];
  char prefix[DSP_PREFIX_WIDTH];
  size_t len;
  bool is_mod = false;
  bool is_overflow = false;
  unsigned int reg_brackets[3];
  bool is_w_mx = false;
  bool is_b_mx = false;
  bool imm = false;
  bool is_quickrot64 = false;
  bool conditional = false;
  const char *cc_flags = NULL;
  bool is_unsigned = false;

  memset (reg_brackets, 0, sizeof (reg_brackets));

  if (template->arg_type & DSP_ARGS_1)
    {
      bool is_template = false;
      const char *addr_reg = NULL;
      bool qr = false;
      bool is_acc_add = false;
      bool is_acc_sub = false;
      bool is_acc_zero = false;
      bool is_split8 = (template->arg_type & DSP_ARGS_SPLIT8);

      /* Read DU bit.  */
      data_unit = ((insn_word >> 24) & 0x1) ? UNIT_D1 : UNIT_D0;

      conditional = ((insn_word >> 24) & 0x4);

      /* Templates can't be conditional.  */
      is_template = (((insn_word & 0x02000002) == 0x2) && !conditional);

      if (is_split8)
	is_mod = (insn_word & 0x80);

      if (template->arg_type & DSP_ARGS_QR)
	{
	  if (!conditional)
	    is_quickrot64 = ((insn_word >> 5) & 0x1);
	}

      if (template->arg_type & DSP_ARGS_DACC)
	{
	  is_mod = (insn_word & 0x8);
	  is_unsigned = (insn_word & 0x40);
	}

      if (is_template)
	{
	  is_w_mx = (insn_word & 0x1);
	  is_dual = ((insn_word >> 0x4) & 0x1);

	  /* De.r,Dx.r,De.r|ACe.r */
	  if (template->arg_type & DSP_ARGS_ACC2)
	    {
	      is_mod = (insn_word & 0x8);
	      is_overflow = (insn_word & 0x20);
	    }

	  /* ACe.e,ACx.r,ACo.e? */
	  if ((template->arg_type & DSP_ARGS_XACC) &&
	      (((insn_word >> 6) & 0x5) == 0x5))
	    {
	      enum metag_unit ac_unit, ao_unit;

	      ac_unit = (data_unit == UNIT_D0) ? UNIT_ACC_D0 : UNIT_ACC_D1;

	      if (ac_unit == UNIT_ACC_D0)
		ao_unit = UNIT_ACC_D1;
	      else
		ao_unit = UNIT_ACC_D0;

	      reg_nums[1] = ((insn_word >> 19) & REG_MASK);

	      /* These are dummy arguments anyway so the register
		 number does not matter.  */
	      reg_names[0] = lookup_dsp_name (16, ac_unit); /* ACe.0 */
	      reg_names[1] = lookup_dsp_name (16, ac_unit); /* ACx.0 */
	      reg_names[2] = lookup_dsp_name (16, ao_unit); /* ACo.0 */
	    }
	  else
	    {
	      /* De.r|ACe.r,Dx.r,De.r */
	      if (template->arg_type & DSP_ARGS_DACC &&
		  ((insn_word & 0x84) != 0))
		{
		  enum metag_unit ac_unit;

		  ac_unit = (data_unit == UNIT_D0) ? UNIT_ACC_D0 : UNIT_ACC_D1;
		  reg_names[0] = lookup_dsp_name (16, ac_unit);

		  is_acc_zero = ((insn_word & 0x84) == 0x04);
		  is_acc_add = ((insn_word & 0x84) == 0x80);
		  is_acc_sub = ((insn_word & 0x84) == 0x84);
		}
	      else
		reg_names[0] = lookup_any_reg_name (data_unit, 0, false);

	      /* These are dummy arguments anyway so the register
		 number does not matter.  */
	      reg_names[1] = lookup_any_reg_name (data_unit, 0, true);

	      /* De.r,Dx.r,De.r|ACe.r */
	      if ((template->arg_type & DSP_ARGS_ACC2) &&
		  ((insn_word & 0x80) == 0x80))
		{
		  enum metag_unit ac_unit;

		  ac_unit = (data_unit == UNIT_D0) ? UNIT_ACC_D0 : UNIT_ACC_D1;
		  reg_names[2] = lookup_dsp_name (16, ac_unit);
		}
	      /* Detection of QUICKRoT and accumulator usage uses the
		 same bits. They are mutually exclusive.  */
	      else if (ac && (template->arg_type & DSP_ARGS_ACC2))
		{
		  reg_nums[2] = ((insn_word >> 9) & REG_MASK);

		  if (data_unit == UNIT_D0)
		    reg_names[2] = lookup_dsp_name (reg_nums[2], UNIT_ACC_D0);
		  else
		    reg_names[2] = lookup_dsp_name (reg_nums[2], UNIT_ACC_D1);
		}
	      else
		{
		  if ((template->arg_type & DSP_ARGS_QR) &&
		      ((insn_word & 0x40) == 0x40))
		    {
		      enum metag_unit aunit;
		      int reg_no;

		      if (conditional)
			reg_no = ((insn_word >> 5) & 0x1);
		      else
			reg_no = ((insn_word >> 7) & 0x1);

		      aunit = (data_unit == UNIT_D0) ? UNIT_A0 : UNIT_A1;
		      addr_reg = lookup_reg_name (aunit, reg_no + 2);

		      qr = true;
		    }

		  reg_names[2] = lookup_any_reg_name (data_unit, 0, true);
		}
	    }

	  if (qr)
	    {
	      len = snprintf (buf, OPERAND_WIDTH, "%s,%s,%s,%s",
			      reg_names[0], reg_names[1], reg_names[2],
			      addr_reg);
	    }
	  else
	    {
	      len = snprintf (buf, OPERAND_WIDTH, "%s,%s,%s%s%s",
			      reg_names[0], reg_names[1],
			      reg_brackets[2] ? "[" : "",
			      reg_names[2], reg_brackets[2] ? "]" : "");
	    }

	  decode_template_definition (insn_word, buf + len,
				      OPERAND_WIDTH - len);
	}
      else			/* Not a template definiton.  */
	{
	  reg_nums[0] = ((insn_word >> 19) & REG_MASK);
	  reg_nums[1] = ((insn_word >> 14) & REG_MASK);
	  reg_nums[2] = ((insn_word >> 9) & REG_MASK);

	  imm = (((insn_word >> 24) & 0x2) && (template->arg_type & DSP_ARGS_IMM));

	  if (imm)
	    is_dual = (insn_word & 0x4);
	  else if (!conditional)
	    is_dual = (insn_word & 0x10);
	  else
	    cc_flags = lookup_scc_flags ((insn_word >> 1) & CC_MASK);

	  /* De.r,Dx.r,De.r|ACe.r */
	  if (template->arg_type & DSP_ARGS_ACC2)
	    {
	      is_mod = (insn_word & 0x8);
	      is_overflow = (insn_word & 0x20);
	    }

	  if (template->arg_type & DSP_ARGS_SPLIT8)
	    {
	      is_overflow = (insn_word & 0x20);
	    }

	  /* ACe.e,ACx.r,ACo.e? */
	  if ((template->arg_type & DSP_ARGS_XACC) &&
	      (((insn_word >> 6) & 0x5) == 0x5))
	    {
	      enum metag_unit ac_unit, ao_unit;

	      ac_unit = (data_unit == UNIT_D0) ? UNIT_ACC_D0 : UNIT_ACC_D1;

	      if (ac_unit == UNIT_ACC_D0)
		ao_unit = UNIT_ACC_D1;
	      else
		ao_unit = UNIT_ACC_D0;

	      reg_nums[1] = ((insn_word >> 19) & REG_MASK);
	      reg_names[0] = lookup_dsp_name (reg_nums[1], ac_unit);
	      reg_names[1] = lookup_dsp_name (reg_nums[1], ac_unit);
	      reg_names[2] = lookup_dsp_name (reg_nums[1], ao_unit);
	    }
	  else
	    {
	      bool o2r = (insn_word & 0x1);

	      /* De.r|ACe.r,Dx.r,De.r */
	      if ((template->arg_type & DSP_ARGS_DACC) &&
		  ((insn_word & 0x84) != 0))
		{
		  enum metag_unit ac_unit;

		  ac_unit = (data_unit == UNIT_D0) ? UNIT_ACC_D0 : UNIT_ACC_D1;
		  reg_names[0] = lookup_dsp_name (reg_nums[0], ac_unit);

		  is_acc_zero = ((insn_word & 0x84) == 0x04);
		  is_acc_add = ((insn_word & 0x84) == 0x80);
		  is_acc_sub = ((insn_word & 0x84) == 0x84);
		}
	      else if (conditional)
		{
		  reg_names[0] = lookup_reg_name (data_unit, reg_nums[0]);
		}
	      else
		{
		  reg_names[0] = lookup_any_reg_name (data_unit,
						      reg_nums[0], false);
		  if (reg_nums[0] > 15)
		    reg_brackets[0] = 1;
		}

	      if (imm)
		{
		  reg_names[1] = lookup_any_reg_name (data_unit, reg_nums[0], true);

		  if (reg_brackets[0])
		    reg_brackets[1] = 1;
		  }
	      else
		{
		  if (is_split8 && is_mod)
		    {
		      reg_names[1] = lookup_reg_name (data_unit, reg_nums[1]);
		    }
		  else
		  {
		    reg_names[1] = lookup_any_reg_name (data_unit, reg_nums[1], true);

		    if (reg_nums[1] > 15)
		      reg_brackets[1] = 1;
		  }
		}

	      /* Detection of QUICKRoT and accumulator usage uses the
		 same bits. They are mutually exclusive.  */
	      if (ac && (template->arg_type & DSP_ARGS_ACC2))
		{
		  if (data_unit == UNIT_D0)
		    reg_names[2] = lookup_dsp_name (reg_nums[2], UNIT_ACC_D0);
		  else
		    reg_names[2] = lookup_dsp_name (reg_nums[2], UNIT_ACC_D1);
		}

	      else
		{
		  if ((template->arg_type & DSP_ARGS_QR) &&
		      ((insn_word & 0x40) == 0x40))
		    {
		      enum metag_unit aunit;
		      int reg_no;

		      if (conditional)
			reg_no = ((insn_word >> 5) & 0x1);
		      else
			reg_no = ((insn_word >> 7) & 0x1);

		      aunit = (data_unit == UNIT_D0) ? UNIT_A0 : UNIT_A1;
		      addr_reg = lookup_reg_name (aunit, reg_no + 2);

		      qr = true;
		    }

		  if (o2r)
		    reg_names[2] = lookup_o2r (data_unit, reg_nums[2]);
		  else
		    {
		      /* Can't use a DSPRAM reg if both QD and L1 are
			 set on a QUICKRoT instruction or if we're a
			 split 8.  */
		      if (((template->arg_type & DSP_ARGS_QR)
			   && ((insn_word & 0x30) == 0x30 && !conditional)) ||
			  (is_split8 && is_mod))
			reg_names[2] = lookup_reg_name (data_unit, reg_nums[2]);
		      else
			{
			  reg_names[2] = lookup_any_reg_name (data_unit,
							      reg_nums[2], true);
			  if (reg_nums[2] > 15)
			    reg_brackets[2] = 1;
			}
		    }
		}
	    }

	  if (qr)
	    {
	      len = snprintf (buf, OPERAND_WIDTH, "%s%s%s,%s%s%s,%s%s%s,%s",
			      reg_brackets[0] ? "[" : "",
			      reg_names[0], reg_brackets[0] ? "]" : "",
			      reg_brackets[1] ? "[" : "",
			      reg_names[1], reg_brackets[1] ? "]" : "",
			      reg_brackets[2] ? "[" : "",
			      reg_names[2], reg_brackets[2] ? "]" : "",
			      addr_reg);
	    }
	  else
	    {
	      if (imm)
		{
		  /* Conform to the embedded assembler's policy of
		     printing negative numbers as decimal and positive
		     as hex.  */
		  int value = ((insn_word >> 3) & IMM16_MASK);

		  if ((value & 0x8000) || value == 0)
		    {
		      value = sign_extend (value, IMM16_BITS);
		      len = snprintf (buf, OPERAND_WIDTH, "%s%s%s,%s%s%s,#%d",
				      reg_brackets[0] ? "[" : "",
				      reg_names[0], reg_brackets[0] ? "]" : "",
				      reg_brackets[1] ? "[" : "",
				      reg_names[1], reg_brackets[1] ? "]" : "",
				      value);
		    }
		  else
		    {
		      len = snprintf (buf, OPERAND_WIDTH, "%s%s%s,%s%s%s,#%#x",
				      reg_brackets[0] ? "[" : "",
				      reg_names[0], reg_brackets[0] ? "]" : "",
				      reg_brackets[1] ? "[" : "",
				      reg_names[1], reg_brackets[1] ? "]" : "",
				      value);
		    }
		}
	      else
		{
		  len = snprintf (buf, OPERAND_WIDTH, "%s%s%s,%s%s%s,%s%s%s",
				  reg_brackets[0] ? "[" : "",
				  reg_names[0], reg_brackets[0] ? "]" : "",
				  reg_brackets[1] ? "[" : "", reg_names[1],
				  reg_brackets[1] ? "]" : "",
				  reg_brackets[2] ? "[" : "",
				  reg_names[2], reg_brackets[2] ? "]" : "");
		}
	    }
	}

      snprintf (prefix, DSP_PREFIX_WIDTH, "D%s%s%s%s%s%s%s%s%s%s%s%s",
		cc_flags ? cc_flags : "",
		is_dual ? "L" : "",
		is_quickrot64 ? "Q" : "",
		is_unsigned ? "U" : "",
		is_mod ? "M" : "",
		is_acc_zero ? "Z" : "",
		is_acc_add ? "P" : "", is_acc_sub ? "N" : "",
		is_overflow ? "O" : "",
		is_w_mx ? "W" : "",
		is_b_mx ? "B" : "",
		is_template ? "T" : "");
    }
  else if (template->arg_type & DSP_ARGS_2) /* Group 2.  */
    {
      bool is_template;
      bool o2r = false;
      int major = MAJOR_OPCODE (template->meta_opcode);
      bool is_neg_or_mov = (major == OPC_ADD || major == OPC_SUB);
      bool is_cmp_tst = major == OPC_CMP && (insn_word & 0x0000002c) == 0;
      bool is_fpu_mov = template->insn_type == INSN_DSP_FPU;
      bool to_fpu = (template->meta_opcode >> 7) & 0x1;

      if (major == OPC_9)
	imm = (insn_word & 0x2);
      else if (template->arg_type & DSP_ARGS_IMM)
	imm = ((insn_word >> 25) & 0x1);

      is_template = (((insn_word & 0x02000002) == 0x2) &&
		     major != OPC_9);

      if (imm)
	is_dual = ((insn_word >> 0x2) & 0x1);
      else
	is_dual = ((insn_word >> 0x4) & 0x1);

      /* MOV and XSD[BW] do not have o2r.  */
      if (major != OPC_9 && major != OPC_MISC)
	o2r = (insn_word & 0x1);

      if (is_neg_or_mov)
	{
	  is_mod = (insn_word & 0x8);
	  is_overflow = (insn_word & 0x20);
	}

      /* XSD */
      if (major == OPC_MISC)
	data_unit = (insn_word & 0x1) ? UNIT_D1 : UNIT_D0;
      else
	data_unit = ((insn_word >> 24) & 0x1) ? UNIT_D1 : UNIT_D0;

      /* Check for NEG,MOV,ABS,FFB, etc.  */
      if (is_neg_or_mov || !is_cmp_tst || imm ||
	  MAJOR_OPCODE (insn_word) == OPC_9 ||
	  MAJOR_OPCODE (insn_word) == OPC_MISC)
	reg_nums[0] = ((insn_word >> 19) & REG_MASK);
      else
	reg_nums[0] = ((insn_word >> 14) & REG_MASK);

      if (is_template)
	{
	  is_w_mx = (insn_word & 0x1);

	  /* These are dummy arguments anyway so the register number
	     does not matter.  */
	  if (is_fpu_mov)
	    {
	      if (to_fpu)
		{
		  reg_names[0] = lookup_reg_name (UNIT_FX, 0);
		  reg_names[1] = lookup_reg_name (data_unit, 0);
		}
	      else
		{
		  reg_names[0] = lookup_reg_name (data_unit, 0);
		  reg_names[1] = lookup_reg_name (UNIT_FX, 0);
		}
	    }
	  else
	    {
	      reg_names[0] = lookup_reg_name (data_unit, 0);
	      reg_names[1] = lookup_reg_name (data_unit, 0);
	    }

	  len = snprintf (buf, OPERAND_WIDTH, "%s,%s",
			  reg_names[0], reg_names[1]);

	  decode_template_definition (insn_word, buf + len,
				      OPERAND_WIDTH - len);
	}
      else
	{
	  if (imm)
	    {
	      /* Conform to the embedded assembler's policy of
		 printing negative numbers as decimal and positive as
		 hex.  */
	      unsigned int value = ((insn_word >> 3) & IMM16_MASK);

	      if (major == OPC_9)
		{
		  data_unit = (insn_word & 0x1) ? UNIT_D1 : UNIT_D0;
		  is_dual = (insn_word & 0x4);

		  reg_names[0] = __lookup_dsp_name (reg_nums[0], data_unit);
		}
	      else
		{
		  reg_names[0] = lookup_any_reg_name (data_unit, reg_nums[0], true);
		  if (reg_nums[0] > 15)
		    reg_brackets[0] = 1;
		}

	      if ((value & 0x8000) || value == 0)
		{
		  value = sign_extend (value, IMM16_BITS);
		  snprintf (buf, OPERAND_WIDTH, "%s%s%s,#%d",
			    reg_brackets[0] ? "[" : "",
			    reg_names[0], reg_brackets[0] ? "]" : "",
			    value);
		}
	      else
		{
		  snprintf (buf, OPERAND_WIDTH, "%s%s%s,#0x%x",
			    reg_brackets[0] ? "[" : "",
			    reg_names[0], reg_brackets[0] ? "]" : "",
			    value);
		}
	    }
	  else
	    {
	      if (is_neg_or_mov || is_cmp_tst)
		reg_nums[1] = ((insn_word >> 9) & REG_MASK);
	      else
		reg_nums[1] = ((insn_word >> 14) & REG_MASK);

	      if (major == OPC_9)
		{
		  is_dual = (insn_word & 0x4);
		  data_unit = (insn_word & 0x1) ? UNIT_D1 : UNIT_D0;

		  if (MINOR_OPCODE (template->meta_opcode) == 0x1)
		    reg_names[0] = __lookup_dsp_name (reg_nums[0], data_unit);
		  else
		    reg_names[0] = lookup_reg_name (data_unit, reg_nums[0]);
		}
	      else
		{
		  unsigned int reg0_unit = data_unit;

		  if (is_fpu_mov && to_fpu)
		    reg0_unit = UNIT_FX;

		  reg_names[0] = lookup_any_reg_name (reg0_unit, reg_nums[0],
						      (!is_neg_or_mov && is_cmp_tst));
		  if (reg_nums[0] > 15)
		    reg_brackets[0] = 1;
		}

	      if (o2r)
		reg_names[1] = lookup_o2r (data_unit, reg_nums[1]);
	      else
		{
		  /* Check for accumulator argument.  */
		  if (is_neg_or_mov && ((insn_word & 0x80) == 0x80))
		    {
		      if (data_unit == UNIT_D0)
			reg_names[1] = lookup_dsp_name (reg_nums[1], UNIT_ACC_D0);
		      else
			reg_names[1] = lookup_dsp_name (reg_nums[1], UNIT_ACC_D1);
		    }
		  else
		    {
		      if (major == OPC_9)
			{
			  if (MINOR_OPCODE (template->meta_opcode) == 0x1)
			    {
			      reg_names[1] = lookup_reg_name (data_unit, reg_nums[1]);
			    }
			  else
			    {
			      enum metag_unit u;

			      u = (insn_word & 0x1) ? UNIT_RAM_D1 : UNIT_RAM_D0;
			      reg_names[1] = lookup_dsp_name (reg_nums[1], u);
			    }
			}
		      else
			{
			  reg_names[1] = lookup_any_reg_name (data_unit,
							      reg_nums[1], true);
			  if (reg_nums[1] > 15)
			    reg_brackets[1] = 1;
			}
		    }
		}

	      snprintf (buf, OPERAND_WIDTH, "%s%s%s,%s%s%s",
			reg_brackets[0] ? "[" : "", reg_names[0],
			reg_brackets[0] ? "]" : "",
			reg_brackets[1] ? "[" : "", reg_names[1],
			reg_brackets[1] ? "]" : "");
	    }
	}

      snprintf (prefix, DSP_PREFIX_WIDTH, "D%s%s%s%s%s%s",
		is_fpu_mov ? "F" : "",
		is_dual ? "L" : "",
		is_mod ? "M" : "", is_overflow ? "O" : "",
		is_w_mx ? "W" : "",
		is_template ? "T" : "");
    }
  else				/* Group 3. */
    {
      /* If both the C and CA bits are set, then the Rd register can
	 be in any unit. Figure out which unit from the Ud field.  */
      bool all_units = (((insn_word) & 0x04000020) == 0x04000020);
      enum metag_unit ud_unit = ((insn_word >> 1) & UNIT_MASK);
      enum metag_unit ram_unit, acc_unit;
      bool round = false;
      bool clamp9 = false;
      bool clamp8 = false;
      bool is_template = ((insn_word & 0x04000002) == 0x2);

      imm = ((insn_word >> 25) & 0x1);
      ac = (insn_word & 0x1);

      conditional = (MINOR_OPCODE (insn_word) & 0x4);

      /* Check for conditional and not Condition Always.  */
      if (conditional && !(insn_word & 0x20))
	cc_flags = lookup_scc_flags ((insn_word >> 1) & CC_MASK);
      else if (!(conditional && (insn_word & 0x20)))
	is_dual = ((insn_word >> 0x4) & 0x1);

      /* Conditional instructions don't have the L1 or RSPP fields.  */
      if ((insn_word & 0x04000000) == 0)
	{
	  round = (((insn_word >> 2) & 0x3) == 0x1);
	  clamp9 = (((insn_word >> 2) & 0x3) == 0x2);
	  clamp8 = (((insn_word >> 2) & 0x3) == 0x3);
	}

      /* Read DU bit.  */
      data_unit = ((insn_word >> 24) & 0x1) ? UNIT_D1 : UNIT_D0;
      reg_nums[0] = ((insn_word >> 19) & REG_MASK);
      reg_nums[1] = ((insn_word >> 14) & REG_MASK);

      ram_unit = (data_unit == UNIT_D0) ? UNIT_RAM_D0 : UNIT_RAM_D1;
      acc_unit = (data_unit == UNIT_D0) ? UNIT_ACC_D0 : UNIT_ACC_D1;

      if (all_units)
	reg_names[0] = lookup_reg_name (ud_unit, reg_nums[0]);
      else
	{
	  if (conditional)
	    reg_names[0] = lookup_reg_name (data_unit, reg_nums[0]);
	  else
	    {
	      reg_names[0] = lookup_any_reg_name (data_unit, reg_nums[0], false);
	      if (reg_nums[0] > 15)
		reg_brackets[0] = 1;
	    }
	}

      if (ac)
	{
	  reg_names[1] = lookup_dsp_name (reg_nums[1], acc_unit);
	}
      else
	{
	  reg_names[1] = lookup_any_reg_name (data_unit, reg_nums[1], true);
	  if (reg_nums[1] > 15)
	    reg_brackets[1] = 1;
	}

      if (imm)
	{
	  snprintf (buf, OPERAND_WIDTH, "%s%s%s,%s%s%s,#%#x",
		    reg_brackets[0] ? "[" : "",
		    reg_names[0], reg_brackets[0] ? "]" : "",
		    reg_brackets[1] ? "[" : "",
		    reg_names[1], reg_brackets[1] ? "]" : "",
		    ((insn_word >> 9) & IMM5_MASK));
	}
      else
	{
	  reg_nums[2] = ((insn_word >> 9) & REG_MASK);

	  reg_names[2] = lookup_any_reg_name (data_unit, reg_nums[2], true);

	  if (reg_nums[2] > 15)
		  reg_brackets[2] = 1;

	  if (is_template)
	    {
	      bool load = ((insn_word >> 13) & 0x1);
	      bool dspram = (((insn_word >> 17) & 0x3) == 0x3);
	      const char *tname[1];
	      unsigned int tidx = ((insn_word >> 9) & TEMPLATE_REGS_MASK);
	      enum metag_unit au;
	      unsigned int addr_reg_nums[2];
	      const char *addr_reg_names[2];
	      const char *post_op = "";
	      const char *join_op = "";

	      is_w_mx = ((insn_word >> 5) & 0x1);

	      tname[0] = lookup_dsp_name (tidx, UNIT_DT);

	      /* These are dummy arguments anyway */
	      reg_names[0] = lookup_reg_name (data_unit, 0);
	      if (ac)
		reg_names[1] = lookup_dsp_name (16, acc_unit);
	      else
		reg_names[1] = lookup_reg_name (data_unit, 0);
	      reg_names[2] = lookup_reg_name (data_unit, 0);

	      addr_reg_names[1] = "";

	      if (dspram)
		{
		  ram_unit = (data_unit == UNIT_D0) ? UNIT_RAM_D0 : UNIT_RAM_D1;
		  addr_reg_nums[0] = ((insn_word >> 19) & REG_MASK);
		  addr_reg_names[0] = lookup_dspram_name (addr_reg_nums[0],
							  ram_unit, load);
		}
	      else
		{
		  bool im = (((insn_word >> 18) & 0x1) != 0);

		  au = (((insn_word >> 23) & 0x1) == 0) ? UNIT_A0 : UNIT_A1;
		  addr_reg_nums[0] = ((insn_word >> 19) & DSP_REG_MASK);

		  addr_reg_names[0] = lookup_reg_name (au, addr_reg_nums[0]);

		  if (im)
		    {
		      unsigned int im_value = ((insn_word >> 14) & 0x3);

		      switch (im_value)
			{
			case 0x1:
			  post_op = "++";
			  break;
			case 0x3:
			  post_op = "--";
			  break;
			}
		    }
		  else
		    {
		      addr_reg_nums[1] = ((insn_word >> 14) & DSP_REG_MASK);
		      addr_reg_names[1] = lookup_reg_name (au, addr_reg_nums[1]);
		      join_op = "+";
		      post_op = "++";
		    }
		}

	      if (load)
		{
		  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s %s,[%s%s%s%s]",
			    reg_names[0], reg_names[1], reg_names[2],
			    tname[0], addr_reg_names[0], join_op,
			    addr_reg_names[1], post_op);
		}
	      else
		{
		  snprintf (buf, OPERAND_WIDTH, "%s,%s,%s [%s%s%s%s],%s",
			    reg_names[0], reg_names[1], reg_names[2],
			    addr_reg_names[0], join_op, addr_reg_names[1],
			    post_op, tname[0]);
		}
	    }
	  else
	    {
	      snprintf (buf, OPERAND_WIDTH, "%s%s%s,%s%s%s,%s%s%s",
			reg_brackets[0] ? "[" : "",
			reg_names[0], reg_brackets[0] ? "]" : "",
			reg_brackets[1] ? "[" : "",
			reg_names[1], reg_brackets[1] ? "]" : "",
			reg_brackets[2] ? "[" : "",
			reg_names[2], reg_brackets[2] ? "]" : "");
	    }
	}

      snprintf (prefix, DSP_PREFIX_WIDTH, "D%s%s%s%s%s%s%s",
		cc_flags ? cc_flags : "",
		is_dual ? "L" : "", clamp9 ? "G" : "",
		clamp8 ? "B" : "", round ? "R" : "",
		is_w_mx ? "W" : "",
		is_template ? "T" : "");
    }

  print_insn (outf, prefix, template->name, buf);

}

typedef void (*insn_printer)(unsigned int, bfd_vma, const insn_template *,
			     disassemble_info *);

/* Printer table.  */
static const insn_printer insn_printers[ENC_MAX] =
  {
    [ENC_NONE] = print_none,
    [ENC_MOV_U2U] = print_mov_u2u,
    [ENC_MOV_PORT] = print_mov_port,
    [ENC_MMOV] = print_mmov,
    [ENC_MDRD] = print_mdrd,
    [ENC_MOVL_TTREC] = print_movl_ttrec,
    [ENC_GET_SET] = print_get_set,
    [ENC_GET_SET_EXT] = print_get_set_ext,
    [ENC_MGET_MSET] = print_mget_mset,
    [ENC_COND_SET] = print_cond_set,
    [ENC_XFR] = print_xfr,
    [ENC_MOV_CT] = print_mov_ct,
    [ENC_SWAP] = print_swap,
    [ENC_JUMP] = print_jump,
    [ENC_CALLR] = print_callr,
    [ENC_ALU] = print_alu,
    [ENC_SHIFT] = print_shift,
    [ENC_MIN_MAX] = print_min_max,
    [ENC_BITOP] = print_bitop,
    [ENC_CMP] = print_cmp,
    [ENC_BRANCH] = print_branch,
    [ENC_KICK] = print_mov_u2u,
    [ENC_SWITCH] = print_switch,
    [ENC_CACHER] = print_cacher,
    [ENC_CACHEW] = print_cachew,
    [ENC_ICACHE] = print_icache,
    [ENC_LNKGET] = print_lnkget,
    [ENC_FMOV] = print_fmov,
    [ENC_FMMOV] = print_fmmov,
    [ENC_FMOV_DATA] = print_fmov_data,
    [ENC_FMOV_I] = print_fmov_i,
    [ENC_FPACK] = print_fpack,
    [ENC_FSWAP] = print_fswap,
    [ENC_FCMP] = print_fcmp,
    [ENC_FMINMAX] = print_fminmax,
    [ENC_FCONV] = print_fconv,
    [ENC_FCONVX] = print_fconvx,
    [ENC_FBARITH] = print_fbarith,
    [ENC_FEARITH] = print_fearith,
    [ENC_FREC] = print_frec,
    [ENC_FSIMD] = print_fsimd,
    [ENC_FGET_SET_ACF] = print_fget_set_acf,
    [ENC_DGET_SET] = print_dget_set,
    [ENC_DTEMPLATE] = print_dtemplate,
    [ENC_DALU] = print_dalu,
  };

/* Entry point for instruction printing.  */
int
print_insn_metag (bfd_vma pc, disassemble_info *outf)
{
  bfd_byte buf[4];
  unsigned int insn_word;
  size_t i;
  int status;

  outf->bytes_per_chunk = 4;
  status = (*outf->read_memory_func) (pc & ~0x03, buf, 4, outf);
  if (status)
    {
      (*outf->memory_error_func) (status, pc, outf);
      return -1;
    }
  insn_word = bfd_getl32 (buf);

  for (i = 0; i < sizeof(metag_optab)/sizeof(metag_optab[0]); i++)
    {
      const insn_template *template = &metag_optab[i];

      if ((insn_word & template->meta_mask) == template->meta_opcode)
	{
	  enum insn_encoding encoding = template->encoding;
	  insn_printer printer = insn_printers[encoding];

	  if (printer)
	    printer (insn_word, pc, template, outf);

	  return 4;
	}
    }

  return 4;
}
