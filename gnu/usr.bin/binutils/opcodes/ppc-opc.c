/* ppc-opc.c -- PowerPC opcode list
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support

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
#include <stdio.h>
#include "opcode/ppc.h"
#include "opintl.h"
#include "libiberty.h"

/* This file holds the PowerPC opcode table.  The opcode table
   includes almost all of the extended instruction mnemonics.  This
   permits the disassembler to use them, and simplifies the assembler
   logic, at the cost of increasing the table size.  The table is
   strictly constant data, so the compiler should be able to put it in
   the text segment.

   This file also holds the operand table.  All knowledge about
   inserting operands into instructions and vice-versa is kept in this
   file.  */

/* The functions used to insert and extract complicated operands.  */

/* The ARX, ARY, RX and RY operands are alternate encodings of GPRs.  */

static uint64_t
insert_arx (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  value -= 8;
  if (value < 0 || value >= 16)
    {
      *errmsg = _("invalid register");
      value = 0xf;
    }
  return insn | value;
}

static int64_t
extract_arx (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return (insn & 0xf) + 8;
}

static uint64_t
insert_ary (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  value -= 8;
  if (value < 0 || value >= 16)
    {
      *errmsg = _("invalid register");
      value = 0xf;
    }
  return insn | (value << 4);
}

static int64_t
extract_ary (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 4) & 0xf) + 8;
}

static uint64_t
insert_rx (uint64_t insn,
	   int64_t value,
	   ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	   const char **errmsg)
{
  if (value >= 0 && value < 8)
    ;
  else if (value >= 24 && value <= 31)
    value -= 16;
  else
    {
      *errmsg = _("invalid register");
      value = 0xf;
    }
  return insn | value;
}

static int64_t
extract_rx (uint64_t insn,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    int *invalid ATTRIBUTE_UNUSED)
{
  int64_t value = insn & 0xf;
  if (value >= 0 && value < 8)
    return value;
  else
    return value + 16;
}

static uint64_t
insert_ry (uint64_t insn,
	   int64_t value,
	   ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	   const char **errmsg)
{
  if (value >= 0 && value < 8)
    ;
  else if (value >= 24 && value <= 31)
    value -= 16;
  else
    {
      *errmsg = _("invalid register");
      value = 0xf;
    }
  return insn | (value << 4);
}

static int64_t
extract_ry (uint64_t insn,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    int *invalid ATTRIBUTE_UNUSED)
{
  int64_t value = (insn >> 4) & 0xf;
  if (value >= 0 && value < 8)
    return value;
  else
    return value + 16;
}

/* The BA and BB fields in an XL form instruction or the RA and RB fields or
   VRA and VRB fields in a VX form instruction when they must be the same.
   This is used for extended mnemonics like crclr.  The extraction function
   enforces that the fields are the same.  */

static uint64_t
insert_bab (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  value &= 0x1f;
  return insn | (value << 16) | (value << 11);
}

static int64_t
extract_bab (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  int64_t ba = (insn >> 16) & 0x1f;
  int64_t bb = (insn >> 11) & 0x1f;

  if (ba != bb)
    *invalid = 1;
  return ba;
}

/* The BT, BA and BB fields in an XL form instruction when they must all be
   the same.  This is used for extended mnemonics like crclr.  The extraction
   function enforces that the fields are the same.  */

static uint64_t
insert_btab (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  value &= 0x1f;
  return (value << 21) | insert_bab (insn, value, dialect, errmsg);
}

static int64_t
extract_btab (uint64_t insn,
	     ppc_cpu_t dialect,
	     int *invalid)
{
  int64_t bt = (insn >> 21) & 0x1f;
  int64_t bab = extract_bab (insn, dialect, invalid);

  if (bt != bab)
    *invalid = 1;
  return bt;
}

/* The BD field in a B form instruction when the - modifier is used.
   This modifier means that the branch is not expected to be taken.
   For chips built to versions of the architecture prior to version 2
   (ie. not Power4 compatible), we set the y bit of the BO field to 1
   if the offset is negative.  When extracting, we require that the y
   bit be 1 and that the offset be positive, since if the y bit is 0
   we just want to print the normal form of the instruction.
   Power4 compatible targets use two bits, "a", and "t", instead of
   the "y" bit.  "at" == 00 => no hint, "at" == 01 => unpredictable,
   "at" == 10 => not taken, "at" == 11 => taken.  The "t" bit is 00001
   in BO field, the "a" bit is 00010 for branch on CR(BI) and 01000
   for branch on CTR.  We only handle the taken/not-taken hint here.
   Note that we don't relax the conditions tested here when
   disassembling with -Many because insns using extract_bdm and
   extract_bdp always occur in pairs.  One or the other will always
   be valid.  */

#define ISA_V2 (PPC_OPCODE_POWER4 | PPC_OPCODE_E500MC | PPC_OPCODE_TITAN)

static uint64_t
insert_bdm (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  if ((dialect & ISA_V2) == 0)
    {
      if ((value & 0x8000) != 0)
	insn |= 1 << 21;
    }
  else
    {
      if ((insn & (0x14 << 21)) == (0x04 << 21))
	insn |= 0x02 << 21;
      else if ((insn & (0x14 << 21)) == (0x10 << 21))
	insn |= 0x08 << 21;
    }
  return insn | (value & 0xfffc);
}

static int64_t
extract_bdm (uint64_t insn,
	     ppc_cpu_t dialect,
	     int *invalid)
{
  if ((dialect & ISA_V2) == 0)
    {
      if (((insn & (1 << 21)) == 0) != ((insn & (1 << 15)) == 0))
	*invalid = 1;
    }
  else
    {
      if ((insn & (0x17 << 21)) != (0x06 << 21)
	  && (insn & (0x1d << 21)) != (0x18 << 21))
	*invalid = 1;
    }

  return ((insn & 0xfffc) ^ 0x8000) - 0x8000;
}

/* The BD field in a B form instruction when the + modifier is used.
   This is like BDM, above, except that the branch is expected to be
   taken.  */

static uint64_t
insert_bdp (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  if ((dialect & ISA_V2) == 0)
    {
      if ((value & 0x8000) == 0)
	insn |= 1 << 21;
    }
  else
    {
      if ((insn & (0x14 << 21)) == (0x04 << 21))
	insn |= 0x03 << 21;
      else if ((insn & (0x14 << 21)) == (0x10 << 21))
	insn |= 0x09 << 21;
    }
  return insn | (value & 0xfffc);
}

static int64_t
extract_bdp (uint64_t insn,
	     ppc_cpu_t dialect,
	     int *invalid)
{
  if ((dialect & ISA_V2) == 0)
    {
      if (((insn & (1 << 21)) == 0) == ((insn & (1 << 15)) == 0))
	*invalid = 1;
    }
  else
    {
      if ((insn & (0x17 << 21)) != (0x07 << 21)
	  && (insn & (0x1d << 21)) != (0x19 << 21))
	*invalid = 1;
    }

  return ((insn & 0xfffc) ^ 0x8000) - 0x8000;
}

static inline int
valid_bo_pre_v2 (int64_t value)
{
  /* Certain encodings have bits that are required to be zero.
     These are (z must be zero, y may be anything):
	 0000y
	 0001y
	 001zy
	 0100y
	 0101y
	 011zy
	 1z00y
	 1z01y
	 1z1zz
  */
  if ((value & 0x14) == 0)
    /* BO: 0000y, 0001y, 0100y, 0101y.  */
    return 1;
  else if ((value & 0x14) == 0x4)
    /* BO: 001zy, 011zy.  */
    return (value & 0x2) == 0;
  else if ((value & 0x14) == 0x10)
    /* BO: 1z00y, 1z01y.  */
    return (value & 0x8) == 0;
  else
    /* BO: 1z1zz.  */
    return value == 0x14;
}

static inline int
valid_bo_post_v2 (int64_t value)
{
  /* Certain encodings have bits that are required to be zero.
     These are (z must be zero, a & t may be anything):
	 0000z
	 0001z
	 001at
	 0100z
	 0101z
	 011at
	 1a00t
	 1a01t
	 1z1zz
  */
  if ((value & 0x14) == 0)
    /* BO: 0000z, 0001z, 0100z, 0101z.  */
    return (value & 0x1) == 0;
  else if ((value & 0x14) == 0x14)
    /* BO: 1z1zz.  */
    return value == 0x14;
  else if ((value & 0x14) == 0x4)
    /* BO: 001at, 011at, with "at" == 0b01 being reserved.  */
    return (value & 0x3) != 1;
  else if ((value & 0x14) == 0x10)
    /* BO: 1a00t, 1a01t, with "at" == 0b01 being reserved.  */
    return (value & 0x9) != 1;
  else
    return 1;
}

/* Check for legal values of a BO field.  */

static int
valid_bo (int64_t value, ppc_cpu_t dialect, int extract)
{
  int valid_y = valid_bo_pre_v2 (value);
  int valid_at = valid_bo_post_v2 (value);

  /* When disassembling with -Many, accept either encoding on the
     second pass through opcodes.  */
  if (extract && dialect == ~(ppc_cpu_t) PPC_OPCODE_ANY)
    return valid_y || valid_at;
  if ((dialect & ISA_V2) == 0)
    return valid_y;
  else
    return valid_at;
}

/* The BO field in a B form instruction.  Warn about attempts to set
   the field to an illegal value.  */

static uint64_t
insert_bo (uint64_t insn,
	   int64_t value,
	   ppc_cpu_t dialect,
	   const char **errmsg)
{
  if (!valid_bo (value, dialect, 0))
    *errmsg = _("invalid conditional option");
  else if (PPC_OP (insn) == 19
	   && (((insn >> 1) & 0x3ff) == 528) && ! (value & 4))
    *errmsg = _("invalid counter access");
  return insn | ((value & 0x1f) << 21);
}

static int64_t
extract_bo (uint64_t insn,
	    ppc_cpu_t dialect,
	    int *invalid)
{
  int64_t value = (insn >> 21) & 0x1f;
  if (!valid_bo (value, dialect, 1))
    *invalid = 1;
  return value;
}

/* For the given BO value, return a bit mask detailing which bits
   define the branch hints.  */

static int64_t
get_bo_hint_mask (int64_t bo, ppc_cpu_t dialect)
{
  if ((dialect & ISA_V2) == 0)
    {
      if ((bo & 0x14) != 0x14)
	/* BO: 0000y, 0001y, 001zy, 0100y, 0101y, 011zy, 1z00y, 1z01y .  */
	return 1;
      else
	/* BO: 1z1zz.  */
	return 0;
    }
  else
    {
      if ((bo & 0x14) == 0x4)
	/* BO: 001at, 011at.  */
	return 0x3;
      else if ((bo & 0x14) == 0x10)
	/* BO: 1a00t, 1a01t.  */
	return 0x9;
      else
	/* BO: 0000z, 0001z, 0100z, 0101z, 1z1zz.  */
	return 0;
    }
}

/* The BO field in a B form instruction when the + or - modifier is used.  */

static uint64_t
insert_boe (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect,
	    const char **errmsg,
	    int branch_taken)
{
  int64_t implied_hint;
  int64_t hint_mask = get_bo_hint_mask (value, dialect);

  if (branch_taken)
    implied_hint = hint_mask;
  else
    implied_hint = hint_mask & ~1;

  /* The branch hint bit(s) in the BO field must either be zero or exactly
     match the branch hint bits implied by the '+' or '-' modifier.  */
  if (implied_hint == 0)
    *errmsg = _("BO value implies no branch hint, when using + or - modifier");
  else if ((value & hint_mask) != 0
	   && (value & hint_mask) != implied_hint)
    {
      if ((dialect & ISA_V2) == 0)
	*errmsg = _("attempt to set y bit when using + or - modifier");
      else
	*errmsg = _("attempt to set 'at' bits when using + or - modifier");
    }

  value |= implied_hint;

  return insert_bo (insn, value, dialect, errmsg);
}

static int64_t
extract_boe (uint64_t insn,
	     ppc_cpu_t dialect,
	     int *invalid,
	     int branch_taken)
{
  int64_t value = (insn >> 21) & 0x1f;
  int64_t implied_hint;
  int64_t hint_mask = get_bo_hint_mask (value, dialect);

  if (branch_taken)
    implied_hint = hint_mask;
  else
    implied_hint = hint_mask & ~1;

  if (!valid_bo (value, dialect, 1)
      || implied_hint == 0
      || (value & hint_mask) != implied_hint)
    *invalid = 1;
  return value;
}

/* The BO field in a B form instruction when the - modifier is used.  */

static uint64_t
insert_bom (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect,
	    const char **errmsg)
{
  return insert_boe (insn, value, dialect, errmsg, 0);
}

static int64_t
extract_bom (uint64_t insn,
	     ppc_cpu_t dialect,
	     int *invalid)
{
  return extract_boe (insn, dialect, invalid, 0);
}

/* The BO field in a B form instruction when the + modifier is used.  */

static uint64_t
insert_bop (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect,
	    const char **errmsg)
{
  return insert_boe (insn, value, dialect, errmsg, 1);
}

static int64_t
extract_bop (uint64_t insn,
	     ppc_cpu_t dialect,
	     int *invalid)
{
  return extract_boe (insn, dialect, invalid, 1);
}

/* The DCMX field in a X form instruction when the field is split
   into separate DC, DM and DX fields.  */

static uint64_t
insert_dcmxs (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      const char **errmsg ATTRIBUTE_UNUSED)
{
  return (insn
	  | ((value & 0x1f) << 16)
	  | ((value & 0x20) >> 3)
	  | (value & 0x40));
}

static int64_t
extract_dcmxs (uint64_t insn,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       int *invalid ATTRIBUTE_UNUSED)
{
  return (insn & 0x40) | ((insn << 3) & 0x20) | ((insn >> 16) & 0x1f);
}

/* The DW field in a X form instruction when the field is split
   into separate D and DX fields.  */

static uint64_t
insert_dw (uint64_t insn,
	   int64_t value,
	   ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	   const char **errmsg ATTRIBUTE_UNUSED)
{
  /* DW offsets must be in the range [-512, -8] and be a multiple of 8.  */
  if (value < -512
      || value > -8
      || (value & 0x7) != 0)
    *errmsg = _("invalid offset: must be in the range [-512, -8] "
		"and be a multiple of 8");

  return insn | ((value & 0xf8) << 18) | ((value >> 8) & 1);
}

static int64_t
extract_dw (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  int64_t dw = ((insn & 1) << 8) | ((insn >> 18) & 0xf8);
  return dw - 512;
}

/* The D field in a DX form instruction when the field is split
   into separate D0, D1 and D2 fields.  */

static uint64_t
insert_dxd (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | (value & 0xffc1) | ((value & 0x3e) << 15);
}

static int64_t
extract_dxd (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  uint64_t dxd = (insn & 0xffc1) | ((insn >> 15) & 0x3e);
  return (dxd ^ 0x8000) - 0x8000;
}

static uint64_t
insert_dxdn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return insert_dxd (insn, -value, dialect, errmsg);
}

static int64_t
extract_dxdn (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid)
{
  return -extract_dxd (insn, dialect, invalid);
}

/* The D field in a 64-bit D form prefix instruction when the field is split
   into separate D0 and D1 fields.  */

static uint64_t
insert_d34 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x3ffff0000ULL) << 16) | (value & 0xffff);
}

static int64_t
extract_d34 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  int64_t mask = 1ULL << 33;
  int64_t value = ((insn >> 16) & 0x3ffff0000ULL) | (insn & 0xffff);
  value = (value ^ mask) - mask;
  return value;
}

/* The NSI34 field in an 8-byte D form prefix instruction.  This is the same
   as the SI34 field, only negated.  The extraction function always marks it
   as invalid, since we never want to recognize an instruction which uses
   a field of this type.  */

static uint64_t
insert_nsi34 (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect,
	      const char **errmsg)
{
  return insert_d34 (insn, -value, dialect, errmsg);
}

static int64_t
extract_nsi34 (uint64_t insn,
	       ppc_cpu_t dialect,
	       int *invalid)
{
  int64_t value = extract_d34 (insn, dialect, invalid);
  *invalid = 1;
  return -value;
}

/* The split IMM32 field in a vector splat insn.  */

static uint64_t
insert_imm32 (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0xffff0000) << 16) | (value & 0xffff);
}

static int64_t
extract_imm32 (uint64_t insn,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       int *invalid ATTRIBUTE_UNUSED)
{
  return (insn & 0xffff) | ((insn >> 16) & 0xffff0000);
}

/* The R field in an 8-byte prefix instruction when there are restrictions
   between R's value and the RA value (ie, they cannot both be non zero).  */

static uint64_t
insert_pcrel (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      const char **errmsg)
{
  value &= 0x1;
  int64_t ra = (insn >> 16) & 0x1f;
  if (ra != 0 && value != 0)
    *errmsg = _("invalid R operand");

  return insn | (value << 52);
}

static int64_t
extract_pcrel (uint64_t insn,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       int *invalid)
{
  /* If called with *invalid < 0 to return the value for missing
     operands, *invalid will be the negative count of missing operands
     including this one.  Return a default value of 1 if the PRA0/PRAQ
     operand was also omitted (ie. *invalid is -2).  Return a default
     value of 0 if the PRA0/PRAQ operand was not omitted
     (ie. *invalid is -1).  */
  if (*invalid < 0)
    return ~ *invalid & 1;

  int64_t ra = (insn >> 16) & 0x1f;
  int64_t pcrel = (insn >> 52) & 0x1;
  if (ra != 0 && pcrel != 0)
    *invalid = 1;

  return pcrel;
}

/* Variant of extract_pcrel that sets invalid for R bit clear.  Used
   to disassemble "paddi rt,0,offset,1" as "pla rt,offset".  */

static int64_t
extract_pcrel1 (uint64_t insn,
		ppc_cpu_t dialect,
		int *invalid)
{
  int64_t pcrel = extract_pcrel (insn, dialect, invalid);
  if (!pcrel)
    *invalid = 1;
  return pcrel;
}

/* FXM mask in mfcr and mtcrf instructions.  */

static uint64_t
insert_fxm (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect,
	    const char **errmsg)
{
  /* If we're handling the mfocrf and mtocrf insns ensure that exactly
     one bit of the mask field is set.  */
  if ((insn & (1 << 20)) != 0)
    {
      if (value == 0 || (value & -value) != value)
	{
	  *errmsg = _("invalid mask field");
	  value = 0;
	}
    }

  /* If only one bit of the FXM field is set, we can use the new form
     of the instruction, which is faster.  Unlike the Power4 branch hint
     encoding, this is not backward compatible.  Do not generate the
     new form unless -mpower4 has been given, or -many and the two
     operand form of mfcr was used.  */
  else if (value > 0
	   && (value & -value) == value
	   && ((dialect & PPC_OPCODE_POWER4) != 0
	       || ((dialect & PPC_OPCODE_ANY) != 0
		   && (insn & (0x3ff << 1)) == 19 << 1)))
    insn |= 1 << 20;

  /* Any other value on mfcr is an error.  */
  else if ((insn & (0x3ff << 1)) == 19 << 1)
    {
      /* A value of -1 means we used the one operand form of
	 mfcr which is valid.  */
      if (value != -1)
	*errmsg = _("invalid mfcr mask");
      value = 0;
    }

  return insn | ((value & 0xff) << 12);
}

static int64_t
extract_fxm (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  /* Return a value of -1 for a missing optional operand, which is
     used as a flag by insert_fxm.  */
  if (*invalid < 0)
    return -1;

  int64_t mask = (insn >> 12) & 0xff;
  /* Is this a Power4 insn?  */
  if ((insn & (1 << 20)) != 0)
    {
      /* Exactly one bit of MASK should be set.  */
      if (mask == 0 || (mask & -mask) != mask)
	*invalid = 1;
    }

  /* Check that non-power4 form of mfcr has a zero MASK.  */
  else if ((insn & (0x3ff << 1)) == 19 << 1)
    {
      if (mask != 0)
	*invalid = 1;
      else
	mask = -1;
    }

  return mask;
}

/* L field in the paste. instruction.  */

static uint64_t
insert_l1opt (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 1) << 21);
}

static int64_t
extract_l1opt (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  /* Return a value of 1 for a missing optional operand.  */
  if (*invalid < 0)
    return 1;

  return (insn >> 21) & 1;
}

static uint64_t
insert_li20 (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return (insn
	  | ((value & 0xf0000) >> 5)
	  | ((value & 0x0f800) << 5)
	  | (value & 0x7ff));
}

static int64_t
extract_li20 (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid ATTRIBUTE_UNUSED)
{
  return ((((insn << 5) & 0xf0000)
	   | ((insn >> 5) & 0xf800)
	   | (insn & 0x7ff)) ^ 0x80000) - 0x80000;
}

/* The 2-bit/3-bit L or 2-bit WC field in a SYNC, DCBF or WAIT instruction.
   For SYNC, some L values are reserved:
     * Values 6 and 7 are reserved on newer server cpus.
     * Value 3 is reserved on all server cpus.
     * Value 2 is reserved on all other cpus.
   For DCBF, some L values are reserved:
     * Values 2, 5 and 7 are reserved on all cpus.
   For WAIT, some WC values are reserved:
     * Value 3 is reserved on all server cpus.
     * Values 1 and 2 are reserved on older server cpus.  */

static uint64_t
insert_ls (uint64_t insn,
	   int64_t value,
	   ppc_cpu_t dialect,
	   const char **errmsg)
{
  int64_t mask;

  if (((insn >> 1) & 0x3ff) == 598)
    {
      /* For SYNC, some L values are illegal.  */
      mask = (dialect & PPC_OPCODE_POWER10) ?  0x7 : 0x3;

      /* If the value is within range, check for other illegal values.  */
      if ((value & mask) == value)
	switch (value)
	  {
	  case 2:
	    if (dialect & PPC_OPCODE_POWER4)
	      break;
	    /* Fall through.  */
	  case 3:
	  case 6:
	  case 7:
	    *errmsg = _("illegal L operand value");
	    break;
	  default:
	    break;
	  }
    }
  else if (((insn >> 1) & 0x3ff) == 86)
    {
      /* For DCBF, some L values are illegal.  */
      mask = (dialect & PPC_OPCODE_POWER10) ?  0x7 : 0x3;

      /* If the value is within range, check for other illegal values.  */
      if ((value & mask) == value)
	switch (value)
	  {
	  case 2:
	  case 5:
	  case 7:
	    *errmsg = _("illegal L operand value");
	    break;
	  default:
	    break;
	  }
    }
  else
    {
      /* For WAIT, some WC values are illegal.  */
      mask = 0x3;

      /* If the value is within range, check for other illegal values.  */
      if ((dialect & PPC_OPCODE_A2) == 0
	  && (dialect & PPC_OPCODE_E500MC) == 0
	  && (value & mask) == value)
	switch (value)
	  {
	  case 1:
	  case 2:
	    if (dialect & PPC_OPCODE_POWER10)
	      break;
	    /* Fall through.  */
	  case 3:
	    *errmsg = _("illegal WC operand value");
	    break;
	  default:
	    break;
	  }
    }

  return insn | ((value & mask) << 21);
}

static int64_t
extract_ls (uint64_t insn,
	    ppc_cpu_t dialect,
	    int *invalid)
{
  uint64_t value;

  /* Missing optional operands have a value of zero.  */
  if (*invalid < 0)
    return 0;

  if (((insn >> 1) & 0x3ff) == 598)
    {
      /* For SYNC, some L values are illegal.  */
      int64_t mask = (dialect & PPC_OPCODE_POWER10) ?  0x7 : 0x3;

      value = (insn >> 21) & mask;
      switch (value)
	{
	case 2:
	  if (dialect & PPC_OPCODE_POWER4)
	    break;
	  /* Fall through.  */
	case 3:
	case 6:
	case 7:
	  *invalid = 1;
	  break;
	default:
	  break;
	}
    }
  else if (((insn >> 1) & 0x3ff) == 86)
    {
      /* For DCBF, some L values are illegal.  */
      int64_t mask = (dialect & PPC_OPCODE_POWER10) ?  0x7 : 0x3;

      value = (insn >> 21) & mask;
      switch (value)
	{
	case 2:
	case 5:
	case 7:
	  *invalid = 1;
	  break;
	default:
	  break;
	}
    }
  else
    {
      /* For WAIT, some WC values are illegal.  */
      value = (insn >> 21) & 0x3;
      if ((dialect & PPC_OPCODE_A2) == 0
	  && (dialect & PPC_OPCODE_E500MC) == 0)
	switch (value)
	  {
	  case 1:
	  case 2:
	    if (dialect & PPC_OPCODE_POWER10)
	      break;
	    /* Fall through.  */
	  case 3:
	    *invalid = 1;
	    break;
	  default:
	    break;
	  }
    }

  return value;
}

/* The 4-bit E field in a sync instruction that accepts 2 operands.
   If ESYNC is non-zero, then the L field must be either 0 or 1 and
   the complement of ESYNC-bit2.  */

static uint64_t
insert_esync (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      const char **errmsg)
{
  uint64_t ls = (insn >> 21) & 0x03;

  if (value != 0
      && ((~value >> 1) & 0x1) != ls)
    *errmsg = _("incompatible L operand value");

  return insn | ((value & 0xf) << 16);
}

static int64_t
extract_esync (uint64_t insn,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       int *invalid)
{
  /* Missing optional operands have a value of zero.  */
  if (*invalid < 0)
    return 0;

  uint64_t ls = (insn >> 21) & 0x3;
  uint64_t value = (insn >> 16) & 0xf;
  if (value != 0
      && ((~value >> 1) & 0x1) != ls)
    *invalid = 1;
  return value;
}

/* The n operand of clrrwi, which sets the ME field to 31 - n.  */

static uint64_t
insert_crwn (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((~value & 0x1f) << 1);
}

static int64_t
extract_crwn (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ~(insn >> 1) & 0x1f;
}

/* The n operand of extlwi, which sets the ME field to n - 1.  */

static uint64_t
insert_elwn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | (((value - 1) & 0x1f) << 1);
}

static int64_t
extract_elwn (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 1) & 0x1f) + 1;
}

/* The n operand of extrwi, sets MB = 32 - n.  */

static uint64_t
insert_erwn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((-value & 0x1f) << 6);
}

static int64_t
extract_erwn (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid ATTRIBUTE_UNUSED)
{
  return (~(insn >> 6) & 0x1f) + 1;
}

/* The b operand of extrwi, sets SH = b + n.  */

static uint64_t
insert_erwb (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  int64_t n = extract_erwn (insn, dialect, NULL);
  return insn | (((n + value) & 0x1f) << 11);
}

static int64_t
extract_erwb (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid ATTRIBUTE_UNUSED)
{
  int64_t n = extract_erwn (insn, dialect, NULL);
  return ((insn >> 11) - n) & 0x1f;
}

/* The n and b operands of clrlslwi.  */

static uint64_t
insert_cslwn (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      const char **errmsg ATTRIBUTE_UNUSED)
{
  uint64_t mb = 0x1f << 6;
  int64_t b = (insn >> 6) & 0x1f;
  return ((insn & ~mb) | ((value & 0x1f) << 11) | (((b - value) & 0x1f) << 6)
	  | ((~value & 0x1f) << 1));
}

static int64_t
extract_cslwb (uint64_t insn,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       int *invalid)
{
  int64_t sh = (insn >> 11) & 0x1f;
  int64_t mb = (insn >> 6) & 0x1f;
  int64_t me = (insn >> 1) & 0x1f;
  if (sh != 31 - me)
    *invalid = 1;
  return (mb + sh) & 0x1f;
}

/* The n and b operands of inslwi.  */

static uint64_t
insert_ilwb (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  uint64_t me = 0x1f << 1;
  int64_t n = (insn >> 1) & 0x1f;
  return ((insn & ~me) | ((-value & 0x1f) << 11) | ((value & 0x1f) << 6)
	  | (((value + n - 1) & 0x1f) << 1));
}

static int64_t
extract_ilwn (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid)
{
  int64_t sh = (insn >> 11) & 0x1f;
  int64_t mb = (insn >> 6) & 0x1f;
  int64_t me = (insn >> 1) & 0x1f;
  if (((sh + mb) & 0x1f) != 0)
    *invalid = 1;
  return ((me - mb) & 0x1f) + 1;
}

/* The n and b operands of insrwi.  */

static uint64_t
insert_irwb (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  uint64_t me = 0x1f << 1;
  int64_t n = (insn >> 1) & 0x1f;
  return ((insn & ~me) | ((-(value + n) & 0x1f) << 11) | ((value & 0x1f) << 6)
	  | (((value + n - 1) & 0x1f) << 1));
}

static int64_t
extract_irwn (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid)
{
  int64_t sh = (insn >> 11) & 0x1f;
  int64_t mb = (insn >> 6) & 0x1f;
  int64_t me = (insn >> 1) & 0x1f;
  if (((sh + me + 1) & 0x1f) != 0)
    *invalid = 1;
  return ((me - mb) & 0x1f) + 1;
}

/* The MB and ME fields in an M form instruction expressed as a single
   operand which is itself a bitmask.  The extraction function always
   marks it as invalid, since we never want to recognize an
   instruction which uses a field of this type.  */

static uint64_t
insert_mbe (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  uint64_t uval, mask;
  long mb, me, mx, count, last;

  uval = value;

  if (uval == 0)
    {
      *errmsg = _("illegal bitmask");
      return insn;
    }

  mb = 0;
  me = 32;
  if ((uval & 1) != 0)
    last = 1;
  else
    last = 0;
  count = 0;

  /* mb: location of last 0->1 transition */
  /* me: location of last 1->0 transition */
  /* count: # transitions */

  for (mx = 0, mask = (uint64_t) 1 << 31; mx < 32; ++mx, mask >>= 1)
    {
      if ((uval & mask) && !last)
	{
	  ++count;
	  mb = mx;
	  last = 1;
	}
      else if (!(uval & mask) && last)
	{
	  ++count;
	  me = mx;
	  last = 0;
	}
    }
  if (me == 0)
    me = 32;

  if (count != 2 && (count != 0 || ! last))
    *errmsg = _("illegal bitmask");

  return insn | (mb << 6) | ((me - 1) << 1);
}

static int64_t
extract_mbe (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  int64_t ret;
  long mb, me;
  long i;

  *invalid = 1;

  mb = (insn >> 6) & 0x1f;
  me = (insn >> 1) & 0x1f;
  if (mb < me + 1)
    {
      ret = 0;
      for (i = mb; i <= me; i++)
	ret |= (uint64_t) 1 << (31 - i);
    }
  else if (mb == me + 1)
    ret = ~0;
  else /* (mb > me + 1) */
    {
      ret = ~0;
      for (i = me + 1; i < mb; i++)
	ret &= ~((uint64_t) 1 << (31 - i));
    }
  return ret;
}

/* The MB or ME field in an MD or MDS form instruction.  The high bit
   is wrapped to the low end.  */

static uint64_t
insert_mb6 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 6) | (value & 0x20);
}

static int64_t
extract_mb6 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 6) & 0x1f) | (insn & 0x20);
}

/* The n operand of extrdi, which sets MB field.  */

static uint64_t
insert_erdn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  return insert_mb6 (insn, -value, dialect, errmsg);
}

static int64_t
extract_erdn (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  return (~extract_mb6 (insn, dialect, invalid) & 63) + 1;
}

/* The n operand of extldi, which sets ME field.  */

static uint64_t
insert_eldn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  return insert_mb6 (insn, value - 1, dialect, errmsg);
}

static int64_t
extract_eldn (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  return extract_mb6 (insn, dialect, invalid) + 1;
}

/* The n operand of clrrdi, which set ME field.  */

static uint64_t
insert_crdn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  return insert_mb6 (insn, 63 - value, dialect, errmsg);
}

static int64_t
extract_crdn (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  return 63 - extract_mb6 (insn, dialect, invalid);
}

/* The NB field in an X form instruction.  The value 32 is stored as
   0.  */

static int64_t
extract_nb (uint64_t insn,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    int *invalid ATTRIBUTE_UNUSED)
{
  int64_t ret;

  ret = (insn >> 11) & 0x1f;
  if (ret == 0)
    ret = 32;
  return ret;
}

/* The NB field in an lswi instruction, which has special value
   restrictions.  The value 32 is stored as 0.  */

static uint64_t
insert_nbi (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  int64_t rtvalue = (insn >> 21) & 0x1f;
  int64_t ravalue = (insn >> 16) & 0x1f;

  if (value == 0)
    value = 32;
  if (rtvalue + (value + 3) / 4 > (rtvalue > ravalue ? ravalue + 32
						     : ravalue))
    *errmsg = _("address register in load range");
  return insn | ((value & 0x1f) << 11);
}

/* The NSI field in a D form instruction.  This is the same as the SI
   field, only negated.  The extraction function always marks it as
   invalid, since we never want to recognize an instruction which uses
   a field of this type.  */

static uint64_t
insert_nsi (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | (-value & 0xffff);
}

static int64_t
extract_nsi (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  *invalid = 1;
  return -(((insn & 0xffff) ^ 0x8000) - 0x8000);
}

/* The 2-bit SC field in a SYNC or PL field in a WAIT instruction.
   For WAIT, some PL values are reserved:
     * Values 1, 2 and 3 are reserved.  */

static uint64_t
insert_pl (uint64_t insn,
	   int64_t value,
	   ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	   const char **errmsg)
{
  /* For WAIT, some PL values are illegal.  */
  if (((insn >> 1) & 0x3ff) == 30
      && value != 0)
    *errmsg = _("illegal PL operand value");
  return insn | ((value & 0x3) << 16);
}

static int64_t
extract_pl (uint64_t insn,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    int *invalid)
{
  /* Missing optional operands have a value of zero.  */
  if (*invalid < 0)
    return 0;

  uint64_t value = (insn >> 16) & 0x3;

  /* For WAIT, some PL values are illegal.  */
  if (((insn >> 1) & 0x3ff) == 30
      && value != 0)
    *invalid = 1;
  return value;
}

/* The 2-bit P field in a MMA XX2-form instruction.  This is split.  */

static uint64_t
insert_p2 (uint64_t insn,
	   int64_t value,
	   ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	   const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x2) << 15) | ((value & 0x1) << 11);
}

static int64_t
extract_p2 (uint64_t insn,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    int *invalid ATTRIBUTE_UNUSED)
{
  uint64_t value = ((insn >> 15) & 0x2) | ((insn >> 11) & 0x1);
  return value;
}

/* The RA field in a D or X form instruction which is an updating
   load, which means that the RA field may not be zero and may not
   equal the RT field.  */

static uint64_t
insert_ral (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  if (value == 0
      || (uint64_t) value == ((insn >> 21) & 0x1f))
    *errmsg = "invalid register operand when updating";
  return insn | ((value & 0x1f) << 16);
}

static int64_t
extract_ral (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  int64_t rtvalue = (insn >> 21) & 0x1f;
  int64_t ravalue = (insn >> 16) & 0x1f;

  if (rtvalue == ravalue || ravalue == 0)
    *invalid = 1;
  return ravalue;
}

/* The RA field in an lmw instruction, which has special value
   restrictions.  */

static uint64_t
insert_ram (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  if ((uint64_t) value >= ((insn >> 21) & 0x1f))
    *errmsg = _("index register in load range");
  return insn | ((value & 0x1f) << 16);
}

static int64_t
extract_ram (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  uint64_t rtvalue = (insn >> 21) & 0x1f;
  uint64_t ravalue = (insn >> 16) & 0x1f;

  if (ravalue >= rtvalue)
    *invalid = 1;
  return ravalue;
}

/* The RA field in the DQ form lq or an lswx instruction, which have special
   value restrictions.  */

static uint64_t
insert_raq (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  int64_t rtvalue = (insn >> 21) & 0x1f;

  if (value == rtvalue)
    *errmsg = _("source and target register operands must be different");
  return insn | ((value & 0x1f) << 16);
}

static int64_t
extract_raq (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  /* Missing optional operands have a value of zero.  */
  if (*invalid < 0)
    return 0;

  uint64_t rtvalue = (insn >> 21) & 0x1f;
  uint64_t ravalue = (insn >> 16) & 0x1f;
  if (ravalue == rtvalue)
    *invalid = 1;
  return ravalue;
}

/* The RA field in a D or X form instruction which is an updating
   store or an updating floating point load, which means that the RA
   field may not be zero.  */

static uint64_t
insert_ras (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  if (value == 0)
    *errmsg = _("invalid register operand when updating");
  return insn | ((value & 0x1f) << 16);
}

static int64_t
extract_ras (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  uint64_t ravalue = (insn >> 16) & 0x1f;

  if (ravalue == 0)
    *invalid = 1;
  return ravalue;
}

/* The RS and RB fields in an X form instruction when they must be the same.
   This is used for extended mnemonics like mr.  The extraction function
   enforces that the fields are the same.  */

static uint64_t
insert_rsb (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  value &= 0x1f;
  return insn | (value << 21) | (value << 11);
}

static int64_t
extract_rsb (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  int64_t rs = (insn >> 21) & 0x1f;
  int64_t rb = (insn >> 11) & 0x1f;

  if (rs != rb)
    *invalid = 1;
  return rs;
}

/* The RB field in an lswx instruction, which has special value
   restrictions.  */

static uint64_t
insert_rbx (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  int64_t rtvalue = (insn >> 21) & 0x1f;

  if (value == rtvalue)
    *errmsg = _("source and target register operands must be different");
  return insn | ((value & 0x1f) << 11);
}

static int64_t
extract_rbx (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  uint64_t rtvalue = (insn >> 21) & 0x1f;
  uint64_t rbvalue = (insn >> 11) & 0x1f;

  if (rbvalue == rtvalue)
    *invalid = 1;
  return rbvalue;
}

/* The SCI8 field is made up of SCL and {U,N}I8 fields.  */
static uint64_t
insert_sci8 (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg)
{
  uint64_t fill_scale = 0;
  uint64_t ui8 = value;

  if ((ui8 & 0xffffff00) == 0)
    ;
  else if ((ui8 & 0xffffff00) == 0xffffff00)
    fill_scale = 0x400;
  else if ((ui8 & 0xffff00ff) == 0)
    {
      fill_scale = 1 << 8;
      ui8 >>= 8;
    }
  else if ((ui8 & 0xffff00ff) == 0xffff00ff)
    {
      fill_scale = 0x400 | (1 << 8);
      ui8 >>= 8;
    }
  else if ((ui8 & 0xff00ffff) == 0)
    {
      fill_scale = 2 << 8;
      ui8 >>= 16;
    }
  else if ((ui8 & 0xff00ffff) == 0xff00ffff)
    {
      fill_scale = 0x400 | (2 << 8);
      ui8 >>= 16;
    }
  else if ((ui8 & 0x00ffffff) == 0)
    {
      fill_scale = 3 << 8;
      ui8 >>= 24;
    }
  else if ((ui8 & 0x00ffffff) == 0x00ffffff)
    {
      fill_scale = 0x400 | (3 << 8);
      ui8 >>= 24;
    }
  else
    {
      *errmsg = _("illegal immediate value");
      ui8 = 0;
    }

  return insn | fill_scale | (ui8 & 0xff);
}

static int64_t
extract_sci8 (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid ATTRIBUTE_UNUSED)
{
  int64_t fill = insn & 0x400;
  int64_t scale_factor = (insn & 0x300) >> 5;
  int64_t value = (insn & 0xff) << scale_factor;

  if (fill != 0)
    value |= ~((int64_t) 0xff << scale_factor);
  return value;
}

static uint64_t
insert_sci8n (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect,
	      const char **errmsg)
{
  return insert_sci8 (insn, -value, dialect, errmsg);
}

static int64_t
extract_sci8n (uint64_t insn,
	       ppc_cpu_t dialect,
	       int *invalid)
{
  return -extract_sci8 (insn, dialect, invalid);
}

static uint64_t
insert_oimm (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | (((value - 1) & 0x1f) << 4);
}

static int64_t
extract_oimm (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 4) & 0x1f) + 1;
}

/* The n operand of rotrwi, sets SH = 32 - n.  */

static uint64_t
insert_rrwn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((-value & 0x1f) << 11);
}

static int64_t
extract_rrwn (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid ATTRIBUTE_UNUSED)
{
  return 31 & -(insn >> 11);
}

/* The n operand of slwi, sets SH = n and ME = 31 - n.  */

static uint64_t
insert_slwn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 11) | ((~value & 0x1f) << 1);
}

static int64_t
extract_slwn (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid)
{
  int64_t sh = (insn >> 11) & 0x1f;
  int64_t nme = ~(insn >> 1) & 0x1f;
  if (sh != nme)
    *invalid = 1;
  return sh;
}

/* The n operand of srwi, sets SH = 32 - n and MB = n.  */

static uint64_t
insert_srwn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((-value & 0x1f) << 11) | ((value & 0x1f) << 6);
}

static int64_t
extract_srwn (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid)
{
  int64_t nsh = -(insn >> 11) & 0x1f;
  int64_t mb = (insn >> 6) & 0x1f;
  if (nsh != mb)
    *invalid = 1;
  return nsh;
}

/* The SH field in an MD form instruction.  This is split.  */

static uint64_t
insert_sh6 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 11) | ((value & 0x20) >> 4);
}

static int64_t
extract_sh6 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 11) & 0x1f) | ((insn << 4) & 0x20);
}

/* The n operand of rotrdi, which writes to SH field.  */

static uint64_t
insert_rrdn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  return insert_sh6 (insn, -value, dialect, errmsg);
}

static int64_t
extract_rrdn (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  return -extract_sh6 (insn, dialect, invalid) & 63;
}

/* The n operand of sldi, which writes to SH and ME fields.  */

static uint64_t
insert_sldn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  insn = insert_sh6 (insn, value, dialect, errmsg);
  return insert_crdn (insn, value, dialect, errmsg);
}

static int64_t
extract_sldn (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  int64_t sh = extract_sh6 (insn, dialect, invalid);
  int64_t me = extract_crdn (insn, dialect, invalid);
  if (me != sh)
    *invalid = 1;
  return sh;
}

/* The n operand of srdi, which writes to SH and MB fields.  */

static uint64_t
insert_srdn (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  insn = insert_rrdn (insn, value, dialect, errmsg);
  return insert_mb6 (insn, value, dialect, errmsg);
}

static int64_t
extract_srdn (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  int64_t sh = extract_rrdn (insn, dialect, invalid);
  int64_t mb = extract_mb6 (insn, dialect, invalid);
  if (mb != sh)
    *invalid = 1;
  return sh;
}

/* The b operand of extrdi, which sets SH field.  */

static uint64_t
insert_erdb (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  int64_t n = extract_erdn (insn, dialect, NULL);
  return insert_sh6 (insn, value + n, dialect, errmsg);
}

static int64_t
extract_erdb (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  int64_t sh = extract_sh6 (insn, dialect, invalid);
  int64_t n = extract_erdn (insn, dialect, invalid);
  return (sh - n) & 63;
}

/* The b and n operands of clrlsldi.  */

static uint64_t
insert_csldn (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect,
	      const char **errmsg)
{
  uint64_t mb6 = 0x3f << 5;
  int64_t b = extract_mb6 (insn, dialect, NULL);
  insn = insert_mb6 (insn & ~mb6, b - value, dialect, errmsg);
  return insert_sh6 (insn, value, dialect, errmsg);
}

static int64_t
extract_csldb (uint64_t insn,
	       ppc_cpu_t dialect,
	       int *invalid)
{
  int64_t sh = extract_sh6 (insn, dialect, invalid);
  int64_t mb = extract_mb6 (insn, dialect, invalid);
  return (mb + sh) & 63;
}

/* The b and n operands of insrdi.  */

static uint64_t
insert_irdb (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  uint64_t sh6 = (0x1f << 11) | 2;
  int64_t n = extract_sh6 (insn, dialect, NULL);
  insn = insert_sh6 (insn & ~sh6, -(value + n), dialect, errmsg);
  return insert_mb6 (insn, value, dialect, errmsg);
}

static int64_t
extract_irdn (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  int64_t sh = extract_sh6 (insn, dialect, invalid);
  int64_t mb = extract_mb6 (insn, dialect, invalid);
  return (~(mb + sh) & 63) + 1;
}

/* The SPR field in an XFX form instruction.  This is flipped--the
   lower 5 bits are stored in the upper 5 and vice- versa.  */

static uint64_t
insert_spr (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 16) | ((value & 0x3e0) << 6);
}

static int64_t
extract_spr (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 16) & 0x1f) | ((insn >> 6) & 0x3e0);
}

/* Some dialects have 8 [DI]BAT registers instead of the standard 4.  */
#define ALLOW8_BAT (PPC_OPCODE_750)

static uint64_t
insert_sprbat (uint64_t insn,
	       int64_t value,
	       ppc_cpu_t dialect,
	       const char **errmsg)
{
  if ((uint64_t) value > 7
      || ((uint64_t) value > 3 && (dialect & ALLOW8_BAT) == 0))
    *errmsg = _("invalid bat number");

  /* If this is [di]bat4..7 then use spr 560..575, otherwise 528..543.  */
  if ((uint64_t) value > 3)
    value = ((value & 3) << 6) | 1;
  else
    value = value << 6;

  return insn | (value << 11);
}

static int64_t
extract_sprbat (uint64_t insn,
		ppc_cpu_t dialect,
		int *invalid)
{
  uint64_t val = (insn >> 17) & 0x3;

  val = val + ((insn >> 9) & 0x4);
  if (val > 3 && (dialect & ALLOW8_BAT) == 0)
    *invalid = 1;
  return val;
}

/* Some dialects have 8 SPRG registers instead of the standard 4.  */
#define ALLOW8_SPRG (PPC_OPCODE_BOOKE | PPC_OPCODE_405)

static uint64_t
insert_sprg (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  if ((uint64_t) value > 7
      || ((uint64_t) value > 3 && (dialect & ALLOW8_SPRG) == 0))
    *errmsg = _("invalid sprg number");

  /* If this is mfsprg4..7 then use spr 260..263 which can be read in
     user mode.  Anything else must use spr 272..279.  */
  if ((uint64_t) value <= 3 || (insn & 0x100) != 0)
    value |= 0x10;

  return insn | ((value & 0x17) << 16);
}

static int64_t
extract_sprg (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  uint64_t val = (insn >> 16) & 0x1f;

  /* mfsprg can use 260..263 and 272..279.  mtsprg only uses spr 272..279
     If not BOOKE, 405 or VLE, then both use only 272..275.  */
  if ((val - 0x10 > 3 && (dialect & ALLOW8_SPRG) == 0)
      || (val - 0x10 > 7 && (insn & 0x100) != 0)
      || val <= 3
      || (val & 8) != 0)
    *invalid = 1;
  return val & 7;
}

/* The TBR field in an XFX instruction.  This is just like SPR, but it
   is optional.  */

static uint64_t
insert_tbr (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  if (value != 268 && value != 269)
    *errmsg = _("invalid tbr number");
  return insn | ((value & 0x1f) << 16) | ((value & 0x3e0) << 6);
}

static int64_t
extract_tbr (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  /* Missing optional operands have a value of 268.  */
  if (*invalid < 0)
    return 268;

  int64_t ret = ((insn >> 16) & 0x1f) | ((insn >> 6) & 0x3e0);
  if (ret != 268 && ret != 269)
    *invalid = 1;
  return ret;
}

/* The XT and XS fields in an XX1 or XX3 form instruction.  This is split.  */

static uint64_t
insert_xt6 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 21) | ((value & 0x20) >> 5);
}

static int64_t
extract_xt6 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn << 5) & 0x20) | ((insn >> 21) & 0x1f);
}

/* The XT and XS fields in an DQ form VSX instruction.  This is split.  */
static uint64_t
insert_xtq6 (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 21) | ((value & 0x20) >> 2);
}

static int64_t
extract_xtq6 (uint64_t insn,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn << 2) & 0x20) | ((insn >> 21) & 0x1f);
}

/* The 5-bit XAp field in an XX3 form instruction.  This is split.  */

static uint64_t
insert_xa5 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1e) << 16) | ((value & 0x20) >> 3);
}

static int64_t
extract_xa5 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn << 3) & 0x20) | ((insn >> 16) & 0x1e);
}

/* The XA field in an XX3 form instruction.  This is split.  */

static uint64_t
insert_xa6 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 16) | ((value & 0x20) >> 3);
}

static int64_t
extract_xa6 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn << 3) & 0x20) | ((insn >> 16) & 0x1f);
}

/* The XA field in an MMA XX3 form instruction.  This is split
   and must not overlap with the ACC operand.  */

static uint64_t
insert_xa6a (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  int64_t acc = (insn >> 23) & 0x7;
  /* Power10 doesn't allow VSRs to overlap ACCs in MMA instructions.  */
  if ((dialect & PPC_OPCODE_FUTURE) == 0
      && (value >> 2) == acc)
    *errmsg = _("VSR overlaps ACC operand");
  return insert_xa6 (insn, value, dialect, errmsg);
}

static int64_t
extract_xa6a (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  int64_t acc = (insn >> 23) & 0x7;
  int64_t value = extract_xa6 (insn, dialect, invalid);
  /* Power10 doesn't allow VSRs to overlap ACCs in MMA instructions.  */
  if ((dialect & PPC_OPCODE_FUTURE) == 0
      && (value >> 2) == acc)
    *invalid = 1;
  return value;
}

/* The 5-bit XB field in an XX3 form instruction.  This is split.  */

static uint64_t
insert_xb5 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1e) << 11) | ((value & 0x20) >> 4);
}

static int64_t
extract_xb5 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn << 4) & 0x20) | ((insn >> 11) & 0x1e);
}
/* The XB field in an XX3 form instruction.  This is split.  */

static uint64_t
insert_xb6 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 11) | ((value & 0x20) >> 4);
}

static int64_t
extract_xb6 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn << 4) & 0x20) | ((insn >> 11) & 0x1f);
}

/* The XB field in an MMA XX3 form instruction.  This is split
   and must not overlap with the ACC operand.  */

static uint64_t
insert_xb6a (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  int64_t acc = (insn >> 23) & 0x7;
  /* Power10 doesn't allow VSRs to overlap ACCs in MMA instructions.  */
  if ((dialect & PPC_OPCODE_FUTURE) == 0
      && (value >> 2) == acc)
    *errmsg = _("VSR overlaps ACC operand");
  return insert_xb6 (insn, value, dialect, errmsg);
}

static int64_t
extract_xb6a (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  int64_t acc = (insn >> 23) & 0x7;
  int64_t value = extract_xb6 (insn, dialect, invalid);
  /* Power10 doesn't allow VSRs to overlap ACCs in MMA instructions.  */
  if ((dialect & PPC_OPCODE_FUTURE) == 0
      && (value >> 2) == acc)
    *invalid = 1;
  return value;
}

/* The XA and XB fields in an XX3 form instruction when they must be the same.
   This is used for extended mnemonics like xvmovdp.  The extraction function
   enforces that the fields are the same.  */

static uint64_t
insert_xab6 (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect,
	     const char **errmsg)
{
  return insert_xa6 (insn, value, dialect, errmsg)
	 | insert_xb6 (insn, value, dialect, errmsg);
}

static int64_t
extract_xab6 (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  int64_t xa6 = extract_xa6 (insn, dialect, invalid);
  int64_t xb6 = extract_xb6 (insn, dialect, invalid);

  if (xa6 != xb6)
    *invalid = 1;
  return xa6;
}

/* The XC field in an XX4 form instruction.  This is split.  */

static uint64_t
insert_xc6 (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 6) | ((value & 0x20) >> 2);
}

static int64_t
extract_xc6 (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn << 2) & 0x20) | ((insn >> 6) & 0x1f);
}

/* The split XTp and XSp field in a vector paired insn.  */

static uint64_t
insert_xtp (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1e) << 21) | ((value & 0x20) << (21 - 5));
}

static int64_t
extract_xtp (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> (21 - 5)) & 0x20) | ((insn >> 21) & 0x1e);
}

/* The split XT field in a vector splat insn.  */

static uint64_t
insert_xts (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1f) << 21) | ((value & 0x20) << (16 - 5));
}

static int64_t
extract_xts (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> (16 - 5)) & 0x20) | ((insn >> 21) & 0x1f);
}

static uint64_t
insert_dm (uint64_t insn,
	   int64_t value,
	   ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	   const char **errmsg)
{
  if (value != 0 && value != 1)
    *errmsg = _("invalid constant");
  return insn | (((value) ? 3 : 0) << 8);
}

static int64_t
extract_dm (uint64_t insn,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    int *invalid)
{
  int64_t value = (insn >> 8) & 3;
  if (value != 0 && value != 3)
    *invalid = 1;
  return (value) ? 1 : 0;
}

/* The VLESIMM field in an I16A form instruction.  This is split.  */

static uint64_t
insert_vlesi (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0xf800) << 10) | (value & 0x7ff);
}

static int64_t
extract_vlesi (uint64_t insn,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       int *invalid ATTRIBUTE_UNUSED)
{
  int64_t value = ((insn >> 10) & 0xf800) | (insn & 0x7ff);
  value = (value ^ 0x8000) - 0x8000;
  return value;
}

static uint64_t
insert_vlensi (uint64_t insn,
	       int64_t value,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       const char **errmsg ATTRIBUTE_UNUSED)
{
  value = -value;
  return insn | ((value & 0xf800) << 10) | (value & 0x7ff);
}
static int64_t
extract_vlensi (uint64_t insn,
		ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		int *invalid)
{
  int64_t value = ((insn >> 10) & 0xf800) | (insn & 0x7ff);
  value = (value ^ 0x8000) - 0x8000;
  /* Don't use for disassembly.  */
  *invalid = 1;
  return -value;
}

/* The VLEUIMM field in an I16A form instruction.  This is split.  */

static uint64_t
insert_vleui (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0xf800) << 10) | (value & 0x7ff);
}

static int64_t
extract_vleui (uint64_t insn,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 10) & 0xf800) | (insn & 0x7ff);
}

/* The VLEUIMML field in an I16L form instruction.  This is split.  */

static uint64_t
insert_vleil (uint64_t insn,
	      int64_t value,
	      ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	      const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0xf800) << 5) | (value & 0x7ff);
}

static int64_t
extract_vleil (uint64_t insn,
	       ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	       int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 5) & 0xf800) | (insn & 0x7ff);
}

static uint64_t
insert_evuimm1_ex0 (uint64_t insn,
		    int64_t value,
		    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		    const char **errmsg)
{
  if (value <= 0 || value > 0x1f)
    *errmsg = _("UIMM = 00000 is illegal");
  return insn | ((value & 0x1f) << 11);
}

static int64_t
extract_evuimm1_ex0 (uint64_t insn,
		     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		     int *invalid)
{
  int64_t value = ((insn >> 11) & 0x1f);
  if (value == 0)
    *invalid = 1;

  return value;
}

static uint64_t
insert_evuimm2_ex0 (uint64_t insn,
		    int64_t value,
		    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		    const char **errmsg)
{
  if (value <= 0 || value > 0x3e)
    *errmsg = _("UIMM = 00000 is illegal");
  return insn | ((value & 0x3e) << 10);
}

static int64_t
extract_evuimm2_ex0 (uint64_t insn,
		     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		     int *invalid)
{
  int64_t value = ((insn >> 10) & 0x3e);
  if (value == 0)
    *invalid = 1;

  return value;
}

static uint64_t
insert_evuimm4_ex0 (uint64_t insn,
		    int64_t value,
		    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		    const char **errmsg)
{
  if (value <= 0 || value > 0x7c)
    *errmsg = _("UIMM = 00000 is illegal");
  return insn | ((value & 0x7c) << 9);
}

static int64_t
extract_evuimm4_ex0 (uint64_t insn,
		     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		     int *invalid)
{
  int64_t value = ((insn >> 9) & 0x7c);
  if (value == 0)
    *invalid = 1;

  return value;
}

static uint64_t
insert_evuimm8_ex0 (uint64_t insn,
		    int64_t value,
		    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		    const char **errmsg)
{
  if (value <= 0 || value > 0xf8)
    *errmsg = _("UIMM = 00000 is illegal");
  return insn | ((value & 0xf8) << 8);
}

static int64_t
extract_evuimm8_ex0 (uint64_t insn,
		     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		     int *invalid)
{
  int64_t value = ((insn >> 8) & 0xf8);
  if (value == 0)
    *invalid = 1;

  return value;
}

static uint64_t
insert_evuimm_lt8 (uint64_t insn,
		   int64_t value,
		   ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		   const char **errmsg)
{
  if (value < 0 || value > 7)
    *errmsg = _("UIMM values >7 are illegal");
  return insn | ((value & 0x7) << 11);
}

static int64_t
extract_evuimm_lt8 (uint64_t insn,
		    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		    int *invalid)
{
  int64_t value = ((insn >> 11) & 0x1f);
  if (value > 7)
    *invalid = 1;

  return value;
}

static uint64_t
insert_evuimm_lt16 (uint64_t insn,
		    int64_t value,
		    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		    const char **errmsg)
{
  if (value < 0 || value > 15)
    *errmsg = _("UIMM values >15 are illegal");
  return insn | ((value & 0xf) << 11);
}

static int64_t
extract_evuimm_lt16 (uint64_t insn,
		     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		     int *invalid)
{
  int64_t value = ((insn >> 11) & 0x1f);
  if (value > 15)
    *invalid = 1;

  return value;
}

static uint64_t
insert_rD_rS_even (uint64_t insn,
		   int64_t value,
		   ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		   const char **errmsg)
{
  if ((value & 0x1) != 0)
    *errmsg = _("GPR odd is illegal");
  return insn | ((value & 0x1e) << 21);
}

static int64_t
extract_rD_rS_even (uint64_t insn,
		    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		    int *invalid)
{
  int64_t value = ((insn >> 21) & 0x1f);
  if ((value & 0x1) != 0)
    *invalid = 1;

  return value;
}

static uint64_t
insert_off_lsp (uint64_t insn,
		int64_t value,
		ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		const char **errmsg)
{
  if (value <= 0 || value > 0x3)
    *errmsg = _("invalid offset");
  return insn | (value & 0x3);
}

static int64_t
extract_off_lsp (uint64_t insn,
		 ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		 int *invalid)
{
  int64_t value = (insn & 0x3);
  if (value == 0)
    *invalid = 1;

  return value;
}

static uint64_t
insert_off_spe2 (uint64_t insn,
		 int64_t value,
		 ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		 const char **errmsg)
{
  if (value <= 0 || value > 0x7)
    *errmsg = _("invalid offset");
  return insn | (value & 0x7);
}

static int64_t
extract_off_spe2 (uint64_t insn,
		  ppc_cpu_t dialect ATTRIBUTE_UNUSED,
		  int *invalid)
{
  int64_t value = (insn & 0x7);
  if (value == 0)
    *invalid = 1;

  return value;
}

static uint64_t
insert_Ddd (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  if (value < 0 || value > 0x7)
    *errmsg = _("invalid Ddd value");
  return insn | ((value & 0x3) << 11) | ((value & 0x4) >> 2);
}

static int64_t
extract_Ddd (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn >> 11) & 0x3) | ((insn << 2) & 0x4);
}

static uint64_t
insert_sxl (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg ATTRIBUTE_UNUSED)
{
  return insn | ((value & 0x1) << 11);
}

static int64_t
extract_sxl (uint64_t insn,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     int *invalid)
{
  /* Missing optional operands have a value of one.  */
  if (*invalid < 0)
    return 1;
  return (insn >> 11) & 0x1;
}

/* The list of embedded processors that use the embedded operand ordering
   for the 3 operand dcbt and dcbtst instructions.  */
#define DCBT_EO (PPC_OPCODE_E500 | PPC_OPCODE_E500MC | PPC_OPCODE_476 \
		 | PPC_OPCODE_A2)

/* ISA 2.03 and later specify extended mnemonics dcbtct, dcbtds, and
   dcbtstct, dcbtstds with a note saying these should be used in new
   programs rather than the base mnemonics "so that it can be coded
   with TH as the last operand for all categories".  For that reason
   the extended mnemonics are enabled in the assembler for the
   embedded processors, but not for the disassembler so as to display
   the embedded dcbt or dcbtst expected form with TH first for
   embedded programmers.  */

static uint64_t
insert_thct (uint64_t insn,
	    int64_t value,
	    ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	    const char **errmsg)
{
  if ((uint64_t) value > 7)
    *errmsg = _("invalid TH value");
  return insn | ((value & 7) << 21);
}

static int64_t
extract_thct (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  /* Missing optional operands have a value of 0.  */
  if (*invalid < 0)
    return 0;

  int64_t value = (insn >> 21) & 0x1f;
  if (value > 7 || (dialect & DCBT_EO) != 0)
    *invalid = 1;

  return value;
}

static uint64_t
insert_thds (uint64_t insn,
	     int64_t value,
	     ppc_cpu_t dialect ATTRIBUTE_UNUSED,
	     const char **errmsg)
{
  if (value < 8 || value > 15)
    *errmsg = _("invalid TH value");
  return insn | ((value & 0x1f) << 21);
}

static int64_t
extract_thds (uint64_t insn,
	      ppc_cpu_t dialect,
	      int *invalid)
{
  /* Missing optional operands have a value of 8.  */
  if (*invalid < 0)
    return 8;

  int64_t value = (insn >> 21) & 0x1f;
  if (value < 8 || value > 15 || (dialect & DCBT_EO) != 0)
    *invalid = 1;

  return value;
}

/* The operands table.

   The fields are bitm, shift, insert, extract, flags.

   We used to put parens around the various additions, like the one
   for BA just below.  However, that caused trouble with feeble
   compilers with a limit on depth of a parenthesized expression, like
   (reportedly) the compiler in Microsoft Developer Studio 5.  So we
   omit the parens, since the macros are never used in a context where
   the addition will be ambiguous.  */

const struct powerpc_operand powerpc_operands[] =
{
  /* The zero index is used to indicate the end of the list of
     operands.  */
#define UNUSED 0
  { 0, 0, NULL, NULL, 0 },

  /* The BA field in an XL form instruction.  */
#define BA UNUSED + 1
  /* The BI field in a B form or XL form instruction.  */
#define BI BA
#define BI_MASK (0x1f << 16)
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_CR_BIT },

  /* The BT, BA and BB fields in a XL form instruction when they must all
     be the same.  */
#define BTAB BA + 1
  { 0x1f, 21, insert_btab, extract_btab, PPC_OPERAND_CR_BIT },

  /* The BB field in an XL form instruction.  */
#define BB BTAB + 1
#define BB_MASK (0x1f << 11)
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_CR_BIT },

  /* The BA and BB fields in a XL form instruction when they must be
     the same.  */
#define BAB BB + 1
  { 0x1f, 16, insert_bab, extract_bab, PPC_OPERAND_CR_BIT },

  /* The VRA and VRB fields in a VX form instruction when they must be the same.
     This is used for extended mnemonics like vmr.  */
#define VAB BAB + 1
  { 0x1f, 16, insert_bab, extract_bab, PPC_OPERAND_VR },

  /* The RA and RB fields in a VX form instruction when they must be the same.
     This is used for extended mnemonics like evmr.  */
#define RAB VAB + 1
  { 0x1f, 16, insert_bab, extract_bab, PPC_OPERAND_GPR },

#define BC RAB + 1
  { 0x1f, 6, NULL, NULL, PPC_OPERAND_CR_BIT },

  /* The BD field in a B form instruction.  The lower two bits are
     forced to zero.  */
#define BD BC + 1
  { 0xfffc, 0, NULL, NULL, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when absolute addressing is
     used.  */
#define BDA BD + 1
  { 0xfffc, 0, NULL, NULL, PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when the - modifier is used.
     This sets the y bit of the BO field appropriately.  */
#define BDM BDA + 1
  { 0xfffc, 0, insert_bdm, extract_bdm,
    PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when the - modifier is used
     and absolute address is used.  */
#define BDMA BDM + 1
  { 0xfffc, 0, insert_bdm, extract_bdm,
    PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when the + modifier is used.
     This sets the y bit of the BO field appropriately.  */
#define BDP BDMA + 1
  { 0xfffc, 0, insert_bdp, extract_bdp,
    PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when the + modifier is used
     and absolute addressing is used.  */
#define BDPA BDP + 1
  { 0xfffc, 0, insert_bdp, extract_bdp,
    PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED },

  /* The BF field in an X or XL form instruction.  */
#define BF BDPA + 1
  /* The CRFD field in an X form instruction.  */
#define CRFD BF
  /* The CRD field in an XL form instruction.  */
#define CRD BF
  { 0x7, 23, NULL, NULL, PPC_OPERAND_CR_REG },

  /* The BF field in an X or XL form instruction.  */
#define BFF BF + 1
  { 0x7, 23, NULL, NULL, 0 },

  /* The ACC field in a VSX ACC 8LS:D-form instruction.  */
#define ACC BFF + 1
  { 0x7, 23, NULL, NULL, PPC_OPERAND_ACC },

  /* The DMR field in a MMA instruction.  */
#define DMR ACC + 1
  { 0x7, 23, NULL, NULL, PPC_OPERAND_DMR },

  /* The second DMR field in a two DMR operand MMA instruction.  */
#define DMRAB DMR + 1
  { 0x7, 13, NULL, NULL, PPC_OPERAND_DMR },

  /* An optional BF field.  This is used for comparison instructions,
     in which an omitted BF field is taken as zero.  */
#define OBF DMRAB + 1
  { 0x7, 23, NULL, NULL, PPC_OPERAND_CR_REG | PPC_OPERAND_OPTIONAL },

  /* The BFA field in an X or XL form instruction.  */
#define BFA OBF + 1
  { 0x7, 18, NULL, NULL, PPC_OPERAND_CR_REG },

  /* The BO field in a B form instruction.  Certain values are
     illegal.  */
#define BO BFA + 1
#define BO_MASK (0x1f << 21)
  { 0x1f, 21, insert_bo, extract_bo, 0 },

  /* The BO field in a B form instruction when the - modifier is used.  */
#define BOM BO + 1
  { 0x1f, 21, insert_bom, extract_bom, 0 },

  /* The BO field in a B form instruction when the + modifier is used.  */
#define BOP BOM + 1
  { 0x1f, 21, insert_bop, extract_bop, 0 },

  /* The RM field in an X form instruction.  */
#define RM BOP + 1
#define DD RM
#define mo1 RM
  { 0x3, 11, NULL, NULL, 0 },

#define BH RM + 1
  { 0x3, 11, NULL, NULL, PPC_OPERAND_OPTIONAL },

  /* The BT field in an X or XL form instruction.  */
#define BT BH + 1
  { 0x1f, 21, NULL, NULL, PPC_OPERAND_CR_BIT },

  /* The BT field in a mtfsb0 or mtfsb1 instruction.  */
#define BTF BT + 1
  { 0x1f, 21, NULL, NULL, PPC_OPERAND_CR_BIT | PPC_OPERAND_CR_REG },

  /* The BI16 field in a BD8 form instruction.  */
#define BI16 BTF + 1
  { 0x3, 8, NULL, NULL, PPC_OPERAND_CR_BIT },

  /* The BI32 field in a BD15 form instruction.  */
#define BI32 BI16 + 1
  { 0xf, 16, NULL, NULL, PPC_OPERAND_CR_BIT },

  /* The BO32 field in a BD15 form instruction.  */
#define BO32 BI32 + 1
  { 0x3, 20, NULL, NULL, 0 },

  /* The B8 field in a BD8 form instruction.  */
#define B8 BO32 + 1
  { 0x1fe, -1, NULL, NULL, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The B15 field in a BD15 form instruction.  The lowest bit is
     forced to zero.  */
#define B15 B8 + 1
  { 0xfffe, 0, NULL, NULL, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The B24 field in a BD24 form instruction.  The lowest bit is
     forced to zero.  */
#define B24 B15 + 1
  { 0x1fffffe, 0, NULL, NULL, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The condition register number portion of the BI field in a B form
     or XL form instruction.  This is used for the extended
     conditional branch mnemonics, which set the lower two bits of the
     BI field.  This field is optional.  */
#define CR B24 + 1
  { 0x7, 18, NULL, NULL, PPC_OPERAND_CR_REG | PPC_OPERAND_OPTIONAL },

  /* The CRB field in an X form instruction.  */
#define CRB CR + 1
  /* The MB field in an M form instruction.  */
#define MB CRB
#define MB_MASK (0x1f << 6)
  { 0x1f, 6, NULL, NULL, 0 },

  /* The CRD32 field in an XL form instruction.  */
#define CRD32 CRB + 1
  { 0x3, 21, NULL, NULL, PPC_OPERAND_CR_REG },

  /* The CRFS field in an X form instruction.  */
#define CRFS CRD32 + 1
  { 0x7, 0, NULL, NULL, PPC_OPERAND_CR_REG },

#define CRS CRFS + 1
  { 0x3, 18, NULL, NULL, PPC_OPERAND_CR_REG | PPC_OPERAND_OPTIONAL },

  /* The CT field in an X form instruction.  */
#define CT CRS + 1
  /* The MO field in an mbar instruction.  */
#define MO CT
  { 0x1f, 21, NULL, NULL, PPC_OPERAND_OPTIONAL },

  /* The TH field in dcbtct.  */
#define THCT CT + 1
  { 0x1f, 21, insert_thct, extract_thct, PPC_OPERAND_OPTIONAL },

  /* The TH field in dcbtds.  */
#define THDS THCT + 1
  { 0x1f, 21, insert_thds, extract_thds, PPC_OPERAND_OPTIONAL },

  /* The D field in a D form instruction.  This is a displacement off
     a register, and implies that the next operand is a register in
     parentheses.  */
#define D THDS + 1
  { 0xffff, 0, NULL, NULL, PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED },

  /* The D8 field in a D form instruction.  This is a displacement off
     a register, and implies that the next operand is a register in
     parentheses.  */
#define D8 D + 1
  { 0xff, 0, NULL, NULL, PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED },

  /* The DCMX field in an X form instruction.  */
#define DCMX D8 + 1
  { 0x7f, 16, NULL, NULL, 0 },

  /* The split DCMX field in an X form instruction.  */
#define DCMXS DCMX + 1
  { 0x7f, PPC_OPSHIFT_INV, insert_dcmxs, extract_dcmxs, 0 },

  /* The DQ field in a DQ form instruction.  This is like D, but the
     lower four bits are forced to zero. */
#define DQ DCMXS + 1
  { 0xfff0, 0, NULL, NULL,
    PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED | PPC_OPERAND_DQ },

  /* The DS field in a DS form instruction.  This is like D, but the
     lower two bits are forced to zero.  */
#define DS DQ + 1
  { 0xfffc, 0, NULL, NULL,
    PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED | PPC_OPERAND_DS },

  /* The D field in an 8-byte D form prefix instruction.  This is a displacement
     off a register, and implies that the next operand is a register in
     parentheses.  */
#define D34 DS + 1
  { UINT64_C(0x3ffffffff), PPC_OPSHIFT_INV, insert_d34, extract_d34,
    PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED },

  /* The SI field in an 8-byte D form prefix instruction.  */
#define SI34 D34 + 1
  { UINT64_C(0x3ffffffff), PPC_OPSHIFT_INV, insert_d34, extract_d34, PPC_OPERAND_SIGNED },

  /* The NSI field in an 8-byte D form prefix instruction.  This is the
     same as the SI34 field, only negated.  */
#define NSI34 SI34 + 1
  { UINT64_C(0x3ffffffff), PPC_OPSHIFT_INV, insert_nsi34, extract_nsi34,
    PPC_OPERAND_NEGATIVE | PPC_OPERAND_SIGNED },

  /* The IMM32 field in a vector splat immediate prefix instruction.  */
#define IMM32 NSI34 + 1
  { 0xffffffff, PPC_OPSHIFT_INV, insert_imm32, extract_imm32, 0},

  /* The UIM field in a vector permute extended prefix instruction.  */
#define UIM3 IMM32 + 1
  { 0x7, 32, NULL, NULL, 0},

  /* The UIM field in a vector eval prefix instruction.  */
#define UIM8 UIM3 + 1
  { 0xff, 32, NULL, NULL, 0},

  /* The IX field in xxsplti32dx.  */
#define IX UIM8 + 1
  { 0x1, 17, NULL, NULL, 0 },

  /* The PMSK field in GER rank 8 prefix instructions.  */
#define PMSK8 IX + 1
  { 0xff, 40, NULL, NULL, 0 },

  /* The PMSK field in GER rank 4 prefix instructions.  */
#define PMSK4 PMSK8 + 1
  { 0xf, 44, NULL, NULL, 0 },

  /* The PMSK field in GER rank 2 prefix instructions.  */
#define PMSK2 PMSK4 + 1
  { 0x3, 46, NULL, NULL, 0 },

  /* The XMSK field in GER prefix instructions.  */
#define XMSK PMSK2 + 1
  { 0xf, 36, NULL, NULL, 0 },

  /* The XMSK field in GERX prefix instructions.  */
#define XMSK8 XMSK + 1
  { 0xff, 36, NULL, NULL, 0 },

  /* The YMSK field in GER prefix instructions.  */
#define YMSK XMSK8 + 1
  { 0xf, 32, NULL, NULL, 0 },

  /* The YMSK field in 64-bit GER prefix instructions.  */
#define YMSK2 YMSK + 1
  { 0x3, 34, NULL, NULL, 0 },

  /* The DUIS or BHRBE fields in a XFX form instruction, 10 bits
     unsigned imediate */
#define DUIS YMSK2 + 1
#define BHRBE DUIS
  { 0x3ff, 11, NULL, NULL, 0 },

  /* The split DW field in a X form instruction.  */
#define DW DUIS + 1
  { -1, PPC_OPSHIFT_INV, insert_dw, extract_dw,
    PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED},

  /* The split D field in a DX form instruction.  */
#define DXD DW + 1
  { 0xffff, PPC_OPSHIFT_INV, insert_dxd, extract_dxd,
    PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT},

  /* The split ND field in a DX form instruction.
     This is the same as the DX field, only negated.  */
#define NDXD DXD + 1
  { 0xffff, PPC_OPSHIFT_INV, insert_dxdn, extract_dxdn,
    PPC_OPERAND_NEGATIVE | PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT},

  /* The E field in a wrteei instruction.  */
  /* And the W bit in the pair singles instructions.  */
  /* And the ST field in a VX form instruction.  */
#define E NDXD + 1
#define PSW E
#define ST E
  { 0x1, 15, NULL, NULL, 0 },

  /* The FL1 field in a POWER SC form instruction.  */
#define FL1 E + 1
  /* The U field in an X form instruction.  */
#define U FL1
  { 0xf, 12, NULL, NULL, 0 },

  /* The FL2 field in a POWER SC form instruction.  */
#define FL2 FL1 + 1
  { 0x7, 2, NULL, NULL, 0 },

  /* The FLM field in an XFL form instruction.  */
#define FLM FL2 + 1
  { 0xff, 17, NULL, NULL, 0 },

  /* The FRA field in an X or A form instruction.  */
#define FRA FLM + 1
#define FRA_MASK (0x1f << 16)
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_FPR },

  /* The FRAp field of DFP instructions.  */
#define FRAp FRA + 1
  { 0x1e, 16, NULL, NULL, PPC_OPERAND_FPR },

  /* The FRB field in an X or A form instruction.  */
#define FRB FRAp + 1
#define FRB_MASK (0x1f << 11)
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_FPR },

  /* The FRBp field of DFP instructions.  */
#define FRBp FRB + 1
  { 0x1e, 11, NULL, NULL, PPC_OPERAND_FPR },

  /* The FRC field in an A form instruction.  */
#define FRC FRBp + 1
#define FRC_MASK (0x1f << 6)
  { 0x1f, 6, NULL, NULL, PPC_OPERAND_FPR },

  /* The FRS field in an X form instruction or the FRT field in a D, X
     or A form instruction.  */
#define FRS FRC + 1
#define FRT FRS
  { 0x1f, 21, NULL, NULL, PPC_OPERAND_FPR },

  /* The FRSp field of stfdp or the FRTp field of lfdp and DFP
     instructions.  */
#define FRSp FRS + 1
#define FRTp FRSp
  { 0x1e, 21, NULL, NULL, PPC_OPERAND_FPR },

  /* The FXM field in an XFX instruction.  */
#define FXM FRSp + 1
  { 0xff, 12, insert_fxm, extract_fxm, 0 },

  /* Power4 version for mfcr.  */
#define FXM4 FXM + 1
  { 0xff, 12, insert_fxm, extract_fxm, PPC_OPERAND_OPTIONAL },

  /* The IMM20 field in an LI instruction.  */
#define IMM20 FXM4 + 1
  { 0xfffff, PPC_OPSHIFT_INV, insert_li20, extract_li20, PPC_OPERAND_SIGNED},

  /* The L field in a D or X form instruction.  */
#define L IMM20 + 1
  { 0x1, 21, NULL, NULL, 0 },

  /* The optional L field in tlbie and tlbiel instructions.  */
#define LOPT L + 1
  /* The R field in a HTM X form instruction.  */
#define HTM_R LOPT
  { 0x1, 21, NULL, NULL, PPC_OPERAND_OPTIONAL },

  /* The optional L field in the paste. instruction. This is similar to LOPT
     above, but with a default value of 1.  */
#define L1OPT LOPT + 1
  { 0x1, 21, insert_l1opt, extract_l1opt, PPC_OPERAND_OPTIONAL },

  /* The optional (for 32-bit) L field in cmp[l][i] instructions.  */
#define L32OPT L1OPT + 1
  { 0x1, 21, NULL, NULL, PPC_OPERAND_OPTIONAL | PPC_OPERAND_OPTIONAL32 },

  /* The 2-bit L or WC field in an X (sync, dcbf or wait) form instruction.  */
#define L2OPT L32OPT + 1
#define LS L2OPT
#define WC L2OPT
  { 0x3, 21, insert_ls, extract_ls, PPC_OPERAND_OPTIONAL },

  /* The LEV field in a POWER SVC / POWER9 SCV form instruction.  */
#define SVC_LEV L2OPT + 1
  { 0x7f, 5, NULL, NULL, 0 },

  /* The LEV field in an SC form instruction.  */
#define LEV SVC_LEV + 1
  { 0x7f, 5, NULL, NULL, PPC_OPERAND_OPTIONAL },

  /* The LI field in an I form instruction.  The lower two bits are
     forced to zero.  */
#define LI LEV + 1
  { 0x3fffffc, 0, NULL, NULL, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The LI field in an I form instruction when used as an absolute
     address.  */
#define LIA LI + 1
  { 0x3fffffc, 0, NULL, NULL, PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED },

  /* The 3-bit L field in a sync or dcbf instruction.  */
#define LS3 LIA + 1
#define L3OPT LS3
  { 0x7, 21, insert_ls, extract_ls, PPC_OPERAND_OPTIONAL },

  /* The ME field in an M form instruction.  */
#define ME LS3 + 1
#define ME_MASK (0x1f << 1)
  { 0x1f, 1, NULL, NULL, 0 },

#define CRWn ME + 1
  { 0x1f, 1, insert_crwn, extract_crwn, 0 },

#define ELWn CRWn + 1
  { 0x1f, 1, insert_elwn, extract_elwn, PPC_OPERAND_PLUS1 },

#define ERWn ELWn + 1
  { 0x1f, 6, insert_erwn, extract_erwn, 0 },

#define ERWb ERWn + 1
  { 0x1f, 11, insert_erwb, extract_erwb, 0 },

#define CSLWb ERWb + 1
  { 0x1f, 6, NULL, extract_cslwb, 0 },

#define CSLWn CSLWb + 1
  { 0x1f, 11, insert_cslwn, NULL, 0 },

#define ILWn CSLWn + 1
  { 0x1f, 1, NULL, extract_ilwn, PPC_OPERAND_PLUS1 },

#define ILWb ILWn + 1
  { 0x1f, 6, insert_ilwb, NULL, 0 },

#define IRWn ILWb + 1
  { 0x1f, 1, NULL, extract_irwn, PPC_OPERAND_PLUS1 },

#define IRWb IRWn + 1
  { 0x1f, 6, insert_irwb, NULL, 0 },

  /* The MB and ME fields in an M form instruction expressed a single
     operand which is a bitmask indicating which bits to select.  This
     is a two operand form using PPC_OPERAND_NEXT.  See the
     description in opcode/ppc.h for what this means.  */
#define MBE IRWb + 1
  { 0x1f, 6, NULL, NULL, PPC_OPERAND_OPTIONAL | PPC_OPERAND_NEXT },
  { -1, 0, insert_mbe, extract_mbe, 0 },

  /* The MB or ME field in an MD or MDS form instruction.  The high
     bit is wrapped to the low end.  */
#define MB6 MBE + 2
#define ME6 MB6
#define MB6_MASK (0x3f << 5)
  { 0x3f, 5, insert_mb6, extract_mb6, 0 },

#define ELDn MB6 + 1
  { 0x3f, 5, insert_eldn, extract_eldn, PPC_OPERAND_PLUS1 },

#define ERDn ELDn + 1
  { 0x3f, 5, insert_erdn, extract_erdn, 0 },

#define CRDn ERDn + 1
  { 0x3f, 5, insert_crdn, extract_crdn, 0 },

  /* The NB field in an X form instruction.  The value 32 is stored as
     0.  */
#define NB CRDn + 1
  { 0x1f, 11, NULL, extract_nb, PPC_OPERAND_PLUS1 },

  /* The NBI field in an lswi instruction, which has special value
     restrictions.  The value 32 is stored as 0.  */
#define NBI NB + 1
  { 0x1f, 11, insert_nbi, extract_nb, PPC_OPERAND_PLUS1 },

  /* The NSI field in a D form instruction.  This is the same as the
     SI field, only negated.  */
#define NSI NBI + 1
  { 0xffff, 0, insert_nsi, extract_nsi,
    PPC_OPERAND_NEGATIVE | PPC_OPERAND_SIGNED },

  /* The NSI field in a D form instruction when we accept a wide range
     of positive values.  */
#define NSISIGNOPT NSI + 1
  { 0xffff, 0, insert_nsi, extract_nsi,
    PPC_OPERAND_NEGATIVE | PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT },

  /* The RA field in an D, DS, DQ, X, XO, M, or MDS form instruction.  */
#define RA NSISIGNOPT + 1
#define RA_MASK (0x1f << 16)
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_GPR },

  /* As above, but 0 in the RA field means zero, not r0.  */
#define RA0 RA + 1
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_GPR_0 },

  /* Similar to above, but optional.  */
#define PRA0 RA0 + 1
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_GPR_0 | PPC_OPERAND_OPTIONAL },

  /* The RA field in the DQ form lq or an lswx instruction, which have
     special value restrictions.  */
#define RAQ PRA0 + 1
#define RAX RAQ
  { 0x1f, 16, insert_raq, extract_raq, PPC_OPERAND_GPR_0 },

  /* Similar to above, but optional.  */
#define PRAQ RAQ + 1
  { 0x1f, 16, insert_raq, extract_raq,
    PPC_OPERAND_GPR_0 | PPC_OPERAND_OPTIONAL },

  /* The R field in an 8-byte D, DS, DQ or X form prefix instruction.  */
#define PCREL PRAQ + 1
#define PCREL_MASK (1ULL << 52)
  { 0x1, 52, insert_pcrel, extract_pcrel, PPC_OPERAND_OPTIONAL },

#define PCREL1 PCREL + 1
  { 0x1, 52, insert_pcrel, extract_pcrel1, PPC_OPERAND_OPTIONAL },

  /* The RA field in a D or X form instruction which is an updating
     load, which means that the RA field may not be zero and may not
     equal the RT field.  */
#define RAL PCREL1 + 1
  { 0x1f, 16, insert_ral, extract_ral, PPC_OPERAND_GPR_0 },

  /* The RA field in an lmw instruction, which has special value
     restrictions.  */
#define RAM RAL + 1
  { 0x1f, 16, insert_ram, extract_ram, PPC_OPERAND_GPR_0 },

  /* The RA field in a D or X form instruction which is an updating
     store or an updating floating point load, which means that the RA
     field may not be zero.  */
#define RAS RAM + 1
  { 0x1f, 16, insert_ras, extract_ras, PPC_OPERAND_GPR_0 },

  /* The RA field of the tlbwe, dccci and iccci instructions,
     which are optional.  */
#define RAOPT RAS + 1
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_GPR | PPC_OPERAND_OPTIONAL },

  /* The RB field in an X, XO, M, or MDS form instruction.  */
#define RB RAOPT + 1
#define RB_MASK (0x1f << 11)
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_GPR },

  /* The RS and RB fields in an X form instruction when they must be the same.
     This is used for extended mnemonics like mr.  */
#define RSB RB + 1
  { 0x1f, 11, insert_rsb, extract_rsb, PPC_OPERAND_GPR },

  /* The RB field in an lswx instruction, which has special value
     restrictions.  */
#define RBX RSB + 1
  { 0x1f, 11, insert_rbx, extract_rbx, PPC_OPERAND_GPR },

  /* The RB field of the dccci and iccci instructions, which are optional.  */
#define RBOPT RBX + 1
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_GPR | PPC_OPERAND_OPTIONAL },

  /* The RC register field in an maddld, maddhd or maddhdu instruction.  */
#define RC RBOPT + 1
  { 0x1f, 6, NULL, NULL, PPC_OPERAND_GPR },

  /* The RS field in a D, DS, X, XFX, XS, M, MD or MDS form
     instruction or the RT field in a D, DS, X, XFX or XO form
     instruction.  */
#define RS RC + 1
#define RT RS
#define RT_MASK (0x1f << 21)
#define RD RS
  { 0x1f, 21, NULL, NULL, PPC_OPERAND_GPR },

#define RD_EVEN RS + 1
#define RS_EVEN RD_EVEN
  { 0x1f, 21, insert_rD_rS_even, extract_rD_rS_even, PPC_OPERAND_GPR },

  /* The RS and RT fields of the DS form stq and DQ form lq instructions,
     which have special value restrictions.  */
#define RSQ RS_EVEN + 1
#define RTQ RSQ
#define Q_MASK (1 << 21)
  { 0x1e, 21, NULL, NULL, PPC_OPERAND_GPR },

  /* The RS field of the tlbwe instruction, which is optional.  */
#define RSO RSQ + 1
#define RTO RSO
  { 0x1f, 21, NULL, NULL, PPC_OPERAND_GPR | PPC_OPERAND_OPTIONAL },

  /* The RX field of the SE_RR form instruction.  */
#define RX RSO + 1
  { 0x1f, PPC_OPSHIFT_INV, insert_rx, extract_rx, PPC_OPERAND_GPR },

  /* The ARX field of the SE_RR form instruction.  */
#define ARX RX + 1
  { 0x1f, PPC_OPSHIFT_INV, insert_arx, extract_arx, PPC_OPERAND_GPR },

  /* The RY field of the SE_RR form instruction.  */
#define RY ARX + 1
#define RZ RY
  { 0x1f, PPC_OPSHIFT_INV, insert_ry, extract_ry, PPC_OPERAND_GPR },

  /* The ARY field of the SE_RR form instruction.  */
#define ARY RY + 1
  { 0x1f, PPC_OPSHIFT_INV, insert_ary, extract_ary, PPC_OPERAND_GPR },

  /* The SCLSCI8 field in a D form instruction.  */
#define SCLSCI8 ARY + 1
  { 0xffffffff, PPC_OPSHIFT_INV, insert_sci8, extract_sci8, 0 },

  /* The SCLSCI8N field in a D form instruction.  This is the same as the
     SCLSCI8 field, only negated.  */
#define SCLSCI8N SCLSCI8 + 1
  { 0xffffffff, PPC_OPSHIFT_INV, insert_sci8n, extract_sci8n,
    PPC_OPERAND_NEGATIVE | PPC_OPERAND_SIGNED },

  /* The SD field of the SD4 form instruction.  */
#define SE_SD SCLSCI8N + 1
  { 0xf, 8, NULL, NULL, PPC_OPERAND_PARENS },

  /* The SD field of the SD4 form instruction, for halfword.  */
#define SE_SDH SE_SD + 1
  { 0x1e, 7, NULL, NULL, PPC_OPERAND_PARENS },

  /* The SD field of the SD4 form instruction, for word.  */
#define SE_SDW SE_SDH + 1
  { 0x3c, 6, NULL, NULL, PPC_OPERAND_PARENS },

  /* The SH field in an X or M form instruction.  */
#define SH SE_SDW + 1
#define SH_MASK (0x1f << 11)
  /* The other UIMM field in a EVX form instruction.  */
#define EVUIMM SH
  /* The FC field in an atomic X form instruction.  */
#define FC SH
#define UIM5 SH
  { 0x1f, 11, NULL, NULL, 0 },

#define RRWn SH + 1
  { 0x1f, 11, insert_rrwn, extract_rrwn, 0 },

#define SLWn RRWn + 1
  { 0x1f, 11, insert_slwn, extract_slwn, 0 },

#define SRWn SLWn + 1
  { 0x1f, 11, insert_srwn, extract_srwn, 0 },

#define EVUIMM_LT8 SRWn + 1
  { 0x1f, 11, insert_evuimm_lt8, extract_evuimm_lt8, 0 },

#define EVUIMM_LT16 EVUIMM_LT8 + 1
  { 0x1f, 11, insert_evuimm_lt16, extract_evuimm_lt16, 0 },

  /* The SI field in a HTM X form instruction.  */
#define HTM_SI EVUIMM_LT16 + 1
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_SIGNED },

  /* The SH field in an MD form instruction.  This is split.  */
#define SH6 HTM_SI + 1
#define SH6_MASK ((0x1f << 11) | (1 << 1))
  { 0x3f, PPC_OPSHIFT_INV, insert_sh6, extract_sh6, 0 },

#define RRDn SH6 + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_rrdn, extract_rrdn, 0 },

#define SLDn RRDn + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_sldn, extract_sldn, 0 },

#define SRDn SLDn + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_srdn, extract_srdn, 0 },

#define ERDb SRDn + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_erdb, extract_erdb, 0 },

#define CSLDn ERDb + 1
  { 0x3f, PPC_OPSHIFT_SH6, insert_csldn, extract_sh6, 0 },

#define CSLDb CSLDn + 1
  { 0x3f, 5, insert_mb6, extract_csldb, 0 },

#define IRDn CSLDb + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_sh6, extract_irdn, PPC_OPERAND_PLUS1 },

#define IRDb IRDn + 1
  { 0x3f, 5, insert_irdb, extract_mb6, 0 },

  /* The SH field of some variants of the tlbre and tlbwe
     instructions, and the ELEV field of the e_sc instruction.  */
#define SHO IRDb + 1
#define ELEV SHO
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_OPTIONAL },

  /* The SI field in a D form instruction.  */
#define SI SHO + 1
  { 0xffff, 0, NULL, NULL, PPC_OPERAND_SIGNED },

  /* The SI field in a D form instruction when we accept a wide range
     of positive values.  */
#define SISIGNOPT SI + 1
  { 0xffff, 0, NULL, NULL, PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT },

  /* The SI8 field in a D form instruction.  */
#define SI8 SISIGNOPT + 1
  { 0xff, 0, NULL, NULL, PPC_OPERAND_SIGNED },

  /* The SPR field in an XFX form instruction.  This is flipped--the
     lower 5 bits are stored in the upper 5 and vice- versa.  */
#define SPR SI8 + 1
#define PMR SPR
#define TMR SPR
#define SPR_MASK (0x3ff << 11)
  { 0x3ff, 11, insert_spr, extract_spr, PPC_OPERAND_SPR },

  /* The BAT index number in an XFX form m[ft]ibat[lu] instruction.  */
#define SPRBAT SPR + 1
#define SPRBAT_MASK (0xc1 << 11)
  { 0x7, PPC_OPSHIFT_INV, insert_sprbat, extract_sprbat, PPC_OPERAND_SPR },

  /* The GQR index number in an XFX form m[ft]gqr instruction.  */
#define SPRGQR SPRBAT + 1
#define SPRGQR_MASK (0x7 << 16)
  { 0x7, 16, NULL, NULL, PPC_OPERAND_GQR },

  /* The SPRG register number in an XFX form m[ft]sprg instruction.  */
#define SPRG SPRGQR + 1
  { 0x1f, 16, insert_sprg, extract_sprg, PPC_OPERAND_SPR },

  /* The SR field in an X form instruction.  */
#define SR SPRG + 1
  /* The 4-bit UIMM field in a VX form instruction.  */
#define UIMM4 SR
  { 0xf, 16, NULL, NULL, 0 },

  /* The STRM field in an X AltiVec form instruction.  */
#define STRM SR + 1
  /* The T field in a tlbilx form instruction.  */
#define T STRM
  /* The L field in wclr instructions.  */
#define L2 STRM
  { 0x3, 21, NULL, NULL, 0 },

  /* The ESYNC field in an X (sync) form instruction.  */
#define ESYNC STRM + 1
  { 0xf, 16, insert_esync, extract_esync, PPC_OPERAND_OPTIONAL },

  /* The SV field in a POWER SC form instruction.  */
#define SV ESYNC + 1
  { 0x3fff, 2, NULL, NULL, 0 },

  /* The TBR field in an XFX form instruction.  This is like the SPR
     field, but it is optional.  */
#define TBR SV + 1
  { 0x3ff, 11, insert_tbr, extract_tbr,
    PPC_OPERAND_SPR | PPC_OPERAND_OPTIONAL },

  /* The TO field in a D or X form instruction.  */
#define TO TBR + 1
#define DUI TO
#define SVme TO
#define SVG TO
#define TO_MASK (0x1f << 21)
  { 0x1f, 21, NULL, NULL, 0 },

  /* The UI field in a D form instruction.  */
#define UI TO + 1
  { 0xffff, 0, NULL, NULL, 0 },

#define UISIGNOPT UI + 1
  { 0xffff, 0, NULL, NULL, PPC_OPERAND_SIGNOPT },

  /* The IMM field in an SE_IM5 instruction.  */
#define UI5 UISIGNOPT + 1
  { 0x1f, 4, NULL, NULL, 0 },

  /* The OIMM field in an SE_OIM5 instruction.  */
#define OIMM5 UI5 + 1
  { 0x1f, 4, insert_oimm, extract_oimm, PPC_OPERAND_PLUS1 },

  /* The UI7 field in an SE_LI instruction.  */
#define UI7 OIMM5 + 1
  { 0x7f, 4, NULL, NULL, 0 },

  /* The VA field in a VA, VX or VXR form instruction.  */
#define VA UI7 + 1
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_VR },

  /* The VB field in a VA, VX or VXR form instruction.  */
#define VB VA + 1
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_VR },

  /* The VC field in a VA form instruction.  */
#define VC VB + 1
  { 0x1f, 6, NULL, NULL, PPC_OPERAND_VR },

  /* The VD or VS field in a VA, VX, VXR or X form instruction.  */
#define VD VC + 1
#define VS VD
  { 0x1f, 21, NULL, NULL, PPC_OPERAND_VR },

  /* The SIMM field in a VX form instruction, and TE in Z form.  */
#define SIMM VD + 1
#define TE SIMM
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_SIGNED},

  /* The UIMM field in a VX form instruction.  */
#define UIMM SIMM + 1
#define DCTL UIMM
#define rmm UIMM
  { 0x1f, 16, NULL, NULL, 0 },

  /* The 3-bit UIMM field in a VX form instruction.  */
#define UIMM3 UIMM + 1
  { 0x7, 16, NULL, NULL, 0 },

  /* The 6-bit UIM field in a X form instruction.  */
#define UIM6 UIMM3 + 1
  { 0x3f, 16, NULL, NULL, 0 },

  /* The SIX field in a VX form instruction.  */
#define SIX UIM6 + 1
#define MMMM SIX
  { 0xf, 11, NULL, NULL, 0 },

  /* The PS field in a VX form instruction.  */
#define PS SIX + 1
  { 0x1, 9, NULL, NULL, 0 },

  /* The SH field in a vector shift double by bit immediate instruction.  */
#define SH3 PS + 1
  { 0x7, 6, NULL, NULL, 0 },

  /* The SHB field in a VA form instruction.  */
#define SHB SH3 + 1
  { 0xf, 6, NULL, NULL, 0 },

  /* The other UIMM field in a half word EVX form instruction.  */
#define EVUIMM_1 SHB + 1
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_PARENS },

#define EVUIMM_1_EX0 EVUIMM_1 + 1
  { 0x1f, 11, insert_evuimm1_ex0, extract_evuimm1_ex0, PPC_OPERAND_PARENS },

#define EVUIMM_2 EVUIMM_1_EX0 + 1
  { 0x3e, 10, NULL, NULL, PPC_OPERAND_PARENS },

#define EVUIMM_2_EX0 EVUIMM_2 + 1
  { 0x3e, 10, insert_evuimm2_ex0, extract_evuimm2_ex0, PPC_OPERAND_PARENS },

  /* The other UIMM field in a word EVX form instruction.  */
#define EVUIMM_4 EVUIMM_2_EX0 + 1
  { 0x7c, 9, NULL, NULL, PPC_OPERAND_PARENS },

#define EVUIMM_4_EX0 EVUIMM_4 + 1
  { 0x7c, 9, insert_evuimm4_ex0, extract_evuimm4_ex0, PPC_OPERAND_PARENS },

  /* The other UIMM field in a double EVX form instruction.  */
#define EVUIMM_8 EVUIMM_4_EX0 + 1
  { 0xf8, 8, NULL, NULL, PPC_OPERAND_PARENS },

#define EVUIMM_8_EX0 EVUIMM_8 + 1
  { 0xf8, 8, insert_evuimm8_ex0, extract_evuimm8_ex0, PPC_OPERAND_PARENS },

  /* The WS or DRM field in an X form instruction.  */
#define WS EVUIMM_8_EX0 + 1
#define DRM WS
  /* The NNN field in a VX form instruction for SPE2  */
#define NNN WS
  { 0x7, 11, NULL, NULL, 0 },

  /* PowerPC paired singles extensions.  */
  /* W bit in the pair singles instructions for x type instructions.  */
#define PSWM WS + 1
  /* The BO16 field in a BD8 form instruction.  */
#define BO16 PSWM
  /* The pst field in a SVRM form instruction.  */
#define pst PSWM
  /* The L field in a XO form instruction.  */
#define XOL PSWM
  {  0x1, 10, 0, 0, 0 },

  /* IDX bits for quantization in the pair singles instructions.  */
#define PSQ PSWM + 1
  {  0x7, 12, 0, 0, PPC_OPERAND_GQR },

  /* IDX bits for quantization in the pair singles x-type instructions.  */
#define PSQM PSQ + 1
  {  0x7, 7, 0, 0, PPC_OPERAND_GQR },

  /* Smaller D field for quantization in the pair singles instructions.  */
#define PSD PSQM + 1
  {  0xfff, 0, 0, 0,  PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED },

  /* The L field in an mtmsrd or A form instruction or R or W in an
     X form.  */
#define A_L PSD + 1
#define W A_L
#define X_R A_L
  { 0x1, 16, NULL, NULL, PPC_OPERAND_OPTIONAL },

  /* The RMC or CY field in a Z23 form instruction.  */
#define RMC A_L + 1
#define CY RMC
#define ew RMC
  { 0x3, 9, NULL, NULL, 0 },

#define R RMC + 1
#define MP R
#define P1 R
  { 0x1, 16, NULL, NULL, 0 },

#define RIC R + 1
  { 0x3, 18, NULL, NULL, PPC_OPERAND_OPTIONAL },

#define PRS RIC + 1
  { 0x1, 17, NULL, NULL, PPC_OPERAND_OPTIONAL },

#define SP PRS + 1
#define mi0 SP
  { 0x3, 19, NULL, NULL, 0 },

#define S SP + 1
  { 0x1, 20, NULL, NULL, 0 },

  /* The S field in a XL form instruction.  */
#define SXL S + 1
  { 0x1, 11, insert_sxl, extract_sxl, PPC_OPERAND_OPTIONAL },

  /* SH field starting at bit position 16.  */
#define SH16 SXL + 1
  /* The DCM and DGM fields in a Z form instruction.  */
#define DCM SH16
#define DGM DCM
  { 0x3f, 10, NULL, NULL, 0 },

  /* The EH field in larx instruction.  */
#define EH SH16 + 1
  { 0x1, 0, NULL, NULL, PPC_OPERAND_OPTIONAL },

  /* The L field in an mtfsf or XFL form instruction.  */
  /* The A field in a HTM X form instruction.  */
#define XFL_L EH + 1
#define HTM_A XFL_L
  { 0x1, 25, NULL, NULL, PPC_OPERAND_OPTIONAL},

  /* Xilinx APU related masks and macros */
#define FCRT XFL_L + 1
#define FCRT_MASK (0x1f << 21)
  { 0x1f, 21, 0, 0, PPC_OPERAND_FCR },

  /* Xilinx FSL related masks and macros */
#define FSL FCRT + 1
#define FSL_MASK (0x1f << 11)
  { 0x1f, 11, 0, 0, PPC_OPERAND_FSL },

  /* Xilinx UDI related masks and macros */
#define URT FSL + 1
  { 0x1f, 21, 0, 0, PPC_OPERAND_UDI },

#define URA URT + 1
  { 0x1f, 16, 0, 0, PPC_OPERAND_UDI },

#define URB URA + 1
  { 0x1f, 11, 0, 0, PPC_OPERAND_UDI },

#define URC URB + 1
  { 0x1f, 6, 0, 0, PPC_OPERAND_UDI },

  /* The VLESIMM field in a D form instruction.  */
#define VLESIMM URC + 1
  { 0xffff, PPC_OPSHIFT_INV, insert_vlesi, extract_vlesi,
    PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT },

  /* The VLENSIMM field in a D form instruction.  */
#define VLENSIMM VLESIMM + 1
  { 0xffff, PPC_OPSHIFT_INV, insert_vlensi, extract_vlensi,
    PPC_OPERAND_NEGATIVE | PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT },

  /* The VLEUIMM field in a D form instruction.  */
#define VLEUIMM VLENSIMM + 1
  { 0xffff, PPC_OPSHIFT_INV, insert_vleui, extract_vleui, 0 },

  /* The VLEUIMML field in a D form instruction.  */
#define VLEUIMML VLEUIMM + 1
  { 0xffff, PPC_OPSHIFT_INV, insert_vleil, extract_vleil, 0 },

  /* The XT and XS fields in an XX1 or XX3 form instruction.  This is
     split.  */
#define XS6 VLEUIMML + 1
#define XT6 XS6
  { 0x3f, PPC_OPSHIFT_INV, insert_xt6, extract_xt6, PPC_OPERAND_VSR },

  /* The XT and XS fields in an DQ form VSX instruction.  This is split.  */
#define XSQ6 XT6 + 1
#define XTQ6 XSQ6
  { 0x3f, PPC_OPSHIFT_INV, insert_xtq6, extract_xtq6, PPC_OPERAND_VSR },

  /* The split XTp and XSp field in a vector paired instruction.  */
#define XTP XSQ6 + 1
#define XSP XTP
  { 0x3e, PPC_OPSHIFT_INV, insert_xtp, extract_xtp, PPC_OPERAND_VSR },

#define XTS XTP + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_xts, extract_xts, PPC_OPERAND_VSR },

  /* The XT field in a plxv instruction.  Runs into the OP field.  */
#define XTOP XTS + 1
  { 0x3f, 21, NULL, NULL, PPC_OPERAND_VSR },

  /* The XA field in an XX3 form instruction.  This is split.  */
#define XA6 XTOP + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_xa6, extract_xa6, PPC_OPERAND_VSR },

  /* The XA field in an MMA XX3 form instruction.  This is split and
     must not overlap with the ACC operand.  */
#define XA6a XA6 + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_xa6a, extract_xa6a, PPC_OPERAND_VSR },

  /* The XAp field in an MMA XX3 form instruction.  This is split.
     This is like XA6a, but must be even.  */
#define XA6ap XA6a + 1
  { 0x3e, PPC_OPSHIFT_INV, insert_xa6a, extract_xa6a, PPC_OPERAND_VSR },

  /* The 5-bit XAp field in an MMA XX3 form instruction.  This is split.
     This is like XA6, but must be even.  */
#define XA5p XA6ap + 1
  { 0x3e, PPC_OPSHIFT_INV, insert_xa5, extract_xa5, PPC_OPERAND_VSR },

  /* The XB field in an XX2 or XX3 form instruction.  This is split.  */
#define XB6 XA5p + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_xb6, extract_xb6, PPC_OPERAND_VSR },

  /* The XB field in an XX3 form instruction.  This is split and
     must not overlap with the ACC operand.  */
#define XB6a XB6 + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_xb6a, extract_xb6a, PPC_OPERAND_VSR },

  /* The 5-bit XBp field in an MMA XX3 form instruction.  This is split.
     This is like XB6, but must be even.  */
#define XB5p XB6a + 1
  { 0x3e, PPC_OPSHIFT_INV, insert_xb5, extract_xb5, PPC_OPERAND_VSR },

  /* The XA and XB fields in an XX3 form instruction when they must be the same.
     This is used in extended mnemonics like xvmovdp.  This is split.  */
#define XAB6 XB5p + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_xab6, extract_xab6, PPC_OPERAND_VSR },

  /* The XC field in an XX4 form instruction.  This is split.  */
#define XC6 XAB6 + 1
  { 0x3f, PPC_OPSHIFT_INV, insert_xc6, extract_xc6, PPC_OPERAND_VSR },

  /* The DM or SHW field in an XX3 form instruction.  */
#define DM XC6 + 1
#define SHW DM
  { 0x3, 8, NULL, NULL, 0 },

  /* The DM field in an extended mnemonic XX3 form instruction.  */
#define DMEX DM + 1
  { 0x3, 8, insert_dm, extract_dm, 0 },

  /* The UIM field in an XX2 form instruction.  */
#define UIM DMEX + 1
  /* The 2-bit UIMM field in a VX form instruction.  */
#define UIMM2 UIM
  /* The 2-bit L field in a darn instruction.  */
#define LRAND UIM
  { 0x3, 16, NULL, NULL, 0 },

#define ERAT_T UIM + 1
  { 0x7, 21, NULL, NULL, 0 },

#define IH ERAT_T + 1
  { 0x7, 21, NULL, NULL, PPC_OPERAND_OPTIONAL },

  /* The 2-bit SC or PL field in an X form instruction.  */
#define SC2 IH + 1
#define PL SC2
  { 0x3, 16, insert_pl, extract_pl, PPC_OPERAND_OPTIONAL },

#define P2 PL + 1
  { 0x3, PPC_OPSHIFT_INV, insert_p2, extract_p2, 0 },

  /* The 8-bit IMM8 field in a XX1 form instruction.  */
#define IMM8 P2 + 1
  { 0xff, 11, NULL, NULL, PPC_OPERAND_SIGNOPT },

#define VX_OFF IMM8 + 1
  { 0x3, 0, insert_off_lsp, extract_off_lsp, 0 },

#define VX_OFF_SPE2 VX_OFF + 1
  { 0x7, 0, insert_off_spe2, extract_off_spe2, 0 },

#define BBB VX_OFF_SPE2 + 1
  { 0x7, 13, NULL, NULL, 0 },

#define DDD BBB + 1
#define VX_MASK_DDD  (VX_MASK & ~0x1)
  { 0x7, PPC_OPSHIFT_INV, insert_Ddd, extract_Ddd, 0 },

#define HH DDD + 1
#define mo0 HH
  { 0x3, 13, NULL, NULL, 0 },

#define SVi HH + 1
  { 0x3f, 9, NULL, NULL, PPC_OPERAND_NONZERO },

#define vf SVi + 1
#define sk vf
  { 0x1, 6, NULL, NULL, 0 },

#define vs vf + 1
#define mm vs
  { 0x1, 7, NULL, NULL, 0 },

#define ms vs + 1
#define yx ms
  { 0x1, 8, NULL, NULL, 0 },

#define SVLcr ms + 1
  { 0x1, 5, NULL, NULL, 0 },

#define SVxd SVLcr + 1
  { 0x1f, 21, NULL, NULL, PPC_OPERAND_NONZERO },

#define SVyd SVxd + 1
  { 0x1f, 16, NULL, NULL, PPC_OPERAND_NONZERO },

#define SVzd SVyd + 1
#define SVd SVzd
  { 0x1f, 11, NULL, NULL, PPC_OPERAND_NONZERO },

#define SVrm SVzd + 1
  { 0xf, 7, NULL, NULL, 0 },

#define mi1 SVrm + 1
  { 0x3, 17, NULL, NULL, 0 },

#define mi2 mi1 + 1
  { 0x3, 15, NULL, NULL, 0 },
};

const unsigned int num_powerpc_operands = ARRAY_SIZE (powerpc_operands);

/* Macros used to form opcodes.  */

/* The main opcode.  */
#define OP(x) ((((uint64_t)(x)) & 0x3f) << 26)
#define OP_MASK OP (0x3f)

/* The prefix opcode.  */
#define PREFIX_OP (1ULL << 58)

/* The 2-bit prefix form.  */
#define PREFIX_FORM(x) ((x & 3ULL) << 56)

#define SUFFIX_MASK ((1ULL << 32) - 1)
#define PREFIX_MASK (SUFFIX_MASK << 32)

/* Prefix insn, eight byte load/store form 8LS.  */
#define P8LS (PREFIX_OP | PREFIX_FORM (0))

/* Prefix insn, eight byte register to register form 8RR.  */
#define P8RR (PREFIX_OP | PREFIX_FORM (1))

/* Prefix insn, modified load/store form MLS.  */
#define PMLS (PREFIX_OP | PREFIX_FORM (2))

/* Prefix insn, modified register to register form MRR.  */
#define PMRR (PREFIX_OP | PREFIX_FORM (3))

/* Prefix insn, modified masked immediate register to register form MMIRR.  */
#define PMMIRR (PREFIX_OP | PREFIX_FORM (3) | (9ULL << 52))

/* An 8-byte D form prefix instruction.  */
#define P_D_MASK (((-1ULL << 50) & ~PCREL_MASK) | OP_MASK)

/* The same as P_D_MASK, but with the RA and PCREL fields specified.  */
#define P_DRAPCREL_MASK (P_D_MASK | PCREL_MASK | RA_MASK)

/* Mask for prefix X form instructions.  */
#define P_X_MASK (PREFIX_MASK | X_MASK)
#define P_XX1_MASK (PREFIX_MASK | XX1_MASK)

/* Mask for prefix vector permute insns.  */
#define P_XX4_MASK (PREFIX_MASK | XX4_MASK)
#define P_UXX4_MASK (P_XX4_MASK & ~(7ULL << 32))
#define P_U8XX4_MASK (P_XX4_MASK & ~(0xffULL << 32))

/* MMIRR:XX3-form 8-byte outer product instructions.  */
#define P_GER_MASK ((-1ULL << 40) | XX3ACC_MASK)
#define P_GER2_MASK (P_GER_MASK & ~(3ULL << 46))
#define P_GER4_MASK (P_GER_MASK & ~(15ULL << 44))
#define P_GER8_MASK (P_GER_MASK & ~(255ULL << 40))
#define P_GER64_MASK (P_GER_MASK | (3ULL << 32))
#define P_GERX4_MASK ((-1ULL << 48) | XX3GERX_MASK)
#define P_GERX2_MASK (P_GERX4_MASK & ~(3ULL << 46))

/* Vector splat immediate op.  */
#define VSOP(op, xop) (OP (op) | (xop << 17))
#define P_VS_MASK ((-1ULL << 48) | VSOP (0x3f, 0xf))
#define P_VSI_MASK ((-1ULL << 48) | VSOP (0x3f, 0xe))

/* The main opcode combined with a trap code in the TO field of a D
   form instruction.  Used for extended mnemonics for the trap
   instructions.  */
#define OPTO(x,to) (OP (x) | ((((uint64_t)(to)) & 0x1f) << 21))
#define OPTO_MASK (OP_MASK | TO_MASK)

/* The main opcode combined with a comparison size bit in the L field
   of a D form or X form instruction.  Used for extended mnemonics for
   the comparison instructions.  */
#define OPL(x,l) (OP (x) | ((((uint64_t)(l)) & 1) << 21))
#define OPL_MASK OPL (0x3f,1)

/* The main opcode combined with an update code in D form instruction.
   Used for extended mnemonics for VLE memory instructions.  */
#define OPVUP(x,vup) (OP (x) | ((((uint64_t)(vup)) & 0xff) << 8))
#define OPVUP_MASK OPVUP (0x3f,  0xff)

/* The main opcode combined with an update code and the RT fields
   specified in D form instruction.  Used for VLE volatile context
   save/restore instructions.  */
#define OPVUPRT(x,vup,rt)			\
  (OPVUP (x, vup)				\
   | ((((uint64_t)(rt)) & 0x1f) << 21))
#define OPVUPRT_MASK OPVUPRT (0x3f, 0xff, 0x1f)

/* An A form instruction.  */
#define A(op, xop, rc)				\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x1f) << 1)	\
   | (((uint64_t)(rc)) & 1))
#define A_MASK A (0x3f, 0x1f, 1)

/* An A_MASK with the FRB field fixed.  */
#define AFRB_MASK (A_MASK | FRB_MASK)

/* An A_MASK with the FRC field fixed.  */
#define AFRC_MASK (A_MASK | FRC_MASK)

/* An A_MASK with the FRA and FRC fields fixed.  */
#define AFRAFRC_MASK (A_MASK | FRA_MASK | FRC_MASK)

/* An AFRAFRC_MASK, but with L bit clear.  */
#define AFRALFRC_MASK (AFRAFRC_MASK & ~((uint64_t) 1 << 16))

/* A B form instruction.  */
#define B(op, aa, lk)				\
  (OP (op)					\
   | ((((uint64_t)(aa)) & 1) << 1)		\
   | ((lk) & 1))
#define B_MASK B (0x3f, 1, 1)

/* A BD8 form instruction.  This is a 16-bit instruction.  */
#define BD8(op, aa, lk)				\
  (((((uint64_t)(op)) & 0x3f) << 10)	\
   | (((aa) & 1) << 9)				\
   | (((lk) & 1) << 8))
#define BD8_MASK BD8 (0x3f, 1, 1)

/* Another BD8 form instruction.  This is a 16-bit instruction.  */
#define BD8IO(op) ((((uint64_t)(op)) & 0x1f) << 11)
#define BD8IO_MASK BD8IO (0x1f)

/* A BD8 form instruction for simplified mnemonics.  */
#define EBD8IO(op, bo, bi) (BD8IO ((op)) | ((bo) << 10) | ((bi) << 8))
/* A mask that excludes BO32 and BI32.  */
#define EBD8IO1_MASK 0xf800
/* A mask that includes BO32 and excludes BI32.  */
#define EBD8IO2_MASK 0xfc00
/* A mask that include BO32 AND BI32.  */
#define EBD8IO3_MASK 0xff00

/* A BD15 form instruction.  */
#define BD15(op, aa, lk)			\
  (OP (op)					\
   | ((((uint64_t)(aa)) & 0xf) << 22)	\
   | ((lk) & 1))
#define BD15_MASK BD15 (0x3f, 0xf, 1)

/* A BD15 form instruction for extended conditional branch mnemonics.  */
#define EBD15(op, aa, bo, lk)			\
  (((op) & 0x3fu) << 26)			\
  | (((aa) & 0xf) << 22)			\
  | (((bo) & 0x3) << 20)			\
  | ((lk) & 1)
#define EBD15_MASK 0xfff00001

/* A BD15 form instruction for extended conditional branch mnemonics
   with BI.  */
#define EBD15BI(op, aa, bo, bi, lk)		\
  ((((op) & 0x3fu) << 26)			\
   | (((aa) & 0xf) << 22)			\
   | (((bo) & 0x3) << 20)			\
   | (((bi) & 0x3) << 16)			\
   | ((lk) & 1))

#define EBD15BI_MASK  0xfff30001

/* A BD24 form instruction.  */
#define BD24(op, aa, lk)			\
  (OP (op)					\
   | ((((uint64_t)(aa)) & 1) << 25)	\
   | ((lk) & 1))
#define BD24_MASK BD24 (0x3f, 1, 1)

/* A B form instruction setting the BO field.  */
#define BBO(op, bo, aa, lk)			\
  (B ((op), (aa), (lk))				\
   | ((((uint64_t)(bo)) & 0x1f) << 21))
#define BBO_MASK BBO (0x3f, 0x1f, 1, 1)

/* A BBO_MASK with the y bit of the BO field removed.  This permits
   matching a conditional branch regardless of the setting of the y
   bit.  Similarly for the 'at' bits used for power4 branch hints.  */
#define Y_MASK	 (((uint64_t) 1) << 21)
#define AT1_MASK (((uint64_t) 3) << 21)
#define AT2_MASK (((uint64_t) 9) << 21)
#define BBOY_MASK  (BBO_MASK &~ Y_MASK)
#define BBOAT_MASK (BBO_MASK &~ AT1_MASK)

/* A B form instruction setting the BO field and the condition bits of
   the BI field.  */
#define BBOCB(op, bo, cb, aa, lk) \
  (BBO ((op), (bo), (aa), (lk)) | ((((uint64_t)(cb)) & 0x3) << 16))
#define BBOCB_MASK BBOCB (0x3f, 0x1f, 0x3, 1, 1)

/* A BBOCB_MASK with the y bit of the BO field removed.  */
#define BBOYCB_MASK (BBOCB_MASK &~ Y_MASK)
#define BBOATCB_MASK (BBOCB_MASK &~ AT1_MASK)
#define BBOAT2CB_MASK (BBOCB_MASK &~ AT2_MASK)

/* A BBOYCB_MASK in which the BI field is fixed.  */
#define BBOYBI_MASK (BBOYCB_MASK | BI_MASK)
#define BBOATBI_MASK (BBOAT2CB_MASK | BI_MASK)

/* A VLE C form instruction.  */
#define C_LK(x, lk) (((((uint64_t)(x)) & 0x7fff) << 1) | ((lk) & 1))
#define C_LK_MASK C_LK(0x7fff, 1)
#define C(x) ((((uint64_t)(x)) & 0xffff))
#define C_MASK C(0xffff)

/* An Context form instruction.  */
#define CTX(op, xop)   (OP (op) | (((uint64_t)(xop)) & 0x7))
#define CTX_MASK CTX(0x3f, 0x7)

/* An User Context form instruction.  */
#define UCTX(op, xop)  (OP (op) | (((uint64_t)(xop)) & 0x1f))
#define UCTX_MASK UCTX(0x3f, 0x1f)

/* The main opcode mask with the RA field clear.  */
#define DRA_MASK (OP_MASK | RA_MASK)

/* A DQ form VSX instruction.  */
#define DQX(op, xop) (OP (op) | ((xop) & 0x7))
#define DQX_MASK DQX (0x3f, 7)

/* A DQ form VSX vector paired instruction.  */
#define DQXP(op, xop) (OP (op) | ((xop) & 0xf))
#define DQXP_MASK DQXP (0x3f, 0xf)

/* A DS form instruction.  */
#define DSO(op, xop) (OP (op) | ((xop) & 0x3))
#define DS_MASK DSO (0x3f, 3)

/* An DX form instruction.  */
#define DX(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0x1f) << 1))
#define DX_MASK DX (0x3f, 0x1f)
/* An DX form instruction with the D bits specified.  */
#define NODX_MASK (DX_MASK | 0x1fffc1)

/* An EVSEL form instruction.  */
#define EVSEL(op, xop) (OP (op) | (((uint64_t)(xop)) & 0xff) << 3)
#define EVSEL_MASK EVSEL(0x3f, 0xff)

/* An IA16 form instruction.  */
#define IA16(op, xop) (OP (op) | (((uint64_t)(xop)) & 0x1f) << 11)
#define IA16_MASK IA16(0x3f, 0x1f)

/* An I16A form instruction.  */
#define I16A(op, xop) (OP (op) | (((uint64_t)(xop)) & 0x1f) << 11)
#define I16A_MASK I16A(0x3f, 0x1f)

/* An I16L form instruction.  */
#define I16L(op, xop) (OP (op) | (((uint64_t)(xop)) & 0x1f) << 11)
#define I16L_MASK I16L(0x3f, 0x1f)

/* An IM7 form instruction.  */
#define IM7(op) ((((uint64_t)(op)) & 0x1f) << 11)
#define IM7_MASK IM7(0x1f)

/* An M form instruction.  */
#define M(op, rc) (OP (op) | ((rc) & 1))
#define M_MASK M (0x3f, 1)

/* An LI20 form instruction.  */
#define LI20(op, xop) (OP (op) | (((uint64_t)(xop)) & 0x1) << 15)
#define LI20_MASK LI20(0x3f, 0x1)

/* An M form instruction with the ME field specified.  */
#define MME(op, me, rc)				\
  (M ((op), (rc))				\
   | ((((uint64_t)(me)) & 0x1f) << 1))

/* An M_MASK with the MB field fixed.  */
#define MMB_MASK (M_MASK | MB_MASK)

/* An M_MASK with the ME field fixed.  */
#define MME_MASK (M_MASK | ME_MASK)

/* An M_MASK with the MB and ME fields fixed.  */
#define MMBME_MASK (M_MASK | MB_MASK | ME_MASK)

/* An M_MASK with the SH and ME fields fixed.  */
#define MSHME_MASK (M_MASK | SH_MASK | ME_MASK)

/* An M_MASK with the SH and MB fields fixed.  */
#define MSHMB_MASK (M_MASK | SH_MASK | MB_MASK)

/* An MD form instruction.  */
#define MD(op, xop, rc)				\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x7) << 2)	\
   | ((rc) & 1))
#define MD_MASK MD (0x3f, 0x7, 1)

/* An MD_MASK with the MB field fixed.  */
#define MDMB_MASK (MD_MASK | MB6_MASK)

/* An MD_MASK with the SH field fixed.  */
#define MDSH_MASK (MD_MASK | SH6_MASK)

/* An MDS form instruction.  */
#define MDS(op, xop, rc)			\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0xf) << 1)	\
   | ((rc) & 1))
#define MDS_MASK MDS (0x3f, 0xf, 1)

/* An MDS_MASK with the MB field fixed.  */
#define MDSMB_MASK (MDS_MASK | MB6_MASK)

/* An SC form instruction.  */
#define SC(op, sa, lk)				\
  (OP (op)					\
   | ((((uint64_t)(sa)) & 1) << 1)		\
   | ((lk) & 1))
#define SC_MASK					\
  (OP_MASK					\
   | (((uint64_t) 0x3ff) << 16)		\
   | (((uint64_t) 1) << 1)			\
   | 1)

/* An SCI8 form instruction.  */
#define SCI8(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0x1f) << 11))
#define SCI8_MASK SCI8(0x3f, 0x1f)

/* An SCI8 form instruction.  */
#define SCI8BF(op, fop, xop)			\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x1f) << 11)	\
   | (((fop) & 7) << 23))
#define SCI8BF_MASK SCI8BF(0x3f, 7, 0x1f)

/* An SD4 form instruction.  This is a 16-bit instruction.  */
#define SD4(op) ((((uint64_t)(op)) & 0xf) << 12)
#define SD4_MASK SD4(0xf)

/* An SE_IM5 form instruction.  This is a 16-bit instruction.  */
#define SE_IM5(op, xop)				\
  (((((uint64_t)(op)) & 0x3f) << 10)	\
   | (((xop) & 0x1) << 9))
#define SE_IM5_MASK SE_IM5(0x3f, 1)

/* An SE_R form instruction.  This is a 16-bit instruction.  */
#define SE_R(op, xop)				\
  (((((uint64_t)(op)) & 0x3f) << 10)	\
   | (((xop) & 0x3f) << 4))
#define SE_R_MASK SE_R(0x3f, 0x3f)

/* An SE_RR form instruction.  This is a 16-bit instruction.  */
#define SE_RR(op, xop)				\
  (((((uint64_t)(op)) & 0x3f) << 10)	\
   | (((xop) & 0x3) << 8))
#define SE_RR_MASK SE_RR(0x3f, 3)

/* A VX form instruction.  */
#define VX(op, xop) (OP (op) | (((uint64_t)(xop)) & 0x7ff))

/* The mask for an VX form instruction.  */
#define VX_MASK	VX(0x3f, 0x7ff)

/* A VX LSP form instruction.  */
#define VX_LSP(op, xop) (OP (op) | (((uint64_t)(xop)) & 0xffff))

/* The mask for an VX LSP form instruction.  */
#define VX_LSP_MASK	VX_LSP(0x3f, 0xffff)
#define VX_LSP_OFF_MASK	VX_LSP(0x3f, 0x7fc)

/* Additional format of VX SPE2 form instruction.   */
#define VX_RA_CONST(op, xop, bits11_15)			\
  (OP (op)						\
   | (((uint64_t)(bits11_15) & 0x1f) << 16)	\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_RA_CONST_MASK VX_RA_CONST(0x3f, 0x7ff, 0x1f)

#define VX_RB_CONST(op, xop, bits16_20)			\
  (OP (op)						\
   | (((uint64_t)(bits16_20) & 0x1f) << 11)	\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_RB_CONST_MASK VX_RB_CONST(0x3f, 0x7ff, 0x1f)

#define VX_OFF_SPE2_MASK VX(0x3f, 0x7f8)

#define VX_SPE_CRFD(op, xop, bits9_10)			\
  (OP (op)						\
   | (((uint64_t)(bits9_10) & 0x3) << 21)		\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_SPE_CRFD_MASK VX_SPE_CRFD(0x3f, 0x7ff, 0x3)

#define VX_SPE2_CLR(op, xop, bit16)			\
  (OP (op)						\
   | (((uint64_t)(bit16) & 0x1) << 15)		\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_SPE2_CLR_MASK VX_SPE2_CLR(0x3f, 0x7ff, 0x1)

#define VX_SPE2_SPLATB(op, xop, bits19_20)		\
  (OP (op)						\
   | (((uint64_t)(bits19_20) & 0x3) << 11)		\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_SPE2_SPLATB_MASK VX_SPE2_SPLATB(0x3f, 0x7ff, 0x3)

#define VX_SPE2_OCTET(op, xop, bits16_17)		\
  (OP (op)						\
   | (((uint64_t)(bits16_17) & 0x3) << 14)		\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_SPE2_OCTET_MASK VX_SPE2_OCTET(0x3f, 0x7ff, 0x7)

#define VX_SPE2_DDHH(op, xop, bit16) 			\
  (OP (op)						\
   | (((uint64_t)(bit16) & 0x1) << 15)		\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_SPE2_DDHH_MASK VX_SPE2_DDHH(0x3f, 0x7ff, 0x1)

#define VX_SPE2_HH(op, xop, bit16, bits19_20)		\
  (OP (op)						\
   | (((uint64_t)(bit16) & 0x1) << 15)		\
   | (((uint64_t)(bits19_20) & 0x3) << 11)	\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_SPE2_HH_MASK VX_SPE2_HH(0x3f, 0x7ff, 0x1, 0x3)

#define VX_SPE2_EVMAR(op, xop)				\
  (OP (op)						\
   | ((uint64_t)(0x1) << 11)			\
   | (((uint64_t)(xop)) & 0x7ff))
#define VX_SPE2_EVMAR_MASK				\
  (VX_SPE2_EVMAR(0x3f, 0x7ff)				\
   | ((uint64_t)(0x1) << 11))

/* A VX_MASK with the VA field fixed.  */
#define VXVA_MASK (VX_MASK | (0x1f << 16))

/* A VX_MASK with the VB field fixed.  */
#define VXVB_MASK (VX_MASK | (0x1f << 11))

/* A VX_MASK with the VA and VB fields fixed.  */
#define VXVAVB_MASK (VX_MASK | (0x1f << 16) | (0x1f << 11))

/* A VX_MASK with the VD and VA fields fixed.  */
#define VXVDVA_MASK (VX_MASK | (0x1f << 21) | (0x1f << 16))

/* A VX_MASK with a UIMM4 field.  */
#define VXUIMM4_MASK (VX_MASK | (0x1 << 20))

/* A VX_MASK with a UIMM3 field.  */
#define VXUIMM3_MASK (VX_MASK | (0x3 << 19))

/* A VX_MASK with a UIMM2 field.  */
#define VXUIMM2_MASK (VX_MASK | (0x7 << 18))

/* A VX_MASK with a PS field.  */
#define VXPS_MASK (VX_MASK & ~(0x1 << 9))

/* A VX_MASK with the VA field fixed with a PS field.  */
#define VXVAPS_MASK (VXVA_MASK & ~(0x1 << 9))

/* A VX_MASK with the VA field fixed with a MP field.  */
#define VXVAM_MASK (VXVA_MASK & ~(0x1 << 16))

/* A VX_MASK for instructions using a BF field.  */
#define VXBF_MASK (VX_MASK | (3 << 21))

/* A VX_MASK for instructions with an RC field.  */
#define VXRC_MASK (VX_MASK & ~(0x1f << 6))

/* A VX_MASK for instructions with a SH field.  */
#define VXSH_MASK (VX_MASK & ~(0x7 << 6))

/* A VA form instruction.  */
#define VXA(op, xop) (OP (op) | (((uint64_t)(xop)) & 0x03f))

/* The mask for an VA form instruction.  */
#define VXA_MASK VXA(0x3f, 0x3f)

/* A VXA_MASK with a SHB field.  */
#define VXASHB_MASK (VXA_MASK | (1 << 10))

/* A VXR form instruction.  */
#define VXR(op, xop, rc)			\
  (OP (op)					\
   | (((uint64_t)(rc) & 1) << 10)		\
   | (((uint64_t)(xop)) & 0x3ff))

/* The mask for a VXR form instruction.  */
#define VXR_MASK VXR(0x3f, 0x3ff, 1)

/* A VX form instruction with a VA tertiary opcode.  */
#define VXVA(op, xop, vaop) (VX(op,xop) | (((vaop) & 0x1f) << 16))

#define VXASH(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0x1f) << 1))
#define VXASH_MASK VXASH (0x3f, 0x1f)

/* An X form instruction.  */
#define X(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0x3ff) << 1))

/* A X form instruction for Quad-Precision FP Instructions.  */
#define XVA(op, xop, vaop) (X(op,xop) | (((vaop) & 0x1f) << 16))

/* An EX form instruction.  */
#define EX(op, xop) (OP (op) | (((uint64_t)(xop)) & 0x7ff))

/* The mask for an EX form instruction.  */
#define EX_MASK EX (0x3f, 0x7ff)

/* An XX2 form instruction.  */
#define XX2(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0x1ff) << 2))

/* A XX2 form instruction with the VA bits specified.  */
#define XX2VA(op, xop, vaop) (XX2(op,xop) | (((vaop) & 0x1f) << 16))

/* An XX3 form instruction.  */
#define XX3(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0xff) << 3))

/* An XX3 form instruction with the RC bit specified.  */
#define XX3RC(op, xop, rc)			\
  (OP (op)					\
   | (((uint64_t)(rc) & 1) << 10)		\
   | ((((uint64_t)(xop)) & 0x7f) << 3))

/* An XX4 form instruction.  */
#define XX4(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0x3) << 4))

/* A Z form instruction.  */
#define Z(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0x1ff) << 1))

/* An X form instruction with the RC bit specified.  */
#define XRC(op, xop, rc) (X ((op), (xop)) | ((rc) & 1))

/* A X form instruction for Quad-Precision FP Instructions with RC bit.  */
#define XVARC(op, xop, vaop, rc) (XVA ((op), (xop), (vaop)) | ((rc) & 1))

/* An X form instruction with the RA bits specified as two ops.  */
#define XMMF(op, xop, mop0, mop1)		\
  (X ((op), (xop))				\
   | ((mop0) & 3) << 19				\
   | ((mop1) & 7) << 16)

/* A Z form instruction with the RC bit specified.  */
#define ZRC(op, xop, rc) (Z ((op), (xop)) | ((rc) & 1))

/* The mask for an X form instruction.  */
#define X_MASK XRC (0x3f, 0x3ff, 1)

/* The mask for an X form instruction with the BF bits specified.  */
#define XBF_MASK (X_MASK | (3 << 21))

/* An X form instruction without the RC field specified.  */
#define XRC_MASK XRC (0x3f, 0x3ff, 0)

/* An X form wait instruction with everything filled in except the WC
   field.  */
#define XWC_MASK (XRC (0x3f, 0x3ff, 1) | (7 << 23) | RA_MASK | RB_MASK)

/* An X form wait instruction with everything filled in except the WC
   and PL fields.  */
#define XWCPL_MASK (XRC (0x3f, 0x3ff, 1) | (7 << 23) | (3 << 18) | RB_MASK)

/* The mask for an XX1 form instruction.  */
#define XX1_MASK X (0x3f, 0x3ff)

/* An XX1_MASK with the RB field fixed.  */
#define XX1RB_MASK (XX1_MASK | RB_MASK)

/* The mask for an XX2 form instruction.  */
#define XX2_MASK (XX2 (0x3f, 0x1ff) | (0x1f << 16))

/* The mask for an XX2 form instruction with the UIM bits specified.  */
#define XX2UIM_MASK (XX2 (0x3f, 0x1ff) | (7 << 18))

/* The mask for an XX2 form instruction with the 4 UIM bits specified.  */
#define XX2UIM4_MASK (XX2 (0x3f, 0x1ff) | (1 << 20))

/* The mask for an XX2 form instruction with the BF bits specified.  */
#define XX2BF_MASK (XX2_MASK | (3 << 21) | (1))

/* The mask for an XX2 form instruction with the BF and DCMX bits
   specified.  */
#define XX2BFD_MASK (XX2 (0x3f, 0x1ff) | 1)

/* The mask for an XX2 form instruction with a split DCMX bits
   specified.  */
#define XX2DCMXS_MASK XX2 (0x3f, 0x1ee)

/* The mask for an XX3 form instruction.  */
#define XX3_MASK XX3 (0x3f, 0xff)

/* The mask for an XX3 form instruction with the BF bits specified.  */
#define XX3BF_MASK (XX3 (0x3f, 0xff) | (3 << 21) | (1))

/* An X_MASK with an accumulator register and the RA and RB fields fixed.  */
#define XACC_MASK (X_MASK | RA_MASK | RB_MASK | (3 << 21))
#define XDMR_MASK XACC_MASK

/* An X_MASK with two dense math register.  */
#define XDMRDMR_MASK (X_MASK | RA_MASK | (3 << 21) | (3 << 11))

/* The mask for an XX3 form instruction with the DM or SHW bits
   specified.  */
#define XX3DM_MASK (XX3 (0x3f, 0x1f) | (1 << 10))
#define XX3SHW_MASK XX3DM_MASK

/* The masks for X* form instructions with an ACC/DMR register.  */
#define XX2ACC_MASK (XX2 (0x3f, 0x1ff) | (3 << 21) | 1)
#define XX3ACC_MASK (XX3_MASK | (3 << 21) | 1)
#define XX3DMR_MASK (XX3ACC_MASK | (1 << 11))
#define XX2DMR_MASK (XX2ACC_MASK | (0xf << 17))
#define XX3GERX_MASK (XX3ACC_MASK | (1 << 16))

/* The mask for an XX4 form instruction.  */
#define XX4_MASK XX4 (0x3f, 0x3)

/* An X form wait instruction with everything filled in except the WC
   field.  */
#define XWC_MASK (XRC (0x3f, 0x3ff, 1) | (7 << 23) | RA_MASK | RB_MASK)

/* The mask for an XMMF form instruction.  */
#define XMMF_MASK (XMMF (0x3f, 0x3ff, 3, 7) | (1))

/* The mask for a Z form instruction.  */
#define Z_MASK ZRC (0x3f, 0x1ff, 1)
#define Z2_MASK ZRC (0x3f, 0xff, 1)

/* An X_MASK with the RA/VA field fixed.  */
#define XRA_MASK (X_MASK | RA_MASK)
#define XVA_MASK XRA_MASK

/* An XRA_MASK with the A_L/W field clear.  */
#define XWRA_MASK (XRA_MASK & ~((uint64_t) 1 << 16))
#define XRLA_MASK XWRA_MASK

/* An X_MASK with the RB field fixed.  */
#define XRB_MASK (X_MASK | RB_MASK)

/* An X_MASK with the RT field fixed.  */
#define XRT_MASK (X_MASK | RT_MASK)

/* An XRT_MASK mask with the 2 L bits clear.  */
#define XLRT_MASK (XRT_MASK & ~((uint64_t) 0x3 << 21))

/* An XRT_MASK mask with the 3 L bits clear.  */
#define XL3RT_MASK (XRT_MASK & ~((uint64_t) 0x7 << 21))

/* An X_MASK with the RA and RB fields fixed.  */
#define XRARB_MASK (X_MASK | RA_MASK | RB_MASK)

/* An XBF_MASK with the RA and RB fields fixed.  */
#define XBFRARB_MASK (XBF_MASK | RA_MASK | RB_MASK)

/* An XRARB_MASK, but with the L bit clear.  */
#define XRLARB_MASK (XRARB_MASK & ~((uint64_t) 1 << 16))

/* An XRARB_MASK, but with the L bits in a darn instruction clear.  */
#define XLRAND_MASK (XRARB_MASK & ~((uint64_t) 3 << 16))

/* An X_MASK with the RT and RA fields fixed.  */
#define XRTRA_MASK (X_MASK | RT_MASK | RA_MASK)

/* An X_MASK with the RT and RB fields fixed.  */
#define XRTRB_MASK (X_MASK | RT_MASK | RB_MASK)

/* An XRTRA_MASK, but with L bit clear.  */
#define XRTLRA_MASK (XRTRA_MASK & ~((uint64_t) 1 << 21))

/* An X_MASK with the RT, RA and RB fields fixed.  */
#define XRTRARB_MASK (X_MASK | RT_MASK | RA_MASK | RB_MASK)

/* An XRTRARB_MASK, but with L bit clear.  */
#define XRTLRARB_MASK (XRTRARB_MASK & ~((uint64_t) 1 << 21))

/* An XRTRARB_MASK, but with A bit clear.  */
#define XRTARARB_MASK (XRTRARB_MASK & ~((uint64_t) 1 << 25))

/* An XRTRARB_MASK, but with BF bits clear.  */
#define XRTBFRARB_MASK (XRTRARB_MASK & ~((uint64_t) 7 << 23))

/* An X form instruction with the L bit specified.  */
#define XOPL(op, xop, l)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(l)) & 1) << 21))

/* An X form instruction with the 2 L bits specified.  */
#define XOPL2(op, xop, l)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(l)) & 3) << 21))

/* An X form instruction with the 3 L bits specified.  */
#define XOPL3(op, xop, l)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(l)) & 7) << 21))

/* An X form instruction with the WC and PL bits specified.  */
#define XWCPL(op, xop, wc, pl)			\
  (XOPL3 ((op), (xop), (wc))			\
   | ((((uint64_t)(pl)) & 3) << 16))

/* An X form instruction with the L bit and RC bit specified.  */
#define XRCL(op, xop, l, rc)			\
  (XRC ((op), (xop), (rc))			\
   | ((((uint64_t)(l)) & 1) << 21))

/* An X form instruction with RT fields specified */
#define XRT(op, xop, rt)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(rt)) & 0x1f) << 21))

/* An X form instruction with RT and RA fields specified */
#define XRTRA(op, xop, rt, ra)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(rt)) & 0x1f) << 21)	\
   | ((((uint64_t)(ra)) & 0x1f) << 16))

/* The mask for an X form comparison instruction.  */
#define XCMP_MASK (X_MASK | (((uint64_t)1) << 22))

/* The mask for an X form comparison instruction with the L field
   fixed.  */
#define XCMPL_MASK (XCMP_MASK | (((uint64_t)1) << 21))

/* An X form trap instruction with the TO field specified.  */
#define XTO(op, xop, to)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(to)) & 0x1f) << 21))
#define XTO_MASK (X_MASK | TO_MASK)

/* An X form tlb instruction with the SH field specified.  */
#define XTLB(op, xop, sh)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(sh)) & 0x1f) << 11))
#define XTLB_MASK (X_MASK | SH_MASK)

/* An X form sync instruction.  */
#define XSYNC(op, xop, l)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(l)) & 3) << 21))

/* An X form sync instruction with everything filled in except the LS
   field.  */
#define XSYNC_MASK (0xff9fffff)

/* An X form sync instruction with everything filled in except the L
   and E fields.  */
#define XSYNCLE_MASK (0xff90ffff)

/* An X form sync instruction.  */
#define XSYNCLS(op, xop, l, s)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(l)) & 7) << 21)		\
   | ((((uint64_t)(s)) & 3) << 16))

/* An X form sync instruction with everything filled in except the
   L and SC fields.  */
#define XSYNCLS_MASK (0xff1cffff)

/* An X_MASK, but with the EH bit clear.  */
#define XEH_MASK (X_MASK & ~((uint64_t )1))

/* An X form AltiVec dss instruction.  */
#define XDSS(op, xop, a) (X ((op), (xop)) | ((((uint64_t)(a)) & 1) << 25))
#define XDSS_MASK XDSS(0x3f, 0x3ff, 1)

/* An XFL form instruction.  */
#define XFL(op, xop, rc)			\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x3ff) << 1)	\
   | (((uint64_t)(rc)) & 1))
#define XFL_MASK XFL (0x3f, 0x3ff, 1)

/* An X form isel instruction.  */
#define XISEL(op, xop, cr)	(OP (op) | ((xop) << 1) | ((cr) << 6))
#define XISEL_MASK	XISEL(0x3f, 0x1f, 0)

/* An XL form instruction with the LK field set to 0.  */
#define XL(op, xop) (OP (op) | ((((uint64_t)(xop)) & 0x3ff) << 1))

/* An XL form instruction which uses the LK field.  */
#define XLLK(op, xop, lk) (XL ((op), (xop)) | ((lk) & 1))

/* The mask for an XL form instruction.  */
#define XL_MASK XLLK (0x3f, 0x3ff, 1)

/* An XL_MASK with the RT, RA and RB fields fixed, but S bit clear.  */
#define XLS_MASK ((XL_MASK | RT_MASK | RA_MASK | RB_MASK) & ~(1 << 11))

/* An XL form instruction which explicitly sets the BO field.  */
#define XLO(op, bo, xop, lk) \
  (XLLK ((op), (xop), (lk)) | ((((uint64_t)(bo)) & 0x1f) << 21))
#define XLO_MASK (XL_MASK | BO_MASK)

/* An XL form instruction which sets the BO field and the condition
   bits of the BI field.  */
#define XLOCB(op, bo, cb, xop, lk) \
  (XLO ((op), (bo), (xop), (lk)) | ((((uint64_t)(cb)) & 3) << 16))

/* An XL_MASK with the BB field fixed.  */
#define XLBB_MASK (XL_MASK | BB_MASK)

/* A mask for branch instructions using the BH field.  */
#define XLBH_MASK (XL_MASK | (BB_MASK & ~(3 << 11)))

/* An XLBH_MASK with the BO field fixed.  */
#define XLBOBB_MASK (XLBH_MASK | BO_MASK)

/* An XLBH_MASK with the BO and BI fields fixed.  */
#define XLBOBIBB_MASK (XLBOBB_MASK | BI_MASK)

/* An XLBH_MASK with the BO and condition bits of the BI fields fixed.  */
#define XLBOCBBB_MASK (XLBOBB_MASK | (3 << 16))

/* An X form mbar instruction with MO field.  */
#define XMBAR(op, xop, mo)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(mo)) & 1) << 21))

/* An XO form instruction.  */
#define XO(op, xop, oe, rc)			\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x1ff) << 1)	\
   | ((((uint64_t)(oe)) & 1) << 10)	\
   | (((unsigned long)(rc)) & 1))
#define XO_MASK XO (0x3f, 0x1ff, 1, 1)
#define XOL_MASK XO (0x3f, 0x1ff, 0, 1)

/* An XO_MASK with the RB field fixed.  */
#define XORB_MASK (XO_MASK | RB_MASK)

/* An XOPS form instruction for paired singles.  */
#define XOPS(op, xop, rc)			\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x3ff) << 1)	\
   | (((uint64_t)(rc)) & 1))
#define XOPS_MASK XOPS (0x3f, 0x3ff, 1)


/* An XS form instruction.  */
#define XS(op, xop, rc)				\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x1ff) << 2)	\
   | (((uint64_t)(rc)) & 1))
#define XS_MASK XS (0x3f, 0x1ff, 1)

/* A mask for the FXM version of an XFX form instruction.  */
#define XFXFXM_MASK (X_MASK | (1 << 11) | (1 << 20))

/* An XFX form instruction with the FXM field filled in.  */
#define XFXM(op, xop, fxm, p4)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(fxm)) & 0xff) << 12)	\
   | ((uint64_t)(p4) << 20))

/* An XFX form instruction with the SPR field filled in.  */
#define XSPR(op, xop, spr)			\
  (X ((op), (xop))				\
   | ((((uint64_t)(spr)) & 0x1f) << 16)	\
   | ((((uint64_t)(spr)) & 0x3e0) << 6))
#define XSPR_MASK (X_MASK | SPR_MASK)

/* An XFX form instruction with the SPR field filled in except for the
   SPRBAT field.  */
#define XSPRBAT_MASK (XSPR_MASK &~ SPRBAT_MASK)

/* An XFX form instruction with the SPR field filled in except for the
   SPRGQR field.  */
#define XSPRGQR_MASK (XSPR_MASK &~ SPRGQR_MASK)

/* An XFX form instruction with the SPR field filled in except for the
   SPRG field.  */
#define XSPRG_MASK (XSPR_MASK & ~(0x1f << 16))

/* An X form instruction with everything filled in except the E field.  */
#define XE_MASK (0xffff7fff)

/* An X form user context instruction.  */
#define XUC(op, xop)  (OP (op) | (((uint64_t)(xop)) & 0x1f))
#define XUC_MASK      XUC(0x3f, 0x1f)

/* An XW form instruction.  */
#define XW(op, xop, rc)				\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x3f) << 1)	\
   | ((rc) & 1))
/* The mask for a G form instruction. rc not supported at present.  */
#define XW_MASK XW (0x3f, 0x3f, 0)

/* An APU form instruction.  */
#define APU(op, xop, rc)			\
  (OP (op)					\
   | (((uint64_t)(xop)) & 0x3ff) << 1	\
   | ((rc) & 1))

/* The mask for an APU form instruction.  */
#define APU_MASK APU (0x3f, 0x3ff, 1)
#define APU_RT_MASK (APU_MASK | RT_MASK)
#define APU_RA_MASK (APU_MASK | RA_MASK)

/* An SVL form instruction. */
#define SVL(op, xop, rc)			\
  (OP (op)					\
   | ((((uint64_t)(xop)) & 0x1f) << 1)		\
   | (((uint64_t)(rc)) & 1))
#define SVL_MASK	SVL (0x3f, 0x1f, 1)

/* An SVM form instruction. */
#define SVM(op, xop)				\
  (OP (op)					\
   | (((uint64_t)(xop)) & 0x3f))
#define SVM_MASK	SVM (0x3f, 0x3f)

/* An SVRM form instruction. */
#define SVRM(op, xop)				\
  (OP (op)					\
   | (((uint64_t)(xop)) & 0x3f))
#define SVRM_MASK	SVRM (0x3f, 0x3f)

/* An SVI form instruction. */
#define SVI(op, xop)				\
  (OP (op)					\
   | (((uint64_t)(xop)) & 0x3f))
#define SVI_MASK	SVI (0x3f, 0x3f)

/* The BO encodings used in extended conditional branch mnemonics.  */
#define BODNZF	(0x0)
#define BODNZFP	(0x1)
#define BODZF	(0x2)
#define BODZFP	(0x3)
#define BODNZT	(0x8)
#define BODNZTP	(0x9)
#define BODZT	(0xa)
#define BODZTP	(0xb)

#define BOF	(0x4)
#define BOFP	(0x5)
#define BOFM4	(0x6)
#define BOFP4	(0x7)
#define BOT	(0xc)
#define BOTP	(0xd)
#define BOTM4	(0xe)
#define BOTP4	(0xf)

#define BODNZ	(0x10)
#define BODNZP	(0x11)
#define BODZ	(0x12)
#define BODZP	(0x13)
#define BODNZM4 (0x18)
#define BODNZP4 (0x19)
#define BODZM4	(0x1a)
#define BODZP4	(0x1b)

#define BOU	(0x14)

/* The BO16 encodings used in extended VLE conditional branch mnemonics.  */
#define BO16F   (0x0)
#define BO16T   (0x1)

/* The BO32 encodings used in extended VLE conditional branch mnemonics.  */
#define BO32F   (0x0)
#define BO32T   (0x1)
#define BO32DNZ (0x2)
#define BO32DZ  (0x3)

/* The BI condition bit encodings used in extended conditional branch
   mnemonics.  */
#define CBLT	(0)
#define CBGT	(1)
#define CBEQ	(2)
#define CBSO	(3)

/* The TO encodings used in extended trap mnemonics.  */
#define TOLGT	(0x1)
#define TOLLT	(0x2)
#define TOEQ	(0x4)
#define TOLGE	(0x5)
#define TOLNL	(0x5)
#define TOLLE	(0x6)
#define TOLNG	(0x6)
#define TOGT	(0x8)
#define TOGE	(0xc)
#define TONL	(0xc)
#define TOLT	(0x10)
#define TOLE	(0x14)
#define TONG	(0x14)
#define TONE	(0x18)
#define TOU	(0x1f)

/* Smaller names for the flags so each entry in the opcodes table will
   fit on a single line.  */
#undef	PPC
#define PPC	PPC_OPCODE_PPC
#define PPCCOM	PPC_OPCODE_PPC | PPC_OPCODE_COMMON
#define POWER4	PPC_OPCODE_POWER4
#define POWER5	PPC_OPCODE_POWER5
#define POWER6	PPC_OPCODE_POWER6
#define POWER7	PPC_OPCODE_POWER7
#define POWER8	PPC_OPCODE_POWER8
#define POWER9	PPC_OPCODE_POWER9
#define POWER10 PPC_OPCODE_POWER10
#define FUTURE	PPC_OPCODE_FUTURE
#define CELL	PPC_OPCODE_CELL
#define PPC64	PPC_OPCODE_64 | PPC_OPCODE_64_BRIDGE
#define NON32	(PPC_OPCODE_64 | PPC_OPCODE_POWER4	\
		 | PPC_OPCODE_EFS | PPC_OPCODE_E500MC | PPC_OPCODE_TITAN)
#define PPC403	PPC_OPCODE_403
#define PPC405	PPC_OPCODE_405
#define PPC440	PPC_OPCODE_440
#define PPC464	PPC440
#define PPC476	PPC_OPCODE_476
#define PPC750	PPC_OPCODE_750
#define GEKKO	PPC_OPCODE_750
#define BROADWAY PPC_OPCODE_750
#define PPC7450 PPC_OPCODE_7450
#define PPC860	PPC_OPCODE_860
#define PPCPS	PPC_OPCODE_PPCPS
#define PPCVEC	PPC_OPCODE_ALTIVEC
#define PPCVEC2	(PPC_OPCODE_POWER8 | PPC_OPCODE_E6500)
#define PPCVEC3	PPC_OPCODE_POWER9
#define PPCVSX	PPC_OPCODE_VSX
#define PPCVSX2	PPC_OPCODE_POWER8
#define PPCVSX3	PPC_OPCODE_POWER9
#define PPCVSX4	PPC_OPCODE_POWER10
#define PPCVSXF	PPC_OPCODE_FUTURE
#define POWER	PPC_OPCODE_POWER
#define POWER2	PPC_OPCODE_POWER | PPC_OPCODE_POWER2
#define PWR2COM PPC_OPCODE_POWER | PPC_OPCODE_POWER2 | PPC_OPCODE_COMMON
#define PPCPWR2 (PPC_OPCODE_PPC | PPC_OPCODE_POWER | PPC_OPCODE_POWER2 \
		 | PPC_OPCODE_COMMON)
#define COM	PPC_OPCODE_POWER | PPC_OPCODE_PPC | PPC_OPCODE_COMMON
#define M601	PPC_OPCODE_POWER | PPC_OPCODE_601
#define PWRCOM	PPC_OPCODE_POWER | PPC_OPCODE_601 | PPC_OPCODE_COMMON
#define MFDEC1	PPC_OPCODE_POWER
#define MFDEC2	(PPC_OPCODE_PPC | PPC_OPCODE_601 | PPC_OPCODE_BOOKE \
		 | PPC_OPCODE_TITAN)
#define BOOKE	PPC_OPCODE_BOOKE
#define NO371	PPC_OPCODE_BOOKE | PPC_OPCODE_PPCPS | PPC_OPCODE_EFS
#define PPCE300 PPC_OPCODE_E300
#define PPCSPE	PPC_OPCODE_SPE
#define PPCSPE2 PPC_OPCODE_SPE2
#define PPCISEL PPC_OPCODE_ISEL
#define PPCEFS	PPC_OPCODE_EFS
#define PPCEFS2	PPC_OPCODE_EFS2
#define PPCBRLK PPC_OPCODE_BRLOCK
#define PPCPMR	PPC_OPCODE_PMR
#define PPCTMR	PPC_OPCODE_TMR
#define PPCCHLK PPC_OPCODE_CACHELCK
#define PPCRFMCI PPC_OPCODE_RFMCI
#define E500MC	PPC_OPCODE_E500MC
#define PPCA2	PPC_OPCODE_A2
#define TITAN	PPC_OPCODE_TITAN
#define MULHW	PPC_OPCODE_405 | PPC_OPCODE_440 | PPC_OPCODE_476 | TITAN
#define E500	PPC_OPCODE_E500
#define E6500	PPC_OPCODE_E6500
#define PPCVLE	PPC_OPCODE_VLE
#define PPCHTM	PPC_OPCODE_POWER8
#define E200Z4	PPC_OPCODE_E200Z4
#define PPCLSP	PPC_OPCODE_LSP
#define SVP64	PPC_OPCODE_SVP64
/* Used to mark extended mnemonic in deprecated field so that -Mraw
   won't use this variant in disassembly.  */
#define EXT	PPC_OPCODE_RAW

/* The opcode table.

   The format of the opcode table is:

   NAME		OPCODE		MASK	     FLAGS	ANTI		{OPERANDS}

   NAME is the name of the instruction.
   OPCODE is the instruction opcode.
   MASK is the opcode mask; this is used to tell the disassembler
     which bits in the actual opcode must match OPCODE.
   FLAGS are flags indicating which processors support the instruction.
   ANTI indicates which processors don't support the instruction.
   OPERANDS is the list of operands.

   The disassembler reads the table in order and prints the first
   instruction which matches, so this table is sorted to put more
   specific instructions before more general instructions.

   This table must be sorted by major opcode.  Please try to keep it
   vaguely sorted within major opcode too, except of course where
   constrained otherwise by disassembler operation.  */

const struct powerpc_opcode powerpc_opcodes[] = {
{"attn",	X(0,256),	X_MASK,	  POWER4|PPCA2,	PPC476|PPCVLE,	{0}},
{"tdlgti",	OPTO(2,TOLGT),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdllti",	OPTO(2,TOLLT),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdeqi",	OPTO(2,TOEQ),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdlgei",	OPTO(2,TOLGE),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdlnli",	OPTO(2,TOLNL),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdllei",	OPTO(2,TOLLE),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdlngi",	OPTO(2,TOLNG),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdgti",	OPTO(2,TOGT),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdgei",	OPTO(2,TOGE),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdnli",	OPTO(2,TONL),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdlti",	OPTO(2,TOLT),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdlei",	OPTO(2,TOLE),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdngi",	OPTO(2,TONG),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdnei",	OPTO(2,TONE),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdui",	OPTO(2,TOU),	OPTO_MASK,   PPC64,	PPCVLE|EXT,	{RA, SI}},
{"tdi",		OP(2),		OP_MASK,     PPC64,	PPCVLE,		{TO, RA, SI}},

{"twlgti",	OPTO(3,TOLGT),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tlgti",	OPTO(3,TOLGT),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twllti",	OPTO(3,TOLLT),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tllti",	OPTO(3,TOLLT),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"tweqi",	OPTO(3,TOEQ),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"teqi",	OPTO(3,TOEQ),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twlgei",	OPTO(3,TOLGE),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tlgei",	OPTO(3,TOLGE),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twlnli",	OPTO(3,TOLNL),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tlnli",	OPTO(3,TOLNL),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twllei",	OPTO(3,TOLLE),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tllei",	OPTO(3,TOLLE),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twlngi",	OPTO(3,TOLNG),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tlngi",	OPTO(3,TOLNG),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twgti",	OPTO(3,TOGT),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tgti",	OPTO(3,TOGT),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twgei",	OPTO(3,TOGE),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tgei",	OPTO(3,TOGE),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twnli",	OPTO(3,TONL),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tnli",	OPTO(3,TONL),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twlti",	OPTO(3,TOLT),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tlti",	OPTO(3,TOLT),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twlei",	OPTO(3,TOLE),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tlei",	OPTO(3,TOLE),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twngi",	OPTO(3,TONG),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tngi",	OPTO(3,TONG),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twnei",	OPTO(3,TONE),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tnei",	OPTO(3,TONE),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twui",	OPTO(3,TOU),	OPTO_MASK,   PPCCOM,	PPCVLE|EXT,	{RA, SI}},
{"tui",		OPTO(3,TOU),	OPTO_MASK,   PWRCOM,	PPCVLE|EXT,	{RA, SI}},
{"twi",		OP(3),		OP_MASK,     PPCCOM,	PPCVLE,		{TO, RA, SI}},
{"ti",		OP(3),		OP_MASK,     PWRCOM,	PPCVLE,		{TO, RA, SI}},

{"ps_cmpu0",	X  (4,	 0),	XBF_MASK,    PPCPS,	0,		{BF, FRA, FRB}},
{"vaddubm",	VX (4,	 0),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmul10cuq",	VX (4,	 1),	VXVB_MASK,   PPCVEC3,	0,		{VD, VA}},
{"vmaxub",	VX (4,	 2),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrlb",	VX (4,	 4),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrlq",	VX (4,	 5),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vcmpequb",	VXR(4,	 6,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpneb",	VXR(4,	 7,0),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"vmuloub",	VX (4,	 8),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vaddfp",	VX (4,	10),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vdivuq",	VX (4,  11),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"psq_lx",	XW (4,	 6,0),	XW_MASK,     PPCPS,	0,		{FRT,RA,RB,PSWM,PSQM}},
{"vmrghb",	VX (4,	12),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vstribl",	VXVA(4,13,0),	VXVA_MASK,   POWER10,	0,		{VD, VB}},
{"vstribr",	VXVA(4,13,1),	VXVA_MASK,   POWER10,	0,		{VD, VB}},
{"vstrihl",	VXVA(4,13,2),	VXVA_MASK,   POWER10,	0,		{VD, VB}},
{"vstrihr",	VXVA(4,13,3),	VXVA_MASK,   POWER10,	0,		{VD, VB}},
{"psq_stx",	XW (4,	 7,0),	XW_MASK,     PPCPS,	0,		{FRS,RA,RB,PSWM,PSQM}},
{"vpkuhum",	VX (4,	14),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinsbvlx",	VX (4,  15),	VX_MASK,     POWER10,	0,		{VD, RA, VB}},
{"mulhhwu",	XRC(4,	 8,0),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"mulhhwu.",	XRC(4,	 8,1),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"mtvsrbmi",	DX (4,10),	DX_MASK,     POWER10,	0,		{VD, DXD}},
{"ps_sum0",	A  (4,	10,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"ps_sum0.",	A  (4,	10,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vsldbi",	VX (4,  22),	VXSH_MASK,   POWER10,	0,		{VD, VA, VB, SH3}},
{"ps_sum1",	A  (4,	11,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"ps_sum1.",	A  (4,	11,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vextdubvlx",	VX (4,  24),	VXRC_MASK,   POWER10,	0,		{VD, VA, VB, RC}},
{"ps_muls0",	A  (4,	12,0),	AFRB_MASK,   PPCPS,	0,		{FRT, FRA, FRC}},
{"machhwu",	XO (4,	12,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vextdubvrx",	VX (4,  25),	VXRC_MASK,   POWER10,	0,		{VD, VA, VB, RC}},
{"ps_muls0.",	A  (4,	12,1),	AFRB_MASK,   PPCPS,	0,		{FRT, FRA, FRC}},
{"machhwu.",	XO (4,	12,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vextduhvlx",	VX (4,  26),	VXRC_MASK,   POWER10,	0,		{VD, VA, VB, RC}},
{"ps_muls1",	A  (4,	13,0),	AFRB_MASK,   PPCPS,	0,		{FRT, FRA, FRC}},
{"vextduhvrx",	VX (4,  27),	VXRC_MASK,   POWER10,	0,		{VD, VA, VB, RC}},
{"ps_muls1.",	A  (4,	13,1),	AFRB_MASK,   PPCPS,	0,		{FRT, FRA, FRC}},
{"vextduwvlx",	VX (4,  28),	VXRC_MASK,   POWER10,	0,		{VD, VA, VB, RC}},
{"ps_madds0",	A  (4,	14,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vextduwvrx",	VX (4,  29),	VXRC_MASK,   POWER10,	0,		{VD, VA, VB, RC}},
{"ps_madds0.",	A  (4,	14,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vextddvlx",	VX (4,  30),	VXRC_MASK,   POWER10,	0,		{VD, VA, VB, RC}},
{"ps_madds1",	A  (4,	15,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vextddvrx",	VX (4,  31),	VXRC_MASK,   POWER10,	0,		{VD, VA, VB, RC}},
{"ps_madds1.",	A  (4,	15,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vmhaddshs",	VXA(4,	32),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"vmhraddshs",	VXA(4,	33),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"vmladduhm",	VXA(4,	34),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"vmsumudm",	VXA(4,	35),	VXA_MASK,    PPCVEC3,	0,		{VD, VA, VB, VC}},
{"ps_div",	A  (4,	18,0),	AFRC_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"vmsumcud",	VXA(4,  23),	VXA_MASK,    POWER10,	0,		{VD, VA, VB, VC}},
{"vmsumubm",	VXA(4,	36),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"ps_div.",	A  (4,	18,1),	AFRC_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"vmsummbm",	VXA(4,	37),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"vmsumuhm",	VXA(4,	38),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"vmsumuhs",	VXA(4,	39),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"ps_sub",	A  (4,	20,0),	AFRC_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"vmsumshm",	VXA(4,	40),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"ps_sub.",	A  (4,	20,1),	AFRC_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"vmsumshs",	VXA(4,	41),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"ps_add",	A  (4,	21,0),	AFRC_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"vsel",	VXA(4,	42),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"ps_add.",	A  (4,	21,1),	AFRC_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"vperm",	VXA(4,	43),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VB, VC}},
{"vsldoi",	VXA(4,	44),	VXASHB_MASK, PPCVEC,	0,		{VD, VA, VB, SHB}},
{"vpermxor",	VXA(4,	45),	VXA_MASK,    PPCVEC2,	0,		{VD, VA, VB, VC}},
{"ps_sel",	A  (4,	23,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vmaddfp",	VXA(4,	46),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VC, VB}},
{"ps_sel.",	A  (4,	23,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vnmsubfp",	VXA(4,	47),	VXA_MASK,    PPCVEC,	0,		{VD, VA, VC, VB}},
{"ps_res",	A  (4,	24,0), AFRAFRC_MASK, PPCPS,	0,		{FRT, FRB}},
{"maddhd",	VXA(4,	48),	VXA_MASK,    POWER9,	0,		{RT, RA, RB, RC}},
{"ps_res.",	A  (4,	24,1), AFRAFRC_MASK, PPCPS,	0,		{FRT, FRB}},
{"maddhdu",	VXA(4,	49),	VXA_MASK,    POWER9,	0,		{RT, RA, RB, RC}},
{"ps_mul",	A  (4,	25,0),	AFRB_MASK,   PPCPS,	0,		{FRT, FRA, FRC}},
{"ps_mul.",	A  (4,	25,1),	AFRB_MASK,   PPCPS,	0,		{FRT, FRA, FRC}},
{"maddld",	VXA(4,	51),	VXA_MASK,    POWER9,	0,		{RT, RA, RB, RC}},
{"ps_rsqrte",	A  (4,	26,0), AFRAFRC_MASK, PPCPS,	0,		{FRT, FRB}},
{"ps_rsqrte.",	A  (4,	26,1), AFRAFRC_MASK, PPCPS,	0,		{FRT, FRB}},
{"ps_msub",	A  (4,	28,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"ps_msub.",	A  (4,	28,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"ps_madd",	A  (4,	29,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"ps_madd.",	A  (4,	29,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vpermr",	VXA(4,	59),	VXA_MASK,    PPCVEC3,	0,		{VD, VA, VB, VC}},
{"ps_nmsub",	A  (4,	30,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vaddeuqm",	VXA(4,	60),	VXA_MASK,    PPCVEC2,	0,		{VD, VA, VB, VC}},
{"ps_nmsub.",	A  (4,	30,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vaddecuq",	VXA(4,	61),	VXA_MASK,    PPCVEC2,	0,		{VD, VA, VB, VC}},
{"ps_nmadd",	A  (4,	31,0),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vsubeuqm",	VXA(4,	62),	VXA_MASK,    PPCVEC2,	0,		{VD, VA, VB, VC}},
{"ps_nmadd.",	A  (4,	31,1),	A_MASK,	     PPCPS,	0,		{FRT, FRA, FRC, FRB}},
{"vsubecuq",	VXA(4,	63),	VXA_MASK,    PPCVEC2,	0,		{VD, VA, VB, VC}},
{"ps_cmpo0",	X  (4,	32),	XBF_MASK,    PPCPS,	0,		{BF, FRA, FRB}},
{"vadduhm",	VX (4,	64),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmul10ecuq",	VX (4,	65),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vmaxuh",	VX (4,	66),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrlh",	VX (4,	68),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrlqmi",	VX (4,	69),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vcmpequh",	VXR(4,	70,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpneh",	VXR(4,	71,0),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"vmulouh",	VX (4,	72),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vsubfp",	VX (4,	74),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"psq_lux",	XW (4,	38,0),	XW_MASK,     PPCPS,	0,		{FRT,RA,RB,PSWM,PSQM}},
{"vmrghh",	VX (4,	76),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"psq_stux",	XW (4,	39,0),	XW_MASK,     PPCPS,	0,		{FRS,RA,RB,PSWM,PSQM}},
{"vpkuwum",	VX (4,	78),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinshvlx",	VX (4,  79),	VX_MASK,     POWER10,	0,		{VD, RA, VB}},
{"ps_neg",	XRC(4,	40,0),	XRA_MASK,    PPCPS,	0,		{FRT, FRB}},
{"mulhhw",	XRC(4,	40,0),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"ps_neg.",	XRC(4,	40,1),	XRA_MASK,    PPCPS,	0,		{FRT, FRB}},
{"mulhhw.",	XRC(4,	40,1),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"machhw",	XO (4,	44,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"machhw.",	XO (4,	44,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmachhw",	XO (4,	46,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmachhw.",	XO (4,	46,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"ps_cmpu1",	X  (4,	64),	XBF_MASK,    PPCPS,	0,		{BF, FRA, FRB}},
{"vadduwm",	VX (4,	128),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmaxuw",	VX (4,	130),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrlw",	VX (4,	132),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrlwmi",	VX (4,	133),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vcmpequw",	VXR(4,	134,0), VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpnew",	VXR(4,	135,0),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"vmulouw",	VX (4,	136),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vmuluwm",	VX (4,	137),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vdivuw",	VX (4,  139),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vmrghw",	VX (4,	140),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vpkuhus",	VX (4,	142),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinswvlx",	VX (4,  143),	VX_MASK,     POWER10,	0,		{VD, RA, VB}},
{"ps_mr",	XRC(4,	72,0),	XRA_MASK,    PPCPS,	0,		{FRT, FRB}},
{"ps_mr.",	XRC(4,	72,1),	XRA_MASK,    PPCPS,	0,		{FRT, FRB}},
{"machhwsu",	XO (4,	76,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"machhwsu.",	XO (4,	76,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"ps_cmpo1",	X  (4,	96),	XBF_MASK,    PPCPS,	0,		{BF, FRA, FRB}},
{"vaddudm",	VX (4, 192),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vmaxud",	VX (4, 194),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vrld",	VX (4, 196),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vrldmi",	VX (4, 197),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vcmpeqfp",	VXR(4, 198,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpequd",	VXR(4, 199,0),	VXR_MASK,    PPCVEC2,	0,		{VD, VA, VB}},
{"vmuloud",	VX (4, 200),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vdivud",	VX (4, 203),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vpkuwus",	VX (4, 206),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinsw",	VX (4, 207),   VXUIMM4_MASK, POWER10,	0,		{VD, RB, UIMM4}},
{"machhws",	XO (4, 108,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"machhws.",	XO (4, 108,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmachhws",	XO (4, 110,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmachhws.",	XO (4, 110,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vadduqm",	VX (4, 256),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vcmpuq",	VX (4, 257),	VXBF_MASK,   POWER10,	0,		{BF, VA, VB}},
{"vmaxsb",	VX (4, 258),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vslb",	VX (4, 260),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vslq",	VX (4, 261),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vcmpnezb",	VXR(4, 263,0),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"vmulosb",	VX (4, 264),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrefp",	VX (4, 266),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vdivsq",	VX (4, 267),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vmrglb",	VX (4, 268),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vpkshus",	VX (4, 270),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinsbvrx",	VX (4, 271),	VX_MASK,     POWER10,	0,		{VD, RA, VB}},
{"ps_nabs",	XRC(4, 136,0),	XRA_MASK,    PPCPS,	0,		{FRT, FRB}},
{"mulchwu",	XRC(4, 136,0),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"ps_nabs.",	XRC(4, 136,1),	XRA_MASK,    PPCPS,	0,		{FRT, FRB}},
{"mulchwu.",	XRC(4, 136,1),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"macchwu",	XO (4, 140,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"macchwu.",	XO (4, 140,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vaddcuq",	VX (4, 320),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vcmpsq",	VX (4, 321),	VXBF_MASK,   POWER10,	0,		{BF, VA, VB}},
{"vmaxsh",	VX (4, 322),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vslh",	VX (4, 324),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrlqnm",	VX (4, 325),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vcmpnezh",	VXR(4, 327,0),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"vmulosh",	VX (4, 328),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrsqrtefp",	VX (4, 330),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vmrglh",	VX (4, 332),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vpkswus",	VX (4, 334),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinshvrx",	VX (4, 335),	VX_MASK,     POWER10,	0,		{VD, RA, VB}},
{"mulchw",	XRC(4, 168,0),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"mulchw.",	XRC(4, 168,1),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"macchw",	XO (4, 172,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"macchw.",	XO (4, 172,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmacchw",	XO (4, 174,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmacchw.",	XO (4, 174,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vaddcuw",	VX (4, 384),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmaxsw",	VX (4, 386),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vslw",	VX (4, 388),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrlwnm",	VX (4, 389),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vcmpnezw",	VXR(4, 391,0),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"vmulosw",	VX (4, 392),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vexptefp",	VX (4, 394),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vdivsw",	VX (4, 395),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vmrglw",	VX (4, 396),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vclrlb",	VX (4, 397),	VX_MASK,     POWER10,	0,		{VD, VA, RB}},
{"vpkshss",	VX (4, 398),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinswvrx",	VX (4, 399),	VX_MASK,     POWER10,	0,		{VD, RA, VB}},
{"macchwsu",	XO (4, 204,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"macchwsu.",	XO (4, 204,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vmaxsd",	VX (4, 450),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vsl",		VX (4, 452),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrldnm",	VX (4, 453),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vcmpgefp",	VXR(4, 454,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpequq",	VXR(4, 455,0),	VXR_MASK,    POWER10,	0,		{VD, VA, VB}},
{"vmulosd",	VX (4, 456),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vmulld",	VX (4, 457),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vlogefp",	VX (4, 458),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vdivsd",	VX (4, 459),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vclrrb",	VX (4, 461),	VX_MASK,     POWER10,	0,		{VD, VA, RB}},
{"vpkswss",	VX (4, 462),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinsd",	VX (4, 463),   VXUIMM4_MASK, POWER10,	0,		{VD, RB, UIMM4}},
{"macchws",	XO (4, 236,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"macchws.",	XO (4, 236,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmacchws",	XO (4, 238,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmacchws.",	XO (4, 238,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evaddw",	VX (4, 512),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vaddubs",	VX (4, 512),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmul10uq",	VX (4, 513),	VXVB_MASK,   PPCVEC3,	0,		{VD, VA}},
{"evaddiw",	VX (4, 514),	VX_MASK,     PPCSPE,	0,		{RS, RB, UIMM}},
{"vminub",	VX (4, 514),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evsubw",	VX (4, 516),	VX_MASK,     PPCSPE,	EXT,		{RS, RB, RA}},
{"evsubfw",	VX (4, 516),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vsrb",	VX (4, 516),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vsrq",	VX (4, 517),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evsubiw",	VX (4, 518),	VX_MASK,     PPCSPE,	EXT,		{RS, RB, UIMM}},
{"evsubifw",	VX (4, 518),	VX_MASK,     PPCSPE,	0,		{RS, UIMM, RB}},
{"vcmpgtub",	VXR(4, 518,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"evabs",	VX (4, 520),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"vmuleub",	VX (4, 520),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evneg",	VX (4, 521),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evextsb",	VX (4, 522),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"vrfin",	VX (4, 522),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vdiveuq",	VX (4, 523),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evextsh",	VX (4, 523),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evrndw",	VX (4, 524),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"vspltb",	VX (4, 524),   VXUIMM4_MASK, PPCVEC,	0,		{VD, VB, UIMM4}},
{"vextractub",	VX (4, 525),   VXUIMM4_MASK, PPCVEC3,	0,		{VD, VB, UIMM4}},
{"evcntlzw",	VX (4, 525),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evcntlsw",	VX (4, 526),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"vupkhsb",	VX (4, 526),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vinsblx",	VX (4, 527),	VX_MASK,     POWER10,	0,		{VD, RA, RB}},
{"brinc",	VX (4, 527),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"ps_abs",	XRC(4, 264,0),	XRA_MASK,    PPCPS,	0,		{FRT, FRB}},
{"ps_abs.",	XRC(4, 264,1),	XRA_MASK,    PPCPS,	0,		{FRT, FRB}},
{"evand",	VX (4, 529),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evandc",	VX (4, 530),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vsrdbi",	VX (4, 534),	VXSH_MASK,   POWER10,	0,		{VD, VA, VB, SH3}},
{"evxor",	VX (4, 534),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmr",	VX (4, 535),	VX_MASK,     PPCSPE,	EXT,		{RS, RAB}},
{"evor",	VX (4, 535),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evnot",	VX (4, 536),	VX_MASK,     PPCSPE,	EXT,		{RS, RAB}},
{"evnor",	VX (4, 536),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"get",		APU(4, 268,0),	APU_RA_MASK, PPC405,	0,		{RT, FSL}},
{"eveqv",	VX (4, 537),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evorc",	VX (4, 539),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evnand",	VX (4, 542),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evsrwu",	VX (4, 544),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evsrws",	VX (4, 545),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evsrwiu",	VX (4, 546),	VX_MASK,     PPCSPE,	0,		{RS, RA, EVUIMM}},
{"evsrwis",	VX (4, 547),	VX_MASK,     PPCSPE,	0,		{RS, RA, EVUIMM}},
{"evslw",	VX (4, 548),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evslwi",	VX (4, 550),	VX_MASK,     PPCSPE,	0,		{RS, RA, EVUIMM}},
{"evrlw",	VX (4, 552),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evsplati",	VX (4, 553),	VX_MASK,     PPCSPE,	0,		{RS, SIMM}},
{"evrlwi",	VX (4, 554),	VX_MASK,     PPCSPE,	0,		{RS, RA, EVUIMM}},
{"evsplatfi",	VX (4, 555),	VX_MASK,     PPCSPE,	0,		{RS, SIMM}},
{"evmergehi",	VX (4, 556),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmergelo",	VX (4, 557),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmergehilo",	VX (4, 558),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmergelohi",	VX (4, 559),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evcmpgtu",	VX (4, 560),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"evcmpgts",	VX (4, 561),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"evcmpltu",	VX (4, 562),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"evcmplts",	VX (4, 563),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"evcmpeq",	VX (4, 564),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"cget",	APU(4, 284,0),	APU_RA_MASK, PPC405,	0,		{RT, FSL}},
{"vadduhs",	VX (4, 576),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmul10euq",	VX (4, 577),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vminuh",	VX (4, 578),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vsrh",	VX (4, 580),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vcmpgtuh",	VXR(4, 582,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vmuleuh",	VX (4, 584),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vrfiz",	VX (4, 586),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vsplth",	VX (4, 588),   VXUIMM3_MASK, PPCVEC,	0,		{VD, VB, UIMM3}},
{"vextractuh",	VX (4, 589),   VXUIMM4_MASK, PPCVEC3,	0,		{VD, VB, UIMM4}},
{"vupkhsh",	VX (4, 590),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vinshlx",	VX (4, 591),	VX_MASK,     POWER10,	0,		{VD, RA, RB}},
{"nget",	APU(4, 300,0),	APU_RA_MASK, PPC405,	0,		{RT, FSL}},
{"evsel",	EVSEL(4,79),	EVSEL_MASK,  PPCSPE,	0,		{RS, RA, RB, CRFS}},
{"ncget",	APU(4, 316,0),	APU_RA_MASK, PPC405,	0,		{RT, FSL}},
{"evfsadd",	VX (4, 640),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vadduws",	VX (4, 640),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evfssub",	VX (4, 641),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evfsmadd",	VX (4, 642),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vminuw",	VX (4, 642),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evfsmsub",	VX (4, 643),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evfsabs",	VX (4, 644),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"vsrw",	VX (4, 644),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evfsnabs",	VX (4, 645),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evfsneg",	VX (4, 646),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"vcmpgtuw",	VXR(4, 646,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpgtuq",	VXR(4, 647,0),	VXR_MASK,    POWER10,	0,		{VD, VA, VB}},
{"evfssqrt",	VX_RB_CONST(4, 647, 0),  VX_RB_CONST_MASK,	PPCEFS2,	0,		{RD, RA}},
{"vmuleuw",	VX (4, 648),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evfsmul",	VX (4, 648),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vmulhuw",	VX (4, 649),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evfsdiv",	VX (4, 649),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evfsnmadd",	VX (4, 650),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vrfip",	VX (4, 650),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vdiveuw",	VX (4, 651),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evfsnmsub",	VX (4, 651),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evfscmpgt",	VX (4, 652),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"vspltw",	VX (4, 652),   VXUIMM2_MASK, PPCVEC,	0,		{VD, VB, UIMM2}},
{"vextractuw",	VX (4, 653),   VXUIMM4_MASK, PPCVEC3,	0,		{VD, VB, UIMM4}},
{"evfscmplt",	VX (4, 653),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"evfscmpeq",	VX (4, 654),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"vupklsb",	VX (4, 654),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vinswlx",	VX (4, 655),	VX_MASK,     POWER10,	0,		{VD, RA, RB}},
{"evfscfui",	VX (4, 656),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfscfh",	VX_RA_CONST(4, 657, 4),  VX_RA_CONST_MASK,	PPCEFS2,	0,		{RD, RB}},
{"evfscfsi",	VX (4, 657),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfscfuf",	VX (4, 658),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfscfsf",	VX (4, 659),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfsctui",	VX (4, 660),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfscth",	VX_RA_CONST(4, 661, 4),  VX_RA_CONST_MASK,	PPCEFS2,	0,		{RD, RB}},
{"evfsctsi",	VX (4, 661),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfsctuf",	VX (4, 662),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfsctsf",	VX (4, 663),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfsctuiz",	VX (4, 664),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"put",		APU(4, 332,0),	APU_RT_MASK, PPC405,	0,		{RA, FSL}},
{"evfsctsiz",	VX (4, 666),	VX_MASK,     PPCSPE,	0,		{RS, RB}},
{"evfststgt",	VX (4, 668),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"evfststlt",	VX (4, 669),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"evfststeq",	VX (4, 670),	VX_MASK,     PPCSPE,	0,		{CRFD, RA, RB}},
{"evfsmax",	VX (4, 672),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsmin",	VX (4, 673),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsaddsub",	VX (4, 674),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfssubadd",	VX (4, 675),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfssum",	VX (4, 676),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsdiff",	VX (4, 677),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfssumdiff",	VX (4, 678),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsdiffsum",	VX (4, 679),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsaddx",	VX (4, 680),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfssubx",	VX (4, 681),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsaddsubx",	VX (4, 682),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfssubaddx",	VX (4, 683),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsmulx",	VX (4, 684),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsmule",	VX (4, 686),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"evfsmulo",	VX (4, 687),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"efsmax",	VX (4, 688),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"efsmin",	VX (4, 689),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"efdmax",	VX (4, 696),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"cput",	APU(4, 348,0),	APU_RT_MASK, PPC405,	0,		{RA, FSL}},
{"efdmin",	VX (4, 697),	VX_MASK,     PPCEFS2,	0,		{RD, RA, RB}},
{"efsadd",	VX (4, 704),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"evsadd",	VX (4, 704),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"efssub",	VX (4, 705),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"evssub",	VX (4, 705),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"efsmadd",	VX (4, 706),	VX_MASK,     PPCEFS2,	0,		{RS, RA, RB}},
{"vminud",	VX (4, 706),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"efsmsub",	VX (4, 707),	VX_MASK,     PPCEFS2,	0,		{RS, RA, RB}},
{"efsabs",	VX (4, 708),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"evsabs",	VX (4, 708),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"vsr",		VX (4, 708),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"efsnabs",	VX (4, 709),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"evsnabs",	VX (4, 709),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"efsneg",	VX (4, 710),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"evsneg",	VX (4, 710),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"vcmpgtfp",	VXR(4, 710,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"efssqrt",	VX_RB_CONST(4, 711, 0), VX_RB_CONST_MASK,PPCEFS2, 0,	{RD, RA}},
{"vcmpgtud",	VXR(4, 711,0),	VXR_MASK,    PPCVEC2,	0,		{VD, VA, VB}},
{"vmuleud",	VX (4, 712),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"efsmul",	VX (4, 712),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"evsmul",	VX (4, 712),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"vmulhud",	VX (4, 713),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"efsdiv",	VX (4, 713),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"evsdiv",	VX (4, 713),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"efsnmadd",	VX (4, 714),	VX_MASK,     PPCEFS2,	0,		{RS, RA, RB}},
{"vrfim",	VX (4, 714),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vdiveud",	VX (4, 715),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"efsnmsub",	VX (4, 715),	VX_MASK,     PPCEFS2,	0,		{RS, RA, RB}},
{"efscmpgt",	VX (4, 716),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"evscmpgt",	VX (4, 716),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"vextractd",	VX (4, 717),   VXUIMM4_MASK, PPCVEC3,	0,		{VD, VB, UIMM4}},
{"efscmplt",	VX (4, 717),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"evsgmplt",	VX (4, 717),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efscmpeq",	VX (4, 718),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"evsgmpeq",	VX (4, 718),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"vupklsh",	VX (4, 718),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vinsdlx",	VX (4, 719),	VX_MASK,     POWER10,	0,		{VD, RA, RB}},
{"efscfd",	VX (4, 719),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efscfui",	VX (4, 720),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evscfui",	VX (4, 720),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efscfh",	VX_RA_CONST(4, 721, 4), VX_RA_CONST_MASK, PPCEFS2, 0,	{RD, RB}},
{"efscfsi",	VX (4, 721),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evscfsi",	VX (4, 721),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efscfuf",	VX (4, 722),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evscfuf",	VX (4, 722),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efscfsf",	VX (4, 723),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evscfsf",	VX (4, 723),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efsctui",	VX (4, 724),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evsctui",	VX (4, 724),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efscth",	VX_RA_CONST(4, 725, 4), VX_RA_CONST_MASK, PPCEFS2, 0,	{RD, RB}},
{"efsctsi",	VX (4, 725),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evsctsi",	VX (4, 725),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efsctuf",	VX (4, 726),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evsctuf",	VX (4, 726),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efsctsf",	VX (4, 727),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evsctsf",	VX (4, 727),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efsctuiz",	VX (4, 728),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evsctuiz",	VX (4, 728),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"nput",	APU(4, 364,0),	APU_RT_MASK, PPC405,	0,		{RA, FSL}},
{"efsctsiz",	VX (4, 730),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"evsctsiz",	VX (4, 730),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efststgt",	VX (4, 732),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"evststgt",	VX (4, 732),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efststlt",	VX (4, 733),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"evststlt",	VX (4, 733),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efststeq",	VX (4, 734),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"evststeq",	VX (4, 734),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efdadd",	VX (4, 736),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"efdsub",	VX (4, 737),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"efdmadd",	VX (4, 738),	VX_MASK,     PPCEFS2, 	E500|E500MC,	{RD, RA, RB}},
{"efdcfuid",	VX (4, 738),	VX_MASK,     E500|E500MC,0,		{RS, RB}},
{"efdmsub",	VX (4, 739),	VX_MASK,     PPCEFS2, 	E500|E500MC,	{RD, RA, RB}},
{"efdcfsid",	VX (4, 739),	VX_MASK,     E500|E500MC,0,		{RS, RB}},
{"efdabs",	VX (4, 740),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"efdnabs",	VX (4, 741),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"efdneg",	VX (4, 742),	VX_MASK,     PPCEFS,	0,		{RS, RA}},
{"efdsqrt",	VX_RB_CONST(4, 743, 0), VX_RB_CONST_MASK, PPCEFS2, 0,	{RD, RA}},
{"efdmul",	VX (4, 744),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"efddiv",	VX (4, 745),	VX_MASK,     PPCEFS,	0,		{RS, RA, RB}},
{"efdnmadd",	VX (4, 746),	VX_MASK,     PPCEFS2, 	E500|E500MC,	{RD, RA, RB}},
{"efdctuidz",	VX (4, 746),	VX_MASK,     E500|E500MC,0,		{RS, RB}},
{"efdnmsub",	VX (4, 747),	VX_MASK,     PPCEFS2, 	E500|E500MC,	{RD, RA, RB}},
{"efdctsidz",	VX (4, 747),	VX_MASK,     E500|E500MC,0,		{RS, RB}},
{"efdcmpgt",	VX (4, 748),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efdcmplt",	VX (4, 749),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efdcmpeq",	VX (4, 750),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efdcfs",	VX (4, 751),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efdcfui",	VX_RA_CONST(4, 752, 0), VX_RA_CONST_MASK, PPCEFS, 0,	{RS, RB}},
{"efdcfuid",	VX_RA_CONST(4, 752, 1), VX_RA_CONST_MASK, PPCEFS, E500|E500MC,	{RS, RB}},
{"efdcfsi",	VX_RA_CONST(4, 753, 0), VX_RA_CONST_MASK, PPCEFS, 0,	{RS, RB}},
{"efdcfsid",	VX_RA_CONST(4, 753, 1), VX_RA_CONST_MASK, PPCEFS, E500|E500MC,	{RS, RB}},
{"efdcfh",	VX_RA_CONST(4, 753, 4), VX_RA_CONST_MASK, PPCEFS2, 0,	{RD, RB}},
{"efdcfuf",	VX (4, 754),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efdcfsf",	VX (4, 755),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efdctui",	VX (4, 756),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efdcth",	VX_RA_CONST(4, 757, 4), VX_RA_CONST_MASK, PPCEFS2, 0,	{RD, RB}},
{"efdctsi",	VX (4, 757),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efdctuf",	VX (4, 758),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efdctsf",	VX (4, 759),	VX_MASK,     PPCEFS,	0,		{RS, RB}},
{"efdctuiz",	VX_RA_CONST(4, 760, 0), VX_RA_CONST_MASK, PPCEFS, 0,	{RS, RB}},
{"efdctuidz",	VX_RA_CONST(4, 760, 1), VX_RA_CONST_MASK, PPCEFS, E500|E500MC,	{RS, RB}},
{"ncput",	APU(4, 380,0),	APU_RT_MASK, PPC405,	0,		{RA, FSL}},
{"efdctsiz",	VX_RA_CONST(4, 762, 0), VX_RA_CONST_MASK, PPCEFS, 0,	{RS, RB}},
{"efdctsidz",	VX_RA_CONST(4, 762, 1), VX_RA_CONST_MASK, PPCEFS, E500|E500MC,	{RS, RB}},
{"efdtstgt",	VX (4, 764),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efdtstlt",	VX (4, 765),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"efdtsteq",	VX (4, 766),	VX_MASK,     PPCEFS,	0,		{CRFD, RA, RB}},
{"evlddx",	VX (4, 768),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vaddsbs",	VX (4, 768),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evldd",	VX (4, 769),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_8, RA}},
{"evldwx",	VX (4, 770),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vminsb",	VX (4, 770),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evldw",	VX (4, 771),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_8, RA}},
{"evldhx",	VX (4, 772),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vsrab",	VX (4, 772),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vsraq",	VX (4, 773),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evldh",	VX (4, 773),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_8, RA}},
{"vcmpgtsb",	VXR(4, 774,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"evlhhesplatx",VX (4, 776),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vmulesb",	VX (4, 776),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evlhhesplat",	VX (4, 777),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_2, RA}},
{"vcfux",	VX (4, 778),	VX_MASK,     PPCVEC,	0,		{VD, VB, UIMM}},
{"vcuxwfp",	VX (4, 778),	VX_MASK,     PPCVEC,	EXT,		{VD, VB, UIMM}},
{"vdivesq",	VX (4, 779),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evlhhousplatx",VX(4, 780),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vspltisb",	VX (4, 780),	VXVB_MASK,   PPCVEC,	0,		{VD, SIMM}},
{"vinsertb",	VX (4, 781),   VXUIMM4_MASK, PPCVEC3,	0,		{VD, VB, UIMM4}},
{"evlhhousplat",VX (4, 781),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_2, RA}},
{"evlhhossplatx",VX(4, 782),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vpkpx",	VX (4, 782),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vinsbrx",	VX (4, 783),	VX_MASK,     POWER10,	0,		{VD, RA, RB}},
{"evlhhossplat",VX (4, 783),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_2, RA}},
{"mullhwu",	XRC(4, 392,0),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"evlwhex",	VX (4, 784),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"mullhwu.",	XRC(4, 392,1),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"evlwhe",	VX (4, 785),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"evlwhoux",	VX (4, 788),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evlwhou",	VX (4, 789),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"evlwhosx",	VX (4, 790),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evlwhos",	VX (4, 791),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"maclhwu",	XO (4, 396,0,0),XO_MASK,     MULHW,	0,		{RT, RA, RB}},
{"evlwwsplatx",	VX (4, 792),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"maclhwu.",	XO (4, 396,0,1),XO_MASK,     MULHW,	0,		{RT, RA, RB}},
{"evlwwsplat",	VX (4, 793),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"evlwhsplatx",	VX (4, 796),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evlwhsplat",	VX (4, 797),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"evstddx",	VX (4, 800),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evstdd",	VX (4, 801),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_8, RA}},
{"evstdwx",	VX (4, 802),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evstdw",	VX (4, 803),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_8, RA}},
{"evstdhx",	VX (4, 804),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evstdh",	VX (4, 805),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_8, RA}},
{"evstwhex",	VX (4, 816),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evstwhe",	VX (4, 817),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"evstwhox",	VX (4, 820),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evstwho",	VX (4, 821),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"evstwwex",	VX (4, 824),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evstwwe",	VX (4, 825),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"evstwwox",	VX (4, 828),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evstwwo",	VX (4, 829),	VX_MASK,     PPCSPE,	0,		{RS, EVUIMM_4, RA}},
{"vaddshs",	VX (4, 832),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"bcdcpsgn.",	VX (4, 833),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vminsh",	VX (4, 834),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vsrah",	VX (4, 836),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vcmpgtsh",	VXR(4, 838,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vmulesh",	VX (4, 840),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vcfsx",	VX (4, 842),	VX_MASK,     PPCVEC,	0,		{VD, VB, UIMM}},
{"vcsxwfp",	VX (4, 842),	VX_MASK,     PPCVEC,	EXT,		{VD, VB, UIMM}},
{"vspltish",	VX (4, 844),	VXVB_MASK,   PPCVEC,	0,		{VD, SIMM}},
{"vinserth",	VX (4, 845),   VXUIMM4_MASK, PPCVEC3,	0,		{VD, VB, UIMM4}},
{"vupkhpx",	VX (4, 846),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vinshrx",	VX (4, 847),	VX_MASK,     POWER10,	0,		{VD, RA, RB}},
{"mullhw",	XRC(4, 424,0),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"mullhw.",	XRC(4, 424,1),	X_MASK,	     MULHW,	0,		{RT, RA, RB}},
{"maclhw",	XO (4, 428,0,0),XO_MASK,     MULHW,	0,		{RT, RA, RB}},
{"maclhw.",	XO (4, 428,0,1),XO_MASK,     MULHW,	0,		{RT, RA, RB}},
{"nmaclhw",	XO (4, 430,0,0),XO_MASK,     MULHW,	0,		{RT, RA, RB}},
{"nmaclhw.",	XO (4, 430,0,1),XO_MASK,     MULHW,	0,		{RT, RA, RB}},
{"vaddsws",	VX (4, 896),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vminsw",	VX (4, 898),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vsraw",	VX (4, 900),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vcmpgtsw",	VXR(4, 902,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpgtsq",	VXR(4, 903,0),	VXR_MASK,    POWER10,	0,		{VD, VA, VB}},
{"vmulesw",	VX (4, 904),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vmulhsw",	VX (4, 905),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vctuxs",	VX (4, 906),	VX_MASK,     PPCVEC,	0,		{VD, VB, UIMM}},
{"vcfpuxws",	VX (4, 906),	VX_MASK,     PPCVEC,	EXT,		{VD, VB, UIMM}},
{"vdivesw",	VX (4, 907),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vspltisw",	VX (4, 908),	VXVB_MASK,   PPCVEC,	0,		{VD, SIMM}},
{"vinsertw",	VX (4, 909),   VXUIMM4_MASK, PPCVEC3,	0,		{VD, VB, UIMM4}},
{"vinswrx",	VX (4, 911),	VX_MASK,     POWER10,	0,		{VD, RA, RB}},
{"maclhwsu",	XO (4, 460,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"maclhwsu.",	XO (4, 460,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vminsd",	VX (4, 962),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vsrad",	VX (4, 964),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vcmpbfp",	VXR(4, 966,0),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpgtsd",	VXR(4, 967,0),	VXR_MASK,    PPCVEC2,	0,		{VD, VA, VB}},
{"vmulesd",	VX (4, 968),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vmulhsd",	VX (4, 969),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vctsxs",	VX (4, 970),	VX_MASK,     PPCVEC,	0,		{VD, VB, UIMM}},
{"vcfpsxws",	VX (4, 970),	VX_MASK,     PPCVEC,	EXT,		{VD, VB, UIMM}},
{"vdivesd",	VX (4, 971),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vinsertd",	VX (4, 973),   VXUIMM4_MASK, PPCVEC3,	0,		{VD, VB, UIMM4}},
{"vupklpx",	VX (4, 974),	VXVA_MASK,   PPCVEC,	0,		{VD, VB}},
{"vinsdrx",	VX (4, 975),	VX_MASK,     POWER10,	0,		{VD, RA, RB}},
{"maclhws",	XO (4, 492,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"maclhws.",	XO (4, 492,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmaclhws",	XO (4, 494,0,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmaclhws.",	XO (4, 494,0,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vsububm",	VX (4,1024),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"bcdadd.",	VX (4,1025),	VXPS_MASK,   PPCVEC2,	0,		{VD, VA, VB, PS}},
{"vavgub",	VX (4,1026),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vabsdub",	VX (4,1027),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmhessf",	VX (4,1027),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vand",	VX (4,1028),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vcmpequb.",	VXR(4,	 6,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vcmpneb.",	VXR(4,	 7,1),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"udi0fcm.",	APU(4, 515,0),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"udi0fcm",	APU(4, 515,1),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"evmhossf",	VX (4,1031),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vpmsumb",	VX (4,1032),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmheumi",	VX (4,1032),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhesmi",	VX (4,1033),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vmaxfp",	VX (4,1034),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmhesmf",	VX (4,1035),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhoumi",	VX (4,1036),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vslo",	VX (4,1036),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vstribl.",	VXVA(4,1037,0),	VXVA_MASK,   POWER10,	0,		{VD, VB}},
{"vstribr.",	VXVA(4,1037,1),	VXVA_MASK,   POWER10,	0,		{VD, VB}},
{"vstrihl.",	VXVA(4,1037,2),	VXVA_MASK,   POWER10,	0,		{VD, VB}},
{"vstrihr.",	VXVA(4,1037,3),	VXVA_MASK,   POWER10,	0,		{VD, VB}},
{"evmhosmi",	VX (4,1037),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhosmf",	VX (4,1039),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"machhwuo",	XO (4,	12,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"machhwuo.",	XO (4,	12,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"ps_merge00",	XOPS(4,528,0),	XOPS_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"ps_merge00.",	XOPS(4,528,1),	XOPS_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"evmhessfa",	VX (4,1059),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhossfa",	VX (4,1063),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmheumia",	VX (4,1064),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhesmia",	VX (4,1065),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhesmfa",	VX (4,1067),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhoumia",	VX (4,1068),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhosmia",	VX (4,1069),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhosmfa",	VX (4,1071),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vsubuhm",	VX (4,1088),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"bcdsub.",	VX (4,1089),	VXPS_MASK,   PPCVEC2,	0,		{VD, VA, VB, PS}},
{"vavguh",	VX (4,1090),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmwlssf",	VX (4,1091),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vabsduh",	VX (4,1091),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vandc",	VX (4,1092),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vcmpequh.",	VXR(4,	70,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi1fcm.",	APU(4, 547,0),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"udi1fcm",	APU(4, 547,1),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"vcmpneh.",	VXR(4,	71,1),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"evmwhssf",	VX (4,1095),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vpmsumh",	VX (4,1096),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwlumi",	VX (4,1096),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vminfp",	VX (4,1098),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmwlsmf",	VX (4,1099),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhumi",	VX (4,1100),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vsro",	VX (4,1100),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmwhsmi",	VX (4,1101),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vpkudum",	VX (4,1102),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwhsmf",	VX (4,1103),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwssf",	VX (4,1107),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"machhwo",	XO (4,	44,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmwumi",	VX (4,1112),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"machhwo.",	XO (4,	44,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmwsmi",	VX (4,1113),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwsmf",	VX (4,1115),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"nmachhwo",	XO (4,	46,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmachhwo.",	XO (4,	46,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"ps_merge01",	XOPS(4,560,0),	XOPS_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"ps_merge01.",	XOPS(4,560,1),	XOPS_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"evmwlssfa",	VX (4,1123),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhssfa",	VX (4,1127),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwlumia",	VX (4,1128),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwlsmfa",	VX (4,1131),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhumia",	VX (4,1132),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwhsmia",	VX (4,1133),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwhsmfa",	VX (4,1135),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwssfa",	VX (4,1139),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwumia",	VX (4,1144),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwsmia",	VX (4,1145),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwsmfa",	VX (4,1147),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vsubuwm",	VX (4,1152),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"bcdus.",	VX (4,1153),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vavguw",	VX (4,1154),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vabsduw",	VX (4,1155),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vmr",		VX (4,1156),	VX_MASK,     PPCVEC,	EXT,		{VD, VAB}},
{"vor",		VX (4,1156),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vcmpnew.",	VXR(4, 135,1),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"vpmsumw",	VX (4,1160),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vcmpequw.",	VXR(4, 134,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi2fcm.",	APU(4, 579,0),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"udi2fcm",	APU(4, 579,1),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"machhwsuo",	XO (4,	76,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"machhwsuo.",	XO (4,	76,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"ps_merge10",	XOPS(4,592,0),	XOPS_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"ps_merge10.",	XOPS(4,592,1),	XOPS_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"vsubudm",	VX (4,1216),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evaddusiaaw",	VX (4,1216),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"bcds.",	VX (4,1217),	VXPS_MASK,   PPCVEC3,	0,		{VD, VA, VB, PS}},
{"evaddssiaaw",	VX (4,1217),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evsubfusiaaw",VX (4,1218),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evsubfssiaaw",VX (4,1219),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evmra",	VX (4,1220),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"vxor",	VX (4,1220),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evdivws",	VX (4,1222),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vcmpeqfp.",	VXR(4, 198,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi3fcm.",	APU(4, 611,0),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"vcmpequd.",	VXR(4, 199,1),	VXR_MASK,    PPCVEC2,	0,		{VD, VA, VB}},
{"udi3fcm",	APU(4, 611,1),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"evdivwu",	VX (4,1223),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vpmsumd",	VX (4,1224),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evaddumiaaw",	VX (4,1224),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evaddsmiaaw",	VX (4,1225),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evsubfumiaaw",VX (4,1226),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"evsubfsmiaaw",VX (4,1227),	VX_MASK,     PPCSPE,	0,		{RS, RA}},
{"vgnb",	VX (4,1228),	VX_MASK,     POWER10,	0,		{RT, VB, UIMM3}},
{"vpkudus",	VX (4,1230),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"machhwso",	XO (4, 108,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"machhwso.",	XO (4, 108,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmachhwso",	XO (4, 110,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmachhwso.",	XO (4, 110,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"ps_merge11",	XOPS(4,624,0),	XOPS_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"ps_merge11.",	XOPS(4,624,1),	XOPS_MASK,   PPCPS,	0,		{FRT, FRA, FRB}},
{"vsubuqm",	VX (4,1280),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmheusiaaw",	VX (4,1280),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"bcdtrunc.",	VX (4,1281),	VXPS_MASK,   PPCVEC3,	0,		{VD, VA, VB, PS}},
{"evmhessiaaw",	VX (4,1281),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vavgsb",	VX (4,1282),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmhessfaaw",	VX (4,1283),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhousiaaw",	VX (4,1284),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vnot",	VX (4,1284),	VX_MASK,     PPCVEC,	EXT,		{VD, VAB}},
{"vnor",	VX (4,1284),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmhossiaaw",	VX (4,1285),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"udi4fcm.",	APU(4, 643,0),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"udi4fcm",	APU(4, 643,1),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"vcmpnezb.",	VXR(4, 263,1),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"evmhossfaaw",	VX (4,1287),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmheumiaaw",	VX (4,1288),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vcipher",	VX (4,1288),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vcipherlast",	VX (4,1289),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmhesmiaaw",	VX (4,1289),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhesmfaaw",	VX (4,1291),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vgbbd",	VX (4,1292),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"evmhoumiaaw",	VX (4,1292),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhosmiaaw",	VX (4,1293),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhosmfaaw",	VX (4,1295),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"macchwuo",	XO (4, 140,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"macchwuo.",	XO (4, 140,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmhegumiaa",	VX (4,1320),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhegsmiaa",	VX (4,1321),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhegsmfaa",	VX (4,1323),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhogumiaa",	VX (4,1324),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhogsmiaa",	VX (4,1325),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhogsmfaa",	VX (4,1327),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vsubcuq",	VX (4,1344),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwlusiaaw",	VX (4,1344),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"bcdutrunc.",	VX (4,1345),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"evmwlssiaaw",	VX (4,1345),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vavgsh",	VX (4,1346),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmwlssfaaw",	VX (4,1347),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhusiaa",	VX (4,1348),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vorc",	VX (4,1348),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwhssmaa",	VX (4,1349),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"udi5fcm.",	APU(4, 675,0),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"udi5fcm",	APU(4, 675,1),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"vcmpnezh.",	VXR(4, 327,1),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"evmwhssfaa",	VX (4,1351),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vncipher",	VX (4,1352),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwlumiaaw",	VX (4,1352),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vncipherlast",VX (4,1353),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwlsmiaaw",	VX (4,1353),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwlsmfaaw",	VX (4,1355),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhumiaa",	VX (4,1356),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vbpermq",	VX (4,1356),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vcfuged",	VX (4,1357),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evmwhsmiaa",	VX (4,1357),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vpksdus",	VX (4,1358),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwhsmfaa",	VX (4,1359),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwssfaa",	VX (4,1363),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"macchwo",	XO (4, 172,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmwumiaa",	VX (4,1368),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"macchwo.",	XO (4, 172,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmwsmiaa",	VX (4,1369),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwsmfaa",	VX (4,1371),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"nmacchwo",	XO (4, 174,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmacchwo.",	XO (4, 174,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmwhgumiaa",	VX (4,1380),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhgsmiaa",	VX (4,1381),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhgssfaa",	VX (4,1383),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhgsmfaa",	VX (4,1391),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmheusianw",	VX (4,1408),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vsubcuw",	VX (4,1408),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmhessianw",	VX (4,1409),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"bcdctsq.",	VXVA(4,1409,0),	VXVA_MASK,   PPCVEC3,	0,		{VD, VB}},
{"bcdcfsq.",	VXVA(4,1409,2),	VXVAPS_MASK, PPCVEC3,	0,		{VD, VB, PS}},
{"bcdctz.",	VXVA(4,1409,4),	VXVAPS_MASK, PPCVEC3,	0,		{VD, VB, PS}},
{"bcdctn.",	VXVA(4,1409,5),	VXVA_MASK,   PPCVEC3,	0,		{VD, VB}},
{"bcdcfz.",	VXVA(4,1409,6),	VXVAPS_MASK, PPCVEC3,	0,		{VD, VB, PS}},
{"bcdcfn.",	VXVA(4,1409,7),	VXVAPS_MASK, PPCVEC3,	0,		{VD, VB, PS}},
{"bcdsetsgn.",	VXVA(4,1409,31), VXVAPS_MASK, PPCVEC3,	0,		{VD, VB, PS}},
{"vavgsw",	VX (4,1410),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"evmhessfanw",	VX (4,1411),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vnand",	VX (4,1412),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmhousianw",	VX (4,1412),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhossianw",	VX (4,1413),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"udi6fcm.",	APU(4, 707,0),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"udi6fcm",	APU(4, 707,1),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"vcmpnezw.",	VXR(4, 391,1),	VXR_MASK,    PPCVEC3,	0,		{VD, VA, VB}},
{"evmhossfanw",	VX (4,1415),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmheumianw",	VX (4,1416),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhesmianw",	VX (4,1417),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhesmfanw",	VX (4,1419),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhoumianw",	VX (4,1420),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"vpextd",	VX (4,1421),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evmhosmianw",	VX (4,1421),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhosmfanw",	VX (4,1423),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"macchwsuo",	XO (4, 204,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"macchwsuo.",	XO (4, 204,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmhegumian",	VX (4,1448),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhegsmian",	VX (4,1449),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhegsmfan",	VX (4,1451),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhogumian",	VX (4,1452),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhogsmian",	VX (4,1453),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmhogsmfan",	VX (4,1455),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwlusianw",	VX (4,1472),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"bcdsr.",	VX (4,1473),	VXPS_MASK,   PPCVEC3,	0,		{VD, VA, VB, PS}},
{"evmwlssianw",	VX (4,1473),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwlssfanw",	VX (4,1475),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhusian",	VX (4,1476),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vsld",	VX (4,1476),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwhssian",	VX (4,1477),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vcmpgefp.",	VXR(4, 454,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi7fcm.",	APU(4, 739,0),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"udi7fcm",	APU(4, 739,1),	APU_MASK, PPC405|PPC440, 0,		{URT, URA, URB}},
{"vcmpequq.",	VXR(4, 455,1),	VXR_MASK,    POWER10,	0,		{VD, VA, VB}},
{"evmwhssfan",	VX (4,1479),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vsbox",	VX (4,1480),	VXVB_MASK,   PPCVEC2,	0,		{VD, VA}},
{"evmwlumianw",	VX (4,1480),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwlsmianw",	VX (4,1481),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwlsmfanw",	VX (4,1483),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhumian",	VX (4,1484),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vbpermd",	VX (4,1484),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vpdepd",	VX (4,1485),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"evmwhsmian",	VX (4,1485),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"vpksdss",	VX (4,1486),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"evmwhsmfan",	VX (4,1487),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwssfan",	VX (4,1491),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"macchwso",	XO (4, 236,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmwumian",	VX (4,1496),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"macchwso.",	XO (4, 236,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"evmwsmian",	VX (4,1497),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwsmfan",	VX (4,1499),	VX_MASK,     PPCSPE,	0,		{RS, RA, RB}},
{"evmwhgumian",	VX (4,1508),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhgsmian",	VX (4,1509),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhgssfan",	VX (4,1511),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"evmwhgsmfan",	VX (4,1519),	VX_MASK,     PPCSPE,	0,		{RD, RA, RB}},
{"nmacchwso",	XO (4, 238,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmacchwso.",	XO (4, 238,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vsububs",	VX (4,1536),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vclzlsbb",	VXVA(4,1538,0), VXVA_MASK,   PPCVEC3,	0,		{RT, VB}},
{"vctzlsbb",	VXVA(4,1538,1), VXVA_MASK,   PPCVEC3,	0,		{RT, VB}},
{"vnegw",	VXVA(4,1538,6), VXVA_MASK,   PPCVEC3,	0,		{VD, VB}},
{"vnegd",	VXVA(4,1538,7), VXVA_MASK,   PPCVEC3,	0,		{VD, VB}},
{"vprtybw",	VXVA(4,1538,8), VXVA_MASK,   PPCVEC3,	0,		{VD, VB}},
{"vprtybd",	VXVA(4,1538,9), VXVA_MASK,   PPCVEC3,	0,		{VD, VB}},
{"vprtybq",	VXVA(4,1538,10), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vextsb2w",	VXVA(4,1538,16), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vextsh2w",	VXVA(4,1538,17), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vextsb2d",	VXVA(4,1538,24), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vextsh2d",	VXVA(4,1538,25), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vextsw2d",	VXVA(4,1538,26), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vextsd2q",	VXVA(4,1538,27), VXVA_MASK,  POWER10,	0,		{VD, VB}},
{"vctzb",	VXVA(4,1538,28), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vctzh",	VXVA(4,1538,29), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vctzw",	VXVA(4,1538,30), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"vctzd",	VXVA(4,1538,31), VXVA_MASK,  PPCVEC3,	0,		{VD, VB}},
{"mfvscr",	VX (4,1540),	VXVAVB_MASK, PPCVEC,	0,		{VD}},
{"vcmpgtub.",	VXR(4, 518,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi8fcm.",	APU(4, 771,0),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"udi8fcm",	APU(4, 771,1),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vsum4ubs",	VX (4,1544),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmoduq",	VX (4,1547),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vextublx",	VX (4,1549),	VX_MASK,     PPCVEC3,	0,		{RT, RA, VB}},
{"vsubuhs",	VX (4,1600),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},

{"vexpandbm",	VXVA(4,1602,0),  VXVA_MASK,  POWER10,	0,		{VD, VB}},
{"vexpandhm",	VXVA(4,1602,1),  VXVA_MASK,  POWER10,	0,		{VD, VB}},
{"vexpandwm",	VXVA(4,1602,2),  VXVA_MASK,  POWER10,	0,		{VD, VB}},
{"vexpanddm",	VXVA(4,1602,3),  VXVA_MASK,  POWER10,	0,		{VD, VB}},
{"vexpandqm",	VXVA(4,1602,4),  VXVA_MASK,  POWER10,	0,		{VD, VB}},
{"vextractbm",	VXVA(4,1602,8),  VXVA_MASK,  POWER10,	0,		{RT, VB}},
{"vextracthm",	VXVA(4,1602,9),  VXVA_MASK,  POWER10,	0,		{RT, VB}},
{"vextractwm",	VXVA(4,1602,10), VXVA_MASK,  POWER10,	0,		{RT, VB}},
{"vextractdm",	VXVA(4,1602,11), VXVA_MASK,  POWER10,	0,		{RT, VB}},
{"vextractqm",	VXVA(4,1602,12), VXVA_MASK,  POWER10,	0,		{RT, VB}},
{"mtvsrbm",	VXVA(4,1602,16), VXVA_MASK,  POWER10,	0,		{VD, RB}},
{"mtvsrhm",	VXVA(4,1602,17), VXVA_MASK,  POWER10,	0,		{VD, RB}},
{"mtvsrwm",	VXVA(4,1602,18), VXVA_MASK,  POWER10,	0,		{VD, RB}},
{"mtvsrdm",	VXVA(4,1602,19), VXVA_MASK,  POWER10,	0,		{VD, RB}},
{"mtvsrqm",	VXVA(4,1602,20), VXVA_MASK,  POWER10,	0,		{VD, RB}},
{"vcntmbb",	VXVA(4,1602,24), VXVAM_MASK, POWER10,	0,		{RT, VB, MP}},
{"vcntmbh",	VXVA(4,1602,26), VXVAM_MASK, POWER10,	0,		{RT, VB, MP}},
{"vcntmbw",	VXVA(4,1602,28), VXVAM_MASK, POWER10,	0,		{RT, VB, MP}},
{"vcntmbd",	VXVA(4,1602,30), VXVAM_MASK, POWER10,	0,		{RT, VB, MP}},

{"mtvscr",	VX (4,1604),	VXVDVA_MASK, PPCVEC,	0,		{VB}},
{"vcmpgtuh.",	VXR(4, 582,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vsum4shs",	VX (4,1608),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"udi9fcm.",	APU(4, 804,0),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"udi9fcm",	APU(4, 804,1),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vextuhlx",	VX (4,1613),	VX_MASK,     PPCVEC3,	0,		{RT, RA, VB}},
{"vupkhsw",	VX (4,1614),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vsubuws",	VX (4,1664),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vshasigmaw",	VX (4,1666),	VX_MASK,     PPCVEC2,	0,		{VD, VA, ST, SIX}},
{"veqv",	VX (4,1668),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vcmpgtuw.",	VXR(4, 646,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi10fcm.",	APU(4, 835,0),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vcmpgtuq.",	VXR(4, 647,1),	VXR_MASK,    POWER10,	0,		{VD, VA, VB}},
{"udi10fcm",	APU(4, 835,1),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vsum2sws",	VX (4,1672),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmoduw",	VX (4,1675),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vmrgow",	VX (4,1676),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vextuwlx",	VX (4,1677),	VX_MASK,     PPCVEC3,	0,		{RT, RA, VB}},
{"vshasigmad",	VX (4,1730),	VX_MASK,     PPCVEC2,	0,		{VD, VA, ST, SIX}},
{"vsrd",	VX (4,1732),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vcmpgtfp.",	VXR(4, 710,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi11fcm.",	APU(4, 867,0),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vcmpgtud.",	VXR(4, 711,1),	VXR_MASK,    PPCVEC2,	0,		{VD, VA, VB}},
{"udi11fcm",	APU(4, 867,1),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vmodud",	VX (4,1739),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vupklsw",	VX (4,1742),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vsubsbs",	VX (4,1792),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vclzb",	VX (4,1794),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vpopcntb",	VX (4,1795),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vsrv",	VX (4,1796),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vcmpgtsb.",	VXR(4, 774,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi12fcm.",	APU(4, 899,0),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"udi12fcm",	APU(4, 899,1),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vsum4sbs",	VX (4,1800),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmodsq",	VX (4,1803),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vextubrx",	VX (4,1805),	VX_MASK,     PPCVEC3,	0,		{RT, RA, VB}},
{"maclhwuo",	XO (4, 396,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"maclhwuo.",	XO (4, 396,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vsubshs",	VX (4,1856),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vclzh",	VX (4,1858),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vpopcnth",	VX (4,1859),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vslv",	VX (4,1860),	VX_MASK,     PPCVEC3,	0,		{VD, VA, VB}},
{"vcmpgtsh.",	VXR(4, 838,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"vextuhrx",	VX (4,1869),	VX_MASK,     PPCVEC3,	0,		{RT, RA, VB}},
{"udi13fcm.",	APU(4, 931,0),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"udi13fcm",	APU(4, 931,1),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"maclhwo",	XO (4, 428,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"maclhwo.",	XO (4, 428,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmaclhwo",	XO (4, 430,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmaclhwo.",	XO (4, 430,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vsubsws",	VX (4,1920),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vclzw",	VX (4,1922),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vpopcntw",	VX (4,1923),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vclzdm",	VX (4,1924),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vcmpgtsw.",	VXR(4, 902,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi14fcm.",	APU(4, 963,0),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vcmpgtsq.",	VXR(4, 903,1),	VXR_MASK,    POWER10,	0,		{VD, VA, VB}},
{"udi14fcm",	APU(4, 963,1),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vsumsws",	VX (4,1928),	VX_MASK,     PPCVEC,	0,		{VD, VA, VB}},
{"vmodsw",	VX (4,1931),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vmrgew",	VX (4,1932),	VX_MASK,     PPCVEC2,	0,		{VD, VA, VB}},
{"vextuwrx",	VX (4,1933),	VX_MASK,     PPCVEC3,	0,		{RT, RA, VB}},
{"maclhwsuo",	XO (4, 460,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"maclhwsuo.",	XO (4, 460,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"vclzd",	VX (4,1986),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vpopcntd",	VX (4,1987),	VXVA_MASK,   PPCVEC2,	0,		{VD, VB}},
{"vctzdm",	VX (4,1988),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"vcmpbfp.",	VXR(4, 966,1),	VXR_MASK,    PPCVEC,	0,		{VD, VA, VB}},
{"udi15fcm.",	APU(4, 995,0),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vcmpgtsd.",	VXR(4, 967,1),	VXR_MASK,    PPCVEC2,	0,		{VD, VA, VB}},
{"udi15fcm",	APU(4, 995,1),	APU_MASK,    PPC440,	0,		{URT, URA, URB}},
{"vmodsd",	VX (4,1995),	VX_MASK,     POWER10,	0,		{VD, VA, VB}},
{"maclhwso",	XO (4, 492,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"maclhwso.",	XO (4, 492,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmaclhwso",	XO (4, 494,1,0), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"nmaclhwso.",	XO (4, 494,1,1), XO_MASK,    MULHW,	0,		{RT, RA, RB}},
{"dcbz_l",	X  (4,1014),	XRT_MASK,    PPCPS,	0,		{RA, RB}},

{"lxvp",	DQXP(6,0),	DQXP_MASK,   POWER10,	PPCVLE,		{XTP, DQ, RA0}},
{"stxvp",	DQXP(6,1),	DQXP_MASK,   POWER10,	PPCVLE,		{XSP, DQ, RA0}},

{"mulli",	OP(7),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, RA, SI}},
{"muli",	OP(7),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, RA, SI}},

{"subfic",	OP(8),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, RA, SI}},
{"sfi",		OP(8),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, RA, SI}},

{"dozi",	OP(9),		OP_MASK,     M601,	PPCVLE,		{RT, RA, SI}},

{"cmplwi",	OPL(10,0),	OPL_MASK,    PPCCOM,	PPCVLE|EXT,	{OBF, RA, UISIGNOPT}},
{"cmpldi",	OPL(10,1),	OPL_MASK,    PPC64,	PPCVLE|EXT,	{OBF, RA, UISIGNOPT}},
{"cmpli",	OP(10),		OP_MASK,     PPC,	PPCVLE,		{BF, L32OPT, RA, UISIGNOPT}},
{"cmpli",	OP(10),		OP_MASK,     PWRCOM,	PPC|PPCVLE,	{BF, RA, UISIGNOPT}},

{"cmpwi",	OPL(11,0),	OPL_MASK,    PPCCOM,	PPCVLE|EXT,	{OBF, RA, SI}},
{"cmpdi",	OPL(11,1),	OPL_MASK,    PPC64,	PPCVLE|EXT,	{OBF, RA, SI}},
{"cmpi",	OP(11),		OP_MASK,     PPC,	PPCVLE,		{BF, L32OPT, RA, SI}},
{"cmpi",	OP(11),		OP_MASK,     PWRCOM,	PPC|PPCVLE,	{BF, RA, SI}},

{"addic",	OP(12),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, RA, SI}},
{"ai",		OP(12),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, RA, SI}},
{"subic",	OP(12),		OP_MASK,     PPCCOM,	PPCVLE|EXT,	{RT, RA, NSI}},

{"addic.",	OP(13),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, RA, SI}},
{"ai.",		OP(13),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, RA, SI}},
{"subic.",	OP(13),		OP_MASK,     PPCCOM,	PPCVLE|EXT,	{RT, RA, NSI}},

{"li",		OP(14),		DRA_MASK,    PPCCOM,	PPCVLE|EXT,	{RT, SI}},
{"lil",		OP(14),		DRA_MASK,    PWRCOM,	PPCVLE|EXT,	{RT, SI}},
{"addi",	OP(14),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, RA0, SI}},
{"cal",		OP(14),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, D, RA0}},
{"subi",	OP(14),		OP_MASK,     PPCCOM,	PPCVLE|EXT,	{RT, RA0, NSI}},
{"la",		OP(14),		OP_MASK,     PPCCOM,	PPCVLE|EXT,	{RT, D, RA0}},

{"lis",		OP(15),		DRA_MASK,    PPCCOM,	PPCVLE|EXT,	{RT, SISIGNOPT}},
{"liu",		OP(15),		DRA_MASK,    PWRCOM,	PPCVLE|EXT,	{RT, SISIGNOPT}},
{"addis",	OP(15),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, RA0, SISIGNOPT}},
{"cau",		OP(15),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, RA0, SISIGNOPT}},
{"subis",	OP(15),		OP_MASK,     PPCCOM,	PPCVLE|EXT,	{RT, RA0, NSISIGNOPT}},

{"bdnz-",    BBO(16,BODNZ,0,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDM}},
{"bdnz+",    BBO(16,BODNZ,0,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDP}},
{"bdnz",     BBO(16,BODNZ,0,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BD}},
{"bdn",	     BBO(16,BODNZ,0,0),		BBOATBI_MASK,  PWRCOM,	 PPCVLE|EXT,	{BD}},
{"bdnzl-",   BBO(16,BODNZ,0,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDM}},
{"bdnzl+",   BBO(16,BODNZ,0,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDP}},
{"bdnzl",    BBO(16,BODNZ,0,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BD}},
{"bdnl",     BBO(16,BODNZ,0,1),		BBOATBI_MASK,  PWRCOM,	 PPCVLE|EXT,	{BD}},
{"bdnza-",   BBO(16,BODNZ,1,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDMA}},
{"bdnza+",   BBO(16,BODNZ,1,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDPA}},
{"bdnza",    BBO(16,BODNZ,1,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDA}},
{"bdna",     BBO(16,BODNZ,1,0),		BBOATBI_MASK,  PWRCOM,	 PPCVLE|EXT,	{BDA}},
{"bdnzla-",  BBO(16,BODNZ,1,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDMA}},
{"bdnzla+",  BBO(16,BODNZ,1,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDPA}},
{"bdnzla",   BBO(16,BODNZ,1,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDA}},
{"bdnla",    BBO(16,BODNZ,1,1),		BBOATBI_MASK,  PWRCOM,	 PPCVLE|EXT,	{BDA}},
{"bdz-",     BBO(16,BODZ,0,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDM}},
{"bdz+",     BBO(16,BODZ,0,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDP}},
{"bdz",	     BBO(16,BODZ,0,0),		BBOATBI_MASK,  COM,	 PPCVLE|EXT,	{BD}},
{"bdzl-",    BBO(16,BODZ,0,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDM}},
{"bdzl+",    BBO(16,BODZ,0,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDP}},
{"bdzl",     BBO(16,BODZ,0,1),		BBOATBI_MASK,  COM,	 PPCVLE|EXT,	{BD}},
{"bdza-",    BBO(16,BODZ,1,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDMA}},
{"bdza+",    BBO(16,BODZ,1,0),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDPA}},
{"bdza",     BBO(16,BODZ,1,0),		BBOATBI_MASK,  COM,	 PPCVLE|EXT,	{BDA}},
{"bdzla-",   BBO(16,BODZ,1,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDMA}},
{"bdzla+",   BBO(16,BODZ,1,1),		BBOATBI_MASK,  PPCCOM,	 PPCVLE|EXT,	{BDPA}},
{"bdzla",    BBO(16,BODZ,1,1),		BBOATBI_MASK,  COM,	 PPCVLE|EXT,	{BDA}},

{"bge-",     BBOCB(16,BOF,CBLT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bge+",     BBOCB(16,BOF,CBLT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bge",	     BBOCB(16,BOF,CBLT,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bnl-",     BBOCB(16,BOF,CBLT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bnl+",     BBOCB(16,BOF,CBLT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bnl",	     BBOCB(16,BOF,CBLT,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bgel-",    BBOCB(16,BOF,CBLT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bgel+",    BBOCB(16,BOF,CBLT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bgel",     BBOCB(16,BOF,CBLT,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bnll-",    BBOCB(16,BOF,CBLT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bnll+",    BBOCB(16,BOF,CBLT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bnll",     BBOCB(16,BOF,CBLT,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bgea-",    BBOCB(16,BOF,CBLT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bgea+",    BBOCB(16,BOF,CBLT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bgea",     BBOCB(16,BOF,CBLT,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bnla-",    BBOCB(16,BOF,CBLT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnla+",    BBOCB(16,BOF,CBLT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnla",     BBOCB(16,BOF,CBLT,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bgela-",   BBOCB(16,BOF,CBLT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bgela+",   BBOCB(16,BOF,CBLT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bgela",    BBOCB(16,BOF,CBLT,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bnlla-",   BBOCB(16,BOF,CBLT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnlla+",   BBOCB(16,BOF,CBLT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnlla",    BBOCB(16,BOF,CBLT,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"ble-",     BBOCB(16,BOF,CBGT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"ble+",     BBOCB(16,BOF,CBGT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"ble",	     BBOCB(16,BOF,CBGT,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bng-",     BBOCB(16,BOF,CBGT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bng+",     BBOCB(16,BOF,CBGT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bng",	     BBOCB(16,BOF,CBGT,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"blel-",    BBOCB(16,BOF,CBGT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"blel+",    BBOCB(16,BOF,CBGT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"blel",     BBOCB(16,BOF,CBGT,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bngl-",    BBOCB(16,BOF,CBGT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bngl+",    BBOCB(16,BOF,CBGT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bngl",     BBOCB(16,BOF,CBGT,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"blea-",    BBOCB(16,BOF,CBGT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"blea+",    BBOCB(16,BOF,CBGT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"blea",     BBOCB(16,BOF,CBGT,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bnga-",    BBOCB(16,BOF,CBGT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnga+",    BBOCB(16,BOF,CBGT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnga",     BBOCB(16,BOF,CBGT,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"blela-",   BBOCB(16,BOF,CBGT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"blela+",   BBOCB(16,BOF,CBGT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"blela",    BBOCB(16,BOF,CBGT,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bngla-",   BBOCB(16,BOF,CBGT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bngla+",   BBOCB(16,BOF,CBGT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bngla",    BBOCB(16,BOF,CBGT,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bne-",     BBOCB(16,BOF,CBEQ,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bne+",     BBOCB(16,BOF,CBEQ,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bne",	     BBOCB(16,BOF,CBEQ,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bnel-",    BBOCB(16,BOF,CBEQ,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bnel+",    BBOCB(16,BOF,CBEQ,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bnel",     BBOCB(16,BOF,CBEQ,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bnea-",    BBOCB(16,BOF,CBEQ,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnea+",    BBOCB(16,BOF,CBEQ,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnea",     BBOCB(16,BOF,CBEQ,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bnela-",   BBOCB(16,BOF,CBEQ,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnela+",   BBOCB(16,BOF,CBEQ,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnela",    BBOCB(16,BOF,CBEQ,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bns-",     BBOCB(16,BOF,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bns+",     BBOCB(16,BOF,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bns",	     BBOCB(16,BOF,CBSO,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bnu-",     BBOCB(16,BOF,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bnu+",     BBOCB(16,BOF,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bnu",	     BBOCB(16,BOF,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BD}},
{"bnsl-",    BBOCB(16,BOF,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bnsl+",    BBOCB(16,BOF,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bnsl",     BBOCB(16,BOF,CBSO,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bnul-",    BBOCB(16,BOF,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bnul+",    BBOCB(16,BOF,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bnul",     BBOCB(16,BOF,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BD}},
{"bnsa-",    BBOCB(16,BOF,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnsa+",    BBOCB(16,BOF,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnsa",     BBOCB(16,BOF,CBSO,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bnua-",    BBOCB(16,BOF,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnua+",    BBOCB(16,BOF,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnua",     BBOCB(16,BOF,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDA}},
{"bnsla-",   BBOCB(16,BOF,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnsla+",   BBOCB(16,BOF,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnsla",    BBOCB(16,BOF,CBSO,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bnula-",   BBOCB(16,BOF,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bnula+",   BBOCB(16,BOF,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bnula",    BBOCB(16,BOF,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDA}},

{"blt-",     BBOCB(16,BOT,CBLT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"blt+",     BBOCB(16,BOT,CBLT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"blt",	     BBOCB(16,BOT,CBLT,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bltl-",    BBOCB(16,BOT,CBLT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bltl+",    BBOCB(16,BOT,CBLT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bltl",     BBOCB(16,BOT,CBLT,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"blta-",    BBOCB(16,BOT,CBLT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"blta+",    BBOCB(16,BOT,CBLT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"blta",     BBOCB(16,BOT,CBLT,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bltla-",   BBOCB(16,BOT,CBLT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bltla+",   BBOCB(16,BOT,CBLT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bltla",    BBOCB(16,BOT,CBLT,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bgt-",     BBOCB(16,BOT,CBGT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bgt+",     BBOCB(16,BOT,CBGT,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bgt",	     BBOCB(16,BOT,CBGT,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bgtl-",    BBOCB(16,BOT,CBGT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bgtl+",    BBOCB(16,BOT,CBGT,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bgtl",     BBOCB(16,BOT,CBGT,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bgta-",    BBOCB(16,BOT,CBGT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bgta+",    BBOCB(16,BOT,CBGT,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bgta",     BBOCB(16,BOT,CBGT,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bgtla-",   BBOCB(16,BOT,CBGT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bgtla+",   BBOCB(16,BOT,CBGT,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bgtla",    BBOCB(16,BOT,CBGT,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"beq-",     BBOCB(16,BOT,CBEQ,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"beq+",     BBOCB(16,BOT,CBEQ,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"beq",	     BBOCB(16,BOT,CBEQ,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"beql-",    BBOCB(16,BOT,CBEQ,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"beql+",    BBOCB(16,BOT,CBEQ,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"beql",     BBOCB(16,BOT,CBEQ,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"beqa-",    BBOCB(16,BOT,CBEQ,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"beqa+",    BBOCB(16,BOT,CBEQ,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"beqa",     BBOCB(16,BOT,CBEQ,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"beqla-",   BBOCB(16,BOT,CBEQ,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"beqla+",   BBOCB(16,BOT,CBEQ,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"beqla",    BBOCB(16,BOT,CBEQ,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bso-",     BBOCB(16,BOT,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bso+",     BBOCB(16,BOT,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bso",	     BBOCB(16,BOT,CBSO,0,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bun-",     BBOCB(16,BOT,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bun+",     BBOCB(16,BOT,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bun",	     BBOCB(16,BOT,CBSO,0,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BD}},
{"bsol-",    BBOCB(16,BOT,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bsol+",    BBOCB(16,BOT,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bsol",     BBOCB(16,BOT,CBSO,0,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BD}},
{"bunl-",    BBOCB(16,BOT,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDM}},
{"bunl+",    BBOCB(16,BOT,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDP}},
{"bunl",     BBOCB(16,BOT,CBSO,0,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BD}},
{"bsoa-",    BBOCB(16,BOT,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bsoa+",    BBOCB(16,BOT,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bsoa",     BBOCB(16,BOT,CBSO,1,0),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"buna-",    BBOCB(16,BOT,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"buna+",    BBOCB(16,BOT,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"buna",     BBOCB(16,BOT,CBSO,1,0),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDA}},
{"bsola-",   BBOCB(16,BOT,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bsola+",   BBOCB(16,BOT,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bsola",    BBOCB(16,BOT,CBSO,1,1),	BBOATCB_MASK,  COM,	 PPCVLE|EXT,	{CR, BDA}},
{"bunla-",   BBOCB(16,BOT,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDMA}},
{"bunla+",   BBOCB(16,BOT,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDPA}},
{"bunla",    BBOCB(16,BOT,CBSO,1,1),	BBOATCB_MASK,  PPCCOM,	 PPCVLE|EXT,	{CR, BDA}},

{"bdnzf-",   BBO(16,BODNZF,0,0),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDM}},
{"bdnzf+",   BBO(16,BODNZF,0,0),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDP}},
{"bdnzf",    BBO(16,BODNZF,0,0),	BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BD}},
{"bdnzfl-",  BBO(16,BODNZF,0,1),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDM}},
{"bdnzfl+",  BBO(16,BODNZF,0,1),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDP}},
{"bdnzfl",   BBO(16,BODNZF,0,1),	BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BD}},
{"bdnzfa-",  BBO(16,BODNZF,1,0),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDMA}},
{"bdnzfa+",  BBO(16,BODNZF,1,0),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDPA}},
{"bdnzfa",   BBO(16,BODNZF,1,0),	BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BDA}},
{"bdnzfla-", BBO(16,BODNZF,1,1),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDMA}},
{"bdnzfla+", BBO(16,BODNZF,1,1),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDPA}},
{"bdnzfla",  BBO(16,BODNZF,1,1),	BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BDA}},
{"bdzf-",    BBO(16,BODZF,0,0),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDM}},
{"bdzf+",    BBO(16,BODZF,0,0),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDP}},
{"bdzf",     BBO(16,BODZF,0,0),		BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BD}},
{"bdzfl-",   BBO(16,BODZF,0,1),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDM}},
{"bdzfl+",   BBO(16,BODZF,0,1),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDP}},
{"bdzfl",    BBO(16,BODZF,0,1),		BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BD}},
{"bdzfa-",   BBO(16,BODZF,1,0),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDMA}},
{"bdzfa+",   BBO(16,BODZF,1,0),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDPA}},
{"bdzfa",    BBO(16,BODZF,1,0),		BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BDA}},
{"bdzfla-",  BBO(16,BODZF,1,1),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDMA}},
{"bdzfla+",  BBO(16,BODZF,1,1),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDPA}},
{"bdzfla",   BBO(16,BODZF,1,1),		BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BDA}},

{"bf-",	     BBO(16,BOF,0,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDM}},
{"bf+",	     BBO(16,BOF,0,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDP}},
{"bf",	     BBO(16,BOF,0,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BD}},
{"bbf",	     BBO(16,BOF,0,0),		BBOAT_MASK,    PWRCOM,	 PPCVLE|EXT,	{BI, BD}},
{"bfl-",     BBO(16,BOF,0,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDM}},
{"bfl+",     BBO(16,BOF,0,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDP}},
{"bfl",	     BBO(16,BOF,0,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BD}},
{"bbfl",     BBO(16,BOF,0,1),		BBOAT_MASK,    PWRCOM,	 PPCVLE|EXT,	{BI, BD}},
{"bfa-",     BBO(16,BOF,1,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDMA}},
{"bfa+",     BBO(16,BOF,1,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDPA}},
{"bfa",	     BBO(16,BOF,1,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDA}},
{"bbfa",     BBO(16,BOF,1,0),		BBOAT_MASK,    PWRCOM,	 PPCVLE|EXT,	{BI, BDA}},
{"bfla-",    BBO(16,BOF,1,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDMA}},
{"bfla+",    BBO(16,BOF,1,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDPA}},
{"bfla",     BBO(16,BOF,1,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDA}},
{"bbfla",    BBO(16,BOF,1,1),		BBOAT_MASK,    PWRCOM,	 PPCVLE|EXT,	{BI, BDA}},

{"bdnzt-",   BBO(16,BODNZT,0,0),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDM}},
{"bdnzt+",   BBO(16,BODNZT,0,0),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDP}},
{"bdnzt",    BBO(16,BODNZT,0,0),	BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BD}},
{"bdnztl-",  BBO(16,BODNZT,0,1),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDM}},
{"bdnztl+",  BBO(16,BODNZT,0,1),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDP}},
{"bdnztl",   BBO(16,BODNZT,0,1),	BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BD}},
{"bdnzta-",  BBO(16,BODNZT,1,0),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDMA}},
{"bdnzta+",  BBO(16,BODNZT,1,0),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDPA}},
{"bdnzta",   BBO(16,BODNZT,1,0),	BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BDA}},
{"bdnztla-", BBO(16,BODNZT,1,1),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDMA}},
{"bdnztla+", BBO(16,BODNZT,1,1),	BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDPA}},
{"bdnztla",  BBO(16,BODNZT,1,1),	BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BDA}},
{"bdzt-",    BBO(16,BODZT,0,0),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDM}},
{"bdzt+",    BBO(16,BODZT,0,0),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDP}},
{"bdzt",     BBO(16,BODZT,0,0),		BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BD}},
{"bdztl-",   BBO(16,BODZT,0,1),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDM}},
{"bdztl+",   BBO(16,BODZT,0,1),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDP}},
{"bdztl",    BBO(16,BODZT,0,1),		BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BD}},
{"bdzta-",   BBO(16,BODZT,1,0),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDMA}},
{"bdzta+",   BBO(16,BODZT,1,0),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDPA}},
{"bdzta",    BBO(16,BODZT,1,0),		BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BDA}},
{"bdztla-",  BBO(16,BODZT,1,1),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDMA}},
{"bdztla+",  BBO(16,BODZT,1,1),		BBOY_MASK,     PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BDPA}},
{"bdztla",   BBO(16,BODZT,1,1),		BBOY_MASK,     PPCCOM,	 PPCVLE|EXT,		{BI, BDA}},

{"bt-",	     BBO(16,BOT,0,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDM}},
{"bt+",	     BBO(16,BOT,0,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDP}},
{"bt",	     BBO(16,BOT,0,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BD}},
{"bbt",	     BBO(16,BOT,0,0),		BBOAT_MASK,    PWRCOM,	 PPCVLE|EXT,	{BI, BD}},
{"btl-",     BBO(16,BOT,0,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDM}},
{"btl+",     BBO(16,BOT,0,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDP}},
{"btl",	     BBO(16,BOT,0,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BD}},
{"bbtl",     BBO(16,BOT,0,1),		BBOAT_MASK,    PWRCOM,	 PPCVLE|EXT,	{BI, BD}},
{"bta-",     BBO(16,BOT,1,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDMA}},
{"bta+",     BBO(16,BOT,1,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDPA}},
{"bta",	     BBO(16,BOT,1,0),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDA}},
{"bbta",     BBO(16,BOT,1,0),		BBOAT_MASK,    PWRCOM,	 PPCVLE|EXT,	{BI, BDA}},
{"btla-",    BBO(16,BOT,1,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDMA}},
{"btla+",    BBO(16,BOT,1,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDPA}},
{"btla",     BBO(16,BOT,1,1),		BBOAT_MASK,    PPCCOM,	 PPCVLE|EXT,	{BI, BDA}},
{"bbtla",    BBO(16,BOT,1,1),		BBOAT_MASK,    PWRCOM,	 PPCVLE|EXT,	{BI, BDA}},

{"bc-",		B(16,0,0),	B_MASK,	     PPCCOM,	PPCVLE|EXT,	{BOM, BI, BDM}},
{"bc+",		B(16,0,0),	B_MASK,	     PPCCOM,	PPCVLE|EXT,	{BOP, BI, BDP}},
{"bc",		B(16,0,0),	B_MASK,	     COM,	PPCVLE,		{BO, BI, BD}},
{"bcl-",	B(16,0,1),	B_MASK,	     PPCCOM,	PPCVLE|EXT,	{BOM, BI, BDM}},
{"bcl+",	B(16,0,1),	B_MASK,	     PPCCOM,	PPCVLE|EXT,	{BOP, BI, BDP}},
{"bcl",		B(16,0,1),	B_MASK,	     COM,	PPCVLE,		{BO, BI, BD}},
{"bca-",	B(16,1,0),	B_MASK,	     PPCCOM,	PPCVLE|EXT,	{BOM, BI, BDMA}},
{"bca+",	B(16,1,0),	B_MASK,	     PPCCOM,	PPCVLE|EXT,	{BOP, BI, BDPA}},
{"bca",		B(16,1,0),	B_MASK,	     COM,	PPCVLE,		{BO, BI, BDA}},
{"bcla-",	B(16,1,1),	B_MASK,	     PPCCOM,	PPCVLE|EXT,	{BOM, BI, BDMA}},
{"bcla+",	B(16,1,1),	B_MASK,	     PPCCOM,	PPCVLE|EXT,	{BOP, BI, BDPA}},
{"bcla",	B(16,1,1),	B_MASK,	     COM,	PPCVLE,		{BO, BI, BDA}},

{"svc",		SC(17,0,0),	SC_MASK,     POWER,	PPCVLE,		{SVC_LEV, FL1, FL2}},
{"scv",		SC(17,0,1),	SC_MASK,     POWER9,	PPCVLE,		{SVC_LEV}},
{"svcl",	SC(17,0,1),	SC_MASK,     POWER,	PPCVLE,		{SVC_LEV, FL1, FL2}},
{"sc",		SC(17,1,0),	SC_MASK,     PPC,	PPCVLE,		{LEV}},
{"svca",	SC(17,1,0),	SC_MASK,     PWRCOM,	PPCVLE,		{SV}},
{"svcla",	SC(17,1,1),	SC_MASK,     POWER,	PPCVLE,		{SV}},

{"b",		B(18,0,0),	B_MASK,	     COM,	PPCVLE,		{LI}},
{"bl",		B(18,0,1),	B_MASK,	     COM,	PPCVLE,		{LI}},
{"ba",		B(18,1,0),	B_MASK,	     COM,	PPCVLE,		{LIA}},
{"bla",		B(18,1,1),	B_MASK,	     COM,	PPCVLE,		{LIA}},

{"mcrf",     XL(19,0), XLBB_MASK|(3<<21)|(3<<16), COM,	PPCVLE,		{BF, BFA}},

{"lnia",     DX(19,2),		NODX_MASK,   POWER9,	PPCVLE|EXT,	{RT}},
{"addpcis",  DX(19,2),		DX_MASK,     POWER9,	PPCVLE,		{RT, DXD}},
{"subpcis",  DX(19,2),		DX_MASK,     POWER9,	PPCVLE|EXT,	{RT, NDXD}},

{"bdnzlr-",  XLO(19,BODNZ,16,0),	XLBOBIBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BH}},
{"bdnzlr+",  XLO(19,BODNZP,16,0),	XLBOBIBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BH}},
{"bdnzlr",   XLO(19,BODNZ,16,0),	XLBOBIBB_MASK, PPCCOM,	 PPCVLE|EXT,		{BH}},
{"bdnzlrl-", XLO(19,BODNZ,16,1),	XLBOBIBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BH}},
{"bdnzlrl+", XLO(19,BODNZP,16,1),	XLBOBIBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BH}},
{"bdnzlrl",  XLO(19,BODNZ,16,1),	XLBOBIBB_MASK, PPCCOM,	 PPCVLE|EXT,		{BH}},
{"bdzlr-",   XLO(19,BODZ,16,0),		XLBOBIBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BH}},
{"bdzlr+",   XLO(19,BODZP,16,0),	XLBOBIBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BH}},
{"bdzlr",    XLO(19,BODZ,16,0),		XLBOBIBB_MASK, PPCCOM,	 PPCVLE|EXT,		{BH}},
{"bdzlrl-",  XLO(19,BODZ,16,1),		XLBOBIBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BH}},
{"bdzlrl+",  XLO(19,BODZP,16,1),	XLBOBIBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BH}},
{"bdzlrl",   XLO(19,BODZ,16,1),		XLBOBIBB_MASK, PPCCOM,	 PPCVLE|EXT,		{BH}},
{"blr",	     XLO(19,BOU,16,0),		XLBOBIBB_MASK, PPCCOM,	 PPCVLE|EXT,		{BH}},
{"br",	     XLO(19,BOU,16,0),		XLBOBIBB_MASK, PWRCOM,	 PPCVLE|EXT,		{BH}},
{"blrl",     XLO(19,BOU,16,1),		XLBOBIBB_MASK, PPCCOM,	 PPCVLE|EXT,		{BH}},
{"brl",	     XLO(19,BOU,16,1),		XLBOBIBB_MASK, PWRCOM,	 PPCVLE|EXT,		{BH}},
{"bdnzlr-",  XLO(19,BODNZM4,16,0),	XLBOBIBB_MASK, ISA_V2,	 PPCVLE|EXT,		{BH}},
{"bdnzlrl-", XLO(19,BODNZM4,16,1),	XLBOBIBB_MASK, ISA_V2,	 PPCVLE|EXT,		{BH}},
{"bdnzlr+",  XLO(19,BODNZP4,16,0),	XLBOBIBB_MASK, ISA_V2,	 PPCVLE|EXT,		{BH}},
{"bdnzlrl+", XLO(19,BODNZP4,16,1),	XLBOBIBB_MASK, ISA_V2,	 PPCVLE|EXT,		{BH}},
{"bdzlr-",   XLO(19,BODZM4,16,0),	XLBOBIBB_MASK, ISA_V2,	 PPCVLE|EXT,		{BH}},
{"bdzlrl-",  XLO(19,BODZM4,16,1),	XLBOBIBB_MASK, ISA_V2,	 PPCVLE|EXT,		{BH}},
{"bdzlr+",   XLO(19,BODZP4,16,0),	XLBOBIBB_MASK, ISA_V2,	 PPCVLE|EXT,		{BH}},
{"bdzlrl+",  XLO(19,BODZP4,16,1),	XLBOBIBB_MASK, ISA_V2,	 PPCVLE|EXT,		{BH}},

{"bgelr-",   XLOCB(19,BOF,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgelr+",   XLOCB(19,BOFP,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgelr",    XLOCB(19,BOF,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bger",     XLOCB(19,BOF,CBLT,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnllr-",   XLOCB(19,BOF,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnllr+",   XLOCB(19,BOFP,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnllr",    XLOCB(19,BOF,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnlr",     XLOCB(19,BOF,CBLT,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgelrl-",  XLOCB(19,BOF,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgelrl+",  XLOCB(19,BOFP,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgelrl",   XLOCB(19,BOF,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgerl",    XLOCB(19,BOF,CBLT,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnllrl-",  XLOCB(19,BOF,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnllrl+",  XLOCB(19,BOFP,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnllrl",   XLOCB(19,BOF,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnlrl",    XLOCB(19,BOF,CBLT,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"blelr-",   XLOCB(19,BOF,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"blelr+",   XLOCB(19,BOFP,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"blelr",    XLOCB(19,BOF,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bler",     XLOCB(19,BOF,CBGT,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnglr-",   XLOCB(19,BOF,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnglr+",   XLOCB(19,BOFP,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnglr",    XLOCB(19,BOF,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bngr",     XLOCB(19,BOF,CBGT,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"blelrl-",  XLOCB(19,BOF,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"blelrl+",  XLOCB(19,BOFP,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"blelrl",   XLOCB(19,BOF,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"blerl",    XLOCB(19,BOF,CBGT,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnglrl-",  XLOCB(19,BOF,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnglrl+",  XLOCB(19,BOFP,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnglrl",   XLOCB(19,BOF,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bngrl",    XLOCB(19,BOF,CBGT,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnelr-",   XLOCB(19,BOF,CBEQ,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnelr+",   XLOCB(19,BOFP,CBEQ,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnelr",    XLOCB(19,BOF,CBEQ,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bner",     XLOCB(19,BOF,CBEQ,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnelrl-",  XLOCB(19,BOF,CBEQ,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnelrl+",  XLOCB(19,BOFP,CBEQ,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnelrl",   XLOCB(19,BOF,CBEQ,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnerl",    XLOCB(19,BOF,CBEQ,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnslr-",   XLOCB(19,BOF,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnslr+",   XLOCB(19,BOFP,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnslr",    XLOCB(19,BOF,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnsr",     XLOCB(19,BOF,CBSO,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnulr-",   XLOCB(19,BOF,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnulr+",   XLOCB(19,BOFP,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnulr",    XLOCB(19,BOF,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnslrl-",  XLOCB(19,BOF,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnslrl+",  XLOCB(19,BOFP,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnslrl",   XLOCB(19,BOF,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnsrl",    XLOCB(19,BOF,CBSO,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnulrl-",  XLOCB(19,BOF,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnulrl+",  XLOCB(19,BOFP,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnulrl",   XLOCB(19,BOF,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgelr-",   XLOCB(19,BOFM4,CBLT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnllr-",   XLOCB(19,BOFM4,CBLT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgelrl-",  XLOCB(19,BOFM4,CBLT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnllrl-",  XLOCB(19,BOFM4,CBLT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"blelr-",   XLOCB(19,BOFM4,CBGT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnglr-",   XLOCB(19,BOFM4,CBGT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"blelrl-",  XLOCB(19,BOFM4,CBGT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnglrl-",  XLOCB(19,BOFM4,CBGT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnelr-",   XLOCB(19,BOFM4,CBEQ,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnelrl-",  XLOCB(19,BOFM4,CBEQ,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnslr-",   XLOCB(19,BOFM4,CBSO,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnulr-",   XLOCB(19,BOFM4,CBSO,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnslrl-",  XLOCB(19,BOFM4,CBSO,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnulrl-",  XLOCB(19,BOFM4,CBSO,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgelr+",   XLOCB(19,BOFP4,CBLT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnllr+",   XLOCB(19,BOFP4,CBLT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgelrl+",  XLOCB(19,BOFP4,CBLT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnllrl+",  XLOCB(19,BOFP4,CBLT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"blelr+",   XLOCB(19,BOFP4,CBGT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnglr+",   XLOCB(19,BOFP4,CBGT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"blelrl+",  XLOCB(19,BOFP4,CBGT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnglrl+",  XLOCB(19,BOFP4,CBGT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnelr+",   XLOCB(19,BOFP4,CBEQ,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnelrl+",  XLOCB(19,BOFP4,CBEQ,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnslr+",   XLOCB(19,BOFP4,CBSO,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnulr+",   XLOCB(19,BOFP4,CBSO,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnslrl+",  XLOCB(19,BOFP4,CBSO,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnulrl+",  XLOCB(19,BOFP4,CBSO,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bltlr-",   XLOCB(19,BOT,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bltlr+",   XLOCB(19,BOTP,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bltlr",    XLOCB(19,BOT,CBLT,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bltr",     XLOCB(19,BOT,CBLT,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bltlrl-",  XLOCB(19,BOT,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bltlrl+",  XLOCB(19,BOTP,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bltlrl",   XLOCB(19,BOT,CBLT,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bltrl",    XLOCB(19,BOT,CBLT,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgtlr-",   XLOCB(19,BOT,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgtlr+",   XLOCB(19,BOTP,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgtlr",    XLOCB(19,BOT,CBGT,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgtr",     XLOCB(19,BOT,CBGT,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgtlrl-",  XLOCB(19,BOT,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgtlrl+",  XLOCB(19,BOTP,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgtlrl",   XLOCB(19,BOT,CBGT,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgtrl",    XLOCB(19,BOT,CBGT,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"beqlr-",   XLOCB(19,BOT,CBEQ,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"beqlr+",   XLOCB(19,BOTP,CBEQ,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"beqlr",    XLOCB(19,BOT,CBEQ,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"beqr",     XLOCB(19,BOT,CBEQ,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"beqlrl-",  XLOCB(19,BOT,CBEQ,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"beqlrl+",  XLOCB(19,BOTP,CBEQ,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"beqlrl",   XLOCB(19,BOT,CBEQ,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"beqrl",    XLOCB(19,BOT,CBEQ,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bsolr-",   XLOCB(19,BOT,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bsolr+",   XLOCB(19,BOTP,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bsolr",    XLOCB(19,BOT,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bsor",     XLOCB(19,BOT,CBSO,16,0),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bunlr-",   XLOCB(19,BOT,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bunlr+",   XLOCB(19,BOTP,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bunlr",    XLOCB(19,BOT,CBSO,16,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bsolrl-",  XLOCB(19,BOT,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bsolrl+",  XLOCB(19,BOTP,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bsolrl",   XLOCB(19,BOT,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bsorl",    XLOCB(19,BOT,CBSO,16,1),	XLBOCBBB_MASK, PWRCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bunlrl-",  XLOCB(19,BOT,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bunlrl+",  XLOCB(19,BOTP,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bunlrl",   XLOCB(19,BOT,CBSO,16,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bltlr-",   XLOCB(19,BOTM4,CBLT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bltlrl-",  XLOCB(19,BOTM4,CBLT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgtlr-",   XLOCB(19,BOTM4,CBGT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgtlrl-",  XLOCB(19,BOTM4,CBGT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"beqlr-",   XLOCB(19,BOTM4,CBEQ,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"beqlrl-",  XLOCB(19,BOTM4,CBEQ,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bsolr-",   XLOCB(19,BOTM4,CBSO,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bunlr-",   XLOCB(19,BOTM4,CBSO,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bsolrl-",  XLOCB(19,BOTM4,CBSO,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bunlrl-",  XLOCB(19,BOTM4,CBSO,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bltlr+",   XLOCB(19,BOTP4,CBLT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bltlrl+",  XLOCB(19,BOTP4,CBLT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgtlr+",   XLOCB(19,BOTP4,CBGT,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgtlrl+",  XLOCB(19,BOTP4,CBGT,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"beqlr+",   XLOCB(19,BOTP4,CBEQ,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"beqlrl+",  XLOCB(19,BOTP4,CBEQ,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bsolr+",   XLOCB(19,BOTP4,CBSO,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bunlr+",   XLOCB(19,BOTP4,CBSO,16,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bsolrl+",  XLOCB(19,BOTP4,CBSO,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bunlrl+",  XLOCB(19,BOTP4,CBSO,16,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},

{"bdnzflr-", XLO(19,BODNZF,16,0),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdnzflr+", XLO(19,BODNZFP,16,0),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdnzflr",  XLO(19,BODNZF,16,0),	XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bdnzflrl-",XLO(19,BODNZF,16,1),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdnzflrl+",XLO(19,BODNZFP,16,1),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdnzflrl", XLO(19,BODNZF,16,1),	XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bdzflr-",  XLO(19,BODZF,16,0),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdzflr+",  XLO(19,BODZFP,16,0),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdzflr",   XLO(19,BODZF,16,0),	XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bdzflrl-", XLO(19,BODZF,16,1),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdzflrl+", XLO(19,BODZFP,16,1),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdzflrl",  XLO(19,BODZF,16,1),	XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bflr-",    XLO(19,BOF,16,0),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bflr+",    XLO(19,BOFP,16,0),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bflr",     XLO(19,BOF,16,0),		XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bbfr",     XLO(19,BOF,16,0),		XLBOBB_MASK,   PWRCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bflrl-",   XLO(19,BOF,16,1),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bflrl+",   XLO(19,BOFP,16,1),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bflrl",    XLO(19,BOF,16,1),		XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bbfrl",    XLO(19,BOF,16,1),		XLBOBB_MASK,   PWRCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bflr-",    XLO(19,BOFM4,16,0),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"bflrl-",   XLO(19,BOFM4,16,1),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"bflr+",    XLO(19,BOFP4,16,0),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"bflrl+",   XLO(19,BOFP4,16,1),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"bdnztlr-", XLO(19,BODNZT,16,0),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdnztlr+", XLO(19,BODNZTP,16,0),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdnztlr",  XLO(19,BODNZT,16,0),	XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bdnztlrl-", XLO(19,BODNZT,16,1),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdnztlrl+", XLO(19,BODNZTP,16,1),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdnztlrl", XLO(19,BODNZT,16,1),	XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bdztlr-",  XLO(19,BODZT,16,0),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdztlr+",  XLO(19,BODZTP,16,0),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdztlr",   XLO(19,BODZT,16,0),	XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bdztlrl-", XLO(19,BODZT,16,1),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdztlrl+", XLO(19,BODZTP,16,1),	XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bdztlrl",  XLO(19,BODZT,16,1),	XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"btlr-",    XLO(19,BOT,16,0),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"btlr+",    XLO(19,BOTP,16,0),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"btlr",     XLO(19,BOT,16,0),		XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bbtr",     XLO(19,BOT,16,0),		XLBOBB_MASK,   PWRCOM,	 PPCVLE|EXT,		{BI, BH}},
{"btlrl-",   XLO(19,BOT,16,1),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"btlrl+",   XLO(19,BOTP,16,1),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"btlrl",    XLO(19,BOT,16,1),		XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bbtrl",    XLO(19,BOT,16,1),		XLBOBB_MASK,   PWRCOM,	 PPCVLE|EXT,		{BI, BH}},
{"btlr-",    XLO(19,BOTM4,16,0),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"btlrl-",   XLO(19,BOTM4,16,1),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"btlr+",    XLO(19,BOTP4,16,0),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"btlrl+",   XLO(19,BOTP4,16,1),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},

{"bclr-",    XLLK(19,16,0),		XLBH_MASK,     PPCCOM,	 PPCVLE|EXT,	{BOM, BI, BH}},
{"bclr+",    XLLK(19,16,0),		XLBH_MASK,     PPCCOM,	 PPCVLE|EXT,	{BOP, BI, BH}},
{"bclr",     XLLK(19,16,0),		XLBH_MASK,     PPCCOM,	 PPCVLE,	{BO, BI, BH}},
{"bcr",	     XLLK(19,16,0),		XLBH_MASK,     PWRCOM,	 PPCVLE,	{BO, BI, BH}},
{"bclrl-",   XLLK(19,16,1),		XLBH_MASK,     PPCCOM,	 PPCVLE|EXT,	{BOM, BI, BH}},
{"bclrl+",   XLLK(19,16,1),		XLBH_MASK,     PPCCOM,	 PPCVLE|EXT,	{BOP, BI, BH}},
{"bclrl",    XLLK(19,16,1),		XLBH_MASK,     PPCCOM,	 PPCVLE,	{BO, BI, BH}},
{"bcrl",     XLLK(19,16,1),		XLBH_MASK,     PWRCOM,	 PPCVLE,	{BO, BI, BH}},

{"rfid",	XL(19,18),	0xffffffff,  PPC64,	PPCVLE,	{0}},

{"crnot",	XL(19,33),	XL_MASK,     PPCCOM,	PPCVLE|EXT,	{BT, BAB}},
{"crnor",	XL(19,33),	XL_MASK,     COM,	PPCVLE,		{BT, BA, BB}},

{"rfmci",	X(19,38),    0xffffffff, PPCRFMCI|PPCA2|PPC476, PPCVLE,	{0}},
{"rfdi",	XL(19,39),	0xffffffff,  E500MC,	PPCVLE,		{0}},
{"rfi",		XL(19,50),	0xffffffff,  COM,	PPCVLE,		{0}},
{"rfci",	XL(19,51), 0xffffffff, PPC403|BOOKE|PPCE300|PPCA2|PPC476, PPCVLE, {0}},

{"rfscv",	XL(19,82),	0xffffffff,  POWER9,	PPCVLE,		{0}},
{"rfsvc",	XL(19,82),	0xffffffff,  POWER,	PPCVLE,		{0}},

{"rfgi",	XL(19,102),   0xffffffff, E500MC|PPCA2,	PPCVLE,		{0}},

{"crandc",	XL(19,129),	XL_MASK,     COM,	PPCVLE,		{BT, BA, BB}},

{"rfebb",	XL(19,146),	XLS_MASK,    POWER8,	PPCVLE,		{SXL}},

{"isync",	XL(19,150),	0xffffffff,  PPCCOM,	PPCVLE,		{0}},
{"ics",		XL(19,150),	0xffffffff,  PWRCOM,	PPCVLE,		{0}},

{"crclr",	XL(19,193),	XL_MASK,     PPCCOM,	PPCVLE|EXT,	{BTAB}},
{"crxor",	XL(19,193),	XL_MASK,     COM,	PPCVLE,		{BT, BA, BB}},

{"dnh",		X(19,198),	X_MASK,	     E500MC,	PPCVLE,		{DUI, DUIS}},

{"crnand",	XL(19,225),	XL_MASK,     COM,	PPCVLE,		{BT, BA, BB}},

{"crand",	XL(19,257),	XL_MASK,     COM,	PPCVLE,		{BT, BA, BB}},

{"hrfid",	XL(19,274),    0xffffffff, POWER5|CELL, PPC476|PPCVLE,	{0}},

{"crset",	XL(19,289),	XL_MASK,     PPCCOM,	PPCVLE|EXT,	{BTAB}},
{"creqv",	XL(19,289),	XL_MASK,     COM,	PPCVLE,		{BT, BA, BB}},

{"urfid",	XL(19,306),	0xffffffff,  POWER9,	PPCVLE,		{0}},
{"stop",	XL(19,370),	0xffffffff,  POWER9,	PPCVLE,		{0}},

{"doze",	XL(19,402),	0xffffffff,  POWER6,	POWER9|PPCVLE,	{0}},

{"crorc",	XL(19,417),	XL_MASK,     COM,	PPCVLE,		{BT, BA, BB}},

{"nap",		XL(19,434),	0xffffffff,  POWER6,	POWER9|PPCVLE,	{0}},

{"crmove",	XL(19,449),	XL_MASK,     PPCCOM,	PPCVLE|EXT,	{BT, BAB}},
{"cror",	XL(19,449),	XL_MASK,     COM,	PPCVLE,		{BT, BA, BB}},

{"sleep",	XL(19,466),	0xffffffff,  POWER6,	POWER9|PPCVLE,	{0}},
{"rvwinkle",	XL(19,498),	0xffffffff,  POWER6,	POWER9|PPCVLE,	{0}},

{"bctr",    XLO(19,BOU,528,0),		XLBOBIBB_MASK, COM,	 PPCVLE|EXT,		{BH}},
{"bctrl",   XLO(19,BOU,528,1),		XLBOBIBB_MASK, COM,	 PPCVLE|EXT,		{BH}},
{"bgectr-", XLOCB(19,BOF,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgectr+", XLOCB(19,BOFP,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgectr",  XLOCB(19,BOF,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnlctr-", XLOCB(19,BOF,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnlctr+", XLOCB(19,BOFP,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnlctr",  XLOCB(19,BOF,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgectrl-",XLOCB(19,BOF,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgectrl+",XLOCB(19,BOFP,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgectrl", XLOCB(19,BOF,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnlctrl-",XLOCB(19,BOF,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnlctrl+",XLOCB(19,BOFP,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnlctrl", XLOCB(19,BOF,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"blectr-", XLOCB(19,BOF,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"blectr+", XLOCB(19,BOFP,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"blectr",  XLOCB(19,BOF,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bngctr-", XLOCB(19,BOF,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bngctr+", XLOCB(19,BOFP,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bngctr",  XLOCB(19,BOF,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"blectrl-",XLOCB(19,BOF,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"blectrl+",XLOCB(19,BOFP,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"blectrl", XLOCB(19,BOF,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bngctrl-",XLOCB(19,BOF,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bngctrl+",XLOCB(19,BOFP,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bngctrl", XLOCB(19,BOF,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnectr-", XLOCB(19,BOF,CBEQ,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnectr+", XLOCB(19,BOFP,CBEQ,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnectr",  XLOCB(19,BOF,CBEQ,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnectrl-",XLOCB(19,BOF,CBEQ,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnectrl+",XLOCB(19,BOFP,CBEQ,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnectrl", XLOCB(19,BOF,CBEQ,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnsctr-", XLOCB(19,BOF,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnsctr+", XLOCB(19,BOFP,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnsctr",  XLOCB(19,BOF,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnuctr-", XLOCB(19,BOF,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnuctr+", XLOCB(19,BOFP,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnuctr",  XLOCB(19,BOF,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnsctrl-",XLOCB(19,BOF,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnsctrl+",XLOCB(19,BOFP,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnsctrl", XLOCB(19,BOF,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bnuctrl-",XLOCB(19,BOF,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnuctrl+",XLOCB(19,BOFP,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bnuctrl", XLOCB(19,BOF,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgectr-", XLOCB(19,BOFM4,CBLT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnlctr-", XLOCB(19,BOFM4,CBLT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgectrl-",XLOCB(19,BOFM4,CBLT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnlctrl-",XLOCB(19,BOFM4,CBLT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"blectr-", XLOCB(19,BOFM4,CBGT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bngctr-", XLOCB(19,BOFM4,CBGT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"blectrl-",XLOCB(19,BOFM4,CBGT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bngctrl-",XLOCB(19,BOFM4,CBGT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnectr-", XLOCB(19,BOFM4,CBEQ,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnectrl-",XLOCB(19,BOFM4,CBEQ,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnsctr-", XLOCB(19,BOFM4,CBSO,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnuctr-", XLOCB(19,BOFM4,CBSO,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnsctrl-",XLOCB(19,BOFM4,CBSO,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnuctrl-",XLOCB(19,BOFM4,CBSO,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgectr+", XLOCB(19,BOFP4,CBLT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnlctr+", XLOCB(19,BOFP4,CBLT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgectrl+",XLOCB(19,BOFP4,CBLT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnlctrl+",XLOCB(19,BOFP4,CBLT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"blectr+", XLOCB(19,BOFP4,CBGT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bngctr+", XLOCB(19,BOFP4,CBGT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"blectrl+",XLOCB(19,BOFP4,CBGT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bngctrl+",XLOCB(19,BOFP4,CBGT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnectr+", XLOCB(19,BOFP4,CBEQ,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnectrl+",XLOCB(19,BOFP4,CBEQ,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnsctr+", XLOCB(19,BOFP4,CBSO,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnuctr+", XLOCB(19,BOFP4,CBSO,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnsctrl+",XLOCB(19,BOFP4,CBSO,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bnuctrl+",XLOCB(19,BOFP4,CBSO,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bltctr-", XLOCB(19,BOT,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bltctr+", XLOCB(19,BOTP,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bltctr",  XLOCB(19,BOT,CBLT,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bltctrl-",XLOCB(19,BOT,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bltctrl+",XLOCB(19,BOTP,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bltctrl", XLOCB(19,BOT,CBLT,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgtctr-", XLOCB(19,BOT,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgtctr+", XLOCB(19,BOTP,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgtctr",  XLOCB(19,BOT,CBGT,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bgtctrl-",XLOCB(19,BOT,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgtctrl+",XLOCB(19,BOTP,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bgtctrl", XLOCB(19,BOT,CBGT,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"beqctr-", XLOCB(19,BOT,CBEQ,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"beqctr+", XLOCB(19,BOTP,CBEQ,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"beqctr",  XLOCB(19,BOT,CBEQ,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"beqctrl-",XLOCB(19,BOT,CBEQ,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"beqctrl+",XLOCB(19,BOTP,CBEQ,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"beqctrl", XLOCB(19,BOT,CBEQ,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bsoctr-", XLOCB(19,BOT,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bsoctr+", XLOCB(19,BOTP,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bsoctr",  XLOCB(19,BOT,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bunctr-", XLOCB(19,BOT,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bunctr+", XLOCB(19,BOTP,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bunctr",  XLOCB(19,BOT,CBSO,528,0),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bsoctrl-",XLOCB(19,BOT,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bsoctrl+",XLOCB(19,BOTP,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bsoctrl", XLOCB(19,BOT,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bunctrl-",XLOCB(19,BOT,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bunctrl+",XLOCB(19,BOTP,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 ISA_V2|PPCVLE|EXT,	{CR, BH}},
{"bunctrl", XLOCB(19,BOT,CBSO,528,1),	XLBOCBBB_MASK, PPCCOM,	 PPCVLE|EXT,		{CR, BH}},
{"bltctr-", XLOCB(19,BOTM4,CBLT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bltctrl-",XLOCB(19,BOTM4,CBLT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgtctr-", XLOCB(19,BOTM4,CBGT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgtctrl-",XLOCB(19,BOTM4,CBGT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"beqctr-", XLOCB(19,BOTM4,CBEQ,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"beqctrl-",XLOCB(19,BOTM4,CBEQ,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bsoctr-", XLOCB(19,BOTM4,CBSO,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bunctr-", XLOCB(19,BOTM4,CBSO,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bsoctrl-",XLOCB(19,BOTM4,CBSO,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bunctrl-",XLOCB(19,BOTM4,CBSO,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bltctr+", XLOCB(19,BOTP4,CBLT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bltctrl+",XLOCB(19,BOTP4,CBLT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgtctr+", XLOCB(19,BOTP4,CBGT,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bgtctrl+",XLOCB(19,BOTP4,CBGT,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"beqctr+", XLOCB(19,BOTP4,CBEQ,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"beqctrl+",XLOCB(19,BOTP4,CBEQ,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bsoctr+", XLOCB(19,BOTP4,CBSO,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bunctr+", XLOCB(19,BOTP4,CBSO,528,0),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bsoctrl+",XLOCB(19,BOTP4,CBSO,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},
{"bunctrl+",XLOCB(19,BOTP4,CBSO,528,1),	XLBOCBBB_MASK, ISA_V2,	 PPCVLE|EXT,		{CR, BH}},

{"bfctr-",  XLO(19,BOF,528,0),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bfctr+",  XLO(19,BOFP,528,0),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bfctr",   XLO(19,BOF,528,0),		XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bfctrl-", XLO(19,BOF,528,1),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bfctrl+", XLO(19,BOFP,528,1),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"bfctrl",  XLO(19,BOF,528,1),		XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"bfctr-",  XLO(19,BOFM4,528,0),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"bfctrl-", XLO(19,BOFM4,528,1),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"bfctr+",  XLO(19,BOFP4,528,0),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"bfctrl+", XLO(19,BOFP4,528,1),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"btctr-",  XLO(19,BOT,528,0),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"btctr+",  XLO(19,BOTP,528,0),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"btctr",   XLO(19,BOT,528,0),		XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"btctrl-", XLO(19,BOT,528,1),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"btctrl+", XLO(19,BOTP,528,1),		XLBOBB_MASK,   PPCCOM,	 ISA_V2|PPCVLE|EXT,	{BI, BH}},
{"btctrl",  XLO(19,BOT,528,1),		XLBOBB_MASK,   PPCCOM,	 PPCVLE|EXT,		{BI, BH}},
{"btctr-",  XLO(19,BOTM4,528,0),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"btctrl-", XLO(19,BOTM4,528,1),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"btctr+",  XLO(19,BOTP4,528,0),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},
{"btctrl+", XLO(19,BOTP4,528,1),	XLBOBB_MASK,   ISA_V2,	 PPCVLE|EXT,		{BI, BH}},

{"bcctr-",  XLLK(19,528,0),		XLBH_MASK,     PPCCOM,	 PPCVLE|EXT,	{BOM, BI, BH}},
{"bcctr+",  XLLK(19,528,0),		XLBH_MASK,     PPCCOM,	 PPCVLE|EXT,	{BOP, BI, BH}},
{"bcctr",   XLLK(19,528,0),		XLBH_MASK,     PPCCOM,	 PPCVLE,	{BO, BI, BH}},
{"bcc",	    XLLK(19,528,0),		XLBH_MASK,     PWRCOM,	 PPCVLE,	{BO, BI, BH}},
{"bcctrl-", XLLK(19,528,1),		XLBH_MASK,     PPCCOM,	 PPCVLE|EXT,	{BOM, BI, BH}},
{"bcctrl+", XLLK(19,528,1),		XLBH_MASK,     PPCCOM,	 PPCVLE|EXT,	{BOP, BI, BH}},
{"bcctrl",  XLLK(19,528,1),		XLBH_MASK,     PPCCOM,	 PPCVLE,	{BO, BI, BH}},
{"bccl",    XLLK(19,528,1),		XLBH_MASK,     PWRCOM,	 PPCVLE,	{BO, BI, BH}},

{"bdnztar",   XLO(19,BODNZ,560,0),	XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdnztarl",  XLO(19,BODNZ,560,1),	XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdztar",    XLO(19,BODZ,560,0),	XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdztarl",   XLO(19,BODZ,560,1),	XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"btar",      XLO(19,BOU,560,0),	XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"btarl",     XLO(19,BOU,560,1),	XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdnztar-",  XLO(19,BODNZM4,560,0),    XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdnztarl-", XLO(19,BODNZM4,560,1),    XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdnztar+",  XLO(19,BODNZP4,560,0),    XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdnztarl+", XLO(19,BODNZP4,560,1),    XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdztar-",   XLO(19,BODZM4,560,0),     XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdztarl-",  XLO(19,BODZM4,560,1),     XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdztar+",   XLO(19,BODZP4,560,0),     XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},
{"bdztarl+",  XLO(19,BODZP4,560,1),     XLBOBIBB_MASK, POWER8,   PPCVLE|EXT,	{BH}},

{"bgetar",  XLOCB(19,BOF,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnltar",  XLOCB(19,BOF,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgetarl", XLOCB(19,BOF,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnltarl", XLOCB(19,BOF,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bletar",  XLOCB(19,BOF,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bngtar",  XLOCB(19,BOF,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bletarl", XLOCB(19,BOF,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bngtarl", XLOCB(19,BOF,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnetar",  XLOCB(19,BOF,CBEQ,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnetarl", XLOCB(19,BOF,CBEQ,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnstar",  XLOCB(19,BOF,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnutar",  XLOCB(19,BOF,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnstarl", XLOCB(19,BOF,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnutarl", XLOCB(19,BOF,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgetar-", XLOCB(19,BOFM4,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnltar-", XLOCB(19,BOFM4,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgetarl-",XLOCB(19,BOFM4,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnltarl-",XLOCB(19,BOFM4,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bletar-", XLOCB(19,BOFM4,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bngtar-", XLOCB(19,BOFM4,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bletarl-",XLOCB(19,BOFM4,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bngtarl-",XLOCB(19,BOFM4,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnetar-", XLOCB(19,BOFM4,CBEQ,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnetarl-",XLOCB(19,BOFM4,CBEQ,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnstar-", XLOCB(19,BOFM4,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnutar-", XLOCB(19,BOFM4,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnstarl-",XLOCB(19,BOFM4,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnutarl-",XLOCB(19,BOFM4,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgetar+", XLOCB(19,BOFP4,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnltar+", XLOCB(19,BOFP4,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgetarl+",XLOCB(19,BOFP4,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnltarl+",XLOCB(19,BOFP4,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bletar+", XLOCB(19,BOFP4,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bngtar+", XLOCB(19,BOFP4,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bletarl+",XLOCB(19,BOFP4,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bngtarl+",XLOCB(19,BOFP4,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnetar+", XLOCB(19,BOFP4,CBEQ,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnetarl+",XLOCB(19,BOFP4,CBEQ,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnstar+", XLOCB(19,BOFP4,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnutar+", XLOCB(19,BOFP4,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnstarl+",XLOCB(19,BOFP4,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bnutarl+",XLOCB(19,BOFP4,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"blttar",  XLOCB(19,BOT,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"blttarl", XLOCB(19,BOT,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgttar",  XLOCB(19,BOT,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgttarl", XLOCB(19,BOT,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"beqtar",  XLOCB(19,BOT,CBEQ,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"beqtarl", XLOCB(19,BOT,CBEQ,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bsotar",  XLOCB(19,BOT,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"buntar",  XLOCB(19,BOT,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bsotarl", XLOCB(19,BOT,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"buntarl", XLOCB(19,BOT,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"blttar-", XLOCB(19,BOTM4,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"blttarl-",XLOCB(19,BOTM4,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgttar-", XLOCB(19,BOTM4,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgttarl-",XLOCB(19,BOTM4,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"beqtar-", XLOCB(19,BOTM4,CBEQ,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"beqtarl-",XLOCB(19,BOTM4,CBEQ,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bsotar-", XLOCB(19,BOTM4,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"buntar-", XLOCB(19,BOTM4,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bsotarl-",XLOCB(19,BOTM4,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"buntarl-",XLOCB(19,BOTM4,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"blttar+", XLOCB(19,BOTP4,CBLT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"blttarl+",XLOCB(19,BOTP4,CBLT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgttar+", XLOCB(19,BOTP4,CBGT,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bgttarl+",XLOCB(19,BOTP4,CBGT,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"beqtar+", XLOCB(19,BOTP4,CBEQ,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"beqtarl+",XLOCB(19,BOTP4,CBEQ,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bsotar+", XLOCB(19,BOTP4,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"buntar+", XLOCB(19,BOTP4,CBSO,560,0),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"bsotarl+",XLOCB(19,BOTP4,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},
{"buntarl+",XLOCB(19,BOTP4,CBSO,560,1),	XLBOCBBB_MASK, POWER8,	 PPCVLE|EXT,	{CR, BH}},

{"bdnzftar",  XLO(19,BODNZF,560,0),     XLBOBB_MASK,   POWER8,   PPCVLE|EXT,	{BI, BH}},
{"bdnzftarl", XLO(19,BODNZF,560,1),     XLBOBB_MASK,   POWER8,   PPCVLE|EXT,	{BI, BH}},
{"bdzftar",   XLO(19,BODZF,560,0),	XLBOBB_MASK,   POWER8,   PPCVLE|EXT,	{BI, BH}},
{"bdzftarl",  XLO(19,BODZF,560,1),	XLBOBB_MASK,   POWER8,   PPCVLE|EXT,	{BI, BH}},

{"bftar",     XLO(19,BOF,560,0),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bftarl",    XLO(19,BOF,560,1),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bftar-",    XLO(19,BOFM4,560,0),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bftarl-",   XLO(19,BOFM4,560,1),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bftar+",    XLO(19,BOFP4,560,0),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bftarl+",   XLO(19,BOFP4,560,1),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},

{"bdnzttar",  XLO(19,BODNZT,560,0),     XLBOBB_MASK,   POWER8,   PPCVLE|EXT,	{BI, BH}},
{"bdnzttarl", XLO(19,BODNZT,560,1),     XLBOBB_MASK,   POWER8,   PPCVLE|EXT,	{BI, BH}},
{"bdzttar",   XLO(19,BODZT,560,0),	XLBOBB_MASK,   POWER8,   PPCVLE|EXT,	{BI, BH}},
{"bdzttarl",  XLO(19,BODZT,560,1),	XLBOBB_MASK,   POWER8,   PPCVLE|EXT,	{BI, BH}},

{"bttar",     XLO(19,BOT,560,0),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bttarl",    XLO(19,BOT,560,1),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bttar-",    XLO(19,BOTM4,560,0),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bttarl-",   XLO(19,BOTM4,560,1),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bttar+",    XLO(19,BOTP4,560,0),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},
{"bttarl+",   XLO(19,BOTP4,560,1),	XLBOBB_MASK,   POWER8,	 PPCVLE|EXT,	{BI, BH}},

{"bctar-",  XLLK(19,560,0),		XLBH_MASK,     POWER8,	 PPCVLE|EXT,	{BOM, BI, BH}},
{"bctar+",  XLLK(19,560,0),		XLBH_MASK,     POWER8,	 PPCVLE|EXT,	{BOP, BI, BH}},
{"bctar",   XLLK(19,560,0),		XLBH_MASK,     POWER8,	 PPCVLE,	{BO, BI, BH}},
{"bctarl-", XLLK(19,560,1),		XLBH_MASK,     POWER8,	 PPCVLE|EXT,	{BOM, BI, BH}},
{"bctarl+", XLLK(19,560,1),		XLBH_MASK,     POWER8,	 PPCVLE|EXT,	{BOP, BI, BH}},
{"bctarl",  XLLK(19,560,1),		XLBH_MASK,     POWER8,	 PPCVLE,	{BO, BI, BH}},

{"rlwimi",	M(20,0),	M_MASK,	     PPCCOM,	PPCVLE,		{RA, RS, SH, MBE, ME}},
{"inslwi",	M(20,0),	M_MASK,	     PPCCOM,	PPCVLE|EXT,	{RA, RS, ILWn, ILWb}},
{"insrwi",	M(20,0),	M_MASK,	     PPCCOM,	PPCVLE|EXT,	{RA, RS, IRWn, IRWb}},
{"rlimi",	M(20,0),	M_MASK,	     PWRCOM,	PPCVLE,		{RA, RS, SH, MBE, ME}},

{"rlwimi.",	M(20,1),	M_MASK,	     PPCCOM,	PPCVLE,		{RA, RS, SH, MBE, ME}},
{"inslwi.",	M(20,1),	M_MASK,	     PPCCOM,	PPCVLE|EXT,	{RA, RS, ILWn, ILWb}},
{"insrwi.",	M(20,1),	M_MASK,	     PPCCOM,	PPCVLE|EXT,	{RA, RS, IRWn, IRWb}},
{"rlimi.",	M(20,1),	M_MASK,	     PWRCOM,	PPCVLE,		{RA, RS, SH, MBE, ME}},

{"rotlwi",	MME(21,31,0),	MMBME_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, SH}},
{"rotrwi",	MME(21,31,0),	MMBME_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, RRWn}},
{"clrlwi",	MME(21,31,0),	MSHME_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, MB}},
{"clrrwi",	M(21,0),	MSHMB_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, CRWn}},
{"slwi",	M(21,0),	MMB_MASK,    PPCCOM,	PPCVLE|EXT,	{RA, RS, SLWn}},
{"srwi",	MME(21,31,0),	MME_MASK,    PPCCOM,	PPCVLE|EXT,	{RA, RS, SRWn}},
{"rlwinm",	M(21,0),	M_MASK,	     PPCCOM,	PPCVLE,		{RA, RS, SH, MBE, ME}},
{"extlwi",	M(21,0),	MMB_MASK,    PPCCOM,	PPCVLE|EXT,	{RA, RS, ELWn, SH}},
{"extrwi",	MME(21,31,0),	MME_MASK,    PPCCOM,	PPCVLE|EXT,	{RA, RS, ERWn, ERWb}},
{"clrlslwi",	M(21,0),	M_MASK,	     PPCCOM,	PPCVLE|EXT,	{RA, RS, CSLWb, CSLWn}},
{"sli",		M(21,0),	MMB_MASK,    PWRCOM,	PPCVLE|EXT,	{RA, RS, SLWn}},
{"sri",		MME(21,31,0),	MME_MASK,    PWRCOM,	PPCVLE|EXT,	{RA, RS, SRWn}},
{"rlinm",	M(21,0),	M_MASK,	     PWRCOM,	PPCVLE,		{RA, RS, SH, MBE, ME}},
{"rotlwi.",	MME(21,31,1),	MMBME_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, SH}},
{"rotrwi.",	MME(21,31,1),	MMBME_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, RRWn}},
{"clrlwi.",	MME(21,31,1),	MSHME_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, MB}},
{"clrrwi.",	M(21,1),	MSHMB_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, CRWn}},
{"slwi.",	M(21,1),	MMB_MASK,    PPCCOM,	PPCVLE|EXT,	{RA, RS, SLWn}},
{"srwi.",	MME(21,31,1),	MME_MASK,    PPCCOM,	PPCVLE|EXT,	{RA, RS, SRWn}},
{"rlwinm.",	M(21,1),	M_MASK,	     PPCCOM,	PPCVLE,		{RA, RS, SH, MBE, ME}},
{"extlwi.",	M(21,1),	MMB_MASK,    PPCCOM,	PPCVLE|EXT,	{RA, RS, ELWn, SH}},
{"extrwi.",	MME(21,31,1),	MME_MASK,    PPCCOM,	PPCVLE|EXT,	{RA, RS, ERWn, ERWb}},
{"clrlslwi.",	M(21,1),	M_MASK,	     PPCCOM,	PPCVLE|EXT,	{RA, RS, CSLWb, CSLWn}},
{"sli.",	M(21,1),	MMB_MASK,    PWRCOM,	PPCVLE|EXT,	{RA, RS, SLWn}},
{"sri.",	MME(21,31,1),	MME_MASK,    PWRCOM,	PPCVLE|EXT,	{RA, RS, SRWn}},
{"rlinm.",	M(21,1),	M_MASK,	     PWRCOM,	PPCVLE,		{RA, RS, SH, MBE, ME}},

{"rlmi",	M(22,0),	M_MASK,	     M601,	PPCVLE,		{RA, RS, RB, MBE, ME}},
{"rlmi.",	M(22,1),	M_MASK,	     M601,	PPCVLE,		{RA, RS, RB, MBE, ME}},

{"svstep",	SVL(22,19,0),	SVL_MASK,	SVP64,	PPCVLE,	{RT, SVi, vf}},
{"svstep.",	SVL(22,19,1),	SVL_MASK,	SVP64,	PPCVLE,	{RT, SVi, vf}},

{"svshape",	SVM(22,25),	SVM_MASK,	SVP64,	PPCVLE,	{SVxd, SVyd, SVzd, SVrm, vf}},

{"setvl",	SVL(22,27,0),	SVL_MASK,	SVP64,	PPCVLE,	{RT, RA, SVi, vf, vs, ms}},
{"setvl.",	SVL(22,27,1),	SVL_MASK,	SVP64,	PPCVLE,	{RT, RA, SVi, vf, vs, ms}},

{"svindex",	SVI(22,41),	SVI_MASK,	SVP64,	PPCVLE,	{SVG, rmm, SVd, ew, yx, mm, sk}},

{"svremap",	SVRM(22,57),	SVRM_MASK,	SVP64,	PPCVLE,	{SVme, mi0, mi1, mi2, mo0, mo1, pst}},

{"rotlw",	MME(23,31,0),	MMBME_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, RB}},
{"rlwnm",	M(23,0),	M_MASK,	     PPCCOM,	PPCVLE,		{RA, RS, RB, MBE, ME}},
{"rlnm",	M(23,0),	M_MASK,	     PWRCOM,	PPCVLE,		{RA, RS, RB, MBE, ME}},
{"rotlw.",	MME(23,31,1),	MMBME_MASK,  PPCCOM,	PPCVLE|EXT,	{RA, RS, RB}},
{"rlwnm.",	M(23,1),	M_MASK,	     PPCCOM,	PPCVLE,		{RA, RS, RB, MBE, ME}},
{"rlnm.",	M(23,1),	M_MASK,	     PWRCOM,	PPCVLE,		{RA, RS, RB, MBE, ME}},

{"nop",		OP(24),		0xffffffff,  PPCCOM,	PPCVLE|EXT,	{0}},
{"exser",	0x63ff0000,	0xffffffff,  POWER9,	PPCVLE|EXT,	{0}},
{"ori",		OP(24),		OP_MASK,     PPCCOM,	PPCVLE,		{RA, RS, UI}},
{"oril",	OP(24),		OP_MASK,     PWRCOM,	PPCVLE,		{RA, RS, UI}},

{"oris",	OP(25),		OP_MASK,     PPCCOM,	PPCVLE,		{RA, RS, UI}},
{"oriu",	OP(25),		OP_MASK,     PWRCOM,	PPCVLE,		{RA, RS, UI}},

{"xnop",	OP(26),		0xffffffff,  PPCCOM,	PPCVLE|EXT,	{0}},
{"xori",	OP(26),		OP_MASK,     PPCCOM,	PPCVLE,		{RA, RS, UI}},
{"xoril",	OP(26),		OP_MASK,     PWRCOM,	PPCVLE,		{RA, RS, UI}},

{"xoris",	OP(27),		OP_MASK,     PPCCOM,	PPCVLE,		{RA, RS, UI}},
{"xoriu",	OP(27),		OP_MASK,     PWRCOM,	PPCVLE,		{RA, RS, UI}},

{"andi.",	OP(28),		OP_MASK,     PPCCOM,	PPCVLE,		{RA, RS, UI}},
{"andil.",	OP(28),		OP_MASK,     PWRCOM,	PPCVLE,		{RA, RS, UI}},

{"andis.",	OP(29),		OP_MASK,     PPCCOM,	PPCVLE,		{RA, RS, UI}},
{"andiu.",	OP(29),		OP_MASK,     PWRCOM,	PPCVLE,		{RA, RS, UI}},

{"rotldi",	MD(30,0,0),	MDMB_MASK,   PPC64,	PPCVLE|EXT,	{RA, RS, SH6}},
{"rotrdi",	MD(30,0,0),	MDMB_MASK,   PPC64,	PPCVLE|EXT,	{RA, RS, RRDn}},
{"clrldi",	MD(30,0,0),	MDSH_MASK,   PPC64,	PPCVLE|EXT,	{RA, RS, MB6}},
{"srdi",	MD(30,0,0),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, SRDn}},
{"rldicl",	MD(30,0,0),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, SH6, MB6}},
{"extrdi",	MD(30,0,0),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, ERDn, ERDb}},
{"rotldi.",	MD(30,0,1),	MDMB_MASK,   PPC64,	PPCVLE|EXT,	{RA, RS, SH6}},
{"rotrdi.",	MD(30,0,1),	MDMB_MASK,   PPC64,	PPCVLE|EXT,	{RA, RS, RRDn}},
{"clrldi.",	MD(30,0,1),	MDSH_MASK,   PPC64,	PPCVLE|EXT,	{RA, RS, MB6}},
{"srdi.",	MD(30,0,1),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, SRDn}},
{"rldicl.",	MD(30,0,1),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, SH6, MB6}},
{"extrdi.",	MD(30,0,1),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, ERDn, ERDb}},

{"clrrdi",	MD(30,1,0),	MDSH_MASK,   PPC64,	PPCVLE|EXT,	{RA, RS, CRDn}},
{"sldi",	MD(30,1,0),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, SLDn}},
{"rldicr",	MD(30,1,0),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, SH6, ME6}},
{"extldi",	MD(30,1,0),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, ELDn, SH6}},
{"clrrdi.",	MD(30,1,1),	MDSH_MASK,   PPC64,	PPCVLE|EXT,	{RA, RS, CRDn}},
{"sldi.",	MD(30,1,1),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, SLDn}},
{"rldicr.",	MD(30,1,1),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, SH6, ME6}},
{"extldi.",	MD(30,1,1),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, ELDn, SH6}},

{"rldic",	MD(30,2,0),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, SH6, MB6}},
{"clrlsldi",	MD(30,2,0),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, CSLDb, CSLDn}},
{"rldic.",	MD(30,2,1),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, SH6, MB6}},
{"clrlsldi.",	MD(30,2,1),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, CSLDb, CSLDn}},

{"rldimi",	MD(30,3,0),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, SH6, MB6}},
{"insrdi",	MD(30,3,0),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, IRDn, IRDb}},
{"rldimi.",	MD(30,3,1),	MD_MASK,     PPC64,	PPCVLE,		{RA, RS, SH6, MB6}},
{"insrdi.",	MD(30,3,1),	MD_MASK,     PPC64,	PPCVLE|EXT,	{RA, RS, IRDn, IRDb}},

{"rotld",	MDS(30,8,0),	MDSMB_MASK,  PPC64,	PPCVLE|EXT,	{RA, RS, RB}},
{"rldcl",	MDS(30,8,0),	MDS_MASK,    PPC64,	PPCVLE,		{RA, RS, RB, MB6}},
{"rotld.",	MDS(30,8,1),	MDSMB_MASK,  PPC64,	PPCVLE|EXT,	{RA, RS, RB}},
{"rldcl.",	MDS(30,8,1),	MDS_MASK,    PPC64,	PPCVLE,		{RA, RS, RB, MB6}},

{"rldcr",	MDS(30,9,0),	MDS_MASK,    PPC64,	PPCVLE,		{RA, RS, RB, ME6}},
{"rldcr.",	MDS(30,9,1),	MDS_MASK,    PPC64,	PPCVLE,		{RA, RS, RB, ME6}},

{"cmpw",	XOPL(31,0,0),	XCMPL_MASK,  PPCCOM,	EXT,		{OBF, RA, RB}},
{"cmpd",	XOPL(31,0,1),	XCMPL_MASK,  PPC64,	EXT,		{OBF, RA, RB}},
{"cmp",		X(31,0),	XCMP_MASK,   PPC,	0,		{BF, L32OPT, RA, RB}},
{"cmp",		X(31,0),	XCMPL_MASK,  PWRCOM,	PPC,		{BF, RA, RB}},

{"twlgt",	XTO(31,4,TOLGT), XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tlgt",	XTO(31,4,TOLGT), XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twllt",	XTO(31,4,TOLLT), XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tllt",	XTO(31,4,TOLLT), XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"tweq",	XTO(31,4,TOEQ),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"teq",		XTO(31,4,TOEQ),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twlge",	XTO(31,4,TOLGE), XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tlge",	XTO(31,4,TOLGE), XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twlnl",	XTO(31,4,TOLNL), XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tlnl",	XTO(31,4,TOLNL), XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twlle",	XTO(31,4,TOLLE), XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tlle",	XTO(31,4,TOLLE), XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twlng",	XTO(31,4,TOLNG), XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tlng",	XTO(31,4,TOLNG), XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twgt",	XTO(31,4,TOGT),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tgt",		XTO(31,4,TOGT),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twge",	XTO(31,4,TOGE),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tge",		XTO(31,4,TOGE),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twnl",	XTO(31,4,TONL),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tnl",		XTO(31,4,TONL),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twlt",	XTO(31,4,TOLT),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tlt",		XTO(31,4,TOLT),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twle",	XTO(31,4,TOLE),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tle",		XTO(31,4,TOLE),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twng",	XTO(31,4,TONG),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tng",		XTO(31,4,TONG),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"twne",	XTO(31,4,TONE),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tne",		XTO(31,4,TONE),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"trap",	XTO(31,4,TOU),	 0xffffffff, PPCCOM,	EXT,		{0}},
{"twu",		XTO(31,4,TOU),	 XTO_MASK,   PPCCOM,	EXT,		{RA, RB}},
{"tu",		XTO(31,4,TOU),	 XTO_MASK,   PWRCOM,	EXT,		{RA, RB}},
{"tw",		X(31,4),	 X_MASK,     PPCCOM,	0,		{TO, RA, RB}},
{"t",		X(31,4),	 X_MASK,     PWRCOM,	0,		{TO, RA, RB}},

{"lvsl",	X(31,6),	X_MASK,	     PPCVEC,	0,		{VD, RA0, RB}},
{"lvebx",	X(31,7),	X_MASK,	     PPCVEC,	0,		{VD, RA0, RB}},
{"lbfcmx",	APU(31,7,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"subfc",	XO(31,8,0,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"sf",		XO(31,8,0,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"subc",	XO(31,8,0,0),	XO_MASK,     PPCCOM,	EXT,		{RT, RB, RA}},
{"subfc.",	XO(31,8,0,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"sf.",		XO(31,8,0,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"subc.",	XO(31,8,0,1),	XO_MASK,     PPCCOM,	EXT,		{RT, RB, RA}},

{"mulhdu",	XO(31,9,0,0),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},
{"mulhdu.",	XO(31,9,0,1),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},

{"addc",	XO(31,10,0,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"a",		XO(31,10,0,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"addc.",	XO(31,10,0,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"a.",		XO(31,10,0,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"mulhwu",	XO(31,11,0,0),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"mulhwu.",	XO(31,11,0,1),	XO_MASK,     PPC,	0,		{RT, RA, RB}},

{"lxsiwzx",	X(31,12),	XX1_MASK,    PPCVSX2,	0,		{XT6, RA0, RB}},

{"lxvrbx",	X(31,13),	XX1_MASK,    POWER10,	0,		{XT6, RA0, RB}},

{"isellt",	XISEL(31,15,0),	X_MASK,	     PPCISEL,	EXT,		{RT, RA0, RB}},
{"iselgt",	XISEL(31,15,1),	X_MASK,	     PPCISEL,	EXT,		{RT, RA0, RB}},
{"iseleq",	XISEL(31,15,2),	X_MASK,	     PPCISEL,	EXT,		{RT, RA0, RB}},
{"isel",	XISEL(31,15,0), XISEL_MASK, PPCISEL|TITAN, 0,		{RT, RA0, RB, BC}},

{"tlbilxlpid",	XTO(31,18,0),	XTO_MASK, E500MC|PPCA2,	0,		{0}},
{"tlbilxpid",	XTO(31,18,1),	XTO_MASK, E500MC|PPCA2,	0,		{0}},
{"tlbilxva",	XTO(31,18,3),	XTO_MASK, E500MC|PPCA2,	0,		{RA0, RB}},
{"tlbilx",	X(31,18),	X_MASK,	  E500MC|PPCA2,	0,		{T, RA0, RB}},

{"mfcr",	XFXM(31,19,0,0), XFXFXM_MASK, COM,	0,		{RT, FXM4}},
{"mfocrf",	XFXM(31,19,0,1), XFXFXM_MASK, COM,	0,		{RT, FXM}},

{"lwarx",	X(31,20),	XEH_MASK,    PPC,	0,		{RT, RA0, RB, EH}},

{"ldx",		X(31,21),	X_MASK,	     PPC64,	0,		{RT, RA0, RB}},

{"icbt",	X(31,22), X_MASK, POWER5|BOOKE|PPCE300, 0,		{CT, RA0, RB}},

{"lwzx",	X(31,23),	X_MASK,	     PPCCOM,	0,		{RT, RA0, RB}},
{"lx",		X(31,23),	X_MASK,	     PWRCOM,	0,		{RT, RA, RB}},

{"slw",		XRC(31,24,0),	X_MASK,	     PPCCOM,	0,		{RA, RS, RB}},
{"sl",		XRC(31,24,0),	X_MASK,	     PWRCOM,	0,		{RA, RS, RB}},
{"slw.",	XRC(31,24,1),	X_MASK,	     PPCCOM,	0,		{RA, RS, RB}},
{"sl.",		XRC(31,24,1),	X_MASK,	     PWRCOM,	0,		{RA, RS, RB}},

{"cntlzw",	XRC(31,26,0),	XRB_MASK,    PPCCOM,	0,		{RA, RS}},
{"cntlz",	XRC(31,26,0),	XRB_MASK,    PWRCOM,	0,		{RA, RS}},
{"cntlzw.",	XRC(31,26,1),	XRB_MASK,    PPCCOM,	0,		{RA, RS}},
{"cntlz.",	XRC(31,26,1),	XRB_MASK,    PWRCOM,	0,		{RA, RS}},

{"sld",		XRC(31,27,0),	X_MASK,	     PPC64,	0,		{RA, RS, RB}},
{"sld.",	XRC(31,27,1),	X_MASK,	     PPC64,	0,		{RA, RS, RB}},

{"and",		XRC(31,28,0),	X_MASK,	     COM,	0,		{RA, RS, RB}},
{"and.",	XRC(31,28,1),	X_MASK,	     COM,	0,		{RA, RS, RB}},

{"maskg",	XRC(31,29,0),	X_MASK,	     M601,	PPCA2,		{RA, RS, RB}},
{"maskg.",	XRC(31,29,1),	X_MASK,	     M601,	PPCA2,		{RA, RS, RB}},

{"ldepx",	X(31,29),	X_MASK,	  E500MC|PPCA2, 0,		{RT, RA0, RB}},

{"waitasec",	X(31,30),      XRTRARB_MASK, POWER8,	POWER9,		{0}},
{"waitrsv",	XWCPL(31,30,1,0),0xffffffff, POWER10,	EXT,		{0}},
{"pause_short",	XWCPL(31,30,2,0),0xffffffff, POWER10,	EXT,		{0}},
{"wait",	X(31,30),	XWCPL_MASK,  POWER10,	0,		{WC, PL}},
{"wait",	X(31,30),	XWC_MASK,    POWER9,	POWER10,	{WC}},

{"lwepx",	X(31,31),	X_MASK,	  E500MC|PPCA2, 0,		{RT, RA0, RB}},

{"cmplw",	XOPL(31,32,0),	XCMPL_MASK,  PPCCOM,	EXT,		{OBF, RA, RB}},
{"cmpld",	XOPL(31,32,1),	XCMPL_MASK,  PPC64,	EXT,		{OBF, RA, RB}},
{"cmpl",	X(31,32),	XCMP_MASK,   PPC,	0,		{BF, L32OPT, RA, RB}},
{"cmpl",	X(31,32),	XCMPL_MASK,  PWRCOM,	PPC,		{BF, RA, RB}},

{"lvsr",	X(31,38),	X_MASK,	     PPCVEC,	0,		{VD, RA0, RB}},
{"lvehx",	X(31,39),	X_MASK,	     PPCVEC,	0,		{VD, RA0, RB}},
{"lhfcmx",	APU(31,39,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"lxvrhx",	X(31,45),	XX1_MASK,    POWER10,	0,		{XT6, RA0, RB}},

{"mviwsplt",	X(31,46),	X_MASK,	     E6500,	0,		{VD, RA, RB}},

{"lvewx",	X(31,71),	X_MASK,	     PPCVEC,	0,		{VD, RA0, RB}},

{"addg6s",	XO(31,74,0,0),	XO_MASK,     POWER6,	0,		{RT, RA, RB}},

{"lxsiwax",	X(31,76),	XX1_MASK,    PPCVSX2,	0,		{XT6, RA0, RB}},

{"lxvrwx",	X(31,77),	XX1_MASK,    POWER10,	0,		{XT6, RA0, RB}},

{"subf",	XO(31,40,0,0),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"sub",		XO(31,40,0,0),	XO_MASK,     PPC,	EXT,		{RT, RB, RA}},
{"subf.",	XO(31,40,0,1),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"sub.",	XO(31,40,0,1),	XO_MASK,     PPC,	EXT,		{RT, RB, RA}},

{"mffprd",	X(31,51),	XX1RB_MASK|1, PPCVSX2,	EXT,		{RA, FRS}},
{"mfvrd",	X(31,51)|1,	XX1RB_MASK|1, PPCVSX2,	EXT,		{RA, VS}},
{"mfvsrd",	X(31,51),	XX1RB_MASK,   PPCVSX2,	0,		{RA, XS6}},
{"eratilx",	X(31,51),	X_MASK,	     PPCA2,	0,		{ERAT_T, RA, RB}},

{"lbarx",	X(31,52),	XEH_MASK, POWER8|E6500, 0,		{RT, RA0, RB, EH}},

{"ldux",	X(31,53),	X_MASK,	     PPC64,	0,		{RT, RAL, RB}},

{"dcbst",	X(31,54),	XRT_MASK,    PPC,	0,		{RA0, RB}},

{"lwzux",	X(31,55),	X_MASK,	     PPCCOM,	0,		{RT, RAL, RB}},
{"lux",		X(31,55),	X_MASK,	     PWRCOM,	0,		{RT, RA, RB}},

{"cntlzd",	XRC(31,58,0),	XRB_MASK,    PPC64,	0,		{RA, RS}},
{"cntlzd.",	XRC(31,58,1),	XRB_MASK,    PPC64,	0,		{RA, RS}},

{"cntlzdm",	X(31,59),	X_MASK,	     POWER10,	0,		{RA, RS, RB}},

{"andc",	XRC(31,60,0),	X_MASK,	     COM,	0,		{RA, RS, RB}},
{"andc.",	XRC(31,60,1),	X_MASK,	     COM,	0,		{RA, RS, RB}},

{"waitrsv",	X(31,62)|(1<<21), 0xffffffff, E500MC|PPCA2, EXT,	{0}},
{"waitimpl",	X(31,62)|(2<<21), 0xffffffff, E500MC|PPCA2, EXT,	{0}},
{"wait",	X(31,62),	XWC_MASK,    E500MC|PPCA2, 0,		{WC}},

{"dcbstep",	XRT(31,63,0),	XRT_MASK,    E500MC|PPCA2, 0,		{RA0, RB}},

{"tdlgt",	XTO(31,68,TOLGT), XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdllt",	XTO(31,68,TOLLT), XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdeq",	XTO(31,68,TOEQ),  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdlge",	XTO(31,68,TOLGE), XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdlnl",	XTO(31,68,TOLNL), XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdlle",	XTO(31,68,TOLLE), XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdlng",	XTO(31,68,TOLNG), XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdgt",	XTO(31,68,TOGT),  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdge",	XTO(31,68,TOGE),  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdnl",	XTO(31,68,TONL),  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdlt",	XTO(31,68,TOLT),  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdle",	XTO(31,68,TOLE),  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdng",	XTO(31,68,TONG),  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdne",	XTO(31,68,TONE),  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"tdu",		XTO(31,68,TOU),	  XTO_MASK,  PPC64,	EXT,		{RA, RB}},
{"td",		X(31,68),	X_MASK,	     PPC64,	0,		{TO, RA, RB}},

{"lwfcmx",	APU(31,71,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},
{"subwus",	XO(31,72,0,0),	XO_MASK,     FUTURE,	EXT,		{RT, RB, RA}},
{"subwus.",	XO(31,72,0,1),	XO_MASK,     FUTURE,	EXT,		{RT, RB, RA}},
{"subdus",	XO(31,72,1,0),	XO_MASK,     FUTURE,	EXT,		{RT, RB, RA}},
{"subdus.",	XO(31,72,1,1),	XO_MASK,     FUTURE,	EXT,		{RT, RB, RA}},
{"subfus",	XO(31,72,0,0),	XOL_MASK,    FUTURE,	0,		{RT, XOL, RA, RB}},
{"subfus.",	XO(31,72,0,1),	XOL_MASK,    FUTURE,	0,		{RT, XOL, RA, RB}},
{"mulhd",	XO(31,73,0,0),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},
{"mulhd.",	XO(31,73,0,1),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},

{"mulhw",	XO(31,75,0,0),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"mulhw.",	XO(31,75,0,1),	XO_MASK,     PPC,	0,		{RT, RA, RB}},

{"msgsndu",	XRTRA(31,78,0,0), XRTRA_MASK, POWER9,	0,		{RB}},
{"dlmzb",	XRC(31,78,0), X_MASK, PPC403|PPC440|PPC476|TITAN, 0,	{RA, RS, RB}},
{"dlmzb.",	XRC(31,78,1), X_MASK, PPC403|PPC440|PPC476|TITAN, 0,	{RA, RS, RB}},

{"mtsrd",	X(31,82),  XRB_MASK|(1<<20), PPC64,	0,		{SR, RS}},

{"mfmsr",	X(31,83),	XRARB_MASK,  COM,	0,		{RT}},

{"ldarx",	X(31,84),	XEH_MASK,    PPC64,	0,		{RT, RA0, RB, EH}},

{"dcbfl",	XOPL(31,86,1),	XRT_MASK,    POWER5,	PPC476|EXT,	{RA0, RB}},
{"dcbflp",	XOPL2(31,86,3), XRT_MASK,    POWER9,	PPC476|EXT,	{RA0, RB}},
{"dcbfps",	XOPL3(31,86,4), XRT_MASK,    POWER10,   PPC476|EXT,	{RA0, RB}},
{"dcbstps",	XOPL3(31,86,6), XRT_MASK,    POWER10,   PPC476|EXT,	{RA0, RB}},
{"dcbf",	X(31,86),	XL3RT_MASK,  POWER10,	PPC476,		{RA0, RB, L3OPT}},
{"dcbf",	X(31,86),	XLRT_MASK,   PPC,	POWER10,	{RA0, RB, L2OPT}},

{"lbzx",	X(31,87),	X_MASK,	     COM,	0,		{RT, RA0, RB}},

{"lbepx",	X(31,95),	X_MASK,	  E500MC|PPCA2, 0,		{RT, RA0, RB}},

{"dni",		XRC(31,97,1),	XRB_MASK,    E6500,	0,		{DUI, DCTL}},

{"lvx",		X(31,103),	X_MASK,	     PPCVEC,	0,		{VD, RA0, RB}},
{"lqfcmx",	APU(31,103,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"neg",		XO(31,104,0,0),	XORB_MASK,   COM,	0,		{RT, RA}},
{"neg.",	XO(31,104,0,1),	XORB_MASK,   COM,	0,		{RT, RA}},

{"mul",		XO(31,107,0,0),	XO_MASK,     M601,	0,		{RT, RA, RB}},
{"mul.",	XO(31,107,0,1),	XO_MASK,     M601,	0,		{RT, RA, RB}},

{"lxvrdx",	X(31,109),	XX1_MASK,    POWER10,	0,		{XT6, RA0, RB}},

{"msgclru",	XRTRA(31,110,0,0), XRTRA_MASK, POWER9,	0,		{RB}},
{"mvidsplt",	X(31,110),	X_MASK,	     E6500,	0,		{VD, RA, RB}},

{"mtsrdin",	X(31,114),	XRA_MASK,    PPC64,	0,		{RS, RB}},

{"mffprwz",	X(31,115),	XX1RB_MASK|1, PPCVSX2,	EXT,		{RA, FRS}},
{"mfvrwz",	X(31,115)|1,	XX1RB_MASK|1, PPCVSX2,	EXT,		{RA, VS}},
{"mfvsrwz",	X(31,115),	XX1RB_MASK,   PPCVSX2,	0,		{RA, XS6}},

{"lharx",	X(31,116),	XEH_MASK, POWER8|E6500, 0,		{RT, RA0, RB, EH}},

{"clf",		X(31,118),	XTO_MASK,    POWER,	0,		{RA, RB}},

{"lbzux",	X(31,119),	X_MASK,	     COM,	0,		{RT, RAL, RB}},

{"popcntb",	X(31,122),	XRB_MASK,    POWER5,	0,		{RA, RS}},

{"not",		XRC(31,124,0),	X_MASK,	     COM,	EXT,		{RA, RSB}},
{"nor",		XRC(31,124,0),	X_MASK,	     COM,	0,		{RA, RS, RB}},
{"not.",	XRC(31,124,1),	X_MASK,	     COM,	EXT,		{RA, RSB}},
{"nor.",	XRC(31,124,1),	X_MASK,	     COM,	0,		{RA, RS, RB}},

{"dcbfep",	XRT(31,127,0),	XRT_MASK, E500MC|PPCA2, 0,		{RA0, RB}},

{"setb",	X(31,128),	XRB_MASK|(3<<16), POWER9, 0,		{RT, BFA}},

{"wrtee",	X(31,131), XRARB_MASK, PPC403|BOOKE|PPCA2|PPC476, 0,	{RS}},

{"dcbtstls",	X(31,134),	X_MASK, PPCCHLK|PPC476|TITAN, 0,	{CT, RA0, RB}},

{"stvebx",	X(31,135),	X_MASK,	     PPCVEC,	0,		{VS, RA0, RB}},
{"stbfcmx",	APU(31,135,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"subfe",	XO(31,136,0,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"sfe",		XO(31,136,0,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"subfe.",	XO(31,136,0,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"sfe.",	XO(31,136,0,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"adde",	XO(31,138,0,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"ae",		XO(31,138,0,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"adde.",	XO(31,138,0,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"ae.",		XO(31,138,0,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"stxsiwx",	X(31,140),	XX1_MASK,    PPCVSX2,	0,		{XS6, RA0, RB}},

{"stxvrbx",	X(31,141),	XX1_MASK,    POWER10,	0,		{XT6, RA0, RB}},

{"msgsndp",	XRTRA(31,142,0,0), XRTRA_MASK, POWER8,	0,		{RB}},
{"dcbtstlse",	X(31,142),	X_MASK,	     PPCCHLK,	E500MC,		{CT, RA0, RB}},

{"mtcr",	XFXM(31,144,0xff,0), XRARB_MASK, COM,	EXT,		{RS}},
{"mtcrf",	XFXM(31,144,0,0), XFXFXM_MASK, COM,	0,		{FXM, RS}},
{"mtocrf",	XFXM(31,144,0,1), XFXFXM_MASK, COM,	0,		{FXM, RS}},

{"mtmsr",	X(31,146),	XRLARB_MASK, COM,	0,		{RS, A_L}},

{"mtsle",	X(31,147),    XRTLRARB_MASK, POWER8,	0,		{L}},
{"eratsx",	XRC(31,147,0),	X_MASK,	     PPCA2,	0,		{RT, RA0, RB}},
{"eratsx.",	XRC(31,147,1),	X_MASK,	     PPCA2,	0,		{RT, RA0, RB}},

{"stdx",	X(31,149),	X_MASK,	     PPC64,	0,		{RS, RA0, RB}},

{"stwcx.",	XRC(31,150,1),	X_MASK,	     PPC,	0,		{RS, RA0, RB}},

{"stwx",	X(31,151),	X_MASK,	     PPCCOM,	0,		{RS, RA0, RB}},
{"stx",		X(31,151),	X_MASK,	     PWRCOM,	0,		{RS, RA, RB}},

{"slq",		XRC(31,152,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"slq.",	XRC(31,152,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"sle",		XRC(31,153,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"sle.",	XRC(31,153,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"prtyw",	X(31,154),    XRB_MASK, POWER6|PPCA2|PPC476, 0,		{RA, RS}},

{"brw",		X(31,155),	XRB_MASK,    POWER10,	0,		{RA, RS}},
{"pdepd",	X(31,156),	X_MASK,	     POWER10,	0,		{RA, RS, RB}},

{"stdepx",	X(31,157),	X_MASK,	  E500MC|PPCA2, 0,		{RS, RA0, RB}},

{"stwepx",	X(31,159),	X_MASK,	  E500MC|PPCA2, 0,		{RS, RA0, RB}},

{"wrteei",	X(31,163), XE_MASK, PPC403|BOOKE|PPCA2|PPC476, 0,	{E}},

{"dcbtls",	X(31,166),	X_MASK,	 PPCCHLK|PPC476|TITAN, 0,	{CT, RA0, RB}},

{"stvehx",	X(31,167),	X_MASK,	     PPCVEC,	0,		{VS, RA0, RB}},
{"sthfcmx",	APU(31,167,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"addex",	ZRC(31,170,0),	Z2_MASK,     POWER9,	0,		{RT, RA, RB, CY}},

{"stxvrhx",	X(31,173),	XX1_MASK,    POWER10,	0,		{XT6, RA0, RB}},

{"msgclrp",	XRTRA(31,174,0,0), XRTRA_MASK, POWER8,	0,		{RB}},
{"dcbtlse",	X(31,174),	X_MASK,	     PPCCHLK,	E500MC,		{CT, RA0, RB}},

{"dmxxmfacc",	XVA(31,177,0),	XACC_MASK,   POWER10, 0,		{ACC}},
{"xxmfacc",	XVA(31,177,0),	XACC_MASK,   POWER10, 0,		{ACC}},
{"dmxxmtacc",	XVA(31,177,1),	XACC_MASK,   POWER10, 0,		{ACC}},
{"xxmtacc",	XVA(31,177,1),	XACC_MASK,   POWER10, 0,		{ACC}},
{"dmsetdmrz",	XVA(31,177,2),	XDMR_MASK,   FUTURE,  0,		{DMR}},
{"dmsetaccz",	XVA(31,177,3),	XACC_MASK,   POWER10, 0,		{ACC}},
{"xxsetaccz",	XVA(31,177,3),	XACC_MASK,   POWER10, 0,		{ACC}},
{"dmmr",	XVA(31,177,6),	XDMRDMR_MASK,FUTURE,  0,		{DMR, DMRAB}},
{"dmxor",	XVA(31,177,7),	XDMRDMR_MASK,FUTURE,  0,		{DMR, DMRAB}},

{"mtmsrd",	X(31,178),	XRLARB_MASK, PPC64,	0,		{RS, A_L}},

{"mtfprd",	X(31,179),	XX1RB_MASK|1, PPCVSX2,	EXT,		{FRT, RA}},
{"mtvrd",	X(31,179)|1,	XX1RB_MASK|1, PPCVSX2,	EXT,		{VD, RA}},
{"mtvsrd",	X(31,179),	XX1RB_MASK,   PPCVSX2,	0,		{XT6, RA}},
{"eratre",	X(31,179),	X_MASK,	     PPCA2,	0,		{RT, RA, WS}},

{"stdux",	X(31,181),	X_MASK,	     PPC64,	0,		{RS, RAS, RB}},

{"stqcx.",	XRC(31,182,1), X_MASK|Q_MASK, POWER8,	0,		{RSQ, RA0, RB}},
{"wchkall",	X(31,182),	X_MASK,	     PPCA2,	0,		{OBF}},

{"stwux",	X(31,183),	X_MASK,	     PPCCOM,	0,		{RS, RAS, RB}},
{"stux",	X(31,183),	X_MASK,	     PWRCOM,	0,		{RS, RA0, RB}},

{"sliq",	XRC(31,184,0),	X_MASK,	     M601,	0,		{RA, RS, SH}},
{"sliq.",	XRC(31,184,1),	X_MASK,	     M601,	0,		{RA, RS, SH}},

{"prtyd",	X(31,186),	XRB_MASK, POWER6|PPCA2,	0,		{RA, RS}},

{"brd",		X(31,187),	XRB_MASK,    POWER10,	0,		{RA, RS}},
{"pextd",	X(31,188),	X_MASK,	     POWER10,	0,		{RA, RS, RB}},

{"cmprb",	X(31,192),	XCMP_MASK,   POWER9,	0,		{BF, L, RA, RB}},

{"icblq.",	XRC(31,198,1),	X_MASK,	     E6500,	0,		{CT, RA0, RB}},

{"stvewx",	X(31,199),	X_MASK,	     PPCVEC,	0,		{VS, RA0, RB}},
{"stwfcmx",	APU(31,199,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"subfze",	XO(31,200,0,0),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"sfze",	XO(31,200,0,0),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},
{"subfze.",	XO(31,200,0,1),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"sfze.",	XO(31,200,0,1),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},

{"addze",	XO(31,202,0,0),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"aze",		XO(31,202,0,0),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},
{"addze.",	XO(31,202,0,1),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"aze.",	XO(31,202,0,1),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},

{"stxvrwx",	X(31,205),	XX1_MASK,    POWER10,	0,		{XT6, RA0, RB}},

{"msgsnd",	XRTRA(31,206,0,0), XRTRA_MASK, E500MC|PPCA2|POWER8, 0,	{RB}},

{"mtsr",	X(31,210), XRB_MASK|(1<<20), COM,	NON32,		{SR, RS}},

{"mtfprwa",	X(31,211),	XX1RB_MASK|1, PPCVSX2,	EXT,		{FRT, RA}},
{"mtvrwa",	X(31,211)|1,	XX1RB_MASK|1, PPCVSX2,	EXT,		{VD, RA}},
{"mtvsrwa",	X(31,211),	XX1RB_MASK,   PPCVSX2,	0,		{XT6, RA}},
{"eratwe",	X(31,211),	X_MASK,	     PPCA2,	0,		{RS, RA, WS}},

{"ldawx.",	XRC(31,212,1),	X_MASK,	     PPCA2,	0,		{RT, RA0, RB}},

{"stdcx.",	XRC(31,214,1),	X_MASK,	     PPC64,	0,		{RS, RA0, RB}},

{"stbx",	X(31,215),	X_MASK,	     COM,	0,		{RS, RA0, RB}},

{"sllq",	XRC(31,216,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"sllq.",	XRC(31,216,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"sleq",	XRC(31,217,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"sleq.",	XRC(31,217,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"brh",		X(31,219),	XRB_MASK,    POWER10,	0,		{RA, RS}},
{"cfuged",	X(31,220),	X_MASK,	     POWER10,	0,		{RA, RS, RB}},

{"stbepx",	X(31,223),	X_MASK,	  E500MC|PPCA2, 0,		{RS, RA0, RB}},

{"cmpeqb",	X(31,224),	XCMPL_MASK,  POWER9,	0,		{BF, RA, RB}},

{"icblc",	X(31,230),	X_MASK,	PPCCHLK|PPC476|TITAN, 0,	{CT, RA0, RB}},

{"stvx",	X(31,231),	X_MASK,	     PPCVEC,	0,		{VS, RA0, RB}},
{"stqfcmx",	APU(31,231,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"subfme",	XO(31,232,0,0),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"sfme",	XO(31,232,0,0),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},
{"subfme.",	XO(31,232,0,1),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"sfme.",	XO(31,232,0,1),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},

{"mulld",	XO(31,233,0,0),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},
{"mulld.",	XO(31,233,0,1),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},

{"addme",	XO(31,234,0,0),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"ame",		XO(31,234,0,0),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},
{"addme.",	XO(31,234,0,1),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"ame.",	XO(31,234,0,1),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},

{"mullw",	XO(31,235,0,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"muls",	XO(31,235,0,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"mullw.",	XO(31,235,0,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"muls.",	XO(31,235,0,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"stxvrdx",	X(31,237),	XX1_MASK,    POWER10,	0,		{XT6, RA0, RB}},

{"icblce",	X(31,238),	X_MASK,	     PPCCHLK,	E500MC|PPCA2,	{CT, RA, RB}},
{"msgclr",	XRTRA(31,238,0,0), XRTRA_MASK, E500MC|PPCA2|POWER8, 0,	{RB}},
{"mtsrin",	X(31,242),	XRA_MASK,    PPC,	NON32,		{RS, RB}},
{"mtsri",	X(31,242),	XRA_MASK,    POWER,	NON32,		{RS, RB}},

{"mtfprwz",	X(31,243),	XX1RB_MASK|1, PPCVSX2,	EXT,		{FRT, RA}},
{"mtvrwz",	X(31,243)|1,	XX1RB_MASK|1, PPCVSX2,	EXT,		{VD, RA}},
{"mtvsrwz",	X(31,243),	XX1RB_MASK,   PPCVSX2,	0,		{XT6, RA}},

{"dcbtstt",	XRT(31,246,0x10), XRT_MASK,  POWER7,	EXT,		{RA0, RB}},
{"dcbtstct",	X(31,246),	X_MASK,	     POWER4,	EXT,		{RA0, RB, THCT}},
{"dcbtstds",	X(31,246),	X_MASK,	     POWER4,	EXT,		{RA0, RB, THDS}},
{"dcbtst",	X(31,246),	X_MASK,	     POWER4,	DCBT_EO,	{RA0, RB, CT}},
{"dcbtst",	X(31,246),	X_MASK,	     DCBT_EO,	0,		{CT, RA0, RB}},
{"dcbtst",	X(31,246),	X_MASK,	     PPC,	POWER4|DCBT_EO,	{RA0, RB}},

{"stbux",	X(31,247),	X_MASK,	     COM,	0,		{RS, RAS, RB}},

{"slliq",	XRC(31,248,0),	X_MASK,	     M601,	0,		{RA, RS, SH}},
{"slliq.",	XRC(31,248,1),	X_MASK,	     M601,	0,		{RA, RS, SH}},

{"bpermd",	X(31,252),	X_MASK,	  POWER7|PPCA2,	0,		{RA, RS, RB}},

{"dcbtstep",	XRT(31,255,0),	X_MASK,	  E500MC|PPCA2, 0,		{RT, RA0, RB}},

{"mfdcrx",	X(31,259),	X_MASK, BOOKE|PPCA2|PPC476, TITAN,	{RS, RA}},
{"mfdcrx.",	XRC(31,259,1),	X_MASK,	     PPCA2,	0,		{RS, RA}},

{"lvexbx",	X(31,261),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"icbt",	X(31,262),	XRT_MASK,    PPC403,	0,		{RA, RB}},

{"lvepxl",	X(31,263),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},
{"ldfcmx",	APU(31,263,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"doz",		XO(31,264,0,0),	XO_MASK,     M601,	0,		{RT, RA, RB}},
{"doz.",	XO(31,264,0,1),	XO_MASK,     M601,	0,		{RT, RA, RB}},

{"modud",	X(31,265),	X_MASK,	     POWER9,	0,		{RT, RA, RB}},

{"add",		XO(31,266,0,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"cax",		XO(31,266,0,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"add.",	XO(31,266,0,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"cax.",	XO(31,266,0,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"moduw",	X(31,267),	X_MASK,	     POWER9,	0,		{RT, RA, RB}},

{"lxvx",	X(31,268),	XX1_MASK|1<<6, PPCVSX3,	0,		{XT6, RA0, RB}},
{"lxvl",	X(31,269),	XX1_MASK,    PPCVSX3,	0,		{XT6, RA0, RB}},

{"ehpriv",	X(31,270),	0xffffffff,  E500MC|PPCA2, 0,		{0}},

{"tlbiel",	X(31,274),	X_MASK|1<<20,POWER9,	0,		{RB, RSO, RIC, PRS, X_R}},
{"tlbiel",	X(31,274),	XRTLRA_MASK, POWER4,	POWER9|PPC476,	{RB, LOPT}},

{"mfapidi",	X(31,275),	X_MASK,	     BOOKE,	E500|TITAN,	{RT, RA}},

{"lqarx",	X(31,276),  XEH_MASK|Q_MASK, POWER8,	0,		{RTQ, RAX, RBX, EH}},

{"lscbx",	XRC(31,277,0),	X_MASK,	     M601,	0,		{RT, RA, RB}},
{"lscbx.",	XRC(31,277,1),	X_MASK,	     M601,	0,		{RT, RA, RB}},

{"dcbtt",	XRT(31,278,0x10), XRT_MASK,  POWER7,	EXT,		{RA0, RB}},
{"dcbna",	XRT(31,278,0x11), XRT_MASK,  POWER10,	EXT,		{RA0, RB}},
{"dcbtct",	X(31,278),	X_MASK,      POWER4,	EXT,		{RA0, RB, THCT}},
{"dcbtds",	X(31,278),	X_MASK,      POWER4,	EXT,		{RA0, RB, THDS}},
{"dcbt",	X(31,278),	X_MASK,	     POWER4,	DCBT_EO,	{RA0, RB, CT}},
{"dcbt",	X(31,278),	X_MASK,	     DCBT_EO,	0,		{CT, RA0, RB}},
{"dcbt",	X(31,278),	X_MASK,	     PPC,	POWER4|DCBT_EO,	{RA0, RB}},

{"lhzx",	X(31,279),	X_MASK,	     COM,	0,		{RT, RA0, RB}},

{"cdtbcd",	X(31,282),	XRB_MASK,    POWER6,	0,		{RA, RS}},

{"eqv",		XRC(31,284,0),	X_MASK,	     COM,	0,		{RA, RS, RB}},
{"eqv.",	XRC(31,284,1),	X_MASK,	     COM,	0,		{RA, RS, RB}},

{"lhepx",	X(31,287),	X_MASK,	  E500MC|PPCA2, 0,		{RT, RA0, RB}},

{"mfdcrux",	X(31,291),	X_MASK,	 PPC464|PPC476,	0,		{RS, RA}},

{"lvexhx",	X(31,293),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},
{"lvepx",	X(31,295),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"lxvll",	X(31,301),	XX1_MASK,    PPCVSX3,	0,		{XT6, RA0, RB}},

{"mfbhrbe",	X(31,302),	X_MASK,	     POWER8,	0,		{RT, BHRBE}},

{"tlbie",	X(31,306),	X_MASK|1<<20,POWER9,	TITAN,		{RB, RS, RIC, PRS, X_R}},
{"tlbie",	X(31,306),	XRA_MASK,    POWER7,	POWER9|TITAN,	{RB, RS}},
{"tlbie",	X(31,306),	XRTLRA_MASK, PPC,    E500|POWER7|TITAN,	{RB, LOPT}},
{"tlbi",	X(31,306),	XRT_MASK,    POWER,	0,		{RA0, RB}},

{"mfvsrld",	X(31,307),	XX1RB_MASK,  PPCVSX3,	0,		{RA, XS6}},

{"eciwx",	X(31,310),	X_MASK,	     PPC,	E500|TITAN,	{RT, RA0, RB}},

{"lhzux",	X(31,311),	X_MASK,	     COM,	0,		{RT, RAL, RB}},

{"cbcdtd",	X(31,314),	XRB_MASK,    POWER6,	0,		{RA, RS}},

{"xor",		XRC(31,316,0),	X_MASK,	     COM,	0,		{RA, RS, RB}},
{"xor.",	XRC(31,316,1),	X_MASK,	     COM,	0,		{RA, RS, RB}},

{"dcbtep",	XRT(31,319,0),	X_MASK,	  E500MC|PPCA2, 0,		{RT, RA0, RB}},

{"mfexisr",	XSPR(31,323, 64), XSPR_MASK, PPC403,	0,		{RT}},
{"mfexier",	XSPR(31,323, 66), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbr0",	XSPR(31,323,128), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbr1",	XSPR(31,323,129), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbr2",	XSPR(31,323,130), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbr3",	XSPR(31,323,131), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbr4",	XSPR(31,323,132), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbr5",	XSPR(31,323,133), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbr6",	XSPR(31,323,134), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbr7",	XSPR(31,323,135), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbear",	XSPR(31,323,144), XSPR_MASK, PPC403,	0,		{RT}},
{"mfbesr",	XSPR(31,323,145), XSPR_MASK, PPC403,	0,		{RT}},
{"mfiocr",	XSPR(31,323,160), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmacr0",	XSPR(31,323,192), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmact0",	XSPR(31,323,193), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmada0",	XSPR(31,323,194), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmasa0",	XSPR(31,323,195), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmacc0",	XSPR(31,323,196), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmacr1",	XSPR(31,323,200), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmact1",	XSPR(31,323,201), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmada1",	XSPR(31,323,202), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmasa1",	XSPR(31,323,203), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmacc1",	XSPR(31,323,204), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmacr2",	XSPR(31,323,208), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmact2",	XSPR(31,323,209), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmada2",	XSPR(31,323,210), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmasa2",	XSPR(31,323,211), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmacc2",	XSPR(31,323,212), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmacr3",	XSPR(31,323,216), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmact3",	XSPR(31,323,217), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmada3",	XSPR(31,323,218), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmasa3",	XSPR(31,323,219), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmacc3",	XSPR(31,323,220), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdmasr",	XSPR(31,323,224), XSPR_MASK, PPC403,	0,		{RT}},
{"mfdcr",	X(31,323), X_MASK, PPC403|BOOKE|PPCA2|PPC476, E500|TITAN, {RT, SPR}},
{"mfdcr.",	XRC(31,323,1),	X_MASK,	     PPCA2,	0,		{RT, SPR}},

{"lvexwx",	X(31,325),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"dcread",	X(31,326),	X_MASK,	  PPC476|TITAN,	0,		{RT, RA0, RB}},

{"div",		XO(31,331,0,0),	XO_MASK,     M601,	0,		{RT, RA, RB}},
{"div.",	XO(31,331,0,1),	XO_MASK,     M601,	0,		{RT, RA, RB}},

{"lxvdsx",	X(31,332),	XX1_MASK,    PPCVSX,	0,		{XT6, RA0, RB}},

{"lxvpx",	X(31,333),	XX1_MASK,    POWER10,	0,		{XTP, RA0, RB}},

{"mfpmr",	X(31,334),	X_MASK, PPCPMR|PPCE300, 0,		{RT, PMR}},
{"mftmr",	X(31,366),	X_MASK,	     PPCTMR,	0,		{RT, TMR}},

{"slbsync",	X(31,338),	0xffffffff,  POWER9,	0,		{0}},

{"mfmq",	XSPR(31,339,  0), XSPR_MASK, M601,	EXT,		{RT}},
{"mfxer",	XSPR(31,339,  1), XSPR_MASK, COM,	EXT,		{RT}},
{"mfudscr",	XSPR(31,339,  3), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfrtcu",	XSPR(31,339,  4), XSPR_MASK, COM,	TITAN|EXT,	{RT}},
{"mfrtcl",	XSPR(31,339,  5), XSPR_MASK, COM,	TITAN|EXT,	{RT}},
{"mfdec",	XSPR(31,339,  6), XSPR_MASK, MFDEC1,	EXT,		{RT}},
{"mflr",	XSPR(31,339,  8), XSPR_MASK, COM,	EXT,		{RT}},
{"mfctr",	XSPR(31,339,  9), XSPR_MASK, COM,	EXT,		{RT}},
{"mfuamr",	XSPR(31,339, 13), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfdscr",	XSPR(31,339, 17), XSPR_MASK, POWER6,	EXT,		{RT}},
{"mftid",	XSPR(31,339, 17), XSPR_MASK, POWER,	EXT,		{RT}},
{"mfdsisr",	XSPR(31,339, 18), XSPR_MASK, COM,	TITAN|EXT,	{RT}},
{"mfdar",	XSPR(31,339, 19), XSPR_MASK, COM,	TITAN|EXT,	{RT}},
{"mfdec",	XSPR(31,339, 22), XSPR_MASK, MFDEC2,	MFDEC1|EXT,	{RT}},
{"mfsdr0",	XSPR(31,339, 24), XSPR_MASK, POWER,	EXT,		{RT}},
{"mfsdr1",	XSPR(31,339, 25), XSPR_MASK, COM,	TITAN|EXT,	{RT}},
{"mfsrr0",	XSPR(31,339, 26), XSPR_MASK, COM,	EXT,		{RT}},
{"mfsrr1",	XSPR(31,339, 27), XSPR_MASK, COM,	EXT,		{RT}},
{"mfcfar",	XSPR(31,339, 28), XSPR_MASK, POWER6,	EXT,		{RT}},
{"mfamr",	XSPR(31,339, 29), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mfpidr",	XSPR(31,339, 48), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfpid",	XSPR(31,339, 48), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfcsrr0",	XSPR(31,339, 58), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfcsrr1",	XSPR(31,339, 59), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfiamr",	XSPR(31,339, 61), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdear",	XSPR(31,339, 61), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfesr",	XSPR(31,339, 62), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivpr",	XSPR(31,339, 63), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfctrl",	XSPR(31,339,136), XSPR_MASK, POWER4,	EXT,		{RT}},
{"mfcmpa",	XSPR(31,339,144), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfcmpb",	XSPR(31,339,145), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfcmpc",	XSPR(31,339,146), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfcmpd",	XSPR(31,339,147), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mficr",	XSPR(31,339,148), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfder",	XSPR(31,339,149), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfcounta",	XSPR(31,339,150), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfcountb",	XSPR(31,339,151), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfcmpe",	XSPR(31,339,152), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mffscr",	XSPR(31,339,153), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfcmpf",	XSPR(31,339,153), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfcmpg",	XSPR(31,339,154), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfcmph",	XSPR(31,339,155), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mflctrl1",	XSPR(31,339,156), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfuamor",	XSPR(31,339,157), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mflctrl2",	XSPR(31,339,157), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfictrl",	XSPR(31,339,158), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfpspb",	XSPR(31,339,159), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfbar",	XSPR(31,339,159), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfdpdes",	XSPR(31,339,176), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdawr0",	XSPR(31,339,180), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdawr1",	XSPR(31,339,181), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfrpr",	XSPR(31,339,186), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfciabr",	XSPR(31,339,187), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdawrx0",	XSPR(31,339,188), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdawrx1",	XSPR(31,339,189), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfhfscr",	XSPR(31,339,190), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfvrsave",	XSPR(31,339,256), XSPR_MASK, PPCVEC,	EXT,		{RT}},
{"mfusprg0",	XSPR(31,339,256), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfsprg",	XSPR(31,339,256), XSPRG_MASK, PPC,	EXT,		{RT, SPRG}},
{"mfusprg3",	XSPR(31,339,259), XSPR_MASK, POWER10,	EXT,		{RT}},
{"mfsprg4",	XSPR(31,339,260), XSPR_MASK, PPC405|BOOKE, EXT,		{RT}},
{"mfsprg5",	XSPR(31,339,261), XSPR_MASK, PPC405|BOOKE, EXT,		{RT}},
{"mfsprg6",	XSPR(31,339,262), XSPR_MASK, PPC405|BOOKE, EXT,		{RT}},
{"mfsprg7",	XSPR(31,339,263), XSPR_MASK, PPC405|BOOKE, EXT,		{RT}},
{"mftbu",	XSPR(31,339,269), XSPR_MASK, POWER4|BOOKE, EXT,		{RT}},
{"mftb",	X(31,339),	  X_MASK,    POWER4|BOOKE, EXT,		{RT, TBR}},
{"mftbl",	XSPR(31,339,268), XSPR_MASK, POWER4|BOOKE, EXT,		{RT}},
{"mfsprg0",	XSPR(31,339,272), XSPR_MASK, PPC,	EXT,		{RT}},
{"mfsprg1",	XSPR(31,339,273), XSPR_MASK, PPC,	EXT,		{RT}},
{"mfsprg2",	XSPR(31,339,274), XSPR_MASK, PPC,	EXT,		{RT}},
{"mfsprg3",	XSPR(31,339,275), XSPR_MASK, PPC,	EXT,		{RT}},
{"mfasr",	XSPR(31,339,280), XSPR_MASK, PPC64,	EXT,		{RT}},
{"mfear",	XSPR(31,339,282), XSPR_MASK, PPC,	TITAN|EXT,	{RT}},
{"mfpir",	XSPR(31,339,286), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfpvr",	XSPR(31,339,287), XSPR_MASK, PPC,	EXT,		{RT}},
{"mfhsprg0",	XSPR(31,339,304), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdbsr",	XSPR(31,339,304), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfhsprg1",	XSPR(31,339,305), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfhdisr",	XSPR(31,339,306), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfhdar",	XSPR(31,339,307), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfspurr",	XSPR(31,339,308), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdbcr0",	XSPR(31,339,308), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfpurr",	XSPR(31,339,309), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdbcr1",	XSPR(31,339,309), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfhdec",	XSPR(31,339,310), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdbcr2",	XSPR(31,339,310), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfiac1",	XSPR(31,339,312), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfhrmor",	XSPR(31,339,313), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfiac2",	XSPR(31,339,313), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfhsrr0",	XSPR(31,339,314), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfiac3",	XSPR(31,339,314), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfhsrr1",	XSPR(31,339,315), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfiac4",	XSPR(31,339,315), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfdac1",	XSPR(31,339,316), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfdac2",	XSPR(31,339,317), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mflpcr",	XSPR(31,339,318), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdvc1",	XSPR(31,339,318), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mflpidr",	XSPR(31,339,319), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfdvc2",	XSPR(31,339,319), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfhmer",	XSPR(31,339,336), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mftsr",	XSPR(31,339,336), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfhmeer",	XSPR(31,339,337), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mfpcr",	XSPR(31,339,338), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfheir",	XSPR(31,339,339), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mftcr",	XSPR(31,339,340), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfamor",	XSPR(31,339,349), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mfivor0",	XSPR(31,339,400), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor1",	XSPR(31,339,401), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor2",	XSPR(31,339,402), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor3",	XSPR(31,339,403), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor4",	XSPR(31,339,404), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor5",	XSPR(31,339,405), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor6",	XSPR(31,339,406), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor7",	XSPR(31,339,407), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor8",	XSPR(31,339,408), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor9",	XSPR(31,339,409), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor10",	XSPR(31,339,410), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor11",	XSPR(31,339,411), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor12",	XSPR(31,339,412), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor13",	XSPR(31,339,413), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor14",	XSPR(31,339,414), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mfivor15",	XSPR(31,339,415), XSPR_MASK, BOOKE,	EXT,		{RT}},
{"mftir",	XSPR(31,339,446), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfptcr",	XSPR(31,339,464), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfusprg0",	XSPR(31,339,496), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfusprg1",	XSPR(31,339,497), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfurmor",	XSPR(31,339,505), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfusrr0",	XSPR(31,339,506), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfusrr1",	XSPR(31,339,507), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfsmfctrl",	XSPR(31,339,511), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfspefscr",	XSPR(31,339,512), XSPR_MASK, PPCSPE,	EXT,		{RT}},
{"mfbbear",	XSPR(31,339,513), XSPR_MASK, PPCBRLK,	EXT,		{RT}},
{"mfbbtar",	XSPR(31,339,514), XSPR_MASK, PPCBRLK,	EXT,		{RT}},
{"mfivor32",	XSPR(31,339,528), XSPR_MASK, PPCSPE|E6500, EXT,		{RT}},
{"mfivor33",	XSPR(31,339,529), XSPR_MASK, PPCSPE|E6500, EXT,		{RT}},
{"mfivor34",	XSPR(31,339,530), XSPR_MASK, PPCSPE,	EXT,		{RT}},
{"mfivor35",	XSPR(31,339,531), XSPR_MASK, PPCPMR,	EXT,		{RT}},
{"mfibatu",	XSPR(31,339,528), XSPRBAT_MASK, PPC,	TITAN|EXT,	{RT, SPRBAT}},
{"mfibatl",	XSPR(31,339,529), XSPRBAT_MASK, PPC,	TITAN|EXT,	{RT, SPRBAT}},
{"mfdbatu",	XSPR(31,339,536), XSPRBAT_MASK, PPC,	TITAN|EXT,	{RT, SPRBAT}},
{"mfdbatl",	XSPR(31,339,537), XSPRBAT_MASK, PPC,	TITAN|EXT,	{RT, SPRBAT}},
{"mfic_cst",	XSPR(31,339,560), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfic_adr",	XSPR(31,339,561), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfic_dat",	XSPR(31,339,562), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfdc_cst",	XSPR(31,339,568), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfdc_adr",	XSPR(31,339,569), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfdc_dat",	XSPR(31,339,570), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmcsrr0",	XSPR(31,339,570), XSPR_MASK, PPCRFMCI,	EXT,		{RT}},
{"mfmcsrr1",	XSPR(31,339,571), XSPR_MASK, PPCRFMCI,	EXT,		{RT}},
{"mfmcsr",	XSPR(31,339,572), XSPR_MASK, PPCRFMCI,	EXT,		{RT}},
{"mfmcar",	XSPR(31,339,573), XSPR_MASK, PPCRFMCI,	TITAN|EXT,	{RT}},
{"mfdpdr",	XSPR(31,339,630), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfdpir",	XSPR(31,339,631), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfimmr",	XSPR(31,339,638), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfusier2",	XSPR(31,339,736), XSPR_MASK, POWER10,	EXT, 		{RT}},
{"mfsier2",	XSPR(31,339,736), XSPR_MASK, POWER10,	EXT, 		{RT}},
{"mfusier3",	XSPR(31,339,737), XSPR_MASK, POWER10,	EXT, 		{RT}},
{"mfsier3",	XSPR(31,339,737), XSPR_MASK, POWER10,	EXT, 		{RT}},
{"mfummcr3",	XSPR(31,339,738), XSPR_MASK, POWER10,	EXT, 		{RT}},
{"mfmmcr3",	XSPR(31,339,738), XSPR_MASK, POWER10,	EXT, 		{RT}},
{"mfusier",	XSPR(31,339,768), XSPR_MASK, POWER10,	EXT, 		{RT}},
{"mfsier",	XSPR(31,339,768), XSPR_MASK, POWER10,	EXT, 		{RT}},
{"mfummcr2",	XSPR(31,339,769), XSPR_MASK, POWER9,	EXT, 		{RT}},
{"mfmmcr2",	XSPR(31,339,769), XSPR_MASK, POWER9,	EXT, 		{RT}},
{"mfummcra",	XSPR(31,339,770), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfmmcra",	XSPR(31,339,770), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mfupmc1",	XSPR(31,339,771), XSPR_MASK, POWER9,	EXT, 		{RT}},
{"mfpmc1",	XSPR(31,339,771), XSPR_MASK, POWER7,	EXT,		{RT}},
{"mfupmc2",	XSPR(31,339,772), XSPR_MASK, POWER9,	EXT,		{RT}},
{"mfpmc2",	XSPR(31,339,772), XSPR_MASK, POWER7,	EXT,		{RT}},
{"mfupmc3",	XSPR(31,339,773), XSPR_MASK, POWER9,	EXT,		{RT}},
{"mfpmc3",	XSPR(31,339,773), XSPR_MASK, POWER7,	EXT,		{RT}},
{"mfupmc4",	XSPR(31,339,774), XSPR_MASK, POWER9,	EXT,		{RT}},
{"mfpmc4",	XSPR(31,339,774), XSPR_MASK, POWER7,	EXT,		{RT}},
{"mfupmc5",	XSPR(31,339,775), XSPR_MASK, POWER9,	EXT,		{RT}},
{"mfpmc5",	XSPR(31,339,775), XSPR_MASK, POWER7,	EXT,		{RT}},
{"mfupmc6",	XSPR(31,339,776), XSPR_MASK, POWER9,	EXT,		{RT}},
{"mfpmc6",	XSPR(31,339,776), XSPR_MASK, POWER7,	EXT,		{RT}},
{"mfummcr0",	XSPR(31,339,779), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfmmcr0",	XSPR(31,339,779), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mfusiar",	XSPR(31,339,780), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfsiar",	XSPR(31,339,780), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfusdar",	XSPR(31,339,781), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfsdar",	XSPR(31,339,781), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfummcr1",	XSPR(31,339,782), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfmmcr1",	XSPR(31,339,782), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mfmi_ctr",	XSPR(31,339,784), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmi_ap",	XSPR(31,339,786), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmi_epn",	XSPR(31,339,787), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmi_twc",	XSPR(31,339,789), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmi_rpn",	XSPR(31,339,790), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmd_ctr",	XSPR(31,339,792), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfm_casid",	XSPR(31,339,793), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmd_ap",	XSPR(31,339,794), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmd_epn",	XSPR(31,339,795), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmd_twb",	XSPR(31,339,796), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmd_twc",	XSPR(31,339,797), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmd_rpn",	XSPR(31,339,798), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfm_tw",	XSPR(31,339,799), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfbescrs",	XSPR(31,339,800), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfbescrsu",	XSPR(31,339,801), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfbescrr",	XSPR(31,339,802), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfbescrru",	XSPR(31,339,803), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfebbhr",	XSPR(31,339,804), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfebbrr",	XSPR(31,339,805), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfbescr",	XSPR(31,339,806), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mftar",	XSPR(31,339,815), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mfasdr",	XSPR(31,339,816), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfmi_dbcam",	XSPR(31,339,816), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmi_dbram0",	XSPR(31,339,817), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmi_dbram1",	XSPR(31,339,818), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfpsscr",	XSPR(31,339,823), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfmd_dbcam",	XSPR(31,339,824), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmd_dbram0",	XSPR(31,339,825), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfmd_dbram1",	XSPR(31,339,826), XSPR_MASK, PPC860,	EXT,		{RT}},
{"mfic",	XSPR(31,339,848), XSPR_MASK, POWER8,	EXT,		{RS}},
{"mfvtb",	XSPR(31,339,849), XSPR_MASK, POWER8,	EXT,		{RS}},
{"mfhpsscr",	XSPR(31,339,855), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mfivndx",	XSPR(31,339,880), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfdvndx",	XSPR(31,339,881), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfivlim",	XSPR(31,339,882), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfdvlim",	XSPR(31,339,883), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfclcsr",	XSPR(31,339,884), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfccr1",	XSPR(31,339,888), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfppr",	XSPR(31,339,896), XSPR_MASK, POWER5,	EXT,		{RT}},
{"mfppr32",	XSPR(31,339,898), XSPR_MASK, POWER5,	EXT,		{RT}},
{"mfgqr",	XSPR(31,339,912), XSPRGQR_MASK, PPCPS,	EXT,		{RT, SPRGQR}},
{"mfhid2",	XSPR(31,339,920), XSPR_MASK, GEKKO,	EXT,		{RT}},
{"mfwpar",	XSPR(31,339,921), XSPR_MASK, GEKKO,	EXT,		{RT}},
{"mfdmau",	XSPR(31,339,922), XSPR_MASK, GEKKO,	EXT,		{RT}},
{"mfdmal",	XSPR(31,339,923), XSPR_MASK, GEKKO,	EXT,		{RT}},
{"mfrstcfg",	XSPR(31,339,923), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfdcdbtrl",	XSPR(31,339,924), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfdcdbtrh",	XSPR(31,339,925), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mficdbtr",	XSPR(31,339,927), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfummcr0",	XSPR(31,339,936), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfupmc1",	XSPR(31,339,937), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfupmc2",	XSPR(31,339,938), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfusia",	XSPR(31,339,939), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfummcr1",	XSPR(31,339,940), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfupmc3",	XSPR(31,339,941), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfupmc4",	XSPR(31,339,942), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfzpr",	XSPR(31,339,944), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfpid",	XSPR(31,339,945), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfmmucr",	XSPR(31,339,946), XSPR_MASK, TITAN,	EXT,		{RT}},
{"mfccr0",	XSPR(31,339,947), XSPR_MASK, PPC405|TITAN, EXT,		{RT}},
{"mfiac3",	XSPR(31,339,948), XSPR_MASK, PPC405,	EXT,		{RT}},
{"mfiac4",	XSPR(31,339,949), XSPR_MASK, PPC405,	EXT,		{RT}},
{"mfdvc1",	XSPR(31,339,950), XSPR_MASK, PPC405,	EXT,		{RT}},
{"mfdvc2",	XSPR(31,339,951), XSPR_MASK, PPC405,	EXT,		{RT}},
{"mfmmcr0",	XSPR(31,339,952), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfpmc1",	XSPR(31,339,953), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfsgr",	XSPR(31,339,953), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfdcwr",	XSPR(31,339,954), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfpmc2",	XSPR(31,339,954), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfsia",	XSPR(31,339,955), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfsler",	XSPR(31,339,955), XSPR_MASK, PPC405,	EXT,		{RT}},
{"mfmmcr1",	XSPR(31,339,956), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfsu0r",	XSPR(31,339,956), XSPR_MASK, PPC405,	EXT,		{RT}},
{"mfdbcr1",	XSPR(31,339,957), XSPR_MASK, PPC405,	EXT,		{RT}},
{"mfpmc3",	XSPR(31,339,957), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfpmc4",	XSPR(31,339,958), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mficdbdr",	XSPR(31,339,979), XSPR_MASK, PPC403|TITAN, EXT,		{RT}},
{"mfesr",	XSPR(31,339,980), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfdear",	XSPR(31,339,981), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfevpr",	XSPR(31,339,982), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfcdbcr",	XSPR(31,339,983), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mftsr",	XSPR(31,339,984), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mftcr",	XSPR(31,339,986), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfpit",	XSPR(31,339,987), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mftbhi",	XSPR(31,339,988), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mftblo",	XSPR(31,339,989), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfsrr2",	XSPR(31,339,990), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfsrr3",	XSPR(31,339,991), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfdbsr",	XSPR(31,339,1008), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfhid0",	XSPR(31,339,1008), XSPR_MASK, GEKKO,	EXT,		{RT}},
{"mfhid1",	XSPR(31,339,1009), XSPR_MASK, GEKKO,	EXT,		{RT}},
{"mfdbcr0",	XSPR(31,339,1010), XSPR_MASK, PPC405,	EXT,		{RT}},
{"mfiabr",	XSPR(31,339,1010), XSPR_MASK, GEKKO,	EXT,		{RT}},
{"mfhid4",	XSPR(31,339,1011), XSPR_MASK, BROADWAY,	EXT,		{RT}},
{"mfdbdr",	XSPR(31,339,1011), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mfiac1",	XSPR(31,339,1012), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfiac2",	XSPR(31,339,1013), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfdabr",	XSPR(31,339,1013), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfdac1",	XSPR(31,339,1014), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfdac2",	XSPR(31,339,1015), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfl2cr",	XSPR(31,339,1017), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfdccr",	XSPR(31,339,1018), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mficcr",	XSPR(31,339,1019), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfictc",	XSPR(31,339,1019), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfpbl1",	XSPR(31,339,1020), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfthrm1",	XSPR(31,339,1020), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfpbu1",	XSPR(31,339,1021), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfthrm2",	XSPR(31,339,1021), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfpbl2",	XSPR(31,339,1022), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfthrm3",	XSPR(31,339,1022), XSPR_MASK, PPC750,	EXT,		{RT}},
{"mfpir",	XSPR(31,339,1023), XSPR_MASK, POWER10,	EXT,		{RT}},
{"mfpbu2",	XSPR(31,339,1023), XSPR_MASK, PPC403,	EXT,		{RT}},
{"mfspr",	X(31,339),	X_MASK,	     COM,	0,		{RT, SPR}},

{"lwax",	X(31,341),	X_MASK,	     PPC64,	0,		{RT, RA0, RB}},

{"dst",		XDSS(31,342,0),	XDSS_MASK,   PPCVEC,	0,		{RA, RB, STRM}},
{"dstt",	XDSS(31,342,1),	XDSS_MASK,   PPCVEC,	0,		{RA, RB, STRM}},

{"lhax",	X(31,343),	X_MASK,	     COM,	0,		{RT, RA0, RB}},

{"lvxl",	X(31,359),	X_MASK,	     PPCVEC,	0,		{VD, RA0, RB}},

{"abs",		XO(31,360,0,0),	XORB_MASK,   M601,	0,		{RT, RA}},
{"abs.",	XO(31,360,0,1),	XORB_MASK,   M601,	0,		{RT, RA}},

{"divs",	XO(31,363,0,0),	XO_MASK,     M601,	0,		{RT, RA, RB}},
{"divs.",	XO(31,363,0,1),	XO_MASK,     M601,	0,		{RT, RA, RB}},

{"lxvwsx",	X(31,364),	XX1_MASK,    PPCVSX3,	0,		{XT6, RA0, RB}},

{"tlbia",	X(31,370),	0xffffffff,  PPC,	E500|TITAN,	{0}},

{"mftbu",	XSPR(31,371,269), XSPR_MASK, PPC,	NO371|POWER4|EXT,	{RT}},
{"mftb",	X(31,371),	X_MASK,	     PPC,	NO371|POWER4,		{RT, TBR}},
{"mftbl",	XSPR(31,371,268), XSPR_MASK, PPC,	NO371|POWER4|EXT,	{RT}},

{"lwaux",	X(31,373),	X_MASK,	     PPC64,	0,		{RT, RAL, RB}},

{"dstst",	XDSS(31,374,0),	XDSS_MASK,   PPCVEC,	0,		{RA, RB, STRM}},
{"dststt",	XDSS(31,374,1),	XDSS_MASK,   PPCVEC,	0,		{RA, RB, STRM}},

{"lhaux",	X(31,375),	X_MASK,	     COM,	0,		{RT, RAL, RB}},

{"popcntw",	X(31,378),	XRB_MASK,    POWER7|PPCA2, 0,		{RA, RS}},

{"setbc",	X(31,384),	XRB_MASK,    POWER10,	0,		{RT, BI}},

{"mtdcrx",	X(31,387),	X_MASK,	     BOOKE|PPCA2|PPC476, TITAN,	{RA, RS}},
{"mtdcrx.",	XRC(31,387,1),	X_MASK,	     PPCA2,	0,		{RA, RS}},

{"stvexbx",	X(31,389),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"dcblc",	X(31,390),	X_MASK,	 PPCCHLK|PPC476|TITAN, 0,	{CT, RA0, RB}},
{"stdfcmx",	APU(31,391,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"divdeu",	XO(31,393,0,0),	XO_MASK,     POWER7|PPCA2, 0,		{RT, RA, RB}},
{"divdeu.",	XO(31,393,0,1),	XO_MASK,     POWER7|PPCA2, 0,		{RT, RA, RB}},
{"divweu",	XO(31,395,0,0),	XO_MASK,     POWER7|PPCA2, 0,		{RT, RA, RB}},
{"divweu.",	XO(31,395,0,1),	XO_MASK,     POWER7|PPCA2, 0,		{RT, RA, RB}},

{"stxvx",	X(31,396),	XX1_MASK,    PPCVSX3,	0,		{XS6, RA0, RB}},
{"stxvl",	X(31,397),	XX1_MASK,    PPCVSX3,	0,		{XS6, RA0, RB}},

{"dcblce",	X(31,398),	X_MASK,	     PPCCHLK,	E500MC,		{CT, RA, RB}},

{"slbmte",	X(31,402),	XRA_MASK,    PPC64,	0,		{RS, RB}},

{"mtvsrws",	X(31,403),	XX1RB_MASK,  PPCVSX3,	0,		{XT6, RA}},

{"pbt.",	XRC(31,404,1),	X_MASK,	     POWER8,	0,		{RS, RA0, RB}},

{"icswx",	XRC(31,406,0),	X_MASK,	  POWER7|PPCA2,	0,		{RS, RA, RB}},
{"icswx.",	XRC(31,406,1),	X_MASK,	  POWER7|PPCA2,	0,		{RS, RA, RB}},

{"sthx",	X(31,407),	X_MASK,	     COM,	0,		{RS, RA0, RB}},

{"orc",		XRC(31,412,0),	X_MASK,	     COM,	0,		{RA, RS, RB}},
{"orc.",	XRC(31,412,1),	X_MASK,	     COM,	0,		{RA, RS, RB}},

{"sthepx",	X(31,415),	X_MASK,	  E500MC|PPCA2, 0,		{RS, RA0, RB}},

{"setbcr",	X(31,416),	XRB_MASK,    POWER10,	0,		{RT, BI}},

{"mtdcrux",	X(31,419),	X_MASK,	 PPC464|PPC476,	0,		{RA, RS}},

{"stvexhx",	X(31,421),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"dcblq.",	XRC(31,422,1),	X_MASK,	     E6500,	0,		{CT, RA0, RB}},

{"divde",	XO(31,425,0,0),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divde.",	XO(31,425,0,1),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divwe",	XO(31,427,0,0),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divwe.",	XO(31,427,0,1),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},

{"stxvll",	X(31,429),	XX1_MASK,    PPCVSX3,	0,		{XS6, RA0, RB}},

{"clrbhrb",	X(31,430),	0xffffffff,  POWER8,	0,		{0}},

{"slbie",	X(31,434),	XRTRA_MASK,  PPC64,	0,		{RB}},

{"mtvsrdd",	X(31,435),	XX1_MASK,    PPCVSX3,	0,		{XT6, RA0, RB}},

{"ecowx",	X(31,438),	X_MASK,	     PPC,	E500|TITAN,	{RT, RA0, RB}},

{"sthux",	X(31,439),	X_MASK,	     COM,	0,		{RS, RAS, RB}},

/* or 1,1,1 */
{"cctpl",	0x7c210b78,	0xffffffff,  CELL,	EXT,		{0}},
/* or 2,2,2 */
{"cctpm",	0x7c421378,	0xffffffff,  CELL,	EXT,		{0}},
/* or 3,3,3 */
{"cctph",	0x7c631b78,	0xffffffff,  CELL,	EXT,		{0}},
/* or 26,26,26 */
{"miso",	0x7f5ad378,   0xffffffff, POWER8|E6500,	EXT,		{0}},
/* or 27,27,27 */
{"yield",	0x7f7bdb78,	0xffffffff,  POWER7,	EXT,		{0}},
/* or 28,28,28 */
{"mdors",	0x7f9ce378,	0xffffffff,  E500MC,	EXT,		{0}},
{"db8cyc",	0x7f9ce378,	0xffffffff,  CELL,	EXT,		{0}},
/* or 29,29,29 */
{"mdoio",	0x7fbdeb78,	0xffffffff,  POWER7,	EXT,		{0}},
{"db10cyc",	0x7fbdeb78,	0xffffffff,  CELL,	EXT,		{0}},
/* or 30,30,30 */
{"mdoom",	0x7fdef378,	0xffffffff,  POWER7,	EXT,		{0}},
{"db12cyc",	0x7fdef378,	0xffffffff,  CELL,	EXT,		{0}},
/* or 31,31,31 */
{"db16cyc",	0x7ffffb78,	0xffffffff,  CELL,	EXT,		{0}},

{"mr",		XRC(31,444,0),	X_MASK,	     COM,	EXT,		{RA, RSB}},
{"or",		XRC(31,444,0),	X_MASK,	     COM,	0,		{RA, RS, RB}},
{"mr.",		XRC(31,444,1),	X_MASK,	     COM,	EXT,		{RA, RSB}},
{"or.",		XRC(31,444,1),	X_MASK,	     COM,	0,		{RA, RS, RB}},

{"setnbc",	X(31,448),	XRB_MASK,    POWER10,	0,		{RT, BI}},

{"mtexisr",	XSPR(31,451, 64), XSPR_MASK, PPC403,	0,		{RS}},
{"mtexier",	XSPR(31,451, 66), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbr0",	XSPR(31,451,128), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbr1",	XSPR(31,451,129), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbr2",	XSPR(31,451,130), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbr3",	XSPR(31,451,131), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbr4",	XSPR(31,451,132), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbr5",	XSPR(31,451,133), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbr6",	XSPR(31,451,134), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbr7",	XSPR(31,451,135), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbear",	XSPR(31,451,144), XSPR_MASK, PPC403,	0,		{RS}},
{"mtbesr",	XSPR(31,451,145), XSPR_MASK, PPC403,	0,		{RS}},
{"mtiocr",	XSPR(31,451,160), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmacr0",	XSPR(31,451,192), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmact0",	XSPR(31,451,193), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmada0",	XSPR(31,451,194), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmasa0",	XSPR(31,451,195), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmacc0",	XSPR(31,451,196), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmacr1",	XSPR(31,451,200), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmact1",	XSPR(31,451,201), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmada1",	XSPR(31,451,202), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmasa1",	XSPR(31,451,203), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmacc1",	XSPR(31,451,204), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmacr2",	XSPR(31,451,208), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmact2",	XSPR(31,451,209), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmada2",	XSPR(31,451,210), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmasa2",	XSPR(31,451,211), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmacc2",	XSPR(31,451,212), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmacr3",	XSPR(31,451,216), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmact3",	XSPR(31,451,217), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmada3",	XSPR(31,451,218), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmasa3",	XSPR(31,451,219), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmacc3",	XSPR(31,451,220), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdmasr",	XSPR(31,451,224), XSPR_MASK, PPC403,	0,		{RS}},
{"mtdcr",	X(31,451), X_MASK, PPC403|BOOKE|PPCA2|PPC476, E500|TITAN, {SPR, RS}},
{"mtdcr.",	XRC(31,451,1), X_MASK,	     PPCA2,	0,		{SPR, RS}},

{"stvexwx",	X(31,453),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"dccci",	X(31,454), XRT_MASK, PPC403|PPC440|PPC476|TITAN|PPCA2, 0, {RAOPT, RBOPT}},
{"dci",		X(31,454),	XRARB_MASK, PPCA2|PPC476, 0,		{CT}},

{"divdu",	XO(31,457,0,0),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},
{"divdu.",	XO(31,457,0,1),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},

{"divwu",	XO(31,459,0,0),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"divwu.",	XO(31,459,0,1),	XO_MASK,     PPC,	0,		{RT, RA, RB}},

{"stxvpx",	X(31,461),	XX1_MASK,    POWER10,	0,		{XSP, RA0, RB}},

{"mtpmr",	X(31,462),	X_MASK, PPCPMR|PPCE300, 0,		{PMR, RS}},
{"mttmr",	X(31,494),	X_MASK,	     PPCTMR,	0,		{TMR, RS}},

{"slbieg",	X(31,466),	XRA_MASK,    POWER9,	0,		{RS, RB}},

{"mtmq",	XSPR(31,467,  0), XSPR_MASK, M601,	EXT,		{RS}},
{"mtxer",	XSPR(31,467,  1), XSPR_MASK, COM,	EXT,		{RS}},
{"mtudscr",	XSPR(31,467,  3), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtlr",	XSPR(31,467,  8), XSPR_MASK, COM,	EXT,		{RS}},
{"mtctr",	XSPR(31,467,  9), XSPR_MASK, COM,	EXT,		{RS}},
{"mtuamr",	XSPR(31,467, 13), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtdscr",	XSPR(31,467, 17), XSPR_MASK, POWER6,	EXT,		{RS}},
{"mttid",	XSPR(31,467, 17), XSPR_MASK, POWER,	EXT,		{RS}},
{"mtdsisr",	XSPR(31,467, 18), XSPR_MASK, COM,	TITAN|EXT,	{RS}},
{"mtdar",	XSPR(31,467, 19), XSPR_MASK, COM,	TITAN|EXT,	{RS}},
{"mtrtcu",	XSPR(31,467, 20), XSPR_MASK, COM,	TITAN|EXT,	{RS}},
{"mtrtcl",	XSPR(31,467, 21), XSPR_MASK, COM,	TITAN|EXT,	{RS}},
{"mtdec",	XSPR(31,467, 22), XSPR_MASK, COM,	EXT,		{RS}},
{"mtsdr0",	XSPR(31,467, 24), XSPR_MASK, POWER,	EXT,		{RS}},
{"mtsdr1",	XSPR(31,467, 25), XSPR_MASK, COM,	TITAN|EXT,	{RS}},
{"mtsrr0",	XSPR(31,467, 26), XSPR_MASK, COM,	EXT,		{RS}},
{"mtsrr1",	XSPR(31,467, 27), XSPR_MASK, COM,	EXT,		{RS}},
{"mtcfar",	XSPR(31,467, 28), XSPR_MASK, POWER6,	EXT,		{RS}},
{"mtamr",	XSPR(31,467, 29), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtpidr",	XSPR(31,467, 48), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtpid",	XSPR(31,467, 48), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtdecar",	XSPR(31,467, 54), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtcsrr0",	XSPR(31,467, 58), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtcsrr1",	XSPR(31,467, 59), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtiamr",	XSPR(31,467, 61), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdear",	XSPR(31,467, 61), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtesr",	XSPR(31,467, 62), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivpr",	XSPR(31,467, 63), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mttfhar",	XSPR(31,467,128), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mttfiar",	XSPR(31,467,129), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mttexasr",	XSPR(31,467,130), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mttexasru",	XSPR(31,467,131), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtcmpa",	XSPR(31,467,144), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtcmpb",	XSPR(31,467,145), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtcmpc",	XSPR(31,467,146), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtcmpd",	XSPR(31,467,147), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mticr",	XSPR(31,467,148), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtder",	XSPR(31,467,149), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtcounta",	XSPR(31,467,150), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtcountb",	XSPR(31,467,151), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtctrl",	XSPR(31,467,152), XSPR_MASK, POWER4,	EXT,		{RS}},
{"mtcmpe",	XSPR(31,467,152), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtfscr",	XSPR(31,467,153), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtcmpf",	XSPR(31,467,153), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtcmpg",	XSPR(31,467,154), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtcmph",	XSPR(31,467,155), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtlctrl1",	XSPR(31,467,156), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtuamor",	XSPR(31,467,157), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtlctrl2",	XSPR(31,467,157), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtictrl",	XSPR(31,467,158), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtpspb",	XSPR(31,467,159), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtbar",	XSPR(31,467,159), XSPR_MASK, PPC860,	EXT,		{RS}},
{"mtdpdes",	XSPR(31,467,176), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdawr0",	XSPR(31,467,180), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdawr1",	XSPR(31,467,181), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtrpr",	XSPR(31,467,186), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtciabr",	XSPR(31,467,187), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdawrx0",	XSPR(31,467,188), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdawrx1",	XSPR(31,467,189), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mthfscr",	XSPR(31,467,190), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtvrsave",	XSPR(31,467,256), XSPR_MASK, PPCVEC,	EXT,		{RS}},
{"mtusprg0",	XSPR(31,467,256), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtsprg",	XSPR(31,467,256), XSPRG_MASK, PPC,	EXT,		{SPRG, RS}},
{"mtsprg0",	XSPR(31,467,272), XSPR_MASK, PPC,	EXT,		{RS}},
{"mtsprg1",	XSPR(31,467,273), XSPR_MASK, PPC,	EXT,		{RS}},
{"mtsprg2",	XSPR(31,467,274), XSPR_MASK, PPC,	EXT,		{RS}},
{"mtsprg3",	XSPR(31,467,275), XSPR_MASK, PPC,	EXT,		{RS}},
{"mtsprg4",	XSPR(31,467,276), XSPR_MASK, PPC405|BOOKE, EXT,		{RS}},
{"mtsprg5",	XSPR(31,467,277), XSPR_MASK, PPC405|BOOKE, EXT,		{RS}},
{"mtsprg6",	XSPR(31,467,278), XSPR_MASK, PPC405|BOOKE, EXT,		{RS}},
{"mtsprg7",	XSPR(31,467,279), XSPR_MASK, PPC405|BOOKE, EXT,		{RS}},
{"mtasr",	XSPR(31,467,280), XSPR_MASK, PPC64,	EXT,		{RS}},
{"mtear",	XSPR(31,467,282), XSPR_MASK, PPC,	TITAN|EXT,	{RS}},
{"mttbl",	XSPR(31,467,284), XSPR_MASK, PPC,	EXT,		{RS}},
{"mttbu",	XSPR(31,467,285), XSPR_MASK, PPC,	EXT,		{RS}},
{"mttbu40",	XSPR(31,467,286), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mthsprg0",	XSPR(31,467,304), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdbsr",	XSPR(31,467,304), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mthsprg1",	XSPR(31,467,305), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mthdisr",	XSPR(31,467,306), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mthdar",	XSPR(31,467,307), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtspurr",	XSPR(31,467,308), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdbcr0",	XSPR(31,467,308), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtpurr",	XSPR(31,467,309), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdbcr1",	XSPR(31,467,309), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mthdec",	XSPR(31,467,310), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdbcr2",	XSPR(31,467,310), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtiac1",	XSPR(31,467,312), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mthrmor",	XSPR(31,467,313), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtiac2",	XSPR(31,467,313), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mthsrr0",	XSPR(31,467,314), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtiac3",	XSPR(31,467,314), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mthsrr1",	XSPR(31,467,315), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtiac4",	XSPR(31,467,315), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtdac1",	XSPR(31,467,316), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtdac2",	XSPR(31,467,317), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtlpcr",	XSPR(31,467,318), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdvc1",	XSPR(31,467,318), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtlpidr",	XSPR(31,467,319), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtdvc2",	XSPR(31,467,319), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mthmer",	XSPR(31,467,336), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mttsr",	XSPR(31,467,336), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mthmeer",	XSPR(31,467,337), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtpcr",	XSPR(31,467,338), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtheir",	XSPR(31,467,339), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mttcr",	XSPR(31,467,340), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtamor",	XSPR(31,467,349), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtivor0",	XSPR(31,467,400), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor1",	XSPR(31,467,401), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor2",	XSPR(31,467,402), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor3",	XSPR(31,467,403), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor4",	XSPR(31,467,404), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor5",	XSPR(31,467,405), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor6",	XSPR(31,467,406), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor7",	XSPR(31,467,407), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor8",	XSPR(31,467,408), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor9",	XSPR(31,467,409), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor10",	XSPR(31,467,410), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor11",	XSPR(31,467,411), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor12",	XSPR(31,467,412), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor13",	XSPR(31,467,413), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor14",	XSPR(31,467,414), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtivor15",	XSPR(31,467,415), XSPR_MASK, BOOKE,	EXT,		{RS}},
{"mtptcr",	XSPR(31,467,464), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtusprg0",	XSPR(31,467,496), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtusprg1",	XSPR(31,467,497), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mturmor",	XSPR(31,467,505), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtusrr0",	XSPR(31,467,506), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtusrr1",	XSPR(31,467,507), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtsmfctrl",	XSPR(31,467,511), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtspefscr",	XSPR(31,467,512), XSPR_MASK, PPCSPE,	EXT,		{RS}},
{"mtbbear",	XSPR(31,467,513), XSPR_MASK, PPCBRLK,	EXT,		{RS}},
{"mtbbtar",	XSPR(31,467,514), XSPR_MASK, PPCBRLK,	EXT,		{RS}},
{"mtivor32",	XSPR(31,467,528), XSPR_MASK, PPCSPE|E6500, EXT,		{RS}},
{"mtivor33",	XSPR(31,467,529), XSPR_MASK, PPCSPE|E6500, EXT,		{RS}},
{"mtivor34",	XSPR(31,467,530), XSPR_MASK, PPCSPE,	EXT,		{RS}},
{"mtivor35",	XSPR(31,467,531), XSPR_MASK, PPCPMR,	EXT,		{RS}},
{"mtibatu",	XSPR(31,467,528), XSPRBAT_MASK, PPC,	TITAN|EXT,	{SPRBAT, RS}},
{"mtibatl",	XSPR(31,467,529), XSPRBAT_MASK, PPC,	TITAN|EXT,	{SPRBAT, RS}},
{"mtdbatu",	XSPR(31,467,536), XSPRBAT_MASK, PPC,	TITAN|EXT,	{SPRBAT, RS}},
{"mtdbatl",	XSPR(31,467,537), XSPRBAT_MASK, PPC,	TITAN|EXT,	{SPRBAT, RS}},
{"mtmcsrr0",	XSPR(31,467,570), XSPR_MASK, PPCRFMCI,	EXT,		{RS}},
{"mtmcsrr1",	XSPR(31,467,571), XSPR_MASK, PPCRFMCI,	EXT,		{RS}},
{"mtmcsr",	XSPR(31,467,572), XSPR_MASK, PPCRFMCI,	EXT,		{RS}},
{"mtsier2",	XSPR(31,467,752), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtsier3",	XSPR(31,467,753), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtmmcr3",	XSPR(31,467,754), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtummcr2",	XSPR(31,467,769), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtmmcr2",	XSPR(31,467,769), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtummcra",	XSPR(31,467,770), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtupmc1",	XSPR(31,467,771), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtupmc2",	XSPR(31,467,772), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtupmc3",	XSPR(31,467,773), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtupmc4",	XSPR(31,467,774), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtupmc5",	XSPR(31,467,775), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtupmc6",	XSPR(31,467,776), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtummcr0",	XSPR(31,467,779), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtsier",	XSPR(31,467,784), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtmmcra",	XSPR(31,467,786), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtpmc1",	XSPR(31,467,787), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtpmc2",	XSPR(31,467,788), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtpmc3",	XSPR(31,467,789), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtpmc4",	XSPR(31,467,790), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtpmc5",	XSPR(31,467,791), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtpmc6",	XSPR(31,467,792), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtmmcr0",	XSPR(31,467,795), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtsiar",	XSPR(31,467,796), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtsdar",	XSPR(31,467,797), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtmmcr1",	XSPR(31,467,798), XSPR_MASK, POWER7,	EXT,		{RS}},
{"mtbescrs",	XSPR(31,467,800), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtbescrsu",	XSPR(31,467,801), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtbescrr",	XSPR(31,467,802), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtbescrru",	XSPR(31,467,803), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtebbhr",	XSPR(31,467,804), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtebbrr",	XSPR(31,467,805), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtbescr",	XSPR(31,467,806), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mttar",	XSPR(31,467,815), XSPR_MASK, POWER9,	EXT,		{RS}},
{"mtasdr",	XSPR(31,467,816), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtpsscr",	XSPR(31,467,823), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtic",	XSPR(31,467,848), XSPR_MASK, POWER8,	EXT,		{RS}},
{"mtvtb",	XSPR(31,467,849), XSPR_MASK, POWER8,	EXT,		{RS}},
{"mthpsscr",	XSPR(31,467,855), XSPR_MASK, POWER10,	EXT,		{RS}},
{"mtivndx",	XSPR(31,467,880), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mtdvndx",	XSPR(31,467,881), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mtivlim",	XSPR(31,467,882), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mtdvlim",	XSPR(31,467,883), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mtclcsr",	XSPR(31,467,884), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mtccr1",	XSPR(31,467,888), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mtppr",	XSPR(31,467,896), XSPR_MASK, POWER5,	EXT,		{RS}},
{"mtppr32",	XSPR(31,467,898), XSPR_MASK, POWER5,	EXT,		{RS}},
{"mtgqr",	XSPR(31,467,912), XSPRGQR_MASK, PPCPS,	EXT,		{SPRGQR, RS}},
{"mthid2",	XSPR(31,467,920), XSPR_MASK, GEKKO,	EXT,		{RS}},
{"mtwpar",	XSPR(31,467,921), XSPR_MASK, GEKKO,	EXT,		{RS}},
{"mtdmau",	XSPR(31,467,922), XSPR_MASK, GEKKO,	EXT,		{RS}},
{"mtdmal",	XSPR(31,467,923), XSPR_MASK, GEKKO,	EXT,		{RS}},
{"mtummcr0",	XSPR(31,467,936), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtupmc1",	XSPR(31,467,937), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtupmc2",	XSPR(31,467,938), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtusia",	XSPR(31,467,939), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtummcr1",	XSPR(31,467,940), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtupmc3",	XSPR(31,467,941), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtupmc4",	XSPR(31,467,942), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtzpr",	XSPR(31,467,944), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtpid",	XSPR(31,467,945), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtrmmucr",	XSPR(31,467,946), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mtccr0",	XSPR(31,467,947), XSPR_MASK, PPC405|TITAN, EXT,		{RS}},
{"mtiac3",	XSPR(31,467,948), XSPR_MASK, PPC405,	EXT,		{RS}},
{"mtiac4",	XSPR(31,467,949), XSPR_MASK, PPC405,	EXT,		{RS}},
{"mtdvc1",	XSPR(31,467,950), XSPR_MASK, PPC405,	EXT,		{RS}},
{"mtdvc2",	XSPR(31,467,951), XSPR_MASK, PPC405,	EXT,		{RS}},
{"mtmmcr0",	XSPR(31,467,952), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtpmc1",	XSPR(31,467,953), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtsgr",	XSPR(31,467,953), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtdcwr",	XSPR(31,467,954), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtpmc2",	XSPR(31,467,954), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtsia",	XSPR(31,467,955), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtsler",	XSPR(31,467,955), XSPR_MASK, PPC405,	EXT,		{RS}},
{"mtmmcr1",	XSPR(31,467,956), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtsu0r",	XSPR(31,467,956), XSPR_MASK, PPC405,	EXT,		{RS}},
{"mtdbcr1",	XSPR(31,467,957), XSPR_MASK, PPC405,	EXT,		{RS}},
{"mtpmc3",	XSPR(31,467,957), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtpmc4",	XSPR(31,467,958), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mticdbdr",	XSPR(31,467,979), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtesr",	XSPR(31,467,980), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtdear",	XSPR(31,467,981), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtevpr",	XSPR(31,467,982), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtcdbcr",	XSPR(31,467,983), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mttsr",	XSPR(31,467,984), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mttcr",	XSPR(31,467,986), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtpit",	XSPR(31,467,987), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mttbhi",	XSPR(31,467,988), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mttblo",	XSPR(31,467,989), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtsrr2",	XSPR(31,467,990), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtsrr3",	XSPR(31,467,991), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtdbsr",	XSPR(31,467,1008), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mthid0",	XSPR(31,467,1008), XSPR_MASK, GEKKO,	EXT,		{RS}},
{"mthid1",	XSPR(31,467,1009), XSPR_MASK, GEKKO,	EXT,		{RS}},
{"mtdbcr0",	XSPR(31,467,1010), XSPR_MASK, PPC405,	EXT,		{RS}},
{"mtiabr",	XSPR(31,467,1010), XSPR_MASK, GEKKO,	EXT,		{RS}},
{"mthid4",	XSPR(31,467,1011), XSPR_MASK, BROADWAY,	EXT,		{RS}},
{"mtdbdr",	XSPR(31,467,1011), XSPR_MASK, TITAN,	EXT,		{RS}},
{"mtiac1",	XSPR(31,467,1012), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtiac2",	XSPR(31,467,1013), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtdabr",	XSPR(31,467,1013), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtdac1",	XSPR(31,467,1014), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtdac2",	XSPR(31,467,1015), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtl2cr",	XSPR(31,467,1017), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtdccr",	XSPR(31,467,1018), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mticcr",	XSPR(31,467,1019), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtictc",	XSPR(31,467,1019), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtpbl1",	XSPR(31,467,1020), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtthrm1",	XSPR(31,467,1020), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtpbu1",	XSPR(31,467,1021), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtthrm2",	XSPR(31,467,1021), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtpbl2",	XSPR(31,467,1022), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtthrm3",	XSPR(31,467,1022), XSPR_MASK, PPC750,	EXT,		{RS}},
{"mtpbu2",	XSPR(31,467,1023), XSPR_MASK, PPC403,	EXT,		{RS}},
{"mtspr",	X(31,467),	X_MASK,	     COM,	0,		{SPR, RS}},

{"dcbi",	X(31,470),	XRT_MASK,    PPC,	0,		{RA0, RB}},

{"nand",	XRC(31,476,0),	X_MASK,	     COM,	0,		{RA, RS, RB}},
{"nand.",	XRC(31,476,1),	X_MASK,	     COM,	0,		{RA, RS, RB}},

{"setnbcr",	X(31,480),	XRB_MASK,    POWER10,	0,		{RT, BI}},

{"dsn",		X(31,483),	XRT_MASK,    E500MC,	0,		{RA, RB}},

{"dcread",	X(31,486),	X_MASK,	 PPC403|PPC440, PPCA2,		{RT, RA0, RB}},

{"icbtls",	X(31,486),	X_MASK,	 PPCCHLK|PPC476|TITAN, 0,	{CT, RA0, RB}},

{"stvxl",	X(31,487),	X_MASK,	     PPCVEC,	0,		{VS, RA0, RB}},

{"nabs",	XO(31,488,0,0),	XORB_MASK,   M601,	0,		{RT, RA}},
{"nabs.",	XO(31,488,0,1),	XORB_MASK,   M601,	0,		{RT, RA}},

{"divd",	XO(31,489,0,0),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},
{"divd.",	XO(31,489,0,1),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},

{"divw",	XO(31,491,0,0),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"divw.",	XO(31,491,0,1),	XO_MASK,     PPC,	0,		{RT, RA, RB}},

{"icbtlse",	X(31,494),	X_MASK,	     PPCCHLK,	E500MC,		{CT, RA, RB}},

{"slbia",	X(31,498),	0xff1fffff,  POWER6,	0,		{IH}},
{"slbia",	X(31,498),	0xffffffff,  PPC64,	POWER6,		{0}},

{"cli",		X(31,502),	XRB_MASK,    POWER,	0,		{RT, RA}},

{"popcntd",	X(31,506),	XRB_MASK, POWER7|PPCA2,	0,		{RA, RS}},

{"cmpb",	X(31,508),	X_MASK, POWER6|PPCA2|PPC476, 0,		{RA, RS, RB}},

{"mcrxr",	X(31,512),	XBFRARB_MASK, COM,	POWER7,		{BF}},

{"lbdcbx",	X(31,514),	X_MASK,      E200Z4,	0,		{RT, RA, RB}},
{"lbdx",	X(31,515),	X_MASK,	 E500MC|E200Z4,	0,		{RT, RA, RB}},

{"bblels",	X(31,518),	X_MASK,	     PPCBRLK,	0,		{0}},

{"lvlx",	X(31,519),	X_MASK,	     CELL,	0,		{VD, RA0, RB}},
{"lbfcmux",	APU(31,519,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"subfco",	XO(31,8,1,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"sfo",		XO(31,8,1,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"subco",	XO(31,8,1,0),	XO_MASK,     PPCCOM,	EXT,		{RT, RB, RA}},
{"subfco.",	XO(31,8,1,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"sfo.",	XO(31,8,1,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"subco.",	XO(31,8,1,1),	XO_MASK,     PPCCOM,	EXT,		{RT, RB, RA}},

{"addco",	XO(31,10,1,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"ao",		XO(31,10,1,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"addco.",	XO(31,10,1,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"ao.",		XO(31,10,1,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"lxsspx",	X(31,524),	XX1_MASK,    PPCVSX2,	0,		{XT6, RA0, RB}},
{"lxvrl",	X(31,525),	XX1_MASK,    PPCVSXF,	0,		{XT6, RA0, RB}},

{"clcs",	X(31,531),	XRB_MASK,    M601,	0,		{RT, RA}},

{"ldbrx",	X(31,532),	X_MASK, CELL|POWER7|PPCA2, 0,		{RT, RA0, RB}},

{"lswx",	X(31,533),	X_MASK,	     PPCCOM,	E500|E500MC,	{RT, RAX, RBX}},
{"lsx",		X(31,533),	X_MASK,	     PWRCOM,	0,		{RT, RA, RB}},

{"lwbrx",	X(31,534),	X_MASK,	     PPCCOM,	0,		{RT, RA0, RB}},
{"lbrx",	X(31,534),	X_MASK,	     PWRCOM,	0,		{RT, RA, RB}},

{"lfsx",	X(31,535),	X_MASK,	     COM,	PPCEFS,		{FRT, RA0, RB}},

{"srw",		XRC(31,536,0),	X_MASK,	     PPCCOM,	0,		{RA, RS, RB}},
{"sr",		XRC(31,536,0),	X_MASK,	     PWRCOM,	0,		{RA, RS, RB}},
{"srw.",	XRC(31,536,1),	X_MASK,	     PPCCOM,	0,		{RA, RS, RB}},
{"sr.",		XRC(31,536,1),	X_MASK,	     PWRCOM,	0,		{RA, RS, RB}},

{"rrib",	XRC(31,537,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"rrib.",	XRC(31,537,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"cnttzw",	XRC(31,538,0),	XRB_MASK,    POWER9,	0,		{RA, RS}},
{"cnttzw.",	XRC(31,538,1),	XRB_MASK,    POWER9,	0,		{RA, RS}},

{"srd",		XRC(31,539,0),	X_MASK,	     PPC64,	0,		{RA, RS, RB}},
{"srd.",	XRC(31,539,1),	X_MASK,	     PPC64,	0,		{RA, RS, RB}},

{"maskir",	XRC(31,541,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"maskir.",	XRC(31,541,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"lhdcbx",	X(31,546),	X_MASK,      E200Z4,	0,		{RT, RA, RB}},
{"lhdx",	X(31,547),	X_MASK,	 E500MC|E200Z4,	0,		{RT, RA, RB}},

{"lvtrx",	X(31,549),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"bbelr",	X(31,550),	X_MASK,	     PPCBRLK,	0,		{0}},

{"lvrx",	X(31,551),	X_MASK,	     CELL,	0,		{VD, RA0, RB}},
{"lhfcmux",	APU(31,551,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"subfo",	XO(31,40,1,0),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"subo",	XO(31,40,1,0),	XO_MASK,     PPC,	EXT,		{RT, RB, RA}},
{"subfo.",	XO(31,40,1,1),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"subo.",	XO(31,40,1,1),	XO_MASK,     PPC,	EXT,		{RT, RB, RA}},

{"lxvrll",	X(31,557),	XX1_MASK,    PPCVSXF,	0,		{XT6, RA0, RB}},

{"tlbsync",	X(31,566),	0xffffffff,  PPC,	0,		{0}},

{"lfsux",	X(31,567),	X_MASK,	     COM,	PPCEFS,		{FRT, RAS, RB}},

{"cnttzd",	XRC(31,570,0),	XRB_MASK,    POWER9,	0,		{RA, RS}},
{"cnttzd.",	XRC(31,570,1),	XRB_MASK,    POWER9,	0,		{RA, RS}},

{"cnttzdm",	X(31,571),	X_MASK,	     POWER10,	0,		{RA, RS, RB}},

{"mcrxrx",	X(31,576),     XBFRARB_MASK, POWER9,	0,		{BF}},

{"lwdcbx",	X(31,578),	X_MASK,      E200Z4,	0,		{RT, RA, RB}},
{"lwdx",	X(31,579),	X_MASK,	 E500MC|E200Z4,	0,		{RT, RA, RB}},

{"lvtlx",	X(31,581),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"lwat",	X(31,582),	X_MASK,	     POWER9,	0,		{RT, RA0, FC}},

{"lwfcmux",	APU(31,583,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"lxsdx",	X(31,588),	XX1_MASK,    PPCVSX,	0,		{XT6, RA0, RB}},
{"lxvprl",	X(31,589),	XX1_MASK,    PPCVSXF,	0,		{XTP, RA0, RB}},

{"mfsr",	X(31,595), XRB_MASK|(1<<20), COM,	NON32,		{RT, SR}},

{"lswi",	X(31,597),	X_MASK,	     PPCCOM,	E500|E500MC,	{RT, RAX, NBI}},
{"lsi",		X(31,597),	X_MASK,	     PWRCOM,	0,		{RT, RA0, NB}},

{"hwsync",	XSYNC(31,598,0), 0xffffffff, POWER4,	BOOKE|PPC476|EXT,	{0}},
{"lwsync",	XSYNC(31,598,1), 0xffffffff, PPC,	E500|EXT,		{0}},
{"ptesync",	XSYNC(31,598,2), 0xffffffff, PPC64,	EXT,			{0}},
{"phwsync",	XSYNCLS(31,598,4,0), 0xffffffff, POWER10, EXT,			{0}},
{"plwsync",	XSYNCLS(31,598,5,0), 0xffffffff, POWER10, EXT,			{0}},
{"stncisync",	XSYNCLS(31,598,1,1), 0xffffffff, POWER10, EXT,			{0}},
{"stcisync",	XSYNCLS(31,598,0,2), 0xffffffff, POWER10, EXT,			{0}},
{"stsync",	XSYNCLS(31,598,0,3), 0xffffffff, POWER10, EXT,			{0}},
{"sync",	X(31,598),     XSYNCLS_MASK, POWER10,	BOOKE|PPC476,		{LS3, SC2}},
{"sync",	X(31,598),     XSYNCLE_MASK, E6500,	0,			{LS, ESYNC}},
{"sync",	X(31,598),     XSYNC_MASK,   PPCCOM,	POWER10|BOOKE|PPC476,	{LS}},
{"msync",	X(31,598),     0xffffffff, BOOKE|PPCA2|PPC476, 0,		{0}},
{"sync",	X(31,598),     0xffffffff,   BOOKE|PPC476, E6500,		{0}},
{"lwsync",	X(31,598),     0xffffffff,   E500,	0,			{0}},
{"dcs",		X(31,598),     0xffffffff,   PWRCOM,	0,			{0}},

{"lfdx",	X(31,599),	X_MASK,	     COM,	PPCEFS,		{FRT, RA0, RB}},

{"mffgpr",	XRC(31,607,0),	XRA_MASK,    POWER6,	POWER7,		{FRT, RB}},
{"lfdepx",	X(31,607),	X_MASK,	  E500MC|PPCA2, 0,		{FRT, RA0, RB}},

{"lddx",	X(31,611),	X_MASK,	     E500MC,	0,		{RT, RA, RB}},

{"lvswx",	X(31,613),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"ldat",	X(31,614),	X_MASK,	     POWER9,	0,		{RT, RA0, FC}},

{"lqfcmux",	APU(31,615,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"nego",	XO(31,104,1,0),	XORB_MASK,   COM,	0,		{RT, RA}},
{"nego.",	XO(31,104,1,1),	XORB_MASK,   COM,	0,		{RT, RA}},

{"mulo",	XO(31,107,1,0),	XO_MASK,     M601,	0,		{RT, RA, RB}},
{"mulo.",	XO(31,107,1,1),	XO_MASK,     M601,	0,		{RT, RA, RB}},

{"lxvprll",	X(31,621),	XX1_MASK,    PPCVSXF,	0,		{XTP, RA0, RB}},

{"mfsri",	X(31,627),	X_MASK,	     M601,	0,		{RT, RA, RB}},

{"dclst",	X(31,630),	XRB_MASK,    M601,	0,		{RS, RA}},

{"lfdux",	X(31,631),	X_MASK,	     COM,	PPCEFS,		{FRT, RAS, RB}},

{"stbdcbx",	X(31,642),	X_MASK,      E200Z4,	0,		{RS, RA, RB}},
{"stbdx",	X(31,643),	X_MASK,	 E500MC|E200Z4,	0,		{RS, RA, RB}},

{"stvlx",	X(31,647),	X_MASK,	     CELL,	0,		{VS, RA0, RB}},
{"stbfcmux",	APU(31,647,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"stxsspx",	X(31,652),	XX1_MASK,    PPCVSX2,	0,		{XS6, RA0, RB}},
{"stxvrl",	X(31,653),	XX1_MASK,    PPCVSXF,	0,		{XS6, RA0, RB}},

{"tbegin.",	XRC(31,654,1), XRTLRARB_MASK, PPCHTM,	0,		{HTM_R}},

{"subfeo",	XO(31,136,1,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"sfeo",	XO(31,136,1,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"subfeo.",	XO(31,136,1,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"sfeo.",	XO(31,136,1,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"addeo",	XO(31,138,1,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"aeo",		XO(31,138,1,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"addeo.",	XO(31,138,1,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"aeo.",	XO(31,138,1,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"hashstp",	X(31,658),	XRC_MASK,    POWER8,	0,		{RB, DW, RA0}},

{"mfsrin",	X(31,659),	XRA_MASK,    PPC,	NON32,		{RT, RB}},

{"stdbrx",	X(31,660),	X_MASK, CELL|POWER7|PPCA2, 0,		{RS, RA0, RB}},

{"stswx",	X(31,661),	X_MASK,	     PPCCOM,	E500|E500MC,	{RS, RA0, RB}},
{"stsx",	X(31,661),	X_MASK,	     PWRCOM,	0,		{RS, RA0, RB}},

{"stwbrx",	X(31,662),	X_MASK,	     PPCCOM,	0,		{RS, RA0, RB}},
{"stbrx",	X(31,662),	X_MASK,	     PWRCOM,	0,		{RS, RA0, RB}},

{"stfsx",	X(31,663),	X_MASK,	     COM,	PPCEFS,		{FRS, RA0, RB}},

{"srq",		XRC(31,664,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"srq.",	XRC(31,664,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"sre",		XRC(31,665,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"sre.",	XRC(31,665,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"sthdcbx",	X(31,674),	X_MASK,      E200Z4,	0,		{RS, RA, RB}},
{"sthdx",	X(31,675),	X_MASK,	 E500MC|E200Z4,	0,		{RS, RA, RB}},

{"stvfrx",	X(31,677),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"stvrx",	X(31,679),	X_MASK,	     CELL,	0,		{VS, RA0, RB}},
{"sthfcmux",	APU(31,679,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"stxvrll",	X(31,685),	XX1_MASK,    PPCVSXF,	0,		{XS6, RA0, RB}},

{"tendall.",	XRC(31,686,1)|(1<<25), XRTRARB_MASK, PPCHTM, 0,		{0}},
{"tend.",	XRC(31,686,1), XRTARARB_MASK, PPCHTM,	0,		{HTM_A}},

{"hashchkp",	X(31,690),	XRC_MASK,    POWER8,	0,		{RB, DW, RA0}},

{"stbcx.",	XRC(31,694,1),	X_MASK,	  POWER8|E6500, 0,		{RS, RA0, RB}},

{"stfsux",	X(31,695),	X_MASK,	     COM,	PPCEFS,		{FRS, RAS, RB}},

{"sriq",	XRC(31,696,0),	X_MASK,	     M601,	0,		{RA, RS, SH}},
{"sriq.",	XRC(31,696,1),	X_MASK,	     M601,	0,		{RA, RS, SH}},

{"stwdcbx",	X(31,706),	X_MASK,	     E200Z4,	0,		{RS, RA, RB}},
{"stwdx",	X(31,707),	X_MASK,	 E500MC|E200Z4,	0,		{RS, RA, RB}},

{"stvflx",	X(31,709),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"stwat",	X(31,710),	X_MASK,	     POWER9,	0,		{RS, RA0, FC}},

{"stwfcmux",	APU(31,711,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"stxsdx",	X(31,716),	XX1_MASK,    PPCVSX,	0,		{XS6, RA0, RB}},
{"stxvprl",	X(31,717),	XX1_MASK,    PPCVSXF,	0,		{XSP, RA0, RB}},

{"tcheck",	X(31,718),   XRTBFRARB_MASK, PPCHTM,	0,		{BF}},

{"subfzeo",	XO(31,200,1,0),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"sfzeo",	XO(31,200,1,0),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},
{"subfzeo.",	XO(31,200,1,1),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"sfzeo.",	XO(31,200,1,1),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},

{"addzeo",	XO(31,202,1,0),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"azeo",	XO(31,202,1,0),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},
{"addzeo.",	XO(31,202,1,1),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"azeo.",	XO(31,202,1,1),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},

{"hashst",	X(31,722),	XRC_MASK,    POWER8,	0,		{RB, DW, RA0}},

{"stswi",	X(31,725),	X_MASK,	     PPCCOM,	E500|E500MC,	{RS, RA0, NB}},
{"stsi",	X(31,725),	X_MASK,	     PWRCOM,	0,		{RS, RA0, NB}},

{"sthcx.",	XRC(31,726,1),	X_MASK,	  POWER8|E6500, 0,		{RS, RA0, RB}},

{"stfdx",	X(31,727),	X_MASK,	     COM,	PPCEFS,		{FRS, RA0, RB}},

{"srlq",	XRC(31,728,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"srlq.",	XRC(31,728,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"sreq",	XRC(31,729,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"sreq.",	XRC(31,729,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"mftgpr",	XRC(31,735,0),	XRA_MASK,    POWER6,	POWER7,		{RT, FRB}},
{"stfdepx",	X(31,735),	X_MASK,	  E500MC|PPCA2, 0,		{FRS, RA0, RB}},

{"stddx",	X(31,739),	X_MASK,	     E500MC,	0,		{RS, RA, RB}},

{"stvswx",	X(31,741),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"stdat",	X(31,742),	X_MASK,	     POWER9,	0,		{RS, RA0, FC}},

{"stqfcmux",	APU(31,743,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"subfmeo",	XO(31,232,1,0),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"sfmeo",	XO(31,232,1,0),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},
{"subfmeo.",	XO(31,232,1,1),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"sfmeo.",	XO(31,232,1,1),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},

{"mulldo",	XO(31,233,1,0),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},
{"mulldo.",	XO(31,233,1,1),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},

{"addmeo",	XO(31,234,1,0),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"ameo",	XO(31,234,1,0),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},
{"addmeo.",	XO(31,234,1,1),	XORB_MASK,   PPCCOM,	0,		{RT, RA}},
{"ameo.",	XO(31,234,1,1),	XORB_MASK,   PWRCOM,	0,		{RT, RA}},

{"mullwo",	XO(31,235,1,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"mulso",	XO(31,235,1,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"mullwo.",	XO(31,235,1,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"mulso.",	XO(31,235,1,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"stxvprll",	X(31,749),	XX1_MASK,    PPCVSXF,	0,		{XSP, RA0, RB}},

{"tsuspend.",	XRCL(31,750,0,1), XRTRARB_MASK,PPCHTM,	EXT,		{0}},
{"tresume.",	XRCL(31,750,1,1), XRTRARB_MASK,PPCHTM,	EXT,		{0}},
{"tsr.",	XRC(31,750,1),	  XRTLRARB_MASK,PPCHTM,	0,		{L}},

{"hashchk",	X(31,754),	XRC_MASK,    POWER8,	0,		{RB, DW, RA0}},

{"darn",	X(31,755),	XLRAND_MASK, POWER9,	0,		{RT, LRAND}},

{"dcba",	X(31,758), XRT_MASK, PPC405|PPC7450|BOOKE|PPCA2|PPC476, 0, {RA0, RB}},
{"dcbal",	XOPL(31,758,1), XRT_MASK,    E500MC,	0,		{RA0, RB}},

{"stfdux",	X(31,759),	X_MASK,	     COM,	PPCEFS,		{FRS, RAS, RB}},

{"srliq",	XRC(31,760,0),	X_MASK,	     M601,	0,		{RA, RS, SH}},
{"srliq.",	XRC(31,760,1),	X_MASK,	     M601,	0,		{RA, RS, SH}},

{"lvsm",	X(31,773),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"copy",	XOPL(31,774,1),	XRT_MASK,    POWER9,	0,		{RA0, RB}},

{"stvepxl",	X(31,775),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},
{"lvlxl",	X(31,775),	X_MASK,	     CELL,	0,		{VD, RA0, RB}},
{"ldfcmux",	APU(31,775,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"dozo",	XO(31,264,1,0),	XO_MASK,     M601,	0,		{RT, RA, RB}},
{"dozo.",	XO(31,264,1,1),	XO_MASK,     M601,	0,		{RT, RA, RB}},

{"addo",	XO(31,266,1,0),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"caxo",	XO(31,266,1,0),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},
{"addo.",	XO(31,266,1,1),	XO_MASK,     PPCCOM,	0,		{RT, RA, RB}},
{"caxo.",	XO(31,266,1,1),	XO_MASK,     PWRCOM,	0,		{RT, RA, RB}},

{"modsd",	X(31,777),	X_MASK,	     POWER9,	0,		{RT, RA, RB}},
{"modsw",	X(31,779),	X_MASK,	     POWER9,	0,		{RT, RA, RB}},

{"lxvw4x",	X(31,780),	XX1_MASK,    PPCVSX,	0,		{XT6, RA0, RB}},
{"lxsibzx",	X(31,781),	XX1_MASK,    PPCVSX3,	0,		{XT6, RA0, RB}},

{"tabortwc.",	XRC(31,782,1),	X_MASK,	     PPCHTM,	0,		{TO, RA, RB}},

{"tlbivax",	X(31,786),	XRT_MASK, BOOKE|PPCA2|PPC476, 0,	{RA0, RB}},

{"lwzcix",	X(31,789),	X_MASK,	     POWER6,	0,		{RT, RA0, RB}},

{"lhbrx",	X(31,790),	X_MASK,	     COM,	0,		{RT, RA0, RB}},

{"lfdpx",	X(31,791),    X_MASK|Q_MASK, POWER6,	POWER7,		{FRTp, RA0, RB}},
{"lfqx",	X(31,791),	X_MASK,	     POWER2,	0,		{FRT, RA, RB}},

{"sraw",	XRC(31,792,0),	X_MASK,	     PPCCOM,	0,		{RA, RS, RB}},
{"sra",		XRC(31,792,0),	X_MASK,	     PWRCOM,	0,		{RA, RS, RB}},
{"sraw.",	XRC(31,792,1),	X_MASK,	     PPCCOM,	0,		{RA, RS, RB}},
{"sra.",	XRC(31,792,1),	X_MASK,	     PWRCOM,	0,		{RA, RS, RB}},

{"srad",	XRC(31,794,0),	X_MASK,	     PPC64,	0,		{RA, RS, RB}},
{"srad.",	XRC(31,794,1),	X_MASK,	     PPC64,	0,		{RA, RS, RB}},

{"evlddepx",    VX (31, 1598),	VX_MASK,     PPCSPE,	0,		{RT, RA, RB}},
{"lfddx",	X(31,803),	X_MASK,	     E500MC,	0,		{FRT, RA, RB}},

{"lvtrxl",	X(31,805),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},
{"stvepx",	X(31,807),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},
{"lvrxl",	X(31,807),	X_MASK,	     CELL,	0,		{VD, RA0, RB}},

{"lxvh8x",	X(31,812),	XX1_MASK,    PPCVSX3,	0,		{XT6, RA0, RB}},
{"lxsihzx",	X(31,813),	XX1_MASK,    PPCVSX3,	0,		{XT6, RA0, RB}},

{"tabortdc.",	XRC(31,814,1),	X_MASK,	     PPCHTM,	0,		{TO, RA, RB}},

{"rac",		X(31,818),	X_MASK,	     M601,	0,		{RT, RA, RB}},

{"erativax",	X(31,819),	X_MASK,	     PPCA2,	0,		{RS, RA0, RB}},

{"lhzcix",	X(31,821),	X_MASK,	     POWER6,	0,		{RT, RA0, RB}},

{"dss",		XDSS(31,822,0),	XDSS_MASK,   PPCVEC,	0,		{STRM}},
{"dssall",	XDSS(31,822,1),	XDSS_MASK,   PPCVEC,	0,		{0}},

{"lfqux",	X(31,823),	X_MASK,	     POWER2,	0,		{FRT, RA, RB}},

{"srawi",	XRC(31,824,0),	X_MASK,	     PPCCOM,	0,		{RA, RS, SH}},
{"srai",	XRC(31,824,0),	X_MASK,	     PWRCOM,	0,		{RA, RS, SH}},
{"srawi.",	XRC(31,824,1),	X_MASK,	     PPCCOM,	0,		{RA, RS, SH}},
{"srai.",	XRC(31,824,1),	X_MASK,	     PWRCOM,	0,		{RA, RS, SH}},

{"sradi",	XS(31,413,0),	XS_MASK,     PPC64,	0,		{RA, RS, SH6}},
{"sradi.",	XS(31,413,1),	XS_MASK,     PPC64,	0,		{RA, RS, SH6}},

{"lvtlxl",	X(31,837),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"cpabort",	X(31,838),	XRTRARB_MASK,POWER9,	0,		{0}},

{"divo",	XO(31,331,1,0),	XO_MASK,     M601,	0,		{RT, RA, RB}},
{"divo.",	XO(31,331,1,1),	XO_MASK,     M601,	0,		{RT, RA, RB}},

{"lxvd2x",	X(31,844),	XX1_MASK,    PPCVSX,	0,		{XT6, RA0, RB}},
{"lxvx",	X(31,844),	XX1_MASK,    POWER8,	POWER9|PPCVSX3,	{XT6, RA0, RB}},

{"tabortwci.",	XRC(31,846,1),	X_MASK,	     PPCHTM,	0,		{TO, RA, HTM_SI}},

{"tlbsrx.",	XRC(31,850,1),	XRT_MASK,    PPCA2,	0,		{RA0, RB}},

{"slbiag",	X(31,850),	XRLARB_MASK, POWER10,	0,		{RS, A_L}},
{"slbiag",	X(31,850),	XRARB_MASK,  POWER9,	POWER10,	{RS}},

{"slbmfev",	X(31,851),	XRLA_MASK,   POWER9,	0,		{RT, RB, A_L}},
{"slbmfev",	X(31,851),	XRA_MASK,    PPC64,	POWER9,		{RT, RB}},

{"lbzcix",	X(31,853),	X_MASK,	     POWER6,	0,		{RT, RA0, RB}},

{"eieio",	X(31,854),	0xffffffff,  PPC,   BOOKE|PPCA2|PPC476,	{0}},
{"mbar",	X(31,854),	X_MASK,	   BOOKE|PPCA2|PPC476, 0,	{MO}},
{"eieio",	XMBAR(31,854,1),0xffffffff,  E500,	0,		{0}},
{"eieio",	X(31,854),	0xffffffff, PPCA2|PPC476, 0,		{0}},

{"lfiwax",	X(31,855),	X_MASK, POWER6|PPCA2|PPC476, 0,		{FRT, RA0, RB}},

{"lvswxl",	X(31,869),	X_MASK,	     E6500,	0,		{VD, RA0, RB}},

{"abso",	XO(31,360,1,0),	XORB_MASK,   M601,	0,		{RT, RA}},
{"abso.",	XO(31,360,1,1),	XORB_MASK,   M601,	0,		{RT, RA}},

{"divso",	XO(31,363,1,0),	XO_MASK,     M601,	0,		{RT, RA, RB}},
{"divso.",	XO(31,363,1,1),	XO_MASK,     M601,	0,		{RT, RA, RB}},

{"lxvb16x",	X(31,876),	XX1_MASK,    PPCVSX3,	0,		{XT6, RA0, RB}},

{"tabortdci.",	XRC(31,878,1),	X_MASK,	     PPCHTM,	0,		{TO, RA, HTM_SI}},

{"rmieg",	X(31,882),	XRTRA_MASK,  POWER9,	0,		{RB}},

{"ldcix",	X(31,885),	X_MASK,	     POWER6,	0,		{RT, RA0, RB}},

{"msgsync",	X(31,886),	0xffffffff,  POWER9,	0,		{0}},

{"lfiwzx",	X(31,887),	X_MASK,	  POWER7|PPCA2,	0,		{FRT, RA0, RB}},

{"extswsli",	XS(31,445,0),	XS_MASK,     POWER9,	0,		{RA, RS, SH6}},
{"extswsli.",	XS(31,445,1),	XS_MASK,     POWER9,	0,		{RA, RS, SH6}},

{"paste.",	XRC(31,902,1),	XLRT_MASK,   POWER10,	0,		{RA0, RB, L1OPT}},
{"paste.",	XRCL(31,902,1,1),XRT_MASK,   POWER9,	POWER10,	{RA0, RB}},

{"stvlxl",	X(31,903),	X_MASK,	     CELL,	0,		{VS, RA0, RB}},
{"stdfcmux",	APU(31,903,0),	APU_MASK,    PPC405,	0,		{FCRT, RA, RB}},

{"divdeuo",	XO(31,393,1,0),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divdeuo.",	XO(31,393,1,1),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divweuo",	XO(31,395,1,0),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divweuo.",	XO(31,395,1,1),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},

{"stxvw4x",	X(31,908),	XX1_MASK,    PPCVSX,	0,		{XS6, RA0, RB}},
{"stxsibx",	X(31,909),	XX1_MASK,    PPCVSX3,	0,		{XS6, RA0, RB}},

{"tabort.",	XRC(31,910,1),	XRTRB_MASK,  PPCHTM,	0,		{RA}},

{"tlbsx",	XRC(31,914,0),	X_MASK, PPC403|BOOKE|PPCA2|PPC476, 0,	{RTO, RA0, RB}},
{"tlbsx.",	XRC(31,914,1),	X_MASK, PPC403|BOOKE|PPCA2|PPC476, 0,	{RTO, RA0, RB}},

{"slbmfee",	X(31,915),	XRLA_MASK,   POWER9,	0,		{RT, RB, A_L}},
{"slbmfee",	X(31,915),	XRA_MASK,    PPC64,	POWER9,		{RT, RB}},

{"stwcix",	X(31,917),	X_MASK,	     POWER6,	0,		{RS, RA0, RB}},

{"sthbrx",	X(31,918),	X_MASK,	     COM,	0,		{RS, RA0, RB}},

{"stfdpx",	X(31,919),    X_MASK|Q_MASK, POWER6,	POWER7,		{FRSp, RA0, RB}},
{"stfqx",	X(31,919),	X_MASK,	     POWER2,	0,		{FRS, RA0, RB}},

{"sraq",	XRC(31,920,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"sraq.",	XRC(31,920,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"srea",	XRC(31,921,0),	X_MASK,	     M601,	0,		{RA, RS, RB}},
{"srea.",	XRC(31,921,1),	X_MASK,	     M601,	0,		{RA, RS, RB}},

{"extsh",	XRC(31,922,0),	XRB_MASK,    PPCCOM,	0,		{RA, RS}},
{"exts",	XRC(31,922,0),	XRB_MASK,    PWRCOM,	0,		{RA, RS}},
{"extsh.",	XRC(31,922,1),	XRB_MASK,    PPCCOM,	0,		{RA, RS}},
{"exts.",	XRC(31,922,1),	XRB_MASK,    PWRCOM,	0,		{RA, RS}},

{"evstddepx",	VX (31, 1854),	VX_MASK,     PPCSPE,	0,		{RT, RA, RB}},
{"stfddx",	X(31,931),	X_MASK,	     E500MC,	0,		{FRS, RA, RB}},

{"stvfrxl",	X(31,933),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"wclrone",	XOPL2(31,934,2),XRT_MASK,    PPCA2,	EXT,		{RA0, RB}},
{"wclrall",	X(31,934),	XRARB_MASK,  PPCA2,	EXT,		{L2}},
{"wclr",	X(31,934),	X_MASK,	     PPCA2,	0,		{L2, RA0, RB}},

{"stvrxl",	X(31,935),	X_MASK,	     CELL,	0,		{VS, RA0, RB}},

{"divdeo",	XO(31,425,1,0),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divdeo.",	XO(31,425,1,1),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divweo",	XO(31,427,1,0),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},
{"divweo.",	XO(31,427,1,1),	XO_MASK,  POWER7|PPCA2,	0,		{RT, RA, RB}},

{"stxvh8x",	X(31,940),	XX1_MASK,    PPCVSX3,	0,		{XS6, RA0, RB}},
{"stxsihx",	X(31,941),	XX1_MASK,    PPCVSX3,	0,		{XS6, RA0, RB}},

{"treclaim.",	XRC(31,942,1),	XRTRB_MASK,  PPCHTM,	0,		{RA}},

{"tlbrehi",	XTLB(31,946,0),	XTLB_MASK,   PPC403,	PPCA2|EXT,	{RT, RA}},
{"tlbrelo",	XTLB(31,946,1),	XTLB_MASK,   PPC403,	PPCA2|EXT,	{RT, RA}},
{"tlbre",	X(31,946),  X_MASK, PPC403|BOOKE|PPCA2|PPC476, 0,	{RSO, RAOPT, SHO}},

{"sthcix",	X(31,949),	X_MASK,	     POWER6,	0,		{RS, RA0, RB}},

{"icswepx",	XRC(31,950,0),	X_MASK,	     PPCA2,	0,		{RS, RA, RB}},
{"icswepx.",	XRC(31,950,1),	X_MASK,	     PPCA2,	0,		{RS, RA, RB}},

{"stfqux",	X(31,951),	X_MASK,	     POWER2,	0,		{FRS, RA, RB}},

{"sraiq",	XRC(31,952,0),	X_MASK,	     M601,	0,		{RA, RS, SH}},
{"sraiq.",	XRC(31,952,1),	X_MASK,	     M601,	0,		{RA, RS, SH}},

{"extsb",	XRC(31,954,0),	XRB_MASK,    PPC,	0,		{RA, RS}},
{"extsb.",	XRC(31,954,1),	XRB_MASK,    PPC,	0,		{RA, RS}},

{"stvflxl",	X(31,965),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"iccci",	X(31,966), XRT_MASK, PPC403|PPC440|PPC476|TITAN|PPCA2, 0, {RAOPT, RBOPT}},
{"ici",		X(31,966),	XRARB_MASK,  PPCA2|PPC476, 0,		{CT}},

{"divduo",	XO(31,457,1,0),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},
{"divduo.",	XO(31,457,1,1),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},

{"divwuo",	XO(31,459,1,0),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"divwuo.",	XO(31,459,1,1),	XO_MASK,     PPC,	0,		{RT, RA, RB}},

{"stxvd2x",	X(31,972),	XX1_MASK,    PPCVSX,	0,		{XS6, RA0, RB}},
{"stxvx",	X(31,972),	XX1_MASK,    POWER8,	POWER9|PPCVSX3,	{XS6, RA0, RB}},

{"tlbld",	X(31,978),	XRTRA_MASK,  PPC, PPC403|BOOKE|PPCA2|PPC476, {RB}},
{"tlbwehi",	XTLB(31,978,0),	XTLB_MASK,   PPC403,	EXT,		{RT, RA}},
{"tlbwelo",	XTLB(31,978,1),	XTLB_MASK,   PPC403,	EXT,		{RT, RA}},
{"tlbwe",	X(31,978),  X_MASK, PPC403|BOOKE|PPCA2|PPC476, 0,	{RSO, RAOPT, SHO}},

{"slbfee.",	XRC(31,979,1),	XRA_MASK,    POWER6,	0,		{RT, RB}},

{"stbcix",	X(31,981),	X_MASK,	     POWER6,	0,		{RS, RA0, RB}},

{"icbi",	X(31,982),	XRT_MASK,    PPC,	0,		{RA0, RB}},

{"stfiwx",	X(31,983),	X_MASK,	     PPC,	PPCEFS,		{FRS, RA0, RB}},

{"extsw",	XRC(31,986,0),	XRB_MASK,    PPC64,	0,		{RA, RS}},
{"extsw.",	XRC(31,986,1),	XRB_MASK,    PPC64,	0,		{RA, RS}},

{"icbiep",	XRT(31,991,0),	XRT_MASK,    E500MC|PPCA2, 0,		{RA0, RB}},

{"stvswxl",	X(31,997),	X_MASK,	     E6500,	0,		{VS, RA0, RB}},

{"icread",	X(31,998),     XRT_MASK, PPC403|PPC440|PPC476|TITAN, 0,	{RA0, RB}},

{"nabso",	XO(31,488,1,0),	XORB_MASK,   M601,	0,		{RT, RA}},
{"nabso.",	XO(31,488,1,1),	XORB_MASK,   M601,	0,		{RT, RA}},

{"divdo",	XO(31,489,1,0),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},
{"divdo.",	XO(31,489,1,1),	XO_MASK,     PPC64,	0,		{RT, RA, RB}},

{"divwo",	XO(31,491,1,0),	XO_MASK,     PPC,	0,		{RT, RA, RB}},
{"divwo.",	XO(31,491,1,1),	XO_MASK,     PPC,	0,		{RT, RA, RB}},

{"stxvb16x",	X(31,1004),	XX1_MASK,    PPCVSX3,	0,		{XS6, RA0, RB}},

{"trechkpt.",	XRC(31,1006,1),	XRTRARB_MASK,PPCHTM,	0,		{0}},

{"tlbli",	X(31,1010),	XRTRA_MASK,  PPC,	TITAN,		{RB}},

{"stdcix",	X(31,1013),	X_MASK,	     POWER6,	0,		{RS, RA0, RB}},

{"dcbz",	X(31,1014),	XRT_MASK,    PPC,	0,		{RA0, RB}},
{"dclz",	X(31,1014),	XRT_MASK,    PPC,	0,		{RA0, RB}},
{"dcbzl",	XOPL(31,1014,1), XRT_MASK,   POWER4|E500MC, PPC476,	{RA0, RB}},

{"dcbzep",	XRT(31,1023,0),	XRT_MASK,    E500MC|PPCA2, 0,		{RA0, RB}},

{"lwz",		OP(32),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, D, RA0}},
{"l",		OP(32),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, D, RA0}},

{"lwzu",	OP(33),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, D, RAL}},
{"lu",		OP(33),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, D, RA0}},

{"lbz",		OP(34),		OP_MASK,     COM,	PPCVLE,		{RT, D, RA0}},

{"lbzu",	OP(35),		OP_MASK,     COM,	PPCVLE,		{RT, D, RAL}},

{"stw",		OP(36),		OP_MASK,     PPCCOM,	PPCVLE,		{RS, D, RA0}},
{"st",		OP(36),		OP_MASK,     PWRCOM,	PPCVLE,		{RS, D, RA0}},

{"stwu",	OP(37),		OP_MASK,     PPCCOM,	PPCVLE,		{RS, D, RAS}},
{"stu",		OP(37),		OP_MASK,     PWRCOM,	PPCVLE,		{RS, D, RA0}},

{"stb",		OP(38),		OP_MASK,     COM,	PPCVLE,		{RS, D, RA0}},

{"stbu",	OP(39),		OP_MASK,     COM,	PPCVLE,		{RS, D, RAS}},

{"lhz",		OP(40),		OP_MASK,     COM,	PPCVLE,		{RT, D, RA0}},

{"lhzu",	OP(41),		OP_MASK,     COM,	PPCVLE,		{RT, D, RAL}},

{"lha",		OP(42),		OP_MASK,     COM,	PPCVLE,		{RT, D, RA0}},

{"lhau",	OP(43),		OP_MASK,     COM,	PPCVLE,		{RT, D, RAL}},

{"sth",		OP(44),		OP_MASK,     COM,	PPCVLE,		{RS, D, RA0}},

{"sthu",	OP(45),		OP_MASK,     COM,	PPCVLE,		{RS, D, RAS}},

{"lmw",		OP(46),		OP_MASK,     PPCCOM,	PPCVLE,		{RT, D, RAM}},
{"lm",		OP(46),		OP_MASK,     PWRCOM,	PPCVLE,		{RT, D, RA0}},

{"stmw",	OP(47),		OP_MASK,     PPCCOM,	PPCVLE,		{RS, D, RA0}},
{"stm",		OP(47),		OP_MASK,     PWRCOM,	PPCVLE,		{RS, D, RA0}},

{"lfs",		OP(48),		OP_MASK,     COM,	PPCEFS|PPCVLE,	{FRT, D, RA0}},

{"lfsu",	OP(49),		OP_MASK,     COM,	PPCEFS|PPCVLE,	{FRT, D, RAS}},

{"lfd",		OP(50),		OP_MASK,     COM,	PPCEFS|PPCVLE,	{FRT, D, RA0}},

{"lfdu",	OP(51),		OP_MASK,     COM,	PPCEFS|PPCVLE,	{FRT, D, RAS}},

{"stfs",	OP(52),		OP_MASK,     COM,	PPCEFS|PPCVLE,	{FRS, D, RA0}},

{"stfsu",	OP(53),		OP_MASK,     COM,	PPCEFS|PPCVLE,	{FRS, D, RAS}},

{"stfd",	OP(54),		OP_MASK,     COM,	PPCEFS|PPCVLE,	{FRS, D, RA0}},

{"stfdu",	OP(55),		OP_MASK,     COM,	PPCEFS|PPCVLE,	{FRS, D, RAS}},

{"lq",		OP(56),	     OP_MASK|Q_MASK, POWER4,	PPC476|PPCVLE,	{RTQ, DQ, RAQ}},
{"psq_l",	OP(56),		OP_MASK,     PPCPS,	PPCVLE,		{FRT,PSD,RA,PSW,PSQ}},
{"lfq",		OP(56),		OP_MASK,     POWER2,	PPCVLE,		{FRT, D, RA0}},

{"lxsd",	DSO(57,2),	DS_MASK,     PPCVSX3,	PPCVLE,		{VD, DS, RA0}},
{"lxssp",	DSO(57,3),	DS_MASK,     PPCVSX3,	PPCVLE,		{VD, DS, RA0}},
{"lfdp",	OP(57),	     OP_MASK|Q_MASK, POWER6,	POWER7|PPCVLE,	{FRTp, DS, RA0}},
{"psq_lu",	OP(57),		OP_MASK,     PPCPS,	PPCVLE,		{FRT,PSD,RA,PSW,PSQ}},
{"lfqu",	OP(57),		OP_MASK,     POWER2,	PPCVLE,		{FRT, D, RA0}},

{"ld",		DSO(58,0),	DS_MASK,     PPC64,	PPCVLE,		{RT, DS, RA0}},
{"ldu",		DSO(58,1),	DS_MASK,     PPC64,	PPCVLE,		{RT, DS, RAL}},
{"lwa",		DSO(58,2),	DS_MASK,     PPC64,	PPCVLE,		{RT, DS, RA0}},

{"dadd",	XRC(59,2,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},
{"dadd.",	XRC(59,2,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},

{"dqua",	ZRC(59,3,0),	Z2_MASK,     POWER6,	PPCVLE,		{FRT,FRA,FRB,RMC}},
{"dqua.",	ZRC(59,3,1),	Z2_MASK,     POWER6,	PPCVLE,		{FRT,FRA,FRB,RMC}},

{"dmxvi8ger4pp",XX3(59,2),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi8ger4pp",	XX3(59,2),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvi8ger4",	XX3(59,3),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi8ger4",	XX3(59,3),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"fdivs",	A(59,18,0),	AFRC_MASK,   PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fdivs.",	A(59,18,1),	AFRC_MASK,   PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},

{"fsubs",	A(59,20,0),	AFRC_MASK,   PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fsubs.",	A(59,20,1),	AFRC_MASK,   PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},

{"fadds",	A(59,21,0),	AFRC_MASK,   PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fadds.",	A(59,21,1),	AFRC_MASK,   PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},

{"fsqrts",	A(59,22,0),    AFRAFRC_MASK, PPC,	TITAN|PPCVLE,	{FRT, FRB}},
{"fsqrts.",	A(59,22,1),    AFRAFRC_MASK, PPC,	TITAN|PPCVLE,	{FRT, FRB}},

{"fres",	A(59,24,0),   AFRAFRC_MASK,  POWER7,	PPCVLE,		{FRT, FRB}},
{"fres",	A(59,24,0),   AFRALFRC_MASK, PPC,	POWER7|PPCVLE,	{FRT, FRB, A_L}},
{"fres.",	A(59,24,1),   AFRAFRC_MASK,  POWER7,	PPCVLE,		{FRT, FRB}},
{"fres.",	A(59,24,1),   AFRALFRC_MASK, PPC,	POWER7|PPCVLE,	{FRT, FRB, A_L}},

{"fmuls",	A(59,25,0),	AFRB_MASK,   PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC}},
{"fmuls.",	A(59,25,1),	AFRB_MASK,   PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC}},

{"frsqrtes",	A(59,26,0),   AFRAFRC_MASK,  POWER7,	PPCVLE,		{FRT, FRB}},
{"frsqrtes",	A(59,26,0),   AFRALFRC_MASK, POWER5,	POWER7|PPCVLE,	{FRT, FRB, A_L}},
{"frsqrtes.",	A(59,26,1),   AFRAFRC_MASK,  POWER7,	PPCVLE,		{FRT, FRB}},
{"frsqrtes.",	A(59,26,1),   AFRALFRC_MASK, POWER5,	POWER7|PPCVLE,	{FRT, FRB, A_L}},

{"fmsubs",	A(59,28,0),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fmsubs.",	A(59,28,1),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},

{"fmadds",	A(59,29,0),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fmadds.",	A(59,29,1),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},

{"fnmsubs",	A(59,30,0),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fnmsubs.",	A(59,30,1),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},

{"fnmadds",	A(59,31,0),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fnmadds.",	A(59,31,1),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},

{"dmul",	XRC(59,34,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},
{"dmul.",	XRC(59,34,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},

{"drrnd",	ZRC(59,35,0),	Z2_MASK,     POWER6,	PPCVLE,		{FRT, FRA, FRB, RMC}},
{"drrnd.",	ZRC(59,35,1),	Z2_MASK,     POWER6,	PPCVLE,		{FRT, FRA, FRB, RMC}},

{"dmxvi8gerx4pp", XX3(59,10),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},
{"dmxvi8gerx4",   XX3(59,11),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"dscli",	ZRC(59,66,0),	Z_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, SH16}},
{"dscli.",	ZRC(59,66,1),	Z_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, SH16}},

{"dquai",	ZRC(59,67,0),	Z2_MASK,     POWER6,	PPCVLE,		{TE, FRT,FRB,RMC}},
{"dquai.",	ZRC(59,67,1),	Z2_MASK,     POWER6,	PPCVLE,		{TE, FRT,FRB,RMC}},

{"dmxvf16ger2pp",XX3(59,18),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf16ger2pp",	 XX3(59,18),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvf16ger2",	 XX3(59,19),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf16ger2",	 XX3(59,19),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dscri",	ZRC(59,98,0),	Z_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, SH16}},
{"dscri.",	ZRC(59,98,1),	Z_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, SH16}},

{"drintx",	ZRC(59,99,0),	Z2_MASK,     POWER6,	PPCVLE,		{R, FRT, FRB, RMC}},
{"drintx.",	ZRC(59,99,1),	Z2_MASK,     POWER6,	PPCVLE,		{R, FRT, FRB, RMC}},

{"dmxvf32gerpp",XX3(59,26),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf32gerpp",	XX3(59,26),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvf32ger",	XX3(59,27),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf32ger",	XX3(59,27),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dcmpo",	X(59,130),	X_MASK,	     POWER6,	PPCVLE,		{BF,  FRA, FRB}},

{"dmxvi4ger8pp",XX3(59,34),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi4ger8pp",	XX3(59,34),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvi4ger8",	XX3(59,35),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi4ger8",	XX3(59,35),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dtstex",	X(59,162),	X_MASK,	     POWER6,	PPCVLE,		{BF,  FRA, FRB}},

{"dmxvi16ger2spp",XX3(59,42),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi16ger2spp",  XX3(59,42),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvi16ger2s",  XX3(59,43),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi16ger2s",	  XX3(59,43),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dtstdc",	Z(59,194),	Z_MASK,	     POWER6,	PPCVLE,		{BF,  FRA, DCM}},

{"dmxvbf16ger2pp",XX3(59,50),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvbf16ger2pp",  XX3(59,50),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvbf16ger2",  XX3(59,51),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvbf16ger2",	  XX3(59,51),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dtstdg",	Z(59,226),	Z_MASK,	     POWER6,	PPCVLE,		{BF,  FRA, DGM}},

{"drintn",	ZRC(59,227,0),	Z2_MASK,     POWER6,	PPCVLE,		{R, FRT, FRB, RMC}},
{"drintn.",	ZRC(59,227,1),	Z2_MASK,     POWER6,	PPCVLE,		{R, FRT, FRB, RMC}},

{"dmxvf64gerpp",XX3(59,58),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},
{"xvf64gerpp",	XX3(59,58),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},
{"dmxvf64ger",	XX3(59,59),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},
{"xvf64ger",	XX3(59,59),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},

{"dctdp",	XRC(59,258,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRB}},
{"dctdp.",	XRC(59,258,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRB}},

{"dmxvf16gerx2pp", XX3(59,66),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},
{"dmxvf16gerx2",   XX3(59,67),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"dctfix",	XRC(59,290,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRB}},
{"dctfix.",	XRC(59,290,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRB}},

{"ddedpd",	XRC(59,322,0),	X_MASK,	     POWER6,	PPCVLE,		{SP, FRT, FRB}},
{"ddedpd.",	XRC(59,322,1),	X_MASK,	     POWER6,	PPCVLE,		{SP, FRT, FRB}},

{"dmxvbf16gerx2pp", XX3(59,74),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},
{"dmxvi16ger2",	XX3(59,75),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi16ger2",	XX3(59,75),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dmxvf16ger2np", XX3(59,82),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf16ger2np",	  XX3(59,82),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvf16gerx2np",XX3(59,83),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"dxex",	XRC(59,354,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRB}},
{"dxex.",	XRC(59,354,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRB}},

{"dmxvf32gernp",  XX3(59,90),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf32gernp",	  XX3(59,90),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvbf16gerx2", XX3(59,91),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"dmxvi8gerx4spp",XX3(59,98),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},
{"dmxvi8ger4spp", XX3(59,99),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi8ger4spp",	  XX3(59,99),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dmxvi16ger2pp", XX3(59,107),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvi16ger2pp",	  XX3(59,107),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dmxvbf16ger2np",XX3(59,114),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvbf16ger2np",  XX3(59,114),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvbf16gerx2np",XX3(59,115),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"dmxvf64gernp",  XX3(59,122),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},
{"xvf64gernp",	  XX3(59,122),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},

{"dsub",	XRC(59,514,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},
{"dsub.",	XRC(59,514,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},

{"ddiv",	XRC(59,546,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},
{"ddiv.",	XRC(59,546,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},

{"dmxvf16ger2pn", XX3(59,146),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf16ger2pn",	  XX3(59,146),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvf16gerx2pn",XX3(59,147),	XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"dmxvf32gerpn",XX3(59,154),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf32gerpn",	XX3(59,154),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dcmpu",	X(59,642),	X_MASK,	     POWER6,	PPCVLE,		{BF,  FRA, FRB}},

{"dtstsf",	X(59,674),	X_MASK,	     POWER6,	PPCVLE,		{BF,  FRA, FRB}},
{"dtstsfi",	X(59,675),	X_MASK|1<<22,POWER9,	PPCVLE,		{BF, UIM6, FRB}},

{"dmxvbf16ger2pn",XX3(59,178),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvbf16ger2pn",  XX3(59,178),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"dmxvbf16gerx2pn", XX3(59,179),XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"dmxvf64gerpn",XX3(59,186),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},
{"xvf64gerpn",	XX3(59,186),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},

{"drsp",	XRC(59,770,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRB}},
{"drsp.",	XRC(59,770,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRB}},

{"dcffix",	XRC(59,802,0), X_MASK|FRA_MASK, POWER7,	PPCVLE,		{FRT, FRB}},
{"dcffix.",	XRC(59,802,1), X_MASK|FRA_MASK, POWER7,	PPCVLE,		{FRT, FRB}},

{"dmxvf16gerx2nn", XX3(59,202),	XX3GERX_MASK,  FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"denbcd",	XRC(59,834,0),	X_MASK,	     POWER6,	PPCVLE,		{S, FRT, FRB}},
{"denbcd.",	XRC(59,834,1),	X_MASK,	     POWER6,	PPCVLE,		{S, FRT, FRB}},

{"dmxvf16ger2nn", XX3(59,210),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf16ger2nn",	  XX3(59,210),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"fcfids",	XRC(59,846,0),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},
{"fcfids.",	XRC(59,846,1),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},

{"diex",	XRC(59,866,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},
{"diex.",	XRC(59,866,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRA, FRB}},

{"dmxvf32gernn",XX3(59,218),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvf32gernn",	XX3(59,218),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"dmxvbf16gerx2nn", XX3(59,234),XX3GERX_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB6}},

{"dmxvbf16ger2nn",XX3(59,242),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},
{"xvbf16ger2nn",  XX3(59,242),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6a, XB6a}},

{"fcfidus",	XRC(59,974,0),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},
{"fcfidus.",	XRC(59,974,1),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},

{"dmxvf64gernn",XX3(59,250),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},
{"xvf64gernn",	XX3(59,250),	XX3ACC_MASK, POWER10,	PPCVLE,		{ACC, XA6ap, XB6a}},

{"xsaddsp",	XX3(60,0),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xsmaddasp",	XX3(60,1),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xxsldwi",	XX3(60,2),	XX3SHW_MASK, PPCVSX,	PPCVLE,		{XT6, XA6, XB6, SHW}},
{"xscmpeqdp",	XX3(60,3),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xsrsqrtesp",	XX2(60,10),	XX2_MASK,    PPCVSX2,	PPCVLE,		{XT6, XB6}},
{"xssqrtsp",	XX2(60,11),	XX2_MASK,    PPCVSX2,	PPCVLE,		{XT6, XB6}},
{"xxsel",	XX4(60,3),	XX4_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6, XC6}},
{"xssubsp",	XX3(60,8),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xsmaddmsp",	XX3(60,9),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xxspltd",	XX3(60,10),	XX3DM_MASK,  PPCVSX,	PPCVLE|EXT,	{XT6, XAB6, DMEX}},
{"xxmrghd",	XX3(60,10),	XX3_MASK,    PPCVSX,	PPCVLE|EXT,	{XT6, XA6, XB6}},
{"xxswapd",	XX3(60,10)|(2<<8), XX3_MASK, PPCVSX,	PPCVLE|EXT,	{XT6, XAB6}},
{"xxmrgld",	XX3(60,10)|(3<<8), XX3_MASK, PPCVSX,	PPCVLE|EXT,	{XT6, XA6, XB6}},
{"xxpermdi",	XX3(60,10),	XX3DM_MASK,  PPCVSX,	PPCVLE,		{XT6, XA6, XB6, DM}},
{"xscmpgtdp",	XX3(60,11),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xsresp",	XX2(60,26),	XX2_MASK,    PPCVSX2,	PPCVLE,		{XT6, XB6}},
{"xsmulsp",	XX3(60,16),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xsmsubasp",	XX3(60,17),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xxmrghw",	XX3(60,18),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xscmpgedp",	XX3(60,19),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xsdivsp",	XX3(60,24),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xsmsubmsp",	XX3(60,25),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xxperm",	XX3(60,26),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xsadddp",	XX3(60,32),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsmaddadp",	XX3(60,33),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xscmpudp",	XX3(60,35),	XX3BF_MASK,  PPCVSX,	PPCVLE,		{BF, XA6, XB6}},
{"xscvdpuxws",	XX2(60,72),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsrdpi",	XX2(60,73),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsrsqrtedp",	XX2(60,74),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xssqrtdp",	XX2(60,75),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xssubdp",	XX3(60,40),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsmaddmdp",	XX3(60,41),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xscmpodp",	XX3(60,43),	XX3BF_MASK,  PPCVSX,	PPCVLE,		{BF, XA6, XB6}},
{"xscvdpsxws",	XX2(60,88),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsrdpiz",	XX2(60,89),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsredp",	XX2(60,90),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsmuldp",	XX3(60,48),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsmsubadp",	XX3(60,49),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xxmrglw",	XX3(60,50),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsrdpip",	XX2(60,105),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xstsqrtdp",	XX2(60,106),	XX2BF_MASK,  PPCVSX,	PPCVLE,		{BF, XB6}},
{"xsrdpic",	XX2(60,107),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsdivdp",	XX3(60,56),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsmsubmdp",	XX3(60,57),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xxpermr",	XX3(60,58),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xscmpexpdp",	XX3(60,59),	XX3BF_MASK,  PPCVSX3,	PPCVLE,		{BF, XA6, XB6}},
{"xsrdpim",	XX2(60,121),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xstdivdp",	XX3(60,61),	XX3BF_MASK,  PPCVSX,	PPCVLE,		{BF, XA6, XB6}},
{"xvaddsp",	XX3(60,64),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvmaddasp",	XX3(60,65),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpeqsp",	XX3RC(60,67,0),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpeqsp.",	XX3RC(60,67,1),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvspuxws",	XX2(60,136),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrspi",	XX2(60,137),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrsqrtesp",	XX2(60,138),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvsqrtsp",	XX2(60,139),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvsubsp",	XX3(60,72),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvmaddmsp",	XX3(60,73),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpgtsp",	XX3RC(60,75,0),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpgtsp.",	XX3RC(60,75,1),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvspsxws",	XX2(60,152),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrspiz",	XX2(60,153),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvresp",	XX2(60,154),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvmulsp",	XX3(60,80),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvmsubasp",	XX3(60,81),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xxspltw",	XX2(60,164),	XX2UIM_MASK, PPCVSX,	PPCVLE,		{XT6, XB6, UIM}},
{"xxextractuw",	XX2(60,165),   XX2UIM4_MASK, PPCVSX3,	PPCVLE,		{XT6, XB6, UIMM4}},
{"xvcmpgesp",	XX3RC(60,83,0),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpgesp.",	XX3RC(60,83,1),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvuxwsp",	XX2(60,168),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrspip",	XX2(60,169),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvtsqrtsp",	XX2(60,170),	XX2BF_MASK,  PPCVSX,	PPCVLE,		{BF, XB6}},
{"xvrspic",	XX2(60,171),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvdivsp",	XX3(60,88),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvmsubmsp",	XX3(60,89),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xxspltib",	X(60,360),   XX1_MASK|3<<19, PPCVSX3,	PPCVLE,		{XT6, IMM8}},
{"lxvkq",	XVA(60,360,31),	XVA_MASK&~1, POWER10,	PPCVLE,		{XT6, UIM5}},
{"xxinsertw",	XX2(60,181),   XX2UIM4_MASK, PPCVSX3,	PPCVLE,		{XT6, XB6, UIMM4}},
{"xvcvsxwsp",	XX2(60,184),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrspim",	XX2(60,185),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvtdivsp",	XX3(60,93),	XX3BF_MASK,  PPCVSX,	PPCVLE,		{BF, XA6, XB6}},
{"xvadddp",	XX3(60,96),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvmaddadp",	XX3(60,97),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpeqdp",	XX3RC(60,99,0),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpeqdp.",	XX3RC(60,99,1),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvdpuxws",	XX2(60,200),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrdpi",	XX2(60,201),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrsqrtedp",	XX2(60,202),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvsqrtdp",	XX2(60,203),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvsubdp",	XX3(60,104),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvmaddmdp",	XX3(60,105),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpgtdp",	XX3RC(60,107,0), XX3_MASK,   PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpgtdp.",	XX3RC(60,107,1), XX3_MASK,   PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvdpsxws",	XX2(60,216),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrdpiz",	XX2(60,217),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvredp",	XX2(60,218),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvmuldp",	XX3(60,112),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvmsubadp",	XX3(60,113),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpgedp",	XX3RC(60,115,0), XX3_MASK,   PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcmpgedp.",	XX3RC(60,115,1), XX3_MASK,   PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvuxwdp",	XX2(60,232),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrdpip",	XX2(60,233),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvtsqrtdp",	XX2(60,234),	XX2BF_MASK,  PPCVSX,	PPCVLE,		{BF, XB6}},
{"xvrdpic",	XX2(60,235),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvdivdp",	XX3(60,120),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvmsubmdp",	XX3(60,121),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvsxwdp",	XX2(60,248),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvrdpim",	XX2(60,249),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvtdivdp",	XX3(60,125),	XX3BF_MASK,  PPCVSX,	PPCVLE,		{BF, XA6, XB6}},
{"xsmaxcdp",	XX3(60,128),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xsnmaddasp",	XX3(60,129),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xxland",	XX3(60,130),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xscvdpsp",	XX2(60,265),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xscvdpspn",	XX2(60,267),	XX2_MASK,    PPCVSX2,	PPCVLE,		{XT6, XB6}},
{"xsmincdp",	XX3(60,136),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xsnmaddmsp",	XX3(60,137),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xxlandc",	XX3(60,138),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsrsp",	XX2(60,281),	XX2_MASK,    PPCVSX2,	PPCVLE,		{XT6, XB6}},
{"xsmaxjdp",	XX3(60,144),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xsnmsubasp",	XX3(60,145),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xxmr",	XX3(60,146),	XX3_MASK,    PPCVSX,	PPCVLE|EXT,	{XT6, XAB6}},
{"xxlor",	XX3(60,146),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xscvuxdsp",	XX2(60,296),	XX2_MASK,    PPCVSX2,	PPCVLE,		{XT6, XB6}},
{"xststdcsp",	XX2(60,298),	XX2BFD_MASK, PPCVSX3,	PPCVLE,		{BF, XB6, DCMX}},
{"xsminjdp",	XX3(60,152),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xsnmsubmsp",	XX3(60,153),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xxlxor",	XX3(60,154),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xscvsxdsp",	XX2(60,312),	XX2_MASK,    PPCVSX2,	PPCVLE,		{XT6, XB6}},
{"xsmaxdp",	XX3(60,160),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsnmaddadp",	XX3(60,161),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xxlnot",	XX3(60,162),	XX3_MASK,    PPCVSX,	PPCVLE|EXT,	{XT6, XAB6}},
{"xxlnor",	XX3(60,162),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xscvdpuxds",	XX2(60,328),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xscvspdp",	XX2(60,329),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xscvspdpn",	XX2(60,331),	XX2_MASK,    PPCVSX2,	PPCVLE,		{XT6, XB6}},
{"xsmindp",	XX3(60,168),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsnmaddmdp",	XX3(60,169),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xxlorc",	XX3(60,170),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xscvdpsxds",	XX2(60,344),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsabsdp",	XX2(60,345),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsxexpdp",	XX2VA(60,347,0),XX2_MASK|1,  PPCVSX3,	PPCVLE,		{RT, XB6}},
{"xsxsigdp",	XX2VA(60,347,1),XX2_MASK|1,  PPCVSX3,	PPCVLE,		{RT, XB6}},
{"xscvhpdp",	XX2VA(60,347,16),XX2_MASK,   PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xscvdphp",	XX2VA(60,347,17),XX2_MASK,   PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xscpsgndp",	XX3(60,176),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xsnmsubadp",	XX3(60,177),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xxlnand",	XX3(60,178),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xscvuxddp",	XX2(60,360),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsnabsdp",	XX2(60,361),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xststdcdp",	XX2(60,362),	XX2BFD_MASK, PPCVSX3,	PPCVLE,		{BF, XB6, DCMX}},
{"xsnmsubmdp",	XX3(60,185),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xxleqv",	XX3(60,186),	XX3_MASK,    PPCVSX2,	PPCVLE,		{XT6, XA6, XB6}},
{"xscvsxddp",	XX2(60,376),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xsnegdp",	XX2(60,377),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvmaxsp",	XX3(60,192),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvnmaddasp",	XX3(60,193),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvspuxds",	XX2(60,392),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvcvdpsp",	XX2(60,393),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvminsp",	XX3(60,200),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvnmaddmsp",	XX3(60,201),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvspsxds",	XX2(60,408),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvabssp",	XX2(60,409),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvmovsp",	XX3(60,208),	XX3_MASK,    PPCVSX,	PPCVLE|EXT,	{XT6, XAB6}},
{"xvcpsgnsp",	XX3(60,208),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvnmsubasp",	XX3(60,209),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvuxdsp",	XX2(60,424),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvnabssp",	XX2(60,425),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvtstdcsp",	XX2(60,426),  XX2DCMXS_MASK, PPCVSX3,	PPCVLE,		{XT6, XB6, DCMXS}},
{"xviexpsp",	XX3(60,216),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xvnmsubmsp",	XX3(60,217),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvsxdsp",	XX2(60,440),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvnegsp",	XX2(60,441),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvmaxdp",	XX3(60,224),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvnmaddadp",	XX3(60,225),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"dmxxextfdmr512",XX3(60,226),	XX3DMR_MASK, FUTURE,	PPCVLE,		{XA5p, XB5p, DMR, P1}},
{"xvcvdpuxds",	XX2(60,456),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvcvspdp",	XX2(60,457),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xxgenpcvbm",	X(60,916),	XX1_MASK,    POWER10,	PPCVLE,		{XT6, VB, UIMM}},
{"xxgenpcvhm",	X(60,917),	XX1_MASK,    POWER10,	PPCVLE,		{XT6, VB, UIMM}},
{"xsiexpdp",	X(60,918),	XX1_MASK,    PPCVSX3,	PPCVLE,		{XT6, RA, RB}},
{"xvmindp",	XX3(60,232),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvnmaddmdp",	XX3(60,233),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"dmxxinstdmr512",XX3(60,234),	XX3DMR_MASK, FUTURE,	PPCVLE,		{DMR, XA5p, XB5p,P1}},
{"xvcvdpsxds",	XX2(60,472),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvabsdp",	XX2(60,473),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xxgenpcvwm",	X(60,948),	XX1_MASK,    POWER10,	PPCVLE,		{XT6, VB, UIMM}},
{"xxgenpcvdm",	X(60,949),	XX1_MASK,    POWER10,	PPCVLE,		{XT6, VB, UIMM}},
{"xvxexpdp",	XX2VA(60,475,0),XX2_MASK,    PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xvxsigdp",	XX2VA(60,475,1),XX2_MASK,    PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xvtlsbb",	XX2VA(60,475,2),XX2BF_MASK,  POWER10,	PPCVLE,		{BF, XB6}},
{"xxbrh",	XX2VA(60,475,7),XX2_MASK,    PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xvxexpsp",	XX2VA(60,475,8),XX2_MASK,    PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xvxsigsp",	XX2VA(60,475,9),XX2_MASK,    PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xxbrw",	XX2VA(60,475,15),XX2_MASK,   PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xvcvbf16spn",	XX2VA(60,475,16),XX2_MASK,   PPCVSX4,	PPCVLE,		{XT6, XB6}},
{"xvcvspbf16",	XX2VA(60,475,17),XX2_MASK,   PPCVSX4,	PPCVLE,		{XT6, XB6}},
{"xxbrd",	XX2VA(60,475,23),XX2_MASK,   PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xvcvhpsp",	XX2VA(60,475,24),XX2_MASK,   PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xvcvsphp",	XX2VA(60,475,25),XX2_MASK,   PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xxbrq",	XX2VA(60,475,31),XX2_MASK,   PPCVSX3,	PPCVLE,		{XT6, XB6}},
{"xvmovdp",	XX3(60,240),	XX3_MASK,    PPCVSX,	PPCVLE|EXT,	{XT6, XAB6}},
{"xvcpsgndp",	XX3(60,240),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvnmsubadp",	XX3(60,241),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"dmxxextfdmr256",XX2(60,484),	XX2DMR_MASK, FUTURE,	PPCVLE,		{XB5p, DMR, P2}},
{"dmxxinstdmr256",XX2(60,485),	XX2DMR_MASK, FUTURE,	PPCVLE,		{DMR, XB5p, P2}},
{"xvcvuxddp",	XX2(60,488),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvnabsdp",	XX2(60,489),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvtstdcdp",	XX2(60,490),  XX2DCMXS_MASK, PPCVSX3,	PPCVLE,		{XT6, XB6, DCMXS}},
{"xviexpdp",	XX3(60,248),	XX3_MASK,    PPCVSX3,	PPCVLE,		{XT6, XA6, XB6}},
{"xvnmsubmdp",	XX3(60,249),	XX3_MASK,    PPCVSX,	PPCVLE,		{XT6, XA6, XB6}},
{"xvcvsxddp",	XX2(60,504),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},
{"xvnegdp",	XX2(60,505),	XX2_MASK,    PPCVSX,	PPCVLE,		{XT6, XB6}},

{"psq_st",	OP(60),		OP_MASK,     PPCPS,	PPCVLE,		{FRS,PSD,RA,PSW,PSQ}},
{"stfq",	OP(60),		OP_MASK,     POWER2,	PPCVLE,		{FRS, D, RA}},

{"lxv",		DQX(61,1),	DQX_MASK,    PPCVSX3,	PPCVLE,		{XTQ6, DQ, RA0}},
{"stxv",	DQX(61,5),	DQX_MASK,    PPCVSX3,	PPCVLE,		{XSQ6, DQ, RA0}},
{"stxsd",	DSO(61,2),	DS_MASK,     PPCVSX3,	PPCVLE,		{VS, DS, RA0}},
{"stxssp",	DSO(61,3),	DS_MASK,     PPCVSX3,	PPCVLE,		{VS, DS, RA0}},
{"stfdp",	OP(61),	     OP_MASK|Q_MASK, POWER6,	POWER7|PPCVLE,	{FRSp, DS, RA0}},
{"psq_stu",	OP(61),		OP_MASK,     PPCPS,	PPCVLE,		{FRS,PSD,RA,PSW,PSQ}},
{"stfqu",	OP(61),		OP_MASK,     POWER2,	PPCVLE,		{FRS, D, RA}},

{"std",		DSO(62,0),	DS_MASK,     PPC64,	PPCVLE,		{RS, DS, RA0}},
{"stdu",	DSO(62,1),	DS_MASK,     PPC64,	PPCVLE,		{RS, DS, RAS}},
{"stq",		DSO(62,2),   DS_MASK|Q_MASK, POWER4,	PPC476|PPCVLE,	{RSQ, DS, RA0}},

{"fcmpu",	X(63,0),	XBF_MASK,    COM,	PPCEFS|PPCVLE,	{BF, FRA, FRB}},

{"daddq",	XRC(63,2,0),  X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp}},
{"daddq.",	XRC(63,2,1),  X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp}},

{"dquaq",	ZRC(63,3,0), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp, RMC}},
{"dquaq.",	ZRC(63,3,1), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp, RMC}},

{"xsaddqp",	XRC(63,4,0),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},
{"xsaddqpo",	XRC(63,4,1),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"xsrqpi",	ZRC(63,5,0),	Z2_MASK,     PPCVSX3,	PPCVLE,		{R, VD, VB, RMC}},
{"xsrqpix",	ZRC(63,5,1),	Z2_MASK,     PPCVSX3,	PPCVLE,		{R, VD, VB, RMC}},

{"fcpsgn",	XRC(63,8,0),	X_MASK, POWER6|PPCA2|PPC476, PPCVLE,	{FRT, FRA, FRB}},
{"fcpsgn.",	XRC(63,8,1),	X_MASK, POWER6|PPCA2|PPC476, PPCVLE,	{FRT, FRA, FRB}},

{"frsp",	XRC(63,12,0),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"frsp.",	XRC(63,12,1),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},

{"fctiw",	XRC(63,14,0),	XRA_MASK,    PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"fcir",	XRC(63,14,0),	XRA_MASK,    PWR2COM,	PPCVLE,		{FRT, FRB}},
{"fctiw.",	XRC(63,14,1),	XRA_MASK,    PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"fcir.",	XRC(63,14,1),	XRA_MASK,    PWR2COM,	PPCVLE,		{FRT, FRB}},

{"fctiwz",	XRC(63,15,0),	XRA_MASK,    PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"fcirz",	XRC(63,15,0),	XRA_MASK,    PWR2COM,	PPCVLE,		{FRT, FRB}},
{"fctiwz.",	XRC(63,15,1),	XRA_MASK,    PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"fcirz.",	XRC(63,15,1),	XRA_MASK,    PWR2COM,	PPCVLE,		{FRT, FRB}},

{"fdiv",	A(63,18,0),	AFRC_MASK,   PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fd",		A(63,18,0),	AFRC_MASK,   PWRCOM,	PPCVLE,		{FRT, FRA, FRB}},
{"fdiv.",	A(63,18,1),	AFRC_MASK,   PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fd.",		A(63,18,1),	AFRC_MASK,   PWRCOM,	PPCVLE,		{FRT, FRA, FRB}},

{"fsub",	A(63,20,0),	AFRC_MASK,   PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fs",		A(63,20,0),	AFRC_MASK,   PWRCOM,	PPCVLE,		{FRT, FRA, FRB}},
{"fsub.",	A(63,20,1),	AFRC_MASK,   PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fs.",		A(63,20,1),	AFRC_MASK,   PWRCOM,	PPCVLE,		{FRT, FRA, FRB}},

{"fadd",	A(63,21,0),	AFRC_MASK,   PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fa",		A(63,21,0),	AFRC_MASK,   PWRCOM,	PPCVLE,		{FRT, FRA, FRB}},
{"fadd.",	A(63,21,1),	AFRC_MASK,   PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRB}},
{"fa.",		A(63,21,1),	AFRC_MASK,   PWRCOM,	PPCVLE,		{FRT, FRA, FRB}},

{"fsqrt",	A(63,22,0),    AFRAFRC_MASK, PPCPWR2,	TITAN|PPCVLE,	{FRT, FRB}},
{"fsqrt.",	A(63,22,1),    AFRAFRC_MASK, PPCPWR2,	TITAN|PPCVLE,	{FRT, FRB}},

{"fsel",	A(63,23,0),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fsel.",	A(63,23,1),	A_MASK,	     PPC,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},

{"fre",		A(63,24,0),   AFRAFRC_MASK,  POWER7,	PPCVLE,		{FRT, FRB}},
{"fre",		A(63,24,0),   AFRALFRC_MASK, POWER5,	POWER7|PPCVLE,	{FRT, FRB, A_L}},
{"fre.",	A(63,24,1),   AFRAFRC_MASK,  POWER7,	PPCVLE,		{FRT, FRB}},
{"fre.",	A(63,24,1),   AFRALFRC_MASK, POWER5,	POWER7|PPCVLE,	{FRT, FRB, A_L}},

{"fmul",	A(63,25,0),	AFRB_MASK,   PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC}},
{"fm",		A(63,25,0),	AFRB_MASK,   PWRCOM,	PPCVLE|PPCVLE,	{FRT, FRA, FRC}},
{"fmul.",	A(63,25,1),	AFRB_MASK,   PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC}},
{"fm.",		A(63,25,1),	AFRB_MASK,   PWRCOM,	PPCVLE|PPCVLE,	{FRT, FRA, FRC}},

{"frsqrte",	A(63,26,0),   AFRAFRC_MASK,  POWER7,	PPCVLE,		{FRT, FRB}},
{"frsqrte",	A(63,26,0),   AFRALFRC_MASK, PPC,	POWER7|PPCVLE,	{FRT, FRB, A_L}},
{"frsqrte.",	A(63,26,1),   AFRAFRC_MASK,  POWER7,	PPCVLE,		{FRT, FRB}},
{"frsqrte.",	A(63,26,1),   AFRALFRC_MASK, PPC,	POWER7|PPCVLE,	{FRT, FRB, A_L}},

{"fmsub",	A(63,28,0),	A_MASK,	     PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fms",		A(63,28,0),	A_MASK,	     PWRCOM,	PPCVLE,		{FRT, FRA, FRC, FRB}},
{"fmsub.",	A(63,28,1),	A_MASK,	     PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fms.",	A(63,28,1),	A_MASK,	     PWRCOM,	PPCVLE,		{FRT, FRA, FRC, FRB}},

{"fmadd",	A(63,29,0),	A_MASK,	     PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fma",		A(63,29,0),	A_MASK,	     PWRCOM,	PPCVLE,		{FRT, FRA, FRC, FRB}},
{"fmadd.",	A(63,29,1),	A_MASK,	     PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fma.",	A(63,29,1),	A_MASK,	     PWRCOM,	PPCVLE,		{FRT, FRA, FRC, FRB}},

{"fnmsub",	A(63,30,0),	A_MASK,	     PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fnms",	A(63,30,0),	A_MASK,	     PWRCOM,	PPCVLE,		{FRT, FRA, FRC, FRB}},
{"fnmsub.",	A(63,30,1),	A_MASK,	     PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fnms.",	A(63,30,1),	A_MASK,	     PWRCOM,	PPCVLE,		{FRT, FRA, FRC, FRB}},

{"fnmadd",	A(63,31,0),	A_MASK,	     PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fnma",	A(63,31,0),	A_MASK,	     PWRCOM,	PPCVLE,		{FRT, FRA, FRC, FRB}},
{"fnmadd.",	A(63,31,1),	A_MASK,	     PPCCOM,	PPCEFS|PPCVLE,	{FRT, FRA, FRC, FRB}},
{"fnma.",	A(63,31,1),	A_MASK,	     PWRCOM,	PPCVLE,		{FRT, FRA, FRC, FRB}},

{"fcmpo",	X(63,32),	XBF_MASK,    COM,	PPCEFS|PPCVLE,	{BF, FRA, FRB}},

{"dmulq",	XRC(63,34,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp}},
{"dmulq.",	XRC(63,34,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp}},

{"drrndq",	ZRC(63,35,0), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRA, FRBp, RMC}},
{"drrndq.",	ZRC(63,35,1), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRA, FRBp, RMC}},

{"xsmulqp",	XRC(63,36,0),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},
{"xsmulqpo",	XRC(63,36,1),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"xsrqpxp",	Z(63,37),	Z2_MASK,     PPCVSX3,	PPCVLE,		{R, VD, VB, RMC}},

{"mtfsb1",	XRC(63,38,0),	XRARB_MASK,  COM,	PPCVLE,		{BTF}},
{"mtfsb1.",	XRC(63,38,1),	XRARB_MASK,  COM,	PPCVLE,		{BTF}},

{"fneg",	XRC(63,40,0),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"fneg.",	XRC(63,40,1),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},

{"mcrfs",      X(63,64), XRB_MASK|(3<<21)|(3<<16), COM,	PPCVLE,		{BF, BFA}},

{"dscliq",	ZRC(63,66,0), Z_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, SH16}},
{"dscliq.",	ZRC(63,66,1), Z_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, SH16}},

{"dquaiq",	ZRC(63,67,0), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{TE, FRTp, FRBp, RMC}},
{"dquaiq.",	ZRC(63,67,1), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{TE, FRTp, FRBp, RMC}},

{"xscmpeqqp",	X(63,68),	X_MASK,	     POWER10,	PPCVLE,		{VD, VA, VB}},

{"mtfsb0",	XRC(63,70,0),	XRARB_MASK,  COM,	PPCVLE,		{BTF}},
{"mtfsb0.",	XRC(63,70,1),	XRARB_MASK,  COM,	PPCVLE,		{BTF}},

{"fmr",		XRC(63,72,0),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"fmr.",	XRC(63,72,1),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},

{"dscriq",	ZRC(63,98,0), Z_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, SH16}},
{"dscriq.",	ZRC(63,98,1), Z_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, SH16}},

{"drintxq",	ZRC(63,99,0), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{R, FRTp, FRBp, RMC}},
{"drintxq.",	ZRC(63,99,1), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{R, FRTp, FRBp, RMC}},

{"xscpsgnqp",	X(63,100),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"ftdiv",	X(63,128),	XBF_MASK,    POWER7,	PPCVLE,		{BF, FRA, FRB}},

{"dcmpoq",	X(63,130),	X_MASK,	     POWER6,	PPCVLE,		{BF, FRAp, FRBp}},

{"xscmpoqp",	X(63,132),	XBF_MASK,    PPCVSX3,	PPCVLE,		{BF, VA, VB}},

{"mtfsfi",  XRC(63,134,0), XWRA_MASK|(3<<21)|(1<<11), POWER6|PPCA2|PPC476, PPCVLE, {BFF, U, W}},
{"mtfsfi",  XRC(63,134,0), XRA_MASK|(3<<21)|(1<<11), COM, POWER6|PPCA2|PPC476|PPCVLE, {BFF, U}},
{"mtfsfi.", XRC(63,134,1), XWRA_MASK|(3<<21)|(1<<11), POWER6|PPCA2|PPC476, PPCVLE, {BFF, U, W}},
{"mtfsfi.", XRC(63,134,1), XRA_MASK|(3<<21)|(1<<11), COM, POWER6|PPCA2|PPC476|PPCVLE, {BFF, U}},

{"fnabs",	XRC(63,136,0),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"fnabs.",	XRC(63,136,1),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},

{"fctiwu",	XRC(63,142,0),	XRA_MASK,    POWER7,	PPCVLE,		{FRT, FRB}},
{"fctiwu.",	XRC(63,142,1),	XRA_MASK,    POWER7,	PPCVLE,		{FRT, FRB}},
{"fctiwuz",	XRC(63,143,0),	XRA_MASK,    POWER7,	PPCVLE,		{FRT, FRB}},
{"fctiwuz.",	XRC(63,143,1),	XRA_MASK,    POWER7,	PPCVLE,		{FRT, FRB}},

{"ftsqrt",	X(63,160),	XBF_MASK|FRA_MASK, POWER7, PPCVLE,	{BF, FRB}},

{"dtstexq",	X(63,162),	X_MASK,	     POWER6,	PPCVLE,		{BF, FRAp, FRBp}},

{"xscmpexpqp",	X(63,164),	XBF_MASK,    PPCVSX3,	PPCVLE,		{BF, VA, VB}},

{"dtstdcq",	Z(63,194),	Z_MASK,	     POWER6,	PPCVLE,		{BF, FRAp, DCM}},

{"xscmpgeqp",	X(63,196),	X_MASK,	     POWER10,	PPCVLE,		{VD, VA, VB}},

{"dtstdgq",	Z(63,226),	Z_MASK,	     POWER6,	PPCVLE,		{BF, FRAp, DGM}},

{"drintnq",	ZRC(63,227,0), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{R, FRTp, FRBp, RMC}},
{"drintnq.",	ZRC(63,227,1), Z2_MASK|Q_MASK, POWER6,	PPCVLE,		{R, FRTp, FRBp, RMC}},

{"xscmpgtqp",	X(63,228),	X_MASK,	     POWER10,	PPCVLE,		{VD, VA, VB}},

{"dctqpq",	XRC(63,258,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRB}},
{"dctqpq.",	XRC(63,258,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRB}},

{"fabs",	XRC(63,264,0),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},
{"fabs.",	XRC(63,264,1),	XRA_MASK,    COM,	PPCEFS|PPCVLE,	{FRT, FRB}},

{"dctfixq",	XRC(63,290,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRBp}},
{"dctfixq.",	XRC(63,290,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRBp}},

{"ddedpdq",	XRC(63,322,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{SP, FRTp, FRBp}},
{"ddedpdq.",	XRC(63,322,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{SP, FRTp, FRBp}},

{"dxexq",	XRC(63,354,0),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRBp}},
{"dxexq.",	XRC(63,354,1),	X_MASK,	     POWER6,	PPCVLE,		{FRT, FRBp}},

{"xsmaddqp",	XRC(63,388,0),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},
{"xsmaddqpo",	XRC(63,388,1),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"frin",	XRC(63,392,0),	XRA_MASK,    POWER5,	PPCVLE,		{FRT, FRB}},
{"frin.",	XRC(63,392,1),	XRA_MASK,    POWER5,	PPCVLE,		{FRT, FRB}},

{"xsmsubqp",	XRC(63,420,0),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},
{"xsmsubqpo",	XRC(63,420,1),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"friz",	XRC(63,424,0),	XRA_MASK,    POWER5,	PPCVLE,		{FRT, FRB}},
{"friz.",	XRC(63,424,1),	XRA_MASK,    POWER5,	PPCVLE,		{FRT, FRB}},

{"xsnmaddqp",	XRC(63,452,0),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},
{"xsnmaddqpo",	XRC(63,452,1),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"frip",	XRC(63,456,0),	XRA_MASK,    POWER5,	PPCVLE,		{FRT, FRB}},
{"frip.",	XRC(63,456,1),	XRA_MASK,    POWER5,	PPCVLE,		{FRT, FRB}},

{"xsnmsubqp",	XRC(63,484,0),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},
{"xsnmsubqpo",	XRC(63,484,1),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"frim",	XRC(63,488,0),	XRA_MASK,    POWER5,	PPCVLE,		{FRT, FRB}},
{"frim.",	XRC(63,488,1),	XRA_MASK,    POWER5,	PPCVLE,		{FRT, FRB}},

{"dsubq",	XRC(63,514,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp}},
{"dsubq.",	XRC(63,514,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp}},

{"xssubqp",	XRC(63,516,0),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},
{"xssubqpo",	XRC(63,516,1),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"ddivq",	XRC(63,546,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp}},
{"ddivq.",	XRC(63,546,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRAp, FRBp}},

{"xsdivqp",	XRC(63,548,0),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},
{"xsdivqpo",	XRC(63,548,1),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"mffs",	XRC(63,583,0),	XRARB_MASK,  COM,	PPCEFS|PPCVLE,	{FRT}},
{"mffs.",	XRC(63,583,1),	XRARB_MASK,  COM,	PPCEFS|PPCVLE,	{FRT}},

{"mffsce",	XMMF(63,583,0,1), XMMF_MASK|RB_MASK, POWER9, PPCVLE,	{FRT}},
{"mffscdrn",	XMMF(63,583,2,4), XMMF_MASK,         POWER9, PPCVLE,	{FRT, FRB}},
{"mffscdrni",	XMMF(63,583,2,5), XMMF_MASK|(3<<14), POWER9, PPCVLE,	{FRT, DRM}},
{"mffscrn",	XMMF(63,583,2,6), XMMF_MASK,         POWER9, PPCVLE,	{FRT, FRB}},
{"mffscrni",	XMMF(63,583,2,7), XMMF_MASK|(7<<13), POWER9, PPCVLE,	{FRT, RM}},
{"mffsl",	XMMF(63,583,3,0), XMMF_MASK|RB_MASK, POWER9, PPCVLE,	{FRT}},

{"dcmpuq",	X(63,642),	X_MASK,	     POWER6,	PPCVLE,		{BF, FRAp, FRBp}},

{"xscmpuqp",	X(63,644),	XBF_MASK,    PPCVSX3,	PPCVLE,		{BF, VA, VB}},

{"dtstsfq",	X(63,674),	X_MASK,	     POWER6,	PPCVLE,		{BF, FRA, FRBp}},
{"dtstsfiq",	X(63,675),	X_MASK|1<<22,POWER9,	PPCVLE,		{BF, UIM6, FRBp}},

{"xsmaxcqp",	X(63,676),	X_MASK,	     POWER10,	PPCVLE,		{VD, VA, VB}},

{"xststdcqp",	X(63,708),	X_MASK,	     PPCVSX3,	PPCVLE,		{BF, VB, DCMX}},

{"mtfsf",	XFL(63,711,0),	XFL_MASK, POWER6|PPCA2|PPC476, PPCVLE,	{FLM, FRB, XFL_L, W}},
{"mtfsf",	XFL(63,711,0),	XFL_MASK,    COM, POWER6|PPCA2|PPC476|PPCEFS|PPCVLE, {FLM, FRB}},
{"mtfsf.",	XFL(63,711,1),	XFL_MASK, POWER6|PPCA2|PPC476, PPCVLE,	{FLM, FRB, XFL_L, W}},
{"mtfsf.",	XFL(63,711,1),	XFL_MASK,    COM, POWER6|PPCA2|PPC476|PPCEFS|PPCVLE, {FLM, FRB}},

{"xsmincqp",	X(63,740),	X_MASK,	     POWER10,	PPCVLE,		{VD, VA, VB}},

{"drdpq",	XRC(63,770,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRBp}},
{"drdpq.",	XRC(63,770,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRBp}},

{"dcffixq",	XRC(63,802,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRB}},
{"dcffixq.",	XRC(63,802,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRB}},

{"xsabsqp",	XVA(63,804,0),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xsxexpqp",	XVA(63,804,2),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xsnabsqp",	XVA(63,804,8),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xsnegqp",	XVA(63,804,16),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xsxsigqp",	XVA(63,804,18),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xssqrtqp",	XVARC(63,804,27,0), XVA_MASK, PPCVSX3,	PPCVLE,		{VD, VB}},
{"xssqrtqpo",	XVARC(63,804,27,1), XVA_MASK, PPCVSX3,	PPCVLE,		{VD, VB}},

{"fctid",	XRC(63,814,0),	XRA_MASK,    PPC64,	PPCVLE,		{FRT, FRB}},
{"fctid",	XRC(63,814,0),	XRA_MASK,    PPC476,	PPCVLE,		{FRT, FRB}},
{"fctid.",	XRC(63,814,1),	XRA_MASK,    PPC64,	PPCVLE,		{FRT, FRB}},
{"fctid.",	XRC(63,814,1),	XRA_MASK,    PPC476,	PPCVLE,		{FRT, FRB}},

{"fctidz",	XRC(63,815,0),	XRA_MASK,    PPC64,	PPCVLE,		{FRT, FRB}},
{"fctidz",	XRC(63,815,0),	XRA_MASK,    PPC476,	PPCVLE,		{FRT, FRB}},
{"fctidz.",	XRC(63,815,1),	XRA_MASK,    PPC64,	PPCVLE,		{FRT, FRB}},
{"fctidz.",	XRC(63,815,1),	XRA_MASK,    PPC476,	PPCVLE,		{FRT, FRB}},

{"denbcdq",	XRC(63,834,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{S, FRTp, FRBp}},
{"denbcdq.",	XRC(63,834,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{S, FRTp, FRBp}},

{"xscvqpuqz",	XVA(63,836,0),	XVA_MASK,    POWER10,	PPCVLE,		{VD, VB}},
{"xscvqpuwz",	XVA(63,836,1),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xscvudqp",	XVA(63,836,2),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xscvuqqp",	XVA(63,836,3),	XVA_MASK,    POWER10,	PPCVLE,		{VD, VB}},
{"xscvqpsqz",	XVA(63,836,8),	XVA_MASK,    POWER10,	PPCVLE,		{VD, VB}},
{"xscvqpswz",	XVA(63,836,9),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xscvsdqp",	XVA(63,836,10),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xscvsqqp",	XVA(63,836,11),	XVA_MASK,    POWER10,	PPCVLE,		{VD, VB}},
{"xscvqpudz",	XVA(63,836,17),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xscvqpdp",	XVARC(63,836,20,0), XVA_MASK, PPCVSX3,	PPCVLE,		{VD, VB}},
{"xscvqpdpo",	XVARC(63,836,20,1), XVA_MASK, PPCVSX3,	PPCVLE,		{VD, VB}},
{"xscvdpqp",	XVA(63,836,22),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},
{"xscvqpsdz",	XVA(63,836,25),	XVA_MASK,    PPCVSX3,	PPCVLE,		{VD, VB}},

{"fmrgow",	X(63,838),	X_MASK,	     PPCVSX2,	PPCVLE,		{FRT, FRA, FRB}},

{"fcfid",	XRC(63,846,0),	XRA_MASK,    PPC64,	PPCVLE,		{FRT, FRB}},
{"fcfid",	XRC(63,846,0),	XRA_MASK,    PPC476,	PPCVLE,		{FRT, FRB}},
{"fcfid.",	XRC(63,846,1),	XRA_MASK,    PPC64,	PPCVLE,		{FRT, FRB}},
{"fcfid.",	XRC(63,846,1),	XRA_MASK,    PPC476,	PPCVLE,		{FRT, FRB}},

{"diexq",	XRC(63,866,0), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRA, FRBp}},
{"diexq.",	XRC(63,866,1), X_MASK|Q_MASK, POWER6,	PPCVLE,		{FRTp, FRA, FRBp}},

{"xsiexpqp",	X(63,868),	X_MASK,	     PPCVSX3,	PPCVLE,		{VD, VA, VB}},

{"fctidu",	XRC(63,942,0),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},
{"fctidu.",	XRC(63,942,1),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},

{"fctiduz",	XRC(63,943,0),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},
{"fctiduz.",	XRC(63,943,1),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},

{"fmrgew",	X(63,966),	X_MASK,	     PPCVSX2,	PPCVLE,		{FRT, FRA, FRB}},

{"fcfidu",	XRC(63,974,0),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},
{"fcfidu.",	XRC(63,974,1),	XRA_MASK, POWER7|PPCA2,	PPCVLE,		{FRT, FRB}},

{"dcffixqq",	XVA(63,994,0),	XVA_MASK,    POWER10,	PPCVLE,		{FRTp, VB}},
{"dctfixqq",	XVA(63,994,1),	XVA_MASK,    POWER10,	PPCVLE,		{VD, FRBp}},
};

const unsigned int powerpc_num_opcodes = ARRAY_SIZE (powerpc_opcodes);

/* The opcode table for 8-byte prefix instructions.

   The format of this opcode table is the same as the main opcode table.  */

const struct powerpc_opcode prefix_opcodes[] = {
{"pnop",	  PMRR,		       PREFIX_MASK,	POWER10, 0,	{0}},
{"pli",		  PMLS|OP(14),	       P_DRAPCREL_MASK,	POWER10, EXT,	{RT, SI34}},
{"pla",		  PMLS|OP(14),	       P_D_MASK,	POWER10, EXT,	{RT, D34, PRA0, PCREL1}},
{"paddi",	  PMLS|OP(14),	       P_D_MASK,	POWER10, 0,	{RT, RA0, SI34, PCREL}},
{"psubi",	  PMLS|OP(14),	       P_D_MASK,	POWER10, EXT,	{RT, RA0, NSI34, PCREL}},
{"xxsplti32dx",	  P8RR|VSOP(32,0),     P_VSI_MASK,	POWER10, 0,	{XTS, IX, IMM32}},
{"xxspltidp",	  P8RR|VSOP(32,2),     P_VS_MASK,	POWER10, 0,	{XTS, IMM32}},
{"xxspltiw",	  P8RR|VSOP(32,3),     P_VS_MASK,	POWER10, 0,	{XTS, IMM32}},
{"plwz",	  PMLS|OP(32),	       P_D_MASK,	POWER10, 0,	{RT, D34, PRA0, PCREL}},
{"xxblendvb",	  P8RR|XX4(33,0),      P_XX4_MASK,	POWER10, 0,	{XT6, XA6, XB6, XC6}},
{"xxblendvh",	  P8RR|XX4(33,1),      P_XX4_MASK,	POWER10, 0,	{XT6, XA6, XB6, XC6}},
{"xxblendvw",	  P8RR|XX4(33,2),      P_XX4_MASK,	POWER10, 0,	{XT6, XA6, XB6, XC6}},
{"xxblendvd",	  P8RR|XX4(33,3),      P_XX4_MASK,	POWER10, 0,	{XT6, XA6, XB6, XC6}},
{"xxpermx",	  P8RR|XX4(34,0),      P_UXX4_MASK,	POWER10, 0,	{XT6, XA6, XB6, XC6, UIM3}},
{"xxeval",	  P8RR|XX4(34,1),      P_U8XX4_MASK,	POWER10, 0,	{XT6, XA6, XB6, XC6, UIM8}},
{"plbz",	  PMLS|OP(34),	       P_D_MASK,	POWER10, 0,	{RT, D34, PRA0, PCREL}},
{"pstw",	  PMLS|OP(36),	       P_D_MASK,	POWER10, 0,	{RS, D34, PRA0, PCREL}},
{"pstb",	  PMLS|OP(38),	       P_D_MASK,	POWER10, 0,	{RS, D34, PRA0, PCREL}},
{"plhz",	  PMLS|OP(40),	       P_D_MASK,	POWER10, 0,	{RT, D34, PRA0, PCREL}},
{"plwa",	  P8LS|OP(41),	       P_D_MASK,	POWER10, 0,	{RT, D34, PRA0, PCREL}},
{"plxsd",	  P8LS|OP(42),	       P_D_MASK,	POWER10, 0,	{VD, D34, PRA0, PCREL}},
{"plha",	  PMLS|OP(42),	       P_D_MASK,	POWER10, 0,	{RT, D34, PRA0, PCREL}},
{"plxssp",	  P8LS|OP(43),	       P_D_MASK,	POWER10, 0,	{VD, D34, PRA0, PCREL}},
{"psth",	  PMLS|OP(44),	       P_D_MASK,	POWER10, 0,	{RS, D34, PRA0, PCREL}},
{"pstxsd",	  P8LS|OP(46),	       P_D_MASK,	POWER10, 0,	{VS, D34, PRA0, PCREL}},
{"pstxssp",	  P8LS|OP(47),	       P_D_MASK,	POWER10, 0,	{VS, D34, PRA0, PCREL}},
{"plfs",	  PMLS|OP(48),	       P_D_MASK,	POWER10, 0,	{FRT, D34, PRA0, PCREL}},
{"plxv",	  P8LS|OP(50),	       P_D_MASK&~OP(1),	POWER10, 0,	{XTOP, D34, PRA0, PCREL}},
{"plfd",	  PMLS|OP(50),	       P_D_MASK,	POWER10, 0,	{FRT, D34, PRA0, PCREL}},
{"pstfs",	  PMLS|OP(52),	       P_D_MASK,	POWER10, 0,	{FRS, D34, PRA0, PCREL}},
{"pstxv",	  P8LS|OP(54),	       P_D_MASK&~OP(1),	POWER10, 0,	{XTOP, D34, PRA0, PCREL}},
{"pstfd",	  PMLS|OP(54),	       P_D_MASK,	POWER10, 0,	{FRS, D34, PRA0, PCREL}},
{"plq",		  P8LS|OP(56),	       P_D_MASK,	POWER10, 0,	{RTQ, D34, PRAQ, PCREL}},
{"pld",		  P8LS|OP(57),	       P_D_MASK,	POWER10, 0,	{RT, D34, PRA0, PCREL}},
{"plxvp",	  P8LS|OP(58),	       P_D_MASK,	POWER10, 0,	{XTP, D34, PRA0, PCREL}},
{"pmdmxvi8ger4pp",PMMIRR|XX3(59,2),    P_GER4_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK4}},
{"pmxvi8ger4pp",  PMMIRR|XX3(59,2),    P_GER4_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK4}},
{"pmdmxvi8ger4",  PMMIRR|XX3(59,3),    P_GER4_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK4}},
{"pmxvi8ger4",	  PMMIRR|XX3(59,3),    P_GER4_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK4}},
{"pmdmxvi8gerx4pp",PMMIRR|XX3(59,10),  P_GERX4_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK4}},
{"pmdmxvi8gerx4", PMMIRR|XX3(59,11),   P_GERX4_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK4}},
{"pmdmxvf16ger2pp",PMMIRR|XX3(59,18),  P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvf16ger2pp", PMMIRR|XX3(59,18),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvf16ger2", PMMIRR|XX3(59,19),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvf16ger2",	  PMMIRR|XX3(59,19),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvf32gerpp",PMMIRR|XX3(59,26),   P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmxvf32gerpp",  PMMIRR|XX3(59,26),   P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmdmxvf32ger",  PMMIRR|XX3(59,27),   P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmxvf32ger",	  PMMIRR|XX3(59,27),   P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmdmxvi4ger8pp",PMMIRR|XX3(59,34),   P_GER8_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK8}},
{"pmxvi4ger8pp",  PMMIRR|XX3(59,34),   P_GER8_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK8}},
{"pmdmxvi4ger8",  PMMIRR|XX3(59,35),   P_GER8_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK8}},
{"pmxvi4ger8",	  PMMIRR|XX3(59,35),   P_GER8_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK8}},
{"pmdmxvi16ger2spp",PMMIRR|XX3(59,42), P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvi16ger2spp",PMMIRR|XX3(59,42),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvi16ger2s",PMMIRR|XX3(59,43),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvi16ger2s",  PMMIRR|XX3(59,43),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvbf16ger2pp",PMMIRR|XX3(59,50), P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvbf16ger2pp",PMMIRR|XX3(59,50),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvbf16ger2",PMMIRR|XX3(59,51),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvbf16ger2",  PMMIRR|XX3(59,51),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvf64gerpp",PMMIRR|XX3(59,58),   P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmxvf64gerpp",  PMMIRR|XX3(59,58),   P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmdmxvf64ger",  PMMIRR|XX3(59,59),   P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmxvf64ger",	  PMMIRR|XX3(59,59),   P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmdmxvf16gerx2pp",PMMIRR|XX3(59,66), P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvf16gerx2",PMMIRR|XX3(59,67),   P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvbf16gerx2pp",PMMIRR|XX3(59,74),P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvi16ger2", PMMIRR|XX3(59,75),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvi16ger2",   PMMIRR|XX3(59,75),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvf16ger2np",PMMIRR|XX3(59,82),  P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvf16ger2np", PMMIRR|XX3(59,82),   P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvf16gerx2np",PMMIRR|XX3(59,83), P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvf32gernp",PMMIRR|XX3(59,90),   P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmxvf32gernp",  PMMIRR|XX3(59,90),   P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmdmxvbf16gerx2",PMMIRR|XX3(59,91),  P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvi8gerx4spp",PMMIRR|XX3(59,98), P_GERX4_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK4}},
{"pmdmxvi8ger4spp",PMMIRR|XX3(59,99),  P_GER4_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK4}},
{"pmxvi8ger4spp", PMMIRR|XX3(59,99),   P_GER4_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK4}},
{"pmdmxvi16ger2pp",PMMIRR|XX3(59,107), P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvi16ger2pp", PMMIRR|XX3(59,107),  P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvbf16ger2np",PMMIRR|XX3(59,114),P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvbf16ger2np",PMMIRR|XX3(59,114),  P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvbf16gerx2np",PMMIRR|XX3(59,115),P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvf64gernp",PMMIRR|XX3(59,122),  P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmxvf64gernp",  PMMIRR|XX3(59,122),  P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmdmxvf16ger2pn",PMMIRR|XX3(59,146), P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvf16ger2pn", PMMIRR|XX3(59,146),  P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvf16gerx2pn",PMMIRR|XX3(59,147),P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvf32gerpn",PMMIRR|XX3(59,154),  P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmxvf32gerpn",  PMMIRR|XX3(59,154),  P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmdmxvbf16ger2pn",PMMIRR|XX3(59,178),P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvbf16ger2pn",PMMIRR|XX3(59,178),  P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvbf16gerx2pn",PMMIRR|XX3(59,179),P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvf64gerpn",PMMIRR|XX3(59,186),  P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmxvf64gerpn",  PMMIRR|XX3(59,186),  P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmdmxvf16gerx2nn",PMMIRR|XX3(59,202),P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvf16ger2nn",PMMIRR|XX3(59,210), P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvf16ger2nn", PMMIRR|XX3(59,210),  P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvf32gernn",PMMIRR|XX3(59,218),  P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmxvf32gernn",  PMMIRR|XX3(59,218),  P_GER_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK}},
{"pmdmxvbf16gerx2nn",PMMIRR|XX3(59,234),P_GERX2_MASK,	FUTURE,  0,	{DMR, XA5p, XB6, XMSK8, YMSK, PMSK2}},
{"pmdmxvbf16ger2nn",PMMIRR|XX3(59,242),P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmxvbf16ger2nn",PMMIRR|XX3(59,242),  P_GER2_MASK,	POWER10, 0,	{ACC, XA6a, XB6a, XMSK, YMSK, PMSK2}},
{"pmdmxvf64gernn",PMMIRR|XX3(59,250),  P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pmxvf64gernn",  PMMIRR|XX3(59,250),  P_GER64_MASK,	POWER10, 0,	{ACC, XA6ap, XB6a, XMSK, YMSK2}},
{"pstq",	  P8LS|OP(60),	       P_D_MASK,	POWER10, 0,	{RSQ, D34, PRA0, PCREL}},
{"pstd",	  P8LS|OP(61),	       P_D_MASK,	POWER10, 0,	{RS, D34, PRA0, PCREL}},
{"pstxvp",	  P8LS|OP(62),	       P_D_MASK,	POWER10, 0,	{XSP, D34, PRA0, PCREL}},
};

const unsigned int prefix_num_opcodes = ARRAY_SIZE (prefix_opcodes);

/* The VLE opcode table.

   The format of this opcode table is the same as the main opcode table.  */

const struct powerpc_opcode vle_opcodes[] = {
{"se_illegal",	C(0),		C_MASK,		PPCVLE,	0,		{}},
{"se_isync",	C(1),		C_MASK,		PPCVLE,	0,		{}},
{"se_sc",	C(2),		C_MASK,		PPCVLE,	0,		{}},
{"se_blr",	C_LK(2,0),	C_LK_MASK,	PPCVLE,	0,		{}},
{"se_blrl",	C_LK(2,1),	C_LK_MASK,	PPCVLE,	0,		{}},
{"se_bctr",	C_LK(3,0),	C_LK_MASK,	PPCVLE,	0,		{}},
{"se_bctrl",	C_LK(3,1),	C_LK_MASK,	PPCVLE,	0,		{}},
{"se_rfi",	C(8),		C_MASK,		PPCVLE,	0,		{}},
{"se_rfci",	C(9),		C_MASK,		PPCVLE,	0,		{}},
{"se_rfdi",	C(10),		C_MASK,		PPCVLE,	0,		{}},
/* PPCRFMCI in the following does not enable the instruction for any
   PPC_OPCODE_RFMCI supporting cpu as vle_opcodes are all added to the
   assembler hash table or searched by the disassembler under control
   of PPC_OPCODE_VLE.  It's there to set apuinfo.  */
{"se_rfmci",	C(11),		C_MASK, PPCRFMCI|PPCVLE, 0,		{}},
{"se_rfgi",	C(12),		C_MASK,		PPCVLE,	0,		{}},
{"se_not",	SE_R(0,2),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_neg",	SE_R(0,3),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_mflr",	SE_R(0,8),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_mtlr",	SE_R(0,9),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_mfctr",	SE_R(0,10),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_mtctr",	SE_R(0,11),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_extzb",	SE_R(0,12),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_extsb",	SE_R(0,13),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_extzh",	SE_R(0,14),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_extsh",	SE_R(0,15),	SE_R_MASK,	PPCVLE,	0,		{RX}},
{"se_mr",	SE_RR(0,1),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_mtar",	SE_RR(0,2),	SE_RR_MASK,	PPCVLE,	0,		{ARX, RY}},
{"se_mfar",	SE_RR(0,3),	SE_RR_MASK,	PPCVLE,	0,		{RX, ARY}},
{"se_add",	SE_RR(1,0),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_mullw",	SE_RR(1,1),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_sub",	SE_RR(1,2),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_subf",	SE_RR(1,3),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_cmp",	SE_RR(3,0),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_cmpl",	SE_RR(3,1),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_cmph",	SE_RR(3,2),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_cmphl",	SE_RR(3,3),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},

/* by major opcode */
{"e_cmpi",	SCI8BF(6,0,21),	SCI8BF_MASK,	PPCVLE,	0,		{CRD32, RA, SCLSCI8}},
{"e_cmpwi",	SCI8BF(6,0,21),	SCI8BF_MASK,	PPCVLE,	0,		{CRD32, RA, SCLSCI8}},
{"e_cmpli",	SCI8BF(6,1,21),	SCI8BF_MASK,	PPCVLE,	0,		{CRD32, RA, SCLSCI8}},
{"e_cmplwi",	SCI8BF(6,1,21),	SCI8BF_MASK,	PPCVLE,	0,		{CRD32, RA, SCLSCI8}},
{"e_addi",	SCI8(6,16),	SCI8_MASK,	PPCVLE,	0,		{RT, RA, SCLSCI8}},
{"e_subi",	SCI8(6,16),	SCI8_MASK,	PPCVLE,	0,		{RT, RA, SCLSCI8N}},
{"e_addi.",	SCI8(6,17),	SCI8_MASK,	PPCVLE,	0,		{RT, RA, SCLSCI8}},
{"e_addic",	SCI8(6,18),	SCI8_MASK,	PPCVLE,	0,		{RT, RA, SCLSCI8}},
{"e_subic",	SCI8(6,18),	SCI8_MASK,	PPCVLE,	EXT,		{RT, RA, SCLSCI8N}},
{"e_addic.",	SCI8(6,19),	SCI8_MASK,	PPCVLE,	0,		{RT, RA, SCLSCI8}},
{"e_subic.",	SCI8(6,19),	SCI8_MASK,	PPCVLE,	EXT,		{RT, RA, SCLSCI8N}},
{"e_mulli",	SCI8(6,20),	SCI8_MASK,	PPCVLE,	0,		{RT, RA, SCLSCI8}},
{"e_subfic",	SCI8(6,22),	SCI8_MASK,	PPCVLE,	0,		{RT, RA, SCLSCI8}},
{"e_subfic.",	SCI8(6,23),	SCI8_MASK,	PPCVLE,	0,		{RT, RA, SCLSCI8}},
{"e_andi",	SCI8(6,24),	SCI8_MASK,	PPCVLE,	0,		{RA, RS, SCLSCI8}},
{"e_andi.",	SCI8(6,25),	SCI8_MASK,	PPCVLE,	0,		{RA, RS, SCLSCI8}},
{"e_nop",	SCI8(6,26),	0xffffffff,	PPCVLE,	EXT,		{0}},
{"e_ori",	SCI8(6,26),	SCI8_MASK,	PPCVLE,	0,		{RA, RS, SCLSCI8}},
{"e_ori.",	SCI8(6,27),	SCI8_MASK,	PPCVLE,	0,		{RA, RS, SCLSCI8}},
{"e_xori",	SCI8(6,28),	SCI8_MASK,	PPCVLE,	0,		{RA, RS, SCLSCI8}},
{"e_xori.",	SCI8(6,29),	SCI8_MASK,	PPCVLE,	0,		{RA, RS, SCLSCI8}},
{"e_lbzu",	OPVUP(6,0),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_lhau",	OPVUP(6,3),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_lhzu",	OPVUP(6,1),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_lmw",	OPVUP(6,8),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_lwzu",	OPVUP(6,2),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_stbu",	OPVUP(6,4),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_sthu",	OPVUP(6,5),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_stwu",	OPVUP(6,6),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_stmw",	OPVUP(6,9),	OPVUP_MASK,	PPCVLE,	0,		{RT, D8, RA0}},
{"e_lmvgprw",	OPVUPRT(6,16,0),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_ldmvgprw",	OPVUPRT(6,16,0),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_stmvgprw",	OPVUPRT(6,17,0),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_lmvsprw",	OPVUPRT(6,16,1),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_ldmvsprw",	OPVUPRT(6,16,1),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_stmvsprw",	OPVUPRT(6,17,1),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_lmvsrrw",	OPVUPRT(6,16,4),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_ldmvsrrw",	OPVUPRT(6,16,4),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_stmvsrrw",	OPVUPRT(6,17,4),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_lmvcsrrw",	OPVUPRT(6,16,5),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_ldmvcsrrw",	OPVUPRT(6,16,5),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_stmvcsrrw",	OPVUPRT(6,17,5),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_lmvdsrrw",	OPVUPRT(6,16,6),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_ldmvdsrrw",	OPVUPRT(6,16,6),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_stmvdsrrw",	OPVUPRT(6,17,6),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_lmvmcsrrw",	OPVUPRT(6,16,7),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_stmvmcsrrw",OPVUPRT(6,17,7),OPVUPRT_MASK,	PPCVLE,	0,		{D8, RA0}},
{"e_add16i",	OP(7),		OP_MASK,	PPCVLE,	0,		{RT, RA, SI}},
{"e_la",	OP(7),		OP_MASK,	PPCVLE,	EXT,		{RT, D, RA0}},
{"e_sub16i",	OP(7),		OP_MASK,	PPCVLE,	EXT,		{RT, RA, NSI}},

{"se_addi",	SE_IM5(8,0),	SE_IM5_MASK,	PPCVLE,	0,		{RX, OIMM5}},
{"se_cmpli",	SE_IM5(8,1),	SE_IM5_MASK,	PPCVLE,	0,		{RX, OIMM5}},
{"se_subi",	SE_IM5(9,0),	SE_IM5_MASK,	PPCVLE,	0,		{RX, OIMM5}},
{"se_subi.",	SE_IM5(9,1),	SE_IM5_MASK,	PPCVLE,	0,		{RX, OIMM5}},
{"se_cmpi",	SE_IM5(10,1),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},
{"se_bmaski",	SE_IM5(11,0),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},
{"se_andi",	SE_IM5(11,1),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},

{"e_lbz",	OP(12),		OP_MASK,	PPCVLE,	0,		{RT, D, RA0}},
{"e_stb",	OP(13),		OP_MASK,	PPCVLE,	0,		{RT, D, RA0}},
{"e_lha",	OP(14),		OP_MASK,	PPCVLE,	0,		{RT, D, RA0}},

{"se_srw",	SE_RR(16,0),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_sraw",	SE_RR(16,1),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_slw",	SE_RR(16,2),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_nop",	SE_RR(17,0),	0xffff,		PPCVLE,	EXT,		{0}},
{"se_or",	SE_RR(17,0),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_andc",	SE_RR(17,1),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_and",	SE_RR(17,2),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_and.",	SE_RR(17,3),	SE_RR_MASK,	PPCVLE,	0,		{RX, RY}},
{"se_li",	IM7(9),		IM7_MASK,	PPCVLE,	0,		{RX, UI7}},

{"e_lwz",	OP(20),		OP_MASK,	PPCVLE,	0,		{RT, D, RA0}},
{"e_stw",	OP(21),		OP_MASK,	PPCVLE,	0,		{RT, D, RA0}},
{"e_lhz",	OP(22),		OP_MASK,	PPCVLE,	0,		{RT, D, RA0}},
{"e_sth",	OP(23),		OP_MASK,	PPCVLE,	0,		{RT, D, RA0}},

{"se_bclri",	SE_IM5(24,0),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},
{"se_bgeni",	SE_IM5(24,1),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},
{"se_bseti",	SE_IM5(25,0),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},
{"se_btsti",	SE_IM5(25,1),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},
{"se_srwi",	SE_IM5(26,0),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},
{"se_srawi",	SE_IM5(26,1),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},
{"se_slwi",	SE_IM5(27,0),	SE_IM5_MASK,	PPCVLE,	0,		{RX, UI5}},

{"e_lis",	I16L(28,28),	I16L_MASK,	PPCVLE,	0,		{RD, VLEUIMML}},
{"e_and2is.",	I16L(28,29),	I16L_MASK,	PPCVLE,	0,		{RD, VLEUIMML}},
{"e_or2is",	I16L(28,26),	I16L_MASK,	PPCVLE,	0,		{RD, VLEUIMML}},
{"e_and2i.",	I16L(28,25),	I16L_MASK,	PPCVLE,	0,		{RD, VLEUIMML}},
{"e_or2i",	I16L(28,24),	I16L_MASK,	PPCVLE,	0,		{RD, VLEUIMML}},
{"e_cmphl16i",	IA16(28,23),	IA16_MASK,	PPCVLE,	0,		{RA, VLEUIMM}},
{"e_cmph16i",	IA16(28,22),	IA16_MASK,	PPCVLE,	0,		{RA, VLESIMM}},
{"e_cmpl16i",	I16A(28,21),	I16A_MASK,	PPCVLE,	0,		{RA, VLEUIMM}},
{"e_mull2i",	I16A(28,20),	I16A_MASK,	PPCVLE,	0,		{RA, VLESIMM}},
{"e_cmp16i",	IA16(28,19),	IA16_MASK,	PPCVLE,	0,		{RA, VLESIMM}},
{"e_sub2is",	I16A(28,18),	I16A_MASK,	PPCVLE,	EXT,		{RA, VLENSIMM}},
{"e_add2is",	I16A(28,18),	I16A_MASK,	PPCVLE,	0,		{RA, VLESIMM}},
{"e_sub2i.",	I16A(28,17),	I16A_MASK,	PPCVLE,	EXT,		{RA, VLENSIMM}},
{"e_add2i.",	I16A(28,17),	I16A_MASK,	PPCVLE,	0,		{RA, VLESIMM}},
{"e_li",	LI20(28,0),	LI20_MASK,	PPCVLE,	0,		{RT, IMM20}},
{"e_rlwimi",	M(29,0),	M_MASK,		PPCVLE,	0,		{RA, RS, SH, MB, ME}},
{"e_inslwi",	M(29,0),	M_MASK,		PPCVLE, EXT,		{RA, RS, ILWn, ILWb}},
{"e_insrwi",	M(29,0),	M_MASK,		PPCVLE, EXT,		{RA, RS, IRWn, IRWb}},
{"e_rotlwi",	MME(29,31,1),	MMBME_MASK,	PPCVLE, EXT,		{RA, RS, SH}},
{"e_rotrwi",	MME(29,31,1),	MMBME_MASK,	PPCVLE, EXT,		{RA, RS, RRWn}},
{"e_clrlwi",	MME(29,31,1),	MSHME_MASK,	PPCVLE, EXT,		{RA, RS, MB}},
{"e_clrrwi",	M(29,1),	MSHMB_MASK,	PPCVLE, EXT,		{RA, RS, CRWn}},
{"e_rlwinm",	M(29,1),	M_MASK,		PPCVLE,	0,		{RA, RS, SH, MBE, ME}},
{"e_extlwi",	M(29,1),	MMB_MASK,	PPCVLE, EXT,		{RA, RS, ELWn, SH}},
{"e_extrwi",	MME(29,31,1),	MME_MASK,	PPCVLE, EXT,		{RA, RS, ERWn, ERWb}},
{"e_clrlslwi",	M(29,1),	M_MASK,		PPCVLE, EXT,		{RA, RS, CSLWb, CSLWn}},
{"e_b",		BD24(30,0,0),	BD24_MASK,	PPCVLE,	0,		{B24}},
{"e_bl",	BD24(30,0,1),	BD24_MASK,	PPCVLE,	0,		{B24}},
{"e_bdnz",	EBD15(30,8,BO32DNZ,0),	EBD15_MASK, PPCVLE, EXT,	{B15}},
{"e_bdnzl",	EBD15(30,8,BO32DNZ,1),	EBD15_MASK, PPCVLE, EXT,	{B15}},
{"e_bdz",	EBD15(30,8,BO32DZ,0),	EBD15_MASK, PPCVLE, EXT,	{B15}},
{"e_bdzl",	EBD15(30,8,BO32DZ,1),	EBD15_MASK, PPCVLE, EXT,	{B15}},
{"e_bge",	EBD15BI(30,8,BO32F,CBLT,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bgel",	EBD15BI(30,8,BO32F,CBLT,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bnl",	EBD15BI(30,8,BO32F,CBLT,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bnll",	EBD15BI(30,8,BO32F,CBLT,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_blt",	EBD15BI(30,8,BO32T,CBLT,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bltl",	EBD15BI(30,8,BO32T,CBLT,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bgt",	EBD15BI(30,8,BO32T,CBGT,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bgtl",	EBD15BI(30,8,BO32T,CBGT,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_ble",	EBD15BI(30,8,BO32F,CBGT,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_blel",	EBD15BI(30,8,BO32F,CBGT,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bng",	EBD15BI(30,8,BO32F,CBGT,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bngl",	EBD15BI(30,8,BO32F,CBGT,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bne",	EBD15BI(30,8,BO32F,CBEQ,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bnel",	EBD15BI(30,8,BO32F,CBEQ,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_beq",	EBD15BI(30,8,BO32T,CBEQ,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_beql",	EBD15BI(30,8,BO32T,CBEQ,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bso",	EBD15BI(30,8,BO32T,CBSO,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bsol",	EBD15BI(30,8,BO32T,CBSO,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bun",	EBD15BI(30,8,BO32T,CBSO,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bunl",	EBD15BI(30,8,BO32T,CBSO,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bns",	EBD15BI(30,8,BO32F,CBSO,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bnsl",	EBD15BI(30,8,BO32F,CBSO,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bnu",	EBD15BI(30,8,BO32F,CBSO,0), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bnul",	EBD15BI(30,8,BO32F,CBSO,1), EBD15BI_MASK, PPCVLE, EXT,	{CRS,B15}},
{"e_bc",	BD15(30,8,0),	BD15_MASK,	PPCVLE,	0,		{BO32, BI32, B15}},
{"e_bcl",	BD15(30,8,1),	BD15_MASK,	PPCVLE,	0,		{BO32, BI32, B15}},

{"e_bf",	EBD15(30,8,BO32F,0), EBD15_MASK, PPCVLE, EXT,		{BI32,B15}},
{"e_bfl",	EBD15(30,8,BO32F,1), EBD15_MASK, PPCVLE, EXT,		{BI32,B15}},
{"e_bt",	EBD15(30,8,BO32T,0), EBD15_MASK, PPCVLE, EXT,		{BI32,B15}},
{"e_btl",	EBD15(30,8,BO32T,1), EBD15_MASK, PPCVLE, EXT,		{BI32,B15}},

{"e_cmph",	X(31,14),	X_MASK,		PPCVLE,	0,		{CRD, RA, RB}},
{"e_sc",	X(31,36),	XRTRA_MASK,	PPCVLE,	0,		{ELEV}},
{"e_cmphl",	X(31,46),	X_MASK,		PPCVLE,	0,		{CRD, RA, RB}},
{"e_crandc",	XL(31,129),	XL_MASK,	PPCVLE,	0,		{BT, BA, BB}},
{"e_crnand",	XL(31,225),	XL_MASK,	PPCVLE,	0,		{BT, BA, BB}},
{"e_crnot",	XL(31,33),	XL_MASK,	PPCVLE,	EXT,		{BT, BAB}},
{"e_crnor",	XL(31,33),	XL_MASK,	PPCVLE,	0,		{BT, BA, BB}},
{"e_crclr",	XL(31,193),	XL_MASK,	PPCVLE,	EXT,		{BTAB}},
{"e_crxor",	XL(31,193),	XL_MASK,	PPCVLE,	0,		{BT, BA, BB}},
{"e_mcrf",	XL(31,16),	XL_MASK,	PPCVLE,	0,		{CRD, CR}},
{"e_slwi",	EX(31,112),	EX_MASK,	PPCVLE,	0,		{RA, RS, SH}},
{"e_slwi.",	EX(31,113),	EX_MASK,	PPCVLE,	0,		{RA, RS, SH}},

{"e_crand",	XL(31,257),	XL_MASK,	PPCVLE,	0,		{BT, BA, BB}},

{"e_rlw",	EX(31,560),	EX_MASK,	PPCVLE,	0,		{RA, RS, RB}},
{"e_rlw.",	EX(31,561),	EX_MASK,	PPCVLE,	0,		{RA, RS, RB}},

{"e_crset",	XL(31,289),	XL_MASK,	PPCVLE,	EXT,		{BTAB}},
{"e_creqv",	XL(31,289),	XL_MASK,	PPCVLE,	0,		{BT, BA, BB}},

{"e_rlwi",	EX(31,624),	EX_MASK,	PPCVLE,	0,		{RA, RS, SH}},
{"e_rlwi.",	EX(31,625),	EX_MASK,	PPCVLE,	0,		{RA, RS, SH}},

{"e_crorc",	XL(31,417),	XL_MASK,	PPCVLE,	0,		{BT, BA, BB}},

{"e_crmove",	XL(31,449),	XL_MASK,	PPCVLE,	EXT,		{BT, BAB}},
{"e_cror",	XL(31,449),	XL_MASK,	PPCVLE,	0,		{BT, BA, BB}},

{"mtmas1",	XSPR(31,467,625), XSPR_MASK,	PPCVLE,	EXT,		{RS}},

{"e_srwi",	EX(31,1136),	EX_MASK,	PPCVLE,	0,		{RA, RS, SH}},
{"e_srwi.",	EX(31,1137),	EX_MASK,	PPCVLE,	0,		{RA, RS, SH}},

{"se_lbz",	SD4(8),		SD4_MASK,	PPCVLE,	0,		{RZ, SE_SD, RX}},

{"se_stb",	SD4(9),		SD4_MASK,	PPCVLE,	0,		{RZ, SE_SD, RX}},

{"se_lhz",	SD4(10),	SD4_MASK,	PPCVLE,	0,		{RZ, SE_SDH, RX}},

{"se_sth",	SD4(11),	SD4_MASK,	PPCVLE,	0,		{RZ, SE_SDH, RX}},

{"se_lwz",	SD4(12),	SD4_MASK,	PPCVLE,	0,		{RZ, SE_SDW, RX}},

{"se_stw",	SD4(13),	SD4_MASK,	PPCVLE,	0,		{RZ, SE_SDW, RX}},

{"se_bge",	EBD8IO(28,0,0),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bnl",	EBD8IO(28,0,0),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_ble",	EBD8IO(28,0,1),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bng",	EBD8IO(28,0,1),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bne",	EBD8IO(28,0,2),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bns",	EBD8IO(28,0,3),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bnu",	EBD8IO(28,0,3),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bf",	EBD8IO(28,0,0),	EBD8IO2_MASK,	PPCVLE,	EXT,		{BI16, B8}},
{"se_blt",	EBD8IO(28,1,0),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bgt",	EBD8IO(28,1,1),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_beq",	EBD8IO(28,1,2),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bso",	EBD8IO(28,1,3),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bun",	EBD8IO(28,1,3),	EBD8IO3_MASK,	PPCVLE,	EXT,		{B8}},
{"se_bt",	EBD8IO(28,1,0),	EBD8IO2_MASK,	PPCVLE,	EXT,		{BI16, B8}},
{"se_bc",	BD8IO(28),	BD8IO_MASK,	PPCVLE,	0,		{BO16, BI16, B8}},
{"se_b",	BD8(58,0,0),	BD8_MASK,	PPCVLE,	0,		{B8}},
{"se_bl",	BD8(58,0,1),	BD8_MASK,	PPCVLE,	0,		{B8}},
};

const unsigned int vle_num_opcodes = ARRAY_SIZE (vle_opcodes);

const struct powerpc_opcode lsp_opcodes[] = {
{"zvaddih",	      VX(4, 0x200), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM}},
{"zvsubifh",	      VX(4, 0x201), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM}},
{"zvaddh",	      VX(4, 0x204), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfh",	      VX(4, 0x205), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvaddsubfh",	      VX(4, 0x206), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfaddh",	      VX(4, 0x207), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvaddhx",	      VX(4, 0x20C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfhx",	      VX(4, 0x20D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvaddsubfhx",	      VX(4, 0x20E), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfaddhx",	      VX(4, 0x20F), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zaddwus",	      VX(4, 0x210), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zsubfwus",	      VX(4, 0x211), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zaddwss",	      VX(4, 0x212), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zsubfwss",	      VX(4, 0x213), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvaddhus",	      VX(4, 0x214), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfhus",	      VX(4, 0x215), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvaddhss",	      VX(4, 0x216), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfhss",	      VX(4, 0x217), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvaddsubfhss",      VX(4, 0x21A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfaddhss",      VX(4, 0x21B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvaddhxss",	      VX(4, 0x21C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfhxss",	      VX(4, 0x21D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvaddsubfhxss",     VX(4, 0x21E), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfaddhxss",     VX(4, 0x21F), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zaddheuw",	      VX(4, 0x220), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zsubfheuw",	      VX(4, 0x221), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zaddhesw",	      VX(4, 0x222), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zsubfhesw",	      VX(4, 0x223), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zaddhouw",	      VX(4, 0x224), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zsubfhouw",	      VX(4, 0x225), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zaddhosw",	      VX(4, 0x226), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zsubfhosw",	      VX(4, 0x227), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmergehih",	      VX(4, 0x22C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmergeloh",	      VX(4, 0x22D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmergehiloh",      VX(4, 0x22E), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmergelohih",      VX(4, 0x22F), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvcmpgthu",	      VX(4, 0x230), VX_MASK,	PPCLSP, 0,		{CRFD, RA, RB}},
{"zvcmpgths",	      VX(4, 0x230), VX_MASK,	PPCLSP, 0,		{CRFD, RA, RB}},
{"zvcmplthu",	      VX(4, 0x231), VX_MASK,	PPCLSP, 0,		{CRFD, RA, RB}},
{"zvcmplths",	      VX(4, 0x231), VX_MASK,	PPCLSP, 0,		{CRFD, RA, RB}},
{"zvcmpeqh",	      VX(4, 0x232), VX_MASK,	PPCLSP, 0,		{CRFD, RA, RB}},
{"zpkswgshfrs",	      VX(4, 0x238), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zpkswgswfrs",	      VX(4, 0x239), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvpkshgwshfrs",     VX(4, 0x23A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvpkswshfrs",	      VX(4, 0x23B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvpkswuhs",	      VX(4, 0x23C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvpkswshs",	      VX(4, 0x23D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvpkuwuhs",	      VX(4, 0x23E), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsplatih",	      VX_LSP(4, 0x23F), VX_LSP_MASK, PPCLSP, 0,		{RD, SIMM}},
{"zvsplatfih",	      VX_LSP(4, 0xA3F), VX_LSP_MASK, PPCLSP, 0,		{RD, SIMM}},
{"zcntlsw",	      VX_LSP(4, 0x2A3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvcntlzh",	      VX_LSP(4, 0x323F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvcntlsh",	      VX_LSP(4, 0x3A3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"znegws",	      VX_LSP(4, 0x4A3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvnegh",	      VX_LSP(4, 0x523F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvneghs",	      VX_LSP(4, 0x5A3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvnegho",	      VX_LSP(4, 0x623F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvneghos",	      VX_LSP(4, 0x6A3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zrndwh",	      VX_LSP(4, 0x823F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zrndwhss",	      VX_LSP(4, 0x8A3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvabsh",	      VX_LSP(4, 0xA23F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvabshs",	      VX_LSP(4, 0xAA3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zabsw",	      VX_LSP(4, 0xB23F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zabsws",	      VX_LSP(4, 0xBA3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zsatswuw",	      VX_LSP(4, 0xC23F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zsatuwsw",	      VX_LSP(4, 0xCA3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zsatswuh",	      VX_LSP(4, 0xD23F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zsatswsh",	      VX_LSP(4, 0xDA3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvsatshuh",	      VX_LSP(4, 0xE23F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zvsatuhsh",	      VX_LSP(4, 0xEA3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zsatuwuh",	      VX_LSP(4, 0xF23F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zsatuwsh",	      VX_LSP(4, 0xFA3F), VX_LSP_MASK, PPCLSP, 0,	{RD, RA}},
{"zsatsduw",	      VX(4, 0x260), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zsatsdsw",	      VX(4, 0x261), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zsatuduw",	      VX(4, 0x262), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvselh",	      VX(4, 0x264), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zxtrw",	      VX(4, 0x264), VX_LSP_OFF_MASK, PPCLSP, 0,		{RD, RA, RB, VX_OFF}},
{"zbrminc",	      VX(4, 0x268), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zcircinc",	      VX(4, 0x269), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zdivwsf",	      VX(4, 0x26B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsrhu",	      VX(4, 0x270), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsrhs",	      VX(4, 0x271), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsrhiu",	      VX(4, 0x272), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM_LT16}},
{"zvsrhis",	      VX(4, 0x273), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM_LT16}},
{"zvslh",	      VX(4, 0x274), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvrlh",	      VX(4, 0x275), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvslhi",	      VX(4, 0x276), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM_LT16}},
{"zvrlhi",	      VX(4, 0x277), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM_LT16}},
{"zvslhus",	      VX(4, 0x278), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvslhss",	      VX(4, 0x279), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvslhius",	      VX(4, 0x27A), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM_LT16}},
{"zvslhiss",	      VX(4, 0x27B), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM_LT16}},
{"zslwus",	      VX(4, 0x27C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zslwss",	      VX(4, 0x27D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zslwius",	      VX(4, 0x27E), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM}},
{"zslwiss",	      VX(4, 0x27F), VX_MASK,	PPCLSP, 0,		{RD, RA, EVUIMM}},
{"zlddx",	      VX(4, 0x300), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zldd",	      VX(4, 0x301), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_8, RA}},
{"zldwx",	      VX(4, 0x302), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zldw",	      VX(4, 0x303), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_8, RA}},
{"zldhx",	      VX(4, 0x304), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zldh",	      VX(4, 0x305), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_8, RA}},
{"zlwgsfdx",	      VX(4, 0x308), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwgsfd",	      VX(4, 0x309), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4, RA}},
{"zlwwosdx",	      VX(4, 0x30A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwwosd",	      VX(4, 0x30B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4, RA}},
{"zlwhsplatwdx",      VX(4, 0x30C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhsplatwd",	      VX(4, 0x30D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4, RA}},
{"zlwhsplatdx",	      VX(4, 0x30E), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhsplatd",	      VX(4, 0x30F), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4, RA}},
{"zlwhgwsfdx",	      VX(4, 0x310), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhgwsfd",	      VX(4, 0x311), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4, RA}},
{"zlwhedx",	      VX(4, 0x312), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhed",	      VX(4, 0x313), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4, RA}},
{"zlwhosdx",	      VX(4, 0x314), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhosd",	      VX(4, 0x315), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4, RA}},
{"zlwhoudx",	      VX(4, 0x316), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhoud",	      VX(4, 0x317), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4, RA}},
{"zlwhx",	      VX(4, 0x318), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlwh",	      VX(4, 0x319), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_4, RA}},
{"zlwwx",	      VX(4, 0x31A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlww",	      VX(4, 0x31B), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_4, RA}},
{"zlhgwsfx",	      VX(4, 0x31C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhgwsf",	      VX(4, 0x31D), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2, RA}},
{"zlhhsplatx",	      VX(4, 0x31E), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhhsplat",	      VX(4, 0x31F), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2, RA}},
{"zstddx",	      VX(4, 0x320), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstdd",	      VX(4, 0x321), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_8, RA}},
{"zstdwx",	      VX(4, 0x322), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstdw",	      VX(4, 0x323), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_8, RA}},
{"zstdhx",	      VX(4, 0x324), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstdh",	      VX(4, 0x325), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_8, RA}},
{"zstwhedx",	      VX(4, 0x328), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstwhed",	      VX(4, 0x329), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_4, RA}},
{"zstwhodx",	      VX(4, 0x32A), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstwhod",	      VX(4, 0x32B), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_4, RA}},
{"zlhhex",	      VX(4, 0x330), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhhe",	      VX(4, 0x331), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2, RA}},
{"zlhhosx",	      VX(4, 0x332), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhhos",	      VX(4, 0x333), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2, RA}},
{"zlhhoux",	      VX(4, 0x334), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhhou",	      VX(4, 0x335), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2, RA}},
{"zsthex",	      VX(4, 0x338), VX_MASK,	PPCLSP, 0,		{RS, RA, RB}},
{"zsthe",	      VX(4, 0x339), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_2, RA}},
{"zsthox",	      VX(4, 0x33A), VX_MASK,	PPCLSP, 0,		{RS, RA, RB}},
{"zstho",	      VX(4, 0x33B), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_2, RA}},
{"zstwhx",	      VX(4, 0x33C), VX_MASK,	PPCLSP, 0,		{RS, RA, RB}},
{"zstwh",	      VX(4, 0x33D), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_4, RA}},
{"zstwwx",	      VX(4, 0x33E), VX_MASK,	PPCLSP, 0,		{RS, RA, RB}},
{"zstww",	      VX(4, 0x33F), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_4, RA}},
{"zlddmx",	      VX(4, 0x340), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlddu",	      VX(4, 0x341), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_8_EX0, RA}},
{"zldwmx",	      VX(4, 0x342), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zldwu",	      VX(4, 0x343), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_8_EX0, RA}},
{"zldhmx",	      VX(4, 0x344), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zldhu",	      VX(4, 0x345), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_8_EX0, RA}},
{"zlwgsfdmx",	      VX(4, 0x348), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwgsfdu",	      VX(4, 0x349), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4_EX0, RA}},
{"zlwwosdmx",	      VX(4, 0x34A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwwosdu",	      VX(4, 0x34B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4_EX0, RA}},
{"zlwhsplatwdmx",     VX(4, 0x34C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhsplatwdu",      VX(4, 0x34D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4_EX0, RA}},
{"zlwhsplatdmx",      VX(4, 0x34E), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhsplatdu",	      VX(4, 0x34F), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4_EX0, RA}},
{"zlwhgwsfdmx",	      VX(4, 0x350), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhgwsfdu",	      VX(4, 0x351), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4_EX0, RA}},
{"zlwhedmx",	      VX(4, 0x352), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhedu",	      VX(4, 0x353), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4_EX0, RA}},
{"zlwhosdmx",	      VX(4, 0x354), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhosdu",	      VX(4, 0x355), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4_EX0, RA}},
{"zlwhoudmx",	      VX(4, 0x356), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zlwhoudu",	      VX(4, 0x357), VX_MASK,	PPCLSP, 0,		{RD_EVEN, EVUIMM_4_EX0, RA}},
{"zlwhmx",	      VX(4, 0x358), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlwhu",	      VX(4, 0x359), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_4_EX0, RA}},
{"zlwwmx",	      VX(4, 0x35A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlwwu",	      VX(4, 0x35B), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_4_EX0, RA}},
{"zlhgwsfmx",	      VX(4, 0x35C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhgwsfu",	      VX(4, 0x35D), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2_EX0, RA}},
{"zlhhsplatmx",	      VX(4, 0x35E), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhhsplatu",	      VX(4, 0x35F), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2_EX0, RA}},
{"zstddmx",	      VX(4, 0x360), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstddu",	      VX(4, 0x361), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_8_EX0, RA}},
{"zstdwmx",	      VX(4, 0x362), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstdwu",	      VX(4, 0x363), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_8_EX0, RA}},
{"zstdhmx",	      VX(4, 0x364), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstdhu",	      VX(4, 0x365), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_8_EX0, RA}},
{"zstwhedmx",	      VX(4, 0x368), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstwhedu",	      VX(4, 0x369), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_4_EX0, RA}},
{"zstwhodmx",	      VX(4, 0x36A), VX_MASK,	PPCLSP, 0,		{RS_EVEN, RA, RB}},
{"zstwhodu",	      VX(4, 0x36B), VX_MASK,	PPCLSP, 0,		{RS_EVEN, EVUIMM_4_EX0, RA}},
{"zlhhemx",	      VX(4, 0x370), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhheu",	      VX(4, 0x371), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2_EX0, RA}},
{"zlhhosmx",	      VX(4, 0x372), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhhosu",	      VX(4, 0x373), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2_EX0, RA}},
{"zlhhoumx",	      VX(4, 0x374), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zlhhouu",	      VX(4, 0x375), VX_MASK,	PPCLSP, 0,		{RD, EVUIMM_2_EX0, RA}},
{"zsthemx",	      VX(4, 0x378), VX_MASK,	PPCLSP, 0,		{RS, RA, RB}},
{"zstheu",	      VX(4, 0x379), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_2_EX0, RA}},
{"zsthomx",	      VX(4, 0x37A), VX_MASK,	PPCLSP, 0,		{RS, RA, RB}},
{"zsthou",	      VX(4, 0x37B), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_2_EX0, RA}},
{"zstwhmx",	      VX(4, 0x37C), VX_MASK,	PPCLSP, 0,		{RS, RA, RB}},
{"zstwhu",	      VX(4, 0x37D), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_4_EX0, RA}},
{"zstwwmx",	      VX(4, 0x37E), VX_MASK,	PPCLSP, 0,		{RS, RA, RB}},
{"zstwwu",	      VX(4, 0x37F), VX_MASK,	PPCLSP, 0,		{RS, EVUIMM_4_EX0, RA}},
{"zaddwgui",	      VX(4, 0x460), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zsubfwgui",	      VX(4, 0x461), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zaddd",	      VX(4, 0x462), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zsubfd",	      VX(4, 0x463), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvaddsubfw",	      VX(4, 0x464), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvsubfaddw",	      VX(4, 0x465), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvaddw",	      VX(4, 0x466), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvsubfw",	      VX(4, 0x467), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zaddwgsi",	      VX(4, 0x468), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zsubfwgsi",	      VX(4, 0x469), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zadddss",	      VX(4, 0x46A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zsubfdss",	      VX(4, 0x46B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvaddsubfwss",      VX(4, 0x46C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvsubfaddwss",      VX(4, 0x46D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvaddwss",	      VX(4, 0x46E), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvsubfwss",	      VX(4, 0x46F), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zaddwgsf",	      VX(4, 0x470), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zsubfwgsf",	      VX(4, 0x471), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zadddus",	      VX(4, 0x472), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zsubfdus",	      VX(4, 0x473), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvaddwus",	      VX(4, 0x476), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvsubfwus",	      VX(4, 0x477), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvunpkhgwsf",	      VX_LSP(4, 0x478), VX_LSP_MASK, PPCLSP, 0,		{RD_EVEN, RA}},
{"zvunpkhsf",	      VX_LSP(4, 0xC78), VX_LSP_MASK, PPCLSP, 0,		{RD_EVEN, RA}},
{"zvunpkhui",	      VX_LSP(4, 0x1478), VX_LSP_MASK, PPCLSP, 0,	{RD_EVEN, RA}},
{"zvunpkhsi",	      VX_LSP(4, 0x1C78), VX_LSP_MASK, PPCLSP, 0,	{RD_EVEN, RA}},
{"zunpkwgsf",	      VX_LSP(4, 0x2478), VX_LSP_MASK, PPCLSP, 0,	{RD_EVEN, RA}},
{"zvdotphgwasmf",     VX(4, 0x488), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwasmfr",    VX(4, 0x489), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwasmfaa",   VX(4, 0x48A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwasmfraa",  VX(4, 0x48B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwasmfan",   VX(4, 0x48C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwasmfran",  VX(4, 0x48D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhulgwsmf",	      VX(4, 0x490), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulgwsmfr",      VX(4, 0x491), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulgwsmfaa",     VX(4, 0x492), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulgwsmfraa",    VX(4, 0x493), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulgwsmfan",     VX(4, 0x494), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulgwsmfran",    VX(4, 0x495), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulgwsmfanp",    VX(4, 0x496), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulgwsmfranp",   VX(4, 0x497), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegwsmf",	      VX(4, 0x498), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhegwsmfr",	      VX(4, 0x499), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhegwsmfaa",	      VX(4, 0x49A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhegwsmfraa",      VX(4, 0x49B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhegwsmfan",	      VX(4, 0x49C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhegwsmfran",      VX(4, 0x49D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxgwasmf",    VX(4, 0x4A8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxgwasmfr",   VX(4, 0x4A9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxgwasmfaa",  VX(4, 0x4AA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxgwasmfraa", VX(4, 0x4AB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxgwasmfan",  VX(4, 0x4AC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxgwasmfran", VX(4, 0x4AD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhllgwsmf",	      VX(4, 0x4B0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllgwsmfr",      VX(4, 0x4B1), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllgwsmfaa",     VX(4, 0x4B2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllgwsmfraa",    VX(4, 0x4B3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllgwsmfan",     VX(4, 0x4B4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllgwsmfran",    VX(4, 0x4B5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllgwsmfanp",    VX(4, 0x4B6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllgwsmfranp",   VX(4, 0x4B7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogwsmf",	      VX(4, 0x4B8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheogwsmfr",	      VX(4, 0x4B9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheogwsmfaa",      VX(4, 0x4BA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheogwsmfraa",     VX(4, 0x4BB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheogwsmfan",      VX(4, 0x4BC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheogwsmfran",     VX(4, 0x4BD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwssmf",     VX(4, 0x4C8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwssmfr",    VX(4, 0x4C9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwssmfaa",   VX(4, 0x4CA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwssmfraa",  VX(4, 0x4CB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwssmfan",   VX(4, 0x4CC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphgwssmfran",  VX(4, 0x4CD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhuugwsmf",	      VX(4, 0x4D0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuugwsmfr",      VX(4, 0x4D1), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuugwsmfaa",     VX(4, 0x4D2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuugwsmfraa",    VX(4, 0x4D3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuugwsmfan",     VX(4, 0x4D4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuugwsmfran",    VX(4, 0x4D5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuugwsmfanp",    VX(4, 0x4D6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuugwsmfranp",   VX(4, 0x4D7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogwsmf",	      VX(4, 0x4D8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhogwsmfr",	      VX(4, 0x4D9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhogwsmfaa",	      VX(4, 0x4DA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhogwsmfraa",      VX(4, 0x4DB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhogwsmfan",	      VX(4, 0x4DC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhogwsmfran",      VX(4, 0x4DD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhxlgwsmf",	      VX(4, 0x4F0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlgwsmfr",      VX(4, 0x4F1), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlgwsmfaa",     VX(4, 0x4F2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlgwsmfraa",    VX(4, 0x4F3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlgwsmfan",     VX(4, 0x4F4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlgwsmfran",    VX(4, 0x4F5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlgwsmfanp",    VX(4, 0x4F6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlgwsmfranp",   VX(4, 0x4F7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegui",	      VX(4, 0x500), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgaui",	      VX(4, 0x501), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheguiaa",	      VX(4, 0x502), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgauiaa",     VX(4, 0x503), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheguian",	      VX(4, 0x504), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgauian",     VX(4, 0x505), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsi",	      VX(4, 0x508), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasi",	      VX(4, 0x509), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsiaa",	      VX(4, 0x50A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasiaa",     VX(4, 0x50B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsian",	      VX(4, 0x50C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasian",     VX(4, 0x50D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsui",	      VX(4, 0x510), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasui",      VX(4, 0x511), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsuiaa",	      VX(4, 0x512), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasuiaa",    VX(4, 0x513), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsuian",	      VX(4, 0x514), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasuian",    VX(4, 0x515), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsmf",	      VX(4, 0x518), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasmf",      VX(4, 0x519), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsmfaa",	      VX(4, 0x51A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasmfaa",    VX(4, 0x51B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhegsmfan",	      VX(4, 0x51C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgasmfan",    VX(4, 0x51D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogui",	      VX(4, 0x520), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgaui",      VX(4, 0x521), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheoguiaa",	      VX(4, 0x522), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgauiaa",    VX(4, 0x523), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheoguian",	      VX(4, 0x524), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgauian",    VX(4, 0x525), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsi",	      VX(4, 0x528), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasi",      VX(4, 0x529), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsiaa",	      VX(4, 0x52A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasiaa",    VX(4, 0x52B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsian",	      VX(4, 0x52C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasian",    VX(4, 0x52D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsui",	      VX(4, 0x530), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasui",     VX(4, 0x531), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsuiaa",	      VX(4, 0x532), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasuiaa",   VX(4, 0x533), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsuian",	      VX(4, 0x534), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasuian",   VX(4, 0x535), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsmf",	      VX(4, 0x538), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasmf",     VX(4, 0x539), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsmfaa",	      VX(4, 0x53A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasmfaa",   VX(4, 0x53B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheogsmfan",	      VX(4, 0x53C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphxgasmfan",   VX(4, 0x53D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogui",	      VX(4, 0x540), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgsui",	      VX(4, 0x541), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhoguiaa",	      VX(4, 0x542), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgsuiaa",     VX(4, 0x543), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhoguian",	      VX(4, 0x544), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgsuian",     VX(4, 0x545), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsi",	      VX(4, 0x548), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssi",	      VX(4, 0x549), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsiaa",	      VX(4, 0x54A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssiaa",     VX(4, 0x54B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsian",	      VX(4, 0x54C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssian",     VX(4, 0x54D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsui",	      VX(4, 0x550), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssui",      VX(4, 0x551), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsuiaa",	      VX(4, 0x552), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssuiaa",    VX(4, 0x553), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsuian",	      VX(4, 0x554), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssuian",    VX(4, 0x555), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsmf",	      VX(4, 0x558), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssmf",      VX(4, 0x559), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsmfaa",	      VX(4, 0x55A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssmfaa",    VX(4, 0x55B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmhogsmfan",	      VX(4, 0x55C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvdotphgssmfan",    VX(4, 0x55D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgui",	      VX(4, 0x560), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwguiaa",	      VX(4, 0x562), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwguiaas",	      VX(4, 0x563), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwguian",	      VX(4, 0x564), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwguians",	      VX(4, 0x565), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsi",	      VX(4, 0x568), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsiaa",	      VX(4, 0x56A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsiaas",	      VX(4, 0x56B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsian",	      VX(4, 0x56C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsians",	      VX(4, 0x56D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsui",	      VX(4, 0x570), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsuiaa",	      VX(4, 0x572), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsuiaas",	      VX(4, 0x573), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsuian",	      VX(4, 0x574), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsuians",	      VX(4, 0x575), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsmf",	      VX(4, 0x578), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsmfr",	      VX(4, 0x579), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsmfaa",	      VX(4, 0x57A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsmfraa",	      VX(4, 0x57B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsmfan",	      VX(4, 0x57C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmwgsmfran",	      VX(4, 0x57D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhului",	      VX(4, 0x580), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuluiaa",	      VX(4, 0x582), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuluiaas",	      VX(4, 0x583), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuluian",	      VX(4, 0x584), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuluians",	      VX(4, 0x585), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuluianp",	      VX(4, 0x586), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuluianps",      VX(4, 0x587), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsi",	      VX(4, 0x588), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsiaa",	      VX(4, 0x58A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsiaas",	      VX(4, 0x58B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsian",	      VX(4, 0x58C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsians",	      VX(4, 0x58D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsianp",	      VX(4, 0x58E), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsianps",      VX(4, 0x58F), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsui",	      VX(4, 0x590), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsuiaa",	      VX(4, 0x592), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsuiaas",      VX(4, 0x593), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsuian",	      VX(4, 0x594), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsuians",      VX(4, 0x595), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsuianp",      VX(4, 0x596), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsuianps",     VX(4, 0x597), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsf",	      VX(4, 0x598), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsfr",	      VX(4, 0x599), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsfaas",	      VX(4, 0x59A), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsfraas",      VX(4, 0x59B), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsfans",	      VX(4, 0x59C), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsfrans",      VX(4, 0x59D), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsfanps",      VX(4, 0x59E), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhulsfranps",     VX(4, 0x59F), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllui",	      VX(4, 0x5A0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhlluiaa",	      VX(4, 0x5A2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhlluiaas",	      VX(4, 0x5A3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhlluian",	      VX(4, 0x5A4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhlluians",	      VX(4, 0x5A5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhlluianp",	      VX(4, 0x5A6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhlluianps",      VX(4, 0x5A7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsi",	      VX(4, 0x5A8), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsiaa",	      VX(4, 0x5AA), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsiaas",	      VX(4, 0x5AB), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsian",	      VX(4, 0x5AC), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsians",	      VX(4, 0x5AD), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsianp",	      VX(4, 0x5AE), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsianps",      VX(4, 0x5AF), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsui",	      VX(4, 0x5B0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsuiaa",	      VX(4, 0x5B2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsuiaas",      VX(4, 0x5B3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsuian",	      VX(4, 0x5B4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsuians",      VX(4, 0x5B5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsuianp",      VX(4, 0x5B6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsuianps",     VX(4, 0x5B7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsf",	      VX(4, 0x5B8), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsfr",	      VX(4, 0x5B9), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsfaas",	      VX(4, 0x5BA), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsfraas",      VX(4, 0x5BB), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsfans",	      VX(4, 0x5BC), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsfrans",      VX(4, 0x5BD), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsfanps",      VX(4, 0x5BE), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhllsfranps",     VX(4, 0x5BF), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuuui",	      VX(4, 0x5C0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuuuiaa",	      VX(4, 0x5C2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuuuiaas",	      VX(4, 0x5C3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuuuian",	      VX(4, 0x5C4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuuuians",	      VX(4, 0x5C5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuuuianp",	      VX(4, 0x5C6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuuuianps",      VX(4, 0x5C7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusi",	      VX(4, 0x5C8), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusiaa",	      VX(4, 0x5CA), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusiaas",	      VX(4, 0x5CB), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusian",	      VX(4, 0x5CC), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusians",	      VX(4, 0x5CD), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusianp",	      VX(4, 0x5CE), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusianps",      VX(4, 0x5CF), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusui",	      VX(4, 0x5D0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusuiaa",	      VX(4, 0x5D2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusuiaas",      VX(4, 0x5D3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusuian",	      VX(4, 0x5D4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusuians",      VX(4, 0x5D5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusuianp",      VX(4, 0x5D6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusuianps",     VX(4, 0x5D7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusf",	      VX(4, 0x5D8), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusfr",	      VX(4, 0x5D9), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusfaas",	      VX(4, 0x5DA), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusfraas",      VX(4, 0x5DB), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusfans",	      VX(4, 0x5DC), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusfrans",      VX(4, 0x5DD), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusfanps",      VX(4, 0x5DE), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhuusfranps",     VX(4, 0x5DF), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlui",	      VX(4, 0x5E0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxluiaa",	      VX(4, 0x5E2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxluiaas",	      VX(4, 0x5E3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxluian",	      VX(4, 0x5E4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxluians",	      VX(4, 0x5E5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxluianp",	      VX(4, 0x5E6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxluianps",      VX(4, 0x5E7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsi",	      VX(4, 0x5E8), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsiaa",	      VX(4, 0x5EA), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsiaas",	      VX(4, 0x5EB), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsian",	      VX(4, 0x5EC), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsians",	      VX(4, 0x5ED), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsianp",	      VX(4, 0x5EE), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsianps",      VX(4, 0x5EF), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsui",	      VX(4, 0x5F0), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsuiaa",	      VX(4, 0x5F2), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsuiaas",      VX(4, 0x5F3), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsuian",	      VX(4, 0x5F4), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsuians",      VX(4, 0x5F5), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsuianp",      VX(4, 0x5F6), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsuianps",     VX(4, 0x5F7), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsf",	      VX(4, 0x5F8), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsfr",	      VX(4, 0x5F9), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsfaas",	      VX(4, 0x5FA), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsfraas",      VX(4, 0x5FB), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsfans",	      VX(4, 0x5FC), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsfrans",      VX(4, 0x5FD), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsfanps",      VX(4, 0x5FE), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zvmhxlsfranps",     VX(4, 0x5FF), VX_MASK,	PPCLSP, 0,		{RD_EVEN, RA, RB}},
{"zmheui",	      VX(4, 0x600), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheuiaa",	      VX(4, 0x602), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheuiaas",	      VX(4, 0x603), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheuian",	      VX(4, 0x604), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheuians",	      VX(4, 0x605), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesi",	      VX(4, 0x608), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesiaa",	      VX(4, 0x60A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesiaas",	      VX(4, 0x60B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesian",	      VX(4, 0x60C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesians",	      VX(4, 0x60D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesui",	      VX(4, 0x610), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesuiaa",	      VX(4, 0x612), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesuiaas",	      VX(4, 0x613), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesuian",	      VX(4, 0x614), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesuians",	      VX(4, 0x615), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesf",	      VX(4, 0x618), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesfr",	      VX(4, 0x619), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesfaas",	      VX(4, 0x61A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesfraas",	      VX(4, 0x61B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesfans",	      VX(4, 0x61C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhesfrans",	      VX(4, 0x61D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheoui",	      VX(4, 0x620), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheouiaa",	      VX(4, 0x622), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheouiaas",	      VX(4, 0x623), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheouian",	      VX(4, 0x624), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheouians",	      VX(4, 0x625), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosi",	      VX(4, 0x628), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosiaa",	      VX(4, 0x62A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosiaas",	      VX(4, 0x62B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosian",	      VX(4, 0x62C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosians",	      VX(4, 0x62D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosui",	      VX(4, 0x630), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosuiaa",	      VX(4, 0x632), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosuiaas",	      VX(4, 0x633), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosuian",	      VX(4, 0x634), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosuians",	      VX(4, 0x635), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosf",	      VX(4, 0x638), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosfr",	      VX(4, 0x639), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosfaas",	      VX(4, 0x63A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosfraas",	      VX(4, 0x63B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosfans",	      VX(4, 0x63C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmheosfrans",	      VX(4, 0x63D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhoui",	      VX(4, 0x640), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhouiaa",	      VX(4, 0x642), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhouiaas",	      VX(4, 0x643), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhouian",	      VX(4, 0x644), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhouians",	      VX(4, 0x645), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosi",	      VX(4, 0x648), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosiaa",	      VX(4, 0x64A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosiaas",	      VX(4, 0x64B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosian",	      VX(4, 0x64C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosians",	      VX(4, 0x64D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosui",	      VX(4, 0x650), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosuiaa",	      VX(4, 0x652), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosuiaas",	      VX(4, 0x653), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosuian",	      VX(4, 0x654), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosuians",	      VX(4, 0x655), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosf",	      VX(4, 0x658), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosfr",	      VX(4, 0x659), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosfaas",	      VX(4, 0x65A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosfraas",	      VX(4, 0x65B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosfans",	      VX(4, 0x65C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmhosfrans",	      VX(4, 0x65D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhuih",	      VX(4, 0x660), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhuihs",	      VX(4, 0x661), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhuiaah",	      VX(4, 0x662), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhuiaahs",	      VX(4, 0x663), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhuianh",	      VX(4, 0x664), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhuianhs",	      VX(4, 0x665), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsihs",	      VX(4, 0x669), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsiaahs",	      VX(4, 0x66B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsianhs",	      VX(4, 0x66D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsuihs",	      VX(4, 0x671), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsuiaahs",	      VX(4, 0x673), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsuianhs",	      VX(4, 0x675), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsfh",	      VX(4, 0x678), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsfrh",	      VX(4, 0x679), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsfaahs",	      VX(4, 0x67A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsfraahs",	      VX(4, 0x67B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsfanhs",	      VX(4, 0x67C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvmhsfranhs",	      VX(4, 0x67D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphaui",	      VX(4, 0x680), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphauis",	      VX(4, 0x681), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphauiaa",      VX(4, 0x682), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphauiaas",     VX(4, 0x683), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphauian",      VX(4, 0x684), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphauians",     VX(4, 0x685), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasi",	      VX(4, 0x688), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasis",	      VX(4, 0x689), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasiaa",      VX(4, 0x68A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasiaas",     VX(4, 0x68B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasian",      VX(4, 0x68C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasians",     VX(4, 0x68D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasui",	      VX(4, 0x690), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasuis",      VX(4, 0x691), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasuiaa",     VX(4, 0x692), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasuiaas",    VX(4, 0x693), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasuian",     VX(4, 0x694), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasuians",    VX(4, 0x695), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasfs",	      VX(4, 0x698), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasfrs",      VX(4, 0x699), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasfaas",     VX(4, 0x69A), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasfraas",    VX(4, 0x69B), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasfans",     VX(4, 0x69C), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphasfrans",    VX(4, 0x69D), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxaui",	      VX(4, 0x6A0), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxauis",      VX(4, 0x6A1), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxauiaa",     VX(4, 0x6A2), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxauiaas",    VX(4, 0x6A3), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxauian",     VX(4, 0x6A4), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxauians",    VX(4, 0x6A5), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasi",	      VX(4, 0x6A8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasis",      VX(4, 0x6A9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasiaa",     VX(4, 0x6AA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasiaas",    VX(4, 0x6AB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasian",     VX(4, 0x6AC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasians",    VX(4, 0x6AD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasui",      VX(4, 0x6B0), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasuis",     VX(4, 0x6B1), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasuiaa",    VX(4, 0x6B2), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasuiaas",   VX(4, 0x6B3), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasuian",    VX(4, 0x6B4), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasuians",   VX(4, 0x6B5), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasfs",      VX(4, 0x6B8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasfrs",     VX(4, 0x6B9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasfaas",    VX(4, 0x6BA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasfraas",   VX(4, 0x6BB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasfans",    VX(4, 0x6BC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphxasfrans",   VX(4, 0x6BD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphsui",	      VX(4, 0x6C0), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphsuis",	      VX(4, 0x6C1), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphsuiaa",      VX(4, 0x6C2), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphsuiaas",     VX(4, 0x6C3), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphsuian",      VX(4, 0x6C4), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphsuians",     VX(4, 0x6C5), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssi",	      VX(4, 0x6C8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssis",	      VX(4, 0x6C9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssiaa",      VX(4, 0x6CA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssiaas",     VX(4, 0x6CB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssian",      VX(4, 0x6CC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssians",     VX(4, 0x6CD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssui",	      VX(4, 0x6D0), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssuis",      VX(4, 0x6D1), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssuiaa",     VX(4, 0x6D2), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssuiaas",    VX(4, 0x6D3), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssuian",     VX(4, 0x6D4), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssuians",    VX(4, 0x6D5), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssfs",	      VX(4, 0x6D8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssfrs",      VX(4, 0x6D9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssfaas",     VX(4, 0x6DA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssfraas",    VX(4, 0x6DB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssfans",     VX(4, 0x6DC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zvdotphssfrans",    VX(4, 0x6DD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwluis",	      VX(4, 0x6E1), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwluiaa",	      VX(4, 0x6E2), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwluiaas",	      VX(4, 0x6E3), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwluian",	      VX(4, 0x6E4), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwluians",	      VX(4, 0x6E5), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwlsis",	      VX(4, 0x6E9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwlsiaas",	      VX(4, 0x6EB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwlsians",	      VX(4, 0x6ED), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwlsuis",	      VX(4, 0x6F1), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwlsuiaas",	      VX(4, 0x6F3), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwlsuians",	      VX(4, 0x6F5), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwsf",	      VX(4, 0x6F8), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwsfr",	      VX(4, 0x6F9), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwsfaas",	      VX(4, 0x6FA), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwsfraas",	      VX(4, 0x6FB), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwsfans",	      VX(4, 0x6FC), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
{"zmwsfrans",	      VX(4, 0x6FD), VX_MASK,	PPCLSP, 0,		{RD, RA, RB}},
};

const unsigned int lsp_num_opcodes = ARRAY_SIZE (lsp_opcodes);

/* SPE v2 instruction set from SPE2PIM Rev. 2 08/2011 */
const struct powerpc_opcode spe2_opcodes[] = {
{"evdotpwcssi",		  VX (4, 128),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcsmi",		  VX (4, 129),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssfr",	  VX (4, 130),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssf",		  VX (4, 131),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgasmf",	  VX (4, 136),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgasmf",	  VX (4, 137),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgasmfr",	  VX (4, 138),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgasmfr",	  VX (4, 139),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgssmf",	  VX (4, 140),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgssmf",	  VX (4, 141),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgssmfr",	  VX (4, 142),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgssmfr",	  VX (4, 143),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssiaaw3",	  VX (4, 144),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcsmiaaw3",	  VX (4, 145),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssfraaw3",	  VX (4, 146),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssfaaw3",	  VX (4, 147),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgasmfaa3",	  VX (4, 152),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgasmfaa3",	  VX (4, 153),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgasmfraa3",	  VX (4, 154),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgasmfraa3",	  VX (4, 155),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgssmfaa3",	  VX (4, 156),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgssmfaa3",	  VX (4, 157),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgssmfraa3",	  VX (4, 158),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgssmfraa3",	  VX (4, 159),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssia",	  VX (4, 160),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcsmia",	  VX (4, 161),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssfra",	  VX (4, 162),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssfa",	  VX (4, 163),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgasmfa",	  VX (4, 168),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgasmfa",	  VX (4, 169),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgasmfra",	  VX (4, 170),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgasmfra",	  VX (4, 171),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgssmfa",	  VX (4, 172),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgssmfa",	  VX (4, 173),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgssmfra",	  VX (4, 174),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgssmfra",	  VX (4, 175),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssiaaw",	  VX (4, 176),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcsmiaaw",	  VX (4, 177),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssfraaw",	  VX (4, 178),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwcssfaaw",	  VX (4, 179),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgasmfaa",	  VX (4, 184),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgasmfaa",	  VX (4, 185),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgasmfraa",	  VX (4, 186),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgasmfraa",	  VX (4, 187),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgssmfaa",	  VX (4, 188),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgssmfaa",	  VX (4, 189),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwgssmfraa",	  VX (4, 190),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwxgssmfraa",	  VX (4, 191),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssi",	  VX (4, 256),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssi",	  VX (4, 257),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssf",	  VX (4, 258),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssf",	  VX (4, 259),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcsmi",	  VX (4, 264),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcsmi",	  VX (4, 265),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssfr",	  VX (4, 266),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssfr",	  VX (4, 267),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssiaaw3",	  VX (4, 272),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssiaaw3",	  VX (4, 273),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssfaaw3",	  VX (4, 274),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssfaaw3",	  VX (4, 275),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcsmiaaw3",	  VX (4, 280),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcsmiaaw3",	  VX (4, 281),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssfraaw3",	  VX (4, 282),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssfraaw3",	  VX (4, 283),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssia",	  VX (4, 288),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssia",	  VX (4, 289),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssfa",	  VX (4, 290),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssfa",	  VX (4, 291),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcsmia",	  VX (4, 296),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcsmia",	  VX (4, 297),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssfra",	  VX (4, 298),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssfra",	  VX (4, 299),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssiaaw",	  VX (4, 304),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssiaaw",	  VX (4, 305),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssfaaw",	  VX (4, 306),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssfaaw",	  VX (4, 307),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcsmiaaw",	  VX (4, 312),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcsmiaaw",	  VX (4, 313),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphihcssfraaw",	  VX (4, 314),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotplohcssfraaw",	  VX (4, 315),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphausi",		  VX (4, 320),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassi",		  VX (4, 321),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasusi",	  VX (4, 322),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassf",		  VX (4, 323),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssf",		  VX (4, 327),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphaumi",		  VX (4, 328),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasmi",		  VX (4, 329),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasumi",	  VX (4, 330),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassfr",	  VX (4, 331),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphssmi",		  VX (4, 333),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssi",		  VX (4, 333),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssfr",	  VX (4, 335),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphausiaaw3",	  VX (4, 336),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassiaaw3",	  VX (4, 337),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasusiaaw3",	  VX (4, 338),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassfaaw3",	  VX (4, 339),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssiaaw3",	  VX (4, 341),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssfaaw3",	  VX (4, 343),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphaumiaaw3",	  VX (4, 344),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasmiaaw3",	  VX (4, 345),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasumiaaw3",	  VX (4, 346),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassfraaw3",	  VX (4, 347),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphssmiaaw3",	  VX (4, 349),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssfraaw3",	  VX (4, 351),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphausia",	  VX (4, 352),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassia",	  VX (4, 353),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasusia",	  VX (4, 354),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassfa",	  VX (4, 355),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssfa",	  VX (4, 359),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphaumia",	  VX (4, 360),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasmia",	  VX (4, 361),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasumia",	  VX (4, 362),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassfra",	  VX (4, 363),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphssmia",	  VX (4, 365),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssia",	  VX (4, 365),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssfra",	  VX (4, 367),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphausiaaw",	  VX (4, 368),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassiaaw",	  VX (4, 369),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasusiaaw",	  VX (4, 370),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassfaaw",	  VX (4, 371),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssiaaw",	  VX (4, 373),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssfaaw",	  VX (4, 375),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphaumiaaw",	  VX (4, 376),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasmiaaw",	  VX (4, 377),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphasumiaaw",	  VX (4, 378),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphassfraaw",	  VX (4, 379),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphssmiaaw",	  VX (4, 381),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotphsssfraaw",	  VX (4, 383),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgaumi",	  VX (4, 384),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasmi",	  VX (4, 385),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasumi",	  VX (4, 386),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasmf",	  VX (4, 387),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgssmi",	  VX (4, 388),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgssmf",	  VX (4, 389),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgasmi",	  VX (4, 390),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgasmf",	  VX (4, 391),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbaumi",		  VX (4, 392),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbasmi",		  VX (4, 393),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbasumi",	  VX (4, 394),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgssmi",	  VX (4, 398),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgssmf",	  VX (4, 399),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgaumiaa3",	  VX (4, 400),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasmiaa3",	  VX (4, 401),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasumiaa3",	  VX (4, 402),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasmfaa3",	  VX (4, 403),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgssmiaa3",	  VX (4, 404),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgssmfaa3",	  VX (4, 405),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgasmiaa3",	  VX (4, 406),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgasmfaa3",	  VX (4, 407),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbaumiaaw3",	  VX (4, 408),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbasmiaaw3",	  VX (4, 409),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbasumiaaw3",	  VX (4, 410),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgssmiaa3",	  VX (4, 414),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgssmfaa3",	  VX (4, 415),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgaumia",	  VX (4, 416),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasmia",	  VX (4, 417),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasumia",	  VX (4, 418),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasmfa",	  VX (4, 419),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgssmia",	  VX (4, 420),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgssmfa",	  VX (4, 421),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgasmia",	  VX (4, 422),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgasmfa",	  VX (4, 423),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbaumia",	  VX (4, 424),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbasmia",	  VX (4, 425),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbasumia",	  VX (4, 426),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgssmia",	  VX (4, 430),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgssmfa",	  VX (4, 431),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgaumiaa",	  VX (4, 432),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasmiaa",	  VX (4, 433),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasumiaa",	  VX (4, 434),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgasmfaa",	  VX (4, 435),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgssmiaa",	  VX (4, 436),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hgssmfaa",	  VX (4, 437),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgasmiaa",	  VX (4, 438),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgasmfaa",	  VX (4, 439),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbaumiaaw",	  VX (4, 440),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbasmiaaw",	  VX (4, 441),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpbasumiaaw",	  VX (4, 442),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgssmiaa",	  VX (4, 446),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotp4hxgssmfaa",	  VX (4, 447),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwausi",		  VX (4, 448),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwassi",		  VX (4, 449),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasusi",	  VX (4, 450),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwaumi",		  VX (4, 456),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasmi",		  VX (4, 457),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasumi",	  VX (4, 458),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwssmi",		  VX (4, 461),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwsssi",		  VX (4, 461),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwausiaa3",	  VX (4, 464),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwassiaa3",	  VX (4, 465),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasusiaa3",	  VX (4, 466),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwsssiaa3",	  VX (4, 469),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwaumiaa3",	  VX (4, 472),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasmiaa3",	  VX (4, 473),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasumiaa3",	  VX (4, 474),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwssmiaa3",	  VX (4, 477),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwausia",	  VX (4, 480),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwassia",	  VX (4, 481),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasusia",	  VX (4, 482),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwaumia",	  VX (4, 488),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasmia",	  VX (4, 489),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasumia",	  VX (4, 490),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwssmia",	  VX (4, 493),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwsssia",	  VX (4, 493),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwausiaa",	  VX (4, 496),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwassiaa",	  VX (4, 497),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasusiaa",	  VX (4, 498),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwsssiaa",	  VX (4, 501),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwaumiaa",	  VX (4, 504),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasmiaa",	  VX (4, 505),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwasumiaa",	  VX (4, 506),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdotpwssmiaa",	  VX (4, 509),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddib",		  VX (4, 515),		VX_MASK,		PPCSPE2, 0, {RD, RB, UIMM}},
{"evaddih",		  VX (4, 513),		VX_MASK,		PPCSPE2, 0, {RD, RB, UIMM}},
{"evsubifh",		  VX (4, 517),		VX_MASK,		PPCSPE2, 0, {RD, UIMM, RB}},
{"evsubifb",		  VX (4, 519),		VX_MASK,		PPCSPE2, 0, {RD, UIMM, RB}},
{"evabsb",		  VX_RB_CONST(4, 520, 2),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evabsh",		  VX_RB_CONST(4, 520, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evabsd",		  VX_RB_CONST(4, 520, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evabss",		  VX_RB_CONST(4, 520, 8),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evabsbs",		  VX_RB_CONST(4, 520, 10), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evabshs",		  VX_RB_CONST(4, 520, 12), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evabsds",		  VX_RB_CONST(4, 520, 14), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegwo",		  VX_RB_CONST(4, 521, 1),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegb",		  VX_RB_CONST(4, 521, 2),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegbo",		  VX_RB_CONST(4, 521, 3),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegh",		  VX_RB_CONST(4, 521, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegho",		  VX_RB_CONST(4, 521, 5),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegd",		  VX_RB_CONST(4, 521, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegs",		  VX_RB_CONST(4, 521, 8),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegwos",		  VX_RB_CONST(4, 521, 9),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegbs",		  VX_RB_CONST(4, 521, 10), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegbos",		  VX_RB_CONST(4, 521, 11), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evneghs",		  VX_RB_CONST(4, 521, 12), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evneghos",		  VX_RB_CONST(4, 521, 13), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evnegds",		  VX_RB_CONST(4, 521, 14), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evextzb",		  VX_RB_CONST(4, 522, 1),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evextsbh",		  VX_RB_CONST(4, 522, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evextsw",		  VX_RB_CONST(4, 523, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndwh",		  VX_RB_CONST(4, 524, 0),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndhb",		  VX_RB_CONST(4, 524, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrnddw",		  VX_RB_CONST(4, 524, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndwhus",		  VX_RB_CONST(4, 524, 8),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndwhss",		  VX_RB_CONST(4, 524, 9),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndhbus",		  VX_RB_CONST(4, 524, 12), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndhbss",		  VX_RB_CONST(4, 524, 13), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrnddwus",		  VX_RB_CONST(4, 524, 14), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrnddwss",		  VX_RB_CONST(4, 524, 15), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndwnh",		  VX_RB_CONST(4, 524, 16), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndhnb",		  VX_RB_CONST(4, 524, 20), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrnddnw",		  VX_RB_CONST(4, 524, 22), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndwnhus",		  VX_RB_CONST(4, 524, 24), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndwnhss",		  VX_RB_CONST(4, 524, 25), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndhnbus",		  VX_RB_CONST(4, 524, 28), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrndhnbss",		  VX_RB_CONST(4, 524, 29), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrnddnwus",		  VX_RB_CONST(4, 524, 30), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evrnddnwss",		  VX_RB_CONST(4, 524, 31), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evcntlzh",		  VX_RB_CONST(4, 525, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evcntlsh",		  VX_RB_CONST(4, 526, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evpopcntb",		  VX_RB_CONST(4, 526, 26), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"circinc",		  VX (4, 528),		   VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evunpkhibui",		  VX_RB_CONST(4, 540, 0),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpkhibsi",		  VX_RB_CONST(4, 540, 1),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpkhihui",		  VX_RB_CONST(4, 540, 2),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpkhihsi",		  VX_RB_CONST(4, 540, 3),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpklobui",		  VX_RB_CONST(4, 540, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpklobsi",		  VX_RB_CONST(4, 540, 5),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpklohui",		  VX_RB_CONST(4, 540, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpklohsi",		  VX_RB_CONST(4, 540, 7),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpklohf",		  VX_RB_CONST(4, 540, 8),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpkhihf",		  VX_RB_CONST(4, 540, 9),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpklowgsf",	  VX_RB_CONST(4, 540, 12), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evunpkhiwgsf",	  VX_RB_CONST(4, 540, 13), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatsduw",		  VX_RB_CONST(4, 540, 16), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatsdsw",		  VX_RB_CONST(4, 540, 17), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatshub",		  VX_RB_CONST(4, 540, 18), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatshsb",		  VX_RB_CONST(4, 540, 19), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatuwuh",		  VX_RB_CONST(4, 540, 20), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatswsh",		  VX_RB_CONST(4, 540, 21), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatswuh",		  VX_RB_CONST(4, 540, 22), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatuhub",		  VX_RB_CONST(4, 540, 23), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatuduw",		  VX_RB_CONST(4, 540, 24), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatuwsw",		  VX_RB_CONST(4, 540, 25), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatshuh",		  VX_RB_CONST(4, 540, 26), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatuhsh",		  VX_RB_CONST(4, 540, 27), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatswuw",		  VX_RB_CONST(4, 540, 28), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatswgsdf",		  VX_RB_CONST(4, 540, 29), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatsbub",		  VX_RB_CONST(4, 540, 30), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsatubsb",		  VX_RB_CONST(4, 540, 31), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evmaxhpuw",		  VX_RB_CONST(4, 541, 0),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evmaxhpsw",		  VX_RB_CONST(4, 541, 1),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evmaxbpuh",		  VX_RB_CONST(4, 541, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evmaxbpsh",		  VX_RB_CONST(4, 541, 5),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evmaxwpud",		  VX_RB_CONST(4, 541, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evmaxwpsd",		  VX_RB_CONST(4, 541, 7),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evminhpuw",		  VX_RB_CONST(4, 541, 8),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evminhpsw",		  VX_RB_CONST(4, 541, 9),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evminbpuh",		  VX_RB_CONST(4, 541, 12), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evminbpsh",		  VX_RB_CONST(4, 541, 13), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evminwpud",		  VX_RB_CONST(4, 541, 14), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evminwpsd",		  VX_RB_CONST(4, 541, 15), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evmaxmagws",		  VX (4, 543),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsl",		  VX (4, 549),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsli",		  VX (4, 551),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM}},
{"evsplatie",		  VX_RB_CONST (4, 553, 1),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatib",		  VX_RB_CONST (4, 553, 2),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatibe",		  VX_RB_CONST (4, 553, 3),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatih",		  VX_RB_CONST (4, 553, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatihe",		  VX_RB_CONST (4, 553, 5),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatid",		  VX_RB_CONST (4, 553, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatia",		  VX_RB_CONST (4, 553, 16), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatiea",		  VX_RB_CONST (4, 553, 17), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatiba",		  VX_RB_CONST (4, 553, 18), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatibea",		  VX_RB_CONST (4, 553, 19), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatiha",		  VX_RB_CONST (4, 553, 20), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatihea",		  VX_RB_CONST (4, 553, 21), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatida",		  VX_RB_CONST (4, 553, 22), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfio",		  VX_RB_CONST (4, 555, 1),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfib",		  VX_RB_CONST (4, 555, 2),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfibo",		  VX_RB_CONST (4, 555, 3),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfih",		  VX_RB_CONST (4, 555, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfiho",		  VX_RB_CONST (4, 555, 5),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfid",		  VX_RB_CONST (4, 555, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfia",		  VX_RB_CONST (4, 555, 16), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfioa",		  VX_RB_CONST (4, 555, 17), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfiba",		  VX_RB_CONST (4, 555, 18), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfiboa",	  VX_RB_CONST (4, 555, 19), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfiha",		  VX_RB_CONST (4, 555, 20), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfihoa",	  VX_RB_CONST (4, 555, 21), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evsplatfida",		  VX_RB_CONST (4, 555, 22), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, SIMM}},
{"evcmpgtdu",		  VX_SPE_CRFD (4, 560, 1), VX_SPE_CRFD_MASK,	PPCSPE2, 0, {CRFD, RA, RB}},
{"evcmpgtds",		  VX_SPE_CRFD (4, 561, 1), VX_SPE_CRFD_MASK,	PPCSPE2, 0, {CRFD, RA, RB}},
{"evcmpltdu",		  VX_SPE_CRFD (4, 562, 1), VX_SPE_CRFD_MASK,	PPCSPE2, 0, {CRFD, RA, RB}},
{"evcmpltds",		  VX_SPE_CRFD (4, 563, 1), VX_SPE_CRFD_MASK,	PPCSPE2, 0, {CRFD, RA, RB}},
{"evcmpeqd",		  VX_SPE_CRFD (4, 564, 1), VX_SPE_CRFD_MASK,	PPCSPE2, 0, {CRFD, RA, RB}},
{"evswapbhilo",		  VX (4, 568),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evswapblohi",		  VX (4, 569),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evswaphhilo",		  VX (4, 570),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evswaphlohi",		  VX (4, 571),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evswaphe",		  VX (4, 572),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evswaphhi",		  VX (4, 573),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evswaphlo",		  VX (4, 574),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evswapho",		  VX (4, 575),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evinsb",		  VX (4, 584),		VX_MASK_DDD,		PPCSPE2, 0, {RD, RA, DDD, BBB}},
{"evxtrb",		  VX (4, 586),		VX_MASK_DDD,		PPCSPE2, 0, {RD, RA, DDD, BBB}},
{"evsplath",		  VX_SPE2_HH (4, 588, 0, 0), VX_SPE2_HH_MASK,	PPCSPE2, 0, {RD, RA, HH}},
{"evsplatb",		  VX_SPE2_SPLATB (4, 588, 2), VX_SPE2_SPLATB_MASK, PPCSPE2, 0, {RD, RA, BBB}},
{"evinsh",		  VX_SPE2_DDHH (4, 589, 0), VX_SPE2_DDHH_MASK,	PPCSPE2, 0, {RD, RA, DD, HH}},
{"evclrbe",		  VX_SPE2_CLR (4, 590, 0), VX_SPE2_CLR_MASK,	PPCSPE2, 0, {RD, RA, MMMM}},
{"evclrbo",		  VX_SPE2_CLR (4, 590, 1), VX_SPE2_CLR_MASK,	PPCSPE2, 0, {RD, RA, MMMM}},
{"evclrh",		  VX_SPE2_CLR (4, 591, 1), VX_SPE2_CLR_MASK,	PPCSPE2, 0, {RD, RA, MMMM}},
{"evxtrh",		  VX_SPE2_DDHH (4, 591, 0), VX_SPE2_DDHH_MASK,	PPCSPE2, 0, {RD, RA, DD, HH}},
{"evselbitm0",		  VX (4, 592),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evselbitm1",		  VX (4, 593),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evselbit",		  VX (4, 594),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evperm",		  VX (4, 596),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evperm2",		  VX (4, 597),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evperm3",		  VX (4, 598),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evxtrd",		  VX (4, 600),		VX_OFF_SPE2_MASK,	PPCSPE2, 0, {RD, RA, RB, VX_OFF_SPE2}},
{"evsrbu",		  VX (4, 608),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsrbs",		  VX (4, 609),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsrbiu",		  VX (4, 610),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM_LT8}},
{"evsrbis",		  VX (4, 611),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM_LT8}},
{"evslb",		  VX (4, 612),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evrlb",		  VX (4, 613),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evslbi",		  VX (4, 614),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM_LT8}},
{"evrlbi",		  VX (4, 615),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM_LT8}},
{"evsrhu",		  VX (4, 616),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsrhs",		  VX (4, 617),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsrhiu",		  VX (4, 618),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM_LT16}},
{"evsrhis",		  VX (4, 619),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM_LT16}},
{"evslh",		  VX (4, 620),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evrlh",		  VX (4, 621),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evslhi",		  VX (4, 622),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM_LT16}},
{"evrlhi",		  VX (4, 623),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM_LT16}},
{"evsru",		  VX (4, 624),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsrs",		  VX (4, 625),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsriu",		  VX (4, 626),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM}},
{"evsris",		  VX (4, 627),		VX_MASK,		PPCSPE2, 0, {RD, RA, EVUIMM}},
{"evlvsl",		  VX (4, 628),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlvsr",		  VX (4, 629),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsroiu",		  VX_SPE2_OCTET (4, 631, 0), VX_SPE2_OCTET_MASK, PPCSPE2, 0, {RD, RA, NNN}},
{"evsrois",		  VX_SPE2_OCTET (4, 631, 1), VX_SPE2_OCTET_MASK, PPCSPE2, 0, {RD, RA, NNN}},
{"evsloi",		  VX_SPE2_OCTET (4, 631, 2), VX_SPE2_OCTET_MASK, PPCSPE2, 0, {RD, RA, NNN}},
{"evldbx",		  VX (4, 774),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evldb",		  VX (4, 775),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_8, RA}},
{"evlhhsplathx",	  VX (4, 778),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlhhsplath",		  VX (4, 779),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_2, RA}},
{"evlwbsplatwx",	  VX (4, 786),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwbsplatw",		  VX (4, 787),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4, RA}},
{"evlwhsplatwx",	  VX (4, 794),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwhsplatw",		  VX (4, 795),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4, RA}},
{"evlbbsplatbx",	  VX (4, 798),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlbbsplatb",		  VX (4, 799),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_1, RA}},
{"evstdbx",		  VX (4, 806),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstdb",		  VX (4, 807),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_8, RA}},
{"evlwbex",		  VX (4, 810),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwbe",		  VX (4, 811),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4, RA}},
{"evlwboux",		  VX (4, 812),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwbou",		  VX (4, 813),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4, RA}},
{"evlwbosx",		  VX (4, 814),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwbos",		  VX (4, 815),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4, RA}},
{"evstwbex",		  VX (4, 818),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwbe",		  VX (4, 819),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4, RA}},
{"evstwbox",		  VX (4, 822),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwbo",		  VX (4, 823),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4, RA}},
{"evstwbx",		  VX (4, 826),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwb",		  VX (4, 827),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4, RA}},
{"evsthbx",		  VX (4, 830),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evsthb",		  VX (4, 831),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_2, RA}},
{"evlddmx",		  VX (4, 832),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlddu",		  VX (4, 833),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_8_EX0, RA}},
{"evldwmx",		  VX (4, 834),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evldwu",		  VX (4, 835),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_8_EX0, RA}},
{"evldhmx",		  VX (4, 836),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evldhu",		  VX (4, 837),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_8_EX0, RA}},
{"evldbmx",		  VX (4, 838),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evldbu",		  VX (4, 839),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_8_EX0, RA}},
{"evlhhesplatmx",	  VX (4, 840),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlhhesplatu",	  VX (4, 841),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_2_EX0, RA}},
{"evlhhsplathmx",	  VX (4, 842),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlhhsplathu",	  VX (4, 843),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_2_EX0, RA}},
{"evlhhousplatmx",	  VX (4, 844),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlhhousplatu",	  VX (4, 845),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_2_EX0, RA}},
{"evlhhossplatmx",	  VX (4, 846),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlhhossplatu",	  VX (4, 847),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_2_EX0, RA}},
{"evlwhemx",		  VX (4, 848),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwheu",		  VX (4, 849),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlwbsplatwmx",	  VX (4, 850),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwbsplatwu",	  VX (4, 851),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlwhoumx",		  VX (4, 852),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwhouu",		  VX (4, 853),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlwhosmx",		  VX (4, 854),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwhosu",		  VX (4, 855),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlwwsplatmx",	  VX (4, 856),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwwsplatu",		  VX (4, 857),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlwhsplatwmx",	  VX (4, 858),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwhsplatwu",	  VX (4, 859),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlwhsplatmx",	  VX (4, 860),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwhsplatu",		  VX (4, 861),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlbbsplatbmx",	  VX (4, 862),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlbbsplatbu",	  VX (4, 863),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_1_EX0, RA}},
{"evstddmx",		  VX (4, 864),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstddu",		  VX (4, 865),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_8_EX0, RA}},
{"evstdwmx",		  VX (4, 866),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstdwu",		  VX (4, 867),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_8_EX0, RA}},
{"evstdhmx",		  VX (4, 868),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstdhu",		  VX (4, 869),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_8_EX0, RA}},
{"evstdbmx",		  VX (4, 870),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstdbu",		  VX (4, 871),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_8_EX0, RA}},
{"evlwbemx",		  VX (4, 874),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwbeu",		  VX (4, 875),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlwboumx",		  VX (4, 876),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwbouu",		  VX (4, 877),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evlwbosmx",		  VX (4, 878),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evlwbosu",		  VX (4, 879),		VX_MASK,		PPCSPE2, 0, {RD, EVUIMM_4_EX0, RA}},
{"evstwhemx",		  VX (4, 880),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwheu",		  VX (4, 881),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4_EX0, RA}},
{"evstwbemx",		  VX (4, 882),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwbeu",		  VX (4, 883),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4_EX0, RA}},
{"evstwhomx",		  VX (4, 884),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwhou",		  VX (4, 885),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4_EX0, RA}},
{"evstwbomx",		  VX (4, 886),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwbou",		  VX (4, 887),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4_EX0, RA}},
{"evstwwemx",		  VX (4, 888),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwweu",		  VX (4, 889),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4_EX0, RA}},
{"evstwbmx",		  VX (4, 890),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwbu",		  VX (4, 891),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4_EX0, RA}},
{"evstwwomx",		  VX (4, 892),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evstwwou",		  VX (4, 893),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_4_EX0, RA}},
{"evsthbmx",		  VX (4, 894),		VX_MASK,		PPCSPE2, 0, {RS, RA, RB}},
{"evsthbu",		  VX (4, 895),		VX_MASK,		PPCSPE2, 0, {RS, EVUIMM_2_EX0, RA}},
{"evmhusi",		  VX (4, 1024),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhssi",		  VX (4, 1025),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhsusi",		  VX (4, 1026),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhssf",		  VX (4, 1028),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhumi",		  VX (4, 1029),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhssfr",		  VX (4, 1030),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhesumi",		  VX (4, 1034),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhosumi",		  VX (4, 1038),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbeumi",		  VX (4, 1048),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesmi",		  VX (4, 1049),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesumi",		  VX (4, 1050),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmboumi",		  VX (4, 1052),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosmi",		  VX (4, 1053),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosumi",		  VX (4, 1054),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhesumia",		  VX (4, 1066),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhosumia",		  VX (4, 1070),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbeumia",		  VX (4, 1080),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesmia",		  VX (4, 1081),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesumia",		  VX (4, 1082),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmboumia",		  VX (4, 1084),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosmia",		  VX (4, 1085),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosumia",		  VX (4, 1086),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwusiw",		  VX (4, 1088),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwssiw",		  VX (4, 1089),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfr",		  VX (4, 1094),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwehgsmfr",		  VX (4, 1110),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwehgsmf",		  VX (4, 1111),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwohgsmfr",		  VX (4, 1118),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwohgsmf",		  VX (4, 1119),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfra",		  VX (4, 1126),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwehgsmfra",	  VX (4, 1142),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwehgsmfa",		  VX (4, 1143),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwohgsmfra",	  VX (4, 1150),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwohgsmfa",		  VX (4, 1151),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddusiaa",		  VX_RB_CONST(4, 1152, 0), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evaddssiaa",		  VX_RB_CONST(4, 1153, 0), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsubfusiaa",		  VX_RB_CONST(4, 1154, 0), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsubfssiaa",		  VX_RB_CONST(4, 1155, 0), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evaddsmiaa",		  VX_RB_CONST(4, 1156, 0), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsubfsmiaa",		  VX_RB_CONST(4, 1158, 0), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evaddh",		  VX (4, 1160),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhss",		  VX (4, 1161),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfh",		  VX (4, 1162),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhss",		  VX (4, 1163),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhx",		  VX (4, 1164),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhxss",		  VX (4, 1165),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhx",		  VX (4, 1166),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhxss",		  VX (4, 1167),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddd",		  VX (4, 1168),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evadddss",		  VX (4, 1169),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfd",		  VX (4, 1170),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfdss",		  VX (4, 1171),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddb",		  VX (4, 1172),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddbss",		  VX (4, 1173),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfb",		  VX (4, 1174),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfbss",		  VX (4, 1175),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddsubfh",		  VX (4, 1176),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddsubfhss",	  VX (4, 1177),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfaddh",		  VX (4, 1178),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfaddhss",	  VX (4, 1179),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddsubfhx",		  VX (4, 1180),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddsubfhxss",	  VX (4, 1181),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfaddhx",		  VX (4, 1182),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfaddhxss",	  VX (4, 1183),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evadddus",		  VX (4, 1184),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddbus",		  VX (4, 1185),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfdus",		  VX (4, 1186),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfbus",		  VX (4, 1187),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwus",		  VX (4, 1188),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwxus",		  VX (4, 1189),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwus",		  VX (4, 1190),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwxus",		  VX (4, 1191),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evadd2subf2h",	  VX (4, 1192),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evadd2subf2hss",	  VX (4, 1193),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubf2add2h",	  VX (4, 1194),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubf2add2hss",	  VX (4, 1195),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhus",		  VX (4, 1196),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhxus",		  VX (4, 1197),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhus",		  VX (4, 1198),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhxus",		  VX (4, 1199),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwss",		  VX (4, 1201),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwss",		  VX (4, 1203),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwx",		  VX (4, 1204),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwxss",		  VX (4, 1205),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwx",		  VX (4, 1206),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwxss",		  VX (4, 1207),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddsubfw",		  VX (4, 1208),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddsubfwss",	  VX (4, 1209),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfaddw",		  VX (4, 1210),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfaddwss",	  VX (4, 1211),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddsubfwx",		  VX (4, 1212),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddsubfwxss",	  VX (4, 1213),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfaddwx",		  VX (4, 1214),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfaddwxss",	  VX (4, 1215),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmar",		  VX_SPE2_EVMAR (4, 1220),  VX_SPE2_EVMAR_MASK, PPCSPE2, 0, {RD}},
{"evsumwu",		  VX_RB_CONST(4, 1221, 0),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsumws",		  VX_RB_CONST(4, 1221, 1),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum4bu",		  VX_RB_CONST(4, 1221, 2),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum4bs",		  VX_RB_CONST(4, 1221, 3),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2hu",		  VX_RB_CONST(4, 1221, 4),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2hs",		  VX_RB_CONST(4, 1221, 5),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evdiff2his",		  VX_RB_CONST(4, 1221, 6),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2his",		  VX_RB_CONST(4, 1221, 7),  VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsumwua",		  VX_RB_CONST(4, 1221, 16), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsumwsa",		  VX_RB_CONST(4, 1221, 17), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum4bua",		  VX_RB_CONST(4, 1221, 18), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum4bsa",		  VX_RB_CONST(4, 1221, 19), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2hua",		  VX_RB_CONST(4, 1221, 20), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2hsa",		  VX_RB_CONST(4, 1221, 21), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evdiff2hisa",		  VX_RB_CONST(4, 1221, 22), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2hisa",		  VX_RB_CONST(4, 1221, 23), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsumwuaa",		  VX_RB_CONST(4, 1221, 24), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsumwsaa",		  VX_RB_CONST(4, 1221, 25), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum4buaaw",		  VX_RB_CONST(4, 1221, 26), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum4bsaaw",		  VX_RB_CONST(4, 1221, 27), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2huaaw",		  VX_RB_CONST(4, 1221, 28), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2hsaaw",		  VX_RB_CONST(4, 1221, 29), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evdiff2hisaaw",	  VX_RB_CONST(4, 1221, 30), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evsum2hisaaw",	  VX_RB_CONST(4, 1221, 31), VX_RB_CONST_MASK,	PPCSPE2, 0, {RD, RA}},
{"evdivwsf",		  VX (4, 1228),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdivwuf",		  VX (4, 1229),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdivs",		  VX (4, 1230),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdivu",		  VX (4, 1231),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwegsi",		  VX (4, 1232),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwegsf",		  VX (4, 1233),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwegsi",		  VX (4, 1234),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwegsf",		  VX (4, 1235),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwogsi",		  VX (4, 1236),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddwogsf",		  VX (4, 1237),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwogsi",		  VX (4, 1238),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfwogsf",		  VX (4, 1239),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhhiuw",		  VX (4, 1240),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhhisw",		  VX (4, 1241),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhhiuw",		  VX (4, 1242),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhhisw",		  VX (4, 1243),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhlouw",		  VX (4, 1244),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evaddhlosw",		  VX (4, 1245),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhlouw",		  VX (4, 1246),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsubfhlosw",		  VX (4, 1247),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhesusiaaw",	  VX (4, 1282),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhosusiaaw",	  VX (4, 1286),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhesumiaaw",	  VX (4, 1290),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhosumiaaw",	  VX (4, 1294),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbeusiaah",		  VX (4, 1296),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbessiaah",		  VX (4, 1297),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesusiaah",	  VX (4, 1298),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbousiaah",		  VX (4, 1300),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbossiaah",		  VX (4, 1301),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosusiaah",	  VX (4, 1302),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbeumiaah",		  VX (4, 1304),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesmiaah",		  VX (4, 1305),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesumiaah",	  VX (4, 1306),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmboumiaah",		  VX (4, 1308),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosmiaah",		  VX (4, 1309),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosumiaah",	  VX (4, 1310),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwlusiaaw3",	  VX (4, 1346),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwlssiaaw3",	  VX (4, 1347),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfraaw3",	  VX (4, 1348),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfaaw3",	  VX (4, 1349),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfraaw",	  VX (4, 1350),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfaaw",		  VX (4, 1351),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwlumiaaw3",	  VX (4, 1354),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwlsmiaaw3",	  VX (4, 1355),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwusiaa",		  VX (4, 1360),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwssiaa",		  VX (4, 1361),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwehgsmfraa",	  VX (4, 1366),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwehgsmfaa",	  VX (4, 1367),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwohgsmfraa",	  VX (4, 1374),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwohgsmfaa",	  VX (4, 1375),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhesusianw",	  VX (4, 1410),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhosusianw",	  VX (4, 1414),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhesumianw",	  VX (4, 1418),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmhosumianw",	  VX (4, 1422),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbeusianh",		  VX (4, 1424),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbessianh",		  VX (4, 1425),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesusianh",	  VX (4, 1426),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbousianh",		  VX (4, 1428),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbossianh",		  VX (4, 1429),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosusianh",	  VX (4, 1430),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbeumianh",		  VX (4, 1432),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesmianh",		  VX (4, 1433),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbesumianh",	  VX (4, 1434),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmboumianh",		  VX (4, 1436),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosmianh",		  VX (4, 1437),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmbosumianh",	  VX (4, 1438),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwlusianw3",	  VX (4, 1474),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwlssianw3",	  VX (4, 1475),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfranw3",	  VX (4, 1476),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfanw3",	  VX (4, 1477),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfranw",	  VX (4, 1478),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwhssfanw",		  VX (4, 1479),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwlumianw3",	  VX (4, 1482),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwlsmianw3",	  VX (4, 1483),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwusian",		  VX (4, 1488),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwssian",		  VX (4, 1489),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwehgsmfran",	  VX (4, 1494),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwehgsmfan",	  VX (4, 1495),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwohgsmfran",	  VX (4, 1502),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmwohgsmfan",	  VX (4, 1503),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evseteqb",		  VX (4, 1536),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evseteqb.",		  VX (4, 1537),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evseteqh",		  VX (4, 1538),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evseteqh.",		  VX (4, 1539),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evseteqw",		  VX (4, 1540),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evseteqw.",		  VX (4, 1541),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgthu",		  VX (4, 1544),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgthu.",		  VX (4, 1545),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgths",		  VX (4, 1546),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgths.",		  VX (4, 1547),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgtwu",		  VX (4, 1548),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgtwu.",		  VX (4, 1549),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgtws",		  VX (4, 1550),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgtws.",		  VX (4, 1551),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgtbu",		  VX (4, 1552),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgtbu.",		  VX (4, 1553),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgtbs",		  VX (4, 1554),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetgtbs.",		  VX (4, 1555),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetltbu",		  VX (4, 1556),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetltbu.",		  VX (4, 1557),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetltbs",		  VX (4, 1558),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetltbs.",		  VX (4, 1559),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetlthu",		  VX (4, 1560),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetlthu.",		  VX (4, 1561),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetlths",		  VX (4, 1562),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetlths.",		  VX (4, 1563),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetltwu",		  VX (4, 1564),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetltwu.",		  VX (4, 1565),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetltws",		  VX (4, 1566),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsetltws.",		  VX (4, 1567),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsaduw",		  VX (4, 1568),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsadsw",		  VX (4, 1569),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad4ub",		  VX (4, 1570),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad4sb",		  VX (4, 1571),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad2uh",		  VX (4, 1572),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad2sh",		  VX (4, 1573),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsaduwa",		  VX (4, 1576),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsadswa",		  VX (4, 1577),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad4uba",		  VX (4, 1578),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad4sba",		  VX (4, 1579),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad2uha",		  VX (4, 1580),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad2sha",		  VX (4, 1581),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evabsdifuw",		  VX (4, 1584),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evabsdifsw",		  VX (4, 1585),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evabsdifub",		  VX (4, 1586),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evabsdifsb",		  VX (4, 1587),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evabsdifuh",		  VX (4, 1588),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evabsdifsh",		  VX (4, 1589),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsaduwaa",		  VX (4, 1592),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsadswaa",		  VX (4, 1593),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad4ubaaw",		  VX (4, 1594),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad4sbaaw",		  VX (4, 1595),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad2uhaaw",		  VX (4, 1596),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evsad2shaaw",		  VX (4, 1597),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkshubs",		  VX (4, 1600),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkshsbs",		  VX (4, 1601),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkswuhs",		  VX (4, 1602),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkswshs",		  VX (4, 1603),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkuhubs",		  VX (4, 1604),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkuwuhs",		  VX (4, 1605),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkswshilvs",	  VX (4, 1606),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkswgshefrs",	  VX (4, 1607),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkswshfrs",		  VX (4, 1608),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkswshilvfrs",	  VX (4, 1609),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpksdswfrs",		  VX (4, 1610),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpksdshefrs",	  VX (4, 1611),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkuduws",		  VX (4, 1612),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpksdsws",		  VX (4, 1613),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evpkswgswfrs",	  VX (4, 1614),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evilveh",		  VX (4, 1616),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evilveoh",		  VX (4, 1617),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evilvhih",		  VX (4, 1618),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evilvhiloh",		  VX (4, 1619),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evilvloh",		  VX (4, 1620),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evilvlohih",		  VX (4, 1621),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evilvoeh",		  VX (4, 1622),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evilvoh",		  VX (4, 1623),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdlveb",		  VX (4, 1624),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdlveh",		  VX (4, 1625),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdlveob",		  VX (4, 1626),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdlveoh",		  VX (4, 1627),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdlvob",		  VX (4, 1628),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdlvoh",		  VX (4, 1629),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdlvoeb",		  VX (4, 1630),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evdlvoeh",		  VX (4, 1631),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmaxbu",		  VX (4, 1632),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmaxbs",		  VX (4, 1633),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmaxhu",		  VX (4, 1634),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmaxhs",		  VX (4, 1635),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmaxwu",		  VX (4, 1636),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmaxws",		  VX (4, 1637),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmaxdu",		  VX (4, 1638),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmaxds",		  VX (4, 1639),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evminbu",		  VX (4, 1640),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evminbs",		  VX (4, 1641),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evminhu",		  VX (4, 1642),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evminhs",		  VX (4, 1643),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evminwu",		  VX (4, 1644),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evminws",		  VX (4, 1645),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evmindu",		  VX (4, 1646),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evminds",		  VX (4, 1647),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgwu",		  VX (4, 1648),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgws",		  VX (4, 1649),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgbu",		  VX (4, 1650),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgbs",		  VX (4, 1651),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavghu",		  VX (4, 1652),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavghs",		  VX (4, 1653),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgdu",		  VX (4, 1654),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgds",		  VX (4, 1655),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgwur",		  VX (4, 1656),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgwsr",		  VX (4, 1657),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgbur",		  VX (4, 1658),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgbsr",		  VX (4, 1659),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavghur",		  VX (4, 1660),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavghsr",		  VX (4, 1661),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgdur",		  VX (4, 1662),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
{"evavgdsr",		  VX (4, 1663),		VX_MASK,		PPCSPE2, 0, {RD, RA, RB}},
};

const unsigned int spe2_num_opcodes = ARRAY_SIZE (spe2_opcodes);
