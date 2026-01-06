#ifndef _FIXES_H_
#define _FIXES_H_
/* fixes.h (was write.h in the original GAS)
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

/*
 * For machines with machine dependent relocation types (encoded in the r_type
 * field of a relocation_info struct) they use NO_RELOC in assembling
 * instructions which they want to indicate have no relocation.
 */
#define NO_RELOC        0x10 /* above the range of r_type:4 */

/*
 * FixSs may be built up in any order.
 */
struct fix {
    fragS	*fx_frag;	/* which frag? */
    int32_t	 fx_where;	/* where is the 1st byte to fix up? */
    symbolS	*fx_addsy;	/* NULL or Symbol whose value we add in */
    symbolS	*fx_subsy;	/* NULL or Symbol whose value we subtract */
#if defined(I386) && defined(ARCH64)
    symbolS *fx_localsy;	/* NULL or pseudo-symbol for this fixup */
#endif
    signed_expr_t	
		 fx_offset;	/* absolute number we add in */
    struct fix	*fx_next;	/* NULL or -> next fixS */
    char	 fx_size;	/* how many bytes are involved? */
    char	 fx_pcrel;	/* TRUE: pc-relative. */
    char	 fx_pcrel_reloc;/* force a pc-relative relocatation entry */
    char	 fx_r_type;	/* relocation type */
    int32_t	 fx_value;	/* the relocated value placed in the frag */
    char	*file;		/* the file name this came from for errors */
    unsigned int line;		/* the line number this came from for errors */

  /* FROM write.h line 82 */
  /* Has this relocation already been applied?  */
  unsigned fx_done : 1,

  /* Non-zero if we have the special assembly time constant expression
     of the difference of two symbols defined in the same section then divided
     by exactly 2. */
           fx_sectdiff_divide_by_two : 1;

  /* FROM write.h line 133 */
  /* This field is sort of misnamed.  It appears to be a sort of random
     scratch field, for use by the back ends.  The main gas code doesn't
     do anything but initialize it to zero.  The use of it does need to
     be coordinated between the cpu and format files, though.  E.g., some
     coff targets pass the `addend' field from the cpu file via this
     field.  I don't know why the `fx_offset' field above can't be used
     for that; investigate later and document. KR  */
  valueT fx_addnumber;

  /* FROM write.h line 142 */
  /* The location of the instruction which created the reloc, used
     in error messages.  */
#ifdef NOTYET
  char *fx_file;
  unsigned fx_line;
#else
  #define fx_file file
  #define fx_line line
  void *tc_fix_data;
#endif
};
typedef struct fix fixS;

extern fixS *fix_new(
	fragS	*frag,		/* which frag? */
	int	where,		/* where in that frag? */
	int	size,		/* 1, 2 or 4 bytes */
	symbolS *add_symbol,	/* X_add_symbol */
	symbolS *sub_symbol,	/* X_subtract_symbol */
	signed_target_addr_t
	offset,		/* X_add_number */
	int	pcrel,		/* TRUE if PC-relative */
	int	pcrel_reloc,	/* TRUE if must have relocation entry */
	int	r_type);	/* relocation type */

/* FROM write.h line 210 */
#include "expr.h"
extern fixS *fix_new_exp
  (fragS * frag, int where, int size, expressionS *exp, int pcrel,
   int pcrel_reloc, int r_type);

#endif /* _FIXES_H_ */
