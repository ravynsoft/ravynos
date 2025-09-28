/* rm-simd-ext.s Test file for AArch64 extension removal in -mcpu option.

   Copyright (C) 2013-2023 Free Software Foundation, Inc.
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
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

	.text
	mov	w3, 200
	mul	w3, w2, w3
	add	w1, w1, w3
	add	w2, w0, w2, lsl 2
	orr	v0.16b, v1.16b, v2.16b
	orr	w1, w1, w3
