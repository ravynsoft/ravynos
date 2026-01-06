/*

  Copyright (C) 2000,2002,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU Lesser General Public 
  License along with this program; if not, write the Free Software 
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
  USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/



#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include "dwarf_global.h"


#ifdef __sgi			/* __sgi should only be defined for
				   IRIX/MIPS. */
/* The 'fixup' here intended for IRIX targets only.
   With a  2+GB Elf64 IRIX executable (under 4GB in size),  
   some DIE offsets wrongly
   got the 32bit upper bit sign extended.  For the cu-header
   offset in the .debug_pubnames section  and in the
   .debug_aranges section.
   the 'varp' here is a pointer to an offset into .debug_info.
   We fix up the offset here if it seems advisable..
   
   As of June 2005 we have identified a series of mistakes
   in ldx64 that can cause this (64 bit values getting passed
   thru 32-bit signed knothole).  
*/
void
_dwarf_fix_up_offset_irix(Dwarf_Debug dbg,
			  Dwarf_Unsigned * varp, char *caller_site_name)
{

    Dwarf_Unsigned var = *varp;

#define UPPER33 0xffffffff80000000LL
#define LOWER32         0xffffffffLL
    /* Restrict the hack to the known case. Upper 32 bits erroneously
       sign extended from lower 32 upper bit. */
    if ((var & UPPER33) == UPPER33) {
	var &= LOWER32;
	/* Apply the fix. Dreadful hack. */
	*varp = var;
    }
#undef UPPER33
#undef LOWER32
    return;
}
#endif


int
dwarf_get_globals(Dwarf_Debug dbg,
		  Dwarf_Global ** globals,
		  Dwarf_Signed * return_count, Dwarf_Error * error)
{
    int res;

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_pubnames_index,
			    &dbg->de_debug_pubnames, error);
    if (res != DW_DLV_OK) {
	return res;
    }



    return _dwarf_internal_get_pubnames_like_data(dbg,
						  dbg->
						  de_debug_pubnames,
						  dbg->
						  de_debug_pubnames_size,
						  globals, return_count,
						  error,
						  DW_DLA_GLOBAL_CONTEXT,
						  DW_DLA_GLOBAL,
						  DW_DLE_PUBNAMES_LENGTH_BAD,
						  DW_DLE_PUBNAMES_VERSION_ERROR);

}

/* Deallocating fully requires deallocating the list
   and all entries.  But some internal data is
   not exposed, so we need a function with internal knowledge.
*/

void
dwarf_globals_dealloc(Dwarf_Debug dbg, Dwarf_Global * dwgl,
		      Dwarf_Signed count)
{
    _dwarf_internal_globals_dealloc(dbg, dwgl,
				    count,
				    DW_DLA_GLOBAL_CONTEXT,
				    DW_DLA_GLOBAL, DW_DLA_LIST);
    return;
}

void
_dwarf_internal_globals_dealloc(Dwarf_Debug dbg, Dwarf_Global * dwgl,
				Dwarf_Signed count,
				int context_code,
				int global_code, int list_code)
{
    Dwarf_Signed i;
    struct Dwarf_Global_Context_s *gcp = 0;
    struct Dwarf_Global_Context_s *lastgcp = 0;

    for (i = 0; i < count; i++) {
	Dwarf_Global dgb = dwgl[i];

	gcp = dgb->gl_context;

	if (lastgcp != gcp) {
	    lastgcp = gcp;
	    dwarf_dealloc(dbg, gcp, context_code);
	}
	dwarf_dealloc(dbg, dgb, global_code);
    }
    dwarf_dealloc(dbg, dwgl, list_code);
    return;
}


