	/* ARMv8 RAS Extension.  */
	.text

	.macro rw_sys_reg sys_reg xreg r w
	.ifc \w, 1
	msr \sys_reg, \xreg
	.endif
	.ifc \r, 1
	mrs \xreg, \sys_reg
	.endif
	.endm

	/* ARMv8-A.  */
	.arch armv8-a+ras
	esb
	hint #0x10

	rw_sys_reg sys_reg=erridr_el1 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=errselr_el1 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=erxfr_el1 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=erxctlr_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxstatus_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxaddr_el1 xreg=x5 r=1 w=1

	rw_sys_reg sys_reg=erxmisc0_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxmisc1_el1 xreg=x5 r=1 w=1

	rw_sys_reg sys_reg=vsesr_el2 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=disr_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=vdisr_el2 xreg=x5 r=1 w=0

	/* ARMv8.1-A.  */

	.arch armv8.1-a+ras
	esb
	hint #0x10

	rw_sys_reg sys_reg=erridr_el1 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=errselr_el1 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=erxfr_el1 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=erxctlr_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxstatus_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxaddr_el1 xreg=x5 r=1 w=1

	rw_sys_reg sys_reg=erxmisc0_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxmisc1_el1 xreg=x5 r=1 w=1

	rw_sys_reg sys_reg=vsesr_el2 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=disr_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=vdisr_el2 xreg=x5 r=1 w=0

	/* ARMv8.2-A.  */

	.arch armv8.2-a+ras
	esb
	hint #0x10

	rw_sys_reg sys_reg=erridr_el1 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=errselr_el1 xreg=x7 r=1 w=1

	rw_sys_reg sys_reg=erxfr_el1 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=erxctlr_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxstatus_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxaddr_el1 xreg=x5 r=1 w=1

	rw_sys_reg sys_reg=erxmisc0_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=erxmisc1_el1 xreg=x5 r=1 w=1

	rw_sys_reg sys_reg=vsesr_el2 xreg=x5 r=1 w=0
	rw_sys_reg sys_reg=disr_el1 xreg=x5 r=1 w=1
	rw_sys_reg sys_reg=vdisr_el2 xreg=x5 r=1 w=0
