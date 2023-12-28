/* aarch64-dis.c -- AArch64 disassembler.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

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
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include <stdint.h>
#include "disassemble.h"
#include "libiberty.h"
#include "opintl.h"
#include "aarch64-dis.h"
#include "elf-bfd.h"
#include "safe-ctype.h"
#include "obstack.h"

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free

#define INSNLEN 4

/* This character is used to encode style information within the output
   buffers.  See get_style_text and print_operands for more details.  */
#define STYLE_MARKER_CHAR '\002'

/* Cached mapping symbol state.  */
enum map_type
{
  MAP_INSN,
  MAP_DATA
};

static aarch64_feature_set arch_variant; /* See select_aarch64_variant.  */
static enum map_type last_type;
static int last_mapping_sym = -1;
static bfd_vma last_stop_offset = 0;
static bfd_vma last_mapping_addr = 0;

/* Other options */
static int no_aliases = 0;	/* If set disassemble as most general inst.  */
static int no_notes = 1;	/* If set do not print disassemble notes in the
				  output as comments.  */

/* Currently active instruction sequence.  */
static aarch64_instr_sequence insn_sequence;

static void
set_default_aarch64_dis_options (struct disassemble_info *info ATTRIBUTE_UNUSED)
{
}

static void
parse_aarch64_dis_option (const char *option, unsigned int len ATTRIBUTE_UNUSED)
{
  /* Try to match options that are simple flags */
  if (startswith (option, "no-aliases"))
    {
      no_aliases = 1;
      return;
    }

  if (startswith (option, "aliases"))
    {
      no_aliases = 0;
      return;
    }

  if (startswith (option, "no-notes"))
    {
      no_notes = 1;
      return;
    }

  if (startswith (option, "notes"))
    {
      no_notes = 0;
      return;
    }

#ifdef DEBUG_AARCH64
  if (startswith (option, "debug_dump"))
    {
      debug_dump = 1;
      return;
    }
#endif /* DEBUG_AARCH64 */

  /* Invalid option.  */
  opcodes_error_handler (_("unrecognised disassembler option: %s"), option);
}

static void
parse_aarch64_dis_options (const char *options)
{
  const char *option_end;

  if (options == NULL)
    return;

  while (*options != '\0')
    {
      /* Skip empty options.  */
      if (*options == ',')
	{
	  options++;
	  continue;
	}

      /* We know that *options is neither NUL or a comma.  */
      option_end = options + 1;
      while (*option_end != ',' && *option_end != '\0')
	option_end++;

      parse_aarch64_dis_option (options, option_end - options);

      /* Go on to the next one.  If option_end points to a comma, it
	 will be skipped above.  */
      options = option_end;
    }
}

/* Functions doing the instruction disassembling.  */

/* The unnamed arguments consist of the number of fields and information about
   these fields where the VALUE will be extracted from CODE and returned.
   MASK can be zero or the base mask of the opcode.

   N.B. the fields are required to be in such an order than the most signficant
   field for VALUE comes the first, e.g. the <index> in
    SQDMLAL <Va><d>, <Vb><n>, <Vm>.<Ts>[<index>]
   is encoded in H:L:M in some cases, the fields H:L:M should be passed in
   the order of H, L, M.  */

aarch64_insn
extract_fields (aarch64_insn code, aarch64_insn mask, ...)
{
  uint32_t num;
  const aarch64_field *field;
  enum aarch64_field_kind kind;
  va_list va;

  va_start (va, mask);
  num = va_arg (va, uint32_t);
  assert (num <= 5);
  aarch64_insn value = 0x0;
  while (num--)
    {
      kind = va_arg (va, enum aarch64_field_kind);
      field = &fields[kind];
      value <<= field->width;
      value |= extract_field (kind, code, mask);
    }
  va_end (va);
  return value;
}

/* Extract the value of all fields in SELF->fields after START from
   instruction CODE.  The least significant bit comes from the final field.  */

static aarch64_insn
extract_all_fields_after (const aarch64_operand *self, unsigned int start,
			  aarch64_insn code)
{
  aarch64_insn value;
  unsigned int i;
  enum aarch64_field_kind kind;

  value = 0;
  for (i = start;
       i < ARRAY_SIZE (self->fields) && self->fields[i] != FLD_NIL; ++i)
    {
      kind = self->fields[i];
      value <<= fields[kind].width;
      value |= extract_field (kind, code, 0);
    }
  return value;
}

/* Extract the value of all fields in SELF->fields from instruction CODE.
   The least significant bit comes from the final field.  */

static aarch64_insn
extract_all_fields (const aarch64_operand *self, aarch64_insn code)
{
  return extract_all_fields_after (self, 0, code);
}

/* Sign-extend bit I of VALUE.  */
static inline uint64_t
sign_extend (aarch64_insn value, unsigned i)
{
  uint64_t ret, sign;

  assert (i < 32);
  ret = value;
  sign = (uint64_t) 1 << i;
  return ((ret & (sign + sign - 1)) ^ sign) - sign;
}

/* N.B. the following inline helpfer functions create a dependency on the
   order of operand qualifier enumerators.  */

/* Given VALUE, return qualifier for a general purpose register.  */
static inline enum aarch64_opnd_qualifier
get_greg_qualifier_from_value (aarch64_insn value)
{
  enum aarch64_opnd_qualifier qualifier = AARCH64_OPND_QLF_W + value;
  assert (value <= 0x1
	  && aarch64_get_qualifier_standard_value (qualifier) == value);
  return qualifier;
}

/* Given VALUE, return qualifier for a vector register.  This does not support
   decoding instructions that accept the 2H vector type.  */

static inline enum aarch64_opnd_qualifier
get_vreg_qualifier_from_value (aarch64_insn value)
{
  enum aarch64_opnd_qualifier qualifier = AARCH64_OPND_QLF_V_8B + value;

  /* Instructions using vector type 2H should not call this function.  Skip over
     the 2H qualifier.  */
  if (qualifier >= AARCH64_OPND_QLF_V_2H)
    qualifier += 1;

  assert (value <= 0x8
	  && aarch64_get_qualifier_standard_value (qualifier) == value);
  return qualifier;
}

/* Given VALUE, return qualifier for an FP or AdvSIMD scalar register.  */
static inline enum aarch64_opnd_qualifier
get_sreg_qualifier_from_value (aarch64_insn value)
{
  enum aarch64_opnd_qualifier qualifier = AARCH64_OPND_QLF_S_B + value;

  assert (value <= 0x4
	  && aarch64_get_qualifier_standard_value (qualifier) == value);
  return qualifier;
}

/* Given the instruction in *INST which is probably half way through the
   decoding and our caller wants to know the expected qualifier for operand
   I.  Return such a qualifier if we can establish it; otherwise return
   AARCH64_OPND_QLF_NIL.  */

static aarch64_opnd_qualifier_t
get_expected_qualifier (const aarch64_inst *inst, int i)
{
  aarch64_opnd_qualifier_seq_t qualifiers;
  /* Should not be called if the qualifier is known.  */
  assert (inst->operands[i].qualifier == AARCH64_OPND_QLF_NIL);
  int invalid_count;
  if (aarch64_find_best_match (inst, inst->opcode->qualifiers_list,
			       i, qualifiers, &invalid_count))
    return qualifiers[i];
  else
    return AARCH64_OPND_QLF_NIL;
}

/* Operand extractors.  */

bool
aarch64_ext_none (const aarch64_operand *self ATTRIBUTE_UNUSED,
		  aarch64_opnd_info *info ATTRIBUTE_UNUSED,
		  const aarch64_insn code ATTRIBUTE_UNUSED,
		  const aarch64_inst *inst ATTRIBUTE_UNUSED,
		  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  return true;
}

bool
aarch64_ext_regno (const aarch64_operand *self, aarch64_opnd_info *info,
		   const aarch64_insn code,
		   const aarch64_inst *inst ATTRIBUTE_UNUSED,
		   aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->reg.regno = (extract_field (self->fields[0], code, 0)
		     + get_operand_specific_data (self));
  return true;
}

bool
aarch64_ext_regno_pair (const aarch64_operand *self ATTRIBUTE_UNUSED, aarch64_opnd_info *info,
		   const aarch64_insn code ATTRIBUTE_UNUSED,
		   const aarch64_inst *inst ATTRIBUTE_UNUSED,
		   aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  assert (info->idx == 1
	  || info->idx ==3);
  info->reg.regno = inst->operands[info->idx - 1].reg.regno + 1;
  return true;
}

/* e.g. IC <ic_op>{, <Xt>}.  */
bool
aarch64_ext_regrt_sysins (const aarch64_operand *self, aarch64_opnd_info *info,
			  const aarch64_insn code,
			  const aarch64_inst *inst ATTRIBUTE_UNUSED,
			  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->reg.regno = extract_field (self->fields[0], code, 0);
  assert (info->idx == 1
	  && (aarch64_get_operand_class (inst->operands[0].type)
	      == AARCH64_OPND_CLASS_SYSTEM));
  /* This will make the constraint checking happy and more importantly will
     help the disassembler determine whether this operand is optional or
     not.  */
  info->present = aarch64_sys_ins_reg_has_xt (inst->operands[0].sysins_op);

  return true;
}

/* e.g. SQDMLAL <Va><d>, <Vb><n>, <Vm>.<Ts>[<index>].  */
bool
aarch64_ext_reglane (const aarch64_operand *self, aarch64_opnd_info *info,
		     const aarch64_insn code,
		     const aarch64_inst *inst ATTRIBUTE_UNUSED,
		     aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* regno */
  info->reglane.regno = extract_field (self->fields[0], code,
				       inst->opcode->mask);

  /* Index and/or type.  */
  if (inst->opcode->iclass == asisdone
    || inst->opcode->iclass == asimdins)
    {
      if (info->type == AARCH64_OPND_En
	  && inst->opcode->operands[0] == AARCH64_OPND_Ed)
	{
	  unsigned shift;
	  /* index2 for e.g. INS <Vd>.<Ts>[<index1>], <Vn>.<Ts>[<index2>].  */
	  assert (info->idx == 1);	/* Vn */
	  aarch64_insn value = extract_field (FLD_imm4_11, code, 0);
	  /* Depend on AARCH64_OPND_Ed to determine the qualifier.  */
	  info->qualifier = get_expected_qualifier (inst, info->idx);
	  shift = get_logsz (aarch64_get_qualifier_esize (info->qualifier));
	  info->reglane.index = value >> shift;
	}
      else
	{
	  /* index and type for e.g. DUP <V><d>, <Vn>.<T>[<index>].
	     imm5<3:0>	<V>
	     0000	RESERVED
	     xxx1	B
	     xx10	H
	     x100	S
	     1000	D  */
	  int pos = -1;
	  aarch64_insn value = extract_field (FLD_imm5, code, 0);
	  while (++pos <= 3 && (value & 0x1) == 0)
	    value >>= 1;
	  if (pos > 3)
	    return false;
	  info->qualifier = get_sreg_qualifier_from_value (pos);
	  info->reglane.index = (unsigned) (value >> 1);
	}
    }
  else if (inst->opcode->iclass == dotproduct)
    {
      /* Need information in other operand(s) to help decoding.  */
      info->qualifier = get_expected_qualifier (inst, info->idx);
      switch (info->qualifier)
	{
	case AARCH64_OPND_QLF_S_4B:
	case AARCH64_OPND_QLF_S_2H:
	  /* L:H */
	  info->reglane.index = extract_fields (code, 0, 2, FLD_H, FLD_L);
	  info->reglane.regno &= 0x1f;
	  break;
	default:
	  return false;
	}
    }
  else if (inst->opcode->iclass == cryptosm3)
    {
      /* index for e.g. SM3TT2A <Vd>.4S, <Vn>.4S, <Vm>S[<imm2>].  */
      info->reglane.index = extract_field (FLD_SM3_imm2, code, 0);
    }
  else
    {
      /* Index only for e.g. SQDMLAL <Va><d>, <Vb><n>, <Vm>.<Ts>[<index>]
         or SQDMLAL <Va><d>, <Vb><n>, <Vm>.<Ts>[<index>].  */

      /* Need information in other operand(s) to help decoding.  */
      info->qualifier = get_expected_qualifier (inst, info->idx);
      switch (info->qualifier)
	{
	case AARCH64_OPND_QLF_S_H:
	  if (info->type == AARCH64_OPND_Em16)
	    {
	      /* h:l:m */
	      info->reglane.index = extract_fields (code, 0, 3, FLD_H, FLD_L,
						    FLD_M);
	      info->reglane.regno &= 0xf;
	    }
	  else
	    {
	      /* h:l */
	      info->reglane.index = extract_fields (code, 0, 2, FLD_H, FLD_L);
	    }
	  break;
	case AARCH64_OPND_QLF_S_S:
	  /* h:l */
	  info->reglane.index = extract_fields (code, 0, 2, FLD_H, FLD_L);
	  break;
	case AARCH64_OPND_QLF_S_D:
	  /* H */
	  info->reglane.index = extract_field (FLD_H, code, 0);
	  break;
	default:
	  return false;
	}

      if (inst->opcode->op == OP_FCMLA_ELEM
	  && info->qualifier != AARCH64_OPND_QLF_S_H)
	{
	  /* Complex operand takes two elements.  */
	  if (info->reglane.index & 1)
	    return false;
	  info->reglane.index /= 2;
	}
    }

  return true;
}

bool
aarch64_ext_reglist (const aarch64_operand *self, aarch64_opnd_info *info,
		     const aarch64_insn code,
		     const aarch64_inst *inst ATTRIBUTE_UNUSED,
		     aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* R */
  info->reglist.first_regno = extract_field (self->fields[0], code, 0);
  /* len */
  info->reglist.num_regs = extract_field (FLD_len, code, 0) + 1;
  info->reglist.stride = 1;
  return true;
}

/* Decode Rt and opcode fields of Vt in AdvSIMD load/store instructions.  */
bool
aarch64_ext_ldst_reglist (const aarch64_operand *self ATTRIBUTE_UNUSED,
			  aarch64_opnd_info *info, const aarch64_insn code,
			  const aarch64_inst *inst,
			  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn value;
  /* Number of elements in each structure to be loaded/stored.  */
  unsigned expected_num = get_opcode_dependent_value (inst->opcode);

  struct
    {
      unsigned is_reserved;
      unsigned num_regs;
      unsigned num_elements;
    } data [] =
  {   {0, 4, 4},
      {1, 4, 4},
      {0, 4, 1},
      {0, 4, 2},
      {0, 3, 3},
      {1, 3, 3},
      {0, 3, 1},
      {0, 1, 1},
      {0, 2, 2},
      {1, 2, 2},
      {0, 2, 1},
  };

  /* Rt */
  info->reglist.first_regno = extract_field (FLD_Rt, code, 0);
  /* opcode */
  value = extract_field (FLD_opcode, code, 0);
  /* PR 21595: Check for a bogus value.  */
  if (value >= ARRAY_SIZE (data))
    return false;
  if (expected_num != data[value].num_elements || data[value].is_reserved)
    return false;
  info->reglist.num_regs = data[value].num_regs;
  info->reglist.stride = 1;

  return true;
}

/* Decode Rt and S fields of Vt in AdvSIMD load single structure to all
   lanes instructions.  */
bool
aarch64_ext_ldst_reglist_r (const aarch64_operand *self ATTRIBUTE_UNUSED,
			    aarch64_opnd_info *info, const aarch64_insn code,
			    const aarch64_inst *inst,
			    aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn value;

  /* Rt */
  info->reglist.first_regno = extract_field (FLD_Rt, code, 0);
  /* S */
  value = extract_field (FLD_S, code, 0);

  /* Number of registers is equal to the number of elements in
     each structure to be loaded/stored.  */
  info->reglist.num_regs = get_opcode_dependent_value (inst->opcode);
  assert (info->reglist.num_regs >= 1 && info->reglist.num_regs <= 4);

  /* Except when it is LD1R.  */
  if (info->reglist.num_regs == 1 && value == (aarch64_insn) 1)
    info->reglist.num_regs = 2;

  info->reglist.stride = 1;
  return true;
}

/* Decode Q, opcode<2:1>, S, size and Rt fields of Vt in AdvSIMD
   load/store single element instructions.  */
bool
aarch64_ext_ldst_elemlist (const aarch64_operand *self ATTRIBUTE_UNUSED,
			   aarch64_opnd_info *info, const aarch64_insn code,
			   const aarch64_inst *inst ATTRIBUTE_UNUSED,
			   aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_field field = {0, 0};
  aarch64_insn QSsize;		/* fields Q:S:size.  */
  aarch64_insn opcodeh2;	/* opcode<2:1> */

  /* Rt */
  info->reglist.first_regno = extract_field (FLD_Rt, code, 0);

  /* Decode the index, opcode<2:1> and size.  */
  gen_sub_field (FLD_asisdlso_opcode, 1, 2, &field);
  opcodeh2 = extract_field_2 (&field, code, 0);
  QSsize = extract_fields (code, 0, 3, FLD_Q, FLD_S, FLD_vldst_size);
  switch (opcodeh2)
    {
    case 0x0:
      info->qualifier = AARCH64_OPND_QLF_S_B;
      /* Index encoded in "Q:S:size".  */
      info->reglist.index = QSsize;
      break;
    case 0x1:
      if (QSsize & 0x1)
	/* UND.  */
	return false;
      info->qualifier = AARCH64_OPND_QLF_S_H;
      /* Index encoded in "Q:S:size<1>".  */
      info->reglist.index = QSsize >> 1;
      break;
    case 0x2:
      if ((QSsize >> 1) & 0x1)
	/* UND.  */
	return false;
      if ((QSsize & 0x1) == 0)
	{
	  info->qualifier = AARCH64_OPND_QLF_S_S;
	  /* Index encoded in "Q:S".  */
	  info->reglist.index = QSsize >> 2;
	}
      else
	{
	  if (extract_field (FLD_S, code, 0))
	    /* UND */
	    return false;
	  info->qualifier = AARCH64_OPND_QLF_S_D;
	  /* Index encoded in "Q".  */
	  info->reglist.index = QSsize >> 3;
	}
      break;
    default:
      return false;
    }

  info->reglist.has_index = 1;
  info->reglist.num_regs = 0;
  info->reglist.stride = 1;
  /* Number of registers is equal to the number of elements in
     each structure to be loaded/stored.  */
  info->reglist.num_regs = get_opcode_dependent_value (inst->opcode);
  assert (info->reglist.num_regs >= 1 && info->reglist.num_regs <= 4);

  return true;
}

/* Decode fields immh:immb and/or Q for e.g.
   SSHR <Vd>.<T>, <Vn>.<T>, #<shift>
   or SSHR <V><d>, <V><n>, #<shift>.  */

bool
aarch64_ext_advsimd_imm_shift (const aarch64_operand *self ATTRIBUTE_UNUSED,
			       aarch64_opnd_info *info, const aarch64_insn code,
			       const aarch64_inst *inst,
			       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int pos;
  aarch64_insn Q, imm, immh;
  enum aarch64_insn_class iclass = inst->opcode->iclass;

  immh = extract_field (FLD_immh, code, 0);
  if (immh == 0)
    return false;
  imm = extract_fields (code, 0, 2, FLD_immh, FLD_immb);
  pos = 4;
  /* Get highest set bit in immh.  */
  while (--pos >= 0 && (immh & 0x8) == 0)
    immh <<= 1;

  assert ((iclass == asimdshf || iclass == asisdshf)
	  && (info->type == AARCH64_OPND_IMM_VLSR
	      || info->type == AARCH64_OPND_IMM_VLSL));

  if (iclass == asimdshf)
    {
      Q = extract_field (FLD_Q, code, 0);
      /* immh	Q	<T>
	 0000	x	SEE AdvSIMD modified immediate
	 0001	0	8B
	 0001	1	16B
	 001x	0	4H
	 001x	1	8H
	 01xx	0	2S
	 01xx	1	4S
	 1xxx	0	RESERVED
	 1xxx	1	2D  */
      info->qualifier =
	get_vreg_qualifier_from_value ((pos << 1) | (int) Q);
    }
  else
    info->qualifier = get_sreg_qualifier_from_value (pos);

  if (info->type == AARCH64_OPND_IMM_VLSR)
    /* immh	<shift>
       0000	SEE AdvSIMD modified immediate
       0001	(16-UInt(immh:immb))
       001x	(32-UInt(immh:immb))
       01xx	(64-UInt(immh:immb))
       1xxx	(128-UInt(immh:immb))  */
    info->imm.value = (16 << pos) - imm;
  else
    /* immh:immb
       immh	<shift>
       0000	SEE AdvSIMD modified immediate
       0001	(UInt(immh:immb)-8)
       001x	(UInt(immh:immb)-16)
       01xx	(UInt(immh:immb)-32)
       1xxx	(UInt(immh:immb)-64)  */
    info->imm.value = imm - (8 << pos);

  return true;
}

/* Decode shift immediate for e.g. sshr (imm).  */
bool
aarch64_ext_shll_imm (const aarch64_operand *self ATTRIBUTE_UNUSED,
		      aarch64_opnd_info *info, const aarch64_insn code,
		      const aarch64_inst *inst ATTRIBUTE_UNUSED,
		      aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int64_t imm;
  aarch64_insn val;
  val = extract_field (FLD_size, code, 0);
  switch (val)
    {
    case 0: imm = 8; break;
    case 1: imm = 16; break;
    case 2: imm = 32; break;
    default: return false;
    }
  info->imm.value = imm;
  return true;
}

/* Decode imm for e.g. BFM <Wd>, <Wn>, #<immr>, #<imms>.
   value in the field(s) will be extracted as unsigned immediate value.  */
bool
aarch64_ext_imm (const aarch64_operand *self, aarch64_opnd_info *info,
		 const aarch64_insn code,
		 const aarch64_inst *inst,
		 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  uint64_t imm;

  imm = extract_all_fields (self, code);

  if (operand_need_sign_extension (self))
    imm = sign_extend (imm, get_operand_fields_width (self) - 1);

  if (operand_need_shift_by_two (self))
    imm <<= 2;
  else if (operand_need_shift_by_three (self))
    imm <<= 3;
  else if (operand_need_shift_by_four (self))
    imm <<= 4;

  if (info->type == AARCH64_OPND_ADDR_ADRP)
    imm <<= 12;

  if (inst->operands[0].type == AARCH64_OPND_PSTATEFIELD
      && inst->operands[0].sysreg.flags & F_IMM_IN_CRM)
    imm &= PSTATE_DECODE_CRM_IMM (inst->operands[0].sysreg.flags);

  info->imm.value = imm;
  return true;
}

/* Decode imm and its shifter for e.g. MOVZ <Wd>, #<imm16>{, LSL #<shift>}.  */
bool
aarch64_ext_imm_half (const aarch64_operand *self, aarch64_opnd_info *info,
		      const aarch64_insn code,
		      const aarch64_inst *inst ATTRIBUTE_UNUSED,
		      aarch64_operand_error *errors)
{
  aarch64_ext_imm (self, info, code, inst, errors);
  info->shifter.kind = AARCH64_MOD_LSL;
  info->shifter.amount = extract_field (FLD_hw, code, 0) << 4;
  return true;
}

/* Decode cmode and "a:b:c:d:e:f:g:h" for e.g.
     MOVI <Vd>.<T>, #<imm8> {, LSL #<amount>}.  */
bool
aarch64_ext_advsimd_imm_modified (const aarch64_operand *self ATTRIBUTE_UNUSED,
				  aarch64_opnd_info *info,
				  const aarch64_insn code,
				  const aarch64_inst *inst ATTRIBUTE_UNUSED,
				  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  uint64_t imm;
  enum aarch64_opnd_qualifier opnd0_qualifier = inst->operands[0].qualifier;
  aarch64_field field = {0, 0};

  assert (info->idx == 1);

  if (info->type == AARCH64_OPND_SIMD_FPIMM)
    info->imm.is_fp = 1;

  /* a:b:c:d:e:f:g:h */
  imm = extract_fields (code, 0, 2, FLD_abc, FLD_defgh);
  if (!info->imm.is_fp && aarch64_get_qualifier_esize (opnd0_qualifier) == 8)
    {
      /* Either MOVI <Dd>, #<imm>
	 or     MOVI <Vd>.2D, #<imm>.
	 <imm> is a 64-bit immediate
	 'aaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffffgggggggghhhhhhhh',
	 encoded in "a:b:c:d:e:f:g:h".	*/
      int i;
      unsigned abcdefgh = imm;
      for (imm = 0ull, i = 0; i < 8; i++)
	if (((abcdefgh >> i) & 0x1) != 0)
	  imm |= 0xffull << (8 * i);
    }
  info->imm.value = imm;

  /* cmode */
  info->qualifier = get_expected_qualifier (inst, info->idx);
  switch (info->qualifier)
    {
    case AARCH64_OPND_QLF_NIL:
      /* no shift */
      info->shifter.kind = AARCH64_MOD_NONE;
      return 1;
    case AARCH64_OPND_QLF_LSL:
      /* shift zeros */
      info->shifter.kind = AARCH64_MOD_LSL;
      switch (aarch64_get_qualifier_esize (opnd0_qualifier))
	{
	case 4: gen_sub_field (FLD_cmode, 1, 2, &field); break;	/* per word */
	case 2: gen_sub_field (FLD_cmode, 1, 1, &field); break;	/* per half */
	case 1: gen_sub_field (FLD_cmode, 1, 0, &field); break;	/* per byte */
	default: return false;
	}
      /* 00: 0; 01: 8; 10:16; 11:24.  */
      info->shifter.amount = extract_field_2 (&field, code, 0) << 3;
      break;
    case AARCH64_OPND_QLF_MSL:
      /* shift ones */
      info->shifter.kind = AARCH64_MOD_MSL;
      gen_sub_field (FLD_cmode, 0, 1, &field);		/* per word */
      info->shifter.amount = extract_field_2 (&field, code, 0) ? 16 : 8;
      break;
    default:
      return false;
    }

  return true;
}

/* Decode an 8-bit floating-point immediate.  */
bool
aarch64_ext_fpimm (const aarch64_operand *self, aarch64_opnd_info *info,
		   const aarch64_insn code,
		   const aarch64_inst *inst ATTRIBUTE_UNUSED,
		   aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->imm.value = extract_all_fields (self, code);
  info->imm.is_fp = 1;
  return true;
}

/* Decode a 1-bit rotate immediate (#90 or #270).  */
bool
aarch64_ext_imm_rotate1 (const aarch64_operand *self, aarch64_opnd_info *info,
			 const aarch64_insn code,
			 const aarch64_inst *inst ATTRIBUTE_UNUSED,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  uint64_t rot = extract_field (self->fields[0], code, 0);
  assert (rot < 2U);
  info->imm.value = rot * 180 + 90;
  return true;
}

/* Decode a 2-bit rotate immediate (#0, #90, #180 or #270).  */
bool
aarch64_ext_imm_rotate2 (const aarch64_operand *self, aarch64_opnd_info *info,
			 const aarch64_insn code,
			 const aarch64_inst *inst ATTRIBUTE_UNUSED,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  uint64_t rot = extract_field (self->fields[0], code, 0);
  assert (rot < 4U);
  info->imm.value = rot * 90;
  return true;
}

/* Decode scale for e.g. SCVTF <Dd>, <Wn>, #<fbits>.  */
bool
aarch64_ext_fbits (const aarch64_operand *self ATTRIBUTE_UNUSED,
		   aarch64_opnd_info *info, const aarch64_insn code,
		   const aarch64_inst *inst ATTRIBUTE_UNUSED,
		   aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->imm.value = 64- extract_field (FLD_scale, code, 0);
  return true;
}

/* Decode arithmetic immediate for e.g.
     SUBS <Wd>, <Wn|WSP>, #<imm> {, <shift>}.  */
bool
aarch64_ext_aimm (const aarch64_operand *self ATTRIBUTE_UNUSED,
		  aarch64_opnd_info *info, const aarch64_insn code,
		  const aarch64_inst *inst ATTRIBUTE_UNUSED,
		  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn value;

  info->shifter.kind = AARCH64_MOD_LSL;
  /* shift */
  value = extract_field (FLD_shift, code, 0);
  if (value >= 2)
    return false;
  info->shifter.amount = value ? 12 : 0;
  /* imm12 (unsigned) */
  info->imm.value = extract_field (FLD_imm12, code, 0);

  return true;
}

/* Return true if VALUE is a valid logical immediate encoding, storing the
   decoded value in *RESULT if so.  ESIZE is the number of bytes in the
   decoded immediate.  */
static bool
decode_limm (uint32_t esize, aarch64_insn value, int64_t *result)
{
  uint64_t imm, mask;
  uint32_t N, R, S;
  unsigned simd_size;

  /* value is N:immr:imms.  */
  S = value & 0x3f;
  R = (value >> 6) & 0x3f;
  N = (value >> 12) & 0x1;

  /* The immediate value is S+1 bits to 1, left rotated by SIMDsize - R
     (in other words, right rotated by R), then replicated.  */
  if (N != 0)
    {
      simd_size = 64;
      mask = 0xffffffffffffffffull;
    }
  else
    {
      switch (S)
	{
	case 0x00 ... 0x1f: /* 0xxxxx */ simd_size = 32;           break;
	case 0x20 ... 0x2f: /* 10xxxx */ simd_size = 16; S &= 0xf; break;
	case 0x30 ... 0x37: /* 110xxx */ simd_size =  8; S &= 0x7; break;
	case 0x38 ... 0x3b: /* 1110xx */ simd_size =  4; S &= 0x3; break;
	case 0x3c ... 0x3d: /* 11110x */ simd_size =  2; S &= 0x1; break;
	default: return false;
	}
      mask = (1ull << simd_size) - 1;
      /* Top bits are IGNORED.  */
      R &= simd_size - 1;
    }

  if (simd_size > esize * 8)
    return false;

  /* NOTE: if S = simd_size - 1 we get 0xf..f which is rejected.  */
  if (S == simd_size - 1)
    return false;
  /* S+1 consecutive bits to 1.  */
  /* NOTE: S can't be 63 due to detection above.  */
  imm = (1ull << (S + 1)) - 1;
  /* Rotate to the left by simd_size - R.  */
  if (R != 0)
    imm = ((imm << (simd_size - R)) & mask) | (imm >> R);
  /* Replicate the value according to SIMD size.  */
  switch (simd_size)
    {
    case  2: imm = (imm <<  2) | imm;
      /* Fall through.  */
    case  4: imm = (imm <<  4) | imm;
      /* Fall through.  */
    case  8: imm = (imm <<  8) | imm;
      /* Fall through.  */
    case 16: imm = (imm << 16) | imm;
      /* Fall through.  */
    case 32: imm = (imm << 32) | imm;
      /* Fall through.  */
    case 64: break;
    default: return 0;
    }

  *result = imm & ~((uint64_t) -1 << (esize * 4) << (esize * 4));

  return true;
}

/* Decode a logical immediate for e.g. ORR <Wd|WSP>, <Wn>, #<imm>.  */
bool
aarch64_ext_limm (const aarch64_operand *self,
		  aarch64_opnd_info *info, const aarch64_insn code,
		  const aarch64_inst *inst,
		  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  uint32_t esize;
  aarch64_insn value;

  value = extract_fields (code, 0, 3, self->fields[0], self->fields[1],
			  self->fields[2]);
  esize = aarch64_get_qualifier_esize (inst->operands[0].qualifier);
  return decode_limm (esize, value, &info->imm.value);
}

/* Decode a logical immediate for the BIC alias of AND (etc.).  */
bool
aarch64_ext_inv_limm (const aarch64_operand *self,
		      aarch64_opnd_info *info, const aarch64_insn code,
		      const aarch64_inst *inst,
		      aarch64_operand_error *errors)
{
  if (!aarch64_ext_limm (self, info, code, inst, errors))
    return false;
  info->imm.value = ~info->imm.value;
  return true;
}

/* Decode Ft for e.g. STR <Qt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}]
   or LDP <Qt1>, <Qt2>, [<Xn|SP>], #<imm>.  */
bool
aarch64_ext_ft (const aarch64_operand *self ATTRIBUTE_UNUSED,
		aarch64_opnd_info *info,
		const aarch64_insn code, const aarch64_inst *inst,
		aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn value;

  /* Rt */
  info->reg.regno = extract_field (FLD_Rt, code, 0);

  /* size */
  value = extract_field (FLD_ldst_size, code, 0);
  if (inst->opcode->iclass == ldstpair_indexed
      || inst->opcode->iclass == ldstnapair_offs
      || inst->opcode->iclass == ldstpair_off
      || inst->opcode->iclass == loadlit)
    {
      enum aarch64_opnd_qualifier qualifier;
      switch (value)
	{
	case 0: qualifier = AARCH64_OPND_QLF_S_S; break;
	case 1: qualifier = AARCH64_OPND_QLF_S_D; break;
	case 2: qualifier = AARCH64_OPND_QLF_S_Q; break;
	default: return false;
	}
      info->qualifier = qualifier;
    }
  else
    {
      /* opc1:size */
      value = extract_fields (code, 0, 2, FLD_opc1, FLD_ldst_size);
      if (value > 0x4)
	return false;
      info->qualifier = get_sreg_qualifier_from_value (value);
    }

  return true;
}

/* Decode the address operand for e.g. STXRB <Ws>, <Wt>, [<Xn|SP>{,#0}].  */
bool
aarch64_ext_addr_simple (const aarch64_operand *self ATTRIBUTE_UNUSED,
			 aarch64_opnd_info *info,
			 aarch64_insn code,
			 const aarch64_inst *inst ATTRIBUTE_UNUSED,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* Rn */
  info->addr.base_regno = extract_field (FLD_Rn, code, 0);
  return true;
}

/* Decode the address operand for e.g.
     stlur <Xt>, [<Xn|SP>{, <amount>}].  */
bool
aarch64_ext_addr_offset (const aarch64_operand *self ATTRIBUTE_UNUSED,
			 aarch64_opnd_info *info,
			 aarch64_insn code, const aarch64_inst *inst,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->qualifier = get_expected_qualifier (inst, info->idx);

  /* Rn */
  info->addr.base_regno = extract_field (self->fields[0], code, 0);

  /* simm9 */
  aarch64_insn imm = extract_fields (code, 0, 1, self->fields[1]);
  info->addr.offset.imm = sign_extend (imm, 8);
  if (extract_field (self->fields[2], code, 0) == 1) {
    info->addr.writeback = 1;
    info->addr.preind = 1;
  }
  return true;
}

/* Decode the address operand for e.g.
     STR <Qt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
bool
aarch64_ext_addr_regoff (const aarch64_operand *self ATTRIBUTE_UNUSED,
			 aarch64_opnd_info *info,
			 aarch64_insn code, const aarch64_inst *inst,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn S, value;

  /* Rn */
  info->addr.base_regno = extract_field (FLD_Rn, code, 0);
  /* Rm */
  info->addr.offset.regno = extract_field (FLD_Rm, code, 0);
  /* option */
  value = extract_field (FLD_option, code, 0);
  info->shifter.kind =
    aarch64_get_operand_modifier_from_value (value, true /* extend_p */);
  /* Fix-up the shifter kind; although the table-driven approach is
     efficient, it is slightly inflexible, thus needing this fix-up.  */
  if (info->shifter.kind == AARCH64_MOD_UXTX)
    info->shifter.kind = AARCH64_MOD_LSL;
  /* S */
  S = extract_field (FLD_S, code, 0);
  if (S == 0)
    {
      info->shifter.amount = 0;
      info->shifter.amount_present = 0;
    }
  else
    {
      int size;
      /* Need information in other operand(s) to help achieve the decoding
	 from 'S' field.  */
      info->qualifier = get_expected_qualifier (inst, info->idx);
      /* Get the size of the data element that is accessed, which may be
	 different from that of the source register size, e.g. in strb/ldrb.  */
      size = aarch64_get_qualifier_esize (info->qualifier);
      info->shifter.amount = get_logsz (size);
      info->shifter.amount_present = 1;
    }

  return true;
}

