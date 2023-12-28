/* Disassemble Xilinx microblaze instructions.

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
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


#include "sysdep.h"
#define STATIC_TABLE
#define DEFINE_TABLE

#include "disassemble.h"
#include <strings.h>
#include "microblaze-opc.h"
#include "microblaze-dis.h"

#define get_field_rd(buf, instr)   get_field (buf, instr, RD_MASK, RD_LOW)
#define get_field_r1(buf, instr)   get_field (buf, instr, RA_MASK, RA_LOW)
#define get_field_r2(buf, instr)   get_field (buf, instr, RB_MASK, RB_LOW)
#define get_int_field_imm(instr)   ((instr & IMM_MASK) >> IMM_LOW)
#define get_int_field_r1(instr)    ((instr & RA_MASK) >> RA_LOW)

#define NUM_STRBUFS 3
#define STRBUF_SIZE 25

struct string_buf
{
  unsigned int which;
  char str[NUM_STRBUFS][STRBUF_SIZE];
};

static inline char *
strbuf (struct string_buf *buf)
{
#ifdef ENABLE_CHECKING
  if (buf->which >= NUM_STRBUFS)
    abort ();
#endif
  return buf->str[buf->which++];
}

static char *
get_field (struct string_buf *buf, long instr, long mask, unsigned short low)
{
  char *p = strbuf (buf);

  sprintf (p, "%s%d", register_prefix, (int)((instr & mask) >> low));
  return p;
}

static char *
get_field_imm (struct string_buf *buf, long instr)
{
  char *p = strbuf (buf);

  sprintf (p, "%d", (short)((instr & IMM_MASK) >> IMM_LOW));
  return p;
}

static char *
get_field_imm5 (struct string_buf *buf, long instr)
{
  char *p = strbuf (buf);

  sprintf (p, "%d", (short)((instr & IMM5_MASK) >> IMM_LOW));
  return p;
}

static char *
get_field_imm5_mbar (struct string_buf *buf, long instr)
{
  char *p = strbuf (buf);

  sprintf (p, "%d", (short)((instr & IMM5_MBAR_MASK) >> IMM_MBAR));
  return p;
}

static char *
get_field_rfsl (struct string_buf *buf, long instr)
{
  char *p = strbuf (buf);

  sprintf (p, "%s%d", fsl_register_prefix,
	   (short)((instr & RFSL_MASK) >> IMM_LOW));
  return p;
}

static char *
get_field_imm15 (struct string_buf *buf, long instr)
{
  char *p = strbuf (buf);

  sprintf (p, "%d", (short)((instr & IMM15_MASK) >> IMM_LOW));
  return p;
}

static char *
get_field_special (struct string_buf *buf, long instr,
		   const struct op_code_struct *op)
{
  char *p = strbuf (buf);
  char *spr;

  switch ((((instr & IMM_MASK) >> IMM_LOW) ^ op->immval_mask))
    {
    case REG_MSR_MASK :
      spr = "msr";
      break;
    case REG_PC_MASK :
      spr = "pc";
      break;
    case REG_EAR_MASK :
      spr = "ear";
      break;
    case REG_ESR_MASK :
      spr = "esr";
      break;
    case REG_FSR_MASK :
      spr = "fsr";
      break;
    case REG_BTR_MASK :
      spr = "btr";
      break;
    case REG_EDR_MASK :
      spr = "edr";
      break;
    case REG_PID_MASK :
      spr = "pid";
      break;
    case REG_ZPR_MASK :
      spr = "zpr";
      break;
    case REG_TLBX_MASK :
      spr = "tlbx";
      break;
    case REG_TLBLO_MASK :
      spr = "tlblo";
      break;
    case REG_TLBHI_MASK :
      spr = "tlbhi";
      break;
    case REG_TLBSX_MASK :
      spr = "tlbsx";
      break;
    case REG_SHR_MASK :
      spr = "shr";
      break;
    case REG_SLR_MASK :
      spr = "slr";
      break;
    default :
      if (((((instr & IMM_MASK) >> IMM_LOW) ^ op->immval_mask) & 0xE000)
	  == REG_PVR_MASK)
	{
	  sprintf (p, "%spvr%d", register_prefix,
		   (unsigned short)(((instr & IMM_MASK) >> IMM_LOW)
				    ^ op->immval_mask) ^ REG_PVR_MASK);
	  return p;
	}
      else
	spr = "pc";
      break;
    }

   sprintf (p, "%s%s", register_prefix, spr);
   return p;
}

static unsigned long
read_insn_microblaze (bfd_vma memaddr,
		      struct disassemble_info *info,
		      const struct op_code_struct **opr)
{
  unsigned char       ibytes[4];
  int                 status;
  const struct op_code_struct *op;
  unsigned long inst;

  status = info->read_memory_func (memaddr, ibytes, 4, info);

  if (status != 0)
    {
      info->memory_error_func (status, memaddr, info);
      return 0;
    }

  if (info->endian == BFD_ENDIAN_BIG)
    inst = (((unsigned) ibytes[0] << 24) | (ibytes[1] << 16)
	    | (ibytes[2] << 8) | ibytes[3]);
  else if (info->endian == BFD_ENDIAN_LITTLE)
    inst = (((unsigned) ibytes[3] << 24) | (ibytes[2] << 16)
	    | (ibytes[1] << 8) | ibytes[0]);
  else
    abort ();

  /* Just a linear search of the table.  */
  for (op = microblaze_opcodes; op->name != 0; op ++)
    if (op->bit_sequence == (inst & op->opcode_mask))
      break;

  *opr = op;
  return inst;
}


