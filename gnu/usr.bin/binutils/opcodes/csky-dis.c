/* C-SKY disassembler.
   Copyright (C) 1988-2023 Free Software Foundation, Inc.
   Contributed by C-SKY Microsystems and Mentor Graphics.

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
#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <elf/csky.h>
#include "disassemble.h"
#include "elf-bfd.h"
#include "opcode/csky.h"
#include "libiberty.h"
#include "csky-opc.h"
#include "floatformat.h"

#define CSKY_INST_TYPE unsigned long
#define HAS_SUB_OPERAND (unsigned int)0xffffffff
#define CSKY_DEFAULT_ISA 0xffffffff

enum sym_type
{
  CUR_TEXT,
  CUR_DATA
};

struct csky_dis_info
{
  /* Mem to disassemble.  */
  bfd_vma mem;
  /* Disassemble info.  */
  disassemble_info *info;
  /* Opcode information.  */
  struct csky_opcode_info const *opinfo;
  uint64_t isa;
  /* The value of operand to show.  */
  int value;
  /* Whether to look up/print a symbol name.  */
  int need_output_symbol;
} dis_info;


enum sym_type last_type;
int last_map_sym = 1;
bfd_vma last_map_addr = 0;
int using_abi = 0;

/* Only for objdump tool.  */
#define INIT_MACH_FLAG  0xffffffff
#define BINARY_MACH_FLAG 0x0

static unsigned int mach_flag = INIT_MACH_FLAG;

static void
print_insn_data (bfd_vma pc ATTRIBUTE_UNUSED,
		 struct disassemble_info *info,
		 long given)
{
  switch (info->bytes_per_chunk)
    {
    case 1:
      info->fprintf_func (info->stream, ".byte\t0x%02lx", given);
      break;
    case 2:
      info->fprintf_func (info->stream, ".short\t0x%04lx", given);
      break;
    case 4:
      info->fprintf_func (info->stream, ".long\t0x%08lx", given);
      break;
    default:
      abort ();
    }
}

static int
get_sym_code_type (struct disassemble_info *info,
		   int n,
		   enum sym_type *sym_type)
{
  const char *name;
  name = bfd_asymbol_name (info->symtab[n]);
  if (name[0] == '$' && (name[1] == 't' || name[1] == 'd')
      && (name[2] == 0 || name[2] == '.'))
    {
      *sym_type = ((name[1] == 't') ? CUR_TEXT : CUR_DATA);
      return true;
    }
  return false;
}

static int
csky_get_operand_mask (struct operand const *oprnd)
{
  int mask = 0;
  if (oprnd->mask == HAS_SUB_OPERAND)
    {
      struct soperand *sop = (struct soperand *)oprnd;
      mask |= csky_get_operand_mask (&sop->subs[0]);
      mask |= csky_get_operand_mask (&sop->subs[1]);
      return mask;
    }
  return oprnd->mask;
}

static int
csky_get_mask (struct csky_opcode_info const *pinfo)
{
  int i = 0;
  int mask = 0;
  /* List type.  */
  if (pinfo->operand_num == -1)
    mask |= csky_get_operand_mask (&pinfo->oprnd.oprnds[i]);
  else
    for (; i < pinfo->operand_num; i++)
      mask |= csky_get_operand_mask (&pinfo->oprnd.oprnds[i]);

  mask = ~mask;
  return mask;
}

static unsigned int
csky_chars_to_number (unsigned char * buf, int n)
{
  int i;
  unsigned int val = 0;

  if (dis_info.info->endian == BFD_ENDIAN_BIG)
    for (i = 0; i < n; i++)
      val = val << 8 | buf[i];
  else
    for (i = n - 1; i >= 0; i--)
      val = val << 8 | buf[i];
  return val;
}

static struct csky_opcode const *g_opcodeP;

static struct csky_opcode const *
csky_find_inst_info (struct csky_opcode_info const **pinfo,
		     CSKY_INST_TYPE inst, int length)
{
  int i;
  unsigned int mask;
  struct csky_opcode const *p;

  p = g_opcodeP;
  while (p->mnemonic)
    {
	if (!(p->isa_flag16 & dis_info.isa)
	      && !(p->isa_flag32 & dis_info.isa))
	{
	  p++;
	  continue;
	}

      /* Get the opcode mask.  */
      for (i = 0; i < OP_TABLE_NUM; i++)
	if (length == 2)
	  {
	    mask =  csky_get_mask (&p->op16[i]);
	    if (mask != 0 && (inst & mask) == p->op16[i].opcode)
	      {
		*pinfo = &p->op16[i];
		g_opcodeP = p;
		return p;
	      }
	  }
	else if (length == 4)
	  {
	    mask =  csky_get_mask (&p->op32[i]);
	    if (mask != 0
		&& ((unsigned long)(inst & mask)
		    == (unsigned long)p->op32[i].opcode))
	      {
		*pinfo = &p->op32[i];
		g_opcodeP = p;
		return p;
	      }
	  }
      p++;
    }

  return NULL;
}

