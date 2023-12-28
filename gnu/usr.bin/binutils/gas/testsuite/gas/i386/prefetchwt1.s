# Check 32bit AVX512PF instructions

	.allow_index_reg
	.text
_start:

	prefetchwt1	(%ecx)
	prefetchwt1	-123456(%esp,%esi,8)

	.intel_syntax noprefix

	prefetchwt1	BYTE PTR [ecx]
	prefetchwt1	BYTE PTR [esp+esi*8-123456]
