/* Assembler source used to create an object file for testing readelf's
   and objdump's ability to process separate dwarf object files.

   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

	
	/* Create a .debug_str section for local use.  This is also to check
	   the ability to dump the same section twice, if it exists in
	   both the main file and the separate debug info file.  */

	.section	.debug_str,"MS",%progbits,1
string1:
	.asciz	"debugfile.dwo"
string2:
	.asciz  "/path/to/dwo/files"
string3:
	.asciz  "/another/path/"
	.balign	2
string_end:
	
	/* Create a .debug_info section that contains the dwo links.  */

	.section	.debug_info,"",%progbits
	.4byte	debugE1 - debugS1	;# Length of Compilation Unit Info
debugS1:
	.short	0x4	;# DWARF version number.
	.4byte	0x0	;# Offset into .debug_abbrev section.
	.byte	0x4	;# Pointer Size (in bytes).

	.uleb128 0x1	;# Use abbrev #1.  This needs strings from the .debug_str section.
	.4byte	string1
	.4byte  string2
debugE1:

	.4byte	debugE2 - debugS2	;# Length of Compilation Unit Info
debugS2:
	.short	0x4	;# DWARF version number.
	.4byte	0x0	;# Offset into .debug_abbrev section.
	.byte	0x4	;# Pointer Size (in bytes).

	.uleb128 0x2	;# Use abbrev #2.
	.asciz   "file.dwo"
	.4byte   string3
	.8byte   0x12345678aabbccdd

	;# Minimal section alignment on alpha-* is 2, so ensure no new invalid CU
	;# will be started.
	.balign	2, 0
debugE2:

	.section	.debug_abbrev,"",%progbits

	/* Create an abbrev containing a DWARF5 style dwo link.  */
	.uleb128 0x01	;# Abbrev code.
	.uleb128 0x11	;# DW_TAG_compile_unit
	.byte	 0x00	;# DW_children_no
	.uleb128 0x76	;# DW_AT_dwo_name
	.uleb128 0x0e	;# DW_FORM_strp
	.uleb128 0x1b	;# DW_AT_comp_dir
	.uleb128 0x0e	;# DW_FORM_strp
	.byte	 0x00	;# End of abbrev
	.byte	 0x00

	/* Create an abbrev containing a GNU style dwo link.  */
	.uleb128 0x02	;# Abbrev code.
	.uleb128 0x11	;# DW_TAG_compile_unit
	.byte	 0x00	;# DW_children_no
	.uleb128 0x2130	;# DW_AT_GNU_dwo_name
	.uleb128 0x08	;# DW_FORM_string
	.uleb128 0x1b	;# DW_AT_comp_dir
	.uleb128 0x0e	;# DW_FORM_strp
	.uleb128 0x2131	;# DW_AT_GNU_dwo_id
	.uleb128 0x07	;# DW_FORM_data8	
	.byte	 0x00	;# End of abbrev
	.byte	 0x00

	.byte	 0x0	;# Abbrevs terminator

