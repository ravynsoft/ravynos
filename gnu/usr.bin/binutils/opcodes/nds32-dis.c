/* NDS32-specific support for 32-bit ELF.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Andes Technology Corporation.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "sysdep.h"
#include <stdio.h>
#include "ansidecl.h"
#include "disassemble.h"
#include "bfd.h"
#include "symcat.h"
#include "libiberty.h"
#include "opintl.h"
#include <stdint.h>
#include "hashtab.h"
#include "nds32-asm.h"
#include "opcode/nds32.h"

/* Get fields macro define.  */
#define MASK_OP(insn, mask)	((insn) & (0x3f << 25 | (mask)))

/* For mapping symbol.  */
enum map_type
{
  MAP_DATA0,
  MAP_DATA1,
  MAP_DATA2,
  MAP_DATA3,
  MAP_DATA4,
  MAP_CODE,
};

struct nds32_private_data
{
  /* Whether any mapping symbols are present in the provided symbol
     table.  -1 if we do not know yet, otherwise 0 or 1.  */
  int has_mapping_symbols;

  /* Track the last type (although this doesn't seem to be useful).  */
  enum map_type last_mapping_type;

  /* Tracking symbol table information.  */
  int last_symbol_index;
  bfd_vma last_addr;
};

/* Default text to print if an instruction isn't recognized.  */
#define UNKNOWN_INSN_MSG _("*unknown*")
#define NDS32_PARSE_INSN16      0x01
#define NDS32_PARSE_INSN32      0x02

static uint32_t nds32_mask_opcode (uint32_t);
static void nds32_special_opcode (uint32_t, struct nds32_opcode **);
static int get_mapping_symbol_type (struct disassemble_info *, int,
				    enum map_type *);
static int is_mapping_symbol (struct disassemble_info *, int,
			      enum map_type *);

/* Hash function for disassemble.  */

static htab_t opcode_htab;

/* Find the value map register name.  */

static const keyword_t *
nds32_find_reg_keyword (const keyword_t *reg, int value)
{
  if (!reg)
    return NULL;

  while (reg->name != NULL && reg->value != value)
    {
      reg++;
    }
  if (reg->name == NULL)
    return NULL;
  return reg;
}

static void
nds32_parse_audio_ext (const field_t *pfd,
		       disassemble_info *info, uint32_t insn)
{
  fprintf_ftype func = info->fprintf_func;
  void *stream = info->stream;
  const keyword_t *psys_reg;
  int int_value, new_value;

  if (pfd->hw_res == HW_INT || pfd->hw_res == HW_UINT)
    {
      if (pfd->hw_res == HW_INT)
	int_value = (unsigned) N32_IMMS (insn >> pfd->bitpos,
					 pfd->bitsize) << pfd->shift;
      else
	int_value = __GF (insn, pfd->bitpos, pfd->bitsize) << pfd->shift;

      if (int_value < 10)
	func (stream, "#%d", int_value);
      else
	func (stream, "#0x%x", int_value);
      return;
    }
  int_value =
    __GF (insn, pfd->bitpos, pfd->bitsize) << pfd->shift;
  new_value = int_value;
  psys_reg = (keyword_t*) nds32_keywords[pfd->hw_res];

  /* p = bit[4].bit[1:0], r = bit[4].bit[3:2].  */
  if (strcmp (pfd->name, "im5_i") == 0)
    {
      new_value = int_value & 0x03;
      new_value |= ((int_value & 0x10) >> 2);
    }
  else if (strcmp (pfd->name, "im5_m") == 0)
    {
      new_value = ((int_value & 0x1C) >> 2);
    }
  /* p = 0.bit[1:0], r = 0.bit[3:2].  */
  /* q = 1.bit[1:0], s = 1.bit[5:4].  */
  else if (strcmp (pfd->name, "im6_iq") == 0)
    {
      new_value |= 0x04;
    }
  else if (strcmp (pfd->name, "im6_ms") == 0)
    {
      new_value |= 0x04;
    }
  /*  Rt CONCAT(c, t21, t0).  */
  else if (strcmp (pfd->name, "a_rt21") == 0)
    {
      new_value = (insn & 0x00000020) >> 5;
      new_value |= (insn & 0x00000C00) >> 9;
      new_value |= (insn & 0x00008000) >> 12;
    }
  else if (strcmp (pfd->name, "a_rte") == 0)
    {
      new_value = (insn & 0x00000C00) >> 9;
      new_value |= (insn & 0x00008000) >> 12;
    }
  else if (strcmp (pfd->name, "a_rte1") == 0)
    {
      new_value = (insn & 0x00000C00) >> 9;
      new_value |= (insn & 0x00008000) >> 12;
      new_value |= 0x01;
    }
  else if (strcmp (pfd->name, "a_rte69") == 0)
    {
      new_value = int_value << 1;
    }
  else if (strcmp (pfd->name, "a_rte69_1") == 0)
    {
      new_value = int_value << 1;
      new_value |= 0x01;
    }

  psys_reg = nds32_find_reg_keyword (psys_reg, new_value);
  if (!psys_reg)
    func (stream, "???");
  else
    func (stream, "$%s", psys_reg->name);
}

/* Match instruction opcode with keyword table.  */