static bool
is_extern_symbol (struct disassemble_info *info, int addr)
{
  unsigned int rel_count = 0;

  if (info->section == NULL)
    return 0;
  if ((info->section->flags & SEC_RELOC) != 0)	/* Fit .o file.  */
    {
      struct reloc_cache_entry *pt = info->section->relocation;
      for (; rel_count < info->section->reloc_count; rel_count++, pt++)
	if ((long unsigned int)addr == pt->address)
	  return true;
      return false;
    }
  return false;
}


/* Suppress printing of mapping symbols emitted by the assembler to mark
   the beginning of code and data sequences.  */

bool
csky_symbol_is_valid (asymbol *sym,
		      struct disassemble_info *info ATTRIBUTE_UNUSED)
{
  const char *name;

  if (sym == NULL)
    return false;
  name = bfd_asymbol_name (sym);
  return name && *name != '$';
}

disassembler_ftype
csky_get_disassembler (bfd *abfd)
{
  obj_attribute *attr;
  const char *sec_name = NULL;
  if (!abfd || bfd_get_flavour (abfd) != bfd_target_elf_flavour)
    dis_info.isa = CSKY_DEFAULT_ISA;
  else
    {
      mach_flag = elf_elfheader (abfd)->e_flags;

      sec_name = get_elf_backend_data (abfd)->obj_attrs_section;
      /* Skip any input that hasn't attribute section.
         This enables to link object files without attribute section with
         any others.  */
      if (bfd_get_section_by_name (abfd, sec_name) != NULL)
        {
          attr = elf_known_obj_attributes_proc (abfd);
          dis_info.isa = attr[Tag_CSKY_ISA_EXT_FLAGS].i;
          dis_info.isa <<= 32;
          dis_info.isa |= attr[Tag_CSKY_ISA_FLAGS].i;
        }
      else
        dis_info.isa = CSKY_DEFAULT_ISA;
    }

   return print_insn_csky;
}

/* Parse the string of disassembler options.  */
static void
parse_csky_dis_options (const char *opts_in)
{
  char *opts = xstrdup (opts_in);
  char *opt = opts;
  char *opt_end = opts;

  for (; opt_end != NULL; opt = opt_end + 1)
    {
      if ((opt_end = strchr (opt, ',')) != NULL)
	*opt_end = 0;
      if (strcmp (opt, "abi-names") == 0)
	using_abi = 1;
      else
	fprintf (stderr,
		 "unrecognized disassembler option: %s", opt);
    }
}

/* Get general register name.  */
static const char *
get_gr_name (int regno)
{
  return csky_get_general_reg_name (mach_flag, regno, using_abi);
}

/* Get control register name.  */
static const char *
get_cr_name (unsigned int regno, int bank)
{
  return csky_get_control_reg_name (mach_flag, bank, regno, using_abi);
}

static int
csky_output_operand (char *str, struct operand const *oprnd,
		     CSKY_INST_TYPE inst, int reloc ATTRIBUTE_UNUSED)
{
  int ret = 0;;
  int bit = 0;
  int result = 0;
  bfd_vma value;
  int mask = oprnd->mask;
  int max = 0;
  char buf[128];

  /* Get operand value with mask.  */
  value = inst & mask;
  for (; mask; mask >>= 1, value >>=1)
    if (mask & 0x1)
      {
	result |= ((value & 0x1) << bit);
	max |= (1 << bit);
	bit++;
      }
  value = result;

  /* Here is general instructions that have no reloc.  */
  switch (oprnd->type)
    {
    case OPRND_TYPE_CTRLREG:
	if (IS_CSKY_V1(mach_flag) && ((value & 0x1f) == 0x1f))
	  return -1;
	strcat (str, get_cr_name((value & 0x1f), (value >> 5)));
	break;
    case OPRND_TYPE_DUMMY_REG:
      mask = dis_info.opinfo->oprnd.oprnds[0].mask;
      value = inst & mask;
      for (; mask; mask >>= 1, value >>=1)
	if (mask & 0x1)
	  {
	    result |= ((value & 0x1) << bit);
	    bit++;
	  }
      value = result;
      strcat (str, get_gr_name (value));
      break;
    case OPRND_TYPE_GREG0_7:
    case OPRND_TYPE_GREG0_15:
    case OPRND_TYPE_GREG16_31:
    case OPRND_TYPE_REGnsplr:
    case OPRND_TYPE_AREG:
      strcat (str, get_gr_name (value));
      break;
    case OPRND_TYPE_CPREG:
      sprintf (buf, "cpr%d", (int)value);
      strcat (str, buf);
      break;
    case OPRND_TYPE_FREG:
      sprintf (buf, "fr%d", (int)value);
      strcat (str, buf);
      break;
    case OPRND_TYPE_VREG:
      dis_info.value = value;
      sprintf (buf, "vr%d", (int)value);
      strcat (str, buf);
      break;
    case OPRND_TYPE_CPCREG:
      sprintf (buf, "cpcr%d", (int)value);
      strcat (str, buf);
      break;
    case OPRND_TYPE_CPIDX:
      sprintf (buf, "cp%d", (int)value);
      strcat (str, buf);
      break;
    case OPRND_TYPE_IMM2b_JMPIX:
      value = (value + 2) << 3;
      sprintf (buf, "%d", (int)value);
      strcat (str, buf);
      break;
    case OPRND_TYPE_IMM_LDST:
    case OPRND_TYPE_IMM_FLDST:
      value <<= oprnd->shift;
      sprintf (buf, "0x%x", (unsigned int)value);
      strcat (str, buf);
      break;
    case OPRND_TYPE_IMM7b_LS2:
    case OPRND_TYPE_IMM8b_LS2:
      sprintf (buf, "%d", (int)(value << 2));
      strcat (str, buf);
      ret = 0;
      break;
    case OPRND_TYPE_IMM5b_BMASKI:
      if ((value != 0) && (value > 31 || value < 8))
	{
	  ret = -1;
	  break;
	}
      sprintf (buf, "%d", (int)value);
      strcat (str, buf);
      ret = 0;
      break;
    case OPRND_TYPE_IMM5b_1_31:
      if (value > 31 || value < 1)
	{
	  ret = -1;
	  break;
	}
      sprintf (buf, "%d", (int)value);
      strcat (str, buf);
      ret = 0;
      break;
    case OPRND_TYPE_IMM5b_7_31:
      if (value > 31 || value < 7)
	{
	  ret = -1;
	  break;
	}
      sprintf (buf, "%d", (int)value);
      strcat (str, buf);
      ret = 0;
      break;
    case OPRND_TYPE_IMM5b_VSH:
      {
	char num[128];
	value = ((value & 0x1) << 4) | (value >> 1);
	sprintf (num, "%d", (int)value);
	strcat (str, num);
	ret = 0;
	break;
      }
    case OPRND_TYPE_MSB2SIZE:
    case OPRND_TYPE_LSB2SIZE:
      {
	static int size;
	if (oprnd->type == OPRND_TYPE_MSB2SIZE)
	  size = value;
	else
	  {
	    str[strlen (str) - 2] = '\0';
	    sprintf (buf, "%d, %d", (int)(size + value), (int)value);
	    strcat (str, buf);
	  }
	break;
      }
    case OPRND_TYPE_IMM1b:
    case OPRND_TYPE_IMM2b:
    case OPRND_TYPE_IMM4b:
    case OPRND_TYPE_IMM5b:
    case OPRND_TYPE_IMM5b_LS:
    case OPRND_TYPE_IMM7b:
    case OPRND_TYPE_IMM8b:
    case OPRND_TYPE_IMM12b:
    case OPRND_TYPE_IMM15b:
    case OPRND_TYPE_IMM16b:
    case OPRND_TYPE_IMM16b_MOVIH:
    case OPRND_TYPE_IMM16b_ORI:
      sprintf (buf, "%d", (int)value);
      strcat (str, buf);
      ret = 0;
      break;
    case OPRND_TYPE_OFF8b:
    case OPRND_TYPE_OFF16b:
      {
	unsigned char ibytes[4];
	int shift = oprnd->shift;
	int status;
	unsigned int mem_val;

	dis_info.info->stop_vma = 0;

	value = ((dis_info.mem + (value << shift)
		  + ((IS_CSKY_V1 (mach_flag)) ? 2 : 0))
		 & 0xfffffffc);
	status = dis_info.info->read_memory_func (value, ibytes, 4,
						  dis_info.info);
	if (status != 0)
	  {
	    dis_info.info->memory_error_func (status, dis_info.mem,
					      dis_info.info);
	    return -1;
	  }
	mem_val = csky_chars_to_number (ibytes, 4);
	/* Remove [] around literal value to match ABI syntax.  */
	sprintf (buf, "0x%X", mem_val);
	strcat (str, buf);
	/* For jmpi/jsri, we'll try to get a symbol for the target.  */
	if (dis_info.info->print_address_func && mem_val != 0)
	  {
	    dis_info.value = mem_val;
	    dis_info.need_output_symbol = 1;
	  }
	else
	  {
	    sprintf (buf, "\t// from address pool at 0x%x",
		     (unsigned int)value);
	    strcat (str, buf);
	  }
	break;
      }
    case OPRND_TYPE_BLOOP_OFF4b:
    case OPRND_TYPE_BLOOP_OFF12b:
    case OPRND_TYPE_OFF11b:
    case OPRND_TYPE_OFF16b_LSL1:
    case OPRND_TYPE_IMM_OFF18b:
    case OPRND_TYPE_OFF26b:
      {
	int shift = oprnd->shift;
	if (value & ((max >> 1) + 1))
	  value |= ~max;
	if (is_extern_symbol (dis_info.info, dis_info.mem))
	  value = 0;
	else if (IS_CSKY_V1 (mach_flag))
	  value = dis_info.mem + 2 + (value << shift);
	else
	  value = dis_info.mem + (value << shift);
	dis_info.need_output_symbol = 1;
	dis_info.value= value;
	sprintf (buf, "0x%x", (unsigned int)value);
	strcat (str, buf);
	break;
      }
    case OPRND_TYPE_CONSTANT:
    case OPRND_TYPE_FCONSTANT:
      {
	int shift = oprnd->shift;
	bfd_byte ibytes[8];
	int status;
	bfd_vma addr;
	int nbytes;

	dis_info.info->stop_vma = 0;
	value <<= shift;

	if (IS_CSKY_V1 (mach_flag))
	  addr = (dis_info.mem + 2 + value) & 0xfffffffc;
	else
	  addr = (dis_info.mem + value) & 0xfffffffc;

	if (oprnd->type == OPRND_TYPE_FCONSTANT
	    && dis_info.opinfo->opcode != CSKYV2_INST_FLRW)
	  nbytes = 8;
	else
	  nbytes = 4;

	status = dis_info.info->read_memory_func (addr, ibytes,
						  nbytes, dis_info.info);
	if (status != 0)
	  /* Address out of bounds.  -> lrw rx, [pc, 0ffset]. */
	  sprintf (buf, "[pc, %d]\t// from address pool at %x", (int)value,
		   (unsigned int)addr);
	else if (oprnd->type == OPRND_TYPE_FCONSTANT)
	  {
	    double f;

	    if (dis_info.opinfo->opcode == CSKYV2_INST_FLRW)
	      /* flrws.  */
	      floatformat_to_double ((dis_info.info->endian == BFD_ENDIAN_BIG
				      ? &floatformat_ieee_single_big
				      : &floatformat_ieee_single_little),
				     ibytes, &f);
	    else
	      floatformat_to_double ((dis_info.info->endian == BFD_ENDIAN_BIG
				      ? &floatformat_ieee_double_big
				      : &floatformat_ieee_double_little),
				     ibytes, &f);
	    sprintf (buf, "%.7g", f);
	  }
	else
	  {
	    dis_info.value = addr;
	    dis_info.need_output_symbol = 1;
	    value = csky_chars_to_number (ibytes, 4);
	    sprintf (buf, "0x%x", (unsigned int) value);
	  }

	strcat (str, buf);
	break;
      }
    case OPRND_TYPE_ELRW_CONSTANT:
      {
	int shift = oprnd->shift;
	char ibytes[4];
	int status;
	bfd_vma addr;
	dis_info.info->stop_vma = 0;

	value = 0x80 + ((~value) & 0x7f);

	value = value << shift;
	addr = (dis_info.mem + value) & 0xfffffffc;

	status = dis_info.info->read_memory_func (addr, (bfd_byte *)ibytes,
						  4, dis_info.info);
	if (status != 0)
	  /* Address out of bounds.  -> lrw rx, [pc, 0ffset]. */
	  sprintf (buf, "[pc, %d]\t// from address pool at %x", (int) value,
		   (unsigned int)addr);
	else
	  {
	    dis_info.value = addr;
	    value = csky_chars_to_number ((unsigned char *)ibytes, 4);
	    dis_info.need_output_symbol = 1;
	    sprintf (buf, "0x%x", (unsigned int)value);
	  }

	strcat (str, buf);
	break;
      }
    case OPRND_TYPE_SFLOAT:
    case OPRND_TYPE_DFLOAT:
      {
	/* This is for fmovis/fmovid, which have an internal 13-bit
	   encoding that they convert to single/double precision
	   (respectively).  We'll convert the 13-bit encoding to an IEEE
	   double and then to host double format to print it.
	   Sign bit: bit 20.
	   4-bit exponent: bits 19:16, biased by 11.
	   8-bit mantissa: split between 24:21 and 7:4.  */
	uint64_t imm4;
	uint64_t imm8;
	uint64_t dbnum;
	unsigned char valbytes[8];
	double fvalue;

	imm4 = ((inst >> 16) & 0xf);
	imm4 = (uint64_t)(1023 - (imm4 - 11)) << 52;

	imm8 = (uint64_t)((inst >> 4) & 0xf) << 44;
	imm8 |= (uint64_t)((inst >> 21) & 0xf) << 48;

	dbnum = (uint64_t)((inst >> 20) & 1) << 63;
	dbnum |= imm4 | imm8;

	/* Do this a byte at a time so we don't have to
	   worry about the host's endianness.  */
	valbytes[0] = dbnum & 0xff;
	valbytes[1] = (dbnum >> 8) & 0xff;
	valbytes[2] = (dbnum >> 16) & 0xff;
	valbytes[3] = (dbnum >> 24) & 0xff;
	valbytes[4] = (dbnum >> 32) & 0xff;
	valbytes[5] = (dbnum >> 40) & 0xff;
	valbytes[6] = (dbnum >> 48) & 0xff;
	valbytes[7] = (dbnum >> 56) & 0xff;

	floatformat_to_double (&floatformat_ieee_double_little, valbytes,
			       &fvalue);

	sprintf (buf, "%.7g", fvalue);
	strcat (str, buf);
	break;
      }
    case OPRND_TYPE_HFLOAT_FMOVI:
    case OPRND_TYPE_SFLOAT_FMOVI:
      {
	int imm4;
	int imm8;
	imm4 = ((inst >> 16) & 0xf);
	imm4 = (138 - imm4) << 23;

	imm8 = ((inst >> 8) & 0x3);
	imm8 |= (((inst >> 20) & 0x3f) << 2);
	imm8 <<= 15;

	value = ((inst >> 5) & 1) << 31;
	value |= imm4 | imm8;

	imm4 = 138 - (imm4 >> 23);
	imm8 >>= 15;
	if ((inst >> 5) & 1)
	  {
	    imm8 = 0 - imm8;
	  }

	float f = 0;
	memcpy (&f, &value, sizeof (float));
	sprintf (buf, "%.7g\t// imm9:%4d, imm4:%2d", f, imm8, imm4);
	strcat (str, buf);

	break;
      }

    case OPRND_TYPE_DFLOAT_FMOVI:
      {
	uint64_t imm4;
	uint64_t imm8;
	uint64_t dvalue;
	imm4 = ((inst >> 16) & 0xf);
	imm4 = (1034 - imm4) << 52;

	imm8 = ((inst >> 8) & 0x3);
	imm8 |= (((inst >> 20) & 0x3f) << 2);
	imm8 <<= 44;

	dvalue = (((uint64_t)inst >> 5) & 1) << 63;
	dvalue |= imm4 | imm8;

	imm4 = 1034 - (imm4 >> 52);
	imm8 >>= 44;
	if (inst >> 5)
	  {
	    imm8 = 0 - imm8;
	  }
	double d = 0;
	memcpy (&d, &dvalue, sizeof (double));
	sprintf (buf, "%.7g\t// imm9:%4ld, imm4:%2ld", d, (long) imm8, (long) imm4);
	strcat (str, buf);

	break;
      }
    case OPRND_TYPE_LABEL_WITH_BRACKET:
      sprintf (buf, "[0x%x]", (unsigned int)value);
      strcat (str, buf);
      strcat (str, "\t// the offset is based on .data");
      break;
    case OPRND_TYPE_OIMM3b:
    case OPRND_TYPE_OIMM4b:
    case OPRND_TYPE_OIMM5b:
    case OPRND_TYPE_OIMM5b_IDLY:
    case OPRND_TYPE_OIMM8b:
    case OPRND_TYPE_OIMM12b:
    case OPRND_TYPE_OIMM16b:
    case OPRND_TYPE_OIMM18b:
      value += 1;
      sprintf (buf, "%d", (int)value);
      strcat (str, buf);
      break;
    case OPRND_TYPE_OIMM5b_BMASKI:
      if (value > 32 || value < 16)
	{
	  ret = -1;
	  break;
	}
      sprintf (buf, "%d", (int)(value + 1));
      strcat (str, buf);
      ret = 0;
      break;
    case OPRND_TYPE_FREGLIST_DASH:
      if (IS_CSKY_V2 (mach_flag))
	{
	  int vrx = 0;
	  int vry = 0;
	  if (dis_info.isa & CSKY_ISA_FLOAT_7E60
	      && (strstr (str, "fstm") != NULL
		  || strstr (str, "fldm") != NULL))
	    {
	      vrx = value & 0x1f;
	      vry = vrx + (value >> 5);
	    }
	  else
	    {
	      vrx = value & 0xf;
	      vry = vrx + (value >> 4);
	    }
	  sprintf (buf, "fr%d-fr%d", vrx, vry);
	  strcat (str, buf);
	}
      break;
    case OPRND_TYPE_REGLIST_DASH:
      if (IS_CSKY_V1 (mach_flag))
	{
	  sprintf (buf, "%s-r15", get_gr_name (value));
	  strcat (str, buf);
	}
      else
	{
	  if ((value & 0x1f) + (value >> 5) > 31)
	    {
	      ret = -1;
	      break;
	    }
	  strcat (str, get_gr_name ((value >> 5)));
	  strcat (str, "-");
	  strcat (str, get_gr_name ((value & 0x1f) + (value >> 5)));
	}
      break;
    case OPRND_TYPE_PSR_BITS_LIST:
      {
	struct psrbit const *bits;
	int first_oprnd = true;
	int i = 0;
	if (IS_CSKY_V1 (mach_flag))
	  {
	    if (value == 0)
	      {
		strcat (str, "af");
		break;
	      }
	    bits = cskyv1_psr_bits;
	  }
	else
	  bits = cskyv2_psr_bits;
	while (value != 0 && bits[i].name != NULL)
	    {
	      if (value & bits[i].value)
		{
		  if (!first_oprnd)
		    strcat (str, ", ");
		  strcat (str, bits[i].name);
		  value &= ~bits[i].value;
		  first_oprnd = false;
		}
	      i++;
	    }
	break;
      }
    case OPRND_TYPE_REGbsp:
      if (IS_CSKY_V1 (mach_flag))
	sprintf(buf, "(%s)", get_gr_name (0));
      else
	sprintf(buf, "(%s)", get_gr_name (14));
      strcat (str, buf);
      break;
    case OPRND_TYPE_REGsp:
      if (IS_CSKY_V1 (mach_flag))
	strcat (str, get_gr_name (0));
      else
	strcat (str, get_gr_name (14));
      break;
    case OPRND_TYPE_REGnr4_r7:
    case OPRND_TYPE_AREG_WITH_BRACKET:
      strcat (str, "(");
      strcat (str, get_gr_name (value));
      strcat (str, ")");
      break;
    case OPRND_TYPE_AREG_WITH_LSHIFT:
      strcat (str, get_gr_name (value >> 5));
      strcat (str, " << ");
      if ((value & 0x1f) == 0x1)
	strcat (str, "0");
      else if ((value & 0x1f) == 0x2)
	strcat (str, "1");
      else if ((value & 0x1f) == 0x4)
	strcat (str, "2");
      else if ((value & 0x1f) == 0x8)
	strcat (str, "3");
      break;
    case OPRND_TYPE_AREG_WITH_LSHIFT_FPU:
      strcat (str, get_gr_name (value >> 2));
      strcat (str, " << ");
      if ((value & 0x3) == 0x0)
	strcat (str, "0");
      else if ((value & 0x3) == 0x1)
	strcat (str, "1");
      else if ((value & 0x3) == 0x2)
	strcat (str, "2");
      else if ((value & 0x3) == 0x3)
	strcat (str, "3");
      break;
    case OPRND_TYPE_VREG_WITH_INDEX:
      {
	unsigned freg_val = value & 0xf;
	unsigned index_val = (value >> 4) & 0xf;
	sprintf (buf, "vr%d[%d]", freg_val, index_val);
	strcat(str, buf);
	break;
      }
    case OPRND_TYPE_FREG_WITH_INDEX:
      {
	unsigned freg_val = value & 0xf;
	unsigned index_val = (value >> 4) & 0xf;
	sprintf (buf, "fr%d[%d]", freg_val, index_val);
	strcat(str, buf);
	break;
      }
    case OPRND_TYPE_REGr4_r7:
      if (IS_CSKY_V1 (mach_flag))
	{
	  sprintf (buf, "%s-%s", get_gr_name (4), get_gr_name (7));
	  strcat (str, buf);
	}
      break;
    case OPRND_TYPE_CONST1:
      strcat (str, "1");
      break;
    case OPRND_TYPE_REG_r1a:
    case OPRND_TYPE_REG_r1b:
      strcat (str, get_gr_name (1));
      break;
    case OPRND_TYPE_REG_r28:
      strcat (str, get_gr_name (28));
      break;
    case OPRND_TYPE_REGLIST_DASH_COMMA:
      /* 16-bit reglist.  */
      if (value & 0xf)
	{
	  strcat (str, get_gr_name (4));
	  if ((value & 0xf) > 1)
	    {
	      strcat (str, "-");
	      strcat (str, get_gr_name ((value & 0xf) + 3));
	    }
	  if (value & ~0xf)
	    strcat (str, ", ");
	}
      if (value & 0x10)
	{
	  /* r15.  */
	  strcat (str, get_gr_name (15));
	  if (value & ~0x1f)
	    strcat (str, ", ");
	}
      if (dis_info.opinfo->oprnd.oprnds[0].mask != OPRND_MASK_0_4)
	{
	  /* 32bits reglist.  */
	  value >>= 5;
	  if (value & 0x3)
	    {
	      strcat (str, get_gr_name (16));
	      if ((value & 0x7) > 1)
		{
		  strcat (str, "-");
		  strcat (str, get_gr_name ((value & 0x7) + 15));
		}
	      if (value & ~0x7)
		strcat (str, ", ");
	      }
	  if (value & 0x8)
	    /* r15.  */
	    strcat (str, get_gr_name (28));
	}
      break;
    case OPRND_TYPE_UNCOND10b:
    case OPRND_TYPE_UNCOND16b:
    case OPRND_TYPE_COND10b:
    case OPRND_TYPE_COND16b:
      {
	int shift = oprnd->shift;

	if (value & ((max >> 1) + 1))
	  value |= ~max;
	if (is_extern_symbol (dis_info.info, dis_info.mem))
	  value = 0;
	else
	  value = dis_info.mem + (value << shift);
	sprintf (buf, "0x%x", (unsigned int)value);
	strcat (str, buf);
	dis_info.need_output_symbol = 1;
	dis_info.value = value;
      }
      break;

    default:
      ret = -1;
      break;
    }
  return ret;
}

