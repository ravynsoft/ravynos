/*

  Copyright (C) 2000,2003,2004 Silicon Graphics, Inc.  All Rights Reserved.

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
#include "dwarf_loc.h"


/*
    Given a Dwarf_Block that represents a location expression,
    this function returns a pointer to a Dwarf_Locdesc struct 
    that has its ld_cents field set to the number of location 
    operators in the block, and its ld_s field pointing to a 
    contiguous block of Dwarf_Loc structs.  However, the 
    ld_lopc and ld_hipc values are uninitialized.  Returns 
    NULL on error.  This function assumes that the length of 
    the block is greater than 0.  Zero length location expressions 
    to represent variables that have been optimized away are 
    handled in the calling function.
*/
static Dwarf_Locdesc *
_dwarf_get_locdesc(Dwarf_Debug dbg,
		   Dwarf_Block * loc_block,
		   Dwarf_Addr lowpc,
		   Dwarf_Addr highpc, Dwarf_Error * error)
{
    /* Size of the block containing the location expression. */
    Dwarf_Unsigned loc_len;

    /* Sweeps the block containing the location expression. */
    Dwarf_Small *loc_ptr;

    /* Current location operator. */
    Dwarf_Small atom;

    /* Offset of current operator from start of block. */
    Dwarf_Unsigned offset;

    /* Operands of current location operator. */
    Dwarf_Unsigned operand1, operand2;

    /* Used to chain the Dwarf_Loc_Chain_s structs. */
    Dwarf_Loc_Chain curr_loc, prev_loc, head_loc = NULL;

    /* Count of the number of location operators. */
    Dwarf_Unsigned op_count;

    /* Contiguous block of Dwarf_Loc's for Dwarf_Locdesc. */
    Dwarf_Loc *block_loc;

    /* Dwarf_Locdesc pointer to be returned. */
    Dwarf_Locdesc *locdesc;

    Dwarf_Word leb128_length;
    Dwarf_Unsigned i;

    /* ***** BEGIN CODE ***** */

    loc_len = loc_block->bl_len;
    loc_ptr = loc_block->bl_data;

    offset = 0;
    op_count = 0;
    while (offset < loc_len) {

	operand1 = 0;
	operand2 = 0;
	op_count++;

	atom = *(Dwarf_Small *) loc_ptr;
	loc_ptr++;
	offset++;

	curr_loc =
	    (Dwarf_Loc_Chain) _dwarf_get_alloc(dbg, DW_DLA_LOC_CHAIN,
					       1);
	if (curr_loc == NULL) {
	    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	    return (NULL);
	}
	curr_loc->lc_offset = offset;
	curr_loc->lc_atom = atom;
	switch (atom) {

	case DW_OP_reg0:
	case DW_OP_reg1:
	case DW_OP_reg2:
	case DW_OP_reg3:
	case DW_OP_reg4:
	case DW_OP_reg5:
	case DW_OP_reg6:
	case DW_OP_reg7:
	case DW_OP_reg8:
	case DW_OP_reg9:
	case DW_OP_reg10:
	case DW_OP_reg11:
	case DW_OP_reg12:
	case DW_OP_reg13:
	case DW_OP_reg14:
	case DW_OP_reg15:
	case DW_OP_reg16:
	case DW_OP_reg17:
	case DW_OP_reg18:
	case DW_OP_reg19:
	case DW_OP_reg20:
	case DW_OP_reg21:
	case DW_OP_reg22:
	case DW_OP_reg23:
	case DW_OP_reg24:
	case DW_OP_reg25:
	case DW_OP_reg26:
	case DW_OP_reg27:
	case DW_OP_reg28:
	case DW_OP_reg29:
	case DW_OP_reg30:
	case DW_OP_reg31:
	    break;

	case DW_OP_regx:
	    operand1 = _dwarf_decode_u_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;

	case DW_OP_lit0:
	case DW_OP_lit1:
	case DW_OP_lit2:
	case DW_OP_lit3:
	case DW_OP_lit4:
	case DW_OP_lit5:
	case DW_OP_lit6:
	case DW_OP_lit7:
	case DW_OP_lit8:
	case DW_OP_lit9:
	case DW_OP_lit10:
	case DW_OP_lit11:
	case DW_OP_lit12:
	case DW_OP_lit13:
	case DW_OP_lit14:
	case DW_OP_lit15:
	case DW_OP_lit16:
	case DW_OP_lit17:
	case DW_OP_lit18:
	case DW_OP_lit19:
	case DW_OP_lit20:
	case DW_OP_lit21:
	case DW_OP_lit22:
	case DW_OP_lit23:
	case DW_OP_lit24:
	case DW_OP_lit25:
	case DW_OP_lit26:
	case DW_OP_lit27:
	case DW_OP_lit28:
	case DW_OP_lit29:
	case DW_OP_lit30:
	case DW_OP_lit31:
	    operand1 = atom - DW_OP_lit0;
	    break;

	case DW_OP_addr:
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned,
			   loc_ptr, dbg->de_pointer_size);
	    loc_ptr += dbg->de_pointer_size;
	    offset += dbg->de_pointer_size;
	    break;

	case DW_OP_const1u:
	    operand1 = *(Dwarf_Small *) loc_ptr;
	    loc_ptr = loc_ptr + 1;
	    offset = offset + 1;
	    break;

	case DW_OP_const1s:
	    operand1 = *(Dwarf_Sbyte *) loc_ptr;
	    loc_ptr = loc_ptr + 1;
	    offset = offset + 1;
	    break;

	case DW_OP_const2u:
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 2);
	    loc_ptr = loc_ptr + 2;
	    offset = offset + 2;
	    break;

	case DW_OP_const2s:
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 2);
	    loc_ptr = loc_ptr + 2;
	    offset = offset + 2;
	    break;

	case DW_OP_const4u:
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 4);
	    loc_ptr = loc_ptr + 4;
	    offset = offset + 4;
	    break;

	case DW_OP_const4s:
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 4);
	    loc_ptr = loc_ptr + 4;
	    offset = offset + 4;
	    break;

	case DW_OP_const8u:
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 8);
	    loc_ptr = loc_ptr + 8;
	    offset = offset + 8;
	    break;

	case DW_OP_const8s:
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 8);
	    loc_ptr = loc_ptr + 8;
	    offset = offset + 8;
	    break;

	case DW_OP_constu:
	    operand1 = _dwarf_decode_u_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;

	case DW_OP_consts:
	    operand1 = _dwarf_decode_s_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;

	case DW_OP_fbreg:
	    operand1 = _dwarf_decode_s_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;

	case DW_OP_breg0:
	case DW_OP_breg1:
	case DW_OP_breg2:
	case DW_OP_breg3:
	case DW_OP_breg4:
	case DW_OP_breg5:
	case DW_OP_breg6:
	case DW_OP_breg7:
	case DW_OP_breg8:
	case DW_OP_breg9:
	case DW_OP_breg10:
	case DW_OP_breg11:
	case DW_OP_breg12:
	case DW_OP_breg13:
	case DW_OP_breg14:
	case DW_OP_breg15:
	case DW_OP_breg16:
	case DW_OP_breg17:
	case DW_OP_breg18:
	case DW_OP_breg19:
	case DW_OP_breg20:
	case DW_OP_breg21:
	case DW_OP_breg22:
	case DW_OP_breg23:
	case DW_OP_breg24:
	case DW_OP_breg25:
	case DW_OP_breg26:
	case DW_OP_breg27:
	case DW_OP_breg28:
	case DW_OP_breg29:
	case DW_OP_breg30:
	case DW_OP_breg31:
	    operand1 = _dwarf_decode_s_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;

	case DW_OP_bregx:
	    /* uleb reg num followed by sleb offset */
	    operand1 = _dwarf_decode_u_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;

	    operand2 = _dwarf_decode_s_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;

	case DW_OP_dup:
	case DW_OP_drop:
	    break;

	case DW_OP_pick:
	    operand1 = *(Dwarf_Small *) loc_ptr;
	    loc_ptr = loc_ptr + 1;
	    offset = offset + 1;
	    break;

	case DW_OP_over:
	case DW_OP_swap:
	case DW_OP_rot:
	case DW_OP_deref:
	    break;

	case DW_OP_deref_size:
	    operand1 = *(Dwarf_Small *) loc_ptr;
	    loc_ptr = loc_ptr + 1;
	    offset = offset + 1;
	    break;

	case DW_OP_xderef:
	    break;

	case DW_OP_xderef_size:
	    operand1 = *(Dwarf_Small *) loc_ptr;
	    loc_ptr = loc_ptr + 1;
	    offset = offset + 1;
	    break;

	case DW_OP_abs:
	case DW_OP_and:
	case DW_OP_div:
	case DW_OP_minus:
	case DW_OP_mod:
	case DW_OP_mul:
	case DW_OP_neg:
	case DW_OP_not:
	case DW_OP_or:
	case DW_OP_plus:
	    break;

	case DW_OP_plus_uconst:
	    operand1 = _dwarf_decode_u_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;

	case DW_OP_shl:
	case DW_OP_shr:
	case DW_OP_shra:
	case DW_OP_xor:
	    break;

	case DW_OP_le:
	case DW_OP_ge:
	case DW_OP_eq:
	case DW_OP_lt:
	case DW_OP_gt:
	case DW_OP_ne:
	    break;

	case DW_OP_skip:
	case DW_OP_bra:
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 2);
	    loc_ptr = loc_ptr + 2;
	    offset = offset + 2;
	    break;

	case DW_OP_piece:
	    operand1 = _dwarf_decode_u_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;

	case DW_OP_nop:
	    break;
	case DW_OP_push_object_address:	/* DWARF3 */
	    break;
	case DW_OP_call2:	/* DWARF3 */
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 2);
	    loc_ptr = loc_ptr + 2;
	    offset = offset + 2;
	    break;

	case DW_OP_call4:	/* DWARF3 */
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr, 4);
	    loc_ptr = loc_ptr + 4;
	    offset = offset + 4;
	    break;
	case DW_OP_call_ref:	/* DWARF3 */
	    READ_UNALIGNED(dbg, operand1, Dwarf_Unsigned, loc_ptr,
			   dbg->de_length_size);
	    loc_ptr = loc_ptr + dbg->de_length_size;
	    offset = offset + dbg->de_length_size;
	    break;

	case DW_OP_form_tls_address:	/* DWARF3f */
	    break;
	case DW_OP_call_frame_cfa:	/* DWARF3f */
	    break;
	case DW_OP_bit_piece:	/* DWARF3f */
	    /* uleb size in bits followed by uleb offset in bits */
	    operand1 = _dwarf_decode_u_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;

	    operand2 = _dwarf_decode_u_leb128(loc_ptr, &leb128_length);
	    loc_ptr = loc_ptr + leb128_length;
	    offset = offset + leb128_length;
	    break;


	default:
	    _dwarf_error(dbg, error, DW_DLE_LOC_EXPR_BAD);
	    return (NULL);
	}


	curr_loc->lc_number = operand1;
	curr_loc->lc_number2 = operand2;

	if (head_loc == NULL)
	    head_loc = prev_loc = curr_loc;
	else {
	    prev_loc->lc_next = curr_loc;
	    prev_loc = curr_loc;
	}
    }

    block_loc =
	(Dwarf_Loc *) _dwarf_get_alloc(dbg, DW_DLA_LOC_BLOCK, op_count);
    if (block_loc == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (NULL);
    }

    curr_loc = head_loc;
    for (i = 0; i < op_count; i++) {
	(block_loc + i)->lr_atom = curr_loc->lc_atom;
	(block_loc + i)->lr_number = curr_loc->lc_number;
	(block_loc + i)->lr_number2 = curr_loc->lc_number2;
	(block_loc + i)->lr_offset = curr_loc->lc_offset;

	prev_loc = curr_loc;
	curr_loc = curr_loc->lc_next;
	dwarf_dealloc(dbg, prev_loc, DW_DLA_LOC_CHAIN);
    }

    locdesc =
	(Dwarf_Locdesc *) _dwarf_get_alloc(dbg, DW_DLA_LOCDESC, 1);
    if (locdesc == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (NULL);
    }

    locdesc->ld_cents = op_count;
    locdesc->ld_s = block_loc;
    locdesc->ld_from_loclist = loc_block->bl_from_loclist;
    locdesc->ld_section_offset = loc_block->bl_section_offset;
    locdesc->ld_lopc = lowpc;
    locdesc->ld_hipc = highpc;

    return (locdesc);
}