/* Sweeps the complete  section. 
*/
int
_dwarf_internal_get_pubnames_like_data(Dwarf_Debug dbg,
				       Dwarf_Small * section_data_ptr,
				       Dwarf_Unsigned section_length,
				       Dwarf_Global ** globals,
				       Dwarf_Signed * return_count,
				       Dwarf_Error * error,
				       int context_code,
				       int global_code,
				       int length_err_num,
				       int version_err_num)
{


    Dwarf_Small *pubnames_like_ptr;



    /* 
       Points to the context for the current set of global names, and
       contains information to identify the compilation-unit that the
       set refers to. */
    Dwarf_Global_Context pubnames_context;

    Dwarf_Half version;

    /* 
       Offset from the start of compilation-unit for the current
       global. */
    Dwarf_Off die_offset_in_cu;

    Dwarf_Unsigned global_count = 0;

    /* Points to the current global read. */
    Dwarf_Global global;

    /* 
       Used to chain the Dwarf_Global_s structs for creating contiguous 
       list of pointers to the structs. */
    Dwarf_Chain curr_chain, prev_chain, head_chain = NULL;

    /* Points to contiguous block of Dwarf_Global's to be returned. */
    Dwarf_Global *ret_globals;

    /* Temporary counter. */
    Dwarf_Unsigned i;




    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    /* We will eventually need the .debug_info data. Load it now. */
    if (!dbg->de_debug_info) {
	int res = _dwarf_load_debug_info(dbg, error);

	if (res != DW_DLV_OK) {
	    return res;
	}
    }

    if (section_data_ptr == NULL) {
	return (DW_DLV_NO_ENTRY);
    }

    pubnames_like_ptr = section_data_ptr;
    do {
	Dwarf_Unsigned length;
	int local_extension_size;
	int local_length_size;

	/* Some compilers emit padding at the end of each cu's area.
	   pubnames_ptr_past_end_cu records the true area end for this
	   cu's data.  Essentially the length in the header and the 0
	   terminator of the data are redundant information. The
	   dwarf2/3 spec does not mention what to do if the length is
	   past the 0 terminator. So we take any bytes left after the 0 
	   as padding and ignore them. */
	Dwarf_Small *pubnames_ptr_past_end_cu = 0;


	pubnames_context = (Dwarf_Global_Context)
	    _dwarf_get_alloc(dbg, context_code, 1);
	if (pubnames_context == NULL) {
	    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	    return (DW_DLV_ERROR);
	}
	/* READ_AREA_LENGTH updates pubnames_like_ptr for consumed
	   bytes */
	READ_AREA_LENGTH(dbg, length, Dwarf_Unsigned,
			 pubnames_like_ptr, local_length_size,
			 local_extension_size);
	pubnames_context->pu_length_size = local_length_size;
	pubnames_context->pu_extension_size = local_extension_size;
	pubnames_context->pu_dbg = dbg;

	pubnames_ptr_past_end_cu = pubnames_like_ptr + length;

	READ_UNALIGNED(dbg, version, Dwarf_Half,
		       pubnames_like_ptr, sizeof(Dwarf_Half));
	pubnames_like_ptr += sizeof(Dwarf_Half);
	if (version != CURRENT_VERSION_STAMP) {
	    _dwarf_error(dbg, error, version_err_num);
	    return (DW_DLV_ERROR);
	}

	/* offset of CU header in debug section */
	READ_UNALIGNED(dbg, pubnames_context->pu_offset_of_cu_header,
		       Dwarf_Off, pubnames_like_ptr,
		       pubnames_context->pu_length_size);
	pubnames_like_ptr += pubnames_context->pu_length_size;

	FIX_UP_OFFSET_IRIX_BUG(dbg,
			       pubnames_context->pu_offset_of_cu_header,
			       "pubnames cu header offset");


	READ_UNALIGNED(dbg, pubnames_context->pu_info_length,
		       Dwarf_Unsigned, pubnames_like_ptr,
		       pubnames_context->pu_length_size);
	pubnames_like_ptr += pubnames_context->pu_length_size;

	if (pubnames_like_ptr > (section_data_ptr + section_length)) {
	    _dwarf_error(dbg, error, length_err_num);
	    return (DW_DLV_ERROR);
	}

	/* read initial offset (of DIE within CU) of a pubname, final
	   entry is not a pair, just a zero offset */
	READ_UNALIGNED(dbg, die_offset_in_cu, Dwarf_Off,
		       pubnames_like_ptr,
		       pubnames_context->pu_length_size);
	pubnames_like_ptr += pubnames_context->pu_length_size;
	FIX_UP_OFFSET_IRIX_BUG(dbg,
			       die_offset_in_cu, "offset of die in cu");

	/* loop thru pairs. DIE off with CU followed by string */
	while (die_offset_in_cu != 0) {

	    /* Already read offset, pubnames_like_ptr now points to the 
	       string */
	    global =
		(Dwarf_Global) _dwarf_get_alloc(dbg, global_code, 1);
	    if (global == NULL) {
		_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
		return (DW_DLV_ERROR);
	    }
	    global_count++;

	    global->gl_context = pubnames_context;

	    global->gl_named_die_offset_within_cu = die_offset_in_cu;

	    global->gl_name = pubnames_like_ptr;

	    pubnames_like_ptr = pubnames_like_ptr +
		strlen((char *) pubnames_like_ptr) + 1;


	    /* finish off current entry chain */
	    curr_chain =
		(Dwarf_Chain) _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
	    if (curr_chain == NULL) {
		_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
		return (DW_DLV_ERROR);
	    }

	    /* Put current global on singly_linked list. */
	    curr_chain->ch_item = (Dwarf_Global) global;

	    if (head_chain == NULL)
		head_chain = prev_chain = curr_chain;
	    else {
		prev_chain->ch_next = curr_chain;
		prev_chain = curr_chain;
	    }

	    /* read offset for the *next* entry */
	    READ_UNALIGNED(dbg, die_offset_in_cu, Dwarf_Off,
			   pubnames_like_ptr,
			   pubnames_context->pu_length_size);

	    pubnames_like_ptr += pubnames_context->pu_length_size;
	    FIX_UP_OFFSET_IRIX_BUG(dbg,
				   die_offset_in_cu,
				   "offset of next die in cu");

	    if (pubnames_like_ptr > (section_data_ptr + section_length)) {
		_dwarf_error(dbg, error, length_err_num);
		return (DW_DLV_ERROR);
	    }
	}
	/* ASSERT: die_offset_in_cu == 0 */
	if (pubnames_like_ptr > pubnames_ptr_past_end_cu) {
	    /* This is some kind of error. This simply cannot happen.
	       The encoding is wrong or the length in the header for
	       this cu's contribution is wrong. */
	    _dwarf_error(dbg, error, length_err_num);
	    return (DW_DLV_ERROR);

	}
	/* If there is some kind of padding at the end of the section,
	   as emitted by some compilers, skip over that padding and
	   simply ignore the bytes thus passed-over.  With most
	   compilers, pubnames_like_ptr == pubnames_ptr_past_end_cu at
	   this point */
	pubnames_like_ptr = pubnames_ptr_past_end_cu;

    } while (pubnames_like_ptr < (section_data_ptr + section_length));

    /* Points to contiguous block of Dwarf_Global's. */
    ret_globals = (Dwarf_Global *)
	_dwarf_get_alloc(dbg, DW_DLA_LIST, global_count);
    if (ret_globals == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    /* 
       Store pointers to Dwarf_Global_s structs in contiguous block,
       and deallocate the chain. */
    curr_chain = head_chain;
    for (i = 0; i < global_count; i++) {
	*(ret_globals + i) = curr_chain->ch_item;
	prev_chain = curr_chain;
	curr_chain = curr_chain->ch_next;
	dwarf_dealloc(dbg, prev_chain, DW_DLA_CHAIN);
    }

    *globals = ret_globals;
    *return_count = (Dwarf_Signed) global_count;
    return DW_DLV_OK;
}


/*
	Given a pubnames entry (or other like section entry)
	return thru the ret_name pointer
	a pointer to the string which is the entry name.
	
*/
int
dwarf_globname(Dwarf_Global glob, char **ret_name, Dwarf_Error * error)
{
    if (glob == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_NULL);
	return (DW_DLV_ERROR);
    }

    *ret_name = (char *) (glob->gl_name);
    return DW_DLV_OK;
}


/*
	Given a pubnames entry (or other like section entry)
	return thru the ret_off pointer the
	global offset of the DIE for this entry.
	The global offset is the offset within the .debug_info
	section as a whole.
*/
int
dwarf_global_die_offset(Dwarf_Global global,
			Dwarf_Off * ret_off, Dwarf_Error * error)
{
    if (global == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_NULL);
	return (DW_DLV_ERROR);
    }

    if (global->gl_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_CONTEXT_NULL);
	return (DW_DLV_ERROR);
    }

    *ret_off = (global->gl_named_die_offset_within_cu +
		global->gl_context->pu_offset_of_cu_header);
    return DW_DLV_OK;
}

/*
	Given a pubnames entry (or other like section entry)
	return thru the ret_off pointer the
	offset of the compilation unit header of the
        compilation unit the global is part of.

	In early versions of this, the value returned was
        the offset of the compilation unit die, and
	other cu-local die offsets were faked so adding this to 
        such a cu-local offset got a true section offset.
        Now things do as they say (adding *cu_header_offset to
        a cu-local offset gets the section offset).

*/
int
dwarf_global_cu_offset(Dwarf_Global global,
		       Dwarf_Off * cu_header_offset,
		       Dwarf_Error * error)
{
    Dwarf_Global_Context con;

    if (global == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_NULL);
	return (DW_DLV_ERROR);
    }

    con = global->gl_context;

    if (con == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_CONTEXT_NULL);
	return (DW_DLV_ERROR);
    }

    /* In early libdwarf, this incorrectly returned the offset of the
       CU DIE. Now correctly returns the header offset. */
    *cu_header_offset = con->pu_offset_of_cu_header;

    return DW_DLV_OK;
}

