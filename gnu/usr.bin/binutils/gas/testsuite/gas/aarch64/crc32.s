/* crc32.s Test file for AArch64 CRC-32 and CRC-32C checksum instructions.

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
	.ifdef DIRECTIVE
	.arch_extension crc
	.endif

	crc32b	w3, w7, w15
	crc32h	w7, w15, w3
	crc32w	w15, w3, w7
	crc32x	w3, w7, x15
	crc32cb	w3, w7, w15
	crc32ch	w7, w15, w3
	crc32cw	w15, w3, w7
	crc32cx	w3, w7, x15

	.arch_extension nocrc
