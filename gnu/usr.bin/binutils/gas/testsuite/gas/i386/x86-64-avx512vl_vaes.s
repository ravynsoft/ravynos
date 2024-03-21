# Check 64bit AVX512VL,VAES instructions

	.allow_index_reg
	.text
_start:
	vaesdec	%xmm28, %xmm29, %xmm30	 # AVX512VL,VAES
	vaesdec	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512VL,VAES
	vaesdec	2032(%rdx), %xmm29, %xmm30	 # AVX512VL,VAES Disp8
	vaesdec	%ymm28, %ymm29, %ymm30	 # AVX512VL,VAES
	vaesdec	(%rcx), %ymm29, %ymm30	 # AVX512VL,VAES
	vaesdec	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512VL,VAES
	vaesdec	4064(%rdx), %ymm29, %ymm30	 # AVX512VL,VAES Disp8
	vaesdeclast	%xmm28, %xmm29, %xmm30	 # AVX512VL,VAES
	vaesdeclast	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512VL,VAES
	vaesdeclast	2032(%rdx), %xmm29, %xmm30	 # AVX512VL,VAES Disp8
	vaesdeclast	%ymm28, %ymm29, %ymm30	 # AVX512VL,VAES
	vaesdeclast	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512VL,VAES
	vaesdeclast	4064(%rdx), %ymm29, %ymm30	 # AVX512VL,VAES Disp8
	vaesenc	%xmm28, %xmm29, %xmm30	 # AVX512VL,VAES
	vaesenc	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512VL,VAES
	vaesenc	2032(%rdx), %xmm29, %xmm30	 # AVX512VL,VAES Disp8
	vaesenc	%ymm28, %ymm29, %ymm30	 # AVX512VL,VAES
	vaesenc	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512VL,VAES
	vaesenc	4064(%rdx), %ymm29, %ymm30	 # AVX512VL,VAES Disp8
	vaesenclast	%xmm28, %xmm29, %xmm30	 # AVX512VL,VAES
	vaesenclast	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512VL,VAES
	vaesenclast	2032(%rdx), %xmm29, %xmm30	 # AVX512VL,VAES Disp8
	vaesenclast	%ymm28, %ymm29, %ymm30	 # AVX512VL,VAES
	vaesenclast	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512VL,VAES
	vaesenclast	4064(%rdx), %ymm29, %ymm30	 # AVX512VL,VAES Disp8

	.intel_syntax noprefix
	vaesdec	xmm30, xmm29, xmm28	 # AVX512VL,VAES
	vaesdec	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,VAES
	vaesdec	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512VL,VAES Disp8
	vaesdec	ymm30, ymm29, ymm28	 # AVX512VL,VAES
	vaesdec	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,VAES
	vaesdec	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512VL,VAES Disp8
	vaesdeclast	xmm30, xmm29, xmm28	 # AVX512VL,VAES
	vaesdeclast	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,VAES
	vaesdeclast	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512VL,VAES Disp8
	vaesdeclast	ymm30, ymm29, ymm28	 # AVX512VL,VAES
	vaesdeclast	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,VAES
	vaesdeclast	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512VL,VAES Disp8
	vaesenc	xmm30, xmm29, xmm28	 # AVX512VL,VAES
	vaesenc	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,VAES
	vaesenc	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512VL,VAES Disp8
	vaesenc	ymm30, ymm29, ymm28	 # AVX512VL,VAES
	vaesenc	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,VAES
	vaesenc	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512VL,VAES Disp8
	vaesenclast	xmm30, xmm29, xmm28	 # AVX512VL,VAES
	vaesenclast	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,VAES
	vaesenclast	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512VL,VAES Disp8
	vaesenclast	ymm30, ymm29, ymm28	 # AVX512VL,VAES
	vaesenclast	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,VAES
	vaesenclast	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512VL,VAES Disp8
