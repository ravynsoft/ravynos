/* Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

	.file	"main.c"
	.text
.Ltext0:
	.p2align 4,,15
	.globl	func
	.type	func, %function
func:
.LFB0:
	/* main.c:5 */
.LM1:
/* BLOCK 2 freq:10000 seq:0 */
/* PRED: ENTRY [100.0%]  (FALLTHRU) */
	/* main.c:5 */
.LM2:
	.dc.b 0
/* SUCC: */
	.dc.b 0
.LFE0:
	.size	func, .-func
	.section	.text.startup,"ax",%progbits
	.p2align 4,,15
	.globl	main
	.type	main, %function
main:
.LFB1:
	/* main.c:6 */
.LM3:
.LVL0:
/* BLOCK 2 freq:10000 seq:0 */
/* PRED: ENTRY [100.0%]  (FALLTHRU) */
	.dc.b 0
	/* main.c:6 */
.LM4:
	.dc.b 0
.LVL1:
	.dc.b 0
.LVL2:
	.dc.b 0
/* SUCC: EXIT [100.0%]  */
	.dc.b 0
.LFE1:
	.size	main, .-main
 .ifdef HPUX
pvar	.comm	8
yvar	.comm	4
 .else
	.comm	pvar,8,8
	.comm	yvar,4,4
 .endif
	.globl	xvar
	.data
	.align 4
	.type	xvar, %object
	.size	xvar, 4
xvar:
	.4byte	42
	.text
