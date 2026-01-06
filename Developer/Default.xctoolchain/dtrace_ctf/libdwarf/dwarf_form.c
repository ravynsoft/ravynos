/*

  Copyright (C) 2000,2002,2004,2005  Silicon Graphics, Inc.  All Rights Reserved.

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
#include "dwarf_die_deliv.h"

int
dwarf_hasform(Dwarf_Attribute attr,
	      Dwarf_Half form,
	      Dwarf_Bool * return_bool, Dwarf_Error * error)
{
    Dwarf_CU_Context cu_context;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }

    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    if (cu_context->cc_dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    *return_bool = (attr->ar_attribute_form == form);
    return DW_DLV_OK;
}

/* Not often called, we do not worry about efficiency here.
   The dwarf_whatform() call does the sanity checks for us.
*/
int
dwarf_whatform_direct(Dwarf_Attribute attr,
		      Dwarf_Half * return_form, Dwarf_Error * error)
{
    int res = dwarf_whatform(attr, return_form, error);

    if (res != DW_DLV_OK) {
	return res;
    }

    *return_form = attr->ar_attribute_form_direct;
    return (DW_DLV_OK);
}

int
dwarf_whatform(Dwarf_Attribute attr,
	       Dwarf_Half * return_form, Dwarf_Error * error)
{
    Dwarf_CU_Context cu_context;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }

    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    if (cu_context->cc_dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    *return_form = attr->ar_attribute_form;
    return (DW_DLV_OK);
}


/*
    This function is analogous to dwarf_whatform.
    It returns the attribute in attr instead of
    the form.
*/
int
dwarf_whatattr(Dwarf_Attribute attr,
	       Dwarf_Half * return_attr, Dwarf_Error * error)
{
    Dwarf_CU_Context cu_context;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }

    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    if (cu_context->cc_dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    *return_attr = (attr->ar_attribute);
    return DW_DLV_OK;
}


/* 
    DW_FORM_ref_addr is considered an incorrect form 
    for this call because this function returns an 
    offset  within the local CU thru the pointer.

    DW_FORM_ref_addr is a global-offset into the debug_info section.
    A DW_FORM_ref_addr cannot be returned by this interface:
    see dwarf_global_formref();

    DW_FORM_ref_addr has a value which was documented in
    DWARF2 as address-size but which was always an offset
    so should have always been offset size (wording
    corrected in DWARF3). 
    
*/
int
dwarf_formref(Dwarf_Attribute attr,
	      Dwarf_Off * ret_offset, Dwarf_Error * error)
{
    Dwarf_Debug dbg;
    Dwarf_Unsigned offset;
    Dwarf_CU_Context cu_context;


    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }

    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    if (cu_context->cc_dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    dbg = cu_context->cc_dbg;

    switch (attr->ar_attribute_form) {

    case DW_FORM_ref1:
	offset = *(Dwarf_Small *) attr->ar_debug_info_ptr;
	break;

    case DW_FORM_ref2:
	READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
		       attr->ar_debug_info_ptr, sizeof(Dwarf_Half));
	break;

    case DW_FORM_ref4:
	READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
		       attr->ar_debug_info_ptr, sizeof(Dwarf_ufixed));
	break;

    case DW_FORM_ref8:
	READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
		       attr->ar_debug_info_ptr, sizeof(Dwarf_Unsigned));
	break;

    case DW_FORM_ref_udata:
	offset = _dwarf_decode_u_leb128(attr->ar_debug_info_ptr, NULL);
	break;

    default:
	_dwarf_error(dbg, error, DW_DLE_BAD_REF_FORM);
	return (DW_DLV_ERROR);
    }

    /* Check that offset is within current cu portion of .debug_info. */

    if (offset >= cu_context->cc_length +
	cu_context->cc_length_size + cu_context->cc_extension_size) {
	_dwarf_error(dbg, error, DW_DLE_ATTR_FORM_OFFSET_BAD);
	return (DW_DLV_ERROR);
    }

    *ret_offset = (offset);
    return DW_DLV_OK;
}

