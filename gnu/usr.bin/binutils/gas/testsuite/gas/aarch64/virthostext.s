/* virthostext.s Test file for ARMv8.1 Virtualization Host Extension
	support.

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

	.macro rw_sys_reg sys_reg xreg
	msr \sys_reg, \xreg
	mrs \xreg, \sys_reg
	.endm

	.text
	.ifdef DIRECTIVE
	.arch armv8.1-a
	.endif

	rw_sys_reg spsr_el12 x7
	rw_sys_reg elr_el12 x7

	rw_sys_reg sctlr_el12 x7
	rw_sys_reg cpacr_el12 x7

	rw_sys_reg ttbr1_el2 x7
	rw_sys_reg ttbr0_el12 x7
	rw_sys_reg ttbr1_el12 x7

	rw_sys_reg tcr_el12 x7

	rw_sys_reg afsr0_el12 x7
	rw_sys_reg afsr1_el12 x7

	rw_sys_reg esr_el12 x7
	rw_sys_reg far_el12 x7
	rw_sys_reg mair_el12 x7
	rw_sys_reg amair_el12 x7
	rw_sys_reg vbar_el12 x7

	rw_sys_reg contextidr_el2 x7
	rw_sys_reg contextidr_el12 x7

	rw_sys_reg cntkctl_el12 x7

	rw_sys_reg cntp_tval_el02 x7
	rw_sys_reg cntp_ctl_el02 x7
	rw_sys_reg cntp_cval_el02 x7

	rw_sys_reg cntv_tval_el02 x7
	rw_sys_reg cntv_ctl_el02 x7
	rw_sys_reg cntv_cval_el02 x7

	rw_sys_reg cnthv_tval_el2 x7
	rw_sys_reg cnthv_ctl_el2 x7
	rw_sys_reg cnthv_cval_el2 x7
