/* frags.c - manage frags -
   Copyright (C) 1987-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "subsegs.h"
#include "obstack.h"

extern fragS zero_address_frag;
extern fragS predefined_address_frag;

static int totalfrags;

int
get_frag_count (void)
{
  return totalfrags;
}

void
clear_frag_count (void)
{
  totalfrags = 0;
}

/* Initialization for frag routines.  */

void
frag_init (void)
{
  zero_address_frag.fr_type = rs_fill;
  predefined_address_frag.fr_type = rs_fill;
}

/* Check that we're not trying to assemble into a section that can't
   allocate frags (currently, this is only possible in the absolute
   section), or into an mri common.  */

static void
frag_alloc_check (const struct obstack *ob)
{
  if (ob->chunk_size == 0)
    {
      as_bad (_("attempt to allocate data in absolute section"));
      subseg_set (text_section, 0);
    }

  if (mri_common_symbol != NULL)
    {
      as_bad (_("attempt to allocate data in common section"));
      mri_common_symbol = NULL;
    }
}

/* Allocate a frag on the specified obstack.
   Call this routine from everywhere else, so that all the weird alignment
   hackery can be done in just one place.  */

fragS *
frag_alloc (struct obstack *ob)
{
  fragS *ptr;
  int oalign;

  (void) obstack_alloc (ob, 0);
  oalign = obstack_alignment_mask (ob);
  obstack_alignment_mask (ob) = 0;
  ptr = (fragS *) obstack_alloc (ob, SIZEOF_STRUCT_FRAG);
  obstack_alignment_mask (ob) = oalign;
  memset (ptr, 0, SIZEOF_STRUCT_FRAG);
  totalfrags++;
  return ptr;
}

/* Try to augment current frag by nchars chars.
   If there is no room, close off the current frag with a ".fill 0"
   and begin a new frag.  Then loop until the new frag has at least
   nchars chars available.  Does not set up any fields in frag_now.  */

void
frag_grow (size_t nchars)
{
  if (obstack_room (&frchain_now->frch_obstack) < nchars)
    {
      size_t oldc;
      size_t newc;

      /* Try to allocate a bit more than needed right now.  But don't do
         this if we would waste too much memory.  Especially necessary
         for extremely big (like 2GB initialized) frags.  */
      if (nchars < 0x10000)
        newc = 2 * nchars;
      else
        newc = nchars + 0x10000;
      newc += SIZEOF_STRUCT_FRAG;

      /* Check for possible overflow.  */
      if (newc < nchars)
	as_fatal (ngettext ("can't extend frag %lu char",
			    "can't extend frag %lu chars",
			    (unsigned long) nchars),
		  (unsigned long) nchars);

      /* Force to allocate at least NEWC bytes, but not less than the
         default.  */
      oldc = obstack_chunk_size (&frchain_now->frch_obstack);
      if (newc > oldc)
	obstack_chunk_size (&frchain_now->frch_obstack) = newc;

      while (obstack_room (&frchain_now->frch_obstack) < nchars)
        {
          /* Not enough room in this frag.  Close it and start a new one.
             This must be done in a loop because the created frag may not
             be big enough if the current obstack chunk is used.  */
          frag_wane (frag_now);
          frag_new (0);
        }

      /* Restore the old chunk size.  */
      obstack_chunk_size (&frchain_now->frch_obstack) = oldc;
    }
}

/* Call this to close off a completed frag, and start up a new (empty)
   frag, in the same subsegment as the old frag.
   [frchain_now remains the same but frag_now is updated.]
   Because this calculates the correct value of fr_fix by
   looking at the obstack 'frags', it needs to know how many
   characters at the end of the old frag belong to the maximal
   variable part;  The rest must belong to fr_fix.
   It doesn't actually set up the old frag's fr_var.  You may have
   set fr_var == 1, but allocated 10 chars to the end of the frag;
   In this case you pass old_frags_var_max_size == 10.
   In fact, you may use fr_var for something totally unrelated to the
   size of the variable part of the frag;  None of the generic frag
   handling code makes use of fr_var.

   Make a new frag, initialising some components. Link new frag at end
   of frchain_now.  */

