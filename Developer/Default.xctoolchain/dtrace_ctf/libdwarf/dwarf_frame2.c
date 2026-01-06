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

/*
	This  implements _dwarf_get_fde_list_internal()
        and related helper functions for reading cie/fde data.
*/



#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <stdlib.h>
#include "dwarf_frame.h"
#include "dwarf_arange.h"	/* using Arange as a way to build a
				   list */


static int dwarf_find_existing_cie_ptr(Dwarf_Small * cie_ptr,
				       Dwarf_Cie cur_cie_ptr,
				       Dwarf_Cie * cie_ptr_to_use_out,
				       Dwarf_Cie head_cie_ptr);
static void dealloc_fde_cie_list_internal(Dwarf_Fde head_fde_ptr,
					  Dwarf_Cie head_cie_ptr);
static int dwarf_create_cie_from_start(Dwarf_Debug dbg,
				       Dwarf_Small * cie_ptr_val,
				       Dwarf_Small * section_ptr,
				       Dwarf_Unsigned section_index,
				       Dwarf_Unsigned section_length,
				       Dwarf_Small * frame_ptr_end,
				       Dwarf_Unsigned cie_id_value,
				       Dwarf_Unsigned cie_count,
				       int use_gnu_cie_calc,
				       Dwarf_Cie * cie_ptr_to_use_out,
				       Dwarf_Error * error);

static Dwarf_Small *get_cieptr_given_offset(Dwarf_Unsigned cie_id_value,
					    int use_gnu_cie_calc,
					    Dwarf_Small * section_ptr,
					    Dwarf_Small * cie_id_addr);
static int get_gcc_eh_augmentation(Dwarf_Debug dbg,
				   Dwarf_Small * frame_ptr,
				   unsigned long
				   *size_of_augmentation_data,
				   enum Dwarf_augmentation_type augtype,
				   Dwarf_Small * section_pointer,
				   Dwarf_Small * fde_eh_encoding_out,
				   char *augmentation);

static int
  gnu_aug_encodings(Dwarf_Debug dbg, char *augmentation,
		    Dwarf_Small * aug_data, Dwarf_Unsigned aug_data_len,
		    unsigned char *pers_hand_enc_out,
		    unsigned char *lsda_enc_out,
		    unsigned char *fde_begin_enc_out,
		    Dwarf_Addr * gnu_pers_addr_out);


static int read_encoded_ptr(Dwarf_Debug dbg,
			    Dwarf_Small * input_field,
			    int gnu_encoding,
			    Dwarf_Unsigned * addr,
			    Dwarf_Small ** input_field_out);



static int qsort_compare(const void *elem1, const void *elem2);


/* Adds 'newone' to the end of the list starting at 'head'
   and makes the new one 'cur'rent. */
static void
chain_up_fde(Dwarf_Fde newone, Dwarf_Fde * head, Dwarf_Fde * cur)
{
    if (*head == NULL)
	*head = newone;
    else {
	(*cur)->fd_next = newone;
    }
    *cur = newone;

}

/* Adds 'newone' to the end of the list starting at 'head'
   and makes the new one 'cur'rent. */
static void
chain_up_cie(Dwarf_Cie newone, Dwarf_Cie * head, Dwarf_Cie * cur)
{
    if (*head == NULL)
	*head = newone;
    else {
	(*cur)->ci_next = newone;
    }
    *cur = newone;
}

#if 0
/* For debugging only. */
static void
print_prefix(struct cie_fde_prefix_s *prefix, int line)
{
    printf("prefix-print, prefix at 0x%lx, line %d\n",
	   (long) prefix, line);
    printf("  start addr 0x%lx after prefix 0x%lx\n",
	   (long) prefix->cf_start_addr,
	   (long) prefix->cf_addr_after_prefix);
    printf("  length 0x%llx, len size %d ext size %d\n", (long long)
	   (unsigned long long) prefix->cf_length,
	   prefix->cf_local_length_size,
	   prefix->cf_local_extension_size);
    printf("  cie_id 0x%llx cie_id  cie_id_addr 0x%lx\n",
	   (unsigned long long) prefix->cf_cie_id,
	   (long) prefix->cf_cie_id_addr);
    printf
	("  sec ptr 0x%lx sec index %lld sec len 0x%llx sec past end 0x%lx\n",
	 (long) prefix->cf_section_ptr,
	 (long long) prefix->cf_section_index,
	 (unsigned long long) prefix->cf_section_length,
	 (long) prefix->cf_section_ptr + prefix->cf_section_length);
}
#endif



/* Internal function called from various places to create
   lists of CIEs and FDEs.  Not directly called
   by consumer code */
