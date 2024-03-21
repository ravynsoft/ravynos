/* sysreg-1.s Test file for AArch64 system registers.

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

	.macro rw_sys_reg sys_reg xreg r w
	.ifc \w, 1
	msr \sys_reg, \xreg
	.endif
	.ifc \r, 1
	mrs \xreg, \sys_reg
	.endif
	.endm

	.text

	rw_sys_reg sys_reg=id_aa64afr0_el1 xreg=x7 r=1 w=0
	rw_sys_reg sys_reg=id_aa64afr1_el1 xreg=x7 r=1 w=0
	rw_sys_reg sys_reg=mvfr2_el1 xreg=x7 r=1 w=0
	rw_sys_reg sys_reg=dlr_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=dspsr_el0 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=sder32_el3 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=mdcr_el3 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=mdccint_el1 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=dbgvcr32_el2 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=fpexc32_el2 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=teecr32_el1 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=teehbr32_el1 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=cntp_tval_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=cntp_ctl_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=cntp_cval_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=cntps_tval_el1 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=cntps_ctl_el1 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=cntps_cval_el1 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=pmccntr_el0 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=pmevcntr0_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr1_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr2_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr3_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr4_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr5_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr6_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr7_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr8_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr9_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr10_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr11_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr12_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr13_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr14_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr15_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr16_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr17_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr18_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr19_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr20_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr21_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr22_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr23_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr24_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr25_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr26_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr27_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr28_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr29_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevcntr30_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper0_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper1_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper2_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper3_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper4_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper5_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper6_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper7_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper8_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper9_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper10_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper11_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper12_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper13_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper14_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper15_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper16_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper17_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper18_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper19_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper20_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper21_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper22_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper23_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper24_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper25_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper26_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper27_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper28_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper29_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmevtyper30_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=pmccfiltr_el0 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=tpidrro_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=tpidr_el0 xreg=x7 r=1 w=1
	rw_sys_reg sys_reg=cntfrq_el0 xreg=x7 r=1 w=1

	//
	// Macros to generate MRS and MSR with all the implementation defined
	// system registers in the form of S3_<op1>_<Cn>_<Cm>_<op2>.

	.altmacro
	.macro	all_op2	op1, crn, crm, from=0, to=7
	rw_sys_reg S3_\op1\()_C\crn\()_C\crm\()_\from x15 1 1
	.if	(\to-\from > 0)
	all_op2 \op1, \crn, \crm, %(\from+1), \to
	.endif
	.endm

	.macro	all_crm	op1, crn, from=0, to=15
	all_op2	\op1, \crn, \from, 0, 7
	.if	(\to-\from > 0)
	all_crm \op1, \crn, %(\from+1), \to
	.endif
	.endm

	.macro all_imple_defined	from=0, to=7
	.irp crn, 11, 15
	all_crm	\from, \crn, 0, 15
	.endr
	.if	\to-\from
	all_imple_defined %(\from+1), \to
	.endif
	.endm

	all_imple_defined	0, 7
	.noaltmacro

	rw_sys_reg sys_reg=dbgdtr_el0 xreg=x15 r=1 w=1
	rw_sys_reg sys_reg=dbgdtrrx_el0 xreg=x15 r=1 w=0

	rw_sys_reg sys_reg=rmr_el1 xreg=x15 r=1 w=1
	rw_sys_reg sys_reg=rmr_el2 xreg=x15 r=1 w=1
	rw_sys_reg sys_reg=rmr_el3 xreg=x15 r=1 w=1

	rw_sys_reg sys_reg=spsr_el1 xreg=x15 r=1 w=1
	rw_sys_reg sys_reg=spsr_el2 xreg=x15 r=1 w=1
	rw_sys_reg sys_reg=spsr_el3 xreg=x15 r=1 w=1

	rw_sys_reg sys_reg=s0_0_C0_C0_0 xreg=x15 r=1 w=1
	rw_sys_reg sys_reg=s1_7_C15_C15_7 xreg=x27 r=1 w=1
	rw_sys_reg sys_reg=s2_4_C6_C8_0 xreg=x14 r=1 w=1
	rw_sys_reg sys_reg=s1_2_C14_C4_2 xreg=x4 r=1 w=1
	rw_sys_reg sys_reg=s0_1_C13_C1_3 xreg=x7 r=1 w=1
