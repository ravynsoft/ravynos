# Check 32bit AVX512ER-RCIG instructions

	.allow_index_reg
	.text
_start:
	vexp2ps	{sae}, %zmm5, %zmm6	 # AVX512ER
	vexp2pd	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrcp28ps	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrcp28pd	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrcp28ss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28ps	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrsqrt28pd	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrsqrt28ss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER

	.intel_syntax noprefix
	vexp2ps	zmm6, zmm5, {sae}	 # AVX512ER
	vexp2pd	zmm6, zmm5, {sae}	 # AVX512ER
	vrcp28ps	zmm6, zmm5, {sae}	 # AVX512ER
	vrcp28pd	zmm6, zmm5, {sae}	 # AVX512ER
	vrcp28ss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512ER
	vrcp28sd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512ER
	vrsqrt28ps	zmm6, zmm5, {sae}	 # AVX512ER
	vrsqrt28pd	zmm6, zmm5, {sae}	 # AVX512ER
	vrsqrt28ss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512ER
	vrsqrt28sd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512ER
