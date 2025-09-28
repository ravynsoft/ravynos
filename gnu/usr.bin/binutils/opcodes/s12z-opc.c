/* s12z-decode.c -- Freescale S12Z disassembly
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

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
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "opcode/s12z.h"

#include "bfd.h"

#include "s12z-opc.h"


typedef int (*insn_bytes_f) (struct mem_read_abstraction_base *);

typedef int (*operands_f) (struct mem_read_abstraction_base *,
			   int *n_operands, struct operand **operand);

typedef enum optr (*discriminator_f) (struct mem_read_abstraction_base *,
				      enum optr hint);

enum OPR_MODE
  {
    OPR_IMMe4,
    OPR_REG,
    OPR_OFXYS,
    OPR_XY_PRE_INC,
    OPR_XY_POST_INC,
    OPR_XY_PRE_DEC,
    OPR_XY_POST_DEC,
    OPR_S_PRE_DEC,
    OPR_S_POST_INC,
    OPR_REG_DIRECT,
    OPR_REG_INDIRECT,
    OPR_IDX_DIRECT,
    OPR_IDX_INDIRECT,
    OPR_EXT1,
    OPR_IDX2_REG,
    OPR_IDX3_DIRECT,
    OPR_IDX3_INDIRECT,

    OPR_EXT18,
    OPR_IDX3_DIRECT_REG,
    OPR_EXT3_DIRECT,
    OPR_EXT3_INDIRECT
  };

struct opr_pb
{
  uint8_t mask;
  uint8_t value;
  int n_operands;
  enum OPR_MODE mode;
};

static const  struct opr_pb opr_pb[] = {
  {0xF0, 0x70, 1, OPR_IMMe4},
  {0xF8, 0xB8, 1, OPR_REG},
  {0xC0, 0x40, 1, OPR_OFXYS},
  {0xEF, 0xE3, 1, OPR_XY_PRE_INC},
  {0xEF, 0xE7, 1, OPR_XY_POST_INC},
  {0xEF, 0xC3, 1, OPR_XY_PRE_DEC},
  {0xEF, 0xC7, 1, OPR_XY_POST_DEC},
  {0xFF, 0xFB, 1, OPR_S_PRE_DEC},
  {0xFF, 0xFF, 1, OPR_S_POST_INC},
  {0xC8, 0x88, 1, OPR_REG_DIRECT},
  {0xE8, 0xC8, 1, OPR_REG_INDIRECT},

  {0xCE, 0xC0, 2, OPR_IDX_DIRECT},
  {0xCE, 0xC4, 2, OPR_IDX_INDIRECT},
  {0xC0, 0x00, 2, OPR_EXT1},

  {0xC8, 0x80, 3, OPR_IDX2_REG},
  {0xFA, 0xF8, 3, OPR_EXT18},

  {0xCF, 0xC2, 4, OPR_IDX3_DIRECT},
  {0xCF, 0xC6, 4, OPR_IDX3_INDIRECT},

  {0xF8, 0xE8, 4, OPR_IDX3_DIRECT_REG},
  {0xFF, 0xFA, 4, OPR_EXT3_DIRECT},
  {0xFF, 0xFE, 4, OPR_EXT3_INDIRECT},
};

/* Return the number of bytes in a OPR operand, including the XB postbyte.
   It does not include any preceeding opcodes. */
static int
x_opr_n_bytes (struct mem_read_abstraction_base *mra, int offset)
{
  bfd_byte xb;
  int status = mra->read (mra, offset, 1, &xb);
  if (status < 0)
    return status;

  size_t i;
  for (i = 0; i < sizeof (opr_pb) / sizeof (opr_pb[0]); ++i)
    {
      const struct opr_pb *pb = opr_pb + i;
      if ((xb & pb->mask) == pb->value)
	{
	  return pb->n_operands;
	}
    }

  return 1;
}

static int
opr_n_bytes_p1 (struct mem_read_abstraction_base *mra)
{
  int n = x_opr_n_bytes (mra, 0);
  if (n < 0)
    return n;
  return 1 + n;
}

static int
opr_n_bytes2 (struct mem_read_abstraction_base *mra)
{
  int s = x_opr_n_bytes (mra, 0);
  if (s < 0)
    return s;
  int n = x_opr_n_bytes (mra, s);
  if (n < 0)
    return n;
  return s + n + 1;
}

enum BB_MODE
  {
    BB_REG_REG_REG,
    BB_REG_REG_IMM,
    BB_REG_OPR_REG,
    BB_OPR_REG_REG,
    BB_REG_OPR_IMM,
    BB_OPR_REG_IMM
  };

struct opr_bb
{
  uint8_t mask;
  uint8_t value;
  int n_operands;
  bool opr;
  enum BB_MODE mode;
};

static const struct opr_bb bb_modes[] =
  {
    {0x60, 0x00, 2, false, BB_REG_REG_REG},
    {0x60, 0x20, 3, false, BB_REG_REG_IMM},
    {0x70, 0x40, 2, true,  BB_REG_OPR_REG},
    {0x70, 0x50, 2, true,  BB_OPR_REG_REG},
    {0x70, 0x60, 3, true,  BB_REG_OPR_IMM},
    {0x70, 0x70, 3, true,  BB_OPR_REG_IMM}
  };

static int
bfextins_n_bytes (struct mem_read_abstraction_base *mra)
{
  bfd_byte bb;
  int status = mra->read (mra, 0, 1, &bb);
  if (status < 0)
    return status;

  size_t i;
  const struct opr_bb *bbs = 0;
  for (i = 0; i < sizeof (bb_modes) / sizeof (bb_modes[0]); ++i)
    {
      bbs = bb_modes + i;
      if ((bb & bbs->mask) == bbs->value)
	{
	  break;
	}
    }

  int n = bbs->n_operands;
  if (bbs->opr)
    {
      int x = x_opr_n_bytes (mra, n - 1);
      if (x < 0)
	return x;
      n += x;
    }

  return n;
}

static int
single (struct mem_read_abstraction_base *mra ATTRIBUTE_UNUSED)
{
  return 1;
}

static int
two (struct mem_read_abstraction_base *mra ATTRIBUTE_UNUSED)
{
  return 2;
}

static int
three (struct mem_read_abstraction_base *mra ATTRIBUTE_UNUSED)
{
  return 3;
}

static int
four (struct mem_read_abstraction_base *mra ATTRIBUTE_UNUSED)
{
  return 4;
}

static int
five (struct mem_read_abstraction_base *mra ATTRIBUTE_UNUSED)
{
  return 5;
}

static int
pcrel_15bit (struct mem_read_abstraction_base *mra)
{
  bfd_byte byte;
  int status = mra->read (mra, 0, 1, &byte);
  if (status < 0)
    return status;
  return (byte & 0x80) ? 3 : 2;
}



static int
xysp_reg_from_postbyte (uint8_t postbyte)
{
  int reg = -1;
  switch ((postbyte & 0x30) >> 4)
    {
    case 0:
      reg = REG_X;
      break;
    case 1:
      reg = REG_Y;
      break;
    case 2:
      reg = REG_S;
      break;
    default:
      reg = REG_P;
    }
  return reg;
}

static struct operand *
create_immediate_operand (int value)
{
  struct immediate_operand *op = malloc (sizeof (*op));

  if (op != NULL)
    {
      op->parent.cl = OPND_CL_IMMEDIATE;
      op->parent.osize = -1;
      op->value = value;
    }
  return (struct operand *) op;
}

static struct operand *
create_bitfield_operand (int width, int offset)
{
  struct bitfield_operand *op = malloc (sizeof (*op));

  if (op != NULL)
    {
      op->parent.cl = OPND_CL_BIT_FIELD;
      op->parent.osize = -1;
      op->width = width;
      op->offset = offset;
    }
  return (struct operand *) op;
}

static struct operand *
create_register_operand_with_size (int reg, short osize)
{
  struct register_operand *op = malloc (sizeof (*op));

  if (op != NULL)
    {
      op->parent.cl = OPND_CL_REGISTER;
      op->parent.osize = osize;
      op->reg = reg;
    }
  return (struct operand *) op;
}

static struct operand *
create_register_operand (int reg)
{
  return create_register_operand_with_size (reg, -1);
}

static struct operand *
create_register_all_operand (void)
{
  struct register_operand *op = malloc (sizeof (*op));

  if (op != NULL)
    {
      op->parent.cl = OPND_CL_REGISTER_ALL;
      op->parent.osize = -1;
    }
  return (struct operand *) op;
}

static struct operand *
create_register_all16_operand (void)
{
  struct register_operand *op = malloc (sizeof (*op));

  if (op != NULL)
    {
      op->parent.cl = OPND_CL_REGISTER_ALL16;
      op->parent.osize = -1;
    }
  return (struct operand *) op;
}


static struct operand *
create_simple_memory_operand (bfd_vma addr, bfd_vma base, bool relative)
{
  struct simple_memory_operand *op;

  assert (relative || base == 0);
  op = malloc (sizeof (*op));
  if (op != NULL)
    {
      op->parent.cl = OPND_CL_SIMPLE_MEMORY;
      op->parent.osize = -1;
      op->addr = addr;
      op->base = base;
      op->relative = relative;
    }
  return (struct operand *) op;
}

static struct operand *
create_memory_operand (bool indirect, int base, int n_regs, int reg0, int reg1)
{
  struct memory_operand *op = malloc (sizeof (*op));

  if (op != NULL)
    {
      op->parent.cl = OPND_CL_MEMORY;
      op->parent.osize = -1;
      op->indirect = indirect;
      op->base_offset = base;
      op->mutation = OPND_RM_NONE;
      op->n_regs = n_regs;
      op->regs[0] = reg0;
      op->regs[1] = reg1;
    }
  return (struct operand *) op;
}

static struct operand *
create_memory_auto_operand (enum op_reg_mutation mutation, int reg)
{
  struct memory_operand *op = malloc (sizeof (*op));

  if (op != NULL)
    {
      op->parent.cl = OPND_CL_MEMORY;
      op->parent.osize = -1;
      op->indirect = false;
      op->base_offset = 0;
      op->mutation = mutation;
      op->n_regs = 1;
      op->regs[0] = reg;
      op->regs[1] = -1;
    }
  return (struct operand *) op;
}



static int
z_ext24_decode (struct mem_read_abstraction_base *mra, int *n_operands,
		struct operand **operand)
{
  struct operand *op;
  uint8_t buffer[3];
  int status = mra->read (mra, 0, 3, buffer);
  if (status < 0)
    return status;

  int i;
  uint32_t addr = 0;
  for (i = 0; i < 3; ++i)
    {
      addr <<= 8;
      addr |= buffer[i];
    }

  op = create_simple_memory_operand (addr, 0, false);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}


static int
z_decode_signed_value (struct mem_read_abstraction_base *mra, int offset,
		       short size, uint32_t *result)
{
  assert (size >0);
  assert (size <= 4);
  bfd_byte buffer[4];
  int status = mra->read (mra, offset, size, buffer);
  if (status < 0)
    return status;

  int i;
  uint32_t value = 0;
  for (i = 0; i < size; ++i)
    value = (value << 8) | buffer[i];

  if (buffer[0] & 0x80)
    {
      /* Deal with negative values */
      value -= 1u << (size * 4) << (size * 4);
    }
  *result = value;
  return 0;
}

static int
decode_signed_value (struct mem_read_abstraction_base *mra, short size,
		     uint32_t *result)
{
  return z_decode_signed_value (mra, 0, size, result);
}