/* Decode the address operand for e.g. LDRSW <Xt>, [<Xn|SP>], #<simm>.  */
bool
aarch64_ext_addr_simm (const aarch64_operand *self, aarch64_opnd_info *info,
		       aarch64_insn code, const aarch64_inst *inst,
		       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn imm;
  info->qualifier = get_expected_qualifier (inst, info->idx);

  /* Rn */
  info->addr.base_regno = extract_field (FLD_Rn, code, 0);
  /* simm (imm9 or imm7)  */
  imm = extract_field (self->fields[0], code, 0);
  info->addr.offset.imm = sign_extend (imm, fields[self->fields[0]].width - 1);
  if (self->fields[0] == FLD_imm7
      || info->qualifier == AARCH64_OPND_QLF_imm_tag)
    /* scaled immediate in ld/st pair instructions.  */
    info->addr.offset.imm *= aarch64_get_qualifier_esize (info->qualifier);
  /* qualifier */
  if (inst->opcode->iclass == ldst_unscaled
      || inst->opcode->iclass == ldstnapair_offs
      || inst->opcode->iclass == ldstpair_off
      || inst->opcode->iclass == ldst_unpriv)
    info->addr.writeback = 0;
  else
    {
      /* pre/post- index */
      info->addr.writeback = 1;
      if (extract_field (self->fields[1], code, 0) == 1)
	info->addr.preind = 1;
      else
	info->addr.postind = 1;
    }

  return true;
}

/* Decode the address operand for e.g. LDRSW <Xt>, [<Xn|SP>{, #<simm>}].  */
bool
aarch64_ext_addr_uimm12 (const aarch64_operand *self, aarch64_opnd_info *info,
			 aarch64_insn code,
			 const aarch64_inst *inst ATTRIBUTE_UNUSED,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int shift;
  info->qualifier = get_expected_qualifier (inst, info->idx);
  shift = get_logsz (aarch64_get_qualifier_esize (info->qualifier));
  /* Rn */
  info->addr.base_regno = extract_field (self->fields[0], code, 0);
  /* uimm12 */
  info->addr.offset.imm = extract_field (self->fields[1], code, 0) << shift;
  return true;
}

/* Decode the address operand for e.g. LDRAA <Xt>, [<Xn|SP>{, #<simm>}].  */
bool
aarch64_ext_addr_simm10 (const aarch64_operand *self, aarch64_opnd_info *info,
			 aarch64_insn code,
			 const aarch64_inst *inst ATTRIBUTE_UNUSED,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn imm;

  info->qualifier = get_expected_qualifier (inst, info->idx);
  /* Rn */
  info->addr.base_regno = extract_field (self->fields[0], code, 0);
  /* simm10 */
  imm = extract_fields (code, 0, 2, self->fields[1], self->fields[2]);
  info->addr.offset.imm = sign_extend (imm, 9) << 3;
  if (extract_field (self->fields[3], code, 0) == 1) {
    info->addr.writeback = 1;
    info->addr.preind = 1;
  }
  return true;
}

/* Decode the address operand for e.g.
     LD1 {<Vt>.<T>, <Vt2>.<T>, <Vt3>.<T>}, [<Xn|SP>], <Xm|#<amount>>.  */
bool
aarch64_ext_simd_addr_post (const aarch64_operand *self ATTRIBUTE_UNUSED,
			    aarch64_opnd_info *info,
			    aarch64_insn code, const aarch64_inst *inst,
			    aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* The opcode dependent area stores the number of elements in
     each structure to be loaded/stored.  */
  int is_ld1r = get_opcode_dependent_value (inst->opcode) == 1;

  /* Rn */
  info->addr.base_regno = extract_field (FLD_Rn, code, 0);
  /* Rm | #<amount>  */
  info->addr.offset.regno = extract_field (FLD_Rm, code, 0);
  if (info->addr.offset.regno == 31)
    {
      if (inst->opcode->operands[0] == AARCH64_OPND_LVt_AL)
	/* Special handling of loading single structure to all lane.  */
	info->addr.offset.imm = (is_ld1r ? 1
				 : inst->operands[0].reglist.num_regs)
	  * aarch64_get_qualifier_esize (inst->operands[0].qualifier);
      else
	info->addr.offset.imm = inst->operands[0].reglist.num_regs
	  * aarch64_get_qualifier_esize (inst->operands[0].qualifier)
	  * aarch64_get_qualifier_nelem (inst->operands[0].qualifier);
    }
  else
    info->addr.offset.is_reg = 1;
  info->addr.writeback = 1;

  return true;
}

/* Decode the condition operand for e.g. CSEL <Xd>, <Xn>, <Xm>, <cond>.  */
bool
aarch64_ext_cond (const aarch64_operand *self ATTRIBUTE_UNUSED,
		  aarch64_opnd_info *info,
		  aarch64_insn code, const aarch64_inst *inst ATTRIBUTE_UNUSED,
		  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn value;
  /* cond */
  value = extract_field (FLD_cond, code, 0);
  info->cond = get_cond_from_value (value);
  return true;
}

/* Decode the system register operand for e.g. MRS <Xt>, <systemreg>.  */
bool
aarch64_ext_sysreg (const aarch64_operand *self ATTRIBUTE_UNUSED,
		    aarch64_opnd_info *info,
		    aarch64_insn code,
		    const aarch64_inst *inst ATTRIBUTE_UNUSED,
		    aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* op0:op1:CRn:CRm:op2 */
  info->sysreg.value = extract_fields (code, 0, 5, FLD_op0, FLD_op1, FLD_CRn,
				       FLD_CRm, FLD_op2);
  info->sysreg.flags = 0;

  /* If a system instruction, check which restrictions should be on the register
     value during decoding, these will be enforced then.  */
  if (inst->opcode->iclass == ic_system)
    {
      /* Check to see if it's read-only, else check if it's write only.
	 if it's both or unspecified don't care.  */
      if ((inst->opcode->flags & (F_SYS_READ | F_SYS_WRITE)) == F_SYS_READ)
	info->sysreg.flags = F_REG_READ;
      else if ((inst->opcode->flags & (F_SYS_READ | F_SYS_WRITE))
	       == F_SYS_WRITE)
	info->sysreg.flags = F_REG_WRITE;
    }

  return true;
}

/* Decode the PSTATE field operand for e.g. MSR <pstatefield>, #<imm>.  */
bool
aarch64_ext_pstatefield (const aarch64_operand *self ATTRIBUTE_UNUSED,
			 aarch64_opnd_info *info, aarch64_insn code,
			 const aarch64_inst *inst ATTRIBUTE_UNUSED,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int i;
  aarch64_insn fld_crm = extract_field (FLD_CRm, code, 0);
  /* op1:op2 */
  info->pstatefield = extract_fields (code, 0, 2, FLD_op1, FLD_op2);
  for (i = 0; aarch64_pstatefields[i].name != NULL; ++i)
    if (aarch64_pstatefields[i].value == (aarch64_insn)info->pstatefield)
      {
        /* PSTATEFIELD name can be encoded partially in CRm[3:1].  */
        uint32_t flags = aarch64_pstatefields[i].flags;
        if ((flags & F_REG_IN_CRM)
            && ((fld_crm & 0xe) != PSTATE_DECODE_CRM (flags)))
          continue;
        info->sysreg.flags = flags;
        return true;
      }
  /* Reserved value in <pstatefield>.  */
  return false;
}

/* Decode the system instruction op operand for e.g. AT <at_op>, <Xt>.  */
bool
aarch64_ext_sysins_op (const aarch64_operand *self ATTRIBUTE_UNUSED,
		       aarch64_opnd_info *info,
		       aarch64_insn code,
		       const aarch64_inst *inst ATTRIBUTE_UNUSED,
		       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int i;
  aarch64_insn value;
  const aarch64_sys_ins_reg *sysins_ops;
  /* op0:op1:CRn:CRm:op2 */
  value = extract_fields (code, 0, 5,
			  FLD_op0, FLD_op1, FLD_CRn,
			  FLD_CRm, FLD_op2);

  switch (info->type)
    {
    case AARCH64_OPND_SYSREG_AT: sysins_ops = aarch64_sys_regs_at; break;
    case AARCH64_OPND_SYSREG_DC: sysins_ops = aarch64_sys_regs_dc; break;
    case AARCH64_OPND_SYSREG_IC: sysins_ops = aarch64_sys_regs_ic; break;
    case AARCH64_OPND_SYSREG_TLBI: sysins_ops = aarch64_sys_regs_tlbi; break;
    case AARCH64_OPND_SYSREG_SR:
	sysins_ops = aarch64_sys_regs_sr;
	 /* Let's remove op2 for rctx.  Refer to comments in the definition of
	    aarch64_sys_regs_sr[].  */
	value = value & ~(0x7);
	break;
    default: return false;
    }

  for (i = 0; sysins_ops[i].name != NULL; ++i)
    if (sysins_ops[i].value == value)
      {
	info->sysins_op = sysins_ops + i;
	DEBUG_TRACE ("%s found value: %x, has_xt: %d, i: %d.",
		     info->sysins_op->name,
		     (unsigned)info->sysins_op->value,
		     aarch64_sys_ins_reg_has_xt (info->sysins_op), i);
	return true;
      }

  return false;
}

/* Decode the memory barrier option operand for e.g. DMB <option>|#<imm>.  */

bool
aarch64_ext_barrier (const aarch64_operand *self ATTRIBUTE_UNUSED,
		     aarch64_opnd_info *info,
		     aarch64_insn code,
		     const aarch64_inst *inst ATTRIBUTE_UNUSED,
		     aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* CRm */
  info->barrier = aarch64_barrier_options + extract_field (FLD_CRm, code, 0);
  return true;
}

/* Decode the memory barrier option operand for DSB <option>nXS|#<imm>.  */

bool
aarch64_ext_barrier_dsb_nxs (const aarch64_operand *self ATTRIBUTE_UNUSED,
		     aarch64_opnd_info *info,
		     aarch64_insn code,
		     const aarch64_inst *inst ATTRIBUTE_UNUSED,
		     aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* For the DSB nXS barrier variant immediate is encoded in 2-bit field.  */
  aarch64_insn field = extract_field (FLD_CRm_dsb_nxs, code, 0);
  info->barrier = aarch64_barrier_dsb_nxs_options + field;
  return true;
}

/* Decode the prefetch operation option operand for e.g.
     PRFM <prfop>, [<Xn|SP>{, #<pimm>}].  */

bool
aarch64_ext_prfop (const aarch64_operand *self ATTRIBUTE_UNUSED,
		   aarch64_opnd_info *info,
		   aarch64_insn code, const aarch64_inst *inst ATTRIBUTE_UNUSED,
		   aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* prfop in Rt */
  info->prfop = aarch64_prfops + extract_field (FLD_Rt, code, 0);
  return true;
}

/* Decode the hint number for an alias taking an operand.  Set info->hint_option
   to the matching name/value pair in aarch64_hint_options.  */

bool
aarch64_ext_hint (const aarch64_operand *self ATTRIBUTE_UNUSED,
		  aarch64_opnd_info *info,
		  aarch64_insn code,
		  const aarch64_inst *inst ATTRIBUTE_UNUSED,
		  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  /* CRm:op2.  */
  unsigned hint_number;
  int i;

  hint_number = extract_fields (code, 0, 2, FLD_CRm, FLD_op2);

  for (i = 0; aarch64_hint_options[i].name != NULL; i++)
    {
      if (hint_number == HINT_VAL (aarch64_hint_options[i].value))
	{
	  info->hint_option = &(aarch64_hint_options[i]);
	  return true;
	}
    }

  return false;
}

/* Decode the extended register operand for e.g.
     STR <Qt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
bool
aarch64_ext_reg_extended (const aarch64_operand *self ATTRIBUTE_UNUSED,
			  aarch64_opnd_info *info,
			  aarch64_insn code,
			  const aarch64_inst *inst ATTRIBUTE_UNUSED,
			  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn value;

  /* Rm */
  info->reg.regno = extract_field (FLD_Rm, code, 0);
  /* option */
  value = extract_field (FLD_option, code, 0);
  info->shifter.kind =
    aarch64_get_operand_modifier_from_value (value, true /* extend_p */);
  /* imm3 */
  info->shifter.amount = extract_field (FLD_imm3_10, code,  0);

  /* This makes the constraint checking happy.  */
  info->shifter.operator_present = 1;

  /* Assume inst->operands[0].qualifier has been resolved.  */
  assert (inst->operands[0].qualifier != AARCH64_OPND_QLF_NIL);
  info->qualifier = AARCH64_OPND_QLF_W;
  if (inst->operands[0].qualifier == AARCH64_OPND_QLF_X
      && (info->shifter.kind == AARCH64_MOD_UXTX
	  || info->shifter.kind == AARCH64_MOD_SXTX))
    info->qualifier = AARCH64_OPND_QLF_X;

  return true;
}

/* Decode the shifted register operand for e.g.
     SUBS <Xd>, <Xn>, <Xm> {, <shift> #<amount>}.  */
bool
aarch64_ext_reg_shifted (const aarch64_operand *self ATTRIBUTE_UNUSED,
			 aarch64_opnd_info *info,
			 aarch64_insn code,
			 const aarch64_inst *inst ATTRIBUTE_UNUSED,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn value;

  /* Rm */
  info->reg.regno = extract_field (FLD_Rm, code, 0);
  /* shift */
  value = extract_field (FLD_shift, code, 0);
  info->shifter.kind =
    aarch64_get_operand_modifier_from_value (value, false /* extend_p */);
  if (info->shifter.kind == AARCH64_MOD_ROR
      && inst->opcode->iclass != log_shift)
    /* ROR is not available for the shifted register operand in arithmetic
       instructions.  */
    return false;
  /* imm6 */
  info->shifter.amount = extract_field (FLD_imm6_10, code,  0);

  /* This makes the constraint checking happy.  */
  info->shifter.operator_present = 1;

  return true;
}

/* Decode an SVE address [<base>, #<offset>*<factor>, MUL VL],
   where <offset> is given by the OFFSET parameter and where <factor> is
   1 plus SELF's operand-dependent value.  fields[0] specifies the field
   that holds <base>.  */
static bool
aarch64_ext_sve_addr_reg_mul_vl (const aarch64_operand *self,
				 aarch64_opnd_info *info, aarch64_insn code,
				 int64_t offset)
{
  info->addr.base_regno = extract_field (self->fields[0], code, 0);
  info->addr.offset.imm = offset * (1 + get_operand_specific_data (self));
  info->addr.offset.is_reg = false;
  info->addr.writeback = false;
  info->addr.preind = true;
  if (offset != 0)
    info->shifter.kind = AARCH64_MOD_MUL_VL;
  info->shifter.amount = 1;
  info->shifter.operator_present = (info->addr.offset.imm != 0);
  info->shifter.amount_present = false;
  return true;
}

/* Decode an SVE address [<base>, #<simm4>*<factor>, MUL VL],
   where <simm4> is a 4-bit signed value and where <factor> is 1 plus
   SELF's operand-dependent value.  fields[0] specifies the field that
   holds <base>.  <simm4> is encoded in the SVE_imm4 field.  */
bool
aarch64_ext_sve_addr_ri_s4xvl (const aarch64_operand *self,
			       aarch64_opnd_info *info, aarch64_insn code,
			       const aarch64_inst *inst ATTRIBUTE_UNUSED,
			       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int offset;

  offset = extract_field (FLD_SVE_imm4, code, 0);
  offset = ((offset + 8) & 15) - 8;
  return aarch64_ext_sve_addr_reg_mul_vl (self, info, code, offset);
}

/* Decode an SVE address [<base>, #<simm6>*<factor>, MUL VL],
   where <simm6> is a 6-bit signed value and where <factor> is 1 plus
   SELF's operand-dependent value.  fields[0] specifies the field that
   holds <base>.  <simm6> is encoded in the SVE_imm6 field.  */
bool
aarch64_ext_sve_addr_ri_s6xvl (const aarch64_operand *self,
			       aarch64_opnd_info *info, aarch64_insn code,
			       const aarch64_inst *inst ATTRIBUTE_UNUSED,
			       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int offset;

  offset = extract_field (FLD_SVE_imm6, code, 0);
  offset = (((offset + 32) & 63) - 32);
  return aarch64_ext_sve_addr_reg_mul_vl (self, info, code, offset);
}

/* Decode an SVE address [<base>, #<simm9>*<factor>, MUL VL],
   where <simm9> is a 9-bit signed value and where <factor> is 1 plus
   SELF's operand-dependent value.  fields[0] specifies the field that
   holds <base>.  <simm9> is encoded in the concatenation of the SVE_imm6
   and imm3 fields, with imm3 being the less-significant part.  */
bool
aarch64_ext_sve_addr_ri_s9xvl (const aarch64_operand *self,
			       aarch64_opnd_info *info,
			       aarch64_insn code,
			       const aarch64_inst *inst ATTRIBUTE_UNUSED,
			       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int offset;

  offset = extract_fields (code, 0, 2, FLD_SVE_imm6, FLD_imm3_10);
  offset = (((offset + 256) & 511) - 256);
  return aarch64_ext_sve_addr_reg_mul_vl (self, info, code, offset);
}

/* Decode an SVE address [<base>, #<offset> << <shift>], where <offset>
   is given by the OFFSET parameter and where <shift> is SELF's operand-
   dependent value.  fields[0] specifies the base register field <base>.  */
