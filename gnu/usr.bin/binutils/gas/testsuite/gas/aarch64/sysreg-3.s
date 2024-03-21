/* sysreg-3.s Test file for ARMv8.3 system registers.  */

	.macro test sys_reg xreg
	msr \sys_reg, \xreg
	mrs \xreg, \sys_reg
	.endm

	.text

	test sys_reg=apiakeylo_el1 xreg=x0
	test sys_reg=apiakeyhi_el1 xreg=x1
	test sys_reg=apibkeylo_el1 xreg=x2
	test sys_reg=apibkeyhi_el1 xreg=x3

	test sys_reg=apdakeylo_el1 xreg=x4
	test sys_reg=apdakeyhi_el1 xreg=x5
	test sys_reg=apdbkeylo_el1 xreg=x6
	test sys_reg=apdbkeyhi_el1 xreg=x7

	test sys_reg=apgakeylo_el1 xreg=x8
	test sys_reg=apgakeyhi_el1 xreg=x9
