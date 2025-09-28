# Check instructions with optimized encoding

	.allow_index_reg
	.text
_start:
	testl	$0x7f, %eax
	testw	$0x7f, %ax
	testb	$0x7f, %al
	test	$0x7f, %ebx
	test	$0x7f, %bx
	test	$0x7f, %bl
	test	$0x7f, %edi
	test	$0x7f, %di

	and	%cl, %cl
	and	%dx, %dx
	and	%ebx, %ebx

	or	%ah, %ah
	or	%bp, %bp
	or	%esi, %esi

	lock xchg %ecx, (%edx)
	lock xchg (%ecx), %edx

	vandnpd	%zmm1, %zmm1, %zmm5

	vmovdqa32	%xmm1, %xmm2
	vmovdqa64	%xmm1, %xmm2
	vmovdqu8	%xmm1, %xmm2
	vmovdqu16	%xmm1, %xmm2
	vmovdqu32	%xmm1, %xmm2
	vmovdqu64	%xmm1, %xmm2

	vmovdqa32	127(%eax), %xmm2
	vmovdqa64	127(%eax), %xmm2
	vmovdqu8	127(%eax), %xmm2
	vmovdqu16	127(%eax), %xmm2
	vmovdqu32	127(%eax), %xmm2
	vmovdqu64	127(%eax), %xmm2

	vmovdqa32	%xmm1, 128(%eax)
	vmovdqa64	%xmm1, 128(%eax)
	vmovdqu8	%xmm1, 128(%eax)
	vmovdqu16	%xmm1, 128(%eax)
	vmovdqu32	%xmm1, 128(%eax)
	vmovdqu64	%xmm1, 128(%eax)

	vmovdqa32	%ymm1, %ymm2
	vmovdqa64	%ymm1, %ymm2
	vmovdqu8	%ymm1, %ymm2
	vmovdqu16	%ymm1, %ymm2
	vmovdqu32	%ymm1, %ymm2
	vmovdqu64	%ymm1, %ymm2

	vmovdqa32	127(%eax), %ymm2
	vmovdqa64	127(%eax), %ymm2
	vmovdqu8	127(%eax), %ymm2
	vmovdqu16	127(%eax), %ymm2
	vmovdqu32	127(%eax), %ymm2
	vmovdqu64	127(%eax), %ymm2

	vmovdqa32	%ymm1, 128(%eax)
	vmovdqa64	%ymm1, 128(%eax)
	vmovdqu8	%ymm1, 128(%eax)
	vmovdqu16	%ymm1, 128(%eax)
	vmovdqu32	%ymm1, 128(%eax)
	vmovdqu64	%ymm1, 128(%eax)

	vmovdqa32	%zmm1, %zmm2
	vmovdqa64	%zmm1, %zmm2
	vmovdqu8	%zmm1, %zmm2
	vmovdqu16	%zmm1, %zmm2
	vmovdqu32	%zmm1, %zmm2
	vmovdqu64	%zmm1, %zmm2

	{evex} vmovdqa32	%ymm1, %ymm2
	{evex} vmovdqa64	%ymm1, %ymm2
	{evex} vmovdqu8		%xmm1, %xmm2
	{evex} vmovdqu16	%xmm1, %xmm2
	{evex} vmovdqu32	%xmm1, %xmm2
	{evex} vmovdqu64	%xmm1, %xmm2

	vmovdqa32	%ymm1, %ymm2{%k1}
	vmovdqa64	%ymm1, %ymm2{%k1}
	vmovdqu8	%xmm1, %xmm2{%k1}
	vmovdqu16	%xmm1, %xmm2{%k1}
	vmovdqu32	%xmm1, %xmm2{%k1}
	vmovdqu64	%xmm1, %xmm2{%k1}

	vmovdqa32	(%eax), %ymm2{%k1}
	vmovdqa64	(%eax), %ymm2{%k1}
	vmovdqu8	(%eax), %xmm2{%k1}
	vmovdqu16	(%eax), %xmm2{%k1}
	vmovdqu32	(%eax), %xmm2{%k1}
	vmovdqu64	(%eax), %xmm2{%k1}

	vmovdqa32	%ymm1, (%eax){%k1}
	vmovdqa64	%ymm1, (%eax){%k1}
	vmovdqu8	%xmm1, (%eax){%k1}
	vmovdqu16	%xmm1, (%eax){%k1}
	vmovdqu32	%xmm1, (%eax){%k1}
	vmovdqu64	%xmm1, (%eax){%k1}

	vmovdqa32	%xmm1, %xmm2{%k1}{z}
	vmovdqa64	%xmm1, %xmm2{%k1}{z}
	vmovdqu8	%xmm1, %xmm2{%k1}{z}
	vmovdqu16	%xmm1, %xmm2{%k1}{z}
	vmovdqu32	%xmm1, %xmm2{%k1}{z}
	vmovdqu64	%xmm1, %xmm2{%k1}{z}

	vpandd		%xmm2, %xmm3, %xmm4
	vpandq		%xmm2, %xmm3, %xmm4
	vpandnd		%xmm2, %xmm3, %xmm4
	vpandnq		%xmm2, %xmm3, %xmm4
	vpord		%xmm2, %xmm3, %xmm4
	vporq		%xmm2, %xmm3, %xmm4
	vpxord		%xmm2, %xmm3, %xmm4
	vpxorq		%xmm2, %xmm3, %xmm4

	vpandd		%ymm2, %ymm3, %ymm4
	vpandq		%ymm2, %ymm3, %ymm4
	vpandnd		%ymm2, %ymm3, %ymm4
	vpandnq		%ymm2, %ymm3, %ymm4
	vpord		%ymm2, %ymm3, %ymm4
	vporq		%ymm2, %ymm3, %ymm4
	vpxord		%ymm2, %ymm3, %ymm4
	vpxorq		%ymm2, %ymm3, %ymm4

	vpandd		112(%eax), %xmm2, %xmm3
	vpandq		112(%eax), %xmm2, %xmm3
	vpandnd		112(%eax), %xmm2, %xmm3
	vpandnq		112(%eax), %xmm2, %xmm3
	vpord		112(%eax), %xmm2, %xmm3
	vporq		112(%eax), %xmm2, %xmm3
	vpxord		112(%eax), %xmm2, %xmm3
	vpxorq		112(%eax), %xmm2, %xmm3

	vpandd		128(%eax), %xmm2, %xmm3
	vpandq		128(%eax), %xmm2, %xmm3
	vpandnd		128(%eax), %xmm2, %xmm3
	vpandnq		128(%eax), %xmm2, %xmm3
	vpord		128(%eax), %xmm2, %xmm3
	vporq		128(%eax), %xmm2, %xmm3
	vpxord		128(%eax), %xmm2, %xmm3
	vpxorq		128(%eax), %xmm2, %xmm3

	vpandd		96(%eax), %ymm2, %ymm3
	vpandq		96(%eax), %ymm2, %ymm3
	vpandnd		96(%eax), %ymm2, %ymm3
	vpandnq		96(%eax), %ymm2, %ymm3
	vpord		96(%eax), %ymm2, %ymm3
	vporq		96(%eax), %ymm2, %ymm3
	vpxord		96(%eax), %ymm2, %ymm3
	vpxorq		96(%eax), %ymm2, %ymm3

	vpandd		128(%eax), %ymm2, %ymm3
	vpandq		128(%eax), %ymm2, %ymm3
	vpandnd		128(%eax), %ymm2, %ymm3
	vpandnq		128(%eax), %ymm2, %ymm3
	vpord		128(%eax), %ymm2, %ymm3
	vporq		128(%eax), %ymm2, %ymm3
	vpxord		128(%eax), %ymm2, %ymm3
	vpxorq		128(%eax), %ymm2, %ymm3

	vpandd		%xmm2, %xmm3, %xmm4{%k5}
	vpandq		%ymm2, %ymm3, %ymm4{%k5}
	vpandnd		%ymm2, %ymm3, %ymm4{%k5}
	vpandnq		%xmm2, %xmm3, %xmm4{%k5}
	vpord		%xmm2, %xmm3, %xmm4{%k5}
	vporq		%ymm2, %ymm3, %ymm4{%k5}
	vpxord		%ymm2, %ymm3, %ymm4{%k5}
	vpxorq		%xmm2, %xmm3, %xmm4{%k5}

	vpandd		(%eax){1to8}, %ymm2, %ymm3
	vpandq		(%eax){1to2}, %xmm2, %xmm3
	vpandnd		(%eax){1to4}, %xmm2, %xmm3
	vpandnq		(%eax){1to4}, %ymm2, %ymm3
	vpord		(%eax){1to8}, %ymm2, %ymm3
	vporq		(%eax){1to2}, %xmm2, %xmm3
	vpxord		(%eax){1to4}, %xmm2, %xmm3
	vpxorq		(%eax){1to4}, %ymm2, %ymm3
