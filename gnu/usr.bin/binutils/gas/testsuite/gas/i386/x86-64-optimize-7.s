# Check 64bit instructions with optimized encoding

	.allow_index_reg
	.text
_start:
	vandnpd %zmm1, %zmm1, %zmm15{%k7}
	vandnpd %zmm1, %zmm1, %zmm15
	vandnpd %zmm1, %zmm1, %zmm16
	vandnpd %zmm17, %zmm17, %zmm1

	vandnps %zmm1, %zmm1, %zmm15{%k7}
	vandnps %zmm1, %zmm1, %zmm15
	vandnps %zmm1, %zmm1, %zmm16
	vandnps %zmm17, %zmm17, %zmm1

	vpandnd %zmm1, %zmm1, %zmm15{%k7}
	vpandnd %zmm1, %zmm1, %zmm15
	vpandnd %zmm1, %zmm1, %zmm16
	vpandnd %zmm17, %zmm17, %zmm1

	vpandnq %zmm1, %zmm1, %zmm15{%k7}
	vpandnq %zmm1, %zmm1, %zmm15
	vpandnq %zmm1, %zmm1, %zmm16
	vpandnq %zmm17, %zmm17, %zmm1

	vxorpd %zmm1, %zmm1, %zmm15{%k7}
	vxorpd %zmm1, %zmm1, %zmm15
	vxorpd %zmm1, %zmm1, %zmm16
	vxorpd %zmm17, %zmm17, %zmm1

	vxorps %zmm1, %zmm1, %zmm15{%k7}
	vxorps %zmm1, %zmm1, %zmm15
	vxorps %zmm1, %zmm1, %zmm16
	vxorps %zmm17, %zmm17, %zmm1

	vpxord %zmm1, %zmm1, %zmm15{%k7}
	vpxord %zmm1, %zmm1, %zmm15
	vpxord %zmm1, %zmm1, %zmm16
	vpxord %zmm17, %zmm17, %zmm1

	vpxorq %zmm1, %zmm1, %zmm15{%k7}
	vpxorq %zmm1, %zmm1, %zmm15
	vpxorq %zmm1, %zmm1, %zmm16
	vpxorq %zmm17, %zmm17, %zmm1

	vpsubb %zmm1, %zmm1, %zmm15{%k7}
	vpsubb %zmm1, %zmm1, %zmm15
	vpsubb %zmm1, %zmm1, %zmm16
	vpsubb %zmm17, %zmm17, %zmm1

	vpsubw %zmm1, %zmm1, %zmm15{%k7}
	vpsubw %zmm1, %zmm1, %zmm15
	vpsubw %zmm1, %zmm1, %zmm16
	vpsubw %zmm17, %zmm17, %zmm1

	vpsubd %zmm1, %zmm1, %zmm15{%k7}
	vpsubd %zmm1, %zmm1, %zmm15
	vpsubd %zmm1, %zmm1, %zmm16
	vpsubd %zmm17, %zmm17, %zmm1

	vpsubq %zmm1, %zmm1, %zmm15{%k7}
	vpsubq %zmm1, %zmm1, %zmm15
	vpsubq %zmm1, %zmm1, %zmm16
	vpsubq %zmm17, %zmm17, %zmm1
