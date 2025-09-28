/* This testcase is derived from a similar test in GDB.

   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* This tests that gdb can read compressed sections.  The contents
   are a basic assembly file, but the .debug_abbrev section has been
   comrpessed using zlib.  */

/* Dummy function to provide debug information for.  */

	.text
	.globl _start
_start:
	.int 0
.Lbegin_text1:
	.globl func_cu1
	.type func_cu1, %function
func_cu1:
.Lbegin_func_cu1:
	.int 0
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
	.4byte		.Ltype_int-.Lcu1_begin	/* DW_AT_type */
	.4byte		.Lbegin_func_cu1	/* DW_AT_low_pc */
	.4byte		.Lend_func_cu1		/* DW_AT_high_pc */
	.byte		1			/* DW_AT_frame_base: length */
	.byte		0x55			/* DW_AT_frame_base: DW_OP_reg5 */

.Ltype_int:
	.uleb128	3			/* Abbrev: DW_TAG_base_type */
	.ascii		"int\0"			/* DW_AT_name */
	.byte		4			/* DW_AT_byte_size */
	.byte		5			/* DW_AT_encoding */

	.byte		0			/* End of children of CU */

.Lcu1_end:

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

/* Abbrev table -- compressed */
	.section .zdebug_abbrev
.Labbrev1_begin:
	.ascii		"ZLIB"
	.4byte		0
	.2byte		0
	.byte		0
	.byte		51
	.byte		0x78
	.byte		0x5e
	.byte		0x63
	.byte		0x14
	.byte		0x64
	.byte		0x14
	.byte		0x60
	.byte		0x13
	.byte		0x62
	.byte		0x14
	.byte		0x64
	.byte		0x64
	.byte		0xe6
	.byte		0x50
	.byte		0xe5
	.byte		0x10
	.byte		0xe6
	.byte		0x66
	.byte		0x60
	.byte		0x60
	.byte		0xd2
	.byte		0x63
	.byte		0xb0
	.byte		0xe7
	.byte		0xb1
	.byte		0xe2
	.byte		0xb6
	.byte		0xe6
	.byte		0x66
	.byte		0xe6
	.byte		0xf0
	.byte		0x14
	.byte		0x16
	.byte		0x64
	.byte		0x14
	.byte		0x62
	.byte		0x74
	.byte		0xe0
	.byte		0x02
	.byte		0x00
	.byte		0x25
	.byte		0x78
	.byte		0x02
	.byte		0x81
	.byte		0x78
	.byte		0x9c
	.byte		0x63
	.byte		0x60
	.byte		0x60
	.byte		0x56
	.byte		0x61
	.byte		0x60
	.byte		0xe6
	.byte		0xe0
	.byte		0xe6
	.byte		0xb6
	.byte		0xe3
	.byte		0x66
	.byte		0x00
	.byte		0x02
	.byte		0x00
	.byte		0x04
	.byte		0x9c
	.byte		0x00
	.byte		0x92
