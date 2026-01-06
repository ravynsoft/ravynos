/*

  Copyright (C) 2000,2002,2004,2005,2006 Silicon Graphics, Inc.  All Rights Reserved.

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

/* This source file used for SGI-IRIX rqs processing.
   Unused otherwise.
*/


#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include "dwarf_line.h"
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif



/*
	Return DW_DLV_OK or, if error,
	DW_DLV_ERROR.

	Thru pointers, return 2 arrays and a count
	for rqs.
*/
int
_dwarf_line_address_offsets(Dwarf_Debug dbg,
			    Dwarf_Die die,
			    Dwarf_Addr ** addrs,
			    Dwarf_Off ** offs,
			    Dwarf_Unsigned * returncount,
			    Dwarf_Error * err)
{
    Dwarf_Addr *laddrs;
    Dwarf_Off *loffsets;
    Dwarf_Signed lcount;
    Dwarf_Signed i;
    int res;
    Dwarf_Line *linebuf;

    res = _dwarf_internal_srclines(die, &linebuf, &lcount,	/* addrlist= 
								 */ true,
				   /* linelist= */ false, err);
    if (res != DW_DLV_OK) {
	return res;
    }
    laddrs = (Dwarf_Addr *)
	_dwarf_get_alloc(dbg, DW_DLA_ADDR, lcount);
    if (laddrs == NULL) {
	dwarf_srclines_dealloc(dbg, linebuf, lcount);
	_dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }
    loffsets = (Dwarf_Off *)
	_dwarf_get_alloc(dbg, DW_DLA_ADDR, lcount);
    if (loffsets == NULL) {
	dwarf_srclines_dealloc(dbg, linebuf, lcount);
	/* We already allocated what laddrs points at, so we'e better
	   deallocate that space since we are not going to return the
	   pointer to the caller. */
	dwarf_dealloc(dbg, laddrs, DW_DLA_ADDR);
	_dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    for (i = 0; i < lcount; i++) {
	laddrs[i] = linebuf[i]->li_address;
	loffsets[i] = linebuf[i]->li_addr_line.li_offset;
    }
    dwarf_srclines_dealloc(dbg, linebuf, lcount);
    *returncount = lcount;
    *offs = loffsets;
    *addrs = laddrs;
    return DW_DLV_OK;
}

/*
   It's impossible for callers of dwarf_srclines() to get to and
   free all the resources (in particular, the li_context and its
   lc_file_entries). 
   So this function, new July 2005, does it.  
*/
