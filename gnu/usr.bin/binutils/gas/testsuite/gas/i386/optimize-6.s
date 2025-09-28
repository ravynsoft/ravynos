# Check instructions with optimized encoding

	.allow_index_reg
	.text
_start:
	vandnpd %zmm1, %zmm1, %zmm5{%k7}
	vandnpd %zmm1, %zmm1, %zmm5

	vandnps %zmm1, %zmm1, %zmm5{%k7}
	vandnps %zmm1, %zmm1, %zmm5

	vpandnd %zmm1, %zmm1, %zmm5{%k7}
	vpandnd %zmm1, %zmm1, %zmm5

	vpandnq %zmm1, %zmm1, %zmm5{%k7}
	vpandnq %zmm1, %zmm1, %zmm5

	vxorpd %zmm1, %zmm1, %zmm5{%k7}
	vxorpd %zmm1, %zmm1, %zmm5

	vxorps %zmm1, %zmm1, %zmm5{%k7}
	vxorps %zmm1, %zmm1, %zmm5

	vpxord %zmm1, %zmm1, %zmm5{%k7}
	vpxord %zmm1, %zmm1, %zmm5

	vpxorq %zmm1, %zmm1, %zmm5{%k7}
	vpxorq %zmm1, %zmm1, %zmm5

	vpsubb %zmm1, %zmm1, %zmm5{%k7}
	vpsubb %zmm1, %zmm1, %zmm5

	vpsubw %zmm1, %zmm1, %zmm5{%k7}
	vpsubw %zmm1, %zmm1, %zmm5

	vpsubd %zmm1, %zmm1, %zmm5{%k7}
	vpsubd %zmm1, %zmm1, %zmm5

	vpsubq %zmm1, %zmm1, %zmm5{%k7}
	vpsubq %zmm1, %zmm1, %zmm5

	kxord %k1, %k1, %k5
	kxorq %k1, %k1, %k5

	kandnd %k1, %k1, %k5
	kandnq %k1, %k1, %k5