void
frag_new (size_t old_frags_var_max_size
	  /* Number of chars (already allocated on obstack frags) in
	     variable_length part of frag.  */)
{
  fragS *former_last_fragP;
  frchainS *frchP;

  gas_assert (frchain_now->frch_last == frag_now);

  /* Fix up old frag's fr_fix.  */
  frag_now->fr_fix = frag_now_fix_octets ();
  gas_assert (frag_now->fr_fix >= old_frags_var_max_size
	      || now_seg == absolute_section);
  frag_now->fr_fix -= old_frags_var_max_size;
  /* Make sure its type is valid.  */
  gas_assert (frag_now->fr_type != 0);

  /* This will align the obstack so the next struct we allocate on it
     will begin at a correct boundary.  */
  obstack_finish (&frchain_now->frch_obstack);
  frchP = frchain_now;
  know (frchP);
  former_last_fragP = frchP->frch_last;
  gas_assert (former_last_fragP != 0);
  gas_assert (former_last_fragP == frag_now);
  frag_now = frag_alloc (&frchP->frch_obstack);

  frag_now->fr_file = as_where (&frag_now->fr_line);

  /* Generally, frag_now->points to an address rounded up to next
     alignment.  However, characters will add to obstack frags
     IMMEDIATELY after the struct frag, even if they are not starting
     at an alignment address.  */
  former_last_fragP->fr_next = frag_now;
  frchP->frch_last = frag_now;

#ifndef NO_LISTING
  {
    extern struct list_info_struct *listing_tail;
    frag_now->line = listing_tail;
  }
#endif

  gas_assert (frchain_now->frch_last == frag_now);

  frag_now->fr_next = NULL;
}

/* Start a new frag unless we have n more chars of room in the current frag.
   Close off the old frag with a .fill 0.

   Return the address of the 1st char to write into. Advance
   frag_now_growth past the new chars.  */

char *
frag_more (size_t nchars)
{
  char *retval;

  frag_alloc_check (&frchain_now->frch_obstack);
  frag_grow (nchars);
  retval = obstack_next_free (&frchain_now->frch_obstack);
  obstack_blank_fast (&frchain_now->frch_obstack, nchars);
  return retval;
}

/* Close the current frag, setting its fields for a relaxable frag.  Start a
   new frag.  */

static void
frag_var_init (relax_stateT type, size_t max_chars, size_t var,
	       relax_substateT subtype, symbolS *symbol, offsetT offset,
               char *opcode)
{
  frag_now->fr_var = var;
  frag_now->fr_type = type;
  frag_now->fr_subtype = subtype;
  frag_now->fr_symbol = symbol;
  frag_now->fr_offset = offset;
  frag_now->fr_opcode = opcode;
#ifdef USING_CGEN
  frag_now->fr_cgen.insn = 0;
  frag_now->fr_cgen.opindex = 0;
  frag_now->fr_cgen.opinfo = 0;
#endif
#ifdef TC_FRAG_INIT
  TC_FRAG_INIT (frag_now, max_chars);
#endif
  frag_now->fr_file = as_where (&frag_now->fr_line);

  frag_new (max_chars);
}

/* Start a new frag unless we have max_chars more chars of room in the
   current frag.  Close off the old frag with a .fill 0.

   Set up a machine_dependent relaxable frag, then start a new frag.
   Return the address of the 1st char of the var part of the old frag
   to write into.  */

char *
frag_var (relax_stateT type, size_t max_chars, size_t var,
	  relax_substateT subtype, symbolS *symbol, offsetT offset,
	  char *opcode)
{
  char *retval;

  frag_grow (max_chars);
  retval = obstack_next_free (&frchain_now->frch_obstack);
  obstack_blank_fast (&frchain_now->frch_obstack, max_chars);
  frag_var_init (type, max_chars, var, subtype, symbol, offset, opcode);
  return retval;
}

/* OVE: This variant of frag_var assumes that space for the tail has been
	allocated by caller.
	No call to frag_grow is done.  */

char *
frag_variant (relax_stateT type, size_t max_chars, size_t var,
	      relax_substateT subtype, symbolS *symbol, offsetT offset,
	      char *opcode)
{
  char *retval;

  retval = obstack_next_free (&frchain_now->frch_obstack);
  frag_var_init (type, max_chars, var, subtype, symbol, offset, opcode);

  return retval;
}

/* Reduce the variable end of a frag to a harmless state.  */

