/* tc-tilepro.c -- Assemble for a TILEPro chip.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "as.h"
#include "subsegs.h"

#include "elf/tilepro.h"
#include "opcode/tilepro.h"

#include "dwarf2dbg.h"
#include "dw2gencfi.h"

#include "safe-ctype.h"


/* Special registers. */
#define TREG_IDN0     57
#define TREG_IDN1     58
#define TREG_UDN0     59
#define TREG_UDN1     60
#define TREG_UDN2     61
#define TREG_UDN3     62
#define TREG_ZERO     63


/* Generic assembler global variables which must be defined by all
   targets.  */

/* Characters which always start a comment.  */
const char comment_chars[] = "#";

/* Characters which start a comment at the beginning of a line.  */
const char line_comment_chars[] = "#";

/* Characters which may be used to separate multiple commands on a
   single line.  */
const char line_separator_chars[] = ";";

/* Characters which are used to indicate an exponent in a floating
   point number.  */
const char EXP_CHARS[] = "eE";

/* Characters which mean that a number is a floating point constant,
   as in 0d1.0.  */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

const char *md_shortopts = "VQ:";

struct option md_longopts[] =
{
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

int
md_parse_option (int c, const char *arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
      /* -Qy, -Qn: SVR4 arguments controlling whether a .comment section
	 should be emitted or not.  FIXME: Not implemented.  */
    case 'Q':
      break;

      /* -V: SVR4 argument to print version ID.  */
    case 'V':
      print_version_id ();
      break;

    default:
      return 0;
    }

  return 1;
}

void
md_show_usage (FILE *stream)
{
  fprintf (stream, _("\
  -Q                      ignored\n\
  -V                      print assembler version number\n"));
}

/* Extra expression types.  */

#define O_lo16        O_md1
#define O_hi16        O_md2
#define O_ha16        O_md3
#define O_got         O_md4
#define O_got_lo16    O_md5
#define O_got_hi16    O_md6
#define O_got_ha16    O_md7
#define O_plt         O_md8
#define O_tls_gd      O_md9
#define O_tls_gd_lo16 O_md10
#define O_tls_gd_hi16 O_md11
#define O_tls_gd_ha16 O_md12
#define O_tls_ie      O_md13
#define O_tls_ie_lo16 O_md14
#define O_tls_ie_hi16 O_md15
#define O_tls_ie_ha16 O_md16
#define O_tls_le      O_md17
#define O_tls_le_lo16 O_md18
#define O_tls_le_hi16 O_md19
#define O_tls_le_ha16 O_md20
#define O_tls_gd_call O_md21
#define O_tls_gd_add  O_md22
#define O_tls_ie_load O_md23

static htab_t special_operator_hash;

/* Hash tables for instruction mnemonic lookup.  */
static htab_t op_hash;

/* Hash table for spr lookup.  */
static htab_t spr_hash;

/* True temporarily while parsing an SPR expression. This changes the
 * namespace to include SPR names.  */
static int parsing_spr;

/* Are we currently inside `{ ... }'?  */
static int inside_bundle;

struct tilepro_instruction
{
  const struct tilepro_opcode *opcode;
  tilepro_pipeline pipe;
  expressionS operand_values[TILEPRO_MAX_OPERANDS];
};

/* This keeps track of the current bundle being built up.  */
static struct tilepro_instruction
current_bundle[TILEPRO_MAX_INSTRUCTIONS_PER_BUNDLE];

/* Index in current_bundle for the next instruction to parse.  */
static int current_bundle_index;

/* Allow 'r63' in addition to 'zero', etc. Normally we disallow this as
   'zero' is not a real register, so using it accidentally would be a
   nasty bug. For other registers, such as 'sp', code using multiple names
   for the same physical register is excessively confusing.

   The '.require_canonical_reg_names' pseudo-op turns this error on,
   and the '.no_require_canonical_reg_names' pseudo-op turns this off.
   By default the error is on.  */
static int require_canonical_reg_names;

/* Allow bundles that do undefined or suspicious things like write
   two different values to the same register at the same time.

   The '.no_allow_suspicious_bundles' pseudo-op turns this error on,
   and the '.allow_suspicious_bundles' pseudo-op turns this off.  */
static int allow_suspicious_bundles;


/* A hash table of main processor registers, mapping each register name
   to its index.

   Furthermore, if the register number is greater than the number
   of registers for that processor, the user used an illegal alias
   for that register (e.g. r63 instead of zero), so we should generate
   a warning. The attempted register number can be found by clearing
   NONCANONICAL_REG_NAME_FLAG.  */
static htab_t main_reg_hash;


/* We cannot unambiguously store a 0 in a hash table and look it up,
   so we OR in this flag to every canonical register.  */
#define CANONICAL_REG_NAME_FLAG    0x1000

/* By default we disallow register aliases like r63, but we record
   them in the hash table in case the .no_require_canonical_reg_names
   directive is used. Noncanonical names have this value added to them.  */
#define NONCANONICAL_REG_NAME_FLAG 0x2000

/* Discards flags for register hash table entries and returns the
   reg number.  */
#define EXTRACT_REGNO(p) ((p) & 63)

/* This function is called once, at assembler startup time.  It should
   set up all the tables, etc., that the MD part of the assembler will
   need.  */
void
md_begin (void)
{
  const struct tilepro_opcode *op;
  int i;

  /* Guarantee text section is aligned.  */
  bfd_set_section_alignment (text_section,
                             TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES);

  require_canonical_reg_names = 1;
  allow_suspicious_bundles = 0;
  current_bundle_index = 0;
  inside_bundle = 0;

  /* Initialize special operator hash table.  */
  special_operator_hash = str_htab_create ();
#define INSERT_SPECIAL_OP(name)					\
  str_hash_insert (special_operator_hash, #name, (void *) O_##name, 0)

  INSERT_SPECIAL_OP(lo16);
  INSERT_SPECIAL_OP(hi16);
  INSERT_SPECIAL_OP(ha16);
  INSERT_SPECIAL_OP(got);
  INSERT_SPECIAL_OP(got_lo16);
  INSERT_SPECIAL_OP(got_hi16);
  INSERT_SPECIAL_OP(got_ha16);
  INSERT_SPECIAL_OP(plt);
  INSERT_SPECIAL_OP(tls_gd);
  INSERT_SPECIAL_OP(tls_gd_lo16);
  INSERT_SPECIAL_OP(tls_gd_hi16);
  INSERT_SPECIAL_OP(tls_gd_ha16);
  INSERT_SPECIAL_OP(tls_ie);
  INSERT_SPECIAL_OP(tls_ie_lo16);
  INSERT_SPECIAL_OP(tls_ie_hi16);
  INSERT_SPECIAL_OP(tls_ie_ha16);
  INSERT_SPECIAL_OP(tls_le);
  INSERT_SPECIAL_OP(tls_le_lo16);
  INSERT_SPECIAL_OP(tls_le_hi16);
  INSERT_SPECIAL_OP(tls_le_ha16);
  INSERT_SPECIAL_OP(tls_gd_call);
  INSERT_SPECIAL_OP(tls_gd_add);
  INSERT_SPECIAL_OP(tls_ie_load);
#undef INSERT_SPECIAL_OP

  /* Initialize op_hash hash table.  */
  op_hash = str_htab_create ();
  for (op = &tilepro_opcodes[0]; op->name != NULL; op++)
    if (str_hash_insert (op_hash, op->name, op, 0) != NULL)
      as_fatal (_("duplicate %s"), op->name);

  /* Initialize the spr hash table.  */
  parsing_spr = 0;
  spr_hash = str_htab_create ();
  for (i = 0; i < tilepro_num_sprs; i++)
    str_hash_insert (spr_hash, tilepro_sprs[i].name, &tilepro_sprs[i], 0);

  /* Set up the main_reg_hash table. We use this instead of
   * creating a symbol in the register section to avoid ambiguities
   * with labels that have the same names as registers.  */
  main_reg_hash = str_htab_create ();
  for (i = 0; i < TILEPRO_NUM_REGISTERS; i++)
    {
      char buf[64];

      str_hash_insert (main_reg_hash, tilepro_register_names[i],
		       (void *) (long) (i | CANONICAL_REG_NAME_FLAG), 0);

      /* See if we should insert a noncanonical alias, like r63.  */
      sprintf (buf, "r%d", i);
      if (strcmp (buf, tilepro_register_names[i]) != 0)
	str_hash_insert (main_reg_hash, xstrdup (buf),
			 (void *) (long) (i | NONCANONICAL_REG_NAME_FLAG), 0);
    }

  /* Insert obsolete backwards-compatibility register names.  */
  str_hash_insert (main_reg_hash, "io0",
		   (void *) (long) (TREG_IDN0 | CANONICAL_REG_NAME_FLAG), 0);
  str_hash_insert (main_reg_hash, "io1",
		   (void *) (long) (TREG_IDN1 | CANONICAL_REG_NAME_FLAG), 0);
  str_hash_insert (main_reg_hash, "us0",
		   (void *) (long) (TREG_UDN0 | CANONICAL_REG_NAME_FLAG), 0);
  str_hash_insert (main_reg_hash, "us1",
		   (void *) (long) (TREG_UDN1 | CANONICAL_REG_NAME_FLAG), 0);
  str_hash_insert (main_reg_hash, "us2",
		   (void *) (long) (TREG_UDN2 | CANONICAL_REG_NAME_FLAG), 0);
  str_hash_insert (main_reg_hash, "us3",
		   (void *) (long) (TREG_UDN3 | CANONICAL_REG_NAME_FLAG), 0);

}


#define BUNDLE_TEMPLATE_MASK(p0, p1, p2) \
  ((p0) | ((p1) << 8) | ((p2) << 16))
#define BUNDLE_TEMPLATE(p0, p1, p2) \
  { { (p0), (p1), (p2) }, \
     BUNDLE_TEMPLATE_MASK(1 << (p0), 1 << (p1), (1 << (p2))) \
  }

#define NO_PIPELINE TILEPRO_NUM_PIPELINE_ENCODINGS

struct bundle_template
{
  tilepro_pipeline pipe[TILEPRO_MAX_INSTRUCTIONS_PER_BUNDLE];
  unsigned int pipe_mask;
};

static const struct bundle_template bundle_templates[] =
{
  /* In Y format we must always have something in Y2, since it has
   * no fnop, so this conveys that Y2 must always be used.  */
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y0, TILEPRO_PIPELINE_Y2, NO_PIPELINE),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y1, TILEPRO_PIPELINE_Y2, NO_PIPELINE),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y2, TILEPRO_PIPELINE_Y0, NO_PIPELINE),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y2, TILEPRO_PIPELINE_Y1, NO_PIPELINE),

  /* Y format has three instructions.  */
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y0, TILEPRO_PIPELINE_Y1, TILEPRO_PIPELINE_Y2),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y0, TILEPRO_PIPELINE_Y2, TILEPRO_PIPELINE_Y1),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y1, TILEPRO_PIPELINE_Y0, TILEPRO_PIPELINE_Y2),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y1, TILEPRO_PIPELINE_Y2, TILEPRO_PIPELINE_Y0),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y2, TILEPRO_PIPELINE_Y0, TILEPRO_PIPELINE_Y1),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_Y2, TILEPRO_PIPELINE_Y1, TILEPRO_PIPELINE_Y0),

  /* X format has only two instructions.  */
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_X0, TILEPRO_PIPELINE_X1, NO_PIPELINE),
  BUNDLE_TEMPLATE(TILEPRO_PIPELINE_X1, TILEPRO_PIPELINE_X0, NO_PIPELINE)
};


static void
prepend_nop_to_bundle (tilepro_mnemonic mnemonic)
{
  memmove (&current_bundle[1], &current_bundle[0],
	   current_bundle_index * sizeof current_bundle[0]);
  current_bundle[0].opcode = &tilepro_opcodes[mnemonic];
  ++current_bundle_index;
}


