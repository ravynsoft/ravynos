# Check 64bit CLWB instructions

	.allow_index_reg
	.text
_start:

	clwb	(%rcx)	 # CLWB
	clwb	0x123(%rax,%r14,8)	 # CLWB

	.intel_syntax noprefix
	clwb	BYTE PTR [rcx]	 # CLWB
	clwb	BYTE PTR [rax+r14*8+0x1234]	 # CLWB
