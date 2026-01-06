/*

  Copyright (C) 2000,2001,2002,2005,2006 Silicon Graphics, Inc.  All Rights Reserved.

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


#ifndef _LIBDWARF_H
#define _LIBDWARF_H
#ifdef __cplusplus
extern "C" {
#endif
/*
	libdwarf.h  
	$Revision: 1.88 $ $Date: 2006/04/18 04:46:07 $

	For libdwarf producers and consumers

	The interface is defined as having 8-byte signed and unsigned
	values so it can handle 64-or-32bit target on 64-or-32bit host.
	Addr is the native size: it represents pointers on
	the host machine (not the target!).

	This contains declarations for types and all producer
	and consumer functions.

	Function declarations are written on a single line each here
	so one can use grep  to each declaration in its entirety.
	The declarations are a little harder to read this way, but...

*/

#ifdef __SGI_FAST_LIBELF
struct elf_sgi;
typedef struct elf_sgi* dwarf_elf_handle;
#else
struct Elf;
typedef struct Elf* dwarf_elf_handle;
#endif

#if (_MIPS_SZLONG == 64)
/* Special case for MIPS, so -64 (LP64) build gets simple -long-.
   Non-MIPS LP64 or ILP64 environments should probably ensure
   _MIPS_SZLONG set to 64 everywhere this header is #included.
*/
typedef int             Dwarf_Bool;         /* boolean type */
typedef unsigned long   Dwarf_Off;          /* 4 or 8 byte file offset */
typedef unsigned long   Dwarf_Unsigned;     /* 4 or 8 byte unsigned value */
typedef unsigned short  Dwarf_Half;         /* 2 byte unsigned value */
typedef unsigned char   Dwarf_Small;        /* 1 byte unsigned value */
typedef signed   long   Dwarf_Signed;       /* 4 or 8 byte signed value */
typedef unsigned long   Dwarf_Addr;         /* target memory address */
#else /* 32-bit */
/* This is for ILP32, allowing i/o of 64bit dwarf info.
   Also should be fine for LP64 and ILP64 cases.
*/
typedef int                 Dwarf_Bool;     /* boolean type */
typedef unsigned long long  Dwarf_Off;      /* 8 byte file offset */
typedef unsigned long long  Dwarf_Unsigned; /* 8 byte unsigned value*/
typedef unsigned short      Dwarf_Half;     /* 2 byte unsigned value */
typedef unsigned char       Dwarf_Small;    /* 1 byte unsigned value */
typedef signed   long long  Dwarf_Signed;   /* 8 byte signed value */
typedef unsigned long long  Dwarf_Addr;     /* target memory address */
#endif
typedef void*		Dwarf_Ptr;          /* host machine pointer */

/* Contains info on an uninterpreted block of data
*/
typedef struct {
        Dwarf_Unsigned  bl_len;         /* length of block */
        Dwarf_Ptr       bl_data;        /* uninterpreted data */
	Dwarf_Small     bl_from_loclist; /*non-0 if loclist, else debug_info*/
	Dwarf_Unsigned  bl_section_offset; /* Section (not CU) offset
                                        which 'data' comes from. */
} Dwarf_Block;


/* location record
*/
typedef struct {
        Dwarf_Small     lr_atom;        /* location operation */
        Dwarf_Unsigned  lr_number;      /* operand */
	Dwarf_Unsigned  lr_number2;     /* for OP_BREGx */
	Dwarf_Unsigned  lr_offset;      /* offset in locexpr for OP_BRA etc */
} Dwarf_Loc;


/* location description
*/
typedef struct {
        Dwarf_Addr      ld_lopc;        /* beginning of active range */ 
        Dwarf_Addr      ld_hipc;        /* end of active range */
        Dwarf_Half      ld_cents;       /* count of location records */
        Dwarf_Loc*      ld_s;           /* pointer to list of same */
	Dwarf_Small     ld_from_loclist; 
				      /* non-0 if loclist, else debug_info*/

	Dwarf_Unsigned  ld_section_offset; /* Section (not CU) offset
					where loc-expr begins*/
} Dwarf_Locdesc;

/* Frame description instructions expanded.
*/
typedef struct {
        Dwarf_Small     fp_base_op;
        Dwarf_Small     fp_extended_op;
        Dwarf_Half      fp_register;

	/* Value may be signed, depends on op. 
           Any applicable data_alignment_factor has
           not been applied, this is the  raw offset. */
        Dwarf_Unsigned  fp_offset;
        Dwarf_Off       fp_instr_offset;
} Dwarf_Frame_Op; /* DWARF2 */

typedef struct {
        Dwarf_Small     fp_base_op;
        Dwarf_Small     fp_extended_op;
        Dwarf_Half      fp_register;

	/* Value may be signed, depends on op. 
           Any applicable data_alignment_factor has
           not been applied, this is the  raw offset. */
        Dwarf_Unsigned  fp_offset_or_block_len; 
	Dwarf_Small     *fp_expr_block;

        Dwarf_Off       fp_instr_offset;
} Dwarf_Frame_Op3;  /* DWARF3 and DWARF2 compatible */

/*  ***IMPORTANT NOTE, TARGET DEPENDENCY ****
   DW_REG_TABLE_SIZE must be at least as large as
   the number of registers
   (DW_FRAME_LAST_REG_NUM) as defined in dwarf.h
   Preferably identical to DW_FRAME_LAST_REG_NUM.
   Ensure [0-DW_REG_TABLE_SIZE] does not overlap 
   DW_FRAME_UNDEFINED_VAL or DW_FRAME_SAME_VAL. 
   Also ensure DW_FRAME_REG_INITIAL_VALUE is set to what
   is appropriate to your cpu.
   For various CPUs  DW_FRAME_UNDEFINED_VAL is correct
   as the value for DW_FRAME_REG_INITIAL_VALUE.

   For consumer apps, this can be set dynamically: see
   dwarf_set_frame_rule_table_size();
 */
#ifndef DW_REG_TABLE_SIZE
#define DW_REG_TABLE_SIZE  66
#endif

/* For MIPS, DW_FRAME_SAME_VAL is the correct default value 
   for a frame register value. For other CPUS another value
   may be better, such as DW_FRAME_UNDEFINED_VAL.
   See dwarf_set_frame_rule_table_size
*/
#ifndef DW_FRAME_REG_INITIAL_VALUE
#define DW_FRAME_REG_INITIAL_VALUE DW_FRAME_SAME_VAL
#endif

/* Taken as meaning 'undefined value', this is not
   a column or register number.
   Only present at libdwarf runtime. Never on disk.
   DW_FRAME_* Values present on disk are in dwarf.h
   Ensure this is > DW_REG_TABLE_SIZE.
*/
#define DW_FRAME_UNDEFINED_VAL          1034

/* Taken as meaning 'same value' as caller had, not a column
   or register number
   Only present at libdwarf runtime. Never on disk.
   DW_FRAME_* Values present on disk are in dwarf.h
   Ensure this is > DW_REG_TABLE_SIZE.
*/
#define DW_FRAME_SAME_VAL               1035

/* For DWARF3 interfaces, make the CFA a column with no
   real table number.  This is what should have been done
   for the DWARF2 interfaces.  This actually works for
   both DWARF2 and DWARF3, but see the libdwarf documentation
   on Dwarf_Regtable3 and  dwarf_get_fde_info_for_reg3()
   and  dwarf_get_fde_info_for_all_regs3()  
   Do NOT use this with the older dwarf_get_fde_info_for_reg()
   or dwarf_get_fde_info_for_all_regs() consumer interfaces.
*/
#define DW_FRAME_CFA_COL3               1036

/* The following are all needed to evaluate DWARF3 register rules.
*/
#define DW_EXPR_OFFSET 0  /* DWARF2 only sees this. */
#define DW_EXPR_VAL_OFFSET 1
#define DW_EXPR_EXPRESSION 2
#define DW_EXPR_VAL_EXPRESSION 3

typedef struct Dwarf_Regtable_Entry_s {
	/*  For each index i (naming a hardware register with dwarf number
            i) the following is true and defines the value of that register:

           If dw_regnum is Register DW_FRAME_UNDEFINED_VAL  
	     it is not DWARF register number but
		a place holder indicating the register has no defined value.
           If dw_regnum is Register DW_FRAME_SAME_VAL 
  	     it  is not DWARF register number but
		a place holder indicating the register has the same
                value in the previous frame.
	   DW_FRAME_UNDEFINED_VAL, DW_FRAME_SAME_VAL are
           only present at libdwarf runtime. Never on disk.
           DW_FRAME_* Values present on disk are in dwarf.h

          Otherwise: the register number is a DWARF register number
          (see ABI documents for how this translates to hardware/
           software register numbers in the machine hardware)
	  and the following applies:

          if dw_value_type == DW_EXPR_OFFSET (the only  possible case for dwarf2):
	    If dw_offset_relevant is non-zero, then
	     the value is stored at at the address CFA+N where N is a signed offset. 
	     Rule: Offset(N)
            If dw_offset_relevant is zero, then the value of the register
             is the value of (DWARF) register number dw_regnum.
             Rule: register(F)
          Other values of dw_value_type are an error.
        */
	Dwarf_Small         dw_offset_relevant;

	/* For DWARF2, always 0 */
        Dwarf_Small         dw_value_type; 

	Dwarf_Half          dw_regnum;

	/* The data type here should  the larger of Dwarf_Addr
           and Dwarf_Unsigned and Dwarf_Signed. */
	Dwarf_Addr          dw_offset;
} Dwarf_Regtable_Entry;

