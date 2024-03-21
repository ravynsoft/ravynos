# Check AVX512DQ instructions not to be accepted (in part only outside of 64-bit mode)

	.text
_start:
	       vpextrq	$0, %xmm0, (%eax)
	{evex} vpextrq	$0, %xmm0, (%eax)

	       vpinsrq	$0, (%eax), %xmm0, %xmm0
	{evex} vpinsrq	$0, (%eax), %xmm0, %xmm0

	.intel_syntax noprefix

	       vpextrq	[eax], xmm0, 0
	{evex} vpextrq	[eax], xmm0, 0
	       vpextrq	qword ptr [eax], xmm0, 0
	{evex} vpextrq	qword ptr [eax], xmm0, 0

	       vpinsrq	xmm0, xmm0, [eax], 0
	{evex} vpinsrq	xmm0, xmm0, [eax], 0
	       vpinsrq	xmm0, xmm0, qword ptr [eax], 0
	{evex} vpinsrq	xmm0, xmm0, qword ptr [eax], 0

	vcvtqq2ps	xmm0, [rax]
	vcvtuqq2ps	xmm0, [rax]

	vfpclasspd	k0, [eax], 0
	vfpclassps	k0, [eax+1], 0

	.att_syntax prefix

	vcvtqq2ps	(%eax), %xmm0
	vcvtuqq2ps	(%eax), %xmm0

	vfpclasspd	$0, (%eax), %k0
	vfpclassps	$0, (%eax), %k0
