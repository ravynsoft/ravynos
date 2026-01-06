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
/* This code used by SGI-IRIX rqs processing, not needed by
   any other system or application.
*/

#include "config.h"
#include "libdwarfdefs.h"
#ifdef HAVE_ELF_H
#include <sys/elf.h>
#endif
#include "dwarf.h"
#include "libdwarf.h"
#include "dwarf_base_types.h"
#include "dwarf_alloc.h"
#include "dwarf_opaque.h"
#include "dwarf_arange.h"
#include "dwarf_line.h"
#include "dwarf_frame.h"
#include "dwarf_addr_finder.h"
#include "dwarf_error.h"

typedef unsigned long long ull;

static int do_this_die_and_dealloc(Dwarf_Debug dbg, Dwarf_Die die,
				   int *errval);
static int
  handle_debug_info(Dwarf_Debug dbg, int *errval);
static int
  handle_debug_frame(Dwarf_Debug dbg, Dwarf_addr_callback_func cb_func, int *errval);
static int
  handle_debug_aranges(Dwarf_Debug dbg, Dwarf_addr_callback_func cb_func, int *errval);
static int
  handle_debug_line(Dwarf_Debug dbg, Dwarf_Die cu_die, Dwarf_addr_callback_func cb_func, int *errval);
static int
  handle_debug_loc(void);


static Dwarf_addr_callback_func send_addr_note;

int
_dwarf_addr_finder(dwarf_elf_handle elf_file_ptr,
		   Dwarf_addr_callback_func cb_func, int *dwerr)
{

    Dwarf_Error err = 0;
    Dwarf_Debug dbg = 0;
    int res = 0;
    int errval = 0;
    int sections_found = 0;

    res = dwarf_elf_init(elf_file_ptr, DW_DLC_READ, /* errhand */ 0,
			 /* errarg */ 0, &dbg, &err);
    if (res == DW_DLV_ERROR) {
	int errv = (int) dwarf_errno(err);

	return errv;
    }
    if (res == DW_DLV_NO_ENTRY) {
	return res;
    }

    send_addr_note = cb_func;

    res = handle_debug_info(dbg, &errval);
    switch (res) {
    case DW_DLV_OK:
	++sections_found;
	break;
    case DW_DLV_NO_ENTRY:

	break;
    default:
    case DW_DLV_ERROR:
	dwarf_finish(dbg, &err);
	*dwerr = errval;
	return res;
    }

    res = handle_debug_aranges(dbg, cb_func, &errval);
    switch (res) {
    case DW_DLV_OK:
	++sections_found;
	break;
    case DW_DLV_NO_ENTRY:
	break;
    default:
    case DW_DLV_ERROR:
	dwarf_finish(dbg, &err);
	*dwerr = errval;
	return res;
    }
    res = handle_debug_frame(dbg, cb_func, &errval);
    switch (res) {
    case DW_DLV_OK:
	++sections_found;
	break;
    case DW_DLV_NO_ENTRY:
	break;
    default:
    case DW_DLV_ERROR:
	dwarf_finish(dbg, &err);
	*dwerr = errval;
	return res;
    }

    res = handle_debug_loc();	/* does nothing */
    switch (res) {
    case DW_DLV_OK:
	++sections_found;
	break;
    case DW_DLV_NO_ENTRY:
	break;
    default:
    case DW_DLV_ERROR:
	/* IMPOSSIBLE : handle_debug_loc cannot return this */
	dwarf_finish(dbg, &err);
	*dwerr = errval;
	return res;
    }



    *dwerr = 0;
    res = dwarf_finish(dbg, &err);
    if (res == DW_DLV_ERROR) {
	*dwerr = (int) dwarf_errno(err);
	return DW_DLV_ERROR;
    }
    if (sections_found == 0) {
	return DW_DLV_NO_ENTRY;
    }
    return DW_DLV_OK;

}