/* 
    Since this returns section-relative debug_info offsets,
    this can represent all REFERENCE forms correctly
    and allows all forms.

    DW_FORM_ref_addr has a value which was documented in
    DWARF2 as address-size but which was always an offset
    so should have always been offset size (wording
    corrected in DWARF3).

    See the DWARF4 document for the 3 cases fitting
    reference forms.  The caller must determine which section the
    reference 'points' to.
*/
int
dwarf_global_formref(Dwarf_Attribute attr,
		     Dwarf_Off * ret_offset, Dwarf_Error * error)
{
  Dwarf_Debug dbg;
  Dwarf_Unsigned offset;
  Dwarf_Unsigned max_offset;
  Dwarf_CU_Context cu_context;
  Dwarf_Half context_version = 0;

  if (attr == NULL) {
    _dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
    return (DW_DLV_ERROR);
  }

  cu_context = attr->ar_cu_context;
  if (cu_context == NULL) {
    _dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
    return (DW_DLV_ERROR);
  }

  context_version = cu_context->cc_version_stamp;

  if (cu_context->cc_dbg == NULL) {
    _dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
    return (DW_DLV_ERROR);
  }

  dbg = cu_context->cc_dbg;

  switch (attr->ar_attribute_form) {
  case DW_FORM_ref1:
    offset = *(Dwarf_Small *) attr->ar_debug_info_ptr;
    goto fixoffset;

  case DW_FORM_ref2:
    READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
                   attr->ar_debug_info_ptr, sizeof(Dwarf_Half));
    goto fixoffset;

  case DW_FORM_ref4:
    READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
                   attr->ar_debug_info_ptr, sizeof(Dwarf_ufixed));
    goto fixoffset;

  case DW_FORM_ref8:
    READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
                   attr->ar_debug_info_ptr, sizeof(Dwarf_Unsigned));
    goto fixoffset;

  case DW_FORM_ref_udata:
    offset = _dwarf_decode_u_leb128(attr->ar_debug_info_ptr, NULL);

/* we have a local offset, make it global */
fixoffset:
	/* check legality of offset */
    max_offset = cu_context->cc_length
               + cu_context->cc_length_size
               + cu_context->cc_extension_size;
    if (offset >= max_offset) {
      _dwarf_error(dbg, error, DW_DLE_ATTR_FORM_OFFSET_BAD);
      return (DW_DLV_ERROR);
    }

    /* globalize the offset */
    offset += cu_context->cc_debug_info_offset;
    break;

    /*
     * The DWARF2 document did not make clear that
     * DW_FORM_data4( and 8) were references with
     * global offsets to some section.
     * That was first clearly documented in DWARF3.
     * In DWARF4 these two forms are no longer references.
     */
  case DW_FORM_data4:
    if(context_version == CURRENT_VERSION_STAMP4) {
      _dwarf_error(dbg, error, DW_DLE_NOT_REF_FORM);
      return (DW_DLV_ERROR);
    }
    READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
                   attr->ar_debug_info_ptr, sizeof(Dwarf_ufixed));
    /* The offset is global. */
    break;
  case DW_FORM_data8:
    if(context_version == CURRENT_VERSION_STAMP4) {
      _dwarf_error(dbg, error, DW_DLE_NOT_REF_FORM);
      return (DW_DLV_ERROR);
    }
    READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
                   attr->ar_debug_info_ptr, sizeof(Dwarf_Unsigned));
    /* The offset is global. */
    break;

  case DW_FORM_ref_addr:
  case DW_FORM_sec_offset:
  {
    /*
     * DW_FORM_sec_offset first exists in DWARF4.
     * It is up to the caller to know what the offset
     * of DW_FORM_sec_offset refers to,
     * the offset is not going to refer to .debug_info!
     */
    unsigned length_size = cu_context->cc_length_size;
    if(length_size == 4) {
      READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
                     attr->ar_debug_info_ptr, sizeof(Dwarf_ufixed));
    } else if (length_size == 8) {
    READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
                   attr->ar_debug_info_ptr, sizeof(Dwarf_Unsigned));
    } else {
      _dwarf_error(dbg, error, DW_DLE_FORM_SEC_OFFSET_LENGTH_BAD);
      return (DW_DLV_ERROR);
    }
  }
    break;

  default:
    _dwarf_error(dbg, error, DW_DLE_BAD_REF_FORM);
    return (DW_DLV_ERROR);
  }

  /* We do not know what section the offset refers to, so
   we have no way to check it for correctness. */
  *ret_offset = offset;
  return DW_DLV_OK;
}