static tilepro_bundle_bits
insert_operand (tilepro_bundle_bits bits,
                const struct tilepro_operand *operand,
                int operand_value,
                const char *file,
                unsigned lineno)
{
  /* Range-check the immediate.  */
  int num_bits = operand->num_bits;

  operand_value >>= operand->rightshift;

  if (bfd_check_overflow (operand->is_signed
                          ? complain_overflow_signed
                          : complain_overflow_unsigned,
                          num_bits,
                          0,
                          bfd_arch_bits_per_address (stdoutput),
                          operand_value)
      != bfd_reloc_ok)
    {
      offsetT min, max;
      if (operand->is_signed)
	{
	  min = -(1 << (num_bits - 1));
	  max = (1 << (num_bits - 1)) - 1;
	}
      else
	{
	  min = 0;
	  max = (1 << num_bits) - 1;
	}
      as_bad_value_out_of_range (_("operand"), operand_value, min, max,
				 file, lineno);
    }

  /* Write out the bits for the immediate.  */
  return bits | operand->insert (operand_value);
}


static int
apply_special_operator (operatorT op, int num)
{
  switch (op)
    {
    case O_lo16:
      return (signed short)num;

    case O_hi16:
      return (signed short)(num >> 16);

    case O_ha16:
      return (signed short)((num + 0x8000) >> 16);

    default:
      abort ();
    }
}


static tilepro_bundle_bits
emit_tilepro_instruction (tilepro_bundle_bits bits,
			  int num_operands,
			  const unsigned char *operands,
			  expressionS *operand_values,
			  char *bundle_start)
{
  int i;

  for (i = 0; i < num_operands; i++)
    {
      const struct tilepro_operand *operand =
	&tilepro_operands[operands[i]];
      expressionS *operand_exp = &operand_values[i];
      int is_pc_relative = operand->is_pc_relative;

      if (operand_exp->X_op == O_register
	  || (operand_exp->X_op == O_constant && !is_pc_relative))
	{
	  /* We know what the bits are right now, so insert them.  */
	  bits = insert_operand (bits, operand, operand_exp->X_add_number,
				 NULL, 0);
	}
      else
	{
	  bfd_reloc_code_real_type reloc = operand->default_reloc;
	  expressionS subexp;
	  int die = 0, use_subexp = 0, require_symbol = 0;
	  fixS *fixP;

	  /* Take an expression like hi16(x) and turn it into x with
	     a different reloc type.  */
	  switch (operand_exp->X_op)
	    {
#define HANDLE_OP16(suffix)					\
	      switch (reloc)					\
		{                                               \
		case BFD_RELOC_TILEPRO_IMM16_X0:                \
		  reloc = BFD_RELOC_TILEPRO_IMM16_X0_##suffix;  \
		  break;                                        \
		case BFD_RELOC_TILEPRO_IMM16_X1:                \
		  reloc = BFD_RELOC_TILEPRO_IMM16_X1_##suffix;  \
		  break;                                        \
		default:                                        \
		  die = 1;                                      \
		  break;                                        \
		}                                               \
	      use_subexp = 1

	    case O_lo16:
	      HANDLE_OP16 (LO);
	      break;

	    case O_hi16:
	      HANDLE_OP16 (HI);
	      break;

	    case O_ha16:
	      HANDLE_OP16 (HA);
	      break;

	    case O_got:
	      HANDLE_OP16 (GOT);
	      require_symbol = 1;
	      break;

	    case O_got_lo16:
	      HANDLE_OP16 (GOT_LO);
	      require_symbol = 1;
	      break;

	    case O_got_hi16:
	      HANDLE_OP16 (GOT_HI);
	      require_symbol = 1;
	      break;

	    case O_got_ha16:
	      HANDLE_OP16 (GOT_HA);
	      require_symbol = 1;
	      break;

	    case O_tls_gd:
	      HANDLE_OP16 (TLS_GD);
	      require_symbol = 1;
	      break;

	    case O_tls_gd_lo16:
	      HANDLE_OP16 (TLS_GD_LO);
	      require_symbol = 1;
	      break;

	    case O_tls_gd_hi16:
	      HANDLE_OP16 (TLS_GD_HI);
	      require_symbol = 1;
	      break;

	    case O_tls_gd_ha16:
	      HANDLE_OP16 (TLS_GD_HA);
	      require_symbol = 1;
	      break;

	    case O_tls_ie:
	      HANDLE_OP16 (TLS_IE);
	      require_symbol = 1;
	      break;

	    case O_tls_ie_lo16:
	      HANDLE_OP16 (TLS_IE_LO);
	      require_symbol = 1;
	      break;

	    case O_tls_ie_hi16:
	      HANDLE_OP16 (TLS_IE_HI);
	      require_symbol = 1;
	      break;

	    case O_tls_ie_ha16:
	      HANDLE_OP16 (TLS_IE_HA);
	      require_symbol = 1;
	      break;

	    case O_tls_le:
	      HANDLE_OP16 (TLS_LE);
	      require_symbol = 1;
	      break;

	    case O_tls_le_lo16:
	      HANDLE_OP16 (TLS_LE_LO);
	      require_symbol = 1;
	      break;

	    case O_tls_le_hi16:
	      HANDLE_OP16 (TLS_LE_HI);
	      require_symbol = 1;
	      break;

	    case O_tls_le_ha16:
	      HANDLE_OP16 (TLS_LE_HA);
	      require_symbol = 1;
	      break;

#undef HANDLE_OP16

	    case O_plt:
	      switch (reloc)
		{
		case BFD_RELOC_TILEPRO_JOFFLONG_X1:
		  reloc = BFD_RELOC_TILEPRO_JOFFLONG_X1_PLT;
		  break;
		default:
		  die = 1;
		  break;
		}
	      use_subexp = 1;
	      require_symbol = 1;
	      break;

	    case O_tls_gd_call:
	      switch (reloc)
		{
		case BFD_RELOC_TILEPRO_JOFFLONG_X1:
		  reloc = BFD_RELOC_TILEPRO_TLS_GD_CALL;
		  break;
		default:
		  die = 1;
		  break;
		}
	      use_subexp = 1;
	      require_symbol = 1;
	      break;

	    case O_tls_gd_add:
	      switch (reloc)
		{
		case BFD_RELOC_TILEPRO_IMM8_X0:
		  reloc = BFD_RELOC_TILEPRO_IMM8_X0_TLS_GD_ADD;
		  break;
		case BFD_RELOC_TILEPRO_IMM8_X1:
		  reloc = BFD_RELOC_TILEPRO_IMM8_X1_TLS_GD_ADD;
		  break;
		case BFD_RELOC_TILEPRO_IMM8_Y0:
		  reloc = BFD_RELOC_TILEPRO_IMM8_Y0_TLS_GD_ADD;
		  break;
		case BFD_RELOC_TILEPRO_IMM8_Y1:
		  reloc = BFD_RELOC_TILEPRO_IMM8_Y1_TLS_GD_ADD;
		  break;
		default:
		  die = 1;
		  break;
		}
	      use_subexp = 1;
	      require_symbol = 1;
	      break;

	    case O_tls_ie_load:
	      switch (reloc)
		{
		case BFD_RELOC_TILEPRO_IMM8_X1:
		  reloc = BFD_RELOC_TILEPRO_TLS_IE_LOAD;
		  break;
		default:
		  die = 1;
		  break;
		}
	      use_subexp = 1;
	      require_symbol = 1;
	      break;

	    default:
	      /* Do nothing.  */
	      break;
	    }

	  if (die)
	    {
	      as_bad (_("Invalid operator for operand."));
	    }
	  else if (use_subexp)
	    {
	      expressionS *sval = NULL;
	      /* Now that we've changed the reloc, change ha16(x) into x,
		 etc.  */

	      if (symbol_symbolS (operand_exp->X_add_symbol))
		sval = symbol_get_value_expression (operand_exp->X_add_symbol);
	      if (sval && sval->X_md)
		{
		  /* HACK: We used X_md to mark this symbol as a fake wrapper
		     around a real expression. To unwrap it, we just grab its
		     value here.  */
		  operand_exp = sval;

		  if (require_symbol)
		    {
		      /* Look at the expression, and reject it if it's not a
			 plain symbol.  */
		      if (operand_exp->X_op != O_symbol
			  || operand_exp->X_add_number != 0)
			as_bad (_("Operator may only be applied to symbols."));
		    }
		}
	      else
		{
		  /* The value of this expression is an actual symbol, so
		     turn that into an expression.  */
		  memset (&subexp, 0, sizeof subexp);
		  subexp.X_op = O_symbol;
		  subexp.X_add_symbol = operand_exp->X_add_symbol;
		  operand_exp = &subexp;
		}
	    }

	  /* Create a fixup to handle this later. */
	  fixP = fix_new_exp (frag_now,
			      bundle_start - frag_now->fr_literal,
			      (operand->num_bits + 7) >> 3,
			      operand_exp,
			      is_pc_relative,
			      reloc);
	  fixP->tc_fix_data = operand;

	  /* Don't do overflow checking if we are applying a function like
	     ha16.  */
	  fixP->fx_no_overflow |= use_subexp;
	}
    }
  return bits;
}


/* Detects and complains if two instructions in current_bundle write
   to the same register, either implicitly or explicitly, or if a
   read-only register is written.  */
static void
check_illegal_reg_writes (void)
{
  uint64_t all_regs_written = 0;
  int j;

  for (j = 0; j < current_bundle_index; j++)
    {
      const struct tilepro_instruction *instr = &current_bundle[j];
      int k;
      uint64_t regs =
	(uint64_t) 1 << instr->opcode->implicitly_written_register;
      uint64_t conflict;

      for (k = 0; k < instr->opcode->num_operands; k++)
	{
	  const struct tilepro_operand *operand =
	    &tilepro_operands[instr->opcode->operands[instr->pipe][k]];

	  if (operand->is_dest_reg)
	    {
	      int regno = instr->operand_values[k].X_add_number;
	      uint64_t mask = (uint64_t) 1 << regno;

	      if ((mask & (  ((uint64_t) 1 << TREG_IDN1)
			   | ((uint64_t) 1 << TREG_UDN1)
			   | ((uint64_t) 1 << TREG_UDN2)
			   | ((uint64_t) 1 << TREG_UDN3))) != 0
		  && !allow_suspicious_bundles)
		{
		  as_bad (_("Writes to register '%s' are not allowed."),
			  tilepro_register_names[regno]);
		}

	      regs |= mask;
	    }
	}

      /* Writing to the zero register doesn't count.  */
      regs &= ~((uint64_t) 1 << TREG_ZERO);

      conflict = all_regs_written & regs;
      if (conflict != 0 && !allow_suspicious_bundles)
	{
	  /* Find which register caused the conflict.  */
	  const char *conflicting_reg_name = "???";
	  int i;

	  for (i = 0; i < TILEPRO_NUM_REGISTERS; i++)
	    {
	      if (((conflict >> i) & 1) != 0)
		{
		  conflicting_reg_name = tilepro_register_names[i];
		  break;
		}
	    }

	  as_bad (_("Two instructions in the same bundle both write "
		    "to register %s, which is not allowed."),
		  conflicting_reg_name);
	}

      all_regs_written |= regs;
    }
}


