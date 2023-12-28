/* lor.s Test file for AArch64 LOR extension instructions.

   Copyright (C) 2015-2023 Free Software Foundation, Inc.  Contributed by ARM Ltd.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

	.text
	.ifdef DIRECTIVE
	.arch_extension lor
	.endif

        stllr w0, [x0]
	stllr x0, [x0]
	
	stllr w1, [x0]
	stllr x2, [x1]
        stllrh w3, [x2]
        stllrb w4, [x3]
	stllrb w5, [sp]
        
        ldlar w0, [x0]
	ldlar x0, [x0]

        ldlar w1, [x0]
	ldlar x2, [x1]
        ldlarb w3, [x2]
        ldlarh w4, [x3]
        ldlar w5, [sp]  

