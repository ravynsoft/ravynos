/* frags.c - manage frags -
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <string.h>
#include <stdint.h> /* cctools-port: for intptr_t */
#include "as.h"
#include "sections.h"
#include "obstack.h"
#include "frags.h"
#include "messages.h"
#include "input-scrub.h"

struct obstack frags = { 0 };	/* All, and only, frags live here. */

fragS *frag_now = NULL;	/* -> current frag we are building. */

fragS zero_address_frag = {
	0,			/* fr_address */
	0,			/* last_fr_address */
	NULL,			/* fr_next */
	0,			/* fr_fix */
	0,			/* fr_var */
	0,			/* fr_symbol */
	0,			/* fr_offset */
	NULL,			/* fr_opcode */
	rs_fill,		/* fr_type */
	0,			/* fr_subtype */
#ifdef ARM
	0			/* fr_literal [0] */
#else
	{0}			/* fr_literal [0] */
#endif
};


/*
 *			frag_grow()
 *
 * Try to augment current frag by nchars chars.
 * If there is no room, close of the current frag with a ".fill 0"
 * and begin a new frag. Unless the new frag has nchars chars available
 * do not return. Do not set up any fields of *now_frag.
 */
void
frag_grow(
unsigned int nchars)
{
    if(frags.chunk_size == 0){
       know(flagseen['n']);
       as_fatal("with -n a section directive must be seen before assembly "
		"can begin");
    }
    if ((int)(obstack_room(&frags)) < nchars) {
	unsigned int n,oldn;
	size_t oldc;

	frag_wane (frag_now);
	frag_new (0);
	oldn=(unsigned)-1;
	oldc=frags.chunk_size;
	if(2*nchars > oldc)
	    frags.chunk_size=2*nchars;
	while((int)(n=obstack_room(&frags)) < nchars && n < oldn) {
		frag_wane(frag_now);
		frag_new(0);
		oldn=n;
	}
	frags.chunk_size=oldc;
    }
    if ((int)(obstack_room(&frags)) < nchars)
	as_fatal ("Can't extend frag %d. chars", nchars);
}

/*
 *			frag_new()
 *
 * Call this to close off a completed frag, and start up a new (empty)
 * frag, in the same subsegment as the old frag.
 * [frchain_now remains the same but frag_now is updated.]
 * Because this calculates the correct value of fr_fix by
 * looking at the obstack 'frags', it needs to know how many
 * characters at the end of the old frag belong to (the maximal)
 * fr_var: the rest must belong to fr_fix.
 * It doesn't actually set up the old frag's fr_var: you may have
 * set fr_var == 1, but allocated 10 chars to the end of the frag:
 * in this case you pass old_frags_var_max_size == 10.
 *
 * Make a new frag, initialising some components. Link new frag at end
 * of frchain_now.
 */
void
frag_new(
int old_frags_var_max_size)	/* Number of chars (already allocated on obstack
				   frags) in variable_length part of frag. */
{
    register    fragS * former_last_fragP;
/*    char   *throw_away_pointer; JF unused */
    register    frchainS * frchP;
    int32_t	tmp;		/* JF */


    if(frags.chunk_size == 0){
       know(flagseen['n']);
       as_fatal("with -n a section directive must be seen before assembly "
		"can begin");
    }
    /* cctools-port: Added (intptr_t) cast to silence warning */
    frag_now->fr_fix = (int)(intptr_t)((char *) (obstack_next_free (&frags)) -
			     (long)(frag_now->fr_literal) -
			     old_frags_var_max_size);
 /* Fix up old frag's fr_fix. */

    (void)obstack_finish (&frags);
 /* This will align the obstack so the */
 /* next struct we allocate on it will */
 /* begin at a correct boundary. */
    frchP = frchain_now;
    know (frchP);
    former_last_fragP = frchP->frch_last;
    know (former_last_fragP);
    know (former_last_fragP == frag_now);
    obstack_blank (&frags, (int)SIZEOF_STRUCT_FRAG);
 /* We expect this will begin at a correct */
 /* boundary for a struct. */
    tmp=obstack_alignment_mask(&frags);
    obstack_alignment_mask(&frags)=0;		/* Turn off alignment */
    						/* If we ever hit a machine
						   where strings must be
						   aligned, we Lose Big */
 frag_now=(fragS *)obstack_finish(&frags);
    obstack_alignment_mask(&frags)=tmp;		/* Restore alignment */

 /* Just in case we don't get zero'd bytes */
 memset(frag_now, '\0', SIZEOF_STRUCT_FRAG);

/*    obstack_unaligned_done (&frags, &frag_now); */
/*    know (frags.obstack_c_next_free == frag_now->fr_literal); */
 /* Generally, frag_now->points to an */
 /* address rounded up to next alignment. */
 /* However, characters will add to obstack */
 /* frags IMMEDIATELY after the struct frag, */
 /* even if they are not starting at an */
 /* alignment address. */
    former_last_fragP->fr_next = frag_now;
    frchP->frch_last = frag_now;
    frag_now->fr_next = NULL;
}				/* frag_new() */

