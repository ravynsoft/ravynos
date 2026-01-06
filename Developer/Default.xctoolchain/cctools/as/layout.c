/* layout.c (was part of write.c in original GAS version)
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

#include <stdlib.h>
#include <string.h>
#include "stuff/rnd.h"
#include "as.h"
#include "sections.h"
#include "frags.h"
#include "symbols.h"
#include "fixes.h"
#include "messages.h"
#include "expr.h"
#include "md.h"
#include "obstack.h"
#include "input-scrub.h"
#include "dwarf2dbg.h"
#if I386
#include "i386.h"
#endif

#ifdef SPARC
/* internal relocation types not to be emitted */
#define SPARC_RELOC_13 (127)
#define SPARC_RELOC_22 (126)
#endif

#ifdef ARM
/* FROM tc-arm.h line 82 */
int
arm_force_relocation (struct fix * fixp);

#define TC_FORCE_RELOCATION(FIX) arm_force_relocation (FIX)

/* FROM tc-arm.h line 133 */
/* This expression evaluates to true if the relocation is for a local
   object for which we still want to do the relocation at runtime.
   False if we are willing to perform this relocation while building
   the .o file.  GOTOFF does not need to be checked here because it is
   not pcrel.  I am not sure if some of the others are ever used with
   pcrel, but it is easier to be safe than sorry.  */

#define TC_FORCE_RELOCATION_LOCAL(FIX)			\
  (!(FIX)->fx_pcrel					\
   || TC_FORCE_RELOCATION (FIX))

/* FROM write.c line 35 */
#ifndef TC_FORCE_RELOCATION
#define TC_FORCE_RELOCATION(FIX)		\
  (0)
#endif

extern int arm_relax_frag (int nsect, fragS *fragp, int32_t stretch);

#endif /* ARM */

/* FROM write.c line 96 */
#ifndef	MD_PCREL_FROM_SECTION
extern int32_t md_pcrel_from_section(fixS * fixP);
#define MD_PCREL_FROM_SECTION(FIX, SEC) md_pcrel_from_section (FIX)
#endif

static void fixup_section(
    fixS *fixP,
    int nsect);
#ifndef SPARC
static int is_assembly_time_constant_subtraction_expression(
    symbolS *add_symbolP,
    int add_symbol_nsect,
    symbolS *sub_symbolP,
    int sub_symbol_nsect);
#endif /* !defined(SPARC) */
static int relax_section(
    struct frag *section_frag_root,
    int nsect);
static relax_addressT relax_align(
    relax_addressT address,
    uint32_t alignment);
#ifndef ARM
static int is_down_range(
    struct frag *f1,
    struct frag *f2);
#endif /* !defined(ARM) */

/*
 * add_last_frags_to_sections() does what layout_addresses() does below about
 * adding a last ".fill 0" frag to each section.  This is called by
 * dwarf2_finish() allow get_frag_fix() in dwarf2dbg.c to work for the last
 * fragment in a section.
 */
void
add_last_frags_to_sections(
void)
{
    struct frchain *frchainP;

	if(frchain_root == NULL)
	    return;

	/*
	 * If there is any current frag close it off.
	 */
	if(frag_now != NULL && frag_now->fr_fix == 0){
	    frag_now->fr_fix = (int32_t)(obstack_next_free(&frags) -
			       frag_now->fr_literal);
	    frag_wane(frag_now);
	}

	/*
	 * For every section, add a last ".fill 0" frag that will later be used
	 * as the ending address of that section.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    /*
	     * We must do the obstack_finish(), so the next object we put on
	     * obstack frags will not appear to start at the fr_literal of the
	     * current frag.  Also, it ensures that the next object will begin
	     * on a address that is aligned correctly for the engine that runs
	     * the assembler.
	     */
	    (void)obstack_finish(&frags);

	    /*
	     * Make a fresh frag for the last frag.
	     */
	    frag_now = (fragS *)obstack_alloc(&frags, (int)SIZEOF_STRUCT_FRAG);
	    memset(frag_now, '\0', SIZEOF_STRUCT_FRAG);
	    frag_now->fr_next = NULL;
	    (void)obstack_finish(&frags);

	    /*
	     * Append the new frag to current frchain.
	     */
	    frchainP->frch_last->fr_next = frag_now;
	    frchainP->frch_last = frag_now;
	    frag_wane(frag_now);
	}
}

/*
 * layout_addresses() is called after all the assembly code has been read and
 * fragments, symbols and fixups have been created.  This routine sets the
 * address of the fragments and symbols.  Then it does the fixups of the frags
 * and prepares the fixes so relocation entries can be created from them.
 */
