/* Test view number decoding.

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

	.file "dwarf2-6.c"
	.text
	.balign 4
	.globl _start
_start:
.L_start:
	.org .+256
.Lend_start:

	.section .debug_line,"",%progbits
	.4byte .Lline_end - .Lline_start  /* Initial length.  */
.Lline_start:
	.2byte 2 /* Dwarf Version.  */
	.4byte .Lline_lines - .Lline_hdr
.Lline_hdr:
	.byte 1 /* Minimum insn length.  */
	.byte 1 /* Default is_stmt.  */
	.byte 1 /* Line base.  */
	.byte 1 /* Line range.  */
	.byte 0x10 /* Opcode base.  */

	/* Standard lengths.  */
	.byte 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0

	.byte 0 /* Include directories.  */

	/* File names.  */
	.ascii "dwarf2-6.c\0"
	.uleb128 0, 0, 0

	.byte 0

.Lline_lines:
	.byte 0 /* DW_LNS_extended_op.  */
	.uleb128 5
	.byte 2 /* DW_LNE_set_address.  */
	.4byte .L_start

	.byte 1 /* DW_LNS_copy view 0.  */

	.byte 1 /* DW_LNS_copy view 1.  */

	.byte 0 /* DW_LNS_extended_op.  */
	.uleb128 5
	.byte 2 /* DW_LNE_set_address.  */
	.4byte .L_start+1

	.byte 1 /* DW_LNS_copy view 0.  */

	.byte 2 /* DW_LNS_advance_pc by 0.  */
	.uleb128 0

	.byte 1 /* DW_LNS_copy view 1.  */

	.byte 2 /* DW_LNS_advance_pc by 1 (reset view).  */
	.uleb128 1

	.byte 1 /* DW_LNS_copy view 0.  */

	.byte 9 /* DW_LNS_fixed_advance_pc by 1.  */
	.2byte 1 /* This opcode does NOT reset view.  */

	.byte 1 /* DW_LNS_copy view 1.  */

	.byte 16 /* Special opcode 0, PC+=0, Line+=1, view 2.  */

	.byte 17 /* Special opcode 1, PC+=1 (reset view), Line+=1.  */

	.byte 1 /* DW_LNS_copy view 1.  */

	.byte 8 /* DW_LNS_const_add_pc by 239 (reset view).  */

	.byte 1 /* DW_LNS_copy view 0.  */

	.byte 0 /* DW_LNS_extended_op.  */
	.uleb128 5
	.byte 2 /* DW_LNE_set_address.  */
	.4byte .Lend_start

	.byte 0 /* DW_LNS_extended_op.  */
	.uleb128 1
	.byte 1 /* DW_LEN_end_of_sequence.  */

.Lline_end:
