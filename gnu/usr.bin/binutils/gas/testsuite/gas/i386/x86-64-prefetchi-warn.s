# Check error for ICACHE-PREFETCH 64-bit instruction

	.allow_index_reg
	.text
_start:
	prefetchit0     0x12345678(%rax)
	prefetchit1     0x12345678(%rax)

	.intel_syntax noprefix
	prefetchit0     BYTE PTR [rax+0x12345678]
	prefetchit1     BYTE PTR [rax+0x12345678]
