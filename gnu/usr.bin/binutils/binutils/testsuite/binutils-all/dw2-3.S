/* This testcase is part of GDB, the GNU debugger.

   Copyright (C) 2004-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Test a minimal file containing DWARF-2 information.  This test also
   serves as a skeleton for other DWARF-2 tests.  Most other tests will
   not be this extensively itemized and commented...  */

/* Dummy function to provide debug information for.  */

	.text
.Lbegin_text1:
	.globl func_cu1
	.type func_cu1, %function
func_cu1:
.Lbegin_func_cu1:
	.4byte 0
.Lend_func_cu1:
	.size func_cu1, .-func_cu1
.Lend_text1:

/* Debug information */

	.section .debug_info
.Lcu1_begin:
	/* CU header */
	.4byte	.Lcu1_end - .Lcu1_start		/* Length of Compilation Unit */
.Lcu1_start:
	.2byte	2				/* DWARF Version */
	.4byte	.Labbrev1_begin			/* Offset into abbrev section */
	.byte	4				/* Pointer size */

	/* CU die */
	.uleb128 1				/* Abbrev: DW_TAG_compile_unit */
	.4byte	.Lline1_begin			/* DW_AT_stmt_list */
	.4byte	.Lend_text1			/* DW_AT_high_pc */
	.4byte	.Lbegin_text1			/* DW_AT_low_pc */
	.ascii	"file1.txt\0"			/* DW_AT_name */
	.ascii	"GNU C 3.3.3\0"			/* DW_AT_producer */
	.byte	1				/* DW_AT_language (C) */

	/* func_cu1 */
	.uleb128	2			/* Abbrev: DW_TAG_subprogram */
	.byte		1			/* DW_AT_external */
	.byte		1			/* DW_AT_decl_file */
	.byte		2			/* DW_AT_decl_line */
	.ascii		"func_cu1\0"		/* DW_AT_name */
	.4byte		.Ltype_int2_in_cu2	/* DW_AT_type */
	.4byte		.Lbegin_func_cu1	/* DW_AT_low_pc */
	.4byte		.Lend_func_cu1		/* DW_AT_high_pc */
	.byte		1			/* DW_AT_frame_base: length */
	.byte		0x55			/* DW_AT_frame_base: DW_OP_reg5 */

	/* This type is named "int1" and not "int" to ensure it is unique,
	   and thus we can easily ensure we expand this CU and not some
	   other CU with "int".  */
.Ltype_int1_in_cu1:
	.uleb128	3			/* Abbrev: DW_TAG_base_type */
	.ascii		"int1\0"		/* DW_AT_name */
	.byte		4			/* DW_AT_byte_size */
	.byte		5			/* DW_AT_encoding */

.Ltype_const_int1_in_cu1:
	.uleb128	4			/* Abbrev: DW_TAG_const_type */
	.4byte		 .Ltype_int1_in_cu1-.Lcu1_begin /* DW_AT_type */

	.uleb128	5			/* Abbrev: DW_TAG_variable */
	.ascii		"one\0"			/* DW_AT_name */
	.4byte		.Ltype_const_int1_in_cu1-.Lcu1_begin   /* DW_AT_type */
	.byte		1			/* DW_AT_const_value */

	.byte		0			/* End of children of CU */

.Lcu1_end:

	/* Second compilation unit.  */
.Lcu2_begin:
	/* CU header */
	.4byte	.Lcu2_end - .Lcu2_start		/* Length of Compilation Unit */
.Lcu2_start:
	.2byte	2				/* DWARF Version */
	.4byte	.Labbrev2_begin			/* Offset into abbrev section */
	.byte	4				/* Pointer size */

	/* CU die */
	.uleb128 1				/* Abbrev: DW_TAG_compile_unit */
	.ascii	"file1.txt\0"			/* DW_AT_name */
	.ascii	"GNU C 3.3.3\0"			/* DW_AT_producer */
	.byte	1				/* DW_AT_language (C) */

	/* This type is named "int2" and not "int" to ensure it is unique,
	   and thus we can easily ensure we expand this CU and not some
	   other CU with "int".  */
.Ltype_int2_in_cu2:
	.uleb128	2			/* Abbrev: DW_TAG_base_type */
	.ascii		"int2\0"		/* DW_AT_name */
	.byte		4			/* DW_AT_byte_size */
	.byte		5			/* DW_AT_encoding */

.Ltype_const_int2_in_cu2:
	.uleb128	3			/* Abbrev: DW_TAG_const_type */
	.4byte		 .Ltype_int2_in_cu2-.Lcu2_begin /* DW_AT_type */

	.uleb128	4			/* Abbrev: DW_TAG_variable */
	.ascii		"two\0"			/* DW_AT_name */
	.4byte		.Ltype_const_int2_in_cu2-.Lcu2_begin   /* DW_AT_type */
	.byte		2			/* DW_AT_const_value */

	.byte		0			/* End of children of CU */

.Lcu2_end:

/* Abbrev table */
	.section .debug_abbrev