static void
tilepro_flush_bundle (void)
{
  unsigned i;
  int j, addr_mod;
  unsigned compatible_pipes;
  const struct bundle_template *match;
  char *f;

  inside_bundle = 0;

  switch (current_bundle_index)
    {
    case 0:
      /* No instructions.  */
      return;
    case 1:
      if (current_bundle[0].opcode->can_bundle)
	{
	  /* Simplify later logic by adding an explicit fnop.  */
	  prepend_nop_to_bundle (TILEPRO_OPC_FNOP);
	}
      else
	{
	  /* This instruction cannot be bundled with anything else.
	     Prepend an explicit 'nop', rather than an 'fnop', because
	     fnops can be replaced by later binary-processing tools
	     while nops cannot.  */
	  prepend_nop_to_bundle (TILEPRO_OPC_NOP);
	}
      break;
    default:
      if (!allow_suspicious_bundles)
	{
	  /* Make sure all instructions can be bundled with other
	     instructions.  */
	  const struct tilepro_opcode *cannot_bundle = NULL;
	  bool seen_non_nop = false;

	  for (j = 0; j < current_bundle_index; j++)
	    {
	      const struct tilepro_opcode *op = current_bundle[j].opcode;

	      if (!op->can_bundle && cannot_bundle == NULL)
		cannot_bundle = op;
	      else if (op->mnemonic != TILEPRO_OPC_NOP
		       && op->mnemonic != TILEPRO_OPC_INFO
		       && op->mnemonic != TILEPRO_OPC_INFOL)
		seen_non_nop = true;
	    }

	  if (cannot_bundle != NULL && seen_non_nop)
	    {
	      current_bundle_index = 0;
	      as_bad (_("'%s' may not be bundled with other instructions."),
		      cannot_bundle->name);
	      return;
	    }
	}
      break;
    }

  compatible_pipes =
    BUNDLE_TEMPLATE_MASK(current_bundle[0].opcode->pipes,
                         current_bundle[1].opcode->pipes,
                         (current_bundle_index == 3
                          ? current_bundle[2].opcode->pipes
                          : (1 << NO_PIPELINE)));

  /* Find a template that works, if any.  */
  match = NULL;
  for (i = 0; i < sizeof bundle_templates / sizeof bundle_templates[0]; i++)
    {
      const struct bundle_template *b = &bundle_templates[i];
      if ((b->pipe_mask & compatible_pipes) == b->pipe_mask)
	{
	  match = b;
	  break;
	}
    }

  if (match == NULL)
    {
      current_bundle_index = 0;
      as_bad (_("Invalid combination of instructions for bundle."));
      return;
    }

  /* If the section seems to have no alignment set yet, go ahead and
     make it large enough to hold code.  */
  if (bfd_section_alignment (now_seg) == 0)
    bfd_set_section_alignment (now_seg,
                               TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES);

  for (j = 0; j < current_bundle_index; j++)
    current_bundle[j].pipe = match->pipe[j];

  if (current_bundle_index == 2 && !tilepro_is_x_pipeline(match->pipe[0]))
    {
      /* We are in Y mode with only two instructions, so add an FNOP.  */
      prepend_nop_to_bundle (TILEPRO_OPC_FNOP);

      /* Figure out what pipe the fnop must be in via arithmetic.
       * p0 + p1 + p2 must sum to the sum of TILEPRO_PIPELINE_Y[012].  */
      current_bundle[0].pipe =
	(tilepro_pipeline)((TILEPRO_PIPELINE_Y0
			    + TILEPRO_PIPELINE_Y1
			    + TILEPRO_PIPELINE_Y2) -
			   (current_bundle[1].pipe + current_bundle[2].pipe));
    }

  check_illegal_reg_writes ();

  f = frag_more (TILEPRO_BUNDLE_SIZE_IN_BYTES);

  /* Check to see if this bundle is at an offset that is a multiple of 8-bytes
     from the start of the frag.  */
  addr_mod = frag_now_fix () & (TILEPRO_BUNDLE_ALIGNMENT_IN_BYTES - 1);
  if (frag_now->has_code && frag_now->insn_addr != addr_mod)
    as_bad (_("instruction address is not a multiple of 8"));
  frag_now->insn_addr = addr_mod;
  frag_now->has_code = 1;

  tilepro_bundle_bits bits = 0;
  for (j = 0; j < current_bundle_index; j++)
    {
      struct tilepro_instruction *instr = &current_bundle[j];
      tilepro_pipeline pipeline = instr->pipe;
      const struct tilepro_opcode *opcode = instr->opcode;

      bits |= emit_tilepro_instruction (opcode->fixed_bit_values[pipeline],
					opcode->num_operands,
					&opcode->operands[pipeline][0],
					instr->operand_values,
					f);
    }

  number_to_chars_littleendian (f, (unsigned int)bits, 4);
  number_to_chars_littleendian (f + 4, (unsigned int)(bits >> 32), 4);
  current_bundle_index = 0;

  /* Emit DWARF2 debugging information.  */
  dwarf2_emit_insn (TILEPRO_BUNDLE_SIZE_IN_BYTES);
}


/* Extend the expression parser to handle hi16(label), etc.
   as well as SPR names when in the context of parsing an SPR.  */
int
tilepro_parse_name (char *name, expressionS *e, char *nextcharP)
{
  operatorT op = O_illegal;

  if (parsing_spr)
    {
      void *val = str_hash_find (spr_hash, name);
      if (val == NULL)
	return 0;

      memset (e, 0, sizeof *e);
      e->X_op = O_constant;
      e->X_add_number = ((const struct tilepro_spr *)val)->number;
      return 1;
    }

  if (*nextcharP != '(')
    {
      /* hi16, etc. not followed by a paren is just a label with that
	 name.  */
      return 0;
    }
  else
    {
      /* Look up the operator in our table.  */
      void *val = str_hash_find (special_operator_hash, name);
      if (val == 0)
	return 0;
      op = (operatorT)(long)val;
    }

  /* Restore old '(' and skip it.  */
  *input_line_pointer = '(';
  ++input_line_pointer;

  expression (e);

  if (*input_line_pointer != ')')
    {
      as_bad (_("Missing ')'"));
      *nextcharP = *input_line_pointer;
      return 0;
    }
  /* Skip ')'.  */
  ++input_line_pointer;

  if (e->X_op == O_register || e->X_op == O_absent)
    {
      as_bad (_("Invalid expression."));
      e->X_op = O_constant;
      e->X_add_number = 0;
    }
  else
    {
      /* Wrap subexpression with a unary operator.  */
      symbolS *sym = make_expr_symbol (e);

      if (sym != e->X_add_symbol)
	{
	  /* HACK: mark this symbol as a temporary wrapper around a proper
	     expression, so we can unwrap it later once we have communicated
	     the relocation type.  */
	  symbol_get_value_expression (sym)->X_md = 1;
	}

      memset (e, 0, sizeof *e);
      e->X_op = op;
      e->X_add_symbol = sym;
      e->X_add_number = 0;
    }

  *nextcharP = *input_line_pointer;
  return 1;
}


/* Parses an expression which must be a register name.  */

static void
parse_reg_expression (expressionS* expression)
{
  /* Zero everything to make sure we don't miss any flags.  */
  memset (expression, 0, sizeof *expression);

  char *regname;
  char terminating_char = get_symbol_name (&regname);

  void* pval = str_hash_find (main_reg_hash, regname);

  if (pval == NULL)
    as_bad (_("Expected register, got '%s'."), regname);

  int regno_and_flags = (int)(size_t)pval;
  int regno = EXTRACT_REGNO(regno_and_flags);

  if ((regno_and_flags & NONCANONICAL_REG_NAME_FLAG)
      && require_canonical_reg_names)
    as_warn (_("Found use of non-canonical register name %s; "
	       "use %s instead."),
	       regname, tilepro_register_names[regno]);

  /* Restore the old character following the register name.  */
  (void) restore_line_pointer (terminating_char);

  /* Fill in the expression fields to indicate it's a register.  */
  expression->X_op = O_register;
  expression->X_add_number = regno;
}


