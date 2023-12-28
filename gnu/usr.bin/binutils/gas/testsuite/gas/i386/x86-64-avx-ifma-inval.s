# Check illegal in AVXIFMA instructions

	.text
	.arch .noavx512ifma
_start:
	vpmadd52huq %xmm2, %xmm4, %xmm2{%k6}
	vpmadd52huq %xmm22, %xmm4, %xmm2{%k1}
	vpmadd52huq %zmm2, %zmm4, %zmm2
