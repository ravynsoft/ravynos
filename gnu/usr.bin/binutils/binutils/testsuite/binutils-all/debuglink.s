/* Assembler source used to create an object file for testing readelf's
   and objdump's ability to process separate debug information files.

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

	
	/* Create a fake .gnu_debuglink section.  */

	.section .gnu_debuglink,"",%progbits
	.asciz "this_is_a_debuglink.debug"
	.balign 4
	.4byte 0x12345678

	/* Create a fake .gnu_debugaltlink section.  */

	.section .gnu_debugaltlink,"",%progbits
	.asciz "linkdebug.debug"
	.dc.b 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
	.dc.b 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
	.dc.b 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
   
	/* Create a .debug_str section for local use.  This is also to check
	   the ability to dump the same section twice, if it exists in
	   both the main file and the separate debug info file.  */

	.section	.debug_str,"MS",%progbits,1
string1:
	.asciz	"string-1"
	.asciz  "string-2"
	.balign	2
string_end:
	
	/* Create a .debug_info section that contains string references into
	   the separate debug info file.  Plus the abbreviations are stored
	   in the separate file too...  */

	.section	.debug_info,"",%progbits
	.4byte	debugE - debugS	;# Length of Compilation Unit Info
debugS:
	.short	0x4	;# DWARF version number.
	.4byte	0x0	;# Offset into .debug_abbrev section.
	.byte	0x4	;# Pointer Size (in bytes).

	.uleb128 0x1	;# Use abbrev #1.  This needs a string from the local string table.
	.4byte	string1

	.uleb128 0x2	;# Use abbrev #2.  This needs a string from the separate string table.
	.4byte   0x0	;# Avoid complicated expression resolution and hard code the offset...

	;# Minimal section alignment on alpha-* is 2, so ensure no new invalid CU
	;# will be started.
	.balign	2, 0
debugE:
