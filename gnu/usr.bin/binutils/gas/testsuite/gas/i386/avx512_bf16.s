# Check 32bit AVX512_BF16 instructions

	.allow_index_reg
	.text
_start:
	vcvtne2ps2bf16	%zmm4, %zmm5, %zmm6	 #AVX512_BF16
	vcvtne2ps2bf16	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512_BF16 MASK_ENABLING
	vcvtne2ps2bf16	(%ecx){1to16}, %zmm5, %zmm6	 #AVX512_BF16 BROADCAST_EN
	vcvtne2ps2bf16	8128(%ecx), %zmm5, %zmm6	 #AVX512_BF16 Disp8
	vcvtne2ps2bf16	-8192(%edx){1to16}, %zmm5, %zmm6{%k7}{z}	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	%zmm5, %ymm6	 #AVX512_BF16
	vcvtneps2bf16	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512_BF16 MASK_ENABLING
	vcvtneps2bf16	(%ecx){1to16}, %ymm6	 #AVX512_BF16 BROADCAST_EN
	vcvtneps2bf16	8128(%ecx), %ymm6	 #AVX512_BF16 Disp8
	vcvtneps2bf16	-8192(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	%zmm4, %zmm5, %zmm6	 #AVX512_BF16
	vdpbf16ps	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512_BF16 MASK_ENABLING
	vdpbf16ps	(%ecx){1to16}, %zmm5, %zmm6	 #AVX512_BF16 BROADCAST_EN
	vdpbf16ps	8128(%ecx), %zmm5, %zmm6	 #AVX512_BF16 Disp8
	vdpbf16ps	-8192(%edx){1to16}, %zmm5, %zmm6{%k7}{z}	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL

.intel_syntax noprefix
	vcvtne2ps2bf16	zmm6, zmm5, zmm4	 #AVX512_BF16
	vcvtne2ps2bf16	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512_BF16 MASK_ENABLING
	vcvtne2ps2bf16	zmm6, zmm5, DWORD BCST [ecx]	 #AVX512_BF16 BROADCAST_EN
	vcvtne2ps2bf16	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512_BF16 Disp8
	vcvtne2ps2bf16	zmm6{k7}{z}, zmm5, DWORD BCST [edx-8192]	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	ymm6, zmm5	 #AVX512_BF16
	vcvtneps2bf16	ymm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512_BF16 MASK_ENABLING
	vcvtneps2bf16	ymm6, DWORD BCST [ecx]	 #AVX512_BF16 BROADCAST_EN
	vcvtneps2bf16	ymm6, ZMMWORD PTR [ecx+8128]	 #AVX512_BF16 Disp8
	vcvtneps2bf16	ymm6{k7}{z}, DWORD BCST [edx-8192]	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	zmm6, zmm5, zmm4	 #AVX512_BF16
	vdpbf16ps	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512_BF16 MASK_ENABLING
	vdpbf16ps	zmm6, zmm5, DWORD BCST [ecx]	 #AVX512_BF16 BROADCAST_EN
	vdpbf16ps	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512_BF16 Disp8
	vdpbf16ps	zmm6{k7}{z}, zmm5, DWORD BCST [edx-8192]	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
