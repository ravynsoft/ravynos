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



/* The dwarf 2.0 standard dictates that only the following
 * fields can be read when an unexpected augmentation string
 * (in the cie) is encountered: CIE length, CIE_id, version and
 * augmentation; FDE: length, CIE pointer, initial location and
 * address range. Unfortunately, with the above restrictions, it
 * is impossible to read the instruction table from a CIE or a FDE
 * when a new augmentation string is encountered.
 * To fix this problem, the following layout is used, if the
 * augmentation string starts with the string "z".
 *   CIE                        FDE
 *   length                     length
 *   CIE_id                     CIE_pointer
 *   version                    initial_location
 *   augmentation               address_range
 *                              length_of_augmented_fields (*NEW*)
 *   code_alignment_factor      Any new fields as necessary
 *   data_alignment_factor      instruction_table
 *   return_address
 *   length_of_augmented fields
 *   Any new fields as necessary
 *   initial_instructions
 *
 * The type of all the old data items are the same as what is
 * described in dwarf 2.0 standard. The length_of_augmented_fields
 * is an LEB128 data item that denotes the size (in bytes) of
 * the augmented fields (not including the size of
 * "length_of_augmented_fields" itself).
 
 * Handling of cie augmentation strings is necessarly a heuristic.
 * See dwarf_frame.c for the currently known augmentation strings.


   ---START SGI-ONLY COMMENT:
 * SGI-IRIX versions of cie or fde  were intended to use "z1", "z2" as the
 * augmenter strings if required for new augmentation.
 * However, that never happened (as of March 2005).
 *
 * The fde's augmented by the string "z" have a new field 
 * (signed constant, 4 byte field)
 * called offset_into_exception_tables, following the 
 * length_of_augmented field.   This field contains an offset 
 * into the "_MIPS_eh_region", which describes
 * the IRIX CC exception handling tables.
   ---END SGI-ONLY COMMENT
 

 * GNU .eh_frame has an augmentation string of z[RLP]* (gcc 3.4)
 * The similarity to IRIX 'z' (and proposed but never
 * implemented IRIX z1, z2 etc) was confusing things.
 * If the section is .eh_frame then 'z' means GNU exception
 * information 'Augmentation Data' not IRIX 'z'.
 * See The Linux Standard Base Core Specification version 3.0
 */

#define DW_DEBUG_FRAME_VERSION                 	1 /* DWARF2 */
#define DW_DEBUG_FRAME_VERSION3                	3 /* DWARF3 */
#define DW_DEBUG_FRAME_AUGMENTER_STRING     	"mti v1"

/* The value of the offset field for Cie's. */
#define DW_CIE_OFFSET		~(0x0)

/* The augmentation string may be NULL.	*/
#define DW_EMPTY_STRING		""

#define DW_FRAME_INSTR_OPCODE_SHIFT		6
#define DW_FRAME_INSTR_OFFSET_MASK		0x3f

/* 
    This struct denotes the rule for a register in a row of
    the frame table.  In other words, it is one element of 
    the table.
*/
struct Dwarf_Reg_Rule_s {

    /* 
       Is a flag indicating whether the rule includes the offset
       field, ie whether the ru_offset field is valid or not. 
       Applies only if DW_EXPR_OFFSET or DW_EXPR_VAL_OFFSET.
       It is important, since reg+offset (offset of 0) is different from
       just 'register' since the former means 'read memory at address
       given by the sum of register contents plus offset to get the
       value'. whereas the latter means 'the value is in the register'.

       The 'register' numbers are either real registers (ie, table
       columns defined as real registers) or defined entries that are
       not really hardware registers, such as DW_FRAME_SAME_VAL or
       DW_FRAME_CFA_COL.

     */
    Dwarf_Sbyte ru_is_off; 

    /* DW_EXPR_OFFSET (0, DWARF2)  
       DW_EXPR_VAL_OFFSET 1 (dwarf2/3)
       DW_EXPR_EXPRESSION 2  (dwarf2/3)
	DW_EXPR_VAL_EXPRESSION 3 (dwarf2/3) 
       See dwarf_frame.h. */
    Dwarf_Sbyte ru_value_type;

    /* Register involved in this rule. */
    Dwarf_Half ru_register;

    /* Offset to add to register, if indicated by ru_is_offset
       and if DW_EXPR_OFFSET or DW_EXPR_VAL_OFFSET. 
       If DW_EXPR_EXPRESSION or DW_EXPR_VAL_EXPRESSION
       this is DW_FORM_block block-length, not offset. */
    Dwarf_Unsigned ru_offset_or_block_len;
    
    /* For DW_EXPR_EXPRESSION DW_EXPR_VAL_EXPRESSION these is set,
       else 0. */
    Dwarf_Small *ru_block;
};

typedef struct Dwarf_Frame_s *Dwarf_Frame;

/* 
    This structure represents a row of the frame table. 
    Fr_loc is the pc value for this row, and Fr_reg
    contains the rule for each column.

    Entry DW_FRAME_CFA_COL of fr_reg was the tradional MIPS
    way of setting CFA.  cfa_rule is the new one.
*/
struct Dwarf_Frame_s {

