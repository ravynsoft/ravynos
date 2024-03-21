# Check illegal AVX512{BF16,VL} instructions

	.allow_index_reg
	.text
_start:
	vcvtneps2bf16	0x10000000(%rbp, %r14, 8), %xmm3{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtneps2bf16	2032(%rcx), %xmm3	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	4064(%rcx), %xmm3	 #AVX512{BF16,VL} Disp8

.intel_syntax noprefix
	vcvtneps2bf16	xmm3{k7}, [rbp+r14*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtneps2bf16	xmm3, [rcx+2032]	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	xmm3, [rcx+4064]	 #AVX512{BF16,VL} Disp8
