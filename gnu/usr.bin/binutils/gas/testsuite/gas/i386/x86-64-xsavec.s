# Check 64bit XSAVEC instructions

	.allow_index_reg
	.text
_start:

	xsavec64	(%rcx)	 # XSAVEC
	xsavec64	0x123(%rax,%r14,8)	 # XSAVEC

	.intel_syntax noprefix
	xsavec64	[rcx]	 # XSAVEC
	xsavec64	[rax+r14*8+0x1234]	 # XSAVEC