.Letext0:
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x160	/* Length of Compilation Unit Info */
	.2byte	0x5	/* DWARF version number */
	.byte	0x1	/* DW_UT_compile */
	.byte	0x8	/* Pointer Size (in bytes) */
	.4byte	.Ldebug_abbrev0	/* Offset Into Abbrev. Section */
	.uleb128 0x6	/* (DIE (0xc) DW_TAG_compile_unit) */
	.4byte	.LASF21	/* DW_AT_producer: "GNU C11 7.0.1 20170218 (experimental) -mtune=generic -march=x86-64 -gdwarf-5 -O2" */
	.byte	0x1d	/* DW_AT_language */
	.4byte	.LASF0	/* DW_AT_name: "main.c" */
	.4byte	.LASF1	/* DW_AT_comp_dir: "" */
	.4byte	.LLRL2	/* DW_AT_ranges */
	.8byte	0	/* DW_AT_low_pc */
	.4byte	.Ldebug_line0	/* DW_AT_stmt_list */
	.uleb128 0x1	/* (DIE (0x2a) DW_TAG_base_type) */
	.byte	0x1	/* DW_AT_byte_size */
	.byte	0x8	/* DW_AT_encoding */
	.4byte	.LASF2	/* DW_AT_name: "unsigned char" */
	.uleb128 0x1	/* (DIE (0x31) DW_TAG_base_type) */
	.byte	0x2	/* DW_AT_byte_size */
	.byte	0x7	/* DW_AT_encoding */
	.4byte	.LASF3	/* DW_AT_name: "short unsigned int" */
	.uleb128 0x1	/* (DIE (0x38) DW_TAG_base_type) */
	.byte	0x4	/* DW_AT_byte_size */
	.byte	0x7	/* DW_AT_encoding */
	.4byte	.LASF4	/* DW_AT_name: "unsigned int" */
	.uleb128 0x1	/* (DIE (0x3f) DW_TAG_base_type) */
	.byte	0x8	/* DW_AT_byte_size */
	.byte	0x7	/* DW_AT_encoding */
	.4byte	.LASF5	/* DW_AT_name: "long unsigned int" */
	.uleb128 0x1	/* (DIE (0x46) DW_TAG_base_type) */
	.byte	0x1	/* DW_AT_byte_size */
	.byte	0x6	/* DW_AT_encoding */
	.4byte	.LASF6	/* DW_AT_name: "signed char" */
	.uleb128 0x1	/* (DIE (0x4d) DW_TAG_base_type) */
	.byte	0x2	/* DW_AT_byte_size */
	.byte	0x5	/* DW_AT_encoding */
	.4byte	.LASF7	/* DW_AT_name: "short int" */
	.uleb128 0x7	/* (DIE (0x54) DW_TAG_base_type) */
	.byte	0x4	/* DW_AT_byte_size */
	.byte	0x5	/* DW_AT_encoding */
	.ascii "int\0"	/* DW_AT_name */
	.uleb128 0x1	/* (DIE (0x5b) DW_TAG_base_type) */
	.byte	0x8	/* DW_AT_byte_size */
	.byte	0x5	/* DW_AT_encoding */
	.4byte	.LASF8	/* DW_AT_name: "long int" */
	.uleb128 0x1	/* (DIE (0x62) DW_TAG_base_type) */
	.byte	0x8	/* DW_AT_byte_size */
	.byte	0x7	/* DW_AT_encoding */
	.4byte	.LASF9	/* DW_AT_name: "sizetype" */
	.uleb128 0x3	/* (DIE (0x69) DW_TAG_pointer_type) */
			/* DW_AT_byte_size (0x8) */
	.4byte	0x6e	/* DW_AT_type */
	.uleb128 0x1	/* (DIE (0x6e) DW_TAG_base_type) */
	.byte	0x1	/* DW_AT_byte_size */
	.byte	0x6	/* DW_AT_encoding */
	.4byte	.LASF10	/* DW_AT_name: "char" */
	.uleb128 0x8	/* (DIE (0x75) DW_TAG_variable) */
	.4byte	.LASF11	/* DW_AT_name: "__environ" */
	.byte	0x2	/* DW_AT_decl_file (/usr/include/unistd.h) */
	.2byte	0x222	/* DW_AT_decl_line */
	.4byte	0x81	/* DW_AT_type */
			/* DW_AT_external */
			/* DW_AT_declaration */
	.uleb128 0x3	/* (DIE (0x81) DW_TAG_pointer_type) */
			/* DW_AT_byte_size (0x8) */
	.4byte	0x69	/* DW_AT_type */
	.uleb128 0x2	/* (DIE (0x86) DW_TAG_variable) */
	.4byte	.LASF12	/* DW_AT_name: "optarg" */
			/* DW_AT_decl_file (3, /usr/include/getopt.h) */
	.byte	0x39	/* DW_AT_decl_line */
	.4byte	0x69	/* DW_AT_type */
			/* DW_AT_external */
			/* DW_AT_declaration */
	.uleb128 0x2	/* (DIE (0x90) DW_TAG_variable) */
	.4byte	.LASF13	/* DW_AT_name: "optind" */
			/* DW_AT_decl_file (3, /usr/include/getopt.h) */
	.byte	0x47	/* DW_AT_decl_line */
	.4byte	0x54	/* DW_AT_type */
			/* DW_AT_external */
			/* DW_AT_declaration */
	.uleb128 0x2	/* (DIE (0x9a) DW_TAG_variable) */
	.4byte	.LASF14	/* DW_AT_name: "opterr" */
			/* DW_AT_decl_file (3, /usr/include/getopt.h) */
	.byte	0x4c	/* DW_AT_decl_line */
	.4byte	0x54	/* DW_AT_type */
			/* DW_AT_external */
			/* DW_AT_declaration */
	.uleb128 0x2	/* (DIE (0xa4) DW_TAG_variable) */
	.4byte	.LASF15	/* DW_AT_name: "optopt" */
			/* DW_AT_decl_file (3, /usr/include/getopt.h) */
	.byte	0x50	/* DW_AT_decl_line */
	.4byte	0x54	/* DW_AT_type */
			/* DW_AT_external */
			/* DW_AT_declaration */
	.uleb128 0x4	/* (DIE (0xae) DW_TAG_variable) */
	.4byte	.LASF16	/* DW_AT_name: "xvar" */
			/* DW_AT_decl_file (1, main.c) */
	.byte	0x2	/* DW_AT_decl_line */
	.4byte	0x54	/* DW_AT_type */
			/* DW_AT_external */
	.uleb128 0x9	/* DW_AT_location */
	.byte	0x3	/* DW_OP_addr */
	.8byte	0x1234
	.uleb128 0x4	/* (DIE (0xc2) DW_TAG_variable) */
	.4byte	.LASF17	/* DW_AT_name: "yvar" */
			/* DW_AT_decl_file (1, main.c) */
	.byte	0x3	/* DW_AT_decl_line */
	.4byte	0x54	/* DW_AT_type */
			/* DW_AT_external */
	.uleb128 0x9	/* DW_AT_location */
	.byte	0x3	/* DW_OP_addr */
	.8byte	0x1234
	.uleb128 0x4	/* (DIE (0xd6) DW_TAG_variable) */
	.4byte	.LASF18	/* DW_AT_name: "pvar" */
			/* DW_AT_decl_file (1, main.c) */
	.byte	0x4	/* DW_AT_decl_line */
	.4byte	0xea	/* DW_AT_type */
			/* DW_AT_external */
	.uleb128 0x9	/* DW_AT_location */
	.byte	0x3	/* DW_OP_addr */
	.8byte	0x1234
	.uleb128 0x3	/* (DIE (0xea) DW_TAG_pointer_type) */
			/* DW_AT_byte_size (0x8) */
	.4byte	0x54	/* DW_AT_type */
	.uleb128 0x9	/* (DIE (0xef) DW_TAG_subprogram) */
			/* DW_AT_external */
	.4byte	.LASF22	/* DW_AT_name: "main" */
	.byte	0x1	/* DW_AT_decl_file (main.c) */
	.byte	0x6	/* DW_AT_decl_line */
			/* DW_AT_prototyped */
	.4byte	0x54	/* DW_AT_type */
	.8byte	0x1234	/* DW_AT_low_pc */
	.8byte	0x5678	/* DW_AT_high_pc */
	.uleb128 0x1	/* DW_AT_frame_base */
	.byte	0x9c	/* DW_OP_call_frame_cfa */
			/* DW_AT_call_all_calls */
	.4byte	0x13e	/* DW_AT_sibling */
	.uleb128 0x5	/* (DIE (0x110) DW_TAG_formal_parameter) */
	.4byte	.LASF19	/* DW_AT_name: "argc" */
			/* DW_AT_decl_file (1, main.c) */
			/* DW_AT_decl_line (0x6) */
	.4byte	0x54	/* DW_AT_type */
	.4byte	.LLST0	/* DW_AT_location */
	.uleb128 0x5	/* (DIE (0x11d) DW_TAG_formal_parameter) */
	.4byte	.LASF20	/* DW_AT_name: "argv" */
			/* DW_AT_decl_file (1, main.c) */
			/* DW_AT_decl_line (0x6) */
	.4byte	0x81	/* DW_AT_type */
	.4byte	.LLST1	/* DW_AT_location */
	.uleb128 0xa	/* (DIE (0x12a) DW_TAG_call_site) */
	.8byte	0x12345	/* DW_AT_call_return_pc */
	.4byte	0x157	/* DW_AT_call_origin */
	.uleb128 0xb	/* (DIE (0x137) DW_TAG_call_site_parameter) */
	.uleb128 0x1	/* DW_AT_location */
	.byte	0x55	/* DW_OP_reg5 */
	.uleb128 0x1	/* DW_AT_call_value */
	.byte	0x30	/* DW_OP_lit0 */
	.byte	0	/* end of children of DIE 0x12a */
	.byte	0	/* end of children of DIE 0xef */
	.uleb128 0xc	/* (DIE (0x13e) DW_TAG_subprogram) */
			/* DW_AT_external */
	.4byte	.LASF23	/* DW_AT_name: "func" */
	.byte	0x1	/* DW_AT_decl_file (main.c) */
	.byte	0x5	/* DW_AT_decl_line */
			/* DW_AT_prototyped */
	.8byte	0x1234	/* DW_AT_low_pc */
	.8byte	0x5678	/* DW_AT_high_pc */
	.uleb128 0x1	/* DW_AT_frame_base */
	.byte	0x9c	/* DW_OP_call_frame_cfa */
			/* DW_AT_call_all_calls */
	.uleb128 0xd	/* (DIE (0x157) DW_TAG_subprogram) */
			/* DW_AT_external */
			/* DW_AT_declaration */
	.4byte	.LASF24	/* DW_AT_linkage_name: "alarm" */
	.4byte	.LASF24	/* DW_AT_name: "alarm" */
	.byte	0x2	/* DW_AT_decl_file (/usr/include/unistd.h) */
	.2byte	0x1b3	/* DW_AT_decl_line */
	.byte	0	/* end of children of DIE 0xc */
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1	/* (abbrev code) */
	.uleb128 0x24	/* (TAG: DW_TAG_base_type) */
	.byte	0	/* DW_children_no */
	.uleb128 0xb	/* (DW_AT_byte_size) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3e	/* (DW_AT_encoding) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.byte	0
	.byte	0
	.uleb128 0x2	/* (abbrev code) */
	.uleb128 0x34	/* (TAG: DW_TAG_variable) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0x21	/* (DW_FORM_implicit_const) */
	.sleb128 3	/* (/usr/include/getopt.h) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x3f	/* (DW_AT_external) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x3c	/* (DW_AT_declaration) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.byte	0
	.byte	0
	.uleb128 0x3	/* (abbrev code) */
	.uleb128 0xf	/* (TAG: DW_TAG_pointer_type) */
	.byte	0	/* DW_children_no */
	.uleb128 0xb	/* (DW_AT_byte_size) */
	.uleb128 0x21	/* (DW_FORM_implicit_const) */
	.sleb128 8
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.byte	0
	.byte	0
	.uleb128 0x4	/* (abbrev code) */
	.uleb128 0x34	/* (TAG: DW_TAG_variable) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0x21	/* (DW_FORM_implicit_const) */
	.sleb128 1	/* (main.c) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x3f	/* (DW_AT_external) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x2	/* (DW_AT_location) */
	.uleb128 0x18	/* (DW_FORM_exprloc) */
	.byte	0
	.byte	0
	.uleb128 0x5	/* (abbrev code) */
	.uleb128 0x5	/* (TAG: DW_TAG_formal_parameter) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0x21	/* (DW_FORM_implicit_const) */
	.sleb128 1	/* (main.c) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0x21	/* (DW_FORM_implicit_const) */
	.sleb128 6
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x2	/* (DW_AT_location) */
	.uleb128 0x17	/* (DW_FORM_sec_offset) */
	.byte	0
	.byte	0
	.uleb128 0x6	/* (abbrev code) */
	.uleb128 0x11	/* (TAG: DW_TAG_compile_unit) */
	.byte	0x1	/* DW_children_yes */
	.uleb128 0x25	/* (DW_AT_producer) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x13	/* (DW_AT_language) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0x1f	/* (DW_FORM_line_strp) */
	.uleb128 0x1b	/* (DW_AT_comp_dir) */
	.uleb128 0x1f	/* (DW_FORM_line_strp) */
	.uleb128 0x55	/* (DW_AT_ranges) */
	.uleb128 0x17	/* (DW_FORM_sec_offset) */
	.uleb128 0x11	/* (DW_AT_low_pc) */
	.uleb128 0x1	/* (DW_FORM_addr) */
	.uleb128 0x10	/* (DW_AT_stmt_list) */
	.uleb128 0x17	/* (DW_FORM_sec_offset) */
	.byte	0
	.byte	0
	.uleb128 0x7	/* (abbrev code) */
	.uleb128 0x24	/* (TAG: DW_TAG_base_type) */
	.byte	0	/* DW_children_no */
	.uleb128 0xb	/* (DW_AT_byte_size) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3e	/* (DW_AT_encoding) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.byte	0
	.byte	0
	.uleb128 0x8	/* (abbrev code) */
	.uleb128 0x34	/* (TAG: DW_TAG_variable) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0x5	/* (DW_FORM_data2) */
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x3f	/* (DW_AT_external) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x3c	/* (DW_AT_declaration) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.byte	0
	.byte	0
	.uleb128 0x9	/* (abbrev code) */
	.uleb128 0x2e	/* (TAG: DW_TAG_subprogram) */
	.byte	0x1	/* DW_children_yes */
	.uleb128 0x3f	/* (DW_AT_external) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x27	/* (DW_AT_prototyped) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x11	/* (DW_AT_low_pc) */
	.uleb128 0x1	/* (DW_FORM_addr) */
	.uleb128 0x12	/* (DW_AT_high_pc) */
	.uleb128 0x7	/* (DW_FORM_data8) */
	.uleb128 0x40	/* (DW_AT_frame_base) */
	.uleb128 0x18	/* (DW_FORM_exprloc) */
	.uleb128 0x7a	/* (DW_AT_call_all_calls) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x1	/* (DW_AT_sibling) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.byte	0
	.byte	0
	.uleb128 0xa	/* (abbrev code) */
	.uleb128 0x48	/* (TAG: DW_TAG_call_site) */
	.byte	0x1	/* DW_children_yes */
	.uleb128 0x7d	/* (DW_AT_call_return_pc) */
	.uleb128 0x1	/* (DW_FORM_addr) */
	.uleb128 0x7f	/* (DW_AT_call_origin) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.byte	0
	.byte	0
	.uleb128 0xb	/* (abbrev code) */
	.uleb128 0x49	/* (TAG: DW_TAG_call_site_parameter) */
	.byte	0	/* DW_children_no */
	.uleb128 0x2	/* (DW_AT_location) */
	.uleb128 0x18	/* (DW_FORM_exprloc) */
	.uleb128 0x7e	/* (DW_AT_call_value) */
	.uleb128 0x18	/* (DW_FORM_exprloc) */
	.byte	0
	.byte	0
	.uleb128 0xc	/* (abbrev code) */
	.uleb128 0x2e	/* (TAG: DW_TAG_subprogram) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3f	/* (DW_AT_external) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x27	/* (DW_AT_prototyped) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x11	/* (DW_AT_low_pc) */
	.uleb128 0x1	/* (DW_FORM_addr) */
	.uleb128 0x12	/* (DW_AT_high_pc) */
	.uleb128 0x7	/* (DW_FORM_data8) */
	.uleb128 0x40	/* (DW_AT_frame_base) */
	.uleb128 0x18	/* (DW_FORM_exprloc) */
	.uleb128 0x7a	/* (DW_AT_call_all_calls) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.byte	0
	.byte	0
	.uleb128 0xd	/* (abbrev code) */
	.uleb128 0x2e	/* (TAG: DW_TAG_subprogram) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3f	/* (DW_AT_external) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x3c	/* (DW_AT_declaration) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x6e	/* (DW_AT_linkage_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0xe	/* (DW_FORM_strp) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0x5	/* (DW_FORM_data2) */
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loclists,"",%progbits
	.4byte	.Ldebug_loc2-.Ldebug_loc1	/* Length of Location Lists */
