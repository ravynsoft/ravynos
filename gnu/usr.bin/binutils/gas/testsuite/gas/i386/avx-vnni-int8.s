# Check 32bit AVX-VNNI-INT8 instructions

	.allow_index_reg
	.text
_start:
	vpdpbssd	%ymm4, %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbssd	%xmm4, %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbssd	0x10000000(%esp, %esi, 8), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbssd	(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbssd	4064(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbssd	-4096(%edx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbssd	0x10000000(%esp, %esi, 8), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbssd	(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbssd	2032(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbssd	-2048(%edx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbssds	%ymm4, %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbssds	%xmm4, %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbssds	0x10000000(%esp, %esi, 8), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbssds	(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbssds	4064(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbssds	-4096(%edx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbssds	0x10000000(%esp, %esi, 8), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbssds	(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbssds	2032(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbssds	-2048(%edx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbsud	%ymm4, %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbsud	%xmm4, %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbsud	0x10000000(%esp, %esi, 8), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbsud	(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbsud	4064(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbsud	-4096(%edx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbsud	0x10000000(%esp, %esi, 8), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbsud	(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbsud	2032(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbsud	-2048(%edx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbsuds	%ymm4, %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbsuds	%xmm4, %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbsuds	0x10000000(%esp, %esi, 8), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbsuds	(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbsuds	4064(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbsuds	-4096(%edx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbsuds	0x10000000(%esp, %esi, 8), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbsuds	(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbsuds	2032(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbsuds	-2048(%edx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbuud	%ymm4, %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbuud	%xmm4, %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbuud	0x10000000(%esp, %esi, 8), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbuud	(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbuud	4064(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbuud	-4096(%edx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbuud	0x10000000(%esp, %esi, 8), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbuud	(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbuud	2032(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbuud	-2048(%edx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbuuds	%ymm4, %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbuuds	%xmm4, %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbuuds	0x10000000(%esp, %esi, 8), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbuuds	(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8
	vpdpbuuds	4064(%ecx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbuuds	-4096(%edx), %ymm5, %ymm6	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbuuds	0x10000000(%esp, %esi, 8), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbuuds	(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8
	vpdpbuuds	2032(%ecx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbuuds	-2048(%edx), %xmm5, %xmm6	 #AVX-VNNI-INT8 Disp32(00f8ffff)

.intel_syntax noprefix
	vpdpbssd	ymm6, ymm5, ymm4	 #AVX-VNNI-INT8
	vpdpbssd	xmm6, xmm5, xmm4	 #AVX-VNNI-INT8
	vpdpbssd	ymm6, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbssd	ymm6, ymm5, YMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbssd	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbssd	ymm6, ymm5, YMMWORD PTR [edx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbssd	xmm6, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbssd	xmm6, xmm5, XMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbssd	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbssd	xmm6, xmm5, XMMWORD PTR [edx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbssds	ymm6, ymm5, ymm4	 #AVX-VNNI-INT8
	vpdpbssds	xmm6, xmm5, xmm4	 #AVX-VNNI-INT8
	vpdpbssds	ymm6, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbssds	ymm6, ymm5, YMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbssds	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbssds	ymm6, ymm5, YMMWORD PTR [edx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbssds	xmm6, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbssds	xmm6, xmm5, XMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbssds	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbssds	xmm6, xmm5, XMMWORD PTR [edx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbsud	ymm6, ymm5, ymm4	 #AVX-VNNI-INT8
	vpdpbsud	xmm6, xmm5, xmm4	 #AVX-VNNI-INT8
	vpdpbsud	ymm6, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbsud	ymm6, ymm5, YMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbsud	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbsud	ymm6, ymm5, YMMWORD PTR [edx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbsud	xmm6, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbsud	xmm6, xmm5, XMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbsud	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbsud	xmm6, xmm5, XMMWORD PTR [edx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbsuds	ymm6, ymm5, ymm4	 #AVX-VNNI-INT8
	vpdpbsuds	xmm6, xmm5, xmm4	 #AVX-VNNI-INT8
	vpdpbsuds	ymm6, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbsuds	ymm6, ymm5, YMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbsuds	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbsuds	ymm6, ymm5, YMMWORD PTR [edx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbsuds	xmm6, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbsuds	xmm6, xmm5, XMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbsuds	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbsuds	xmm6, xmm5, XMMWORD PTR [edx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbuud	ymm6, ymm5, ymm4	 #AVX-VNNI-INT8
	vpdpbuud	xmm6, xmm5, xmm4	 #AVX-VNNI-INT8
	vpdpbuud	ymm6, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbuud	ymm6, ymm5, YMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbuud	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbuud	ymm6, ymm5, YMMWORD PTR [edx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbuud	xmm6, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbuud	xmm6, xmm5, XMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbuud	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbuud	xmm6, xmm5, XMMWORD PTR [edx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
	vpdpbuuds	ymm6, ymm5, ymm4	 #AVX-VNNI-INT8
	vpdpbuuds	xmm6, xmm5, xmm4	 #AVX-VNNI-INT8
	vpdpbuuds	ymm6, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbuuds	ymm6, ymm5, YMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbuuds	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX-VNNI-INT8 Disp32(e00f0000)
	vpdpbuuds	ymm6, ymm5, YMMWORD PTR [edx-4096]	 #AVX-VNNI-INT8 Disp32(00f0ffff)
	vpdpbuuds	xmm6, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX-VNNI-INT8
	vpdpbuuds	xmm6, xmm5, XMMWORD PTR [ecx]	 #AVX-VNNI-INT8
	vpdpbuuds	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX-VNNI-INT8 Disp32(f0070000)
	vpdpbuuds	xmm6, xmm5, XMMWORD PTR [edx-2048]	 #AVX-VNNI-INT8 Disp32(00f8ffff)