static bool
aarch64_ext_sve_addr_reg_imm (const aarch64_operand *self,
			      aarch64_opnd_info *info, aarch64_insn code,
			      int64_t offset)
{
  info->addr.base_regno = extract_field (self->fields[0], code, 0);
  info->addr.offset.imm = offset * (1 << get_operand_specific_data (self));
  info->addr.offset.is_reg = false;
  info->addr.writeback = false;
  info->addr.preind = true;
  info->shifter.operator_present = false;
  info->shifter.amount_present = false;
  return true;
}

/* Decode an SVE address [X<n>, #<SVE_imm4> << <shift>], where <SVE_imm4>
   is a 4-bit signed number and where <shift> is SELF's operand-dependent
   value.  fields[0] specifies the base register field.  */
bool
aarch64_ext_sve_addr_ri_s4 (const aarch64_operand *self,
			    aarch64_opnd_info *info, aarch64_insn code,
			    const aarch64_inst *inst ATTRIBUTE_UNUSED,
			    aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int offset = sign_extend (extract_field (FLD_SVE_imm4, code, 0), 3);
  return aarch64_ext_sve_addr_reg_imm (self, info, code, offset);
}

/* Decode an SVE address [X<n>, #<SVE_imm6> << <shift>], where <SVE_imm6>
   is a 6-bit unsigned number and where <shift> is SELF's operand-dependent
   value.  fields[0] specifies the base register field.  */
bool
aarch64_ext_sve_addr_ri_u6 (const aarch64_operand *self,
			    aarch64_opnd_info *info, aarch64_insn code,
			    const aarch64_inst *inst ATTRIBUTE_UNUSED,
			    aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int offset = extract_field (FLD_SVE_imm6, code, 0);
  return aarch64_ext_sve_addr_reg_imm (self, info, code, offset);
}

/* Decode an SVE address [X<n>, X<m>{, LSL #<shift>}], where <shift>
   is SELF's operand-dependent value.  fields[0] specifies the base
   register field and fields[1] specifies the offset register field.  */
bool
aarch64_ext_sve_addr_rr_lsl (const aarch64_operand *self,
			     aarch64_opnd_info *info, aarch64_insn code,
			     const aarch64_inst *inst ATTRIBUTE_UNUSED,
			     aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int index_regno;

  index_regno = extract_field (self->fields[1], code, 0);
  if (index_regno == 31 && (self->flags & OPD_F_NO_ZR) != 0)
    return false;

  info->addr.base_regno = extract_field (self->fields[0], code, 0);
  info->addr.offset.regno = index_regno;
  info->addr.offset.is_reg = true;
  info->addr.writeback = false;
  info->addr.preind = true;
  info->shifter.kind = AARCH64_MOD_LSL;
  info->shifter.amount = get_operand_specific_data (self);
  info->shifter.operator_present = (info->shifter.amount != 0);
  info->shifter.amount_present = (info->shifter.amount != 0);
  return true;
}

/* Decode an SVE address [X<n>, Z<m>.<T>, (S|U)XTW {#<shift>}], where
   <shift> is SELF's operand-dependent value.  fields[0] specifies the
   base register field, fields[1] specifies the offset register field and
   fields[2] is a single-bit field that selects SXTW over UXTW.  */
bool
aarch64_ext_sve_addr_rz_xtw (const aarch64_operand *self,
			     aarch64_opnd_info *info, aarch64_insn code,
			     const aarch64_inst *inst ATTRIBUTE_UNUSED,
			     aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->addr.base_regno = extract_field (self->fields[0], code, 0);
  info->addr.offset.regno = extract_field (self->fields[1], code, 0);
  info->addr.offset.is_reg = true;
  info->addr.writeback = false;
  info->addr.preind = true;
  if (extract_field (self->fields[2], code, 0))
    info->shifter.kind = AARCH64_MOD_SXTW;
  else
    info->shifter.kind = AARCH64_MOD_UXTW;
  info->shifter.amount = get_operand_specific_data (self);
  info->shifter.operator_present = true;
  info->shifter.amount_present = (info->shifter.amount != 0);
  return true;
}

/* Decode an SVE address [Z<n>.<T>, #<imm5> << <shift>], where <imm5> is a
   5-bit unsigned number and where <shift> is SELF's operand-dependent value.
   fields[0] specifies the base register field.  */
bool
aarch64_ext_sve_addr_zi_u5 (const aarch64_operand *self,
			    aarch64_opnd_info *info, aarch64_insn code,
			    const aarch64_inst *inst ATTRIBUTE_UNUSED,
			    aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int offset = extract_field (FLD_imm5, code, 0);
  return aarch64_ext_sve_addr_reg_imm (self, info, code, offset);
}

/* Decode an SVE address [Z<n>.<T>, Z<m>.<T>{, <modifier> {#<msz>}}],
   where <modifier> is given by KIND and where <msz> is a 2-bit unsigned
   number.  fields[0] specifies the base register field and fields[1]
   specifies the offset register field.  */
static bool
aarch64_ext_sve_addr_zz (const aarch64_operand *self, aarch64_opnd_info *info,
			 aarch64_insn code, enum aarch64_modifier_kind kind)
{
  info->addr.base_regno = extract_field (self->fields[0], code, 0);
  info->addr.offset.regno = extract_field (self->fields[1], code, 0);
  info->addr.offset.is_reg = true;
  info->addr.writeback = false;
  info->addr.preind = true;
  info->shifter.kind = kind;
  info->shifter.amount = extract_field (FLD_SVE_msz, code, 0);
  info->shifter.operator_present = (kind != AARCH64_MOD_LSL
				    || info->shifter.amount != 0);
  info->shifter.amount_present = (info->shifter.amount != 0);
  return true;
}

/* Decode an SVE address [Z<n>.<T>, Z<m>.<T>{, LSL #<msz>}], where
   <msz> is a 2-bit unsigned number.  fields[0] specifies the base register
   field and fields[1] specifies the offset register field.  */
bool
aarch64_ext_sve_addr_zz_lsl (const aarch64_operand *self,
			     aarch64_opnd_info *info, aarch64_insn code,
			     const aarch64_inst *inst ATTRIBUTE_UNUSED,
			     aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  return aarch64_ext_sve_addr_zz (self, info, code, AARCH64_MOD_LSL);
}

/* Decode an SVE address [Z<n>.<T>, Z<m>.<T>, SXTW {#<msz>}], where
   <msz> is a 2-bit unsigned number.  fields[0] specifies the base register
   field and fields[1] specifies the offset register field.  */
bool
aarch64_ext_sve_addr_zz_sxtw (const aarch64_operand *self,
			      aarch64_opnd_info *info, aarch64_insn code,
			      const aarch64_inst *inst ATTRIBUTE_UNUSED,
			      aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  return aarch64_ext_sve_addr_zz (self, info, code, AARCH64_MOD_SXTW);
}

/* Decode an SVE address [Z<n>.<T>, Z<m>.<T>, UXTW {#<msz>}], where
   <msz> is a 2-bit unsigned number.  fields[0] specifies the base register
   field and fields[1] specifies the offset register field.  */
bool
aarch64_ext_sve_addr_zz_uxtw (const aarch64_operand *self,
			      aarch64_opnd_info *info, aarch64_insn code,
			      const aarch64_inst *inst ATTRIBUTE_UNUSED,
			      aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  return aarch64_ext_sve_addr_zz (self, info, code, AARCH64_MOD_UXTW);
}

/* Finish decoding an SVE arithmetic immediate, given that INFO already
   has the raw field value and that the low 8 bits decode to VALUE.  */
static bool
decode_sve_aimm (aarch64_opnd_info *info, int64_t value)
{
  info->shifter.kind = AARCH64_MOD_LSL;
  info->shifter.amount = 0;
  if (info->imm.value & 0x100)
    {
      if (value == 0)
	/* Decode 0x100 as #0, LSL #8.  */
	info->shifter.amount = 8;
      else
	value *= 256;
    }
  info->shifter.operator_present = (info->shifter.amount != 0);
  info->shifter.amount_present = (info->shifter.amount != 0);
  info->imm.value = value;
  return true;
}

/* Decode an SVE ADD/SUB immediate.  */
bool
aarch64_ext_sve_aimm (const aarch64_operand *self,
		      aarch64_opnd_info *info, const aarch64_insn code,
		      const aarch64_inst *inst,
		      aarch64_operand_error *errors)
{
  return (aarch64_ext_imm (self, info, code, inst, errors)
	  && decode_sve_aimm (info, (uint8_t) info->imm.value));
}

bool
aarch64_ext_sve_aligned_reglist (const aarch64_operand *self,
				 aarch64_opnd_info *info, aarch64_insn code,
				 const aarch64_inst *inst ATTRIBUTE_UNUSED,
				 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  unsigned int num_regs = get_operand_specific_data (self);
  unsigned int val = extract_field (self->fields[0], code, 0);
  info->reglist.first_regno = val * num_regs;
  info->reglist.num_regs = num_regs;
  info->reglist.stride = 1;
  return true;
}

/* Decode an SVE CPY/DUP immediate.  */
bool
aarch64_ext_sve_asimm (const aarch64_operand *self,
		       aarch64_opnd_info *info, const aarch64_insn code,
		       const aarch64_inst *inst,
		       aarch64_operand_error *errors)
{
  return (aarch64_ext_imm (self, info, code, inst, errors)
	  && decode_sve_aimm (info, (int8_t) info->imm.value));
}

/* Decode a single-bit immediate that selects between #0.5 and #1.0.
   The fields array specifies which field to use.  */
bool
aarch64_ext_sve_float_half_one (const aarch64_operand *self,
				aarch64_opnd_info *info, aarch64_insn code,
				const aarch64_inst *inst ATTRIBUTE_UNUSED,
				aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  if (extract_field (self->fields[0], code, 0))
    info->imm.value = 0x3f800000;
  else
    info->imm.value = 0x3f000000;
  info->imm.is_fp = true;
  return true;
}

/* Decode a single-bit immediate that selects between #0.5 and #2.0.
   The fields array specifies which field to use.  */
bool
aarch64_ext_sve_float_half_two (const aarch64_operand *self,
				aarch64_opnd_info *info, aarch64_insn code,
				const aarch64_inst *inst ATTRIBUTE_UNUSED,
				aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  if (extract_field (self->fields[0], code, 0))
    info->imm.value = 0x40000000;
  else
    info->imm.value = 0x3f000000;
  info->imm.is_fp = true;
  return true;
}

/* Decode a single-bit immediate that selects between #0.0 and #1.0.
   The fields array specifies which field to use.  */
bool
aarch64_ext_sve_float_zero_one (const aarch64_operand *self,
				aarch64_opnd_info *info, aarch64_insn code,
				const aarch64_inst *inst ATTRIBUTE_UNUSED,
				aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  if (extract_field (self->fields[0], code, 0))
    info->imm.value = 0x3f800000;
  else
    info->imm.value = 0x0;
  info->imm.is_fp = true;
  return true;
}

/* Decode ZA tile vector, vector indicator, vector selector, qualifier and
   immediate on numerous SME instruction fields such as MOVA.  */
bool
aarch64_ext_sme_za_hv_tiles (const aarch64_operand *self,
                             aarch64_opnd_info *info, aarch64_insn code,
                             const aarch64_inst *inst ATTRIBUTE_UNUSED,
                             aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int fld_size = extract_field (self->fields[0], code, 0);
  int fld_q = extract_field (self->fields[1], code, 0);
  int fld_v = extract_field (self->fields[2], code, 0);
  int fld_rv = extract_field (self->fields[3], code, 0);
  int fld_zan_imm = extract_field (self->fields[4], code, 0);

  /* Deduce qualifier encoded in size and Q fields.  */
  if (fld_size == 0)
    {
      info->indexed_za.regno = 0;
      info->indexed_za.index.imm = fld_zan_imm;
    }
  else if (fld_size == 1)
    {
      info->indexed_za.regno = fld_zan_imm >> 3;
      info->indexed_za.index.imm = fld_zan_imm & 0x07;
    }
  else if (fld_size == 2)
    {
      info->indexed_za.regno = fld_zan_imm >> 2;
      info->indexed_za.index.imm = fld_zan_imm & 0x03;
    }
  else if (fld_size == 3 && fld_q == 0)
    {
      info->indexed_za.regno = fld_zan_imm >> 1;
      info->indexed_za.index.imm = fld_zan_imm & 0x01;
    }
  else if (fld_size == 3 && fld_q == 1)
    {
      info->indexed_za.regno = fld_zan_imm;
      info->indexed_za.index.imm = 0;
    }
  else
    return false;

  info->indexed_za.index.regno = fld_rv + 12;
  info->indexed_za.v = fld_v;

  return true;
}

bool
aarch64_ext_sme_za_hv_tiles_range (const aarch64_operand *self,
				   aarch64_opnd_info *info, aarch64_insn code,
				   const aarch64_inst *inst ATTRIBUTE_UNUSED,
				   aarch64_operand_error *errors
				     ATTRIBUTE_UNUSED)
{
  int ebytes = aarch64_get_qualifier_esize (info->qualifier);
  int range_size = get_opcode_dependent_value (inst->opcode);
  int fld_v = extract_field (self->fields[0], code, 0);
  int fld_rv = extract_field (self->fields[1], code, 0);
  int fld_zan_imm = extract_field (self->fields[2], code, 0);
  int max_value = 16 / range_size / ebytes;

  if (max_value == 0)
    max_value = 1;

  int regno = fld_zan_imm / max_value;
  if (regno >= ebytes)
    return false;

  info->indexed_za.regno = regno;
  info->indexed_za.index.imm = (fld_zan_imm % max_value) * range_size;
  info->indexed_za.index.countm1 = range_size - 1;
  info->indexed_za.index.regno = fld_rv + 12;
  info->indexed_za.v = fld_v;

  return true;
}

/* Decode in SME instruction ZERO list of up to eight 64-bit element tile names
   separated by commas, encoded in the "imm8" field.

   For programmer convenience an assembler must also accept the names of
   32-bit, 16-bit and 8-bit element tiles which are converted into the
   corresponding set of 64-bit element tiles.
*/
bool
aarch64_ext_sme_za_list (const aarch64_operand *self,
                         aarch64_opnd_info *info, aarch64_insn code,
                         const aarch64_inst *inst ATTRIBUTE_UNUSED,
                         aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int mask = extract_field (self->fields[0], code, 0);
  info->imm.value = mask;
  return true;
}

/* Decode ZA array vector select register (Rv field), optional vector and
   memory offset (imm4_11 field).
*/
bool
aarch64_ext_sme_za_array (const aarch64_operand *self,
                          aarch64_opnd_info *info, aarch64_insn code,
                          const aarch64_inst *inst,
                          aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int regno = extract_field (self->fields[0], code, 0);
  if (info->type == AARCH64_OPND_SME_ZA_array_off4)
    regno += 12;
  else
    regno += 8;
  int imm = extract_field (self->fields[1], code, 0);
  int num_offsets = get_operand_specific_data (self);
  if (num_offsets == 0)
    num_offsets = 1;
  info->indexed_za.index.regno = regno;
  info->indexed_za.index.imm = imm * num_offsets;
  info->indexed_za.index.countm1 = num_offsets - 1;
  info->indexed_za.group_size = get_opcode_dependent_value (inst->opcode);
  return true;
}

bool
aarch64_ext_sme_addr_ri_u4xvl (const aarch64_operand *self,
                               aarch64_opnd_info *info, aarch64_insn code,
                               const aarch64_inst *inst ATTRIBUTE_UNUSED,
                               aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int regno = extract_field (self->fields[0], code, 0);
  int imm = extract_field (self->fields[1], code, 0);
  info->addr.base_regno = regno;
  info->addr.offset.imm = imm;
  /* MUL VL operator is always present for this operand.  */
  info->shifter.kind = AARCH64_MOD_MUL_VL;
  info->shifter.operator_present = (imm != 0);
  return true;
}

/* Decode {SM|ZA} filed for SMSTART and SMSTOP instructions.  */
bool
aarch64_ext_sme_sm_za (const aarch64_operand *self,
                       aarch64_opnd_info *info, aarch64_insn code,
                       const aarch64_inst *inst ATTRIBUTE_UNUSED,
                       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->pstatefield = 0x1b;
  aarch64_insn fld_crm = extract_field (self->fields[0], code, 0);
  fld_crm >>= 1;    /* CRm[3:1].  */

  if (fld_crm == 0x1)
    info->reg.regno = 's';
  else if (fld_crm == 0x2)
    info->reg.regno = 'z';
  else
    return false;

  return true;
}

bool
aarch64_ext_sme_pred_reg_with_index (const aarch64_operand *self,
				     aarch64_opnd_info *info, aarch64_insn code,
				     const aarch64_inst *inst ATTRIBUTE_UNUSED,
				     aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  aarch64_insn fld_rm = extract_field (self->fields[0], code, 0);
  aarch64_insn fld_pn = extract_field (self->fields[1], code, 0);
  aarch64_insn fld_i1 = extract_field (self->fields[2], code, 0);
  aarch64_insn fld_tszh = extract_field (self->fields[3], code, 0);
  aarch64_insn fld_tszl = extract_field (self->fields[4], code, 0);
  int imm;

  info->indexed_za.regno = fld_pn;
  info->indexed_za.index.regno = fld_rm + 12;

  if (fld_tszl & 0x1)
    imm = (fld_i1 << 3) | (fld_tszh << 2) | (fld_tszl >> 1);
  else if (fld_tszl & 0x2)
    imm = (fld_i1 << 2) | (fld_tszh << 1) | (fld_tszl >> 2);
  else if (fld_tszl & 0x4)
    imm = (fld_i1 << 1) | fld_tszh;
  else if (fld_tszh)
    imm = fld_i1;
  else
    return false;

  info->indexed_za.index.imm = imm;
  return true;
}

/* Decode Zn[MM], where MM has a 7-bit triangular encoding.  The fields
   array specifies which field to use for Zn.  MM is encoded in the
   concatenation of imm5 and SVE_tszh, with imm5 being the less
   significant part.  */
bool
aarch64_ext_sve_index (const aarch64_operand *self,
		       aarch64_opnd_info *info, aarch64_insn code,
		       const aarch64_inst *inst ATTRIBUTE_UNUSED,
		       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int val;

  info->reglane.regno = extract_field (self->fields[0], code, 0);
  val = extract_fields (code, 0, 2, FLD_SVE_tszh, FLD_imm5);
  if ((val & 31) == 0)
    return 0;
  while ((val & 1) == 0)
    val /= 2;
  info->reglane.index = val / 2;
  return true;
}

/* Decode a logical immediate for the MOV alias of SVE DUPM.  */
bool
aarch64_ext_sve_limm_mov (const aarch64_operand *self,
			  aarch64_opnd_info *info, const aarch64_insn code,
			  const aarch64_inst *inst,
			  aarch64_operand_error *errors)
{
  int esize = aarch64_get_qualifier_esize (inst->operands[0].qualifier);
  return (aarch64_ext_limm (self, info, code, inst, errors)
	  && aarch64_sve_dupm_mov_immediate_p (info->imm.value, esize));
}

/* Decode Zn[MM], where Zn occupies the least-significant part of the field
   and where MM occupies the most-significant part.  The operand-dependent
   value specifies the number of bits in Zn.  */
bool
aarch64_ext_sve_quad_index (const aarch64_operand *self,
			    aarch64_opnd_info *info, aarch64_insn code,
			    const aarch64_inst *inst ATTRIBUTE_UNUSED,
			    aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  unsigned int reg_bits = get_operand_specific_data (self);
  unsigned int val = extract_all_fields (self, code);
  info->reglane.regno = val & ((1 << reg_bits) - 1);
  info->reglane.index = val >> reg_bits;
  return true;
}

/* Decode {Zn.<T> - Zm.<T>}.  The fields array specifies which field
   to use for Zn.  The opcode-dependent value specifies the number
   of registers in the list.  */
bool
aarch64_ext_sve_reglist (const aarch64_operand *self,
			 aarch64_opnd_info *info, aarch64_insn code,
			 const aarch64_inst *inst ATTRIBUTE_UNUSED,
			 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->reglist.first_regno = extract_field (self->fields[0], code, 0);
  info->reglist.num_regs = get_opcode_dependent_value (inst->opcode);
  info->reglist.stride = 1;
  return true;
}

/* Decode a strided register list.  The first field holds the top bit
   (0 or 16) and the second field holds the lower bits.  The stride is
   16 divided by the list length.  */
bool
aarch64_ext_sve_strided_reglist (const aarch64_operand *self,
				 aarch64_opnd_info *info, aarch64_insn code,
				 const aarch64_inst *inst ATTRIBUTE_UNUSED,
				 aarch64_operand_error *errors
				   ATTRIBUTE_UNUSED)
{
  unsigned int upper = extract_field (self->fields[0], code, 0);
  unsigned int lower = extract_field (self->fields[1], code, 0);
  info->reglist.first_regno = upper * 16 + lower;
  info->reglist.num_regs = get_operand_specific_data (self);
  info->reglist.stride = 16 / info->reglist.num_regs;
  return true;
}

/* Decode <pattern>{, MUL #<amount>}.  The fields array specifies which
   fields to use for <pattern>.  <amount> - 1 is encoded in the SVE_imm4
   field.  */
bool
aarch64_ext_sve_scale (const aarch64_operand *self,
		       aarch64_opnd_info *info, aarch64_insn code,
		       const aarch64_inst *inst, aarch64_operand_error *errors)
{
  int val;

  if (!aarch64_ext_imm (self, info, code, inst, errors))
    return false;
  val = extract_field (FLD_SVE_imm4, code, 0);
  info->shifter.kind = AARCH64_MOD_MUL;
  info->shifter.amount = val + 1;
  info->shifter.operator_present = (val != 0);
  info->shifter.amount_present = (val != 0);
  return true;
}

/* Return the top set bit in VALUE, which is expected to be relatively
   small.  */
static uint64_t
get_top_bit (uint64_t value)
{
  while ((value & -value) != value)
    value -= value & -value;
  return value;
}

/* Decode an SVE shift-left immediate.  */
bool
aarch64_ext_sve_shlimm (const aarch64_operand *self,
			aarch64_opnd_info *info, const aarch64_insn code,
			const aarch64_inst *inst, aarch64_operand_error *errors)
{
  if (!aarch64_ext_imm (self, info, code, inst, errors)
      || info->imm.value == 0)
    return false;

  info->imm.value -= get_top_bit (info->imm.value);
  return true;
}

/* Decode an SVE shift-right immediate.  */
bool
aarch64_ext_sve_shrimm (const aarch64_operand *self,
			aarch64_opnd_info *info, const aarch64_insn code,
			const aarch64_inst *inst, aarch64_operand_error *errors)
{
  if (!aarch64_ext_imm (self, info, code, inst, errors)
      || info->imm.value == 0)
    return false;

  info->imm.value = get_top_bit (info->imm.value) * 2 - info->imm.value;
  return true;
}

/* Decode X0-X30.  Register 31 is unallocated.  */
bool
aarch64_ext_x0_to_x30 (const aarch64_operand *self, aarch64_opnd_info *info,
		       const aarch64_insn code,
		       const aarch64_inst *inst ATTRIBUTE_UNUSED,
		       aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  info->reg.regno = extract_field (self->fields[0], code, 0);
  return info->reg.regno <= 30;
}

/* Decode an indexed register, with the first field being the register
   number and the remaining fields being the index.  */
bool
aarch64_ext_simple_index (const aarch64_operand *self, aarch64_opnd_info *info,
			  const aarch64_insn code,
			  const aarch64_inst *inst ATTRIBUTE_UNUSED,
			  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  int bias = get_operand_specific_data (self);
  info->reglane.regno = extract_field (self->fields[0], code, 0) + bias;
  info->reglane.index = extract_all_fields_after (self, 1, code);
  return true;
}

/* Decode a plain shift-right immediate, when there is only a single
   element size.  */
bool
aarch64_ext_plain_shrimm (const aarch64_operand *self, aarch64_opnd_info *info,
			  const aarch64_insn code,
			  const aarch64_inst *inst ATTRIBUTE_UNUSED,
			  aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  unsigned int base = 1 << get_operand_field_width (self, 0);
  info->imm.value = base - extract_field (self->fields[0], code, 0);
  return true;
}

/* Bitfields that are commonly used to encode certain operands' information
   may be partially used as part of the base opcode in some instructions.
   For example, the bit 1 of the field 'size' in
     FCVTXN <Vb><d>, <Va><n>
   is actually part of the base opcode, while only size<0> is available
   for encoding the register type.  Another example is the AdvSIMD
   instruction ORR (register), in which the field 'size' is also used for
   the base opcode, leaving only the field 'Q' available to encode the
   vector register arrangement specifier '8B' or '16B'.

   This function tries to deduce the qualifier from the value of partially
   constrained field(s).  Given the VALUE of such a field or fields, the
   qualifiers CANDIDATES and the MASK (indicating which bits are valid for
   operand encoding), the function returns the matching qualifier or
   AARCH64_OPND_QLF_NIL if nothing matches.

   N.B. CANDIDATES is a group of possible qualifiers that are valid for
   one operand; it has a maximum of AARCH64_MAX_QLF_SEQ_NUM qualifiers and
   may end with AARCH64_OPND_QLF_NIL.  */

static enum aarch64_opnd_qualifier
get_qualifier_from_partial_encoding (aarch64_insn value,
				     const enum aarch64_opnd_qualifier* \
				     candidates,
				     aarch64_insn mask)
{
  int i;
  DEBUG_TRACE ("enter with value: %d, mask: %d", (int)value, (int)mask);
  for (i = 0; i < AARCH64_MAX_QLF_SEQ_NUM; ++i)
    {
      aarch64_insn standard_value;
      if (candidates[i] == AARCH64_OPND_QLF_NIL)
	break;
      standard_value = aarch64_get_qualifier_standard_value (candidates[i]);
      if ((standard_value & mask) == (value & mask))
	return candidates[i];
    }
  return AARCH64_OPND_QLF_NIL;
}

/* Given a list of qualifier sequences, return all possible valid qualifiers
   for operand IDX in QUALIFIERS.
   Assume QUALIFIERS is an array whose length is large enough.  */

static void
get_operand_possible_qualifiers (int idx,
				 const aarch64_opnd_qualifier_seq_t *list,
				 enum aarch64_opnd_qualifier *qualifiers)
{
  int i;
  for (i = 0; i < AARCH64_MAX_QLF_SEQ_NUM; ++i)
    if ((qualifiers[i] = list[i][idx]) == AARCH64_OPND_QLF_NIL)
      break;
}

/* Decode the size Q field for e.g. SHADD.
   We tag one operand with the qualifer according to the code;
   whether the qualifier is valid for this opcode or not, it is the
   duty of the semantic checking.  */

static int
decode_sizeq (aarch64_inst *inst)
{
  int idx;
  enum aarch64_opnd_qualifier qualifier;
  aarch64_insn code;
  aarch64_insn value, mask;
  enum aarch64_field_kind fld_sz;
  enum aarch64_opnd_qualifier candidates[AARCH64_MAX_QLF_SEQ_NUM];

  if (inst->opcode->iclass == asisdlse
     || inst->opcode->iclass == asisdlsep
     || inst->opcode->iclass == asisdlso
     || inst->opcode->iclass == asisdlsop)
    fld_sz = FLD_vldst_size;
  else
    fld_sz = FLD_size;

  code = inst->value;
  value = extract_fields (code, inst->opcode->mask, 2, fld_sz, FLD_Q);
  /* Obtain the info that which bits of fields Q and size are actually
     available for operand encoding.  Opcodes like FMAXNM and FMLA have
     size[1] unavailable.  */
  mask = extract_fields (~inst->opcode->mask, 0, 2, fld_sz, FLD_Q);

  /* The index of the operand we are going to tag a qualifier and the qualifer
     itself are reasoned from the value of the size and Q fields and the
     possible valid qualifier lists.  */
  idx = aarch64_select_operand_for_sizeq_field_coding (inst->opcode);
  DEBUG_TRACE ("key idx: %d", idx);

  /* For most related instruciton, size:Q are fully available for operand
     encoding.  */
  if (mask == 0x7)
    {
      inst->operands[idx].qualifier = get_vreg_qualifier_from_value (value);
      return 1;
    }

  get_operand_possible_qualifiers (idx, inst->opcode->qualifiers_list,
				   candidates);
#ifdef DEBUG_AARCH64
  if (debug_dump)
    {
      int i;
      for (i = 0; candidates[i] != AARCH64_OPND_QLF_NIL
	   && i < AARCH64_MAX_QLF_SEQ_NUM; ++i)
	DEBUG_TRACE ("qualifier %d: %s", i,
		     aarch64_get_qualifier_name(candidates[i]));
      DEBUG_TRACE ("%d, %d", (int)value, (int)mask);
    }
#endif /* DEBUG_AARCH64 */

  qualifier = get_qualifier_from_partial_encoding (value, candidates, mask);

  if (qualifier == AARCH64_OPND_QLF_NIL)
    return 0;

  inst->operands[idx].qualifier = qualifier;
  return 1;
}

/* Decode size[0]:Q, i.e. bit 22 and bit 30, for
     e.g. FCVTN<Q> <Vd>.<Tb>, <Vn>.<Ta>.  */

static int
decode_asimd_fcvt (aarch64_inst *inst)
{
  aarch64_field field = {0, 0};
  aarch64_insn value;
  enum aarch64_opnd_qualifier qualifier;

  gen_sub_field (FLD_size, 0, 1, &field);
  value = extract_field_2 (&field, inst->value, 0);
  qualifier = value == 0 ? AARCH64_OPND_QLF_V_4S
    : AARCH64_OPND_QLF_V_2D;
  switch (inst->opcode->op)
    {
    case OP_FCVTN:
    case OP_FCVTN2:
      /* FCVTN<Q> <Vd>.<Tb>, <Vn>.<Ta>.  */
      inst->operands[1].qualifier = qualifier;
      break;
    case OP_FCVTL:
    case OP_FCVTL2:
      /* FCVTL<Q> <Vd>.<Ta>, <Vn>.<Tb>.  */
      inst->operands[0].qualifier = qualifier;
      break;
    default:
      return 0;
    }

  return 1;
}

/* Decode size[0], i.e. bit 22, for
     e.g. FCVTXN <Vb><d>, <Va><n>.  */

static int
decode_asisd_fcvtxn (aarch64_inst *inst)
{
  aarch64_field field = {0, 0};
  gen_sub_field (FLD_size, 0, 1, &field);
  if (!extract_field_2 (&field, inst->value, 0))
    return 0;
  inst->operands[0].qualifier = AARCH64_OPND_QLF_S_S;
  return 1;
}

/* Decode the 'opc' field for e.g. FCVT <Dd>, <Sn>.  */
static int
decode_fcvt (aarch64_inst *inst)
{
  enum aarch64_opnd_qualifier qualifier;
  aarch64_insn value;
  const aarch64_field field = {15, 2};

  /* opc dstsize */
  value = extract_field_2 (&field, inst->value, 0);
  switch (value)
    {
    case 0: qualifier = AARCH64_OPND_QLF_S_S; break;
    case 1: qualifier = AARCH64_OPND_QLF_S_D; break;
    case 3: qualifier = AARCH64_OPND_QLF_S_H; break;
    default: return 0;
    }
  inst->operands[0].qualifier = qualifier;

  return 1;
}

/* Do miscellaneous decodings that are not common enough to be driven by
   flags.  */

static int
do_misc_decoding (aarch64_inst *inst)
{
  unsigned int value;
  switch (inst->opcode->op)
    {
    case OP_FCVT:
      return decode_fcvt (inst);

    case OP_FCVTN:
    case OP_FCVTN2:
    case OP_FCVTL:
    case OP_FCVTL2:
      return decode_asimd_fcvt (inst);

    case OP_FCVTXN_S:
      return decode_asisd_fcvtxn (inst);

    case OP_MOV_P_P:
    case OP_MOVS_P_P:
      value = extract_field (FLD_SVE_Pn, inst->value, 0);
      return (value == extract_field (FLD_SVE_Pm, inst->value, 0)
	      && value == extract_field (FLD_SVE_Pg4_10, inst->value, 0));

    case OP_MOV_Z_P_Z:
      return (extract_field (FLD_SVE_Zd, inst->value, 0)
	      == extract_field (FLD_SVE_Zm_16, inst->value, 0));

    case OP_MOV_Z_V:
      /* Index must be zero.  */
      value = extract_fields (inst->value, 0, 2, FLD_SVE_tszh, FLD_imm5);
      return value > 0 && value <= 16 && value == (value & -value);

    case OP_MOV_Z_Z:
      return (extract_field (FLD_SVE_Zn, inst->value, 0)
	      == extract_field (FLD_SVE_Zm_16, inst->value, 0));

    case OP_MOV_Z_Zi:
      /* Index must be nonzero.  */
      value = extract_fields (inst->value, 0, 2, FLD_SVE_tszh, FLD_imm5);
      return value > 0 && value != (value & -value);

    case OP_MOVM_P_P_P:
      return (extract_field (FLD_SVE_Pd, inst->value, 0)
	      == extract_field (FLD_SVE_Pm, inst->value, 0));

    case OP_MOVZS_P_P_P:
    case OP_MOVZ_P_P_P:
      return (extract_field (FLD_SVE_Pn, inst->value, 0)
	      == extract_field (FLD_SVE_Pm, inst->value, 0));

    case OP_NOTS_P_P_P_Z:
    case OP_NOT_P_P_P_Z:
      return (extract_field (FLD_SVE_Pm, inst->value, 0)
	      == extract_field (FLD_SVE_Pg4_10, inst->value, 0));

    default:
      return 0;
    }
}

/* Opcodes that have fields shared by multiple operands are usually flagged
   with flags.  In this function, we detect such flags, decode the related
   field(s) and store the information in one of the related operands.  The
   'one' operand is not any operand but one of the operands that can
   accommadate all the information that has been decoded.  */

static int
do_special_decoding (aarch64_inst *inst)
{
  int idx;
  aarch64_insn value;
  /* Condition for truly conditional executed instructions, e.g. b.cond.  */
  if (inst->opcode->flags & F_COND)
    {
      value = extract_field (FLD_cond2, inst->value, 0);
      inst->cond = get_cond_from_value (value);
    }
  /* 'sf' field.  */
  if (inst->opcode->flags & F_SF)
    {
      idx = select_operand_for_sf_field_coding (inst->opcode);
      value = extract_field (FLD_sf, inst->value, 0);
      inst->operands[idx].qualifier = get_greg_qualifier_from_value (value);
      if ((inst->opcode->flags & F_N)
	  && extract_field (FLD_N, inst->value, 0) != value)
	return 0;
    }
  /* 'sf' field.  */
  if (inst->opcode->flags & F_LSE_SZ)
    {
      idx = select_operand_for_sf_field_coding (inst->opcode);
      value = extract_field (FLD_lse_sz, inst->value, 0);
      inst->operands[idx].qualifier = get_greg_qualifier_from_value (value);
    }
  /* size:Q fields.  */
  if (inst->opcode->flags & F_SIZEQ)
    return decode_sizeq (inst);

  if (inst->opcode->flags & F_FPTYPE)
    {
      idx = select_operand_for_fptype_field_coding (inst->opcode);
      value = extract_field (FLD_type, inst->value, 0);
      switch (value)
	{
	case 0: inst->operands[idx].qualifier = AARCH64_OPND_QLF_S_S; break;
	case 1: inst->operands[idx].qualifier = AARCH64_OPND_QLF_S_D; break;
	case 3: inst->operands[idx].qualifier = AARCH64_OPND_QLF_S_H; break;
	default: return 0;
	}
    }

  if (inst->opcode->flags & F_SSIZE)
    {
      /* N.B. some opcodes like FCMGT <V><d>, <V><n>, #0 have the size[1] as part
	 of the base opcode.  */
      aarch64_insn mask;
      enum aarch64_opnd_qualifier candidates[AARCH64_MAX_QLF_SEQ_NUM];
      idx = select_operand_for_scalar_size_field_coding (inst->opcode);
      value = extract_field (FLD_size, inst->value, inst->opcode->mask);
      mask = extract_field (FLD_size, ~inst->opcode->mask, 0);
      /* For most related instruciton, the 'size' field is fully available for
	 operand encoding.  */
      if (mask == 0x3)
	inst->operands[idx].qualifier = get_sreg_qualifier_from_value (value);
      else
	{
	  get_operand_possible_qualifiers (idx, inst->opcode->qualifiers_list,
					   candidates);
	  inst->operands[idx].qualifier
	    = get_qualifier_from_partial_encoding (value, candidates, mask);
	}
    }

  if (inst->opcode->flags & F_T)
    {
      /* Num of consecutive '0's on the right side of imm5<3:0>.  */
      int num = 0;
      unsigned val, Q;
      assert (aarch64_get_operand_class (inst->opcode->operands[0])
	      == AARCH64_OPND_CLASS_SIMD_REG);
      /* imm5<3:0>	q	<t>
	 0000		x	reserved
	 xxx1		0	8b
	 xxx1		1	16b
	 xx10		0	4h
	 xx10		1	8h
	 x100		0	2s
	 x100		1	4s
	 1000		0	reserved
	 1000		1	2d  */
      val = extract_field (FLD_imm5, inst->value, 0);
      while ((val & 0x1) == 0 && ++num <= 3)
	val >>= 1;
      if (num > 3)
	return 0;
      Q = (unsigned) extract_field (FLD_Q, inst->value, inst->opcode->mask);
      inst->operands[0].qualifier =
	get_vreg_qualifier_from_value ((num << 1) | Q);
    }

  if (inst->opcode->flags & F_GPRSIZE_IN_Q)
    {
      /* Use Rt to encode in the case of e.g.
	 STXP <Ws>, <Xt1>, <Xt2>, [<Xn|SP>{,#0}].  */
      idx = aarch64_operand_index (inst->opcode->operands, AARCH64_OPND_Rt);
      if (idx == -1)
	{
	  /* Otherwise use the result operand, which has to be a integer
	     register.  */
	  assert (aarch64_get_operand_class (inst->opcode->operands[0])
		  == AARCH64_OPND_CLASS_INT_REG);
	  idx = 0;
	}
      assert (idx == 0 || idx == 1);
      value = extract_field (FLD_Q, inst->value, 0);
      inst->operands[idx].qualifier = get_greg_qualifier_from_value (value);
    }

  if (inst->opcode->flags & F_LDS_SIZE)
    {
      aarch64_field field = {0, 0};
      assert (aarch64_get_operand_class (inst->opcode->operands[0])
	      == AARCH64_OPND_CLASS_INT_REG);
      gen_sub_field (FLD_opc, 0, 1, &field);
      value = extract_field_2 (&field, inst->value, 0);
      inst->operands[0].qualifier
	= value ? AARCH64_OPND_QLF_W : AARCH64_OPND_QLF_X;
    }

  /* Miscellaneous decoding; done as the last step.  */
  if (inst->opcode->flags & F_MISC)
    return do_misc_decoding (inst);

  return 1;
}

/* Converters converting a real opcode instruction to its alias form.  */

/* ROR <Wd>, <Ws>, #<shift>
     is equivalent to:
   EXTR <Wd>, <Ws>, <Ws>, #<shift>.  */
static int
convert_extr_to_ror (aarch64_inst *inst)
{
  if (inst->operands[1].reg.regno == inst->operands[2].reg.regno)
    {
      copy_operand_info (inst, 2, 3);
      inst->operands[3].type = AARCH64_OPND_NIL;
      return 1;
    }
  return 0;
}

/* UXTL<Q> <Vd>.<Ta>, <Vn>.<Tb>
     is equivalent to:
   USHLL<Q> <Vd>.<Ta>, <Vn>.<Tb>, #0.  */
static int
convert_shll_to_xtl (aarch64_inst *inst)
{
  if (inst->operands[2].imm.value == 0)
    {
      inst->operands[2].type = AARCH64_OPND_NIL;
      return 1;
    }
  return 0;
}

/* Convert
     UBFM <Xd>, <Xn>, #<shift>, #63.
   to
     LSR <Xd>, <Xn>, #<shift>.  */
static int
convert_bfm_to_sr (aarch64_inst *inst)
{
  int64_t imms, val;

  imms = inst->operands[3].imm.value;
  val = inst->operands[2].qualifier == AARCH64_OPND_QLF_imm_0_31 ? 31 : 63;
  if (imms == val)
    {
      inst->operands[3].type = AARCH64_OPND_NIL;
      return 1;
    }

  return 0;
}

/* Convert MOV to ORR.  */
static int
convert_orr_to_mov (aarch64_inst *inst)
{
  /* MOV <Vd>.<T>, <Vn>.<T>
     is equivalent to:
     ORR <Vd>.<T>, <Vn>.<T>, <Vn>.<T>.  */
  if (inst->operands[1].reg.regno == inst->operands[2].reg.regno)
    {
      inst->operands[2].type = AARCH64_OPND_NIL;
      return 1;
    }
  return 0;
}

/* When <imms> >= <immr>, the instruction written:
     SBFX <Xd>, <Xn>, #<lsb>, #<width>
   is equivalent to:
     SBFM <Xd>, <Xn>, #<lsb>, #(<lsb>+<width>-1).  */

static int
convert_bfm_to_bfx (aarch64_inst *inst)
{
  int64_t immr, imms;

  immr = inst->operands[2].imm.value;
  imms = inst->operands[3].imm.value;
  if (imms >= immr)
    {
      int64_t lsb = immr;
      inst->operands[2].imm.value = lsb;
      inst->operands[3].imm.value = imms + 1 - lsb;
      /* The two opcodes have different qualifiers for
	 the immediate operands; reset to help the checking.  */
      reset_operand_qualifier (inst, 2);
      reset_operand_qualifier (inst, 3);
      return 1;
    }

  return 0;
}

/* When <imms> < <immr>, the instruction written:
     SBFIZ <Xd>, <Xn>, #<lsb>, #<width>
   is equivalent to:
     SBFM <Xd>, <Xn>, #((64-<lsb>)&0x3f), #(<width>-1).  */

static int
convert_bfm_to_bfi (aarch64_inst *inst)
{
  int64_t immr, imms, val;

  immr = inst->operands[2].imm.value;
  imms = inst->operands[3].imm.value;
  val = inst->operands[2].qualifier == AARCH64_OPND_QLF_imm_0_31 ? 32 : 64;
  if (imms < immr)
    {
      inst->operands[2].imm.value = (val - immr) & (val - 1);
      inst->operands[3].imm.value = imms + 1;
      /* The two opcodes have different qualifiers for
	 the immediate operands; reset to help the checking.  */
      reset_operand_qualifier (inst, 2);
      reset_operand_qualifier (inst, 3);
      return 1;
    }

  return 0;
}

/* The instruction written:
     BFC <Xd>, #<lsb>, #<width>
   is equivalent to:
     BFM <Xd>, XZR, #((64-<lsb>)&0x3f), #(<width>-1).  */

static int
convert_bfm_to_bfc (aarch64_inst *inst)
{
  int64_t immr, imms, val;

  /* Should have been assured by the base opcode value.  */
  assert (inst->operands[1].reg.regno == 0x1f);

  immr = inst->operands[2].imm.value;
  imms = inst->operands[3].imm.value;
  val = inst->operands[2].qualifier == AARCH64_OPND_QLF_imm_0_31 ? 32 : 64;
  if (imms < immr)
    {
      /* Drop XZR from the second operand.  */
      copy_operand_info (inst, 1, 2);
      copy_operand_info (inst, 2, 3);
      inst->operands[3].type = AARCH64_OPND_NIL;

      /* Recalculate the immediates.  */
      inst->operands[1].imm.value = (val - immr) & (val - 1);
      inst->operands[2].imm.value = imms + 1;

      /* The two opcodes have different qualifiers for the operands; reset to
	 help the checking.  */
      reset_operand_qualifier (inst, 1);
      reset_operand_qualifier (inst, 2);
      reset_operand_qualifier (inst, 3);

      return 1;
    }

  return 0;
}

/* The instruction written:
     LSL <Xd>, <Xn>, #<shift>
   is equivalent to:
     UBFM <Xd>, <Xn>, #((64-<shift>)&0x3f), #(63-<shift>).  */

static int
convert_ubfm_to_lsl (aarch64_inst *inst)
{
  int64_t immr = inst->operands[2].imm.value;
  int64_t imms = inst->operands[3].imm.value;
  int64_t val
    = inst->operands[2].qualifier == AARCH64_OPND_QLF_imm_0_31 ? 31 : 63;

  if ((immr == 0 && imms == val) || immr == imms + 1)
    {
      inst->operands[3].type = AARCH64_OPND_NIL;
      inst->operands[2].imm.value = val - imms;
      return 1;
    }

  return 0;
}

/* CINC <Wd>, <Wn>, <cond>
     is equivalent to:
   CSINC <Wd>, <Wn>, <Wn>, invert(<cond>)
     where <cond> is not AL or NV.  */

static int
convert_from_csel (aarch64_inst *inst)
{
  if (inst->operands[1].reg.regno == inst->operands[2].reg.regno
      && (inst->operands[3].cond->value & 0xe) != 0xe)
    {
      copy_operand_info (inst, 2, 3);
      inst->operands[2].cond = get_inverted_cond (inst->operands[3].cond);
      inst->operands[3].type = AARCH64_OPND_NIL;
      return 1;
    }
  return 0;
}

/* CSET <Wd>, <cond>
     is equivalent to:
   CSINC <Wd>, WZR, WZR, invert(<cond>)
     where <cond> is not AL or NV.  */

static int
convert_csinc_to_cset (aarch64_inst *inst)
{
  if (inst->operands[1].reg.regno == 0x1f
      && inst->operands[2].reg.regno == 0x1f
      && (inst->operands[3].cond->value & 0xe) != 0xe)
    {
      copy_operand_info (inst, 1, 3);
      inst->operands[1].cond = get_inverted_cond (inst->operands[3].cond);
      inst->operands[3].type = AARCH64_OPND_NIL;
      inst->operands[2].type = AARCH64_OPND_NIL;
      return 1;
    }
  return 0;
}

/* MOV <Wd>, #<imm>
     is equivalent to:
   MOVZ <Wd>, #<imm16_5>, LSL #<shift>.

   A disassembler may output ORR, MOVZ and MOVN as a MOV mnemonic, except when
   ORR has an immediate that could be generated by a MOVZ or MOVN instruction,
   or where a MOVN has an immediate that could be encoded by MOVZ, or where
   MOVZ/MOVN #0 have a shift amount other than LSL #0, in which case the
   machine-instruction mnemonic must be used.  */

static int
convert_movewide_to_mov (aarch64_inst *inst)
{
  uint64_t value = inst->operands[1].imm.value;
  /* MOVZ/MOVN #0 have a shift amount other than LSL #0.  */
  if (value == 0 && inst->operands[1].shifter.amount != 0)
    return 0;
  inst->operands[1].type = AARCH64_OPND_IMM_MOV;
  inst->operands[1].shifter.kind = AARCH64_MOD_NONE;
  value <<= inst->operands[1].shifter.amount;
  /* As an alias convertor, it has to be clear that the INST->OPCODE
     is the opcode of the real instruction.  */
  if (inst->opcode->op == OP_MOVN)
    {
      int is32 = inst->operands[0].qualifier == AARCH64_OPND_QLF_W;
      value = ~value;
      /* A MOVN has an immediate that could be encoded by MOVZ.  */
      if (aarch64_wide_constant_p (value, is32, NULL))
	return 0;
    }
  inst->operands[1].imm.value = value;
  inst->operands[1].shifter.amount = 0;
  return 1;
}

/* MOV <Wd>, #<imm>
     is equivalent to:
   ORR <Wd>, WZR, #<imm>.

   A disassembler may output ORR, MOVZ and MOVN as a MOV mnemonic, except when
   ORR has an immediate that could be generated by a MOVZ or MOVN instruction,
   or where a MOVN has an immediate that could be encoded by MOVZ, or where
   MOVZ/MOVN #0 have a shift amount other than LSL #0, in which case the
   machine-instruction mnemonic must be used.  */

static int
convert_movebitmask_to_mov (aarch64_inst *inst)
{
  int is32;
  uint64_t value;

  /* Should have been assured by the base opcode value.  */
  assert (inst->operands[1].reg.regno == 0x1f);
  copy_operand_info (inst, 1, 2);
  is32 = inst->operands[0].qualifier == AARCH64_OPND_QLF_W;
  inst->operands[1].type = AARCH64_OPND_IMM_MOV;
  value = inst->operands[1].imm.value;
  /* ORR has an immediate that could be generated by a MOVZ or MOVN
     instruction.  */
  if (inst->operands[0].reg.regno != 0x1f
      && (aarch64_wide_constant_p (value, is32, NULL)
	  || aarch64_wide_constant_p (~value, is32, NULL)))
    return 0;

  inst->operands[2].type = AARCH64_OPND_NIL;
  return 1;
}

/* Some alias opcodes are disassembled by being converted from their real-form.
   N.B. INST->OPCODE is the real opcode rather than the alias.  */

static int
convert_to_alias (aarch64_inst *inst, const aarch64_opcode *alias)
{
  switch (alias->op)
    {
    case OP_ASR_IMM:
    case OP_LSR_IMM:
      return convert_bfm_to_sr (inst);
    case OP_LSL_IMM:
      return convert_ubfm_to_lsl (inst);
    case OP_CINC:
    case OP_CINV:
    case OP_CNEG:
      return convert_from_csel (inst);
    case OP_CSET:
    case OP_CSETM:
      return convert_csinc_to_cset (inst);
    case OP_UBFX:
    case OP_BFXIL:
    case OP_SBFX:
      return convert_bfm_to_bfx (inst);
    case OP_SBFIZ:
    case OP_BFI:
    case OP_UBFIZ:
      return convert_bfm_to_bfi (inst);
    case OP_BFC:
      return convert_bfm_to_bfc (inst);
    case OP_MOV_V:
      return convert_orr_to_mov (inst);
    case OP_MOV_IMM_WIDE:
    case OP_MOV_IMM_WIDEN:
      return convert_movewide_to_mov (inst);
    case OP_MOV_IMM_LOG:
      return convert_movebitmask_to_mov (inst);
    case OP_ROR_IMM:
      return convert_extr_to_ror (inst);
    case OP_SXTL:
    case OP_SXTL2:
    case OP_UXTL:
    case OP_UXTL2:
      return convert_shll_to_xtl (inst);
    default:
      return 0;
    }
}

static bool
aarch64_opcode_decode (const aarch64_opcode *, const aarch64_insn,
		       aarch64_inst *, int, aarch64_operand_error *errors);

/* Given the instruction information in *INST, check if the instruction has
   any alias form that can be used to represent *INST.  If the answer is yes,
   update *INST to be in the form of the determined alias.  */

/* In the opcode description table, the following flags are used in opcode
   entries to help establish the relations between the real and alias opcodes:

	F_ALIAS:	opcode is an alias
	F_HAS_ALIAS:	opcode has alias(es)
	F_P1
	F_P2
	F_P3:		Disassembly preference priority 1-3 (the larger the
			higher).  If nothing is specified, it is the priority
			0 by default, i.e. the lowest priority.

   Although the relation between the machine and the alias instructions are not
   explicitly described, it can be easily determined from the base opcode
   values, masks and the flags F_ALIAS and F_HAS_ALIAS in their opcode
   description entries:

   The mask of an alias opcode must be equal to or a super-set (i.e. more
   constrained) of that of the aliased opcode; so is the base opcode value.

   if (opcode_has_alias (real) && alias_opcode_p (opcode)
       && (opcode->mask & real->mask) == real->mask
       && (real->mask & opcode->opcode) == (real->mask & real->opcode))
   then OPCODE is an alias of, and only of, the REAL instruction

   The alias relationship is forced flat-structured to keep related algorithm
   simple; an opcode entry cannot be flagged with both F_ALIAS and F_HAS_ALIAS.

   During the disassembling, the decoding decision tree (in
   opcodes/aarch64-dis-2.c) always returns an machine instruction opcode entry;
   if the decoding of such a machine instruction succeeds (and -Mno-aliases is
   not specified), the disassembler will check whether there is any alias
   instruction exists for this real instruction.  If there is, the disassembler
   will try to disassemble the 32-bit binary again using the alias's rule, or
   try to convert the IR to the form of the alias.  In the case of the multiple
   aliases, the aliases are tried one by one from the highest priority
   (currently the flag F_P3) to the lowest priority (no priority flag), and the
   first succeeds first adopted.

   You may ask why there is a need for the conversion of IR from one form to
   another in handling certain aliases.  This is because on one hand it avoids
   adding more operand code to handle unusual encoding/decoding; on other
   hand, during the disassembling, the conversion is an effective approach to
   check the condition of an alias (as an alias may be adopted only if certain
   conditions are met).

   In order to speed up the alias opcode lookup, aarch64-gen has preprocessed
   aarch64_opcode_table and generated aarch64_find_alias_opcode and
   aarch64_find_next_alias_opcode (in opcodes/aarch64-dis-2.c) to help.  */

