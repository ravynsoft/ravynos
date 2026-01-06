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
#include "dwarf_types.h"
#include "dwarf_global.h"

int
dwarf_get_types(Dwarf_Debug dbg,
		Dwarf_Type ** types,
		Dwarf_Signed * ret_type_count, Dwarf_Error * error)
{
    int res;

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_typenames_index,
			    &dbg->de_debug_typenames, error);
    if (res != DW_DLV_OK) {
	return res;
    }

    return _dwarf_internal_get_pubnames_like_data(dbg, dbg->de_debug_typenames, dbg->de_debug_typenames_size, (Dwarf_Global **) types,	/* type 
																	   punning, 
																	   Dwarf_Type 
																	   is 
																	   never 
																	   a 
																	   completed 
																	   type 
																	 */
						  ret_type_count,
						  error,
						  DW_DLA_TYPENAME_CONTEXT,
						  DW_DLA_TYPENAME,
						  DW_DLE_DEBUG_TYPENAMES_LENGTH_BAD,
						  DW_DLE_DEBUG_TYPENAMES_VERSION_ERROR);

}

/* Deallocating fully requires deallocating the list
   and all entries.  But some internal data is
   not exposed, so we need a function with internal knowledge.
*/

void
dwarf_types_dealloc(Dwarf_Debug dbg, Dwarf_Type * dwgl,
		    Dwarf_Signed count)
{
    _dwarf_internal_globals_dealloc(dbg, (Dwarf_Global *) dwgl,
				    count,
				    DW_DLA_TYPENAME_CONTEXT,
				    DW_DLA_TYPENAME, DW_DLA_LIST);
    return;
}


int
dwarf_typename(Dwarf_Type type_in, char **ret_name, Dwarf_Error * error)
{
    Dwarf_Global type = (Dwarf_Global) type_in;

    if (type == NULL) {
	_dwarf_error(NULL, error, DW_DLE_TYPE_NULL);
	return (DW_DLV_ERROR);
    }

    *ret_name = (char *) (type->gl_name);
    return DW_DLV_OK;
}


int
dwarf_type_die_offset(Dwarf_Type type_in,
		      Dwarf_Off * ret_offset, Dwarf_Error * error)
{
    Dwarf_Global type = (Dwarf_Global) type_in;

    return dwarf_global_die_offset(type, ret_offset, error);
}


int
dwarf_type_cu_offset(Dwarf_Type type_in,
		     Dwarf_Off * ret_offset, Dwarf_Error * error)
{
    Dwarf_Global type = (Dwarf_Global) type_in;

    return dwarf_global_cu_offset(type, ret_offset, error);

}


int
dwarf_type_name_offsets(Dwarf_Type type_in,
			char **returned_name,
			Dwarf_Off * die_offset,
			Dwarf_Off * cu_die_offset, Dwarf_Error * error)
{
    Dwarf_Global type = (Dwarf_Global) type_in;

    return dwarf_global_name_offsets(type,
				     returned_name,
				     die_offset, cu_die_offset, error);
}