.Ldebug_loc1:
	.2byte	0x5	/* DWARF Version */
	.byte	0x8	/* Address Size */
	.byte	0	/* Segment Size */
	.4byte	0	/* Offset Entry Count */
.Ldebug_loc0:
.LLST0:
	.byte	0x6	/* DW_LLE_base_address (*.LLST0) */
	.8byte	0x1234	/* Base address (*.LLST0) */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST0) */
	.uleb128 .LVL0-.LVL0	/* Location list begin address (*.LLST0) */
	.uleb128 .LVL1-.LVL0	/* Location list end address (*.LLST0) */
	.uleb128 0x1	/* Location expression size */
	.byte	0x55	/* DW_OP_reg5 */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST0) */
	.uleb128 .LVL1-.LVL0	/* Location list begin address (*.LLST0) */
	.uleb128 .LFE1-.LVL0	/* Location list end address (*.LLST0) */
	.uleb128 0x4	/* Location expression size */
	.byte	0xa3	/* DW_OP_entry_value */
	.uleb128 0x1
	.byte	0x55	/* DW_OP_reg5 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0	/* DW_LLE_end_of_list (*.LLST0) */
.LLST1:
	.byte	0x6	/* DW_LLE_base_address (*.LLST1) */
	.8byte	0x1234	/* Base address (*.LLST1) */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST1) */
	.uleb128 .LVL0-.LVL0	/* Location list begin address (*.LLST1) */
	.uleb128 .LVL2-1-.LVL0	/* Location list end address (*.LLST1) */
	.uleb128 0x1	/* Location expression size */
	.byte	0x54	/* DW_OP_reg4 */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST1) */
	.uleb128 .LVL2-1-.LVL0	/* Location list begin address (*.LLST1) */
	.uleb128 .LFE1-.LVL0	/* Location list end address (*.LLST1) */
	.uleb128 0x4	/* Location expression size */
	.byte	0xa3	/* DW_OP_entry_value */
	.uleb128 0x1
	.byte	0x54	/* DW_OP_reg4 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0	/* DW_LLE_end_of_list (*.LLST1) */
