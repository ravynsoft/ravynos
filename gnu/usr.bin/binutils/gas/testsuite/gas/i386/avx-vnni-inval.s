# Check illegal in AVXVNNI instructions

	.text
	.arch .noavx512_vnni
_start:
	vpdpbusd %xmm2, %xmm4, %xmm2{%k6}
	vpdpbusd %zmm2, %zmm4, %zmm2