void
layout_addresses(
void)
{
    struct frchain *frchainP;
    fragS *fragP;
    uint64_t slide, tmp;
    symbolS *symbolP;
    uint32_t nbytes, fill_size, repeat_expression, partial_bytes, layout_pass;
    uint32_t section_type;
    relax_stateT old_fr_type;
    int changed;

	if(frchain_root == NULL)
	    return;

	/*
	 * If there is any current frag close it off.
	 */
	if(frag_now != NULL && frag_now->fr_fix == 0){
	    frag_now->fr_fix = (int32_t)(obstack_next_free(&frags) -
			       frag_now->fr_literal);
	    frag_wane(frag_now);
	}

	/*
	 * For every section, add a last ".fill 0" frag that will later be used
	 * as the ending address of that section.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    /*
	     * We must do the obstack_finish(), so the next object we put on
	     * obstack frags will not appear to start at the fr_literal of the
	     * current frag.  Also, it ensures that the next object will begin
	     * on a address that is aligned correctly for the engine that runs
	     * the assembler.
	     */
	    (void)obstack_finish(&frags);

	    /*
	     * Make a fresh frag for the last frag.
	     */
	    frag_now = (fragS *)obstack_alloc(&frags, (int)SIZEOF_STRUCT_FRAG);
	    memset(frag_now, '\0', SIZEOF_STRUCT_FRAG);
	    frag_now->fr_next = NULL;
	    (void)obstack_finish(&frags);

	    /*
	     * Append the new frag to current frchain.
	     */
	    frchainP->frch_last->fr_next = frag_now;
	    frchainP->frch_last = frag_now;
	    frag_wane(frag_now);

	}

	/*
	 * Now set the relative addresses of frags within the section by
	 * relaxing each section.  That is all sections will start at address
	 * zero and addresses of the frags in that section will increase from
	 * there.
	 *
	 * The debug sections are done last as other section are needed to be
	 * done first becase debug sections may have line numbers with .loc
	 * directives in them and their sizes need to be set before processing
	 * the line number sections.  We also do sections that have rs_leb128s
	 * in them before debug sections but after other sections since they
	 * are used for things like exception tables and they may be refering to
	 * sections such that their sizes too must be known first.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    if((frchainP->frch_section.flags & S_ATTR_DEBUG) == S_ATTR_DEBUG)
		frchainP->layout_pass = 2;
	    else if(frchainP->has_rs_leb128s == TRUE)
		frchainP->layout_pass = 1;
	    else
		frchainP->layout_pass = 0;
	}
	for(layout_pass = 0; layout_pass < 3; layout_pass++){
	    do{
		changed = 0;
		for(frchainP = frchain_root;
		    frchainP;
		    frchainP = frchainP->frch_next){
		    if(frchainP->layout_pass != layout_pass)
			continue;
		    section_type = frchainP->frch_section.flags & SECTION_TYPE;
		    if(section_type == S_ZEROFILL ||
		       section_type == S_THREAD_LOCAL_ZEROFILL)
			continue;
		    /*
		     * This is done so in case md_estimate_size_before_relax()
		     * (called by relax_section) wants to make fixSs they are
		     * for this section.
		     */
		    frchain_now = frchainP;

		    changed += relax_section(frchainP->frch_root,
					    frchainP->frch_nsect);
		}
	    }
	    while(changed != 0);
	}

	/*
	 * Now set the absolute addresses of all frags by sliding the frags in
	 * each non-zerofill section by the address ranges taken up by the
	 * sections before it.
	 */ 
	slide = 0;
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    section_type = frchainP->frch_section.flags & SECTION_TYPE;
	    if(section_type == S_ZEROFILL ||
	       section_type == S_THREAD_LOCAL_ZEROFILL)
		continue;
	    slide = rnd(slide, 1 << frchainP->frch_section.align);
	    tmp = frchainP->frch_last->fr_address;
	    if(slide != 0){
		for(fragP = frchainP->frch_root; fragP; fragP = fragP->fr_next){
		    fragP->fr_address += slide;
		}
	    }
	    slide += tmp;
	}
	/*
	 * Now with the non-zerofill section addresses set set all of the
	 * addresses of the zerofill sections.  Comming in the fr_address is
	 * the size of the section and going out it is the start address.  This
	 * will make layout_symbols() work out naturally.  The only funky thing
	 * is that section numbers do not end up in address order.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    section_type = frchainP->frch_section.flags & SECTION_TYPE;
	    if(section_type != S_ZEROFILL &&
	       section_type != S_THREAD_LOCAL_ZEROFILL)
		continue;
	    slide = rnd(slide, 1 << frchainP->frch_section.align);

	    tmp = frchainP->frch_root->fr_address;
	    frchainP->frch_root->fr_address = slide;
	    frchainP->frch_last->fr_address = tmp + slide;
	    slide += tmp;
	}

	/*
	 * Set the symbol addresses based on their frag's address.
	 * First forward references are handled.
	 */
	for(symbolP = symbol_rootP; symbolP; symbolP = symbolP->sy_next){
	    if(symbolP->sy_forward != NULL){
		if(symbolP->sy_nlist.n_type & N_STAB)
		    symbolP->sy_other = symbolP->sy_forward->sy_other;
		symbolP->sy_value += symbolP->sy_forward->sy_value +
				     symbolP->sy_forward->sy_frag->fr_address;
		symbolP->sy_forward = 0;
	    }
	}
	for(symbolP = symbol_rootP; symbolP; symbolP = symbolP->sy_next){
	    symbolP->sy_value += symbolP->sy_frag->fr_address;
	}

	/*
	 * At this point the addresses of frags now reflect addresses we use in 
	 * the object file and the symbol values are correct.
	 * Scan the frags, converting any ".org"s and ".align"s to ".fill"s.
	 * Also converting any machine-dependent frags using md_convert_frag();
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    /*
	     * This is done so any fixes created by md_convert_frag() are for
	     * this section.
	     */
	    frchain_now = frchainP;

	    for(fragP = frchainP->frch_root; fragP; fragP = fragP->fr_next){
		switch(fragP->fr_type){
		case rs_align:
		case rs_org:
		    old_fr_type = fragP->fr_type;
		    /* convert this frag to an rs_fill type */
		    fragP->fr_type = rs_fill;
		    /*
		     * Calculate the number of bytes the variable part of the
		     * the rs_fill frag will need to fill.  Then calculate this
		     * as the fill_size * repeat_expression + partial_bytes.
		     */
		    know(fragP->fr_next != NULL);
		    nbytes = (uint32_t)(fragP->fr_next->fr_address -
					fragP->fr_address -
					fragP->fr_fix);
		    if((int)nbytes < 0){
			as_warn("rs_org invalid, dot past value by %d bytes",
				nbytes);
			nbytes = 0;
		    }
		    fill_size = fragP->fr_var;
		    repeat_expression = nbytes / fill_size;
#ifdef I386
		    /*
		     * For x86 architecures in sections containing only
		     * instuctions being padded with nops that are aligned to 16
		     * bytes or less and are assembled with -dynamic we will
		     * actually end up padding with the optimal nop sequence.
		     * Previously there has been the maximum number of bytes
		     * allocated in the frag to use for this.
		     */
		    if(old_fr_type == rs_align &&
		       (frchain_now->frch_section.flags &
			S_ATTR_PURE_INSTRUCTIONS) != 0 &&
			 fill_size == 1 &&
			 fragP->fr_literal[fragP->fr_fix] == (char)0x90 &&
			 nbytes > 0 && nbytes < 16 &&
			 flagseen['k'] == TRUE){
			i386_align_code(fragP, nbytes);
			/*
			 * The call to i386_align_code() has set the fill_size
			 * in fragP->fr_var to nbytes. So we set the fr_offset
			 * to the fill repeat_expression to 1 to match for this
			 * now an rs_fill type frag.
			 */ 
			fragP->fr_offset = 1;
			break;
		    }
#endif /* I386 */
		    partial_bytes = nbytes - (repeat_expression * fill_size);
		    /*
		     * Now set the fr_offset to the fill repeat_expression
		     * since this is now an rs_fill type.  The fr_var is still
		     * the fill_size.
		     */
		    fragP->fr_offset = repeat_expression;
		    /*
		     * For rs_align frags there may be partial_bytes to fill
		     * with zeros before we can fill with the fill_expression
		     * of fill_size.  When the rs_align frag was created it was
		     * created with fill_size-1 extra bytes in the fixed part.
		     */
		    if(partial_bytes != 0){
			/* moved the fill_expression bytes foward */
			memmove(fragP->fr_literal +fragP->fr_fix +partial_bytes,
				fragP->fr_literal +fragP->fr_fix,
				fragP->fr_var);
    			/* zero out the partial_bytes */
    			memset(fragP->fr_literal + fragP->fr_fix,
			       '\0',
			       partial_bytes);
			/* adjust the fixed part of the frag */
    			fragP->fr_fix += partial_bytes;
		    }
		    break;

		case rs_fill:
		    break;

		case rs_machine_dependent:
		    md_convert_frag(fragP);
		    /*
		     * After md_convert_frag, we make the frag into a ".fill 0"
		     * md_convert_frag() should set up any fixSs and constants
		     * required.
		     */
		    frag_wane(fragP);
		    break;

		case rs_dwarf2dbg:
		    dwarf2dbg_convert_frag(fragP);
		    break;

		case rs_leb128:
		  {
		    int size;
#ifdef OLD
		    valueT value = S_GET_VALUE (fragP->fr_symbol);
#else
		    valueT value;
  		      expressionS *expression;
  		
		      if(fragP->fr_symbol->expression != NULL){
			expression =
			  (expressionS *)fragP->fr_symbol->expression;
			value = 0;
			if(expression->X_add_symbol != NULL)
			    value += expression->X_add_symbol->sy_nlist.n_value;
			if(expression->X_subtract_symbol != NULL)
			   value -= 
			     expression->X_subtract_symbol->sy_nlist.n_value;
			value += expression->X_add_number;
		      }
		      else{
			value = (valueT)(fragP->fr_symbol->sy_nlist.n_value +
					 fragP->fr_address);
		      }
#endif

		    size = output_leb128 (fragP->fr_literal + fragP->fr_fix,
					  value,
					  fragP->fr_subtype);
	       
		    fragP->fr_fix += size;
		    fragP->fr_type = rs_fill;
		    fragP->fr_var = 0;
		    fragP->fr_offset = 0;
		    fragP->fr_symbol = NULL; 
		  }
		  break;


		default:
		    BAD_CASE(fragP->fr_type);
		    break;
		}
	    }
	}

	/*
	 * For each section do the fixups for the frags.
	 */
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    now_seg = frchainP->frch_nsect;
	    fixup_section(frchainP->frch_fix_root, frchainP->frch_nsect);
	}
}

/*
 * fixup_section() does the fixups of the frags and prepares the fixes so
 * relocation entries can be created from them.  The fixups cause the contents
 * of the frag to have the value for the fixup expression.  A fix structure that
 * ends up with a non-NULL fx_addsy will have a relocation entry created for it.
 */
static
void
fixup_section(
fixS *fixP,
int nsect)
{
    symbolS *add_symbolP;
    symbolS *sub_symbolP;
    signed_expr_t value;
    int size;
    char *place;
    int32_t where;
    char pcrel;
    fragS *fragP;
    int	add_symbol_N_TYPE;
    int	add_symbol_nsect;
#ifndef SPARC
    int sub_symbol_nsect;
#endif

	/*
	 * The general fix expression is "fx_addsy - fx_subsy + fx_offset".
	 * The goal is to put the result of this expression into the frag at
	 * "place" for size "size".  The value of the expression is calculated
	 * in the variable "value" and starts with just the fx_offset.
	 */
	for( ; fixP != NULL; fixP = fixP->fx_next){
	    fragP       = fixP->fx_frag;
	    know(fragP);
	    where	= fixP->fx_where;
	    place       = fragP->fr_literal + where;
	    size	= fixP->fx_size;
#ifdef TC_FIXUP_SYMBOL
		fixP->fx_offset += TC_FIXUP_SYMBOL(fixP, nsect, &fixP->fx_addsy);
		fixP->fx_offset -= TC_FIXUP_SYMBOL(fixP, nsect, &fixP->fx_subsy);
#endif
#if defined(I386) && defined(ARCH64)
		if(fixP->fx_addsy == fixP->fx_subsy){
			/*
			 * If we've fixed up both symbols to the same location,
			 * we don't need a relocation entry.
			 */
			fixP->fx_addsy = NULL;
			fixP->fx_subsy = NULL;
		}
#endif
	    add_symbolP = fixP->fx_addsy;
	    sub_symbolP = fixP->fx_subsy;
	    value  	= fixP->fx_offset;
	    pcrel       = fixP->fx_pcrel;

#if ARM
	    /* If the symbol is defined in this file, the linker won't set the
	       low-order bit for a Thumb symbol, so we have to do it here.  */
	    if(add_symbolP != NULL && add_symbolP->sy_desc & N_ARM_THUMB_DEF &&
	       !(sub_symbolP != NULL && sub_symbolP->sy_desc & N_ARM_THUMB_DEF) &&
	       !pcrel){
	        value |= 1;
	    }
#endif

	    add_symbol_N_TYPE = 0;
	    add_symbol_nsect = 0;

	    if(add_symbolP != NULL){
		add_symbol_N_TYPE = add_symbolP->sy_type & N_TYPE;
		if(add_symbol_N_TYPE == N_SECT)
		    add_symbol_nsect = add_symbolP->sy_other;
	    }

	    /*
	     * Is there a subtract symbol?
	     */
	    if(sub_symbolP){
		/* is it just -sym ? */
		if(add_symbolP == NULL){
		    if(sub_symbolP->sy_type != N_ABS)
			as_warn("Negative of non-absolute symbol %s",
				sub_symbolP->sy_name);
#if !(defined(I386) && defined(ARCH64))
			/* Symbol offsets are not part of fixups for x86_64. */
		    value -= sub_symbolP->sy_value;
#endif
		    fixP->fx_subsy = NULL;
		}
		/*
		 * There are both an add symbol and a subtract symbol at this
		 * point.
		 *
		 * If both symbols are absolute then just calculate the
		 * value of the fix expression and no relocation entry will be
		 * needed.
		 */
		else if((sub_symbolP->sy_type & N_TYPE) == N_ABS &&
		        (add_symbolP->sy_type & N_TYPE) == N_ABS){
		    value += add_symbolP->sy_value - sub_symbolP->sy_value;
		    add_symbolP = NULL;
		    fixP->fx_addsy = NULL; /* no relocation entry */
		    fixP->fx_subsy = NULL;
		}
		/*
		 * If both symbols are defined in a section then calculate the
		 * value of the fix expression and let a section difference
		 * relocation entry be created.
		 */
		else if((sub_symbolP->sy_type & N_TYPE) == N_SECT &&
		        (add_symbolP->sy_type & N_TYPE) == N_SECT){
#if defined(I386) && !defined(ARCH64)
		    /*
		     * For 'symbol@TLVP - subtract_symbol' type relocations the
		     * subtract_symbol value is stored in the contents of the
		     * item to be relocated.
		     */
		    if(fixP->fx_r_type == GENERIC_RELOC_TLV){
			value += fixP->fx_frag->fr_address + where +
				 fixP->fx_size - sub_symbolP->sy_value;
			fixP->fx_subsy = NULL; /* no SECTDIFF reloc entry */
			fixP->fx_pcrel = TRUE; /* force pcrel */
			goto down;
		    }
#endif
		    /*
		     * We are use the new features that are incompatible with
		     * 3.2 then just calculate the value and let this create a
		     * SECTDIFF relocation type.
		     */
#ifdef SPARC
		    /*
		     * Special case dealing with assembler internal relocation
		     * entries SPARC_RELOC_13 and RELOC_22. The can not be
		     * output and must be resolved.
		     */
		    if((fixP->fx_r_type == SPARC_RELOC_13) ||
		       (fixP->fx_r_type == SPARC_RELOC_22)){
			if(sub_symbolP->sy_other == add_symbolP->sy_other){
			    value += add_symbolP->sy_value -
			    sub_symbolP->sy_value;
			    add_symbolP = NULL;
			    fixP->fx_addsy = NULL; /* no relocation entry */
			    fixP->fx_subsy = NULL;
			}
			else{
			    as_warn("Can't emit reloc type %u {-symbol \"%s\"} "
			            "@ file address %llu (mode?).",
				    fixP->fx_r_type, sub_symbolP->sy_name,
				    fragP->fr_address + where);
			}
		    }
		    else
			value += add_symbolP->sy_value - sub_symbolP->sy_value;
#else
#if !(defined(I386) && defined(ARCH64))
			/*
			 * Special case for x86_64.  'value' doesn't include
			 * the difference between the two symbols because
			 * that's handled by the subtractor/vanilla reloc pair.
			 */
		    value += add_symbolP->sy_value;
		    value -= sub_symbolP->sy_value;
#else
		    /*
		     * But for x86_64 expressions in the debug section must
		     * be the actual value of the expression.
		     */
		    if(is_section_debug(nsect)){
			value += add_symbolP->sy_value;
			value -= sub_symbolP->sy_value;
		    }
#endif
		    sub_symbol_nsect = sub_symbolP->sy_other;
		    /*
		     * If we have the special assembly time constant expression
		     * of the difference of two symbols defined in the same
		     * section then divided by exactly 2 adjust the value and
		     * make sure these symbols will produce an assembly time
		     * constant.
		     */
		    if(fixP->fx_sectdiff_divide_by_two == 1){
			value = value / 2;
			if(is_assembly_time_constant_subtraction_expression(
				add_symbolP, add_symbol_nsect,
				sub_symbolP, sub_symbol_nsect) == TRUE){
			    fixP->fx_addsy = NULL; /* no relocation entry */
			    goto down;
			}
			else{
			    layout_line = fixP->line;
			    layout_file = fixP->file;
			    as_warn("section difference divide by two "
				    "expression, \"%s\" minus \"%s\" divide by "
				    "2 will not produce an assembly time "
				    "constant", add_symbolP->sy_name,
				    sub_symbolP->sy_name);
			}
		    }
		    if(is_end_section_address(add_symbol_nsect,
					      add_symbolP->sy_value) ||
		       is_end_section_address(sub_symbol_nsect,
					      sub_symbolP->sy_value)){
			if(is_assembly_time_constant_subtraction_expression(
				add_symbolP, add_symbol_nsect,
				sub_symbolP, sub_symbol_nsect) == TRUE){
			    fixP->fx_addsy = NULL; /* no relocation entry */
			    goto down;
			}
			if(is_section_debug(nsect) &&
	   		   strcmp(add_symbolP->sy_name, FAKE_LABEL_NAME) == 0 &&
	   		   strcmp(sub_symbolP->sy_name, FAKE_LABEL_NAME) == 0){
			    fixP->fx_addsy = NULL; /* no relocation entry */
			    goto down;
			}
			layout_line = fixP->line;
			layout_file = fixP->file;
			as_warn("section difference relocatable subtraction "
				"expression, \"%s\" minus \"%s\" using a "
				"symbol at the end of section will not "
				"produce an assembly time constant",
				add_symbolP->sy_name, sub_symbolP->sy_name);
			as_warn("use a symbol with a constant value created "
				"with an assignment instead of the expression, "
				"L_const_sym = %s - %s", add_symbolP->sy_name,
				sub_symbolP->sy_name);
			layout_line = 0;
			layout_file = NULL;
		    }
#endif
		    goto down;
		}
		/*
		 * If the subtract symbol is absolute subtract it's value from
		 * the fix expression and let a relocation entry get created
		 * that is not a section difference type.
		 */
		else if(sub_symbolP->sy_type == N_ABS){
		    value -= sub_symbolP->sy_value;
		    fixP->fx_subsy = NULL; /* no SECTDIFF relocation entry */
		}
#if defined(I386) && !defined(ARCH64)
		/*
		 * For 'symbol@TLVP - subtract_symbol' type relocations the
		 * subtract_symbol value is stored in the contents of the item
		 * to be relocated.
		 */
		else if(fixP->fx_r_type == GENERIC_RELOC_TLV){
		    value += fixP->fx_frag->fr_address + where + fixP->fx_size -
			     sub_symbolP->sy_value;
		    fixP->fx_subsy = NULL; /* no SECTDIFF relocation entry */
		    fixP->fx_pcrel = TRUE; /* force pcrel */
		}
#endif
		/*
		 * At this point we have something we can't generate a
		 * relocation entry for (two undefined symbols, etc.).
		 */
	        else{
		     layout_line = fixP->line;
		     layout_file = fixP->file;
		     as_bad("non-relocatable subtraction expression, \"%s\" "
			     "minus \"%s\"", add_symbolP->sy_name,
			     sub_symbolP->sy_name);
		     if((add_symbolP->sy_type & N_TYPE) == N_UNDF)
			as_bad("symbol: \"%s\" can't be undefined in a "
				"subtraction expression", add_symbolP->sy_name);
		     if((sub_symbolP->sy_type & N_TYPE) == N_UNDF)
			as_bad("symbol: \"%s\" can't be undefined in a "
				"subtraction expression", sub_symbolP->sy_name);
		     layout_line = 0;
		     layout_file = NULL;
		}
	    }

	    /*
	     * If a there is an add symbol in the fixup expression then add
	     * the symbol value into the fixup expression's value.
	     */
	    if(add_symbolP){
		/*
		 * If this symbol is in this section and is pc-relative and we
		 * do not want to force a pc-relative relocation entry (to
		 * support scattered loading) then just calculate the value.
		 */
		if(add_symbol_nsect == nsect
		   /* FROM write.c line 2659 */
#ifdef ARM
		   && !TC_FORCE_RELOCATION_LOCAL (fixP)
#else
		   && pcrel
#endif
		   && !(fixP->fx_pcrel_reloc)){
		    /*
		     * This fixup was made when the symbol's section was
		     * unknown, but it is now in this section. So we know how
		     * to do the address without relocation.
		     */
		    value += add_symbolP->sy_value;
#ifdef ARM
		    /* FROM write.c line 2667 */
		    value -= MD_PCREL_FROM_SECTION (fixP, nsect);
#else
		    value -= size + where + fragP->fr_address;
#endif
		    pcrel = 0;	/* Lie. Don't want further pcrel processing. */
		    fixP->fx_addsy = NULL; /* No relocations please. */
		    /*
		     * It would be nice to check that the address does not
		     * overflow.
		     * I didn't do this check because:
		     * +  It is machine dependent in the general case (eg 32032)
		     * +  Compiler output will never need this checking, so why
		     *    slow down the usual case?
		     */
		}
		else{
		    switch(add_symbol_N_TYPE){
		    case N_ABS:
			/*
			 * If the value of the symbol was an expression then
			 * now evaluate the expression now.  This can happen
			 * when symbols like:
			 *	.set x,a-b
			 * are used and the value of x is not known till all
			 * of the symbols are seen and had their values set.
			 */
			if(add_symbolP->expression != NULL){
			    expressionS *exp;

			    exp = (expressionS *)add_symbolP->expression;
			    value +=
				exp->X_add_symbol->sy_value +
				exp->X_add_number -
				exp->X_subtract_symbol->sy_value;
			}
			else
			{
			    value += add_symbolP->sy_value;
			}
			fixP->fx_addsy = NULL; /* no relocation entry */
			add_symbolP = NULL;
			break;
			
		    case N_SECT:
#if (defined(I386) && defined(ARCH64))
			/*
			 * Symbol offsets are not part of fixups for external
			 * symbols for x86_64.
			 */
			if((is_section_debug(nsect) &&
			    add_symbol_N_TYPE != N_UNDF) ||
			   (add_symbol_N_TYPE == N_SECT &&
			    is_local_symbol(add_symbolP) &&
			    !is_section_cstring_literals(add_symbol_nsect)) )
#else
			if(((add_symbolP->sy_type & N_EXT) != N_EXT ||
			    add_symbol_N_TYPE != N_SECT ||
			    !is_section_coalesced(add_symbol_nsect)) &&
			   (add_symbolP->sy_desc & N_WEAK_DEF) != N_WEAK_DEF
#if defined(I386) && !defined(ARCH64)
			   &&
			   fixP->fx_r_type != GENERIC_RELOC_TLV
#endif
			  )
#endif
			    value += add_symbolP->sy_value;
			break;
			
		    case N_UNDF:
			break;
			
		    default:
			BAD_CASE(add_symbol_N_TYPE);
			break;
		    }
		}
	    }
down:
	    /*
	     * If the fixup expression is pc-relative then the value of the pc
	     * will be added to the expression when the machine executes the
	     * the instruction so we adjust the fixup expression's value by
	     * subtracting off the pc value (where) and adjust for insn size.
	     */
	    if(pcrel){
#ifdef ARM
	        /* This should work for both */
	        /* FROM write.c line 2688 */
		value -= MD_PCREL_FROM_SECTION (fixP, nsect);
#elif !(defined(I386) && defined(ARCH64))
		/* Symbol offsets are not part of fixups for x86_64. */
		value -= size + where + fragP->fr_address;
#endif
		if(add_symbolP == NULL){
		    fixP->fx_addsy = &abs_symbol; /* force relocation entry */
		}
	    }

	    if((size == 1 && (value & 0xffffff00) &&
			    ((value & 0xffffff80) != 0xffffff80)) ||
	       (size == 2 && (value & 0xffff0000) &&
			    ((value & 0xffff8000) != 0xffff8000))){
		layout_line = fixP->line;
		layout_file = fixP->file;
		as_bad("Fixup of %lld too large for field width of %d",
			value, size);
		layout_line = 0;
		layout_file = NULL;
	    }

	    /*
	     * Now place the fix expression's value in the place for the size.
	     * And save the fix expression's value to be used when creating
	     * a relocation entry if required.
	     */
	    md_number_to_imm((unsigned char *)place, value, size, fixP, nsect);
	    fixP->fx_value = (int32_t)value;

	    /*
	     * If this is a non-lazy pointer section and this fix is for a
	     * local symbol without an subtract symbol then cause this not to
	     * generate a relocation entry.  This is used with code gen for
	     * fix-n-continue where the compiler generates indirection for
	     * static data references.  So the assembly looks like this:
	     *
	     * 	.non_lazy_symbol_pointer
	     * 	L_i$non_lazy_ptr:
       	     * 	.indirect_symbol _i
       	     * 	.long   _i
	     *
	     * this allows the value of the symbol to be set into the pointer
	     * but not cause the relocation entry to be created.  The code in
	     * write_object() then changes the indirect symbol table entry to
	     * INDIRECT_SYMBOL_LOCAL when the symbol is local.  This is what
	     * the static and dynamic linkers expect and will then cause the
	     * pointer to be correctly relocated.
	     */
	    if(is_section_non_lazy_symbol_pointers(nsect) &&
	       (add_symbolP->sy_type & N_EXT) != N_EXT &&
	       sub_symbolP == NULL){
		fixP->fx_addsy = NULL; /* no relocation entry */
	    }
	}
}

#ifndef SPARC
/*
 * is_assembly_time_constant_subtraction_expression() is passed the symbols and
 * section numbers of a subtraction expression invloving symbols both defined in
 * some section.  If the subtraction expression is an assembly time constant
 * value then this returns 1 (TRUE) else this returns 0 (FALSE).
 *
 * Since the static link editor can break apart a section this routine can only
 * return TRUE when it is known for sure these symbols will not be moved apart
 * from each other.  So this is an assembly time constant subtraction expression
 * if the following are all true:
 * - the expression's symbols are assembly temporary symbols (starting with 'L')
 * - assembly temporary symbol are not being saved (no -L flag)
 * - the two symbols are in the same section
 * - the section is a regular section or coalesced section (non-literal section)
 * - there are no non-assembly temporary symbols defined between two symbols of
 *   the expression.  For example if the assembly code is:
 *	L1: nop
 *	foo: nop
 *	L2: nop
 *   the expression is L1-L2 is not an assembly time constant because the block
 *   of code after foo (including the address of L2) could be link edited away
 *   from the block of code with L1.
 */
static
int
is_assembly_time_constant_subtraction_expression(
symbolS *add_symbolP,
int add_symbol_nsect,
symbolS *sub_symbolP,
int sub_symbol_nsect)
{
    struct frchain *frchainP;
    uint32_t section_type, section_attributes;
    symbolS *prev_symbol;
    int non_assembly_temporary_symbol;

	/* see if both symbols are assembly temporary symbols */
	if(add_symbolP->sy_name == NULL || add_symbolP->sy_name[0] != 'L' ||
	   sub_symbolP->sy_name == NULL || sub_symbolP->sy_name[0] != 'L')
	    return(0);

	/* make sure we are not saving assembly temporary symbols */
	if(flagseen[(int)'L'])
	    return(0);

	/* make sure the two symbols are in the same section */
	if(add_symbol_nsect != sub_symbol_nsect)
	    return(0);

	/* make sure the section is a regular or coalesced section */
	section_attributes = 0;
	for(frchainP = frchain_root; frchainP; frchainP = frchainP->frch_next){
	    if(frchainP->frch_nsect == add_symbol_nsect){
		section_type = frchainP->frch_section.flags & SECTION_TYPE;
		section_attributes = frchainP->frch_section.flags &
				     SECTION_ATTRIBUTES;
		if(section_type == S_REGULAR || section_type == S_COALESCED)
		    break;
		else
		    return(0);
	    }
	}

	/*
	 * See if we can find the chain of symbols from the add_symbolP through
	 * its previous symbols to the sub_symbolP.  And check for non assembler
	 * temporary symbols along that chain.
	 */
	non_assembly_temporary_symbol = 0;
	for(prev_symbol = add_symbolP->sy_prev_by_index;
	    prev_symbol != NULL;
	    prev_symbol = prev_symbol->sy_prev_by_index){
	    if((prev_symbol->sy_type & N_SECT) == N_SECT &&
	       (prev_symbol->sy_type & N_STAB) == 0 &&
		prev_symbol->sy_other == add_symbol_nsect){
		if(prev_symbol == sub_symbolP){
		    if(non_assembly_temporary_symbol == 0)
			return(1);
		    else
			return(0);
		}
		if(prev_symbol->sy_name != NULL &&
		   prev_symbol->sy_name[0] != 'L')
		    non_assembly_temporary_symbol = 1;
	    }
	}

	/*
	 * Couldn't find the chain above, so now try we can find the chain of
	 * symbols from the sub_symbolP through its previous symbols to the
	 * add_symbolP.  And check for non assembler temporary symbols along
	 * that chain.
	 */
	non_assembly_temporary_symbol = 0;
	for(prev_symbol = sub_symbolP->sy_prev_by_index;
	    prev_symbol != NULL;
	    prev_symbol = prev_symbol->sy_prev_by_index){
	    if((prev_symbol->sy_type & N_SECT) == N_SECT &&
	       (prev_symbol->sy_type & N_STAB) == 0 &&
		prev_symbol->sy_other == sub_symbol_nsect){
		if(prev_symbol == add_symbolP){
		    if(non_assembly_temporary_symbol == 0)
			return(1);
		    else
			return(0);
		}
		if(prev_symbol->sy_name != NULL &&
		   prev_symbol->sy_name[0] != 'L')
		    non_assembly_temporary_symbol = 1;
	    }
	}

	/*
	 * It is possible that this expression is coming from a dwarf section
	 * made from .file and .loc directives.  If so both symbols would have
	 * the FAKE_LABEL_NAME and the section_type would be
	 * and in this case the then the expression is an assembly time
	 * constant.
	 */
	if((section_attributes & S_ATTR_DEBUG) == S_ATTR_DEBUG &&
	   strcmp(add_symbolP->sy_name, FAKE_LABEL_NAME) == 0 &&
	   strcmp(sub_symbolP->sy_name, FAKE_LABEL_NAME) == 0)
	    return(1);

	return(0);
}
#endif /* !defined(SPARC) */

