# Check 64bit AVX512PF instructions

	.allow_index_reg
	.text
_start:

	prefetchwt1	(%rcx)	 # AVX512PF
	prefetchwt1	0x123(%rax,%r14,8)	 # AVX512PF

	.intel_syntax noprefix

	prefetchwt1	BYTE PTR [rcx]	 # AVX512PF
	prefetchwt1	BYTE PTR [rax+r14*8+0x1234]	 # AVX512PF