int
dwarf_formaddr(Dwarf_Attribute attr,
	       Dwarf_Addr * return_addr, Dwarf_Error * error)
{
    Dwarf_Debug dbg;
    Dwarf_Addr ret_addr;
    Dwarf_CU_Context cu_context;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }

    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    if (cu_context->cc_dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    dbg = cu_context->cc_dbg;

    if (attr->ar_attribute_form == DW_FORM_addr
	/* || attr->ar_attribute_form == DW_FORM_ref_addr Allowance of
	   DW_FORM_ref_addr was a mistake. The value returned in that
	   case is NOT an address it is a global debug_info offset (ie, 
	   not CU-relative offset within the CU in debug_info). The
	   Dwarf document refers to it as an address (misleadingly) in
	   sec 6.5.4 where it describes the reference form. It is
	   address-sized so that the linker can easily update it, but
	   it is a reference inside the debug_info section. No longer
	   allowed. */
	) {

	READ_UNALIGNED(dbg, ret_addr, Dwarf_Addr,
		       attr->ar_debug_info_ptr, dbg->de_pointer_size);
	*return_addr = ret_addr;
	return (DW_DLV_OK);
    }

    _dwarf_error(dbg, error, DW_DLE_ATTR_FORM_BAD);
    return (DW_DLV_ERROR);
}


int
dwarf_formflag(Dwarf_Attribute attr,
               Dwarf_Bool* ret_bool,
               Dwarf_Error * error)
{
  Dwarf_CU_Context cu_context;

  if (attr == NULL) {
    _dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
    return (DW_DLV_ERROR);
  }

  cu_context = attr->ar_cu_context;
  if (cu_context == NULL) {
    _dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
    return (DW_DLV_ERROR);
  }

  if (cu_context->cc_dbg == NULL) {
    _dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
    return (DW_DLV_ERROR);
  }

  if (attr->ar_attribute_form == DW_FORM_flag) {
    *ret_bool = (*(Dwarf_Small *) attr->ar_debug_info_ptr != 0);
    return (DW_DLV_OK);
  }

  /*
   * DWARF4: the attribute is implicitly indicated as present, and no value is
   * encoded in the debugging information entry itself.
   */
  if (attr->ar_attribute_form == DW_FORM_flag_present) {
    *ret_bool = 1;
    return (DW_DLV_OK);
  }

  _dwarf_error(cu_context->cc_dbg, error, DW_DLE_ATTR_FORM_BAD);
  return (DW_DLV_ERROR);
}

Dwarf_Bool
dwarf_formisdata(Dwarf_Attribute attr)
{
	Dwarf_Half form = attr->ar_attribute_form;
	return form == DW_FORM_udata || form == DW_FORM_data1 ||
		form == DW_FORM_data2 || form == DW_FORM_data4 ||
		form == DW_FORM_data8 || form == DW_FORM_sdata;
}

Dwarf_Bool
dwarf_formisudata(Dwarf_Attribute attr)
{
	return attr->ar_attribute_form == DW_FORM_udata;
}

