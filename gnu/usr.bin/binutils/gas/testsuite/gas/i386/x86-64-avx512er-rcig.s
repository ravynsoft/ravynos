# Check 64bit AVX512ER-RCIG instructions

	.allow_index_reg
	.text
_start:
	vexp2ps	{sae}, %zmm29, %zmm30	 # AVX512ER
	vexp2pd	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrcp28ps	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrcp28pd	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrcp28ss	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512ER
	vrcp28sd	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512ER
	vrsqrt28ps	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrsqrt28pd	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrsqrt28ss	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512ER
	vrsqrt28sd	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512ER

	.intel_syntax noprefix
	vexp2ps	zmm30, zmm29, {sae}	 # AVX512ER
	vexp2pd	zmm30, zmm29, {sae}	 # AVX512ER
	vrcp28ps	zmm30, zmm29, {sae}	 # AVX512ER
	vrcp28pd	zmm30, zmm29, {sae}	 # AVX512ER
	vrcp28ss	xmm30, xmm29, xmm28, {sae}	 # AVX512ER
	vrcp28sd	xmm30, xmm29, xmm28, {sae}	 # AVX512ER
	vrsqrt28ps	zmm30, zmm29, {sae}	 # AVX512ER
	vrsqrt28pd	zmm30, zmm29, {sae}	 # AVX512ER
	vrsqrt28ss	xmm30, xmm29, xmm28, {sae}	 # AVX512ER
	vrsqrt28sd	xmm30, xmm29, xmm28, {sae}	 # AVX512ER
