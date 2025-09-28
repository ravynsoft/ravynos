/* fp-const0-parse.s Test file For AArch64 float constant 0 parse.

   Copyright (C) 2014-2023 Free Software Foundation, Inc.
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
	// Check #0 with scalar register.
	fcmeq	s0, s30, #0
	fcmge	s1, s29, #0
	fcmgt	s2, s28, #0
	fcmle	s3, s27, #0
	fcmlt	s4, s26, #0
	fcmeq	d0, d30, #0
	fcmge	d1, d29, #0
	fcmgt	d2, d28, #0
	fcmle	d3, d27, #0
	fcmlt	d4, d26, #0

	// Check #0 with vector register.
	fcmeq	v0.2s, v30.2s, #0
	fcmge	v1.4s, v29.4s, #0
	fcmgt	v2.2d, v28.2d, #0
	fcmle	v3.2s, v27.2s, #0
	fcmlt	v4.4s, v26.4s, #0

	// Check #0.0 with scalar register.
	fcmeq	s0, s30, #0.0
	fcmge	s1, s29, #0.0
	fcmgt	s2, s28, #0.0
	fcmle	s3, s27, #0.0
	fcmlt	s4, s26, #0.0
	fcmeq	d0, d30, #0.0
	fcmge	d1, d29, #0.0
	fcmgt	d2, d28, #0.0
	fcmle	d3, d27, #0.0
	fcmlt	d4, d26, #0.0

	// Check #0.0 with vector register.
	fcmeq	v0.2s, v30.2s, #0.0
	fcmge	v1.4s, v29.4s, #0.0
	fcmgt	v2.2d, v28.2d, #0.0
	fcmle	v3.2s, v27.2s, #0.0
	fcmlt	v4.4s, v26.4s, #0.0