void
frag_wane (fragS *fragP)
{
  fragP->fr_type = rs_fill;
  fragP->fr_offset = 0;
  fragP->fr_var = 0;
}

/* Return the number of bytes by which the current frag can be grown.  */

size_t
frag_room (void)
{
  return obstack_room (&frchain_now->frch_obstack);
}

/* Make an alignment frag.  The size of this frag will be adjusted to
   force the next frag to have the appropriate alignment.  ALIGNMENT
   is the power of two to which to align.  FILL_CHARACTER is the
   character to use to fill in any bytes which are skipped.  MAX is
   the maximum number of characters to skip when doing the alignment,
   or 0 if there is no maximum.  */

void
frag_align (int alignment, int fill_character, int max)
{
  if (now_seg == absolute_section)
    {
      addressT new_off;
      addressT mask;

      mask = (~(addressT) 0) << alignment;
      new_off = (abs_section_offset + ~mask) & mask;
      if (max == 0 || new_off - abs_section_offset <= (addressT) max)
	abs_section_offset = new_off;
    }
  else
    {
      char *p;

      p = frag_var (rs_align, 1, 1, (relax_substateT) max,
		    (symbolS *) 0, (offsetT) alignment, (char *) 0);
      *p = fill_character;
    }
}

/* Make an alignment frag like frag_align, but fill with a repeating
   pattern rather than a single byte.  ALIGNMENT is the power of two
   to which to align.  FILL_PATTERN is the fill pattern to repeat in
   the bytes which are skipped.  N_FILL is the number of bytes in
   FILL_PATTERN.  MAX is the maximum number of characters to skip when
   doing the alignment, or 0 if there is no maximum.  */

void
frag_align_pattern (int alignment, const char *fill_pattern,
		    size_t n_fill, int max)
{
  char *p;

  p = frag_var (rs_align, n_fill, n_fill, (relax_substateT) max,
		(symbolS *) 0, (offsetT) alignment, (char *) 0);
  memcpy (p, fill_pattern, n_fill);
}

/* The NOP_OPCODE is for the alignment fill value.  Fill it with a nop
   instruction so that the disassembler does not choke on it.  */
#ifndef NOP_OPCODE
#define NOP_OPCODE 0x00
#endif

/* Use this to restrict the amount of memory allocated for representing
   the alignment code.  Needs to be large enough to hold any fixed sized
   prologue plus the replicating portion.  */
#ifndef MAX_MEM_FOR_RS_ALIGN_CODE
  /* Assume that if HANDLE_ALIGN is not defined then no special action
     is required to code fill, which means that we get just repeat the
     one NOP_OPCODE byte.  */
# ifndef HANDLE_ALIGN
#  define MAX_MEM_FOR_RS_ALIGN_CODE  1
# else
#  define MAX_MEM_FOR_RS_ALIGN_CODE  (((size_t) 1 << alignment) - 1)
# endif
#endif

void
frag_align_code (int alignment, int max)
{
  char *p;

  p = frag_var (rs_align_code, MAX_MEM_FOR_RS_ALIGN_CODE, 1,
		(relax_substateT) max, (symbolS *) 0,
		(offsetT) alignment, (char *) 0);
  *p = NOP_OPCODE;
}

addressT
frag_now_fix_octets (void)
{
  if (now_seg == absolute_section)
    return abs_section_offset;

  return ((char *) obstack_next_free (&frchain_now->frch_obstack)
	  - frag_now->fr_literal);
}

addressT
frag_now_fix (void)
{
  /* Symbols whose section has SEC_ELF_OCTETS set,
     resolve to octets instead of target bytes.  */
  if (now_seg->flags & SEC_OCTETS)
    return frag_now_fix_octets ();
  else
    return frag_now_fix_octets () / OCTETS_PER_BYTE;
}

void
frag_append_1_char (int datum)
{
  frag_alloc_check (&frchain_now->frch_obstack);
  if (obstack_room (&frchain_now->frch_obstack) <= 1)
    {
      frag_wane (frag_now);
      frag_new (0);
    }
  obstack_1grow (&frchain_now->frch_obstack, datum);
}

/* Return TRUE if FRAG1 and FRAG2 have a fixed relationship between
   their start addresses.  Set OFFSET to the difference in address
   not already accounted for in the frag FR_ADDRESS.  */