    /* Pc value corresponding to this row of the frame table. */
    Dwarf_Addr fr_loc;

    /* Rules for all the registers in this row. */
    struct Dwarf_Reg_Rule_s fr_cfa_rule;

	/* fr_reg_count is the the number of
	entries of the fr_reg array. */
    unsigned long            fr_reg_count;
    struct Dwarf_Reg_Rule_s *fr_reg;

    Dwarf_Frame fr_next;
};

typedef struct Dwarf_Frame_Op_List_s *Dwarf_Frame_Op_List;

/* This is used to chain together Dwarf_Frame_Op structures. */
struct Dwarf_Frame_Op_List_s {
    Dwarf_Frame_Op *fl_frame_instr;
    Dwarf_Frame_Op_List fl_next;
};

/* See dwarf_frame.c for the heuristics used to set the
   Dwarf_Cie ci_augmentation_type.  

   This succinctly helps interpret the size and meaning of .debug_frame
   and (for gcc) .eh_frame.

   In the case of gcc .eh_frame (gcc 3.3, 3.4)
   z may be followed by one or more of
   L R P.  

*/
enum Dwarf_augmentation_type {
        aug_empty_string, /* Default empty augmentation string.  */
        aug_irix_exception_table,  /* IRIX  plain  "z",
                   for exception handling, IRIX CC compiler.
		   Proposed z1 z2 ... never implemented.  */
        aug_gcc_eh_z,       /* gcc z augmentation,  (including
			L R P variations). gcc 3.3 3.4 exception
			handling in eh_frame.  */
        aug_irix_mti_v1,  /* IRIX "mti v1" augmentation string. Probably
                             never in any released SGI-IRIX compiler. */
        aug_eh,           /* For gcc .eh_frame, "eh" is the string.,
				gcc 1,2, egcs. Older values.  */
        aug_unknown,      /* Unknown augmentation, we cannot do much. */
        aug_past_last
};


/* 
    This structure contains all the pertinent info for a Cie. Most 
    of the fields are taken straight from the definition of a Cie.  
    Ci_cie_start points to the address (in .debug_frame) where this 
    Cie begins.  Ci_cie_instr_start points to the first byte of the 
    frame instructions for this Cie.  Ci_dbg points to the associated 
    Dwarf_Debug structure.  Ci_initial_table is a pointer to the table 
    row generated by the instructions for this Cie.
*/
struct Dwarf_Cie_s {
    Dwarf_Word ci_length;
    char *ci_augmentation;
    Dwarf_Small ci_code_alignment_factor;
    Dwarf_Sbyte ci_data_alignment_factor;
    Dwarf_Small ci_return_address_register;
    Dwarf_Small *ci_cie_start;
    Dwarf_Small *ci_cie_instr_start;
    Dwarf_Debug ci_dbg;
    Dwarf_Frame ci_initial_table;
    Dwarf_Cie ci_next;
    Dwarf_Small ci_length_size;
    Dwarf_Small ci_extension_size;
    Dwarf_Half ci_cie_version_number;
    enum Dwarf_augmentation_type ci_augmentation_type;	 

    /* The following 2 for GNU .eh_frame exception handling
       Augmentation Data. Set if ci_augmentation_type
       is aug_gcc_eh_z. Zero if unused. */
    Dwarf_Unsigned ci_gnu_eh_augmentation_len;
    Dwarf_Ptr      ci_gnu_eh_augmentation_bytes;

    /* These are extracted from the gnu eh_frame
       augmentation if the
       augmentation begins with 'z'. See Linux LSB documents.
       Otherwize these are zero. */
    unsigned char    ci_gnu_personality_handler_encoding;
    unsigned char    ci_gnu_lsda_encoding;
    unsigned char    ci_gnu_fde_begin_encoding;

    /* If 'P' augmentation present, is handler addr. Else
	is zero. */
    Dwarf_Addr     ci_gnu_personality_handler_addr;


    /* In creating list of cie's (which will become an array)
       record the position so fde can get it on fde creation. */
    Dwarf_Unsigned ci_index;
    Dwarf_Small *  ci_section_ptr;
};

/*
	This structure contains all the pertinent info for a Fde.
	Most of the fields are taken straight from the definition.
	fd_cie_index is the index of the Cie associated with this
	Fde in the list of Cie's for this debug_frame.  Fd_cie
	points to the corresponsing Dwarf_Cie structure.  Fd_fde_start
	points to the start address of the Fde.  Fd_fde_instr_start
	points to the start of the instructions for this Fde.  Fd_dbg
	points to the associated Dwarf_Debug structure.
*/
struct Dwarf_Fde_s {
    Dwarf_Unsigned fd_length;
    Dwarf_Addr fd_cie_offset;
    Dwarf_Unsigned fd_cie_index;
    Dwarf_Cie fd_cie;
    Dwarf_Addr fd_initial_location;
    Dwarf_Small *fd_initial_loc_pos;
    Dwarf_Addr fd_address_range;
    Dwarf_Small *fd_fde_start;
    Dwarf_Small *fd_fde_instr_start;
    Dwarf_Debug fd_dbg;

