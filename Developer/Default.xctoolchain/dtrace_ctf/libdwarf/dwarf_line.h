/*

  Copyright (C) 2000, 2004, 2006 Silicon Graphics, Inc.  All Rights Reserved.

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



#define DW_EXTENDED_OPCODE	0

/*
    This is used as the starting value for an algorithm
    to get the minimum difference between 2 values.
    UINT_MAX is used as our approximation to infinity.
*/
#define MAX_LINE_DIFF       UINT_MAX


/*
    This structure is used to build a list of all the
    files that are used in the current compilation unit.
    All of the fields execpt fi_next have meanings that
    are obvious from section 6.2.4 of the Libdwarf Doc.
*/
struct Dwarf_File_Entry_s {
    /* Points to string naming the file. */
    Dwarf_Small *fi_file_name;

    /* 
       Index into the list of directories of the directory in which
       this file exits. */
    Dwarf_Sword fi_dir_index;

    /* Time of last modification of the file. */
    Dwarf_Unsigned fi_time_last_mod;

    /* Length in bytes of the file. */
    Dwarf_Unsigned fi_file_length;

    /* Pointer for chaining file entries. */
    Dwarf_File_Entry fi_next;
};


typedef struct Dwarf_Line_Context_s *Dwarf_Line_Context;

/* 
    This structure provides the context in which the fields of 
    a Dwarf_Line structure are interpreted.  They come from the 
    statement program prologue.  **Updated by dwarf_srclines in 
    dwarf_line.c.
*/
struct Dwarf_Line_Context_s {
    /* 
       Points to a chain of entries providing info about source files
       for the current set of Dwarf_Line structures. File number
       'li_file 1' is last on the list, the first list entry is the
       file numbered lc_file_entry_count. The numbering of the file
       names matches the dwarf2/3 line table specification file table
       and DW_LNE_define_file numbering rules.  */
    Dwarf_File_Entry lc_file_entries;
    /* 
       Count of number of source files for this set of Dwarf_Line
       structures. */
    Dwarf_Sword lc_file_entry_count;
    /* 
       Points to the portion of .debug_line section that contains a
       list of strings naming the included directories. */
    Dwarf_Small *lc_include_directories;

    /* Count of the number of included directories. */
    Dwarf_Sword lc_include_directories_count;

    /* Count of the number of lines for this cu. */
    Dwarf_Sword lc_line_count;

    /* Points to name of compilation directory. */
    Dwarf_Small *lc_compilation_directory;

    Dwarf_Debug lc_dbg;

    Dwarf_Half lc_version_number;	/* DWARF2/3 version number, 2
					   for DWARF2, 3 for DWARF3. */
};


/*
    This structure defines a row of the line table.
    All of the fields except li_offset have the exact 
    same meaning that is defined in Section 6.2.2 
    of the Libdwarf Document. 

    li_offset is used by _dwarf_addr_finder() which is called
    by rqs(1), an sgi utility for 'moving' shared libraries
    as if the static linker (ld) had linked the shared library
    at the newly-specified address.  Most libdwarf-using 
    apps will ignore li_offset and _dwarf_addr_finder().
    
*/
struct Dwarf_Line_s {
    Dwarf_Addr li_address;	/* pc value of machine instr */
    union addr_or_line_s {
	struct li_inner_s {
	    Dwarf_Sword li_file;	/* int identifying src file */
	    /* li_file is a number 1-N, indexing into a conceptual
	       source file table as described in dwarf2/3 spec line
	       table doc. (see Dwarf_File_Entry lc_file_entries; and
	       Dwarf_Sword lc_file_entry_count;) */

	    Dwarf_Sword li_line;	/* source file line number. */
	    Dwarf_Half li_column;	/* source file column number */
	    Dwarf_Small li_isa;

	    /* To save space, use bit flags. */
	    /* indicate start of stmt */
	    unsigned char li_is_stmt:1;

	    /* indicate start basic block */
	    unsigned char li_basic_block:1;

	    /* first post sequence instr */
	    unsigned char li_end_sequence:1;

	    unsigned char li_prologue_end:1;
	    unsigned char li_epilogue_begin:1;
	} li_l_data;
	Dwarf_Off li_offset;	/* for rqs */
    } li_addr_line;
    Dwarf_Line_Context li_context;	/* assoc Dwarf_Line_Context_s */
};


int _dwarf_line_address_offsets(Dwarf_Debug dbg,
				Dwarf_Die die,
				Dwarf_Addr ** addrs,
				Dwarf_Off ** offs,
				Dwarf_Unsigned * returncount,
				Dwarf_Error * err);
int _dwarf_internal_srclines(Dwarf_Die die,
			     Dwarf_Line ** linebuf,
			     Dwarf_Signed * count,
			     Dwarf_Bool doaddrs,
			     Dwarf_Bool dolines, Dwarf_Error * error);



/* The LOP, WHAT_IS_OPCODE stuff is here so it can
   be reused in 3 places.  Seemed hard to keep
   the 3 places the same without an inline func or
   a macro.

   Handling the line section where the header and the
   file being processed do not match (unusual, but
   planned for in the  design of .debug_line)
   is too tricky to recode this several times and keep
   it right.

   As it is the code starting up line-reading is duplicated
   and that is just wrong to do. FIXME!
*/
#define LOP_EXTENDED 1
#define LOP_DISCARD  2
#define LOP_STANDARD 3
#define LOP_SPECIAL  4

#define WHAT_IS_OPCODE(type,opcode,base,opcode_length,line_ptr,highest_std) \
        if( (opcode) < (base) ) {                          \
           /* we know we must treat as a standard op       \
                or a special case.                         \
           */                                              \
           if((opcode) == DW_EXTENDED_OPCODE) {            \
                type = LOP_EXTENDED;                       \
           } else  if( ((highest_std)+1) >=  (base)) {     \
                /* == Standard case: compile of            \
                   dwarf_line.c and object                 \
                   have same standard op codes set.        \
                                                           \
                   >  Special case: compile of dwarf_line.c\
                   has things in standard op codes list    \
                   in dwarf.h header not                   \
                   in the object: handle this as a standard\
                   op code in switch below.                \
                   The header special ops overlap the      \
                   object standard ops.                    \
                   The new standard op codes will not      \
                   appear in the object.                   \
                */                                         \
                type = LOP_STANDARD;                       \
           } else  {                                       \
                /* These are standard opcodes in the object\
                ** that were not defined  in the header    \
                ** at the time dwarf_line.c                \
                ** was compiled. Provides the ability of   \
                ** out-of-date dwarf reader to read newer  \
                ** line table data transparently.          \
                */                                         \
                type = LOP_DISCARD;                         \
           }                                                \
                                                            \
        } else {                                            \
	   /* Is  a special op code.                        \
	   */                                               \
           type =  LOP_SPECIAL;                             \
        }

/* The following is from  the dwarf definition of 'ubyte'
   and is specifically  mentioned in section  6.2.5.1, page 54
   of the Rev 2.0.0 dwarf specification.
*/

#define MAX_LINE_OP_CODE  255


/* The following structs (Line_Table_File_Entry_s,Line_Table_Prefix_s)
   and functions allow refactoring common code into a single
   reader routine.
*/
/* There can be zero of more of these needed for 1 line prologue. */
struct Line_Table_File_Entry_s {
    Dwarf_Small *lte_filename;
    Dwarf_Unsigned lte_directory_index;
    Dwarf_Unsigned lte_last_modification_time;
    Dwarf_Unsigned lte_length_of_file;
};

/* Data  picked up from the line table prologue for a single
CU. */
struct Line_Table_Prefix_s {

    /* pf_total_length is the value of the length field for the line
       table of this CU. So it does not count the length of itself (the 
       length value) for consistency with the say lenghts recorded in
       DWARF2/3. */
    Dwarf_Unsigned pf_total_length;

    /* Length of the initial length field itself. */
    Dwarf_Half pf_length_field_length;

    /* The version is 2 for DWARF2, 3 for DWARF3 */
    Dwarf_Half pf_version;

    Dwarf_Unsigned pf_prologue_length;
    Dwarf_Small pf_minimum_instruction_length;

    /* Start and end of this CU line area. pf_line_ptr_start +
       pf_total_length + pf_length_field_length == pf_line_ptr_end.
       Meaning pf_line_ptr_start is before the length info. */
    Dwarf_Small *pf_line_ptr_start;
    Dwarf_Small *pf_line_ptr_end;

    /* Used to check that decoding of the line prologue is done right. */
    Dwarf_Small *pf_line_prologue_start;

    Dwarf_Small pf_default_is_stmt;
    Dwarf_Sbyte pf_line_base;
    Dwarf_Small pf_line_range;

    /* Highest std opcode (+1).  */
    Dwarf_Small pf_opcode_base;

    /* pf_opcode_base -1 entries (each a count, normally the value of
       each entry is 0 or 1). */
    Dwarf_Small *pf_opcode_length_table;

    Dwarf_Unsigned pf_include_directories_count;
    /* Array of pointers to dir strings. pf_include_directories_count
       entriesin the array. */
    Dwarf_Small **pf_include_directories;

    /* Count of entries in line_table_file_entries array. */
    Dwarf_Unsigned pf_files_count;
    struct Line_Table_File_Entry_s *pf_line_table_file_entries;

    /* The number to treat as standard ops. This is a special
       accomodation of gcc using the new standard opcodes but not
       updating the version number. It's legal dwarf2, but much better
       for the user to understand as dwarf3 when 'it looks ok'. */
    Dwarf_Bool pf_std_op_count;

};

void dwarf_init_line_table_prefix(struct Line_Table_Prefix_s *pf);
void dwarf_free_line_table_prefix(struct Line_Table_Prefix_s *pf);

int
  dwarf_read_line_table_prefix(Dwarf_Debug dbg,
			       Dwarf_Small * data_start,
			       Dwarf_Unsigned data_length,
			       Dwarf_Small ** updated_data_start_out,
			       struct Line_Table_Prefix_s *prefix_out,
			       Dwarf_Error * err);
