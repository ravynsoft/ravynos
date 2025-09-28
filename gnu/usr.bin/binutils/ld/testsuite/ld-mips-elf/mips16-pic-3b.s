	.abicalls
	.option	pic0
	.set	noreorder
	.include "mips16-pic-3.inc"

	call_stub unused3
	call_stub used5
	call_stub used9
	call_stub extern3

	call_fp_stub unused4
	call_fp_stub used6
	call_fp_stub used10
	call_fp_stub extern4