/* Parses and type-checks comma-separated operands in input_line_pointer.  */
static void
parse_operands (const char *opcode_name,
                const unsigned char *operands,
                int num_operands,
                expressionS *operand_values)
{
  int i;

  memset (operand_values, 0, num_operands * sizeof operand_values[0]);

  SKIP_WHITESPACE ();
  for (i = 0; i < num_operands; i++)
    {
      tilepro_operand_type type = tilepro_operands[operands[i]].type;

      SKIP_WHITESPACE ();

      if (type == TILEPRO_OP_TYPE_REGISTER)
	{
	  parse_reg_expression (&operand_values[i]);
	}
      else if (*input_line_pointer == '}')
	{
	  operand_values[i].X_op = O_absent;
	}
      else if (type == TILEPRO_OP_TYPE_SPR)
	{
	  /* Modify the expression parser to add SPRs to the namespace.  */
	  parsing_spr = 1;
	  expression (&operand_values[i]);
	  parsing_spr = 0;
	}
      else
	{
	  expression (&operand_values[i]);
	}

      SKIP_WHITESPACE ();

      if (i + 1 < num_operands)
	{
	  int separator = (unsigned char)*input_line_pointer++;

	  if (is_end_of_line[separator] || (separator == '}'))
	    {
	      as_bad (_("Too few operands to '%s'."), opcode_name);
	      return;
	    }
	  else if (separator != ',')
	    {
	      as_bad (_("Unexpected character '%c' after operand %d to %s."),
		      (char)separator, i + 1, opcode_name);
	      return;
	    }
	}

      /* Arbitrarily use the first valid pipe to get the operand type,
	 since they are all the same.  */
      switch (tilepro_operands[operands[i]].type)
	{
	case TILEPRO_OP_TYPE_REGISTER:
	  /* Handled in parse_reg_expression already.  */
	  break;
	case TILEPRO_OP_TYPE_SPR:
	  /* Fall through  */
	case TILEPRO_OP_TYPE_IMMEDIATE:
	  /* Fall through  */
	case TILEPRO_OP_TYPE_ADDRESS:
	  if (   operand_values[i].X_op == O_register
	      || operand_values[i].X_op == O_illegal
	      || operand_values[i].X_op == O_absent)
	    as_bad (_("Expected immediate expression"));
	  break;
	default:
	  abort ();
	}
    }

  if (!is_end_of_line[(unsigned char)*input_line_pointer])
    {
      switch (*input_line_pointer)
	{
	case '}':
	  if (!inside_bundle)
	    as_bad (_("Found '}' when not bundling."));
	  ++input_line_pointer;
	  inside_bundle = 0;
	  demand_empty_rest_of_line ();
	  break;

	case ',':
	  as_bad (_("Too many operands"));
	  break;

	default:
	  /* Use default error for unrecognized garbage.  */
	  demand_empty_rest_of_line ();
	  break;
	}
    }
}


/* This is the guts of the machine-dependent assembler.  STR points to a
   machine dependent instruction.  This function is supposed to emit
   the frags/bytes it assembles to.  */
void
md_assemble (char *str)
{
  char old_char;
  size_t opname_len;
  char *old_input_line_pointer;
  const struct tilepro_opcode *op;
  int first_pipe;

  /* Split off the opcode and look it up.  */
  opname_len = strcspn (str, " {}");
  old_char = str[opname_len];
  str[opname_len] = '\0';

  op = str_hash_find (op_hash, str);
  str[opname_len] = old_char;
  if (op == NULL)
    {
      as_bad (_("Unknown opcode `%.*s'."), (int)opname_len, str);
      return;
    }

  /* Prepare to parse the operands.  */
  old_input_line_pointer = input_line_pointer;
  input_line_pointer = str + opname_len;
  SKIP_WHITESPACE ();

  if (current_bundle_index == TILEPRO_MAX_INSTRUCTIONS_PER_BUNDLE)
    {
      as_bad (_("Too many instructions for bundle."));
      tilepro_flush_bundle ();
    }

  /* Make sure we have room for the upcoming bundle before we
     create any fixups. Otherwise if we have to switch to a new
     frag the fixup dot_value fields will be wrong.  */
  frag_grow (TILEPRO_BUNDLE_SIZE_IN_BYTES);

  /* Find a valid pipe for this opcode. */
  for (first_pipe = 0; (op->pipes & (1 << first_pipe)) == 0; first_pipe++)
    ;

  /* Call the function that assembles this instruction.  */
  current_bundle[current_bundle_index].opcode = op;
  parse_operands (op->name,
                  &op->operands[first_pipe][0],
                  op->num_operands,
                  current_bundle[current_bundle_index].operand_values);
  ++current_bundle_index;

  /* Restore the saved value of input_line_pointer.  */
  input_line_pointer = old_input_line_pointer;

  /* If we weren't inside curly braces, go ahead and emit
     this lone instruction as a bundle right now.  */
  if (!inside_bundle)
    tilepro_flush_bundle ();
}

static void
s_require_canonical_reg_names (int require)
{
  demand_empty_rest_of_line ();
  require_canonical_reg_names = require;
}

static void
s_allow_suspicious_bundles (int allow)
{
  demand_empty_rest_of_line ();
  allow_suspicious_bundles = allow;
}

const pseudo_typeS md_pseudo_table[] =
{
  {"align", s_align_bytes, 0},	/* Defaulting is invalid (0).  */
  {"word", cons, 4},
  {"require_canonical_reg_names", s_require_canonical_reg_names, 1 },
  {"no_require_canonical_reg_names", s_require_canonical_reg_names, 0 },
  {"allow_suspicious_bundles", s_allow_suspicious_bundles, 1 },
  {"no_allow_suspicious_bundles", s_allow_suspicious_bundles, 0 },
  { NULL, 0, 0 }
};

