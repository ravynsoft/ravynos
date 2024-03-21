# Test .arch .noavx512vl
	.text
	vpabsb %zmm5, %zmm6{%k7}		# AVX512BW
	vpabsb %xmm5, %xmm6{%k7}		# AVX512BW + AVX512VL
	vpabsb %ymm5, %ymm6{%k7}		# AVX512BW + AVX512VL
	vpconflictd %zmm5, %zmm6		# AVX412CD
	vpconflictd %xmm5, %xmm6		# AVX412CD + AVX512VL
	vpconflictd %ymm5, %ymm6		# AVX412CD + AVX512VL
	vcvtpd2qq (%ecx), %zmm6{%k7}		# AVX512DQ
	vcvtpd2qq (%ecx), %xmm6{%k7}		# AVX512DQ + AVX512VL
	vcvtpd2qq (%ecx), %ymm6{%k7}		# AVX512DQ + AVX512VL
	vexp2ps %zmm5, %zmm6{%k7}		# AVX512ER
	vaddpd %zmm4, %zmm5, %zmm6{%k7}		# AVX512F
	vaddpd %xmm4, %xmm5, %xmm6{%k7}		# AVX512F + AVX512VL
	vaddpd %ymm4, %ymm5, %ymm6{%k7}		# AVX512F + AVX512VL
	vpmadd52luq %zmm4, %zmm5, %zmm6{%k7}	# AVX512IFMA
	vpmadd52luq %xmm4, %xmm5, %xmm6{%k7}	# AVX512IFMA + AVX512VL
	vpmadd52luq %ymm4, %ymm5, %ymm6{%k7}	# AVX512IFMA + AVX512VL
	vgatherpf0dpd 23(%ebp,%ymm7,8){%k1}	# AVX512PF
	vpermb %zmm4, %zmm5, %zmm6{%k7}		# AVX512VBMI
	vpermb %xmm4, %xmm5, %xmm6{%k7}		# AVX512VBMI + AVX512VL
	vpermb %ymm4, %ymm5, %ymm6{%k7}		# AVX512VBMI + AVX512VL

	.arch .noavx512vl
	vpabsb %zmm5, %zmm6{%k7}		# AVX512BW
	vpabsb %xmm5, %xmm6{%k7}		# AVX512BW + AVX512VL
	vpabsb %ymm5, %ymm6{%k7}		# AVX512BW + AVX512VL
	vpconflictd %zmm5, %zmm6		# AVX412CD
	vpconflictd %xmm5, %xmm6		# AVX412CD + AVX512VL
	vpconflictd %ymm5, %ymm6		# AVX412CD + AVX512VL
	vcvtpd2qq (%ecx), %zmm6{%k7}		# AVX512DQ
	vcvtpd2qq (%ecx), %xmm6{%k7}		# AVX512DQ + AVX512VL
	vcvtpd2qq (%ecx), %ymm6{%k7}		# AVX512DQ + AVX512VL
	vexp2ps %zmm5, %zmm6{%k7}		# AVX512ER
	vaddpd %zmm4, %zmm5, %zmm6{%k7}		# AVX512F
	vaddpd %xmm4, %xmm5, %xmm6{%k7}		# AVX512F + AVX512VL
	vaddpd %ymm4, %ymm5, %ymm6{%k7}		# AVX512F + AVX512VL
	vpmadd52luq %zmm4, %zmm5, %zmm6{%k7}	# AVX512IFMA
	vpmadd52luq %xmm4, %xmm5, %xmm6{%k7}	# AVX512IFMA + AVX512VL
	vpmadd52luq %ymm4, %ymm5, %ymm6{%k7}	# AVX512IFMA + AVX512VL
	vgatherpf0dpd 23(%ebp,%ymm7,8){%k1}	# AVX512PF
	vpermb %zmm4, %zmm5, %zmm6{%k7}		# AVX512VBMI
	vpermb %xmm4, %xmm5, %xmm6{%k7}		# AVX512VBMI + AVX512VL
	vpermb %ymm4, %ymm5, %ymm6{%k7}		# AVX512VBMI + AVX512VL

	vpabsb %xmm5, %xmm6
	vpabsb %ymm5, %ymm6
	vaddpd %xmm4, %xmm5, %xmm6
	vaddpd %ymm4, %ymm5, %ymm6
	pabsb %xmm5, %xmm6
	addpd %xmm4, %xmm6

	.intel_syntax noprefix
	vfpclasspd k0, [eax], 0
	vfpclassps k0, [eax+0x80], 0

	.p2align 4
