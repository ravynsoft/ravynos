# Check 64bit AVX512BW swap instructions

	.allow_index_reg
	.text
_start:
	vmovdqu8	%zmm29, %zmm30	 # AVX512BW
	vmovdqu8.s	%zmm29, %zmm30	 # AVX512BW
	vmovdqu8	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu8.s	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu8	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu8.s	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu8	%zmm29, %zmm30	 # AVX512BW
	vmovdqu8.s	%zmm29, %zmm30	 # AVX512BW
	vmovdqu8	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu8.s	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu8	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu8.s	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30	 # AVX512BW
	vmovdqu16.s	%zmm29, %zmm30	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu16.s	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu16.s	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30	 # AVX512BW
	vmovdqu16.s	%zmm29, %zmm30	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu16.s	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu16.s	%zmm29, %zmm30{%k7}{z}	 # AVX512BW

	.intel_syntax noprefix
	vmovdqu8	zmm30, zmm29	 # AVX512BW
	vmovdqu8.s	zmm30, zmm29	 # AVX512BW
	vmovdqu8	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu8.s	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu8	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu8.s	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu8	zmm30, zmm29	 # AVX512BW
	vmovdqu8.s	zmm30, zmm29	 # AVX512BW
	vmovdqu8	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu8.s	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu8	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu8.s	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu16	zmm30, zmm29	 # AVX512BW
	vmovdqu16.s	zmm30, zmm29	 # AVX512BW
	vmovdqu16	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu16.s	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu16	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu16.s	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu16	zmm30, zmm29	 # AVX512BW
	vmovdqu16.s	zmm30, zmm29	 # AVX512BW
	vmovdqu16	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu16.s	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu16	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu16.s	zmm30{k7}{z}, zmm29	 # AVX512BW