static int
csky_print_operand (char *str, struct operand const *oprnd,
		    CSKY_INST_TYPE inst, int reloc)
{
  int ret = -1;
  char *lc = "";
  char *rc = "";
  if (oprnd->mask == HAS_SUB_OPERAND)
    {
      struct soperand *sop = (struct soperand *)oprnd;
      if (oprnd->type == OPRND_TYPE_BRACKET)
	{
	  lc = "(";
	  rc = ")";
	}
      else if (oprnd->type == OPRND_TYPE_ABRACKET)
	{
	  lc = "<";
	  rc = ">";
	}
      strcat (str, lc);
      ret = csky_print_operand (str, &sop->subs[0], inst, reloc);
      if (ret)
	return ret;
      strcat (str, ", ");
      ret = csky_print_operand (str, &sop->subs[1], inst, reloc);
      strcat (str, rc);
      return ret;
    }
  return csky_output_operand (str, oprnd, inst, reloc);
}

static int
csky_print_operands (char *str, struct csky_opcode_info const *pinfo,
		     struct disassemble_info *info, CSKY_INST_TYPE inst,
		     int reloc)
{
  int i = 0;
  int ret = 0;
  if (pinfo->operand_num)
    strcat (str, "      \t");
  if (pinfo->operand_num == -1)
    {
      ret = csky_print_operand (str, &pinfo->oprnd.oprnds[i], inst, reloc);
      if (ret)
	return ret;
    }
  else
    for (; i < pinfo->operand_num; i++)
      {
	if (i != 0)
	  strcat (str, ", ");
	ret = csky_print_operand (str, &pinfo->oprnd.oprnds[i], inst, reloc);
	if (ret)
	  return ret;
      }
  info->fprintf_func (info->stream, "%s", str);
  if (dis_info.need_output_symbol)
    {
      info->fprintf_func (info->stream, "\t// ");
      info->print_address_func (dis_info.value, dis_info.info);
    }
  return 0;
}

