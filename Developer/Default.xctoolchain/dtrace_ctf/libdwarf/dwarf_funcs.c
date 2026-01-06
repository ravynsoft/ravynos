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
#include "dwarf_funcs.h"
#include "dwarf_global.h"

int
dwarf_get_funcs(Dwarf_Debug dbg,
		Dwarf_Func ** funcs,
		Dwarf_Signed * ret_func_count, Dwarf_Error * error)
{
    int res;

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_funcnames_index,
			    &dbg->de_debug_funcnames, error);
    if (res != DW_DLV_OK) {
	return res;
    }

    return _dwarf_internal_get_pubnames_like_data(dbg, dbg->de_debug_funcnames, dbg->de_debug_funcnames_size, (Dwarf_Global **) funcs,	/* type 
																	   punning, 
																	   Dwarf_Type 
																	   is 
																	   never 
																	   a 
																	   completed 
																	   type 
																	 */
						  ret_func_count,
						  error,
						  DW_DLA_FUNC_CONTEXT,
						  DW_DLA_FUNC,
						  DW_DLE_DEBUG_FUNCNAMES_LENGTH_BAD,
						  DW_DLE_DEBUG_FUNCNAMES_VERSION_ERROR);

}

/* Deallocating fully requires deallocating the list
   and all entries.  But some internal data is
   not exposed, so we need a function with internal knowledge.
*/

void
dwarf_funcs_dealloc(Dwarf_Debug dbg, Dwarf_Func * dwgl,
		    Dwarf_Signed count)
{
    _dwarf_internal_globals_dealloc(dbg, (Dwarf_Global *) dwgl,
				    count,
				    DW_DLA_FUNC_CONTEXT,
				    DW_DLA_FUNC, DW_DLA_LIST);
    return;
}



int
dwarf_funcname(Dwarf_Func func_in, char **ret_name, Dwarf_Error * error)
{
    Dwarf_Global func = (Dwarf_Global) func_in;

    if (func == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FUNC_NULL);
	return (DW_DLV_ERROR);
    }

    *ret_name = (char *) (func->gl_name);
    return DW_DLV_OK;
}

int
dwarf_func_die_offset(Dwarf_Func func_in,
		      Dwarf_Off * return_offset, Dwarf_Error * error)
{
    Dwarf_Global func = (Dwarf_Global) func_in;

    return dwarf_global_die_offset(func, return_offset, error);
}


int
dwarf_func_cu_offset(Dwarf_Func func_in,
		     Dwarf_Off * return_offset, Dwarf_Error * error)
{
    Dwarf_Global func = (Dwarf_Global) func_in;

    return dwarf_global_cu_offset(func, return_offset, error);
}


int
dwarf_func_name_offsets(Dwarf_Func func_in,
			char **ret_func_name,
			Dwarf_Off * die_offset,
			Dwarf_Off * cu_die_offset, Dwarf_Error * error)
{
    Dwarf_Global func = (Dwarf_Global) func_in;

    return dwarf_global_name_offsets(func,
				     ret_func_name,
				     die_offset, cu_die_offset, error);
}