/* Turn the string pointed to by litP into a floating point constant
   of type TYPE, and emit the appropriate bytes.  The number of
   LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  LITTLENUM_TYPE *wordP;
  char *t;

  switch (type)
    {
    case 'f':
    case 'F':
      prec = 2;
      break;

    case 'd':
    case 'D':
      prec = 4;
      break;

    default:
      *sizeP = 0;
      return _("Bad call to md_atof ()");
    }
  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizeP = prec * sizeof (LITTLENUM_TYPE);
  /* This loops outputs the LITTLENUMs in REVERSE order; in accord with
     the bigendian 386.  */
  for (wordP = words + prec - 1; prec--;)
    {
      md_number_to_chars (litP, (valueT) (*wordP--), sizeof (LITTLENUM_TYPE));
      litP += sizeof (LITTLENUM_TYPE);
    }
  return 0;
}


/* We have no need to default values of symbols.  */

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}


void
tilepro_cons_fix_new (fragS *frag,
		      int where,
		      int nbytes,
		      expressionS *exp)
{
  expressionS subexp;
  bfd_reloc_code_real_type reloc = BFD_RELOC_NONE;
  int no_overflow = 0;
  fixS *fixP;

  /* See if it's one of our special functions.  */
  switch (exp->X_op)
    {
    case O_lo16:
      reloc = BFD_RELOC_LO16;
      no_overflow = 1;
      break;
    case O_hi16:
      reloc = BFD_RELOC_HI16;
      no_overflow = 1;
      break;
    case O_ha16:
      reloc = BFD_RELOC_HI16_S;
      no_overflow = 1;
      break;

    default:
      /* Do nothing.  */
      break;
    }

  if (reloc != BFD_RELOC_NONE)
    {
      if (nbytes != 2)
	{
	  as_bad (_("This operator only produces two byte values."));
	  nbytes = 2;
	}

      memset (&subexp, 0, sizeof subexp);
      subexp.X_op = O_symbol;
      subexp.X_add_symbol = exp->X_add_symbol;
      exp = &subexp;
    }
  else
    {
      switch (nbytes)
	{
	case 1:
	  reloc = BFD_RELOC_8;
	  break;
	case 2:
	  reloc = BFD_RELOC_16;
	  break;
	case 4:
	  reloc = BFD_RELOC_32;
	  break;
	case 8:
	  reloc = BFD_RELOC_64;
	  break;
	default:
	  as_bad (_("unsupported BFD relocation size %d"), nbytes);
	  reloc = BFD_RELOC_32;
	  break;
	}
    }

  fixP = fix_new_exp (frag, where, nbytes, exp, 0, reloc);
  fixP->tc_fix_data = NULL;
  fixP->fx_no_overflow |= no_overflow;
}


void
md_apply_fix (fixS *fixP, valueT * valP, segT seg ATTRIBUTE_UNUSED)
{
  const struct tilepro_operand *operand;
  valueT value = *valP;
  char *p;

  /* Leave these for the linker.  */
  if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
      || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    return;

  if (fixP->fx_subsy != (symbolS *) NULL)
    {
      /* We can't actually support subtracting a symbol.  */
      as_bad_subtract (fixP);
    }

  /* Correct relocation types for pc-relativeness.  */
  switch (fixP->fx_r_type)
    {
#define FIX_PCREL(rtype)                        \
      case rtype:				\
	if (fixP->fx_pcrel)			\
	  fixP->fx_r_type = rtype##_PCREL;	\
      break;					\
                                                \
    case rtype##_PCREL:				\
      if (!fixP->fx_pcrel)			\
	fixP->fx_r_type = rtype;		\
      break

      FIX_PCREL (BFD_RELOC_8);
      FIX_PCREL (BFD_RELOC_16);
      FIX_PCREL (BFD_RELOC_32);
      FIX_PCREL (BFD_RELOC_TILEPRO_IMM16_X0);
      FIX_PCREL (BFD_RELOC_TILEPRO_IMM16_X1);
      FIX_PCREL (BFD_RELOC_TILEPRO_IMM16_X0_LO);
      FIX_PCREL (BFD_RELOC_TILEPRO_IMM16_X1_LO);
      FIX_PCREL (BFD_RELOC_TILEPRO_IMM16_X0_HI);
      FIX_PCREL (BFD_RELOC_TILEPRO_IMM16_X1_HI);
      FIX_PCREL (BFD_RELOC_TILEPRO_IMM16_X0_HA);
      FIX_PCREL (BFD_RELOC_TILEPRO_IMM16_X1_HA);

#undef FIX_PCREL

    default:
      /* Do nothing */
      break;
    }

  if (fixP->fx_addsy != NULL)
    {
#ifdef OBJ_ELF
      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_TILEPRO_IMM8_X0_TLS_GD_ADD:
	case BFD_RELOC_TILEPRO_IMM8_X1_TLS_GD_ADD:
	case BFD_RELOC_TILEPRO_IMM8_Y0_TLS_GD_ADD:
	case BFD_RELOC_TILEPRO_IMM8_Y1_TLS_GD_ADD:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_LO:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_LO:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_HI:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_HI:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_HA:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_HA:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_LO:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_LO:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_HI:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_HI:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_HA:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_HA:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_LO:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_LO:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_HI:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_HI:
	case BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_HA:
	case BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_HA:
	case BFD_RELOC_TILEPRO_TLS_GD_CALL:
	case BFD_RELOC_TILEPRO_TLS_IE_LOAD:
	case BFD_RELOC_TILEPRO_TLS_DTPMOD32:
	case BFD_RELOC_TILEPRO_TLS_DTPOFF32:
	case BFD_RELOC_TILEPRO_TLS_TPOFF32:
	  S_SET_THREAD_LOCAL (fixP->fx_addsy);
	  break;

	default:
	  /* Do nothing */
	  break;
	}
#endif
      return;
    }

  /* Apply lo16, hi16, ha16, etc. munging. */
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_LO16:
    case BFD_RELOC_TILEPRO_IMM16_X0_LO:
    case BFD_RELOC_TILEPRO_IMM16_X1_LO:
    case BFD_RELOC_TILEPRO_IMM16_X0_LO_PCREL:
    case BFD_RELOC_TILEPRO_IMM16_X1_LO_PCREL:
      *valP = value = apply_special_operator (O_lo16, value);
      break;

    case BFD_RELOC_HI16:
    case BFD_RELOC_TILEPRO_IMM16_X0_HI:
    case BFD_RELOC_TILEPRO_IMM16_X1_HI:
    case BFD_RELOC_TILEPRO_IMM16_X0_HI_PCREL:
    case BFD_RELOC_TILEPRO_IMM16_X1_HI_PCREL:
      *valP = value = apply_special_operator (O_hi16, value);
      break;

    case BFD_RELOC_HI16_S:
    case BFD_RELOC_TILEPRO_IMM16_X0_HA:
    case BFD_RELOC_TILEPRO_IMM16_X1_HA:
    case BFD_RELOC_TILEPRO_IMM16_X0_HA_PCREL:
    case BFD_RELOC_TILEPRO_IMM16_X1_HA_PCREL:
      *valP = value = apply_special_operator (O_ha16, value);
      break;

    default:
      /* Do nothing  */
      break;
    }

  p = fixP->fx_frag->fr_literal + fixP->fx_where;

  operand = fixP->tc_fix_data;
  if (operand != NULL)
    {
      /* It's an instruction operand.  */
      tilepro_bundle_bits bits =
	insert_operand (0, operand, value, fixP->fx_file, fixP->fx_line);

      /* Note that we might either be writing out bits for a bundle or a
	 static network instruction, which are different sizes, so it's
	 important to stop touching memory once we run out of bits.  ORing in
	 values is OK since we know the existing bits for this operand are
	 zero.  */
      for (; bits != 0; bits >>= 8)
	*p++ |= (char)bits;
    }
  else
    {
      /* Some other kind of relocation.  */
      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_8:
	case BFD_RELOC_8_PCREL:
	  md_number_to_chars (p, value, 1);
	  break;

	case BFD_RELOC_16:
	case BFD_RELOC_16_PCREL:
	  md_number_to_chars (p, value, 2);
	  break;

	case BFD_RELOC_32:
	case BFD_RELOC_32_PCREL:
	  md_number_to_chars (p, value, 4);
	  break;

	default:
	  /* Leave it for the linker.  */
	  return;
	}
    }

  fixP->fx_done = 1;
}


