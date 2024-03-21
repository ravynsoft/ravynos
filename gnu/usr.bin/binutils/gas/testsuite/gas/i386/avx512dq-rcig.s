# Check 32bit AVX512DQ-RCIG instructions

	.allow_index_reg
	.text
_start:
	vrangepd	$0xab, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$0xab, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangesd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducepd	$0xab, {sae}, %zmm5, %zmm6	 # AVX512DQ
	vreducepd	$123, {sae}, %zmm5, %zmm6	 # AVX512DQ
	vreduceps	$0xab, {sae}, %zmm5, %zmm6	 # AVX512DQ
	vreduceps	$123, {sae}, %zmm5, %zmm6	 # AVX512DQ
	vreducesd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vcvttpd2qq	{sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvttpd2uqq	{sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvttps2qq	{sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	{sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ

	.intel_syntax noprefix
	vrangepd	zmm6, zmm5, zmm4, {sae}, 0xab	 # AVX512DQ
	vrangepd	zmm6, zmm5, zmm4, {sae}, 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, zmm4, {sae}, 0xab	 # AVX512DQ
	vrangeps	zmm6, zmm5, zmm4, {sae}, 123	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512DQ
	vreducepd	zmm6, zmm5, {sae}, 0xab	 # AVX512DQ
	vreducepd	zmm6, zmm5, {sae}, 123	 # AVX512DQ
	vreduceps	zmm6, zmm5, {sae}, 0xab	 # AVX512DQ
	vreduceps	zmm6, zmm5, {sae}, 123	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512DQ
	vcvttpd2qq	zmm6, zmm5, {sae}	 # AVX512DQ
	vcvttpd2uqq	zmm6, zmm5, {sae}	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, ymm5, {sae}	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, ymm5, {sae}	 # AVX512DQ