/*
	Return DW_DLV_OK, ERROR, or NO_ENTRY.
*/
static int
handle_debug_info(Dwarf_Debug dbg, int *errval)
{
    Dwarf_Unsigned nxtoff = 1;
    Dwarf_Unsigned hdr_length;
    Dwarf_Half version_stamp;
    Dwarf_Unsigned abbrev_offset;
    Dwarf_Half addr_size;
    Dwarf_Error err;
    int terminate_now = 0;
    int res = 0;
    Dwarf_Die sibdie;
    int sibres;
    int nres = DW_DLV_OK;


    for (nres = dwarf_next_cu_header(dbg, &hdr_length, &version_stamp,
				     &abbrev_offset,
				     &addr_size, &nxtoff, &err);
	 terminate_now == 0 && nres == DW_DLV_OK;
	 nres = dwarf_next_cu_header(dbg, &hdr_length, &version_stamp,
				     &abbrev_offset,
				     &addr_size, &nxtoff, &err)
	) {

	Dwarf_Die curdie = 0;

	/* try to get the compilation unit die */
	sibres = dwarf_siblingof(dbg, curdie, &sibdie, &err);
	if (sibres == DW_DLV_OK) {
	    res = do_this_die_and_dealloc(dbg, sibdie, errval);
	    switch (res) {
	    case DW_DLV_OK:
		break;
	    case DW_DLV_NO_ENTRY:
		break;
	    default:
	    case DW_DLV_ERROR:
		return DW_DLV_ERROR;
	    }
	} else if (sibres == DW_DLV_ERROR) {
	    *errval = (int) dwarf_errno(err);
	    return DW_DLV_ERROR;
	} else {
	    /* NO ENTRY! */
	    /* impossible? */
	}

    }
    if (nres == DW_DLV_ERROR) {
	int localerr = (int) dwarf_errno(err);

	*errval = localerr;
	return DW_DLV_ERROR;
    }
    return DW_DLV_OK;
}

static int
  might_have_addr[] = {
    DW_AT_high_pc,
    DW_AT_low_pc,
};
static int
  might_have_locdesc[] = {
    DW_AT_segment,
    DW_AT_return_addr,
    DW_AT_frame_base,
    DW_AT_static_link,
    DW_AT_data_member_location,
    DW_AT_string_length,
    DW_AT_location,
    DW_AT_use_location,
    DW_AT_vtable_elem_location,
};

/*
	Return DW_DLV_OK if handling this went ok.
*/
static int
handle_attr_addr(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum,
		 Dwarf_Error * perr)
{
    int res = DW_DLV_OK;
    Dwarf_Off offset;
    Dwarf_Addr addr;
    Dwarf_Half form;
    int ares;

    Dwarf_Attribute attr;

    ares = dwarf_attr(die, attrnum, &attr, perr);
    if (ares == DW_DLV_OK) {
	int formres = dwarf_whatform(attr, &form, perr);

	switch (formres) {
	case DW_DLV_OK:
	    break;
	case DW_DLV_ERROR:
	case DW_DLV_NO_ENTRY:	/* impossible. */
	    return formres;

	}

	switch (form) {
	case DW_FORM_ref_addr:
	case DW_FORM_addr:
	    res = dwarf_attr_offset(die, attr, &offset, perr);
	    if (res == DW_DLV_OK) {
		ares = dwarf_formaddr(attr, &addr, perr);
		if (ares == DW_DLV_OK) {
		    send_addr_note(DW_SECTION_INFO, offset, addr);
		} else if (ares == DW_DLV_ERROR) {
		    return ares;
		}		/* no entry: ok. */
	    } else {
		res = DW_DLV_ERROR;	/* NO_ENTRY is impossible. */
	    }
	    break;

	default:
	    /* surprising! An error? */

	    ;			/* do nothing */
	}
	dwarf_dealloc(dbg, attr, DW_DLA_ATTR);

    } else {
	res = ares;
    }
    return res;
}