/*
 *			frag_more()
 *
 * Start a new frag unless we have n more chars of room in the current frag.
 * Close off the old frag with a .fill 0.
 *
 * Return the address of the 1st char to write into. Advance
 * frag_now_growth past the new chars.
 */
char *
frag_more(
int nchars)
{
    register char  *retval;

    frag_grow (nchars);
    retval = obstack_next_free (&frags);
    obstack_blank_fast (&frags, nchars);
    return (retval);
}				/* frag_more() */

/*
 *			frag_var()
 *
 * Start a new frag unless we have max_chars more chars of room in the current frag.
 * Close off the old frag with a .fill 0.
 *
 * Set up a machine_dependent relaxable frag, then start a new frag.
 * Return the address of the 1st char of the var part of the old frag
 * to write into.
 */
char *
frag_var(
relax_stateT type,
int max_chars,
int var,
relax_substateT subtype,
symbolS *symbol,
int32_t offset,
char *opcode)
{
    register char  *retval;

#ifdef ARM
    as_file_and_line (&frag_now->fr_file, &frag_now->fr_line);
#endif /* ARM */
    frag_grow (max_chars);
    retval = obstack_next_free (&frags);
    obstack_blank_fast (&frags, max_chars);
    frag_now->fr_var = var;
    frag_now->fr_type = type;
    frag_now->fr_subtype = subtype;
    frag_now->fr_symbol = symbol;
    frag_now->fr_offset = offset;
    frag_now->fr_opcode = opcode;
    frag_new (max_chars);
#ifdef ARM
    as_file_and_line (&frag_now->fr_file, &frag_now->fr_line);
#endif /* ARM */
    return (retval);
}				/* frag_var() */

/*
 *			frag_wane()
 *
 * Reduce the variable end of a frag to a harmless state.
 */
void
frag_wane(
fragS *fragP)
{
    fragP->fr_type = rs_fill;
    fragP->fr_offset = 0;
    fragP->fr_var = 0;
}

/*
 *			frag_align()
 *
 * Make an rs_align frag for:
 *   .align power_of_2_alignment, fill_expression, fill_size, max_bytes_to_fill
 * the fill_size must be 1, 2 or 4.  An rs_align frag stores the
 * power_of_2_alignment in the fr_offset field of the frag, the fill_expression
 * in the fr_literal bytes, the fill_size in the fr_var field and the
 * max_bytes_to_fill in the fr_subtype field.  We call frag_var() with max_chars
 * parameter large enough to hold the fill_expression of fill_size plus the
 * maximum number of partial bytes that may be needed to be zeroed before the
 * fill.
 *
 * Call to close off the current frag with a ".align", then start a new
 * (so far empty) frag, in the same subsegment as the last frag.
 */
void
frag_align(
int power_of_2_alignment,
char *fill,
int fill_size,
int max_bytes_to_fill)
{
    void *fr_literal;
    int max_chars;
#ifdef I386
	/*
	 * For x86 architecures in sections containing only instuctions being
	 * padded with nops that are aligned to 16 bytes or less and are
	 * assembled -dynamic we will actually end up padding with the optimal
	 * nop sequence.  So for that make sure there is the maximum number of
	 * bytes allocated in the frag to use for this.
	 */
	if((frchain_now->frch_section.flags & S_ATTR_PURE_INSTRUCTIONS) != 0 &&
	   fill_size == 1 && *fill == (char)0x90 && flagseen['k'] == TRUE){
	    if(power_of_2_alignment > 4)
		max_chars = 15;
	    else
		max_chars = (1 << power_of_2_alignment) - 1;
	}
	else
#endif /* 386 */
	max_chars = fill_size + (fill_size - 1);

	fr_literal = (void *)
	    frag_var(rs_align,				/* type */
		     max_chars,				/* max_chars */
		     fill_size,				/* var */
		     (relax_substateT)max_bytes_to_fill,/* subtype */
		     (symbolS *)0,			/* symbol */
		     (int32_t)power_of_2_alignment,	/* offset */
		     (char *)0);			/* opcode */
    if(fill_size == 1 || fill_size == 2 || fill_size == 4)
	memcpy(fr_literal, fill, fill_size);
    else
	as_bad("Invalid width for fill expression.");
}

addressT
frag_now_fix_octets (void)
{
#ifdef NeXT_MOD
  return ((char *) obstack_next_free (&frags)
	  - frag_now->fr_literal);
#else
  if (now_seg == absolute_section)
    return abs_section_offset;

  return ((char *) obstack_next_free (&frchain_now->frch_obstack)
	  - frag_now->fr_literal);
#endif
}

addressT
frag_now_fix (void)
{
  return frag_now_fix_octets () / OCTETS_PER_BYTE;
}

/* end: frags.c */
