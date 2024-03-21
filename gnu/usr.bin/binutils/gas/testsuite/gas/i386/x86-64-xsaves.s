# Check 64bit XSAVES instructions

	.allow_index_reg
	.text
_start:

	xsaves64	(%rcx)	 # XSAVES
	xsaves64	0x123(%rax,%r14,8)	 # XSAVES
	xrstors64	(%rcx)	 # XSAVES
	xrstors64	0x123(%rax,%r14,8)	 # XSAVES

	.intel_syntax noprefix
	xsaves64	[rcx]	 # XSAVES
	xsaves64	[rax+r14*8+0x1234]	 # XSAVES
	xrstors64	[rcx]	 # XSAVES
	xrstors64	[rax+r14*8+0x1234]	 # XSAVES