static void
number_to_chars_littleendian (char *buf, CSKY_INST_TYPE val, int n)
{
  if (n <= 0)
    abort ();
  while (n--)
    {
      *buf++ = val & 0xff;
      val >>= 8;
    }
}

#define CSKY_READ_DATA()                                        \
{                                                               \
  status = info->read_memory_func (memaddr, buf, 2, info);      \
  if (status)                                                   \
    {                                                           \
      info->memory_error_func (status, memaddr, info);          \
      return -1;                                                \
    }                                                           \
  if (info->endian == BFD_ENDIAN_BIG)                           \
    inst |= (buf[0] << 8) | buf[1];                             \
  else if (info->endian == BFD_ENDIAN_LITTLE)                   \
    inst |= (buf[1] << 8) | buf[0];                             \
  else                                                          \
    abort();                                                    \
  info->bytes_per_chunk += 2;                                   \
  memaddr += 2;                                                 \
}

int
print_insn_csky (bfd_vma memaddr, struct disassemble_info *info)
{
  unsigned char buf[4];
  CSKY_INST_TYPE inst = 0;
  int status;
  char str[256];
  unsigned long given;
  int is_data = false;
  void (*printer) (bfd_vma, struct disassemble_info *, long);
  unsigned int  size = 4;

  memset (str, 0, sizeof (str));
  info->bytes_per_chunk = 0;
  info->bytes_per_chunk = 0;
  dis_info.mem = memaddr;
  dis_info.info = info;
  dis_info.need_output_symbol = 0;

  if (info->disassembler_options)
    {
      parse_csky_dis_options (info->disassembler_options);
      info->disassembler_options = NULL;
    }

  if (mach_flag != INIT_MACH_FLAG && mach_flag != BINARY_MACH_FLAG)
    info->mach = mach_flag;
  else if (mach_flag == INIT_MACH_FLAG)
    {
      mach_flag = info->mach;
      dis_info.isa = CSKY_DEFAULT_ISA;
    }

  if (mach_flag == BINARY_MACH_FLAG && info->endian == BFD_ENDIAN_UNKNOWN)
    {
      info->endian = BFD_ENDIAN_LITTLE;
      dis_info.isa = CSKY_DEFAULT_ISA;
    }

  /* First check the full symtab for a mapping symbol, even if there
     are no usable non-mapping symbols for this address.  */
  if (info->symtab_size != 0
      && bfd_asymbol_flavour (*info->symtab) == bfd_target_elf_flavour)
    {
      bfd_vma addr;
      int n;
      int last_sym = -1;
      enum sym_type type = CUR_TEXT;

      if (memaddr <= last_map_addr)
	last_map_sym = -1;
      /* Start scanning at the start of the function, or wherever
	 we finished last time.  */
      n = 0;
      if (n < last_map_sym)
	n = last_map_sym;

      /* Scan up to the location being disassembled.  */
      for (; n < info->symtab_size; n++)
	{
	  addr = bfd_asymbol_value (info->symtab[n]);
	  if (addr > memaddr)
	    break;
	  if ((info->section == NULL
	       || info->section == info->symtab[n]->section)
	      && get_sym_code_type (info, n, &type))
	    last_sym = n;
	}
      last_map_sym = last_sym;
      last_type = type;
      is_data = (last_type == CUR_DATA);
      if (is_data)
	{
	  size = 4 - ( memaddr & 3);
	  for (n = last_sym + 1; n < info->symtab_size; n++)
	    {
	      addr = bfd_asymbol_value (info->symtab[n]);
	      if (addr > memaddr)
		{
		  if (addr - memaddr < size)
		    size = addr - memaddr;
		  break;
		}
	    }
	  /* If the next symbol is after three bytes, we need to
	     print only part of the data, so that we can use either
	     .byte or .short.  */
	  if (size == 3)
	    size = (memaddr & 1) ? 1 : 2;
	}
    }
  info->bytes_per_line = 4;

  if (is_data)
    {
      int i;

      /* Size was already set above.  */
      info->bytes_per_chunk = size;
      printer = print_insn_data;

      status = info->read_memory_func (memaddr, (bfd_byte *) buf, size, info);
      given = 0;
      if (info->endian == BFD_ENDIAN_LITTLE)
	for (i = size - 1; i >= 0; i--)
	  given = buf[i] | (given << 8);
      else
	for (i = 0; i < (int) size; i++)
	  given = buf[i] | (given << 8);

      printer (memaddr, info, given);
      return info->bytes_per_chunk;
    }

  /* Handle instructions.  */
  CSKY_READ_DATA();
  if ((inst & 0xc000) == 0xc000 && IS_CSKY_V2 (mach_flag))
    {
      /* It's a 32-bit instruction.  */
      inst <<= 16;
      CSKY_READ_DATA();
      if (info->buffer && (info->endian == BFD_ENDIAN_LITTLE))
	{
	  char* src = (char *)(info->buffer
			       + ((memaddr - 4 - info->buffer_vma)
				  * info->octets_per_byte));
	  if (info->endian == BFD_ENDIAN_LITTLE)
	    number_to_chars_littleendian (src, inst, 4);
	}
    }

  if (IS_CSKY_V1 (mach_flag))
    g_opcodeP = csky_v1_opcodes;
  else
    g_opcodeP = csky_v2_opcodes;

  do
    {
      struct csky_opcode const *op;
      struct csky_opcode_info const *pinfo = NULL;
      int reloc;

      memset (str, 0, sizeof (str));
      op = csky_find_inst_info (&pinfo, inst, info->bytes_per_chunk);
      if (!op)
	{
	  if (IS_CSKY_V1 (mach_flag))
	    info->fprintf_func (info->stream, ".short: 0x%04x",
				(unsigned short)inst);
	  else
	    info->fprintf_func (info->stream, ".long: 0x%08x",
				(unsigned int)inst);
	  return info->bytes_per_chunk;
	}

      if (info->bytes_per_chunk == 2)
	reloc = op->reloc16;
      else
	reloc = op->reloc32;
      dis_info.opinfo = pinfo;
      strcat (str, op->mnemonic);

      if (csky_print_operands (str, pinfo, info, inst, reloc))
	g_opcodeP++;
      else
	break;
    } while (1);

  return info->bytes_per_chunk;
}