static field_t *
match_field (char *name)
{
  field_t *pfd;
  int k;

  for (k = 0; k < NDS32_CORE_COUNT; k++)
    {
      pfd = (field_t *) nds32_field_table[k];
      while (1)
	{
	  if (pfd->name == NULL)
	    break;
	  if (strcmp (name, pfd->name) == 0)
	    return pfd;
	  pfd++;
	}
    }

  return NULL;
}

/* Dump instruction.  If the opcode is unknown, return FALSE.  */

static void
nds32_parse_opcode (struct nds32_opcode *opc, bfd_vma pc ATTRIBUTE_UNUSED,
		    disassemble_info *info, uint32_t insn,
		    uint32_t parse_mode)
{
  int op = 0;
  fprintf_ftype func = info->fprintf_func;
  void *stream = info->stream;
  const char *pstr_src;
  char *pstr_tmp;
  char tmp_string[16];
  unsigned int push25gpr = 0, lsmwRb, lsmwRe, lsmwEnb4, checkbit, i;
  int int_value, ifthe1st = 1;
  const field_t *pfd;
  const keyword_t *psys_reg;

  if (opc == NULL)
    {
      func (stream, UNKNOWN_INSN_MSG);
      return;
    }

  pstr_src = opc->instruction;
  if (*pstr_src == 0)
    {
      func (stream, "%s", opc->opcode);
      return;
    }
  /* NDS32_PARSE_INSN16.  */
  if (parse_mode & NDS32_PARSE_INSN16)
    {
      func (stream, "%s ", opc->opcode);
    }

  /* NDS32_PARSE_INSN32.  */
  else
    {
      op = N32_OP6 (insn);
      if (op == N32_OP6_LSMW)
	func (stream, "%s.", opc->opcode);
      else if (strstr (opc->instruction, "tito"))
	func (stream, "%s", opc->opcode);
      else
	func (stream, "%s\t", opc->opcode);
    }

  while (*pstr_src)
    {
      switch (*pstr_src)
	{
	case '%':
	case '=':
	case '&':
	  pstr_src++;
	  /* Compare with nds32_operand_fields[].name.  */
	  pstr_tmp = &tmp_string[0];
	  while (*pstr_src)
	    {
	      if ((*pstr_src == ',') || (*pstr_src == ' ')
		  || (*pstr_src == '{') || (*pstr_src == '}')
		  || (*pstr_src == '[') || (*pstr_src == ']')
		  || (*pstr_src == '(') || (*pstr_src == ')')
		  || (*pstr_src == '+') || (*pstr_src == '<'))
		break;
	      *pstr_tmp++ = *pstr_src++;
	    }
	  *pstr_tmp = 0;

	  if ((pfd = match_field (&tmp_string[0])) == NULL)
	    return;

	  /* For insn-16.  */
	  if (parse_mode & NDS32_PARSE_INSN16)
	    {
	      if (pfd->hw_res == HW_GPR)
		{
		  int_value =
		    __GF (insn, pfd->bitpos, pfd->bitsize) << pfd->shift;
		  /* push25/pop25.  */
		  if ((opc->value == 0xfc00) || (opc->value == 0xfc80))
		    {
		      if (int_value == 0)
			int_value = 6;
		      else
			int_value = (6 + (0x01 << int_value));
		      push25gpr = int_value;
		    }
		  else if (strcmp (pfd->name, "rt4") == 0)
		    {
		      int_value = nds32_r45map[int_value];
		    }
		  func (stream, "$%s", nds32_keyword_gpr[int_value].name);
		}
	      else if ((pfd->hw_res == HW_INT) || (pfd->hw_res == HW_UINT))
		{
		  if (pfd->hw_res == HW_INT)
		    int_value
		      = (unsigned) N32_IMMS (insn >> pfd->bitpos,
					     pfd->bitsize) << pfd->shift;
		  else
		    int_value =
		      __GF (insn, pfd->bitpos, pfd->bitsize) << pfd->shift;

		  /* movpi45.  */
		  if (opc->value == 0xfa00)
		    {
		      int_value += 16;
		      func (stream, "#0x%x", int_value);
		    }
		  /* lwi45.fe.  */
		  else if (opc->value == 0xb200)
		    {
		      int_value = 0 - (128 - int_value);
		      func (stream, "#%d", int_value);
		    }
		  /* beqz38/bnez38/beqs38/bnes38/j8/beqzs8/bnezs8.  */
		  else if ((opc->value == 0xc000) || (opc->value == 0xc800)
			   || (opc->value == 0xd000) || (opc->value == 0xd800)
			   || (opc->value == 0xd500) || (opc->value == 0xe800)
			   || (opc->value == 0xe900))
		    {
		      info->print_address_func (int_value + pc, info);
		    }
		  /* push25/pop25.  */
		  else if ((opc->value == 0xfc00) || (opc->value == 0xfc80))
		    {
		      func (stream, "#%d    ! {$r6", int_value);
		      if (push25gpr != 6)
			func (stream, "~$%s", nds32_keyword_gpr[push25gpr].name);
		      func (stream, ", $fp, $gp, $lp}");
		    }
		  else if (pfd->hw_res == HW_INT)
		    {
		      if (int_value < 10)
			func (stream, "#%d", int_value);
		      else
			func (stream, "#0x%x", int_value);
		    }
		  else /* if (pfd->hw_res == HW_UINT).  */
		    {
		      if (int_value < 10)
			func (stream, "#%u", int_value);
		      else
			func (stream, "#0x%x", int_value);
		    }
		}

	    }
	  /* for audio-ext.  */
	  else if (op == N32_OP6_AEXT)
	    {
	      nds32_parse_audio_ext (pfd, info, insn);
	    }
	  /* for insn-32.  */
	  else if (pfd->hw_res < HW_INT)
	    {
	      int_value =
		__GF (insn, pfd->bitpos, pfd->bitsize) << pfd->shift;

	      psys_reg = *(nds32_keyword_table[pfd->hw_res >> 8]
			   + (pfd->hw_res & 0xff));

	      psys_reg = nds32_find_reg_keyword (psys_reg, int_value);
	      /* For HW_SR, dump the index when it can't
		 map the register name.  */
	      if (!psys_reg && pfd->hw_res == HW_SR)
		func (stream, "%d", int_value);
	      else if (!psys_reg)
		func (stream, "???");
	      else
		{
		  if (pfd->hw_res == HW_GPR || pfd->hw_res == HW_CPR
		      || pfd->hw_res == HW_FDR || pfd->hw_res == HW_FSR
		      || pfd->hw_res == HW_DXR || pfd->hw_res == HW_SR
		      || pfd->hw_res == HW_USR)
		    func (stream, "$%s", psys_reg->name);
		  else if (pfd->hw_res == HW_DTITON
			   || pfd->hw_res == HW_DTITOFF)
		    func (stream, ".%s", psys_reg->name);
		  else
		    func (stream, "%s", psys_reg->name);
		}
	    }
	  else if ((pfd->hw_res == HW_INT) || (pfd->hw_res == HW_UINT))
	    {
	      if (pfd->hw_res == HW_INT)
		int_value = (unsigned) N32_IMMS (insn >> pfd->bitpos,
						 pfd->bitsize) << pfd->shift;
	      else
		int_value =
		  __GF (insn, pfd->bitpos, pfd->bitsize) << pfd->shift;

	      if ((op == N32_OP6_BR1) || (op == N32_OP6_BR2))
		{
		  info->print_address_func (int_value + pc, info);
		}
	      else if ((op == N32_OP6_BR3) && (pfd->bitpos == 0))
		{
		  info->print_address_func (int_value + pc, info);
		}
	      else if (op == N32_OP6_JI)
		{
		  /* FIXME: Handle relocation.  */
		  if (info->flags & INSN_HAS_RELOC)
		    pc = 0;
		  info->print_address_func (int_value + pc, info);
		}
	      else if (op == N32_OP6_LSMW)
		{
		  /* lmw.adm/smw.adm.  */
		  func (stream, "#0x%x    ! {", int_value);
		  lsmwEnb4 = int_value;
		  lsmwRb = ((insn >> 20) & 0x1F);
		  lsmwRe = ((insn >> 10) & 0x1F);

		  /* If [Rb, Re] specifies at least one register,
		     Rb(4,0) <= Re(4,0) and 0 <= Rb(4,0), Re(4,0) < 28.
		     Disassembling does not consider this currently because of
		     the convience comparing with bsp320.  */
		  if (lsmwRb != 31 || lsmwRe != 31)
		    {
		      func (stream, "$%s", nds32_keyword_gpr[lsmwRb].name);
		      if (lsmwRb != lsmwRe)
			func (stream, "~$%s", nds32_keyword_gpr[lsmwRe].name);
		      ifthe1st = 0;
		    }
		  if (lsmwEnb4 != 0)
		    {
		      /* $fp, $gp, $lp, $sp.  */
		      checkbit = 0x08;
		      for (i = 0; i < 4; i++)
			{
			  if (lsmwEnb4 & checkbit)
			    {
			      if (ifthe1st == 1)
				{
				  ifthe1st = 0;
				  func (stream, "$%s", nds32_keyword_gpr[28 + i].name);
				}
			      else
				func (stream, ", $%s", nds32_keyword_gpr[28 + i].name);
			    }
			  checkbit >>= 1;
			}
		    }
		  func (stream, "}");
		}
	      else if (pfd->hw_res == HW_INT)
		{
		  if (int_value < 10)
		    func (stream, "#%d", int_value);
		  else
		    func (stream, "#0x%x", int_value);
		}
	      else /* if (pfd->hw_res == HW_UINT).  */
		{
		  if (int_value < 10)
		    func (stream, "#%u", int_value);
		  else
		    func (stream, "#0x%x", int_value);
		}
	    }
	  break;

	case '{':
	case '}':
	  pstr_src++;
	  break;

	case ',':
	  func (stream, ", ");
	  pstr_src++;
	  break;
	  
	case '+':
	  func (stream, " + ");
	  pstr_src++;
	  break;
	  
	case '<':
	  if (pstr_src[1] == '<')
	    {
	      func (stream, " << ");
	      pstr_src += 2;
	    }
	  else
	    {
	      func (stream, " <");
	      pstr_src++;
	    }
	  break;
	  
	default:
	  func (stream, "%c", *pstr_src++);
	  break;
	}
    }
}

/* Filter instructions with some bits must be fixed.  */

static void
nds32_filter_unknown_insn (uint32_t insn, struct nds32_opcode **opc)
{
  if (!(*opc))
    return;

  switch ((*opc)->value)
    {
    case JREG (JR):
    case JREG (JRNEZ):
      /* jr jr.xtoff */
      if (__GF (insn, 6, 2) != 0 || __GF (insn, 15, 10) != 0)
        *opc = NULL;
      break;
    case MISC (STANDBY):
      if (__GF (insn, 7, 18) != 0)
        *opc = NULL;
      break;
    case SIMD (PBSAD):
    case SIMD (PBSADA):
      if (__GF (insn, 5, 5) != 0)
        *opc = NULL;
      break;
    case BR2 (SOP0):
      if (__GF (insn, 20, 5) != 0)
        *opc = NULL;
      break;
    case JREG (JRAL):
      if (__GF (insn, 5, 3) != 0 || __GF (insn, 15, 5) != 0)
        *opc = NULL;
      break;
    case ALU1 (NOR):
    case ALU1 (SLT):
    case ALU1 (SLTS):
    case ALU1 (SLLI):
    case ALU1 (SRLI):
    case ALU1 (SRAI):
    case ALU1 (ROTRI):
    case ALU1 (SLL):
    case ALU1 (SRL):
    case ALU1 (SRA):
    case ALU1 (ROTR):
    case ALU1 (SEB):
    case ALU1 (SEH):
    case ALU1 (ZEH):
    case ALU1 (WSBH):
    case ALU1 (SVA):
    case ALU1 (SVS):
    case ALU1 (CMOVZ):
    case ALU1 (CMOVN):
      if (__GF (insn, 5, 5) != 0)
        *opc = NULL;
      break;
    case MISC (IRET):
    case MISC (ISB):
    case MISC (DSB):
      if (__GF (insn, 5, 20) != 0)
        *opc = NULL;
      break;
    }
}