/*
 * relax_section() here we set the fr_address values in the frags.
 * After this, all frags in this segment have addresses that are correct
 * relative to the section (that is the section starts at address zero).
 * After all of the sections have been processed by this call and their sizes
 * are know then they can be slid to their final address.
 */
static
int
relax_section(
struct frag *frag_root,
int nsect)
{
    struct frag *fragP;
    relax_addressT address;

    int32_t stretch; /* May be any size, 0 or negative. */
		     /* Cumulative number of addresses we have */
		     /* relaxed this pass. */
		     /* We may have relaxed more than one address. */
    int32_t stretched;  /* Have we stretched on this pass? */
		    /* This is 'cuz stretch may be zero, when,
		       in fact some piece of code grew, and
		       another shrank.  If a branch instruction
		       doesn't fit anymore, we need another pass */

#ifndef ARM
    const relax_typeS *this_type;
    const relax_typeS *start_type;
    relax_substateT next_state;
    relax_substateT this_state;
    int32_t aim;
#endif /* !defined(ARM) */

    uint64_t growth;
    relax_addressT was_address;
    int32_t offset;
    symbolS *symbolP;
    int32_t target;
    uint64_t after;
    relax_addressT oldoff, newoff;
    int ret;

	ret = 0;
	growth = 0;

	/*
	 * For each frag in segment count and store (a 1st guess of) fr_address.
	 */
	address = 0;
	for(fragP = frag_root; fragP != NULL; fragP = fragP->fr_next){
#ifdef ARM
            fragP->relax_marker = 0;
#endif /* ARM */
	    fragP->fr_address = address;
	    address += fragP->fr_fix;
	    switch(fragP->fr_type){
	    case rs_fill:
		address += fragP->fr_offset * fragP->fr_var;
		break;

	    case rs_align:
		offset = (int32_t)relax_align (address, (int) fragP->fr_offset);
		/*
		 * If a maximum number of bytes to fill was specified for this
		 * align (stored in fr_subtype) then check to see if this align
		 * can be done.  If not ignore it.  If so and this alignment is
		 * larger than any previous alignment then this becomes the
		 * section's alignment.
		 */
		if(fragP->fr_subtype != 0){
		    if(offset > (int32_t)fragP->fr_subtype){
			offset = 0;
		    }
		    else{
			if(frchain_now->frch_section.align <
			   (uint32_t)fragP->fr_offset)
			    frchain_now->frch_section.align = fragP->fr_offset;
		    }
		}
		address += offset;
		break;

	    case rs_org:
		/*
		 * Assume .org is nugatory. It will grow with 1st relax.
		 */
		break;

	    case rs_machine_dependent:
		address += md_estimate_size_before_relax(fragP, nsect);
		break;

	    case rs_dwarf2dbg:
		address += dwarf2dbg_estimate_size_before_relax(fragP);
		break;

	    case rs_leb128:
	      /* Initial guess is always 1; doing otherwise can result in
		 stable solutions that are larger than the minimum.  */
	      address += fragP->fr_offset = 1;
	      break;

	    default:
		BAD_CASE(fragP->fr_type);
		break;
	    }
	}

	/*
	 * Do relax().
	 * Make repeated passes over the chain of frags allowing each frag to
	 * grow if needed.  On each pass each frag's address is incremented by
	 * the accumulated growth, kept in stretched.  Passes are continued 
	 * until there is no stretch on the previous pass.
	 */
	do{
	    stretch = 0;
	    stretched = 0;
	    for(fragP = frag_root; fragP != NULL; fragP = fragP->fr_next){
#ifdef ARM
                fragP->relax_marker ^= 1;
#endif /* ARM */
		was_address = (relax_addressT)fragP->fr_address;
		fragP->fr_address += stretch;
		address = (relax_addressT)fragP->fr_address;
		symbolP = fragP->fr_symbol;
		offset = fragP->fr_offset;
		switch(fragP->fr_type){
		case rs_fill:	/* .fill never relaxes. */
		    growth = 0;
		    break;

		case rs_align:
		    oldoff = relax_align(was_address + fragP->fr_fix, offset);
		    newoff = relax_align(address + fragP->fr_fix, offset);
		    /*
		     * Check if a maximum number of bytes to fill was specified
		     * for this align (stored in fr_subtype).
		     */
		    if(fragP->fr_subtype != 0){
			if(oldoff > fragP->fr_subtype)
			    oldoff = 0;
			if(newoff > fragP->fr_subtype)
			    newoff = 0;
		    }
		    growth = newoff - oldoff;
		    break;

		case rs_org:
		    target = offset;
		    if(symbolP != NULL){
			know(((symbolP->sy_type & N_TYPE) == N_ABS) ||
			     ((symbolP->sy_type & N_TYPE) == N_SECT));
			know(symbolP->sy_frag);
			know((symbolP->sy_type & N_TYPE) != N_ABS ||
			     symbolP->sy_frag == &zero_address_frag );
			target += symbolP->sy_value +
				  symbolP->sy_frag->fr_address;
		    }
		    know(fragP->fr_next);
		    after = fragP->fr_next->fr_address;
		    /*
		     * Growth may be negative, but variable part of frag cannot
		     * have < 0 chars. That is, we can't .org backwards.
		     */
		    growth = ((target - after ) > 0) ? (target - after) : 0;

		    growth -= stretch;	/* This is an absolute growth factor */
		    break;

		case rs_machine_dependent:
#ifdef ARM
		    growth = arm_relax_frag(nsect, fragP, stretch);
#else /* !defined(ARM) */
		    this_state = fragP->fr_subtype;
		    this_type = md_relax_table + this_state;
		    start_type = this_type;

		    target = offset;
		    if(symbolP){
			know(((symbolP->sy_type & N_TYPE) == N_ABS) ||
			     ((symbolP->sy_type & N_TYPE) == N_SECT));
			know(symbolP->sy_frag);
			know((symbolP->sy_type & N_TYPE) != N_ABS ||
			     symbolP->sy_frag == &zero_address_frag);

			target += symbolP->sy_value +
				  symbolP->sy_frag->fr_address;
			/*
			 * If frag has yet to be reached on this pass,
			 * assume it will move by STRETCH just as we did.
			 * If this is not so, it will be because some frag
			 * between grows, and that will force another pass.
			 */
			if(symbolP->sy_frag->fr_address >= was_address &&
			   is_down_range(fragP, symbolP->sy_frag))
			    target += stretch;
		    }
		    aim = (int32_t)(target - address - fragP->fr_fix);
		    if(aim < 0){
			/* Look backwards. */
			for(next_state = this_type->rlx_more; next_state; ){
			    if(aim >= this_type->rlx_backward)
				next_state = 0;
			    else{	/* Grow to next state. */
				this_state = next_state;
				this_type = md_relax_table + this_state;
				next_state = this_type->rlx_more;
			    }
			}
		    }
		    else{
			/* Look forwards. */
			for(next_state = this_type->rlx_more; next_state; ){
			    if(aim <= this_type->rlx_forward)
				next_state = 0;
			    else{	/* Grow to next state. */
				this_state = next_state;
				this_type = md_relax_table + this_state;
				next_state = this_type->rlx_more;
			    }
			}
		    }
		    if((growth = this_type->rlx_length -start_type->rlx_length))
			  fragP->fr_subtype = this_state;
#endif /* !defined(ARM) */
		    break;
		  case rs_dwarf2dbg:
		      growth = dwarf2dbg_relax_frag(fragP);
		      break;

		  case rs_leb128:
		    {
		      valueT value;
		      offsetT size;
#ifdef OLD
		      value = resolve_symbol_value (fragP->fr_symbol);
#else
  		      expressionS *expression;
  		
		      if(fragP->fr_symbol->expression != NULL){
			expression =
			  (expressionS *)fragP->fr_symbol->expression;
			value = 0;
			if(expression->X_add_symbol != NULL)
			    value +=
			     (expression->X_add_symbol->sy_nlist.n_value +
			      expression->X_add_symbol->sy_frag->fr_address);
			if(expression->X_subtract_symbol != NULL)
			   value -= 
			     (expression->X_subtract_symbol->sy_nlist.n_value +
			      expression->X_subtract_symbol->
							   sy_frag->fr_address);
			value += expression->X_add_number;
		      }
		      else{
			value = (valueT)(fragP->fr_symbol->sy_nlist.n_value +
					 fragP->fr_address);
		      }
#endif
		      size = sizeof_leb128 (value, fragP->fr_subtype);
		      growth = size - fragP->fr_offset;
		      fragP->fr_offset = (int32_t)size;
		    }
		    break;

		  default:
		      BAD_CASE(fragP->fr_type);
		      break;
		}
		if(growth) {
		    stretch += growth;
		    stretched++;
		}
	    }			/* For each frag in the segment. */
	}while(stretched);	/* Until nothing further to relax. */

	/*
	 * We now have valid fr_address'es for each frag.  All fr_address's
	 * are correct, relative to their own section.  We have made all the
	 * fixS for this section that will be made.
	 */

	for(fragP = frag_root; fragP != NULL; fragP = fragP->fr_next){
	    if(fragP->last_fr_address != fragP->fr_address){
		fragP->last_fr_address = fragP->fr_address;
		ret = 1;
	    }
	}
	return(ret);
}

/*
 * Relax_align. Advance location counter to next address that has 'alignment'
 * lowest order bits all 0s.
 */
static
relax_addressT		/* How many addresses does the .align take? */
relax_align(
relax_addressT address, /* Address now. */
uint32_t alignment)		/* Alignment (binary). */
{
    relax_addressT mask;
    relax_addressT new_address;

	mask = ~ ( (~0) << alignment );
	new_address = (address + mask) & (~ mask);
	return(new_address - address);
}

#ifndef ARM
/*
 * is_down_range() is used in relax_section() to determine it one fragment is
 * after another to know if it will also be moved if the first is moved.
 */
static
int
is_down_range(
struct frag *f1,
struct frag *f2)
{
	while(f1){
	    if(f1->fr_next == f2)
		return(1);
	    f1 = f1->fr_next;
	}
	return(0);
}
#endif /* !defined(ARM) */
