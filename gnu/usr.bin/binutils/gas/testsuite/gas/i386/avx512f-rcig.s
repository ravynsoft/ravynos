# Check 32bit AVX512F-RCIG instructions

	.allow_index_reg
	.text
_start:
	vcmppd	$0xab, {sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmppd	$123, {sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpps	$0xab, {sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpps	$123, {sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpsd	$0xab, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpsd	$123, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$0xab, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$123, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcomisd	{sae}, %xmm5, %xmm6	 # AVX512F
	vcomiss	{sae}, %xmm5, %xmm6	 # AVX512F
	vcvtph2ps	{sae}, %ymm5, %zmm6{%k7}	 # AVX512F
	vcvtps2pd	{sae}, %ymm5, %zmm6{%k7}	 # AVX512F
	vcvtps2ph	$0xab, {sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtps2ph	$123, {sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtss2sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvttpd2dq	{sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvttps2dq	{sae}, %zmm5, %zmm6	 # AVX512F
	vcvttsd2si	{sae}, %xmm6, %eax	 # AVX512F
	vcvttsd2si	{sae}, %xmm6, %ebp	 # AVX512F
	vcvttss2si	{sae}, %xmm6, %eax	 # AVX512F
	vcvttss2si	{sae}, %xmm6, %ebp	 # AVX512F
	vgetexppd	{sae}, %zmm5, %zmm6	 # AVX512F
	vgetexpps	{sae}, %zmm5, %zmm6	 # AVX512F
	vgetexpsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantpd	$0xab, {sae}, %zmm5, %zmm6	 # AVX512F
	vgetmantpd	$123, {sae}, %zmm5, %zmm6	 # AVX512F
	vgetmantps	$0xab, {sae}, %zmm5, %zmm6	 # AVX512F
	vgetmantps	$123, {sae}, %zmm5, %zmm6	 # AVX512F
	vgetmantsd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantsd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxpd	{sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmaxps	{sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmaxsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vminpd	{sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vminps	{sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vminsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vminss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vucomisd	{sae}, %xmm5, %xmm6	 # AVX512F
	vucomiss	{sae}, %xmm5, %xmm6	 # AVX512F
	vfixupimmpd	$0xab, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$0xab, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmsd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmsd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalepd	$0xab, {sae}, %zmm5, %zmm6	 # AVX512F
	vrndscalepd	$123, {sae}, %zmm5, %zmm6	 # AVX512F
	vrndscaleps	$0xab, {sae}, %zmm5, %zmm6	 # AVX512F
	vrndscaleps	$123, {sae}, %zmm5, %zmm6	 # AVX512F
	vrndscalesd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvttpd2udq	{sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvttps2udq	{sae}, %zmm5, %zmm6	 # AVX512F
	vcvttsd2usi	{sae}, %xmm6, %eax	 # AVX512F
	vcvttsd2usi	{sae}, %xmm6, %ebp	 # AVX512F
	vcvttss2usi	{sae}, %xmm6, %eax	 # AVX512F
	vcvttss2usi	{sae}, %xmm6, %ebp	 # AVX512F

	.intel_syntax noprefix
	vcmppd	k5, zmm6, zmm5, {sae}, 0xab	 # AVX512F
	vcmppd	k5, zmm6, zmm5, {sae}, 123	 # AVX512F
	vcmpps	k5, zmm6, zmm5, {sae}, 0xab	 # AVX512F
	vcmpps	k5, zmm6, zmm5, {sae}, 123	 # AVX512F
	vcmpsd	k5{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512F
	vcmpsd	k5{k7}, xmm5, xmm4, {sae}, 123	 # AVX512F
	vcmpss	k5{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512F
	vcmpss	k5{k7}, xmm5, xmm4, {sae}, 123	 # AVX512F
	vcomisd	xmm6, xmm5, {sae}	 # AVX512F
	vcomiss	xmm6, xmm5, {sae}	 # AVX512F
	vcvtph2ps	zmm6{k7}, ymm5, {sae}	 # AVX512F
	vcvtps2pd	zmm6{k7}, ymm5, {sae}	 # AVX512F
	vcvtps2ph	ymm6{k7}, zmm5, {sae}, 0xab	 # AVX512F
	vcvtps2ph	ymm6{k7}, zmm5, {sae}, 123	 # AVX512F
	vcvtss2sd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512F
	vcvttpd2dq	ymm6{k7}, zmm5, {sae}	 # AVX512F
	vcvttps2dq	zmm6, zmm5, {sae}	 # AVX512F
	vcvttsd2si	eax, xmm6, {sae}	 # AVX512F
	vcvttsd2si	ebp, xmm6, {sae}	 # AVX512F
	vcvttss2si	eax, xmm6, {sae}	 # AVX512F
	vcvttss2si	ebp, xmm6, {sae}	 # AVX512F
	vgetexppd	zmm6, zmm5, {sae}	 # AVX512F
	vgetexpps	zmm6, zmm5, {sae}	 # AVX512F
	vgetexpsd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512F
	vgetexpss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512F
	vgetmantpd	zmm6, zmm5, {sae}, 0xab	 # AVX512F
	vgetmantpd	zmm6, zmm5, {sae}, 123	 # AVX512F
	vgetmantps	zmm6, zmm5, {sae}, 0xab	 # AVX512F
	vgetmantps	zmm6, zmm5, {sae}, 123	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512F
	vmaxpd	zmm6, zmm5, zmm4, {sae}	 # AVX512F
	vmaxps	zmm6, zmm5, zmm4, {sae}	 # AVX512F
	vmaxsd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512F
	vmaxss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512F
	vminpd	zmm6, zmm5, zmm4, {sae}	 # AVX512F
	vminps	zmm6, zmm5, zmm4, {sae}	 # AVX512F
	vminsd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512F
	vminss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512F
	vucomisd	xmm6, xmm5, {sae}	 # AVX512F
	vucomiss	xmm6, xmm5, {sae}	 # AVX512F
	vfixupimmpd	zmm6, zmm5, zmm4, {sae}, 0xab	 # AVX512F
	vfixupimmpd	zmm6, zmm5, zmm4, {sae}, 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, zmm4, {sae}, 0xab	 # AVX512F
	vfixupimmps	zmm6, zmm5, zmm4, {sae}, 123	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512F
	vrndscalepd	zmm6, zmm5, {sae}, 0xab	 # AVX512F
	vrndscalepd	zmm6, zmm5, {sae}, 123	 # AVX512F
	vrndscaleps	zmm6, zmm5, {sae}, 0xab	 # AVX512F
	vrndscaleps	zmm6, zmm5, {sae}, 123	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512F
	vcvttpd2udq	ymm6{k7}, zmm5, {sae}	 # AVX512F
	vcvttps2udq	zmm6, zmm5, {sae}	 # AVX512F
	vcvttsd2usi	eax, xmm6, {sae}	 # AVX512F
	vcvttsd2usi	ebp, xmm6, {sae}	 # AVX512F
	vcvttss2usi	eax, xmm6, {sae}	 # AVX512F
	vcvttss2usi	ebp, xmm6, {sae}	 # AVX512F
