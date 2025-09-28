/* Altera Nios II disassemble routines
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Nigel Gray (ngray@altera.com).
   Contributed by Mentor Graphics, Inc.

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
#include "disassemble.h"
#include "opintl.h"
#include "opcode/nios2.h"
#include "libiberty.h"
#include <string.h>
#include <assert.h>

/* No symbol table is available when this code runs out in an embedded
   system as when it is used for disassembler support in a monitor.  */
#if !defined(EMBEDDED_ENV)
#define SYMTAB_AVAILABLE 1
#include "elf-bfd.h"
#include "elf/nios2.h"
#endif

/* Default length of Nios II instruction in bytes.  */
#define INSNLEN 4

/* Data structures used by the opcode hash table.  */
typedef struct _nios2_opcode_hash
{
  const struct nios2_opcode *opcode;
  struct _nios2_opcode_hash *next;
} nios2_opcode_hash;

/* Hash table size.  */
#define OPCODE_HASH_SIZE (IW_R1_OP_UNSHIFTED_MASK + 1)

/* Extract the opcode from an instruction word.  */
static unsigned int
nios2_r1_extract_opcode (unsigned int x)
{
  return GET_IW_R1_OP (x);
}

static unsigned int
nios2_r2_extract_opcode (unsigned int x)
{
  return GET_IW_R2_OP (x);
}

/* We maintain separate hash tables for R1 and R2 opcodes, and pseudo-ops
   are stored in a different table than regular instructions.  */

typedef struct _nios2_disassembler_state
{
  const struct nios2_opcode *opcodes;
  const int *num_opcodes;
  unsigned int (*extract_opcode) (unsigned int);
  nios2_opcode_hash *hash[OPCODE_HASH_SIZE];
  nios2_opcode_hash *ps_hash[OPCODE_HASH_SIZE];
  const struct nios2_opcode *nop;
  bool init;
} nios2_disassembler_state;

static nios2_disassembler_state
nios2_r1_disassembler_state = {
  nios2_r1_opcodes,
  &nios2_num_r1_opcodes,
  nios2_r1_extract_opcode,
  {},
  {},
  NULL,
  0
};

static nios2_disassembler_state
nios2_r2_disassembler_state = {
  nios2_r2_opcodes,
  &nios2_num_r2_opcodes,
  nios2_r2_extract_opcode,
  {},
  {},
  NULL,
  0
};

/* Function to initialize the opcode hash table.  */
static void
nios2_init_opcode_hash (nios2_disassembler_state *state)
{
  unsigned int i;
  register const struct nios2_opcode *op;

  for (i = 0; i < OPCODE_HASH_SIZE; i++)
    for (op = state->opcodes; op < &state->opcodes[*(state->num_opcodes)]; op++)
      {
	nios2_opcode_hash *new_hash;
	nios2_opcode_hash **bucket = NULL;

	if ((op->pinfo & NIOS2_INSN_MACRO) == NIOS2_INSN_MACRO)
	  {
	    if (i == state->extract_opcode (op->match)
		&& (op->pinfo & (NIOS2_INSN_MACRO_MOV | NIOS2_INSN_MACRO_MOVI)
		    & 0x7fffffff))
	      {
		bucket = &(state->ps_hash[i]);
		if (strcmp (op->name, "nop") == 0)
		  state->nop = op;
	      }
	  }
	else if (i == state->extract_opcode (op->match))
	  bucket = &(state->hash[i]);

	if (bucket)
	  {
	    new_hash =
	      (nios2_opcode_hash *) malloc (sizeof (nios2_opcode_hash));
	    if (new_hash == NULL)
	      {
		/* xgettext:c-format */
		opcodes_error_handler (_("out of memory"));
		exit (1);
	      }
	    new_hash->opcode = op;
	    new_hash->next = NULL;
	    while (*bucket)
	      bucket = &((*bucket)->next);
	    *bucket = new_hash;
	  }
      }
  state->init = 1;

#ifdef DEBUG_HASHTABLE
  for (i = 0; i < OPCODE_HASH_SIZE; ++i)
    {
      nios2_opcode_hash *tmp_hash = state->hash[i];
      printf ("index: 0x%02X	ops: ", i);
      while (tmp_hash != NULL)
	{
	  printf ("%s ", tmp_hash->opcode->name);
	  tmp_hash = tmp_hash->next;
	}
      printf ("\n");
    }

  for (i = 0; i < OPCODE_HASH_SIZE; ++i)
    {
      nios2_opcode_hash *tmp_hash = state->ps_hash[i];
      printf ("index: 0x%02X	ops: ", i);
      while (tmp_hash != NULL)
	{
	  printf ("%s ", tmp_hash->opcode->name);
	  tmp_hash = tmp_hash->next;
	}
      printf ("\n");
    }
#endif /* DEBUG_HASHTABLE */
}

/* Return a pointer to an nios2_opcode struct for a given instruction
   word OPCODE for bfd machine MACH, or NULL if there is an error.  */
const struct nios2_opcode *
nios2_find_opcode_hash (unsigned long opcode, unsigned long mach)
{
  nios2_opcode_hash *entry;
  nios2_disassembler_state *state;

  /* Select the right instruction set, hash tables, and opcode accessor
     for the mach variant.  */
  if (mach == bfd_mach_nios2r2)
    state = &nios2_r2_disassembler_state;
  else
    state = &nios2_r1_disassembler_state;

  /* Build a hash table to shorten the search time.  */
  if (!state->init)
    nios2_init_opcode_hash (state);

  /* Check for NOP first.  Both NOP and MOV are macros that expand into
     an ADD instruction, and we always want to give priority to NOP.  */
  if (state->nop->match == (opcode & state->nop->mask))
    return state->nop;

  /* First look in the pseudo-op hashtable.  */
  for (entry = state->ps_hash[state->extract_opcode (opcode)];
       entry; entry = entry->next)
    if (entry->opcode->match == (opcode & entry->opcode->mask))
      return entry->opcode;

  /* Otherwise look in the main hashtable.  */
  for (entry = state->hash[state->extract_opcode (opcode)];
       entry; entry = entry->next)
    if (entry->opcode->match == (opcode & entry->opcode->mask))
      return entry->opcode;

  return NULL;
}

/* There are 32 regular registers, 32 coprocessor registers,
   and 32 control registers.  */
#define NUMREGNAMES 32

/* Return a pointer to the base of the coprocessor register name array.  */
static struct nios2_reg *
nios2_coprocessor_regs (void)
{
  static struct nios2_reg *cached = NULL;

  if (!cached)
    {
      int i;
      for (i = NUMREGNAMES; i < nios2_num_regs; i++)
	if (!strcmp (nios2_regs[i].name, "c0"))
	  {
	    cached = nios2_regs + i;
	    break;
	  }
      assert (cached);
    }
  return cached;
}

/* Return a pointer to the base of the control register name array.  */
static struct nios2_reg *
nios2_control_regs (void)
{
  static struct nios2_reg *cached = NULL;

  if (!cached)
    {
      int i;
      for (i = NUMREGNAMES; i < nios2_num_regs; i++)
	if (!strcmp (nios2_regs[i].name, "status"))
	  {
	    cached = nios2_regs + i;
	    break;
	  }
      assert (cached);
    }
  return cached;
}

/* Helper routine to report internal errors.  */
static void
bad_opcode (const struct nios2_opcode *op)
{
  opcodes_error_handler
    /* xgettext:c-format */
    (_("internal error: broken opcode descriptor for `%s %s'"),
     op->name, op->args);
  abort ();
}

/* The function nios2_print_insn_arg uses the character pointed
   to by ARGPTR to determine how it print the next token or separator
   character in the arguments to an instruction.  */
