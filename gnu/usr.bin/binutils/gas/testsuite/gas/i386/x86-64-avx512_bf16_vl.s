# Check 64bit AVX512{BF16,VL} instructions

	.allow_index_reg
	.text
_start:
	vcvtne2ps2bf16	%ymm28, %ymm29, %ymm30	 #AVX512{BF16,VL}
	vcvtne2ps2bf16	%xmm28, %xmm29, %xmm30	 #AVX512{BF16,VL}
	vcvtne2ps2bf16	0x10000000(%rbp, %r14, 8), %ymm29, %ymm30{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtne2ps2bf16	(%r9){1to8}, %ymm29, %ymm30	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtne2ps2bf16	4064(%rcx), %ymm29, %ymm30	 #AVX512{BF16,VL} Disp8
	vcvtne2ps2bf16	-4096(%rdx){1to8}, %ymm29, %ymm30{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtne2ps2bf16	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtne2ps2bf16	(%r9){1to4}, %xmm29, %xmm30	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtne2ps2bf16	2032(%rcx), %xmm29, %xmm30	 #AVX512{BF16,VL} Disp8
	vcvtne2ps2bf16	-2048(%rdx){1to4}, %xmm29, %xmm28{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	%xmm29, %xmm30	 #AVX512{BF16,VL}
	vcvtneps2bf16	%ymm29, %xmm30	 #AVX512{BF16,VL}
	vcvtneps2bf16x	0x10000000(%rbp, %r14, 8), %xmm30{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtneps2bf16	(%r9){1to4}, %xmm21	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16x	(%rcx){1to4}, %xmm1	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16x	2032(%rcx), %xmm30	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	-2048(%rdx){1to4}, %xmm29{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	(%r9){1to8}, %xmm22	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16y	(%rcx){1to8}, %xmm2	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16y	4064(%rcx), %xmm23	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	-4096(%rdx){1to8}, %xmm27{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	%ymm28, %ymm29, %ymm30	 #AVX512{BF16,VL}
	vdpbf16ps	%xmm28, %xmm29, %xmm30	 #AVX512{BF16,VL}
	vdpbf16ps	0x10000000(%rbp, %r14, 8), %ymm29, %ymm30{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vdpbf16ps	(%r9){1to8}, %ymm29, %ymm30	 #AVX512{BF16,VL} BROADCAST_EN
	vdpbf16ps	4064(%rcx), %ymm29, %ymm30	 #AVX512{BF16,VL} Disp8
	vdpbf16ps	-4096(%rdx){1to8}, %ymm29, %ymm30{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512{BF16,VL} MASK_ENABLING
	vdpbf16ps	(%r9){1to4}, %xmm29, %xmm30	 #AVX512{BF16,VL} BROADCAST_EN
	vdpbf16ps	2032(%rcx), %xmm29, %xmm30	 #AVX512{BF16,VL} Disp8
	vdpbf16ps	-2048(%rdx){1to4}, %xmm29, %xmm30{%k7}{z}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL

.intel_syntax noprefix
	vcvtne2ps2bf16	ymm30, ymm29, ymm28	 #AVX512{BF16,VL}
	vcvtne2ps2bf16	xmm30, xmm29, xmm28	 #AVX512{BF16,VL}
	vcvtne2ps2bf16	ymm30{k7}, ymm29, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtne2ps2bf16	ymm30, ymm29, DWORD BCST [r9]	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtne2ps2bf16	ymm30, ymm29, YMMWORD PTR [rcx+4064]	 #AVX512{BF16,VL} Disp8
	vcvtne2ps2bf16	ymm30{k7}{z}, ymm29, DWORD BCST [rdx-4096]	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtne2ps2bf16	xmm30{k7}, xmm29, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtne2ps2bf16	xmm30, xmm29, DWORD BCST [r9]	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtne2ps2bf16	xmm30, xmm29, XMMWORD PTR [rcx+2032]	 #AVX512{BF16,VL} Disp8
	vcvtne2ps2bf16	xmm30{k7}{z}, xmm29, DWORD BCST [rdx-2048]	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	xmm30, xmm29	 #AVX512{BF16,VL}
	vcvtneps2bf16	xmm30, ymm29	 #AVX512{BF16,VL}
	vcvtneps2bf16	xmm30{k7}, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vcvtneps2bf16	xmm5, [rcx]{1to4}	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16	xmm25, DWORD BCST [r9]{1to4}	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16	xmm30, XMMWORD PTR [rcx+2032]	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	xmm30{k7}{z}, DWORD BCST [rdx-2048]{1to4}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	xmm4, [rcx]{1to8}	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16	xmm24, DWORD BCST [r9]{1to8}	 #AVX512{BF16,VL} BROADCAST_EN
	vcvtneps2bf16	xmm30, YMMWORD PTR [rcx+4064]	 #AVX512{BF16,VL} Disp8
	vcvtneps2bf16	xmm30{k7}{z}, DWORD BCST [rdx-4096]{1to8}	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	ymm30, ymm29, ymm28	 #AVX512{BF16,VL}
	vdpbf16ps	xmm30, xmm29, xmm28	 #AVX512{BF16,VL}
	vdpbf16ps	ymm30{k7}, ymm29, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vdpbf16ps	ymm30, ymm29, DWORD BCST [r9]	 #AVX512{BF16,VL} BROADCAST_EN
	vdpbf16ps	ymm30, ymm29, YMMWORD PTR [rcx+4064]	 #AVX512{BF16,VL} Disp8
	vdpbf16ps	ymm30{k7}{z}, ymm29, DWORD BCST [rdx-4096]	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	xmm30{k7}, xmm29, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512{BF16,VL} MASK_ENABLING
	vdpbf16ps	xmm30, xmm29, DWORD BCST [r9]	 #AVX512{BF16,VL} BROADCAST_EN
	vdpbf16ps	xmm30, xmm29, XMMWORD PTR [rcx+2032]	 #AVX512{BF16,VL} Disp8
	vdpbf16ps	xmm30{k7}{z}, xmm29, DWORD BCST [rdx-2048]	 #AVX512{BF16,VL} Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