typedef struct Dwarf_Regtable_s {
    struct Dwarf_Regtable_Entry_s rules[DW_REG_TABLE_SIZE];
} Dwarf_Regtable;

/* opaque type. Functional interface shown later. */
struct Dwarf_Reg_value3_s;
typedef struct Dwarf_Reg_value3_s Dwarf_Reg_Value3; 

typedef struct Dwarf_Regtable_Entry3_s {
	/*  For each index i (naming a hardware register with dwarf number
            i) the following is true and defines the value of that register:

           If dw_regnum is Register DW_FRAME_UNDEFINED_VAL  
	     it is not DWARF register number but
		a place holder indicating the register has no defined value.
           If dw_regnum is Register DW_FRAME_SAME_VAL 
  	     it  is not DWARF register number but
		a place holder indicating the register has the same
                value in the previous frame.
	   DW_FRAME_UNDEFINED_VAL, DW_FRAME_SAME_VAL are
           only present at libdwarf runtime. Never on disk.
           DW_FRAME_* Values present on disk are in dwarf.h

          Otherwise: the register number is a DWARF register number
          (see ABI documnets for how this translates to hardware/
           software register numbers in the machine hardware)
	  and the following applies:

          if dw_value_type == DW_EXPR_OFFSET (the only  possible case for 
		dwarf2):
	    If dw_offset_relevant is non-zero, then
	     the value is stored at at the address 
	     CFA+N where N is a signed offset. 
	     Rule: Offset(N)
            If dw_offset_relevant is zero, then the value of the register
             is the value of (DWARF) register number dw_regnum.
             Rule: register(R)
          if dw_value_type  == DW_EXPR_VAL_OFFSET
            the  value of this register is CFA +N where N is a signed offset.
            Rule: val_offset(N)

	  E is pointed to by dw_block_ptr (length is dw_offset_or_block_len);
          if dw_value_type  == DW_EXPR_EXPRESSION
	    The value of the register is the value at the address
            computed by evaluating the DWARF expression E.
            Rule: expression(E)
          if dw_value_type  == DW_EXPR_VAL_EXPRESSION
	    The value of the register is the value
            computed by evaluating the DWARF expression E.
            Rule: val_expression(E)
          Other values of dw_value_type are an error.
        */
	Dwarf_Small         dw_offset_relevant;
        Dwarf_Small         dw_value_type; 
	Dwarf_Half          dw_regnum;
	Dwarf_Unsigned      dw_offset_or_block_len;
	Dwarf_Ptr           dw_block_ptr;

}Dwarf_Regtable_Entry3;

/* For the DWARF3 version, moved the DW_FRAME_CFA_COL
   out of the array and into its own struct.  
   Having it part of the array is not very easy to work
   with from a portability point of view: changing
   the number for every architecture is a pain (if one fails
   to set it correctly a register rule gets clobbered when
   setting CFA).
   With MIPS it just happened to be easy to use DW_FRAME_CFA_COL.

   rt3_rules and rt3_reg_table_size must be filled in 
    before calling libdwarf.
    Filled in with a pointer to an array (pointer and
    array  set up by the calling application) of rt3_reg_table_size 
    Dwarf_Regtable_Entry3_s structs.   
    libdwarf does not allocate or deallocate space for the
    rules, you must do so.   libdwarf will initialize the
    contents rules array, you do not need to do so (though
    if you choose to initialize the array somehow that is ok:
    libdwarf will overwrite your initializations with its own).

*/
typedef struct Dwarf_Regtable3_s {
    struct Dwarf_Regtable_Entry3_s   rt3_cfa_rule;

    Dwarf_Half                       rt3_reg_table_size;
    struct Dwarf_Regtable_Entry3_s * rt3_rules;
} Dwarf_Regtable3;


/* Use for DW_EPXR_STANDARD., DW_EXPR_VAL_OFFSET. 
   Returns DW_DLV_OK if the value is available.
   If DW_DLV_OK returns the regnum and offset thru the pointers
   (which the consumer must use appropriately). 
*/
int dwarf_frame_get_reg_register(struct Dwarf_Regtable_Entry3_s *reg_in,
	Dwarf_Small *offset_relevant,
	Dwarf_Half *regnum_out,
	Dwarf_Signed *offset_out);

/* Use for DW_EXPR_EXPRESSION, DW_EXPR_VAL_EXPRESSION.
   Returns DW_DLV_OK if the value is available.
   The caller must pass in the address of a valid
   Dwarf_Block (the caller need not initialize it).
*/
int dwarf_frame_get_reg_expression(struct Dwarf_Regtable_Entry3_s *reg_in,
	Dwarf_Block *block_out);


/* for DW_DLC_SYMBOLIC_RELOCATIONS output to caller 
   v2, adding drd_length: some relocations are 4 and
   some 8 bytes (pointers are 8, section offsets 4) in
   some dwarf environments. (MIPS relocations are all one
   size in any given ABI.) Changing drd_type to an unsigned char
   to keep struct size down.
*/
enum Dwarf_Rel_Type {
		dwarf_drt_none, /* should not get to caller */
                dwarf_drt_data_reloc, /* simple normal relocation */
                dwarf_drt_segment_rel, /* special reloc, exceptions*/
                dwarf_drt_first_of_length_pair,/* this and drt_second 
				for .word end - begin
			 	case */
                dwarf_drt_second_of_length_pair
};
typedef struct Dwarf_Relocation_Data_s  * Dwarf_Relocation_Data;
struct Dwarf_Relocation_Data_s {
        unsigned char drd_type; /* cast to/from Dwarf_Rel_Type
					  to keep size small in struct */
	unsigned char drd_length; /* length in bytes
			         of data being relocated. 4 for 32bit.
				 8 for 64bit data */
        Dwarf_Unsigned       drd_offset; /* where the data to reloc is */
        Dwarf_Unsigned       drd_symbol_index;
};

/* Opaque types for Consumer Library. */
typedef struct Dwarf_Debug_s*      Dwarf_Debug;
typedef struct Dwarf_Die_s*        Dwarf_Die;
typedef struct Dwarf_Line_s*       Dwarf_Line;
typedef struct Dwarf_Global_s*     Dwarf_Global;
typedef struct Dwarf_Func_s*       Dwarf_Func;
typedef struct Dwarf_Type_s*       Dwarf_Type;
typedef struct Dwarf_Var_s*        Dwarf_Var;
typedef struct Dwarf_Weak_s*       Dwarf_Weak;
typedef struct Dwarf_Error_s*      Dwarf_Error;
typedef struct Dwarf_Attribute_s*  Dwarf_Attribute;
typedef struct Dwarf_Abbrev_s*	   Dwarf_Abbrev;
typedef struct Dwarf_Fde_s*  	   Dwarf_Fde;
typedef struct Dwarf_Cie_s*  	   Dwarf_Cie;
typedef struct Dwarf_Arange_s*	   Dwarf_Arange;

/* error handler function
*/
typedef void  (*Dwarf_Handler)(Dwarf_Error /*error*/, Dwarf_Ptr /*errarg*/); 


/* 
    Dwarf_dealloc() alloc_type arguments.
    Argument points to:
*/
#define DW_DLA_STRING      	0x01     /* char* */
#define DW_DLA_LOC         	0x02     /* Dwarf_Loc */
#define DW_DLA_LOCDESC     	0x03     /* Dwarf_Locdesc */
#define DW_DLA_ELLIST      	0x04     /* Dwarf_Ellist (not used)*/
#define DW_DLA_BOUNDS      	0x05     /* Dwarf_Bounds (not used) */
#define DW_DLA_BLOCK       	0x06     /* Dwarf_Block */
#define DW_DLA_DEBUG       	0x07     /* Dwarf_Debug */
#define DW_DLA_DIE         	0x08     /* Dwarf_Die */
#define DW_DLA_LINE        	0x09     /* Dwarf_Line */
#define DW_DLA_ATTR        	0x0a     /* Dwarf_Attribute */
#define DW_DLA_TYPE        	0x0b     /* Dwarf_Type  (not used) */
#define DW_DLA_SUBSCR      	0x0c     /* Dwarf_Subscr (not used) */
#define DW_DLA_GLOBAL      	0x0d     /* Dwarf_Global */
#define DW_DLA_ERROR       	0x0e     /* Dwarf_Error */
#define DW_DLA_LIST        	0x0f     /* a list */
#define DW_DLA_LINEBUF     	0x10     /* Dwarf_Line* (not used) */
#define DW_DLA_ARANGE      	0x11     /* Dwarf_Arange */
#define DW_DLA_ABBREV		0x12 	 /* Dwarf_Abbrev */
#define DW_DLA_FRAME_OP		0x13 	 /* Dwarf_Frame_Op */
#define DW_DLA_CIE		0x14	 /* Dwarf_Cie */
#define DW_DLA_FDE		0x15	 /* Dwarf_Fde */
#define DW_DLA_LOC_BLOCK	0x16	 /* Dwarf_Loc Block (not used) */
#define DW_DLA_FRAME_BLOCK	0x17	 /* Dwarf_Frame Block (not used) */
#define DW_DLA_FUNC		0x18	 /* Dwarf_Func */
#define DW_DLA_TYPENAME		0x19	 /* Dwarf_Type */
#define DW_DLA_VAR		0x1a	 /* Dwarf_Var */
#define DW_DLA_WEAK		0x1b	 /* Dwarf_Weak */
#define DW_DLA_ADDR		0x1c	 /* Dwarf_Addr sized entries */

/* The augmenter string for CIE */
#define DW_CIE_AUGMENTER_STRING_V0              "z"

/* dwarf_init() access arguments
*/
#define DW_DLC_READ        0        /* read only access */
#define DW_DLC_WRITE       1        /* write only access */
#define DW_DLC_RDWR        2        /* read/write access NOT SUPPORTED*/

/* dwarf_init() access flag modifiers
*/
#define DW_DLC_SIZE_64     0x40000000 /* 32-bit target */
#define DW_DLC_SIZE_32     0x20000000 /* 64-bit target */

/* dwarf_init() access flag modifiers
*/
#define DW_DLC_ISA_MIPS             0x00000000 /* MIPS target */
#define DW_DLC_ISA_IA64             0x01000000 /* IA64 target */
#define DW_DLC_STREAM_RELOCATIONS   0x02000000 /* old style binary relocs */
#define DW_DLC_SYMBOLIC_RELOCATIONS 0x04000000 /* usable with assem output */
#define DW_DLC_TARGET_BIGENDIAN     0x08000000 /* big    endian target */
#define DW_DLC_TARGET_LITTLEENDIAN  0x00100000 /* little endian target */

/* dwarf_pcline() slide arguments
*/
#define DW_DLS_BACKWARD   -1       /* slide backward to find line */
#define DW_DLS_NOSLIDE     0       /* match exactly without sliding */ 
#define DW_DLS_FORWARD     1       /* slide forward to find line */

/* libdwarf error numbers
*/
#define DW_DLE_NE          0     /* no error */ 
#define DW_DLE_VMM         1     /* dwarf format/library version mismatch */
#define DW_DLE_MAP         2     /* memory map failure */
#define DW_DLE_LEE         3     /* libelf error */
#define DW_DLE_NDS         4     /* no debug section */
#define DW_DLE_NLS         5     /* no line section */
#define DW_DLE_ID          6     /* invalid descriptor for query */
#define DW_DLE_IOF         7     /* I/O failure */
#define DW_DLE_MAF         8     /* memory allocation failure */
#define DW_DLE_IA          9     /* invalid argument */ 
#define DW_DLE_MDE         10     /* mangled debugging entry */
#define DW_DLE_MLE         11     /* mangled line number entry */
#define DW_DLE_FNO         12     /* file not open */
#define DW_DLE_FNR         13     /* file not a regular file */
#define DW_DLE_FWA         14     /* file open with wrong access */
#define DW_DLE_NOB         15     /* not an object file */
#define DW_DLE_MOF         16     /* mangled object file header */
#define DW_DLE_EOLL        17     /* end of location list entries */
#define DW_DLE_NOLL        18     /* no location list section */
#define DW_DLE_BADOFF      19     /* Invalid offset */
#define DW_DLE_EOS         20     /* end of section  */
#define DW_DLE_ATRUNC      21     /* abbreviations section appears truncated*/
#define DW_DLE_BADBITC     22     /* Address size passed to dwarf bad*/
				    /* It is not an allowed size (64 or 32) */
    /* Error codes defined by the current Libdwarf Implementation. */
#define DW_DLE_DBG_ALLOC                        23
#define DW_DLE_FSTAT_ERROR                      24
#define DW_DLE_FSTAT_MODE_ERROR                 25
#define DW_DLE_INIT_ACCESS_WRONG                26
#define DW_DLE_ELF_BEGIN_ERROR                  27
#define DW_DLE_ELF_GETEHDR_ERROR                28
#define DW_DLE_ELF_GETSHDR_ERROR                29
#define DW_DLE_ELF_STRPTR_ERROR                 30
#define DW_DLE_DEBUG_INFO_DUPLICATE             31
#define DW_DLE_DEBUG_INFO_NULL                  32
#define DW_DLE_DEBUG_ABBREV_DUPLICATE           33
#define DW_DLE_DEBUG_ABBREV_NULL                34
#define DW_DLE_DEBUG_ARANGES_DUPLICATE          35
#define DW_DLE_DEBUG_ARANGES_NULL               36
#define DW_DLE_DEBUG_LINE_DUPLICATE             37
#define DW_DLE_DEBUG_LINE_NULL                  38
#define DW_DLE_DEBUG_LOC_DUPLICATE              39
#define DW_DLE_DEBUG_LOC_NULL                   40
#define DW_DLE_DEBUG_MACINFO_DUPLICATE          41
#define DW_DLE_DEBUG_MACINFO_NULL               42
#define DW_DLE_DEBUG_PUBNAMES_DUPLICATE         43
#define DW_DLE_DEBUG_PUBNAMES_NULL              44
#define DW_DLE_DEBUG_STR_DUPLICATE              45
#define DW_DLE_DEBUG_STR_NULL                   46
#define DW_DLE_CU_LENGTH_ERROR                  47
#define DW_DLE_VERSION_STAMP_ERROR              48
#define DW_DLE_ABBREV_OFFSET_ERROR              49
#define DW_DLE_ADDRESS_SIZE_ERROR               50
#define DW_DLE_DEBUG_INFO_PTR_NULL              51
#define DW_DLE_DIE_NULL                         52
#define DW_DLE_STRING_OFFSET_BAD                53
#define DW_DLE_DEBUG_LINE_LENGTH_BAD            54
#define DW_DLE_LINE_PROLOG_LENGTH_BAD           55
#define DW_DLE_LINE_NUM_OPERANDS_BAD            56
#define DW_DLE_LINE_SET_ADDR_ERROR              57
#define DW_DLE_LINE_EXT_OPCODE_BAD              58
#define DW_DLE_DWARF_LINE_NULL                  59
#define DW_DLE_INCL_DIR_NUM_BAD                 60
#define DW_DLE_LINE_FILE_NUM_BAD                61
#define DW_DLE_ALLOC_FAIL                       62
#define DW_DLE_NO_CALLBACK_FUNC		    	63
#define DW_DLE_SECT_ALLOC		    	64
#define DW_DLE_FILE_ENTRY_ALLOC		    	65
#define DW_DLE_LINE_ALLOC		    	66
#define DW_DLE_FPGM_ALLOC		    	67
#define DW_DLE_INCDIR_ALLOC		    	68
#define DW_DLE_STRING_ALLOC		    	69
#define DW_DLE_CHUNK_ALLOC		    	70
#define DW_DLE_BYTEOFF_ERR		    	71
#define	DW_DLE_CIE_ALLOC		    	72
#define DW_DLE_FDE_ALLOC		    	73
#define DW_DLE_REGNO_OVFL		    	74
#define DW_DLE_CIE_OFFS_ALLOC		    	75
#define DW_DLE_WRONG_ADDRESS		    	76
#define DW_DLE_EXTRA_NEIGHBORS		    	77
#define	DW_DLE_WRONG_TAG		    	78
#define DW_DLE_DIE_ALLOC		    	79
#define DW_DLE_PARENT_EXISTS		    	80
#define DW_DLE_DBG_NULL                         81
#define DW_DLE_DEBUGLINE_ERROR		    	82
#define DW_DLE_DEBUGFRAME_ERROR		    	83
#define DW_DLE_DEBUGINFO_ERROR		    	84
#define DW_DLE_ATTR_ALLOC		    	85
#define DW_DLE_ABBREV_ALLOC		    	86
#define DW_DLE_OFFSET_UFLW		    	87
#define DW_DLE_ELF_SECT_ERR		    	88
#define DW_DLE_DEBUG_FRAME_LENGTH_BAD	    	89
#define DW_DLE_FRAME_VERSION_BAD	    	90
#define DW_DLE_CIE_RET_ADDR_REG_ERROR	    	91
#define DW_DLE_FDE_NULL			    	92
#define DW_DLE_FDE_DBG_NULL		    	93
#define DW_DLE_CIE_NULL			    	94
#define DW_DLE_CIE_DBG_NULL		    	95
#define DW_DLE_FRAME_TABLE_COL_BAD	    	96
#define DW_DLE_PC_NOT_IN_FDE_RANGE	    	97
#define DW_DLE_CIE_INSTR_EXEC_ERROR	    	98
#define DW_DLE_FRAME_INSTR_EXEC_ERROR	    	99
#define DW_DLE_FDE_PTR_NULL		    	100
#define DW_DLE_RET_OP_LIST_NULL		    	101
#define DW_DLE_LINE_CONTEXT_NULL	    	102
#define DW_DLE_DBG_NO_CU_CONTEXT	    	103
#define DW_DLE_DIE_NO_CU_CONTEXT	    	104
#define DW_DLE_FIRST_DIE_NOT_CU		    	105
#define DW_DLE_NEXT_DIE_PTR_NULL	    	106
#define DW_DLE_DEBUG_FRAME_DUPLICATE	    	107
#define DW_DLE_DEBUG_FRAME_NULL		    	108
#define DW_DLE_ABBREV_DECODE_ERROR	    	109
#define DW_DLE_DWARF_ABBREV_NULL		110
#define DW_DLE_ATTR_NULL		    	111
#define DW_DLE_DIE_BAD			    	112
#define DW_DLE_DIE_ABBREV_BAD		    	113
#define DW_DLE_ATTR_FORM_BAD		    	114
#define DW_DLE_ATTR_NO_CU_CONTEXT	    	115
#define DW_DLE_ATTR_FORM_SIZE_BAD	    	116
#define DW_DLE_ATTR_DBG_NULL		    	117
#define DW_DLE_BAD_REF_FORM		    	118
#define DW_DLE_ATTR_FORM_OFFSET_BAD	    	119
#define DW_DLE_LINE_OFFSET_BAD		    	120
#define DW_DLE_DEBUG_STR_OFFSET_BAD	    	121
#define DW_DLE_STRING_PTR_NULL		    	122
#define DW_DLE_PUBNAMES_VERSION_ERROR	    	123
#define DW_DLE_PUBNAMES_LENGTH_BAD	    	124
#define DW_DLE_GLOBAL_NULL		    	125
#define DW_DLE_GLOBAL_CONTEXT_NULL	    	126
#define DW_DLE_DIR_INDEX_BAD		    	127
#define DW_DLE_LOC_EXPR_BAD		    	128
#define DW_DLE_DIE_LOC_EXPR_BAD		    	129
#define DW_DLE_ADDR_ALLOC		    	130
#define DW_DLE_OFFSET_BAD		    	131
#define DW_DLE_MAKE_CU_CONTEXT_FAIL	    	132
#define DW_DLE_REL_ALLOC		    	133
#define DW_DLE_ARANGE_OFFSET_BAD	    	134
#define DW_DLE_SEGMENT_SIZE_BAD		    	135
#define DW_DLE_ARANGE_LENGTH_BAD	    	136
#define DW_DLE_ARANGE_DECODE_ERROR	    	137
#define DW_DLE_ARANGES_NULL		    	138
#define DW_DLE_ARANGE_NULL		    	139
#define DW_DLE_NO_FILE_NAME		    	140
#define DW_DLE_NO_COMP_DIR		    	141
#define DW_DLE_CU_ADDRESS_SIZE_BAD	    	142
#define DW_DLE_INPUT_ATTR_BAD		    	143
#define DW_DLE_EXPR_NULL		    	144
#define DW_DLE_BAD_EXPR_OPCODE		    	145
#define DW_DLE_EXPR_LENGTH_BAD		    	146
#define DW_DLE_MULTIPLE_RELOC_IN_EXPR	    	147
#define DW_DLE_ELF_GETIDENT_ERROR	    	148
#define DW_DLE_NO_AT_MIPS_FDE		    	149
#define DW_DLE_NO_CIE_FOR_FDE		    	150
#define DW_DLE_DIE_ABBREV_LIST_NULL	    	151
#define DW_DLE_DEBUG_FUNCNAMES_DUPLICATE    	152
#define DW_DLE_DEBUG_FUNCNAMES_NULL	    	153
#define DW_DLE_DEBUG_FUNCNAMES_VERSION_ERROR    154
#define DW_DLE_DEBUG_FUNCNAMES_LENGTH_BAD       155
#define DW_DLE_FUNC_NULL		    	156
#define DW_DLE_FUNC_CONTEXT_NULL	    	157
#define DW_DLE_DEBUG_TYPENAMES_DUPLICATE    	158
#define DW_DLE_DEBUG_TYPENAMES_NULL	    	159
#define DW_DLE_DEBUG_TYPENAMES_VERSION_ERROR    160
#define DW_DLE_DEBUG_TYPENAMES_LENGTH_BAD       161
#define DW_DLE_TYPE_NULL		    	162
#define DW_DLE_TYPE_CONTEXT_NULL	    	163
#define DW_DLE_DEBUG_VARNAMES_DUPLICATE	    	164
#define DW_DLE_DEBUG_VARNAMES_NULL	    	165
#define DW_DLE_DEBUG_VARNAMES_VERSION_ERROR     166
#define DW_DLE_DEBUG_VARNAMES_LENGTH_BAD        167
#define DW_DLE_VAR_NULL			    	168
#define DW_DLE_VAR_CONTEXT_NULL		    	169
#define DW_DLE_DEBUG_WEAKNAMES_DUPLICATE    	170
#define DW_DLE_DEBUG_WEAKNAMES_NULL	    	171
#define DW_DLE_DEBUG_WEAKNAMES_VERSION_ERROR    172
#define DW_DLE_DEBUG_WEAKNAMES_LENGTH_BAD       173
#define DW_DLE_WEAK_NULL		    	174
#define DW_DLE_WEAK_CONTEXT_NULL	    	175
#define DW_DLE_LOCDESC_COUNT_WRONG              176
#define DW_DLE_MACINFO_STRING_NULL              177
#define DW_DLE_MACINFO_STRING_EMPTY             178
#define DW_DLE_MACINFO_INTERNAL_ERROR_SPACE     179
#define DW_DLE_MACINFO_MALLOC_FAIL              180
#define DW_DLE_DEBUGMACINFO_ERROR		181
#define DW_DLE_DEBUG_MACRO_LENGTH_BAD		182
#define DW_DLE_DEBUG_MACRO_MAX_BAD		183
#define DW_DLE_DEBUG_MACRO_INTERNAL_ERR		184
#define DW_DLE_DEBUG_MACRO_MALLOC_SPACE	        185
#define DW_DLE_DEBUG_MACRO_INCONSISTENT	        186
#define DW_DLE_DF_NO_CIE_AUGMENTATION          	187
#define DW_DLE_DF_REG_NUM_TOO_HIGH  		188 
#define DW_DLE_DF_MAKE_INSTR_NO_INIT          	189 
#define DW_DLE_DF_NEW_LOC_LESS_OLD_LOC         	190
#define DW_DLE_DF_POP_EMPTY_STACK              	191
#define DW_DLE_DF_ALLOC_FAIL                   	192
#define DW_DLE_DF_FRAME_DECODING_ERROR         	193
#define DW_DLE_DEBUG_LOC_SECTION_SHORT         	194
#define DW_DLE_FRAME_AUGMENTATION_UNKNOWN       195
#define DW_DLA_PUBTYPE_CONTEXT                  196
#define DW_DLE_DEBUG_PUBTYPES_LENGTH_BAD        197
#define DW_DLE_DEBUG_PUBTYPES_VERSION_ERROR     198
#define DW_DLE_DEBUG_PUBTYPES_DUPLICATE         199
#define DW_DLE_FRAME_CIE_DECODE_ERROR           200
#define DW_DLE_FRAME_REGISTER_UNREPRESENTABLE   201
#define DW_DLE_NOT_REF_FORM                     202
#define DW_DLE_FORM_SEC_OFFSET_LENGTH_BAD       203



    /* DW_DLE_LAST MUST EQUAL LAST ERROR NUMBER */
#define DW_DLE_LAST        			203
#define DW_DLE_LO_USER     0x10000

        /* taken as meaning 'undefined value', this is not
           a column or register number.
           Only present at libdwarf runtime. Never on disk.
	   DW_FRAME_* Values present on disk are in dwarf.h
        */
#define DW_FRAME_UNDEFINED_VAL          1034

        /* taken as meaning 'same value' as caller had, not a column
           or register number
           Only present at libdwarf runtime. Never on disk.
	   DW_FRAME_* Values present on disk are in dwarf.h
        */
#define DW_FRAME_SAME_VAL               1035



/* error return values  
*/
#define DW_DLV_BADADDR     (~(Dwarf_Addr)0)   
	/* for functions returning target address */

#define DW_DLV_NOCOUNT     ((Dwarf_Signed)-1) 
	/* for functions returning count */

#define DW_DLV_BADOFFSET   (~(Dwarf_Off)0)    
	/* for functions returning offset */

/* standard return values for functions */
#define DW_DLV_NO_ENTRY -1
#define DW_DLV_OK        0
#define DW_DLV_ERROR     1

