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
#include <time.h>
#include "dwarf_line.h"
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

/* FIXME Need to add prologue_end epilogue_begin isa fields. */
static void
print_line_header(void)
{
    printf
	("                                                         s b e\n"
	 "                                                         t l s\n"
	 "                                                         m c e\n"
	 " section    op                                       col t k q\n"
	 " offset     code               address     file line umn ? ? ?\n");
}

/* FIXME: print new line values:   prologue_end epilogue_begin isa */
static void
print_line_detail(char *prefix,
		  int opcode,
		  unsigned long long address,
		  unsigned long file,
		  unsigned long line,
		  unsigned long column,
		  int is_stmt, int basic_block, int end_sequence,
		  int prologue_end, int epilogue_begin, int isa)
{
#pragma unused(prologue_end, epilogue_begin, isa)
    printf("%-15s %2d 0x%08llx "
	   "%2lu   %4lu %2lu   %1d %1d %1d\n",
	   prefix,
	   (int) opcode,
	   (long long) address,
	   (unsigned long) file,
	   (unsigned long) line,
	   (unsigned long) column,
	   (int) is_stmt, (int) basic_block, (int) end_sequence);

}


/*
	return DW_DLV_OK if ok. else DW_DLV_NO_ENTRY or DW_DLV_ERROR
*/
static int
_dwarf_internal_printlines(Dwarf_Die die, Dwarf_Error * error)
{
    /* 
       This pointer is used to scan the portion of the .debug_line
       section for the current cu. */
    Dwarf_Small *line_ptr;
    Dwarf_Small *orig_line_ptr;

    /* 
       This points to the last byte of the .debug_line portion for the
       current cu. */
    Dwarf_Small *line_ptr_end = 0;

    /* 
       Pointer to a DW_AT_stmt_list attribute in case it exists in the
       die. */
    Dwarf_Attribute stmt_list_attr;

    /* Pointer to DW_AT_comp_dir attribute in die. */
    Dwarf_Attribute comp_dir_attr;

    /* Pointer to name of compilation directory. */
    Dwarf_Small *comp_dir = NULL;

    /* 
       Offset into .debug_line specified by a DW_AT_stmt_list
       attribute. */
    Dwarf_Unsigned line_offset;

    struct Line_Table_Prefix_s prefix;


    /* These are the state machine state variables. */
    Dwarf_Addr address = 0;
    Dwarf_Word file = 1;
    Dwarf_Word line = 1;
    Dwarf_Word column = 0;
    Dwarf_Bool is_stmt = false;
    Dwarf_Bool basic_block = false;
    Dwarf_Bool end_sequence = false;
    Dwarf_Bool prologue_end = false;
    Dwarf_Bool epilogue_begin = false;
    Dwarf_Small isa = 0;


    Dwarf_Sword i;

    /* 
       This is the current opcode read from the statement program. */
    Dwarf_Small opcode;


    /* 
       These variables are used to decode leb128 numbers. Leb128_num
       holds the decoded number, and leb128_length is its length in
       bytes. */
    Dwarf_Word leb128_num;
    Dwarf_Word leb128_length;
    Dwarf_Sword advance_line;
    Dwarf_Half attrform = 0;


    /* 
       This is the operand of the latest fixed_advance_pc extended
       opcode. */
    Dwarf_Half fixed_advance_pc;


    /* The Dwarf_Debug this die belongs to. */
    Dwarf_Debug dbg;
    int resattr;
    int lres;

    int res;

    /* ***** BEGIN CODE ***** */

    if (error != NULL)
	*error = NULL;

    CHECK_DIE(die, DW_DLV_ERROR)
	dbg = die->di_cu_context->cc_dbg;

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_line_index,
			    &dbg->de_debug_line, error);
    if (res != DW_DLV_OK) {
	return res;
    }

    resattr = dwarf_attr(die, DW_AT_stmt_list, &stmt_list_attr, error);
    if (resattr != DW_DLV_OK) {
	return resattr;
    }



  /* The list of relevant FORMs is small.
   DW_FORM_data4, DW_FORM_data8, DW_FORM_sec_offset
   */
  lres = dwarf_whatform(stmt_list_attr,&attrform,error);
  if (lres != DW_DLV_OK) {
    return lres;
  }
  if (attrform != DW_FORM_data4 && attrform != DW_FORM_data8 &&
      attrform != DW_FORM_sec_offset ) {
    _dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
    return (DW_DLV_ERROR);
  }
  lres = dwarf_global_formref(stmt_list_attr, &line_offset, error);
  if (lres != DW_DLV_OK) {
    return lres;
  }

    if (line_offset >= dbg->de_debug_line_size) {
	_dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
	return (DW_DLV_ERROR);
    }
    orig_line_ptr = dbg->de_debug_line;
    line_ptr = dbg->de_debug_line + line_offset;
    dwarf_dealloc(dbg, stmt_list_attr, DW_DLA_ATTR);

    /* 
       If die has DW_AT_comp_dir attribute, get the string that names
       the compilation directory. */
    resattr = dwarf_attr(die, DW_AT_comp_dir, &comp_dir_attr, error);
    if (resattr == DW_DLV_ERROR) {
	return resattr;
    }
    if (resattr == DW_DLV_OK) {
	int cres;
	char *cdir;

	cres = dwarf_formstring(comp_dir_attr, &cdir, error);
	if (cres == DW_DLV_ERROR) {
	    return cres;
	} else if (cres == DW_DLV_OK) {
	    comp_dir = (Dwarf_Small *) cdir;
	}
    }
    if (resattr == DW_DLV_OK) {
	dwarf_dealloc(dbg, comp_dir_attr, DW_DLA_ATTR);
    }

    dwarf_init_line_table_prefix(&prefix);
    {
	Dwarf_Small *line_ptr_out = 0;
	int dres = dwarf_read_line_table_prefix(dbg,
						line_ptr,
						dbg->
						de_debug_line_size -
						line_offset,
						&line_ptr_out,
						&prefix, error);

	if (dres == DW_DLV_ERROR) {
	    dwarf_free_line_table_prefix(&prefix);
	    return dres;
	}
	if (dres == DW_DLV_NO_ENTRY) {
	    dwarf_free_line_table_prefix(&prefix);
	    return dres;
	}
	line_ptr_end = prefix.pf_line_ptr_end;
	line_ptr = line_ptr_out;
    }



    printf("total line info length %ld bytes, "
	   "line offset 0x%llx %lld\n",
	   (long) prefix.pf_total_length,
	   (long long) line_offset, (long long) line_offset);
    printf("compilation_directory %s\n",
	   comp_dir ? ((char *) comp_dir) : "");

    printf("  min instruction length %d\n",
	   (int) prefix.pf_minimum_instruction_length);
    printf("  default is stmt        %d\n", (int)
	   prefix.pf_default_is_stmt);
    printf("  line base              %d\n", (int)
	   prefix.pf_line_base);
    printf("  line_range             %d\n", (int)
	   prefix.pf_line_range);


    for (i = 1; i < prefix.pf_opcode_base; i++) {
	printf("  opcode[%d] length %d\n", (int) i,
	       (int) prefix.pf_opcode_length_table[i - 1]);
    }

    for (i = 0; i < prefix.pf_include_directories_count; ++i) {
	printf("  include dir[%d] %s\n",
	       (int) i, prefix.pf_include_directories[i]);
    }


    for (i = 0; i < prefix.pf_files_count; ++i) {
	struct Line_Table_File_Entry_s *lfile =
	    prefix.pf_line_table_file_entries + i;

	Dwarf_Unsigned tlm2 = lfile->lte_last_modification_time;
	Dwarf_Unsigned di = lfile->lte_directory_index;
	Dwarf_Unsigned fl = lfile->lte_length_of_file;

	printf("  file[%d]  %s\n",
	       (int) i, (char *) lfile->lte_filename);

	printf("    dir index %d\n", (int) di);
	{
	    time_t tt = (time_t) tlm2;

	    printf("    last time 0x%x %s",	/* ctime supplies
						   newline */
		   (unsigned) tlm2, ctime(&tt));
	}
	printf("    file length %ld 0x%lx\n",
	       (long) fl, (unsigned long) fl);


    }


    {
	unsigned long long offset = line_ptr - orig_line_ptr;

	printf("  statement prog offset in section: %llu 0x%llx\n",
	       offset, offset);
    }

    /* Initialize the part of the state machine dependent on the
       prefix.  */
    is_stmt = prefix.pf_default_is_stmt;


    print_line_header();
    /* Start of statement program.  */
    while (line_ptr < line_ptr_end) {
	int type;

	printf(" [0x%06llx] ", (long long) (line_ptr - orig_line_ptr));
	opcode = *(Dwarf_Small *) line_ptr;
	line_ptr++;
	/* 'type' is the output */
	WHAT_IS_OPCODE(type, opcode, prefix.pf_opcode_base,
		       prefix.pf_opcode_length_table, line_ptr,
		       prefix.pf_std_op_count);

	if (type == LOP_DISCARD) {
	    int oc;
	    int opcnt = prefix.pf_opcode_length_table[opcode];

	    printf(" DISCARD standard opcode %d with %d operands: "
		   "not understood:", opcode, opcnt);
	    for (oc = 0; oc < opcnt; oc++) {
		/* 
		 * Read and discard operands we don't
		 * understand.
		 * Arbitrary choice of unsigned read.
		 * Signed read would work as well.
		 */
		Dwarf_Unsigned utmp2;

		DECODE_LEB128_UWORD(line_ptr, utmp2)
		    printf(" %llu (0x%llx)",
			   (unsigned long long) utmp2,
			   (unsigned long long) utmp2);
	    }

	    printf("\n");
	    /* do nothing, necessary ops done */
	} else if (type == LOP_SPECIAL) {
	    /* This op code is a special op in the object, no matter
	       that it might fall into the standard op range in this
	       compile Thatis, these are special opcodes between
	       special_opcode_base and MAX_LINE_OP_CODE.  (including
	       special_opcode_base and MAX_LINE_OP_CODE) */
	    char special[50];
	    unsigned origop = opcode;

	    opcode = opcode - prefix.pf_opcode_base;
	    address = address + prefix.pf_minimum_instruction_length *
		(opcode / prefix.pf_line_range);
	    line =
		line + prefix.pf_line_base +
		opcode % prefix.pf_line_range;

	    sprintf(special, "Specialop %3u", origop);
	    print_line_detail(special,
			      opcode, address, (int) file, line, column,
			      is_stmt, basic_block, end_sequence,
			      prologue_end, epilogue_begin, isa);

	    basic_block = false;

	} else if (type == LOP_STANDARD) {
	    switch (opcode) {

	    case DW_LNS_copy:{

		    print_line_detail("DW_LNS_copy",
				      opcode, address, file, line,
				      column, is_stmt, basic_block,
				      end_sequence, prologue_end,
				      epilogue_begin, isa);

		    basic_block = false;
		    break;
		}

	    case DW_LNS_advance_pc:{
		    Dwarf_Unsigned utmp2;


		    DECODE_LEB128_UWORD(line_ptr, utmp2)
			printf("DW_LNS_advance_pc val %lld 0x%llx\n",
			       (long long) (Dwarf_Word) utmp2,
			       (long long) (Dwarf_Word) utmp2);
		    leb128_num = (Dwarf_Word) utmp2;
		    address =
			address +
			prefix.pf_minimum_instruction_length *
			leb128_num;
		    break;
		}

	    case DW_LNS_advance_line:{
		    Dwarf_Signed stmp;


		    DECODE_LEB128_SWORD(line_ptr, stmp)
			advance_line = (Dwarf_Sword) stmp;
		    printf("DW_LNS_advance_line val %lld 0x%llx\n",
			   (long long) advance_line,
			   (long long) advance_line);
		    line = line + advance_line;
		    break;
		}

	    case DW_LNS_set_file:{
		    Dwarf_Unsigned utmp2;


		    DECODE_LEB128_UWORD(line_ptr, utmp2)
			file = (Dwarf_Word) utmp2;
		    printf("DW_LNS_set_file  %ld\n", (long) file);
		    break;
		}

	    case DW_LNS_set_column:{
		    Dwarf_Unsigned utmp2;


		    DECODE_LEB128_UWORD(line_ptr, utmp2)
			column = (Dwarf_Word) utmp2;
		    printf("DW_LNS_set_column val %lld 0x%llx\n",
			   (long long) column, (long long) column);
		    break;
		}

	    case DW_LNS_negate_stmt:{
		    is_stmt = !is_stmt;
		    printf("DW_LNS_negate_stmt\n");
		    break;
		}

	    case DW_LNS_set_basic_block:{

		    printf("DW_LNS_set_basic_block\n");
		    basic_block = true;
		    break;
		}

	    case DW_LNS_const_add_pc:{
		    opcode = MAX_LINE_OP_CODE - prefix.pf_opcode_base;
		    address =
			address +
			prefix.pf_minimum_instruction_length * (opcode /
								prefix.
								pf_line_range);

		    printf("DW_LNS_const_add_pc new address 0x%llx\n",
			   (long long) address);
		    break;
		}

	    case DW_LNS_fixed_advance_pc:{

		    READ_UNALIGNED(dbg, fixed_advance_pc, Dwarf_Half,
				   line_ptr, sizeof(Dwarf_Half));
		    line_ptr += sizeof(Dwarf_Half);
		    address = address + fixed_advance_pc;
		    printf("DW_LNS_fixed_advance_pc val %lld 0x%llx"
			   " new address 0x%llx\n",
			   (long long) fixed_advance_pc,
			   (long long) fixed_advance_pc,
			   (long long) address);
		    break;
		}
	    case DW_LNS_set_prologue_end:{

		    prologue_end = true;
		    printf("DW_LNS_set_prologue_end set true.\n");
		    break;


		}
		/* New in DWARF3 */
	    case DW_LNS_set_epilogue_begin:{
		    epilogue_begin = true;
		    printf("DW_LNS_set_epilogue_begin set true.\n");
		    break;
		}

		/* New in DWARF3 */
	    case DW_LNS_set_isa:{
		    Dwarf_Unsigned utmp2;

		    DECODE_LEB128_UWORD(line_ptr, utmp2)
			isa = utmp2;
		    printf("DW_LNS_set_isa new value 0x%llx.\n",
			   (unsigned long long) utmp2);
		    if (isa != utmp2) {
			/* The value of the isa did not fit in our
			   local so we record it wrong. declare an
			   error. */
			dwarf_free_line_table_prefix(&prefix);

			_dwarf_error(dbg, error,
				     DW_DLE_LINE_NUM_OPERANDS_BAD);
			return (DW_DLV_ERROR);
		    }
		    break;
		}
	    }


	} else if (type == LOP_EXTENDED) {
	    Dwarf_Unsigned utmp3 = 0;
	    Dwarf_Word instr_length = 0;
	    Dwarf_Small ext_opcode = 0;

	    DECODE_LEB128_UWORD(line_ptr, utmp3)
		instr_length = (Dwarf_Word) utmp3;
	    ext_opcode = *(Dwarf_Small *) line_ptr;
	    line_ptr++;
	    switch (ext_opcode) {

	    case DW_LNE_end_sequence:{
		    end_sequence = true;

		    print_line_detail("DW_LNE_end_sequence extended",
				      opcode, address, file, line,
				      column, is_stmt, basic_block,
				      end_sequence, prologue_end,
				      epilogue_begin, isa);

		    address = 0;
		    file = 1;
		    line = 1;
		    column = 0;
		    is_stmt = prefix.pf_default_is_stmt;
		    basic_block = false;
		    end_sequence = false;
		    prologue_end = false;
		    epilogue_begin = false;


		    break;
		}

	    case DW_LNE_set_address:{
		    if (instr_length - 1 == dbg->de_pointer_size) {
			READ_UNALIGNED(dbg, address, Dwarf_Addr,
				       line_ptr, dbg->de_pointer_size);

			line_ptr += dbg->de_pointer_size;
			printf("DW_LNE_set_address address 0x%llx\n",
			       (long long) address);
		    } else {
			dwarf_free_line_table_prefix(&prefix);
			_dwarf_error(dbg, error,
				     DW_DLE_LINE_SET_ADDR_ERROR);
			return (DW_DLV_ERROR);
		    }

		    break;
		}

	    case DW_LNE_define_file:{


		    Dwarf_Small *fn;
		    Dwarf_Unsigned di;
		    Dwarf_Unsigned tlm;
		    Dwarf_Unsigned fl;

		    fn = (Dwarf_Small *) line_ptr;
		    line_ptr = line_ptr + strlen((char *) line_ptr) + 1;

		    di = _dwarf_decode_u_leb128(line_ptr,
						&leb128_length);
		    line_ptr = line_ptr + leb128_length;

		    tlm = _dwarf_decode_u_leb128(line_ptr,
						 &leb128_length);
		    line_ptr = line_ptr + leb128_length;

		    fl = _dwarf_decode_u_leb128(line_ptr,
						&leb128_length);
		    line_ptr = line_ptr + leb128_length;


		    printf("DW_LNE_define_file %s \n", fn);
		    printf("    dir index %d\n", (int) di);
		    {
			time_t tt3 = (time_t) tlm;

			/* ctime supplies newline */
			printf("    last time 0x%x %s",
			       (unsigned) tlm, ctime(&tt3));
		    }
		    printf("    file length %ld 0x%lx\n",
			   (long) fl, (unsigned long) fl);

		    break;
		}

	    default:{
		    dwarf_free_line_table_prefix(&prefix);
		    _dwarf_error(dbg, error,
				 DW_DLE_LINE_EXT_OPCODE_BAD);
		    return (DW_DLV_ERROR);
		}
	    }

	}
    }

    dwarf_free_line_table_prefix(&prefix);
    return (DW_DLV_OK);
}

/*
	Caller passes in compilation unit DIE.
*/
int
_dwarf_print_lines(Dwarf_Die die, Dwarf_Error * error)
{
    int res;

    res = _dwarf_internal_printlines(die, error);
    if (res != DW_DLV_OK) {
	return res;
    }
    return res;
}
