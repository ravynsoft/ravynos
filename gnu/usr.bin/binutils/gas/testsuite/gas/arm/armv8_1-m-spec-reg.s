	.syntax unified
func:
	vmrs r0, FPSCR
	vmrs r1, FPSCR_nzcvqc
	vmrs r2, VPR
	vmrs r3, P0
	vmrs r4, FPCXT_NS
	vmrs r5, FPCXT_S

	vmsr fpscr, r0
	vmsr fpscr_nzcvqc, r1
	vmsr vpr, r2
	vmsr p0, r3
	vmsr fpcxt_ns, r4
	vmsr fpcxt_s, r5
