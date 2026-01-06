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

int
dwarf_get_str(Dwarf_Debug dbg,
	      Dwarf_Off offset,
	      char **string,
	      Dwarf_Signed * returned_str_len, Dwarf_Error * error)
{
    int res;

    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    if (offset == dbg->de_debug_str_size) {
	/* Normal (if we've iterated thru the set of strings using
	   dwarf_get_str and are at the end). */
	return DW_DLV_NO_ENTRY;
    }
    if (offset > dbg->de_debug_str_size) {
	_dwarf_error(dbg, error, DW_DLE_DEBUG_STR_OFFSET_BAD);
	return (DW_DLV_ERROR);
    }

    if (string == NULL) {
	_dwarf_error(dbg, error, DW_DLE_STRING_PTR_NULL);
	return (DW_DLV_ERROR);
    }

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_str_index,
			    &dbg->de_debug_str, error);
    if (res != DW_DLV_OK) {
	return res;
    }

    *string = (char *) dbg->de_debug_str + offset;

    *returned_str_len = (strlen(*string));
    return DW_DLV_OK;
}