/* Using a loclist offset to get the in-memory
   address of .debug_loc data to read, returns the loclist 
   'header' info in return_block.
*/

#define MAX_ADDR ((address_size == 8)?0xffffffffffffffffULL:0xffffffff)

static int
_dwarf_read_loc_section(Dwarf_Debug dbg,
			Dwarf_Block * return_block,
			Dwarf_Addr * lowpc, Dwarf_Addr * hipc,
			Dwarf_Off sec_offset, Dwarf_Error * error)
{
    Dwarf_Small *beg = dbg->de_debug_loc + sec_offset;
    int address_size = dbg->de_pointer_size;

    Dwarf_Addr start_addr = 0;
    Dwarf_Addr end_addr = 0;
    Dwarf_Half exprblock_size = 0;
    Dwarf_Unsigned exprblock_off =
	2 * address_size + sizeof(Dwarf_Half);

    if (sec_offset >= dbg->de_debug_loc_size) {
	/* We're at the end. No more present. */
	return DW_DLV_NO_ENTRY;
    }

    /* If it goes past end, error */
    if (exprblock_off > dbg->de_debug_loc_size) {
	_dwarf_error(NULL, error, DW_DLE_DEBUG_LOC_SECTION_SHORT);
	return DW_DLV_ERROR;
    }

    READ_UNALIGNED(dbg, start_addr, Dwarf_Addr, beg, address_size);
    READ_UNALIGNED(dbg, end_addr, Dwarf_Addr,
		   beg + address_size, address_size);
    if (start_addr == 0 && end_addr == 0) {
	/* If start_addr and end_addr are 0, it's the end and no
	   exprblock_size field follows. */
	exprblock_size = 0;
	exprblock_off -= sizeof(Dwarf_Half);
    } else if (start_addr == MAX_ADDR) {
	/* end address is a base address, no exprblock_size field here
	   either */
	exprblock_size = 0;
	exprblock_off -= sizeof(Dwarf_Half);
    } else {

	READ_UNALIGNED(dbg, exprblock_size, Dwarf_Half,
		       beg + 2 * address_size, sizeof(Dwarf_Half));
	/* exprblock_size can be zero, means no expression */
	if ((exprblock_off + exprblock_size) > dbg->de_debug_loc_size) {
	    _dwarf_error(NULL, error, DW_DLE_DEBUG_LOC_SECTION_SHORT);
	    return DW_DLV_ERROR;
	}
    }
#undef MAX_ADDR
    *lowpc = start_addr;
    *hipc = end_addr;

    return_block->bl_len = exprblock_size;
    return_block->bl_from_loclist = 1;
    return_block->bl_data = beg + exprblock_off;
    return_block->bl_section_offset =
	((Dwarf_Small *) return_block->bl_data) - dbg->de_debug_loc;

    return DW_DLV_OK;

}
static int
_dwarf_get_loclist_count(Dwarf_Debug dbg,
			 Dwarf_Off loclist_offset,
			 int *loclist_count, Dwarf_Error * error)
{
    int count = 0;
    Dwarf_Off offset = loclist_offset;


    for (;;) {
	Dwarf_Block b;
	Dwarf_Addr lowpc;
	Dwarf_Addr highpc;
	int res = _dwarf_read_loc_section(dbg, &b,

					  &lowpc, &highpc,
					  offset, error);

	if (res != DW_DLV_OK) {
	    return res;
	}
	offset = b.bl_len + b.bl_section_offset;
	if (lowpc == 0 && highpc == 0) {
	    break;
	}
	count++;
    }
    *loclist_count = count;
    return DW_DLV_OK;
}

/* Helper routine to avoid code duplication. 
*/
static int
_dwarf_setup_loc(Dwarf_Attribute attr,
		 Dwarf_Debug * dbg_ret,
		 Dwarf_CU_Context *cucontext_ret,
		 Dwarf_Half * form_ret, Dwarf_Error * error)
{
    Dwarf_Debug dbg = 0;
    Dwarf_Half form = 0;
    int blkres;

    if (attr == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
	return (DW_DLV_ERROR);
    }
    if (attr->ar_cu_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
	return (DW_DLV_ERROR);
    }

    *cucontext_ret = attr->ar_cu_context;
    dbg = attr->ar_cu_context->cc_dbg;

    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    *dbg_ret = dbg;
    blkres = dwarf_whatform(attr, &form, error);
    if (blkres != DW_DLV_OK) {
	_dwarf_error(dbg, error, DW_DLE_LOC_EXPR_BAD);
	return blkres;
    }
    *form_ret = form;

    return DW_DLV_OK;
}

/* Helper routine  to avoid code duplication.
*/
static int
_dwarf_get_loclist_header_start(Dwarf_Debug dbg,
				Dwarf_Attribute attr,
				Dwarf_Unsigned * loclist_offset,
				Dwarf_Error * error)
{
    int secload = 0;
    int blkres = dwarf_formudata(attr, loclist_offset, error);

    if (blkres != DW_DLV_OK) {
	return (blkres);
    }

    if (!dbg->de_debug_loc) {
	secload = _dwarf_load_section(dbg,
				      dbg->de_debug_loc_index,
				      &dbg->de_debug_loc, error);
	if (secload != DW_DLV_OK) {
	    return secload;
	}
    }
    return DW_DLV_OK;
}

/* When llbuf (see dwarf_loclist_n) is partially set up
   and an error is encountered, tear it down as it
   won't be used.
*/
static void
_dwarf_cleanup_llbuf(Dwarf_Debug dbg, Dwarf_Locdesc ** llbuf, int count)
{
    int i;

    for (i = 0; i < count; ++i) {
	dwarf_dealloc(dbg, llbuf[i]->ld_s, DW_DLA_LOC_BLOCK);
	dwarf_dealloc(dbg, llbuf[i], DW_DLA_LOCDESC);
    }
    dwarf_dealloc(dbg, llbuf, DW_DLA_LIST);
}

/* 
	Handles simple location entries and loclists.
	Returns all the Locdesc's thru llbuf. 
	
*/
int
dwarf_loclist_n(Dwarf_Attribute attr,
                Dwarf_Locdesc *** llbuf_out,
                Dwarf_Signed * listlen_out, Dwarf_Error * error)
{
  Dwarf_Debug dbg;
  
  /*
   Dwarf_Attribute that describes the DW_AT_location in die, if
   present. */
  Dwarf_Attribute loc_attr = attr;
  
  /* Dwarf_Block that describes a single location expression. */
  Dwarf_Block loc_block;
  
  /* A pointer to the current Dwarf_Locdesc read. */
  Dwarf_Locdesc *locdesc = 0;
  
  Dwarf_Half form = 0;
  Dwarf_Addr lowpc = 0;
  Dwarf_Addr highpc = 0;
  Dwarf_Signed listlen = 0;
  Dwarf_Locdesc **llbuf = 0;
  Dwarf_CU_Context cucontext = 0;
  unsigned address_size = 0;
  
  int blkres;
  int setup_res;
  
  /* ***** BEGIN CODE ***** */
  setup_res = _dwarf_setup_loc(attr, &dbg, &cucontext, &form, error);
  if (setup_res != DW_DLV_OK) {
    return setup_res;
  }
  address_size = cucontext->cc_address_size;
  /* If this is a form_block then it's a location expression. If it's
   DW_FORM_data4 or DW_FORM_data8 it's a loclist offset */
  if (((cucontext->cc_version_stamp == CURRENT_VERSION_STAMP ||
        cucontext->cc_version_stamp == CURRENT_VERSION_STAMP3) &&
       (form == DW_FORM_data4 || form == DW_FORM_data8)) ||
      (cucontext->cc_version_stamp == CURRENT_VERSION_STAMP4 &&
       form == DW_FORM_sec_offset))
  {
    /* A reference to .debug_loc, with an offset in .debug_loc of a
	   loclist */
    Dwarf_Unsigned loclist_offset = 0;
    int off_res;
    int count_res;
    int loclist_count;
    int lli;
    
    off_res = _dwarf_get_loclist_header_start(dbg,
                                              attr, &loclist_offset,
                                              error);
    if (off_res != DW_DLV_OK) {
	    return off_res;
    }
    count_res = _dwarf_get_loclist_count(dbg, loclist_offset,
                                         &loclist_count, error);
    listlen = loclist_count;
    if (count_res != DW_DLV_OK) {
	    return count_res;
    }
    if (loclist_count == 0) {
	    return DW_DLV_NO_ENTRY;
    }
    
    llbuf = (Dwarf_Locdesc **)
    _dwarf_get_alloc(dbg, DW_DLA_LIST, loclist_count);
    if (!llbuf) {
	    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	    return (DW_DLV_ERROR);
    }
    
    for (lli = 0; lli < loclist_count; ++lli) {
	    blkres = _dwarf_read_loc_section(dbg, &loc_block,
                                       &lowpc,
                                       &highpc,
                                       loclist_offset, error);
	    if (blkres != DW_DLV_OK) {
        _dwarf_cleanup_llbuf(dbg, llbuf, lli);
        return (blkres);
	    }
	    locdesc = _dwarf_get_locdesc(dbg, &loc_block,
                                   lowpc, highpc, error);
	    if (locdesc == NULL) {
        _dwarf_cleanup_llbuf(dbg, llbuf, lli);
        /* low level error already set: let it be passed back */
        return (DW_DLV_ERROR);
	    }
	    llbuf[lli] = locdesc;
      
	    /* Now get to next loclist entry offset. */
	    loclist_offset = loc_block.bl_section_offset +
      loc_block.bl_len;
    }
    
    
  } else {
    Dwarf_Block *tblock = 0;
    
    blkres = dwarf_formblock(loc_attr, &tblock, error);
    if (blkres != DW_DLV_OK) {
	    return (blkres);
    }
    loc_block = *tblock;
    /* We copied tblock contents to the stack var, so can dealloc
	   tblock now.  Avoids leaks. */
    dwarf_dealloc(dbg, tblock, DW_DLA_BLOCK);
    listlen = 1;		/* One by definition of a location
                     entry. */
    lowpc = 0;		/* HACK */
    highpc = (Dwarf_Unsigned) (-1LL);	/* HACK */
    
    /* An empty location description (block length 0) means the
	   code generator emitted no variable, the variable was not
	   generated, it was unused or perhaps never tested after being
	   set. Dwarf2, section 2.4.1 In other words, it is not an
	   error, and we don't test for block length 0 specially here. */
    locdesc = _dwarf_get_locdesc(dbg, &loc_block,
                                 lowpc, highpc, error);
    if (locdesc == NULL) {
	    /* low level error already set: let it be passed back */
	    return (DW_DLV_ERROR);
    }
    llbuf = (Dwarf_Locdesc **)
    _dwarf_get_alloc(dbg, DW_DLA_LIST, listlen);
    if (!llbuf) {
	    /* Free the locdesc we allocated but won't use. */
	    dwarf_dealloc(dbg, locdesc, DW_DLA_LOCDESC);
	    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	    return (DW_DLV_ERROR);
    }
    llbuf[0] = locdesc;
  }
  
  *llbuf_out = llbuf;
  *listlen_out = listlen;
  return (DW_DLV_OK);
}

