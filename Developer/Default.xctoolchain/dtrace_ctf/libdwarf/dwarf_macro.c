/*

  Copyright (C) 2000,2002,2004 Silicon Graphics, Inc.  All Rights Reserved.

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
#include <limits.h>
#include "dwarf_macro.h"


#define LEFTPAREN '('
#define RIGHTPAREN ')'
#define SPACE ' '

/*
	Given the dwarf macro string, return a pointer to
	the value.  Returns pointer to 0 byte at end of string
	if no value found (meaning the value is the empty string).

	Only understands well-formed dwarf macinfo strings.
*/
char *
dwarf_find_macro_value_start(char *str)
{
    char *lcp;
    int funclike = 0;

    for (lcp = str; *lcp; ++lcp) {
	switch (*lcp) {
	case LEFTPAREN:
	    funclike = 1;
	    break;
	case RIGHTPAREN:
	    /* lcp+1 must be a space, and following char is the value */
	    return lcp + 2;
	case SPACE:
	    /* we allow extraneous spaces inside macro parameter **
	       list, just in case... This is not really needed. */
	    if (!funclike) {
		return lcp + 1;
	    }
	    break;
	}
    }
    /* never found value: returns pointer to the 0 byte at end of
       string */
    return lcp;

}


/*
   Try to keep fileindex correct in every Macro_Details
   record by tracking file starts and ends.
   Uses high water mark: space reused, not freed.
   Presumption is that this makes sense for most uses.
   STARTERMAX is set so that the array need not be expanded for
   most files: it is the initial include file depth.
*/
static Dwarf_Signed *st_base;
static long max;
static long next_to_use;
static int was_fault = 0;

#define STARTERMAX 10
static void
_dwarf_reset_index_stack(void)
{
    next_to_use = 0;
    was_fault = 0;
}
static int
_dwarf_mac_push_index(Dwarf_Debug dbg, Dwarf_Signed indx)
{
    Dwarf_Signed *newbase;

    if (next_to_use >= max) {
	long new_size;

	if (max == 0) {
	    max = STARTERMAX;
	}
	new_size = max * 2;
	newbase =
	    _dwarf_get_alloc(dbg, DW_DLA_STRING,
			     new_size * sizeof(Dwarf_Signed));
	if (newbase == 0) {
	    /* just leave the old array in place */
	    was_fault = 1;
	    return DW_DLV_ERROR;
	}
	memcpy(newbase, st_base, next_to_use * sizeof(Dwarf_Signed));
	dwarf_dealloc(dbg, st_base, DW_DLA_STRING);
	st_base = newbase;
	max = new_size;
    }
    st_base[next_to_use] = indx;
    ++next_to_use;
    return DW_DLV_OK;
}

static Dwarf_Signed
_dwarf_mac_pop_index(void)
{
    if (was_fault) {
	return -1;
    }
    if (next_to_use > 0) {
	next_to_use--;
	return (*(st_base + next_to_use));
    }
    return -1;
}

/* starting at macro_offset in .debug_macinfo,
	if maximum_count is 0, treat as if it is infinite.
	get macro data up thru
	maximum_count entries or the end of a compilation
	unit's entries (whichever comes first). 
*/

int
dwarf_get_macro_details(Dwarf_Debug dbg,
			Dwarf_Off macro_offset,
			Dwarf_Unsigned maximum_count,
			Dwarf_Signed * entry_count,
			Dwarf_Macro_Details ** details,
			Dwarf_Error * error)
{
    Dwarf_Small *macro_base = 0;
    Dwarf_Small *pnext = 0;
    Dwarf_Unsigned endloc = 0;
    unsigned char uc = 0;
    unsigned long depth = 0;	/* By section 6.3.2 Dwarf3 draft 8/9,
				   the base file should appear as
				   DW_MACINFO_start_file. See
				   http://gcc.gnu.org/ml/gcc-bugs/2005-02/msg03442.html
				   on "[Bug debug/20253] New: [3.4/4.0 regression]:
				   Macro debug info broken due to lexer change" for how
				   gcc is broken in some versions. We no longer use
				   depth as a stopping point, it's not needed as a
				   stopping point anyway.  */


    int res = 0;

    /* count space used by strings */
    unsigned long str_space = 0;
    int done = 0;
    unsigned long space_needed = 0;
    unsigned long string_offset = 0;
    Dwarf_Small *return_data = 0;
    Dwarf_Small *pdata = 0;
    unsigned long final_count = 0;
    Dwarf_Signed fileindex = -1;
    Dwarf_Small *latest_str_loc = 0;

    unsigned long count = 0;
    unsigned long max_count = (unsigned long) maximum_count;

    _dwarf_reset_index_stack();
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_macinfo_index,
			    &dbg->de_debug_macinfo, error);
    if (res != DW_DLV_OK) {
	return res;
    }

    macro_base = dbg->de_debug_macinfo;
    if (macro_base == NULL) {
	return (DW_DLV_NO_ENTRY);
    }
    if (macro_offset >= dbg->de_debug_macinfo_size) {
	return (DW_DLV_NO_ENTRY);
    }

    pnext = macro_base + macro_offset;
    if (maximum_count == 0) {
	max_count = ULONG_MAX;
    }


    /* how many entries and how much space will they take? */

    endloc = (pnext - macro_base);
    if (endloc >= dbg->de_debug_macinfo_size) {
	if (endloc == dbg->de_debug_macinfo_size) {
	    /* normal: found last entry */
	    return DW_DLV_NO_ENTRY;
	}
	_dwarf_error(dbg, error, DW_DLE_DEBUG_MACRO_LENGTH_BAD);
	return (DW_DLV_ERROR);
    }
    for (count = 0; !done && count < max_count; ++count) {
	unsigned long slen;
	Dwarf_Word len;

	uc = *pnext;
	++pnext;		/* get past the type code */
	switch (uc) {
	case DW_MACINFO_define:
	case DW_MACINFO_undef:
	    /* line, string */
	case DW_MACINFO_vendor_ext:
	    /* number, string */
	    (void) _dwarf_decode_u_leb128(pnext, &len);

	    pnext += len;
	    if (((pnext - macro_base)) >= dbg->de_debug_macinfo_size) {
		_dwarf_error(dbg, error,
			     DW_DLE_DEBUG_MACRO_INCONSISTENT);
		return (DW_DLV_ERROR);
	    }
	    slen = strlen((char *) pnext) + 1;
	    pnext += slen;
	    if (((pnext - macro_base)) >= dbg->de_debug_macinfo_size) {
		_dwarf_error(dbg, error,
			     DW_DLE_DEBUG_MACRO_INCONSISTENT);
		return (DW_DLV_ERROR);
	    }
	    str_space += slen;
	    break;
	case DW_MACINFO_start_file:
	    /* line, file index */
	    (void) _dwarf_decode_u_leb128(pnext, &len);
	    pnext += len;
	    if (((pnext - macro_base)) >= dbg->de_debug_macinfo_size) {
		_dwarf_error(dbg, error,
			     DW_DLE_DEBUG_MACRO_INCONSISTENT);
		return (DW_DLV_ERROR);
	    }
	    (void) _dwarf_decode_u_leb128(pnext, &len);
	    pnext += len;
	    if (((pnext - macro_base)) >= dbg->de_debug_macinfo_size) {
		_dwarf_error(dbg, error,
			     DW_DLE_DEBUG_MACRO_INCONSISTENT);
		return (DW_DLV_ERROR);
	    }
	    ++depth;
	    break;

	case DW_MACINFO_end_file:
	    if (--depth == 0) {
		/* done = 1; no, do not stop here, at least one gcc had 
		   the wrong depth settings in the gcc 3.4 timeframe. */
	    }
	    break;		/* no string or number here */
	case 0:
	    /* end of cu's entries */
	    done = 1;
	    break;
	default:
	    _dwarf_error(dbg, error, DW_DLE_DEBUG_MACRO_INCONSISTENT);
	    return (DW_DLV_ERROR);
	    /* bogus macinfo! */
	}

	endloc = (pnext - macro_base);
	if (endloc == dbg->de_debug_macinfo_size) {
	    done = 1;
	} else if (endloc > dbg->de_debug_macinfo_size) {
	    _dwarf_error(dbg, error, DW_DLE_DEBUG_MACRO_LENGTH_BAD);
	    return (DW_DLV_ERROR);
	}
    }
    if (count == 0) {
	_dwarf_error(dbg, error, DW_DLE_DEBUG_MACRO_INTERNAL_ERR);
	return (DW_DLV_ERROR);
    }

    /* we have 'count' array entries to allocate and str_space bytes of 
       string space to provide for. */

    string_offset = count * sizeof(Dwarf_Macro_Details);

    /* extra 2 not really needed */
    space_needed = string_offset + str_space + 2;
    return_data = pdata =
	_dwarf_get_alloc(dbg, DW_DLA_STRING, space_needed);
    latest_str_loc = pdata + string_offset;
    if (pdata == 0) {
	_dwarf_error(dbg, error, DW_DLE_DEBUG_MACRO_MALLOC_SPACE);
	return (DW_DLV_ERROR);
    }
    pnext = macro_base + macro_offset;

    done = 0;

    /* A series ends with a type code of 0. */

    for (final_count = 0; !done && final_count < count; ++final_count) {
	unsigned long slen;
	Dwarf_Word len;
	Dwarf_Unsigned v1;
	Dwarf_Macro_Details *pdmd = (Dwarf_Macro_Details *) (pdata +
							     final_count
							     *
							     sizeof
							     (Dwarf_Macro_Details));

	endloc = (pnext - macro_base);
	if (endloc > dbg->de_debug_macinfo_size) {
	    _dwarf_error(dbg, error, DW_DLE_DEBUG_MACRO_LENGTH_BAD);
	    return (DW_DLV_ERROR);
	}
	uc = *pnext;
	pdmd->dmd_offset = (pnext - macro_base);
	pdmd->dmd_type = uc;
	pdmd->dmd_fileindex = fileindex;
	pdmd->dmd_lineno = 0;
	pdmd->dmd_macro = 0;
	++pnext;		/* get past the type code */
	switch (uc) {
	case DW_MACINFO_define:
	case DW_MACINFO_undef:
	    /* line, string */
	case DW_MACINFO_vendor_ext:
	    /* number, string */
	    v1 = _dwarf_decode_u_leb128(pnext, &len);
	    pdmd->dmd_lineno = v1;

	    pnext += len;
	    if (((pnext - macro_base)) >= dbg->de_debug_macinfo_size) {
		_dwarf_error(dbg, error,
			     DW_DLE_DEBUG_MACRO_INCONSISTENT);
		return (DW_DLV_ERROR);
	    }
	    slen = strlen((char *) pnext) + 1;
	    strcpy((char *) latest_str_loc, (char *) pnext);
	    pdmd->dmd_macro = (char *) latest_str_loc;
	    latest_str_loc += slen;
	    pnext += slen;
	    if (((pnext - macro_base)) >= dbg->de_debug_macinfo_size) {
		_dwarf_error(dbg, error,
			     DW_DLE_DEBUG_MACRO_INCONSISTENT);
		return (DW_DLV_ERROR);
	    }
	    str_space += slen;
	    break;
	case DW_MACINFO_start_file:
	    /* line, file index */
	    v1 = _dwarf_decode_u_leb128(pnext, &len);
	    pdmd->dmd_lineno = v1;
	    pnext += len;
	    if (((pnext - macro_base)) >= dbg->de_debug_macinfo_size) {
		_dwarf_error(dbg, error,
			     DW_DLE_DEBUG_MACRO_INCONSISTENT);
		return (DW_DLV_ERROR);
	    }
	    v1 = _dwarf_decode_u_leb128(pnext, &len);
	    pdmd->dmd_fileindex = v1;
	    (void) _dwarf_mac_push_index(dbg, fileindex);
	    /* we ignore the error, we just let fileindex ** be -1 when 
	       we pop this one */
	    fileindex = v1;
	    pnext += len;
	    if (((pnext - macro_base)) >= dbg->de_debug_macinfo_size) {
		_dwarf_error(dbg, error,
			     DW_DLE_DEBUG_MACRO_INCONSISTENT);
		return (DW_DLV_ERROR);
	    }
	    break;

	case DW_MACINFO_end_file:
	    fileindex = _dwarf_mac_pop_index();
	    break;		/* no string or number here */
	case 0:
	    /* end of cu's entries */
	    done = 1;
	    break;
	default:
	    /* FIXME: leaks memory via return_data. */
	    _dwarf_error(dbg, error, DW_DLE_DEBUG_MACRO_INCONSISTENT);
	    return (DW_DLV_ERROR);
	    /* bogus macinfo! */
	}
    }
    *entry_count = count;
    *details = (Dwarf_Macro_Details *) return_data;

    return DW_DLV_OK;
}
