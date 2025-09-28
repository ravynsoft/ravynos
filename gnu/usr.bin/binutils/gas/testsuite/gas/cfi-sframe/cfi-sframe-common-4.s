## Testcase for cfi_def_cfa_offset
	.cfi_startproc
	.long 0
	.cfi_def_cfa_offset 16
	.long 0
	.cfi_def_cfa_offset 32
	.long 0
	.cfi_endproc