.Ldebug_loc2:
	.section	.debug_aranges,"",%progbits
	.4byte	0x3c	/* Length of Address Ranges Info */
	.2byte	0x2	/* DWARF Version */
	.4byte	.Ldebug_info0	/* Offset of Compilation Unit Info */
	.byte	0x8	/* Size of Address */
	.byte	0	/* Size of Segment Descriptor */
	.2byte	0	/* Pad to 16 byte boundary */
	.2byte	0
	.8byte	0x1234	/* Address */
	.8byte	0x4567	/* Length */
	.8byte	0x1234	/* Address */
	.8byte	0x5678	/* Length */
	.8byte	0
	.8byte	0
	.section	.debug_rnglists,"",%progbits
.Ldebug_ranges0:
	.4byte	.Ldebug_ranges3-.Ldebug_ranges2	/* Length of Range Lists */
.Ldebug_ranges2:
	.2byte	0x5	/* DWARF Version */
	.byte	0x8	/* Address Size */
	.byte	0	/* Segment Size */
	.4byte	0	/* Offset Entry Count */
.LLRL2:
	.byte	0x7	/* DW_RLE_start_length (*.LLRL2) */
	.8byte	0x1234	/* Range begin address (*.LLRL2) */
	.uleb128 .Letext0-.Ltext0	/* Range length (*.LLRL2) */
	.byte	0x7	/* DW_RLE_start_length (*.LLRL2) */
	.8byte	0x1234	/* Range begin address (*.LLRL2) */
	.uleb128 .LFE1-.LFB1	/* Range length (*.LLRL2) */
	.byte	0	/* DW_RLE_end_of_list (*.LLRL2) */