static void
print_insn32 (bfd_vma pc, disassemble_info *info, uint32_t insn,
	      uint32_t parse_mode)
{
  /* Get the final correct opcode and parse.  */
  struct nds32_opcode *opc;
  uint32_t opcode = nds32_mask_opcode (insn);
  opc = (struct nds32_opcode *) htab_find (opcode_htab, &opcode);

  nds32_special_opcode (insn, &opc);
  nds32_filter_unknown_insn (insn, &opc);
  nds32_parse_opcode (opc, pc, info, insn, parse_mode);
}

static void
print_insn16 (bfd_vma pc, disassemble_info *info,
	      uint32_t insn, uint32_t parse_mode)
{
  struct nds32_opcode *opc;
  uint32_t opcode;

  /* Get highest 7 bit in default.  */
  unsigned int mask = 0xfe00;

  /* Classify 16-bit instruction to 4 sets by bit 13 and 14.  */
  switch (__GF (insn, 13, 2))
    {
    case 0x0:
      /* mov55 movi55 */
      if (__GF (insn, 11, 2) == 0)
	{
	  mask = 0xfc00;
	  /* ifret16 = mov55 $sp, $sp*/
	  if (__GF (insn, 0, 11) == 0x3ff)
	    mask = 0xffff;
	}
      else if (__GF (insn, 9, 4) == 0xb)
	mask = 0xfe07;
      break;
    case 0x1:
      /* lwi37 swi37 */
      if (__GF (insn, 11, 2) == 0x3)
	mask = 0xf880;
      break;
    case 0x2:
      mask = 0xf800;
      /* Exclude beqz38, bnez38, beqs38, and bnes38.  */
      if (__GF (insn, 12, 1) == 0x1
	  && __GF (insn, 8, 3) == 0x5)
	{
	  if (__GF (insn, 11, 1) == 0x0)
	    mask = 0xff00;
	  else
	    mask = 0xffe0;
	}
      break;
    case 0x3:
      switch (__GF (insn, 11, 2))
	{
	case 0x1:
	  /* beqzs8 bnezs8 */
	  if (__GF (insn, 9, 2) == 0x0)
	    mask = 0xff00;
	  /* addi10s */
	  else if (__GF(insn, 10, 1) == 0x1)
	    mask = 0xfc00;
	  break;
	case 0x2:
	  /* lwi37.sp swi37.sp */
	  mask = 0xf880;
	  break;
	case 0x3:
	  if (__GF (insn, 8, 3) == 0x5)
	    mask = 0xff00;
	  else if (__GF (insn, 8, 3) == 0x4)
	    mask = 0xff80;
	  else if (__GF (insn, 9 , 2) == 0x3)
	    mask = 0xfe07;
	  break;
	}
      break;
    }
  opcode = insn & mask;
  opc = (struct nds32_opcode *) htab_find (opcode_htab, &opcode);

  nds32_special_opcode (insn, &opc);
  /* Get the final correct opcode and parse it.  */
  nds32_parse_opcode (opc, pc, info, insn, parse_mode);
}

static hashval_t
htab_hash_hash (const void *p)
{
  return (*(unsigned int *) p) % 49;
}

static int
htab_hash_eq (const void *p, const void *q)
{
  uint32_t pinsn = ((struct nds32_opcode *) p)->value;
  uint32_t qinsn = *((uint32_t *) q);

  return (pinsn == qinsn);
}

/* Get the format of instruction.  */

