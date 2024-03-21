# Check 32bit AVX512{BF16,VL} instructions

	.allow_index_reg
	.text
_start:
	vcvtne2ps2bf16	%ymm4, %ymm5, %ymm6	 #AVX512{BF16,VL}
	vcvtne2ps2bf16	%xmm4, %xmm5, %xmm6	 #AVX512{BF16,VL}
	vcvtne2ps2bf16	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtne2ps2bf16	(%ecx){1to8}, %ymm5, %ymm6	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtne2ps2bf16	4064(%ecx), %ymm5, %ymm6	 #AVX512{BF16,VL} Disp8
	vcvtne2ps2bf16	-4096(%edx){1to8}, %ymm5, %ymm6{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtne2ps2bf16	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtne2ps2bf16	(%ecx){1to4}, %xmm5, %xmm6	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtne2ps2bf16	2032(%ecx), %xmm5, %xmm6	 #AVX512{BF16,VL} Disp8
	vcvtne2ps2bf16	-2048(%edx){1to4}, %xmm5, %xmm6{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	%xmm5, %xmm6	 #AVX512{BF16,VL}
	vcvtneps2bf16	%ymm5, %xmm6	 #AVX512{BF16,VL}
	vcvtneps2bf16x	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtneps2bf16	(%ecx){1to4}, %xmm6	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16x	(%ecx){1to4}, %xmm6	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16x	2032(%ecx), %xmm6	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	-2048(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	(%ecx){1to8}, %xmm6	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16y	(%ecx){1to8}, %xmm6	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16y	4064(%ecx), %xmm6	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	-4096(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	%ymm4, %ymm5, %ymm6	 #AVX512{BF16,VL}
	vdpbf16ps	%xmm4, %xmm5, %xmm6	 #AVX512{BF16,VL}
	vdpbf16ps	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vdpbf16ps	(%ecx){1to8}, %ymm5, %ymm6	 #AVX512{BF16,VL} BROADCAST_EN
	vdpbf16ps	4064(%ecx), %ymm5, %ymm6	 #AVX512{BF16,VL} Disp8
	vdpbf16ps	-4096(%edx){1to8}, %ymm5, %ymm6{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vdpbf16ps	(%ecx){1to4}, %xmm5, %xmm6	 #AVX512{BF16,VL} BROADCAST_EN
	vdpbf16ps	2032(%ecx), %xmm5, %xmm6	 #AVX512{BF16,VL} Disp8
	vdpbf16ps	-2048(%edx){1to4}, %xmm5, %xmm6{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL

.intel_syntax noprefix
	vcvtne2ps2bf16	ymm6, ymm5, ymm4	 #AVX512{BF16,VL}
	vcvtne2ps2bf16	xmm6, xmm5, xmm4	 #AVX512{BF16,VL}
	vcvtne2ps2bf16	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtne2ps2bf16	ymm6, ymm5, DWORD BCST [ecx]	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtne2ps2bf16	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512{BF16,VL} Disp8
	vcvtne2ps2bf16	ymm6{k7}{z}, ymm5, DWORD BCST [edx-4096]	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtne2ps2bf16	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtne2ps2bf16	xmm6, xmm5, DWORD BCST [ecx]	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtne2ps2bf16	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512{BF16,VL} Disp8
	vcvtne2ps2bf16	xmm6{k7}{z}, xmm5, DWORD BCST [edx-2048]	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	xmm6, xmm5	 #AVX512{BF16,VL}
	vcvtneps2bf16	xmm6, ymm5	 #AVX512{BF16,VL}
	vcvtneps2bf16	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtneps2bf16	xmm6, [ecx]{1to4}	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16	xmm6, DWORD BCST [ecx]{1to4}	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	xmm6{k7}{z}, DWORD BCST [edx-2048]{1to4}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	xmm6, [ecx]{1to8}	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16	xmm6, DWORD BCST [ecx]{1to8}	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16	xmm6, YMMWORD PTR [ecx+4064]	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	xmm6{k7}{z}, DWORD BCST [edx-4096]{1to8}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	ymm6, ymm5, ymm4	 #AVX512{BF16,VL}
	vdpbf16ps	xmm6, xmm5, xmm4	 #AVX512{BF16,VL}
	vdpbf16ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vdpbf16ps	ymm6, ymm5, DWORD BCST [ecx]	 #AVX512{BF16,VL} BROADCAST_EN
	vdpbf16ps	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512{BF16,VL} Disp8
	vdpbf16ps	ymm6{k7}{z}, ymm5, DWORD BCST [edx-4096]	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vdpbf16ps	xmm6, xmm5, DWORD BCST [ecx]	 #AVX512{BF16,VL} BROADCAST_EN
	vdpbf16ps	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512{BF16,VL} Disp8
	vdpbf16ps	xmm6{k7}{z}, xmm5, DWORD BCST [edx-2048]	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