.Ldebug_ranges3:
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.4byte	.LELT0-.LSLT0	/* Length of Source Line Info */
.LSLT0:
	.2byte	0x5	/* DWARF Version */
	.byte	0x8	/* Address Size */
	.byte	0	/* Segment Size */
	.4byte	.LELTP0-.LASLTP0	/* Prolog Length */
.LASLTP0:
	.byte	0x1	/* Minimum Instruction Length */
	.byte	0x1	/* Maximum Operations Per Instruction */
	.byte	0x1	/* Default is_stmt_start flag */
	.byte	0xf6	/* Line Base Value (Special Opcodes) */
	.byte	0xf2	/* Line Range Value (Special Opcodes) */
	.byte	0xd	/* Special Opcode Base */
	.byte	0	/* opcode: 0x1 has 0 args */
	.byte	0x1	/* opcode: 0x2 has 1 arg */
	.byte	0x1	/* opcode: 0x3 has 1 arg */
	.byte	0x1	/* opcode: 0x4 has 1 arg */
	.byte	0x1	/* opcode: 0x5 has 1 arg */
	.byte	0	/* opcode: 0x6 has 0 args */
	.byte	0	/* opcode: 0x7 has 0 args */
	.byte	0	/* opcode: 0x8 has 0 args */
	.byte	0x1	/* opcode: 0x9 has 1 arg */
	.byte	0	/* opcode: 0xa has 0 args */
	.byte	0	/* opcode: 0xb has 0 args */
	.byte	0x1	/* opcode: 0xc has 1 arg */
	.byte	0x1	/* Directory entry format count */
	.uleb128 0x1	/* DW_LNCT_path */
	.uleb128 0x1f	/* DW_FORM_line_strp */
	.uleb128 0x3	/* Directories count */
	.4byte	.LASF1	/* Directory Entry: 0: "" */
	.4byte	.LASF25	/* Directory Entry: 0: "" */
	.4byte	.LASF26	/* Directory Entry: 0: "/usr/include" */
	.byte	0x2	/* File name entry format count */
	.uleb128 0x1	/* DW_LNCT_path */
	.uleb128 0x1f	/* DW_FORM_line_strp */
	.uleb128 0x2	/* DW_LNCT_directory_index */
	.uleb128 0xb	/* DW_FORM_data1 */
	.uleb128 0x4	/* File names count */
	.4byte	.LASF0	/* File Entry: 0: "main.c" */
	.byte	0
	.4byte	.LASF27	/* File Entry: 0: "main.c" */
	.byte	0x1
	.4byte	.LASF28	/* File Entry: 0: "unistd.h" */
	.byte	0x2
	.4byte	.LASF29	/* File Entry: 0: "getopt.h" */
	.byte	0x2