static void
determine_disassembling_preference (struct aarch64_inst *inst,
				    aarch64_operand_error *errors)
{
  const aarch64_opcode *opcode;
  const aarch64_opcode *alias;

  opcode = inst->opcode;

  /* This opcode does not have an alias, so use itself.  */
  if (!opcode_has_alias (opcode))
    return;

  alias = aarch64_find_alias_opcode (opcode);
  assert (alias);

#ifdef DEBUG_AARCH64
  if (debug_dump)
    {
      const aarch64_opcode *tmp = alias;
      printf ("####   LIST    orderd: ");
      while (tmp)
	{
	  printf ("%s, ", tmp->name);
	  tmp = aarch64_find_next_alias_opcode (tmp);
	}
      printf ("\n");
    }
#endif /* DEBUG_AARCH64 */

  for (; alias; alias = aarch64_find_next_alias_opcode (alias))
    {
      DEBUG_TRACE ("try %s", alias->name);
      assert (alias_opcode_p (alias) || opcode_has_alias (opcode));

      /* An alias can be a pseudo opcode which will never be used in the
	 disassembly, e.g. BIC logical immediate is such a pseudo opcode
	 aliasing AND.  */
      if (pseudo_opcode_p (alias))
	{
	  DEBUG_TRACE ("skip pseudo %s", alias->name);
	  continue;
	}

      if ((inst->value & alias->mask) != alias->opcode)
	{
	  DEBUG_TRACE ("skip %s as base opcode not match", alias->name);
	  continue;
	}

      if (!AARCH64_CPU_HAS_FEATURE (arch_variant, *alias->avariant))
	{
	  DEBUG_TRACE ("skip %s: we're missing features", alias->name);
	  continue;
	}

      /* No need to do any complicated transformation on operands, if the alias
	 opcode does not have any operand.  */
      if (aarch64_num_of_operands (alias) == 0 && alias->opcode == inst->value)
	{
	  DEBUG_TRACE ("succeed with 0-operand opcode %s", alias->name);
	  aarch64_replace_opcode (inst, alias);
	  return;
	}
      if (alias->flags & F_CONV)
	{
	  aarch64_inst copy;
	  memcpy (&copy, inst, sizeof (aarch64_inst));
	  /* ALIAS is the preference as long as the instruction can be
	     successfully converted to the form of ALIAS.  */
	  if (convert_to_alias (&copy, alias) == 1)
	    {
	      aarch64_replace_opcode (&copy, alias);
	      if (aarch64_match_operands_constraint (&copy, NULL) != 1)
		{
		  DEBUG_TRACE ("FAILED with alias %s ", alias->name);
		}
	      else
		{
		  DEBUG_TRACE ("succeed with %s via conversion", alias->name);
		  memcpy (inst, &copy, sizeof (aarch64_inst));
		}
	      return;
	    }
	}
      else
	{
	  /* Directly decode the alias opcode.  */
	  aarch64_inst temp;
	  memset (&temp, '\0', sizeof (aarch64_inst));
	  if (aarch64_opcode_decode (alias, inst->value, &temp, 1, errors) == 1)
	    {
	      DEBUG_TRACE ("succeed with %s via direct decoding", alias->name);
	      memcpy (inst, &temp, sizeof (aarch64_inst));
	      return;
	    }
	}
    }
}

/* Some instructions (including all SVE ones) use the instruction class
   to describe how a qualifiers_list index is represented in the instruction
   encoding.  If INST is such an instruction, decode the appropriate fields
   and fill in the operand qualifiers accordingly.  Return true if no
   problems are found.  */

static bool
aarch64_decode_variant_using_iclass (aarch64_inst *inst)
{
  int i, variant;

  variant = 0;
  switch (inst->opcode->iclass)
    {
    case sme_mov:
      variant = extract_fields (inst->value, 0, 2, FLD_SME_Q, FLD_SME_size_22);
      if (variant >= 4 && variant < 7)
	return false;
      if (variant == 7)
	variant = 4;
      break;

    case sme_psel:
      i = extract_fields (inst->value, 0, 2, FLD_SME_tszh, FLD_SME_tszl);
      if (i == 0)
	return false;
      while ((i & 1) == 0)
	{
	  i >>= 1;
	  variant += 1;
	}
      break;

    case sme_shift:
      i = extract_field (FLD_SVE_tszh, inst->value, 0);
      goto sve_shift;

    case sme_size_12_bhs:
      variant = extract_field (FLD_SME_size_12, inst->value, 0);
      if (variant >= 3)
	return false;
      break;

    case sme_size_12_hs:
      variant = extract_field (FLD_SME_size_12, inst->value, 0);
      if (variant != 1 && variant != 2)
	return false;
      variant -= 1;
      break;

    case sme_size_22:
      variant = extract_field (FLD_SME_size_22, inst->value, 0);
      break;

    case sme_size_22_hsd:
      variant = extract_field (FLD_SME_size_22, inst->value, 0);
      if (variant < 1)
	return false;
      variant -= 1;
      break;

    case sme_sz_23:
      variant = extract_field (FLD_SME_sz_23, inst->value, 0);
      break;

    case sve_cpy:
      variant = extract_fields (inst->value, 0, 2, FLD_size, FLD_SVE_M_14);
      break;

    case sve_index:
      i = extract_fields (inst->value, 0, 2, FLD_SVE_tszh, FLD_imm5);
      if ((i & 31) == 0)
	return false;
      while ((i & 1) == 0)
	{
	  i >>= 1;
	  variant += 1;
	}
      break;

    case sve_limm:
      /* Pick the smallest applicable element size.  */
      if ((inst->value & 0x20600) == 0x600)
	variant = 0;
      else if ((inst->value & 0x20400) == 0x400)
	variant = 1;
      else if ((inst->value & 0x20000) == 0)
	variant = 2;
      else
	variant = 3;
      break;

    case sme2_mov:
      /* .D is preferred over the other sizes in disassembly.  */
      variant = 3;
      break;

    case sme_misc:
    case sve_misc:
      /* These instructions have only a single variant.  */
      break;

    case sve_movprfx:
      variant = extract_fields (inst->value, 0, 2, FLD_size, FLD_SVE_M_16);
      break;

    case sve_pred_zm:
      variant = extract_field (FLD_SVE_M_4, inst->value, 0);
      break;

    case sve_shift_pred:
      i = extract_fields (inst->value, 0, 2, FLD_SVE_tszh, FLD_SVE_tszl_8);
    sve_shift:
      if (i == 0)
	return false;
      while (i != 1)
	{
	  i >>= 1;
	  variant += 1;
	}
      break;

    case sve_shift_unpred:
      i = extract_fields (inst->value, 0, 2, FLD_SVE_tszh, FLD_SVE_tszl_19);
      goto sve_shift;

    case sve_size_bhs:
      variant = extract_field (FLD_size, inst->value, 0);
      if (variant >= 3)
	return false;
      break;

    case sve_size_bhsd:
      variant = extract_field (FLD_size, inst->value, 0);
      break;

    case sve_size_hsd:
      i = extract_field (FLD_size, inst->value, 0);
      if (i < 1)
	return false;
      variant = i - 1;
      break;

    case sme_fp_sd:
    case sme_int_sd:
    case sve_size_bh:
    case sve_size_sd:
      variant = extract_field (FLD_SVE_sz, inst->value, 0);
      break;

    case sve_size_sd2:
      variant = extract_field (FLD_SVE_sz2, inst->value, 0);
      break;

    case sve_size_hsd2:
      i = extract_field (FLD_SVE_size, inst->value, 0);
      if (i < 1)
	return false;
      variant = i - 1;
      break;

    case sve_size_13:
      /* Ignore low bit of this field since that is set in the opcode for
	 instructions of this iclass.  */
      i = (extract_field (FLD_size, inst->value, 0) & 2);
      variant = (i >> 1);
      break;

    case sve_shift_tsz_bhsd:
      i = extract_fields (inst->value, 0, 2, FLD_SVE_tszh, FLD_SVE_tszl_19);
      if (i == 0)
	return false;
      while (i != 1)
	{
	  i >>= 1;
	  variant += 1;
	}
      break;

    case sve_size_tsz_bhs:
      i = extract_fields (inst->value, 0, 2, FLD_SVE_sz, FLD_SVE_tszl_19);
      if (i == 0)
	return false;
      while (i != 1)
	{
	  if (i & 1)
	    return false;
	  i >>= 1;
	  variant += 1;
	}
      break;

    case sve_shift_tsz_hsd:
      i = extract_fields (inst->value, 0, 2, FLD_SVE_sz, FLD_SVE_tszl_19);
      if (i == 0)
	return false;
      while (i != 1)
	{
	  i >>= 1;
	  variant += 1;
	}
      break;

    default:
      /* No mapping between instruction class and qualifiers.  */
      return true;
    }

  for (i = 0; i < AARCH64_MAX_OPND_NUM; ++i)
    inst->operands[i].qualifier = inst->opcode->qualifiers_list[variant][i];
  return true;
}
/* Decode the CODE according to OPCODE; fill INST.  Return 0 if the decoding
   fails, which meanes that CODE is not an instruction of OPCODE; otherwise
   return 1.

   If OPCODE has alias(es) and NOALIASES_P is 0, an alias opcode may be
   determined and used to disassemble CODE; this is done just before the
   return.  */

static bool
aarch64_opcode_decode (const aarch64_opcode *opcode, const aarch64_insn code,
		       aarch64_inst *inst, int noaliases_p,
		       aarch64_operand_error *errors)
{
  int i;

  DEBUG_TRACE ("enter with %s", opcode->name);

  assert (opcode && inst);

  /* Clear inst.  */
  memset (inst, '\0', sizeof (aarch64_inst));

  /* Check the base opcode.  */
  if ((code & opcode->mask) != (opcode->opcode & opcode->mask))
    {
      DEBUG_TRACE ("base opcode match FAIL");
      goto decode_fail;
    }

  inst->opcode = opcode;
  inst->value = code;

  /* Assign operand codes and indexes.  */
  for (i = 0; i < AARCH64_MAX_OPND_NUM; ++i)
    {
      if (opcode->operands[i] == AARCH64_OPND_NIL)
	break;
      inst->operands[i].type = opcode->operands[i];
      inst->operands[i].idx = i;
    }

  /* Call the opcode decoder indicated by flags.  */
  if (opcode_has_special_coder (opcode) && do_special_decoding (inst) == 0)
    {
      DEBUG_TRACE ("opcode flag-based decoder FAIL");
      goto decode_fail;
    }

  /* Possibly use the instruction class to determine the correct
     qualifier.  */
  if (!aarch64_decode_variant_using_iclass (inst))
    {
      DEBUG_TRACE ("iclass-based decoder FAIL");
      goto decode_fail;
    }

  /* Call operand decoders.  */
  for (i = 0; i < AARCH64_MAX_OPND_NUM; ++i)
    {
      const aarch64_operand *opnd;
      enum aarch64_opnd type;

      type = opcode->operands[i];
      if (type == AARCH64_OPND_NIL)
	break;
      opnd = &aarch64_operands[type];
      if (operand_has_extractor (opnd)
	  && (! aarch64_extract_operand (opnd, &inst->operands[i], code, inst,
					 errors)))
	{
	  DEBUG_TRACE ("operand decoder FAIL at operand %d", i);
	  goto decode_fail;
	}
    }

  /* If the opcode has a verifier, then check it now.  */
  if (opcode->verifier
      && opcode->verifier (inst, code, 0, false, errors, NULL) != ERR_OK)
    {
      DEBUG_TRACE ("operand verifier FAIL");
      goto decode_fail;
    }

  /* Match the qualifiers.  */
  if (aarch64_match_operands_constraint (inst, NULL) == 1)
    {
      /* Arriving here, the CODE has been determined as a valid instruction
	 of OPCODE and *INST has been filled with information of this OPCODE
	 instruction.  Before the return, check if the instruction has any
	 alias and should be disassembled in the form of its alias instead.
	 If the answer is yes, *INST will be updated.  */
      if (!noaliases_p)
	determine_disassembling_preference (inst, errors);
      DEBUG_TRACE ("SUCCESS");
      return true;
    }
  else
    {
      DEBUG_TRACE ("constraint matching FAIL");
    }

 decode_fail:
  return false;
}

/* This does some user-friendly fix-up to *INST.  It is currently focus on
   the adjustment of qualifiers to help the printed instruction
   recognized/understood more easily.  */

static void
user_friendly_fixup (aarch64_inst *inst)
{
  switch (inst->opcode->iclass)
    {
    case testbranch:
      /* TBNZ Xn|Wn, #uimm6, label
	 Test and Branch Not Zero: conditionally jumps to label if bit number
	 uimm6 in register Xn is not zero.  The bit number implies the width of
	 the register, which may be written and should be disassembled as Wn if
	 uimm is less than 32. Limited to a branch offset range of +/- 32KiB.
	 */
      if (inst->operands[1].imm.value < 32)
	inst->operands[0].qualifier = AARCH64_OPND_QLF_W;
      break;
    default: break;
    }
}

/* Decode INSN and fill in *INST the instruction information.  An alias
   opcode may be filled in *INSN if NOALIASES_P is FALSE.  Return zero on
   success.  */

enum err_type
aarch64_decode_insn (aarch64_insn insn, aarch64_inst *inst,
		     bool noaliases_p,
		     aarch64_operand_error *errors)
{
  const aarch64_opcode *opcode = aarch64_opcode_lookup (insn);

#ifdef DEBUG_AARCH64
  if (debug_dump)
    {
      const aarch64_opcode *tmp = opcode;
      printf ("\n");
      DEBUG_TRACE ("opcode lookup:");
      while (tmp != NULL)
	{
	  aarch64_verbose ("  %s", tmp->name);
	  tmp = aarch64_find_next_opcode (tmp);
	}
    }
#endif /* DEBUG_AARCH64 */

  /* A list of opcodes may have been found, as aarch64_opcode_lookup cannot
     distinguish some opcodes, e.g. SSHR and MOVI, which almost share the same
     opcode field and value, apart from the difference that one of them has an
     extra field as part of the opcode, but such a field is used for operand
     encoding in other opcode(s) ('immh' in the case of the example).  */
  while (opcode != NULL)
    {
      /* But only one opcode can be decoded successfully for, as the
	 decoding routine will check the constraint carefully.  */
      if (aarch64_opcode_decode (opcode, insn, inst, noaliases_p, errors) == 1)
	return ERR_OK;
      opcode = aarch64_find_next_opcode (opcode);
    }

  return ERR_UND;
}

/* Return a short string to indicate a switch to STYLE.  These strings
   will be embedded into the disassembled operand text (as produced by
   aarch64_print_operand), and then spotted in the print_operands function
   so that the disassembler output can be split by style.  */

static const char *
get_style_text (enum disassembler_style style)
{
  static bool init = false;
  static char formats[16][4];
  unsigned num;

  /* First time through we build a string for every possible format.  This
     code relies on there being no more than 16 different styles (there's
     an assert below for this).  */
  if (!init)
    {
      int i;

      for (i = 0; i <= 0xf; ++i)
	{
	  int res = snprintf (&formats[i][0], sizeof (formats[i]), "%c%x%c",
			      STYLE_MARKER_CHAR, i, STYLE_MARKER_CHAR);
	  assert (res == 3);
	}

      init = true;
    }

  /* Return the string that marks switching to STYLE.  */
  num = (unsigned) style;
  assert (style <= 0xf);
  return formats[num];
}

/* Callback used by aarch64_print_operand to apply STYLE to the
   disassembler output created from FMT and ARGS.  The STYLER object holds
   any required state.  Must return a pointer to a string (created from FMT
   and ARGS) that will continue to be valid until the complete disassembled
   instruction has been printed.

   We return a string that includes two embedded style markers, the first,
   places at the start of the string, indicates a switch to STYLE, and the
   second, placed at the end of the string, indicates a switch back to the
   default text style.

   Later, when we print the operand text we take care to collapse any
   adjacent style markers, and to ignore any style markers that appear at
   the very end of a complete operand string.  */

static const char *aarch64_apply_style (struct aarch64_styler *styler,
					enum disassembler_style style,
					const char *fmt,
					va_list args)
{
  int res;
  char *ptr, *tmp;
  struct obstack *stack = (struct obstack *) styler->state;
  va_list ap;

  /* These are the two strings for switching styles.  */
  const char *style_on = get_style_text (style);
  const char *style_off = get_style_text (dis_style_text);

  /* Calculate space needed once FMT and ARGS are expanded.  */
  va_copy (ap, args);
  res = vsnprintf (NULL, 0, fmt, ap);
  va_end (ap);
  assert (res >= 0);

  /* Allocate space on the obstack for the expanded FMT and ARGS, as well
     as the two strings for switching styles, then write all of these
     strings onto the obstack.  */
  ptr = (char *) obstack_alloc (stack, res + strlen (style_on)
				+ strlen (style_off) + 1);
  tmp = stpcpy (ptr, style_on);
  res = vsnprintf (tmp, (res + 1), fmt, args);
  assert (res >= 0);
  tmp += res;
  strcpy (tmp, style_off);

  return ptr;
}

/* Print operands.  */