/*
	Return DW_DLV_OK if handling this went ok.
*/
static int
handle_attr_locdesc(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum,
		    Dwarf_Error * perr)
{
    int retval = DW_DLV_OK;
    Dwarf_Attribute attr;
    Dwarf_Locdesc *llbuf;
    Dwarf_Signed i;
    Dwarf_Off offset;
    Dwarf_Loc *locp;
    unsigned int entindx;
    int res;
    int ares;


    ares = dwarf_attr(die, attrnum, &attr, perr);
    if (ares == DW_DLV_OK) {
	Dwarf_Half form;
	int fres = dwarf_whatform(attr, &form, perr);

	if (fres == DW_DLV_OK) {
	    switch (form) {
	    case DW_FORM_block1:
	    case DW_FORM_block2:
	    case DW_FORM_block4:
		/* must be location description */
		res = dwarf_attr_offset(die, attr, &offset, perr);
		llbuf = 0;
		if (res == DW_DLV_OK) {
		    Dwarf_Signed count;
		    int lres =
			dwarf_loclist(attr, &llbuf, &count, perr);
		    if (lres != DW_DLV_OK) {
			return lres;
		    }
		    if (count != 1) {
			/* this cannot happen! */
			/* perr? */
			_dwarf_error(dbg, perr,
				     DW_DLE_LOCDESC_COUNT_WRONG);
			retval = DW_DLV_ERROR;
			return retval;
		    }
		    for (i = 0; i < count; ++i) {
			unsigned int ents = llbuf[i].ld_cents;

			locp = llbuf[i].ld_s;
			for (entindx = 0; entindx < ents; entindx++) {
			    Dwarf_Loc *llocp;

			    llocp = locp + entindx;
			    if (llocp->lr_atom == DW_OP_addr) {
				send_addr_note(DW_SECTION_INFO, offset +
					       llocp->lr_offset + 1
					       /* The offset is the
					          offset of the atom,
					          ** and we know the
					          addr is 1 past it. */
					       , llocp->lr_number);
			    }
			}
		    }


		    if (count > 0) {
			for (i = 0; i < count; ++i) {
			    dwarf_dealloc(dbg, llbuf[i].ld_s,
					  DW_DLA_LOC_BLOCK);
			}
			dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
		    }
		} else {
		    retval = res;
		}
		break;

	    default:
		/* must be a const offset in debug_loc */
		;		/* do nothing */
	    }
	    dwarf_dealloc(dbg, attr, DW_DLA_ATTR);
	}			/* else error or no entry */
	retval = fres;
    } else {
	retval = ares;
    }
    return retval;
}

/*
  Return DW_DLV_OK, or DW_DLV_ERROR

  Handle the addrs in a single die.
*/
static int
process_this_die_attrs(Dwarf_Debug dbg, Dwarf_Die newdie, int *errval)
{
    Dwarf_Error err;
    Dwarf_Half i;
    Dwarf_Half newattrnum;
    int res;
    int tres;
    Dwarf_Half ltag;

    Dwarf_Off doff;
    int doffres = dwarf_dieoffset(newdie, &doff, &err);

    if (doffres != DW_DLV_OK) {
	if (doffres == DW_DLV_ERROR) {
	    *errval = (int) dwarf_errno(err);
	}
	return doffres;
    }
    tres = dwarf_tag(newdie, &ltag, &err);
    if (tres != DW_DLV_OK) {
	return tres;
    }
    if (DW_TAG_compile_unit == ltag) {
	/* because of the way the dwarf_line code works, we ** do lines 
	   only per compile unit. ** This may turn out to be wrong if
	   we have lines ** left unconnected to a CU. ** of course such 
	   lines will not, at present, be ** used by gnome ** This is
	   not ideal as coded due to the dwarf_line.c issue. */
	int lres;

	lres = handle_debug_line(dbg, newdie, send_addr_note, errval);
	if (lres == DW_DLV_ERROR) {
	    return lres;
	}
    }

    for (i = 0; i < sizeof(might_have_addr) / sizeof(int); i++) {
	int resattr;
	Dwarf_Bool hasattr;

	newattrnum = might_have_addr[i];
	err = 0;
	resattr = dwarf_hasattr(newdie, newattrnum, &hasattr, &err);
	if (DW_DLV_OK == resattr) {
	    if (hasattr) {
		res = handle_attr_addr(dbg, newdie, newattrnum, &err);
		if (res != DW_DLV_OK) {
		    *errval = (int) dwarf_errno(err);
		    return DW_DLV_ERROR;
		}
	    }
	} else {
	    if (resattr == DW_DLV_ERROR) {
		*errval = (int) dwarf_errno(err);
		return resattr;
	    }
	}
    }
    for (i = 0; i < sizeof(might_have_locdesc) / sizeof(int); i++) {
	int resattr;
	Dwarf_Bool hasattr;

	newattrnum = might_have_locdesc[i];
	err = 0;
	resattr = dwarf_hasattr(newdie, newattrnum, &hasattr, &err);
	if (DW_DLV_OK == resattr) {
	    if (hasattr) {
		res =
		    handle_attr_locdesc(dbg, newdie, newattrnum, &err);
		if (res != DW_DLV_OK) {
		    *errval = (int) dwarf_errno(err);
		    return DW_DLV_ERROR;
		}
	    }
	} else {
	    if (resattr == DW_DLV_ERROR) {
		*errval = (int) dwarf_errno(err);
		return resattr;
	    }
	}
    }

    return DW_DLV_OK;
}

/*
	Handle siblings as a list,
	Do children by recursing.
	Effectively this is walking the tree preorder.

	This dealloc's any die passed to it, so the
	caller should not do that dealloc.
	It seems more logical to have the one causing
	the alloc to do the dealloc, but that way this
	routine became a mess.

*/
static int
do_this_die_and_dealloc(Dwarf_Debug dbg, Dwarf_Die die, int *errval)
{

    Dwarf_Die prevdie = 0;
    Dwarf_Die newdie = die;
    Dwarf_Error err = 0;
    int res = 0;
    int sibres = DW_DLV_OK;
    int tres = DW_DLV_OK;
    Dwarf_Die sibdie;

    while (sibres == DW_DLV_OK) {
	Dwarf_Die ch_die;


	res = process_this_die_attrs(dbg, newdie, errval);
	switch (res) {
	case DW_DLV_OK:
	    break;
	case DW_DLV_NO_ENTRY:
	    break;
	default:
	case DW_DLV_ERROR:
	    if (prevdie) {
		dwarf_dealloc(dbg, prevdie, DW_DLA_DIE);
		prevdie = 0;
	    }
	    return DW_DLV_ERROR;
	}

	tres = dwarf_child(newdie, &ch_die, &err);

	if (tres == DW_DLV_OK) {
	    res = do_this_die_and_dealloc(dbg, ch_die, errval);
	    switch (res) {
	    case DW_DLV_OK:
		break;
	    case DW_DLV_NO_ENTRY:
		break;
	    default:
	    case DW_DLV_ERROR:
		if (prevdie) {
		    dwarf_dealloc(dbg, prevdie, DW_DLA_DIE);
		    prevdie = 0;
		}
		return DW_DLV_ERROR;
	    }
	} else if (tres == DW_DLV_ERROR) {
	    /* An error! */
	    *errval = (int) dwarf_errno(err);
	    if (prevdie) {
		dwarf_dealloc(dbg, prevdie, DW_DLA_DIE);
		prevdie = 0;
	    }
	    dwarf_dealloc(dbg, err, DW_DLA_ERROR);
	    return DW_DLV_ERROR;
	}			/* else was NO ENTRY */
	prevdie = newdie;
	sibdie = 0;
	sibres = dwarf_siblingof(dbg, newdie, &sibdie, &err);
	if (prevdie) {
	    dwarf_dealloc(dbg, prevdie, DW_DLA_DIE);
	    prevdie = 0;
	}
	newdie = sibdie;

    }
    if (sibres == DW_DLV_NO_ENTRY) {
	return DW_DLV_OK;
    }
    /* error. */
    *errval = (int) dwarf_errno(err);
    if (prevdie) {
	dwarf_dealloc(dbg, prevdie, DW_DLA_DIE);
	prevdie = 0;
    }
    dwarf_dealloc(dbg, err, DW_DLA_ERROR);
    return DW_DLV_ERROR;

}


