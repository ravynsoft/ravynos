#ifndef _FRAGS_H_
#define _FRAGS_H_
/* frags.h - Header file for the frag concept.
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

#import "as.h"
#import "relax.h"
#import "struc-symbol.h"

/*
 * A code fragment (frag) is some known number of chars, followed by some
 * unknown number of chars. Typically the unknown number of chars is an
 * instruction address whose size is yet unknown. We always know the greatest
 * possible size the unknown number of chars may become, and reserve that
 * much room at the end of the frag.
 * Once created, frags do not change address during assembly.
 * We chain the frags in (a) forward-linked list(s). The object-file address
 * of the 1st char of a frag is generally not known until after relax().
 * Many things at assembly time describe an address by {object-file-address
 * of a particular frag}+offset.

 BUG: it may be smarter to have a single pointer off to various different
notes for different frag kinds. See how code pans out.

 */
struct frag			/* a code fragment */
{
    uint64_t fr_address;	/* Object file address. */
    uint64_t last_fr_address;	/* When relaxing multiple times, remember the */
				/* address the frag had in the last relax pass*/
    struct frag *fr_next;	/* Chain forward; ascending address order. */
				/* Rooted in frch_root. */

    int32_t fr_fix;		/* (Fixed) number of chars we know we have. */
				/* May be 0. */
    int32_t fr_var;		/* (Variable) number of chars after above. */
				/* May be 0. */
    struct symbol *fr_symbol;	/* For variable-length tail. */
    int32_t fr_offset;		/* For variable-length tail. */
    char *fr_opcode;		/* ->opcode low addr byte,for relax()ation*/
    relax_stateT fr_type;	/* What state is my tail in? */
    relax_substateT fr_subtype;	/* Used to index in to md_relax_table for */
				/*  fr_type == rs_machine_dependent frags. */
#ifdef ARM
    /* Where the frag was created, or where it became a variant frag.  */
    char *fr_file;
    unsigned int fr_line;
    /* Flipped each relax pass so we can easily determine whether
       fr_address has been adjusted.  */
    unsigned int relax_marker:1,
		 pad:31;
#endif /* ARM */
    char fr_literal[1];		/* Chars begin here. */
				/* One day we will compile fr_literal[0]. */
};

/* We want to say fr_literal[0] below */
#define SIZEOF_STRUCT_FRAG \
 ((uintptr_t)zero_address_frag.fr_literal - (uintptr_t)&zero_address_frag)

/*
 * frag_now points at the current frag we are building. This frag is incomplete.
 * It is, however, included in frchain_now. Frag_now->fr_fix is not the total
 * bytes in use for the frag.  For that use:
 * frag_now->fr_fix + obstack_next_free(&frags) - frag_now->fr_literal.
 */
extern fragS *frag_now;
extern addressT frag_now_fix (void);
extern addressT frag_now_fix_octets (void);

/*
 * Frags ONLY live in this obstack.  We use obstack_next_free() macro 
 * so please don't put any other objects on this stack!
 */
extern struct obstack frags;

/* For foreign-segment symbol fixups. */
extern fragS zero_address_frag;

void frag_grow (unsigned int nchars);
extern void frag_new(
    int old_frags_var_max_size);
extern char * frag_more(
    int nchars);
extern char *frag_var(
    relax_stateT type,
    int max_chars,
    int var,
    relax_substateT subtype,
    symbolS *symbol,
    int32_t offset,
    char *opcode);
extern void frag_wane(
    fragS *fragP);
extern void frag_align(
    int power_of_2_alignment,
    char *fill,
    int fill_size,
    int max_bytes_to_fill);

/*
 * A macro to speed up appending exactly 1 char
 * to current frag.
 */
/* JF changed < 1 to <= 1 to avoid a race conditon */
#define FRAG_APPEND_1_CHAR(datum)	\
{					\
	if (obstack_room( &frags ) <= 1) {\
		frag_wane (frag_now);	\
		frag_new (0);		\
	}				\
	(void)obstack_1grow( &frags, datum );	\
}
#endif /* _FRAGS_H_ */
