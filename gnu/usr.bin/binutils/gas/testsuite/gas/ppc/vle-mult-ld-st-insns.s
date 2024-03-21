# VLE Instructions for Improving Interrupt Handler Efficiency (e200z760RM.pdf)
# Original Engineering Bullet (EB696.pdf) contains two writings of load instructions
# and has no ones for MCSRRs.

# e_lmvgprw, e_stmvgprw - load/store multiple volatile GPRs (r0, r3:r12)
# e_lmvsprw, e_stmvsprw - load/store multiple volatile SPRs (CR, LR, CTR, and XER)
# e_lmvsrrw, e_stmvsrrw - load/store multiple volatile SRRs (SRR0, SRR1)
# e_lmvcsrrw, e_stmvcsrrw - load/store multiple volatile CSRRs (CSRR0, CSRR1)
# e_lmvdsrrw, e_stmvdsrrw - load/store multiple volatile DSRRs (DSRR0, DSRR1)
# e_lmvmcsrrw, e_stmvmcsrrw - load/store multiple volatile MCSRRs (MCSRR0, MCSRR1)

	.text
prolog:
	e_stmvgprw	0x00 (r1)
	e_stmvsprw	0x04 (r2)
	e_stmvsrrw	0x08 (r3)
	e_stmvcsrrw	0x0c (r4)
	e_stmvdsrrw	0x10 (r5)
	e_stmvmcsrrw	0x14 (r6)

epilog:
	e_lmvgprw	0x18 (r7)
	e_lmvsprw	0x1c (r8)
	e_lmvsrrw	0x20 (r9)
	e_lmvcsrrw	0x24 (r10)
	e_lmvdsrrw	0x28 (r11)
	e_lmvmcsrrw	0x2c (r12)

epilog_alt:
	e_ldmvgprw	0x30 (r13)
	e_ldmvsprw	0x34 (r14)
	e_ldmvsrrw	0x38 (r15)
	e_ldmvcsrrw	0x3c (r16)
	e_ldmvdsrrw	0x40 (r17)