int
_dwarf_get_fde_list_internal(Dwarf_Debug dbg, Dwarf_Cie ** cie_data,
			     Dwarf_Signed * cie_element_count,
			     Dwarf_Fde ** fde_data,
			     Dwarf_Signed * fde_element_count,
			     Dwarf_Small * section_ptr,
			     Dwarf_Unsigned section_index,
			     Dwarf_Unsigned section_length,
			     Dwarf_Unsigned cie_id_value,
			     int use_gnu_cie_calc, Dwarf_Error * error)
{
    /* Scans the debug_frame section. */
    Dwarf_Small *frame_ptr = section_ptr;
    Dwarf_Small *frame_ptr_end = section_ptr + section_length;



    /* 
       New_cie points to the Cie being read, and head_cie_ptr and
       cur_cie_ptr are used for chaining them up in sequence. */
    Dwarf_Cie head_cie_ptr = NULL;
    Dwarf_Cie cur_cie_ptr = NULL;
    Dwarf_Word cie_count = 0;

    /* 
       Points to a list of contiguous pointers to Dwarf_Cie structures. 
     */
    Dwarf_Cie *cie_list_ptr = 0;


    /* 
       New_fde points to the Fde being created, and head_fde_ptr and
       cur_fde_ptr are used to chain them up. */
    Dwarf_Fde head_fde_ptr = NULL;
    Dwarf_Fde cur_fde_ptr = NULL;
    Dwarf_Word fde_count = 0;

    /* 
       Points to a list of contiguous pointers to Dwarf_Fde structures. 
     */
    Dwarf_Fde *fde_list_ptr = NULL;

    Dwarf_Word i = 0;
    int res = 0;

    if (frame_ptr == 0) {
	return DW_DLV_NO_ENTRY;
    }

    /* We create the fde and cie arrays. Processing each CIE as we come 
       to it or as an FDE refers to it.  We cannot process 'late' CIEs
       late as GNU .eh_frame complexities mean we need the whole CIE
       before we can process the FDE correctly. */
    while (frame_ptr < frame_ptr_end) {

	struct cie_fde_prefix_s prefix;

	/* First read in the 'common prefix' to figure out what we are
	   to do with this entry. */
	memset(&prefix, 0, sizeof(prefix));
	res = dwarf_read_cie_fde_prefix(dbg,
					frame_ptr, section_ptr,
					section_index,
					section_length, &prefix, error);
	if (res == DW_DLV_ERROR) {
	    dealloc_fde_cie_list_internal(head_fde_ptr, head_cie_ptr);
	    return res;
	}
	if (res == DW_DLV_NO_ENTRY)
	    break;
	frame_ptr = prefix.cf_addr_after_prefix;
	if (frame_ptr >= frame_ptr_end) {
	    dealloc_fde_cie_list_internal(head_fde_ptr, head_cie_ptr);
	    _dwarf_error(dbg, error, DW_DLE_DEBUG_FRAME_LENGTH_BAD);
	    return DW_DLV_ERROR;

	}

	if (prefix.cf_cie_id == cie_id_value) {
	    /* This is a CIE.  */
	    Dwarf_Cie cie_ptr_to_use = 0;

	    res = dwarf_find_existing_cie_ptr(prefix.cf_start_addr,
						  cur_cie_ptr,
						  &cie_ptr_to_use,
						  head_cie_ptr);

	    if (res == DW_DLV_OK) {
		cur_cie_ptr = cie_ptr_to_use;
		/* Ok. Seen already. */
	    } else if (res == DW_DLV_NO_ENTRY) {
		/* CIE before its FDE in this case. */
		res = dwarf_create_cie_from_after_start(dbg,
							&prefix,
							frame_ptr,
							cie_count,
							use_gnu_cie_calc,
							&cie_ptr_to_use,
							error);
		/* ASSERT: res==DW_DLV_NO_ENTRY impossible. */
		if (res == DW_DLV_ERROR) {
		    dealloc_fde_cie_list_internal(head_fde_ptr,
						  head_cie_ptr);
		    return res;
		}
		/* ASSERT res != DW_DLV_NO_ENTRY */
		cie_count++;
		chain_up_cie(cie_ptr_to_use, &head_cie_ptr,
			     &cur_cie_ptr);
	    } else {		/* res == DW_DLV_ERROR */

		dealloc_fde_cie_list_internal(head_fde_ptr,
					      head_cie_ptr);
		return res;
	    }
	    frame_ptr = cie_ptr_to_use->ci_cie_start +
		cie_ptr_to_use->ci_length +
		cie_ptr_to_use->ci_length_size +
		cie_ptr_to_use->ci_extension_size;
	    continue;
	} else {
	    /* this is an FDE, Frame Description Entry, see the Dwarf
	       Spec, section 6.4.1 */
	    res = 0;
	    Dwarf_Cie cie_ptr_to_use = 0;
	    Dwarf_Fde fde_ptr_to_use = 0;

	    /* Do not call this twice on one prefix, as
	       prefix.cf_cie_id_addr is altered as a side effect. */
	    Dwarf_Small *cieptr_val =
		get_cieptr_given_offset(prefix.cf_cie_id,
					use_gnu_cie_calc,
					section_ptr,
					prefix.cf_cie_id_addr);

	    res = dwarf_find_existing_cie_ptr(cieptr_val,
					      cur_cie_ptr,
					      &cie_ptr_to_use,
					      head_cie_ptr);
	    if (res == DW_DLV_OK) {
		cur_cie_ptr = cie_ptr_to_use;
		/* Ok. Seen CIE already. */
	    } else if (res == DW_DLV_NO_ENTRY) {
		res = dwarf_create_cie_from_start(dbg,
						  cieptr_val,
						  section_ptr,
						  section_index,
						  section_length,
						  frame_ptr_end,
						  cie_id_value,
						  cie_count,
						  use_gnu_cie_calc,
						  &cie_ptr_to_use,
						  error);
		if (res == DW_DLV_ERROR) {
		    dealloc_fde_cie_list_internal(head_fde_ptr,
						  head_cie_ptr);
		    return res;
		} else if (res == DW_DLV_NO_ENTRY) {
		    return res;
		}
		++cie_count;
		chain_up_cie(cie_ptr_to_use, &head_cie_ptr,
			     &cur_cie_ptr);

	    } else {
		/* DW_DLV_ERROR */
		return res;
	    }

	    res = dwarf_create_fde_from_after_start(dbg,
						    &prefix,
						    frame_ptr,
						    use_gnu_cie_calc,
						    cie_ptr_to_use,
						    &fde_ptr_to_use,
						    error);
	    if (res == DW_DLV_ERROR) {
		return res;
	    }
	    chain_up_fde(fde_ptr_to_use, &head_fde_ptr, &cur_fde_ptr);
	    fde_count++;
	    /* ASSERT: DW_DLV_OK. */
	    frame_ptr = fde_ptr_to_use->fd_fde_start +
		fde_ptr_to_use->fd_length +
		fde_ptr_to_use->fd_length_size +
		fde_ptr_to_use->fd_extension_size;
	    continue;

	}

    }

    /* Now build list of CIEs from the list. */
    if (cie_count > 0) {
	cie_list_ptr = (Dwarf_Cie *)
	    _dwarf_get_alloc(dbg, DW_DLA_LIST, cie_count);
    } else {
	dealloc_fde_cie_list_internal(head_fde_ptr, head_cie_ptr);
	return (DW_DLV_NO_ENTRY);
    }
    if (cie_list_ptr == NULL) {
	dealloc_fde_cie_list_internal(head_fde_ptr, head_cie_ptr);
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }
    cur_cie_ptr = head_cie_ptr;
    for (i = 0; i < cie_count; i++) {
	*(cie_list_ptr + i) = cur_cie_ptr;
	cur_cie_ptr = cur_cie_ptr->ci_next;
    }



    /* Now build array of FDEs from the list. */
    if (fde_count > 0) {
	fde_list_ptr = (Dwarf_Fde *)
	    _dwarf_get_alloc(dbg, DW_DLA_LIST, fde_count);
    } else {
	dwarf_fde_cie_list_dealloc(dbg, cie_list_ptr, cie_count,
				   /* fde_data */ 0,
				   /* fde_element_count */ 0);
	dealloc_fde_cie_list_internal(head_fde_ptr,	/* head cie_ptr 
							 */
				      0);
	return (DW_DLV_NO_ENTRY);
    }
    if (fde_list_ptr == NULL) {
	dwarf_fde_cie_list_dealloc(dbg, cie_list_ptr, cie_count,
				   /* fde_data */ 0,
				   /* fde_element_count */ 0);
	dealloc_fde_cie_list_internal(head_fde_ptr,	/* head cie_ptr 
							 */
				      0);
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }
    cur_fde_ptr = head_fde_ptr;
    for (i = 0; i < fde_count; i++) {
	*(fde_list_ptr + i) = cur_fde_ptr;
	cur_fde_ptr = cur_fde_ptr->fd_next;
    }


    /* Return arguments. */
    *cie_data = cie_list_ptr;
    *cie_element_count = cie_count;
    dbg->de_cie_data = cie_list_ptr;
    dbg->de_cie_count = cie_count;

    *fde_data = fde_list_ptr;
    *fde_element_count = fde_count;
    dbg->de_fde_data = fde_list_ptr;
    dbg->de_fde_count = fde_count;

    /* Sort the list by the address so that dwarf_get_fde_at_pc() can
       binary search this list. */
    qsort((void *) fde_list_ptr, fde_count, sizeof(Dwarf_Ptr),
	  qsort_compare);

    return (DW_DLV_OK);
}