.Labbrev1_begin:
	.uleb128	1			/* Abbrev code */
	.uleb128	0x11			/* DW_TAG_compile_unit */
	.byte		1			/* has_children */
	.uleb128	0x10			/* DW_AT_stmt_list */
	.uleb128	0x6			/* DW_FORM_data4 */
	.uleb128	0x12			/* DW_AT_high_pc */
	.uleb128	0x1			/* DW_FORM_addr */
	.uleb128	0x11			/* DW_AT_low_pc */
	.uleb128	0x1			/* DW_FORM_addr */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x25			/* DW_AT_producer */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x13			/* DW_AT_language */
	.uleb128	0xb			/* DW_FORM_data1 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.uleb128	2			/* Abbrev code */
	.uleb128	0x2e			/* DW_TAG_subprogram */
	.byte		0			/* has_children */
	.uleb128	0x3f			/* DW_AT_external */
	.uleb128	0xc			/* DW_FORM_flag */
	.uleb128	0x3a			/* DW_AT_decl_file */
	.uleb128	0xb			/* DW_FORM_data1 */
	.uleb128	0x3b			/* DW_AT_decl_line */
	.uleb128	0xb			/* DW_FORM_data1 */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x49			/* DW_AT_type */
	.uleb128	0x10			/* DW_FORM_ref_addr */
	.uleb128	0x11			/* DW_AT_low_pc */
	.uleb128	0x1			/* DW_FORM_addr */
	.uleb128	0x12			/* DW_AT_high_pc */
	.uleb128	0x1			/* DW_FORM_addr */
	.uleb128	0x40			/* DW_AT_frame_base */
	.uleb128	0xa			/* DW_FORM_block1 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.uleb128	3			/* Abbrev code */
	.uleb128	0x24			/* DW_TAG_base_type */
	.byte		0			/* has_children */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0xb			/* DW_AT_byte_size */
	.uleb128	0xb			/* DW_FORM_data1 */
	.uleb128	0x3e			/* DW_AT_encoding */
	.uleb128	0xb			/* DW_FORM_data1 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.uleb128	4			/* Abbrev code */
	.uleb128	0x26			/* DW_TAG_const_type */
	.byte		0x0			/* DW_children_no */
	.uleb128	0x49			/* DW_AT_type */
	.uleb128	0x13			/* DW_FORM_ref4 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.uleb128	5			/* Abbrev code */
	.uleb128	0x34			/* DW_TAG_variable */
	.byte		0x0			/* DW_children_no */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x49			/* DW_AT_type */
	.uleb128	0x13			/* DW_FORM_ref4 */
	.uleb128	0x1c			/* DW_AT_const_value */
	.uleb128	0xb			/* DW_FORM_data1 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

.Labbrev2_begin:
	.uleb128	1			/* Abbrev code */
	.uleb128	0x11			/* DW_TAG_compile_unit */
	.byte		1			/* has_children */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x25			/* DW_AT_producer */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x13			/* DW_AT_language */
	.uleb128	0xb			/* DW_FORM_data1 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.uleb128	2			/* Abbrev code */
	.uleb128	0x24			/* DW_TAG_base_type */
	.byte		0			/* has_children */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0xb			/* DW_AT_byte_size */
	.uleb128	0xb			/* DW_FORM_data1 */
	.uleb128	0x3e			/* DW_AT_encoding */
	.uleb128	0xb			/* DW_FORM_data1 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.uleb128	3			/* Abbrev code */
	.uleb128	0x26			/* DW_TAG_const_type */
	.byte		0x0			/* DW_children_no */
	.uleb128	0x49			/* DW_AT_type */
	.uleb128	0x13			/* DW_FORM_ref4 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.uleb128	4			/* Abbrev code */
	.uleb128	0x34			/* DW_TAG_variable */
	.byte		0x0			/* DW_children_no */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x49			/* DW_AT_type */
	.uleb128	0x13			/* DW_FORM_ref4 */
	.uleb128	0x1c			/* DW_AT_const_value */
	.uleb128	0xb			/* DW_FORM_data1 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

/* Line table */
	.section .debug_line
.Lline1_begin:
	.4byte		.Lline1_end - .Lline1_start	/* Initial length */
.Lline1_start:
	.2byte		2			/* Version */
	.4byte		.Lline1_lines - .Lline1_hdr	/* header_length */
.Lline1_hdr:
	.byte		1			/* Minimum insn length */
	.byte		1			/* default_is_stmt */
	.byte		1			/* line_base */
 	.byte		1			/* line_range */
	.byte		0x10			/* opcode_base */

	/* Standard lengths */
	.byte		0
	.byte		1
	.byte		1
	.byte		1
	.byte		1
	.byte		0
	.byte		0
	.byte		0
	.byte		1
	.byte		0
	.byte		0
	.byte		1
	.byte		0
	.byte		0
	.byte		0

	/* Include directories */
	.byte		0

	/* File names */
	.ascii		"file1.txt\0"
	.uleb128	0
	.uleb128	0
	.uleb128	0

	.byte		0

.Lline1_lines:
	.byte		0	/* DW_LNE_set_address */
	.uleb128	5
	.byte		2
	.4byte		.Lbegin_func_cu1

	.byte		3	/* DW_LNS_advance_line */
	.sleb128	3	/* ... to 4 */

	.byte		1	/* DW_LNS_copy */

	.byte		1	/* DW_LNS_copy (second time as an end-of-prologue marker) */

	.byte		0	/* DW_LNE_set_address */
	.uleb128	5
	.byte		2
	.4byte		.Lend_func_cu1

	.byte		0	/* DW_LNE_end_of_sequence */
	.uleb128	1
	.byte		1

.Lline1_end:
