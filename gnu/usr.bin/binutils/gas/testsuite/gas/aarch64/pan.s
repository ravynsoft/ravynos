/* pan.s Test file for AArch64 PAN instructions.

   Copyright (C) 2015-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

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
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */


	.text
	.ifdef DIRECTIVE
	.arch_extension pan
	.endif

	msr pan, #1
	msr pan, #0

	msr pan, x0
	mrs x1, pan

	.ifdef ERROR
	.irp N,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	msr pan, #\N
	.endr
	.endif

	.arch_extension nopan