.LELTP0:
	.byte	0	/* set address *.LM3 */
	.uleb128 0x9
	.byte	0x2
	.8byte	0x1234
	.byte	0x1c	/* line 6 */
	.byte	0	/* set address *.LM4 */
	.uleb128 0x9
	.byte	0x2
	.8byte	0x12346
	.byte	0x1	/* copy line 6 */
	.byte	0	/* set address *.LFE1 */
	.uleb128 0x9
	.byte	0x2
	.8byte	0x1234
	.byte	0	/* end sequence */
	.uleb128 0x1
	.byte	0x1
	.byte	0	/* set address *.LM1 */
	.uleb128 0x9
	.byte	0x2
	.8byte	0x1234
	.byte	0x1b	/* line 5 */
	.byte	0	/* set address *.LM2 */
	.uleb128 0x9
	.byte	0x2
	.8byte	0x1234
	.byte	0x1	/* copy line 5 */
	.byte	0	/* set address *.Letext0 */
	.uleb128 0x9
	.byte	0x2
	.8byte	0x1234
	.byte	0	/* end sequence */
	.uleb128 0x1
	.byte	0x1
.LELT0:
	.section	.debug_str,"MS",%progbits,1
.LASF4:
	.asciz	"unsigned int"
.LASF15:
	.asciz	"optopt"
