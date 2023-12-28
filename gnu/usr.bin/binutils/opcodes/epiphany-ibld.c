/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Instruction building/extraction support for epiphany. -*- C -*-

   THIS FILE IS MACHINE GENERATED WITH CGEN: Cpu tools GENerator.
   - the resultant file is machine generated, cgen-ibld.in isn't

   Copyright (C) 1996-2023 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/* ??? Eventually more and more of this stuff can go to cpu-independent files.
   Keep that in mind.  */

#include "sysdep.h"
#include <stdio.h>
#include "ansidecl.h"
#include "dis-asm.h"
#include "bfd.h"
#include "symcat.h"
#include "epiphany-desc.h"
#include "epiphany-opc.h"
#include "cgen/basic-modes.h"
#include "opintl.h"
#include "safe-ctype.h"

#undef  min
#define min(a,b) ((a) < (b) ? (a) : (b))
#undef  max
#define max(a,b) ((a) > (b) ? (a) : (b))

/* Used by the ifield rtx function.  */
#define FLD(f) (fields->f)

static const char * insert_normal
  (CGEN_CPU_DESC, long, unsigned int, unsigned int, unsigned int,
   unsigned int, unsigned int, unsigned int, CGEN_INSN_BYTES_PTR);
static const char * insert_insn_normal
  (CGEN_CPU_DESC, const CGEN_INSN *,
   CGEN_FIELDS *, CGEN_INSN_BYTES_PTR, bfd_vma);
static int extract_normal
  (CGEN_CPU_DESC, CGEN_EXTRACT_INFO *, CGEN_INSN_INT,
   unsigned int, unsigned int, unsigned int, unsigned int,
   unsigned int, unsigned int, bfd_vma, long *);
static int extract_insn_normal
  (CGEN_CPU_DESC, const CGEN_INSN *, CGEN_EXTRACT_INFO *,
   CGEN_INSN_INT, CGEN_FIELDS *, bfd_vma);
#if CGEN_INT_INSN_P
static void put_insn_int_value
  (CGEN_CPU_DESC, CGEN_INSN_BYTES_PTR, int, int, CGEN_INSN_INT);
#endif
#if ! CGEN_INT_INSN_P
static CGEN_INLINE void insert_1
  (CGEN_CPU_DESC, unsigned long, int, int, int, unsigned char *);
static CGEN_INLINE int fill_cache
  (CGEN_CPU_DESC, CGEN_EXTRACT_INFO *,  int, int, bfd_vma);
static CGEN_INLINE long extract_1
  (CGEN_CPU_DESC, CGEN_EXTRACT_INFO *, int, int, int, unsigned char *, bfd_vma);
#endif

/* Operand insertion.  */

#if ! CGEN_INT_INSN_P

/* Subroutine of insert_normal.  */

static CGEN_INLINE void
insert_1 (CGEN_CPU_DESC cd,
	  unsigned long value,
	  int start,
	  int length,
	  int word_length,
	  unsigned char *bufp)
{
  unsigned long x, mask;
  int shift;

  x = cgen_get_insn_value (cd, bufp, word_length, cd->endian);

  /* Written this way to avoid undefined behaviour.  */
  mask = (1UL << (length - 1) << 1) - 1;
  if (CGEN_INSN_LSB0_P)
    shift = (start + 1) - length;
  else
    shift = (word_length - (start + length));
  x = (x & ~(mask << shift)) | ((value & mask) << shift);

  cgen_put_insn_value (cd, bufp, word_length, (bfd_vma) x, cd->endian);
}

#endif /* ! CGEN_INT_INSN_P */

/* Default insertion routine.

   ATTRS is a mask of the boolean attributes.
   WORD_OFFSET is the offset in bits from the start of the insn of the value.
   WORD_LENGTH is the length of the word in bits in which the value resides.
   START is the starting bit number in the word, architecture origin.
   LENGTH is the length of VALUE in bits.
   TOTAL_LENGTH is the total length of the insn in bits.

   The result is an error message or NULL if success.  */

/* ??? This duplicates functionality with bfd's howto table and
   bfd_install_relocation.  */
/* ??? This doesn't handle bfd_vma's.  Create another function when
   necessary.  */

static const char *
insert_normal (CGEN_CPU_DESC cd,
	       long value,
	       unsigned int attrs,
	       unsigned int word_offset,
	       unsigned int start,
	       unsigned int length,
	       unsigned int word_length,
	       unsigned int total_length,
	       CGEN_INSN_BYTES_PTR buffer)
{
  static char errbuf[100];
  unsigned long mask;

  /* If LENGTH is zero, this operand doesn't contribute to the value.  */
  if (length == 0)
    return NULL;

  /* Written this way to avoid undefined behaviour.  */
  mask = (1UL << (length - 1) << 1) - 1;

  if (word_length > 8 * sizeof (CGEN_INSN_INT))
    abort ();

  /* For architectures with insns smaller than the base-insn-bitsize,
     word_length may be too big.  */
  if (cd->min_insn_bitsize < cd->base_insn_bitsize)
    {
      if (word_offset == 0
	  && word_length > total_length)
	word_length = total_length;
    }

  /* Ensure VALUE will fit.  */
  if (CGEN_BOOL_ATTR (attrs, CGEN_IFLD_SIGN_OPT))
    {
      long minval = - (1UL << (length - 1));
      unsigned long maxval = mask;

      if ((value > 0 && (unsigned long) value > maxval)
	  || value < minval)
	{
	  /* xgettext:c-format */
	  sprintf (errbuf,
		   _("operand out of range (%ld not between %ld and %lu)"),
		   value, minval, maxval);
	  return errbuf;
	}
    }
  else if (! CGEN_BOOL_ATTR (attrs, CGEN_IFLD_SIGNED))
    {
      unsigned long maxval = mask;
      unsigned long val = (unsigned long) value;

      /* For hosts with a word size > 32 check to see if value has been sign
	 extended beyond 32 bits.  If so then ignore these higher sign bits
	 as the user is attempting to store a 32-bit signed value into an
	 unsigned 32-bit field which is allowed.  */
      if (sizeof (unsigned long) > 4 && ((value >> 32) == -1))
	val &= 0xFFFFFFFF;

      if (val > maxval)
	{
	  /* xgettext:c-format */
	  sprintf (errbuf,
		   _("operand out of range (0x%lx not between 0 and 0x%lx)"),
		   val, maxval);
	  return errbuf;
	}
    }
  else
    {
      if (! cgen_signed_overflow_ok_p (cd))
	{
	  long minval = - (1UL << (length - 1));
	  long maxval =   (1UL << (length - 1)) - 1;

	  if (value < minval || value > maxval)
	    {
	      sprintf
		/* xgettext:c-format */
		(errbuf, _("operand out of range (%ld not between %ld and %ld)"),
		 value, minval, maxval);
	      return errbuf;
	    }
	}
    }

#if CGEN_INT_INSN_P

  {
    int shift_within_word, shift_to_word, shift;

    /* How to shift the value to BIT0 of the word.  */
    shift_to_word = total_length - (word_offset + word_length);

    /* How to shift the value to the field within the word.  */
    if (CGEN_INSN_LSB0_P)
      shift_within_word = start + 1 - length;
    else
      shift_within_word = word_length - start - length;

    /* The total SHIFT, then mask in the value.  */
    shift = shift_to_word + shift_within_word;
    *buffer = (*buffer & ~(mask << shift)) | ((value & mask) << shift);
  }

#else /* ! CGEN_INT_INSN_P */

  {
    unsigned char *bufp = (unsigned char *) buffer + word_offset / 8;

    insert_1 (cd, value, start, length, word_length, bufp);
  }

#endif /* ! CGEN_INT_INSN_P */

  return NULL;
}