/* Special values for offset_into_exception_table field of dwarf fde's. */
/* The following value indicates that there is no Exception table offset
   associated with a dwarf frame. */
#define DW_DLX_NO_EH_OFFSET  	   (-1LL)
/* The following value indicates that the producer was unable to analyse the
   source file to generate Exception tables for this function. */
#define DW_DLX_EH_OFFSET_UNAVAILABLE  (-2LL)


/*===========================================================================*/
/*  Dwarf consumer interface initialization and termination operations */

/* non-elf initialization */
int dwarf_init(int 	/*fd*/, 
    Dwarf_Unsigned 	/*access*/, 
    Dwarf_Handler 	/*errhand*/, 
    Dwarf_Ptr 		/*errarg*/, 
    Dwarf_Debug      *  /*dbg*/,
    Dwarf_Error* 	/*error*/);

/* elf intialization */
int dwarf_elf_init(dwarf_elf_handle /*elf*/,
    Dwarf_Unsigned 	/*access*/, 
    Dwarf_Handler 	/*errhand*/, 
    Dwarf_Ptr 		/*errarg*/, 
    Dwarf_Debug      *  /*dbg*/,
    Dwarf_Error* 	/*error*/);

/* Undocumented function for memory allocator. */
void dwarf_print_memory_stats(Dwarf_Debug  /*dbg*/);


int dwarf_get_elf(Dwarf_Debug /*dbg*/,
    dwarf_elf_handle*   /*return_elfptr*/,
    Dwarf_Error*	/*error*/);

int dwarf_finish(Dwarf_Debug /*dbg*/, Dwarf_Error* /*error*/);

/* die traversal operations */
int dwarf_next_cu_header(Dwarf_Debug /*dbg*/, 
    Dwarf_Unsigned* 	/*cu_header_length*/, 
    Dwarf_Half*     	/*version_stamp*/, 
    Dwarf_Off*  	/*abbrev_offset*/, 
    Dwarf_Half* 	/*address_size*/, 
    Dwarf_Unsigned*     /*next_cu_header_offset*/,
    Dwarf_Error* 	/*error*/);

int dwarf_siblingof(Dwarf_Debug /*dbg*/, 
    Dwarf_Die 		/*die*/, 
    Dwarf_Die*          /*return_siblingdie*/,
    Dwarf_Error* 	/*error*/);

int dwarf_child(Dwarf_Die /*die*/, 
    Dwarf_Die*          /*return_childdie*/,
    Dwarf_Error* 	/*error*/);

/* finding die given offset */
int dwarf_offdie(Dwarf_Debug /*dbg*/, 
    Dwarf_Off 		/*offset*/, 
    Dwarf_Die*          /*return_die*/,
    Dwarf_Error* 	/*error*/);

/* higher level functions (Unimplemented) */
int dwarf_pcfile(Dwarf_Debug /*dbg*/, 
    Dwarf_Addr 		/*pc*/, 
    Dwarf_Die*          /*return_die*/,
    Dwarf_Error* 	/*error*/);

/* Unimplemented */
int dwarf_pcsubr(Dwarf_Debug /*dbg*/, 
    Dwarf_Addr 		/*pc*/, 
    Dwarf_Die*          /*return_die*/,
    Dwarf_Error* 	/*error*/);

/* Unimplemented */
int dwarf_pcscope(Dwarf_Debug /*dbg*/, 
    Dwarf_Addr 		/*pc*/, 
    Dwarf_Die*          /*return_die*/,
    Dwarf_Error* 	/*error*/);

/* operations on DIEs */
int dwarf_tag(Dwarf_Die /*die*/, 
    Dwarf_Half*	        /*return_tag*/,
    Dwarf_Error* 	/*error*/);

/* utility? */
int dwarf_dieoffset(Dwarf_Die /*die*/, 
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error* 	/*error*/);

int dwarf_die_CU_offset(Dwarf_Die /*die*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_attr (Dwarf_Die /*die*/, 
    Dwarf_Half 		/*attr*/, 
    Dwarf_Attribute *   /*returned_attr*/,
    Dwarf_Error* 	/*error*/);

int dwarf_diename(Dwarf_Die /*die*/, 
    char   **           /*diename*/,
    Dwarf_Error* 	/*error*/);

/* convenience functions, alternative to using dwarf_attrlist() */
int dwarf_hasattr(Dwarf_Die /*die*/, 
    Dwarf_Half 		/*attr*/, 
    Dwarf_Bool     *    /*returned_bool*/,
    Dwarf_Error* 	/*error*/);

/* dwarf_loclist_n preferred over dwarf_loclist */
int dwarf_loclist_n(Dwarf_Attribute /*attr*/,  
    Dwarf_Locdesc***	/*llbuf*/, 
    Dwarf_Signed *      /*locCount*/,
    Dwarf_Error* 	/*error*/);

int dwarf_loclist(Dwarf_Attribute /*attr*/,  /* inflexible! */
    Dwarf_Locdesc** 	/*llbuf*/, 
    Dwarf_Signed *      /*locCount*/,
    Dwarf_Error* 	/*error*/);

/* Unimplemented */
int dwarf_stringlen(Dwarf_Die /*die*/, 
    Dwarf_Locdesc **    /*returned_locdesc*/,
    Dwarf_Error* 	/*error*/);

/* Unimplemented */
int dwarf_subscrcnt(Dwarf_Die /*die*/, 
    Dwarf_Signed *      /*returned_count*/,
    Dwarf_Error* 	/*error*/);

/* Unimplemented */
int dwarf_nthsubscr(Dwarf_Die /*die*/, 
    Dwarf_Unsigned 	/*ssndx*/, 
    Dwarf_Die *         /*returned_die*/,
    Dwarf_Error* 	/*error*/);

int dwarf_lowpc(Dwarf_Die /*die*/, 
    Dwarf_Addr  *       /*returned_addr*/,
    Dwarf_Error* 	/*error*/);

int dwarf_highpc(Dwarf_Die /*die*/, 
    Dwarf_Addr  *       /*returned_addr*/,
    Dwarf_Error* 	/*error*/);

int dwarf_bytesize(Dwarf_Die /*die*/, 
    Dwarf_Unsigned *    /*returned_size*/,
    Dwarf_Error* 	/*error*/);

/* Unimplemented */
int dwarf_isbitfield(Dwarf_Die /*die*/, 
    Dwarf_Bool  *       /*returned_bool*/,
    Dwarf_Error* 	/*error*/);

int dwarf_bitsize(Dwarf_Die /*die*/, 
    Dwarf_Unsigned *    /*returned_size*/,
    Dwarf_Error* 	/*error*/);

int dwarf_bitoffset(Dwarf_Die /*die*/, 
    Dwarf_Unsigned *    /*returned_offset*/,
    Dwarf_Error* 	/*error*/);

int dwarf_srclang(Dwarf_Die /*die*/, 
    Dwarf_Unsigned *    /*returned_lang*/,
    Dwarf_Error* 	/*error*/);

int dwarf_arrayorder(Dwarf_Die /*die*/, 
    Dwarf_Unsigned *    /*returned_order*/,
    Dwarf_Error* 	/*error*/);

/* end of convenience function list */

/* this is the main interface to attributes of a DIE */
int dwarf_attrlist(Dwarf_Die /*die*/, 
    Dwarf_Attribute** 	/*attrbuf*/, 
    Dwarf_Signed   *    /*attrcount*/,
    Dwarf_Error* 	/*error*/);

/* query operations for attributes */
int dwarf_hasform(Dwarf_Attribute /*attr*/, 
    Dwarf_Half 		/*form*/, 
    Dwarf_Bool *        /*returned_bool*/,
    Dwarf_Error* 	/*error*/);

int dwarf_whatform(Dwarf_Attribute /*attr*/, 
    Dwarf_Half *        /*returned_form*/,
    Dwarf_Error* 	/*error*/);

int dwarf_whatform_direct(Dwarf_Attribute /*attr*/, 
    Dwarf_Half *        /*returned_form*/,
    Dwarf_Error* 	/*error*/);

int dwarf_whatattr(Dwarf_Attribute /*attr*/, 
    Dwarf_Half *        /*returned_attr_num*/,
    Dwarf_Error* 	/*error*/);

/* 
    The following are concerned with the Primary Interface: getting
    the actual data values. One function per 'kind' of FORM.
*/
	/*dwarf_formref returns, thru return_offset, a CU-relative offset
	** and does not allow DW_FORM_ref_addr*/
int dwarf_formref(Dwarf_Attribute /*attr*/, 
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error* 	/*error*/);
	/*dwarf_global_formref returns, thru return_offset, 
	 a debug_info-relative offset and does allow all reference forms*/
int dwarf_global_formref(Dwarf_Attribute /*attr*/, 
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error* 	/*error*/);

int dwarf_formaddr(Dwarf_Attribute /*attr*/, 
    Dwarf_Addr   *      /*returned_addr*/,
    Dwarf_Error* 	/*error*/);