int
print_insn_microblaze (bfd_vma memaddr, struct disassemble_info * info)
{
  fprintf_ftype print_func = info->fprintf_func;
  void *stream = info->stream;
  unsigned long inst, prev_inst;
  const struct op_code_struct *op, *pop;
  int immval = 0;
  bool immfound = false;
  static bfd_vma prev_insn_addr = -1;	/* Init the prev insn addr.  */
  static int prev_insn_vma = -1;	/* Init the prev insn vma.  */
  int curr_insn_vma = info->buffer_vma;
  struct string_buf buf;

  buf.which = 0;
  info->bytes_per_chunk = 4;

  inst = read_insn_microblaze (memaddr, info, &op);
  if (inst == 0)
    return -1;

  if (prev_insn_vma == curr_insn_vma)
    {
      if (memaddr-(info->bytes_per_chunk) == prev_insn_addr)
	{
	  prev_inst = read_insn_microblaze (prev_insn_addr, info, &pop);
	  if (prev_inst == 0)
	    return -1;
	  if (pop->instr == imm)
	    {
	      immval = (get_int_field_imm (prev_inst) << 16) & 0xffff0000;
	      immfound = true;
	    }
	  else
	    {
	      immval = 0;
	      immfound = false;
	    }
	}
    }

  /* Make curr insn as prev insn.  */
  prev_insn_addr = memaddr;
  prev_insn_vma = curr_insn_vma;

  if (op->name == NULL)
    print_func (stream, ".short 0x%04x", (unsigned int) inst);
  else
    {
      print_func (stream, "%s", op->name);

      switch (op->inst_type)
	{
	case INST_TYPE_RD_R1_R2:
	  print_func (stream, "\t%s, %s, %s", get_field_rd (&buf, inst),
		      get_field_r1 (&buf, inst), get_field_r2 (&buf, inst));
	  break;
	case INST_TYPE_RD_R1_IMM:
	  print_func (stream, "\t%s, %s, %s", get_field_rd (&buf, inst),
		      get_field_r1 (&buf, inst), get_field_imm (&buf, inst));
	  if (info->print_address_func && get_int_field_r1 (inst) == 0
	      && info->symbol_at_address_func)
	    {
	      if (immfound)
		immval |= (get_int_field_imm (inst) & 0x0000ffff);
	      else
		{
		  immval = get_int_field_imm (inst);
		  if (immval & 0x8000)
		    immval |= 0xFFFF0000;
		}
	      if (immval > 0 && info->symbol_at_address_func (immval, info))
		{
		  print_func (stream, "\t// ");
		  info->print_address_func (immval, info);
		}
	    }
	  break;
	case INST_TYPE_RD_R1_IMM5:
	  print_func (stream, "\t%s, %s, %s", get_field_rd (&buf, inst),
		      get_field_r1 (&buf, inst), get_field_imm5 (&buf, inst));
	  break;
	case INST_TYPE_RD_RFSL:
	  print_func (stream, "\t%s, %s", get_field_rd (&buf, inst),
		      get_field_rfsl (&buf, inst));
	  break;
	case INST_TYPE_R1_RFSL:
	  print_func (stream, "\t%s, %s", get_field_r1 (&buf, inst),
		      get_field_rfsl (&buf, inst));
	  break;
	case INST_TYPE_RD_SPECIAL:
	  print_func (stream, "\t%s, %s", get_field_rd (&buf, inst),
		      get_field_special (&buf, inst, op));
	  break;
	case INST_TYPE_SPECIAL_R1:
	  print_func (stream, "\t%s, %s", get_field_special (&buf, inst, op),
		      get_field_r1 (&buf, inst));
	  break;
	case INST_TYPE_RD_R1:
	  print_func (stream, "\t%s, %s", get_field_rd (&buf, inst),
		      get_field_r1 (&buf, inst));
	  break;
	case INST_TYPE_R1_R2:
	  print_func (stream, "\t%s, %s", get_field_r1 (&buf, inst),
		      get_field_r2 (&buf, inst));
	  break;
	case INST_TYPE_R1_IMM:
	  print_func (stream, "\t%s, %s", get_field_r1 (&buf, inst),
		      get_field_imm (&buf, inst));
	  /* The non-pc relative instructions are returns, which shouldn't
	     have a label printed.  */
	  if (info->print_address_func && op->inst_offset_type == INST_PC_OFFSET
	      && info->symbol_at_address_func)
	    {
	      if (immfound)
		immval |= (get_int_field_imm (inst) & 0x0000ffff);
	      else
		{
		  immval = get_int_field_imm (inst);
		  if (immval & 0x8000)
		    immval |= 0xFFFF0000;
		}
	      immval += memaddr;
	      if (immval > 0 && info->symbol_at_address_func (immval, info))
		{
		  print_func (stream, "\t// ");
		  info->print_address_func (immval, info);
		}
	      else
		{
		  print_func (stream, "\t\t// ");
		  print_func (stream, "%x", immval);
		}
	    }
	  break;
	case INST_TYPE_RD_IMM:
	  print_func (stream, "\t%s, %s", get_field_rd (&buf, inst),
		      get_field_imm (&buf, inst));
	  if (info->print_address_func && info->symbol_at_address_func)
	    {
	      if (immfound)
		immval |= (get_int_field_imm (inst) & 0x0000ffff);
	      else
		{
		  immval = get_int_field_imm (inst);
		  if (immval & 0x8000)
		    immval |= 0xFFFF0000;
		}
	      if (op->inst_offset_type == INST_PC_OFFSET)
		immval += (int) memaddr;
	      if (info->symbol_at_address_func (immval, info))
		{
		  print_func (stream, "\t// ");
		  info->print_address_func (immval, info);
		}
	    }
	  break;
	case INST_TYPE_IMM:
	  print_func (stream, "\t%s", get_field_imm (&buf, inst));
	  if (info->print_address_func && info->symbol_at_address_func
	      && op->instr != imm)
	    {
	      if (immfound)
		immval |= (get_int_field_imm (inst) & 0x0000ffff);
	      else
		{
		  immval = get_int_field_imm (inst);
		  if (immval & 0x8000)
		    immval |= 0xFFFF0000;
		}
	      if (op->inst_offset_type == INST_PC_OFFSET)
		immval += (int) memaddr;
	      if (immval > 0 && info->symbol_at_address_func (immval, info))
		{
		  print_func (stream, "\t// ");
		  info->print_address_func (immval, info);
		}
	      else if (op->inst_offset_type == INST_PC_OFFSET)
		{
		  print_func (stream, "\t\t// ");
		  print_func (stream, "%x", immval);
		}
	    }
	  break;
	case INST_TYPE_RD_R2:
	  print_func (stream, "\t%s, %s", get_field_rd (&buf, inst),
		      get_field_r2 (&buf, inst));
	  break;
	case INST_TYPE_R2:
	  print_func (stream, "\t%s", get_field_r2 (&buf, inst));
	  break;
	case INST_TYPE_R1:
	  print_func (stream, "\t%s", get_field_r1 (&buf, inst));
	  break;
	case INST_TYPE_R1_R2_SPECIAL:
	  print_func (stream, "\t%s, %s", get_field_r1 (&buf, inst),
		      get_field_r2 (&buf, inst));
	  break;
	case INST_TYPE_RD_IMM15:
	  print_func (stream, "\t%s, %s", get_field_rd (&buf, inst),
		      get_field_imm15 (&buf, inst));
	  break;
	  /* For mbar insn.  */
	case INST_TYPE_IMM5:
	  print_func (stream, "\t%s", get_field_imm5_mbar (&buf, inst));
	  break;
	  /* For mbar 16 or sleep insn.  */
	case INST_TYPE_NONE:
	  break;
	  /* For tuqula instruction */
	case INST_TYPE_RD:
	  print_func (stream, "\t%s", get_field_rd (&buf, inst));
	  break;
	case INST_TYPE_RFSL:
	  print_func (stream, "\t%s", get_field_rfsl (&buf, inst));
	  break;
	default:
	  /* If the disassembler lags the instruction set.  */
	  print_func (stream, "\tundecoded operands, inst is 0x%04x",
		      (unsigned int) inst);
	  break;
	}
    }

  /* Say how many bytes we consumed.  */
  return 4;
}

enum microblaze_instr
get_insn_microblaze (long inst,
  		     bool *isunsignedimm,
  		     enum microblaze_instr_type *insn_type,
  		     short *delay_slots)
{
  const struct op_code_struct *op;
  *isunsignedimm = false;

  /* Just a linear search of the table.  */
  for (op = microblaze_opcodes; op->name != 0; op ++)
    if (op->bit_sequence == (inst & op->opcode_mask))
      break;

  if (op->name == 0)
    return invalid_inst;
  else
    {
      *isunsignedimm = (op->inst_type == INST_TYPE_RD_R1_UNSIGNED_IMM);
      *insn_type = op->instr_type;
      *delay_slots = op->delay_slots;
      return op->instr;
    }
}

enum microblaze_instr
microblaze_decode_insn (long insn, int *rd, int *ra, int *rb, int *immed)
{
  enum microblaze_instr op;
  bool t1;
  enum microblaze_instr_type t2;
  short t3;

  op = get_insn_microblaze (insn, &t1, &t2, &t3);
  *rd = (insn & RD_MASK) >> RD_LOW;
  *ra = (insn & RA_MASK) >> RA_LOW;
  *rb = (insn & RB_MASK) >> RB_LOW;
  t3 = (insn & IMM_MASK) >> IMM_LOW;
  *immed = (int) t3;
  return (op);
}

unsigned long
microblaze_get_target_address (long inst, bool immfound, int immval,
			       long pcval, long r1val, long r2val,
			       bool *targetvalid,
			       bool *unconditionalbranch)
{
  const struct op_code_struct *op;
  long targetaddr = 0;

  *unconditionalbranch = false;
  /* Just a linear search of the table.  */
  for (op = microblaze_opcodes; op->name != 0; op ++)
    if (op->bit_sequence == (inst & op->opcode_mask))
      break;

  if (op->name == 0)
    {
      *targetvalid = false;
    }
  else if (op->instr_type == branch_inst)
    {
      switch (op->inst_type)
	{
        case INST_TYPE_R2:
          *unconditionalbranch = true;
        /* Fall through.  */
        case INST_TYPE_RD_R2:
        case INST_TYPE_R1_R2:
          targetaddr = r2val;
          *targetvalid = true;
          if (op->inst_offset_type == INST_PC_OFFSET)
	    targetaddr += pcval;
          break;
        case INST_TYPE_IMM:
          *unconditionalbranch = true;
        /* Fall through.  */
        case INST_TYPE_RD_IMM:
        case INST_TYPE_R1_IMM:
          if (immfound)
	    {
	      targetaddr = (immval << 16) & 0xffff0000;
	      targetaddr |= (get_int_field_imm (inst) & 0x0000ffff);
	    }
	  else
	    {
	      targetaddr = get_int_field_imm (inst);
	      if (targetaddr & 0x8000)
	        targetaddr |= 0xFFFF0000;
            }
          if (op->inst_offset_type == INST_PC_OFFSET)
	    targetaddr += pcval;
          *targetvalid = true;
          break;
	default:
	  *targetvalid = false;
	  break;
        }
    }
  else if (op->instr_type == return_inst)
    {
      if (immfound)
	{
	  targetaddr = (immval << 16) & 0xffff0000;
	  targetaddr |= (get_int_field_imm (inst) & 0x0000ffff);
	}
      else
	{
	  targetaddr = get_int_field_imm (inst);
	  if (targetaddr & 0x8000)
	    targetaddr |= 0xFFFF0000;
	}
      targetaddr += r1val;
      *targetvalid = true;
    }
  else
    *targetvalid = false;
  return targetaddr;
}