static int
nios2_print_insn_arg (const char *argptr,
		      unsigned long opcode, bfd_vma address,
		      disassemble_info *info,
		      const struct nios2_opcode *op)
{
  unsigned long i = 0;
  long s = 0;
  int32_t o = 0;
  struct nios2_reg *reg_base;

  switch (*argptr)
    {
    case ',':
    case '(':
    case ')':
      (*info->fprintf_func) (info->stream, "%c", *argptr);
      break;

    case 'c':
      /* Control register index.  */
      switch (op->format)
	{
	case iw_r_type:
	  i = GET_IW_R_IMM5 (opcode);
	  break;
	case iw_F3X6L5_type:
	  i = GET_IW_F3X6L5_IMM5 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      reg_base = nios2_control_regs ();
      (*info->fprintf_func) (info->stream, "%s", reg_base[i].name);
      break;

    case 'd':
      reg_base = nios2_regs;
      switch (op->format)
	{
	case iw_r_type:
	  i = GET_IW_R_C (opcode);
	  break;
	case iw_custom_type:
	  i = GET_IW_CUSTOM_C (opcode);
	  if (GET_IW_CUSTOM_READC (opcode) == 0)
	    reg_base = nios2_coprocessor_regs ();
	  break;
	case iw_F3X6L5_type:
	case iw_F3X6_type:
	  i = GET_IW_F3X6L5_C (opcode);
	  break;
	case iw_F3X8_type:
	  i = GET_IW_F3X8_C (opcode);
	  if (GET_IW_F3X8_READC (opcode) == 0)
	    reg_base = nios2_coprocessor_regs ();
	  break;
	case iw_F2_type:
	  i = GET_IW_F2_B (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      if (i < NUMREGNAMES)
	(*info->fprintf_func) (info->stream, "%s", reg_base[i].name);
      else
	(*info->fprintf_func) (info->stream, "unknown");
      break;

    case 's':
      reg_base = nios2_regs;
      switch (op->format)
	{
	case iw_r_type:
	  i = GET_IW_R_A (opcode);
	  break;
	case iw_i_type:
	  i = GET_IW_I_A (opcode);
	  break;
	case iw_custom_type:
	  i = GET_IW_CUSTOM_A (opcode);
	  if (GET_IW_CUSTOM_READA (opcode) == 0)
	    reg_base = nios2_coprocessor_regs ();
	  break;
	case iw_F2I16_type:
	  i = GET_IW_F2I16_A (opcode);
	  break;
	case iw_F2X4I12_type:
	  i = GET_IW_F2X4I12_A (opcode);
	  break;
	case iw_F1X4I12_type:
	  i = GET_IW_F1X4I12_A (opcode);
	  break;
	case iw_F1X4L17_type:
	  i = GET_IW_F1X4L17_A (opcode);
	  break;
	case iw_F3X6L5_type:
	case iw_F3X6_type:
	  i = GET_IW_F3X6L5_A (opcode);
	  break;
	case iw_F2X6L10_type:
	  i = GET_IW_F2X6L10_A (opcode);
	  break;
	case iw_F3X8_type:
	  i = GET_IW_F3X8_A (opcode);
	  if (GET_IW_F3X8_READA (opcode) == 0)
	    reg_base = nios2_coprocessor_regs ();
	  break;
	case iw_F1X1_type:
	  i = GET_IW_F1X1_A (opcode);
	  break;
	case iw_F1I5_type:
	  i = 27;   /* Implicit stack pointer reference.  */
	  break;
	case iw_F2_type:
	  i = GET_IW_F2_A (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      if (i < NUMREGNAMES)
	(*info->fprintf_func) (info->stream, "%s", reg_base[i].name);
      else
	(*info->fprintf_func) (info->stream, "unknown");
      break;

    case 't':
      reg_base = nios2_regs;
      switch (op->format)
	{
	case iw_r_type:
	  i = GET_IW_R_B (opcode);
	  break;
	case iw_i_type:
	  i = GET_IW_I_B (opcode);
	  break;
	case iw_custom_type:
	  i = GET_IW_CUSTOM_B (opcode);
	  if (GET_IW_CUSTOM_READB (opcode) == 0)
	    reg_base = nios2_coprocessor_regs ();
	  break;
	case iw_F2I16_type:
	  i = GET_IW_F2I16_B (opcode);
	  break;
	case iw_F2X4I12_type:
	  i = GET_IW_F2X4I12_B (opcode);
	  break;
	case iw_F3X6L5_type:
	case iw_F3X6_type:
	  i = GET_IW_F3X6L5_B (opcode);
	  break;
	case iw_F2X6L10_type:
	  i = GET_IW_F2X6L10_B (opcode);
	  break;
	case iw_F3X8_type:
	  i = GET_IW_F3X8_B (opcode);
	  if (GET_IW_F3X8_READB (opcode) == 0)
	    reg_base = nios2_coprocessor_regs ();
	  break;
	case iw_F1I5_type:
	  i = GET_IW_F1I5_B (opcode);
	  break;
	case iw_F2_type:
	  i = GET_IW_F2_B (opcode);
	  break;
	case iw_T1X1I6_type:
	  i = 0;
	  break;
	default:
	  bad_opcode (op);
	}
      if (i < NUMREGNAMES)
	(*info->fprintf_func) (info->stream, "%s", reg_base[i].name);
      else
	(*info->fprintf_func) (info->stream, "unknown");
      break;

    case 'D':
      switch (op->format)
	{
	case iw_T1I7_type:
	  i = GET_IW_T1I7_A3 (opcode);
	  break;
	case iw_T2X1L3_type:
	  i = GET_IW_T2X1L3_B3 (opcode);
	  break;
	case iw_T2X1I3_type:
	  i = GET_IW_T2X1I3_B3 (opcode);
	  break;
	case iw_T3X1_type:
	  i = GET_IW_T3X1_C3 (opcode);
	  break;
	case iw_T2X3_type:
	  if (op->num_args == 3)
	    i = GET_IW_T2X3_A3 (opcode);
	  else
	    i = GET_IW_T2X3_B3 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      i = nios2_r2_reg3_mappings[i];
      (*info->fprintf_func) (info->stream, "%s", nios2_regs[i].name);
      break;

    case 'M':
      /* 6-bit unsigned immediate with no shift.  */
      switch (op->format)
	{
	case iw_T1X1I6_type:
	  i = GET_IW_T1X1I6_IMM6 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'N':
      /* 6-bit unsigned immediate with 2-bit shift.  */
      switch (op->format)
	{
	case iw_T1X1I6_type:
	  i = GET_IW_T1X1I6_IMM6 (opcode) << 2;
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'S':
      switch (op->format)
	{
	case iw_T1I7_type:
	  i = GET_IW_T1I7_A3 (opcode);
	  break;
	case iw_T2I4_type:
	  i = GET_IW_T2I4_A3 (opcode);
	  break;
	case iw_T2X1L3_type:
	  i = GET_IW_T2X1L3_A3 (opcode);
	  break;
	case iw_T2X1I3_type:
	  i = GET_IW_T2X1I3_A3 (opcode);
	  break;
	case iw_T3X1_type:
	  i = GET_IW_T3X1_A3 (opcode);
	  break;
	case iw_T2X3_type:
	  i = GET_IW_T2X3_A3 (opcode);
	  break;
	case iw_T1X1I6_type:
	  i = GET_IW_T1X1I6_A3 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      i = nios2_r2_reg3_mappings[i];
      (*info->fprintf_func) (info->stream, "%s", nios2_regs[i].name);
      break;

    case 'T':
      switch (op->format)
	{
	case iw_T2I4_type:
	  i = GET_IW_T2I4_B3 (opcode);
	  break;
	case iw_T3X1_type:
	  i = GET_IW_T3X1_B3 (opcode);
	  break;
	case iw_T2X3_type:
	  i = GET_IW_T2X3_B3 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      i = nios2_r2_reg3_mappings[i];
      (*info->fprintf_func) (info->stream, "%s", nios2_regs[i].name);
      break;

    case 'i':
      /* 16-bit signed immediate.  */
      switch (op->format)
	{
	case iw_i_type:
	  s = ((int32_t) ((GET_IW_I_IMM16 (opcode) & 0xffff) ^ 0x8000)
	       - 0x8000);
	  break;
	case iw_F2I16_type:
	  s = ((int32_t) ((GET_IW_F2I16_IMM16 (opcode) & 0xffff) ^ 0x8000)
	       - 0x8000);
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", s);
      break;

    case 'I':
      /* 12-bit signed immediate.  */
      switch (op->format)
	{
	case iw_F2X4I12_type:
	  s = ((int32_t) ((GET_IW_F2X4I12_IMM12 (opcode) & 0xfff) ^ 0x800)
	       - 0x800);
	  break;
	case iw_F1X4I12_type:
	  s = ((int32_t) ((GET_IW_F1X4I12_IMM12 (opcode) & 0xfff) ^ 0x800)
	       - 0x800);
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", s);
      break;

    case 'u':
      /* 16-bit unsigned immediate.  */
      switch (op->format)
	{
	case iw_i_type:
	  i = GET_IW_I_IMM16 (opcode);
	  break;
	case iw_F2I16_type:
	  i = GET_IW_F2I16_IMM16 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'U':
      /* 7-bit unsigned immediate with 2-bit shift.  */
      switch (op->format)
	{
	case iw_T1I7_type:
	  i = GET_IW_T1I7_IMM7 (opcode) << 2;
	  break;
	case iw_X1I7_type:
	  i = GET_IW_X1I7_IMM7 (opcode) << 2;
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'V':
      /* 5-bit unsigned immediate with 2-bit shift.  */
      switch (op->format)
	{
	case iw_F1I5_type:
	  i = GET_IW_F1I5_IMM5 (opcode) << 2;
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'W':
      /* 4-bit unsigned immediate with 2-bit shift.  */
      switch (op->format)
	{
	case iw_T2I4_type:
	  i = GET_IW_T2I4_IMM4 (opcode) << 2;
	  break;
	case iw_L5I4X1_type:
	  i = GET_IW_L5I4X1_IMM4 (opcode) << 2;
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'X':
      /* 4-bit unsigned immediate with 1-bit shift.  */
      switch (op->format)
	{
	case iw_T2I4_type:
	  i = GET_IW_T2I4_IMM4 (opcode) << 1;
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'Y':
      /* 4-bit unsigned immediate without shift.  */
      switch (op->format)
	{
	case iw_T2I4_type:
	  i = GET_IW_T2I4_IMM4 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'o':
      /* 16-bit signed immediate address offset.  */
      switch (op->format)
	{
	case iw_i_type:
	  o = ((GET_IW_I_IMM16 (opcode) & 0xffff) ^ 0x8000) - 0x8000;
	  break;
	case iw_F2I16_type:
	  o = ((GET_IW_F2I16_IMM16 (opcode) & 0xffff) ^ 0x8000) - 0x8000;
	  break;
	default:
	  bad_opcode (op);
	}
      address = address + 4 + o;
      (*info->print_address_func) (address, info);
      break;

    case 'O':
      /* 10-bit signed address offset with 1-bit shift.  */
      switch (op->format)
	{
	case iw_I10_type:
	  o = (((GET_IW_I10_IMM10 (opcode) & 0x3ff) ^ 0x200) - 0x200) * 2;
	  break;
	default:
	  bad_opcode (op);
	}
      address = address + 2 + o;
      (*info->print_address_func) (address, info);
      break;

    case 'P':
      /* 7-bit signed address offset with 1-bit shift.  */
      switch (op->format)
	{
	case iw_T1I7_type:
	  o = (((GET_IW_T1I7_IMM7 (opcode) & 0x7f) ^ 0x40) - 0x40) * 2;
	  break;
	default:
	  bad_opcode (op);
	}
      address = address + 2 + o;
      (*info->print_address_func) (address, info);
      break;

    case 'j':
      /* 5-bit unsigned immediate.  */
      switch (op->format)
	{
	case iw_r_type:
	  i = GET_IW_R_IMM5 (opcode);
	  break;
	case iw_F3X6L5_type:
	  i = GET_IW_F3X6L5_IMM5 (opcode);
	  break;
	case iw_F2X6L10_type:
	  i = GET_IW_F2X6L10_MSB (opcode);
	  break;
	case iw_X2L5_type:
	  i = GET_IW_X2L5_IMM5 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'k':
      /* Second 5-bit unsigned immediate field.  */
      switch (op->format)
	{
	case iw_F2X6L10_type:
	  i = GET_IW_F2X6L10_LSB (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'l':
      /* 8-bit unsigned immediate.  */
      switch (op->format)
	{
	case iw_custom_type:
	  i = GET_IW_CUSTOM_N (opcode);
	  break;
	case iw_F3X8_type:
	  i = GET_IW_F3X8_N (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%lu", i);
      break;

    case 'm':
      /* 26-bit unsigned immediate.  */
      switch (op->format)
	{
	case iw_j_type:
	  i = GET_IW_J_IMM26 (opcode);
	  break;
	case iw_L26_type:
	  i = GET_IW_L26_IMM26 (opcode);
	  break;
	default:
	  bad_opcode (op);
	}
      /* This translates to an address because it's only used in call
	 instructions.  */
      address = (address & 0xf0000000) | (i << 2);
      (*info->print_address_func) (address, info);
      break;

    case 'e':
      /* Encoded enumeration for addi.n/subi.n.  */
      switch (op->format)
	{
	case iw_T2X1I3_type:
	  i = nios2_r2_asi_n_mappings[GET_IW_T2X1I3_IMM3 (opcode)];
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%lu", i);
      break;

    case 'f':
      /* Encoded enumeration for slli.n/srli.n.  */
      switch (op->format)
	{
	case iw_T2X1L3_type:
	  i = nios2_r2_shi_n_mappings[GET_IW_T2X1I3_IMM3 (opcode)];
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%lu", i);
      break;

    case 'g':
      /* Encoded enumeration for andi.n.  */
      switch (op->format)
	{
	case iw_T2I4_type:
	  i = nios2_r2_andi_n_mappings[GET_IW_T2I4_IMM4 (opcode)];
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%lu", i);
      break;

    case 'h':
      /* Encoded enumeration for movi.n.  */
      switch (op->format)
	{
	case iw_T1I7_type:
	  i = GET_IW_T1I7_IMM7 (opcode);
	  if (i == 125)
	    i = 0xff;
	  else if (i == 126)
	    i = -2;
	  else if (i == 127)
	    i = -1;
	  break;
	default:
	  bad_opcode (op);
	}
      (*info->fprintf_func) (info->stream, "%ld", i);
      break;

    case 'R':
      {
	unsigned long reglist = 0;
	int dir = 1;
	int k, t;

	switch (op->format)
	  {
	  case iw_F1X4L17_type:
	    /* Encoding for ldwm/stwm.  */
	    i = GET_IW_F1X4L17_REGMASK (opcode);
	    if (GET_IW_F1X4L17_RS (opcode))
	      {
		reglist = ((i << 14) & 0x00ffc000);
		if (i & (1 << 10))
		  reglist |= (1 << 28);
		if (i & (1 << 11))
		  reglist |= (1u << 31);
	      }
	    else
	      reglist = i << 2;
	    dir = GET_IW_F1X4L17_REGMASK (opcode) ? 1 : -1;
	    break;

	  case iw_L5I4X1_type:
	    /* Encoding for push.n/pop.n.  */
	    reglist |= (1u << 31);
	    if (GET_IW_L5I4X1_FP (opcode))
	      reglist |= (1 << 28);
	    if (GET_IW_L5I4X1_CS (opcode))
	      {
		int val = GET_IW_L5I4X1_REGRANGE (opcode);
		reglist |= nios2_r2_reg_range_mappings[val];
	      }
	    dir = (op->match == MATCH_R2_POP_N ? 1 : -1);
	    break;

	  default:
	    bad_opcode (op);
	  }

	t = 0;
	(*info->fprintf_func) (info->stream, "{");
	for (k = (dir == 1 ? 0 : 31);
	     (dir == 1 && k < 32) || (dir == -1 && k >= 0);
	     k += dir)
	  if (reglist & (1u << k))
	    {
	      if (t)
		(*info->fprintf_func) (info->stream, ",");
	      else
		t++;
	      (*info->fprintf_func) (info->stream, "%s", nios2_regs[k].name);
	    }
	(*info->fprintf_func) (info->stream, "}");
	break;
      }

    case 'B':
      /* Base register and options for ldwm/stwm.  */
      switch (op->format)
	{
	case iw_F1X4L17_type:
	  if (GET_IW_F1X4L17_ID (opcode) == 0)
	    (*info->fprintf_func) (info->stream, "--");

	  i = GET_IW_F1X4I12_A (opcode);
	  (*info->fprintf_func) (info->stream, "(%s)",
				 nios2_builtin_regs[i].name);

	  if (GET_IW_F1X4L17_ID (opcode))
	    (*info->fprintf_func) (info->stream, "++");
	  if (GET_IW_F1X4L17_WB (opcode))
	    (*info->fprintf_func) (info->stream, ",writeback");
	  if (GET_IW_F1X4L17_PC (opcode))
	    (*info->fprintf_func) (info->stream, ",ret");
	  break;
	default:
	  bad_opcode (op);
	}
      break;

    default:
      (*info->fprintf_func) (info->stream, "unknown");
      break;
    }
  return 0;
}

/* nios2_disassemble does all the work of disassembling a Nios II
   instruction opcode.  */
static int
nios2_disassemble (bfd_vma address, unsigned long opcode,
		   disassemble_info *info)
{
  const struct nios2_opcode *op;

  info->bytes_per_line = INSNLEN;
  info->bytes_per_chunk = INSNLEN;
  info->display_endian = info->endian;
  info->insn_info_valid = 1;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->insn_type = dis_nonbranch;
  info->target = 0;
  info->target2 = 0;

  /* Find the major opcode and use this to disassemble
     the instruction and its arguments.  */
  op = nios2_find_opcode_hash (opcode, info->mach);

  if (op != NULL)
    {
      const char *argstr = op->args;
      (*info->fprintf_func) (info->stream, "%s", op->name);
      if (argstr != NULL && *argstr != '\0')
	{
	  (*info->fprintf_func) (info->stream, "\t");
	  while (*argstr != '\0')
	    {
	      nios2_print_insn_arg (argstr, opcode, address, info, op);
	      ++argstr;
	    }
	}
      /* Tell the caller how far to advance the program counter.  */
      info->bytes_per_chunk = op->size;
      return op->size;
    }
  else
    {
      /* Handle undefined instructions.  */
      info->insn_type = dis_noninsn;
      (*info->fprintf_func) (info->stream, "0x%lx", opcode);
      return INSNLEN;
    }
}


/* print_insn_nios2 is the main disassemble function for Nios II.
   The function diassembler(abfd) (source in disassemble.c) returns a
   pointer to this either print_insn_big_nios2 or
   print_insn_little_nios2, which in turn call this function when the
   bfd machine type is Nios II. print_insn_nios2 reads the
   instruction word at the address given, and prints the disassembled
   instruction on the stream info->stream using info->fprintf_func. */

static int
print_insn_nios2 (bfd_vma address, disassemble_info *info,
		  enum bfd_endian endianness)
{
  bfd_byte buffer[INSNLEN];
  int status;

  status = (*info->read_memory_func) (address, buffer, INSNLEN, info);
  if (status == 0)
    {
      unsigned long insn;
      if (endianness == BFD_ENDIAN_BIG)
	insn = (unsigned long) bfd_getb32 (buffer);
      else
	insn = (unsigned long) bfd_getl32 (buffer);
      return nios2_disassemble (address, insn, info);
    }

  /* We might have a 16-bit R2 instruction at the end of memory.  Try that.  */
  if (info->mach == bfd_mach_nios2r2)
    {
      status = (*info->read_memory_func) (address, buffer, 2, info);
      if (status == 0)
	{
	  unsigned long insn;
	  if (endianness == BFD_ENDIAN_BIG)
	    insn = (unsigned long) bfd_getb16 (buffer);
	  else
	    insn = (unsigned long) bfd_getl16 (buffer);
	  return nios2_disassemble (address, insn, info);
	}
    }

  /* If we got here, we couldn't read anything.  */
  (*info->memory_error_func) (status, address, info);
  return -1;
}

/* These two functions are the main entry points, accessed from
   disassemble.c.  */
int
print_insn_big_nios2 (bfd_vma address, disassemble_info *info)
{
  return print_insn_nios2 (address, info, BFD_ENDIAN_BIG);
}

int
print_insn_little_nios2 (bfd_vma address, disassemble_info *info)
{
  return print_insn_nios2 (address, info, BFD_ENDIAN_LITTLE);
}
