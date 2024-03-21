# Check 64bit instructions with optimized encoding

	.allow_index_reg
	.text
_start:
	vandnpd %zmm1, %zmm1, %zmm15{%k7}
	vandnpd %ymm1, %ymm1, %ymm15	 {%k7}		{z}
	vandnpd %zmm1, %zmm1, %zmm15
	vandnpd %ymm1, %ymm1, %ymm15
	vandnpd %zmm1, %zmm1, %zmm16
	vandnpd %ymm1, %ymm1, %ymm16
	vandnpd %zmm17, %zmm17, %zmm1
	vandnpd %ymm17, %ymm17, %ymm1

	vandnps %zmm1, %zmm1, %zmm15{%k7}
	vandnps %ymm1, %ymm1, %ymm15{z}{%k7}
	vandnps %zmm1, %zmm1, %zmm15
	vandnps %ymm1, %ymm1, %ymm15
	vandnps %zmm1, %zmm1, %zmm16
	vandnps %ymm1, %ymm1, %ymm16
	vandnps %zmm17, %zmm17, %zmm1
	vandnps %ymm17, %ymm17, %ymm1

	vpandn %ymm1, %ymm1, %ymm15

	vpandnd %zmm1, %zmm1, %zmm15{%k7}
	vpandnd %ymm1, %ymm1, %ymm15{z}{%k7}
	vpandnd %zmm1, %zmm1, %zmm15
	vpandnd %ymm1, %ymm1, %ymm15
	vpandnd %zmm1, %zmm1, %zmm16
	vpandnd %ymm1, %ymm1, %ymm16
	vpandnd %zmm17, %zmm17, %zmm1
	vpandnd %ymm17, %ymm17, %ymm1

	vpandnq %zmm1, %zmm1, %zmm15{%k7}
	vpandnq %ymm1, %ymm1, %ymm15{z}{%k7}
	vpandnq %zmm1, %zmm1, %zmm15
	vpandnq %ymm1, %ymm1, %ymm15
	vpandnq %zmm1, %zmm1, %zmm16
	vpandnq %ymm1, %ymm1, %ymm16
	vpandnq %zmm17, %zmm17, %zmm1
	vpandnq %ymm17, %ymm17, %ymm1

	vxorpd %zmm1, %zmm1, %zmm15{%k7}
	vxorpd %ymm1, %ymm1, %ymm15{z}{%k7}
	vxorpd %zmm1, %zmm1, %zmm15
	vxorpd %ymm1, %ymm1, %ymm15
	vxorpd %zmm1, %zmm1, %zmm16
	vxorpd %ymm1, %ymm1, %ymm16
	vxorpd %zmm17, %zmm17, %zmm1
	vxorpd %ymm17, %ymm17, %ymm1

	vxorps %zmm1, %zmm1, %zmm15{%k7}
	vxorps %ymm1, %ymm1, %ymm15{z}{%k7}
	vxorps %zmm1, %zmm1, %zmm15
	vxorps %ymm1, %ymm1, %ymm15
	vxorps %zmm1, %zmm1, %zmm16
	vxorps %ymm1, %ymm1, %ymm16
	vxorps %zmm17, %zmm17, %zmm1
	vxorps %ymm17, %ymm17, %ymm1

	vpxor %ymm1, %ymm1, %ymm15

	vpxord %zmm1, %zmm1, %zmm15{%k7}
	vpxord %ymm1, %ymm1, %ymm15{z}{%k7}
	vpxord %zmm1, %zmm1, %zmm15
	vpxord %ymm1, %ymm1, %ymm15
	vpxord %zmm1, %zmm1, %zmm16
	vpxord %ymm1, %ymm1, %ymm16
	vpxord %zmm17, %zmm17, %zmm1
	vpxord %ymm17, %ymm17, %ymm1

	vpxorq %zmm1, %zmm1, %zmm15{%k7}
	vpxorq %ymm1, %ymm1, %ymm15{z}{%k7}
	vpxorq %zmm1, %zmm1, %zmm15
	vpxorq %ymm1, %ymm1, %ymm15
	vpxorq %zmm1, %zmm1, %zmm16
	vpxorq %ymm1, %ymm1, %ymm16
	vpxorq %zmm17, %zmm17, %zmm1
	vpxorq %ymm17, %ymm17, %ymm1

	vpsubb %zmm1, %zmm1, %zmm15{%k7}
	vpsubb %ymm1, %ymm1, %ymm15{z}{%k7}
	vpsubb %zmm1, %zmm1, %zmm15
	vpsubb %ymm1, %ymm1, %ymm15
	vpsubb %zmm1, %zmm1, %zmm16
	vpsubb %ymm1, %ymm1, %ymm16
	vpsubb %zmm17, %zmm17, %zmm1
	vpsubb %ymm17, %ymm17, %ymm1

	vpsubw %zmm1, %zmm1, %zmm15{%k7}
	vpsubw %ymm1, %ymm1, %ymm15{z}{%k7}
	vpsubw %zmm1, %zmm1, %zmm15
	vpsubw %ymm1, %ymm1, %ymm15
	vpsubw %zmm1, %zmm1, %zmm16
	vpsubw %ymm1, %ymm1, %ymm16
	vpsubw %zmm17, %zmm17, %zmm1
	vpsubw %ymm17, %ymm17, %ymm1

	vpsubd %zmm1, %zmm1, %zmm15{%k7}
	vpsubd %ymm1, %ymm1, %ymm15{z}{%k7}
	vpsubd %zmm1, %zmm1, %zmm15
	vpsubd %ymm1, %ymm1, %ymm15
	vpsubd %zmm1, %zmm1, %zmm16
	vpsubd %ymm1, %ymm1, %ymm16
	vpsubd %zmm17, %zmm17, %zmm1
	vpsubd %ymm17, %ymm17, %ymm1

	vpsubq %zmm1, %zmm1, %zmm15{%k7}
	vpsubq %ymm1, %ymm1, %ymm15{z}{%k7}
	vpsubq %zmm1, %zmm1, %zmm15
	vpsubq %ymm1, %ymm1, %ymm15
	vpsubq %zmm1, %zmm1, %zmm16
	vpsubq %ymm1, %ymm1, %ymm16
	vpsubq %zmm17, %zmm17, %zmm1
	vpsubq %ymm17, %ymm17, %ymm1

	vmovdqa32	%xmm1, %xmm2
	vmovdqa64	%xmm1, %xmm2
	vmovdqu8	%xmm1, %xmm2
	vmovdqu16	%xmm1, %xmm2
	vmovdqu32	%xmm1, %xmm2
	vmovdqu64	%xmm1, %xmm2

	vmovdqa32	%xmm11, %xmm12
	vmovdqa64	%xmm11, %xmm12
	vmovdqu8	%xmm11, %xmm12
	vmovdqu16	%xmm11, %xmm12
	vmovdqu32	%xmm11, %xmm12
	vmovdqu64	%xmm11, %xmm12

	vmovdqa32	127(%rax), %xmm2
	vmovdqa64	127(%rax), %xmm2
	vmovdqu8	127(%rax), %xmm2
	vmovdqu16	127(%rax), %xmm2
	vmovdqu32	127(%rax), %xmm2
	vmovdqu64	127(%rax), %xmm2

	vmovdqa32	%xmm1, 128(%rax)
	vmovdqa64	%xmm1, 128(%rax)
	vmovdqu8	%xmm1, 128(%rax)
	vmovdqu16	%xmm1, 128(%rax)
	vmovdqu32	%xmm1, 128(%rax)
	vmovdqu64	%xmm1, 128(%rax)

	vmovdqa32	%ymm1, %ymm2
	vmovdqa64	%ymm1, %ymm2
	vmovdqu8	%ymm1, %ymm2
	vmovdqu16	%ymm1, %ymm2
	vmovdqu32	%ymm1, %ymm2
	vmovdqu64	%ymm1, %ymm2

	vmovdqa32	%ymm11, %ymm12
	vmovdqa64	%ymm11, %ymm12
	vmovdqu8	%ymm11, %ymm12
	vmovdqu16	%ymm11, %ymm12
	vmovdqu32	%ymm11, %ymm12
	vmovdqu64	%ymm11, %ymm12

	vmovdqa32	127(%rax), %ymm2
	vmovdqa64	127(%rax), %ymm2
	vmovdqu8	127(%rax), %ymm2
	vmovdqu16	127(%rax), %ymm2
	vmovdqu32	127(%rax), %ymm2
	vmovdqu64	127(%rax), %ymm2

	vmovdqa32	%ymm1, 128(%rax)
	vmovdqa64	%ymm1, 128(%rax)
	vmovdqu8	%ymm1, 128(%rax)
	vmovdqu16	%ymm1, 128(%rax)
	vmovdqu32	%ymm1, 128(%rax)
	vmovdqu64	%ymm1, 128(%rax)

	vmovdqa32	(%rax), %zmm2

	vpandd		%xmm2, %xmm3, %xmm4
	vpandq		%xmm12, %xmm3, %xmm4
	vpandnd		%xmm2, %xmm13, %xmm4
	vpandnq		%xmm2, %xmm3, %xmm14
	vpord		%xmm2, %xmm3, %xmm4
	vporq		%xmm12, %xmm3, %xmm4
	vpxord		%xmm2, %xmm13, %xmm4
	vpxorq		%xmm2, %xmm3, %xmm14

	vpandd		%ymm2, %ymm3, %ymm4
	vpandq		%ymm12, %ymm3, %ymm4
	vpandnd		%ymm2, %ymm13, %ymm4
	vpandnq		%ymm2, %ymm3, %ymm14
	vpord		%ymm2, %ymm3, %ymm4
	vporq		%ymm12, %ymm3, %ymm4
	vpxord		%ymm2, %ymm13, %ymm4
	vpxorq		%ymm2, %ymm3, %ymm14

	vpandd		112(%rax), %xmm2, %xmm3
	vpandq		112(%rax), %xmm2, %xmm3
	vpandnd		112(%rax), %xmm2, %xmm3
	vpandnq		112(%rax), %xmm2, %xmm3
	vpord		112(%rax), %xmm2, %xmm3
	vporq		112(%rax), %xmm2, %xmm3
	vpxord		112(%rax), %xmm2, %xmm3
	vpxorq		112(%rax), %xmm2, %xmm3

	vpandd		128(%rax), %xmm2, %xmm3
	vpandq		128(%rax), %xmm2, %xmm3
	vpandnd		128(%rax), %xmm2, %xmm3
	vpandnq		128(%rax), %xmm2, %xmm3
	vpord		128(%rax), %xmm2, %xmm3
	vporq		128(%rax), %xmm2, %xmm3
	vpxord		128(%rax), %xmm2, %xmm3
	vpxorq		128(%rax), %xmm2, %xmm3

	vpandd		96(%rax), %ymm2, %ymm3
	vpandq		96(%rax), %ymm2, %ymm3
	vpandnd		96(%rax), %ymm2, %ymm3
	vpandnq		96(%rax), %ymm2, %ymm3
	vpord		96(%rax), %ymm2, %ymm3
	vporq		96(%rax), %ymm2, %ymm3
	vpxord		96(%rax), %ymm2, %ymm3
	vpxorq		96(%rax), %ymm2, %ymm3

	vpandd		128(%rax), %ymm2, %ymm3
	vpandq		128(%rax), %ymm2, %ymm3
	vpandnd		128(%rax), %ymm2, %ymm3
	vpandnq		128(%rax), %ymm2, %ymm3
	vpord		128(%rax), %ymm2, %ymm3
	vporq		128(%rax), %ymm2, %ymm3
	vpxord		128(%rax), %ymm2, %ymm3
	vpxorq		128(%rax), %ymm2, %ymm3
