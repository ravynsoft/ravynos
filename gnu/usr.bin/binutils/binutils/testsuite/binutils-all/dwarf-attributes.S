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


/* This file is intended to check the encoding and decoding of DWARF attributes.
   Currently only numeric attributes are tested, but this file should be extended
   to cover other types of attribute as well.  */
	
	.file	"dwarf-attributes.S"

	.section	.debug_info,"",%progbits
	.4byte	.Ldebug_info_end - .Ldebug_info_start	/* Length of Compilation Unit Info.  */

.Ldebug_info_start:
	.2byte	 0x5	/* DWARF version number.  */
	.byte	 0x1	/* DW_UT_compile.  */
	.byte	 0x4	/* Pointer Size (in bytes).  */
	.4byte	 .Ldebug_abbrevs /* Offset Into Abbrev. Section.  */

	/* Start of DIE 0xc.  */

	.uleb128 0x1	/* Using abbrev #1 */
	.byte  1  	/* Ordering: column major.  */
	.2byte 1	/* Language: C89.  */
	.byte  1	/* Visibility: local.  */
	.byte  1	/* Inline: inlined.  */
	.byte  1	/* Accessibility: public.  */
	.byte  1	/* Calling convention: normal.  */
	.byte  3,1,1,2	/* Discriminate list: range.  */
	.byte  1	/* Encoding: address.  */
	.byte  1	/* Identifier case: up.  */
	.byte  1	/* Virtuality: virtual.  */
	.byte  1	/* Decimal sign: unsigned.  */
	.byte  1	/* Endianity: big.  */
	.byte  1       	/* Defaulted: in class.  */
	
	.uleb128 0x1	/* Using abbrev #1 */
	.byte  0  	/* Ordering: row major */
	.2byte 0x0016	/* Language: Go.  */
	.byte  2	/* Visibility: exported.  */
	.byte  0	/* Inline: not.  */
	.byte  2	/* Accessibility: protected.  */
	.byte  5	/* Calling convention: pass by value.  */
	.byte  2,0,1	/* Discriminate list: label.  */
	.byte  0x12	/* Encoding: ASCII.  */
	.byte  0	/* Identifier case: sensitive.  */
	.byte  0	/* Virtuality: none.  */
	.byte  2	/* Decimal sign: leading overpunch.  */
	.byte  0	/* Endianity: default.  */
	.byte  0	/* Defaulted: no.  */

	.uleb128 0x1	/* Using abbrev #1 */
	.byte  -1  	/* Ordering: undefined.  */
	.2byte 0x8001	/* Language: MIPS Assembler.  */
	.byte  3	/* Visibility: qualified.  */
	.byte  3	/* Inline: declared.  */
	.byte  3	/* Accessibility: private.  */
	.byte  0x40	/* Calling convention: Renesas SH.  */
	.byte  5,1,2,3,0,4	/* Discriminate list: range and label.  */
	.byte  0x81	/* Encoding: user specified.  */
	.byte  3	/* Identifier case: insensitive.  */
	.byte  2	/* Virtuality: pure.  */
	.byte  5	/* Decimal sign: trailing separate.  */
	.byte  0x50	/* Endianity: user specified.  */
	.byte  2	/* Defaulted: out of class.  */
	
	.byte	0	/* End of children of DIE 0xc.  */
.Ldebug_info_end:


	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrevs:
	.uleb128 0x1	/* (abbrev code) */
	.uleb128 0x5555	/* (TAG: DW_TAG_lo_user + 0x1555) */
	.byte	0	/* DW_children_no */

	/* Attributes to be tested.  Sorted by attribute value.  */
	
	.uleb128 0x9	/* (DW_AT_ordering) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x13	/* (DW_AT_language) */
	.uleb128 0x05	/* (DW_FORM_data2) */

	.uleb128 0x17	/* (DW_AT_visibility) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x20	/* (DW_AT_inline) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x32	/* (DW_AT_accessibility) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x36	/* (DW_AT_calling_convention) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x3d	/* (DW_AT_discr_lists) */
	.uleb128 0x0a	/* (DW_FORM_block1) */
	
	.uleb128 0x3e	/* (DW_AT_encoding) */
	.uleb128 0x0b	/* (DW_FORM_data1) */

	.uleb128 0x42	/* (DW_AT_identifier_case) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x4c	/* (DW_AT_virtuality) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x5e	/* (DW_AT_decimal_sign) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x65	/* (DW_AT_endianity) */
	.uleb128 0x0b	/* (DW_FORM_data1) */
	
	.uleb128 0x8b	/* (DW_AT_defaulted) */
	.uleb128 0x0b	/* (DW_FORM_data1) */	

	.byte	0	/* End of abbreviation.  */
	.byte	0

	.byte	0	/* End of abbreviations.  */