static int
x_imm1 (struct mem_read_abstraction_base *mra,
	int offset,
	int *n_operands, struct operand **operand)
{
  struct operand *op;
  bfd_byte byte;
  int status = mra->read (mra, offset, 1, &byte);
  if (status < 0)
    return status;

  op = create_immediate_operand (byte);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

/* An eight bit immediate operand.  */
static int
imm1_decode (struct mem_read_abstraction_base *mra,
	     int *n_operands, struct operand **operand)
{
  return x_imm1 (mra, 0, n_operands, operand);
}

static int
trap_decode (struct mem_read_abstraction_base *mra,
	     int *n_operands, struct operand **operand)
{
  return x_imm1 (mra, -1, n_operands, operand);
}


static struct operand *
x_opr_decode_with_size (struct mem_read_abstraction_base *mra, int offset,
			short osize)
{
  bfd_byte postbyte;
  int status = mra->read (mra, offset, 1, &postbyte);
  if (status < 0)
    return NULL;
  offset++;

  enum OPR_MODE mode = -1;
  size_t i;
  for (i = 0; i < sizeof (opr_pb) / sizeof (opr_pb[0]); ++i)
    {
      const struct opr_pb *pb = opr_pb + i;
      if ((postbyte & pb->mask) == pb->value)
	{
	  mode = pb->mode;
	  break;
	}
    }

  struct operand *operand = NULL;
  switch (mode)
    {
    case OPR_IMMe4:
      {
	int n;
	uint8_t x = (postbyte & 0x0F);
	if (x == 0)
	  n = -1;
	else
	  n = x;

	operand = create_immediate_operand (n);
	break;
      }
    case OPR_REG:
      {
	uint8_t x = (postbyte & 0x07);
	operand = create_register_operand (x);
	break;
      }
    case OPR_OFXYS:
      {
	operand = create_memory_operand (false, postbyte & 0x0F, 1,
					 xysp_reg_from_postbyte (postbyte), -1);
	break;
      }
    case OPR_REG_DIRECT:
      {
	operand = create_memory_operand (false, 0, 2, postbyte & 0x07,
					 xysp_reg_from_postbyte (postbyte));
	break;
      }
    case OPR_REG_INDIRECT:
      {
	operand = create_memory_operand (true, 0, 2, postbyte & 0x07,
					 (postbyte & 0x10) ? REG_Y : REG_X);
	break;
      }

    case OPR_IDX_INDIRECT:
      {
	uint8_t x1;
	status = mra->read (mra, offset, 1, &x1);
	if (status < 0)
	  return NULL;
	int idx = x1;

	if (postbyte & 0x01)
	  {
	    /* Deal with negative values */
	    idx -= 0x1UL << 8;
	  }

	operand = create_memory_operand (true, idx, 1,
					 xysp_reg_from_postbyte (postbyte), -1);
	break;
      }

    case OPR_IDX3_DIRECT:
      {
	uint8_t x[3];
	status = mra->read (mra, offset, 3, x);
	if (status < 0)
	  return NULL;
	int idx = x[0] << 16 | x[1] << 8 | x[2];

	if (x[0] & 0x80)
	  {
	    /* Deal with negative values */
	    idx -= 0x1UL << 24;
	  }

	operand = create_memory_operand (false, idx, 1,
					 xysp_reg_from_postbyte (postbyte), -1);
	break;
      }

    case OPR_IDX3_DIRECT_REG:
      {
	uint8_t x[3];
	status = mra->read (mra, offset, 3, x);
	if (status < 0)
	  return NULL;
	int idx = x[0] << 16 | x[1] << 8 | x[2];

	if (x[0] & 0x80)
	  {
	    /* Deal with negative values */
	    idx -= 0x1UL << 24;
	  }

	operand = create_memory_operand (false, idx, 1, postbyte & 0x07, -1);
	break;
      }

    case OPR_IDX3_INDIRECT:
      {
	uint8_t x[3];
	status = mra->read (mra, offset, 3, x);
	if (status < 0)
	  return NULL;
	int idx = x[0] << 16 | x[1] << 8 | x[2];

	if (x[0] & 0x80)
	  {
	    /* Deal with negative values */
	    idx -= 0x1UL << 24;
	  }

	operand = create_memory_operand (true, idx, 1,
					 xysp_reg_from_postbyte (postbyte), -1);
	break;
      }

    case OPR_IDX_DIRECT:
      {
	uint8_t x1;
	status = mra->read (mra, offset, 1, &x1);
	if (status < 0)
	  return NULL;
	int idx = x1;

	if (postbyte & 0x01)
	  {
	    /* Deal with negative values */
	    idx -= 0x1UL << 8;
	  }

	operand = create_memory_operand (false, idx, 1,
					 xysp_reg_from_postbyte (postbyte), -1);
	break;
      }

    case OPR_IDX2_REG:
      {
	uint8_t x[2];
	status = mra->read (mra, offset, 2, x);
	if (status < 0)
	  return NULL;
	uint32_t idx = x[1] | x[0] << 8 ;
	idx |= (postbyte & 0x30) << 12;

	operand = create_memory_operand (false, idx, 1, postbyte & 0x07, -1);
	break;
      }

    case OPR_XY_PRE_INC:
      {
	operand = create_memory_auto_operand (OPND_RM_PRE_INC,
					      (postbyte & 0x10) ? REG_Y: REG_X);
	break;
      }
    case OPR_XY_POST_INC:
      {
	operand = create_memory_auto_operand (OPND_RM_POST_INC,
					      (postbyte & 0x10) ? REG_Y: REG_X);
	break;
      }
    case OPR_XY_PRE_DEC:
      {
	operand = create_memory_auto_operand (OPND_RM_PRE_DEC,
					      (postbyte & 0x10) ? REG_Y: REG_X);
	break;
      }
    case OPR_XY_POST_DEC:
      {
	operand = create_memory_auto_operand (OPND_RM_POST_DEC,
					      (postbyte & 0x10) ? REG_Y: REG_X);
	break;
      }
    case OPR_S_PRE_DEC:
      {
	operand = create_memory_auto_operand (OPND_RM_PRE_DEC, REG_S);
	break;
      }
    case OPR_S_POST_INC:
      {
	operand = create_memory_auto_operand (OPND_RM_POST_INC, REG_S);
	break;
      }

    case OPR_EXT18:
      {
	const size_t size = 2;
	bfd_byte buffer[4];
	status = mra->read (mra, offset, size, buffer);
	if (status < 0)
	  return NULL;

	uint32_t ext18 = 0;
	for (i = 0; i < size; ++i)
	  {
	    ext18 <<= 8;
	    ext18 |= buffer[i];
	  }

	ext18 |= (postbyte & 0x01) << 16;
	ext18 |= (postbyte & 0x04) << 15;

	operand = create_simple_memory_operand (ext18, 0, false);
	break;
      }

    case OPR_EXT1:
      {
	uint8_t x1 = 0;
	status = mra->read (mra, offset, 1, &x1);
	if (status < 0)
	  return NULL;
	int16_t addr;
	addr = x1;
	addr |= (postbyte & 0x3f) << 8;

	operand = create_simple_memory_operand (addr, 0, false);
	break;
      }

    case OPR_EXT3_DIRECT:
      {
	const size_t size = 3;
	bfd_byte buffer[4];
	status = mra->read (mra, offset, size, buffer);
	if (status < 0)
	  return NULL;

	uint32_t ext24 = 0;
	for (i = 0; i < size; ++i)
	  {
	    ext24 |= buffer[i] << (8 * (size - i - 1));
	  }

	operand = create_simple_memory_operand (ext24, 0, false);
	break;
      }

    case OPR_EXT3_INDIRECT:
      {
	const size_t size = 3;
	bfd_byte buffer[4];
	status = mra->read (mra, offset, size, buffer);
	if (status < 0)
	  return NULL;

	uint32_t ext24 = 0;
	for (i = 0; i < size; ++i)
	  {
	    ext24 |= buffer[i] << (8 * (size - i - 1));
	  }

	operand = create_memory_operand (true, ext24, 0, -1, -1);
	break;
      }

    default:
      printf ("Unknown OPR mode #0x%x (%d)", postbyte, mode);
      abort ();
    }

  if (operand != NULL)
    operand->osize = osize;

  return operand;
}

static struct operand *
x_opr_decode (struct mem_read_abstraction_base *mra, int offset)
{
  return x_opr_decode_with_size (mra, offset, -1);
}

static int
z_opr_decode (struct mem_read_abstraction_base *mra,
	      int *n_operands, struct operand **operand)
{
  struct operand *op = x_opr_decode (mra, 0);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
z_opr_decode2 (struct mem_read_abstraction_base *mra,
	       int *n_operands, struct operand **operand)
{
  int n = x_opr_n_bytes (mra, 0);
  if (n < 0)
    return n;
  struct operand *op = x_opr_decode (mra, 0);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = x_opr_decode (mra, n);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
imm1234 (struct mem_read_abstraction_base *mra, int base,
	 int *n_operands, struct operand **operand)
{
  struct operand *op;
  bfd_byte opcode;
  int status = mra->read (mra, -1, 1, &opcode);
  if (status < 0)
    return status;

  opcode -= base;

  int size = registers[opcode & 0xF].bytes;

  uint32_t imm;
  if (decode_signed_value (mra, size, &imm) < 0)
    return -1;

  op = create_immediate_operand (imm);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}


/* Special case of LD and CMP with register S and IMM operand */
static int
reg_s_imm (struct mem_read_abstraction_base *mra, int *n_operands,
	   struct operand **operand)
{
  struct operand *op;

  op = create_register_operand (REG_S);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;

  uint32_t imm;
  if (decode_signed_value (mra, 3, &imm) < 0)
    return -1;
  op = create_immediate_operand (imm);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

/* Special case of LD, CMP and ST with register S and OPR operand */
static int
reg_s_opr (struct mem_read_abstraction_base *mra, int *n_operands,
	   struct operand **operand)
{
  struct operand *op;

  op = create_register_operand (REG_S);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = x_opr_decode (mra, 0);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
z_imm1234_8base (struct mem_read_abstraction_base *mra, int *n_operands,
		 struct operand **operand)
{
  return imm1234 (mra, 8, n_operands, operand);
}

static int
z_imm1234_0base (struct mem_read_abstraction_base *mra, int *n_operands,
		 struct operand **operand)
{
  return imm1234 (mra, 0, n_operands, operand);
}


static int
z_tfr (struct mem_read_abstraction_base *mra, int *n_operands,
       struct operand **operand)
{
  struct operand *op;
  bfd_byte byte;
  int status = mra->read (mra, 0, 1, &byte);
  if (status < 0)
    return status;

  op = create_register_operand (byte >> 4);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = create_register_operand (byte & 0x0F);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
z_reg (struct mem_read_abstraction_base *mra, int *n_operands,
       struct operand **operand)
{
  struct operand *op;
  bfd_byte byte;
  int status = mra->read (mra, -1, 1, &byte);
  if (status < 0)
    return status;

  op = create_register_operand (byte & 0x07);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}


static int
reg_xy (struct mem_read_abstraction_base *mra,
	int *n_operands, struct operand **operand)
{
  struct operand *op;
  bfd_byte byte;
  int status = mra->read (mra, -1, 1, &byte);
  if (status < 0)
    return status;

  op = create_register_operand ((byte & 0x01) ? REG_Y : REG_X);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
lea_reg_xys_opr (struct mem_read_abstraction_base *mra,
		 int *n_operands, struct operand **operand)
{
  struct operand *op;
  bfd_byte byte;
  int status = mra->read (mra, -1, 1, &byte);
  if (status < 0)
    return status;

  int reg_xys = -1;
  switch (byte & 0x03)
    {
    case 0x00:
      reg_xys = REG_X;
      break;
    case 0x01:
      reg_xys = REG_Y;
      break;
    case 0x02:
      reg_xys = REG_S;
      break;
    }

  op = create_register_operand (reg_xys);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = x_opr_decode (mra, 0);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
lea_reg_xys (struct mem_read_abstraction_base *mra,
	     int *n_operands, struct operand **operand)
{
  struct operand *op;
  bfd_byte byte;
  int status = mra->read (mra, -1, 1, &byte);
  if (status < 0)
    return status;

  int reg_n = -1;
  switch (byte & 0x03)
    {
    case 0x00:
      reg_n = REG_X;
      break;
    case 0x01:
      reg_n = REG_Y;
      break;
    case 0x02:
      reg_n = REG_S;
      break;
    }

  status = mra->read (mra, 0, 1, &byte);
  if (status < 0)
    return status;

  op = create_register_operand (reg_n);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = create_memory_operand (false, (int8_t) byte, 1, reg_n, -1);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}


/* PC Relative offsets of size 15 or 7 bits */
static int
rel_15_7 (struct mem_read_abstraction_base *mra, int offset,
	  int *n_operands, struct operand **operands)
{
  struct operand *op;
  bfd_byte upper;
  int status = mra->read (mra, offset - 1, 1, &upper);
  if (status < 0)
    return status;

  bool rel_size = (upper & 0x80);

  int16_t addr = upper;
  if (rel_size)
    {
      /* 15 bits.  Get the next byte */
      bfd_byte lower;
      status = mra->read (mra, offset, 1, &lower);
      if (status < 0)
	return status;

      addr <<= 8;
      addr |= lower;
      addr &= 0x7FFF;

      bool negative = (addr & 0x4000);
      addr &= 0x3FFF;
      if (negative)
	addr = addr - 0x4000;
    }
  else
    {
      /* 7 bits. */
      bool negative = (addr & 0x40);
      addr &= 0x3F;
      if (negative)
	addr = addr - 0x40;
    }

  op = create_simple_memory_operand (addr, mra->posn (mra) - 1, true);
  if (op == NULL)
    return -1;
  operands[(*n_operands)++] = op;
  return 0;
}


/* PC Relative offsets of size 15 or 7 bits */
static int
decode_rel_15_7 (struct mem_read_abstraction_base *mra,
		 int *n_operands, struct operand **operand)
{
  return rel_15_7 (mra, 1, n_operands, operand);
}

static int shift_n_bytes (struct mem_read_abstraction_base *);
static int mov_imm_opr_n_bytes (struct mem_read_abstraction_base *);
static int loop_prim_n_bytes (struct mem_read_abstraction_base *);
static int bm_rel_n_bytes (struct mem_read_abstraction_base *);
static int mul_n_bytes (struct mem_read_abstraction_base *);
static int bm_n_bytes (struct mem_read_abstraction_base *);

static int psh_pul_decode (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operand);
static int shift_decode (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operand);
static int mul_decode (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operand);
static int bm_decode (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operand);
static int bm_rel_decode (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operand);
static int mov_imm_opr (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operand);
static int loop_primitive_decode (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operands);
static int bit_field_decode (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operands);
static int exg_sex_decode (struct mem_read_abstraction_base *mra, int *n_operands, struct operand **operands);


static enum optr shift_discrim (struct mem_read_abstraction_base *mra, enum optr hint);
static enum optr psh_pul_discrim (struct mem_read_abstraction_base *mra, enum optr hint);
static enum optr mul_discrim (struct mem_read_abstraction_base *mra, enum optr hint);
static enum optr loop_primitive_discrim (struct mem_read_abstraction_base *mra, enum optr hint);
static enum optr bit_field_discrim (struct mem_read_abstraction_base *mra, enum optr hint);
static enum optr exg_sex_discrim (struct mem_read_abstraction_base *mra, enum optr hint);


static int
cmp_xy (struct mem_read_abstraction_base *mra ATTRIBUTE_UNUSED,
	int *n_operands, struct operand **operand)
{
  struct operand *op;

  op = create_register_operand (REG_X);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = create_register_operand (REG_Y);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
sub_d6_x_y (struct mem_read_abstraction_base *mra ATTRIBUTE_UNUSED,
	    int *n_operands, struct operand **operand)
{
  struct operand *op;

  op = create_register_operand (REG_D6);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = create_register_operand (REG_X);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = create_register_operand (REG_Y);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
sub_d6_y_x (struct mem_read_abstraction_base *mra ATTRIBUTE_UNUSED,
	    int *n_operands, struct operand **operand)
{
  struct operand *op;

  op = create_register_operand (REG_D6);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = create_register_operand (REG_Y);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = create_register_operand (REG_X);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}

static int
ld_18bit_decode (struct mem_read_abstraction_base *mra, int *n_operands,
		 struct operand **operand);

static enum optr
mul_discrim (struct mem_read_abstraction_base *mra, enum optr hint)
{
  uint8_t mb;
  int status = mra->read (mra, 0, 1, &mb);
  if (status < 0)
    return OP_INVALID;

  bool signed_op = (mb & 0x80);

  switch (hint)
    {
    case OPBASE_mul:
      return signed_op ? OP_muls : OP_mulu;
      break;
    case OPBASE_div:
      return signed_op ? OP_divs : OP_divu;
      break;
    case OPBASE_mod:
      return signed_op ? OP_mods : OP_modu;
      break;
    case OPBASE_mac:
      return signed_op ? OP_macs : OP_macu;
      break;
    case OPBASE_qmul:
      return signed_op ? OP_qmuls : OP_qmulu;
      break;
    default:
      abort ();
    }

  return OP_INVALID;
}

struct opcode
{
  /* The operation that this opcode performs.  */
  enum optr operator;

  /* The size of this operation.  May be -1 if it is implied
     in the operands or if size is not applicable.  */
  short osize;

  /* Some operations need this function to work out which operation
   is intended.  */
  discriminator_f discriminator;

  /* A function returning the number of bytes in this instruction.  */
  insn_bytes_f insn_bytes;

  operands_f operands;
  operands_f operands2;
};

static const struct opcode page2[] =
  {
    [0x00] = {OP_ld, -1, 0,  opr_n_bytes_p1, reg_s_opr, 0},
    [0x01] = {OP_st, -1, 0,  opr_n_bytes_p1, reg_s_opr, 0},
    [0x02] = {OP_cmp, -1, 0, opr_n_bytes_p1, reg_s_opr, 0},
    [0x03] = {OP_ld, -1, 0,  four, reg_s_imm, 0},
    [0x04] = {OP_cmp, -1, 0, four, reg_s_imm, 0},
    [0x05] = {OP_stop, -1, 0, single, 0, 0},
    [0x06] = {OP_wai, -1, 0,  single, 0, 0},
    [0x07] = {OP_sys, -1, 0,  single, 0, 0},
    [0x08] = {0xFFFF, -1, bit_field_discrim,  bfextins_n_bytes, bit_field_decode, 0},  /* BFEXT / BFINS */
    [0x09] = {0xFFFF, -1, bit_field_discrim,  bfextins_n_bytes, bit_field_decode, 0},
    [0x0a] = {0xFFFF, -1, bit_field_discrim,  bfextins_n_bytes, bit_field_decode, 0},
    [0x0b] = {0xFFFF, -1, bit_field_discrim,  bfextins_n_bytes, bit_field_decode, 0},
    [0x0c] = {0xFFFF, -1, bit_field_discrim,  bfextins_n_bytes, bit_field_decode, 0},
    [0x0d] = {0xFFFF, -1, bit_field_discrim,  bfextins_n_bytes, bit_field_decode, 0},
    [0x0e] = {0xFFFF, -1, bit_field_discrim,  bfextins_n_bytes, bit_field_decode, 0},
    [0x0f] = {0xFFFF, -1, bit_field_discrim,  bfextins_n_bytes, bit_field_decode, 0},
    [0x10] = {OP_minu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x11] = {OP_minu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x12] = {OP_minu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x13] = {OP_minu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x14] = {OP_minu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x15] = {OP_minu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x16] = {OP_minu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x17] = {OP_minu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x18] = {OP_maxu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x19] = {OP_maxu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x1a] = {OP_maxu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x1b] = {OP_maxu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x1c] = {OP_maxu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x1d] = {OP_maxu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x1e] = {OP_maxu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x1f] = {OP_maxu, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x20] = {OP_mins, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x21] = {OP_mins, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x22] = {OP_mins, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x23] = {OP_mins, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x24] = {OP_mins, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x25] = {OP_mins, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x26] = {OP_mins, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x27] = {OP_mins, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x28] = {OP_maxs, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x29] = {OP_maxs, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x2a] = {OP_maxs, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x2b] = {OP_maxs, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x2c] = {OP_maxs, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x2d] = {OP_maxs, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x2e] = {OP_maxs, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x2f] = {OP_maxs, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x30] = {OPBASE_div, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x31] = {OPBASE_div, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x32] = {OPBASE_div, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x33] = {OPBASE_div, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x34] = {OPBASE_div, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x35] = {OPBASE_div, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x36] = {OPBASE_div, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x37] = {OPBASE_div, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x38] = {OPBASE_mod, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x39] = {OPBASE_mod, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x3a] = {OPBASE_mod, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x3b] = {OPBASE_mod, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x3c] = {OPBASE_mod, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x3d] = {OPBASE_mod, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x3e] = {OPBASE_mod, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x3f] = {OPBASE_mod, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x40] = {OP_abs, -1, 0, single, z_reg, 0},
    [0x41] = {OP_abs, -1, 0, single, z_reg, 0},
    [0x42] = {OP_abs, -1, 0, single, z_reg, 0},
    [0x43] = {OP_abs, -1, 0, single, z_reg, 0},
    [0x44] = {OP_abs, -1, 0, single, z_reg, 0},
    [0x45] = {OP_abs, -1, 0, single, z_reg, 0},
    [0x46] = {OP_abs, -1, 0, single, z_reg, 0},
    [0x47] = {OP_abs, -1, 0, single, z_reg, 0},
    [0x48] = {OPBASE_mac, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x49] = {OPBASE_mac, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4a] = {OPBASE_mac, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4b] = {OPBASE_mac, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4c] = {OPBASE_mac, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4d] = {OPBASE_mac, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4e] = {OPBASE_mac, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4f] = {OPBASE_mac, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x50] = {OP_adc, -1, 0, three, z_reg, z_imm1234_0base},
    [0x51] = {OP_adc, -1, 0, three, z_reg, z_imm1234_0base},
    [0x52] = {OP_adc, -1, 0, three, z_reg, z_imm1234_0base},
    [0x53] = {OP_adc, -1, 0, three, z_reg, z_imm1234_0base},
    [0x54] = {OP_adc, -1, 0, two,   z_reg, z_imm1234_0base},
    [0x55] = {OP_adc, -1, 0, two,   z_reg, z_imm1234_0base},
    [0x56] = {OP_adc, -1, 0, five,  z_reg, z_imm1234_0base},
    [0x57] = {OP_adc, -1, 0, five,  z_reg, z_imm1234_0base},
    [0x58] = {OP_bit, -1, 0, three, z_reg, z_imm1234_8base},
    [0x59] = {OP_bit, -1, 0, three, z_reg, z_imm1234_8base},
    [0x5a] = {OP_bit, -1, 0, three, z_reg, z_imm1234_8base},
    [0x5b] = {OP_bit, -1, 0, three, z_reg, z_imm1234_8base},
    [0x5c] = {OP_bit, -1, 0, two,   z_reg, z_imm1234_8base},
    [0x5d] = {OP_bit, -1, 0, two,   z_reg, z_imm1234_8base},
    [0x5e] = {OP_bit, -1, 0, five,  z_reg, z_imm1234_8base},
    [0x5f] = {OP_bit, -1, 0, five,  z_reg, z_imm1234_8base},
    [0x60] = {OP_adc, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x61] = {OP_adc, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x62] = {OP_adc, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x63] = {OP_adc, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x64] = {OP_adc, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x65] = {OP_adc, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x66] = {OP_adc, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x67] = {OP_adc, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x68] = {OP_bit, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x69] = {OP_bit, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6a] = {OP_bit, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6b] = {OP_bit, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6c] = {OP_bit, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6d] = {OP_bit, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6e] = {OP_bit, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6f] = {OP_bit, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x70] = {OP_sbc, -1, 0, three, z_reg, z_imm1234_0base},
    [0x71] = {OP_sbc, -1, 0, three, z_reg, z_imm1234_0base},
    [0x72] = {OP_sbc, -1, 0, three, z_reg, z_imm1234_0base},
    [0x73] = {OP_sbc, -1, 0, three, z_reg, z_imm1234_0base},
    [0x74] = {OP_sbc, -1, 0, two,   z_reg, z_imm1234_0base},
    [0x75] = {OP_sbc, -1, 0, two,   z_reg, z_imm1234_0base},
    [0x76] = {OP_sbc, -1, 0, five,  z_reg, z_imm1234_0base},
    [0x77] = {OP_sbc, -1, 0, five,  z_reg, z_imm1234_0base},
    [0x78] = {OP_eor, -1, 0, three, z_reg, z_imm1234_8base},
    [0x79] = {OP_eor, -1, 0, three, z_reg, z_imm1234_8base},
    [0x7a] = {OP_eor, -1, 0, three, z_reg, z_imm1234_8base},
    [0x7b] = {OP_eor, -1, 0, three, z_reg, z_imm1234_8base},
    [0x7c] = {OP_eor, -1, 0, two,   z_reg, z_imm1234_8base},
    [0x7d] = {OP_eor, -1, 0, two,   z_reg, z_imm1234_8base},
    [0x7e] = {OP_eor, -1, 0, five,  z_reg, z_imm1234_8base},
    [0x7f] = {OP_eor, -1, 0, five,  z_reg, z_imm1234_8base},
    [0x80] = {OP_sbc, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x81] = {OP_sbc, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x82] = {OP_sbc, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x83] = {OP_sbc, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x84] = {OP_sbc, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x85] = {OP_sbc, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x86] = {OP_sbc, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x87] = {OP_sbc, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x88] = {OP_eor, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x89] = {OP_eor, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x8a] = {OP_eor, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x8b] = {OP_eor, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x8c] = {OP_eor, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x8d] = {OP_eor, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x8e] = {OP_eor, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x8f] = {OP_eor, -1, 0,  opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x90] = {OP_rti, -1, 0,  single, 0, 0},
    [0x91] = {OP_clb, -1, 0,   two, z_tfr, 0},
    [0x92] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x93] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x94] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x95] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x96] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x97] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x98] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x99] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x9a] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x9b] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x9c] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x9d] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x9e] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0x9f] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xa0] = {OP_sat, -1, 0, single, z_reg, 0},
    [0xa1] = {OP_sat, -1, 0, single, z_reg, 0},
    [0xa2] = {OP_sat, -1, 0, single, z_reg, 0},
    [0xa3] = {OP_sat, -1, 0, single, z_reg, 0},
    [0xa4] = {OP_sat, -1, 0, single, z_reg, 0},
    [0xa5] = {OP_sat, -1, 0, single, z_reg, 0},
    [0xa6] = {OP_sat, -1, 0, single, z_reg, 0},
    [0xa7] = {OP_sat, -1, 0, single, z_reg, 0},
    [0xa8] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xa9] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xaa] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xab] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xac] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xad] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xae] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xaf] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xb0] = {OPBASE_qmul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0xb1] = {OPBASE_qmul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0xb2] = {OPBASE_qmul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0xb3] = {OPBASE_qmul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0xb4] = {OPBASE_qmul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0xb5] = {OPBASE_qmul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0xb6] = {OPBASE_qmul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0xb7] = {OPBASE_qmul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0xb8] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xb9] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xba] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xbb] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xbc] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xbd] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xbe] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xbf] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc0] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc1] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc2] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc3] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc4] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc5] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc6] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc7] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc8] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xc9] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xca] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xcb] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xcc] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xcd] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xce] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xcf] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd0] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd1] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd2] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd3] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd4] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd5] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd6] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd7] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd8] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xd9] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xda] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xdb] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xdc] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xdd] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xde] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xdf] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe0] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe1] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe2] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe3] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe4] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe5] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe6] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe7] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe8] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xe9] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xea] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xeb] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xec] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xed] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xee] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xef] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf0] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf1] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf2] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf3] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf4] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf5] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf6] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf7] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf8] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xf9] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xfa] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xfb] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xfc] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xfd] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xfe] = {OP_trap, -1, 0,  single, trap_decode, 0},
    [0xff] = {OP_trap, -1, 0,  single, trap_decode, 0},
  };