/* Internal function, not called by consumer code.
   'prefix' has accumulated the info up thru the cie-id
   and now we consume the rest and build a Dwarf_Cie_s structure.
*/
int
dwarf_create_cie_from_after_start(Dwarf_Debug dbg,
				  struct cie_fde_prefix_s *prefix,
				  Dwarf_Small * frame_ptr,
				  Dwarf_Unsigned cie_count,
				  int use_gnu_cie_calc,
				  Dwarf_Cie * cie_ptr_out,
				  Dwarf_Error * error)
{
    Dwarf_Cie new_cie = 0;

    /* egcs-1.1.2 .eh_frame uses 0 as the distinguishing id. sgi uses
       -1 (in .debug_frame). .eh_frame not quite identical to
       .debug_frame */
    Dwarf_Small eh_fde_encoding = 0;
    Dwarf_Small *augmentation = 0;
    Dwarf_Sword data_alignment_factor = -1;
    Dwarf_Word code_alignment_factor = 4;
    Dwarf_Unsigned return_address_register = 31;
    int local_length_size = 0;
    Dwarf_Word leb128_length = 0;
    Dwarf_Unsigned cie_aug_data_len = 0;
    Dwarf_Small *cie_aug_data = 0;
    Dwarf_Addr gnu_personality_handler_addr = 0;
    unsigned char gnu_personality_handler_encoding = 0;
    unsigned char gnu_lsda_encoding = 0;
    unsigned char gnu_fde_begin_encoding = 0;


    enum Dwarf_augmentation_type augt = aug_unknown;


    /* this is a CIE, Common Information Entry: See the dwarf spec,
       section 6.4.1 */
    Dwarf_Small version = *(Dwarf_Small *) frame_ptr;

    frame_ptr++;
    if (version != DW_CIE_VERSION && version != DW_CIE_VERSION3 && version != DW_CIE_VERSION4) {
	_dwarf_error(dbg, error, DW_DLE_FRAME_VERSION_BAD);
	return (DW_DLV_ERROR);
    }

    augmentation = frame_ptr;
    frame_ptr = frame_ptr + strlen((char *) frame_ptr) + 1;
    augt = _dwarf_get_augmentation_type(dbg,
					augmentation, use_gnu_cie_calc);
    if (augt == aug_eh) {
	/* REFERENCED *//* Not used in this instance */
	Dwarf_Unsigned exception_table_addr;

	/* this is per egcs-1.1.2 as on RH 6.0 */
	READ_UNALIGNED(dbg, exception_table_addr,
		       Dwarf_Unsigned, frame_ptr, local_length_size);
	frame_ptr += local_length_size;
    }
    {
	Dwarf_Unsigned lreg = 0;
	unsigned long size = 0;

	DECODE_LEB128_UWORD(frame_ptr, lreg);
	code_alignment_factor = (Dwarf_Word) lreg;

	data_alignment_factor =
	    (Dwarf_Sword) _dwarf_decode_s_leb128(frame_ptr,
						 &leb128_length);

	frame_ptr = frame_ptr + leb128_length;

	return_address_register =
	    _dwarf_get_return_address_reg(frame_ptr, version, &size);
	if (return_address_register > DW_FRAME_LAST_REG_NUM) {
	    _dwarf_error(dbg, error, DW_DLE_CIE_RET_ADDR_REG_ERROR);
	    return (DW_DLV_ERROR);
	}
	frame_ptr += size;
    }
    switch (augt) {
    case aug_empty_string:
	break;
    case aug_irix_mti_v1:
	break;
    case aug_irix_exception_table:{
	    Dwarf_Unsigned lreg = 0;
	    Dwarf_Word length_of_augmented_fields;

	    /* Decode the length of augmented fields. */
	    DECODE_LEB128_UWORD(frame_ptr, lreg);
	    length_of_augmented_fields = (Dwarf_Word) lreg;


	    /* set the frame_ptr to point at the instruction start. */
	    frame_ptr += length_of_augmented_fields;
	}
	break;

    case aug_eh:{

	    int err = 0;
	    unsigned long increment = 0;

	    if (!use_gnu_cie_calc) {
		/* This should be impossible. */
		_dwarf_error(dbg, error,
			     DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
		return DW_DLV_ERROR;
	    }

	    err = get_gcc_eh_augmentation(dbg, frame_ptr, &increment,
					  augt,
					  prefix->cf_section_ptr,
					  &eh_fde_encoding,
					  (char *) augmentation);
	    if (err == DW_DLV_ERROR) {
		_dwarf_error(dbg, error,
			     DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
		return DW_DLV_ERROR;
	    }
	    frame_ptr += increment;
	    break;
	}
    case aug_gcc_eh_z:{
	    /* Here we have Augmentation Data Length (uleb128) followed 
	       by Augmentation Data bytes. */
	    int res;
	    Dwarf_Unsigned adlen = 0;

	    DECODE_LEB128_UWORD(frame_ptr, adlen);
	    cie_aug_data_len = adlen;
	    cie_aug_data = frame_ptr;
	    res = gnu_aug_encodings(dbg,
				    (char *) augmentation,
				    cie_aug_data,
				    cie_aug_data_len,
				    &gnu_personality_handler_encoding,
				    &gnu_lsda_encoding,
				    &gnu_fde_begin_encoding,
				    &gnu_personality_handler_addr);
	    if (res != DW_DLV_OK) {
		_dwarf_error(dbg, error,
			     DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
		return res;
	    }


	    frame_ptr += adlen;
	    break;
	}
    default:{
	    /* We do not understand the augmentation string. No
	       assumption can be made about any fields other than what
	       we have already read. */
	    frame_ptr = prefix->cf_start_addr +
		prefix->cf_length + prefix->cf_local_length_size
		+ prefix->cf_local_extension_size;
	    /* FIX -- What are the values of data_alignment_factor,
	       code_alignement_factor, return_address_register and
	       instruction start? They were clearly uninitalized in the 
	       previous version and I am leaving them the same way. */
	    break;
	}
    }				/* End switch on augmentation type. */

    new_cie = (Dwarf_Cie) _dwarf_get_alloc(dbg, DW_DLA_CIE, 1);
    if (new_cie == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    new_cie->ci_cie_version_number = version;
    new_cie->ci_initial_table = NULL;
    new_cie->ci_length = (Dwarf_Word) prefix->cf_length;
    new_cie->ci_length_size = prefix->cf_local_length_size;
    new_cie->ci_extension_size = prefix->cf_local_extension_size;
    new_cie->ci_augmentation = (char *) augmentation;

    new_cie->ci_data_alignment_factor =
	(Dwarf_Sbyte) data_alignment_factor;
    new_cie->ci_code_alignment_factor =
	(Dwarf_Small) code_alignment_factor;
    new_cie->ci_return_address_register = return_address_register;
    new_cie->ci_cie_start = prefix->cf_start_addr;
    new_cie->ci_cie_instr_start = frame_ptr;
    new_cie->ci_dbg = dbg;
    new_cie->ci_augmentation_type = augt;
    new_cie->ci_gnu_eh_augmentation_len = cie_aug_data_len;
    new_cie->ci_gnu_eh_augmentation_bytes = cie_aug_data;
    new_cie->ci_gnu_personality_handler_encoding =
	gnu_personality_handler_encoding;
    new_cie->ci_gnu_personality_handler_addr =
	gnu_personality_handler_addr;
    new_cie->ci_gnu_lsda_encoding = gnu_lsda_encoding;
    new_cie->ci_gnu_fde_begin_encoding = gnu_fde_begin_encoding;

    new_cie->ci_index = cie_count;
    new_cie->ci_section_ptr = prefix->cf_section_ptr;

    *cie_ptr_out = new_cie;
    return DW_DLV_OK;

}


/* Internal function, not called by consumer code.
   'prefix' has accumulated the info up thru the cie-id
   and now we consume the rest and build a Dwarf_Fde_s structure.
*/

int
dwarf_create_fde_from_after_start(Dwarf_Debug dbg,
				  struct cie_fde_prefix_s *prefix,
				  Dwarf_Small * frame_ptr,
				  int use_gnu_cie_calc,
				  Dwarf_Cie cie_ptr_in,
				  Dwarf_Fde * fde_ptr_out,
				  Dwarf_Error * error)
{
    Dwarf_Fde new_fde = 0;
    Dwarf_Cie cieptr = cie_ptr_in;
    Dwarf_Small *saved_frame_ptr = 0;

    Dwarf_Small *initloc = frame_ptr;
    Dwarf_Signed offset_into_exception_tables
	/* must be min dwarf_sfixed in size */
	= (Dwarf_Signed) DW_DLX_NO_EH_OFFSET;
    Dwarf_Small *fde_aug_data = 0;
    Dwarf_Unsigned fde_aug_data_len = 0;
    Dwarf_Addr cie_base_offset = prefix->cf_cie_id;
    Dwarf_Addr initial_location = 0;	/* must be min de_pointer_size
					   bytes in size */
    Dwarf_Addr address_range = 0;	/* must be min de_pointer_size
					   bytes in size */

    enum Dwarf_augmentation_type augt = cieptr->ci_augmentation_type;

    if (augt == aug_gcc_eh_z) {
	/* If z augmentation this is eh_frame, and initial_location and 
	   address_range in the FDE are read according to the CIE
	   augmentation string instructions.  */

	{
	    Dwarf_Small *fp_updated = 0;
	    int res = res = read_encoded_ptr(dbg, frame_ptr,
					     cieptr->
					     ci_gnu_fde_begin_encoding,
					     &initial_location,
					     &fp_updated);

	    if (res != DW_DLV_OK) {
		_dwarf_error(dbg, error,
			     DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
		return DW_DLV_ERROR;
	    }
	    frame_ptr = fp_updated;
	    res = read_encoded_ptr(dbg, frame_ptr,
				   cieptr->ci_gnu_fde_begin_encoding,
				   &address_range, &fp_updated);
	    if (res != DW_DLV_OK) {
		_dwarf_error(dbg, error,
			     DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
		return DW_DLV_ERROR;
	    }
	    frame_ptr = fp_updated;
	}
	{
	    Dwarf_Unsigned adlen = 0;

	    DECODE_LEB128_UWORD(frame_ptr, adlen);
	    fde_aug_data_len = adlen;
	    fde_aug_data = frame_ptr;
	    frame_ptr += adlen;
	}

    } else {
	READ_UNALIGNED(dbg, initial_location, Dwarf_Addr,
		       frame_ptr, dbg->de_pointer_size);
	frame_ptr += dbg->de_pointer_size;

	READ_UNALIGNED(dbg, address_range, Dwarf_Addr,
		       frame_ptr, dbg->de_pointer_size);
	frame_ptr += dbg->de_pointer_size;
    }





    switch (augt) {
    case aug_irix_mti_v1:
    case aug_empty_string:
	break;
    case aug_irix_exception_table:{
	    Dwarf_Unsigned lreg = 0;
	    Dwarf_Word length_of_augmented_fields = 0;

	    DECODE_LEB128_UWORD(frame_ptr, lreg);
	    length_of_augmented_fields = (Dwarf_Word) lreg;

	    saved_frame_ptr = frame_ptr;
	    /* The first word is an offset into exception tables.
	       Defined as a 32bit offset even for CC -64. */
	    READ_UNALIGNED(dbg, offset_into_exception_tables,
			   Dwarf_Addr, frame_ptr, sizeof(Dwarf_sfixed));
	    SIGN_EXTEND(offset_into_exception_tables,
			sizeof(Dwarf_sfixed));
	    frame_ptr = saved_frame_ptr + length_of_augmented_fields;
	}
	break;
    case aug_eh:{
	    Dwarf_Unsigned eh_table_value = 0;

	    if (!use_gnu_cie_calc) {
		/* This should be impossible. */
		_dwarf_error(dbg, error,
			     DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
		return DW_DLV_ERROR;
	    }

	    /* gnu eh fde case. we do not need to do anything */
	     /*REFERENCED*/	/* Not used in this instance of the
				   macro */
		READ_UNALIGNED(dbg, eh_table_value,
			       Dwarf_Unsigned, frame_ptr,
			       dbg->de_pointer_size);
	    frame_ptr += dbg->de_pointer_size;
	}
	break;

    case aug_gcc_eh_z:{
	    /* The Augmentation Data Length is here, followed by the
	       Augmentation Data bytes themselves. */
	}
	break;
	default:;
    }				/* End switch on augmentation type */
    new_fde = (Dwarf_Fde) _dwarf_get_alloc(dbg, DW_DLA_FDE, 1);
    if (new_fde == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    new_fde->fd_length = prefix->cf_length;
    new_fde->fd_length_size = prefix->cf_local_length_size;
    new_fde->fd_extension_size = prefix->cf_local_extension_size;
    new_fde->fd_cie_offset = cie_base_offset;
    new_fde->fd_cie_index = cieptr->ci_index;
    new_fde->fd_cie = cieptr;
    new_fde->fd_initial_location = initial_location;
    new_fde->fd_initial_loc_pos = initloc;
    new_fde->fd_address_range = address_range;
    new_fde->fd_fde_start = prefix->cf_start_addr;
    new_fde->fd_fde_instr_start = frame_ptr;
    new_fde->fd_dbg = dbg;
    new_fde->fd_offset_into_exception_tables =
	offset_into_exception_tables;

    new_fde->fd_section_ptr = prefix->cf_section_ptr;
    new_fde->fd_section_index = prefix->cf_section_index;
    new_fde->fd_section_length = prefix->cf_section_length;

    new_fde->fd_gnu_eh_augmentation_bytes = fde_aug_data;
    new_fde->fd_gnu_eh_augmentation_len = fde_aug_data_len;

    *fde_ptr_out = new_fde;
    return DW_DLV_OK;
}

/* called by qsort to compare FDE entries.
   Consumer code expects the array of FDE pointers to be in address order.
*/
static int
qsort_compare(const void *elem1, const void *elem2)
{
    Dwarf_Fde fde1 = *(Dwarf_Fde *) elem1;
    Dwarf_Fde fde2 = *(Dwarf_Fde *) elem2;
    Dwarf_Addr addr1 = fde1->fd_initial_location;
    Dwarf_Addr addr2 = fde2->fd_initial_location;

    if (addr1 < addr2) {
	return -1;
    } else if (addr1 > addr2) {
	return 1;
    }
    return 0;
}


/* Read in the common cie/fde prefix, including reading
 * the cie-value which shows which this is: cie or fde.
 * */
int
dwarf_read_cie_fde_prefix(Dwarf_Debug dbg,
			  Dwarf_Small * frame_ptr_in,
			  Dwarf_Small * section_ptr_in,
			  Dwarf_Unsigned section_index_in,
			  Dwarf_Unsigned section_length_in,
			  struct cie_fde_prefix_s *data_out,
			  Dwarf_Error * error)
{
    Dwarf_Unsigned length = 0;
    int local_length_size = 0;
    int local_extension_size = 0;
    Dwarf_Small *frame_ptr = frame_ptr_in;
    Dwarf_Small *cie_ptr_addr = 0;
    Dwarf_Unsigned cie_id = 0;

    /* READ_AREA_LENGTH updates frame_ptr for consumed bytes */
    READ_AREA_LENGTH(dbg, length, Dwarf_Unsigned,
		     frame_ptr, local_length_size,
		     local_extension_size);

    if (length % local_length_size != 0) {
	_dwarf_error(dbg, error, DW_DLE_DEBUG_FRAME_LENGTH_BAD);
	return (DW_DLV_ERROR);
    }

    if (length == 0) {
	/* nul bytes at end of section, seen at end of egcs eh_frame
	   sections (in a.out). Take this as meaning no more CIE/FDE
	   data. We should be very close to end of section. */
	return DW_DLV_NO_ENTRY;
    }

    cie_ptr_addr = frame_ptr;
    READ_UNALIGNED(dbg, cie_id, Dwarf_Unsigned,
		   frame_ptr, local_length_size);
    SIGN_EXTEND(cie_id, local_length_size);
    frame_ptr += local_length_size;

    data_out->cf_start_addr = frame_ptr_in;
    data_out->cf_addr_after_prefix = frame_ptr;

    data_out->cf_length = length;
    data_out->cf_local_length_size = local_length_size;
    data_out->cf_local_extension_size = local_extension_size;
    data_out->cf_cie_id = cie_id;
    data_out->cf_cie_id_addr = cie_ptr_addr;
    data_out->cf_section_ptr = section_ptr_in;
    data_out->cf_section_index = section_index_in;
    data_out->cf_section_length = section_length_in;
    return DW_DLV_OK;
}

/* On various errors previously-allocated CIEs and FDEs
   must be cleaned up.
   This helps avoid leaks in case of errors.
*/
static void
dealloc_fde_cie_list_internal(Dwarf_Fde head_fde_ptr,
			      Dwarf_Cie head_cie_ptr)
{
    Dwarf_Fde curfde = 0;
    Dwarf_Cie curcie = 0;
    Dwarf_Fde nextfde = 0;
    Dwarf_Cie nextcie = 0;

    for (curfde = head_fde_ptr; curfde; curfde = nextfde) {
	nextfde = curfde->fd_next;
	dwarf_dealloc(curfde->fd_dbg, curfde, DW_DLA_FDE);
    }
    for (curcie = head_cie_ptr; curcie; curcie = nextcie) {
	Dwarf_Frame frame = curcie->ci_initial_table;

	nextcie = curcie->ci_next;
	if (frame)
	    dwarf_dealloc(curcie->ci_dbg, frame, DW_DLA_FRAME);
	dwarf_dealloc(curcie->ci_dbg, curcie, DW_DLA_CIE);
    }
}

/* Find the cie whose id value is given: the id
 * value is, per DWARF2/3, an offset in the section. 
 * For .debug_frame, zero is a legal offset. For
 * GNU .eh_frame it is not a legal offset.
 * 'cie_ptr' is a pointer into our section, not an offset. */
static int
dwarf_find_existing_cie_ptr(Dwarf_Small * cie_ptr,
			    Dwarf_Cie cur_cie_ptr,
			    Dwarf_Cie * cie_ptr_to_use_out,
			    Dwarf_Cie head_cie_ptr)
{
    Dwarf_Cie next = 0;

    if (cur_cie_ptr && cie_ptr == cur_cie_ptr->ci_cie_start) {
	/* Usually, we use the same cie again and again. */
	*cie_ptr_to_use_out = cur_cie_ptr;
	return DW_DLV_OK;
    }
    for (next = head_cie_ptr; next; next = next->ci_next) {
	if (cie_ptr == next->ci_cie_start) {
	    *cie_ptr_to_use_out = next;
	    return DW_DLV_OK;
	}
    }
    return DW_DLV_NO_ENTRY;
}


/* We have a valid cie_ptr_val that has not been
 * turned into an internal Cie yet. Do so now.
 * Returns DW_DLV_OK or DW_DLV_ERROR, never
 * DW_DLV_NO_ENTRY.

 'section_ptr'    - Points to first byte of section data.
 'section_length' - Length of the section, in bytes.
 'frame_ptr_end'  - Points 1-past last byte of section data.
 * */
static int
dwarf_create_cie_from_start(Dwarf_Debug dbg,
			    Dwarf_Small * cie_ptr_val,
			    Dwarf_Small * section_ptr,
			    Dwarf_Unsigned section_index,
			    Dwarf_Unsigned section_length,
			    Dwarf_Small * frame_ptr_end,
			    Dwarf_Unsigned cie_id_value,
			    Dwarf_Unsigned cie_count,
			    int use_gnu_cie_calc,
			    Dwarf_Cie * cie_ptr_to_use_out,
			    Dwarf_Error * error)
{
    struct cie_fde_prefix_s prefix;
    int res = 0;
    Dwarf_Small *frame_ptr = cie_ptr_val;

    if (frame_ptr < section_ptr || frame_ptr > frame_ptr_end) {
	_dwarf_error(dbg, error, DW_DLE_DEBUG_FRAME_LENGTH_BAD);
	return DW_DLV_ERROR;
    }
    /* First read in the 'common prefix' to figure out what * we are to 
       do with this entry. IF it's not a cie * we are in big trouble. */
    memset(&prefix, 0, sizeof(prefix));
    res = dwarf_read_cie_fde_prefix(dbg, frame_ptr, section_ptr,
				    section_index, section_length,
				    &prefix, error);
    if (res == DW_DLV_ERROR) {
	return res;
    }
    if (res == DW_DLV_NO_ENTRY) {
	/* error. */
	_dwarf_error(dbg, error, DW_DLE_FRAME_CIE_DECODE_ERROR);
	return DW_DLV_ERROR;

    }

    if (prefix.cf_cie_id != cie_id_value) {
	_dwarf_error(dbg, error, DW_DLE_FRAME_CIE_DECODE_ERROR);
	return DW_DLV_ERROR;
    }
    frame_ptr = prefix.cf_addr_after_prefix;
    res = dwarf_create_cie_from_after_start(dbg,
					    &prefix,
					    frame_ptr,
					    cie_count,
					    use_gnu_cie_calc,
					    cie_ptr_to_use_out, error);

    return res;

}


/* This is for gnu eh frames, the 'z' case.
   We find the letter involved
   Return the augmentation character and, if applicable,
   the personality routine address.

   personality_routine_out - 
	if 'P' is augchar, is personality handler addr. 
        Otherwise is not set.
   aug_data  - if 'P' points  to data space of the
   aug_data_len - length of areas aug_data points to.
   
*/
#if 0
/* For debugging only. */
static void
dump_bytes(Dwarf_Small * start, long len)
{
    Dwarf_Small *end = start + len;
    Dwarf_Small *cur = start;

    for (; cur < end; cur++) {
	printf(" byte %d, data %02x\n", (int) (cur - start), *cur);
    }

}
#endif
static int
gnu_aug_encodings(Dwarf_Debug dbg, char *augmentation,
		  Dwarf_Small * aug_data, Dwarf_Unsigned aug_data_len,
		  unsigned char *pers_hand_enc_out,
		  unsigned char *lsda_enc_out,
		  unsigned char *fde_begin_enc_out,
		  Dwarf_Addr * gnu_pers_addr_out)
{
    char *nc = 0;
    Dwarf_Small *cur_aug_p = aug_data;
    Dwarf_Small *end_aug_p = aug_data + aug_data_len;

    for (nc = augmentation; *nc; ++nc) {
	char c = *nc;

	switch (c) {
	case 'z':
	    continue;

	case 'L':
	    if (cur_aug_p > end_aug_p) {
		return DW_DLV_ERROR;
	    }
	    *lsda_enc_out = *(unsigned char *) cur_aug_p;
	    ++cur_aug_p;
	    break;
	case 'R':
	    if (cur_aug_p >= end_aug_p) {
		return DW_DLV_ERROR;
	    }
	    *fde_begin_enc_out = *(unsigned char *) cur_aug_p;
	    ++cur_aug_p;
	    break;
	case 'P':{
		int res = 0;
		Dwarf_Small *updated_aug_p = 0;
		unsigned char encoding = 0;

		if (cur_aug_p >= end_aug_p) {
		    return DW_DLV_ERROR;
		}
		encoding = *(unsigned char *) cur_aug_p;
		*pers_hand_enc_out = encoding;
		++cur_aug_p;
		if (cur_aug_p > end_aug_p) {
		    return DW_DLV_ERROR;
		}
		res = read_encoded_ptr(dbg,
				       cur_aug_p,
				       encoding,
				       gnu_pers_addr_out,
				       &updated_aug_p);
		if (res != DW_DLV_OK) {
		    return res;
		}
		cur_aug_p = updated_aug_p;
		if (cur_aug_p > end_aug_p) {
		    return DW_DLV_ERROR;
		}
	    }
	    break;
	default:
	    return DW_DLV_ERROR;

	}
    }

    return DW_DLV_OK;
}

/* Given augmentation character (the encoding) giving the
address format, read the address from input_field
and return an incremented value 1 past the input bytes of the
address.
Push the address read back thru the *addr pointer.
See LSB (Linux Standar Base)  exception handling documents. 
*/
static int
read_encoded_ptr(Dwarf_Debug dbg,
		 Dwarf_Small * input_field,
		 int gnu_encoding,
		 Dwarf_Unsigned * addr,
		 Dwarf_Small ** input_field_updated)
{
    Dwarf_Word length = 0;
    int value_type = gnu_encoding & 0xf;

    if (gnu_encoding == 0xff) {
	/* There is no data here. */

	*addr = 0;
	*input_field_updated = input_field;
	/* Should we return DW_DLV_NO_ENTRY? */
	return DW_DLV_OK;
    }
    switch (value_type) {
    case DW_EH_PE_absptr:{
	    /* value_type is zero. Treat as pointer size of the object. 
	     */
	    Dwarf_Unsigned ret_value = 0;

	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   input_field, dbg->de_pointer_size);
	    *addr = ret_value;
	    *input_field_updated = input_field + dbg->de_pointer_size;
	}
	break;
    case DW_EH_PE_uleb128:{
	    Dwarf_Unsigned val = _dwarf_decode_u_leb128(input_field,
							&length);

	    *addr = val;
	    *input_field_updated = input_field + length;
	}
	break;
    case DW_EH_PE_udata2:{
	    Dwarf_Unsigned ret_value = 0;

	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   input_field, 2);
	    *addr = ret_value;
	    *input_field_updated = input_field + 2;
	}
	break;

    case DW_EH_PE_udata4:{

	    Dwarf_Unsigned ret_value = 0;

	    /* ASSERT: sizeof(Dwarf_ufixed) == 4 */
	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   input_field, sizeof(Dwarf_ufixed));
	    *addr = ret_value;
	    *input_field_updated = input_field + sizeof(Dwarf_ufixed);
	}
	break;

    case DW_EH_PE_udata8:{
	    Dwarf_Unsigned ret_value = 0;

	    /* ASSERT: sizeof(Dwarf_Unsigned) == 8 */
	    READ_UNALIGNED(dbg, ret_value, Dwarf_Unsigned,
			   input_field, sizeof(Dwarf_Unsigned));
	    *addr = ret_value;
	    *input_field_updated = input_field + sizeof(Dwarf_Unsigned);
	}
	break;

    case DW_EH_PE_sleb128:{
	    Dwarf_Signed val = _dwarf_decode_s_leb128(input_field,
						      &length);

	    *addr = (Dwarf_Unsigned) val;
	    *input_field_updated = input_field + length;
	}
	break;
    case DW_EH_PE_sdata2:{
	    Dwarf_Unsigned val = 0;

	    READ_UNALIGNED(dbg, val, Dwarf_Unsigned, input_field, 2);
	    SIGN_EXTEND(val, 2);
	    *addr = (Dwarf_Unsigned) val;
	    *input_field_updated = input_field + 2;
	}
	break;

    case DW_EH_PE_sdata4:{
	    Dwarf_Unsigned val = 0;

	    /* ASSERT: sizeof(Dwarf_ufixed) == 4 */
	    READ_UNALIGNED(dbg, val,
			   Dwarf_Unsigned, input_field,
			   sizeof(Dwarf_ufixed));
	    SIGN_EXTEND(val, sizeof(Dwarf_ufixed));
	    *addr = (Dwarf_Unsigned) val;
	    *input_field_updated = input_field + sizeof(Dwarf_ufixed);
	}
	break;
    case DW_EH_PE_sdata8:{
	    Dwarf_Unsigned val = 0;

	    /* ASSERT: sizeof(Dwarf_Unsigned) == 8 */
	    READ_UNALIGNED(dbg, val,
			   Dwarf_Unsigned, input_field,
			   sizeof(Dwarf_Unsigned));
	    *addr = (Dwarf_Unsigned) val;
	    *input_field_updated = input_field + sizeof(Dwarf_Unsigned);
	}
	break;
    default:
	return DW_DLV_ERROR;

    };

    return DW_DLV_OK;
}




/* 
	All augmentation string checking done here now.

	For .eh_frame, gcc from 3.3 uses the z style, earlier used
 	only "eh" as augmentation.  We don't yet handle 
        decoding .eh_frame with the z style extensions like L P. 

	These are nasty heuristics, but then that's life
        as augmentations are implementation specific.
*/
/* ARGSUSED */
enum Dwarf_augmentation_type
_dwarf_get_augmentation_type(Dwarf_Debug dbg,
			     Dwarf_Small * augmentation_string,
			     int is_gcc_eh_frame)
{
#pragma unused(dbg)
    enum Dwarf_augmentation_type t = aug_unknown;
    char *ag_string = (char *) augmentation_string;

    if (ag_string[0] == 0) {
	/* Empty string. We'll just guess that we know what this means: 
	   standard dwarf2/3 with no implementation-defined fields.  */
	t = aug_empty_string;
    } else if (strcmp(ag_string, DW_DEBUG_FRAME_AUGMENTER_STRING) == 0) {
	/* The string is "mti v1". Used internally at SGI, probably
	   never shipped. Replaced by "z". Treat like 'nothing
	   special'.  */
	t = aug_irix_mti_v1;
    } else if (ag_string[0] == 'z') {
	/* If it's IRIX cc, z means aug_irix_exception_table. z1 z2
	   were designed as for IRIX CC, but never implemented */
	/* If it's gcc, z may be any of several things. "z" or z
	   followed optionally followed by one or more of L R P, each
	   of which means a value may be present. Should be in eh_frame 
	   only, I think. */
	if (is_gcc_eh_frame) {
	    t = aug_gcc_eh_z;
	} else if (ag_string[1] == 0) {
	    /* This is the normal IRIX C++ case, where there is an
	       offset into a table in each fde. The table being for
	       IRIX CC exception handling.  */
	    /* DW_CIE_AUGMENTER_STRING_V0 "z" */
	    t = aug_irix_exception_table;
	}			/* Else unknown. */
    } else if (strncmp(ag_string, "eh", 2) == 0) {
	/* gcc .eh_frame augmentation for egcs and gcc 2.x, at least
	   for x86. */
	t = aug_eh;
    }
    return t;
}

/* Using augmentation, and version 
   read in the augmentation data for GNU eh. 

   Return DW_DLV_OK if we succeeded,
   DW_DLV_ERR if we fail.

   On success, update  'size_of_augmentation_data' with
   the length of the fields that are part of augmentation (so the
   caller can increment frame_ptr appropriately).

   'frame_ptr' points within section.
   'section_pointer' points to section base address in memory.
*/
/* ARGSUSED */
static int
get_gcc_eh_augmentation(Dwarf_Debug dbg, Dwarf_Small * frame_ptr,
			unsigned long *size_of_augmentation_data,
			enum Dwarf_augmentation_type augtype,
			Dwarf_Small * section_pointer,
			Dwarf_Small * fde_eh_encoding_out,
			char *augmentation)
{
#pragma unused(dbg, section_pointer, fde_eh_encoding_out)
    char *suffix = 0;
    unsigned long augdata_size = 0;

    if (augtype == aug_gcc_eh_z) {
	/* Has leading 'z'. */
	Dwarf_Word leb128_length = 0;

	/* Dwarf_Unsigned eh_value = */
	_dwarf_decode_u_leb128(frame_ptr, &leb128_length);
	augdata_size += leb128_length;
	frame_ptr += leb128_length;
	suffix = augmentation + 1;
    } else {
	/* Prefix is 'eh'.  As in gcc 3.2. No suffix present
	   apparently. */
	suffix = augmentation + 2;
    }
    for (; *suffix; ++suffix) {
	/* We have no idea what this is as yet. Some extensions beyond
	   dwarf exist which we do not yet handle. */
	return DW_DLV_ERROR;

    }

    *size_of_augmentation_data = augdata_size;
    return DW_DLV_OK;
}


/* Make the 'cie_id_addr' consistent across .debug_frame and .eh_frame.
   Calculate a pointer into section bytes given a cie_id, which is
   trivial for .debug_frame, but a bit more work for .eh_frame.  
*/
static Dwarf_Small *
get_cieptr_given_offset(Dwarf_Unsigned cie_id_value,
			int use_gnu_cie_calc,
			Dwarf_Small * section_ptr,
			Dwarf_Small * cie_id_addr)
{
    Dwarf_Small *cieptr = 0;

    if (use_gnu_cie_calc) {
	/* cie_id value is offset, in section, of the cie_id itself, to 
	   use vm ptr of the value, less the value, to get to the cie
	   itself. In addition, munge *cie_id_addr to look *as if* it
	   was from real dwarf. */
	cieptr = (Dwarf_Small *) ((Dwarf_Unsigned) cie_id_addr) -
	    ((Dwarf_Unsigned) cie_id_value);
    } else {
	/* Traditional dwarf section offset is in cie_id */
	cieptr = (section_ptr + cie_id_value);
    }
    return cieptr;
}

/* To properly release all spaced used.
   Earlier approaches (before July 15, 2005)
   letting client do the dealloc directly left
   some data allocated.
   This is directly called by consumer code.
*/
void
dwarf_fde_cie_list_dealloc(Dwarf_Debug dbg,
			   Dwarf_Cie * cie_data,
			   Dwarf_Signed cie_element_count,
			   Dwarf_Fde * fde_data,
			   Dwarf_Signed fde_element_count)
{
    Dwarf_Signed i = 0;

    for (i = 0; i < cie_element_count; ++i) {
	Dwarf_Frame frame = cie_data[i]->ci_initial_table;

	if (frame)
	    dwarf_dealloc(dbg, frame, DW_DLA_FRAME);
	dwarf_dealloc(dbg, cie_data[i], DW_DLA_CIE);
    }
    for (i = 0; i < fde_element_count; ++i) {
	dwarf_dealloc(dbg, fde_data[i], DW_DLA_FDE);
    }
    if (cie_data)
	dwarf_dealloc(dbg, cie_data, DW_DLA_LIST);
    if (fde_data)
	dwarf_dealloc(dbg, fde_data, DW_DLA_LIST);

}