static int
handle_debug_frame(Dwarf_Debug dbg, Dwarf_addr_callback_func cb_func,
		   int *errval)
{
    int retval = DW_DLV_OK;
    int res;
    Dwarf_Error err;
    Dwarf_Addr *addrlist;
    Dwarf_Off *offsetlist;
    Dwarf_Signed count;
    int i;

    res =
	_dwarf_frame_address_offsets(dbg, &addrlist, &offsetlist,
				     &count, &err);
    if (res == DW_DLV_OK) {
	for (i = 0; i < count; i++) {
	    cb_func(DW_SECTION_FRAME, offsetlist[i], addrlist[i]);
	}
	dwarf_dealloc(dbg, offsetlist, DW_DLA_ADDR);
	dwarf_dealloc(dbg, addrlist, DW_DLA_ADDR);
    } else if (res == DW_DLV_NO_ENTRY) {
	retval = res;
    } else {
	*errval = (int) dwarf_errno(err);
	retval = DW_DLV_ERROR;
    }
    return retval;

}
static int
handle_debug_aranges(Dwarf_Debug dbg, Dwarf_addr_callback_func cb_func,
		     int *errval)
{
    int retval = DW_DLV_OK;
    Dwarf_Error err;
    Dwarf_Addr *aranges;
    Dwarf_Signed count;
    int indx;
    Dwarf_Off *offsets;

    retval =
	_dwarf_get_aranges_addr_offsets(dbg, &aranges, &offsets, &count,
					&err);
    if (retval == DW_DLV_OK) {
	if (count == 0) {
	    retval = DW_DLV_NO_ENTRY;
	} else {
	    for (indx = 0; indx < count; indx++) {
		cb_func(DW_SECTION_ARANGES, offsets[indx],
			aranges[indx]);
	    }
	}
	dwarf_dealloc(dbg, aranges, DW_DLA_ADDR);
	dwarf_dealloc(dbg, offsets, DW_DLA_ADDR);
    } else if (retval == DW_DLV_NO_ENTRY) {
	;			/* do nothing */
    } else {
	*errval = (int) dwarf_errno(err);
	retval = DW_DLV_ERROR;
    }
    return retval;
}
static int
handle_debug_line(Dwarf_Debug dbg, Dwarf_Die cu_die,
		  Dwarf_addr_callback_func cb_func, int *errval)
{
    int retval = DW_DLV_OK;
    int res;
    Dwarf_Error err;
    Dwarf_Addr *addrlist;
    Dwarf_Off *offsetlist;
    Dwarf_Unsigned count;
    Dwarf_Unsigned i;

    res =
	_dwarf_line_address_offsets(dbg, cu_die, &addrlist, &offsetlist,
				    &count, &err);
    if (res == DW_DLV_OK) {
	for (i = 0; i < count; i++) {
	    cb_func(DW_SECTION_LINE, offsetlist[i], addrlist[i]);

	}
	dwarf_dealloc(dbg, offsetlist, DW_DLA_ADDR);
	dwarf_dealloc(dbg, addrlist, DW_DLA_ADDR);
    } else if (res == DW_DLV_NO_ENTRY) {
	retval = res;
    } else {
	*errval = (int) dwarf_errno(err);
	retval = DW_DLV_ERROR;
    }
    return retval;
}

/*
	We need to add support for this. Currently we do not
	generate this section.
	FIX!
*/
static int
handle_debug_loc(void)
{
    int retval = DW_DLV_NO_ENTRY;

    return retval;
}