static const struct opcode page1[] =
  {
    [0x00] = {OP_bgnd, -1, 0, single, 0, 0},
    [0x01] = {OP_nop, -1, 0,  single, 0, 0},
    [0x02] = {OP_brclr, -1, 0, bm_rel_n_bytes, bm_rel_decode, 0},
    [0x03] = {OP_brset, -1, 0, bm_rel_n_bytes, bm_rel_decode, 0},
    [0x04] = {0xFFFF, -1, psh_pul_discrim,   two, psh_pul_decode, 0}, /* psh/pul */
    [0x05] = {OP_rts, -1, 0,  single, 0, 0},
    [0x06] = {OP_lea, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x07] = {OP_lea, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x08] = {OP_lea, -1, 0, opr_n_bytes_p1, lea_reg_xys_opr, 0},
    [0x09] = {OP_lea, -1, 0, opr_n_bytes_p1, lea_reg_xys_opr, 0},
    [0x0a] = {OP_lea, -1, 0, opr_n_bytes_p1, lea_reg_xys_opr, 0},
    [0x0b] = {0xFFFF, -1, loop_primitive_discrim, loop_prim_n_bytes, loop_primitive_decode, 0}, /* Loop primitives TBcc / DBcc */
    [0x0c] = {OP_mov, 0, 0, mov_imm_opr_n_bytes, mov_imm_opr, 0},
    [0x0d] = {OP_mov, 1, 0, mov_imm_opr_n_bytes, mov_imm_opr, 0},
    [0x0e] = {OP_mov, 2, 0, mov_imm_opr_n_bytes, mov_imm_opr, 0},
    [0x0f] = {OP_mov, 3, 0, mov_imm_opr_n_bytes, mov_imm_opr, 0},
    [0x10] = {0xFFFF, -1, shift_discrim,  shift_n_bytes, shift_decode, 0},  /* lsr/lsl/asl/asr/rol/ror */
    [0x11] = {0xFFFF, -1, shift_discrim,  shift_n_bytes, shift_decode, 0},
    [0x12] = {0xFFFF, -1, shift_discrim,  shift_n_bytes, shift_decode, 0},
    [0x13] = {0xFFFF, -1, shift_discrim,  shift_n_bytes, shift_decode, 0},
    [0x14] = {0xFFFF, -1, shift_discrim,  shift_n_bytes, shift_decode, 0},
    [0x15] = {0xFFFF, -1, shift_discrim,  shift_n_bytes, shift_decode, 0},
    [0x16] = {0xFFFF, -1, shift_discrim,  shift_n_bytes, shift_decode, 0},
    [0x17] = {0xFFFF, -1, shift_discrim,  shift_n_bytes, shift_decode, 0},
    [0x18] = {OP_lea, -1, 0,  two, lea_reg_xys, NULL},
    [0x19] = {OP_lea, -1, 0,  two, lea_reg_xys, NULL},
    [0x1a] = {OP_lea, -1, 0,  two, lea_reg_xys, NULL},
    /* 0x1b PG2 */
    [0x1c] = {OP_mov, 0, 0, opr_n_bytes2, z_opr_decode2, 0},
    [0x1d] = {OP_mov, 1, 0, opr_n_bytes2, z_opr_decode2, 0},
    [0x1e] = {OP_mov, 2, 0, opr_n_bytes2, z_opr_decode2, 0},
    [0x1f] = {OP_mov, 3, 0, opr_n_bytes2, z_opr_decode2, 0},
    [0x20] = {OP_bra, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x21] = {OP_bsr, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x22] = {OP_bhi, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x23] = {OP_bls, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x24] = {OP_bcc, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x25] = {OP_bcs, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x26] = {OP_bne, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x27] = {OP_beq, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x28] = {OP_bvc, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x29] = {OP_bvs, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x2a] = {OP_bpl, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x2b] = {OP_bmi, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x2c] = {OP_bge, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x2d] = {OP_blt, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x2e] = {OP_bgt, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x2f] = {OP_ble, -1, 0,  pcrel_15bit, decode_rel_15_7, 0},
    [0x30] = {OP_inc, -1, 0, single, z_reg, 0},
    [0x31] = {OP_inc, -1, 0, single, z_reg, 0},
    [0x32] = {OP_inc, -1, 0, single, z_reg, 0},
    [0x33] = {OP_inc, -1, 0, single, z_reg, 0},
    [0x34] = {OP_inc, -1, 0, single, z_reg, 0},
    [0x35] = {OP_inc, -1, 0, single, z_reg, 0},
    [0x36] = {OP_inc, -1, 0, single, z_reg, 0},
    [0x37] = {OP_inc, -1, 0, single, z_reg, 0},
    [0x38] = {OP_clr, -1, 0, single, z_reg, 0},
    [0x39] = {OP_clr, -1, 0, single, z_reg, 0},
    [0x3a] = {OP_clr, -1, 0, single, z_reg, 0},
    [0x3b] = {OP_clr, -1, 0, single, z_reg, 0},
    [0x3c] = {OP_clr, -1, 0, single, z_reg, 0},
    [0x3d] = {OP_clr, -1, 0, single, z_reg, 0},
    [0x3e] = {OP_clr, -1, 0, single, z_reg, 0},
    [0x3f] = {OP_clr, -1, 0, single, z_reg, 0},
    [0x40] = {OP_dec, -1, 0, single, z_reg, 0},
    [0x41] = {OP_dec, -1, 0, single, z_reg, 0},
    [0x42] = {OP_dec, -1, 0, single, z_reg, 0},
    [0x43] = {OP_dec, -1, 0, single, z_reg, 0},
    [0x44] = {OP_dec, -1, 0, single, z_reg, 0},
    [0x45] = {OP_dec, -1, 0, single, z_reg, 0},
    [0x46] = {OP_dec, -1, 0, single, z_reg, 0},
    [0x47] = {OP_dec, -1, 0, single, z_reg, 0},
    [0x48] = {OPBASE_mul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x49] = {OPBASE_mul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4a] = {OPBASE_mul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4b] = {OPBASE_mul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4c] = {OPBASE_mul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4d] = {OPBASE_mul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4e] = {OPBASE_mul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x4f] = {OPBASE_mul, -1, mul_discrim, mul_n_bytes, mul_decode, 0},
    [0x50] = {OP_add, -1, 0, three, z_reg, z_imm1234_0base},
    [0x51] = {OP_add, -1, 0, three, z_reg, z_imm1234_0base},
    [0x52] = {OP_add, -1, 0, three, z_reg, z_imm1234_0base},
    [0x53] = {OP_add, -1, 0, three, z_reg, z_imm1234_0base},
    [0x54] = {OP_add, -1, 0, two,   z_reg, z_imm1234_0base},
    [0x55] = {OP_add, -1, 0, two,   z_reg, z_imm1234_0base},
    [0x56] = {OP_add, -1, 0, five,  z_reg, z_imm1234_0base},
    [0x57] = {OP_add, -1, 0, five,  z_reg, z_imm1234_0base},
    [0x58] = {OP_and, -1, 0, three, z_reg, z_imm1234_8base},
    [0x59] = {OP_and, -1, 0, three, z_reg, z_imm1234_8base},
    [0x5a] = {OP_and, -1, 0, three, z_reg, z_imm1234_8base},
    [0x5b] = {OP_and, -1, 0, three, z_reg, z_imm1234_8base},
    [0x5c] = {OP_and, -1, 0, two,   z_reg, z_imm1234_8base},
    [0x5d] = {OP_and, -1, 0, two,   z_reg, z_imm1234_8base},
    [0x5e] = {OP_and, -1, 0, five,  z_reg, z_imm1234_8base},
    [0x5f] = {OP_and, -1, 0, five,  z_reg, z_imm1234_8base},
    [0x60] = {OP_add, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x61] = {OP_add, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x62] = {OP_add, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x63] = {OP_add, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x64] = {OP_add, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x65] = {OP_add, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x66] = {OP_add, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x67] = {OP_add, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x68] = {OP_and, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x69] = {OP_and, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6a] = {OP_and, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6b] = {OP_and, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6c] = {OP_and, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6d] = {OP_and, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6e] = {OP_and, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x6f] = {OP_and, -1, 0, opr_n_bytes_p1, z_reg, z_opr_decode},
    [0x70] = {OP_sub, -1, 0, three, z_reg, z_imm1234_0base},
    [0x71] = {OP_sub, -1, 0, three, z_reg, z_imm1234_0base},
    [0x72] = {OP_sub, -1, 0, three, z_reg, z_imm1234_0base},
    [0x73] = {OP_sub, -1, 0, three, z_reg, z_imm1234_0base},
    [0x74] = {OP_sub, -1, 0, two,   z_reg, z_imm1234_0base},
    [0x75] = {OP_sub, -1, 0, two,   z_reg, z_imm1234_0base},
    [0x76] = {OP_sub, -1, 0, five,  z_reg, z_imm1234_0base},
    [0x77] = {OP_sub, -1, 0, five,  z_reg, z_imm1234_0base},
    [0x78] = {OP_or, -1, 0, three, z_reg, z_imm1234_8base},
    [0x79] = {OP_or, -1, 0, three, z_reg, z_imm1234_8base},
    [0x7a] = {OP_or, -1, 0, three, z_reg, z_imm1234_8base},
    [0x7b] = {OP_or, -1, 0, three, z_reg, z_imm1234_8base},
    [0x7c] = {OP_or, -1, 0, two,   z_reg, z_imm1234_8base},
    [0x7d] = {OP_or, -1, 0, two,   z_reg, z_imm1234_8base},
    [0x7e] = {OP_or, -1, 0, five,  z_reg, z_imm1234_8base},
    [0x7f] = {OP_or, -1, 0, five,  z_reg, z_imm1234_8base},
    [0x80] = {OP_sub, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x81] = {OP_sub, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x82] = {OP_sub, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x83] = {OP_sub, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x84] = {OP_sub, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x85] = {OP_sub, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x86] = {OP_sub, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x87] = {OP_sub, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x88] = {OP_or, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x89] = {OP_or, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x8a] = {OP_or, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x8b] = {OP_or, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x8c] = {OP_or, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x8d] = {OP_or, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x8e] = {OP_or, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x8f] = {OP_or, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0x90] = {OP_ld, -1, 0, three,  z_reg, z_imm1234_0base},
    [0x91] = {OP_ld, -1, 0, three,  z_reg, z_imm1234_0base},
    [0x92] = {OP_ld, -1, 0, three,  z_reg, z_imm1234_0base},
    [0x93] = {OP_ld, -1, 0, three,  z_reg, z_imm1234_0base},
    [0x94] = {OP_ld, -1, 0, two,    z_reg, z_imm1234_0base},
    [0x95] = {OP_ld, -1, 0, two,    z_reg, z_imm1234_0base},
    [0x96] = {OP_ld, -1, 0, five,   z_reg, z_imm1234_0base},
    [0x97] = {OP_ld, -1, 0, five,   z_reg, z_imm1234_0base},
    [0x98] = {OP_ld, -1, 0, four,   reg_xy, z_imm1234_0base},
    [0x99] = {OP_ld, -1, 0, four,   reg_xy, z_imm1234_0base},
    [0x9a] = {OP_clr, -1, 0, single, reg_xy, 0},
    [0x9b] = {OP_clr, -1, 0, single, reg_xy, 0},
    [0x9c] = {OP_inc, 0, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0x9d] = {OP_inc, 1, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0x9e] = {OP_tfr, -1, 0, two, z_tfr, NULL},
    [0x9f] = {OP_inc, 3, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xa0] = {OP_ld, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xa1] = {OP_ld, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xa2] = {OP_ld, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xa3] = {OP_ld, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xa4] = {OP_ld, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xa5] = {OP_ld, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xa6] = {OP_ld, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xa7] = {OP_ld, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xa8] = {OP_ld, -1, 0, opr_n_bytes_p1, reg_xy, z_opr_decode},
    [0xa9] = {OP_ld, -1, 0, opr_n_bytes_p1, reg_xy, z_opr_decode},
    [0xaa] = {OP_jmp, -1, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xab] = {OP_jsr, -1, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xac] = {OP_dec, 0, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xad] = {OP_dec, 1, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xae] = {0xFFFF, -1, exg_sex_discrim,   two, exg_sex_decode, 0},  /* EXG / SEX */
    [0xaf] = {OP_dec, 3, 0, opr_n_bytes_p1, 0, z_opr_decode},
    [0xb0] = {OP_ld, -1, 0, four,  z_reg, z_ext24_decode},
    [0xb1] = {OP_ld, -1, 0, four,  z_reg, z_ext24_decode},
    [0xb2] = {OP_ld, -1, 0, four,  z_reg, z_ext24_decode},
    [0xb3] = {OP_ld, -1, 0, four,  z_reg, z_ext24_decode},
    [0xb4] = {OP_ld, -1, 0, four,  z_reg, z_ext24_decode},
    [0xb5] = {OP_ld, -1, 0, four,  z_reg, z_ext24_decode},
    [0xb6] = {OP_ld, -1, 0, four,  z_reg, z_ext24_decode},
    [0xb7] = {OP_ld, -1, 0, four,  z_reg, z_ext24_decode},
    [0xb8] = {OP_ld, -1, 0, four,  reg_xy, z_ext24_decode},
    [0xb9] = {OP_ld, -1, 0, four,  reg_xy, z_ext24_decode},
    [0xba] = {OP_jmp, -1, 0, four, z_ext24_decode, 0},
    [0xbb] = {OP_jsr, -1, 0, four, z_ext24_decode, 0},
    [0xbc] = {OP_clr, 0, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xbd] = {OP_clr, 1, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xbe] = {OP_clr, 2, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xbf] = {OP_clr, 3, 0, opr_n_bytes_p1, z_opr_decode, 0},
    [0xc0] = {OP_st, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xc1] = {OP_st, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xc2] = {OP_st, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xc3] = {OP_st, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xc4] = {OP_st, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xc5] = {OP_st, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xc6] = {OP_st, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xc7] = {OP_st, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xc8] = {OP_st, -1, 0, opr_n_bytes_p1, reg_xy, z_opr_decode},
    [0xc9] = {OP_st, -1, 0, opr_n_bytes_p1, reg_xy, z_opr_decode},
    [0xca] = {OP_ld, -1, 0, three, reg_xy, ld_18bit_decode},
    [0xcb] = {OP_ld, -1, 0, three, reg_xy, ld_18bit_decode},
    [0xcc] = {OP_com, 0, 0, opr_n_bytes_p1, NULL, z_opr_decode},
    [0xcd] = {OP_com, 1, 0, opr_n_bytes_p1, NULL, z_opr_decode},
    [0xce] = {OP_andcc, -1, 0, two, imm1_decode, 0},
    [0xcf] = {OP_com, 3, 0, opr_n_bytes_p1, NULL, z_opr_decode},
    [0xd0] = {OP_st, -1, 0, four,  z_reg, z_ext24_decode},
    [0xd1] = {OP_st, -1, 0, four,  z_reg, z_ext24_decode},
    [0xd2] = {OP_st, -1, 0, four,  z_reg, z_ext24_decode},
    [0xd3] = {OP_st, -1, 0, four,  z_reg, z_ext24_decode},
    [0xd4] = {OP_st, -1, 0, four,  z_reg, z_ext24_decode},
    [0xd5] = {OP_st, -1, 0, four,  z_reg, z_ext24_decode},
    [0xd6] = {OP_st, -1, 0, four,  z_reg, z_ext24_decode},
    [0xd7] = {OP_st, -1, 0, four,  z_reg, z_ext24_decode},
    [0xd8] = {OP_st, -1, 0, four,  reg_xy, z_ext24_decode},
    [0xd9] = {OP_st, -1, 0, four,  reg_xy, z_ext24_decode},
    [0xda] = {OP_ld, -1, 0, three, reg_xy, ld_18bit_decode},
    [0xdb] = {OP_ld, -1, 0, three, reg_xy, ld_18bit_decode},
    [0xdc] = {OP_neg, 0, 0, opr_n_bytes_p1, NULL, z_opr_decode},
    [0xdd] = {OP_neg, 1, 0, opr_n_bytes_p1, NULL, z_opr_decode},
    [0xde] = {OP_orcc, -1, 0,  two,  imm1_decode, 0},
    [0xdf] = {OP_neg,  3, 0, opr_n_bytes_p1, NULL, z_opr_decode},
    [0xe0] = {OP_cmp, -1, 0, three,  z_reg, z_imm1234_0base},
    [0xe1] = {OP_cmp, -1, 0, three,  z_reg, z_imm1234_0base},
    [0xe2] = {OP_cmp, -1, 0, three,  z_reg, z_imm1234_0base},
    [0xe3] = {OP_cmp, -1, 0, three,  z_reg, z_imm1234_0base},
    [0xe4] = {OP_cmp, -1, 0, two,    z_reg, z_imm1234_0base},
    [0xe5] = {OP_cmp, -1, 0, two,    z_reg, z_imm1234_0base},
    [0xe6] = {OP_cmp, -1, 0, five,   z_reg, z_imm1234_0base},
    [0xe7] = {OP_cmp, -1, 0, five,   z_reg, z_imm1234_0base},
    [0xe8] = {OP_cmp, -1, 0, four,   reg_xy, z_imm1234_0base},
    [0xe9] = {OP_cmp, -1, 0, four,   reg_xy, z_imm1234_0base},
    [0xea] = {OP_ld, -1, 0, three, reg_xy, ld_18bit_decode},
    [0xeb] = {OP_ld, -1, 0, three, reg_xy, ld_18bit_decode},
    [0xec] = {OP_bclr, -1, 0, bm_n_bytes, bm_decode, 0},
    [0xed] = {OP_bset, -1, 0, bm_n_bytes, bm_decode, 0},
    [0xee] = {OP_btgl, -1, 0, bm_n_bytes, bm_decode, 0},
    [0xef] = {OP_INVALID, -1, 0, NULL, NULL, NULL}, /* SPARE */
    [0xf0] = {OP_cmp, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xf1] = {OP_cmp, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xf2] = {OP_cmp, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xf3] = {OP_cmp, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xf4] = {OP_cmp, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xf5] = {OP_cmp, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xf6] = {OP_cmp, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xf7] = {OP_cmp, -1, 0, opr_n_bytes_p1, z_reg,    z_opr_decode},
    [0xf8] = {OP_cmp, -1, 0, opr_n_bytes_p1, reg_xy, z_opr_decode},
    [0xf9] = {OP_cmp, -1, 0, opr_n_bytes_p1, reg_xy, z_opr_decode},
    [0xfa] = {OP_ld, -1, 0,  three, reg_xy, ld_18bit_decode},
    [0xfb] = {OP_ld, -1, 0,  three, reg_xy, ld_18bit_decode},
    [0xfc] = {OP_cmp, -1, 0, single, cmp_xy, 0},
    [0xfd] = {OP_sub, -1, 0, single, sub_d6_x_y, 0},
    [0xfe] = {OP_sub, -1, 0, single, sub_d6_y_x, 0},
    [0xff] = {OP_swi, -1, 0, single, 0, 0}
  };

