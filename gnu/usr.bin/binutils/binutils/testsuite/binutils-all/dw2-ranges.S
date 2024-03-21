/* Copyright (C) 2015-2023 Free Software Foundation, Inc.

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

/* This tests makes use of the .debug_ranges section, especially,
   making sure that the base address encoding scheme is used.  */

/* Dummy function to provide debug information for.  */

	.text
	.globl _start
_start:
	.4byte 0
.Lbegin_text1:
	.globl func_cu1
	.type func_cu1, %function
func_cu1:
.Lbegin_func_cu1:
	.4byte 0
.Lend_func_cu1:
	.size func_cu1, .-func_cu1
.Lend_text1:

.Lbegin_text2:
	.globl func_cu2
	.type func_cu2, %function
func_cu2:
.Lbegin_func_cu2:
	.4byte 0
.Lend_func_cu2:
	.size func_cu2, .-func_cu2
.Lend_text2:

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
        .4byte  .Lrange1_begin
	.ascii	"file1.c\0"			/* DW_AT_name */
	.byte	1				/* DW_AT_language (C) */

	/* func_cu1 */
	.uleb128	2			/* Abbrev: DW_TAG_subprogram */
	.ascii		"func_cu1\0"		/* DW_AT_name */
	.4byte		.Ltype_int-.Lcu1_begin	/* DW_AT_type */
	.4byte		.Lbegin_func_cu1	/* DW_AT_low_pc */
	.4byte		.Lend_func_cu1		/* DW_AT_high_pc */

	/* func_cu1 */
	.uleb128	2			/* Abbrev: DW_TAG_subprogram */
	.ascii		"func_cu2\0"		/* DW_AT_name */
	.4byte		.Ltype_int-.Lcu1_begin	/* DW_AT_type */
	.4byte		.Lbegin_func_cu2	/* DW_AT_low_pc */
	.4byte		.Lend_func_cu2		/* DW_AT_high_pc */

.Ltype_int:
	.uleb128	3			/* Abbrev: DW_TAG_base_type */
	.ascii		"int\0"			/* DW_AT_name */
	.byte		4			/* DW_AT_byte_size */
	.byte		5			/* DW_AT_encoding */

	.byte		0			/* End of children of CU */

.Lcu1_end:

        .section .debug_ranges
.Lrange1_begin:
        .4byte  0xffffffff                      /* base address marker */
	.4byte	.Lbegin_text1                   /* base address */
        .4byte  0                               /* start offset */
	.4byte	.Lend_text1 - .Lbegin_text1     /* end offset */
        .4byte  0xffffffff                      /* base address marker */
	.4byte	.Lbegin_text2                   /* base address */
        .4byte  0                               /* start offset */
	.4byte	.Lend_text2 - .Lbegin_text2     /* end offset */
        .4byte  0                               /* End marker (Part 1) */
        .4byte  0                               /* End marker (Part 2) */

	.section .debug_abbrev
.Labbrev1_begin:
	.uleb128	1			/* Abbrev code */
	.uleb128	0x11			/* DW_TAG_compile_unit */
	.byte		1			/* has_children */
        .uleb128	0x55			/* DW_AT_ranges */
	.uleb128	0x17			/* DW_FORM_sec_offset */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x13			/* DW_AT_language */
	.uleb128	0xb			/* DW_FORM_data1 */
	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */

	.uleb128	2			/* Abbrev code */
	.uleb128	0x2e			/* DW_TAG_subprogram */
	.byte		0			/* has_children */
	.uleb128	0x3			/* DW_AT_name */
	.uleb128	0x8			/* DW_FORM_string */
	.uleb128	0x49			/* DW_AT_type */
	.uleb128	0x13			/* DW_FORM_ref4 */
	.uleb128	0x11			/* DW_AT_low_pc */
	.uleb128	0x1			/* DW_FORM_addr */
	.uleb128	0x12			/* DW_AT_high_pc */
	.uleb128	0x1			/* DW_FORM_addr */
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

	.byte		0x0			/* Terminator */
	.byte		0x0			/* Terminator */
