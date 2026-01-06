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
#include "dwarf_line.h"
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

static int
is_path_separator(Dwarf_Small s)
{
  if(s == '/') {
    return 1;
  }
#ifdef HAVE_WINDOWS_PATH
  if(s == '\\') {
    return 1;
  }
#endif
  return 0;
}

/* Return 0 if false, 1 if true.
 If HAVE_WINDOWS_PATH is defined we
 attempt to handle windows full paths:
 \\something   or  C:cwdpath.c
 */
static int
file_name_is_full_path(Dwarf_Small  *fname)
{
  Dwarf_Small firstc = *fname;
  if(is_path_separator(firstc)) {
    /* Full path. */
    return 1;
  }
  if(!firstc) {
    return 0;
  }
#ifdef HAVE_WINDOWS_PATH
  if((firstc >= 'A' && firstc <= 'Z') ||
     (firstc >= 'a' && firstc <= 'z')) {
    Dwarf_Small secondc = fname[1];
    if (secondc == ':') {
      return 1;
    }
  }
#endif
  return 0;
}

/*
    Although source files is supposed to return the
    source files in the compilation-unit, it does
    not look for any in the statement program.  In
    other words, it ignores those defined using the
    extended opcode DW_LNE_define_file.
*/
int
dwarf_srcfiles(Dwarf_Die die,
	       char ***srcfiles,
	       Dwarf_Signed * srcfilecount, Dwarf_Error * error)
{
  /* This pointer is used to scan the portion of the .debug_line
   section for the current cu. */
  Dwarf_Small *line_ptr;
  
  /* Pointer to a DW_AT_stmt_list attribute in case it exists in the
   die. */
  Dwarf_Attribute stmt_list_attr;
  
  /* Pointer to DW_AT_comp_dir attribute in die. */
  Dwarf_Attribute comp_dir_attr;
  
  /* Pointer to name of compilation directory. */
  Dwarf_Small *comp_dir = 0;
  
  /* Offset into .debug_line specified by a DW_AT_stmt_list
   attribute. */
  Dwarf_Unsigned line_offset = 0;
  
  /* This points to a block of char *'s, each of which points to a
   file name. */
  char **ret_files = 0;
  
  /* The Dwarf_Debug this die belongs to. */
  Dwarf_Debug dbg = 0;

  /* Used to chain the file names. */
  Dwarf_Chain curr_chain = NULL;
  Dwarf_Chain prev_chain = NULL;
  Dwarf_Chain head_chain = NULL;
  Dwarf_Half attrform = 0;
  int resattr = DW_DLV_ERROR;
  int lres = DW_DLV_ERROR;
  struct Line_Table_Prefix_s line_prefix;
  int i = 0;
  int res = DW_DLV_ERROR;

    /* ***** BEGIN CODE ***** */

  /* ***** BEGIN CODE ***** */
  /* Reset error. */
  if (error != NULL)
    *error = NULL;

  CHECK_DIE(die, DW_DLV_ERROR);
  dbg = die->di_cu_context->cc_dbg;

  resattr = dwarf_attr(die, DW_AT_stmt_list, &stmt_list_attr, error);
  if (resattr != DW_DLV_OK) {
    return resattr;
  }

  if (dbg->de_debug_line_index== 0) {
    _dwarf_error(dbg, error, DW_DLE_DEBUG_LINE_NULL);
    return (DW_DLV_ERROR);
  }

  res = _dwarf_load_section(dbg, dbg->de_debug_line_index, &dbg->de_debug_line, error);
  if (res != DW_DLV_OK) {
    return res;
  }

  lres = dwarf_whatform(stmt_list_attr, &attrform, error);
  if (lres != DW_DLV_OK) {
    return lres;
  }
  if (attrform != DW_FORM_data4
      && attrform != DW_FORM_data8
      && attrform != DW_FORM_sec_offset )
  {
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
    int cres = DW_DLV_ERROR;
    char *cdir = 0;
    
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

  dwarf_init_line_table_prefix(&line_prefix);

  {
    Dwarf_Small *line_ptr_out = 0;
    int dres = dwarf_read_line_table_prefix(dbg,
      line_ptr,
      dbg->de_debug_line_size,
      &line_ptr_out,
      &line_prefix, error);

    if (dres == DW_DLV_ERROR)
      return dres;
    if (dres == DW_DLV_NO_ENTRY) {
      dwarf_free_line_table_prefix(&line_prefix);
      return dres;
    }

    line_ptr = line_ptr_out;
  }

  for (i = 0; i < line_prefix.pf_files_count; ++i) {
    struct Line_Table_File_Entry_s *fe =
    line_prefix.pf_line_table_file_entries + i;
    char *file_name = (char *) fe->lte_filename;
    char *dir_name = 0;
    char *full_name = 0;
    Dwarf_Unsigned dir_index = fe->lte_directory_index;
    
    if (dir_index == 0) {
      dir_name = (char *) comp_dir;
    } else {
      dir_name =
      (char *) line_prefix.pf_include_directories[
                                                  fe->lte_directory_index - 1];
    }

    /* dir_name can be NULL if there is no DW_AT_comp_dir */
    if(dir_name == 0 || file_name_is_full_path((Dwarf_Small *)file_name)) {
      /* This is safe because dwarf_dealloc is careful to not
       dealloc strings which are part of the raw .debug_* data.
       */
      full_name = file_name;
    } else {
      full_name = (char *) _dwarf_get_alloc(dbg, DW_DLA_STRING,
                                            strlen(dir_name) + 1 +
                                            strlen(file_name) +
                                            1);
      if (full_name == NULL) {
        dwarf_free_line_table_prefix(&line_prefix);
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return (DW_DLV_ERROR);
      }
      
      /* This is not careful to avoid // in the output, Nothing
       forces a 'canonical' name format here. Unclear if this
       needs to be fixed. */
      strcpy(full_name, dir_name);
      strcat(full_name, "/");
      strcat(full_name, file_name);
    }

    curr_chain =
    (Dwarf_Chain) _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
    if (curr_chain == NULL) {
      dwarf_free_line_table_prefix(&line_prefix);
      _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
      return (DW_DLV_ERROR);
    }
    curr_chain->ch_item = full_name;
    if (head_chain == NULL)
      head_chain = prev_chain = curr_chain;
    else {
      prev_chain->ch_next = curr_chain;
      prev_chain = curr_chain;
    }
  }

  curr_chain = (Dwarf_Chain) _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
  if (curr_chain == NULL) {
    dwarf_free_line_table_prefix(&line_prefix);
    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
    return (DW_DLV_ERROR);
  }

  if (line_prefix.pf_files_count == 0) {
    *srcfiles = NULL;
    *srcfilecount = 0;
    dwarf_free_line_table_prefix(&line_prefix);
    return (DW_DLV_NO_ENTRY);
  }
  
  ret_files = (char **)
  _dwarf_get_alloc(dbg, DW_DLA_LIST, line_prefix.pf_files_count);
  if (ret_files == NULL) {
    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
    dwarf_free_line_table_prefix(&line_prefix);
    return (DW_DLV_ERROR);
  }
  
  curr_chain = head_chain;
  for (i = 0; i < line_prefix.pf_files_count; i++) {
    *(ret_files + i) = curr_chain->ch_item;
    prev_chain = curr_chain;
    curr_chain = curr_chain->ch_next;
    dwarf_dealloc(dbg, prev_chain, DW_DLA_CHAIN);
  }
  
  *srcfiles = ret_files;
  *srcfilecount = line_prefix.pf_files_count;
  dwarf_free_line_table_prefix(&line_prefix);
  return (DW_DLV_OK);
}


/*
	return DW_DLV_OK if ok. else DW_DLV_NO_ENTRY or DW_DLV_ERROR
*/
int
_dwarf_internal_srclines(Dwarf_Die die,
			 Dwarf_Line ** linebuf,
			 Dwarf_Signed * count,
			 Dwarf_Bool doaddrs,
			 Dwarf_Bool dolines, Dwarf_Error * error)
{
    /* 
       This pointer is used to scan the portion of the .debug_line
       section for the current cu. */
    Dwarf_Small *line_ptr;

    /* 
       This points to the last byte of the .debug_line portion for the
       current cu. */
    Dwarf_Small *line_ptr_end;


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

    Dwarf_File_Entry file_entries;

    /* These are the state machine state variables. */
    Dwarf_Addr address = 0;
    Dwarf_Word file = 1;
    Dwarf_Word line = 1;
    Dwarf_Word column = 0;

    /* phony init. See below for true initialization. */
    Dwarf_Bool is_stmt = false;

    Dwarf_Bool basic_block = false;
    Dwarf_Bool prologue_end = false;
    Dwarf_Bool epilogue_begin = false;
    Dwarf_Small isa = 0;
    Dwarf_Bool end_sequence = false;

    /* 
       These pointers are used to build the list of files names by this 
       cu.  cur_file_entry points to the file name being added, and
       prev_file_entry to the previous one. */
    Dwarf_File_Entry cur_file_entry, prev_file_entry;

    Dwarf_Sword i = 0;
    Dwarf_Sword file_entry_count = 0;

    /* 
       This is the current opcode read from the statement program. */
    Dwarf_Small opcode;

    /* 
       Pointer to a Dwarf_Line_Context_s structure that contains the
       context such as file names and include directories for the set
       of lines being generated. */
    Dwarf_Line_Context line_context;

    /* 
       This is a pointer to the current line being added to the line
       matrix. */
    Dwarf_Line curr_line;

    /* 
       These variables are used to decode leb128 numbers. Leb128_num
       holds the decoded number, and leb128_length is its length in
       bytes. */
    Dwarf_Word leb128_num;
    Dwarf_Word leb128_length;
    Dwarf_Sword advance_line;

    /* 
       This is the operand of the latest fixed_advance_pc extended
       opcode. */
    Dwarf_Half fixed_advance_pc;

    /* 
       Counts the number of lines in the line matrix. */
    Dwarf_Sword line_count = 0;

    /* This is the length of an extended opcode instr.  */
    Dwarf_Word instr_length;
    Dwarf_Small ext_opcode;
    struct Line_Table_Prefix_s prefix;

    /* 
       Used to chain together pointers to line table entries that are
       later used to create a block of Dwarf_Line entries. */
    Dwarf_Chain chain_line, head_chain = NULL, curr_chain;

    /* 
       This points to a block of Dwarf_Lines, a pointer to which is
       returned in linebuf. */
    Dwarf_Line *block_line;

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



    lres = dwarf_formudata(stmt_list_attr, &line_offset, error);
    if (lres != DW_DLV_OK) {
	return lres;
    }

    if (line_offset >= dbg->de_debug_line_size) {
	_dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
	return (DW_DLV_ERROR);
    }
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
	Dwarf_Small *newlinep = 0;
	res = dwarf_read_line_table_prefix(dbg,
					       line_ptr,
					       dbg->de_debug_line_size,
					       &newlinep,
					       &prefix,
					       error);

	if (res == DW_DLV_ERROR) {
	    dwarf_free_line_table_prefix(&prefix);
	    return res;
	}
	if (res == DW_DLV_NO_ENTRY) {
	    dwarf_free_line_table_prefix(&prefix);
	    return res;
	}
	line_ptr_end = prefix.pf_line_ptr_end;
	line_ptr = newlinep;
    }


    /* Set up context structure for this set of lines. */
    line_context = (Dwarf_Line_Context)
	_dwarf_get_alloc(dbg, DW_DLA_LINE_CONTEXT, 1);
    if (line_context == NULL) {
	dwarf_free_line_table_prefix(&prefix);
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    /* Fill out a Dwarf_File_Entry list as we use that to implement the 
       define_file operation. */
    file_entries = prev_file_entry = NULL;
    for (i = 0; i < prefix.pf_files_count; ++i) {
	struct Line_Table_File_Entry_s *pfxfile =
	    prefix.pf_line_table_file_entries + i;

	cur_file_entry = (Dwarf_File_Entry)
	    _dwarf_get_alloc(dbg, DW_DLA_FILE_ENTRY, 1);
	if (cur_file_entry == NULL) {
	    dwarf_free_line_table_prefix(&prefix);
	    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	    return (DW_DLV_ERROR);
	}

	cur_file_entry->fi_file_name = pfxfile->lte_filename;
	cur_file_entry->fi_dir_index = pfxfile->lte_directory_index;
	cur_file_entry->fi_time_last_mod =
	    pfxfile->lte_last_modification_time;

	cur_file_entry->fi_file_length = pfxfile->lte_length_of_file;

	if (file_entries == NULL)
	    file_entries = cur_file_entry;
	else
	    prev_file_entry->fi_next = cur_file_entry;
	prev_file_entry = cur_file_entry;

	file_entry_count++;
    }


    /* Initialize the one state machine variable that depends on the
       prefix.  */
    is_stmt = prefix.pf_default_is_stmt;


    /* Start of statement program.  */
    while (line_ptr < line_ptr_end) {
	int type;

	opcode = *(Dwarf_Small *) line_ptr;
	line_ptr++;


	/* 'type' is the output */
	WHAT_IS_OPCODE(type, opcode, prefix.pf_opcode_base,
		       prefix.pf_opcode_length_table, line_ptr,
		       prefix.pf_std_op_count);

	if (type == LOP_DISCARD) {
	    int oc;
	    int opcnt = prefix.pf_opcode_length_table[opcode];

	    for (oc = 0; oc < opcnt; oc++) {
		/* 
		 ** Read and discard operands we don't 
		 ** understand.                        
		 ** arbitrary choice of unsigned read. 
		 ** signed read would work as well.    
		 */
		Dwarf_Unsigned utmp2;

		DECODE_LEB128_UWORD(line_ptr, utmp2)
	    }
	} else if (type == LOP_SPECIAL) {
	    /* This op code is a special op in the object, no matter
	       that it might fall into the standard op range in this
	       compile. That is, these are special opcodes between
	       opcode_base and MAX_LINE_OP_CODE.  (including
	       opcode_base and MAX_LINE_OP_CODE) */

	    opcode = opcode - prefix.pf_opcode_base;
	    address = address + prefix.pf_minimum_instruction_length *
		(opcode / prefix.pf_line_range);
	    line =
		line + prefix.pf_line_base +
		opcode % prefix.pf_line_range;

	    if (dolines) {
		curr_line =
		    (Dwarf_Line) _dwarf_get_alloc(dbg, DW_DLA_LINE, 1);
		if (curr_line == NULL) {
		    dwarf_free_line_table_prefix(&prefix);
		    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
		    return (DW_DLV_ERROR);
		}

		curr_line->li_address = address;
		curr_line->li_addr_line.li_l_data.li_file =
		    (Dwarf_Sword) file;
		curr_line->li_addr_line.li_l_data.li_line =
		    (Dwarf_Sword) line;
		curr_line->li_addr_line.li_l_data.li_column =
		    (Dwarf_Half) column;
		curr_line->li_addr_line.li_l_data.li_is_stmt = is_stmt;
		curr_line->li_addr_line.li_l_data.li_basic_block =
		    basic_block;
		curr_line->li_addr_line.li_l_data.li_end_sequence =
		    curr_line->li_addr_line.li_l_data.
		    li_epilogue_begin = epilogue_begin;
		curr_line->li_addr_line.li_l_data.li_prologue_end =
		    prologue_end;
		curr_line->li_addr_line.li_l_data.li_isa = isa;
		curr_line->li_context = line_context;
		line_count++;

		chain_line = (Dwarf_Chain)
		    _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
		if (chain_line == NULL) {
		    dwarf_free_line_table_prefix(&prefix);
		    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
		    return (DW_DLV_ERROR);
		}
		chain_line->ch_item = curr_line;

		if (head_chain == NULL)
		    head_chain = curr_chain = chain_line;
		else {
		    curr_chain->ch_next = chain_line;
		    curr_chain = chain_line;
		}
	    }

	    basic_block = false;
	} else if (type == LOP_STANDARD) {
	    switch (opcode) {

	    case DW_LNS_copy:{
		    if (dolines) {

			curr_line =
			    (Dwarf_Line) _dwarf_get_alloc(dbg,
							  DW_DLA_LINE,
							  1);
			if (curr_line == NULL) {
			    dwarf_free_line_table_prefix(&prefix);
			    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
			    return (DW_DLV_ERROR);
			}

			curr_line->li_address = address;
			curr_line->li_addr_line.li_l_data.li_file =
			    (Dwarf_Sword) file;
			curr_line->li_addr_line.li_l_data.li_line =
			    (Dwarf_Sword) line;
			curr_line->li_addr_line.li_l_data.li_column =
			    (Dwarf_Half) column;
			curr_line->li_addr_line.li_l_data.li_is_stmt =
			    is_stmt;
			curr_line->li_addr_line.li_l_data.
			    li_basic_block = basic_block;
			curr_line->li_addr_line.li_l_data.
			    li_end_sequence = end_sequence;
			curr_line->li_context = line_context;
			curr_line->li_addr_line.li_l_data.
			    li_epilogue_begin = epilogue_begin;
			curr_line->li_addr_line.li_l_data.
			    li_prologue_end = prologue_end;
			curr_line->li_addr_line.li_l_data.li_isa = isa;
			line_count++;

			chain_line = (Dwarf_Chain)
			    _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
			if (chain_line == NULL) {
			    dwarf_free_line_table_prefix(&prefix);
			    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
			    return (DW_DLV_ERROR);
			}
			chain_line->ch_item = curr_line;
			if (head_chain == NULL)
			    head_chain = curr_chain = chain_line;
			else {
			    curr_chain->ch_next = chain_line;
			    curr_chain = chain_line;
			}
		    }

		    basic_block = false;
		    prologue_end = false;
		    epilogue_begin = false;
		    break;
		}

	    case DW_LNS_advance_pc:{
		    Dwarf_Unsigned utmp2;

		    DECODE_LEB128_UWORD(line_ptr, utmp2)
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
		    line = line + advance_line;
		    break;
		}

	    case DW_LNS_set_file:{
		    Dwarf_Unsigned utmp2;

		    DECODE_LEB128_UWORD(line_ptr, utmp2)
			file = (Dwarf_Word) utmp2;
		    break;
		}

	    case DW_LNS_set_column:{
		    Dwarf_Unsigned utmp2;

		    DECODE_LEB128_UWORD(line_ptr, utmp2)
			column = (Dwarf_Word) utmp2;
		    break;
		}

	    case DW_LNS_negate_stmt:{

		    is_stmt = !is_stmt;
		    break;
		}

	    case DW_LNS_set_basic_block:{

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

		    break;
		}

	    case DW_LNS_fixed_advance_pc:{

		    READ_UNALIGNED(dbg, fixed_advance_pc, Dwarf_Half,
				   line_ptr, sizeof(Dwarf_Half));
		    line_ptr += sizeof(Dwarf_Half);
		    address = address + fixed_advance_pc;
		    break;
		}

		/* New in DWARF3 */
	    case DW_LNS_set_prologue_end:{

		    prologue_end = true;
		    break;


		}
		/* New in DWARF3 */
	    case DW_LNS_set_epilogue_begin:{
		    epilogue_begin = true;
		    break;
		}

		/* New in DWARF3 */
	    case DW_LNS_set_isa:{
		    Dwarf_Unsigned utmp2;

		    DECODE_LEB128_UWORD(line_ptr, utmp2)
			isa = utmp2;
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
	    Dwarf_Unsigned utmp3;

	    DECODE_LEB128_UWORD(line_ptr, utmp3)
		instr_length = (Dwarf_Word) utmp3;
	    /* Dwarf_Small is a ubyte and the extended opcode is a
	       ubyte, though not stated as clearly in the 2.0.0 spec as 
	       one might hope. */
	    ext_opcode = *(Dwarf_Small *) line_ptr;
	    line_ptr++;
	    switch (ext_opcode) {

	    case DW_LNE_end_sequence:{
		    end_sequence = true;

		    if (dolines) {
			curr_line = (Dwarf_Line)
			    _dwarf_get_alloc(dbg, DW_DLA_LINE, 1);
			if (curr_line == NULL) {
			    dwarf_free_line_table_prefix(&prefix);
			    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
			    return (DW_DLV_ERROR);
			}

			curr_line->li_address = address;
			curr_line->li_addr_line.li_l_data.li_file =
			    (Dwarf_Sword) file;
			curr_line->li_addr_line.li_l_data.li_line =
			    (Dwarf_Sword) line;
			curr_line->li_addr_line.li_l_data.li_column =
			    (Dwarf_Half) column;
			curr_line->li_addr_line.li_l_data.li_is_stmt =
			    prefix.pf_default_is_stmt;
			curr_line->li_addr_line.li_l_data.
			    li_basic_block = basic_block;
			curr_line->li_addr_line.li_l_data.
			    li_end_sequence = end_sequence;
			curr_line->li_context = line_context;
			curr_line->li_addr_line.li_l_data.
			    li_epilogue_begin = epilogue_begin;
			curr_line->li_addr_line.li_l_data.
			    li_prologue_end = prologue_end;
			curr_line->li_addr_line.li_l_data.li_isa = isa;
			line_count++;

			chain_line = (Dwarf_Chain)
			    _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
			if (chain_line == NULL) {
			    dwarf_free_line_table_prefix(&prefix);
			    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
			    return (DW_DLV_ERROR);
			}
			chain_line->ch_item = curr_line;

			if (head_chain == NULL)
			    head_chain = curr_chain = chain_line;
			else {
			    curr_chain->ch_next = chain_line;
			    curr_chain = chain_line;
			}
		    }

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
			if (doaddrs) {
			    curr_line =
				(Dwarf_Line) _dwarf_get_alloc(dbg,
							      DW_DLA_LINE,
							      1);
			    if (curr_line == NULL) {
				dwarf_free_line_table_prefix(&prefix);
				_dwarf_error(dbg, error,
					     DW_DLE_ALLOC_FAIL);
				return (DW_DLV_ERROR);
			    }

			    curr_line->li_address = address;
			    curr_line->li_addr_line.li_offset =
				line_ptr - dbg->de_debug_line;

			    line_count++;

			    chain_line = (Dwarf_Chain)
				_dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
			    if (chain_line == NULL) {
				dwarf_free_line_table_prefix(&prefix);
				_dwarf_error(dbg, error,
					     DW_DLE_ALLOC_FAIL);
				return (DW_DLV_ERROR);
			    }
			    chain_line->ch_item = curr_line;

			    if (head_chain == NULL)
				head_chain = curr_chain = chain_line;
			    else {
				curr_chain->ch_next = chain_line;
				curr_chain = chain_line;
			    }
			}

			line_ptr += dbg->de_pointer_size;
		    } else {
			dwarf_free_line_table_prefix(&prefix);
			_dwarf_error(dbg, error,
				     DW_DLE_LINE_SET_ADDR_ERROR);
			return (DW_DLV_ERROR);
		    }

		    break;
		}

	    case DW_LNE_define_file:{

		    if (dolines) {
			cur_file_entry = (Dwarf_File_Entry)
			    _dwarf_get_alloc(dbg, DW_DLA_FILE_ENTRY, 1);
			if (cur_file_entry == NULL) {
			    dwarf_free_line_table_prefix(&prefix);
			    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
			    return (DW_DLV_ERROR);
			}

			cur_file_entry->fi_file_name =
			    (Dwarf_Small *) line_ptr;
			line_ptr =
			    line_ptr + strlen((char *) line_ptr) + 1;

			cur_file_entry->fi_dir_index = (Dwarf_Sword)
			    _dwarf_decode_u_leb128(line_ptr,
						   &leb128_length);
			line_ptr = line_ptr + leb128_length;

			cur_file_entry->fi_time_last_mod =
			    _dwarf_decode_u_leb128(line_ptr,
						   &leb128_length);
			line_ptr = line_ptr + leb128_length;

			cur_file_entry->fi_file_length =
			    _dwarf_decode_u_leb128(line_ptr,
						   &leb128_length);
			line_ptr = line_ptr + leb128_length;

			if (file_entries == NULL)
			    file_entries = cur_file_entry;
			else
			    prev_file_entry->fi_next = cur_file_entry;
			prev_file_entry = cur_file_entry;

			file_entry_count++;
		    }
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

    block_line = (Dwarf_Line *)
	_dwarf_get_alloc(dbg, DW_DLA_LIST, line_count);
    if (block_line == NULL) {
	dwarf_free_line_table_prefix(&prefix);
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    curr_chain = head_chain;
    for (i = 0; i < line_count; i++) {
	*(block_line + i) = curr_chain->ch_item;
	head_chain = curr_chain;
	curr_chain = curr_chain->ch_next;
	dwarf_dealloc(dbg, head_chain, DW_DLA_CHAIN);
    }

    line_context->lc_file_entries = file_entries;
    line_context->lc_file_entry_count = file_entry_count;
    line_context->lc_include_directories_count =
	prefix.pf_include_directories_count;
    if (prefix.pf_include_directories_count > 0) {
	/* This gets a pointer to the *first* include dir. The others
	   follow directly with the standard DWARF2/3 NUL byte
	   following the last. */
	line_context->lc_include_directories =
	    prefix.pf_include_directories[0];
    }

    line_context->lc_line_count = line_count;
    line_context->lc_compilation_directory = comp_dir;
    line_context->lc_version_number = prefix.pf_version;
    line_context->lc_dbg = dbg;
    *count = line_count;

    *linebuf = block_line;
    dwarf_free_line_table_prefix(&prefix);
    return (DW_DLV_OK);
}

int
dwarf_srclines(Dwarf_Die die,
	       Dwarf_Line ** linebuf,
	       Dwarf_Signed * linecount, Dwarf_Error * error)
{
    Dwarf_Signed count;
    int res;

    res = _dwarf_internal_srclines(die, linebuf, &count,	/* addrlist= 
								 */ false,
				   /* linelist= */ true, error);
    if (res != DW_DLV_OK) {
	return res;
    }
    *linecount = count;
    return res;
}



/* Every line table entry (except DW_DLE_end_sequence,
   which is returned using dwarf_lineendsequence())
   potentially has the begin-statement
   flag marked 'on'.   This returns thru *return_bool,
   the begin-statement flag.
*/

int
dwarf_linebeginstatement(Dwarf_Line line,
			 Dwarf_Bool * return_bool, Dwarf_Error * error)
{
    if (line == NULL || return_bool == 0) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
	return (DW_DLV_ERROR);
    }

    *return_bool = (line->li_addr_line.li_l_data.li_is_stmt);
    return DW_DLV_OK;
}

/* At the end of any contiguous line-table there may be
   a DW_LNE_end_sequence operator.
   This returns non-zero thru *return_bool
   if and only if this 'line' entry was a DW_LNE_end_sequence.

   Within a compilation unit or function there may be multiple
   line tables, each ending with a DW_LNE_end_sequence.
   Each table describes a contiguous region.
   Because compilers may split function code up in arbitrary ways
   compilers may need to emit multiple contigous regions (ie
   line tables) for a single function.
   See the DWARF3 spec section 6.2.
*/
int
dwarf_lineendsequence(Dwarf_Line line,
		      Dwarf_Bool * return_bool, Dwarf_Error * error)
{
    if (line == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
	return (DW_DLV_ERROR);
    }

    *return_bool = (line->li_addr_line.li_l_data.li_end_sequence);
    return DW_DLV_OK;
}


/* Each 'line' entry has a line-number.
   If the entry is a DW_LNE_end_sequence the line-number is
   meaningless (see dwarf_lineendsequence(), just above).
*/
int
dwarf_lineno(Dwarf_Line line,
	     Dwarf_Unsigned * ret_lineno, Dwarf_Error * error)
{
    if (line == NULL || ret_lineno == 0) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
	return (DW_DLV_ERROR);
    }

    *ret_lineno = (line->li_addr_line.li_l_data.li_line);
    return DW_DLV_OK;
}

/* Each 'line' entry has a file-number, and index into the file table.
   If the entry is a DW_LNE_end_sequence the index is
   meaningless (see dwarf_lineendsequence(), just above).
   The file number returned is an index into the file table
   produced by dwarf_srcfiles(), but care is required: the
   li_file begins with 1 for real files, so that the li_file returned here
   is 1 greater than its index into the dwarf_srcfiles() output array.
   And entries from DW_LNE_define_file don't appear in
   the dwarf_srcfiles() output so file indexes from here may exceed
   the size of the dwarf_srcfiles() output array size.
*/
int
dwarf_line_srcfileno(Dwarf_Line line,
		     Dwarf_Unsigned * ret_fileno, Dwarf_Error * error)
{
    if (line == NULL || ret_fileno == 0) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
	return (DW_DLV_ERROR);
    }
    /* li_file must be <= line->li_context->lc_file_entry_count else it 
       is trash. li_file 0 means not attributable to any source file
       per dwarf2/3 spec. */

    *ret_fileno = (line->li_addr_line.li_l_data.li_file);
    return DW_DLV_OK;
}


/* Each 'line' entry has a line-address.
   If the entry is a DW_LNE_end_sequence the adddress
   is one-beyond the last address this contigous region
   covers, so the address is not inside the region, 
   but is just outside it.
*/
int
dwarf_lineaddr(Dwarf_Line line,
	       Dwarf_Addr * ret_lineaddr, Dwarf_Error * error)
{
    if (line == NULL || ret_lineaddr == 0) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
	return (DW_DLV_ERROR);
    }

    *ret_lineaddr = (line->li_address);
    return DW_DLV_OK;
}


/* Each 'line' entry has a column-within-line (offset
   within the line) where the
   source text begins.
   If the entry is a DW_LNE_end_sequence the line-number is
   meaningless (see dwarf_lineendsequence(), just above).
   Lines of text begin at column 1.  The value 0
   means the line begins at the left edge of the line.
   (See the DWARF3 spec, section 6.2.2).
*/
int
dwarf_lineoff(Dwarf_Line line,
	      Dwarf_Signed * ret_lineoff, Dwarf_Error * error)
{
    if (line == NULL || ret_lineoff == 0) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
	return (DW_DLV_ERROR);
    }

    *ret_lineoff =
	(line->li_addr_line.li_l_data.li_column ==
	 0 ? -1 : line->li_addr_line.li_l_data.li_column);
    return DW_DLV_OK;
}


int
dwarf_linesrc(Dwarf_Line line, char **ret_linesrc, Dwarf_Error * error)
{
    Dwarf_Signed i;
    Dwarf_File_Entry file_entry;
    Dwarf_Small *name_buffer;
    Dwarf_Small *include_directories;
    Dwarf_Debug dbg;
    unsigned int comp_dir_len;

    if (line == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
	return (DW_DLV_ERROR);
    }

    if (line->li_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_LINE_CONTEXT_NULL);
	return (DW_DLV_ERROR);
    }
    dbg = line->li_context->lc_dbg;

    if (line->li_addr_line.li_l_data.li_file >
	line->li_context->lc_file_entry_count) {
	_dwarf_error(dbg, error, DW_DLE_LINE_FILE_NUM_BAD);
	return (DW_DLV_ERROR);
    }

    if (line->li_addr_line.li_l_data.li_file == 0) {
	/* No file name known: see dwarf2/3 spec. */
	_dwarf_error(dbg, error, DW_DLE_NO_FILE_NAME);
	return (DW_DLV_ERROR);
    }
    file_entry = line->li_context->lc_file_entries;
    /* ASSERT: li_file > 0, dwarf correctness issue, see line table
       definition of dwarf2/3 spec. */
    /* Example: if li_file is 2 and lc_file_entry_count is 3,
       file_entry is file 3 (1 based), aka 2( 0 based) file_entry->next 
       is file 2 (1 based), aka 1( 0 based) file_entry->next->next is
       file 1 (1 based), aka 0( 0 based) file_entry->next->next->next
       is NULL.

       and this loop finds the file_entry we need (2 (1 based) in this
       case). Because lc_file_entries are in reverse order and
       effectively zero based as a count whereas li_file is 1 based. */
    for (i = line->li_addr_line.li_l_data.li_file - 1; i > 0; i--)
	file_entry = file_entry->fi_next;

    if (file_entry->fi_file_name == NULL) {
	_dwarf_error(dbg, error, DW_DLE_NO_FILE_NAME);
	return (DW_DLV_ERROR);
    }

    if (*(char *) file_entry->fi_file_name == '/') {
	*ret_linesrc = ((char *) file_entry->fi_file_name);
	return DW_DLV_OK;
    }

    if (file_entry->fi_dir_index == 0) {

	/* dir_index of 0 means that the compilation was in the
	   'current directory of compilation' */
	if (line->li_context->lc_compilation_directory == NULL) {
	    /* we don't actually *have* a current directory of
	       compilation: DW_AT_comp_dir was not present Rather than
	       emitting DW_DLE_NO_COMP_DIR lets just make an empty name 
	       here. In other words, do the best we can with what we do 
	       have instead of reporting an error. _dwarf_error(dbg,
	       error, DW_DLE_NO_COMP_DIR); return(DW_DLV_ERROR); */
	    comp_dir_len = 0;
	} else {
	    comp_dir_len = strlen((char *)
				  (line->li_context->
				   lc_compilation_directory));
	}

	name_buffer =
	    _dwarf_get_alloc(line->li_context->lc_dbg, DW_DLA_STRING,
			     comp_dir_len + 1 +
			     strlen((char *) file_entry->fi_file_name) +
			     1);
	if (name_buffer == NULL) {
	    _dwarf_error(line->li_context->lc_dbg, error,
			 DW_DLE_ALLOC_FAIL);
	    return (DW_DLV_ERROR);
	}

	if (comp_dir_len > 0) {
	    /* if comp_dir_len is 0 we do not want to put a / in front
	       of the fi_file_name as we just don't know anything. */
	    strcpy((char *) name_buffer,
		   (char *) (line->li_context->
			     lc_compilation_directory));
	    strcat((char *) name_buffer, "/");
	}
	strcat((char *) name_buffer, (char *) file_entry->fi_file_name);
	*ret_linesrc = ((char *) name_buffer);
	return DW_DLV_OK;
    }

    if (file_entry->fi_dir_index >
	line->li_context->lc_include_directories_count) {
	_dwarf_error(dbg, error, DW_DLE_INCL_DIR_NUM_BAD);
	return (DW_DLV_ERROR);
    }

    include_directories = line->li_context->lc_include_directories;
    for (i = file_entry->fi_dir_index - 1; i > 0; i--)
	include_directories += strlen((char *) include_directories) + 1;

    if (line->li_context->lc_compilation_directory) {
	comp_dir_len = strlen((char *)
			      (line->li_context->
			       lc_compilation_directory));
    } else {
	/* No DW_AT_comp_dir present. Do the best we can without it. */
	comp_dir_len = 0;
    }

    name_buffer = _dwarf_get_alloc(dbg, DW_DLA_STRING,
				   (*include_directories == '/' ?
				    0 : comp_dir_len + 1) +
				   strlen((char *) include_directories)
				   + 1 +
				   strlen((char *) file_entry->
					  fi_file_name) + 1);
    if (name_buffer == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }

    if (*include_directories != '/') {
	if (comp_dir_len > 0) {
	    strcpy((char *) name_buffer,
		   (char *) line->li_context->lc_compilation_directory);
	    /* Who provides the / needed after the compilation
	       directory? */
	    if (name_buffer[comp_dir_len - 1] != '/') {
		/* Here we provide the / separator */
		name_buffer[comp_dir_len] = '/';	/* overwrite
							   previous nul 
							   terminator
							   with needed
							   / */
		name_buffer[comp_dir_len + 1] = 0;
	    }
	}
    } else {
	strcpy((char *) name_buffer, "");
    }
    strcat((char *) name_buffer, (char *) include_directories);
    strcat((char *) name_buffer, "/");
    strcat((char *) name_buffer, (char *) file_entry->fi_file_name);
    *ret_linesrc = ((char *) name_buffer);
    return DW_DLV_OK;
}

/* Every line table entry potentially has the basic-block-start
   flag marked 'on'.   This returns thru *return_bool,
   the basic-block-start flag.
*/
int
dwarf_lineblock(Dwarf_Line line,
		Dwarf_Bool * return_bool, Dwarf_Error * error)
{
    if (line == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
	return (DW_DLV_ERROR);
    }

    *return_bool = (line->li_addr_line.li_l_data.li_basic_block);
    return DW_DLV_OK;
}


#if 0				/* Ignore this.  This needs major
				   re-work. */
/* 
    This routine works by looking for exact matches between 
    the current line address and pc, and crossovers from
    from less than pc value to greater than.  At each line
    that satisfies the above, it records a pointer to the
    line, and the difference between the address and pc.
    It then scans these pointers and picks out those with
    the smallest difference between pc and address.         
*/
int
dwarf_pclines(Dwarf_Debug dbg,
	      Dwarf_Addr pc,
	      Dwarf_Line ** linebuf,
	      Dwarf_Signed slide,
	      Dwarf_Signed * linecount, Dwarf_Error * error)
{
    /* 
       Scans the line matrix for the current cu to which a pointer
       exists in dbg. */
    Dwarf_Line line;
    Dwarf_Line prev_line;

    /* 
       These flags are for efficiency reasons. Check_line is true
       initially, but set false when the address of the current line is 
       greater than pc.  It is set true only when the address of the
       current line falls below pc.  This assumes that addresses within 
       the same segment increase, and we are only interested in the
       switch from a less than pc address to a greater than. First_line 
       is set true initially, but set false after the first line is
       scanned.  This is to prevent looking at the address of previous
       line when slide is DW_DLS_BACKWARD, and the first line is being
       scanned. */
    Dwarf_Bool check_line, first_line;

    /* 
       Diff tracks the smallest difference a line address and the input 
       pc value. */
    Dwarf_Signed diff, i;

    /* 
       For the slide = DW_DLS_BACKWARD case, pc_less is the value of
       the address of the line immediately preceding the first line
       that has value greater than pc. For the slide = DW_DLS_FORWARD
       case, pc_more is the values of address for the first line that
       is greater than pc. Diff is the difference between either of the 
       these values and pc. */
    Dwarf_Addr pc_less, pc_more;

    /* 
       Pc_line_buf points to a chain of pointers to lines of which
       those with a diff equal to the smallest difference will be
       returned. */
    Dwarf_Line *pc_line_buf, *pc_line;

    /* 
       Chain_count counts the number of lines in the above chain for
       which the diff is equal to the smallest difference This is the
       number returned by this routine. */
    Dwarf_Signed chain_count;

    chain_head = NULL;

    check_line = true;
    first_line = true;
    diff = MAX_LINE_DIFF;

    for (i = 0; i < dbg->de_cu_line_count; i++) {

	line = *(dbg->de_cu_line_ptr + i);
	prev_line = first_line ? NULL : *(dbg->de_cu_line_ptr + i - 1);

	if (line->li_address == pc) {
	    chain_ptr = (struct chain *)
		_dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
	    if (chain_ptr == NULL) {
		_dwarf_error(NULL, error, DW_DLE_ALLOC_FAIL);
		return (DW_DLV_ERROR);
	    }

	    chain_ptr->line = line;
	    chain_ptr->diff = diff = 0;
	    chain_ptr->next = chain_head;
	    chain_head = chain_ptr;
	} else
	    /* 
	       Look for crossover from less than pc address to greater
	       than. */
	if (check_line && line->li_address > pc &&
		(first_line ? 0 : prev_line->li_address) < pc)

	    if (slide == DW_DLS_BACKWARD && !first_line) {
		pc_less = prev_line->li_address;
		if (pc - pc_less <= diff) {
		    chain_ptr = (struct chain *)
			_dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
		    if (chain_ptr == NULL) {
			_dwarf_error(NULL, error, DW_DLE_ALLOC_FAIL);
			return (DW_DLV_ERROR);
		    }

		    chain_ptr->line = prev_line;
		    chain_ptr->diff = diff = pc - pc_less;
		    chain_ptr->next = chain_head;
		    chain_head = chain_ptr;
		}
		check_line = false;
	    } else if (slide == DW_DLS_FORWARD) {
		pc_more = line->li_address;
		if (pc_more - pc <= diff) {
		    chain_ptr = (struct chain *)
			_dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
		    if (chain_ptr == NULL) {
			_dwarf_error(NULL, error, DW_DLE_ALLOC_FAIL);
			return (DW_DLV_ERROR);
		    }

		    chain_ptr->line = line;
		    chain_ptr->diff = diff = pc_more - pc;
		    chain_ptr->next = chain_head;
		    chain_head = chain_ptr;
		}
		check_line = false;
	    } else
		/* Check addresses only when they go */
		/* below pc.  */
	    if (line->li_address < pc)
		check_line = true;

	first_line = false;
    }

    chain_count = 0;
    for (chain_ptr = chain_head; chain_ptr != NULL;
	 chain_ptr = chain_ptr->next)
	if (chain_ptr->diff == diff)
	    chain_count++;

    pc_line_buf = pc_line = (Dwarf_Line)
	_dwarf_get_alloc(dbg, DW_DLA_LIST, chain_count);
    for (chain_ptr = chain_head; chain_ptr != NULL;
	 chain_ptr = chain_ptr->next)
	if (chain_ptr->diff == diff) {
	    *pc_line = chain_ptr->line;
	    pc_line++;
	}

    for (chain_ptr = chain_head; chain_ptr != NULL;) {
	chain_head = chain_ptr;
	chain_ptr = chain_ptr->next;
	dwarf_dealloc(dbg, chain_head, DW_DLA_CHAIN);
    }

    *linebuf = pc_line_buf;
    return (chain_count);
}
#endif



/*
   It's impossible for callers of dwarf_srclines() to get to and
   free all the resources (in particular, the li_context and its
   lc_file_entries). 
   So this function, new July 2005, does it.  
*/

void
dwarf_srclines_dealloc(Dwarf_Debug dbg, Dwarf_Line * linebuf,
		       Dwarf_Signed count)
{

    Dwarf_Signed i = 0;
    struct Dwarf_Line_Context_s *context = 0;

    if (count > 0) {
	/* All these entries share a single context */
	context = linebuf[0]->li_context;
    }
    for (i = 0; i < count; ++i) {
	dwarf_dealloc(dbg, linebuf[i], DW_DLA_LINE);
    }
    dwarf_dealloc(dbg, linebuf, DW_DLA_LIST);

    if (context) {
	Dwarf_File_Entry fe = context->lc_file_entries;

	while (fe) {
	    Dwarf_File_Entry fenext = fe->fi_next;

	    dwarf_dealloc(dbg, fe, DW_DLA_FILE_ENTRY);
	    fe = fenext;
	}
	dwarf_dealloc(dbg, context, DW_DLA_LINE_CONTEXT);
    }

    return;
}

/* Operand counts per standard operand.
   The initial zero is for DW_LNS_copy. 
   This is an economical way to verify we understand the table
   of standard-opcode-lengths in the line table prologue.  */
#define STANDARD_OPERAND_COUNT_DWARF2 9
#define STANDARD_OPERAND_COUNT_DWARF3 12
static unsigned char
  dwarf_standard_opcode_operand_count[STANDARD_OPERAND_COUNT_DWARF3] = {
    /* DWARF2 */
    0,
    1, 1, 1, 1,
    0, 0, 0,
    1,
    /* Following are new for DWARF3. */
    0, 0, 1
};

/* Common line table prefix reading code. 
   Returns DW_DLV_OK, DW_DLV_ERROR.
   DW_DLV_NO_ENTRY cannot be returned, but callers should
   assume it is possible.

   The prefix_out area must be initialized properly before calling this.

   Has the side effect of allocating arrays which
   must be freed (see the Line_Table_Prefix_s struct which
   holds the pointers to space we allocate here).
*/
int
dwarf_read_line_table_prefix(Dwarf_Debug dbg,
			     Dwarf_Small * data_start,
			     Dwarf_Unsigned data_length,
			     Dwarf_Small ** updated_data_start_out,
			     struct Line_Table_Prefix_s *prefix_out,
			     Dwarf_Error * err)
{
    Dwarf_Small *line_ptr = data_start;
    Dwarf_Unsigned total_length = 0;
    int local_length_size = 0;
    int local_extension_size = 0;
    Dwarf_Unsigned prologue_length = 0;
    Dwarf_Half version = 0;
    Dwarf_Unsigned directories_count = 0;
    Dwarf_Unsigned directories_malloc = 0;
    Dwarf_Unsigned files_count = 0;
    Dwarf_Unsigned files_malloc = 0;
    Dwarf_Small *line_ptr_end = 0;

    prefix_out->pf_line_ptr_start = line_ptr;
    /* READ_AREA_LENGTH updates line_ptr for consumed bytes */
    READ_AREA_LENGTH(dbg, total_length, Dwarf_Unsigned,
		     line_ptr, local_length_size, local_extension_size);


    line_ptr_end = line_ptr + total_length;
    prefix_out->pf_line_ptr_end = line_ptr_end;
    prefix_out->pf_length_field_length = local_length_size +
	local_extension_size;
    /* ASSERT: prefix_out->pf_length_field_length == line_ptr
       -prefix_out->pf_line_ptr_start; */
    if (line_ptr_end > dbg->de_debug_line + dbg->de_debug_line_size) {
	_dwarf_error(dbg, err, DW_DLE_DEBUG_LINE_LENGTH_BAD);
	return (DW_DLV_ERROR);
    }
    if (line_ptr_end > data_start + data_length) {
	_dwarf_error(dbg, err, DW_DLE_DEBUG_LINE_LENGTH_BAD);
	return (DW_DLV_ERROR);
    }
    prefix_out->pf_total_length = total_length;

    READ_UNALIGNED(dbg, version, Dwarf_Half,
		   line_ptr, sizeof(Dwarf_Half));
    prefix_out->pf_version = version;
    line_ptr += sizeof(Dwarf_Half);
    if (version != CURRENT_VERSION_STAMP &&
	version != CURRENT_VERSION_STAMP3 &&
	version != CURRENT_VERSION_STAMP4) {
	_dwarf_error(dbg, err, DW_DLE_VERSION_STAMP_ERROR);
	return (DW_DLV_ERROR);
    }

    READ_UNALIGNED(dbg, prologue_length, Dwarf_Unsigned,
		   line_ptr, local_length_size);
    prefix_out->pf_prologue_length = prologue_length;
    line_ptr += local_length_size;
    prefix_out->pf_line_prologue_start = line_ptr;


    prefix_out->pf_minimum_instruction_length =
	*(unsigned char *) line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Small);

    prefix_out->pf_default_is_stmt = *(unsigned char *) line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Small);

    prefix_out->pf_line_base = *(signed char *) line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Sbyte);

    prefix_out->pf_line_range = *(unsigned char *) line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Small);

    prefix_out->pf_opcode_base = *(unsigned char *) line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Small);

    /* Set up the array of standard opcode lengths. */
    /* We think this works ok even for cross-endian processing of
       objects.  It might be wrong, we might need to specially process
       the array of ubyte into host order.  */
    prefix_out->pf_opcode_length_table = line_ptr;

    /* pf_opcode_base is one greater than the size of the array. */
    line_ptr += prefix_out->pf_opcode_base - 1;

    {
	/* Determine (as best we can) whether the
	   pf_opcode_length_table holds 9 or 12 standard-conforming
	   entries.  gcc4 upped to DWARF3's 12 without updating the
	   version number.  */
	int operand_ck_fail = true;

	if (prefix_out->pf_opcode_base >= STANDARD_OPERAND_COUNT_DWARF3) {
	    int mismatch = memcmp(dwarf_standard_opcode_operand_count,
				  prefix_out->pf_opcode_length_table,
				  STANDARD_OPERAND_COUNT_DWARF3);

	    if (!mismatch) {
		operand_ck_fail = false;
		prefix_out->pf_std_op_count =
		    STANDARD_OPERAND_COUNT_DWARF3;
	    }

	}
	if (operand_ck_fail) {
	    if (prefix_out->pf_opcode_base >=
		STANDARD_OPERAND_COUNT_DWARF2) {

		int mismatch =
		    memcmp(dwarf_standard_opcode_operand_count,
			   prefix_out->pf_opcode_length_table,
			   STANDARD_OPERAND_COUNT_DWARF2);

		if (!mismatch) {
		    operand_ck_fail = false;
		    prefix_out->pf_std_op_count =
			STANDARD_OPERAND_COUNT_DWARF2;
		}
	    }
	}
	if (operand_ck_fail) {
	    _dwarf_error(dbg, err, DW_DLE_LINE_NUM_OPERANDS_BAD);
	    return (DW_DLV_ERROR);
	}
    }
    /* At this point we no longer need to check operand counts. */


    directories_count = 0;
    directories_malloc = 5;
    prefix_out->pf_include_directories = malloc(sizeof(Dwarf_Small *) *
						directories_malloc);
    if (prefix_out->pf_include_directories == NULL) {
	_dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }
    memset(prefix_out->pf_include_directories, 0,
	   sizeof(Dwarf_Small *) * directories_malloc);

    while ((*(char *) line_ptr) != '\0') {
	if (directories_count >= directories_malloc) {
	    Dwarf_Unsigned expand = 2 * directories_malloc;
	    Dwarf_Unsigned bytesalloc = sizeof(Dwarf_Small *) * expand;
	    Dwarf_Small **newdirs =
		realloc(prefix_out->pf_include_directories,
			bytesalloc);

	    if (!newdirs) {
		_dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
		return (DW_DLV_ERROR);
	    }
	    /* Doubled size, zero out second half. */
	    memset(newdirs + directories_malloc, 0,
		   sizeof(Dwarf_Small *) * directories_malloc);
	    directories_malloc = expand;
	    prefix_out->pf_include_directories = newdirs;
	}
	prefix_out->pf_include_directories[directories_count] =
	    line_ptr;
	line_ptr = line_ptr + strlen((char *) line_ptr) + 1;
	directories_count++;
    }
    prefix_out->pf_include_directories_count = directories_count;
    line_ptr++;

    files_count = 0;
    files_malloc = 5;
    prefix_out->pf_line_table_file_entries =
	malloc(sizeof(struct Line_Table_File_Entry_s) * files_malloc);
    if (prefix_out->pf_line_table_file_entries == NULL) {
	_dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
	return (DW_DLV_ERROR);
    }
    memset(prefix_out->pf_line_table_file_entries, 0,
	   sizeof(struct Line_Table_File_Entry_s) * files_malloc);

    while (*(char *) line_ptr != '\0') {
	Dwarf_Unsigned utmp;
	Dwarf_Unsigned dir_index = 0;
	Dwarf_Unsigned lastmod = 0;
	Dwarf_Unsigned file_length = 0;
	struct Line_Table_File_Entry_s *curline;
	Dwarf_Word leb128_length = 0;


	if (files_count >= files_malloc) {
	    Dwarf_Unsigned expand = 2 * files_malloc;
	    struct Line_Table_File_Entry_s *newfiles =
		realloc(prefix_out->pf_line_table_file_entries,
			sizeof(struct Line_Table_File_Entry_s) *
			expand);
	    if (!newfiles) {
		_dwarf_error(dbg, err, DW_DLE_ALLOC_FAIL);
		return (DW_DLV_ERROR);
	    }
	    memset(newfiles + files_malloc, 0,
		   sizeof(struct Line_Table_File_Entry_s) *
		   files_malloc);
	    files_malloc = expand;
	    prefix_out->pf_line_table_file_entries = newfiles;
	}
	curline = prefix_out->pf_line_table_file_entries + files_count;

	curline->lte_filename = line_ptr;
	line_ptr = line_ptr + strlen((char *) line_ptr) + 1;

	DECODE_LEB128_UWORD(line_ptr, utmp)
	    dir_index = (Dwarf_Sword) utmp;
	if (dir_index > directories_count) {
	    _dwarf_error(dbg, err, DW_DLE_DIR_INDEX_BAD);
	    return (DW_DLV_ERROR);
	}
	curline->lte_directory_index = dir_index;

	lastmod = _dwarf_decode_u_leb128(line_ptr, &leb128_length);
	line_ptr = line_ptr + leb128_length;
	curline->lte_last_modification_time = lastmod;

	/* Skip over file length. */
	file_length = _dwarf_decode_u_leb128(line_ptr, &leb128_length);
	line_ptr = line_ptr + leb128_length;
	curline->lte_length_of_file = file_length;

	++files_count;

    }
    prefix_out->pf_files_count = files_count;
    /* Skip trailing nul byte */
    ++line_ptr;


    if (line_ptr != (prefix_out->pf_line_prologue_start +
		     prefix_out->pf_prologue_length)) {
	_dwarf_error(dbg, err, DW_DLE_LINE_PROLOG_LENGTH_BAD);
	return (DW_DLV_ERROR);
    }


    *updated_data_start_out = line_ptr;
    return DW_DLV_OK;
}


/* Initialize the Line_Table_Prefix_s struct. 
   memset is not guaranteed a portable initializer, but works
   fine for current architectures.   AFAIK.
*/
void
dwarf_init_line_table_prefix(struct Line_Table_Prefix_s *pf)
{
    memset(pf, 0, sizeof(*pf));
}

/* Free any malloc'd area.  of the Line_Table_Prefix_s struct. */
void
dwarf_free_line_table_prefix(struct Line_Table_Prefix_s *pf)
{
    if (pf->pf_include_directories) {
	free(pf->pf_include_directories);
	pf->pf_include_directories = 0;
    }
    if (pf->pf_line_table_file_entries) {
	free(pf->pf_line_table_file_entries);
	pf->pf_line_table_file_entries = 0;
    }
    return;
}
