# Check 32bit AVX512{BW,VL} swap instructions

	.allow_index_reg
	.text
_start:
	vmovdqu8	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8.s	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8.s	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8.s	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8.s	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8.s	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8.s	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8.s	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8.s	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16.s	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16.s	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16.s	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16.s	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16.s	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16.s	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16.s	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16.s	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}

	.intel_syntax noprefix
	vmovdqu8	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu8.s	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu8.s	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu8.s	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu8.s	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu8.s	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu8.s	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu8.s	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu8.s	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu16.s	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu16.s	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu16.s	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu16.s	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu16.s	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu16.s	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu16.s	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu16.s	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