int
dwarf_formudata(Dwarf_Attribute attr,
		Dwarf_Unsigned * return_uval, Dwarf_Error * error)
{
    Dwarf_Unsigned ret_value;
    Dwarf_Debug dbg;
    Dwarf_CU_Context cu_context;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }


    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    dbg = cu_context->cc_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    switch (attr->ar_attribute_form) {

    case DW_FORM_data1:
	READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
		       attr->ar_debug_info_ptr, sizeof(Dwarf_Small));
	*return_uval = ret_value;
	return DW_DLV_OK;

    case DW_FORM_data2:{
	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   attr->ar_debug_info_ptr, sizeof(Dwarf_Half));
	    *return_uval = ret_value;
	    return DW_DLV_OK;
	}

    case DW_FORM_data4:{
	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   attr->ar_debug_info_ptr,
			   sizeof(Dwarf_ufixed));
	    *return_uval = ret_value;
	    return DW_DLV_OK;
	}

    case DW_FORM_data8:{
	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   attr->ar_debug_info_ptr,
			   sizeof(Dwarf_Unsigned));
	    *return_uval = ret_value;
	    return DW_DLV_OK;
	}

    case DW_FORM_udata:
	ret_value =
	    (_dwarf_decode_u_leb128(attr->ar_debug_info_ptr, NULL));
	*return_uval = ret_value;
	return DW_DLV_OK;


	/* see bug 583450. We do not allow reading sdata from a udata
	   value. Caller can retry, calling sdata */


    default:
	break;
    }
    _dwarf_error(dbg, error, DW_DLE_ATTR_FORM_BAD);
    return (DW_DLV_ERROR);
}


int
dwarf_formsdata(Dwarf_Attribute attr,
		Dwarf_Signed * return_sval, Dwarf_Error * error)
{
    Dwarf_Signed ret_value;
    Dwarf_Debug dbg;
    Dwarf_CU_Context cu_context;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }

    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    dbg = cu_context->cc_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    switch (attr->ar_attribute_form) {

    case DW_FORM_data1:
	*return_sval = (*(Dwarf_Sbyte *) attr->ar_debug_info_ptr);
	return DW_DLV_OK;

    case DW_FORM_data2:{
	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   attr->ar_debug_info_ptr,
			   sizeof(Dwarf_Shalf));
	    *return_sval = (Dwarf_Shalf) ret_value;
	    return DW_DLV_OK;

	}

    case DW_FORM_data4:{
	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   attr->ar_debug_info_ptr,
			   sizeof(Dwarf_sfixed));
	    *return_sval = (Dwarf_Sword) ret_value;
	    return DW_DLV_OK;
	}

    case DW_FORM_data8:{
	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   attr->ar_debug_info_ptr,
			   sizeof(Dwarf_Signed));
	    *return_sval = (Dwarf_Signed) ret_value;
	    return DW_DLV_OK;
	}

    case DW_FORM_sdata:
	ret_value =
	    (_dwarf_decode_s_leb128(attr->ar_debug_info_ptr, NULL));
	*return_sval = ret_value;
	return DW_DLV_OK;


	/* see bug 583450. We do not allow reading sdata from a udata
	   value. Caller can retry, calling sdata */


    default:
	break;
    }
    _dwarf_error(dbg, error, DW_DLE_ATTR_FORM_BAD);
    return (DW_DLV_ERROR);
}


int
dwarf_formblock(Dwarf_Attribute attr,
		Dwarf_Block ** return_block, Dwarf_Error * error)
{
    Dwarf_CU_Context cu_context;
    Dwarf_Debug dbg;
    Dwarf_Unsigned length;
    Dwarf_Small *data;
    Dwarf_Word leb128_length;
    Dwarf_Block *ret_block;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }

    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    if (cu_context->cc_dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    dbg = cu_context->cc_dbg;

    switch (attr->ar_attribute_form) {

    case DW_FORM_block1:
	length = *(Dwarf_Small *) attr->ar_debug_info_ptr;
	data = attr->ar_debug_info_ptr + sizeof(Dwarf_Small);
	break;

    case DW_FORM_block2:
	READ_UNALIGNED(dbg, length, Dwarf_Unsigned,
		       attr->ar_debug_info_ptr, sizeof(Dwarf_Half));
	data = attr->ar_debug_info_ptr + sizeof(Dwarf_Half);
	break;

    case DW_FORM_block4:
	READ_UNALIGNED(dbg, length, Dwarf_Unsigned,
		       attr->ar_debug_info_ptr, sizeof(Dwarf_ufixed));
	data = attr->ar_debug_info_ptr + sizeof(Dwarf_ufixed);
	break;

    case DW_FORM_block:
	length = _dwarf_decode_u_leb128(attr->ar_debug_info_ptr,
					&leb128_length);
	data = attr->ar_debug_info_ptr + leb128_length;
	break;

    default:
	_dwarf_error(cu_context->cc_dbg, error, DW_DLE_ATTR_FORM_BAD);
	return (DW_DLV_ERROR);
    }

    /* Check that block lies within current cu in .debug_info. */
    if (attr->ar_debug_info_ptr + length >=
	dbg->de_debug_info + cu_context->cc_debug_info_offset +
	cu_context->cc_length + cu_context->cc_length_size +
	cu_context->cc_extension_size) {
	_dwarf_error(dbg, error, DW_DLE_ATTR_FORM_SIZE_BAD);
	return (DW_DLV_ERROR);
    }

    ret_block = (Dwarf_Block *) _dwarf_get_alloc(dbg, DW_DLA_BLOCK, 1);
    if (ret_block == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    ret_block->bl_len = length;
    ret_block->bl_data = (Dwarf_Ptr) data;
    ret_block->bl_from_loclist = 0;
    ret_block->bl_section_offset = data - dbg->de_debug_info;


    *return_block = ret_block;
    return (DW_DLV_OK);
}


/* Contrary to long standing documentation,
   The string pointer returned thru return_str must
   never have dwarf_dealloc() applied to it.
   Documentation fixed July 2005.
*/
int
dwarf_formstring(Dwarf_Attribute attr,
		 char **return_str, Dwarf_Error * error)
{
    Dwarf_CU_Context cu_context;
    Dwarf_Debug dbg;
    Dwarf_Unsigned offset;
    int res;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }

    cu_context = attr->ar_cu_context;
    if (cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    if (cu_context->cc_dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    dbg = cu_context->cc_dbg;

    if (attr->ar_attribute_form == DW_FORM_string) {

	void *begin = attr->ar_debug_info_ptr;

	if (0 == dbg->de_assume_string_in_bounds) {
	    /* Check that string lies within current cu in .debug_info. 
	     */
	    void *end = dbg->de_debug_info +
		cu_context->cc_debug_info_offset +
		cu_context->cc_length + cu_context->cc_length_size +
		cu_context->cc_extension_size;
	    if (0 == _dwarf_string_valid(begin, end)) {
		_dwarf_error(dbg, error, DW_DLE_ATTR_FORM_SIZE_BAD);
		return (DW_DLV_ERROR);
	    }
	}
	*return_str = (char *) (begin);
	return DW_DLV_OK;
    }

    if (attr->ar_attribute_form == DW_FORM_strp) {
	READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,
		       attr->ar_debug_info_ptr,
		       cu_context->cc_length_size);

	res =
	    _dwarf_load_section(dbg,
				dbg->de_debug_str_index,
				&dbg->de_debug_str, error);
	if (res != DW_DLV_OK) {
	    return res;
	}

	*return_str = (char *) (dbg->de_debug_str + offset);
	return DW_DLV_OK;
    }

    _dwarf_error(dbg, error, DW_DLE_ATTR_FORM_BAD);
    return (DW_DLV_ERROR);
}