/*
  Give back the pubnames entry (or any other like section)
  name, symbol DIE offset, and the cu-DIE offset.

  Various errors are possible.

  The string pointer returned thru ret_name is not
  dwarf_get_alloc()ed, so no dwarf_dealloc() 
  DW_DLA_STRING should be applied to it.

*/
int
dwarf_global_name_offsets(Dwarf_Global global,
			  char **ret_name,
			  Dwarf_Off * die_offset,
			  Dwarf_Off * cu_die_offset,
			  Dwarf_Error * error)
{
    Dwarf_Global_Context con;
    Dwarf_Debug dbg;
    Dwarf_Off off;

    if (global == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_NULL);
	return (DW_DLV_ERROR);
    }

    con = global->gl_context;

    if (con == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_CONTEXT_NULL);
	return (DW_DLV_ERROR);
    }

    off = con->pu_offset_of_cu_header;
    /* The offset had better not be too close to the end. If it is,
       _dwarf_length_of_cu_header() will step off the end and therefore 
       must not be used. 10 is a meaningless heuristic, but no CU
       header is that small so it is safe. An erroneous offset is due
       to a bug in the tool chain. A bug like this has been seen on
       IRIX with MIPSpro 7.3.1.3 and an executable > 2GB in size and
       with 2 million pubnames entries. */
#define MIN_CU_HDR_SIZE 10
    dbg = con->pu_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    if (dbg->de_debug_info_size &&
	((off + MIN_CU_HDR_SIZE) >= dbg->de_debug_info_size)) {
	_dwarf_error(NULL, error, DW_DLE_OFFSET_BAD);
	return (DW_DLV_ERROR);
    }
#undef MIN_CU_HDR_SIZE
    if (die_offset != NULL) {
	*die_offset = global->gl_named_die_offset_within_cu + off;
    }

    *ret_name = (char *) global->gl_name;

    if (cu_die_offset != NULL) {
	int res = _dwarf_load_debug_info(dbg, error);

	if (res != DW_DLV_OK) {
	    return res;
	}
	/* The offset had better not be too close to the end. If it is, 
	   _dwarf_length_of_cu_header() will step off the end and
	   therefore must not be used. 10 is a meaningless heuristic,
	   but no CU header is that small so it is safe. */
	if ((off + 10) >= dbg->de_debug_info_size) {
	    _dwarf_error(NULL, error, DW_DLE_OFFSET_BAD);
	    return (DW_DLV_ERROR);
	}
	*cu_die_offset = off + _dwarf_length_of_cu_header(dbg, off);
    }


    return DW_DLV_OK;
}

/*
	We have the offset to a CU header.
	Return thru outFileOffset the offset of the CU DIE.
	
	New June, 2001.
	Used by SGI debuggers.
	No error is possible.
*/

/* ARGSUSED */
int
dwarf_get_cu_die_offset_given_cu_header_offset(Dwarf_Debug dbg,
					       Dwarf_Off
					       in_cu_header_offset,
					       Dwarf_Off *
					       out_cu_die_offset,
					       Dwarf_Error * err)
{
#pragma unused(err)
    Dwarf_Off len =
	_dwarf_length_of_cu_header(dbg, in_cu_header_offset);

    Dwarf_Off newoff = in_cu_header_offset + len;

    *out_cu_die_offset = newoff;
    return DW_DLV_OK;
}