static const int oprregs1[] =
  {
    REG_D3, REG_D2, REG_D1, REG_D0, REG_CCL, REG_CCH
  };

static const int oprregs2[] =
  {
    REG_Y,  REG_X,  REG_D7, REG_D6, REG_D5,  REG_D4
  };




enum MUL_MODE
  {
    MUL_REG_REG,
    MUL_REG_OPR,
    MUL_REG_IMM,
    MUL_OPR_OPR
  };

struct mb
{
  uint8_t mask;
  uint8_t value;
  enum MUL_MODE mode;
};

static const struct mb mul_table[] = {
  {0x40, 0x00, MUL_REG_REG},

  {0x47, 0x40, MUL_REG_OPR},
  {0x47, 0x41, MUL_REG_OPR},
  {0x47, 0x43, MUL_REG_OPR},

  {0x47, 0x44, MUL_REG_IMM},
  {0x47, 0x45, MUL_REG_IMM},
  {0x47, 0x47, MUL_REG_IMM},

  {0x43, 0x42, MUL_OPR_OPR},
};


static int
mul_decode (struct mem_read_abstraction_base *mra,
	    int *n_operands, struct operand **operand)
{
  uint8_t mb;
  struct operand *op;
  int status = mra->read (mra, 0, 1, &mb);
  if (status < 0)
    return status;

  uint8_t byte;
  status = mra->read (mra, -1, 1, &byte);
  if (status < 0)
    return status;

  enum MUL_MODE mode = -1;
  size_t i;
  for (i = 0; i < sizeof (mul_table) / sizeof (mul_table[0]); ++i)
    {
      const struct mb *mm = mul_table + i;
      if ((mb & mm->mask) == mm->value)
	{
	  mode = mm->mode;
	  break;
	}
    }
  op = create_register_operand (byte & 0x07);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;

  switch (mode)
    {
    case MUL_REG_IMM:
      {
	int size = (mb & 0x3);
	op = create_register_operand_with_size ((mb & 0x38) >> 3, size);
	if (op == NULL)
	  return -1;
	operand[(*n_operands)++] = op;

	uint32_t imm;
	if (z_decode_signed_value (mra, 1, size + 1, &imm) < 0)
	  return -1;
	op = create_immediate_operand (imm);
	if (op == NULL)
	  return -1;
	operand[(*n_operands)++] = op;
      }
      break;
    case MUL_REG_REG:
      op = create_register_operand ((mb & 0x38) >> 3);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      op = create_register_operand (mb & 0x07);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case MUL_REG_OPR:
      op = create_register_operand ((mb & 0x38) >> 3);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      op = x_opr_decode_with_size (mra, 1, mb & 0x3);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case MUL_OPR_OPR:
      {
	int first = x_opr_n_bytes (mra, 1);
	if (first < 0)
	  return first;
	op = x_opr_decode_with_size (mra, 1, (mb & 0x30) >> 4);
	if (op == NULL)
	  return -1;
	operand[(*n_operands)++] = op;
	op = x_opr_decode_with_size (mra, first + 1, (mb & 0x0c) >> 2);
	if (op == NULL)
	  return -1;
	operand[(*n_operands)++] = op;
	break;
      }
    }
  return 0;
}


static int
mul_n_bytes (struct mem_read_abstraction_base *mra)
{
  int nx = 2;
  int first, second;
  uint8_t mb;
  int status = mra->read (mra, 0, 1, &mb);
  if (status < 0)
    return status;

  enum MUL_MODE mode = -1;
  size_t i;
  for (i = 0; i < sizeof (mul_table) / sizeof (mul_table[0]); ++i)
    {
      const struct mb *mm = mul_table + i;
      if ((mb & mm->mask) == mm->value)
	{
	  mode = mm->mode;
	  break;
	}
    }

  int size = (mb & 0x3) + 1;

  switch (mode)
    {
    case MUL_REG_IMM:
      nx += size;
      break;
    case MUL_REG_REG:
      break;
    case MUL_REG_OPR:
      first = x_opr_n_bytes (mra, 1);
      if (first < 0)
	return first;
      nx += first;
      break;
    case MUL_OPR_OPR:
      first = x_opr_n_bytes (mra, nx - 1);
      if (first < 0)
	return first;
      nx += first;
      second = x_opr_n_bytes (mra, nx - 1);
      if (second < 0)
	return second;
      nx += second;
      break;
    }

  return nx;
}


/* The NXP documentation is vague about BM_RESERVED0 and BM_RESERVED1,
   and contains obvious typos.
   However the Freescale tools and experiments with the chip itself
   seem to indicate that they behave like BM_REG_IMM and BM_OPR_REG
   respectively.  */

enum BM_MODE
{
  BM_REG_IMM,
  BM_RESERVED0,
  BM_OPR_B,
  BM_OPR_W,
  BM_OPR_L,
  BM_OPR_REG,
  BM_RESERVED1
};

struct bm
{
  uint8_t mask;
  uint8_t value;
  enum BM_MODE mode;
};

static const  struct bm bm_table[] = {
  { 0xC6, 0x04,     BM_REG_IMM},
  { 0x84, 0x00,     BM_REG_IMM},
  { 0x06, 0x06,     BM_REG_IMM},
  { 0xC6, 0x44,     BM_RESERVED0},

  { 0x8F, 0x80,     BM_OPR_B},
  { 0x8E, 0x82,     BM_OPR_W},
  { 0x8C, 0x88,     BM_OPR_L},

  { 0x83, 0x81,     BM_OPR_REG},
  { 0x87, 0x84,     BM_RESERVED1},
};

static int
bm_decode (struct mem_read_abstraction_base *mra,
	   int *n_operands, struct operand **operand)
{
  struct operand *op;
  uint8_t bm;
  int status = mra->read (mra, 0, 1, &bm);
  if (status < 0)
    return status;

  size_t i;
  enum BM_MODE mode = -1;
  for (i = 0; i < sizeof (bm_table) / sizeof (bm_table[0]); ++i)
    {
      const struct bm *bme = bm_table + i;
      if ((bm & bme->mask) == bme->value)
	{
	  mode = bme->mode;
	  break;
	}
    }

  switch (mode)
    {
    case BM_REG_IMM:
    case BM_RESERVED0:
      op = create_register_operand (bm & 0x07);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_OPR_B:
      op = x_opr_decode_with_size (mra, 1, 0);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_OPR_W:
      op = x_opr_decode_with_size (mra, 1, 1);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_OPR_L:
      op = x_opr_decode_with_size (mra, 1, 3);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_OPR_REG:
    case BM_RESERVED1:
      {
	uint8_t xb;
	status = mra->read (mra, 1, 1, &xb);
	if (status < 0)
	  return status;
	/* Don't emit a size suffix for register operands */
	if ((xb & 0xF8) != 0xB8)
	  op = x_opr_decode_with_size (mra, 1, (bm & 0x0c) >> 2);
	else
	  op = x_opr_decode (mra, 1);
	if (op == NULL)
	  return -1;
	operand[(*n_operands)++] = op;
      }
      break;
    }

  uint8_t imm = 0;
  switch (mode)
    {
    case BM_REG_IMM:
    case BM_RESERVED0:
      imm = (bm & 0x38) >> 3;
      op = create_immediate_operand (imm);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_OPR_L:
      imm |= (bm & 0x03) << 3;
      /* fallthrough */
    case BM_OPR_W:
      imm |= (bm & 0x01) << 3;
      /* fallthrough */
    case BM_OPR_B:
      imm |= (bm & 0x70) >> 4;
      op = create_immediate_operand (imm);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_OPR_REG:
    case BM_RESERVED1:
      op = create_register_operand ((bm & 0x70) >> 4);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    }
  return 0;
}


static int
bm_rel_decode (struct mem_read_abstraction_base *mra,
	       int *n_operands, struct operand **operand)
{
  struct operand *op;
  uint8_t bm;
  int status = mra->read (mra, 0, 1, &bm);
  if (status < 0)
    return status;

  size_t i;
  enum BM_MODE mode = -1;
  for (i = 0; i < sizeof (bm_table) / sizeof (bm_table[0]); ++i)
    {
      const struct bm *bme = bm_table + i;
      if ((bm & bme->mask) == bme->value)
	{
	  mode = bme->mode;
	  break;
	}
    }

  int n = 1;
  switch (mode)
    {
    case BM_REG_IMM:
    case BM_RESERVED0:
      op = create_register_operand (bm & 0x07);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_OPR_B:
      op = x_opr_decode_with_size (mra, 1, 0);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      n = x_opr_n_bytes (mra, 1);
      if (n < 0)
	return n;
      n += 1;
      break;
    case BM_OPR_W:
      op = x_opr_decode_with_size (mra, 1, 1);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      n = x_opr_n_bytes (mra, 1);
      if (n < 0)
	return n;
      n += 1;
      break;
    case BM_OPR_L:
      op = x_opr_decode_with_size (mra, 1, 3);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      n = x_opr_n_bytes (mra, 1);
      if (n < 0)
	return n;
      n += 1;
      break;
    case BM_OPR_REG:
    case BM_RESERVED1:
      {
	uint8_t xb;
	status = mra->read (mra, +1, 1, &xb);
	if (status < 0)
	  return status;
	/* Don't emit a size suffix for register operands */
	if ((xb & 0xF8) != 0xB8)
	  {
	    short os = (bm & 0x0c) >> 2;
	    op = x_opr_decode_with_size (mra, 1, os);
	  }
	else
	  op = x_opr_decode (mra, 1);
	if (op == NULL)
	  return -1;
	operand[(*n_operands)++] = op;
      }
      break;
    }

  int x, imm = 0;
  switch (mode)
    {
    case BM_OPR_L:
      imm |= (bm & 0x02) << 3;
      /* fall through */
    case BM_OPR_W:
      imm |= (bm & 0x01) << 3;
      /* fall through */
    case BM_OPR_B:
      imm |= (bm & 0x70) >> 4;
      op = create_immediate_operand (imm);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_RESERVED0:
      imm = (bm & 0x38) >> 3;
      op = create_immediate_operand (imm);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_REG_IMM:
      imm = (bm & 0xF8) >> 3;
      op = create_immediate_operand (imm);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      break;
    case BM_OPR_REG:
    case BM_RESERVED1:
      op = create_register_operand ((bm & 0x70) >> 4);
      if (op == NULL)
	return -1;
      operand[(*n_operands)++] = op;
      x = x_opr_n_bytes (mra, 1);
      if (x < 0)
	return x;
      n += x;
      break;
    }

  return rel_15_7 (mra, n + 1, n_operands, operand);
}

static int
bm_n_bytes (struct mem_read_abstraction_base *mra)
{
  uint8_t bm;
  int status = mra->read (mra, 0, 1, &bm);
  if (status < 0)
    return status;

  size_t i;
  enum BM_MODE mode = -1;
  for (i = 0; i < sizeof (bm_table) / sizeof (bm_table[0]); ++i)
    {
      const struct bm *bme = bm_table + i;
      if ((bm & bme->mask) == bme->value)
	{
	  mode = bme->mode;
	  break;
	}
    }

  int n = 0;
  switch (mode)
    {
    case BM_REG_IMM:
    case BM_RESERVED0:
      break;

    case BM_OPR_B:
    case BM_OPR_W:
    case BM_OPR_L:
    case BM_OPR_REG:
    case BM_RESERVED1:
      n = x_opr_n_bytes (mra, 1);
      if (n < 0)
	return n;
      break;
    }

  return n + 2;
}

static int
bm_rel_n_bytes (struct mem_read_abstraction_base *mra)
{
  int n = 1 + bm_n_bytes (mra);

  bfd_byte rb;
  int status = mra->read (mra, n - 2, 1, &rb);
  if (status != 0)
    return status;

  if (rb & 0x80)
    n++;

  return n;
}





/* shift direction */
enum SB_DIR
  {
    SB_LEFT,
    SB_RIGHT
  };

enum SB_TYPE
  {
    SB_ARITHMETIC,
    SB_LOGICAL
  };


enum SB_MODE
  {
    SB_REG_REG_N_EFF,
    SB_REG_REG_N,
    SB_REG_OPR_EFF,
    SB_ROT,
    SB_REG_OPR_OPR,
    SB_OPR_N
  };

struct sb
{
  uint8_t mask;
  uint8_t value;
  enum SB_MODE mode;
};

static const  struct sb sb_table[] = {
  {0x30, 0x00,     SB_REG_REG_N_EFF},
  {0x30, 0x10,     SB_REG_REG_N},
  {0x34, 0x20,     SB_REG_OPR_EFF},
  {0x34, 0x24,     SB_ROT},
  {0x34, 0x30,     SB_REG_OPR_OPR},
  {0x34, 0x34,     SB_OPR_N},
};

static int
shift_n_bytes (struct mem_read_abstraction_base *mra)
{
  bfd_byte sb;
  int opr1, opr2;
  int status = mra->read (mra, 0, 1, &sb);
  if (status != 0)
    return status;

  size_t i;
  enum SB_MODE mode = -1;
  for (i = 0; i < sizeof (sb_table) / sizeof (sb_table[0]); ++i)
    {
      const struct sb *sbe = sb_table + i;
      if ((sb & sbe->mask) == sbe->value)
	mode = sbe->mode;
    }

  switch (mode)
    {
    case SB_REG_REG_N_EFF:
      return 2;
    case SB_REG_OPR_EFF:
    case SB_ROT:
      opr1 = x_opr_n_bytes (mra, 1);
      if (opr1 < 0)
	return opr1;
      return 2 + opr1;
    case SB_REG_OPR_OPR:
      opr1 = x_opr_n_bytes (mra, 1);
      if (opr1 < 0)
	return opr1;
      opr2 = 0;
      if ((sb & 0x30) != 0x20)
	{
	  opr2 = x_opr_n_bytes (mra, opr1 + 1);
	  if (opr2 < 0)
	    return opr2;
	}
      return 2 + opr1 + opr2;
    default:
      return 3;
    }

  /* not reached */
  return -1;
}


static int
mov_imm_opr_n_bytes (struct mem_read_abstraction_base *mra)
{
  bfd_byte byte;
  int status = mra->read (mra, -1, 1, &byte);
  if (status < 0)
    return status;

  int size = byte - 0x0c + 1;
  int n = x_opr_n_bytes (mra, size);
  if (n < 0)
    return n;

  return size + n + 1;
}

static int
mov_imm_opr (struct mem_read_abstraction_base *mra,
	     int *n_operands, struct operand **operand)
{
  struct operand *op;
  bfd_byte byte;
  int status = mra->read (mra, -1, 1, &byte);
  if (status < 0)
    return status;

  int size = byte - 0x0c + 1;
  uint32_t imm;
  if (decode_signed_value (mra, size, &imm))
    return -1;

  op = create_immediate_operand (imm);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  op = x_opr_decode (mra, size);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}



static int
ld_18bit_decode (struct mem_read_abstraction_base *mra,
		 int *n_operands, struct operand **operand)
{
  struct operand *op;
  size_t size = 3;
  bfd_byte buffer[3];
  int status = mra->read (mra, 0, 2, buffer + 1);
  if (status < 0)
    return status;

  status = mra->read (mra, -1, 1, buffer);
  if (status < 0)
    return status;

  buffer[0] = (buffer[0] & 0x30) >> 4;

  size_t i;
  uint32_t imm = 0;
  for (i = 0; i < size; ++i)
    {
      imm |= buffer[i] << (8 * (size - i - 1));
    }

  op = create_immediate_operand (imm);
  if (op == NULL)
    return -1;
  operand[(*n_operands)++] = op;
  return 0;
}



/* Loop Primitives */

enum LP_MODE {
  LP_REG,
  LP_XY,
  LP_OPR
};

struct lp
{
  uint8_t mask;
  uint8_t value;
  enum LP_MODE mode;
};

static const struct lp lp_mode[] = {
  {0x08, 0x00, LP_REG},
  {0x0C, 0x08, LP_XY},
  {0x0C, 0x0C, LP_OPR},
};


static int
loop_prim_n_bytes (struct mem_read_abstraction_base *mra)
{
  int mx = 0;
  uint8_t lb;
  int status = mra->read (mra, mx++, 1, &lb);
  if (status < 0)
    return status;

  enum LP_MODE mode = -1;
  size_t i;
  for (i = 0; i < sizeof (lp_mode) / sizeof (lp_mode[0]); ++i)
    {
      const struct lp *pb = lp_mode + i;
      if ((lb & pb->mask) == pb->value)
	{
	  mode = pb->mode;
	  break;
	}
    }

  if (mode == LP_OPR)
    {
      int n = x_opr_n_bytes (mra, mx);
      if (n < 0)
	return n;
      mx += n;
    }

  uint8_t rb;
  status = mra->read (mra, mx++, 1, &rb);
  if (status < 0)
    return status;
  if (rb & 0x80)
    mx++;

  return mx + 1;
}




static enum optr
exg_sex_discrim (struct mem_read_abstraction_base *mra,
		 enum optr hint ATTRIBUTE_UNUSED)
{
  uint8_t eb;
  int status = mra->read (mra, 0, 1, &eb);
  enum optr operator = OP_INVALID;
  if (status < 0)
    return operator;

  struct operand *op0 = create_register_operand ((eb & 0xf0) >> 4);
  if (op0 == NULL)
    return -1;
  struct operand *op1 = create_register_operand (eb & 0xf);
  if (op1 == NULL)
    return -1;

  int reg0 = ((struct register_operand *) op0)->reg;
  int reg1 = ((struct register_operand *) op1)->reg;
  if (reg0 >= 0 && reg0 < S12Z_N_REGISTERS
      && reg1 >= 0 && reg1 < S12Z_N_REGISTERS)
    {
      const struct reg *r0 = registers + reg0;
      const struct reg *r1 = registers + reg1;

      operator = r0->bytes < r1->bytes ? OP_sex : OP_exg;
    }

  free (op0);
  free (op1);

  return operator;
}


static int
exg_sex_decode (struct mem_read_abstraction_base *mra,
		int *n_operands, struct operand **operands)
{
  struct operand *op;
  uint8_t eb;
  int status = mra->read (mra, 0, 1, &eb);
  if (status < 0)
    return status;

  /* Ship out the operands.  */
  op = create_register_operand ((eb & 0xf0) >> 4);
  if (op == NULL)
    return -1;
  operands[(*n_operands)++] = op;
  op = create_register_operand (eb & 0xf);
  if (op == NULL)
    return -1;
  operands[(*n_operands)++] = op;
  return 0;
}

static enum optr
loop_primitive_discrim (struct mem_read_abstraction_base *mra,
			enum optr hint ATTRIBUTE_UNUSED)
{
  uint8_t lb;
  int status = mra->read (mra, 0, 1, &lb);
  if (status < 0)
    return OP_INVALID;

  enum optr opbase = (lb & 0x80) ? OP_dbNE : OP_tbNE;
  return opbase + ((lb & 0x70) >> 4);
}

static int
loop_primitive_decode (struct mem_read_abstraction_base *mra,
		       int *n_operands, struct operand **operands)
{
  struct operand *op;
  int n, offs = 1;
  uint8_t lb;
  int status = mra->read (mra, 0, 1, &lb);
  if (status < 0)
    return status;

  enum LP_MODE mode = -1;
  size_t i;
  for (i = 0; i < sizeof (lp_mode) / sizeof (lp_mode[0]); ++i)
    {
      const struct lp *pb = lp_mode + i;
      if ((lb & pb->mask) == pb->value)
	{
	  mode = pb->mode;
	  break;
	}
    }

  switch (mode)
    {
    case LP_REG:
      op = create_register_operand (lb & 0x07);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    case LP_XY:
      op = create_register_operand ((lb & 0x01) + REG_X);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    case LP_OPR:
      n = x_opr_n_bytes (mra, 1);
      if (n < 0)
	return n;
      offs += n;
      op = x_opr_decode_with_size (mra, 1, lb & 0x03);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    }

  return rel_15_7 (mra, offs + 1, n_operands, operands);
}


static enum optr
shift_discrim (struct mem_read_abstraction_base *mra,
	       enum optr hint ATTRIBUTE_UNUSED)
{
  size_t i;
  uint8_t sb;
  int status = mra->read (mra, 0, 1, &sb);
  if (status < 0)
    return OP_INVALID;

  enum SB_DIR  dir = (sb & 0x40) ? SB_LEFT : SB_RIGHT;
  enum SB_TYPE type = (sb & 0x80) ? SB_ARITHMETIC : SB_LOGICAL;
  enum SB_MODE mode = -1;
  for (i = 0; i < sizeof (sb_table) / sizeof (sb_table[0]); ++i)
    {
      const struct sb *sbe = sb_table + i;
      if ((sb & sbe->mask) == sbe->value)
	mode = sbe->mode;
    }

  if (mode == SB_ROT)
    return (dir == SB_LEFT) ? OP_rol : OP_ror;

  if (type == SB_LOGICAL)
    return (dir == SB_LEFT) ? OP_lsl : OP_lsr;

