# Test .arch [.avxX|.noavxX]
	.text
	.arch generic32
	vcvtph2ps %xmm4,%ymm4
	vfmadd132pd %ymm4,%ymm6,%ymm2
	vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
	vfrczpd %xmm7,%xmm7
	.arch .mmx
	.arch .sse2
	emms
	lfence
	vzeroupper
	.arch .avx
	vzeroupper
	vpermpd $7,%ymm6,%ymm2
	.arch .avx2
	vpermpd $7,%ymm6,%ymm2
	.arch .f16c
	vcvtph2ps %xmm4,%ymm4
	.arch .fma
	vfmadd132pd %ymm4,%ymm6,%ymm2
	.arch .fma4
	vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
	.arch .xop
	vfrczpd %xmm7,%xmm7
	.arch .noavx2
	vzeroupper
	vcvtph2ps %xmm4,%ymm4
	vfmadd132pd %ymm4,%ymm6,%ymm2
	vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
	vfrczpd %xmm7,%xmm7
	vpermpd $7,%ymm6,%ymm2
	lfence
	.arch .noavx
	vzeroupper
	vcvtph2ps %xmm4,%ymm4
	vfmadd132pd %ymm4,%ymm6,%ymm2
	vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
	vfrczpd %xmm7,%xmm7
	emms
	lfence
	vaddps	%xmm6, %xmm5, %xmm4
	vaddps	%ymm6, %ymm5, %ymm4
	vaddps	%zmm6, %zmm5, %zmm4
	vaddss	%xmm6, %xmm5, %xmm4
	.p2align 4