.LASF22:
	.asciz	"main"
.LASF6:
	.asciz	"signed char"
.LASF16:
	.asciz	"xvar"
.LASF5:
	.asciz	"long unsigned int"
.LASF14:
	.asciz	"opterr"
.LASF21:
	.asciz	"GNU C11 7.0.1 20170218 (experimental) -mtune=generic -march=x86-64 -gdwarf-5 -O2"
.LASF2:
	.asciz	"unsigned char"
.LASF10:
	.asciz	"char"
.LASF13:
	.asciz	"optind"
.LASF8:
	.asciz	"long int"
.LASF19:
	.asciz	"argc"
.LASF3:
	.asciz	"short unsigned int"
.LASF17:
	.asciz	"yvar"
.LASF18:
	.asciz	"pvar"
.LASF11:
	.asciz	"__environ"
.LASF23:
	.asciz	"func"
.LASF12:
	.asciz	"optarg"
.LASF7:
	.asciz	"short int"
.LASF24:
	.asciz	"alarm"
.LASF9:
	.asciz	"sizetype"
.LASF20:
	.asciz	"argv"
	.section	.debug_line_str,"MS",%progbits,1
.LASF1:
	.asciz	""
.LASF25:
	.asciz	""
.LASF29:
	.asciz	"getopt.h"
.LASF28:
	.asciz	"unistd.h"
.LASF0:
	.asciz	"main.c"
.LASF27:
	.asciz	"main.c"
.LASF26:
	.asciz	"/usr/include"
	.ident	"GCC: (GNU) 7.0.1 20170218 (experimental)"
	.section	.note.GNU-stack,"",%progbits