  return (dir == SB_LEFT) ? OP_asl : OP_asr;
}


static int
shift_decode (struct mem_read_abstraction_base *mra, int *n_operands,
	      struct operand **operands)
{
  struct operand *op;
  size_t i;
  uint8_t byte;
  int status = mra->read (mra, -1, 1, &byte);
  if (status < 0)
    return status;

  uint8_t sb;
  status = mra->read (mra, 0, 1, &sb);
  if (status < 0)
    return status;

  enum SB_MODE mode = -1;
  for (i = 0; i < sizeof (sb_table) / sizeof (sb_table[0]); ++i)
    {
      const struct sb *sbe = sb_table + i;
      if ((sb & sbe->mask) == sbe->value)
	mode = sbe->mode;
    }

  short osize = -1;
  switch (mode)
    {
    case SB_REG_OPR_EFF:
    case SB_ROT:
    case SB_REG_OPR_OPR:
      osize = sb & 0x03;
      break;
    case SB_OPR_N:
      {
	uint8_t xb;
	status = mra->read (mra, 1, 1, &xb);
	if (status < 0)
	  return status;
	/* The size suffix is not printed if the OPR operand refers
	   directly to a register, because the size is implied by the
	   size of that register. */
	if ((xb & 0xF8) != 0xB8)
	  osize = sb & 0x03;
      }
      break;
    default:
      break;
    };

  /* Destination register */
  switch (mode)
    {
    case SB_REG_REG_N_EFF:
    case SB_REG_REG_N:
      op = create_register_operand (byte & 0x07);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    case SB_REG_OPR_EFF:
    case SB_REG_OPR_OPR:
      op = create_register_operand (byte & 0x07);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;

    case SB_ROT:
      op = x_opr_decode_with_size (mra, 1, osize);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;

    default:
      break;
    }

  /* Source register */
  switch (mode)
    {
    case SB_REG_REG_N_EFF:
    case SB_REG_REG_N:
      op = create_register_operand_with_size (sb & 0x07, osize);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;

    case SB_REG_OPR_OPR:
      op = x_opr_decode_with_size (mra, 1, osize);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;

    default:
      break;
    }

  /* 3rd arg */
  switch (mode)
    {
    case SB_REG_OPR_EFF:
    case SB_OPR_N:
      op = x_opr_decode_with_size (mra, 1, osize);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;

    case SB_REG_REG_N:
      {
	uint8_t xb;
	status = mra->read (mra, 1, 1, &xb);
	if (status < 0)
	  return status;

	/* This case is slightly unusual.
	   If XB matches the binary pattern 0111XXXX, then instead of
	   interpreting this as a general OPR postbyte in the IMMe4 mode,
	   the XB byte is interpreted in s special way.  */
	if ((xb & 0xF0) == 0x70)
	  {
	    if (byte & 0x10)
	      {
		int shift = ((sb & 0x08) >> 3) | ((xb & 0x0f) << 1);
		op = create_immediate_operand (shift);
		if (op == NULL)
		  return -1;
		operands[(*n_operands)++] = op;
	      }
	    else
	      {
		/* This should not happen.  */
		abort ();
	      }
	  }
	else
	  {
	    op = x_opr_decode (mra, 1);
	    if (op == NULL)
	      return -1;
	    operands[(*n_operands)++] = op;
	  }
      }
      break;
    case SB_REG_OPR_OPR:
      {
	uint8_t xb;
	int n = x_opr_n_bytes (mra, 1);
	if (n < 0)
	  return n;
	status = mra->read (mra, 1 + n, 1, &xb);
	if (status < 0)
	  return status;

	if ((xb & 0xF0) == 0x70)
	  {
	    int imm = xb & 0x0F;
	    imm <<= 1;
	    imm |= (sb & 0x08) >> 3;
	    op = create_immediate_operand (imm);
	    if (op == NULL)
	      return -1;
	    operands[(*n_operands)++] = op;
	  }
	else
	  {
	    op = x_opr_decode (mra, 1 + n);
	    if (op == NULL)
	      return -1;
	    operands[(*n_operands)++] = op;
	  }
      }
      break;
    default:
      break;
    }

  switch (mode)
    {
    case SB_REG_REG_N_EFF:
    case SB_REG_OPR_EFF:
    case SB_OPR_N:
      {
	int imm = (sb & 0x08) ? 2 : 1;
	op = create_immediate_operand (imm);
	if (op == NULL)
	  return -1;
	operands[(*n_operands)++] = op;
      }
      break;

    default:
      break;
    }
  return 0;
}

static enum optr
psh_pul_discrim (struct mem_read_abstraction_base *mra,
		 enum optr hint ATTRIBUTE_UNUSED)
{
  uint8_t byte;
  int status = mra->read (mra, 0, 1, &byte);
  if (status != 0)
    return OP_INVALID;

  return (byte & 0x80) ? OP_pull: OP_push;
}


static int
psh_pul_decode (struct mem_read_abstraction_base *mra,
		int *n_operands, struct operand **operand)
{
  struct operand *op;
  uint8_t byte;
  int status = mra->read (mra, 0, 1, &byte);
  if (status != 0)
    return status;
  int bit;
  if (byte & 0x40)
    {
      if ((byte & 0x3F) == 0)
	{
	  op = create_register_all16_operand ();
	  if (op == NULL)
	    return -1;
	  operand[(*n_operands)++] = op;
	}
      else
	for (bit = 5; bit >= 0; --bit)
	  {
	    if (byte & (0x1 << bit))
	      {
		op = create_register_operand (oprregs2[bit]);
		if (op == NULL)
		  return -1;
		operand[(*n_operands)++] = op;
	      }
	  }
    }
  else
    {
      if ((byte & 0x3F) == 0)
	{
	  op = create_register_all_operand ();
	  if (op == NULL)
	    return -1;
	  operand[(*n_operands)++] = op;
	}
      else
	for (bit = 5; bit >= 0; --bit)
	  {
	    if (byte & (0x1 << bit))
	      {
		op = create_register_operand (oprregs1[bit]);
		if (op == NULL)
		  return -1;
		operand[(*n_operands)++] = op;
	      }
	  }
    }
  return 0;
}

static enum optr
bit_field_discrim (struct mem_read_abstraction_base *mra,
		   enum optr hint ATTRIBUTE_UNUSED)
{
  int status;
  bfd_byte bb;
  status = mra->read (mra, 0, 1, &bb);
  if (status != 0)
    return OP_INVALID;

  return (bb & 0x80) ? OP_bfins : OP_bfext;
}

static int
bit_field_decode (struct mem_read_abstraction_base *mra,
		  int *n_operands, struct operand **operands)
{
  struct operand *op;
  int status;

  bfd_byte byte2;
  status = mra->read (mra, -1, 1, &byte2);
  if (status != 0)
    return status;

  bfd_byte bb;
  status = mra->read (mra, 0, 1, &bb);
  if (status != 0)
    return status;

  enum BB_MODE mode = -1;
  size_t i;
  const struct opr_bb *bbs = 0;
  for (i = 0; i < sizeof (bb_modes) / sizeof (bb_modes[0]); ++i)
    {
      bbs = bb_modes + i;
      if ((bb & bbs->mask) == bbs->value)
	{
	  mode = bbs->mode;
	  break;
	}
    }
  int reg1 = byte2 & 0x07;
  /* First operand */
  switch (mode)
    {
    case BB_REG_REG_REG:
    case BB_REG_REG_IMM:
    case BB_REG_OPR_REG:
    case BB_REG_OPR_IMM:
      op = create_register_operand (reg1);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    case BB_OPR_REG_REG:
      op = x_opr_decode_with_size (mra, 1, (bb >> 2) & 0x03);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    case BB_OPR_REG_IMM:
      op = x_opr_decode_with_size (mra, 2, (bb >> 2) & 0x03);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    }

  /* Second operand */
  switch (mode)
    {
    case BB_REG_REG_REG:
    case BB_REG_REG_IMM:
      {
	int reg_src = (bb >> 2) & 0x07;
	op = create_register_operand (reg_src);
	if (op == NULL)
	  return -1;
	operands[(*n_operands)++] = op;
      }
      break;
    case BB_OPR_REG_REG:
    case BB_OPR_REG_IMM:
      {
	int reg_src = (byte2 & 0x07);
	op = create_register_operand (reg_src);
	if (op == NULL)
	  return -1;
	operands[(*n_operands)++] = op;
      }
      break;
    case BB_REG_OPR_REG:
      op = x_opr_decode_with_size (mra, 1, (bb >> 2) & 0x03);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    case BB_REG_OPR_IMM:
      op = x_opr_decode_with_size (mra, 2, (bb >> 2) & 0x03);
      if (op == NULL)
	return -1;
      operands[(*n_operands)++] = op;
      break;
    }

  /* Third operand */
  switch (mode)
    {
    case BB_REG_REG_REG:
    case BB_OPR_REG_REG:
    case BB_REG_OPR_REG:
      {
	int reg_parm = bb & 0x03;
	op = create_register_operand (reg_parm);
	if (op == NULL)
	  return -1;
	operands[(*n_operands)++] = op;
      }
      break;
    case BB_REG_REG_IMM:
    case BB_OPR_REG_IMM:
    case BB_REG_OPR_IMM:
      {
	bfd_byte i1;
	status = mra->read (mra, 1, 1, &i1);
	if (status < 0)
	  return status;
	int offset = i1 & 0x1f;
	int width = bb & 0x03;
	width <<= 3;
	width |= i1 >> 5;
	op = create_bitfield_operand (width, offset);
	if (op == NULL)
	  return -1;
	operands[(*n_operands)++] = op;
      }
      break;
    }
  return 0;
}


/* Decode the next instruction at MRA, according to OPC.
   The operation to be performed is returned.
   The number of operands, will be placed in N_OPERANDS.
   The operands themselved into OPERANDS.  */
static enum optr
decode_operation (const struct opcode *opc,
		  struct mem_read_abstraction_base *mra,
		  int *n_operands, struct operand **operands)
{
  enum optr op = opc->operator;
  if (opc->discriminator)
    {
      op = opc->discriminator (mra, opc->operator);
      if (op == OP_INVALID)
	return op;
    }

  if (opc->operands)
    if (opc->operands (mra, n_operands, operands) < 0)
      return OP_INVALID;

  if (opc->operands2)
    if (opc->operands2 (mra, n_operands, operands) < 0)
      return OP_INVALID;

  return op;
}

int
decode_s12z (enum optr *myoperator, short *osize,
	     int *n_operands, struct operand **operands,
	     struct mem_read_abstraction_base *mra)
{
  int n_bytes = 0;
  bfd_byte byte;

  int status = mra->read (mra, 0, 1, &byte);
  if (status < 0)
    return status;

  mra->advance (mra);

  const struct opcode *opc = page1 + byte;
  if (byte == PAGE2_PREBYTE)
    {
      /* Opcodes in page2 have an additional byte */
      n_bytes++;

      bfd_byte byte2;
      status = mra->read (mra, 0, 1, &byte2);
      if (status < 0)
	return status;
      mra->advance (mra);
      opc = page2 + byte2;
    }
  *myoperator = decode_operation (opc, mra, n_operands, operands);
  *osize = opc->osize;

  /* Return the number of bytes in the instruction.  */
  if (*myoperator != OP_INVALID && opc->insn_bytes)
    {
      int n = opc->insn_bytes (mra);
      if (n < 0)
	return n;
      n_bytes += n;
    }
  else
    n_bytes += 1;

  return n_bytes;
}

