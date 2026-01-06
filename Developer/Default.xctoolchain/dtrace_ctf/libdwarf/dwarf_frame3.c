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



#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <stdlib.h>
#include "dwarf_frame.h"
#include "dwarf_arange.h"	/* using Arange as a way to build a
				   list */

/*
	Used by rqs (an IRIX application).  
        Not needed except for that one application.
        Should be moved to its own source file since
        it is so rarely needed.
	Returns DW_DLV_OK if returns the arrays.
	Returns DW_DLV_NO_ENTRY if no section. ?? (How do I tell?)
	Returns DW_DLV_ERROR if there is an error.

        Uses DW_FRAME_CFA_COL because IRIX is only DWARF2
        and that is what IRIX compilers and compatible
	compilers support on IRIX.
*/
int
_dwarf_frame_address_offsets(Dwarf_Debug dbg, Dwarf_Addr ** addrlist,
			     Dwarf_Off ** offsetlist,
			     Dwarf_Signed * returncount,
			     Dwarf_Error * err)
{
    int retval = DW_DLV_OK;
    int res;
    Dwarf_Cie *cie_data;
    Dwarf_Signed cie_count;
    Dwarf_Fde *fde_data;
    Dwarf_Signed fde_count;
    Dwarf_Signed i;
    Dwarf_Frame_Op *frame_inst;
    Dwarf_Fde fdep;
    Dwarf_Cie ciep;
    Dwarf_Chain curr_chain = 0;
    Dwarf_Chain head_chain = 0;
    Dwarf_Chain prev_chain = 0;
    Dwarf_Arange arange;
    Dwarf_Unsigned arange_count = 0;
    Dwarf_Addr *arange_addrs = 0;
    Dwarf_Off *arange_offsets = 0;

    res = dwarf_get_fde_list(dbg, &cie_data, &cie_count,
			     &fde_data, &fde_count, err);
    if (res != DW_DLV_OK) {
	return res;
    }

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_frame_index,
			    &dbg->de_debug_frame, err);
    if (res != DW_DLV_OK) {
	return res;
    }

    for (i = 0; i < cie_count; i++) {
	Dwarf_Off instoff = 0;
	Dwarf_Signed initial_instructions_length = 0;
	Dwarf_Small *instr_end = 0;
	Dwarf_Sword icount = 0;
	int j;
	int dw_err;

	ciep = cie_data[i];
	instoff = ciep->ci_cie_instr_start - dbg->de_debug_frame;
	initial_instructions_length = ciep->ci_length +
	    ciep->ci_length_size + ciep->ci_extension_size -
	    (ciep->ci_cie_instr_start - ciep->ci_cie_start);
	instr_end = ciep->ci_cie_instr_start +
	    initial_instructions_length;
	res = _dwarf_exec_frame_instr( /* make_instr */ true,
				      &frame_inst,
				      /* search_pc= */ false,
				      /* search_pc_val= */ 0,
				      /* location */ 0,
				      ciep->ci_cie_instr_start,
				      instr_end,
				      /* Dwarf_frame= */ 0,
				      /* cie= */ 0,
				      dbg,
				      DW_FRAME_CFA_COL,
				      &icount, &dw_err);
	if (res == DW_DLV_ERROR) {
	    _dwarf_error(dbg, err, dw_err);
	    return (res);
	} else if (res == DW_DLV_NO_ENTRY) {
	    continue;
	}

	for (j = 0; j < icount; ++j) {
	    Dwarf_Frame_Op *finst = frame_inst + j;

	    if (finst->fp_base_op == 0 && finst->fp_extended_op == 1) {
		/* is DW_CFA_set_loc */
		Dwarf_Addr add = (Dwarf_Addr) finst->fp_offset;
		Dwarf_Off off = finst->fp_instr_offset + instoff;

		arange = (Dwarf_Arange)
		    _dwarf_get_alloc(dbg, DW_DLA_ARANGE, 1);
		if (arange == NULL) {
		    _dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
		    return (DW_DLV_ERROR);
		}
		arange->ar_address = add;
		arange->ar_info_offset = off;
		arange_count++;
		curr_chain = (Dwarf_Chain)
		    _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
		if (curr_chain == NULL) {
		    _dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
		    return (DW_DLV_ERROR);
		}
		curr_chain->ch_item = arange;
		if (head_chain == NULL)
		    head_chain = prev_chain = curr_chain;
		else {
		    prev_chain->ch_next = curr_chain;
		    prev_chain = curr_chain;
		}
	    }
	}
	dwarf_dealloc(dbg, frame_inst, DW_DLA_FRAME_BLOCK);

    }
    for (i = 0; i < fde_count; i++) {
	Dwarf_Small *instr_end = 0;
	Dwarf_Sword icount = 0;
	Dwarf_Signed instructions_length = 0;
	Dwarf_Off instoff = 0;
	Dwarf_Off off = 0;
	Dwarf_Addr addr = 0;
	int j;
	int dw_err;

	fdep = fde_data[i];
	off = fdep->fd_initial_loc_pos - dbg->de_debug_frame;
	addr = fdep->fd_initial_location;
	arange = (Dwarf_Arange)
	    _dwarf_get_alloc(dbg, DW_DLA_ARANGE, 1);
	if (arange == NULL) {
	    _dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
	    return (DW_DLV_ERROR);
	}
	arange->ar_address = addr;
	arange->ar_info_offset = off;
	arange_count++;
	curr_chain = (Dwarf_Chain)
	    _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
	if (curr_chain == NULL) {
	    _dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
	    return (DW_DLV_ERROR);
	}
	curr_chain->ch_item = arange;
	if (head_chain == NULL)
	    head_chain = prev_chain = curr_chain;
	else {
	    prev_chain->ch_next = curr_chain;
	    prev_chain = curr_chain;
	}


	instoff = fdep->fd_fde_instr_start - dbg->de_debug_frame;
	instructions_length = fdep->fd_length +
	    fdep->fd_length_size + fdep->fd_extension_size -
	    (fdep->fd_fde_instr_start - fdep->fd_fde_start);
	instr_end = fdep->fd_fde_instr_start + instructions_length;
	res = _dwarf_exec_frame_instr( /* make_instr */ true,
				      &frame_inst,
				      /* search_pc= */ false,
				      /* search_pc_val= */ 0,
				      /* location */ 0,
				      fdep->fd_fde_instr_start,
				      instr_end,
				      /* Dwarf_frame= */ 0,
				      /* cie= */ 0,
				      dbg,
				      DW_FRAME_CFA_COL,
				      &icount, &dw_err);
	if (res == DW_DLV_ERROR) {
	    _dwarf_error(dbg, err, dw_err);
	    return (res);
	} else if (res == DW_DLV_NO_ENTRY) {
	    continue;
	}

	for (j = 0; j < icount; ++j) {
	    Dwarf_Frame_Op *finst2 = frame_inst + j;

	    if (finst2->fp_base_op == 0 && finst2->fp_extended_op == 1) {
		/* is DW_CFA_set_loc */
		Dwarf_Addr add = (Dwarf_Addr) finst2->fp_offset;
		Dwarf_Off foff = finst2->fp_instr_offset + instoff;

		arange = (Dwarf_Arange)
		    _dwarf_get_alloc(dbg, DW_DLA_ARANGE, 1);
		if (arange == NULL) {
		    _dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
		    return (DW_DLV_ERROR);
		}
		arange->ar_address = add;
		arange->ar_info_offset = foff;
		arange_count++;
		curr_chain = (Dwarf_Chain)
		    _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
		if (curr_chain == NULL) {
		    _dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
		    return (DW_DLV_ERROR);
		}
		curr_chain->ch_item = arange;
		if (head_chain == NULL)
		    head_chain = prev_chain = curr_chain;
		else {
		    prev_chain->ch_next = curr_chain;
		    prev_chain = curr_chain;
		}

	    }
	}
	dwarf_dealloc(dbg, frame_inst, DW_DLA_FRAME_BLOCK);

    }
    dwarf_dealloc(dbg, fde_data, DW_DLA_LIST);
    dwarf_dealloc(dbg, cie_data, DW_DLA_LIST);
    arange_addrs = (Dwarf_Addr *)
	_dwarf_get_alloc(dbg, DW_DLA_ADDR, arange_count);
    if (arange_addrs == NULL) {
	_dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }
    arange_offsets = (Dwarf_Off *)
	_dwarf_get_alloc(dbg, DW_DLA_ADDR, arange_count);
    if (arange_offsets == NULL) {
	_dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    curr_chain = head_chain;
    for (i = 0; i < arange_count; i++) {
	Dwarf_Arange ar = curr_chain->ch_item;

	arange_addrs[i] = ar->ar_address;
	arange_offsets[i] = ar->ar_info_offset;
	prev_chain = curr_chain;
	curr_chain = curr_chain->ch_next;
	dwarf_dealloc(dbg, ar, DW_DLA_ARANGE);
	dwarf_dealloc(dbg, prev_chain, DW_DLA_CHAIN);
    }
    *returncount = arange_count;
    *offsetlist = arange_offsets;
    *addrlist = arange_addrs;
    return retval;
}
