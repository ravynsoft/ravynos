/* crypto.s Test file for AArch64 Advanced-SIMD Crypto instructions.

   Copyright (C) 2012-2023 Free Software Foundation, Inc.  Contributed by ARM Ltd.

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
	.if DIRECTIVE > 1
	.arch_extension aes
	.arch_extension sha2
	.else
	.arch_extension crypto
	.endif
	.if DIRECTIVE == 3
	.arch_extension nosha3
	.endif
	.endif

	aese	v7.16b, v31.16b
	aesd	v7.16b, v31.16b
	aesmc	v7.16b, v31.16b
	aesimc	v7.16b, v31.16b

	sha1h	s7, s31
	sha1su1	v7.4s, v31.4s
	sha256su0	v7.4s, v31.4s

	sha1c	q7, s15, v31.4s
	sha1p	q7, s15, v31.4s
	sha1m	q7, s15, v31.4s

	sha1su0	v7.4s, v15.4s, v31.4s
	sha256h	q7, q15, v31.4s
	sha256h2	q7, q15, v31.4s
	sha256su1	v7.4s, v15.4s, v31.4s

	pmull	v7.8h, v15.8b, v31.8b
	pmull	v7.1q, v15.1d, v31.1d
	pmull2	v7.8h, v15.16b, v31.16b
	pmull2	v7.1q, v15.2d, v31.2d

	.arch_extension nocrypto