int dwarf_formflag(Dwarf_Attribute /*attr*/,
    Dwarf_Bool *        /*returned_bool*/,
    Dwarf_Error*	/*error*/);

Dwarf_Bool dwarf_formisdata(Dwarf_Attribute attr);
Dwarf_Bool dwarf_formisudata(Dwarf_Attribute attr);

int dwarf_formudata(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned  *   /*returned_val*/,
    Dwarf_Error* 	/*error*/);

int dwarf_formsdata(Dwarf_Attribute 	/*attr*/, 
    Dwarf_Signed  *     /*returned_val*/,
    Dwarf_Error* 	/*error*/);

int dwarf_formblock(Dwarf_Attribute /*attr*/, 
    Dwarf_Block    **   /*returned_block*/,
    Dwarf_Error* 	/*error*/);

int dwarf_formstring(Dwarf_Attribute /*attr*/, 
    char   **           /*returned_string*/,
    Dwarf_Error* 	/*error*/);

/* end attribute query operations. */

/* line number operations */
/* dwarf_srclines  is the normal interface */
int dwarf_srclines(Dwarf_Die /*die*/, 
    Dwarf_Line** 	/*linebuf*/, 
    Dwarf_Signed *      /*linecount*/,
    Dwarf_Error* 	/*error*/);

/* dwarf_srclines_dealloc, created July 2005, is the new
   method for deallocating what dwarf_srclines returns.
   More complete free than using dwarf_dealloc directly. */
void dwarf_srclines_dealloc(Dwarf_Debug /*dbg*/, 
   Dwarf_Line*  /*linebuf*/,
   Dwarf_Signed /*count */);


int dwarf_srcfiles(Dwarf_Die /*die*/, 
    char*** 		/*srcfiles*/, 
    Dwarf_Signed *      /*filecount*/,
    Dwarf_Error* 	/*error*/);

/* Unimplemented. */
int dwarf_dieline(Dwarf_Die /*die*/, 
    Dwarf_Line  *       /*returned_line*/,
    Dwarf_Error *       /*error*/);

int dwarf_linebeginstatement(Dwarf_Line /*line*/, 
    Dwarf_Bool  *       /*returned_bool*/,
    Dwarf_Error* 	/*error*/);

int dwarf_lineendsequence(Dwarf_Line /*line*/,
    Dwarf_Bool  *       /*returned_bool*/,
    Dwarf_Error*        /*error*/);

int dwarf_lineno(Dwarf_Line /*line*/, 
    Dwarf_Unsigned *    /*returned_lineno*/,
    Dwarf_Error* 	/*error*/);

int dwarf_line_srcfileno(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*ret_fileno*/, 
    Dwarf_Error *    /*error*/);

int dwarf_lineaddr(Dwarf_Line /*line*/, 
    Dwarf_Addr *        /*returned_addr*/,
    Dwarf_Error* 	/*error*/);

int dwarf_lineoff(Dwarf_Line /*line*/, 
    Dwarf_Signed  *     /*returned_lineoffset*/,
    Dwarf_Error* 	/*error*/);

int dwarf_linesrc(Dwarf_Line /*line*/, 
    char   **           /*returned_name*/,
    Dwarf_Error* 	/*error*/);

int dwarf_lineblock(Dwarf_Line /*line*/, 
    Dwarf_Bool  *       /*returned_bool*/,
    Dwarf_Error* 	/*error*/);

/* tertiary interface to line info */
/* Unimplemented */
int dwarf_pclines(Dwarf_Debug /*dbg*/, 
    Dwarf_Addr 		/*pc*/, 
    Dwarf_Line** 	/*linebuf*/, 
    Dwarf_Signed *      /*linecount*/,
    Dwarf_Signed 	/*slide*/, 
    Dwarf_Error* 	/*error*/);
/* end line number operations */

/* global name space operations (.debug_pubnames access) */
int dwarf_get_globals(Dwarf_Debug /*dbg*/, 
    Dwarf_Global** 	/*globals*/, 
    Dwarf_Signed *      /*number_of_globals*/,
    Dwarf_Error* 	/*error*/);
void dwarf_globals_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Global* /*globals*/,
    Dwarf_Signed /*number_of_globals*/);

int dwarf_globname(Dwarf_Global /*glob*/, 
    char   **           /*returned_name*/,
    Dwarf_Error* 	/*error*/);

int dwarf_global_die_offset(Dwarf_Global /*global*/, 
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error * 	/*error*/);

int dwarf_get_cu_die_offset_given_cu_header_offset(
	Dwarf_Debug     /*dbg*/,
	Dwarf_Off       /*in_cu_header_offset*/,
        Dwarf_Off *     /*out_cu_die_offset*/, 
	Dwarf_Error *   /*err*/);
#ifdef __sgi /* pragma is sgi MIPS only */
#pragma optional dwarf_get_cu_die_offset_given_cu_header_offset
#endif

int dwarf_global_cu_offset(Dwarf_Global /*global*/, 
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error* 	/*error*/);

int dwarf_global_name_offsets(Dwarf_Global /*global*/, 
    char   **           /*returned_name*/,
    Dwarf_Off* 		/*die_offset*/, 
    Dwarf_Off* 		/*cu_offset*/, 
    Dwarf_Error* 	/*error*/);

/* Static function name operations.  */
int dwarf_get_funcs(Dwarf_Debug	/*dbg*/,
    Dwarf_Func**	/*funcs*/,
    Dwarf_Signed *      /*number_of_funcs*/,
    Dwarf_Error*	/*error*/);
void dwarf_funcs_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Func* /*funcs*/,
    Dwarf_Signed /*number_of_funcs*/);

int dwarf_funcname(Dwarf_Func /*func*/,
    char   **           /*returned_name*/,
    Dwarf_Error*	/*error*/);

int dwarf_func_die_offset(Dwarf_Func /*func*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_func_cu_offset(Dwarf_Func /*func*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_func_name_offsets(Dwarf_Func /*func*/,
    char   **           /*returned_name*/,
    Dwarf_Off*		/*die_offset*/,
    Dwarf_Off*		/*cu_offset*/,
    Dwarf_Error*	/*error*/);

/* User-defined type name operations, SGI IRIX .debug_typenames section.
   Same content as DWARF3 .debug_pubtypes, but defined years before
   .debug_pubtypes was defined.   SGI IRIX only. */
int dwarf_get_types(Dwarf_Debug	/*dbg*/,
    Dwarf_Type**	/*types*/,
    Dwarf_Signed *      /*number_of_types*/,
    Dwarf_Error*	/*error*/);
void dwarf_types_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Type* /*types*/,
    Dwarf_Signed /*number_of_types*/);


int dwarf_typename(Dwarf_Type /*type*/,
    char   **           /*returned_name*/,
    Dwarf_Error*	/*error*/);

int dwarf_type_die_offset(Dwarf_Type /*type*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_type_cu_offset(Dwarf_Type /*type*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_type_name_offsets(Dwarf_Type	/*type*/,
    char   **           /*returned_name*/,
    Dwarf_Off*		/*die_offset*/,
    Dwarf_Off*		/*cu_offset*/,
    Dwarf_Error*	/*error*/);

/* User-defined type name operations, DWARF3  .debug_pubtypes section. 
*/
int dwarf_get_pubtypes(Dwarf_Debug	/*dbg*/,
    Dwarf_Type**	/*types*/,
    Dwarf_Signed *      /*number_of_types*/,
    Dwarf_Error*	/*error*/);
void dwarf_pubtypes_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Type* /*pubtypes*/,
    Dwarf_Signed /*number_of_pubtypes*/);


int dwarf_pubtypename(Dwarf_Type /*type*/,
    char   **           /*returned_name*/,
    Dwarf_Error*	/*error*/);

int dwarf_pubtype_die_offset(Dwarf_Type /*type*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_pubtype_cu_offset(Dwarf_Type /*type*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_pubtype_name_offsets(Dwarf_Type	/*type*/,
    char   **           /*returned_name*/,
    Dwarf_Off*		/*die_offset*/,
    Dwarf_Off*		/*cu_offset*/,
    Dwarf_Error*	/*error*/);

/* File-scope static variable name operations.  */
int dwarf_get_vars(Dwarf_Debug	/*dbg*/,
    Dwarf_Var**		/*vars*/,
    Dwarf_Signed *      /*number_of_vars*/,
    Dwarf_Error*	/*error*/);
void dwarf_vars_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Var* /*vars*/,
    Dwarf_Signed /*number_of_vars*/);


int dwarf_varname(Dwarf_Var /*var*/,
    char   **           /*returned_name*/,
    Dwarf_Error*	/*error*/);

int dwarf_var_die_offset(Dwarf_Var /*var*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_var_cu_offset(Dwarf_Var /*var*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_var_name_offsets(Dwarf_Var /*var*/,
    char   **           /*returned_name*/,
    Dwarf_Off*		/*die_offset*/,
    Dwarf_Off*		/*cu_offset*/,
    Dwarf_Error*	/*error*/);

/* weak name operations.  */
int dwarf_get_weaks(Dwarf_Debug	/*dbg*/,
    Dwarf_Weak**	/*weaks*/,
    Dwarf_Signed *      /*number_of_weaks*/,
    Dwarf_Error*	/*error*/);
void dwarf_weaks_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Weak* /*weaks*/,
    Dwarf_Signed /*number_of_weaks*/);


int dwarf_weakname(Dwarf_Weak /*weak*/,
    char   **           /*returned_name*/,
    Dwarf_Error*	/*error*/);

int dwarf_weak_die_offset(Dwarf_Weak /*weak*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_weak_cu_offset(Dwarf_Weak /*weak*/,
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error*	/*error*/);

int dwarf_weak_name_offsets(Dwarf_Weak	/*weak*/,
    char   **           /*returned_name*/,
    Dwarf_Off*		/*die_offset*/,
    Dwarf_Off*		/*cu_offset*/,
    Dwarf_Error*	/*error*/);

/* location list section operation.  (.debug_loc access) */
/* Unimplemented. */
int dwarf_get_loclist_entry(Dwarf_Debug /*dbg*/, 
    Dwarf_Unsigned 	/*offset*/, 
    Dwarf_Addr* 	/*hipc*/, 
    Dwarf_Addr* 	/*lopc*/, 
    Dwarf_Ptr* 		/*data*/, 
    Dwarf_Unsigned* 	/*entry_len*/, 
    Dwarf_Unsigned* 	/*next_entry*/, 
    Dwarf_Error* 	/*error*/);

/* abbreviation section operations */
int dwarf_get_abbrev(Dwarf_Debug /*dbg*/, 
    Dwarf_Unsigned 	/*offset*/, 
    Dwarf_Abbrev  *     /*returned_abbrev*/,
    Dwarf_Unsigned* 	/*length*/, 
    Dwarf_Unsigned* 	/*attr_count*/, 
    Dwarf_Error* 	/*error*/);

int dwarf_get_abbrev_tag(Dwarf_Abbrev /*abbrev*/, 
    Dwarf_Half*        /*return_tag_number*/,
    Dwarf_Error* 	/*error*/);
int dwarf_get_abbrev_code(Dwarf_Abbrev /*abbrev*/, 
    Dwarf_Unsigned*        /*return_code_number*/,
    Dwarf_Error* 	/*error*/);

int dwarf_get_abbrev_children_flag(Dwarf_Abbrev /*abbrev*/, 
    Dwarf_Signed*        /*return_flag*/,
    Dwarf_Error* 	/*error*/);

int dwarf_get_abbrev_entry(Dwarf_Abbrev /*abbrev*/, 
    Dwarf_Signed  	/*index*/, 
    Dwarf_Half  *       /*returned_attr_num*/,
    Dwarf_Signed* 	/*form*/, 
    Dwarf_Off*    	/*offset*/, 
    Dwarf_Error*  	/*error*/);

/* consumer string section operation */
int dwarf_get_str(Dwarf_Debug /*dbg*/, 
    Dwarf_Off    	/*offset*/, 
    char** 		/*string*/, 
    Dwarf_Signed *      /*strlen_of_string*/,
    Dwarf_Error*  	/*error*/);

/* Consumer op on  gnu .eh_frame info */
int dwarf_get_fde_list_eh(
    Dwarf_Debug        /*dbg*/,
    Dwarf_Cie       ** /*cie_data*/,
    Dwarf_Signed    *  /*cie_element_count*/,
    Dwarf_Fde       ** /*fde_data*/,
    Dwarf_Signed    *  /*fde_element_count*/,
    Dwarf_Error     *  /*error*/);


/* consumer operations on frame info: .debug_frame */
int dwarf_get_fde_list(Dwarf_Debug /*dbg*/, 
    Dwarf_Cie**   	/*cie_data*/, 
    Dwarf_Signed* 	/*cie_element_count*/, 
    Dwarf_Fde**   	/*fde_data*/, 
    Dwarf_Signed* 	/*fde_element_count*/, 
    Dwarf_Error* 	/*error*/);

/* Release storage gotten by dwarf_get_fde_list_eh() or
   dwarf_get_fde_list() */
void dwarf_fde_cie_list_dealloc(Dwarf_Debug dbg,
        Dwarf_Cie *cie_data,
        Dwarf_Signed cie_element_count,
        Dwarf_Fde *fde_data,
        Dwarf_Signed fde_element_count);



int dwarf_get_fde_range(Dwarf_Fde /*fde*/, 
    Dwarf_Addr* 	/*low_pc*/, 
    Dwarf_Unsigned* 	/*func_length*/, 
    Dwarf_Ptr*    	/*fde_bytes*/, 
    Dwarf_Unsigned* 	/*fde_byte_length*/, 
    Dwarf_Off*    	/*cie_offset*/, 
    Dwarf_Signed*  	/*cie_index*/, 
    Dwarf_Off*   	/*fde_offset*/, 
    Dwarf_Error* 	/*error*/);

/*  Useful for IRIX only:  see dwarf_get_cie_augmentation_data()
       dwarf_get_fde_augmentation_data() for GNU .eh_frame. */
int dwarf_get_fde_exception_info(Dwarf_Fde /*fde*/,
    Dwarf_Signed*	/* offset_into_exception_tables */,
    Dwarf_Error*        /*error*/);


int dwarf_get_cie_of_fde(Dwarf_Fde /*fde*/,
    Dwarf_Cie *         /*cie_returned*/,
    Dwarf_Error*        /*error*/);

int dwarf_get_cie_info(Dwarf_Cie /*cie*/, 
    Dwarf_Unsigned *    /*bytes_in_cie*/,
    Dwarf_Small*    	/*version*/, 
    char        **      /*augmenter*/,
    Dwarf_Unsigned* 	/*code_alignment_factor*/, 
    Dwarf_Signed* 	/*data_alignment_factor*/, 
    Dwarf_Half*     	/*return_address_register_rule*/, 
    Dwarf_Ptr*     	/*initial_instructions*/, 
    Dwarf_Unsigned*  	/*initial_instructions_length*/, 
    Dwarf_Error* 	/*error*/);

int dwarf_get_fde_instr_bytes(Dwarf_Fde /*fde*/, 
    Dwarf_Ptr * /*outinstrs*/, Dwarf_Unsigned * /*outlen*/, 
    Dwarf_Error * /*error*/);

int dwarf_get_fde_info_for_all_regs(Dwarf_Fde /*fde*/, 
    Dwarf_Addr          /*pc_requested*/,
    Dwarf_Regtable*     /*reg_table*/,
    Dwarf_Addr*         /*row_pc*/,
    Dwarf_Error*        /*error*/);

int dwarf_get_fde_info_for_all_regs3(Dwarf_Fde /*fde*/, 
    Dwarf_Addr          /*pc_requested*/,
    Dwarf_Regtable3*     /*reg_table*/,
    Dwarf_Addr*         /*row_pc*/,
    Dwarf_Error*        /*error*/);

/* In this older interface DW_FRAME_CFA_COL is a meaningful
    column (which does not work well with DWARF3 or
    non-MIPS architectures). */
int dwarf_get_fde_info_for_reg(Dwarf_Fde /*fde*/, 
    Dwarf_Half    	/*table_column*/, 
    Dwarf_Addr    	/*pc_requested*/, 
    Dwarf_Signed*       /*offset_relevant*/,
    Dwarf_Signed* 	/*register*/,  
    Dwarf_Signed* 	/*offset*/, 
    Dwarf_Addr* 	/*row_pc*/, 
    Dwarf_Error* 	/*error*/);

/* See discussion of dw_value_type, libdwarf.h. 
   Use of DW_FRAME_CFA_COL is not meaningful in this interface.
   Nor is DDW_FRAME_CFA_COL3.
   See dwarf_get_fde_info_for_cfa_reg3().
*/
int dwarf_get_fde_info_for_reg3(Dwarf_Fde /*fde*/, 
    Dwarf_Half    	/*table_column*/, 
    Dwarf_Addr    	/*pc_requested*/, 
    Dwarf_Small  *  	/*value_type*/, 
    Dwarf_Signed *      /*offset_relevant*/,
    Dwarf_Signed* 	/*register*/,  
    Dwarf_Signed* 	/*offset_or_block_len*/, 
    Dwarf_Ptr   *       /*block_ptr */,
    Dwarf_Addr* 	/*row_pc_out*/, 
    Dwarf_Error* 	/*error*/);

/* Use this to get the cfa. */
int dwarf_get_fde_info_for_cfa_reg3(Dwarf_Fde /*fde*/, 
    Dwarf_Addr    	/*pc_requested*/, 
    Dwarf_Small  *  	/*value_type*/, 
    Dwarf_Signed *      /*offset_relevant*/,
    Dwarf_Signed* 	/*register*/,  
    Dwarf_Signed* 	/*offset_or_block_len*/, 
    Dwarf_Ptr   *       /*block_ptr */,
    Dwarf_Addr* 	/*row_pc_out*/, 
    Dwarf_Error* 	/*error*/);

