# Check instructions with optimized encoding

	.allow_index_reg
	.text
_start:
	vandnpd %zmm1, %zmm1, %zmm5{%k7}
	vandnpd %ymm1, %ymm1, %ymm5	 {%k7}		{z}
	vandnpd %zmm1, %zmm1, %zmm5
	vandnpd %ymm1, %ymm1, %ymm5

	vandnps %zmm1, %zmm1, %zmm5{%k7}
	vandnps %ymm1, %ymm1, %ymm5{z}{%k7}
	vandnps %zmm1, %zmm1, %zmm5
	vandnps %ymm1, %ymm1, %ymm5

	vpandn %ymm1, %ymm1, %ymm5

	vpandnd %zmm1, %zmm1, %zmm5{%k7}
	vpandnd %ymm1, %ymm1, %ymm5{z}{%k7}
	vpandnd %zmm1, %zmm1, %zmm5
	vpandnd %ymm1, %ymm1, %ymm5

	vpandnq %zmm1, %zmm1, %zmm5{%k7}
	vpandnq %ymm1, %ymm1, %ymm5{z}{%k7}
	vpandnq %zmm1, %zmm1, %zmm5
	vpandnq %ymm1, %ymm1, %ymm5

	vxorpd %zmm1, %zmm1, %zmm5{%k7}
	vxorpd %ymm1, %ymm1, %ymm5{z}{%k7}
	vxorpd %zmm1, %zmm1, %zmm5
	vxorpd %ymm1, %ymm1, %ymm5

	vxorps %zmm1, %zmm1, %zmm5{%k7}
	vxorps %ymm1, %ymm1, %ymm5{z}{%k7}
	vxorps %zmm1, %zmm1, %zmm5
	vxorps %ymm1, %ymm1, %ymm5

	vpxor %ymm1, %ymm1, %ymm5

	vpxord %zmm1, %zmm1, %zmm5{%k7}
	vpxord %ymm1, %ymm1, %ymm5{z}{%k7}
	vpxord %zmm1, %zmm1, %zmm5
	vpxord %ymm1, %ymm1, %ymm5

	vpxorq %zmm1, %zmm1, %zmm5{%k7}
	vpxorq %ymm1, %ymm1, %ymm5{z}{%k7}
	vpxorq %zmm1, %zmm1, %zmm5
	vpxorq %ymm1, %ymm1, %ymm5

	vpsubb %zmm1, %zmm1, %zmm5{%k7}
	vpsubb %ymm1, %ymm1, %ymm5{z}{%k7}
	vpsubb %zmm1, %zmm1, %zmm5
	vpsubb %ymm1, %ymm1, %ymm5

	vpsubw %zmm1, %zmm1, %zmm5{%k7}
	vpsubw %ymm1, %ymm1, %ymm5{z}{%k7}
	vpsubw %zmm1, %zmm1, %zmm5
	vpsubw %ymm1, %ymm1, %ymm5

	vpsubd %zmm1, %zmm1, %zmm5{%k7}
	vpsubd %ymm1, %ymm1, %ymm5{z}{%k7}
	vpsubd %zmm1, %zmm1, %zmm5
	vpsubd %ymm1, %ymm1, %ymm5

	vpsubq %zmm1, %zmm1, %zmm5{%k7}
	vpsubq %ymm1, %ymm1, %ymm5{z}{%k7}
	vpsubq %zmm1, %zmm1, %zmm5
	vpsubq %ymm1, %ymm1, %ymm5

	kxord %k1, %k1, %k5
	kxorq %k1, %k1, %k5

	kandnd %k1, %k1, %k5
	kandnq %k1, %k1, %k5

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

	vmovdqa32	(%eax), %zmm2

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

	bt	$15, %ax
	bt	$16, %ax
	btc	$15, %ax
	btr	$15, %ax
	bts	$15, %ax

	.code16
	bt	$15, %eax
	bt	$16, %eax
	btc	$15, %eax
	btr	$15, %eax
	bts	$15, %eax
