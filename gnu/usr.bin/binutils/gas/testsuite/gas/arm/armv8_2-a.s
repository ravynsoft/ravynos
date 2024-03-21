	/* ARMv8.2 features.  */

	/* RAS instructions.  */
A1:
	.arm
	esb
T1:	.thumb
	esb

	/* RAS system registers.  */
	.macro test_sysreg Opc1 CRn CRm Opc2 rw
	mrc p15, \Opc1,\() r0, \CRn\(), \CRm\(), \Opc2\()
	.if \rw
	mcr p15, \Opc1\(), r1, \CRn\(), \CRm\(), \Opc2\()
	.endif
	.endm

A2:
	.arm
	test_sysreg 0 c0 c1 0 0
	test_sysreg 0 c0 c2 6 0
	test_sysreg 0 c5 c3 0 0
	test_sysreg 0 c5 c3 1 1

	test_sysreg 0 c5 c4 0 0
	test_sysreg 0 c5 c4 1 1
	test_sysreg 0 c5 c4 2 1
	test_sysreg 0 c5 c4 3 1
	test_sysreg 0 c5 c4 4 0
	test_sysreg 0 c5 c4 5 1
	test_sysreg 0 c5 c4 7 1

	test_sysreg 0 c5 c5 0 1
	test_sysreg 0 c5 c5 1 1
	test_sysreg 0 c5 c5 4 1
	test_sysreg 0 c5 c5 5 1

	test_sysreg 0 c12 c1 1 1
	test_sysreg 4 c1  c1 4 1
	test_sysreg 4 c5  c2 3 1
	test_sysreg 4 c1  c1 1 1
	test_sysreg 4 c12 c1 1 1

	test_sysreg 6 c1 c1 0 1