bool
frag_offset_fixed_p (const fragS *frag1, const fragS *frag2, offsetT *offset)
{
  const fragS *frag;
  offsetT off;

  /* Start with offset initialised to difference between the two frags.
     Prior to assigning frag addresses this will be zero.  */
  off = frag1->fr_address - frag2->fr_address;
  if (frag1 == frag2)
    {
      *offset = off;
      return true;
    }

  /* Maybe frag2 is after frag1.  */
  frag = frag1;
  while (frag->fr_type == rs_fill)
    {
      off += frag->fr_fix + frag->fr_offset * frag->fr_var;
      frag = frag->fr_next;
      if (frag == NULL)
	break;
      if (frag == frag2)
	{
	  *offset = off;
	  return true;
	}
    }

  /* Maybe frag1 is after frag2.  */
  off = frag1->fr_address - frag2->fr_address;
  frag = frag2;
  while (frag->fr_type == rs_fill)
    {
      off -= frag->fr_fix + frag->fr_offset * frag->fr_var;
      frag = frag->fr_next;
      if (frag == NULL)
	break;
      if (frag == frag1)
	{
	  *offset = off;
	  return true;
	}
    }

  return false;
}

/* Return TRUE if FRAG2 follows FRAG1 with a fixed relationship
   between the two assuming alignment frags do nothing.  Set OFFSET to
   the difference in address not already accounted for in the frag
   FR_ADDRESS.  */

bool
frag_offset_ignore_align_p (const fragS *frag1, const fragS *frag2,
			    offsetT *offset)
{
  const fragS *frag;
  offsetT off;

  /* Start with offset initialised to difference between the two frags.
     Prior to assigning frag addresses this will be zero.  */
  off = frag1->fr_address - frag2->fr_address;
  if (frag1 == frag2)
    {
      *offset = off;
      return true;
    }

  frag = frag1;
  while (frag->fr_type == rs_fill
	 || frag->fr_type == rs_align
	 || frag->fr_type == rs_align_code
	 || frag->fr_type == rs_align_test)
    {
      if (frag->fr_type == rs_fill)
	off += frag->fr_fix + frag->fr_offset * frag->fr_var;
      frag = frag->fr_next;
      if (frag == NULL)
	break;
      if (frag == frag2)
	{
	  *offset = off;
	  return true;
	}
    }

  return false;
}

/* Return TRUE if we can determine whether FRAG2 OFF2 appears after
   (strict >, not >=) FRAG1 OFF1, assuming it is not before.  Set
   *OFFSET so that resolve_expression will resolve an O_gt operation
   between them to false (0) if they are guaranteed to be at the same
   location, or to true (-1) if they are guaranteed to be at different
   locations.  Return FALSE conservatively, e.g. if neither result can
   be guaranteed (yet).

   They are known to be in the same segment, and not the same frag
   (this is a fallback for frag_offset_fixed_p, that always takes care
   of this case), and it is expected (from the uses this is designed
   to simplify, namely location view increments) that frag2 is
   reachable from frag1 following the fr_next links, rather than the
   other way round.  */

bool
frag_gtoffset_p (valueT off2, const fragS *frag2,
		 valueT off1, const fragS *frag1, offsetT *offset)
{
  /* Insanity check.  */
  if (frag2 == frag1 || off1 > frag1->fr_fix)
    return false;

  /* If the first symbol offset is at the end of the first frag and
     the second symbol offset at the beginning of the second frag then
     it is possible they are at the same address.  Go looking for a
     non-zero fr_fix in any frag between these frags.  If found then
     we can say the O_gt result will be true.  If no such frag is
     found we assume that frag1 or any of the following frags might
     have a variable tail and thus the answer is unknown.  This isn't
     strictly true; some frags don't have a variable tail, but it
     doesn't seem worth optimizing for those cases.  */
  const fragS *frag = frag1;
  offsetT delta = off2 - off1;
  for (;;)
    {
      delta += frag->fr_fix;
      frag = frag->fr_next;
      if (frag == frag2)
	{
	  if (delta == 0)
	    return false;
	  break;
	}
      /* If we run off the end of the frag chain then we have a case
	 where frag2 is not after frag1, ie. an O_gt expression not
	 created for .loc view.  */
      if (frag == NULL)
	return false;
    }

  *offset = (off2 - off1 - delta) * OCTETS_PER_BYTE;
  return true;
}
