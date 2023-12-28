# Check 64bit AVX-VNNI-INT8 instructions

	.allow_index_reg
	.text
_start:
	vpdpbssd	%ymm8, %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbssd	%xmm8, %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbssd	0x10000000(%rbp, %r14, 8), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbssd	(%r9), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbssd	4064(%rcx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbssd	-4096(%rdx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbssd	0x10000000(%rbp, %r14, 8), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbssd	(%r9), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbssd	2032(%rcx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbssd	-2048(%rdx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbssds	%ymm8, %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbssds	%xmm8, %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbssds	0x10000000(%rbp, %r14, 8), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbssds	(%r9), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbssds	4064(%rcx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbssds	-4096(%rdx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbssds	0x10000000(%rbp, %r14, 8), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbssds	(%r9), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbssds	2032(%rcx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbssds	-2048(%rdx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbsud	%ymm8, %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbsud	%xmm8, %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbsud	0x10000000(%rbp, %r14, 8), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbsud	(%r9), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbsud	4064(%rcx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbsud	-4096(%rdx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbsud	0x10000000(%rbp, %r14, 8), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbsud	(%r9), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbsud	2032(%rcx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbsud	-2048(%rdx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbsuds	%ymm8, %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbsuds	%xmm8, %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbsuds	0x10000000(%rbp, %r14, 8), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbsuds	(%r9), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbsuds	4064(%rcx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbsuds	-4096(%rdx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbsuds	0x10000000(%rbp, %r14, 8), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbsuds	(%r9), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbsuds	2032(%rcx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbsuds	-2048(%rdx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbuud	%ymm8, %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbuud	%xmm8, %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbuud	0x10000000(%rbp, %r14, 8), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbuud	(%r9), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbuud	4064(%rcx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbuud	-4096(%rdx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbuud	0x10000000(%rbp, %r14, 8), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbuud	(%r9), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbuud	2032(%rcx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbuud	-2048(%rdx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbuuds	%ymm8, %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbuuds	%xmm8, %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbuuds	0x10000000(%rbp, %r14, 8), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbuuds	(%r9), %ymm9, %ymm10	 #AVX-VNNI-INT8
	vpdpbuuds	4064(%rcx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbuuds	-4096(%rdx), %ymm9, %ymm10	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbuuds	0x10000000(%rbp, %r14, 8), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbuuds	(%r9), %xmm9, %xmm10	 #AVX-VNNI-INT8
	vpdpbuuds	2032(%rcx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbuuds	-2048(%rdx), %xmm9, %xmm10	 #AVX-VNNI-INT8 Disp32(00f8ffff)

.intel_syntax noprefix
	vpdpbssd	ymm10, ymm9, ymm8	 #AVX-VNNI-INT8
	vpdpbssd	xmm10, xmm9, xmm8	 #AVX-VNNI-INT8
	vpdpbssd	ymm10, ymm9, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbssd	ymm10, ymm9, YMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbssd	ymm10, ymm9, YMMWORD PTR [rcx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbssd	ymm10, ymm9, YMMWORD PTR [rdx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbssd	xmm10, xmm9, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbssd	xmm10, xmm9, XMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbssd	xmm10, xmm9, XMMWORD PTR [rcx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbssd	xmm10, xmm9, XMMWORD PTR [rdx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbssds	ymm10, ymm9, ymm8	 #AVX-VNNI-INT8
	vpdpbssds	xmm10, xmm9, xmm8	 #AVX-VNNI-INT8
	vpdpbssds	ymm10, ymm9, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbssds	ymm10, ymm9, YMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbssds	ymm10, ymm9, YMMWORD PTR [rcx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbssds	ymm10, ymm9, YMMWORD PTR [rdx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbssds	xmm10, xmm9, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbssds	xmm10, xmm9, XMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbssds	xmm10, xmm9, XMMWORD PTR [rcx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbssds	xmm10, xmm9, XMMWORD PTR [rdx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbsud	ymm10, ymm9, ymm8	 #AVX-VNNI-INT8
	vpdpbsud	xmm10, xmm9, xmm8	 #AVX-VNNI-INT8
	vpdpbsud	ymm10, ymm9, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbsud	ymm10, ymm9, YMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbsud	ymm10, ymm9, YMMWORD PTR [rcx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbsud	ymm10, ymm9, YMMWORD PTR [rdx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbsud	xmm10, xmm9, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbsud	xmm10, xmm9, XMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbsud	xmm10, xmm9, XMMWORD PTR [rcx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbsud	xmm10, xmm9, XMMWORD PTR [rdx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbsuds	ymm10, ymm9, ymm8	 #AVX-VNNI-INT8
	vpdpbsuds	xmm10, xmm9, xmm8	 #AVX-VNNI-INT8
	vpdpbsuds	ymm10, ymm9, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbsuds	ymm10, ymm9, YMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbsuds	ymm10, ymm9, YMMWORD PTR [rcx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbsuds	ymm10, ymm9, YMMWORD PTR [rdx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbsuds	xmm10, xmm9, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbsuds	xmm10, xmm9, XMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbsuds	xmm10, xmm9, XMMWORD PTR [rcx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbsuds	xmm10, xmm9, XMMWORD PTR [rdx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbuud	ymm10, ymm9, ymm8	 #AVX-VNNI-INT8
	vpdpbuud	xmm10, xmm9, xmm8	 #AVX-VNNI-INT8
	vpdpbuud	ymm10, ymm9, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbuud	ymm10, ymm9, YMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbuud	ymm10, ymm9, YMMWORD PTR [rcx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbuud	ymm10, ymm9, YMMWORD PTR [rdx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbuud	xmm10, xmm9, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbuud	xmm10, xmm9, XMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbuud	xmm10, xmm9, XMMWORD PTR [rcx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbuud	xmm10, xmm9, XMMWORD PTR [rdx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbuuds	ymm10, ymm9, ymm8	 #AVX-VNNI-INT8
	vpdpbuuds	xmm10, xmm9, xmm8	 #AVX-VNNI-INT8
	vpdpbuuds	ymm10, ymm9, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbuuds	ymm10, ymm9, YMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbuuds	ymm10, ymm9, YMMWORD PTR [rcx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbuuds	ymm10, ymm9, YMMWORD PTR [rdx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbuuds	xmm10, xmm9, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbuuds	xmm10, xmm9, XMMWORD PTR [r9]	 #AVX-VNNI-INT8
	vpdpbuuds	xmm10, xmm9, XMMWORD PTR [rcx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbuuds	xmm10, xmm9, XMMWORD PTR [rdx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
