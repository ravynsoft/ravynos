# Check 64bit AVX512_BF16 instructions

	.allow_index_reg
	.text
_start:
	vcvtne2ps2bf16	%zmm28, %zmm29, %zmm30	 #AVX512_BF16
	vcvtne2ps2bf16	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512_BF16 MASK_ENABLING
	vcvtne2ps2bf16	(%r9){1to16}, %zmm29, %zmm30	 #AVX512_BF16 BROADCAST_EN
	vcvtne2ps2bf16	8128(%rcx), %zmm29, %zmm30	 #AVX512_BF16 Disp8
	vcvtne2ps2bf16	-8192(%rdx){1to16}, %zmm29, %zmm30{%k7}{z}	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	%zmm29, %ymm30	 #AVX512_BF16
	vcvtneps2bf16	0x10000000(%rbp, %r14, 8), %ymm30{%k7}	 #AVX512_BF16 MASK_ENABLING
	vcvtneps2bf16	(%r9){1to16}, %ymm30	 #AVX512_BF16 BROADCAST_EN
	vcvtneps2bf16	8128(%rcx), %ymm30	 #AVX512_BF16 Disp8
	vcvtneps2bf16	-8192(%rdx){1to16}, %ymm30{%k7}{z}	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	%zmm28, %zmm29, %zmm30	 #AVX512_BF16
	vdpbf16ps	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512_BF16 MASK_ENABLING
	vdpbf16ps	(%r9){1to16}, %zmm29, %zmm30	 #AVX512_BF16 BROADCAST_EN
	vdpbf16ps	8128(%rcx), %zmm29, %zmm30	 #AVX512_BF16 Disp8
	vdpbf16ps	-8192(%rdx){1to16}, %zmm29, %zmm30{%k7}{z}	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL

.intel_syntax noprefix
	vcvtne2ps2bf16	zmm30, zmm29, zmm28	 #AVX512_BF16
	vcvtne2ps2bf16	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512_BF16 MASK_ENABLING
	vcvtne2ps2bf16	zmm30, zmm29, DWORD BCST [r9]	 #AVX512_BF16 BROADCAST_EN
	vcvtne2ps2bf16	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512_BF16 Disp8
	vcvtne2ps2bf16	zmm30{k7}{z}, zmm29, DWORD BCST [rdx-8192]	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vcvtneps2bf16	ymm30, zmm29	 #AVX512_BF16
	vcvtneps2bf16	ymm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512_BF16 MASK_ENABLING
	vcvtneps2bf16	ymm30, DWORD BCST [r9]	 #AVX512_BF16 BROADCAST_EN
	vcvtneps2bf16	ymm30, ZMMWORD PTR [rcx+8128]	 #AVX512_BF16 Disp8
	vcvtneps2bf16	ymm30{k7}{z}, DWORD BCST [rdx-8192]	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
	vdpbf16ps	zmm30, zmm29, zmm28	 #AVX512_BF16
	vdpbf16ps	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512_BF16 MASK_ENABLING
	vdpbf16ps	zmm30, zmm29, DWORD BCST [r9]	 #AVX512_BF16 BROADCAST_EN
	vdpbf16ps	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512_BF16 Disp8
	vdpbf16ps	zmm30{k7}{z}, zmm29, DWORD BCST [rdx-8192]	 #AVX512_BF16 Disp8 BROADCAST_EN MASK_ENABLING ZEROCTL
