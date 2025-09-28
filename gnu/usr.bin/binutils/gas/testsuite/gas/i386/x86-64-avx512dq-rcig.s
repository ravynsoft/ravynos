# Check 64bit AVX512DQ-RCIG instructions

	.allow_index_reg
	.text
_start:
	vrangepd	$0xab, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$0xab, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangesd	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangesd	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangess	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangess	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducepd	$0xab, {sae}, %zmm29, %zmm30	 # AVX512DQ
	vreducepd	$123, {sae}, %zmm29, %zmm30	 # AVX512DQ
	vreduceps	$0xab, {sae}, %zmm29, %zmm30	 # AVX512DQ
	vreduceps	$123, {sae}, %zmm29, %zmm30	 # AVX512DQ
	vreducesd	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducesd	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducess	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducess	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vcvttpd2qq	{sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvttpd2uqq	{sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvttps2qq	{sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvttps2uqq	{sae}, %ymm29, %zmm30	 # AVX512DQ

	.intel_syntax noprefix
	vrangepd	zmm30, zmm29, zmm28, {sae}, 0xab	 # AVX512DQ
	vrangepd	zmm30, zmm29, zmm28, {sae}, 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, zmm28, {sae}, 0xab	 # AVX512DQ
	vrangeps	zmm30, zmm29, zmm28, {sae}, 123	 # AVX512DQ
	vrangesd	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512DQ
	vrangesd	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512DQ
	vrangess	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512DQ
	vrangess	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512DQ
	vreducepd	zmm30, zmm29, {sae}, 0xab	 # AVX512DQ
	vreducepd	zmm30, zmm29, {sae}, 123	 # AVX512DQ
	vreduceps	zmm30, zmm29, {sae}, 0xab	 # AVX512DQ
	vreduceps	zmm30, zmm29, {sae}, 123	 # AVX512DQ
	vreducesd	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512DQ
	vreducesd	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512DQ
	vreducess	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512DQ
	vreducess	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512DQ
	vcvttpd2qq	zmm30, zmm29, {sae}	 # AVX512DQ
	vcvttpd2uqq	zmm30, zmm29, {sae}	 # AVX512DQ
	vcvttps2qq	zmm30, ymm29, {sae}	 # AVX512DQ
	vcvttps2uqq	zmm30, ymm29, {sae}	 # AVX512DQ
