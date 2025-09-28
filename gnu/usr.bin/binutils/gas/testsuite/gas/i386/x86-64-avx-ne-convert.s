# Check 64bit AVX-NE-CONVERT instructions

	.allow_index_reg
	.text
_start:
	vbcstnebf162ps	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	vbcstnebf162ps	(%r9), %xmm6	 #AVX-NE-CONVERT
	vbcstnebf162ps	254(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(fe000000)
	vbcstnebf162ps	-256(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00ffffff)
	vbcstnebf162ps	0x10000000(%rbp, %r14, 8), %ymm6	 #AVX-NE-CONVERT
	vbcstnebf162ps	(%r9), %ymm6	 #AVX-NE-CONVERT
	vbcstnebf162ps	254(%rcx), %ymm6	 #AVX-NE-CONVERT Disp32(fe000000)
	vbcstnebf162ps	-256(%rdx), %ymm6	 #AVX-NE-CONVERT Disp32(00ffffff)
	vbcstnesh2ps	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	vbcstnesh2ps	(%r9), %xmm6	 #AVX-NE-CONVERT
	vbcstnesh2ps	254(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(fe000000)
	vbcstnesh2ps	-256(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00ffffff)
	vbcstnesh2ps	0x10000000(%rbp, %r14, 8), %ymm6	 #AVX-NE-CONVERT
	vbcstnesh2ps	(%r9), %ymm6	 #AVX-NE-CONVERT
	vbcstnesh2ps	254(%rcx), %ymm6	 #AVX-NE-CONVERT Disp32(fe000000)
	vbcstnesh2ps	-256(%rdx), %ymm6	 #AVX-NE-CONVERT Disp32(00ffffff)
	vcvtneebf162ps	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	vcvtneebf162ps	(%r9), %xmm6	 #AVX-NE-CONVERT
	vcvtneebf162ps	2032(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneebf162ps	-2048(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneebf162ps	0x10000000(%rbp, %r14, 8), %ymm6	 #AVX-NE-CONVERT
	vcvtneebf162ps	(%r9), %ymm6	 #AVX-NE-CONVERT
	vcvtneebf162ps	4064(%rcx), %ymm6	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneebf162ps	-4096(%rdx), %ymm6	 #AVX-NE-CONVERT Disp32(00f0ffff)
	vcvtneeph2ps	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	vcvtneeph2ps	(%r9), %xmm6	 #AVX-NE-CONVERT
	vcvtneeph2ps	2032(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneeph2ps	-2048(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneeph2ps	0x10000000(%rbp, %r14, 8), %ymm6	 #AVX-NE-CONVERT
	vcvtneeph2ps	(%r9), %ymm6	 #AVX-NE-CONVERT
	vcvtneeph2ps	4064(%rcx), %ymm6	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneeph2ps	-4096(%rdx), %ymm6	 #AVX-NE-CONVERT Disp32(00f0ffff)
	vcvtneobf162ps	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	vcvtneobf162ps	(%r9), %xmm6	 #AVX-NE-CONVERT
	vcvtneobf162ps	2032(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneobf162ps	-2048(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneobf162ps	0x10000000(%rbp, %r14, 8), %ymm6	 #AVX-NE-CONVERT
	vcvtneobf162ps	(%r9), %ymm6	 #AVX-NE-CONVERT
	vcvtneobf162ps	4064(%rcx), %ymm6	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneobf162ps	-4096(%rdx), %ymm6	 #AVX-NE-CONVERT Disp32(00f0ffff)
	vcvtneoph2ps	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	vcvtneoph2ps	(%r9), %xmm6	 #AVX-NE-CONVERT
	vcvtneoph2ps	2032(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneoph2ps	-2048(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneoph2ps	0x10000000(%rbp, %r14, 8), %ymm6	 #AVX-NE-CONVERT
	vcvtneoph2ps	(%r9), %ymm6	 #AVX-NE-CONVERT
	vcvtneoph2ps	4064(%rcx), %ymm6	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneoph2ps	-4096(%rdx), %ymm6	 #AVX-NE-CONVERT Disp32(00f0ffff)
	vcvtneps2bf16	%xmm5, %xmm6	 #AVX-NE-CONVERT
	{evex} vcvtneps2bf16	%xmm5, %xmm6	 #AVX-NE-CONVERT
	{vex} vcvtneps2bf16	%xmm5, %xmm6	 #AVX-NE-CONVERT
	{vex3} vcvtneps2bf16	%xmm5, %xmm6	 #AVX-NE-CONVERT
	vcvtneps2bf16	%ymm5, %xmm6	 #AVX-NE-CONVERT
	{evex} vcvtneps2bf16	%ymm5, %xmm6	 #AVX-NE-CONVERT
	{vex} vcvtneps2bf16	%ymm5, %xmm6	 #AVX-NE-CONVERT
	{vex3} vcvtneps2bf16	%ymm5, %xmm6	 #AVX-NE-CONVERT
	vcvtneps2bf16x	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	{evex} vcvtneps2bf16x	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	{vex} vcvtneps2bf16x	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	{vex3} vcvtneps2bf16x	0x10000000(%rbp, %r14, 8), %xmm6	 #AVX-NE-CONVERT
	vcvtneps2bf16x	(%r9), %xmm6	 #AVX-NE-CONVERT
	{evex} vcvtneps2bf16x	(%r9), %xmm6	 #AVX-NE-CONVERT
	{vex} vcvtneps2bf16x	(%r9), %xmm6	 #AVX-NE-CONVERT
	{vex3} vcvtneps2bf16x	(%r9), %xmm6	 #AVX-NE-CONVERT
	vcvtneps2bf16x	2032(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(f0070000)
	{evex} vcvtneps2bf16x	2032(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(f0070000)
	{vex} vcvtneps2bf16x	2032(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(f0070000)
	{vex3} vcvtneps2bf16x	2032(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneps2bf16x	-2048(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f8ffff)
	{evex} vcvtneps2bf16x	-2048(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f8ffff)
	{vex} vcvtneps2bf16x	-2048(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f8ffff)
	{vex3} vcvtneps2bf16x	-2048(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneps2bf16y	4064(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(e00f0000)
	{evex} vcvtneps2bf16y	4064(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(e00f0000)
	{vex} vcvtneps2bf16y	4064(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(e00f0000)
	{vex3} vcvtneps2bf16y	4064(%rcx), %xmm6	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneps2bf16y	-4096(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f0ffff)
	{evex} vcvtneps2bf16y	-4096(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f0ffff)
	{vex} vcvtneps2bf16y	-4096(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f0ffff)
	{vex3} vcvtneps2bf16y	-4096(%rdx), %xmm6	 #AVX-NE-CONVERT Disp32(00f0ffff)

.intel_syntax noprefix
	vbcstnebf162ps	xmm6, WORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vbcstnebf162ps	xmm6, WORD PTR [r9]	 #AVX-NE-CONVERT
	vbcstnebf162ps	xmm6, WORD PTR [rcx+254]	 #AVX-NE-CONVERT Disp32(fe000000)
	vbcstnebf162ps	xmm6, WORD PTR [rdx-256]	 #AVX-NE-CONVERT Disp32(00ffffff)
	vbcstnebf162ps	ymm6, WORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vbcstnebf162ps	ymm6, WORD PTR [r9]	 #AVX-NE-CONVERT
	vbcstnebf162ps	ymm6, WORD PTR [rcx+254]	 #AVX-NE-CONVERT Disp32(fe000000)
	vbcstnebf162ps	ymm6, WORD PTR [rdx-256]	 #AVX-NE-CONVERT Disp32(00ffffff)
	vbcstnesh2ps	xmm6, WORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vbcstnesh2ps	xmm6, WORD PTR [r9]	 #AVX-NE-CONVERT
	vbcstnesh2ps	xmm6, WORD PTR [rcx+254]	 #AVX-NE-CONVERT Disp32(fe000000)
	vbcstnesh2ps	xmm6, WORD PTR [rdx-256]	 #AVX-NE-CONVERT Disp32(00ffffff)
	vbcstnesh2ps	ymm6, WORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vbcstnesh2ps	ymm6, WORD PTR [r9]	 #AVX-NE-CONVERT
	vbcstnesh2ps	ymm6, WORD PTR [rcx+254]	 #AVX-NE-CONVERT Disp32(fe000000)
	vbcstnesh2ps	ymm6, WORD PTR [rdx-256]	 #AVX-NE-CONVERT Disp32(00ffffff)
	vcvtneebf162ps	xmm6, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneebf162ps	xmm6, XMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneebf162ps	xmm6, XMMWORD PTR [rcx+2032]	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneebf162ps	xmm6, XMMWORD PTR [rdx-2048]	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneebf162ps	ymm6, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneebf162ps	ymm6, YMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneebf162ps	ymm6, YMMWORD PTR [rcx+4064]	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneebf162ps	ymm6, YMMWORD PTR [rdx-4096]	 #AVX-NE-CONVERT Disp32(00f0ffff)
	vcvtneeph2ps	xmm6, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneeph2ps	xmm6, XMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneeph2ps	xmm6, XMMWORD PTR [rcx+2032]	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneeph2ps	xmm6, XMMWORD PTR [rdx-2048]	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneeph2ps	ymm6, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneeph2ps	ymm6, YMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneeph2ps	ymm6, YMMWORD PTR [rcx+4064]	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneeph2ps	ymm6, YMMWORD PTR [rdx-4096]	 #AVX-NE-CONVERT Disp32(00f0ffff)
	vcvtneobf162ps	xmm6, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneobf162ps	xmm6, XMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneobf162ps	xmm6, XMMWORD PTR [rcx+2032]	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneobf162ps	xmm6, XMMWORD PTR [rdx-2048]	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneobf162ps	ymm6, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneobf162ps	ymm6, YMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneobf162ps	ymm6, YMMWORD PTR [rcx+4064]	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneobf162ps	ymm6, YMMWORD PTR [rdx-4096]	 #AVX-NE-CONVERT Disp32(00f0ffff)
	vcvtneoph2ps	xmm6, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneoph2ps	xmm6, XMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneoph2ps	xmm6, XMMWORD PTR [rcx+2032]	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneoph2ps	xmm6, XMMWORD PTR [rdx-2048]	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneoph2ps	ymm6, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneoph2ps	ymm6, YMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneoph2ps	ymm6, YMMWORD PTR [rcx+4064]	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneoph2ps	ymm6, YMMWORD PTR [rdx-4096]	 #AVX-NE-CONVERT Disp32(00f0ffff)
	vcvtneps2bf16	xmm6, xmm5	 #AVX-NE-CONVERT
	{evex} vcvtneps2bf16	xmm6, xmm5	 #AVX-NE-CONVERT
	{vex} vcvtneps2bf16	xmm6, xmm5	 #AVX-NE-CONVERT
	{vex3} vcvtneps2bf16	xmm6, xmm5	 #AVX-NE-CONVERT
	vcvtneps2bf16	xmm6, ymm5	 #AVX-NE-CONVERT
	{evex} vcvtneps2bf16	xmm6, ymm5	 #AVX-NE-CONVERT
	{vex} vcvtneps2bf16	xmm6, ymm5	 #AVX-NE-CONVERT
	{vex3} vcvtneps2bf16	xmm6, ymm5	 #AVX-NE-CONVERT
	vcvtneps2bf16	xmm6, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	{evex} vcvtneps2bf16	xmm6, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	{vex} vcvtneps2bf16	xmm6, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	{vex3} vcvtneps2bf16	xmm6, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX-NE-CONVERT
	vcvtneps2bf16	xmm6, XMMWORD PTR [r9]	 #AVX-NE-CONVERT
	{evex} vcvtneps2bf16	xmm6, XMMWORD PTR [r9]	 #AVX-NE-CONVERT
	{vex} vcvtneps2bf16	xmm6, XMMWORD PTR [r9]	 #AVX-NE-CONVERT
	{vex3} vcvtneps2bf16	xmm6, XMMWORD PTR [r9]	 #AVX-NE-CONVERT
	vcvtneps2bf16	xmm6, XMMWORD PTR [rcx+2032]	 #AVX-NE-CONVERT Disp32(f0070000)
	{evex} vcvtneps2bf16	xmm6, XMMWORD PTR [rcx+2032]	 #AVX-NE-CONVERT Disp32(f0070000)
	{vex} vcvtneps2bf16	xmm6, XMMWORD PTR [rcx+2032]	 #AVX-NE-CONVERT Disp32(f0070000)
	{vex3} vcvtneps2bf16	xmm6, XMMWORD PTR [rcx+2032]	 #AVX-NE-CONVERT Disp32(f0070000)
	vcvtneps2bf16	xmm6, XMMWORD PTR [rdx-2048]	 #AVX-NE-CONVERT Disp32(00f8ffff)
	{evex} vcvtneps2bf16	xmm6, XMMWORD PTR [rdx-2048]	 #AVX-NE-CONVERT Disp32(00f8ffff)
	{vex} vcvtneps2bf16	xmm6, XMMWORD PTR [rdx-2048]	 #AVX-NE-CONVERT Disp32(00f8ffff)
	{vex3} vcvtneps2bf16	xmm6, XMMWORD PTR [rdx-2048]	 #AVX-NE-CONVERT Disp32(00f8ffff)
	vcvtneps2bf16	xmm6, YMMWORD PTR [rcx+4064]	 #AVX-NE-CONVERT Disp32(e00f0000)
	{evex} vcvtneps2bf16	xmm6, YMMWORD PTR [rcx+4064]	 #AVX-NE-CONVERT Disp32(e00f0000)
	{vex} vcvtneps2bf16	xmm6, YMMWORD PTR [rcx+4064]	 #AVX-NE-CONVERT Disp32(e00f0000)
	{vex3} vcvtneps2bf16	xmm6, YMMWORD PTR [rcx+4064]	 #AVX-NE-CONVERT Disp32(e00f0000)
	vcvtneps2bf16	xmm6, YMMWORD PTR [rdx-4096]	 #AVX-NE-CONVERT Disp32(00f0ffff)
	{evex} vcvtneps2bf16	xmm6, YMMWORD PTR [rdx-4096]	 #AVX-NE-CONVERT Disp32(00f0ffff)
	{vex} vcvtneps2bf16	xmm6, YMMWORD PTR [rdx-4096]	 #AVX-NE-CONVERT Disp32(00f0ffff)
	{vex3} vcvtneps2bf16	xmm6, YMMWORD PTR [rdx-4096]	 #AVX-NE-CONVERT Disp32(00f0ffff)