/* Default insn builder (insert handler).
   The instruction is recorded in CGEN_INT_INSN_P byte order (meaning
   that if CGEN_INSN_BYTES_PTR is an int * and thus, the value is
   recorded in host byte order, otherwise BUFFER is an array of bytes
   and the value is recorded in target byte order).
   The result is an error message or NULL if success.  */

static const char *
insert_insn_normal (CGEN_CPU_DESC cd,
		    const CGEN_INSN * insn,
		    CGEN_FIELDS * fields,
		    CGEN_INSN_BYTES_PTR buffer,
		    bfd_vma pc)
{
  const CGEN_SYNTAX *syntax = CGEN_INSN_SYNTAX (insn);
  unsigned long value;
  const CGEN_SYNTAX_CHAR_TYPE * syn;

  CGEN_INIT_INSERT (cd);
  value = CGEN_INSN_BASE_VALUE (insn);

  /* If we're recording insns as numbers (rather than a string of bytes),
     target byte order handling is deferred until later.  */

#if CGEN_INT_INSN_P

  put_insn_int_value (cd, buffer, cd->base_insn_bitsize,
		      CGEN_FIELDS_BITSIZE (fields), value);

#else

  cgen_put_insn_value (cd, buffer, min ((unsigned) cd->base_insn_bitsize,
                                        (unsigned) CGEN_FIELDS_BITSIZE (fields)),
		       value, cd->insn_endian);

#endif /* ! CGEN_INT_INSN_P */

  /* ??? It would be better to scan the format's fields.
     Still need to be able to insert a value based on the operand though;
     e.g. storing a branch displacement that got resolved later.
     Needs more thought first.  */

  for (syn = CGEN_SYNTAX_STRING (syntax); * syn; ++ syn)
    {
      const char *errmsg;

      if (CGEN_SYNTAX_CHAR_P (* syn))
	continue;

      errmsg = (* cd->insert_operand) (cd, CGEN_SYNTAX_FIELD (*syn),
				       fields, buffer, pc);
      if (errmsg)
	return errmsg;
    }

  return NULL;
}

#if CGEN_INT_INSN_P
/* Cover function to store an insn value into an integral insn.  Must go here
   because it needs <prefix>-desc.h for CGEN_INT_INSN_P.  */

static void
put_insn_int_value (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
		    CGEN_INSN_BYTES_PTR buf,
		    int length,
		    int insn_length,
		    CGEN_INSN_INT value)
{
  /* For architectures with insns smaller than the base-insn-bitsize,
     length may be too big.  */
  if (length > insn_length)
    *buf = value;
  else
    {
      int shift = insn_length - length;
      /* Written this way to avoid undefined behaviour.  */
      CGEN_INSN_INT mask = length == 0 ? 0 : (1UL << (length - 1) << 1) - 1;

      *buf = (*buf & ~(mask << shift)) | ((value & mask) << shift);
    }
}
#endif

/* Operand extraction.  */

#if ! CGEN_INT_INSN_P

/* Subroutine of extract_normal.
   Ensure sufficient bytes are cached in EX_INFO.
   OFFSET is the offset in bytes from the start of the insn of the value.
   BYTES is the length of the needed value.
   Returns 1 for success, 0 for failure.  */

static CGEN_INLINE int
fill_cache (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	    CGEN_EXTRACT_INFO *ex_info,
	    int offset,
	    int bytes,
	    bfd_vma pc)
{
  /* It's doubtful that the middle part has already been fetched so
     we don't optimize that case.  kiss.  */
  unsigned int mask;
  disassemble_info *info = (disassemble_info *) ex_info->dis_info;

  /* First do a quick check.  */
  mask = (1 << bytes) - 1;
  if (((ex_info->valid >> offset) & mask) == mask)
    return 1;

  /* Search for the first byte we need to read.  */
  for (mask = 1 << offset; bytes > 0; --bytes, ++offset, mask <<= 1)
    if (! (mask & ex_info->valid))
      break;

  if (bytes)
    {
      int status;

      pc += offset;
      status = (*info->read_memory_func)
	(pc, ex_info->insn_bytes + offset, bytes, info);

      if (status != 0)
	{
	  (*info->memory_error_func) (status, pc, info);
	  return 0;
	}

      ex_info->valid |= ((1 << bytes) - 1) << offset;
    }

  return 1;
}

/* Subroutine of extract_normal.  */

static CGEN_INLINE long
extract_1 (CGEN_CPU_DESC cd,
	   CGEN_EXTRACT_INFO *ex_info ATTRIBUTE_UNUSED,
	   int start,
	   int length,
	   int word_length,
	   unsigned char *bufp,
	   bfd_vma pc ATTRIBUTE_UNUSED)
{
  unsigned long x;
  int shift;

  x = cgen_get_insn_value (cd, bufp, word_length, cd->endian);

  if (CGEN_INSN_LSB0_P)
    shift = (start + 1) - length;
  else
    shift = (word_length - (start + length));
  return x >> shift;
}

#endif /* ! CGEN_INT_INSN_P */

/* Default extraction routine.

   INSN_VALUE is the first base_insn_bitsize bits of the insn in host order,
   or sometimes less for cases like the m32r where the base insn size is 32
   but some insns are 16 bits.
   ATTRS is a mask of the boolean attributes.  We only need `SIGNED',
   but for generality we take a bitmask of all of them.
   WORD_OFFSET is the offset in bits from the start of the insn of the value.
   WORD_LENGTH is the length of the word in bits in which the value resides.
   START is the starting bit number in the word, architecture origin.
   LENGTH is the length of VALUE in bits.
   TOTAL_LENGTH is the total length of the insn in bits.

   Returns 1 for success, 0 for failure.  */

/* ??? The return code isn't properly used.  wip.  */

/* ??? This doesn't handle bfd_vma's.  Create another function when
   necessary.  */

static int
extract_normal (CGEN_CPU_DESC cd,
#if ! CGEN_INT_INSN_P
		CGEN_EXTRACT_INFO *ex_info,
#else
		CGEN_EXTRACT_INFO *ex_info ATTRIBUTE_UNUSED,
#endif
		CGEN_INSN_INT insn_value,
		unsigned int attrs,
		unsigned int word_offset,
		unsigned int start,
		unsigned int length,
		unsigned int word_length,
		unsigned int total_length,
#if ! CGEN_INT_INSN_P
		bfd_vma pc,
#else
		bfd_vma pc ATTRIBUTE_UNUSED,
#endif
		long *valuep)
{
  long value, mask;

  /* If LENGTH is zero, this operand doesn't contribute to the value
     so give it a standard value of zero.  */
  if (length == 0)
    {
      *valuep = 0;
      return 1;
    }

  if (word_length > 8 * sizeof (CGEN_INSN_INT))
    abort ();

  /* For architectures with insns smaller than the insn-base-bitsize,
     word_length may be too big.  */
  if (cd->min_insn_bitsize < cd->base_insn_bitsize)
    {
      if (word_offset + word_length > total_length)
	word_length = total_length - word_offset;
    }

  /* Does the value reside in INSN_VALUE, and at the right alignment?  */

  if (CGEN_INT_INSN_P || (word_offset == 0 && word_length == total_length))
    {
      if (CGEN_INSN_LSB0_P)
	value = insn_value >> ((word_offset + start + 1) - length);
      else
	value = insn_value >> (total_length - ( word_offset + start + length));
    }

#if ! CGEN_INT_INSN_P

  else
    {
      unsigned char *bufp = ex_info->insn_bytes + word_offset / 8;

      if (word_length > 8 * sizeof (CGEN_INSN_INT))
	abort ();

      if (fill_cache (cd, ex_info, word_offset / 8, word_length / 8, pc) == 0)
	{
	  *valuep = 0;
	  return 0;
	}

      value = extract_1 (cd, ex_info, start, length, word_length, bufp, pc);
    }

#endif /* ! CGEN_INT_INSN_P */

  /* Written this way to avoid undefined behaviour.  */
  mask = (1UL << (length - 1) << 1) - 1;

  value &= mask;
  /* sign extend? */
  if (CGEN_BOOL_ATTR (attrs, CGEN_IFLD_SIGNED)
      && (value & (1UL << (length - 1))))
    value |= ~mask;

  *valuep = value;

  return 1;
}

/* Default insn extractor.

   INSN_VALUE is the first base_insn_bitsize bits, translated to host order.
   The extracted fields are stored in FIELDS.
   EX_INFO is used to handle reading variable length insns.
   Return the length of the insn in bits, or 0 if no match,
   or -1 if an error occurs fetching data (memory_error_func will have
   been called).  */

static int
extract_insn_normal (CGEN_CPU_DESC cd,
		     const CGEN_INSN *insn,
		     CGEN_EXTRACT_INFO *ex_info,
		     CGEN_INSN_INT insn_value,
		     CGEN_FIELDS *fields,
		     bfd_vma pc)
{
  const CGEN_SYNTAX *syntax = CGEN_INSN_SYNTAX (insn);
  const CGEN_SYNTAX_CHAR_TYPE *syn;

  CGEN_FIELDS_BITSIZE (fields) = CGEN_INSN_BITSIZE (insn);

  CGEN_INIT_EXTRACT (cd);

  for (syn = CGEN_SYNTAX_STRING (syntax); *syn; ++syn)
    {
      int length;

      if (CGEN_SYNTAX_CHAR_P (*syn))
	continue;

      length = (* cd->extract_operand) (cd, CGEN_SYNTAX_FIELD (*syn),
					ex_info, insn_value, fields, pc);
      if (length <= 0)
	return length;
    }

  /* We recognized and successfully extracted this insn.  */
  return CGEN_INSN_BITSIZE (insn);
}

/* Machine generated code added here.  */

const char * epiphany_cgen_insert_operand
  (CGEN_CPU_DESC, int, CGEN_FIELDS *, CGEN_INSN_BYTES_PTR, bfd_vma);

/* Main entry point for operand insertion.

   This function is basically just a big switch statement.  Earlier versions
   used tables to look up the function to use, but
   - if the table contains both assembler and disassembler functions then
     the disassembler contains much of the assembler and vice-versa,
   - there's a lot of inlining possibilities as things grow,
   - using a switch statement avoids the function call overhead.

   This function could be moved into `parse_insn_normal', but keeping it
   separate makes clear the interface between `parse_insn_normal' and each of
   the handlers.  It's also needed by GAS to insert operands that couldn't be
   resolved during parsing.  */

const char *
epiphany_cgen_insert_operand (CGEN_CPU_DESC cd,
			     int opindex,
			     CGEN_FIELDS * fields,
			     CGEN_INSN_BYTES_PTR buffer,
			     bfd_vma pc ATTRIBUTE_UNUSED)
{
  const char * errmsg = NULL;
  unsigned int total_length = CGEN_FIELDS_BITSIZE (fields);

  switch (opindex)
    {
    case EPIPHANY_OPERAND_DIRECTION :
      errmsg = insert_normal (cd, fields->f_addsubx, 0, 0, 20, 1, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_DISP11 :
      {
{
  FLD (f_disp8) = ((((UINT) (FLD (f_disp11)) >> (3))) & (255));
  FLD (f_disp3) = ((FLD (f_disp11)) & (7));
}
        errmsg = insert_normal (cd, fields->f_disp3, 0, 0, 9, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_disp8, 0, 0, 23, 8, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_DISP3 :
      errmsg = insert_normal (cd, fields->f_disp3, 0, 0, 9, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_DPMI :
      errmsg = insert_normal (cd, fields->f_subd, 0, 0, 24, 1, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_FRD :
      errmsg = insert_normal (cd, fields->f_rd, 0, 0, 15, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_FRD6 :
      {
{
  FLD (f_rd) = ((FLD (f_rd6)) & (7));
  FLD (f_rd_x) = ((UINT) (FLD (f_rd6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_rd_x, 0, 0, 31, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_rd, 0, 0, 15, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_FRM :
      errmsg = insert_normal (cd, fields->f_rm, 0, 0, 9, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_FRM6 :
      {
{
  FLD (f_rm) = ((FLD (f_rm6)) & (7));
  FLD (f_rm_x) = ((UINT) (FLD (f_rm6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_rm_x, 0, 0, 25, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_rm, 0, 0, 9, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_FRN :
      errmsg = insert_normal (cd, fields->f_rn, 0, 0, 12, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_FRN6 :
      {
{
  FLD (f_rn) = ((FLD (f_rn6)) & (7));
  FLD (f_rn_x) = ((UINT) (FLD (f_rn6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_rn_x, 0, 0, 28, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_rn, 0, 0, 12, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_IMM16 :
      {
{
  FLD (f_imm8) = ((FLD (f_imm16)) & (255));
  FLD (f_imm_27_8) = ((UINT) (FLD (f_imm16)) >> (8));
}
        errmsg = insert_normal (cd, fields->f_imm8, 0, 0, 12, 8, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_imm_27_8, 0, 0, 27, 8, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_IMM8 :
      errmsg = insert_normal (cd, fields->f_imm8, 0, 0, 12, 8, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_RD :
      errmsg = insert_normal (cd, fields->f_rd, 0, 0, 15, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_RD6 :
      {
{
  FLD (f_rd) = ((FLD (f_rd6)) & (7));
  FLD (f_rd_x) = ((UINT) (FLD (f_rd6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_rd_x, 0, 0, 31, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_rd, 0, 0, 15, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_RM :
      errmsg = insert_normal (cd, fields->f_rm, 0, 0, 9, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_RM6 :
      {
{
  FLD (f_rm) = ((FLD (f_rm6)) & (7));
  FLD (f_rm_x) = ((UINT) (FLD (f_rm6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_rm_x, 0, 0, 25, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_rm, 0, 0, 9, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_RN :
      errmsg = insert_normal (cd, fields->f_rn, 0, 0, 12, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_RN6 :
      {
{
  FLD (f_rn) = ((FLD (f_rn6)) & (7));
  FLD (f_rn_x) = ((UINT) (FLD (f_rn6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_rn_x, 0, 0, 28, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_rn, 0, 0, 12, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SD :
      errmsg = insert_normal (cd, fields->f_sd, 0, 0, 15, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_SD6 :
      {
{
  FLD (f_sd) = ((FLD (f_sd6)) & (7));
  FLD (f_sd_x) = ((UINT) (FLD (f_sd6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_sd_x, 0, 0, 31, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_sd, 0, 0, 15, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SDDMA :
      {
{
  FLD (f_sd) = ((FLD (f_sd6)) & (7));
  FLD (f_sd_x) = ((UINT) (FLD (f_sd6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_sd_x, 0, 0, 31, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_sd, 0, 0, 15, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SDMEM :
      {
{
  FLD (f_sd) = ((FLD (f_sd6)) & (7));
  FLD (f_sd_x) = ((UINT) (FLD (f_sd6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_sd_x, 0, 0, 31, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_sd, 0, 0, 15, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SDMESH :
      {
{
  FLD (f_sd) = ((FLD (f_sd6)) & (7));
  FLD (f_sd_x) = ((UINT) (FLD (f_sd6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_sd_x, 0, 0, 31, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_sd, 0, 0, 15, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SHIFT :
      errmsg = insert_normal (cd, fields->f_shift, 0, 0, 9, 5, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_SIMM11 :
      {
{
  FLD (f_disp8) = ((255) & (((USI) (FLD (f_sdisp11)) >> (3))));
  FLD (f_disp3) = ((FLD (f_sdisp11)) & (7));
}
        errmsg = insert_normal (cd, fields->f_disp3, 0, 0, 9, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_disp8, 0, 0, 23, 8, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SIMM24 :
      {
        long value = fields->f_simm24;
        value = ((SI) (((value) - (pc))) >> (1));
        errmsg = insert_normal (cd, value, 0|(1<<CGEN_IFLD_SIGNED)|(1<<CGEN_IFLD_RELOC)|(1<<CGEN_IFLD_PCREL_ADDR), 0, 31, 24, 32, total_length, buffer);
      }
      break;
    case EPIPHANY_OPERAND_SIMM3 :
      errmsg = insert_normal (cd, fields->f_sdisp3, 0|(1<<CGEN_IFLD_SIGNED), 0, 9, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_SIMM8 :
      {
        long value = fields->f_simm8;
        value = ((SI) (((value) - (pc))) >> (1));
        errmsg = insert_normal (cd, value, 0|(1<<CGEN_IFLD_SIGNED)|(1<<CGEN_IFLD_RELOC)|(1<<CGEN_IFLD_PCREL_ADDR), 0, 15, 8, 32, total_length, buffer);
      }
      break;
    case EPIPHANY_OPERAND_SN :
      errmsg = insert_normal (cd, fields->f_sn, 0, 0, 12, 3, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_SN6 :
      {
{
  FLD (f_sn) = ((FLD (f_sn6)) & (7));
  FLD (f_sn_x) = ((UINT) (FLD (f_sn6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_sn_x, 0, 0, 28, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_sn, 0, 0, 12, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SNDMA :
      {
{
  FLD (f_sn) = ((FLD (f_sn6)) & (7));
  FLD (f_sn_x) = ((UINT) (FLD (f_sn6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_sn_x, 0, 0, 28, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_sn, 0, 0, 12, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SNMEM :
      {
{
  FLD (f_sn) = ((FLD (f_sn6)) & (7));
  FLD (f_sn_x) = ((UINT) (FLD (f_sn6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_sn_x, 0, 0, 28, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_sn, 0, 0, 12, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SNMESH :
      {
{
  FLD (f_sn) = ((FLD (f_sn6)) & (7));
  FLD (f_sn_x) = ((UINT) (FLD (f_sn6)) >> (3));
}
        errmsg = insert_normal (cd, fields->f_sn_x, 0, 0, 28, 3, 32, total_length, buffer);
        if (errmsg)
          break;
        errmsg = insert_normal (cd, fields->f_sn, 0, 0, 12, 3, 32, total_length, buffer);
        if (errmsg)
          break;
      }
      break;
    case EPIPHANY_OPERAND_SWI_NUM :
      errmsg = insert_normal (cd, fields->f_trap_num, 0, 0, 15, 6, 32, total_length, buffer);
      break;
    case EPIPHANY_OPERAND_TRAPNUM6 :
      errmsg = insert_normal (cd, fields->f_trap_num, 0, 0, 15, 6, 32, total_length, buffer);
      break;

    default :
      /* xgettext:c-format */
      opcodes_error_handler
	(_("internal error: unrecognized field %d while building insn"),
	 opindex);
      abort ();
  }

  return errmsg;
}

int epiphany_cgen_extract_operand
  (CGEN_CPU_DESC, int, CGEN_EXTRACT_INFO *, CGEN_INSN_INT, CGEN_FIELDS *, bfd_vma);

/* Main entry point for operand extraction.
   The result is <= 0 for error, >0 for success.
   ??? Actual values aren't well defined right now.

   This function is basically just a big switch statement.  Earlier versions
   used tables to look up the function to use, but
   - if the table contains both assembler and disassembler functions then
     the disassembler contains much of the assembler and vice-versa,
   - there's a lot of inlining possibilities as things grow,
   - using a switch statement avoids the function call overhead.

   This function could be moved into `print_insn_normal', but keeping it
   separate makes clear the interface between `print_insn_normal' and each of
   the handlers.  */

int
epiphany_cgen_extract_operand (CGEN_CPU_DESC cd,
			     int opindex,
			     CGEN_EXTRACT_INFO *ex_info,
			     CGEN_INSN_INT insn_value,
			     CGEN_FIELDS * fields,
			     bfd_vma pc)
{
  /* Assume success (for those operands that are nops).  */
  int length = 1;
  unsigned int total_length = CGEN_FIELDS_BITSIZE (fields);

  switch (opindex)
    {
    case EPIPHANY_OPERAND_DIRECTION :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 20, 1, 32, total_length, pc, & fields->f_addsubx);
      break;
    case EPIPHANY_OPERAND_DISP11 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 9, 3, 32, total_length, pc, & fields->f_disp3);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 23, 8, 32, total_length, pc, & fields->f_disp8);
        if (length <= 0) break;
{
  FLD (f_disp11) = ((((FLD (f_disp8)) << (3))) | (FLD (f_disp3)));
}
      }
      break;
    case EPIPHANY_OPERAND_DISP3 :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 9, 3, 32, total_length, pc, & fields->f_disp3);
      break;
    case EPIPHANY_OPERAND_DPMI :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 24, 1, 32, total_length, pc, & fields->f_subd);
      break;
    case EPIPHANY_OPERAND_FRD :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_rd);
      break;
    case EPIPHANY_OPERAND_FRD6 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 31, 3, 32, total_length, pc, & fields->f_rd_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_rd);
        if (length <= 0) break;
{
  FLD (f_rd6) = ((((FLD (f_rd_x)) << (3))) | (FLD (f_rd)));
}
      }
      break;
    case EPIPHANY_OPERAND_FRM :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 9, 3, 32, total_length, pc, & fields->f_rm);
      break;
    case EPIPHANY_OPERAND_FRM6 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 25, 3, 32, total_length, pc, & fields->f_rm_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 9, 3, 32, total_length, pc, & fields->f_rm);
        if (length <= 0) break;
{
  FLD (f_rm6) = ((((FLD (f_rm_x)) << (3))) | (FLD (f_rm)));
}
      }
      break;
    case EPIPHANY_OPERAND_FRN :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_rn);
      break;
    case EPIPHANY_OPERAND_FRN6 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 28, 3, 32, total_length, pc, & fields->f_rn_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_rn);
        if (length <= 0) break;
{
  FLD (f_rn6) = ((((FLD (f_rn_x)) << (3))) | (FLD (f_rn)));
}
      }
      break;
    case EPIPHANY_OPERAND_IMM16 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 8, 32, total_length, pc, & fields->f_imm8);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 27, 8, 32, total_length, pc, & fields->f_imm_27_8);
        if (length <= 0) break;
{
  FLD (f_imm16) = ((((FLD (f_imm_27_8)) << (8))) | (FLD (f_imm8)));
}
      }
      break;
    case EPIPHANY_OPERAND_IMM8 :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 8, 32, total_length, pc, & fields->f_imm8);
      break;
    case EPIPHANY_OPERAND_RD :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_rd);
      break;
    case EPIPHANY_OPERAND_RD6 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 31, 3, 32, total_length, pc, & fields->f_rd_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_rd);
        if (length <= 0) break;
{
  FLD (f_rd6) = ((((FLD (f_rd_x)) << (3))) | (FLD (f_rd)));
}
      }
      break;
    case EPIPHANY_OPERAND_RM :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 9, 3, 32, total_length, pc, & fields->f_rm);
      break;
    case EPIPHANY_OPERAND_RM6 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 25, 3, 32, total_length, pc, & fields->f_rm_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 9, 3, 32, total_length, pc, & fields->f_rm);
        if (length <= 0) break;
{
  FLD (f_rm6) = ((((FLD (f_rm_x)) << (3))) | (FLD (f_rm)));
}
      }
      break;
    case EPIPHANY_OPERAND_RN :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_rn);
      break;
    case EPIPHANY_OPERAND_RN6 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 28, 3, 32, total_length, pc, & fields->f_rn_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_rn);
        if (length <= 0) break;
{
  FLD (f_rn6) = ((((FLD (f_rn_x)) << (3))) | (FLD (f_rn)));
}
      }
      break;
    case EPIPHANY_OPERAND_SD :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_sd);
      break;
    case EPIPHANY_OPERAND_SD6 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 31, 3, 32, total_length, pc, & fields->f_sd_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_sd);
        if (length <= 0) break;
{
  FLD (f_sd6) = ((((FLD (f_sd_x)) << (3))) | (FLD (f_sd)));
}
      }
      break;
    case EPIPHANY_OPERAND_SDDMA :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 31, 3, 32, total_length, pc, & fields->f_sd_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_sd);
        if (length <= 0) break;
{
  FLD (f_sd6) = ((((FLD (f_sd_x)) << (3))) | (FLD (f_sd)));
}
      }
      break;
    case EPIPHANY_OPERAND_SDMEM :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 31, 3, 32, total_length, pc, & fields->f_sd_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_sd);
        if (length <= 0) break;
{
  FLD (f_sd6) = ((((FLD (f_sd_x)) << (3))) | (FLD (f_sd)));
}
      }
      break;
    case EPIPHANY_OPERAND_SDMESH :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 31, 3, 32, total_length, pc, & fields->f_sd_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 3, 32, total_length, pc, & fields->f_sd);
        if (length <= 0) break;
{
  FLD (f_sd6) = ((((FLD (f_sd_x)) << (3))) | (FLD (f_sd)));
}
      }
      break;
    case EPIPHANY_OPERAND_SHIFT :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 9, 5, 32, total_length, pc, & fields->f_shift);
      break;
    case EPIPHANY_OPERAND_SIMM11 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 9, 3, 32, total_length, pc, & fields->f_disp3);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 23, 8, 32, total_length, pc, & fields->f_disp8);
        if (length <= 0) break;
{
  FLD (f_sdisp11) = ((((((((((FLD (f_disp8)) << (3))) | (FLD (f_disp3)))) & (2047))) ^ (1024))) - (1024));
}
      }
      break;
    case EPIPHANY_OPERAND_SIMM24 :
      {
        long value;
        length = extract_normal (cd, ex_info, insn_value, 0|(1<<CGEN_IFLD_SIGNED)|(1<<CGEN_IFLD_RELOC)|(1<<CGEN_IFLD_PCREL_ADDR), 0, 31, 24, 32, total_length, pc, & value);
        value = ((((value) * (2))) + (pc));
        fields->f_simm24 = value;
      }
      break;
    case EPIPHANY_OPERAND_SIMM3 :
      length = extract_normal (cd, ex_info, insn_value, 0|(1<<CGEN_IFLD_SIGNED), 0, 9, 3, 32, total_length, pc, & fields->f_sdisp3);
      break;
    case EPIPHANY_OPERAND_SIMM8 :
      {
        long value;
        length = extract_normal (cd, ex_info, insn_value, 0|(1<<CGEN_IFLD_SIGNED)|(1<<CGEN_IFLD_RELOC)|(1<<CGEN_IFLD_PCREL_ADDR), 0, 15, 8, 32, total_length, pc, & value);
        value = ((((value) * (2))) + (pc));
        fields->f_simm8 = value;
      }
      break;
    case EPIPHANY_OPERAND_SN :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_sn);
      break;
    case EPIPHANY_OPERAND_SN6 :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 28, 3, 32, total_length, pc, & fields->f_sn_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_sn);
        if (length <= 0) break;
{
  FLD (f_sn6) = ((((FLD (f_sn_x)) << (3))) | (FLD (f_sn)));
}
      }
      break;
    case EPIPHANY_OPERAND_SNDMA :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 28, 3, 32, total_length, pc, & fields->f_sn_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_sn);
        if (length <= 0) break;
{
  FLD (f_sn6) = ((((FLD (f_sn_x)) << (3))) | (FLD (f_sn)));
}
      }
      break;
    case EPIPHANY_OPERAND_SNMEM :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 28, 3, 32, total_length, pc, & fields->f_sn_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_sn);
        if (length <= 0) break;
{
  FLD (f_sn6) = ((((FLD (f_sn_x)) << (3))) | (FLD (f_sn)));
}
      }
      break;
    case EPIPHANY_OPERAND_SNMESH :
      {
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 28, 3, 32, total_length, pc, & fields->f_sn_x);
        if (length <= 0) break;
        length = extract_normal (cd, ex_info, insn_value, 0, 0, 12, 3, 32, total_length, pc, & fields->f_sn);
        if (length <= 0) break;
{
  FLD (f_sn6) = ((((FLD (f_sn_x)) << (3))) | (FLD (f_sn)));
}
      }
      break;
    case EPIPHANY_OPERAND_SWI_NUM :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 6, 32, total_length, pc, & fields->f_trap_num);
      break;
    case EPIPHANY_OPERAND_TRAPNUM6 :
      length = extract_normal (cd, ex_info, insn_value, 0, 0, 15, 6, 32, total_length, pc, & fields->f_trap_num);
      break;

    default :
      /* xgettext:c-format */
      opcodes_error_handler
	(_("internal error: unrecognized field %d while decoding insn"),
	 opindex);
      abort ();
    }

  return length;
}

cgen_insert_fn * const epiphany_cgen_insert_handlers[] =
{
  insert_insn_normal,
};

cgen_extract_fn * const epiphany_cgen_extract_handlers[] =
{
  extract_insn_normal,
};

int epiphany_cgen_get_int_operand     (CGEN_CPU_DESC, int, const CGEN_FIELDS *);
bfd_vma epiphany_cgen_get_vma_operand (CGEN_CPU_DESC, int, const CGEN_FIELDS *);

/* Getting values from cgen_fields is handled by a collection of functions.
   They are distinguished by the type of the VALUE argument they return.
   TODO: floating point, inlining support, remove cases where result type
   not appropriate.  */

int
epiphany_cgen_get_int_operand (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
			     int opindex,
			     const CGEN_FIELDS * fields)
{
  int value;

  switch (opindex)
    {
    case EPIPHANY_OPERAND_DIRECTION :
      value = fields->f_addsubx;
      break;
    case EPIPHANY_OPERAND_DISP11 :
      value = fields->f_disp11;
      break;
    case EPIPHANY_OPERAND_DISP3 :
      value = fields->f_disp3;
      break;
    case EPIPHANY_OPERAND_DPMI :
      value = fields->f_subd;
      break;
    case EPIPHANY_OPERAND_FRD :
      value = fields->f_rd;
      break;
    case EPIPHANY_OPERAND_FRD6 :
      value = fields->f_rd6;
      break;
    case EPIPHANY_OPERAND_FRM :
      value = fields->f_rm;
      break;
    case EPIPHANY_OPERAND_FRM6 :
      value = fields->f_rm6;
      break;
    case EPIPHANY_OPERAND_FRN :
      value = fields->f_rn;
      break;
    case EPIPHANY_OPERAND_FRN6 :
      value = fields->f_rn6;
      break;
    case EPIPHANY_OPERAND_IMM16 :
      value = fields->f_imm16;
      break;
    case EPIPHANY_OPERAND_IMM8 :
      value = fields->f_imm8;
      break;
    case EPIPHANY_OPERAND_RD :
      value = fields->f_rd;
      break;
    case EPIPHANY_OPERAND_RD6 :
      value = fields->f_rd6;
      break;
    case EPIPHANY_OPERAND_RM :
      value = fields->f_rm;
      break;
    case EPIPHANY_OPERAND_RM6 :
      value = fields->f_rm6;
      break;
    case EPIPHANY_OPERAND_RN :
      value = fields->f_rn;
      break;
    case EPIPHANY_OPERAND_RN6 :
      value = fields->f_rn6;
      break;
    case EPIPHANY_OPERAND_SD :
      value = fields->f_sd;
      break;
    case EPIPHANY_OPERAND_SD6 :
      value = fields->f_sd6;
      break;
    case EPIPHANY_OPERAND_SDDMA :
      value = fields->f_sd6;
      break;
    case EPIPHANY_OPERAND_SDMEM :
      value = fields->f_sd6;
      break;
    case EPIPHANY_OPERAND_SDMESH :
      value = fields->f_sd6;
      break;
    case EPIPHANY_OPERAND_SHIFT :
      value = fields->f_shift;
      break;
    case EPIPHANY_OPERAND_SIMM11 :
      value = fields->f_sdisp11;
      break;
    case EPIPHANY_OPERAND_SIMM24 :
      value = fields->f_simm24;
      break;
    case EPIPHANY_OPERAND_SIMM3 :
      value = fields->f_sdisp3;
      break;
    case EPIPHANY_OPERAND_SIMM8 :
      value = fields->f_simm8;
      break;
    case EPIPHANY_OPERAND_SN :
      value = fields->f_sn;
      break;
    case EPIPHANY_OPERAND_SN6 :
      value = fields->f_sn6;
      break;
    case EPIPHANY_OPERAND_SNDMA :
      value = fields->f_sn6;
      break;
    case EPIPHANY_OPERAND_SNMEM :
      value = fields->f_sn6;
      break;
    case EPIPHANY_OPERAND_SNMESH :
      value = fields->f_sn6;
      break;
    case EPIPHANY_OPERAND_SWI_NUM :
      value = fields->f_trap_num;
      break;
    case EPIPHANY_OPERAND_TRAPNUM6 :
      value = fields->f_trap_num;
      break;

    default :
      /* xgettext:c-format */
      opcodes_error_handler
	(_("internal error: unrecognized field %d while getting int operand"),
	 opindex);
      abort ();
  }

  return value;
}

bfd_vma
epiphany_cgen_get_vma_operand (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
			     int opindex,
			     const CGEN_FIELDS * fields)
{
  bfd_vma value;

  switch (opindex)
    {
    case EPIPHANY_OPERAND_DIRECTION :
      value = fields->f_addsubx;
      break;
    case EPIPHANY_OPERAND_DISP11 :
      value = fields->f_disp11;
      break;
    case EPIPHANY_OPERAND_DISP3 :
      value = fields->f_disp3;
      break;
    case EPIPHANY_OPERAND_DPMI :
      value = fields->f_subd;
      break;
    case EPIPHANY_OPERAND_FRD :
      value = fields->f_rd;
      break;
    case EPIPHANY_OPERAND_FRD6 :
      value = fields->f_rd6;
      break;
    case EPIPHANY_OPERAND_FRM :
      value = fields->f_rm;
      break;
    case EPIPHANY_OPERAND_FRM6 :
      value = fields->f_rm6;
      break;
    case EPIPHANY_OPERAND_FRN :
      value = fields->f_rn;
      break;
    case EPIPHANY_OPERAND_FRN6 :
      value = fields->f_rn6;
      break;
    case EPIPHANY_OPERAND_IMM16 :
      value = fields->f_imm16;
      break;
    case EPIPHANY_OPERAND_IMM8 :
      value = fields->f_imm8;
      break;
    case EPIPHANY_OPERAND_RD :
      value = fields->f_rd;
      break;
    case EPIPHANY_OPERAND_RD6 :
      value = fields->f_rd6;
      break;
    case EPIPHANY_OPERAND_RM :
      value = fields->f_rm;
      break;
    case EPIPHANY_OPERAND_RM6 :
      value = fields->f_rm6;
      break;
    case EPIPHANY_OPERAND_RN :
      value = fields->f_rn;
      break;
    case EPIPHANY_OPERAND_RN6 :
      value = fields->f_rn6;
      break;
    case EPIPHANY_OPERAND_SD :
      value = fields->f_sd;
      break;
    case EPIPHANY_OPERAND_SD6 :
      value = fields->f_sd6;
      break;
    case EPIPHANY_OPERAND_SDDMA :
      value = fields->f_sd6;
      break;
    case EPIPHANY_OPERAND_SDMEM :
      value = fields->f_sd6;
      break;
    case EPIPHANY_OPERAND_SDMESH :
      value = fields->f_sd6;
      break;
    case EPIPHANY_OPERAND_SHIFT :
      value = fields->f_shift;
      break;
    case EPIPHANY_OPERAND_SIMM11 :
      value = fields->f_sdisp11;
      break;
    case EPIPHANY_OPERAND_SIMM24 :
      value = fields->f_simm24;
      break;
    case EPIPHANY_OPERAND_SIMM3 :
      value = fields->f_sdisp3;
      break;
    case EPIPHANY_OPERAND_SIMM8 :
      value = fields->f_simm8;
      break;
    case EPIPHANY_OPERAND_SN :
      value = fields->f_sn;
      break;
    case EPIPHANY_OPERAND_SN6 :
      value = fields->f_sn6;
      break;
    case EPIPHANY_OPERAND_SNDMA :
      value = fields->f_sn6;
      break;
    case EPIPHANY_OPERAND_SNMEM :
      value = fields->f_sn6;
      break;
    case EPIPHANY_OPERAND_SNMESH :
      value = fields->f_sn6;
      break;
    case EPIPHANY_OPERAND_SWI_NUM :
      value = fields->f_trap_num;
      break;
    case EPIPHANY_OPERAND_TRAPNUM6 :
      value = fields->f_trap_num;
      break;

    default :
      /* xgettext:c-format */
      opcodes_error_handler
	(_("internal error: unrecognized field %d while getting vma operand"),
	 opindex);
      abort ();
  }

  return value;
}

void epiphany_cgen_set_int_operand  (CGEN_CPU_DESC, int, CGEN_FIELDS *, int);
void epiphany_cgen_set_vma_operand  (CGEN_CPU_DESC, int, CGEN_FIELDS *, bfd_vma);

/* Stuffing values in cgen_fields is handled by a collection of functions.
   They are distinguished by the type of the VALUE argument they accept.
   TODO: floating point, inlining support, remove cases where argument type
   not appropriate.  */

void
epiphany_cgen_set_int_operand (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
			     int opindex,
			     CGEN_FIELDS * fields,
			     int value)
{
  switch (opindex)
    {
    case EPIPHANY_OPERAND_DIRECTION :
      fields->f_addsubx = value;
      break;
    case EPIPHANY_OPERAND_DISP11 :
      fields->f_disp11 = value;
      break;
    case EPIPHANY_OPERAND_DISP3 :
      fields->f_disp3 = value;
      break;
    case EPIPHANY_OPERAND_DPMI :
      fields->f_subd = value;
      break;
    case EPIPHANY_OPERAND_FRD :
      fields->f_rd = value;
      break;
    case EPIPHANY_OPERAND_FRD6 :
      fields->f_rd6 = value;
      break;
    case EPIPHANY_OPERAND_FRM :
      fields->f_rm = value;
      break;
    case EPIPHANY_OPERAND_FRM6 :
      fields->f_rm6 = value;
      break;
    case EPIPHANY_OPERAND_FRN :
      fields->f_rn = value;
      break;
    case EPIPHANY_OPERAND_FRN6 :
      fields->f_rn6 = value;
      break;
    case EPIPHANY_OPERAND_IMM16 :
      fields->f_imm16 = value;
      break;
    case EPIPHANY_OPERAND_IMM8 :
      fields->f_imm8 = value;
      break;
    case EPIPHANY_OPERAND_RD :
      fields->f_rd = value;
      break;
    case EPIPHANY_OPERAND_RD6 :
      fields->f_rd6 = value;
      break;
    case EPIPHANY_OPERAND_RM :
      fields->f_rm = value;
      break;
    case EPIPHANY_OPERAND_RM6 :
      fields->f_rm6 = value;
      break;
    case EPIPHANY_OPERAND_RN :
      fields->f_rn = value;
      break;
    case EPIPHANY_OPERAND_RN6 :
      fields->f_rn6 = value;
      break;
    case EPIPHANY_OPERAND_SD :
      fields->f_sd = value;
      break;
    case EPIPHANY_OPERAND_SD6 :
      fields->f_sd6 = value;
      break;
    case EPIPHANY_OPERAND_SDDMA :
      fields->f_sd6 = value;
      break;
    case EPIPHANY_OPERAND_SDMEM :
      fields->f_sd6 = value;
      break;
    case EPIPHANY_OPERAND_SDMESH :
      fields->f_sd6 = value;
      break;
    case EPIPHANY_OPERAND_SHIFT :
      fields->f_shift = value;
      break;
    case EPIPHANY_OPERAND_SIMM11 :
      fields->f_sdisp11 = value;
      break;
    case EPIPHANY_OPERAND_SIMM24 :
      fields->f_simm24 = value;
      break;
    case EPIPHANY_OPERAND_SIMM3 :
      fields->f_sdisp3 = value;
      break;
    case EPIPHANY_OPERAND_SIMM8 :
      fields->f_simm8 = value;
      break;
    case EPIPHANY_OPERAND_SN :
      fields->f_sn = value;
      break;
    case EPIPHANY_OPERAND_SN6 :
      fields->f_sn6 = value;
      break;
    case EPIPHANY_OPERAND_SNDMA :
      fields->f_sn6 = value;
      break;
    case EPIPHANY_OPERAND_SNMEM :
      fields->f_sn6 = value;
      break;
    case EPIPHANY_OPERAND_SNMESH :
      fields->f_sn6 = value;
      break;
    case EPIPHANY_OPERAND_SWI_NUM :
      fields->f_trap_num = value;
      break;
    case EPIPHANY_OPERAND_TRAPNUM6 :
      fields->f_trap_num = value;
      break;

    default :
      /* xgettext:c-format */
      opcodes_error_handler
	(_("internal error: unrecognized field %d while setting int operand"),
	 opindex);
      abort ();
  }
}

void
epiphany_cgen_set_vma_operand (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
			     int opindex,
			     CGEN_FIELDS * fields,
			     bfd_vma value)
{
  switch (opindex)
    {
    case EPIPHANY_OPERAND_DIRECTION :
      fields->f_addsubx = value;
      break;
    case EPIPHANY_OPERAND_DISP11 :
      fields->f_disp11 = value;
      break;
    case EPIPHANY_OPERAND_DISP3 :
      fields->f_disp3 = value;
      break;
    case EPIPHANY_OPERAND_DPMI :
      fields->f_subd = value;
      break;
    case EPIPHANY_OPERAND_FRD :
      fields->f_rd = value;
      break;
    case EPIPHANY_OPERAND_FRD6 :
      fields->f_rd6 = value;
      break;
    case EPIPHANY_OPERAND_FRM :
      fields->f_rm = value;
      break;
    case EPIPHANY_OPERAND_FRM6 :
      fields->f_rm6 = value;
      break;
    case EPIPHANY_OPERAND_FRN :
      fields->f_rn = value;
      break;
    case EPIPHANY_OPERAND_FRN6 :
      fields->f_rn6 = value;
      break;
    case EPIPHANY_OPERAND_IMM16 :
      fields->f_imm16 = value;
      break;
    case EPIPHANY_OPERAND_IMM8 :
      fields->f_imm8 = value;
      break;
    case EPIPHANY_OPERAND_RD :
      fields->f_rd = value;
      break;
    case EPIPHANY_OPERAND_RD6 :
      fields->f_rd6 = value;
      break;
    case EPIPHANY_OPERAND_RM :
      fields->f_rm = value;
      break;
    case EPIPHANY_OPERAND_RM6 :
      fields->f_rm6 = value;
      break;
    case EPIPHANY_OPERAND_RN :
      fields->f_rn = value;
      break;
    case EPIPHANY_OPERAND_RN6 :
      fields->f_rn6 = value;
      break;
    case EPIPHANY_OPERAND_SD :
      fields->f_sd = value;
      break;
    case EPIPHANY_OPERAND_SD6 :
      fields->f_sd6 = value;
      break;
    case EPIPHANY_OPERAND_SDDMA :
      fields->f_sd6 = value;
      break;
    case EPIPHANY_OPERAND_SDMEM :
      fields->f_sd6 = value;
      break;
    case EPIPHANY_OPERAND_SDMESH :
      fields->f_sd6 = value;
      break;
    case EPIPHANY_OPERAND_SHIFT :
      fields->f_shift = value;
      break;
    case EPIPHANY_OPERAND_SIMM11 :
      fields->f_sdisp11 = value;
      break;
    case EPIPHANY_OPERAND_SIMM24 :
      fields->f_simm24 = value;
      break;
    case EPIPHANY_OPERAND_SIMM3 :
      fields->f_sdisp3 = value;
      break;
    case EPIPHANY_OPERAND_SIMM8 :
      fields->f_simm8 = value;
      break;
    case EPIPHANY_OPERAND_SN :
      fields->f_sn = value;
      break;
    case EPIPHANY_OPERAND_SN6 :
      fields->f_sn6 = value;
      break;
    case EPIPHANY_OPERAND_SNDMA :
      fields->f_sn6 = value;
      break;
    case EPIPHANY_OPERAND_SNMEM :
      fields->f_sn6 = value;
      break;
    case EPIPHANY_OPERAND_SNMESH :
      fields->f_sn6 = value;
      break;
    case EPIPHANY_OPERAND_SWI_NUM :
      fields->f_trap_num = value;
      break;
    case EPIPHANY_OPERAND_TRAPNUM6 :
      fields->f_trap_num = value;
      break;

    default :
      /* xgettext:c-format */
      opcodes_error_handler
	(_("internal error: unrecognized field %d while setting vma operand"),
	 opindex);
      abort ();
  }
}

/* Function to call before using the instruction builder tables.  */

void
epiphany_cgen_init_ibld_table (CGEN_CPU_DESC cd)
{
  cd->insert_handlers = & epiphany_cgen_insert_handlers[0];
  cd->extract_handlers = & epiphany_cgen_extract_handlers[0];

  cd->insert_operand = epiphany_cgen_insert_operand;
  cd->extract_operand = epiphany_cgen_extract_operand;

  cd->get_int_operand = epiphany_cgen_get_int_operand;
  cd->set_int_operand = epiphany_cgen_set_int_operand;
  cd->get_vma_operand = epiphany_cgen_get_vma_operand;
  cd->set_vma_operand = epiphany_cgen_set_vma_operand;
}