/* 
	Handles only a location expression.
	If called on a loclist, just returns one of those.
	Cannot not handle a real loclist. 
 	It returns the location expression as a loclist with
	a single entry.
	See dwarf_loclist_n() which handles any number
        of location list entries.

	This is the original definition, and it simply
	does not work for loclists. Kept for compatibility.
*/
int
dwarf_loclist(Dwarf_Attribute attr,
              Dwarf_Locdesc ** llbuf,
              Dwarf_Signed * listlen, Dwarf_Error * error)
{
  Dwarf_Debug dbg;
  
  /*
   Dwarf_Attribute that describes the DW_AT_location in die, if
   present. */
  Dwarf_Attribute loc_attr = attr;
  
  /* Dwarf_Block that describes a single location expression. */
  Dwarf_Block loc_block;
  
  /* A pointer to the current Dwarf_Locdesc read. */
  Dwarf_Locdesc *locdesc = 0;
  
  Dwarf_Half form = 0;
  Dwarf_Addr lowpc = 0;
  Dwarf_Addr highpc = 0;
  Dwarf_CU_Context cucontext = 0;
  unsigned address_size = 0;
  
  int blkres;
  int setup_res;
  
  /* ***** BEGIN CODE ***** */
  setup_res = _dwarf_setup_loc(attr, &dbg, &cucontext, &form, error);
  if (setup_res != DW_DLV_OK) {
    return setup_res;
  }
  address_size = cucontext->cc_address_size;
  /* If this is a form_block then it's a location expression. If it's
   DW_FORM_data4 or DW_FORM_data8 it's a loclist offset */
  if (((cucontext->cc_version_stamp == CURRENT_VERSION_STAMP ||
        cucontext->cc_version_stamp == CURRENT_VERSION_STAMP3) &&
       (form == DW_FORM_data4 || form == DW_FORM_data8)) ||
      (cucontext->cc_version_stamp == CURRENT_VERSION_STAMP4 &&
       form == DW_FORM_sec_offset))
  {
    
    /* A reference to .debug_loc, with an offset in .debug_loc of a
	   loclist */
    Dwarf_Unsigned loclist_offset = 0;
    int off_res;
    
    off_res = _dwarf_get_loclist_header_start(dbg,
                                              attr, &loclist_offset,
                                              error);
    if (off_res != DW_DLV_OK) {
	    return off_res;
    }
    
    /* With dwarf_loclist, just read a single entry */
    blkres = _dwarf_read_loc_section(dbg, &loc_block,
                                     &lowpc,
                                     &highpc,
                                     loclist_offset, error);
    if (blkres != DW_DLV_OK) {
	    return (blkres);
    }
    
    
    
    
  } else {
    Dwarf_Block *tblock = 0;
    
    blkres = dwarf_formblock(loc_attr, &tblock, error);
    if (blkres != DW_DLV_OK) {
	    return (blkres);
    }
    loc_block = *tblock;
    /* We copied tblock contents to the stack var, so can dealloc
	   tblock now.  Avoids leaks. */
    dwarf_dealloc(dbg, tblock, DW_DLA_BLOCK);
    lowpc = 0;		/* HACK */
    highpc = (Dwarf_Unsigned) (-1LL);	/* HACK */
  }
  
  /* An empty location description (block length 0) means the code
   generator emitted no variable, the variable was not generated,
   it was unused or perhaps never tested after being set. Dwarf2,
   section 2.4.1 In other words, it is not an error, and we don't
   test for block length 0 specially here. FIXME: doing this once
   is wrong, needs to handle low/hi pc sets. */
  locdesc = _dwarf_get_locdesc(dbg, &loc_block, lowpc, highpc, error);
  if (locdesc == NULL) {
    /* low level error already set: let it be passed back */
    return (DW_DLV_ERROR);
  }
  
  *llbuf = locdesc;
  *listlen = 1;
  return (DW_DLV_OK);
}

/* Usable to read a single loclist or to read a block of them
   or to read an entire section's loclists.

*/

 /*ARGSUSED*/ int
dwarf_get_loclist_entry(Dwarf_Debug dbg,
			Dwarf_Unsigned offset,
			Dwarf_Addr * hipc_offset,
			Dwarf_Addr * lopc_offset,
			Dwarf_Ptr * data,
			Dwarf_Unsigned * entry_len,
			Dwarf_Unsigned * next_entry,
			Dwarf_Error * error)
{
    Dwarf_Block b;
    Dwarf_Addr lowpc;
    Dwarf_Addr highpc;
    int res;

    if (!dbg->de_debug_loc) {
	int secload = _dwarf_load_section(dbg,
					  dbg->de_debug_loc_index,
					  &dbg->de_debug_loc,
					  error);

	if (secload != DW_DLV_OK) {
	    return secload;
	}
    }

    res = _dwarf_read_loc_section(dbg,
				  &b, &lowpc, &highpc, offset, error);
    if (res != DW_DLV_OK) {
	return res;
    }
    *hipc_offset = highpc;
    *lopc_offset = lowpc;
    *entry_len = b.bl_len;
    *data = b.bl_data;
    *next_entry = b.bl_len + b.bl_section_offset;

    return DW_DLV_OK;



}
