# Check 64bit CLFLUSHOPT instructions

	.allow_index_reg
	.text
_start:

	clflushopt	(%rcx)	 # CLFLUSHOPT
	clflushopt	0x123(%rax,%r14,8)	 # CLFLUSHOPT

	.intel_syntax noprefix
	clflushopt	BYTE PTR [rcx]	 # CLFLUSHOPT
	clflushopt	BYTE PTR [rax+r14*8+0x1234]	 # CLFLUSHOPT
