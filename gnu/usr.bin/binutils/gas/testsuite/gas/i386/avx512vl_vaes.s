# Check 32bit AVX512VL,VAES instructions

	.allow_index_reg
	.text
_start:
	vaesdec	%xmm4, %xmm5, %xmm6	 # AVX512VL,VAES
	vaesdec	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512VL,VAES
	vaesdec	2032(%edx), %xmm5, %xmm6	 # AVX512VL,VAES Disp8
	vaesdec	%ymm4, %ymm5, %ymm6	 # AVX512VL,VAES
	vaesdec	-123456(%esp,%esi,8), %ymm5, %ymm6	 # AVX512VL,VAES
	vaesdec	4064(%edx), %ymm5, %ymm6	 # AVX512VL,VAES Disp8

	vaesdeclast	%xmm4, %xmm5, %xmm6	 # AVX512VL,VAES
	vaesdeclast	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512VL,VAES
	vaesdeclast	2032(%edx), %xmm5, %xmm6	 # AVX512VL,VAES Disp8
	vaesdeclast	%ymm4, %ymm5, %ymm6	 # AVX512VL,VAES
	vaesdeclast	-123456(%esp,%esi,8), %ymm5, %ymm6	 # AVX512VL,VAES
	vaesdeclast	4064(%edx), %ymm5, %ymm6	 # AVX512VL,VAES Disp8

	vaesenc	%xmm4, %xmm5, %xmm6	 # AVX512VL,VAES
	vaesenc	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512VL,VAES
	vaesenc	2032(%edx), %xmm5, %xmm6	 # AVX512VL,VAES Disp8
	vaesenc	%ymm4, %ymm5, %ymm6	 # AVX512VL,VAES
	vaesenc	-123456(%esp,%esi,8), %ymm5, %ymm6	 # AVX512VL,VAES
	vaesenc	4064(%edx), %ymm5, %ymm6	 # AVX512VL,VAES Disp8

	vaesenclast	%xmm4, %xmm5, %xmm6	 # AVX512VL,VAES
	vaesenclast	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512VL,VAES
	vaesenclast	2032(%edx), %xmm5, %xmm6	 # AVX512VL,VAES Disp8
	vaesenclast	%ymm4, %ymm5, %ymm6	 # AVX512VL,VAES
	vaesenclast	-123456(%esp,%esi,8), %ymm5, %ymm6	 # AVX512VL,VAES
	vaesenclast	4064(%edx), %ymm5, %ymm6	 # AVX512VL,VAES Disp8

	{evex} vaesdec	%xmm4, %xmm5, %xmm6	 # AVX512VL,VAES
	{evex} vaesdec	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512VL,VAES
	{evex} vaesdec	2032(%edx), %xmm5, %xmm6	 # AVX512VL,VAES Disp8
	{evex} vaesdec	%ymm4, %ymm5, %ymm6	 # AVX512VL,VAES
	{evex} vaesdec	-123456(%esp,%esi,8), %ymm5, %ymm6	 # AVX512VL,VAES
	{evex} vaesdec	4064(%edx), %ymm5, %ymm6	 # AVX512VL,VAES Disp8

	{evex} vaesdeclast	%xmm4, %xmm5, %xmm6	 # AVX512VL,VAES
	{evex} vaesdeclast	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512VL,VAES
	{evex} vaesdeclast	2032(%edx), %xmm5, %xmm6	 # AVX512VL,VAES Disp8
	{evex} vaesdeclast	%ymm4, %ymm5, %ymm6	 # AVX512VL,VAES
	{evex} vaesdeclast	-123456(%esp,%esi,8), %ymm5, %ymm6	 # AVX512VL,VAES
	{evex} vaesdeclast	4064(%edx), %ymm5, %ymm6	 # AVX512VL,VAES Disp8

	{evex} vaesenc	%xmm4, %xmm5, %xmm6	 # AVX512VL,VAES
	{evex} vaesenc	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512VL,VAES
	{evex} vaesenc	2032(%edx), %xmm5, %xmm6	 # AVX512VL,VAES Disp8
	{evex} vaesenc	%ymm4, %ymm5, %ymm6	 # AVX512VL,VAES
	{evex} vaesenc	-123456(%esp,%esi,8), %ymm5, %ymm6	 # AVX512VL,VAES
	{evex} vaesenc	4064(%edx), %ymm5, %ymm6	 # AVX512VL,VAES Disp8

	{evex} vaesenclast	%xmm4, %xmm5, %xmm6	 # AVX512VL,VAES
	{evex} vaesenclast	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512VL,VAES
	{evex} vaesenclast	2032(%edx), %xmm5, %xmm6	 # AVX512VL,VAES Disp8
	{evex} vaesenclast	%ymm4, %ymm5, %ymm6	 # AVX512VL,VAES
	{evex} vaesenclast	-123456(%esp,%esi,8), %ymm5, %ymm6	 # AVX512VL,VAES
	{evex} vaesenclast	4064(%edx), %ymm5, %ymm6	 # AVX512VL,VAES Disp8

	.intel_syntax noprefix
	vaesdec	xmm6, xmm5, xmm4	 # AVX512VL,VAES
	vaesdec	xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	vaesdec	xmm6, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,VAES Disp8
	vaesdec	ymm6, ymm5, ymm4	 # AVX512VL,VAES
	vaesdec	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	vaesdec	ymm6, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,VAES Disp8

	vaesdeclast	xmm6, xmm5, xmm4	 # AVX512VL,VAES
	vaesdeclast	xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	vaesdeclast	xmm6, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,VAES Disp8
	vaesdeclast	ymm6, ymm5, ymm4	 # AVX512VL,VAES
	vaesdeclast	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	vaesdeclast	ymm6, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,VAES Disp8

	vaesenc	xmm6, xmm5, xmm4	 # AVX512VL,VAES
	vaesenc	xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	vaesenc	xmm6, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,VAES Disp8
	vaesenc	ymm6, ymm5, ymm4	 # AVX512VL,VAES
	vaesenc	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	vaesenc	ymm6, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,VAES Disp8

	vaesenclast	xmm6, xmm5, xmm4	 # AVX512VL,VAES
	vaesenclast	xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	vaesenclast	xmm6, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,VAES Disp8
	vaesenclast	ymm6, ymm5, ymm4	 # AVX512VL,VAES
	vaesenclast	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	vaesenclast	ymm6, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,VAES Disp8

	{evex} vaesdec	xmm6, xmm5, xmm4	 # AVX512VL,VAES
	{evex} vaesdec	xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	{evex} vaesdec	xmm6, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,VAES Disp8
	{evex} vaesdec	ymm6, ymm5, ymm4	 # AVX512VL,VAES
	{evex} vaesdec	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	{evex} vaesdec	ymm6, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,VAES Disp8

	{evex} vaesdeclast	xmm6, xmm5, xmm4	 # AVX512VL,VAES
	{evex} vaesdeclast	xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	{evex} vaesdeclast	xmm6, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,VAES Disp8
	{evex} vaesdeclast	ymm6, ymm5, ymm4	 # AVX512VL,VAES
	{evex} vaesdeclast	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	{evex} vaesdeclast	ymm6, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,VAES Disp8

	{evex} vaesenc	xmm6, xmm5, xmm4	 # AVX512VL,VAES
	{evex} vaesenc	xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	{evex} vaesenc	xmm6, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,VAES Disp8
	{evex} vaesenc	ymm6, ymm5, ymm4	 # AVX512VL,VAES
	{evex} vaesenc	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	{evex} vaesenc	ymm6, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,VAES Disp8

	{evex} vaesenclast	xmm6, xmm5, xmm4	 # AVX512VL,VAES
	{evex} vaesenclast	xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	{evex} vaesenclast	xmm6, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,VAES Disp8
	{evex} vaesenclast	ymm6, ymm5, ymm4	 # AVX512VL,VAES
	{evex} vaesenclast	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,VAES
	{evex} vaesenclast	ymm6, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,VAES Disp8
