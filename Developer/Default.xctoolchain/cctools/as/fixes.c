/* fixes.h (was taken from write.c in the original GAS)
   Copyright (C) 1986,1987 Free Software Foundation, Inc.

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

/* FROM line 25 */
#include "as.h"

#include "sections.h"
#include "obstack.h"
#include "frags.h"
#include "fixes.h"
#include "symbols.h"
#include "input-scrub.h"
#include <mach-o/x86_64/reloc.h>


static fixS *
fix_new_internal (fragS	*frag,		/* Which frag? */
		  int	where,		/* where in that frag? */
		  int	size,		/* 1, 2 or 4 bytes */
		  symbolS *add_symbol,	/* X_add_symbol */
		  symbolS *sub_symbol,	/* X_subtract_symbol */
		  signed_target_addr_t offset,		/* X_add_number */
		  int	pcrel,		/* TRUE if PC-relative relocation */
		  int	pcrel_reloc,	/* TRUE if must have relocation entry */
		  int	r_type)		/* relocation type */
{
    struct fix *fixP;

	fixP = (struct fix *)obstack_alloc(&notes, sizeof(struct fix));

	fixP->fx_frag	     = frag;
	fixP->fx_where       = where;
	fixP->fx_size	     = size;
	fixP->fx_addsy       = add_symbol;
	fixP->fx_subsy       = sub_symbol;
	fixP->fx_offset      = offset;
	fixP->fx_pcrel       = pcrel;
	fixP->fx_pcrel_reloc = pcrel_reloc;
	fixP->fx_r_type      = r_type;
	fixP->fx_sectdiff_divide_by_two = 0;
#if defined(I386) && defined(ARCH64)
	if(fixP->fx_r_type == X86_64_RELOC_SIGNED){
		switch(offset){
			case -1:
				fixP->fx_r_type = X86_64_RELOC_SIGNED_1;
				break;
			case -2:
				fixP->fx_r_type = X86_64_RELOC_SIGNED_2;
				break;
			case -4:
				fixP->fx_r_type = X86_64_RELOC_SIGNED_4;
				break;
			default:
				break;
		}
	}
	if(fixP->fx_r_type == X86_64_RELOC_GOT ||
	   fixP->fx_r_type == X86_64_RELOC_GOT_LOAD){
		/*
		 * GOT and GOT_LOAD relocs are always PC-relative and
		 * should not be converted to non-PC-relative addressing
		 * later.
		 */
		fixP->fx_pcrel = TRUE;
		fixP->fx_pcrel_reloc = TRUE;
	}
	/* We don't need this for non-local symbols, but it doesn't hurt. */
	fixP->fx_localsy = symbol_new("L0\002", N_SECT, frchain_now->frch_nsect,
	                              0, where, frag);
	symbol_assign_index(fixP->fx_localsy);
#endif
	as_file_and_line (&fixP->file, &fixP->line);

	fixP->fx_next              = frchain_now->frch_fix_root;
	frchain_now->frch_fix_root = fixP;
	
	return fixP;
}

#include "expr.h" /* HACK */
/*
 * fix_new() creates a fixS in obstack 'notes'.
 */
fixS *
fix_new(
fragS	*frag,		/* which frag? */
int	where,		/* where in that frag? */
int	size,		/* 1, 2 or 4 bytes */
symbolS *add_symbol,	/* X_add_symbol */
symbolS *sub_symbol,	/* X_subtract_symbol */
	signed_target_addr_t
	offset,		/* X_add_number */
int	pcrel,		/* TRUE if PC-relative relocation */
int	pcrel_reloc,	/* TRUE if must have relocation entry */
int	r_type)		/* relocation type */
{
  return fix_new_internal (frag, where, size, add_symbol,
			   sub_symbol, offset, pcrel, pcrel_reloc, r_type);
}

/* FROM write.c line 291 */
typedef int RELOC_ENUM;
#define X_op X_seg
#define X_op_symbol X_add_symbol
/* Create a fixup for an expression.  Currently we only support fixups
   for difference expressions.  That is itself more than most object
   file formats support anyhow.  */

fixS *
fix_new_exp (fragS *frag,		/* Which frag?  */
	     int where,			/* Where in that frag?  */
	     int size,			/* 1, 2, or 4 usually.  */
	     expressionS *exp,		/* Expression.  */
	     int pcrel,			/* TRUE if PC-relative relocation.  */
	     int pcrel_reloc,		/* TRUE if must have relocation entry */
	     RELOC_ENUM r_type		/* Relocation type.  */)
{
  symbolS *add = NULL;
  symbolS *sub = NULL;
  signed_target_addr_t off = 0;

  switch (exp->X_op)
    {
#ifdef NOTYET
    case O_absent:
      break;

    case O_register:
      as_bad (_("register value used as expression"));
      break;

    case O_add:
      /* This comes up when _GLOBAL_OFFSET_TABLE_+(.-L0) is read, if
	 the difference expression cannot immediately be reduced.  */
      {
	symbolS *stmp = make_expr_symbol (exp);

	exp->X_op = O_symbol;
	exp->X_op_symbol = 0;
	exp->X_add_symbol = stmp;
	exp->X_add_number = 0;

	return fix_new_exp (frag, where, size, exp, pcrel, r_type);
      }

    case O_symbol_rva:
      add = exp->X_add_symbol;
      off = exp->X_add_number;

#if defined(BFD_ASSEMBLER)
      r_type = BFD_RELOC_RVA;
#else
#if defined(TC_RVA_RELOC)
      r_type = TC_RVA_RELOC;
#else
      as_fatal (_("rva not supported"));
#endif
#endif
      break;

    case O_uminus:
      sub = exp->X_add_symbol;
      off = exp->X_add_number;
      break;

    case O_subtract:
      sub = exp->X_op_symbol;
      /* Fall through.  */
#endif
    case O_symbol:
      add = exp->X_add_symbol;
      /* Fall through.  */
    case O_constant:
      off = (signed_target_addr_t)exp->X_add_number;
      break;

    default:
      add = make_expr_symbol (exp);
      break;
    }

  return fix_new_internal (frag, where, size, add, sub, off, pcrel, pcrel_reloc, r_type);
}
