# Check 32bit AVX512BW swap instructions

	.allow_index_reg
	.text
_start:
	vmovdqu8	%zmm5, %zmm6	 # AVX512BW
	vmovdqu8.s	%zmm5, %zmm6	 # AVX512BW
	vmovdqu8	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu8.s	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu8	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu8.s	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu8	%zmm5, %zmm6	 # AVX512BW
	vmovdqu8.s	%zmm5, %zmm6	 # AVX512BW
	vmovdqu8	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu8.s	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu8	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu8.s	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6	 # AVX512BW
	vmovdqu16.s	%zmm5, %zmm6	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu16.s	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu16.s	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6	 # AVX512BW
	vmovdqu16.s	%zmm5, %zmm6	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu16.s	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu16.s	%zmm5, %zmm6{%k7}{z}	 # AVX512BW

	.intel_syntax noprefix
	vmovdqu8	zmm6, zmm5	 # AVX512BW
	vmovdqu8.s	zmm6, zmm5	 # AVX512BW
	vmovdqu8	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu8.s	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu8	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu8.s	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu8	zmm6, zmm5	 # AVX512BW
	vmovdqu8.s	zmm6, zmm5	 # AVX512BW
	vmovdqu8	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu8.s	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu8	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu8.s	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu16	zmm6, zmm5	 # AVX512BW
	vmovdqu16.s	zmm6, zmm5	 # AVX512BW
	vmovdqu16	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu16.s	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu16	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu16.s	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu16	zmm6, zmm5	 # AVX512BW
	vmovdqu16.s	zmm6, zmm5	 # AVX512BW
	vmovdqu16	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu16.s	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu16	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu16.s	zmm6{k7}{z}, zmm5	 # AVX512BW