static void
print_operands (bfd_vma pc, const aarch64_opcode *opcode,
		const aarch64_opnd_info *opnds, struct disassemble_info *info,
		bool *has_notes)
{
  char *notes = NULL;
  int i, pcrel_p, num_printed;
  struct aarch64_styler styler;
  struct obstack content;
  obstack_init (&content);

  styler.apply_style = aarch64_apply_style;
  styler.state = (void *) &content;

  for (i = 0, num_printed = 0; i < AARCH64_MAX_OPND_NUM; ++i)
    {
      char str[128];
      char cmt[128];

      /* We regard the opcode operand info more, however we also look into
	 the inst->operands to support the disassembling of the optional
	 operand.
	 The two operand code should be the same in all cases, apart from
	 when the operand can be optional.  */
      if (opcode->operands[i] == AARCH64_OPND_NIL
	  || opnds[i].type == AARCH64_OPND_NIL)
	break;

      /* Generate the operand string in STR.  */
      aarch64_print_operand (str, sizeof (str), pc, opcode, opnds, i, &pcrel_p,
			     &info->target, &notes, cmt, sizeof (cmt),
			     arch_variant, &styler);

      /* Print the delimiter (taking account of omitted operand(s)).  */
      if (str[0] != '\0')
	(*info->fprintf_styled_func) (info->stream, dis_style_text, "%s",
				      num_printed++ == 0 ? "\t" : ", ");

      /* Print the operand.  */
      if (pcrel_p)
	(*info->print_address_func) (info->target, info);
      else
	{
	  /* This operand came from aarch64_print_operand, and will include
	     embedded strings indicating which style each character should
	     have.  In the following code we split the text based on
	     CURR_STYLE, and call the styled print callback to print each
	     block of text in the appropriate style.  */
	  char *start, *curr;
	  enum disassembler_style curr_style = dis_style_text;

	  start = curr = str;
	  do
	    {
	      if (*curr == '\0'
		  || (*curr == STYLE_MARKER_CHAR
		      && ISXDIGIT (*(curr + 1))
		      && *(curr + 2) == STYLE_MARKER_CHAR))
		{
		  /* Output content between our START position and CURR.  */
		  int len = curr - start;
		  if (len > 0)
		    {
		      if ((*info->fprintf_styled_func) (info->stream,
							curr_style,
							"%.*s",
							len, start) < 0)
			break;
		    }

		  if (*curr == '\0')
		    break;

		  /* Skip over the initial STYLE_MARKER_CHAR.  */
		  ++curr;

		  /* Update the CURR_STYLE.  As there are less than 16
		     styles, it is possible, that if the input is corrupted
		     in some way, that we might set CURR_STYLE to an
		     invalid value.  Don't worry though, we check for this
		     situation.  */
		  if (*curr >= '0' && *curr <= '9')
		    curr_style = (enum disassembler_style) (*curr - '0');
		  else if (*curr >= 'a' && *curr <= 'f')
		    curr_style = (enum disassembler_style) (*curr - 'a' + 10);
		  else
		    curr_style = dis_style_text;

		  /* Check for an invalid style having been selected.  This
		     should never happen, but it doesn't hurt to be a
		     little paranoid.  */
		  if (curr_style > dis_style_comment_start)
		    curr_style = dis_style_text;

		  /* Skip the hex character, and the closing STYLE_MARKER_CHAR.  */
		  curr += 2;

		  /* Reset the START to after the style marker.  */
		  start = curr;
		}
	      else
		++curr;
	    }
	  while (true);
	}

      /* Print the comment.  This works because only the last operand ever
	 adds a comment.  If that ever changes then we'll need to be
	 smarter here.  */
      if (cmt[0] != '\0')
	(*info->fprintf_styled_func) (info->stream, dis_style_comment_start,
				      "\t// %s", cmt);
    }

    if (notes && !no_notes)
      {
	*has_notes = true;
	(*info->fprintf_styled_func) (info->stream, dis_style_comment_start,
				      "  // note: %s", notes);
      }

    obstack_free (&content, NULL);
}

/* Set NAME to a copy of INST's mnemonic with the "." suffix removed.  */

static void
remove_dot_suffix (char *name, const aarch64_inst *inst)
{
  char *ptr;
  size_t len;

  ptr = strchr (inst->opcode->name, '.');
  assert (ptr && inst->cond);
  len = ptr - inst->opcode->name;
  assert (len < 8);
  strncpy (name, inst->opcode->name, len);
  name[len] = '\0';
}

/* Print the instruction mnemonic name.  */

static void
print_mnemonic_name (const aarch64_inst *inst, struct disassemble_info *info)
{
  if (inst->opcode->flags & F_COND)
    {
      /* For instructions that are truly conditionally executed, e.g. b.cond,
	 prepare the full mnemonic name with the corresponding condition
	 suffix.  */
      char name[8];

      remove_dot_suffix (name, inst);
      (*info->fprintf_styled_func) (info->stream, dis_style_mnemonic,
				    "%s.%s", name, inst->cond->names[0]);
    }
  else
    (*info->fprintf_styled_func) (info->stream, dis_style_mnemonic,
				  "%s", inst->opcode->name);
}

/* Decide whether we need to print a comment after the operands of
   instruction INST.  */

static void
print_comment (const aarch64_inst *inst, struct disassemble_info *info)
{
  if (inst->opcode->flags & F_COND)
    {
      char name[8];
      unsigned int i, num_conds;

      remove_dot_suffix (name, inst);
      num_conds = ARRAY_SIZE (inst->cond->names);
      for (i = 1; i < num_conds && inst->cond->names[i]; ++i)
	(*info->fprintf_styled_func) (info->stream, dis_style_comment_start,
				      "%s %s.%s",
				      i == 1 ? "  //" : ",",
				      name, inst->cond->names[i]);
    }
}

/* Build notes from verifiers into a string for printing.  */

static void
print_verifier_notes (aarch64_operand_error *detail,
		      struct disassemble_info *info)
{
  if (no_notes)
    return;

  /* The output of the verifier cannot be a fatal error, otherwise the assembly
     would not have succeeded.  We can safely ignore these.  */
  assert (detail->non_fatal);

  (*info->fprintf_styled_func) (info->stream, dis_style_comment_start,
				"  // note: ");
  switch (detail->kind)
    {
    case AARCH64_OPDE_A_SHOULD_FOLLOW_B:
      (*info->fprintf_styled_func) (info->stream, dis_style_text,
				    _("this `%s' should have an immediately"
				      " preceding `%s'"),
				    detail->data[0].s, detail->data[1].s);
      break;

    case AARCH64_OPDE_EXPECTED_A_AFTER_B:
      (*info->fprintf_styled_func) (info->stream, dis_style_text,
				    _("expected `%s' after previous `%s'"),
				    detail->data[0].s, detail->data[1].s);
      break;

    default:
      assert (detail->error);
      (*info->fprintf_styled_func) (info->stream, dis_style_text,
				    "%s", detail->error);
      if (detail->index >= 0)
	(*info->fprintf_styled_func) (info->stream, dis_style_text,
				      " at operand %d", detail->index + 1);
      break;
    }
}

/* Print the instruction according to *INST.  */

static void
print_aarch64_insn (bfd_vma pc, const aarch64_inst *inst,
		    const aarch64_insn code,
		    struct disassemble_info *info,
		    aarch64_operand_error *mismatch_details)
{
  bool has_notes = false;

  print_mnemonic_name (inst, info);
  print_operands (pc, inst->opcode, inst->operands, info, &has_notes);
  print_comment (inst, info);

  /* We've already printed a note, not enough space to print more so exit.
     Usually notes shouldn't overlap so it shouldn't happen that we have a note
     from a register and instruction at the same time.  */
  if (has_notes)
    return;

  /* Always run constraint verifiers, this is needed because constraints need to
     maintain a global state regardless of whether the instruction has the flag
     set or not.  */
  enum err_type result = verify_constraints (inst, code, pc, false,
					     mismatch_details, &insn_sequence);
  switch (result)
    {
    case ERR_VFI:
      print_verifier_notes (mismatch_details, info);
      break;
    case ERR_UND:
    case ERR_UNP:
    case ERR_NYI:
    default:
      break;
    }
}

/* Entry-point of the instruction disassembler and printer.  */

static void
print_insn_aarch64_word (bfd_vma pc,
			 uint32_t word,
			 struct disassemble_info *info,
			 aarch64_operand_error *errors)
{
  static const char *err_msg[ERR_NR_ENTRIES+1] =
    {
      [ERR_OK]  = "_",
      [ERR_UND] = "undefined",
      [ERR_UNP] = "unpredictable",
      [ERR_NYI] = "NYI"
    };

  enum err_type ret;
  aarch64_inst inst;

  info->insn_info_valid = 1;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->target = 0;
  info->target2 = 0;

  if (info->flags & INSN_HAS_RELOC)
    /* If the instruction has a reloc associated with it, then
       the offset field in the instruction will actually be the
       addend for the reloc.  (If we are using REL type relocs).
       In such cases, we can ignore the pc when computing
       addresses, since the addend is not currently pc-relative.  */
    pc = 0;

  ret = aarch64_decode_insn (word, &inst, no_aliases, errors);

  if (((word >> 21) & 0x3ff) == 1)
    {
      /* RESERVED for ALES.  */
      assert (ret != ERR_OK);
      ret = ERR_NYI;
    }

  switch (ret)
    {
    case ERR_UND:
    case ERR_UNP:
    case ERR_NYI:
      /* Handle undefined instructions.  */
      info->insn_type = dis_noninsn;
      (*info->fprintf_styled_func) (info->stream,
				    dis_style_assembler_directive,
				    ".inst\t");
      (*info->fprintf_styled_func) (info->stream, dis_style_immediate,
				    "0x%08x", word);
      (*info->fprintf_styled_func) (info->stream, dis_style_comment_start,
				    " ; %s", err_msg[ret]);
      break;
    case ERR_OK:
      user_friendly_fixup (&inst);
      print_aarch64_insn (pc, &inst, word, info, errors);
      break;
    default:
      abort ();
    }
}

/* Disallow mapping symbols ($x, $d etc) from
   being displayed in symbol relative addresses.  */

bool
aarch64_symbol_is_valid (asymbol * sym,
			 struct disassemble_info * info ATTRIBUTE_UNUSED)
{
  const char * name;

  if (sym == NULL)
    return false;

  name = bfd_asymbol_name (sym);

  return name
    && (name[0] != '$'
	|| (name[1] != 'x' && name[1] != 'd')
	|| (name[2] != '\0' && name[2] != '.'));
}

/* Print data bytes on INFO->STREAM.  */

static void
print_insn_data (bfd_vma pc ATTRIBUTE_UNUSED,
		 uint32_t word,
		 struct disassemble_info *info,
		 aarch64_operand_error *errors ATTRIBUTE_UNUSED)
{
  switch (info->bytes_per_chunk)
    {
    case 1:
      info->fprintf_styled_func (info->stream, dis_style_assembler_directive,
				 ".byte\t");
      info->fprintf_styled_func (info->stream, dis_style_immediate,
				 "0x%02x", word);
      break;
    case 2:
      info->fprintf_styled_func (info->stream, dis_style_assembler_directive,
				 ".short\t");
      info->fprintf_styled_func (info->stream, dis_style_immediate,
				 "0x%04x", word);
      break;
    case 4:
      info->fprintf_styled_func (info->stream, dis_style_assembler_directive,
				 ".word\t");
      info->fprintf_styled_func (info->stream, dis_style_immediate,
				 "0x%08x", word);
      break;
    default:
      abort ();
    }
}

/* Try to infer the code or data type from a symbol.
   Returns nonzero if *MAP_TYPE was set.  */

static int
get_sym_code_type (struct disassemble_info *info, int n,
		   enum map_type *map_type)
{
  asymbol * as;
  elf_symbol_type *es;
  unsigned int type;
  const char *name;

  /* If the symbol is in a different section, ignore it.  */
  if (info->section != NULL && info->section != info->symtab[n]->section)
    return false;

  if (n >= info->symtab_size)
    return false;

  as = info->symtab[n];
  if (bfd_asymbol_flavour (as) != bfd_target_elf_flavour)
    return false;
  es = (elf_symbol_type *) as;

  type = ELF_ST_TYPE (es->internal_elf_sym.st_info);

  /* If the symbol has function type then use that.  */
  if (type == STT_FUNC)
    {
      *map_type = MAP_INSN;
      return true;
    }

  /* Check for mapping symbols.  */
  name = bfd_asymbol_name(info->symtab[n]);
  if (name[0] == '$'
      && (name[1] == 'x' || name[1] == 'd')
      && (name[2] == '\0' || name[2] == '.'))
    {
      *map_type = (name[1] == 'x' ? MAP_INSN : MAP_DATA);
      return true;
    }

  return false;
}

/* Set the feature bits in arch_variant in order to get the correct disassembly
   for the chosen architecture variant.

   Currently we only restrict disassembly for Armv8-R and otherwise enable all
   non-R-profile features.  */
static void
select_aarch64_variant (unsigned mach)
{
  switch (mach)
    {
    case bfd_mach_aarch64_8R:
      arch_variant = AARCH64_ARCH_V8_R;
      break;
    default:
      arch_variant = AARCH64_ANY & ~(AARCH64_FEATURE_V8_R);
    }
}

/* Entry-point of the AArch64 disassembler.  */

int
print_insn_aarch64 (bfd_vma pc,
		    struct disassemble_info *info)
{
  bfd_byte	buffer[INSNLEN];
  int		status;
  void		(*printer) (bfd_vma, uint32_t, struct disassemble_info *,
			    aarch64_operand_error *);
  bool   found = false;
  unsigned int	size = 4;
  unsigned long	data;
  aarch64_operand_error errors;
  static bool set_features;

  if (info->disassembler_options)
    {
      set_default_aarch64_dis_options (info);

      parse_aarch64_dis_options (info->disassembler_options);

      /* To avoid repeated parsing of these options, we remove them here.  */
      info->disassembler_options = NULL;
    }

  if (!set_features)
    {
      select_aarch64_variant (info->mach);
      set_features = true;
    }

  /* Aarch64 instructions are always little-endian */
  info->endian_code = BFD_ENDIAN_LITTLE;

  /* Default to DATA.  A text section is required by the ABI to contain an
     INSN mapping symbol at the start.  A data section has no such
     requirement, hence if no mapping symbol is found the section must
     contain only data.  This however isn't very useful if the user has
     fully stripped the binaries.  If this is the case use the section
     attributes to determine the default.  If we have no section default to
     INSN as well, as we may be disassembling some raw bytes on a baremetal
     HEX file or similar.  */
  enum map_type type = MAP_DATA;
  if ((info->section && info->section->flags & SEC_CODE) || !info->section)
    type = MAP_INSN;

  /* First check the full symtab for a mapping symbol, even if there
     are no usable non-mapping symbols for this address.  */
  if (info->symtab_size != 0
      && bfd_asymbol_flavour (*info->symtab) == bfd_target_elf_flavour)
    {
      int last_sym = -1;
      bfd_vma addr, section_vma = 0;
      bool can_use_search_opt_p;
      int n;

      if (pc <= last_mapping_addr)
	last_mapping_sym = -1;

      /* Start scanning at the start of the function, or wherever
	 we finished last time.  */
      n = info->symtab_pos + 1;

      /* If the last stop offset is different from the current one it means we
	 are disassembling a different glob of bytes.  As such the optimization
	 would not be safe and we should start over.  */
      can_use_search_opt_p = last_mapping_sym >= 0
			     && info->stop_offset == last_stop_offset;

      if (n >= last_mapping_sym && can_use_search_opt_p)
	n = last_mapping_sym;

      /* Look down while we haven't passed the location being disassembled.
	 The reason for this is that there's no defined order between a symbol
	 and an mapping symbol that may be at the same address.  We may have to
	 look at least one position ahead.  */
      for (; n < info->symtab_size; n++)
	{
	  addr = bfd_asymbol_value (info->symtab[n]);
	  if (addr > pc)
	    break;
	  if (get_sym_code_type (info, n, &type))
	    {
	      last_sym = n;
	      found = true;
	    }
	}

      if (!found)
	{
	  n = info->symtab_pos;
	  if (n >= last_mapping_sym && can_use_search_opt_p)
	    n = last_mapping_sym;

	  /* No mapping symbol found at this address.  Look backwards
	     for a preceeding one, but don't go pass the section start
	     otherwise a data section with no mapping symbol can pick up
	     a text mapping symbol of a preceeding section.  The documentation
	     says section can be NULL, in which case we will seek up all the
	     way to the top.  */
	  if (info->section)
	    section_vma = info->section->vma;

	  for (; n >= 0; n--)
	    {
	      addr = bfd_asymbol_value (info->symtab[n]);
	      if (addr < section_vma)
		break;

	      if (get_sym_code_type (info, n, &type))
		{
		  last_sym = n;
		  found = true;
		  break;
		}
	    }
	}

      last_mapping_sym = last_sym;
      last_type = type;
      last_stop_offset = info->stop_offset;

      /* Look a little bit ahead to see if we should print out
	 less than four bytes of data.  If there's a symbol,
	 mapping or otherwise, after two bytes then don't
	 print more.  */
      if (last_type == MAP_DATA)
	{
	  size = 4 - (pc & 3);
	  for (n = last_sym + 1; n < info->symtab_size; n++)
	    {
	      addr = bfd_asymbol_value (info->symtab[n]);
	      if (addr > pc)
		{
		  if (addr - pc < size)
		    size = addr - pc;
		  break;
		}
	    }
	  /* If the next symbol is after three bytes, we need to
	     print only part of the data, so that we can use either
	     .byte or .short.  */
	  if (size == 3)
	    size = (pc & 1) ? 1 : 2;
	}
    }
  else
    last_type = type;

  /* PR 10263: Disassemble data if requested to do so by the user.  */
  if (last_type == MAP_DATA && ((info->flags & DISASSEMBLE_DATA) == 0))
    {
      /* size was set above.  */
      info->bytes_per_chunk = size;
      info->display_endian = info->endian;
      printer = print_insn_data;
    }
  else
    {
      info->bytes_per_chunk = size = INSNLEN;
      info->display_endian = info->endian_code;
      printer = print_insn_aarch64_word;
    }

  status = (*info->read_memory_func) (pc, buffer, size, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, pc, info);
      return -1;
    }

  data = bfd_get_bits (buffer, size * 8,
		       info->display_endian == BFD_ENDIAN_BIG);

  (*printer) (pc, data, info, &errors);

  return size;
}

void
print_aarch64_disassembler_options (FILE *stream)
{
  fprintf (stream, _("\n\
The following AARCH64 specific disassembler options are supported for use\n\
with the -M switch (multiple options should be separated by commas):\n"));

  fprintf (stream, _("\n\
  no-aliases         Don't print instruction aliases.\n"));

  fprintf (stream, _("\n\
  aliases            Do print instruction aliases.\n"));

  fprintf (stream, _("\n\
  no-notes         Don't print instruction notes.\n"));

  fprintf (stream, _("\n\
  notes            Do print instruction notes.\n"));

#ifdef DEBUG_AARCH64
  fprintf (stream, _("\n\
  debug_dump         Temp switch for debug trace.\n"));
#endif /* DEBUG_AARCH64 */

  fprintf (stream, _("\n"));
}