static uint32_t
nds32_mask_opcode (uint32_t insn)
{
  uint32_t opcode = N32_OP6 (insn);
  switch (opcode)
    {
    case N32_OP6_LBI:
    case N32_OP6_LHI:
    case N32_OP6_LWI:
    case N32_OP6_LDI:
    case N32_OP6_LBI_BI:
    case N32_OP6_LHI_BI:
    case N32_OP6_LWI_BI:
    case N32_OP6_LDI_BI:
    case N32_OP6_SBI:
    case N32_OP6_SHI:
    case N32_OP6_SWI:
    case N32_OP6_SDI:
    case N32_OP6_SBI_BI:
    case N32_OP6_SHI_BI:
    case N32_OP6_SWI_BI:
    case N32_OP6_SDI_BI:
    case N32_OP6_LBSI:
    case N32_OP6_LHSI:
    case N32_OP6_LWSI:
    case N32_OP6_LBSI_BI:
    case N32_OP6_LHSI_BI:
    case N32_OP6_LWSI_BI:
    case N32_OP6_MOVI:
    case N32_OP6_SETHI:
    case N32_OP6_ADDI:
    case N32_OP6_SUBRI:
    case N32_OP6_ANDI:
    case N32_OP6_XORI:
    case N32_OP6_ORI:
    case N32_OP6_SLTI:
    case N32_OP6_SLTSI:
    case N32_OP6_CEXT:
    case N32_OP6_BITCI:
      return MASK_OP (insn, 0);
    case N32_OP6_ALU2:
      /* FFBI */
      if (__GF (insn, 0, 7) == (N32_ALU2_FFBI | N32_BIT (6)))
	return MASK_OP (insn, 0x7f);
      else if (__GF (insn, 0, 7) == (N32_ALU2_MFUSR | N32_BIT (6))
	       || __GF (insn, 0, 7) == (N32_ALU2_MTUSR | N32_BIT (6)))
	/* RDOV CLROV */
	return MASK_OP (insn, 0xf81ff);
      else if (__GF (insn, 0, 10) == (N32_ALU2_ONEOP | N32_BIT (7)))
	{
	  /* INSB */
	  if (__GF (insn, 12, 3) == 4)
	    return MASK_OP (insn, 0x73ff);
	  return MASK_OP (insn, 0x7fff);
	}
      return MASK_OP (insn, 0x3ff);
    case N32_OP6_ALU1:
    case N32_OP6_SIMD:
      return MASK_OP (insn, 0x1f);
    case N32_OP6_MEM:
      return MASK_OP (insn, 0xff);
    case N32_OP6_JREG:
      return MASK_OP (insn, 0x7f);
    case N32_OP6_LSMW:
      return MASK_OP (insn, 0x23);
    case N32_OP6_SBGP:
    case N32_OP6_LBGP:
      return MASK_OP (insn, 0x1 << 19);
    case N32_OP6_HWGP:
      if (__GF (insn, 18, 2) == 0x3)
	return MASK_OP (insn, 0x7 << 17);
      return MASK_OP (insn, 0x3 << 18);
    case N32_OP6_DPREFI:
      return MASK_OP (insn, 0x1 << 24);
    case N32_OP6_LWC:
    case N32_OP6_SWC:
    case N32_OP6_LDC:
    case N32_OP6_SDC:
      return MASK_OP (insn, 0x1 << 12);
    case N32_OP6_JI:
      return MASK_OP (insn, 0x1 << 24);
    case N32_OP6_BR1:
      return MASK_OP (insn, 0x1 << 14);
    case N32_OP6_BR2:
      if (__GF (insn, 16, 4) == 0)
	return MASK_OP (insn, 0x1ff << 16);
      else
	return MASK_OP (insn, 0xf << 16);
    case N32_OP6_BR3:
      return MASK_OP (insn, 0x1 << 19);
    case N32_OP6_MISC:
      switch (__GF (insn, 0, 5))
	{
	case N32_MISC_MTSR:
	  /* SETGIE and SETEND  */
	  if (__GF (insn, 5, 5) == 0x1 || __GF (insn, 5, 5) == 0x2)
	    return MASK_OP (insn, 0x1fffff);
	  return MASK_OP (insn, 0x1f);
	case N32_MISC_TLBOP:
	  if (__GF (insn, 5, 5) == 5 || __GF (insn, 5, 5) == 7)
	    /* PB FLUA  */
	    return MASK_OP (insn, 0x3ff);
	  return MASK_OP (insn, 0x1f);
	default:
	  return MASK_OP (insn, 0x1f);
	}
    case N32_OP6_COP:
      if (__GF (insn, 4, 2) == 0)
	{
	  /* FPU */
	  switch (__GF (insn, 0, 4))
	    {
	    case 0x0:
	    case 0x8:
	      /* FS1/F2OP FD1/F2OP */
	      if (__GF (insn, 6, 4) == 0xf)
		return MASK_OP (insn, 0x7fff);
	      /* FS1 FD1 */
	      return MASK_OP (insn, 0x3ff);
	    case 0x4:
	    case 0xc:
	      /* FS2 */
	      return MASK_OP (insn, 0x3ff);
	    case 0x1:
	    case 0x9:
	      /* XR */
	      if (__GF (insn, 6, 4) == 0xc)
		return MASK_OP (insn, 0x7fff);
	      /* MFCP MTCP */
	      return MASK_OP (insn, 0x3ff);
	    default:
	      return MASK_OP (insn, 0xff);
	    }
	}
      else if  (__GF (insn, 0, 2) == 0)
	return MASK_OP (insn, 0xf);
      return MASK_OP (insn, 0xcf);
    case N32_OP6_AEXT:
      /* AUDIO */
      switch (__GF (insn, 23, 2))
	{
	case 0x0:
	  if (__GF (insn, 5, 4) == 0)
	    /* AMxxx AMAyyS AMyyS AMAWzS AMWzS */
	    return MASK_OP (insn, (0x1f << 20) | 0x1ff);
	  else if (__GF (insn, 5, 4) == 1)
	    /* ALR ASR ALA ASA AUPI */
	    return MASK_OP (insn, (0x1f << 20) | (0xf << 5));
	  else if (__GF (insn, 20, 3) == 0 && __GF (insn, 6, 3) == 1)
	    /* ALR2 */
	    return MASK_OP (insn, (0x1f << 20) | (0x7 << 6));
	  else if (__GF (insn, 20 ,3) == 2 && __GF (insn, 6, 3) == 1)
	    /* AWEXT ASATS48 */
	    return MASK_OP (insn, (0x1f << 20) | (0xf << 5));
	  else if (__GF (insn, 20 ,3) == 3 && __GF (insn, 6, 3) == 1)
	    /* AMTAR AMTAR2 AMFAR AMFAR2 */
	    return MASK_OP (insn, (0x1f << 20) | (0x1f << 5));
	  else if (__GF (insn, 7, 2) == 3)
	    /* AMxxxSA */
	    return MASK_OP (insn, (0x1f << 20) | (0x3 << 7));
	  else if (__GF (insn, 6, 3) == 2)
	    /* AMxxxL.S  */
	    return MASK_OP (insn, (0x1f << 20) | (0xf << 5));
	  else
	    /* AmxxxL.l AmxxxL2.S AMxxxL2.L  */
	    return MASK_OP (insn, (0x1f << 20) | (0x7 << 6));
	case 0x1:
	  if (__GF (insn, 20, 3) == 0)
	    /* AADDL ASUBL */
	    return MASK_OP (insn, (0x1f << 20) | (0x1 << 5));
	  else if (__GF (insn, 20, 3) == 1)
	    /* AMTARI Ix AMTARI Mx */
	    return MASK_OP (insn, (0x1f << 20));
	  else if (__GF (insn, 6, 3) == 2)
	    /* AMAWzSl.S AMWzSl.S */
	    return MASK_OP (insn, (0x1f << 20) | (0xf << 5));
	  else if (__GF (insn, 7, 2) == 3)
	    /* AMAWzSSA AMWzSSA */
	    return MASK_OP (insn, (0x1f << 20) | (0x3 << 7));
	  else
	    /* AMAWzSL.L AMAWzSL2.S AMAWzSL2.L
	       AMWzSL.L AMWzSL.L AMWzSL2.S */
	    return MASK_OP (insn, (0x1f << 20) | (0x7 << 6));
	case 0x2:
	  if (__GF (insn, 6, 3) == 2)
	    /* AMAyySl.S AMWyySl.S */
	    return MASK_OP (insn, (0x1f << 20) | (0xf << 5));
	  else if (__GF (insn, 7, 2) == 3)
	    /* AMAWyySSA AMWyySSA */
	    return MASK_OP (insn, (0x1f << 20) | (0x3 << 7));
	  else
	    /* AMAWyySL.L AMAWyySL2.S AMAWyySL2.L
	       AMWyySL.L AMWyySL.L AMWyySL2.S */
	    return MASK_OP (insn, (0x1f << 20) | (0x7 << 6));
	}
      return MASK_OP (insn, 0x1f << 20);
    default:
      return 1u << 31;
    }
}