int dwarf_get_fde_for_die(Dwarf_Debug /*dbg*/, 
    Dwarf_Die 		/*subr_die */, 
    Dwarf_Fde  *        /*returned_fde*/,
    Dwarf_Error*	/*error*/);

int dwarf_get_fde_n(Dwarf_Fde* /*fde_data*/, 
    Dwarf_Unsigned 	/*fde_index*/, 
    Dwarf_Fde  *        /*returned_fde*/,
    Dwarf_Error*  	/*error*/);

int dwarf_get_fde_at_pc(Dwarf_Fde* /*fde_data*/, 
    Dwarf_Addr 		/*pc_of_interest*/, 
    Dwarf_Fde  *        /*returned_fde*/,
    Dwarf_Addr* 	/*lopc*/, 
    Dwarf_Addr* 	/*hipc*/, 
    Dwarf_Error* 	/*error*/);

/* GNU .eh_frame augmentation information, raw form, see
   Linux Standard Base Core Specification version 3.0 . */
int dwarf_get_cie_augmentation_data(Dwarf_Cie /* cie*/,
    Dwarf_Small **         /* augdata */,
    Dwarf_Unsigned *    /* augdata_len */,
    Dwarf_Error*        /*error*/);
/* GNU .eh_frame augmentation information, raw form, see
   Linux Standard Base Core Specification version 3.0 . */
int dwarf_get_fde_augmentation_data(Dwarf_Fde /* fde*/,
    Dwarf_Small **         /* augdata */,
    Dwarf_Unsigned *    /* augdata_len */,
    Dwarf_Error*        /*error*/);

int dwarf_expand_frame_instructions(Dwarf_Debug /*dbg*/, 
    Dwarf_Ptr 		/*instruction*/, 
    Dwarf_Unsigned  	/*i_length*/, 
    Dwarf_Frame_Op** 	/*returned_op_list*/, 
    Dwarf_Signed*       /*op_count*/,
    Dwarf_Error* 	/*error*/);

/* Operations on .debug_aranges. */
int dwarf_get_aranges(Dwarf_Debug /*dbg*/, 
    Dwarf_Arange** 	/*aranges*/, 
    Dwarf_Signed *      /*arange_count*/,
    Dwarf_Error* 	/*error*/);



int dwarf_get_arange(
    Dwarf_Arange* 	/*aranges*/, 
    Dwarf_Unsigned 	/*arange_count*/, 
    Dwarf_Addr 		/*address*/, 
    Dwarf_Arange *      /*returned_arange*/,
    Dwarf_Error* 	/*error*/);

int dwarf_get_cu_die_offset(
    Dwarf_Arange 	/*arange*/, 
    Dwarf_Off*          /*return_offset*/,
    Dwarf_Error* 	/*error*/);

int dwarf_get_arange_cu_header_offset(
    Dwarf_Arange 	/*arange*/, 
    Dwarf_Off*          /*return_cu_header_offset*/,
    Dwarf_Error* 	/*error*/);
#ifdef __sgi /* pragma is sgi MIPS only */
#pragma optional dwarf_get_arange_cu_header_offset
#endif

int dwarf_get_arange_info(
    Dwarf_Arange 	/*arange*/, 
    Dwarf_Addr* 	/*start*/, 
    Dwarf_Unsigned* 	/*length*/, 
    Dwarf_Off* 		/*cu_die_offset*/, 
    Dwarf_Error* 	/*error*/);


/* consumer .debug_macinfo information interface.
*/
struct Dwarf_Macro_Details_s {
  Dwarf_Off    dmd_offset; /* offset, in the section,
                              of this macro info */
  Dwarf_Small  dmd_type;   /* the type, DW_MACINFO_define etc*/
  Dwarf_Signed dmd_lineno; /* the source line number where
                              applicable and vend_def # if
                              vendor_extension op
                           */

  Dwarf_Signed dmd_fileindex;/* the source file index:
                              applies to define undef start_file
                             */
  char *       dmd_macro;  /* macro name (with value for defineop)
                              string from vendor ext
                           */
};

/* _dwarf_print_lines is for use by dwarfdump: it prints
   line info to stdout.
*/
int _dwarf_print_lines(Dwarf_Die cu_die,Dwarf_Error * /*error*/);

/* _dwarf_ld_sort_lines is for use solely by ld for
   rearranging lines in .debug_line in a .o created with a text
   section per function.  
		-OPT:procedure_reorder=ON
   where ld-cord (cord(1)ing by ld, 
   not by cord(1)) may have changed the function order.
*/
int _dwarf_ld_sort_lines(
        void * orig_buffer,
        unsigned long   buffer_len,
        int is_64_bit,
        int *any_change,
        int * err_code);

/* Used by dwarfdump -v to print offsets, for debugging
   dwarf info
*/
int _dwarf_fde_section_offset(Dwarf_Debug dbg,Dwarf_Fde in_fde,
        Dwarf_Off *fde_off, Dwarf_Off *cie_off,
        Dwarf_Error *err);

/* Used by dwarfdump -v to print offsets, for debugging
   dwarf info
*/
int _dwarf_cie_section_offset(Dwarf_Debug dbg,Dwarf_Cie in_cie,
        Dwarf_Off *cie_off,
        Dwarf_Error *err);




typedef struct Dwarf_Macro_Details_s Dwarf_Macro_Details;

int dwarf_get_macro(Dwarf_Debug /*dbg*/,
    char *        /*requested_macro_name*/,
    Dwarf_Addr    /*pc_of_request*/,
    char **       /*returned_macro_value*/,
    Dwarf_Error * /*error*/);

int dwarf_get_all_defined_macros(Dwarf_Debug /*dbg*/,
    Dwarf_Addr     /*pc_of_request*/,
    Dwarf_Signed * /*returned_count*/,
    char ***       /*returned_pointers_to_macros*/,
    Dwarf_Error *  /*error*/);

char *dwarf_find_macro_value_start(char * /*macro_string*/);

int dwarf_get_macro_details(Dwarf_Debug /*dbg*/,
    Dwarf_Off              /*macro_offset*/,
    Dwarf_Unsigned	   /*maximum_count*/,
    Dwarf_Signed         * /*entry_count*/,
    Dwarf_Macro_Details ** /*details*/,
    Dwarf_Error *          /*err*/);


int dwarf_get_address_size(Dwarf_Debug /*dbg*/,
        Dwarf_Half  * /*addr_size*/,
        Dwarf_Error * /*error*/);

/* utility operations */
Dwarf_Unsigned dwarf_errno(Dwarf_Error 	/*error*/);

char* dwarf_errmsg(Dwarf_Error	/*error*/);

/* stringcheck zero is default and means do all
** string length validity checks.
** Call with parameter value 1 to turn off many such checks (and
** increase performance).
** Call with zero for safest running.
** Actual value saved and returned is only 8 bits! Upper bits
** ignored by libdwarf (and zero on return).
** Returns previous value.
*/
int dwarf_set_stringcheck(int /*stringcheck*/);

/* Unimplemented */
Dwarf_Handler dwarf_seterrhand(Dwarf_Debug /*dbg*/, Dwarf_Handler /*errhand*/);

/* Unimplemented */
Dwarf_Ptr dwarf_seterrarg(Dwarf_Debug /*dbg*/, Dwarf_Ptr /*errarg*/);

void dwarf_dealloc(Dwarf_Debug /*dbg*/, void* /*space*/, 
    Dwarf_Unsigned /*type*/);

int dwarf_attr_offset(Dwarf_Die /*die*/,
    Dwarf_Attribute /*attr of above die*/,
    Dwarf_Off     * /*returns offset thru this ptr */,
    Dwarf_Error   * /*error*/);

/* This is a hack so clients can verify offsets.
   Added April 2005 so that debugger can detect broken offsets
   (which happened in an IRIX executable larger than 2GB
    with MIPSpro 7.3.1.3 toolchain.).
*/
int
dwarf_get_section_max_offsets(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*debug_info_size*/,
    Dwarf_Unsigned * /*debug_abbrev_size*/,
    Dwarf_Unsigned * /*debug_line_size*/,
    Dwarf_Unsigned * /*debug_loc_size*/,
    Dwarf_Unsigned * /*debug_aranges_size*/,
    Dwarf_Unsigned * /*debug_macinfo_size*/,
    Dwarf_Unsigned * /*debug_pubnames_size*/,
    Dwarf_Unsigned * /*debug_str_size*/,
    Dwarf_Unsigned * /*debug_frame_size*/,
    Dwarf_Unsigned * /*debug_ranges_size*/,
    Dwarf_Unsigned * /*debug_pubtypes_size*/);

Dwarf_Half
dwarf_set_frame_rule_inital_value(Dwarf_Debug /*dbg*/,
	Dwarf_Half /*value*/);

Dwarf_Half
dwarf_set_frame_rule_table_size(Dwarf_Debug dbg,
        Dwarf_Half value);


#ifdef __cplusplus
}
#endif
#endif /* _LIBDWARF_H */