/* Generate the BFD reloc to be stuck in the object file from the
   fixup used internally in the assembler.  */

arelent *
tc_gen_reloc (asection *sec ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc;

  reloc = XNEW (arelent);
  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  /* Make sure none of our internal relocations make it this far.
     They'd better have been fully resolved by this point.  */
  gas_assert ((int) fixp->fx_r_type > 0);

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("cannot represent `%s' relocation in object file"),
		    bfd_get_reloc_code_name (fixp->fx_r_type));
      return NULL;
    }

  if (!fixp->fx_pcrel != !reloc->howto->pc_relative)
    {
      as_fatal (_("internal error? cannot generate `%s' relocation (%d, %d)"),
		bfd_get_reloc_code_name (fixp->fx_r_type),
                fixp->fx_pcrel, reloc->howto->pc_relative);
    }
  gas_assert (!fixp->fx_pcrel == !reloc->howto->pc_relative);

  reloc->addend = fixp->fx_offset;

  return reloc;
}


/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from (fixS *fixP)
{
  return fixP->fx_frag->fr_address + fixP->fx_where;
}


/* Return 1 if it's OK to adjust a reloc by replacing the symbol with
   a section symbol plus some offset.  */
int
tilepro_fix_adjustable (fixS *fix)
{
  /* Prevent all adjustments to global symbols  */
  if (S_IS_EXTERNAL (fix->fx_addsy) || S_IS_WEAK (fix->fx_addsy))
    return 0;

  return 1;
}


int
tilepro_unrecognized_line (int ch)
{
  switch (ch)
    {
    case '{':
      if (inside_bundle)
	{
	  as_bad (_("Found '{' when already bundling."));
	}
      else
	{
	  inside_bundle = 1;
	  current_bundle_index = 0;
	}
      return 1;

    case '}':
      if (!inside_bundle)
	{
	  as_bad (_("Found '}' when not bundling."));
	}
      else
	{
	  tilepro_flush_bundle ();
	}

      /* Allow '{' to follow on the same line.  We also allow ";;", but that
	 happens automatically because ';' is an end of line marker.  */
      SKIP_WHITESPACE ();
      if (input_line_pointer[0] == '{')
	{
	  input_line_pointer++;
	  return tilepro_unrecognized_line ('{');
	}

      demand_empty_rest_of_line ();
      return 1;

    default:
      break;
    }

  /* Not a valid line.  */
  return 0;
}


/* This is called from HANDLE_ALIGN in write.c.  Fill in the contents
   of an rs_align_code fragment.  */

void
tilepro_handle_align (fragS *fragp)
{
  int bytes, fix;
  char *p;

  if (fragp->fr_type != rs_align_code)
    return;

  bytes = fragp->fr_next->fr_address - fragp->fr_address - fragp->fr_fix;
  p = fragp->fr_literal + fragp->fr_fix;
  fix = 0;

  /* Determine the bits for NOP.  */
  const struct tilepro_opcode *nop_opcode =
    &tilepro_opcodes[TILEPRO_OPC_NOP];
  tilepro_bundle_bits nop =
    (  nop_opcode->fixed_bit_values[TILEPRO_PIPELINE_X0]
       | nop_opcode->fixed_bit_values[TILEPRO_PIPELINE_X1]);

  if ((bytes & (TILEPRO_BUNDLE_SIZE_IN_BYTES - 1)) != 0)
    {
      fix = bytes & (TILEPRO_BUNDLE_SIZE_IN_BYTES - 1);
      memset (p, 0, fix);
      p += fix;
      bytes -= fix;
    }

  number_to_chars_littleendian (p, (unsigned int)nop, 4);
  number_to_chars_littleendian (p + 4, (unsigned int)(nop >> 32), 4);
  fragp->fr_fix += fix;
  fragp->fr_var = TILEPRO_BUNDLE_SIZE_IN_BYTES;
}

/* Standard calling conventions leave the CFA at SP on entry.  */
void
tilepro_cfi_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa_register (54);
}

int
tc_tilepro_regname_to_dw2regnum (char *regname)
{
  int i;

  for (i = 0; i < TILEPRO_NUM_REGISTERS; i++)
    {
      if (!strcmp (regname, tilepro_register_names[i]))
	return i;
    }

  return -1;
}