	/* fd_offset_into_exception_tables is SGI/IRIX exception table
	   offset. Unused and zero if not IRIX .debug_frame. */
    Dwarf_Signed fd_offset_into_exception_tables;

    Dwarf_Fde fd_next;
    Dwarf_Small fd_length_size;
    Dwarf_Small fd_extension_size;
    /* The following 2 for GNU .eh_frame exception handling
       Augmentation Data. Set if CIE ci_augmentation_type
       is aug_gcc_eh_z. Zero if unused. */
    Dwarf_Unsigned fd_gnu_eh_augmentation_len;
    Dwarf_Ptr fd_gnu_eh_augmentation_bytes;
    Dwarf_Addr fd_gnu_eh_lsda; /* If 'L' augmentation letter
         present:  is address of the 
         Language Specific Data Area (LSDA). If not 'L" is zero. */

    /* The following 3 are about the Elf section the FDEs come from. */
    Dwarf_Small * fd_section_ptr;
    Dwarf_Unsigned fd_section_length;
    Dwarf_Unsigned fd_section_index; 

};


int
  _dwarf_frame_address_offsets(Dwarf_Debug dbg, Dwarf_Addr ** addrlist,
			       Dwarf_Off ** offsetlist,
			       Dwarf_Signed * returncount,
			       Dwarf_Error * err);

int
_dwarf_get_fde_list_internal(Dwarf_Debug dbg,
                              Dwarf_Cie ** cie_data,
                              Dwarf_Signed * cie_element_count,
                              Dwarf_Fde ** fde_data,
                              Dwarf_Signed * fde_element_count,
                              Dwarf_Small * section_ptr,
                              Dwarf_Unsigned section_index,
                              Dwarf_Unsigned section_length,
                              Dwarf_Unsigned cie_id_value,
                              int use_gnu_cie_calc,  /* If non-zero,
                                this is gcc eh_frame. */
                              Dwarf_Error * error);

enum Dwarf_augmentation_type
_dwarf_get_augmentation_type(Dwarf_Debug dbg,
        Dwarf_Small *augmentation_string,
        int is_gcc_eh_frame);

Dwarf_Unsigned _dwarf_get_return_address_reg(Dwarf_Small *frame_ptr,
                        int version,
                        unsigned long *size);

/* Temporary recording of crucial cie/fde prefix data.
 * Vastly simplifies some argument lists.
 */
struct cie_fde_prefix_s {
   Dwarf_Small *  cf_start_addr;
   Dwarf_Small *  cf_addr_after_prefix;
   Dwarf_Unsigned cf_length;
   int            cf_local_length_size;
   int            cf_local_extension_size;
   Dwarf_Unsigned cf_cie_id;
   Dwarf_Small *  cf_cie_id_addr; /* used for eh_frame calculations. */

   /* Simplifies passing around these values to create fde having
      these here. */
   Dwarf_Small *  cf_section_ptr;
   Dwarf_Unsigned cf_section_index; 
   Dwarf_Unsigned cf_section_length; 
};

int
_dwarf_exec_frame_instr(Dwarf_Bool make_instr,
                        Dwarf_Frame_Op ** ret_frame_instr,
                        Dwarf_Bool search_pc,
                        Dwarf_Addr search_pc_val,
                        Dwarf_Addr initial_loc,
                        Dwarf_Small * start_instr_ptr,
                        Dwarf_Small * final_instr_ptr,
                        Dwarf_Frame table,
                        Dwarf_Cie cie,
                        Dwarf_Debug dbg,
			Dwarf_Half reg_num_of_cfa,
                        Dwarf_Sword * returned_count,
                        int *returned_error);


int dwarf_read_cie_fde_prefix(Dwarf_Debug dbg,
        Dwarf_Small *frame_ptr_in,
        Dwarf_Small *section_ptr_in,
        Dwarf_Unsigned section_index_in,
	Dwarf_Unsigned section_length_in,
        struct cie_fde_prefix_s *prefix_out,
        Dwarf_Error *error);

int dwarf_create_fde_from_after_start(Dwarf_Debug dbg,
        struct cie_fde_prefix_s *  prefix,
        Dwarf_Small *frame_ptr,
        int use_gnu_cie_calc,
        Dwarf_Cie  cie_ptr_in,
        Dwarf_Fde *fde_ptr_out,
        Dwarf_Error *error);

int dwarf_create_cie_from_after_start(Dwarf_Debug dbg,
        struct cie_fde_prefix_s *prefix,
        Dwarf_Small* frame_ptr,
        Dwarf_Unsigned cie_count,
        int use_gnu_cie_calc,
        Dwarf_Cie *cie_ptr_out,
        Dwarf_Error *error);


int _dwarf_frame_constructor(Dwarf_Debug dbg,void * );
void _dwarf_frame_destructor (void *);
