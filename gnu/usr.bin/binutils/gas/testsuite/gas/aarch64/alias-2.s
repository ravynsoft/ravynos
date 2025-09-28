/* alias-2.s Test file for ARMv8.2 AArch64 instructions aliases or disassembly
   preference.  */

	/* <bfm> [Xd|xzr], [xzr|<Xr>], <imm>, <width>  */
	.macro bfm_inst op imm width
	\op     x0, xzr, #\imm, #\width
	\op     x0, x1, #\imm, #\width
	\op     xzr, x1, #\imm, #\width
	\op     xzr, xzr, #\imm, #\width
	.endm

	/* bfc [Xd|xzr], <imm>, <width>  */
	.macro bfc_inst imm width
	bfc     x0, #\imm, #\width
	bfc     xzr, #\imm, #\width
	.endm

	/* <rev> [Xd|xzr], [Xr|xzr]  */
	.macro rev_inst op
	\op     x0, xzr
	\op     x0, x1
	\op     xzr, x1
	\op     xzr, xzr
	.endm

.text
	.irp op, bfm, bfi
	.irp imm, 1, 16, 31
	.irp width, 1, 8, 15
	bfm_inst \op, \imm, \width
	.endr
	.endr
	.endr

	.irp imm, 1, 16, 31
	.irp width, 1, 8, 15
	bfc_inst \imm, \width
	.endr
	.endr

	.irp op, rev, rev16, rev64
	rev_inst \op
	.endr