/* Define cctl subtype.  */
static char *cctl_subtype [] =
{
  /* 0x0 */
  "st0", "st0", "st0", "st2", "st2", "st3", "st3", "st4",
  "st1", "st1", "st1", "st0", "st0", NULL, NULL, "st5",
  /* 0x10 */
  "st0", NULL, NULL, "st2", "st2", "st3", "st3", NULL,
  "st1", NULL, NULL, "st0", "st0", NULL, NULL, NULL
};

/* Check the subset of opcode.  */

static void
nds32_special_opcode (uint32_t insn, struct nds32_opcode **opc)
{
  char *string = NULL;
  uint32_t op;

  if (!(*opc))
    return;

  /* Check if special case.  */
  switch ((*opc)->value)
    {
    case OP6 (LWC):
    case OP6 (SWC):
    case OP6 (LDC):
    case OP6 (SDC):
    case FPU_RA_IMMBI (LWC):
    case FPU_RA_IMMBI (SWC):
    case FPU_RA_IMMBI (LDC):
    case FPU_RA_IMMBI (SDC):
      /* Check if cp0 => FPU.  */
      if (__GF (insn, 13, 2) == 0)
      {
	while (!((*opc)->attr & ATTR (FPU)) && (*opc)->next)
	  *opc = (*opc)->next;
      }
      break;
    case ALU1 (ADD):
    case ALU1 (SUB):
    case ALU1 (AND):
    case ALU1 (XOR):
    case ALU1 (OR):
      /* Check if (add/add_slli) (sub/sub_slli) (and/and_slli).  */
      if (N32_SH5(insn) != 0)
        string = "sh";
      break;
    case ALU1 (SRLI):
      /* Check if nop.  */
      if (__GF (insn, 10, 15) == 0)
        string = "nop";
      break;
    case MISC (CCTL):
      string = cctl_subtype [__GF (insn, 5, 5)];
      break;
    case JREG (JR):
    case JREG (JRAL):
    case JREG (JR) | JREG_RET:
      if (__GF (insn, 8, 2) != 0)
	string = "tit";
      break;
    case N32_OP6_COP:
      break;
    case 0x9200:
      /* nop16 */
      if (__GF (insn, 0, 9) == 0)
	string = "nop16";
      break;
    }

  if (string)
    {
      while (strstr ((*opc)->opcode, string) == NULL
	     && strstr ((*opc)->instruction, string) == NULL && (*opc)->next)
	*opc = (*opc)->next;
      return;
    }

  /* Classify instruction is COP or FPU.  */
  op = N32_OP6 (insn);
  if (op == N32_OP6_COP && __GF (insn, 4, 2) != 0)
    {
      while (((*opc)->attr & ATTR (FPU)) != 0 && (*opc)->next)
	*opc = (*opc)->next;
    }
}

int
print_insn_nds32 (bfd_vma pc, disassemble_info *info)
{
  int status;
  bfd_byte buf[4];
  bfd_byte buf_data[16];
  uint64_t given;
  uint64_t given1;
  uint32_t insn;
  int n;
  int last_symbol_index = -1;
  bfd_vma addr;
  int is_data = false;
  bool found = false;
  struct nds32_private_data *private_data;
  unsigned int size;
  enum map_type mapping_type = MAP_CODE;

  if (info->private_data == NULL)
    {
      /* Note: remain lifecycle throughout whole execution.  */
      static struct nds32_private_data private;
      private.has_mapping_symbols = -1;	/* unknown yet.  */
      private.last_symbol_index = -1;
      private.last_addr = 0;
      info->private_data = &private;
    }
  private_data = info->private_data;

  if (info->symtab_size != 0)
    {
      int start;
      if (pc == 0)
	start = 0;
      else
	{
	  start = info->symtab_pos;
	  if (start < private_data->last_symbol_index)
	    start = private_data->last_symbol_index;
	}

      if (0 > start)
	start = 0;

      if (private_data->has_mapping_symbols != 0
	  && ((strncmp (".text", info->section->name, 5) == 0)))
	{
	  for (n = start; n < info->symtab_size; n++)
	    {
	      addr = bfd_asymbol_value (info->symtab[n]);
	      if (addr > pc)
		break;
	      if (get_mapping_symbol_type (info, n, &mapping_type))
		{
		  last_symbol_index = n;
		  found = true;
		}
	    }

	  if (found)
	    private_data->has_mapping_symbols = 1;
	  else if (!found && private_data->has_mapping_symbols == -1)
	    {
	      /* Make sure there are no any mapping symbol.  */
	      for (n = 0; n < info->symtab_size; n++)
		{
		  if (is_mapping_symbol (info, n, &mapping_type))
		    {
		      private_data->has_mapping_symbols = -1;
		      break;
		    }
		}
	      if (private_data->has_mapping_symbols == -1)
		private_data->has_mapping_symbols = 0;
	    }

	  private_data->last_symbol_index = last_symbol_index;
	  private_data->last_mapping_type = mapping_type;
	  is_data = (private_data->last_mapping_type == MAP_DATA0
		     || private_data->last_mapping_type == MAP_DATA1
		     || private_data->last_mapping_type == MAP_DATA2
		     || private_data->last_mapping_type == MAP_DATA3
		     || private_data->last_mapping_type == MAP_DATA4);
	}
    }

  /* Wonder data or instruction.  */
  if (is_data)
    {
      unsigned int i1;

      /* Fix corner case: there is no next mapping symbol,
	 let mapping type decides size */
      size = 16;
      if (last_symbol_index + 1 >= info->symtab_size)
	{
	  if (mapping_type == MAP_DATA0)
	    size = 1;
	  if (mapping_type == MAP_DATA1)
	    size = 2;
	  if (mapping_type == MAP_DATA2)
	    size = 4;
	  if (mapping_type == MAP_DATA3)
	    size = 8;
	  if (mapping_type == MAP_DATA4)
	    size = 16;
	}
      for (n = last_symbol_index + 1; n < info->symtab_size; n++)
	{
	  addr = bfd_asymbol_value (info->symtab[n]);

	  enum map_type fake_mapping_type;
	  if (get_mapping_symbol_type (info, n, &fake_mapping_type)
	      && (addr > pc
		  && ((info->section == NULL)
		      || (info->section == info->symtab[n]->section)))
	      && (addr - pc < size))
	    {
	      size = addr - pc;
	      break;
	    }
	}

      if (size == 3)
	size = (pc & 1) ? 1 : 2;

      /* Read bytes from BFD.  */
      info->read_memory_func (pc, buf_data, size, info);
      given = 0;
      given1 = 0;
      /* Start assembling data.  */
      /* Little endian of data.  */
      if (info->endian == BFD_ENDIAN_LITTLE)
	{
	  for (i1 = size - 1;; i1--)
	    {
	      if (i1 >= 8)
		given1 = buf_data[i1] | (given1 << 8);
	      else
		given = buf_data[i1] | (given << 8);

	      if (i1 == 0)
		break;
	    }
	}
      else
	{
	  /* Big endian of data.  */
	  for (i1 = 0; i1 < size; i1++)
	    {
	      if (i1 <= 7)
		given = buf_data[i1] | (given << 8);
	      else
		given1 = buf_data[i1] | (given1 << 8);
	    }
	}

      info->bytes_per_line = 4;

      if (size == 16)
	info->fprintf_func (info->stream, ".qword\t0x%016" PRIx64 "%016" PRIx64,
			    given, given1);
      else if (size == 8)
	info->fprintf_func (info->stream, ".dword\t0x%016" PRIx64, given);
      else if (size == 4)
	info->fprintf_func (info->stream, ".word\t0x%08" PRIx64, given);
      else if (size == 2)
	{
	  /* short */
	  if (mapping_type == MAP_DATA0)
	    info->fprintf_func (info->stream, ".byte\t0x%02" PRIx64,
				given & 0xFF);
	  else
	    info->fprintf_func (info->stream, ".short\t0x%04" PRIx64, given);
	}
      else
	{
	  /* byte */
	  info->fprintf_func (info->stream, ".byte\t0x%02" PRIx64, given);
	}

      return size;
    }

  size = 4;
  status = info->read_memory_func (pc, buf, 4, info);
  if (status)
    {
      /* For the last 16-bit instruction.  */
      size = 2;
      status = info->read_memory_func (pc, buf, 2, info);
      if (status)
	{
	  (*info->memory_error_func) (status, pc, info);
	  return -1;
	}
      buf[2] = 0;
      buf[3] = 0;
    }

  insn = bfd_getb32 (buf);
  /* 16-bit instruction.  */
  if (insn & 0x80000000)
    {
      print_insn16 (pc, info, (insn >> 16), NDS32_PARSE_INSN16);
      return 2;
    }

  /* 32-bit instructions.  */
  if (size == 4)
    print_insn32 (pc, info, insn, NDS32_PARSE_INSN32);
  else
    info->fprintf_func (info->stream,
			_("insufficient data to decode instruction"));
  return 4;
}

/* Ignore disassembling unnecessary name.  */

static bool
nds32_symbol_is_valid (asymbol *sym,
		       struct disassemble_info *info ATTRIBUTE_UNUSED)
{
  const char *name;

  if (sym == NULL)
    return false;

  name = bfd_asymbol_name (sym);

  /* Mapping symbol is invalid.  */
  if (name[0] == '$')
    return false;
  return true;
}

static void
nds32_add_opcode_hash_table (unsigned indx)
{
  opcode_t *opc;

  opc = nds32_opcode_table[indx];
  if (opc == NULL)
    return;

  while (opc->opcode != NULL)
    {
      opcode_t **slot;

      slot = (opcode_t **) htab_find_slot
	(opcode_htab, &opc->value, INSERT);
      if (*slot == NULL)
	{
	  /* This is the new one.  */
	  *slot = opc;
	}
      else
	{
	  opcode_t *tmp;

	  /* Already exists.  Append to the list.  */
	  tmp = *slot;
	  while (tmp->next)
	    tmp = tmp->next;
	  tmp->next = opc;
	  opc->next = NULL;
	}
      opc++;
    }
}

void
disassemble_init_nds32 (struct disassemble_info *info)
{
  static unsigned init_done = 0;
  unsigned k;

  /* Set up symbol checking function.  */
  info->symbol_is_valid = nds32_symbol_is_valid;

  /* Only need to initialize once:
     High level will call this function for every object file.
     For example, when disassemble all members of a library.  */
  if (init_done)
    return;

  /* Setup main core.  */
  nds32_keyword_table[NDS32_MAIN_CORE] = &nds32_keywords[0];
  nds32_opcode_table[NDS32_MAIN_CORE] = &nds32_opcodes[0];
  nds32_field_table[NDS32_MAIN_CORE] = &nds32_operand_fields[0];

  /* Build opcode table.  */
  opcode_htab = htab_create_alloc (1024, htab_hash_hash, htab_hash_eq,
				   NULL, xcalloc, free);

  for (k = 0; k < NDS32_CORE_COUNT; k++)
    {
      /* Add op-codes.  */
      nds32_add_opcode_hash_table (k);
    }

  init_done = 1;
}

static int
is_mapping_symbol (struct disassemble_info *info, int n,
		   enum map_type *map_type)
{
  const char *name = NULL;

  /* Get symbol name.  */
  name = bfd_asymbol_name (info->symtab[n]);

  if (name[1] == 'c')
    {
      *map_type = MAP_CODE;
      return true;
    }
  else if (name[1] == 'd' && name[2] == '0')
    {
      *map_type = MAP_DATA0;
      return true;
    }
  else if (name[1] == 'd' && name[2] == '1')
    {
      *map_type = MAP_DATA1;
      return true;
    }
  else if (name[1] == 'd' && name[2] == '2')
    {
      *map_type = MAP_DATA2;
      return true;
    }
  else if (name[1] == 'd' && name[2] == '3')
    {
      *map_type = MAP_DATA3;
      return true;
    }
  else if (name[1] == 'd' && name[2] == '4')
    {
      *map_type = MAP_DATA4;
      return true;
    }

  return false;
}

static int
get_mapping_symbol_type (struct disassemble_info *info, int n,
			 enum map_type *map_type)
{
  /* If the symbol is in a different section, ignore it.  */
  if (info->section != NULL
      && info->section != info->symtab[n]->section)
    return false;

  return is_mapping_symbol (info, n, map_type);
}
