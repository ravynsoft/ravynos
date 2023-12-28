/*
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
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

func:
	msr daifset, #0
	msr daifset, #1
	msr daifset, #15



	msr daifclr, #0
	msr daifclr, #1
	msr daifclr, #15

	msr daif, x0
	mrs x0, daif

	msr spsel, #0
	msr spsel, #1

	msr csselr_el1, x0
	mrs x0, csselr_el1

	msr vsesr_el2, x0
	mrs x0, vsesr_el2

	msr osdtrrx_el1, x0
	mrs x0, osdtrrx_el1

	msr osdtrtx_el1, x0
	mrs x0, osdtrtx_el1

	mrs x0, pmsidr_el1
