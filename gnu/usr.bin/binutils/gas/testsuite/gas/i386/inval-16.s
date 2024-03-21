	.text
	.arch i186; .code16
	vmovapd	%xmm0,%xmm1
	vaddsd	%xmm4, %xmm5, %xmm6{%k7}
	vfrczpd	%xmm7,%xmm7
	andn    (%eax), %ecx, %ecx
	bzhi    %ecx, (%eax), %ecx
	llwpcb	%ecx
	blcfill	%ecx, %ecx
